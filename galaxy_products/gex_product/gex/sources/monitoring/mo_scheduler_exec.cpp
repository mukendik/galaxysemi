//#include <QCoreApplication>
#include <QProgressBar>
#include <QApplication>
#include <gqtl_log.h>
#include <gqtl_sysutils.h>
#include "scheduler_engine.h"
#include "browser_dialog.h"
#include "engine.h"
#include "db_engine.h"
#include "db_external_database.h"
#include "gex_database_entry.h"
#include "mo_task.h"
#include "datapump/datapump_taskdata.h"
#include "trigger_pat_task.h"

extern QProgressBar *	GexProgressBar;	// Handle to progress bar in status bar
extern GexMainwindow *	pGexMainWindow;

///////////////////////////////////////////////////////////
// Execute DataPump task...
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::InsertDataFileLoop(CGexMoTaskDataPump *ptTask, QString DataFile)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Insert DataFile Loop for task %1...")
          .arg(ptTask?ptTask->m_strName:"?").toLatin1().constData());

    QString     strErrorMessage;
    QString     strFileName, strFullFileName;
    QStringList strDataFiles;

    if(ptTask == NULL)
        return;

    // Process GUI pending messages
    QCoreApplication::processEvents();

    if(DataFile.isEmpty())
    {
        // Get list of files only once
        strDataFiles = GS::Gex::Engine::GetInstance().GetDatabaseEngine().GetListOfFiles(
                    ptTask->GetDataFilePath(), ptTask->GetDataFileExtension(),
                    ptTask->IsDataFileScanSubFolder(), ptTask->GetDataFileSort(),
                    ptTask->GetPriority());
    }
    else
        strDataFiles << DataFile;

    // Check if any file retrieved
    QFileInfo FileInfo;
    QStringList::iterator it = strDataFiles.begin();
    while(it != strDataFiles.end())
    {
        strErrorMessage = "";

        // case 4863 : Check pause requested or Execution Window
        // Pause requested, so stop insertion for now...
        // Execution forced
        if((isSchedulerStopped()) && (ptTask->m_tLastExecuted != 1))
            break;


        // If a Time window is defined, make sure we still fall into it!
        if(!ptTask->IsUsable(!isSchedulerStopped()))
            break;	// NOT in window, then quiet exit!

        // Get first file and process it (unless already processed and belongs to list of processed files).
        strFileName = *it;
        strDataFiles.erase(it);

        //strFullFileName = ptTask->ptDataPump->strDataPath + "/" + strFileName;
        //FileInfo.setFile(ptTask->ptDataPump->strDataPath,strFileName);
        FileInfo.setFile(strFileName);
        strFullFileName = FileInfo.absoluteFilePath();

        // Process GUI pending messages
        QCoreApplication::processEvents();

        CGexSystemUtils::NormalizePath(strFullFileName);

        // Make sure file still exists in directory before importing it
        if(QFile::exists(strFullFileName))
        {
            // If insertion successful...then update the Status HTML reports (if enabled)
            if(GS::Gex::Engine::GetInstance().GetDatabaseEngine().ImportOneFile(
                        ptTask,strFullFileName,strErrorMessage,false))
            {
                ExecuteStatusTask();
            }
        }
        else
            strErrorMessage = "File not exists or not accessible";

        // Execution forced
        // Just execute one insertion
        if((isSchedulerStopped()) && (ptTask->m_tLastExecuted == 1))
            break;

        it++;

    }

}


///////////////////////////////////////////////////////////
// Execute DataPump task...
///////////////////////////////////////////////////////////
QString GS::Gex::SchedulerEngine::ExecuteTriggerPumpTask(CGexMoTaskTriggerPatPump *ptTask, QString DataFile)
{
    ///////////////////////
    // LoadBalancing
    // Get the file to process
    // GetNextAction(ActionId)
    // If no action => NOTHING TO DO

    // Debug message
    QString strString;
    strString = "Execute Trigger Pump Task '";
    strString += ptTask->GetProperties()->strTitle;
    strString += "'";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strString.toLatin1().data());

    // Ensure the Status HTML reports (if enabled) are up to date.
    ExecuteStatusTask();

    ExecuteTriggerPumpLoop(ptTask, DataFile);

    return "";
}

