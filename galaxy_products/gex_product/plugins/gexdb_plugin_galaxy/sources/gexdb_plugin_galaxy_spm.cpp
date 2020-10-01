#include <QSqlError>

#include "gexdb_plugin_galaxy.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include <gqtl_utils.h>
#include <QStringBuilder>

#include "statistical_monitoring_tables.h"
#include "statistical_monitoring_alarm_struct.h"
#include "statistical_monitoring_datapoint_struct.h"
#include "statistical_monitoring_item_desc.h"

void GexDbPlugin_Galaxy::SPM_SetQueryError(GexDbPlugin_Query &lQuery)
{
    // Error Code: 1305. PROCEDURE picker does not exist
    if(lQuery.lastError().number() == 1305)
    {
        QString lError = "Incompatible ADR\n\n";
        QStringList lOwnerVersion;
        SPM_GetOwnerVersion(lOwnerVersion);
        if(lOwnerVersion.isEmpty())
            lError += "No agent is managing this database";
        lError += ""+lOwnerVersion.join("\n");
        lError += "\n\n";
        lError += lQuery.lastError().text();
        GSET_ERROR1(GexDbPlugin_Base, eDB_Status, NULL, lError.toLatin1().constData());
    }
    else
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.lastQuery().left(1000).toLatin1().constData(), lQuery.lastError().text().toLatin1().constData());
    }
}

void GexDbPlugin_Galaxy::SPM_GetOwnerVersion(QStringList &lOwnerVersion)
{
    GexDbPlugin_Query lDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString lQuery;
    lOwnerVersion.clear();
    lQuery = "SELECT table_name FROM information_schema.tables WHERE table_schema='"+m_pclDatabaseConnector->m_strDatabaseName+"'";
    lQuery+= " AND table_name LIKE '%owner'";
    if(lDbQuery.exec(lQuery))
    {
        QStringList lTables;
        while (lDbQuery.Next())
        {
            lTables << lDbQuery.value("table_name").toString();
        }
        foreach (QString lTable, lTables)
        {
            lQuery = "SELECT owner_name, version FROM "+lTable;
            if(lDbQuery.exec(lQuery) && lDbQuery.First())
            {
                lOwnerVersion += QString("%1: %2")
                        .arg(lDbQuery.value("owner_name").toString())
                        .arg(lDbQuery.value("version").toString());
            }
        }
    }
}

bool GexDbPlugin_Galaxy::SPM_GetMetadatasList(QString metaName,
                                              QString testingStage,
                                              QString productRegexp,
                                              QString flowRegexp,
                                              QStringList & cMatchingValues)
{
    SetTestingStage(testingStage);
    GexDbPlugin_Query lDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString lQuery;
    QString lMetadatasConditions;

    ////////////////////////////
    // Build the where clause //
    ////////////////////////////
    /// \brief productSubQueries
    QString lCondition;
    QStringList conditionsSubQueries;
    QStringList lRegExps;
    if(!productRegexp.isEmpty())
        lRegExps = productRegexp.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("product_name"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }

    lRegExps.clear();
    if(!flowRegexp.isEmpty())
        lRegExps = flowRegexp.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("test_flow"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }


    ////////////////////////////////
    // Query the matched products //
    ////////////////////////////////
    // test_stats_spm_wt_get_metadatas(metadata_name,aggregation_level,aggregation_type,metadatas_conditions,tests_conditions)
    lQuery = "CALL test_stats_spm_"+m_strPrefixTable+"_get_metadatas(";
    lQuery+= TranslateStringToSqlVarChar(metaName) + ",";
    lQuery+= "'',";
    lQuery+= "'',";
    lQuery+= TranslateStringToSqlVarChar(lMetadatasConditions) + ",";
    lQuery+= "'')";
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }
    cMatchingValues.clear();
    while(lDbQuery.next())
    {
        cMatchingValues.append(lDbQuery.value("metadata_name").toString());
    }

    if(cMatchingValues.isEmpty())
    {
        bool lEmptyADR = false;
        if(lMetadatasConditions.isEmpty())
        {
            // No condition
            lEmptyADR = true;
        }
        else
        {
            // Check if have something in the ADR
            lQuery = "CALL test_stats_spm_"+m_strPrefixTable+"_get_metadatas_count(";
            lQuery+= TranslateStringToSqlVarChar(metaName) + ",";
            lQuery+= "'',";
            lQuery+= "'',";
            lQuery+= "'',";
            lQuery+= "'')";
            if(lDbQuery.exec(lQuery))
            {
                if(lDbQuery.First() && (lDbQuery.value("metadata_count").toInt()==0))
                {
                    lEmptyADR = true;
                }
            }
        }
        if(lEmptyADR)
        {
            QString lError = "ADR is empty for TestingStage="+testingStage+"\n\n";
            QStringList lOwnerVersion;
            SPM_GetOwnerVersion(lOwnerVersion);
            if(lOwnerVersion.isEmpty())
                lError += "No agent is managing this database";
            else if(lOwnerVersion.count() <= 1)
                lError += "Missing some agents to manage this database";
            lError += ""+lOwnerVersion.join("\n");
            GSET_ERROR1(GexDbPlugin_Base, eDB_Status, NULL, lError.toLatin1().constData());
            return false;
        }
    }
    return true;
}

bool GexDbPlugin_Galaxy::SPM_GetProductList(QString testingStage,
                                            QString productRegexp,
                                            QStringList & cMatchingValues)
{
    return SPM_GetMetadatasList("product_name",testingStage,productRegexp,QString(),cMatchingValues);
}

