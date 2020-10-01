// gexdb_plugin_galaxy_admin_consolidated.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// Consolidated tables/procedures creation/upate
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

// Local includes
#include "gexdb_plugin_galaxy.h"
#include "gexdb_getroot_dialog.h"
#include "import_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include "consolidation_tree_query_filter.h"
#include "consolidation_tree_query_engine.h"
#include "consolidation_tree.h"
#include "consolidation_tree_defines.h"


// Standard includes
#include <math.h>

// Qt includes
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QTextEdit>
#include <QDir>
#include <QProgressBar>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gstdl_utils_c.h>

bool GexDbPlugin_Galaxy::AddConsolidatedFields(const QString & strCastNumber)
{
    QString									strFieldSpec, strDbField, strDbTable;
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping, itStartTField;
    GexDbPlugin_Mapping_Field				clFieldMap, clStartTFieldMap;

    if (!m_pmapFields_GexToRemote)
        return false;


    CTQueryFilter filters;
    QString optionValue, startDateConsolidationRule;
    if (m_eTestingStage == eWaferTest)
        filters.add(CTQueryFilter::FilterTestingStage, "Wafer Sort");
    else if (m_eTestingStage == eFinalTest)
        filters.add(CTQueryFilter::FilterTestingStage, "Final Test");
    filters.add(CTQueryFilter::FilterOption, CT_XML_OPTION_DATE_CONSOLIDATION);  // "date_consolidation" with possible value [min_start_time|max_start_time]

    if (m_pConsolidationTree)
    {
        ConsolidationTreeQueryEngine queryEng(*m_pConsolidationTree);
        if (m_pConsolidationTree->isValid())
        {
            // Get the test conditions
            if (queryEng.findOption(filters, optionValue))
            {
                if (optionValue == "min_start_time")
                    startDateConsolidationRule = "min";
                else if (optionValue == "max_start_time")
                    startDateConsolidationRule = "max";
                else
                {
                    GSET_ERROR1(GexDbPlugin_Base, eDB_Consolidation, NULL, QString("Invalid value in consolidation tree for " + QString(CT_XML_OPTION_DATE_CONSOLIDATION)).toLatin1().constData());
                    return false;
                }
            }
            else
            {
                // Error unvalid option or result (unable to get option)
                GSET_ERROR1(GexDbPlugin_Base, eDB_Consolidation, NULL, QString("Unable to get value for " + QString(CT_XML_OPTION_DATE_CONSOLIDATION)).toLatin1().constData());
                return false;
            }
        }
        else
        {
            // Error Tree unvalid
            GSET_ERROR1(GexDbPlugin_Base, eDB_Consolidation, NULL, QString("Consolidated tree not valid").toLatin1().constData());
            return false;
        }
    }
    else
    {
        // G7 Tree uninstancited
        GSET_ERROR1(GexDbPlugin_Base, eDB_Consolidation, NULL, QString("Consolidated tree not instanciated").toLatin1().constData());
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // FIRST ADD MOST SIGNIFICANT FIELDS
    ////////////////////////////////////////////////////////////////////////////////////
    // PRODUCT_ID
    itMapping = m_pmapFields_GexToRemote->find(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT]);
    if(itMapping != m_pmapFields_GexToRemote->end())
    {
        clFieldMap = itMapping.value();
        strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
        Query_AddField(strFieldSpec, strDbField, strDbTable, true, true);
    }
    // TRACKING_LOT_ID
    itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_TRACKING_LOT);
    if(itMapping != m_pmapFields_GexToRemote->end())
    {
        clFieldMap = itMapping.value();
        strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
        Query_AddField(strFieldSpec, strDbField, strDbTable, true, true);
    }
    // LOT_ID
    itMapping = m_pmapFields_GexToRemote->find(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT]);
    if(itMapping != m_pmapFields_GexToRemote->end())
    {
        clFieldMap = itMapping.value();
        strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
        Query_AddField(strFieldSpec, strDbField, strDbTable, true, true);
    }
    // SUBLOT_ID
    itMapping = m_pmapFields_GexToRemote->find(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT]);
    if(itMapping != m_pmapFields_GexToRemote->end())
    {
        clFieldMap = itMapping.value();
        strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
        Query_AddField(strFieldSpec, strDbField, strDbTable, true, true);
    }
    // WAFER_ID
    itMapping = m_pmapFields_GexToRemote->find(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID]);
    if(itMapping != m_pmapFields_GexToRemote->end())
    {
        clFieldMap = itMapping.value();
        strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
        Query_AddField(strFieldSpec, strDbField, strDbTable, true, true);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // NEXT ADD ALL TIME FIELDS
    ////////////////////////////////////////////////////////////////////////////////////
    // Get mapping for start_t
    itStartTField = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_STARTDATETIME);
    if(itStartTField != m_pmapFields_GexToRemote->end())
    {
        clStartTFieldMap = itStartTField.value();
        // GEXDB_PLUGIN_DBFIELD_STARTDATETIME
        strFieldSpec = "Function=" + clStartTFieldMap.m_strNormalizedName;
        strFieldSpec += "|" + clStartTFieldMap.m_strSqlFullField;
        strFieldSpec += "|" + startDateConsolidationRule + "";
        m_strlQuery_Fields.append(strFieldSpec);
        // GEXDB_PLUGIN_DBFIELD_DAY
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_DAY);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Expression=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clStartTFieldMap.m_strSqlFullField;
            strFieldSpec += "|" + TranslateUnixTimeStampToSqlDateTime("" + startDateConsolidationRule + "(" + clStartTFieldMap.m_strSqlFullField + ")",eDate);
            m_strlQuery_Fields.append(strFieldSpec);
        }
        // GEXDB_PLUGIN_DBFIELD_WEEK
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_WEEK);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Expression=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clStartTFieldMap.m_strSqlFullField;
            strFieldSpec += "|cast("+TranslateUnixTimeStampToSqlDateTime("" + startDateConsolidationRule + "(" + clStartTFieldMap.m_strSqlFullField + ")",eWeek)+ " as " + strCastNumber + ")";
            m_strlQuery_Fields.append(strFieldSpec);
        }
        // GEXDB_PLUGIN_DBFIELD_MONTH
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_MONTH);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Expression=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clStartTFieldMap.m_strSqlFullField;
            strFieldSpec += "|cast("+TranslateUnixTimeStampToSqlDateTime("" + startDateConsolidationRule + "(" + clStartTFieldMap.m_strSqlFullField + ")",eMonth)+ " as " + strCastNumber + ")";
            m_strlQuery_Fields.append(strFieldSpec);
        }
        // GEXDB_PLUGIN_DBFIELD_QUARTER
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_QUARTER);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Expression=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clStartTFieldMap.m_strSqlFullField;
            strFieldSpec += "|" + TranslateUnixTimeStampToSqlDateTime("" + startDateConsolidationRule + "(" + clStartTFieldMap.m_strSqlFullField + ")",eQuarter);
            m_strlQuery_Fields.append(strFieldSpec);
        }
        // GEXDB_PLUGIN_DBFIELD_YEAR
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_YEAR);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Expression=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clStartTFieldMap.m_strSqlFullField;
            strFieldSpec += "|cast("+TranslateUnixTimeStampToSqlDateTime("" + startDateConsolidationRule + "(" + clStartTFieldMap.m_strSqlFullField + ")",eYear)+ " as " + strCastNumber + ")";
            m_strlQuery_Fields.append(strFieldSpec);
        }
        // GEXDB_PLUGIN_DBFIELD_YEAR_WEEK
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_YEAR_WEEK);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Expression=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clStartTFieldMap.m_strSqlFullField;
            strFieldSpec += "|" + TranslateUnixTimeStampToSqlDateTime("" + startDateConsolidationRule + "(" + clStartTFieldMap.m_strSqlFullField + ")",eWYear,eWeek).replace("||","'concatOp'");
            m_strlQuery_Fields.append(strFieldSpec);
        }
        // GEXDB_PLUGIN_DBFIELD_YEAR_MONTH
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_YEAR_MONTH);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Expression=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clStartTFieldMap.m_strSqlFullField;
            strFieldSpec += "|" + TranslateUnixTimeStampToSqlDateTime("" + startDateConsolidationRule + "(" + clStartTFieldMap.m_strSqlFullField + ")",eYear,eMonth).replace("||","'concatOp'");
            m_strlQuery_Fields.append(strFieldSpec);
        }
        // GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Expression=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clStartTFieldMap.m_strSqlFullField;
            strFieldSpec += "|" + TranslateUnixTimeStampToSqlDateTime("" + startDateConsolidationRule + "(" + clStartTFieldMap.m_strSqlFullField + ")",eYear,eQuarter).replace("||","'concatOp'");
            m_strlQuery_Fields.append(strFieldSpec);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // NEXT ADD ALL FACT FIELDS
    ////////////////////////////////////////////////////////////////////////////////////
    if(m_eTestingStage == eFinalTest)
    {
        // GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Function=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clFieldMap.m_strSqlFullField;
            strFieldSpec += "|max";
            m_strlQuery_Fields.append(strFieldSpec);
            // Link conditions
            if(!clFieldMap.m_strSqlLinkName.isEmpty())
                Query_AddLinkCondition(clFieldMap.m_strSqlLinkName);
            //strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
            //Query_AddField(strFieldSpec, strDbField, strDbTable, true, false);
        }
        // GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Function=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clFieldMap.m_strSqlFullField;
            strFieldSpec += "|max";
            m_strlQuery_Fields.append(strFieldSpec);
            // Link conditions
            if(!clFieldMap.m_strSqlLinkName.isEmpty())
                Query_AddLinkCondition(clFieldMap.m_strSqlLinkName);
            //strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
            //Query_AddField(strFieldSpec, strDbField, strDbTable, true, false);
        }
    }
    if(m_eTestingStage == eWaferTest)
    {
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Function=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clFieldMap.m_strSqlFullField;
            strFieldSpec += "|max";
            m_strlQuery_Fields.append(strFieldSpec);
            // Link conditions
            if(!clFieldMap.m_strSqlLinkName.isEmpty())
                Query_AddLinkCondition(clFieldMap.m_strSqlLinkName);
            //strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
            //Query_AddField(strFieldSpec, strDbField, strDbTable, true, false);
        }
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Function=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clFieldMap.m_strSqlFullField;
            strFieldSpec += "|max";
            m_strlQuery_Fields.append(strFieldSpec);
            // Link conditions
            if(!clFieldMap.m_strSqlLinkName.isEmpty())
                Query_AddLinkCondition(clFieldMap.m_strSqlLinkName);
            //strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
            //Query_AddField(strFieldSpec, strDbField, strDbTable, true, false);
        }
        itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE);
        if(itMapping != m_pmapFields_GexToRemote->end())
        {
            clFieldMap = itMapping.value();
            strFieldSpec = "Function=" + clFieldMap.m_strNormalizedName;
            strFieldSpec += "|" + clFieldMap.m_strSqlFullField;
            strFieldSpec += "|max";
            m_strlQuery_Fields.append(strFieldSpec);
            // Link conditions
            if(!clFieldMap.m_strSqlLinkName.isEmpty())
                Query_AddLinkCondition(clFieldMap.m_strSqlLinkName);
            //strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
            //Query_AddField(strFieldSpec, strDbField, strDbTable, true, false);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // NEXT ADD ALL OTHER META-DATA STRING FIELDS
    ////////////////////////////////////////////////////////////////////////////////////
    for(itMapping = m_pmapFields_GexToRemote->begin(); itMapping != m_pmapFields_GexToRemote->end(); itMapping++)
    {
        clFieldMap = itMapping.value();

        // Ignore fields already added
        if(clFieldMap.m_bTime || clFieldMap.m_bFact)
            continue;
        if(clFieldMap.m_strMetaDataName == m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT])
            continue;
        if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_TRACKING_LOT)
            continue;
        if(clFieldMap.m_strMetaDataName == m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT])
            continue;
        if(clFieldMap.m_strMetaDataName == m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT])
            continue;
        if(clFieldMap.m_strMetaDataName == m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID])
            continue;

        // Add other string fields if consolidated flag set
        if(clFieldMap.m_bConsolidated && !clFieldMap.m_bNumeric)
        {
            strFieldSpec = clFieldMap.m_strMetaDataName + "=" + clFieldMap.m_strNormalizedName;
            Query_AddField(strFieldSpec, strDbField, strDbTable, true, true);
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Drop and recreate consolidated tables
// Update indexes
////////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::CreateConsolidatedTables(
        enum TestingStage eTestingStage/*=eUnknownStage*/)
{
    if (!m_pclDatabaseConnector)
        return false;

