#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include "gexdb_plugin_galaxy_defines.h"
#include "gex_shared.h" // for GEX_QUERY_FILTER_LOT
#include <gqtl_log.h>
#include "abstract_query_progress.h"

#include <gqtl_sysutils.h>

#include <QDir>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>

QString GexDbPlugin_Galaxy::GetTrackingLotID(const QString LotID,
  const QString TestingStage, QString &TLID)
{
    QString strQuery; //strSubQuery, strFieldSpec;
    Query_Empty();

    if (!SetTestingStage(TestingStage))
      return QString("error : unsupported testing stage '%s'").arg(TestingStage);

    m_strlQuery_Fields.append("Field|"+m_strTablePrefix+LOT_TN+".tracking_lot_id");

    if (!Query_AddValueCondition(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], LotID))
        return "error : can't Query_AddValueCondition(...) !";
    if (!Query_BuildSqlString(strQuery, true))
        return "error : can't Query_BuildSqlString !"; // distinct ?

    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1").arg( strQuery.replace('\n', ' ')).toLatin1().constData()));.arg(	//GSLOG(SYSLOG_SEV_INFORMATIONAL, "%1").arg( strQuery.replace('\n'.arg( ' ')).toLatin1().constData()));

    GexDbPlugin_Query	clGexDbQuery(this,
      QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        QString m=QString("error executing query %1 : %2")
                .arg(strQuery.replace('\n',' '))
                .arg(clGexDbQuery.lastError().text());
        GSLOG(SYSLOG_SEV_ERROR, m.toLatin1().data());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
          strQuery.toLatin1().constData(),
          clGexDbQuery.lastError().text().toLatin1().constData());
        return m;
    }

    if (clGexDbQuery.size()==0) 	//if(!clGexDbQuery.Next())
        return "error : no TrackingLot found for this lot !";

    //for (int i=0; i<clGexDbQuery.size(); i++) //numRowsAffected
    while (clGexDbQuery.next())
    {
        TLID=clGexDbQuery.value(0).toString();
    }

    return "ok";
}

// return list of lots and sublots concerning the filters
// Lot|Sublot/Wafer|TestFlow|ProdData list
int	GexDbPlugin_Galaxy::GetLotSublotsList(GexDbPlugin_Filter &cFilters,
                                          QList< QMap< QString,QString > > &lFlowToExtract)
{
    QString strQuery; //strSubQuery, strFieldSpec;
    Query_Empty();

    // Flag only if have a BINNING_CONSOLIDATION result in the _CONSOLIDATION table
    QString lWaferIdField = "wafer_id";
    QString lSplitlotTable = NormalizeTableName("_SPLITLOT");
    if(m_eTestingStage == eFinalTest)
    {
        // At FinalTest, use sublot_id
        lWaferIdField = "sublot_id";
    }
    // SELECT distinct Lot,Sublot,TestFlow,TestInsertion,ProdData
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)

    // Set field
    m_strlQuery_Fields.append("Field|"+lSplitlotTable+".lot_id");
    m_strlQuery_Fields.append("Field=wafer_id|"+lSplitlotTable+"."+lWaferIdField);
    m_strlQuery_Fields.append("Field|"+lSplitlotTable+".prod_data");
    m_strlQuery_Fields.append("Field|"+lSplitlotTable+".test_flow");

    if (!Query_AddFilters(cFilters))
        return -1;
    if (!Query_AddTimePeriodCondition(cFilters))
        return -1;
    // Construct query from table and conditions
    if (!Query_BuildSqlString(strQuery, true)) // distinct
        return -1;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
          strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return -1;
    }


    // Lot|Sublot|TestFlow|ProdData list
    while (clGexDbQuery.next())
    {
        QMap< QString,QString > lProperties;
        lProperties["lot_id"]       = clGexDbQuery.value("lot_id").toString();
        if(m_eTestingStage == eFinalTest)
        {
            lProperties["sublot_id"]     = clGexDbQuery.value("wafer_id").toString();
        }
        else
        {
            lProperties["wafer_id"]     = clGexDbQuery.value("wafer_id").toString();
        }
        lProperties["test_flow"]    = clGexDbQuery.value("test_flow").toString();
        lProperties["prod_data"]    = clGexDbQuery.value("prod_data").toString();

        lFlowToExtract.append(lProperties);
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 lots found.").arg(lFlowToExtract.count()).toLatin1().data() );
    return lFlowToExtract.count();
}

