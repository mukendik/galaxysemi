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

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gstdl_utils_c.h>

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////

bool GexDbPlugin_Galaxy::AddConsolidatedField_TL(QString & strQuery_Fields, const QString & strTableAlias, const QString & strMetaDataName, const QString & strCastNumber)
{
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping, itStartTField;
    GexDbPlugin_Mapping_Field				clFieldMap, clStartTFieldMap;
    QString									strDbField;

    if (!m_pmapFields_GexToRemote)
        return false;

    itMapping = m_pmapFields_GexToRemote->find(strMetaDataName);
    if(itMapping == m_pmapFields_GexToRemote->end())
        return true;

    clFieldMap = itMapping.value();
    if(!clFieldMap.m_bConsolidated)
        return true;

    // When use Oracle, add dummy aggregate function for group by
    QString									strAggregateFunction;
    QString									strCastAs;
    strAggregateFunction = "";
    strCastAs = "CHAR(255)";

    if(m_pclDatabaseConnector->IsOracleDB())
    {
        strAggregateFunction = " MAX";
        strCastAs = "VARCHAR2(255)";
    }

    // Standard string metadata field
    if(!clFieldMap.m_bFact && !clFieldMap.m_bNumeric && !clFieldMap.m_bTime)
    {
        // gcore-1855 (MySQL only)
        strDbField = strTableAlias + "." + clFieldMap.m_strNormalizedName;
        if(!strQuery_Fields.isEmpty())
            strQuery_Fields += ",\n";
        strQuery_Fields += "case WHEN (count(distinct " + strDbField + ") + (" + strDbField + " is null)";
        strQuery_Fields += ") <=1 THEN case WHEN "+strAggregateFunction+"(" + strDbField;
        strQuery_Fields += ") is null OR "+strAggregateFunction+"(" + strDbField;
        strQuery_Fields += ")='' THEN 'n/a' ELSE "+strAggregateFunction+"(";
        if(clFieldMap.m_bCustom)
            strQuery_Fields += "CAST(" + strDbField + " as "+strCastAs+")";
        else
            strQuery_Fields += strDbField;
        strQuery_Fields += ") END ELSE 'MULTI' END AS " + clFieldMap.m_strNormalizedName;

        return true;
    }

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

    // Time metadata field
    if(clFieldMap.m_bTime)
    {
        itStartTField = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_STARTDATETIME);
        if(itStartTField != m_pmapFields_GexToRemote->end())
        {
            clStartTFieldMap = itStartTField.value();
            strDbField = strTableAlias + "." + clStartTFieldMap.m_strNormalizedName;
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_STARTDATETIME)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += startDateConsolidationRule+"(" + strDbField + ") AS " + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_DAY)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + strDbField + ")",eDate) + " AS " + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_WEEK)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += "cast("+TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + strDbField + ")",eWeek)+ " as " + strCastNumber + ") AS " + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_MONTH)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += "cast("+TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + strDbField + ")",eMonth)+ " as " + strCastNumber + ") AS " + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_QUARTER)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + strDbField + ")",eQuarter) + " AS " + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_YEAR)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += "cast("+TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + strDbField + ")",eYear)+ " as " + strCastNumber + ") AS " + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_YEAR_WEEK)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + strDbField + ")",eWYear,eWeek).replace("||","'concatOp'") + " AS " + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_YEAR_MONTH)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + strDbField + ")",eYear,eMonth).replace("||","'concatOp'") + " AS " + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + strDbField + ")",eYear,eQuarter).replace("||","'concatOp'") + " AS " + clFieldMap.m_strNormalizedName;
                return true;
            }
        }
    }

    // Facts metadata field
    if(clFieldMap.m_bFact)
    {
        strDbField = strTableAlias + "." + clFieldMap.m_strNormalizedName;
        if(	(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_LOT_PARTS) ||
                (clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD) ||
                (clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_WAFER_PARTS) ||
                (clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD) ||
                (clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE) )
        {
            if(!strQuery_Fields.isEmpty())
                strQuery_Fields += ",\n";
            strQuery_Fields += "sum(" + strDbField + ") AS " + clFieldMap.m_strNormalizedName;
            return true;
        }
    }

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidatedSelectQuery_TL(QString & strQuery, const QString & strFromTableName, const QString & strTableAlias, const QString & strFtTrackingLotID, const QString & strCastNumber)
{
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping, itStartTField;
    GexDbPlugin_Mapping_Field				clFieldMap;
    QString									strQuery_Fields;

    strQuery = "";

    if (!m_pmapFields_GexToRemote)
        return false;

    itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_TRACKING_LOT);
    clFieldMap = itMapping.value();

    ////////////////////////////////////////////////////////////////////////////////////
    // FIRST ADD MOST SIGNIFICANT FIELDS
    ////////////////////////////////////////////////////////////////////////////////////
    // PRODUCT_ID
    AddConsolidatedField_TL(strQuery_Fields, strTableAlias, m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strCastNumber);
    // TRACKING_LOT_ID
    AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_TRACKING_LOT, strCastNumber);
    // LOT_ID
    AddConsolidatedField_TL(strQuery_Fields, strTableAlias, m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], strCastNumber);
    // SUBLOT_ID
    AddConsolidatedField_TL(strQuery_Fields, strTableAlias, m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], strCastNumber);
    // WAFER_ID
    AddConsolidatedField_TL(strQuery_Fields, strTableAlias, m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], strCastNumber);

    ////////////////////////////////////////////////////////////////////////////////////
    // NEXT ADD ALL TIME FIELDS
    ////////////////////////////////////////////////////////////////////////////////////
    // Get mapping for start_t
    itStartTField = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_STARTDATETIME);
    if(itStartTField != m_pmapFields_GexToRemote->end())
    {
        // GEXDB_PLUGIN_DBFIELD_STARTDATETIME
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_STARTDATETIME, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_DAY
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_DAY, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_WEEK
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_WEEK, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_MONTH
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_MONTH, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_QUARTER
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_QUARTER, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_YEAR
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_YEAR, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_YEAR_WEEK
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_YEAR_WEEK, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_YEAR_MONTH
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_YEAR_MONTH, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER, strCastNumber);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // NEXT ADD ALL FACT FIELDS
    ////////////////////////////////////////////////////////////////////////////////////
    if(m_eTestingStage == eFinalTest)
    {
        if(!strQuery_Fields.isEmpty())
            strQuery_Fields += ",\n";
        strQuery_Fields += "count(" + strTableAlias + ".lot_id) AS nb_lots";
        // GEXDB_PLUGIN_DBFIELD_LOT_PARTS
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_LOT_PARTS, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD, strCastNumber);
    }
    if(m_eTestingStage == eWaferTest)
    {
        if(!strQuery_Fields.isEmpty())
            strQuery_Fields += ",\n";
        strQuery_Fields += "count(" + strTableAlias + ".wafer_id) AS nb_wafers";
        // GEXDB_PLUGIN_DBFIELD_WAFER_PARTS
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_WAFER_PARTS, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD, strCastNumber);
        // GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE, strCastNumber);
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
        AddConsolidatedField_TL(strQuery_Fields, strTableAlias, clFieldMap.m_strMetaDataName, strCastNumber);
    }

    // Build all query elements
    strQuery = "SELECT\n";
    strQuery += strQuery_Fields + "\n";
    strQuery += "FROM " + strFromTableName + " " + strTableAlias + "\n";
    if(!strFtTrackingLotID.isEmpty())
        strQuery += "WHERE " + strTableAlias + ".tracking_lot_id=" + strFtTrackingLotID + "\n";
    strQuery += "GROUP BY " + strTableAlias + ".tracking_lot_id," + strTableAlias + ".product_name\n";

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidatedTable_TL(const QString & strTableName, const QString & strTableAlias, const QString & strFromTableName, const QString & strCastNumber, const QString & strTableEngine, const QString & /*strIndexSuffix*/)
{
    if (!m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString				strLogMessage, strQuery, strQuery_Select;

    // TD-78

    ////////////////////////////////////////////////////////////////////////////////////
    // 1) Drop table
    ////////////////////////////////////////////////////////////////////////////////////
    clQuery.Execute("DROP TABLE " + strTableName);

    ////////////////////////////////////////////////////////////////////////////////////
    // 2) Create table
    ////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Creating/populating " + strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    // Select query
    CreateConsolidatedSelectQuery_TL(strQuery_Select, strFromTableName, strTableAlias, QString(""), strCastNumber);

    // Create query...
    strQuery =  "CREATE TABLE " + strTableName + strTableEngine + " AS\n";
    strQuery += strQuery_Select;

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    ////////////////////////////////////////////////////////////////////////////////////
    // 3) Create PK index in table
    ////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Adding index to " + strTableName+"(tracking_lot_id)...";
    InsertIntoUpdateLog(strLogMessage);
    //strQuery = "CREATE INDEX " + strTableName + strIndexSuffix + " ON " + strTableName + " (tracking_lot_id)";
    strQuery = "ALTER TABLE " + strTableName + " ADD PRIMARY KEY (tracking_lot_id,product_name)";
    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidatedTables_TL(enum TestingStage eTestingStage/*=eUnknownStage*/)
{
    QString				strTableName, strFromTableName, strTableAlias;
    QString				strQuery;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    QString strTableSpace, strTableEngine, strIndexSuffix, strCastNumber;
    if(m_pclDatabaseConnector->IsMySqlDB())
    {
        strIndexSuffix = "";
        strCastNumber = "unsigned ";
        // Get storage engine
        QString	strEngine,strFormat;
        GetStorageEngineName(strEngine,strFormat);
        strTableEngine = " ENGINE=" + strEngine + " " + strFormat + " ";
    }
    else if(m_pclDatabaseConnector->IsSQLiteDB())
    {
        strTableEngine = "";
        strIndexSuffix = "_index";
        strCastNumber = "integer";
    }
    else if(m_pclDatabaseConnector->IsOracleDB())
    {
        // Oracle
        // Found some specification
        // Get some information like for the PRODUCT table
        strQuery = "SELECT TABLESPACE_NAME,LOGGING FROM USER_TABLES WHERE TABLE_NAME='FT_LOT'";
        if(!clQuery.exec(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
            return false;
        }

        clQuery.first();

        strTableSpace = clQuery.value(0).toString();
        if(clQuery.value(1).toString().compare("NO"))
            strTableEngine = " TABLESPACE "+strTableSpace+" PCTFREE 5 PCTUSED 80 NOLOGGING COMPRESS ";
        else
            strTableEngine = " TABLESPACE "+strTableSpace+" PCTFREE 5 PCTUSED 80 LOGGING COMPRESS ";

        strIndexSuffix = "";
        strCastNumber = "number";
    }

    if((eTestingStage == eUnknownStage) || (eTestingStage == eFinalTest))
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // 1.0) Create ft_consolidated_tl table
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eFinalTest);

        // Drop old name
        clQuery.Execute("DROP TABLE " + NormalizeTableName("_consolidated_tracking_lot"));
        strTableName = NormalizeTableName("_consolidated_tl");
        strFromTableName = NormalizeTableName("_consolidated_sublot");
        strTableAlias="CL";

        if(!CreateConsolidatedTable_TL(strTableName, strTableAlias, strFromTableName, strCastNumber, strTableEngine, strIndexSuffix))
            return false;
    }
    IncrementProgress();

    if((eTestingStage == eUnknownStage) || (eTestingStage == eWaferTest))
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // 2.0) Create wt_consolidated_tl table
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eWaferTest);

        // Drop old name
        clQuery.Execute("DROP TABLE " + NormalizeTableName("_consolidated_tracking_lot"));
        strTableName = NormalizeTableName("_consolidated_tl");
        strFromTableName = NormalizeTableName("_consolidated_wafer");
        strTableAlias="CW";

        if(!CreateConsolidatedTable_TL(strTableName, strTableAlias, strFromTableName, strCastNumber, strTableEngine, strIndexSuffix))
            return false;
    }
    IncrementProgress();

    return true;
}