    QString				strTableName, strLogMessage, strQuery, strSelectQuery;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    QString strTableEngine;
    QString strIndexSuffix;
    QString strCastNumber;

        strIndexSuffix = "";
        strCastNumber = "unsigned ";
        // Get storage engine
        QString	strEngine,strFormat;
        GetStorageEngineName(strEngine,strFormat);
        strTableEngine = " ENGINE=" + strEngine + " " + strFormat + " ";


    if((eTestingStage == eUnknownStage) || (eTestingStage == eFinalTest))
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // 1) Drop and create FT consolidated tables
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eFinalTest);

        ////////////////////////////////////////////////////////////////////////////////////
        // 1.0) Drop ft_consolidated_lot table
        ////////////////////////////////////////////////////////////////////////////////////
        strTableName = NormalizeTableName("_consolidated_lot");
        clQuery.Execute("DROP TABLE IF EXISTS " + strTableName);

        ////////////////////////////////////////////////////////////////////////////////////
        // 1.1) Drop ft_consolidated_sublot table
        ////////////////////////////////////////////////////////////////////////////////////
        strTableName = NormalizeTableName("_consolidated_sublot");
        clQuery.Execute("DROP TABLE IF EXISTS " + strTableName);

        ////////////////////////////////////////////////////////////////////////////////////
        // 1.2) Create ft_consolidated_sublot table
        ////////////////////////////////////////////////////////////////////////////////////
        strTableName = NormalizeTableName("_consolidated_sublot");
        strLogMessage = "Creating/populating " + strTableName+"...";
        InsertIntoUpdateLog(strLogMessage);

        // Empty query
        Query_Empty();

        // Add all consolidated fields
        AddConsolidatedFields(strCastNumber);

        // ADD CONDITIONS
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".prod_data|String|Y");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".lot_id");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".sublot_id");
        m_strlQuery_GroupFields.append("ft_sublot_consolidation.consolidation_name");

        if(!Query_BuildSqlString_UsingJoins(strSelectQuery,false, "", false, GexDbPlugin_Base::eQueryJoin_LeftOuter))
            return false;


        // Use INNER JOIN instead of LEFT OUTER JOIN. This is to avoid to have some line in the ft_consolidated_sublot having
        // a row in the ft_sublot_info but not in ft_sublot_consolidation
        QString strTableSublotConsolidation = NormalizeTableName("_sublot_consolidation");
        if(!m_pclDatabaseConnector->m_strSchemaName.isEmpty() && (strTableSublotConsolidation.indexOf(".") == -1))
        {
            strTableSublotConsolidation = "." + strTableSublotConsolidation;
            strTableSublotConsolidation = m_pclDatabaseConnector->m_strSchemaName + strTableSublotConsolidation;
        }
        strSelectQuery = strSelectQuery.simplified().replace("LEFT OUTER JOIN " + strTableSublotConsolidation,
                                                             "INNER JOIN " + strTableSublotConsolidation, Qt::CaseInsensitive);

        strQuery = "CREATE TABLE " + strTableName + strTableEngine + " AS\n";
        strQuery += strSelectQuery.replace("'concatOp'","||");
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(),
                        clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // Add a PK index
        QString strQueryPK = "ALTER TABLE " + strTableName + " ADD PRIMARY KEY (lot_id, sublot_id,consolidation_name,consolidation_flow)";
        if(!clQuery.Execute(strQueryPK))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQueryPK.left(1024).toLatin1().constData(),
                        clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 3.0) Create ft_consolidated_sublot_inter table
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eFinalTest);
        strTableName = NormalizeTableName("_consolidated_sublot_inter");

        // Drop table first
        clQuery.Execute("DROP TABLE IF EXISTS " + strTableName);

        // Create table
        strLogMessage = "Creating/populating " + strTableName+"...";
        InsertIntoUpdateLog(strLogMessage);

        // -- !! use _consolidated_sublot here will be change just after by a replace.
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".test_insertion|Expression|"+ NormalizeTableName("_sublot_consolidation")+".consolidation_name");

        if(!Query_BuildSqlString_UsingJoins(strSelectQuery,false, "", false, GexDbPlugin_Base::eQueryJoin_LeftOuter))
            return false;
        strQuery = "CREATE TABLE " + NormalizeTableName("_consolidated_sublot") + strTableEngine + " AS\n";
        strQuery += strSelectQuery.replace("'concatOp'","||");

        // Re-cycle ft_consolidated_sublot Query based on ft_sublot_consolidation_inter table
        strQuery.replace("_sublot_consolidation", "_sublot_consolidation_inter");
        strQuery.replace("_consolidated_sublot", "_consolidated_sublot_inter");

        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // Add a PK index
        strQueryPK = "ALTER TABLE " + strTableName + " ADD PRIMARY KEY (lot_id,sublot_id,consolidation_name,consolidation_flow)";
        if(!clQuery.Execute(strQueryPK))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQueryPK.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        InsertIntoUpdateLog("DONE.", false);
    }
    IncrementProgress();

    if((eTestingStage == eUnknownStage) || (eTestingStage == eWaferTest))
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // 2) Drop and create WT consolidated tables
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eWaferTest);

        ////////////////////////////////////////////////////////////////////////////////////
        // 2.0) Drop wt_consolidated_wafer table
        ////////////////////////////////////////////////////////////////////////////////////
        strTableName = NormalizeTableName("_consolidated_wafer");
        clQuery.Execute("DROP TABLE IF EXISTS " + strTableName);

        ////////////////////////////////////////////////////////////////////////////////////
        // 2.1) Create wt_consolidated_wafer table
        ////////////////////////////////////////////////////////////////////////////////////
        strTableName = NormalizeTableName("_consolidated_wafer");
        strLogMessage = "Creating/populating " + strTableName+"...";
        InsertIntoUpdateLog(strLogMessage);

        // Empty query
        Query_Empty();

        // Add all consolidated fields
        AddConsolidatedFields(strCastNumber);

        // ADD CONDITIONS
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".prod_data|String|Y");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".lot_id");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".wafer_id");
        m_strlQuery_GroupFields.append("wt_wafer_consolidation.consolidation_name");

        if(!Query_BuildSqlString_UsingJoins(strSelectQuery,false, "", false, GexDbPlugin_Base::eQueryJoin_LeftOuter))
            return false;

        // Use INNER JOIN instead of LEFT OUTER JOIN. This is to avoid to have some line in the wt_consolidated_wafer having
        // a row in the wt_wafer_info but not in wt_wafer_consolidation
        QString strTableWaferConsolidation = NormalizeTableName("_wafer_consolidation");
        if(!m_pclDatabaseConnector->m_strSchemaName.isEmpty() && (strTableWaferConsolidation.indexOf(".") == -1))
        {
            strTableWaferConsolidation = "." + strTableWaferConsolidation;
            strTableWaferConsolidation = m_pclDatabaseConnector->m_strSchemaName + strTableWaferConsolidation;
        }
        strSelectQuery = strSelectQuery.simplified().replace("LEFT OUTER JOIN " + strTableWaferConsolidation,
                                                             "INNER JOIN " + strTableWaferConsolidation, Qt::CaseInsensitive);

        strQuery = "CREATE TABLE " + strTableName + strTableEngine + " AS\n";
        strQuery += strSelectQuery.replace("'concatOp'","||");
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // Add a PK index
        QString strQueryPK = "ALTER TABLE " + strTableName + " ADD PRIMARY KEY (lot_id,wafer_id,consolidation_name,consolidation_flow)";
        if(!clQuery.Execute(strQueryPK))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQueryPK.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 3.0) Create wt_consolidated_wafer_inter table
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eWaferTest);
        strTableName = NormalizeTableName("_consolidated_wafer_inter");

        // Drop table first
        clQuery.Execute("DROP TABLE IF EXISTS " + strTableName);

        // Create table
        strLogMessage = "Creating/populating " + strTableName+"...";
        InsertIntoUpdateLog(strLogMessage);

        // Re-cycle wt_consolidated_wafer Query based on wt_wafer_consolidation_inter table
        strQuery.replace("_wafer_consolidation", "_wafer_consolidation_inter");
        strQuery.replace("_consolidated_wafer", "_consolidated_wafer_inter");

        // HT: Comment this line below, as we use a INNER JOIN instead a LEFT OUTER JOIN, so line missing in the
        // wt_wafer_consolidation_inter are not anymore inserted into the wt_consolidated_wafer_inter.
        //    // Ensure that the select query will be empty by using an invalid value 'P' for valid_splitlot
        //    strQuery.replace("wt_splitlot.valid_splitlot = 'Y'", "wt_splitlot.valid_splitlot = 'Y'");

        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        // Add a PK index
        strQueryPK = "ALTER TABLE " + strTableName + " ADD PRIMARY KEY (lot_id,wafer_id,consolidation_name,consolidation_flow)";
        if(!clQuery.Execute(strQueryPK))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQueryPK.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        InsertIntoUpdateLog("DONE.", false);

    }
    IncrementProgress();

    // The TestingStage is defined during a HouseKeeping update
    // The TestingStage is unknown during a TDR creation/update
    // During the creation/update, the INDEXes will be automatically created
    // Call the Check only for HouseKeeping
    if(eTestingStage != eUnknownStage)
    {
        QStringList lstIndexesToCheck = GetIndexForConsolidatedTables(eTestingStage);
        return UpdateDb_UpdateIndexes(lstIndexesToCheck);
    }

    return true;
}


