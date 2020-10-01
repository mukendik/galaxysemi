#include "browser_dialog.h"
#include "db_transactions.h"
#include "engine.h"
#include "gex_database_entry.h"
#include "gex_report.h"
#include "datapump/datapump_task.h"
#include "datapump/datapump_taskdata.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "mo_task.h"
#include "trigger_pat_task.h"
#include "scheduler_engine.h"
#include "gexmo_constants.h"
#include "db_engine.h"
#include "gex_scriptengine.h"
#include "db_datakeys_dialog.h"
#include "gqtl_datakeys_engine.h"
#include <gqtl_log.h>
#include "engine.h"
#include "product_info.h"

extern CGexReport*         gexReport;              // Handle to report class
extern GexScriptEngine*    pGexScriptEngine;

bool GS::Gex::DatabaseEngine::ExecuteScriptFile(CGexMoTaskTriggerPatPump *ptTask,
                                                QString &strSourceFile,
                                                QString &strErrorMessage)
{

    int             iStatus;
    QString         PumpName;
    QFileInfo       cFileInfo;
    QString         strLocalErrorMessageLine,strLocalErrorMessage;
    bool            bLogTaskDetails=false;
    QString         strShortFileName;

    // Debug message
    QString strMessage;
    strMessage= " process "+ QString("%1").arg(strSourceFile.count()) +" file(s)";

    GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data() );

    if(ptTask == NULL)
        return false;

    if((ptTask->GetTaskType() != GEXMO_TASK_PATPUMP)
            && (ptTask->GetTaskType() != GEXMO_TASK_TRIGGERPUMP))
        return false;

    // Check if have to log all details!
    bLogTaskDetails = GS::Gex::Engine::GetInstance().GetSchedulerEngine().LogTaskDetails();

    // Set pump name
    if(ptTask->GetProperties())
        PumpName = ptTask->GetProperties()->strTitle;

    // Display progress dialog...
    // Update Examinator status progress bar
    UpdateStatusMessage("Processing file: "+QFileInfo(strSourceFile).fileName());
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,1,1);	// Show process bar...step 1

    QString	strStatusMessage;
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        emit sDisplayStatusMessage("Processing file:<br>" + strSourceFile);

    // Disable the refresh database list
    emit sButtonReloadListEnabled(false);

    // Read the list of files selected, and add them to the database
    QString strShortErrorMsg;
    QString strDest;	// Used if rename/delete destination

    // Notify user is still active (not Idle)

    cFileInfo.setFile(strSourceFile);
    strShortFileName = cFileInfo.fileName();

    QTime	cTime;
    cTime = QTime::currentTime();	// Keep track of time of file processing

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        strStatusMessage = "Processing file:<br>" + strSourceFile;
        emit sDisplayStatusMessage(strStatusMessage);
    }

    // If log file has to report all details
    if(bLogTaskDetails)
    {
        strLocalErrorMessageLine = "[Task Details] Prepare execution for: " + strSourceFile;
        UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine);
    }


    m_strInsertionShortErrorMessage=""; // should be filled by ImportFile(...
    bool            bDeleteTrigger;
    QTime clTimer;
    clTimer.start();

    if(IsTriggerFile(strSourceFile))
    {
        // Force trigger to be not deleted in order to follow the datapump housekeeping settings
        iStatus = ProcessTriggerFile(ptTask, strSourceFile, &bDeleteTrigger,
                                     strErrorMessage, strShortErrorMsg);
    }
    else
    {
        // force .js to be not deleted to follow housekeeping settings
        iStatus = ProcessScriptFile(strSourceFile, &bDeleteTrigger,
                                    strErrorMessage, strShortErrorMsg);
    }

    // case 6419 : we dont need anymore the report whch could eat lot of mem
    if (gexReport)
    {
        delete gexReport;
        gexReport = new CGexReport;
        gexReport->setReportOptions(NULL);	// Will remain NULL unless a report is created...
    }

    // Key content
    GS::QtLib::DatakeysContent cKeyContent;
    cKeyContent.SetInternal("FullDestinationName",strSourceFile);
    strLocalErrorMessageLine = GS::Gex::Engine::GetInstance().GetSchedulerEngine().
            ProcessFileAfterExecution(iStatus,ptTask,cKeyContent);
    if(!strLocalErrorMessageLine.isEmpty())
        UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine);

    strDest = cKeyContent.Get("FullDestinationName").toString();

    switch(iStatus)
    {
    case Failed:
        strShortErrorMsg = m_strInsertionShortErrorMessage; //should n't it be: strInsertionShortErrorMessage=strShortErrorMsg;

        // Update report log
        cFileInfo.setFile(strDest);
        UpdateReportLog(GEXMO_INSERTION_STATUS_NOK,strShortFileName,
                        m_strInsertionShortErrorMessage,PumpName,cFileInfo.path());

        strLocalErrorMessageLine = "["+PumpName+"] Execution failed: ";
        if(!bLogTaskDetails)
        {
            strLocalErrorMessageLine += "\n\tFile: " + strShortFileName;
            strLocalErrorMessageLine += "\n\tError: ";
        }
        strLocalErrorMessageLine += strErrorMessage;

        UpdateLogError(PumpName,strLocalErrorMessageLine);
        strErrorMessage = "error:"+strErrorMessage;

        // If log file has to report all details
        if(bLogTaskDetails)
        {
            strLocalErrorMessageLine = "[Task Details] Execution failed for: " + strSourceFile;
            UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine);
        }
        break;

    case Passed:
        // Success inserting file. See what to do now!
        // If log file has to report all details
        if(bLogTaskDetails)
        {
            strLocalErrorMessageLine = "[Task Details] Execution successful for: " + strSourceFile;
            UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine);
        }
        UpdateReportLog(GEXMO_INSERTION_STATUS_OK, strShortFileName,
                        m_strInsertionShortErrorMessage, PumpName, cFileInfo.path(),
                        true, 0, 0, clTimer.elapsed()/1000);

        // Trace the ALARMS info
        // with the execution of the task into YmAdminDb
        if(ptTask && !m_strInsertionShortErrorMessage.isEmpty() && (m_strInsertionShortErrorMessage!="ok"))
        {
            strLocalErrorMessageLine = "["+PumpName+"] Execution warning(s): ";
            if(!bLogTaskDetails)
            {
                strLocalErrorMessageLine += "\n\tFile: " + strShortFileName;
                strLocalErrorMessageLine += "\n\tWarning: ";
            }
            strLocalErrorMessageLine += m_strInsertionShortErrorMessage;

            UpdateLogError(PumpName,strLocalErrorMessageLine);
        }

        strErrorMessage = "ok:";
        break; // case PassedInsertion:

    case Delay:	// File failed insertion for extrenal reasons (network busy, etc). So try again later!
        // Update report log
        cFileInfo.setFile(strSourceFile);
        UpdateReportLog(GEXMO_INSERTION_STATUS_DELAY,strShortFileName,
                        m_strInsertionShortErrorMessage,PumpName,cFileInfo.path());
        UpdateLogError(PumpName,"["+PumpName+"] Execution delayed: "+strErrorMessage);
        // If log file has to report all details
        if(bLogTaskDetails)
        {
            strLocalErrorMessageLine = "[Task Details] Execution delayed (will try later) for: " + strSourceFile;
            UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine);
        }
        strErrorMessage = "delay:"+strErrorMessage;
        break;
    }

    // Update status process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.

    // Full progress done...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,0,-1,false);	// hide status bar.
    UpdateStatusMessage("");

    // Restore the refresh database list
    emit sButtonReloadListEnabled(true);

    // Clear Task status message line (as insertion is completed)
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        emit sDisplayStatusMessage("");

    // Return status
    if(iStatus == Failed)
        return false;	// Failed importing file(s)
    else
        return true;	// Success
}