bool GexDbPlugin_Galaxy::QueryRetestMapForLotSublot(const QPair<QString,QString> &lotSublotName,
                                                    QMap<int, QStringList> &retestIndexOfSublot)
{
    //GSLOG(SYSLOG_SEV_DEBUG, QString("Query Retest Index List For Lot %1").arg(lot_name));
    QString lQuery; //strSubQuery, strFieldSpec;
    Query_Empty();
    // select distinct ft_splitlot.retest_index, ft_splitlot.retest_hbins from ft_splitlot
    //          where lot_id='9153280' and sublot_id='10' order by ft_splitlot.RETEST_INDEX ASC;
    m_strlQuery_Fields.append("Field|"+m_strTablePrefix + SPLITLOT_TN + ".retest_index ");
    m_strlQuery_Fields.append("Field|"+m_strTablePrefix + SPLITLOT_TN + ".retest_hbins ");

    m_strlQuery_OrderFields.append(m_strTablePrefix + SPLITLOT_TN + ".retest_index|ASC");
    m_strlQuery_ValueConditions.append( m_strTablePrefix + SPLITLOT_TN + ".lot_id|String|" +lotSublotName.first);
    m_strlQuery_ValueConditions.append( m_strTablePrefix + SPLITLOT_TN + ".sublot_id|String|" +lotSublotName.second);
    m_strlQuery_ValueConditions.append( m_strTablePrefix + SPLITLOT_TN + ".prod_data|String|Y");

    if (!Query_BuildSqlString(lQuery, true)) // distinct
        return false;
    //GSLOG(SYSLOG_SEV_DEBUG, QString("GexDbPlugin_Galaxy::QueryRetestIndexListForLot: %1").arg(strQuery.replace('\n', ' ')));

    GexDbPlugin_Query lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    lGexDbQuery.setForwardOnly(true);
    if(!lGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                    lQuery.toLatin1().constData(),
                    lGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Get the result of the of the request
    QString lString;
    while (lGexDbQuery.next())
    {
        int lRestestIndex = lGexDbQuery.value(0).toInt();
        QString hbins=lGexDbQuery.value(1).toString();
        QStringList lLotSublotList;
        if(retestIndexOfSublot.contains(lRestestIndex))
            lLotSublotList = retestIndexOfSublot.value(lRestestIndex);
        lLotSublotList.append(hbins);
        retestIndexOfSublot.insert(lRestestIndex, lLotSublotList);
        //l.push_back(clGexDbQuery.value(0).toInt());
        lString.append(QString(" %1(%2),").arg(lRestestIndex).arg(hbins));
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString(" Lot %1, Sublot%2 : %3").arg(lotSublotName.first)
                                                             .arg(lotSublotName.second)
                                                             .arg(lString).toLatin1().data());

    return true;
}


// unused function
bool GexDbPlugin_Galaxy::QuerySplitlotsList(const QString& lot, const int& ri, QList<int>& splitlots)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Query splitlots list for lot %1 and ri %2").arg(lot).arg(ri).toLatin1().data() );
    QString strQuery; //strSubQuery, strFieldSpec;
    Query_Empty();
    // select ft_splitlot.splitlot_id from ft_splitlot where lot_id='9153280' AND retest_index=1;
    m_strlQuery_Fields.append("Field|"+m_strTablePrefix + SPLITLOT_TN + ".splitlot_id ");

    m_strlQuery_ValueConditions.append( m_strTablePrefix + SPLITLOT_TN + ".lot_id|String|" +lot);
    m_strlQuery_ValueConditions.append( m_strTablePrefix + SPLITLOT_TN + ".retest_index|Numeric|" +QString("%1").arg(ri));
    m_strlQuery_ValueConditions.append( m_strTablePrefix + SPLITLOT_TN + ".prod_data|String|Y");

    if (!Query_BuildSqlString(strQuery, false)) // distinct ?
        return false;
    //GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg(strQuery.replace('\n', ' ')));

    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
          strQuery.toLatin1().constData(),
          clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    while (clGexDbQuery.next())
    {
        splitlots.push_back(clGexDbQuery.value(0).toInt());
    }
    return true;
}

QString GexDbPlugin_Galaxy::QueryHBIN_NOListForLotSublotAndHBINS(const QPair<QString, QString> &lotSublot,
                                                                 const QString& hbins,
                                                                 QVector<int> &binId)
{
    //GSLOG(SYSLOG_SEV_DEBUG, QString("GexDbPlugin_Galaxy::QueryHBIN_NOListForLotSublotAndHBINS for lot %1 and hbins %2").arg(lot).arg(hbins));

    // Make sure not all parts have been retested
    if(hbins.toLower() == "all")
        return "ok: nothing to retrieve as all parts have been retested";

    QString strQuery; //strSubQuery, strFieldSpec;
    Query_Empty();

    // select ft_hbin.HBIN_NO from ft_hbin HB left outer join ft_splitlot SL on SL.splitlot_id = HB.splitlot_id where HB.hbin_cat='F' AND SL.lot_id='9153280' AND SL.sublot_id='9153280';
    m_strlQuery_Fields.append("Field|"+m_strTablePrefix + "hbin.hbin_no ");
    m_strlQuery_ValueConditions.append( m_strTablePrefix + "splitlot.lot_id|String|" +lotSublot.first);
    m_strlQuery_ValueConditions.append( m_strTablePrefix + "splitlot.sublot_id|String|" +lotSublot.second);

    // Check if retest_hbins is "all_pass", "all_fail", or a list of binnings.
    if(hbins.toLower().startsWith("all"))
    {
        QString cat = (hbins=="all_pass"?"P":"F");
        m_strlQuery_ValueConditions.append( m_strTablePrefix + "hbin.hbin_cat|String|"+cat);
    }
    else
        m_strlQuery_ValueConditions.append( m_strTablePrefix + "hbin.hbin_no|Numeric|"+hbins);

    m_strlQuery_LinkConditions.append(m_strTablePrefix + "hbin.splitlot_id|"
                                 + m_strTablePrefix + "splitlot.splitlot_id");

    if (!Query_BuildSqlString_UsingJoins(strQuery, false, "", false,
           GexDbPlugin_Base::eQueryJoin_LeftOuter)) // distinct ?
        return "error: Build Sql String UsingJoins failed";
    //const char *pChar = strQuery.toLatin1().data();
    //GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg(strQuery.replace('\n', ' ')));

    GexDbPlugin_Query clGexDbQuery(this,
      QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
          strQuery.toLatin1().constData(),
          clGexDbQuery.lastError().text().toLatin1().constData());
        return "error: query exec failed: "+clGexDbQuery.lastError().text();
    }

    while (clGexDbQuery.next())
    {
        int nHbin = clGexDbQuery.value(0).toInt();
        if(!binId.contains(nHbin))
            binId.push_back(nHbin);
    }

    return "ok";
}


