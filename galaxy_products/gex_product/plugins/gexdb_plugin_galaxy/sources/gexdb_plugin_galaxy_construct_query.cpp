#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_base.h"
#include "gex_constants.h"
#include "gex_shared.h"

///////////////////////////////////////////////////////////
// Query: construct query string for UPH query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConstructUPHQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, const QString & strSplitField, const QString & strCumulField)
{
    QString strCondition, strSubQuery, strFieldSpec, strDbField, strDbTable;

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <fields>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields
    Query_Empty();

    // Add split field?
    if(!strSplitField.isEmpty() && !Query_AddFieldExpression(strSplitField, "UPH_Split", strDbField, strDbTable))
        return false;

    // Add cumul field
    if(!Query_AddFieldExpression(strCumulField, "UPH_X_Axis", strDbField, strDbTable))
        return false;

    // Add fields needed for UPH
    strFieldSpec = "Field=UPH_NbParts|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts");
    strFieldSpec = "Field=UPH_Start_t|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.start_t");
    strFieldSpec = "Field=UPH_Finish_t|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.finish_t");

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))
        return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strSubQuery, false);

    // Construct UPH query
    if(!strSplitField.isEmpty())
    {
        strQuery = "SELECT\n";
        strQuery += "UPH_Split,\n";
        strQuery += "UPH_X_Axis,\n";
        strQuery += "SUM(UPH_NbParts) AS UPH_PARTS,\n";
        strQuery += "SUM(UPH_Finish_t-UPH_Start_t) AS UPH_TIME\n";
        strQuery += "FROM\n(";
        strQuery += strSubQuery;
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            strQuery += ") AS T1\nGROUP BY UPH_Split,UPH_X_Axis\nORDER BY UPH_Split ASC,UPH_X_Axis ASC";
        else
            strQuery += ")\nGROUP BY UPH_Split,UPH_X_Axis\nORDER BY UPH_Split ASC,UPH_X_Axis ASC";
    }
    else
    {
        strQuery = "SELECT\n";
        strQuery += "UPH_X_Axis,\n";
        strQuery += "SUM(UPH_NbParts) AS UPH_PARTS,\n";
        strQuery += "SUM(UPH_Finish_t-UPH_Start_t) AS UPH_TIME\n";
        strQuery += "FROM\n(";
        strQuery += strSubQuery;
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            strQuery += ") AS T1\nGROUP BY UPH_X_Axis\nORDER BY UPH_X_Axis ASC";
        else
            strQuery += ")\nGROUP BY UPH_X_Axis\nORDER BY UPH_X_Axis ASC";
    }

    return true;
}