///////////////////////////////////////////////////////////
// Execute TriggerPump task...
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::ExecuteTriggerPumpLoop(CGexMoTaskTriggerPatPump *ptTask, QString DataFile)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Execute TriggerPump Loop for task %1...")
          .arg( ptTask?ptTask->m_strName:"?").toLatin1().constData());

    QString     strErrorMessage;
    QString     strFileName, strFullFileName;
    QStringList strDataFiles;

    if(ptTask == NULL)
        return;

    // Process GUI pending messages
    QCoreApplication::processEvents();

    if(DataFile.isEmpty())
    {
        // Get list of files only once
        strDataFiles = GS::Gex::Engine::GetInstance().GetDatabaseEngine().GetListOfFiles(
                    ptTask->GetDataFilePath(), ptTask->GetDataFileExtension(),
                    ptTask->IsDataFileScanSubFolder(), ptTask->GetDataFileSort(),
                    ptTask->GetPriority());
    }
    else
        strDataFiles << DataFile;

    // Check if any file retrieved
    QFileInfo FileInfo;
    QStringList::iterator it = strDataFiles.begin();
    while(it != strDataFiles.end())
    {

        strErrorMessage = "";

        // case 4863 : Check pause requested or Execution Window
        // Pause requested, so stop insertion for now...
        // Execution forced
        if((isSchedulerStopped()) && (ptTask->m_tLastExecuted != 1))
            break;


        // If a Time window is defined, make sure we still fall into it!
        if(!ptTask->IsUsable(!isSchedulerStopped()))
            break;	// NOT in window, then quiet exit!

        // Get first file and process it (unless already processed and belongs to list of processed files).
        strFileName = *it;
        strDataFiles.erase(it);

        //strFullFileName = ptTask->ptDataPump->strDataPath + "/" + strFileName;
        //FileInfo.setFile(ptTask->ptDataPump->strDataPath,strFileName);
        FileInfo.setFile(strFileName);
        strFullFileName = FileInfo.absoluteFilePath();

        QString cause;
        // Check if the file can be processed
        // .read, .delay, .bad, .quarantine must be ignored
        if(!GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckIfFileToProcess(FileInfo.fileName(),ptTask, cause))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("This file does not have to be processed: %1").arg( cause).toLatin1().constData());
            continue;	// No insertion done.
        }

        onStartTask(ptTask,"FileName="+strFullFileName);

        // Process GUI pending messages
        QCoreApplication::processEvents();

        CGexSystemUtils::NormalizePath(strFullFileName);

        // Check if file already imported
        if(QFile::exists(strFullFileName))
        {
            // Check if the folder and the file can be edited
            if(!GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckFileForCopy(strFullFileName,ptTask,cause))
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("This file cannot be processed: %1").arg( cause).toLatin1().constData());
                strErrorMessage =  "error: ";
                strErrorMessage += "Execution failed: " + cause;
                strErrorMessage += "\n\tSource: " + strFileName;
            }
        }
        else
            strErrorMessage = "error:File not exists or not accessible";

        // Make sure file still exists in directory before importing it
        if(strErrorMessage.isEmpty())
        {
            // If insertion successful...then update the Status HTML reports (if enabled)
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().ExecuteScriptFile(ptTask,strFullFileName,strErrorMessage);
        }

        onStopTask(ptTask,strErrorMessage);

        // Execution forced
        // Just execute one insertion
        if((isSchedulerStopped()) && (ptTask->m_tLastExecuted == 1))
            break;

        it++;
    }
}


///////////////////////////////////////////////////////////
// Execute DataPump task...
///////////////////////////////////////////////////////////
QString GS::Gex::SchedulerEngine::ExecuteDataPumpTask(CGexMoTaskDataPump *ptTask, QString DataFile)
{
    ///////////////////////
    // LoadBalancing
    // Get the file to process
    // GetNextAction(ActionId)
    // If no action => NOTHING TO DO

    QString strString;
    strString = "Execute Data Pump Task '";
    strString += ptTask->GetProperties()->strTitle;
    strString += "'";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strString.toLatin1().data());

    // Ensure the Status HTML reports (if enabled) are up to date.
    ExecuteStatusTask();

    InsertDataFileLoop(ptTask, DataFile);

    return "";
}