bool GexDbPlugin_Galaxy::isDataBaseContainRetestPhase()
{
    QString lQuery;
    Query_Empty();

    QString lstrQueryToken = m_strTablePrefix + SPLITLOT_TN + ".test_insertion ";
    QString lstrDbField, lstrDbTable;
    Query_NormalizeToken(lstrQueryToken, lstrDbField, lstrDbTable);

    lQuery = "SELECT count(";
    lQuery.append(lstrDbField);
    lQuery.append(") FROM ");
    lQuery.append(lstrDbTable);
    lQuery.append(" where ");
    lQuery.append(lstrDbField);
    lQuery.append(" is not null and trim(");
    lQuery.append(lstrDbField);
    lQuery.append(") != '' ");


    GexDbPlugin_Query lGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    lGexDbQuery.setForwardOnly(true);
    if(!lGexDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
                    lQuery.toLatin1().constData(),
                    lGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    if (lGexDbQuery.next() && lGexDbQuery.value(0).isValid() && lGexDbQuery.value(0).toInt() > 0)
        return true;

    return false;
}

// Example from GCORE-1152
//+-------------+----------+------------+------------+----------+---------------+--------------+--------------+--------------+-----------+----------------+
//| splitlot_id | part_typ | lot_id     | sublot_id  | nb_parts | nb_parts_good |test_insertion| retest_index | retest_hbins | prod_data | valid_splitlot |
//+-------------+----------+------------+------------+----------+---------------+--------------+--------------+--------------+-----------+----------------+
//|  1603200002 | SIT8208  | 1DA04T4_02 | 1DA04T4_02 |     1683 |          1155 | PRE_FT       |            0 | NULL         | Y         | Y              |
//|  1603200003 | SIT8208  | 1DA04T4_02 | 1DA04T4_02 |      336 |           221 | PRE_FT       |            0 | NULL         | Y         | Y              |
//|                                                  => nb_parts = 2019
//|  1603200005 | SIT8208  | 1DA04T4_02 | 1DA04T4_02 |      636 |           195 | PRE_FT       |            1 | all_fail     | Y         | Y              |
//|                                                  => nb_fails = 441 + 7 missing fail from retest_index 0
//|  1603200004 | SIT8208  | 1DA04T4_02 | 1DA04T4_02 |      389 |           367 | POST_FT      |            0 | all_pass     | Y         | Y              |
//|  1603200001 | SIT8208  | 1DA04T4_02 | 1DA04T4_02 |     1128 |          1000 | POST_FT      |            0 | all_pass     | Y         | Y              |
//|                                                  => nb_parts = 1517 + 54 missing pass from PRE_FT phase
//|  1603200006 | SIT8208  | 1DA04T4_02 | 1DA04T4_02 |       66 |            59 | POST_FT      |            1 | all_fail     | Y         | Y              |
//|                                                  => nb_fails = 7 + 84 missing fail from retest_index 0
//+-------------+----------+------------+------------+----------+---------------+--------------+--------------+--------------+-----------+----------------+
// Final Consolidation (diff if missing parts)
//+------------+------------+----------+---------------+------------------------+--------------------+--------------------+
//| lot_id     | sublot_id  | nb_parts | nb_parts_good | consolidated_data_type | consolidation_name | consolidation_flow |
//+------------+------------+----------+---------------+------------------------+--------------------+--------------------+
//| 1DA04T4_02 | 1DA04T4_02 |     1874 |          1426 | PHYSICAL               | PHYSICAL           | P                  |
//|            |            |=1426 (pass POST) +448 (fail PRE) (84 missing fail + 54 missing pass + 7 missing fail)
//| 1DA04T4_02 | 1DA04T4_02 |     2019 |          1426 | PHYSICAL               | PHYSICAL           | P                  |
//|                         | WITH MISSING_PARTS OPTION
//+------------+------------+----------+---------------+------------------------+--------------------+-------------------------+
// Intermediate Consolidation (diff if missing parts)
//+------------+------------+----------+---------------+------------------------+--------------------+--------------------+
//| lot_id     | sublot_id  | nb_parts | nb_parts_good | consolidated_data_type | consolidation_name | consolidation_flow |
//+------------+------------+----------+---------------+------------------------+--------------------+--------------------+
//| 1DA04T4_02 | 1DA04T4_02 |     2019 |          1571 | INTERMEDIATE           | PRE_FT             | P                  |
//|            |            |=1683+336 | =1155+221+195
//| 1DA04T4_02 | 1DA04T4_02 |     2012 |          1571 | INTERMEDIATE           | PRE_FT             | P                  |
//|                         | INTERMEDIATE CONSOLIDATED EXTRACTION (based on existing parts)
//| 1DA04T4_02 | 1DA04T4_02 |     1517 |          1426 | INTERMEDIATE           | POST_FT            | P                  |
//|            |            |=1128+389 |  =1000+367+59 |
//| 1DA04T4_02 | 1DA04T4_02 |     1433 |          1426 | INTERMEDIATE           | POST_FT            | P                  |
//|                         | INTERMEDIATE CONSOLIDATED EXTRACTION (based on existing parts)
//+------------+------------+----------+---------------+------------------------+--------------------+--------------------+

// The Final consolidation contains :
// * The Intermediate Consolidation POST_FT (the last retest phase)
//   * The last restest from POST_FT (all data from 1603200006)
//   * data from 1603200001 and 1603200004 without all FAIL parts
// * The Intermediate Consolidation PRE_FT without PASS parts
//   * The last retest from PRE_FT without PASS parts (data from 1603200005 without PASS parts)
//   * data from 1603200002 and 1603200003 without all FAIL parts and all PASS parts
// In this case splitlots 1603200002 and 1603200003 can be ignored because of the retest 'all_fail'
// In a case where only some FAIL bins are retested, splitlots 1603200002 and 1603200003 must be extracted
bool GexDbPlugin_Galaxy::QueryDataFilesConsolidatedForFinalTest(GexDbPlugin_Filter& cFilters,
                                                                GexDbPlugin_Galaxy_TestFilter& clTestFilter,
                                                                tdGexDbPluginDataFileList& cMatchingFiles,
                                                                const QString& lLocalDir)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Query data files for consolidated FinalTest starting from %1 runs...")
        .arg(m_uiTotalRuns).toLatin1().data() );

    #ifdef NO_DUMP_PERF
        this->m_bCustomerDebugMode=false;
    #endif

    QString     lLot, lSubLot, lFlow;
    QString     lProdData;
    QStringList lRetestPhases, lRetestNb;
    QStringList lPhases; // list of all PHASES order by StartTime
    QMap<QString, QMap<int,QStringList > > mapPhaseRetestHBins; // list of all Indexes and retest bin per PHASE

    // Need to extract some filters from the GexDbPlugin_Galaxy_TestFilter& clTestFilter
    // to be able to check and retrieve the Consolidation step from the TDR

    // For GetFtConsolidationRetestInfo
    // If the ProdData is not specified, the algo takes "Y" if exists
    // If the ProdData is specified, force the algo to work with
    // If the RetestPhase is not specified, the algo extracts all phases
    // If the RetestPhase is specified, force the algo to work with

    // For the extraction
    // The RetestIndex description is used to extract parts and exclude some binning
    // If the RetestIndex is specified, check if it is consistant (contiguous index start by 0)
    // and stop the extraction at the limit specified
    QStringList::const_iterator lFilterBegin = cFilters.strlQueryFilters.begin();
    QStringList::const_iterator lEnd = cFilters.strlQueryFilters.end();
    //foreach(const QString &lFilterBegin, cFilters.strlQueryFilters)
    for (; lFilterBegin != lEnd; ++lFilterBegin)
    {
        // Check if have a filter on the Retest Phase
        if((*lFilterBegin).section("=",0,0) == GEXDB_PLUGIN_DBFIELD_TESTINSERTION)
            lRetestPhases = (*lFilterBegin).section("=",1).split("|");

        // Check if have a filter on the Retest Instances
        if((*lFilterBegin).section("=",0,0) == m_gexLabelFilterChoices[GEX_QUERY_FILTER_RETESTNBR])
            lRetestNb = (*lFilterBegin).section("=",1).split("|");
    }

    if(!lRetestNb.isEmpty() &&
            (!lRetestNb.startsWith("0") || !lRetestNb.endsWith(QString::number(lRetestNb.count()-1))))
    {
        if(!lRetestNb.startsWith("0"))
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_ConsolidatedFinalTestExtractionError, NULL,
                        QString("Retest Instance list doesn't start with RETEST_INDEX=0 - Consolidation is not possible").toLatin1().data() );
        }
        else
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_ConsolidatedFinalTestExtractionError, NULL,
                        QString("No contiguous Retest Instance list - Consolidation is not possible").toLatin1().data() );
        }
        return false;
    }

    QList< QMap< QString,QString > > lFlowToExtract;
    // Lot|Sublot|TestFlow|ProdData list

    // Extract the Lot/Sublot list from the TDR according to the filter
    int lNumLots=GetLotSublotsList(cFilters, lFlowToExtract);
    if (lNumLots<=0)
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_ConsolidatedFinalTestExtractionError, NULL,
                    QString("Empty dataset for selected filters. "
                            "Please review your filters to match some results").toLatin1().data() );
        return false;
    }
    // for all lots/Sublots
    // Check if have only one Test Flow
    for (int i=0; i<lFlowToExtract.size(); i++)
    {
        QMap< QString,QString > lProperties;
        lProperties = lFlowToExtract.at(i);

        // Check if have several TEST FLOW
        if(lFlow.isEmpty())
            lFlow = lProperties["test_flow"];

        if(lFlow != lProperties["test_flow"])
        {
            // Need a new filter
            QString lErrorMessage = "Several test flows match the selected filters. "
                    "You need to review your filters to match a single test flow for consolidated extraction. "
                    "You may filter on the 'Test Flow' field to restrict your dataset to a single test flow";
            GSET_ERROR1(GexDbPlugin_Base, eDB_ConsolidatedFinalTestExtractionError, NULL, lErrorMessage.toLatin1().data() );
            return false;
        }

        // Check if have several PROD DATA
        if(lProdData.isEmpty())
            lProdData = lProperties["prod_data"];

        if(lProdData != lProperties["prod_data"])
        {
            // Need a new filter
            QString lErrorMessage = "Several prod_data values match the selected filters. "
                    "You need to review your filters to match a single prod_data value for consolidated extraction. "
                    "Please restrict your dataset to a single prod_data value";
            GSET_ERROR1(GexDbPlugin_Base, eDB_ConsolidatedFinalTestExtractionError, NULL, lErrorMessage.toLatin1().data() );
            return false;
        }
    }

    // Force the PROD_DATA and TEST_FLOW in the GENERAL filter
    cFilters.strlQueryFilters.append( QString(GEXDB_PLUGIN_DBFIELD_TESTFLOW)+ "=" + lFlow);
    cFilters.strlQueryFilters.append( QString(GEXDB_PLUGIN_DBFIELD_PROD_DATA)+ "=" + lProdData);

    int total_number_of_stdf=0, total_extracted_size=0;
    GSLOG(SYSLOG_SEV_DEBUG,QString("Query Data Files Consolidated For FinalTest : %1 lots retrieved.")
                                    .arg(lFlowToExtract.count()).toLatin1().data() );
    if (mQueryProgress)
    {
        mQueryProgress->ClearLogs();
        mQueryProgress->AddLog(QString("%1 lot/sublot(s)to be extracted...").arg(lFlowToExtract.count()));
    }

    // for all lots/Sublots
    for (int i=0; i<lFlowToExtract.size(); i++)
    {
        // Init data
        QMap< QString,QString > lProperties;
        lProperties = lFlowToExtract.at(i);

        lLot = lProperties["lot_id"];
        lSubLot =lProperties["sublot_id"];
        lFlow = lProperties["test_flow"];
        lProdData = lProperties["prod_data"];

        // Clean data
        lPhases.clear();
        m_strlWarnings.clear();
        m_mapHBinInfo.clear();
        m_mapSBinInfo.clear();

        // Select specified Phase if Retest Phase filter
        if(!lRetestPhases.isEmpty())
            lPhases.append(lRetestPhases);

        // Extract Consolidation step info from the TDR
        if(!GetConsolidationRetestInfo(lLot, lSubLot, lProdData, lPhases, mapPhaseRetestHBins))
            return false;

        // If no Phase extracted, check if there is a Warning issue
        if(lPhases.isEmpty())
        {
            if(!m_strlWarnings.isEmpty())
            {
                GSET_ERROR1(GexDbPlugin_Base, eDB_ConsolidatedFinalTestExtractionError, NULL, m_strlWarnings.takeLast().toLatin1().data() );
            }
            return false;
        }

        // the m_mapHBinInfo updated by the call of GetFtConsolidationRetestInfo
        // contains all bins from this lot/sublot
        // Extract the list of Good and Fail bins
        // We need this info to know if all or all_pass or all_fail
        QStringList lGoodBins;
        QStringList lBadBins;

        QMap<int,structBinInfo>::Iterator itBinInfo;
        for(itBinInfo=m_mapHBinInfo.begin(); itBinInfo!=m_mapHBinInfo.end(); itBinInfo++)
        {
            if(itBinInfo.value().m_cBinCat == 'P')
                lGoodBins << QString::number(itBinInfo.value().m_nBinNum);
            else
                lBadBins << QString::number(itBinInfo.value().m_nBinNum);
        }

        QMap<int, QStringList> lRetestIndexHbin; // Retest Indexes and hbins
        QList<int> lRestestIndexes;

        // For each RetestPhase
        // Start with the Last Phase
        // Process a Consolidation extraction
        // Start with the Last RetestIndex
        // Apply the RetestHBin from the Last to the previous Retest Phase
        QStringList lRetestPhaseHbins;
        QString     lPhaseName;
        QString     lMsgLog;
        for(int index=lPhases.count()-1; index>=0; --index)
        {
            lPhaseName = lPhases.at(index);

            lRestestIndexes = mapPhaseRetestHBins[lPhaseName].keys();

            // If the Retest Instances were specified
            if(!lRetestNb.isEmpty())
            {
                // Remove extra Retest index if Retest Instance filter
                const int lRetestCount = lRetestNb.count();
                while(lRestestIndexes.count() > lRetestCount)
                    mapPhaseRetestHBins[lPhaseName].remove(lRestestIndexes.takeLast());
            }

            // Then work on the list of Retest for this Phase
            lRetestIndexHbin = mapPhaseRetestHBins[lPhaseName];
            // Between each retest phase, exclude HBin retested
            if(lPhaseName!=lPhases.last())
            {
                // RetestHBins between 2 phases must be all, all_pass, all_fail
                if(mapPhaseRetestHBins[lPhases.at(index+1)][0].first() == "all_pass")
                    lRetestPhaseHbins = lGoodBins;
                if(mapPhaseRetestHBins[lPhases.at(index+1)][0].first() == "all_fail")
                    lRetestPhaseHbins = lBadBins;
                // Stop the extraction is all are retested
                if(mapPhaseRetestHBins[lPhases.at(index+1)][0].first() == "all")
                    break;
            }

            // for all retest index
            // Start the extraction retest index per retest index, splitlot_id per splitlot_id
            // Start by the end (retest) and go to the beginning (test)
            for (int lRetestIndex=lRestestIndexes.size()-1; lRetestIndex>=0; --lRetestIndex)
            {
                // Exclude the HBIN from the next RetestIndex
                // Exclude the HBIN from the next RetestPhase
                QStringList		lRetestHbins = lRetestPhaseHbins;
                if(lRetestIndex<lRestestIndexes.size()-1)
                    lRetestHbins << lRetestIndexHbin[lRetestIndex+1];

                // Stop the extraction is all are retested
                if(lRetestHbins.count() == (lGoodBins.count() + lBadBins.count()))
                    break;

                lMsgLog = "for Lot["+lLot+"]";
                if(lLot != lSubLot)
                    lMsgLog+= "SubLot["+lSubLot+"]";
                if(lProdData!="Y")
                    lMsgLog+= "ProdData["+lProdData+"]";
                lMsgLog+= " at ";
                if(!lPhaseName.isEmpty())
                    lMsgLog+= "Phase["+lPhaseName+"]";
                lMsgLog+= "Index["+QString::number(lRetestIndex)+"]";

                // Constuct the query
                QString strQuery;
                Query_Empty();

                // select ... from ft_splitlot where lot_id='9153280' AND retest_index=1;
                ConstructSplitlotQuery_Fields(); // add plenty of Fields function to ET, WT or FT

                // where
                m_strlQuery_ValueConditions.append("ft_splitlot.lot_id|String|" +lLot);
                m_strlQuery_ValueConditions.append( "ft_splitlot.sublot_id|String|" +lSubLot);
                if(!lPhaseName.isEmpty())
                    m_strlQuery_ValueConditions.append( "ft_splitlot.test_insertion|String|" +lPhaseName);
                m_strlQuery_ValueConditions.append( "ft_splitlot.retest_index|Numeric|" +QString("%1").arg(lRetestIndex));
                m_strlQuery_ValueConditions.append( "ft_splitlot.prod_data|String|"+lProdData);
                m_strlQuery_ValueConditions.append( "ft_splitlot.valid_splitlot|NotString|N");

                // Order by lot_id, sublot_id, start_t
                m_strlQuery_OrderFields.append("ft_splitlot.lot_id");
                m_strlQuery_OrderFields.append("ft_splitlot.sublot_id");
                m_strlQuery_OrderFields.append("ft_splitlot.start_t");

                // Set filters
                bool b=Query_AddFilters(cFilters);
                if (!b)
                  GSLOG(SYSLOG_SEV_ERROR, "Query_AddFilters failed");

                b=Query_BuildSqlString(strQuery, true); // distinct ? Yes
                if (!b)
                  GSLOG(SYSLOG_SEV_ERROR, "Query_BuildSqlString failed ");

                GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
                GexDbPlugin_Galaxy_SplitlotInfo	clSplitlotInfo;
                clGexDbQuery.setForwardOnly(true);

                if(!clGexDbQuery.Execute(strQuery))
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Fail to execute splitlot query "+lMsgLog).toLatin1().data() );
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                                                clGexDbQuery.lastError().text().toLatin1().constData());
                    return false;
                }

                if (clGexDbQuery.size()==0)
                {
                    QString r=QString("Consolidated data extraction for FinalTest error : no splitlot found "+lMsgLog);
                    GSLOG(SYSLOG_SEV_ERROR, r.toLatin1().data());
                    if (mQueryProgress)
                        mQueryProgress->AddLog(r);
                    GSET_ERROR1(GexDbPlugin_Base, eDB_ConsolidatedFinalTestExtractionError, NULL, r.toLatin1().data() );
                    return false;
                }

                // Show query progress dialog
                if (mQueryProgress)
                    mQueryProgress->Start( clGexDbQuery.size() );

                if (clGexDbQuery.size()!=-1)	// some SQL drivers cant retrieve results size...
                {
                    if (mQueryProgress)
                        mQueryProgress->AddLog(
                      QString("Extracting %1 splitlot(s) "+lMsgLog+"%2 ...").arg(clGexDbQuery.size())
                      .arg(lRetestHbins.isEmpty()?"":" (excluding parts with HardBin in "+lRetestHbins.join(",")+")") );
                }
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Query DataFiles Consolidated For FinalTest: creating %1 stdf...")
                  .arg(clGexDbQuery.size()>0?QString::number(clGexDbQuery.size()):"?").toLatin1().data() );

                QString	lStdfFilename;
                QVector<int> lExcludeHBins;
                while(!lRetestHbins.isEmpty())
                    lExcludeHBins << lRetestHbins.takeFirst().toInt();

                while(clGexDbQuery.Next())
                {
                    // Fill SplitlotInfo object
                    FillSplitlotInfo(clGexDbQuery, &clSplitlotInfo, cFilters, &lExcludeHBins);

                    // first check that we have all the samples data for this splitlot
                    // meaning splitlot.NB_PARTS_SAMPLES == splitlot.NB_PARTS_SUMMARY
                    if((clSplitlotInfo.m_uiNbPartsSamples != clSplitlotInfo.m_uiNbPartsSummary)
                            && (clSplitlotInfo.m_uiNbPartsSummary > 0))
                    {
                        QString strMessage = QString
                            ("Consolidated extraction warning: parts from samples (%1) differs from parts from summary (%2).")
                            .arg(clGexDbQuery.value(14).toUInt()).arg(clGexDbQuery.value(16).toUInt());
                        if (mQueryProgress)
                            mQueryProgress->AddLog(strMessage);
                        GSLOG(SYSLOG_SEV_WARNING, strMessage.toLatin1().data() );
                    }

                    // Create STDF file
                    if (!CreateStdfFile(clSplitlotInfo, clTestFilter, lLocalDir,lStdfFilename, cFilters))
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "CreateStdfFile failed !");
                        // Remove STDF file if exist
                        CGexSystemUtils			clSysUtils;
                        QDir					cDir;
                        QString	strStdfFullFileName = lLocalDir + lStdfFilename;
                        clSysUtils.NormalizePath(strStdfFullFileName);
                        if(cDir.exists(strStdfFullFileName))
                            cDir.remove(strStdfFullFileName);
                        // Display error message if not user abort
                        if(!mQueryProgress->IsAbortRequested() && !m_bAbortForUnusableIndex)
                        {
                            GSLOG(SYSLOG_SEV_ERROR, "Query DataFiles Consolidated For FinalTest: can't create STDF file for this splitlot ! deleting file and aborting..."
                                );
                            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, "can't create STDF file !", "deleting file and aborting...");
                            DisplayErrorMessage();
                        }
                        // Clear matching files
                        qDeleteAll(cMatchingFiles);
                        cMatchingFiles.clear();

                        return false;
                    }

                    total_number_of_stdf++;

                    QString	lStdfFullFileName = lLocalDir + lStdfFilename;
                    int s=0;
                    if (QFile::exists(lStdfFullFileName))
                    {
                        QFile f(lStdfFullFileName);
                        s=(f.size());
                        total_extracted_size+=s;
                    }
                    GSLOG(SYSLOG_SEV_INFORMATIONAL,
                      QString("Query DataFiles Consolidated For FinalTest: %1 written (%2ko)")
                      .arg(lStdfFilename).arg(s/1024).toLatin1().data()
                    );
                    // STDF file successfully created
                    GexDbPlugin_DataFile	*pStdfFile= new GexDbPlugin_DataFile;
                    pStdfFile->m_strFileName = lStdfFilename;
                    pStdfFile->m_strFilePath = lLocalDir;
                    pStdfFile->m_bRemoteFile = false;
                    // Append to list
                    cMatchingFiles.append(pStdfFile);
                }	// next splitlot
            }	// next retest index
        }
    }	// next lot

    GSLOG(SYSLOG_SEV_NOTICE, QString("OK : %1 STDF files written, total %2ko.\n")
      .arg(total_number_of_stdf).arg(total_extracted_size/1024).toLatin1().data() );

    if (mQueryProgress)
        mQueryProgress->AddLog(
      QString("Finished (%1 file(s) created, total %2 Mo) !\n")
      .arg(total_number_of_stdf)
      .arg(total_extracted_size/1024/1024) );

    return true;
}

