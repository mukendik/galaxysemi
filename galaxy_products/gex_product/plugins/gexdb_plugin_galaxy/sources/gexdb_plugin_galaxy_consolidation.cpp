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

//////////////////////////////////////////////////////////////////////
// Update Consolidation results for Product level
//////////////////////////////////////////////////////////////////////
/// \brief GexDbPlugin_Galaxy::UpdateProductConsolidation
/// Update XX_PRODUCT_XBIN for PROD_DATA='Y' from Lot Consolidation results
/// \param ProductName
/// \return
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateProductConsolidation(QString &ProductName)
{
    bool bStatus = true;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Update Consolidation results for Product level (%1)...").arg(ProductName).toLatin1().constData());

    if(ProductName.isEmpty())
        return true;

    // first verify if have already information into the table
    QString            lQuery;
    GexDbPlugin_Query  clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    QMap<int,structBinInfo> mapSBinInfo;
    QMap<int,structBinInfo> mapHBinInfo;
    QMap<int,int>           mapHBin;
    QMap<int,int>           mapSBin;
    QMap<int,structBinInfo>::Iterator itBinInfo;
    int     nBinNo;
    QString lBinCat;
    QString lProdData;
    QString lQueryHeader; // INSERT INTO ...
    QString lNewValue;    // NEW VALUE FOR INSERTION
    QString lAllValues;   // MULTI QUERIES
    QString strTokenKey;

    bStatus = false;

    /////////////////////////
    // DEADLOCK PREVENTION
    // Clean the Token table if needed
    if (! InitTokens(NormalizeTableName("_CONSOLIDATION")))
    {
        return false;
    }
    // Consolidation through Incremental Update
    // Fix collision on same Product
    strTokenKey = ProductName;
    if(!GetToken(NormalizeTableName("_CONSOLIDATION"),strTokenKey,15))
    {
        // Reject the current consolidation
        goto labelUnlock;
    }

    ///////////////////////////////////////////////////////////
    // ONLY FOR PROD_DATA='Y'
    lProdData = "Y";

    // Reset PRODUCT_HBIN table
    lQuery =  "DELETE FROM " + NormalizeTableName("_PRODUCT_HBIN");
    lQuery += " WHERE PRODUCT_NAME="+TranslateStringToSqlVarChar(ProductName);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    // Reset PRODUCT_SBIN table
    lQuery =  "DELETE FROM " + NormalizeTableName("_PRODUCT_SBIN");
    lQuery += " WHERE PRODUCT_NAME="+TranslateStringToSqlVarChar(ProductName);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }


    ///////////////////////////////////////////////////////////
    // Lot Binning Consolidation
    // COLLECT HBIN NUM,NAME,CAT FROM PREVIOUS INSERTION
    // COLLECT SBIN NUM,NAME,CAT FROM PREVIOUS INSERTION
    ///////////////////////////////////////////////////////////

    // Update HBin count
    lQuery =  " SELECT count(*)";
    lQuery += " FROM "+NormalizeTableName("_LOT_HBIN") + " B ";
    lQuery += " WHERE ";
    lQuery += "    B.PRODUCT_NAME="+TranslateStringToSqlVarChar(ProductName);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    if(clGexDbQuery.First() && (clGexDbQuery.value(0).toInt()>0))
    {
        // Update mapHbinInfo and mapSBinInfo with previous info
        // For HBIN
        // FROM SPLITLOT(LOT/PROD_DATA
        lQuery =  " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" B.HBIN_NO, ";
        lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.HBIN_CAT)),'#:#',-1) AS HBIN_CAT, ";
        lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.HBIN_NAME)),'#:#',-1) AS HBIN_NAME ";
        lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery += " INNER JOIN "+NormalizeTableName("_LOT_HBIN")+" B ";
        lQuery += "     ON S.LOT_ID=B.LOT_ID AND S.PRODUCT_NAME=B.PRODUCT_NAME";
        lQuery += " WHERE S.PRODUCT_NAME="+TranslateStringToSqlVarChar(ProductName);
        lQuery += "   AND S.PROD_DATA="+TranslateStringToSqlVarChar(lProdData);
        lQuery += "   AND S.VALID_SPLITLOT<>'N'";
        lQuery += " GROUP BY B.HBIN_NO";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
        while(clGexDbQuery.Next())
        {
            // update values for Bin info
            nBinNo = clGexDbQuery.value(0).toInt();
            lBinCat = clGexDbQuery.value(1).toString().toUpper();
            mapHBinInfo[nBinNo].m_nBinNum = nBinNo;
            // Keep the last definition according to the start_t
            if(mapHBinInfo[nBinNo].m_strBinName.isEmpty())
                mapHBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(2).toString();

            if(lBinCat=="P")
                mapHBinInfo[nBinNo].m_cBinCat = 'P';
            else if(lBinCat=="A")
                mapHBinInfo[nBinNo].m_cBinCat = 'A';
            else
                mapHBinInfo[nBinNo].m_cBinCat = 'F';
            // GET ALSO BINNING FROM INTERMEDIATE CONSOLIDATION FOR AER
            mapHBin[nBinNo] = 0;
        }

        // Update HBin count
        lQuery =  " SELECT ";
        lQuery += "    B.HBIN_NO, ";
        lQuery += "    SUM(B.NB_PARTS) AS BIN_COUNT,";
        lQuery += "    MAX(B.HBIN_CAT),";
        lQuery += "    MAX(B.HBIN_NAME)";
        lQuery += " FROM "+NormalizeTableName("_LOT_HBIN") + " B ";
        lQuery += " WHERE ";
        lQuery += "    B.PRODUCT_NAME="+TranslateStringToSqlVarChar(ProductName);
        lQuery += " GROUP BY B.HBIN_NO";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        while(clGexDbQuery.Next())
        {
            // update values for Bin info
            nBinNo = clGexDbQuery.value(0).toInt();
            if(!mapHBinInfo.contains(nBinNo))
            {
                // If Initial Test doesn't contains this BinNo
                // just create it
                lBinCat = clGexDbQuery.value(2).toString().toUpper();
                if(lBinCat=="P")
                    mapHBinInfo[nBinNo].m_cBinCat = 'P';
                else if(lBinCat=="A")
                    mapHBinInfo[nBinNo].m_cBinCat = 'A';
                else
                    mapHBinInfo[nBinNo].m_cBinCat = 'F';
                mapHBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(3).toString();
            }
            mapHBin[nBinNo]=clGexDbQuery.value(1).toInt();
        }

        // for each HBIN
        lQueryHeader = "INSERT INTO " + NormalizeTableName("_PRODUCT_HBIN") + " VALUES";
        lNewValue = lAllValues = "";

        for(itBinInfo=mapHBinInfo.begin(); itBinInfo!=mapHBinInfo.end(); itBinInfo++)
        {
            nBinNo = itBinInfo.key();

            // ignore specific Bin -1 used to have global summary on all Bin
            if(nBinNo == MERGE_BIN)
                continue;

            if(!mapHBin.contains(nBinNo))
                mapHBin[nBinNo] = 0;

            // insert new entry
            lNewValue = "(";
            // PRODUCT_NAME
            lNewValue += TranslateStringToSqlVarChar(ProductName);
            lNewValue += ",";
            // HBIN_NO
            lNewValue += QString::number(nBinNo);
            lNewValue += ",";
            // HBIN_NAME
            lNewValue += TranslateStringToSqlVarChar(mapHBinInfo[nBinNo].m_strBinName);
            lNewValue += ",";
            // HBIN_CAT
            lNewValue += mapHBinInfo[nBinNo].m_cBinCat == ' ' ? "null" : TranslateStringToSqlVarChar((QChar)mapHBinInfo[nBinNo].m_cBinCat);
            lNewValue += ",";
            // NB_PARTS
            lNewValue += QString::number(mapHBin[nBinNo]);
            lNewValue += ")";

            if(!AddInMultiInsertQuery(NormalizeTableName("_PRODUCT_HBIN"), lQueryHeader, lNewValue, lAllValues))
                goto labelUnlock;
        }

        if(!lAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_PRODUCT_HBIN"), lAllValues)))
            goto labelUnlock;
    }
    lQuery =  " SELECT count(*)";
    lQuery += " FROM "+NormalizeTableName("_LOT_SBIN") + " B ";
    lQuery += " WHERE ";
    lQuery += "    B.PRODUCT_NAME="+TranslateStringToSqlVarChar(ProductName);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    if(clGexDbQuery.First() && (clGexDbQuery.value(0).toInt()>0))
    {
        // For SBIN
        lQuery =  " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" B.SBIN_NO, ";
        lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.SBIN_CAT)),'#:#',-1) AS SBIN_CAT, ";
        lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.SBIN_NAME)),'#:#',-1) AS SBIN_NAME ";
        lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery += " INNER JOIN "+NormalizeTableName("_LOT_SBIN")+" B ";
        lQuery += "     ON S.LOT_ID=B.LOT_ID AND S.PRODUCT_NAME=B.PRODUCT_NAME";
        lQuery += " WHERE S.PRODUCT_NAME="+TranslateStringToSqlVarChar(ProductName);
        lQuery += "   AND S.PROD_DATA="+TranslateStringToSqlVarChar(lProdData);
        lQuery += "   AND S.VALID_SPLITLOT<>'N'";
        lQuery += " GROUP BY B.SBIN_NO";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        while(clGexDbQuery.Next())
        {
            // update values for Bin info
            nBinNo = clGexDbQuery.value(0).toInt();
            lBinCat = clGexDbQuery.value(1).toString().toUpper();
            mapSBinInfo[nBinNo].m_nBinNum = nBinNo;
            // Keep the last definition according to the start_t
            if(mapSBinInfo[nBinNo].m_strBinName.isEmpty())
                mapSBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(2).toString();
            if(lBinCat=="P")
                mapSBinInfo[nBinNo].m_cBinCat = 'P';
            else if(lBinCat=="A")
                mapSBinInfo[nBinNo].m_cBinCat = 'A';
            else
                mapSBinInfo[nBinNo].m_cBinCat = 'F';
            // GET ALSO BINNING FROM INTERMEDIATE CONSOLIDATION FOR AER
            mapSBin[nBinNo] = 0;
        }

        // Update SBin count
        lQuery =  " SELECT B.SBIN_NO, ";
        lQuery += "    SUM(B.NB_PARTS) AS BIN_COUNT,";
        lQuery += "    MAX(B.SBIN_CAT),";
        lQuery += "    MAX(B.SBIN_NAME)";
        lQuery += " FROM "+NormalizeTableName("_LOT_SBIN") + " B ";
        lQuery += " WHERE ";
        lQuery += "    B.PRODUCT_NAME="+TranslateStringToSqlVarChar(ProductName);
        lQuery += " GROUP BY B.SBIN_NO";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        while(clGexDbQuery.Next())
        {
            // update values for Bin info
            nBinNo = clGexDbQuery.value(0).toInt();
            if(!mapSBinInfo.contains(nBinNo))
            {
                // If Initial Test doesn't contains this BinNo
                // just create it
                lBinCat = clGexDbQuery.value(2).toString().toUpper();
                if(lBinCat=="P")
                    mapSBinInfo[nBinNo].m_cBinCat = 'P';
                else if(lBinCat=="A")
                    mapSBinInfo[nBinNo].m_cBinCat = 'A';
                else
                    mapSBinInfo[nBinNo].m_cBinCat = 'F';
                mapSBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(3).toString();
            }
            mapSBin[nBinNo]=clGexDbQuery.value(1).toInt();
        }


        // for each SBIN
        lQueryHeader = "INSERT INTO " + NormalizeTableName("_PRODUCT_SBIN") + " VALUES";
        lNewValue = lAllValues = "";

        for(itBinInfo=mapSBinInfo.begin(); itBinInfo!=mapSBinInfo.end(); itBinInfo++)
        {
            nBinNo = itBinInfo.key();

            // ignore specific Bin -1 used to have global summary on all Bin
            if(nBinNo == MERGE_BIN)
                continue;

            if(!mapSBin.contains(nBinNo))
                mapSBin[nBinNo] = 0;

            // insert new entry
            lNewValue = "(";
            // PRODUCT_NAME
            lNewValue += TranslateStringToSqlVarChar(ProductName);
            lNewValue += ",";
            // SBIN_NO
            lNewValue += QString::number(nBinNo);
            lNewValue += ",";
            // SBIN_NAME
            lNewValue += TranslateStringToSqlVarChar(mapSBinInfo[nBinNo].m_strBinName);
            lNewValue += ",";
            // SBIN_CAT
            lNewValue += mapSBinInfo[nBinNo].m_cBinCat == ' ' ? "null" : TranslateStringToSqlVarChar((QChar)mapSBinInfo[nBinNo].m_cBinCat);
            lNewValue += ",";
            // NB_PARTS
            lNewValue += QString::number(mapSBin[nBinNo]);
            lNewValue += ")";

            if(!AddInMultiInsertQuery(NormalizeTableName("_PRODUCT_SBIN"), lQueryHeader, lNewValue, lAllValues))
                goto labelUnlock;
        }

        if(!lAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_PRODUCT_SBIN"), lAllValues)))
            goto labelUnlock;
    }

    bStatus = true;

labelUnlock:

    ReleaseToken(NormalizeTableName("_CONSOLIDATION"),strTokenKey);

    return bStatus;
}