bool
GexDbPlugin_Galaxy::
CreateConsolidatedUpdateProcedure_TL(const QString& strTableName,
                                     const QString& strTableAlias,
                                     const QString& strFromTableName,
                                     const QString& strCastNumber,
                                     const QString& /*strTableEngine*/,
                                     const QString& /*strIndexSuffix*/)
{
    if (!m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString				strLogMessage, strQuery, strSubQuery;
    QString				strProcedureName = strTableName + "_update";

    // TD-78

    ////////////////////////////////////////////////////////////////////////////////////
    // 1) Drop procedure
    ////////////////////////////////////////////////////////////////////////////////////
    clQuery.Execute("DROP PROCEDURE " + strProcedureName);

    ////////////////////////////////////////////////////////////////////////////////////
    // 2) Create procedure
    ////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Creating stored procedure " + strProcedureName+"...";
    InsertIntoUpdateLog(strLogMessage);

    if(m_eTestingStage == eFinalTest) // Final
    {
        // Select sub-query
        CreateConsolidatedSelectQuery_TL(strSubQuery, strFromTableName, strTableAlias, QString("TrackingLotID"), strCastNumber);

        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery =  "CREATE PROCEDURE " + strProcedureName + "(\n";
            strQuery += "	 IN_LotID	IN	varchar2)\n";
            strQuery += "AS\n";
            strQuery += "Message			varchar2(1024);\n";
            strQuery += "LogMessage		varchar2(1024);\n";
            strQuery += "NbParts			varchar2(20);\n";
            strQuery += "TrackingLotID	varchar2(255);\n";
            strQuery += "BEGIN\n";
            strQuery += "\n";
            strQuery += "-- Get TrackingLotID\n";
            strQuery += "SELECT DISTINCT tracking_lot_id INTO TrackingLotID FROM ft_lot WHERE lot_id=IN_LotID ;\n";
            strQuery += "\n";
            strQuery += "-- Log message\n";
            strQuery += "select 'Update " + m_strTablePrefix + "consolidated_tl: TrackingLotID=' into LogMessage from dual;\n";
            strQuery += "select concat(LogMessage, TrackingLotID) into LogMessage from dual;\n";
            strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(sysdate,'" + strProcedureName + "',LogMessage);\n";
            strQuery += "\n";
            strQuery += "-- Remove old row\n";
            strQuery += "DELETE FROM " + strTableName + " where tracking_lot_id=TrackingLotID;\n";
            strQuery += "\n";
            strQuery += "-- Add new row\n";
            strQuery += "INSERT INTO " + strTableName + "\n";
            strQuery += strSubQuery;
            strQuery += ";\n";
            strQuery += "END;\n";
        }
        else
        {
            strQuery =  "CREATE PROCEDURE " + strProcedureName + "(\n";
            strQuery += "	 IN	IN_LotID	varchar(255))\n";
            strQuery += "BEGIN\n";
            strQuery += "DECLARE Message			varchar(1024);\n";
            strQuery += "DECLARE LogMessage		varchar(1024);\n";
            strQuery += "DECLARE NbParts			varchar(20);\n";
            strQuery += "DECLARE TrackingLotID	varchar(255);\n";
            strQuery += "\n";
            strQuery += "-- Get TrackingLotID\n";
            strQuery += "SELECT DISTINCT tracking_lot_id FROM ft_lot WHERE lot_id=IN_LotID INTO TrackingLotID;\n";
            strQuery += "\n";
            strQuery += "-- Log message\n";
            strQuery += "select 'Update " + m_strTablePrefix + "consolidated_tl: TrackingLotID=' into LogMessage from dual;\n";
            strQuery += "select concat(LogMessage, TrackingLotID) into LogMessage from dual;\n";
            strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(NOW(),'" + strProcedureName + "',LogMessage);\n";
            strQuery += "\n";
            strQuery += "-- Remove old row\n";
            strQuery += "DELETE FROM " + strTableName + " where tracking_lot_id=TrackingLotID;\n";
            strQuery += "\n";
            strQuery += "-- Add new row\n";
            strQuery += "INSERT INTO " + strTableName + "\n";
            strQuery += strSubQuery;
            strQuery += ";\n";
            strQuery += "\n";
            strQuery += "call az_consolidated_tl_update(TrackingLotID);\n";
            strQuery += "\n";
            strQuery += "END\n";
        }
    }
    else // Wafer
    {
        // Select sub-query
        CreateConsolidatedSelectQuery_TL(strSubQuery, strFromTableName, strTableAlias, QString("TrackingLotID"), strCastNumber);

        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strQuery =  "CREATE PROCEDURE " + strProcedureName + "(\n";
            strQuery += "	 IN_LotID	IN	varchar2)\n";
            strQuery += "AS\n";
            strQuery += "Message			varchar2(1024);\n";
            strQuery += "LogMessage		varchar2(1024);\n";
            strQuery += "NbParts			varchar2(20);\n";
            strQuery += "TrackingLotID	varchar2(255);\n";
            strQuery += "BEGIN\n";
            strQuery += "\n";
            strQuery += "-- Get TrackingLotID\n";
            strQuery += "SELECT DISTINCT tracking_lot_id INTO TrackingLotID FROM wt_lot WHERE lot_id=IN_LotID ;\n";
            strQuery += "\n";
            strQuery += "-- Log message\n";
            strQuery += "select 'Update " + m_strTablePrefix + "consolidated_tl: TrackingLotID=' into LogMessage from dual;\n";
            strQuery += "select concat(LogMessage, TrackingLotID) into LogMessage from dual;\n";
            strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(sysdate,'" + strProcedureName + "',LogMessage);\n";
            strQuery += "\n";
            strQuery += "-- Remove old row\n";
            strQuery += "DELETE FROM " + strTableName + " where tracking_lot_id=TrackingLotID;\n";
            strQuery += "\n";
            strQuery += "-- Add new row\n";
            strQuery += "INSERT INTO " + strTableName + "\n";
            strQuery += strSubQuery;
            strQuery += ";\n";
            strQuery += "END;\n";
        }
        else
        {
            strQuery =  "CREATE PROCEDURE " + strProcedureName + "(\n";
            strQuery += "	 IN	IN_LotID	varchar(255))\n";
            strQuery += "BEGIN\n";
            strQuery += "DECLARE Message			varchar(1024);\n";
            strQuery += "DECLARE LogMessage		varchar(1024);\n";
            strQuery += "DECLARE NbParts			varchar(20);\n";
            strQuery += "DECLARE TrackingLotID	varchar(255);\n";
            strQuery += "\n";
            strQuery += "DECLARE last_row_fetched	int;\n";
            strQuery += "DECLARE cFtTrackingLots CURSOR FOR\n";
            strQuery += "	select distinct ft_tracking_lot_id from ft_die_tracking where wt_tracking_lot_id=TrackingLotID;\n";
            strQuery += "DECLARE CONTINUE HANDLER FOR NOT FOUND SET last_row_fetched=1;\n";
            strQuery += "\n";
            strQuery += "-- Get TrackingLotID\n";
            strQuery += "select DISTINCT tracking_lot_id from wt_lot where lot_id=IN_LotID into TrackingLotID;\n";
            strQuery += "\n";
            strQuery += "-- Log message\n";
            strQuery += "select 'Update " + m_strTablePrefix + "consolidated_tl: TrackingLotID=' into LogMessage from dual;\n";
            strQuery += "select concat(LogMessage, TrackingLotID) into LogMessage from dual;\n";
            strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(NOW(),'" + strProcedureName + "',LogMessage);\n";
            strQuery += "\n";
            strQuery += "-- Remove old row\n";
            strQuery += "DELETE FROM " + strTableName + " where tracking_lot_id=TrackingLotID;\n";
            strQuery += "\n";
            strQuery += "-- Add new row\n";
            strQuery += "INSERT INTO " + strTableName + "\n";
            strQuery += strSubQuery;
            strQuery += ";\n";
            strQuery += "SET last_row_fetched=0;\n";
            strQuery += "OPEN cFtTrackingLots;\n";
            strQuery += "cFtTrackingLots_loop:LOOP\n";
            strQuery += "  FETCH cFtTrackingLots INTO TrackingLotID;\n";
            strQuery += "  IF last_row_fetched=1 THEN\n";
            strQuery += "    LEAVE cFtTrackingLots_loop;\n";
            strQuery += "  END IF;\n";
            strQuery += "  call az_consolidated_tl_update(TrackingLotID);\n";
            strQuery += "END LOOP cFtTrackingLots_loop;\n";
            strQuery += "CLOSE cFtTrackingLots;\n";
            strQuery += "\n";
            strQuery += "END\n";
        }
    }

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidatedUpdateProcedures_TL(
        enum TestingStage eTestingStage/*=eUnknownStage*/)
{
    QString	strTableName, strFromTableName, strTableAlias;
    QString strTableEngine, strIndexSuffix, strCastNumber;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    if(m_pclDatabaseConnector->IsMySqlDB())
        strCastNumber = "unsigned ";
    else
        if(m_pclDatabaseConnector->IsSQLiteDB())
            strCastNumber = "integer";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                strCastNumber = "number";

