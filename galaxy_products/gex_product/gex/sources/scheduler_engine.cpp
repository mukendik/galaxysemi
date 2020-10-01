#include <QSqlError>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gqtl_log.h>
#include <gstdl_utils_c.h>

#include "engine.h"
#include "browser.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "scheduler_engine.h"
#include "datapump/olddatapump_task.h"
#include "datapump/datapump_task.h"
#include "datapump/datapump_taskdata.h"
#include "yield/yield_task.h"
#include "yield/yield_taskdata.h"
#include "reporting/reporting_task.h"
#include "reporting/reporting_taskdata.h"
#include "status/status_task.h"
#include "status/status_taskdata.h"
#include "outlierremoval/outlierremoval_task.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "autoadmin/autoadmin_task.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "converter/converter_task.h"
#include "converter/converter_taskdata.h"
#include "pat/pat_task.h"
#include "mo_task.h"
#include "mo_email.h"
//#include "gex_constants.h"
#include "db_engine.h"
#include "gex_scriptengine.h"
#include "gexmo_constants.h"
//#include "temporary_files_manager.h"
#include "product_info.h"
#include "gex_database_entry.h"
#include "db_datakeys_dialog.h"
#include "gqtl_datakeys.h"
#include "admin_engine.h"
#include "csl/csl_engine.h"
#include "gexdb_plugin_option.h"

extern GexScriptEngine* pGexScriptEngine;

// in script_wizard.h
extern void ConvertToScriptString(QString &strFile);

///////////////////////////////////////////////////////////
// Write FTP trigger file in same folder as specified data file.
///////////////////////////////////////////////////////////
static int  WriteFtpTriggerFile(QString strFileName,GS::QtLib::DatakeysContent & cKeyContent)
{
    QDateTime   cCurrentDateTime = QDateTime::currentDateTime();
    QFileInfo   cFileInfo(strFileName);
    QString     strTriggerFile = cFileInfo.absolutePath();
    if((strTriggerFile.endsWith("/") || strTriggerFile.endsWith("\\")) == false)
        strTriggerFile += "/ftp_trigger_";
    strTriggerFile += cCurrentDateTime.toString("yyyy_MM_dd_hhmmss.zzz");
    strTriggerFile += ".gtf";

    // Build path to the 'Tasks' list.
    QFile file(strTriggerFile); // Write the text to the file
    if(file.open(QIODevice::WriteOnly) == false)
        return -1;  // Failed writing FTP trigger file

    // Write FTP Trigger
    QTextStream hFtpTrigger(&file); // Assign file handle to data stream

    // Check if valid header...or empty!
    hFtpTrigger << "<GalaxyTrigger>" << endl;
    hFtpTrigger << "Action=FTP" << endl;
    hFtpTrigger << "# FTP file" << endl;
    hFtpTrigger << "SourceFile=" << strFileName << endl;
    hFtpTrigger << "FtpServer=" << cKeyContent.Get("FtpServer").toString() << endl;
    hFtpTrigger << "FtpPort=" << QString::number(cKeyContent.Get("FtpPort").toInt()) << endl;
    hFtpTrigger << "FtpUser=" << cKeyContent.Get("FtpUser").toString() << endl;
    hFtpTrigger << "FtpPassword=" << cKeyContent.Get("FtpPassword").toString() << endl;
    hFtpTrigger << "FtpPath=" << cKeyContent.Get("FtpPath").toString() << endl;
    hFtpTrigger << "</GalaxyTrigger>" << endl;

    // Success: FTP trigger created
    return 0;
}

///////////////////////////////////////////////////////////
// Move file: re-try for 'iTimeoutSec' seconds
// retry period: 'iMillSecRetry'
///////////////////////////////////////////////////////////
bool    toolFileMove(QString &strSource,QString &strDest, bool bDeleteSource=true,bool bMoveFile=true,int iTimeoutSec=10,int iMillSecRetry=100)
{
    // change the message here
    QString strString;
    QDir    cDir;
    bool    bResult;
    QTime   cTime,cWait;
    QTime   cTimeoutTime = QTime::currentTime();
    cTimeoutTime = cTimeoutTime.addSecs(iTimeoutSec);

    if(bDeleteSource)
    {
        strString = "Delete the File ";
        strString += strDest;
        GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().data());

        do
        {
            // clear flags
            bResult = false;

            // Delete destination file (unless doesn't exist!)
            if(cDir.exists(strDest) == false)
                bResult = true;
            else
                bResult = GS::Gex::Engine::RemoveFileFromDisk(strDest);

            // Delete file
            if(bResult == true)
                break;	// Success deleting destination file.

            // Failed deleting file....then wait Xmsec & retry.
            cTime = QTime::currentTime();
            cWait = cTime.addMSecs(iMillSecRetry);
            do
            {
                cTime = QTime::currentTime();
            }
            while(cTime < cWait);
        }
        while(cTime < cTimeoutTime);

        // Timeout error: failed to delete file.
        if(!bResult)
            return false;
    }

    // Now move source to target
    if(bMoveFile)
    {
        strString = "File Move ";
        strString += strSource;
        strString += " to ";
        strString += strDest;
        GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().data());

        QString strDir;
        strDir = cDir.absoluteFilePath(strDest);
        if(cDir.exists(strDir))
        {
            // Try to create folder if it doesn't exist
            int     nEndSection = 1;
            QString strPartialDir;

            strDir.replace('\\', '/');
            strPartialDir = strDir.section('/', 0, nEndSection++, QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep);
            while(strPartialDir.length() < strDir.length())
            {
                if(!cDir.exists(strPartialDir) && !cDir.mkdir(strPartialDir))
                    break;
                strPartialDir = strDir.section('/', 0, nEndSection++, QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep);
            }
        }

        do
        {
            bResult = cDir.rename(strSource,strDest);

            // Move file
            if(bResult == true)
                break;  // Success moving file.

            // Failed moving file....then wait Xmsec & retry.
            cTime = QTime::currentTime();
            cWait = cTime.addMSecs(iMillSecRetry);
            do
            {
                cTime = QTime::currentTime();
            }
            while(cTime < cWait);
        }
        while(cTime < cTimeoutTime);

        // Timeout error: failed to move file.
        if(!bResult)
            return false;
    }

    return true;
}


namespace GS
{
namespace Gex
{
SchedulerEngine::SchedulerEngine(QObject* parent):QObject(parent)
{
    GSLOG(6, "Creating a Scheduler Engine...");

    setObjectName("GSSchedulerEngine");

    mAllowed = false;
    // Flag set to 'true' when currenty processing tasks.
    mSchedulerProcessing=false;
    mTaskBeingEdited = false;
    mAllTasksLoaded = false;

    // Flag set to 'true' when the monitoring timer has been started
    mSchedulerTimerStarted = false;

    // Clear status message window.
    mSchedulerStopped = true;
    mLocalTasksIndex = -1;

    mLastTimeTasksLoaded.start();
    mLastDbTasksUpdateCheckSum = 0;
    mLastDbTasksIdChecksum = 0;
}


QString SchedulerEngine::ExecuteDataFileAnalysis(QtLib::DatakeysContent &dbKeysContent,
                                                 QList<int>& /*cSitesList*/)
{
    QString strScriptFile;
    QString strErrorMessage;

    // Create script that will read data file + compute all statistics (but NO report created)
    strScriptFile = GS::Gex::Engine::GetInstance().GetAssistantScript();
    FILE *hFile = fopen(strScriptFile.toLatin1().constData(),"w");

    if(hFile == NULL)
    {
        strErrorMessage = "  > Failed to create script file: " + strScriptFile;
        return strErrorMessage;
    }

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        GEX_ASSERT(false);
        GSLOG(3, "Failed to write option section");
        strErrorMessage = "  > Failed to write option section";
        return strErrorMessage;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  var group_id;\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");

    // Check if single-site or multi-sites...
    QString strFileName = dbKeysContent.Get("StdfFileName").toString();
    ConvertToScriptString(strFileName);	// Ensure name is script-compatible (no \)
    dbKeysContent.SetInternal("StdfFileName","");
    dbKeysContent.SetInternal("StdfFileName",strFileName);
#if 0
    if(cSitesList.count() > 1)
    {
        for(uSite = 0; uSite < cSitesList.count(); uSite++)
        {
            fprintf(hFile,"  group_id = gexGroup('insert','Site %d');\n",cSitesList[uSite]);
            fprintf(hFile,"  gexFile(group_id,'insert','%s','%d','all',' ','');\n",
                    dbKeysContent.strStdfFileName.toLatin1().constData(), cSitesList[uSite]);
        }
    }
    else
#endif
    {
        fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");
        fprintf(hFile,"  gexFile(group_id,'insert','%s','All','all',' ','');\n", dbKeysContent.Get("StdfFileName").toString().toLatin1().constData());
    }
    fprintf(hFile,"}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n");
    fprintf(hFile,"  gexOptions('dataprocessing','data_cleaning_mode','none');\n");   // If outlier removal enabled, standard histograms are forced over the X.sigma space only.
    fprintf(hFile,"  gexReportType('histogram','test_over_limits','all');\n");
    fprintf(hFile,"  SetProcessData();\n");
    fprintf(hFile,"  gexOptions('report','build','false');\n");	// Only data analysis, no report created!
    fprintf(hFile,"  gexBuildReport('home','0');\n");
    fprintf(hFile,"}\n\n");
    fclose(hFile);

    // Execute script.
    GS::Gex::CSLStatus lStatus = GS::Gex::CSLEngine::GetInstance().RunScript(strScriptFile);

    if(lStatus.IsFailed())
        strErrorMessage = "  > " + lStatus.GetErrorMessage();

    return strErrorMessage;
}

///////////////////////////////////////////////////////////
// Scheduler engine
///////////////////////////////////////////////////////////
void SchedulerEngine::Activate()
{
    // If already initialized
    if(mAllowed) return;

    mAllowed = true;

    emit sDisplayStatusMessage("");

    // Load tasks from disk
    LoadTasksList(true);

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // Timer used to check for "Rename of temporary files" (resolution: .5 sec)
        connect( &timerRenameFiles, SIGNAL(timeout()),this, SLOT(OnCheckRenameFiles(void)) );

        // Write into Log file
        AppendMoHistoryLog("", "Launching", Engine::GetInstance().Get("AppFullName").toString() );

        AppendMoHistoryLog("Server Profile loaded (" +
                               Engine::GetInstance().Get("UserFolder").toString() + ")");

        // SCHEDULER_INTERVAL_DEFINITION (use this tag to retrieve all linked codes)
        // Start scheduler timer (resolution: 1 minute, unless we are in Y123 mode)
        connect( &timerScheduler, SIGNAL(timeout()),this, SLOT(OnCheckScheduler(void)) );
    }
}

///////////////////////////////////////////////////////////
// Task Scheduler Destructor function.
///////////////////////////////////////////////////////////
SchedulerEngine::~SchedulerEngine()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // Write into Log file
        AppendMoHistoryLog("","Closing", Engine::GetInstance().Get("AppFullName").toString() );
    }

    qDeleteAll(mTasksList);
    mTasksList.clear();
}

