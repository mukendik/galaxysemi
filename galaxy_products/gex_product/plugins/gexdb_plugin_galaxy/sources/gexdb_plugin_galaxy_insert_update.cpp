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
#include <gqtl_log.h>
#include "gqtl_datakeys.h"


// Standard includes

// Qt includes
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QProgressBar>
#include <QDir>
#include <QApplication>

// Galaxy modules includes
#include <stdf.h>
#include <stdfparse.h>

// FLAG DEFINITION
#define INVALID_SITE                               -2
#define MERGE_SITE                                 -1
#define MERGE_BIN                                  -1

///////////////////////////////////////////////////////////
// Update GALAXY database : StartSplitlotInsertion
///////////////////////////////////////////////////////////
// MULTI INSERTION MULTI SQL_THREAD
///////////////////////////////////////////////////////////
// Use a temporary
//                * MySql : splitlot_id = 101000000 + MySql(SID)
//                * Oracle : splitlot_id = 101010000 + Oracle(SID)
// All threads check definitif splitlot_id with VALID_SPLITLOT for reject
// All threads check temporary splitlot_id from actif SID for delay
///////////////////////////////////////////////////////////
// This temporary splitlot_id must be present until the end of the insertion
// with the current definitif splitlot_id
///////////////////////////////////////////////////////////
// Update Splitlot Table with temporary splitlot_id
//                * new row 101000123 with INVALID_SPLITLOT
// ValidationFunction check 101000123 for delay
// Update Splitlot Table with definitif splitlot_id
//                * keep the temporary splitlot_id 101000123
//                * new row 1104000015 with INVALID_SPLITLOT
// ValidationFunction check 101000123 for delay
// When updating to VALID_SPLITLOT, delete the temporary splitlot_id
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::StartSplitlotInsertion(bool bTemporarySplitlot)
{
    bool bStatus = true;


    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("Start Splitlot Insertion: LotId=%1 SubLot=%2 WaferId=%3 (%4)")
                                     .arg(m_strLotId.toLatin1().data())
                                     .arg(m_strSubLotId.toLatin1().data())
                                     .arg(mpDbKeysEngine->dbKeysContent().Get("Wafer").toString().toLatin1().data())
                                     .arg(bTemporarySplitlot?"tempInsertion":"finalInsertion" )).toLatin1().constData());

    QString             strTableName, strFileName, strFilePath;
    QString             strQuery, strMessage;
    QString             lColumns, lValues;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    strTableName = NormalizeTableName("_SPLITLOT");

    QFileInfo qFile(mpDbKeysEngine->dbKeysContent().Get("SourceArchive").toString());
    strFileName = qFile.fileName();
    if(mpDbKeysEngine->dbKeysContent().Get("MoveFile").toString() == "TRUE")
        strFilePath = mpDbKeysEngine->dbKeysContent().Get("MovePath").toString();
    else
        strFilePath = qFile.canonicalPath();

    // Update the splitlot_sequence only when the insertion is OK
    // After checking ValidationFunction, CALL insertion_validation procedure.

    // Before this validation, use a temporary SplitlotId
    // Then only update this temporary SplitlotId with a valid splitlot_sequence

    if(bTemporarySplitlot)
    {
        // MULTI INSERTION MULTI SQL_THREAD
        // No transaction

        if(!GetTemporarySequenceIndex())
        {
            bStatus = false;
            goto labelUnlock;
        }

        // Use Temporary splitlot_id for reference
        m_nSplitLotId = m_nTemporarySplitLotId;

        // Debug message
        strMessage =  "Inserting new row into " + strTableName;
        strMessage += " with splitlot_id=" + QString::number(m_nSplitLotId);
        WriteDebugMessageFile(strMessage);

        int nSetupTime;
        int nStartTime;
        int nFinishTime;
        nSetupTime =  mpDbKeysEngine->dbKeysContent().Get("SetupTime").toLongLong();
        nStartTime = mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong();
        nFinishTime = mpDbKeysEngine->dbKeysContent().Get("FinishTime").toLongLong();

        // case 7233: have WCR but no WIR
        if((m_eTestingStage != eFinalTest) && (m_clStdfWIR.m_u4START_T>0))
            nStartTime = m_clStdfWIR.m_u4START_T;
        if((m_eTestingStage == eWaferTest) && (m_clStdfWRR.m_u4FINISH_T>0))
            nFinishTime = m_clStdfWRR.m_u4FINISH_T;

        // If overwritten, then force to Key value
        if(mpDbKeysEngine->dbKeysContent().IsOverloaded("StartTime"))
            nStartTime = mpDbKeysEngine->dbKeysContent().Get("StartTime").toLongLong();
        if(mpDbKeysEngine->dbKeysContent().IsOverloaded("FinishTime"))
            nFinishTime = mpDbKeysEngine->dbKeysContent().Get("FinishTime").toLongLong();

        QString lTestInsertion = mpDbKeysEngine->dbKeysContent().Get("TestInsertion").toString();

        // insert new entry
        lColumns += "SPLITLOT_ID,";
        lValues += QString::number(m_nSplitLotId);
        lValues += ",";
        lColumns += "LOT_ID,";
        lValues += TranslateStringToSqlVarChar(m_strLotId);
        lValues += ",";
        lColumns += "SUBLOT_ID,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("SubLot").toString());
        if(m_eTestingStage != eFinalTest)
        {
            lValues += ",";
            lColumns += "WAFER_ID,";
            if((m_nWaferIndexToProcess==0) || (m_strWaferId.isEmpty()))
                lValues += "null";
            else
                lValues += TranslateStringToSqlVarChar(m_strWaferId);
        }
        lValues += ",";
        lColumns += "PRODUCT_NAME,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Product").toString());
        lValues += ",";
        if(m_eTestingStage != eElectTest)
        {
            lColumns += "SETUP_T,";
            lValues += QString::number(nSetupTime);
            lValues += ",";
        }
        lColumns += "START_T,";
        lValues += QString::number(nStartTime);
        lValues += ",";
        lColumns += "FINISH_T,";
        lValues += QString::number(nFinishTime);
        lValues += ",";
        if(m_eTestingStage != eElectTest)
        {
            lColumns += "STAT_NUM,";
            lValues += QString::number(mpDbKeysEngine->dbKeysContent().Get("Station").toInt());
            lValues += ",";
        }
        lColumns += "TESTER_NAME,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("TesterName").toString());
        lValues += ",";
        lColumns += "TESTER_TYPE,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("TesterType").toString());
        lValues += ",";
        lColumns += "SPLITLOT_FLAGS,";
        lValues += QString::number(m_nSplitLotFlags);
        lValues += ",";
        lColumns += "NB_PARTS,";
        lValues += m_bUsePcrForSummary ? QString::number(m_mapPCRNbParts[MERGE_SITE]) : QString::number(m_mapNbRuns[MERGE_SITE]);
        lValues += ",";
        lColumns += "NB_PARTS_GOOD,";
        lValues += m_bUsePcrForSummary ? QString::number(m_mapPCRNbPartsGood[MERGE_SITE]) : QString::number(m_mapNbRunsGood[MERGE_SITE]);
        lValues += ",";
        if(m_eTestingStage != eElectTest)
        {
            lColumns += "NB_PARTS_SAMPLES,";
            lValues += QString::number(m_mapNbRuns[MERGE_SITE]);
            lValues += ",";
            lColumns += "NB_PARTS_SAMPLES_GOOD,";
            lValues += QString::number(m_mapNbRunsGood[MERGE_SITE]);
            lValues += ",";
            lColumns += "NB_PARTS_SUMMARY,";
            lValues += QString::number(m_mapPCRNbParts[MERGE_SITE]);
            lValues += ",";
            lColumns += "NB_PARTS_SUMMARY_GOOD,";
            lValues += QString::number(m_mapPCRNbPartsGood[MERGE_SITE]);
            lValues += ",";
        }
        lColumns += "DATA_PROVIDER,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("DataOrigin").toString());
        lValues += ",";
        lColumns += "DATA_TYPE,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("DataType").toString());
        lValues += ",";
        lColumns += "PROD_DATA,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ProdData").toBool() ? "Y" : "N",true);
        lValues += ",";
        lColumns += "TEST_INSERTION,";
        lValues += TranslateStringToSqlVarChar(lTestInsertion);
        lValues += ",";
        lColumns += "TEST_FLOW,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("TestFlow").toString());
        lValues += ",";
        lColumns += "RETEST_INDEX,";
        lValues += QString::number(mpDbKeysEngine->dbKeysContent().Get("RetestIndex").toInt());
        lValues += ",";
        lColumns += "RETEST_HBINS,";
        if(!mpDbKeysEngine->dbKeysContent().Get("RetestBinList").toString().isEmpty())
        {
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("RetestBinList").toString());
        }
        else if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_RDR]>0)
        {
            lValues += "'";
            // 1,3-6,9
            int i, iBin=-1, iFirst=-1, iLast=-1;
            QString strSep = "";
            QString strFirst, strLast, strBin;
            for(i=0; i<m_clStdfRDR.m_u2NUM_BINS; i++)
            {
                iBin = m_clStdfRDR.m_ku2RTST_BIN[i];
                if(iFirst < 0)
                    iFirst = iLast = iBin;
                if(iBin == iLast+1)
                    iLast = iBin;
                else
                {
                    strFirst.sprintf("%d",iFirst);
                    strLast.sprintf("%d",iLast);
                    strBin.sprintf("%d",iBin);
                    if(iFirst == iLast)
                        lValues += strSep + strFirst;
                    else
                        lValues += strSep + strFirst + "-" + strLast;
                    strSep = ",";
                    iFirst = iLast = iBin;

                }
            }
            strFirst.sprintf("%d",iFirst);
            strLast.sprintf("%d",iLast);
            strBin.sprintf("%d",iBin);
            if(iFirst == iLast)
                lValues += strSep + strFirst;
            else
                lValues += strSep + strFirst + "-" + strLast;
            lValues += "'";
        }
        else
            lValues += "null";
        lValues += ",";
        if(m_eTestingStage != eElectTest)
        {
            lColumns += "REWORK_CODE,";
            lValues += QString::number(m_nReworkCode);
            lValues += ",";
        }
        lColumns += "JOB_NAM,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ProgramName").toString());
        lValues += ",";
        lColumns += "JOB_REV,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ProgramRevision").toString());
        lValues += ",";
        lColumns += "OPER_NAM,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Operator").toString());
        lValues += ",";
        lColumns += "EXEC_TYP,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ExecType").toString());
        lValues += ",";
        lColumns += "EXEC_VER,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ExecVersion").toString());
        lValues += ",";
        if(m_eTestingStage != eElectTest)
        {
            lColumns += "TEST_COD,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("TestingCode").toString());
            lValues += ",";
        }
        lColumns += "FACIL_ID,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Facility").toString());
        lValues += ",";
        lColumns += "MODE_COD,";
        lValues += TranslateStringToSqlVarChar((QChar)mpDbKeysEngine->dbKeysContent().Get("DataType").toString()[0]);
        lValues += ",";
        if(m_eTestingStage != eElectTest)
        {
            lColumns += "TST_TEMP,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Temperature").toString());
            lValues += ",";
            lColumns += "RTST_COD,";
            lValues += TranslateStringToSqlVarChar((QChar)mpDbKeysEngine->dbKeysContent().Get("RetestCode").toString()[0]);
            lValues += ",";
            lColumns += "PROT_COD,";
            lValues += TranslateStringToSqlVarChar((QChar)mpDbKeysEngine->dbKeysContent().Get("ProtectionCode").toString()[0]);
            lValues += ",";
            lColumns += "BURN_TIM,";
            lValues += mpDbKeysEngine->dbKeysContent().Get("BurninTime").toString();
            lValues += ",";
            lColumns += "CMOD_COD,";
            lValues += TranslateStringToSqlVarChar((QChar)mpDbKeysEngine->dbKeysContent().Get("CommandModeCode").toString()[0]);
            lValues += ",";
        }
        lColumns += "PART_TYP,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Product").toString());
        lValues += ",";
        lColumns += "USER_TXT,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("UserText").toString());
        lValues += ",";
        if(m_eTestingStage != eElectTest)
        {
            lColumns += "AUX_FILE,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("AuxiliaryFile").toString());
            lValues += ",";
            lColumns += "PKG_TYP,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("PackageType").toString());
            lValues += ",";
        }
        lColumns += "FAMLY_ID,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Family").toString());
        lValues += ",";
        if(m_eTestingStage != eElectTest)
        {
            lColumns += "DATE_COD,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("DateCode").toString());
            lValues += ",";
            lColumns += "FLOOR_ID,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Floor").toString());
            lValues += ",";
        }
        lColumns += "PROC_ID,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Process").toString());
        lValues += ",";
        if(m_eTestingStage != eElectTest)
        {
            lColumns += "OPER_FRQ,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("FrequencyStep").toString());
            lValues += ",";
        }
        lColumns += "SPEC_NAM,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().
                                               Get("SpecName").toString());
        lValues += ",";
        lColumns += "SPEC_VER,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().
                                               Get("SpecVersion").toString());
        lValues += ",";
        if (m_eTestingStage != eElectTest)
        {
            lColumns += "FLOW_ID,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Flow").toString());
            lValues += ",";
            lColumns += "SETUP_ID,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Setup").toString());
            lValues += ",";
            lColumns += "DSGN_REV,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("DesignRevision").toString());
            lValues += ",";
            lColumns += "ENG_ID,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Engineering").toString());
            lValues += ",";
            lColumns += "ROM_COD,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("RomCode").toString());
            lValues += ",";
            lColumns += "SERL_NUM,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("SerialNumber").toString());
            lValues += ",";
            lColumns += "SUPR_NAM,";
            lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("SupervisorName").toString());
            lValues += ",";
            lColumns += "NB_SITES,";
            lValues += QString::number(m_nNbSites);
            lValues += ",";
            lColumns += "HEAD_NUM,";
            lValues += m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SDR] == 0 ? "null" : QString::number(m_clStdfSDR.m_u1HEAD_NUM);
            lValues += ",";
            lColumns += "HANDLER_TYP,";
            lValues += mpDbKeysEngine->dbKeysContent().Get("ProberType").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ProberType").toString());
            lValues += ",";
            lColumns += "HANDLER_ID,";
            lValues += mpDbKeysEngine->dbKeysContent().Get("ProberName").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ProberName").toString());
            lValues += ",";
            lColumns += "CARD_TYP,";
            lValues += (mpDbKeysEngine->dbKeysContent().Get("CardType").toString().isEmpty()) ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("CardType").toString());
            lValues += ",";
            lColumns += "CARD_ID,";
            lValues += (mpDbKeysEngine->dbKeysContent().Get("Card").toString().isEmpty()) ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Card").toString());
            lValues += ",";
            lColumns += "LOADBOARD_TYP,";
            lValues += mpDbKeysEngine->dbKeysContent().Get("LoadBoardType").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("LoadBoardType").toString());
            lValues += ",";
            lColumns += "LOADBOARD_ID,";
            lValues += mpDbKeysEngine->dbKeysContent().Get("LoadBoard").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("LoadBoard").toString());
            lValues += ",";
            lColumns += "DIB_TYP,";
            lValues += mpDbKeysEngine->dbKeysContent().Get("DibType").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("DibType").toString());
            lValues += ",";
            lColumns += "DIB_ID,";
            lValues += mpDbKeysEngine->dbKeysContent().Get("DibName").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("DibName").toString());
            lValues += ",";
            lColumns += "CABLE_TYP,";
            lValues += (mpDbKeysEngine->dbKeysContent().Get("CableType").toString().isEmpty()) ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("CableType").toString());
            lValues += ",";
            lColumns += "CABLE_ID,";
            lValues += (mpDbKeysEngine->dbKeysContent().Get("Cable").toString().isEmpty()) ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Cable").toString());
            lValues += ",";
            lColumns += "CONTACTOR_TYP,";
            lValues += (mpDbKeysEngine->dbKeysContent().Get("ContactorType").toString().isEmpty()) ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ContactorType").toString());
            lValues += ",";
            lColumns += "CONTACTOR_ID,";
            lValues += (mpDbKeysEngine->dbKeysContent().Get("Contactor").toString().isEmpty()) ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Contactor").toString());
            lValues += ",";
            lColumns += "LASER_TYP,";
            lValues += (mpDbKeysEngine->dbKeysContent().Get("LaserType").toString().isEmpty()) ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("LaserType").toString());
            lValues += ",";
            lColumns += "LASER_ID,";
            lValues += (mpDbKeysEngine->dbKeysContent().Get("Laser").toString().isEmpty()) ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("Laser").toString());
            lValues += ",";
            lColumns += "EXTRA_TYP,";
            lValues += mpDbKeysEngine->dbKeysContent().Get("ExtraType").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ExtraType").toString());
            lValues += ",";
            lColumns += "EXTRA_ID,";
            lValues += mpDbKeysEngine->dbKeysContent().Get("ExtraName").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ExtraName").toString());
            lValues += ",";
        }
        lColumns += "FILE_HOST_ID,";
        lValues += "null";
        lValues += ",";
        lColumns += "FILE_PATH,";
        lValues += TranslateStringToSqlVarChar(strFilePath);
        lValues += ",";
        lColumns += "FILE_NAME,";
        lValues += TranslateStringToSqlVarChar(strFileName);
        lValues += ",";
        lColumns += "VALID_SPLITLOT,";
        lValues += TranslateStringToSqlVarChar((QChar)FLAG_SPLITLOT_INVALID);
        lValues += ",";
        lColumns += "INSERTION_TIME,";
        lValues += "UNIX_TIMESTAMP(now())";
        lValues += ",";
        lColumns += "SUBCON_LOT_ID,";
        lValues += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("SubconLot").toString());
        lValues += ",";
        lColumns += "INCREMENTAL_UPDATE,";
        lValues += "null";
        lValues += ",";
        lColumns += "SYA_ID,";
        lValues += "null";
        lValues += ",";

        QDateTime clDateTime;
        clDateTime.setTime_t(nStartTime);
        clDateTime.setTimeSpec(Qt::UTC);
        // Use SQL function if available
        lColumns += "DAY,";
        lValues += TranslateUnixTimeStampToSqlDateTime(QString::number(nStartTime),eDate);
        lValues += ",";
        lColumns += "WEEK_NB,";
        lValues += TranslateUnixTimeStampToSqlDateTime(QString::number(nStartTime),eWeek);
        lValues += ",";
        lColumns += "MONTH_NB,";
        lValues += TranslateUnixTimeStampToSqlDateTime(QString::number(nStartTime),eMonth);
        lValues += ",";
        lColumns += "QUARTER_NB,";
        lValues += TranslateUnixTimeStampToSqlDateTime(QString::number(nStartTime),eQuarter);
        lValues += ",";
        lColumns += "YEAR_NB,";
        lValues += TranslateUnixTimeStampToSqlDateTime(QString::number(nStartTime),eYear);
        lValues += ",";
        lColumns += "YEAR_AND_WEEK,";
        lValues += TranslateUnixTimeStampToSqlDateTime(QString::number(nStartTime),eWYear,eWeek);
        lValues += ",";
        lColumns += "YEAR_AND_MONTH,";
        lValues += TranslateUnixTimeStampToSqlDateTime(QString::number(nStartTime),eYear,eMonth);
        lValues += ",";
        lColumns += "YEAR_AND_QUARTER";
        lValues += TranslateUnixTimeStampToSqlDateTime(QString::number(nStartTime),eYear,eQuarter);

        if(m_eTestingStage != eFinalTest)
        {
            lColumns += ",WAFER_NB";
            lValues += ",";
            bool bOk;
            int nWaferNb = m_strWaferId.trimmed().toInt(&bOk);
            if(!bOk)
                nWaferNb=-1;
            if(m_eTestingStage == eWaferTest)
            {
                // At WT, check if WaferNb has been overloaded through KeyContent
                if(!mpDbKeysEngine->dbKeysContent().Get("WaferNb").toString().isEmpty())
                {
                    int nWaferNbFromKey = mpDbKeysEngine->dbKeysContent().Get("WaferNb").toString().trimmed().toInt(&bOk);
                    if(bOk)
                        nWaferNb = nWaferNbFromKey;
                }
            }
            lValues += (nWaferNb < 0) ? "null" : QString::number(nWaferNb);
        }
        if(m_eTestingStage == eElectTest)
        {
            lColumns += ",SITE_CONFIG";
            lValues += ",";
            lValues += mpDbKeysEngine->dbKeysContent().Get("EtestSiteConfig").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("EtestSiteConfig").toString());
        }

        strQuery = "INSERT INTO " + strTableName + "(" + lColumns + ")" + "VALUES(" + lValues + ")";
        // Execute insertion query
        if(!InsertSplitlotDataIntoGexdb(strTableName, strQuery))
        {
            WriteDebugMessageFile(QString("---- GexDbPlugin_Galaxy::UpdateSplitLotTable: InsertSplitlotDataIntoGexdb failed !"));
            bStatus = false;
            goto labelUnlock;
        }
    }
    else
    {
        // MULTI INSERTION MULTI SQL_THREAD
        // insert new entry and keep the temporary splitlot_id entry

        QString strTableColumns;
        // Check if have columns definition for this table
        if(!m_mapTablesDesc.contains(QString(strTableName+".columns").toUpper()))
        {
            if(m_pclDatabaseConnector->IsMySqlDB())
                strQuery = "SELECT COLUMN_NAME FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='"+m_pclDatabaseConnector->m_strSchemaName
                        +"' AND TABLE_NAME='"+m_strPrefixTable.toLower()+"_splitlot' ORDER BY ORDINAL_POSITION";
            else if(m_pclDatabaseConnector->IsOracleDB())
                strQuery = "SELECT COLUMN_NAME FROM USER_TAB_COLUMNS WHERE lower(TABLE_NAME)='"+m_strPrefixTable.toLower()+"_splitlot'  ORDER BY COLUMN_ID";
            if(!clGexDbQuery.Execute(strQuery))
            {
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                bStatus = false;
                goto labelUnlock;
            }
            strTableColumns = "";
            while(clGexDbQuery.Next())
            {
                if(!strTableColumns.isEmpty())
                    strTableColumns += ",";
                strTableColumns += clGexDbQuery.value(0).toString();
            }
            m_mapTablesDesc[QString(strTableName+".columns").toUpper()] = strTableColumns.toUpper();
        }

        // Get columns definition
        strTableColumns = m_mapTablesDesc[QString(strTableName+".columns").toUpper()];

        // Use AUTO_INCREMENT for splitlot_id
        if(m_pclDatabaseConnector->IsMySqlDB())
            strTableColumns = strTableColumns.replace("SPLITLOT_ID","null",Qt::CaseInsensitive);
        else if(m_pclDatabaseConnector->IsOracleDB())
            strTableColumns = strTableColumns.replace("SPLITLOT_ID",m_strPrefixTable+"_SPLITLOT_SEQUENCE.nextval",Qt::CaseInsensitive);

        strQuery = "INSERT INTO " + strTableName;
        strQuery+= " (SELECT ";
        strQuery+= strTableColumns;
        strQuery+= " FROM "+strTableName+" T WHERE T.SPLITLOT_ID="+QString::number(m_nTemporarySplitLotId)+")";

        if(!clGexDbQuery.Execute(strQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }

        // Retrieve the current AUTO_INCREMENT
        if(m_pclDatabaseConnector->IsMySqlDB())
            strQuery = "SELECT LAST_INSERT_ID()";
        else if(m_pclDatabaseConnector->IsOracleDB())
            strQuery = "SELECT "+NormalizeTableName("_SPLITLOT_SEQUENCE")+".currval FROM DUAL";
        if(!clGexDbQuery.Execute(strQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }
        clGexDbQuery.First();
        m_nSplitLotId = clGexDbQuery.value(0).toLongLong();

        // Debug message
        strMessage =  "Updating row in " + strTableName;
        strMessage += " with new splitlot_id (" + QString::number(m_nTemporarySplitLotId);
        strMessage += " -> " + QString::number(m_nSplitLotId) + ")";
        WriteDebugMessageFile(strMessage);
    }

    m_nlInsertedSplitlots.append(m_nSplitLotId);

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::StartSplitlotInsertion completed");

labelUnlock:

    // Commit and unlock splitlot table
    UnlockTables();

    return bStatus;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : StopSplitlotInsertion
///////////////////////////////////////////////////////////
// When updating to VALID_SPLITLOT, delete the temporary splitlot_id
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::StopSplitlotInsertion()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Stop Splitlot Insertion");

    QString             strTableName;
    QString             strQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    QStringList lIncrementalUpdates;
    QString consolidationEnabledString;
    QString agentEnabledString;

    if(GetGlobalOptionValue(eBinningConsolidationProcess, consolidationEnabledString) && (consolidationEnabledString == "TRUE"))
    {
        lIncrementalUpdates.append("BINNING_CONSOLIDATION");
    }
    if(GetGlobalOptionValue(eExternalServicesAgentWorkflowEnabled, agentEnabledString) && agentEnabledString == "TRUE")
    {
        lIncrementalUpdates.append("AGENT_WORKFLOW");
    }

    // Debug message
    WriteDebugMessageFile(QString("---- GexDbPlugin_Galaxy::StopSplitlotInsertion: update VALID_SPLITLOT flag"));

    strTableName = NormalizeTableName("_SPLITLOT");

    int lOverwrittenSplitlotID = -1;

    if (mpDbKeysEngine && mpDbKeysEngine->dbKeysContent().Get("OverwrittenSplitlotID").isNull() == false)
        lOverwrittenSplitlotID = mpDbKeysEngine->dbKeysContent().Get("OverwrittenSplitlotID").toInt();

    // Validate current insertion
    if (lOverwrittenSplitlotID == -1)
    {
        // If no splitlot is overwritten, make the inserted splitlot valid
        strQuery = "UPDATE " + strTableName + " SET ";
        strQuery += "VALID_SPLITLOT=";
        strQuery += TranslateStringToSqlVarChar((QChar)FLAG_SPLITLOT_VALID);
        strQuery += ",INCREMENTAL_UPDATE=" + (lIncrementalUpdates.length() == 0 ? "NULL" : TranslateStringToSqlVarChar(lIncrementalUpdates.join("|")));
        strQuery += " WHERE SPLITLOT_ID=";
        strQuery += QString::number(m_nSplitLotId);
    }
    else
    {
        // If a previous splitlot is overwritten
        //    - make the inserted splitlot valid
        //    - make the previous splitlot invalid
        // Both are made in a single query.
        strQuery = "UPDATE " + strTableName + " SET ";
        strQuery += "VALID_SPLITLOT=";
        strQuery += " CASE WHEN SPLITLOT_ID = ";
        strQuery += QString::number(m_nSplitLotId);
        strQuery += " THEN ";
        strQuery += TranslateStringToSqlVarChar((QChar)FLAG_SPLITLOT_VALID);
        strQuery += " ELSE ";
        strQuery += TranslateStringToSqlVarChar((QChar)FLAG_SPLITLOT_INVALID);
        strQuery += " END";
        strQuery += ",INCREMENTAL_UPDATE=";
        strQuery += " CASE WHEN SPLITLOT_ID = ";
        strQuery += QString::number(m_nSplitLotId);
        strQuery += " THEN " + (lIncrementalUpdates.length() == 0 ? "NULL" : TranslateStringToSqlVarChar(lIncrementalUpdates.join("|")));
        strQuery += " ELSE null";
        strQuery += " END";
        strQuery += " WHERE SPLITLOT_ID=";
        strQuery += QString::number(m_nSplitLotId);
        strQuery += " OR SPLITLOT_ID=";
        strQuery += QString::number(lOverwrittenSplitlotID);
    }

    if(!clGexDbQuery.Execute(strQuery))
    {
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Commit current transaction
    // before to delete temporary insertion
    // Commit and unlock splitlot table
    UnlockTables();

    // Delete temporary insertion
    strQuery = "DELETE FROM " + strTableName;
    strQuery += " WHERE SPLITLOT_ID=";
    strQuery += QString::number(m_nTemporarySplitLotId);
    clGexDbQuery.Execute(strQuery);

    return true;
}

bool GexDbPlugin_Galaxy::PreprocessingProcedure(int lSplitlotId)
{
    QString lProcedureName  = NormalizeTableName("_insertion_preprocessing");
    QString lMessage;
    int     lStatus         = 1;


    // Debug message
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("calling %1 stored procedure...").arg(lProcedureName).toLatin1().data());

    if(m_pclDatabaseConnector->IsMySqlDB())
    {
        QString             lQuery;
        GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

        TimersTraceStart("Procedure "+ lProcedureName);

        lQuery = "CALL " + lProcedureName;
        lQuery += "(" + QString::number(lSplitlotId) + ",";
        lQuery += "@message, @status)";

        // Prepare query
        if (clGexDbQuery.Execute(lQuery))
        {
            lQuery = "select @message, @status";
            if(!clGexDbQuery.Execute(lQuery))
            {
                TimersTraceStop("Procedure "+lProcedureName);

                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }

            if (clGexDbQuery.First())
            {
                // Retrieve parameter values
                lMessage = clGexDbQuery.value(0).toString();        // the returned message
                lStatus  = clGexDbQuery.value(1).toInt();              // nStatus is the return status: 0 is NOK, 1 is OK, 2 delay insertion
            }
            else
            {
                lMessage = "Unable to retrieve out variables from procedure" + lProcedureName;
                lStatus  = 2;
            }
        }
        else
        {
            TimersTraceStop("Procedure "+lProcedureName);

            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        TimersTraceStop("Procedure "+lProcedureName);
    }
    else
    {
        lStatus     = 0;
        lMessage    = "Unsupported Database " +
                m_pclDatabaseConnector->GetFriendlyName(m_pclDatabaseConnector->m_strDriver);
    }

    if(lStatus == 0)
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_InsertionPreProcessingProcedure, NULL,
                    lProcedureName.toLatin1().constData(),
                    QString("Status[%1] Message[%2]").arg(lStatus).arg(lMessage).toLatin1().constData());
        // Reject the current insertion file
        m_bStopProcess = true;
        return false;
    }
    if(lStatus == 2)
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_InsertionPreProcessingProcedure, NULL,
                    lProcedureName.toLatin1().constData(),
                    QString("Status[%1] Message[%2]").arg(lStatus).arg(lMessage).toLatin1().constData());
        m_bStopProcess = *m_pbDelayInsertion = true;
        return false;
    }

    // Check if have a warning to display
    if(!lMessage.isEmpty() && lMessage!="Success")
        WarningMessage(QString("Call Procedure %1 - Status[%2] Message[%3]").arg(lProcedureName).arg(lStatus).arg(lMessage));

    return true;
}