////////////////////////////////////////////////////////////////////////////////////
// Check and update indexes/triggers for all consolidated tables
////////////////////////////////////////////////////////////////////////////////////
QStringList GexDbPlugin_Galaxy::GetIndexForConsolidatedTables(
        enum TestingStage eTestingStage/*=eUnknownStage*/)
{
    QStringList lstIndexesToCheck;
    if (!m_pclDatabaseConnector)
        return lstIndexesToCheck;

    QString             lQuery, lValue;
    GexDbPlugin_Query   clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));


    QString lPrefixTable;
    if((eTestingStage == eUnknownStage) || (eTestingStage == eFinalTest))
    {
        lPrefixTable = GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX;

        lstIndexesToCheck.append("product_name|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("tracking_lot_id|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("sublot_id|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("lot_id|"+lPrefixTable+"consolidated%");

        lstIndexesToCheck.append("tracking_lot_id|"+lPrefixTable+"lot");
        lstIndexesToCheck.append("wt_tracking_lot_id|"+lPrefixTable+"die_tracking");
        lstIndexesToCheck.append("ft_tracking_lot_id|"+lPrefixTable+"die_tracking");
        lstIndexesToCheck.append("ft_tracking_lot_id|az_consolidated_tl%");

        lstIndexesToCheck.append("start_t|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("year_nb|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("year_and_week|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("year_and_month|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("year_and_quarter|"+lPrefixTable+"consolidated%");

        lstIndexesToCheck.append("start_t|az_consolidated_tl");
        lstIndexesToCheck.append("year_nb|az_consolidated_tl");
        lstIndexesToCheck.append("year_and_week|az_consolidated_tl");
        lstIndexesToCheck.append("year_and_month|az_consolidated_tl");
        lstIndexesToCheck.append("year_and_quarter|az_consolidated_tl");

        lstIndexesToCheck.append("start_t|az_consolidated_tl_data");
        lstIndexesToCheck.append("year_nb|az_consolidated_tl_data");
        lstIndexesToCheck.append("year_and_week|az_consolidated_tl_data");
        lstIndexesToCheck.append("year_and_month|az_consolidated_tl_data");
        lstIndexesToCheck.append("year_and_quarter|az_consolidated_tl_data");

        // Get the list of MetaData with INDEX from the xx_splitlot_metadata
        lValue = "SELECT distinct S.column_name from information_schema.statistics S WHERE S.table_schema='"
                + m_pclDatabaseConnector->m_strDatabaseName + "' AND S.table_name = '"
                + lPrefixTable + "splitlot_metadata' and S.index_name != 'PRIMARY'";
        // Then reproduce the index for all xx_consolidated_xx
        lQuery = "SELECT distinct S.table_name, S.column_name from information_schema.columns S WHERE S.table_schema='"
                + m_pclDatabaseConnector->m_strDatabaseName + "' AND S.table_name LIKE ('"
                + lPrefixTable + "consolidated%') AND S.column_name IN ("+
                lValue+")";
        if(clQuery.Execute(lQuery))
        {
            while(clQuery.Next())
            {
                lValue = clQuery.value("column_name").toString()+"|"+clQuery.value("table_name").toString();
                if(lstIndexesToCheck.contains(lValue))
                    continue;
                lstIndexesToCheck.append(lValue);
            }
        }
    }
    if((eTestingStage == eUnknownStage) || (eTestingStage == eWaferTest))
    {
        lPrefixTable = GEXDB_PLUGIN_GALAXY_WTEST_TABLE_PREFIX;

        lstIndexesToCheck.append("product_name|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("tracking_lot_id|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("wafer_id|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("sublot_id|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("lot_id|"+lPrefixTable+"consolidated%");

        lstIndexesToCheck.append("tracking_lot_id|"+lPrefixTable+"lot");
        lstIndexesToCheck.append("wt_tracking_lot_id|"+lPrefixTable+"die_tracking");
        lstIndexesToCheck.append("ft_tracking_lot_id|"+lPrefixTable+"die_tracking");
        lstIndexesToCheck.append("ft_tracking_lot_id|az_consolidated_tl%");

        lstIndexesToCheck.append("start_t|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("year_nb|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("year_and_week|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("year_and_month|"+lPrefixTable+"consolidated%");
        lstIndexesToCheck.append("year_and_quarter|"+lPrefixTable+"consolidated%");

        lstIndexesToCheck.append("start_t|az_consolidated_tl");
        lstIndexesToCheck.append("year_nb|az_consolidated_tl");
        lstIndexesToCheck.append("year_and_week|az_consolidated_tl");
        lstIndexesToCheck.append("year_and_month|az_consolidated_tl");
        lstIndexesToCheck.append("year_and_quarter|az_consolidated_tl");

        lstIndexesToCheck.append("start_t|az_consolidated_tl_data");
        lstIndexesToCheck.append("year_nb|az_consolidated_tl_data");
        lstIndexesToCheck.append("year_and_week|az_consolidated_tl_data");
        lstIndexesToCheck.append("year_and_month|az_consolidated_tl_data");
        lstIndexesToCheck.append("year_and_quarter|az_consolidated_tl_data");

        // Get the list of MetaData with INDEX from the xx_splitlot_metadata
        lValue = "SELECT distinct S.column_name from information_schema.statistics S WHERE S.table_schema='"
                + m_pclDatabaseConnector->m_strDatabaseName + "' AND S.table_name = '"
                + lPrefixTable + "splitlot_metadata' and S.index_name != 'PRIMARY'";
        // Then reproduce the index for all xx_consolidated_xx
        lQuery = "SELECT distinct S.table_name, S.column_name from information_schema.columns S WHERE S.table_schema='"
                + m_pclDatabaseConnector->m_strDatabaseName + "' AND S.table_name LIKE ('"
                + lPrefixTable + "consolidated%') AND S.column_name IN ("+
                lValue+")";
        if(clQuery.Execute(lQuery))
        {
            while(clQuery.Next())
            {
                lValue = clQuery.value("column_name").toString()+"|"+clQuery.value("table_name").toString();
                if(lstIndexesToCheck.contains(lValue))
                    continue;
                lstIndexesToCheck.append(lValue);
            }
        }
    }

    return lstIndexesToCheck;
}

void GexDbPlugin_Galaxy::buildQueryForWConsolidateProcedures(QString &strQuery,  QString &strSelectQuery, const QString &strTableName)
{
    strSelectQuery = strSelectQuery.replace("'concatOp'","||");
    // For Oracle
    // table wt_wafer_consolidation is mutating
    // new values are not enabled in the wt_wafer_consolidation table during the execution of the trigger
    // rebuild the wt_wafer_consolidation for the query
    QString strDbTable = NormalizeTableName("_wafer_consolidation");
    if(!m_pclDatabaseConnector->m_strSchemaName.isEmpty() && (strDbTable.indexOf(".") == -1))
    {
        strDbTable = "." + strDbTable;
        strDbTable = m_pclDatabaseConnector->m_strSchemaName + strDbTable;
    }

    strQuery = "(SELECT "
               "IN_LotID                 as lot_id,\n"
               "IN_WaferID               as wafer_id,\n"
               "IN_NbParts               as nb_parts,\n"
               "IN_NbPartsGood           as nb_parts_good,\n"
               "IN_ConsolidatedDataType  as consolidated_data_type,\n"
               "IN_ConsolidationName     as consolidation_name,\n"
               "IN_ConsolidationFlow     as consolidation_flow\n"
               "FROM dual) wt_wafer_consolidation";
    strSelectQuery = strSelectQuery.replace(strDbTable,strQuery);


    strQuery =  "CREATE PROCEDURE %1(\n";
    strQuery += "   IN  OLD_LotID                varchar(255),\n";
    strQuery += "   IN  OLD_WaferID              varchar(255),\n";
    strQuery += "   IN  OLD_ConsolidationName    varchar(255),\n";
    strQuery += "   IN  IN_LotID                 varchar(255),\n";
    strQuery += "   IN  IN_WaferID               varchar(255),\n";
    strQuery += "   IN  IN_NbParts               int,\n";
    strQuery += "   IN  IN_NbPartsGood           int,\n";
    strQuery += "   IN  IN_ConsolidatedDataType  varchar(255),\n";
    strQuery += "   IN  IN_ConsolidationName     varchar(255),\n";
    strQuery += "   IN  IN_ConsolidationFlow     varchar(255))\n";
    strQuery += "BEGIN\n";
    strQuery += "   DECLARE Message      varchar(1024);\n";
    strQuery += "   DECLARE LogMessage   varchar(1024);\n";
    strQuery += "   DECLARE NbParts      varchar(20);\n";
    strQuery += "\n";
    strQuery += "   -- Log message\n";
    strQuery += "SELECT 'Update wt_consolidated_wafer: LotID=' into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, IN_LotID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', WaferID=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, IN_WaferID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', ConsolidationName=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, IN_ConsolidationName) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', LotID=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, OLD_LotID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', WaferID=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, OLD_WaferID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', ConsolidationName=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, OLD_ConsolidationName) into LogMessage from dual;\n";
    strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(NOW(),'%1',LogMessage);\n";
    strQuery += "\n";
    strQuery += "   -- Remove old row\n";
    strQuery += "DELETE FROM " + strTableName + "\n";
    strQuery += " where lot_id=OLD_LotID\n";
    strQuery += " and wafer_id=OLD_WaferID\n";
    strQuery += " and consolidation_name=OLD_ConsolidationName;\n";
    strQuery += "\n";
    strQuery += "   -- Add new row\n";
    strQuery += "INSERT INTO " + strTableName + "\n";
    strQuery += strSelectQuery;
    strQuery += ";\nEND\n";


    strSelectQuery = strSelectQuery.replace("'concatOp'","||");
    // For Oracle
    // table wt_wafer_consolidation is mutating
    // new values are not enabled in the wt_wafer_consolidation table during the execution of the trigger
    // rebuild the wt_wafer_consolidation for the query
    strDbTable = NormalizeTableName("_wafer_consolidation");
    if(!m_pclDatabaseConnector->m_strSchemaName.isEmpty() && (strDbTable.indexOf(".") == -1))
    {
        strDbTable = "." + strDbTable;
        strDbTable = m_pclDatabaseConnector->m_strSchemaName + strDbTable;
    }

    strQuery = "(SELECT "
               "IN_LotID                 as lot_id,\n"
               "IN_WaferID               as wafer_id,\n"
               "IN_NbParts               as nb_parts,\n"
               "IN_NbPartsGood           as nb_parts_good,\n"
               "IN_ConsolidatedDataType  as consolidated_data_type,\n"
               "IN_ConsolidationName     as consolidation_name,\n"
               "IN_ConsolidationFlow     as consolidation_flow\n"
               "FROM dual) wt_wafer_consolidation";
    strSelectQuery = strSelectQuery.replace(strDbTable,strQuery);


    strQuery =  "CREATE PROCEDURE %1(\n";
    strQuery += "   IN  OLD_LotID                varchar(255),\n";
    strQuery += "   IN  OLD_WaferID              varchar(255),\n";
    strQuery += "   IN  OLD_ConsolidationName    varchar(255),\n";
    strQuery += "   IN  IN_LotID                 varchar(255),\n";
    strQuery += "   IN  IN_WaferID               varchar(255),\n";
    strQuery += "   IN  IN_NbParts               int,\n";
    strQuery += "   IN  IN_NbPartsGood           int,\n";
    strQuery += "   IN  IN_ConsolidatedDataType  varchar(255),\n";
    strQuery += "   IN  IN_ConsolidationName     varchar(255),\n";
    strQuery += "   IN  IN_ConsolidationFlow     varchar(255))\n";
    strQuery += "BEGIN\n";
    strQuery += "   DECLARE Message      varchar(1024);\n";
    strQuery += "   DECLARE LogMessage   varchar(1024);\n";
    strQuery += "   DECLARE NbParts      varchar(20);\n";
    strQuery += "\n";
    strQuery += "   -- Log message\n";
    strQuery += "SELECT 'Update wt_consolidated_wafer: LotID=' into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, IN_LotID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', WaferID=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, IN_WaferID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', ConsolidationName=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, IN_ConsolidationName) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', LotID=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, OLD_LotID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', WaferID=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, OLD_WaferID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', ConsolidationName=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, OLD_ConsolidationName) into LogMessage from dual;\n";
    strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(NOW(),'%1',LogMessage);\n";
    strQuery += "\n";
    strQuery += "   -- Remove old row\n";
    strQuery += "DELETE FROM " + strTableName + "\n";
    strQuery += " where lot_id=OLD_LotID\n";
    strQuery += " and wafer_id=OLD_WaferID\n";
    strQuery += " and consolidation_name=OLD_ConsolidationName;\n";
    strQuery += "\n";
    strQuery += "   -- Add new row\n";
    strQuery += "INSERT INTO " + strTableName + "\n";
    strQuery += strSelectQuery;
    strQuery += ";\nEND\n";

}

void GexDbPlugin_Galaxy::buildQueryForFTConsolidateProcedures(QString &strQuery,  QString &strSelectQuery, const QString &strTableName)
{
    QString									strDbTable;
    strSelectQuery = strSelectQuery.replace("'concatOp'","||");
    // For Oracle
    // table ft_sublot_info is mutating
    // new values are not enabled in the ft_sublot_consolidation table during the execution of the trigger
    // rebuild the ft_sublot_consolidation for the query
    strDbTable = NormalizeTableName("_sublot_consolidation");
    if(!m_pclDatabaseConnector->m_strSchemaName.isEmpty() && (strDbTable.indexOf(".") == -1))
    {
        strDbTable = "." + strDbTable;
        strDbTable = m_pclDatabaseConnector->m_strSchemaName + strDbTable;
    }

    strQuery = "(SELECT "
               "IN_LotID                   as lot_id, "
               "IN_SublotID                as sublot_id, "
               "IN_NbParts                 as nb_parts, "
               "IN_NbPartsGood             as nb_parts_good, "
               "IN_ConsolidatedDataType    as consolidated_data_type,\n"
               "IN_ConsolidationName       as consolidation_name,\n"
               "IN_ConsolidationFlow       as consolidation_flow\n"
               "FROM dual) ft_sublot_consolidation";

    strSelectQuery = strSelectQuery.replace(strDbTable,strQuery);

    strQuery =  "CREATE PROCEDURE %1(\n";
    strQuery += "   IN  OLD_LotID                   varchar(255),\n";
    strQuery += "   IN  OLD_SublotID                varchar(255),\n";
    strQuery += "   IN  OLD_ConsolidationName       varchar(255),\n";
    strQuery += "	IN	IN_LotID                    varchar(255),\n";
    strQuery += "	IN	IN_SublotID                 varchar(255),\n";
    strQuery += "	IN	IN_NbParts                  int,\n";
    strQuery += "	IN	IN_NbPartsGood              int,\n";
    strQuery += "   IN  IN_ConsolidatedDataType     varchar(255),\n";
    strQuery += "   IN  IN_ConsolidationName        varchar(255),\n";
    strQuery += "   IN  IN_ConsolidationFlow        varchar(255))\n";
    strQuery += "BEGIN\n";
    strQuery += "	DECLARE Message		varchar(1024);\n";
    strQuery += "	DECLARE LogMessage	varchar(1024);\n";
    strQuery += "	DECLARE NbParts		varchar(20);\n";
    strQuery += "\n";
    strQuery += "	-- Log message\n";
    strQuery += "select 'Update ft_consolidated_lot: LotID=' into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, IN_LotID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', SublotID=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, IN_SublotID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', ConsolidationName=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, IN_ConsolidationName) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', LotID=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, OLD_LotID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', SublotID=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, OLD_SublotID) into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, ', ConsolidationName=') into LogMessage from dual;\n";
    strQuery += "select concat(LogMessage, OLD_ConsolidationName) into LogMessage from dual;\n";
    strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(NOW(),'%1',LogMessage);\n";
    strQuery += "\n";
    strQuery += "   -- Transform OLD consolidation name if required\n";
    strQuery += "SELECT CASE WHEN OLD_ConsolidationName = ''\n";
    strQuery += "            THEN 'n/a'\n";
    strQuery += "            ELSE OLD_ConsolidationName END\n";
    strQuery += "       INTO OLD_ConsolidationName FROM dual;\n";
    strQuery += "\n";
    strQuery += "	-- Remove old row\n";
    strQuery += "DELETE FROM " + strTableName + "\n";
    strQuery += " where lot_id=OLD_LotID\n";
    strQuery += " and sublot_id=OLD_SublotID\n";
    strQuery += " and consolidation_name=OLD_ConsolidationName;\n";
    strQuery += "\n";
    strQuery += "	-- Add new row\n";
    strQuery += "INSERT INTO " + strTableName + "\n";
    strQuery += strSelectQuery;
    strQuery += ";\nEND\n";

}

bool GexDbPlugin_Galaxy::CreateConsolidatedUpdateProcedures(enum TestingStage eTestingStage/*=eUnknownStage*/)
{
    QString									strTableName, strProcedureName, strLogMessage, strQuery, strSelectQuery;
    // QString									strDbTable;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query						clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    if(m_pclDatabaseConnector->IsSQLiteDB())
        return true;

    QString strCastNumber;

        strCastNumber = "unsigned ";

    // TD-78

    if((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // 1) Drop and create FT consolidated update stored procedures
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eFinalTest);

        ////////////////////////////////////////////////////////////////////////////////////
        // 1.0) Drop ft_consolidated_lot_update stored procedure
        ////////////////////////////////////////////////////////////////////////////////////
        strProcedureName = NormalizeTableName("_consolidated_lot_update");
        clQuery.Execute("DROP PROCEDURE IF EXISTS " + strProcedureName);

        ////////////////////////////////////////////////////////////////////////////////////
        // 1.1) Drop ft_consolidated_sublot_update stored procedure
        ////////////////////////////////////////////////////////////////////////////////////
        strProcedureName = NormalizeTableName("_consolidated_sublot_update");
        clQuery.Execute("DROP PROCEDURE IF EXISTS " + strProcedureName);

        ////////////////////////////////////////////////////////////////////////////////////
        // 1.2) Create ft_consolidated_sublot_update stored procedure
        ////////////////////////////////////////////////////////////////////////////////////
        strTableName = NormalizeTableName("_consolidated_sublot");
        strProcedureName = NormalizeTableName("_consolidated_sublot_update");
        strLogMessage = "Creating stored procedure " + strProcedureName+"...";
        InsertIntoUpdateLog(strLogMessage);

        // Empty query
        Query_Empty();

        // Add all consolidated fields
        AddConsolidatedFields(strCastNumber);

        // ADD CONDITIONS
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".prod_data|String|Y");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".lot_id|Expression|IN_LotID");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".sublot_id|Expression|IN_SublotID");
        m_strlQuery_ValueConditions.append("ft_sublot_consolidation.consolidation_name|Expression|IN_ConsolidationName");
        m_strlQuery_ValueConditions.append("ft_sublot_consolidation.consolidation_flow|Expression|IN_ConsolidationFlow");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".lot_id");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".sublot_id");
        m_strlQuery_GroupFields.append("ft_sublot_consolidation.consolidation_name");
        m_strlQuery_GroupFields.append("ft_sublot_consolidation.consolidation_flow");
        // m_strlQuery_ValueConditions.append("ft_splitlot.test_insertion|Expression|IN_ConsolidationName");
        if(!Query_BuildSqlString_UsingJoins(strSelectQuery,false, "", false, GexDbPlugin_Base::eQueryJoin_LeftOuter))
            return false;

        buildQueryForFTConsolidateProcedures(strQuery, strSelectQuery, strTableName);

        if(!clQuery.Execute(strQuery.arg(strProcedureName)))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 3) Create ft_consolidated_sublot_i_update stored procedure
        ////////////////////////////////////////////////////////////////////////////////////

        // Empty query
        Query_Empty();

        // Add all consolidated fields
        AddConsolidatedFields(strCastNumber);

        // ADD CONDITIONS
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".prod_data|String|Y");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".lot_id|Expression|IN_LotID");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".sublot_id|Expression|IN_SublotID");
        m_strlQuery_ValueConditions.append("ft_sublot_consolidation.consolidation_name|Expression|IN_ConsolidationName");
        m_strlQuery_ValueConditions.append("ft_sublot_consolidation.consolidation_flow|Expression|IN_ConsolidationFlow");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".lot_id");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".sublot_id");
        m_strlQuery_GroupFields.append("ft_sublot_consolidation.consolidation_name");
        m_strlQuery_GroupFields.append("ft_sublot_consolidation.consolidation_flow");
        m_strlQuery_ValueConditions.append("ft_splitlot.test_insertion|Expression|IN_ConsolidationName");
        if(!Query_BuildSqlString_UsingJoins(strSelectQuery,false, "", false, GexDbPlugin_Base::eQueryJoin_LeftOuter))
            return false;

        buildQueryForFTConsolidateProcedures(strQuery, strSelectQuery, strTableName);

        strProcedureName = NormalizeTableName("_consolidated_sublot_i_update");

        // Drop procedure first
        clQuery.Execute("DROP PROCEDURE IF EXISTS " + strProcedureName);

        strLogMessage = "Creating stored procedure " + strProcedureName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery.replace("_consolidated_sublot", "_consolidated_sublot_inter");
        strQuery.replace("_sublot_consolidation", "_sublot_consolidation_inter");
        // m_strlQuery_ValueConditions.append("ft_splitlot.test_insertion|Expression|IN_ConsolidationName");
        if(!clQuery.Execute(strQuery.arg(strProcedureName)))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.arg(strProcedureName).left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }
    IncrementProgress();

    if((eTestingStage==eUnknownStage) || (eTestingStage==eWaferTest))
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // 2) Create wt_consolidated_wafer_update stored procedure
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eWaferTest);
        strTableName = NormalizeTableName("_consolidated_wafer");
        strProcedureName = NormalizeTableName("_consolidated_wafer_update");

        // Drop procedure first
        clQuery.Execute("DROP PROCEDURE IF EXISTS " + strProcedureName);

        // Create procedure
        strLogMessage = "Creating stored procedure " + strProcedureName+"...";
        InsertIntoUpdateLog(strLogMessage);

        // Empty query
        Query_Empty();

        // Add all consolidated fields
        AddConsolidatedFields(strCastNumber);

        // ADD CONDITIONS
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".prod_data|String|Y");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".lot_id|Expression|IN_LotID");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".wafer_id|Expression|IN_WaferID");
        m_strlQuery_ValueConditions.append("wt_wafer_consolidation.consolidation_name|Expression|IN_ConsolidationName");
        m_strlQuery_ValueConditions.append("wt_wafer_consolidation.consolidation_flow|Expression|IN_ConsolidationFlow");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".lot_id");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".wafer_id");
        m_strlQuery_GroupFields.append("wt_wafer_consolidation.consolidation_name");
        m_strlQuery_GroupFields.append("wt_wafer_consolidation.consolidation_flow");
        if(!Query_BuildSqlString_UsingJoins(strSelectQuery,false, "", false, GexDbPlugin_Base::eQueryJoin_LeftOuter))
            return false;

        buildQueryForWConsolidateProcedures(strQuery, strSelectQuery, strTableName);

        if(!clQuery.Execute(strQuery.arg(strProcedureName)))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.arg(strProcedureName).left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 3) Create wt_consolidated_wafer_i_update stored procedure
        ////////////////////////////////////////////////////////////////////////////////////
        // Empty query
        Query_Empty();

        // Add all consolidated fields
        AddConsolidatedFields(strCastNumber);

        // ADD CONDITIONS
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".prod_data|String|Y");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".lot_id|Expression|IN_LotID");
        m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".wafer_id|Expression|IN_WaferID");
        m_strlQuery_ValueConditions.append("wt_wafer_consolidation.consolidation_name|Expression|IN_ConsolidationName");
        m_strlQuery_ValueConditions.append("wt_wafer_consolidation.consolidation_flow|Expression|IN_ConsolidationFlow");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".lot_id");
        m_strlQuery_GroupFields.append(NormalizeTableName("_splitlot")+".wafer_id");
        m_strlQuery_GroupFields.append("wt_wafer_consolidation.consolidation_name");
        m_strlQuery_GroupFields.append("wt_wafer_consolidation.consolidation_flow");

        if(!Query_BuildSqlString_UsingJoins(strSelectQuery,false, "", false, GexDbPlugin_Base::eQueryJoin_LeftOuter))
            return false;

        buildQueryForWConsolidateProcedures(strQuery, strSelectQuery, strTableName);



        strProcedureName = NormalizeTableName("_consolidated_wafer_i_update");

        // Drop procedure first
        clQuery.Execute("DROP PROCEDURE IF EXISTS " + strProcedureName);

        strLogMessage = "Creating stored procedure " + strProcedureName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery.replace("_consolidated_wafer", "_consolidated_wafer_inter");
        strQuery.replace("_wafer_consolidation", "_wafer_consolidation_inter");

        if(!clQuery.Execute(strQuery.arg(strProcedureName)))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.arg(strProcedureName).left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

    }
    IncrementProgress();

    return true;
}