void SchedulerEngine::RunTaskScheduler(bool bRun)
{
    if(bRun)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "RunTaskScheduler start");
    }
    else
    {
        GSLOG(SYSLOG_SEV_NOTICE, "RunTaskScheduler stop");
    }

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    // Keep track of scheduler status.
    mSchedulerStopped = !bRun;

    // Update node status
    // m_ActiveScheduler can be forced by ym_admin_db
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        GS::Gex::Engine::GetInstance().GetAdminEngine().UpdateNodeStatus();

    // Update GUI accordingly
    emit sDisplayStatusMessage("");

    // Call Scheduler loop now just to have GUI fully updated!
    QTimer::singleShot(0, this, SLOT(OnCheckScheduler(void)));
}

bool  SchedulerEngine::EnableAllTasks(QString type)
{
    //LoadDbTasks(false);
    GSLOG(6, QString("Enable all tasks of type %1").arg(type).toLatin1().data());
    QVector<CGexMoTaskItem*> output;

    FindAllTasksOfType(type.toUpper(), output);
    foreach(CGexMoTaskItem* task, output)
        if (task){
            task->SetEnabledState(true);
            CheckTaskStatus(task,"Folder|DatabaseStatus");
        }
    return true;
}

bool  SchedulerEngine::DisableAllTasks(QString type)
{
    //LoadDbTasks(false);
    GSLOG(6, QString("Disable all tasks of type '%1'").arg(type).toLatin1().data());
    QVector<CGexMoTaskItem*> output;

    FindAllTasksOfType(type.toUpper(), output);
    foreach(CGexMoTaskItem* task, output)
        if (task){
            task->SetEnabledState(false);
            CheckTaskStatus(task);
        }

    return true;
}