///////////////////////////////////////////////////////////
// Query: construct query string for Consolidated Yield query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConstructConsolidatedYieldQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, const QString & strSplitField, const QString & strCumulField)
{
    QString strField, strCondition, strSubQuery, strFieldSpec, strDbField, strDbTable, strGroupBy;
    bool	bDistinct = true, bDateRelatedSplitField = true, bDateRelatedCumulField = true;

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <fields>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields
    Query_Empty();

    // Add split field if not date-related
    strField = strSplitField.toLower();
    if(!strField.isEmpty() &&	(strField.toLower() != "day") && (strField.toLower() != "week") &&
                                (strField.toLower() != "month") && (strField.toLower() != "year"))
    {
        if(!Query_AddFieldExpression(strSplitField, "YIELD_Split", strDbField, strDbTable))
            return false;
        strGroupBy = strDbField;
        bDateRelatedSplitField = false;
    }

    // Add cumul field if not date-related
    strField = strCumulField.toLower();
    if(	(strField.toLower() != "day") && (strField.toLower() != "week") &&
        (strField.toLower() != "month") && (strField.toLower() != "year"))
    {
        if(!Query_AddFieldExpression(strCumulField, "YIELD_X_Axis", strDbField, strDbTable))
            return false;
        if(!strGroupBy.isEmpty())
            strGroupBy += ",";
        strGroupBy += strDbField;
        bDateRelatedCumulField = false;
    }

    // Add fields needed for Yield (add lot_id only if not already inserted as cumul or split field)
    strFieldSpec = "Field=YIELD_X_Axis|" + m_strTablePrefix;
    strFieldSpec += "lot.lot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
    {
        strFieldSpec = "Field=YIELD_Split|" + m_strTablePrefix;
        strFieldSpec += "lot.lot_id";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        {
            strFieldSpec = "Field=YIELD_LotID|" + m_strTablePrefix;
            strFieldSpec += "lot.lot_id";
            m_strlQuery_Fields.append(strFieldSpec);
            if(!strGroupBy.isEmpty())
                strGroupBy += ",";
            strGroupBy += m_strTablePrefix + "lot.lot_id";
        }
    }
    strFieldSpec = "Field=YIELD_NbParts|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "lot.nb_parts");
    if(!strGroupBy.isEmpty())
        strGroupBy += ",";
    strGroupBy += m_strTablePrefix + "lot.nb_parts";
    strFieldSpec = "Field=YIELD_NbParts_Pass|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "lot.nb_parts_good");
    if(!strGroupBy.isEmpty())
        strGroupBy += ",";
    strGroupBy += m_strTablePrefix + "lot.nb_parts_good";

    // Add split field if date-related
    if(!strSplitField.isEmpty() && bDateRelatedSplitField)
    {
        if(!Query_AddFieldExpression(strSplitField, "YIELD_Split", strDbField, strDbTable, true))
            return false;
        bDistinct = false;
    }

    // Add cumul field if not date-related
    if(bDateRelatedCumulField)
    {
        if(!Query_AddFieldExpression(strCumulField, "YIELD_X_Axis", strDbField, strDbTable, true))
            return false;
        bDistinct = false;
    }

    // Add filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))
        return false;

    // Make sure the link condition between lot and splitlot tables is set
    strCondition = m_strTablePrefix;
    strCondition += "lot";
    strCondition += "-";
    strCondition += m_strTablePrefix;
    strCondition += "splitlot";
    Query_AddLinkCondition(strCondition);

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strSubQuery, bDistinct);

    // Add GROUP BY to the subquery?
    if(!bDistinct)
    {
        strSubQuery += "GROUP BY ";
        strSubQuery += strGroupBy;
    }

    // Construct Consolidated Yield query
    if(!strSplitField.isEmpty())
    {
        strQuery = "SELECT\n";
        strQuery += "YIELD_Split,\n";
        strQuery += "YIELD_X_Axis,\n";
        strQuery += "SUM(YIELD_NbParts) AS YIELD_PARTS,\n";
        strQuery += "SUM(YIELD_NbParts_Pass) AS YIELD_PASS\n";
        strQuery += "FROM\n(";
        strQuery += strSubQuery;
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            strQuery += ") AS T1\nGROUP BY YIELD_Split,YIELD_X_Axis\nORDER BY YIELD_Split ASC,YIELD_X_Axis ASC";
        else
            strQuery += ")\nGROUP BY YIELD_Split,YIELD_X_Axis\nORDER BY YIELD_Split ASC,YIELD_X_Axis ASC";
    }
    else
    {
        strQuery = "SELECT\n";
        strQuery += "YIELD_X_Axis,\n";
        strQuery += "SUM(YIELD_NbParts) AS YIELD_PARTS,\n";
        strQuery += "SUM(YIELD_NbParts_Pass) AS YIELD_PASS\n";
        strQuery += "FROM\n(";
        strQuery += strSubQuery;
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            strQuery += ") AS T1\nGROUP BY YIELD_X_Axis\nORDER BY YIELD_X_Axis ASC";
        else
            strQuery += ")\nGROUP BY YIELD_X_Axis\nORDER BY YIELD_X_Axis ASC";
    }

    return true;
}


///////////////////////////////////////////////////////////
// Query: construct query string for Yield query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConstructYieldQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, const QString & strSplitField, const QString & strCumulField, int nBinning, bool bSoftBin)
{
    // Check wich type of query should be generated:
    // 1) No Site# in cumul/split fields AND no BINNING specified: standard query
    // 2) Site# in cumul/split fields AND no BINNING specified: query with a UNION (samples, summary)
    // 3) BINNING specified: query with UNION (samples, summary) and LEFT JOIN (parts, binning)

    if((strSplitField != m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR])
            && (strCumulField != m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR])
            && (nBinning == -1))
        return ConstructYieldQuery_NoSite_NoBinning(cFilters, strQuery, strSplitField, strCumulField);

    if(nBinning == -1)
        return ConstructYieldQuery_Site_NoBinning(cFilters, strQuery, strSplitField, strCumulField);

    return ConstructYieldQuery_Binning(cFilters, strQuery, strSplitField, strCumulField, nBinning, bSoftBin);
}