bool GexDbPlugin_Galaxy::DropConsolidationTriggers(
        enum TestingStage eTestingStage/*=eUnknownStage*/,
        QString* pstrRootUser /*= NULL*/,
        QString* pstrRootPassword /*= NULL*/)
{
    QString						strTriggerName, strLogMessage, strQuery, strValue;
    if (!m_pclDatabaseConnector)
        return false;
    QSqlDatabase				clSqlDatabase = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName);
    GexDbPlugin_Connector		pclDatabaseConnector(m_strPluginName, this);
    QStringList					strlTriggers;

    if(m_pclDatabaseConnector->IsSQLiteDB())
        return true;

    // Check if root user/password specified
    if(pstrRootUser)
    {
        pclDatabaseConnector = *m_pclDatabaseConnector;
        pclDatabaseConnector.m_strUserName_Admin = *pstrRootUser;
        pclDatabaseConnector.m_strPassword_Admin = *pstrRootPassword;
        pclDatabaseConnector.m_strDatabaseName = m_pclDatabaseConnector->m_strDatabaseName;
        pclDatabaseConnector.m_strSchemaName = m_pclDatabaseConnector->m_strSchemaName;
        pclDatabaseConnector.m_strHost_Name = "localhost";
        pclDatabaseConnector.m_strHost_IP = "127.0.0.1";
        pclDatabaseConnector.SetAdminLogin(true);
        if(!pclDatabaseConnector.Connect())
            return false;
        clSqlDatabase = QSqlDatabase::database(pclDatabaseConnector.m_strConnectionName);
    }

    GexDbPlugin_Query	clQuery(this, clSqlDatabase);

    if(pstrRootUser && m_pclDatabaseConnector->IsMySqlDB())
    {
        // GRANT TRIGGER PRIVILEGE
        // on localhost
        strQuery = "GRANT TRIGGER ON " + m_pclDatabaseConnector->m_strSchemaName + ".* TO '" + m_pclDatabaseConnector->m_strUserName_Admin + "'@'localhost'";
        clQuery.Execute(strQuery);
        // for all other hosts
        strQuery = "GRANT TRIGGER ON " + m_pclDatabaseConnector->m_strSchemaName + ".* TO '" + m_pclDatabaseConnector->m_strUserName_Admin + "'@'%'";
        clQuery.Execute(strQuery);
        clQuery.Execute("FLUSH PRIVILEGES");
    }

    // Get list of triggers
        strQuery = "SELECT trigger_name, definer FROM information_schema.triggers WHERE trigger_schema='"+m_pclDatabaseConnector->m_strSchemaName+"'";
    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }

    SetAttribute("TriggersDefiner","");
    while(clQuery.Next())
    {
        if(!clQuery.isNull(0))
        {
            strValue = clQuery.value(0).toString();
            // table can be stored as 'table_name' or 'owner.table_name'
            // Add the 2 syntaxes
            strlTriggers.append(strValue);
            strlTriggers.append(m_pclDatabaseConnector->m_strUserName_Admin+"."+strValue);
        }
        if(m_pclDatabaseConnector->IsMySqlDB())
            SetAttribute("TriggersDefiner",clQuery.value(1).toString());
    }
    if(strlTriggers.isEmpty())
        return true;

    strLogMessage = "o Dropping consolidated triggers.";
    InsertIntoUpdateLog(strLogMessage);

    ////////////////////////////////////////////////////////////////////////////////////
    // 1) Drop triggers for ft_consolidated_lot
    ////////////////////////////////////////////////////////////////////////////////////
    SetTestingStage(eFinalTest);

    ////////////////////////////////////////////////////////////////////////////////////
    // 1.0) Drop insert trigger for ft_consolidated_lot
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_lot_insert_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 1.1) Drop update trigger for ft_consolidated_lot
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_lot_update_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 2) Drop triggers for ft_consolidated_sublot
    ////////////////////////////////////////////////////////////////////////////////////
    SetTestingStage(eFinalTest);

    ////////////////////////////////////////////////////////////////////////////////////
    // 2.0) Drop insert trigger for ft_consolidated_sublot
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_sublot_info_insert_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 2.1) Drop update trigger for ft_consolidated_sublot
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_sublot_info_update_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 2.2) Drop insert trigger for ft_consolidated_sublot
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_sublot_cons_insert_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 2.3) Drop update trigger for ft_consolidated_sublot
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_sublot_cons_update_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 2.4) Drop delete trigger for ft_consolidated_sublot
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_sublot_cons_delete_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ///////////////////////////////////////////////////////////////////////////////////
    // 3) Drop triggers for ft_consolidated_sublot_inter
    ////////////////////////////////////////////////////////////////////////////////////
    SetTestingStage(eFinalTest);

    ////////////////////////////////////////////////////////////////////////////////////
    // 3.0) Drop insert trigger for ft_consolidated_sublot_inter
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_sublot_cons_i_insert_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 3.1) Drop update trigger for ft_consolidated_sublot_inter
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_sublot_cons_i_update_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 3.2) Drop delete trigger for ft_consolidated_sublot_inter
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_sublot_cons_i_delete_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 4) Drop triggers for wt_consolidated_wafer
    ////////////////////////////////////////////////////////////////////////////////////
    SetTestingStage(eWaferTest);

    ////////////////////////////////////////////////////////////////////////////////////
    // 4.0) Drop insert trigger for wt_consolidated_wafer
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_wafer_cons_insert_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eWaferTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 4.1) Drop update trigger for wt_consolidated_wafer
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_wafer_cons_update_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eWaferTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 4.2) Drop delete trigger for wt_consolidated_wafer
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_wafer_cons_delete_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eWaferTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 5) Drop triggers for wt_consolidated_wafer_inter
    ////////////////////////////////////////////////////////////////////////////////////
    SetTestingStage(eWaferTest);

    ////////////////////////////////////////////////////////////////////////////////////
    // 5.0) Drop insert trigger for wt_consolidated_wafer_inter
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_wafer_cons_i_insert_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eWaferTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 5.1) Drop update trigger for wt_consolidated_wafer_inter
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_wafer_cons_i_update_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eWaferTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 5.2) Drop delete trigger for wt_consolidated_wafer_inter
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_wafer_cons_i_delete_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eWaferTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 6) Drop triggers for wt_consolidated_wafer for old version
    ////////////////////////////////////////////////////////////////////////////////////
    SetTestingStage(eWaferTest);

    ////////////////////////////////////////////////////////////////////////////////////
    // 6.0) Drop insert trigger for wt_consolidated_wafer
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_wafer_info_insert_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eWaferTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // 6.1) Drop update trigger for wt_consolidated_wafer
    ////////////////////////////////////////////////////////////////////////////////////
    strTriggerName = NormalizeTableName("_wafer_info_update_trigger");
    if(((eTestingStage==eUnknownStage) || (eTestingStage==eWaferTest))
            && (strlTriggers.contains(strTriggerName, Qt::CaseInsensitive)))
    {
        strLogMessage = "Dropping trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);
        strQuery = "DROP TRIGGER " + strTriggerName;
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            if(pstrRootUser == NULL)
                WriteDebugMessageFile(clQuery.lastError().text());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }
    IncrementProgress();

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidationTriggers(QString* pstrRootUser /*= NULL*/,
                                                     QString* pstrRootPassword /*= NULL*/,
                                                     enum TestingStage eTestingStage /*= eUnknownStage*/)
{
    QString						strTableName, strTriggerName, strLogMessage, strQuery;
    if (!m_pclDatabaseConnector)
        return false;
    QSqlDatabase				clSqlDatabase = QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName);
    GexDbPlugin_Connector		pclDatabaseConnector(m_strPluginName, this);
    QString						strDefiner;

    if(m_pclDatabaseConnector->IsSQLiteDB())
        return true;

    // Only during the last update when CurrentVersion = LastBuild-1
    // or when the CurrentVersion=LastBuild
    if(GEXDB_DB_VERSION_BUILD > (m_uiDbVersionBuild+1))
        return true;


    // TD-78

    // Check if root user/password specified
    if(pstrRootUser)
    {
        pclDatabaseConnector = *m_pclDatabaseConnector;
        pclDatabaseConnector.m_strUserName_Admin = *pstrRootUser;
        pclDatabaseConnector.m_strPassword_Admin = *pstrRootPassword;
        pclDatabaseConnector.m_strDatabaseName = m_pclDatabaseConnector->m_strDatabaseName;
        pclDatabaseConnector.m_strSchemaName = m_pclDatabaseConnector->m_strSchemaName;
        pclDatabaseConnector.m_strHost_Name = "localhost";
        pclDatabaseConnector.m_strHost_IP = "127.0.0.1";
        pclDatabaseConnector.SetAdminLogin(true);
        if(!pclDatabaseConnector.Connect())
            return false;
        clSqlDatabase = QSqlDatabase::database(pclDatabaseConnector.m_strConnectionName);

    }

    GexDbPlugin_Query	clQuery(this, clSqlDatabase);

    if(m_pclDatabaseConnector->IsMySqlDB())
    {
        if(pstrRootUser)
        {

            strDefiner = "DEFINER='"+m_pclDatabaseConnector->m_strUserName_Admin+"'";
            // GRANT TRIGGER PRIVILEGE
            // on localhost
            strQuery = "GRANT TRIGGER ON " + m_pclDatabaseConnector->m_strSchemaName + ".* TO '" + m_pclDatabaseConnector->m_strUserName_Admin + "'@'localhost'";
            clQuery.Execute(strQuery);
            // for all other hosts
            strQuery = "GRANT TRIGGER ON " + m_pclDatabaseConnector->m_strSchemaName + ".* TO '" + m_pclDatabaseConnector->m_strUserName_Admin + "'@'%'";
            clQuery.Execute(strQuery);
            clQuery.Execute("FLUSH PRIVILEGES");
        }
    }
    if(!GetAttribute("TriggersDefiner").toString().isEmpty())
        strDefiner = "DEFINER='"+GetAttribute("TriggersDefiner").toString().replace("@","'@'")+"'";

    if((eTestingStage==eUnknownStage) || (eTestingStage==eFinalTest))
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // 1) Create triggers for FT consolidation
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eFinalTest);

        ////////////////////////////////////////////////////////////////////////////////////
        // 1.0) Create insert trigger for ft_consolidated_sublot
        ////////////////////////////////////////////////////////////////////////////////////
        strTableName = NormalizeTableName("_sublot_consolidation");
        strTriggerName = NormalizeTableName("_sublot_cons_insert_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER IF EXISTS " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER INSERT ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";
            // Update wt_sublot_info
            strQuery += "UPDATE " + NormalizeTableName("_sublot_info") + "\n"
                                                                         " SET nb_parts = NEW.nb_parts,\n"
                                                                         "nb_parts_good = NEW.nb_parts_good\n"
                                                                         "WHERE lot_id=NEW.lot_id AND sublot_id=NEW.sublot_id;\n";

            // For insert trigger, there is no OLD value for lot_id and sublot_id
            // Use the NEW.lot
            strQuery += "IF (NEW.nb_parts > 0) THEN\n";
            strQuery += "call " + NormalizeTableName("_consolidated_sublot_update") + "(\n"
                                                                                      "NEW.lot_id,\n"
                                                                                      "NEW.sublot_id,\n"
                                                                                      "NEW.consolidation_name,\n"
                                                                                      "NEW.lot_id,\n"
                                                                                      "NEW.sublot_id,\n"
                                                                                      "NEW.nb_parts,\n"
                                                                                      "NEW.nb_parts_good,\n"
                                                                                      "NEW.consolidated_data_type,\n"
                                                                                      "NEW.consolidation_name,\n"
                                                                                      "NEW.consolidation_flow\n"
                                                                                      ");\n";
            strQuery += "call " + NormalizeTableName("_consolidated_tl_update") + "(NEW.lot_id);\n";
            strQuery += "END IF;";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 1.1) Create update trigger for ft_consolidated_sublot
        ////////////////////////////////////////////////////////////////////////////////////
        strTriggerName = NormalizeTableName("_sublot_cons_update_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER UPDATE ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";

            // Update wt_sublot_info
            strQuery += "UPDATE " + NormalizeTableName("_sublot_info") + "\n"
                                                                         "SET nb_parts = NEW.nb_parts,\n"
                                                                         "nb_parts_good = NEW.nb_parts_good\n"
                                                                         "WHERE lot_id=NEW.lot_id AND sublot_id=NEW.sublot_id;\n";

            strQuery += "IF (NEW.nb_parts > 0) THEN\n";
            strQuery += "call " + NormalizeTableName("_consolidated_sublot_update") + "(\n"
                                                                                      "OLD.lot_id,\n"
                                                                                      "OLD.sublot_id,\n"
                                                                                      "OLD.consolidation_name,\n"
                                                                                      "NEW.lot_id,\n"
                                                                                      "NEW.sublot_id,\n"
                                                                                      "NEW.nb_parts,\n"
                                                                                      "NEW.nb_parts_good,\n"
                                                                                      "NEW.consolidated_data_type,\n"
                                                                                      "NEW.consolidation_name,\n"
                                                                                      "NEW.consolidation_flow\n"
                                                                                      ");\n";
            strQuery += "call " + NormalizeTableName("_consolidated_tl_update") + "(NEW.lot_id);\n";
            strQuery += "END IF;";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 1.2) Create delete trigger for ft_consolidated_sublot
        ////////////////////////////////////////////////////////////////////////////////////
        strTriggerName = NormalizeTableName("_sublot_cons_delete_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER DELETE ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";
            strQuery += " -- Transform OLD.consolidation_name if required\n";
            strQuery += "DECLARE consolidationName varchar(255);\n";
            strQuery += "\n";
            strQuery += "SELECT CASE WHEN OLD.consolidation_name = '' \n";
            strQuery += "            THEN 'n/a' \n";
            strQuery += "            ELSE OLD.consolidation_name END\n";
            strQuery += "INTO consolidationName FROM dual;\n";
            strQuery += "\n";
            strQuery += " -- Delete old consolidated data from ft_consolidated_sublot\n";
            strQuery += "DELETE FROM ";
            strQuery += NormalizeTableName("_consolidated_sublot") + " \n"
                                                                     "WHERE lot_id=OLD.lot_id AND \n"
                                                                     "sublot_id=OLD.sublot_id AND \n"
                                                                     "consolidation_name=consolidationName;\n";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 2) Create triggers for ft_cconsolidated_sublot_inter
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eFinalTest);
        strTableName = NormalizeTableName("_sublot_consolidation_inter");

        ////////////////////////////////////////////////////////////////////////////////////
        // 2.0) Create insert trigger for ft_cconsolidated_sublot_inter
        ////////////////////////////////////////////////////////////////////////////////////
        strTriggerName = NormalizeTableName("_sublot_cons_i_insert_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER IF EXISTS " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName + "...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER INSERT ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";

            // For insert trigger, there is no OLD value for lot_id and sublot_id
            // Use the NEW.lot
            strQuery += "IF (NEW.nb_parts > 0) THEN\n";
            strQuery += "call " + NormalizeTableName("_consolidated_sublot_i_update") + "(\n"
                                                                                        "NEW.lot_id,\n"
                                                                                        "NEW.sublot_id,\n"
                                                                                        "NEW.consolidation_name,\n"
                                                                                        "NEW.lot_id,\n"
                                                                                        "NEW.sublot_id,\n"
                                                                                        "NEW.nb_parts,\n"
                                                                                        "NEW.nb_parts_good,\n"
                                                                                        "NEW.consolidated_data_type,\n"
                                                                                        "NEW.consolidation_name,\n"
                                                                                        "NEW.consolidation_flow\n"
                                                                                        ");\n";
            strQuery += "END IF;";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 2.1) Create update trigger for ft_consolidated_sublot_inter
        ////////////////////////////////////////////////////////////////////////////////////
        strTriggerName = NormalizeTableName("_sublot_cons_i_update_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER UPDATE ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";

            strQuery += "IF (NEW.nb_parts > 0) THEN\n";
            strQuery += "call " + NormalizeTableName("_consolidated_sublot_i_update") + "(\n"
                                                                                        "OLD.lot_id,\n"
                                                                                        "OLD.sublot_id,\n"
                                                                                        "OLD.consolidation_name,\n"
                                                                                        "NEW.lot_id,\n"
                                                                                        "NEW.sublot_id,\n"
                                                                                        "NEW.nb_parts,\n"
                                                                                        "NEW.nb_parts_good,\n"
                                                                                        "NEW.consolidated_data_type,\n"
                                                                                        "NEW.consolidation_name,\n"
                                                                                        "NEW.consolidation_flow\n"
                                                                                        ");\n";
            strQuery += "END IF;";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 2.2) Create delete trigger for ft_consolidated_sublot_inter
        ////////////////////////////////////////////////////////////////////////////////////
        strTriggerName = NormalizeTableName("_sublot_cons_i_delete_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER DELETE ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";

            strQuery += " -- Transform OLD.consolidation_name if required\n";
            strQuery += "DECLARE consolidationName varchar(255);\n";
            strQuery += "\n";
            strQuery += "SELECT CASE WHEN OLD.consolidation_name = '' \n";
            strQuery += "            THEN 'n/a' \n";
            strQuery += "            ELSE OLD.consolidation_name END\n";
            strQuery += "INTO consolidationName FROM dual;\n";
            strQuery += "\n";
            strQuery += " -- Delete old consolidated data from ft_consolidated_sublot_inter\n";
            strQuery += "DELETE FROM ";
            strQuery += NormalizeTableName("_consolidated_sublot_inter") + " \n"
                                                                           "WHERE lot_id=OLD.lot_id AND \n"
                                                                           "sublot_id=OLD.sublot_id AND \n"
                                                                           "consolidation_name=consolidationName;\n";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    if((eTestingStage==eUnknownStage) || (eTestingStage==eWaferTest))
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // 3) Create triggers for wt_consolidated_wafer
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eWaferTest);
        strTableName = NormalizeTableName("_wafer_consolidation");

        ////////////////////////////////////////////////////////////////////////////////////
        // 3.0) Create insert trigger for wt_consolidated_wafer
        ////////////////////////////////////////////////////////////////////////////////////
        // For wt_consolidated_wafer
        strTriggerName = NormalizeTableName("_wafer_cons_insert_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER INSERT ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";

            // Update wt_wafer_info
            strQuery += "UPDATE " + NormalizeTableName("_wafer_info") + "\n"
                                                                        " SET nb_parts = NEW.nb_parts,\n"
                                                                        "nb_parts_good = NEW.nb_parts_good\n"
                                                                        "WHERE lot_id=NEW.lot_id AND wafer_id=NEW.wafer_id;\n";

            strQuery += "IF (NEW.nb_parts > 0) THEN\n";
            strQuery += "call " + NormalizeTableName("_consolidated_wafer_update") + "(\n"
                                                                                     "NEW.lot_id,\n"
                                                                                     "NEW.wafer_id,\n"
                                                                                     "NEW.consolidation_name,\n"
                                                                                     "NEW.lot_id,\n"
                                                                                     "NEW.wafer_id,\n"
                                                                                     "NEW.nb_parts,\n"
                                                                                     "NEW.nb_parts_good,\n"
                                                                                     "NEW.consolidated_data_type,\n"
                                                                                     "NEW.consolidation_name,\n"
                                                                                     "NEW.consolidation_flow\n"
                                                                                     ");\n";
            strQuery += "call " + NormalizeTableName("_consolidated_tl_update") + "(NEW.lot_id);\n";
            strQuery += "END IF;";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 3.1) Create update trigger for wt_consolidated_wafer
        ////////////////////////////////////////////////////////////////////////////////////
        strTriggerName = NormalizeTableName("_wafer_cons_update_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER UPDATE ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";

            // Update wt_wafer_info
            strQuery += "UPDATE " + NormalizeTableName("_wafer_info") + "\n"
                                                                        "SET nb_parts = NEW.nb_parts,\n"
                                                                        "nb_parts_good = NEW.nb_parts_good\n"
                                                                        "WHERE lot_id=NEW.lot_id AND wafer_id=NEW.wafer_id;\n";

            strQuery += "IF (NEW.nb_parts > 0) THEN\n";
            strQuery += "call " + NormalizeTableName("_consolidated_wafer_update") + "(\n"
                                                                                     "OLD.lot_id,\n"
                                                                                     "OLD.wafer_id,\n"
                                                                                     "OLD.consolidation_name,\n"
                                                                                     "NEW.lot_id,\n"
                                                                                     "NEW.wafer_id,\n"
                                                                                     "NEW.nb_parts,\n"
                                                                                     "NEW.nb_parts_good,\n"
                                                                                     "NEW.consolidated_data_type,\n"
                                                                                     "NEW.consolidation_name,\n"
                                                                                     "NEW.consolidation_flow\n"
                                                                                     ");\n";
            strQuery += "call " + NormalizeTableName("_consolidated_tl_update") + "(NEW.lot_id);\n";
            strQuery += "END IF;";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 3.2) Create delete trigger for wt_consolidated_wafer
        ////////////////////////////////////////////////////////////////////////////////////
        strTriggerName = NormalizeTableName("_wafer_cons_delete_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER DELETE ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";

            strQuery += " -- Transform OLD.consolidation_name if required\n";
            strQuery += "DECLARE consolidationName varchar(255);\n";
            strQuery += "\n";
            strQuery += "SELECT CASE WHEN OLD.consolidation_name = '' \n";
            strQuery += "            THEN 'n/a' \n";
            strQuery += "            ELSE OLD.consolidation_name END\n";
            strQuery += "INTO consolidationName FROM dual;\n";
            strQuery += "\n";
            strQuery += " -- Delete old consolidated data from wt_consolidated_wafer\n";
            strQuery += "DELETE FROM ";
            strQuery += NormalizeTableName("_consolidated_wafer") + " \n"
                                                                    "WHERE lot_id=OLD.lot_id AND \n"
                                                                    "wafer_id=OLD.wafer_id AND \n"
                                                                    "consolidation_name=consolidationName;\n";


        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 4) Create triggers for wt_consolidated_wafer_inter table
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eWaferTest);
        strTableName = NormalizeTableName("_wafer_consolidation_inter");

        ////////////////////////////////////////////////////////////////////////////////////
        // 4.0) Create insert trigger for wt_consolidated_wafer_inter table
        ////////////////////////////////////////////////////////////////////////////////////
        // For wt_consolidated_wafer
        strTriggerName = NormalizeTableName("_wafer_cons_i_insert_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER INSERT ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";

            strQuery += "IF (NEW.nb_parts > 0) THEN\n";
            strQuery += "call " + NormalizeTableName("_consolidated_wafer_i_update") + "(\n"
                                                                                       "NEW.lot_id,\n"
                                                                                       "NEW.wafer_id,\n"
                                                                                       "NEW.consolidation_name,\n"
                                                                                       "NEW.lot_id,\n"
                                                                                       "NEW.wafer_id,\n"
                                                                                       "NEW.nb_parts,\n"
                                                                                       "NEW.nb_parts_good,\n"
                                                                                       "NEW.consolidated_data_type,\n"
                                                                                       "NEW.consolidation_name,\n"
                                                                                       "NEW.consolidation_flow\n"
                                                                                       ");\n";
            strQuery += "END IF;";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 4.1) Create update trigger for wt_consolidated_wafer_inter table
        ////////////////////////////////////////////////////////////////////////////////////
        strTriggerName = NormalizeTableName("_wafer_cons_i_update_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER UPDATE ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";

            strQuery += "IF (NEW.nb_parts > 0) THEN\n";
            strQuery += "call " + NormalizeTableName("_consolidated_wafer_i_update") + "(\n"
                                                                                       "OLD.lot_id,\n"
                                                                                       "OLD.wafer_id,\n"
                                                                                       "OLD.consolidation_name,\n"
                                                                                       "NEW.lot_id,\n"
                                                                                       "NEW.wafer_id,\n"
                                                                                       "NEW.nb_parts,\n"
                                                                                       "NEW.nb_parts_good,\n"
                                                                                       "NEW.consolidated_data_type,\n"
                                                                                       "NEW.consolidation_name,\n"
                                                                                       "NEW.consolidation_flow\n"
                                                                                       ");\n";
            strQuery += "END IF;";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);

        ////////////////////////////////////////////////////////////////////////////////////
        // 4.2) Create delete trigger for wt_consolidated_wafer_inter table
        ////////////////////////////////////////////////////////////////////////////////////
        strTriggerName = NormalizeTableName("_wafer_cons_i_delete_trigger");

        // Drop trigger first
        clQuery.Execute("DROP TRIGGER " + strTriggerName);

        // Create trigger
        strLogMessage = "Creating trigger " + strTriggerName+"...";
        InsertIntoUpdateLog(strLogMessage);

        strQuery =  "CREATE "+strDefiner+" TRIGGER " + strTriggerName + " AFTER DELETE ON " + strTableName + "\n";
        strQuery += "FOR EACH ROW\nBEGIN\n";

            strQuery += " -- Transform OLD.consolidation_name if required\n";
            strQuery += "DECLARE consolidationName varchar(255);\n";
            strQuery += "\n";
            strQuery += "SELECT CASE WHEN OLD.consolidation_name = '' \n";
            strQuery += "            THEN 'n/a' \n";
            strQuery += "            ELSE OLD.consolidation_name END\n";
            strQuery += "INTO consolidationName FROM dual;\n";
            strQuery += "\n";
            strQuery += " -- Delete old consolidated data from wt_consolidated_wafer_inter\n";
            strQuery += "DELETE FROM ";
            strQuery += NormalizeTableName("_consolidated_wafer_inter") + " \n"
                                                                          "WHERE lot_id=OLD.lot_id AND \n"
                                                                          "wafer_id=OLD.wafer_id AND \n"
                                                                          "consolidation_name=consolidationName;\n";

        strQuery += "END\n";
        if(!clQuery.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }
        InsertIntoUpdateLog("DONE.", false);
    }

    return true;
}