///////////////////////////////////////////////////////////////////////
////// \brief GexDbPlugin_Galaxy::UpdateSubLotBinningConsolidationTable
/// Update XX_SUBLOT_XBIN for PROD_DATA='Y' from Consolidation results
/// Update XX_SUBLOT_CONSOLIDATION for PROD_DATA='Y' from Consolidation results
/// Update XX_SUBLOT_CONSOLIDATION_INTER for all PROD_DATA from Flexible Consolidation results
////// \param LotId
////// \param SubLotId
////// \param ProdData
////// \param PhysicalConsolidationName: the name of the PHYSICAL Consolidation name
////// \param ConsolidationSummary: Error message if any
////// \return
bool GexDbPlugin_Galaxy::UpdateSubLotBinningConsolidationTable(const QString &LotId,const QString &SubLotId,const QString &ProdData,
                                                       QString &PhysicalConsolidationName, QString &ConsolidationSummary)
{
    if(m_eTestingStage != eFinalTest)
        return true;


    QString lMessage = "Update SubLot Binning Consolidation : Consolidation process for LotId = " + LotId;
    lMessage += "and SubLotId = "+SubLotId;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data() );

    TimersTraceStart("Function SubLot Binning Consolidation");

    QString             lQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);

    ///////////////////////////////////////////////////////////
    // SubLot Binning Consolidation
    ///////////////////////////////////////////////////////////
    QString lQueryHeader;      // INSERT INTO ...
    QString lNewValue;         // NEW VALUE FOR INSERTION
    QString lAllValues;        // MULTI QUERIES
    QString lValue;

    int     nNbParts=0;
    int     nNbPartsGood=0;
    int     lRetestMissingPartsBin;

    QStringList                             lstPhases;
    QMap<QString, QMap<int,QStringList> >   mapPhaseRetestHBins;
    QMap<QString, int>                      mapPhaseMissingParts;

    // For HBin and SBin consolidation per wafer
    QMap<int,structBinInfo> mapHBinInfo;
    QMap<int,structBinInfo> mapSBinInfo;
    QMap<int,structBinInfo>::Iterator   itBinInfo;
    QMap<int,int>::Iterator             itBinCount;
    int                             nBinNo;
    QString                         lBinCat;
    QMap<int,int>                   mapHBin;
    QMap<int,int>                   mapSBin;
    QMap<int,int>                   mapPhysicalHBin;
    QMap<QString, QMap<int,int> >   mapPhaseHBin;
    QMap<QString, QMap<int,int> >   mapPhaseSBin;
    QMap<QString, int >             mapPhaseNbParts;
    QMap<QString, int >             mapPhaseNbPartsGood;
    QList<int>                      lstRetestIndex;

    QString lSuffixTable;
    QString lPhaseType;
    QMap<QString, int>::Iterator itPhase;

    bool        bStatus = true;
    bool        lConsolidateBinning = false;
    bool        lConsolidationFromSamples = false;
    bool        lUpdateFailingTest = false;
    QString     lPhaseName;


    QString                lSamplesData;
    bool                   lUpdateTableForPhysicalData = (ProdData == "Y");
    QString                lProdData = ProdData;
    QString                lTestFlow;
    QStringList            lSplitlots;


    TimersTraceStart("Function SubLot Binning Consolidation");

    ///////////////////////////////////////////////////////////
    // CHECK IF AT LEAST ONE SPLITLOT WAS FLAGED WITH BINNING_CONSOLIDATION
    ///////////////////////////////////////////////////////////
    // PROD_DATA INFO
    // PROD_DATA, START_TIME, SAMPLES_FLAGS
    // FROM SPLITLOT(LOT/WAFER)
    ///////////////////////////////////////////////////////////
    // FOR ALL TEST CONDITIONS
    // Upate lot information with data
    // from production data if have
    // else from non-production data

    // Construct the query condition to select only wafer with data samples
    // FT_SPLITLOT.splitlot_flags & Bit2(0x04)
    lSamplesData = "(splitlot_flags & "+QString::number(FLAG_SPLITLOT_PARTSFROMSAMPLES)+")";

    // Check if have to use SUMMARY
    // Check if have
    // The test flow is not totally implemented
    // Use the first one
    lQuery = "SELECT MAX(TEST_FLOW) AS TEST_FLOW, ";
    lQuery += " MAX("+lSamplesData+") AS SAMPLES_DATA ";
    lQuery += " FROM "+NormalizeTableName("_SPLITLOT");
    lQuery += " WHERE ";
    lQuery += "   VALID_SPLITLOT<>'N'";
    lQuery += "   AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "   AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
    lQuery += "   AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
    if(!clGexDbQuery.Execute(lQuery) || !clGexDbQuery.First())
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    lConsolidationFromSamples = !(clGexDbQuery.value("SAMPLES_DATA").toInt() == 0);
    lTestFlow = clGexDbQuery.value("TEST_FLOW").toString();

    ///////////////////////////////////////
    // START CONSOLIDATION NOW
    ///////////////////////////////////////

    ///////////////////////////////////////////////////////////
    // FT_YIELDCONSOLIDATION_MISSINGPARTSBIN INFO
    ///////////////////////////////////////////////////////////
    // if a MissingBin can be define
    // During the insertion, the config.gexdbkeys is present
    // but during the incremental update BINNING_CONSOLIDATION
    // the file config.gexdbkeys isn't define
    // Use the var env GEX_FT_YIELDCONSOLIDATION_MISSINGPARTSBIN
    lRetestMissingPartsBin = -1;
    GetGlobalOptionValue(eBinningFtYieldConsolidationMissingPartsBin,lValue);
    if(!lValue.isEmpty())
        lRetestMissingPartsBin = lValue.toInt();

    ///////////////////////////////
    /// FINAL TEST INTERMEDIATE CONSOLIDATION
    // Intermediate consolidation is possible
    // only for the 'standard' flow
    // NO PHASE
    // NO PHASE + NEW PHASE ?
    // PHASE A - RETEST_INDEX 0,1,2, ...
    // PHASE B - RETEST_INDEX 0,1,2, ...
    // PHASE A + StartTime < PHASE b +StartTime
    /// FINAL TEST INTERMEDIATE CONSOLIDATION
    ///////////////////////////////
    // Init with false
    bStatus = false;

    // List of Retest bins
    // With this information, we can reset all HBin/SBin from RetestIndex-1 and add new HBin/SBin from RetestIndex
    nNbParts=0;
    nNbPartsGood=0;

    if(!GetConsolidationRetestInfo(LotId, SubLotId, lProdData, lstPhases, mapPhaseRetestHBins))
        goto labelUnlock;

    if(!lstPhases.isEmpty())
        lConsolidateBinning = true;

    ///////////////////////////////////////////////////////////
    // RETEST INFO
    ///////////////////////////////////////////////////////////
    // Final Test
    // List of HBins retested from RetestIndex
    // R1 = R0.bin7 => <0,<7>>
    // R2 = R1.bin8 => <1,<8>>
    // for FinalTest
    // if(m_eTestingStage == eFinalTest)
    if(lConsolidateBinning)
    {
        if(lUpdateFailingTest)
        {
            // This step is to correct the PART_RETEST_INDEX
            // if the file was inserted before the feature
            // BUT AT FINAL TEST, the INLINE part_retest is less than 1% ?

            // Check if we need to recompute the PART_FLAGS into the RUN
            // based of the PART_RETEST_INDEX
            lQuery = " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" GROUP_CONCAT(distinct R.SPLITLOT_ID) AS splitlots, "
                     "  sum(R.PART_RETEST_INDEX>0) AS nb_retests, "
                     "  sum((R.PART_FLAGS&"+QString::number(FLAG_RUN_PARTRETESTED)+")="+QString::number(FLAG_RUN_PARTRETESTED)+") as nb_retested "
                     " FROM "+NormalizeTableName("_SPLITLOT")+" S "
                     " INNER JOIN "+NormalizeTableName("_RUN")+" R "
                     "  ON S.SPLITLOT_ID=R.SPLITLOT_ID "
                     " WHERE S.VALID_SPLITLOT<>'N'"
                    "   AND  S.LOT_ID="+TranslateStringToSqlVarChar(LotId)+
                    "   AND  S.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId)+
                    "   AND  S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
                    //"   AND  S.RETEST_INDEX="+QString::number(nRetestIndex)+
                    //"   AND  S.TEST_INSERTION="+TranslateStringToSqlVarChar(lPhaseName);
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
            if(clGexDbQuery.First())
            {
                if((clGexDbQuery.value("nb_retested").toInt()==0)
                        && (clGexDbQuery.value("nb_retests").toInt()>0))
                {
                    // The RUN.PART_FLAGS is not up-to-date
                    // Need to recompute the flags
                    QStringList lSplitlots = clGexDbQuery.value("splitlots").toString().split(",");
                    while(!lSplitlots.isEmpty())
                    {
                        UpdateRunTablePartFlags(lSplitlots.takeFirst().toLong());
                    }
                }
            }
        }


        ///////////////////////////////////////////////////////////
        // Always update the RUN CONSOLIDATION FLAGS
        // Even if SUMMARY or SAMPLING
        if(!UpdateSubLotRunConsolidationTable(LotId, SubLotId, ProdData, lstPhases, mapPhaseRetestHBins, mapPhaseMissingParts))
            goto labelUnlock;

        ///////////////////////////////////////////////////////////
        // Lot Binning Consolidation
        // COLLECT HBIN NUM,NAME,CAT FROM PREVIOUS INSERTION
        // COLLECT SBIN NUM,NAME,CAT FROM PREVIOUS INSERTION
        ///////////////////////////////////////////////////////////
        // Update mapHbinInfo and mapSBinInfo with previous info
        // For HBIN
        // FROM SPLITLOT/LOT/PROD_DATA
        lQuery =  " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" B.HBIN_NO, ";
        lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.HBIN_CAT)),'#:#',-1) AS HBIN_CAT, ";
        lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.HBIN_NAME)),'#:#',-1) AS HBIN_NAME ";
        lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery += " INNER JOIN "+NormalizeTableName("_HBIN")+" B ";
        lQuery += "     ON S.SPLITLOT_ID=B.SPLITLOT_ID ";
        lQuery += " WHERE ";
        lQuery += "   S.VALID_SPLITLOT<>'N'";
        lQuery += "   AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "   AND S.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
        lQuery += "   AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        lQuery += " GROUP BY B.HBIN_NO";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
        while(clGexDbQuery.Next())
        {
            // update values for Bin info
            nBinNo = clGexDbQuery.value(0).toInt();
            lBinCat = clGexDbQuery.value(1).toString().toUpper();
            mapHBinInfo[nBinNo].m_nBinNum = nBinNo;
            // Keep the last definition according to the start_t
            if(mapHBinInfo[nBinNo].m_strBinName.isEmpty())
                mapHBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(2).toString();

            if(lBinCat=="P")
                mapHBinInfo[nBinNo].m_cBinCat = 'P';
            else if(lBinCat=="A")
                mapHBinInfo[nBinNo].m_cBinCat = 'A';
            else
                mapHBinInfo[nBinNo].m_cBinCat = 'F';
        }

        // The RUN CONSOLIDATION flags was populated
        // Check if have SAMPLING for each PhaseName
        // Because we need to generate the run consolidation flags for all consolidation_name
        // the HBin/SBin consolidated count can be extract from this table
        // Special case:
        // if the SUMMARY=NB_PARTS (no samplings), the process can stop here
        // if the SUMMARY>NB_PARTS (samplings), we need to compute also the consolidated count from the HBin summary
        // the consolidated count for the SBin will be skipped (empty ft_sublot_sbin)

        // Init the MissingParts
        if(lRetestMissingPartsBin)
        {
            mapHBinInfo[lRetestMissingPartsBin].m_nBinNum = lRetestMissingPartsBin;
            mapHBinInfo[lRetestMissingPartsBin].m_strBinName = "Missing Parts";
            mapHBinInfo[lRetestMissingPartsBin].m_cBinCat = 'F';
            mapSBinInfo[lRetestMissingPartsBin].m_nBinNum = lRetestMissingPartsBin;
            mapSBinInfo[lRetestMissingPartsBin].m_strBinName = "Missing Parts";
            mapSBinInfo[lRetestMissingPartsBin].m_cBinCat = 'F';
        }

        if(!lConsolidationFromSamples)
        {
            ///////////////////////////////////////////////////////////
            // Cannot use the RUN CONSOLIDATION flags
            // because SAMPLING or SUMMARY
            // Try to do the consolidation with Summary data

            ConsolidationSummary = "Consolidation from summary";

            // For all HarbBin Consolidation
            // lPhases contains the list of all phases order by StartTime
            // if no Phase defined, the lPhases contains an empty string

            // List of all HBin
            int nRetestIndex;
            int nTotalParts=0;

            TimersTraceStart("Function SubLot Binning Consolidation: consolidation phase");

            // GET THE LIST OF ALL HBIN AND SBIN
            int     i, nBinNo;

            // use the struct HBinInfo for retest information
            // HBinInfo[nBinNo].m_mapBinCnt[nRetestIndex] : Nb of Parts with bin=nBinNo for the retest=nRetestIndex
            // HBinInfo[nBinNo].m_mapBinNbParts[nRetestIndex] : Total of parts retested in this step
            // if more than 1 bin retested in the file, don't use m_mapBinNbParts but reset binCnt[nRetestIndex-1]

            // List of all binnings from Summary
            // For HBIN
            // For each phases
            // Start with the first phase
            // Reset needed bin counts between 2 phases (check RETEST_HBINS)
            // Test/Retest phase results are stored into m_mapHBinInfo
            // Intermediate Phase results are stored into mapHBin
            //foreach(lPhaseName, lstPhases)
            for(int index=0; index<lstPhases.count(); ++index)
            {
                lPhaseName = lstPhases.at(index);

                // reset all bin count from the Test/Retest phase
                mapHBin.clear();
                for ( itBinInfo = mapHBinInfo.begin(); itBinInfo != mapHBinInfo.end(); ++itBinInfo )
                    mapHBinInfo[itBinInfo.key()].m_mapBinCnt.clear();

                // Initialize bin count for Test/Retest phase
                lQuery = " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" ";
                lQuery += "     B.HBIN_NO, ";
                lQuery += "     S.RETEST_INDEX, ";
                lQuery += "     SUM(IF(NOT S.NB_PARTS=S.NB_PARTS_SAMPLES , IF(BS.SITE_NO="+QString::number(MERGE_SITE)+" , BS.BIN_COUNT , 0 ) , 0 )) AS SITE_MERGE, ";
                lQuery += "     SUM(IF(NOT S.NB_PARTS=S.NB_PARTS_SAMPLES , IF(NOT BS.SITE_NO="+QString::number(MERGE_SITE)+" , BS.BIN_COUNT , 0 ) , 0 )) AS NB_PARTS ";
                lQuery += " FROM "+NormalizeTableName("_SPLITLOT")+" S ";
                lQuery += " INNER JOIN "+NormalizeTableName("_HBIN")+" B ";
                lQuery += "     ON S.SPLITLOT_ID=B.SPLITLOT_ID ";
                lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
                lQuery += "     AND S.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
                lQuery += "     AND  (S.TEST_INSERTION="+TranslateStringToSqlVarChar(lPhaseName);
                if(lPhaseName.isEmpty())
                    lQuery+= " OR S.TEST_INSERTION IS NULL";
                lQuery += "     ) AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
                lQuery += "     AND S.VALID_SPLITLOT<>'N'";
                lQuery += " INNER JOIN "+NormalizeTableName("_HBIN_STATS_SUMMARY")+" BS ";
                lQuery += "     ON BS.SPLITLOT_ID=B.SPLITLOT_ID ";
                lQuery += "     AND B.HBIN_NO=BS.HBIN_NO ";
                lQuery += " GROUP BY B.HBIN_NO, S.RETEST_INDEX ";
                lQuery += " ORDER BY S.RETEST_INDEX";
                if(!clGexDbQuery.Execute(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                    goto labelUnlock;
                }
                while(clGexDbQuery.Next())
                {
                    // update values for Bin info
                    nBinNo = clGexDbQuery.value(0).toInt();
                    nRetestIndex = clGexDbQuery.value(1).toInt();
                    nNbParts = clGexDbQuery.value(2).toInt();
                    if(nNbParts == 0)
                        nNbParts = clGexDbQuery.value(3).toInt();

                    if(!mapHBinInfo.contains(nBinNo))
                        mapHBinInfo[nBinNo].m_cBinCat = 'F';

                    if(!mapHBinInfo[nBinNo].m_mapBinCnt.contains(nRetestIndex))
                        mapHBinInfo[nBinNo].m_mapBinCnt[nRetestIndex] = 0;
                    mapHBinInfo[nBinNo].m_mapBinCnt[nRetestIndex] += nNbParts;

                    // Update global counter for each RetestIndex
                    if(!mapHBinInfo[MERGE_BIN].m_mapBinCnt.contains(nRetestIndex))
                        mapHBinInfo[MERGE_BIN].m_mapBinCnt[nRetestIndex] = 0;
                    mapHBinInfo[MERGE_BIN].m_mapBinCnt[nRetestIndex] += nNbParts;
                }

                // DO WE REALLY NEED TO EXTRACT THE DATA FROM SAMPLES
                // WHEN WE KNOW THAT WE WANT DATA FROM SUMMARY ?????
                // List of all binnings from Samples
                // For HBIN
                lQuery = " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" ";
                lQuery += "     B.HBIN_NO, ";
                lQuery += "     S.RETEST_INDEX, ";
                lQuery += "     SUM(IF(S.NB_PARTS=S.NB_PARTS_SAMPLES , IF(BS.SITE_NO="+QString::number(MERGE_SITE)+" , BS.NB_PARTS , 0 ) , 0 )) AS SITE_MERGE, ";
                lQuery += "     SUM(IF(S.NB_PARTS=S.NB_PARTS_SAMPLES , IF(NOT BS.SITE_NO="+QString::number(MERGE_SITE)+" , BS.NB_PARTS , 0 ) , 0 )) AS NB_PARTS ";
                lQuery += " FROM "+NormalizeTableName("_SPLITLOT")+" S ";
                lQuery += " INNER JOIN "+NormalizeTableName("_HBIN")+" B ";
                lQuery += "     ON S.SPLITLOT_ID=B.SPLITLOT_ID ";
                lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
                lQuery += "     AND S.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
                lQuery += "     AND  (S.TEST_INSERTION="+TranslateStringToSqlVarChar(lPhaseName);
                if(lPhaseName.isEmpty())
                    lQuery+= " OR S.TEST_INSERTION IS NULL";
                lQuery += "     ) AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
                lQuery += "     AND S.VALID_SPLITLOT<>'N'";
                lQuery += " INNER JOIN "+NormalizeTableName("_HBIN_STATS_SAMPLES")+" BS ";
                lQuery += "     ON BS.SPLITLOT_ID=B.SPLITLOT_ID ";
                lQuery += "     AND B.HBIN_NO=BS.HBIN_NO ";
                lQuery += " GROUP BY B.HBIN_NO, S.RETEST_INDEX ";
                lQuery += " ORDER BY S.RETEST_INDEX";
                if(!clGexDbQuery.Execute(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                    goto labelUnlock;
                }
                while(clGexDbQuery.Next())
                {
                    // update values for Bin info
                    nBinNo = clGexDbQuery.value(0).toInt();
                    nRetestIndex = clGexDbQuery.value(1).toInt();
                    nNbParts = clGexDbQuery.value(2).toInt();
                    if(nNbParts == 0)
                        nNbParts = clGexDbQuery.value(3).toInt();

                    if(!mapHBinInfo.contains(nBinNo))
                        mapHBinInfo[nBinNo].m_cBinCat = 'F';

                    if(!mapHBinInfo[nBinNo].m_mapBinCnt.contains(nRetestIndex))
                        mapHBinInfo[nBinNo].m_mapBinCnt[nRetestIndex] = 0;
                    mapHBinInfo[nBinNo].m_mapBinCnt[nRetestIndex] += nNbParts;

                    // Update global counter for each RetestIndex
                    if(!mapHBinInfo[MERGE_BIN].m_mapBinCnt.contains(nRetestIndex))
                        mapHBinInfo[MERGE_BIN].m_mapBinCnt[nRetestIndex] = 0;
                    mapHBinInfo[MERGE_BIN].m_mapBinCnt[nRetestIndex] += nNbParts;
                }

                ///////////////////////////////////////////////////////////
                // Consolidation is only possible if we have all parts retested (from one or more HBin) present in the RestestIndex-1
                // If not, tables sublot, sublot_hbin and sublot_sbin must be reset.

                QList<int>::Iterator itRetestIndex;
                int nRetestLevel;
                lstRetestIndex = mapPhaseRetestHBins[lPhaseName].keys();
                for(itRetestIndex=lstRetestIndex.begin(); itRetestIndex!=lstRetestIndex.end(); itRetestIndex++)
                {
                    nNbParts = nNbPartsGood = 0;

                    nRetestIndex = (*itRetestIndex);
                    nRetestLevel = nRetestIndex-1;

                    if(nRetestIndex == 0)
                        continue;

                    for(int index=0; index<mapPhaseRetestHBins[lPhaseName][nRetestIndex].count(); ++index)
                    {
                        lValue = mapPhaseRetestHBins[lPhaseName][nRetestIndex].at(index);
                        i = lValue.toInt();
                        // Before reset HBin counter for RetestIndex-1
                        // Check if have TotalParts for RetestIndex <= NbBinRetested
                        if(mapHBinInfo.contains(i))
                        {
                            nNbParts += mapHBinInfo[i].m_mapBinCnt[nRetestLevel];

                            // Reset all parts in the N-1 retestIndex
                            mapHBinInfo[i].m_mapBinCnt[nRetestLevel]=0;
                        }
                    }

                    // Check if have TotalParts for RetestIndex <= NbBinRetested
                    // If the nb of parts for a given binning in the re-test N phase
                    // is greater than the nb of parts for this binning in the re-test N-1 phase
                    // then don't perform any consolidation, and the part counts in the ft_lot table should be 0.
                    // If the nb of parts is matching or greater in the N-1 phase,
                    // then first remove all parts for the given HBin, and the corresponding SBins from the N-1 phase,
                    // and add all parts of the re-test.

                    // NEW FEATURES
                    // if a MissingBin can be define
                    if(lRetestMissingPartsBin > 0)
                    {
                        mapHBinInfo[lRetestMissingPartsBin].m_mapBinCnt[nRetestIndex] = 0;
                        mapSBinInfo[lRetestMissingPartsBin].m_mapBinCnt[nRetestIndex] = 0;
                    }

                    int nParts = mapHBinInfo[MERGE_BIN].m_mapBinCnt[nRetestIndex];
                    if(nParts > nNbParts)
                    {
                        // NEW FEATURES
                        // if a MissingBin can be define or if all this level has been completely retested
                        if((lRetestMissingPartsBin > 0))// || bRetestLevelCompletelyRetested)
                        {
                        }
                        else
                        {
                            // No consolidation possible
                            lstRetestIndex.clear();
                            QString lMsg =
                                    QString("Missing some retest parts: nbParts retested=%1, nbParts from the previous level=%2 - Pending for new data")
                                    .arg(nParts).arg(nNbParts);
                            WarningMessage(lMsg);
                            GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported,NULL,
                                        lMsg.toLatin1().data());
                            goto labelUnlock;
                        }
                    }
                    else
                    {
                        // NEW FEATURES
                        // if a MissingBin can be define
                        if(lRetestMissingPartsBin > 0)
                        {
                            mapHBinInfo[lRetestMissingPartsBin].m_mapBinCnt[nRetestIndex] = nNbParts - nParts;
                            mapSBinInfo[lRetestMissingPartsBin].m_mapBinCnt[nRetestIndex] = nNbParts - nParts;
                        }
                    }
                }

                nNbParts = nNbPartsGood = 0;

                // lstRetestIndex.isEmpty()
                // No consolidation possible
                // have to reset the lot/lot_hbin/lot_sbin table
                // else
                // Update consolidation information
                if(lstRetestIndex.isEmpty())
                {
                    mapPhysicalHBin.clear();
                    mapHBin.clear();
                    nNbParts = nTotalParts = nNbPartsGood = 0;
                    lConsolidateBinning = false;
                    goto labelUnlock;
                }
                else
                {
                    // For HBIN
                    for ( itBinInfo = mapHBinInfo.begin(); itBinInfo != mapHBinInfo.end(); ++itBinInfo )
                    {
                        if(itBinInfo.key() == MERGE_BIN)
                            continue;

                        for(itBinCount = itBinInfo.value().m_mapBinCnt.begin() ; itBinCount != itBinInfo.value().m_mapBinCnt.end(); ++itBinCount )
                        {
                            if(itBinInfo.value().m_mapBinCnt[itBinCount.key()] == 0)
                                continue;

                            if(itBinInfo.value().m_cBinCat == 'P')
                                nNbPartsGood += itBinInfo.value().m_mapBinCnt[itBinCount.key()];
                            nNbParts += itBinInfo.value().m_mapBinCnt[itBinCount.key()];
                            // For the current Intermediate
                            if(mapHBin.contains(itBinInfo.key()))
                                mapHBin[itBinInfo.key()] += itBinInfo.value().m_mapBinCnt[itBinCount.key()];
                            else
                                mapHBin[itBinInfo.key()] = itBinInfo.value().m_mapBinCnt[itBinCount.key()];
                        }
                    }
                    // Use only the NbParts from the first step if initialized
                    nTotalParts = mapHBinInfo[MERGE_BIN].m_mapBinCnt[0];
                    if(nTotalParts > 0)
                    {
                        if(lRetestMissingPartsBin > 0)
                            nTotalParts = nNbParts;

                        nNbParts = nTotalParts;
                    }
                }

                // reset or initialize bin count from intermediate phase
                if(!lPhaseName.isEmpty())
                {
                    // Before each new Phase, we need to initialize the bin count with the previous phase
                    // if all_pass => reset all bin counts for bin PASS
                    // if all_fail => reset all bin counts for bin FAIL
                    // if no info or all => reset all
                    // First, get the RETEST_HBINS for RETEST_INDEX=0

                    // INTO mapPhysicalHBin, the last consolidated level
                    if(!mapPhaseRetestHBins.contains(lPhaseName))
                    {
                        GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, "mapPhaseRetestHBins.contains(lPhaseName)");
                        goto labelUnlock;
                    }
                    if(!mapPhaseRetestHBins[lPhaseName].contains(0))
                    {
                        GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, "mapPhaseRetestHBins[lPhaseName].contains(0)");
                        goto labelUnlock;
                    }

                    // Total parts retested
                    nTotalParts = 0;
                    for(itBinCount=mapPhysicalHBin.begin(); itBinCount!=mapPhysicalHBin.end(); itBinCount++)
                        nTotalParts += mapPhysicalHBin[itBinCount.key()];

                    if(mapPhaseRetestHBins[lPhaseName][0].isEmpty())
                        mapPhysicalHBin.clear();
                    else
                    {
                        lValue = mapPhaseRetestHBins[lPhaseName][0].first();
                        // Reset only bin retested
                        // The retest PHASE can be 'all', 'all_reset', 'all_pass', 'all_fail', ''
                        if(lValue.isEmpty() || lValue.startsWith("all", Qt::CaseInsensitive))
                        {
                            if(lValue.isEmpty() || (lValue == "all") || (lValue == "all_reset"))
                                mapPhysicalHBin.clear();
                            else
                            {
                                char lCat = 'P';
                                bool bCheckPass = true;
                                if(lValue == "all_fail")
                                    bCheckPass = false;
                                nTotalParts = 0;
                                for(itBinCount=mapPhysicalHBin.begin(); itBinCount!=mapPhysicalHBin.end(); itBinCount++)
                                {
                                    if(mapHBinInfo.contains(itBinCount.key())
                                            && ((mapHBinInfo[itBinCount.key()].m_cBinCat == lCat) == bCheckPass))
                                    {
                                        nTotalParts += mapPhysicalHBin[itBinCount.key()];
                                        mapPhysicalHBin[itBinCount.key()] = 0;
                                    }
                                }
                            }
                        }
                        else
                        {
                            nTotalParts = 0;
                            for(itBinCount=mapPhysicalHBin.begin(); itBinCount!=mapPhysicalHBin.end(); itBinCount++)
                            {
                                if(mapPhaseRetestHBins[lPhaseName][0].contains(QString::number(itBinCount.key())))
                                {
                                    nTotalParts += mapPhysicalHBin[itBinCount.key()];
                                    mapPhysicalHBin[itBinCount.key()] = 0;
                                }
                            }
                        }
                    }

                    // Check if have to add MISSING PARTS
                    if(nTotalParts > 0)
                    {
                        if(nTotalParts > mapHBinInfo[MERGE_BIN].m_mapBinCnt[0])
                        {
                            // NEW FEATURES
                            // if a MissingBin can be define
                            if(lRetestMissingPartsBin > 0)
                                mapPhysicalHBin[lRetestMissingPartsBin] += nTotalParts - mapHBinInfo[MERGE_BIN].m_mapBinCnt[0];
                        }
                    }
                    // Update the PHYSICAL count with the current PHASE
                    for(itBinCount=mapHBin.begin(); itBinCount!=mapHBin.end(); itBinCount++)
                        mapPhysicalHBin[itBinCount.key()] += itBinCount.value();

                }

                // Save the result of the consolidation for the current phase
                if(lPhaseName.isEmpty())
                    lPhaseName = "FINAL";
                mapPhaseHBin[lPhaseName] = mapHBin;
                mapPhaseNbParts[lPhaseName] = nNbParts;
                mapPhaseNbPartsGood[lPhaseName] = nNbPartsGood;
            }
        }
        else
        {
            if(lRetestMissingPartsBin<0 && !mapPhaseMissingParts.isEmpty())
            {
                // No consolidation possible
                lstRetestIndex.clear();
                QString lMsg =
                        QString("Missing some retest parts: need to activate the BINNING_FT_YIELDCONSOLIDATION_MISSINGPARTSBIN option - No consolidation possible");
                WarningMessage(lMsg);
                GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported,NULL,
                            lMsg.toLatin1().data());
                goto labelUnlock;
            }

            ///////////////////////////////////////////////////////
            // All the needed parts are into the RUN CONSOLIDATION flags
            // Use the new RUN CONSOLIDATION flags
            // SoftBinConsolidation is AVAILABLE
            // For SBIN
            lQuery = " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" B.SBIN_NO, ";
            lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.SBIN_CAT)),'#:#',-1) AS SBIN_CAT, ";
            lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.SBIN_NAME)),'#:#',-1) AS SBIN_NAME ";
            lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" S ";
            lQuery += " INNER JOIN "+NormalizeTableName("_SBIN")+" B ";
            lQuery += "     ON B.SPLITLOT_ID=S.SPLITLOT_ID ";
            lQuery += " WHERE ";
            lQuery += "   S.VALID_SPLITLOT<>'N'";
            lQuery += "   AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
            lQuery += "   AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
            lQuery += "   AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
            lQuery += " GROUP BY B.SBIN_NO";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }

            while(clGexDbQuery.Next())
            {
                // update values for Bin info
                nBinNo = clGexDbQuery.value(0).toInt();
                lBinCat = clGexDbQuery.value(1).toString().toUpper();
                mapSBinInfo[nBinNo].m_nBinNum = nBinNo;
                // Keep the last definition according to the start_t
                if(mapSBinInfo[nBinNo].m_strBinName.isEmpty())
                    mapSBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(2).toString();
                if(lBinCat=="P")
                    mapSBinInfo[nBinNo].m_cBinCat = 'P';
                else if(lBinCat=="A")
                    mapSBinInfo[nBinNo].m_cBinCat = 'A';
                else
                    mapSBinInfo[nBinNo].m_cBinCat = 'F';
            }

            // For each RetestPhase
            // Extract Bin info from RUN CONSOLIDATION flags
            lQuery = "SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" S.TEST_INSERTION, "
                     " R.HBIN_NO, R.SBIN_NO, "
                     " COUNT(*) AS NB_PARTS, SUM(R.PART_STATUS='P') AS NB_PARTS_GOOD , "
                     " MIN(S.START_T) AS FIRT_START_T"
                     "  FROM "+NormalizeTableName("_SPLITLOT")+ " S "
                     " INNER JOIN "+NormalizeTableName("_RUN")+" R "
                     "      ON S.splitlot_id=R.splitlot_id AND R.intermediate=true  "
                     " WHERE "
                     "      S.VALID_SPLITLOT<>'N'"
                    "       AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId)+
                    "       AND S.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId)+
                    "       AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData)+
                    " GROUP BY S.TEST_INSERTION, R.HBIN_NO, R.SBIN_NO "
                    " ORDER BY FIRT_START_T";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }

            // Init the PhaseName with a impossible value for the first run
            lPhaseName = "#START A NEW PHASE#";
            mapPhaseMissingParts["FINAL"]=0;
            mapPhaseNbParts["FINAL"] = 0;
            mapPhaseNbPartsGood["FINAL"] = 0;
            mapPhaseHBin["FINAL"][lRetestMissingPartsBin] = 0;
            mapPhaseSBin["FINAL"][lRetestMissingPartsBin] = 0;

            while(clGexDbQuery.Next())
            {
                // For each new Phase
                // Check if need to add MISSING BIN PARTS
                if(lPhaseName != clGexDbQuery.value("TEST_INSERTION").toString())
                {
                    lPhaseName = clGexDbQuery.value("TEST_INSERTION").toString();
                    // Init
                    if(!mapPhaseNbParts.contains(lPhaseName))
                    {
                        mapPhaseNbParts[lPhaseName] = 0;
                        mapPhaseNbPartsGood[lPhaseName] = 0;
                    }

                    if( mapPhaseMissingParts.contains(lPhaseName))
                    {
                        mapPhaseHBin[lPhaseName][lRetestMissingPartsBin] = mapPhaseMissingParts[lPhaseName];
                        mapPhaseSBin[lPhaseName][lRetestMissingPartsBin] = mapPhaseMissingParts[lPhaseName];
                        mapPhaseNbParts[lPhaseName] += mapPhaseMissingParts[lPhaseName];

                        mapPhaseNbParts["FINAL"] += mapPhaseMissingParts[lPhaseName];
                        mapPhaseHBin["FINAL"][lRetestMissingPartsBin] += mapPhaseMissingParts[lPhaseName];
                        mapPhaseSBin["FINAL"][lRetestMissingPartsBin] += mapPhaseMissingParts[lPhaseName];
                    }
                }
                // Init
                if(!mapPhaseHBin[lPhaseName].contains(clGexDbQuery.value("HBIN_NO").toInt()))
                    mapPhaseHBin[lPhaseName][clGexDbQuery.value("HBIN_NO").toInt()] = 0;
                if(!mapPhaseSBin[lPhaseName].contains(clGexDbQuery.value("SBIN_NO").toInt()))
                    mapPhaseSBin[lPhaseName][clGexDbQuery.value("SBIN_NO").toInt()] = 0;

                mapPhaseHBin[lPhaseName][clGexDbQuery.value("HBIN_NO").toInt()] += clGexDbQuery.value("NB_PARTS").toInt();
                mapPhaseSBin[lPhaseName][clGexDbQuery.value("SBIN_NO").toInt()] += clGexDbQuery.value("NB_PARTS").toInt();
                mapPhaseNbParts[lPhaseName] += clGexDbQuery.value("NB_PARTS").toInt();
                mapPhaseNbPartsGood[lPhaseName] += clGexDbQuery.value("NB_PARTS_GOOD").toInt();
            }

            // For the FINAL RetestPhase
            // Extract Bin info from RUN CONSOLIDATION flags
            lQuery = "SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" R.HBIN_NO, R.SBIN_NO, count(*) AS NB_PARTS, sum(R.PART_STATUS='P') AS NB_PARTS_GOOD "
                     " FROM "+NormalizeTableName("_SPLITLOT")+ " S "
                     " INNER JOIN "+NormalizeTableName("_RUN")+" R "
                     "      ON S.SPLITLOT_ID=R.SPLITLOT_ID AND R.final=true "
                     " WHERE "
                     "      S.VALID_SPLITLOT<>'N'"
                    "       AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId)+
                    "       AND S.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId)+
                    "       AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData)+
                    " GROUP BY R.HBIN_NO, R.SBIN_NO";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }
            lPhaseName = "FINAL";
            while(clGexDbQuery.Next())
            {
                // Init
                if(!mapPhaseHBin[lPhaseName].contains(clGexDbQuery.value("HBIN_NO").toInt()))
                    mapPhaseHBin[lPhaseName][clGexDbQuery.value("HBIN_NO").toInt()] = 0;
                if(!mapPhaseSBin[lPhaseName].contains(clGexDbQuery.value("SBIN_NO").toInt()))
                    mapPhaseSBin[lPhaseName][clGexDbQuery.value("SBIN_NO").toInt()] = 0;

                mapPhaseHBin[lPhaseName][clGexDbQuery.value("HBIN_NO").toInt()] += clGexDbQuery.value("NB_PARTS").toInt();
                mapPhaseSBin[lPhaseName][clGexDbQuery.value("SBIN_NO").toInt()] += clGexDbQuery.value("NB_PARTS").toInt();
                mapPhaseNbParts[lPhaseName] += clGexDbQuery.value("NB_PARTS").toInt();
                mapPhaseNbPartsGood[lPhaseName] += clGexDbQuery.value("NB_PARTS_GOOD").toInt();
            }
        }

        TimersTraceStop("Function SubLot Binning Consolidation: consolidation phase");
    }

    // Init with false
    bStatus = false;


    TimersTraceStart("Function SubLot Binning Consolidation: consolidation trigger");


    // Physical Consolidation
    // Check if it is the last Phase
    // Then need to save the PHYSICAL consolidation
    // Create the last PHYSICAL Phase
    // Get the total of parts from the mapPhysicalHBin
    lPhaseName = "FINAL";
    if(!mapPhaseNbParts.contains(lPhaseName))
    {
        nNbParts = nNbPartsGood = 0;
        for(itBinCount=mapPhysicalHBin.begin(); itBinCount!=mapPhysicalHBin.end(); itBinCount++)
        {
            nNbParts += itBinCount.value();
            if(mapHBinInfo.contains(itBinCount.key())
                    && mapHBinInfo[itBinCount.key()].m_cBinCat == 'P')
                nNbPartsGood += itBinCount.value();
        }

        mapPhaseHBin[lPhaseName] = mapPhysicalHBin;
        mapPhaseNbParts[lPhaseName] = nNbParts;
        mapPhaseNbPartsGood[lPhaseName] = nNbPartsGood;
    }
    else
    {
        nNbParts = mapPhaseNbParts[lPhaseName];
        nNbPartsGood = mapPhaseNbPartsGood[lPhaseName];
    }


    if(lUpdateTableForPhysicalData)
    {
        // Phase Consolidation results are into mapHBin
        // update values for Parts and Runs
        lQuery = "UPDATE " + NormalizeTableName("_SUBLOT_INFO") + " SET ";
        lQuery += "NB_PARTS_GOOD="+QString::number(nNbPartsGood);
        lQuery += ",NB_PARTS="+QString::number(nNbParts);
        lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += " AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
        // execute query
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
    }

    TimersTraceStop("Function SubLot Binning Consolidation: consolidation trigger");

    TimersTraceStart("Function SubLot Binning Consolidation: consolidation update");

    for(itPhase = mapPhaseNbParts.begin(); itPhase != mapPhaseNbParts.end(); itPhase++)
    {
        lSuffixTable = "_INTER";
        lPhaseType = "INTERMEDIATE";
        lPhaseName = itPhase.key();
        nNbParts = mapPhaseNbParts[lPhaseName];
        nNbPartsGood = mapPhaseNbPartsGood[lPhaseName];
        mapHBin = mapPhaseHBin[lPhaseName];
        mapSBin = mapPhaseSBin[lPhaseName];

        if(lPhaseName == "FINAL")
        {
            if(lUpdateTableForPhysicalData)
                lSuffixTable = "";
            lPhaseType = "PHYSICAL";
            PhysicalConsolidationName = lPhaseName;
        }
        // For empty phase name, a PHYSICAL was also inserted
        if(lPhaseName.isEmpty())
            continue;

        // UPDATE PHYSICAL/INTERMEDIATE CONSOLIDATION
        TimersTraceStart("Function SubLot Consolidation: consolidation trigger "+lSuffixTable);
        lQuery = "INSERT INTO " + NormalizeTableName("_SUBLOT_CONSOLIDATION"+lSuffixTable);
        lQuery +=" VALUES( " + TranslateStringToSqlVarChar(LotId);
        lQuery +=", " + TranslateStringToSqlVarChar(SubLotId);
        lQuery +=", " + QString::number(nNbParts);
        lQuery +=", " + QString::number(nNbPartsGood);
        lQuery +=", " + TranslateStringToSqlVarChar(lPhaseType);
        lQuery +=", " + TranslateStringToSqlVarChar(lPhaseName);
        lQuery +=", " + TranslateStringToSqlVarChar(lTestFlow);
        lQuery +=")";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        TimersTraceStop("Function SubLot Consolidation: consolidation trigger "+lSuffixTable);

        //if(lUpdateTableForPhysicalData)
        {
            if(!mapHBin.isEmpty())
            {
                lQueryHeader = "INSERT INTO " + NormalizeTableName("_SUBLOT_HBIN"+lSuffixTable) + " VALUES";
                lNewValue = lAllValues = "";

                for(itBinInfo=mapHBinInfo.begin(); itBinInfo!=mapHBinInfo.end(); itBinInfo++)
                {
                    nBinNo = itBinInfo.key();

                    // ignore specific Bin -1 used to have global summary on all Bin
                    if(nBinNo == MERGE_BIN)
                        continue;

                    // ignore MissingPartsBin is empty
                    if((nBinNo == lRetestMissingPartsBin)
                            && !mapHBin.contains(nBinNo))
                        continue;

                    if(!mapHBin.contains(nBinNo))
                        mapHBin[nBinNo] = 0;

                    // insert new entry
                    lNewValue = "(";
                    // LOT_ID
                    lNewValue += TranslateStringToSqlVarChar(LotId);
                    lNewValue += ",";
                    // SUBLOT_ID
                    lNewValue += TranslateStringToSqlVarChar(SubLotId);
                    lNewValue += ",";
                    // HBIN_NO
                    lNewValue += QString::number(nBinNo);
                    lNewValue += ",";
                    // HBIN_NAME
                    lNewValue += TranslateStringToSqlVarChar(mapHBinInfo[nBinNo].m_strBinName);
                    lNewValue += ",";
                    // HBIN_CAT
                    lNewValue += mapHBinInfo[nBinNo].m_cBinCat == ' ' ? "null" : TranslateStringToSqlVarChar((QChar)mapHBinInfo[nBinNo].m_cBinCat);
                    lNewValue += ",";
                    // NB_PARTS
                    lNewValue += QString::number(mapHBin[nBinNo]);
                    if(!lSuffixTable.isEmpty())
                    {
                        // CONSOLIDATION_NAME
                        lNewValue += ", " + TranslateStringToSqlVarChar(lPhaseName);
                        // CONSOLIDATION_FLOW
                        lNewValue += ", " + TranslateStringToSqlVarChar(lTestFlow);
                    }
                    lNewValue += ")";

                    if(!AddInMultiInsertQuery(NormalizeTableName("_SUBLOT_HBIN"+lSuffixTable), lQueryHeader, lNewValue, lAllValues))
                        goto labelUnlock;
                }

                if(!lAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_SUBLOT_HBIN"+lSuffixTable), lAllValues)))
                    goto labelUnlock;
            }

            if(!mapSBin.isEmpty())
            {
                lQueryHeader = "INSERT INTO " + NormalizeTableName("_SUBLOT_SBIN"+lSuffixTable) + " VALUES";
                lNewValue = lAllValues = "";

                for(itBinInfo=mapSBinInfo.begin(); itBinInfo!=mapSBinInfo.end(); itBinInfo++)
                {
                    nBinNo = itBinInfo.key();

                    // ignore specific Bin -1 used to have global summary on all Bin
                    if(nBinNo == MERGE_BIN)
                        continue;

                    // ignore MissingPartsBin is empty
                    if((nBinNo == lRetestMissingPartsBin)
                            && !mapHBin.contains(nBinNo))
                        continue;

                    if(!mapSBin.contains(nBinNo))
                        mapSBin[nBinNo] = 0;

                    // insert new entry
                    lNewValue = "(";
                    // LOT_ID
                    lNewValue += TranslateStringToSqlVarChar(LotId);
                    lNewValue += ",";
                    // SUBLOT_ID
                    lNewValue += TranslateStringToSqlVarChar(SubLotId);
                    lNewValue += ",";
                    // HBIN_NO
                    lNewValue += QString::number(nBinNo);
                    lNewValue += ",";
                    // HBIN_NAME
                    lNewValue += TranslateStringToSqlVarChar(mapSBinInfo[nBinNo].m_strBinName);
                    lNewValue += ",";
                    // HBIN_CAT
                    lNewValue += mapSBinInfo[nBinNo].m_cBinCat == ' ' ? "null" : TranslateStringToSqlVarChar((QChar)mapSBinInfo[nBinNo].m_cBinCat);
                    lNewValue += ",";
                    // NB_PARTS
                    lNewValue += QString::number(mapSBin[nBinNo]);
                    if(!lSuffixTable.isEmpty())
                    {
                        // CONSOLIDATION_NAME
                        lNewValue += ", " + TranslateStringToSqlVarChar(lPhaseName);
                        // CONSOLIDATION_FLOW
                        lNewValue += ", " + TranslateStringToSqlVarChar(lTestFlow);
                    }
                    lNewValue += ")";

                    if(!AddInMultiInsertQuery(NormalizeTableName("_SUBLOT_SBIN"+lSuffixTable), lQueryHeader, lNewValue, lAllValues))
                        goto labelUnlock;
                }

                if(!lAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_SUBLOT_SBIN"+lSuffixTable), lAllValues)))
                    goto labelUnlock;
            }
        }
    }

    // To disabled the Gap Lock on xx_splitlot
    // we need to work on the PRIMARY KEY
    lQuery = "SELECT splitlot_id FROM "+NormalizeTableName("_SPLITLOT")+" S ";
    lQuery += " WHERE ";
    lQuery += "   VALID_SPLITLOT<>'N'";
    lQuery += "   AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "   AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
    lQuery += "   AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    while(clGexDbQuery.Next())
    {
        lSplitlots += clGexDbQuery.value("splitlot_id").toString();
    }

    // Update the test_insertion_index for all phases
    lQuery = "UPDATE "+NormalizeTableName("_SPLITLOT")+" S ";
    lQuery+= " SET S.TEST_INSERTION_INDEX=";
    lQuery+= "      IF(FIND_IN_SET(S.TEST_INSERTION,"+TranslateStringToSqlVarChar(lstPhases.join(","))+")=0,";
    lQuery+= "          null,";
    lQuery+= "          FIND_IN_SET(S.TEST_INSERTION,"+TranslateStringToSqlVarChar(lstPhases.join(","))+")-1)";
    lQuery += " WHERE ";
    lQuery += "   SPLITLOT_ID IN ("+lSplitlots.join(",")+")";
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }


    TimersTraceStop("Function SubLot Binning Consolidation: consolidation update");

    // VALID CONSOLIDATION
    bStatus = true;

    // Debug message
    lMessage = "     GexDbPlugin_Galaxy::UpdateSubLotConsolidation() : Consolidation completed";
    WriteDebugMessageFile(lMessage);

labelUnlock:

    TimersTraceStop("Function SubLot Binning Consolidation");
    TimersTraceStop("Function SubLot Binning Consolidation: consolidation phase");
    TimersTraceStop("Function SubLot Binning Consolidation: consolidation trigger");
    TimersTraceStop("Function SubLot Binning Consolidation: consolidation update");

    // Commit transaction before to update INCREMENTAL_UPDATE
    clGexDbQuery.Execute("COMMIT");

    if(!bStatus)
    {
        if(m_strlWarnings.isEmpty())
        {
            QString lError = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            WarningMessage(lError);
        }

        ConsolidationSummary = m_strlWarnings.takeLast();
    }

    // Remove the BINNING FLAGS
    return bStatus;
}