bool GexDbPlugin_Galaxy::RetrieveDistinctLotsWafersList(
    GexDbPlugin_Filter &cFilters,
    QMap<QString, QList< QPair<int, QString> > > &lotswafers)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" for %1 ConsolidatedExtraction=%2")
           .arg(cFilters.strDataTypeQuery)
           .arg(cFilters.bConsolidatedExtraction?"true":"false")
           .toLatin1().data() );
    QString strQuery, strSubQuery;
    Query_Empty();

    if (!Query_AddFilters(cFilters))
        return false;

    // in consolidated point of view, the way to identify wafers is different
    if (cFilters.bConsolidatedExtraction)
    {
        //Query_AddFilters(cFilters);
        m_strlQuery_Fields.append("Field|"+m_strTablePrefix + SPLITLOT_TN + ".lot_id ");
        m_strlQuery_Fields.append("Field|"+m_strTablePrefix + SPLITLOT_TN + ".wafer_nb ");
        m_strlQuery_Fields.append("Field|"+m_strTablePrefix + SPLITLOT_TN + ".wafer_id ");
        m_strlQuery_Fields.append("Function=min_start_t|"+m_strTablePrefix + SPLITLOT_TN + ".start_t|MIN");
        // group by lot, wafer id
        m_strlQuery_GroupFields.append(m_strTablePrefix + SPLITLOT_TN + ".lot_id");
        m_strlQuery_GroupFields.append(m_strTablePrefix + SPLITLOT_TN + ".wafer_id");
        m_strlQuery_GroupFields.append(m_strTablePrefix + SPLITLOT_TN + ".wafer_nb");
        m_strlQuery_ValueConditions.append( m_strTablePrefix + SPLITLOT_TN + ".valid_splitlot|NotString|N");
        if (!Query_BuildSqlString(strSubQuery, false)) // distinct or not ?
            return false;
        strQuery=QString("select * from ( ");
        strQuery+=strSubQuery;
        strQuery+=QString(" ) T where T.min_start_t BETWEEN "
                           + QString::number(cFilters.tQueryFrom)
                           +" AND "
                           + QString::number(cFilters.tQueryTo) );
    }
    else
    {
        //Query_AddFilters(cFilters);
        if (!Query_AddTimePeriodCondition(cFilters))
            return false;

        m_strlQuery_Fields.append("Field|"+m_strTablePrefix + SPLITLOT_TN + ".lot_id ");
        m_strlQuery_Fields.append("Field|"+m_strTablePrefix + SPLITLOT_TN + ".wafer_nb ");
        m_strlQuery_Fields.append("Field|"+m_strTablePrefix + SPLITLOT_TN + ".wafer_id ");
        // group by lot, wafer id
        m_strlQuery_GroupFields.append(m_strTablePrefix + SPLITLOT_TN + ".lot_id");
        m_strlQuery_GroupFields.append(m_strTablePrefix + SPLITLOT_TN + ".wafer_id");
        m_strlQuery_GroupFields.append(m_strTablePrefix + SPLITLOT_TN + ".wafer_nb");

        // m_strlQuery_ValueConditions.append( m_strTablePrefix + SPLITLOT_TN + ".prod_data|String|Y");
        m_strlQuery_ValueConditions.append( m_strTablePrefix + SPLITLOT_TN + ".valid_splitlot|NotString|N");

        if (!Query_BuildSqlString(strQuery, true)) // distinct or not ?
            return false;
    }

    GexDbPlugin_Query clGexDbQuery(this,
      QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);

    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL,
          strQuery.toLatin1().constData(),
          clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    while (clGexDbQuery.next())
    {
        QPair< int, QString > p;
        p.first=clGexDbQuery.value(1).toInt();
        p.second=clGexDbQuery.value(2).toString();
        lotswafers[clGexDbQuery.value(0).toString()].append( p );
    }

    return true;
}