///////////////////////////////////////////////////////////
// eTestingStage:
//		0 : to update all testingstage
//		else : to update only one testing stage
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateConsolidationProcess(
        int eTestingStage/*=0*/)
{
    if (!m_pclDatabaseConnector)
        return false;

    // No consolidation allowed for characterization DB
    if (IsCharacTdr())
    {
        //InsertIntoUpdateLog("TDR Characterization DB does not use the consolidation process.");
        RemoveDbUpdateSteps(eUpdateConsTree|eUpdateConsTriggers|eUpdateConsTables|eUpdateConsProcedures);
        return true;
    }

    QString	strRootUser, strRootPassword, lErrorMsg, lLogMsg;

    ResetProgress(false);
    SetMaxProgress(35);

    // To update m_uiDbVersionBuild
    ConnectToCorporateDb();

    // Only if the database is uptodate
    // Only check indexes when recent version
    if(m_uiDbVersionBuild < GEXDB_DB_VERSION_BUILD_B17)
        //if(m_uiDbVersionBuild != GEXDB_DB_VERSION_BUILD)
    {
        lLogMsg = "Database is not uptodate";
        InsertIntoUpdateLog(lLogMsg);
        return false;
    }

    // Update db_status to record the start
    unsigned int uiUpdateFlags;
    GetDbUpdateSteps(uiUpdateFlags);
    if(!(  (uiUpdateFlags&eUpdateConsTriggers)
           ||(uiUpdateFlags&eUpdateConsTables)
           ||(uiUpdateFlags&eUpdateConsProcedures)))
    {
        if (!AddDbUpdateSteps(eUpdateConsTriggers|eUpdateConsTables|eUpdateConsProcedures))
            return false;
        GetDbUpdateSteps(uiUpdateFlags);
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1) Reload Metadata
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(!LoadMetaData())
        return false;

    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 2) Drop consolidation triggers if exist (to avoid updating any consolidated tables during the update process)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(uiUpdateFlags&eUpdateConsTriggers)
    {
        InsertIntoUpdateLog("o Drop consolidation triggers...");
        if(!DropConsolidationTriggers((enum TestingStage)eTestingStage))
        {
            InsertIntoUpdateLog("TRIGGER drop FAILED, trying with root access.");

            GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

            if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
                return false;

            // Retrieve values from dialog
            strRootUser = clGexdbGetrootDialog.GetRootUsername();
            strRootPassword = clGexdbGetrootDialog.GetRootPassword();

            if(!DropConsolidationTriggers((enum TestingStage)eTestingStage, &strRootUser, &strRootPassword))
                goto updatedb_cons_process_error_noset;

        }
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 3) Re-Create consolidated tables
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(uiUpdateFlags&eUpdateConsTables)
    {
        InsertIntoUpdateLog("o Create consolidated tables...");
        if(!CreateConsolidatedTables((enum TestingStage) eTestingStage))
            goto updatedb_cons_process_error_noset;
    }
    IncrementProgress();


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 4) Re-Create consolidated update procedures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(uiUpdateFlags&eUpdateConsProcedures)
    {
        InsertIntoUpdateLog("o Create consolidated update procedures...");
        if(!CreateConsolidatedUpdateProcedures((enum TestingStage) eTestingStage))
            goto updatedb_cons_process_error_noset;
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 5) Create TL tables (TrackingLot level consolidated tables)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(uiUpdateFlags&eUpdateConsTables)
    {
        InsertIntoUpdateLog("o Create consolidated tables (tracking_lot level)...");
        if(!CreateConsolidatedTables_TL((enum TestingStage) eTestingStage))
            goto updatedb_cons_process_error_noset;
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 6) Create TL tables update stored procedures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(uiUpdateFlags&eUpdateConsProcedures)
    {
        InsertIntoUpdateLog("o Create consolidated update procedures (tracking_lot level)...");
        if(!CreateConsolidatedUpdateProcedures_TL((enum TestingStage) eTestingStage))
            goto updatedb_cons_process_error_noset;
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 7) Create AZ tables (AZ level consolidated tables)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(uiUpdateFlags&eUpdateConsTables)
    {
        InsertIntoUpdateLog("o Create consolidated AZ tables...");
        if(!CreateConsolidatedTables_AZ())
            goto updatedb_cons_process_error_noset;
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 8) Create AZ tables update stored procedures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(uiUpdateFlags&eUpdateConsProcedures)
    {
        InsertIntoUpdateLog("o Create consolidated AZ update procedures...");
        if(!CreateConsolidatedUpdateProcedure_AZ())
            goto updatedb_cons_process_error_noset;
    }
    IncrementProgress();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 9) Re-Create consolidated triggers
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(uiUpdateFlags&eUpdateConsTriggers)
    {
        InsertIntoUpdateLog("o Create consolidated triggers...");
        if(!CreateConsolidationTriggers(NULL, NULL, (enum TestingStage) eTestingStage))
        {
            InsertIntoUpdateLog("TRIGGER generation FAILED, trying with root access.");

            // Get root user+password if not already done for the drop
            if(strRootUser.isEmpty())
            {
                GexdbGetrootDialog clGexdbGetrootDialog(m_pGexSkin, mParentWidget);

                if(clGexdbGetrootDialog.exec() == QDialog::Rejected)
                    return false;

                // Retrieve values from dialog
                strRootUser = clGexdbGetrootDialog.GetRootUsername();
                strRootPassword = clGexdbGetrootDialog.GetRootPassword();
            }

            if(!CreateConsolidationTriggers(&strRootUser, &strRootPassword, (enum TestingStage) eTestingStage))
                goto updatedb_cons_process_error_noset;

        }
    }
    IncrementProgress();
    goto updatedb_cons_process;