///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateSubLotConsolidation
// For consolidation phase
///////////////////////////////////////////////////////////
////// \brief GexDbPlugin_Galaxy::UpdateSubLotConsolidation
/// Update the FT_SUBLOT_INFO with the result of the Flexible Consolidation
////// \param LotId
////// \param SubLotId
////// \return
bool GexDbPlugin_Galaxy::UpdateSubLotConsolidation(const QString &LotId, const QString &SubLotId)
{
    if(m_eTestingStage != eFinalTest)
        return true;

    // Clear Warning messages
    m_nInsertionSID = m_pclDatabaseConnector->GetConnectionSID();
    m_strlWarnings.clear();

    QString lMessage = "Update SubLot Consolidation : Consolidation process for LotId = " + LotId;
    lMessage += "and SubLotId = "+SubLotId;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data() );

    QString lTableName = NormalizeTableName("_SUBLOT_INFO");

    QString             lQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);

    ///////////////////////////////////////////////////////////
    // SubLot Binning Consolidation
    ///////////////////////////////////////////////////////////
    int     nNbParts=0;
    int     nNbPartsGood=0;

    bool bStatus = true;

    QString                lConsolidationSummary;
    QString                lProdData;
    QString                lPhysicalConsolidationName;
    int                    lNumberProdData;
    QStringList            lProdDatas;
    QStringList            lProdDatasForConsolidation;
    QMap<QString,QStringList> lTestFlowProdDatas;
    bool                   lUpdateTableForPhysicalData = false;
    int                    lNumOfRows = 0;
    QStringList            lMatchedTestFlows;

    TimersTraceStart("Function SubLot Consolidation");

    /////////////////////////
    // DEADLOCK PREVENTION
    // Clean the Token table if needed
    if (! InitTokens(NormalizeTableName("_CONSOLIDATION")))
    {
        return false;
    }
    // Consolidation through Incremental Update
    // Fix collision on same Lot
    QString lTokenKey = LotId+"|"+SubLotId;
    if(!GetToken(NormalizeTableName("_CONSOLIDATION"),lTokenKey,15))
    {
        // Reject the current consolidation
        bStatus = false;
        goto labelUnlock;
    }

    ///////////////////////////////////////////////////////////
    // CHECK IF AT LEAST ONE SPLITLOT WAS FLAGED WITH BINNING_CONSOLIDATION
    ///////////////////////////////////////////////////////////
    // PROD_DATA INFO
    // PROD_DATA, START_TIME, SAMPLES_FLAGS
    // FROM SPLITLOT(LOT/WAFER)
    ///////////////////////////////////////////////////////////
    // FOR ALL TEST CONDITIONS
    // Upate lot information with data
    // from production data if have
    // else from non-production data

    // Construct the query condition to select only wafer with data samples
    lNumberProdData = 0;

    // Check if have to use SUMMARY
    // Check if have
    lQuery = "SELECT PROD_DATA,TEST_FLOW, ";
    lQuery += " MAX(INCREMENTAL_UPDATE LIKE '%BINNING_CONSOLIDATION%') AS FOR_CONSOLIDATION ";
    lQuery += " FROM "+NormalizeTableName("_SPLITLOT");
    lQuery += " WHERE ";
    lQuery += "   VALID_SPLITLOT<>'N'";
    lQuery += "   AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "   AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
    lQuery += " GROUP BY PROD_DATA,TEST_FLOW ";
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    while(clGexDbQuery.Next())
    {
        ++lNumOfRows;

        // No consolidation for TEST_FLOW empty
        if(clGexDbQuery.value("TEST_FLOW").toString().isEmpty())
            continue;

        lNumberProdData++;

        lTestFlowProdDatas[clGexDbQuery.value("TEST_FLOW").toString()] += clGexDbQuery.value("PROD_DATA").toString();
        lProdDatas += clGexDbQuery.value("PROD_DATA").toString();
        // Check if have several test_flow for the prod data Y
        if(lProdDatas.count("Y") > 1)
        {
            // This feature is not already implemented
            GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported,NULL,
                        QString("The LOT[%1] SUBLOT[%2] contains several TEST_FLOW for the PROD_DATA=Y. This feature is not supported")
                        .arg(LotId).arg(SubLotId).toLatin1().data());
            bStatus = false;
            goto labelUnlock;
        }
        // Check if have the same test_flow for several prod data (Y and N)
        if(lTestFlowProdDatas[clGexDbQuery.value("TEST_FLOW").toString()].size() > 1)
        {
            // This feature is not supported
            GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported,NULL,
                        QString("The LOT[%1] SUBLOT[%2] contains the same TEST_FLOW for PROD_DATA=Y and PROD_DATA=N. This feature is not supported")
                        .arg(LotId).arg(SubLotId).toLatin1().data());
            bStatus = false;
            goto labelUnlock;
        }

        if(clGexDbQuery.value("FOR_CONSOLIDATION").toInt() == 1)
        {
            lProdDatasForConsolidation += clGexDbQuery.value("PROD_DATA").toString();
        }
    }

    if(lProdDatasForConsolidation.isEmpty())
    {
        // NO SPLITLOT FLAGGED WITH BINNING_CONSOLIDATON
        // Do nothing
        QString lMsg;
        if(lNumOfRows == 0)
            lMsg = QString("No VAILD splitlots for Lot[%1]SubLot[%2] - No consolidation needed")
                    .arg(LotId).arg(SubLotId);
        else if(lNumberProdData == 0)
            lMsg = QString("No TEST_FLOW for Lot[%1]SubLot[%2] - Consolidation disabled")
                    .arg(LotId).arg(SubLotId);
        else
            lMsg = QString("No BINNING_CONSOLIDATION flag for Lot[%1]SubLot[%2] - No consolidation needed")
                    .arg(LotId).arg(SubLotId);
        WarningMessage(lMsg);

        bStatus = true;
        goto labelUnlock;
    }

    // If 2 PROD_DATA for consolidation
    // We need to do the 2 consolidation in one pass
    // After the update, the incremental update process will remove the BINNING_CONSOLIDATION flag for ALL
    if(lProdDatasForConsolidation.contains("Y"))
        lUpdateTableForPhysicalData = true;

    ///////////////////////////////////////
    // START CONSOLIDATION NOW
    ///////////////////////////////////////
    // RESET CONSOLIDATION COUNT
    // For each PROD_DATA for CONSOLIDATION
    if(lUpdateTableForPhysicalData || (lNumberProdData==1))
    {
        // RESET the PHYSICAL CONSOLIDATION STATUS
        lQuery = "UPDATE "+lTableName;
        lQuery+= " SET ";
        lQuery+= " NB_PARTS=0,NB_PARTS_GOOD=0";
        lQuery+= " ,CONSOLIDATION_STATUS=null";
        lQuery+= " ,CONSOLIDATION_SUMMARY='IN PROGESS'";
        lQuery+= " ,CONSOLIDATION_DATE=now()";
        lQuery+= " WHERE ";
        lQuery+= "  LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery+= "  AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        clGexDbQuery.Execute("COMMIT");

        ////////////////////////////////////////////
        // DELETE ALL PHYSICAL CONSOLIDATED DATA
        lQuery = "DELETE FROM %1";
        lQuery += " WHERE ";
        lQuery += "     LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
        if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_SUBLOT_HBIN"))))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
        if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_SUBLOT_SBIN"))))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        ///////////////////////////////////////
        // RESET CONSOLIDATION COUNT FOR LOT LEVEL
        lQuery = "UPDATE "+ NormalizeTableName("_LOT");
        lQuery+= " SET NB_PARTS=0,NB_PARTS_GOOD=0";
        lQuery+= " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

    }

    // To disabled the Gap Lock on xx_splitlot
    // we need to work on the PRIMARY KEY
    // not possible in this case
    lQuery = "SELECT distinct S.TEST_FLOW FROM "+ NormalizeTableName("_SPLITLOT") +" S ";
    lQuery += " WHERE ";
    lQuery += "     S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "     AND S.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
    // Reset all PROD_DATA if only ONE PROD_DATA
    if(lNumberProdData > 1 && (lProdDatasForConsolidation.size() == 1))
        lQuery += " AND S.PROD_DATA="+TranslateStringToSqlVarChar(lProdDatasForConsolidation.first());
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    while(clGexDbQuery.Next())
    {
        lMatchedTestFlows += clGexDbQuery.value("TEST_FLOW").toString();
    }

    // DELETE ALL INTERMEDIATE CONSOLIDATED DATA
    lQuery = "DELETE FROM %1 ";
    lQuery += " WHERE ";
    lQuery += "     LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "     AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
    lQuery += "     AND CONSOLIDATION_FLOW IN ('"+lMatchedTestFlows.join("','")+"')";
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_SUBLOT_HBIN_INTER"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_SUBLOT_SBIN_INTER"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_SUBLOT_CONSOLIDATION"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_SUBLOT_CONSOLIDATION_INTER"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }


    // Init with false
    bStatus = false;

    // List of Retest bins
    // With this information, we can reset all HBin/SBin from RetestIndex-1 and add new HBin/SBin from RetestIndex
    nNbParts=0;
    nNbPartsGood=0;

    ///////////////////////////////////////
    // UPDATE THE RUN_CONSOLIDATON TABLE
    // For all cases
    //  * Consolidation from samples
    //  * Consolidation from summary
    // For each PROD_DATA
    // Create the RunConsolidation
    // And Compute the Binning Consolidation
    // Then extract the new PHYSICAL consolidation from WAFER_HBIN table
    // For the update of the WAFER_INFO table
    for(int i=0; i< lProdDatasForConsolidation.size(); ++i)
    {
        lProdData = lProdDatasForConsolidation.at(i);
        // Will fill the RUN CONSOLIDATION flags for intermediate and final
        // Will fill the FT_SUBLOT_HBIN for PROD_DATA='Y'
        // Will fill the FT_SUBLOT_CONSOLIDATION for all PROD_DATA
        if(!UpdateSubLotBinningConsolidationTable(LotId, SubLotId,lProdData, lPhysicalConsolidationName, lConsolidationSummary))
            goto labelUnlock;
        if(lPhysicalConsolidationName.isEmpty())
            goto labelUnlock;
    }

    if(lUpdateTableForPhysicalData)
    {
        // New PHYSICAL CONSOLIDATION
        // All BIN tables and CONSOLIDATION tables were updated
        // Retreive the good count from this tables
        lQuery = "SELECT ";
        lQuery+= "      SUM(NB_PARTS) AS NB_PARTS, ";
        lQuery+= "      SUM(IF((HBIN_CAT='P'),NB_PARTS,0)) AS NB_PARTS_GOOD ";
        lQuery+= " FROM "+NormalizeTableName("_SUBLOT_HBIN");
        lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
        if(!clGexDbQuery.Execute(lQuery) || !clGexDbQuery.First())
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        nNbParts = clGexDbQuery.value("NB_PARTS").toInt();
        nNbPartsGood = clGexDbQuery.value("NB_PARTS_GOOD").toInt();

        // Update NB_PARTS, NB_PARTS_GOOD
        lQuery =  "UPDATE "+lTableName;
        lQuery += "        SET NB_PARTS="+QString::number(nNbParts);
        lQuery += "        ,   NB_PARTS_GOOD="+QString::number(nNbPartsGood);
        lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
    }
    // VALID CONSOLIDATION

    bStatus = true;

    // Debug message
    lMessage = "     GexDbPlugin_Galaxy::UpdateSubLotConsolidation() : Consolidation completed";
    WriteDebugMessageFile(lMessage);

labelUnlock:

    TimersTraceStop("Function SubLot Consolidation");

    // Commit transaction before to update INCREMENTAL_UPDATE
    /////////////////////////
    // DEADLOCK PREVENTION
    // Commit transaction
    clGexDbQuery.Execute("COMMIT");

    // Display or Propagate the Error message
    if(!bStatus)
    {
        if(lConsolidationSummary.isEmpty())
        {
            lConsolidationSummary = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        }
        if(!lConsolidationSummary.isEmpty())
        {
            WarningMessage(lConsolidationSummary);
        }
    }
    if(!m_strlWarnings.isEmpty()
            && lConsolidationSummary.isEmpty())
    {
        lConsolidationSummary = m_strlWarnings.last();
    }

    QString lStatus = "'F'";
    if(bStatus)
    {
        lStatus = "'P'";
    }
    QString lSummary = TranslateStringToSqlVarChar(lConsolidationSummary);
    if(lConsolidationSummary.isEmpty())
    {
        lSummary = "null";
    }
    lQuery =  "UPDATE "+lTableName;
    lQuery += "    SET CONSOLIDATION_STATUS="+lStatus;
    lQuery+= "      ,CONSOLIDATION_SUMMARY="+lSummary;
    lQuery += "     ,CONSOLIDATION_DATE=now() ";
    lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "     AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        bStatus = false;
    }

    // Release TOKEN on WT_CONSOLIDATION keys
    ReleaseTokens();

    return bStatus;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateLotConsolidation
// For consolidation phase
///////////////////////////////////////////////////////////
////// \brief GexDbPlugin_Galaxy::UpdateLotConsolidation
/// Call AFTER the SUBLOT/WAFER consolidation
////// \param lotId
////// \return
bool GexDbPlugin_Galaxy::UpdateLotConsolidation(const QString &LotId)
{
    QString lMessage = "Update Lot Consolidation : Consolidation process for LotId = " + LotId;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().data() );

    QString lTableName;

    lTableName = NormalizeTableName("_LOT");

    TimersTraceStart("Function Lot Consolidation");

    QString             lQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);

    ///////////////////////////////////////////////////////////
    // Lot Binning Consolidation
    ///////////////////////////////////////////////////////////

    QString lQueryHeader;      // INSERT INTO ...
    QString lNewValue;         // NEW VALUE FOR INSERTION
    QString lAllValues;        // MULTI QUERIES

    // For Product consolidation
    QString lProdData;
    QMap<int,structBinInfo> mapHBinInfo;
    QMap<int,structBinInfo> mapSBinInfo;
    int     nBinNo;
    QString lBinCat;
    QStringList lProductNames;
    bool bStatus = true;

    lQuery = "SELECT TRACKING_LOT_ID, PRODUCT_NAME FROM "+lTableName;
    lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);

    /////////////////////////
    // DEADLOCK PREVENTION
    // Clean the Token table if needed
    if (! InitTokens(NormalizeTableName("_CONSOLIDATION")))
    {
        return false;
    }
    // Consolidation through Incremental Update
    // Fix collision on same Lot
    QString lTokenKey = LotId;
    if(!GetToken(NormalizeTableName("_CONSOLIDATION"),lTokenKey,15))
    {
        // Reject the current consolidation
        bStatus = false;
        goto labelUnlock;
    }

    // Init with false
    bStatus = false;

    ///////////////////////////////////////////////////////////
    // PRODUCT INFO
    ///////////////////////////////////////////////////////////
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSLOG(SYSLOG_SEV_NOTICE, (QString("QUERY=%1 STATUS=%2").arg(
                                      clGexDbQuery.lastQuery().toLatin1().constData())
                                  .arg(clGexDbQuery.lastError().text())).toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    if(!clGexDbQuery.First())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Update lot consolidation : no result from : %1")
              .arg( lQuery).toLatin1().constData());
        GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, lQuery.toLatin1().constData());
        goto labelUnlock;
    }

    ///////////////////////////////////////////////////////////
    // CHECK IF AT LEAST ONE SPLITLOT WAS FLAGED WITH BINNING_CONSOLIDATION
    lQuery = "SELECT count(*), MAX(PROD_DATA)";
    lQuery += " FROM "+NormalizeTableName("_SPLITLOT");
    lQuery += " WHERE ";
    lQuery += "   LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "   AND VALID_SPLITLOT<>'N'";
    lQuery += "   AND INCREMENTAL_UPDATE LIKE '%BINNING_CONSOLIDATION%'";
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSLOG(SYSLOG_SEV_NOTICE, (QString("QUERY=%1 STATUS=%2")
                                  .arg(clGexDbQuery.lastQuery().toLatin1().constData())
                                  .arg(clGexDbQuery.lastError().text())).toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    clGexDbQuery.First();
    if(clGexDbQuery.value(0).toInt() == 0)
    {
        // NO SPLITLOT FLAGGED WITH BINNING_CONSOLIDATON
        // Do nothing
        GSLOG(SYSLOG_SEV_NOTICE, "NO SPLITLOT FLAGGED WITH BINNING_CONSOLIDATON. Nothing to be done.");
        bStatus = true;
        goto labelUnlock;
    }
    lProdData = clGexDbQuery.value(1).toString();

    // GCORE-2501 Multi-Products
    // xT_LOT table can contains more than one line per LOT_ID
    // for Multi-Products
    // lot_id, trackinglot_id, product_name, nb_parts, ...
    // LOT_AA, TRACKINGLOT_AA, PRODUCT_AAAA, 100
    // LOT_AA, TRACKINGLOT_AA, PRODUCT_BBBB, 50
    // xT_SUBLOT_INFO table can contains more than one line per LOT_ID
    // lot_id, sublot_id, product_name, nb_parts, ...
    // LOT_AA, SUBLOT_11, PRODUCT_AAAA, 100
    // LOT_AA, SUBLOT_22, PRODUCT_BBBB, 50

    // DB-67 - Overwrite alrady inserted WaferId+BADProduct
    // Do the verification directly on the SPLITLOT table as the PRODUCT_NAME is in the table now
    // For each PRODUCT_NAME from xT_SUBLOT_INFO, need to have one line into xT_LOT
    lQuery = "SELECT distinct PRODUCT_NAME ";
    lQuery+= " FROM " + NormalizeTableName("_SPLITLOT");
    lQuery+= " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += " AND VALID_SPLITLOT<>'N'";
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    while(clGexDbQuery.Next())
    {
        lProductNames << TranslateStringToSqlVarChar(clGexDbQuery.value("PRODUCT_NAME").toString());
    }

    if(lProductNames.isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Update lot consolidation : no result from : %1")
              .arg( lQuery).toLatin1().constData());
        GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, lQuery.toLatin1().constData());
        goto labelUnlock;
    }

    // Clean INVALID ProductName
    lQuery = "DELETE FROM %1";
    lQuery+= " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery+= " AND PRODUCT_NAME NOT IN ("+lProductNames.join(",")+")";
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_LOT"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_LOT_HBIN"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_LOT_SBIN"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    while(!lProductNames.isEmpty())
    {
        lQuery = "SELECT PRODUCT_NAME ";
        lQuery+= " FROM "+NormalizeTableName("_LOT");
        lQuery+= " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery+= " AND PRODUCT_NAME="+lProductNames.takeFirst();
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
        if(!clGexDbQuery.First())
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Update lot consolidation : no result from : %1")
                  .arg( lQuery).toLatin1().constData());
            GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, lQuery.toLatin1().constData());
            goto labelUnlock;
        }
    }


    ///////////////////////////////////////
    // START CONSOLIDATION NOW
    ///////////////////////////////////////
    // RESET CONSOLIDATION COUNT
    lQuery = "UPDATE "+lTableName;
    lQuery+= " SET NB_PARTS=0,NB_PARTS_GOOD=0";
    lQuery+= " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    // Reset LOT_HBIN table
    lQuery =  "DELETE FROM " + NormalizeTableName("_LOT_HBIN");
    lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    // Reset LOT_SBIN table
    lQuery =  "DELETE FROM " + NormalizeTableName("_LOT_SBIN");
    lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    /////////////////////////
    // DEADLOCK PREVENTION
    // Commit transaction
    clGexDbQuery.Execute("COMMIT");

    // Init with false
    bStatus = false;

    TimersTraceStart("Function Lot Consolidation: consolidation trigger");

    // update values for Parts and Runs
    // NB_PARTS and NB_PARTS_GOOD from WAFER_INFO after wafer consolidation
    // NB_PARTS and NB_PARTS_GOOD from SUBLOT_INFO after SubLot consolidation
    lQuery = "UPDATE "+NormalizeTableName("_LOT")+" L ";
    if(m_eTestingStage == eFinalTest)
    {
        lQuery+= " SET L.NB_PARTS=(SELECT SUM(S.NB_PARTS) FROM "+NormalizeTableName("_SUBLOT_INFO")+" S WHERE S.LOT_ID=L.LOT_ID AND S.PRODUCT_NAME=L.PRODUCT_NAME) ";
        lQuery+= " ,L.NB_PARTS_GOOD=(SELECT SUM(S.NB_PARTS_GOOD) FROM "+NormalizeTableName("_SUBLOT_INFO")+" S WHERE S.LOT_ID=L.LOT_ID AND S.PRODUCT_NAME=L.PRODUCT_NAME) ";
    }
    else
    {
        lQuery+= " SET L.NB_PARTS=(SELECT SUM(S.NB_PARTS) FROM "+NormalizeTableName("_WAFER_INFO")+" S WHERE S.LOT_ID=L.LOT_ID AND S.PRODUCT_NAME=L.PRODUCT_NAME) ";
        lQuery+= " ,L.NB_PARTS_GOOD=(SELECT SUM(S.NB_PARTS_GOOD) FROM "+NormalizeTableName("_WAFER_INFO")+" S WHERE S.LOT_ID=L.LOT_ID AND S.PRODUCT_NAME=L.PRODUCT_NAME) ";
    }
    lQuery+= " WHERE L.LOT_ID="+TranslateStringToSqlVarChar(LotId);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSLOG(SYSLOG_SEV_NOTICE, (QString("QUERY=%1 STATUS=%2")
                                  .arg(clGexDbQuery.lastQuery().toLatin1().constData())
                                  .arg(clGexDbQuery.lastError().text())).toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    TimersTraceStop("Function Lot Consolidation: consolidation trigger");

    TimersTraceStart("Function Lot Consolidation: consolidation update");

    ///////////////////////////////////////////////////////////
    // Lot Binning Consolidation
    ///////////////////////////////////////////////////////////
    // COLLECT HBIN NUM,NAME,CAT FROM PREVIOUS INSERTION
    // COLLECT SBIN NUM,NAME,CAT FROM PREVIOUS INSERTION
    ///////////////////////////////////////////////////////////
    // Update mapHbinInfo and mapSBinInfo with previous info
    // For HBIN
    // FROM SPLITLOT/LOT/PROD_DATA
    lQuery =  " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" H.HBIN_NO, ";
    lQuery += " SUBSTRING_INDEX(MAX(CONCAT(F.START_T,'#:#',H.HBIN_CAT)),'#:#',-1) AS HBIN_CAT, ";
    lQuery += " SUBSTRING_INDEX(MAX(CONCAT(F.START_T,'#:#',H.HBIN_NAME)),'#:#',-1) AS HBIN_NAME ";
    lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" F ";
    lQuery += " INNER JOIN "+NormalizeTableName("_SUBLOT_HBIN")+" H ";
    lQuery += "     ON F.LOT_ID=H.LOT_ID AND F.SUBLOT_ID=H.SUBLOT_ID";
    lQuery += " WHERE ";
    lQuery += "     F.VALID_SPLITLOT<>'N'";
    lQuery += "     AND F.LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "     AND F.PROD_DATA="+TranslateStringToSqlVarChar(lProdData);
    lQuery += " GROUP BY H.HBIN_NO";
    if(m_eTestingStage != eFinalTest)
    {
        // use the WAFER tables instead of SUBLOT tables
        // use the WAFER id instead of SUBLOT id
        lQuery = lQuery.replace("SUBLOT","WAFER",Qt::CaseSensitive);
        lQuery = lQuery.replace("sublot","wafer",Qt::CaseSensitive);
    }

    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    while(clGexDbQuery.Next())
    {
        // update values for Bin info
        nBinNo = clGexDbQuery.value(0).toInt();
        lBinCat = clGexDbQuery.value(1).toString().toUpper();
        mapHBinInfo[nBinNo].m_nBinNum = nBinNo;
        // Keep the last definition according to the start_t
        if(mapHBinInfo[nBinNo].m_strBinName.isEmpty())
            mapHBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(2).toString();

        if(lBinCat=="P")
            mapHBinInfo[nBinNo].m_cBinCat = 'P';
        else if(lBinCat=="A")
            mapHBinInfo[nBinNo].m_cBinCat = 'A';
        else
            mapHBinInfo[nBinNo].m_cBinCat = 'F';
    }

    // For SBIN
    // use the SBIN tables instead of HBIN tables
    // use the SBIN id instead of HBIN id
    lQuery = lQuery.replace("HBIN","SBIN",Qt::CaseSensitive);
    lQuery = lQuery.replace("hbin","sbin",Qt::CaseSensitive);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    while(clGexDbQuery.Next())
    {
        // update values for Bin info
        nBinNo = clGexDbQuery.value(0).toInt();
        lBinCat = clGexDbQuery.value(1).toString().toUpper();
        mapSBinInfo[nBinNo].m_nBinNum = nBinNo;
        // Keep the last definition according to the start_t
        if(mapSBinInfo[nBinNo].m_strBinName.isEmpty())
            mapSBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(2).toString();
        if(lBinCat=="P")
            mapSBinInfo[nBinNo].m_cBinCat = 'P';
        else if(lBinCat=="A")
            mapSBinInfo[nBinNo].m_cBinCat = 'A';
        else
            mapSBinInfo[nBinNo].m_cBinCat = 'F';
    }

    lQuery =  " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" L.PRODUCT_NAME,";
    lQuery += "    B.HBIN_NO, ";
    lQuery += "    SUM(B.NB_PARTS) AS BIN_COUNT ,";
    lQuery += "    MAX(B.HBIN_CAT),";
    lQuery += "    MAX(B.HBIN_NAME)";
    lQuery += " FROM "+NormalizeTableName("_SUBLOT_INFO") + " L ";
    lQuery += " INNER JOIN "+NormalizeTableName("_SUBLOT_HBIN") + " B ";
    lQuery += "     ON B.LOT_ID=L.LOT_ID AND B.SUBLOT_ID=L.SUBLOT_ID";
    lQuery += " WHERE ";
    lQuery += "     B.LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += " GROUP BY L.PRODUCT_NAME, B.HBIN_NO";
    // for each HBIN
    // Update HBin count
    if(m_eTestingStage != eFinalTest)
    {
        // use the WAFER tables instead of SUBLOT tables
        // use the WAFER id instead of SUBLOT id
        lQuery = lQuery.replace("SUBLOT","WAFER",Qt::CaseSensitive);
        lQuery = lQuery.replace("sublot","wafer",Qt::CaseSensitive);
    }

    if(!clGexDbQuery.Execute(lQuery))
    {
        GSLOG(SYSLOG_SEV_NOTICE, (QString("QUERY=%1 STATUS=%2")
                                  .arg(clGexDbQuery.lastQuery().toLatin1().constData())
                                  .arg(clGexDbQuery.lastError().text())).toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    lQueryHeader = "INSERT INTO " + NormalizeTableName("_LOT_HBIN") + " VALUES";
    lNewValue = lAllValues = "";
    while(clGexDbQuery.Next())
    {
        nBinNo = clGexDbQuery.value(1).toInt();
        if(!mapHBinInfo.contains(nBinNo))
        {
            // If Initial Test doesn't contains this BinNo
            // just create it
            lBinCat = clGexDbQuery.value(3).toString().toUpper();
            if(lBinCat=="P")
                mapHBinInfo[nBinNo].m_cBinCat = 'P';
            else if(lBinCat=="A")
                mapHBinInfo[nBinNo].m_cBinCat = 'A';
            else
                mapHBinInfo[nBinNo].m_cBinCat = 'F';
            mapHBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(4).toString();
        }

        // insert new entry
        lNewValue  = "(";
        // LOT_ID
        lNewValue += TranslateStringToSqlVarChar(LotId);
        lNewValue += ",";
        // PRODUCT_NAME
        lNewValue += TranslateStringToSqlVarChar(clGexDbQuery.value(0).toString());
        lNewValue += ",";
        // HBIN_NO
        lNewValue += QString::number(nBinNo);
        lNewValue += ",";
        // HBIN_NAME
        lNewValue += TranslateStringToSqlVarChar(mapHBinInfo[nBinNo].m_strBinName);
        lNewValue += ",";
        // HBIN_CAT
        lNewValue += TranslateStringToSqlVarChar((QChar)mapHBinInfo[nBinNo].m_cBinCat);
        lNewValue += ",";
        // NB_PARTS
        lNewValue += QString::number(clGexDbQuery.value(2).toInt());
        lNewValue += ")";

        if(!AddInMultiInsertQuery(NormalizeTableName("_LOT_HBIN"), lQueryHeader, lNewValue, lAllValues))
            goto labelUnlock;
    }
    if(!lAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_LOT_HBIN"), lAllValues)))
        goto labelUnlock;

    // for each SBIN
    // Update SBin count
    // use SBIN instead of HBIN
    lQuery = lQuery.replace("HBIN","SBIN",Qt::CaseSensitive);
    lQuery = lQuery.replace("hbin","sbin",Qt::CaseSensitive);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSLOG(SYSLOG_SEV_NOTICE, (QString("QUERY=%1 STATUS=%2")
                                  .arg(clGexDbQuery.lastQuery().toLatin1().constData())
                                  .arg(clGexDbQuery.lastError().text())).toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    lQueryHeader = "INSERT INTO " + NormalizeTableName("_LOT_SBIN") + " VALUES";
    lNewValue = lAllValues = "";
    while(clGexDbQuery.Next())
    {
        nBinNo = clGexDbQuery.value(1).toInt();
        if(!mapSBinInfo.contains(nBinNo))
        {
            // If Initial Test doesn't contains this BinNo
            // just create it
            lBinCat = clGexDbQuery.value(3).toString().toUpper();
            if(lBinCat=="P")
                mapSBinInfo[nBinNo].m_cBinCat = 'P';
            else if(lBinCat=="A")
                mapSBinInfo[nBinNo].m_cBinCat = 'A';
            else
                mapSBinInfo[nBinNo].m_cBinCat = 'F';
            mapSBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(4).toString();
        }

        // insert new entry
        lNewValue  = "(";
        // LOT_ID
        lNewValue += TranslateStringToSqlVarChar(LotId);
        lNewValue += ",";
        // PRODUCT_NAME
        lNewValue += TranslateStringToSqlVarChar(clGexDbQuery.value(0).toString());
        lNewValue += ",";
        // SBIN_NO
        lNewValue += QString::number(nBinNo);
        lNewValue += ",";
        // SBIN_NAME
        lNewValue += TranslateStringToSqlVarChar(mapSBinInfo[nBinNo].m_strBinName);
        lNewValue += ",";
        // SBIN_CAT
        lNewValue += TranslateStringToSqlVarChar((QChar)mapSBinInfo[nBinNo].m_cBinCat);
        lNewValue += ",";
        // NB_PARTS
        lNewValue += QString::number(clGexDbQuery.value(2).toInt());
        lNewValue += ")";

        if(!AddInMultiInsertQuery(NormalizeTableName("_LOT_SBIN"), lQueryHeader, lNewValue, lAllValues))
            goto labelUnlock;
    }
    if(!lAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_LOT_SBIN"), lAllValues)))
        goto labelUnlock;


    TimersTraceStop("Function Lot Consolidation: consolidation update");

    // VALID CONSOLIDATION

    // END OF THE LOT CONSOLIDATION
    // MULTI INSERTION MULTI SQL_THREAD
    // Unlock and commit all lot tables
    clGexDbQuery.Execute("COMMIT");

    ///////////////////////////////////////////////////////////
    // Product Binning Consolidation
    ///////////////////////////////////////////////////////////

    TimersTraceStart("Function Lot Consolidation: product update");

    // For each PRODUCT_NAME from xT_LOT, update the product table
    lQuery = "SELECT distinct PRODUCT_NAME ";
    if(m_eTestingStage == eFinalTest)
        lQuery+= " FROM " + NormalizeTableName("_SUBLOT_INFO");
    else
        lQuery+= " FROM " + NormalizeTableName("_WAFER_INFO");
    lQuery+= " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    while(clGexDbQuery.Next())
    {
        lNewValue = clGexDbQuery.value(0).toString();
        if(!UpdateProductConsolidation(lNewValue))
            goto labelUnlock;
    }
    TimersTraceStop("Function Lot Consolidation: product update");

    bStatus = true;

    // Debug message
    lMessage = "     GexDbPlugin_Galaxy::UpdateLotConsolidation() : Consolidation completed";
    WriteDebugMessageFile(lMessage);

labelUnlock:

    TimersTraceStop("Function Lot Consolidation");
    TimersTraceStop("Function Lot Consolidation: consolidation trigger");
    TimersTraceStop("Function Lot Consolidation: consolidation update");
    TimersTraceStop("Function Lot Consolidation: product update");

    // Commit transaction before to update INCREMENTAL_UPDATE
    clGexDbQuery.Execute("COMMIT");

    ReleaseTokens();

    // When all is OK
    // The BINNING_CONSOLIDATION flags will be removed by the IncremenatlUpdate process

    return bStatus;
}

///////////////////////////////////////////////////////////
// Get Retest Info
// For each internal Test/Retest phase, we need to have the list of
// bins retested for each RetestLevel
// For each Phase, we need to know the method used to initialize the stage
// ex 1:
// HOT 0            => reset all
// HOT 1 all_fail   => consolidation from HOT 0 and HOT 1
// COLD 0 all_pass  => initialization with the result PASS from INTER HOT
// COLD 1 all_fail  => consolidation from COLD 0 and COLD 1
// AMB 0 all_pass   => initialization with the result PASS from INTER COLD
// AMB 1 all_fail   => consolidation from AMB 0 + AMB 1
// ex 2:
// HOT 0            => reset all
// HOT 1 all_fail   => consolidation from HOT 0 and HOT 1
// COLD 0 all_reset => reset all
// COLD 1 all_fail  => consolidation from COLD 0 and COLD 1
// AMB 0 all_pass   => initialization with the result PASS from INTER COLD
// AMB 1 all_fail   => consolidation from AMB 0 + AMB 1
///////////////////////////////////////////////////////////
// lRetestPhases contains the list of Phases order by StartTime
// mapPhaseRetestHBins contains for each phases and each retest level the list of Bins retested
// The retest level 0 is the first internal Test/Retest phase
// if for this index, there is no Bins retested, indicates that we restart from 0 (reset all)
// if for this index, there is a list of Bins, indicates that we start from the previous consolidation
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetConsolidationRetestPhases(const QString &LotId, const QString &SubLotId,
                                                        QString &ProdData, QStringList &lstPhases)
{
    if(m_eTestingStage != eFinalTest)
    {
        QString lValue;
        GetTestingStageName(m_eTestingStage,lValue);
        QString lMsg = QString("Testing Stage [%1] not supported").arg(lValue);
        GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported,NULL, lMsg.toLatin1().data());
        return false;
    }

    QString           lQuery;
    QString           lPhaseName;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Check if need to filter on the retest phases
    QString lPhaseClause;
    QStringList::const_iterator lPhaseNameBegin = lstPhases.begin();
    QStringList::const_iterator lPhaseEnd = lstPhases.end();
    for (; lPhaseNameBegin != lPhaseEnd; ++lPhaseNameBegin)
    {
        if(!lPhaseClause.isEmpty())
            lPhaseClause += " OR ";
        lPhaseClause += "TEST_INSERTION="+TranslateStringToSqlVarChar(*lPhaseNameBegin);
    }

    ///////////////////////////////////////////////////////////
    // Update lot information with data
    // from production data if have
    // else from non-production data
    if(ProdData.isEmpty())
    {
        ProdData = "N";
        lQuery = "SELECT DISTINCT PROD_DATA FROM "+NormalizeTableName("_SPLITLOT");
        lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += " AND SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
        if(!lPhaseClause.isEmpty())
            lQuery += " AND ("+lPhaseClause+")";
        lQuery += " AND  ";
        lQuery += " VALID_SPLITLOT<>'N'";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        if(!clGexDbQuery.First())
        {
            // must have one element
            GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, lQuery.toLatin1().constData());
            return false;
        }

        do
        {
            if(clGexDbQuery.value(0).toString().startsWith("Y",Qt::CaseInsensitive))
                ProdData = "Y";
        }
        while(clGexDbQuery.Next());
    }

    ///////////////////////////////
    /// FINAL TEST INTERMEDIATE CONSOLIDATION
    // Intermediate consolidation is possible
    // only for the 'standard' flow
    // NO PHASE
    // NO PHASE + NEW PHASE ?
    // PHASE A - RETEST_INDEX 0,1,2, ...
    // PHASE B - RETEST_INDEX 0,1,2, ...
    // PHASE A + StartTime < PHASE b +StartTime
    /// FINAL TEST INTERMEDIATE CONSOLIDATION
    ///////////////////////////////
    bool    lHaveEmptyPhase = false;
    // Check if consolidation is possible
    // t0, HOT, 0
    // t1, HOT, 1, all_fail
    // t2, HOT, 2, all_fail
    // t3, COLD, 0, all_pass
    // t4, COLD, 1, all_fail
    // t5, COLD, 2, all_fail
    // t6, END, 0, all_pass
    // t7, END, 1, bin7
    // t8, END, 1, bin8
    // t9, END, 2, all_fail
    lQuery = " SELECT F.TEST_INSERTION ";
    lQuery += " FROM "+NormalizeTableName("_SPLITLOT")+" F ";
    lQuery += " WHERE ";
    lQuery += " F.LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += " AND  ";
    lQuery += " F.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
    if(!lPhaseClause.isEmpty())
        lQuery += " AND ("+lPhaseClause+")";
    lQuery += " AND  ";
    lQuery += " F.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
    lQuery += " AND ";
    lQuery += " F.VALID_SPLITLOT<>'N'";
    lQuery += " ORDER BY F.START_T";

    // Mixe between NO PHASE and PHASE is not allowed
    // Get the list of PHASE order by StartTime
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    lstPhases.clear();
    while(clGexDbQuery.Next())
    {
        lPhaseName = clGexDbQuery.value(0).toString();
        if(lPhaseName.isEmpty())
            lHaveEmptyPhase = true;
        if(!lstPhases.contains(lPhaseName))
            lstPhases << lPhaseName;
        if(lHaveEmptyPhase && (lstPhases.count()>1))
        {
            QString lMsg = QString("Mix between empty TEST_INSERTION and not empty TEST_INSERTION");
            GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported,NULL, lMsg.toLatin1().data());
            return false;
        }
    }

    return true;
}

/*
+-------------+---------------+-----------+------------+----------+---------------+--------------+--------------+----------------------+
| splitlot_id | part_typ      | lot_id    | start_t    | nb_parts | nb_parts_good |test_insertion| retest_index | retest_hbins         |
+-------------+---------------+-----------+------------+----------+---------------+--------------+--------------+----------------------+
|  1533500417 | I56150A1KFSBH | 4354717E1 | 1448752201 |     4391 |          4226 | FT1          |            0 | all_pass             |
|  1533500436 | I56150A1KFSBH | 4354717E1 | 1448872541 |      153 |           130 | FT1          |            1 | 3,4,5,8,9            |
|  1533500565 | I56150A1KFSBH | 4354717E1 | 1448883612 |       23 |             8 | FT1          |            2 | 3,4,5,8,9            |
|  1533600385 | I56150A1KFSBH | 4354717E1 | 1448934550 |     4364 |          4311 | FT2          |            0 | all_pass             |
|  1533600402 | I56150A1KFSBH | 4354717E1 | 1448959667 |       53 |            41 | FT2          |            1 | 3,4,5,8,9,7,9999,6,2 |
|  1533600436 | I56150A1KFSBH | 4354717E1 | 1448962802 |        5 |             2 | FT2          |            2 | 3,4,5,8,9            |
|  1533500485 | I56150A1KFSBH | 4354717E1 | 1448878130 |      144 |           142 | QA1          |            0 | all_pass             |
|  1533500474 | I56150A1KFSBH | 4354717E1 | 1448882211 |        2 |             2 | QA1          |            1 | 7,9999,6,3,4,5,8,9,2 |
+-------------+---------------+-----------+------------+----------+---------------+--------------+--------------+----------------------+
+-----------+-----------+----------+---------------+------------------------+--------------------+--------------------+
| lot_id    | sublot_id | nb_parts | nb_parts_good | consolidated_data_type | consolidation_name | consolidation_flow |
+-----------+-----------+----------+---------------+------------------------+--------------------+--------------------+
| 4354717E1 | 4354717E1 |     4391 |          4364 | INTERMEDIATE           | FT1                | P                  |
| 4354717E1 | 4354717E1 |     4364 |          4354 | INTERMEDIATE           | FT2                | P                  |
| 4354717E1 | 4354717E1 |      144 |           144 | INTERMEDIATE           | QA1                | P                  |
+-----------+-----------+----------+---------------+------------------------+--------------------+--------------------+

 */
///////////////////////////////////////////////////////////
////// \brief GexDbPlugin_Galaxy::GetFtConsolidationRetestInfo
////// \param LotId
////// \param SubLotId
////// \param ProdData
////// \param lstPhases: contains the list of Phases order by StartTime
////// \param mapPhaseRetestHBins: contains for each phases and each retest level the list of Bins retested
// The retest level 0 is the first internal Test/Retest phase
// if for this index, there is no Bins retested, indicates that we restart from 0 (reset all)
// if for this index, there is a list of Bins, indicates that we start from the previous consolidation
// if the consolidation is not possible, lstPhases, mapPhaseRetestHBins are empty
//      * if missing RetestPhase
//      * if missing a RetestIndex
//      * if invalid RetestHbins
//      * the m_strlWarnings will contain this info
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetConsolidationRetestInfo(const QString &LotId, const QString &SubLotId,
                                                      QString &ProdData, QStringList &lstPhases,
                                                      QMap<QString, QMap<int,QStringList> > &mapPhaseRetestHBins)
{
    if(m_eTestingStage != eFinalTest)
        return false;

    QString           lQuery;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    m_mapHBinInfo.clear();
    m_mapSBinInfo.clear();

    mapPhaseRetestHBins.clear();

    if(!GetConsolidationRetestPhases(LotId,SubLotId,ProdData,lstPhases))
        return false;

    QString lPhaseName;
    QMap<QString, QList<int> > lMapPhaseIndexes; // list of all Indexes per PHASE

    // Get the list of Retest_index for each Phases
    //foreach(lPhaseName, lstPhases)
    for(int index=0; index<lstPhases.count(); ++index)
    {
        lPhaseName = lstPhases.at(index);

        lQuery = " SELECT DISTINCT F.RETEST_INDEX ";
        lQuery += " FROM "+NormalizeTableName("_SPLITLOT")+" F ";
        lQuery += " WHERE ";
        lQuery += " F.LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += " AND  ";
        lQuery += " F.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
        lQuery += " AND  (";
        lQuery += " F.TEST_INSERTION="+TranslateStringToSqlVarChar(lPhaseName);
        if(lPhaseName.isEmpty())
            lQuery+= " OR F.TEST_INSERTION IS NULL";
        lQuery += " ) AND  ";
        lQuery += " F.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        lQuery += " AND ";
        lQuery += " F.VALID_SPLITLOT<>'N'";
        lQuery += " ORDER BY F.RETEST_INDEX";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            lstPhases.clear();
            mapPhaseRetestHBins.clear();
            return false;
        }
        while(clGexDbQuery.Next())
            lMapPhaseIndexes[lPhaseName] << clGexDbQuery.value(0).toInt();
    }

    // For each PHASE, check the list of RETEST_INDEX
    //foreach(lPhaseName, lstPhases)
    for(int index=0; index<lstPhases.count(); ++index)
    {
        lPhaseName = lstPhases.at(index);

        // Check if the RETEST_INDEX starts with 0
        if(!lMapPhaseIndexes[lPhaseName].contains(0))
        {
            // all Phases must start with the RETEST_INDEX = 0
            QString lMsg =
                    QString("TEST_INSERTION[%1] doesn't start with RETEST_INDEX=0 for LOT_ID[%2] and SUBLOT_ID[%3] - Pending for new data")
                    .arg(lPhaseName).arg(LotId).arg(SubLotId) ;
            WarningMessage(lMsg);
            lstPhases.clear();
            mapPhaseRetestHBins.clear();
            // Consolidation not ready
            return true;
        }
        // Check if have all RETEST_INDEX between 0 and MAX
        // ex: 0, 1, 2, 3 => ok
        // ex: 0, 2, 3, 4 => ko
        if((lMapPhaseIndexes[lPhaseName].last()+1) != lMapPhaseIndexes[lPhaseName].count())
        {
            // all Phases must have all the RETEST_INDEX between 0 and MAX
            QString lMsg =
                    QString("Missing RETEST_INDEXES for the TEST_INSERTION[%1], LOT_ID[%2] and SUBLOT_ID[%3] - Pending for new data")
                    .arg(lPhaseName).arg(LotId).arg(SubLotId);
            WarningMessage(lMsg);
            lstPhases.clear();
            mapPhaseRetestHBins.clear();
            // Consolidation not ready
            return true;
        }
    }

    if(lstPhases.isEmpty())
    {
        // all Phases must have all the RETEST_INDEX between 0 and MAX
        QString lMsg =
                QString("No TEST_INSERTION detected - Pending for new data")
                .arg(lPhaseName).arg(LotId).arg(SubLotId);
        WarningMessage(lMsg);
        lstPhases.clear();
        mapPhaseRetestHBins.clear();
        // Consolidation not ready
        return true;
    }

    // Consolidation can be done
    // have all PHASE and RETEST_INDEX
    // The default is to reset the initial RETEST_INDEX (=0)
    // If the Phase specifies an other RETEST_HBINS (ie: all_pass), reset only needed bins
    if(m_mapHBinInfo.isEmpty())
    {
        // Update mapHbinInfo and mapSBinInfo with previous info if necessary
        // For HBIN
        lQuery = " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" H.HBIN_NO, ";
        lQuery += " SUBSTRING_INDEX(MAX(CONCAT(F.START_T,'#:#',H.HBIN_CAT)),'#:#',-1) AS HBIN_CAT, ";
        lQuery += " SUBSTRING_INDEX(MAX(CONCAT(F.START_T,'#:#',H.HBIN_NAME)),'#:#',-1) AS HBIN_NAME ";
        lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" F ";
        lQuery += " INNER JOIN "+NormalizeTableName("_HBIN")+" H ";
        lQuery += "     ON F.SPLITLOT_ID=H.SPLITLOT_ID ";
        lQuery += " WHERE ";
        lQuery += "     F.VALID_SPLITLOT<>'N'";
        lQuery += "     AND F.LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND F.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
        lQuery += "     AND F.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        lQuery += " GROUP BY H.HBIN_NO";

        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            lstPhases.clear();
            mapPhaseRetestHBins.clear();
            return false;
        }

        int nBinNo;
        QString lBinCat;
        while(clGexDbQuery.Next())
        {
            // update values for Bin info
            nBinNo = clGexDbQuery.value(0).toInt();
            lBinCat = clGexDbQuery.value(1).toString();
            m_mapHBinInfo[nBinNo].m_nBinNum = nBinNo;
            // Keep the last definition according to the start_t
            if(m_mapHBinInfo[nBinNo].m_strBinName.isEmpty())
                m_mapHBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(2).toString();
            if(lBinCat=="P")
                m_mapHBinInfo[nBinNo].m_cBinCat = 'P';
            else if(lBinCat=="A")
                m_mapHBinInfo[nBinNo].m_cBinCat = 'A';
            else
                m_mapHBinInfo[nBinNo].m_cBinCat = 'F';
        }
    }

    clGexDbQuery.setForwardOnly(false);

    int     i, iStart, iEnd;
    int     nRetestIndex;
    bool    bIsInt;
    QString lHBins;
    QString lValue;

    //foreach(lPhaseName, lstPhases)
    for(int index=0; index<lstPhases.count(); ++index)
    {
        lPhaseName = lstPhases.at(index);

        // Start with the first phase and finish with the last
        // List of Retest bins
        // With this information, we can reset all HBin/SBin from RetestIndex-1 and add new HBin/SBin from RetestIndex
        lQuery = " SELECT F.RETEST_INDEX, F.RETEST_HBINS ";
        lQuery += " FROM  ";
        lQuery += " "+NormalizeTableName("_SPLITLOT")+" F ";
        lQuery += " WHERE  ";
        lQuery += " F.LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += " AND ";
        lQuery += " F.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
        lQuery += " AND (";
        lQuery += " F.TEST_INSERTION="+TranslateStringToSqlVarChar(lPhaseName);
        if(lPhaseName.isEmpty())
            lQuery += " OR  F.TEST_INSERTION IS NULL";
        lQuery += ") AND ";
        lQuery += " F.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        lQuery += " AND ";
        lQuery += " VALID_SPLITLOT<>'N'";
        lQuery += " ORDER BY F.RETEST_INDEX ";

        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            lstPhases.clear();
            mapPhaseRetestHBins.clear();
            return false;
        }

        while(clGexDbQuery.Next())
        {
            // update values for Bin info
            nRetestIndex = clGexDbQuery.value(0).toInt();
            lHBins = clGexDbQuery.value(1).toString().simplified().toLower().remove(" ");

            // If the RetestHBins is empty for the RetestIndex 0
            // and if it is not the first Phase
            // this meens that we need to retest all_pass
            // else this meens that we need to retest all
            if((nRetestIndex == 0) && (lPhaseName != lstPhases.first()) && lHBins.isEmpty())
                lHBins = "all_pass";
            if((nRetestIndex == 0) && lHBins == "all")
                lHBins = "";
            if(lHBins == "all_reset")
                lHBins = "";

            // RESTRICTION FOR FIRST INTERMEDIATE CONSOLIDATION VERSION
            // Reject consolidation if the retest PHASE doesn't use 'all', 'all_reset', 'all_pass', 'all_fail', ''
            if(nRetestIndex == 0)
            {
                if(!lHBins.isEmpty() && !lHBins.startsWith("all", Qt::CaseInsensitive))
                {
                    QString lMsg = QString("Invalid RetestHBins[%1] for RetestIndex[0] and Phase[%2]")
                            .arg(lHBins).arg(lPhaseName);
                    lMsg += " - use 'all', 'all_reset', 'all_pass', 'all_fail'";
                    GSET_ERROR1(GexDbPlugin_Base, eValidation_NotSupported,NULL,
                                lMsg.toLatin1().data());
                    lstPhases.clear();
                    mapPhaseRetestHBins.clear();
                    // Generate a new warning
                    lMsg = "Sublot/Lot binning consolidation error [";
                    lMsg+= GGET_LASTERRORMSG(GexDbPlugin_Base,this);
                    lMsg+= "]";
                    WarningMessage(lMsg);
                    return false;
                }

                mapPhaseRetestHBins[lPhaseName][nRetestIndex] << lHBins;
                continue;
            }

            // For all F.RETEST_INDEX, have to concat F.RETEST_HBINS in the same strHBins
            // to check each RetestIndex in one step
            while(clGexDbQuery.Next())
            {
                if(clGexDbQuery.value(0).toInt() != nRetestIndex)
                {
                    // retrieve the previous position
                    clGexDbQuery.previous();
                    break;
                }

                lHBins += "," +clGexDbQuery.value(1).toString().simplified().toLower().remove(" ");
            }

            QStringList lstHBins;
            lHBins = lHBins.replace("+",",");

            // Accept old syntax: "all", "all_pass" and "all_fail" KeyWords
            // Add syntax "all_reset" that retest all levels before
            // Check if strHBins contains this KeyWord
            if(lHBins.toLower().indexOf("all") >= 0)
            {
                // Have to parse strHBins
                QMap<int,structBinInfo>::Iterator itBinInfo;
                int iIndex;
                for(iIndex = 0; iIndex<=lHBins.count(","); iIndex++)
                {
                    lValue = lHBins.section(",", iIndex, iIndex).toLower();

                    if(lValue.startsWith("all_pass"))
                    {
                        // Have to retrieve all pass binnings
                        for ( itBinInfo = m_mapHBinInfo.begin(); itBinInfo != m_mapHBinInfo.end(); itBinInfo++ )
                        {
                            if(itBinInfo.key() == MERGE_BIN)
                                continue;

                            if(itBinInfo.value().m_cBinCat == 'P')
                                lstHBins.append(QString::number(itBinInfo.key()));
                        }
                    }
                    else if(lValue.startsWith("all_fail"))
                    {
                        // Have to retrieve all fail binnings
                        for ( itBinInfo = m_mapHBinInfo.begin(); itBinInfo != m_mapHBinInfo.end(); itBinInfo++ )
                        {
                            if(itBinInfo.key() == MERGE_BIN)
                                continue;

                            if(itBinInfo.value().m_cBinCat != 'P')
                                lstHBins.append(QString::number(itBinInfo.key()));
                        }
                    }
                    else if(lValue.startsWith("all_reset"))
                    {
                        // Have to retrieve all binnings
                        for ( itBinInfo = m_mapHBinInfo.begin(); itBinInfo != m_mapHBinInfo.end(); itBinInfo++ )
                        {
                            if(itBinInfo.key() == MERGE_BIN)
                                continue;

                            lstHBins.append(QString::number(itBinInfo.key()));
                        }
                    }
                    else if(lValue.startsWith("all"))
                    {
                        // Have to retrieve all binnings
                        for ( itBinInfo = m_mapHBinInfo.begin(); itBinInfo != m_mapHBinInfo.end(); itBinInfo++ )
                        {
                            if(itBinInfo.key() == MERGE_BIN)
                                continue;

                            lstHBins.append(QString::number(itBinInfo.key()));
                        }
                    }
                    else
                        lstHBins.append(lValue);
                }

                lHBins = lstHBins.join(",");
            }

            // Check if have a valid list
            QRegExp reg("^((\\d+-\\d+)|(\\d+))(,((\\d+-\\d+)|(\\d+)))*$");
            if(!reg.exactMatch(lHBins))
            {
                QString lMsg = QString("Invalid RetestHBins: %1").arg(lHBins);
                WarningMessage(lMsg);
                lstPhases.clear();
                mapPhaseRetestHBins.clear();
                return true;
            }

            // strHBins = "1,4-7,2" == "1,2,4,5,6,7"
            // strHBins = "01,04-07,02" == "1,2,4,5,6,7"
            while(!lHBins.isEmpty())
            {
                while((lHBins.startsWith("0") && (lHBins.length() > 1)))
                    lHBins = lHBins.mid(1);

                i=1;
                bIsInt=true;
                while(bIsInt)
                {
                    lHBins.left(i).toInt(&bIsInt);
                    if(bIsInt)
                        i++;
                    if((int)lHBins.length() < i)
                        break;
                }
                i--;
                iStart = lHBins.left(i).toInt();
                lHBins = lHBins.mid(i).simplified();
                if(lHBins.startsWith("-"))
                {
                    lHBins = lHBins.mid(1).simplified();

                    while((lHBins.startsWith("0") && (lHBins.length() > 1)))
                        lHBins = lHBins.mid(1);

                    i=1;
                    bIsInt=true;
                    while(bIsInt)
                    {
                        lHBins.left(i).toInt(&bIsInt);
                        if(bIsInt)
                            i++;
                        if((int)lHBins.length() < i)
                            break;
                    }
                    i--;
                    iEnd = lHBins.left(i).toInt();
                    lHBins = lHBins.mid(i).simplified();
                }
                else
                    iEnd=iStart;

                // for each bin found
                // have to reset counter for this bin for the retest_index before (-1)
                for(i=iStart; i<=iEnd; i++)
                {
                    // Save all HBin retested
                    if(!lstHBins.contains(QString::number(i)))
                        lstHBins << QString::number(i);
                }


                if(lHBins.startsWith(","))
                    lHBins = lHBins.mid(1).simplified();
            }

            //foreach(lValue, lstHBins)
            for(int index=0; index<lstHBins.count(); ++index)
            {
                lValue = lstHBins.at(index);
                mapPhaseRetestHBins[lPhaseName][nRetestIndex] << lValue;

            }
        }
    }

    return true;
}