bool SchedulerEngine::EnableTask(QString TaskName)
{
    GSLOG(6, QString("Enable task '%1'").arg(TaskName).toLatin1().data());
    bool isTaskFound =false;
    CGexMoTaskItem* ptTask = NULL;
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    lstIteratorTask.toFront();
    while (lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();
        if(!ptTask)
            continue;
        if(ptTask->m_strName.toUpper()==TaskName.toUpper())
        {
            ptTask->SetEnabledState(true);
            CheckTaskStatus(ptTask,"Folder|DatabaseStatus");
            isTaskFound = true;

            // Check if the linked database is connected
            if(ptTask->m_iDatabaseId > 0)
            {
                // Check if database is referenced
                GexDatabaseEntry *pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(ptTask->m_iDatabaseId);
                if(pDatabaseEntry
                        && !(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
                        &&  (pDatabaseEntry->StatusFlags() & STATUS_MANUALLY_DISCONNECTED))
                {
                    // Database not connected
                    // Force the auto connection
                    pDatabaseEntry->UnsetStatusFlags(STATUS_MANUALLY_DISCONNECTED);
                    GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckDatabaseEntryStatus(pDatabaseEntry,false);
                    CheckTaskStatus(ptTask,"Folder|DatabaseStatus");
                }
            }
        }
    }
    if (!isTaskFound)
        GSLOG(3, QString("Task '%1' not found!").arg(TaskName).toLatin1().data());

    return true;
}

bool SchedulerEngine::DisableTask(QString TaskName)
{
    GSLOG(6, QString("Disable task '%1'").arg(TaskName).toLatin1().data());
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    CGexMoTaskItem* ptTask = NULL;
    lstIteratorTask.toFront();
    while (lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();
        if (!ptTask)
            continue;
        if (ptTask->m_strName.toUpper()==TaskName.toUpper())
        {
            ptTask->SetEnabledState(false);
            CheckTaskStatus(ptTask);
        }
    }

    return true;
}

QString SchedulerEngine::FindTasksByName(const QString &taskname, QList<CGexMoTaskItem*> &list)
{
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    CGexMoTaskItem* ptTask = NULL;
    lstIteratorTask.toFront();
    while (lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();
        if (!ptTask)
            continue;
        if (ptTask->m_strName.toUpper()==QString(taskname).toUpper())
        {
            list.push_back(ptTask);
        }
    }

    return "ok";
}

bool    SchedulerEngine::FindAllTasksOfType(const QString & type, QVector<CGexMoTaskItem*>& output)
{
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    CGexMoTaskItem * ptTask = NULL;

    lstIteratorTask.toFront();

    while (lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();
        if (!ptTask)
            continue;
        if (type.isEmpty() || ptTask->GetTypeName()==type)
            output.push_back(ptTask);
    }
    return true;
}

bool    SchedulerEngine::FindAllTasksOfType(int type, QVector<CGexMoTaskItem*>& output)
{
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    CGexMoTaskItem * ptTask = NULL;

    lstIteratorTask.toFront();

    while (lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();
        if (!ptTask)
            continue;
        if (type==-1 || ptTask->GetTaskType()==type) // 3 = SYA
            output.push_back(ptTask);
    }
    return true;
}

QScriptValue SchedulerEngine::FindAllTasksOfType(int type)
{
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    CGexMoTaskItem* ptTask = NULL;
    lstIteratorTask.toFront();
    int i=0;
    while (lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();
        if (!ptTask)
            continue;
        if (type==-1 || ptTask->GetTaskType()==type)
            i++; //output.append(ptTask->GetID()); // push_back
    }

    QScriptValue sv=pGexScriptEngine->newArray( i );
    i=0;
    lstIteratorTask.toFront();
    while (lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();
        if (!ptTask)
            continue;

        if (type==-1 || ptTask->GetTaskType()==type)
        {
            //QScriptValue tsv; tsv.setData();
            //sv.setProperty(i++, ptTask->GetID());
            sv.setProperty(i++, pGexScriptEngine->newQObject(ptTask) );
        }
    }

    return sv;
}

CGexMoTaskItem *SchedulerEngine::FindTaskInList(int iTaskId)
{
    QListIterator<CGexMoTaskItem*> lstIteratorTask(mTasksList);
    CGexMoTaskItem * ptTask = NULL;

    lstIteratorTask.toFront();

    while(lstIteratorTask.hasNext())
    {
        ptTask = lstIteratorTask.next();

        // Task identified by iTaskId number
        if(ptTask->GetID() == iTaskId)
            return ptTask;
    }

    // Task title not found!
    return NULL;
}


///////////////////////////////////////////////////////////
// Delete given task structure from list (name specified by caller).
///////////////////////////////////////////////////////////
void SchedulerEngine::DeleteTaskInList(CGexMoTaskItem *ptTask)
{
    if(ptTask == NULL)
        return;

    if(mTasksList.contains(ptTask))
        mTasksList.removeAll(ptTask);

    // Remove task in gui
    emit sUpdateListViewItem(ptTask,"delete");

    // Destroy it from memory.
    delete ptTask;
}


///////////////////////////////////////////////////////////
// Delete Task list in memory, empty list view.
///////////////////////////////////////////////////////////
void SchedulerEngine::DeleteTasksList()
{
    CGexMoTaskItem * ptTask = NULL;
    while(!mTasksList.isEmpty())
    {
        ptTask = mTasksList.takeFirst();

        // Delete node
        delete ptTask;
    };

    // Clear list view.
    emit sUpdateListView(QStringList()<<"Clear");

    mLastTimeTasksLoaded.start();
    mLastLocalTasksLoaded = QDateTime::currentDateTime().addYears(-10);
    // Case 7200: use CHECKSUM on last_update rather than MAX
    mLastDbTasksUpdateCheckSum = 0;
    mLastDbTasksIdChecksum = 0;
    mLocalTasksIndex = -1;
    mAllTasksLoaded = false;

}

QString SchedulerEngine::ExecuteOneAction(int ActionId)
{
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        return "";
    if(ActionId < 0)
        return "";

    if (GS::Gex::Engine::GetInstance().CheckForLicense()!=0)
        return "error: bad license";

    CGexMoTaskItem* ptTask = NULL;
    int     TaskId,DatabaseId;
    QString ActionType;
    QString Command;

    GS::Gex::Engine::GetInstance().GetAdminEngine().GetActionInfo(ActionId,ActionType,TaskId,DatabaseId,Command);

    QString     strError;
    if(ActionType == "INCREMENTAL_UPDATE")
    {
        // Check if have a good DatabaseId
        if(DatabaseId < 0)
            strError = "Bad action item";
        else
        {
            GexDatabaseEntry *pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(DatabaseId);
            if((pDatabaseEntry == NULL)
                    || (pDatabaseEntry->m_pExternalDatabase == NULL))
                strError = "Bad action item";
            else
            {
                if(pDatabaseEntry->m_pExternalDatabase->IsDbConnected())
                {
                    QMap<QString,QString> lUpdate = GS::Gex::Engine::GetInstance().GetAdminEngine().SplitCommand(Command.trimmed());

                    // For each, insert an action
                    QString strStatusMessage = "Performing incremental updates on database <b>" + pDatabaseEntry->m_strDatabaseRef+"</b>";
                    emit sDisplayStatusMessage(strStatusMessage);
                    GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage(
                                QString("Executing Update[%1] - TestingStage[%2] - Target[%3]...")
                                .arg(lUpdate["db_update_name"])
                                .arg(lUpdate["testing_stage"])
                                .arg(lUpdate["target"]));

                    // Set task ActionId
                    pDatabaseEntry->SetActionMng(ActionId,Command);;
                    if(!pDatabaseEntry->IncrementalUpdate(lUpdate["db_update_name"],
                                                          lUpdate["testing_stage"],
                                                          lUpdate["target"],
                                                          lUpdate))
                        strError = lUpdate["error"];

                    emit sDisplayStatusMessage("");
                    GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage("");
                }
            }
        }
    }
    else
    {
        // Check if have a good TaskId
        if(TaskId < 0)
            strError = "Bad action item";
        else
        {
            // Get the Task ptr
            ptTask = FindTaskInList(TaskId);
            if(ptTask == NULL)
                strError = "Bad task item";
            else
            {
                // Check if the Task can be executed (check also ExecutionWindows or Frenquency)
                if(!ptTask->IsUsable(!isSchedulerStopped()))
                {
                    // Action can be validated by another node
                    // Check again the status
                    CheckTaskStatus(ptTask,"Folder|DatabaseStatus");
                    if(!ptTask->IsUsable(!isSchedulerStopped()))
                    {
                        strError = "Task was disabled";
                        if(!ptTask->m_strLastInfoMsg.isEmpty())
                            strError+= ": "+ptTask->m_strLastInfoMsg;
                    }
                }
            }
        }
        if(strError.isEmpty())
        {
            QString strFileName;
            // Extract file_name
            strFileName = GS::Gex::Engine::GetInstance().GetAdminEngine().SplitCommand(Command.trimmed())["FileName"];

            CheckTaskStatus(ptTask);

            if(ptTask->IsUsable(true))
            {
                // Set task
                ptTask->SetActionMng(ActionId,Command);
                strError = ExecuteTask(ptTask,strFileName);
            }
            else
                strError = ptTask->m_strLastInfoMsg.section(".",0,0);
        }
    }

    return strError;
}

bool SchedulerEngine::FindPassWafersOrSublots( const QStringList &all_wafers_or_sublots,
                                               QStringList &pass_wafers_or_sublots ) const
{
    for( int i = 0, end = all_wafers_or_sublots.size(); i < end; ++i )
    {
        // current string
        const QString str = all_wafers_or_sublots[ i ];

        const QStringList value_and_status = str.split( ':' );

        // current value
        const QString value = value_and_status[0];

        // current status (pass or fail)
        const QString status = value_and_status[1];

        if( status.compare( "pass", Qt::CaseInsensitive ) == 0 )
        {
            pass_wafers_or_sublots << value;
        }
    }

   return true;
}

bool SchedulerEngine::GetPassingValuesRelatedTo( const QStringList &all_values,
                                                 const QStringList &collection_of_related,
                                                 QStringList &passing_values ) const
{
    for( int i = 0, end = all_values.length(); i < end; ++i )
    {
        // current value
        const QString line = all_values[ i ];

        if(!line.contains(":"))
            // Bad format
            return false;

        // getting value and related
        const QStringList related_with_value = line.split( ':' );

        const QString related = related_with_value[ 0 ];
        const QString value = related_with_value[ 1 ];

        // storing only values whose related are in related collection
        if( collection_of_related.contains( related ) )
        {
            passing_values << value;
        }
    }
    return true;
}

QString SchedulerEngine::InsertNewActions(GexDatabaseEntry *pDatabaseEntry)
{
    if (GS::Gex::Engine::GetInstance().CheckForLicense()!=0)
        return "";

    if((pDatabaseEntry == NULL)
            || (pDatabaseEntry->m_pExternalDatabase == NULL))
        return "";

    QMap< QString,QMap< QString,QStringList > > lIncrementalUpdatesList;
    // Check if have some incremental update to do
    pDatabaseEntry->m_pExternalDatabase->GetNextAutomaticIncrementalUpdatesList(lIncrementalUpdatesList);
    if(lIncrementalUpdatesList.isEmpty())
        return "";

    QString lError;
    foreach(const QString lIncrementalName, lIncrementalUpdatesList.keys())
    {
        foreach(const QString lTestingStage, lIncrementalUpdatesList[lIncrementalName].keys())
        {
            foreach(const QString lTarget, lIncrementalUpdatesList[lIncrementalName][lTestingStage])
            {
                if(!GS::Gex::Engine::GetInstance().GetAdminEngine().AddNewAction("EXECUTION",
                                                                                 "INCREMENTAL_UPDATE",
                                                                                 -1,
                                                                                 pDatabaseEntry->m_nDatabaseId,
                                                                                 "db_update_name="+lIncrementalName
                                                                                 +"|testing_stage="+lTestingStage
                                                                                 +"|target="+lTarget,
                                                                                 lError))
                {
                    // Check why the insert failed
                    GSLOG(SYSLOG_SEV_ALERT, lError.toLatin1().data());
                    return lError;
                }
            }
        }
    }

    return "";
}

QString SchedulerEngine::InsertNewActions(CGexMoTaskItem *ptTask)
{
    if (GS::Gex::Engine::GetInstance().CheckForLicense()!=0)
        return "";

    QString    strError;

    if(ptTask == NULL)
        return "";

    bool        bWithFile = false;
    QString     File;
    QStringList DataFiles;

    switch(ptTask->GetTaskType())
    {
    case GEXMO_TASK_DATAPUMP:
    case GEXMO_TASK_PATPUMP:
    case GEXMO_TASK_TRIGGERPUMP:
    case GEXMO_TASK_CONVERTER:

        bWithFile = true;
        // Get list of files only once
        DataFiles = GS::Gex::Engine::GetInstance().GetDatabaseEngine().GetListOfFiles(
                    ptTask->GetDataFilePath(), ptTask->GetDataFileExtension(),
                    ptTask->IsDataFileScanSubFolder(), ptTask->GetDataFileSort(),
                    ptTask->GetPriority());
        break;

    }

    if(bWithFile)
    {
        // Insert N action for DataPump or Convert tasks
        if(DataFiles.isEmpty())
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("No file found: Action for task %1, database %2...")
                  .arg(QString::number(ptTask->GetID())).arg(QString::number(ptTask->m_iDatabaseId))
                  .toLatin1().data());
        }
        else
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 files found: Action for task %2, database %3...")
                  .arg(DataFiles.count()).arg(QString::number(ptTask->GetID()),QString::number(ptTask->m_iDatabaseId))
                  .toLatin1().data());
        }
        while(!DataFiles.isEmpty())
        {
            File = DataFiles.takeFirst();
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().AddNewAction("EXECUTION",
                                                                             ptTask->GetTypeName(),
                                                                             ptTask->GetID(),
                                                                             ptTask->m_iDatabaseId,
                                                                             "FileName="+File,
                                                                             strError))
            {
                // Check why the insert failed
                GSLOG(SYSLOG_SEV_ALERT, strError.toLatin1().data());
                if(strError.startsWith("Action aborted:"))
                {
                    ptTask->TraceExecution("SCHEDULE NEW ACTION","START","Targeted FileName="+File);
                    GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError("",
                                                                                      QString("[%1] - TASK WAS TEMPORARILY DISABLED. \n%2")
                                                                                      .arg(ptTask->GetName())
                                                                                      .arg(strError));
                    ptTask->TraceExecution("SCHEDULE NEW ACTION","FAIL","Targeted FileName="+File);
                }
                break;
            }
        }
    }
    else
    {
        // Insert only one action
        if(!GS::Gex::Engine::GetInstance().GetAdminEngine().AddNewAction("EXECUTION",
                                                                         ptTask->GetTypeName(),
                                                                         ptTask->GetID(),
                                                                         ptTask->m_iDatabaseId,
                                                                         "",
                                                                         strError))
            GSLOG(SYSLOG_SEV_ALERT, strError.toLatin1().data());
    }

    DataFiles.clear();
    return strError;
}