updatedb_cons_process_error_noset:
    lErrorMsg = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
    InsertIntoUpdateLog(lErrorMsg);
    lLogMsg = "Status = ERROR (";
    lLogMsg+= lErrorMsg.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    lLogMsg+= ").";
    InsertIntoUpdateLog(lLogMsg);
    return false;

updatedb_cons_process:
    InsertIntoUpdateLog("Consolidation process updated");

    // Update db_status to record the end
    return RemoveDbUpdateSteps(eUpdateConsTriggers|eUpdateConsTables|eUpdateConsProcedures);
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::UpdateAllConsolidatedTableStartTimeFields(
        int eTestingStage,
        const QString& /*startDateConsolidationRule*/)
{
    return UpdateConsolidationProcess(eTestingStage);
    //  SetTestingStage(eTestingStage);

    //  if ((eTestingStage == eUnknownStage) || (eTestingStage == eElectTest))
    //    return false;

    //  if(!CreateConsolidatedUpdateProcedures(NULL, NULL, (enum TestingStage)eTestingStage))
    //    return false;
    //  if(!CreateConsolidatedUpdateProcedures_TL(NULL, NULL,(enum TestingStage) eTestingStage))
    //    return false;

    //  ///////////////////////////////////////////////////////////
    //  /// For each consolidated tables
    //  ///////////////////////////////////////////////////////////
    //  // _consolidated_tl
    //  if (!UpdateConsolidatedTableStartTimeFields(NormalizeTableName("_consolidated_tl"), eTestingStage, startDateConsolidationRule))
    //      return false;
    //  if (eTestingStage == eWaferTest)
    //  {
    //      // _consolidated_wafer
    //      if (!UpdateConsolidatedTableStartTimeFields(NormalizeTableName("_consolidated_wafer"), eTestingStage, startDateConsolidationRule))
    //          return false;
    //      // _consolidated_wafer_inter
    //      if (!UpdateConsolidatedTableStartTimeFields(NormalizeTableName("_consolidated_wafer_inter"), eTestingStage, startDateConsolidationRule))
    //          return false;
    //  }
    //  else if (eTestingStage == eFinalTest)
    //  {
    //      // _consolidated_lot
    //      if (!UpdateConsolidatedTableStartTimeFields(NormalizeTableName("_consolidated_lot"), eTestingStage, startDateConsolidationRule))
    //          return false;
    //  }

    //  return true;
}