bool GexDbPlugin_Galaxy::SPM_GetFlowList(QString testingStage,
                                         QString productRegexp,
                                         QStringList & cMatchingValues)
{
    return SPM_GetMetadatasList("test_flow",testingStage,productRegexp,QString(),cMatchingValues);
}
bool GexDbPlugin_Galaxy::SPM_GetInsertionList(QString testingStage,
                                              QString productRegexp,
                                              QString flowRegexp,
                                              QStringList & cMatchingValues)
{
    return SPM_GetMetadatasList("test_insertion",testingStage,productRegexp,flowRegexp,cMatchingValues);
}
bool GexDbPlugin_Galaxy::SPM_GetItemsList(QString testingStage,
                                          QString productRegexp,
                                          QString flowRegexp,
                                          QString insertionRegexp,
                                          QString testType,
                                          QStringList & cMatchingValues)
{
    SetTestingStage(testingStage);
    GexDbPlugin_Query lDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString lQuery;
    QString lMetadatasConditions;


    ////////////////////////////
    // Build the where clause //
    ////////////////////////////
    /// \brief productSubQueries
    QString lCondition;
    QStringList conditionsSubQueries;
    QStringList lRegExps;
    lRegExps.clear();
    if(!productRegexp.isEmpty())
        lRegExps = productRegexp.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("product_name"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }

    lRegExps.clear();
    if(!flowRegexp.isEmpty())
        lRegExps = flowRegexp.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("test_flow"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }

    lRegExps.clear();
    if(!insertionRegexp.isEmpty())
        lRegExps = insertionRegexp.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("test_insertion"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }

    QStringList testTypeSubQueries;
    lRegExps.clear();
    if(!testType.isEmpty())
        lRegExps = testType.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("test_type"), lRegExps.at(i), lCondition);
        testTypeSubQueries.append("(" + lCondition + ")");
    }


    /////////////////////////////
    // Query the matched tests //
    /////////////////////////////
    // test_stats_spm_wt_get_tests(test_fields,metadatas_conditions,tests_conditions)
    lQuery = "CALL test_stats_spm_"+m_strPrefixTable+"_get_tests(";
    lQuery+= "'test_number,test_name,test_type',";
    lQuery+= TranslateStringToSqlVarChar(testTypeSubQueries.join(" OR ")) + ",";
    lQuery+= TranslateStringToSqlVarChar(lMetadatasConditions) + ")";
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }
    cMatchingValues.clear();
    while(lDbQuery.next())
    {
        cMatchingValues.append(lDbQuery.value("test_number").toString());
        cMatchingValues.append(lDbQuery.value("test_name").toString());
        cMatchingValues.append(lDbQuery.value("test_type").toString());
    }

    return true;
}

bool GexDbPlugin_Galaxy::SPM_GetConditionsFromFilters(QString testingStage,
                                                      const QMap<QString, QString> &filtersMetaData,
                                                      QMap<QString, QStringList> &filtersConditions)
{
    SetTestingStage(testingStage);

    ////////////////////////////
    // Build the where clause //
    // Set user filters
    // key is the MetaData Name
    // value is the value
    // Need to swith the MetaData Name to the destination column_name with NormalizedName
    ////////////////////////////
    // If TDR is not Connected
    // No MetaData
    if(!filtersMetaData.isEmpty() && m_pmapFields_GexToRemote->isEmpty())
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, "No MetaData loaded");
        return false;
    }

    QString lMetaDataName;
    QString lColumnName;
    QString lValue;
    GexDbPlugin_Mapping_FieldMap::Iterator itMapping;
    for(QMap<QString,QString>::const_iterator iter = filtersMetaData.begin(); iter != filtersMetaData.end(); ++iter)
    {
        lMetaDataName = iter.key();
        itMapping = m_pmapFields_GexToRemote->find(lMetaDataName);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            // Valid MetaData Name
            lValue = iter.value();
            if(!lValue.isEmpty())
            {
                // Valid Value
                lColumnName = (*itMapping).m_strNormalizedName;
                filtersConditions[lColumnName] = lValue.split(',');
            }
        }
        else
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, QString("MetaData '%1' is not loaded").arg(lMetaDataName).toLatin1().constData());
            return false;
        }
    }

    return true;
}


bool GexDbPlugin_Galaxy::SPM_FetchWaferKeysFromFilters(QString testingStage,
                                                       QString productRegexp,
                                                       QString lotId,
                                                       QString sublotId,
                                                       QString waferId,
                                                       const QMap<QString, QString> &filtersMetaData,
                                                       QStringList &waferKeyList)
{
    SetTestingStage(testingStage);
    QString lQuery;
    GexDbPlugin_Query lDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString strDbField, strDbTable, strField, strCondition;
    GexDbPlugin_Filter cFilters(this);
    cFilters.strDataTypeQuery = testingStage;

    Query_Empty();

    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], strDbField, strDbTable);
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], strDbField, strDbTable);
    // Select the wafer/sublot id
    if(m_eTestingStage!=eFinalTest)
    {
        Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], strDbField, strDbTable);
    }

    // Set product/lot/sublot/wafer filters
    bool lotSpecified = false;
    bool sublotSpecified = false;
    bool waferSpecified = false;

    if(!lotId.isEmpty() && (lotId != "*"))
        lotSpecified = true;

    if(!sublotId.isEmpty() && (sublotId != "*"))
        sublotSpecified = true;

    if(!waferId.isEmpty() && (waferId != "*"))
        waferSpecified = true;

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

    // Set user filters
    QStringList lNameValues;
    for(QMap<QString,QString>::const_iterator iter = filtersMetaData.begin(); iter != filtersMetaData.end(); ++iter)
    {
        lNameValues.append(iter.key() + "=" + iter.value());
    }
    if(lNameValues.size() != 0)
    {
        cFilters.SetFilters(lNameValues);
    }

    Query_AddFilters(cFilters);

    // Set data filter
    strCondition = NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = NormalizeTableName("_splitlot")+".prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Build query
    if (!Query_BuildSqlString_UsingJoins(lQuery, false))
{
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, "Query_BuildSqlString_UsingJoins(lSubSplitlotQuery, false)", "Unexpected error trying to create query");
        return false;
    }

    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }
    waferKeyList.clear();
    while(lDbQuery.next())
        {
        waferKeyList.append(lDbQuery.value("lot_id").toString() + "," + lDbQuery.value("sublot_id").toString()
                            + (
                                m_eTestingStage != eFinalTest ?
                                "," + lDbQuery.value("wafer_id").toString() :
                                "," + lDbQuery.value("sublot_id").toString()
                              )
                           );
    }

    return true;
}