//////////////////////////////////////////////////////////
////// \brief GexDbPlugin_Galaxy::UpdateFtRunConsolidationTable
/// Update the RUN CONSOLIDATION flags
////// \param LotId
////// \param SubLotId
////// \param ProdData
////// \param lstPhases : computed by GetFtConsolidationRetestInfo
////// \param mapPhaseRetestHBins : computed by GetFtConsolidationRetestInfo
////// \param mapPhaseMissingParts : updated if found MissingParts between RetestPhase or RetestIndex
// The MissingParts info will be used to retrieve the good Consolidation Results
////// \return FALSE if SQL issue
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateSubLotRunConsolidationTable(const QString &LotId,const QString &SubLotId,const QString &ProdData,
                                                       QStringList &lstPhases,
                                                       QMap<QString, QMap<int,QStringList> > &mapPhaseRetestHBins,
                                                       QMap<QString, int> &mapPhaseMissingParts)
{
    if(m_eTestingStage != eFinalTest)
        return false;

    QString            lQuery;
    GexDbPlugin_Query  clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    clGexDbQuery.setForwardOnly(true);
    mapPhaseMissingParts.clear();

    ///////////////////////////////////////////////////////////
    // Run Consolidation
    ///////////////////////////////////////////////////////////
    QString lValue;
    QString lPhaseName;

    // The RUN CONSOLIDATION flags
    // for FinalTest
    if(!lstPhases.isEmpty())
    {
        int     nRetestIndex;
        QString lPhaseBefore;
        QString lBinClause;
        QString lConsolidationAlgo;
        int     nRetestedParts;
        int     nNewParts;

        QMap<QString, QStringList> lTestInsertionSplitlots;
        // Extract the list of splitlots for each test_insertion
        lQuery = "SELECT S.TEST_INSERTION, S.RETEST_INDEX, GROUP_CONCAT(S.SPLITLOT_ID) as SPLITLOTS "
                " FROM  "+NormalizeTableName("_SPLITLOT")+" S "
                " WHERE S.VALID_SPLITLOT<>'N'"
                "   AND  S.LOT_ID="+TranslateStringToSqlVarChar(LotId)+
                "   AND  S.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId)+
                "   AND  S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData)+
                " GROUP BY S.TEST_INSERTION, S.RETEST_INDEX";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        while(clGexDbQuery.Next())
        {
            // Global level
            lTestInsertionSplitlots["SPLITLOTSFORALL"] += clGexDbQuery.value("SPLITLOTS").toStringList();
            // Test insertionlevel
            lTestInsertionSplitlots[clGexDbQuery.value("TEST_INSERTION").toString()] += clGexDbQuery.value("SPLITLOTS").toStringList();
            // Retest level
            lTestInsertionSplitlots[clGexDbQuery.value("TEST_INSERTION").toString()+":"+clGexDbQuery.value("RETEST_INDEX").toString()] += clGexDbQuery.value("SPLITLOTS").toStringList();
        }

        // Reset the current RUN CONSOLIDATION flags if any
        lQuery = "UPDATE "+NormalizeTableName("_RUN")+" R "
                "   SET R.intermediate=false, R.final=false "
                " WHERE R.SPLITLOT_ID IN ("+lTestInsertionSplitlots["SPLITLOTSFORALL"].join(",")+")"
                // To disabled the Gap Lock on xx_run
                " AND R.RUN_ID>0 ";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }
        /////////////////////////
        // DEADLOCK PREVENTION
        // Commit transaction
        clGexDbQuery.Execute("COMMIT");

        for(int index=0; index<lstPhases.count(); ++index)
        {
            lPhaseName = lstPhases.at(index);
            // Update consolidation information
            QList<int>              lstRetestIndex;

            QList<int>::Iterator itRetestIndex;
            lstRetestIndex = mapPhaseRetestHBins[lPhaseName].keys();
            for(itRetestIndex=lstRetestIndex.begin(); itRetestIndex!=lstRetestIndex.end(); itRetestIndex++)
            {
                nRetestIndex = (*itRetestIndex);
                nRetestedParts = 0;
                nNewParts = 0;

                // Check if the RETEST_HBINS is valid
                // and constuct the bin clause
                lValue = mapPhaseRetestHBins[lPhaseName][nRetestIndex].first();
                // Reset only bin retested
                // The retest PHASE can be 'all', 'all_reset', 'all_pass', 'all_fail', ''
                if(nRetestIndex == 0)
                {
                    // Reset the FINAL flags
                    if(lValue.isEmpty() || (lValue == "all") || (lValue == "all_reset"))
                    {
                        // Reset ALL previews runs from ALL RetestPhases
                        lBinClause = "";
                    }
                    else if(lValue == "all_fail")
                        // Reset ALL fail runs
                        lBinClause = " R.SPLITLOT_ID IN ("+lTestInsertionSplitlots[lPhaseBefore].join(",")+") AND R.PART_STATUS='F' ";
                    else if(lValue == "all_pass")
                        // Reset ALL pass runs
                        lBinClause = " R.SPLITLOT_ID IN ("+lTestInsertionSplitlots[lPhaseBefore].join(",")+") AND R.PART_STATUS='P' ";

                    if(!lValue.isEmpty())
                    {
                        // Just keep the last for SUBLOT_INFO update
                        lConsolidationAlgo = lValue;
                    }
                }
                else
                {
                    // Remove the runs
                    lBinClause = " R.SPLITLOT_ID IN ("+lTestInsertionSplitlots[lPhaseName+":"+QString::number(nRetestIndex-1)].join(",")+") ";
                    if(lValue.isEmpty() || (lValue == "all") || (lValue == "all_reset"))
                    {
                        // Reset ALL previews runs from this RetestPhase
                    }
                    else if(lValue == "all_fail")
                        // Reset ALL fail runs
                        lBinClause += " AND R.PART_STATUS='F' ";
                    else if(lValue == "all_pass")
                        // Reset ALL pass runs
                        lBinClause += " AND R.PART_STATUS='P' ";
                    else
                        // Reset a list of bin runs
                        lBinClause += " AND R.HBIN_NO IN ("+mapPhaseRetestHBins[lPhaseName][nRetestIndex].join(",")+") ";
                }


                // Check if it is a next new Phase
                // then update the FINAL flags
                if(!lPhaseBefore.isEmpty() && nRetestIndex==0)
                {
                    // Just between 2 Phases
                    // Do not remove the retested parts from the previous phase in the RUN CONSOLIDATION flags
                    // but flags them at not FINAL
                    if(lBinClause.isEmpty())
                    {
                        // All the runs from the PhaseBefore (all retest_index) was retested
                        lQuery = "UPDATE "+NormalizeTableName("_RUN")+" R "
                                " SET R.FINAL=false"
                                " WHERE R.SPLITLOT_ID IN ("+lTestInsertionSplitlots[lPhaseBefore].join(",")+")"
                                // To disabled the Gap Lock on xx_run
                                " AND R.RUN_ID>0 ";
                    }
                    else
                    {
                        // Only some HBIN_NO from the PhaseBefore (all retest_index) was retested
                        lQuery = "UPDATE "+NormalizeTableName("_RUN")+" R "
                                " SET R.FINAL=false "
                                " WHERE R.SPLITLOT_ID IN ("+lTestInsertionSplitlots[lPhaseBefore].join(",")+")"
                                // To disabled the Gap Lock on xx_run
                                " AND R.RUN_ID>0 "
                                "   AND "+lBinClause;
                    }
                    if(!clGexDbQuery.Execute(lQuery))
                    {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                        return false;
                    }
                    // number of retested parts from the previous phase
                    nRetestedParts = clGexDbQuery.numRowsAffected();
                }
                else if(nRetestIndex > 0)
                {
                    // Into a same RetestPhase
                    // Between 2 RetestIndex
                    // Check if have to reset all - EMPTY BinList
                    lQuery = "UPDATE "+NormalizeTableName("_RUN")+" R "
                            "   SET R.intermediate=false, R.final=false "
                            " WHERE R.SPLITLOT_ID IN ("+lTestInsertionSplitlots["SPLITLOTSFORALL"].join(",")+")"
                            // To disabled the Gap Lock on xx_run
                            " AND R.RUN_ID>0 "
                            "   AND "+lBinClause ;
                    if(!clGexDbQuery.Execute(lQuery))
                    {
                        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                        return false;
                    }
                    nRetestedParts = clGexDbQuery.numRowsAffected();
                }

                lPhaseBefore = lPhaseName;

                // Then insert the new runs from this TEST_INSERTION+RETEST_HBINS

                // The new RUN CONSOLIDATION flags must be populate here
                // The consolidation results (HBin/SBin count) will be based on the  RUN CONSOLIDATION flags
                // First update all runs from this set of splitlots
                // Except the INLINE retested
                lQuery = "UPDATE "+NormalizeTableName("_RUN")+" R "
                        "   SET R.intermediate=true, R.final=true "
                        " WHERE R.SPLITLOT_ID IN ("+lTestInsertionSplitlots[lPhaseName+":"+QString::number(nRetestIndex)].join(",")+") "
                        // To disabled the Gap Lock on xx_run
                        " AND R.RUN_ID>0 "
                        // IGNORE INLINE RETESTED PARTS
                        "   AND (R.PART_FLAGS&"+QString::number(FLAG_RUN_PARTRETESTED)+")=0 ";
                if(!clGexDbQuery.Execute(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                    return false;
                }
                nNewParts = clGexDbQuery.numRowsAffected();

                // When some parts were retested
                // Check if have some MissingParts
                if((nRetestedParts > 0)
                        && (nNewParts != nRetestedParts))
                {
                    if(nRetestIndex == 0)
                    {
                        if(nRetestedParts > nNewParts)
                        {
                            // ie 50 parts tested in the PhaseBefore are Missing in the next Phase
                            // must be added to the next Phase AS MISSING PARTS
                            // This MISSING PARTS will be display in the FINAL to complete the TOTAL INITIAL PARTS
                            mapPhaseMissingParts[lPhaseName] = nRetestedParts - nNewParts;
                        }
                        else
                        {
                            // ie 50 new parts tested in the next Phase that doesn't exist in the PhaseBefore
                            // Nothing to do
                            // This 50 new parts are flagged in the RUN CONSOLIDATION flags
                        }
                    }
                    else
                    {
                        if(nRetestedParts > nNewParts)
                        {
                            // ie 50 parts tested in the RetestIndex before are Missing in the next Index
                            mapPhaseMissingParts[lPhaseName] += nRetestedParts - nNewParts;
                        }
                        else
                        {
                            // ie 50 new parts tested in the next Phase that doesn't exist in the PhaseBefore
                            // Nothing to do
                            // This 50 new parts are flagged in the RUN CONSOLIDATION flags
                        }
                    }
                }

                /////////////////////////
                // DEADLOCK PREVENTION
                // Commit transaction
                clGexDbQuery.Execute("COMMIT");
            }
        }

        if(!lConsolidationAlgo.isEmpty())
        {
            // BECAUSE WE HAVE THE new CONSOLIDATION Configuration
            // Then update the SPLITLOT/SUBLOT table for TEST_INSERTION, RETEST_HBINS, CONSOLIDATION_ALGO
            lQuery = "UPDATE "+NormalizeTableName("_SUBLOT_INFO")+" SB "
                    "   SET SB.CONSOLIDATION_ALGO='"+lConsolidationAlgo+"' "
                    " WHERE SB.LOT_ID="+TranslateStringToSqlVarChar(LotId)+
                    "  AND SB.SUBLOT_ID="+TranslateStringToSqlVarChar(SubLotId);
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }
    }

    return true;
}