//////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::KeysContentDumpToTempTable()
{
    bool bStatus = false;

    GexDbPlugin_Query       clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString                 strQuery;
    QString                 strQueryHeader;  // INSERT INTO ...
    QString                 strNewValue;     // NEW VALUE FOR INSERTION
    QString                 strAllValues;    // MULTI QUERIES
    QMap<QString, QVariant> lDbKeys;
    QString                 lKey, lContent;

    // Debug message
    WriteDebugMessageFile("     Export GexDbKeys into the temporary table 'keyscontent'");

    // Make available All Keys to Replace with AIMS results (GexKeysContent)
    // CREATE TEMPORARY TABLE
    // Check if TmpTable exist for this session
    QString strTableName = NormalizeTableName("keyscontent",false);

    // gcore-1677: EnumTables() does not list temporary tables
    // => Systematically try to drop the table
    clGexDbQuery.Execute("DROP TABLE IF EXISTS "+strTableName);

    if( m_pclDatabaseConnector->IsMySqlDB() )
    {
        strQuery = "CREATE TEMPORARY TABLE "+strTableName+" (";
        //strQuery = "CREATE TABLE "+strTableName+" (";
        strQuery+= " KEY_NAME VARCHAR(255) ,";
        strQuery+= " KEY_VALUE TEXT";
        strQuery+= ")";
        if(!clGexDbQuery.Execute(strQuery))
        {
            if(clGexDbQuery.lastError().number() == 1050)
            {
                // Table already exists
                // Just truncate
                clGexDbQuery.exec("TRUNCATE TABLE "+strTableName);
            }
            else
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }
        }
    }
    else
    {
        strQuery = "CREATE GLOBAL TEMPORARY TABLE "+strTableName+"  (";
        strQuery+= " KEY_NAME VARCHAR2(255) ,";
        strQuery+= " KEY_VALUE CLOB";
        strQuery+= ") ON COMMIT PRESERVE ROWS";

        if(!clGexDbQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
    }

    // If have an Error, need to update the KeysContent
    if(m_bStopProcess)
    {
        QString lError = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        QString lErrorCode = "TDR-"+QString::number(GGET_LASTERRORCODE(GexDbPlugin_Base, this));
        QString lErrorDescription = GGET_LASTERRORDESC(GexDbPlugin_Base, this);

        mpDbKeysEngine->dbKeysContent().SetInternal("Error",lError);
        mpDbKeysEngine->dbKeysContent().SetInternal("ErrorCode",lErrorCode);
        mpDbKeysEngine->dbKeysContent().SetInternal("ErrorShortDesc",lErrorDescription);
    }

    // Then update all GexDbKeys content
    // gcore-1666: add bool parameter to specify we want to keep empty keys
    lDbKeys = mpDbKeysEngine->dbKeysContent().toMap(true);
    // Allow to overload WaferNb
    if(!lDbKeys.contains("wafernb"))
        lDbKeys["wafernb"] = "";

    strQueryHeader = "INSERT INTO " + strTableName + " VALUES";
    strNewValue = strAllValues = "";

    foreach (lKey, lDbKeys.keys())
    {
        lContent = lDbKeys[lKey].toString();

        if(lKey.endsWith("overloaded"))
            continue;

        // Insert new values
        strNewValue = "(";
        strNewValue += TranslateStringToSqlVarChar(lKey);
        strNewValue += ", " + TranslateStringToSqlVarChar(lContent);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
            goto labelUnlock;
    }
    if(!strAllValues.isEmpty() && (!ExecuteMultiInsertQuery(strTableName, strAllValues)))
        goto labelUnlock;
    bStatus = true;

labelUnlock:
    // Commit and unlock splitlot table
    UnlockTables();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::KeysContentUpdateFromTempTable(int SplitlotId)
{
    bool bStatus = false;

    QString                 strQuery, strNewValues;
    GexDbPlugin_Query       clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    // gcore-1666: add bool parameter to specify we want to keep empty keys
    QMap<QString, QVariant> lDbKeys = mpDbKeysEngine->dbKeysContent().toMap(true);
    QStringList             lDbKeysUpdated;
    QString                 lKey, lContent;
    QString                 lColumn, lType;

    // Debug message
    WriteDebugMessageFile("     Import GexDbKeys from the temporary table 'keyscontent'");

    // Make available All Keys to Replace with AIMS results (GexKeysContent)
    // Check if TmpTable exist for this session
    QString strTableName = NormalizeTableName("keyscontent",false);

    // gcore-1677: EnumTables() does not list temporary tables
    // directly try the select statement, do not check if the table exists.
    // if the select fails, there is an error anyway (as the table should exist), and we have to log it!
    strQuery = "SELECT KEY_NAME, KEY_VALUE FROM "+strTableName;
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1000).toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    else
    {
        while(clGexDbQuery.Next())
        {
            lKey = clGexDbQuery.value(0).toString().toLower();
            lContent = clGexDbQuery.value(1).toString();

            // Allow to overload WaferNb
            if(!lDbKeys.contains("wafernb") && lContent.isEmpty())
                continue;

            if(!lDbKeys.contains(lKey) // New entry
                    || (lContent != mpDbKeysEngine->dbKeysContent().Get(lKey).toString())) // Updated entry
            {
                lDbKeys[lKey]=lContent;
                if(mpDbKeysEngine->dbKeysContent().allowedStaticDbKeys().contains(lKey))
                    lDbKeysUpdated << lKey;
                if(!mpDbKeysEngine->dbKeysContent().Set(lKey, QVariant(lContent)))
                {
                    GSET_ERROR3(GexDbPlugin_Base, eValidation_DbKeyOverload, NULL, lKey.toLatin1().constData(),
                                lContent.toLatin1().constData(),
                                "invalid value or not allowed");
                    goto labelUnlock;
                }
            }
        }
    }

    if(SplitlotId>0)
    {
        // For each Static Keys associated with the splitlot table
        // Update the splitlot table
        strQuery = strNewValues = "";
        foreach(const QString &lKeyUpdate, lDbKeysUpdated)
        {
            lColumn = lType = "";
            lColumn = KeysContentToSplitlotColumn(lKeyUpdate);
            if(lColumn.isEmpty())
                continue;

            lContent = mpDbKeysEngine->dbKeysContent().Get(lKeyUpdate).toString();
            if(!strNewValues.isEmpty())
                strNewValues += ", ";

            lType = "STRING";
            if(mpDbKeysEngine->dbKeysContent().allowedStaticDbKeys().contains(lKeyUpdate.toLower()))
                lType = mpDbKeysEngine->dbKeysContent().DataKeysDefinition().GetDataKeysData(lKeyUpdate.toLower()).GetDataType();

            strNewValues += lColumn + "=";
            if((lType == "NUMBER") || (lType == "TIMESTAMP"))
                strNewValues += lContent;
            else
            {
                if(lType == "BOOLEAN")
                {
                    lContent = "N";
                    if(mpDbKeysEngine->dbKeysContent().Get(lKeyUpdate).toBool())
                        lContent = "Y";
                }
                else if(lType == "CHAR")
                {
                    if(lContent.isEmpty())
                        lContent = " ";
                    lContent = lContent.left(1);
                }

                strNewValues += TranslateStringToSqlVarChar(lContent);
            }

            if(lKeyUpdate == "starttime")
            {
                // Use SQL function if available
                strNewValues += ",";
                // DAY
                strNewValues += "DAY="+TranslateUnixTimeStampToSqlDateTime(lContent,eDate);
                strNewValues += ",";
                // WEEK_NB
                strNewValues += "WEEK_NB="+TranslateUnixTimeStampToSqlDateTime(lContent,eWeek);
                strNewValues += ",";
                // MONTH_NB
                strNewValues += "MONTH_NB="+TranslateUnixTimeStampToSqlDateTime(lContent,eMonth);
                strNewValues += ",";
                // QUARTER_NB
                strNewValues += "QUARTER_NB="+TranslateUnixTimeStampToSqlDateTime(lContent,eQuarter);
                strNewValues += ",";
                // YEAR_NB
                strNewValues += "YEAR_NB="+TranslateUnixTimeStampToSqlDateTime(lContent,eYear);
                strNewValues += ",";
                // YEAR_AND_WEEK
                strNewValues += "YEAR_AND_WEEK="+TranslateUnixTimeStampToSqlDateTime(lContent,eWYear,eWeek);
                strNewValues += ",";
                // YEAR_AND_MONTH
                strNewValues += "YEAR_AND_MONTH="+TranslateUnixTimeStampToSqlDateTime(lContent,eYear,eMonth);
                strNewValues += ",";
                // YEAR_AND_QUARTER
                strNewValues += "YEAR_AND_QUARTER="+TranslateUnixTimeStampToSqlDateTime(lContent,eYear,eQuarter);

            }
        }
        if(!strNewValues.isEmpty())
        {
            strQuery = "UPDATE " + NormalizeTableName("_SPLITLOT") + " SET " + strNewValues;
            strQuery+= " WHERE splitlot_id=" + QString::number(SplitlotId);
            if(!clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }
        }
    }
    bStatus = true;

labelUnlock:
    // Commit and unlock splitlot table
    UnlockTables();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::KeysContentToSplitlotColumn(const QString &lKey)
{
    QMap<QString,QString> lKeyToSplitlotColumn;
    lKeyToSplitlotColumn[QString("DataOrigin").toLower()]       ="DATA_PROVIDER";
    lKeyToSplitlotColumn[QString("ExecType").toLower()]         ="EXEC_TYP";
    lKeyToSplitlotColumn[QString("ExecVersion").toLower()]      ="EXEC_VER";
    lKeyToSplitlotColumn[QString("Facility").toLower()]         ="FACIL_ID";
    lKeyToSplitlotColumn[QString("Family").toLower()]           ="FAMLY_ID";
    lKeyToSplitlotColumn[QString("FinishTime").toLower()]       ="FINISH_T";
    lKeyToSplitlotColumn[QString("Lot").toLower()]              ="LOT_ID";
    lKeyToSplitlotColumn[QString("Operator").toLower()]         ="OPER_NAM";
    lKeyToSplitlotColumn[QString("Process").toLower()]          ="PROC_ID";
    lKeyToSplitlotColumn[QString("ProdData").toLower()]         ="PROD_DATA";
    lKeyToSplitlotColumn[QString("Product").toLower()]          ="PRODUCT_NAME";
    lKeyToSplitlotColumn[QString("ProgramName").toLower()]      ="JOB_NAM";
    lKeyToSplitlotColumn[QString("ProgramRevision").toLower()]  ="JOB_REV";
    lKeyToSplitlotColumn[QString("StartTime").toLower()]        ="START_T";
    lKeyToSplitlotColumn[QString("SubconLot").toLower()]        ="SUBCON_LOT_ID";
    lKeyToSplitlotColumn[QString("SubLot").toLower()]           ="SUBLOT_ID";
    lKeyToSplitlotColumn[QString("TesterName").toLower()]       ="TESTER_NAME";
    lKeyToSplitlotColumn[QString("TesterType").toLower()]       ="TESTER_TYPE";
    lKeyToSplitlotColumn[QString("UserText").toLower()]         ="USER_TXT";
    lKeyToSplitlotColumn[QString("TestInsertion").toLower()]    ="TEST_INSERTION";
    lKeyToSplitlotColumn[QString("TestFlow").toLower()]         ="TEST_FLOW";
    lKeyToSplitlotColumn[QString("RetestBinList").toLower()]    ="RETEST_HBINS";
    lKeyToSplitlotColumn[QString("RetestIndex").toLower()]      ="RETEST_INDEX";

    if(m_eTestingStage == eElectTest)
    {
        lKeyToSplitlotColumn[QString("DataType").toLower()]       ="DATA_TYPE";
        lKeyToSplitlotColumn[QString("EtestSiteConfig").toLower()]="SITE_CONFIG";
    }
    else
    {
        lKeyToSplitlotColumn[QString("AuxiliaryFile").toLower()]="AUX_FILE";
        lKeyToSplitlotColumn[QString("BurninTime").toLower()]   ="BURN_TIM";
        lKeyToSplitlotColumn[QString("Cable").toLower()]        ="CABLE_ID";
        lKeyToSplitlotColumn[QString("CableType").toLower()]    ="CABLE_TYP";
        lKeyToSplitlotColumn[QString("Card").toLower()]         ="CARD_ID";
        lKeyToSplitlotColumn[QString("CardType").toLower()]     ="CARD_TYP";
        lKeyToSplitlotColumn[QString("CommandModeCode").toLower()]="CMOD_COD";
        lKeyToSplitlotColumn[QString("Contactor").toLower()]    ="CONTACTOR_ID";
        lKeyToSplitlotColumn[QString("ContactorType").toLower()]="CONTACTOR_TYP";
        lKeyToSplitlotColumn[QString("DataType").toLower()]     ="DATA_TYPE";//,MODE_CODE|CHAR";
        lKeyToSplitlotColumn[QString("DateCode").toLower()]     ="DATE_COD";
        lKeyToSplitlotColumn[QString("DesignRevision").toLower()]="DSGN_REV";
        lKeyToSplitlotColumn[QString("DibName").toLower()]      ="DIB_ID";
        lKeyToSplitlotColumn[QString("DibType").toLower()]      ="DIB_TYP";
        lKeyToSplitlotColumn[QString("Engineering").toLower()]  ="ENG_ID";
        lKeyToSplitlotColumn[QString("ExtraName").toLower()]    ="EXTRA_ID";
        lKeyToSplitlotColumn[QString("ExtraType").toLower()]    ="EXTRA_TYP";
        lKeyToSplitlotColumn[QString("Floor").toLower()]        ="FLOOR_ID";
        lKeyToSplitlotColumn[QString("Flow").toLower()]         ="FLOW_ID";
        lKeyToSplitlotColumn[QString("FrequencyStep").toLower()]="OPER_FRQ";
        lKeyToSplitlotColumn[QString("HeadNumber").toLower()]   ="HEAD_NUM";
        lKeyToSplitlotColumn[QString("Laser").toLower()]        ="LASER_ID";
        lKeyToSplitlotColumn[QString("LaserType").toLower()]    ="LASER_TYP";
        lKeyToSplitlotColumn[QString("LoadBoard").toLower()]    ="LOADBOARD_ID";
        lKeyToSplitlotColumn[QString("LoadBoardType").toLower()]="LOADBOARD_TYP";
        lKeyToSplitlotColumn[QString("PackageType").toLower()]  ="PKG_TYP";
        lKeyToSplitlotColumn[QString("ProberName").toLower()]   ="HANDLER_ID";
        lKeyToSplitlotColumn[QString("ProberType").toLower()]   ="HANDLER_TYP";
        lKeyToSplitlotColumn[QString("ProtectionCode").toLower()]="PROT_COD";
        lKeyToSplitlotColumn[QString("RetestCode").toLower()]   ="RTST_COD";
        lKeyToSplitlotColumn[QString("RomCode").toLower()]      ="ROM_COD";
        lKeyToSplitlotColumn[QString("SerialNumber").toLower()] ="SERL_NUM";
        lKeyToSplitlotColumn[QString("Setup").toLower()]        ="SETUP_ID";
        lKeyToSplitlotColumn[QString("SetupTime").toLower()]    ="SETUP_T";
        lKeyToSplitlotColumn[QString("SpecName").toLower()]     ="SPEC_NAM";
        lKeyToSplitlotColumn[QString("SpecVersion").toLower()]  ="SPEC_VER";
        lKeyToSplitlotColumn[QString("Station").toLower()]      ="STAT_NUM";
        lKeyToSplitlotColumn[QString("SupervisorName").toLower()]="SUPR_NAM";
        lKeyToSplitlotColumn[QString("Temperature").toLower()]  ="TST_TEMP";
        lKeyToSplitlotColumn[QString("TestingCode").toLower()]  ="TEST_COD";
    }

    if(m_eTestingStage != eFinalTest)
    {
        lKeyToSplitlotColumn[QString("Wafer").toLower()]        ="WAFER_ID";
        lKeyToSplitlotColumn[QString("WaferNb").toLower()]      ="WAFER_NB";
    }

    if(lKeyToSplitlotColumn.contains(lKey.toLower()))
        return lKeyToSplitlotColumn[lKey.toLower()];
    return "";
}

//////////////////////////////////////////////////////////////////////
// Insert an alarm for specified splitlot (waferv testing stage)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::InsertAlarm_Wafer(long lSplitlotID, GexDbPlugin_Base::AlarmCategories eAlarmCat, GexDbPlugin_Base::AlarmLevels eAlarmLevel, long lItemNumber, QString & strItemName, unsigned int uiFlags, float fLCL, float fUCL, float fValue, QString strUnits)
{
    return InsertAlarm(lSplitlotID, eWaferTest, eAlarmCat, eAlarmLevel, lItemNumber, strItemName, uiFlags, fLCL, fUCL, fValue, strUnits);
}

//////////////////////////////////////////////////////////////////////
// Insert an alarm for specified splitlot/testing stage
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::InsertAlarm(long lSplitlotID, int nTestingStage,
                                     GexDbPlugin_Base::AlarmCategories eAlarmCat,
                                     GexDbPlugin_Base::AlarmLevels eAlarmLevel,
                                     long lItemNumber,
                                     QString & strItemName, unsigned int uiFlags, float fLCL, float fUCL, float fValue, QString strUnits)
{
    GSLOG(SYSLOG_SEV_ERROR, (QString("Insert Alarm splitlot %1d TS %2")
                             .arg( lSplitlotID)
                             .arg(nTestingStage)).toLatin1().constData());

    SetTestingStage(nTestingStage);

    // Execute query to add the alarm to the table
    QString             strAlarmCat, strAlarmType, strQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Get alarm cat and alarm type
    switch(eAlarmCat)
    {
    case eAlarmCat_Parametric_Value:        strAlarmCat = "Parametric (Value)"; break;
    case eAlarmCat_Parametric_Cp:           strAlarmCat = "Parametric (Cp)"; break;
    case eAlarmCat_Parametric_Cpk:          strAlarmCat = "Parametric (Cpk)"; break;
    case eAlarmCat_Parametric_Mean:         strAlarmCat = "Parametric (Mean)"; break;
    case eAlarmCat_Parametric_Range:        strAlarmCat = "Parametric (Range)"; break;
    case eAlarmCat_Parametric_Sigma:        strAlarmCat = "Parametric (Sigma)"; break;
    case eAlarmCat_Parametric_PassRate:     strAlarmCat = "Parametric (Pass rate)"; break;
    case eAlarmCat_Yield:                   strAlarmCat = "Yield"; break;
    case eAlarmCat_PatYieldLoss_Yield:      strAlarmCat = "PatYieldLoss (Yield)"; break;
    case eAlarmCat_PatYieldLoss_Parts:      strAlarmCat = "PatYieldLoss (Parts)"; break;
    case eAlarmCat_PatDistributionMismatch: strAlarmCat = "PatDistributionMismatch"; break;
    case eAlarmCat_SYA:                     strAlarmCat = "SYA";  break;
    case eAlarmCat_SYA_SBL:                 strAlarmCat = "SYA (SBL)";  break;
    case eAlarmCat_SYA_SYL:                 strAlarmCat = "SYA (SYL)";  break;
    case eAlarmCat_BinsPercentPerSite:      strAlarmCat = "BinsPercentPerSite";  break;
    default:
        strAlarmCat = "Unknown";
    }

    switch(eAlarmLevel)
    {
    case eAlarmLevel_Standard: strAlarmType = "Standard"; break;
    case eAlarmLevel_Critical: strAlarmType = "Critical"; break;
    case eAlarmLevel_Unknown: GSLOG(SYSLOG_SEV_ERROR, "unknown alarm level !"); break;
    }

    // GCore - 1732
    // Increment the lItemNumber if necessary and only when doesn't come from Parametric alarm
    bool bParametricAlarm = false;
    int lItemNo = lItemNumber;
    // Check if Parametric Alarm
    switch(eAlarmCat)
    {
    case eAlarmCat_Parametric_Value:
    case eAlarmCat_Parametric_Cp:
    case eAlarmCat_Parametric_Cpk:
    case eAlarmCat_Parametric_Mean:
    case eAlarmCat_Parametric_Range:
    case eAlarmCat_Parametric_Sigma:
    case eAlarmCat_Parametric_PassRate:
        bParametricAlarm = true;
        break;
    default:
        bParametricAlarm = false;
    }

    QString strTableName = NormalizeTableName("_PROD_ALARM");
    if(!bParametricAlarm)
    {
        // Retrieve the last ItemNo for this alarm
        strQuery = "SELECT MAX(item_no) FROM "+strTableName;
        strQuery += " WHERE splitlot_id=" + QString::number(lSplitlotID);
        strQuery += "   AND alarm_cat=" + TranslateStringToSqlVarChar(strAlarmCat, true);
        strQuery += "   AND alarm_type=" + TranslateStringToSqlVarChar(strAlarmType, true);
        // execute query
        if(!clGexDbQuery.Execute(strQuery))
        {
            qDebug("query exec failed : %s\n %s", strQuery.replace('\n',' ').toLatin1().data(),
                   clGexDbQuery.lastError().text().toLatin1().data());
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        if(clGexDbQuery.First())
        {
            // Have an alarm for this splitlot and for this cat/type
            // Increment the item_no
            lItemNo =clGexDbQuery.value(0).toInt() + 1;
        }
    }

    strQuery = "INSERT INTO "+strTableName;
    strQuery += " VALUES(" + QString::number(lSplitlotID);
    strQuery += "," + TranslateStringToSqlVarChar(strAlarmCat, true);
    strQuery += "," + TranslateStringToSqlVarChar(strAlarmType, true);
    strQuery += "," + QString::number(lItemNo);
    strQuery += "," + TranslateStringToSqlVarChar(strItemName, true);
    strQuery += "," + QString::number(uiFlags);
    strQuery += "," + NormalizeNumber(fLCL);
    strQuery += "," + NormalizeNumber(fUCL);
    strQuery += "," + NormalizeNumber(fValue);
    strQuery += "," + TranslateStringToSqlVarChar(strUnits, true);
    strQuery += ")";

    // execute query
    if(!clGexDbQuery.Execute(strQuery))
    {
        qDebug("query exec failed : %s\n %s", strQuery.replace('\n',' ').toLatin1().data(),
               clGexDbQuery.lastError().text().toLatin1().data());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Initialize Product table
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateProductTable(QString &ProductName)
{
    bool bStatus = true;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Update product table (%1)...").arg(ProductName).toLatin1().constData());

    if(ProductName.isEmpty())
        return true;

    // first verify if have already information into the table
    QString            lQuery;
    QString            lTableName = NormalizeTableName("PRODUCT",false);
    GexDbPlugin_Query  clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // MULTI INSERTION MULTI SQL_THREAD
    // lock table on product name before selection and insertion
    // Oracle transaction
    // Lock the selected row
    if(m_pclDatabaseConnector->IsMySqlDB())
        bStatus = GetLock("ConsolidationProcess|"+ProductName);
    else
        bStatus = LockTables(lTableName,false);
    if(!bStatus)
        goto labelUnlock;

    lQuery = "SELECT PRODUCT_NAME FROM "+lTableName+" WHERE PRODUCT_NAME=";
    lQuery += TranslateStringToSqlVarChar(ProductName);

    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        bStatus = false;
        goto labelUnlock;
    }
    if(!clGexDbQuery.First())
    {
        // have to insert new entry
        lQuery = "INSERT INTO "+lTableName+" VALUES(";
        lQuery += TranslateStringToSqlVarChar(ProductName);
        lQuery += ", '')";

        // execute query
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }
    }

labelUnlock:

    if(m_pclDatabaseConnector->IsMySqlDB())
        ReleaseLock();
    else
        UnlockTables();

    return bStatus;
}


///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateFtDieTrackingTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateFtDieTrackingTable()
{
    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MIR]==0)
        return false;

    if(m_mapDieTrackingInfo.isEmpty())
        return true;

    bool                bStatus;
    QString             strQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Loop through dies
    QString      strDieID, strMultiDieCommand, strWtProductID, strWtTrackingLotID, strWtSublotID;
    QString      strKeyword, strValue;
    QStringList  strlTokens;
    QString      strTableName = NormalizeTableName("_die_tracking", true);
    unsigned int uiIndex;

    QString strQueryHeader;   // INSERT INTO ...
    QString strNewValue;      // NEW VALUE FOR INSERTION
    QString strAllValues;     // MULTI QUERIES
    QMapIterator<QString, QString>        itMap(m_mapDieTrackingInfo);

    bStatus = true;

    // MULTI INSERTION MULTI SQL_THREAD
    // lock table on TrackingLotId before selection and insertion
    bStatus = LockTables(strTableName,false);
    if(!bStatus)
        goto labelUnlock;

    // Oracle transaction
    // Lock the inserted row
    strQueryHeader = "INSERT INTO " + strTableName + " VALUES";
    strNewValue = strAllValues = "";

    while(itMap.hasNext())
    {
        itMap.next();
        strDieID = itMap.key();
        strMultiDieCommand = itMap.value();
        strWtProductID = strWtTrackingLotID = strWtSublotID = "";

        // Parse MultiDie Info and extract values for individual fields
        // Syntax is: <cmd> die-tracking die=<die #>;wafer_product=<wafer ProductID>;wafer_lot=<wafer LotID>;wafer_sublot=<wafer SublotID>
        // Legacy syntax: <cmd> multi-die die=<die #>;wafer_product=<wafer ProductID>;wafer_lot=<wafer LotID>;wafer_sublot=<wafer SublotID>
        // Example: <cmd> die-tracking die=1;wafer_product=TNAP60DIZ;wafer_lot=Z08D020;wafer_sublot=Z08D020.2
        strlTokens = strMultiDieCommand.section(';',1).simplified().split(';');
        for(uiIndex=0; uiIndex< (UINT)strlTokens.count(); uiIndex++)
        {
            strKeyword = strlTokens[uiIndex].section('=',0,0).simplified().toLower();
            strValue = strlTokens[uiIndex].section('=',1,1).simplified();
            if(strKeyword == "wafer_product")
                strWtProductID = strValue;
            else if(strKeyword == "wafer_lot")
                strWtTrackingLotID = strValue;
            else if(strKeyword == "wafer_sublot")
                strWtSublotID = strValue;
            else
            {
                GSET_ERROR1(GexDbPlugin_Base, eStdf_DtrCommand_BadSyntax, NULL, strMultiDieCommand.toLatin1().constData());
                bStatus = false;
                goto labelUnlock;
            }
        }
        if(strWtTrackingLotID.isEmpty())
        {
            GSET_ERROR1(GexDbPlugin_Base, eStdf_DtrCommand_BadSyntax, NULL, strMultiDieCommand.toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }

        // Check if data for this final test lot already present in the table
        strQuery = "SELECT * FROM "+strTableName+" WHERE FT_TRACKING_LOT_ID=";
        strQuery += TranslateStringToSqlVarChar(m_strTrackingLotId);
        strQuery += " AND lower(DIE_ID)=";
        strQuery += TranslateStringToSqlVarChar(strDieID).toLower();
        strQuery += " AND lower(WT_TRACKING_LOT_ID)=";
        strQuery += TranslateStringToSqlVarChar(strWtTrackingLotID).toLower();
        if(!clGexDbQuery.Execute(strQuery))
        {
            if(m_pbDelayInsertion) *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }
        if(clGexDbQuery.First())
        {
            // If data already present, delete it
            ///////////////////////////////////////////////////////////
            strQuery = "DELETE FROM "+strTableName+" WHERE FT_TRACKING_LOT_ID=";
            strQuery += TranslateStringToSqlVarChar(m_strTrackingLotId);
            strQuery += " AND lower(DIE_ID)=";
            strQuery += TranslateStringToSqlVarChar(strDieID).toLower();
            strQuery += " AND lower(WT_TRACKING_LOT_ID)=";
            strQuery += TranslateStringToSqlVarChar(strWtTrackingLotID).toLower();
            if(!clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                bStatus = false;
                goto labelUnlock;
            }
        }

        // Insert new values
        strNewValue = "(";
        strNewValue += TranslateStringToSqlVarChar(m_strTrackingLotId);
        strNewValue += ", " + TranslateStringToSqlVarChar(strWtProductID);
        strNewValue += ", " + TranslateStringToSqlVarChar(strWtTrackingLotID);
        strNewValue += ", " + TranslateStringToSqlVarChar(strWtSublotID);
        strNewValue += ", " + TranslateStringToSqlVarChar(strDieID);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
        {
            if(m_pbDelayInsertion) *m_pbDelayInsertion = true;
            bStatus = false;
            goto labelUnlock;
        }
    }

    if(!strAllValues.isEmpty() && (!ExecuteMultiInsertQuery(strTableName, strAllValues)))
    {
        if(m_pbDelayInsertion) *m_pbDelayInsertion = true;
        bStatus = false;
        goto labelUnlock;
    }

labelUnlock:
    UnlockTables();

    return bStatus;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateFileHostTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateFileHostTable()
{
    bool bStatus = true;

    // Check if we have ftp settings
    if(mpDbKeysEngine->dbKeysContent().Get("UploadFile").toString() == "FALSE")
        return true;

    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateFileHostTable");

    // Construct path to file on upload ftp site depending on Ftp settings

    QString strFtpPath = mpDbKeysEngine->dbKeysContent().Get("FtpPath").toString();
    QString strTableName = NormalizeTableName("FILE_HOST",false);

    // first verify if have already information into the table
    QString             strQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // MULTI INSERTION MULTI SQL_THREAD
    // lock table before selection and insertion
    LockTables(strTableName,false);

    strQuery = "SELECT FILE_HOST_ID FROM "+strTableName+" WHERE lower(HOST_NAME)=";
    strQuery += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("FtpServer").toString()).toLower();
    strQuery += " AND HOST_FTPUSER=";
    strQuery += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("FtpUser").toString());
    strQuery += " AND (HOST_FTPPATH=";
    strQuery += TranslateStringToSqlVarChar(strFtpPath);
    if(strFtpPath.isEmpty())
        strQuery += " OR HOST_FTPPATH IS NULL";
    strQuery += " )";

    if(!clGexDbQuery.Execute(strQuery))
    {
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        bStatus = false;
        goto labelUnlock;
    }
    if(!clGexDbQuery.First())
    {
        // have to insert new entry
        // get the autoincrement value
        strQuery = "SELECT MAX(FILE_HOST_ID) FROM "+strTableName;
        if(!clGexDbQuery.Execute(strQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }
        if(!clGexDbQuery.First())
            m_nFileHostId = 1;
        else
        {
            if(clGexDbQuery.isNull(0))
                m_nFileHostId = 1;
            else
                m_nFileHostId = clGexDbQuery.value(0).toInt() + 1;
        }

        strQuery = "INSERT INTO "+strTableName+" VALUES(";
        strQuery += QString::number(m_nFileHostId);
        strQuery += ",";
        strQuery += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("FtpServer").toString());
        strQuery += ",";
        strQuery += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("FtpUser").toString());
        strQuery += ",";
        strQuery += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("FtpPassword").toString());
        strQuery += ",";
        strQuery += TranslateStringToSqlVarChar(strFtpPath);
        strQuery += ",";
        strQuery += QString::number(mpDbKeysEngine->dbKeysContent().Get("FtpPort").toInt());
        strQuery += ")";

        // execute query
        if(!clGexDbQuery.Execute(strQuery))
        {
            qDebug("GexDbPlugin_Galaxy::UpdateFileHostTable: error executing INSERT query !");
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }
    }
    else
    {
        // do not insert this ftp settings but get the autoincrement value
        m_nFileHostId = clGexDbQuery.value(0).toInt();
    }

    // Update splitlot table
    strQuery = "UPDATE " + NormalizeTableName("_SPLITLOT") + " SET ";
    strQuery += "FILE_HOST_ID=";
    strQuery += QString::number(m_nFileHostId);
    if(mpDbKeysEngine->dbKeysContent().Get("MoveFile").toString() == "TRUE")
    {
        strQuery += ",FILE_PATH=";
        strQuery += TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("MovePath").toString(), true);
    }
    strQuery += " WHERE SPLITLOT_ID=";
    strQuery += QString::number(m_nSplitLotId);

    if(!clGexDbQuery.Execute(strQuery))
    {
        qDebug("GexDbPlugin_Galaxy::UpdateFileHostTable: error executing UPDATE query !");
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        bStatus = false;
        goto labelUnlock;
    }