bool GexDbPlugin_Galaxy::SPM_FetchDataPointsForComputing(QString testingStage,
                                                         QString productRegexp,
                                                         QString monitoredItemType,
                                                         const QList<MonitoredItemRule>& monitoredItemRules,
                                                         MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                         QString testFlow,
                                                         QString consolidationType,
                                                         QString consolidationLevel,
                                                         QString testInsertion,
                                                         const QStringList& statsToMonitor,
                                                         QString siteMergeMode,
                                                         bool /*useGrossDie*/,
                                                         const QMap<QString, QStringList>& filtersConditions,
                                                         QDateTime dateFrom,
                                                         QDateTime dateTo,
                                                         QStringList &productsMatched,
                                                         int &numLotsMatched,
                                                         int &numDataPointsMatched,
                                                         QSet<int>& siteList,
                                                         QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > > &testToSiteToStatToValues)
{
    SetTestingStage(testingStage);
    GexDbPlugin_Query lDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString lQuery;
    QString lMetadatasConditions;

    ////////////////////////////
    // Build the where clause //
    ////////////////////////////
    QString lCondition;
    QStringList conditionsSubQueries;
    QStringList lRegExps;
    // Product
    if(!productRegexp.isEmpty())
        lRegExps = productRegexp.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("product_name"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }

    lRegExps.clear();
    // TestFlow
    if(!testFlow.isEmpty())
        lRegExps = testFlow.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("test_flow"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }

    lRegExps.clear();
    // TestInsertion
    if(!testInsertion.isEmpty())
        lRegExps = testInsertion.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("test_insertion"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }

    // Date
    if(dateFrom.isValid())
    {
        QString lDateFilter = QString("%1#GEXDB#TO#%2").arg(dateFrom.toTime_t()).arg(dateTo.toTime_t());
        GexDbPlugin_Base::Query_BuildSqlString_ExpressionRange(QString("start_t"),lDateFilter,lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }

    // Other filters
    QString lMetaDataName;
    for(QMap<QString,QStringList>::const_iterator iter = filtersConditions.begin(); iter != filtersConditions.end(); ++iter)
    {
        lMetaDataName = iter.key();
        lRegExps = iter.value();
        if(!lRegExps.isEmpty())
        {
            for(int i=0; i<lRegExps.size(); ++i)
            {
                GexDbPlugin_Base::Query_BuildSqlString_StringFilter(lMetaDataName, lRegExps.at(i), lCondition);
                conditionsSubQueries.append("(" + lCondition + ")");
            }
        }
        if(conditionsSubQueries.size() > 0)
        {
            if(lMetadatasConditions.size() > 0)
                lMetadatasConditions += " AND ";
            lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
            conditionsSubQueries.clear();
        }
    }

    QStringList testConditions;

    // Build the test where clause
    QStringList itemSubQueries;
    QListIterator<MonitoredItemRule> iter(monitoredItemRules);
    while(iter.hasNext())
    {
        MonitoredItemRule rule = iter.next();
        QListIterator<QString> iter2(rule.ruleItems);
        switch(rule.ruleType)
        {
            case singleItem:
                itemSubQueries.append("(test_type='" + monitoredItemType + "' AND test_number=" + rule.ruleItems.first() + ")");
            break;
            case range:
                itemSubQueries.append("(test_type='" + monitoredItemType + "' AND test_number>=" + rule.ruleItems.first() + " AND test_number<=" + rule.ruleItems.last() + ")");
            break;
            case all:
            break;
            case groupOfItems:
                while(iter2.hasNext())
                {
                    itemSubQueries.append("(test_type='" + monitoredItemType + "' AND test_number=" + iter2.next() + ")");
                }
            break;
            case mergeOfItems:
                // merged items will the processed separately
            break;
            default:
                itemSubQueries.append("(test_type='" + monitoredItemType + "' AND test_number=" + rule.ruleItems.first() + ")");
            break;
        }
    }

    if(testConditions.length() > 0)
    {
        testConditions.append("(" + itemSubQueries.join(" OR ") + ")");
    }

    // build the site where clause
    if(siteMergeMode == "merged_sites")
    {
        testConditions.append("(site_num = -1)");
    }
    else if(siteMergeMode == "per_site")
    {
        testConditions.append("(site_num <> -1)");
    }

    ////////////////////////////////
    // Query the matched products //
    ////////////////////////////////
    // test_stats_spm_wt_get_metadatas(metadata_name,aggregation_level,aggregation_type,metadatas_conditions,tests_conditions)
    lQuery = "CALL test_stats_spm_"+m_strPrefixTable+"_get_metadatas(";
    lQuery+= "'product_name',";
    lQuery+= TranslateStringToSqlVarChar(consolidationLevel )+ ",";
    lQuery+= TranslateStringToSqlVarChar(consolidationType) + ",";
    lQuery+= TranslateStringToSqlVarChar(lMetadatasConditions) + ",";
    lQuery+= (testConditions.isEmpty() ? "''" : TranslateStringToSqlVarChar("(" + testConditions.join(" AND ") + ")")) + ")";
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }
    productsMatched.clear();
    while(lDbQuery.next())
    {
        productsMatched.append(lDbQuery.value("metadata_name").toString());
    }
    if(productsMatched.isEmpty())
    {
        // Check if have an empry ADR
        if(SPM_GetMetadatasList("product_name",testingStage,productRegexp,QString(),productsMatched))
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, QString("No products were found matching your criteria within the provided date range").toLatin1().constData());
        }
        return false;
    }

    //////////////////////////////////////
    // Query the number of matched lots //
    //////////////////////////////////////
    // test_stats_spm_wt_get_metadatas_count(metadata_name,aggregation_level,aggregation_type,metadatas_conditions,tests_conditions)
    lQuery = "CALL test_stats_spm_"+m_strPrefixTable+"_get_metadatas_count(";
    lQuery+= "'lot_id',";
    lQuery+= TranslateStringToSqlVarChar(consolidationLevel )+ ",";
    lQuery+= TranslateStringToSqlVarChar(consolidationType) + ",";
    lQuery+= TranslateStringToSqlVarChar(lMetadatasConditions) + ",";
    lQuery+= (testConditions.isEmpty() ? "''" : TranslateStringToSqlVarChar("(" + testConditions.join(" AND ") + ")")) + ")";
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }
    if(!lDbQuery.first())
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, QString("No lots were found matching your criteria within the provided date range").toLatin1().constData());
        return false;
    }
    numLotsMatched = lDbQuery.value("metadata_count").toInt();

    ////////////////////////////////////////////
    // Query the number of matched datapoints //
    ////////////////////////////////////////////
    // test_stats_spm_wt_get_num_datapoints(aggregation_level,aggregation_type,metadatas_conditions,tests_conditions)
    lQuery = "CALL test_stats_spm_"+m_strPrefixTable+"_get_num_datapoints(";
    lQuery+= TranslateStringToSqlVarChar(consolidationLevel )+ ",";
    lQuery+= TranslateStringToSqlVarChar(consolidationType) + ",";
    lQuery+= TranslateStringToSqlVarChar(lMetadatasConditions) + ",";
    lQuery+= (testConditions.isEmpty() ? "''" : TranslateStringToSqlVarChar("(" + testConditions.join(" AND ") + ")")) + ")";
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }
    numDataPointsMatched = 0;
    if(lDbQuery.first())
    {
        numDataPointsMatched = lDbQuery.value("numDatapoints").toInt();
    }

    if(numDataPointsMatched == 0)
    {
        // no datapoints will be fetched
        return true;
    }

    ///////////////////////////
    // Query the data points //
    ///////////////////////////
    QString statToMonitor;
    // test_stats_spm_wt_get_datapoints(aggregation_level,aggregation_type,metadatas_conditions,tests_conditions)
    lQuery = "CALL test_stats_spm_"+m_strPrefixTable+"_get_datapoints(";
    lQuery+= TranslateStringToSqlVarChar(consolidationLevel )+ ",";
    lQuery+= TranslateStringToSqlVarChar(consolidationType) + ",";
    lQuery+= TranslateStringToSqlVarChar(lMetadatasConditions) + ",";
    lQuery+= (testConditions.isEmpty() ? "''" : TranslateStringToSqlVarChar("(" + testConditions.join(" AND ") + ")")) + ")";
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }

    // Fill the data structure
    while(lDbQuery.next())
    {
        MonitoredItemDesc item = MonitoredItemDesc::CreateItemDesc(
            uniqueKeyRule,
            lDbQuery.value("test_type").toString().toUpper(),
            lDbQuery.value("test_number").toString(),
            lDbQuery.value("test_name").toString(),
            "",
            lDbQuery.value("test_unit").toString(),
            lDbQuery.value("scale").toInt());

        int siteNumber = lDbQuery.value("site_num").toInt();
        for(int i=0; i<statsToMonitor.size(); ++i)
        {
            statToMonitor = statsToMonitor.at(i);
            testToSiteToStatToValues[item][siteNumber].insert(statToMonitor, lDbQuery.value(statToMonitor).toDouble());
        }
        siteList.insert(siteNumber);
    }

    //////////////////////////////////
    // Query the merged data points //
    //////////////////////////////////

    // Not supported for SPM

    return true;
}