bool GexDbPlugin_Galaxy::CreateStdfForLotWafer(
    QString lotid,
    QString waferid,
    GexDbPlugin_Filter &cFilters,
    GexDbPlugin_Galaxy_TestFilter & clTestFilter,
    const QString & strLocalDir, QString &stdfFN,
    unsigned int uiGexDbBuild
    )
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Create file for lot %1 wafer %2...")
           .arg(lotid).arg(waferid).toLatin1().data() );

    GexDbPlugin_Galaxy_WaferInfo		*pclWaferInfo=NULL;

    stdfFN="Lot"+ lotid.simplified() + "-Wafer" + waferid;
    if(cFilters.strSiteFilterValue.isEmpty() == false)
        stdfFN += "-Site" + cFilters.strSiteFilterValue +".stdf";
     else
        stdfFN += "-overall.stdf";


    QString	strStdfFullFileName = strLocalDir + stdfFN;
    CGexSystemUtils	clSysUtils;
    clSysUtils.NormalizePath(strStdfFullFileName);

    GQTL_STDF::StdfParse stdf;
    if(!stdf.Open(strStdfFullFileName.toLatin1().constData(), STDF_WRITE))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        GSLOG(SYSLOG_SEV_DEBUG, QString(" error while opening STDF %1 ! Aborting...").arg(
               strStdfFullFileName).toLatin1().data());
        return false;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Create '%1'...").arg(strStdfFullFileName).toLatin1().data() );

    QString extraction_mode;	//could be "last_test_instance_only"
    QString desired_notch;

    GSLOG(SYSLOG_SEV_DEBUG, QString("Create stdf : %1 query option(s).")
          .arg( cFilters.m_gexQueries.size() ).toLatin1().constData());
    QList<QString> ls;
    foreach(ls, cFilters.m_gexQueries)
        if (ls.size()>1)
        {
            if (ls.first()=="db_extraction_mode")
                extraction_mode=ls[1];
            if (ls.first()=="db_notch")
                desired_notch=ls[1];
        }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Create stdf for extraction mode : %1").arg( extraction_mode).toLatin1().constData());

    QMap< QPair<int, int>, QMap<QString, QVariant> > xys;
    if (extraction_mode=="last_test_instance_only")
    {
        if (!GetDistinctXY(lotid, waferid, cFilters, xys))
        {
            QString lError = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
            GSLOG(SYSLOG_SEV_WARNING, lError.toLatin1().data());
            return false;
        }
        GSLOG(SYSLOG_SEV_NOTICE, QString("Create Stdf : %1 distinct XY found")
              .arg( xys.size() ).toLatin1().constData());
    }

    GexDbPlugin_Galaxy_SplitlotInfo clSplitlotInfo;

    // Retrieve all the splitlots for this lotwafer
    QString strQuery;
    Query_Empty();
    bool b=Query_AddFilters(cFilters);	//	 Check me : do we have to apply the filter ???
    if (!b)
        GSLOG(SYSLOG_SEV_ERROR, "Query_AddFilters failed");
    b=Query_AddTimePeriodCondition(cFilters); //	 Check me : do we have to apply the filter ???
    if (!b)
        GSLOG(SYSLOG_SEV_ERROR, "Query_AddTimePeriodCondition failed");
    m_strlQuery_ValueConditions.append("wt_splitlot.lot_id|String|"+lotid );
    m_strlQuery_ValueConditions.append("wt_splitlot.wafer_id|String|"+waferid );
    m_strlQuery_ValueConditions.append(m_strTablePrefix + SPLITLOT_TN + ".valid_splitlot|NotString|N");
    m_strlQuery_OrderFields.append(m_strTablePrefix + SPLITLOT_TN + ".start_t|ASC");

    if(!ConstructSplitlotQuery(cFilters, strQuery, uiGexDbBuild, false))
    {
        GSLOG(SYSLOG_SEV_ERROR, "ConstructSplitlotQuery failed");
        return false;
    }

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Create file : query exec error : %1").arg( clGexDbQuery.lastError().text()).toLatin1().constData() );
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        stdf.Close();
        return false;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Create Stdf : %1 splitlots for this lot wafer...")
          .arg( clGexDbQuery.size() ).toLatin1().constData());

    if (mQueryProgress)
        mQueryProgress->AddLog(
                QString("...%1 splitlot(s) found for this wafer").arg(clGexDbQuery.size()));

    bool IsFirstSplitlot=true;

    // For all splitlots
    while(clGexDbQuery.Next())
    {
        // Fill SplitlotInfo object
        FillSplitlotInfo(clGexDbQuery, &clSplitlotInfo, cFilters);

        // Init diferent maps: X,Y matrix with part results, binning map,...
        if(!InitMaps(clSplitlotInfo, cFilters, IsFirstSplitlot,
                     (extraction_mode=="last_test_instance_only")?&xys:NULL))
        {
            if(pclWaferInfo)
                delete pclWaferInfo;
            stdf.Close();
            GSLOG(SYSLOG_SEV_ERROR, "Create Stdf For LotWafer : error in InitMaps ! Aborting...");
            return false;
        }

        // Create testlist
        if(!CreateTestlist(clSplitlotInfo, clTestFilter, cFilters))
        {
            if(pclWaferInfo)
                delete pclWaferInfo;
            stdf.Close();
            GSLOG(SYSLOG_SEV_ERROR, "Create Stdf For LotWafer : error in CreateTestlist ! Aborting...");
            return false;
        }

        if (IsFirstSplitlot)
        {
            IsFirstSplitlot=false;
            if (!BeginStdf(stdf, clSplitlotInfo, clTestFilter, strLocalDir, desired_notch))
            {
                GSLOG(SYSLOG_SEV_ERROR, "BeginStdf failed");
                if(pclWaferInfo)
                 delete pclWaferInfo;
                stdf.Close();
                return false;
            }
        }

        // Write test results. Should do the m_pQueryProgressDlg->UpdateTestCounter(uiNbRuns, m_uiTotalTestResults);
        QString r=WriteTestResults(stdf, clSplitlotInfo, clTestFilter, (extraction_mode=="last_test_instance_only")?&xys:NULL);
        if(r.startsWith("error"))
        {
            if(pclWaferInfo)
                delete pclWaferInfo;
            stdf.Close();
            GSLOG(SYSLOG_SEV_ERROR, QString("WriteTestResults failed : %1").arg( r).toLatin1().constData() );
            return false;
        }

        m_clTestList.ClearData(true);
    }

    if (!EndStdf(stdf, clSplitlotInfo, cFilters, clTestFilter, strLocalDir, desired_notch))
    {
        GSLOG(SYSLOG_SEV_ERROR, "End stdf failed");
        return false;
    }

    // Free ressources
    if(pclWaferInfo)
        delete pclWaferInfo;
    pclWaferInfo=0;
    stdf.Close();

    // Clear maps
    m_mapHardBins.clear();
    m_mapSoftBins.clear();

    return true;
}