QString SchedulerEngine::ExecuteTask(CGexMoTaskItem *ptTask, QString DataFile)
{
    if (GS::Gex::Engine::GetInstance().CheckForLicense()!=0)
        return "";

    QString     strErrorTitle, strErrorTask, strError;

    if(ptTask == NULL)
        return "";

    strError = ptTask->Execute(DataFile);

    if(!strError.isEmpty())
    {
        strErrorTitle = ptTask->GetPropertiesTitle();
        strErrorTask = "Reporting";

        AppendMoHistoryLog(QString(),strErrorTask,strErrorTitle);
        AppendMoHistoryLog(strError);

        strError = "error: "+strError;
    }

    emit sDisplayStatusMessage("");

    return strError;
}


///////////////////////////////////////////////////////////
// return false if Scheduler is paused without tasks running
///////////////////////////////////////////////////////////
bool SchedulerEngine::isSchedulerRunning()
{
    if(Engine::GetInstance().HasTasksRunning())
        return true;

    return !isSchedulerStopped();
}

///////////////////////////////////////////////////////////
// return the Status of the Scheduler
///////////////////////////////////////////////////////////
QString SchedulerEngine::getStatus()
{
    QString lStatus = "STARTED";

    if(Engine::GetInstance().HasTasksRunning())
    {
        lStatus = "RUNNING";
        if(isSchedulerStopped())
            lStatus = "STOP_REQUESTED";
    }
    else if(isSchedulerStopped())
        lStatus = "STOPPED";


    return lStatus;
}

///////////////////////////////////////////////////////////
// Stop or Start the Scheduler
// Refresh the GUI
///////////////////////////////////////////////////////////
bool SchedulerEngine::stopScheduler(bool stop)
{
    // Scheduler Engine can be started/stopped only with monitoring products
    if (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        mSchedulerStopped = stop;
        emit sDisplayStatusMessage("");
        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Starting/Stopping scheduler is only allowed for PAT-Man and Yield-Man products");

        return false;
    }
}


///////////////////////////////////////////////////////////
// Execute Alarm Shell...
// return: true on success (no error)
///////////////////////////////////////////////////////////
bool SchedulerEngine::LaunchAlarmShell(int iAlarmType, int iSeverity, int iAlarmCount,
                                       QString product, QString lot, QString sublot, QString wafer,
                                       QString testername, QString operatorname, QString filename,
                                       QString logfilepath
                                       )
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Launch alarm shell AlarmType=%1 Severity=%2 AlarmCount=%3 product='%4'")
          .arg(iAlarmType).arg(iSeverity).arg(iAlarmCount).arg(product)
          .toLatin1().constData() );

    // Get handle to Auto Admin task.
    CGexMoTaskAutoAdmin *ptAutoAdminTask = GetAutoAdminTask();
    // If no such task, then no shell enabled...quietly return
    if(ptAutoAdminTask == NULL)
    {
        GSLOG(5, "Launch alarm shell : no auto admin task found, aborting");
        return true;
    }

    QString lShellFile, lArgumentsLine, lStatusAttribute, lAttribute;

    switch(iAlarmType)
    {
    case ShellYield:       // Yield alarm
        // Arg1: Alarm type
        // For PAT and YIELD the SeverityLevel is -1:PASS, 0:STANDARD, 1:CRITICAL
        lArgumentsLine = "YIELD_ALARM ";
        if (iSeverity == -1) // Pass
        {
            lStatusAttribute = C_ShellYieldPassIsEnabled;
            lAttribute = C_ShellYieldPass;
        }
        else if (iSeverity == 0) // Std
        {
            lStatusAttribute = C_ShellYieldAlarmIsEnabled;
            lAttribute = C_ShellYieldAlarm;
        }
        else if (iSeverity == 1) //Crit
        {
            lStatusAttribute = C_ShellYieldAlarmCriticalIsEnabled;
            lAttribute = C_ShellYieldAlarmCritical;
        }
        else
        {
            GSLOG(SYSLOG_SEV_CRITICAL, "Launch alarm shell : unexpected Severity definition, aborting");
            return false;
        }
        break;

    case ShellSpm:       // SPM alarm
        // Arg1: Alarm type
        // For SYA and SPM the SeverityLevel is 0:PASS, 1:STANDARD, 2:CRITICAL
        lArgumentsLine = "SPM_ALARM ";
        if (iSeverity == 0) // Pass
        {
            lStatusAttribute = C_ShellSpmPassIsEnabled;
            lAttribute = C_ShellSpmPass;
        }
        else if (iSeverity == 1) // Std
        {
            lStatusAttribute = C_ShellSpmAlarmIsEnabled;
            lAttribute = C_ShellSpmAlarm;
        }
        else if (iSeverity == 2) //Crit
        {
            lStatusAttribute = C_ShellSpmAlarmCriticalIsEnabled;
            lAttribute = C_ShellSpmAlarmCritical;
        }
        else
        {
            GSLOG(SYSLOG_SEV_CRITICAL, "Launch alarm shell : unexpected Severity definition, aborting");
            return false;
        }
        break;

    case ShellPat:         // PAT alarm
        // Arg1: Alarm type
        // For PAT and YIELD the SeverityLevel is -1:PASS, 0:STANDARD, 1:CRITICAL
        lArgumentsLine = "PAT_ALARM ";
        if (iSeverity == -1) // Pass
        {
            lStatusAttribute = C_ShellPatPassIsEnabled;
            lAttribute = C_ShellPatPass;
        }
        else if (iSeverity == 0) // Std
        {
            lStatusAttribute = C_ShellPatAlarmIsEnabled;
            lAttribute = C_ShellPatAlarm;
        }
        else if (iSeverity == 1) //Crit
        {
            lStatusAttribute = C_ShellPatAlarmCriticalIsEnabled;
            lAttribute = C_ShellPatAlarmCritical;
        }
        else
        {
            GSLOG(SYSLOG_SEV_CRITICAL, "Launch alarm shell : unexpected Severity definition, aborting");
            return false;
        }
        break;

    case ShellSya:         // SYA alarm
        // Arg1: Alarm type
        // For SYA and SPM the SeverityLevel is 0:PASS, 1:STANDARD, 2:CRITICAL
        lArgumentsLine = "SYA_ALARM ";
        if (iSeverity == 0) // Pass
        {
            lStatusAttribute = C_ShellSyaPassIsEnabled;
            lAttribute = C_ShellSyaPass;
        }
        else if (iSeverity == 1) // Std
        {
            lStatusAttribute = C_ShellSyaAlarmIsEnabled;
            lAttribute = C_ShellSyaAlarm;
        }
        else if (iSeverity == 2)  //Crit
        {
            lStatusAttribute = C_ShellSyaAlarmCriticalIsEnabled;
            lAttribute = C_ShellSyaAlarmCritical;
        }
        else
        {
            GSLOG(SYSLOG_SEV_CRITICAL, "Launch alarm shell : Unexpected Severity definition, aborting");
            return false;
        }
        break;

    default:
        GSLOG(SYSLOG_SEV_ERROR, QString("The shell file '%1' does not exist. Aborting.").
              arg(lShellFile).toLatin1().data() );
        break;
    }

    if(lStatusAttribute.isEmpty())
    {
        GSLOG(SYSLOG_SEV_WARNING, "Launch alarm shell : No shell to execute");
        return true;
    }

    // Script not enabled for that condition
    if (ptAutoAdminTask->GetProperties()->GetAttribute(lStatusAttribute).toBool() == false)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Launch alarm shell : No shell to execute");
        return true;
    }

    lShellFile = ptAutoAdminTask->GetProperties()->GetAttribute(lAttribute).toString();

    if (lShellFile.isEmpty())
    {
        GSLOG(SYSLOG_SEV_WARNING, "Launch alarm shell : No shell to execute");
        return true;
    }

    // If no shell (or shell doesn't exist), simply quietly return.
    if (QFile::exists(lShellFile) == false)
    {
        GSLOG(SYSLOG_SEV_CRITICAL, QString("The shell file '%1' does not exist. Aborting.").
              arg(lShellFile).toLatin1().data() );
        return true;
    }

    // Arg2: Date & Time argument...
    lArgumentsLine += QDateTime::currentDateTime().toString(Qt::ISODate);

    // Arg3: Total alarms...
    lArgumentsLine += " " + QString::number(iAlarmCount);

    // Arg4: ProductID...
    //if(pKeyContent->strProductID.isEmpty())
    if (product.isEmpty())
        lArgumentsLine += " ?";
    else
        lArgumentsLine += " " +product; //pKeyContent->strProductID;

    // Arg5: LotID...
    if (lot.isEmpty()) //if(pKeyContent->strLot.isEmpty())
        lArgumentsLine += " ?";
    else
        lArgumentsLine += " " + lot; //pKeyContent->strLot;

    // Arg6: SubLotID...
    if (sublot.isEmpty()) //(pKeyContent->strSubLot.isEmpty())
        lArgumentsLine += " ?";
    else
        lArgumentsLine += " " + sublot; //pKeyContent->strSubLot;

    // Arg7: WaferID...
    if (wafer.isEmpty()) //if(pKeyContent->strWaferID.isEmpty())
        lArgumentsLine += " ?";
    else
        lArgumentsLine += " " + wafer; //pKeyContent->strWaferID;

    // Arg8: Tester name...
    if (testername.isEmpty()) //(pKeyContent->strTesterName.isEmpty())
        lArgumentsLine += " ?";
    else
        lArgumentsLine += " " + testername; //pKeyContent->strTesterName;

    // Arg9: Operator...
    if (operatorname.isEmpty()) //(pKeyContent->strOperator.isEmpty())
        lArgumentsLine += " ?";
    else
        lArgumentsLine += " " + operatorname; //pKeyContent->strOperator;

    // Arg10: Data file name...
    if (filename.isEmpty()) //(pKeyContent->strFileName.isEmpty())
        lArgumentsLine += " ?";
    else
        lArgumentsLine += " " + filename; //pKeyContent->strFileName;

    // Arg11 : logfile if any
    lArgumentsLine += " " + (logfilepath.isEmpty()?"? ":logfilepath);


    GSLOG(SYSLOG_SEV_DEBUG, QString("Launch alarm shell: %1 %2").arg(lShellFile).arg(lArgumentsLine).toLatin1().data() );

    // Launch Shell command (minimized)