    if((eTestingStage == eUnknownStage) || (eTestingStage == eFinalTest))
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // 1.0) Create update procedure for ft_consolidated_tl table
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eFinalTest);

        // Drop old name
        clQuery.Execute("DROP TABLE " + NormalizeTableName("_consolidated_tracking_lot"));
        strTableName = NormalizeTableName("_consolidated_tl");
        strFromTableName = NormalizeTableName("_consolidated_sublot");
        strTableAlias="CSL";

        if(!CreateConsolidatedUpdateProcedure_TL(strTableName, strTableAlias, strFromTableName, strCastNumber, strTableEngine, strIndexSuffix))
            return false;
    }
    IncrementProgress();

    if((eTestingStage == eUnknownStage) || (eTestingStage == eWaferTest))
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // 2.0) Create update procedutre for wt_consolidated_tl table
        ////////////////////////////////////////////////////////////////////////////////////
        SetTestingStage(eWaferTest);

        // Drop old name
        clQuery.Execute("DROP TABLE " + NormalizeTableName("_consolidated_tracking_lot"));
        strTableName = NormalizeTableName("_consolidated_tl");
        strFromTableName = NormalizeTableName("_consolidated_wafer");
        strTableAlias="CW";

        if(!CreateConsolidatedUpdateProcedure_TL(strTableName, strTableAlias, strFromTableName, strCastNumber, strTableEngine, strIndexSuffix))
            return false;
    }
    IncrementProgress();

    return true;
}