bool GexDbPlugin_Galaxy::UpdateConsolidatedTableStartTimeFields(
        const QString& consolidatedTable,
        int eTestingStage,
        const QString& startDateConsolidationRule)
{
    if ((eTestingStage == eUnknownStage) || (eTestingStage == eElectTest))
        return false;

    QString				strQuery, strCastNumber;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

        strCastNumber = "unsigned";

    // Set testing stage
    SetTestingStage(eTestingStage);

    // TD-78

    QString strQueryClauseWafer;
    QString strLotIdField = consolidatedTable.toLower().startsWith("az_") ? "ft_lot_id" : "lot_id";
    if(consolidatedTable.contains("_wafer",Qt::CaseInsensitive))
        strQueryClauseWafer =  " AND " + NormalizeTableName("_splitlot") + ".wafer_id=" + consolidatedTable + ".wafer_id";

    strQuery = "UPDATE " + consolidatedTable + "\n";
    ////////////////////////////////////////////////////////////////////////////////////
    // UPDATE ALL TIME FIELDS
    ////////////////////////////////////////////////////////////////////////////////////

    // GEXDB_PLUGIN_DBFIELD_STARTDATETIME
    strQuery += "SET start_t=\n"
                "(\n"
                "SELECT " + startDateConsolidationRule + "(start_t)\n"
                                                         "FROM " + NormalizeTableName("_splitlot") + "\n"
                                                                                                     "WHERE\n"
            + NormalizeTableName("_splitlot") + ".lot_id=" + consolidatedTable + "." + strLotIdField + " \n"
            + strQueryClauseWafer + "\n"
                                    "),\n";
    // GEXDB_PLUGIN_DBFIELD_DAY
    strQuery += "day=\n"
                "(\n"
                "SELECT " + TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule + "(start_t)",eDate) + "\n"
                                                                                                                  "FROM " + NormalizeTableName("_splitlot") + "\n"
                                                                                                                                                              "WHERE\n"
            + NormalizeTableName("_splitlot") + ".lot_id=" + consolidatedTable + "." + strLotIdField + " \n"
            + strQueryClauseWafer + "\n"
                                    "),\n";
    // GEXDB_PLUGIN_DBFIELD_WEEK
    strQuery += "week_nb=\n"
                "(\n"
                "SELECT cast(" + TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule + "(start_t)",eWeek) + " as " + strCastNumber + ")\n"
                                                                                                                                                "FROM " + NormalizeTableName("_splitlot") + "\n"
                                                                                                                                                                                            "WHERE\n"
            + NormalizeTableName("_splitlot") + ".lot_id=" + consolidatedTable + "." + strLotIdField + " \n"
            + strQueryClauseWafer + "\n"
                                    "),\n";
    // GEXDB_PLUGIN_DBFIELD_MONTH
    strQuery += "month_nb=\n"
                "(\n"
                "SELECT cast(" + TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule + "(start_t)",eMonth) + " as " + strCastNumber + ")\n"
                                                                                                                                                 "FROM " + NormalizeTableName("_splitlot") + "\n"
                                                                                                                                                                                             "WHERE\n"
            + NormalizeTableName("_splitlot") + ".lot_id=" + consolidatedTable + "." + strLotIdField + " \n"
            + strQueryClauseWafer + "\n"
                                    "),\n";
    // GEXDB_PLUGIN_DBFIELD_QUARTER
    strQuery += "quarter_nb=\n"
                "(\n"
                "SELECT " + TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule + "(start_t)",eQuarter) + "\n"
                                                                                                                     "FROM " + NormalizeTableName("_splitlot") + "\n"
                                                                                                                                                                 "WHERE\n"
            + NormalizeTableName("_splitlot") + ".lot_id=" + consolidatedTable + "." + strLotIdField + " \n"
            + strQueryClauseWafer + "\n"
                                    "),\n";
    // GEXDB_PLUGIN_DBFIELD_YEAR
    strQuery += "year_nb=\n"
                "(\n"
                "SELECT cast("+TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule + "(start_t)",eYear) + " as " + strCastNumber + ")\n"
                                                                                                                                              "FROM " + NormalizeTableName("_splitlot") + "\n"
                                                                                                                                                                                          "WHERE\n"
            + NormalizeTableName("_splitlot") + ".lot_id=" + consolidatedTable + "." + strLotIdField + " \n"
            + strQueryClauseWafer + "\n"
                                    "),\n";
    // GEXDB_PLUGIN_DBFIELD_YEAR_WEEK
    strQuery += "year_and_week=\n"
                "(\n"
                "SELECT " + TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule + "(start_t)",eWYear,eWeek) + "\n"
                                                                                                                         "FROM " + NormalizeTableName("_splitlot") + "\n"
                                                                                                                                                                     "WHERE\n"
            + NormalizeTableName("_splitlot") + ".lot_id=" + consolidatedTable + "." + strLotIdField + " \n"
            + strQueryClauseWafer + "\n"
                                    "),\n";
    // GEXDB_PLUGIN_DBFIELD_YEAR_MONTH
    strQuery += "year_and_month=\n"
                "(\n"
                "SELECT " + TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule + "(start_t)",eYear,eMonth) + "\n"
                                                                                                                         "FROM " + NormalizeTableName("_splitlot") + "\n"
                                                                                                                                                                     "WHERE\n"
            + NormalizeTableName("_splitlot") + ".lot_id=" + consolidatedTable + "." + strLotIdField + " \n"
            + strQueryClauseWafer + "\n"
                                    "),\n";
    // GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER
    strQuery += "year_and_quarter=\n"
                "(\n"
                "SELECT " + TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule + "(start_t)",eYear,eQuarter) + "\n"
                                                                                                                           "FROM " + NormalizeTableName("_splitlot") + "\n"
                                                                                                                                                                       "WHERE\n"
            + NormalizeTableName("_splitlot") + ".lot_id=" + consolidatedTable + "." + strLotIdField + " \n"
            + strQueryClauseWafer + "\n"
                                    ")\n";

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}