///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateWaferRunConsolidationTable
// For WT_RUN CONSOLIDATION flags
///////////////////////////////////////////////////////////
////// \brief GexDbPlugin_Galaxy::UpdateWaferRunConsolidationTable
/// Based on the Consolidation Tree
////// \param ProductName
////// \param LotId
////// \param WaferId
////// \param ProdData
////// \param PhysicalConsolidationName : Give PHYSICAL consolidation name (empty if error)
////// \param RunConsolidationSummary : Consolidation error message if any
/// \return
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateWaferRunConsolidationTable(const QString &LotId,const QString &WaferId,const QString &ProdData,
                                                     QString &PhysicalConsolidationName, QString &ConsolidationSummary)
{

    if(m_eTestingStage == eFinalTest)
        return true;

    // Debug message
    QString strMessage = "---- GexDbPlugin_Galaxy::UpdateWaferRunConsolidationTable() : Consolidation process for table ";
    strMessage += NormalizeTableName("_RUN");
    WriteDebugMessageFile(strMessage);

    PhysicalConsolidationName = "";
    ConsolidationSummary = "";

    bool       bQueryIssue = false;
    bool       bConsolidationIssue = false;
    QString    lNewValue;              // NEW VALUE FOR INSERTION

    QString             lQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    clGexDbQuery.setForwardOnly(true);


    // WAFER CONSOLIDATION
    // For HBin and SBin consolidation per wafer
    QMap<QString,QString>  mapConsolidationTables;

    // For Test Conditions and Consolidation Rules

    TimersTraceStart("Function Wafer Run Consolidation");

    ///////////////////////////////////////////////////////////
    // PROD_DATA INFO
    // PROD_DATA, START_TIME, SAMPLES_FLAGS
    // FROM SPLITLOT(LOT/WAFER)
    ///////////////////////////////////////////////////////////
    // FOR ALL TEST CONDITIONS
    // Upate lot information with data
    // only from production data

    ///////////////////////////////////////////////////////////
    // FOR ALL TEST CONDITIONS
    // Check if have only one splitlot for (LotId,WaferId)
    // And get the Last Splitlot_id for summary
    // FROM SPLITLOT(LOT/WAFER/PROD_DATA/FLAGS)
    ///////////////////////////////////////////////////////////
    // Retest for WaferSort
    //  * Dies retested
    //      - during reconsolidation: check retest_index in wt_run table
    //  * more that one splitlot
    ///////////////////////////////////////////////////////////

    ///////////////////////////////////////
    // START RUN CONSOLIDATION NOW
    ///////////////////////////////////////
    if(m_eTestingStage == eWaferTest)
    {
        // For STM_PATMAN
        // wt_sbin and wt_sbin_stats_samples was updated by the call of the stored procedure wt_insertion_postprocessing()
        // For all other cases, we have to check if the good counts comes from samples or summary

        ///////////////////////////////
        // BEGIN FLEXIBLE CONSOLIDATION
        // DATA SAMPLES / DATA SUMMARY
        // Check if have some test conditions defined
        CTTestCondition tcItem;
        QList<CTTestCondition> tcList;
        CTConsolidationRule crItem;
        QList<CTConsolidationRule> crList;
        CTQueryFilter filters;

        QString lProductName;
        QString crName;
        QString crBaseName;
        QString crDataType;
        QString crAlgorithm;
        QString crStoredResult;
        QString crProductionFlow;
        QString crGroupBy;

        QString tcName;
        QString tcField;
        QString tcValue;
        QStringList tcListValues;

        // FOR UPDATE PHYSICAL CONSOLIDATION TABLE WHEN NO RETEST
        // TABLE WT_WAFER_CONSOLIDATION
        QString lConsolidationName;
        QString lConsolidationFlow;

        lQuery = "SELECT PRODUCT_NAME, ";
        lQuery += TranslateUnixTimeStampToSqlDateTime("MIN(START_T)",eDate);
        lQuery += " AS START_T, ";
        lQuery += " MAX(TEST_FLOW) AS TEST_FLOW ";
        lQuery += " FROM "+NormalizeTableName("_SPLITLOT");
        lQuery += " WHERE ";
        lQuery += "   VALID_SPLITLOT<>'N'";
        lQuery += "   AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "   AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "   AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }
        clGexDbQuery.First();
        lProductName = clGexDbQuery.value("PRODUCT_NAME").toString();
        lConsolidationFlow = clGexDbQuery.value("TEST_FLOW").toString();

        // STORE THE RESULTS FOR EACH TEST CONDITIONS
        QMap<QString, QString >       mapConsolidationDataType;
        QMap<QString, QString >       mapConsolidationProductionFlow;

        // From consolidation tree
        filters.add(CTQueryFilter::FilterTestingStage, "Wafer Sort");
        filters.add(CTQueryFilter::FilterProductID, lProductName);
        filters.add(CTQueryFilter::FilterDate, clGexDbQuery.value("START_T").toString());

        ConsolidationTreeQueryEngine queryEng(*m_pConsolidationTree);
        // Get the test conditions
        tcList = queryEng.findTestConditions(filters);

        // Get the consolidation rules
        crList = queryEng.findConsolidationRules(filters);

        // Check if some condition rules are defined for this product
        if(crList.isEmpty())
        {
            QString strError = "No default consolidation Rule defined for this product [%1]";
            ConsolidationSummary = strError.arg(lProductName);
            bConsolidationIssue = true;
            goto labelUnlock;
        }

        // Found the PHYSICAL CONSOLIDATION RULE
        // Build the PHYSICAL CONSOLIDATION NAME
        // with TEST CONDITION LABEL
        foreach(crItem,crList)
        {
            crDataType = crItem.dataType();

            if(crDataType.toUpper() == "PHYSICAL")
            {
                // Get info about CONSOLIDATION RULE
                // ex: rule name = 'final'
                PhysicalConsolidationName = crItem.name();
                // Add TEST CONDITION name to the CONSOLIDATION name
                // ex: consolidation name = 'final[TEMP]'
                foreach(tcItem,tcList)
                {
                    tcName = tcItem.conditionName();
                    PhysicalConsolidationName += "["+tcName+"]";
                }

                mapConsolidationDataType[PhysicalConsolidationName] = crDataType;
                mapConsolidationProductionFlow[PhysicalConsolidationName] = (crItem.isProductionFlow()?"Y":"N");
                break;
            }
        }
        if(PhysicalConsolidationName.isEmpty())
        {
            QString strError = "No PHYSICAL Consolidation Rule defined for this product [%1]";
            ConsolidationSummary = strError.arg(lProductName);
            bConsolidationIssue = true;
            goto labelUnlock;
        }

        // Collect all TC with avalaible values
        // contains all needed info for Test Conditions
        // list ordered by Test Condition
        // element : field|name|value1,value2,value3
        // ex: TEMP|tst_temp|HOT,ROOM,COLD
        //     VCC|test_cod|3V,3.6V
        int         nTCNumber;
        QStringList lstTestConditions;
        // From splitlots table
        // Check if test conditions from splitlots are valid
        foreach(tcItem, tcList)
        {
            tcName = tcItem.conditionName();
            tcField = tcItem.splitlotField();
            tcListValues = tcItem.allowedValues();

            // Check if the test condition field is valid
            lQuery = "SELECT "+tcField+" FROM "+NormalizeTableName("_SPLITLOT");
            lQuery += " WHERE ";
            lQuery += "   VALID_SPLITLOT<>'N'";
            lQuery += "   AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
            lQuery += "   AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
            lQuery += "   AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
            lQuery += " ORDER BY START_T ASC";
            if(!clGexDbQuery.exec(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                bQueryIssue = true;
                goto labelUnlock;
            }

            lNewValue = tcName+"|"+tcField+"|";
            while(clGexDbQuery.next())
            {
                tcValue = clGexDbQuery.value(0).toString();

                // If already in the list, then ignore
                if(lNewValue.contains("|"+tcValue+",",Qt::CaseInsensitive)
                        || lNewValue.contains(","+tcValue+",",Qt::CaseInsensitive)
                        || lNewValue.endsWith("|"+tcValue,Qt::CaseInsensitive)
                        || lNewValue.endsWith(","+tcValue,Qt::CaseInsensitive))
                    continue;

                // Check if the test condition value is allowed
                if(!tcItem.isAllowedValue(tcValue))
                {
                    QString strError = "Test Condition value [%1] is not defined for this product [%2]";
                    ConsolidationSummary = strError.arg(tcValue,lProductName);
                    bConsolidationIssue = true;
                    goto labelUnlock;
                }

                // Then add this value to the list
                if(!lNewValue.endsWith("|")) lNewValue += ",";
                lNewValue += tcValue;
            }

            // Check if have some Test Conditions collected
            if(lNewValue.endsWith("|"))
            {
                QString strError = "No Test Condition found for this product [%1]";
                ConsolidationSummary = strError.arg(lProductName);
                bConsolidationIssue = true;
                goto labelUnlock;
            }
            lstTestConditions += lNewValue;
        }

        ///////////////////////////////
        // BEGIN FLEXIBLE CONSOLIDATION
        // DATA SAMPLES
        // Check if have some test conditions defined
        // Get the value of the current test condition
        QString strTestConditionsFields;
        QString strGroupBy;
        QString strLastBinning;


        /////////////////////////////
        // COMPUTE THE STACKED WAFER MAP WITH ALL RESULTS
        // Absolute_part_id, TestCondition, Last_binning
        // Order by date and group by test conditions

        QString lAgregateFunction;
        QString lAgregateExpression;
        QString lLastBinningExpression;

        QString strBaseTable;
        QString strConsolidationTable;
        QString strRunConsolidationTable;
        QString lQueryCreateTable;
        QStringList lSplitlots;

        // To disabled the Gap Lock on xx_run
        // we need to work on the PRIMARY KEY
        lQuery = "SELECT splitlot_id FROM "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery += " WHERE ";
        lQuery += "     S.VALID_SPLITLOT<>'N'";
        lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND S.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "     AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
        while(clGexDbQuery.Next())
        {
            lSplitlots += clGexDbQuery.value("splitlot_id").toString();
        }

        // Need to clean all the RUN CONSOLIDATION flags before
        lQuery  = "UPDATE "+NormalizeTableName("_RUN")+" R ";
        lQuery += "  SET R.intermediate=false, R.final=false ";
        lQuery += "  WHERE  ";
        lQuery += "     R.SPLITLOT_ID IN ("+lSplitlots.join(",")+")";
        // To disabled the Gap Lock on xx_run
        lQuery += "     AND R.RUN_ID>0 ";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }

        /////////////////////////
        // DEADLOCK PREVENTION
        // Commit transaction
        clGexDbQuery.Execute("COMMIT");

        // Check if have RUN results
        lQuery =  "SELECT S.splitlot_id ";
        lQuery += "  FROM ";
        lQuery += "     "+NormalizeTableName("_SPLITLOT")+ " S ";
        lQuery += "  INNER JOIN "+NormalizeTableName("_RUN")+" R ";
        lQuery += "     ON R.SPLITLOT_ID=S.SPLITLOT_ID  ";
        lQuery += "  WHERE  ";
        lQuery += "     VALID_SPLITLOT<>'N'";
        lQuery += "     AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "     AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        lQuery += " LIMIT 1";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }
        if(!clGexDbQuery.first())
        {
            // No samples
            ConsolidationSummary = "No RUN samples";
            goto labelUnlock;
        }
        // use a unique TmpTable for this session
        // MAXIMUM = 30 char
        //strConsolidationTable ="WT_WAFERMAP_"+QString::number(m_nInsertionSID)+"_PARTS";
        strConsolidationTable ="WT_WAFERMAP_PARTS";
        if(strConsolidationTable.length() > 30)
            strConsolidationTable = strConsolidationTable.left(30);

        mapConsolidationTables["ABS_PARTS"] = strConsolidationTable;

        // CREATE TEMPORARY TABLE
        // FROM ALL PARTS
        // FROM ALL WAFERS
        // FROM ALL TEST CONDITIONS
        // One line per PART_ID and the last result HBIN and SBIN

        lQueryCreateTable = "CREATE TEMPORARY TABLE %1 (";
        lQueryCreateTable+= " ABS_PART_ID VARCHAR(255) ,";
        // ADD HERE ALL TEST CONDITIONS INFO
        nTCNumber = 1;
        foreach(lNewValue, lstTestConditions)
        {
            // TEST CONDITION NAME
            lQueryCreateTable+= " TC_NAME_"+QString::number(nTCNumber)+" VARCHAR(255) ,";
            // TEST CONDITION VALUE
            lQueryCreateTable+= " TC_VALUE_"+QString::number(nTCNumber)+" VARCHAR(255) ,";
            nTCNumber++;
        }
        // LAST_BINNING ALWAYS IN THE SAME FORMAT (START_TIME+RETEST_INDEX:BIN_CAT:HBIN_NO:SBIN_NO:SPLITLOT_ID:RUN_ID)
        lQueryCreateTable+= " LAST_BINNING VARCHAR(255),";
        lQueryCreateTable+= " CONSOLIDATION_NAME VARCHAR(255),";
        lQueryCreateTable+= " CONSOLIDATION_ALGO VARCHAR(45),";
        lQueryCreateTable+= " VALID_PART CHAR(1), ";
        lQueryCreateTable+= " KEY wm_abspart (ABS_PART_ID), ";
        lQueryCreateTable+= " KEY wm_consolidation (CONSOLIDATION_NAME), ";
        lQueryCreateTable+= " KEY wm_valid (VALID_PART) ";
        lQueryCreateTable+= ")";

        // CREATE ABS_PARTS TEMPORARY TABLE
        clGexDbQuery.Execute("DROP TABLE IF EXISTS "+strConsolidationTable);
        lQuery = lQueryCreateTable.arg(strConsolidationTable);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }

        ProcessEvents();

        // case 7233: if no start_t, add 1000000000 to have the good lenght
        //STDF file:
        //* 1 WCR, 0 WIR, 0 WRR
        //=> wt_splitlot.start_t = 0 (from WIR.start_t instead of MIR.start_t)
        //=> wt_splitlot.finish_t = 0 (from WRR.finish_t instead of MRR.finish_t)
        //=> compute the string to have the last_part description doesn't have the good lenght (time:cat:hbin:sbin) and return empty string
        //* correction
        //=> use MIR.start_t instead of WIR.start_t if null
        //=> use MRR.finish_t instead of WRR.finish_t if null
        //=> add a time offset to retrieve the good size for the last_part description
        // LAST_BINNING ALWAYS IN THE SAME FORMAT (START_TIME+RETEST_INDEX:BIN_CAT:HBIN_NO:SBIN_NO:SPLITLOT_ID:RUN_ID)
        // To be able to extract with a MAX or MIN, need to write all value with the same size for the ORDER BY
        // Add the splitlot_id+run_id info
        // 1000000000:P:100000:100000:1624500002:1
        // For the splitlot+run_id, no need to ORDER BY, just add the info
        lLastBinningExpression = "CONCAT(";
        lLastBinningExpression+= "(1000000000+S.START_T+R.PART_RETEST_INDEX),";
        lLastBinningExpression+= "':',";
        lLastBinningExpression+= "(R.PART_STATUS),";
        lLastBinningExpression+= "':',";
        lLastBinningExpression+= "(100000+IFNULL(HBIN_NO,65535)),";
        lLastBinningExpression+= "':',";
        lLastBinningExpression+= "(100000+IFNULL(SBIN_NO,65535)),";
        lLastBinningExpression+= "':',";
        lLastBinningExpression+= "(R.SPLITLOT_ID),";
        lLastBinningExpression+= "':',";
        lLastBinningExpression+= "(R.RUN_ID)";
        lLastBinningExpression+= ")";


        // FOR ALL TEST CONDITIONS
        lQuery = "                 SELECT  "+mAttributes["STRAIGHT_JOIN"].toString()+" ";
        lQuery += "                 IF((PART_X IS NULL) OR (PART_Y IS NULL) ";
        lQuery += "                 , IF(PART_ID IS NULL ";
        lQuery += "                         , CONCAT(IFNULL(S.WAFER_ID,''),':',RUN_ID) ";           // no identification then RUN_ID
        lQuery += "                         , CONCAT(IFNULL(S.WAFER_ID,''),':',PART_ID))";          // only PART_ID
        lQuery += "                 , ";
        lQuery += "                         CONCAT(IFNULL(S.WAFER_ID,''),':',PART_X,':',PART_Y) )";  // PART_X AND PART_Y
        lQuery += "                 AS ABS_PART_ID, ";
        // COLLECT ALL INFO ABOUT TEST CONDITIONS
        nTCNumber = 1;
        foreach(lNewValue, lstTestConditions)
        {
            // strNewValue = TEMP|tst_temp|HOT,ROOM,COLD
            // strNewValue = VCC|test_cod|3V,3.6V
            lQuery += "'"+lNewValue.section("|",0,0)+"' AS TC_NAME_"+QString::number(nTCNumber)+",";
            lQuery += lNewValue.section("|",1,1)+" AS TC_VALUE_"+QString::number(nTCNumber)+",";
            nTCNumber++;
        }
        lQuery += "       "+lLastBinningExpression;
        lQuery += "       AS LAST_BINNING ,";
        lQuery += "       IF((R.PART_RETEST_INDEX = 0) , 'TEST' , 'RETEST' ) AS CONSOLIDATION_NAME ,";
        lQuery += "       '' AS CONSOLIDATION_ALGO ,";
        lQuery += "       IF(((HBIN_NO IS NULL) OR (SBIN_NO IS NULL)) , NULL , 'Y' ) AS VALID_PART";
        lQuery += "                 FROM ";
        lQuery += "                 "+NormalizeTableName("_SPLITLOT")+ " S ";
        lQuery += "                 LEFT OUTER JOIN "+NormalizeTableName("_RUN")+" R ";
        lQuery += "                     ON R.SPLITLOT_ID=S.SPLITLOT_ID  ";
        lQuery += "                 WHERE  ";
        lQuery += "                     VALID_SPLITLOT<>'N'";
        lQuery += "                     AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "                     AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "                     AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);

        lQuery = "INSERT INTO "+strConsolidationTable+" ("+lQuery+")";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }
        clGexDbQuery.DumpPerformance();
        ProcessEvents();

        // Delete rows having null binnings
        lQuery = "DELETE FROM " + strConsolidationTable+" WHERE VALID_PART IS NULL";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }

        // Make sure there are some rows left
        lQuery = "SELECT CONSOLIDATION_NAME FROM " + strConsolidationTable + " LIMIT 1";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }

        if(!clGexDbQuery.First())
        {
            ConsolidationSummary = "No valid parts found for LOT_ID="+LotId+", WAFER_ID="+WaferId+", PROD_DATA="+ProdData;
            GSET_ERROR1(GexDbPlugin_Base, eDB_Consolidation, NULL, ConsolidationSummary.toLatin1().constData());
            goto labelUnlock;
        }

        // CONSOLIDATION HAS SOME RESULT
        // WITH VALID_PART
        ProcessEvents();

        // FLEXIBLE CONSOLIDATION
        // FOR EACH CONSOLIDATION RULES AND TEST CONDITIONS
        // HAVE TO CREATE A TEMPORARY TABLE FOR STACKED WAFER MAP
        // FOR EACH CONSOLIDATION, HAVE TO EXTRACT THE COUNT OF PART

        // PRIMARY
        //  FROM ABS_PARTS
        //  GROUP BY TC_NAME_1, TC_NAME_2, TC_NAME_N
        // INTERMEDIATE
        //  FROM PRIMARY
        //  GROUP BY TC_NAME_1
        // PHYSICAL
        //   FROM INTERMEDIATE
        //   NO GROUP BY


        // For each rules
        foreach(crItem,crList)
        {
            crName = crItem.name();
            crBaseName = crItem.baseConsolidation();
            crGroupBy = crItem.groupedBy().toUpper();
            crAlgorithm = crItem.algorithm().toLower();
            crStoredResult = crItem.storedResult().toUpper();
            crProductionFlow = (crItem.isProductionFlow() ? 'Y' : 'N');
            crDataType = crItem.dataType();

            // Ignore Consolidation Rule with/without Test Conditions
            if((crGroupBy != "NONE") && tcList.isEmpty())
                continue;
            if((crGroupBy == "NONE") && !tcList.isEmpty())
                continue;

            // Check if the rule name not already used
            if(mapConsolidationTables.contains(crName))
            {
                QString strError = "Consolidation Rule name [%1] is not unique for this product [%2]";
                ConsolidationSummary = strError.arg(crName,lProductName);
                GSET_ERROR1(GexDbPlugin_Base, eDB_Consolidation, NULL, ConsolidationSummary.toLatin1().constData());
                bConsolidationIssue = true;
                goto labelUnlock;
            }

            if(crBaseName.isEmpty())
                crBaseName = "ABS_PARTS";

            strBaseTable = mapConsolidationTables[crBaseName];
            // Check if the baseTable exists
            lQuery = "SELECT * FROM " + strBaseTable + " LIMIT 1";
            if(!clGexDbQuery.Execute(lQuery))
            {
                QString strError = "Consolidation name [%1] not exists";
                GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL,
                            strError.arg(crBaseName).toLatin1().constData());
                bQueryIssue = true;
                goto labelUnlock;
            }


            // GENERATE THE TEMPORARY TABLE
            strConsolidationTable ="WT_WAFERMAP_"+crName.toUpper();
            strConsolidationTable = strConsolidationTable.replace(' ','_').remove(QRegExp("[&'@/{}\\\\,$=!#()%.+~ -]"));
            if(strConsolidationTable.length() > 30)
                strConsolidationTable = strConsolidationTable.left(30);

            mapConsolidationTables[crName] = strConsolidationTable;

            // CREATE TEMPORARY TABLE
            clGexDbQuery.Execute("DROP TABLE IF EXISTS "+strConsolidationTable);
            lQuery = lQueryCreateTable.arg(strConsolidationTable);
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                bQueryIssue = true;
                goto labelUnlock;
            }

            // FROM ALL PARTS
            // FROM ALL WAFERS
            // FROM ALL TEST CONDITIONS
            // One line per PART_ID and the last result HBIN and SBIN

            strTestConditionsFields = "";
            lConsolidationName = "'"+crName+"'";
            strGroupBy = " GROUP BY ABS_PART_ID";
            int nTCNumber = 1;
            foreach(lNewValue, lstTestConditions)
            {
                // strNewValue = TEMP|tst_temp|HOT,ROOM,COLD
                // strNewValue = VCC|test_cod|3V,3.6V
                if((crGroupBy != "ALL") && (crGroupBy.contains(lNewValue.section("|",0,0)) == false))
                {
                    strTestConditionsFields += "MAX(TC_NAME_"+QString::number(nTCNumber)+") AS TC_NAME_"+QString::number(nTCNumber)+",";
                    strTestConditionsFields += "MAX('') AS TC_VALUE_"+QString::number(nTCNumber)+",";
                }
                else
                {
                    strTestConditionsFields += "MAX(TC_NAME_"+QString::number(nTCNumber)+") AS TC_NAME_"+QString::number(nTCNumber)+",";
                    strTestConditionsFields += "TC_VALUE_"+QString::number(nTCNumber)+" AS TC_VALUE_"+QString::number(nTCNumber)+",";
                    strGroupBy += ",TC_VALUE_"+QString::number(nTCNumber);
                }

                if(crGroupBy == "NULL")
                    lConsolidationName += ",'[',TC_NAME_"+QString::number(nTCNumber)+",']'";
                else
                    lConsolidationName += ",'[',TC_VALUE_"+QString::number(nTCNumber)+",']'";

                nTCNumber++;
            }

            // ConsolidationName = terminal[TEMP][VCC] for terminal
            // ConsolidationName = intermediate[HOT][3V] for intermediate
            // ConsolidationName = retest when no test condition
            lConsolidationName = "CONCAT(" + lConsolidationName + ")";

            // Agregate Expression must always ending with the LAST_BINNING field
            // LAST_BINNING ALWAYS IN THE SAME FORMAT (START_TIME+RETEST_INDEX:BIN_CAT:HBIN_NO:SBIN_NO:SPLITLOT_ID:RUN_ID)
            // START_T = SUBSTRING_INDEX(LAST_BINNING,':',1)
            // BIN_CAT = SUBSTRING_INDEX(SUBSTRING_INDEX(LAST_BINNING,':',2),':',-1)
            // HBIN_NO = SUBSTRING_INDEX(SUBSTRING_INDEX(LAST_BINNING,':',3),':',-1)
            // SBIN_NO = SUBSTRING_INDEX(SUBSTRING_INDEX(LAST_BINNING,':',4),':',-1)
            if(crAlgorithm.startsWith("highest_bin", Qt::CaseInsensitive))
            {
                // Append the SBIN_NO and MAX
                lAgregateFunction = "MAX";
                lAgregateExpression = "CONCAT(";
                lAgregateExpression+= "SUBSTRING_INDEX(SUBSTRING_INDEX(LAST_BINNING,':',4),':',-1),";
                lAgregateExpression+= "':',";
                // If several, take the first
                lAgregateExpression+= "10000000000-SUBSTRING_INDEX(LAST_BINNING,':',1),";
                lAgregateExpression+= "'#',";
                lAgregateExpression+= "LAST_BINNING";
                lAgregateExpression+= ")";
            }
            else if(crAlgorithm.startsWith("lowest_bin", Qt::CaseInsensitive))
            {
                // Append the SBIN_NO and MIN
                lAgregateFunction = "MIN";
                lAgregateExpression = "CONCAT(";
                lAgregateExpression+= "SUBSTRING_INDEX(SUBSTRING_INDEX(LAST_BINNING,':',4),':',-1),";
                lAgregateExpression+= "':',";
                // If several, take the first
                lAgregateExpression+= "SUBSTRING_INDEX(LAST_BINNING,':',1),";
                lAgregateExpression+= "'#',";
                lAgregateExpression+= "LAST_BINNING";
                lAgregateExpression+= ")";
            }
            else if(crAlgorithm.startsWith("last_pass", Qt::CaseInsensitive))
            {
                // Append the BIN_CAT and MAX
                lAgregateFunction = "MAX";
                lAgregateExpression = "CONCAT(";
                lAgregateExpression+= "SUBSTRING_INDEX(SUBSTRING_INDEX(LAST_BINNING,':',2),':',-1),";
                lAgregateExpression+= "':',";
                // If several, take the last
                lAgregateExpression+= "SUBSTRING_INDEX(LAST_BINNING,':',1),";
                lAgregateExpression+= "'#',";
                lAgregateExpression+= "LAST_BINNING";
                lAgregateExpression+= ")";
            }
            else if(crAlgorithm.startsWith("first_pass", Qt::CaseInsensitive))
            {
                // Append the BIN_CAT and MIN
                lAgregateFunction = "MIN";
                lAgregateExpression = "CONCAT(";
                lAgregateExpression+= "(IF(SUBSTRING_INDEX(SUBSTRING_INDEX(LAST_BINNING,':',2),':',-1) = 'P',0,9)),";
                lAgregateExpression+= "':',";
                // If several, take the first
                lAgregateExpression+= "10000000000-SUBSTRING_INDEX(LAST_BINNING,':',1),";
                lAgregateExpression+= "'#',";
                lAgregateExpression+= "LAST_BINNING";
                lAgregateExpression+= ")";
            }
            else if(crAlgorithm.startsWith("last_fail", Qt::CaseInsensitive))
            {
                // Append the BIN_CAT and MAX
                lAgregateFunction = "MAX";
                lAgregateExpression = "CONCAT(";
                lAgregateExpression+= "(IF(SUBSTRING_INDEX(SUBSTRING_INDEX(LAST_BINNING,':',2),':',-1) = 'P',0,9)),";
                lAgregateExpression+= "':',";
                // If several, take the last
                lAgregateExpression+= "SUBSTRING_INDEX(LAST_BINNING,':',1),";
                lAgregateExpression+= "'#',";
                lAgregateExpression+= "LAST_BINNING";
                lAgregateExpression+= ")";
            }
            else if(crAlgorithm.startsWith("first_fail", Qt::CaseInsensitive))
            {
                // Append the BIN_CAT and MIN
                lAgregateFunction = "MIN";
                lAgregateExpression = "CONCAT(";
                lAgregateExpression+= "(SUBSTR(SUBSTRING_INDEX(LAST_BINNING,':',2),-1)),";
                lAgregateExpression+= "':',";
                // If several, take the first
                lAgregateExpression+= "10000000000-SUBSTRING_INDEX(LAST_BINNING,':',1),";
                lAgregateExpression+= "'#',";
                lAgregateExpression+= "LAST_BINNING";
                lAgregateExpression+= ")";
            }
            else if(crAlgorithm.startsWith("last_bin", Qt::CaseInsensitive))
            {
                // Only on the SPLITLOT_ID+PART_RETEST_INDEX and MAX
                lAgregateFunction = "MAX";
                lAgregateExpression = "LAST_BINNING";
            }
            else if(crAlgorithm.startsWith("first_bin", Qt::CaseInsensitive))
            {
                // Only on the SPLITLOT_ID+PART_RETEST_INDEX and MIN
                lAgregateFunction = "MIN";
                lAgregateExpression = "LAST_BINNING";
            }
            else
            {
                // last_bin
                lAgregateFunction = "MAX";
                lAgregateExpression = "LAST_BINNING";
            }

            strLastBinning = lAgregateFunction+"("+lAgregateExpression+")";

            lQuery = "SELECT ";
            lQuery+= "        ABS_PART_ID, ";
            lQuery+= strTestConditionsFields;
            lQuery+= "        SUBSTRING_INDEX("+strLastBinning+",'#',-1) AS LAST_BINNING, ";
            lQuery+= "        MAX("+lConsolidationName+") AS CONSOLIDATION_NAME, ";
            lQuery+= "        '"+crAlgorithm+"' AS CONSOLIDATION_ALGO, ";
            lQuery+= "        MAX('Y') AS VALID_PART ";
            lQuery+= " FROM "+strBaseTable;
            lQuery+= strGroupBy;

            lQuery = "INSERT INTO "+strConsolidationTable+" ("+lQuery+")";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                bQueryIssue = true;
                goto labelUnlock;
            }
            clGexDbQuery.DumpPerformance();

            // Update the RUN CONSOLIDATION flags
            // Probaly with a link with the splitlot table
            // only for intermediate consolidation
            // For final consolidation, flag the splitlot_id,run_id

            // Generate a new table to extract and link the splitlot_id
            strRunConsolidationTable = NormalizeTableName("_RUN_CONSOLIDATION_"+crDataType.toUpper());
            // For the auto DROP at the end
            mapConsolidationTables["RUN_"+crDataType.toUpper()] = strRunConsolidationTable;
            clGexDbQuery.exec("DROP TABLE IF EXISTS "+strRunConsolidationTable);
            lQuery = "CREATE TEMPORARY TABLE "+strRunConsolidationTable+" ("
                     " SPLITLOT_ID INT(10) unsigned, "
                     " RUN_ID MEDIUMINT(7) unsigned, "
                     " CONSOLIDATION_NAME VARCHAR(255), "
                     " CONSOLIDATION_ALGO VARCHAR(45), "
                     " PRIMARY KEY (SPLITLOT_ID, RUN_ID)"
                     ")";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                bQueryIssue = true;
                goto labelUnlock;
            }
            lQuery = "INSERT INTO "+strRunConsolidationTable+" "
                     " SELECT SUBSTRING_INDEX(SUBSTRING_INDEX(CT.LAST_BINNING,':',-2),':',1) AS SPLITLOT_ID, "
                     " SUBSTRING_INDEX(CT.LAST_BINNING,':',-1) AS RUN_ID , "
                     " CONSOLIDATION_NAME, "
                     " CONSOLIDATION_ALGO "
                     " FROM "+strConsolidationTable+ " CT ";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                bQueryIssue = true;
                goto labelUnlock;
            }

            // Update each INTERMEDIATE
            // But also the PHYSICAL if no INTERMEDIATE
            if((crDataType.toUpper() != "PHYSICAL")
                    || (crList.size() == 1))
            {
                lQuery = "INSERT INTO "+NormalizeTableName("_RUN_ CONSOLIDATION")+" ";
                lQuery += "   (CONSOLIDATION_NAME,CONSOLIDATION_FLOW,SPLITLOT_ID,RUN_ID,FINAL) ";
                lQuery += " SELECT CONSOLIDATION_NAME, ";
                lQuery += TranslateStringToSqlVarChar(lConsolidationFlow)+" AS CONSOLIDATION_FLOW, ";
                lQuery += " SPLITLOT_ID, ";
                lQuery += " RUN_ID, ";
                lQuery += " false AS FINAL ";
                lQuery += " FROM "+strRunConsolidationTable;

                lQuery = "UPDATE "+NormalizeTableName("_RUN")+" R ";
                lQuery += "   INNER JOIN "+strRunConsolidationTable+" RC ";
                lQuery += " ON R.SPLITLOT_ID=RC.SPLITLOT_ID AND R.RUN_ID=RC.RUN_ID ";
                lQuery += " SET R.intermediate=true, R.final=false ";
                if(!clGexDbQuery.Execute(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                    bQueryIssue = true;
                    goto labelUnlock;
                }

                // BECAUSE WE HAVE THE CONSOLIDATION TREE
                // Then update the SPLITLOT/WAFER table for TEST_INSERTION, RETEST_HBINS, CONSOLIDATION_ALGO
                lQuery = "UPDATE "+NormalizeTableName("_SPLITLOT")+" S "
                        " INNER JOIN "+strRunConsolidationTable+" RC "
                        "   ON S.SPLITLOT_ID=RC.SPLITLOT_ID "
                        " SET S.TEST_INSERTION=RC.CONSOLIDATION_NAME, "
                        " S.RETEST_HBINS=RC.CONSOLIDATION_ALGO ";
                if(!clGexDbQuery.Execute(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                    bQueryIssue = true;
                    goto labelUnlock;
                }
            }
            if(crDataType.toUpper() == "PHYSICAL")
            {
                // Update the FINAL flags from FINAL consolidation
                // Need to create a TEMPORARY table with INDEX
                lQuery  = "UPDATE "+NormalizeTableName("_RUN")+" R ";
                lQuery += "  INNER JOIN "+strRunConsolidationTable+" RC ";
                lQuery += "     ON R.SPLITLOT_ID=RC.SPLITLOT_ID  ";
                lQuery += "     AND R.RUN_ID=RC.RUN_ID   ";
                lQuery += " SET R.FINAL=true ";
                if(!clGexDbQuery.Execute(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                    bQueryIssue = true;
                    goto labelUnlock;
                }

                // BECAUSE WE HAVE THE CONSOLIDATION TREE
                // Then update the SPLITLOT/WAFER table for TEST_INSERTION, RETEST_HBINS, CONSOLIDATION_ALGO
                lQuery = "UPDATE "+NormalizeTableName("_WAFER_INFO")+" W "
                        "   SET W.CONSOLIDATION_ALGO='"+crAlgorithm+"' "
                        " WHERE W.LOT_ID="+TranslateStringToSqlVarChar(LotId)+
                        "  AND W.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
                if(!clGexDbQuery.Execute(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                    bQueryIssue = true;
                    goto labelUnlock;
                }
            }

            /////////////////////////
            // DEADLOCK PREVENTION
            // Commit transaction
            clGexDbQuery.Execute("COMMIT");
        }
        // END OF CONSOLIDATION PHASE
    }

    // for ElectTest
    if(m_eTestingStage == eElectTest)
    {
        // No retest in ETEST
        QStringList lSplitlots;

        // To disabled the Gap Lock on xx_run
        // we need to work on the PRIMARY KEY
        lQuery = "SELECT splitlot_id FROM "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery += " WHERE ";
        lQuery += "     S.VALID_SPLITLOT<>'N'";
        lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND S.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "     AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
        while(clGexDbQuery.Next())
        {
            lSplitlots += clGexDbQuery.value("splitlot_id").toString();
        }
        // Just flag the last splitlot id into thr RUN CONSOLIDATION flags
        // First clean the RUN CONSOLIDATION flags
        lQuery  = "UPDATE "+NormalizeTableName("_RUN")+" R ";
        lQuery += "  SET R.intermediate=false, R.final=false  ";
        lQuery += "  WHERE  ";
        lQuery += "     R.SPLITLOT_ID IN ("+lSplitlots.join(",")+")";
        // To disabled the Gap Lock on xx_run
        lQuery += "     AND R.RUN_ID>0 ";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }

        QString lLastSplitlot;
        QString lConsolidationName;
        QString lConsolidationFlow;
        //Take ONLY the last splitlot_id
        // No consolidation
        // The last replace the initial
        lQuery = "SELECT SPLITLOT_ID, TEST_INSERTION, TEST_FLOW ";
        lQuery += " FROM "+NormalizeTableName("_SPLITLOT");
        lQuery += " WHERE ";
        lQuery += "   VALID_SPLITLOT<>'N'";
        lQuery += "   AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "   AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "   AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        lQuery += " ORDER BY START_T";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }
        while(clGexDbQuery.Next())
        {
            lLastSplitlot = clGexDbQuery.value("SPLITLOT_ID").toString();
            lConsolidationName = clGexDbQuery.value("TEST_INSERTION").toString();
            lConsolidationFlow = clGexDbQuery.value("TEST_FLOW").toString();
        }

        // Check if have RUN results
        lQuery =  "SELECT S.splitlot_id ";
        lQuery += "  FROM ";
        lQuery += "     "+NormalizeTableName("_SPLITLOT")+ " S ";
        lQuery += "  INNER JOIN "+NormalizeTableName("_RUN")+" R ";
        lQuery += "     ON R.SPLITLOT_ID=S.SPLITLOT_ID  ";
        lQuery += "  WHERE  ";
        lQuery += "     VALID_SPLITLOT<>'N'";
        lQuery += "     AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "     AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        lQuery += " LIMIT 1";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }
        if(!clGexDbQuery.first())
        {
            // No samples
            ConsolidationSummary = "No RUN samples";
            goto labelUnlock;
        }

        lQuery = "UPDATE "+NormalizeTableName("_RUN")+" R ";
        lQuery+= " SET R.intermediate=true, R.final=true ";
        lQuery+= " WHERE R.SPLITLOT_ID="+lLastSplitlot;
        // To disabled the Gap Lock on xx_run
        lQuery+= " AND R.RUN_ID>0 ";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }

        /////////////////////////
        // DEADLOCK PREVENTION
        // Commit transaction
        clGexDbQuery.Execute("COMMIT");

        // BECAUSE WE HAVE THE new CONSOLIDATION Config
        // Then update the SPLITLOT/WAFER table for TEST_INSERTION, RETEST_HBINS, CONSOLIDATION_ALGO
        lQuery = "UPDATE "+NormalizeTableName("_WAFER_INFO")+" W "
                "   SET W.CONSOLIDATION_ALGO='last_bin' "
                " WHERE W.LOT_ID="+TranslateStringToSqlVarChar(LotId)+
                "  AND W.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            bQueryIssue = true;
            goto labelUnlock;
        }


        PhysicalConsolidationName = "FINAL";
    }

    bQueryIssue = false;

    // Debug message
    WriteDebugMessageFile("GexDbPlugin_Galaxy::UpdateWaferRunConsolidationTable() : Consolidation completed");

labelUnlock:

    TimersTraceStop("Function Wafer Run Consolidation");
    TimersTraceStop("Function Wafer Run Consolidation: consolidation phase");

    // DROP TEMPORARY TABLE IF ANY
    while(!mapConsolidationTables.isEmpty())
    {
        lNewValue = mapConsolidationTables.begin().value();
        mapConsolidationTables.erase(mapConsolidationTables.begin());

        if(!lNewValue.isEmpty())
            clGexDbQuery.exec("DROP TABLE IF EXISTS "+lNewValue);
    }

    return !bQueryIssue && !bConsolidationIssue;
}

