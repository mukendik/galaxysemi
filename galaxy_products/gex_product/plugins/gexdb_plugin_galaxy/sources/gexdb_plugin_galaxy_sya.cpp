#include <QSqlError>

#include "gexdb_plugin_galaxy.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include <gqtl_utils.h>

#include "statistical_monitoring_tables.h"
#include "statistical_monitoring_alarm_struct.h"
#include "statistical_monitoring_datapoint_struct.h"
#include "statistical_monitoring_item_desc.h"

bool GexDbPlugin_Galaxy::SYA_FetchDataPointsForComputing(QString testingStage,
                                                         QString productRegexp,
                                                         const QMap<QString,QString> &filtersMetaData,
                                                         QString monitoredItemType,
                                                         const QList<MonitoredItemRule> &monitoredItemRules,
                                                         const QStringList& binsToExclude,
                                                         MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                         QString /*testFlow*/,
                                                         QString /*consolidationType*/,
                                                         QString /*consolidationLevel*/,
                                                         QString /*testInsertion*/,
                                                         const QStringList &/*statsToMonitor*/,
                                                         QString /*siteMergeMode*/,
                                                         bool useGrossDie,
                                                         QDateTime computeFrom,
                                                         QDateTime computeTo,
                                                         QStringList& productsMatched,
                                                         int& numLotsMatched,
                                                         int& numDataPointsMatched,
                                                         QSet<int>& siteList,
                                                         QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& binToSiteToStatToValues)
{
    // /!\ Parameters that are not used are kept for future migration of SYA tasks to the ADR

    // Consolidated results on TDR are always merged sites
    siteList.insert(-1);

    /////////////////////////////////////////////////////////////////////////////
    // 1. Subquery: list of wafer_id's for given product/time period/meta-data //
    /////////////////////////////////////////////////////////////////////////////

    SetTestingStage(testingStage);
    QString waferSublot = (m_eTestingStage==eFinalTest ? "sublot" : "wafer");
    QString lQuery;
    GexDbPlugin_Query lDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Prepare the GexDbPlugin_Filter instance
    QString lSubSplitlotQuery, strDbField, strDbTable, strField, strCondition;
    GexDbPlugin_Filter cFilters(this);
    cFilters.strDataTypeQuery = testingStage;
    cFilters.calendarFrom = computeFrom.date();
    cFilters.calendarFrom_Time = computeFrom.time();
    cFilters.calendarTo = computeTo.date();
    cFilters.calendarTo_Time = computeTo.time();
    cFilters.iTimePeriod = GEX_QUERY_TIMEPERIOD_CALENDAR;

    QStringList lNameValues;
    for(QMap<QString,QString>::const_iterator iter = filtersMetaData.begin(); iter != filtersMetaData.end(); ++iter)
    {
        lNameValues.append(iter.key() + "=" + iter.value());
    }
    if(lNameValues.size() != 0)
    {
        cFilters.SetFilters(lNameValues);
    }

    Query_Empty();

    // Select on product_name,lot_id,sublot_id/wafer_id,nb_parts,...
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strDbField, strDbTable);
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], strDbField, strDbTable);
    if(m_eTestingStage==eFinalTest)
    {
        Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], strDbField, strDbTable);
        Query_AddField(GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS, strDbField, strDbTable);
        Query_AddField(GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD, strDbField, strDbTable);
    }
    else
    {
        Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], strDbField, strDbTable);
        Query_AddField(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS, strDbField, strDbTable);
        Query_AddField(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD, strDbField, strDbTable);
        Query_AddField(GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE, strDbField, strDbTable);
    }

    // Set product filters
    cFilters.strlQueryFilters += QString(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT]) +"=" + productRegexp;
    Query_AddFilters(cFilters);

    // Set data filter
    strCondition = NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = NormalizeTableName("_splitlot")+".prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Add time period condition
    Query_ComputeDateConstraints(cFilters);
    if(!Query_AddTimePeriodCondition(cFilters))
    {
        return false;
    }

    // Build subquery
    if (!Query_BuildSqlString_UsingJoins(lSubSplitlotQuery, false))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, "Query_BuildSqlString_UsingJoins(lSubSplitlotQuery, false)", "Unexpected error trying to create query");
        return false;
    }

    // Set the group by clause
    lSubSplitlotQuery +=  " GROUP BY product_name, lot_id, " + waferSublot + "_id";

    ///////////////////////////////////////////
    // 2. Extract matched product, lots & co //
    ///////////////////////////////////////////

    lQuery = "SELECT "
             "   GROUP_CONCAT(DISTINCT S.product_name SEPARATOR '|') AS matched_products, "
             "   COUNT(DISTINCT S.lot_id) AS num_matched_lots, "
             "   COUNT(DISTINCT CONCAT('(',S.lot_id,',',S." + waferSublot + "_id,')')) AS num_matched_datapoints "
             "FROM "
             "   ("+lSubSplitlotQuery+") S ";
    if(!lDbQuery.Execute(lQuery) || !lDbQuery.first())
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    productsMatched.clear();
    if(!lDbQuery.value("matched_products").toString().isEmpty())
    {
        productsMatched = lDbQuery.value("matched_products").toString().split('|');
    }
    numLotsMatched = lDbQuery.value("num_matched_lots").toInt();
    numDataPointsMatched = lDbQuery.value("num_matched_datapoints").toUInt();

    if(numDataPointsMatched == 0)
    {
        // no datapoints will be fetched
        return true;
    }

    /////////////////////////////////
    // 3. Extract the global yield //
    /////////////////////////////////

    lQuery = "SELECT ";
    if(binsToExclude.size() > 0)
    {
        lQuery += "    S.nb_parts - IFNULL(excluded_volumes.excluded_parts, 0) as nb_parts, ";
    }
    else
    {
        lQuery += "    S.nb_parts, ";
    }
    lQuery += "    S.nb_parts_good ";
    if(m_eTestingStage!=eFinalTest)
    {
        if(binsToExclude.size() > 0)
        {
            lQuery += ",S.gross_die - IFNULL(excluded_volumes.excluded_parts, 0) as gross_die ";
        }
        else
        {
            lQuery += ",S.gross_die ";
        }
    }
    lQuery += "FROM "
              "    (" + lSubSplitlotQuery + ") S ";
    if(binsToExclude.size() > 0)
    {
        lQuery += "LEFT OUTER JOIN ("
                  "    SELECT"
                  "        S.lot_id,"
                  "        S." + waferSublot + "_id,"
                  "        SUM(B.nb_parts) as excluded_parts "
                  "    FROM "
                  "        (" + lSubSplitlotQuery + ") S "
                  "    INNER JOIN " + NormalizeTableName("_" + waferSublot + "_" + monitoredItemType.toLower() + "bin") + " B "
                  "        ON B.lot_id=S.lot_id"
                  "        AND B." + waferSublot + "_id=S." + waferSublot + "_id"
                  "        AND B." + monitoredItemType.toLower() + "bin_no IN(" + binsToExclude.join(',') + ")"
                  "    GROUP BY"
                  "        lot_id,"
                  "        " + waferSublot + "_id"
                  ") excluded_volumes"
                  "    ON excluded_volumes.lot_id=S.lot_id AND excluded_volumes." + waferSublot + "_id=S." + waferSublot + "_id";
    }

    if(!lDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Fill the data structure
    MonitoredItemDesc bin = MonitoredItemDesc::CreateItemDesc(
        uniqueKeyRule,
        monitoredItemType.toUpper(),
        "-1",
        "Yield",
        "P",
        "%",
        0);

    while(lDbQuery.next())
    {
        double nbGood = lDbQuery.value("nb_parts_good").toDouble();
        double nbParts = lDbQuery.value("nb_parts").toDouble();
        double nbGross = lDbQuery.value("gross_die").isValid() ? lDbQuery.value("gross_die").toDouble() : 0;
        double yield = ((nbGross != 0) && useGrossDie) ? (nbGood / nbGross * 100) : (nbGood / nbParts * 100);

        binToSiteToStatToValues[bin][-1].insert(C_STATS_RATIO, yield);
    }

    ////////////////////////////////////////////////
    // 4. Extract the datapoints per selected bin //
    ////////////////////////////////////////////////

    // Build the bin where clause
    QString binFilterClause = "";
    QStringList itemSubQueries;
    bool fetchAllItems = false;

    QListIterator<MonitoredItemRule> iter(monitoredItemRules);
    while(iter.hasNext())
    {
        MonitoredItemRule rule = iter.next();
        QListIterator<QString> iter2(rule.ruleItems);
        switch(rule.ruleType)
        {
            case singleItem:
                itemSubQueries.append("(B.bin_no=" + rule.ruleItems.first() + ")");
            break;
            case range:
                itemSubQueries.append("(B.bin_no>=" + rule.ruleItems.first() + " AND B.bin_no<=" + rule.ruleItems.last() + ")");
            break;
            case all:
                fetchAllItems = true;
            break;
            case groupOfItems:
                while(iter2.hasNext())
                {
                    itemSubQueries.append("(B.bin_no=" +iter2.next() + ")");
                }
            break;
            case mergeOfItems:
                // merged items will be fetched separately
            break;
            default:
                itemSubQueries.append("(B.bin_no=" + rule.ruleItems.first() + ")");
            break;
        }
    }
    if(itemSubQueries.size() > 0 && !fetchAllItems)
    {
        binFilterClause += " AND (" + itemSubQueries.join(" OR ") + ")";
    }

    lQuery = "SELECT ";
    lQuery += "    BL.bin_no AS bin_no, "
              "    BL.bin_name AS bin_name, "
              "    BL.bin_cat AS bin_cat, "
              "    IFNULL(B.nb_parts,0) AS bin_parts, ";
    if(binsToExclude.size() > 0)
    {
        lQuery += "    S.nb_parts - IFNULL(excluded_volumes.excluded_parts, 0) as nb_parts ";
    }
    else
    {
        lQuery += "    S.nb_parts ";
    }
    if(m_eTestingStage!=eFinalTest)
    {
        if(binsToExclude.size() > 0)
        {
            lQuery += ",S.gross_die - IFNULL(excluded_volumes.excluded_parts, 0) as gross_die ";
        }
        else
        {
            lQuery += ",S.gross_die ";
        }
    }
    lQuery += "FROM "
              "    (" + lSubSplitlotQuery + ") S "
              "INNER JOIN ("
              "    SELECT"
              "        B.bin_no,"
              "        B.bin_name,"
              "        B.bin_cat"
              "    FROM (" + lSubSplitlotQuery + ") S"
              "    INNER JOIN"
              "        "+ NormalizeTableName("_product_" + monitoredItemType.toLower() + "bin") + " B "
              "        ON B.product_name=S.product_name"
              + binFilterClause +
              "    GROUP BY"
              "        B.bin_no"
              ") BL "
              "LEFT OUTER JOIN " + NormalizeTableName("_" + waferSublot + "_" + monitoredItemType.toLower() + "bin") + " B "
              "    ON B.lot_id=S.lot_id"
              "    AND B." + waferSublot + "_id=S." + waferSublot + "_id"
              "    AND B." + monitoredItemType.toLower() + "bin_no = BL.bin_no ";
    if(binsToExclude.size() > 0)
    {
        lQuery += "LEFT OUTER JOIN ("
                  "    SELECT"
                  "        S.lot_id,"
                  "        S." + waferSublot + "_id,"
                  "        SUM(B.nb_parts) as excluded_parts "
                  "    FROM "
                  "        (" + lSubSplitlotQuery + ") S "
                  "    INNER JOIN " + NormalizeTableName("_" + waferSublot + "_" + monitoredItemType.toLower() + "bin") + " B "
                  "        ON B.lot_id=S.lot_id"
                  "        AND B." + waferSublot + "_id=S." + waferSublot + "_id"
                  "        AND B." + monitoredItemType.toLower() + "bin_no IN(" + binsToExclude.join(',') + ")"
                  "    GROUP BY"
                  "        lot_id,"
                  "        " + waferSublot + "_id"
                  ") excluded_volumes"
                  "    ON excluded_volumes.lot_id=S.lot_id"
                  "    AND excluded_volumes." + waferSublot + "_id=S." + waferSublot + "_id";
    }
    if(!lDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Fill the data structure
    while(lDbQuery.next())
    {
        double nbBin = lDbQuery.value("bin_parts").toDouble();
        double nbParts = lDbQuery.value("nb_parts").toDouble();
        double nbGross = lDbQuery.value("gross_die").isValid() ? lDbQuery.value("gross_die").toDouble() : 0;
        double yield = ((nbGross != 0) && useGrossDie) ? (nbBin / nbGross * 100) : (nbBin / nbParts * 100);
        MonitoredItemDesc bin = MonitoredItemDesc::CreateItemDesc(
            uniqueKeyRule,
            monitoredItemType.toUpper(),
            lDbQuery.value("bin_no").toString(),
            lDbQuery.value("bin_name").toString(),
            lDbQuery.value("bin_cat").toString(),
            "%",
            0);

        binToSiteToStatToValues[bin][-1].insert(C_STATS_RATIO, yield);
    }

    ///////////////////////////////////////////////
    // 5. Extract the datapoints for merged bins //
    ///////////////////////////////////////////////

    iter.toFront();
    while(iter.hasNext())
    {
        MonitoredItemRule rule = iter.next();
        if(rule.ruleType == mergeOfItems)
        {
            lQuery = "SELECT "
                     "    GROUP_CONCAT(DISTINCT BL.bin_no ORDER BY BL.bin_no SEPARATOR '+') AS bin_nos, "
                     "    GROUP_CONCAT(DISTINCT BL.bin_name ORDER BY BL.bin_no SEPARATOR '+') AS bin_names, "
                     "    MIN(BL.bin_cat) AS bin_cat, "
                     "    SUM(B.nb_parts) as bin_parts, ";
            if(binsToExclude.size() > 0)
            {
                lQuery += "    S.nb_parts - IFNULL(excluded_volumes.excluded_parts, 0) as nb_parts ";
            }
            else
            {
                lQuery += "    S.nb_parts ";
            }
            if(m_eTestingStage!=eFinalTest)
            {
                if(binsToExclude.size() > 0)
                {
                    lQuery += ",S.gross_die - IFNULL(excluded_volumes.excluded_parts, 0) as gross_die ";
                }
                else
                {
                    lQuery += ",S.gross_die ";
                }
            }
            lQuery += "FROM "
                      "    (" + lSubSplitlotQuery + ") S "
                      "INNER JOIN ("
                      "    SELECT"
                      "        B.bin_no,"
                      "        B.bin_name,"
                      "        B.bin_cat"
                      "    FROM (" + lSubSplitlotQuery + ") S"
                      "    INNER JOIN"
                      "        "+ NormalizeTableName("_product_" + monitoredItemType.toLower() + "bin") + " B "
                      "        ON B.product_name=S.product_name"
                      "        AND B.bin_no IN ("+ rule.ruleItems.join(',') + ") "
                      "    GROUP BY"
                      "        B.bin_no"
                      ") BL "
                      "LEFT OUTER JOIN " + NormalizeTableName("_" + waferSublot + "_" + monitoredItemType.toLower() + "bin") + " B "
                      "    ON B.lot_id=S.lot_id"
                      "    AND B." + waferSublot + "_id=S." + waferSublot + "_id"
                      "    AND B." + monitoredItemType.toLower() + "bin_no=BL.bin_no ";
            if(binsToExclude.size() > 0)
            {
                lQuery += "LEFT OUTER JOIN ("
                          "    SELECT"
                          "        S.lot_id,"
                          "        S." + waferSublot + "_id,"
                          "        SUM(B.nb_parts) as excluded_parts "
                          "    FROM "
                          "        (" + lSubSplitlotQuery + ") S "
                          "    INNER JOIN " + NormalizeTableName("_" + waferSublot + "_" + monitoredItemType.toLower() + "bin") + " B "
                          "        ON B.lot_id=S.lot_id"
                          "        AND B." + waferSublot + "_id=S." + waferSublot + "_id"
                          "        AND B." + monitoredItemType.toLower() + "bin_no IN(" + binsToExclude.join(',') + ")"
                          "    GROUP BY"
                          "        lot_id,"
                          "        " + waferSublot + "_id"
                          ") excluded_volumes"
                          "    ON excluded_volumes.lot_id=S.lot_id"
                          "    AND excluded_volumes." + waferSublot + "_id=S." + waferSublot + "_id ";
            }
            lQuery += "GROUP BY S.product_name, S.lot_id, S." + waferSublot + "_id";

            if(!lDbQuery.Execute(lQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
                return false;
            }

            // Fill the data structure
            while(lDbQuery.next())
            {
                double nbBin = lDbQuery.value("bin_parts").toDouble();
                double nbParts = lDbQuery.value("nb_parts").toDouble();
                double nbGross = lDbQuery.value("gross_die").isValid() ? lDbQuery.value("gross_die").toDouble() : 0;
                double yield = ((nbGross != 0) && useGrossDie) ? (nbBin / nbGross * 100) : (nbBin / nbParts * 100);
                MonitoredItemDesc bin = MonitoredItemDesc::CreateItemDesc(
                    uniqueKeyRule,
                    monitoredItemType.toUpper(),
                    lDbQuery.value("bin_nos").toString(),
                    lDbQuery.value("bin_names").toString(),
                    lDbQuery.value("bin_cat").toString(),
                    "%",
                    0);

                binToSiteToStatToValues[bin][-1].insert(C_STATS_RATIO, yield);
            }
        }
    }

    return true;
}

bool GexDbPlugin_Galaxy::SYA_FetchDataPointsForCheckOnTrigger(QString testingStage,
                                                              QString productRegexp,
                                                              QString lotId,
                                                              QString sublotId,
                                                              QString waferId,
                                                              const QMap<QString,QString> &filtersMetaData,
                                                              const QList<MonitoredItemDesc> &binList,
                                                              const QStringList& binsToExclude,
                                                              MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                              QString /*testFlow*/,
                                                              QString /*consolidationType*/,
                                                              QString /*consolidationLevel*/,
                                                              QString /*testInsertion*/,
                                                              const QList<int> &/*siteList*/,
                                                              const QList<QString> &/*statsList*/,
                                                              bool useGrossDie,
                                                              const QDateTime* dateFrom,
                                                              const QDateTime* dateTo,
                                                              QString& productList,
                                                              QString& lotList,
                                                              QString& sublotList,
                                                              QString& waferList,
                                                              int& numParts,
                                                              QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& binToSiteToStatToDataPoint)
{
    // /!\ Parameters that are not used are kept for future migration of SYA tasks to the ADR

    /////////////////////////////////////////////////////////////////////////////
    // 1. Subquery: list of wafer_id's for given product/time period/meta-data //
    /////////////////////////////////////////////////////////////////////////////

    SetTestingStage(testingStage);
    QString waferSublot = (m_eTestingStage==eFinalTest ? "sublot" : "wafer");
    QString lQuery;
    GexDbPlugin_Query lDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    bool lotSpecified = false;
    bool sublotSpecified = false;
    bool waferSpecified = false;

    if(!lotId.isEmpty() && (lotId != "*"))
        lotSpecified = true;

    if(!sublotId.isEmpty() && (sublotId != "*"))
        sublotSpecified = true;

    if(!waferId.isEmpty() && (waferId != "*"))
        waferSpecified = true;

    // Prepare the GexDbPlugin_Filter instance
    QString lSubSplitlotQuery, strDbField, strDbTable, strCondition;
    GexDbPlugin_Filter cFilters(this);
    cFilters.strDataTypeQuery = testingStage;
    if(dateFrom && dateTo)
    {
        cFilters.calendarFrom = dateFrom->date();
        cFilters.calendarFrom_Time = dateFrom->time();
        cFilters.calendarTo = dateTo->date();
        cFilters.calendarTo_Time = dateTo->time();
        cFilters.iTimePeriod = GEX_QUERY_TIMEPERIOD_CALENDAR;
    }
    else
    {
        cFilters.iTimePeriod = GEX_QUERY_TIMEPERIOD_ALLDATES;
    }

    QStringList lNameValues;
    for(QMap<QString,QString>::const_iterator iter = filtersMetaData.begin(); iter != filtersMetaData.end(); ++iter)
    {
        lNameValues.append(iter.key() + "=" + iter.value());
    }
    if(lNameValues.size() != 0)
    {
        cFilters.SetFilters(lNameValues);
    }

    Query_Empty();

    // Select on product_name,lot_id,sublot_id/wafer_id,nb_parts,...
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strDbField, strDbTable);
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], strDbField, strDbTable);
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], strDbField, strDbTable);
    if(m_eTestingStage==eFinalTest)
    {
        Query_AddField(GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS, strDbField, strDbTable);
        Query_AddField(GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD, strDbField, strDbTable);
    }
    else
    {
        Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], strDbField, strDbTable);
        Query_AddField(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS, strDbField, strDbTable);
        Query_AddField(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD, strDbField, strDbTable);
        Query_AddField(GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE, strDbField, strDbTable);
    }

    // Set product/lot/sublot/wafer filters
    strCondition = QString(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT]) +"=" + productRegexp;
    cFilters.strlQueryFilters += strCondition;
    if(lotSpecified)
    {
        strCondition = QString(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT]) +"=" + lotId;
        cFilters.strlQueryFilters += strCondition;
    }
    if(sublotSpecified)
    {
        strCondition = QString(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT]) +"=" + sublotId;
        cFilters.strlQueryFilters += strCondition;
    }
    if(waferSpecified)
    {
        strCondition = QString(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID]) +"=" + waferId;
        cFilters.strlQueryFilters += strCondition;
    }
    Query_AddFilters(cFilters);

    // Set data filter
    strCondition = NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = NormalizeTableName("_splitlot")+".prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Add time period condition
    Query_ComputeDateConstraints(cFilters);
    if(!Query_AddTimePeriodCondition(cFilters))
    {
        return false;
    }

    // Build subquery
    if (!Query_BuildSqlString_UsingJoins(lSubSplitlotQuery, false))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, "Query_BuildSqlString_UsingJoins(lSubSplitlotQuery, false)", "Unexpected error trying to create query");
        return false;
    }

    // Set the group by clause
    lSubSplitlotQuery +=  " GROUP BY product_name, lot_id, " + waferSublot + "_id";

    /////////////////////////////////////////////////////////////
    // 2. Extract matched product, lots & co for the reporting //
    /////////////////////////////////////////////////////////////

    lQuery = "SELECT "
             "   GROUP_CONCAT(DISTINCT S.product_name SEPARATOR '|') AS matched_products, "
             "   GROUP_CONCAT(DISTINCT S.lot_id SEPARATOR '|') AS matched_lots, "
             "   GROUP_CONCAT(DISTINCT CONCAT('(',S.lot_id,',',S.sublot_id,')') SEPARATOR '|') AS matched_sublots, ";
    if(m_eTestingStage!=eFinalTest)
    {
        lQuery += "   GROUP_CONCAT(DISTINCT CONCAT('(',S.lot_id,',',S.wafer_id,')') SEPARATOR '|') AS matched_wafers, ";
    }
    lQuery += "   SUM(S.nb_parts) as nb_parts "
             "FROM "
             "   ("+lSubSplitlotQuery+") S ";
    if(!lDbQuery.Execute(lQuery) || !lDbQuery.first())
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    productList = lDbQuery.value("matched_products").toString();
    lotList = lDbQuery.value("matched_lots").toString();
    sublotList = lDbQuery.value("matched_sublots").toString();
    if(m_eTestingStage!=eFinalTest)
    {
        waferList = lDbQuery.value("matched_wafers").toString();
    }
    else
    {
        waferList = lDbQuery.value("matched_sublots").toString();
    }
    numParts = lDbQuery.value("nb_parts").toUInt();

    if(numParts == 0)
    {
        // no datapoints will be fetched
        return true;
    }

    /////////////////////////////////////////////////////////////////
    // 2.5 Extract matched splitlots data for the binning coverage //
    /////////////////////////////////////////////////////////////////

    lQuery = "SELECT ";
    if(m_eTestingStage!=eFinalTest)
    {
        lQuery += "   DISTINCT CONCAT(S.product_name,',',S.lot_id,',',S.sublot_id,',',S.wafer_id,',',S.nb_parts) AS matched_splitlots_data ";
    }
    else
    {
        lQuery += "   DISTINCT CONCAT(S.product_name,',',S.lot_id,',',S.sublot_id,',',S.sublot_id,',',S.nb_parts) AS matched_splitlots_data ";
    }
    lQuery += "FROM "
             "   ("+lSubSplitlotQuery+") S ";

    if(!lDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    QStringList matchedSplitlotsData;
    while(lDbQuery.next())
    {
        matchedSplitlotsData.append(lDbQuery.value("matched_splitlots_data").toString());
    }

    /////////////////////////////////
    // 3. Extract the global yield //
    /////////////////////////////////

    QString monitoredItemType = binList[0].type;

    lQuery = "SELECT "
             "    S.product_name,"
             "    S.lot_id,"
             "    S.sublot_id,";
    if(m_eTestingStage!=eFinalTest)
    {
        lQuery += "   S.wafer_id,";
    }
    if(binsToExclude.size() > 0)
    {
        lQuery += "    S.nb_parts - IFNULL(excluded_volumes.excluded_parts, 0) as nb_parts, ";
    }
    else
    {
        lQuery += "    S.nb_parts, ";
    }
    lQuery += "    S.nb_parts_good ";
    if(m_eTestingStage!=eFinalTest)
    {
        if(binsToExclude.size() > 0)
        {
            lQuery += ",S.gross_die - IFNULL(excluded_volumes.excluded_parts, 0) as gross_die ";
        }
        else
        {
            lQuery += ",S.gross_die ";
        }
    }
    lQuery += "FROM "
              "    (" + lSubSplitlotQuery + ") S ";
    if(binsToExclude.size() > 0)
    {
        lQuery += "LEFT OUTER JOIN ("
                  "    SELECT"
                  "        S.lot_id,"
                  "        S." + waferSublot + "_id,"
                  "        SUM(B.nb_parts) as excluded_parts "
                  "    FROM "
                  "        (" + lSubSplitlotQuery + ") S "
                  "    INNER JOIN " + NormalizeTableName("_" + waferSublot + "_" + monitoredItemType.toLower() + "bin") + " B "
                  "        ON B.lot_id=S.lot_id"
                  "        AND B." + waferSublot + "_id=S." + waferSublot + "_id"
                  "        AND B." + monitoredItemType.toLower() + "bin_no IN(" + binsToExclude.join(',') + ")"
                  "    GROUP BY"
                  "        lot_id,"
                  "        " + waferSublot + "_id"
                  ") excluded_volumes"
                  "    ON excluded_volumes.lot_id=S.lot_id"
                  "    AND excluded_volumes." + waferSublot + "_id=S." + waferSublot + "_id";
    }

    if(!lDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Fill the data structure
    MonitoredItemDesc bin = MonitoredItemDesc::CreateItemDesc(
        uniqueKeyRule,
        monitoredItemType.toUpper(),
        "-1",
        "Yield",
        "P",
        "%",
        0);

    while(lDbQuery.next())
    {
        double nbGood = lDbQuery.value("nb_parts_good").toDouble();
        double nbParts = lDbQuery.value("nb_parts").toDouble();
        double nbGross = lDbQuery.value("gross_die").isValid() ? lDbQuery.value("gross_die").toDouble() : 0;
        double yield = ((nbGross != 0) && useGrossDie) ? (nbGood / nbGross * 100) : (nbGood / nbParts * 100);
        binToSiteToStatToDataPoint[bin][-1]
                .insert(C_STATS_RATIO, StatMonDataPoint(lDbQuery.value("product_name").toString(),
                                                        lDbQuery.value("lot_id").toString(),
                                                        lDbQuery.value("sublot_id").toString(),
                                                        lDbQuery.value(waferSublot + "_id").toString(),
                                                        0,
                                                        nbParts,
                                                        nbParts - nbGood,
                                                        nbGood,
                                                        yield));
    }

    //////////////////////////////////////////////////////////////////////////
    // 4. Extract all the bins consolidated (including those not monitored) //
    //////////////////////////////////////////////////////////////////////////

    // Construct the group by
    lQuery = "SELECT "
             "    S.product_name,"
             "    S.lot_id,"
             "    S.sublot_id,";
    if(m_eTestingStage!=eFinalTest)
    {
        lQuery += "    S.wafer_id,";
    }
    lQuery += "    B." + monitoredItemType.toLower() + "bin_no AS bin_no, "
             "    B." + monitoredItemType.toLower() + "bin_name AS bin_name, "
             "    B." + monitoredItemType.toLower() + "bin_cat AS bin_cat, "
             "    IFNULL(B.nb_parts,0) AS bin_parts, ";
    if(binsToExclude.size() > 0)
    {
        lQuery += "    S.nb_parts - IFNULL(excluded_volumes.excluded_parts, 0) as nb_parts ";
    }
    else
    {
        lQuery += "    S.nb_parts ";
    }
    if(m_eTestingStage!=eFinalTest)
    {
        if(binsToExclude.size() > 0)
        {
            lQuery += ",S.gross_die - IFNULL(excluded_volumes.excluded_parts, 0) as gross_die ";
        }
        else
        {
            lQuery += ",S.gross_die ";
        }
    }
    lQuery += "FROM "
              "    (" + lSubSplitlotQuery + ") S "
              "INNER JOIN "
                  + NormalizeTableName("_" + waferSublot + "_" + monitoredItemType.toLower() + "bin") + " B "
              "    ON B.lot_id=S.lot_id"
              "    AND B." + waferSublot + "_id=S." + waferSublot + "_id ";
    if(binsToExclude.size() > 0)
    {
        lQuery += "LEFT OUTER JOIN ("
                  "    SELECT"
                  "        S.lot_id,"
                  "        S." + waferSublot + "_id,"
                  "        SUM(B.nb_parts) as excluded_parts "
                  "    FROM "
                  "        (" + lSubSplitlotQuery + ") S "
                  "    INNER JOIN " + NormalizeTableName("_" + waferSublot + "_" + monitoredItemType.toLower() + "bin") + " B "
                  "        ON B.lot_id=S.lot_id"
                  "        AND B." + waferSublot + "_id=S." + waferSublot + "_id"
                  "        AND B." + monitoredItemType.toLower() + "bin_no IN(" + binsToExclude.join(',') + ")"
                  "    GROUP BY"
                  "        lot_id,"
                  "        " + waferSublot + "_id"
                  ") excluded_volumes"
                  "    ON excluded_volumes.lot_id=S.lot_id"
                  "    AND excluded_volumes." + waferSublot + "_id=S." + waferSublot + "_id";
    }
    if(!lDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Fill the data structure
    while(lDbQuery.next())
    {
        double nbBin = lDbQuery.value("bin_parts").toDouble();
        double nbParts = lDbQuery.value("nb_parts").toDouble();
        double nbGross = lDbQuery.value("gross_die").isValid() ? lDbQuery.value("gross_die").toDouble() : 0;
        double yield = ((nbGross != 0) && useGrossDie) ? (nbBin / nbGross * 100) : (nbBin / nbParts * 100);
        binToSiteToStatToDataPoint
                [MonitoredItemDesc::CreateItemDesc(
                    uniqueKeyRule,
                    monitoredItemType.toUpper(),
                    lDbQuery.value("bin_no").toString(),
                    lDbQuery.value("bin_name").toString(),
                    lDbQuery.value("bin_cat").toString(),
                    "%",
                    0)]
                [-1]
                .insert(C_STATS_RATIO, StatMonDataPoint(
                    lDbQuery.value("product_name").toString(),
                    lDbQuery.value("lot_id").toString(),
                    lDbQuery.value("sublot_id").toString(),
                    lDbQuery.value(waferSublot + "_id").toString(),
                    0,
                    nbParts,
                    lDbQuery.value("bin_cat").toString() == "F" ? nbBin : 0,
                    lDbQuery.value("bin_cat").toString() == "P" ? nbBin : 0,
                    yield));
    }

    ///////////////////////////////////////////////
    // 5. Extract the datapoints for merged bins //
    ///////////////////////////////////////////////

    for(QList<MonitoredItemDesc>::const_iterator iter = binList.begin(); iter != binList.end(); ++iter)
    {
        if((*iter).num.contains('+'))
        {
            QStringList whereClause;
            QStringList binNumList = (*iter).num.split('+');
            QStringList binNameList = (*iter).name.split('+');

            if(binNumList.length() == binNameList.length())
            {
                for(int i = 0; i < binNumList.length(); ++i)
                {
                    switch(uniqueKeyRule)
                    {
                        case useNum:
                            whereClause.append("B." + monitoredItemType.toLower() + "bin_no = " + binNumList[i]);
                        break;
                        case useName:
                            whereClause.append("B." + monitoredItemType.toLower() + "bin_name = '" + binNameList[i] + "'");
                        break;
                        case useNumAndName:
                        default:
                            whereClause.append("(B." + monitoredItemType.toLower() + "bin_no = " + binNumList[i] + " AND B." + monitoredItemType.toLower() + "bin_name = '" + binNameList[i] + "')");
                        break;
                    }
                }

                lQuery = "SELECT "
                         "    S.product_name,"
                         "    S.lot_id,"
                         "    S.sublot_id,";
                if(m_eTestingStage!=eFinalTest)
                {
                    lQuery += "    S.wafer_id,";
                }
                if(binsToExclude.size() > 0)
                {
                    lQuery += "    S.nb_parts - IFNULL(excluded_volumes.excluded_parts, 0) as nb_parts, ";
                }
                else
                {
                    lQuery += "    S.nb_parts, ";
                }
                if(m_eTestingStage!=eFinalTest)
                {
                    if(binsToExclude.size() > 0)
                    {
                        lQuery += "    S.gross_die - IFNULL(excluded_volumes.excluded_parts, 0) as gross_die, ";
                    }
                    else
                    {
                        lQuery += "    S.gross_die, ";
                    }
                }
                lQuery += "    SUM(B.nb_parts) as bin_parts "
                          "FROM "
                          "    (" + lSubSplitlotQuery + ") S "
                          "INNER JOIN "
                              + NormalizeTableName("_" + waferSublot + "_" + monitoredItemType.toLower() + "bin") + " B "
                          "    ON B.lot_id = S.lot_id"
                          "    AND B." + waferSublot + "_id = S." + waferSublot + "_id"
                          "    AND (" + whereClause.join(" OR ") + ") ";
                if(binsToExclude.size() > 0)
                {
                    lQuery += "LEFT OUTER JOIN ("
                              "    SELECT"
                              "        S.lot_id,"
                              "        S." + waferSublot + "_id,"
                              "        SUM(B.nb_parts) as excluded_parts "
                              "    FROM "
                              "        (" + lSubSplitlotQuery + ") S "
                              "    INNER JOIN " + NormalizeTableName("_" + waferSublot + "_" + monitoredItemType.toLower() + "bin") + " B "
                              "        ON B.lot_id=S.lot_id"
                              "        AND B." + waferSublot + "_id=S." + waferSublot + "_id"
                              "        AND B." + monitoredItemType.toLower() + "bin_no IN(" + binsToExclude.join(',') + ")"
                              "    GROUP BY"
                              "        lot_id,"
                              "        " + waferSublot + "_id"
                              ") excluded_volumes"
                              "    ON excluded_volumes.lot_id=S.lot_id"
                              "    AND excluded_volumes." + waferSublot + "_id=S." + waferSublot + "_id ";
                }
                lQuery += "GROUP BY S.product_name, S.lot_id, S." + waferSublot + "_id";

                if(!lDbQuery.Execute(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
                    return false;
                }

                // Fill the data structure
                while(lDbQuery.next())
                {
                    double nbBin = lDbQuery.value("bin_parts").toDouble();
                    double nbParts = lDbQuery.value("nb_parts").toDouble();
                    double nbGross = lDbQuery.value("gross_die").isValid() ? lDbQuery.value("gross_die").toDouble() : 0;
                    double yield = ((nbGross != 0) && useGrossDie) ? (nbBin / nbGross * 100) : (nbBin / nbParts * 100);
                    binToSiteToStatToDataPoint
                            [*iter]
                            [-1]
                            .insert(C_STATS_RATIO, StatMonDataPoint(
                                lDbQuery.value("product_name").toString(),
                                lDbQuery.value("lot_id").toString(),
                                lDbQuery.value("sublot_id").toString(),
                                lDbQuery.value(waferSublot + "_id").toString(),
                                0,
                                nbParts,
                                (*iter).cat.contains("F") ? nbBin : 0,
                                (*iter).cat.contains("P") ? nbBin : 0,
                                yield));
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // 6. Create empty datapoints for monitored bins with no parts in one or several wafers //
    //////////////////////////////////////////////////////////////////////////////////////////

    for(QList<MonitoredItemDesc>::const_iterator iter = binList.begin(); iter != binList.end(); ++iter)
    {
        QList<StatMonDataPoint> datapoints = binToSiteToStatToDataPoint[*iter][-1].values(C_STATS_RATIO);
        for(QStringList::iterator matchedSplitlotsIter = matchedSplitlotsData.begin(); matchedSplitlotsIter != matchedSplitlotsData.end(); ++matchedSplitlotsIter)
        {
            QStringList splitlotData = (*matchedSplitlotsIter).split(',');
            bool lotAndWaferFoundInDatapoints = false;
            for(QList<StatMonDataPoint>::iterator datapointIter = datapoints.begin(); datapointIter != datapoints.end(); ++datapointIter)
            {
                if((*datapointIter).mLot == splitlotData[1] && (*datapointIter).mSublot == splitlotData[2] && (*datapointIter).mWafer == splitlotData[3])
                {
                    lotAndWaferFoundInDatapoints = true;
                    break;
                }
            }
            if(!lotAndWaferFoundInDatapoints)
            {
                binToSiteToStatToDataPoint
                        [*iter]
                        [-1]
                        .insert(C_STATS_RATIO, StatMonDataPoint(
                            splitlotData[0],
                            splitlotData[1],
                            splitlotData[2],
                            splitlotData[3],
                            0,
                            splitlotData[4].toInt(),
                            0,
                            0,
                            0));
            }
        }
    }

    return true;
}

bool GexDbPlugin_Galaxy::SYA_FetchDataPointsForCheckOnInsertion(QString testingStage,
                                                                int splitlotId,
                                                                const QMap<QString,QString> &filtersMetaData,
                                                                const QList<MonitoredItemDesc> &binList,
                                                                const QStringList& binsToExclude,
                                                                MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                                const QList<int> &/*siteList*/,
                                                                const QList<QString> &/*statsList*/,
                                                                bool useGrossDie,
                                                                QString& productList,
                                                                QString& lotList,
                                                                QString& sublotList,
                                                                QString& waferList,
                                                                int& numParts,
                                                                QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& binToSiteToStatToDataPoint)
{
    // /!\ Parameters that are not used are kept for future migration of SYA tasks to the ADR

    ///////////////////////////////////////////////////////////////////////////
    // 1. Subquery: list of lot_id's for given product/time period/meta-data //
    ///////////////////////////////////////////////////////////////////////////

    SetTestingStage(testingStage);
    QString lQuery;
    GexDbPlugin_Query lDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Prepare the GexDbPlugin_Filter instance
    QString lSubSplitlotQuery, strDbField, strDbTable, strField, strCondition;
    GexDbPlugin_Filter cFilters(this);
    cFilters.strDataTypeQuery = testingStage;
    cFilters.iTimePeriod = GEX_QUERY_TIMEPERIOD_ALLDATES;

    QStringList lNameValues;
    for(QMap<QString,QString>::const_iterator iter = filtersMetaData.begin(); iter != filtersMetaData.end(); ++iter)
    {
        lNameValues.append(iter.key() + "=" + iter.value());
    }
    if(lNameValues.size() != 0)
    {
        cFilters.SetFilters(lNameValues);
    }

    Query_Empty();

    // Select the splitlot_id
    strField = "Field|"+NormalizeTableName("_splitlot")+".splitlot_id";
    m_strlQuery_Fields.append(strField);

    // Select on product_name,lot_id,sublot_id/wafer_id,nb_parts
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strDbField, strDbTable);
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], strDbField, strDbTable);
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], strDbField, strDbTable);
    if(m_eTestingStage!=eFinalTest)
    {
        Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], strDbField, strDbTable);
        Query_AddField(GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE, strDbField, strDbTable);
    }
    Query_AddField(GEXDB_PLUGIN_DBFIELD_PARTS, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_DBFIELD_PARTS_GOOD, strDbField, strDbTable);

    strField = "Field|"+NormalizeTableName("_splitlot")+".splitlot_flags";
    m_strlQuery_Fields.append(strField);

    // Set filter on splitlot
    strCondition = NormalizeTableName("_splitlot")+".splitlot_id|Numeric|"+QString::number(splitlotId);
    m_strlQuery_ValueConditions.append(strCondition);

    Query_AddFilters(cFilters);

    // Set data filter
    strCondition = NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = NormalizeTableName("_splitlot")+".prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Build subquery
    if (!Query_BuildSqlString_UsingJoins(lSubSplitlotQuery, false))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, "Query_BuildSqlString_UsingJoins(lSubSplitlotQuery, false)", "Unexpected error trying to create query");
        return false;
    }

    /////////////////////////////////////////////////////////
    // 2. Extract matched product, lots & splitlot volumes //
    /////////////////////////////////////////////////////////

    lQuery = "SELECT"
             "   S.product_name AS matched_products, "
             "   S.lot_id AS matched_lots, "
             "   S.splitlot_flags AS splitlot_flags, "
             "   CONCAT('(',S.lot_id,',',S.sublot_id,')') AS matched_sublots, "
             "   S.sublot_id AS sublot, ";
    if(m_eTestingStage!=eFinalTest)
    {
        lQuery += "   CONCAT('(',S.lot_id,',',S.wafer_id,')') AS matched_wafers, "
                  "   S.wafer_id AS wafer, "
                  "   S.gross_die,";
    }
    lQuery += "    S.nb_parts,"
              "    S.nb_parts_good ";
    lQuery += "FROM "
              "   ("+lSubSplitlotQuery+") S ";

    if(!lDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
        return false;
    }
    if(!lDbQuery.first())
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("No datapoint matched. SYA won't process.").toLatin1().constData());
        numParts = 0;
        return true;
    }

    // Passed this point, we know that the splitlot did successfully passe the rule filters, there's no need for
    // a complex subquery anymore, we can now query xt_splitlot using the splitlot_id directly.

    productList = lDbQuery.value("matched_products").toString();
    lotList = lDbQuery.value("matched_lots").toString();
    sublotList = lDbQuery.value("matched_sublots").toString();
    waferList = lDbQuery.value("matched_sublots").toString();
    QString sublotId = lDbQuery.value("sublot").toString();
    QString waferId = lDbQuery.value("sublot").toString();
    int grossDie = 0;
    if(m_eTestingStage!=eFinalTest)
    {
        waferList = lDbQuery.value("matched_wafers").toString();
        waferId = lDbQuery.value("wafer").toString();
        grossDie = lDbQuery.value("gross_die").toInt();
    }
    numParts = lDbQuery.value("nb_parts").toInt();
    int numPartsGood = lDbQuery.value("nb_parts_good").toInt();

    if(numParts == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("No datapoint matched. SYA won't process.").toLatin1().constData());
        return true;
    }

    unsigned int lFlag = lDbQuery.value("splitlot_flags").toUInt();

    QString lTableStats =  (lFlag & FLAG_SPLITLOT_PARTSFROMSAMPLES)?"bin_stats_samples":"bin_stats_summary";
    QString lNbParts = (lFlag & FLAG_SPLITLOT_PARTSFROMSAMPLES)?"nb_parts":"bin_count";
    QString lBinStatCondition = (lFlag & FLAG_SPLITLOT_PARTSFROMSAMPLES)?"":" AND site_no = -1 ";

    /////////////////////////////////
    // 3. Extract the global yield //
    /////////////////////////////////

    QString monitoredItemType = binList[0].type;
    int excludedVolume = 0;

    if(binsToExclude.size() > 0)
    {
        if(m_eTestingStage == eElectTest)
        {
            lQuery = "SELECT "
                     "    SUM(nb_parts) as excluded_parts "
                     "FROM "
                         + NormalizeTableName("_" + monitoredItemType.toLower() + "bin") + " "
                     "WHERE "
                     "    splitlot_id = " + QString::number(splitlotId) + " "
                     "    AND " + monitoredItemType.toLower() + "bin_no IN(" + binsToExclude.join(',') + ")";
        }
        else
        {
            lQuery = "SELECT "
                     "    SUM("+ lNbParts +") as excluded_parts "
                     "FROM "
                         + NormalizeTableName("_" + monitoredItemType.toLower() + lTableStats) + " "
                     "WHERE "
                     "    splitlot_id = " + QString::number(splitlotId) + " "
                     "    AND " + monitoredItemType.toLower() + "bin_no IN(" + binsToExclude.join(',') + ") "
                          + lBinStatCondition;
        }

        if(!lDbQuery.Execute(lQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        if(lDbQuery.first())
        {
            excludedVolume = lDbQuery.value("excluded_parts").toInt();
        }
    }

    // Fill the data structure
    int adjustedNumParts = numParts - excludedVolume;
    int adjustedGrossDie = grossDie - excludedVolume;
    int numPartsBad = adjustedNumParts - numPartsGood;
    int totalParts = ((grossDie != 0) && useGrossDie) ? adjustedGrossDie : adjustedNumParts;
    double lYield = ((double)numPartsGood / (double)totalParts) * 100.0;

    binToSiteToStatToDataPoint
            [MonitoredItemDesc::CreateItemDesc(
                uniqueKeyRule,
                monitoredItemType.toUpper(),
                "-1",
                "Yield",
                "P",
                "%",
                0)]
            [-1]
            .insert(C_STATS_RATIO, StatMonDataPoint(productList,
                                                    lotList,
                                                    sublotId,
                                                    waferId,
                                                    splitlotId,
                                                    totalParts,
                                                    numPartsBad,
                                                    numPartsGood,
                                                    lYield));

    //////////////////////////////////////////////////////////////////////
    // 4. Extract all the bins inserted (including those not monitored) //
    //////////////////////////////////////////////////////////////////////

    if(m_eTestingStage == eElectTest)
    {
        lQuery = "SELECT "
                     + monitoredItemType.toLower() + "bin_no AS bin_no,"
                     + monitoredItemType.toLower() + "bin_name AS bin_name,"
                     + monitoredItemType.toLower() + "bin_cat AS bin_cat,"
                 "    bin_count AS bin_parts "
                 "FROM "
                     + NormalizeTableName("_" + monitoredItemType.toLower() + "bin") + " "
                 "WHERE "
                 "    splitlot_id = " + QString::number(splitlotId);
    }
    else
    {
        lQuery = "SELECT "
                 "    B." + monitoredItemType.toLower() + "bin_no AS bin_no,"
                 "    B." + monitoredItemType.toLower() + "bin_name AS bin_name,"
                 "    B." + monitoredItemType.toLower() + "bin_cat AS bin_cat,"
                 "    SUM("+ lNbParts +") AS bin_parts "
                 "FROM "
                     + NormalizeTableName("_" + monitoredItemType.toLower() + "bin") + " B "
                 "INNER JOIN "
                     + NormalizeTableName("_" + monitoredItemType.toLower() + lTableStats) + " BS "
                     "ON B.splitlot_id = BS.splitlot_id "
                     "AND B." + monitoredItemType.toLower() + "bin_no = BS." + monitoredItemType.toLower() + "bin_no "
                     + lBinStatCondition + " "
                 "WHERE "
                 "    B.splitlot_id = " + QString::number(splitlotId) + " "
                 "GROUP BY "
                 "    bin_no";
    }

    if(!lDbQuery.Execute(lQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    while(lDbQuery.next())
    {
        int lBinCount = lDbQuery.value("bin_parts").toInt();
        double lPercentage = ((double)lBinCount / (double)totalParts) * 100.0;

        binToSiteToStatToDataPoint
                [MonitoredItemDesc::CreateItemDesc(
                    uniqueKeyRule,
                    monitoredItemType.toUpper(),
                    lDbQuery.value("bin_no").toString(),
                    lDbQuery.value("bin_name").toString(),
                    lDbQuery.value("bin_cat").toString(),
                    "%",
                    0)]
                [-1]
                .insert(C_STATS_RATIO, StatMonDataPoint(productList,
                                                        lotList,
                                                        sublotId,
                                                        waferId,
                                                        splitlotId,
                                                        totalParts,
                                                        lDbQuery.value("bin_cat").toString() == "F" ? lBinCount : 0,
                                                        lDbQuery.value("bin_cat").toString() == "P" ? lBinCount : 0,
                                                        lPercentage));
    }

    /////////////////////////////////////////////////////////
    // 5. Extract the datapoints for monitored merged bins //
    /////////////////////////////////////////////////////////

    for(QList<MonitoredItemDesc>::const_iterator iter = binList.begin(); iter != binList.end(); ++iter)
    {
        if((*iter).num.contains('+'))
        {
            QStringList whereClause;
            QStringList binNumList = (*iter).num.split('+');
            QStringList binNameList = (*iter).name.split('+');
            if(binNumList.length() == binNameList.length())
            {
                for(int i = 0; i < binNumList.length(); ++i)
                {
                    switch(uniqueKeyRule)
                    {
                        case useNum:
                            whereClause.append("B." + monitoredItemType.toLower() + "bin_no = " + binNumList[i]);
                        break;
                        case useName:
                            whereClause.append("B." + monitoredItemType.toLower() + "bin_name = '" + binNameList[i] + "'");
                        break;
                        case useNumAndName:
                        default:
                            whereClause.append("(B." + monitoredItemType.toLower() + "bin_no = " + binNumList[i] + " AND B." + monitoredItemType.toLower() + "bin_name = '" + binNameList[i] + "')");
                        break;
                    }
                }

                if(m_eTestingStage == eElectTest)
                {
                    lQuery = "SELECT "
                             "    SUM(bin_count) AS bin_parts "
                             "FROM "
                                 + NormalizeTableName("_" + monitoredItemType.toLower() + "bin") + " B "
                             "WHERE "
                             "    splitlot_id = " + QString::number(splitlotId) + " "
                             "    AND (" + whereClause.join(" OR ") + ")";
                }
                else
                {
                    lQuery = "SELECT "
                             "    SUM("+ lNbParts+ ") AS bin_parts "
                             "FROM "
                                 + NormalizeTableName("_" + monitoredItemType.toLower() + "bin") + " B "
                             "INNER JOIN "
                                 + NormalizeTableName("_" + monitoredItemType.toLower() + lTableStats) + " BS "
                                 "ON B.splitlot_id = BS.splitlot_id "
                                 "AND B." + monitoredItemType.toLower() + "bin_no = BS." + monitoredItemType.toLower() + "bin_no "
                                 + lBinStatCondition + " "
                             "WHERE "
                             "    B.splitlot_id = " + QString::number(splitlotId) + " "
                             "    AND (" + whereClause.join(" OR ") + ")";
                }

                if(!lDbQuery.Execute(lQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), lDbQuery.lastError().text().toLatin1().constData());
                    return false;
                }

                if(lDbQuery.first())
                {
                    int lBinCount = lDbQuery.value("bin_parts").toInt();
                    double lPercentage = ((double)lBinCount / (double)totalParts) * 100.0;

                    binToSiteToStatToDataPoint
                            [*iter]
                            [-1]
                            .insert(C_STATS_RATIO, StatMonDataPoint(productList,
                                                                    lotList,
                                                                    sublotId,
                                                                    waferId,
                                                                    splitlotId,
                                                                    totalParts,
                                                                    (*iter).cat.contains("F") ? lBinCount : 0,
                                                                    (*iter).cat.contains("P") ? lBinCount : 0,
                                                                    lPercentage));
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // 6. Create empty datapoints for monitored bins with no parts in this splitlot //
    //////////////////////////////////////////////////////////////////////////////////

    for(QList<MonitoredItemDesc>::const_iterator iter = binList.begin(); iter != binList.end(); ++iter)
    {
        if(!binToSiteToStatToDataPoint.contains(*iter))
        {
            binToSiteToStatToDataPoint[*iter][-1].insert(C_STATS_RATIO, StatMonDataPoint(productList,
                                                                                         lotList,
                                                                                         sublotId,
                                                                                         waferId,
                                                                                         splitlotId,
                                                                                         totalParts,
                                                                                         0,
                                                                                         0,
                                                                                         0));
        }
    }

    return true;
}