bool GexDbPlugin_Galaxy::SPM_FetchDataPointsForCheckOnTrigger(QString testingStage,
                                                              QString productRegexp,
                                                              QString lotId,
                                                              QString sublotId,
                                                              QString waferId,
                                                              const QList<MonitoredItemDesc>& /*testList*/,
                                                              MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                              QString testFlow,
                                                              QString consolidationType,
                                                              QString consolidationLevel,
                                                              QString testInsertion,
                                                              const QList<int>& siteList,
                                                              const QList<QString>& statsList,
                                                              bool /*useGrossDie*/,
                                                              const QDateTime* dateFrom,
                                                              const QDateTime* dateTo,
                                                              const QMap<QString, QStringList>& filtersConditions,
                                                              QString& productList,
                                                              QString& lotList,
                                                              QString& sublotList,
                                                              QString& waferList,
                                                              int& numParts,
                                                              QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > > &testToSiteToStatToDataPoint)
{
    SetTestingStage(testingStage);
    GexDbPlugin_Query lDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString lQuery;
    QString lMetadatasConditions;

    ////////////////////////////
    // Build the where clause //
    ////////////////////////////
    QString lCondition;
    QStringList conditionsSubQueries;
    QStringList lRegExps;
    // Product
    if(!productRegexp.isEmpty())
        lRegExps = productRegexp.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("product_name"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }

    lRegExps.clear();
    // TestFlow
    if(!testFlow.isEmpty())
        lRegExps = testFlow.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("test_flow"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
    }

    lRegExps.clear();
    // TestInsertion
    if(!testInsertion.isEmpty())
        lRegExps = testInsertion.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("test_insertion"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
        lRegExps.clear();
    }

    // Date
    if(dateFrom && dateTo)
    {
        QString lDateFilter = QString("%1#GEXDB#TO#%2").arg(dateFrom->toTime_t()).arg(dateTo->toTime_t());
        GexDbPlugin_Base::Query_BuildSqlString_ExpressionRange(QString("start_t"),lDateFilter,lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
        lRegExps.clear();
    }

    // Lot/SubLot/Wafer filter
    if(!lotId.isEmpty())
        lRegExps = lotId.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("lot_id"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
        lRegExps.clear();
    }
    // Lot/SubLot/Wafer filter
    if(!sublotId.isEmpty())
        lRegExps = sublotId.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("sublot_id"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
        lRegExps.clear();
    }
    // Lot/SubLot/Wafer filter
    if(!waferId.isEmpty())
        lRegExps = waferId.split(',');
    for(int i=0; i<lRegExps.size(); ++i)
    {
        GexDbPlugin_Base::Query_BuildSqlString_StringFilter(QString("wafer_id"), lRegExps.at(i), lCondition);
        conditionsSubQueries.append("(" + lCondition + ")");
    }
    if(conditionsSubQueries.size() > 0)
    {
        if(lMetadatasConditions.size() > 0)
            lMetadatasConditions += " AND ";
        lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
        conditionsSubQueries.clear();
        lRegExps.clear();
    }

    // Other filters
    QString lMetaDataName;
    for(QMap<QString,QStringList>::const_iterator iter = filtersConditions.begin(); iter != filtersConditions.end(); ++iter)
    {
        lMetaDataName = iter.key();
        lRegExps = iter.value();
        if(!lRegExps.isEmpty())
        {
            for(int i=0; i<lRegExps.size(); ++i)
            {
                GexDbPlugin_Base::Query_BuildSqlString_StringFilter(lMetaDataName, lRegExps.at(i), lCondition);
                conditionsSubQueries.append("(" + lCondition + ")");
            }
        }
        if(conditionsSubQueries.size() > 0)
        {
            if(lMetadatasConditions.size() > 0)
                lMetadatasConditions += " AND ";
            lMetadatasConditions += "(" + conditionsSubQueries.join(" OR ") + ")";
            conditionsSubQueries.clear();
            lRegExps.clear();
        }
    }

    QStringList testSubQueries;
    QString lTestsConditions;
    /*
    // Build the test where clause
    for(int i=0; i<testList.size(); ++i)
    {
        if(!testList.at(i).num.contains('+'))
        {
            testSubQueries.append("(test_type='" + testList.at(i).type + "' AND test_number=" + testList.at(i).num + " AND test_name='" + testList.at(i).name + "')");
        }
    }
    if(lTestsConditions.size() > 0)
        lTestsConditions += " AND ";
    lTestsConditions += "(" + testSubQueries.join(" OR ") + ")";
    testSubQueries.clear();
    lRegExps.clear();
    */

    // Build the site clause
    for(int i=0; i<siteList.size(); ++i)
    {
        testSubQueries.append("site_num=" + QString::number(siteList.at(i)));
    }
    if(lTestsConditions.size() > 0)
        lTestsConditions += " AND ";
    lTestsConditions += "(" + testSubQueries.join(" OR ") + ")";
    testSubQueries.clear();
    lRegExps.clear();

    /////////////////////////////
    // Query the matched lists //
    /////////////////////////////
    // test_stats_spm_wt_get_matched_items(IN_aggregation_level,IN_aggregation_type,IN_metadatas_conditions,IN_test_conditions)
    lQuery = "CALL test_stats_spm_"+m_strPrefixTable+"_get_matched_items(";
    lQuery+= TranslateStringToSqlVarChar(consolidationLevel )+ ",";
    lQuery+= TranslateStringToSqlVarChar(consolidationType) + ",";
    lQuery+= TranslateStringToSqlVarChar(lMetadatasConditions) + ",";
    lQuery+= TranslateStringToSqlVarChar(lTestsConditions) + ")";
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }
    productList.clear();
    lotList.clear();
    sublotList.clear();
    waferList.clear();
    numParts = 0;

    if(lDbQuery.first())
    {
        productList = lDbQuery.value("matched_products").toString();
        lotList = lDbQuery.value("matched_lots").toString();
        sublotList = lDbQuery.value("matched_sublots").toString();
        waferList = lDbQuery.value("matched_wafers").toString();
        numParts = lDbQuery.value("num_parts").toInt();
    }

    if(numParts == 0)
    {
        // no datapoints will be fetched
        return true;
    }

    ///////////////////////////
    // Query the data points //
    ///////////////////////////
    QString statToMonitor;
    // test_stats_spm_wt_get_datapoints(IN_aggregation_level,IN_aggregation_type,IN_metadatas_conditions,IN_test_conditions)
    lQuery = "CALL test_stats_spm_"+m_strPrefixTable+"_get_datapoints(";
    lQuery+= TranslateStringToSqlVarChar(consolidationLevel )+ ",";
    lQuery+= TranslateStringToSqlVarChar(consolidationType) + ",";
    lQuery+= TranslateStringToSqlVarChar(lMetadatasConditions) + ",";
    lQuery+= TranslateStringToSqlVarChar(lTestsConditions) + ")";
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }

    // Fill the data structure
    while(lDbQuery.next())
    {
        for(int i=0; i<statsList.size(); ++i)
        {
            statToMonitor = statsList.at(i);
            testToSiteToStatToDataPoint
                    [MonitoredItemDesc::CreateItemDesc(
                        uniqueKeyRule,
                        lDbQuery.value("test_type").toString().toUpper(),
                        lDbQuery.value("test_number").toString(),
                        lDbQuery.value("test_name").toString(),
                        "",
                        "",
                        0)]
                    [lDbQuery.value("site_num").toInt()]
                    .insert(statToMonitor, StatMonDataPoint(lDbQuery.value("product_name").toString(),
                                                        lDbQuery.value("lot_id").toString(),
                                                        lDbQuery.value("sublot_id").toString(),
                                                        lDbQuery.value("wafer_id").toString(),
                                                        0,
                                                        lDbQuery.value("exec_count").toInt(),
                                                        lDbQuery.value("fail_count").toInt(),
                                                        lDbQuery.value("exec_count").toInt() - lDbQuery.value("fail_count").toInt(),
                                                        lDbQuery.value(statToMonitor).toDouble()));
        }
    }

    return true;
}

