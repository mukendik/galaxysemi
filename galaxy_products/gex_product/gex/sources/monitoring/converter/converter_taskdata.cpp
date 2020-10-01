//#include "QApplication"
//#include <QFileDialog>

// Galaxy modules includes
#include <gstdl_utils_c.h>
#include <gqtl_log.h>

// Local includes
#include "converter_task.h"
#include "converter_taskdata.h"
#include "scheduler_engine.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "gex_report.h"
#include "db_engine.h"
#include "export_csv.h"
#include "export_atdf.h"
#include "gexmo_constants.h"
#include "mo_task.h"
#include "engine.h"
#include "admin_engine.h"
#include "message.h"

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

// main.cpp
extern void				WriteDebugMessageFile(const QString & strMessage);

// in report_build.cpp
extern CGexReport*		gexReport;				// Handle to report class

///////////////////////////////////////////////////////////
// Structure constructor
///////////////////////////////////////////////////////////
GexMoFileConverterTaskData::GexMoFileConverterTaskData(QObject* parent): TaskProperties(parent)
{
    strFileExtensions   = GS::Gex::ConvertToSTDF::GetListOfSupportedFormat().join(";");    // list of files extensions to convert (eg: *.pcm;*.wat)
    iFormat             = GEXMO_TASK_CONVERTER_OUTPUT_STDF;     // STDF or CSV
    bTimeStampName      = false;
    iOnSuccess          = GEXMO_TASK_CONVERTER_SUCCESS_RENAME;  // What to do with files if successfuly converted
    iOnError            = GEXMO_TASK_CONVERTER_FAIL_RENAME;     // What to do with files if failed conversion

    iFrequency      = GEXMO_TASKFREQ_30MIN;                      // Task frequency.
    iDayOfWeek      = 0;                                         // Day of Week to execute task (0= Monday, ...6=Sunday)

    mPriority = 1;

}

void GexMoFileConverterTaskData::UpdatePrivateAttributes()
{
    ResetPrivateAttributes();
    // Insert new values
    SetPrivateAttribute("Title",strTitle);
    SetPrivateAttribute("InputFolder",strInputFolder);
    SetPrivateAttribute("ImportFileExtensions",strFileExtensions);
    SetPrivateAttribute("Frequency",QString::number(iFrequency));
    SetPrivateAttribute("DayOfWeek",QString::number(iDayOfWeek));
    SetPrivateAttribute("OutputFolder",strOutputFolder);
    SetPrivateAttribute("OutputFormat",QString::number(iFormat));
    SetPrivateAttribute("TimeStampFile",QString::number(bTimeStampName));
    SetPrivateAttribute("SuccessMode",QString::number(iOnSuccess));
    SetPrivateAttribute("SuccessFolder",strOutputSuccess);
    SetPrivateAttribute("FailMode",QString::number(iOnError));
    SetPrivateAttribute("FailFolder",strOutputError);
    SetPrivateAttribute("Priority",QVariant(mPriority));

}

GexMoFileConverterTaskData &GexMoFileConverterTaskData::operator=(const GexMoFileConverterTaskData &copy)
{
    if(this != &copy)
    {
        strTitle            = copy.strTitle;
        strInputFolder      = copy.strInputFolder;
        strFileExtensions   = copy.strFileExtensions;
        strOutputFolder     = copy.strOutputFolder;
        iFormat             = copy.iFormat;
        bTimeStampName      = copy.bTimeStampName;
        iFrequency          = copy.iFrequency;
        iDayOfWeek          = copy.iDayOfWeek;
        iOnSuccess          = copy.iOnSuccess;
        strOutputSuccess    = copy.strOutputSuccess;
        iOnError            = copy.iOnError;
        strOutputError      = copy.strOutputError;
        mPriority           = copy.mPriority;

        TaskProperties::operator =(copy);
        UpdatePrivateAttributes();
    }
    return *this;
}