labelUnlock:

    // unlock table
    UnlockTables();

    return bStatus;
}


///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateSubLotInfoTable
// Call during the insertion process
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateSubLotInfoTable()
{
    bool bStatus = true;

    if(m_eTestingStage != eFinalTest)
        return true;

    if(m_pbDelayInsertion == NULL)
        return false;

    if(mpDbKeysEngine == NULL)
        return false;

    // Debug message
    QString lMessage = "---- GexDbPlugin_Galaxy::UpdateSubLotInfoTable()";
    lMessage += NormalizeTableName("_SPLITLOT");
    WriteDebugMessageFile(lMessage);

    QString              lTableName;
    QString              lQuery;
    GexDbPlugin_Query    clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    clGexDbQuery.setForwardOnly(true);
    lTableName = NormalizeTableName("_SUBLOT_INFO");


    // First, check if have this sublot already in the DB
    lQuery =  "SELECT LOT_ID FROM "+lTableName;
    lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(m_strLotId);
    lQuery += " AND SUBLOT_ID="+TranslateStringToSqlVarChar(m_strSubLotId);


    // MULTI INSERTION MULTI SQL_THREAD
    // Start a transaction for update WT_WAFER_INFO table
    // MySql
    // Start a virtual GETLOCK on LotWafer to insert rows if not exists
    // Oracle
    // Lock all the table
    if(m_pclDatabaseConnector->IsMySqlDB())
        bStatus = GetLock("ConsolidationProcess|"+m_strPrefixTable+"|"+m_strLotId+"|"+m_strSubLotId);
    else
        bStatus = LockTables(lTableName,false);
    if(!bStatus)
        goto labelUnlock;

    if(!clGexDbQuery.Execute(lQuery))
    {
        // The file must be delay and all data must be cleaned
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        bStatus = false;
        goto labelUnlock;
    }

    if(!clGexDbQuery.First())
    {
        int nFlags = 0;

        // insert new entry
        lQuery = "INSERT INTO " + lTableName + " VALUES(";
        // LOT_ID
        lQuery += TranslateStringToSqlVarChar(m_strLotId);
        lQuery += ",";
        // SUBLOT_ID
        lQuery += TranslateStringToSqlVarChar(m_strSubLotId);
        lQuery += ",";
        // PRODUCT_NAME
        lQuery += m_strProductName.isEmpty() ? "null" : TranslateStringToSqlVarChar(m_strProductName);
        lQuery += ",";
        // NB_PARTS
        lQuery += "0";
        lQuery += ",";
        // NB_PARTS_GOOD
        lQuery += "0";
        lQuery += ",";
        // SUBLOT_FLAGS
        lQuery += QString::number(nFlags);
        lQuery += ",";
        // CONSOLIDATION_ALGO
        lQuery += mpDbKeysEngine->dbKeysContent().Get("ConsolidationAlgo").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ConsolidationAlgo").toString());
        lQuery += ",";
        // CONSOLIDATION_STATUS
        lQuery += "null";
        lQuery += ",";
        // CONSOLIDATION_SUMMARY
        lQuery += "null";
        lQuery += ",";
        // CONSIOLIDATION_REF_DATE
        lQuery += "null";

        lQuery += ")";

        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }
    }
    else
    {
        // DB-67 - Overwrite alrady inserted WaferId+BADProduct
        // Overwrite the product name with the new one
        // The check was already done in the Validation process
        lQuery = "UPDATE " + lTableName + " SET PRODUCT_NAME=";
        lQuery += m_strProductName.isEmpty() ? "null" : TranslateStringToSqlVarChar(m_strProductName);
        lQuery += " WHERE LOT_ID=" + TranslateStringToSqlVarChar(m_strLotId);
        lQuery += " AND SUBLOT_ID=" + TranslateStringToSqlVarChar(m_strSubLotId);

        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }
    }

    // Debug message
    WriteDebugMessageFile("GexDbPlugin_Galaxy::UpdateSubLotInfoTable() : Completed");

labelUnlock:

    // unlock tables
    if(m_pclDatabaseConnector->IsMySqlDB())
        ReleaseLock();
    else
        UnlockTables();

    WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::UpdateSubLotInfoTable: end. status = %1").arg(bStatus?"true":"false"));
    return bStatus;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateLotTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateLotTable()
{
    // Debug message

    QString strMessage = "---- GexDbPlugin_Galaxy::UpdateLotTable()";
    WriteDebugMessageFile(strMessage);

    ///////////////////////////////////////////////////////////
    // FIRST INSERTION
    // Make sure we have some MIR information
    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MIR]==0)
        return false;

    QString strTableName;

    strTableName = NormalizeTableName("_LOT");

    QString            strQuery;
    GexDbPlugin_Query  clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);

    ///////////////////////////////////////////////////////////
    // Lot Table Insertion
    ///////////////////////////////////////////////////////////
    bool bStatus = true;

    strQuery = "SELECT TRACKING_LOT_ID, PRODUCT_NAME FROM "+strTableName;
    strQuery+=" WHERE LOT_ID="+TranslateStringToSqlVarChar(m_strLotId);
    strQuery+=" AND PRODUCT_NAME="+TranslateStringToSqlVarChar(m_strProductName);

    // MULTI INSERTION MULTI SQL_THREAD
    // Transaction active
    // lock table on LotId before selection and insertion
    // MySql
    // Start a virtual GETLOCK on Lot to insert rows if not exists
    // Oracle
    // Lock all the table
    if(m_pclDatabaseConnector->IsMySqlDB())
        bStatus = GetLock("ConsolidationProcess|"+m_strPrefixTable+"|"+m_strLotId);
    else
        bStatus = LockTables(strTableName,false);
    if(!bStatus)
        goto labelUnlock;


    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        bStatus=false;
        goto labelUnlock;
    }
    if(!clGexDbQuery.First())
    {
        // insert new entry
        strQuery = "INSERT INTO " + strTableName + " VALUES(";
        // LOT_ID
        strQuery += TranslateStringToSqlVarChar(m_strLotId);
        strQuery += ",";
        // TRACKING_LOT_ID
        strQuery += TranslateStringToSqlVarChar(m_strTrackingLotId);
        strQuery += ",";
        // PRODUCT_NAME
        strQuery += m_strProductName.isEmpty() ? "null" : TranslateStringToSqlVarChar(m_strProductName);
        strQuery += ",";
        // NB_PARTS
        strQuery += QString::number(0);
        strQuery += ",";
        // NB_PARTS_GOOD
        strQuery += QString::number(0);
        strQuery += ",";
        // SPLITLOT_FLAGS
        strQuery += QString::number(0);
        strQuery += ")";

        if(!clGexDbQuery.Execute(strQuery))
        {
            qDebug("GexDbPlugin_Galaxy::UpdateLotTable : error executing INSERT query !");
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus=false;
            goto labelUnlock;
        }
    }

    // Debug message
    strMessage = "     GexDbPlugin_Galaxy::UpdateLotTable() : Completed";
    WriteDebugMessageFile(strMessage);

labelUnlock:

    if(m_pclDatabaseConnector->IsMySqlDB())
        ReleaseLock();
    else
        UnlockTables();

    return bStatus;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateWaferInfoTable
// Call during the insertion process
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateWaferInfoTable()
{
    bool bStatus = true;

    if(m_eTestingStage == eFinalTest)
        return true;

    if(m_pbDelayInsertion == NULL)
        return false;

    if(mpDbKeysEngine == NULL)
        return false;

    // Debug message
    QString strMessage = "---- GexDbPlugin_Galaxy::UpdateWaferInfoTable()";
    strMessage += NormalizeTableName("_SPLITLOT");
    WriteDebugMessageFile(strMessage);

    QString              lTableName;
    QString              lQuery;
    GexDbPlugin_Query    clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    clGexDbQuery.setForwardOnly(true);
    lTableName = NormalizeTableName("_WAFER_INFO");


    // First, check if have this wafer already in the DB
    lQuery =  "SELECT LOT_ID FROM "+lTableName;
    lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(m_strLotId);
    if(m_strWaferId.isEmpty())
        lQuery += " AND WAFER_ID IS NULL";
    else
        lQuery += " AND WAFER_ID="+TranslateStringToSqlVarChar(m_strWaferId);


    // MULTI INSERTION MULTI SQL_THREAD
    // Start a transaction for update WT_WAFER_INFO table
    // MySql
    // Start a virtual GETLOCK on LotWafer to insert rows if not exists
    // Oracle
    // Lock all the table
    if(m_pclDatabaseConnector->IsMySqlDB())
        bStatus = GetLock("ConsolidationProcess|"+m_strPrefixTable+"|"+m_strLotId+"|"+m_strWaferId);
    else
        bStatus = LockTables(lTableName,false);
    if(!bStatus)
        goto labelUnlock;

    if(!clGexDbQuery.Execute(lQuery))
    {
        // The file must be delay and all data must be cleaned
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        bStatus = false;
        goto labelUnlock;
    }

    if(!clGexDbQuery.First())
    {
        int nFlags = 0;
        int nWaferNb = -1;
        int nNbGrossDie = 0;
        if(mpDbKeysEngine->dbKeysContent().Get("GrossDie").toInt() > 0)
        {
            nFlags = FLAG_WAFERINFO_GROSSDIEOVERLOADED;
            nNbGrossDie = mpDbKeysEngine->dbKeysContent().Get("GrossDie").toInt();
        }

        // insert new entry
        lQuery = "INSERT INTO " + lTableName ;
        QString lFields = "(";
        QString lValues = " VALUES(";
        lFields += "LOT_ID,";
        lValues += TranslateStringToSqlVarChar(m_strLotId);
        lValues += ",";

        lFields += "WAFER_ID,";
        lValues += m_strWaferId.isEmpty()                    ? "null" : TranslateStringToSqlVarChar(m_strWaferId);
        lValues += ",";

        lFields += "PRODUCT_NAME,";
        lValues += m_strProductName.isEmpty()                ? "null" : TranslateStringToSqlVarChar(m_strProductName);
        lValues += ",";

        lFields += "FAB_ID,";
        lValues += (m_clStdfWRR.m_cnFABWF_ID.isEmpty())      ? "null" : TranslateStringToSqlVarChar(m_clStdfWRR.m_cnFABWF_ID);
        lValues += ",";

        lFields += "FRAME_ID,";
        lValues += (m_clStdfWRR.m_cnFRAME_ID.isEmpty())      ? "null" : TranslateStringToSqlVarChar(m_clStdfWRR.m_cnFRAME_ID);
        lValues += ",";

        lFields += "MASK_ID,";
        lValues += (m_clStdfWRR.m_cnMASK_ID.isEmpty())       ? "null" : TranslateStringToSqlVarChar(m_clStdfWRR.m_cnMASK_ID);
        lValues += ",";

        lFields += "WAFER_SIZE,";
        lValues += (m_clStdfWCR.m_r4WAFR_SIZ == 0)           ? "null" : NormalizeNumber(m_clStdfWCR.m_r4WAFR_SIZ);
        lValues += ",";

        lFields += "DIE_HT,";
        lValues += (m_clStdfWCR.m_r4DIE_HT == 0)             ? "null" : NormalizeNumber(m_clStdfWCR.m_r4DIE_HT);
        lValues += ",";

        lFields += "DIE_WID,";
        lValues += (m_clStdfWCR.m_r4DIE_WID == 0)            ? "null" : NormalizeNumber(m_clStdfWCR.m_r4DIE_WID);
        lValues += ",";

        lFields += "WAFER_UNITS,";
        lValues += (m_clStdfWCR.m_u1WF_UNITS == 0)            ? "null" : QString::number(m_clStdfWCR.m_u1WF_UNITS);
        lValues += ",";

        lFields += "WAFER_FLAT,";
        lValues += (mpDbKeysEngine->dbKeysContent().Get("WaferNotch").toString().isEmpty() || mpDbKeysEngine->dbKeysContent().Get("WaferNotch").toString() == QString(" ")) ? "null" : TranslateStringToSqlVarChar((QChar)mpDbKeysEngine->dbKeysContent().Get("WaferNotch").toString().at(0));
        lValues += ",";

        lFields += "CENTER_X,";
        lValues += (m_clStdfWCR.m_i2CENTER_X == INVALID_SMALLINT)? "null" : QString::number(m_clStdfWCR.m_i2CENTER_X);
        lValues += ",";

        lFields += "CENTER_Y,";
        lValues += (m_clStdfWCR.m_i2CENTER_Y == INVALID_SMALLINT)? "null" : QString::number(m_clStdfWCR.m_i2CENTER_Y);
        lValues += ",";

        lFields += "POS_X,";
        lValues += (m_clStdfWCR.m_c1POS_X == 0 || m_clStdfWCR.m_c1POS_X == ' ') ? "null" : TranslateStringToSqlVarChar((QChar)m_clStdfWCR.m_c1POS_X);
        lValues += ",";

        lFields += "POS_Y,";
        lValues += (m_clStdfWCR.m_c1POS_Y == 0 || m_clStdfWCR.m_c1POS_Y == ' ') ? "null" : TranslateStringToSqlVarChar((QChar)m_clStdfWCR.m_c1POS_Y);
        lValues += ",";

        lFields += "GROSS_DIE,";
        lValues += QString::number(nNbGrossDie);
        lValues += ",";

        lFields += "NB_PARTS,";
        lValues += "0";
        lValues += ",";

        lFields += "NB_PARTS_GOOD,";
        lValues += "0";
        lValues += ",";

        lFields += "WAFER_FLAGS,";
        lValues += QString::number(nFlags);
        lValues += ",";

        lFields += "WAFER_NB,";
        bool bOk;
        nWaferNb = m_strWaferId.trimmed().toInt(&bOk);
        if(!bOk)        nWaferNb=-1;
        if(m_eTestingStage == eWaferTest)
        {
            // At WT, check if WaferNb has been overloaded through KeyContent
            if(mpDbKeysEngine->dbKeysContent().Get("WaferNb").toString().isEmpty())
            {
                int nWaferNbFromKey = mpDbKeysEngine->dbKeysContent().Get("WaferNb").toString().trimmed().toInt(&bOk);
                if(bOk)
                    nWaferNb = nWaferNbFromKey;
            }
        }
        lValues += (nWaferNb < 0) ? "null" : QString::number(nWaferNb);
        lValues += ",";

        lFields += "CONSOLIDATION_ALGO,";
        lValues += mpDbKeysEngine->dbKeysContent().Get("ConsolidationAlgo").toString().isEmpty() ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("ConsolidationAlgo").toString());
        lValues += ",";

        lFields += "CONSOLIDATION_STATUS,";
        lValues += "null";
        lValues += ",";

        lFields += "CONSOLIDATION_SUMMARY,";
        lValues += "null";
        lValues += ",";

        lFields += "CONSOLIDATION_DATE";
        lValues += "null";
        if(m_eTestingStage == eElectTest)
        {
            lFields += ",SITE_CONFIG";
            lValues += ",";
            lValues += (mpDbKeysEngine->dbKeysContent().Get("EtestSiteConfig").toString().isEmpty()) ? "null" : TranslateStringToSqlVarChar(mpDbKeysEngine->dbKeysContent().Get("EtestSiteConfig").toString());
        }
        lFields += ")";
        lValues += ")";
        lQuery += lFields + lValues;

        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }
    }
    else
    {
        // DB-67 - Overwrite alrady inserted WaferId+BADProduct
        // Overwrite the product name with the new one
        // The check was already done in the Validation process
        lQuery = "UPDATE " + lTableName + " SET PRODUCT_NAME=";
        lQuery += m_strProductName.isEmpty() ? "null" : TranslateStringToSqlVarChar(m_strProductName);
        lQuery += " WHERE LOT_ID=" + TranslateStringToSqlVarChar(m_strLotId);
        if(m_strWaferId.isEmpty())
            lQuery += " AND WAFER_ID IS NULL";
        else
            lQuery += " AND WAFER_ID="+TranslateStringToSqlVarChar(m_strWaferId);

        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            bStatus = false;
            goto labelUnlock;
        }
    }
    // Debug message
    WriteDebugMessageFile("GexDbPlugin_Galaxy::UpdateWaferInfoTable() : Completed");