bool GexDbPlugin_Galaxy::AddConsolidatedField_AZ(QString& strQuery_Fields,
                                                 const QString& strTableAlias,
                                                 const QString& strMetaDataName,
                                                 const QString& strCastNumber,
                                                 bool  bAddPrefix)
{
    GexDbPlugin_Mapping_FieldMap::Iterator  itMapping, itStartTField;
    GexDbPlugin_Mapping_Field               clFieldMap, clStartTFieldMap;
    QString                                 lDbField;
    QString                                 lAsPrefix;

    // Add a prefix to the column name
    // ie : column = AS nb_parts
    // if true       AS ft_nb_parts
    if(bAddPrefix)
        lAsPrefix = m_strTablePrefix;

    if (!m_pmapFields_GexToRemote)
        return false;

    itMapping = m_pmapFields_GexToRemote->find(strMetaDataName);
    if(itMapping == m_pmapFields_GexToRemote->end())
        return true;

    clFieldMap = itMapping.value();
    if(!clFieldMap.m_bAZ || !clFieldMap.m_bConsolidated)
        return true;

    // case 6950 - MULTI for multi data
    if(!clFieldMap.m_bFact && !clFieldMap.m_bNumeric && !clFieldMap.m_bTime)
    {
        // When use Oracle, add dummy aggregate function for group by
        QString									strAggregateFunction;
        QString									strCastAs;
        strAggregateFunction = "";
        strCastAs = "CHAR(255)";

        if(m_pclDatabaseConnector->IsOracleDB())
        {
            strAggregateFunction = " MAX";
            strCastAs = "VARCHAR2(255)";
        }
        // gcore-1855 (MySQL only)
        lDbField = strTableAlias + "." + clFieldMap.m_strNormalizedName;
        if(!strQuery_Fields.isEmpty())
            strQuery_Fields += ",\n";
        strQuery_Fields += "case WHEN (count(distinct " + lDbField + ") + (" + lDbField + " is null)";
        strQuery_Fields += ") <=1 THEN case WHEN "+strAggregateFunction+"(" + lDbField;
        strQuery_Fields += ") is null OR "+strAggregateFunction+"(" + lDbField;
        strQuery_Fields += ")='' THEN 'n/a' ELSE "+strAggregateFunction+"(";
        if(clFieldMap.m_bCustom)
            strQuery_Fields += "CAST(" + lDbField + " as "+strCastAs+")";
        else
            strQuery_Fields += lDbField;
        strQuery_Fields += ") END ELSE 'MULTI' END AS " + m_strTablePrefix + clFieldMap.m_strNormalizedName;

        return true;
    }

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

    // Time metadata field
    if(clFieldMap.m_bTime && (m_eTestingStage == eFinalTest))
    {
        itStartTField = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_STARTDATETIME);
        if(itStartTField != m_pmapFields_GexToRemote->end())
        {
            clStartTFieldMap = itStartTField.value();
            lDbField = strTableAlias + "." + clStartTFieldMap.m_strNormalizedName;
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_STARTDATETIME)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += startDateConsolidationRule+"(" + lDbField + ") AS " + lAsPrefix + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_DAY)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + lDbField + ")",eDate) + " AS " + lAsPrefix + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_WEEK)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += "cast("+TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + lDbField + ")",eWeek)+ " as " + strCastNumber + ") AS " + lAsPrefix + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_MONTH)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += "cast("+TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + lDbField + ")",eMonth)+ " as " + strCastNumber + ") AS " + lAsPrefix + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_QUARTER)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + lDbField + ")",eQuarter) + " AS " + lAsPrefix + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_YEAR)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += "cast("+TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + lDbField + ")",eYear)+ " as " + strCastNumber + ") AS " + lAsPrefix + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_YEAR_WEEK)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + lDbField + ")",eWYear,eWeek).replace("||","'concatOp'") + " AS " + lAsPrefix + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_YEAR_MONTH)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + lDbField + ")",eYear,eMonth).replace("||","'concatOp'") + " AS " + lAsPrefix + clFieldMap.m_strNormalizedName;
                return true;
            }
            if(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER)
            {
                if(!strQuery_Fields.isEmpty())
                    strQuery_Fields += ",\n";
                strQuery_Fields += TranslateUnixTimeStampToSqlDateTime(startDateConsolidationRule+"(" + lDbField + ")",eYear,eQuarter).replace("||","'concatOp'") + " AS " + lAsPrefix + clFieldMap.m_strNormalizedName;
                return true;
            }
        }
    }

    // Facts metadata field
    if(clFieldMap.m_bFact)
    {
        lDbField = strTableAlias + "." + clFieldMap.m_strNormalizedName;
        if(	(clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_LOT_PARTS) ||
                (clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD) ||
                (clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_WAFER_PARTS) ||
                (clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD) ||
                (clFieldMap.m_strMetaDataName == GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE) )
        {
            if(!strQuery_Fields.isEmpty())
                strQuery_Fields += ",\n";
            strQuery_Fields += "sum(" + lDbField + ") AS " + lAsPrefix + clFieldMap.m_strNormalizedName;
            return true;
        }
    }

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidatedSelectQuery_AZ_Data(
        QString & strQuery,
        QString & strTableAlias_FT,
        QString & strTableAlias_WT,
        const QString & strFtTrackingLotID,
        const QString & strCastNumber)
{
    GexDbPlugin_Mapping_FieldMap::Iterator	itStartTField, itMapping;
    GexDbPlugin_Mapping_Field				clFieldMap;
    QString									strFromTableName_FT;
    QString									strFromTableName_WT;
    QString									strQuery_Fields;

    // Add consolidated fields from FT
    SetTestingStage(eFinalTest);
    strFromTableName_FT = NormalizeTableName("_consolidated_tl");
    strTableAlias_FT="FTL";

    ////////////////////////////////////////////////////////////////////////////////////
    // FIRST ADD Main Tracking LotID (FT) and MOST SIGNIFICANT FT fields
    ////////////////////////////////////////////////////////////////////////////////////
    // TRACKING_LOT_ID
    AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, GEXDB_PLUGIN_DBFIELD_TRACKING_LOT, strCastNumber);
    // PRODUCT_ID
    AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strCastNumber);
    // LOT_ID
    AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], strCastNumber);
    // SUBLOT_ID
    AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], strCastNumber);
    // WAFER_ID
    AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], strCastNumber);

    ////////////////////////////////////////////////////////////////////////////////////
    // NEXT ADD ALL TIME FIELDS (FT)
    ////////////////////////////////////////////////////////////////////////////////////
    // Get mapping for start_t

    if (!m_pmapFields_GexToRemote)
        return false;

    itStartTField = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_DBFIELD_STARTDATETIME);
    if(itStartTField != m_pmapFields_GexToRemote->end())
    {
        // GEXDB_PLUGIN_DBFIELD_STARTDATETIME
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, GEXDB_PLUGIN_DBFIELD_STARTDATETIME, strCastNumber, false);
        // GEXDB_PLUGIN_DBFIELD_DAY
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, GEXDB_PLUGIN_DBFIELD_DAY, strCastNumber, false);
        // GEXDB_PLUGIN_DBFIELD_WEEK
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, GEXDB_PLUGIN_DBFIELD_WEEK, strCastNumber, false);
        // GEXDB_PLUGIN_DBFIELD_MONTH
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, GEXDB_PLUGIN_DBFIELD_MONTH, strCastNumber, false);
        // GEXDB_PLUGIN_DBFIELD_QUARTER
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, GEXDB_PLUGIN_DBFIELD_QUARTER, strCastNumber, false);
        // GEXDB_PLUGIN_DBFIELD_YEAR
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, GEXDB_PLUGIN_DBFIELD_YEAR, strCastNumber, false);
        // GEXDB_PLUGIN_DBFIELD_YEAR_WEEK
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, GEXDB_PLUGIN_DBFIELD_YEAR_WEEK, strCastNumber, false);
        // GEXDB_PLUGIN_DBFIELD_YEAR_MONTH
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, GEXDB_PLUGIN_DBFIELD_YEAR_MONTH, strCastNumber, false);
        // GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER, strCastNumber, false);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // NEXT ADD ALL other FT FIELDS
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
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_FT, clFieldMap.m_strMetaDataName, strCastNumber);
    }

    // Add consolidated fields from WT
    SetTestingStage(eWaferTest);
    strFromTableName_WT = NormalizeTableName("_consolidated_tl");
    strTableAlias_WT="WTL";

    // TRACKING_LOT_ID
    AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_WT, GEXDB_PLUGIN_DBFIELD_TRACKING_LOT, strCastNumber);
    // PRODUCT_ID
    AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_WT, m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strCastNumber);
    // LOT_ID
    AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_WT, m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], strCastNumber);
    // SUBLOT_ID
    AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_WT, m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], strCastNumber);
    // WAFER_ID
    AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_WT, m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], strCastNumber);

    ////////////////////////////////////////////////////////////////////////////////////
    // NEXT ADD ALL other WT FIELDS
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
        AddConsolidatedField_AZ(strQuery_Fields, strTableAlias_WT, clFieldMap.m_strMetaDataName, strCastNumber);
    }

    // Build all query elements
    strQuery = "SELECT\n" + strQuery_Fields + "\n";
    strQuery += "FROM " + strFromTableName_FT + " " + strTableAlias_FT + "\n";
    strQuery += "LEFT OUTER JOIN ft_die_tracking FDT\n";
    strQuery += "on FDT.ft_tracking_lot_id=" + strTableAlias_FT + ".tracking_lot_id\n";
    strQuery += "LEFT OUTER JOIN " + strFromTableName_WT + " " + strTableAlias_WT + "\n";
    strQuery += "on " + strTableAlias_WT + ".tracking_lot_id=" + "FDT.wt_tracking_lot_id\n";
    if(!strFtTrackingLotID.isEmpty())
    {
        strQuery += "WHERE " + strTableAlias_FT + ".tracking_lot_id=" + strFtTrackingLotID + "\n";
        strQuery += "AND " + strTableAlias_FT + ".nb_parts > 0\n";
    }
    else
        strQuery += "WHERE " + strTableAlias_FT + ".nb_parts > 0\n";
    strQuery += "GROUP BY " + strTableAlias_FT + ".tracking_lot_id, " + strTableAlias_FT + ".product_name\n";

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidatedTable_AZ_Data(const QString & strCastNumber, const QString & strTableEngine, const QString & /*strIndexSuffix*/)
{
    if (!m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString				strLogMessage, strQuery, strTableName, strTableAlias_FT, strTableAlias_WT;
    QString				strQuery_Select;

    // Drop old name
    clQuery.Execute("DROP TABLE az_consolidated_tracking_lot_data");
    strTableName = "az_consolidated_tl_data";

    // TD-78

    ////////////////////////////////////////////////////////////////////////////////////
    // 1) Drop table
    ////////////////////////////////////////////////////////////////////////////////////
    clQuery.Execute("DROP TABLE " + strTableName);

    ////////////////////////////////////////////////////////////////////////////////////
    // 2) Create table
    ////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Creating/populating " + strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    CreateConsolidatedSelectQuery_AZ_Data(strQuery_Select, strTableAlias_FT, strTableAlias_WT, QString(""), strCastNumber);

    // Create query...
    strQuery =  "CREATE TABLE " + strTableName + strTableEngine + " AS\n";
    strQuery += strQuery_Select;

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    ////////////////////////////////////////////////////////////////////////////////////
    // 3) Create PK index in table
    ////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Adding index to " + strTableName + "(" + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "tracking_lot_id)...";
    InsertIntoUpdateLog(strLogMessage);
    //strQuery = "CREATE INDEX " + strTableName + strIndexSuffix + " ON " + strTableName + " (" + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "tracking_lot_id)";
    strQuery = "ALTER TABLE " + strTableName + " ADD PRIMARY KEY (" + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "tracking_lot_id," + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "product_name)";
    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    return true;
}

//////////////////////////////////////////////////////////////////////////////
////// \brief GexDbPlugin_Galaxy::CreateConsolidatedSelectQuery_AZ_Facts
/// Call this method to create the new consolidated table (no filster on the ft_tracking_lot_id)
/// or Call this method to generat the query to populate the exisiting consolidated table (filter on ft_tracking_lot_id)
////// \param strQuery
////// \param strFtTrackingLotID
////// \param strCastNumber
////// \return
bool GexDbPlugin_Galaxy::CreateConsolidatedSelectQuery_AZ_Facts(QString & strQuery, const QString & strFtTrackingLotID, const QString & strCastNumber)
{
    QString lFromTableName_FT, lFromTableName_WT;
    QString lTableAlias_FT, lTableAlias_WT, lTableAlias_AZ;
    QString lSelectQuery_FT, lSelectQuery_WT, lSelectQuery_AZ;

    // When use Oracle, add dummy aggregate function for group by
    QString     lAggregateFunctionMax;
    QString     lAggregateFunctionSum;
    QString     lCastAs;
    lAggregateFunctionMax = " MAX(%1)";
    lAggregateFunctionSum = " SUM(%1)";
    lCastAs = "SIGNED";

    if (!m_pclDatabaseConnector)
        return false;

    // FT query
    SetTestingStage(eFinalTest);
    lFromTableName_FT = NormalizeTableName("_consolidated_tl");
    lTableAlias_FT="FTL";

    SetTestingStage(eWaferTest);
    lFromTableName_WT = NormalizeTableName("_consolidated_tl");
    lTableAlias_WT="WTL";

    // Aggregate data at Wafer Sort
    lSelectQuery_WT = "SELECT \n";
    lSelectQuery_WT+= lTableAlias_FT + ".tracking_lot_id\n,";
    lSelectQuery_WT+= lTableAlias_FT + ".product_name,";
    lSelectQuery_WT+= lTableAlias_FT + ".nb_parts,";
    lSelectQuery_WT+= lTableAlias_FT + ".nb_parts_good";
    // AddConsolidatedField_AZ add the comma before
    AddConsolidatedField_AZ(lSelectQuery_WT, lTableAlias_WT, GEXDB_PLUGIN_DBFIELD_TRACKING_LOT, strCastNumber);
    AddConsolidatedField_AZ(lSelectQuery_WT, lTableAlias_WT, m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strCastNumber);
    // GEXDB_PLUGIN_DBFIELD_WAFER_PARTS
    AddConsolidatedField_AZ(lSelectQuery_WT, lTableAlias_WT, GEXDB_PLUGIN_DBFIELD_WAFER_PARTS, strCastNumber,true);
    // GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE
    AddConsolidatedField_AZ(lSelectQuery_WT, lTableAlias_WT, GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE, strCastNumber,true);
    // GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD
    AddConsolidatedField_AZ(lSelectQuery_WT, lTableAlias_WT, GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD, strCastNumber,true);
    // FROM
    lSelectQuery_WT += "\n";
    lSelectQuery_WT += "FROM " + lFromTableName_FT + " " + lTableAlias_FT + "\n";
    lSelectQuery_WT += "LEFT OUTER JOIN\n";
    lSelectQuery_WT += "(";
    // Add a WHERE clause to reduce the execution time
    lSelectQuery_WT += "select distinct ft_tracking_lot_id, wt_tracking_lot_id from ft_die_tracking\n";
    if(!strFtTrackingLotID.isEmpty())
    {
        lSelectQuery_WT += "where ft_tracking_lot_id=" + strFtTrackingLotID + "\n";
    }
    lSelectQuery_WT += ") DT\n";
    lSelectQuery_WT += "on DT.ft_tracking_lot_id=" + lTableAlias_FT + ".tracking_lot_id\n";
    lSelectQuery_WT += "LEFT OUTER JOIN " + lFromTableName_WT + " " + lTableAlias_WT +"\n";
    lSelectQuery_WT += "on WTL.tracking_lot_id=DT.wt_tracking_lot_id\n";
    if(!strFtTrackingLotID.isEmpty())
    {
        lSelectQuery_WT += "WHERE " + lTableAlias_FT + ".tracking_lot_id=" + strFtTrackingLotID + "\n";
        lSelectQuery_WT += "AND " + lTableAlias_FT + ".nb_parts > 0\n";
    }
    else
        lSelectQuery_WT += "WHERE " + lTableAlias_FT + ".nb_parts > 0\n";
    // GROUP BY tracking_lot_id
    lSelectQuery_WT += "GROUP BY " + lTableAlias_FT + ".tracking_lot_id," + lTableAlias_FT + ".product_name\n";

    // Aggregate data at Final Test
    SetTestingStage(eFinalTest);
    lTableAlias_FT = "T1";
    lSelectQuery_FT = "";
    // TRACKING_LOT_ID from FINAL TEST
    // PRODUCT_NAME from FINAL TEST
    AddConsolidatedField_AZ(lSelectQuery_FT, lTableAlias_FT, GEXDB_PLUGIN_DBFIELD_TRACKING_LOT, strCastNumber);
    AddConsolidatedField_AZ(lSelectQuery_FT, lTableAlias_FT, m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strCastNumber);
    // AddConsolidatedField_AZ add the comma before
    lSelectQuery_FT = "SELECT\n"+lSelectQuery_FT;
    // GEXDB_PLUGIN_DBFIELD_LOT_PARTS
    AddConsolidatedField_AZ(lSelectQuery_FT, lTableAlias_FT, GEXDB_PLUGIN_DBFIELD_LOT_PARTS, strCastNumber,true);
    // GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD
    AddConsolidatedField_AZ(lSelectQuery_FT, lTableAlias_FT, GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD, strCastNumber,true);
    // WT data
    lSelectQuery_FT+= ",\n";
    lSelectQuery_FT+= "wt_tracking_lot_id,\n";
    lSelectQuery_FT+= "wt_product_name,\n";
    lSelectQuery_FT+= "wt_nb_parts,\n";
    lSelectQuery_FT+= "wt_gross_die,\n";
    lSelectQuery_FT+= "wt_nb_parts_good\n";
    // FROM
    lSelectQuery_FT += "\n";
    lSelectQuery_FT += "FROM (" + lSelectQuery_WT + ") " + lTableAlias_FT + "\n";
    // GROUP BY tracking_lot_id
    lSelectQuery_FT += "GROUP BY " + lTableAlias_FT + ".tracking_lot_id,  " + lTableAlias_FT + ".product_name\n";

    // Aggretate data per Production Stage
    // Facts
    lTableAlias_AZ = "AZF";
    lSelectQuery_AZ = "SELECT\n";
    // FT data
    lSelectQuery_AZ+= "ft_tracking_lot_id,\n";
    lSelectQuery_AZ+= "ft_product_name,\n";
    lSelectQuery_AZ+= "production_stage,\n";
    // AZ facts
    lSelectQuery_AZ+= "case WHEN production_stage = 'FT' THEN ft_nb_parts\n";
    lSelectQuery_AZ+= "    ELSE case WHEN production_stage = 'WT' THEN wt_nb_parts\n";
    lSelectQuery_AZ+= "    ELSE case WHEN ((wt_nb_parts IS NULL) OR (wt_nb_parts = 0))  THEN max(ft_nb_parts)\n";
    lSelectQuery_AZ+= "    ELSE CAST(max(ft_nb_parts)/(sum(wt_nb_parts_good)/sum(wt_nb_parts)) AS SIGNED) END END END AS nb_parts,\n";
    lSelectQuery_AZ+= "case WHEN production_stage = 'FT' THEN ft_nb_parts\n";
    lSelectQuery_AZ+= "    ELSE case WHEN production_stage = 'WT' THEN wt_gross_die\n";
    lSelectQuery_AZ+= "    ELSE case WHEN ((wt_nb_parts IS NULL) OR (wt_nb_parts = 0)) THEN max(ft_nb_parts)\n";
    lSelectQuery_AZ+= "    ELSE CAST(max(ft_nb_parts)/(sum(wt_nb_parts_good)/sum(wt_gross_die)) AS SIGNED) END END END AS gross_die,\n";
    lSelectQuery_AZ+= "case WHEN production_stage = 'FT' THEN ft_nb_parts_good\n";
    lSelectQuery_AZ+= "    ELSE case WHEN production_stage = 'WT' THEN wt_nb_parts_good\n";
    lSelectQuery_AZ+= "    ELSE max(ft_nb_parts_good) END END as nb_parts_good\n";
    // FROM
    lSelectQuery_AZ += "\n";
    lSelectQuery_AZ += "FROM (" + lSelectQuery_FT + ") " + lTableAlias_AZ + "\n";
    lSelectQuery_AZ += "INNER JOIN\n";
    lSelectQuery_AZ += "(SELECT 'FT' AS production_stage \n";
    lSelectQuery_AZ += "    UNION select 'WT' AS production_stage \n";
    lSelectQuery_AZ += "    UNION select 'AZ' AS production_stage ) PS\n";
    lSelectQuery_AZ += "GROUP BY ft_tracking_lot_id, ft_product_name, production_stage\n";

    strQuery = lSelectQuery_AZ;

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidatedTable_AZ_Facts(const QString & strCastNumber, const QString & strTableEngine, const QString & /*strIndexSuffix*/)
{
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString				strLogMessage, strQuery, strTableName, strQuery_Select;

    // TD-78

    // Drop old table
    clQuery.Execute("DROP TABLE az_consolidated_tracking_lot_facts");
    strTableName = "az_consolidated_tl_facts";

    ////////////////////////////////////////////////////////////////////////////////////
    // 1) Drop table
    ////////////////////////////////////////////////////////////////////////////////////
    clQuery.Execute("DROP TABLE " + strTableName);

    ////////////////////////////////////////////////////////////////////////////////////
    // 2) Create table
    ////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Creating/populating " + strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    CreateConsolidatedSelectQuery_AZ_Facts(strQuery_Select, QString(""), strCastNumber);
    strQuery =  "CREATE TABLE " + strTableName + strTableEngine + " AS\n";
    strQuery += strQuery_Select;

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    ////////////////////////////////////////////////////////////////////////////////////
    // 3) Create index in table
    ////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Adding index to " + strTableName+"(" + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "tracking_lot_id)...";
    InsertIntoUpdateLog(strLogMessage);
    //strQuery = "CREATE INDEX " + strTableName + strIndexSuffix + " ON " + strTableName + " (" + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "tracking_lot_id)";
    strQuery = "ALTER TABLE " + strTableName + " ADD PRIMARY KEY (" + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "tracking_lot_id, " + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "product_name,production_stage)";
    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidatedSelectQuery_AZ(QString & strQuery, const QString & strFtTrackingLotID)
{
    strQuery += "SELECT\n";
    strQuery += "AZD.*,\n";
    strQuery += "AZF.production_stage,\n";
    strQuery += "AZF.nb_parts,\n";
    strQuery += "AZF.gross_die,\n";
    strQuery += "AZF.nb_parts_good\n";
    strQuery += "FROM\n";
    strQuery += "az_consolidated_tl_data AZD\n";
    strQuery += "left outer join az_consolidated_tl_facts AZF\n";
    strQuery += "on AZF.ft_tracking_lot_id=AZD.ft_tracking_lot_id\n";
    strQuery += "and AZF.ft_product_name=AZD.ft_product_name\n";
    if(!strFtTrackingLotID.isEmpty())
        strQuery += "WHERE AZD.ft_tracking_lot_id="+strFtTrackingLotID;

    return true;
}

bool
GexDbPlugin_Galaxy::CreateConsolidatedTable_AZ(const QString& /*strCastNumber*/,
                                               const QString& strTableEngine,
                                               const QString& /*strIndexSuffix*/)
{
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString				strLogMessage, strQuery, strQuery_Select, strTableName;

    // TD-78

    // Drop old table
    clQuery.Execute("DROP TABLE az_consolidated_tracking_lot");
    strTableName = "az_consolidated_tl";

    ////////////////////////////////////////////////////////////////////////////////////
    // 1) Drop table
    ////////////////////////////////////////////////////////////////////////////////////
    clQuery.Execute("DROP TABLE " + strTableName);

    ////////////////////////////////////////////////////////////////////////////////////
    // 2) Create table
    ////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Creating/populating " + strTableName+"...";
    InsertIntoUpdateLog(strLogMessage);

    SetTestingStage(eFinalTest);

    // Create query...
    CreateConsolidatedSelectQuery_AZ(strQuery_Select, QString(""));
    strQuery =  "CREATE TABLE " + strTableName + strTableEngine + " AS\n";
    strQuery += strQuery_Select;

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    ////////////////////////////////////////////////////////////////////////////////////
    // 3) Create index in table
    ////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Adding index to " + strTableName+"(" + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "tracking_lot_id)...";
    InsertIntoUpdateLog(strLogMessage);
    //strQuery = "CREATE INDEX " + strTableName + strIndexSuffix + " ON " + strTableName + " (" + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "tracking_lot_id)";
    strQuery = "ALTER TABLE " + strTableName + " ADD PRIMARY KEY (" + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "tracking_lot_id, " + GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX + "product_name,production_stage)";
    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidatedTables_AZ()
{
    QString				strQuery;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    QString strTableSpace, strTableEngine, strIndexSuffix, strCastNumber;
    if(m_pclDatabaseConnector->IsMySqlDB())
    {
        strIndexSuffix = "";
        strCastNumber = "unsigned ";
        // Get storage engine
        QString	strEngine,strFormat;
        GetStorageEngineName(strEngine,strFormat);
        strTableEngine = " ENGINE=" + strEngine + " " + strFormat + " ";
    }
    else
        if(m_pclDatabaseConnector->IsSQLiteDB())
        {
            strTableEngine = "";
            strIndexSuffix = "_index";
            strCastNumber = "integer";
        }
        else
            if(m_pclDatabaseConnector->IsOracleDB())
            {
                // Oracle
                // Found some specification
                // Get some information like for the PRODUCT table
                strQuery = "SELECT TABLESPACE_NAME,LOGGING FROM USER_TABLES WHERE TABLE_NAME='FT_LOT'";
                if(!clQuery.exec(strQuery))
                {
                    GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
                    return false;
                }

                clQuery.first();

                strTableSpace = clQuery.value(0).toString();
                if(clQuery.value(1).toString().compare("NO"))
                    strTableEngine = " TABLESPACE "+strTableSpace+" PCTFREE 5 PCTUSED 80 NOLOGGING COMPRESS ";
                else
                    strTableEngine = " TABLESPACE "+strTableSpace+" PCTFREE 5 PCTUSED 80 LOGGING COMPRESS ";

                strIndexSuffix = "";
                strCastNumber = "number";
            }

    ////////////////////////////////////////////////////////////////////////////////////
    // 1.0) Create az_consolidated_tl_data table
    ////////////////////////////////////////////////////////////////////////////////////
    if(!CreateConsolidatedTable_AZ_Data(strCastNumber, strTableEngine, strIndexSuffix))
        return false;

    ////////////////////////////////////////////////////////////////////////////////////
    // 2.0) Create az_consolidated_tl_facts table
    ////////////////////////////////////////////////////////////////////////////////////
    if(!CreateConsolidatedTable_AZ_Facts(strCastNumber, strTableEngine, strIndexSuffix))
        return false;

    ////////////////////////////////////////////////////////////////////////////////////
    // 3.0) Create az_consolidated_tl table
    ////////////////////////////////////////////////////////////////////////////////////
    if(!CreateConsolidatedTable_AZ(strCastNumber, strTableEngine, strIndexSuffix))
        return false;

    IncrementProgress();

    return true;
}

bool GexDbPlugin_Galaxy::CreateConsolidatedUpdateProcedure_AZ()
{
    if (!m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query	clQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString				strLogMessage, strQuery, strSubQuery_Data, strSubQuery_Facts, strSubQuery;
    QString				strProcedureName, strCastNumber, strTableAlias_FT, strTableAlias_WT;

    // TD-78

    // Drop old procedure
    clQuery.Execute("DROP PROCEDURE az_consolidated_tracking_lot_update");
    strProcedureName =   "az_consolidated_tl_update";

    if(m_pclDatabaseConnector->IsMySqlDB())
        strCastNumber = "unsigned ";
    else
        if(m_pclDatabaseConnector->IsSQLiteDB())
            strCastNumber = "integer";
        else
            if(m_pclDatabaseConnector->IsOracleDB())
                strCastNumber = "number";

    ////////////////////////////////////////////////////////////////////////////////////
    // 1) Drop procedure
    ////////////////////////////////////////////////////////////////////////////////////
    clQuery.Execute("DROP PROCEDURE " + strProcedureName);

    ////////////////////////////////////////////////////////////////////////////////////
    // 2) Create procedure
    ////////////////////////////////////////////////////////////////////////////////////
    strLogMessage = "Creating stored procedure " + strProcedureName+"...";
    InsertIntoUpdateLog(strLogMessage);

    // Build select queries
    CreateConsolidatedSelectQuery_AZ_Data(strSubQuery_Data, strTableAlias_FT, strTableAlias_WT, QString("IN_TrackingLotID"), strCastNumber);
    CreateConsolidatedSelectQuery_AZ_Facts(strSubQuery_Facts, QString("IN_TrackingLotID"), strCastNumber);
    CreateConsolidatedSelectQuery_AZ(strSubQuery, QString("IN_TrackingLotID"));

    // query for az_consolidated_tl_facts...

    if(m_pclDatabaseConnector->IsOracleDB())
    {
        strQuery =  "CREATE PROCEDURE " + strProcedureName + "(\n";
        strQuery += "	 IN_TrackingLotID	IN	varchar2)\n";
        strQuery += "AS\n";
        strQuery += "	Message		varchar2(1024);\n";
        strQuery += "	LogMessage	varchar2(1024);\n";
        strQuery += "	NbParts		varchar2(20);\n";
        strQuery += "BEGIN\n";
        strQuery += "\n";
        strQuery += "select SUM(nb_parts) into NbParts from ft_consolidated_tl where tracking_lot_id=IN_TrackingLotID;\n";
        strQuery += "IF (NbParts > 0) THEN\n";
        strQuery += "\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "-- az_consolidated_lot_data\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "	-- Log message\n";
        strQuery += "select 'Update az_consolidated_tl_data: TrackingLotID=' into LogMessage from dual;\n";
        strQuery += "select concat(LogMessage, IN_TrackingLotID) into LogMessage from dual;\n";
        strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(sysdate,'" + strProcedureName + "',LogMessage);\n";
        strQuery += "\n";
        strQuery += "	-- Remove old row\n";
        strQuery += "DELETE FROM az_consolidated_tl_data where ft_tracking_lot_id=IN_TrackingLotID;\n";
        strQuery += "\n";
        strQuery += "	-- Add new row\n";
        strQuery += "INSERT INTO az_consolidated_tl_data\n";
        strQuery += strSubQuery_Data;
        strQuery += ";\n";
        strQuery += "\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "-- az_consolidated_lot_facts\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "	-- Log message\n";
        strQuery += "select 'Update az_consolidated_tl_facts: TrackingLotID=' into LogMessage from dual;\n";
        strQuery += "select concat(LogMessage, IN_TrackingLotID) into LogMessage from dual;\n";
        strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(sysdate,'" + strProcedureName + "',LogMessage);\n";
        strQuery += "\n";
        strQuery += "	-- Remove old row\n";
        strQuery += "DELETE FROM az_consolidated_tl_facts where ft_tracking_lot_id=IN_TrackingLotID;\n";
        strQuery += "\n";
        strQuery += "	-- Add new row\n";
        strQuery += "INSERT INTO az_consolidated_tl_facts\n";
        strQuery += strSubQuery_Facts;
        strQuery += ";\n";
        strQuery += "\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "-- az_consolidated_lot\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "	-- Log message\n";
        strQuery += "select 'Update az_consolidated_tl: TrackingLotID=' into LogMessage from dual;\n";
        strQuery += "select concat(LogMessage, IN_TrackingLotID) into LogMessage from dual;\n";
        strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(sysdate,'" + strProcedureName + "',LogMessage);\n";
        strQuery += "\n";
        strQuery += "	-- Remove old row\n";
        strQuery += "DELETE FROM az_consolidated_tl where ft_tracking_lot_id=IN_TrackingLotID;\n";
        strQuery += "\n";
        strQuery += "	-- Add new row\n";
        strQuery += "INSERT INTO az_consolidated_tl\n";
        strQuery += strSubQuery;
        strQuery += ";\n";
        strQuery += "\n";
        strQuery += "END IF;\n";
        strQuery += "\n";
        strQuery += "END;\n";

    }
    else
    {
        strQuery =  "CREATE PROCEDURE " + strProcedureName + "(\n";
        strQuery += "	 IN	IN_TrackingLotID	varchar(255))\n";
        strQuery += "BEGIN\n";
        strQuery += "	DECLARE Message		varchar(1024);\n";
        strQuery += "	DECLARE LogMessage	varchar(1024);\n";
        strQuery += "	DECLARE NbParts		varchar(20);\n";
        strQuery += "\n";
        strQuery += "\n";
        strQuery += "select SUM(nb_parts) from ft_consolidated_tl where tracking_lot_id=IN_TrackingLotID into NbParts;\n";
        strQuery += "IF (NbParts > 0) THEN\n";
        strQuery += "\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "-- az_consolidated_lot_data\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "	-- Log message\n";
        strQuery += "select 'Update az_consolidated_tl_data: TrackingLotID=' into LogMessage from dual;\n";
        strQuery += "select concat(LogMessage, IN_TrackingLotID) into LogMessage from dual;\n";
        strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(NOW(),'" + strProcedureName + "',LogMessage);\n";
        strQuery += "\n";
        strQuery += "	-- Remove old row\n";
        strQuery += "DELETE FROM az_consolidated_tl_data where ft_tracking_lot_id=IN_TrackingLotID;\n";
        strQuery += "\n";
        strQuery += "	-- Add new row\n";
        strQuery += "INSERT INTO az_consolidated_tl_data\n";
        strQuery += strSubQuery_Data;
        strQuery += ";\n";
        strQuery += "\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "-- az_consolidated_lot_facts\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "	-- Log message\n";
        strQuery += "select 'Update az_consolidated_tl_facts: TrackingLotID=' into LogMessage from dual;\n";
        strQuery += "select concat(LogMessage, IN_TrackingLotID) into LogMessage from dual;\n";
        strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(NOW(),'" + strProcedureName + "',LogMessage);\n";
        strQuery += "\n";
        strQuery += "	-- Remove old row\n";
        strQuery += "DELETE FROM az_consolidated_tl_facts where ft_tracking_lot_id=IN_TrackingLotID;\n";
        strQuery += "\n";
        strQuery += "	-- Add new row\n";
        strQuery += "INSERT INTO az_consolidated_tl_facts\n";
        strQuery += strSubQuery_Facts;
        strQuery += ";\n";
        strQuery += "\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "-- az_consolidated_lot\n";
        strQuery += "-- -----------------------------------------------------------------\n";
        strQuery += "	-- Log message\n";
        strQuery += "select 'Update az_consolidated_tl: TrackingLotID=' into LogMessage from dual;\n";
        strQuery += "select concat(LogMessage, IN_TrackingLotID) into LogMessage from dual;\n";
        strQuery += "insert into gexdb_log (log_date,log_type,log_string) values(NOW(),'" + strProcedureName + "',LogMessage);\n";
        strQuery += "\n";
        strQuery += "	-- Remove old row\n";
        strQuery += "DELETE FROM az_consolidated_tl where ft_tracking_lot_id=IN_TrackingLotID;\n";
        strQuery += "\n";
        strQuery += "	-- Add new row\n";
        strQuery += "INSERT INTO az_consolidated_tl\n";
        strQuery += strSubQuery;
        strQuery += ";\n";
        strQuery += "\n";
        strQuery += "END IF;\n";
        strQuery += "\n";
        strQuery += "END\n";

    }

    if(!clQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clQuery.lastError().text().toLatin1().constData());
        return false;
    }
    InsertIntoUpdateLog("DONE.", false);
    IncrementProgress();

    return true;
}