///////////////////////////////////////////////////////////
// Execute Converter task...
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::ConvertDataFileLoop(CGexMoTaskConverter *ptTask, QString DataFile)
{
    QStringList             strDataFiles;
    QStringList::iterator   itFile;
    QString                 lConvertStatusMsg;
    QString                 strFileName;
    QString                 strFullFileName;

    if(ptTask == NULL)
        return;

    if (ptTask->GetProperties()->strOutputFolder.isEmpty())
    {
        lConvertStatusMsg += "Output folder is not defined for Converter task:" + ptTask->m_strName;
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError(QString(),lConvertStatusMsg);
        return;
    }


    // Process GUI pending messages
    QCoreApplication::processEvents();

    if(DataFile.isEmpty())
    {
        strDataFiles = GS::Gex::Engine::GetInstance().GetDatabaseEngine().GetListOfFiles(
                    ptTask->GetDataFilePath(), ptTask->GetDataFileExtension(),
                    ptTask->IsDataFileScanSubFolder(), ptTask->GetDataFileSort(),
                    ptTask->GetPriority());
    }
    else
        strDataFiles << DataFile;

    itFile = strDataFiles.begin();
    while (itFile != strDataFiles.end())
    {
        // Pause requested, so stop insertion for now...
        // Execution forced
        if((isSchedulerStopped()) && (ptTask->m_tLastExecuted != 1))
            return;

        // Get first file and process it (unless already processed and belongs to list of processed files).
        strFileName = *itFile;
        strDataFiles.erase(itFile);

        //strFullFileName = ptTask->GetProperties()->strInputFolder + "/" + strFileName;
        strFullFileName = strFileName;

        QString cause;
        // Check if file already imported
        // Check if the file can be processed
        // .read, .delay, .bad, .quarantine must be ignored
        if(!GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckIfFileToProcess(QFileInfo(strFullFileName).fileName(),ptTask, cause))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("This file does not have to be processed: %1")
                  .arg(cause).toLatin1().constData());
            continue;	// No insertion done.
        }

        onStartTask(ptTask,"FileName="+strFullFileName);
        // Check if the folder and the file can be edited
        if(!GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckFileForCopy(strFullFileName,ptTask,cause))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("This file cannot be processed: %1").arg( cause).toLatin1().constData());
            lConvertStatusMsg = "Error: ";
            lConvertStatusMsg += "Convertion failed: " + cause;
            lConvertStatusMsg += "\n\tSource: " + strFileName;
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError(QString(),lConvertStatusMsg);
        }

        if(lConvertStatusMsg.isEmpty())
            lConvertStatusMsg = ConvertOneFile(ptTask, strFullFileName);
        onStopTask(ptTask, lConvertStatusMsg);

        ++itFile;
    }

    emit sDisplayStatusMessage("");
}