labelUnlock:

    // unlock tables
    if(m_pclDatabaseConnector->IsMySqlDB())
        ReleaseLock();
    else
        UnlockTables();

    WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::UpdateWaferInfoTable: end. status = %1").arg(bStatus?"true":"false"));
    return bStatus;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateHBinTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateHBinTable()
{
    QString strTableName;

    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateHBinTable");

    if((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]==0)
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR]==0))
        return true;

    strTableName = NormalizeTableName("_HBIN");

    QString strQueryHeader;     // INSERT INTO ...
    QString strNewValue;        // NEW VALUE FOR INSERTION
    QString strAllValues;       // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName;
    strQueryHeader+= " (SPLITLOT_ID,HBIN_NO,HBIN_NAME,HBIN_CAT";
    if(m_eTestingStage==eElectTest)
        strQueryHeader += ",BIN_COUNT";
    strQueryHeader+= ") VALUES";
    strNewValue = strAllValues = "";

    QMap<int,structBinInfo>::Iterator itBinInfo;
    for ( itBinInfo = m_mapHBinInfo.begin(); itBinInfo != m_mapHBinInfo.end(); ++itBinInfo )
    {
        // for all sites
        // Do not skip if BinCount = 0
        if(!itBinInfo.value().m_mapBinCnt.contains(INVALID_SITE))
            itBinInfo.value().m_mapBinCnt[INVALID_SITE] = 0;

        // if HBR contains only merge information, INVALID_SITE not initialized
        // Initialize INVALID_SITE if necessary
        if(itBinInfo.value().m_mapBinCnt.contains(INVALID_SITE)
                && itBinInfo.value().m_mapBinCnt.contains(MERGE_SITE)
                && (itBinInfo.value().m_mapBinCnt[INVALID_SITE] == 0))
            itBinInfo.value().m_mapBinCnt[INVALID_SITE] = itBinInfo.value().m_mapBinCnt[MERGE_SITE];


        // insert new entry
        strNewValue  = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // HBIN_NO
        strNewValue += QString::number(itBinInfo.value().m_nBinNum);
        strNewValue += ",";
        // HBIN_NAME
        strNewValue += TranslateStringToSqlVarChar(itBinInfo.value().m_strBinName);
        strNewValue += ",";
        // HBIN_CAT
        strNewValue += itBinInfo.value().m_cBinCat == ' ' ? "null" : TranslateStringToSqlVarChar((QChar)itBinInfo.value().m_cBinCat);
        if(m_eTestingStage==eElectTest)
        {
            strNewValue += ",";
            // BIN_COUNT
            strNewValue += QString::number(itBinInfo.value().m_mapBinCnt[INVALID_SITE]);
        }
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
            return false;
    }

    if(strAllValues.isEmpty())
        return true;

    if(!ExecuteMultiInsertQuery(strTableName, strAllValues))
        return false;

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateHBinTable completed");

    // Unlock table
    UnlockTables();

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateSBinTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateSBinTable()
{
    QString strTableName;

    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateSBinTable");

    if((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]==0)
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SBR]==0))
        return true;

    strTableName = NormalizeTableName("_SBIN");

    QString strQueryHeader;  // INSERT INTO ...
    QString strNewValue;     // NEW VALUE FOR INSERTION
    QString strAllValues;    // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName;
    strQueryHeader+= " (SPLITLOT_ID,SBIN_NO,SBIN_NAME,SBIN_CAT";
    if(m_eTestingStage==eElectTest)
        strQueryHeader += ",BIN_COUNT";
    strQueryHeader+= ") VALUES";

    strNewValue = strAllValues = "";

    QMap<int,structBinInfo>::Iterator itBinInfo;
    for ( itBinInfo = m_mapSBinInfo.begin(); itBinInfo != m_mapSBinInfo.end(); ++itBinInfo )
    {
        // for all sites
        // Do not skip if BinCount = 0
        if(!itBinInfo.value().m_mapBinCnt.contains(INVALID_SITE))
            itBinInfo.value().m_mapBinCnt[INVALID_SITE] = 0;

        // if HBR contains only merge information, INVALID_SITE not initialized
        // Initialize INVALID_SITE if necessary
        if(itBinInfo.value().m_mapBinCnt.contains(INVALID_SITE)
                && itBinInfo.value().m_mapBinCnt.contains(MERGE_SITE)
                && (itBinInfo.value().m_mapBinCnt[INVALID_SITE] == 0))
            itBinInfo.value().m_mapBinCnt[INVALID_SITE] = itBinInfo.value().m_mapBinCnt[MERGE_SITE];

        // insert new entry
        strNewValue  = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // SBIN_NO
        strNewValue += QString::number(itBinInfo.value().m_nBinNum);
        strNewValue += ",";
        // SBIN_NAME
        strNewValue += TranslateStringToSqlVarChar(itBinInfo.value().m_strBinName);
        strNewValue += ",";
        // SBIN_CAT
        strNewValue += itBinInfo.value().m_cBinCat == ' ' ? "null" : TranslateStringToSqlVarChar((QChar)itBinInfo.value().m_cBinCat);
        if(m_eTestingStage==eElectTest)
        {
            strNewValue += ",";
            // BIN_COUNT
            strNewValue += QString::number(itBinInfo.value().m_mapBinCnt[INVALID_SITE]);
        }
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
            return false;
    }
    if(strAllValues.isEmpty())
        return true;

    if(!ExecuteMultiInsertQuery(strTableName, strAllValues))
        return false;

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateSBinTable completed");

    // Unlock table
    UnlockTables();

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdatePartsStatsSamplesTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdatePartsStatsSamplesTable()
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdatePartsStatsSamplesTable");


    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    QString lPartsSamplesTable;
    QString lQuery;
    GexDbPlugin_Query   lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    lPartsSamplesTable = NormalizeTableName("_PARTS_STATS_SAMPLES");

    QString strQueryHeader;                // INSERT INTO ...
    QString strNewValue;                // NEW VALUE FOR INSERTION
    QString strAllValues;                // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + lPartsSamplesTable + " VALUES";
    strNewValue = strAllValues = "";

    // Extract directly from the run table the FINAL results
    lQuery = "SELECT site_no, count(*) AS nb_parts, sum(part_status='P') as nb_parts_good "
             " FROM "+NormalizeTableName("_RUN")+
            " WHERE splitlot_id="+QString::number(m_nSplitLotId)+
    // Do not filter on the NOT PARTRETESTED
    // We want all parts for the StatsSamples
    // No use the Inline Consolidation
    //      " AND (part_flags&"+QString::number(FLAG_RUN_PARTRETESTED)+")=0 "
            " GROUP BY site_no ";
    if(!lGexDbQuery.Execute(lQuery))
    {
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                    lGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Recompute the NbParts and NbPartsGood
    int lNbParts = 0;
    int lNbPartsGood = 0;
    // for each site
    while ( lGexDbQuery.Next() )
    {
        // insert new entry
        strNewValue  = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // SITE_NO
        strNewValue += QString::number(lGexDbQuery.value("site_no").toInt());
        strNewValue += ",";
        // NB_PARTS
        strNewValue += QString::number(lGexDbQuery.value("nb_parts").toInt());
        strNewValue += ",";
        // NB_PARTS_GOOD
        strNewValue += QString::number(lGexDbQuery.value("nb_parts_good").toInt());
        strNewValue += ")";

        lNbParts += lGexDbQuery.value("nb_parts").toInt();
        lNbPartsGood += lGexDbQuery.value("nb_parts_good").toInt();

        if(!AddInMultiInsertQuery(lPartsSamplesTable, strQueryHeader, strNewValue, strAllValues))
            return false;
    }

    if(strAllValues.isEmpty())
        return true;

    if(!ExecuteMultiInsertQuery(lPartsSamplesTable, strAllValues))
        return false;

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdatePartsStatsSamplesTable completed");

    // Unlock table
    UnlockTables();

    return true;
}
///////////////////////////////////////////////////////////
// Update GALAXY database : UpdatePartsStatsSummaryTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdatePartsStatsSummaryTable()
{
    QString strTableName;

    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdatePartsStatsSummaryTable");

    // nothing todo if no PCR
    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PCR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    strTableName = NormalizeTableName("_PARTS_STATS_SUMMARY");

    QString strQueryHeader;                // INSERT INTO ...
    QString strNewValue;                // NEW VALUE FOR INSERTION
    QString strAllValues;                // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName + " VALUES";
    strNewValue = strAllValues = "";

    // for each site
    QMap<int,int>::Iterator itSite;
    int        nSiteNumber;

    for ( itSite = m_mapPCRNbParts.begin(); itSite != m_mapPCRNbParts.end(); ++itSite )
    {
        nSiteNumber = itSite.key();
        if(nSiteNumber == INVALID_SITE)
        {
            if(m_mapPCRNbParts.contains(MERGE_SITE))
                continue;
            nSiteNumber = MERGE_SITE;
        }

        // insert new entry
        strNewValue  = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // SITE_NO
        strNewValue += QString::number(nSiteNumber);
        strNewValue += ",";
        // NB_PARTS
        strNewValue += QString::number(m_mapPCRNbParts[itSite.key()]);
        strNewValue += ",";
        // NB_GOOD
        strNewValue += !m_mapPCRNbPartsGood.contains(itSite.key()) ? "null" : QString::number(m_mapPCRNbPartsGood[itSite.key()]);
        strNewValue += ",";
        // NB_RTST
        strNewValue += !m_mapPCRNbRetestParts.contains(itSite.key()) ? "null" : QString::number(m_mapPCRNbRetestParts[itSite.key()]);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
            return false;
    }

    if(strAllValues.isEmpty())
        return true;

    if(!ExecuteMultiInsertQuery(strTableName, strAllValues))
        return false;

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdatePartsStatsSummaryTable completed");

    // Unlock table
    UnlockTables();

    return true;
}
///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateStatsSamplesHBinTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateHBinStatsSamplesTable()
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateHBinStatsSamplesTable");


    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    QString lTableName = NormalizeTableName("_HBIN_STATS_SAMPLES");
    QString lQueryHeader;       // INSERT INTO ...
    QString lNewValue;          // NEW VALUE FOR INSERTION
    QString lAllValues;         // MULTI QUERIES

    lQueryHeader = "INSERT INTO " + lTableName + " VALUES";
    lNewValue = lAllValues = "";

    // Extract directly from the run table the FINAL results
    QString lQuery;
    GexDbPlugin_Query   lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    lQuery = "SELECT site_no, hbin_no, count(*) AS nb_parts "
             " FROM "+NormalizeTableName("_RUN")+
            " WHERE splitlot_id="+QString::number(m_nSplitLotId)+
    // Do not filter on the NOT PARTRETESTED
    // We want all parts for the StatsSamples
    // No use the Inline Consolidation
    //      " AND (part_flags&"+QString::number(FLAG_RUN_PARTRETESTED)+")=0 "
            " GROUP BY site_no, hbin_no ";
    if(!lGexDbQuery.Execute(lQuery))
    {
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                    lGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // for each site
    while ( lGexDbQuery.Next() )
    {
        // insert new entry
        lNewValue  = "(";
        // SPLITLOT_ID
        lNewValue += QString::number(m_nSplitLotId);
        lNewValue += ",";
        // SITE_NO
        lNewValue += QString::number(lGexDbQuery.value("site_no").toInt());
        lNewValue += ",";
        // HBIN_NO
        lNewValue += QString::number(lGexDbQuery.value("hbin_no").toInt());
        lNewValue += ",";
        // NB_PARTS
        lNewValue += QString::number(lGexDbQuery.value("nb_parts").toInt());
        lNewValue += ")";


        if(!AddInMultiInsertQuery(lTableName, lQueryHeader, lNewValue, lAllValues))
            return false;
    }

    if(lAllValues.isEmpty())
        return true;

    if(!ExecuteMultiInsertQuery(lTableName, lAllValues))
        return false;

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateHBinStatsSamplesTable completed");

    // Unlock table
    UnlockTables();

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateStatsSummaryHBinTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateHBinStatsSummaryTable()
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateHBinStatsSummaryTable");

    QString strTableName;

    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    // Binning information
    // if HBR=0 and SBR>0 then HBR is rebuild from SBR information (end of pass1)
    //
    // MERGE_SITE = HBR[255] = HBR merge_site summary
    // INVALID_SITE = our merge_site summary
    //
    // if HBR[255] then use it and ignore our merge_site summary => siteNum=-1
    // else use our merge_site summary => SiteNum=-1

    strTableName = NormalizeTableName("_HBIN_STATS_SUMMARY");

    QString strQueryHeader;                // INSERT INTO ...
    QString strNewValue;                // NEW VALUE FOR INSERTION
    QString strAllValues;                // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName + " VALUES";
    strNewValue = strAllValues = "";

    int        nSiteNumber;
    QMap<int,structBinInfo>::Iterator itBin;
    for ( itBin = m_mapHBinInfo.begin(); itBin != m_mapHBinInfo.end(); ++itBin )
    {
        // verify if have information from HBR
        if(itBin.value().m_mapBinCnt.isEmpty())
            continue;

        // for each Site
        QMap<int,int>::Iterator itSite;
        for (itSite = itBin.value().m_mapBinCnt.begin();        itSite != itBin.value().m_mapBinCnt.end(); ++itSite )
        {
            nSiteNumber = itSite.key();
            if(nSiteNumber == INVALID_SITE)
            {
                if(itBin.value().m_mapBinCnt.contains(MERGE_SITE))
                    continue;
                nSiteNumber = MERGE_SITE;
            }

            // insert new entry
            strNewValue  = "(";
            // SPLITLOT_ID
            strNewValue += QString::number(m_nSplitLotId);
            strNewValue += ",";
            // SITE_NO
            strNewValue += QString::number(nSiteNumber);
            strNewValue += ",";
            // HBIN_NO
            strNewValue += QString::number(itBin.value().m_nBinNum);
            strNewValue += ",";
            // BIN_COUNT
            strNewValue += QString::number(itBin.value().m_mapBinCnt[itSite.key()]);
            strNewValue += ")";

            if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
                return false;
        }
    }

    if(strAllValues.isEmpty())
        return true;

    if(!ExecuteMultiInsertQuery(strTableName, strAllValues))
        return false;

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateHBinStatsSummaryTable completed");

    // Unlock table
    UnlockTables();

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateStatsSamplesSBinTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateSBinStatsSamplesTable()
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateSBinStatsSamplesTable");


    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    QString lTableName = NormalizeTableName("_SBIN_STATS_SAMPLES");

    QString lQueryHeader;                // INSERT INTO ...
    QString lNewValue;                // NEW VALUE FOR INSERTION
    QString lAllValues;                // MULTI QUERIES

    lQueryHeader = "INSERT INTO " + lTableName + " VALUES";
    lNewValue = lAllValues = "";

    // Extract directly from the run table the FINAL results
    QString lQuery;
    GexDbPlugin_Query   lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    lQuery = "SELECT site_no, sbin_no, count(*) AS nb_parts "
             " FROM "+NormalizeTableName("_RUN")+
            " WHERE splitlot_id="+QString::number(m_nSplitLotId)+
    // Do not filter on the NOT PARTRETESTED
    // We want all parts for the StatsSamples
    // No use the Inline Consolidation
    //      " AND (part_flags&"+QString::number(FLAG_RUN_PARTRETESTED)+")=0 "
            " GROUP BY site_no, sbin_no ";
    if(!lGexDbQuery.Execute(lQuery))
    {
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                    lGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // for each site
    while ( lGexDbQuery.Next() )
    {
        // insert new entry
        lNewValue  = "(";
        // SPLITLOT_ID
        lNewValue += QString::number(m_nSplitLotId);
        lNewValue += ",";
        // SITE_NO
        lNewValue += QString::number(lGexDbQuery.value("site_no").toInt());
        lNewValue += ",";
        // SBIN_NO
        lNewValue += QString::number(lGexDbQuery.value("sbin_no").toInt());
        lNewValue += ",";
        // NB_PARTS
        lNewValue += QString::number(lGexDbQuery.value("nb_parts").toInt());
        lNewValue += ")";


        if(!AddInMultiInsertQuery(lTableName, lQueryHeader, lNewValue, lAllValues))
            return false;
    }

    if(lAllValues.isEmpty())
        return true;

    if(!ExecuteMultiInsertQuery(lTableName, lAllValues))
        return false;

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateSBinStatsSamplesTable completed");

    // Unlock table
    UnlockTables();

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateStatsSummarySBinTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateSBinStatsSummaryTable()
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateSBinStatsSummaryTable");

    QString strTableName;

    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_SBR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    // Binning information
    //
    // MERGE_SITE = SBR[255] = SBR merge_site summary
    // INVALID_SITE = our merge_site summary
    //
    // if SBR[255] then use it and ignore our merge_site summary => siteNum=-1
    // else use our merge_site summary => SiteNum=-1

    strTableName = NormalizeTableName("_SBIN_STATS_SUMMARY");

    QString strQueryHeader;                // INSERT INTO ...
    QString strNewValue;                // NEW VALUE FOR INSERTION
    QString strAllValues;                // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName + " VALUES";
    strNewValue = strAllValues = "";

    // for each Bin
    int nSiteNumber;
    QMap<int,structBinInfo>::Iterator itBin;
    for ( itBin = m_mapSBinInfo.begin(); itBin != m_mapSBinInfo.end(); ++itBin )
    {
        if(itBin.value().m_mapBinCnt.isEmpty())
            continue;

        // for each Site
        QMap<int,int>::Iterator itSite;
        for ( itSite = itBin.value().m_mapBinCnt.begin(); itSite != itBin.value().m_mapBinCnt.end(); ++itSite )
        {
            nSiteNumber = itSite.key();
            if(nSiteNumber == INVALID_SITE)
            {
                if(itBin.value().m_mapBinCnt.contains(MERGE_SITE))
                    continue;
                nSiteNumber = MERGE_SITE;
            }

            // insert new entry
            strNewValue  = "(";
            // SPLITLOT_ID
            strNewValue += QString::number(m_nSplitLotId);
            strNewValue += ",";
            // SITE_NO
            strNewValue += QString::number(nSiteNumber);
            strNewValue += ",";
            // SBIN_NO
            strNewValue += QString::number(itBin.value().m_nBinNum);
            strNewValue += ",";
            // BIN_COUNT
            strNewValue += QString::number(itBin.value().m_mapBinCnt[itSite.key()]);
            strNewValue += ")";

            if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
                return false;
        }
    }

    if(strAllValues.isEmpty())
        return true;

    if(!ExecuteMultiInsertQuery(strTableName, strAllValues))
        return false;

    GSLOG(SYSLOG_SEV_DEBUG, "UpdateSBinStatsSummaryTable completed");

    // Unlock table
    bool b=UnlockTables();
    if (!b)
        GSLOG(SYSLOG_SEV_WARNING, "UnlockTables failed");

    return true;
}


///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateRunTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdatePMRInfoTable()
{
    if(m_mapPinDef.isEmpty())
        return true;

    QString strQueryHeader;   // INSERT INTO ...
    QString strNewValue;      // NEW VALUE FOR INSERTION
    QString strAllValues;     // MULTI QUERIES


    QString lValue;

    // Insert into xx_pin_map table
    strQueryHeader = "INSERT INTO " + NormalizeTableName("_PIN_MAP")
            + "(splitlot_id,tpin_pmrindex,chan_typ,chan_nam,phy_nam,log_nam,head_num,site_num) VALUES";
    strNewValue = strAllValues = "";

    foreach(lValue, m_mapPinDef.values())
    {
        strNewValue += "("+QString::number(m_nSplitLotId) + ",";
        strNewValue += lValue;
        strNewValue += ")";

        if(!AddInMultiInsertQuery(NormalizeTableName("_PIN_MAP"), strQueryHeader, strNewValue, strAllValues))
            return false;
    }

    // Execute insertion query
    if(!strAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_PIN_MAP"), strAllValues)))
        return false;

    m_mapPinDef.clear();
    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateRunTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateRunTable()
{
    QString lBuffer;
    QString lTableName;

    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]==0)
        return true;

    static QTime timeInsertion;
    if (m_nNbRunProcessed==0)
        timeInsertion.start();

    // this function is called for each PRR in ProcessPRR()
    //GSLOG(SYSLOG_SEV_DEBUG, QString("UpdateRunTable %1 PRR...").arg( m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]));

    // extract the site number from the prr
    unsigned short siteNumber =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            ( m_clStdfParse, m_clStdfPRR );

    // Check if it is a valid PRR
    if((m_clStdfPRR.m_u2HARD_BIN <= 32767) || (m_clStdfPRR.m_u2SOFT_BIN <= 32767))
    {
        // This is a valid PRR
        // Have to Update the Run table
        lTableName = NormalizeTableName("_RUN");

        QString lPartStatus = " ";
        QString lPartId = (m_clStdfPRR.m_cnPART_ID.length()!=0) ? m_clStdfPRR.m_cnPART_ID : "null";
        QString lXCoord = (m_clStdfPRR.m_i2X_COORD != INVALID_SMALLINT) ? QString::number(m_clStdfPRR.m_i2X_COORD) : "null";
        QString lYCoord = (m_clStdfPRR.m_i2Y_COORD != INVALID_SMALLINT) ? QString::number(m_clStdfPRR.m_i2Y_COORD) : "null";
        QString lPartTxt = (m_clStdfPRR.m_cnPART_TXT.length()!=0) ? m_clStdfPRR.m_cnPART_TXT : "null";

        // Check if Pass/Fail flag valid
        if((m_clStdfPRR.m_b1PART_FLG & STDF_MASK_BIT4) == 0)
        {
            lPartStatus = "P";
            if(m_clStdfPRR.m_b1PART_FLG & STDF_MASK_BIT3)
                lPartStatus = "F";
        }

        if(!m_mapTestsExecuted.contains(siteNumber))
        {
            // Reject the file
            lBuffer = "Invalid Site number - ";
            lBuffer+= " SiteNo["+QString::number(siteNumber)+"] - ";
            lBuffer+= " PartId["+m_clStdfPRR.m_cnPART_ID+"] - ";
            lBuffer+= " Cannot associate this PRR with a valid PIR";
            GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidField, NULL,lBuffer.toLatin1().data());
            GSLOG(SYSLOG_SEV_WARNING, lBuffer.toLatin1().data());
            return false;
        }

        QString strQueryHeader;  // INSERT INTO ...
        QString strNewValue;     // NEW VALUE FOR INSERTION
        QString strAllValues;    // MULTI QUERIES

        strQueryHeader = "INSERT INTO " + lTableName;
        strNewValue = strAllValues = "";

        // insert new entry
        strQueryHeader += "(";
        strNewValue = "(";
        // SPLITLOT_ID
        strQueryHeader += "SPLITLOT_ID";
        strQueryHeader += ",";
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // RUN_ID
        strQueryHeader += "RUN_ID";
        strQueryHeader += ",";
        strNewValue += QString::number(m_mapRunId[siteNumber]);
        strNewValue += ",";
        // SITE_NO
        if(m_eTestingStage != eElectTest)
        {
            strQueryHeader += "SITE_NO";
            strQueryHeader += ",";
            strNewValue += QString::number(siteNumber);
            strNewValue += ",";
        }
        // PART_ID
        strQueryHeader += "PART_ID";
        strQueryHeader += ",";
        strNewValue += lPartId == "null" ? "null" : TranslateStringToSqlVarChar(lPartId);
        strNewValue += ",";
        // PART_X
        strQueryHeader += "PART_X";
        strQueryHeader += ",";
        strNewValue += lXCoord;
        strNewValue += ",";
        // PART_Y
        strQueryHeader += "PART_Y";
        strQueryHeader += ",";
        strNewValue += lYCoord;
        strNewValue += ",";
        // PART_STATUS
        strQueryHeader += "PART_STATUS";
        strQueryHeader += ",";
        strNewValue += TranslateStringToSqlVarChar(lPartStatus);
        strNewValue += ",";
        // PART_FLAGS
        strQueryHeader += "PART_FLAGS";
        strQueryHeader += ",";
        strNewValue += QString::number(0);
        strNewValue += ",";
        // HBIN_NO
        strQueryHeader += "HBIN_NO";
        strQueryHeader += ",";
        strNewValue += QString::number(m_clStdfPRR.m_u2HARD_BIN);
        strNewValue += ",";
        // SBIN_NO
        strQueryHeader += "SBIN_NO";
        strQueryHeader += ",";
        strNewValue += m_clStdfPRR.m_u2SOFT_BIN == 65535 ? "null" : QString::number(m_clStdfPRR.m_u2SOFT_BIN);
        strNewValue += ",";
        // TTIME
        strQueryHeader += "TTIME";
        strQueryHeader += ",";
        strNewValue += m_clStdfPRR.m_u4TEST_T == 0 ? "null" : QString::number(m_clStdfPRR.m_u4TEST_T);
        strNewValue += ",";
        // TESTS_EXECUTED
        strQueryHeader += "TESTS_EXECUTED";
        strQueryHeader += ",";
        strNewValue += QString::number(m_mapTestsExecuted[siteNumber]);
        strNewValue += ",";
        // TESTS_FAILED
        strQueryHeader += "TESTS_FAILED";
        strQueryHeader += ",";
        strNewValue += QString::number(m_mapTestsFailed[siteNumber]);
        strNewValue += ",";
        // FIRSTFAIL_TEST_TYPE
        strQueryHeader += "FIRSTFAIL_TEST_TYPE";
        strQueryHeader += ",";
        strNewValue += m_mapTestsFailed[siteNumber] == 0 ? "null" : TranslateStringToSqlVarChar(m_mapFirstFailTType[siteNumber]);
        strNewValue += ",";
        // FIRSTFAIL_TEST_ID
        strQueryHeader += "FIRSTFAIL_TEST_ID";
        strQueryHeader += ",";
        strNewValue += m_mapTestsFailed[siteNumber] == 0 ? "null" : QString::number(m_mapFirstFailTId[siteNumber]);
        strNewValue += ",";
        // LASTFAIL_TEST_TYPE
        strQueryHeader += "LASTFAIL_TEST_TYPE";
        strQueryHeader += ",";
        strNewValue += m_mapTestsFailed[siteNumber] == 0 ? "null" : TranslateStringToSqlVarChar(m_mapLastFailTType[siteNumber]);
        strNewValue += ",";
        // LASTFAIL_TEST_ID
        strQueryHeader += "LASTFAIL_TEST_ID";
        strQueryHeader += ",";
        strNewValue += m_mapTestsFailed[siteNumber] == 0 ? "null" : QString::number(m_mapLastFailTId[siteNumber]);
        strNewValue += ",";
        // PART_RETEST_INDEX
        strQueryHeader += "PART_RETEST_INDEX";
        strQueryHeader += ",";
        strNewValue += QString::number(0);
        strNewValue += ",";
        // SITE_NO
        if(m_eTestingStage == eElectTest)
        {
            strQueryHeader += "SITE_NO";
            strQueryHeader += ",";
            strNewValue += QString::number(siteNumber);
            strNewValue += ",";
        }
        // WAFER_ID
        if(m_eTestingStage == eFinalTest)
        {
            strQueryHeader += "WAFER_ID";
            strQueryHeader += ",";
            strNewValue += m_strWaferId.isEmpty() ? "null" : TranslateStringToSqlVarChar(m_strWaferId);
            strNewValue += ",";
        }
        // PART_TXT
        strQueryHeader += "PART_TXT";
        strNewValue += lPartTxt == "null" ? "null" : TranslateStringToSqlVarChar(lPartTxt);

        strQueryHeader += ") VALUES";
        strNewValue += ")";

        if(!AddInMultiInsertQuery(lTableName, strQueryHeader, strNewValue, m_strQueryRun))
        {
            GSLOG(SYSLOG_SEV_ERROR, "AddInMultiInsertQuery failed");
            return false;
        }

        m_nNbRunInQuery++;

        // UPDATE ALL DATA FILE FOR ORACLE AND MYSQL FOR "LOAD DATA INFILE" USAGE
        // FOR PTEST RESULTS
        if(!m_strPTestResults.isEmpty())
        {
            if (!AddInLoadDataInfile(GQTL_STDF::Stdf_Record::Rec_PTR, m_strPTestResults))
            {
                GSLOG(SYSLOG_SEV_ERROR, "AddInLoadDataInfile PTR failed");
                return false;
            }
        }
        // FOR MPTEST RESULTS
        if(!m_strMPTestResults.isEmpty())
        {
            if (!AddInLoadDataInfile(GQTL_STDF::Stdf_Record::Rec_MPR, m_strMPTestResults))
            {
                GSLOG(SYSLOG_SEV_ERROR, "AddInLoadDataInfile MPR failed");
                return false;
            }
        }
        // FOR FTEST RESULTS
        if(!m_strFTestResults.isEmpty())
        {
            if (!AddInLoadDataInfile(GQTL_STDF::Stdf_Record::Rec_FTR, m_strFTestResults))
            {
                GSLOG(SYSLOG_SEV_ERROR, "AddInLoadDataInfile FTR failed");
                return false;
            }
        }

    }

    m_nNbRunProcessed++;

    //if(m_nNbRunProcessed >= m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR])
    if ( ((mForceSqlInsertionTimeout>0) && (timeInsertion.elapsed()>mForceSqlInsertionTimeout)) // 6390
         || (m_nNbRunProcessed >= m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]) )
    {
        QCoreApplication::processEvents();
        GSLOG(SYSLOG_SEV_NOTICE, QString("Update run table : executing insertion query(s) : NbRunProcessed=%1")
              .arg( m_nNbRunProcessed).toLatin1().constData());
        if(!m_strQueryRun.isEmpty())
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("Starting MultiInsert to %1")
                  .arg( lTableName ).toLatin1().constData());
            if(!ExecuteMultiInsertQuery(lTableName, m_strQueryRun))
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("ExecuteMultiInsertQuery on %1 failed")
                      .arg( lTableName).toLatin1().constData());
                return false;
            }
            GSLOG(SYSLOG_SEV_DEBUG, QString("Ending MultiInsert to %1")
                  .arg( lTableName ).toLatin1().constData());
            m_strQueryRun = "";
        }
        // update all tables results
        GSLOG(SYSLOG_SEV_DEBUG, QString("Starting Insert to PTR Records").toLatin1().constData());
        if(!ExecuteLoadDataInfile(GQTL_STDF::Stdf_Record::Rec_PTR))
        {
            GSLOG(SYSLOG_SEV_ERROR, "ExecuteLoadDataInfile PTR failed");
            return false;
        }
        GSLOG(SYSLOG_SEV_DEBUG, QString("Ending Insert to PTR Records").toLatin1().constData());

        // Now do the Multi-parametric table
        GSLOG(SYSLOG_SEV_DEBUG, QString("Starting Insert to MPR Records").toLatin1().constData());

        if(!ExecuteLoadDataInfile(GQTL_STDF::Stdf_Record::Rec_MPR))
        {
            GSLOG(SYSLOG_SEV_ERROR, "ExecuteLoadDataInfile MPR failed");
            return false;
        }
        GSLOG(SYSLOG_SEV_DEBUG, QString("Ending Insert to MPR Records").toLatin1().constData());

        // Finally do FTR records ..
        GSLOG(SYSLOG_SEV_DEBUG, QString("Starting Insert to FTR Records").toLatin1().constData());
        if(!ExecuteLoadDataInfile(GQTL_STDF::Stdf_Record::Rec_FTR))
        {
            GSLOG(SYSLOG_SEV_ERROR, "ExecuteLoadDataInfile FTR failed");
            return false;
        }
        GSLOG(SYSLOG_SEV_DEBUG, QString("Ending Insert to MPR Records").toLatin1().constData());

        m_nNbRunInQuery = 0;
        timeInsertion.start();
    }

    // GCORE-2327 - YieldMan rejects files on Duplicate results entry
    // m_mapTestsExecuted[siteNumber] = m_mapTestsFailed[siteNumber] = m_mapFirstFailTNum[siteNumber] = 0;
    m_mapTestsExecuted.remove(siteNumber);
    m_mapTestsFailed[siteNumber] = m_mapFirstFailTId[siteNumber] = 0;
    m_mapFirstFailTType[siteNumber] = "";
    m_mapFirstFailTId[siteNumber] = 0;
    m_mapLastFailTType[siteNumber] = "";
    m_mapLastFailTId[siteNumber] = 0;
    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateRunTableForRetestedParts
