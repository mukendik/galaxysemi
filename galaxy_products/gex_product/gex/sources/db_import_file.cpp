/******************************************************************************!
 * \file db_import_file.cpp
 ******************************************************************************/
#include <gqtl_archivefile.h>
#include <gqtl_sysutils.h>
#include "browser_dialog.h"
#include "gqtl_datakeys.h"
#include "gqtl_datakeys_engine.h"
#include "report_build.h"  // For CGexReport
#include "import_constants.h"	//
#include "engine.h"
#include "gex_shared.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "db_engine.h"
#include "classes.h"  // For CGexRange...
#include "gex_database_entry.h"
#include "gexmo_constants.h"
#include "scheduler_engine.h"
#include "datapump/datapump_task.h"
#include "datapump/datapump_taskdata.h"
#include "yield/yield_taskdata.h"
#include "statisticalMonitoring/statistical_monitoring_task.h"
#include "status/status_task.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "mo_email.h"
#include "status/status_taskdata.h"
#include "mo_task.h"
#include "report_options.h"
#include "stdfrecords_v4.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "script_wizard.h"
#include "csl/csl_engine.h"
#include "gex_scriptengine.h"
#include "gs_data.h"
#include "admin_engine.h"

extern CGexReport*				gexReport;				// Handle to report class
extern void						WriteDebugMessageFile(const QString & strMessage);
extern CReportOptions			ReportOptions;		// Holds options (report_build.h)
extern GexScriptEngine*         pGexScriptEngine;
// In script_wizard.h
extern void ConvertToScriptString(QString &strFile);

namespace GS
{

namespace Gex
{

/******************************************************************************!
 * \fn ImportFile
 ******************************************************************************/
QString DatabaseEngine::ImportFile(QtLib::DatakeysContent *lKC)
{
    QString lFN = lKC->Get("FileName").toString();
    QString lCFN = lKC->Get("ConfigFileName").toString();
    QString lDBN = lKC->Get("DatabaseName").toString();

    GSLOG(SYSLOG_SEV_NOTICE,
          QString("Import file : %1 into DB %2 using key %3").
          arg(lFN.toLatin1().data()).
          arg(lDBN.toLatin1().data()).
          arg(lCFN.toLatin1().data()).
          toLatin1().constData());

    GexDatabaseEntry* pDatabaseEntry = NULL;
    QString FileName = lFN;
    QString FileInserted;
    QString Message;
    QString ErrorMessage;
    bool    bEditHeaderInfo = false;

    QFileInfo       FileInfo(FileName);
    unsigned int    FileSize=0;                                   // File size in bytes
    unsigned int    OriginalSize = FileInfo.size();               // Size of compressed file
    unsigned int    UncompressTime=0;                             // Uncompression time in seconds
    unsigned int    ConvertTime=0;                                // Conversion time in seconds
    unsigned int    TotalInsertionTime=0;                         // Total insertion time in seconds

    pDatabaseEntry = FindDatabaseEntry(lKC->Get("DatabaseName").toString());
    bool bLogTaskDetails = false;
    bLogTaskDetails = GS::Gex::Engine::GetInstance().GetSchedulerEngine().LogTaskDetails();

    UpdateStatusMessage("Processing file: "+FileInfo.fileName());

    Message = "Processing file:<br>" + FileName;
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        emit sDisplayStatusMessage(Message);
    }

    // If log file has to report all details
    if(bLogTaskDetails)
    {
        Message = "[Task Details] Prepare insertion for: " + FileName;
        UpdateLogError("",Message);
    }

    // Key content
    lKC->SetInternal("FullDestinationName", FileName);
    GS::QtLib::DatakeysEngine dbKeysEngine(*lKC);

    m_strInsertionShortErrorMessage = "";  // Should be filled by ImportFile()

    // LOAD-BALANCING
    // Trace the execution of this import
    CGexMoTaskDataPump Task;
    Task.SetID(0);
    Task.m_strName = "Import file from JavaScript";
    if(pDatabaseEntry)
    {
        Task.m_iDatabaseId = pDatabaseEntry->m_nDatabaseId;
    }
    Task.TraceExecution("FileName=" + FileName,
                        "START",
                        "Import file from JavaScript");

    QTime clTimer;
    clTimer.start();

    int Status =
        ImportFile(pDatabaseEntry,
                   FileName,
                   false,
                   FileName,
                   FileInserted,
                            ErrorMessage,
                            bEditHeaderInfo,
                            dbKeysEngine,
                   &FileSize,
                   &UncompressTime,
                   &ConvertTime);

    Task.TraceExecution("FileName=" + FileName, "",
                        dbKeysEngine.dbKeysContent().toList().join("|"));

    TotalInsertionTime = clTimer.elapsed()/1000;

    int InsertionTimeInMilliSec=clTimer.elapsed();

    switch(Status)
    {
    case Failed:
    case FailedValidationStep:
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","ImportFile: Error importing file " + FileInfo.fileName(), m_strInsertionShortErrorMessage);
        // Update report log
        UpdateReportLog(GEXMO_INSERTION_STATUS_NOK,
                        FileInfo.fileName(),
                        m_strInsertionShortErrorMessage, "from JS",
                        FileInfo.path(), true,
                        FileSize,OriginalSize,0,0,0);
        // If log file has to report all details
        if(bLogTaskDetails)
        {
            Message = "[Task Details] Insertion failed for: " + FileName;
            UpdateLogError("",Message,true);
        }
        break;

    case Passed:
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("INFO","ImportFile: file " + FileInfo.fileName() + " imported without error", m_strInsertionShortErrorMessage);
        // Success inserting file. See what to do now
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString(" imported %1 ko in total %2ms (%3)").
              arg(FileSize / 1000).
              arg(InsertionTimeInMilliSec).
              arg(FileName).
              toLatin1().data());
        UpdateReportLog(GEXMO_INSERTION_STATUS_OK,
                        FileInfo.fileName(),
                        m_strInsertionShortErrorMessage, "from JS",
                        FileInfo.path(), true,
                        FileSize, OriginalSize,
                        TotalInsertionTime, UncompressTime,
                        ConvertTime);
        break; // case PassedInsertion:

    case Delay:
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("WARNING","ImportFile: file " + FileInfo.fileName() + " import delayed", m_strInsertionShortErrorMessage);
        // File failed insertion for extrenal reasons (network busy, etc.)
        // So try again later
        // Update report log
        UpdateReportLog(GEXMO_INSERTION_STATUS_DELAY,
                        FileInfo.fileName(),
                        m_strInsertionShortErrorMessage, "from JS",
                        FileInfo.path(),
                        true,FileSize,OriginalSize,0,0,0);
        // If log file has to report all details
        if(bLogTaskDetails)
        {
            Message =
                "[Task Details] Insertion delayed (will try later) for: " +
                FileName;
            UpdateLogError("",Message,true);
        }
        break;
    }

    // Update status process bar

    // Increment to next step
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false, -1, -1);

    // Full progress done
    // Hide status bar
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true, 0, -1, false);

    // Status:
    //    PassedInsertion = 0,        // File inserted with success
    //    FailedInsertion,            // File failed insertion (eg: file corrupted)
    //   FailedValidationStep,  // Failed validation step, file not corrupted
    // but doesn't match with the validation step
    //   DelayInsertion         // File failed insertion but not corrupted (eg:
    // copy problem, etc.), so delay insertion to try
    // again later
    QString KCError;
    if(Status == Passed)
    {
        KCError = "ok:";
    }
    if(Status == FailedValidationStep)
    {
        KCError = "error:";
    }
    if(Status == Failed)
    {
        KCError = "error:";
    }
    if(Status == Delay)
    {
        KCError = "delay:";
    }

    // Get the dbkeys content updated during import file
    *(lKC) = dbKeysEngine.dbKeysContent();
    QString lError;
    lError = lKC->Get("Error").toString();

    if(lError.isEmpty())
    {
        KCError += m_strInsertionShortErrorMessage;
    }
    else
    {
        KCError += lError;
    }

    lKC->Set("Status",Status);
    lKC->Set("Error",KCError);

    return KCError;
}

/******************************************************************************!
 * \fn SetImportError
 ******************************************************************************/
void DatabaseEngine::SetImportError(GS::QtLib::DatakeysEngine& dbKeysEngine,
                                    QString& ErrorMsg,
                                    const int ImportStatus)
{
    // Return/set error message and shirt description from error map
    ErrorMsg = GGET_LASTERRORMSG(DatabaseEngine, this);
    m_strInsertionShortErrorMessage = GGET_LASTERRORDESC(DatabaseEngine, this);

    // Overload error keys only if not already set
    // This is to avoid overloading error keys already set by external modules
    // (TDR insertion plugin, pre-insertion JS...)

    QString lErrorCode =
        dbKeysEngine.dbKeysContent().Get("ErrorCode").toString();
    if(lErrorCode.isEmpty())
    {
        // Set error message
        dbKeysEngine.dbKeysContent().Set("Error", ErrorMsg);

        // Set error code (IMP-<code>)
        lErrorCode =
            QString("IMP-%1").arg(GGET_LASTERRORCODE(DatabaseEngine, this));
        dbKeysEngine.dbKeysContent().Set("ErrorCode", lErrorCode);

        // Set error short description
        dbKeysEngine.dbKeysContent().Set("ErrorShortDesc",
                                         m_strInsertionShortErrorMessage);
    }

    // Set import status
    dbKeysEngine.dbKeysContent().Set("Status", ImportStatus);
}

/******************************************************************************!
 * \fn AddLastErrorToImportWarnings
 ******************************************************************************/
void DatabaseEngine::AddLastErrorToImportWarnings(
    GS::QtLib::DatakeysEngine& dbKeysEngine,
    QString& LastErrorMsg)
{
    LastErrorMsg = GGET_LASTERRORMSG(DatabaseEngine, this);
    QStringList lErrorMsg(LastErrorMsg);
    AddWarningsToImportWarnings(dbKeysEngine, lErrorMsg);
}

/******************************************************************************!
 * \fn AddWarningsToImportWarnings
 ******************************************************************************/
void DatabaseEngine::AddWarningsToImportWarnings(
    GS::QtLib::DatakeysEngine& dbKeysEngine,
    const QStringList& Warnings)
{
    QString lInfo = dbKeysEngine.dbKeysContent().Get("Info").toString();

    if(lInfo.isEmpty())
    {
        lInfo = QString("Insertion warning(s): \n\tFile: %1\n\tSource: %2").
            arg(dbKeysEngine.dbKeysContent().Get("StdfFileName").toString()).
            arg(dbKeysEngine.dbKeysContent().Get("SourceArchive").toString());
    }

    QStringList::ConstIterator lWarning;
    for(lWarning = Warnings.begin(); lWarning != Warnings.end(); ++lWarning)
    {
        lInfo += QString("\n\tWarning: %1").arg(*lWarning);
    }
    dbKeysEngine.dbKeysContent().Set("Info", lInfo);
}

/******************************************************************************!
 * \fn DbKeyContentSetFileInfo
 ******************************************************************************/
void
DatabaseEngine::DbKeyContentSetFileInfo(GS::QtLib::DatakeysEngine& dbKeysEngine,
                                        QString lFileToInsert,
                                        QString lFileNameSTDF,
                                        QString strSourceArchive)
{
    dbKeysEngine.dbKeysContent().SetInternal("FileName", lFileToInsert);
    dbKeysEngine.dbKeysContent().SetInternal("FileSize",
                                             QFileInfo(lFileToInsert).size());
    dbKeysEngine.dbKeysContent().SetInternal("StdfFileName", lFileNameSTDF);
    dbKeysEngine.dbKeysContent().SetInternal("StdfFileSize",
                                             QFileInfo(lFileNameSTDF).size());
    dbKeysEngine.dbKeysContent().SetInternal("SourceArchive", strSourceArchive);
}

/******************************************************************************!
 * \fn DatabaseInsertFile
 ******************************************************************************/