#ifdef _WIN32
    // Replace '/' to '\' to avoid MS-DOS compatibility issues
    lArgumentsLine = lArgumentsLine.replace('/','\\');

    ShellExecuteA(NULL,
                  "open",
                  lShellFile.toLatin1().constData(),
                  lArgumentsLine.toLatin1().constData(),
                  NULL,
                  SW_SHOWMINIMIZED);
#else
    lShellFile = lShellFile + " " + lArgumentsLine;
    if (system(lShellFile.toLatin1().constData()) == -1) {
        //FIXME: send error
    }
#endif

    // Success
    return true;
}

void SchedulerEngine::setLastTaskId(int iId)
{
    mLastTaskIdReceived   = iId;
    mTaskIdReceived       = true;
}

void SchedulerEngine::sendInsertionLogsMail(GexMoAutoAdminTaskData* ptAutoAdminTask, QDate dLastLog)
{
    QString strFrom,strTo,strSubject,strEmailBody,strInsertionReport,strHistoryLogFile,strReportLogFile;
    QMap<QString, QMap<QString, QMap<QString, int>*>*> mapDataPump;
    QMap<QString, int> mapDatapumpStatus;
    QStringList lstAttachments,lstStatusInFile;
    int iLineLength = 0;
    int iMaxDataPumpNameLength = 0;

    if(Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        // Create the query
        QString     lQueryString;
        QSqlQuery   lQuery(QSqlDatabase::database(Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
        QDateTime   lBeginTime(QDate::currentDate().addDays(-1), QTime(0,0));
        QDateTime   lEndTime(QDate::currentDate().addDays(-1), QTime(23,59,59,99));
        QMap<int, QString>  lTasksName;

        // Then extract the DATAPUMP Task Name
        lQueryString = "SELECT task_id,  name ";
        lQueryString += " FROM ym_tasks WHERE ";
        lQueryString += " type="+QString::number(GEXMO_TASK_DATAPUMP);
        if (lQuery.exec(lQueryString) == false)
        {
            GSLOG(SYSLOG_SEV_WARNING, "Failed to retrieve reports log from ym_admindb");
            return;
        }
        while(lQuery.next())
        {
            lTasksName[lQuery.value("task_id").toInt()] = lQuery.value("name").toString();
        }

        // Extract the summary report
        // To avoid Gap Lock
        // Use the PRIMARY KEY
        // And to limited the execution time
        // and do not JOIN with the ym_tasks
        lQueryString = "SELECT status, task_id, summary ";
        lQueryString += " FROM ym_events ";
        lQueryString += " WHERE type = 'DATAPUMP' AND category = 'EXECUTION' ";
        lQueryString += " AND status IN ('PASS', 'FAIL', 'DELAY') AND ";
        lQueryString += " start_time BETWEEN ";
        lQueryString += Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(lBeginTime);
        lQueryString += " AND ";
        lQueryString += Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(lEndTime);
        // Parse query results
        if (lQuery.exec(lQueryString) == false)
        {
            GSLOG(SYSLOG_SEV_WARNING, "Failed to retrieve reports log from ym_admindb");
            return;
        }
        while(lQuery.next())
        {
            int         lTaskId     = lQuery.value("task_id").toInt();
            QString     lStatus     = lQuery.value("status").toString();
            QStringList lSummary    = lQuery.value("summary").toString().split("|", QString::SkipEmptyParts);
            QString     lDataPump   = lTasksName[lTaskId];
            QString     lErrorCode;
            QString     lErrorDesc;
            QString     lMsgLog;

            // Find the error code field if any
            int lErrorIdx = lSummary.indexOf(QRegExp("errorcode=.*", Qt::CaseInsensitive));

            // If found, extract the error code
            if (lErrorIdx > 0)
                lErrorCode = lSummary.at(lErrorIdx).section("=", 1);

            // Find the error description field if any
            int lErrorDescIdx = lSummary.indexOf(QRegExp("errorshortdesc=.*", Qt::CaseInsensitive));

            // If found, extract the error desc
            if (lErrorDescIdx > 0)
                lErrorDesc = lSummary.at(lErrorDescIdx).section("=", 1);

            // Concatenate Error code and description
            if (lErrorCode.isEmpty() == false)
                lMsgLog = lErrorCode + ": " + lErrorDesc;
            else
                lMsgLog = lErrorDesc;

            // Get datapump name max length
            if (lDataPump.length() > iMaxDataPumpNameLength)
                iMaxDataPumpNameLength = lDataPump.length();

            // Get differents status in file (for the 1st summary)
            if (!lstStatusInFile.contains(lStatus))
                lstStatusInFile << lStatus;

            if (!mapDataPump.contains(lDataPump))
                mapDataPump.insert(lDataPump, new QMap<QString, QMap<QString, int>*>);

            // For the second part of summary
            if (!mapDataPump[lDataPump]->contains(lStatus))
                mapDataPump[lDataPump]->insert(lStatus, new QMap<QString, int>);

            // If status exists -> increment
            if (mapDataPump.value(lDataPump)->value(lStatus)->contains(lMsgLog))
                mapDataPump.value(lDataPump)->value(lStatus)->insert(lMsgLog, mapDataPump.value(lDataPump)->value(lStatus)->value(lMsgLog) + 1);
            // If status doesn't create it, set to value 1
            else
                mapDataPump.value(lDataPump)->value(lStatus)->insert(lMsgLog, 1);
        }
    }
    else
    {
        // Build Yield-Man history log file name
        // GCORE-2901
        // HTH: Use the new monitoring folder
        strHistoryLogFile = Get(SchedulerEngine::sMonitoringLogsFolderKey).toString();
        strHistoryLogFile += GEXMO_LOG_FILE_ROOT;
        strHistoryLogFile += "_";
        strHistoryLogFile += dLastLog.toString(Qt::ISODate);
        strHistoryLogFile += ".log";
        CGexSystemUtils::NormalizePath(strHistoryLogFile);

        // Build Yield-Man report file and view file name
        // GCORE-2901
        // HTH: Use the new monitoring folder
        strReportLogFile = Get(SchedulerEngine::sMonitoringLogsFolderKey).toString();
        strReportLogFile += GEXMO_REPORT_FILE_ROOT;
        strReportLogFile += "_";
        strReportLogFile += dLastLog.toString(Qt::ISODate);
        strReportLogFile += ".log";
        CGexSystemUtils::NormalizePath(strReportLogFile);

        // Read report file to build a summary
        QFile file(strReportLogFile);
        if (file.exists())
        {
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            QTextStream streamReader(&file);
            QString line;
            streamReader.readLine();          // Pass header line
            while(! streamReader.atEnd())
            {
                line = streamReader.readLine();
                if (!line.isEmpty())
                {
                    QString strDataPump = line.section(",", 5, 5);
                    QString strStatus = line.section(",", 2, 2);
                    QString strMsgLog = line.section(",", 4, 4);
                    // Get datapump name max length
                    if (strDataPump.length() > iMaxDataPumpNameLength)
                        iMaxDataPumpNameLength = strDataPump.length();

                    // Get differents status in file (for the 1st summary)
                    if (!lstStatusInFile.contains(strStatus))
                        lstStatusInFile << strStatus;

                    if (!mapDataPump.contains(strDataPump))
                        mapDataPump.insert(strDataPump, new QMap<QString, QMap<QString, int>*>);

                    // For the second part of summary
                    if (!mapDataPump[strDataPump]->contains(strStatus))
                        mapDataPump[strDataPump]->insert(strStatus, new QMap<QString, int>);

                    // If status exists -> increment
                    if (mapDataPump.value(strDataPump)->value(strStatus)->contains(strMsgLog))
                        mapDataPump.value(strDataPump)->value(strStatus)->insert(strMsgLog, mapDataPump.value(strDataPump)->value(strStatus)->value(strMsgLog) + 1);
                    // If status doesn't create it, set to value 1
                    else
                        mapDataPump.value(strDataPump)->value(strStatus)->insert(strMsgLog, 1);
                }
            }
            file.close();
            lstAttachments << strReportLogFile;

        }
    }

    if (lstStatusInFile.isEmpty() == false)
        lstStatusInFile.sort();

    if (iMaxDataPumpNameLength < QString("DATAPUMP \\ File").length())
        iMaxDataPumpNameLength = QString("DATAPUMP \\ File").length();

    // Length of the first column with the Datapump name
    iMaxDataPumpNameLength++;

    // Get Email data
    strFrom = ptAutoAdminTask->mEmailFrom;
    strTo = ptAutoAdminTask->mEmailNotify;
    strSubject = ptAutoAdminTask->mTitle + QString(":  Insertion summary (") + dLastLog.toString("MMMM, d yyyy") + ")";

    // Build email body
    strEmailBody= "Yield-Man insertion summary\n";
    strEmailBody+= "###########################\n";
    strEmailBody+= QString("Date: ") + dLastLog.toString("MMMM, d yyyy");
    strEmailBody+= "\n\n";
    strInsertionReport = "";

    // Build the table header
    if (!lstStatusInFile.isEmpty())
    {
        iLineLength = 2 + iMaxDataPumpNameLength + QString("| " + lstStatusInFile.join(" | ") + " |").length();
        strEmailBody += QString("").leftJustified(iLineLength , '-') + "\n";


        strEmailBody += "| " + QString("DATAPUMP \\ File").leftJustified(iMaxDataPumpNameLength, ' ') + "| " + lstStatusInFile.join(" | ").toUpper() + " |\n";
        strEmailBody += QString("").leftJustified(iLineLength , '-') + "\n";
    }

    if (QFile::exists(strHistoryLogFile))
        lstAttachments << strHistoryLogFile;

    // Write one summary by datapump
    QMapIterator<QString, QMap<QString, QMap<QString,int>*>*> itDataPump(mapDataPump);
    while (itDataPump.hasNext())
    {
        itDataPump.next();
        mapDatapumpStatus.clear();
        // Write datapump name in the table
        strEmailBody += "| " + itDataPump.key().leftJustified(iMaxDataPumpNameLength,' ') + "|";

        strInsertionReport += "------------------------------------------------------------\n";
        strInsertionReport += QString("DATAPUMP: ") + itDataPump.key() + "\n";
        strInsertionReport += "------------------------------------------------------------\n";

        // For each status of the datapump
        QMapIterator<QString, QMap<QString, int>*> itStatus(*(itDataPump.value()));
        while (itStatus.hasNext())
        {
            itStatus.next();
            QMapIterator<QString, int> itMsgLogPrePro(*(itStatus.value()));
            int iMaxMsgLength = 0;
            // Preprocessing get key values (max msg length, number of status...)
            while (itMsgLogPrePro.hasNext())
            {
                itMsgLogPrePro.next();
                if (itMsgLogPrePro.key().trimmed().length() > iMaxMsgLength )
                    iMaxMsgLength = itMsgLogPrePro.key().trimmed().length();
                if (mapDatapumpStatus.contains(itStatus.key()))
                    mapDatapumpStatus[itStatus.key()] = mapDatapumpStatus[itStatus.key()] + itMsgLogPrePro.value();
                else
                    mapDatapumpStatus.insert(itStatus.key(), itMsgLogPrePro.value());
            }
            iMaxMsgLength++; // To have breathy presentation

            // Write status
            strInsertionReport += itStatus.key().leftJustified(10, ' ') + "(" + QString::number(mapDatapumpStatus[itStatus.key()]) + ")" + "\n";

            // Write all counted msg for this status
            QMapIterator<QString, int> itMsgLog(*(itStatus.value()));
            while (itMsgLog.hasNext())
            {
                itMsgLog.next();
                if (!itMsgLog.key().trimmed().isEmpty())
                {
                    strInsertionReport += "- " + itMsgLog.key().leftJustified(iMaxMsgLength, ' ') + "(" + QString::number(itMsgLog.value()) + ")\n";
                }
            }
            strInsertionReport += "\n";
        }

        // Write summary table status counted
        for (int i = 0; i < lstStatusInFile.size(); i++)
        {
            int iNameLength = lstStatusInFile.at(i).length() + 1;
            int iNbStatus = 0;
            if (mapDatapumpStatus.contains(lstStatusInFile.at(i)))
                iNbStatus = mapDatapumpStatus[lstStatusInFile.at(i)];
            strEmailBody += QString::number(iNbStatus).rightJustified(iNameLength, ' ') + " |";
        }
        strEmailBody += "\n";
        strInsertionReport += "\n";
    }
    strInsertionReport += "\n";
    if (lstStatusInFile.size() > 0)
        strEmailBody += QString("").leftJustified(iLineLength , '-') + "\n\n\n";


    // Add the developped summary after the built table
    strEmailBody += strInsertionReport;

    if (mapDataPump.size() == 0)
        strEmailBody += QString("No insertions processed!\n\n");

    if (lstAttachments.size() > 0)
        strEmailBody += QString("Please find attached the insertion log files for ") + dLastLog.toString("MMMM, d yyyy") + QString(".\n");

    // Send email
    GexMoSendEmail Email;
    QString strFilePath;
    CGexMoTaskStatus *ptStatusTask = GetStatusTask();
    if(ptStatusTask != NULL)
    {
        // We have a spolling folder: create email file in it!
        strFilePath = ptStatusTask->GetProperties()->intranetPath() + "/";
        strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
        strFilePath += GEXMO_AUTOREPORT_EMAILS;
        CGexSystemUtils::NormalizePath(strFilePath);
    }

    QStringList::iterator it;
    for (it = lstAttachments.begin(); it != lstAttachments.end(); ++it)
    {
        QFileInfo fileInfo(*it);
        QString strNewPath = strFilePath + QString("/") + fileInfo.fileName();
        CGexSystemUtils::NormalizePath(strNewPath);
        QFile::copy((*it), strNewPath);
        *it = strNewPath;
    }

    Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody,false , lstAttachments.join(";"));

    ptAutoAdminTask->mLastInsertionLogsSent = QDate::currentDate();
}

QString SchedulerEngine::SendEmail(QString strFrom, QString strTo,
                                   QString strSubject, QString strBody,
                                   bool bHtmlFormat, QString strAttachments)
{
    GSLOG(5, QString("Send email from %1 to %2 : %3 %4 %5 %6")
          .arg(strFrom).arg(strTo).arg(strSubject).arg(strBody.leftRef(6).toString())
          .arg(bHtmlFormat?"HTML":"TXT").arg(strAttachments).toLatin1().data()  );

    CGexMoTaskStatus *ptStatusTask = GetStatusTask();
    if(ptStatusTask == NULL)
    {
        GSLOG(3, "Cannot find StatusTask to send emails.");
        return "error : cannot find StatusTask.";
    }

    if (strTo.isEmpty())
    {
        GSLOG(4, "To email parameter is empty. Let's searching for emails according to current Product/Lot/...");
        //UpdateGexScriptEngine(ReportOptions); // Do not reset Script engine in order to keep current product/lot/sublot

        if (!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        {
            GSLOG(3,
                  "The feature to retrieve list of users/emails according to context (Product/Lot/...) is only available through Admin DB which is NOT activated.");
        }
        else
        {
            AdminUser* u=0;
            foreach(u, GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.values())
            {
                if (!u)
                    continue;
                if (u->m_strEmail.isEmpty())
                    continue;

                QString track=u->GetAttribute("TrackWhen").toString();
                //track="ProductID=='FrTB9324FGXXXR20'";
                if ( !track.isEmpty() )
                {
                    QScriptValue sv=pGexScriptEngine->evaluate(track);
                    if ( sv.isError() || pGexScriptEngine->hasUncaughtException() )
                    {
                        QString m=QString("Script evaluation : '%1'")
                                .arg(pGexScriptEngine->uncaughtException().toString());
                        GSLOG(4, m.toLatin1().data());
                    }
                    else
                        GSLOG(5, QString("user %1 will %2 receive the email (%3) : tracking conditions: %4")
                              .arg(u->m_strLogin).arg(sv.toBool()?"":"NOT")
                              .arg(sv.toString()).arg(track).toLatin1().data() );
                    if (sv.toBool())
                        strTo.append(u->m_strEmail+";");
                    // GCORE-499
                    pGexScriptEngine->collectGarbage();
                }
                else
                    strTo.append(u->m_strEmail+";");
            }
        }
    }

    // We have a spooling folder: create email file in it!
    QString strFilePath = ptStatusTask->GetProperties()->intranetPath() + "/";
    strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
    strFilePath += GEXMO_AUTOREPORT_EMAILS;
    CGexSystemUtils::NormalizePath(strFilePath);

    GexMoSendEmail Email;
    bool b=Email.Send(strFilePath, strFrom,strTo, strSubject, strBody, bHtmlFormat, strAttachments );
    QString r=b?"ok":"error : failed to generate email";
    return r;
}

///////////////////////////////////////////////////////////
// Load Tasks from YieldManDb
///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void SchedulerEngine::ReloadDbTaskIfUpdated(CGexMoTaskItem *ptTask)
{
    if(mTaskBeingEdited)
        return;

    if(ptTask == NULL)
        return;

    if(ptTask->IsLocal())
        return;

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            &&  !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    LoadUploadedTasks(ptTask);
}

void SchedulerEngine::CheckTaskStatus(CGexMoTaskItem *ptTask, QString Options)
{
    if(ptTask == NULL)
    {
        return;
    }

    bool    bCheckTask = ptTask->GetEnabledState();
    bool    bCheckFolderOption = Options.contains("Folder",Qt::CaseInsensitive);
    bool    bCheckDatabaseOption = Options.contains("DatabaseStatus",Qt::CaseInsensitive);

    if(Options.contains("All",Qt::CaseInsensitive))
    {
        bCheckTask = bCheckFolderOption = bCheckDatabaseOption = true;
    }

    int     nOldStatus = ptTask->m_iStatus;
    QString strOldInfoMsg = ptTask->m_strLastInfoMsg;

    if((ptTask->GetTaskType() != GEXMO_TASK_OLD_DATAPUMP)
            && !bCheckTask){
        if((nOldStatus != ptTask->m_iStatus) || (strOldInfoMsg != ptTask->m_strLastInfoMsg))
            ptTask->m_LastStatusUpdate = GS::Gex::Engine::GetInstance().GetServerDateTime();
        return;
    }

    ptTask->CheckTaskStatus(bCheckDatabaseOption, bCheckFolderOption);

    if((nOldStatus != ptTask->m_iStatus) || (strOldInfoMsg != ptTask->m_strLastInfoMsg))
    {
        TimersTraceStart("Checking tasks - Update logs");
        ptTask->m_LastStatusUpdate = GS::Gex::Engine::GetInstance().GetServerDateTime();
        ptTask->TraceInfo();
        TimersTraceStop("Checking tasks - Update logs");
    }
}

QString SchedulerEngine::ProcessFileAfterExecution(int TaskStatus, int TaskId, QString FileName, QString &FileDest)
{
    CGexMoTaskItem *Task = FindTaskInList(TaskId);
    if(Task == NULL)
        return "Task not defined";

    QString FullFileName = FileName;
    if(Task->GetDataPumpData() && QFileInfo(FullFileName).isRelative())
        FullFileName = QFileInfo(Task->GetDataPumpData()->strDataPath, FileName).absoluteFilePath();

    return ProcessFileAfterExecution(TaskStatus, Task, FullFileName, FileDest);
}

QString SchedulerEngine::ProcessFileAfterExecution(int TaskStatus, CGexMoTaskItem *Task, QString FullFileName, QString &FileDest)
{
    if(Task == NULL)
        return "Task not defined";

    QString strError;
    GS::QtLib::DatakeysContent KeyContent(NULL, true);
    KeyContent.SetInternal("FullDestinationName", FullFileName);

    GexMoDataPumpTaskData* dpData = Task->GetDataPumpData();
    if(dpData)
    {
        if(dpData->iPostImport == GEXMO_POSTIMPORT_FTP)
        {
            KeyContent.SetInternal("UploadFile","TRUE");
            KeyContent.Set("MovePath",dpData->strPostImportMoveFolder);
        }
        else if(dpData->iPostImport == GEXMO_POSTIMPORT_MOVE)
        {
            KeyContent.SetInternal("MoveFile","TRUE");
            KeyContent.Set("MovePath", dpData->strPostImportMoveFolder);
        }
    }

    strError = ProcessFileAfterExecution(TaskStatus, Task, KeyContent);
    FileDest = KeyContent.Get("FullDestinationName").toString();

    return strError;
}



QString   SchedulerEngine::ProcessFileAfterExecution(int TaskStatus, CGexMoTaskItem *Task, QtLib::DatakeysContent &KeysContent)
{
    if(Task == NULL)
        return "Task not defined";

    QString Error;
    QString FileDest, FileExt;
    QString FullFileName = KeysContent.Get("FullDestinationName").toString();

    QFileInfo FileInfo(FullFileName);
    QString FileName = FileInfo.fileName();
    QDir    Dir(FileInfo.absoluteFilePath());

    bool bDeleteFile = false;
    bool bRenameFile = false;
    QString strQuarantineMail;

    GexMoDataPumpTaskData* dpData = Task->GetDataPumpData();
    if(dpData)
    {

        switch(TaskStatus)
        {
        case -1:
            // Quarantine after a Gex crash
            // Caller is Monitoring: see what action to be done after failed insertion
            // *.quarantine         => rename datapump/file.stdf to datapump/file.stdf.quarantine
            // /quarantineFolder/*  => move datapump/file.stdf to datapump/quarantineFolder/file.stdf
            // C:/qurantineFolder/* => move datapump/file.stdf to C:/quarantineFolder/file.stdf
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
                GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeSettingsValue("LOADBALANCING_QUARANTINE_HOUSEKEEPING_OPTION",
                                                                                     FileDest);
            // Check if have an option defined
            // else use the default
            if(FileDest.isEmpty())
                FileDest = "*.quarantine";

            if(FileDest.startsWith("*."))
            {
                FileExt = FileDest.section("*.",1).trimmed();
                FileDest = FullFileName + "." + FileExt;
                bRenameFile = true;
            }
            else
            {
                // Move the *
                if(FileDest.endsWith("*"))
                    FileDest = FileDest.left(FileDest.length()-1);

                // Build destination name
                FileInfo.setFile(FullFileName);
                if( (FileDest.startsWith("/") || FileDest.startsWith("\\"))
                        && !(FileDest.startsWith("//") || FileDest.startsWith("\\\\")))
                    FileDest = FileInfo.absoluteFilePath() + FileDest;

                if( (FileDest.endsWith("/") || FileDest.endsWith("\\")) == false)
                    FileDest += "/";

                FileDest += FileName;
                bRenameFile = true;
            }
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
                GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeSettingsValue("LOADBALANCING_QUARANTINE_EMAIL_OPTION",
                                                                                     strQuarantineMail);
            break;
        case DatabaseEngine::Failed:
        case DatabaseEngine::FailedValidationStep:
            // Caller is Monitoring: see what action to be done after failed insertion
            switch(dpData->iPostImportFailure)
            {
            case GEXMO_BAD_POSTIMPORT_RENAME:	// Rename BAD file to ".bad".
                FileExt = "bad";
                FileDest = FullFileName + "." + FileExt;
                bRenameFile = true;

                break;

            case GEXMO_BAD_POSTIMPORT_MOVE:
                // Build destination name
                FileInfo.setFile(FullFileName);
                FileDest = dpData->strPostImportFailureMoveFolder.trimmed();

                // MovePath can be updated to overwrite the FailureMovePath
                // If MovePath diff than pass MoveFolder, use it for failure
                if(dpData->strPostImportMoveFolder
                        != KeysContent.Get("MovePath").toString())
                {
                    FileDest = KeysContent.Get("MovePath").toString().trimmed();

                    if((Dir.exists(FileDest) == false) && (Dir.mkpath(FileDest) == false))
                        FileDest = dpData->strPostImportFailureMoveFolder.trimmed();
                    FileDest = QDir::toNativeSeparators(QDir::cleanPath(FileDest));
                }

                if( (FileDest.endsWith("/") || FileDest.endsWith("\\")) == false)
                    FileDest += "/";
                FileDest += FileName;
                bRenameFile = true;
                break;
            }
            break;
        case DatabaseEngine::Passed:
            // Caller is Monitoring: see what action to be done after successful insertion
            switch(dpData->iPostImport)
            {
            case GEXMO_POSTIMPORT_RENAME:	// Rename file to ".read".
                FileExt = "read";
                FileDest = FullFileName + "." + FileExt;
                bRenameFile = true;
                break;

            case GEXMO_POSTIMPORT_DELETE:	// Delete files after import
                FileDest = FullFileName;
                bDeleteFile = true;
                break;

            case GEXMO_POSTIMPORT_FTP:		// FTP file (in fact, move file to other folder + create trigger file for side FTP application)
                // Build destination name (in all cases: move or FTP, we need to move the file)
                FileInfo.setFile(FullFileName);
                FileDest = dpData->strPostImportMoveFolder.trimmed();
                if((FileDest.endsWith("/") || FileDest.endsWith("\\")) == false)
                    FileDest += "/";
                FileDest += FileName;
                bRenameFile = true;

                // Create trigger file for FTP task.
                if(WriteFtpTriggerFile(FileDest, KeysContent))
                {
                    // Failed creating FTP trigger event file
                    Error = "Failed creating FTP trigger event for file:" + FileDest;
                }
                break;

            case GEXMO_POSTIMPORT_MOVE:		// Move file to custom folder
                // Build destination name
                FileInfo.setFile(FullFileName);
                FileDest = KeysContent.Get("MovePath").toString().trimmed();

                FileDest = QDir::toNativeSeparators(QDir::cleanPath(FileDest));
                if((Dir.exists(FileDest) == false) && (Dir.mkpath(FileDest) == false))
                    FileDest = dpData->strPostImportMoveFolder.trimmed();

                if((FileDest.endsWith("/") || FileDest.endsWith("\\")) == false)
                    FileDest += "/";
                FileDest += FileName;
                bRenameFile = true;
                break;
            }
            break; // case PassedInsertion:

        case DatabaseEngine::Delay:	// File failed insertion for extrenal reasons (network busy, etc). So try again later!
            // Caller is Monitoring: see what action to be done after delayed insertion
            switch(dpData->iPostImportDelay)
            {
            case GEXMO_DELAY_POSTIMPORT_RENAME:	// Rename DELAY file to ".delay".
                FileExt = "delay";
                FileDest = FullFileName + "." + FileExt;
                bRenameFile = true;
                break;

            case GEXMO_DELAY_POSTIMPORT_MOVE:
                // Build destination name
                FileInfo.setFile(FullFileName);
                FileDest = dpData->strPostImportDelayMoveFolder.trimmed();

                // MovePath can be updated to overwrite the FailureMovePath
                // If MovePath diff than pass MoveFolder, use it for failure
                if(dpData->strPostImportMoveFolder
                        != KeysContent.Get("MovePath").toString())
                {
                    FileDest = KeysContent.Get("MovePath").toString().trimmed();

                    if((Dir.exists(FileDest) == false) && (Dir.mkpath(FileDest) == false))
                        FileDest = dpData->strPostImportDelayMoveFolder.trimmed();
                    FileDest = QDir::toNativeSeparators(QDir::cleanPath(FileDest));
                }

                if( (FileDest.endsWith("/") || FileDest.endsWith("\\")) == false)
                    FileDest += "/";
                FileDest += FileName;
                bRenameFile = true;
                break;

            default:
                // Leave file
                break;
            }
        }
    }

    GexMoFileConverterTaskData* convData = Task->GetConverterData();
    if(convData)
    {
        if(TaskStatus == -1)
        {
            // Quarantine after a Gex crash
            // Caller is Monitoring: see what action to be done after failed insertion
            // *.quarantine         => rename datapump/file.stdf to datapump/file.stdf.quarantine
            // /quarantineFolder/*  => move datapump/file.stdf to datapump/quarantineFolder/file.stdf
            // C:/quarantineFolder/* => move datapump/file.stdf to C:/quarantineFolder/file.stdf
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
                GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeSettingsValue("LOADBALANCING_QUARANTINE_HOUSEKEEPING_OPTION",
                                                                                     FileDest);
            // Check if have an option defined
            // else use the default
            if(FileDest.isEmpty())
                FileDest = "*.quarantine";

            if(FileDest.startsWith("*."))
            {
                FileExt = FileDest.section("*.",1).trimmed();
                FileDest = FullFileName + "." + FileExt;
                bRenameFile = true;
            }
            else
            {
                // Move the *
                if(FileDest.endsWith("*"))
                    FileDest = FileDest.left(FileDest.length()-1);

                // Build destination name
                FileInfo.setFile(FullFileName);
                if( (FileDest.startsWith("/") || FileDest.startsWith("\\"))
                        && !(FileDest.startsWith("//") || FileDest.startsWith("\\\\")))
                    FileDest = FileInfo.absoluteFilePath() + FileDest;

                if( (FileDest.endsWith("/") || FileDest.endsWith("\\")) == false)
                    FileDest += "/";

                FileDest += FileName;
                bRenameFile = true;
            }
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
                GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeSettingsValue("LOADBALANCING_QUARANTINE_EMAIL_OPTION",
                                                                                     strQuarantineMail);
        }
        else if(TaskStatus == ConvertToSTDF::eConvertSuccess)
        {
            // Check if replication required (if so, copy input file to specified folder)
            char *ptChar = getenv("GEX_SERVER_INPUT_REPLICATION_FOLDER");
            if(ptChar != NULL)
            {
                // Copy file for replication
                QDir		Dir;
                FileDest = ptChar;
                FileDest += "/" + FileInfo.dir().dirName();
                if(!Dir.exists(FileDest))
                    Dir.mkpath(FileDest);
                FileDest += "/" + FileName;
                ut_CopyFile(FullFileName.toLatin1().constData(), FileDest.toLatin1().constData(), UT_COPY_USETEMPNAME);
            }

            // Successful conversion
            switch(convData->iOnSuccess)
            {
            case GEXMO_TASK_CONVERTER_SUCCESS_RENAME:
                // Rename to .read
                FileExt = "read";
                FileDest = FullFileName + "." + FileExt;
                bRenameFile = true;
                break;

            case GEXMO_TASK_CONVERTER_SUCCESS_DELETE:
                // Check for error
                bDeleteFile = true;
                break;

            case GEXMO_TASK_CONVERTER_SUCCESS_MOVE:
                FileDest = convData->strOutputSuccess.trimmed() + "/" + FileName;
                bRenameFile = true;
                break;
            }
        }
        else if(TaskStatus == ConvertToSTDF::eConvertError)
        {
            // Conversion failed
            switch(convData->iOnError)
            {
            case GEXMO_TASK_CONVERTER_FAIL_RENAME:
                // Rename to .read
                FileExt = "bad";
                FileDest = FullFileName + ".bad";
                bRenameFile = true;
                break;
            case GEXMO_TASK_CONVERTER_FAIL_MOVE:
                FileDest = convData->strOutputError.trimmed() + "/" + FileName;
                bRenameFile = true;
                break;
            }
        }
    }

    // Initialize dbKeys engine
    GS::QtLib::DatakeysEngine dbKeysEngine(KeysContent);

    if(bRenameFile)
    {
        //Check if the destination file is the same than source file.
        if(QFileInfo(FileDest).absoluteFilePath() != QFileInfo(FullFileName).absoluteFilePath() )
        {
            if(!toolFileMove(FullFileName,FileDest))
            {
                // Failed moving file
                if(FileExt.isEmpty())
                    Error = "Failed moving file";
                else
                    Error = "Failed renaming file";
                Error += "\nFrom:" + FullFileName;
                Error += "\nTo:" + FileDest;
            }
            else
            {
                KeysContent.SetInternal("FullDestinationName",FileDest);
            }
        }
        // Move config keys file
        if(FileExt.isEmpty())
            dbKeysEngine.moveConfigDbKeysFile(QFileInfo(FileDest).absolutePath());
        else
            dbKeysEngine.renameConfigDbKeysFile(FileExt);
    }

    if(bDeleteFile)
    {
        if(!toolFileMove(FullFileName,FullFileName,true,false))
        {
            Error = "Failed to delete file: " + FullFileName;
        }
        dbKeysEngine.deleteConfigDbKeysFile();
    }

    if(!strQuarantineMail.isEmpty())
    {
        if((strQuarantineMail.toUpper() == "OWNER")
                || (strQuarantineMail.toUpper() == "TASK_OWNER"))
        {
            // Get the owner email
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.contains(Task->m_iUserId))
                strQuarantineMail = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[Task->m_iUserId]->m_strEmail;
            else
                strQuarantineMail = "";
        }
        if(strQuarantineMail.isEmpty()
                || (strQuarantineMail == "1")
                || (strQuarantineMail.toUpper() == "YES")
                || (strQuarantineMail.toUpper() == "true"))
        {
            // Get the admin email
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.contains(1))
                strQuarantineMail = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[1]->m_strEmail;
            else
                strQuarantineMail = "";
        }

        if(!strQuarantineMail.isEmpty())
        {
            // Have to send an email
            QString strFrom = "no-reply@mentor.com";
            QString strBody = "Quarantine alarm:\n";
            strBody+= "Task\t: "+Task->m_strName+"\n";
            strBody+= "File\t: "+FullFileName+"\n";
            strBody+= "Dest\t: "+FileDest+"\n";

            SendEmail(strFrom,strQuarantineMail,"Quarantine alarm",strBody,false,"");

        }
    }

    return Error;

}