// Check the validity of the PART_X, PART_Y, PART_ID
// and update the RETEST_INDEX and RETESTED_PART flags
// During the insertion process, the lSplitlotID=0
// But can be executed out side the insertion for an incremental update
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateRunTablePartFlags(long lSplitlotID)
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateRunTableForRetestedParts");

    if(lSplitlotID > 0)
    {
        m_nSplitLotId = lSplitlotID;
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]=1;
        m_bUsePcrForSummary = false;
        m_strlWarnings.clear();
    }

    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    // If have SAMPLING
    // No consolidation possible
    // Ignore this update
    if(m_bUsePcrForSummary)
        return true;

    QString lRunTable = NormalizeTableName("_RUN");
    QString lMessage;
    QString lQuery;
    GexDbPlugin_Query   lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    int lNbRuns = 0;
    int lNbFailRuns = 0;
    int lNbParts = 0;
    int lNbSites = 1;
    bool lHaveValidPartXY = false;
    bool lHaveValidPartId = false;
    bool lUpdateForRetest = true;
    bool lComputeFullTouchDown = false;
    bool lUpdateTestFailed = false;

    // Check the number of runs
    lQuery = "SELECT count(*) AS nb_runs, "
             "      count(distinct PART_X, PART_Y) AS nb_part_XY, "
             "      count(distinct PART_ID) AS nb_part_ID, "
             "      count(distinct SITE_NO) AS nb_sites, "
             "      sum(tests_failed > 0) AS nb_fail_runs, "
             "      sum(FIRSTFAIL_TEST_TYPE IS NOT NULL) AS nb_fail_tests "
             " FROM "+lRunTable+
             " WHERE splitlot_id="+QString::number(m_nSplitLotId);
    if(!lGexDbQuery.Execute(lQuery))
    {
        if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                    lGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    if(!lGexDbQuery.first()
            || (lGexDbQuery.value("nb_runs").toInt()==0))
    {
        // No data ?
        // Nothing to do
        return true;
    }
    lNbRuns = lGexDbQuery.value("nb_runs").toInt();
    lNbFailRuns = lGexDbQuery.value("nb_fail_runs").toInt();
    lNbSites = lGexDbQuery.value("nb_sites").toInt();
    lHaveValidPartXY = (lGexDbQuery.value("nb_part_XY").toInt()>1);
    lHaveValidPartId = (lGexDbQuery.value("nb_part_ID").toInt()>1);
    lComputeFullTouchDown = (lNbSites > 1);

    if(lHaveValidPartXY)
        // If have valid XY, it is possible to know the number of PHYSICAL parts
        lNbParts = lGexDbQuery.value("nb_part_XY").toInt();
    else if(lHaveValidPartId)
        // If have valid ID, it is possible to know the number of PHYSICAL parts
        lNbParts = lGexDbQuery.value("nb_part_ID").toInt();
    else
        // else considere each RUN as a separate PART
        lNbParts = lNbRuns;

    // Check if have some PARTS RETESTED
    if(lNbParts == lNbRuns)
    {
        // Nothing to do
        // The PART_RETEST_INDEX must be set to 0
        // flags for LAST_RETESTED is setted for ALL
        lUpdateForRetest = false;
    }

    // Check if have fail PARTS
    if(lNbFailRuns > 0)
    {
        // Have fail PARTS
        // Only for incremental update
        if((lSplitlotID > 0)
                // And only if no first/last info
                && (lGexDbQuery.value("nb_fail_tests").toInt() == 0))
        {
            lUpdateTestFailed = true;
        }
    }

    if(lUpdateForRetest)
    {
        // Check the consistence of the PART identification
        if(lHaveValidPartXY)
        {
            // Check the max occurence of
            lQuery = "SELECT MAX(nb_occurences) AS max_occurences FROM ("
                     "SELECT PART_X, PART_Y, count(*) as nb_occurences "
                     " FROM "+lRunTable+
                     " WHERE splitlot_id="+QString::number(m_nSplitLotId)+
                    " GROUP BY PART_X, PART_Y )T";
            if(!lGexDbQuery.Execute(lQuery))
            {
                if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            lGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            lGexDbQuery.first();
            if(lGexDbQuery.value("max_occurences").toInt() > 255)
            {
                // OUT OF RANGE
                lHaveValidPartXY = false;
            }
        }
        if(!lHaveValidPartXY & lHaveValidPartId)
        {
            // Check the max occurence of
            lQuery = "SELECT MAX(nb_occurences) AS max_occurences FROM ("
                     "SELECT PART_ID, count(*) as nb_occurences "
                     " FROM "+lRunTable+
                     " WHERE splitlot_id="+QString::number(m_nSplitLotId)+
                    " GROUP BY PART_ID )T";
            if(!lGexDbQuery.Execute(lQuery))
            {
                if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            lGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            lGexDbQuery.first();
            if(lGexDbQuery.value("max_occurences").toInt() > 255)
            {
                // OUT OF RANGE
                lHaveValidPartId = false;
            }
        }
        lUpdateForRetest = (lHaveValidPartXY || lHaveValidPartId);
        if(!lUpdateForRetest)
        {
            // Case 5815
            // Check if RetestIndex is not out of range
            // Delay the file
            QString lError = "Parts location (PART_X, PART_Y, PART_ID) are invalid";
            GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidField, NULL,lError.toLatin1().data());
            GSLOG(SYSLOG_SEV_WARNING, lError.toLatin1().data());
            if(m_pbDelayInsertion) *m_pbDelayInsertion = true;
            return false;
        }
    }

    if(lUpdateForRetest)
    {

        // To disabled the Gap Lock on xx_run
        // we need to work on the PRIMARY KEY (SPLITLOT_ID,RUN_ID[,SITE_NO])
        if(lHaveValidPartXY || lHaveValidPartId)

        {
            // Extract the RUN_ID that need to be updated
            QString lPartDefinition = " PART_X SMALLINT(6), PART_Y SMALLINT(6) ";
            QString lPartExtraction = " PART_X, PART_Y ";
            QString lPartJoin = " r.PART_X=rt.PART_X AND r.PART_Y=rt.PART_Y ";
            if(!lHaveValidPartXY)
            {
                lPartDefinition = " PART_ID VARCHAR(255) ";
                lPartExtraction = " PART_ID ";
                lPartJoin = " r.PART_ID=rt.PART_ID ";
            }

            // Create a TEMPORARY tables
            lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_INLINE"));
            lQuery="CREATE TEMPORARY TABLE "+NormalizeTableName("_RUN_INLINE")+
                    " ("
                    "   SPLITLOT_ID int(10) unsigned NOT NULL,"
                    "   RUN_ID mediumint(8) unsigned NOT NULL,"
                    "   SITE_NO smallint(5) NOT NULL DEFAULT '1',"
                    +lPartDefinition+","
                    "   PART_RETEST_INDEX tinyint(3) unsigned NOT NULL DEFAULT '0',"
                    "   PART_FLAGS tinyint(1) unsigned NOT NULL DEFAULT '0' ,"
                    "   PRIMARY KEY (SPLITLOT_ID,RUN_ID,SITE_NO),"
                    "   KEY ("+lPartExtraction+")"
                    " )";
            if(!lGexDbQuery.Execute(lQuery))
            {
                if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            lGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }

            lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_RETEST"));
            lQuery = "CREATE TEMPORARY TABLE "+NormalizeTableName("_RUN_RETEST")+
                    " ( "
                    +lPartDefinition+", "
                    "   LIST_RUN_ID TEXT, "
                    "   NB_RUNS TINYINT(3) unsigned,"
                    "   PRIMARY KEY ("+lPartExtraction+") "
                    ")";
            if(!lGexDbQuery.Execute(lQuery))
            {
                if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            lGexDbQuery.lastError().text().toLatin1().constData());
                lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_INLINE"));
                lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_RETEST"));

                return false;
            }

            lQuery = "INSERT INTO "+NormalizeTableName("_RUN_INLINE")+
                    "(SPLITLOT_ID,RUN_ID,SITE_NO, "+lPartExtraction+", PART_RETEST_INDEX, PART_FLAGS)"
                    " (SELECT SPLITLOT_ID,RUN_ID,SITE_NO, "+lPartExtraction+", 0, 0 "
                    "  FROM "+lRunTable+" "
                    "  WHERE splitlot_id="+QString::number(m_nSplitLotId)+")";
            if(!lGexDbQuery.Execute(lQuery))
            {
                if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            lGexDbQuery.lastError().text().toLatin1().constData());
                lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_INLINE"));
                lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_RETEST"));
                return false;
            }
            lQuery = " INSERT INTO "+NormalizeTableName("_RUN_RETEST")+
                     " SELECT "+lPartExtraction+", list_run_id, nb_runs "
                     " FROM "
                     " ( "
                     "      SELECT "+lPartExtraction+", GROUP_CONCAT(run_id ORDER BY run_id) AS list_run_id, count(*) AS nb_runs "
                     "      FROM "+NormalizeTableName("_RUN_INLINE")+
                     "      GROUP BY "+lPartExtraction+
                     " ) T "
                     " WHERE nb_runs>1";
            if(!lGexDbQuery.Execute(lQuery))
            {
                if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            lGexDbQuery.lastError().text().toLatin1().constData());
                lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_INLINE"));
                lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_RETEST"));
                return false;
            }

            // FIND_IN_SET
            // The update of the FLAGS LAST_RETESTED is on run_id=last_run_id
            // The update of the PART_RETEST_INDEX is
            lQuery = "UPDATE "+NormalizeTableName("_RUN_RETEST")+" rt "
                    " INNER JOIN "+NormalizeTableName("_RUN_INLINE")+" r "
                    " ON "+lPartJoin+
                    // Extract the position of the run_id from the list_run_id
                    "   SET r.PART_RETEST_INDEX=FIND_IN_SET(r.run_id,rt.list_run_id)-1, "
                    // Add the RETESTED PART_flags if not the last
                    "   r.PART_FLAGS=IF(FIND_IN_SET(r.run_id,rt.list_run_id)=NB_RUNS , PART_FLAGS , (PART_FLAGS | "+QString::number(FLAG_RUN_PARTRETESTED)+")) ";
            if(!lGexDbQuery.Execute(lQuery))
            {
                if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            lGexDbQuery.lastError().text().toLatin1().constData());
                lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_INLINE"));
                lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_RETEST"));
                return false;
            }

            lQuery = "UPDATE "+NormalizeTableName("_RUN_INLINE")+" rt "
                    " INNER JOIN "+lRunTable+" r "
                    " ON r.splitlot_id=rt.splitlot_id AND r.RUN_ID=rt.RUN_ID AND r.SITE_NO=rt.SITE_NO "
                    " SET r.PART_RETEST_INDEX=rt.PART_RETEST_INDEX, "
                    " r.PART_FLAGS=rt.PART_FLAGS";
            if(!lGexDbQuery.Execute(lQuery))
            {
                if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            lGexDbQuery.lastError().text().toLatin1().constData());
                lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_INLINE"));
                lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_RETEST"));
                return false;
            }

            lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_INLINE"));
            lGexDbQuery.Execute("DROP TEMPORARY TABLE IF EXISTS "+NormalizeTableName("_RUN_RETEST"));

            if(m_eTestingStage == eFinalTest)
            {
                lMessage = "[Record#PRR] Final test with valid PART_X / PART_Y or PART_ID";
                WarningMessage(lMessage);
            }
        }
    }
    else
    {
        // If no Valid XY for WaferSort
        if((m_eTestingStage == eWaferTest) && (!lHaveValidPartXY && lHaveValidPartId))
        {
            lMessage = "[Record#PRR] Wafer without valid PART_X / PART_Y / PART_ID";
            WarningMessage(lMessage);
        }
    }

    // Update NoFullTouchDown
    // If multi-site
    // Check for all consecutif run_id (with PART_RETEST_INDEX=0)
    // if each parts for the N sites was tested
    if(lComputeFullTouchDown)
    {
        if(!lUpdateForRetest)
        {
            // part_flags wasn't reset to 0
            // First, reset the PART_RETEST_INDEX to 0 for ALL
            // and the PART_FLAGS - RETESTED for ALL
            lQuery = "UPDATE "+lRunTable+
                    " SET PART_FLAGS=0 "
                    " WHERE splitlot_id="+QString::number(m_nSplitLotId);
            if(!lGexDbQuery.Execute(lQuery))
            {
                if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            lGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }

        // Generate the TOUCHDOWN_ID for each RUN_ID
        lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TOUCHDOWN"));
        lQuery = "CREATE TEMPORARY TABLE "+NormalizeTableName("_RUN_TOUCHDOWN")+
                " ( "
                " RUN_ID MEDIUMINT(8) unsigned, "
                " TOUCHDOWN_ID MEDIUMINT(8), "
                " SITE_NO SMALLINT(5), "
                " FULL_TOUCHDOWN TINYINT(1) DEFAULT 1, "
                " PRIMARY KEY (RUN_ID), "
                " KEY touchdown_id (TOUCHDOWN_ID) "
                " ) ";
        if(!lGexDbQuery.Execute(lQuery))
        {
            if(m_pbDelayInsertion)  *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        lQuery = "INSERT INTO "+NormalizeTableName("_RUN_TOUCHDOWN")+
                " (RUN_ID, TOUCHDOWN_ID, SITE_NO) "
                " SELECT run_id, "
                "        IF(site_no <= @lastsite0, @touchdown0:=@touchdown0+1, @touchdown0) as touchdown_id, "
                "        @lastsite0:=site_no "
                " FROM "+NormalizeTableName("_RUN")+" r "
                "    , (SELECT @touchdown0:=0) s0, (SELECT @lastsite0:=0) s1 "
                " WHERE splitlot_id="+QString::number(m_nSplitLotId);
        if(!lGexDbQuery.Execute(lQuery))
        {
            if(m_pbDelayInsertion != NULL) *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TOUCHDOWN"));
            return false;
        }

        // Compute the nb of touchdown
        lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TOUCHDOWN_COUNT"));
        lQuery = "CREATE TEMPORARY TABLE "+NormalizeTableName("_RUN_TOUCHDOWN_COUNT")+
                " ( "
                " TOUCHDOWN_ID MEDIUMINT(8), "
                " NB_TOUCHDOWN SMALLINT(5), "
                " PRIMARY KEY (TOUCHDOWN_ID) "
                " ) ";
        if(!lGexDbQuery.Execute(lQuery))
        {
            if(m_pbDelayInsertion != NULL) *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TOUCHDOWN"));
            return false;
        }

        lQuery = "INSERT INTO "+NormalizeTableName("_RUN_TOUCHDOWN_COUNT")+" "
                " SELECT TOUCHDOWN_ID, COUNT(distinct SITE_NO) "
                " FROM "+NormalizeTableName("_RUN_TOUCHDOWN")+
                " GROUP BY TOUCHDOWN_ID";
        if(!lGexDbQuery.Execute(lQuery))
        {
            if(m_pbDelayInsertion != NULL) *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TOUCHDOWN"));
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TOUCHDOWN_COUNT"));
            return false;
        }

        // Update the falgs NO_FULL_TOUCHDOWN
        lQuery = "UPDATE "+NormalizeTableName("_RUN")+" r "
                " INNER JOIN "+NormalizeTableName("_RUN_TOUCHDOWN")+" rt "
                "   ON r.splitlot_id="+QString::number(m_nSplitLotId)+
                "   AND r.run_id=rt.run_id "
                "   AND r.site_no=rt.site_no "
                " INNER JOIN "+NormalizeTableName("_RUN_TOUCHDOWN_COUNT")+" tc "
                "   ON rt.TOUCHDOWN_ID=tc.TOUCHDOWN_ID "
                " SET PART_FLAGS=(PART_FLAGS | "+QString::number(FLAG_RUN_NOFULLTOUCHDOWN)+")"
                " WHERE tc.NB_TOUCHDOWN<"+QString::number(lNbSites);
        if(!lGexDbQuery.Execute(lQuery))
        {
            if(m_pbDelayInsertion != NULL) *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TOUCHDOWN"));
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TOUCHDOWN_COUNT"));
            return false;
        }
        lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TOUCHDOWN"));
        lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TOUCHDOWN_COUNT"));

        /////////////////////////
        // DEADLOCK PREVENTION
        // Commit transaction
        lGexDbQuery.Execute("COMMIT");
    }

    // Update FIRST/LAST test fail
    if(lUpdateTestFailed)
    {
        // For all FAIL parts
        // Need to retreive the FIRST/LAST test fail
        lQuery = "CREATE TEMPORARY TABLE "+NormalizeTableName("_RUN_TEST_FAIL")+
                " ( "
                " SPLITLOT_ID INT(10) unsigned, "
                " RUN_ID MEDIUMINT(8) unsigned, "
                " SITE_NO SMALLINT(5), "
                " first_test_seq smallint(5) unsigned DEFAULT 0, "
                " firstfail_test_type char(1) DEFAULT NULL,"
                " firstfail_test_id smallint(5) DEFAULT NULL,"
                " last_test_seq smallint(5) unsigned DEFAULT 0, "
                " lastfail_test_type char(1) DEFAULT NULL,"
                " lastfail_test_id smallint(5) DEFAULT NULL,"
                " PRIMARY KEY (SPLITLOT_ID,RUN_ID) "
                " ) ";
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TEST_FAIL"));
            return false;
        }

        lQuery = "INSERT INTO "+NormalizeTableName("_RUN_TEST_FAIL")+
                " SELECT SPLITLOT_ID, RUN_ID, SITE_NO FROM "+NormalizeTableName("_RUN")+
                " WHERE SPLITLOT_ID="+QString::number(lSplitlotID)+
                " AND PART_STATUS='F'";
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TEST_FAIL"));
            return false;
        }

        // Update first fail for PTR
        lQuery = "UPDATE "+NormalizeTableName("_RUN_TEST_FAIL")+" RT "
                " INNER JOIN "+NormalizeTableName("_PTEST_RESULTS")+" TR"
                " ON RT.SPLITLOT_ID=TR.SPLITLOT_ID AND RT.RUN_ID=TR.RUN_ID "
                " SET RT.first_test_seq=TR.testseq, RT.firstfail_test_type='P', RT.firstfail_test_id=TR.ptest_info_id "
                " WHERE (TR.RESULT_FLAGS & 1)=0 AND (RT.first_test_seq=0 OR (RT.first_test_seq<TR.testseq)) ";
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TEST_FAIL"));
            return false;
        }
        // Update last fail for PTR
        lQuery = "UPDATE "+NormalizeTableName("_RUN_TEST_FAIL")+" RT "
                " INNER JOIN "+NormalizeTableName("_PTEST_RESULTS")+" TR"
                " ON RT.SPLITLOT_ID=TR.SPLITLOT_ID AND RT.RUN_ID=TR.RUN_ID "
                " SET RT.last_test_seq=TR.testseq, RT.lastfail_test_type='P', RT.lastfail_test_id=TR.ptest_info_id "
                " WHERE (TR.RESULT_FLAGS & 1)=0 AND (RT.last_test_seq=0 OR (RT.first_test_seq>TR.testseq)) ";
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TEST_FAIL"));
            return false;
        }

        // Update first fail for FTR
        lQuery = "UPDATE "+NormalizeTableName("_RUN_TEST_FAIL")+" RT "
                " INNER JOIN "+NormalizeTableName("_FTEST_RESULTS")+" TR"
                " ON RT.SPLITLOT_ID=TR.SPLITLOT_ID AND RT.RUN_ID=TR.RUN_ID "
                " SET RT.first_test_seq=TR.testseq, RT.firstfail_test_type='F', RT.firstfail_test_id=TR.ftest_info_id "
                " WHERE (TR.RESULT_FLAGS & 1)=0 AND (RT.first_test_seq=0 OR (RT.first_test_seq<TR.testseq)) ";
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TEST_FAIL"));
            return false;
        }
        // Update last fail for FTR
        lQuery = "UPDATE "+NormalizeTableName("_RUN_TEST_FAIL")+" RT "
                " INNER JOIN "+NormalizeTableName("_FTEST_RESULTS")+" TR"
                " ON RT.SPLITLOT_ID=TR.SPLITLOT_ID AND RT.RUN_ID=TR.RUN_ID "
                " SET RT.last_test_seq=TR.testseq, RT.lastfail_test_type='F', RT.lastfail_test_id=TR.ftest_info_id "
                " WHERE (TR.RESULT_FLAGS & 1)=0 AND (RT.last_test_seq=0 OR (RT.first_test_seq>TR.testseq)) ";
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TEST_FAIL"));
            return false;
        }

        // Update first fail for MPR
        lQuery = "UPDATE "+NormalizeTableName("_RUN_TEST_FAIL")+" RT "
                " INNER JOIN "+NormalizeTableName("_MPTEST_RESULTS")+" TR"
                " ON RT.SPLITLOT_ID=TR.SPLITLOT_ID AND RT.RUN_ID=TR.RUN_ID "
                " SET RT.first_test_seq=TR.testseq, RT.firstfail_test_type='M', RT.firstfail_test_id=TR.mptest_info_id "
                " WHERE (TR.RESULT_FLAGS & 1)=0 AND (RT.first_test_seq=0 OR (RT.first_test_seq<TR.testseq)) ";
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TEST_FAIL"));
            return false;
        }
        // Update last fail for FTR
        lQuery = "UPDATE "+NormalizeTableName("_RUN_TEST_FAIL")+" RT "
                " INNER JOIN "+NormalizeTableName("_MPTEST_RESULTS")+" TR"
                " ON RT.SPLITLOT_ID=TR.SPLITLOT_ID AND RT.RUN_ID=TR.RUN_ID "
                " SET RT.last_test_seq=TR.testseq, RT.lastfail_test_type='M', RT.lastfail_test_id=TR.mptest_info_id "
                " WHERE (TR.RESULT_FLAGS & 1)=0 AND (RT.last_test_seq=0 OR (RT.first_test_seq>TR.testseq)) ";
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TEST_FAIL"));
            return false;
        }

        // Then update the RUN table
        lQuery = "UPDATE "+NormalizeTableName("_RUN")+" R "+
                " INNER JOIN "+NormalizeTableName("_RUN_TEST_FAIL")+" RT "
                " ON R.SPLITLOT_ID=RT.SPLITLOT_ID AND R.RUN_ID=RT.RUN_ID AND R.SITE_NO=RT.SITE_NO "
                " SET R.firstfail_test_type=RT.firstfail_test_type, R.firstfail_test_id=RT.firstfail_test_id, "
                " R.lastfail_test_type=RT.lastfail_test_type, R.lastfail_test_id=RT.lastfail_test_id ";
        if(!lGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                        lGexDbQuery.lastError().text().toLatin1().constData());
            lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TEST_FAIL"));
            return false;
        }
        lGexDbQuery.Execute("DROP TABLE IF EXISTS "+NormalizeTableName("_RUN_TEST_FAIL"));

        /////////////////////////
        // DEADLOCK PREVENTION
        // Commit transaction
        lGexDbQuery.Execute("COMMIT");
    }
    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateTestConditionTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateTestConditionTable()
{
    if(mpDbKeysEngine == NULL)
        return true;

    // Check if have some TestCondition
    if(mpDbKeysEngine->dbKeysContent().testConditions().isEmpty() &&
            (mpDbKeysEngine->GetTestConditionsOrigin() != GS::QtLib::DatakeysEngine::DTR))
        return true;

    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateTestConditionTable");

    QString strTableName;
    QString lTestConditionValue;
    QString lTestCondKey;
    QStringList lTestConditionsKeys;
    QStringList lContextTestConditions;
    int  lTestConditionIdx;
    structTestInfo *pTest=0;

    // if Test conditions from DTR extract them from tests
    int  lTestInfoIdx;
    bool lDbKeysTestCondOrigin = false;
    if(mpDbKeysEngine->GetTestConditionsOrigin() != GS::QtLib::DatakeysEngine::DTR)
        lDbKeysTestCondOrigin = true;
    for ( lTestInfoIdx = 0; lTestInfoIdx < m_lstTestInfo.size(); ++lTestInfoIdx)
    {
        pTest = m_lstTestInfo.at(lTestInfoIdx);

        lContextTestConditions.append(pTest->mTestConditions.keys());
        if(lDbKeysTestCondOrigin && (!lContextTestConditions.isEmpty()))
        {
            // When the TestCond are defined by the config.gexdbkeys
            // all TestInfo have the same TestCond list
            // Just extract the first one and exit
            break;
        }
    }
    lContextTestConditions.removeDuplicates();

    strTableName = NormalizeTableName("_TEST_CONDITIONS");

    QString strQueryHeader;        // INSERT INTO ...
    QString strNewValue;           // NEW VALUE FOR INSERTION
    QString strAllValues;          // MULTI QUERIES

    // Have to find all test_condition column to be updated
    strQueryHeader = "INSERT INTO " + strTableName + "(SPLITLOT_ID,TEST_INFO_ID,TEST_TYPE";
    // Add the column name for test condition
    for ( lTestConditionIdx = 0; lTestConditionIdx < lContextTestConditions.size(); ++lTestConditionIdx)
    {
        lTestCondKey = lContextTestConditions.at(lTestConditionIdx);
        lTestConditionsKeys.append(lTestCondKey);
        strQueryHeader+= "," + getTestConditionColumn(lTestCondKey);
    }
    strQueryHeader+= ") VALUES";
    strNewValue = strAllValues = "";

    pTest=0;
    for ( lTestInfoIdx = 0; lTestInfoIdx < m_lstTestInfo.size(); ++lTestInfoIdx)
    {
        pTest = m_lstTestInfo.at(lTestInfoIdx);

        // Apply TestCondition process
        bool    bHaveValidValues = false;
        QMap<QString, QString> lTestConditions;
        lTestConditions = pTest->mTestConditions;

        // insert new entry
        strNewValue         = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // TEST_INFO_ID
        strNewValue += QString::number(pTest->m_nInfoId);
        strNewValue += ",";
        // TEST_TYPE
        if(pTest->m_nTestType == GQTL_STDF::Stdf_Record::Rec_PTR)
            strNewValue += "'P'";
        else if(pTest->m_nTestType == GQTL_STDF::Stdf_Record::Rec_MPR)
            strNewValue += "'M'";
        else
            strNewValue += "'F'";
        // Update TestCondition
        for ( int lIter = 0; lIter < lTestConditionsKeys.size(); ++lIter )
        {
            lTestConditionValue = lTestConditions.value(lTestConditionsKeys.at(lIter));
            strNewValue+= ",";
            if(lTestConditionValue.isEmpty())
                strNewValue+= TranslateStringToSqlVarChar("n/a");
            else
            {
                strNewValue+= TranslateStringToSqlVarChar(lTestConditionValue);
                bHaveValidValues = true;
            }
        }
        strNewValue += ")";

        // If no valid test condition, do not insert line
        if(!bHaveValidValues)
            continue;

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
            return false;
    }
    if(!strAllValues.isEmpty())
    {
        if(!ExecuteMultiInsertQuery(strTableName, strAllValues))
            return false;
    }

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateTestConditionTable completed");

    // Unlock table
    UnlockTables();

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdatePTestInfoTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdatePTestInfoTable()
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdatePTestInfoTable");

    QString strTableName;

    if((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PTR]==0)
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR]==0))
        return true;

    QMap<QString,qint64> DbTestNames;
    qint64 MaxTestNumber = 0;
    if(mAutoincrementTestNumbers)
    {
        // Retrieve the list of tests from DB and the associated TestNumber
        GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
        QString strQuery;
        strQuery = "SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" DISTINCT tnum,tname FROM "+NormalizeTableName("_SPLITLOT");
        strQuery+= " S LEFT OUTER JOIN "+NormalizeTableName("_PTEST_INFO");
        strQuery+= " T ON S.splitlot_id=T.splitlot_id";
        strQuery+= " WHERE ";
        strQuery+= " S.VALID_SPLITLOT<>'N'";
        if(!IsCharacTdr())
        {
            // For Charac TDR, Get the TNum from all the TDR
            // For other, restrict on the current Lot
            strQuery+= " AND S.lot_id="+TranslateStringToSqlVarChar(m_strLotId);
        }
        if(!clGexDbQuery.Execute(strQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        while(clGexDbQuery.Next())
        {
            if(DbTestNames.contains(clGexDbQuery.value(1).toString())
                    && (DbTestNames[clGexDbQuery.value(1).toString()] != clGexDbQuery.value(0).toUInt()))
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Autoincrement Test Numbers WARNING: found a same TestName[%1] from DB with a different TestNumber. Keep the last TestNumber")
                      .arg(clGexDbQuery.value(1).toString()).toLatin1().constData());
            }
            DbTestNames[clGexDbQuery.value(1).toString()]=clGexDbQuery.value(0).toUInt();
            if(MaxTestNumber < clGexDbQuery.value(0).toUInt())
                MaxTestNumber = clGexDbQuery.value(0).toUInt();
        }
    }

    strTableName = NormalizeTableName("_PTEST_INFO");

    QString strQueryHeader;        // INSERT INTO ...
    QString strNewValue;           // NEW VALUE FOR INSERTION
    QString strAllValues;          // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName + " VALUES";
    strNewValue = strAllValues = "";

    int nInfoId = 1;
    structTestInfo *pTest=0;

    int  lTestInfoIdx;
    for ( lTestInfoIdx = 0; lTestInfoIdx < m_lstTestInfo.size(); ++lTestInfoIdx)
    {
        pTest = m_lstTestInfo.at(lTestInfoIdx);

        if(pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_PTR)
            continue;

        // if multisites
        // verify if multi limit definition
        bool bMultiLimits = false;
        float        fLL, fHL;


        bool bValidLL, bValidHL;
        bValidLL = !pTest->m_mapfLL.isEmpty();
        bValidHL = !pTest->m_mapfHL.isEmpty();

        // if have limit => have MERGE_SITE
        if(pTest->m_mapfLL.contains(MERGE_SITE))
            fLL = pTest->m_mapfLL[MERGE_SITE];
        if(pTest->m_mapfHL.contains(MERGE_SITE))
            fHL = pTest->m_mapfHL[MERGE_SITE];

        if(m_nNbSites > 1)
        {
            // SC 2010 02 11
            // Nb limits definition per site can be diff between HL and LL
            // Check limits with all avalaible sites
            QList<int> lstAllSites;
            QList<int>::Iterator itSite;
            QMap<int,float>::Iterator itLimitSite;

            // Retrieve all sites used for LL and HL
            for ( itLimitSite = pTest->m_mapfLL.begin(); itLimitSite != pTest->m_mapfLL.end(); ++itLimitSite )
                lstAllSites.append(itLimitSite.key());
            for ( itLimitSite = pTest->m_mapfHL.begin(); itLimitSite != pTest->m_mapfHL.end(); ++itLimitSite )
                if(!lstAllSites.contains(itLimitSite.key()))
                    lstAllSites.append(itLimitSite.key());

            // For each site
            // Check LL and HL
            for ( itSite = lstAllSites.begin(); itSite != lstAllSites.end(); ++itSite )
            {
                if(*itSite == MERGE_SITE)
                    continue;
                if((pTest->m_mapfLL.contains(*itSite) && (fLL != pTest->m_mapfLL[*itSite]))
                        || (pTest->m_mapfHL.contains(*itSite) && (fHL != pTest->m_mapfHL[*itSite])))
                {
                    // BG 4/4/08: enable writing of first limit even in mult-limit mode
                    // fHL = fLL = 0;
                    bMultiLimits = true;
                    if(pTest->m_mapfLL.contains(*itSite))
                        fLL = qMin(fLL, pTest->m_mapfLL[*itSite]);
                    if(pTest->m_mapfHL.contains(*itSite))
                        fHL = qMax(fHL, pTest->m_mapfHL[*itSite]);

                    if((!m_mapPCRNbParts.contains(*itSite) || (m_mapPCRNbParts.contains(*itSite) && (m_mapPCRNbParts[*itSite]==0))) // No summary info
                            && (!m_mapNbRuns.contains(*itSite) || (m_mapNbRuns.contains(*itSite) && (m_mapNbRuns[*itSite]==0)))) // No samples info
                    {
                        // This is the reference site for limits (site = 255)
                        if(pTest->m_mapfLL.contains(*itSite))
                            fLL = pTest->m_mapfLL[*itSite];
                        if(pTest->m_mapfHL.contains(*itSite))
                            fHL = pTest->m_mapfHL[*itSite];

                        break;
                    }
                }
            }
        }

        // Apply TestCondition process
        QString strTestName = pTest->m_strTestName;
        qint64  nTestNum = pTest->m_nOriginalTestNum;
        if(mpDbKeysEngine
                && !mpDbKeysEngine->dbKeysContent().testAttributes().isEmpty())
        {
            int     nLineError;
            bool    bNoError;
            QString strError;
            QString strValue;
            mpDbKeysEngine->dbKeysContent().SetDbKeyContent("test[name]",strTestName);
            mpDbKeysEngine->dbKeysContent().SetDbKeyContent("test[number]",QString::number(nTestNum));
            bNoError = mpDbKeysEngine->evaluateDynamicDbKeys(nLineError,strError);
            if(bNoError)
            {
                mpDbKeysEngine->dbKeysContent().GetDbKeyContent("test[name]",strTestName);
                mpDbKeysEngine->dbKeysContent().GetDbKeyContent("test[number]",strValue);
                if(!mAutoincrementTestNumbers)
                    nTestNum = strValue.toUInt(&bNoError);

                // Keep Test Conditions
                pTest->mTestConditions = mpDbKeysEngine->dbKeysContent().testConditions();
            }
            if(!bNoError)
            {
                strValue = "DbKeyConfig file at line "+QString::number(nLineError);
                if(nLineError == 0)
                    strValue = "DbKeyConfig file at line test[number]";
                if(strError.isEmpty())
                    strError = "Invalid expression - not a valid number";

                if(m_pbDelayInsertion) *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                            QString(strValue).toLatin1().constData(),
                            strError.toLatin1().constData());
                return false;
            }
        }

        if(mAutoincrementTestNumbers)
        {
            // Apply the associated TestNumber
            if(DbTestNames.contains(strTestName))
                nTestNum = DbTestNames[strTestName];
            else
            {
                nTestNum = MaxTestNumber;
                ++MaxTestNumber;
                DbTestNames[strTestName] = nTestNum;
            }
        }
        // insert new entry
        strNewValue         = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // PTEST_INFO_ID
        strNewValue += QString::number(nInfoId);
        strNewValue += ",";
        // TNUM
        strNewValue += QString::number(nTestNum);
        strNewValue += ",";
        // TNAME
        strNewValue += TranslateStringToSqlVarChar(strTestName);
        strNewValue += ",";
        // UNITS
        strNewValue += TranslateStringToSqlVarChar(pTest->m_strUnits);
        strNewValue += ",";
        // TEST_FLAGS
        strNewValue += QString::number(pTest->m_cFlags);
        strNewValue += ",";
        // LL
        strNewValue += !bValidLL ? "null" : NormalizeNumber(fLL);
        strNewValue += ",";
        // HL
        strNewValue += !bValidHL ? "null" : NormalizeNumber(fHL);
        strNewValue += ",";
        // TESTSEQ
        strNewValue += pTest->m_nTestSeq == -1 ? "null" : QString::number(pTest->m_nTestSeq);
        strNewValue += ",";
        // SPEC_LL
        strNewValue += pTest->m_bHaveSpecLL ? NormalizeNumber(pTest->m_fSpecLL) : "null";
        strNewValue += ",";
        // SPEC_HL
        strNewValue += pTest->m_bHaveSpecHL ? NormalizeNumber(pTest->m_fSpecHL) : "null";
        strNewValue += ",";
        // SPEC_TARGET
        strNewValue += pTest->m_bHaveSpecTarget ? NormalizeNumber(pTest->m_fSpecTarget) : "null";
        strNewValue += ",";
        // RES_SCAL
        strNewValue += pTest->m_bHaveScalRes ? NormalizeNumber(pTest->m_nScalRes) : "null";
        strNewValue += ",";
        // LL_SCAL
        strNewValue += pTest->m_bHaveScalLL ? NormalizeNumber(pTest->m_nScalLL) : "null";
        strNewValue += ",";
        // HL_SCAL
        strNewValue += pTest->m_bHaveScalHL ? NormalizeNumber(pTest->m_nScalHL) : "null";
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
            return false;

        pTest->m_nInfoId = nInfoId;
        nInfoId++;

        if(bMultiLimits)
            if(!UpdatePTestLimitsTable(pTest))
                return false;

        if(!UpdatePTestStaticLimitsTable(*pTest))
            return false;
        if(!UpdatePTestStatsSamplesTable(pTest))
            return false;
        if(!UpdatePTestStatsSummaryTable(pTest))
            return false;
    }
    if(!strAllValues.isEmpty())
    {
        if(!ExecuteMultiInsertQuery(strTableName, strAllValues))
            return false;
    }

    if(!mQueryPTestLimits.isEmpty())
    {
        strTableName = NormalizeTableName("_PTEST_LIMITS");

        if(!ExecuteMultiInsertQuery(strTableName, mQueryPTestLimits))
            return false;
        mQueryPTestLimits = "";
    }
    if(!mQueryPTestStaticLimits.isEmpty())
    {
        strTableName = NormalizeTableName("_PTEST_STATIC_LIMITS");

        if(!ExecuteMultiInsertQuery(strTableName, mQueryPTestStaticLimits))
            return false;
        mQueryPTestStaticLimits = "";
    }
    if(!mQuerySamplesPTest.isEmpty())
    {
        if(m_eTestingStage == eElectTest)
            strTableName = NormalizeTableName("_PTEST_STATS");
        else
            strTableName = NormalizeTableName("_PTEST_STATS_SAMPLES");

        if(!ExecuteMultiInsertQuery(strTableName, mQuerySamplesPTest))
            return false;
        mQuerySamplesPTest = "";
    }
    if(!mQuerySummaryPTest.isEmpty())
    {
        if(m_eTestingStage != eElectTest)
        {
            strTableName = NormalizeTableName("_PTEST_STATS_SUMMARY");
            if(!ExecuteMultiInsertQuery(strTableName, mQuerySummaryPTest))
                return false;
        }
        mQuerySummaryPTest = "";
    }

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdatePTestInfoTable completed");

    // Unlock table
    UnlockTables();

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdatePTestLimitsTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdatePTestLimitsTable(structTestInfo *pTest)
{
    QString strTableName;

    if(m_eTestingStage == eElectTest)
        return true;

    strTableName = NormalizeTableName("_PTEST_LIMITS");
    QString strQueryHeader;                // INSERT INTO ...
    QString strNewValue;                // NEW VALUE FOR INSERTION
    QString strAllValues;                // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName + " VALUES";
    strNewValue = strAllValues = "";

    QString strQuery;

    // SC 2010 02 11
    // Nb limits definition per site can be diff between HL and LL
    // Check limits with all avalaible sites
    QList<int> lstAllSites;
    QList<int>::Iterator itSite;
    QMap<int,float>::Iterator itLimitSite;

    // Retrieve all sites used for LL and HL
    for ( itLimitSite = pTest->m_mapfLL.begin(); itLimitSite != pTest->m_mapfLL.end(); ++itLimitSite )
        lstAllSites.append(itLimitSite.key());
    for ( itLimitSite = pTest->m_mapfHL.begin(); itLimitSite != pTest->m_mapfHL.end(); ++itLimitSite )
        if(!lstAllSites.contains(itLimitSite.key()))
            lstAllSites.append(itLimitSite.key());

    // For each site
    // Check LL and HL
    for ( itSite = lstAllSites.begin(); itSite != lstAllSites.end(); ++itSite )
    {
        if(*itSite == MERGE_SITE)
            continue;

        // Have to check if have execution on this site
        // else ignore this limits set
        if((!m_mapPCRNbParts.contains(*itSite) || (m_mapPCRNbParts.contains(*itSite) && (m_mapPCRNbParts[*itSite]==0))) // No summary info
                && (!m_mapNbRuns.contains(*itSite) || (m_mapNbRuns.contains(*itSite) && (m_mapNbRuns[*itSite]==0)))) // No samples info
            continue;

        strNewValue  = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // PTEST_INFO_ID
        strNewValue += QString::number(pTest->m_nInfoId);
        strNewValue += ",";
        // SITE_NO
        strNewValue += QString::number(*itSite);
        strNewValue += ",";
        // LL
        strNewValue += !(pTest->m_mapfLL.contains(*itSite)) ? "null" : NormalizeNumber(pTest->m_mapfLL[*itSite]);
        strNewValue += ",";
        // HL
        strNewValue += !(pTest->m_mapfHL.contains(*itSite)) ? "null" : NormalizeNumber(pTest->m_mapfHL[*itSite]);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader,strNewValue,mQueryPTestLimits))
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdatePTestStaticLimitsTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdatePTestStaticLimitsTable(structTestInfo &cTest)
{
    QString lTableName;

    if(m_eTestingStage == eElectTest)
        return true;

    lTableName = NormalizeTableName("_PTEST_STATIC_LIMITS");
    QString lQueryHeader;                // INSERT INTO ...
    QString lNewValue;                // NEW VALUE FOR INSERTION
    QString lAllValues;                // MULTI QUERIES

    lQueryHeader = "INSERT INTO " + lTableName + "(splitlot_id,ptest_info_id,limit_id,site_no,hbin_no,sbin_no,ll,hl) VALUES";
    lNewValue = lAllValues = "";

    int lIndex;

    for ( lIndex = 0; lIndex < cTest.mMultiLimits.count(); ++lIndex )
    {
        lNewValue  = "(";
        // SPLITLOT_ID
        lNewValue += QString::number(m_nSplitLotId);
        lNewValue += ",";
        // PTEST_INFO_ID
        lNewValue += QString::number(cTest.m_nInfoId);
        lNewValue += ",";
        // LIMIT_ID
        lNewValue += QString::number(lIndex);
        lNewValue += ",";
        // SITE_NO
        lNewValue += !cTest.mMultiLimits[lIndex].IsValidSite() ? "null" : QString::number(cTest.mMultiLimits[lIndex].GetSite());
        lNewValue += ",";
        // HBIN_NO
        lNewValue += !cTest.mMultiLimits[lIndex].IsValidHardBin() ? "null" : QString::number(cTest.mMultiLimits[lIndex].GetHardBin());
        lNewValue += ",";
        // SBIN_NO
        lNewValue += !cTest.mMultiLimits[lIndex].IsValidSoftBin() ? "null" : QString::number(cTest.mMultiLimits[lIndex].GetSoftBin());
        lNewValue += ",";
        // LL
        lNewValue += !cTest.mMultiLimits[lIndex].IsValidLowLimit() ? "null" : NormalizeNumber(cTest.mMultiLimits[lIndex].GetLowLimit());
        lNewValue += ",";
        // HL
        lNewValue += !cTest.mMultiLimits[lIndex].IsValidHighLimit() ? "null" : NormalizeNumber(cTest.mMultiLimits[lIndex].GetHighLimit());
        lNewValue += ")";

        if(!AddInMultiInsertQuery(lTableName, lQueryHeader,lNewValue,mQueryPTestStaticLimits))
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateMPTestInfoTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateMPTestInfoTable()
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateMPTestInfoTable");

    QString strTableName;

    if((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR]==0)
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR]==0))
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    QMap<QString,qint64> DbTestNames;
    qint64 MaxTestNumber = 0;
    if(mAutoincrementTestNumbers)
    {
        // Retrieve the list of tests from DB and the associated TestNumber
        GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
        QString strQuery;
        strQuery = "SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" DISTINCT tnum,tname,tpin_arrayindex FROM "+NormalizeTableName("_SPLITLOT");
        strQuery+= " S LEFT OUTER JOIN "+NormalizeTableName("_MPTEST_INFO");
        strQuery+= " T ON S.splitlot_id=T.splitlot_id";
        strQuery+= " WHERE ";
        strQuery+= " S.VALID_SPLITLOT<>'N'";
        if(!IsCharacTdr())
        {
            // For Charac TDR, Get the TNum from all the TDR
            // For other, restrict on the current Lot
            strQuery+= " AND S.lot_id="+TranslateStringToSqlVarChar(m_strLotId);
        }
        if(!clGexDbQuery.Execute(strQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        while(clGexDbQuery.Next())
        {
            if(DbTestNames.contains(clGexDbQuery.value(1).toString()+"-"+clGexDbQuery.value(2).toString())
                    && (DbTestNames[clGexDbQuery.value(1).toString()+"-"+clGexDbQuery.value(2).toString()] != clGexDbQuery.value(0).toUInt()))
            {
                GSLOG(SYSLOG_SEV_ERROR,QString("Autoincrement Test Numbers WARNING: found a same TestName[%1] from DB with a different TestNumber. Keep the last TestNumber").arg(
                          clGexDbQuery.value(1).toString()).toLatin1().constData());
            }
            DbTestNames[clGexDbQuery.value(1).toString()+"-"+clGexDbQuery.value(2).toString()]=clGexDbQuery.value(0).toUInt();
            if(MaxTestNumber < clGexDbQuery.value(0).toUInt())
                MaxTestNumber = clGexDbQuery.value(0).toUInt();
        }
    }

    strTableName = NormalizeTableName("_MPTEST_INFO");

    QString strQueryHeader;                // INSERT INTO ...
    QString strNewValue;                // NEW VALUE FOR INSERTION
    QString strAllValues;                // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName + " VALUES";
    strNewValue = strAllValues = "";

    int nInfoId = 1;
    structTestInfo *pTest=0;
    int  lTestInfoIdx;
    for ( lTestInfoIdx = 0; lTestInfoIdx < m_lstTestInfo.size(); ++lTestInfoIdx)
    {
        pTest = m_lstTestInfo.at(lTestInfoIdx);

        if(pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_MPR)
            continue;

        // if multisites
        // verify if multi limit definition
        bool bMultiLimits = false;
        float        fLL, fHL;

        bool bValidLL, bValidHL;
        bValidLL = !pTest->m_mapfLL.isEmpty();
        bValidHL = !pTest->m_mapfHL.isEmpty();

        // if have limit => have MERGE_SITE
        if(pTest->m_mapfLL.contains(MERGE_SITE))
            fLL = pTest->m_mapfLL[MERGE_SITE];
        if(pTest->m_mapfHL.contains(MERGE_SITE))
            fHL = pTest->m_mapfHL[MERGE_SITE];

        if(m_nNbSites > 1)
        {
            // SC 2010 02 11
            // Nb limits definition per site can be diff between HL and LL
            // Check limits with all avalaible sites
            QList<int> lstAllSites;
            QList<int>::Iterator itSite;
            QMap<int,float>::Iterator itLimitSite;

            // Retrieve all sites used for LL and HL
            for ( itLimitSite = pTest->m_mapfLL.begin(); itLimitSite != pTest->m_mapfLL.end(); ++itLimitSite )
                lstAllSites.append(itLimitSite.key());
            for ( itLimitSite = pTest->m_mapfHL.begin(); itLimitSite != pTest->m_mapfHL.end(); ++itLimitSite )
                if(!lstAllSites.contains(itLimitSite.key()))
                    lstAllSites.append(itLimitSite.key());

            // For each site
            // Check LL and HL
            for ( itSite = lstAllSites.begin(); itSite != lstAllSites.end(); ++itSite )
            {
                if(*itSite == MERGE_SITE)
                    continue;

                if((pTest->m_mapfLL.contains(*itSite) && (fLL != pTest->m_mapfLL[*itSite]))
                        || (pTest->m_mapfHL.contains(*itSite) && (fHL != pTest->m_mapfHL[*itSite])))
                {
                    fHL = fLL = 0;
                    bMultiLimits = true;
                    break;
                }
            }
        }
        // Apply TestCondition process
        QString strTestName = pTest->m_strTestName;
        qint64  nTestNum = pTest->m_nOriginalTestNum;
        if(mpDbKeysEngine
                && !mpDbKeysEngine->dbKeysContent().testAttributes().isEmpty())
        {
            int     nLineError;
            bool    bNoError;
            QString strError;
            QString strValue;
            mpDbKeysEngine->dbKeysContent().SetDbKeyContent("test[name]",strTestName);
            mpDbKeysEngine->dbKeysContent().SetDbKeyContent("test[number]",QString::number(nTestNum));
            bNoError = mpDbKeysEngine->evaluateDynamicDbKeys(nLineError,strError);
            if(bNoError)
            {
                mpDbKeysEngine->dbKeysContent().GetDbKeyContent("test[name]",strTestName);
                mpDbKeysEngine->dbKeysContent().GetDbKeyContent("test[number]",strValue);
                if(!mAutoincrementTestNumbers)
                    nTestNum = strValue.toUInt(&bNoError);

                // Keep Test Conditions
                pTest->mTestConditions = mpDbKeysEngine->dbKeysContent().testConditions();
            }
            if(!bNoError)
            {
                strValue = "DbKeyConfig file at line "+QString::number(nLineError);
                if(nLineError == 0)
                    strValue = "DbKeyConfig file at line test[number]";
                if(strError.isEmpty())
                    strError = "Invalid expression - not a valid number";

                if(m_pbDelayInsertion) *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                            QString(strValue).toLatin1().constData(),
                            strError.toLatin1().constData());
                return false;
            }
        }

        if(mAutoincrementTestNumbers)
        {
            // Apply the associated TestNumber
            if(DbTestNames.contains(strTestName+"-"+QString::number(pTest->m_nTestPinIndex)))
                nTestNum = DbTestNames[strTestName+"-"+QString::number(pTest->m_nTestPinIndex)];
            else
            {
                nTestNum = MaxTestNumber;
                MaxTestNumber++;
                DbTestNames[strTestName]=nTestNum;
            }
        }
        // insert new entry
        strNewValue = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // MPTEST_INFO_ID
        strNewValue += QString::number(nInfoId);
        strNewValue += ",";
        // TEST_NUM
        strNewValue += QString::number(nTestNum);
        strNewValue += ",";
        // TEST_NAME
        strNewValue += TranslateStringToSqlVarChar(strTestName);
        strNewValue += ",";
        // TPIN_ARRAYINDEX // SC 20080918 = null not allowed : use -1 for undefined value
        strNewValue += QString::number(pTest->m_nTestPinIndex);
        strNewValue += ",";
        // UNITS
        strNewValue += TranslateStringToSqlVarChar(pTest->m_strUnits);
        strNewValue += ",";
        // TEST_FLAGS
        strNewValue += QString::number(pTest->m_cFlags);
        strNewValue += ",";
        // LL
        strNewValue += bMultiLimits || !bValidLL ? "null" : NormalizeNumber(fLL);
        strNewValue += ",";
        // HL
        strNewValue += bMultiLimits || !bValidHL ? "null" : NormalizeNumber(fHL);
        strNewValue += ",";
        // TESTSEQ
        strNewValue += pTest->m_nTestSeq == -1 ? "null" : QString::number(pTest->m_nTestSeq);
        strNewValue += ",";
        // SPEC_LL
        strNewValue += pTest->m_bHaveSpecLL ? NormalizeNumber(pTest->m_fSpecLL) : "null";
        strNewValue += ",";
        // SPEC_HL
        strNewValue += pTest->m_bHaveSpecHL ? NormalizeNumber(pTest->m_fSpecHL) : "null";
        strNewValue += ",";
        // SPEC_TARGET
        strNewValue += pTest->m_bHaveSpecTarget ? NormalizeNumber(pTest->m_fSpecTarget) : "null";
        strNewValue += ",";
        // RES_SCAL
        strNewValue += pTest->m_bHaveScalRes ? NormalizeNumber(pTest->m_nScalRes) : "null";
        strNewValue += ",";
        // LL_SCAL
        strNewValue += pTest->m_bHaveScalLL ? NormalizeNumber(pTest->m_nScalLL) : "null";
        strNewValue += ",";
        // HL_SCAL
        strNewValue += pTest->m_bHaveScalHL ? NormalizeNumber(pTest->m_nScalHL) : "null";
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
            return false;

        pTest->m_nInfoId = nInfoId;
        nInfoId++;

        if(bMultiLimits)
            if(!UpdateMPTestLimitsTable(pTest))
                return false;
        if(!UpdateMPTestStaticLimitsTable(*pTest))
            return false;
        if(!UpdateMPTestStatsSamplesTable(pTest))
            return false;
        if(!UpdateMPTestStatsSummaryTable(pTest))
            return false;

    }
    if(!strAllValues.isEmpty())
    {
        // execute query
        if(!ExecuteMultiInsertQuery(strTableName, strAllValues))
            return false;
    }
    if(!mQueryMPTestLimits.isEmpty())
    {
        strTableName = NormalizeTableName("_MPTEST_LIMITS");

        // execute query
        if(!ExecuteMultiInsertQuery(strTableName, mQueryMPTestLimits))
            return false;
        mQueryMPTestLimits = "";
    }
    if(!mQueryMPTestStaticLimits.isEmpty())
    {
        strTableName = NormalizeTableName("_MPTEST_STATIC_LIMITS");

        // execute query
        if(!ExecuteMultiInsertQuery(strTableName, mQueryMPTestStaticLimits))
            return false;
        mQueryMPTestStaticLimits = "";
    }
    if(!mQuerySamplesMPTest.isEmpty())
    {
        strTableName = NormalizeTableName("_MPTEST_STATS_SAMPLES");

        // execute query
        if(!ExecuteMultiInsertQuery(strTableName, mQuerySamplesMPTest))
            return false;
        mQuerySamplesMPTest = "";
    }
    if(!mQuerySummaryMPTest.isEmpty())
    {
        strTableName = NormalizeTableName("_MPTEST_STATS_SUMMARY");

        // execute query
        if(!ExecuteMultiInsertQuery(strTableName, mQuerySummaryMPTest))
            return false;
        mQuerySummaryMPTest = "";
    }
    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateMPTestInfoTable completed");

    // Unlock table
    UnlockTables();

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateMPTestLimitsTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateMPTestLimitsTable(structTestInfo *pTest)
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateMPTestLimitsTable");

    QString strTableName;

    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    strTableName = NormalizeTableName("_MPTEST_LIMITS");

    QString strQueryHeader;                // INSERT INTO ...
    QString strNewValue;                // NEW VALUE FOR INSERTION
    QString strAllValues;                // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName + " VALUES";
    strNewValue = strAllValues = "";

    // SC 2010 02 11
    // Nb limits definition per site can be diff between HL and LL
    // Check limits with all avalaible sites
    QList<int> lstAllSites;
    QList<int>::Iterator itSite;
    QMap<int,float>::Iterator itLimitSite;

    // Retrieve all sites used for LL and HL
    for ( itLimitSite = pTest->m_mapfLL.begin(); itLimitSite != pTest->m_mapfLL.end(); ++itLimitSite )
        lstAllSites.append(itLimitSite.key());
    for ( itLimitSite = pTest->m_mapfHL.begin(); itLimitSite != pTest->m_mapfHL.end(); ++itLimitSite )
        if(!lstAllSites.contains(itLimitSite.key()))
            lstAllSites.append(itLimitSite.key());

    // For each site
    // Check LL and HL
    for ( itSite = lstAllSites.begin(); itSite != lstAllSites.end(); ++itSite )
    {
        if(*itSite == MERGE_SITE)
            continue;

        // Have to check if have execution on this site
        // else ignore this limits set
        if((!m_mapPCRNbParts.contains(*itSite) || (m_mapPCRNbParts.contains(*itSite) && (m_mapPCRNbParts[*itSite]==0))) // No summary info
                && (!m_mapNbRuns.contains(*itSite) || (m_mapNbRuns.contains(*itSite) && (m_mapNbRuns[*itSite]==0)))) // No samples info
            continue;

        // insert new entry
        strNewValue = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // MPTEST_INFO_ID
        strNewValue += QString::number(pTest->m_nInfoId);
        strNewValue += ",";
        // SITE_NO
        strNewValue += QString::number(*itSite);
        strNewValue += ",";
        // LL
        strNewValue += !pTest->m_mapfLL.contains(*itSite) ? "null" : NormalizeNumber(pTest->m_mapfLL[*itSite]);
        strNewValue += ",";
        // HL
        strNewValue += !pTest->m_mapfHL.contains(*itSite) ? "null" : NormalizeNumber(pTest->m_mapfHL[*itSite]);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, mQueryMPTestLimits))
            return false;
    }

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateMPTestLimitsTable completed");

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateMPTestStaticLimitsTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateMPTestStaticLimitsTable(structTestInfo &cTest)
{
    // not implemented
    return true;

    QString lTableName;

    if(m_eTestingStage == eElectTest)
        return true;

    lTableName = NormalizeTableName("_MPTEST_STATIC_LIMITS");
    QString lQueryHeader;                // INSERT INTO ...
    QString lNewValue;                // NEW VALUE FOR INSERTION
    QString lAllValues;                // MULTI QUERIES

    lQueryHeader = "INSERT INTO " + lTableName + "(splitlot_id,mptest_info_id,limit_id,site_no,hbin_no,sbin_no,ll,hl) VALUES";
    lNewValue = lAllValues = "";

    int lIndex;

    for ( lIndex = 0; lIndex < cTest.mMultiLimits.count(); ++lIndex )
    {
        lNewValue  = "(";
        // SPLITLOT_ID
        lNewValue += QString::number(m_nSplitLotId);
        lNewValue += ",";
        // MPTEST_INFO_ID
        lNewValue += QString::number(cTest.m_nInfoId);
        lNewValue += ",";
        // LIMIT_ID
        lNewValue += QString::number(lIndex);
        lNewValue += ",";
        // SITE_NO
        lNewValue += !cTest.mMultiLimits[lIndex].IsValidSite() ? "null" : QString::number(cTest.mMultiLimits[lIndex].GetSite());
        lNewValue += ",";
        // HBIN_NO
        lNewValue += !cTest.mMultiLimits[lIndex].IsValidHardBin() ? "null" : QString::number(cTest.mMultiLimits[lIndex].GetHardBin());
        lNewValue += ",";
        // SBIN_NO
        lNewValue += !cTest.mMultiLimits[lIndex].IsValidSoftBin() ? "null" : QString::number(cTest.mMultiLimits[lIndex].GetSoftBin());
        lNewValue += ",";
        // LL
        lNewValue += !cTest.mMultiLimits[lIndex].IsValidLowLimit() ? "null" : NormalizeNumber(cTest.mMultiLimits[lIndex].GetLowLimit());
        lNewValue += ",";
        // HL
        lNewValue += !cTest.mMultiLimits[lIndex].IsValidHighLimit() ? "null" : NormalizeNumber(cTest.mMultiLimits[lIndex].GetHighLimit());
        lNewValue += ")";

        if(!AddInMultiInsertQuery(lTableName, lQueryHeader,lNewValue,mQueryMPTestStaticLimits))
            return false;
    }

    return true;
}


