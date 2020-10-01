// gexdb_plugin_galaxy_insertion.cpp: implementation of the GexDbPlugin_Galaxy class for insertion.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes: defines the entry point for the DLL application.
//
// ----------------------------------------------------------------------------------------------------------

// Local includes
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_option.h"
#include "import_constants.h"
#include "consolidation_tree_consolidation_rule.h"
#include "consolidation_tree_test_condition.h"
#include "consolidation_tree_query_engine.h"
#include "consolidation_tree_query_filter.h"
#include <gqtl_log.h>
#include "gqtl_datakeys_content.h"
#include "gqtl_datakeys_engine.h"
#include "multi_limit_item.h"
#include "gs_data.h"

// Standard includes

// Qt includes
#include <QSqlRecord>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QTimer>
#include <QApplication>

// Galaxy modules includes
#include <stdf.h>
#include <stdfparse.h>

//////////////////////////////////////////////
// For LOGGING or NOLOGGING mode
#define LOGGING_MODE 0
#if LOGGING_MODE
QString strLogging = "LOGGING";
#else
QString strLogging = "NOLOGGING";
#endif

//////////////////////////////////////////////
// Cumul time for debug
int iTime_Pass1 = 0;
int iTime_Stats_Pass1 = 0;
int iTime_AllRun_Pass1 = 0;
int iTime_Pass2 = 0;
int iTime_Stats_Pass2 = 0;
int iTime_AllRun_Pass2 = 0;

//////////////////////////////////////////////
// For PARTITION MAINTENANCE
// 0 : Use the Partition granularity (Month,Week,Day)
// else Partitions by limit
int iPartitionSplitlotsLimit = 0;
QString PartitionGranularityOption;

QTime m_clInsertStdfFileTime;

// FLAG DEFINITION
#define INVALID_SITE -2
#define MERGE_SITE -1

#define PERCENT_FOR_SAMPLING 0.1

// Limit when Gex switches between MultiValues insertion and SQL LOADER
int gLoadDataLimit = 800;

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GALAXY database type
////////////////////////////////////////////////////////////////////////////////////
#if 0
// Debug to enable insertion of Vishay files with duplicate test nb using InnoDB
int nSeqTest7;
#endif

bool GexDbPlugin_Galaxy::InitStdfFields()
{
    // Reset objects used to store STDF records content
    m_clStdfFAR.Reset();
    m_clStdfATR.Reset();
    m_clStdfMIR.Reset();
    m_clStdfMRR.Reset();
    m_clStdfPCR.Reset();
    m_clStdfHBR.Reset();
    m_clStdfSBR.Reset();
    m_clStdfPMR.Reset();
    m_clStdfPGR.Reset();
    m_clStdfPLR.Reset();
    m_clStdfRDR.Reset();
    m_clStdfSDR.Reset();
    m_clStdfWIR.Reset();
    m_clStdfWRR.Reset();
    m_clStdfWCR.Reset();
    m_clStdfPIR.Reset();
    m_clStdfPRR.Reset();
    m_clStdfTSR.Reset();
    m_clStdfPTR.Reset();
    m_clStdfMPR.Reset();
    m_clStdfFTR.Reset();
    m_clStdfBPS.Reset();
    m_clStdfEPS.Reset();
    m_clStdfGDR.Reset();
    m_clStdfDTR.Reset();

    mpDbKeysEngine = NULL;
    m_pbDelayInsertion = NULL;
    m_strStdfFile = "";
    m_eTestingStage = eUnknownStage;
    m_strPrefixTable = "";
    m_strProductName = "";
    m_nFileHostId = 0;

    m_strTrackingLotId = "";
    m_strLotId = "";
    m_strSubLotId = "";
    m_strWaferId = "";
    m_nMaxPacketSize = 1048576;
    m_nTotalWaferToProcess = 1;
    mProgress = 0;
    m_uiDtrIndex_InRun = 0;
    m_uiDtrIndex_InSplitlot = 0;

    m_nSplitLotId = 0;
    m_nTemporarySplitLotId = 0;
    m_nInsertionSID = 0;
    m_strInsertionHostName = "";

    m_strCurrentLockSession = "";

    m_strDataProvider = "";
    m_strDataType = "";

    m_bRemoveSequencerName = false;
    m_bRemovePinName = false;

    mIgnoreDtrTestConditions = false;
    mDtrTestConditionsCount = 0;


    mAllowedDuplicateTestNumbers = false;
    mAutoincrementTestNumbers = false;
    mMergeByTestName = false;
    mMergeByTestNumber = false;

    // Update Database Prefix for table access
    m_strPrefixDB = m_strSuffixDB = "";
    if (m_pclDatabaseConnector)
    {
        m_strPrefixDB = m_strSuffixDB = "";
        if (m_pclDatabaseConnector->m_bUseQuotesInSqlQueries)
        {
            m_strPrefixDB += "\"";
            m_strSuffixDB += "\"";
        }
    }

    if (m_clPTestResultsDataFile.isOpen())
    {
        m_clPTestResultsDataFile.close();
        QFile::remove(m_clPTestResultsDataFile.fileName());
    }
    if (m_clMPTestResultsDataFile.isOpen())
    {
        m_clMPTestResultsDataFile.close();
        QFile::remove(m_clMPTestResultsDataFile.fileName());
    }
    if (m_clFTestResultsDataFile.isOpen())
    {
        m_clFTestResultsDataFile.close();
        QFile::remove(m_clFTestResultsDataFile.fileName());
    }

    // Clear die-tracking info
    m_mapDieTrackingInfo.clear();

    // Clear list of updated tables
    m_clUpdatedTables.clear();

    // Clear members used for Auto-increment test number feature
    mAutoIncrementTestMap.clear();
    mAIDummyMaxTestNumber.clear();

    // Initialize some counters
    mConsistentPartsFlow = 0;
    mInconsistentPartsFlow = 0;
    mConsistentTests = 0;
    mInconsistentTests = 0;

    mTotalTestsCreated = 0;
    mTotalTestsCreatedForReferenceFlows = 0;
    mReferenceFlows.clear();

    // For the current Runs
    mTotalTestsExecuted.clear();
    mTotalNewTests.clear();
    mTotalInconsistentTests.clear();
    mLastInconsistentTestNumber.clear();
    mLastInconsistentTestName.clear();

    return Clear();
}

///////////////////////////////////////////////////////////
// Init some variables ...
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::Clear(bool bFirstWafer)
{
    if (bFirstWafer)
    {
        m_bPTestResultsBySqlLoader = true;
        m_bMPTestResultsBySqlLoader = true;
        m_bFTestResultsBySqlLoader = true;
        m_nNbPTestResults = 0;
        m_nNbFTestResults = 0;
        m_nNbMPTestResults = 0;
        m_strPTestResults = "";
        m_strFTestResults = "";
        m_strMPTestResults = "";
        m_nlInsertedSplitlots.clear();

    }
    else
    {
        m_lPass = 0;
        m_bStopProcess = false;
        mStopRequested = false;
        m_bLoadNextRecord = true;

        m_nSplitLotId = 0;
        m_nTemporarySplitLotId = 0;
        m_nInsertionSID = 0;
        if (m_pclDatabaseConnector)
            m_nInsertionSID = m_pclDatabaseConnector->GetConnectionSID();
        m_strInsertionHostName = "";
        if (m_pclDatabaseConnector)
            m_strInsertionHostName = m_pclDatabaseConnector->GetConnectionHostName();

        m_nSplitLotFlags = FLAG_SPLITLOT_NOTESTRESULTS;
        m_nReworkCode = 0;

        m_nNbSites = 0;

        m_nNbParts = 0;
        m_nNbPartsGood = 0;
        m_mapNbRuns.clear();
        m_mapNbRunsGood.clear();
        m_mapNbRuns[MERGE_SITE] = 0;
        m_mapNbRunsGood[MERGE_SITE] = 0;
        m_bUsePcrForSummary = false;

        m_bIgnoreFirstRun = false;

        m_nCurrentWaferIndex = 0;
        m_nNbHeadPerWafer = 0;
        m_bHaveMultiHeadWafer = false;
        m_nWaferIndexToProcess = 1;

        m_mapPCRNbParts.clear();
        m_mapPCRNbRetestParts.clear();
        m_mapPCRNbPartsGood.clear();

        m_nRunId = 0;
        m_mapRunId.clear();

        m_nTestsSequence = 0;
        m_mapTestsExecuted.clear();
        m_mapTestsFailed.clear();
        m_mapFirstFailTType.clear();
        m_mapFirstFailTId.clear();
        m_mapLastFailTType.clear();
        m_mapLastFailTId.clear();

        mSBinToHBin.clear();
        mInvalidSBinToHBin.clear();
        mInvalidHBinToSBin.clear();
        m_mapHBinInfo.clear();
        m_mapSBinInfo.clear();
        m_mapPinDef.clear();

        if (m_nTotalWaferToProcess == 1)
        {
            while (! m_lstTestInfo.isEmpty())
                delete m_lstTestInfo.takeFirst();
            m_lstTestInfo_Index = -1;
            mTestNumbers.clear();
            mTestNames.clear();
            mAssociatedTestNames.clear();
        }
        else
        {
            structTestInfo* pTest = NULL;
            int  lTestInfoIdx;
            for ( lTestInfoIdx = 0; lTestInfoIdx < m_lstTestInfo.size(); ++lTestInfoIdx)
            {
                pTest = m_lstTestInfo.at(lTestInfoIdx);

                if (pTest)
                {
                    pTest->m_mapExecCount.clear();
                    pTest->m_mapFailCount.clear();
                    pTest->m_mapMinValue.clear();
                    pTest->m_mapMaxValue.clear();
                    pTest->m_mapSum.clear();
                    pTest->m_mapSquareSum.clear();

                    pTest->mTsrSites.clear();
                    pTest->mTsrExecCount.clear();
                    pTest->mTsrFailCount.clear();
                    pTest->mTsrMinValue.clear();
                    pTest->mTsrMaxValue.clear();
                    pTest->mTsrSum.clear();
                    pTest->mTsrSquareSum.clear();

                    pTest->m_nTestSeq = -1;
                    pTest->m_cFlags = 0;
                    pTest->m_mapExecCount[MERGE_SITE] = 0;
                    pTest->m_mapFailCount[MERGE_SITE] = 0;
                    pTest->m_mapSum[MERGE_SITE] = 0;
                    pTest->m_mapSquareSum[MERGE_SITE] = 0;
                }
            }
            m_lstTestInfo_Index = -1;
        }


        m_nNbRunProcessed = 0;
        m_nNbRunInQuery = 0;
        m_strQueryRun = "";

        mQuerySamplesPTest = "";
        mQuerySamplesFTest = "";
        mQuerySamplesMPTest = "";
        mQuerySummaryPTest = "";
        mQuerySummaryFTest = "";
        mQuerySummaryMPTest = "";
        mQueryPTestLimits = "";
        mQueryPTestStaticLimits = "";
        mQueryMPTestLimits = "";
        mQueryMPTestStaticLimits = "";

        // Init table of records to process
        int i;
        for (i = 0; i < GQTL_STDF::Stdf_Record::Rec_COUNT; i++)
            m_nStdfRecordsCount[i] = 0;

        m_nFileSize = 0;
        mForceSqlInsertionTimeout = 10 * 1000;

    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Insert specified Data File into the current GALAXY database
bool
GexDbPlugin_Galaxy::InsertDataFile(struct GsData* lGsData,
                                   int lSqliteSplitlotId,
                                   const QString& strDataFileName,
                                   GS::QtLib::DatakeysEngine& dbKeysEngine,
                                   bool* pbDelayInsertion,
                                   long* plSplitlotID,
                                   int* pnTestingStage)
{
    if (! m_pclDatabaseConnector->IsMySqlDB())
        return false;

    QString strStdfFileName;
    GQTL_STDF::StdfParse clStdfParse;  // STDF V4 parser

    // Set start time of the InsertDataFile function
    m_clInsertDataFileTime.start();

    *pbDelayInsertion = false;
    *plSplitlotID = 0;
    *pnTestingStage = 0;

    QString strMessage = "Inserting DataFile " + strDataFileName + " on " + m_pclDatabaseConnector->m_strDatabaseName;
    GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data());  // why level 3 ?

    // First check if GEXDB is up-to-date

    QString strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int uiGexDbBuild, uiBuildSupportedByPlugin;
    bool bDbUpToDate = false;
    if(!IsDbUpToDateForInsertion(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        *pbDelayInsertion = true;
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
        return false;
    }
    if (! bDbUpToDate)
    {
        *pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
        return false;
    }

    // Check DB connection
    SetAdminLogin(true);
    if (! ConnectToCorporateDb())
    {
        *pbDelayInsertion = true;
        return false;
    }

    ProcessEvents();

    if (QFileInfo(strDataFileName).suffix().toLower() == "sqlite" &&
            lGsData != NULL)
    {
        m_pbDelayInsertion = pbDelayInsertion;
        // Dump KeysContent
        mpDbKeysEngine = &dbKeysEngine;
        if (! this->KeysContentDumpToTempTable())
        {
            return false;
        }
        // Insert
        if (this->InsertDataFileSqlite(lGsData,
                                       lSqliteSplitlotId,
                                       strDataFileName))
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Insert DataFile: Status=INSERTED");
            return true;
        }
        else if (m_pbDelayInsertion && *m_pbDelayInsertion)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Insert DataFile: Status=DELAY");
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "Insert DataFile: Status=FAIL");
        }
        return false;
    }

    // Check type of the data file we try to insert
    if (clStdfParse.Open(strDataFileName.toLatin1().constData()))
        strStdfFileName = strDataFileName;
    // Other formats than STDF V4 should be handled here...
    else
    {
        // Error. Can't open STDF file in read mode!
        *pbDelayInsertion = true;
        GSET_ERROR0(GexDbPlugin_Base, eStdf_Open, GGET_LASTERROR(StdfParse, &clStdfParse));
        ErrorMessage(GGET_LASTERRORMSG(StdfParse, &clStdfParse));
        return false;
    }

    // Init
    InitStdfFields();

    GSLOG(SYSLOG_SEV_NOTICE, QString("SQL conection ID=%1").arg(m_nInsertionSID).toLatin1().constData());
    if (m_nInsertionSID == 0)
    {
        // no SID found
        // Insertion not allowed
        *pbDelayInsertion = true;
        GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported, NULL,
                    QString("The connection ID (thread ID) for the SQL connection was not found. Every connection has an ID that is unique among the set of currently connected clients. Without connection ID, the insertion is not allowed.").toLatin1().data());
        ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));

        // Force to close the current connection to try to reinitialize ConnectionID
        QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName, false).close();

        return false;
    }

    iTime_Pass1 = 0;
    iTime_Stats_Pass1 = 0;
    iTime_AllRun_Pass1 = 0;
    iTime_Pass2 = 0;
    iTime_Stats_Pass2 = 0;
    iTime_AllRun_Pass2 = 0;
    TimersTraceReset("All");

    // Clear Warning messages
    m_strlWarnings.clear();
    mAttributes.clear();

    m_pbDelayInsertion = pbDelayInsertion;
    mpDbKeysEngine = &dbKeysEngine;

    ResetProgress(false);

    // Init Sql Session
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // MULTI INSERTION MULTI SQL_THREAD
    // During data insertion
    // Disable transaction for MySql InnoDb database
    // Release all active locks
    if (m_pclDatabaseConnector->IsMySqlDB())
        ReleaseLock(true);

    /////////////////////////
    // DEADLOCK PREVENTION
    // Commit transaction
    clGexDbQuery.Execute("COMMIT");

    QString strValue;
    if (! InitTokens())
    {
        *pbDelayInsertion = true;
        return false;
    }

    if (m_pclDatabaseConnector->IsSQLiteDB())
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("SQLite (%1): setting synchronous=OFF").arg(
                  (m_pclDatabaseConnector->GetSQLiteDriverVersion()).toLatin1().constData()).toLatin1().constData());
        if (! clGexDbQuery.Execute("PRAGMA synchronous=OFF"))
        {
            QString logMessage(clGexDbQuery.lastQuery() + "\n" +
                               clGexDbQuery.lastError().text() +
                               "\nInsertion will be very slow !");
            if (! mGexScriptEngine->property("GS_DAEMON").toBool())
                QMessageBox::critical(0, "error executing querry :", logMessage);
            else
                GSLOG(SYSLOG_SEV_CRITICAL, logMessage.toLatin1().constData());
        }
    }

    // GCORE-489: allow multi SDR insertions with a warning
    m_uiInsertionValidationFailOnFlag = m_uiInsertionValidationFailOnFlag & ~GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDSDRCOUNT;

    // Enable dynamic test limits ?
    GetGlobalOptionValue(eTestAllowDynamicLimits, strValue);
    if (strValue == "TRUE")
        m_uiInsertionValidationOptionFlag = m_uiInsertionValidationOptionFlag & ~GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTLIMITREDEFINITION;

    // SequencerName should be removed from testname ?
    GetGlobalOptionValue(eTestRemoveSequencerName, strValue);
    m_bRemoveSequencerName = (strValue == "TRUE");
    // Then check if the user overload the option through keycontents
    if (mpDbKeysEngine
            && mpDbKeysEngine->dbKeysContent().IsOverloaded("TestRemoveSequencerName"))
        m_bRemoveSequencerName = (mpDbKeysEngine->dbKeysContent().Get("TestRemoveSequencerName").toString() == "TRUE");
    if (m_bRemoveSequencerName)
        GSLOG(SYSLOG_SEV_DEBUG, QString(" Option TEST_REMOVE_SEQUENCER_NAME = TRUE").toLatin1().constData());


    // Ignore DTR tests conditions ?
    GetGlobalOptionValue(eTestIgnoreDtrTestcond, strValue);
    mIgnoreDtrTestConditions = (strValue == "TRUE");
    mDtrTestConditionsCount = 0;
    // Check if overladed by db key
    if (mpDbKeysEngine && mpDbKeysEngine->dbKeysContent().IsOverloaded("TestIgnoreDtrTestConditions"))
    {
        mIgnoreDtrTestConditions = (mpDbKeysEngine->dbKeysContent().Get("TestIgnoreDtrTestConditions").toString() == "TRUE");
    }
    if (mIgnoreDtrTestConditions)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString(" Option TEST_IGNORE_DTR_TEST_CONDITIONS = TRUE").toLatin1().constData());
    }

    // SequencerName should be removed from testname ?
    GetGlobalOptionValue(eTestRemovePinName,strValue);
    m_bRemovePinName = (strValue == "TRUE");
    // Then check if the user overload the option through keycontents
    if(mpDbKeysEngine
            && mpDbKeysEngine->dbKeysContent().IsOverloaded("TestRemovePinName"))
        m_bRemovePinName = (mpDbKeysEngine->dbKeysContent().Get("TestRemovePinName").toString() == "TRUE");
    if(m_bRemovePinName)
        GSLOG(SYSLOG_SEV_DEBUG, QString(" Option TEST_REMOVE_PIN_NAME = TRUE").toLatin1().constData() );

    // GCORE-3068: Consolidation Process managment
    GetGlobalOptionValue(eBinningConsolidationProcess, strValue);
    if (mpDbKeysEngine)
        mpDbKeysEngine->dbKeysContent().SetInternal("ConsolidationProcess", strValue);

    // Autoincrement TestNumber
    if (mpDbKeysEngine && ! mpDbKeysEngine->dbKeysContent().testAttributes().isEmpty())
    {
        // GCORE-853 : setting test[number] in pre-script does not work.
        GSLOG(SYSLOG_SEV_NOTICE, QString("testAttributes test[number] : '%1'").arg(
                  mpDbKeysEngine->dbKeysContent().testAttributes().value("test[number]")).toLatin1().data());
        GSLOG(SYSLOG_SEV_NOTICE, QString("KeysContent test[number] : '%1'").arg(
                  mpDbKeysEngine->dbKeysContent().Get("test[number]").toString()).toLatin1().data());

        int nLineError = 0;
        QString strError;
        mpDbKeysEngine->dbKeysContent().SetDbKeyContent("test[number]", "TestNum");
        if (mpDbKeysEngine->evaluateDynamicDbKeys(nLineError, strError))
        {
            mpDbKeysEngine->dbKeysContent().GetDbKeyContent("test[number]", strValue);
            // Save the first evaluation for WarningMessage
            mpDbKeysEngine->dbKeysContent().SetDbKeyContent("CountTestConditions", mpDbKeysEngine->dbKeysContent().testConditions().size());
        }

        if (strValue.startsWith("auto", Qt::CaseInsensitive))
        {
            // Check DB version
            // GCORE-853 : Allow Auto Test number option for Production TDR
            //if(!IsCharacTdr() && !IsYmProdTdr())
            if (IsManualProdTdr())
            {
                // Autoincrement Test Numbers is not allowed: Option not allowed
                *m_pbDelayInsertion = true;
                GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported, NULL,
                            QString("DbKeys Test[number]=Auto is not allowed for this TDR databases. Disable Test[Number] option")
                            .toLatin1().data());
                ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
                return false;
            }
            GSLOG(SYSLOG_SEV_NOTICE, QString(" DbKeys Test[number]=%1. TSR records will be ignored")
                  .arg(mpDbKeysEngine->dbKeysContent().testAttributes()["test[number]"]).toLatin1().constData());
            mAutoincrementTestNumbers = true;
            mMergeByTestName = true;
            mMergeByTestNumber = false;
            mAllowedDuplicateTestNumbers = true;
        }
    }

    // Duplicate test number allowed?
    GetGlobalOptionValue(eTestAllowDuplicateTestNumbers, strValue);
    if (strValue == "TRUE")
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString(" Option TEST_ALLOW_DUPLICATE_TESTNUMBERS = %1").arg( strValue).toLatin1().constData() );
        mAllowedDuplicateTestNumbers = true;
    }

    // Merge by TestName or TestNumber
    if (mpDbKeysEngine
            && mpDbKeysEngine->dbKeysContent().IsOverloaded("TestMergeBy")
            && (mpDbKeysEngine->dbKeysContent().Get("TestMergeBy") != "NEVER"))
    {
        mMergeByTestName = mMergeByTestNumber = false;
        if (mpDbKeysEngine->dbKeysContent().Get("TestMergeBy") == "NUMBER")
            mMergeByTestNumber = true;
        else if (mpDbKeysEngine->dbKeysContent().Get("TestMergeBy") == "NAME")
            mMergeByTestName = true;

        if (mAutoincrementTestNumbers && mMergeByTestNumber)
        {
            // Autoincrement Test Numbers is not allowed: Option not allowed
            *m_pbDelayInsertion = true;
            GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported, NULL,
                        QString("DbKeys Test[number]=Auto cannot be with with the DbKeys TestMergeBy=NUMBER. Disable Test[Number] option")
                        .toLatin1().data());
            ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
            return false;
        }
    }

    // Allow missing WaferNb
    GetGlobalOptionValue(eInsertionAllowMissingWaferNb, strValue);
    if (strValue == "TRUE")
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString(" Option INSERTION_ALLOW_MISSING_WAFERNB = %1").arg( strValue).toLatin1().constData() );
        m_uiInsertionValidationOptionFlag = m_uiInsertionValidationOptionFlag & ~GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTWAFERNO;
    }

    // 6390
    GetGlobalOptionValue(eInsertionFlushSqlBufferAfter, strValue);
    bool lIsNum;
    mForceSqlInsertionTimeout = 1 * 60 * 1000;
    gLoadDataLimit = 800;
    if (strValue.toInt(&lIsNum) && lIsNum)
    {
        mForceSqlInsertionTimeout = strValue.toInt() * 1000;
        GSLOG(SYSLOG_SEV_DEBUG, QString("Setting ForceSqlInsertionTimeout to %1 s")
              .arg(mForceSqlInsertionTimeout / 1000).toLatin1().constData());
    }


    mAttributes["SQLLOADER_LOCAL_OPTION"] = "";
    mAttributes["SQLLOADER_THREAD_OPTION"] = "";
    mAttributes["SQLLOADER_MODE_OPTION"] = "";
    mAttributes["SQLLOADER_QUERY"] = "";
    mAttributes["SQLLOADER_DATAFILES_CREATE"] = "";
    mAttributes["SQLLOADER_DATAFILES_MOVE"] = "";
    GetGlobalOptionValue("SQLLOADER_ACTIVATION", strValue);
    if (strValue == "TRUE")
    {

        // MySQL LOAD DATA INFILE
        //SQLLOADER_LOCAL_OPTION FORCE,AUTO
        // If AUTO, the LOCAL option is added only for remote TDR
        // If FORCE, the LOCAL option is added for ALL
        GetGlobalOptionValue("SQLLOADER_LOCAL_OPTION", strValue);
        mAttributes["SQLLOADER_LOCAL_OPTION"] = strValue;

        //SQLLOADER_THREAD_OPTION TDR,LOADER,FILE,EXEC
        // If TDR, use the TDR thread
        // If LOADER, create a new thread dedicated for the LOADER and use it for all file insertions
        // If FILE, create a new thread per FILE and remove it at the end of the insertion
        // If EXEC, create a new thread per EXEC and remove it at the end of the exec
        GetGlobalOptionValue("SQLLOADER_THREAD_OPTION", strValue);
        mAttributes["SQLLOADER_THREAD_OPTION"] = strValue;

        // Force all results to be populated by LOAD DATA
        // ex:
        // NORMAL = more than 700 rows,
        // FORCE = for all results
        // DELAY = do not execute the LOAD DATA but force the creation
        // DISABLED = MULTI-INSERT for all results
        // number = New limit
        GetGlobalOptionValue("SQLLOADER_MODE_OPTION", strValue);
        mAttributes["SQLLOADER_MODE_OPTION"] = strValue;
        if (! strValue.simplified().isEmpty()
                && ((strValue.simplified().toUpper() == "FORCE")
                    || (strValue.simplified().toUpper() == "DELAY")))
        {
            gLoadDataLimit = 0;
        }
        else if (! strValue.simplified().isEmpty()
                 && strValue.simplified().toUpper() == "DISABLED")
        {
            mForceSqlInsertionTimeout = -1;
            gLoadDataLimit = -1;
        }
        else if (! strValue.simplified().isEmpty()
                 && strValue.simplified().toInt() > 0)
        {
            gLoadDataLimit = strValue.simplified().toInt();
        }
        else
        {
            gLoadDataLimit = 800;
        }

        // Query to execute before the LOAD DATA statement
        // ex: unique_checks=OFF;
        GetGlobalOptionValue("SQLLOADER_OPTIONS_QUERY", strValue);
        mAttributes["SQLLOADER_QUERY"] = strValue.simplified();

        // The location where the DATAFILES must be created
        // ex: S:/data_files
        GetGlobalOptionValue("SQLLOADER_OPTIONS_DATAFILES_CREATE", strValue);
        // Check if folder exists or can be created
        if (! strValue.isEmpty())
        {
            if(!strValue.endsWith("/") && !strValue.endsWith("\\"))    strValue += "/";
            if (!QDir().exists(strValue))        QDir().mkpath(strValue);
            if (!QDir().exists(strValue))        strValue = "";
        }
        mAttributes["SQLLOADER_DATAFILES_CREATE"] = strValue.simplified();

        // The location where the DATAFILES must be moved (else delete)
        // ex: S:/data_files_for_replication
        GetGlobalOptionValue("SQLLOADER_OPTIONS_DATAFILES_MOVE", strValue);
        // Check if folder exists or can be created
        if (! strValue.isEmpty())
        {
            if(!strValue.endsWith("/") && !strValue.endsWith("\\"))      strValue += "/";
            if (!QDir().exists(strValue))        QDir().mkpath(strValue);
            if (!QDir().exists(strValue))        strValue = "";
        }
        mAttributes["SQLLOADER_DATAFILES_MOVE"] = strValue.simplified();

    }

    // To activate DEBUG trace managed by TimersTraceStart and TimersTraceStop
    // insert into global_options values('INSERTION_TRACE_ACTIVATION','ALL');
    // insert into global_options values('INSERTION_TRACE_ACTIVATION','RECORD|PROCEDURE|LOADDATAINFILE|FUNCTION|PROCESSEVENTS|GETLOCK|LOCKTABLE|DATABASEMANAGEMENT|TRANSACTION');
    mAttributes["INSERTION_TRACE_ACTIVATION"] = "";
    GetGlobalOptionValue("INSERTION_TRACE_ACTIVATION", strValue);
    if (! strValue.isEmpty())
    {
        if (strValue.contains("ALL"))
            mAttributes["INSERTION_TRACE_ACTIVATION"] = "ALL";
        else
            mAttributes["INSERTION_TRACE_ACTIVATION"] = strValue.toUpper();
        if (! m_bCustomerDebugMode)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Option INSERTION_TRACE_ACTIVATION must be used with -D option").toLatin1().constData() );
        }
    }

    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        // Check if have a specified partition ganularity
        GetGlobalOptionValue(eMysqlSplitPartitionBy, strValue);
        if (! strValue.isEmpty())
        {
            PartitionGranularityOption = strValue.toUpper();
            //iPartitionSplitlotsLimit = PartitionGranularity.section("|",1).toInt();
        }
        GSLOG(SYSLOG_SEV_DEBUG, QString(" Option MYSQL_SPLIT_PARTITION_BY = %1")
              .arg(PartitionGranularityOption).toLatin1().constData());

        // Add an optimizer query for MySql
        mAttributes["STRAIGHT_JOIN"] = "STRAIGHT_JOIN";
    }

    ///////////////////////////////////
    // Get new options
    ///////////////////////////////////
    // Reset
    mInsertionOptions.clear();

    // Allow Bin PRR Mismatch : ERROR, WARNING, IGNORE
    mInsertionOptions["AllowBinPRRMismatch"] = QVariant("ERROR");
    if (! mpDbKeysEngine->dbKeysContent().Get("AllowBinPRRMismatch").toString().isEmpty())
        mInsertionOptions["AllowBinPRRMismatch"] = mpDbKeysEngine->dbKeysContent().Get("AllowBinPRRMismatch");

    // Insert STDF file
    bool bStatus = InsertStdfFile(strStdfFileName);

    // Return splitlot_id and testing stage
    *plSplitlotID = m_nSplitLotId;
    *pnTestingStage = m_eTestingStage;

    // Cumul time for debug
    GSLOG(SYSLOG_SEV_DEBUG, " INSERTION TIME STATS ");
    GSLOG(SYSLOG_SEV_DEBUG, "Insert Stdf File CUMUL TIME");
    TimersTraceDump("All");
    GSLOG(SYSLOG_SEV_DEBUG, QString("     >>>>  all runs pass 1 cumul time %1")
          .arg(iTime_AllRun_Pass1).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("     >>>>  pass 1 cumul time %1")
          .arg(iTime_Pass1).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("     >>>>  stats pass 1 cumul time %1")
          .arg(iTime_Stats_Pass1).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("     >>>>  all runs pass 2 cumul time %1")
          .arg(iTime_AllRun_Pass2).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("     >>>>  pass 2 cumul time %1")
          .arg(iTime_Pass2).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("     >>>>  stats pass 2 cumul time %1")
          .arg(iTime_Stats_Pass2).toLatin1().constData());

    // Clear before exit
    ReleaseTokens();

    // Debug message
    strMessage = "Insert DataFile: Status=";
    if (bStatus)
    {
        strMessage += "INSERTED";
        strMessage += " Time=" + QString::number(m_clInsertDataFileTime.elapsed()) + " ms.";
        GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data());
    }
    else if (m_pbDelayInsertion && (*m_pbDelayInsertion))
    {
        strMessage += "DELAY";
        strMessage += " Time=" + QString::number(m_clInsertDataFileTime.elapsed()) + " ms.";
        GSLOG(SYSLOG_SEV_ERROR, strMessage.toLatin1().data());
    }
    else
    {
        strMessage += "FAIL";
        strMessage += " Time=" + QString::number(m_clInsertDataFileTime.elapsed()) + " ms.";
        GSLOG(SYSLOG_SEV_ERROR, strMessage.toLatin1().data());
    }

    InitStdfFields();
    return bStatus;
}

void GexDbPlugin_Galaxy::InsertStdfFileRefresh()
{
    ProcessEvents();

    // Add some auto trace here to see the progression of the insertion
    static QTime timeLog;
    if (m_nNbRunProcessed == 0)
        timeLog.start();

    QString lMessage = "Analysis phase:";
    if (m_lPass == 2)
        lMessage = "Insertion phase:";

    if (timeLog.elapsed() > (30 * 1000))  // every 30 secondes
    {
        lMessage += " %1 runs processed";
        GSLOG(SYSLOG_SEV_NOTICE, lMessage.arg(m_nNbRunProcessed).toLatin1().constData());
        timeLog.start();
    }
    else
    {
        lMessage += " %1 records read...";
        GSLOG(SYSLOG_SEV_DEBUG, lMessage.arg(mNumOfRecordsRead).toLatin1().data());
    }
}

void GexDbPlugin_Galaxy::StopProcess()
{
    // Delay the current insertion file
    if (m_pbDelayInsertion)
        *m_pbDelayInsertion = true;
    m_bStopProcess = true;
    mStopRequested = true;

    // Generate a new error
    QString strMessage = QString("Stop requested on " + m_pclDatabaseConnector->m_strDatabaseName);
    WarningMessage(strMessage);
}

bool GexDbPlugin_Galaxy::InsertStdfFile(const QString& strStdfFileName, int nWaferIndexToProcess  /*=1*/)
{
    bool bCallValidation = true;
    QString lQuery, lMessage;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Set start time of the InsertStdfFile function
    m_clInsertStdfFileTime.start();

    Clear(nWaferIndexToProcess == 1);

    m_nWaferIndexToProcess = nWaferIndexToProcess;
    // Set STDF file name
    m_strStdfFile = strStdfFileName;

    // Read STDF file...and insert information in SQL database
    // Pass 1: Collect information for check validity
    // Pass 2: Insert information in Galaxy database

    QTime qCumulTime;
    qCumulTime.start();

    // Ignore Custom STDF reocrds
    int nCustomRecords = 0;
    bool bIgnoreCustomRecords = false;
    QString strValue;
    GetGlobalOptionValue(eInsertionAllowCustomStdfRecords, strValue);
    if (strValue == "TRUE")
        bIgnoreCustomRecords = true;

    QTimer localTimer;
    GetGlobalOptionValue(eInsertionTimerInterval, strValue);
    bool ok = false;
    int interval = strValue.toInt(&ok);
    if (! ok)
        interval = 1000;
    localTimer.setInterval(interval);
    if (! QObject::connect(&localTimer, SIGNAL(timeout()), this, SLOT(InsertStdfFileRefresh())))
        GSLOG(SYSLOG_SEV_ERROR, "Cannot connect insertion timer");
    localTimer.start();

    // -- retrive the list of the HBIN/SBin get during the loadDbKey process
    QString lListStrBinNumber = mpDbKeysEngine->dbKeysContent().Get("HBRListBinNumber").toString();
    mListHBinNUmber = lListStrBinNumber.split(",");

    lListStrBinNumber = mpDbKeysEngine->dbKeysContent().Get("SBRListBinNumber").toString();
    mListSBinNUmber = lListStrBinNumber.split(",");


    for (m_lPass = 1; m_lPass <= 2; m_lPass++)
    {
        TimersTraceStart(QString("Pass %1").arg((m_lPass == 1 ? "Analyze" : "Insertion")));
        lMessage = "InsertStdfFile: Parsing file. Pass " + QString::number(m_lPass);
        GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data());

        // for progress bar
        GS::StdLib::Stdf clStdf;
        m_nFileSize = clStdf.GetFileSize(m_strStdfFile.toLatin1().data());
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Insert Stdf file : %1 ko...")
              .arg(m_nFileSize / 1024).toLatin1().constData());
        m_nCurrentWaferIndex = 0;
        m_nNbHeadPerWafer = 1;
        m_bHaveMultiHeadWafer = false;

        // DTR insertion
        m_uiDtrIndex_InRun = 0;
        m_uiDtrIndex_InSplitlot = 0;

        // Open STDF file to read...
        m_clStdfParse.Close();

        if (mIgnoreDtrTestConditions && (mDtrTestConditionsCount > 0))
        {
            WarningMessage(QString("[Record#DTR] %1 test conditions from DTR ignored in this file")
                           .arg(mDtrTestConditionsCount));
        }

        m_iStatus = m_clStdfParse.Open(m_strStdfFile.toLatin1().data());
        if (m_iStatus == false)
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "Can't open STDF file in read mode");
            // Error. Can't open STDF file in read mode!
            *m_pbDelayInsertion = true;
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Open, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            m_bStopProcess = true;
            break;
        }
        UpdateInsertionProgress();

        // Read one record from STDF file.
        TimersTraceStart("Record Load");
        m_iStatus = m_clStdfParse.LoadNextRecord(&m_nRecordType);
        TimersTraceStop("Record Load");

        mNumOfRecordsRead = 0;
        while ((m_iStatus == GQTL_STDF::StdfParse::NoError) || (m_iStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
        {
            // Proccess Events except for PTR, FTR, MPR, TSR, PMR, DTR
            if ((m_nRecordType != GQTL_STDF::Stdf_Record::Rec_PTR)
                    && (m_nRecordType != GQTL_STDF::Stdf_Record::Rec_FTR)
                    && (m_nRecordType != GQTL_STDF::Stdf_Record::Rec_PMR)
                    && (m_nRecordType != GQTL_STDF::Stdf_Record::Rec_MPR)
                    && (m_nRecordType != GQTL_STDF::Stdf_Record::Rec_DTR))
                ProcessEvents();

            // Process STDF record read.
            switch (m_nRecordType)
            {
            case GQTL_STDF::Stdf_Record::Rec_FAR:
                // ignore
                break;
            case GQTL_STDF::Stdf_Record::Rec_ATR:
                //ignore
                break;
            case GQTL_STDF::Stdf_Record::Rec_VUR:
                // read VUR information
                if (! ProcessVUR())
                    m_bStopProcess = true;
                break;
            case GQTL_STDF::Stdf_Record::Rec_MIR:
                TimersTraceStart("Record MIR");
                // read MIR information
                if (! ProcessMIR())
                    m_bStopProcess = true;
                TimersTraceStop("Record MIR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_MRR:
                // read MRR information
                TimersTraceStart("Record MRR");
                if (! ProcessMRR())
                    m_bStopProcess = true;
                TimersTraceStop("Record MRR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_PCR:
                TimersTraceStart("Record PCR");
                if (! mpDbKeysEngine->dbKeysContent().Get("IgnoreSummary").toBool())
                {
                    // read PCR information
                    if (! ProcessPCR())
                        m_bStopProcess = true;
                }
                TimersTraceStop("Record PCR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_HBR:
                TimersTraceStart("Record HBR");
                // read HBR information
                if (! ProcessHBR())
                    m_bStopProcess = true;
                TimersTraceStop("Record HBR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_SBR:
                TimersTraceStart("Record SBR");
                // read SBR information
                if (! ProcessSBR())
                    m_bStopProcess = true;
                TimersTraceStop("Record SBR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_PMR:
                TimersTraceStart("Record PMR");
                // read PMR information
                if (! ProcessPMR())
                    m_bStopProcess = true;
                TimersTraceStop("Record PMR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_PGR:
                // ignore
                break;
            case GQTL_STDF::Stdf_Record::Rec_PLR:
                // ignore
                break;
            case GQTL_STDF::Stdf_Record::Rec_RDR:
                // read RDR information
                if (! ProcessRDR())
                    m_bStopProcess = true;
                break;
            case GQTL_STDF::Stdf_Record::Rec_SDR:
                TimersTraceStart("Record SDR");
                // read RDR information
                if (! ProcessSDR())
                    m_bStopProcess = true;
                TimersTraceStop("Record SDR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_WIR:
                TimersTraceStart("Record WIR");
                // read WIR information
                if (! ProcessWIR())
                    m_bStopProcess = true;
                TimersTraceStop("Record WIR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_WRR:
                TimersTraceStart("Record WRR");
                // read WRR information
                if (! ProcessWRR())
                    m_bStopProcess = true;
                TimersTraceStop("Record WRR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_WCR:
                TimersTraceStart("Record WCR");
                // read WCinformation
                if (! ProcessWCR())
                    m_bStopProcess = true;
                TimersTraceStop("Record WCR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_PIR:
                TimersTraceStart("Record PIR");
                // read PIR information
                if (! ProcessPIR())
                    m_bStopProcess = true;
                // here, have enougth information for first validation
                if (bCallValidation)
                    m_bStopProcess = ! ValidationFunction(true);
                bCallValidation = false;
                TimersTraceStop("Record PIR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_PRR:
                TimersTraceStart("Record PRR");
                iTime_AllRun_Pass2 = qCumulTime.elapsed();
                // read PRR information
                if (! ProcessPRR())
                    m_bStopProcess = true;
                TimersTraceStop("Record PRR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_TSR:
                TimersTraceStart("Record TSR");
                if (! mpDbKeysEngine->dbKeysContent().Get("IgnoreSummary").toBool())
                {
                    // read TSR information
                    if (! ProcessTSR())
                        m_bStopProcess = true;
                }
                TimersTraceStop("Record TSR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_PTR:
                TimersTraceStart("Record PTR");
                // format of the TEST_TST
                // test_name <> test_seq
                // keep only test_name like in TSR
                // read PTR information
                if (! ProcessPTR())
                    m_bStopProcess = true;
                TimersTraceStop("Record PTR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_MPR:
                TimersTraceStart("Record MPR");
                // format of the TEST_TST
                // test_name <> test_seq
                // keep only test_name like in TSR
                // read MPR information
                if (! ProcessMPR())
                    m_bStopProcess = true;
                TimersTraceStop("Record MPR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_FTR:
                TimersTraceStart("Record FTR");
                // format of the TEST_TST
                // test_name <> test_seq
                // keep only test_name like in TSR
                // read FTR information
                if (! ProcessFTR())
                    m_bStopProcess = true;
                TimersTraceStop("Record FTR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_BPS:
                // ignore
                break;
            case GQTL_STDF::Stdf_Record::Rec_EPS:
                // ignore
                break;
            case GQTL_STDF::Stdf_Record::Rec_GDR:
                // modify site number deciphering mode
                m_clStdfParse.ReadRecord(&m_clStdfGDR);
                GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInParser
                        (m_clStdfGDR, m_clStdfParse);
                break;
            case GQTL_STDF::Stdf_Record::Rec_DTR:
                TimersTraceStart("Record DTR");
                if (! ProcessDTR())
                    m_bStopProcess = true;
                TimersTraceStop("Record DTR");
                break;
            case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
                // ignore
                break;
            case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
                // ignore
                break;
            case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
                // ignore
                if (m_lPass == 1)
                {
                    int nRecordType, nRecordSubType;
                    m_clStdfParse.GetRecordType(&nRecordType, &nRecordSubType);
                    if (! bIgnoreCustomRecords || (nRecordType < 200))
                    {
                        // Error reading STDF file
                        lMessage = "Invalid record found - RecordType[%1] RecordSubType[%2]";
                        lMessage = lMessage.arg(QString::number(nRecordType), QString::number(nRecordSubType));
                        GSLOG(SYSLOG_SEV_WARNING, lMessage.toLatin1().data());
                        GSET_ERROR1(GexDbPlugin_Base, eStdf_Corrupted, NULL,
                                    lMessage.toLatin1().constData());
                        m_bStopProcess = true;
                    }
                    else
                        nCustomRecords++;
                }
                break;
            default:
                // ignore
                break;
            }

            // Check if dump should be stopped
            if (m_bStopProcess)
                break;

            // Read one record from STDF file.
            if (m_bLoadNextRecord)
            {
                TimersTraceStart("Record Load");
                m_iStatus = m_clStdfParse.LoadNextRecord(&m_nRecordType);
                TimersTraceStop("Record Load");
            }
            m_bLoadNextRecord = true;
            mNumOfRecordsRead++;
            //emit NewMessage(QString("Reading records (%1)...").arg(numOfRecordRead) );
            //if (m_pProgressBar)
            //m_pProgressBar->setWindowTitle(QString("reading records (%1)...").arg(numOfRecordRead) );
        };

        lMessage = "InsertStdfFile: all records parsed for pass " + QString::number(m_lPass);
        lMessage += QString(" StopProcess ") + (m_bStopProcess ? "" : "NOT") + QString(" requested");
        GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data());

        if (! m_bStopProcess && m_iStatus != GQTL_STDF::StdfParse::EndOfFile)
        {
            // Ignore the end of the file only if have the MRR
            //if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MRR] == 0)
            //{
            //        GSET_ERROR1(GexDbPlugin_Base, eStdf_Corrupted, GGET_LASTERROR(StdfParse, &m_clStdfParse),"end of the file");
            //        m_bStopProcess = true;
            //}
            //else
            // This step will be check in the ValidationFunction
            m_iStatus = GQTL_STDF::StdfParse::EndOfFile;
        }


        if (m_bStopProcess)
            break;

        if (m_lPass == 2)
        {
            iTime_Pass2 = qCumulTime.elapsed();
            qCumulTime.start();

            // Update file host table
            if ((mpDbKeysEngine->dbKeysContent().Get("UploadFile").toString() == "TRUE") && ! UpdateFileHostTable())
            {
                GSLOG(SYSLOG_SEV_ERROR, "InsertStdfFile: UpdateFileHostTable failed !");
                m_bStopProcess = true;
                break;
            }

            ////////////////////////////////////////

            // During the consolidation WAIT,
            // this splitlot must remain invalid to not be consolidate by another Thread
            // all data are valid
            // validate transaction and unlock tables
            if (! StopSplitlotInsertion())
            {
                m_bStopProcess = true;
                break;
            }

            if (! UpdateWaferInfoTable())
            {
                // Update Wafer table fails
                // Reject the current insertion file

                m_bStopProcess = *m_pbDelayInsertion = true;

                // If out of resources, have to close all opened files
                clGexDbQuery.Execute("FLUSH TABLES");

                // Generate a new error
                lMessage = "Wafer insertion error [";
                lMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                lMessage += "]";
                WarningMessage(lMessage);
                break;
            }

            if (! UpdateSubLotInfoTable())
            {
                // Update sublot table fails
                // Reject the current insertion file

                m_bStopProcess = *m_pbDelayInsertion = true;

                // If out of resources, have to close all opened files
                clGexDbQuery.Execute("FLUSH TABLES");

                // Generate a new error
                lMessage = "Sublot insertion error [";
                lMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                lMessage += "]";
                WarningMessage(lMessage);
                break;
            }

            // Update Lot Table before Wafer_info table
            // Trigger consolitated_wafer usage
            if (! UpdateLotTable())
            {
                // Update Lot table fails
                // Reject the current insertion file
                *m_pbDelayInsertion = m_bStopProcess = true;

                // If out of resources, have to close all opened files
                clGexDbQuery.Execute("FLUSH TABLES");

                // Generate a new error
                lMessage = QString("Lot insertion error [") + GGET_LASTERRORMSG(GexDbPlugin_Base, this) + "]";
                WarningMessage(lMessage);
                break;
            }

            // all information about this splitlot are valid
            // update global information
            if (! UpdateProductTable(m_strProductName))
            {
                // Update Wafer table fails
                // Reject the current insertion file
                m_bStopProcess = *m_pbDelayInsertion = true;
                // Generate a new error
                lMessage = QString("Product insertion error [") + GGET_LASTERRORMSG(GexDbPlugin_Base, this) + "]";
                WarningMessage(lMessage);
                break;
            }

            if (! UpdateFtDieTrackingTable())
            {
                // Update Wafer table fails
                // Reject the current insertion file
                m_bStopProcess = *m_pbDelayInsertion = true;
                // Generate a new error
                lMessage = QString("FtDieTracking insertion error [") + GGET_LASTERRORMSG(GexDbPlugin_Base, this) + "]";
                WarningMessage(lMessage);
                break;
            }

            // CALL THE STORED PROCEDURE 'INSERTION_POSTPROCESSING'
            // Debug message
            lMessage = "Insert Stdf File : calling insertion_postprocessing stored procedure...";
            GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data());

            int nStatus;
            QString lProcedureName;

            // Success if no procedure
            nStatus = 1;
            lMessage = "Success";

            lProcedureName = NormalizeTableName("_insertion_postprocessing");
            TimersTraceStart("Procedure " + lProcedureName);
            // Check if the procedure exists
            lQuery = "SELECT ROUTINE_NAME FROM information_schema.ROUTINES WHERE lower(ROUTINE_NAME)='";
            if (m_strPrefixDB.isEmpty())
                lQuery += lProcedureName.toLower() + "'";
            else
                lQuery += lProcedureName.section(m_strPrefixDB, 1).toLower() + "'";

            if (! clGexDbQuery.Execute(lQuery))
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Query exec failed : %1")
                      .arg(clGexDbQuery.lastError().text()).toLatin1().constData());
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                m_bStopProcess = true;
                break;
            }
            if (clGexDbQuery.First())
            {
                lQuery = "CALL " + lProcedureName;
                lQuery += "(" + QString::number(m_nSplitLotId) + ",@strMessage,@nStatus)";

                if (! clGexDbQuery.Execute(lQuery))
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Query exec failed : %1")
                          .arg(clGexDbQuery.lastError().text()).toLatin1().constData());
                    *m_pbDelayInsertion = true;
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                                clGexDbQuery.lastError().text().toLatin1().constData());
                    m_bStopProcess = true;
                    break;
                }

                lQuery = "select @strMessage, @nStatus";
                if (! clGexDbQuery.Execute(lQuery))
                {
                    *m_pbDelayInsertion = true;
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                                clGexDbQuery.lastError().text().toLatin1().constData());
                    m_bStopProcess = true;
                    break;
                }
                clGexDbQuery.First();
                // Retrieve parameter values
                lMessage = clGexDbQuery.value(0).toString();  // the returned message
                nStatus = clGexDbQuery.value(1).toInt();  // nStatus is the return status: 0 is NOK, 1 is OK, 2 delay insertion
            }


            if (nStatus == 0)
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_InsertionPostProcessingProcedure, NULL,
                            lProcedureName.toLatin1().constData(),
                            QString("Status[%1] Message[%2]").arg(nStatus).arg(lMessage).toLatin1().constData());
                // Reject the current insertion file
                m_bStopProcess = true;
                break;
            }
            if (nStatus == 2)
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_InsertionPostProcessingProcedure, NULL,
                            lProcedureName.toLatin1().constData(),
                            QString("Status[%1] Message[%2]").arg(nStatus).arg(lMessage).toLatin1().constData());
                m_bStopProcess = *m_pbDelayInsertion = true;
                break;
            }
            TimersTraceStop("Procedure " + lProcedureName);

            // Check if have a warning to display
            if (! lMessage.isEmpty() && lMessage != "Success")
                WarningMessage(QString("Call Procedure %1 - Status[%2] Message[%3]").arg(lProcedureName).arg(nStatus).arg(lMessage));

            // >>>> After the call of the POSTPROCESSING stored procedure
            // Some update could be done on the RUN table
            // to update the PART_X, PART_Y and PART_ID
            // This means that we need to recompute the PART_RETEST_INDEX
            // This means that we need to recompute the HBIN/SBIN count info
            // to extract the NB_PARTS, NB_PARTS_GOOD after the PART_RETEST_INDEX
            // Then update the Parts/HBin/SBin StatsSamples from the REAL status of the RUN table
            if (! UpdateRunTablePartFlags())
            {
                m_bStopProcess = *m_pbDelayInsertion = true;
                break;
            }
            // Now update the Parts/Bins Summary from samples
            if (! UpdatePartsStatsSamplesTable())
            {
                m_bStopProcess = *m_pbDelayInsertion = true;
                break;
            }

            if (! UpdateHBinStatsSamplesTable())
            {
                m_bStopProcess = *m_pbDelayInsertion = true;
                break;
            }

            if (! UpdateSBinStatsSamplesTable())
            {
                m_bStopProcess = *m_pbDelayInsertion = true;
                break;
            }


            // MySql
            // Realease GetLock
            iTime_Stats_Pass2 = qCumulTime.elapsed();
            qCumulTime.start();

        }  // pass 2

        if (m_lPass == 1)
        {
            iTime_Pass1 = qCumulTime.elapsed();
            iTime_AllRun_Pass1 = iTime_AllRun_Pass2;
            iTime_AllRun_Pass2 = 0;
            qCumulTime.start();

            // End of the Pass 1 and Begining of the Pass 2
            // open connection on Pass 2
            bCallValidation = false;

            // TestingStage must be defined for Stats computation
            SetTestingStage();

            // Check Test conditions meta-data
            // HTH-TOCHECK
            if (checkMetaDataForTestConditions() == false)
                m_bStopProcess = true;

            if (m_bStopProcess)
                break;

            // Custom records supported whith options
            // Display warning
            if (bIgnoreCustomRecords && (nCustomRecords > 0))
            {
                QString strMessage = "[STDF Custom Record] You are inserting a STDF file ";
                strMessage += "with " + QString::number(nCustomRecords) + " Custom records. ";
                strMessage += "All Custom records will be ignored.";
                WarningMessage(strMessage);
            }

            m_nNbRunProcessed = 0;
            m_nNbRunInQuery = 0;
            m_strQueryRun = "";
            m_strPTestResults = "";
            m_strFTestResults = "";
            m_strMPTestResults = "";
#if 0
            // case 7769
            // Update the FlowId
            structTestInfo* pTest = NULL;
            m_nTestsSequence = 0;
            foreach(pTest, m_lstTestInfo)
            {
                if (pTest && (pTest->m_nTestSeq > -1))
                {
                    m_nTestsSequence++;
                    pTest->m_nTestSeq = m_nTestsSequence;
                }
            }
#endif

            // NbParts = NbRuns
            m_nNbParts = m_nNbPartsGood = 0;
            // PARTS INFO INITIALIZATION AS RUNS INFO
            QMap<int, structBinInfo>::Iterator itBinInfo;
            QMap<int, int>::Iterator itSiteInfo;
            for (itBinInfo = m_mapHBinInfo.begin(); itBinInfo != m_mapHBinInfo.end(); ++itBinInfo)
                for(itSiteInfo = itBinInfo.value().m_mapBinNbRuns.begin(); itSiteInfo != itBinInfo.value().m_mapBinNbRuns.end(); ++itSiteInfo)
                    itBinInfo.value().m_mapBinNbParts[itSiteInfo.key()]=itBinInfo.value().m_mapBinNbRuns[itSiteInfo.key()];

            for (itBinInfo = m_mapSBinInfo.begin(); itBinInfo != m_mapSBinInfo.end(); ++itBinInfo)
                for(itSiteInfo = itBinInfo.value().m_mapBinNbRuns.begin(); itSiteInfo != itBinInfo.value().m_mapBinNbRuns.end(); ++itSiteInfo)
                    itBinInfo.value().m_mapBinNbParts[itSiteInfo.key()]=itBinInfo.value().m_mapBinNbRuns[itSiteInfo.key()];

            m_nNbPartsGood = m_mapNbRunsGood[MERGE_SITE];
            m_nNbParts = m_mapNbRuns[MERGE_SITE];

            if (m_eTestingStage == eElectTest)
            {
                // verify how many parts they have
                // if only one part then try to use information from WRR
                if (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR] <= 1)
                {
                    // use summary from WRR
                    m_nNbPartsGood = m_clStdfWRR.m_u4GOOD_CNT;
                    m_nNbParts = m_clStdfWRR.m_u4PART_CNT;

                    // ignore all information from HBR SBR
                    // rebuild from WRR information
                    // only 2 bins : 1 for PASS and 0 for FAIL

                    m_mapHBinInfo.clear();
                    m_mapSBinInfo.clear();
                    // For good Bin
                    m_mapHBinInfo[1].m_nBinNum = m_mapSBinInfo[1].m_nBinNum = 1;
                    m_mapHBinInfo[1].m_cBinCat = m_mapSBinInfo[1].m_cBinCat = 'P';
                    m_mapHBinInfo[1].m_mapBinCnt[INVALID_SITE] = m_mapSBinInfo[1].m_mapBinCnt[INVALID_SITE] = m_nNbPartsGood;
                    m_mapHBinInfo[1].m_mapBinNbRuns[MERGE_SITE] = m_mapSBinInfo[1].m_mapBinNbRuns[MERGE_SITE] = m_nNbPartsGood;

                    // For fail Bin
                    m_mapHBinInfo[0].m_nBinNum = m_mapSBinInfo[0].m_nBinNum = 0;
                    m_mapHBinInfo[0].m_cBinCat = m_mapSBinInfo[0].m_cBinCat = 'F';
                    m_mapHBinInfo[0].m_mapBinCnt[INVALID_SITE] = m_mapSBinInfo[0].m_mapBinCnt[INVALID_SITE] = m_nNbParts - m_nNbPartsGood;
                    m_mapHBinInfo[0].m_mapBinNbRuns[MERGE_SITE] = m_mapSBinInfo[0].m_mapBinNbRuns[MERGE_SITE] = m_nNbParts - m_nNbPartsGood;
                }
                else
                {
                    QMap<int, structBinInfo>::Iterator itBin;
                    // ignore all information from HBR SBR
                    // rebuild from PRR info
                    // BinCnt = NbRuns
                    for (itBin = m_mapHBinInfo.begin(); itBin != m_mapHBinInfo.end(); ++itBin)
                    {
                        itBin.value().m_mapBinCnt.clear();
                        itBin.value().m_mapBinCnt[INVALID_SITE] = itBin.value().m_mapBinNbRuns[MERGE_SITE];
                    }
                    for (itBin = m_mapSBinInfo.begin(); itBin != m_mapSBinInfo.end(); ++itBin)
                    {
                        itBin.value().m_mapBinCnt.clear();
                        itBin.value().m_mapBinCnt[INVALID_SITE] = itBin.value().m_mapBinNbRuns[MERGE_SITE];
                    }
                }

                m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR] = 0;
                m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SBR] = 0;

            }

            // if PCR record
            // if no HBR record and if have SBR record then have to rebuild HBR information
            if ((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR] == 0)
                    && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SBR] > 0))
            {
                QMap<int, structBinInfo>::Iterator itBin;
                for (itBin = m_mapSBinInfo.begin(); itBin != m_mapSBinInfo.end(); ++itBin)
                {
                    // verify if have information from SBR
                    if (itBin.value().m_mapBinCnt.isEmpty())
                        continue;

                    m_mapHBinInfo[itBin.key()].m_nBinNum = m_mapSBinInfo[itBin.key()].m_nBinNum;
                    m_mapHBinInfo[itBin.key()].m_cBinCat = m_mapSBinInfo[itBin.key()].m_cBinCat;
                    m_mapHBinInfo[itBin.key()].m_strBinName = m_mapSBinInfo[itBin.key()].m_strBinName;
                    // for each Site
                    QMap<int, int>::Iterator itSite;
                    for (itSite = itBin.value().m_mapBinCnt.begin();        itSite != itBin.value().m_mapBinCnt.end(); ++itSite )
                    {
                        m_mapHBinInfo[itBin.key()].m_mapBinCnt[itSite.key()] = m_mapSBinInfo[itBin.key()].m_mapBinCnt[itSite.key()];
                    }
                }
                // Increment the count to force the save in the DB
                m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR]++;
            }

            // if no PCR record and if have HBR record then have to rebuild PCR information
            if ((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR] == 0)
                    && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR] > 0))
            {
                int nSiteNum;

                QMap<int, structBinInfo>::Iterator itBin;
                for (itBin = m_mapHBinInfo.begin(); itBin != m_mapHBinInfo.end(); ++itBin)
                {
                    // verify if have information from HBR
                    if (itBin.value().m_mapBinCnt.isEmpty())
                        continue;

                    // for each Site
                    QMap<int, int>::Iterator itSite;
                    for (itSite = itBin.value().m_mapBinCnt.begin();        itSite != itBin.value().m_mapBinCnt.end(); ++itSite )
                    {
                        nSiteNum = INVALID_SITE;

                        // Check if value for all sites
                        // No information from INVALID_SITE
                        // Have to reconstruct it
                        if (! itBin.value().m_mapBinCnt.contains(INVALID_SITE))
                        {
                            if (! m_mapPCRNbParts.contains(nSiteNum))
                                m_mapPCRNbParts[nSiteNum] = 0;
                            if (! m_mapPCRNbRetestParts.contains(nSiteNum))
                                m_mapPCRNbRetestParts[nSiteNum] = 0;
                            if (! m_mapPCRNbPartsGood.contains(nSiteNum))
                                m_mapPCRNbPartsGood[nSiteNum] = 0;

                            if ((itBin.value().m_cBinCat == 'P') && (itBin.value().m_mapBinCnt.contains(nSiteNum)))
                                m_mapPCRNbPartsGood[nSiteNum] += itBin.value().m_mapBinCnt[nSiteNum];

                            if (itBin.value().m_mapBinCnt.contains(nSiteNum))
                                m_mapPCRNbParts[nSiteNum] += itBin.value().m_mapBinCnt[nSiteNum];
                        }


                        // Check if value for one site
                        nSiteNum = itSite.key();

                        if (! m_mapPCRNbParts.contains(nSiteNum))
                            m_mapPCRNbParts[nSiteNum] = 0;
                        if (! m_mapPCRNbRetestParts.contains(nSiteNum))
                            m_mapPCRNbRetestParts[nSiteNum] = 0;
                        if (! m_mapPCRNbPartsGood.contains(nSiteNum))
                            m_mapPCRNbPartsGood[nSiteNum] = 0;

                        if (itBin.value().m_cBinCat == 'P')
                            m_mapPCRNbPartsGood[nSiteNum] += itBin.value().m_mapBinCnt[nSiteNum];

                        m_mapPCRNbParts[nSiteNum] += itBin.value().m_mapBinCnt[nSiteNum];

                    }
                }
                // Increment the count to force the save in the DB
                m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR]++;
            }

            if((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR] > 0) || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR] > 0))
            {
                // have collect some info
                // verify if have the same value in PCR

                int nSiteCumul = INVALID_SITE;
                if (m_mapPCRNbParts.contains(MERGE_SITE))
                    nSiteCumul = MERGE_SITE;

                if (nSiteCumul == INVALID_SITE)
                {
                    m_mapPCRNbParts[MERGE_SITE] = m_mapPCRNbParts[INVALID_SITE];
                    m_mapPCRNbPartsGood[MERGE_SITE] = m_mapPCRNbPartsGood[INVALID_SITE];
                    m_mapPCRNbRetestParts[MERGE_SITE] = m_mapPCRNbRetestParts[INVALID_SITE];
                }

                // For Final Test
                // if no PCR then Samples
                // if PCR +- = Samples then Samples
                // if PCR >> Samples then PCR (probably sampling)
                if (m_eTestingStage == eFinalTest)
                {
                    //o Sum = 0      => use summary, no warning
                    //o Sum = 0      => use samples, no warning
                    //o Sum < Sam    => use samples, warning
                    //o Sum = Sam    => use samples, no warning
                    //o Sum > Sam
                    //   $ (Sum-Sam)/Sum >= 0,1 => use summary, no warning
                    //   $ (Sum-Sam)/Sum < 0,1  => use samples, warning
                    if (m_nNbParts == 0)
                        m_bUsePcrForSummary = true;
                    else if((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR] == 0) || (m_mapPCRNbParts[nSiteCumul] == 0))
                        m_bUsePcrForSummary = false;
                    else if (m_mapPCRNbParts[nSiteCumul] < m_nNbParts)
                        m_bUsePcrForSummary = false;
                    else if (m_mapPCRNbParts[nSiteCumul] == m_nNbParts)
                        m_bUsePcrForSummary = false;
                    else if(((float)(m_mapPCRNbParts[nSiteCumul]-m_nNbParts)/(float)m_mapPCRNbParts[nSiteCumul]) >= PERCENT_FOR_SAMPLING)
                        m_bUsePcrForSummary = true;
                    else
                        m_bUsePcrForSummary = false;
                }
                else
                    // For Elect Test
                    // always use Summary if exist
                    if (m_eTestingStage == eElectTest)
                    {
                        m_bUsePcrForSummary = true;
                    }
                    else
                        // For Wafer Sort
                        if (m_eTestingStage == eWaferTest)
                        {
                            // For Wafer Test Summary
                            if (m_nNbParts == 0)
                                m_bUsePcrForSummary = true;
                            else
                                m_bUsePcrForSummary = false;
                        }
            }

            if (m_bUsePcrForSummary)
                m_nSplitLotFlags &= ~FLAG_SPLITLOT_PARTSFROMSAMPLES;
            else
                m_nSplitLotFlags |= FLAG_SPLITLOT_PARTSFROMSAMPLES;

            m_nTotalWaferToProcess = m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR];

            // have to check the validity of the STDF file
            // after Stats computations
            if (! ValidationFunction())  // m_lPass==1
            {
                GSLOG(SYSLOG_SEV_WARNING, "ValidationFunction returned false");
                m_bStopProcess = true;
                break;
            }
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "Insert Stdf file : Validation ok");

            // reset WIR and WRR count
            m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] = 0;
            m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR] = 0;

            // Reset PIR count
            m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] = 0;

            // Determine here if load results data
            // from Sql Loader Data Infile
            // or with Sql Script
            if((gLoadDataLimit<0) || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PTR]*qMax(1,m_nTotalWaferToProcess)<gLoadDataLimit))
                m_bPTestResultsBySqlLoader = false;
            if((gLoadDataLimit<0) || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR]*qMax(1,m_nTotalWaferToProcess)<gLoadDataLimit))
                m_bMPTestResultsBySqlLoader = false;
            if((gLoadDataLimit<0) || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_FTR]*qMax(1,m_nTotalWaferToProcess)<gLoadDataLimit))
                m_bFTestResultsBySqlLoader = false;

            if (m_pclDatabaseConnector->IsSQLiteDB())
            {
                // in SQLite, manual insert for each kind of data (To Do : integrate the SQlite "LOAD DATA IN FILE")
                m_bPTestResultsBySqlLoader = m_bMPTestResultsBySqlLoader = m_bFTestResultsBySqlLoader = false;
            }

            // In MySql mode, Queries are limited by MaxPacketSize
            if (m_pclDatabaseConnector->IsMySqlDB())
            {
                // initialization for m_nMaxPacketSize
                lQuery = "SELECT @@max_allowed_packet";
                if (! clGexDbQuery.Execute(lQuery))
                {
                    GSLOG(SYSLOG_SEV_WARNING, QString("Query exec failed :%1")
                          .arg(clGexDbQuery.lastError().text()).toLatin1().constData());
                    *m_pbDelayInsertion = true;
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                                clGexDbQuery.lastError().text().toLatin1().constData());
                    m_bStopProcess = true;
                    break;
                }
                clGexDbQuery.First();
                bool ok = false;
                m_nMaxPacketSize = clGexDbQuery.value(0).toInt(&ok);
                if (! ok)
                    GSLOG(SYSLOG_SEV_ERROR, QString("Cannot interpret MaxPacketSize query value %1")
                          .arg(clGexDbQuery.value(0).toString()).toLatin1().constData());
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("MaxPacketSize for this SQL server : %1")
                      .arg(m_nMaxPacketSize).toLatin1().constData());
            }
            else
                m_nMaxPacketSize = 100000;

            iTime_Stats_Pass1 = qCumulTime.elapsed();
            qCumulTime.start();

            TimersTraceStart("DatabaseManagement");

            if (! UpdateTablespacePartitionsSequence())
            {
                *m_pbDelayInsertion = true;
                m_bStopProcess = true;
                break;
            }
            TimersTraceStop("DatabaseManagement");
            qCumulTime.start();

        }  // Pass 1
        TimersTraceStop(QString("Pass %1").arg((m_lPass == 1 ? "Analyze" : "Insertion")));
    }  // for all passes (currently 2).

    localTimer.stop();

    if (m_pclDatabaseConnector->IsMySqlDB())
        ReleaseLock(true);

    /////////////////////////
    // DEADLOCK PREVENTION
    // Commit transaction
    clGexDbQuery.Execute("COMMIT");

    // Have to update KeyContent
    // the original m_pKeyContent->strProductID was saved in the PART_TYP
    mpDbKeysEngine->dbKeysContent().Set("Product", m_strProductName);
    mpDbKeysEngine->dbKeysContent().Set("Lot", m_strLotId);
    if (! mpDbKeysEngine->dbKeysContent().IsOverloaded("Wafer"))
        mpDbKeysEngine->dbKeysContent().Set("Wafer", m_strWaferId, true);
    mpDbKeysEngine->dbKeysContent().Set("TrackingLot", m_strTrackingLotId);
    mpDbKeysEngine->dbKeysContent().SetInternal("SplitlotId", m_nSplitLotId);
    QString lTestingStage;
    GetTestingStageName(m_eTestingStage, lTestingStage);
    mpDbKeysEngine->dbKeysContent().SetInternal("TestingStage", lTestingStage);

    // Dump KeysContent
    // Create the TEMPORARY gexdbkeys
    // for STORED PROCEDURES access (to have the Error and the ErrorCode)
    KeysContentDumpToTempTable();
    QString lProcedureName;

    // CALL THE STORED PROCEDURE 'FILEARCHIVE_SETTINGS'
    bool lUseArchiveSettings = false;
    bool lSuccess = false;
    int nStatus = 1;
    if (m_bStopProcess)
        nStatus = 0;
    if (m_pbDelayInsertion && (*m_pbDelayInsertion))
        nStatus = 2;

    lProcedureName = NormalizeTableName("_filearchive_settings");
    TimersTraceStart("Procedure " + lProcedureName);

    // Debug message
    lMessage = "Calling filearchive_settings stored procedure...";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data());

    // First try to call the procedure with the Status parameter
    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        lQuery = "CALL " + lProcedureName;
        lQuery += "(" + QString::number(m_nSplitLotId) + ",";
        lQuery += TranslateStringToSqlVarChar(m_strTrackingLotId);
        lQuery += ",";
        lQuery += TranslateStringToSqlVarChar(m_strLotId);
        lQuery += ",";
        lQuery += TranslateStringToSqlVarChar(m_strWaferId);
        lQuery += " %1 ";
        lQuery += ",@bUseArchiveSettings,@strMovePath,@uiFtpPort,@strFtpServer,@strFtpUser,@strFtpPassword,@strFtpPath)";

        if (! clGexDbQuery.Execute(lQuery.arg("," + QString::number(nStatus))))
        {
            if (clGexDbQuery.lastError().number() == 1318)  // Error Code: 1318. Incorrect number of arguments
            {
                // Try without Status only if PASS
                if (! m_bStopProcess)
                {
                    if (! clGexDbQuery.Execute(lQuery.arg(" ")))
                    {
                        lMessage = clGexDbQuery.lastError().text();
                        lMessage += "\nFailed calling stored procedure " + lProcedureName;
                        WarningMessage(lMessage);
                    }
                    else
                    {
                        lSuccess = true;
                        // Message to the user
                        lMessage = "Obsolete definition of "+lProcedureName+" without Status parameter. See Gex documentation for update";
                        WarningMessage(lMessage);
                    }
                }
            }
            else
            {
                lMessage = clGexDbQuery.lastError().text();
                lMessage += "\nFailed calling stored procedure " + lProcedureName;
                WarningMessage(lMessage);
            }
        }
        else
            lSuccess = true;

        if (lSuccess)
        {
            // Extract results
            lQuery = "select @bUseArchiveSettings,@strMovePath,@uiFtpPort,@strFtpServer,@strFtpUser,@strFtpPassword,@strFtpPath";
            if (! clGexDbQuery.Execute(lQuery))
            {
                lMessage = clGexDbQuery.lastError().text();
                lMessage += "\nFailed calling stored procedure " + lProcedureName;
                WarningMessage(lMessage);
            }
            else
            {
                clGexDbQuery.First();
                // Retrieve parameter values
                lUseArchiveSettings = clGexDbQuery.value(0).toInt();
                if (lUseArchiveSettings)
                {
                    mpDbKeysEngine->dbKeysContent().Set("MovePath", clGexDbQuery.value(1).toString());
                    mpDbKeysEngine->dbKeysContent().Set("FtpPort", clGexDbQuery.value(2).toUInt());
                    mpDbKeysEngine->dbKeysContent().Set("FtpServer", clGexDbQuery.value(3).toString());
                    mpDbKeysEngine->dbKeysContent().Set("FtpUser", clGexDbQuery.value(4).toString());
                    mpDbKeysEngine->dbKeysContent().Set("FtpPassword", clGexDbQuery.value(5).toString());
                    mpDbKeysEngine->dbKeysContent().Set("FtpPath", clGexDbQuery.value(6).toString());
                }
            }
        }
    }
    // If move path change
    // need to update the splitlot.file_path
    if (! m_bStopProcess && lUseArchiveSettings)
    {
        // Update splitlot table
        if (mpDbKeysEngine->dbKeysContent().Get("MoveFile").toString() == "TRUE")
        {
            lQuery = "UPDATE " + NormalizeTableName("_SPLITLOT") + " SET ";
            lQuery += "FILE_PATH=";
            lQuery += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("MovePath").toString(), true);
            lQuery += " WHERE SPLITLOT_ID=";
            lQuery += QString::number(m_nSplitLotId);
            if (! clGexDbQuery.Execute(lQuery))
            {
                lMessage = clGexDbQuery.lastError().text();
                lMessage += "\nFailed updating FILE_PATH from " + lProcedureName;
                WarningMessage(lMessage);
            }
        }
    }


    // CALL THE STORED PROCEDURE 'INSERTION_STATUS'
    lMessage = "Insert Stdf File : calling insertion_status stored procedure...";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data());

    lMessage = "Success";
    if (m_bStopProcess)
        lMessage = TranslateStringToSqlVarChar(GGET_LASTERRORMSG(GexDbPlugin_Base, this), false);

    lProcedureName = NormalizeTableName("_insertion_status");
    TimersTraceStart("Procedure " + lProcedureName);
    lQuery = "CALL " + lProcedureName;
    lQuery += "(" + QString::number(m_nSplitLotId) + ",";
    lQuery += TranslateStringToSqlVarChar(m_strTrackingLotId);
    lQuery += ",";
    lQuery += TranslateStringToSqlVarChar(m_strLotId);
    lQuery += ",";
    lQuery += TranslateStringToSqlVarChar(m_strWaferId);
    lQuery += ",";
    lQuery += TranslateStringToSqlVarChar(lMessage);
    lQuery += "," + QString::number(nStatus) + ")";

    if (! m_pclDatabaseConnector->IsSQLiteDB())  // No procedure in SQLite
        if (! clGexDbQuery.Execute(lQuery))
        {
            lMessage = clGexDbQuery.lastError().text();
            lMessage += "\nFailed calling stored procedure " + lProcedureName;
            WarningMessage(lMessage);
        }
    TimersTraceStop("Procedure " + lProcedureName);

    // Update the incremental_update and global_info table
    if (mpDbKeysEngine
            && mpDbKeysEngine->dbKeysContent().Get("ConsolidationProcess").toString() == "TRUE")
    {
        lQuery = "UPDATE incremental_update SET remaining_splitlots=remaining_splitlots+1";
        lQuery += " WHERE db_update_name='BINNING_CONSOLIDATION'";
        clGexDbQuery.Execute(lQuery);
        lQuery = "UPDATE global_info SET incremental_splitlots=(SELECT SUM(remaining_splitlots) FROM incremental_update)";
        clGexDbQuery.Execute(lQuery);
    }

    // Commit?
    if (m_bStopProcess)
        CleanInvalidSplitlot();

    // Close STDF file
    m_clStdfParse.Close();

    // for multi wafer only
    if(!m_bStopProcess && (m_nCurrentWaferIndex>1) && (((m_eTestingStage == eWaferTest) || (m_eTestingStage == eElectTest))))
    {
        // recursif call for other Wafer Index
        // until parse all wafer records
        if (m_nWaferIndexToProcess < m_nCurrentWaferIndex)
            InsertStdfFile(strStdfFileName, m_nWaferIndexToProcess + 1);
    }

    // Force progress bar to completed
    ResetProgress(true);

    if (m_nWaferIndexToProcess == nWaferIndexToProcess)
    {
        if (m_bStopProcess)
            ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
    }

    return ! m_bStopProcess;
}

//////////////////////////////////////////////////////////////////////
// Read MIR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessVUR()
{
    if (m_bStopProcess)
        return true;

    if (m_lPass == 1)
    {

        // Check if file date is not more recent than license expiration date!
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfVUR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSLOG(SYSLOG_SEV_ERROR, "failed to read a record of the STDF file");
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_VUR]++;

        QString strMessage = "[STDF " + m_clStdfVUR.m_cnUPD_NAM + "] ";
        strMessage += "You are inserting a " + m_clStdfVUR.m_cnUPD_NAM + " STDF file. ";
        strMessage += "All the fields in this file beyond V4 are not supported in your database version. ";
        strMessage += "Only the V4 fields are reflected in your database. ";
        strMessage += m_clStdfVUR.m_cnUPD_NAM + " specific data will be ignored.";
        WarningMessage(strMessage);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read MIR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessMIR()
{
#if 0
    // Debug to enable insertion of Vishay files with duplicate test nb using InnoDB
    nSeqTest7 = 0;
#endif

    //GSLOG(SYSLOG_SEV_DEBUG, QString("pass %1.").arg( m_lPass));

    if (m_bStopProcess)
        return true;

    // reset list pos
    m_lstTestInfo_Index = -1;

    if (m_lPass == 1)
    {
        // Check if have already load MIR record
        if ((m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDMIRCOUNT)
                && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MIR] > 1))
        {
            QString strMessage;
            strMessage.sprintf("[Record#MIR] MIR record appears multiple time. Only one MIR is allowed! This record was ignored.");
            WarningMessage(strMessage);
            // ignore this record
            return true;
        }

        // Check if file date is not more recent than license expiration date!
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfMIR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSLOG(SYSLOG_SEV_ERROR, "failed to read a record of the STDF file");
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MIR]++;
        m_strLotId = mpDbKeysEngine->dbKeysContent().Get("Lot").toString().simplified();
        m_strTrackingLotId = mpDbKeysEngine->dbKeysContent().Get("TrackingLot").toString().simplified();
        m_strProductName = mpDbKeysEngine->dbKeysContent().Get("Product").toString().simplified();
        m_strWaferId = mpDbKeysEngine->dbKeysContent().Get("Wafer").toString().simplified();
        m_strSubLotId = mpDbKeysEngine->dbKeysContent().Get("SubLot").toString().simplified();

        // For FLEX only
        QString strExecType = m_clStdfMIR.m_cnEXEC_TYP;
        QString strTesterType = m_clStdfMIR.m_cnTSTR_TYP;
        // Check if flex
        if ((strExecType.toUpper() == "IG-XL")
                || (strTesterType.toUpper() == "J750")
                || (strTesterType.toUpper() == "JAGUAR")
                || (strTesterType.toUpper() == "INTEGRAFLEX")
                )
        {
            m_bRemovePinName = true;
        }

        // Debug message
        QString strMessage = " ProcessMIR : ";
        strMessage += "LotId=" + m_strLotId + " - ";
        strMessage += "TrackingLotId=" + m_strTrackingLotId;
        GSLOG(SYSLOG_SEV_DEBUG, QString("%1").arg(strMessage).toLatin1().constData());
        return true;
    }

    if (m_lPass == 2)
    {
        // Check Test conditions meta-data
        // HTH-TOCHECK
        if (checkMetaDataForTestConditions() == false)
            return false;

        // Clear now the TestConditions
        // Will be regenerate with ProcessDTR
        mpDbKeysEngine->dbKeysContent().ClearConditions();

        // with temporary splitlot_id for validation_function and MULTI INSERTION MULTI SQL_THREAD
        if(!StartSplitlotInsertion(true))                return false;

        // Create the TEMPORARY gexdbkeys
        // for STORED PROCEDURES access
        if (! KeysContentDumpToTempTable())
        {
            *m_pbDelayInsertion = true;
            return false;
        }

        // CALL THE STORED PROCEDURE 'INSERTION_VALIDATION'
        int nStatus = 0;
        QString lMessage, lProcedureName, lQuery, lProductName, lTrackingLot, lLot, lSubLot, lWaferId;
        GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

        lProductName = m_strProductName;
        lTrackingLot = m_strTrackingLotId;
        lLot = m_strLotId;
        lSubLot = m_strSubLotId;
        lWaferId = m_strWaferId;

        // Debug message
        lMessage = "calling insertion_validation stored procedure...";
        GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data());

        lProcedureName = NormalizeTableName("_insertion_validation");
        TimersTraceStart("Procedure " + lProcedureName);
        if (m_pclDatabaseConnector->IsMySqlDB())
        {
            QString lWaferIdOut;
            // Try with the 2 options
            // First try to call the procedure with the WaferIdOut parameter
            QStringList lstAddinParameters;
            lstAddinParameters << "@WaferIdOut ," << "";

            while (! lstAddinParameters.isEmpty())
            {
                lWaferIdOut = lstAddinParameters.takeFirst();
                lQuery = "CALL " + lProcedureName;
                lQuery += "(" + QString::number(m_nSplitLotId) + ",";
                lQuery += TranslateStringToSqlVarChar(m_strTrackingLotId);
                lQuery += ",";
                lQuery += TranslateStringToSqlVarChar(m_strLotId);
                lQuery += ",";
                lQuery += TranslateStringToSqlVarChar(m_strWaferId);
                lQuery += ",@strTrackingLot,@strLot," + lWaferIdOut + "@strProductName,@strMessage,@nStatus)";

                // If call successful, break loop
                if (clGexDbQuery.Execute(lQuery))
                {
                    if (lWaferIdOut.isEmpty())
                    {
                        // Message to the user
                        lMessage = "Obsolete definition of "+lProcedureName+" without WaferIdOut parameter. See Gex documentation for update";
                        GSLOG(SYSLOG_SEV_WARNING, lMessage.toLatin1().data());
                    }
                    break;
                }

                // Else check if all attempts done
                if ((clGexDbQuery.lastError().number() != 1318)  // Error Code: 1318. Incorrect number of arguments
                        || (lstAddinParameters.isEmpty()))
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString(" failed to execute query : m_nSplitLotId=%1")
                          .arg(m_nSplitLotId).toLatin1().constData());
                    *m_pbDelayInsertion = true;
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                                clGexDbQuery.lastError().text().toLatin1().constData());
                    return false;
                }
            }

            lQuery = "select @strTrackingLot, @strLot, " + lWaferIdOut + "@strProductName, @strMessage, @nStatus";
            if (! clGexDbQuery.Execute(lQuery))
            {
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            clGexDbQuery.First();
            // Retrieve parameter values
            int iIndex = 0;
            lTrackingLot = clGexDbQuery.value(iIndex++).toString();  // the returned tracking lot
            lLot = clGexDbQuery.value(iIndex++).toString();  // the returned lot
            if (! lWaferIdOut.isEmpty())
                lWaferId = clGexDbQuery.value(iIndex++).toString();  // the returned wafer
            lProductName = clGexDbQuery.value(iIndex++).toString();  // the returned product name
            lMessage = clGexDbQuery.value(iIndex++).toString();  // the returned message
            nStatus = clGexDbQuery.value(iIndex++).toInt();  // nStatus is the return status: 0 is NOK, 1 is OK, 2 delay insertion
        }
        else if (m_pclDatabaseConnector->IsSQLiteDB())
        {
            GSLOG(SYSLOG_SEV_WARNING,
                  (QString("Process MIR: pass %1. SplitLotId=%2. SQLite : insertion_validation stored procedure replacement : Code Me !")
                   .arg(m_lPass)
                   .arg(m_nSplitLotId)).toLatin1().constData());
            //strTrackingLot = ;
            //strLot = ;
            //strProductName = ;
            //strMessage = ;
            nStatus = 1;
        }

        TimersTraceStop("Procedure " + lProcedureName);

        // Read the TEMPORARY gexdbkeys
        // Update the gexdbkeyxcontent object
        // And the SPLITLOT table
        if (! KeysContentUpdateFromTempTable(m_nSplitLotId))
        {
            *m_pbDelayInsertion = true;
            return false;
        }
        else
        {
            // Overload lot and tracking lot with returned one
            if (mpDbKeysEngine->dbKeysContent().Get("TrackingLot").toString() != m_strTrackingLotId)
                lTrackingLot = mpDbKeysEngine->dbKeysContent().Get("TrackingLot").toString();
            if (mpDbKeysEngine->dbKeysContent().Get("Lot").toString() != m_strLotId)
                lLot = mpDbKeysEngine->dbKeysContent().Get("Lot").toString();
            if (mpDbKeysEngine->dbKeysContent().Get("SubLot").toString() != m_strSubLotId)
                lSubLot = mpDbKeysEngine->dbKeysContent().Get("SubLot").toString();
            if (mpDbKeysEngine->dbKeysContent().Get("Wafer").toString() != m_strWaferId)
                lWaferId = mpDbKeysEngine->dbKeysContent().Get("Wafer").toString();
            if (mpDbKeysEngine->dbKeysContent().Get("Product").toString() != m_strProductName)
                lProductName = mpDbKeysEngine->dbKeysContent().Get("Product").toString();
        }

        // Check if product name has been overloaded. The overloaded productID will be used to populate
        // the xx_lot.product_name and product.product_name fields, the original ProductID from STDF file
        // (or overloaded in config.gexdbkeys) will be used to populate field xx_splitlot.part_typ.
        if (! lProductName.simplified().isEmpty())
            m_strProductName = lProductName;

        // Overload lot and tracking lot with returned one
        if (! m_pclDatabaseConnector->IsSQLiteDB())
        {
            m_strTrackingLotId = lTrackingLot;
            m_strLotId = lLot;
            m_strSubLotId = lSubLot;

            if (! lWaferId.simplified().isEmpty())
                m_strWaferId = lWaferId;
        }

        if (nStatus == 0)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Process MIR fail : nStatus = 0");
            GSET_ERROR2(GexDbPlugin_Base, eDB_InsertionValidationProcedure, NULL,
                        lProcedureName.toLatin1().constData(),
                        QString("Status[%1] Message[%2]").arg(nStatus).arg(lMessage).toLatin1().constData());
            return false;
        }
        if (nStatus == 2)
        {
            GSLOG(SYSLOG_SEV_ERROR, "ProcessMIR : error : nStatus = 2 (delay insertion) !");
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_InsertionValidationProcedure, NULL,
                        lProcedureName.toLatin1().constData(),
                        QString("Status[%1] Message[%2]").arg(nStatus).arg(lMessage).toLatin1().constData());
            return false;
        }

        // Check if have a warning to display
        if (! lMessage.isEmpty() && lMessage != "Success")
            WarningMessage(QString("Call Procedure %1 - Status[%2] Message[%3]").arg(lProcedureName).arg(nStatus).arg(lMessage));

        if ((m_eTestingStage == eFinalTest)
                && m_strSubLotId.isEmpty())
        {
            // GCORE-1080 - FT Sublot: allow empty SubLot for Lot consolidation level
            // For Final Test
            // For Lot level Consolidation
            // if the option INSERTION_FT_ALLOW_EMPTY_SUBLOT is true
            // copy Lot into SubLot in order to allow inserting files with empty SubLot ID
            // Insertion will be rejected by the ValidationFunction
            QString lValue;
            GetGlobalOptionValue(eInsertionFtAllowEmptySubLot, lValue);
            if (lValue == "TRUE")
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("Option INSERTION_FT_ALLOW_EMPTY_SUBLOT = %1 - Copy Lot ID into Sublot ID")
                      .arg(lValue).toLatin1().constData());
                mpDbKeysEngine->dbKeysContent().Set("SubLot", QVariant(m_strLotId));
                m_strSubLotId = m_strLotId;
                // The update of the Splitlot table is done just after
                // see: Update SplitLot table with new lotID
            }
        }

        // Get current SYA limit set for this product if any
        lQuery = "SELECT SYA_ID FROM " + NormalizeTableName("_SYA_SET");
        lQuery += " WHERE PRODUCT_ID=" + TranslateStringToSqlVarChar(m_strProductName);
        lQuery += " ORDER BY CREATION_DATE DESC";
        if (! clGexDbQuery.Execute(lQuery))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Query exec failed : %1")
                  .arg(clGexDbQuery.lastError().text()).toLatin1().constData());
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        if (clGexDbQuery.First())
        {
            // Update Sya_ID table with new lotID
            lQuery = "UPDATE " + NormalizeTableName("_SPLITLOT");
            lQuery += " SET SYA_ID=" + TranslateStringToSqlVarChar(QString::number(clGexDbQuery.value(0).toUInt()));
            lQuery += " WHERE SPLITLOT_ID=" + QString::number(m_nSplitLotId);
            if (! clGexDbQuery.Execute(lQuery))
            {
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }

        // Update SplitLot table with new lotID
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Update SplitLot table with new lotID : %1")
              .arg(m_strLotId).toLatin1().constData());
        lQuery = "UPDATE " + NormalizeTableName("_SPLITLOT");
        lQuery += " SET LOT_ID=" + TranslateStringToSqlVarChar(m_strLotId);
        lQuery += " , SUBLOT_ID=" + TranslateStringToSqlVarChar(m_strSubLotId);
        if (m_eTestingStage != eFinalTest)
            lQuery += " , WAFER_ID=" + TranslateStringToSqlVarChar(m_strWaferId);
        lQuery += " WHERE SPLITLOT_ID=" + QString::number(m_nSplitLotId);
        if (! clGexDbQuery.Execute(lQuery))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("%1").arg(clGexDbQuery.lastError().text()).toLatin1().constData());
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }


        // Check if data already inserted, using overloaded lotID
        if (! ValidationFunction(true))  // m_lPass==2
        {
            GSLOG(SYSLOG_SEV_WARNING, "ValidationFunction failed");
            return false;
        }

        TimersTraceStop("Procedure " + lProcedureName);
        // Debug message
        lMessage = "---- GexDbPlugin_Galaxy::ProcessMIR: ";
        lMessage += "LotId=" + m_strLotId + " - ";
        lMessage += "TrackingLotId=" + m_strTrackingLotId;
        WriteDebugMessageFile(lMessage);

        // with good splitlot_id for data insertion
        if(!StartSplitlotInsertion())            return false;
        if(!UpdatePartsStatsSummaryTable())      return false;
        if(!UpdateHBinTable())                   return false;
        if(!UpdateSBinTable())                   return false;
        if(!UpdateHBinStatsSummaryTable())       return false;
        if(!UpdateSBinStatsSummaryTable())       return false;
        if(!UpdatePMRInfoTable())                return false;
        if(!UpdatePTestInfoTable())              return false;
        if(!UpdateFTestInfoTable())              return false;
        if(!UpdateMPTestInfoTable())             return false;
        if(!UpdateTestConditionTable())          return false;

        // Move the call of this function after Summary update
        // Call xx_insertion_preprocessing procedure
        if (PreprocessingProcedure(m_nSplitLotId) == false)
            return false;
    }

    m_nRunId = 1;
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read SDR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessSDR()
{
    if (m_bStopProcess)
        return true;

    // Read record
    TimersTraceStart("Record Read");
    if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfSDR) == false)
    {
        // Error reading STDF file
        // Convertion failed.
        GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
        return false;
    }
    TimersTraceStop("Record Read");

    // Pass#1:
    if (m_lPass == 1)
    {
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SDR]++;
        return true;
    }

    // Remaining code only executed in pass 2
    QString lMessage, lQuery, lTableName = NormalizeTableName("_SDR");

    // Debug message
    lMessage = QString("---- GexDbPlugin_Galaxy::ProcessSDR (SITE_GRP=%1, SITE_CNT=%2").arg(m_clStdfSDR.m_u1SITE_GRP)
            .arg(m_clStdfSDR.m_u1SITE_CNT);
    WriteDebugMessageFile(lMessage);

    // Insert into xx_sdr table
    for (int i = 0; i < m_clStdfSDR.m_u1SITE_CNT; i++)
    {
        lQuery = QString
                ("INSERT INTO %1 VALUES(%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,%21)")
                .arg(lTableName)
                .arg(m_nSplitLotId)
                .arg(m_clStdfSDR.m_u1SITE_GRP)
                .arg(i)
                .arg(m_clStdfSDR.m_ku1SITE_NUM[i])
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnHAND_TYP))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnHAND_ID))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnCARD_TYP))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnCARD_ID))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnLOAD_TYP))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnLOAD_ID))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnDIB_TYP))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnDIB_ID))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnCABL_TYP))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnCABL_ID))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnCONT_TYP))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnCONT_ID))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnLASR_TYP))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnLASR_ID))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnEXTR_TYP))
                .arg(TranslateStringToSqlVarChar(m_clStdfSDR.m_cnEXTR_ID));

        // Execute insertion query
        if (! InsertSplitlotDataIntoGexdb(lTableName, lQuery))
            return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read MRR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessMRR()
{
    if (m_bStopProcess)
        return true;

    // Pass#1:
    if (m_lPass == 1)
    {
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfMRR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MRR]++;
        // Debug message
        QString strMessage = "---- GexDbPlugin_Galaxy::ProcessMRR: ";
        strMessage += "PTR=" + QString::number(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PTR]) + " - ";
        strMessage += "FTR=" + QString::number(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_FTR]) + " - ";
        strMessage += "MPR=" + QString::number(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR]);
        WriteDebugMessageFile(strMessage);

        return true;
    }

    if (m_lPass == 2)
    {
        // Debug message
        QString strMessage = "---- GexDbPlugin_Galaxy::ProcessMRR: ";
        strMessage += "PTR=" + QString::number(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PTR]) + " - ";
        strMessage += "FTR=" + QString::number(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_FTR]) + " - ";
        strMessage += "MPR=" + QString::number(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR]);
        WriteDebugMessageFile(strMessage);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read PCR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessPCR()
{
    if (m_bStopProcess)
        return true;

    if (m_lPass == 1)
    {
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfPCR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        // Check if have all needed record
        // else ignore this record
        if(!m_clStdfPCR.IsFieldValid(GQTL_STDF::Stdf_PCR_V4::eposPART_CNT) || !m_clStdfPCR.IsFieldValid(GQTL_STDF::Stdf_PCR_V4::eposGOOD_CNT))
            return true;

        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR]++;
        // update information
        return UpdatePCRInfo(m_clStdfPCR);
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
// Read HBR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessHBR()
{
    if (m_bStopProcess)
        return true;

    if (m_lPass == 1)
    {
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfHBR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        if (((int) m_clStdfHBR.m_u2HBIN_NUM == 0)
                && ((int) m_clStdfHBR.m_u4HBIN_CNT == 0))
            // invalid record
            // ignore it
            return true;

        // For each invalid SBin
        // We must know if we need to map it
        // or if we ahve to ignore it
        if (m_clStdfHBR.m_u2HBIN_NUM > 32767)
        {
            if (mInvalidHBinToSBin.contains(m_clStdfHBR.m_u2HBIN_NUM))
            {
                // Have the option force invalid SBin with HBin
                // Have the association between SBin and HBin
                m_clStdfHBR.m_u2HBIN_NUM = mInvalidHBinToSBin[m_clStdfHBR.m_u2HBIN_NUM];
            }
        }

        if (m_clStdfHBR.m_u2HBIN_NUM > 32767)
            return true;

        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR]++;
        // update information
        return UpdateHBinInfo(m_clStdfHBR);
    }

    if (m_lPass == 2)
    {
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read SBR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessSBR()
{
    if (m_bStopProcess)
        return true;

    if (m_lPass == 1)
    {
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfSBR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        if (((int) m_clStdfSBR.m_u2SBIN_NUM == 0)
                && ((int) m_clStdfSBR.m_u4SBIN_CNT == 0))
            // invalid record
            // ignore it
            return true;

        // For each invalid SBin
        // We must know if we need to map it
        // or if we ahve to ignore it
        if (m_clStdfSBR.m_u2SBIN_NUM > 32767)
        {
            if (mInvalidSBinToHBin.contains(m_clStdfSBR.m_u2SBIN_NUM))
            {
                // Have the option force invalid SBin with HBin
                // Have the association between SBin and HBin
                m_clStdfSBR.m_u2SBIN_NUM = mInvalidSBinToHBin[m_clStdfSBR.m_u2SBIN_NUM];
            }
        }

        // If invalid
        // Ignore it
        if (m_clStdfSBR.m_u2SBIN_NUM > 32767)
            return true;

        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SBR]++;
        // update information
        UpdateSBinInfo(m_clStdfSBR);

        if (mSBinToHBin[m_clStdfSBR.m_u2SBIN_NUM] > 32767)
        {
            // INVALID HBIN
            m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR]++;

            // MAP HBIN WITH SBIN
            m_clStdfHBR.m_c1HBIN_PF = m_clStdfSBR.m_c1SBIN_PF;
            m_clStdfHBR.m_cnHBIN_NAM = m_clStdfSBR.m_cnSBIN_NAM;
            m_clStdfHBR.m_u1HEAD_NUM = m_clStdfSBR.m_u1HEAD_NUM;
            m_clStdfHBR.m_u1SITE_NUM = m_clStdfSBR.m_u1SITE_NUM;
            m_clStdfHBR.m_u2HBIN_NUM = m_clStdfSBR.m_u2SBIN_NUM;
            m_clStdfHBR.m_u4HBIN_CNT = m_clStdfSBR.m_u4SBIN_CNT;

            UpdateHBinInfo(m_clStdfHBR);
        }
    }

    if (m_lPass == 2)
    {
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read RDR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessRDR()
{
    if (m_bStopProcess)
        return true;

    // Pass#1:
    if (m_lPass == 1)
    {
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfRDR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_RDR]++;
    }

    if (m_lPass == 2)
    {
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read WIR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessWIR()
{
    if (m_bStopProcess)
        return true;

    if (m_eTestingStage == eFinalTest)
        return true;

    m_nCurrentWaferIndex++;
    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR]++;
    // to process multi wafer in stdf file
    if (m_nWaferIndexToProcess != m_nCurrentWaferIndex)
    {
        if (m_nWaferIndexToProcess > m_nCurrentWaferIndex)
        {
            // have a WIR
            // goto the good WRR
            // Read one record from STDF file.
            TimersTraceStart("Record Load");
            m_iStatus = m_clStdfParse.LoadNextRecord(&m_nRecordType);
            TimersTraceStop("Record Load");

            m_nNbHeadPerWafer = 1;
            // jump to WRR (corresponding to the current WIR found)
            while((m_iStatus == GQTL_STDF::StdfParse::NoError) || (m_iStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
            {
                // if WIR before WRR
                // multi head
                if (m_nRecordType == GQTL_STDF::Stdf_Record::Rec_WIR)
                {
                    m_nStdfRecordsCount[m_nRecordType]++;
                    m_nNbHeadPerWafer++;
                    m_bHaveMultiHeadWafer = true;
                }
                // have to find the good WRR before break
                if (m_nRecordType == GQTL_STDF::Stdf_Record::Rec_WRR)
                {
                    m_nStdfRecordsCount[m_nRecordType]++;
                    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] == m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR])
                        break;  // have found the end of the wafer
                }
                TimersTraceStart("Record Load");
                m_iStatus = m_clStdfParse.LoadNextRecord(&m_nRecordType);
                TimersTraceStop("Record Load");
            }
            if (m_nRecordType != GQTL_STDF::Stdf_Record::Rec_WRR)
            {
                GSET_ERROR4(GexDbPlugin_Base, eValidation_CountMismatch, NULL, "WIR",m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR], "WRR",m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR]);
                return false;
            }
        }
        else
        {
            // if multi head, don't skeep this wafer
            if (m_nWaferIndexToProcess + m_nNbHeadPerWafer == m_nCurrentWaferIndex)
            {
                if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR]+2 == m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR])
                {
                    // no WRR for the first WIR found
                    // probably multi head
                    int nHead = m_clStdfWIR.m_u1HEAD_NUM;
                    TimersTraceStart("Record Read");
                    if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfWIR) == false)
                    {
                        // Error reading STDF file
                        // Convertion failed.
                        GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
                        return false;
                    }
                    TimersTraceStop("Record Read");

                    if (nHead != m_clStdfWIR.m_u1HEAD_NUM)
                    {
                        m_bHaveMultiHeadWafer = true;
                        m_nNbHeadPerWafer++;
                        return true;
                    }
                }
            }
            // have a WIR
            // goto the good WRR
            // Read one record from STDF file.
            TimersTraceStart("Record Load");
            m_iStatus = m_clStdfParse.LoadNextRecord(&m_nRecordType);
            TimersTraceStop("Record Load");
            m_nNbHeadPerWafer = 1;
            // jump to WRR (corresponding to the current WIR found)
            while((m_iStatus == GQTL_STDF::StdfParse::NoError) || (m_iStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
            {
                // if WIR before WRR
                // multi head
                if (m_nRecordType == GQTL_STDF::Stdf_Record::Rec_WIR)
                {
                    m_nStdfRecordsCount[m_nRecordType]++;
                    m_nNbHeadPerWafer++;
                    m_bHaveMultiHeadWafer = true;
                }
                // have to find the good WRR before break
                if (m_nRecordType == GQTL_STDF::Stdf_Record::Rec_WRR)
                {
                    m_nStdfRecordsCount[m_nRecordType]++;
                    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] == m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR])
                        break;  // have found the end of the wafer
                }
                TimersTraceStart("Record Load");
                m_iStatus = m_clStdfParse.LoadNextRecord(&m_nRecordType);
                TimersTraceStop("Record Load");
            }
            if (m_nRecordType != GQTL_STDF::Stdf_Record::Rec_WRR)
            {
                if (m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRWRRCOUNT)
                {
                    GSET_ERROR4(GexDbPlugin_Base, eValidation_CountMismatch, NULL, "WIR",m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR], "WRR",m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR]);
                    if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRWRRCOUNT)
                        return false;
                    QString strMessage = "[Record#WIR] ";
                    strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                    WarningMessage(strMessage);
                }
                // Ignore this wafer
                m_nCurrentWaferIndex--;
                m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR]--;
                return true;
            }
        }

    }
    else
    {
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfWIR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");
    }
    UpdateInsertionProgress();
    if (m_lPass == 1)
    {
        if((mpDbKeysEngine->dbKeysContent().IsOverloaded("Wafer"))
                && !mpDbKeysEngine->dbKeysContent().Get("Wafer").toString().isEmpty())
            m_strWaferId = mpDbKeysEngine->dbKeysContent().Get("Wafer").toString();
        else if (m_clStdfWIR.m_cnWAFER_ID.isEmpty() && m_clStdfWRR.m_cnWAFER_ID.isEmpty())
            m_strWaferId = "";
        else if (m_clStdfWIR.m_cnWAFER_ID == m_clStdfWRR.m_cnWAFER_ID)
            m_strWaferId = m_clStdfWIR.m_cnWAFER_ID;
        else if (m_clStdfWIR.m_cnWAFER_ID.isEmpty())
            m_strWaferId = m_clStdfWRR.m_cnWAFER_ID;
        else
            m_strWaferId = m_clStdfWIR.m_cnWAFER_ID;

        // For ET, because there are several WIR/WCR
        // update the dbKeysContent with the good value
        if (! mpDbKeysEngine->dbKeysContent().IsOverloaded("Wafer"))
            mpDbKeysEngine->dbKeysContent().Set("Wafer", m_strWaferId, true);

    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read WRR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessWRR()
{
    if (m_bStopProcess)
        return true;

    if (m_eTestingStage == eFinalTest)
        return true;

    if (m_lPass == 1)
    {
        if (m_nWaferIndexToProcess == m_nCurrentWaferIndex)
        {
            TimersTraceStart("Record Read");
            if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfWRR) == false)
            {
                // Error reading STDF file
                // Convertion failed.
                GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
                return false;
            }
            TimersTraceStop("Record Read");
        }
    }
    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR]++;

    if (m_lPass == 2)
    {
        //return UpdateWaferInfoTable();
    }
    return true;
}

///////////////////////////////////////////////////////////////////////
// Read WIR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessWCR()
{
    if (m_bStopProcess)
        return true;

    if (m_eTestingStage == eFinalTest)
        return true;

    if (m_lPass == 1)
    {
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfWCR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WCR]++;
    }

    if (m_lPass == 2)
    {
    }
    return true;
}

/////////////////////////////////////////////////////////////////////
// Read PIR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessPIR()
{
#if 0
    // Debug to enable insertion of Vishay files with duplicate test nb using InnoDB
    nSeqTest7 = 0;
#endif

    if (m_bStopProcess)
        return true;

    UpdateInsertionProgress();

    TimersTraceStart("Record Read");
    if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfPIR) == false)
    {
        // Error reading STDF file
        // Convertion failed.
        GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
        return false;
    }

    unsigned short siteNumber =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            (m_clStdfParse, m_clStdfPIR);

    TimersTraceStop("Record Read");

    if (m_lPass == 1)
    {
        if (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR] > 0)
        {
            GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported,NULL, QString("Found TSR records before test data (PIR/PRR)").toLatin1().data());
            return false;
        }
        // New PIR PIR PIR PRR PRR PRR
        mTotalNewTests[siteNumber] = 0;
        mTotalInconsistentTests[siteNumber] = 0;
        mTotalTestsExecuted[siteNumber] = 0;
        mLastInconsistentTestNumber.remove(siteNumber);
        mLastInconsistentTestName.remove(siteNumber);
    }

    m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR]++;

    if (m_lPass == 2)
    {
        // Reset all counter
        m_mapTestsExecuted[siteNumber] = 0;
        m_mapTestsFailed[siteNumber] = 0;
        m_mapFirstFailTType[siteNumber] = "";
        m_mapFirstFailTId[siteNumber] = 0;
        m_mapLastFailTType[siteNumber] = "";
        m_mapLastFailTId[siteNumber] = 0;

        // Update Run id
        m_mapRunId[siteNumber] = m_nRunId;
    }

    // reset list pos
    m_lstTestInfo_Index = -1;

    m_nRunId++;

    return true;
}

/////////////////////////////////////////////////////////////////////
// Read PRR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessPRR()
{
    if (m_bStopProcess)
        return true;

    UpdateInsertionProgress();

    // reset list pos
    m_lstTestInfo_Index = -1;

    TimersTraceStart("Record Read");
    if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfPRR) == false)
    {
        // Error reading STDF file
        // Convertion failed.
        GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
        return false;
    }

    unsigned short siteNumber =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            (m_clStdfParse, m_clStdfPRR);

    TimersTraceStop("Record Read");

    if (m_lPass == 1)
    {
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]++;

        // Check if it is a valid PRR
        if (((m_clStdfPRR.m_u2HARD_BIN > 32767) && (m_clStdfPRR.m_u2HARD_BIN < 65535))
                || ((m_clStdfPRR.m_u2SOFT_BIN > 32767) && (m_clStdfPRR.m_u2SOFT_BIN < 65535)))
        {
            GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch,NULL, QString("Invalid HBin or SBin - PartNb[%1]-HBin[%2]-SBin[%3]")
                        .arg(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
                    .arg(m_clStdfPRR.m_u2HARD_BIN)
                    .arg(m_clStdfPRR.m_u2SOFT_BIN).toLatin1().data());
            return false;
        }

        if ((m_clStdfPRR.m_u2NUM_TEST == 65535)
                && (m_clStdfPRR.m_u2HARD_BIN == 65535)
                && (m_clStdfPRR.m_u2SOFT_BIN == 65535))
        {
            if (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR] == 1)
            {
                // Test Plan Delimiter
                // allowed only for the first PRR
                // PIR:1|255
                // PRR:1|255|Test Plan Delimiter|65535|P|65535|65535|
                //                              |num_t|P|h_bin|s_bin|
                m_bIgnoreFirstRun = true;
                return true;
            }
        }

        // All SBin must be associated with a specific and unique HBin

        // Case 1: SBin are valid and HBin are invalid but have SBR
        // PRR:1|8|548|38|P|65535|1|33|29|||466
        // PRR:1|8|549|38|F|65535|219|34|29|||424
        // SBR:||1|682|P
        // SBR:||219|4|F
        // SBin=219 always associated with HBin=65535 and have SBR=219 => HBin=SBin and HBR=SBR=219
        // Need option BINNING_XX_MAP_INVALID_HBIN_WITH_SBIN

        // Case 2: SBin are invalid and HBin are invalid but have valid HBR and SBR
        // PRR:1|0|1161|1031|F|65535|65535|||||1036738|LotID=ZA452101,WafNr=005,X=36,Y=38,Seria
        // PRR:1|1|1162|1071|F|65535|65535|||||1036738|LotID=ZA452101,WafNr=002,X=42,Y=46,Seria
        // HBR:||65535|8|F
        // SBR:||65535|8|F
        // SBin= 65535 always associated with HBin=65535 and SBR=HBR=65535=> HBin=SBin=# and SBR=HBR=65535
        // Need option BINNING_XX_FORCE_INVALID_PART_BIN_WITH

        // Case 3: SBin are invalid and HBin are valid (no SBin info)
        // PRR:1|0|1161|1031|F|12|65535|||||1036738|LotID=ZA452101,WafNr=005,X=36,Y=38,Seria
        // PRR:1|1|1162|1071|F|4|65535|||||1036738|LotID=ZA452101,WafNr=002,X=42,Y=46,Seria
        // All SBin must be invalid

        // Case 4: SBin and HBin are valid, but one SBin is associated with several HBins
        // PRR:1|0|33|301|F|4|11|||||8891
        // PRR:1|0|147|298|F|9|11|||||6875
        // Need option BINNING_XX_ALLOW_HBIN_SBIN_MISMATCH
        if (m_clStdfPRR.m_u2HARD_BIN == 65535)
            mInvalidHBinToSBin[m_clStdfPRR.m_u2HARD_BIN] = m_clStdfPRR.m_u2SOFT_BIN;
        if (m_clStdfPRR.m_u2SOFT_BIN == 65535)
            mInvalidSBinToHBin[m_clStdfPRR.m_u2SOFT_BIN] = m_clStdfPRR.m_u2HARD_BIN;


        if ((m_clStdfPRR.m_u2HARD_BIN == 65535)
                && (m_clStdfPRR.m_u2SOFT_BIN == 65535))
        {
            // Case 2: Force invalid HBin/SBin
            // PRR:1|0|1161|1031|F|65535|1111|||||1036738|LotID=ZA452101,WafNr=005,X=36,Y=38,Seria
            // PRR:1|1|1162|1071|F|65535|1111|||||1036738|LotID=ZA452101,WafNr=002,X=42,Y=46,Seria
            // Then this SBin must be always associated with an invalid HBin
            // For the SBin
            // Check if have the appropriate option
            if (! mInsertionOptions.contains("FORCE_INVALID_PART_BIN_WITH"))
            {
                QString lValue = "-1";
                if (m_eTestingStage == eFinalTest)
                    GetGlobalOptionValue(eBinningFtForceInvalidPartBinWith, lValue);
                if (m_eTestingStage == eWaferTest)
                    GetGlobalOptionValue(eBinningWtForceInvalidPartBinWith, lValue);
                mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"] = lValue;
                // Keep the option into the logs
                mpDbKeysEngine->dbKeysContent().Set("Option[BINNING_FORCE_INVALID_PART_BIN_WITH]", lValue);

                // If there is no results
                // the parts can be ignored if the summary ignores it
                // the parts can be validated, if the option is setted and if the summary counts it
                // If there is some results
                // the parts cannot be ignored
                // the parts must be considere as valid and the summary must count it
                // Check if have the option
                if (mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt() == -1)
                {
                    if (mTotalTestsExecuted[siteNumber] > 0)
                    {
                        GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch,NULL, QString("Unable to ignore Invalid Parts Bin with RESULTS - Invalid HBin and SBin - PartNb[%1]-HBin[%2]-SBin[%3]")
                                    .arg(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
                                .arg(m_clStdfPRR.m_u2HARD_BIN)
                                .arg(m_clStdfPRR.m_u2SOFT_BIN)
                                .toLatin1().data());
                        return false;
                    }
                }

            }

            // Keep the info from this invalid Parts
            if (mInsertionOptions.contains("UNVALID_PARTS_BIN_INFO"))
                mInsertionOptions["UNVALID_PARTS_BIN_INFO"] = mInsertionOptions["UNVALID_PARTS_BIN_INFO"].toString() + "#";

            mInsertionOptions["UNVALID_PARTS_BIN_INFO"] = mInsertionOptions["UNVALID_PARTS_BIN_INFO"].toString() +
                    QString("%1|%2|%3|%4")
                    .arg(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
                    .arg(m_clStdfPRR.m_u2HARD_BIN)
                    .arg(m_clStdfPRR.m_u2SOFT_BIN)
                    .arg(mTotalTestsExecuted[siteNumber]);

            if (mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt() > -1)
            {
                // Check if have the option and that the invalide HBIN is present. If not the PRR must be ignored
                if (isInTheHBinSBinListNumber("65535"))
                {
                    mInvalidHBinToSBin[m_clStdfPRR.m_u2HARD_BIN] = mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt();
                    mInvalidSBinToHBin[m_clStdfPRR.m_u2SOFT_BIN] = mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt();
                    m_clStdfPRR.m_u2HARD_BIN = m_clStdfPRR.m_u2SOFT_BIN = mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt();
                }
                else
                {
                    if (mTotalTestsExecuted[m_clStdfPRR.m_u1SITE_NUM] > 0)
                    {
                        GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch,NULL, QString("Unable to ignore Invalid Parts Bin with RESULTS - Invalid HBin and SBin - PartNb[%1]-HBin[%2]-SBin[%3]")
                                    .arg(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
                                .arg(m_clStdfPRR.m_u2HARD_BIN)
                                .arg(m_clStdfPRR.m_u2SOFT_BIN)
                                .toLatin1().data());
                        return false;
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("ProcessPRR: Ignore PRR record with invalid HBIN/SBIN and no match with an invalid HBR/SBR")).toLatin1().constData());
                        return true;
                    }
                }
            }
        }
        else if (mInsertionOptions.contains("FORCE_INVALID_PART_BIN_WITH"))
        {
            // Found an invalid PIR/PRR not at the end of the file
            // Must be the last one
            QString lPartInfo;
            QStringList lPartsInfo = mInsertionOptions["UNVALID_PARTS_BIN_INFO"].toString().split("#");
            while (! lPartsInfo.isEmpty())
            {
                lPartInfo = lPartsInfo.takeFirst();
                GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch,NULL, QString("Unable to process Invalid Parts Bin - Illegal location - Invalid HBin and SBin - PartNb[%1]-HBin[%2]-SBin[%3]")
                            .arg(lPartInfo.section("|", 0, 0))
                            .arg(lPartInfo.section("|", 1, 1))
                            .arg(lPartInfo.section("|", 2, 2))
                            .toLatin1().data());
            }
            return false;
        }

        // With the new m_clStdfPRR.m_u2SOFT_BIN
        if (! mSBinToHBin.contains(m_clStdfPRR.m_u2SOFT_BIN))
            mSBinToHBin[m_clStdfPRR.m_u2SOFT_BIN] = m_clStdfPRR.m_u2HARD_BIN;

        if ((m_clStdfPRR.m_u2SOFT_BIN <= 32767)
                && (mSBinToHBin[m_clStdfPRR.m_u2SOFT_BIN] != m_clStdfPRR.m_u2HARD_BIN))
        {
            // Error SBin is not associated with the same HBin
            // Case 4: SBin and HBin are valid, but one SBin is associated with several HBins
            // PRR:1|0|33|301|F|4|11|||||8891
            // PRR:1|0|147|298|F|9|11|||||6875
            // Need option BINNING_XX_ALLOW_HBIN_SBIN_MISMATCH
            if (! mInsertionOptions.contains("ALLOW_HBIN_SBIN_MISMATCH"))
            {
                QString lValue = "FALSE";
                if (m_eTestingStage == eFinalTest)
                    GetGlobalOptionValue(eBinningFtAllowHbinSbinMismatch, lValue);
                if (m_eTestingStage == eWaferTest)
                    GetGlobalOptionValue(eBinningWtAllowHbinSbinMismatch, lValue);
                mInsertionOptions["ALLOW_HBIN_SBIN_MISMATCH"] = lValue;
                // Keep the option into the logs
                mpDbKeysEngine->dbKeysContent().Set("Option[BINNING_ALLOW_HBIN_SBIN_MISMATCH]", lValue);
                // Keep the error or the warning
                GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch, NULL,
                            QString("Invalid SBin - PartNb[%1]-HBin[%2]-SBin[%3]-Previous HBin[%4]")
                            .arg(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
                        .arg(m_clStdfPRR.m_u2HARD_BIN)
                        .arg(m_clStdfPRR.m_u2SOFT_BIN)
                        .arg(mSBinToHBin[m_clStdfPRR.m_u2SOFT_BIN]).toLatin1().data());
                // Check if have the option
                if (mInsertionOptions["ALLOW_HBIN_SBIN_MISMATCH"] == "FALSE")
                    return false;

                // Warning for the first part with the mismatch
                QString lWarning = "[Record#PRR] ";
                lWarning += QString(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
                WarningMessage(lWarning);
            }
        }

        if ((m_clStdfPRR.m_u2HARD_BIN == 65535)
                && (m_clStdfPRR.m_u2SOFT_BIN <= 32767))
        {
            // Case 1: Force invalid HBin to Valid SBin
            // PRR:1|8|548|38|P|1|1|33|29|||466
            // PRR:1|8|549|38|F|219|219|34|29|||424
            // Case 2: Force invalid HBin/SBin
            // PRR:1|0|1161|1031|F|1111|1111|||||1036738|LotID=ZA452101,WafNr=005,X=36,Y=38,Seria
            // PRR:1|1|1162|1071|F|1111|1111|||||1036738|LotID=ZA452101,WafNr=002,X=42,Y=46,Seria
            // For HBin to SBin
            // Check if have the appropriate option
            if (! mInsertionOptions.contains("BINNING_MAP_INVALID_HBIN_WITH_SBIN"))
            {
                QString lValue = "FALSE";
                if (m_eTestingStage == eFinalTest)
                    GetGlobalOptionValue(eBinningFtMapInvalidHBinWithSBin, lValue);
                if (m_eTestingStage == eWaferTest)
                    GetGlobalOptionValue(eBinningWtMapInvalidHBinWithSBin, lValue);
                mInsertionOptions["BINNING_MAP_INVALID_HBIN_WITH_SBIN"] = lValue;
                // Keep the option into the logs
                mpDbKeysEngine->dbKeysContent().Set("Option[BINNING_MAP_INVALID_HBIN_WITH_SBIN]", lValue);
                // Keep the error or the warning
                GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch,NULL, QString("Invalid HBin - PartNb[%1]-HBin[%2]-SBin[%3]")
                            .arg(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
                        .arg(m_clStdfPRR.m_u2HARD_BIN)
                        .arg(m_clStdfPRR.m_u2SOFT_BIN).toLatin1().data());
                // Check if have the option
                if (mInsertionOptions["BINNING_MAP_INVALID_HBIN_WITH_SBIN"] == "FALSE")
                    return false;
                // Only for the first
                QString lWarning = "[Record#PRR] Map # to " + QString::number(m_clStdfPRR.m_u2SOFT_BIN) + " for ";
                lWarning += QString(GGET_LASTERRORMSG(GexDbPlugin_Base, this)).section(":", 1);
                WarningMessage(lWarning);
            }
            // This will generate the HBR info
            m_clStdfPRR.m_u2HARD_BIN = m_clStdfPRR.m_u2SOFT_BIN;
        }


        if ((m_clStdfPRR.m_u2SOFT_BIN == 65535)
                && (m_clStdfPRR.m_u2HARD_BIN <= 32767))
        {
            // Case 1: Force invalid SBin to Valid HBin
            // TDR-1210: Binning mismatch: Invalid HBin or SBin - PartNb[10]-HBin[4]-SBin[65535]
            // For SBin to HBin
            // Check if have the appropriate option
            if (! mInsertionOptions.contains("BINNING_MAP_INVALID_SBIN_WITH_HBIN"))
            {
                QString lValue = "FALSE";
                if (m_eTestingStage == eFinalTest)
                    GetGlobalOptionValue(eBinningFtMapInvalidSBinWithHBin, lValue);
                if (m_eTestingStage == eWaferTest)
                    GetGlobalOptionValue(eBinningWtMapInvalidSBinWithHBin, lValue);
                mInsertionOptions["BINNING_MAP_INVALID_SBIN_WITH_HBIN"] = lValue;
                // Keep the option into the logs
                mpDbKeysEngine->dbKeysContent().Set("Option[BINNING_MAP_INVALID_SBIN_WITH_HBIN]", lValue);
                // Keep the error or the warning
                GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch,NULL, QString("Invalid SBin - PartNb[%1]-HBin[%2]-SBin[%3]")
                            .arg(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
                        .arg(m_clStdfPRR.m_u2HARD_BIN)
                        .arg(m_clStdfPRR.m_u2SOFT_BIN).toLatin1().data());
                // Check if have the option
                if (mInsertionOptions["BINNING_MAP_INVALID_SBIN_WITH_HBIN"] == "FALSE")
                    return false;
                // Only for the first
                QString lWarning = "[Record#PRR] Map # to " + QString::number(m_clStdfPRR.m_u2HARD_BIN) + " for ";
                lWarning += QString(GGET_LASTERRORMSG(GexDbPlugin_Base, this)).section(":", 1);
                WarningMessage(lWarning);
            }
            // This will generate the HBR info
            m_clStdfPRR.m_u2SOFT_BIN = m_clStdfPRR.m_u2HARD_BIN;
        }

        // Test result valid
        QString strBuffer;
        bool bValid = false;
        bool bPass = false;

        // Check if Pass/Fail flag valid
        if ((m_clStdfPRR.m_b1PART_FLG & STDF_MASK_BIT4) == 0)
        {
            bValid = true;
            bPass = true;
            if (m_clStdfPRR.m_b1PART_FLG & STDF_MASK_BIT3)
                bPass = false;
        }

        m_mapNbRuns[MERGE_SITE]++;
        if (bPass)
            m_mapNbRunsGood[MERGE_SITE]++;

        // Update Binning Info
        if (! UpdateHBinInfo(m_clStdfPRR))
            return false;
        if (! UpdateSBinInfo(m_clStdfPRR))
            return false;

        // Check if Bin cat matches with PRR flags
        if (bValid && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR] > 1))
        {
            QString strMessage;
            if (bPass != (m_mapHBinInfo[m_clStdfPRR.m_u2HARD_BIN].m_cBinCat == 'P'))
            {
                if (mInsertionOptions["AllowBinPRRMismatch"].toString() != "IGNORE")
                {
                    strMessage = "PRR and HardBin Pass/fail flags does not match.";
                    strMessage += " For run#%1, PRR Pass/Fail flag is %2 and HBin#%3 Cat is '%4'";
                    strMessage =
                            strMessage.arg(QString::number(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]),
                            (bPass ? "Pass" : "Fail"),
                            QString::number(m_clStdfPRR.m_u2HARD_BIN),
                            QString((QChar) m_mapHBinInfo[m_clStdfPRR.m_u2HARD_BIN].m_cBinCat));
                    if (mInsertionOptions["AllowBinPRRMismatch"].toString() == "ERROR")
                    {
                        GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch, NULL, strMessage.toLatin1().constData());
                        return false;
                    }
                    WarningMessage(strMessage);
                    // Then ignore for all other runs
                    mInsertionOptions["AllowBinPRRMismatch"] = "IGNORE";
                }
            }
            if (bPass != (m_mapSBinInfo[m_clStdfPRR.m_u2SOFT_BIN].m_cBinCat == 'P'))
            {
                if (mInsertionOptions["AllowBinPRRMismatch"].toString() != "IGNORE")
                {
                    strMessage = "PRR and SoftBin Pass/fail flags does not match.";
                    strMessage += " For run#%1, PRR Pass/Fail flag is %2 and SBin#%3 Cat is '%4'";
                    strMessage =
                            strMessage.arg(QString::number(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]),
                            (bPass ? "Pass" : "Fail"),
                            QString::number(m_clStdfPRR.m_u2SOFT_BIN),
                            QString((QChar) m_mapSBinInfo[m_clStdfPRR.m_u2SOFT_BIN].m_cBinCat));
                    if (mInsertionOptions["AllowBinPRRMismatch"].toString() == "ERROR")
                    {
                        GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch, NULL, strMessage.toLatin1().constData());
                        return false;
                    }
                    WarningMessage(strMessage);
                    // Then ignore for all other runs
                    mInsertionOptions["AllowBinPRRMismatch"] = "IGNORE";
                }
            }
            if (m_mapSBinInfo[m_clStdfPRR.m_u2SOFT_BIN].m_cBinCat != m_mapHBinInfo[m_clStdfPRR.m_u2HARD_BIN].m_cBinCat)
            {
                if (mInsertionOptions["AllowBinPRRMismatch"].toString() != "IGNORE")
                {
                    strMessage = "HardBin and SoftBin Pass/fail flags does not match.";
                    strMessage += " For run#%1, HBin#%2 Cat is '%3' and SBin#%4 Cat is '%5'";
                    strMessage =
                            strMessage.arg(QString::number(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]),
                            QString::number(m_clStdfPRR.m_u2HARD_BIN),
                            QString(m_mapHBinInfo[m_clStdfPRR.m_u2HARD_BIN].m_cBinCat),
                            QString::number(m_clStdfPRR.m_u2SOFT_BIN),
                            QString((QChar) m_mapSBinInfo[m_clStdfPRR.m_u2SOFT_BIN].m_cBinCat));
                    if (mInsertionOptions["AllowBinPRRMismatch"].toString() == "ERROR")
                    {
                        GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch, NULL, strMessage.toLatin1().constData());
                        return false;
                    }
                    WarningMessage(strMessage);
                    // Then ignore for all other runs
                    mInsertionOptions["AllowBinPRRMismatch"] = "IGNORE";
                }
            }
        }

        if (! m_mapNbRuns.contains(siteNumber))
        {
            m_nNbSites++;
            m_mapNbRuns[siteNumber] = 0;
            m_mapNbRunsGood[siteNumber] = 0;
        }


        m_mapNbRuns[siteNumber]++;
        if (bPass)
            m_mapNbRunsGood[siteNumber]++;

        // Check the integrity of the file
        // Analyze 10% of the file before to check the integrity
        // too many duplicated test numbers found
        // Check if the number of test executed is <= MaxTestExecuted
        // how many tests executed into the run
        // how many tests defined during the run
        // if more than the 1/2 test executed is new
        // how many time you find this condition
        // The number of test extecuted if stored into each PRR

        bool lConsistentPart = true;
        bool lConsistentFlow = true;
        QString lInconsistentMessage;
        lConsistentFlow = (mTotalNewTests[siteNumber] == 0);

        // GCORE-6768
        // If stop on Fail, the number of test executed can vary until we have a good part
        // Accept to have new tests at the end of the flow
        if ((m_mapNbRunsGood[MERGE_SITE] < 1  // no Pass found
             || (bPass && m_mapNbRunsGood[MERGE_SITE] == 1)) // First Pass
                && ! lConsistentFlow)
        {
            // Check if the number of tests executed matchs with the total tests
            // A Consistent flow will always have the same number of tests executed as the total number of test created
            if (mTotalTestsExecuted[siteNumber] == (mTotalTestsCreatedForReferenceFlows + mTotalNewTests[siteNumber]))
                lConsistentFlow = true;
        }
        // GCORE-4756
        QString lKeyReferenceFlow =
                QString("Site[%1] HBin[%2] SBin[%3]")
                .arg(siteNumber)
                .arg(m_clStdfPRR.m_u2HARD_BIN)
                .arg(m_clStdfPRR.m_u2SOFT_BIN);

        // Allow 3 distinct flows per Site/Pass/Fail
        // For the first referenced flows
        if (! mReferenceFlows.contains(lKeyReferenceFlow))
            mReferenceFlows[lKeyReferenceFlow] = 1;

        // Must have at least one execution
        if (
                // All sites must have at least one part for the PASS/FAIL flag
                // Allow inconsistency for the first execution of the SITE at PASS and FAIL flag
                ((bPass && (m_mapNbRunsGood[siteNumber] > 1))
                 || (! bPass && ((m_mapNbRuns[siteNumber] - m_mapNbRunsGood[siteNumber]) > 1)))
                &&
                // If have New Tests
                ! lConsistentFlow
                )
        {
            // Allow 3 distinct flows per Site/Binning
            ++mReferenceFlows[lKeyReferenceFlow];
            // No more than 3 disctinct flows
            if (mReferenceFlows[lKeyReferenceFlow] > 2)
            {
                lConsistentPart = false;
                // Get the last Test created
                if (mLastInconsistentTestNumber.contains(siteNumber))
                {
                    lInconsistentMessage = QString(" - (PartId[%1] %2 Flag[%3] - %4 new Tests for %5 Tests executed - last Inconsistent Test Number %6)")
                            .arg(m_clStdfPRR.m_cnPART_ID)
                            .arg(lKeyReferenceFlow)
                            .arg(bPass ? "P" : "F")
                            .arg(mTotalNewTests[siteNumber])
                            .arg(mTotalTestsExecuted[siteNumber])
                            .arg(mLastInconsistentTestNumber[siteNumber]);
                    WarningMessage(QString("Inconsistent Test detected") + lInconsistentMessage);
                }
                else if (mLastInconsistentTestName.contains(siteNumber))
                {
                    lInconsistentMessage = QString(" - (PartId[%1] %2 Flag[%3] - %4 new Tests for %5 Tests executed - last Inconsistent Test Name %6)")
                            .arg(m_clStdfPRR.m_cnPART_ID)
                            .arg(lKeyReferenceFlow)
                            .arg(bPass ? "P" : "F")
                            .arg(mTotalNewTests[siteNumber])
                            .arg(mTotalTestsExecuted[siteNumber])
                            .arg(mLastInconsistentTestName[siteNumber]);
                    WarningMessage(QString("Inconsistent Test detected") + lInconsistentMessage);
                }

            }
        }

        mTotalTestsCreated += mTotalNewTests[siteNumber];

        if (lConsistentPart)
        {
            ++mConsistentPartsFlow;
            mConsistentTests += mTotalNewTests[siteNumber];
            // For the first execution of this flow
            // Must check inconsistency between Sites flow
            // Must check inconsistency between Pass/Fail flow
            // With this info, we can decide how many Tests can be created according to all the Sites/Pass/Fail flow
            if (mTotalTestsCreatedForReferenceFlows == 0)
                mTotalTestsCreatedForReferenceFlows = mTotalTestsExecuted[siteNumber];
            else
                mTotalTestsCreatedForReferenceFlows += mTotalNewTests[siteNumber];
        }
        else
        {
            ++mInconsistentPartsFlow;
            mInconsistentTests += mTotalInconsistentTests[siteNumber];


            int lTotalTests = mTotalTestsCreated;
            if (
                    // Process more than 100 parts
                    ((m_mapNbRuns[MERGE_SITE] >= 100)
                     // Inconsistent Parts
                     && ((mInconsistentPartsFlow > mConsistentPartsFlow)
                         // Inconsistent Tests
                         || (mInconsistentTests * 100 > mConsistentTests * 50)
                         )
                     )
                    // 300% max for Tests created compared to the standard flows
                    || (lTotalTests * 100 > mTotalTestsCreatedForReferenceFlows * 300)

                    )
            {

                // Found inconsistent Test flows
                bool bInconsistentTestName = false;
                bool bInconsistentTestNumber = false;

                // First check Inconsistent Test Number
                // if Total Test Names < 80% Total Tests
                QList<QString> lTestDistinctNames;
                QList<QString> lTestNames = mTestNames.keys();
                QString lName;
                while (! lTestNames.isEmpty())
                {
                    lName = lTestNames.takeFirst();
                    if (lTestDistinctNames.contains(lName))
                        continue;
                    lTestDistinctNames.append(lName);
                }
                int lTotalDistinctTestsName = lTestDistinctNames.count();
                QList<qint64> lTestDistinctNumbers;
                QList<qint64> lTestNumbers = mTestNumbers.keys();
                qint64 lNumber;
                while (! lTestNumbers.isEmpty())
                {
                    lNumber = lTestNumbers.takeFirst();
                    if (lTestDistinctNumbers.contains(lNumber))
                        continue;
                    lTestDistinctNumbers.append(lNumber);
                }
                int lTotalDistinctTestsNumber = lTestDistinctNumbers.count();
                // Check Inconsistent Test Number
                // if Total distinct Test Names < 60% Total Tests
                // This means that the same TestName is used with different TestNumber
                if (lTotalDistinctTestsName * 100 < lTotalTests * 60)
                    bInconsistentTestNumber = true;
                else
                {
                    // Check Inconsistent Test Name
                    // if Total distinct Test Numbers < 60% Total Tests
                    // This means that the same TestNumber is used with different TestName
                    if (lTotalDistinctTestsNumber * 100 < lTotalTests * 60)
                        bInconsistentTestName = true;
                }

                lInconsistentMessage += QString(" - (%1 total Tests created for %2 processed Parts with a max of %3 Tests executed from the Reference Flows)")
                        .arg(lTotalTests).arg(m_mapNbRuns[MERGE_SITE]).arg(mTotalTestsCreatedForReferenceFlows);
                if (bInconsistentTestName)
                {
                    lInconsistentMessage.prepend(QString("found %1 distinct TestNumber for %2 distinct TestName")
                                                 .arg(lTotalDistinctTestsNumber).arg(lTotalDistinctTestsName));
                    GSET_ERROR1(GexDbPlugin_Base, eValidation_InconsistentTestName,NULL, lInconsistentMessage.toLatin1().constData());
                    return false;
                }
                else if (bInconsistentTestNumber)
                {
                    lInconsistentMessage.prepend(QString("found %1 distinct TestName for %2 distinct TestNumber")
                                                 .arg(lTotalDistinctTestsName).arg(lTotalDistinctTestsNumber));
                    GSET_ERROR1(GexDbPlugin_Base, eValidation_InconsistentTestNumber,NULL, lInconsistentMessage.toLatin1().constData());
                    return false;
                }


                lInconsistentMessage.prepend(QString("found %1 inconsistent Parts for %2 consistent Parts")
                                             .arg(mInconsistentPartsFlow).arg(mConsistentPartsFlow));

                GSET_ERROR1(GexDbPlugin_Base, eValidation_InconsistentTestFlow,NULL, lInconsistentMessage.toLatin1().constData());
                return false;
            }
        }

        // Reset the key
        mTotalNewTests[siteNumber] = 0;
        mTotalTestsExecuted[siteNumber] = 0;
        mTotalInconsistentTests[siteNumber] = 0;
        mLastInconsistentTestNumber.remove(siteNumber);
        mLastInconsistentTestName.remove(siteNumber);
        return true;

    }

    if (m_lPass == 2)
    {
        if ((m_clStdfPRR.m_u2HARD_BIN == 65535)
                && (m_clStdfPRR.m_u2SOFT_BIN == 65535))
        {
            // Case 2: Force invalid HBin/SBin
            if (mInsertionOptions.contains("FORCE_INVALID_PART_BIN_WITH"))
                m_clStdfPRR.m_u2HARD_BIN = m_clStdfPRR.m_u2SOFT_BIN = mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt();
        }

        if ((m_clStdfPRR.m_u2HARD_BIN == 65535)
                && (m_clStdfPRR.m_u2SOFT_BIN <= 32767))
        {
            // Case 1: Force invalid HBin to Valid SBin
            m_clStdfPRR.m_u2HARD_BIN = m_clStdfPRR.m_u2SOFT_BIN;
        }

        if ((m_clStdfPRR.m_u2SOFT_BIN == 65535)
                && (m_clStdfPRR.m_u2HARD_BIN <= 32767))
        {
            // Case 1: Force invalid SBin to Valid HBin
            m_clStdfPRR.m_u2SOFT_BIN = m_clStdfPRR.m_u2HARD_BIN;
        }

        if (! UpdateRunTable())
            return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read TSR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessTSR()
{
    if (m_bStopProcess)
        return true;

    // Invalid TestNumber
    // Ignore TSR
    if (mMergeByTestName)
        return true;

    structTestInfo* pTest;
    int nTestType = -1;
    // The pass1 creates all needed entries
    // And merge TSR information
    //      Test can be merge by test number (FLEX format) or by test number + normalized test name
    if (m_lPass == 1)
    {
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR]++;
        m_nSplitLotFlags |= FLAG_SPLITLOT_TSUMMARY_OK;

        // From standard STDF file, TSR are always after the program flow (PIR PTR FTR MPR PRR) = m_lstTest already filled
        // When only summary (no PTR, FTR, MPR), have to create pTest from TSR

        // case 6071
        // When the PinName/SequencerName appears into the TestName, YieldMan splits the Test for each PinName.
        // Yield-Man must provides a tools to clean the TestName and merge all splitted Test with the good TestName.
        // TSR/PTR match by test#/test name.
        // If no match, but only 1 test for this test#, match by test#, else create a test with summary only.
        // Pass1 : The pTestInfo structure is updated during the first pass with all PTR/MPR/FTR records
        //         TSR records are included after the program flow
        // Pass2 : The ptest_info table was updated with all info from pTestInfo structure
        //         All tests that must be stored into the DB (samples or summary) must be present in the pTestInfo structure at this step
        //         TSR records directly update the ptest_summary table at this step
        // ==> TSR must be read at Pass1 to update the pTestInfo with all not executed tests (no PTR/MPR/FTR records)
        // ==> PTR and TSR must match on TestNum and TextName (formated without PinName and SequencerName)
        //     else PTR and TSR must match only on TestNum if no duplicate TestNum
        // ==> TSR must be read at Paas2 to update ptest_summary table
        //if((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PTR]==0)
        //&& (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_FTR]==0)
        //&& (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR]==0)
        //)
        {
            TimersTraceStart("Record Read");
            if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfTSR) == false)
            {
                // Error reading STDF file
                // Convertion failed.
                GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
                return false;
            }
            TimersTraceStop("Record Read");

            if ((m_clStdfTSR.m_c1TEST_TYP == 'P') || (m_clStdfTSR.m_c1TEST_TYP == 'p'))
                nTestType = GQTL_STDF::Stdf_Record::Rec_PTR;
            if ((m_clStdfTSR.m_c1TEST_TYP == 'F') || (m_clStdfTSR.m_c1TEST_TYP == 'f'))
                nTestType = GQTL_STDF::Stdf_Record::Rec_FTR;
            if ((m_clStdfTSR.m_c1TEST_TYP == 'M') || (m_clStdfTSR.m_c1TEST_TYP == 'm'))
                nTestType = GQTL_STDF::Stdf_Record::Rec_MPR;

            // This is not possible to create an entry if no type defined
            if (nTestType == -1)
            {
                GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("ProcessTSR: Ignore TSR record Type='%1' Num='%2' Name='%3'")
                                                 .arg(QString(m_clStdfTSR.m_c1TEST_TYP).toLatin1().data())
                                                 .arg(QString::number(m_clStdfTSR.m_u4TEST_NUM).toLatin1().data())
                                                 .arg(QString(m_clStdfTSR.m_cnTEST_NAM).toLatin1().data())).toLatin1().constData());
                return true;
            }

            // case 6488 - 6489
            // Allow Duplicated test name => find TestInfo on Type and TestNumber and TestName
            // Reject Duplicated test name => find TestInfo only on Type and TestNumber
            // Autoincrement test number => find TestInfo only on Type and TestName
            qint64 TestNumber = m_clStdfTSR.m_u4TEST_NUM;
            QString TestName = m_clStdfTSR.m_cnTEST_NAM;
            // Autoincrement test number => may be only the first record have the test name stored
            // In this case, we cannot use the test number to find the TestInfo
            // we must retrieve the original test name used for this test number
            // test number = 0 and test name empry is not allowed
            if(mMergeByTestName && TestName.isEmpty() && mAssociatedTestNames.contains(QString("[%1][%2]").arg(nTestType).arg(m_clStdfTSR.m_u4TEST_NUM)))
                TestName = mAssociatedTestNames[QString("[%1][%2]").arg(nTestType).arg(m_clStdfTSR.m_u4TEST_NUM)];

            if (mMergeByTestName)
                TestNumber = -1;
            else if (mMergeByTestNumber)
                TestName = "";
            else if (! mAllowedDuplicateTestNumbers)
                TestName = "";

            if ((TestNumber == -1) && TestName.isEmpty())
                // not possible to retrieve TestInfo
                // ignore this TSR
                return true;

            // general case => TYPE/TESTNUM
            // AllowDuplicateTestNum => TYPE/TESTNUM/TESTNAME
            // AutoIncrement => TYPE/TESTNAME

            // ManualProdTDR NotAllowDuplicateTestNumber + NotAutoIncrement
            // Chara TDR NotAllowDuplicateTestNumber + [yes|Not]AutoIncrement
            pTest = FindTestInfo(nTestType, TestNumber, TestName);
            if (pTest == NULL)
            {
                // Try with no TestName
                if (! TestName.isEmpty() && mTestNumbers.count(TestNumber) == 1)
                {
                    // Only if no duplicated TestNum
                    pTest = FindTestInfo(nTestType, TestNumber);
                }
                // If found, already have an entry
                if (pTest == NULL)
                {
                    // new entry
                    pTest = CreateTestInfo(nTestType, TestNumber, m_clStdfTSR.m_u4TEST_NUM,
                                           NormalizeTestName(m_clStdfTSR.m_cnTEST_NAM),
                                           m_clStdfTSR.m_cnTEST_NAM);
                }
                if (pTest == NULL)
                    return false;
            }
            else if (! mAllowedDuplicateTestNumbers
                     && ! m_clStdfTSR.m_cnTEST_NAM.isEmpty()
                     && ! pTest->m_strOriginalTestName.isEmpty()
                     && ! mMergeByTestNumber
                     && (NormalizeTestName(pTest->m_strOriginalTestName) != NormalizeTestName(m_clStdfTSR.m_cnTEST_NAM))
                     )
            {

                GSET_ERROR3(GexDbPlugin_Base, eValidation_DuplicateTestNumber, NULL,
                            pTest->m_nOriginalTestNum,
                            m_clStdfTSR.m_cnTEST_NAM.toLatin1().constData(),
                            "TSR");
                return false;
            }

            // Merge TSR info
            if (mMergeByTestNumber
                    || pTest->m_strTestName.isEmpty())
            {
                // Keep the TestName from the TSR record
                pTest->m_strTestName = NormalizeTestName(m_clStdfTSR.m_cnTEST_NAM);
                pTest->m_strOriginalTestName = m_clStdfTSR.m_cnTEST_NAM;
            }
            if (mMergeByTestName)
            {
                if (! mAssociatedTestNames.contains(QString("[%1][%2]").arg(nTestType).arg(m_clStdfTSR.m_u4TEST_NUM)))
                    mAssociatedTestNames[QString("[%1][%2]").arg(nTestType).arg(m_clStdfTSR.m_u4TEST_NUM)] = m_clStdfTSR.m_cnTEST_NAM;
            }

            int lSiteNumber = GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn(m_clStdfParse, m_clStdfTSR);
            int lHeadNumber = GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn(m_clStdfParse, m_clStdfTSR);

            if (lHeadNumber == 255)
                lSiteNumber = MERGE_SITE;

            if(!pTest->mTsrSites.contains(lSiteNumber)) pTest->mTsrSites.append(lSiteNumber);

            if (m_clStdfTSR.m_u4EXEC_CNT != INVALID_INT)
            {
                if(!pTest->mTsrExecCount.contains(lSiteNumber)) pTest->mTsrExecCount[lSiteNumber] = 0;
                pTest->mTsrExecCount[lSiteNumber] += m_clStdfTSR.m_u4EXEC_CNT;
            }
            if (m_clStdfTSR.m_u4FAIL_CNT != INVALID_INT)
            {
                if(!pTest->mTsrFailCount.contains(lSiteNumber)) pTest->mTsrFailCount[lSiteNumber] = 0;
                pTest->mTsrFailCount[lSiteNumber] += m_clStdfTSR.m_u4FAIL_CNT;
            }
            if ((m_clStdfTSR.m_b1OPT_FLAG & STDF_MASK_BIT7) && ! (m_clStdfTSR.m_b1OPT_FLAG & STDF_MASK_BIT0))
            {
                if(!pTest->mTsrMinValue.contains(lSiteNumber))  pTest->mTsrMinValue[lSiteNumber] = m_clStdfTSR.m_r4TEST_MIN;
                pTest->mTsrMinValue[lSiteNumber] = (pTest->mTsrMinValue[lSiteNumber]<m_clStdfTSR.m_r4TEST_MIN) ? pTest->mTsrMinValue[lSiteNumber] : m_clStdfTSR.m_r4TEST_MIN;
            }
            if ((m_clStdfTSR.m_b1OPT_FLAG & STDF_MASK_BIT7) && ! (m_clStdfTSR.m_b1OPT_FLAG & STDF_MASK_BIT1))
            {
                if(!pTest->mTsrMaxValue.contains(lSiteNumber))  pTest->mTsrMaxValue[lSiteNumber] = m_clStdfTSR.m_r4TEST_MAX;
                pTest->mTsrMaxValue[lSiteNumber] = (pTest->mTsrMaxValue[lSiteNumber]>m_clStdfTSR.m_r4TEST_MAX) ? pTest->mTsrMaxValue[lSiteNumber] : m_clStdfTSR.m_r4TEST_MAX;
            }
            if ((m_clStdfTSR.m_b1OPT_FLAG & STDF_MASK_BIT7) && ! (m_clStdfTSR.m_b1OPT_FLAG & STDF_MASK_BIT4))
            {
                if(!pTest->mTsrSum.contains(lSiteNumber))       pTest->mTsrSum[lSiteNumber] = 0;
                pTest->mTsrSum[lSiteNumber] += m_clStdfTSR.m_r4TST_SUMS;
            }
            if ((m_clStdfTSR.m_b1OPT_FLAG & STDF_MASK_BIT7) && ! (m_clStdfTSR.m_b1OPT_FLAG & STDF_MASK_BIT5))
            {
                if(!pTest->mTsrSquareSum.contains(lSiteNumber)) pTest->mTsrSquareSum[lSiteNumber] = 0;
                pTest->mTsrSquareSum[lSiteNumber] += m_clStdfTSR.m_r4TST_SQRS;
            }
            if ((m_clStdfTSR.m_b1OPT_FLAG & STDF_MASK_BIT7) && ! (m_clStdfTSR.m_b1OPT_FLAG & STDF_MASK_BIT2))
            {
                if(!pTest->mTsrTTime.contains(lSiteNumber))     pTest->mTsrTTime[lSiteNumber] = 0;
                pTest->mTsrTTime[lSiteNumber] += m_clStdfTSR.m_r4TEST_TIM;
            }
        }
    }

    // The pass2 inserts all info into GexDb
    // No pass 2
    return true;
}


//////////////////////////////////////////////////////////////////////
// Read PTR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessPTR()
{
    if (m_bStopProcess)
        return true;

    if (m_lPass == 1)
    {
        // collect information on limit
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfPTR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PTR]++;
        m_nSplitLotFlags &= ~FLAG_SPLITLOT_NOTESTRESULTS;
        return UpdateTestInfo(m_clStdfPTR);
    }

    if (m_lPass == 2)
    {
        // Check if test results should be inserted
        if (mpDbKeysEngine->dbKeysContent().Get("IgnoreResults").toString() == "TRUE")
            return true;

        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfPTR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

#if 0
        // Debug to enable insertion of Vishay files with duplicate test nb using InnoDB
        if (m_clStdfPTR.m_u4TEST_NUM == 7)
        {
            m_clStdfPTR.m_u4TEST_NUM = m_clStdfPTR.m_u4TEST_NUM + nSeqTest7 * 100;
            nSeqTest7++;
        }
#endif

        // Check if test was executed
        // Special case ...
        // Datalog test result for TestTime : it is not a "TEST" so BIT4=1 (test not executed) but has a valid result
        bool bValidRun = (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] > 0);  // && ((m_clStdfPTR.m_b1TEST_FLG & STDF_MASK_BIT4) == 0);
        if (! bValidRun)
            return true;

        return UpdatePTestResultsTable();
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
// Read MPR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessMPR()
{
    if (m_bStopProcess)
        return true;

    if (m_lPass == 1)
    {
        // collect information on limit
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfMPR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        // Ignore MPR not executed, missing record information
        // Check if test was executed
        bool bExecuted = (m_clStdfMPR.m_b1TEST_FLG & STDF_MASK_BIT4) == 0;
        if (! bExecuted)
            return true;

        // m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR] was the number of MPR records
        // m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR]++;

        // m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR] is used for the choice between SqlLoad and MultiQuery
        // If the number of results is > iLimitForSqlLoader then use SqlLoader
        // Else use MultiQuery
        // m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR] is now the number of results under MPR records
        int nMPTestCnt = qMin(m_clStdfMPR.m_u2RTN_ICNT, m_clStdfMPR.m_u2RSLT_CNT);
        if (m_clStdfMPR.m_u2RTN_ICNT == 0)
        {
            if (m_clStdfMPR.m_u2RSLT_CNT == 0)
                nMPTestCnt = 1;
            else
                nMPTestCnt = m_clStdfMPR.m_u2RSLT_CNT;
        }
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR] += nMPTestCnt;
        m_nSplitLotFlags &= ~FLAG_SPLITLOT_NOTESTRESULTS;
        return UpdateTestInfo(m_clStdfMPR);
    }

    if (m_lPass == 2)
    {
        // Check if test results should be inserted
        if (mpDbKeysEngine->dbKeysContent().Get("IgnoreResults").toString() == "TRUE")
            return true;

        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfMPR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        // Ignore MPR not executed, missing record information
        // Check if test was executed
        bool bExecuted = ((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] > 0) && ((m_clStdfMPR.m_b1TEST_FLG & STDF_MASK_BIT4) == 0));
        if (! bExecuted)
            return true;

        return UpdateMPTestResultsTable();
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read PMR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessPMR()
{
    // Only for GEXDB TDR > B24
    if (m_uiDbVersionBuild <= GEXDB_DB_VERSION_BUILD_B24)
        return true;

    if (m_bStopProcess)
        return true;

    // Remaining commands only processed in pass 2
    if (m_lPass == 1)
    {
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PMR]++;

        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfPMR) == FALSE)
        {
            // Error reading STDF file
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }

        unsigned short siteNumber =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                (m_clStdfParse, m_clStdfPMR);

        // overwrite head number by deciphering it if needed
        m_clStdfPMR.SetHEAD_NUM
                (
                    GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
                    (m_clStdfParse, m_clStdfPMR)
                    );

        TimersTraceStop("Record Read");

        // Ignore splitlot_id field - add at Pass 2
        // PRIMARY KEY on splitlot_id,tpin_pmrindex,head_num,site_num
        QString lKey = QString::number(m_clStdfPMR.m_u2PMR_INDX) + " - "
                + QString::number(m_clStdfPMR.m_u1HEAD_NUM) + " - "
                + QString::number(siteNumber);
        // splitlot_id,tpin_pmrindex,chan_typ,chan_nam,phy_nam,log_nam,head_num,site_num
        QString lValue = QString::number(m_clStdfPMR.m_u2PMR_INDX) + ",";
        lValue += QString::number(m_clStdfPMR.m_u2CHAN_TYP) + ",";
        lValue += TranslateStringToSqlVarChar(m_clStdfPMR.m_cnCHAN_NAM) + ",";
        lValue += TranslateStringToSqlVarChar(m_clStdfPMR.m_cnPHY_NAM) + ",";
        lValue += TranslateStringToSqlVarChar(m_clStdfPMR.m_cnLOG_NAM) + ",";
        lValue += QString::number(m_clStdfPMR.m_u1HEAD_NUM) + ",";
        lValue += QString::number(siteNumber);

        m_mapPinDef[lKey] = lValue;
        return true;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
// Read FTR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessFTR()
{
    if (m_bStopProcess)
        return true;

    if (m_lPass == 1)
    {
        // collect information on limit
        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfFTR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_FTR]++;
        m_nSplitLotFlags &= ~FLAG_SPLITLOT_NOTESTRESULTS;
        return UpdateTestInfo(m_clStdfFTR);
    }

    if (m_lPass == 2)
    {
        // Check if test results should be inserted
        if (mpDbKeysEngine->dbKeysContent().Get("IgnoreResults").toString() == "TRUE")
            return true;

        TimersTraceStart("Record Read");
        if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfFTR) == false)
        {
            // Error reading STDF file
            // Convertion failed.
            GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
            return false;
        }
        TimersTraceStop("Record Read");

        // Check if test was executed
        // Special case ...
        // Datalog test result for TestTime : it is not a "TEST" so BIT4=1 (test not executed) but has a valid result
        bool bValidRun = (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] > 0);  // && ((m_clStdfFTR.m_b1TEST_FLG & STDF_MASK_BIT4) == 0);
        if (! bValidRun)
            return true;

        return UpdateFTestResultsTable();
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read DTR Record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ProcessDTR()
{
    if (m_bStopProcess)
        return true;

    TimersTraceStart("Record Read");
    if (m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record*) &m_clStdfDTR) == false)
    {
        // Error reading STDF file
        GSET_ERROR0(GexDbPlugin_Base, eStdf_Read, GGET_LASTERROR(StdfParse, &m_clStdfParse));
        return false;
    }
    TimersTraceStop("Record Read");

    if (m_lPass == 1)
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_DTR]++;

    // Get command
    bool lOK;
    unsigned int uiGrossDie;
    QString strDieID, lCommand, strQuery, lMessage;

    // Check if GEX Gross Die command:
    if (m_clStdfDTR.IsGexCommand_PATTestList(lCommand))
    {
        // Debug message
        lMessage = "---- GexDbPlugin_Galaxy::ProcessDTR (GEX PATTestList command: " + m_clStdfDTR.m_cnTEXT_DAT + ")";
        WriteDebugMessageFile(lMessage);

        bool bIsNumber;
        qint64 nTestNumber;
        QString strTestNumber;
        structTestInfo* pTest;

        while (! lCommand.isEmpty())
        {
            pTest = NULL;
            strTestNumber = lCommand.section(",", 0, 0).simplified();
            lCommand = lCommand.section(",", 1).simplified();

            nTestNumber = strTestNumber.toUInt(&bIsNumber);
            if (bIsNumber)
                pTest = FindTestInfo(GQTL_STDF::Stdf_Record::Rec_PTR, nTestNumber);

            if (pTest)
                pTest->m_cFlags |= FLAG_TESTINFO_PAT_ACTIVE;
        }
    }

    // Check if GEX Gross Die command:
    if (m_clStdfDTR.IsGexCommand_GrossDie(&uiGrossDie, &lOK))
    {
        // Debug message
        lMessage = "---- GexDbPlugin_Galaxy::ProcessDTR (GEX GrossDie command: " + m_clStdfDTR.m_cnTEXT_DAT + ")";
        WriteDebugMessageFile(lMessage);

        if (! lOK)
        {
            GSET_ERROR1(GexDbPlugin_Base, eStdf_DtrCommand_BadSyntax, NULL, lCommand.toLatin1().constData());
            return false;
        }
        mpDbKeysEngine->dbKeysContent().Set("GrossDie", uiGrossDie);
    }

    // Check if GEX Die-Tracking command:
    if (m_clStdfDTR.IsGexCommand_DieTracking(strDieID, lCommand, &lOK))
    {
        // Debug message
        lMessage = "---- GexDbPlugin_Galaxy::ProcessDTR (GEX DieTracking command: " + m_clStdfDTR.m_cnTEXT_DAT + ")";
        WriteDebugMessageFile(lMessage);

        if (! lOK)
        {
            GSET_ERROR1(GexDbPlugin_Base, eStdf_DtrCommand_BadSyntax, NULL, lCommand.toLatin1().constData());
            return false;
        }
        // Save mult-die info
        m_mapDieTrackingInfo.insert(strDieID, lCommand);
    }

    // Check if Syntricity Test Condition
    bool lIsTestOK = false;
    QMap<QString, QString> lTestConditions;
    if (m_clStdfDTR.IsSyntricityTestcond(lTestConditions, lIsTestOK))
    {
        // Spec: The condition will apply to all tests following the statement.
        // Any new DTR will clear prior conditions. In other words, the condition statements are not additive.
        // All DTR conditions stay in effect until there are any duplicate DTR conditions which will clear prior identical conditions.
        // To clear all conditions, you can use the following statement: COND: CLEAR

        ++mDtrTestConditionsCount;
        if (! mIgnoreDtrTestConditions)
        {
            // if Test cond in DTR and not a characDB --> reject
            if (! IsCharacTdr())
            {
                GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidDbTypeForDTRTestcond, NULL, NULL);
                return false;
            }
            // if syntax error --> reject
            if (! lIsTestOK)
            {
                GSET_ERROR1(GexDbPlugin_Base, eStdf_DtrSyntricity_BadSyntax, NULL, m_clStdfDTR.m_cnTEXT_DAT.toLatin1().constData());
                return false;
            }
            // if test cond in config file and in DTR -> reject
            if (mpDbKeysEngine->GetTestConditionsOrigin() == GS::QtLib::DatakeysEngine::CONFIG_FILE)
            {
                GSET_ERROR1(GexDbPlugin_Base, eValidation_TestConditionMultipleSources, NULL, NULL);
                return false;
            }

            mpDbKeysEngine->SetTestConditionsOrigin(GS::QtLib::DatakeysEngine::DTR);
            // if everything is OK, save test conditions

            QMapIterator<QString, QString> lCondIter(lTestConditions);
            while (lCondIter.hasNext())
            {
                lCondIter.next();
                // If condition is "CLEAR" it's already applied
                if (lCondIter.key().toUpper() != "CLEAR")
                    mpDbKeysEngine->dbKeysContent().Set(lCondIter.key(), lCondIter.value());
                else
                    mpDbKeysEngine->dbKeysContent().ClearConditions();
            }
        }
    }

    // GCORE-1972 - support multi-limits for PTR test
    if (m_lPass == 1)
    {
        lCommand = "ML";
        QJsonObject lJson = m_clStdfDTR.GetGsJson(lCommand);
        if (! lJson.isEmpty())
        {
            QString lError;
            if (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] == 0)
            {
                lError = QString("this GS JSON DTR was defined before any parts");
                lError += " for " + m_clStdfDTR.m_cnTEXT_DAT;
                GSET_ERROR1(GexDbPlugin_Base, eStdf_DtrCommand_BadUsage, NULL, lError.toLatin1().constData());
                return false;
            }

            GS::Core::MultiLimitItem lLimitItem;
            if (! lLimitItem.LoadFromJSon(lJson, lError) || ! lLimitItem.IsValid())
            {
                lError = "Error when reading DTR record: " + m_clStdfDTR.m_cnTEXT_DAT + " - " + lError;
                GSET_ERROR1(GexDbPlugin_Base, eStdf_DtrCommand_BadSyntax, NULL, lError.toLatin1().constData());
                return false;
            }

            // have a valid GS JSON struct for TYPE=ML
            // ex: ”{"TYPE":"ML","TNUM":2,"LL":-3e-6,"HL":20e-6,"HBIN":1}”
            structTestInfo* pTest = NULL;

            int lNum = GS::Core::MultiLimitItem::ExtractTestNumber(lJson);
            if (lNum == -1)
            {
                lError = "Missing \"TNUM\" key in GS JSON DTR of type \"ML\"";
                lError += " for " + m_clStdfDTR.m_cnTEXT_DAT;
                GSET_ERROR1(GexDbPlugin_Base, eStdf_DtrCommand_BadSyntax, NULL, lError.toLatin1().constData());
                return false;
            }

            QString lName = GS::Core::MultiLimitItem::ExtractTestName(lJson);
            pTest = FindTestInfo(GQTL_STDF::Stdf_Record::Rec_PTR, lNum, lName);
            //pTest = FindTestInfo(GQTL_STDF::Stdf_Record::Rec_MPR, lNum, lName);
            //pTest = FindTestInfo(-1, lNum, lName);
            if (pTest == NULL)
            {
                if (mTestNumbers.contains(lNum))
                {
                    pTest = FindTestInfo(GQTL_STDF::Stdf_Record::Rec_MPR, lNum, lName);
                    if (pTest != NULL)
                    {
                        // MPR test
                        lError = QString("Multi-limits definition for multi-results parametric tests is not yet supported (test %1)").arg(lNum);
                        lError += " for " + m_clStdfDTR.m_cnTEXT_DAT;
                        GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported, NULL, lError.toLatin1().constData());
                        return false;
                    }
                }
                lError = QString("this GS JSON DTR contains a \"TNUM\" (%1) which cannot be associated with any previous PTR").arg(lNum);
                lError += " for " + m_clStdfDTR.m_cnTEXT_DAT;
                GSET_ERROR1(GexDbPlugin_Base, eStdf_DtrCommand_BadUsage, NULL, lError.toLatin1().constData());
                return false;
            }

            // GCORE-6093: Allow to keep duplicate Multi-limit
            pTest->mMultiLimits.append(lLimitItem);
        }


    }
    // Remaining commands only processed in pass 2
    if (m_lPass == 1)
        return true;

    // Check if GEX logPAT command
    if (m_clStdfDTR.IsGexCommand_logPAT(lCommand))
    {
        // Debug message
        lMessage = "---- GexDbPlugin_Galaxy::ProcessDTR (GEX logPAT command: " + m_clStdfDTR.m_cnTEXT_DAT + ")";
        WriteDebugMessageFile(lMessage);

        // Insert into xx_dtr table
        // m_nRunId is initialized to 1 and incremented after each PIR
        // The value associated with a PIR is stored into a RunId per Site
        // To have the correct value, need to use m_nRunId-1
        QString strTableName = NormalizeTableName("_DTR");
        strQuery = "INSERT INTO " + strTableName + " VALUES(";
        strQuery += QString::number(m_nSplitLotId);
        strQuery += ", "+QString::number(m_nRunId-1)+", " + QString::number(m_uiDtrIndex_InSplitlot) + ", 'logPAT', ";
        strQuery += TranslateStringToSqlVarChar(lCommand) + ")";

        // Execute insertion query
        if (! InsertSplitlotDataIntoGexdb(strTableName, strQuery))
            return false;

        m_uiDtrIndex_InSplitlot++;
    }

    return true;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateInsertionProgress(int nProg)
{
    ProcessEvents();

    int nProgress = nProg;
    long lPos;
    GS::StdLib::Stdf* pHandle;

    if(nProgress>100)        nProgress=100;
    if ((nProgress > 0) && (nProgress != mProgress))
    {
        mProgress = nProgress;
        SetProgress(mProgress);
        return true;
    }

    m_clStdfParse.GetStdfHandle(&pHandle);
    lPos = pHandle->GetPos();
    nProgress = (int) ((float) (lPos * 50.0) / (float) m_nFileSize) + (m_lPass - 1) * 50;
    if (m_nTotalWaferToProcess > 1)
    {
        nProgress /= m_nTotalWaferToProcess;
        nProgress += 50;
        nProgress += 50 / m_nTotalWaferToProcess * m_nWaferIndexToProcess;
    }

    if (nProgress > mProgress)
    {
        mProgress = nProgress;
        SetProgress(mProgress);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Test List management
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateTestInfo(const GQTL_STDF::Stdf_PTR_V4& clRecord)
{
    // first find if Test already exist
    // else create new Test Info
    structTestInfo* pTest = NULL;

    // case 6488 - 6489
    // Allow Duplicated test name => find TestInfo on Type and TestNumber and TestName
    // Reject Duplicated test name => find TestInfo only on Type and TestNumber
    // Autoincrement test number => find TestInfo only on Type and TestName
    qint64 TestNumber = clRecord.m_u4TEST_NUM;
    QString TestName = clRecord.m_cnTEST_TXT;
    // Autoincrement test number => may be only the first record have the test name stored
    // In this case, we cannot use the test number to find the TestInfo
    // we must retrieve the original test name used for this test number
    // test number = 0 and test name empry is not allowed
    if(mMergeByTestName && TestName.isEmpty() && mAssociatedTestNames.contains(QString("[%1][%2]").arg(GQTL_STDF::Stdf_Record::Rec_PTR).arg(clRecord.m_u4TEST_NUM)))
        TestName = mAssociatedTestNames[QString("[%1][%2]").arg(GQTL_STDF::Stdf_Record::Rec_PTR).arg(clRecord.m_u4TEST_NUM)];

    if (mMergeByTestName)
        TestNumber = GetAutoIncrementTestNumber(GQTL_STDF::Stdf_Record::Rec_PTR, TestName);
    else if (mMergeByTestNumber)
        TestName = "";
    else if (! mAllowedDuplicateTestNumbers)
        TestName = "";

    if ((TestNumber == -1) && TestName.isEmpty())
    {
        GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidField, NULL,
                    QString("%1 option cannot be used with empty TestName. PTR[%2]")
                    .arg(mAutoincrementTestNumbers ? "Test[number]=Auto" : "TestMergeBy=NAME")
                    .arg(clRecord.m_u4TEST_NUM).toLatin1().constData());
        return false;
    }

    pTest = FindTestInfo(GQTL_STDF::Stdf_Record::Rec_PTR, TestNumber, TestName);

    unsigned short siteNumber =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            (m_clStdfParse, clRecord);

    if (pTest == NULL)
    {
        // To Check Test Inconsistency
        ++mTotalNewTests[siteNumber];
        if (mTestNumbers.contains(TestNumber) || mTestNames.contains(NormalizeTestName(clRecord.m_cnTEST_TXT)))
        {
            ++mTotalInconsistentTests[siteNumber];
            if (mTestNames.contains(NormalizeTestName(clRecord.m_cnTEST_TXT)))
                mLastInconsistentTestNumber[siteNumber] = QString("%2 - %3 - %4")
                        .arg("PTR").arg(TestNumber).arg(NormalizeTestName(clRecord.m_cnTEST_TXT));
            else
                mLastInconsistentTestName[siteNumber] = QString("%2 - %3 - %4")
                        .arg("PTR").arg(TestNumber).arg(NormalizeTestName(clRecord.m_cnTEST_TXT));

        }
        // new entry
        pTest = CreateTestInfo(GQTL_STDF::Stdf_Record::Rec_PTR, TestNumber, clRecord.m_u4TEST_NUM,
                               NormalizeTestName(clRecord.m_cnTEST_TXT),
                               clRecord.m_cnTEST_TXT);
        if (pTest == NULL)
            return false;


    }
    else if (pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_PTR)
    {
        // verify if test is really a PTR
        // error
        GSET_ERROR2(GexDbPlugin_Base, eValidation_DuplicateTest, NULL, pTest->m_nOriginalTestNum, pTest->m_strTestName.toLatin1().constData());
        if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTTYPEREDEFINITION)
            return false;
        QString strMessage = "[Record#PTR] ";
        strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        WarningMessage(strMessage);
        // Ignore this test
        return true;
    }
    else if (! mAllowedDuplicateTestNumbers && ! clRecord.m_cnTEST_TXT.isEmpty() && ! mMergeByTestNumber &&
             (NormalizeTestName(pTest->m_strOriginalTestName) != NormalizeTestName(clRecord.m_cnTEST_TXT)))
    {
        GSET_ERROR3(GexDbPlugin_Base, eValidation_DuplicateTestNumber, NULL
                    , pTest->m_nOriginalTestNum,
                    clRecord.m_cnTEST_TXT.toLatin1().constData(),
                    "PTR");
        return false;
    }

    ++mTotalTestsExecuted[siteNumber];

    bool bSaveLL, bSaveHL;
    bSaveLL = bSaveHL = false;
    // case 7498
    // PTR.OPT_FLAG bit 1 set =	Reserved for future used; must be 1
    // if(clRecord.m_b1OPT_FLAG & STDF_MASK_BIT1) // valid flag
    if (clRecord.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposOPT_FLAG))
    {
        if ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT0) == 0)
        {
            pTest->m_bHaveScalRes = true;
            pTest->m_nScalRes = clRecord.m_i1RES_SCAL;
        }

        // Verify if have new limits
        bSaveLL = (((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT4) == 0)
                   && ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT6) == 0));
        // FOR MULTIPLE LIMITS REDEFINITION
        if (bSaveLL)
        {
            pTest->m_bHaveScalLL = true;
            pTest->m_nScalLL = clRecord.m_i1LLM_SCAL;

            // Have a valid limit
            if (pTest->m_mapfLL.contains(MERGE_SITE))
            {
                // SC 2008 09 17
                // Save the min low limit from all sites
                if (pTest->m_mapfLL[MERGE_SITE] <= clRecord.m_r4LO_LIMIT)
                    bSaveLL = false;

            }
        }
        bSaveHL = (((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT5) == 0)
                   && ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT7) == 0));
        // FOR MULTIPLE LIMITS REDEFINITION
        if (bSaveHL)
        {
            pTest->m_bHaveScalHL = true;
            pTest->m_nScalHL = clRecord.m_i1HLM_SCAL;

            // Have a valid limit
            if (pTest->m_mapfHL.contains(MERGE_SITE))
            {
                // SC 2008 09 17
                // Save the max high limit from all sites
                if (pTest->m_mapfHL[MERGE_SITE] >= clRecord.m_r4HI_LIMIT)
                    bSaveHL = false;
            }
        }

        // then update
        if (bSaveLL)
            pTest->m_mapfLL[MERGE_SITE] = clRecord.m_r4LO_LIMIT;
        if (bSaveHL)
            pTest->m_mapfHL[MERGE_SITE] = clRecord.m_r4HI_LIMIT;
        if (pTest->m_strUnits.isEmpty())
            pTest->m_strUnits = clRecord.m_cnUNITS;

        // Update Strict flag
        // Only for the first saved
        if (bSaveLL && bSaveHL)
        {
            // If LowLimit == HighLimit then no strict limits
            if ((pTest->m_mapfLL.contains(MERGE_SITE))
                    && (pTest->m_mapfHL.contains(MERGE_SITE))
                    && (pTest->m_mapfLL[MERGE_SITE] == pTest->m_mapfHL[MERGE_SITE]))
            {
                pTest->m_cFlags &= ~FLAG_TESTINFO_LL_STRICT;
                pTest->m_cFlags &= ~FLAG_TESTINFO_HL_STRICT;
            }
            else
            {
                if ((clRecord.m_b1PARM_FLG & STDF_MASK_BIT6) == 0)
                    pTest->m_cFlags |= FLAG_TESTINFO_LL_STRICT;
                if ((clRecord.m_b1PARM_FLG & STDF_MASK_BIT7) == 0)
                    pTest->m_cFlags |= FLAG_TESTINFO_HL_STRICT;
            }
        }

        pTest->m_bHaveSpecLL = ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT2) == 0);
        pTest->m_bHaveSpecHL = ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT3) == 0);

        if (pTest->m_bHaveSpecLL)
            pTest->m_fSpecLL = clRecord.m_r4LO_SPEC;

        if (pTest->m_bHaveSpecHL)
            pTest->m_fSpecHL = clRecord.m_r4HI_SPEC;

    }

    // Check if test was executed, and if test result is valid
    // Special case ...
    // Datalog test result for TestTime : it is not a "TEST" so BIT4=1 (test not executed) but has a valid result
    bool bValidRun = (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] > 0);  // && ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT4) == 0);
    bool bValidResult = bValidRun && ! IsTestResultInvalid(clRecord);

    // Check if has a valid result outside a PIR PRR
    if ((m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTRESULTOUTSIDE) &&
            (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] == 0) &&                    // Outside PIR/PRR block
            ! IsTestResultInvalid(clRecord))                                           // Test result is valid: test executed and valid
    {
        QString strMessage = "[Record#PTR] ";
        strMessage += "Found record outside of PIR/PRR bloc. Some results were ignored for test number ";
        strMessage += QString::number(clRecord.m_u4TEST_NUM) + " (" + clRecord.m_cnTEST_TXT + ").";
        WarningMessage(strMessage);
    }

    // GCORE-2044 : Not good. Must now reject. To be reported in the 3 UpdateTestInfo(...).
    if (bValidResult
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] > 0)
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] == m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
            )
    {
        QString strMessage = "[Record#PTR] ";
        strMessage += "Found illegal record outside of PIR/PRR bloc.";
        strMessage += QString::number(clRecord.m_u4TEST_NUM) + " (" + clRecord.m_cnTEST_TXT + ").";
        GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data());
        GSET_ERROR1(GexDbPlugin_Base, eValidation_InconsistentTestResult, NULL, strMessage.toLatin1().constData());
        return false;
    }

    // Test result valid
    int nFail = 0;
    // Check if Pass/Fail flag valid
    if (bValidRun && IsTestFail(clRecord, pTest))
        nFail = 1;

    if (bValidResult)
    {
        if (! pTest->m_mapMinValue.contains(MERGE_SITE))
        {
            pTest->m_mapMinValue[MERGE_SITE] = clRecord.m_r4RESULT;
            pTest->m_mapMaxValue[MERGE_SITE] = clRecord.m_r4RESULT;
        }
        pTest->m_mapSum[MERGE_SITE] += clRecord.m_r4RESULT;
        pTest->m_mapSquareSum[MERGE_SITE] += clRecord.m_r4RESULT * clRecord.m_r4RESULT;
        if (pTest->m_mapMinValue[MERGE_SITE] > clRecord.m_r4RESULT)
            pTest->m_mapMinValue[MERGE_SITE] = clRecord.m_r4RESULT;
        if (pTest->m_mapMaxValue[MERGE_SITE] < clRecord.m_r4RESULT)
            pTest->m_mapMaxValue[MERGE_SITE] = clRecord.m_r4RESULT;
    }

    if (bValidRun)
    {
        if (pTest->m_nTestSeq == -1)
        {
            m_nTestsSequence++;
            pTest->m_nTestSeq = m_nTestsSequence;
        }
        pTest->m_mapExecCount[MERGE_SITE]++;
        pTest->m_mapFailCount[MERGE_SITE] += nFail;
    }

    if (! pTest->m_mapExecCount.contains(siteNumber))
    {
        pTest->m_mapExecCount[siteNumber] = 0;
        pTest->m_mapFailCount[siteNumber] = 0;
        pTest->m_mapSum[siteNumber] = 0;
        pTest->m_mapSquareSum[siteNumber] = 0;
    }
    if (bValidResult)
    {
        if (! pTest->m_mapMinValue.contains(siteNumber))
        {
            pTest->m_mapMinValue[siteNumber] = clRecord.m_r4RESULT;
            pTest->m_mapMaxValue[siteNumber] = clRecord.m_r4RESULT;
        }
        pTest->m_mapSum[siteNumber] += clRecord.m_r4RESULT;
        pTest->m_mapSquareSum[siteNumber] += clRecord.m_r4RESULT * clRecord.m_r4RESULT;
        if (pTest->m_mapMinValue[siteNumber] > clRecord.m_r4RESULT)
            pTest->m_mapMinValue[siteNumber] = clRecord.m_r4RESULT;
        if (pTest->m_mapMaxValue[siteNumber] < clRecord.m_r4RESULT)
            pTest->m_mapMaxValue[siteNumber] = clRecord.m_r4RESULT;
    }

    if (bValidRun)
    {
        pTest->m_mapExecCount[siteNumber]++;
        pTest->m_mapFailCount[siteNumber] += nFail;
    }


    // if new limit definition for this site then error
    // case 7498
    // PTR.OPT_FLAG bit 1 set =	Reserved for future used; must be 1
    // if(clRecord.m_b1OPT_FLAG & STDF_MASK_BIT1)        // valid flag
    if (clRecord.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposOPT_FLAG))
    {
        if (((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT4) == 0)
                && ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT6) == 0))
        {
            if (pTest->m_mapfLL.contains(siteNumber))
            {
                if((pTest->m_mapfLL[siteNumber] != (float)clRecord.m_r4LO_LIMIT) && (pTest->m_mapExecCount[MERGE_SITE] > 1))
                {
                    // limit redifinition after first test execution
                    if(m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTLIMITREDEFINITION)
                    {
                        GSET_ERROR2(GexDbPlugin_Base, eValidation_MultipleLimits, NULL, clRecord.m_u4TEST_NUM, clRecord.m_cnTEST_TXT.toLatin1().constData());
                        if(m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTLIMITREDEFINITION)
                            return false;
                        QString strMessage = "[Record#PTR] ";
                        strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                        WarningMessage(strMessage);
                    }

                    // SC 2008 09 17
                    // Save the min low limit for this site
                    if (pTest->m_mapfLL[siteNumber] > (float) clRecord.m_r4LO_LIMIT)
                        pTest->m_mapfLL[siteNumber] = clRecord.m_r4LO_LIMIT;
                }
            }
            else
                pTest->m_mapfLL[siteNumber] = clRecord.m_r4LO_LIMIT;
        }
        if (((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT5) == 0)
                && ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT7) == 0))
        {
            if (pTest->m_mapfHL.contains(siteNumber))
            {
                if((pTest->m_mapfHL[siteNumber] != (float)clRecord.m_r4HI_LIMIT) && (pTest->m_mapExecCount[MERGE_SITE] > 1))
                {
                    // limit redifinition after first test execution
                    if(m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTLIMITREDEFINITION)
                    {
                        GSET_ERROR2(GexDbPlugin_Base, eValidation_MultipleLimits, NULL, clRecord.m_u4TEST_NUM, clRecord.m_cnTEST_TXT.toLatin1().constData());
                        if(m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTLIMITREDEFINITION)
                            return false;
                        QString strMessage = "[Record#PTR] ";
                        strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                        WarningMessage(strMessage);
                    }

                    // SC 2008 09 17
                    // Save the max high limit for this site
                    if (pTest->m_mapfHL[siteNumber] < (float) clRecord.m_r4HI_LIMIT)
                        pTest->m_mapfHL[siteNumber] = clRecord.m_r4HI_LIMIT;
                }
            }
            else
                pTest->m_mapfHL[siteNumber] = clRecord.m_r4HI_LIMIT;
        }
    }

    pTest = NULL;
    return true;
}

//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateTestInfo(const GQTL_STDF::Stdf_FTR_V4& clRecord)
{
    // first find if Test already exist
    // else create new Test Info
    structTestInfo* pTest = NULL;

    // case 6488 - 6489
    // Allow Duplicated test name => find TestInfo on Type and TestNumber and TestName
    // Reject Duplicated test name => find TestInfo only on Type and TestNumber
    // Autoincrement test number => find TestInfo only on Type and TestName
    qint64 TestNumber = clRecord.m_u4TEST_NUM;
    QString TestName = clRecord.m_cnTEST_TXT;
    // Autoincrement test number => may be only the first record have the test name stored
    // In this case, we cannot use the test number to find the TestInfo
    // we must retrieve the original test name used for this test number
    // test number = 0 and test name empry is not allowed
    if(mMergeByTestName && TestName.isEmpty() && mAssociatedTestNames.contains(QString("[%1][%2]").arg(GQTL_STDF::Stdf_Record::Rec_FTR).arg(clRecord.m_u4TEST_NUM)))
        TestName = mAssociatedTestNames[QString("[%1][%2]").arg(GQTL_STDF::Stdf_Record::Rec_FTR).arg(clRecord.m_u4TEST_NUM)];

    if (mMergeByTestName)
        TestNumber = GetAutoIncrementTestNumber(GQTL_STDF::Stdf_Record::Rec_FTR, TestName);
    else if (mMergeByTestNumber)
        TestName = "";
    else if (! mAllowedDuplicateTestNumbers)
        TestName = "";

    if ((TestNumber == -1) && TestName.isEmpty())
    {
        GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidField, NULL,
                    QString("%1 option cannot be used with empty TestName. PTR[%2]")
                    .arg(mAutoincrementTestNumbers ? "Test[number]=Auto" : "TestMergeBy=NAME")
                    .arg(clRecord.m_u4TEST_NUM).toLatin1().constData());
        return false;
    }

    pTest = FindTestInfo(GQTL_STDF::Stdf_Record::Rec_FTR, TestNumber, TestName);

    // Check if test was executed
    // Special case ...
    // Datalog test result for TestTime : it is not a "TEST" so BIT4=1 (test not executed) but has a valid result
    bool bValidRun = (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] > 0);  // && ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT4) == 0);

    // Check if has a valid result outside a PIR PRR
    if ((m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTRESULTOUTSIDE) &&
            (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] == 0) &&                       // Outside PIR/PRR block
            ! IsTestResultInvalid(clRecord))                                                                      // Test result is valid: test executed and valid
    {
        QString strMessage = "[Record#FTR] ";
        strMessage += "Found record outside of PIR/PRR bloc. Some results were ignored for test number ";
        strMessage += QString::number(clRecord.m_u4TEST_NUM) + " (" + clRecord.m_cnTEST_TXT + ").";
        WarningMessage(strMessage);
    }

    bool bValidResult = ! (clRecord.m_b1TEST_FLG & STDF_MASK_BIT4);
    // GCORE-2044 : Not good. Must now reject. To be reported in the 3 UpdateTestInfo(...).
    if (bValidResult
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] > 0)
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] == m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
            )
    {
        QString strMessage = "[Record#FTR] ";
        strMessage += "Found illegal record outside of PIR/PRR bloc.";
        strMessage += QString::number(clRecord.m_u4TEST_NUM) + " (" + clRecord.m_cnTEST_TXT + ").";
        GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data());
        GSET_ERROR1(GexDbPlugin_Base, eValidation_InconsistentTestResult, NULL, strMessage.toLatin1().constData());
        return false;
    }


    // Test result valid
    int nFail = 0;

    // Check if Pass/Fail flag valid
    if (bValidRun && IsTestFail(clRecord, pTest))
        nFail = 1;

    unsigned short siteNumber =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            (m_clStdfParse, clRecord);

    if (pTest == NULL)
    {
        ++mTotalNewTests[siteNumber];
        if (mTestNumbers.contains(TestNumber) || mTestNames.contains(NormalizeTestName(clRecord.m_cnTEST_TXT)))
        {
            ++mTotalInconsistentTests[siteNumber];
            if (mTestNames.contains(NormalizeTestName(clRecord.m_cnTEST_TXT)))
                mLastInconsistentTestNumber[siteNumber] = QString("%2 - %3 - %4")
                        .arg("FTR").arg(TestNumber).arg(NormalizeTestName(clRecord.m_cnTEST_TXT));
            else
                mLastInconsistentTestName[siteNumber] = QString("%2 - %3 - %4")
                        .arg("FTR").arg(TestNumber).arg(NormalizeTestName(clRecord.m_cnTEST_TXT));
        }
        // new entry
        pTest = CreateTestInfo(GQTL_STDF::Stdf_Record::Rec_FTR, TestNumber, clRecord.m_u4TEST_NUM,
                               NormalizeTestName(clRecord.m_cnTEST_TXT),
                               clRecord.m_cnTEST_TXT);
        if (pTest == NULL)
            return false;
    }
    else if (pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_FTR)
    {
        // verify if test is really a FTR
        // error
        GSET_ERROR2(GexDbPlugin_Base, eValidation_DuplicateTest, NULL, pTest->m_nOriginalTestNum, pTest->m_strTestName.toLatin1().constData());
        if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTTYPEREDEFINITION)
            return false;
        QString strMessage = "[Record#FTR] ";
        strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        WarningMessage(strMessage);
        // Ignore this test
        return true;
    }
    else if (! mAllowedDuplicateTestNumbers && ! clRecord.m_cnTEST_TXT.isEmpty() && ! mMergeByTestNumber &&
             (NormalizeTestName(pTest->m_strOriginalTestName) != NormalizeTestName(clRecord.m_cnTEST_TXT)))
    {
        GSET_ERROR3(GexDbPlugin_Base, eValidation_DuplicateTestNumber, NULL,
                    pTest->m_nOriginalTestNum,
                    clRecord.m_cnTEST_TXT.toLatin1().constData(),
                    "FTR");
        return false;
    }

    ++mTotalTestsExecuted[siteNumber];

    if (bValidRun)
    {
        if (pTest->m_nTestSeq == -1)
        {
            m_nTestsSequence++;
            pTest->m_nTestSeq = m_nTestsSequence;
        }
        pTest->m_mapExecCount[MERGE_SITE]++;
        pTest->m_mapFailCount[MERGE_SITE] += nFail;
        if (! pTest->m_mapExecCount.contains(siteNumber))
        {
            pTest->m_mapExecCount[siteNumber] = 0;
            pTest->m_mapFailCount[siteNumber] = 0;
        }
        pTest->m_mapExecCount[siteNumber]++;
        pTest->m_mapFailCount[siteNumber] += nFail;
    }

    pTest = NULL;
    return true;
}

//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateTestInfo(const GQTL_STDF::Stdf_MPR_V4& clRecord)
{
    // first find if Test already exist
    // else create new Test Info
    structTestInfo* pTest = NULL;

    // Check if test was executed
    // Special case ...
    // Datalog test result for TestTime : it is not a "TEST" so BIT4=1 (test not executed) but has a valid result
    bool bValidRun = (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] > 0);  // && ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT4) == 0);

    // Check if has a valid result outside a PIR/PRR
    if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTRESULTOUTSIDE) &&
            (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] == 0) &&             // Outside PIR/PRR block
            ! IsTestResultInvalid(clRecord))                                           // Test result is valid: test executed and valid
    {
        QString strMessage = "[Record#MPR] ";
        strMessage += "Found record outside of PIR/PRR bloc. Some results were ignored for test number ";
        strMessage += QString::number(clRecord.m_u4TEST_NUM) + " (" + clRecord.m_cnTEST_TXT + ").";
        WarningMessage(strMessage);
    }

    // GCORE-2044 : Not good. Must now reject. To be reported in the 3 UpdateTestInfo(...).
    bool bValidResult = ! (clRecord.m_b1TEST_FLG & STDF_MASK_BIT4);
    if (bValidResult
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] > 0)
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR] == m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
            )
    {
        QString strMessage = "[Record#MPR] ";
        strMessage += "Found illegal record outside of PIR/PRR bloc.";
        strMessage += QString::number(clRecord.m_u4TEST_NUM) + " (" + clRecord.m_cnTEST_TXT + ").";
        GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data());
        GSET_ERROR1(GexDbPlugin_Base, eValidation_InconsistentTestResult, NULL, strMessage.toLatin1().constData());
        return false;
    }


    int nTestPin, nTestPinIndex;
    int nMPTestCnt;  // The number of MPTest to save
    // m_u2RTN_ICNT=0        && m_u2RSLT_CNT=0 : TestPinArrayIndex=-1 && TestPinPMRIndex=-1 && NoResultFlag
    // m_u2RTN_ICNT=0        && m_u2RSLT_CNT>=1 : TestPinArrayIndex=i && TestPinPMRIndex=-1 or PMRIndex saved && Save all Results
    // m_u2RTN_ICNT>=1  && <m_u2RSLT_CNT : Warning and TestPinArrayIndex=i && TestPinPMRIndex=index[i] && Save m_u2RTN_ICNT Results
    // m_u2RTN_ICNT>=1  && >m_u2RSLT_CNT : Warning and TestPinArrayIndex=i && TestPinPMRIndex=index[i] && Save m_u2RSLT_CNT Results
    // m_u2RTN_ICNT>=1  && =m_u2RSLT_CNT  : OK
    // m_u2RTN_ICNT>=1  && m_u2RSLT_CNT=0 : Save m_u2RTN_ICNT Results && NoResultFlag
    if ((clRecord.m_u2RTN_ICNT > 1)
            && (clRecord.m_u2RTN_ICNT != clRecord.m_u2RSLT_CNT))
    {
        // Error reading STDF file
        // Convertion failed.
        QString strMessage = "[Record#MPR] ";
        strMessage += "RTN_ICNT <> RSLT_CNT for the Multi-Parametric test number ";
        strMessage += QString::number(clRecord.m_u4TEST_NUM) + " (" + clRecord.m_cnTEST_TXT + ").";
        WarningMessage(strMessage);
    }


    nMPTestCnt = qMin(clRecord.m_u2RTN_ICNT, clRecord.m_u2RSLT_CNT);
    nTestPinIndex = 0;
    if (clRecord.m_u2RTN_ICNT == 0)
    {
        if (clRecord.m_u2RSLT_CNT == 0)
        {
            // have to save this record without valid result
            nMPTestCnt = 1;
            nTestPinIndex = -1;
        }
        else
            nMPTestCnt = clRecord.m_u2RSLT_CNT;
    }  // 7699
    else if (clRecord.m_u2RSLT_CNT == 0)
        nMPTestCnt = clRecord.m_u2RTN_ICNT;


    int i;
    float fResult;
    // Test result valid
    int nFail = 0;

    // SAVED THE TPIN_INDEX 0 INSTEAD OF TPIN_NUMBER IN clRecord.m_ku2RTN_INDX[0]

    // case 6488 - 6489
    // Allow Duplicated test name => find TestInfo on Type and TestNumber and TestName
    // Rejectd Duplicated test name => find TestInfo only on Type and TestNumber
    // Autoincrement test number => find TestInfo only on Type and TestName
    qint64 TestNumber = clRecord.m_u4TEST_NUM;
    QString TestName = clRecord.m_cnTEST_TXT;
    // Autoincrement test number => may be only the first record have the test name stored
    // In this case, we cannot use the test number to find the TestInfo
    // we must retrieve the original test name used for this test number
    // test number = 0 and test name empry is not allowed
    if(mMergeByTestName && TestName.isEmpty() && mAssociatedTestNames.contains(QString("[%1][%2]").arg(GQTL_STDF::Stdf_Record::Rec_MPR).arg(clRecord.m_u4TEST_NUM)))
        TestName = mAssociatedTestNames[QString("[%1][%2]").arg(GQTL_STDF::Stdf_Record::Rec_MPR).arg(clRecord.m_u4TEST_NUM)];

    if (mMergeByTestName)
        TestNumber = GetAutoIncrementTestNumber(GQTL_STDF::Stdf_Record::Rec_MPR, TestName);
    else if (mMergeByTestNumber)
        TestName = "";
    else if (! mAllowedDuplicateTestNumbers)
        TestName = "";

    if ((TestNumber == -1) && TestName.isEmpty())
    {
        GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidField, NULL,
                    QString("%1 option cannot be used with empty TestName. MPR[%2]")
                    .arg(mAutoincrementTestNumbers ? "Test[number]=Auto" : "TestMergeBy=NAME")
                    .arg(clRecord.m_u4TEST_NUM).toLatin1().constData());
        return false;
    }

    // Check if the Parameter already saved
    pTest = FindTestInfo(GQTL_STDF::Stdf_Record::Rec_MPR, TestNumber, TestName, nTestPinIndex);

    unsigned short siteNumber =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            (m_clStdfParse, clRecord);

    // If pTest == NULL , it's the first time with have this test
    // Have to create and initialyze each tests
    if (pTest == NULL)
    {
        ++mTotalNewTests[siteNumber];
        if (mTestNumbers.contains(TestNumber) || mTestNames.contains(NormalizeTestName(clRecord.m_cnTEST_TXT)))
        {
            ++mTotalInconsistentTests[siteNumber];
            if (mTestNames.contains(NormalizeTestName(clRecord.m_cnTEST_TXT)))
                mLastInconsistentTestNumber[siteNumber] = QString("%2 - %3 - %4")
                        .arg("MPR").arg(TestNumber).arg(NormalizeTestName(clRecord.m_cnTEST_TXT));
            else
                mLastInconsistentTestName[siteNumber] = QString("%2 - %3 - %4")
                        .arg("MPR").arg(TestNumber).arg(NormalizeTestName(clRecord.m_cnTEST_TXT));
        }
        for (i = 0; i < nMPTestCnt; i++)
        {
            if (nTestPinIndex == -1)
                nTestPin = -1;
            else
            {
                nTestPinIndex = i;
                if (clRecord.m_ku2RTN_INDX)
                    nTestPin = clRecord.m_ku2RTN_INDX[i];
                else
                    nTestPin = -1;
            }

            // new entry
            pTest = CreateTestInfo(GQTL_STDF::Stdf_Record::Rec_MPR, TestNumber, clRecord.m_u4TEST_NUM,
                                   NormalizeTestName(clRecord.m_cnTEST_TXT),
                                   clRecord.m_cnTEST_TXT,
                                   nTestPinIndex, nTestPin);
            if (pTest == NULL)
                return false;
        }
    }
    else if (! mAllowedDuplicateTestNumbers && ! clRecord.m_cnTEST_TXT.isEmpty() && ! mMergeByTestNumber &&
             (NormalizeTestName(pTest->m_strOriginalTestName) != NormalizeTestName(clRecord.m_cnTEST_TXT)))
    {
        GSET_ERROR3(GexDbPlugin_Base, eValidation_DuplicateTestNumber, NULL,
                    pTest->m_nOriginalTestNum,
                    clRecord.m_cnTEST_TXT.toLatin1().constData(),
                    "MPR");
        return false;
    }

    ++mTotalTestsExecuted[siteNumber];

    // Check if Pass/Fail flag valid
    if (bValidRun && IsTestFail(clRecord, pTest))
        nFail = 1;

    // Get the last index for insertion
    //int lLastIndex = -1;

    for (i = 0; i < nMPTestCnt; i++)
    {

        if (clRecord.m_u2RSLT_CNT > 0)
            fResult = clRecord.m_kr4RTN_RSLT[i];
        else
            fResult = 0;  // No valid result

        if (nTestPinIndex == -1)
            nTestPin = -1;
        else
        {
            nTestPinIndex = i;
            if (clRecord.m_ku2RTN_INDX)
                nTestPin = clRecord.m_ku2RTN_INDX[i];
            else
                nTestPin = -1;
        }
        pTest = FindTestInfo(GQTL_STDF::Stdf_Record::Rec_MPR, TestNumber, TestName, nTestPinIndex);
        // case 7769
        // Allow variable TestPin definition
        // Test already exists (on PinIndex = 0)
        // But PinIndex can variate
        if (pTest == NULL)
        {
            QString lError = QString("Multi-Parametric redefinition is not allowed. Test %1 (%2) now has %3 pins definitions against %4 before")
                    .arg(clRecord.m_u4TEST_NUM)
                    .arg(m_clStdfMPR.m_cnTEST_TXT)
                    .arg(nMPTestCnt)
                    .arg(i);
            GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported, NULL, lError.toLatin1().data());
            *m_pbDelayInsertion = true;
            return false;
        }
        else
        {
            //lLastIndex = m_lstTestInfo_Index;
            // Check if the nTestPin has a reference
            if ((pTest->m_nTestPin < 0) && (nTestPin > 0))
                pTest->m_nTestPin = nTestPin;
        }
        if (pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_MPR)
        {
            // verify if test is really a MPR
            // error
            GSET_ERROR2(GexDbPlugin_Base, eValidation_DuplicateTest, NULL, pTest->m_nOriginalTestNum, pTest->m_strTestName.toLatin1().constData());
            if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTTYPEREDEFINITION)
                return false;
            QString strMessage = "[Record#MPR] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(strMessage);
            // Ignore this test
            return true;
        }

        if ((clRecord.m_b1PARM_FLG & STDF_MASK_BIT6) == 0)
            pTest->m_cFlags |= FLAG_TESTINFO_LL_STRICT;
        if ((clRecord.m_b1PARM_FLG & STDF_MASK_BIT7) == 0)
            pTest->m_cFlags |= FLAG_TESTINFO_HL_STRICT;

        if (bValidRun)
        {
            if (pTest->m_nTestSeq == -1)
            {
                m_nTestsSequence++;
                pTest->m_nTestSeq = m_nTestsSequence;
            }

            pTest->m_mapExecCount[MERGE_SITE]++;
            pTest->m_mapFailCount[MERGE_SITE] += nFail;
            if (clRecord.m_u2RSLT_CNT > 0)
            {
                pTest->m_mapSum[MERGE_SITE] += fResult;
                pTest->m_mapSquareSum[MERGE_SITE] += fResult * fResult;
                if (! pTest->m_mapMinValue.contains(MERGE_SITE) || (pTest->m_mapMinValue[MERGE_SITE] > fResult))
                    pTest->m_mapMinValue[MERGE_SITE] = fResult;
                if (! pTest->m_mapMaxValue.contains(MERGE_SITE) || (pTest->m_mapMaxValue[MERGE_SITE] < fResult))
                    pTest->m_mapMaxValue[MERGE_SITE] = fResult;
            }
        }

        // case 7498
        // MPR.OPT_FLAG bit 1 set =	START_IN and INCR_IN are invalid.
        // if(clRecord.m_b1OPT_FLAG & STDF_MASK_BIT1)        // valid flag
        if (clRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposOPT_FLAG))
        {
            if ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT0) == 0)
            {
                pTest->m_bHaveScalRes = true;
                pTest->m_nScalRes = clRecord.m_i1RES_SCAL;
            }

            bool bSaveLL, bSaveHL;
            bSaveLL = bSaveHL = false;
            // Verify if have new limits
            bSaveLL = (((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT4) == 0)
                       && ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT6) == 0));
            // FOR MULTIPLE LIMITS REDEFINITION
            if (bSaveLL)
            {
                pTest->m_bHaveScalLL = true;
                pTest->m_nScalLL = clRecord.m_i1LLM_SCAL;

                // Have a valid limit
                if (pTest->m_mapfLL.contains(MERGE_SITE))
                {
                    // SC 2008 09 17
                    // Save the min low limit from all sites
                    if (pTest->m_mapfLL[MERGE_SITE] <= clRecord.m_r4LO_LIMIT)
                        bSaveLL = false;

                }
            }
            bSaveHL = (((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT5) == 0)
                       && ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT7) == 0));
            // FOR MULTIPLE LIMITS REDEFINITION
            if (bSaveHL)
            {
                pTest->m_bHaveScalHL = true;
                pTest->m_nScalHL = clRecord.m_i1HLM_SCAL;

                // Have a valid limit
                if (pTest->m_mapfHL.contains(MERGE_SITE))
                {
                    // SC 2008 09 17
                    // Save the max high limit from all sites
                    if (pTest->m_mapfHL[MERGE_SITE] >= clRecord.m_r4HI_LIMIT)
                        bSaveHL = false;
                }
            }

            // then update
            if (bSaveLL)
                pTest->m_mapfLL[MERGE_SITE] = clRecord.m_r4LO_LIMIT;
            if (bSaveHL)
                pTest->m_mapfHL[MERGE_SITE] = clRecord.m_r4HI_LIMIT;
            if (pTest->m_strUnits.isEmpty())
                pTest->m_strUnits = clRecord.m_cnUNITS;

            // Update Strict flag
            // Only for the first saved
            if (bSaveLL && bSaveHL)
            {
                // If LowLimit == HighLimit then no strict limits
                if ((pTest->m_mapfLL.contains(MERGE_SITE))
                        && (pTest->m_mapfHL.contains(MERGE_SITE))
                        && (pTest->m_mapfLL[MERGE_SITE] == pTest->m_mapfHL[MERGE_SITE]))
                {
                    pTest->m_cFlags &= ~FLAG_TESTINFO_LL_STRICT;
                    pTest->m_cFlags &= ~FLAG_TESTINFO_HL_STRICT;
                }
                else
                {
                    if ((clRecord.m_b1PARM_FLG & STDF_MASK_BIT6) == 0)
                        pTest->m_cFlags |= FLAG_TESTINFO_LL_STRICT;
                    if ((clRecord.m_b1PARM_FLG & STDF_MASK_BIT7) == 0)
                        pTest->m_cFlags |= FLAG_TESTINFO_HL_STRICT;
                }
            }

            if (! pTest->m_bHaveSpecLL)
            {
                pTest->m_bHaveSpecLL = ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT2) == 0);
                if (pTest->m_bHaveSpecLL)
                    pTest->m_fSpecLL = clRecord.m_r4LO_SPEC;
            }
            if (! pTest->m_bHaveSpecHL)
            {
                pTest->m_bHaveSpecHL = ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT3) == 0);
                if (pTest->m_bHaveSpecHL)
                    pTest->m_fSpecHL = clRecord.m_r4HI_SPEC;
            }
        }

        if (bValidRun)
        {
            if (! pTest->m_mapExecCount.contains(siteNumber))
            {
                pTest->m_mapExecCount[siteNumber] = 0;
                pTest->m_mapFailCount[siteNumber] = 0;
                if (clRecord.m_u2RSLT_CNT > 0)
                {
                    pTest->m_mapMinValue[siteNumber] = fResult;
                    pTest->m_mapMaxValue[siteNumber] = fResult;
                    pTest->m_mapSum[siteNumber] = 0;
                    pTest->m_mapSquareSum[siteNumber] = 0;
                }

            }

            pTest->m_mapExecCount[siteNumber]++;
            pTest->m_mapFailCount[siteNumber] += nFail;
            if (clRecord.m_u2RSLT_CNT > 0)
            {
                pTest->m_mapSum[siteNumber] += fResult;
                pTest->m_mapSquareSum[siteNumber] += fResult * fResult;
                if (pTest->m_mapMinValue[siteNumber] > fResult)
                    pTest->m_mapMinValue[siteNumber] = fResult;
                if (pTest->m_mapMaxValue[siteNumber] < fResult)
                    pTest->m_mapMaxValue[siteNumber] = fResult;
            }
        }

        // if new limit definition for this site then error
        // case 7498
        // MPR.OPT_FLAG bit 1 set =	START_IN and INCR_IN are invalid.
        //if(clRecord.m_b1OPT_FLAG & STDF_MASK_BIT1)        // valid flag
        if (clRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposOPT_FLAG))
        {
            if (((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT4) == 0)
                    && ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT6) == 0))
            {
                if (pTest->m_mapfLL.contains(siteNumber))
                {
                    if((pTest->m_mapfLL[siteNumber] != (float)clRecord.m_r4LO_LIMIT) && (pTest->m_mapExecCount[MERGE_SITE] > 1))
                    {
                        // limit redifinition after first test execution
                        if(m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTLIMITREDEFINITION)
                        {
                            GSET_ERROR2(GexDbPlugin_Base, eValidation_MultipleLimits, NULL, clRecord.m_u4TEST_NUM, pTest->m_strTestName.toLatin1().constData());
                            if(m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTLIMITREDEFINITION)
                                return false;
                            QString strMessage = "[Record#MPR] ";
                            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                            WarningMessage(strMessage);
                        }

                        // Save the min low limit for this site
                        if (pTest->m_mapfLL[siteNumber] > (float) clRecord.m_r4LO_LIMIT)
                            pTest->m_mapfLL[siteNumber] = clRecord.m_r4LO_LIMIT;
                    }
                }
                else
                    pTest->m_mapfLL[siteNumber] = clRecord.m_r4LO_LIMIT;
            }
            if (((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT5) == 0)
                    && ((clRecord.m_b1OPT_FLAG & STDF_MASK_BIT7) == 0))
            {
                if (pTest->m_mapfHL.contains(siteNumber))
                {
                    if((pTest->m_mapfHL[siteNumber] != (float)clRecord.m_r4HI_LIMIT) && (pTest->m_mapExecCount[MERGE_SITE] > 1))
                    {
                        // limit redifinition after first test execution
                        if(m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTLIMITREDEFINITION)
                        {
                            GSET_ERROR2(GexDbPlugin_Base, eValidation_MultipleLimits, NULL, clRecord.m_u4TEST_NUM, pTest->m_strTestName.toLatin1().constData());
                            if(m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTLIMITREDEFINITION)
                                return false;
                            QString strMessage = "[Record#MPR] ";
                            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                            WarningMessage(strMessage);
                        }

                        // Save the max high limit for this site
                        if (pTest->m_mapfHL[siteNumber] < (float) clRecord.m_r4HI_LIMIT)
                            pTest->m_mapfHL[siteNumber] = clRecord.m_r4HI_LIMIT;
                    }
                }
                else
                    pTest->m_mapfHL[siteNumber] = clRecord.m_r4HI_LIMIT;
            }
        }
    }

    pTest = NULL;
    return true;
}

//////////////////////////////////////////////////////////////////////
/// \brief CreateTestInfo
/// Create the Test Info struct into the m_lstTestInfo
/// \return
/////////////////////////////////////////////////////////////////////////
GexDbPlugin_Galaxy::structTestInfo* GexDbPlugin_Galaxy::CreateTestInfo(int nTestType, qint64 nTestNumber, qint64 nOriginalTestNumber,
                                                                       QString lTestName, QString lOriginalTestName,
                                                                       int nTestPinIndex, int nTestPin
                                                                       )
{
    structTestInfo* pTest = NULL;

    try
    {
        pTest = new structTestInfo;
    }
    catch(const std::bad_alloc& e)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
        GSET_ERROR0(GexDbPlugin_Base, eMemoryAllocation, NULL);
        return NULL;
    }
    catch (...)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
        GSET_ERROR0(GexDbPlugin_Base, eMemoryAllocation, NULL);
        return NULL;
    }

    m_lstTestInfo.append(pTest);
    pTest->m_nTestInfoIndex = m_lstTestInfo.count() - 1;
    pTest->m_nOriginalTestNum = nOriginalTestNumber;
    pTest->m_strOriginalTestName = lOriginalTestName;

    pTest->m_nInfoId = 0;

    pTest->m_nTestType = nTestType;
    pTest->m_nTestNum = nTestNumber;
    pTest->m_nTestSeq = -1;
    pTest->m_nTestPinIndex = nTestPinIndex;
    pTest->m_nTestPin = nTestPin;
    pTest->m_strTestName = lTestName;
    pTest->m_cFlags = 0;
    pTest->m_mapExecCount[MERGE_SITE] = 0;
    pTest->m_mapFailCount[MERGE_SITE] = 0;
    pTest->m_mapSum[MERGE_SITE] = 0;
    pTest->m_mapSquareSum[MERGE_SITE] = 0;

    pTest->m_bHaveSpecHL = false;
    pTest->m_bHaveSpecLL = false;
    pTest->m_bHaveSpecTarget = false;

    pTest->m_bHaveScalRes = false;
    pTest->m_bHaveScalLL = false;
    pTest->m_bHaveScalHL = false;

    //    QString msg = "New test: " + QString::number(pTest->m_nTestNum) + "(" + QString::number(pTest->m_nOriginalTestNum) + ")" + "-" + pTest->m_strTestName;
    //    GSLOG(SYSLOG_SEV_CRITICAL, msg.toLatin1().data());
    mTestNumbers.insert(nTestNumber, pTest);
    mTestNames.insert(pTest->m_strTestName, pTest);
    if (mMergeByTestName)
        mAssociatedTestNames[QString("[%1][%2]").arg(nTestType).arg(nOriginalTestNumber)] = lOriginalTestName;

    // in case of Charac Database AND test conditions from DTR we store them by test
    if (IsCharacTdr() && mpDbKeysEngine->GetTestConditionsOrigin() == GS::QtLib::DatakeysEngine::DTR)
        pTest->mTestConditions = mpDbKeysEngine->dbKeysContent().testConditions();

    return pTest;
}

bool GexDbPlugin_Galaxy::MatchingTestInfo(structTestInfo *pTest, int nTestType, qint64 nTestNum, QString lTestName, int nTestPinIndex)
{
    if (pTest == NULL)
        return false;

    // Check test type redefinition
    // If a TestNum/TestName was defined as FTR the first time
    // and if the second time, it is a PTR for the same TestNum/TestName
    // If the option AllowTestRedefintion is not TRUE
    // return the FTR => the check will be done after
    // If the option AllowTestRedefintion is TRUE
    // check the Type
    // First compare same TestType if no TESTTYPE option
    if ((m_uiInsertionValidationOptionFlag & ~GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTTYPEREDEFINITION)
            && ((nTestType != -1) && (pTest->m_nTestType != nTestType)))
        return false;

    // The TestNum can be = to -1 if MergerByTestName
    // In this case, ALL m_nTestNum are = to -1
    if (pTest->m_nTestNum == nTestNum)
    {
        // Same TestNum
        // Check if the TestName is the same
        // The TestName can be empty
        // - to reduce the size of the STDF file
        // - or for MergeByTestNumber or AutoTestNumber
        if (lTestName.isEmpty() || (pTest->m_strTestName.toUpper() == lTestName.toUpper()))
        {
            // TestNum and TestName are the same
            // if both MPR test type, check also the PinNumber
            if((nTestType != GQTL_STDF::Stdf_Record::Rec_MPR) || (pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_MPR))
                return true;

            // Both tests are MPR
            // Check the PinIndex
            if ((nTestPinIndex == -1) || (pTest->m_nTestPinIndex == nTestPinIndex))
                return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////
// Following algorithm is used to find the specified test in the
// testlist, looping through the list:
//
// ??Same test number?? --N--> CONTINUE
//          |
//          Y
//          |
//          V
// ??Same test name?? --Y--> ??(2 MPR)?? --Y--> ??Same Pin Nb?? --Y--> FOUND
//          |                     |                              |
//          |                     N                              N
//          |                     |                              |
//          N                     V                              V
//          |                   FOUND                        CONTINUE
//          |
//          V
// ??(Same type) && (Type != -1)?? --Y--> ??Test name empty?? --Y--> FOUND
//                |                                 |
//                N                                 N
//                |                                 |
//                V                                 V
//            CONTINUE                          CONTINUE
//
//////////////////////////////////////////////////////////////////////
GexDbPlugin_Galaxy::structTestInfo* GexDbPlugin_Galaxy::FindTestInfo(int nTestType, qint64 nTestNumber, QString lTestName, int nTestPinIndex/*=-1*/)
{

    TimersTraceStart("Function FindTestInfo");

    structTestInfo* pTest = NULL;

    // Normalize test name
    QString strNormalizedTestName;
    // Check if have sequencer name
    // Case
    // TSR => TestName [PinName] (Flex)
    // PTR => TestName [PinName] [<> SequencerName]
    // When FindTestInfo from TSR, always check without SequencerName and PinName
    // When FindTestInfo from other, use the user option
    // Original PTR => TestName PinName <> SequencerName => TestName
    // TSR => TestName
    // AutoIncrement + KeepSequencerOption (IGNORE TSR)
    // PTR1 => TestAA PinAA <> SequencerAA
    // PTR2 => TestAA => noSequencer => PTR1 TestAA, PTR2 TestAA => 1 entry
    // PTR1 => TestAA
    // PTR2 => TestAA <> SequencerAA => Sequencer => 2 entries

    // If MergeByTestNum => TestName = ''
    // If unique  => TestName = ''
    // If not Unique Test number, check each elt
    // If MergeByTestName or AutoTestNumber => TestNumber = -1

    // If the flow is stable after the first run
    // Just check if the current or the next pos is the good one into the QList<structTestInfo*>
    // If the position doesn't match
    // Check if the mTestNumbers contains the num (if valid TestNum)
    // Check if the mTestName contains the name

    // Inititalize test ptr
    // Reset the index of necessary
    if ((m_lstTestInfo_Index < 0) || (m_lstTestInfo_Index >= m_lstTestInfo.size()))
        m_lstTestInfo_Index = 0;

    strNormalizedTestName = NormalizeTestName(lTestName);

    bool lCheckTestCond = IsCharacTdr() && (mpDbKeysEngine->GetTestConditionsOrigin() == GS::QtLib::DatakeysEngine::DTR);
    QMap<QString, QString> lTestCondContext;
    if (lCheckTestCond)
        lTestCondContext = mpDbKeysEngine->dbKeysContent().testConditions();

    while ((m_lstTestInfo_Index > -1) && (m_lstTestInfo_Index < m_lstTestInfo.size()))
    {

        pTest = m_lstTestInfo.at(m_lstTestInfo_Index);

        if (MatchingTestInfo(pTest, nTestType, nTestNumber, strNormalizedTestName, nTestPinIndex))
        {
            // if Test found AND test has to be done AND cond doesn't match --> test not found
            if (lCheckTestCond && (lTestCondContext != pTest->mTestConditions))
            {
                // not the good one let s continue
            }
            else
            {
                TimersTraceStop("Function FindTestInfo");
                return pTest;
            }
        }
        ++m_lstTestInfo_Index;
    }

    // The flow is not stable
    // It is not possible to have the TestNum AND the TestName empty at the same time
    // Retrieve the Test and the position
    QList<structTestInfo*> lTestLists;
    if (nTestNumber == -1)
        lTestLists = mTestNames.values(strNormalizedTestName);
    else
        lTestLists = mTestNumbers.values(nTestNumber);

    // Check if found something
    // Values return the list of Tests that match the request
    if (! lTestLists.isEmpty())
    {
        QList<structTestInfo*>::Iterator itTest;
        for (itTest = lTestLists.begin(); itTest != lTestLists.end(); ++itTest)
        {
            // more that one elt
            pTest = *itTest;
            if (MatchingTestInfo(pTest, nTestType, nTestNumber, strNormalizedTestName, nTestPinIndex))
            {
                // Found the test
                // re-initialize the current index position
                if (lCheckTestCond && (lTestCondContext != pTest->mTestConditions))
                {
                    // not the good one let s continue
                    continue;
                }
                else
                {
                    m_lstTestInfo_Index = pTest->m_nTestInfoIndex;
                    TimersTraceStop("Function FindTestInfo");
                    return pTest;
                }
            }
        }
    }

    TimersTraceStop("Function FindTestInfo");
    return NULL;

    /*
    bool AllowUserOptions = true;
    strNormalizedTestName = NormalizeTestName(strTestName,AllowUserOptions);

    //if(!strTestName.isEmpty() && !mTestNames.contains(QString::number(nTestNum)+strNormalizedTestName))
    if(!strTestName.isEmpty() && !mTestNames.contains(strNormalizedTestName))
    {
        TimersTraceStop("Function FindTestInfo");
        return NULL;
    }

    strNormalizedTestName = strNormalizedTestName.toUpper();

    // Inititalize test ptr
    if(bStartToFirst || (m_lstTestInfo_Index < 0) || (m_lstTestInfo_Index >= m_lstTestInfo.size()))
        m_lstTestInfo_Index = 0;

    while((m_lstTestInfo_Index > -1) && (m_lstTestInfo_Index < m_lstTestInfo.size()))
    {
        pTest = m_lstTestInfo.at(m_lstTestInfo_Index);

        // Do not check test type redefinition
        // First compare same TestType if no TESTTYPE option
        if((m_uiInsertionValidationOptionFlag & ~GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTTYPEREDEFINITION)
                && ((nTestType != -1) && (pTest->m_nTestType != nTestType)))
        {
            m_lstTestInfo_Index++;
            continue;
        }

        if(pTest->m_nTestNum == nTestNum)
        {
            // Same TestNum
            if(NormalizeTestName(pTest->m_strOriginalTestName,AllowUserOptions).toUpper() == strNormalizedTestName)
            {
                // TestNum and TestName are the same: if both MPR test type, check PinNumber
                if((nTestType != GQTL_STDF::Stdf_Record::Rec_MPR) || (pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_MPR))
                {
                    TimersTraceStop("Function FindTestInfo");
                    return pTest;
                }

                // Both tests are MPR
                if((nTestPinIndex == -1) || (pTest->m_nTestPinIndex == nTestPinIndex))
                {
                    TimersTraceStop("Function FindTestInfo");
                    return pTest;
                }
            }
            else
            {
                // Same TestNum, but different TestName
                if((nTestType != -1) && (pTest->m_nTestType == nTestType) && (strNormalizedTestName.isEmpty()))
                {
                    // TestNum are the same: if both MPR test type, check PinNumber
                    if((nTestType != GQTL_STDF::Stdf_Record::Rec_MPR) || (pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_MPR))
                    {
                        TimersTraceStop("Function FindTestInfo");
                        return pTest;
                    }

                    // Both tests are MPR
                    if((nTestPinIndex == -1) || (pTest->m_nTestPinIndex == nTestPinIndex))
                    {
                        TimersTraceStop("Function FindTestInfo");
                        return pTest;
                    }
                }

                // Do not run through all the tests list if we have only one TextNumber encountered
                if(mTestNumbers.count(nTestNum) == 1)
                {
                    TimersTraceStop("Function FindTestInfo");
                    return NULL;
                }
            }
        }
        m_lstTestInfo_Index++;
    }

    // Restart at beginning of the list?
    // Restart at the begining of the list only if current Run is not the first
    if(!bStartToFirst)
    {
        TimersTraceStop("Function FindTestInfo");
        return FindTestInfo(nTestType, nTestNum, strTestName, nTestPinIndex, true);
    }

    TimersTraceStop("Function FindTestInfo");
    return NULL;
 */
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::NormalizeTestName(const QString& lTestName)
{
    if (lTestName.isEmpty())
        return lTestName;

    QString lPinName;

    QString lName = lTestName.trimmed();
    QString lSequencerName;

    // Need to separate Test and Sequencer to work on PinName
    if (lName.contains("<>"))
    {
        lSequencerName = lName.section("<>", 1).trimmed();
        lName = lName.section("<>", 0, 0).trimmed();
    }

    // Check if this is a Flex/J750 data file (if so, we MAY need to remove leading pin# in test names!)
    // If ends with pin#, remove it eg: xxxxx 11.a30
    bool bValidPin = false;
    if (m_bRemovePinName && lName.contains(" "))
    {
        lPinName = lName.section(' ', -1, -1);
        lPinName.right(1).toInt(&bValidPin);
    }
    if (! lPinName.isEmpty() && bValidPin)
    {
        // 11.a30
        // 11.ab30
        // -1
        // +1
        // 1
        // RegExp :digit:dot:alpha:digit: => \d+\.[^\d]+\d+
        QString lPinPattern = "(\\d+\\.[^\\d]+\\d+|(\\+|-)?\\d+)";
        QRegExp::PatternSyntax lPatternSyntax = QRegExp::RegExp;
        QRegExp  lRegExpPattern("", Qt::CaseInsensitive, lPatternSyntax);
        lRegExpPattern.setPattern(lPinPattern);

        // Check match
        bool bMatch = lRegExpPattern.exactMatch(lPinName);
        if(bMatch)
        {
            // Drop this pin number !
            lName.truncate(lName.length() - lPinName.length());
            lName = lName.trimmed();
        }
    }

    if (! m_bRemoveSequencerName && ! lSequencerName.isEmpty())
        lName = lName + " <> " + lSequencerName;

    return lName.trimmed();
}

//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::NormalizeNumber(const float& fValue, QString strDecSepValue  /*="."*/)
{
    QString strValue = QString::number(fValue, 'g', 24);
    if (strValue == "inf")
        strValue = "1e+38";
    else if (strValue == "-inf")
        strValue = "-1e+38";
    else if (strValue == "nan")
        strValue = "null";
    else if (strDecSepValue != ".")
        strValue.replace(".", strDecSepValue);

    return strValue;
}

//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::NormalizeTableName(const QString& strValue, bool bAddPrefixTable  /*=true*/)
{
    return NormalizeTableName(strValue, m_strPrefixTable, bAddPrefixTable);
}

QString        GexDbPlugin_Galaxy::NormalizeTableName(const QString &strValue, const QString &strPrefixTable, bool bAddPrefixTable/*=true*/)
{
    QString strString;

    if (bAddPrefixTable)
        strString = strPrefixTable;
    strString += strValue;
    return m_strPrefixDB + strString.toLower() + m_strSuffixDB;
}

QMap<QString, QString> GexDbPlugin_Galaxy::GetPartitionNameFromGranularity(QString& Granularity,
                                                                           UINT MinSequence,
                                                                           UINT MaxSequence,
                                                                           UINT CurrentSequence)
{
    QMap<QString, QString> PartitionInfo;
    QDate Date = QDate::currentDate();

    // Get the MinSequence from ActivePartition
    UINT lMinSequence = MinSequence;
    UINT lMaxSequence = 0;

    // if MaxSequence is defined
    // Check if current sequence is < MaxSequence
    // Then allow new convention option
    if (MaxSequence > 0)
    {
        if ((MaxSequence <=
             QString(Date.toString("yy") + QString::number(Date.dayOfYear()).rightJustified(3, '0'))
             .leftJustified(10, '0').toUInt()))
            return PartitionInfo;
    }

    if (Granularity.startsWith("M"))
    {
        // Only for MySql
        // Sequence start at the first day of the month
        Date = QDate(Date.year(), Date.month(), 1);

        PartitionInfo["Granularity"] = "MONTH";

        lMinSequence = QString(Date.toString("yy")
                               + QString::number(Date.dayOfYear()).rightJustified(3, '0'))
                .leftJustified(10, '0').toUInt();
        if ((MinSequence > 0)
                && (MinSequence > lMinSequence))
            lMinSequence = MinSequence;

        // Sequence end at the last day of the month
        Date = Date.addMonths(1);
        lMaxSequence = QString(Date.toString("yy")
                               + QString::number(Date.dayOfYear()).rightJustified(3, '0'))
                .leftJustified(10, '0').toUInt() - 1;

    }
    else if (Granularity.startsWith("W"))
    {
        // Sequence start at the first day of the week
        Date = Date.addDays(1 - Date.dayOfWeek());

        PartitionInfo["Granularity"] = "WEEK";

        lMinSequence = QString(Date.toString("yy")
                               + QString::number(Date.dayOfYear()).rightJustified(3, '0'))
                .leftJustified(10, '0').toUInt();

        if ((MinSequence > 0)
                && (MinSequence > lMinSequence))
            lMinSequence = MinSequence;

        Date = Date.addDays(7);
        lMaxSequence = QString(Date.toString("yy")
                               + QString::number(Date.dayOfYear()).rightJustified(3, '0'))
                .leftJustified(10, '0').toUInt() - 1;
    }
    else if (Granularity.startsWith("D"))
    {
        // Sequence start at the current day
        PartitionInfo["Granularity"] = "DAY";

        lMinSequence = QString(Date.toString("yy")
                               + QString::number(Date.dayOfYear()).rightJustified(3, '0'))
                .leftJustified(10, '0').toUInt();

        if ((MinSequence > 0)
                && (MinSequence > lMinSequence))
            lMinSequence = MinSequence;

        Date = Date.addDays(1);
        lMaxSequence = QString(Date.toString("yy")
                               + QString::number(Date.dayOfYear()).rightJustified(3, '0'))
                .leftJustified(10, '0').toUInt() - 1;
    }
    else if (Granularity.startsWith("S"))
    {
        // Sequence start at the current day + autoincrement
        PartitionInfo["Granularity"] = "SPLITLOT";

        lMinSequence = QString(Date.toString("yy")
                               + QString::number(Date.dayOfYear()).rightJustified(3, '0'))
                .leftJustified(10, '0').toUInt();

        if ((MinSequence > 0)
                && (MinSequence > lMinSequence))
            lMinSequence = MinSequence;

        lMaxSequence = lMinSequence + PartitionGranularityOption.section("|", 1).toUInt();
        // Check if need to create a new partition after N splitlots insertion
        if (CurrentSequence >= lMaxSequence)
        {
            lMinSequence = CurrentSequence;
            lMaxSequence = lMinSequence + PartitionGranularityOption.section("|", 1).toUInt();
        }
    }
    else
        return PartitionInfo;

    PartitionInfo["MinSequence"] = QString::number(lMinSequence);
    PartitionInfo["MaxSequence"] = QString::number(lMaxSequence);
    PartitionInfo["PartitionName"] = Granularity[0] + PartitionInfo["MinSequence"].leftJustified(10, '0');

    return PartitionInfo;
}

//////////////////////////////////////////////////////////////////////
// Partition Naming convention
// PYYMM000000 => OLD_MYSQL CONVENTION (by month)
//  YYMMDD0000 => OLD_ORACLE CONVENTION (by day or splitlot)
// SYYDDD00000 => SPLITLOT CONVENTION (by splitlot)
// DYYDDD00000 => DAY CONVENTION (by day)
// WYYDDD00000 => WEEK CONVENTION (by week)
// MYYDDD00000 => MONTH CONVENTION (by month)
//////////////////////////////////////////////////////////////////////
// If From is not empty, retrieve the Convention used and return the current PartitionName
// If From is empty, apply the Convention and return the current PartitionName
//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::GetPartitionName(int* LimitSequence)
{
    QString TableKey = m_strPrefixTable + "_PTEST_RESULTS|active_partition";
    UINT CurrentSequence = 0;
    UINT MinSequence = 0;
    UINT MaxSequence = 0;
    QString ActivePartitionName;
    QString NewPartitionName;
    QString PartitionGranularity;
    bool ApplyActiveConvention = false;
    QMap<QString, QString> PartitionNameConvention;

    PartitionGranularity = NewPartitionName = ActivePartitionName = PartitionGranularityOption;

    if (m_mapTablesDesc.contains(TableKey.toUpper()))
    {
        PartitionGranularity = NewPartitionName = ActivePartitionName = m_mapTablesDesc[TableKey.toUpper()].section("|",0,0);
        CurrentSequence = m_mapTablesDesc[NormalizeTableName("_SPLITLOT_SEQUENCE").toUpper()].toUInt();
        MaxSequence = m_mapTablesDesc[TableKey.toUpper()].section("|", 1).toUInt(&ApplyActiveConvention);
        // For new convention
        // partition starts with M, W, D, S
        // For old convention
        // MySql Pyymm
        // Oracle yymmdd0000
        if (ActivePartitionName.toUInt() > 0)
        {
            MinSequence = ActivePartitionName.leftJustified(10, '0').toUInt();
            PartitionGranularity = "OLD";
        }
        else
        {
            MinSequence = ActivePartitionName.mid(1).leftJustified(10, '0').toUInt();
            // Have an active partition with the new convention name
            // Check if have to change the partition according to the current Date and the ActivePartitionName
            PartitionNameConvention = GetPartitionNameFromGranularity(ActivePartitionName,
                                                                      MinSequence,
                                                                      MaxSequence,
                                                                      CurrentSequence);
            if (! PartitionNameConvention.isEmpty() &&
                    (PartitionNameConvention["PartitionName"] == ActivePartitionName))
            {
                // ActivePartition is the good one
                // Do not allow to change, wait the end partition validity
                if (LimitSequence)
                    (*LimitSequence) = PartitionNameConvention["MaxSequence"].toUInt();
                return NewPartitionName;
            }

            if (ActivePartitionName.startsWith("P"))
                PartitionGranularity = "OLD";
        }



        // When the Partition granularity changes between the active_partition and the PartitionGranularity option
        // a. if the MaxSequence value is a valid sequence then wait the next partition change (for MySql)
        // b. if the MaxSequence value is MAXVALUE then propose the new partition name (for Oracle)
        // case 6529 - for ALL wait the next partition change (for MySql and Oracle)
        // Force to apply active convention for ALL
        if (ApplyActiveConvention)
        {
            // Check if the new day is greater than the MaxSequence
            QDate Date = QDate::currentDate();
            ApplyActiveConvention =
                    (MaxSequence >
                     QString(Date.toString("yy") + QString::number(Date.dayOfYear()).rightJustified(3, '0'))
                     .leftJustified(10, '0').toUInt());
            if (! ApplyActiveConvention)
            {
                // TODO
                // W13021000000 => sequence 13021000005 and max sequence 130279999999
                // day 13028 (new week) + Partition MONTH
                // M13001000000 => current sequence 13021000005
                // => M13021000005 => splitlot still inserted into W13021000000
                // TODO
                // use max sequence for update current sequence
                // M13001000000 => current sequence 13021000005 BUT max sequence 130279999999
                // => M13028000000
                if (CurrentSequence < MaxSequence)
                    CurrentSequence = MaxSequence + 1;
                MinSequence = MaxSequence = 0;
            }
        }

        // if have an active partition that starts with MinSequence and CurrentSequence=MinSequence
        // this is not possible to split the active partition
        // we need to use the current Active Convention
        if (CurrentSequence == MinSequence)
            ApplyActiveConvention = true;
    }

    if (! ApplyActiveConvention)
    {
        // Use the User Partition Granularity
        // Reset the Min Sequence to compute the new Sequence
        MinSequence = MaxSequence = 0;
        PartitionGranularity = PartitionGranularityOption;
    }

    PartitionNameConvention = GetPartitionNameFromGranularity(PartitionGranularity,
                                                              MinSequence,
                                                              MaxSequence,
                                                              CurrentSequence);
    if (! PartitionNameConvention.isEmpty())
    {
        PartitionGranularity = PartitionNameConvention["Granularity"];
        MinSequence = PartitionNameConvention["MinSequence"].toUInt();
        MaxSequence = PartitionNameConvention["MaxSequence"].toUInt();
        NewPartitionName = PartitionNameConvention["PartitionName"];
    }

    // Apply the Naming Convention
    if (PartitionGranularity != "OLD")
    {
        NewPartitionName = PartitionGranularity[0] + QString::number(MinSequence).leftJustified(10, '0');
        if (ActivePartitionName != NewPartitionName)
        {

            // Have to create new partition
            //////////////////////////////
            // After a very BIG insertion (more than 99999 splitlots in one day)
            // the splitlot_sequence is over the standard day range
            // Ex:
            // Last Partition
            // DayBefore = 09109
            // HighValue for split = 0910900000
            //
            // Add 123456 splitlots inserted or tested
            // current Index = 0911023456
            //
            // Current Day = 09110
            // logic HighValue for split = 0911100000
            // HAVE TO ADJUST HighValue to MAX(currentIndex, HighValue)

            // Check if the current sequence is not out of range
            // Then start the partition to tis value
            if (CurrentSequence > MinSequence)
                NewPartitionName = PartitionGranularity[0] + QString::number(CurrentSequence).leftJustified(10, '0');
        }
    }

    if (LimitSequence)
        (*LimitSequence) = MaxSequence;
    return NewPartitionName;
}

//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::NormalizeFileName(const QString& strFile, const QString strExt)
{
    QString strFileName;
    QString strTimestamp =
            QString::number(m_nSplitLotId) + "_"
            + QString::number(m_nInsertionSID) + "_"
            + QDateTime::currentDateTime().toString("ddMMyyyy_hhmmsszzz");

    // Construct file name
    if (m_pclDatabaseConnector->IsMySqlDB()
            && ! mAttributes["SQLLOADER_DATAFILES_CREATE"].toString().isEmpty())
        strFileName = mAttributes["SQLLOADER_DATAFILES_CREATE"].toString();
    else
        strFileName = m_strLocalFolder + "/GalaxySemi/temp/";
    QDir d(strFileName);
    if (! d.exists(strFileName))
        d.mkpath(strFileName);

    strFileName += QString(strFile + "_" + strTimestamp).replace('.', '_').remove('"');

    return QDir::cleanPath(strFileName + strExt);
}

//////////////////////////////////////////////////////////////////////
// translate string value for SQL query (ex: ' is not supported in a field)
//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::TranslateStringToSqlVarChar(const QString& strValue, bool bAddQuote)
{
    return m_pclDatabaseConnector->TranslateStringToSqlVarChar(strValue, bAddQuote);
}

//////////////////////////////////////////////////////////////////////
// apply a sql date format to translate a time
//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::TranslateUnixTimeStampToSqlDateTime(const QString strUnixTimeStamp, enum SqlUnixDateFormat eFormat, enum SqlUnixDateFormat eConcatFormat)
{
    return m_pclDatabaseConnector->TranslateUnixTimeStampToSqlDateTime(strUnixTimeStamp, eFormat, eConcatFormat);
}


void GexDbPlugin_Galaxy::WarningMessage(QString strMessage)
{
    // Add warning to warning stack
    if (! m_strlWarnings.contains(strMessage))
    {
        GSLOG(SYSLOG_SEV_WARNING, strMessage.toLatin1().constData());
        m_strlWarnings.append(strMessage);
    }
}

void GexDbPlugin_Galaxy::ErrorMessage(QString strMessage)
{
    GSLOG(SYSLOG_SEV_ERROR, QString("%1").arg(strMessage).toLatin1().constData());
}

bool GexDbPlugin_Galaxy::ValidationFunction(bool bOnlyHeaderValidation)
{
    GSLOG(SYSLOG_SEV_DEBUG, (QString("Performing validation for pass %1... Only Header Validation=%2")
                             .arg(m_lPass)
                             .arg(bOnlyHeaderValidation ? "true" : "false")).toLatin1().constData());

    QString strMessage;

    // Is Testing Stage forced to WT or ET?
    // If so, make sure Wafer info is set even if no wafer records present
    if ((mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().toLower() == "e")
            || (mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().toLower() == "w"))
    {
        // WT and ET, Check if a WaferID has been set
        if (mpDbKeysEngine->dbKeysContent().Get("Wafer").toString().isEmpty())
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Forcing TS to %1 but no WaferID defined").arg(
                      mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().toLatin1().data() ).toLatin1().constData());
            GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL, "Testing stage",
                        mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().toLatin1().constData(),
                        "but no WaferID defined");
            return false;
        }

        //
        // Check if have no WIR
        if (m_lPass == 1)
        {
            // After the first PIR
            if (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] == 0)
            {
                m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] = 1;
                m_nCurrentWaferIndex = 1;  // To read the WRR if any

                // OVERWRITE WITH LATEST VALUES
                // Set Wafer_ID
                m_strWaferId = mpDbKeysEngine->dbKeysContent().Get("Wafer").toString();
                // Simulate a WIR
                m_clStdfWIR.SetWAFER_ID(m_strWaferId);
                m_clStdfWIR.SetSTART_T(mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong());
            }
        }

    }

    // At the begining of the Pass2 (reading the MIR)
    // Set the testing stage
    SetTestingStage();

    // Check some records
    if (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MIR] == 0)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Missing MIR");
        GSET_ERROR1(GexDbPlugin_Base, eValidation_MissingRecords, NULL, "MIR");
        return false;
    }

    // Do not call ValidationFunction for Multi wafers in one file
    // If the first wafer in the file was inserted, continue with all other wafers in the file
    // Check only for the first wafer if have more than one
    if (m_nWaferIndexToProcess > 1)
        return true;

    // MULTI INSERTION MULTI SQL_THREAD
    // If MyIsam database
    // Multi insertion in the same testing stage from different threads is not allowed
    // TestingStage must be defined
    // Check temporary splitlot_id into the splitlot table
    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        if (! m_pclDatabaseConnector->m_bTransactionDb)
        {
            QString strTableName = NormalizeTableName("_SPLITLOT");
            QString strQuery;
            GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

            // Lock tables not necessary for MyISAM table (AUTOCOMMIT ON)
            // Get the list of actif SID (add to temporary splitlot format) (get the current SID with insertion)
            // And check if have those temporary splitlots in splitlot table (= active insertion)
            strQuery = "SELECT SPLITLOT_ID, INSERTION_TIME FROM " + strTableName;
            strQuery += " WHERE (SPLITLOT_ID >= 101000000 AND SPLITLOT_ID < 200000000) AND SPLITLOT_ID IN(";
            strQuery += "        SELECT 101000000+ID FROM information_schema.PROCESSLIST ";
            strQuery += "        WHERE USER='" + m_pclDatabaseConnector->m_strUserName_Admin + "'";
            strQuery += ")";
            if (! clGexDbQuery.Execute(strQuery))
            {
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            int nCurrentInsertionTime = 0;
            int nOtherInsertionTime = 0;
            int nOtherSplitlotId = 0;
            bool bInsertionToDelay = false;

            // case 5607
            // The gexdb insertions start exactly at the same time
            // insertion_time are identical
            // use the SID to select the winner

            while (clGexDbQuery.Next())
            {
                if (clGexDbQuery.value(0).toLongLong() == m_nSplitLotId)
                {
                    // The current insertion
                    nCurrentInsertionTime = clGexDbQuery.value(1).toInt();
                }
                else if (nOtherInsertionTime == clGexDbQuery.value(2).toInt())
                {
                    // Another process
                    // Special case (same insertion time)
                    if (nOtherSplitlotId < clGexDbQuery.value(0).toLongLong())
                        nOtherSplitlotId = clGexDbQuery.value(0).toLongLong();
                }
                else if (nOtherInsertionTime < clGexDbQuery.value(2).toInt())
                {
                    // Another process
                    // start before the previous selected
                    nOtherSplitlotId = clGexDbQuery.value(0).toLongLong();
                    nOtherInsertionTime = clGexDbQuery.value(2).toInt();
                }
            }
            if ((nOtherInsertionTime > 0) && (nCurrentInsertionTime > 0))
            {
                // an insertion in the same testingstage occurs in another thread
                // allow the first, delay the other
                if (nCurrentInsertionTime > nOtherInsertionTime)
                    bInsertionToDelay = true;
                // if exactly the same insertion time
                // select the first SID
                if ((nCurrentInsertionTime == nOtherInsertionTime)
                        && (m_nSplitLotId < nOtherSplitlotId))
                    bInsertionToDelay = true;
            }

            if (bInsertionToDelay)
            {
                // delay this insertion
                *m_pbDelayInsertion = true;
                GSET_ERROR1(GexDbPlugin_Base, eDB_InvalidTransaction, NULL,
                            "MyISAM database doesn't support multi-threading insertion in the same testing stage");
                return false;
            }
        }
    }

    // Check some records
    // MIR.START_TIME != 0
    if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME)
            && (mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong() == 0))
    {
        GSET_ERROR1(GexDbPlugin_Base, eValidation_EmptyField, NULL, "START_TIME");
        if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME)
            return false;
        strMessage = "[Record#MIR] ";
        strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        WarningMessage(strMessage);
    }

    // Check if Time was overwritten by KeyContent
    // Check if new values are allowed
    if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME)
            && (mpDbKeysEngine->dbKeysContent().Get("SetupTime").toLongLong() != (int) m_clStdfMIR.m_u4SETUP_T)
            && ((mpDbKeysEngine->dbKeysContent().Get("SetupTime").toLongLong() > mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong())
                || (mpDbKeysEngine->dbKeysContent().Get("SetupTime").toLongLong() > mpDbKeysEngine->dbKeysContent().Get("FinishTime").toLongLong())))
    {
        // Incorrect date.
        //"Date %s has been overloaded with value %s (through dbkeys mapping): %s."
        GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL, "Date SetupTime",
                    QString::number(mpDbKeysEngine->dbKeysContent().Get("SetupTime").toLongLong()).toLatin1().data(),
                    "SetupTime value must be less than StartTime and FinishTime");
        if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME)
        {
            *m_pbDelayInsertion = true;
            return false;
        }
        strMessage = "[Record#MIR] ";
        strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        WarningMessage(strMessage);
    }
    if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME)
            && ((mpDbKeysEngine->dbKeysContent().IsOverloaded("StartTime")) && !mpDbKeysEngine->dbKeysContent().Get("StartTime").toString().isEmpty())
            && (   (mpDbKeysEngine->dbKeysContent().Get("SetupTime").toLongLong() > mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong())
                   || (mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong() > mpDbKeysEngine->dbKeysContent().Get("FinishTime").toLongLong())))
    {
        // Incorrect date.
        //"Date %s has been overloaded with value %s (through dbkeys mapping): %s."
        GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL, "Date StartTime",
                    QString::number(mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong()).toLatin1().data(),
                    "StartTime value must be between SetupTime and FinishTime");

        if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME)
        {
            *m_pbDelayInsertion = true;
            return false;
        }
        strMessage = "[Record#MIR] ";
        strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        WarningMessage(strMessage);
    }

    // After m_strProductName updated on Pass 2
    // After m_strLotId updated on Pass 2
    // After m_strWaferId updated on Pass 2
    // After m_strSubLotId updated on Pass 2


    if (m_lPass == 2)
    {
        QString strTableName = NormalizeTableName("_SPLITLOT");
        QString strQuery, strWaferID;
        GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

        ///////////////////////////////
        // BEGIN FLEXIBLE CONSOLIDATION
        // One Pass 2, all needed data are inserted into the wt_splitlot table
        if(m_eTestingStage == eWaferTest)
        {

            // PROD_DATA='N' => INSERT WITHOUT CONSOLIDATION
            // PROD_DATA='Y' AND SUMMARY FILE
            //   o IF TEST CONDITIONS DEFINED FOR THIS PRODUCT => REJECT THE INSERTION
            //   o IF CONSOLIDATION RULES DEFINED FOR THIS PRODUCT => REJECT THE INSERTION
            //   o THEN UPDATE WITH THE LAST SUMMARY
            // PROD_DATA='Y' AND SAMPLES FILE => INSERT WITH CONSOLIDATION

            if(mpDbKeysEngine->dbKeysContent().Get("ProdData").toBool())
            {
                if(m_pConsolidationTree == NULL)
                {
                    GSET_ERROR1(GexDbPlugin_Base, eValidation_MissingRecords, NULL, "No default consolidation rule defined");
                    return false;
                }

                // Warning if Wt_YieldConsolidation_Rule is defined
                if(!mpDbKeysEngine->dbKeysContent().Get("Wt_YieldConsolidation_Rule").toString().isEmpty())
                {
                    strMessage = QString("Wt_YieldConsolidation_Rule has been overloaded "
                                         "with value %1 (through dbkeys mapping). ").
                            arg(mpDbKeysEngine->dbKeysContent().Get("Wt_YieldConsolidation_Rule").toString());
                    strMessage+= "This dbkeys was ignored";
                    GSET_ERROR1(GexDbPlugin_Base, eDB_Consolidation, NULL,
                                strMessage.toLatin1().constData());
                    WarningMessage(strMessage);

                }

                CTQueryFilter filters;
                ConsolidationTreeQueryEngine queryEng(*m_pConsolidationTree);

                CTTestCondition tcItem;
                QList<CTTestCondition> tcList;
                QList<CTConsolidationRule> crList;

                QDate clStartTime = QDateTime::fromTime_t(mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong()).date();

                // From consolidation tree
                // Check if some consolidation rules are defined for this product
                filters.add(CTQueryFilter::FilterTestingStage, "Wafer Sort");
                filters.add(CTQueryFilter::FilterProductID, m_strProductName);
                filters.add(CTQueryFilter::FilterDate,clStartTime.toString("yyyy-MM-dd"));

                tcList = queryEng.findTestConditions(filters);
                crList = queryEng.findConsolidationRules(filters);

                // Check TEST CONDITIONS
                if(m_bUsePcrForSummary && !tcList.isEmpty())
                {
                    *m_pbDelayInsertion = true;
                    GSET_ERROR1(GexDbPlugin_Base, eValidation_MissingRecords, NULL, "Summary wafer with specific Test Conditions defined");
                    return false;
                }

                if(!tcList.isEmpty())
                {

                    QString tcName;
                    QString tcField;
                    QStringList tcValues;

                    // From splitlot table
                    // Check if test conditions are valid
                    foreach(tcItem, tcList)
                    {
                        tcName = tcItem.conditionName();
                        tcField = tcItem.splitlotField();
                        tcValues = tcItem.allowedValues();

                        // Check if the test condition field is valid
                        strQuery = "SELECT "+tcField+" FROM " + strTableName;
                        strQuery += " WHERE SPLITLOT_ID="+QString::number(m_nSplitLotId);

                        if(!clGexDbQuery.Execute(strQuery))
                        {
                            *m_pbDelayInsertion = true;
                            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                                        clGexDbQuery.lastError().text().toLatin1().constData());
                            return false;
                        }

                        if(!clGexDbQuery.First())
                        {
                            *m_pbDelayInsertion = true;
                            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), " return no result");
                            return false;
                        }

                        // Check if the test condition value is allowed
                        if(!tcItem.isAllowedValue(clGexDbQuery.value(0).toString()))
                        {
                            *m_pbDelayInsertion = true;
                            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), " not allowed test condition value");
                            return false;
                        }
                    }
                }

                // Check CONDITION RULES
                if(m_bUsePcrForSummary && (queryEng.hasConsolidation(filters)))
                {
                    *m_pbDelayInsertion = true;
                    GSET_ERROR1(GexDbPlugin_Base, eValidation_MissingRecords, NULL, "Summary wafer with specific Condition Rules defined");
                    return false;
                }
            }
        }
        // END FLEXIBLE CONSOLIDATION
        /////////////////////////////

        // MIR.SUBLOT_ID != ""
        if ((m_eTestingStage == eFinalTest)
                && (m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID)
                && (m_strSubLotId.isEmpty()))
        {
            GSET_ERROR1(GexDbPlugin_Base, eValidation_EmptyField, NULL, "SUBLOT_ID");
            if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID)
                return false;
            strMessage = "[Record#MIR] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(strMessage);
        }

        // MIR.LOT_ID != ""
        if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID)
                && (m_strLotId.isEmpty()))
        {
            GSET_ERROR1(GexDbPlugin_Base, eValidation_EmptyField, NULL, "LOT_ID");
            if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID)
                return false;
            strMessage = "[Record#MIR] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(strMessage);
        }

        // MIR.PART_TYP != ""
        if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID)
                && (m_strProductName.isEmpty()))
        {
            if ((! m_clStdfMIR.m_cnPART_TYP.isEmpty())
                    && (mpDbKeysEngine->dbKeysContent().Get("Product").toString().isEmpty()))
            {
                // gexdbkeys error
                GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL, "ProductName", m_strProductName.toLatin1().constData(), "Empty ProductName not allowed");
            }
            else
            {
                GSET_ERROR1(GexDbPlugin_Base, eValidation_EmptyField, NULL, "PART_TYP");
            }
            if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID)
                return false;
            strMessage = "[Record#MIR] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(strMessage);
        }

        QString lTestInsertion = mpDbKeysEngine->dbKeysContent().Get("TestInsertion").toString();
        if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID)
                && (lTestInsertion.isEmpty()))
        {
            // gexdbkeys error
            GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL, "TestInsertion", lTestInsertion.toLatin1().constData(), "Empty TestInsertion not allowed");
            if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID)
                return false;
            strMessage = "[dbkeys#TestInsertion] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(strMessage);
        }

        if ((m_eTestingStage == eWaferTest)
                || (m_eTestingStage == eElectTest))
        {
            // WIR.WAFER_ID != ""
            if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID)
                    && (m_strWaferId.isEmpty()))
            {
                GSET_ERROR1(GexDbPlugin_Base, eValidation_EmptyField, NULL, "WAFER_ID");
                if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FIELDLOTID)
                    return false;
                strMessage = "[Record#WIR] ";
                strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                WarningMessage(strMessage);
            }
        }

        if (m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_ALREADYINSERTED)
        {
            int nStartTime;
            nStartTime = mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong();

            // case 7233: have WCR but no WIR
            if ((m_eTestingStage != eFinalTest) && (m_clStdfWIR.m_u4START_T > 0))
                nStartTime = m_clStdfWIR.m_u4START_T;

            // If overwritten, then force to Key value
            if ((mpDbKeysEngine->dbKeysContent().IsOverloaded("StartTime"))
                    && ! mpDbKeysEngine->dbKeysContent().Get("StartTime").toString().isEmpty())
                nStartTime = mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong();

            strQuery = "SELECT SPLITLOT_ID, VALID_SPLITLOT, INSERTION_TIME FROM " + strTableName;
            if ((m_eTestingStage != eFinalTest) && (! m_strWaferId.isEmpty()))
            {
                // join with wafer_info
                strWaferID = m_strWaferId;
                strQuery += " WHERE ";
                strQuery += " WAFER_ID=" + TranslateStringToSqlVarChar(m_strWaferId);
                strQuery += " AND ";
            }
            else
                strQuery += " WHERE ";
            strQuery += " LOT_ID=" + TranslateStringToSqlVarChar(m_strLotId);
            strQuery += " AND START_T=";
            strQuery += QString::number(nStartTime);
            if (m_eTestingStage != eElectTest)
            {
                strQuery += " AND STAT_NUM=";
                strQuery += QString::number(mpDbKeysEngine->dbKeysContent().Get("Station").toInt());
                strQuery += " AND (lower(TESTER_NAME)=";
                strQuery += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("TesterName").toString()).toLower();
                if (mpDbKeysEngine->dbKeysContent().Get("TesterName").toString().isEmpty())
                    strQuery += " OR TESTER_NAME IS NULL";
                strQuery += " )";
                // case 6897 - same MIR but split files or retest
                // allow duplication when the retest_index is different between the files
                strQuery += " AND RETEST_INDEX=";
                strQuery += QString::number(mpDbKeysEngine->dbKeysContent().Get("RetestIndex").toInt());
                // and/or the part counts are different
                strQuery += " AND NB_PARTS_SAMPLES=";
                strQuery += QString::number(m_mapNbRuns[MERGE_SITE]);
            }
            strQuery += " AND ";
            // MULTI INSERTION MULTI SQL_THREAD
            // Ignore all invalid_splitlot excepts for actif SID
            strQuery += "( (VALID_SPLITLOT<>'N')";
            // GCORE-4202
            // Because of Spider
            // The MySql session_id cannot be used
            // Use the key inserted into the 'token' table that lock the temporary splitlot_id for each YM
            QString lTokenTable = NormalizeTableName("token", false);
            strQuery += " OR (SPLITLOT_ID IN (SELECT key_value FROM "+lTokenTable+" WHERE name='"+NormalizeTableName("_INSERTION")+"'))";
            /*
               if(m_pclDatabaseConnector->IsMySqlDB())
               {
                strQuery += " OR ";
                strQuery += "  ((SPLITLOT_ID >= 101000000 AND SPLITLOT_ID < 200000000) AND SPLITLOT_ID IN (";
                strQuery += "                SELECT 101000000+ID FROM information_schema.PROCESSLIST ";
                strQuery += "                WHERE USER='"+m_pclDatabaseConnector->m_strUserName_Admin+"'";
                strQuery += "                                        )";
                strQuery += "        )";
               }
             */
            strQuery += ")";

            if (! clGexDbQuery.Execute(strQuery))
            {
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }

            int nCurrentInsertionTime = 0;
            int nOtherInsertionTime = 0;
            int nOtherSplitlotId = 0;
            int lExistingSplilotID = 0;
            bool bHaveValidSplitlot = false;
            bool bInsertionToDelay = false;

            // case 5607
            // The gexdb insertions start exactly at the same time
            // insertion_time are identical
            // use the SID in the SplitlotId to select the winner

            while (clGexDbQuery.Next())
            {
                if (clGexDbQuery.value("VALID_SPLITLOT").toString().toUpper() == QString(FLAG_SPLITLOT_VALID).toUpper())
                {
                    bHaveValidSplitlot = true;
                    lExistingSplilotID = clGexDbQuery.value(0).toLongLong();
                }

                if (clGexDbQuery.value("SPLITLOT_ID").toLongLong() == m_nSplitLotId)
                {
                    // The current insertion
                    nCurrentInsertionTime = clGexDbQuery.value("INSERTION_TIME").toInt();
                }
                else if (nOtherInsertionTime == clGexDbQuery.value("INSERTION_TIME").toInt())
                {
                    // Another process
                    // Special case (same insertion time)
                    if (nOtherSplitlotId < clGexDbQuery.value("SPLITLOT_ID").toLongLong())
                        nOtherSplitlotId = clGexDbQuery.value("SPLITLOT_ID").toLongLong();
                }
                else if (nOtherInsertionTime < clGexDbQuery.value("INSERTION_TIME").toInt())
                {
                    // Another process
                    // start before the previous selected
                    nOtherSplitlotId = clGexDbQuery.value("SPLITLOT_ID").toLongLong();
                    nOtherInsertionTime = clGexDbQuery.value("INSERTION_TIME").toInt();
                }
            }

            if ((nOtherInsertionTime > 0) && (nCurrentInsertionTime > 0))
            {
                // an insertion in the same testingstage occurs in another thread
                // allow the first, delay the other
                if (nCurrentInsertionTime > nOtherInsertionTime)
                    bInsertionToDelay = true;
                // if exactly the same insertion time
                // select the first SID
                if ((nCurrentInsertionTime == nOtherInsertionTime)
                        && (m_nSplitLotId < nOtherSplitlotId))
                    bInsertionToDelay = true;
            }

            if (bHaveValidSplitlot || bInsertionToDelay)
            {
                QDateTime clDateTime;
                clDateTime.setTime_t(nStartTime);
                clDateTime.setTimeSpec(Qt::UTC);
                QString strTestingStageName;
                GetCurrentTestingStageName(strTestingStageName);

                // Check if it is a VALID_SPLITLOT
                if (bHaveValidSplitlot)
                {
                    // Valid splitlot already inserted
                    // If existing data have to be replaced
                    //    - Keep the existing splitlotID in the DBKeysContent. This existing splitlot id will be set
                    //      to invalid once the new data will be successfully inserted
                    // Otherwise
                    //    - Reject insertion as data already exists.
                    if (mpDbKeysEngine && mpDbKeysEngine->dbKeysContent().Get("ReplaceExistingData").toBool() == true)
                    {
                        mpDbKeysEngine->dbKeysContent().Set("OverwrittenSplitlotID", lExistingSplilotID);
                    }
                    else
                    {
                        GSET_ERROR6(GexDbPlugin_Base, eValidation_AlreadyInserted, NULL,
                                    strTestingStageName.toLatin1().constData(), m_strLotId.toLatin1().constData(),
                                    strWaferID.isEmpty() ? "" : strWaferID.toLatin1().constData(),
                                    mpDbKeysEngine->dbKeysContent().Get("TesterName").toString().toLatin1().constData(),
                                    mpDbKeysEngine->dbKeysContent().Get("Station").toInt(),
                                    clDateTime.toString(Qt::ISODate).toLatin1().constData());

                        if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_ALREADYINSERTED)
                            return false;
                        strMessage = "[Record#MIR] ";
                        strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                        WarningMessage(strMessage);
                    }
                }
                else
                {
                    // An other thread currently inserts this splitlot
                    // Have to wait
                    *m_pbDelayInsertion = true;
                    strMessage = "Data for this splitlot is currently being inserted into the DB through multi-threading insertion (Testing stage=%1, Lot=%2, WaferID=%3, Tester=%4, Station=%5, Start_Time=%6)";
                    GSET_ERROR1(GexDbPlugin_Base, eDB_InvalidTransaction, NULL,
                                strMessage.arg(strTestingStageName, m_strLotId, strWaferID.isEmpty() ? "" : strWaferID,
                                               mpDbKeysEngine->dbKeysContent().Get("TesterName").toString(), QString::number(mpDbKeysEngine->dbKeysContent().Get("Station").toInt()),
                                               clDateTime.toString(Qt::ISODate)).toLatin1().data());

                    return false;
                }
            }

            // case 9725: Warning if have MULTI-SUBLOTS for a specific wafer
            // If the DB already contains MULTI-SUBLOTS with the current
            // Get the list of inserted SubLots for this wafer
            if ((m_eTestingStage == eWaferTest) || (m_eTestingStage == eElectTest))
            {
                QStringList lInsertedSublots;
                strQuery = "SELECT distinct SUBLOT_ID FROM " + strTableName;
                // join with wafer_info
                strWaferID = m_strWaferId;
                strQuery += " WHERE ";
                strQuery += " WAFER_ID=" + TranslateStringToSqlVarChar(m_strWaferId);
                strQuery += " AND ";
                strQuery += " LOT_ID=" + TranslateStringToSqlVarChar(m_strLotId);
                strQuery += " AND ";
                strQuery += " VALID_SPLITLOT<>'N'";
                if (! clGexDbQuery.Execute(strQuery))
                {
                    *m_pbDelayInsertion = true;
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                                clGexDbQuery.lastError().text().toLatin1().constData());
                    return false;
                }
                while (clGexDbQuery.Next())
                {
                    if (! lInsertedSublots.contains(clGexDbQuery.value("SUBLOT_ID").toString().toUpper()))
                        lInsertedSublots << clGexDbQuery.value("SUBLOT_ID").toString().toUpper();

                }
                if (! lInsertedSublots.isEmpty()
                        && ! lInsertedSublots.contains(m_strSubLotId.toUpper()))
                {
                    // Some data were already inserted
                    // and the current Sublot doesn't match
                    // Warning for MULTI-SUBLOTS insertion
                    strMessage = QString("This Lot[%1]Sublot[%2]Wafer[%3] was already inserted but with a different SubLot[%4]")
                            .arg(m_strLotId).arg(m_strSubLotId).arg(m_strWaferId).arg(lInsertedSublots.first());
                    WarningMessage(strMessage);
                }
            }

            // GCORE-2682 Multi-product: Insertion
            // PRODUCT/LOT/SUBLOT into XX_SPLITLOT
            // 1 line per PRODUCT/LOT/SUBLOT into XX_SUBLOT_INFO
            //      A, LOT1, SUBLOT1
            //      A, LOT1, SUBLOT2
            //      B, LOT1, SUBLOT3
            //      B, LOT1, SUBLOT4
            // 1 line per PRODUC/LOT into XX_LOT
            //      A, LOT1
            //      B, LOT1
            // If option=TRUE
            //      then allow more than one product per lot
            //      but ONLY one product per sublot
            // If option=FALSE
            //      then ONLY one product per lot
            QString lMultiProducts;
            GetGlobalOptionValue(eInsertionAllowMultiProducts, lMultiProducts);
            // DB-67 - Overwrite alrady inserted WaferId+BADProduct
            // Do the verification directly on the SPLITLOT table as the PRODUCT_NAME is in the table now
            // Then during the UpdateSubLot, overwrite the product name
            strQuery = "SELECT DISTINCT PRODUCT_NAME FROM " + NormalizeTableName("_SPLITLOT");;
            strQuery += " WHERE LOT_ID=" + TranslateStringToSqlVarChar(m_strLotId);
            strQuery += " AND VALID_SPLITLOT<>'N'";
            if (lMultiProducts == "TRUE")
            {
                if (m_eTestingStage == eFinalTest)
                {
                    strQuery += " AND SUBLOT_ID=" + TranslateStringToSqlVarChar(m_strSubLotId);
                }
                else
                {
                    strQuery += " AND WAFER_ID=" + TranslateStringToSqlVarChar(m_strWaferId);
                }
            }

            // Check if have a new Product
            strQuery += " AND PRODUCT_NAME<>" + TranslateStringToSqlVarChar(m_strProductName);
            if (! clGexDbQuery.Execute(strQuery))
            {
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            if (clGexDbQuery.First())
            {
                // Already have an insertion for this lot/sublot
                // The ProductName already inserted into the GexDb doesn't match with the current ProductName
                *m_pbDelayInsertion = true;
                if (lMultiProducts == "TRUE")
                    strMessage =
                            QString("This Lot[%1]-SubLot[%2] was already inserted but with a different ProductName: ProductName already inserted = '%3', New ProductName = '%4'")
                            .arg(m_strLotId, m_strSubLotId, clGexDbQuery.value(0).toString(), m_strProductName);
                else
                    strMessage =
                            QString("This Lot[%1] was already inserted but with a different ProductName: ProductName already inserted = '%2', New ProductName = '%3'")
                            .arg(m_strLotId, clGexDbQuery.value(0).toString(), m_strProductName);
                GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidField, NULL,
                            strMessage.toLatin1().constData());
                return false;
                // case 6006: case sensitive issue with Oracle
                // Do not support Oracle and the case sensitive issue
            }


            // Check if have new TrackingLot
            // The TrackingLot must be unique per Lot
            strQuery = "SELECT distinct TRACKING_LOT FROM " + NormalizeTableName("_LOT");
            strQuery += " WHERE LOT_ID=" + TranslateStringToSqlVarChar(m_strLotId);
            // Check if have a new TrackingLot
            strQuery += " AND TRACKING_LOT<>" + TranslateStringToSqlVarChar(m_strTrackingLotId);
            if (clGexDbQuery.First())
            {
                // Already have an insertion for this lot
                // The TrackingLot already inserted into the GexDb doesn't match with the current TrackingLot
                *m_pbDelayInsertion = true;
                strMessage =
                        QString("This Lot[%1] was already inserted but with a different TrackingLot: TrackingLot already inserted = '%3', New TrackingLot = '%4'")
                        .arg(m_strLotId, clGexDbQuery.value(0).toString(), m_strTrackingLotId);
                GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidField, NULL,
                            strMessage.toLatin1().constData());
                return false;
            }
        }

        // From config.gexdbkeys
        if(mpDbKeysEngine->GetTestConditionsOrigin() != GS::QtLib::DatakeysEngine::DTR)
        {
            QString lValue;
            mpDbKeysEngine->dbKeysContent().GetDbKeyContent("CountTestConditions",lValue);
            int lCountDynamicKeys = mpDbKeysEngine->GetCountDynamicKeys();
            int lCountTestConditions = lValue.toInt();
            int lCountTests = m_lstTestInfo.size();
            // With a lot of Tests
            if((lCountTests > 10000)
                    // More than 10 definitions
                    && (lCountDynamicKeys > 10))
            {
                bool lWarningMessage = false;
                if(lCountTestConditions == 0)
                {
                    lWarningMessage = true;
                }
                else
                {
                    // More than 10 distinct TestConds
                    if((lCountTestConditions > 10)
                            // More than 2 definitions per TestCond)
                            && (lCountDynamicKeys > (lCountTestConditions * 2)))
                    {
                        lWarningMessage = true;
                    }
                }
                if(lWarningMessage)
                {
                    WarningMessage(QString("Check your gexdbkeys file (according to the number of Tests(=%1), the number of DynamicKeys(=%2) can have an impact on the insertion time)")
                                   .arg(lCountTests)
                                   .arg(lCountDynamicKeys));
                }
            }
        }

        //////////////////////
        // INTERMEDIATE CONSOLIDATION AT FINAL TEST
        // Check the RetestPhase
        if(m_eTestingStage == eFinalTest)
        {
            QString lProdData;
            QStringList lstPhases;
            strQuery = "SELECT count(*) FROM "+NormalizeTableName("_SPLITLOT");
            strQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(m_strLotId);
            strQuery += " AND SUBLOT_ID="+TranslateStringToSqlVarChar(m_strSubLotId);
            strQuery += " AND VALID_SPLITLOT<>'N'";
            if(!clGexDbQuery.Execute(strQuery))
            {
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            if(clGexDbQuery.First() && clGexDbQuery.value(0)>0)
                if(!GetConsolidationRetestPhases(m_strLotId, m_strSubLotId,lProdData,lstPhases))
                    return false;
        }
    }

    if (mpDbKeysEngine->dbKeysContent().Get("RejectIfNbPartsLessThan").toInt() > -1)
    {
        // KeysContent containts the count of all the STDF records
        int NbParts = mpDbKeysEngine->dbKeysContent().StdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR];
        if (NbParts < mpDbKeysEngine->dbKeysContent().Get("RejectIfNbPartsLessThan").toInt())
        {
            strMessage = "The number of Parts[" + QString::number(NbParts) + "] is insufficient";
            GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL,
                        "RejectIfNbPartsLessThan",QString::number(mpDbKeysEngine->dbKeysContent().Get("RejectIfNbPartsLessThan").toInt()).toLatin1().constData(),
                        strMessage.toLatin1().constData());
            return false;
        }
    }

    if ((mpDbKeysEngine->dbKeysContent().Get("RejectIfNbPartsPercentOfGDPWLessThan").toInt() > -1)
            && (mpDbKeysEngine->dbKeysContent().Get("GrossDie").toUInt() > 0))
    {
        // KeysContent containts the count of all the STDF records
        // Nb Parts < GrossDie*Percent
        int NbParts = mpDbKeysEngine->dbKeysContent().StdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR];
        int GrossDie = static_cast<int> (mpDbKeysEngine->dbKeysContent().Get("GrossDie").toUInt());
        if (NbParts < (GrossDie * mpDbKeysEngine->dbKeysContent().Get("RejectIfNbPartsPercentOfGDPWLessThan").toInt()))
        {
            strMessage = "Gross Die = " + QString::number(GrossDie);
            strMessage+= ". The number of Parts["+QString::number(mpDbKeysEngine->dbKeysContent().StdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR])+"] is insufficient";
            GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL,
                        "RejectIfNbPartsPercentOfGDPWLessThan",QString::number(mpDbKeysEngine->dbKeysContent().Get("RejectIfNbPartsPercentOfGDPWLessThan").toInt()).toLatin1().constData(),
                        strMessage.toLatin1().constData());
            return false;
        }
    }

    if (mpDbKeysEngine->dbKeysContent().Get("IgnoreResultsIfNbPartsMoreThan").toInt() > -1)
    {
        // KeysContent containts the count of all the STDF records
        int NbParts = mpDbKeysEngine->dbKeysContent().StdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR];
        if (NbParts > mpDbKeysEngine->dbKeysContent().Get("IgnoreResultsIfNbPartsMoreThan").toInt())
        {
            // Ignore all results
            mpDbKeysEngine->dbKeysContent().SetInternal("IgnoreResults", "TRUE");
        }
    }

    if (bOnlyHeaderValidation)
    {
        // Debug message
        WriteDebugMessageFile("     GexDbPlugin_Galaxy::ValidationFunction completed");
        return true;
    }

    // Only at the end of the Pass 1
    if (m_lPass == 1)
    {
        // WAFER VALIDITY
        // Only at the end of the Pass 1 and at the beginning of the Pass 2 (before reinitialization)

        // Is Testing Stage forced to WT or ET?
        // If so, make sure Wafer info is set even if no wafer records present
        if ((mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().toLower() == "e")
                || (mpDbKeysEngine->dbKeysContent().Get("ForceTestingStage").toString().toLower() == "w"))
        {
            //
            // Check if have no WIR
            // At the end of the Pass1
            if (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] == 0)
            {
                m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] = 1;
                m_nTotalWaferToProcess = 1;
            }

            if (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR] == 0)
            {
                m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR] = 1;
                // Simulate a WRR
                m_clStdfWRR.SetWAFER_ID(m_strWaferId);
                m_clStdfWRR.SetFINISH_T(mpDbKeysEngine->dbKeysContent().Get("FinishTime").toULongLong());
            }
        }

        // MRR was read
        // Check if Time was overwritten for StartTime and FinishTime
        // Not allowed if have multi-wafer
        if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME)
                && (((mpDbKeysEngine->dbKeysContent().IsOverloaded("StartTime")) && !mpDbKeysEngine->dbKeysContent().Get("StartTime").toString().isEmpty())
                    ||  ((mpDbKeysEngine->dbKeysContent().IsOverloaded("FinishTime")) && !mpDbKeysEngine->dbKeysContent().Get("FinishTime").toString().isEmpty()))
                && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] > 1))
        {
            // Incorrect date.
            //"Date %s has been overloaded with value %s (through dbkeys mapping): %s."
            GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL, "Date (StartTime or FinishTime)", "",
                        "StartTime and FinishTime cannot be overloaded for a multi-wafer file");
            if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME)
            {
                *m_pbDelayInsertion = true;
                return false;
            }
            strMessage = "[Record#MIR] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(strMessage);
        }

        if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME)
                && ((mpDbKeysEngine->dbKeysContent().IsOverloaded("FinishTime")) && !mpDbKeysEngine->dbKeysContent().Get("FinishTime").toString().isEmpty())
                && (   (mpDbKeysEngine->dbKeysContent().Get("SetupTime").toLongLong() > mpDbKeysEngine->dbKeysContent().Get("FinishTime").toLongLong())
                       || (mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong() > mpDbKeysEngine->dbKeysContent().Get("FinishTime").toLongLong())))
        {
            // Incorrect date.
            //"Date %s has been overloaded with value %s (through dbkeys mapping): %s."
            GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL, "Date FinishTime",
                        QString::number(mpDbKeysEngine->dbKeysContent().Get("FinishTime").toLongLong()).toLatin1().data(),
                        "FinishTime value must be greater than SetupTime and StartTime");

            if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTDATETIME)
            {
                *m_pbDelayInsertion = true;
                return false;
            }
            strMessage = "[Record#MIR] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(strMessage);
        }

        if ((m_eTestingStage == eWaferTest)
                || (m_eTestingStage == eElectTest))
        {
            // before the second pass we have information about the number of wafer in the file
            // if the wafer_id was overloaded by DbKeys, it's not possible to process multi wafer in one file
            if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRMULTIHEAD)
                    && ((mpDbKeysEngine->dbKeysContent().IsOverloaded("Wafer")) && !mpDbKeysEngine->dbKeysContent().Get("Wafer").toString().isEmpty() )
                    && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] > 1))
            {
                GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL, "WaferID",
                            mpDbKeysEngine->dbKeysContent().Get("Wafer").toString().toLatin1().constData(),
                            "Not allowed for a multi-wafer file");

                if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRMULTIHEAD)
                    return false;
                strMessage = "[Record#WIR] ";
                strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                WarningMessage(strMessage);
            }

            // Check if all WIR records have one WRR record
            // else ERROR
            if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRWRRCOUNT)
                    && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] != m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR]))
            {
                GSET_ERROR4(GexDbPlugin_Base, eValidation_CountMismatch, NULL,
                            "WIR", m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR],
                        "WRR", m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR]);

                // GCORE-3021 - allow WIR<>WRR if have some summary records
                bool bIgnoreMissingRecords = false;
                QString lOptionValue;
                GetGlobalOptionValue(eInsertionAllowMissingStdfRecords, lOptionValue);
                if (lOptionValue.contains("WRR"))
                {
                    // User allows missing WRR
                    // Check if we have some summary data
                    // Only 1 wafer per file
                    if (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] == 1)
                    {
                        // At least one summary records
                        if ((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR] > 0)
                                || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR] > 0)
                                || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SBR] > 0)
                                || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR] > 0)
                                || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MRR] > 0)
                                )
                            bIgnoreMissingRecords = true;
                    }

                }
                if (! bIgnoreMissingRecords)
                {
                    if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRWRRCOUNT)
                        return false;
                }
                strMessage = "[Record#WIR] ";
                strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                WarningMessage(strMessage);
            }
            // Check if not multi-head for wafer
            // else ERROR
            if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRMULTIHEAD)
                    && (m_bHaveMultiHeadWafer))
            {
                GSET_ERROR1(GexDbPlugin_Base, eValidation_MultiRecords, NULL, "heads");
                if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRMULTIHEAD)
                    return false;
                strMessage = "[Record#WIR] ";
                strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                WarningMessage(strMessage);
            }
            // Check if have the same WaferId for WIR and WRR
            // if wafer id comes from pKeys then ignore
            if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRWRRWAFERID)
                    && ((mpDbKeysEngine->dbKeysContent().IsOverloaded("Wafer")) && mpDbKeysEngine->dbKeysContent().Get("Wafer").toString().isEmpty() )
                    && ! m_clStdfWIR.m_cnWAFER_ID.isEmpty()
                    && ! m_clStdfWRR.m_cnWAFER_ID.isEmpty()
                    && (m_clStdfWIR.m_cnWAFER_ID != m_clStdfWRR.m_cnWAFER_ID))
            {
                strMessage.sprintf("[Record#WIR] Not same WaferId between WIR(%s) record and WRR(%s) record",m_clStdfWIR.m_cnWAFER_ID.toLatin1().constData(),m_clStdfWRR.m_cnWAFER_ID.toLatin1().constData());
                WarningMessage(strMessage);
            }

            // Check if WaferNb is avalaible for Genealogy
            if (m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTWAFERNO)
            {
                bool bOk;
                int nWaferNb = m_strWaferId.trimmed().toInt(&bOk);
                if(!bOk)        nWaferNb=-1;
                if (m_eTestingStage == eWaferTest)
                {
                    // At WT, check if WaferNb has been overloaded through KeyContent
                    if (! mpDbKeysEngine->dbKeysContent().Get("WaferNb").toString().isEmpty())
                    {
                        int nWaferNbFromKey = mpDbKeysEngine->dbKeysContent().Get("WaferNb").toString().trimmed().toInt(&bOk);
                        if (bOk)
                            nWaferNb = nWaferNbFromKey;
                    }
                }
                if (nWaferNb < 0)
                {
                    // Delay the file
                    if (mpDbKeysEngine->dbKeysContent().Get("WaferNb").toString().isEmpty())
                        GSET_ERROR1(GexDbPlugin_Base, eValidation_EmptyField, NULL,QString("WaferId '"+m_strWaferId+"' cannot be used for WaferNb - overload WaferNb through dbkeys mapping").toLatin1().data());
                    else
                        GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL,"WaferNb", mpDbKeysEngine->dbKeysContent().Get("WaferNb").toString().toLatin1().data(),  "Incorrect expression");
                    if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_INCORRECTWAFERNO)
                    {
                        *m_pbDelayInsertion = true;
                        return false;
                    }
                    strMessage = "[Record#WIR] ";
                    strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                    WarningMessage(strMessage);
                }
            }
        }

        // Have valid Hard Bin
        if (! mpDbKeysEngine->dbKeysContent().Get("RejectIfPassHardBinNotInList").toString().isEmpty())
        {
            // Check if all Pass HardBin are in the list
            QStringList GoodBins = mpDbKeysEngine->dbKeysContent().Get("RejectIfPassHardBinNotInList").toString().split(",");
            QMap<int, structBinInfo>::Iterator itBinInfo;
            for (itBinInfo = m_mapHBinInfo.begin(); itBinInfo != m_mapHBinInfo.end(); ++itBinInfo)
            {
                if (itBinInfo.value().m_cBinCat == 'P')
                {
                    // Check if HardBin is in the list
                    if (! GoodBins.contains(QString::number(itBinInfo.key())))
                    {
                        strMessage = "The Hard bin[" + QString::number(itBinInfo.key()) + "] is not in the list";
                        GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL,
                                    "RejectIfPassHardBinsNotInList",mpDbKeysEngine->dbKeysContent().Get("RejectIfPassHardBinNotInList").toString().toLatin1().constData(),
                                    strMessage.toLatin1().constData());
                        return false;
                    }
                }
            }

        }

        // Error if WaferTest has more than 1 WIR
        if (m_eTestingStage == eWaferTest)
        {
            if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRCOUNT)
                    && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WIR] > 1))
            {
                GSET_ERROR1(GexDbPlugin_Base, eValidation_MultiRecords, NULL, "W-Sort wafers");
                if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDWIRCOUNT)
                    return false;
                strMessage = "[Record#WIR] ";
                strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                WarningMessage(strMessage);
            }
        }

        // Check some records
        // PIR/PRR count mismatch
        if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDMISMATCHCOUNT)
                && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]!=m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR]))
        {
            GSET_ERROR4(GexDbPlugin_Base, eValidation_CountMismatch, NULL,
                        "PRR", m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR],
                    "PIR", m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PIR]);
            // GCORE-3022 - allow PIR<>PRR if have some summary records
            bool bIgnoreMissingRecords = false;
            QString lOptionValue;
            GetGlobalOptionValue(eInsertionAllowMissingStdfRecords, lOptionValue);
            if (lOptionValue.contains("PRR"))
            {
                // User allows missing PRR
                // Check if we have some summary data
                // same count from summary
                // no result after the last pir
                if (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR] > 0)
                {
                    // At least one summary records
                    // PCR is a summary
                    // same count from summary
                    if ((int) m_clStdfPCR.m_u4PART_CNT == m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
                    {
                        // extract the real site number of the pir
                        unsigned short siteNumber =
                                GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                                (m_clStdfParse, m_clStdfPIR);
                        // no result after the last pir

                        if (mTotalTestsExecuted[siteNumber] == 0)
                            bIgnoreMissingRecords = true;
                    }
                }
            }
            if (! bIgnoreMissingRecords)
            {
                if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDMISMATCHCOUNT)
                    return false;
            }
            strMessage = "[Record#PIR] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(strMessage);
        }

        // MRR missing record
        if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDMRRCOUNT)
                && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MRR] == 0))
        {
            GSET_ERROR1(GexDbPlugin_Base, eValidation_MissingRecords, NULL, "MRR");
            // GCORE-3086 - allow no MRR if have some summary records
            bool bIgnoreMissingRecords = false;
            QString lOptionValue;
            GetGlobalOptionValue(eInsertionAllowMissingStdfRecords, lOptionValue);
            if (lOptionValue.contains("MRR"))
            {
                // User allows missing MRR
                // Check if we have some summary data
                // At least one summary records
                if ((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_WRR] > 0)
                        || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR] > 0)
                        || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR] > 0)
                        || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SBR] > 0)
                        || (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR] > 0)
                        )
                    bIgnoreMissingRecords = true;
            }
            if (! bIgnoreMissingRecords)
            {
                if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDMRRCOUNT)
                    return false;
            }
            strMessage = "[Record#MRR] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(strMessage);
        }

        if (! mpDbKeysEngine->dbKeysContent().Get("IgnoreSummary").toBool() && (m_eTestingStage != eElectTest))
        {
            // Check PCR records
            // if no then no error
            if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDPCRCOUNT)
                    && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR] == 0))
            {
                GSET_ERROR1(GexDbPlugin_Base, eValidation_MissingRecords, NULL, "PCR");
                if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDPCRCOUNT)
                    return false;
                strMessage = "[Record#PCR] ";
                strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
                WarningMessage(strMessage);
            }
        }

        // GCORE-489: now allowed to insert multiple SDR records, as we added the xx_sdr tables
        // GCORE-489: the check & warning could be removed
        // GCORE-489: we for now keep a warning, because only 1 of the SDR will be used in the xx_splitlot table
        // Check if only one SDR record
        // else WARNING (multi SDR detected)
        if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDSDRCOUNT)
                && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SDR] > 1))
        {
            GSET_ERROR1(GexDbPlugin_Base, eValidation_MultiRecords, NULL, "SDR");
            if (m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDSDRCOUNT)
                return false;
            strMessage = "[Record#SDR] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(strMessage);
        }

        // un jeu de limits par site au maximum (PTR)

        // dans les MPR
        // autant de pin que de resultat j=k
        // ou j=0 ou =1 pour une pin
        // chaque pin cree une entree dans MPTEST_INFO

        // Check if the user can use the option FORCE_INVALID_PART_BIN_WITH
        // This var only exists if the file contains invalid Part Bin
        // This option only allow if have one PCR/SBR/HBR that matchs with the Part count
        // With results
        // The PRR must be considered as valid if have the option and PRR count matchs with the summary
        // Without results,
        // case 1, PRR can be considered as valid if have the option and PRR count matchs with the summary
        // case 2, PRR can be ignored if have the option or not and valid PRR matchs with the summary
        if (mInsertionOptions.contains("FORCE_INVALID_PART_BIN_WITH"))
        {
            // Retrieve some info from Parts
            int lNbParts = m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR];
            int lNbInvalidParts = 0;
            bool lHaveResults = false;
            QString lInfo;
            QString lLastPartInfo;
            QStringList lPartsInfo = mInsertionOptions["UNVALID_PARTS_BIN_INFO"].toString().split("#");
            while (! lPartsInfo.isEmpty())
            {
                lInfo = lPartsInfo.takeFirst();
                GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch,NULL, QString("Invalid HBin and SBin - PartNb[%1]-HBin[%2]-SBin[%3] ")
                            .arg(lInfo.section("|", 0, 0))
                            .arg(lInfo.section("|", 1, 1))
                            .arg(lInfo.section("|", 2, 2))
                            .toLatin1().data());
                lLastPartInfo = QString(GGET_LASTERRORMSG(GexDbPlugin_Base, this)).section(":", 1);
                if(lLastPartInfo.endsWith(".")) lLastPartInfo.chop(1);
                WarningMessage("[Record#PRR] " + lLastPartInfo);
                ++lNbInvalidParts;
                if (lInfo.section("|", 3, 3).toInt() > 0)
                    lHaveResults = true;
            }

            // Retrieve some info from Summary
            QMap<int, structBinInfo>::Iterator itBin;
            int lNbPartsFromSummary = 0;
            if (m_mapPCRNbParts.contains(MERGE_SITE))
                lNbPartsFromSummary = m_mapPCRNbParts[MERGE_SITE];
            else
                lNbPartsFromSummary = m_mapPCRNbParts[INVALID_SITE];
            if (lNbPartsFromSummary == 0)
            {
                for (itBin = m_mapHBinInfo.begin(); itBin != m_mapHBinInfo.end(); ++itBin)
                    if (itBin.value().m_mapBinCnt.contains(INVALID_SITE))
                        lNbPartsFromSummary += itBin.value().m_mapBinCnt[INVALID_SITE];
            }
            if (lNbPartsFromSummary == 0)
            {
                for (itBin = m_mapSBinInfo.begin(); itBin != m_mapSBinInfo.end(); ++itBin)
                    if (itBin.value().m_mapBinCnt.contains(INVALID_SITE))
                        lNbPartsFromSummary += itBin.value().m_mapBinCnt[INVALID_SITE];
            }

            // Found some invalid PRR
            // or considere as unvalid, if no results + at the end of the file and matchs with the summary
            if (mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt() == -1)
            {
                // With results and without option, the file was already rejected
                // Else Parts must be ignored
                if (lNbPartsFromSummary != (lNbParts - lNbInvalidParts))
                {
                    GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch, NULL,
                                QString("Unable to ignore Invalid Parts Bin - Summary data doesn't match - %1").arg(lLastPartInfo).toLatin1().data());
                    return false;
                }
            }
            else
            {
                if (lNbPartsFromSummary == (lNbParts - lNbInvalidParts))
                {
                    // Summary only matchs Valid Parts
                    // Invalid Parts must be ignored
                    if (lHaveResults)
                    {
                        GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch, NULL,
                                    QString("Unable to ignore Invalid Parts Bin - Parts conntains results - %1").arg(lLastPartInfo).toLatin1().data());
                        return false;
                    }
                    // Update the option to ignore the Invalid Parts
                    mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"] = -1;
                }
                else if (lNbPartsFromSummary == lNbParts)
                {
                    // we have to check now if the HBin/SBin list contains the new number
                    // updated when processSBR/processHBR
                    if (! m_mapSBinInfo.contains(mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt())
                            || !m_mapSBinInfo[mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt()].m_mapBinCnt.contains(INVALID_SITE)
                            || ! m_mapHBinInfo.contains(mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt())
                            || !m_mapHBinInfo[mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toInt()].m_mapBinCnt.contains(INVALID_SITE))
                    {
                        GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch, NULL,
                                    QString("Unable to force Invalid Parts Bin - Summary data doesn't match - %1").arg(lLastPartInfo).toLatin1().data());
                        return false;
                    }
                }
                else
                {
                    GSET_ERROR1(GexDbPlugin_Base, eValidation_BinMismatch, NULL,
                                QString("Unable to ignore/force Invalid Parts Bin - Summary data doesn't match - %1").arg(lLastPartInfo).toLatin1().data());
                    return false;
                }
            }

            QString lForceBin = mInsertionOptions["FORCE_INVALID_PART_BIN_WITH"].toString();
            if (lForceBin == "-1")
            {
                lForceBin = "IGNORE";

                // Clean Parts count
                QMap<int, int>::Iterator itSiteInfo;
                if (m_mapHBinInfo.contains(65535)
                        && ! m_mapHBinInfo[65535].m_mapBinNbParts.isEmpty())
                {
                    for(itSiteInfo = m_mapHBinInfo[65535].m_mapBinNbParts.begin(); itSiteInfo != m_mapHBinInfo[65535].m_mapBinNbParts.end(); ++itSiteInfo)
                        if (m_mapNbRuns.contains(itSiteInfo.key()))
                            m_mapNbRuns[itSiteInfo.key()] -= m_mapHBinInfo[65535].m_mapBinNbParts[itSiteInfo.key()];
                }
                else if (m_mapSBinInfo.contains(65535)
                         && ! m_mapSBinInfo[65535].m_mapBinNbParts.isEmpty())
                {
                    for(itSiteInfo = m_mapSBinInfo[65535].m_mapBinNbParts.begin(); itSiteInfo != m_mapSBinInfo[65535].m_mapBinNbParts.end(); ++itSiteInfo)
                        if (m_mapNbRuns.contains(itSiteInfo.key()))
                            m_mapNbRuns[itSiteInfo.key()] -= m_mapSBinInfo[65535].m_mapBinNbParts[itSiteInfo.key()];
                }
                // Remove any info from PRR
                m_mapHBinInfo.remove(65535);
                m_mapSBinInfo.remove(65535);
            }
            QString lWarning = "[Record#PRR] Force Invalid Parts Bin # to " + lForceBin;
            WarningMessage(lWarning);
        }

        // if PCR record
        // verify have the same value
        // else warning
        if ((m_eTestingStage == eFinalTest)
                && ((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR] > 0)
                    && (! m_bUsePcrForSummary)))
        {
            // have collect some info
            // verify if have the same data in PCR

            int nSiteCumul = INVALID_SITE;
            if (m_mapPCRNbParts.contains(MERGE_SITE))
                nSiteCumul = MERGE_SITE;

            if ((m_uiInsertionValidationOptionFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_RECORDPCRCOUNT)
                    && (m_mapPCRNbParts[nSiteCumul] != 0)
                    && (m_mapPCRNbParts[nSiteCumul] != m_mapNbRuns[nSiteCumul]))
            {
                strMessage.sprintf("[Record#PCR] Not same Parts count between PCR(%d) record and Parts Results(%d)",m_mapPCRNbParts[nSiteCumul],m_mapNbRuns[nSiteCumul]);
                WarningMessage(strMessage);
            }
        }
    }

    // Debug message
    GSLOG(SYSLOG_SEV_INFORMATIONAL, " validation completed");

    return true;
}

//////////////////////////////////////////////////////////////////////
// CleanInvalidSplitlot
//////////////////////////////////////////////////////////////////////
// If the insertion was failed, have to make a roll back
// clean the database of last insertion
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::CleanInvalidSplitlot()
{
    if (mStopRequested)
    {
        WriteDebugMessageFile("Stop requested by user/service: no clean Invalid Splitlot");
        return;
    }
    if (m_nlInsertedSplitlots.isEmpty())
    {
        return;
    }

    // Debug message
    QString strMessage;
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::CleanInvalidSplitlot()");


    QString lQuery, lTableName;
    QStringList lTables;
    QStringList lSplitlots;
    QStringList lTablesCleaned;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // GCORE-12319
    if(mDbGlobalOptions[eMysqlSplitPartitionBy].mDefaultValue == "DISABLED")
    {
        // Select all tables linked to the splitlot_id field
        // except all views
        lQuery = "SELECT distinct T.table_name FROM  ";
        lQuery+= " information_schema.tables T ";
        lQuery+= " INNER JOIN ";
        lQuery+= " information_schema.columns C  ";
        lQuery+= " ON T.table_schema=C.table_schema AND T.table_name=C.table_name  ";
        lQuery+= " WHERE T.table_schema='"+m_pclDatabaseConnector->m_strSchemaName+"' AND T.table_name LIKE '"+m_strPrefixTable+"%'  ";
        lQuery+= " AND T.table_type='BASE TABLE'  ";
        lQuery+= " AND C.column_name='splitlot_id' ";
    }
    else
    {
        // Select all tables linked to the splitlot_id field
        // except tables with partitions
        lQuery = "SELECT distinct T.table_name FROM  ";
        lQuery += " information_schema.partitions T ";
        lQuery += " INNER JOIN ";
        lQuery += " information_schema.columns C  ";
        lQuery += " ON T.table_schema=C.table_schema AND T.table_name=C.table_name AND T.partition_name IS NULL ";
        lQuery += " WHERE T.table_schema='" + m_pclDatabaseConnector->m_strSchemaName + "' AND T.table_name LIKE '" +
                m_strPrefixTable + "%'  ";
        lQuery += " AND C.column_name='splitlot_id' ";
    }

    clGexDbQuery.Execute(lQuery);
    while (clGexDbQuery.Next())
    {
        lTables << clGexDbQuery.value("table_name").toString();
    }

    QList<int>::iterator it;
    for (it = m_nlInsertedSplitlots.begin(); it != m_nlInsertedSplitlots.end(); it++)
    {
        lSplitlots << QString::number(*it);
    }

    QStringList::const_iterator itTables;
    for (itTables = lTables.constBegin(); itTables != lTables.constEnd(); ++itTables)
    {
        lTableName = (*itTables);
        lQuery = "DELETE FROM " + lTableName + " WHERE SPLITLOT_ID IN (" + lSplitlots.join(",") + ")";
        clGexDbQuery.Execute(lQuery);
        if (clGexDbQuery.numRowsAffected() > 0)
        {
            lTablesCleaned << lTableName;
        }
    }
    if (! lTablesCleaned.isEmpty())
    {
        // Trace message
        strMessage = "     Following tables cleaned for splitlot " + lSplitlots.join(",");
        strMessage += ": " + lTablesCleaned.join("; ");
        WriteDebugMessageFile(strMessage);
    }

    WriteDebugMessageFile("     GexDbPlugin_Galaxy::CleanInvalidSplitlot completed.");

    // GCORE-4202
    // Because some Temporary splitlot_id are not correctly removed after the insertion process
    // We need to clean the TDR for all old insertion
    // when the splitlot_id is > 101000000 and < 2000000000
    // and when the insertion_time is oldest than 1 day
    QString lTokenTable = NormalizeTableName("token", false);
    lQuery = "DELETE FROM " + NormalizeTableName("_SPLITLOT");
    lQuery += " WHERE (SPLITLOT_ID >= 101000000 AND SPLITLOT_ID < 200000000)";
    lQuery += " AND VALID_SPLITLOT='N' ";
    lQuery += " AND INSERTION_TIME<UNIX_TIMESTAMP(SUBDATE(now(),1))";
    lQuery += " AND (SPLITLOT_ID NOT IN (SELECT key_value FROM " + lTokenTable + " WHERE name='" + NormalizeTableName(
                "_INSERTION") + "'))";
    clGexDbQuery.Execute(lQuery);

}

//////////////////////////////////////////////////////////////////////
// IsTestFail
//////////////////////////////////////////////////////////////////////
// Check if the PTR test is fail
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsTestFail(const GQTL_STDF::Stdf_PTR_V4& clRecord, structTestInfo* pTest)
{

    // Check if test was executed
    if ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT4))
        return false;

    // Check if this test is marked 'fail' (bits 6 cleared, bit 7 set) ?
    if (((clRecord.m_b1TEST_FLG & STDF_MASK_BIT6) == 0) &&
            ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT7)))
        return true;

    // Check if this test is marked 'pass' (bits 6 cleared, bit 7 cleared) ?
    if (((clRecord.m_b1TEST_FLG & STDF_MASK_BIT6) == 0) &&
            ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT7) == 0))
        return false;

    // Check if this test result is valid
    if (IsTestResultInvalid(clRecord))
        return false;

    bool bFail = false;
    if (pTest->m_mapfLL.contains(MERGE_SITE))
    {
        // Low limit exists (manages Strict and not-strict limits)
        if (pTest->m_cFlags & FLAG_TESTINFO_LL_STRICT)
            bFail = (pTest->m_mapfLL[MERGE_SITE] < clRecord.m_r4RESULT) ? false : true;
        else
            bFail = (pTest->m_mapfLL[MERGE_SITE] <= clRecord.m_r4RESULT) ? false : true;
    }

    if (! bFail && (pTest->m_mapfHL.contains(MERGE_SITE)))
    {
        // High limit exists (manages Strict and not-strict limits)
        if (pTest->m_cFlags & FLAG_TESTINFO_HL_STRICT)
            bFail = (pTest->m_mapfHL[MERGE_SITE] > clRecord.m_r4RESULT) ? false : true;
        else
            bFail = (pTest->m_mapfHL[MERGE_SITE] >= clRecord.m_r4RESULT) ? false : true;
    }

    return bFail;  // Test shouldn't be considered fail!
}

//////////////////////////////////////////////////////////////////////
// IsTestFail
//////////////////////////////////////////////////////////////////////
// Check if the MPR test is fail
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsTestFail(const GQTL_STDF::Stdf_MPR_V4& clRecord,
                                    structTestInfo*  /*pTest*/)
{
    // Check if test was executed
    if ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT4))
        return false;

    // Check if this test is marked 'fail' (bits 6 cleared, bit 7 set) ?
    if (((clRecord.m_b1TEST_FLG & STDF_MASK_BIT6) == 0) &&
            ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT7)))
        return true;

    return false;

}

//////////////////////////////////////////////////////////////////////
// IsTestFail
//////////////////////////////////////////////////////////////////////
// Check if the FTR test is fail
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsTestFail(const GQTL_STDF::Stdf_FTR_V4& clRecord,
                                    structTestInfo*  /*pTest*/)
{

    // Check if test was executed
    if ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT4))
        return false;

    // Check if this test is marked 'fail' (bits 6 cleared, bit 7 set) ?
    if (((clRecord.m_b1TEST_FLG & STDF_MASK_BIT6) == 0) &&
            ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT7)))
        return true;

    return false;  // Test shouldn't be considered fail!
}

//////////////////////////////////////////////////////////////////////
// IsTestResultInvalid
//////////////////////////////////////////////////////////////////////
// Check if the PTR result is valid
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsTestResultInvalid(const GQTL_STDF::Stdf_PTR_V4& clRecord)
{

    // Check if test result is valid
    // Datalog test result for TestTime : it is not a "TEST" so BIT4=1 (test not executed) but has a valid result
    if (clRecord.m_b1TEST_FLG & STDF_MASK_BIT1)  // || (clRecord.m_b1TEST_FLG & STDF_MASK_BIT4))
        return true;

    // Check if test result is NaN
    if (clRecord.m_bRESULT_IsNAN)
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////
// IsTestResultInvalid
//////////////////////////////////////////////////////////////////////
// Check if the MPR result is valid
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsTestResultInvalid(const GQTL_STDF::Stdf_MPR_V4& clRecord)
{
    // Check if test result is valid
    if (clRecord.m_b1TEST_FLG & STDF_MASK_BIT4)
        return true;

    if ((clRecord.m_u2RTN_ICNT == 0) && (clRecord.m_u2RSLT_CNT == 0))
        return true;

    return false;

}

//////////////////////////////////////////////////////////////////////
// IsTestResultInvalid
//////////////////////////////////////////////////////////////////////
// Check if the FTR result is valid
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsTestResultInvalid(const GQTL_STDF::Stdf_FTR_V4&  /*clRecord*/)
{

    // Check if test result is valid
    // Datalog test result for TestTime : it is not a "TEST" so BIT4=1 (test not executed) but has a valid result
    //if(clRecord.m_b1TEST_FLG & STDF_MASK_BIT4)
    //        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////
// IsTestNotExecuted
//////////////////////////////////////////////////////////////////////
// Check if the PTR is executed
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsTestNotExecuted(const GQTL_STDF::Stdf_PTR_V4& clRecord)
{

    // Check if test is executed
    if ((clRecord.m_b1TEST_FLG & STDF_MASK_BIT4))
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////
// IsTestNotExecuted
//////////////////////////////////////////////////////////////////////
// Check if the MPR is executed
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsTestNotExecuted(const GQTL_STDF::Stdf_MPR_V4& clRecord)
{
    // Check if test is executed
    if (clRecord.m_b1TEST_FLG & STDF_MASK_BIT4)
        return true;

    return false;

}

//////////////////////////////////////////////////////////////////////
// IsTestNotExecuted
//////////////////////////////////////////////////////////////////////
// Check if the FTR result is valid
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::IsTestNotExecuted(const GQTL_STDF::Stdf_FTR_V4& clRecord)
{

    // Check if test is executed
    if (clRecord.m_b1TEST_FLG & STDF_MASK_BIT4)
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////
// AddInMultiInsertQuery
//////////////////////////////////////////////////////////////////////
// Add new insertion values in the current query
// And verify if the query is not at the limit of the MaxPacketSize
// Then execute this query and reset the current query
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::AddInMultiInsertQuery(const QString & strTableName, QString &strHeader, QString &strNewValue, QString &strAllValues)
{
    if (strAllValues.isEmpty())
        strAllValues = strHeader + strNewValue;
    else
    {
        if (m_pclDatabaseConnector->IsSQLiteDB())
            strAllValues += ";\n" + strHeader + strNewValue;
        else
            strAllValues += "," + strNewValue;
    }
    if (m_nMaxPacketSize < (int) (strHeader.length() + strAllValues.length() + (5 * strNewValue.length())))
    {
        // execute query
        if (! ExecuteMultiInsertQuery(strTableName, strAllValues))
            return false;

        strAllValues = "";
    }
    strNewValue = "";
    return true;
}

//////////////////////////////////////////////////////////////////////
// ExecuteMultiInsertQuery
//////////////////////////////////////////////////////////////////////
// Before execute multi insertion, have to verify if the query is not
// bigger than the max size defined by the mySql server (error)
// Execute multi insertion queries for MySql and Oracle
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ExecuteMultiInsertQuery(const QString& strTableName, const QString& strQuery)
{
    // Debug message
    WriteDebugMessageFile(QString("---- GexDbPlugin_Galaxy::ExecuteMultiInsertQuery into %1").arg(strTableName));

    QString strString;
    // Error management: truncate Query and SQL error message used in error
    // texts, because those can be huge
    QString strTruncatedQuery, strTruncatedSqlError;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Update list of updated tables
    m_clUpdatedTables.Add(strTableName);

    // for MySQL
    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        // have to verify if not bigger than the max_packet_size
        if ((int) strQuery.length() > m_nMaxPacketSize)
        {
            strTruncatedQuery = strQuery.left(150);
            strTruncatedQuery += " ...";
            if(m_pbDelayInsertion) *m_pbDelayInsertion = true;
            GSET_ERROR1(GexDbPlugin_Base, eDB_PacketSizeOverflow, NULL, strTruncatedQuery.toLatin1().constData());
            return false;
        }
        // = "INSERT INTO table VALUES(1,2,3),(4,5,6);"
        if (! clGexDbQuery.Execute(strQuery))
        {
            if (strQuery.length() >= 146)
            {
                strTruncatedQuery = strQuery.left(146);
                strTruncatedQuery += " ...";
            }
            else
                strTruncatedQuery = strQuery;
            strString = clGexDbQuery.lastError().text();
            if (strString.length() >= 146)
            {
                strTruncatedSqlError = strString.left(146);
                strTruncatedSqlError += " ...";
            }
            else
                strTruncatedSqlError = strString;
            if(m_pbDelayInsertion) *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strTruncatedQuery.toLatin1().constData(), strTruncatedSqlError.toLatin1().constData());
            return false;
        }
    }

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::ExecuteMultiInsertQuery completed");

    if (m_pclDatabaseConnector->IsSQLiteDB())
    {
        return ExecuteMultiInsertQuery_SQLite(strTableName, strQuery);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// InitLoadDataInfile
//////////////////////////////////////////////////////////////////////
// Prepare intermediaire values for Load Data Infile statement
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::InitLoadDataInfile(GQTL_STDF::Stdf_Record::RecordTypes nRecordType)
{
    QString strDataFileExt;
    QString strTableName, strTableParams;
    QString strFileName, strFileInit;
    bool bTestResultsByLoader;
    QFile* pFileResults;
    int* pNbResults;
    QString* pstrTestResults = NULL;

    // Check if test results should be inserted
    if (mpDbKeysEngine->dbKeysContent().Get("IgnoreResults").toString() == "TRUE")
        return true;

    // Check if the file is already initialized
    switch (nRecordType)
    {
    case GQTL_STDF::Stdf_Record::Rec_MPR:
        pFileResults = &m_clMPTestResultsDataFile;
        bTestResultsByLoader = m_bMPTestResultsBySqlLoader;
        break;

    case GQTL_STDF::Stdf_Record::Rec_FTR:
        pFileResults = &m_clFTestResultsDataFile;
        bTestResultsByLoader = m_bFTestResultsBySqlLoader;
        break;

    case GQTL_STDF::Stdf_Record::Rec_PTR:
    default:
        pFileResults = &m_clPTestResultsDataFile;
        bTestResultsByLoader = m_bPTestResultsBySqlLoader;
        break;
    }
    // Already initialized
    // DO NOT SKIP THE END OF THE CODE (MULTI METHOD ERROR)
    //if(bTestResultsByLoader && pFileResults->isOpen())
    //    return true;

    switch (nRecordType)
    {
    case GQTL_STDF::Stdf_Record::Rec_MPR:
        strTableName = NormalizeTableName("_MPTEST_RESULTS");
        strTableParams = m_strMPTestResultsTableColumns = "("+m_mapTablesDesc[QString(strTableName+".columns").toUpper()]+")";
        pFileResults = &m_clMPTestResultsDataFile;
        pNbResults = &m_nNbMPTestResults;
        bTestResultsByLoader = m_bMPTestResultsBySqlLoader;
        pstrTestResults = &m_strMPTestResults;
        break;

    case GQTL_STDF::Stdf_Record::Rec_FTR:
        strTableName = NormalizeTableName("_FTEST_RESULTS");
        strTableParams = m_strFTestResultsTableColumns = "("+m_mapTablesDesc[QString(strTableName+".columns").toUpper()]+")";
        pFileResults = &m_clFTestResultsDataFile;
        pNbResults = &m_nNbFTestResults;
        bTestResultsByLoader = m_bFTestResultsBySqlLoader;
        pstrTestResults = &m_strFTestResults;
        break;

    case GQTL_STDF::Stdf_Record::Rec_PTR:
    default:
        strTableName = NormalizeTableName("_PTEST_RESULTS");
        strTableParams = m_strPTestResultsTableColumns = "("+m_mapTablesDesc[QString(strTableName+".columns").toUpper()]+")";
        pFileResults = &m_clPTestResultsDataFile;
        pNbResults = &m_nNbPTestResults;
        bTestResultsByLoader = m_bPTestResultsBySqlLoader;
        pstrTestResults = &m_strPTestResults;
        break;
    }

    m_strLoadDataInfileDecSepValues = ".";
    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        if (bTestResultsByLoader)
        {
            strDataFileExt = ".dat";
            m_strLoadDataInfileNullValues = "\\N";
            m_strLoadDataInfileBeginValues = "";
            m_strLoadDataInfileSepValues = ";";
            m_strLoadDataInfileEndValues = ";";
        }
        else
        {
            m_strLoadDataInfileDecSepValues = ".";
            m_strLoadDataInfileNullValues = "null";
            if ((*pstrTestResults).isEmpty())
            {
                QString strQuery;
                if (! mAttributes["SQLLOADER_QUERY"].toString().isEmpty())
                {
                    // Add pre queries config
                    strQuery = mAttributes["SQLLOADER_QUERY"].toString().replace(";", ";\n");
                    if (! mAttributes["SQLLOADER_QUERY"].toString().endsWith(";"))
                        strQuery += ";\n";
                }
                strQuery += "INSERT INTO " + strTableName + " VALUES";
                (*pstrTestResults) = strQuery;
                m_strLoadDataInfileBeginValues = "(";
            }
            else
                m_strLoadDataInfileBeginValues = ",(";
            m_strLoadDataInfileSepValues = ",";
            m_strLoadDataInfileEndValues = ")";
        }
    }
    else if (m_pclDatabaseConnector->IsSQLiteDB())
    {
        m_strLoadDataInfileDecSepValues = ".";
        m_strLoadDataInfileNullValues = "null";
        m_strLoadDataInfileBeginValues = "INSERT INTO " + strTableName + " VALUES(";
        m_strLoadDataInfileSepValues = ",";
        m_strLoadDataInfileEndValues = ");";
    }

    if (bTestResultsByLoader && ! pFileResults->isOpen())
    {
        (*pNbResults) = 0;
        strFileName = NormalizeFileName(strTableName, strDataFileExt);
        QFile::remove(strFileName);

        pFileResults->setFileName(strFileName);
        if (pFileResults->open(QIODevice::ReadWrite) == false)
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR1(GexDbPlugin_Base, eDB_OpenSqlLoaderFile, NULL, strFileName.toLatin1().constData());
            return false;  // Failed opening temp file.
        }

        // For MySql LOAD DATA INFILE is a query
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// AddInLoadDataInfile
//////////////////////////////////////////////////////////////////////
// Add new values in the current data file
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::AddInLoadDataInfile(GQTL_STDF::Stdf_Record::RecordTypes nRecordType, QString& strAllValues)
{
    bool bTestResultsByLoader;
    QFile* pFileResults;

    // Check if test results should be inserted
    if (mpDbKeysEngine->dbKeysContent().Get("IgnoreResults").toString() == "TRUE")
    {
        // Reset Values saved
        strAllValues = "";
        return true;
    }

    switch (nRecordType)
    {
    case GQTL_STDF::Stdf_Record::Rec_MPR:
        pFileResults = &m_clMPTestResultsDataFile;
        bTestResultsByLoader = m_bMPTestResultsBySqlLoader;
        break;

    case GQTL_STDF::Stdf_Record::Rec_FTR:
        pFileResults = &m_clFTestResultsDataFile;
        bTestResultsByLoader = m_bFTestResultsBySqlLoader;
        break;

    case GQTL_STDF::Stdf_Record::Rec_PTR:
    default:
        pFileResults = &m_clPTestResultsDataFile;
        bTestResultsByLoader = m_bPTestResultsBySqlLoader;
        break;
    }

    if (bTestResultsByLoader)
    {
        TimersTraceStart("LoadDataInFile FileWrite");
        if (! pFileResults->isOpen())  // 6390
            if (! pFileResults->open(QIODevice::WriteOnly))
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Cannot open %1 for writing")
                      .arg(pFileResults->fileName()).toLatin1().constData());
                return false;
            }
        QTextStream clFile(pFileResults);
        clFile << strAllValues;
        strAllValues = "";
        TimersTraceStop("LoadDataInFile FileWrite");
    }
    else
    {
        // have to verify if not bigger than the max_packet_size
        // Detecte before the size of next result
        int nLength = strAllValues.length();
        if (nLength > m_nMaxPacketSize)
        {
            // = "INSERT INTO table VALUES(1,2,3),(4,5,6);"
            // Check if the buffer is not already bigger
            // Set it to the value just after in MB
            int nCutAt = strAllValues.left(m_nMaxPacketSize).lastIndexOf(")\n,(") + 1;
            QString strQuery = strAllValues.left(nCutAt);
            strAllValues = strQuery.section("VALUES", 0, 0) + "VALUES(" + strAllValues.right(nLength - nCutAt - 3);
            GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
            if (! clGexDbQuery.Execute(strQuery))
            {
                if(m_pbDelayInsertion) *m_pbDelayInsertion = true;
                GSLOG(SYSLOG_SEV_ERROR, QString("Failed to exec query : %1")
                      .arg(strQuery.left(256)).toLatin1().constData());
                GSLOG(SYSLOG_SEV_ERROR, QString("Query size : %1")
                      .arg(strQuery.length()).toLatin1().constData());
                GSLOG(SYSLOG_SEV_ERROR, QString("Query error : %1")
                      .arg(clGexDbQuery.lastError().text()).toLatin1().constData());
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(256).toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                strAllValues = "";
                return false;
            }
            // InitLoad automatically call after
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// ExecuteLoadDataInfile
//////////////////////////////////////////////////////////////////////
// Have create control file for statement LOAD DATA INFILE
// For MySql, execute Query statement LOAD DATA INFILE
// For Oracle, execute program Sql Loader
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ExecuteLoadDataInfile(GQTL_STDF::Stdf_Record::RecordTypes nRecordType)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("ExecuteLoadDataInfile record type %1...")
          .arg(nRecordType).toLatin1().constData());
    QString strTableName, strTableParams;
    QString strFileName;
    bool bTestResultsByLoader;
    QString* pstrTestResults = NULL;
    QFile* pFileResults = NULL;
    int* pNbResults = NULL;

    switch (nRecordType)
    {
    case GQTL_STDF::Stdf_Record::Rec_MPR:
        strTableName = NormalizeTableName("_MPTEST_RESULTS");
        strTableParams = m_strMPTestResultsTableColumns;
        pstrTestResults = &m_strMPTestResults;
        pFileResults = &m_clMPTestResultsDataFile;
        pNbResults = &m_nNbMPTestResults;
        bTestResultsByLoader = m_bMPTestResultsBySqlLoader;
        break;

    case GQTL_STDF::Stdf_Record::Rec_FTR:
        strTableName = NormalizeTableName("_FTEST_RESULTS");
        strTableParams = m_strFTestResultsTableColumns;
        pstrTestResults = &m_strFTestResults;
        pFileResults = &m_clFTestResultsDataFile;
        pNbResults = &m_nNbFTestResults;
        bTestResultsByLoader = m_bFTestResultsBySqlLoader;
        break;

    case GQTL_STDF::Stdf_Record::Rec_PTR:
    default:
        strTableName = NormalizeTableName("_PTEST_RESULTS");
        strTableParams = m_strPTestResultsTableColumns;
        pstrTestResults = &m_strPTestResults;
        pFileResults = &m_clPTestResultsDataFile;
        pNbResults = &m_nNbPTestResults;
        bTestResultsByLoader = m_bPTestResultsBySqlLoader;
        break;
    }

    GSLOG(SYSLOG_SEV_DEBUG, (QString("Execute Load Data Infile : %1 results into table %2 using SQLLoader=%3...")
                             .arg(*pNbResults)
                             .arg(strTableName.toLatin1().data())
                             .arg(bTestResultsByLoader ? "true" : "false")).toLatin1().constData());
    // Check if test results should be inserted
    if (mpDbKeysEngine->dbKeysContent().Get("IgnoreResults").toString() == "TRUE")
    {
        *pstrTestResults = "";
        *pNbResults = 0;
        return true;
    }

    if ((*pNbResults) == 0)
        return true;

    // if multi wafer file
    // execute all SqlLoader insertion only at the end of all parse
    if ((bTestResultsByLoader)
            && (m_nTotalWaferToProcess > 1)
            && (m_nTotalWaferToProcess != m_nWaferIndexToProcess))
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Multiple wafers (%1), insertion postponed...")
              .arg(m_nTotalWaferToProcess).toLatin1().constData());
        return true;
    }

    // Update list of updated tables
    m_clUpdatedTables.Add(strTableName);

    QString strMessage = "ExecuteLoadDataInfile : ";
    strMessage += "Table=" + strTableName + " - ";
    if (bTestResultsByLoader)
        strMessage += "Method=SQL*LOADER .";
    else
        strMessage += "Method=SQL*QUERY .";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data());

    // for MySQL
    // Use the query LOAD DATA INFILE
    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        TimersTraceStart("LoadDataInFile Execution");
        if (bTestResultsByLoader)
        {
            pFileResults->close();
            strFileName = pFileResults->fileName();
            GSLOG(SYSLOG_SEV_NOTICE, QString("Insertion by MySQL loader of %1 Ko...")
                  .arg(pFileResults->size() / 1024).toLatin1().constData());

            if (mAttributes["SQLLOADER_OPTIONS_MODE"].toString().isEmpty()
                    || mAttributes["SQLLOADER_OPTIONS_MODE"].toString() != "DELAY")
            {
                QString strQuery;
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!! YIELDMAN SERVER CRASH WORKAROUND !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                // HAVE TO CREATE A NEW CONNECTION TO USE THE  SQL*LOADER
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!! YIELDMAN SERVER CRASH WORKAROUND !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                bool success = true;
                QString error = "";
                QString lConnection = m_pclDatabaseConnector->m_strConnectionName + "_SQLLOADER";
                // { } To keep QSqlQuery out of removeDatabase scope
                {
                    QCoreApplication::processEvents();

                    QSqlDatabase dbTest;
                    if (! mAttributes["SQLLOADER_THREAD_OPTION"].toString().isEmpty())
                    {
                        if (mAttributes["SQLLOADER_THREAD_OPTION"].toString() == "TDR")
                        {
                            // Use the thread from the TDR connector
                            lConnection = m_pclDatabaseConnector->m_strConnectionName;
                        }
                        else if (mAttributes["SQLLOADER_THREAD_OPTION"].toString() == "LOADER")
                        {
                            // Create a thread for the LOADER
                            lConnection = m_pclDatabaseConnector->m_strConnectionName + "_SQLLOADER";
                        }
                        else if (mAttributes["SQLLOADER_THREAD_OPTION"].toString() == "EXEC")
                        {
                            // Create a thread for each exec
                            lConnection = m_pclDatabaseConnector->m_strConnectionName + "_" + strFileName;
                        }
                    }

                    // Add database link: make sure we use a unique name for the database connection
                    if (! QSqlDatabase::contains(lConnection))
                    {
                        // On some MySql server with AUTO connection configuration
                        // The addDatabase can directly open a default connection (see at ASE, not reproducible ?)
                        // QSqlDatabase::addDatabase(m_pclDatabaseConnector->m_strDriver, lConnection);
                        dbTest = QSqlDatabase::cloneDatabase(
                                    QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName, false),
                                    lConnection);
                        GSLOG(SYSLOG_SEV_WARNING, QString("Create a new SQLLOADER connection - %1").arg(lConnection).toLatin1().constData());
                    }
                    else
                        dbTest = QSqlDatabase::database(lConnection);

                    if (! dbTest.isOpen())
                    {
                        dbTest.open();
                        GSLOG(SYSLOG_SEV_WARNING, QString("Open the SQLLOADER connection - %1").arg(lConnection).toLatin1().constData());
                    }

                    QSqlQuery clQuery(dbTest);
                    if (! mAttributes["SQLLOADER_QUERY"].toString().isEmpty())
                    {
                        // Add pre queries config
                        strQuery = mAttributes["SQLLOADER_QUERY"].toString().replace(";", ";\n");
                        if (! mAttributes["SQLLOADER_QUERY"].toString().endsWith(";"))
                            strQuery += ";\n";

                        GSLOG(SYSLOG_SEV_DEBUG, QString("Execute Load Data infile with pre-queries config %1")
                              .arg(strQuery).toLatin1().constData());
                    }

                    // USE LOCAL ONLY IF IT IS NECESSARY
                    // without IGNORE or REPLACE option, all duplicated data generates an ERROR
                    // with the LOCAL option, no ERROR but WARNING (like IGNORE option)
                    // SHOW WARNINGS indicates any invalid or duplicated data
                    QString lLocal = "";
                    //if(!m_pclDatabaseConnector->m_bIsLocalHost)
                    // Always use LOCAL to fix insertion access denied on WINDOWS
                    lLocal = "LOCAL";
#if defined __APPLE__ & __MACH__
                    // On Mac, LOCAL is not supported if the DB is local.
                    // Only use LOCAL if the DB is remote (and the file local)
                    if (m_pclDatabaseConnector->m_bIsLocalHost)
                        lLocal = "";
#endif
                    if (! mAttributes["SQLLOADER_LOCAL_OPTION"].toString().isEmpty())
                    {
                        if (mAttributes["SQLLOADER_LOCAL_OPTION"].toString() == "FORCE")
                            lLocal = "LOCAL";
                        else if (mAttributes["SQLLOADER_LOCAL_OPTION"].toString() == "AUTO")
                            lLocal = (m_pclDatabaseConnector->m_bIsLocalHost ? "" : "LOCAL");
                    }

                    strQuery += "LOAD DATA " + lLocal + " INFILE '" + strFileName + "'\n";
                    strQuery += "INTO TABLE " + strTableName + "\n";
                    strQuery += "FIELDS TERMINATED BY \";\"\n";
                    strQuery += "OPTIONALLY ENCLOSED BY \"'\"\n";
                    strQuery += "ESCAPED BY \"\\\\\"\n";
                    strQuery += strTableParams + "\n";
                    GSLOG(SYSLOG_SEV_DEBUG, QString("Execute Load Data infile %1")
                          .arg(strFileName).toLatin1().constData());
                    // = "LOAD DATA INFILE"
                    if (! dbTest.isOpen())
                    {
                        success = false;
                        error = "Unable to open new connection for SQL*LOADER!";
                    }
                    else if (! clQuery.exec(strQuery))
                    {
                        success = false;
                        error = clQuery.lastError().text();
                    }
                    else
                    {
                        // No error
                        // Check if have some Warnings
                        clQuery.exec("SHOW WARNINGS");
                        if (clQuery.first())
                        {
                            success = false;
                            error = clQuery.value("Message").toString();
                        }
                    }

                    if (! success)
                    {
                        if (error.isEmpty())
                            error = "Unknown error";
                        GSLOG(SYSLOG_SEV_ERROR, QString("Query exec failed : %1")
                              .arg(error).toLatin1().constData());
                        QString strLogFile = strFileName.left(strFileName.length() - 4) + ".log";
                        QFile clFile(strLogFile);
                        // append du nom dans le fichier log
                        bool b = clFile.open(QIODevice::WriteOnly | QIODevice::Append);
                        if (! b)
                            GSLOG(SYSLOG_SEV_WARNING, QString("Cannot open %1").arg(strLogFile).toLatin1().constData());
                        QTextStream clTextFile(&clFile);
                        clTextFile << endl << endl;
                        clTextFile << "* Stdf File = " << m_strStdfFile << endl;
                        clTextFile << "* File Size = " << m_nFileSize << endl;
                        clTextFile << "* Data File = " << strFileName << endl;
                        clTextFile << "* Data Rows = " << QString::number((int) (*pNbResults)) << endl;
                        clTextFile << "* Sql Load Query = " << strQuery << endl;
                        clTextFile << "* Error Message = " << error << endl;
                        clFile.close();

                        *m_pbDelayInsertion = true;
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(256).toLatin1().constData(),
                                    error.toLatin1().constData());
                        success = false;
                    }
                    QCoreApplication::processEvents();
                }

                if (mAttributes["SQLLOADER_THREAD_OPTION"].toString().isEmpty()
                        || (mAttributes["SQLLOADER_THREAD_OPTION"].toString() == "EXEC"))
                {
                    // Delete a thread for each exec
                    QSqlDatabase::removeDatabase(lConnection);
                }

                if (! success)
                {
                    return false;
                }
            }

            // Cleanup and return
            if (! mAttributes["SQLLOADER_DATAFILES_MOVE"].toString().isEmpty())
            {
                // Try to move the file in other location
                if(!QFile::rename(strFileName,mAttributes["SQLLOADER_DATAFILES_MOVE"].toString()+QFileInfo(strFileName).fileName()))
                {
                    GSLOG(SYSLOG_SEV_WARNING, QString("Cannot rename %1 to %2")
                          .arg(strFileName)
                          .arg(mAttributes["SQLLOADER_DATAFILES_MOVE"].toString() + QFileInfo(strFileName).fileName())
                            .toLatin1().constData());
                    // else remove
                    QFile::remove(strFileName);
                }
            }
            else
            {
                bool b = QFile::remove(strFileName);
                if (! b)
                    GSLOG(SYSLOG_SEV_WARNING, QString("Cannot remove %1").arg(strFileName).toLatin1().constData());
            }
        }
        else
        {
            QString strQuery;
            GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
            strQuery = (*pstrTestResults);
            GSLOG(SYSLOG_SEV_DEBUG, QString("Not using LOAD DATA INFILE but usual queries : query size = %1 chars")
                  .arg(strQuery.size()).toLatin1().constData());
            if (! clGexDbQuery.Execute(strQuery))
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Failed to exec query : %1")
                      .arg(clGexDbQuery.lastError().text()).toLatin1().constData());
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(256).toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }
        TimersTraceStop("LoadDataInFile Execution");

        QString strMessage = "ExecuteLoadDataInfile done.";
        GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data());

        (*pstrTestResults) = "";
        (*pNbResults) = 0;
        return true;
    }

    if (m_pclDatabaseConnector->IsSQLiteDB())
    {
        return ExecuteLoadDataInfile_SQLite(pstrTestResults, pNbResults);
    }

    GSLOG(SYSLOG_SEV_ERROR, QString("ExecuteLoadDataInfile : unsupported SQL driver : %1").arg(
              m_pclDatabaseConnector->m_strDriver).toLatin1().constData());
    *m_pbDelayInsertion = true;
    GSET_ERROR1(GexDbPlugin_Base, eDB_UnsupportedDriver, NULL, m_pclDatabaseConnector->m_strDriver.toLatin1().constData());
    return false;
}


///////////////////////////////////////////////////////////
// LockTables
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::LockTables(QString strTableName, bool bRowMode)
{
    QString strQuery;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    TimersTraceStart("LockTable on " + strTableName);

    clGexDbQuery.Execute("COMMIT");
    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        if (m_pclDatabaseConnector->m_bTransactionDb && bRowMode)
            strQuery = "LOCK TABLES " + strTableName + " READ LOCAL";
        else
            strQuery = "LOCK TABLES " + strTableName + " WRITE";
    }

    clGexDbQuery.Execute(strQuery);

    TimersTraceStop("LockTable on " + strTableName);

    return true;
}

///////////////////////////////////////////////////////////
//UnlockTables
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UnlockTables()
{
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // The same for Oracle and Mysql
    if (m_pclDatabaseConnector->m_bTransactionDb)
        clGexDbQuery.Execute("COMMIT");

    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        //#
        //SET autocommit=0;
        //LOCK TABLES t1 WRITE, t2 READ, ...;
        //... do something with tables t1 and t2 here ...
        //COMMIT;
        //UNLOCK TABLES;
        //When you call LOCK TABLES, InnoDB internally takes its own table lock,
        //and MySQL takes its own table lock. InnoDB releases its internal table lock at the next commit,
        //but for MySQL to release its table lock, you have to call UNLOCK TABLES.
        //You should not have autocommit = 1,
        //because then InnoDB releases its internal table lock immediately after the call of LOCK TABLES,
        //and deadlocks can very easily happen.
        //InnoDB does not acquire the internal table lock at all if autocommit = 1,
        //to help old applications avoid unnecessary deadlocks.

        clGexDbQuery.Execute("UNLOCK TABLES");
    }

    // Debug message
    WriteDebugMessageFile(QString("---- GexDbPlugin_Galaxy::UnlockTables()"));

    return true;
}

bool GexDbPlugin_Galaxy::checkMetaDataForTestConditions()
{
    // Get the test conditions list
    QStringList lTestConditions;
    structTestInfo* pTest = 0;

    if (mpDbKeysEngine->GetTestConditionsOrigin() == GS::QtLib::DatakeysEngine::DTR)
    {
        int  lTestInfoIdx;
        for ( lTestInfoIdx = 0; lTestInfoIdx < m_lstTestInfo.size(); ++lTestInfoIdx)
        {
            pTest = m_lstTestInfo.at(lTestInfoIdx);

            lTestConditions.append(pTest->mTestConditions.keys());
        }
        lTestConditions.removeDuplicates();
    }
    else
        lTestConditions = mpDbKeysEngine->dbKeysContent().testConditions().keys();
    QStringList newTestConditions;

    foreach(const QString &lKey, lTestConditions)
    {
        // Test conditions missing from meta-data
        // We need to declare it into the test_conditions table
        if (m_pmapFields_GexToRemote->contains(lKey) == false)
            newTestConditions.append(lKey);
    }

    // New test conditions to declare
    if (newTestConditions.isEmpty() == false)
    {
        // Query Test condition table
        GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
        clGexDbQuery.setForwardOnly(true);

        // Query mapping for Test Conditions
        QString sqlTableName = QString("%1test_conditions").arg(m_strTablePrefix);
        QString strQuery;
        QMap<QString, QString> conditionFieldMap;

        strQuery = "SELECT * \n";
        strQuery += "FROM " + sqlTableName + " \n";
        strQuery += "WHERE splitlot_id = 0 and test_info_id = 0 and test_type = '-'";

        // HTH-TOCHECK
        if (! clGexDbQuery.Execute(strQuery))
            return false;

        // Read extracted Test conditions
        if (clGexDbQuery.first())
        {
            QString fieldName;
            int fieldIndex = -1;

            for (int idxField = 1; idxField <= GEXDB_PLUGIN_GALAXY_TEST_CONDITIONS_COLUMNS && !newTestConditions.isEmpty(); ++idxField)
            {
                fieldName = "condition_" + QString::number(idxField);
                fieldIndex = clGexDbQuery.record().indexOf(fieldName);

                if (fieldIndex != -1)
                {
                    if (clGexDbQuery.value(fieldIndex).isNull())
                        conditionFieldMap.insert(fieldName, newTestConditions.takeFirst());
                }
            }

            // HTH-TOCHECK
            if (conditionFieldMap.isEmpty() == false)
            {
                // Update test_condition table
                strQuery = "UPDATE " + sqlTableName + " \n";
                strQuery += "SET \n";

                QString fieldToSet;
                foreach(const QString &lKey, conditionFieldMap.keys())
                {
                    if (fieldToSet.isEmpty() == false)
                        fieldToSet += ",\n";

                    fieldToSet += lKey + "=";
                    fieldToSet += m_pclDatabaseConnector->TranslateStringToSqlVarChar(conditionFieldMap.value(lKey));
                }

                strQuery += fieldToSet + "\n";
                strQuery += "WHERE splitlot_id = 0 and test_info_id = 0 and test_type = '-'";

                // HTH-TOCHECK
                if (! clGexDbQuery.Execute(strQuery))
                    return false;
            }
        }
        else
        {
            // No test condition definition line existing, we have to create it
            QString field;
            QString value;
            int columnIdx = 1;

            // Insert into test_condition table
            strQuery = "INSERT INTO " + sqlTableName + " \n";

            QString fieldsToSet = "(splitlot_id, test_info_id, test_type";
            QString valuesToSet = "(0,0,'-'";

            while (newTestConditions.isEmpty() == false)
            {
                field = "condition_" + QString::number(columnIdx);
                value = newTestConditions.takeFirst();

                conditionFieldMap.insert(field, value);

                fieldsToSet += "," + field;
                valuesToSet += "," + m_pclDatabaseConnector->TranslateStringToSqlVarChar(value);

                ++columnIdx;
            }

            fieldsToSet += ")";
            valuesToSet += ")";

            strQuery += fieldsToSet + "\n";
            strQuery += "values " + valuesToSet + "\n";

            // HTH-TOCHECK
            if (! clGexDbQuery.Execute(strQuery))
                return false;
        }

        // Create new meta-data
        GexDbPlugin_Mapping_Field tcMetadataField;
        QString metadataName;
        QString fullFieldName;
        QString linkName = QString("%1test_conditions-%2splitlot").arg(m_strTablePrefix).arg(m_strTablePrefix);

        foreach(const QString &lKey, conditionFieldMap.keys())
        {
            metadataName = conditionFieldMap.value(lKey);
            fullFieldName = sqlTableName + "." + lKey;

            tcMetadataField = GexDbPlugin_Mapping_Field(metadataName, "",
                                                        sqlTableName, fullFieldName,
                                                        linkName,
                                                        true, "N", false, false, false,
                                                        false, false, false, false);

            tcMetadataField.setTestCondition(true);

            // Insert new metadata field into the map
            m_pmapFields_GexToRemote->insert(metadataName, tcMetadataField);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// QString GexDbPlugin_Galaxy::getTestConditionColumn(const QString &testConditionName) const
//
// Retrieve the name of the sql column for a given test condition
// Return an empty string if the condition has no colmumn associated
///////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::getTestConditionColumn(const QString& testConditionName) const
{
    QString columnName;

    if (m_pmapFields_GexToRemote->contains(testConditionName))
        columnName = m_pmapFields_GexToRemote->value(testConditionName).getSqlFieldName();

    return columnName;
}

///////////////////////////////////////////////////////////
// GetLock Session
///////////////////////////////////////////////////////////
// Tries to obtain a lock with a name given by the string str,
// using a timeout of timeout seconds.
// Returns 1 if the lock was obtained successfully,
// 0 if the attempt timed out (for example, because another client has previously locked the name),
// or NULL if an error occurred
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetLock(QString strLockName)
{
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    bool bStatus = false;
    TimersTraceStart("GetLock on " + strLockName);

    // GET_LOCK automaticaly release all other locked sessions in this process
    // If the lock already obtains by this process, the command has no effect
    if (clGexDbQuery.Execute("SELECT GET_LOCK(\"" + strLockName + "\",600)"))
    {
        clGexDbQuery.First();
        if (clGexDbQuery.value(0).toInt() == 1)
        {
            // Obtained the lock
            m_strCurrentLockSession = strLockName;
            bStatus = true;
        }
    }

    // Debug message
    WriteDebugMessageFile(QString("---- GexDbPlugin_Galaxy::GetLock(%1)").arg(m_strCurrentLockSession));

    TimersTraceStop("GetLock on " + strLockName);

    return bStatus;
}

///////////////////////////////////////////////////////////
//Release Lock
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ReleaseLock(bool bForce)
{
    if (m_strCurrentLockSession.isEmpty() && ! bForce)
        return true;

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    if (bForce)
    {
        // Get a dummy lock to release all other locks
        // Then release it
        GetLock("DummyLock" + QString::number(m_nInsertionSID));
        ReleaseLock();

        return true;
    }

    // Debug message
    WriteDebugMessageFile(QString("---- GexDbPlugin_Galaxy::ReleaseLock(%1)").arg(m_strCurrentLockSession));

    TimersTraceStart("GetLock Release");

    if (clGexDbQuery.Execute("SELECT RELEASE_LOCK(\"" + m_strCurrentLockSession + "\")")
            && clGexDbQuery.First() && (clGexDbQuery.value(0).isNull() || clGexDbQuery.value(0).toInt() == 1))
        m_strCurrentLockSession = "";

    TimersTraceStop("GetLock Release");

    return true;
}

///////////////////////////////////////////////////////////
// Get Token
// Time in secondes
///////////////////////////////////////////////////////////
// Time=-1: exit if GetToken fails
// Time=MaxTime: Try to GetToken and exit if End of Time
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetToken(QString Name, QString Key, int Time)
{
    QString strQuery, strQueryCheck;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    QString lTokenTable = NormalizeTableName("token", false);
    QTime qTime;
    QTime qTimeOut;
    qTimeOut.start();
    // Try GetToken unique KEY
    QString lDate = "SYSDATE";
    if (m_pclDatabaseConnector->IsMySqlDB())
        lDate = "now()";
    strQuery = "INSERT INTO " + lTokenTable + " Values(" + lDate + ",";
    strQuery += TranslateStringToSqlVarChar(Name.left(255)) + ",";
    strQuery += TranslateStringToSqlVarChar(Key.left(512)) + ",";
    strQuery += "'" + QString::number(m_nInsertionSID) + ":" + m_strInsertionHostName + "')";

    strQueryCheck = "SELECT * FROM " + lTokenTable + " WHERE name=";
    strQueryCheck += TranslateStringToSqlVarChar(Name.left(255)) + " AND key_value=";
    strQueryCheck += TranslateStringToSqlVarChar(Key.left(512)) + " AND session_id=";
    strQueryCheck += "'" + QString::number(m_nInsertionSID) + ":" + m_strInsertionHostName + "'";

    // Check if I already have the token
    if (clGexDbQuery.Execute(strQueryCheck) && clGexDbQuery.First())
        return true;

    // Don't use Execute
    // We don't want to log issue on DUPLICATE KEYS
    // before to retry and have the Lock
    while (! clGexDbQuery.exec(strQuery))
    {
        if (m_pclDatabaseConnector->IsMySqlDB())
        {
            // MySql
            // Error: 1022 SQLSTATE: 23000 (ER_DUP_KEY)
            // Message: Can't write; duplicate key in table '%s'
            // Error: 1062 SQLSTATE: 23000 (ER_DUP_ENTRY)
            // Message: Duplicate entry '%s' for key %d
            if ((clGexDbQuery.lastError().number() != 1022)
                    && (clGexDbQuery.lastError().number() != 1062))
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("%1").arg(clGexDbQuery.lastError().text()).toLatin1().constData());
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }

        // Check if it is ME
        if (clGexDbQuery.Execute(strQueryCheck) && clGexDbQuery.First())
            return true;

        // If no wait
        if (Time < 0)
        {
            QString lError = QString("Another thread already starts process on TestingStage=%1 Lot=%2 %3 (your SID=%4)");
            QString lSubLot = "";
            if(m_strWaferId.isEmpty())
            {
                lSubLot = "Sublot=" + m_strSubLotId;
            }
            else
            {
                lSubLot = "Wafer=" + m_strWaferId;
            }
            lError = lError.arg(m_strTestingStage).arg(m_strLotId).arg(lSubLot).arg(m_nInsertionSID);;
            GSET_ERROR1(GexDbPlugin_Base, eDB_InvalidTransaction, NULL,
                        lError.toLatin1().constData());
            return false;
        }
        // If Time out
        if ((qTimeOut.elapsed() / 1000) > Time)
        {
            QString lError = QString("Another thread already starts process on TestingStage=%1 Lot=%2 %3 (since more than %4ms) (your SID=%5)");;
            QString lSubLot = "";
            if(m_strWaferId.isEmpty())
            {
                lSubLot = "Sublot=" + m_strSubLotId;
            }
            else
            {
                lSubLot = "Wafer=" + m_strWaferId;
            }
            lError = lError.arg(m_strTestingStage).arg(m_strLotId).arg(lSubLot).arg(qTimeOut.elapsed()).arg(m_nInsertionSID);;
            GSET_ERROR1(GexDbPlugin_Base, eDB_InvalidTransaction, NULL,
                        lError.toLatin1().constData());
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("TIMEOUT: GetToken WAIT=%1ms")
                  .arg(qTimeOut.elapsed()).toLatin1().constData());
            return false;
        }

        // Wait
        qTime.start();
        while (qTime.elapsed() < 500)
        {
            ProcessEvents();
        }
    }

    if (qTimeOut.elapsed() > 500)
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("DONE: GetToken WAIT=%1ms")
              .arg(qTimeOut.elapsed()).toLatin1().constData());
    return true;
}

void GexDbPlugin_Galaxy::ReleaseToken(QString Name, QString Key)
{
    QString strQuery;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    QString lTokenTable = NormalizeTableName("token", false);
    // Clean token table
    strQuery = "DELETE FROM " + lTokenTable + " WHERE"
                                              " session_id='" + QString::number(m_nInsertionSID) + ":" + m_strInsertionHostName + "' AND"
                                                                                                                                  " name=" + TranslateStringToSqlVarChar(Name.left(255)) + " AND"
                                                                                                                                                                                           " key_value=" + TranslateStringToSqlVarChar(Key.left(512));
    if (! clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
    }
}

// Name allows to focus only on specific TestingStage needed
// Used for Consolidation process
bool GexDbPlugin_Galaxy::InitTokens(QString Name)
{
    QString lQuery;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    if (! m_pclDatabaseConnector->IsMySqlDB())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("%1").arg(clGexDbQuery.lastError().text()).toLatin1().constData());
        QString lDriver;
        m_pclDatabaseConnector->GetDriverName(lDriver);
        GSET_ERROR1(GexDbPlugin_Base, eDB_UnsupportedDriver, NULL, lDriver.toLatin1().constData());
        return false;
    }

    bool lHaveEmptyHostName = false;
    if(m_strInsertionHostName.isEmpty())
        m_strInsertionHostName = m_pclDatabaseConnector->GetConnectionHostName();

    // Do not reject the insertion if the query fails
    // This is only for cleaning the token table
    // and remove EXPIRED session_id
    // Just trace the error

    QString lTokenTable = NormalizeTableName("token", false);
    QString lConnection;
    QString lHostName;
    QString lOtherHostName;
    QStringList lHostNames;
    lQuery = "SELECT DISTINCT MID(session_id,INSTR(session_id,':')+1) AS OtherHostName, '" + m_strInsertionHostName + "' FROM " +
            lTokenTable;
    if(!Name.isEmpty())
    {
        // Focus on a specific use case
        // ex: name=ft_consolidation
        lQuery += " WHERE name=" + TranslateStringToSqlVarChar(Name.left(255));
    }
    if (! clGexDbQuery.Execute(lQuery))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("%1").arg(clGexDbQuery.lastError().text()).toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        return true;
    }
    while (clGexDbQuery.Next())
    {
        // If there is some insertion, token table contains some lines
        // Extract the HostName for each line
        // and check if the session is ALIVE
        // the HostName is UNIQUE in a standard environment
        // For a SPIDER environment, can be Comp01 and Comp02
        lOtherHostName = clGexDbQuery.value(0).toString();
        // The HostName cannot be empty
        if (lOtherHostName.isEmpty())
        {
            lHaveEmptyHostName = true;
            continue;
        }
        lHostName = clGexDbQuery.value(1).toString();
        if (! lHostNames.contains(lOtherHostName))
            lHostNames << lOtherHostName;
    }
    foreach(const QString &lKey, lHostNames)
    {
        // The HostName cannot be empty
        if (lKey.isEmpty())
            continue;
        lConnection = m_pclDatabaseConnector->m_strConnectionName;
        if (lHostName != lKey)
        {
            // Connect to this IP and check
            lConnection = m_pclDatabaseConnector->m_strConnectionName;
            lConnection += " on ";
            lConnection += lKey;

            // Add database link: make sure we use a unique name for the database connection
            // On some MySql server with AUTO connection configuration
            // The addDatabase can directly open a default connection (see at ASE, not reproducible ?)
            QSqlDatabase clSqlDatabase;
            if (! QSqlDatabase::contains(lConnection))
            {
                clSqlDatabase = QSqlDatabase::addDatabase(m_pclDatabaseConnector->m_strDriver, lConnection);
                // Force to close the connection if AUTO connection
                if (clSqlDatabase.isOpen())
                    clSqlDatabase.close();
                clSqlDatabase.setHostName(lKey);
                clSqlDatabase.setDatabaseName(m_pclDatabaseConnector->m_strDatabaseName);
                clSqlDatabase.setUserName(m_pclDatabaseConnector->m_strUserName_Admin);
                clSqlDatabase.setPassword(m_pclDatabaseConnector->m_strPassword_Admin);
                clSqlDatabase.setPort(m_pclDatabaseConnector->m_uiPort);
                GSLOG(SYSLOG_SEV_WARNING, QString("Create a new connection to clean EXPIRED session_id on %1 for %2: db[%3]user[%4]port[%5]")
                      .arg(lHostName).arg(lKey).arg(m_pclDatabaseConnector->m_strDatabaseName)
                      .arg(m_pclDatabaseConnector->m_strUserName_Admin)
                      .arg(m_pclDatabaseConnector->m_uiPort).toLatin1().constData());
            }
            else
                clSqlDatabase = QSqlDatabase::database(lConnection);

            if (! clSqlDatabase.isOpen())
            {
                clSqlDatabase.open();
                GSLOG(SYSLOG_SEV_WARNING, QString("Open the connection to clean EXPIRED session_id on %1 for %2: db[%3]user[%4]port[%5]")
                      .arg(lHostName).arg(lKey).arg(m_pclDatabaseConnector->m_strDatabaseName)
                      .arg(m_pclDatabaseConnector->m_strUserName_Admin)
                      .arg(m_pclDatabaseConnector->m_uiPort).toLatin1().constData());

            }
        }
        GexDbPlugin_Query lTokenQuery(this, QSqlDatabase::database(lConnection));
        lQuery = "DELETE FROM " + lTokenTable + " WHERE ";
        lQuery += "session_id NOT IN (";
        lQuery += "SELECT  CONCAT(ID,':','" + lKey + "')";
        lQuery += " FROM information_schema.PROCESSLIST";
        lQuery += ") ";
        lQuery += "AND session_id LIKE '%" + lKey + "' ";
        if(!Name.isEmpty())
        {
            // Focus on a specific use case
            lQuery += " AND name=" + TranslateStringToSqlVarChar(Name.left(255));
        }
        else
        {
            // To avoid some inexpected quarantine
            // Focus only on actions oldest than 1h
            lQuery += "AND start_time<DATE_SUB(now(),INTERVAL 1 hour)";
        }
        if (! lTokenQuery.Execute(lQuery))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Unable to clean the EXPIRED session_id - ignore this step: %1").arg( lTokenQuery.lastError().text()).toLatin1().constData());
            // Clean the connection if unable to open it
            if (lConnection != m_pclDatabaseConnector->m_strConnectionName)
                QSqlDatabase::removeDatabase(lConnection);
        }
    }

    if(lHaveEmptyHostName)
    {
        // GCORE-14433 Consolidation hangs.
        lQuery = "DELETE FROM " + lTokenTable + " WHERE ";
        lQuery += " session_id LIKE '%:' ";
        if (! clGexDbQuery.Execute(lQuery))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("%1").arg(clGexDbQuery.lastError().text()).toLatin1().constData());
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
        }
    }
    clGexDbQuery.exec("COMMIT");

    return true;
}

void GexDbPlugin_Galaxy::ReleaseTokens()
{
    QString strQuery;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    QString lTokenTable = NormalizeTableName("token", false);
    // Clean token table
    strQuery = "DELETE FROM "+lTokenTable+" WHERE session_id='"+QString::number(m_nInsertionSID)+":"+m_strInsertionHostName+"'";
    if (! clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
    }
    clGexDbQuery.exec("COMMIT");
}

///////////////////////////////////////////////////////////
// Update GALAXY database : Start transaction mode
///////////////////////////////////////////////////////////
// Start a transaction if necesary
///////////////////////////////////////////////////////////
// Use transaction during the consolidation phase
// All tables with splitlot_id index must be filled in NON TRANSACTION MODE
// for MULTI INSERTION MULTI SQL_THREAD. If start a transaction during this phase,
// all other thread will be paused or DeadLocked killed
// During the consolidation phase, data checked must be coherent
// Start a transaction in READ ONLY mode to allow all other thread in
// read acces mode but stop threads that wants update consolidated table
// Update the variable INNODB_LOCK_WAIT_TIMEOUT that was 60 seconds by default
// to 10 mn to provide the process to finish the consolidation phase
// Split the consolidation phase between wafer/lot or other process
// with Stop and Start new transaction to allow other thread to start
// a consolidation phase even if the current consolidation phase
// is not finished
///////////////////////////////////////////////////////////
// Query contains the selection of rows to lock
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::StartTransactionMode(QString strQueryForUpdate)
{
    // NOT TRANSACTION MODE
    // USE TOKEN TABLE
    return true;

    QString strQuery;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    if (! m_pclDatabaseConnector->m_bTransactionDb)
        return true;

    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        QString strValue;
        strQuery = "SHOW VARIABLES LIKE 'AUTOCOMMIT'";
        if (! clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        clGexDbQuery.First();
        strValue = clGexDbQuery.value(1).toString().toUpper();
        if ((strValue != "OFF") && (strValue != "0"))
        {
            // Valid the current transaction
            clGexDbQuery.Execute("COMMIT");
            clGexDbQuery.Execute("UNLOCK TABLES");

            // Start transaction
            clGexDbQuery.Execute("SET AUTOCOMMIT=0");  // Same as START TRANSACTION
            clGexDbQuery.Execute("SET TRANSACTION ISOLATION LEVEL REPEATABLE READ");
            //In REPEATABLE READ every lock acquired during a transaction is held for the duration of the transaction.
            //In READ COMMITTED the locks that did not match the scan are released after the STATEMENT completes.
            //clGexDbQuery.Execute("SET TRANSACTION ISOLATION LEVEL READ COMMITTED");

            // Debug message
            WriteDebugMessageFile(QString("---- GexDbPlugin_Galaxy::StartTransactionMode: Start transaction Mode"));
        }
        else
            WriteDebugMessageFile(QString("---- GexDbPlugin_Galaxy::StartTransactionMode: Start transaction Mode already started"));
    }

    if (! strQueryForUpdate.isEmpty())
    {
        GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

        strQuery = strQueryForUpdate;
        if (! strQuery.endsWith("FOR UPDATE", Qt::CaseInsensitive))
            strQuery += " FOR UPDATE";

        TimersTraceStart("Transaction Start on "+strQueryForUpdate.toUpper().simplified().section("FROM",1).section("WHERE",0,0));

        if (! clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        TimersTraceStop("Transaction Start on "+strQueryForUpdate.toUpper().simplified().section("FROM",1).section("WHERE",0,0));
    }

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : Stop transaction mode
///////////////////////////////////////////////////////////
// if bCommit then validated the current transaction and stop
// else Rollback before stop the transaction
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::StopTransactionMode(bool bCommit)
{
    QString strQuery;

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // NOT TRANSACTION MODE
    // USE TOKEN TABLE
    clGexDbQuery.Execute("COMMIT");
    return true;

    if (! m_pclDatabaseConnector->m_bTransactionDb)
        return true;

    TimersTraceStart("Transaction Stop");

    // Valid the current transaction
    if (bCommit)
        clGexDbQuery.Execute("COMMIT");
    else
        clGexDbQuery.Execute("ROLLBACK");

    // Unlock InnoDb table
    if (m_pclDatabaseConnector->IsMySqlDB())
        clGexDbQuery.Execute("UNLOCK TABLES");

    TimersTraceStop("Transaction Stop");

    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        QString strValue;
        strQuery = "SHOW VARIABLES LIKE 'AUTOCOMMIT'";
        if (! clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        clGexDbQuery.First();
        strValue = clGexDbQuery.value(1).toString().toUpper();
        if ((strValue == "OFF") || (strValue == "0"))
        {
            // Stop transaction mode
            clGexDbQuery.Execute("SET AUTOCOMMIT=1");

            // Debug message
            WriteDebugMessageFile(QString("---- GexDbPlugin_Galaxy::StopTransactionMode: Stop transaction Mode"));
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
// AddNewPartition
//////////////////////////////////////////////////////////////////////
// Before execute insertion into result table
// have to create new partition by day on new tablespaces by week
// NEW TABLESPACE FOR THE WEEK : GEXDB_FT_R_0637
// NEW PARTITIONS FOR THE DAY  : 0609130000 for table ft_ptest_results on tablespace GEXDB_FT_R_0637
// NEW SEQUENCE   FOR THE DAY  : 609130000 for the ft_splitlot_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateTablespacePartitionsSequence()
{
    QString strValue;
    QString strTableKey;
    QString strTableDesc;

    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateTablespacePartitionsSequence()");

    /////////////////////////////////////////
    // FIRST EXECUTION
    // Have the TestingStage defined
    // Update the table description if empty
    // Then will be updated after each new partition
    QString strQuery;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    strTableKey = NormalizeTableName("_SPLITLOT") + ".columns";
    if (! m_mapTablesDesc.contains(strTableKey.toUpper()))
    {
        /////////////////////////////////////////
        // RETRIEVE SPLITLOT TABLE COLUMNS
        // m_mapPartitionsDesc contains <table_name.columns , columns_list>
        if (m_pclDatabaseConnector->IsMySqlDB())
        {
            strQuery = "SELECT COLUMN_NAME FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='"+m_pclDatabaseConnector->m_strSchemaName
                    +"' AND TABLE_NAME='"+QString(m_strPrefixTable+"_splitlot").toLower()+"' ORDER BY ORDINAL_POSITION";
            if (! clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            strTableDesc = "";
            while (clGexDbQuery.Next())
            {
                if (! strTableDesc.isEmpty())
                    strTableDesc += ",";
                strTableDesc += clGexDbQuery.value(0).toString();
            }
            m_mapTablesDesc[strTableKey.toUpper()] = strTableDesc.toUpper();
        }

    }
    strTableKey = NormalizeTableName("_RUN") + ".columns";
    if (! m_mapTablesDesc.contains(strTableKey.toUpper()))
    {
        /////////////////////////////////////////
        // RETRIEVE RUN TABLE COLUMNS
        // m_mapPartitionsDesc contains <table_name.columns , columns_list>
        if (m_pclDatabaseConnector->IsMySqlDB())
        {
            strQuery = "SELECT COLUMN_NAME FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='"+m_pclDatabaseConnector->m_strSchemaName
                    + "' AND TABLE_NAME='" + QString(m_strPrefixTable + "_run").toLower() + "' ORDER BY ORDINAL_POSITION";
            if (! clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            strTableDesc = "";
            while (clGexDbQuery.Next())
            {
                if (! strTableDesc.isEmpty())
                    strTableDesc += ",";
                strTableDesc += clGexDbQuery.value(0).toString();
            }
            m_mapTablesDesc[strTableKey.toUpper()] = strTableDesc.toUpper();
        }
    }
    strTableKey = NormalizeTableName("_PTEST_RESULTS") + ".columns";
    if (! m_mapTablesDesc.contains(strTableKey.toUpper()))
    {
        /////////////////////////////////////////
        // RETRIEVE PTEST_RESULTS TABLE COLUMNS
        // RETRIEVE MPTEST_RESULTS TABLE COLUMNS
        // RETRIEVE FTEST_RESULTS TABLE COLUMNS
        // m_mapPartitionsDesc contains <table_name.columns , columns_list>
        if (m_pclDatabaseConnector->IsMySqlDB())
            strQuery = "SELECT TABLE_NAME,COLUMN_NAME FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='"+m_pclDatabaseConnector->m_strSchemaName
                    +"' AND TABLE_NAME LIKE '"+QString(m_strPrefixTable+"_%test_results").toLower()+"' ORDER BY TABLE_NAME,ORDINAL_POSITION";

        if (! clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        strTableDesc = "";
        QString lTableName;
        while (clGexDbQuery.Next())
        {
            strTableKey = NormalizeTableName(clGexDbQuery.value(0).toString(), false) + ".columns";
            if (clGexDbQuery.value(0).toString() != lTableName)
            {
                if (! lTableName.isEmpty())
                {
                    // Save the columns for the previous table
                    m_mapTablesDesc[QString(NormalizeTableName(lTableName,false)+".columns").toUpper()] = strTableDesc.toUpper();
                }

                strTableDesc = "";
                lTableName = clGexDbQuery.value(0).toString();
            }
            if (! strTableDesc.isEmpty())
                strTableDesc += ",";
            strTableDesc += clGexDbQuery.value(1).toString();
        }
        // Save the columns for the last table
        m_mapTablesDesc[strTableKey.toUpper()] = strTableDesc.toUpper();
    }
    /////////////////////////////////////////
    // RETRIEVE THE CURRENT SPLITLOT SEQUENCE
    // Always update the splitlot sequence to have the last from the database
    // m_mapPartitionsDesc contains <table_name_sequence , splitlot_id>
    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        // update sequence for splitlot
        // only if not already updated
        strTableKey = NormalizeTableName("_SPLITLOT_SEQUENCE");

        strQuery = "SELECT AUTO_INCREMENT FROM information_schema.tables";
        strQuery += " WHERE TABLE_SCHEMA='" + m_pclDatabaseConnector->m_strSchemaName + "'";
        strQuery += " AND TABLE_NAME='" + QString(m_strPrefixTable + "_splitlot").toLower() + "'";
        if (! clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // if already exist
        if (clGexDbQuery.First() && ! clGexDbQuery.isNull(0))
            m_mapTablesDesc[strTableKey.toUpper()] = clGexDbQuery.value(0).toString();
        else
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        QString(m_strPrefixTable.toLower() +
                                "_splitlot.AUTO_INCREMENT is undefined").toLatin1().constData());
            return false;
        }
    }

    //////////////////////////////
    // TABLESPACE MAINTENANCE
    QString strWeek, strTablespaceName;
    if (! CheckAndUpdateTablespace(strWeek, strTablespaceName))
    {
        *m_pbDelayInsertion = true;
        return false;
    }
    strTableKey = m_strPrefixTable + "_PTEST_RESULTS|active_tablespace";
    m_mapTablesDesc[strTableKey.toUpper()] = strTablespaceName;

    // Partition management can be DISABLE
    if (PartitionGranularityOption == "DISABLED")
    {
        // Nothing to do
        // The Partition and the AutoIncrement must be manage outside YM
        return true;
    }

    /////////////////////////////////////////
    // Then force the current partition mode if necessary
    // Retrieve the partitioning granularity from last active partition
    // New partition will be active only at the end of the active partitioning mode
    // Retreive the current Partition Naming convention
    strValue = GetPartitionName();
    // Check if have to create a new Partition
    // If not, apply the current Naming Convention
    strTableKey = m_strPrefixTable + "_PTEST_RESULTS|" + strValue;
    if (m_mapTablesDesc.contains(strTableKey.toUpper()))
    {
        // Partition already exists
        // Debug message
        WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateTablespacePartitionsSequence() completed - Partition already exists");
        return true;
    }

    // Partition not already exists
    // Update Partitions Granularity
    if (! UpdateTablespacePartitionsSequenceBy())
        return false;

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateTablespacePartitionsSequence() completed");

    return true;
}

//////////////////////////////////////////////////////////////////////
// TABLESPACE FORMAT = GEXDB_FT_R_0931 [GexAdminUser_TestingStage_R_YYWW]
//////////////////////////////////////////////////////////////////////
QMap<QString, int> g_mapTableSpaceNbFileId;
QMap<QString, int> g_mapTableSpaceLastFileId;
QMap<QString, float> g_mapTableSpaceSizePerPartition;
QMap<QString, float> g_mapTableSpaceSizeAvailable;

bool GexDbPlugin_Galaxy::CheckAndUpdateTablespace(QString& strWeek, QString& strTablespaceName)
{

    //////////////////////////////
    // TABLESPACE MAINTENANCE
    //////////////////////////////
    // yearNumber is not always the same as year().
    // 1 January 2000 has week number 52 in the year 1999
    // 31 December 2002 has week number 1 in the year 2003
    int nWeekYear;
    int nWeek = QDate::currentDate().weekNumber(&nWeekYear);
    int nNb = 1000000 + nWeekYear * 100 + nWeek;
    strWeek = QString::number(nNb).right(4);
    strTablespaceName = m_pclDatabaseConnector->m_strSchemaName.toUpper() + "_" + m_strPrefixTable.toUpper() + "_R_" + strWeek;

    return true;
}

//////////////////////////////////////////////////////////////////////
// Update the Sequence Splitlot with the given param
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::CheckAndUpdateSequence(int nFirstSplitlotSequence)
{
    //////////////////////////////
    // SEQUENCE SPLITLOT MAINTENANCE

    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        // update sequence for splitlot
        QString strQuery;
        GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

        // update sequence for splitlot
        // only if not already updated
        int nSplitlotSequence = 0;
        QString strTableName = NormalizeTableName("_SPLITLOT");

        if (m_mapTablesDesc.contains(QString(strTableName + "_SEQUENCE").toUpper()))
            nSplitlotSequence = m_mapTablesDesc[QString(strTableName + "_SEQUENCE").toUpper()].toLongLong();

        if (nSplitlotSequence >= nFirstSplitlotSequence)
            return true;

        // MULTI INSERTION MULTI SQL_THREAD
        // Lock table to pause other thread executions
        // Create
        // Unlock
        WriteDebugMessageFile("     GexDbPlugin_Galaxy::CheckAndUpdateSequence("+QString::number(nFirstSplitlotSequence)+")");
        LockTables(NormalizeTableName("_SPLITLOT"), false);

        strQuery = "SELECT AUTO_INCREMENT FROM information_schema.tables";
        strQuery += " WHERE TABLE_SCHEMA='" + m_pclDatabaseConnector->m_strSchemaName + "'";
        strQuery += " AND TABLE_NAME='" + QString(m_strPrefixTable + "_splitlot").toLower() + "'";
        if (! clGexDbQuery.Execute(strQuery))
        {
            // cannot create new tablespace
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            UnlockTables();
            return false;
        }

        // if already exist
        if (clGexDbQuery.First() && ! clGexDbQuery.isNull(0))
        {
            if (clGexDbQuery.value(0).toLongLong() >= nFirstSplitlotSequence)
            {
                m_mapTablesDesc[QString(strTableName + "_SEQUENCE").toUpper()] = clGexDbQuery.value(0).toString();
                UnlockTables();
                return true;
            }
        }
        else
        {
            // No results
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        QString(strTableName + ".AUTO_INCREMENT is undefined").toLatin1().constData());
            return false;
        }


        strQuery = "ALTER TABLE " + strTableName;
        strQuery += " AUTO_INCREMENT=" + QString::number(nFirstSplitlotSequence + 1);
        if (! clGexDbQuery.Execute(strQuery))
        {
            // cannot create new tablespace
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            UnlockTables();
            return false;
        }

        m_mapTablesDesc[QString(strTableName + "_SEQUENCE").toUpper()] = QString::number(nFirstSplitlotSequence);
        UnlockTables();
        WriteDebugMessageFile("     GexDbPlugin_Galaxy::CheckAndUpdateSequence() completed");

        return true;
    }
    else if (m_pclDatabaseConnector->IsSQLiteDB())
    {
        qDebug("GexDbPlugin_Galaxy::CheckAndUpdateSequence : SQLite : Is there something to do ?");
        return true;
    }


    return true;
}

//////////////////////////////////////////////////////////////////////
// PARTITIONS BY DAY
//////////////////////////////////////////////////////////////////////
// ORACLE
//////////////////////////////////////////////////////////////////////
// One TableSpace by week
// One partition by week or day or splitlot
// One sequence splitlot by week or day or splitlot
//////////////////////////////////////////////////////////////////////
// TABLESPACE FORMAT = GEXDB_FT_R_0931 [GexAdminUser_TestingStage_R_YYWW]
// PARTITION FORMAT = D0907300000 [mode + YYDDD00000]
// SEQUENCE SPLITLOT FORMAT = 907300000 [YYDDD00000 + autoincrement]
//////////////////////////////////////////////////////////////////////
// MYSQL 5.1
//////////////////////////////////////////////////////////////////////
// One TableSpace/partition by month, week or day
// One sequence splitlot by month, week or day
//////////////////////////////////////////////////////////////////////
// PARTITION FORMAT = D0907300000 [mode + YYDDD00000]
// SEQUENCE SPLITLOT FORMAT = 907300000 [YYDDD00000 + autoincrement]
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateTablespacePartitionsSequenceBy()
{
    QString strPartitionKey;
    QString strPartitionDesc;
    QString strTablespaceName;
    strPartitionKey = m_strPrefixTable + "_PTEST_RESULTS|active_tablespace";
    strTablespaceName = m_mapTablesDesc[strPartitionKey.toUpper()];

    //////////////////////////////
    // TABLESPACE MAINTENANCE
    if (strTablespaceName.isEmpty())
    {
        *m_pbDelayInsertion = true;
        return false;
    }

    // SQLITE
    if (m_pclDatabaseConnector->IsSQLiteDB())
    {
        qDebug("GexDbPlugin_Galaxy::UpdateTablespacePartitionsSequenceBy : SQLite : Is there something to do ?");
        return true;
    }

    QString strQuery;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    /////////////////////////////////////////
    // RETRIEVE PARTITION INFORMATION
    // m_mapPartitionsDesc contains <table_name|last_tablespace , tablespace_name>
    // m_mapPartitionsDesc contains <table_name|last_partition , partition_name|MAXVALUE>
    // m_mapPartitionsDesc contains <table_name|active_partition , partition_name|MAXVALUE>
    // m_mapPartitionsDesc contains <table_name|partition_name , partition_position|MAXVALUE>
    // last_partition and active_partition can be different for MySql
    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        strQuery = "SELECT TABLE_NAME, PARTITION_NAME, PARTITION_ORDINAL_POSITION, PARTITION_DESCRIPTION FROM information_schema.PARTITIONS";
        strQuery += " WHERE ";
        strQuery += " ((TABLE_NAME='" + QString(m_strPrefixTable + "_run").toLower() + "')";
        strQuery += " OR (TABLE_NAME='" + QString(m_strPrefixTable + "_run_consolidation").toLower() + "') ";
        strQuery += " OR (TABLE_NAME='" + QString(m_strPrefixTable + "_ptest_results").toLower() + "')";
        if (m_eTestingStage != eElectTest)
        {
            strQuery += " OR (TABLE_NAME='" + QString(m_strPrefixTable + "_mptest_results").toLower() + "') ";
            strQuery += " OR (TABLE_NAME='" + QString(m_strPrefixTable + "_ftest_results").toLower() + "') ";
        }
        strQuery += " ) AND TABLE_SCHEMA = '" + m_pclDatabaseConnector->m_strSchemaName + "'";
        strQuery += " ORDER BY PARTITION_ORDINAL_POSITION";

        if (! clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        while (clGexDbQuery.next())
        {
            // PARTITION_NAME can be NULL
            // ignore this line
            if (clGexDbQuery.value(1).toString().isEmpty())
                continue;

            // Update the active partition with the last partition (before the last in the query)
            strPartitionKey = clGexDbQuery.value(0).toString() + "|active_partition";
            strPartitionDesc = clGexDbQuery.value(1).toString() + "|" + clGexDbQuery.value(3).toString();
            // Ignore LASTPART partition
            if (! strPartitionDesc.startsWith("LASTPART"))
                m_mapTablesDesc[strPartitionKey.toUpper()] = strPartitionDesc.toUpper();
            // Update the last partition with the last partition (last in the query)
            strPartitionKey = clGexDbQuery.value(0).toString() + "|last_partition";
            strPartitionDesc = clGexDbQuery.value(1).toString() + "|" + clGexDbQuery.value(3).toString();
            m_mapTablesDesc[strPartitionKey.toUpper()] = strPartitionDesc.toUpper();
            // Update partitions description
            strPartitionKey = clGexDbQuery.value(0).toString() + "|" + clGexDbQuery.value(1).toString();
            strPartitionDesc = clGexDbQuery.value(2).toString() + "|" + clGexDbQuery.value(3).toString();
            m_mapTablesDesc[strPartitionKey.toUpper()] = strPartitionDesc.toUpper();
        }
    }

    // MYSQL WITH PARTITION
    if (m_pclDatabaseConnector->IsMySqlDB() && m_pclDatabaseConnector->m_bPartitioningDb)
    {
        // FOR OPITIMIZATION, REOGANIZE MUST TO BE DONE IN AN EMPTY PARTITION
        // WE HAVE TO GENERATE ALSO THE PARTITION FOR THE NEXT MONTH WITH 'MAXVALUE'
        // AND WHEN THE DATE CHANGE FOR THIS MONTH, REORGANIZE/SPLIT THIS EMPTY PARTITION

        // only in MySql Partitioning mode
        // and only if partition is supported by the current version
        QString strPartitionName;  // the partition name for this day
        QString strLastPartitionName;  // for empty partition
        int nLowLimitValue;  // the new hight value for this day
        int nHighLimitValue;

        strPartitionName = GetPartitionName(&nHighLimitValue);
        nLowLimitValue = strPartitionName.mid(1).toInt();
        strLastPartitionName = "LASTPART";

        QString strPartitionKey;
        QString strPartitionDesc;
        QString strTableName;
        QString strIndexName;
        QStringList strLstTables;
        //////////////////////////////
        // Check if need to create new PARTITION for all tables results
        // even if no result will be insert (ex for MPTEST_RESULTS, partition can be always empty)
        //
        // E_TEST has only PTEST tables (no MPTEST, FTEST)
        //////////////////////////////

        strLstTables.append("_PTEST_RESULTS");
        if (m_eTestingStage != eElectTest)
        {
            strLstTables.append("_MPTEST_RESULTS");
            strLstTables.append("_FTEST_RESULTS");
        }

        // And also create a partition for RUN table
        strLstTables.append("_RUN");

        QStringList::Iterator itTable;
        for (itTable = strLstTables.begin(); itTable != strLstTables.end(); ++itTable)
        {
            strTableName = NormalizeTableName(*itTable);
            strIndexName = QString(m_strPrefixTable + (*itTable)).toLower();

            strPartitionKey = strIndexName + "|" + strPartitionName;

            if (m_mapTablesDesc.contains(strPartitionKey.toUpper()))
            {
                continue;  // already exist
            }

            // Partition not presents in the local structure
            // For the first execution of the day, Partition probably not exists
            // have to create it
            // For the second execution, Partition probably exists
            // have just to check and update the local structure

            // MULTI INSERTION MULTI SQL_THREAD
            // Before each table update, LOCK TABLES forces to pause other thread
            // Threat1 has the lock
            // Thread2 waits
            // Thread1 check the partition
            // Thread1 creates the partition if needed
            // Thread1 unlock
            // Thread2 has the lock
            // Thread2 check the partition (create by Thread1)
            // Thread2 unlock
            LockTables(strTableName, false);

            // Query over information_schema.partitions takes long time
            // Check if have already this information before to execute the query
            // m_mapPartitionsDesc contains <table_name|partition_name , partition_position|MAXVALUE>
            strQuery = "SELECT PARTITION_NAME, PARTITION_ORDINAL_POSITION, PARTITION_DESCRIPTION FROM information_schema.PARTITIONS";
            strQuery += " WHERE (TABLE_NAME)='" + strIndexName.toLower() + "'";
            strQuery += " AND (TABLE_SCHEMA) = '" + m_pclDatabaseConnector->m_strSchemaName + "'";
            strQuery += " AND lower(PARTITION_NAME) = '" + strPartitionName.toLower() + "'";
            strQuery += " ORDER BY PARTITION_ORDINAL_POSITION";

            if (! clGexDbQuery.Execute(strQuery))
            {
                // cannot create new tablespace
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                UnlockTables();
                return false;
            }

            if (clGexDbQuery.First())
            {
                // Update the active partition with the last partition (before the last in the query)
                strPartitionKey = strIndexName + "|active_partition";
                strPartitionDesc = clGexDbQuery.value(0).toString() + "|" + clGexDbQuery.value(2).toString();
                // Ignore LASTPART partition
                if (! strPartitionDesc.startsWith("LASTPART", Qt::CaseInsensitive))
                    m_mapTablesDesc[strPartitionKey.toUpper()] = strPartitionDesc.toUpper();
                // Update the last partition with the last partition (last in the query)
                strPartitionKey = strIndexName + "|last_partition";
                strPartitionDesc = clGexDbQuery.value(0).toString() + "|" + clGexDbQuery.value(2).toString();
                m_mapTablesDesc[strPartitionKey.toUpper()] = strPartitionDesc.toUpper();
                // Update partitions description
                strPartitionKey = strIndexName + "|" + clGexDbQuery.value(0).toString();
                strPartitionDesc = clGexDbQuery.value(1).toString() + "|" + clGexDbQuery.value(2).toString();
                m_mapTablesDesc[strPartitionKey.toUpper()] = strPartitionDesc.toUpper();

                // Already updated
                // Nothing to do
                UnlockTables();
                continue;  // already exist
            }

            // check the list of partition
            // to create new one
            QString strCurrentPartitionName;  // the last partition find in MySql
            QString strLastHighValue;  // the high_value for this partition

            strPartitionKey = strIndexName + "|last_partition";
            strPartitionDesc = m_mapTablesDesc[strPartitionKey.toUpper()];
            strCurrentPartitionName = strPartitionDesc.section("|", 0, 0);
            strLastHighValue = strPartitionDesc.section("|", 1);

            // split the last partition in 2 partitions
            if (strLastHighValue.contains("MAXVALUE", Qt::CaseInsensitive))
            {
                if (strCurrentPartitionName == strLastPartitionName)
                {
                    // P(YYMM-1) LESS THAN VALUES(YYMM000000)
                    // LASTPART LESS THAN VALUES(MAXVALUE)

                    // then split LASTPART partition into 2 partitions
                    // P(YYMM-1) LESS THAN VALUES(YYMM000000)
                    // P(YYMM) LESS THAN VALUES ((YYMM+1)000000)
                    // LASTPART LESS THAN VALUES(MAXVALUE)

                    strQuery = "ALTER TABLE " + strTableName;
                    strQuery += " REORGANIZE PARTITION " + strCurrentPartitionName;
                    strQuery +=" INTO (PARTITION "+strPartitionName+" VALUES LESS THAN ("+QString::number(nHighLimitValue)+"),";
                    strQuery += " PARTITION " + strLastPartitionName + " VALUES LESS THAN MAXVALUE)";
                }
                else
                {
                    // P(YYMM-1) LESS THAN VALUES(MAXVALUE)

                    // then split P(YYMM-1) partition into 3 partitions
                    // P(YYMM-1) LESS THAN VALUES(YYMM000000)
                    // P(YYMM) LESS THAN VALUES ((YYMM+1)000000)
                    // LASTPART LESS THAN VALUES(MAXVALUE)

                    strQuery = "ALTER TABLE " + strTableName;
                    strQuery += " REORGANIZE PARTITION " + strCurrentPartitionName;
                    strQuery +=" INTO (PARTITION "+strCurrentPartitionName+" VALUES LESS THAN ("+QString::number(nLowLimitValue)+"),";
                    strQuery +=" PARTITION "+strPartitionName+" VALUES LESS THAN ("+QString::number(nHighLimitValue)+"),";
                    strQuery += " PARTITION " + strLastPartitionName + " VALUES LESS THAN MAXVALUE)";
                }
                if (! clGexDbQuery.Execute(strQuery))
                {
                    // cannot create new tablespace
                    *m_pbDelayInsertion = true;
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                                clGexDbQuery.lastError().text().toLatin1().constData());
                    UnlockTables();
                    return false;
                }

            }
            else if (strCurrentPartitionName.isEmpty())
            {
                // no partition then add the firspart and the new partition
                // FIRSTPART LESS THAN VALUES(YYMM000000)
                // P(YYMM) LESS THAN VALUES ((YYMM+1)000000)
                // LASTPART LESS THAN VALUES(MAXVALUE)
                strQuery = "ALTER TABLE " + strTableName;
                strQuery += " PARTITION BY RANGE(SPLITLOT_ID)";
                strQuery += "(PARTITION FIRSTPART VALUES LESS THAN (" + QString::number(nLowLimitValue) + "),";
                strQuery +=" PARTITION "+strPartitionName+" VALUES LESS THAN ("+QString::number(nHighLimitValue)+"),";
                strQuery += " PARTITION " + strLastPartitionName + " VALUES LESS THAN MAXVALUE)";
                if (! clGexDbQuery.Execute(strQuery))
                {
                    // cannot create new tablespace
                    *m_pbDelayInsertion = true;
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                                clGexDbQuery.lastError().text().toLatin1().constData());
                    UnlockTables();
                    return false;
                }
            }
            else
            {
                // then add new partition
                // P(YYMM) LESS THAN VALUES ((YYMM+1)000000)
                // LASTPART LESS THAN VALUES(MAXVALUE)
                strQuery = "ALTER TABLE " + strTableName;
                strQuery += " ADD PARTITION ";
                strQuery +="(PARTITION "+strPartitionName+" VALUES LESS THAN ("+QString::number(nHighLimitValue)+"),";
                strQuery += " PARTITION " + strLastPartitionName + " VALUES LESS THAN MAXVALUE)";
                if (! clGexDbQuery.Execute(strQuery))
                {
                    // cannot create new tablespace
                    *m_pbDelayInsertion = true;
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                    UnlockTables();
                    return false;
                }
            }

            UnlockTables();

            GSLOG(SYSLOG_SEV_DEBUG, QString(" CREATE PARTITION %1").arg(strPartitionName).toLatin1().constData());

        }

        //////////////////////////////
        // SEQUENCE SPLITLOT MAINTENANCE

        // Each new month
        // have to update the sequence splitlot
        if (! CheckAndUpdateSequence(nLowLimitValue))
            return false;

        // Debug message
        WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateTablespacePartitionsSequenceBy() completed");

        return true;
    }

    // MYSQL WITHOUT PARTITION
    if (m_pclDatabaseConnector->IsMySqlDB() && ! m_pclDatabaseConnector->m_bPartitioningDb)
    {
        //////////////////////////////
        // SEQUENCE SPLITLOT MAINTENANCE

        // Each new month
        // have to update the sequence splitlot
        QString strPartitionName;
        int nLowLimitValue;  // the low value for this partition

        // Use the GetPartitionName to have the Normalized SplitlotId
        strPartitionName = GetPartitionName();
        nLowLimitValue = strPartitionName.mid(1).toLongLong();

        if (! CheckAndUpdateSequence(nLowLimitValue))
            return false;

        // Debug message
        WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateTablespacePartitionsSequenceBy() completed");

        return true;
    }

    // NONE OF THE SUPPORTED DRIVERS
    qDebug("GexDbPlugin_Galaxy::UpdateTablespacePartitionsSequenceBy: Unsupported SQL Driver !");
    *m_pbDelayInsertion = true;
    GSET_ERROR1(GexDbPlugin_Base, eDB_UnsupportedDriver, NULL, m_pclDatabaseConnector->m_strDriver.toLatin1().constData());
    return false;
}


///////////////////////////////////////////////////////////
// AutoIncrement manager
///////////////////////////////////////////////////////////
// For each new Splitlot, the SPLITLOT table is updated for
// possible access in the STORED PROCEDURE insertion_validation.
// Each line inserted in the SPLITLOT table, a new SPLITLOT_ID is used
// If the STORED PROCEDURE return a fail or a delay, this new SPLITLOT_ID
// was lost and generate a hold in the splitlot_sequence.
//
///////////////////////////////////////////////////////////
// The goal of this 2 functions is to update the splitlot_sequence
// only when the insertion is OK
// Before the validation phase
// get a new SPLITLOT_ID not used but without action in the splitlot_sequence
// (under the current sequence for MySql or without sequence updated for Oracle).
//
// After checking ValidationFunction, CALL insertion_validation procedure.
// get a valid SPLITLOT_ID (update the splitlot_sequence)
// then only update this temporary SplitlotId with a valid splitlot_sequence
///////////////////////////////////////////////////////////
// MULTI INSERTION MULTI SQL_THREAD
///////////////////////////////////////////////////////////
// * The temporary splitlot_id contains the SID connection
// The SID connection is usefull to know if a file is currently inserted by another thread
// Another SID connection can be checked to know if it is always actif (gex crash)
// Another SID connection actif can be used to delay the current insertion for a same file (LOT/WAFER/...)
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Update GALAXY database : GetTemporarySequenceIndex
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetTemporarySequenceIndex()
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::GetTemporarySequenceIndex");

    // autoincrement
    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        // For MySql
        // use a temporary sequence
        // Get the unique sid from MySql to have a unique temporary sequence
        m_nTemporarySplitLotId = 101000000 + m_nInsertionSID;

        // Because of the SPIDER environment, the SID can be NOT unique
        // if it was generated on 2 differents Sql Server
        // Open the TOKEN Session
        // This session will be released at the end of the insertion
        while (! GetToken(NormalizeTableName("_INSERTION"), QString::number(m_nTemporarySplitLotId), 0))
            ++m_nTemporarySplitLotId;

        QString lQuery;
        GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
        QString strTableName = NormalizeTableName("_SPLITLOT");

        // If gex crash, this splitlot_id could exist
        // Then delete this row
        lQuery = "DELETE FROM " + strTableName + " WHERE splitlot_id=" + QString::number(m_nTemporarySplitLotId);
        if (! clGexDbQuery.Execute(lQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

    }
    else if (m_pclDatabaseConnector->IsSQLiteDB())
    {
        WriteDebugMessageFile("---- GexDbPlugin_Galaxy::GetTemporarySequenceIndex : check me :");
        m_nTemporarySplitLotId = GetNextSplitlotIndex_SQLite();
    }
    else
        return false;

    WriteDebugMessageFile("TemporarySequenceIndex=" + QString::number(m_nTemporarySplitLotId));

    return true;
}

//////////////////////////////////////////////////////////////////////
// Insert splitlot data into GEXDB database
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::InsertSplitlotDataIntoGexdb(const QString& strTableName, const QString& strInsertQuery)
{
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    // Update list of updated tables
    if (! m_clUpdatedTables.contains(strTableName))
        m_clUpdatedTables.Add(strTableName);

    // execute query
    if (! clGexDbQuery.Execute(strInsertQuery))
    {
        WriteDebugMessageFile("GexDbPlugin_Galaxy::InsertSplitlotDataIntoGexdb: error executing query !");
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strInsertQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

bool GexDbPlugin_Galaxy::isInTheHBinSBinListNumber(const QString& number)
{
    return(mListHBinNUmber.contains(number) || mListSBinNUmber.contains(number));
}

//////////////////////////////////////////////////////////////////////
// Make sure the qApplication->processEvent function is not called too often
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::ProcessEvents()
{
    TimersTraceStart("ProcessEvents");

    GexDbPlugin_Base::ProcessEvents();
    //Todo : get the refresh rate from options : static int PEI=mGexScriptEngine?mGexScriptEngine->evaluate("").toInt32():PROCESS_EVENTS_INTERVAL;

    TimersTraceStop("ProcessEvents");
}

bool GexDbPlugin_Galaxy::TimersTraceReset(const QString& TimerName)
{
    if (! m_bCustomerDebugMode)
        return true;

    if (mAttributes["INSERTION_TRACE_ACTIVATION"].toString().isEmpty())
        return true;

    if (TimerName.isEmpty() || (TimerName.toUpper() == "ALL"))
        mTimersTrace.clear();
    else
    {
        mTimersTrace[TimerName] = 0;
        mTimersTrace[TimerName + " count"] = 0;
        mTimersTrace[TimerName + " status"] = 0;
    }
    return true;
}

bool GexDbPlugin_Galaxy::TimersTraceStart(const QString& TimerName)
{
    if (! m_bCustomerDebugMode)
        return true;

    if (mAttributes["INSERTION_TRACE_ACTIVATION"].toString().isEmpty())
        return true;

    int lElapsed = m_clInsertStdfFileTime.elapsed();
    if ((mAttributes["INSERTION_TRACE_ACTIVATION"].toString() != "ALL")
            && !mAttributes["INSERTION_TRACE_ACTIVATION"].toString().contains(TimerName.section(" ",0,0).toUpper())) return true;

    if (! mTimersTrace.contains(TimerName))
    {
        mTimersTrace[TimerName] = 0;
        mTimersTrace[TimerName + " count"] = 0;
        mTimersTrace[TimerName + " status"] = 0;
    }

    if (mTimersTrace[TimerName + " status"] == 1)
        return false;

    mTimersTrace[TimerName + " status"] = 1;
    mTimersTrace[TimerName] -= lElapsed;
    return true;
}

bool GexDbPlugin_Galaxy::TimersTraceStop(const QString& TimerName)
{
    if (! m_bCustomerDebugMode)
        return true;

    if (mAttributes["INSERTION_TRACE_ACTIVATION"].toString().isEmpty())
        return true;

    if ((mAttributes["INSERTION_TRACE_ACTIVATION"].toString() != "ALL")
            && !mAttributes["INSERTION_TRACE_ACTIVATION"].toString().contains(TimerName.section(" ",0,0).toUpper())) return true;

    if (! mTimersTrace.contains(TimerName))
        return false;

    if (mTimersTrace[TimerName + " status"] == 0)
        return false;

    mTimersTrace[TimerName + " status"] = 0;
    mTimersTrace[TimerName + " count"] = mTimersTrace[TimerName + " count"] + 1;
    mTimersTrace[TimerName] += m_clInsertStdfFileTime.elapsed();
    return true;
}

bool GexDbPlugin_Galaxy::TimersTraceDump(const QString& TimerName)
{
    if (! m_bCustomerDebugMode)
        return true;

    if (mAttributes["INSERTION_TRACE_ACTIVATION"].toString().isEmpty())
        return true;

    if (TimerName.isEmpty() || (TimerName.toUpper() == "ALL"))
    {
        foreach(const QString &lKey, mTimersTrace.keys())
        {
            if (lKey.endsWith(" status"))
                continue;
            if (lKey.endsWith(" count"))
                continue;
            if (mTimersTrace[lKey + " status"] != 0)
                continue;
            if (mTimersTrace[lKey] == 0)
                continue;

            GSLOG(SYSLOG_SEV_DEBUG, (QString("     >>>>  %1 processing cumul time %2 and count %3")
                                     .arg(lKey.simplified().toLatin1().constData())
                                     .arg(mTimersTrace[lKey])
                                     .arg(mTimersTrace[lKey + " count"])).toLatin1().constData());
            // Add trace info into the ym_events logs
            mpDbKeysEngine->dbKeysContent().Set(
                        QString("trace_%1_time").arg(lKey.simplified().replace(" ","_").replace("|","_").toLower()),QString::number(mTimersTrace[lKey]/1000));
            mpDbKeysEngine->dbKeysContent().Set(
                        QString("trace_%1_count").arg(lKey.simplified().replace(" ","_").replace("|","_").toLower()),QString::number(mTimersTrace[lKey + " count"]));
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, (QString("     >>>>  %1 processing cumul time %2 and count %3").arg(
                                     TimerName.simplified().toLatin1().constData())
                                 .arg(mTimersTrace[TimerName])
                                 .arg(mTimersTrace[TimerName + " count"])).toLatin1().constData());
        // Add trace info into the ym_events logs
        mpDbKeysEngine->dbKeysContent().Set(
                    QString("trace_%1_time").arg(TimerName.simplified().replace(" ","_").replace("|","_").toLower()),QString::number(mTimersTrace[TimerName]/1000));
        mpDbKeysEngine->dbKeysContent().Set(
                    QString("trace_%1_count").arg(TimerName.simplified().replace(" ","_").replace("|","_").toLower()),QString::number(mTimersTrace[TimerName + " count"]));
    }
    return true;
}