bool GexDbPlugin_Galaxy::QueryDataFilesConsolidatedForWaferSort(
  GexDbPlugin_Filter& cFilters,
  GexDbPlugin_Galaxy_TestFilter& clTestFilter,
  tdGexDbPluginDataFileList& cMatchingFiles,
  const QString& /*strDatabasePhysicalPath*/,
  const QString& strLocalDir,
  bool* /*pbFilesCreatedInFinalLocation*/,
  GexDbPlugin_Base::StatsSource /*eStatsSource*/,
  unsigned int	uiGexDbBuild)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,  "Query Data Files Consolidated For WaferSort...");

    QString desired_notch;
    bool exclude_wafers_not_available_at_etest=false;
    QList<QList <QString> >::iterator i;
    for (i=cFilters.m_gexQueries.begin(); i!=cFilters.m_gexQueries.end(); i++)
    {
        QList<QString> l=*i;
        if (l.first()=="db_notch")
        {
            if (l.size()>1)
                desired_notch=l.at(1);
            continue;
        }
        if (l.first()=="db_exclude_wafers_not_available_at")
        {	if (l.size()>1)
                if (l.at(1)=="etest")
                    exclude_wafers_not_available_at_etest=true;
            continue;
        }
    }
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("desired notch = %1").arg( desired_notch).toLatin1().constData() );

    QMap<QString, QList< QPair<int, QString> > > lotswafers;
    QMap<QString, QStringList> lotswaferids;
    if (!RetrieveDistinctLotsWafersList(cFilters, lotswafers))
    {
        GSLOG(SYSLOG_SEV_ERROR, "RetrieveDistinctLotsWafersList failed !");
        return false;
    }

    if (mQueryProgress)
        mQueryProgress->Start( lotswaferids.size() ); // n lots

    QString lotstring;
    //QMapIterator<QString, QStringList> it(lotswaferids);
    QMapIterator<QString, QList< QPair<int,QString> > > it(lotswafers);

    while (it.hasNext())
    {
        it.next();
        lotstring.append(" lot "); lotstring.append(it.key()); lotstring.append(": ");
        if (mQueryProgress)
            mQueryProgress->AddLog(QString("extracting %1...").arg(lotstring) );
        //m_pQueryProgressDlg->editLotID->setText(it.key());

        QPair<int,QString> wafer;
        foreach( wafer, it.value() )
        {
            if (exclude_wafers_not_available_at_etest)
            {
                QString TLID, r;
                r=GetTrackingLotID(it.key(), GEXDB_PLUGIN_GALAXY_WTEST, TLID);
                if (r.startsWith("error"))
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("GetTrackingLotID() failed : %1").arg(r).toLatin1().data() );
                    continue;
                }
                QString ETLotID, ETWaferID;
                r=GetMatchingLotWafer(TLID, wafer.first, GEXDB_PLUGIN_GALAXY_ETEST, ETLotID, ETWaferID);
                if (r.startsWith("error"))
                {
                    if (mQueryProgress)
                        mQueryProgress->AddLog(QString("Warning : wafer (%1) is unfindable at ETest ! ignoring...").arg(wafer.first));
                    GSLOG(SYSLOG_SEV_ERROR, QString("GetMatchingLotWafer() failed : %1").arg(r).toLatin1().data() );
                    continue;
                }
            }

            if (mQueryProgress)
            {
                mQueryProgress->AddLog(
                            QString(" extracting wafer %1...").arg(wafer.second));
                mQueryProgress->SetFileInfo("?",it.key(),"?", wafer.second );
            }
            lotstring.append(QString("%1").arg(wafer.second)+", ");
            QString stdfFN;
            if (mQueryProgress)
                mQueryProgress->AddLog(
              QString("Extracting Lot %1 Wafer %2").arg(it.key()).arg(wafer.second)
              );

            if (!CreateStdfForLotWafer(it.key(), wafer.second, cFilters,
              clTestFilter, strLocalDir, stdfFN, uiGexDbBuild))
            {
                GSLOG(SYSLOG_SEV_ERROR, "CreateStdfForLotWafer failed !");
                return false;
            }
            // STDF file successfully created
            GexDbPlugin_DataFile	*pStdfFile = new GexDbPlugin_DataFile;
            pStdfFile->m_strFileName = stdfFN;
            pStdfFile->m_strFilePath = strLocalDir;
            pStdfFile->m_bRemoteFile = false;
            // Append to list
            cMatchingFiles.append(pStdfFile);
        }
        lotstring.append(" ok.");
        mQueryProgress->EndFileProgress();
    }
    return true;
}