///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateFTestInfoTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateFTestInfoTable()
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdateFTestInfoTable");

    QString strTableName;

    if((m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_FTR]==0)
            && (m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR]==0))
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    QMap<QString,qint64> DbTestNames;
    qint64 MaxTestNumber = 0;
    if(mAutoincrementTestNumbers)
    {
        // Retrieve the list of tests from DB and the associated TestNumber
        GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
        QString strQuery;
        strQuery = "SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" DISTINCT tnum,tname FROM "+NormalizeTableName("_SPLITLOT");
        strQuery+= " S LEFT OUTER JOIN "+NormalizeTableName("_FTEST_INFO");
        strQuery+= " T ON S.splitlot_id=T.splitlot_id";
        strQuery+= " WHERE ";
        strQuery+= " S.VALID_SPLITLOT<>'N'";
        if(!IsCharacTdr())
        {
            // For Charac TDR, Get the TNum from all the TDR
            // For other, restrict on the current Lot
            strQuery+= " AND S.lot_id="+TranslateStringToSqlVarChar(m_strLotId);
        }
        if(!clGexDbQuery.Execute(strQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        while(clGexDbQuery.Next())
        {
            if(DbTestNames.contains(clGexDbQuery.value(1).toString())
                    && (DbTestNames[clGexDbQuery.value(1).toString()] != clGexDbQuery.value(0).toUInt()))
            {
                GSLOG(SYSLOG_SEV_ERROR,QString("Autoincrement Test Numbers WARNING: found a same TestName[%1] from DB with a different TestNumber. Keep the last TestNumber").arg(
                          clGexDbQuery.value(1).toString()).toLatin1().constData());
            }
            DbTestNames[clGexDbQuery.value(1).toString()]=clGexDbQuery.value(0).toUInt();
            if(MaxTestNumber < clGexDbQuery.value(0).toUInt())
                MaxTestNumber = clGexDbQuery.value(0).toUInt();
        }
    }

    strTableName = NormalizeTableName("_FTEST_INFO");

    QString strQueryHeader; // INSERT INTO ...
    QString strNewValue;    // NEW VALUE FOR INSERTION
    QString strAllValues;   // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName + " VALUES";
    strNewValue = strAllValues = "";

    structTestInfo *pTest=0;
    int nInfoId = 1;

    int  lTestInfoIdx;
    for ( lTestInfoIdx = 0; lTestInfoIdx < m_lstTestInfo.size(); ++lTestInfoIdx)
    {
        pTest = m_lstTestInfo.at(lTestInfoIdx);

        if(pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_FTR)
            continue;

        // Apply TestCondition process
        QString strTestName = pTest->m_strTestName;
        qint64  nTestNum = pTest->m_nOriginalTestNum;
        if(mpDbKeysEngine
                && !mpDbKeysEngine->dbKeysContent().testAttributes().isEmpty())
        {
            int     nLineError;
            bool    bNoError;
            QString strError;
            QString strValue;
            mpDbKeysEngine->dbKeysContent().SetDbKeyContent("test[name]",strTestName);
            mpDbKeysEngine->dbKeysContent().SetDbKeyContent("test[number]",QString::number(nTestNum));
            bNoError = mpDbKeysEngine->evaluateDynamicDbKeys(nLineError,strError);
            if(bNoError)
            {
                mpDbKeysEngine->dbKeysContent().GetDbKeyContent("test[name]",strTestName);
                mpDbKeysEngine->dbKeysContent().GetDbKeyContent("test[number]",strValue);
                if(!mAutoincrementTestNumbers)
                    nTestNum = strValue.toUInt(&bNoError);

                // Keep Test Conditions
                pTest->mTestConditions = mpDbKeysEngine->dbKeysContent().testConditions();
            }
            if(!bNoError)
            {
                strValue = "DbKeyConfig file at line "+QString::number(nLineError);
                if(nLineError == 0)
                    strValue = "DbKeyConfig file at line test[number]";
                if(strError.isEmpty())
                    strError = "Invalid expression - not a valid number";

                if(m_pbDelayInsertion) *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                            QString(strValue).toLatin1().constData(),
                            strError.toLatin1().constData());
                return false;
            }
        }

        if(mAutoincrementTestNumbers)
        {
            // Apply the associated TestNumber
            if(DbTestNames.contains(strTestName))
                nTestNum = DbTestNames[strTestName];
            else
            {
                nTestNum = MaxTestNumber;
                MaxTestNumber++;
                DbTestNames[strTestName]=nTestNum;
            }
        }
        // insert new entry
        strNewValue = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // FTEST_INFO_ID
        strNewValue += QString::number(nInfoId);
        strNewValue += ",";
        // TNUM
        strNewValue += QString::number(nTestNum);
        strNewValue += ",";
        // TNAME
        strNewValue += TranslateStringToSqlVarChar(strTestName);
        strNewValue += ",";
        // TESTSEQ
        strNewValue += pTest->m_nTestSeq == -1 ? "null" : QString::number(pTest->m_nTestSeq);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader, strNewValue, strAllValues))
            return false;

        pTest->m_nInfoId = nInfoId;
        nInfoId++;

        if(!UpdateFTestStatsSamplesTable(pTest))
            return false;
        if(!UpdateFTestStatsSummaryTable(pTest))
            return false;

    }
    if(!strAllValues.isEmpty())
    {
        // execute query
        if(!ExecuteMultiInsertQuery(strTableName, strAllValues))
            return false;
    }
    if(!mQuerySamplesFTest.isEmpty())
    {
        strTableName = NormalizeTableName("_FTEST_STATS_SAMPLES");

        // execute query
        if(!ExecuteMultiInsertQuery(strTableName, mQuerySamplesFTest))
            return false;
        mQuerySamplesFTest = "";
    }
    if(!mQuerySummaryFTest.isEmpty())
    {
        strTableName = NormalizeTableName("_FTEST_STATS_SUMMARY");

        // execute query
        if(!ExecuteMultiInsertQuery(strTableName, mQuerySummaryFTest))
            return false;
        mQuerySummaryFTest = "";
    }

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdateFTestInfoTable completed");

    // Unlock table
    UnlockTables();

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateStatsSummaryPTestTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdatePTestStatsSummaryTable(structTestInfo *pTest)
{
    QString strTableName;

    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    strTableName = NormalizeTableName("_PTEST_STATS_SUMMARY");

    QString strQueryHeader;          // INSERT INTO ...
    QString strNewValue;             // NEW VALUE FOR INSERTION
    QString strAllValues;            // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName
            + "(SPLITLOT_ID, PTEST_INFO_ID, SITE_NO, EXEC_COUNT, FAIL_COUNT, MIN_VALUE, MAX_VALUE, SUM, SQUARE_SUM, TTIME) VALUES";
    strNewValue = strAllValues = "";

    QList<int>::Iterator itSite;
    for ( itSite = pTest->mTsrSites.begin(); itSite != pTest->mTsrSites.end(); ++itSite )
    {
        // insert new entry
        strNewValue = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // PTEST_INFO_ID
        strNewValue += QString::number(pTest->m_nInfoId);
        // SITE_NO
        strNewValue += ",";
        strNewValue += QString::number(*itSite);
        strNewValue += ",";
        // EXEC_COUNT
        strNewValue += !pTest->mTsrExecCount.contains(*itSite)        ? "null" : QString::number(pTest->mTsrExecCount[*itSite]);
        strNewValue += ",";
        // FAIL_COUNT
        strNewValue += !pTest->mTsrFailCount.contains(*itSite)        ? "null" : QString::number(pTest->mTsrFailCount[*itSite]);
        strNewValue += ",";
        // MIN_VALUE
        strNewValue += !pTest->mTsrMinValue.contains(*itSite)        ? "null" : NormalizeNumber(pTest->mTsrMinValue[*itSite]);
        strNewValue += ",";
        // MAX_VALUE
        strNewValue += !pTest->mTsrMaxValue.contains(*itSite)        ? "null" : NormalizeNumber(pTest->mTsrMaxValue[*itSite]);
        strNewValue += ",";
        // SUM
        strNewValue += !pTest->mTsrSum.contains(*itSite)                ? "null" : NormalizeNumber(pTest->mTsrSum[*itSite]);
        strNewValue += ",";
        // SQUARE_SUM
        strNewValue += !pTest->mTsrSquareSum.contains(*itSite)? "null" : NormalizeNumber(pTest->mTsrSquareSum[*itSite]);
        strNewValue += ",";
        // TTIME
        strNewValue += !pTest->mTsrTTime.contains(*itSite)? "null" : NormalizeNumber(pTest->mTsrTTime[*itSite]);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader,strNewValue,mQuerySummaryPTest))
        {
            WriteDebugMessageFile(
                        QString("GexDbPlugin_Galaxy::UpdatePTestStatsSummaryTable: ERROR :\nTable=%1\nQueryHeader=%2\nNewValue=%3\nSamples=%4 ")
                        .arg(strTableName,strQueryHeader,strNewValue,mQuerySummaryPTest));
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateStatsSamplesPTestTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdatePTestStatsSamplesTable(structTestInfo *pTest)
{
    QString strTableName;

    if(m_eTestingStage == eElectTest)
        strTableName = NormalizeTableName("_PTEST_STATS");
    else
        strTableName = NormalizeTableName("_PTEST_STATS_SAMPLES");

    QString strQueryHeader;                // INSERT INTO ...
    QString strNewValue;                // NEW VALUE FOR INSERTION
    QString strAllValues;                // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName + "(SPLITLOT_ID, PTEST_INFO_ID,";
    if(m_eTestingStage != eElectTest)
        strQueryHeader+= " SITE_NO,";
    strQueryHeader+= " EXEC_COUNT, FAIL_COUNT, MIN_VALUE, MAX_VALUE, SUM, SQUARE_SUM) VALUES";
    strNewValue = strAllValues = "";

    QMap<int, unsigned int>::Iterator itSite;
    for ( itSite = pTest->m_mapExecCount.begin(); itSite != pTest->m_mapExecCount.end(); ++itSite )
    {
        if((m_eTestingStage != eElectTest) && (itSite.key() == MERGE_SITE))
            continue;
        if((m_eTestingStage == eElectTest) && (itSite.key() != MERGE_SITE))
            continue;

        // If no execution for this site (PTR for information only)
        if(pTest->m_mapExecCount[itSite.key()] == 0)
            continue;

        // insert new entry
        strNewValue = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // PTEST_INFO_ID
        strNewValue += QString::number(pTest->m_nInfoId);
        // SITE_NO
        if(m_eTestingStage != eElectTest)
        {
            strNewValue += ",";
            strNewValue += QString::number(itSite.key());
        }
        strNewValue += ",";
        // EXEC_COUNT
        strNewValue += QString::number(pTest->m_mapExecCount[itSite.key()]);
        strNewValue += ",";
        // FAIL_COUNT
        strNewValue += !pTest->m_mapFailCount.contains(itSite.key())        ? "null" : QString::number(pTest->m_mapFailCount[itSite.key()]);
        strNewValue += ",";
        // MIN_VALUE
        strNewValue += !pTest->m_mapMinValue.contains(itSite.key())        ? "null" : NormalizeNumber(pTest->m_mapMinValue[itSite.key()]);
        strNewValue += ",";
        // MAX_VALUE
        strNewValue += !pTest->m_mapMaxValue.contains(itSite.key())        ? "null" : NormalizeNumber(pTest->m_mapMaxValue[itSite.key()]);
        strNewValue += ",";
        // SUM
        strNewValue += !pTest->m_mapSum.contains(itSite.key())                ? "null" : NormalizeNumber(pTest->m_mapSum[itSite.key()]);
        strNewValue += ",";
        // SQUARE_SUM
        strNewValue += !pTest->m_mapSquareSum.contains(itSite.key())? "null" : NormalizeNumber(pTest->m_mapSquareSum[itSite.key()]);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader,strNewValue,mQuerySamplesPTest))
        {
            WriteDebugMessageFile(
                        QString("GexDbPlugin_Galaxy::UpdatePTestStatsSamplesTable: ERROR :\nTable=%1\nQueryHeader=%2\nNewValue=%3\nSamples=%4 ")
                        .arg(strTableName,strQueryHeader,strNewValue,mQuerySamplesPTest));
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateStatsSummaryMPTestTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateMPTestStatsSummaryTable(structTestInfo *pTest)
{
    QString strTableName;

    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    strTableName = NormalizeTableName("_MPTEST_STATS_SUMMARY");

    QString strQueryHeader;          // INSERT INTO ...
    QString strNewValue;             // NEW VALUE FOR INSERTION
    QString strAllValues;            // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName
            + "(SPLITLOT_ID, MPTEST_INFO_ID, SITE_NO, EXEC_COUNT, FAIL_COUNT, MIN_VALUE, MAX_VALUE, SUM, SQUARE_SUM, TTIME) VALUES";
    strNewValue = strAllValues = "";

    QList<int>::Iterator itSite;
    for ( itSite = pTest->mTsrSites.begin(); itSite != pTest->mTsrSites.end(); ++itSite )
    {
        // insert new entry
        strNewValue = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // MPTEST_INFO_ID
        strNewValue += QString::number(pTest->m_nInfoId);
        // SITE_NO
        strNewValue += ",";
        strNewValue += QString::number(*itSite);
        strNewValue += ",";
        // EXEC_COUNT
        strNewValue += !pTest->mTsrExecCount.contains(*itSite)        ? "0" : QString::number(pTest->mTsrExecCount[*itSite]);
        strNewValue += ",";
        // FAIL_COUNT
        strNewValue += !pTest->mTsrFailCount.contains(*itSite)        ? "0" : QString::number(pTest->mTsrFailCount[*itSite]);
        strNewValue += ",";
        // MIN_VALUE
        strNewValue += !pTest->mTsrMinValue.contains(*itSite)        ? "null" : NormalizeNumber(pTest->mTsrMinValue[*itSite]);
        strNewValue += ",";
        // MAX_VALUE
        strNewValue += !pTest->mTsrMaxValue.contains(*itSite)        ? "null" : NormalizeNumber(pTest->mTsrMaxValue[*itSite]);
        strNewValue += ",";
        // SUM
        strNewValue += !pTest->mTsrSum.contains(*itSite)                ? "null" : NormalizeNumber(pTest->mTsrSum[*itSite]);
        strNewValue += ",";
        // SQUARE_SUM
        strNewValue += !pTest->mTsrSquareSum.contains(*itSite)? "null" : NormalizeNumber(pTest->mTsrSquareSum[*itSite]);
        strNewValue += ",";
        // TTIME
        strNewValue += !pTest->mTsrTTime.contains(*itSite)? "null" : NormalizeNumber(pTest->mTsrTTime[*itSite]);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader,strNewValue,mQuerySummaryMPTest))
        {
            WriteDebugMessageFile(
                        QString("GexDbPlugin_Galaxy::UpdateMPTestStatsSummaryTable: ERROR :\nTable=%1\nQueryHeader=%2\nNewValue=%3\nSamples=%4 ")
                        .arg(strTableName,strQueryHeader,strNewValue,mQuerySummaryMPTest));
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateStatsSamplesMPTestTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateMPTestStatsSamplesTable(structTestInfo *pTest)
{
    QString strTableName;

    if(m_eTestingStage == eElectTest)
        return true;

    strTableName = NormalizeTableName("_MPTEST_STATS_SAMPLES");
    QString strQueryHeader;                // INSERT INTO ...
    QString strNewValue;                // NEW VALUE FOR INSERTION
    QString strAllValues;                // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName
            + "(SPLITLOT_ID, MPTEST_INFO_ID, SITE_NO, EXEC_COUNT, FAIL_COUNT, MIN_VALUE, MAX_VALUE, SUM, SQUARE_SUM) VALUES";
    strNewValue = strAllValues = "";
    QString strQuery;
    QMap<int,unsigned int>::Iterator itSite;
    for ( itSite = pTest->m_mapExecCount.begin(); itSite != pTest->m_mapExecCount.end(); ++itSite )
    {
        if(itSite.key() == MERGE_SITE)
            continue;

        // If no execution for this site (MPR for information only)
        if(pTest->m_mapExecCount[itSite.key()] == 0)
            continue;

        // insert new entry
        strNewValue  = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // MPTEST_INFO_ID
        strNewValue += QString::number(pTest->m_nInfoId);
        strNewValue += ",";
        // SITE_NO
        strNewValue += QString::number(itSite.key());
        strNewValue += ",";
        // EXEC_COUNT
        strNewValue += QString::number(pTest->m_mapExecCount[itSite.key()]);
        strNewValue += ",";
        // FAIL_COUNT
        strNewValue += !pTest->m_mapFailCount.contains(itSite.key())? "null" : QString::number(pTest->m_mapFailCount[itSite.key()]);
        strNewValue += ",";
        // MIN_VALUE
        strNewValue += !pTest->m_mapMinValue.contains(itSite.key())        ? "null" : NormalizeNumber(pTest->m_mapMinValue[itSite.key()]);
        strNewValue += ",";
        // MAX_VALUE
        strNewValue += !pTest->m_mapMaxValue.contains(itSite.key())        ? "null" : NormalizeNumber(pTest->m_mapMaxValue[itSite.key()]);
        strNewValue += ",";
        // SUM
        strNewValue += !pTest->m_mapSum.contains(itSite.key())                ? "null" : NormalizeNumber(pTest->m_mapSum[itSite.key()]);
        strNewValue += ",";
        // SQUARE_SUM
        strNewValue += !pTest->m_mapSquareSum.contains(itSite.key())? "null" : NormalizeNumber(pTest->m_mapSquareSum[itSite.key()]);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader,strNewValue,mQuerySamplesMPTest))
            return false;
    }
    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateStatsSummaryFTestTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateFTestStatsSummaryTable(structTestInfo *pTest)
{
    QString strTableName;

    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_TSR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    strTableName = NormalizeTableName("_FTEST_STATS_SUMMARY");
    QString strQueryHeader;          // INSERT INTO ...
    QString strNewValue;             // NEW VALUE FOR INSERTION
    QString strAllValues;            // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName
            + "(SPLITLOT_ID, FTEST_INFO_ID, SITE_NO, EXEC_COUNT, FAIL_COUNT, TTIME) VALUES";
    strNewValue = strAllValues = "";

    QList<int>::Iterator itSite;
    for ( itSite = pTest->mTsrSites.begin(); itSite != pTest->mTsrSites.end(); ++itSite )
    {
        // insert new entry
        strNewValue = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // PTEST_INFO_ID
        strNewValue += QString::number(pTest->m_nInfoId);
        // SITE_NO
        strNewValue += ",";
        strNewValue += QString::number(*itSite);
        strNewValue += ",";
        // EXEC_COUNT
        strNewValue += !pTest->mTsrExecCount.contains(*itSite)        ? "null" : QString::number(pTest->mTsrExecCount[*itSite]);
        strNewValue += ",";
        // FAIL_COUNT
        strNewValue += !pTest->mTsrFailCount.contains(*itSite)        ? "null" : QString::number(pTest->mTsrFailCount[*itSite]);
        strNewValue += ",";
        // TTIME
        strNewValue += !pTest->mTsrTTime.contains(*itSite)? "null" : NormalizeNumber(pTest->mTsrTTime[*itSite]);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader,strNewValue,mQuerySummaryFTest))
        {
            WriteDebugMessageFile(
                        QString("GexDbPlugin_Galaxy::UpdateFTestStatsSummaryTable: ERROR :\nTable=%1\nQueryHeader=%2\nNewValue=%3\nSamples=%4 ")
                        .arg(strTableName,strQueryHeader,strNewValue,mQuerySummaryFTest));
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateStatsSamplesFTestTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateFTestStatsSamplesTable(structTestInfo *pTest)
{
    QString strTableName;

    if(m_eTestingStage == eElectTest)
        return true;

    strTableName = NormalizeTableName("_FTEST_STATS_SAMPLES");

    QString strQueryHeader;                // INSERT INTO ...
    QString strNewValue;                // NEW VALUE FOR INSERTION
    QString strAllValues;                // MULTI QUERIES

    strQueryHeader = "INSERT INTO " + strTableName
            + "(SPLITLOT_ID, FTEST_INFO_ID, SITE_NO, EXEC_COUNT, FAIL_COUNT) VALUES";
    strNewValue = strAllValues = "";

    QMap<int,unsigned int>::Iterator itSite;
    for ( itSite = pTest->m_mapExecCount.begin(); itSite != pTest->m_mapExecCount.end(); ++itSite )
    {
        if(itSite.key() == MERGE_SITE)
            continue;

        // If no execution for this site (FTR for information only)
        if(pTest->m_mapExecCount[itSite.key()] == 0)
            continue;

        // insert new entry
        strNewValue  = "(";
        // SPLITLOT_ID
        strNewValue += QString::number(m_nSplitLotId);
        strNewValue += ",";
        // FTEST_INFO_ID
        strNewValue += QString::number(pTest->m_nInfoId);
        strNewValue += ",";
        // SITE_NO
        strNewValue += QString::number(itSite.key());
        strNewValue += ",";
        // EXEC_COUNT
        strNewValue += QString::number(pTest->m_mapExecCount[itSite.key()]);
        strNewValue += ",";
        // FAIL_COUNT
        strNewValue += !pTest->m_mapFailCount.contains(itSite.key()) ? "null" : QString::number(pTest->m_mapFailCount[itSite.key()]);
        strNewValue += ")";

        if(!AddInMultiInsertQuery(strTableName, strQueryHeader,strNewValue,mQuerySamplesFTest))
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateResultPTestTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdatePTestResultsTable()
{
    QString strTableName;
    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PTR]==0)
        return true;

    if(m_bIgnoreFirstRun && (m_nNbRunProcessed == 0))
        return true;

    strTableName = NormalizeTableName("_PTEST_RESULTS");

    // extract the site number of the ptr
    unsigned short siteNumber =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            ( m_clStdfParse, m_clStdfPTR );

    // Find test in testlist
    // case 6488 - 6489
    // Allow Duplicated test name => find TestInfo on Type and TestNumber and TestName
    // Reject Duplicated test name => find TestInfo only on Type and TestNumber
    // Autoincrement test number => find TestInfo only on Type and TestName
    qint64  TestNumber = m_clStdfPTR.m_u4TEST_NUM;
    int     TestType = GQTL_STDF::Stdf_Record::Rec_PTR;
    QString TestName   = m_clStdfPTR.m_cnTEST_TXT;
    // Autoincrement test number => may be only the first record have the test name stored
    // In this case, we cannot use the test number to find the TestInfo
    // we must retrieve the original test name used for this test number
    // test number = 0 and test name empry is not allowed
    if(TestName.isEmpty() && mAssociatedTestNames.contains(QString("[%1][%2]").arg(TestType).arg(m_clStdfPTR.m_u4TEST_NUM)))
        TestName = mAssociatedTestNames[QString("[%1][%2]").arg(TestType).arg(m_clStdfPTR.m_u4TEST_NUM)];

    if(mMergeByTestName)
        TestNumber = GetAutoIncrementTestNumber(TestType, TestName);
    else if(mMergeByTestNumber)
        TestName = "";
    else if(!mAllowedDuplicateTestNumbers)
        TestName = "";
    /* GCORE-506: Duplicated Test# issue with Flex format
       Generate issues when ALLOW_DUPLICATE_TEST_NUMBERS is activated, this is not needed any more
       as in the FindTestInfo method, we automatically remove pin name from the test name for Flex format
    // For FLEX only
    // Find the test only with the TestNum
    else if(m_bFlexFormat)
        TestName = "";
    */

    structTestInfo *pTest = FindTestInfo(TestType, TestNumber, TestName);
    if(pTest == NULL)
    {
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eInternal_MissingTest, NULL,
                    m_clStdfPTR.m_u4TEST_NUM, m_clStdfPTR.m_cnTEST_TXT.toLatin1().constData());
        return false;
    }

    // verify if test is really a PTR
    if(pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_PTR)
    {
        GSET_ERROR2(GexDbPlugin_Base, eValidation_DuplicateTest, NULL,
                    pTest->m_nOriginalTestNum, pTest->m_strTestName.toLatin1().constData());
        if(m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTTYPEREDEFINITION)
            return false;
        QString strMessage = "[Record#PTR] ";
        strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base,this);
        WarningMessage(strMessage);
        // Ignore this test
        return true;
    }

    if(!m_mapTestsExecuted.contains(siteNumber))
    {
        QString lBuffer;
        lBuffer = "Invalid Site number - ";
        lBuffer+= " SiteNo["+QString::number(siteNumber)+"] - ";
        lBuffer+= " near PartId["+m_clStdfPRR.m_cnPART_ID+"] - ";
        lBuffer+= " TestNumber["+QString::number(m_clStdfPTR.m_u4TEST_NUM)+"] - ";
        lBuffer+= " Cannot associate this PTR result with a valid PIR/PRR";
        GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidField, NULL,lBuffer.toLatin1().data());
        GSLOG(SYSLOG_SEV_WARNING, lBuffer.toLatin1().data());
        return false;
    }

    // Increment execution counter
    m_mapTestsExecuted[siteNumber]++;

    // Test result valid
    char cFlags= 0;        // Check if Result is valid and if Pass or Fail

    // Check if test is executed
    if(IsTestNotExecuted(m_clStdfPTR))
        cFlags |= FLAG_TESTRESULT_NOTEXECUTED;

    // Check if result flag valid
    if(IsTestResultInvalid(m_clStdfPTR))
        cFlags |= FLAG_TESTRESULT_INVALID_RESULT;

    // Check if Pass/Fail flag valid
    if(IsTestFail(m_clStdfPTR, pTest))
    {
        m_mapTestsFailed[siteNumber]++;
        if(m_mapTestsFailed[siteNumber] == 1)
        {
            m_mapFirstFailTType[siteNumber] = "P";
            m_mapFirstFailTId[siteNumber] = pTest->m_nInfoId;
        }
        m_mapLastFailTType[siteNumber] = "P";
        m_mapLastFailTId[siteNumber] = pTest->m_nInfoId;
    }

    // bit 0:        1 =        Alarm detected during testing
    if(m_clStdfPTR.m_b1TEST_FLG & STDF_MASK_BIT0)
        cFlags |= FLAG_TESTRESULT_ALARM;

    // bit 6:        1 =        no pass/fail indication
    if(m_clStdfPTR.m_b1TEST_FLG & STDF_MASK_BIT6)
        cFlags |= FLAG_TESTRESULT_INVALID_PASSFLAG;

    // bit 7:        0 =        Test passed
    if((m_clStdfPTR.m_b1TEST_FLG & STDF_MASK_BIT7) == 0)
        cFlags |= FLAG_TESTRESULT_PASS;

    if(!InitLoadDataInfile(GQTL_STDF::Stdf_Record::Rec_PTR))
        return false;

    m_strPTestResults += m_strLoadDataInfileBeginValues;
    // SPLITLOT_ID
    m_strPTestResults += QString::number(m_nSplitLotId);
    m_strPTestResults += m_strLoadDataInfileSepValues;
    // PTEST_INFO_ID
    m_strPTestResults += QString::number(pTest->m_nInfoId);
    m_strPTestResults += m_strLoadDataInfileSepValues;
    // RUN_ID
    m_strPTestResults += QString::number(m_mapRunId[siteNumber]);
    m_strPTestResults += m_strLoadDataInfileSepValues;
    // TESTSEQ
    if(m_strPTestResultsTableColumns.contains("TESTSEQ",Qt::CaseInsensitive))
    {
        m_strPTestResults += QString::number(m_mapTestsExecuted[siteNumber]);
        m_strPTestResults += m_strLoadDataInfileSepValues;
    }
    // Result_FLAGS
    m_strPTestResults += QString::number(cFlags);
    m_strPTestResults += m_strLoadDataInfileSepValues;
    // VALUE
    QString strValue = NormalizeNumber(m_clStdfPTR.m_r4RESULT,m_strLoadDataInfileDecSepValues);
    if((cFlags & FLAG_TESTRESULT_INVALID_RESULT) || (strValue == "null"))
        strValue = m_strLoadDataInfileNullValues;
    m_strPTestResults += strValue;
    m_strPTestResults += m_strLoadDataInfileEndValues+"\n";
    m_nNbPTestResults++;

    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateResultMPTestTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateMPTestResultsTable()
{
    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_MPR]==0)
        return true;
    QString strTableName;

    if(m_eTestingStage == eElectTest)
        return true;

    if(m_bIgnoreFirstRun && (m_nNbRunProcessed == 0))
        return true;

    strTableName = NormalizeTableName("_MPTEST_RESULTS");

    structTestInfo *pTest=NULL;
    int i, nTestPin, nTestPinIndex;
    char cFlags;
    // Check if Result is valid and if Pass or Fail
    // m_u2RTN_ICNT=0        && m_u2RSLT_CNT=0 : TestPinArrayIndex=-1 && TestPinPMRIndex=-1 && NoResultFlag
    // m_u2RTN_ICNT=0        && m_u2RSLT_CNT>=1 : TestPinArrayIndex=i && TestPinPMRIndex=-1 or PMRIndex saved && Save all Results
    // m_u2RTN_ICNT>=1  && <m_u2RSLT_CNT : Warning and TestPinArrayIndex=i && TestPinPMRIndex=index[i] && Save m_u2RTN_ICNT Results
    // m_u2RTN_ICNT>=1  && >m_u2RSLT_CNT : Warning and TestPinArrayIndex=i && TestPinPMRIndex=index[i] && Save m_u2RSLT_CNT Results
    // m_u2RTN_ICNT>=1  && =m_u2RSLT_CNT  : OK
    // m_u2RTN_ICNT>=1  && m_u2RSLT_CNT=0 : Save m_u2RTN_ICNT Results && NoResultFlag
    int nMPTestCnt   = qMin(m_clStdfMPR.m_u2RTN_ICNT,m_clStdfMPR.m_u2RSLT_CNT);   // The number of MPTest to save
    nTestPinIndex    = 0;
    cFlags = 0;
    if(m_clStdfMPR.m_u2RTN_ICNT == 0)
    {
        if(m_clStdfMPR.m_u2RSLT_CNT == 0)
        {
            // For result table
            nMPTestCnt = 1;
            nTestPinIndex        = -1;
            cFlags |= FLAG_TESTRESULT_INVALID_RESULT;
        }
        else
            nMPTestCnt = m_clStdfMPR.m_u2RSLT_CNT;
    } // 7699
    else if(m_clStdfMPR.m_u2RSLT_CNT == 0)
    {
        nMPTestCnt = m_clStdfMPR.m_u2RTN_ICNT;
        cFlags |= FLAG_TESTRESULT_INVALID_RESULT;
    }

    float fResult;

    // get the site number in the mpr
    unsigned short siteNumber =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            ( m_clStdfParse, m_clStdfMPR );

    // Find test in testlist

    // SAVED THE TPIN_INDEX 0 INSTEAD OF TPIN_NUMBER IN m_clStdfMPR.m_ku2RTN_INDX[0]
    // Check if the Parameter already saved
    // case 6488 - 6489
    // Allow Duplicated test name => find TestInfo on Type and TestNumber and TestName
    // Reject Duplicated test name => find TestInfo only on Type and TestNumber
    // Autoincrement test number => find TestInfo only on Type and TestName
    qint64  TestNumber = m_clStdfMPR.m_u4TEST_NUM;
    int     TestType = GQTL_STDF::Stdf_Record::Rec_MPR;
    QString TestName   = m_clStdfMPR.m_cnTEST_TXT;
    // Autoincrement test number => may be only the first record have the test name stored
    // In this case, we cannot use the test number to find the TestInfo
    // we must retrieve the original test name used for this test number
    // test number = 0 and test name empry is not allowed
    if(TestName.isEmpty() && mAssociatedTestNames.contains(QString("[%1][%2]").arg(TestType).arg(m_clStdfMPR.m_u4TEST_NUM)))
        TestName = mAssociatedTestNames[QString("[%1][%2]").arg(TestType).arg(m_clStdfMPR.m_u4TEST_NUM)];

    if(mMergeByTestName)
        TestNumber = GetAutoIncrementTestNumber(TestType, TestName);
    else if(mMergeByTestNumber)
        TestName = "";
    else if(!mAllowedDuplicateTestNumbers)
        TestName = "";

    pTest = FindTestInfo(TestType, TestNumber, TestName, nTestPinIndex);
    // If pTest == NULL Then error
    if(pTest == NULL)
    {
        GSLOG(3, QString("Cant find test %1 '%2' pin %3").arg(TestNumber).arg(TestName).arg(nTestPinIndex)
              .toLatin1().data() );
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eInternal_MissingTest, NULL,
                    m_clStdfMPR.m_u4TEST_NUM, m_clStdfMPR.m_cnTEST_TXT.toLatin1().constData());
        return false;
    }

    if(!m_mapTestsExecuted.contains(siteNumber))
    {
        // Delay the file
        QString lBuffer = "Invalid Site number - ";
        lBuffer+= " SiteNo["+QString::number(siteNumber)+"] - ";
        lBuffer+= " near PartId["+m_clStdfPRR.m_cnPART_ID+"] - ";
        lBuffer+= " TestNumber["+QString::number(m_clStdfMPR.m_u4TEST_NUM)+"] - ";
        lBuffer+= " Cannot associate this MPR result with a valid PIR/PRR";
        GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidField, NULL,lBuffer.toLatin1().data());
        GSLOG(SYSLOG_SEV_WARNING, lBuffer.toLatin1().data());
        return false;
    }

    // Increment execution counter
    m_mapTestsExecuted[siteNumber]++;

    // Check if test is executed
    if(IsTestNotExecuted(m_clStdfMPR))
        cFlags |= FLAG_TESTRESULT_NOTEXECUTED;

    // Check if result flag valid
    if(IsTestResultInvalid(m_clStdfMPR))
        cFlags |= FLAG_TESTRESULT_INVALID_RESULT;

    // Test result valid
    if(IsTestFail(m_clStdfMPR, pTest))
    {
        m_mapTestsFailed[siteNumber]++;
        if(m_mapTestsFailed[siteNumber] == 1)
        {
            m_mapFirstFailTType[siteNumber] = "M";
            m_mapFirstFailTId[siteNumber] = pTest->m_nInfoId;
        }
        m_mapLastFailTType[siteNumber] = "M";
        m_mapLastFailTId[siteNumber] = pTest->m_nInfoId;
    }
    // bit 0:        1 =        Alarm detected during testing
    if(m_clStdfMPR.m_b1TEST_FLG & STDF_MASK_BIT0)
        cFlags |= FLAG_TESTRESULT_ALARM;

    // bit 6:        1 =        no pass/fail indication
    if(m_clStdfMPR.m_b1TEST_FLG & STDF_MASK_BIT6)
        cFlags |= FLAG_TESTRESULT_INVALID_PASSFLAG;

    // bit 7:        0 =        Test passed
    if((m_clStdfMPR.m_b1TEST_FLG & STDF_MASK_BIT7) == 0)
        cFlags |= FLAG_TESTRESULT_PASS;

    for(i=0; i<nMPTestCnt; i++)
    {
        if(m_clStdfMPR.m_u2RSLT_CNT > 0)
        {
            nTestPinIndex   = i;
            fResult         = m_clStdfMPR.m_kr4RTN_RSLT[i];
        }
        else
            fResult         = 0;

        // Find test with specific pin in testlist
        pTest = FindTestInfo(GQTL_STDF::Stdf_Record::Rec_MPR, TestNumber, TestName, nTestPinIndex);

        if(pTest == NULL)
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR3(GexDbPlugin_Base, eInternal_MissingTestPin, NULL, m_clStdfMPR.m_u4TEST_NUM,
                        m_clStdfMPR.m_cnTEST_TXT.toLatin1().constData(), nTestPinIndex);
            return false;
        }

        // verify if test is really a MPR
        if(pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_MPR)
        {
            GSET_ERROR2(GexDbPlugin_Base, eValidation_DuplicateTest, NULL,
                        pTest->m_nOriginalTestNum, pTest->m_strTestName.toLatin1().constData());
            if(m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTTYPEREDEFINITION)
                return false;
            QString strMessage = "[Record#MPR] ";
            strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base,this);
            WarningMessage(strMessage);
            // Ignore this test
            return true;
        }

        // Update the TPIN info directly from m_ku2RTN_INDX if any
        if((m_clStdfMPR.m_u2RTN_ICNT>0) && m_clStdfMPR.m_ku2RTN_INDX)
            nTestPin        = m_clStdfMPR.m_ku2RTN_INDX[i];
        else
            nTestPin        = pTest->m_nTestPin;  // first m_ku2RTN_INDX[i] saved or -1 if never


        if(!InitLoadDataInfile(GQTL_STDF::Stdf_Record::Rec_MPR))
            return false;

        QString strValue = NormalizeNumber(fResult,m_strLoadDataInfileDecSepValues);
        char cPinFlags = cFlags;
        if((cPinFlags & FLAG_TESTRESULT_INVALID_RESULT) || (strValue == "null"))
        {
            strValue = m_strLoadDataInfileNullValues;
            cPinFlags |= FLAG_TESTRESULT_INVALID_RESULT;

            // value NULL is not allowed
            // use the invalid test_Flags and 0 for the value
            strValue = NormalizeNumber(0.0,m_strLoadDataInfileDecSepValues);
        }

        // insert new entry
        m_strMPTestResults += m_strLoadDataInfileBeginValues;
        // SPLITLOT_ID
        m_strMPTestResults += QString::number(m_nSplitLotId);
        m_strMPTestResults += m_strLoadDataInfileSepValues;
        // MPTEST_INFO_ID
        m_strMPTestResults += QString::number(pTest->m_nInfoId);
        m_strMPTestResults += m_strLoadDataInfileSepValues;
        // RUN_ID
        m_strMPTestResults += QString::number(m_mapRunId[siteNumber]);
        m_strMPTestResults += m_strLoadDataInfileSepValues;
        // TESTSEQ
        if(m_strMPTestResultsTableColumns.contains("TESTSEQ",Qt::CaseInsensitive))
        {
            m_strMPTestResults += QString::number(m_mapTestsExecuted[siteNumber]);
            m_strMPTestResults += m_strLoadDataInfileSepValues;
        }
        // RESULT_FLAGS
        m_strMPTestResults += QString::number(cPinFlags);
        m_strMPTestResults += m_strLoadDataInfileSepValues;
        // VALUE
        m_strMPTestResults += strValue;
        m_strMPTestResults += m_strLoadDataInfileSepValues;
        // TPIN_MPRINDEX
        m_strMPTestResults += QString::number(nTestPin);
        m_strMPTestResults += m_strLoadDataInfileEndValues+"\n";
        m_nNbMPTestResults++;
    }
    return true;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateResultFTestTable
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateFTestResultsTable()
{
    if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_FTR]==0)
        return true;

    if(m_eTestingStage == eElectTest)
        return true;

    if(m_bIgnoreFirstRun && (m_nNbRunProcessed == 0))
        return true;

    QString strTableName;

    strTableName = NormalizeTableName("_FTEST_RESULTS");

    // get the site number inside the ftr
    unsigned short siteNumber =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            ( m_clStdfParse, m_clStdfFTR );

    // Find test in testlist
    // Find test in testlist
    // case 6488 - 6489
    // Allow Duplicated test name => find TestInfo on Type and TestNumber and TestName
    // Reject Duplicated test name => find TestInfo only on Type and TestNumber
    // Autoincrement test number => find TestInfo only on Type and TestName
    qint64  TestNumber = m_clStdfFTR.m_u4TEST_NUM;
    int     TestType = GQTL_STDF::Stdf_Record::Rec_FTR;
    QString TestName   = m_clStdfFTR.m_cnTEST_TXT;
    // Autoincrement test number => may be only the first record have the test name stored
    // In this case, we cannot use the test number to find the TestInfo
    // we must retrieve the original test name used for this test number
    // test number = 0 and test name empry is not allowed
    if(TestName.isEmpty() && mAssociatedTestNames.contains(QString("[%1][%2]").arg(TestType).arg(m_clStdfFTR.m_u4TEST_NUM)))
        TestName = mAssociatedTestNames[QString("[%1][%2]").arg(TestType).arg(m_clStdfFTR.m_u4TEST_NUM)];

    if(mMergeByTestName)
        TestNumber = GetAutoIncrementTestNumber(TestType, TestName);
    else if(mMergeByTestNumber)
        TestName = "";
    else if(!mAllowedDuplicateTestNumbers)
        TestName = "";

    structTestInfo *pTest = FindTestInfo(TestType, TestNumber, TestName);
    if(pTest == NULL)
    {
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eInternal_MissingTest, NULL,
                    m_clStdfFTR.m_u4TEST_NUM, m_clStdfFTR.m_cnTEST_TXT.toLatin1().constData());
        return false;
    }

    // verify if test is really a FTR
    if(pTest->m_nTestType != GQTL_STDF::Stdf_Record::Rec_FTR)
    {
        GSET_ERROR2(GexDbPlugin_Base, eValidation_DuplicateTest, NULL,
                    pTest->m_nOriginalTestNum, pTest->m_strTestName.toLatin1().constData());
        if(m_uiInsertionValidationFailOnFlag & GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_TESTTYPEREDEFINITION)
            return false;
        QString strMessage = "[Record#FTR] ";
        strMessage += GGET_LASTERRORMSG(GexDbPlugin_Base,this);
        WarningMessage(strMessage);
        // Ignore this test
        return true;
    }

    if(!m_mapTestsExecuted.contains(siteNumber))
    {
        // Delay the file
        QString lBuffer = "Invalid Site number - ";
        lBuffer+= " SiteNo["+QString::number(siteNumber)+"] - ";
        lBuffer+= " near PartId["+m_clStdfPRR.m_cnPART_ID+"] - ";
        lBuffer+= " TestNumber["+QString::number(m_clStdfFTR.m_u4TEST_NUM)+"] - ";
        lBuffer+= " Cannot associate this FTR result with a valid PIR/PRR";
        GSET_ERROR1(GexDbPlugin_Base, eValidation_InvalidField, NULL,lBuffer.toLatin1().data());
        GSLOG(SYSLOG_SEV_WARNING, lBuffer.toLatin1().data());
        return false;
    }

    // Increment execution counter
    m_mapTestsExecuted[siteNumber]++;

    // Test result valid
    char cFlags= 0;        // Check if Result is valid and if Pass or Fail

    // Check if test is executed
    if(IsTestNotExecuted(m_clStdfFTR))
        cFlags |= FLAG_TESTRESULT_NOTEXECUTED;

    // Check if result flag valid
    if(IsTestResultInvalid(m_clStdfFTR))
        cFlags |= FLAG_TESTRESULT_INVALID_RESULT;

    // Check if Pass/Fail flag valid
    if(IsTestFail(m_clStdfFTR, pTest))
    {
        m_mapTestsFailed[siteNumber]++;
        if(m_mapTestsFailed[siteNumber] == 1)
        {
            m_mapFirstFailTType[siteNumber] = "F";
            m_mapFirstFailTId[siteNumber] = pTest->m_nInfoId;
        }
        m_mapLastFailTType[siteNumber] = "F";
        m_mapLastFailTId[siteNumber] = pTest->m_nInfoId;
    }
    // bit 0:        1 =        Alarm detected during testing
    if(m_clStdfFTR.m_b1TEST_FLG & STDF_MASK_BIT0)
        cFlags |= FLAG_TESTRESULT_ALARM;

    // bit 6:        1 =        no pass/fail indication
    if(m_clStdfFTR.m_b1TEST_FLG & STDF_MASK_BIT6)
        cFlags |= FLAG_TESTRESULT_INVALID_PASSFLAG;

    // bit 7:        0 =        Test passed
    if((m_clStdfFTR.m_b1TEST_FLG & STDF_MASK_BIT7) == 0)
        cFlags |= FLAG_TESTRESULT_PASS;

    if(!InitLoadDataInfile(GQTL_STDF::Stdf_Record::Rec_FTR))
        return false;

    // insert new entry
    m_strFTestResults += m_strLoadDataInfileBeginValues;
    // SPLITLOT_ID
    m_strFTestResults += QString::number(m_nSplitLotId);
    m_strFTestResults += m_strLoadDataInfileSepValues;
    // FTEST_INFO_ID
    m_strFTestResults += QString::number(pTest->m_nInfoId);
    m_strFTestResults += m_strLoadDataInfileSepValues;
    // RUN_ID
    m_strFTestResults += QString::number(m_mapRunId[siteNumber]);
    m_strFTestResults += m_strLoadDataInfileSepValues;
    // TESTSEQ
    if(m_strFTestResultsTableColumns.contains("TESTSEQ",Qt::CaseInsensitive))
    {
        m_strFTestResults += QString::number(m_mapTestsExecuted[siteNumber]);
        m_strFTestResults += m_strLoadDataInfileSepValues;
    }
    // RESULT_FLAGS
    m_strFTestResults += QString::number(cFlags);
    m_strFTestResults += m_strLoadDataInfileSepValues;
    // VECT_NAM
    m_strFTestResults += TranslateStringToSqlVarChar(m_clStdfFTR.m_cnVECT_NAM);
    m_strFTestResults += m_strLoadDataInfileSepValues;
    // VECT_OFF
    m_strFTestResults += !(m_clStdfFTR.m_b1OPT_FLAG & STDF_MASK_BIT7) || (m_clStdfFTR.m_b1OPT_FLAG & STDF_MASK_BIT5) ? m_strLoadDataInfileNullValues : QString::number(m_clStdfFTR.m_i2VECT_OFF);
    m_strFTestResults += m_strLoadDataInfileEndValues+"\n";
    m_nNbFTestResults++;

    return true;
}