/******************************************************************************!
 * \fn stderrMessageHandler
 ******************************************************************************/
#ifdef QT_DEBUG
void stderrMessageHandler(QtMsgType ,
                          const QMessageLogContext& ,
                          const QString& msg)
{
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
}
#endif

/******************************************************************************!
 * \fn ImportFiles
 ******************************************************************************/
bool    GS::Gex::DatabaseEngine::ImportFiles(const QString &strDatabaseLogicalName,
                                             const QStringList &sFilesSelected,
                                             QStringList *pCorruptedFiles,
                                             GexDatabaseInsertedFilesList & listInsertedFiles,
                                             QString &strErrorMessage,
                                             bool &bEditHeaderInfo,
                                             bool bDelete/*=false*/,
                                             CGexMoTaskDataPump *ptTask/*=NULL*/,
                                             bool bImportFromRemoteDB/*=false*/,
                                             bool bFilesCreatedInFinalLocation/*=false*/)
{
    // if ptTask and DB
    // => Database insertion (only one file)
    // if ptTask = NULL and DB
    // => Manual insertion into a DB
    // if ptTask = NULL and DB
    // => Database insertion into a DB from Trigger execution

    int             iStatus;
    int             iErrorCount=0;
    int             iNumberOfFiles;
    QStringList::Iterator it;
    QString         strFileName, strDataPump, strFileInserted;
    QFileInfo       cFileInfo;
    QString         strLocalErrorMessageLine,strLocalErrorMessage;
    bool            bLogTaskDetails=false;
    QString         strShortFileName;

    QString strMessage;

    m_strInsertionShortErrorMessage = "";

    strMessage= " import "+ QString("%1").arg(sFilesSelected.count()) +" file(s) in DB " + strDatabaseLogicalName;
    GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data() );

    // Check if some files in list
    if(sFilesSelected.count() == 0)
        return true;

    if(ptTask && ptTask->GetProperties())
        strDataPump = ptTask->GetProperties()->strTitle;

    QStringList lFilesSelected = sFilesSelected;

    // Set short file name and data pump name

    cFileInfo.setFile(lFilesSelected[0]);

    strShortFileName = cFileInfo.fileName();

    if(strDatabaseLogicalName.isEmpty())
    {
        // Failed finding entry!
        QString strMessage = "Missing database - No data file inserted.";
        UpdateLogError(strErrorMessage,strMessage,!bImportFromRemoteDB);
        UpdateReportLog(GEXMO_INSERTION_STATUS_DELAY,strShortFileName,
                        "Couldn't find database",strDataPump,"",!bImportFromRemoteDB);

        // Return error message, but do not rename files to .bad, will try to import again later!
        return true;
    }

    bLogTaskDetails = GS::Gex::Engine::GetInstance().GetSchedulerEngine().LogTaskDetails();

    // Find database path to the logical name given...
    GexDatabaseEntry *pDatabaseEntry=NULL;
    pDatabaseEntry = FindDatabaseEntry(strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
    {
        // Failed finding entry!
        QString strMessage = "Failed to find database '" + strDatabaseLogicalName
                + "' (invalid path or missing database) - No data file inserted.";
        UpdateLogError(strErrorMessage,strMessage,!bImportFromRemoteDB);
        UpdateReportLog(GEXMO_INSERTION_STATUS_DELAY,strShortFileName,
                        "Couldn't find database",strDataPump,"",!bImportFromRemoteDB);

        // Return error message, but do not rename files to .bad, will try to import again later!
        return true;
    }

    // Check if Read ONLY database
    if((pDatabaseEntry->m_pExternalDatabase != NULL)
            && (bImportFromRemoteDB == false)
            && (pDatabaseEntry->m_pExternalDatabase->IsInsertionSupported()==false))
    {
        // Reject any insertion!!
        QString strMessage = "Data insertion rejected for database '" +
                strDatabaseLogicalName
                + "' (READ ONLY database) - No data file inserted.";
        UpdateLogError(strErrorMessage,strMessage,!bImportFromRemoteDB);
        UpdateReportLog(GEXMO_INSERTION_STATUS_DELAY,strShortFileName,
                        "READ ONLY database",strDataPump,"",!bImportFromRemoteDB);

        // Return error message, but do not rename files to .bad, will try to import again later!
        return true;
    }

    // Special case: .CSV file that includes Multiple-lots: needs preprocessing so to create as many CSV files as lots.
    SplitMultiLotCsvFiles(lFilesSelected);
    if(ptTask && ((lFilesSelected.count()>1) || (sFilesSelected.count()>1)))
    {
        // If more than one file, display warning...
        // Cannot process more than one files in the same transaction
        // Set short error message
        if(sFilesSelected.count()>1)
        {
            // Internal error ? No compatible with database insertion
            // Set complete error msg for log file
            strLocalErrorMessage = "Error: Selection includes Multiple files";
            strLocalErrorMessage +="\n\tFiles count to import : " + QString::number(sFilesSelected.count());
            strLocalErrorMessage +="\n\tYou are not allowed to insert a more than one file in a same transaction.";
            m_strInsertionShortErrorMessage = "Selection includes Multiple files!";
        }
        else
        {
            // From SplitMultiLotCsvFiles
            // User must convert this files first before to Pump the result
            // Set complete error msg for log file
            strLocalErrorMessage = "Error: CSV file includes Multiple-lots";
            strLocalErrorMessage +="\n\tFile to import : " + sFilesSelected.first();
            strLocalErrorMessage +="\n\tYou are not allowed to insert a CSV with multiple lots in it.";
            strLocalErrorMessage +="\n\tFile needs a preprocessing convertion task.";
            m_strInsertionShortErrorMessage = "CSV file includes Multiple-lots";
            // Remove uncompressed files created
            foreach(const QString &File , lFilesSelected)
            {
                // Remove file
                GS::Gex::Engine::RemoveFileFromDisk(File);
            }
        }

        // Trace the error as a FAIL TASK EXECUTION
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().onStartTask(ptTask,"FileName="+strFileName);

        GSLOG(SYSLOG_SEV_WARNING, QString("This file cannot be processed: %1").arg( strLocalErrorMessage).toLatin1().constData());
        UpdateLogError(strErrorMessage,strLocalErrorMessage,!bImportFromRemoteDB);

        GS::Gex::Engine::GetInstance().GetSchedulerEngine().onStopTask(ptTask,strLocalErrorMessage);

        return false;	// Error: Reject CSV file with multiple Lots in it!
    }

    mFilesToImport = sFilesSelected;

    // Display progress dialog...
    iNumberOfFiles = lFilesSelected.count();
    // Update Examinator status progress bar
    UpdateStatusMessage("Processing files");
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,iNumberOfFiles,1);	// Show process bar...step 1

    QString	strStatusMessage;
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        emit sDisplayStatusMessage("");

    // Disable the refresh database list
    emit sButtonReloadListEnabled(false);

    QTime	cTime;

    // Read the list of files selected, and add them to the database
    QString strDest;	// Used if rename/delete destination
    int     iFileID=0;
    for (it = lFilesSelected.begin(); it != lFilesSelected.end(); ++it )
    {
        strFileName = *it;
        cFileInfo.setFile(strFileName);
        strShortFileName = cFileInfo.fileName();


        QString cause;
        // Check if file already imported
        // Check if the file can be processed
        // .read, .delay, .bad, .quarantine must be ignored
        if(!CheckIfFileToProcess(cFileInfo.fileName(),ptTask, cause))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("This file does not have to be processed: %1").arg( cause).toLatin1().constData());
            *(pCorruptedFiles) += strFileName;
            iErrorCount++;
            continue;	// No insertion done.
        }

        // Notify user is still active (not Idle)
        // Only if not Monitoring (Action from tasks)
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().onStartTask(ptTask,"FileName="+strFileName);

        iFileID++;                      // Keeps track of File# inserted in database

        UpdateStatusMessage("Processing file: "+cFileInfo.fileName());

        if(lFilesSelected.count() > 1)
            strStatusMessage = "Data processing: File " + QString::number(iFileID) + " of "
                    + QString::number(iNumberOfFiles) + "<br>" + strFileName;
        else
            strStatusMessage = "Processing file:<br>" + strFileName;

        cTime = QTime::currentTime();   // Keep track of time of file processing

        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            emit sDisplayStatusMessage(strStatusMessage);

        // If log file has to report all details
        if(bLogTaskDetails)
        {
            strLocalErrorMessageLine = "[Task Details] Prepare insertion for: " + strFileName;
            UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine,!bImportFromRemoteDB);
        }

        // Check if the folder and the file can be edited
        if(!CheckFileForCopy(cFileInfo.canonicalFilePath(),ptTask,cause))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("This file cannot be processed: %1").arg( cause).toLatin1().constData());
            strLocalErrorMessage = "Insertion failed: " + cause;
            strLocalErrorMessage += "\n\tSource: " + strFileName;
            m_strInsertionShortErrorMessage = "Insertion failed";
            UpdateLogError(strErrorMessage,strLocalErrorMessage,!bImportFromRemoteDB);

            GS::Gex::Engine::GetInstance().GetSchedulerEngine().onStopTask(ptTask,strLocalErrorMessage);
            *(pCorruptedFiles) += strFileName;
            iErrorCount++;
            continue;	// No insertion done.
        }

        // Key content
        GS::QtLib::DatakeysContent cKeyContent(NULL, true);
        cKeyContent.SetInternal("FullDestinationName", strFileName);

        if(ptTask)
        {
            cKeyContent.SetInternal("DatabaseName", ptTask->GetDatabaseName());
            cKeyContent.SetInternal("DataPumpName", ptTask->m_strName);
            if(ptTask->GetProperties()->iPostImport == GEXMO_POSTIMPORT_FTP)
            {
                cKeyContent.SetInternal("UploadFile", "TRUE");
                cKeyContent.Set("MovePath", ptTask->GetProperties()->strPostImportMoveFolder);
            }
            else if(ptTask->GetProperties()->iPostImport == GEXMO_POSTIMPORT_MOVE)
            {
                cKeyContent.SetInternal("MoveFile", "TRUE");
                cKeyContent.Set("MovePath", ptTask->GetProperties()->strPostImportMoveFolder);
            }
        }

        m_strInsertionShortErrorMessage=""; // should be filled by ImportFile(...
        unsigned int uiFileSize=0;										// File size in bytes
        unsigned int uiOriginalSize = QFileInfo(strFileName).size();	// Size of compressed file
        unsigned int uiUncompressTime=0;									// Uncompression time in seconds
        unsigned int uiConvertTime=0;									// Conversion time in seconds
        unsigned int uiTotalInsertionTime=0;								// Total insertion time in seconds
        QTime clTimer;
        clTimer.start();

        // Initialize dbKeys engine
        GS::QtLib::DatakeysEngine dbKeysEngine(cKeyContent);

        // Get the dbkeys content updated during import file
        cKeyContent = dbKeysEngine.dbKeysContent();

        if(pDatabaseEntry->m_pExternalDatabase
                && ptTask && (ptTask->GetProperties()->uiDataType == GEX_DATAPUMP_DATATYPE_WYR))
            iStatus = ImportWyrFile(
                        pDatabaseEntry,strFileName,strErrorMessage,
                        dbKeysEngine,&uiFileSize,ptTask);
        else
            iStatus =
                 ImportFile(pDatabaseEntry,
                            strFileName,
                            false,
                            strFileName,
                            strFileInserted,
                            strErrorMessage,
                            bEditHeaderInfo,
                            dbKeysEngine,
                            &uiFileSize,
                            &uiUncompressTime,
                            &uiConvertTime,
                            ptTask,
                            bImportFromRemoteDB,
                            bFilesCreatedInFinalLocation);

        // case 6419 : we dont need anymore the report which could eat lot of mem
        if (gexReport && !bImportFromRemoteDB)
        {
            delete gexReport;
            gexReport = new CGexReport;
            gexReport->setReportOptions(NULL);	// Will remain NULL unless a report is created...
        }

        // Get the dbkeys content updated during import file
        cKeyContent = dbKeysEngine.dbKeysContent();

        // Check if any post insertion script to run
        QString lPostScript;
        if (ptTask && ptTask->GetProperties())
        {
            // was "PostInsertionJavaScript" // moved to "PostScript"
            QVariant lRun = ptTask->GetAttribute(GexMoDataPumpTaskData::sPostScriptActivatedAttrName);
            if(lRun.isValid() && lRun.toBool())
            {
                QVariant lContent = ptTask->GetAttribute(GexMoDataPumpTaskData::sPostScriptAttrName);
                if(lContent.isValid())
                   lPostScript=lContent.toString();
            }
        }

        if (ptTask && ptTask->GetProperties() && !lPostScript.isEmpty() )
        {
            // Let's add the KC to the engine
            QScriptValue KCobject = pGexScriptEngine->newQObject( &cKeyContent );
            if (!KCobject.isNull())
                pGexScriptEngine->globalObject().setProperty("CurrentGSKeysContent", KCobject);
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Failed to register CurrentGSKeysContent");
            }

            // Needed for ASE
            QScriptValue lCurrentTask=pGexScriptEngine->newQObject( ptTask );
            if (!lCurrentTask.isNull())
                pGexScriptEngine->globalObject().setProperty("CurrentGSTask", lCurrentTask);
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Failed to register CurrentGSKeysContent");
            }

            // Let's exec it