void SchedulerEngine::ExtractStringValue(QString strValue,double &lfValue,double lfInvalid)
{
    char szString[100];

    if(strValue.isEmpty())
    {
        // Empty string...use 'invalid'/'unknown' value
        lfValue = lfInvalid;
        return;
    }

    int iArgs = sscanf(strValue.toLatin1().constData(),"%lf%s",&lfValue,szString);
    switch(iArgs)
    {
    case 0:	// Empty field
        lfValue = lfInvalid;
        break;

    case 1:	// No units specified...assume value as a normalized value.
        break;

    case 2:	// units specified...scale value accordingly
        // Ignore units if we have not at least 2 characters (because a prefix + units always uses 2 chars. or more!).
        if(strlen(szString) <= 1)
            break;

        switch(*szString)
        {
        // List of units we accept to scale to m, u, p, f, etc...
        case 'm':	// Milli...
            lfValue /= 1e3;
            break;

        case 'u':	// Micro...
            lfValue /= 1e6;
            break;

        case 'n':	// Milli...
            lfValue /= 1e9;
            break;

        case 'p':	// Pico...
            lfValue /= 1e12;
            break;

        case 'f':	// Fento...
            lfValue /= 1e15;
            break;

        case 'K':	// Kilo...
            lfValue *= 1e3;
            break;

        case 'M':	// Mega...
            lfValue *= 1e6;
            break;

        case 'G':	// Giga...
            lfValue *= 1e9;
            break;

        case 'T':	// Tera...
            lfValue *= 1e12;
            break;
        }
    }
}



}
}