//////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdatePCRInfo(const GQTL_STDF::Stdf_PCR_V4 &clRecord)
{
    // Debug message
    WriteDebugMessageFile("---- GexDbPlugin_Galaxy::UpdatePCRInfo");

    int nSiteNum = INVALID_SITE;

    // Check if data for all sites
    if(!m_mapPCRNbParts.contains(nSiteNum))
        m_mapPCRNbParts[nSiteNum] = 0;
    if((clRecord.m_u4RTST_CNT != INVALID_INT) && (!m_mapPCRNbRetestParts.contains(nSiteNum)))
        m_mapPCRNbRetestParts[nSiteNum] = 0;
    if((clRecord.m_u4GOOD_CNT != INVALID_INT) && (!m_mapPCRNbPartsGood.contains(nSiteNum)))
        m_mapPCRNbPartsGood[nSiteNum] = 0;

    m_mapPCRNbParts[nSiteNum] += clRecord.m_u4PART_CNT;
    if(clRecord.m_u4RTST_CNT != INVALID_INT)
        m_mapPCRNbRetestParts[nSiteNum] += clRecord.m_u4RTST_CNT;
    if(clRecord.m_u4GOOD_CNT != INVALID_INT)
        m_mapPCRNbPartsGood[nSiteNum] += clRecord.m_u4GOOD_CNT;

    // Check if data for one site
    nSiteNum =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            ( m_clStdfParse, clRecord );

    BYTE headNumber =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
            ( m_clStdfParse, clRecord );

    if(headNumber == 255)
        nSiteNum = MERGE_SITE;

    if(!m_mapPCRNbParts.contains(nSiteNum))
        m_mapPCRNbParts[nSiteNum] = 0;
    if((clRecord.m_u4RTST_CNT != INVALID_INT) && (!m_mapPCRNbRetestParts.contains(nSiteNum)))
        m_mapPCRNbRetestParts[nSiteNum] = 0;
    if((clRecord.m_u4GOOD_CNT != INVALID_INT) && (!m_mapPCRNbPartsGood.contains(nSiteNum)))
        m_mapPCRNbPartsGood[nSiteNum] = 0;

    m_mapPCRNbParts[nSiteNum] += clRecord.m_u4PART_CNT;
    if(clRecord.m_u4RTST_CNT != INVALID_INT)
        m_mapPCRNbRetestParts[nSiteNum] += clRecord.m_u4RTST_CNT;
    if(clRecord.m_u4GOOD_CNT != INVALID_INT)
        m_mapPCRNbPartsGood[nSiteNum] += clRecord.m_u4GOOD_CNT;

    // Debug message
    WriteDebugMessageFile("     GexDbPlugin_Galaxy::UpdatePCRInfo completed");

    return true;
}