int
DatabaseEngine::DatabaseInsertFile(struct GsData* lGsData,
                                   int lSqliteSplitlotId,
                                   GexDatabaseEntry* pDatabaseEntry,
                                   QString lFileToInsert,
                                   QString& strErrorMessage,
                                   GS::QtLib::DatakeysEngine& dbKeysEngine,
                                   CGexMoTaskItem* ptTask,
                                   bool bImportFromRemoteDB)
{
    bool bIsImportIntoRDB = (pDatabaseEntry->m_pExternalDatabase != NULL) &&
        (bImportFromRemoteDB == false);
    if (! bIsImportIntoRDB)
    {
        return Passed;
    }

    // Support the service STOP for YM/PM running in daemon mode
    // Check if the Action insertion file is already DONE
    // Then have to update dbKeysEngine from database
    if (ptTask &&
        ptTask->GetActionMngAttribute("InsertionStatus") == "PASS")
    {
        // insertion already done
        // skip the insertion but update dbKeysEngine
        foreach(const QString &Key, ptTask->GetActionMngAttributes().keys())
            dbKeysEngine.dbKeysContent().SetInternal(Key, ptTask->GetActionMngAttributes()[Key]);

        dbKeysEngine.dbKeysContent().Set("Status", Passed);
        dbKeysEngine.dbKeysContent().Set("Error", "");
        dbKeysEngine.dbKeysContent().SetInternal("TimeInsertion", 0);
    }
    else
    {
        // If external database, and file should be imported to the remote DB
        // (and not to the local DB with data coming from the remote DB),
        // call relevant insertion routine
        bool bDelayInsertion = false;

        dbKeysEngine.dbKeysContent().SetInternal("InsertionStatus", "");

        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("insert into %1...").
              arg(pDatabaseEntry->m_pExternalDatabase->GetPluginName()).
              toLatin1().constData());
        // Check all options
        pDatabaseEntry->m_pExternalDatabase->
            SetInsertionValidationOptionFlag(
                GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_ALL);
        pDatabaseEntry->m_pExternalDatabase->
            SetInsertionValidationFailOnFlag(
                GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FAILON_YIELDMAN);

        bool Status = pDatabaseEntry->m_pExternalDatabase->
            InsertDataFile(lGsData,
                           lSqliteSplitlotId,
                           lFileToInsert,
                           dbKeysEngine,
                           &bDelayInsertion);
        if (Status == false)
        {
            // Failed inserting file in remote/corporate database file
            int lStatus = Passed;
            QString lLocalErrorMessage;

            // Initialize Error keys with TDR codes & messages,
            // so that they don't get overloaded by
            // SetImportError()
            dbKeysEngine.dbKeysContent().
                Set("Error", pDatabaseEntry->
                    m_pExternalDatabase->GetPluginErrorMsg());
            dbKeysEngine.dbKeysContent().
                Set("ErrorCode", pDatabaseEntry->
                    m_pExternalDatabase->GetPluginErrorCode());
            dbKeysEngine.dbKeysContent().
                Set("ErrorShortDesc", pDatabaseEntry->
                    m_pExternalDatabase->GetPluginErrorDescription());

            QStringList strlWarnings;
            pDatabaseEntry->m_pExternalDatabase->GetWarnings(strlWarnings);
            if (! strlWarnings.empty())
            {
                dbKeysEngine.dbKeysContent().
                    Set("Info", "Insertion warning(s):");
                AddWarningsToImportWarnings(dbKeysEngine, strlWarnings);
            }
            // Get error message from plug-in
            pDatabaseEntry->
                m_pExternalDatabase->GetLastError(lLocalErrorMessage);
            if (bDelayInsertion)
            {
                GSET_ERROR3(DatabaseEngine, eTdrInsertionDelay, NULL,
                            lFileToInsert.toLatin1().constData(),
                            dbKeysEngine.dbKeysContent().
                            Get("SourceArchive").
                            toString().toLatin1().constData(),
                            lLocalErrorMessage.toLatin1().constData());
                lStatus = Delay;
                dbKeysEngine.dbKeysContent().SetInternal("InsertionStatus",
                                                         "DELAY");
            }
            else
            {
                GSET_ERROR3(DatabaseEngine, eTdrInsertionFail, NULL,
                            lFileToInsert.toLatin1().constData(),
                            dbKeysEngine.dbKeysContent().
                            Get("SourceArchive").
                            toString().toLatin1().constData(),
                            lLocalErrorMessage.toLatin1().constData());
                lStatus = Failed;
                dbKeysEngine.dbKeysContent().SetInternal("InsertionStatus",
                                                         "FAIL");
            }
            SetImportError(dbKeysEngine, lLocalErrorMessage, lStatus);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);

            // Set PluginVersion key
            dbKeysEngine.dbKeysContent().
                Set("PluginVersion",
                    pDatabaseEntry->m_pExternalDatabase->GetPluginName());

            // Erase intermediate source created at root of database folder
            if (QFileInfo(lFileToInsert).suffix().toLower() != "sqlite")
            {
                GS::Gex::Engine::RemoveFileFromDisk(lFileToInsert);
            }
            return lStatus;
        }
        else
        {
            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("Import file: splilot %1 successfully inserted").
                  arg(dbKeysEngine.dbKeysContent().Get("SplitlotId").
                      toLongLong()).
                  toLatin1().constData());

            dbKeysEngine.dbKeysContent().Set("Status", Passed);
            dbKeysEngine.dbKeysContent().Set("Error", "");
            dbKeysEngine.dbKeysContent().Set("ErrorCode", "");
            dbKeysEngine.dbKeysContent().Set("ErrorShortDesc", "");

            // Insertion successful, write warnings to Log file if any
            QStringList strlWarnings;
            pDatabaseEntry->m_pExternalDatabase->GetWarnings(strlWarnings);
            if (! strlWarnings.empty())
            {
                AddWarningsToImportWarnings(dbKeysEngine, strlWarnings);
                QString lLocalWarningMessage =
                    dbKeysEngine.dbKeysContent().Get("Info").toString();
                QString lWarningMessage;
                UpdateLogError(lWarningMessage,
                               lLocalWarningMessage,
                               ! bImportFromRemoteDB);
            }

            // Support the service STOP for YM/PM running in daemon mode
            // Update the Actions list to indicate that
            // the importe file is DONE
            dbKeysEngine.dbKeysContent().SetInternal("InsertionStatus",
                                                     "PASS");
            if (ptTask)
            {
                ptTask->UpdateActionMng(dbKeysEngine.dbKeysContent().toList().join("|"));
            }
        }
    }

    return Passed;
}

/******************************************************************************!
 * \fn RunPreInsertionScript
 ******************************************************************************/
int
DatabaseEngine::RunPreInsertionScript(
    QString& strErrorMessage,
    GS::QtLib::DatakeysEngine& dbKeysEngine,
    CGexMoTaskItem* ptTask,
    bool bImportFromRemoteDB,
    QString& lLocalErrorMessage)
{
    QString lPreIscript;

    if (! ptTask ||
        ! ptTask->GetDataPumpData())
    {
        return Passed;
    }
    // Was "PreInsertionJavaScript" // moved to "PreScript"
    QVariant lRun = ptTask->
        GetAttribute(GexMoDataPumpTaskData::sPreScriptActivatedAttrName);

    if (lRun.isValid() && lRun.toBool())
    {
        QVariant lContent =
            ptTask->GetAttribute(GexMoDataPumpTaskData::sPreScriptAttrName);
        if (lContent.isValid())
        {
            lPreIscript = lContent.toString();
        }
    }

    if (ptTask && ptTask->GetDataPumpData() && ! lPreIscript.isEmpty())
    {
        // Let's add the KC to the engine
        QScriptValue KCobject =
            pGexScriptEngine->newQObject(&dbKeysEngine.dbKeysContent());
        if (KCobject.isNull())
        {
            lLocalErrorMessage = "Failed to register CurrentGSKeysContent";
            // Update error log
            GSET_ERROR1(DatabaseEngine, eFailedToExecPreInsertionJs, NULL,
                        lLocalErrorMessage.toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, Delay);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            GSLOG(SYSLOG_SEV_ERROR, lLocalErrorMessage.toLatin1().data());
            return Delay;
        }

        pGexScriptEngine->globalObject().setProperty("CurrentGSKeysContent",
                                                     KCobject);
        dbKeysEngine.dbKeysContent().Set("Status", 0);

        // Let's exec it
        QScriptValue lSV = pGexScriptEngine->evaluate(lPreIscript);

        // GCORE-499
        pGexScriptEngine->collectGarbage();

        if (lSV.isError() || pGexScriptEngine->hasUncaughtException())
        {
            // Do not add the script to the log
            lLocalErrorMessage =
                pGexScriptEngine->uncaughtException().toString() +
                " at line " +
                QString::number(pGexScriptEngine->
                                uncaughtExceptionLineNumber());
            // Update error log
            GSET_ERROR1(DatabaseEngine, eFailedToExecPreInsertionJs, NULL,
                        lLocalErrorMessage.toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, Delay);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            GSLOG(SYSLOG_SEV_ERROR, lLocalErrorMessage.toLatin1().data());
            return Delay;
        }

        GSLOG(SYSLOG_SEV_NOTICE, QString("PreInsertion script returned %1").
              arg(lSV.toString().toLatin1().data()).
              toLatin1().constData());
        bool lIsNum;
        int lScriptStatus =
            dbKeysEngine.dbKeysContent().Get("Status").toInt(&lIsNum);
        if (! lIsNum || (lScriptStatus < 0))
        {
            lLocalErrorMessage = "incorrect Status " +
                dbKeysEngine.dbKeysContent().Get("Status").toString();
            // Update error log
            GSET_ERROR1(DatabaseEngine, eErrorInPreInsertionJs, NULL,
                        lLocalErrorMessage.toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, Delay);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            GSLOG(SYSLOG_SEV_ERROR, lLocalErrorMessage.toLatin1().data());
            return Delay;
        }

        if (lScriptStatus != Passed)
        {
            lLocalErrorMessage =
                dbKeysEngine.dbKeysContent().Get("Error").toString();
            // Update error log (lLocalErrorMessage already set above)
            GSET_ERROR1(DatabaseEngine, ePreInsertionJsReject, NULL,
                        lLocalErrorMessage.toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, lScriptStatus);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            GSLOG(SYSLOG_SEV_ERROR, lLocalErrorMessage.toLatin1().data());
            return lScriptStatus;
        }
    }

    return Passed;
}

/******************************************************************************!
 * \fn CheckOverloadedKeyWords
 ******************************************************************************/
void
DatabaseEngine::CheckOverloadedKeyWords(long t_SetupTime,
                                        long t_StartTime,
                                        long t_FinishTime,
                                        GS::QtLib::DatakeysEngine& dbKeysEngine,
                                        QString& strError)
{
    long timeStart;
    long timeEnd;

    timeStart = QDateTime(QDate(2000, 1, 1)).toTime_t();
    timeEnd = GS::Gex::Engine::GetInstance().GetClientDateTime().addDays(1).toTime_t();

    if (strError.isEmpty() && (t_SetupTime > 0) &&
        (dbKeysEngine.dbKeysContent().Get("SetupTime").toLongLong() !=
         t_SetupTime) &&
        ((dbKeysEngine.dbKeysContent().Get("SetupTime").toLongLong() <
          timeStart) ||
         (dbKeysEngine.dbKeysContent().Get("SetupTime").toLongLong() >
          timeEnd)))
    {
        strError = "Invalid TimeStamp value for SetupTime : " +
            QString::number(dbKeysEngine.dbKeysContent().
                            Get("SetupTime").toLongLong());
    }

    if (strError.isEmpty() && (t_StartTime > 0) &&
        (dbKeysEngine.dbKeysContent().IsOverloaded("StartTime")) &&
        ((dbKeysEngine.dbKeysContent().Get("StartTime").toLongLong() <
          timeStart) ||
         (dbKeysEngine.dbKeysContent().Get("StartTime").toLongLong() >
          timeEnd)))
    {
        strError = "Invalid TimeStamp value for StartTime : " +
            QString::number(dbKeysEngine.dbKeysContent().
                            Get("StartTime").toLongLong());
    }

    if (strError.isEmpty() && (t_FinishTime > 0) &&
        (! dbKeysEngine.dbKeysContent().IsOverloaded("FinishTime")) &&
        ((dbKeysEngine.dbKeysContent().Get("FinishTime").toLongLong() <
          timeStart) ||
         (dbKeysEngine.dbKeysContent().Get("FinishTime").toLongLong() >
          timeEnd)))
    {
        strError = "Invalid TimeStamp value for FinishTime : " +
            QString::number(dbKeysEngine.dbKeysContent().
                            Get("FinishTime").toLongLong());
    }
}

/******************************************************************************!
 * \fn ImportFile
 ******************************************************************************/