#           ifdef QT_DEBUG
            QtMessageHandler lOldHandler =
                qInstallMessageHandler(stderrMessageHandler);
#           endif
            QScriptValue lSV=pGexScriptEngine->evaluate(lPostScript);
#           ifdef QT_DEBUG
            qInstallMessageHandler(lOldHandler);
#           endif
            if(lSV.isError() || pGexScriptEngine->hasUncaughtException())
            {
                // Do not add the script to the log
                GSLOG(SYSLOG_SEV_ERROR, QString("PostInsertion script exec failed at line %1: %2")
                      .arg(pGexScriptEngine->uncaughtExceptionLineNumber())
                      .arg(pGexScriptEngine->uncaughtException().toString().toLatin1().data())
                      .toLatin1().data()
                      //.arg(lPIscript.toLatin1().data() )
                      );
            }
            else
            {
                GSLOG(SYSLOG_SEV_NOTICE, QString("PostInsertion script returned '%1'")
                      .arg(lSV.toString().toLatin1().data())
                      .toLatin1().constData());
            }
            // GCORE-499
            pGexScriptEngine->collectGarbage();
        }

        uiTotalInsertionTime = clTimer.elapsed()/1000;
        int InsertionTimeInMilliSec=clTimer.elapsed();

        if(ptTask)
        {
            strLocalErrorMessageLine = GS::Gex::Engine::GetInstance().GetSchedulerEngine().
                    ProcessFileAfterExecution(iStatus,ptTask,cKeyContent);
            if(!strLocalErrorMessageLine.isEmpty())
                UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine,!bImportFromRemoteDB);

            strDest = cKeyContent.Get("FullDestinationName").toString();
        }

        switch(iStatus)
        {
        case Failed:
        case FailedValidationStep:
            // Keep track of files not imported (corrupted)
            *(pCorruptedFiles) += strFileName;
            iErrorCount++;

            // Update report log
            cFileInfo.setFile(strDest);
            UpdateReportLog(GEXMO_INSERTION_STATUS_NOK,strShortFileName,
                            m_strInsertionShortErrorMessage,strDataPump,cFileInfo.path(),!bImportFromRemoteDB,
                            uiFileSize,uiOriginalSize,0,0,0);
            // If log file has to report all details
            if(bLogTaskDetails)
            {
                strLocalErrorMessageLine = "[Task Details] Insertion failed for: " + strFileName;
                UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine,!bImportFromRemoteDB);
            }
            break;

        case Passed:
            // Success inserting file. See what to do now!
            if(!strFileInserted.isEmpty())
                listInsertedFiles.append(GexDatabaseInsertedFiles(strFileName, strFileInserted));

            if(ptTask != NULL)
            {
                // If log file has to report all details
                if(bLogTaskDetails)
                {
                    strLocalErrorMessageLine = "[Task Details] Insertion successful for: " + strFileName;
                    UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine,!bImportFromRemoteDB);
                }

            } // if(ptTask != NULL)
            else
            {
                if(!bFilesCreatedInFinalLocation && bDelete)
                    GS::Gex::Engine::RemoveFileFromDisk(strFileName);	// Delete original file if requested.
            }

            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" imported %1 ko in total %2ms (%3)")
                   .arg(uiFileSize/1000).arg(InsertionTimeInMilliSec).arg(strFileName)
                   .toLatin1().data());
            UpdateReportLog(GEXMO_INSERTION_STATUS_OK, strShortFileName,
                            m_strInsertionShortErrorMessage, strDataPump, cFileInfo.path(),!bImportFromRemoteDB,
                            uiFileSize, uiOriginalSize, uiTotalInsertionTime, uiUncompressTime, uiConvertTime);

            if((ptTask == NULL)
                    && (!bImportFromRemoteDB))
            {
                // With a DataPump, YM will schedule the Consolidation Process
                // Without a DataPump, the Consolidation Process must be done now
                QMap< QString, QString > lSummary;
                QString lIncrementalName = "BINNING_CONSOLIDATION";
                QString lTestingStage = cKeyContent.Get("TestingStage").toString();
                QString lTarget = cKeyContent.Get("Lot").toString();
                if(!pDatabaseEntry->IncrementalUpdate(lIncrementalName,lTestingStage,lTarget,lSummary))
                {
                    strLocalErrorMessageLine = lSummary["error"];
                    UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine,!bImportFromRemoteDB);
                }
            }
            break; // case PassedInsertion:

        case Delay:	// File failed insertion for extrenal reasons (network busy, etc). So try again later!
            // Update report log
            cFileInfo.setFile(strFileName);
            UpdateReportLog(GEXMO_INSERTION_STATUS_DELAY,strShortFileName,
                            m_strInsertionShortErrorMessage,strDataPump,cFileInfo.path(),
                            !bImportFromRemoteDB,uiFileSize,uiOriginalSize,0,0,0);
            // If log file has to report all details
            if(bLogTaskDetails)
            {
                strLocalErrorMessageLine = "[Task Details] Insertion delayed (will try later) for: " + strFileName;
                UpdateLogError(strLocalErrorMessage,strLocalErrorMessageLine,!bImportFromRemoteDB);
            }
            break;
        }

        // Execute batch after insertion?
        if(ptTask && (ptTask->GetProperties()->bExecuteBatchAfterInsertion))
            LaunchPostInsertionShell(iStatus, strErrorMessage, cKeyContent, ptTask);

        // Update Database entry file (so to reflect correct database size)
        pDatabaseEntry->SaveToXmlFile(QString()); // TODO TDR check with sandrine if it's not to do only for file based DB
        //        UpdateDatabaseEntry(pDatabaseEntry->DbPhysicalPath(),pDatabaseEntry);

        // Update status process bar...
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.

        GS::Gex::Engine::GetInstance().GetSchedulerEngine().onStopTask(ptTask,cKeyContent.toList().join("|"));
    }

    // Full progress done...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,0,-1,false);	// hide status bar.
    UpdateStatusMessage("");

    // Restore the refresh database list
    emit sButtonReloadListEnabled(true);

    // Clear Task status message line (as insertion is completed)
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        emit sDisplayStatusMessage("");

    // Empty file list
    lFilesSelected.clear();
    mFilesToImport.clear();
    // Return status
    if(iErrorCount)
        return false;	// Failed importing file(s)
    else
        return true;	// Success
}