//////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateHBinInfo(const GQTL_STDF::Stdf_PRR_V4 &clRecord)
{
    // Test result valid
    bool bValid = false;
    bool bPass = true;
    // Check if Pass/Fail flag valid
    if((clRecord.m_b1PART_FLG & STDF_MASK_BIT4) == 0)
    {
        bValid = true;
        if(clRecord.m_b1PART_FLG & STDF_MASK_BIT3)
            bPass = false;
    }

    if(!m_mapHBinInfo.contains(clRecord.m_u2HARD_BIN))
    {
        m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_nBinNum = clRecord.m_u2HARD_BIN;
        m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_cBinCat = ' ';
    }

    // IF THE BINCAT NOT DEFINED (HBR CORRUPTED)
    // UPDATE IT WITH INFO FROM PRR
    // GCORE-3283
    bool lValidBinCat = (m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_cBinCat == 'P')
            || (m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_cBinCat == 'F')
            || (m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_cBinCat == 'A');

    if(!lValidBinCat)
    {
        if(bValid)
        {
            if(bPass)
                m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_cBinCat = 'P';
            else
                m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_cBinCat = 'F';
        }
        else
        {
            if(clRecord.m_u2HARD_BIN == 1)
                m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_cBinCat = 'P';
            else
                m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_cBinCat = 'F';
        }
    }

    // MERGE_SITE for merge site information
    if(!m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_mapBinNbRuns.contains(MERGE_SITE))
        m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_mapBinNbRuns[MERGE_SITE]=0;
    m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_mapBinNbRuns[MERGE_SITE]++;

    int nSiteNum =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            ( m_clStdfParse, clRecord );

    BYTE headNum =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
            ( m_clStdfParse, clRecord );

    if(headNum == 255)
        return true;

    if(!m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_mapBinNbRuns.contains(nSiteNum))
        m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_mapBinNbRuns[nSiteNum]=0;
    m_mapHBinInfo[clRecord.m_u2HARD_BIN].m_mapBinNbRuns[nSiteNum]++;

    return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateHBinInfo(const GQTL_STDF::Stdf_HBR_V4 &clRecord)
{
    if(!m_mapHBinInfo.contains(clRecord.m_u2HBIN_NUM))
    {
        m_mapHBinInfo[clRecord.m_u2HBIN_NUM].m_nBinNum = clRecord.m_u2HBIN_NUM;
        m_mapHBinInfo[clRecord.m_u2HBIN_NUM].m_cBinCat = ' ';
    }

    // IF THE BINCAT NOT DEFINED (HBR CORRUPTED)
    // UPDATE IT WITH INFO FROM PRR
    // GCORE-3283
    bool lValidBinCat = (clRecord.m_c1HBIN_PF == 'P')
            || (clRecord.m_c1HBIN_PF == 'F')
            || (clRecord.m_c1HBIN_PF == 'A');

    // update HBin statique info
    if(lValidBinCat)
        m_mapHBinInfo[clRecord.m_u2HBIN_NUM].m_cBinCat = clRecord.m_c1HBIN_PF;
    else
    {
        // Only force the BinCat if no PRR
        if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR] == 0)
        {
            if(clRecord.m_u2HBIN_NUM == 1)
                m_mapHBinInfo[clRecord.m_u2HBIN_NUM].m_cBinCat = 'P';
            else
                m_mapHBinInfo[clRecord.m_u2HBIN_NUM].m_cBinCat = 'F';
        }
    }

    QString lBinName = QString(clRecord.m_cnHBIN_NAM).trimmed();
    if(!lBinName.isEmpty())
        m_mapHBinInfo[clRecord.m_u2HBIN_NUM].m_strBinName = lBinName;


    if(mpDbKeysEngine->dbKeysContent().Get("IgnoreSummary").toBool())
    {
        m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_HBR]--;
        return true;
    }

    // Save HBR info
    int nSiteNum =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            ( m_clStdfParse, clRecord );

    BYTE headNum =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
            ( m_clStdfParse, clRecord );

    if(headNum == 255)
        nSiteNum = MERGE_SITE;

    // Update Rebuild merge sites information
    if(!m_mapHBinInfo[clRecord.m_u2HBIN_NUM].m_mapBinCnt.contains(INVALID_SITE))
        m_mapHBinInfo[clRecord.m_u2HBIN_NUM].m_mapBinCnt[INVALID_SITE]=0;
    if(nSiteNum != MERGE_SITE)
        m_mapHBinInfo[clRecord.m_u2HBIN_NUM].m_mapBinCnt[INVALID_SITE]+=clRecord.m_u4HBIN_CNT;

    m_mapHBinInfo[clRecord.m_u2HBIN_NUM].m_mapBinCnt[nSiteNum]=clRecord.m_u4HBIN_CNT;


    return true;
}

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateSBinInfo(const GQTL_STDF::Stdf_PRR_V4 &clRecord)
{
    // Test result valid
    bool bValid = false;
    bool bPass = true;
    // Check if Pass/Fail flag valid
    if((clRecord.m_b1PART_FLG & STDF_MASK_BIT4) == 0)
    {
        bValid = true;
        if(clRecord.m_b1PART_FLG & STDF_MASK_BIT3)
            bPass = false;
    }

    if(!m_mapSBinInfo.contains(clRecord.m_u2SOFT_BIN))
    {
        m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_nBinNum = clRecord.m_u2SOFT_BIN;
        m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_cBinCat = ' ';
    }

    // IF THE BINCAT NOT DEFINED (SBR CORRUPTED)
    // UPDATE IT WITH INFO FROM PRR
    // GCORE-3283
    bool lValidBinCat = (m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_cBinCat == 'P')
            || (m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_cBinCat == 'F')
            || (m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_cBinCat == 'A');

    if(!lValidBinCat)
    {
        if(bValid)
        {
            if(bPass)
                m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_cBinCat = 'P';
            else
                m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_cBinCat = 'F';
        }
        else
        {
            if(clRecord.m_u2SOFT_BIN == 1)
                m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_cBinCat = 'P';
            else
                m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_cBinCat = 'F';

        }
    }

    // MERGE_SITE for merge site information
    if(!m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_mapBinNbRuns.contains(MERGE_SITE))
        m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_mapBinNbRuns[MERGE_SITE]=0;
    m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_mapBinNbRuns[MERGE_SITE]++;

    int nSiteNum =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            ( m_clStdfParse, clRecord );

    BYTE headNum =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
            ( m_clStdfParse, clRecord );

    if(headNum == 255)
        return true;

    if(!m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_mapBinNbRuns.contains(nSiteNum))
        m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_mapBinNbRuns[nSiteNum]=0;
    m_mapSBinInfo[clRecord.m_u2SOFT_BIN].m_mapBinNbRuns[nSiteNum]++;

    return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateSBinInfo(const GQTL_STDF::Stdf_SBR_V4 &clRecord)
{

    if(!m_mapSBinInfo.contains(clRecord.m_u2SBIN_NUM))
    {
        m_mapSBinInfo[clRecord.m_u2SBIN_NUM].m_nBinNum = clRecord.m_u2SBIN_NUM;
        m_mapSBinInfo[clRecord.m_u2SBIN_NUM].m_cBinCat = ' ';
    }

    // IF THE BINCAT NOT DEFINED (SBR CORRUPTED)
    // UPDATE IT WITH INFO FROM PRR
    // GCORE-3283
    bool lValidBinCat = (clRecord.m_c1SBIN_PF == 'P')
            || (clRecord.m_c1SBIN_PF == 'F')
            || (clRecord.m_c1SBIN_PF == 'A');

    // update SBin statique info
    if(lValidBinCat)
        m_mapSBinInfo[clRecord.m_u2SBIN_NUM].m_cBinCat = clRecord.m_c1SBIN_PF;
    else
    {

        // Only force the BinCat if no PRR
        if(m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR] == 0)
        {
            if(clRecord.m_u2SBIN_NUM == 1)
                m_mapSBinInfo[clRecord.m_u2SBIN_NUM].m_cBinCat = 'P';
            else
                m_mapSBinInfo[clRecord.m_u2SBIN_NUM].m_cBinCat = 'F';
        }
    }

    QString lBinName = QString(clRecord.m_cnSBIN_NAM).trimmed();
    if(!lBinName.isEmpty())
        m_mapSBinInfo[clRecord.m_u2SBIN_NUM].m_strBinName = lBinName;

    if(mpDbKeysEngine->dbKeysContent().Get("IgnoreSummary").toBool())
    {
        m_nStdfRecordsCount[GQTL_STDF::Stdf_PCR_V4::Rec_SBR]--;
        return true;
    }

    // Save SBR info
    int nSiteNum =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
            ( m_clStdfParse, clRecord );

    BYTE headNum =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
            ( m_clStdfParse, clRecord );

    if(headNum == 255)
        nSiteNum = MERGE_SITE;

    // Update Rebuild merge sites information
    if(!m_mapSBinInfo[clRecord.m_u2SBIN_NUM].m_mapBinCnt.contains(INVALID_SITE))
        m_mapSBinInfo[clRecord.m_u2SBIN_NUM].m_mapBinCnt[INVALID_SITE]=0;
    if(nSiteNum != MERGE_SITE)
        m_mapSBinInfo[clRecord.m_u2SBIN_NUM].m_mapBinCnt[INVALID_SITE]+=clRecord.m_u4SBIN_CNT;

    m_mapSBinInfo[clRecord.m_u2SBIN_NUM].m_mapBinCnt[nSiteNum]=clRecord.m_u4SBIN_CNT;

    return true;
}