///////////////////////////////////////////////////////////
// Query: construct query string for Yield query (no site, no binning)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConstructYieldQuery_NoSite_NoBinning(GexDbPlugin_Filter &cFilters, QString & strQuery, const QString & strSplitField, const QString & strCumulField)
{
    QString strCondition, strSubQuery, strFieldSpec, strDbField, strDbTable, strUnionQuery1, strUnionQuery2;

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <fields>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields

    Query_Empty();
    // Add split field?
    if(!strSplitField.isEmpty() && !Query_AddFieldExpression(strSplitField, "YIELD_Split", strDbField, strDbTable))
        return false;

    // Add cumul field
    if(!Query_AddFieldExpression(strCumulField, "YIELD_X_Axis", strDbField, strDbTable))
        return false;

    // Add fields needed for Yield
    strFieldSpec = "Field=YIELD_NbParts|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts");
    strFieldSpec = "Field=YIELD_NbParts_Pass|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts_good");

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strSubQuery, false);

    if (!m_pclDatabaseConnector)
        return false;

    // Construct Yield query
    if(!strSplitField.isEmpty())
    {
        strQuery = "SELECT\n";
        strQuery += "YIELD_Split,\n";
        strQuery += "YIELD_X_Axis,\n";
        strQuery += "SUM(YIELD_NbParts) AS YIELD_PARTS,\n";
        strQuery += "SUM(YIELD_NbParts_Pass) AS YIELD_PASS\n";
        strQuery += "FROM\n(";
        strQuery += strSubQuery;
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            strQuery += ") AS T1\nGROUP BY YIELD_Split,YIELD_X_Axis\nORDER BY YIELD_Split ASC,YIELD_X_Axis ASC";
        else
            strQuery += ")\nGROUP BY YIELD_Split,YIELD_X_Axis\nORDER BY YIELD_Split ASC,YIELD_X_Axis ASC";
    }
    else
    {
        strQuery = "SELECT\n";
        strQuery += "YIELD_X_Axis,\n";
        strQuery += "SUM(YIELD_NbParts) AS YIELD_PARTS,\n";
        strQuery += "SUM(YIELD_NbParts_Pass) AS YIELD_PASS\n";
        strQuery += "FROM\n(";
        strQuery += strSubQuery;
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            strQuery += ") AS T1\nGROUP BY YIELD_X_Axis\nORDER BY YIELD_X_Axis ASC";
        else
            strQuery += ")\nGROUP BY YIELD_X_Axis\nORDER BY YIELD_X_Axis ASC";
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: construct query string for Yield query (site, no binning)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConstructYieldQuery_Site_NoBinning(GexDbPlugin_Filter &cFilters, QString & strQuery, const QString & strSplitField, const QString & strCumulField)
{
    QString strCondition, strSubQuery, strFieldSpec, strDbField, strDbTable, strUnionQuery1, strUnionQuery2;

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <fields>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields

    // Create a union between samples and summary data
    /////////////////// FIRST PART OF THE UNION: SAMPLES ///////////////////////////////////////
    Query_Empty();
    // Add split field?
    if(!strSplitField.isEmpty()
            && !Query_AddFieldExpression(strSplitField, "YIELD_Split", strDbField, strDbTable, false, true))
        return false;

    // Add cumul field
    if(!Query_AddFieldExpression(strCumulField, "YIELD_X_Axis", strDbField, strDbTable, false, true))
        return false;

    // Add fields needed for Yield (use samples per site)
    if(!Query_AddFieldExpression("stats_nbparts", "YIELD_NbParts", strDbField, strDbTable, false, true))
        return false;
    if(!Query_AddFieldExpression("stats_nbmatching", "YIELD_NbParts_Pass", strDbField, strDbTable, false, true))
        return false;
    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=YIELD_SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);
    // Add site_no to make sure each line is unique (in case different sites for same splitlot have same part counts)
    strFieldSpec = "Field=YIELD_X_Axis|" + m_strTablePrefix;
    strFieldSpec += "parts_stats_samples.site_no";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
    {
        strFieldSpec = "Field=YIELD_Split|" + m_strTablePrefix;
        strFieldSpec += "parts_stats_samples.site_no";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        {
            strFieldSpec = "Field=YIELD_SiteNo|" + m_strTablePrefix;
            strFieldSpec += "parts_stats_samples.site_no";
            m_strlQuery_Fields.append(strFieldSpec);
        }
    }

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strUnionQuery1, true);

    /////////////////// SECOND PART OF THE UNION: SUMMARY ///////////////////////////////////////
    Query_Empty();
    // Add split field?
    if(!strSplitField.isEmpty()
            && !Query_AddFieldExpression(strSplitField, "YIELD_Split", strDbField, strDbTable, false, false))
        return false;

    // Add cumul field
    if(!Query_AddFieldExpression(strCumulField, "YIELD_X_Axis", strDbField, strDbTable, false, false))
        return false;

    // Add fields needed for Yield (use samples per site)
    if(!Query_AddFieldExpression("stats_nbparts", "YIELD_NbParts", strDbField, strDbTable, false, false))
        return false;
    if(!Query_AddFieldExpression("stats_nbmatching", "YIELD_NbParts_Pass", strDbField, strDbTable, false, false))
        return false;
    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=YIELD_SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);
    // Add site_no to make sure each line is unique (in case different sites for same splitlot have same part counts)
    strFieldSpec = "Field=YIELD_X_Axis|" + m_strTablePrefix;
    strFieldSpec += "parts_stats_summary.site_no";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
    {
        strFieldSpec = "Field=YIELD_Split|" + m_strTablePrefix;
        strFieldSpec += "parts_stats_summary.site_no";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        {
            strFieldSpec = "Field=YIELD_SiteNo|" + m_strTablePrefix;
            strFieldSpec += "parts_stats_summary.site_no";
            m_strlQuery_Fields.append(strFieldSpec);
        }
    }

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strUnionQuery2, true);

    // Build the union query
    strSubQuery = "(\n";
    strSubQuery += strUnionQuery1;
    strSubQuery += ")\nUNION\n(\n";
    strSubQuery += strUnionQuery2;
    strSubQuery += ")\n";

    if (!m_pclDatabaseConnector)
        return false;

    // Construct Yield query
    if(!strSplitField.isEmpty())
    {
        strQuery = "SELECT\n";
        strQuery += "YIELD_Split,\n";
        strQuery += "YIELD_X_Axis,\n";
        strQuery += "SUM(YIELD_NbParts) AS YIELD_PARTS,\n";
        strQuery += "SUM(YIELD_NbParts_Pass) AS YIELD_PASS\n";
        strQuery += "FROM\n(";
        strQuery += strSubQuery;
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            strQuery += ") AS T1\nGROUP BY YIELD_Split,YIELD_X_Axis\nORDER BY YIELD_Split ASC,YIELD_X_Axis ASC";
        else
            strQuery += ")\nGROUP BY YIELD_Split,YIELD_X_Axis\nORDER BY YIELD_Split ASC,YIELD_X_Axis ASC";
    }
    else
    {
        strQuery = "SELECT\n";
        strQuery += "YIELD_X_Axis,\n";
        strQuery += "SUM(YIELD_NbParts) AS YIELD_PARTS,\n";
        strQuery += "SUM(YIELD_NbParts_Pass) AS YIELD_PASS\n";
        strQuery += "FROM\n(";
        strQuery += strSubQuery;
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            strQuery += ") AS T1\nGROUP BY YIELD_X_Axis\nORDER BY YIELD_X_Axis ASC";
        else
            strQuery += ")\nGROUP BY YIELD_X_Axis\nORDER BY YIELD_X_Axis ASC";
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: construct query string for Yield query (binning)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConstructYieldQuery_Binning(
        GexDbPlugin_Filter &cFilters,
        QString & strQuery,
        const QString & strSplitField,
        const QString & strCumulField,
        int nBinning, bool bSoftBin)
{
    QString strCondition, strSubQuery, strFieldSpec, strDbField, strDbTable, strUnionQuery1, strUnionQuery2;
    QString	strJoinQuery1, strJoinQuery2;

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <fields>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields

    // Create a union between samples and summary data
    /////////////////// FIRST PART OF THE UNION: SAMPLES ///////////////////////////////////////

    // Create a LEFT JOIN between parts and binning data
    /////////////////// FIRST PART OF THE JOIN: PARTS ///////////////////////////////////////
    Query_Empty();
    // Add split field?
    if(!strSplitField.isEmpty()
            && !Query_AddFieldExpression(strSplitField, "YIELD_Split", strDbField, strDbTable, false, true))
        return false;

    // Add cumul field
    if(!Query_AddFieldExpression(strCumulField, "YIELD_X_Axis", strDbField, strDbTable, false, true))
        return false;

    // Add fields needed for Yield (use samples per site)
    if(!Query_AddFieldExpression("stats_nbparts", "YIELD_NbParts", strDbField, strDbTable, false, true))
        return false;
    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=YIELD_SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);
    // Add site_no to make sure each line is unique (in case different sites for same splitlot have same part counts)
    strFieldSpec = "Field=YIELD_SiteNo|" + m_strTablePrefix;
    strFieldSpec += "parts_stats_samples.site_no";
    m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strJoinQuery1, true);

    /////////////////// SECOND PART OF THE JOIN: BINNING ///////////////////////////////////////
    Query_Empty();
    // Add split field?
    if(!strSplitField.isEmpty()
        && !Query_AddFieldExpression(strSplitField, "YIELD_Split", strDbField, strDbTable, false, true, nBinning, bSoftBin))
        return false;

    // Add cumul field
    if(!Query_AddFieldExpression(strCumulField, "YIELD_X_Axis", strDbField, strDbTable, false, true, nBinning, bSoftBin))
        return false;

    // Add fields needed for Yield (use samples per site)
    if(!Query_AddFieldExpression("stats_nbmatching", "YIELD_NbParts_Matching", strDbField, strDbTable, false, true, nBinning, bSoftBin))
        return false;
    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=YIELD_SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);
    // Add site_no to make sure each line is unique (in case different sites for same splitlot have same part counts)
    strFieldSpec = "Field=YIELD_SiteNo|" + m_strTablePrefix;
    if(bSoftBin)
        strFieldSpec += "sbin_stats_samples.site_no";
    else
        strFieldSpec += "hbin_stats_samples.site_no";
    m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strJoinQuery2, true);

    // Build the JOIN query
    strUnionQuery1 = "SELECT\n";
    if(!strSplitField.isEmpty())
        strUnionQuery1 += "TPARTS.YIELD_Split AS YIELD_Split,\n";
    strUnionQuery1 += "TPARTS.YIELD_X_Axis AS YIELD_X_Axis,\n";
    strUnionQuery1 += "TPARTS.YIELD_NbParts AS YIELD_NbParts,\n";
    strUnionQuery1 += "TBINNING.YIELD_NbParts_Matching AS YIELD_NbParts_Matching,\n";
    strUnionQuery1 += "TPARTS.YIELD_SplitlotId AS YIELD_SplitlotId,\n";
    strUnionQuery1 += "TPARTS.YIELD_SiteNo AS YIELD_SiteNo\n";
    strUnionQuery1 += "FROM\n";
    strUnionQuery1 += "(\n";
    strUnionQuery1 += strJoinQuery1;
    strUnionQuery1 += ") TPARTS\n";
    strUnionQuery1 += "LEFT OUTER JOIN\n";
    strUnionQuery1 += "(\n";
    strUnionQuery1 += strJoinQuery2;
    strUnionQuery1 += ") TBINNING\n";
    strUnionQuery1 += "ON TPARTS.YIELD_SplitlotId=TBINNING.YIELD_SplitlotId AND TPARTS.YIELD_SiteNo=TBINNING.YIELD_SiteNo\n";

    /////////////////// SECOND PART OF THE UNION: SUMMARY ///////////////////////////////////////

    // Create a LEFT JOIN between parts and binning data
    /////////////////// FIRST PART OF THE JOIN: PARTS ///////////////////////////////////////
    Query_Empty();
    // Add split field?
    if(!strSplitField.isEmpty() && !Query_AddFieldExpression(strSplitField, "YIELD_Split", strDbField, strDbTable, false, false))
        return false;

    // Add cumul field
    if(!Query_AddFieldExpression(strCumulField, "YIELD_X_Axis", strDbField, strDbTable, false, false))
        return false;

    // Add fields needed for Yield (use samples per site)
    if(!Query_AddFieldExpression("stats_nbparts", "YIELD_NbParts", strDbField, strDbTable, false, false))
        return false;
    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=YIELD_SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);
    // Add site_no to make sure each line is unique (in case different sites for same splitlot have same part counts)
    strFieldSpec = "Field=YIELD_SiteNo|" + m_strTablePrefix;
    strFieldSpec += "parts_stats_summary.site_no";
    m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strJoinQuery1, true);

    /////////////////// SECOND PART OF THE JOIN: BINNING ///////////////////////////////////////
    Query_Empty();
    // Add split field?
    if(!strSplitField.isEmpty() && !Query_AddFieldExpression(strSplitField, "YIELD_Split", strDbField, strDbTable, false, false, nBinning, bSoftBin))
        return false;

    // Add cumul field
    if(!Query_AddFieldExpression(strCumulField, "YIELD_X_Axis", strDbField, strDbTable, false, false, nBinning, bSoftBin))
        return false;

    // Add fields needed for Yield (use samples per site)
    if(!Query_AddFieldExpression("stats_nbmatching", "YIELD_NbParts_Matching", strDbField, strDbTable, false, false, nBinning, bSoftBin))
        return false;

    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=YIELD_SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);
    // Add site_no to make sure each line is unique (in case different sites for same splitlot have same part counts)
    strFieldSpec = "Field=YIELD_SiteNo|" + m_strTablePrefix;
    if(bSoftBin)
        strFieldSpec += "sbin_stats_summary.site_no";
    else
        strFieldSpec += "hbin_stats_summary.site_no";
    m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strJoinQuery2, true);

    // Build the JOIN query
    strUnionQuery2 = "SELECT\n";
    if(!strSplitField.isEmpty())
        strUnionQuery2 += "TPARTS.YIELD_Split AS YIELD_Split,\n";
    strUnionQuery2 += "TPARTS.YIELD_X_Axis AS YIELD_X_Axis,\n";
    strUnionQuery2 += "TPARTS.YIELD_NbParts AS YIELD_NbParts,\n";
    strUnionQuery2 += "TBINNING.YIELD_NbParts_Matching AS YIELD_NbParts_Matching,\n";
    strUnionQuery2 += "TPARTS.YIELD_SplitlotId AS YIELD_SplitlotId,\n";
    strUnionQuery2 += "TPARTS.YIELD_SiteNo AS YIELD_SiteNo\n";
    strUnionQuery2 += "FROM\n";
    strUnionQuery2 += "(\n";
    strUnionQuery2 += strJoinQuery1;
    strUnionQuery2 += ") TPARTS\n";
    strUnionQuery2 += "LEFT OUTER JOIN\n";
    strUnionQuery2 += "(\n";
    strUnionQuery2 += strJoinQuery2;
    strUnionQuery2 += ") TBINNING\n";
    strUnionQuery2 += "ON TPARTS.YIELD_SplitlotId=TBINNING.YIELD_SplitlotId AND TPARTS.YIELD_SiteNo=TBINNING.YIELD_SiteNo\n";

    // Build the union query
    strSubQuery = "(\n";
    strSubQuery += strUnionQuery1;
    strSubQuery += ")\nUNION\n(\n";
    strSubQuery += strUnionQuery2;
    strSubQuery += ")\n";

    if(!m_pclDatabaseConnector)
        return false;

    // Construct Yield query
    if(!strSplitField.isEmpty())
    {
        strQuery = "SELECT\n";
        strQuery += "YIELD_Split,\n";
        strQuery += "YIELD_X_Axis,\n";
        strQuery += "SUM(YIELD_NbParts) AS YIELD_PARTS,\n";
        strQuery += "SUM((case when YIELD_NbParts_Matching IS NULL then 0 else YIELD_NbParts_Matching end)) AS YIELD_MATCHING\n";
        strQuery += "FROM\n(";
        strQuery += strSubQuery;
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            strQuery += ") AS T1\nGROUP BY YIELD_Split,YIELD_X_Axis\nORDER BY YIELD_Split ASC,YIELD_X_Axis ASC";
        else
            strQuery += ")\nGROUP BY YIELD_Split,YIELD_X_Axis\nORDER BY YIELD_Split ASC,YIELD_X_Axis ASC";
    }
    else
    {
        strQuery = "SELECT\n";
        strQuery += "YIELD_X_Axis,\n";
        strQuery += "SUM(YIELD_NbParts) AS YIELD_PARTS,\n";
        strQuery += "SUM((case when YIELD_NbParts_Matching IS NULL then 0 else YIELD_NbParts_Matching end)) AS YIELD_MATCHING\n";
        strQuery += "FROM\n(";
        strQuery += strSubQuery;
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            strQuery += ") AS T1\nGROUP BY YIELD_X_Axis\nORDER BY YIELD_X_Axis ASC";
        else
            strQuery += ")\nGROUP BY YIELD_X_Axis\nORDER BY YIELD_X_Axis ASC";
    }

    return true;
}