bool GexDbPlugin_Galaxy::SPM_FetchDataPointsForCheckOnInsertion(QString testingStage,
                                                                int splitlotId,
                                                                const QMap<QString,QString> &filtersMetaData,
                                                                const QList<MonitoredItemDesc> &testList,
                                                                MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                                const QList<int> &siteList,
                                                                const QList<QString> &statsList,
                                                                QString& productList,
                                                                QString& lotList,
                                                                QString& sublotList,
                                                                QString& waferList,
                                                                int& numParts,
                                                                QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > > &testToSiteToStatToDataPoint)
{
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

    ///////////////////////////////////////////////////////////////////////////
    // 1. Subquery: list of lot_id's for given product/time period/meta-data //
    ///////////////////////////////////////////////////////////////////////////

    Query_Empty();

    // Select the splitlot_id
    strField = "Field|"+NormalizeTableName("_splitlot")+".splitlot_id";
    m_strlQuery_Fields.append(strField);

    // Select on product_name,lot_id,sublot_id/wafer_id,nb_parts
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strDbField, strDbTable);
    Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], strDbField, strDbTable);
    if(m_eTestingStage==eFinalTest)
        Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], strDbField, strDbTable);
    else
        Query_AddField(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_DBFIELD_PARTS, strDbField, strDbTable);

    // Set data filter
    strCondition = NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = NormalizeTableName("_splitlot")+".prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);
    // Set filter on splitlot
    strCondition = NormalizeTableName("_splitlot")+".splitlot_id|Numeric|"+QString::number(splitlotId);
    m_strlQuery_ValueConditions.append(strCondition);

    // Build subquery
    if (!Query_BuildSqlString_UsingJoins(lSubSplitlotQuery, false))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, "Query_BuildSqlString_UsingJoins(lSubSplitlotQuery, false)", "Unexpected error trying to create query");
        return false;
    }

    // Construct the group by
    QString lGroupBy = "S.product_name, S.lot_id,";
    if(m_eTestingStage==eFinalTest)
    {
        lGroupBy+= " S.sublot_id,";
    }
    else
    {
        lGroupBy+= " S.wafer_id,";
    }
    lGroupBy+= " S.splitlot_id,";

    ///////////////////////////////////////////
    // 2. Extract matched product, lots & co //
    ///////////////////////////////////////////

    lQuery ="SELECT "
            "   S.product_name AS matched_products, "
            "   S.lot_id AS matched_lots, ";
    if(m_eTestingStage==eFinalTest)
    {
        lQuery+="   CONCAT('(',S.lot_id,',',S.sublot_id,')') AS matched_sublots, ";
    }
    else
    {
        lQuery+="   CONCAT('(',S.lot_id,',',S.wafer_id,')') AS matched_sublots, ";
    }
    lQuery+="   S.nb_parts as nb_parts "
            "FROM "
            "   ("+lSubSplitlotQuery+") S ";
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }
    productList.clear();
    lotList.clear();
    sublotList.clear();
    waferList.clear();
    numParts = 0;

    if(lDbQuery.first())
    {
        productList = lDbQuery.value("matched_products").toString();
        lotList = lDbQuery.value("matched_lots").toString();
        sublotList = lDbQuery.value("matched_sublots").toString();
        waferList = lDbQuery.value("matched_sublots").toString();
        numParts = lDbQuery.value("nb_parts").toUInt();
    }

    if(numParts == 0)
    {
        // no datapoints will be fetched
        return true;
    }

    ///////////////////////////////
    // 3. Extract the datapoints //
    ///////////////////////////////

    // Use TEMPORARY tables to ease the extraction
    // We need to extract each datapoints and then compute the stats on it
    lQuery = "DROP TABLE IF EXISTS "+NormalizeTableName("_SPM_CHECK_TEST_DATAPOINTS");
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }

#ifdef QT_DEBUG
    lQuery = "CREATE"
#else
    lQuery = "CREATE TEMPORARY"
#endif
             " TABLE "+NormalizeTableName("_SPM_CHECK_TEST_DATAPOINTS")
            +" "
             "("
             "  `product_name` varchar(255) NOT NULL,"
             "  `lot_id` varchar(255) NOT NULL,"
             "  `sublot_id` varchar(255) NOT NULL,"
             "  `splitlot_id` varchar(255) NOT NULL COMMENT 'first splitlot referenced by the datapoint',"
             "  `tnum` int(10) unsigned NOT NULL,"
             "  `tname` varchar(255) DEFAULT '',"
             "  `site_no` smallint(5) NOT NULL COMMENT '-1 for MERGE',"
             "  `ll` double DEFAULT NULL,"
             "  `hl` double DEFAULT NULL,"
             "  `exec_count` mediumint(8) unsigned NOT NULL DEFAULT '0',"
             "  `fail_count` mediumint(8) unsigned DEFAULT NULL,"
             "  `min_value` double DEFAULT NULL,"
             "  `max_value` double DEFAULT NULL,"
             "  `sum` double DEFAULT NULL,"
             "  `square_sum` double DEFAULT NULL,"
             "  KEY `product_name` (`product_name`),"
             "  KEY `lot_id` (`lot_id`),"
             "  KEY `sublot_id` (`sublot_id`),"
             "  KEY `tnum` (`tnum`),"
             "  KEY `tname` (`tname`)"
             ") ENGINE=InnoDB DEFAULT CHARSET=latin1";
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }

    QString monitoredItemType = testList[0].type;
    /*
    QStringList testNums;
    for(QList<MonitoredItemDesc>::const_iterator iter = testList.begin(); iter != testList.end(); ++iter)
    {
        if(!(*iter).num.contains('+'))
        {
            testNums.append((*iter).num);
        }
    }

    if(testNums.empty())
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_SPMCheckLimitsError, NULL, "No test found to check");
        return false;
    }
    */

    QStringList siteNums;
    bool splitSites = false;
    bool mergedSites = false;
    for(QList<int>::const_iterator iter = siteList.begin(); iter != siteList.end(); ++iter)
    {
        if((*iter) == -1)
        {
            mergedSites = true;
        }
        else
        {
            splitSites = true;
        }
        siteNums.append(QString::number(*iter));
    }

    if(!mergedSites && !splitSites)
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_SPMCheckLimitsError, NULL, "No site specified, (not even merged ones) to check");
        return false;
    }

    // Extract tests stats from ptest_stats_samples agregate by product/lot/sublot/test/site
    lQuery ="INSERT INTO "+NormalizeTableName("_SPM_CHECK_TEST_DATAPOINTS");
    // if(!testNums.isEmpty())
    {
        // PTR for each site
        lQuery+="           SELECT "+lGroupBy+
                "               TI.tnum, TI.tname, "
                "               TS.site_no AS site_no,"
                "               min(ll) AS ll, max(hl) AS hl,"
                "               sum(TS.exec_count) AS exec_count, sum(TS.fail_count) AS fail_count, "
                "               min(TS.min_value) AS min_value, max(TS.max_value) AS max_value, "
                "               sum(TS.sum) AS sum, sum(TS.square_sum) AS square_sum"
                "           FROM"
                "               ("+lSubSplitlotQuery+") S"
                "               INNER JOIN "
                "               "+NormalizeTableName("_" + monitoredItemType + "TEST_INFO")+" TI"
                "                   ON S.splitlot_id=TI.splitlot_id"
                "               INNER JOIN"
                "               "+NormalizeTableName("_" + monitoredItemType + "TEST_STATS_SAMPLES")+" TS"
                "                   ON S.splitlot_id=TS.splitlot_id AND TI." + monitoredItemType + "test_info_id=TS." + monitoredItemType + "test_info_id"
/*
                "           WHERE "
                "               TI.tnum IN ("+testNums.join(",")+") "
*/
                "           GROUP BY "+lGroupBy+
                "                    TI.tnum, TI.tname,TS.site_no";
    }
    if(!lDbQuery.exec(lQuery))
    {
        SPM_SetQueryError(lDbQuery);
        return false;
    }

    if(splitSites)
    {
        // Compute all other stats for each site agregate by product/lot/sublot/(splitlot)/test/site
        // mean, sigma, cp, cpk
        lQuery = "SELECT product_name, lot_id, sublot_id, splitlot_id, "
                "   tnum, tname, "
                "   site_no, ll, hl, exec_count, fail_count, min_value, max_value, max_value-min_value, "
                "   sum, square_sum, yield, mean, sigma, cp, cpkL, cpkH, "
                "   if(isnull(cpkL),cpkH,if(cpkH<cpkL,cpkH,cpkL)) AS cpk "
                "FROM "
                "( "
                "   SELECT product_name, lot_id, sublot_id, splitlot_id, "
                "       tnum, tname, "
                "       site_no, ll, hl, exec_count, fail_count, min_value, max_value, sum, square_sum, yield, mean, sigma, "
                "       abs(hl-ll)/(6*sigma) AS cp, "
                "       ((sum/exec_count)-ll)/(3*sigma) AS cpkL, "
                "       (hl-(sum/exec_count))/(3*sigma) AS cpkH "
                "   FROM "
                "   ( "
        // PTR for each site
                "       SELECT product_name, lot_id, sublot_id, splitlot_id, "
                "       tnum, tname, "
                "       site_no, ll, hl, exec_count, fail_count, min_value, max_value, sum, square_sum, "
        // compute the yield, mean, sigma
                "       (exec_count-fail_count)/exec_count*100 AS yield, "
                "       sum/exec_count AS mean, "
                "       sqrt(abs((exec_count*square_sum) - pow(sum,2))/(exec_count*(exec_count-1))) AS sigma "
                "       FROM "
                "       ( "
        // Test for all site
                "           SELECT product_name, lot_id, sublot_id, splitlot_id, "
                "               tnum, tname, "
                "               site_no, ll, hl, exec_count, fail_count, min_value, max_value, sum, square_sum "
                "           FROM "+NormalizeTableName("_SPM_CHECK_TEST_DATAPOINTS")+
                "       )T0 "
                "   )T1 "
                ")T2";
        if(!lDbQuery.exec(lQuery))
        {
            SPM_SetQueryError(lDbQuery);
            return false;
        }
        while(lDbQuery.next())
        {
            for(int i=0; i<statsList.size(); ++i)
            {
                testToSiteToStatToDataPoint
                        [MonitoredItemDesc::CreateItemDesc(
                            uniqueKeyRule,
                            monitoredItemType.toUpper(),
                            lDbQuery.value("tnum").toString(),
                            lDbQuery.value("tname").toString(),
                            "",
                            "",
                            0)]
                        [lDbQuery.value("site_no").toInt()]
                        .insert(statsList.at(i), StatMonDataPoint(lDbQuery.value("product_name").toString(),
                                                                lDbQuery.value("lot_id").toString(),
                                                                lDbQuery.value("sublot_id").toString(),
                                                                m_eTestingStage == eWaferTest ? lDbQuery.value("wafer_id").toString() : lDbQuery.value("sublot_id").toString(),
                                                                splitlotId,
                                                                lDbQuery.value("exec_count").toInt(),
                                                                lDbQuery.value("fail_count").toInt(),
                                                                lDbQuery.value("exec_count").toInt() - lDbQuery.value("fail_count").toInt(),
                                                                lDbQuery.value(statsList.at(i)).toDouble()));
            }
        }
    }
    if(mergedSites)
    {
        // Compute all other stats for MERGE site agregate by product/lot/sublot/test
        lQuery ="SELECT product_name, lot_id, sublot_id, splitlot_id, "
                "   tnum, tname, "
                "   site_no, ll, hl, exec_count, fail_count, min_value, max_value, max_value-min_value, "
                "   sum, square_sum, yield, mean, sigma, cp, cpkL, cpkH, "
                "   if(isnull(cpkL),cpkH,if(cpkH<cpkL,cpkH,cpkL)) AS cpk "
                "FROM "
                "( "
                "   SELECT product_name, lot_id, sublot_id, splitlot_id, "
                "       tnum, tname, "
                "       site_no, ll, hl, exec_count, fail_count, min_value, max_value, sum, square_sum, yield, mean, sigma, "
                "       abs(hl-ll)/(6*sigma) AS cp, "
                "       ((sum/exec_count)-ll)/(3*sigma) AS cpkL, "
                "       (hl-(sum/exec_count))/(3*sigma) AS cpkH "
                "   FROM "
                "   ( "
                "       SELECT product_name, lot_id, sublot_id, splitlot_id, "
                "       tnum, tname, "
                "       site_no, ll, hl, exec_count, fail_count, min_value, max_value, sum, square_sum, "
        // compute the yield, mean, sigma
                "       (exec_count-fail_count)/exec_count*100 AS yield, "
                "       sum/exec_count AS mean, "
                "       sqrt(abs((exec_count*square_sum) - pow(sum,2))/(exec_count*(exec_count-1))) AS sigma "
                "       FROM "
                "       ( "
        // test for MERGE site
        // Test for all site
                "           SELECT product_name, lot_id, sublot_id, max(splitlot_id) as splitlot_id, "
                "               tnum, tname, "
                "               -1 as site_no, min(ll) as ll, max(hl) as hl, "
                "               sum(exec_count) as exec_count, sum(fail_count) as fail_count, "
                "               min(min_value) as min_value, max(max_value) as max_value, "
                "               sum(sum) as sum, sum(square_sum) as square_sum "
                "           FROM "+NormalizeTableName("_SPM_CHECK_TEST_DATAPOINTS")+
                "           GROUP BY product_name, lot_id, sublot_id, tnum, tname "
                "       )T0 "
                "   )T1 "
                ")T2";
        if(!lDbQuery.exec(lQuery))
        {
            SPM_SetQueryError(lDbQuery);
            return false;
        }
        while(lDbQuery.next())
        {
            for(int i=0; i<statsList.size(); ++i)
            {
                testToSiteToStatToDataPoint
                        [MonitoredItemDesc::CreateItemDesc(
                            uniqueKeyRule,
                            monitoredItemType.toUpper(),
                            lDbQuery.value("tnum").toString(),
                            lDbQuery.value("tname").toString())]
                        [lDbQuery.value("site_no").toInt()]
                        .insert(statsList.at(i), StatMonDataPoint(lDbQuery.value("product_name").toString(),
                                                                lDbQuery.value("lot_id").toString(),
                                                                lDbQuery.value("sublot_id").toString(),
                                                                m_eTestingStage == eWaferTest ? lDbQuery.value("wafer_id").toString() : lDbQuery.value("sublot_id").toString(),
                                                                splitlotId,
                                                                lDbQuery.value("exec_count").toInt(),
                                                                lDbQuery.value("fail_count").toInt(),
                                                                lDbQuery.value("exec_count").toInt() - lDbQuery.value("fail_count").toInt(),
                                                                lDbQuery.value(statsList.at(i)).toDouble()));
            }
        }
    }

    return true;
}