int
DatabaseEngine::ImportFile(GexDatabaseEntry* pDatabaseEntry,
                           QString strSourceArchive,
                           bool /*bFromArchive*/,
                           QString strFileName,
                           QString& strFileInserted,
                           QString& strErrorMessage,
                           bool& bEditHeaderInfo,
                           GS::QtLib::DatakeysEngine& dbKeysEngine,
                           unsigned int* puiFileSize,
                           unsigned int* puiUncompressTime,
                           unsigned int* puiConvertTime,
                           CGexMoTaskDataPump* ptTask,
                           bool bImportFromRemoteDB,
                           bool bFilesCreatedInFinalLocation)
{
    // if ptTask and DB
    // => Database insertion
    // if ptTask = NULL and DB
    // => Manual insertion into a DB
    // if ptTask = NULL and DB
    // => Database insertion into a DB from Trigger execution

    bool            lEditHeaderInfo=false, lResult, lMonitoring;
    bool lFileCreated, lMakeLocalCopy, lZipCopy = false, lSummaryOnly,
         bBlackHole;
    ConvertToSTDF lStdfConvert;
    QString lFileNameSTDF, lFileShortSTDF;
    QString lLocalErrorMessage;
    float lFileSize = 0.0;
    int lStatus = Passed;
    QString lFileUncompressed;
    QDir lDir;
    QTime lTimer;
    QString lFileToInsert = strFileName;

    QString lMessage = "Import File: "; lMessage += lFileToInsert;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data());

    // Init variables
    m_strInsertionShortErrorMessage = "";
    strFileInserted = "";
    *puiFileSize = 0;
    *puiConvertTime = 0;
    // Do not edit header if file has been imported for report
    lEditHeaderInfo = !bImportFromRemoteDB;

    // Check if Monitoring with full details in log file
    bool	lLogTaskDetails=false;
    QString	lLocalErrorMessageLine;
    lLogTaskDetails =
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().LogTaskDetails();

    // Process GUI messages if any pending
    // Allows to avoid client/server timout if heart-bit not sent
    QCoreApplication::processEvents();

    if (GS::LPPlugin::ProductInfo::getInstance()->isPATMan() &&
        GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        GSET_ERROR0(DatabaseEngine, eWrongLicense, NULL);
        SetImportError(dbKeysEngine, strErrorMessage, Delay);
        return Delay;
    }

    if(pDatabaseEntry == NULL)
    {
        // Failed finding entry
        GSET_ERROR0(DatabaseEngine, eDatabaseNotFound, NULL);
        SetImportError(dbKeysEngine, strErrorMessage, Delay);
        return Delay;
    }

    // Check if Read ONLY database
    if ((pDatabaseEntry->m_pExternalDatabase != NULL) &&
        (bImportFromRemoteDB == false) &&
        (pDatabaseEntry->m_pExternalDatabase->IsInsertionSupported() == false))
    {
        // Reject any insertion
        GSET_ERROR1(DatabaseEngine,
                    eReadOnlyDatabase,
                    NULL,
                    pDatabaseEntry->LogicalName().toLatin1().constData());
        SetImportError(dbKeysEngine, strErrorMessage, Delay);
        // Return error message, but do not rename files to .bad,
        // will try to import again later
        return Delay;
    }

    if(!QFileInfo(lFileToInsert).exists())
    {
        // File not exists
        GSET_ERROR1(DatabaseEngine, eFileNotFound, NULL,
                    lFileToInsert.toLatin1().constData());
        SetImportError(dbKeysEngine, strErrorMessage, Failed);
        // Return error message, but do not rename files to .bad,
        // will try to import again later
        return Failed;
    }

    // Only Pro editions support compressed format
    // (except Monitoring which doesn't exist in 2 editions so far)
    // case 4768 cleanup
    // bAllowAdvancedFormats = true;

    if(ptTask != NULL)
    {
        lMonitoring = true;
    }
    else
    {
        lMonitoring = false;
    }

    // bMonitoring depends of the application, not of the task
    lMonitoring = GS::LPPlugin::ProductInfo::getInstance()->isMonitoring();
    // Task can be NULL if the process doesn't come from the OnCheckScheduler

    // Check if compressed file, if so, uncompress it first
    // Check if it is a compressed file
    CArchiveFile	clZipCheck("");
    if(clZipCheck.IsCompressedFile(lFileToInsert))
    {
        lTimer.start();

        // Make sure the database /.temp/ folder exists!
        QString	strUnzipFolder;
        // Get custom path if defined
        if (ReportOptions.GetOption("dataprocessing",
                                    "stdf_intermediate").toString() == "custom")
        {
            strUnzipFolder =
                ReportOptions.GetOption("dataprocessing",
                                        "stdf_intermediate_path").toString();
            if(!strUnzipFolder.isEmpty())
            {
                if(!QFile::exists(strUnzipFolder))
                {
                    QDir lDir;
                    // If option defined, create the path
                    if(!lDir.mkpath(strUnzipFolder))
                    {
                        GSLOG(SYSLOG_SEV_WARNING,
                              QString("Unable to create custom path %1").
                              arg(strUnzipFolder).
                              toLatin1().constData());
                        strUnzipFolder = "";
                    }
                }
            }
        }
        // Get the output file
        if(strUnzipFolder.isEmpty())
        {
            // Try to use the OutPut folder defined
            strUnzipFolder = pDatabaseEntry->PhysicalPath();
        }

        // If '.temp' folder doesn't exists create it
        if(QFile::exists(strUnzipFolder) == false)
        {
            if(lDir.mkdir(strUnzipFolder) == false)
            {
                GSET_ERROR2(DatabaseEngine,
                            eFailedToCreateUnzipFolder,
                            NULL,
                            strUnzipFolder.toLatin1().constData(),
                            strSourceArchive.toLatin1().constData());
                SetImportError(dbKeysEngine, lLocalErrorMessage, Delay);
                UpdateLogError(strErrorMessage,
                               lLocalErrorMessage,
                               ! bImportFromRemoteDB);
                // Do not rename file to '.bad': simply process on next round
                return Delay;
            }
        }

        // Check if it is a compressed file
        CArchiveFile clZip(strUnzipFolder);
        GSLOG(SYSLOG_SEV_DEBUG, " file seems compressed...");

        // Log details
        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine = "[Task Details] Prepare to unzip: " +
                lFileToInsert;
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }

        GSLOG(SYSLOG_SEV_DEBUG, "uncompress file...");

        // Uncompress file
        QStringList	strUncompressedFiles;
        lResult = clZip.Uncompress(lFileToInsert, strUncompressedFiles);
        *puiUncompressTime = lTimer.elapsed()/1000;
        if(lResult == false)
        {
            GSET_ERROR3(DatabaseEngine,
                        eFailedToUnzipFile,
                        NULL,
                        strUnzipFolder.toLatin1().constData(),
                        strSourceArchive.toLatin1().constData(),
                        clZip.GetLastErrorMsg().toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            return Failed;	// Error uncompressing
        }

        // Log details
        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine = "[Task Details] Unzip successful on: " +
                lFileToInsert;
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }

        // If more than one file in the archive, display warning
        if(strUncompressedFiles.count() > 1)
        {
            GSET_ERROR1(DatabaseEngine,
                        eMultipleFilesInArchive,
                        NULL,
                        strSourceArchive.toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            // Remove uncompressed files created
            for (QStringList::Iterator itFile = strUncompressedFiles.begin();
                 itFile != strUncompressedFiles.end();
                 itFile++)
            {
                // Build full path to unzipped file
                lFileUncompressed = pDatabaseEntry->PhysicalPath();
                lFileUncompressed += "/" + *itFile;
                // Remove file
                GS::Gex::Engine::RemoveFileFromDisk(lFileUncompressed);
            }
            // Error: Reject compressed file with multiple files in it
            return Failed;
        }

        // Only one file

        // Recursive call to process all these files
        // Build full path to zip file
        lFileUncompressed = strUnzipFolder;
        lFileUncompressed += "/" + strUncompressedFiles.takeFirst();

        lStatus =
            ImportFile(pDatabaseEntry,
                       lFileToInsert,
                       true,
                       lFileUncompressed,
                       strFileInserted,
                       strErrorMessage,
                       bEditHeaderInfo,
                       dbKeysEngine,
                       puiFileSize,
                       puiUncompressTime,
                       puiConvertTime,
                       ptTask,
                       bImportFromRemoteDB,
                       bFilesCreatedInFinalLocation);

        // Remove uncompressed file in temporary folder file
        GS::Gex::Engine::RemoveFileFromDisk(lFileUncompressed);

        // If only one file inserted but failed, return it's exact status
        return lStatus;
    } // compressed

    GSLOG(SYSLOG_SEV_DEBUG, QString(" importing (uncompressed) file %1...").
          arg(lFileToInsert).toLatin1().constData());

    // SQLite
    if (QFileInfo(lFileToInsert).suffix().toLower() == "sqlite")
    {
        lTimer.start();
        unsigned int uiInsertionTime = lTimer.elapsed() / 1000;
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("importing sqlite file").toUtf8().constData());
        lStatus = this->ImportFileSqlite(pDatabaseEntry,
                                         strSourceArchive,
                                         lFileToInsert,
                                         strErrorMessage,
                                         bEditHeaderInfo,
                                         dbKeysEngine,
                                         ptTask,
                                         bImportFromRemoteDB);
        if (lStatus != Passed)
        {
            return lStatus;
        }
        dbKeysEngine.dbKeysContent().SetInternal("TimeUncompress",
                                                 *puiUncompressTime);
        dbKeysEngine.dbKeysContent().SetInternal("TimeInsertion",
                                                 uiInsertionTime);
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Import file: UncompressTime=%1, InsertionTime=%2").
              arg(*puiUncompressTime).
              arg(uiInsertionTime).
              toLatin1().constData());
        return Passed;
    }

    // Build destination path to the database
    // Note: If database storage mode = 'Link', we still create a STDF file for
    // non-stdf file files imported

    // STDF target name in the database
    QString strExt = ".std";
    if(lFileToInsert.endsWith(QString(".stdf"),Qt::CaseInsensitive))
    {
        strExt = ".stdf";
    }

    // Insert file name 'as-is': keep current name in database
    QFileInfo cFileInfo(lFileToInsert);
    lFileShortSTDF = "/" + cFileInfo.completeBaseName();
    if (! lFileShortSTDF.endsWith(QString(".std"), Qt::CaseInsensitive) &&
        ! lFileShortSTDF.endsWith(QString(".stdf"), Qt::CaseInsensitive))
    {
        lFileShortSTDF += strExt;
    }

    // File name built from current date = <database>/fYYMMDDHHMMSS_<index>.std
    lFileNameSTDF = pDatabaseEntry->PhysicalPath();
    lFileNameSTDF += lFileShortSTDF;

    lMessage = "check if conversion required for "+lFileNameSTDF;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data());

    // Convert to STDF
    lTimer.start();
    int nConvertStatus = lStdfConvert.
        Convert(lMonitoring,
                true,
                true,
                true,
                lFileToInsert,
                lFileNameSTDF,
                "",
                lFileCreated,
                lLocalErrorMessage);
    *puiConvertTime = lTimer.elapsed()/1000;
    if(nConvertStatus == ConvertToSTDF::eConvertError)
    {
        // Error converting file to STDF format
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("Conversion error: %1").
              arg(lLocalErrorMessage).toLatin1().data());
        // Simplify double-CRLF
        lLocalErrorMessage = lLocalErrorMessage.replace("\n\n", "\n");
        lLocalErrorMessage = lLocalErrorMessage.replace("\n","\n\t");
        GSET_ERROR2(DatabaseEngine, eFailedToConvertFile, NULL,
                    strSourceArchive.toLatin1().constData(),
                    lLocalErrorMessage.toLatin1().constData());
        SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
        UpdateLogError(strErrorMessage,
                       lLocalErrorMessage,
                       ! bImportFromRemoteDB);
        // Remove uncompressed file in temporary folder file
        // Only for created file
        if(strSourceArchive != lFileToInsert)
        {
            GS::Gex::Engine::RemoveFileFromDisk(lFileToInsert);
        }
        return Failed;	// Error
    }

    lSummaryOnly = pDatabaseEntry->IsSummaryOnly();
    bBlackHole   = pDatabaseEntry->IsBlackHole();
    lResult = true;
    // Set flag: Local copy or Link?
    if(lFileCreated == 0)
    {
        // No intermediate STDF file created, because importing a STDF file
        if(bFilesCreatedInFinalLocation)
        {
            lMakeLocalCopy = false;
        }
        else
        {
            lMakeLocalCopy = pDatabaseEntry->HoldsFileCopy();
        }
        if(lMakeLocalCopy == false)
        {
            lFileNameSTDF = lFileToInsert;
        }
        lZipCopy = pDatabaseEntry->IsCompressed();
    }
    else
    {
        // Importing a non-STDF file,
        // so intermediate STDF file created in database
        lMakeLocalCopy = true;
    }

    if((lFileCreated == 0) && (lMakeLocalCopy == true))
    {
        // File not created as it is already a STDF file,
        // then make a local copy, unless we are in the 'Link' mode,
        // or already copied (if unzipped in this folder)
        // Get file size to be added to the database
        lFileSize = QFile(lFileToInsert)	.size()/(1024.0*1024.0);

        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine =
                "[Task Details] Prepare to copy into database root: " +
                lFileToInsert;
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }

        lFileNameSTDF = pDatabaseEntry->PhysicalPath();
        lFileNameSTDF += lFileShortSTDF;

        // Trigger copy, unless source = destination folder
        QFileInfo cSource(lFileToInsert);
        QFileInfo cDest(lFileNameSTDF);
        if(cSource.absolutePath() != cDest.absolutePath())
        {
            lResult = CGexSystemUtils::CopyFile(lFileToInsert,lFileNameSTDF);
        }
        else
        {
            // File already copied locally, no need to execute copy function
            // (usually happens if unzipped first)
            // Then simply rename in case file name slightly different
            // (eg: .STDF insteda or .STD, etc.)
            if (cSource.fileName().compare(cDest.fileName(),
                                           Qt::CaseInsensitive) != 0)
            {
                GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
                lResult = lDir.rename(lFileToInsert,lFileNameSTDF);
            }
        }

        // Check if copy successful
        if(lResult == false)
        {
            GSET_ERROR2(DatabaseEngine, eFailedToCopyFile, NULL,
                        lFileToInsert.toLatin1().constData(),
                        lFileNameSTDF.toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, Delay);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            // Do not rename file to '.bad': simply process on next round
            return Delay;
        }
        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine =
                "[Task Details] Copy into database root successful";
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }

    } // Storage mode = 'Copy'
    else
    {
        if(lMakeLocalCopy)
        {
            lFileSize = QFile(lFileNameSTDF).size()/(1024.0*1024.0);
        }
        else
        {
            // Assume maximum index tables have grown by 300 bytes
            lFileSize = 0.000286;
        }
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("Copy mode: file will use %1 Mb").
              arg(lFileSize).toLatin1().data());
    }

    UpdateStatusMessage("Processing file: "+cFileInfo.fileName());

    // Init file size information:
    // use size of the final STDF file being inserted
    *puiFileSize = QFileInfo(lFileNameSTDF).size();

    // Read STDF MIR record to
    //GS::StdLib::StdfRecordReadInfo   StdfRecordHeader;
    //BYTE            bData, bPassFailInfo;
    //long            lData;
    //int   wData, wBinning=65535, wHardBin, wSoftBin, wNumTests;
    //char            szString[GEX_MAX_PATH];
    //float           fData;
    long            t_SetupTime;
    long            t_StartTime;
    long            t_FinishTime;
    //char            cWaferNotch;

    t_SetupTime = t_StartTime = t_FinishTime = 0;
    //cWaferNotch = 0;

    bool        bCheckYieldLevel;

    // Used for Examinator-Monitoring
    long        lTotalParts=0;
    long        lTotalGoodBins=0;
    long        lSBRTotalParts=0;
    long        lSBRTotalPartsAllSites=0;
    long        lSBRTotalGoodBins=0;
    long        lHBRTotalParts=0;
    long        lHBRTotalPartsAllSites=0;
    long        lHBRTotalGoodBins=0;
    float       fYieldLevel=0.0F;
    GS::QtLib::Range   *pGlobalYieldBinCheck=NULL;

    // Used for checking Yield on a specific ProductID
    CGexMoTaskYieldMonitor  *ptYieldTask=NULL;
    QList<CGexMoTaskStatisticalMonitoring*> lSpmTask;
    QList<CGexMoTaskStatisticalMonitoring*> lSyaTasks;

    //BYTE            bHeadNumber;

    // Used to check if file should be rejected
    // due to PASS Hbins not in specified list
    GS::QtLib::Range clRange_PassBinlistForRejectTest;
    bool            bCheckPassBinNotInList=false;
    int             nPassBinNotInList=-1;
    QString         strPassBinNotInList;

    // Open STDF file to read until MIR record found,
    // have a 50K cache buffer used (instead of 2M)
    this->DbKeyContentSetFileInfo(dbKeysEngine,
                                  lFileToInsert,
                                  lFileNameSTDF,
                                  strSourceArchive);

    if(lLogTaskDetails)
    {
        lLocalErrorMessageLine = "[Task Details] STDF Bin Scan on : " +
            lFileNameSTDF;
        UpdateLogError(lLocalErrorMessage,
                       lLocalErrorMessageLine,
                       ! bImportFromRemoteDB);
    }

    // If File imported automatically (Examinator-Monitoring DataPump),
    // may need to check Yield
    if(ptTask)
    {
        // List of Good Bins
        pGlobalYieldBinCheck =
            new GS::QtLib::Range(ptTask->GetProperties()->strYieldBinsList.
                                 toLatin1().constData());
        if(ptTask->GetProperties()->bCheckYield)
        {
            bCheckYieldLevel = true;
        }
        else
        {
            bCheckYieldLevel = false;
        }

        // Check if we should check for PASS Hard bins not in specified list
        if(ptTask->GetProperties()->bRejectFilesOnPassBinlist)
        {
            bCheckPassBinNotInList = true;
            clRange_PassBinlistForRejectTest.SetRange(
                ptTask->GetProperties()->strPassBinListForRejectTest);
        }
    }
    else
    {
        pGlobalYieldBinCheck = new GS::QtLib::Range("1-1");
        bCheckYieldLevel = false;
    }

    // Read all file
    int     iInvalidRecord=0;
    //int     iCustomRecord=0;
    long    lTotalRecordsScanned=0;
    bool    bMirFound = false, bStdfCorrupted = false;
    bool    bFailedValidationStep = false;
    //BYTE    bTestingSite;
    QList<int> cSitesList;  // Holds list of testing sites
    cSitesList.clear();
    QString lErrorMessage;
    dbKeysEngine.dbKeysContent().Set("TotalRecordsScanned", QVariant((int)0));

    lResult =
        GS::QtLib::DatakeysLoader::Load(lFileNameSTDF,
                                        dbKeysEngine.dbKeysContent(),
                                        lErrorMessage,
                                        &bCheckPassBinNotInList,
                                        &clRange_PassBinlistForRejectTest,
                                        pGlobalYieldBinCheck,
                                        true);

    lTotalRecordsScanned =
        dbKeysEngine.dbKeysContent().Get("TotalRecordsScanned").toInt();
    bMirFound = (dbKeysEngine.dbKeysContent().
                 StdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MIR] > 0);

    if(!lResult)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("failed to load  %1 with error : %2").
              arg(lFileNameSTDF).arg(lErrorMessage).
              toLatin1().constData());
        GSET_ERROR2(DatabaseEngine, eFailedToLoadDataKeys, NULL,
                    lFileNameSTDF.toLatin1().constData(),
                    lErrorMessage.toLatin1().constData());
        SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
        UpdateLogError(strErrorMessage,
                       lLocalErrorMessage,
                       ! bImportFromRemoteDB);
        return Failed;
    }

    lSBRTotalPartsAllSites =
        dbKeysEngine.dbKeysContent().Get("SBRTotalPartsAllSites").toLongLong();
    lSBRTotalParts =
        dbKeysEngine.dbKeysContent().Get("SBRTotalParts").toLongLong();
    lSBRTotalGoodBins =
        dbKeysEngine.dbKeysContent().Get("SBRTotalGoodBins").toLongLong();
    if(lSBRTotalGoodBins == 0)
        lSBRTotalGoodBins =
            dbKeysEngine.dbKeysContent().Get("SBRTotalGoodBinsAllSites").toLongLong();
    lHBRTotalPartsAllSites =
        dbKeysEngine.dbKeysContent().Get("HBRTotalPartsAllSites").toLongLong();
    lHBRTotalParts =
        dbKeysEngine.dbKeysContent().Get("HBRTotalParts").toLongLong();
    lHBRTotalGoodBins =
        dbKeysEngine.dbKeysContent().Get("HBRTotalGoodBins").toLongLong();
    if(lHBRTotalGoodBins == 0)
        lHBRTotalGoodBins =
            dbKeysEngine.dbKeysContent().Get("HBRTotalGoodBinsAllSites").toLongLong();
    nPassBinNotInList =
        dbKeysEngine.dbKeysContent().Get("PassBinNotInList").toInt();
    strPassBinNotInList =
        dbKeysEngine.dbKeysContent().Get("PassBinNotInListStr").toString();
    lTotalGoodBins =
        dbKeysEngine.dbKeysContent().Get("TotalGoodBins").toLongLong();
    lTotalParts = dbKeysEngine.dbKeysContent().Get("TotalParts").toLongLong();

    QString sites;
    foreach(int s, cSitesList)
        sites+=QString::number(s)+" ";
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Import file : %1 sites found in this file : %2").
          arg(cSitesList.size()).
          arg(sites.toLatin1().data()).
          toLatin1().constData());

    // LoadConfigKeys
    t_SetupTime = dbKeysEngine.dbKeysContent().Get("SetupTime").toLongLong();
    t_StartTime = dbKeysEngine.dbKeysContent().Get("StartTime").toLongLong();
    t_FinishTime = dbKeysEngine.dbKeysContent().Get("FinishTime").toLongLong();
    if (! LoadConfigKeys(dbKeysEngine, ptTask, lEditHeaderInfo,
                         bFailedValidationStep,
                         lLocalErrorMessage, m_strInsertionShortErrorMessage,
                         lFileNameSTDF))
    {
        // Syntax error from config.gexdbkeys file
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("failed to load config keys : %1").
              arg(m_strInsertionShortErrorMessage).toLatin1().constData());
        // Update error log
        // (lLocalErrorMessage already set by LoadConfigKeys,
        // m_strInsertionShortErrorMessage will be overwritten)
        lStatus = bFailedValidationStep ? FailedValidationStep : Delay;
        GSET_ERROR2(DatabaseEngine, eFailedToLoadConfigKeysFile, NULL,
                    lFileNameSTDF.toLatin1().constData(),
                    lLocalErrorMessage.toLatin1().constData());
        SetImportError(dbKeysEngine, lLocalErrorMessage, lStatus);
        UpdateLogError(strErrorMessage,
                       lLocalErrorMessage,
                       ! bImportFromRemoteDB);
        // Remove data file in temporary folder
        GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
        return lStatus;
    }

    // Check if any pre insertion script to run
    bool bIsImportIntoRDB = (pDatabaseEntry->m_pExternalDatabase != NULL) &&
        (bImportFromRemoteDB == false);
    if (bIsImportIntoRDB)
    {
        lStatus = this->RunPreInsertionScript(strErrorMessage,
                                              dbKeysEngine,
                                              ptTask,
                                              bImportFromRemoteDB,
                                              lLocalErrorMessage);
        if (lStatus != Passed)
        {
            GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
            return lStatus;
        }
    }

    {
        QString strError;
        // Check Overloaded KeyWords
        this->CheckOverloadedKeyWords(t_SetupTime, t_StartTime, t_FinishTime,
                                      dbKeysEngine, strError);

        if (strError.isEmpty() &&
            (dbKeysEngine.dbKeysContent().IsOverloaded("WaferNotch")) &&
            ! dbKeysEngine.dbKeysContent().
            Get("WaferNotch").toString().isEmpty() &&
            ! QString(" DULR").contains(dbKeysEngine.dbKeysContent().
                                        Get("WaferNotch").toString()))
        {
            strError = "Invalid value for WaferNotch : " +
                QString(dbKeysEngine.dbKeysContent().Get("WaferNotch").
                        toString().toLatin1());
        }

        // Check if overloaded Notch is valid
        if(!strError.isEmpty())
        {
            GSET_ERROR4(DatabaseEngine, eConfigkeysSyntaxError, NULL,
                        dbKeysEngine.dbKeysContent().
                        Get("ConfigFileName").toString().toLatin1().constData(),
                        strError.toLatin1().constData(),
                        lFileNameSTDF.toLatin1().constData(),
                        dbKeysEngine.dbKeysContent().
                        Get("SourceArchive").toString().toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, Delay);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            // Remove data file in temporary folder
            GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
            return Delay;
        }
    }

    if(lLogTaskDetails)
    {
        lLocalErrorMessageLine = "[Task Details] STDF Bin Scan completed.";
        UpdateLogError(lLocalErrorMessage,
                       lLocalErrorMessageLine,
                       ! bImportFromRemoteDB);
    }

    // Check if file holds corrupted records
    if(bStdfCorrupted)
    {
        if(iInvalidRecord > 0)
        {
            lLocalErrorMessage.sprintf("Includes %d unknown records",
                                       iInvalidRecord);
        }
        else
        {
            lLocalErrorMessage = "Unexpected end of file";
        }
        GSET_ERROR3(DatabaseEngine, eStdfCorrupted, NULL,
                    lFileNameSTDF.toLatin1().constData(),
                    dbKeysEngine.dbKeysContent().
                    Get("SourceArchive").toString().toLatin1().constData(),
                    lLocalErrorMessage.toLatin1().constData());
        SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
        UpdateLogError(strErrorMessage,
                       lLocalErrorMessage,
                       ! bImportFromRemoteDB);
        // Remove data file in temporary folder
        GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
        return Failed;  // Error. Can't open STDF file
    }

    // Check if read some records in the file (if not, the file is corrupted)
    if(lTotalRecordsScanned<=0 || !bMirFound)
    {
        if(lTotalRecordsScanned <=0)
        {
            lLocalErrorMessage =
                "Unable to detect records (file incomplete, "
                "or FTP not performed in 'binary' mode).";
        }
        else if(!bMirFound)
        {
            lLocalErrorMessage = "MIR record missing.";
        }
        else
        {
            lLocalErrorMessage = "Unknown.";
        }
        GSET_ERROR3(DatabaseEngine, eStdfCorrupted, NULL,
                    lFileNameSTDF.toLatin1().constData(),
                    dbKeysEngine.dbKeysContent().
                    Get("SourceArchive").toString().toLatin1().constData(),
                    lLocalErrorMessage.toLatin1().constData());
        SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
        UpdateLogError(strErrorMessage,
                       lLocalErrorMessage,
                       ! bImportFromRemoteDB);
        // Remove data file in temporary folder
        GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
        return Failed;  // Error. Can't open STDF file
    }

    // Update exact part count
    // (as it may be split per site, or merged: head=255)
    lSBRTotalParts = gex_max(lSBRTotalParts,lSBRTotalPartsAllSites);
    lHBRTotalParts = gex_max(lHBRTotalParts,lHBRTotalPartsAllSites);

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("run filed overload (config.gexdbkeys) : "
                  "SBRTotalParts=%1d HBRTotalParts=%2d").
          arg(lSBRTotalParts).
          arg(lHBRTotalParts).
          toLatin1().constData());

    // Delete temporary buffers created
    if (pGlobalYieldBinCheck)
    {
        delete pGlobalYieldBinCheck;
    }
    pGlobalYieldBinCheck = 0;

    // If Examinator-Monitoring is the caller, check yield ?
    bool bYieldAlarm = false;
    int	iAlarmIfOverLimit=0;

    // If no PRR data or fewer than summary, use summary
    if(lTotalParts < gex_max(lHBRTotalParts,lSBRTotalParts))
    {
        if(lHBRTotalParts > lSBRTotalParts)
        {
            // Use HBR data!
            lTotalParts = lHBRTotalParts;
            lTotalGoodBins = gex_max(lTotalGoodBins,lHBRTotalGoodBins);
        }
        else
        {
            // Use SBR data!
            lTotalParts = lSBRTotalParts;
            lTotalGoodBins = gex_max(lTotalGoodBins,lSBRTotalGoodBins);
        }
    }

    // If File imported automatically (Examinator-Monitoring DataPump),
    // may need to check Yield
    if(lMonitoring && ptTask)
    {
        // If has to check the standard global yield (data Pump)
        if ((bCheckYieldLevel) &&
            (lTotalParts > ptTask->GetProperties()->lMinimumyieldParts))
        {
            fYieldLevel = (100.0f*lTotalGoodBins)/(float)lTotalParts;
        }
        else
        {
            fYieldLevel = 200;  // 200%, so won't fail following tests

        }
        if(fYieldLevel < ptTask->GetProperties()->iAlarmLevel)
        {
            bYieldAlarm = true;
        }

        // Check if file has enough parts to be inserted
        unsigned int uiMinParts =
            ptTask->GetProperties()->uiRejectSmallSplitlots_NbParts;
        if ((ptTask->GetProperties()->bRejectSmallSplitlots_NbParts) &&
            (lTotalParts < (long) uiMinParts))
        {
            lLocalErrorMessage =
                QString("Total parts (%1) < required parts (%2)").
                arg(lTotalParts).arg(uiMinParts);
            GSET_ERROR3(DatabaseEngine, eTooFewParts, NULL,
                        lFileNameSTDF.toLatin1().constData(),
                        dbKeysEngine.dbKeysContent().
                        Get("SourceArchive").toString().toLatin1().constData(),
                        lLocalErrorMessage.toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            // Erase intermediate source created at root of database folder
            GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
            return Failed;	// Rename file to '.bad'
        }

        uiMinParts = (unsigned int)
            (ptTask->GetProperties()->lfRejectSmallSplitlots_GdpwPercent *
             (double) dbKeysEngine.dbKeysContent().
             Get("GrossDie").toDouble());
        if ((ptTask->GetProperties()->bRejectSmallSplitlots_GdpwPercent) &&
            (lTotalParts < (long) uiMinParts))
        {
            lLocalErrorMessage =
                QString("Total parts (%1) < ratio (%2) * GrossDie (%3)").
                arg(lTotalParts).
                arg(ptTask->GetProperties()->lfRejectSmallSplitlots_GdpwPercent,
                    0, 'f', 2).
                arg(dbKeysEngine.dbKeysContent().Get("GrossDie").toUInt());
            GSET_ERROR3(DatabaseEngine, eTooFewParts, NULL,
                        lFileNameSTDF.toLatin1().constData(),
                        dbKeysEngine.dbKeysContent().
                        Get("SourceArchive").toString().toLatin1().constData(),
                        lLocalErrorMessage.toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            // Erase intermediate source created at root of database folder
            GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
            return Failed;	// Rename file to '.bad'
        }

        // Check if file should be rejected due to
        // PASS Bin not in specified list
        if(bCheckPassBinNotInList && (nPassBinNotInList != -1))
        {
            lLocalErrorMessage = QString("HBinNo = %1, HBinName = %2").
                arg(nPassBinNotInList).
                arg(strPassBinNotInList);
            GSET_ERROR3(DatabaseEngine, eUnexpectedPassHardBins, NULL,
                        lFileNameSTDF.toLatin1().constData(),
                        dbKeysEngine.dbKeysContent().
                        Get("SourceArchive").toString().toLatin1().constData(),
                        lLocalErrorMessage.toLatin1().constData());
            SetImportError(dbKeysEngine, lLocalErrorMessage, Failed);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            // Erase intermediate source created at root of database folder
            GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
            return Failed;	// Rename file to '.bad'
        }

        // Check if raw test results should be inserted
        if ((ptTask->GetProperties()->nMaxPartsForTestResultInsertion >= 0) &&
            ((lTotalParts == 0) ||
             (lTotalParts >
              ptTask->GetProperties()->nMaxPartsForTestResultInsertion)))
        {
            dbKeysEngine.dbKeysContent().SetInternal("IgnoreResults",
                                                     "TRUE");
        }
    }

    // DATABASE INSERTION
    if(bIsImportIntoRDB)
    {
        lTimer.start();
        lStatus = this->DatabaseInsertFile(NULL,
                                           -1,
                                           pDatabaseEntry,
                                           lFileNameSTDF,
                                           strErrorMessage,
                                           dbKeysEngine,
                                           ptTask,
                                           bImportFromRemoteDB);
        unsigned int uiInsertionTime = lTimer.elapsed() / 1000;
        dbKeysEngine.dbKeysContent().SetInternal("TimeUncompress",
                                                 *puiUncompressTime);
        dbKeysEngine.dbKeysContent().SetInternal("TimeConvertion",
                                                 *puiConvertTime);
        dbKeysEngine.dbKeysContent().SetInternal("TimeInsertion",
                                                 uiInsertionTime);
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Import file: Status=%1, UncompressTime=%2, "
                      "ConvertionTime=%3, InsertionTime=%4").
              arg(lStatus).
              arg(*puiUncompressTime).
              arg(*puiConvertTime).
              arg(uiInsertionTime).
              toLatin1().constData());

        if (lStatus != Passed)
        {
            return lStatus;
        }
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Executing Monitoring tasks (bMonitoring =%1)...").
          arg(lMonitoring ? "true" : "false").toLatin1().constData());

    bool bCompleteFile=true;
    CGexGroupOfFiles *pGroup = NULL;
    CGexFileInGroup *pFile = NULL;
    // If Monitoring enabled, perform relevant tasks
    // Get task pointers
    // (with eventual new Product IDs overloaded in insertion procedure)
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("Getting monitoring tasks if any for product '%1'...").
              arg(dbKeysEngine.dbKeysContent().Get("Product").toString()).
              toLatin1().constData());

        if(ptTask)
        {
            GSLOG(SYSLOG_SEV_NOTICE,
                  "Searching for any Yield, "
                  "SYA and specs tasks for that product...");

            ptYieldTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().
                GetProductYieldInfo(dbKeysEngine.dbKeysContent());

            dbKeysEngine.dbKeysContent().Set("TaskType", "SPM");
            lSpmTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductStatMonTask(dbKeysEngine.dbKeysContent());

            dbKeysEngine.dbKeysContent().Set("TaskType", "SYA");
            lSyaTasks = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductStatMonTask(dbKeysEngine.dbKeysContent());

            // Reset
            dbKeysEngine.dbKeysContent().Set("TaskType", QVariant());
        }
    }

    // For each of these tasks, we need the gexReport
    if(bYieldAlarm || ptYieldTask || (lSpmTask.isEmpty() == false) || (lSyaTasks.isEmpty() == false))
    {
        GSLOG(5, "Analyzing file...");
        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine = "[Task Details] Data analysis for: " +
                lFileNameSTDF;
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }

        // 1) Analyze data file: If multi-sites ensure to COMPARE each site
        lLocalErrorMessageLine =
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().
                ExecuteDataFileAnalysis(dbKeysEngine.dbKeysContent(), cSitesList);

        // Check for error during analysis
        if(lLocalErrorMessageLine.isEmpty() == false)
        {
            GSET_ERROR3(DatabaseEngine, eFailedToAnalyzeFile, NULL,
                        lFileNameSTDF.toLatin1().constData(),
                        dbKeysEngine.dbKeysContent().
                        Get("SourceArchive").toString().toLatin1().constData(),
                        lLocalErrorMessageLine.toLatin1().constData());
            AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
            m_strInsertionShortErrorMessage = GGET_LASTERRORDESC(DatabaseEngine,
                                                                 this);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            return bIsImportIntoRDB ? Passed : Delay;
        }

        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine = "[Task Details] Data analysis done.";
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }

        // Tell if this file is complete or not
        // (complete means: it has a MRR record)
        if (!gexReport || gexReport->getGroupsList().size()<1)
        {
            pGroup=NULL;
        }
        else
        {
            pGroup = gexReport->getGroupsList().first();
        }

        if(pGroup == NULL)
        {
            GSET_ERROR3(DatabaseEngine, eFailedToAnalyzeFile, NULL,
                        lFileNameSTDF.toLatin1().constData(),
                        dbKeysEngine.dbKeysContent().
                        Get("SourceArchive").toString().toLatin1().constData(),
                        "");
            AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
            m_strInsertionShortErrorMessage = GGET_LASTERRORDESC(DatabaseEngine,
                                                                 this);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            return bIsImportIntoRDB ? Passed : Delay;
        }

        pFile =
            (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        bCompleteFile = pFile->getMirDatas().bMrrRecord;

        // 2) Execute Yield monitoring tasks
        if(ptYieldTask != NULL)
        {
            if(lLogTaskDetails)
            {
                lLocalErrorMessageLine =
                    "[Task Details] Check for Yield alarm.";
                UpdateLogError(lLocalErrorMessage,
                               lLocalErrorMessageLine,
                               ! bImportFromRemoteDB);
            }
            QString s =
                GS::Gex::Engine::GetInstance().GetSchedulerEngine().
                ExecuteYieldTask(pDatabaseEntry,
                                 ptYieldTask, dbKeysEngine.dbKeysContent());
            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("ExecuteYieldTask returned '%1'").arg(s).
                  toLatin1().constData());
            if(lLogTaskDetails)
            {
                lLocalErrorMessageLine =
                    "[Task Details] Yield alarm check done.";
                UpdateLogError(lLocalErrorMessage,
                               lLocalErrorMessageLine,
                               ! bImportFromRemoteDB);
            }
        }

        // 4) Execute SPM monitoring tasks
        if(lSpmTask.isEmpty() == false )
        {
            GSLOG(6, "Check SPM...");
            if(lLogTaskDetails)
            {
                lLocalErrorMessageLine =
                    "[Task Details] Check for Stat Parameter alarm for file "+ dbKeysEngine.dbKeysContent().Get("SourceArchive").toString();
                UpdateLogError(lLocalErrorMessage,
                               lLocalErrorMessageLine,
                               ! bImportFromRemoteDB);
            }

            QList<CGexMoTaskStatisticalMonitoring*>::Iterator lIter(lSpmTask.begin());
            QList<CGexMoTaskStatisticalMonitoring*>::Iterator lIterEnd(lSpmTask.end());

            for(; lIter != lIterEnd; ++lIter)
            {
                strErrorMessage =
                    GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteStatMonTask((*lIter), dbKeysEngine.dbKeysContent());
                if(!strErrorMessage.isEmpty())
                {
                    // Display warning message
                    lLocalErrorMessageLine = "Insertion warning(s): ";
                    if(!lLogTaskDetails)
                    {
                        lLocalErrorMessageLine += "\n\tFile: " + dbKeysEngine.dbKeysContent().Get("BaseSourceArchive").toString();
                        lLocalErrorMessageLine += "\n\tWarning: ";
                    }
                    lLocalErrorMessageLine += strErrorMessage.section(":",1).simplified();

                    UpdateLogError(lLocalErrorMessage,
                                   lLocalErrorMessageLine,
                                   ! bImportFromRemoteDB);
                }
            }
            if(lLogTaskDetails)
            {
                lLocalErrorMessageLine =
                    "[Task Details] Stat Parameter alarm check done.";
                UpdateLogError(lLocalErrorMessage,
                               lLocalErrorMessageLine,
                               ! bImportFromRemoteDB);
            }
        }

        // 5) Execute SYA monitoring tasks
        if(lSyaTasks.isEmpty() == false)
        {
            GSLOG(6, "Check SYA...");
            if(lLogTaskDetails)
            {
                lLocalErrorMessageLine =
                    "[Task Details] Check for Stat Parameter alarm for file "+ dbKeysEngine.dbKeysContent().Get("SourceArchive").toString();
                UpdateLogError(lLocalErrorMessage,
                               lLocalErrorMessageLine,
                               ! bImportFromRemoteDB);
            }

            QList<CGexMoTaskStatisticalMonitoring*>::Iterator lIter(lSyaTasks.begin());
            QList<CGexMoTaskStatisticalMonitoring*>::Iterator lIterEnd(lSyaTasks.end());

            for(; lIter != lIterEnd; ++lIter)
            {
                strErrorMessage =
                    GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteStatMonTask(*lIter, dbKeysEngine.dbKeysContent());
                if(!strErrorMessage.isEmpty())
                {
                    // Display warning message
                    lLocalErrorMessageLine = "Insertion warning(s): ";
                    if(!lLogTaskDetails)
                    {
                        lLocalErrorMessageLine += "\n\tFile: " + dbKeysEngine.dbKeysContent().Get("BaseSourceArchive").toString();
                        lLocalErrorMessageLine += "\n\tWarning: ";
                    }
                    lLocalErrorMessageLine += strErrorMessage.section(":",1).simplified();

                    UpdateLogError(lLocalErrorMessage,
                                   lLocalErrorMessageLine,
                                   ! bImportFromRemoteDB);
                }

            }
            if(lLogTaskDetails)
            {
                lLocalErrorMessageLine =
                    "[Task Details] Stat Parameter alarm check done.";
                UpdateLogError(lLocalErrorMessage,
                               lLocalErrorMessageLine,
                               ! bImportFromRemoteDB);
            }
        }

        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine = "[Task Details] Report done.";
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }
    }

    // Sets flag to say if we succeeded reading the STDF file
    // (and so can now create it's summary version)
    bool bCreateSummaryFile = false;

    // FILE-BASED
    if(!pDatabaseEntry->IsExternal())
    {
        // Have the STDF file analysed so we can create a summary version
        // Unless Corporate database: then NO summary can be created
        // (would cause a crash because the FTP script is already running)
        QString strScriptFile;

        // Create script that will read data file +
        // compute all statistics (but NO report created)
        strScriptFile = GS::Gex::Engine::GetInstance().GetAssistantScript();
        FILE *hFile = fopen(strScriptFile.toLatin1().constData(),"w");

        if(hFile == NULL)
        {
            GSET_ERROR1(DatabaseEngine, eFailedToCreateScriptFile, NULL,
                        strScriptFile.toLatin1().constData());
            AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
            m_strInsertionShortErrorMessage =
                GGET_LASTERRORDESC(DatabaseEngine, this);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            return bIsImportIntoRDB ? Passed : Delay;
        }

        // Creates 'SetOptions' section
        if(!ReportOptions.WriteOptionSectionToFile(hFile))
        {
            GEX_ASSERT(false);
            GSET_ERROR2(DatabaseEngine, eFailedToWriteScriptFunction, NULL,
                        "SetOptions()", strScriptFile.toLatin1().constData());
            AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
            m_strInsertionShortErrorMessage =
                GGET_LASTERRORDESC(DatabaseEngine, this);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            return bIsImportIntoRDB ? Passed : Delay;
        }

        // Creates 'SetProcessData' section
        fprintf(hFile,"SetProcessData()\n");
        fprintf(hFile,"{\n");
        fprintf(hFile,"  var group_id;\n");
        fprintf(hFile,"  gexGroup('reset','all');\n");
        fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");
        ConvertToScriptString(lFileNameSTDF);
        fprintf(hFile,
                "  gexFile(group_id,'insert','%s','All','all',' ','');\n",
                lFileNameSTDF.toLatin1().constData());
        fprintf(hFile,"}\n\n");

        // Creates 'main' section
        fprintf(hFile,"main()\n");
        fprintf(hFile,"{\n");
        // Nowadays, Gex will write only the latest version of the csl format.
        // fprintf(hFile,"  gexCslVersion('%1.2f');\n\n", GEX_MAX_CSL_VERSION);
        fprintf(hFile,"  SetOptions();\n");
        fprintf(hFile,
                "  gexOptions('statistics','computation',"
                "'summary_then_samples');\n");  // consider TSR Summary records
        // Avoids editor to be launched if output options is
        // set to Word or Excel
        fprintf(hFile, "  gexOptions('output','format','html');\n");
        fprintf(hFile,
                "  gexReportType('histogram','test_over_limits','all');\n");
        fprintf(hFile,"  SetProcessData();\n");
        // Only data analysis, no report created
        fprintf(hFile, "  gexOptions('report','build','false');\n");
        // Show 'Database Admin' page after processing file
        fprintf(hFile, "  gexBuildReport('admin','0');\n");
        fprintf(hFile,"}\n\n");
        fclose(hFile);

        // Execute script
        if (GS::Gex::CSLEngine::GetInstance().
            RunScript(strScriptFile).IsFailed() == false)
        {
            bCreateSummaryFile = true;
    }
    }

    // Debug trace
    lMessage = "Update Intranet Website tables...";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data());

    // Update Monitoring log file (used later for Report generation)
    if(lMonitoring && bCompleteFile)
    {
        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine = "[Task Details] Updating Intranet tables";
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }

        // File: <database_physical_path>/.monitor/YYYYMMDD.dat
        QString		strMonitoringPath;
        strMonitoringPath = pDatabaseEntry->PhysicalPath() + QDir::separator();
        strMonitoringPath += GEXMO_STATUS_FOLDER;
        lDir.mkdir(strMonitoringPath);

        QDateTime cCurrentDateTime;
        cCurrentDateTime = GS::Gex::Engine::GetInstance().GetClientDateTime();
        strMonitoringPath += "/" + cCurrentDateTime.toString("yyyyMMdd");
        strMonitoringPath += ".dat";

        // Update data file to include Sub-Lot info just processed
        // Write All Data files info :
        // date, Product, LotID, SublotID, Tester name, etc.
        // WARNING: If the order of the following parameters is changed,
        // then update the relevant variables 'GEXMO_LOT_PROCESSED_xxx' in
        // 'gex_constants.h'
        QString	strInsertionDetails;
        strInsertionDetails =
            QString::number(dbKeysEngine.dbKeysContent().Get(
                                "StartTime").toULongLong()) + ",";

        if (dbKeysEngine.dbKeysContent().
            Get("Product").toString().isEmpty() == false)
        {
            strInsertionDetails +=
                dbKeysEngine.dbKeysContent().Get("Product").toString();
        }
        strInsertionDetails += ",";

        if (dbKeysEngine.dbKeysContent().
            Get("Lot").toString().isEmpty() == false)
        {
            strInsertionDetails +=
                dbKeysEngine.dbKeysContent().Get("Lot").toString();
        }
        strInsertionDetails += ",";

        if (dbKeysEngine.dbKeysContent().
            Get("SubLot").toString().isEmpty() == false)
        {
            strInsertionDetails +=
                dbKeysEngine.dbKeysContent().Get("SubLot").toString();
        }
        strInsertionDetails += ",";

        if (dbKeysEngine.dbKeysContent().
            Get("TesterName").toString().isEmpty() == false)
        {
            strInsertionDetails +=
                dbKeysEngine.dbKeysContent().Get("TesterName").toString();
        }
        strInsertionDetails += ",";

        if (dbKeysEngine.dbKeysContent().
            Get("Operator").toString().isEmpty() == false)
        {
            strInsertionDetails +=
                dbKeysEngine.dbKeysContent().Get("Operator").toString();
        }
        strInsertionDetails += ",";

        if (dbKeysEngine.dbKeysContent().
            Get("ProgramName").toString().isEmpty() == false)
        {
            strInsertionDetails +=
                dbKeysEngine.dbKeysContent().Get("ProgramName").toString();
        }
        strInsertionDetails += ",";

        if (dbKeysEngine.dbKeysContent().
            Get("ProgramRevision").toString().isEmpty() == false)
        {
            strInsertionDetails +=
                dbKeysEngine.dbKeysContent().Get("ProgramRevision").toString();
        }
        strInsertionDetails += ",";

        strInsertionDetails += QString::number(lTotalGoodBins) + ",";
        strInsertionDetails += QString::number(lTotalParts) + ",";

        if (dbKeysEngine.dbKeysContent().
            Get("Wafer").toString().isEmpty() == false)
        {
            strInsertionDetails +=
                dbKeysEngine.dbKeysContent().Get("Wafer").toString();
        }

        // Check if Insertion history file already exists
        QFile f(strMonitoringPath);
        QTextStream hDataFilesProcessed(&f);
        bool	bUpdateDatFile=true;
        bool	bNewMonitoringFile;
        if(f.exists())
        {
            // If file exists, check if file inserted
            // already listed in history file
            bNewMonitoringFile = false;

            if(!f.open(QIODevice::ReadOnly))
            {
                GSET_ERROR1(DatabaseEngine, eFailedToReadIndexFile, NULL,
                            strMonitoringPath.toLatin1().constData());
                AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
                m_strInsertionShortErrorMessage =
                    GGET_LASTERRORDESC(DatabaseEngine, this);
                UpdateLogError(strErrorMessage,
                               lLocalErrorMessage,
                               ! bImportFromRemoteDB);
                return bIsImportIntoRDB ? Passed : Delay;
            }

            // Read file to see if any line already matches
            // the one to insert (if so, do not added it again)
            hDataFilesProcessed.setDevice(&f);
            do
            {
                // Read one line from file
                if(hDataFilesProcessed.readLine() == strInsertionDetails)
                {
                    // This file was already previously inserted,
                    // so do not duplicate this entry inn the Web status page
                    bUpdateDatFile = false;
                    break;
                }
            }
            while(hDataFilesProcessed.atEnd() == false);
            f.close();
        }
        else
        {
            bNewMonitoringFile = true;  // First file inserted today

        }
        // Add details about file just inserted in DAT file,
        // so Web status pages can be updated accordingly
        if(bUpdateDatFile)
        {
            // Open CSV Parameter table file
            if(!f.open(QIODevice::WriteOnly | QIODevice::Append))
            {
                GSET_ERROR1(DatabaseEngine, eFailedToCreateIndexFile, NULL,
                            strMonitoringPath.toLatin1().constData());
                AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
                m_strInsertionShortErrorMessage =
                    GGET_LASTERRORDESC(DatabaseEngine, this);
                UpdateLogError(strErrorMessage,
                               lLocalErrorMessage,
                               ! bImportFromRemoteDB);
                return bIsImportIntoRDB ? Passed : Delay;
            }
            hDataFilesProcessed.setDevice(&f);

            // If new file, write XML header!
            if(bNewMonitoringFile == true)
            {
                hDataFilesProcessed << "<monitoring_lots>" << endl;
            }
            hDataFilesProcessed << strInsertionDetails << endl;
            f.close();
        }

        // Update config file: keep track of time of last data file processed
        // (check this time to decide if new HTML report has to be built)
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().WriteConfigFile();

        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine = "[Task Details] Intranet tables updated.";
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }

    } // Caller is Examinator-Monitoring

    if((bYieldAlarm == true) && ptTask && gexReport)
    {
        lMessage = "Import File: send email notification";
        GSLOG(SYSLOG_SEV_DEBUG, lMessage.toLatin1().data());

        // Data file with Yield level under the limit,
        // send Email notification
        QString strEmailBody;

        QString strFrom,strTo,strSubject;
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        {
            strFrom = GEX_EMAIL_PAT_MAN;
        }
        else
        {
            strFrom = GEX_EMAIL_YIELD_MAN;
        }
        if ( ptTask->GetProperties()->strEmailFrom.isEmpty() == false )
        {
            strFrom = ptTask->GetProperties()->strEmailFrom;
        }
        strTo = ptTask->GetProperties()->strEmailNotify;
        strSubject = QString("Yield Alarm: ") + ptTask->GetProperties()->strTitle;

        GexMoBuildEmailString cEmailString;
        // 'true' if email to be sent in HTML format
        bool bHtmlEmail = ptTask->GetProperties()->bHtmlEmail;

        QString strYield = QString::number(fYieldLevel,'f',2) + QString("% ");
        if(iAlarmIfOverLimit == 0)
        {
            strYield += "< ";
        }
        else
        {
            strYield += "> ";
        }
        strYield +=
            QString::number(ptTask->GetProperties()->iAlarmLevel, 'f', 2) + "%";

        time_t lStartTime = dbKeysEngine.dbKeysContent().Get("StartTime").toULongLong();

        if(bHtmlEmail)
        {
            // HTML Header
            cEmailString.CreatePage(strSubject);

            // Table with Lot Info
            cEmailString.WriteHtmlOpenTable();

            // Write table
            cEmailString.WriteInfoLine("Testing date", TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss"));
            cEmailString.WriteInfoLine("Product",
                                       dbKeysEngine.dbKeysContent().
                                       Get("Product").toString());
            cEmailString.WriteInfoLine("Lot",
                                       dbKeysEngine.dbKeysContent().
                                       Get("Lot").toString());
            cEmailString.WriteInfoLine("SubLot",
                                       dbKeysEngine.dbKeysContent().
                                       Get("SubLot").toString());
            if(pFile != NULL)
            {
                cEmailString.WriteInfoLine("Wafer ID",
                                           pFile->getWaferMapData().szWaferID);
            }
            // Highlight result in RED as it is a yield Alarm
            cEmailString.WriteInfoLine("Yield Level", strYield, true);
            cEmailString.WriteInfoLine();

            if (dbKeysEngine.dbKeysContent().
                Get("TesterName").toString().isEmpty() == false)
            {
                cEmailString.WriteInfoLine("Tester",
                                           dbKeysEngine.dbKeysContent().
                                           Get("TesterName").toString());
            }

            if (dbKeysEngine.dbKeysContent().
                Get("Operator").toString().isEmpty() == false)
            {
                cEmailString.WriteInfoLine("Operator",
                                           dbKeysEngine.dbKeysContent().
                                           Get("Operator").toString());
            }

            if (dbKeysEngine.dbKeysContent().
                Get("ProgramName").toString().isEmpty() == false)
            {
                cEmailString.WriteInfoLine("Program name",
                                           dbKeysEngine.dbKeysContent().
                                           Get("ProgramName").toString());
            }

            if (dbKeysEngine.dbKeysContent().
                Get("ProgramRevision").toString().isEmpty() == false)
            {
                cEmailString.WriteInfoLine("Program revision",
                                           dbKeysEngine.dbKeysContent().
                                           Get("ProgramRevision").toString());
            }

            cEmailString.WriteInfoLine("Total parts tested",
                                       QString::number(lTotalParts));
            cEmailString.WriteInfoLine("Total good parts",
                                       QString::number(lTotalGoodBins));
            cEmailString.WriteInfoLine("Total failing parts",
                                       QString::number(lTotalParts -
                                                       lTotalGoodBins));

            // Table with Lot Info
            cEmailString.WriteHtmlCloseTable();
        }
        else
        {
            // Email in Plain Text
            strEmailBody =
                "\n###############################"
                "################################\n";
            strEmailBody += QString("Testing date: ") + TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss") + QString("\n");
            strEmailBody += QString("Product     : ") +
                dbKeysEngine.dbKeysContent().Get("Product").toString() +
                QString("\n");
            strEmailBody += QString("Lot         : ") +
                dbKeysEngine.dbKeysContent().Get("Lot").toString() +
                QString("\n");
            strEmailBody += QString("SubLot      : ") +
                dbKeysEngine.dbKeysContent().Get("SubLot").toString() +
                QString("\n");
            strEmailBody += QString("Yield Level : ") + strYield;
            strEmailBody += "\n";
            strEmailBody +=
                "#################################"
                "##############################\n\n";

            if (dbKeysEngine.dbKeysContent().
                Get("TesterName").toString().isEmpty() == false)
            {
                strEmailBody += QString("Tester             : ") +
                    dbKeysEngine.dbKeysContent().Get("TesterName").toString() +
                    QString("\n");
            }

            if (dbKeysEngine.dbKeysContent().
                Get("Operator").toString().isEmpty() == false)
            {
                strEmailBody += QString("Operator           : ") +
                    dbKeysEngine.dbKeysContent().Get("Operator").toString() +
                    QString("\n");
            }

            if (dbKeysEngine.dbKeysContent().
                Get("ProgramName").toString().isEmpty() == false)
            {
                strEmailBody += QString("Program name       : ") +
                    dbKeysEngine.dbKeysContent().Get("ProgramName").toString() +
                    QString("\n");
            }

            if (dbKeysEngine.dbKeysContent().
                Get("ProgramRevision").toString().isEmpty() == false)
            {
                strEmailBody += QString("Program revision   : ") +
                    dbKeysEngine.dbKeysContent().Get("ProgramRevision").
                    toString() + QString("\n");
            }
            strEmailBody += "Total parts tested : " +
                QString::number(lTotalParts) + QString("\n");
            strEmailBody += "Total good parts   : " +
                QString::number(lTotalGoodBins) + QString("\n");
            strEmailBody += "Total failing parts: " +
                QString::number(lTotalParts - lTotalGoodBins) + QString("\n");
        }

        // Add soft Bin summary
        bool	bMultiSiteFile = (gexReport->getGroupsList().count() > 1);
        QString strSummary;

        for (int nGroupIndex = 0;
             nGroupIndex < gexReport->getGroupsList().count();
             ++nGroupIndex)
        {
            strSummary +=
                GS::Gex::Engine::GetInstance().GetSchedulerEngine().
                GetBinSummaryString(gexReport->getGroupsList().at(nGroupIndex),
                                    lTotalParts,
                                    bHtmlEmail,
                                    bMultiSiteFile);
        }

        if(bHtmlEmail)
        {
            cEmailString.AddHtmlString(strSummary);
        }
        else
        {
            strEmailBody += strSummary;
        }

        // Close HTML email string if need be
        if(bHtmlEmail)
        {
            strEmailBody = cEmailString.ClosePage();
        }

        // Send email
        GexMoSendEmail Email;
        QString strFilePath;
        // Used to create emails in spooling folder
        CGexMoTaskStatus* ptStatusTask = NULL;
        ptStatusTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetStatusTask();

        if(ptStatusTask == NULL)
        {
            GSET_ERROR0(DatabaseEngine, eFailedToCreateEmail, NULL);
            AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
            m_strInsertionShortErrorMessage =
                GGET_LASTERRORDESC(DatabaseEngine, this);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            return Passed;
        }

        // We have a spolling folder: create email file in it
        strFilePath = ptStatusTask->GetProperties()->intranetPath() + "/";
        strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
        strFilePath += GEXMO_AUTOREPORT_EMAILS;

        Email.Send(strFilePath,
                   strFrom,
                   strTo,
                   strSubject,
                   strEmailBody,
                   ptTask->GetProperties()->bHtmlEmail);
    }

    // Or if black Hole database (no storage), then quietly return
    if(bBlackHole)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Cleaning up blackhole...");
        // Erase any local file created
        if(lMakeLocalCopy)
        {
            // Erase intermediate source created at root of database folder
            GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
        }
        return Passed;	// Success
    }

    // If Monitoring on non-completed file,
    // then display error if stringent insertion
    if (bCompleteFile == false &&
        (ReportOptions.GetOption("dataprocessing",
                                 "stdf_compliancy").toString() == "stringent"))
    {
        GSET_ERROR2(DatabaseEngine, eIncompleteFile, NULL,
                    lFileNameSTDF.toLatin1().constData(),
                    strSourceArchive.toLatin1().constData());
        AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
        m_strInsertionShortErrorMessage =
            GGET_LASTERRORDESC(DatabaseEngine, this);
        UpdateLogError(strErrorMessage,
                       lLocalErrorMessage,
                       ! bImportFromRemoteDB);
        // Erase intermediate source created at root of database folder
        GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
        return bIsImportIntoRDB ? Passed : Delay;
    }

    // DATABASE INSERTION
    // If external database, return successful code from here
    if((pDatabaseEntry->m_pExternalDatabase != NULL)
            && (bImportFromRemoteDB == false))
    {
        // Erase intermediate source created at root of database folder
        GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
        return Passed;	// Success
    }

    // FILE-BASED
    // Proprietary database: maintain index files
    // Create folder: <database>/<yyyy>/<mm>/<dd> (date data-set was created)
    QDateTime	cFileDate;
    QString		strDestinationPath,lDestinationFile;

    if(lLogTaskDetails)
    {
        lLocalErrorMessageLine =
            "[Task Details] Moving STDF file into database sub-folder";
        UpdateLogError(lLocalErrorMessage,
                       lLocalErrorMessageLine,
                       ! bImportFromRemoteDB);
    }

    // Date data-set was created
    cFileDate.setTime_t(dbKeysEngine.dbKeysContent().
                        Get("StartTime").toULongLong());
    strDestinationPath = pDatabaseEntry->PhysicalPath();
    strDestinationPath += cFileDate.toString("/yyyy");
    lDir.mkdir(strDestinationPath);

    strDestinationPath += cFileDate.toString("/MM");
    lDir.mkdir(strDestinationPath);

    strDestinationPath += cFileDate.toString("/dd");
    lDir.mkdir(strDestinationPath);

    // Move file from <database> to <database>/<yyyy>/<mm>/<dd>,
    // unless we're in storage mode = 'Link'
    lDestinationFile = strDestinationPath + lFileShortSTDF;

    if(lMakeLocalCopy == true)
    {
        // Copy file to folder only if we have to
        // (STDF file created, and storage mode=copy)
#if  unix || __MACH__
        if(CGexSystemUtils::CopyFile(lFileNameSTDF,lDestinationFile))
        {
            lStatus = 1;	// Success moving file
            // Erase intermediate source
            GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
        }
        else
        {
            lStatus = 0;
        }
#else
        // Erase destination source
        GS::Gex::Engine::RemoveFileFromDisk(lDestinationFile);
        lStatus = CGexSystemUtils::CopyFile(
                lFileNameSTDF.toLatin1().constData(),
                                            lDestinationFile.toLatin1().constData());
#endif
        // Check if file moved sucessfuly to the database
        if(!lStatus)
        {
            GSET_ERROR2(DatabaseEngine, eFailedToMoveFile, NULL,
                        lFileNameSTDF.toLatin1().constData(),
                        lDestinationFile.toLatin1().constData());
            AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
            m_strInsertionShortErrorMessage =
                GGET_LASTERRORDESC(DatabaseEngine, this);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            // Erase intermediate source created at root of database folder
            GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
            return bIsImportIntoRDB ? Passed : Delay;
        }
    }

    if(lLogTaskDetails)
    {
        lLocalErrorMessageLine =
            "[Task Details] File sucessfuly moved in database sub-folder";
        UpdateLogError(lLocalErrorMessage,
                       lLocalErrorMessageLine,
                       ! bImportFromRemoteDB);
    }

    // Check if can create the SUMMARY file (summary of the STDF file)
    unsigned long uSummaryFileSize=0;
    if(bCreateSummaryFile)
    {
        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine = "[Task Details] Building summary file";
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }

        uSummaryFileSize = BuildSummarySTDF(strSourceArchive,
                                            lMakeLocalCopy,
                                            lDestinationFile,
                                            dbKeysEngine.dbKeysContent());

        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine = "[Task Details] Summary file created";
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }
    }

    // If summary only, erase copy of file if it exists
    if(lSummaryOnly && lMakeLocalCopy)
    {
        // Erase Destination file as we didn't have to copy it
        // (well in fact we needed it to create the summary, that's all)
        GS::Gex::Engine::RemoveFileFromDisk(lDestinationFile);

        // Database size is increased by the size of the summary file only
        lFileSize = uSummaryFileSize/(1024.0*1024.0);
    }
    else
    {
        // Disk space used= File size + summary size (if created)
        lFileSize += uSummaryFileSize / (1024.0 * 1024.0);
    }
    // Check if storage is in COMPRESSED mode
    if(lZipCopy)
    {
        // Compress file when saved into database
        CArchiveFile	clZip(strDestinationPath);
        QString			strZippedFile;

        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine = "[Task Details] Zipping file inserted";
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }

        lResult = clZip.Compress(lDestinationFile, strZippedFile);
        if(lResult == true)
        {
            // Remove original file as we only keep the compressed version
            GS::Gex::Engine::RemoveFileFromDisk(lDestinationFile);

            // Get size of compressed (.gz) file created
            strZippedFile = strDestinationPath + "/" + strZippedFile;
            lFileSize = QFile(strZippedFile).size()/(1024.0*1024.0);
        }
        else
        {
            GSET_ERROR2(DatabaseEngine, eFailedToCompressFile, NULL,
                        lDestinationFile.toLatin1().constData(),
                        strZippedFile.toLatin1().constData());
            AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
            m_strInsertionShortErrorMessage =
                GGET_LASTERRORDESC(DatabaseEngine, this);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            // Remove iserted file & zip file (if possible)
            GS::Gex::Engine::RemoveFileFromDisk(lDestinationFile);
            GS::Gex::Engine::RemoveFileFromDisk(strZippedFile);
            return bIsImportIntoRDB ? Passed : Delay;
        }
        if(lLogTaskDetails)
        {
            lLocalErrorMessageLine =
                "[Task Details] Zipping inserted file completed.";
            UpdateLogError(lLocalErrorMessage,
                           lLocalErrorMessageLine,
                           ! bImportFromRemoteDB);
        }
    }

    // File moved successfuly (if was needed). Update the database index file
    if(lLogTaskDetails)
    {
        lLocalErrorMessageLine =
            "[Task Details] Updating database index tables";
        UpdateLogError(lLocalErrorMessage,
                       lLocalErrorMessageLine,
                       ! bImportFromRemoteDB);
    }

    // Check if file 'GEX_DATABASE_ENTRY_DEFINITION' exists in this folder,
    // and if so, append entry
    QString strIndexFile = strDestinationPath + GEX_DATABASE_INDEX_DEFINITION;
    QFile f;
    f.setFileName(strIndexFile);
    if(f.exists() == false)
    {
        // There is no Index file in this folder...then create it
        if(f.open(QIODevice::WriteOnly) == false)
        {
            GSET_ERROR1(DatabaseEngine,
                        eFailedToCreateIndexFile,
                        NULL,
                        strIndexFile.toLatin1().constData());
            AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
            m_strInsertionShortErrorMessage =
                GGET_LASTERRORDESC(DatabaseEngine, this);
            UpdateLogError(strErrorMessage,
                           lLocalErrorMessage,
                           ! bImportFromRemoteDB);
            // Remove imported file
            if(lMakeLocalCopy == true)
            {
                GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
            }
            return bIsImportIntoRDB ? Passed : Delay;
        }

        // Create new Database definition File
        // Assign file handle to data stream
        QTextStream hNewDefinitionFile(&f);

        // Fill file with Database details
        // Start indexes definition marker
        hNewDefinitionFile << "<indexes>" << endl;
        f.close();
    }

    // Open Index file in Append mode to append
    // the reference to our new STDF file
    if(SmartOpenFile(f, QIODevice::WriteOnly | QIODevice::Append) == false)
    {
        GSET_ERROR1(DatabaseEngine,
                    eFailedToCreateIndexFile,
                    NULL,
                    strIndexFile.toLatin1().constData());
        AddLastErrorToImportWarnings(dbKeysEngine, lLocalErrorMessage);
        m_strInsertionShortErrorMessage =
            GGET_LASTERRORDESC(DatabaseEngine, this);
        UpdateLogError(strErrorMessage,
                       lLocalErrorMessage,
                       ! bImportFromRemoteDB);
        // Remove imported file
        if(lMakeLocalCopy == true)
        {
            GS::Gex::Engine::RemoveFileFromDisk(lFileNameSTDF);
        }
        return bIsImportIntoRDB ? Passed : Delay;
    }

    // Update Database definition File
    QTextStream hDefinitionFile(&f);	// Assign file handle to data stream

    // Update database size.
    pDatabaseEntry->SetCacheSize( pDatabaseEntry->CacheSize() + lFileSize);

    // Save name of destination file
    if(lMakeLocalCopy)
    {
        strFileInserted = strDestinationPath + lFileShortSTDF;
    }
    else
    {
        strFileInserted = strSourceArchive;
    }

    // Write as many lines as WaferIDs found in the file,
    // and 1 line if none found
    QStringList lWaferIdList = dbKeysEngine.dbKeysContent().Get("WaferIdList").toString()
            .split(',', QString::KeepEmptyParts);
    if(lWaferIdList.count() == 1)
    {
        // In case there is only 1 wafer, make sure we use the WaferID eventually overloaded by the config keys file
        lWaferIdList.clear();
        lWaferIdList.append(dbKeysEngine.dbKeysContent().Get("Wafer").toString());
    }

    int	iLoop = lWaferIdList.count();
    if (iLoop <= 0)
    {
        iLoop = 1;
    }
    QStringList::Iterator itWaferID = lWaferIdList.begin();
    while(iLoop)
    {
        // Fill file with Database details
        if(lMakeLocalCopy == true)
        {
            // File is stored (copy) into the database structure,
            // so we specify a relative path
            hDefinitionFile << "C,";												// Param#0
            hDefinitionFile << lFileShortSTDF << ",";								// Param#1
        }
        else
        {
            // Only a link to the file is specified,
            // so specify full absolute path to file location
            hDefinitionFile << "L,";
            // Write 'strSourceArchive' instead of 'strFileShortSTDF' because
            // link mode on compressed files need to know the original archive
            // file, not the uncompressed reference
            hDefinitionFile << strSourceArchive << ",";
        }

        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#2
                             Get("Lot").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#3
                             Get("SubLot").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#4
                             Get("ProgramName").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#5
                             Get("ProgramRevision").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#6
                             Get("Operator").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#7
                             Get("TesterName").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#8
                             Get("TesterType").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#9
                             Get("TestingCode").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#10
                             Get("Temperature").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#11
                             Get("PackageType").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#12
                             Get("Product").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#13
                             Get("Family").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#14
                             Get("Facility").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#15
                             Get("Floor").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#16
                             Get("Process").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#17
                             Get("FrequencyStep").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#18
                             Get("BurninTime").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#19
                             Get("ProberType").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#20
                             Get("ProberName").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#21
                             Get("LoadBoardType").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#22
                             Get("LoadBoard").toString())) << ",";
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().  // Param#23
                             Get("DataOrigin").toString())) << ",";

        // WaferID
        // Param#24
        // Test if list is empty to avoid crash when we call is empty method
        if(lWaferIdList.count() == 0 || (*itWaferID).isEmpty() == true)
        {
            hDefinitionFile << "*,";
        }
        else
        {
            hDefinitionFile << (*itWaferID) << ",";
        }
        // Next WaferID, if any
        itWaferID++;

        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().
                             Get("User1").toString())) << ",";  // Param#25
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().
                             Get("User2").toString())) << ",";  // Param#26
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().
                             Get("User3").toString())) << ",";  // Param#27
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().
                             Get("User4").toString())) << ",";  // Param#28
        hDefinitionFile <<
            FormatIdxString((dbKeysEngine.dbKeysContent().
                             Get("User5").toString())) << ",";  // Param#29
        hDefinitionFile << lTotalParts << ",";										// Param#30
        hDefinitionFile << lTotalGoodBins << ",";									// Param#31
        hDefinitionFile <<
            dbKeysEngine.dbKeysContent().
            Get("RetestIndex").toInt() << ","; // Param#32
        hDefinitionFile <<
            dbKeysEngine.dbKeysContent().
            Get("DibName").toString() << ",";  // Param#33
        hDefinitionFile <<
            dbKeysEngine.dbKeysContent().
            Get("DibType").toString() << ",";  // Param#34

        ///// **** VERY IMPORTANT ****
        // For EACH parameter inserted in this file,
        // remember to update the 'FiltersMapping' structure with its
        // matching offset
        // ***************************

        hDefinitionFile << endl;  // End of line
        iLoop--;
    }

    f.close();

    // Update the Filter tables to include these values if not already present
    strDestinationPath =
        pDatabaseEntry->PhysicalPath() + GEX_DATABASE_FILTER_FOLDER;
    lDir.mkdir(strDestinationPath);

    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_LOT,
                          dbKeysEngine.dbKeysContent().
                          Get("Lot").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_SUBLOT,
                          dbKeysEngine.dbKeysContent().
                          Get("SubLot").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_PROGRAMNAME,
                          dbKeysEngine.dbKeysContent().
                          Get("ProgramName").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_PROGRAMREVISION,
                          dbKeysEngine.dbKeysContent().
                          Get("ProgramRevision").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_OPERATOR,
                          dbKeysEngine.dbKeysContent().
                          Get("Operator").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_TESTERNAME,
                          dbKeysEngine.dbKeysContent().
                          Get("TesterName").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_TESTERTYPE,
                          dbKeysEngine.dbKeysContent().
                          Get("TesterType").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_TESTCODE,
                          dbKeysEngine.dbKeysContent().
                          Get("TestingCode").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_TEMPERATURE,
                          dbKeysEngine.dbKeysContent().
                          Get("Temperature").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_PACKAGE,
                          dbKeysEngine.dbKeysContent().
                          Get("PackageType").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_PRODUCT,
                          dbKeysEngine.dbKeysContent().
                          Get("Product").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_FAMILY,
                          dbKeysEngine.dbKeysContent().
                          Get("Family").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_FACILITY,
                          dbKeysEngine.dbKeysContent().
                          Get("Facility").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_FLOOR,
                          dbKeysEngine.dbKeysContent().
                          Get("Floor").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_PROCESS,
                          dbKeysEngine.dbKeysContent().
                          Get("Process").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_FREQUENCYSTEP,
                          dbKeysEngine.dbKeysContent().
                          Get("FrequencyStep").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_BURNIN,
                          dbKeysEngine.dbKeysContent().
                          Get("BurninTime").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_PROBERTYPE,
                          dbKeysEngine.dbKeysContent().
                          Get("ProberType").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_PROBERNAME,
                          dbKeysEngine.dbKeysContent().
                          Get("ProberName").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_LOADBOARDTYPE,
                          dbKeysEngine.dbKeysContent().
                          Get("LoadBoardType").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_LOADBOARDNAME,
                          dbKeysEngine.dbKeysContent().
                          Get("LoadBoard").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_DIBTYPE,
                          dbKeysEngine.dbKeysContent().
                          Get("DibType").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_DIBNAME,
                          dbKeysEngine.dbKeysContent().
                          Get("DibName").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_ORIGIN,
                          dbKeysEngine.dbKeysContent().
                          Get("DataOrigin").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_USER1,
                          dbKeysEngine.dbKeysContent().
                          Get("User1").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_USER2,
                          dbKeysEngine.dbKeysContent().
                          Get("User2").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_USER3,
                          dbKeysEngine.dbKeysContent().
                          Get("User3").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_USER4,
                          dbKeysEngine.dbKeysContent().
                          Get("User4").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_USER5,
                          dbKeysEngine.dbKeysContent().
                          Get("User5").toString());
    UpdateFilterTableFile(strDestinationPath, GEX_QUERY_FILTER_RETESTNBR,
                          QString::number(dbKeysEngine.dbKeysContent().
                                          Get("RetestIndex").toInt()));

    for (itWaferID = lWaferIdList.begin();
         itWaferID != lWaferIdList.end();
         ++itWaferID)
    {
        UpdateFilterTableFile(strDestinationPath,
                              GEX_QUERY_FILTER_WAFERID,
                              *itWaferID);
    }

    if(lLogTaskDetails)
    {
        lLocalErrorMessageLine =
            "[Task Details] Full insertion completed: "
            "Database index tables updated.";
        UpdateLogError(lLocalErrorMessage,
                       lLocalErrorMessageLine,
                       ! bImportFromRemoteDB);
    }

    return Passed;
}

} // namespace GS
} // namespace Gex