///////////////////////////////////////////////////////////
// Execute Converter task...
///////////////////////////////////////////////////////////
QString GS::Gex::SchedulerEngine::ConvertOneFile(CGexMoTaskConverter *ptTask, QString strFullFileName)
{
    ConvertToSTDF     StdfConvert;		// To Convert to STDF


    QString             strFileName;
    QString             strFileNameSTDF;
    QFileInfo           cFileInfo;
    QString             lConvertStatusMsg;
    bool                bFileCreated;
    QString             strTimeStamp;

    if(ptTask == NULL)
        return "";

    // Pause requested, so stop insertion for now...
    // Execution forced
    if((isSchedulerStopped()) && (ptTask->m_tLastExecuted != 1))
        return "";

    cFileInfo.setFile(strFullFileName);
    strFileName = cFileInfo.fileName();

    // If output file name must include timestamp, build string!
    // GCORE-5447 : Use MM for decimal month to ensure proper sorting of files by filename if needed.
    if(ptTask->GetProperties()->bTimeStampName)
    {
        strTimeStamp = "_" + GS::Gex::Engine::GetInstance().GetClientDateTime().toString("yyyyMMdd.hhmmss");
    }
    else
        strTimeStamp = "";

    strFileNameSTDF = ptTask->GetProperties()->strOutputFolder+ "/" + strFileName + strTimeStamp + "_gexmo.std";


    // Display file name in status bar
    QString strStatusMessage = "Converting file:<br>" + strFullFileName;
    emit sDisplayStatusMessage(strStatusMessage);

    // Debug message
    QString strString;
    strString = "     GexMoScheduler::ConvertDataFileLoop(): Converting: " + strFullFileName;
    cFileInfo.setFile(strFullFileName);
    strString += " [Created:" + cFileInfo.created().toString("dd MMMM yyyy h:mm:ss") + " --- Last Modified: "+ cFileInfo.lastModified().toString("dd MMMM yyyy h:mm:ss") + "]";
    WriteDebugMessageFile(strString);

    // Process GUI pending messages
    QCoreApplication::processEvents();

    // 1: Convert file to STDF
    // Allow multi files
    QStringList lstFilesNamesConverted;
    QStringList lstFilesNameSTDF;
    lstFilesNameSTDF += strFileNameSTDF;
    int nConvertStatus = StdfConvert.Convert(true,false,false, true, strFullFileName, lstFilesNameSTDF,"",bFileCreated,lConvertStatusMsg);
    if(nConvertStatus != ConvertToSTDF::eConvertSuccess)
    {
        lConvertStatusMsg += "\nFile: " + strFullFileName;
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError(QString(),lConvertStatusMsg);
    }

    // If no conversion done because input format and output format
    else if(bFileCreated == false && ptTask->GetProperties()->iFormat == GEXMO_TASK_CONVERTER_OUTPUT_STDF)
    {
        if (QFile::exists(strFileNameSTDF) == false)
        {
            if (QFile::copy(strFullFileName, strFileNameSTDF) == false)
            {
                nConvertStatus = ConvertToSTDF::eConvertError;
                lConvertStatusMsg = "Failed to copy input file";
                lConvertStatusMsg += "\nFile: " + strFullFileName;
                GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError(QString(),lConvertStatusMsg);
            }
        }
    }
    // If conversion done
    else
    {
        while(!lstFilesNameSTDF.isEmpty())
        {
            strFileNameSTDF = lstFilesNameSTDF.takeFirst();

            // If output format is .CSV, then convert to .CSV
            if(ptTask->GetProperties()->iFormat == GEXMO_TASK_CONVERTER_OUTPUT_CSV)
            {
                QFileInfo   inputFile(strFileNameSTDF);
                QString		strFileNameCSV     = ptTask->GetProperties()->strOutputFolder + "/" + inputFile.completeBaseName();

                if (bFileCreated)
                {
                    if (strTimeStamp.isEmpty() || inputFile.fileName().contains(strTimeStamp))
                        strFileNameCSV += ".csv";
                    else
                        strFileNameCSV += strTimeStamp + ".csv";
                }
                else
                    strFileNameCSV += strTimeStamp + "_gexmo.csv";

                CSTDFtoCSV  CsvConvert;

                // STDF convertion successful, so now convert to CSV!
                if(!CsvConvert.Convert(strFileNameSTDF,strFileNameCSV))
                {
                    nConvertStatus = ConvertToSTDF::eConvertError;
                    lConvertStatusMsg = CsvConvert.GetLastError();
                    lConvertStatusMsg += "\nFile: " + strFullFileName;
                    GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError(QString(),lConvertStatusMsg);
                }
                else
                    lstFilesNamesConverted += strFileNameCSV;

                // Delete intermediate STDF file (if created)
                if(bFileCreated)
                    unlink(strFileNameSTDF.toLatin1().constData());
            }

            // If output format is ATDF, then convert to ATDF
            else if(ptTask->GetProperties()->iFormat == GEXMO_TASK_CONVERTER_OUTPUT_ATDF)
            {
                QFileInfo   inputFile(strFileNameSTDF);
                QString		strFileNameATDF     = ptTask->GetProperties()->strOutputFolder + "/" + inputFile.completeBaseName();

                if (bFileCreated)
                {
                    if (strTimeStamp.isEmpty() || inputFile.fileName().contains(strTimeStamp))
                        strFileNameATDF += ".atd";
                    else
                        strFileNameATDF += strTimeStamp + ".atd";
                }
                else
                    strFileNameATDF += strTimeStamp + "_gexmo.atd";

                // If input file was already a STDF file...
                CSTDFtoATDF	AtdfConvert(false);			// To Convert to ATDF (when used with YM converter task, we are not in evaluation mode)
                AtdfConvert.SetProcessRecord(true);		// All records should be processed

                // STDF convertion successful, so now convert to ATDF!
                if(!AtdfConvert.Convert(strFileNameSTDF,strFileNameATDF))
                {
                    nConvertStatus = ConvertToSTDF::eConvertError;
                    AtdfConvert.GetLastError(lConvertStatusMsg);
                    lConvertStatusMsg += "\nFile: " + strFullFileName;
                    GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError(QString(),lConvertStatusMsg);
                }
                else
                    lstFilesNamesConverted += strFileNameATDF;

                // Delete intermediate STDF file (if created)
                if(bFileCreated)
                    unlink(strFileNameSTDF.toLatin1().constData());
            }
            else if(ptTask->GetProperties()->iFormat == GEXMO_TASK_CONVERTER_OUTPUT_STDF)
            {
                lstFilesNamesConverted += strFileNameSTDF;
            }
        }
    }

    if(nConvertStatus == ConvertToSTDF::eConvertSuccess)
    {
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateReportLog(GEXMO_INSERTION_STATUS_OK,cFileInfo.fileName(),"",ptTask->GetProperties()->strTitle,ptTask->GetProperties()->strInputFolder);
    }
    else if(nConvertStatus == ConvertToSTDF::eConvertError)
    {
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateReportLog(GEXMO_INSERTION_STATUS_NOK,strFileName,"Error converting file",ptTask->GetProperties()->strTitle,ptTask->GetProperties()->strInputFolder);
    }
    else
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateReportLog(GEXMO_INSERTION_STATUS_DELAY,strFileName,"Error converting file",ptTask->GetProperties()->strTitle,ptTask->GetProperties()->strInputFolder);

    if(ptTask)
    {
        QString strDestFile;
        QString lErrorMsg = ProcessFileAfterExecution(nConvertStatus,ptTask,strFullFileName, strDestFile);
        if(!lErrorMsg.isEmpty())
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError(QString(),lErrorMsg);
    }

    if(nConvertStatus == ConvertToSTDF::eConvertError)
    {
        // Delete converted STDF file (if created)
        while(!lstFilesNamesConverted.isEmpty())
            unlink(lstFilesNamesConverted.takeFirst().toLatin1().constData());
    }

    bool bFailure = !lConvertStatusMsg.isEmpty();
    bool bComplete = lConvertStatusMsg.isEmpty();
    if(ptTask->GetProperties()->GetAttribute("RunScript").toBool())
    {
        bool bRunScript = bFailure && (ptTask->GetProperties()->GetAttribute("RunningMode").toInt() == 0);
        QString strAlarm;
        if(ptTask->GetProperties()->GetAttribute("RunningMode").toInt() == 0)
            strAlarm = "1";//"Failure";
        else
            strAlarm = "0";//"Completion";
        bRunScript = bRunScript || (bComplete && (ptTask->GetProperties()->GetAttribute("RunningMode").toInt() == 1));
        if(bRunScript)
        {
            QString strScript = ptTask->GetProperties()->GetAttribute("ScriptToRun").toString();
            //Alarm source file dest file
            QString strArguments = strAlarm + QString(" \"") + strFullFileName + QString("\" ") ;
            if(!lstFilesNamesConverted.isEmpty())
                strArguments += QString(" \"") + lstFilesNamesConverted.last() + QString("\" ");
            else
                strArguments += "\"Not Converted\"";
#ifdef _WIN32
            // Replace '/' to '\' to avoid MS-DOS compatibility issues
            strScript = strScript.replace('/','\\');
            strArguments = strArguments.replace('/','\\');
            ShellExecuteA(NULL,
                          "open",
                          strScript.toLatin1().constData(),
                          strArguments.toLatin1().constData(),
                          NULL,
                          SW_SHOWMINIMIZED);
#else
            QString strShellFile = strScript + QString(" ") + strArguments;
            if (system(strShellFile.toLatin1().constData()) == -1) {
                //FIXME: send error
            }
#endif
        }
    }

    if(!lConvertStatusMsg.isEmpty())
        lConvertStatusMsg = "Error: "+lConvertStatusMsg;

    return lConvertStatusMsg;
}

///////////////////////////////////////////////////////////
// Execute File Converter task!
///////////////////////////////////////////////////////////
QString GS::Gex::SchedulerEngine::ExecuteFileConverterTask(CGexMoTaskConverter *ptTask, QString DataFile)
{
    QDateTime	cCurrentDateTime = GS::Gex::Engine::GetInstance().GetClientDateTime();

    // Debug message
    QString strString;
    strString = "---- GexMoScheduler::ExecuteFileConverterTask(";
    strString += ptTask->GetProperties()->strTitle;
    strString += ", ";
    strString += cCurrentDateTime.toString("hh:mm:ss")+"): ";
    WriteDebugMessageFile(strString);

    ConvertDataFileLoop(ptTask, DataFile);

    return "";
}