///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateWaferConsolidation
// For consolidation phase
// For all PROD_DATA flow
///////////////////////////////////////////////////////////
////// \brief GexDbPlugin_Galaxy::UpdateWaferConsolidation
////// \param strLotId
////// \param strWaferId
////// \return
bool GexDbPlugin_Galaxy::UpdateWaferConsolidation(const QString &LotId, const QString &WaferId)
{

    if(m_eTestingStage == eFinalTest)
        return true;

    // Clear Warning messages
    m_nInsertionSID = m_pclDatabaseConnector->GetConnectionSID();
    m_strLotId = LotId;
    m_strSubLotId = "";
    m_strWaferId = WaferId;
    m_strlWarnings.clear();

    // Debug message
    QString strMessage = "---- GexDbPlugin_Galaxy::UpdateWaferConsolidation() : Consolidation process for table ";
    strMessage += NormalizeTableName("_SPLITLOT");
    WriteDebugMessageFile(strMessage);

    bool       bStatus = false;
    QString    lWaferInfoTable;

    QString             lQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    clGexDbQuery.setForwardOnly(true);
    lWaferInfoTable = NormalizeTableName("_WAFER_INFO");

    // have WRR and WCR ?
    int nFlags        = 0;
    int nNbGrossDie   = 0;
//    int nWaferNb      = -1;

    // For Test Conditions and Consolidation Rules
    QString                lPhysicalConsolidationName;
    QString                lConsolidationSummary;
    QString                lProdData;
    int                    lNumberProdData;
    QStringList            lProdDataForConsolidation;
    bool                   lUpdateTableForPhysicalData = false;
    int                    lNumOfRows = 0;
    QStringList            lMatchedTestFlows;
    QStringList            lMatchedSplitlots;

    TimersTraceStart("Function Wafer Consolidation");


    /////////////////////////
    // DEADLOCK PREVENTION
    // Clean the Token table if needed
    if (! InitTokens(NormalizeTableName("_CONSOLIDATION")))
    {
        return false;
    }
    // Consolidation through Incremental Update
    // Fix collision on same Lot
    QString lTokenKey = LotId+"|"+WaferId;
    if(!GetToken(NormalizeTableName("_CONSOLIDATION"),lTokenKey,15))
    {
        // Reject the current consolidation
        bStatus = false;
        goto labelUnlock;
    }

    ///////////////////////////////////////////////////////////
    // CHECK IF AT LEAST ONE SPLITLOT WAS FLAGED WITH BINNING_CONSOLIDATION
    ///////////////////////////////////////////////////////////
    // PROD_DATA INFO
    // PROD_DATA, START_TIME, SAMPLES_FLAGS
    // FROM SPLITLOT(LOT/WAFER)
    ///////////////////////////////////////////////////////////
    // FOR ALL TEST CONDITIONS
    // Upate lot information with data
    // from production data if have
    // else from non-production data

    // Construct the query condition to select only wafer with data samples
    lNumberProdData = 0;

    // Check if have to use SUMMARY
    // Check if have
    lQuery = "SELECT distinct PROD_DATA, TEST_FLOW, ";
    lQuery += " MAX(INCREMENTAL_UPDATE = 'BINNING_CONSOLIDATION' ";         // Extact match
    lQuery += " OR INCREMENTAL_UPDATE LIKE 'BINNING_CONSOLIDATION|%' ";     // Starts with
    lQuery += " OR INCREMENTAL_UPDATE LIKE '%|BINNING_CONSOLIDATION|%' ";   // Contains
    lQuery += " OR INCREMENTAL_UPDATE LIKE '%|BINNING_CONSOLIDATION') ";    // Ends with
    lQuery += " AS FOR_CONSOLIDATION ";
    lQuery += " FROM "+NormalizeTableName("_SPLITLOT");
    lQuery += " WHERE ";
    lQuery += "   VALID_SPLITLOT<>'N'";
    lQuery += "   AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "   AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    while(clGexDbQuery.Next())
    {
        ++lNumOfRows;

        // No consolidation for TEST_FLOW empty
        if(clGexDbQuery.value("TEST_FLOW").toString().isEmpty())
            continue;

        lNumberProdData++;
        lProdData = clGexDbQuery.value("PROD_DATA").toString();
        if(clGexDbQuery.value("FOR_CONSOLIDATION").toInt() == 1)
        {
            lProdDataForConsolidation += lProdData;
        }
    }
    // For E-Test, no CONSOLIDATION table
    // Just do the 'Y'
    if(m_eTestingStage == eElectTest)
    {
        if(lNumberProdData > 1)
        {
            lNumberProdData = 1;
            lProdDataForConsolidation.removeOne("N");
        }
        lUpdateTableForPhysicalData = true;
    }

    if(lProdDataForConsolidation.isEmpty())
    {
        // NO SPLITLOT FLAGGED WITH BINNING_CONSOLIDATON
        // Do nothing
        QString lMsg;
        if(lNumOfRows == 0)
            lMsg = QString("No VAILD splitlots for Lot[%1]Wafer[%2] - No consolidation needed")
                    .arg(LotId).arg(WaferId);
        else if(lNumberProdData == 0)
            lMsg = QString("No TEST_FLOW for Lot[%1]Wafer[%2] - Consolidation disabled")
                    .arg(LotId).arg(WaferId);
        else
            lMsg = QString("No BINNING_CONSOLIDATION flag for Lot[%1]Wafer[%2] - No consolidation needed")
                    .arg(LotId).arg(WaferId);
        WarningMessage(lMsg);

        bStatus = true;
        goto labelUnlock;
    }

    // If 2 PROD_DATA for consolidation
    // We need to do the 2 consolidation in one pass
    // After the update, the incremental update process will remove the BINNING_CONSOLIDATION flag for ALL
    if(lProdDataForConsolidation.contains("Y"))
    {
        lUpdateTableForPhysicalData = true;
    }

    bStatus = false;
    ///////////////////////////////////////////////////////////
    // FOR ALL CONDITIONS
    // First, check if have this wafer already in the DB
    // And LOCK WAFER_INFO(LOT/WAFER) FOR TRANSACTION
    ///////////////////////////////////////////////////////////
    lQuery =  "SELECT GROSS_DIE, WAFER_NB, WAFER_FLAGS";
    lQuery += " FROM "+lWaferInfoTable;
    lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += " AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    ///////////////////////////////////////////////////////////
    // GROSS_DIE,WAFER_NB AND CONSOLIDATION_PERIOD
    // FROM WAFER_INFO(LOT/WAFER)
    ///////////////////////////////////////////////////////////
    if(!clGexDbQuery.First())
    {
        // Insertion process
        // must have one element
        GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, lQuery.toLatin1().constData());
        goto labelUnlock;
    }
    else
    {
        // Case 6300 - gross_die issue
        // impacted wafer inserted with V6.5
        // corrupted gross_die are fixed with a script

        // GrossDie management
        // There are 2 possible cases :
        //  1- Consolidation during insertion
        //  2- Consolidation by BINNING_CONSOLIDATION
        // 1- Consolidation during insertion
        // if wt_wafer_info.wafer_flat & FLAG_WAFERINFO_GROSSDIEOVERLOADED
        //   a* wt_wafer_info.gross_die = USER_GROSS_DIE
        // else
        //   b* wt_wafer_info.gross_die = nb_parts
        // a* do not update the gross_die OR update with the KeyContent USER_GROSS_DIE
        // b* always update the gross_die
        // 2- Consolidation by BINNING_CONSOLIDATION
        // Never update the gross_die (NOTHING TODO)

        // Get GROSS_DIE from DB
        nNbGrossDie = clGexDbQuery.value("GROSS_DIE").toInt();
        nFlags = clGexDbQuery.value("WAFER_FLAGS").toInt();

        // GCORE-6541
        // Then check the flag from the DB
        // If the Gross die wasn't overloaded by the user
        // It must represent the consolidated number of parts
        // => Gross die = nb_parts
        if(!(nFlags & FLAG_WAFERINFO_GROSSDIEOVERLOADED))
        {
            // Reset the gross_die
            nNbGrossDie = 0;
        }
    }

    ///////////////////////////////////////
    // START CONSOLIDATION NOW
    ///////////////////////////////////////
    // For each PROD_DATA for CONSOLIDATION
    if(lUpdateTableForPhysicalData)
    {
        // RESET the PHYSICAL CONSOLIDATION STATUS
        lQuery = "UPDATE "+lWaferInfoTable;
        lQuery+= " SET ";
        lQuery+= " NB_PARTS=0,NB_PARTS_GOOD=0";
        //Force the re-compute of the groos_die
        if(nNbGrossDie==0)
            lQuery+= " ,GROSS_DIE=0";
        lQuery+= " ,CONSOLIDATION_STATUS=null";
        lQuery+= " ,CONSOLIDATION_SUMMARY='IN PROGESS'";
        lQuery+= " ,CONSOLIDATION_DATE=now()";
        lQuery+= " WHERE ";
        lQuery+= "  LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery+= "  AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        ////////////////////////////////////////////
        // DELETE ALL PHYSICAL CONSOLIDATED DATA
        lQuery = "DELETE FROM %1";
        lQuery += " WHERE ";
        lQuery += "     LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_WAFER_HBIN"))))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
        if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_WAFER_SBIN"))))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        ///////////////////////////////////////
        // RESET CONSOLIDATION COUNT FOR LOT LEVEL
        lQuery = "UPDATE "+ NormalizeTableName("_LOT");
        lQuery+= " SET NB_PARTS=0,NB_PARTS_GOOD=0";
        lQuery+= " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
    }

    // To disabled the Gap Lock on xx_splitlot
    // we need to work on the PRIMARY KEY
    // not possible in this case
    lQuery = "SELECT distinct S.TEST_FLOW FROM "+ NormalizeTableName("_SPLITLOT") +" S ";
    lQuery += " WHERE ";
    lQuery += "     S.VALID_SPLITLOT<>'N'";
    lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "     AND S.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
    // Reset all PROD_DATA if only ONE PROD_DATA
    if(lNumberProdData > 1 && (lProdDataForConsolidation.size() == 1))
        lQuery += " AND S.PROD_DATA="+TranslateStringToSqlVarChar(lProdData);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    while(clGexDbQuery.Next())
    {
        lMatchedTestFlows += clGexDbQuery.value("TEST_FLOW").toString();
    }

    // DELETE ALL INTERMEDIATE CONSOLIDATED DATA
    lQuery = "DELETE FROM %1 ";
    lQuery += " WHERE ";
    lQuery += "     LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "     AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
    lQuery += "     AND CONSOLIDATION_FLOW IN ('"+lMatchedTestFlows.join("','")+"')";
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_WAFER_CONSOLIDATION"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_WAFER_CONSOLIDATION_INTER"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_WAFER_HBIN_INTER"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    if(!clGexDbQuery.Execute(lQuery.arg(NormalizeTableName("_WAFER_SBIN_INTER"))))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, clGexDbQuery.lastQuery().toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }

    ///////////////////////////////////////
    // UPDATE THE RUN_CONSOLIDATON TABLE
    // For all cases
    //  * Consolidation from samples
    //  * Consolidation from summary
    // For each PROD_DATA
    // Create the RunConsolidation
    // And Compute the Binning Consolidation
    // Then extract the new PHYSICAL consolidation from WAFER_HBIN table
    // For the update of the WAFER_INFO table
    for(int i=0; i< lProdDataForConsolidation.size(); ++i)
    {
        lProdData = lProdDataForConsolidation.at(i);
        // Will fill the RUN CONSOLIDATION flags
        // Will fill the XX_WAFER_INFO
        // Will fill the XX_WAFER_HBIN for PROD_DATA='Y'
        // Will fill the XX_WAFER_CONSOLIDATION for all PROD_DATA
        if(!UpdateWaferBinningConsolidationTable(LotId, WaferId,lProdData, lPhysicalConsolidationName, lConsolidationSummary))
            goto labelUnlock;
    }

    bStatus = true;
    // Debug message
    WriteDebugMessageFile("GexDbPlugin_Galaxy::UpdateWaferConsolidation() : Consolidation completed");

labelUnlock:

    TimersTraceStop("Function Wafer Consolidation");

    // Display or Propagate the Error message
    if(!bStatus)
    {
        if(lConsolidationSummary.isEmpty())
        {
            lConsolidationSummary = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        }
        if(!lConsolidationSummary.isEmpty())
        {
            WarningMessage(lConsolidationSummary);
        }
    }
    if(!m_strlWarnings.isEmpty()
            && lConsolidationSummary.isEmpty())
    {
        lConsolidationSummary = m_strlWarnings.last();
    }

    QString lStatus = "'F'";
    if(bStatus)
    {
        lStatus = "'P'";
    }
    QString lSummary = TranslateStringToSqlVarChar(lConsolidationSummary);
    if(lConsolidationSummary.isEmpty())
    {
        lSummary= "null";
    }
    lQuery =  "UPDATE "+lWaferInfoTable;
    lQuery += "    SET CONSOLIDATION_STATUS="+lStatus;
    lQuery+= "      ,CONSOLIDATION_SUMMARY="+lSummary;
    lQuery += "     ,CONSOLIDATION_DATE=now() ";
    lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "     AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        bStatus = false;
    }

    /////////////////////////
    // DEADLOCK PREVENTION
    // Commit transaction
    clGexDbQuery.Execute("COMMIT");

    // Release TOKEN on WT_CONSOLIDATION keys
    ReleaseTokens();

    WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::UpdateWaferConsolidation: end. status = %1").arg(bStatus?"true":"false"));
    return bStatus;
}


///////////////////////////////////////////////////////////
// Update GALAXY database : UpdateWaferConsolidation
// For consolidation phase
// For all PROD_DATA flow
///////////////////////////////////////////////////////////
////// \brief GexDbPlugin_Galaxy::UpdateWaferBinningConsolidationTable
////// \param LotId
////// \param WaferId
////// \param ProdData
////// \param PhysicalConsolidationName : give the PHYSICAL Consolidation name (empty if error)
////// \param ConsolidationSummary : Error Consolidation message if any
////// \return
bool GexDbPlugin_Galaxy::UpdateWaferBinningConsolidationTable(const QString &LotId, const QString &WaferId, const QString &ProdData, QString &PhysicalConsolidationName, QString &ConsolidationSummary)
{

    if(m_eTestingStage == eFinalTest)
        return true;

    bool       bStatus = false;
    QString    lQueryHeader;           // INSERT INTO ...
    QString    lNewValue;              // NEW VALUE FOR INSERTION
    QString    lAllValues;             // MULTI QUERIES

    QString             lQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    clGexDbQuery.setForwardOnly(true);

    int nCounts      = 0;
    int nNbParts      = 0;
    int nNbPartsGood  = 0;

    // WAFER CONSOLIDATION
    // For HBin and SBin consolidation per wafer
    QMap<int,int>          mapHBin, mapSBin;
    QMap<int,structBinInfo>::Iterator itBinInfo;

    // For Test Conditions and Consolidation Rules
    QString                lTestFlow;
    QString                lSamplesData;
    bool                   lConsolidationFromSamples = true;
    QString                lFirstSplitlot;
    QString                lLastSplitlot;
    QStringList            lSplitlots;

    WriteDebugMessageFile("Wafer Binning consolidation: Consolidation process for BINNING tables.");
    TimersTraceStart("Function Wafer Binning Consolidation");

    ///////////////////////////////////////////////////////////
    // CHECK IF AT LEAST ONE SPLITLOT WAS FLAGED WITH BINNING_CONSOLIDATION
    ///////////////////////////////////////////////////////////
    // PROD_DATA INFO
    // PROD_DATA, START_TIME, SAMPLES_FLAGS
    // FROM SPLITLOT(LOT/WAFER)
    ///////////////////////////////////////////////////////////
    // FOR ALL TEST CONDITIONS
    // Upate lot information with data
    // from production data if have
    // else from non-production data

    // Construct the query condition to select only wafer with data samples
    // WT_SPLITLOT.splitlot_flags & Bit2(0x04)
    lSamplesData = "(splitlot_flags & "+QString::number(FLAG_SPLITLOT_PARTSFROMSAMPLES)+")";

    // Check if have to use SUMMARY
    // Check if have
    lQuery = "SELECT ";
    lQuery += " MAX("+lSamplesData+") AS SAMPLES_DATA, ";
    lQuery += " MAX(TEST_FLOW) AS TEST_FLOW, ";
    lQuery += " SUBSTRING_INDEX(MIN(CONCAT(START_T,':',SPLITLOT_ID)),':',-1) AS FIRST_SPLITLOT, ";
    lQuery += " SUBSTRING_INDEX(MAX(CONCAT(START_T,':',SPLITLOT_ID)),':',-1) AS LAST_SPLITLOT ";
    lQuery += " FROM "+NormalizeTableName("_SPLITLOT");
    lQuery += " WHERE ";
    lQuery += "   VALID_SPLITLOT<>'N'";
    lQuery += "   AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
    lQuery += "   AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
    lQuery += "   AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
    if(!clGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelUnlock;
    }
    clGexDbQuery.First();
    lTestFlow = clGexDbQuery.value("TEST_FLOW").toString();
    lConsolidationFromSamples = !(clGexDbQuery.value("SAMPLES_DATA").toInt() == 0);
    lFirstSplitlot = clGexDbQuery.value("FIRST_SPLITLOT").toString();
    lLastSplitlot = clGexDbQuery.value("LAST_SPLITLOT").toString();

    bStatus = false;

    // For Elect Test
    // always use Summary if exist
    if(m_eTestingStage == eElectTest)
    {
        // !!!!!!!!!!!!!!!!!!!!!!!
        // USE SUMMARY FOR BINNING INFO
        // BUT SPLITLOT ARE FLAGGED AS SAMPLES !!!!
        // do not use strSamplesData
        // !!!!!!!!!!!!!!!!!!!!!!!
        lConsolidationFromSamples = false;
    }

    ///////////////////////////////////////
    // START CONSOLIDATION NOW
    ///////////////////////////////////////

    ///////////////////////////////////////
    // UPDATE THE RUN_CONSOLIDATON TABLE
    // For all cases
    //  * Consolidation from samples
    //  * Consolidation from summary
    if(!UpdateWaferRunConsolidationTable(LotId, WaferId, ProdData,PhysicalConsolidationName, ConsolidationSummary))
    {
        // Consolidation configuration issue
        goto labelUnlock;
    }
    if(PhysicalConsolidationName.isEmpty())
    {
        // Consolidation configuration issue
        goto labelUnlock;
    }

    if(m_eTestingStage == eWaferTest)
    {
        // For STM_PATMAN
        // wt_sbin and wt_sbin_stats_samples was updated by the call of the stored procedure wt_insertion_postprocessing()
        // For all other cases, we have to check if the good counts comes from samples or summary

        // GET THE LIST OF ALL HBIN AND SBIN FROM SPLITLOT
        int     nBinNo;
        QString lBinCat;
        QString lBinName;

        ///////////////////////////////////////////////////////////
        // Wafer Binning Consolidation
        // COLLECT HBIN NUM,NAME,CAT FROM PREVIOUS INSERTION
        // COLLECT SBIN NUM,NAME,CAT FROM PREVIOUS INSERTION
        ///////////////////////////////////////////////////////////
        QMap<int,structBinInfo> mapHBinInfo;
        QMap<int,structBinInfo> mapSBinInfo;


        // For one PROD_DATA referenced for CONSOLIDATION process
        // STORE THE RESULTS FOR EACH CONSOLIDATION_NAME
        QString                       lConsolidationName;
        QString                       lConsolidationType;
        QMap<QString, QMap<int,int> > mapConsolidationHBin;
        QMap<QString, QMap<int,int> > mapConsolidationSBin;
        QMap<QString, int >           mapConsolidationNbParts;
        QMap<QString, int >           mapConsolidationNbPartsGood;

        if(!lConsolidationFromSamples )
        {
            // && (strRetestBinList.isEmpty()) // already tested
            // then get BINNING INFO FROM the last splitlot

            ConsolidationSummary = "Consolidation from summary";

            ///////////////////////////////////////////////////////////
            // BINS INFO FROM SUMMARY
            ///////////////////////////////////////////////////////////

            ///////////////////////////////////////////////////////////
            // Get the Last Splitlot_id for summary
            // FROM SPLITLOT(LOT/WAFER/PROD_DATA/FLAGS)
            ///////////////////////////////////////////////////////////

            ///////////////////////////////////////////////////////////
            // Only update the WAFER_INFO table with additionnal value
            // For NbParts and NbPartsGood
            // Collect info from WAFER_INFO on old result
            // Update mapHbinInfo and mapSBinInfo with previous info from SUMMARY
            // FROM HBIN_SUMMARY(LastSplitlot)
            // For HBIN
            lQuery = " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" B.HBIN_NO, B.HBIN_CAT, B.HBIN_NAME, ";
            lQuery += " S.SITE_NO, S.BIN_COUNT ";
            lQuery += " FROM  "+NormalizeTableName("_HBIN")+" B ";
            lQuery += " INNER JOIN "+NormalizeTableName("_HBIN_STATS_SUMMARY")+" S ";
            lQuery += "     ON B.SPLITLOT_ID=S.SPLITLOT_ID AND B.HBIN_NO=S.HBIN_NO";
            lQuery += " WHERE ";
            lQuery += "     B.SPLITLOT_ID="+lLastSplitlot;
            lQuery += "     AND S.SITE_NO=-1";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }

            while(clGexDbQuery.Next())
            {
                // update values for Bin info
                nBinNo = clGexDbQuery.value("HBIN_NO").toInt();
                lBinCat = clGexDbQuery.value("HBIN_CAT").toString().toUpper();
                lBinName = clGexDbQuery.value("HBIN_NAME").toString();
                nCounts = clGexDbQuery.value("BIN_COUNT").toInt();

                mapHBinInfo[nBinNo].m_nBinNum = nBinNo;
                mapHBinInfo[nBinNo].m_strBinName = lBinName;

                if(lBinCat=="P")
                    mapHBinInfo[nBinNo].m_cBinCat = 'P';
                else if(lBinCat=="A")
                    mapHBinInfo[nBinNo].m_cBinCat = 'A';
                else
                    mapHBinInfo[nBinNo].m_cBinCat = 'F';

                nNbParts += nCounts;
                if(lBinCat=="P")
                    nNbPartsGood += nCounts;

                mapHBinInfo[nBinNo].m_mapBinCnt[MERGE_SITE] = nCounts;
            }

            // FROM SBIN_SUMMARY(LastSplitlot)
            // For SBIN
            lQuery = " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" B.SBIN_NO, B.SBIN_CAT, B.SBIN_NAME, ";
            lQuery += " S.BIN_COUNT ";
            lQuery += " FROM  "+NormalizeTableName("_SBIN")+" B ";
            lQuery += " INNER JOIN "+NormalizeTableName("_SBIN_STATS_SUMMARY")+" S ";
            lQuery += "     ON B.SPLITLOT_ID=S.SPLITLOT_ID AND B.SBIN_NO=S.SBIN_NO";
            lQuery += " WHERE ";
            lQuery += "     B.SPLITLOT_ID="+lLastSplitlot;
            lQuery += "     AND S.SITE_NO=-1";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(),
                            clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }

            while(clGexDbQuery.Next())
            {
                // update values for Bin info
                nBinNo = clGexDbQuery.value("SBIN_NO").toInt();
                lBinCat = clGexDbQuery.value("SBIN_CAT").toString().toUpper();
                lBinName = clGexDbQuery.value("SBIN_NAME").toString();
                nCounts = clGexDbQuery.value("BIN_COUNT").toInt();

                mapSBinInfo[nBinNo].m_nBinNum = nBinNo;
                mapSBinInfo[nBinNo].m_strBinName = lBinName;

                if(lBinCat=="P")
                    mapSBinInfo[nBinNo].m_cBinCat = 'P';
                else if(lBinCat=="A")
                    mapSBinInfo[nBinNo].m_cBinCat = 'A';
                else
                    mapSBinInfo[nBinNo].m_cBinCat = 'F';

                mapSBinInfo[nBinNo].m_mapBinCnt[MERGE_SITE] = nCounts;
            }

            // WAFER SUMMARY
            // Current wafer is the reference for wt_wafer tables
            // Use info from this wafer

            // And collect NbParts and NbPartsGood on new result (current Splitlot)
            // add current m_nNbPartsGood and m_nNbParts
            // add values for Parts and Runs
            // And the same for each HBin and SBin
            QMap<int,structBinInfo>::Iterator itBinInfo;

            // For data from summary, have to check if have a MERGE_SITE (from HBR) then use it
            int nSite = INVALID_SITE;
            if(mapHBinInfo.count() > 0)
            {
                itBinInfo=mapHBinInfo.begin();
                if(((itBinInfo.value())).m_mapBinCnt.contains(MERGE_SITE))
                    nSite = MERGE_SITE;
            }

            for(itBinInfo=mapHBinInfo.begin(); itBinInfo!=mapHBinInfo.end(); itBinInfo++)
                mapHBin[itBinInfo.key()] = (itBinInfo.value()).m_mapBinCnt[nSite];

            nSite = INVALID_SITE;
            if(mapSBinInfo.count() > 0)
            {
                itBinInfo=mapSBinInfo.begin();
                if(((itBinInfo.value())).m_mapBinCnt.contains(MERGE_SITE))
                    nSite = MERGE_SITE;
            }

            for(itBinInfo=mapSBinInfo.begin(); itBinInfo!=mapSBinInfo.end(); itBinInfo++)
                mapSBin[itBinInfo.key()] = (itBinInfo.value()).m_mapBinCnt[nSite];

            // THEN UPDATE PHYSICAL WAFER TABLE
            // WITH mapHBin, mapSBin
            mapConsolidationHBin[PhysicalConsolidationName] = mapHBin;
            mapConsolidationSBin[PhysicalConsolidationName] = mapSBin;
            mapConsolidationNbParts[PhysicalConsolidationName] = nNbParts;
            mapConsolidationNbPartsGood[PhysicalConsolidationName] = nNbPartsGood;
        }
        else
        {
            ///////////////////////////////////////////////////////////
            // BINS INFO FROM SAMPLES
            ///////////////////////////////////////////////////////////
            // IF HAVE RETEST OR NOT
            // ALWAYS USE THE RUN CONSOLIDATION flags
            // AND COMPUTE THE HBIN/SBIN FROM THIS
            ///////////////////////////////////////
            // GET THE LIST OF ALL HBIN AND SBIN FROM ALL SPLITLOTS

            int     nBinNo;
            QString lBinCat;
            int     iHBin;
            int     iSBin;
            bool    lFinal;

            // Collect info from initial tables (wafer tables can be corrupted after a bug or a crash)
            // Update mapHbinInfo and mapSBinInfo with previous info if necessary
            // For HBIN
            // FROM SPLITLOT(LOT/WAFER/PROD_DATA/FLAGS
            lQuery =  " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" B.HBIN_NO, ";
            lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.HBIN_CAT)),'#:#',-1) AS HBIN_CAT, ";
            lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.HBIN_NAME)),'#:#',-1) AS HBIN_NAME ";
            lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" S ";
            lQuery += " INNER JOIN "+NormalizeTableName("_HBIN")+" B ";
            lQuery += "     ON S.SPLITLOT_ID=B.SPLITLOT_ID ";
            lQuery += " WHERE ";
            lQuery += "     S.VALID_SPLITLOT<>'N'";
            lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
            lQuery += "     AND S.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
            lQuery += "     AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
            lQuery += " GROUP BY B.HBIN_NO\n";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }
            while(clGexDbQuery.Next())
            {
                // update values for Bin info
                nBinNo = clGexDbQuery.value(0).toInt();
                lBinCat = clGexDbQuery.value(1).toString().toUpper();
                mapHBinInfo[nBinNo].m_nBinNum = nBinNo;
                // Keep the last definition according to the start_t
                if(mapHBinInfo[nBinNo].m_strBinName.isEmpty())
                    mapHBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(2).toString();

                if(lBinCat=="P")
                    mapHBinInfo[nBinNo].m_cBinCat = 'P';
                else if(lBinCat=="A")
                    mapHBinInfo[nBinNo].m_cBinCat = 'A';
                else
                    mapHBinInfo[nBinNo].m_cBinCat = 'F';
            }


            // FOR ALL TEST CONDITIONS
            // For SBIN
            lQuery =  " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" B.SBIN_NO, ";
            lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.SBIN_CAT)),'#:#',-1) AS SBIN_CAT, ";
            lQuery += " SUBSTRING_INDEX(MAX(CONCAT(S.START_T,'#:#',B.SBIN_NAME)),'#:#',-1) AS SBIN_NAME ";
            lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" S ";
            lQuery += " INNER JOIN "+NormalizeTableName("_SBIN")+" B ";
            lQuery += "     ON S.SPLITLOT_ID=B.SPLITLOT_ID ";
            lQuery += " WHERE ";
            lQuery += "     S.VALID_SPLITLOT<>'N'";
            lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
            lQuery += "     AND S.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
            lQuery += "     AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
            lQuery += " GROUP BY B.SBIN_NO\n";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }

            while(clGexDbQuery.Next())
            {
                // update values for Bin info
                nBinNo = clGexDbQuery.value(0).toInt();
                lBinCat = clGexDbQuery.value(1).toString().toUpper();
                mapSBinInfo[nBinNo].m_nBinNum = nBinNo;
                // Keep the last definition according to the start_t
                if(mapSBinInfo[nBinNo].m_strBinName.isEmpty())
                    mapSBinInfo[nBinNo].m_strBinName = clGexDbQuery.value(2).toString();
                if(lBinCat=="P")
                    mapSBinInfo[nBinNo].m_cBinCat = 'P';
                else if(lBinCat=="A")
                    mapSBinInfo[nBinNo].m_cBinCat = 'A';
                else
                    mapSBinInfo[nBinNo].m_cBinCat = 'F';
            }


            ///////////////////////////////
            // FLEXIBLE CONSOLIDATION IS READY INTO THE RUN CONSOLIDATION flags
            // FOR DATA SAMPLES
            lQuery  = "SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" ";
            lQuery += "  S.TEST_INSERTION, ";
            lQuery += "  R.PART_STATUS, ";
            lQuery += "  R.HBIN_NO, ";
            lQuery += "  R.SBIN_NO, ";
            lQuery += "  R.FINAL ";
            lQuery += " FROM "+NormalizeTableName("_SPLITLOT")+" S ";
            lQuery += " INNER JOIN "+NormalizeTableName("_RUN")+" R ";
            lQuery += "     ON R.SPLITLOT_ID=S.SPLITLOT_ID AND R.intermediate=true";
            lQuery += " WHERE ";
            lQuery += "     S.VALID_SPLITLOT<>'N'";
            lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
            lQuery += "     AND S.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
            lQuery += "     AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.left(1000).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }

            if(!clGexDbQuery.First())
            {
                GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, lQuery.toLatin1().constData());
                goto labelUnlock;
            }

            // For each line, update the mapXBin count
            do
            {
                // Get the consolidation name
                lConsolidationName = clGexDbQuery.value("TEST_INSERTION").toString();
                lBinCat = clGexDbQuery.value("PART_STATUS").toString().toUpper();
                iHBin = clGexDbQuery.value("HBIN_NO").toInt();
                iSBin = clGexDbQuery.value("SBIN_NO").toInt();
                lFinal = clGexDbQuery.value("FINAL").toBool();

                // Init if needed
                if(!mapConsolidationHBin[lConsolidationName].contains(iHBin))
                    mapConsolidationHBin[lConsolidationName][iHBin]=0;
                if(!mapConsolidationSBin[lConsolidationName].contains(iSBin))
                    mapConsolidationSBin[lConsolidationName][iSBin]=0;
                if(!mapConsolidationNbPartsGood.contains(lConsolidationName))
                    mapConsolidationNbPartsGood[lConsolidationName] = 0;
                if(!mapConsolidationNbParts.contains(lConsolidationName))
                    mapConsolidationNbParts[lConsolidationName] = 0;

                // Update HBin count
                mapConsolidationHBin[lConsolidationName][iHBin]++;

                // Update SBin count
                mapConsolidationSBin[lConsolidationName][iSBin]++;

                // Update NbParts and NbPartsGood
                if(lBinCat == "P")
                    mapConsolidationNbPartsGood[lConsolidationName]++;;
                mapConsolidationNbParts[lConsolidationName]++;

                // The PHYSICAL CONSOLIDATION is ONLY for PROD_DAT='Y'
                if(lFinal && (ProdData=="Y") && (lConsolidationName!=PhysicalConsolidationName))
                {
                    if(!mapConsolidationHBin[PhysicalConsolidationName].contains(iHBin))
                        mapConsolidationHBin[PhysicalConsolidationName][iHBin]=0;
                    if(!mapConsolidationSBin[PhysicalConsolidationName].contains(iSBin))
                        mapConsolidationSBin[PhysicalConsolidationName][iSBin]=0;
                    if(!mapConsolidationNbPartsGood.contains(PhysicalConsolidationName))
                        mapConsolidationNbPartsGood[PhysicalConsolidationName] = 0;
                    if(!mapConsolidationNbParts.contains(PhysicalConsolidationName))
                        mapConsolidationNbParts[PhysicalConsolidationName] = 0;

                    // Update HBin count
                    mapConsolidationHBin[PhysicalConsolidationName][iHBin]++;
                    mapConsolidationSBin[PhysicalConsolidationName][iSBin]++;
                    mapConsolidationNbParts[PhysicalConsolidationName]++;
                    if(lBinCat == "P")
                        mapConsolidationNbPartsGood[PhysicalConsolidationName]++;;
                }
            }
            while(clGexDbQuery.Next());
        }

        clGexDbQuery.Execute("COMMIT");

        bStatus = false;

        // GCORE-10978
        // Before to update the wafer_consolidation that triggers the update of the consolidated tables
        // we need to update the wafer_info gross_die if gross_die=0
        // Gross_die was re-initialized to 0 if necessary at the begining of the consolidation process
        if(mapConsolidationNbParts.contains(PhysicalConsolidationName))
        {
            // Update NB_PARTS, NB_PARTS_GOOD
            lQuery =  "UPDATE "+NormalizeTableName("_WAFER_INFO");
            lQuery += "        SET GROSS_DIE=IF(GROSS_DIE=0,"+QString::number(mapConsolidationNbParts[PhysicalConsolidationName])+",GROSS_DIE)";
            lQuery += "        ,   NB_PARTS="+QString::number(mapConsolidationNbParts[PhysicalConsolidationName]);
            lQuery += "        ,   NB_PARTS_GOOD="+QString::number(mapConsolidationNbPartsGood[PhysicalConsolidationName]);
            lQuery += " WHERE LOT_ID="+TranslateStringToSqlVarChar(LotId);
            lQuery += "     AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }
        }

        if(mapConsolidationNbParts.contains(PhysicalConsolidationName) && (mapConsolidationNbParts.keys().size() == 1))
        {
            // GCORE-14382
            // If there's only one insertion, we duplicate the final consolidation values to allow one line to be written in the _inter table
            // For PHYSICAL flow with a unique phase name, a PHYSICAL must be inserted in _inter
            // For not PHYSICAL flow with a unique phase name, a INTERMEDIATE is already inserted
            mapConsolidationNbParts["F.I.N.A.L"] = mapConsolidationNbParts[PhysicalConsolidationName];
            mapConsolidationNbPartsGood["F.I.N.A.L"] = mapConsolidationNbPartsGood[PhysicalConsolidationName];
            mapConsolidationHBin["F.I.N.A.L"] = mapConsolidationHBin[PhysicalConsolidationName];
            mapConsolidationSBin["F.I.N.A.L"] = mapConsolidationSBin[PhysicalConsolidationName];
        }

        QString strSuffixTable;
        QMap<QString, int>::Iterator itConsolidation;
        for(itConsolidation = mapConsolidationNbParts.begin(); itConsolidation != mapConsolidationNbParts.end(); itConsolidation++)
        {
            strSuffixTable = "";
            lConsolidationName = itConsolidation.key();

            nNbParts = mapConsolidationNbParts[lConsolidationName];
            nNbPartsGood = mapConsolidationNbPartsGood[lConsolidationName];
            mapHBin = mapConsolidationHBin[lConsolidationName];
            mapSBin = mapConsolidationSBin[lConsolidationName];

            if(lConsolidationName.toUpper() != PhysicalConsolidationName.toUpper())
            {
                strSuffixTable = "_INTER";
                lConsolidationType = "INTERMEDIATE";
                // If F.I.N.A.L => duplicate the final in the _inter
                if(lConsolidationName == "F.I.N.A.L")
                {
                    lConsolidationName = PhysicalConsolidationName;
                }
            }
            else
            {
                lConsolidationType = "PHYSICAL";
            }


            // UPDATE PHYSICAL/INTERMEDIATE CONSOLIDATION
            TimersTraceStart("Function Wafer Consolidation: consolidation trigger "+strSuffixTable);
            lQuery = "INSERT INTO " + NormalizeTableName("_WAFER_CONSOLIDATION"+strSuffixTable);
            lQuery +=" VALUES( " + TranslateStringToSqlVarChar(LotId);
            lQuery +=", " + TranslateStringToSqlVarChar(WaferId);
            lQuery +=", " + QString::number(nNbParts);
            lQuery +=", " + QString::number(nNbPartsGood);
            lQuery +=", " + TranslateStringToSqlVarChar(lConsolidationType);
            lQuery +=", " + TranslateStringToSqlVarChar(lConsolidationName);
            lQuery +=", " + TranslateStringToSqlVarChar(lTestFlow);
            lQuery +=")";
            if(!clGexDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                goto labelUnlock;
            }

            TimersTraceStop("Function Wafer Consolidation: consolidation trigger "+strSuffixTable);
            // WAFER_HBIN ONLY CONTAINS PHYSICAL CONSOLIDATION
            if(!mapHBin.isEmpty() && (ProdData == "Y"))
            {
                lQueryHeader = "INSERT INTO " + NormalizeTableName("_WAFER_HBIN"+strSuffixTable) + " VALUES";
                lNewValue = lAllValues = "";

                for(itBinInfo=mapHBinInfo.begin(); itBinInfo!=mapHBinInfo.end(); itBinInfo++)
                {
                    nBinNo = itBinInfo.key();

                    // ignore specific Bin -1 used to have global summary on all Bin
                    if(nBinNo == MERGE_BIN)
                        continue;

                    if(!mapHBin.contains(nBinNo))
                        mapHBin[nBinNo] = 0;

                    // insert new entry
                    lNewValue = "(";
                    // LOT_ID
                    lNewValue += TranslateStringToSqlVarChar(LotId);
                    lNewValue += ",";
                    // WAFER_ID
                    lNewValue += TranslateStringToSqlVarChar(WaferId);
                    lNewValue += ",";
                    // HBIN_NO
                    lNewValue += QString::number(nBinNo);
                    lNewValue += ",";
                    // HBIN_NAME
                    lNewValue += TranslateStringToSqlVarChar(mapHBinInfo[nBinNo].m_strBinName);
                    lNewValue += ",";
                    // HBIN_CAT
                    lNewValue += mapHBinInfo[nBinNo].m_cBinCat == ' ' ? "null" : TranslateStringToSqlVarChar((QChar)mapHBinInfo[nBinNo].m_cBinCat);
                    lNewValue += ",";
                    // NB_PARTS
                    lNewValue += QString::number(mapHBin[nBinNo]);
                    if(!strSuffixTable.isEmpty())
                    {
                        // CONSOLIDATION_NAME
                        lNewValue += ", " + TranslateStringToSqlVarChar(lConsolidationName);
                        // CONSOLIDATION_FLOW
                        lNewValue += ", " + TranslateStringToSqlVarChar(lTestFlow);
                    }
                    lNewValue += ")";

                    if(!AddInMultiInsertQuery(NormalizeTableName("_WAFER_HBIN"+strSuffixTable), lQueryHeader, lNewValue, lAllValues))
                        goto labelUnlock;
                }

                if(!lAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_WAFER_HBIN"+strSuffixTable), lAllValues)))
                    goto labelUnlock;
            }

            // WAFER_SBIN ONLY CONTAINS PHYSICAL CONSOLIDATION
            if(!mapSBin.isEmpty() && (ProdData == "Y"))
            {
                lQueryHeader = "INSERT INTO " + NormalizeTableName("_WAFER_SBIN"+strSuffixTable) + " VALUES";
                lNewValue = lAllValues = "";

                for(itBinInfo=mapSBinInfo.begin(); itBinInfo!=mapSBinInfo.end(); itBinInfo++)
                {
                    nBinNo = itBinInfo.key();

                    // ignore specific Bin -1 used to have global summary on all Bin
                    if(nBinNo == MERGE_BIN)
                        continue;

                    if(!mapSBin.contains(nBinNo))
                        mapSBin[nBinNo] = 0;

                    // insert new entry
                    lNewValue = "(";
                    // LOT_ID
                    lNewValue += TranslateStringToSqlVarChar(LotId);
                    lNewValue += ",";
                    // WAFER_ID
                    lNewValue += TranslateStringToSqlVarChar(WaferId);
                    lNewValue += ",";
                    // SBIN_NO
                    lNewValue += QString::number(nBinNo);
                    lNewValue += ",";
                    // SBIN_NAME
                    lNewValue += TranslateStringToSqlVarChar(mapSBinInfo[nBinNo].m_strBinName);
                    lNewValue += ",";
                    // SBIN_CAT
                    lNewValue += mapSBinInfo[nBinNo].m_cBinCat == ' ' ? "null" : TranslateStringToSqlVarChar((QChar)mapSBinInfo[nBinNo].m_cBinCat);
                    lNewValue += ",";
                    // NB_PARTS
                    lNewValue += QString::number(mapSBin[nBinNo]);
                    if(!strSuffixTable.isEmpty())
                    {
                        // CONSOLIDATION_NAME
                        lNewValue += ", " + TranslateStringToSqlVarChar(lConsolidationName);
                        // CONSOLIDATION_FLOW
                        lNewValue += ", " + TranslateStringToSqlVarChar(lTestFlow);
                    }
                    lNewValue += ")";

                    if(!AddInMultiInsertQuery(NormalizeTableName("_WAFER_SBIN"+strSuffixTable), lQueryHeader, lNewValue, lAllValues))
                        goto labelUnlock;
                }

                if(!lAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_WAFER_SBIN"+strSuffixTable), lAllValues)))
                    goto labelUnlock;
            }
        }

        QStringList lstTestInsertion;
        lQuery =  " SELECT S.TEST_INSERTION ";
        lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery += " WHERE ";
        lQuery += "     S.VALID_SPLITLOT<>'N'";
        lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND S.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "     AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        lQuery += " ORDER BY S.START_T";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        // Get the test_insertion flow
        while(clGexDbQuery.Next())
        {
            if(!lstTestInsertion.contains(clGexDbQuery.value("TEST_INSERTION").toString()))
                lstTestInsertion << clGexDbQuery.value("TEST_INSERTION").toString();
        }

        // To disabled the Gap Lock on xx_splitlot
        // we need to work on the PRIMARY KEY
        lQuery = "SELECT splitlot_id FROM "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery += " WHERE ";
        lQuery += "   VALID_SPLITLOT<>'N'";
        lQuery += "   AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "   AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "   AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
        while(clGexDbQuery.Next())
        {
            lSplitlots += clGexDbQuery.value("splitlot_id").toString();
        }

        // Update the test_insertion_index for all phases
        lQuery = "UPDATE "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery+= " SET S.TEST_INSERTION_INDEX=";
        lQuery+= "      IF(FIND_IN_SET(S.TEST_INSERTION,"+TranslateStringToSqlVarChar(lstTestInsertion.join(","))+")=0,";
        lQuery+= "          null,";
        lQuery+= "          FIND_IN_SET(S.TEST_INSERTION,"+TranslateStringToSqlVarChar(lstTestInsertion.join(","))+")-1)";
        lQuery += " WHERE ";
        lQuery += "   SPLITLOT_ID IN ("+lSplitlots.join(",")+")";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        // END OF CONSOLIDATION PHASE

        bStatus = true;
    }

    // for ElectTest
    if(m_eTestingStage == eElectTest)
    {
        // No retest in ETEST
        // GET THE LIST OF ALL HBIN AND SBIN FROM ALL SPLITLOTS

        int     nBinNo;
        QString lBinCat;

        // Collect good HBins
        // AND
        // Collect info from initial tables (wafer tables can be corrupted after a bug or a crash)
        QMap<int,structBinInfo> mapHBinInfo;
        QMap<int,structBinInfo> mapSBinInfo;

        QString lTestFlow;
        QString lTestInsertion;
        QStringList lstTestInsertion;
        ///////////////////////////////////////////////////////////
        // Get the Last Splitlot_id for summary
        // FROM SPLITLOT(LOT/WAFER/PROD_DATA/FLAGS)
        ///////////////////////////////////////////////////////////
        // Update mapHbinInfo and mapSBinInfo with previous info if necessary
        // For HBIN
        lQuery =  " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" S.SPLITLOT_ID, S.TEST_INSERTION, S.TEST_FLOW, ";
        lQuery += " B.HBIN_NO, B.HBIN_CAT, B.HBIN_NAME, B.BIN_COUNT ";
        lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery += " INNER JOIN "+NormalizeTableName("_HBIN")+" B ";
        lQuery += "     ON S.SPLITLOT_ID=B.SPLITLOT_ID ";
        lQuery += " WHERE ";
        lQuery += "     S.VALID_SPLITLOT<>'N'";
        lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND S.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "     AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        lQuery += " ORDER BY S.START_T DESC\n";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        nNbPartsGood = 0;
        nNbParts = 0;

        while(clGexDbQuery.Next())
        {
            // Get the test_insertion flow
            if(!lstTestInsertion.contains(clGexDbQuery.value("TEST_INSERTION").toString()))
                lstTestInsertion << clGexDbQuery.value("TEST_INSERTION").toString();

            // update values for Bin info
            nBinNo = clGexDbQuery.value("HBIN_NO").toInt();
            lBinCat = clGexDbQuery.value("HBIN_CAT").toString().toUpper();
            mapHBinInfo[nBinNo].m_nBinNum = nBinNo;
            // Keep the last definition according to the start_t
            if(mapHBinInfo[nBinNo].m_strBinName.isEmpty())
                mapHBinInfo[nBinNo].m_strBinName = clGexDbQuery.value("HBIN_NAME").toString();
            if(lBinCat=="P")
                mapHBinInfo[nBinNo].m_cBinCat = 'P';
            else if(lBinCat=="A")
                mapHBinInfo[nBinNo].m_cBinCat = 'A';
            else
                mapHBinInfo[nBinNo].m_cBinCat = 'F';

            if(!mapHBin.contains(nBinNo))
                mapHBin[nBinNo] = 0;

            if(clGexDbQuery.value("SPLITLOT_ID").toString() == lLastSplitlot)
            {
                lTestFlow = clGexDbQuery.value("TEST_FLOW").toString();
                lTestInsertion = clGexDbQuery.value("TEST_INSERTION").toString();
                if(lBinCat == "P")
                    nNbPartsGood+=clGexDbQuery.value("BIN_COUNT").toInt();
                nNbParts+=clGexDbQuery.value("BIN_COUNT").toInt();
                mapHBin[nBinNo] += clGexDbQuery.value("BIN_COUNT").toInt();
            }
        }

        // For SBIN
        lQuery =  " SELECT "+mAttributes["STRAIGHT_JOIN"].toString()+" S.SPLITLOT_ID, B.SBIN_NO, B.SBIN_CAT, ";
        lQuery += " B.SBIN_NAME, B.BIN_COUNT ";
        lQuery += " FROM  "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery += " INNER JOIN "+NormalizeTableName("_SBIN")+" B ";
        lQuery += "     ON B.SPLITLOT_ID=S.SPLITLOT_ID ";
        lQuery += " WHERE ";
        lQuery += "     S.VALID_SPLITLOT<>'N'";
        lQuery += "     AND S.LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "     AND S.WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "     AND S.PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        lQuery += " ORDER BY S.START_T DESC\n";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        while(clGexDbQuery.Next())
        {
            // update values for Bin info
            nBinNo = clGexDbQuery.value("SBIN_NO").toInt();
            lBinCat = clGexDbQuery.value("SBIN_CAT").toString().toUpper();
            mapSBinInfo[nBinNo].m_nBinNum = nBinNo;
            // Keep the last definition according to the start_t
            if(mapSBinInfo[nBinNo].m_strBinName.isEmpty())
                mapSBinInfo[nBinNo].m_strBinName = clGexDbQuery.value("SBIN_NAME").toString();
            if(lBinCat=="P")
                mapSBinInfo[nBinNo].m_cBinCat = 'P';
            else if(lBinCat=="A")
                mapSBinInfo[nBinNo].m_cBinCat = 'A';
            else
                mapSBinInfo[nBinNo].m_cBinCat = 'F';

            if(!mapSBin.contains(nBinNo))
                mapSBin[nBinNo] = 0;

            if(clGexDbQuery.value("SPLITLOT_ID").toString() == lLastSplitlot)
            {
                mapSBin[nBinNo] += clGexDbQuery.value("BIN_COUNT").toInt();
            }
        }



        // UPDATE PHYSICAL/INTERMEDIATE CONSOLIDATION
        TimersTraceStart("Function Wafer Consolidation: consolidation trigger ");
        lQuery = "INSERT INTO " + NormalizeTableName("_WAFER_CONSOLIDATION");
        lQuery +=" VALUES( " + TranslateStringToSqlVarChar(LotId);
        lQuery +=", " + TranslateStringToSqlVarChar(WaferId);
        lQuery +=", " + QString::number(nNbParts);
        lQuery +=", " + QString::number(nNbPartsGood);
        lQuery +=", " + TranslateStringToSqlVarChar("PHYSICAL");
        lQuery +=", " + TranslateStringToSqlVarChar(PhysicalConsolidationName);
        lQuery +=", " + TranslateStringToSqlVarChar(lTestFlow);
        lQuery +=")";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }

        // UPDATE CONSOLIDATED DATA
        // for each HBIN
        lQueryHeader = "INSERT INTO " + NormalizeTableName("_WAFER_HBIN") + " VALUES";
        lNewValue = lAllValues = "";

        for(itBinInfo=mapHBinInfo.begin(); itBinInfo!=mapHBinInfo.end(); itBinInfo++)
        {
            nBinNo = itBinInfo.key();

            // ignore specific Bin -1 used to have global summary on all Bin
            if(nBinNo == MERGE_BIN)
                continue;

            if(!mapHBin.contains(nBinNo))
                mapHBin[nBinNo] = 0;

            // insert new entry
            lNewValue = "(";
            // LOT_ID
            lNewValue += TranslateStringToSqlVarChar(LotId);
            lNewValue += ",";
            // WAFER_ID
            lNewValue += TranslateStringToSqlVarChar(WaferId);
            lNewValue += ",";
            // HBIN_NO
            lNewValue += QString::number(nBinNo);
            lNewValue += ",";
            // HBIN_NAME
            lNewValue += TranslateStringToSqlVarChar(mapHBinInfo[nBinNo].m_strBinName);
            lNewValue += ",";
            // HBIN_CAT
            lNewValue += mapHBinInfo[nBinNo].m_cBinCat == ' ' ? "null" : TranslateStringToSqlVarChar((QChar)mapHBinInfo[nBinNo].m_cBinCat);
            lNewValue += ",";
            // NB_PARTS
            lNewValue += QString::number(mapHBin[nBinNo]);
            lNewValue += ")";

            if(!AddInMultiInsertQuery(NormalizeTableName("_WAFER_HBIN"), lQueryHeader, lNewValue, lAllValues))
                goto labelUnlock;
        }

        if(!lAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_WAFER_HBIN"), lAllValues)))
            goto labelUnlock;

        // for each SBIN
        // Delete this all entry for new consolidation
        lQueryHeader = "INSERT INTO " + NormalizeTableName("_WAFER_SBIN") + " VALUES";
        lNewValue = lAllValues = "";

        for(itBinInfo=mapSBinInfo.begin(); itBinInfo!=mapSBinInfo.end(); itBinInfo++)
        {
            nBinNo = itBinInfo.key();

            // ignore specific Bin -1 used to have global summary on all Bin
            if(nBinNo == MERGE_BIN)
                continue;

            if(!mapSBin.contains(nBinNo))
                mapSBin[nBinNo] = 0;

            // insert new entry
            lNewValue = "(";
            // LOT_ID
            lNewValue += TranslateStringToSqlVarChar(LotId);
            lNewValue += ",";
            // WAFER_ID
            lNewValue += TranslateStringToSqlVarChar(WaferId);
            lNewValue += ",";
            // SBIN_NO
            lNewValue += QString::number(nBinNo);
            lNewValue += ",";
            // SBIN_NAME
            lNewValue += TranslateStringToSqlVarChar(mapSBinInfo[nBinNo].m_strBinName);
            lNewValue += ",";
            // SBIN_CAT
            lNewValue += mapSBinInfo[nBinNo].m_cBinCat == ' ' ? "null" : TranslateStringToSqlVarChar((QChar)mapSBinInfo[nBinNo].m_cBinCat);
            lNewValue += ",";
            // NB_PARTS
            lNewValue += QString::number(mapSBin[nBinNo]);
            lNewValue += ")";

            if(!AddInMultiInsertQuery(NormalizeTableName("_WAFER_SBIN"), lQueryHeader, lNewValue, lAllValues))
                goto labelUnlock;
        }

        if(!lAllValues.isEmpty() && (!ExecuteMultiInsertQuery(NormalizeTableName("_WAFER_SBIN"), lAllValues)))
            goto labelUnlock;

        // To disabled the Gap Lock on xx_splitlot
        // we need to work on the PRIMARY KEY
        lQuery = "SELECT splitlot_id FROM "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery += " WHERE ";
        lQuery += "   VALID_SPLITLOT<>'N'";
        lQuery += "   AND LOT_ID="+TranslateStringToSqlVarChar(LotId);
        lQuery += "   AND WAFER_ID="+TranslateStringToSqlVarChar(WaferId);
        lQuery += "   AND PROD_DATA="+TranslateStringToSqlVarChar(ProdData);
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }
        while(clGexDbQuery.Next())
        {
            lSplitlots += clGexDbQuery.value("splitlot_id").toString();
        }

        // Update the test_insertion_index for all phases
        lQuery = "UPDATE "+NormalizeTableName("_SPLITLOT")+" S ";
        lQuery+= " SET S.TEST_INSERTION_INDEX=";
        lQuery+= "      IF(FIND_IN_SET(S.TEST_INSERTION,"+TranslateStringToSqlVarChar(lstTestInsertion.join(","))+")=0,";
        lQuery+= "          null,";
        lQuery+= "          FIND_IN_SET(S.TEST_INSERTION,"+TranslateStringToSqlVarChar(lstTestInsertion.join(","))+")-1)";
        lQuery += " WHERE ";
        lQuery += "   SPLITLOT_ID IN ("+lSplitlots.join(",")+")";
        if(!clGexDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            goto labelUnlock;
        }


        bStatus = true;
    }

    // Debug message
    WriteDebugMessageFile("GexDbPlugin_Galaxy::UpdateWaferConsolidation() : Consolidation completed");

labelUnlock:

    TimersTraceStop("Function Wafer Binning Consolidation");

    /////////////////////////
    // DEADLOCK PREVENTION
    // Commit transaction
    clGexDbQuery.Execute("COMMIT");

    // Display or Propagate the Error message
    if(!bStatus)
    {
        if(ConsolidationSummary.isEmpty())
            ConsolidationSummary = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        if(!ConsolidationSummary.isEmpty())
            WarningMessage(ConsolidationSummary);
    }

    WriteDebugMessageFile("Wafer Binning consolidation: Consolidation completed.");

    return bStatus;
}

