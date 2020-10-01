// gexdb_plugin_galaxy_extraction_er.cpp:
// implementation of Enterprise Reports data extraction functions of GexDbPlugin_Galaxy class.
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
#include <gqtl_log.h>
#include "gexdb_plugin_galaxy.h"
#include "gex_constants.h"
#include "gex_shared.h"


// Standard includes
#include <math.h>

// Qt includes
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>

// Galaxy modules includes
#include <gqtl_sysutils.h>

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Get bin counts for Enterprise Report graphs (Yield, UPH)
// (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Prod_GetBinnings(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData, GexDbPlugin_ER_Parts_Graph *pGraph, GexDbPlugin_ER_Parts_Layer *pLayer, const QString & strAggregateLabel, GexDbPlugin_BinList & clBinList)
{
    // Trace message
    WriteDebugMessageFile("---- ER_Prod_GetBinnings()...");

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate)
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

    // Check testingstage to query
    if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = "et_";
    }
    else if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = "wt_";
    }
    else if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
        m_strTablePrefix = "ft_";
    }
    else
        return false;

    m_strTestingStage = cFilters.strDataTypeQuery;

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // Construct SQL query for Yield
    QString strQuery;
    if(!ER_Prod_ConstructBinCountQuery(cFilters, clER_PartsData, pGraph, pLayer, strAggregateLabel, strQuery))
        return false;

    // Save query
    //clER_PartsData.m_strYieldQuery = strQuery;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clGexDbQuery.lastError().text().left(1024).toLatin1().constData());
        return false;
    }

    // Fill Binning List
    int					nRows=0, nBinCount=0;
    GexDbPlugin_BinInfo	*pBinInfo;
    while(clGexDbQuery.Next())
    {
        nRows++;

        if(clGexDbQuery.isNull(0))
            continue;
        if(clGexDbQuery.isNull(2))
            continue;

        nBinCount = clGexDbQuery.value(2).toInt();
        if(nBinCount == 0)
            continue;

        pBinInfo = new GexDbPlugin_BinInfo;
        pBinInfo->m_nBinNo			= clGexDbQuery.value(0).toInt();
        if(!clGexDbQuery.isNull(1))
            pBinInfo->m_strBinName	= clGexDbQuery.value(1).toString();
        pBinInfo->m_nBinCount		= nBinCount;
        if(!clGexDbQuery.isNull(3))
            pBinInfo->m_cBinCat		= clGexDbQuery.value(3).toString()[0];

        clBinList.append(pBinInfo);
    }

    // Check if we have some results
    if(nRows == 0)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoResult_0, NULL);
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Computes data for Enterprise Report Production graphs (Yield, UPH)
// (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Prod_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData)
{
    // Trace message
    WriteDebugMessageFile("---- ER_Prod_GetParts()...");

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate)
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

    // Check testingstage to query
    if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = "et_";
    }
    else if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = "wt_";
    }
    else if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
        m_strTablePrefix = "ft_";
    }
    else
        return false;

    m_strTestingStage = cFilters.strDataTypeQuery;

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

#if 0 // m_strBinlist_Pass and m_strBinlist_Fail not used anymore in ER queries
    // Get list of 'Pass' bins and list of 'Fail' bins
    if(!QueryBinlist(cFilters,clER_PartsData.m_strBinlist_Pass,"pass",clER_PartsData.m_bSoftBin, true, false, true))
        return false;
    if(!QueryBinlist(cFilters,clER_PartsData.m_strBinlist_Fail,"fail",clER_PartsData.m_bSoftBin, true, false, true))
        return false;
#endif

    // Construct SQL query for Yield
    QString strQuery;
    if(!ER_Prod_ConstructPartQuery(cFilters, clER_PartsData, strQuery))
        return false;

    // Save query
    clER_PartsData.m_strYieldQuery = strQuery;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clGexDbQuery.lastError().text().left(1024).toLatin1().constData());
        return false;
    }

    // Fill Parts data object
    QString										strGraphSplit_Current, strGraphSplit_New;
    QString										strLayerSplit_Current, strLayerSplit_New;
    QString										strValue, strAggregate, strGraphLayerSplit;
    QStringList									strlGraphSplit, strlLayerSplit;
    int											nRows=0, nIndex;
    unsigned int								uiNbParts, uiNbParts_Good, uiIndex, uiNbSplitlots, uiNbWafers, uiNbLots, uiNbMaverickWafers, uiNbMaverickLots;
    bool										bCreateNewGraph=true, bCreateNewLayer=true;
    QStringList::iterator						it;
    GexDbPlugin_ER_Parts_Graph					*pGraph=NULL;
    GexDbPlugin_ER_Parts_Layer					*pLayer=NULL;
    QList<unsigned int>							uilMatchingParts;
    QMap<QString, GexDbPlugin_ER_Parts_Layer*>	mapLayers;

    strGraphSplit_Current = strLayerSplit_Current = "";
    while(clGexDbQuery.Next())
    {
        nRows++;

        // Get query data
        nIndex = 0;
        strGraphSplit_New="";
        strLayerSplit_New="";
        strlGraphSplit.clear();
        strlLayerSplit.clear();
        for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
        {
            strValue = clGexDbQuery.value(nIndex++).toString();
            strlGraphSplit.append(strValue);
            strGraphSplit_New += strValue;
        }
        for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
        {
            strValue = clGexDbQuery.value(nIndex++).toString();
            strlLayerSplit.append(strValue);
            strLayerSplit_New += strValue;
        }
        strGraphLayerSplit = strGraphSplit_New + strLayerSplit_New;
        strAggregate = clGexDbQuery.value(nIndex++).toString();
        if(clGexDbQuery.isNull(nIndex))
            uiNbParts = 0;
        else
            uiNbParts = clGexDbQuery.value(nIndex).toUInt();
        nIndex++;
        if(clGexDbQuery.isNull(nIndex))
            uiNbParts_Good = 0;
        else
            uiNbParts_Good = clGexDbQuery.value(nIndex).toUInt();
        nIndex++;
        uilMatchingParts.clear();
        for(uiIndex=0; uiIndex<clER_PartsData.m_uiSeries; uiIndex++)
        {
            if(clGexDbQuery.isNull(nIndex))
                uilMatchingParts.append(0);
            else
                uilMatchingParts.append(clGexDbQuery.value(nIndex).toUInt());
            nIndex++;
        }

        // Check if new graph should be created
        if(strGraphSplit_New != strGraphSplit_Current)
            bCreateNewGraph = true;
        strGraphSplit_Current = strGraphSplit_New;

        // Check if new layer should be created
        if(strLayerSplit_New != strLayerSplit_Current)
            bCreateNewLayer = true;
        strLayerSplit_Current = strLayerSplit_New;

        // Create new graph/layer
        if(bCreateNewGraph)
        {
            if(pLayer)
                pGraph->append(pLayer);
            if(pGraph)
                clER_PartsData.append(pGraph);
            pGraph = new GexDbPlugin_ER_Parts_Graph(strlGraphSplit);
            pLayer = new GexDbPlugin_ER_Parts_Layer(clER_PartsData.m_uiSeries, strlLayerSplit);
            bCreateNewGraph = false;
            bCreateNewLayer = false;

            // Add layer to map
            mapLayers[strGraphLayerSplit] = pLayer;
        }
        else if(bCreateNewLayer)
        {
            if(pLayer)
                pGraph->append(pLayer);
            pLayer = new GexDbPlugin_ER_Parts_Layer(clER_PartsData.m_uiSeries, strlLayerSplit);
            bCreateNewLayer = false;

            // Add layer to map
            mapLayers[strGraphLayerSplit] = pLayer;
        }

        // Add data to layer
        pLayer->Add(strAggregate, uiNbParts, uiNbParts_Good, 0.0, uilMatchingParts);
    }

    // Add last layer/graph
    if(pLayer)
        pGraph->append(pLayer);
    if(pGraph)
        clER_PartsData.append(pGraph);

    // Check if we have some results
    if(nRows == 0)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoResult_0, NULL);
        return false;
    }

    // Construct SQL query for Nb splitlots, wafers, lots, Mav wafers, Mav lots
    if(!ER_Prod_ConstructSplitCountQuery(cFilters, clER_PartsData, strQuery))
        return false;

    // Save query
    clER_PartsData.m_strSplitCountQuery = strQuery;

    // Execute query
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clGexDbQuery.lastError().text().left(1024).toLatin1().constData());
        return false;
    }

    QString strF1;
    while(clGexDbQuery.Next())
    {
        uiNbWafers = uiNbMaverickWafers = uiNbMaverickLots = 0;

        // Get query data
        nIndex = 0;
        strGraphSplit_New="";
        strLayerSplit_New="";
        for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
            strGraphSplit_New += clGexDbQuery.value(nIndex++).toString();
        for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
            strLayerSplit_New += clGexDbQuery.value(nIndex++).toString();
        strGraphLayerSplit = strGraphSplit_New + strLayerSplit_New;
        strAggregate = clGexDbQuery.value(nIndex++).toString();
        uiNbSplitlots = clGexDbQuery.value(nIndex++).toUInt();
        if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
            uiNbWafers = clGexDbQuery.value(nIndex++).toUInt();
        uiNbLots = clGexDbQuery.value(nIndex++).toUInt();
        if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
        {
            uiNbMaverickWafers = clGexDbQuery.value(nIndex++).toUInt();
            uiNbMaverickLots = clGexDbQuery.value(nIndex++).toUInt();
        }

        // Check if data found for this Graph/Layer
        if(mapLayers.contains(strGraphLayerSplit))
        {
            // Update layer
            pLayer = mapLayers[strGraphLayerSplit];
            pLayer->Update(strAggregate, uiNbSplitlots, uiNbWafers, uiNbLots, uiNbMaverickWafers, uiNbMaverickLots);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Prod_ConstructPartQuery(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData, QString & strQuery)
{
    // Check wich type of query should be generated:
    // 1) BINNING OR Site# specified: query with UNION (samples, summary) and LEFT JOIN (parts, binning)
    // 2) No Site# in cumul/split fields AND no BINNING specified: standard query
    if(clER_PartsData.m_bWorkOnBinnings)
        return ER_Prod_ConstructPartQuery_Binning(cFilters, clER_PartsData, strQuery);

    if(	(clER_PartsData.m_strField_Aggregate != m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR]) &&
        (!clER_PartsData.m_strlFields_GraphSplit.contains(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR])) &&
        (!clER_PartsData.m_strlFields_LayerSplit.contains(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR])))
        return ER_Prod_ConstructPartQuery_NoBinning(cFilters, clER_PartsData, strQuery);

    return ER_Prod_ConstructPartQuery_Binning(cFilters, clER_PartsData, strQuery);
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts (no site, no binning)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Prod_ConstructPartQuery_NoBinning(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData, QString & strQuery)
{
    QString							strAs, strBinList, strCondition, strSubQuery, strFieldSpec, strDbField, strDbTable;
    QString							strGraphSplitAliases_GroupBy, strLayerSplitAliases_GroupBy;
    QString							strGraphSplitAliases_OrderBy, strLayerSplitAliases_OrderBy;
    QStringList::iterator			it;
    GexDbPlugin_ER_Parts_SerieDef	*pSerieDef;

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <fields>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields

    Query_Empty();

    // Add Graph split fields?
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;

        // Save list of graph split aliases
        if(!strGraphSplitAliases_GroupBy.isEmpty())
        {
            strGraphSplitAliases_GroupBy += ",";
            strGraphSplitAliases_OrderBy += ",";
        }
        strGraphSplitAliases_GroupBy += strAs;
        strGraphSplitAliases_OrderBy += strAs + " ASC";
    }

    // Add Layer split fields?
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;

        // Save list of layer split aliases
        if(!strLayerSplitAliases_GroupBy.isEmpty())
        {
            strLayerSplitAliases_GroupBy += ",";
            strLayerSplitAliases_OrderBy += ",";
        }
        strLayerSplitAliases_GroupBy += strAs;
        strLayerSplitAliases_OrderBy += strAs + " ASC";
    }

    // Add aggregate field
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);

    if(!ER_Query_AddFieldExpression(clER_PartsData.m_strField_Aggregate , strAs, strDbField, strDbTable))
        return false;

    // Add total parts field
    strFieldSpec = "Field=Parts|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts");
    strFieldSpec = "Field=Parts_Good|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts_good");

    // Add fields for data series
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        strAs = pSerieDef->m_strSerieName;
        Query_NormalizeAlias(strAs);
        strBinList = pSerieDef->m_strBinnings.toLower();
        if(strBinList.indexOf("pass") != -1)
        {
            strFieldSpec = "Field=" + strAs;
            strFieldSpec += "|" + m_strTablePrefix;
            m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts_good");
        }
        else if(strBinList.indexOf("fail") != -1)
        {
            strFieldSpec = "Expression=" + strAs;
            strFieldSpec += "|" + m_strTablePrefix;
            strFieldSpec += "splitlot.nb_parts";
            strFieldSpec += "|(" + m_strTablePrefix;
            strFieldSpec += "splitlot.nb_parts-";
            strFieldSpec += m_strTablePrefix + "splitlot.nb_parts_good)";
            m_strlQuery_Fields.append(strFieldSpec);
        }
        else if(strBinList.indexOf("all") != -1)
        {
            strFieldSpec = "Field=" + strAs;
            strFieldSpec += "|" + m_strTablePrefix;
            m_strlQuery_Fields.append(strFieldSpec + "splitlot.nb_parts");
        }
        else
        {
            // ToDO: set error message
            return false;
        }
//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = m_strTablePrefix + "splitlot.prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString_UsingJoins(strSubQuery, false);

    // Construct Yield query
    strQuery = "SELECT\n";
    if(!strGraphSplitAliases_GroupBy.isEmpty())
    {
        strQuery += strGraphSplitAliases_GroupBy;
        strQuery += ",\n";
    }
    if(!strLayerSplitAliases_GroupBy.isEmpty())
    {
        strQuery += strLayerSplitAliases_GroupBy;
        strQuery += ",\n";
    }
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    strQuery += strAs + ",\n";
    strQuery += "SUM(Parts) AS Parts,\n";
    strQuery += "SUM(Parts_Good) AS Parts_Good";
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        strAs = pSerieDef->m_strSerieName;
        Query_NormalizeAlias(strAs);
        strQuery += ",\nSUM(" + strAs;
        strQuery += ") AS " + strAs;
//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }
    strQuery += "\n";
    strQuery += "FROM\n(\n";
    strQuery += strSubQuery + ")";
    if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        strQuery += " AS T1";
    strQuery += "\n";
    strQuery += "GROUP BY ";
    if(!strGraphSplitAliases_GroupBy.isEmpty())
    {
        strQuery += strGraphSplitAliases_GroupBy;
        strQuery += ",";
    }
    if(!strLayerSplitAliases_GroupBy.isEmpty())
    {
        strQuery += strLayerSplitAliases_GroupBy;
        strQuery += ",";
    }
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    strQuery += strAs + "\n";
    strQuery += "ORDER BY ";
    if(!strGraphSplitAliases_OrderBy.isEmpty())
    {
        strQuery += strGraphSplitAliases_OrderBy;
        strQuery += ",";
    }
    if(!strLayerSplitAliases_OrderBy.isEmpty())
    {
        strQuery += strLayerSplitAliases_OrderBy;
        strQuery += ",";
    }
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    strQuery += strAs + " ASC\n";

    return true;
}

#if 0
///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts (binning)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Prod_ConstructPartQuery_Binning(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData, QString & strQuery)
{
    QString							strAs, strCondition, strFieldSpec, strDbField, strDbTable;
    QString							strSerieName, strBinList;
    QString							strAliases_GroupBy, strAliases_OrderBy;
    QString							strAliases_GroupBy_As, strAliases_OrderBy_As;
    QStringList::iterator			it, itSerie;
    QString							strSubQuery, strUnionQuery1, strUnionQuery2;
    QStringList						strlJoinQueries;
    GexDbPlugin_ER_Parts_SerieDef	*pSerieDef;

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

    // Add Graph split fields?
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;

        // Save list of graph split fields for 'group by' and 'order by'
        if(!strAliases_GroupBy.isEmpty())
        {
            strAliases_GroupBy += ",";
            strAliases_OrderBy += ",";
            strAliases_GroupBy_As += ",";
            strAliases_OrderBy_As += ",";
        }
        strAliases_GroupBy += strDbField;
        strAliases_OrderBy += strDbField + " ASC";
        strAliases_GroupBy_As += strAs;
        strAliases_OrderBy_As += strAs + " ASC";
    }

    // Add Layer split fields?
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;

        // Save list of layer split fields for 'group by' and 'order by'
        if(!strAliases_GroupBy.isEmpty())
        {
            strAliases_GroupBy += ",";
            strAliases_OrderBy += ",";
            strAliases_GroupBy_As += ",";
            strAliases_OrderBy_As += ",";
        }
        strAliases_GroupBy += strDbField;
        strAliases_OrderBy += strDbField + " ASC";
        strAliases_GroupBy_As += strAs;
        strAliases_OrderBy_As += strAs + " ASC";
    }

    // Add aggregate field
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    if(!ER_Query_AddFieldExpression(clER_PartsData.m_strField_Aggregate , strAs, strDbField, strDbTable))
        return false;

    // Save aggregate field for 'group by' and 'order by'
    if(!strAliases_GroupBy.isEmpty())
    {
        strAliases_GroupBy += ",";
        strAliases_OrderBy += ",";
        strAliases_GroupBy_As += ",";
        strAliases_OrderBy_As += ",";
    }
    strAliases_GroupBy += strDbField;
    strAliases_OrderBy += strDbField + " ASC";
    strAliases_GroupBy_As += strAs;
    strAliases_OrderBy_As += strAs + " ASC";

    // Add total parts field
    if(!ER_Query_AddPartCount("stats_nbparts", "Parts", true, clER_PartsData, "", true))
        return false;

    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);

    // Add site_no to make sure each line is unique (in case different sites for same splitlot have same part counts)
    strFieldSpec = "Field=SiteNo|" + m_strTablePrefix;
    strFieldSpec += "parts_stats_samples.site_no";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
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
    Query_BuildSqlString(strSubQuery, false);

    // Prepare the subquery to be used for a jointure
    strSubQuery = "(\n" + strSubQuery;
    strSubQuery += ") TPARTS\n";

    // Add to list of jointures
    strlJoinQueries.append(strSubQuery);

    /////////////////// OTHER PARTS OF THE JOIN: SERIES ///////////////////////////////////////
    // Add fields for data series
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        Query_Empty();

        strSerieName = pSerieDef->m_strSerieName;
        Query_NormalizeAlias(strSerieName);
        strBinList = pSerieDef->m_strBinnings.toLower();

        // Add Graph split fields?
        for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
        {
            strAs = *it;
            Query_NormalizeAlias(strAs);

            if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
                return false;
        }

        // Add Layer split fields?
        for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
        {
            strAs = *it;
            Query_NormalizeAlias(strAs);

            if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
                return false;
        }

        // Add aggregate field
        strAs = clER_PartsData.m_strField_Aggregate;
        Query_NormalizeAlias(strAs);
        if(!ER_Query_AddFieldExpression(clER_PartsData.m_strField_Aggregate , strAs, strDbField, strDbTable))
            return false;

        // Add total parts field
        if(!ER_Query_AddPartCount("stats_nbmatching", strSerieName, true, clER_PartsData, strBinList, true))
            return false;

        // Add splitlot ID to make sure each line is unique
        strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
        strFieldSpec += "splitlot.splitlot_id";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);

        // Add site_no to make sure each line is unique (in case different sites for same splitlot have same part counts)
        strFieldSpec = "Field=SiteNo|" + m_strTablePrefix;
        if(clER_PartsData.m_bSoftBin)
        {
            strFieldSpec += "sbin_stats_samples.site_no";
            strDbField = m_strTablePrefix + "sbin_stats_samples.site_no";
        }
        else
        {
            strFieldSpec += "hbin_stats_samples.site_no";
            strDbField = m_strTablePrefix + "hbin_stats_samples.site_no";
        }
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
        Query_BuildSqlString(strSubQuery, false);

        // Prepare the subquery to be used for a jointure
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strSubQuery += "GROUP BY " + strAliases_GroupBy_As;
            strSubQuery += ",SplitlotId,SiteNo\n";
        }
        else
        {
            strSubQuery += "GROUP BY " + strAliases_GroupBy;
            strSubQuery += "," + m_strTablePrefix;
            strSubQuery += "splitlot.splitlot_id," + strDbField;
            strSubQuery += "\n";
        }
        strSubQuery = "(\n" + strSubQuery;
        strSubQuery += ") ";
        strSubQuery += strSerieName + "\n";
        strSubQuery += "ON TPARTS.SplitlotId=";
        strSubQuery += strSerieName + ".SplitlotId AND TPARTS.SiteNo=";
        strSubQuery += strSerieName + ".SiteNo\n";

        // Add to list of jointures
        strlJoinQueries.append(strSubQuery);

//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    // Build the JOIN query
    strUnionQuery1 = "SELECT\n";
    // Add Graph split fields?
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        strUnionQuery1 += "TPARTS." + strAs;
        strUnionQuery1 += " AS " + strAs;
        strUnionQuery1 += ",\n";
    }

    // Add Layer split fields?
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        strUnionQuery1 += "TPARTS." + strAs;
        strUnionQuery1 += " AS " + strAs;
        strUnionQuery1 += ",\n";
    }

    // Add aggregate field
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    strUnionQuery1 += "TPARTS." + strAs;
    strUnionQuery1 += " AS " + strAs;
    strUnionQuery1 += ",\n";

    // Add parts field
    strUnionQuery1 += "TPARTS.Parts AS Parts,\n";

    // Add series
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        strSerieName = pSerieDef->m_strSerieName;
        Query_NormalizeAlias(strSerieName);
        strUnionQuery1 += strSerieName + "." + strSerieName;
        strUnionQuery1 += " AS " + strSerieName;
        strUnionQuery1 += ",\n";

//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    // Add SplitlotId and SiteNo
    strUnionQuery1 += "TPARTS.SplitlotId AS SplitlotId,\n";
    strUnionQuery1 += "TPARTS.SiteNo AS SiteNo\n";

    // Add jointures
    strUnionQuery1 += "FROM\n";
    it = strlJoinQueries.begin();
    strUnionQuery1 += (*it);
    it++;
    while(it != strlJoinQueries.end())
    {
        strUnionQuery1 += "LEFT OUTER JOIN\n";
        strUnionQuery1 += (*it);
        it++;
    }

    /////////////////// SECOND PART OF THE UNION: SUMMARY ///////////////////////////////////////

    // Create a LEFT JOIN between parts and binning data
    /////////////////// FIRST PART OF THE JOIN: PARTS ///////////////////////////////////////
    Query_Empty();
    strlJoinQueries.clear();

    // Add Graph split fields?
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;
    }

    // Add Layer split fields?
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;
    }

    // Add aggregate field
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    if(!ER_Query_AddFieldExpression(clER_PartsData.m_strField_Aggregate , strAs, strDbField, strDbTable))
        return false;

    // Add total parts field
    if(!ER_Query_AddPartCount("stats_nbparts", "Parts", false, clER_PartsData, "", true))
        return false;

    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);

    // Add site_no to make sure each line is unique (in case different sites for same splitlot have same part counts)
    strFieldSpec = "Field=SiteNo|" + m_strTablePrefix;
    strFieldSpec += "parts_stats_summary.site_no";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
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
    Query_BuildSqlString(strSubQuery, false);

    // Prepare the subquery to be used for a jointure
    strSubQuery = "(\n" + strSubQuery;
    strSubQuery += ") TPARTS\n";

    // Add to list of jointures
    strlJoinQueries.append(strSubQuery);

    /////////////////// OTHER PARTS OF THE JOIN: SERIES ///////////////////////////////////////
    // Add fields for data series
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        Query_Empty();

        strSerieName = pSerieDef->m_strSerieName;
        Query_NormalizeAlias(strSerieName);
        strBinList = pSerieDef->m_strBinnings.toLower();

        // Add Graph split fields?
        for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
        {
            strAs = *it;
            Query_NormalizeAlias(strAs);

            if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
                return false;
        }

        // Add Layer split fields?
        for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
        {
            strAs = *it;
            Query_NormalizeAlias(strAs);

            if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
                return false;
        }

        // Add aggregate field
        strAs = clER_PartsData.m_strField_Aggregate;
        Query_NormalizeAlias(strAs);
        if(!ER_Query_AddFieldExpression(clER_PartsData.m_strField_Aggregate , strAs, strDbField, strDbTable))
            return false;

        // Add total parts field
        if(!ER_Query_AddPartCount("stats_nbmatching", strSerieName, false, clER_PartsData, strBinList, true))
            return false;

        // Add splitlot ID to make sure each line is unique
        strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
        strFieldSpec += "splitlot.splitlot_id";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);

        // Add site_no to make sure each line is unique (in case different sites for same splitlot have same part counts)
        strFieldSpec = "Field=SiteNo|" + m_strTablePrefix;
        if(clER_PartsData.m_bSoftBin)
        {
            strFieldSpec += "sbin_stats_summary.site_no";
            strDbField = m_strTablePrefix + "sbin_stats_summary.site_no";
        }
        else
        {
            strFieldSpec += "hbin_stats_summary.site_no";
            strDbField = m_strTablePrefix + "hbin_stats_summary.site_no";
        }
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
        Query_BuildSqlString(strSubQuery, false);

        // Prepare the subquery to be used for a jointure
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strSubQuery += "GROUP BY " + strAliases_GroupBy_As;
            strSubQuery += ",SplitlotId,SiteNo\n";
        }
        else
        {
            strSubQuery += "GROUP BY " + strAliases_GroupBy;
            strSubQuery += "," + m_strTablePrefix;
            strSubQuery += "splitlot.splitlot_id," + strDbField;
            strSubQuery += "\n";
        }
        strSubQuery = "(\n" + strSubQuery;
        strSubQuery += ") ";
        strSubQuery += strSerieName + "\n";
        strSubQuery += "ON TPARTS.SplitlotId=";
        strSubQuery += strSerieName + ".SplitlotId AND TPARTS.SiteNo=";
        strSubQuery += strSerieName + ".SiteNo\n";

        // Add to list of jointures
        strlJoinQueries.append(strSubQuery);

//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    // Build the JOIN query
    strUnionQuery2 = "SELECT\n";
    // Add Graph split fields?
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        strUnionQuery2 += "TPARTS." + strAs;
        strUnionQuery2 += " AS " + strAs;
        strUnionQuery2 += ",\n";
    }

    // Add Layer split fields?
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        strUnionQuery2 += "TPARTS." + strAs;
        strUnionQuery2 += " AS " + strAs;
        strUnionQuery2 += ",\n";
    }

    // Add aggregate field
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    strUnionQuery2 += "TPARTS." + strAs;
    strUnionQuery2 += " AS " + strAs;
    strUnionQuery2 += ",\n";

    // Add parts field
    strUnionQuery2 += "TPARTS.Parts AS Parts,\n";

    // Add series
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        strSerieName = pSerieDef->m_strSerieName;
        Query_NormalizeAlias(strSerieName);
        strUnionQuery2 += strSerieName + "." + strSerieName;
        strUnionQuery2 += " AS " + strSerieName;
        strUnionQuery2 += ",\n";

//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    // Add SplitlotId and SiteNo
    strUnionQuery2 += "TPARTS.SplitlotId AS SplitlotId,\n";
    strUnionQuery2 += "TPARTS.SiteNo AS SiteNo\n";

    // Add jointures
    strUnionQuery2 += "FROM\n";
    it = strlJoinQueries.begin();
    strUnionQuery2 += (*it);
    it++;
    while(it != strlJoinQueries.end())
    {
        strUnionQuery2 += "LEFT OUTER JOIN\n";
        strUnionQuery2 += (*it);
        it++;
    }

    // Build the union query
    strSubQuery = "(\n";
    strSubQuery += strUnionQuery1;
    strSubQuery += ")\nUNION\n(\n";
    strSubQuery += strUnionQuery2;
    strSubQuery += ")\n";

    // Construct FINAL query... at least
    strQuery = "SELECT\n";
    // Add Graph split fields?
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);
        strQuery += strAs + ",\n";
    }

    // Add Layer split fields?
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);
        strQuery += strAs + ",\n";
    }

    // Add aggregate field
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    strQuery += strAs + ",\n";

    // Add parts field
    strQuery += "SUM(Parts) AS Parts";

    // Add series
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        strSerieName = pSerieDef->m_strSerieName;
        Query_NormalizeAlias(strSerieName);
        strQuery += ",\nSUM(case when ";
        strQuery += strSerieName + " IS NULL then 0 else ";
        strQuery += strSerieName + " end) AS ";
        strQuery += strSerieName;

//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    strQuery += "\nFROM\n(";
    strQuery += strSubQuery;
    if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        strQuery += ") AS T1\nGROUP BY ";
    else
        strQuery += ")\nGROUP BY ";
    strQuery += strAliases_GroupBy_As + "\nORDER BY ";
    strQuery += strAliases_OrderBy_As;

    return true;
}
#else
///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts (binning)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Prod_ConstructPartQuery_Binning(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData, QString & strQuery)
{
    QString							strAs, strCondition, strFieldSpec, strDbField, strDbTable;
    QString							strSerieName, strBinList;
    QString							strAliases_GroupBy, strAliases_OrderBy;
    QString							strAliases_GroupBy_As, strAliases_OrderBy_As;
    QStringList::iterator			it, itSerie;
    QString							strUnionQuery1, strUnionQuery2;
    GexDbPlugin_ER_Parts_SerieDef	*pSerieDef;
    unsigned int					uiSerie=0;
    QString							strSerieTableAlias, strOption;


    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <fields>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields

    // Create a union between samples and summary data
    /////////////////// FIRST PART OF THE UNION: SAMPLES ///////////////////////////////////////
    /////////////////// Create a LEFT JOIN between parts and binning data
    Query_Empty();

    // Add Graph split fields?
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;

        // Save list of graph split fields for 'group by' and 'order by'
        if(!strAliases_GroupBy.isEmpty())
        {
            strAliases_GroupBy += ",";
            strAliases_OrderBy += ",";
            strAliases_GroupBy_As += ",";
            strAliases_OrderBy_As += ",";
        }
        strAliases_GroupBy += strDbField;
        strAliases_OrderBy += strDbField + " ASC";
        strAliases_GroupBy_As += strAs;
        strAliases_OrderBy_As += strAs + " ASC";
    }

    // Add Layer split fields?
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;

        // Save list of layer split fields for 'group by' and 'order by'
        if(!strAliases_GroupBy.isEmpty())
        {
            strAliases_GroupBy += ",";
            strAliases_OrderBy += ",";
            strAliases_GroupBy_As += ",";
            strAliases_OrderBy_As += ",";
        }
        strAliases_GroupBy += strDbField;
        strAliases_OrderBy += strDbField + " ASC";
        strAliases_GroupBy_As += strAs;
        strAliases_OrderBy_As += strAs + " ASC";
    }

    // Add aggregate field
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    if(!ER_Query_AddFieldExpression(clER_PartsData.m_strField_Aggregate , strAs, strDbField, strDbTable))
        return false;

    // Save aggregate field for 'group by' and 'order by'
    if(!strAliases_GroupBy.isEmpty())
    {
        strAliases_GroupBy += ",";
        strAliases_OrderBy += ",";
        strAliases_GroupBy_As += ",";
        strAliases_OrderBy_As += ",";
    }
    strAliases_GroupBy += strDbField;
    strAliases_OrderBy += strDbField + " ASC";
    strAliases_GroupBy_As += strAs;
    strAliases_OrderBy_As += strAs + " ASC";

    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = m_strTablePrefix + "splitlot.prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Add filter to extract only samples
    if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
    {
        strDbField = m_strTablePrefix+"splitlot.splitlot_flags";
        strCondition = strDbField + "|FieldExpression_Numeric|4";
        strCondition += "|(" + strDbField + " & 4)";
        m_strlQuery_ValueConditions.append(strCondition);
    }

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    QString strSelect, strFrom, strWhere, strGroup, strOrder;
    Query_BuildSqlString_UsingJoins(strSelect, strFrom, strWhere, strGroup, strOrder, false);

    // Add site_no
    strDbField = "parts.site_no";
    strAs = "SiteNo";
    strSelect += "," + strDbField + " AS " + strAs + "\n";

#if 0
    // Save site_no for 'group by' and 'order by'
    if(!strAliases_GroupBy.isEmpty())
    {
        strAliases_GroupBy += ",";
        strAliases_OrderBy += ",";
        strAliases_GroupBy_As += ",";
        strAliases_OrderBy_As += ",";
    }
    strAliases_GroupBy += strDbField;
    strAliases_OrderBy += strDbField + " ASC";
    strAliases_GroupBy_As += strAs;
    strAliases_OrderBy_As += strAs + " ASC";
#endif

    // Add total parts count
    strDbField = "parts.nb_parts";
    strAs = "Parts";
    strSelect += "," + strDbField + " AS " + strAs + "\n";
    // Add total good count
    strDbField = "parts.nb_parts_good";
    strAs = "Parts_Good";
    strSelect += "," + strDbField + " AS " + strAs + "\n";

    // Add serie parts count: one per serie
    uiSerie=0;
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        strAs = pSerieDef->m_strSerieName;
        Query_NormalizeAlias(strAs);
        strBinList = pSerieDef->m_strBinnings.toLower();

        // Create list of binnings to use
        if(strBinList.indexOf("pass") != -1)
            strDbField = "parts.nb_parts_good";
        else if(strBinList.indexOf("fail") != -1)
            strDbField = "(parts.nb_parts-parts.nb_parts_good)";
        else if(strBinList.indexOf("all") != -1)
            strDbField = "parts.nb_parts";
        else
        {
            strSerieTableAlias = "serie_" + QString::number(uiSerie++);
            strDbField = strSerieTableAlias + ".nb_parts";
        }
        strSelect += "," + strDbField + " AS " + strAs + "\n";

//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    strUnionQuery1 = strSelect + strFrom;

    // First OUTER JOIN = Parts
    strCondition = m_strTablePrefix+"parts_stats_samples.site_no";
    Query_NormalizeToken(strCondition, strDbField, strDbTable);
    strUnionQuery1 += "LEFT OUTER JOIN\n";
    strUnionQuery1 +=  strDbTable+" parts\n";
    strUnionQuery1 += "ON " + m_strTablePrefix+"splitlot.splitlot_id=parts.splitlot_id AND parts.site_no >= 0\n";

    // Next OUTER JOINS: one per serie
    uiSerie=0;
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        // Check if we need to add the jointure
        strBinList = pSerieDef->m_strBinnings.toLower();
        if((strBinList.indexOf("pass") == -1) && (strBinList.indexOf("fail") == -1) && (strBinList.indexOf("all") == -1))
        {
            strSerieTableAlias = "serie_" + QString::number(uiSerie++);
            if(clER_PartsData.m_bSoftBin)
                strCondition = m_strTablePrefix+"sbin_stats_samples.site_no";
            else
                strCondition = m_strTablePrefix+"hbin_stats_samples.site_no";
            Query_NormalizeToken(strCondition, strDbField, strDbTable);
            strUnionQuery1 += "LEFT OUTER JOIN\n";
            strUnionQuery1 +=  strDbTable + " " + strSerieTableAlias + "\n";
            strUnionQuery1 += "ON " + m_strTablePrefix+"splitlot.splitlot_id=" + strSerieTableAlias + ".splitlot_id";
            strUnionQuery1 += " AND parts.site_no=" + strSerieTableAlias + ".site_no";
            if(clER_PartsData.m_bSoftBin)
                strDbField = strSerieTableAlias + ".sbin_no";
            else
                strDbField = strSerieTableAlias + ".hbin_no";
            Query_BuildSqlString_NumericFilter(strDbField, strBinList, strCondition);
            strUnionQuery1 += " AND " + strCondition + "\n";
        }

//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    strUnionQuery1 += strWhere + strGroup + strOrder;

    /////////////////// SECOND PART OF THE UNION: SUMMARY ///////////////////////////////////////
    /////////////////// Create a LEFT JOIN between parts and binning data
    Query_Empty();

    // Add Graph split fields?
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;
    }

    // Add Layer split fields?
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;
    }

    // Add aggregate field
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    if(!ER_Query_AddFieldExpression(clER_PartsData.m_strField_Aggregate , strAs, strDbField, strDbTable))
        return false;

    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = m_strTablePrefix + "splitlot.prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Add filter to extract only summary
    if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
    {
        strDbField = m_strTablePrefix+"splitlot.splitlot_flags";
        strCondition = strDbField + "|FieldExpression_Numeric|0";
        strCondition += "|(" + strDbField + " & 4)";
        m_strlQuery_ValueConditions.append(strCondition);
    }

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString_UsingJoins(strSelect, strFrom, strWhere, strGroup, strOrder, false);

    // Add site_no
    strDbField = "parts.site_no";
    strAs = "SiteNo";
    strSelect += "," + strDbField + " AS " + strAs + "\n";

#if 0
    // Save site_no for 'group by' and 'order by'
    if(!strAliases_GroupBy.isEmpty())
    {
        strAliases_GroupBy += ",";
        strAliases_OrderBy += ",";
        strAliases_GroupBy_As += ",";
        strAliases_OrderBy_As += ",";
    }
    strAliases_GroupBy += strDbField;
    strAliases_OrderBy += strDbField + " ASC";
    strAliases_GroupBy_As += strAs;
    strAliases_OrderBy_As += strAs + " ASC";
#endif

    // Add total parts count
    strDbField = "parts.nb_parts";
    strAs = "Parts";
    strSelect += "," + strDbField + " AS " + strAs + "\n";
    // Add total good count
    strDbField = "parts.nb_good";
    strAs = "Parts_Good";
    strSelect += "," + strDbField + " AS " + strAs + "\n";

    // Add serie parts count: one per serie
    uiSerie=0;
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        strAs = pSerieDef->m_strSerieName;
        Query_NormalizeAlias(strAs);
        strBinList = pSerieDef->m_strBinnings.toLower();

        // Create list of binnings to use
        if(strBinList.indexOf("pass") != -1)
            strDbField = "parts.nb_good";
        else if(strBinList.indexOf("fail") != -1)
            strDbField = "(parts.nb_parts-parts.nb_good)";
        else if(strBinList.indexOf("all") != -1)
            strDbField = "parts.nb_parts";
        else
        {
            strSerieTableAlias = "serie_" + QString::number(uiSerie++);
            strDbField = strSerieTableAlias + ".bin_count";
        }
        strSelect += "," + strDbField + " AS " + strAs + "\n";

//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    strUnionQuery2 = strSelect + strFrom;

    // First OUTER JOIN = Parts
    strCondition = m_strTablePrefix+"parts_stats_summary.site_no";
    Query_NormalizeToken(strCondition, strDbField, strDbTable);
    strUnionQuery2 += "LEFT OUTER JOIN\n";
    strUnionQuery2 +=  strDbTable+" parts\n";
    strUnionQuery2 += "ON " + m_strTablePrefix+"splitlot.splitlot_id=parts.splitlot_id AND parts.site_no = -1\n";

    // Next OUTER JOINS: one per serie
    uiSerie=0;
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        // Check if we need to add the jointure
        strBinList = pSerieDef->m_strBinnings.toLower();
        if((strBinList.indexOf("pass") == -1) && (strBinList.indexOf("fail") == -1) && (strBinList.indexOf("all") == -1))
        {
            strSerieTableAlias = "serie_" + QString::number(uiSerie++);
            if(clER_PartsData.m_bSoftBin)
                strCondition = m_strTablePrefix+"sbin_stats_summary.site_no";
            else
                strCondition = m_strTablePrefix+"hbin_stats_summary.site_no";
            Query_NormalizeToken(strCondition, strDbField, strDbTable);
            strUnionQuery2 += "LEFT OUTER JOIN\n";
            strUnionQuery2 +=  strDbTable + " " + strSerieTableAlias + "\n";
            strUnionQuery2 += "ON " + m_strTablePrefix+"splitlot.splitlot_id=" + strSerieTableAlias + ".splitlot_id";
            strUnionQuery2 += " AND parts.site_no=" + strSerieTableAlias + ".site_no";
            if(clER_PartsData.m_bSoftBin)
                strDbField = strSerieTableAlias + ".sbin_no";
            else
                strDbField = strSerieTableAlias + ".hbin_no";
            Query_BuildSqlString_NumericFilter(strDbField, strBinList, strCondition);
            strUnionQuery2 += " AND " + strCondition + "\n";
        }

//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    strUnionQuery2 += strWhere + strGroup + strOrder;

    // Build the union query
    QString strSubQuery = "(\n";
    strSubQuery += strUnionQuery1;
    strSubQuery += ")\nUNION ALL\n(\n";
    strSubQuery += strUnionQuery2;
    strSubQuery += ")\n";

    // Construct FINAL query... at least
    strQuery = "SELECT\n";
    // Add Graph split fields?
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);
        strQuery += strAs + ",\n";
    }

    // Add Layer split fields?
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);
        strQuery += strAs + ",\n";
    }

    // Add aggregate field
    strAs = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAs);
    strQuery += strAs + ",\n";

    // Add parts field
    strQuery += "SUM(Parts) AS Parts";
    // Add good parts field
    strQuery += ",\nSUM(Parts_Good) AS Parts_Good";

    // Add series
//	pSerieDef = clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, clER_PartsData.m_plistSerieDefs)
    {
        strSerieName = pSerieDef->m_strSerieName;
        Query_NormalizeAlias(strSerieName);
        strQuery += ",\nSUM(" + strSerieName + ") AS ";
        strQuery += strSerieName;

//		pSerieDef = clER_PartsData.m_plistSerieDefs.next();
    }

    strQuery += "\nFROM\n(";
    strQuery += strSubQuery;
    if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        strQuery += ") AS T1\nGROUP BY ";
    else
        strQuery += ")\nGROUP BY ";
    strQuery += strAliases_GroupBy_As + "\nORDER BY ";
    strQuery += strAliases_OrderBy_As;

    return true;
}
#endif

#if 0
///////////////////////////////////////////////////////////
// Query: Construct query to retrieve bincount data for ER Production reports
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Prod_ConstructSplitCountQuery(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData, QString & strQuery)
{
    QString							strAs, strBinList, strCondition, strSubQuery, strFieldSpec, strDbField, strDbTable;
    QString							strSubQuery_1, strSubQuery_2;
    QString							strGraphSplitAliases, strLayerSplitAliases;
    QString							strAggregateAlias;
    QStringList::iterator			it;

    // Clear query string
    strQuery = "";

    ///////////////////// JOIN BETWEEN Splitlot and prod_alarm

    ///////////////////// First part of the JOIN: Splitlot
    Query_Empty();

    // Add Graph split fields?
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;

        // Save list of graph split aliases
        if(!strGraphSplitAliases.isEmpty())
            strGraphSplitAliases += ",";
        strGraphSplitAliases += strAs;
    }

    // Add Layer split fields?
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
    {
        strAs = *it;
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(*it, strAs, strDbField, strDbTable))
            return false;

        // Save list of layer split aliases
        if(!strLayerSplitAliases.isEmpty())
            strLayerSplitAliases += ",";
        strLayerSplitAliases += strAs;
    }

    // Add aggregate field
    strAggregateAlias = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAggregateAlias);

    if(!ER_Query_AddFieldExpression(clER_PartsData.m_strField_Aggregate , strAggregateAlias, strDbField, strDbTable))
        return false;

    // Add splitlot_id, wafer_id, lot_id
    strFieldSpec = "Field=C_SplitlotID|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.splitlot_id");
    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
    {
        strFieldSpec = "Field=C_WaferID|" + m_strTablePrefix;
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.wafer_id");
    }
    strFieldSpec = "Field=C_LotID|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.lot_id");

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strSubQuery_1, false);

    ///////////////////// Second part of the JOIN: Prod_Alarm
    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
    {
        Query_Empty();

        // Add splitlot_id, nb_alarms
        strFieldSpec = "Field=C_SplitlotID|" + m_strTablePrefix;
        m_strlQuery_Fields.append(strFieldSpec + "prod_alarm.splitlot_id");
        strFieldSpec = "Function=C_Alarms|" + m_strTablePrefix;
        m_strlQuery_Fields.append(strFieldSpec + "prod_alarm.alarm_type|COUNT");

        // Add alarm type condition
        QString strAlarmType = clER_PartsData.m_strMaverickWafer_AlarmType.toLower();
        if(strAlarmType.indexOf("critical") >= 0)
        {
            strCondition = m_strTablePrefix + "prod_alarm.alarm_type|String|Critical";
            m_strlQuery_ValueConditions.append(strCondition);
        }
        else if(strAlarmType.indexOf("standard") >= 0)
        {
            strCondition = m_strTablePrefix + "prod_alarm.alarm_type|String|Standard";
            m_strlQuery_ValueConditions.append(strCondition);
        }

        // Add group by condition
        m_strlQuery_GroupFields.append(m_strTablePrefix + "prod_alarm.splitlot_id");

        // Construct sub-query from table and conditions
        Query_BuildSqlString(strSubQuery_2, false);
    }

    //////////// CONSTRUCT FINAL QUERY
    strQuery = "SELECT\n";
    if(!strGraphSplitAliases.isEmpty())
    {
        strQuery += strGraphSplitAliases;
        strQuery += ",\n";
    }
    if(!strLayerSplitAliases.isEmpty())
    {
        strQuery += strLayerSplitAliases;
        strQuery += ",\n";
    }
    strQuery += strAggregateAlias;
    strQuery += ",\n";
    strQuery += "SUM(NbSplitlots) AS NbSplitlots,\n";
    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
        strQuery += "SUM(NbWafers) AS NbWafers,\n";
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        strQuery += "COUNT(DISTINCT C_LotID) AS NbLots\n";
    else
    {
        strQuery += "COUNT(DISTINCT C_LotID) AS NbLots,\n";
        strQuery += "SUM(NbMaverickWafers) AS NbMaverickWafers,\n";
        strQuery += "SUM(CASE WHEN NbMaverickWafers >= ";
        strQuery += QString::number(clER_PartsData.m_nMaverickLot_WaferCount);
        strQuery += " THEN 1 ELSE 0 END) AS NbMaverickLots\n";
    }
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += "SELECT\n";
    if(!strGraphSplitAliases.isEmpty())
    {
        strQuery += strGraphSplitAliases;
        strQuery += ",\n";
    }
    if(!strLayerSplitAliases.isEmpty())
    {
        strQuery += strLayerSplitAliases;
        strQuery += ",\n";
    }
    strQuery += strAggregateAlias;
    strQuery += ",\n";
    strQuery += "COUNT(C_SplitlotID) AS NbSplitlots,\n";
    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
        strQuery += "COUNT(C_WaferID) AS NbWafers,\n";
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        strQuery += "C_LotID\n";
    else
    {
        strQuery += "C_LotID,\n";
        strQuery += "SUM(CASE WHEN C_Alarms >= ";
        strQuery += QString::number(clER_PartsData.m_nMaverickWafer_AlarmCount);
        strQuery += " THEN 1 ELSE 0 END) AS NbMaverickWafers\n";
    }
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += "SELECT\n";
    if(!strGraphSplitAliases.isEmpty())
    {
        strQuery += "T0." + strGraphSplitAliases;
        strQuery += ",\n";
    }
    if(!strLayerSplitAliases.isEmpty())
    {
        strQuery += "T0." + strLayerSplitAliases;
        strQuery += ",\n";
    }
    strQuery += "T0." + strAggregateAlias;
    strQuery += ",\n";
    strQuery += "T0.C_SplitlotID,\n";
    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
        strQuery += "T0.C_WaferID,\n";
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        strQuery += "T0.C_LotID\n";
    else
    {
        strQuery += "T0.C_LotID,\n";
        strQuery += "T1.C_Alarms\n";
    }
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += strSubQuery_1;
    strQuery += ") T0\n";
    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
    {
        strQuery += "LEFT OUTER JOIN\n";
        strQuery += "(\n";
        strQuery += strSubQuery_2;
        strQuery += ") T1\n";
        strQuery += "ON T0.C_SplitlotID=T1.C_SplitlotID\n";
    }
    strQuery += ") TAB1\n";
    strQuery += "GROUP BY ";
    if(!strGraphSplitAliases.isEmpty())
    {
        strQuery += strGraphSplitAliases;
        strQuery += ",\n";
    }
    if(!strLayerSplitAliases.isEmpty())
    {
        strQuery += strLayerSplitAliases;
        strQuery += ",\n";
    }
    strQuery += strAggregateAlias;
    strQuery += ",C_LotID\n";
    strQuery += ") TAB2\n";
    strQuery += "GROUP BY ";
    if(!strGraphSplitAliases.isEmpty())
    {
        strQuery += strGraphSplitAliases;
        strQuery += ",\n";
    }
    if(!strLayerSplitAliases.isEmpty())
    {
        strQuery += strLayerSplitAliases;
        strQuery += ",\n";
    }
    strQuery += strAggregateAlias;
    strQuery += "\n";

    return true;
}
#else
///////////////////////////////////////////////////////////
// Query: Construct query to retrieve bincount data for ER Production reports
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Prod_ConstructSplitCountQuery(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData, QString & strQuery)
{
    QString						strAs, strBinList, strCondition, strSubQuery, strFieldSpec, strDbField, strDbTable, strField;
    QString						strTM1, strTM2, strTSPLITS, strTMAV;
    QStringList					strlGraphSplitFields, strlLayerSplitFields;
    QStringList					strlGraphSplitAliases, strlLayerSplitAliases;
    QStringList					strlGraphSplitAliases_T, strlLayerSplitAliases_T;
    QString						strAggregateField, strAggregateAlias;
    int							nIndex;

    // Clear query string
    strQuery = "";

    //////////////////////////////////////////////////////////
    // General structure of the query is:
    //
    //	SELECT	TSPLITS.F1, TSPLITS.F2,...
    //			TSPLITS.NbSplitlots, TSPLITS.NbWafers, TSPLITS.NbLots
    //			case when TMAV.MaverickLots is null then 0 else TMAV.MaverickLots end as MaverickLots,
    //			case when TMAV.MaverickWafers is null then 0 else TMAV.MaverickWafers end as MaverickWafers
    //	FROM
    //	(
    //		SELECT	xx_splitlot.F1 AS F1, wt_splitlot.F2 AS F2,...
    //				count(*) AS NbSplitlots,
    // MySQL		count(distinct lot_id,wafer_id) AS NbWafers,
    // Oracle       count(distinct concat(lot_id, wafer_id) AS NbWafers,
    //				count(distinct lot_id) AS NbLots
    //		FROM
    //				xx_splitlot
    //		WHERE
    //				(xx_splitlot.valid_splitlot = 'Y')
    //		GROUP BY F1, F2, ...
    //	) TSPLITS
    //	LEFT OUTER JOIN
    //	(
    //		SELECT	TM2.F1 as F1, TM2.F2 as F2, ...
    //				SUM(TM2.MaverickWafers) AS MaverickWafers,
    //				SUM(CASE WHEN TM2.MaverickWafers >= 1 THEN 1 ELSE 0 END) AS MaverickLots
    //		FROM
    //		(
    //			SELECT DISTINCT	TM1.F1 as F1, TM1.F2 as F2 ...
    //							TM1.LotID as LotID,
    //							count(distinct TM1.WaferID) as MaverickWafers
    //			FROM
    //			(
    //				SELECT	TSL.F1 as F1, TSL.F2 as F2, ...
    //						TSL.lot_id AS LotID,
    //						TSL.wafer_id as WaferID,
    //						count(TPA.splitlot_id) as NbAlarms
    //				FROM
    //						xx_splitlot TSL
    //				INNER JOIN
    //						xx_prod_alarm TPA
    //				ON TSL.splitlot_id=TPA.splitlot_id
    //				WHERE TSL.valid_splitlot='Y' and TPA.alarm_type='Critical'
    //				GROUP BY TSL.splitlot_id
    //			) TM1
    //			where TM1.NbAlarms >= 1
    //			group by TM1.F1, TM1.F2, ..., TM1.LotID
    //		) TM2
    //		group by TM2.F1, TM2.F2, ...
    //	) TMAV
    //	ON TMAV.F1=TSPLITS.F1, TMAV.F2=TSPLITS.F2, ...
    //
    //////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////
    // Query for TSPLITS
    //////////////////////////////////////////////////////////
    Query_Empty();

    // Add Graph split fields?
    for(nIndex=0; nIndex<clER_PartsData.m_strlFields_GraphSplit.size(); nIndex++)
    {
        strField = strAs = clER_PartsData.m_strlFields_GraphSplit.at(nIndex);
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(strField, strAs, strDbField, strDbTable))
            return false;

        // Save list of graph split aliases
        strlGraphSplitFields.append(strDbField);
        strlGraphSplitAliases.append(strAs);
        strlGraphSplitAliases_T.append(QString("TABLE." + strAs));
    }

    // Add Layer split fields?
    for(nIndex=0; nIndex<clER_PartsData.m_strlFields_LayerSplit.size(); nIndex++)
    {
        strField = strAs = clER_PartsData.m_strlFields_LayerSplit.at(nIndex);
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(strField, strAs, strDbField, strDbTable))
            return false;

        // Save list of layer split aliases
        strlLayerSplitFields.append(strDbField);
        strlLayerSplitAliases.append(strAs);
        strlLayerSplitAliases_T.append(QString("TABLE." + strAs));
    }

    // Add aggregate field
    strAggregateAlias = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAggregateAlias);

    if(!ER_Query_AddFieldExpression(clER_PartsData.m_strField_Aggregate , strAggregateAlias, strDbField, strDbTable))
        return false;
    strAggregateField = strDbField;

    // Add count(splitlot_id), count(lot_id, wafer_id), count(lot_id)
    strFieldSpec = "Function=NbSplitlots|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.splitlot_id|count");
    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
    {
        strFieldSpec = "DistinctFunction=NbWafers|" + m_strTablePrefix + "splitlot.lot_id," + m_strTablePrefix;
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.wafer_id|count");
    }
    strFieldSpec = "DistinctFunction=NbLots|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.lot_id|count");

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = m_strTablePrefix + "splitlot.prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Add group by condition
    if(!strlGraphSplitFields.isEmpty())
        m_strlQuery_GroupFields += strlGraphSplitFields;
    if(!strlLayerSplitFields.isEmpty())
        m_strlQuery_GroupFields += strlLayerSplitFields;
    m_strlQuery_GroupFields.append(strAggregateField);

    // Construct sub-query from table and conditions
    Query_BuildSqlString_UsingJoins(strTSPLITS, false);

    //////////////////////////////////////////////////////////
    // Query for TM1
    //////////////////////////////////////////////////////////
    Query_Empty();

    // Add Graph split fields?
    for(nIndex=0; nIndex<clER_PartsData.m_strlFields_GraphSplit.size(); nIndex++)
    {
        strField = strAs = clER_PartsData.m_strlFields_GraphSplit.at(nIndex);
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(strField, strAs, strDbField, strDbTable))
            return false;
    }

    // Add Layer split fields?
    for(nIndex=0; nIndex<clER_PartsData.m_strlFields_LayerSplit.size(); nIndex++)
    {
        strField = strAs = clER_PartsData.m_strlFields_LayerSplit.at(nIndex);
        Query_NormalizeAlias(strAs);

        if(!ER_Query_AddFieldExpression(strField, strAs, strDbField, strDbTable))
            return false;
    }

    // Add aggregate field
    strAggregateAlias = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAggregateAlias);

    if(!ER_Query_AddFieldExpression(clER_PartsData.m_strField_Aggregate , strAggregateAlias, strDbField, strDbTable))
        return false;

    // Add lot_id, wafer_id, count(splitlot_id) as NbAlarms
    strFieldSpec = "Field=C_LotID|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.lot_id");
    if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST)
    {
        strFieldSpec = "Field=C_WaferID|" + m_strTablePrefix;
        m_strlQuery_Fields.append(strFieldSpec + "splitlot.wafer_id");
    }
    strFieldSpec = "Function=NbAlarms|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "splitlot.splitlot_id|count");

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = m_strTablePrefix + "splitlot.prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Add link with prod_alarm table
    strCondition = m_strTablePrefix +"prod_alarm.splitlot_id|";
    strCondition += m_strTablePrefix +"splitlot.splitlot_id";
    if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
        m_strlQuery_LinkConditions.append(strCondition);

    // Add filter on alarm type
    QString strAlarmType = clER_PartsData.m_strMaverickWafer_AlarmType.toLower();
    if(strAlarmType.indexOf("critical") >= 0)
    {
        strCondition = m_strTablePrefix + "prod_alarm.alarm_type|String|Critical";
        m_strlQuery_ValueConditions.append(strCondition);
    }
    else if(strAlarmType.indexOf("standard") >= 0)
    {
        strCondition = m_strTablePrefix + "prod_alarm.alarm_type|String|Standard";
        m_strlQuery_ValueConditions.append(strCondition);
    }

    // Add group by condition
    m_strlQuery_GroupFields.append(m_strTablePrefix + "splitlot.splitlot_id");

    // Construct sub-query from table and conditions
    Query_BuildSqlString_UsingJoins(strTM1, false);

    //////////////////////////////////////////////////////////
    // FINAL QUERY
    //////////////////////////////////////////////////////////
    strQuery = "SELECT\n";
    if(!strlGraphSplitAliases_T.isEmpty())
    {
        for(nIndex=0; nIndex<strlGraphSplitAliases_T.size(); nIndex++)
        {
            strSubQuery = strlGraphSplitAliases_T.at(nIndex);
            strSubQuery.replace("TABLE.", "TSPLITS.");
            strQuery += strSubQuery;
            strQuery += " AS ";
            strQuery += strlGraphSplitAliases.at(nIndex);
            strQuery += ",\n";
        }
    }
    if(!strlLayerSplitAliases_T.isEmpty())
    {
        for(nIndex=0; nIndex<strlLayerSplitAliases_T.size(); nIndex++)
        {
            strSubQuery = strlLayerSplitAliases_T.at(nIndex);
            strSubQuery.replace("TABLE.", "TSPLITS.");
            strQuery += strSubQuery;
            strQuery += " AS ";
            strQuery += strlLayerSplitAliases.at(nIndex);
            strQuery += ",\n";
        }
    }
    strQuery += "TSPLITS." + strAggregateAlias;
    strQuery += ",\n";
    strQuery += "TSPLITS.NbSplitlots,\n";
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        strQuery += "TSPLITS.NbLots\n";
    else
    {
        strQuery += "TSPLITS.NbWafers,\n";
        strQuery += "TSPLITS.NbLots,\n";
        strQuery += "case when TMAV.MaverickWafers is null then 0 else TMAV.MaverickWafers end AS MaverickWafers,\n";
        strQuery += "case when TMAV.MaverickLots is null then 0 else TMAV.MaverickLots end AS MaverickLots\n";
    }
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += strTSPLITS;
    strQuery += ") TSPLITS\n";
    strQuery += "LEFT OUTER JOIN\n";
    strQuery += "(\n";
    strQuery += "SELECT\n";
    if(!strlGraphSplitAliases_T.isEmpty())
    {
        for(nIndex=0; nIndex<strlGraphSplitAliases_T.size(); nIndex++)
        {
            strSubQuery = strlGraphSplitAliases_T.at(nIndex);
            strSubQuery.replace("TABLE.", "TM2.");
            strQuery += strSubQuery;
            strQuery += " AS ";
            strQuery += strlGraphSplitAliases.at(nIndex);
            strQuery += ",\n";
        }
    }
    if(!strlLayerSplitAliases_T.isEmpty())
    {
        for(nIndex=0; nIndex<strlLayerSplitAliases_T.size(); nIndex++)
        {
            strSubQuery = strlLayerSplitAliases_T.at(nIndex);
            strSubQuery.replace("TABLE.", "TM2.");
            strQuery += strSubQuery;
            strQuery += " AS ";
            strQuery += strlLayerSplitAliases.at(nIndex);
            strQuery += ",\n";
        }
    }
    strQuery += "TM2." + strAggregateAlias;
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        strQuery += "\n";
    else
    {
        strQuery += ",\n";
        strQuery += "SUM(TM2.MaverickWafers) AS MaverickWafers,\n";
        strQuery += "SUM(case when TM2.MaverickWafers >= " + QString::number(clER_PartsData.m_nMaverickLot_WaferCount) + " THEN 1 ELSE 0 END) AS MaverickLots\n";
    }
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += "SELECT DISTINCT\n";
    if(!strlGraphSplitAliases_T.isEmpty())
    {
        for(nIndex=0; nIndex<strlGraphSplitAliases_T.size(); nIndex++)
        {
            strSubQuery = strlGraphSplitAliases_T.at(nIndex);
            strSubQuery.replace("TABLE.", "TM1.");
            strQuery += strSubQuery;
            strQuery += " AS ";
            strQuery += strlGraphSplitAliases.at(nIndex);
            strQuery += ",\n";
        }
    }
    if(!strlLayerSplitAliases_T.isEmpty())
    {
        for(nIndex=0; nIndex<strlLayerSplitAliases_T.size(); nIndex++)
        {
            strSubQuery = strlLayerSplitAliases_T.at(nIndex);
            strSubQuery.replace("TABLE.", "TM1.");
            strQuery += strSubQuery;
            strQuery += " AS ";
            strQuery += strlLayerSplitAliases.at(nIndex);
            strQuery += ",\n";
        }
    }
    strQuery += "TM1." + strAggregateAlias;
    strQuery += ",\n";
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        strQuery += "TM1.C_LotID\n";
    else
    {
        strQuery += "TM1.C_LotID,\n";
        strQuery += "count(distinct TM1.C_WaferID) as MaverickWafers\n";
    }
    strQuery += "FROM\n(\n";
    strQuery += strTM1;
    strQuery += ") TM1\n";
    strQuery += "WHERE TM1.NbAlarms >= " + QString::number(clER_PartsData.m_nMaverickWafer_AlarmCount) + "\n";
    strQuery += "GROUP BY ";
    if(!strlGraphSplitAliases_T.isEmpty())
    {
        strSubQuery = strlGraphSplitAliases_T.join(",");
        strSubQuery.replace("TABLE.", "TM1.");
        strQuery += strSubQuery;
        strQuery += ",\n";
    }
    if(!strlLayerSplitAliases_T.isEmpty())
    {
        strSubQuery = strlLayerSplitAliases_T.join(",");
        strSubQuery.replace("TABLE.", "TM1.");
        strQuery += strSubQuery;
        strQuery += ",\n";
    }
    strQuery += "TM1." + strAggregateAlias;
    strQuery += ",TM1.C_LotID\n";
    strQuery += ") TM2\n";
    strQuery += "GROUP BY ";
    if(!strlGraphSplitAliases_T.isEmpty())
    {
        strSubQuery = strlGraphSplitAliases_T.join(",");
        strSubQuery.replace("TABLE.", "TM2.");
        strQuery += strSubQuery;
        strQuery += ",\n";
    }
    if(!strlLayerSplitAliases_T.isEmpty())
    {
        strSubQuery = strlLayerSplitAliases_T.join(",");
        strSubQuery.replace("TABLE.", "TM2.");
        strQuery += strSubQuery;
        strQuery += ",\n";
    }
    strQuery += "TM2." + strAggregateAlias + "\n";
    strQuery += ") TMAV\n";
    strQuery += "ON ";

    strSubQuery = "";
    // Add Graph split fields?
    for(nIndex=0; nIndex<clER_PartsData.m_strlFields_GraphSplit.size(); nIndex++)
    {
        strAs = clER_PartsData.m_strlFields_GraphSplit.at(nIndex);
        Query_NormalizeAlias(strAs);
        if(!strSubQuery.isEmpty())
            strSubQuery += " AND ";
        strSubQuery += "TMAV." + strAs + "=TSPLITS." + strAs;
    }
    // Add Layer split fields?
    for(nIndex=0; nIndex<clER_PartsData.m_strlFields_LayerSplit.size(); nIndex++)
    {
        strAs = clER_PartsData.m_strlFields_LayerSplit.at(nIndex);
        Query_NormalizeAlias(strAs);
        if(!strSubQuery.isEmpty())
            strSubQuery += " AND ";
        strSubQuery += "TMAV." + strAs + "=TSPLITS." + strAs;
    }
    // Add aggregate field
    strAggregateAlias = clER_PartsData.m_strField_Aggregate;
    Query_NormalizeAlias(strAggregateAlias);
    if(!strSubQuery.isEmpty())
        strSubQuery += " AND ";
    strSubQuery += "TMAV." + strAggregateAlias + "=TSPLITS." + strAggregateAlias;
    strQuery += strSubQuery + "\n";

    return true;
}
#endif

#if 0
///////////////////////////////////////////////////////////
// Query: Construct query to retrieve bincount data for ER Production reports
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Prod_ConstructBinCountQuery(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData, GexDbPlugin_ER_Parts_Graph *pGraph, GexDbPlugin_ER_Parts_Layer *pLayer, const QString & strAggregateLabel, QString & strQuery)
{
    QString					strCondition, strFieldSpec, strDbField, strDbTable;
    QStringList::iterator	it;
    QString					strSubQuery, strJoinQuery2, strJoinQuery1, strUnionQuery1, strUnionQuery2;
    int						nIndex;

    // Clear query string
    strQuery = "";

    // Create a union between samples and summary data
    /////////////////// FIRST PART OF THE UNION: SAMPLES ///////////////////////////////////////
    Query_Empty();

    // Add splitlot ID
    strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);

    // Add part count field
    if(!ER_Query_AddPartCount("stats_nbmatching", "BinCount", true, clER_PartsData, "all", false))
        return false;

    // Add site_no
    strFieldSpec = "Field=SiteNo|" + m_strTablePrefix;
    if(clER_PartsData.m_bSoftBin)
    {
        strFieldSpec += "sbin_stats_samples.site_no";
        strDbField = m_strTablePrefix + "sbin_stats_samples.site_no";
    }
    else
    {
        strFieldSpec += "hbin_stats_samples.site_no";
        strDbField = m_strTablePrefix + "hbin_stats_samples.site_no";
    }
    m_strlQuery_Fields.append(strFieldSpec);

    // Add bin_no
    strFieldSpec = "Field=BinNo|" + m_strTablePrefix;
    if(clER_PartsData.m_bSoftBin)
    {
        strFieldSpec += "sbin_stats_samples.sbin_no";
        strDbField = m_strTablePrefix + "sbin_stats_samples.sbin_no";
    }
    else
    {
        strFieldSpec += "hbin_stats_samples.hbin_no";
        strDbField = m_strTablePrefix + "hbin_stats_samples.hbin_no";
    }
    m_strlQuery_Fields.append(strFieldSpec);

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add graph, layer and aggregate conditions
    nIndex=0;
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
        ER_Query_AddConditionExpression(*it, pGraph->m_strlGraphSplitValues[nIndex++], strDbField, strDbTable, false);
    nIndex=0;
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
        ER_Query_AddConditionExpression(*it, pLayer->m_strlLayerSplitValues[nIndex++], strDbField, strDbTable, false);

    // Add aggregate value condition
    ER_Query_AddConditionExpression(clER_PartsData.m_strField_Aggregate, strAggregateLabel, strDbField, strDbTable, false);

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strUnionQuery1, false);

    /////////////////// SECOND PART OF THE UNION: SUMMARY ///////////////////////////////////////
    Query_Empty();

    // Add splitlot ID
    strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);

    // Add part count field
    if(!ER_Query_AddPartCount("stats_nbmatching", "BinCount", false, clER_PartsData, "all", false))
        return false;

    // Add site_no
    strFieldSpec = "Field=SiteNo|" + m_strTablePrefix;
    if(clER_PartsData.m_bSoftBin)
    {
        strFieldSpec += "sbin_stats_summary.site_no";
        strDbField = m_strTablePrefix + "sbin_stats_summary.site_no";
    }
    else
    {
        strFieldSpec += "hbin_stats_summary.site_no";
        strDbField = m_strTablePrefix + "hbin_stats_summary.site_no";
    }
    m_strlQuery_Fields.append(strFieldSpec);

    // Add bin_no
    strFieldSpec = "Field=BinNo|" + m_strTablePrefix;
    if(clER_PartsData.m_bSoftBin)
    {
        strFieldSpec += "sbin_stats_summary.sbin_no";
        strDbField = m_strTablePrefix + "sbin_stats_summary.sbin_no";
    }
    else
    {
        strFieldSpec += "hbin_stats_summary.hbin_no";
        strDbField = m_strTablePrefix + "hbin_stats_summary.hbin_no";
    }
    m_strlQuery_Fields.append(strFieldSpec);

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add graph, layer and aggregate conditions
    nIndex=0;
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
        ER_Query_AddConditionExpression(*it, pGraph->m_strlGraphSplitValues[nIndex++], strDbField, strDbTable, false);
    nIndex=0;
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
        ER_Query_AddConditionExpression(*it, pLayer->m_strlLayerSplitValues[nIndex++], strDbField, strDbTable, false);

    // Add aggregate value condition
    ER_Query_AddConditionExpression(clER_PartsData.m_strField_Aggregate, strAggregateLabel, strDbField, strDbTable, false);

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strUnionQuery2, false);

    /////////////////// CREATE THE UNION QUERY, which will be joined to another query ///////////////////////////////////////
    strJoinQuery1 = "(\n";
    strJoinQuery1 += strUnionQuery1;
    strJoinQuery1 += "UNION\n";
    strJoinQuery1 += strUnionQuery2;
    strJoinQuery1 += ") BIN_RESULTS\n";

    /////////////////// CREATE THE SECOND PART OF THE JOIN ///////////////////////////////////////
    Query_Empty();

    // Add splitlot ID
    strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);

    // Add bin_no
    strFieldSpec = "Field=BinNo|" + m_strTablePrefix;
    if(clER_PartsData.m_bSoftBin)
    {
        strFieldSpec += "sbin.sbin_no";
        strDbField = m_strTablePrefix + "sbin.sbin_no";
    }
    else
    {
        strFieldSpec += "hbin.hbin_no";
        strDbField = m_strTablePrefix + "hbin.hbin_no";
    }
    m_strlQuery_Fields.append(strFieldSpec);

    // Add bin_name
    strFieldSpec = "Field=BinName|" + m_strTablePrefix;
    if(clER_PartsData.m_bSoftBin)
    {
        strFieldSpec += "sbin.sbin_name";
        strDbField = m_strTablePrefix + "sbin.sbin_name";
    }
    else
    {
        strFieldSpec += "hbin.hbin_name";
        strDbField = m_strTablePrefix + "hbin.hbin_name";
    }
    m_strlQuery_Fields.append(strFieldSpec);

    // Link condition
    if(clER_PartsData.m_bSoftBin)
        strCondition = m_strTablePrefix +"sbin.splitlot_id|";
    else
        strCondition = m_strTablePrefix +"hbin.splitlot_id|";
    strCondition += m_strTablePrefix +"splitlot.splitlot_id";
    if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
        m_strlQuery_LinkConditions.append(strCondition);

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add graph, layer and aggregate conditions
    nIndex=0;
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
        ER_Query_AddConditionExpression(*it, pGraph->m_strlGraphSplitValues[nIndex++], strDbField, strDbTable, false);
    nIndex=0;
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
        ER_Query_AddConditionExpression(*it, pLayer->m_strlLayerSplitValues[nIndex++], strDbField, strDbTable, false);

    // Add aggregate value condition
    ER_Query_AddConditionExpression(clER_PartsData.m_strField_Aggregate, strAggregateLabel, strDbField, strDbTable, false);

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    // Construct sub-query from table and conditions
    Query_BuildSqlString(strSubQuery, false);
    strJoinQuery2 = "(\n";
    strJoinQuery2 += strSubQuery;
    strJoinQuery2 += ") BIN_INFO\n";


    /////////////////// CREATE FINAL QUERY ///////////////////////////////////////
    strQuery = "SELECT\n";
    strQuery += "BinNo,\n";
    strQuery += "BinName,\n";
    strQuery += "SUM(BinCount) AS BinCount\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += "SELECT\n";
    strQuery += "BIN_RESULTS.SplitlotId,\n";
    strQuery += "BIN_RESULTS.SiteNo,\n";
    strQuery += "BIN_RESULTS.BinNo,\n";
    strQuery += "BIN_INFO.BinName,\n";
    strQuery += "BIN_RESULTS.BinCount\n";
    strQuery += "FROM\n";
    strQuery += strJoinQuery1;
    strQuery += "INNER JOIN\n";
    strQuery += strJoinQuery2;
    strQuery += "ON BIN_RESULTS.SplitlotId=BIN_INFO.SplitlotId AND BIN_RESULTS.BinNo=BIN_INFO.BinNo\n";
    if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        strQuery += ") AS BINNINGS\n";
    else
        strQuery += ") BINNINGS\n";
    strQuery += "GROUP BY BinNo, BinName\n";
    strQuery += "ORDER BY BinNo, BinName\n";

    return true;
}
#else
///////////////////////////////////////////////////////////
// Query: Construct query to retrieve bincount data for ER Production reports
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Prod_ConstructBinCountQuery(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData, GexDbPlugin_ER_Parts_Graph *pGraph, GexDbPlugin_ER_Parts_Layer *pLayer, const QString & strAggregateLabel, QString & strQuery)
{
    QString					strCondition, strFieldSpec, strDbField, strDbTable;
    QStringList::iterator	it;
    QString					strSubQuery, strJoinQuery2, strJoinQuery1, strUnionQuery1, strUnionQuery2;
    int						nIndex;

    // Clear query string
    strQuery = "";

    // Create a union between samples and summary data
    /////////////////// FIRST PART OF THE UNION: SAMPLES ///////////////////////////////////////
    Query_Empty();

    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = m_strTablePrefix + "splitlot.prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Add filter to extract only samples
    if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
    {
        strDbField = m_strTablePrefix+"splitlot.splitlot_flags";
        strCondition = strDbField + "|FieldExpression_Numeric|4";
        strCondition += "|(" + strDbField + " & 4)";
        m_strlQuery_ValueConditions.append(strCondition);
    }

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    if(clER_PartsData.m_bSoftBin)
    {
        // Add Bin#/Bin Name/ Bin Cat
        strFieldSpec = "Field=BinName|" + m_strTablePrefix;
        strFieldSpec += "sbin.sbin_name";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinNo|" + m_strTablePrefix;
        strFieldSpec += "sbin.sbin_no";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinCat|" + m_strTablePrefix;
        strFieldSpec += "sbin.sbin_cat";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strCondition = m_strTablePrefix +"sbin.splitlot_id|";
        strCondition += m_strTablePrefix +"splitlot.splitlot_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);

        // Add SiteNo,BinCount
        strFieldSpec = "Field=SiteNo|" + m_strTablePrefix;
        strFieldSpec += "sbin_stats_samples.site_no";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinCount|" + m_strTablePrefix;
        strFieldSpec += "sbin_stats_samples.nb_parts";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strCondition = m_strTablePrefix +"sbin_stats_samples.splitlot_id|";
        strCondition += m_strTablePrefix +"splitlot.splitlot_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);
        strCondition = m_strTablePrefix +"sbin_stats_samples.sbin_no|";
        strCondition += m_strTablePrefix +"sbin.sbin_no";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);

        // Extract only values for site != -1
        strCondition = m_strTablePrefix+"sbin_stats_samples.site_no|Numeric|!=-1";
        m_strlQuery_ValueConditions.append(strCondition);
    }
    else
    {
        // Add Bin#/Bin Name/ Bin Cat
        strFieldSpec = "Field=BinName|" + m_strTablePrefix;
        strFieldSpec += "hbin.hbin_name";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinNo|" + m_strTablePrefix;
        strFieldSpec += "hbin.hbin_no";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinCat|" + m_strTablePrefix;
        strFieldSpec += "hbin.hbin_cat";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strCondition = m_strTablePrefix +"hbin.splitlot_id|";
        strCondition += m_strTablePrefix +"splitlot.splitlot_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);

        // Add SiteNo,BinCount
        strFieldSpec = "Field=SiteNo|" + m_strTablePrefix;
        strFieldSpec += "hbin_stats_samples.site_no";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinCount|" + m_strTablePrefix;
        strFieldSpec += "hbin_stats_samples.nb_parts";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strCondition = m_strTablePrefix +"hbin_stats_samples.splitlot_id|";
        strCondition += m_strTablePrefix +"splitlot.splitlot_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);
        strCondition = m_strTablePrefix +"hbin_stats_samples.hbin_no|";
        strCondition += m_strTablePrefix +"hbin.hbin_no";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);

        // Extract only values for site != -1
        strCondition = m_strTablePrefix+"hbin_stats_samples.site_no|Numeric|!=-1";
        m_strlQuery_ValueConditions.append(strCondition);
    }

    // Add graph, layer and aggregate conditions
    nIndex=0;
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
        ER_Query_AddConditionExpression(*it, pGraph->m_strlGraphSplitValues[nIndex++], strDbField, strDbTable, false);
    nIndex=0;
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
        ER_Query_AddConditionExpression(*it, pLayer->m_strlLayerSplitValues[nIndex++], strDbField, strDbTable, false);

    // Add aggregate value condition
    ER_Query_AddConditionExpression(clER_PartsData.m_strField_Aggregate, strAggregateLabel, strDbField, strDbTable, false);

    // Construct sub-query from table and conditions
    Query_BuildSqlString_UsingJoins(strUnionQuery1, false, "", false, GexDbPlugin_Base::eQueryJoin_LeftOuter);

    /////////////////// SECOND PART OF THE UNION: SUMMARY ///////////////////////////////////////
    Query_Empty();

    // Add splitlot ID to make sure each line is unique
    strFieldSpec = "Field=SplitlotId|" + m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
        m_strlQuery_Fields.append(strFieldSpec);

    // Add data filter
    strCondition = m_strTablePrefix + "splitlot.valid_splitlot|NotString|N";
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = m_strTablePrefix + "splitlot.prod_data|String|Y";
    m_strlQuery_ValueConditions.append(strCondition);

    // Add filter to extract only summary
    if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
    {
        strDbField = m_strTablePrefix+"splitlot.splitlot_flags";
        strCondition = strDbField + "|FieldExpression_Numeric|0";
        strCondition += "|(" + strDbField + " & 4)";
        m_strlQuery_ValueConditions.append(strCondition);
    }

    // Set filters
    if(!Query_AddFilters(cFilters))
        return false;

    // Add time period condition
    if(!Query_AddTimePeriodCondition(cFilters))	return false;

    if(clER_PartsData.m_bSoftBin)
    {
        // Add Bin#/Bin Name/ Bin Cat
        strFieldSpec = "Field=BinName|" + m_strTablePrefix;
        strFieldSpec += "sbin.sbin_name";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinNo|" + m_strTablePrefix;
        strFieldSpec += "sbin.sbin_no";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinCat|" + m_strTablePrefix;
        strFieldSpec += "sbin.sbin_cat";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strCondition = m_strTablePrefix +"sbin.splitlot_id|";
        strCondition += m_strTablePrefix +"splitlot.splitlot_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);

        // Add SiteNo,BinCount
        strFieldSpec = "Field=SiteNo|" + m_strTablePrefix;
        strFieldSpec += "sbin_stats_summary.site_no";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinCount|" + m_strTablePrefix;
        strFieldSpec += "sbin_stats_summary.bin_count";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strCondition = m_strTablePrefix +"sbin_stats_summary.splitlot_id|";
        strCondition += m_strTablePrefix +"splitlot.splitlot_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);
        strCondition = m_strTablePrefix +"sbin_stats_summary.sbin_no|";
        strCondition += m_strTablePrefix +"sbin.sbin_no";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);

        // Extract only values for site != -1
        strCondition = m_strTablePrefix+"sbin_stats_summary.site_no|Numeric|<0";
        m_strlQuery_ValueConditions.append(strCondition);
    }
    else
    {
        // Add Bin#/Bin Name/ Bin Cat
        strFieldSpec = "Field=BinName|" + m_strTablePrefix;
        strFieldSpec += "hbin.hbin_name";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinNo|" + m_strTablePrefix;
        strFieldSpec += "hbin.hbin_no";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinCat|" + m_strTablePrefix;
        strFieldSpec += "hbin.hbin_cat";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strCondition = m_strTablePrefix +"hbin.splitlot_id|";
        strCondition += m_strTablePrefix +"splitlot.splitlot_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);

        // Add SiteNo,BinCount
        strFieldSpec = "Field=SiteNo|" + m_strTablePrefix;
        strFieldSpec += "hbin_stats_summary.site_no";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strFieldSpec = "Field=BinCount|" + m_strTablePrefix;
        strFieldSpec += "hbin_stats_summary.bin_count";
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        strCondition = m_strTablePrefix +"hbin_stats_summary.splitlot_id|";
        strCondition += m_strTablePrefix +"splitlot.splitlot_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);
        strCondition = m_strTablePrefix +"hbin_stats_summary.hbin_no|";
        strCondition += m_strTablePrefix +"hbin.hbin_no";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);

        // Extract only values for site != -1
        strCondition = m_strTablePrefix+"hbin_stats_summary.site_no|Numeric|<0";
        m_strlQuery_ValueConditions.append(strCondition);
    }

    // Add graph, layer and aggregate conditions
    nIndex=0;
    for(it=clER_PartsData.m_strlFields_GraphSplit.begin(); it!=clER_PartsData.m_strlFields_GraphSplit.end(); it++)
        ER_Query_AddConditionExpression(*it, pGraph->m_strlGraphSplitValues[nIndex++], strDbField, strDbTable, false);
    nIndex=0;
    for(it=clER_PartsData.m_strlFields_LayerSplit.begin(); it!=clER_PartsData.m_strlFields_LayerSplit.end(); it++)
        ER_Query_AddConditionExpression(*it, pLayer->m_strlLayerSplitValues[nIndex++], strDbField, strDbTable, false);

    // Add aggregate value condition
    ER_Query_AddConditionExpression(clER_PartsData.m_strField_Aggregate, strAggregateLabel, strDbField, strDbTable, false);

    // Construct sub-query from table and conditions
    Query_BuildSqlString_UsingJoins(strUnionQuery2, false, "", false, GexDbPlugin_Base::eQueryJoin_LeftOuter);

    /////////////////// CREATE THE UNION QUERY, which will be joined to another query ///////////////////////////////////////
    strJoinQuery1 = "(\n";
    strJoinQuery1 += strUnionQuery1;
    strJoinQuery1 += "UNION ALL\n";
    strJoinQuery1 += strUnionQuery2;
    strJoinQuery1 += ") BIN_RESULTS\n";

    /////////////////// CREATE FINAL QUERY ///////////////////////////////////////
    strQuery = "SELECT\n";
    strQuery += "BinNo,\n";
    strQuery += "BinName,\n";
    strQuery += "SUM(BinCount) AS BinCount,\n";
    strQuery += "BinCat\n";
    strQuery += "FROM\n";
    strQuery += strJoinQuery1;
    strQuery += "WHERE BinCount is not null\n";
    strQuery += "GROUP BY BinNo, BinName, BinCat\n";
    strQuery += "ORDER BY BinNo, BinName, BinCat\n";

    return true;
}
#endif

///////////////////////////////////////////////////////////
// Query: add a field to query selection, eventually based
// on an expression (day, week...)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Query_AddFieldExpression(const QString & strMetaField, const QString & strAs, QString & strDbField, QString & strDbTable, bool bUseMax/*=false*/)
{
    QString strField, strFieldSpec, strDbField0, strExpression;

    // Add field and conditions
    strField = strMetaField.toLower();
    if(strField == "day")
    {
        strDbField0 = m_strTablePrefix+"splitlot.start_t";
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strExpression = "date_format(convert_tz(from_unixtime(";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "),'SYSTEM','+0:00'),'%Y-%m-%d')";
            strFieldSpec += "|" + strExpression;
            m_strlQuery_Fields.append(strFieldSpec);
        }
        else
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strExpression = "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "/(24*3600),'YYYY-MM-DD')";
            strFieldSpec += "|" + strExpression;
            m_strlQuery_Fields.append(strFieldSpec);
        }
        strDbField = strExpression;
        strDbTable = m_strTablePrefix+"splitlot";
    }
    else if(strField == "week")
    {
        strDbField0 = m_strTablePrefix+"splitlot.start_t";
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strExpression = "date_format(convert_tz(from_unixtime(";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "),'SYSTEM','+0:00'),'%Y-%v')";
            strFieldSpec += "|" + strExpression;
            m_strlQuery_Fields.append(strFieldSpec);
        }
        else
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strExpression = "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "/(24*3600),'YYYY-IW')";
            strFieldSpec += "|" + strExpression;
            m_strlQuery_Fields.append(strFieldSpec);
        }
        strDbField = strExpression;
        strDbTable = m_strTablePrefix+"splitlot";
    }
    else if(strField == "month")
    {
        strDbField0 = m_strTablePrefix+"splitlot.start_t";
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strExpression = "date_format(convert_tz(from_unixtime(";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "),'SYSTEM','+0:00'),'%Y-%m')";
            strFieldSpec += "|" + strExpression;
            m_strlQuery_Fields.append(strFieldSpec);
        }
        else
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strExpression = "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "/(24*3600),'YYYY-MM')";
            strFieldSpec += "|" + strExpression;
            m_strlQuery_Fields.append(strFieldSpec);
        }
        strDbField = strExpression;
        strDbTable = m_strTablePrefix+"splitlot";
    }
    else if(strField == "year")
    {
        strDbField0 = m_strTablePrefix+"splitlot.start_t";
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strExpression = "date_format(convert_tz(from_unixtime(";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "),'SYSTEM','+0:00'),'%Y')";
            strFieldSpec += "|" + strExpression;
            m_strlQuery_Fields.append(strFieldSpec);
        }
        else
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strExpression = "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "/(24*3600),'YYYY')";
            strFieldSpec += "|" + strExpression;
            m_strlQuery_Fields.append(strFieldSpec);
        }
        strDbField = strExpression;
        strDbTable = m_strTablePrefix+"splitlot";
    }
    else if(bUseMax)
    {

    }
    else
    {
        strFieldSpec = strMetaField;
        strFieldSpec += "=";
        strFieldSpec += strAs;
        if(!Query_AddField(strFieldSpec, strDbField, strDbTable))
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: add a condition to query selection, eventually based
// on an expression (day, week...)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Query_AddConditionExpression(const QString & strMetaField, const QString & strValue, QString & strDbField, QString & strDbTable, bool bUseMax/*=false*/)
{
    QString strField, strCondition, strDbField0, strExpression;

    // Add field and conditions
    strField = strMetaField.toLower();
    if(strField == "day")
    {
        strDbField0 = m_strTablePrefix+"splitlot.start_t";
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strExpression = "date_format(convert_tz(from_unixtime(";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "),'SYSTEM','+0:00'),'%Y-%m-%d')";
            strCondition = strDbField0 + "|FieldExpression_String|";
            strCondition += strValue;
            strCondition += "|" + strExpression;
            m_strlQuery_ValueConditions.append(strCondition);
        }
        else
        {
            strExpression = "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "/(24*3600),'YYYY-MM-DD')";
            strCondition = strDbField0 + "|FieldExpression_String|";
            strCondition += strValue;
            strCondition += "|" + strExpression;
            m_strlQuery_ValueConditions.append(strCondition);
        }
        strDbField = strExpression;
        strDbTable = m_strTablePrefix+"splitlot";
    }
    else if(strField == "week")
    {
        strDbField0 = m_strTablePrefix+"splitlot.start_t";
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strExpression = "date_format(convert_tz(from_unixtime(";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "),'SYSTEM','+0:00'),'%Y-%v')";
            strCondition = strDbField0 + "|FieldExpression_String|";
            strCondition += strValue;
            strCondition += "|" + strExpression;
            m_strlQuery_ValueConditions.append(strCondition);
        }
        else
        {
            strExpression = "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "/(24*3600),'YYYY-IW')";
            strCondition = strDbField0 + "|FieldExpression_String|";
            strCondition += strValue;
            strCondition += "|" + strExpression;
            m_strlQuery_ValueConditions.append(strCondition);
        }
        strDbField = strExpression;
        strDbTable = m_strTablePrefix+"splitlot";
    }
    else if(strField == "month")
    {
        strDbField0 = m_strTablePrefix+"splitlot.start_t";
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strExpression = "date_format(convert_tz(from_unixtime(";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "),'SYSTEM','+0:00'),'%Y-%m')";
            strCondition = strDbField0 + "|FieldExpression_String|";
            strCondition += strValue;
            strCondition += "|" + strExpression;
            m_strlQuery_ValueConditions.append(strCondition);
        }
        else
        {
            strExpression = "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "/(24*3600),'YYYY-MM')";
            strCondition = strDbField0 + "|FieldExpression_String|";
            strCondition += strValue;
            strCondition += "|" + strExpression;
            m_strlQuery_ValueConditions.append(strCondition);
        }
        strDbField = strExpression;
        strDbTable = m_strTablePrefix+"splitlot";
    }
    else if(strField == "year")
    {
        strDbField0 = m_strTablePrefix+"splitlot.start_t";
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strExpression = "date_format(convert_tz(from_unixtime(";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "),'SYSTEM','+0:00'),'%Y')";
            strCondition = strDbField0 + "|FieldExpression_String|";
            strCondition += strValue;
            strCondition += "|" + strExpression;
            m_strlQuery_ValueConditions.append(strCondition);
        }
        else
        {
            strExpression = "to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMax)
            {
                strExpression += "max(";
                strExpression += strDbField0;
                strExpression += ")";
            }
            else
                strExpression += strDbField0;
            strExpression += "/(24*3600),'YYYY')";
            strCondition = strDbField0 + "|FieldExpression_String|";
            strCondition += strValue;
            strCondition += "|" + strExpression;
            m_strlQuery_ValueConditions.append(strCondition);
        }
        strDbField = strExpression;
        strDbTable = m_strTablePrefix+"splitlot";
    }
    else
    {
        if(!Query_AddValueCondition(strMetaField, strValue))
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: add a part count to query selection
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Query_AddPartCount(const QString & strMetaField, const QString & strAs, bool bUseSamples, GexDbPlugin_ER_Parts & clER_PartsData, const QString & strBinspec, bool bSumParts)
{
    QString strField, strFieldSpec, strDbField0, strDbField1, strDbLink0, strCondition, strBinType, strBinlist;

    strBinlist = strBinspec.toLower();

    // Create list of binnings to use
    if(strBinlist.indexOf("pass") != -1)
        strBinlist = clER_PartsData.m_strBinlist_Pass;
    else if(strBinlist.indexOf("fail") != -1)
        strBinlist = clER_PartsData.m_strBinlist_Fail;
    else if(strBinlist.indexOf("all") != -1)
        strBinlist = "*";
    else if(!strBinspec.isEmpty())
        strBinlist = strBinspec;

    // Set bin table to be used (SBIN or HBIN)
    if(clER_PartsData.m_bSoftBin)
        strBinType = "sbin";
    else
        strBinType = "hbin";

    // Add field and conditions
    strField = strMetaField.toLower();
    if(strField == "stats_nbparts")
    {
        if(bUseSamples)
        {
            // Field
            strDbField0 = m_strTablePrefix+"parts_stats_samples.nb_parts";
            strDbField1 = m_strTablePrefix+"splitlot.splitlot_flags";
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            {
                strFieldSpec += "|(case when (";
                strFieldSpec += strDbField1;
                strFieldSpec += " & 4)=4 then ";
            }
            strFieldSpec += strDbField0;
            strFieldSpec += " else 0 end)";
            m_strlQuery_Fields.append(strFieldSpec);

            // Link condition
            strDbLink0 = m_strTablePrefix +"parts_stats_samples.splitlot_id|";
            strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
            if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
                m_strlQuery_LinkConditions.append(strDbLink0);

            // Value condition
            strCondition = m_strTablePrefix+"parts_stats_samples.site_no";
            strCondition += "|Numeric|0-9999";
            if(m_strlQuery_ValueConditions.indexOf(strCondition))
                m_strlQuery_ValueConditions.append(strCondition);
        }
        else
        {
            // Field
            strDbField0 = m_strTablePrefix+"parts_stats_summary.nb_parts";
            strDbField1 = m_strTablePrefix+"splitlot.splitlot_flags";
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            {
                strFieldSpec += "|(case when (";
                strFieldSpec += strDbField1;
                strFieldSpec += " & 4)=4 then 0 else ";
            }

            strFieldSpec += strDbField0;
            strFieldSpec += " end)";
            m_strlQuery_Fields.append(strFieldSpec);

            // Link condition
            strDbLink0 = m_strTablePrefix +"parts_stats_summary.splitlot_id|";
            strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
            if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
                m_strlQuery_LinkConditions.append(strDbLink0);

            // Value condition
            strCondition = m_strTablePrefix+"parts_stats_summary.site_no";
            strCondition += "|Expression|-1";
            if(m_strlQuery_ValueConditions.indexOf(strCondition) == -1)
                m_strlQuery_ValueConditions.append(strCondition);
        }
    }
    else if(strField == "stats_nbmatching")
    {
        if(bUseSamples)
        {
            // Field
            strDbField0 = m_strTablePrefix+strBinType+"_stats_samples.nb_parts";
            strDbField1 = m_strTablePrefix+"splitlot.splitlot_flags";
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            {
                if(bSumParts)
                    strFieldSpec += "|SUM(case when (";
                else
                    strFieldSpec += "|(case when (";
                strFieldSpec += strDbField1;
                strFieldSpec += " & 4)=4 then ";
            }
            else
            {
                if(bSumParts)
                    strFieldSpec += "|SUM(case when bitand(";
                else
                    strFieldSpec += "|(case when bitand(";
                strFieldSpec += strDbField1;
                strFieldSpec += ",4)=4 then ";
            }
            strFieldSpec += strDbField0;
            strFieldSpec += " else 0 end)";
            m_strlQuery_Fields.append(strFieldSpec);

            // Link condition
            strDbLink0 = m_strTablePrefix +strBinType+"_stats_samples.splitlot_id|";
            strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
            if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
                m_strlQuery_LinkConditions.append(strDbLink0);

            // Value condition
            strCondition = m_strTablePrefix+strBinType+"_stats_samples.site_no";
            strCondition += "|Numeric|0-9999";
            if(m_strlQuery_ValueConditions.indexOf(strCondition) == -1)
                m_strlQuery_ValueConditions.append(strCondition);

            if(strBinlist != "*")
            {
                strCondition = m_strTablePrefix+strBinType+"_stats_samples.";
                strCondition += strBinType+"_no";
                strCondition += "|Numeric|";
                strCondition += strBinlist;
                if(m_strlQuery_ValueConditions.indexOf(strCondition) == -1)
                    m_strlQuery_ValueConditions.append(strCondition);
            }
        }
        else
        {
            // Field
            strDbField0 = m_strTablePrefix+strBinType+"_stats_summary.bin_count";
            strDbField1 = m_strTablePrefix+"splitlot.splitlot_flags";
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            {
                if(bSumParts)
                    strFieldSpec += "|SUM(case when (";
                else
                    strFieldSpec += "|(case when (";
                strFieldSpec += strDbField1;
                strFieldSpec += " & 4)=4 then 0 else ";
            }
            else
            {
                if(bSumParts)
                    strFieldSpec += "|SUM(case when bitand(";
                else
                    strFieldSpec += "|(case when bitand(";
                strFieldSpec += strDbField1;
                strFieldSpec += ",4)=4 then 0 else ";
            }
            strFieldSpec += strDbField0;
            strFieldSpec += " end)";
            m_strlQuery_Fields.append(strFieldSpec);

            // Link condition
            strDbLink0 = m_strTablePrefix +strBinType+"_stats_summary.splitlot_id|";
            strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
            if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
                m_strlQuery_LinkConditions.append(strDbLink0);

            // Value condition
            strCondition = m_strTablePrefix+strBinType+"_stats_summary.site_no";
            strCondition += "|Expression|-1";
            if(m_strlQuery_ValueConditions.indexOf(strCondition) == -1)
                m_strlQuery_ValueConditions.append(strCondition);

            if(strBinlist != "*")
            {
                strCondition = m_strTablePrefix+strBinType+"_stats_summary.";
                strCondition += strBinType+"_no";
                strCondition += "|Numeric|";
                strCondition += strBinlist;
                if(m_strlQuery_ValueConditions.indexOf(strCondition) == -1)
                    m_strlQuery_ValueConditions.append(strCondition);
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: add a test count to query selection (execs, fails, sum)
///////////////////////////////////////////////////////////
bool
GexDbPlugin_Galaxy::
ER_Query_AddTestCount(GexDbPlugin_ER_Parts_SerieDef* pSerieDef,
                      const QString& strMetaField,
                      const QString& strAs,
                      bool bUseSamples,
                      bool /*bSumTests*/)
{
    QString strMetaField_lower, strDbField, strStatsTable, strFieldSpec, strDbField0, strDbField1, strDbLink0, strCondition;

    // Get field to query
    strMetaField_lower = strMetaField.toLower();
    if(strMetaField_lower == "stats_ptest_execs")
        strDbField = "exec_count";
    else if(strMetaField_lower == "stats_ptest_fails")
        strDbField = "fail_count";
    else if(strMetaField_lower == "stats_ptest_sum")
        strDbField = "sum";

    // Get stats table to use
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        strStatsTable = "ptest_stats";
        bUseSamples = false;
    }
    else if(bUseSamples)
        strStatsTable = "ptest_stats_samples";
    else
        strStatsTable = "ptest_stats_summary";

    // Construct query
    strDbField0 = m_strTablePrefix+strStatsTable+"."+strDbField;
    strDbField1 = m_strTablePrefix+"splitlot.splitlot_flags";
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        strFieldSpec = "Field=" + strAs;
        strFieldSpec += "|" + strDbField0;
    }
    else
    {
        strFieldSpec = "Expression=";
        strFieldSpec += strAs;
        strFieldSpec += "|";
        strFieldSpec += strDbField0;
        if(bUseSamples)
        {
            if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            {
                strFieldSpec += "|(case when (";
                strFieldSpec += strDbField1;
                strFieldSpec += " & 4)=4 then ";
            }

            strFieldSpec += strDbField0;
            strFieldSpec += " else 0 end)";
        }
        else
        {
            if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
            {
                strFieldSpec += "|(case when (";
                strFieldSpec += strDbField1;
                strFieldSpec += " & 4)=4 then 0 else ";
            }

            strFieldSpec += strDbField0;
            strFieldSpec += " end)";
        }
    }
    m_strlQuery_Fields.append(strFieldSpec);

    // Add some value conditions
    strCondition = m_strTablePrefix+"ptest_info.tnum";
    strCondition += "|Numeric|" + pSerieDef->m_strParameter;
    if(m_strlQuery_ValueConditions.indexOf(strCondition) == -1)
        m_strlQuery_ValueConditions.append(strCondition);
    if(pSerieDef->m_strTestingStage != GEXDB_PLUGIN_GALAXY_ETEST)
    {
        strCondition = m_strTablePrefix+strStatsTable+".site_no";
        if(bUseSamples)
            strCondition += "|Numeric|0-9999";
        else
            strCondition += "|Expression|-1";
        if(m_strlQuery_ValueConditions.indexOf(strCondition) == -1)
            m_strlQuery_ValueConditions.append(strCondition);
    }

    // Add some link conditions
    strDbLink0 = m_strTablePrefix +strStatsTable+".splitlot_id|";
    strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
    if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
        m_strlQuery_LinkConditions.append(strDbLink0);
    strDbLink0 = m_strTablePrefix +"ptest_info.splitlot_id|";
    strDbLink0 += m_strTablePrefix +strStatsTable+".splitlot_id";
    if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
        m_strlQuery_LinkConditions.append(strDbLink0);
    strDbLink0 = m_strTablePrefix +"ptest_info.ptest_info_id|";
    strDbLink0 += m_strTablePrefix +strStatsTable+".ptest_info_id";
    if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
        m_strlQuery_LinkConditions.append(strDbLink0);

    WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::ER_Query_AddTestCount: added %1 LinkConditions : %2")
                          .arg(m_strlQuery_LinkConditions.size())
                          .arg(m_strlQuery_LinkConditions.join("\t")));

    return true;
}

//////////////////////////////////////////////////////////////////////
// Compute data for Enterprise Report graphs (Genealogy - Yield vs Yield)
// (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_YieldVsYield_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData)
{
    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate)
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // Construct SQL query for Yield
    QString strQuery;
    if(!ER_Genealogy_YieldVsYield_ConstructQuery(cFilters, clER_PartsData, strQuery))
        return false;

    // Save query
    clER_PartsData.m_strYieldQuery = strQuery;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clGexDbQuery.lastError().text().left(1024).toLatin1().constData());
        return false;
    }

    // Fill Parts data object
    QString										strAggregate;
    QStringList									strlEmptySplit;
    int											nRows=0, nIndex;
    unsigned int								uiIndex;
    GexDbPlugin_ER_Parts_Graph					*pGraph=NULL;
    GexDbPlugin_ER_Parts_Layer					*pLayer=NULL;
    QList<unsigned int>							uilParts, uilMatchingParts;

    // Create new graph/layer
    pGraph = new GexDbPlugin_ER_Parts_Graph(strlEmptySplit);
    pLayer = new GexDbPlugin_ER_Parts_Layer(clER_PartsData.m_uiSeries, strlEmptySplit);

    while(clGexDbQuery.Next())
    {
        nRows++;

        // Get query data
        nIndex = 0;
        strlEmptySplit.clear();

        strAggregate = clGexDbQuery.value(nIndex++).toString();
        if(clER_PartsData.m_strGenealogy_Granularity.toLower() == "wafer")
            strAggregate += "/" + clGexDbQuery.value(nIndex++).toString();
        uilMatchingParts.clear();
        uilParts.clear();
        for(uiIndex=0; uiIndex<clER_PartsData.m_uiSeries; uiIndex++)
        {
            uilParts.append(clGexDbQuery.value(nIndex++).toUInt());
            uilMatchingParts.append(clGexDbQuery.value(nIndex++).toUInt());
        }

        // Add data to layer
        pLayer->Add(strAggregate, uilParts, uilMatchingParts);
    }

    // Add last layer/graph
    pGraph->append(pLayer);
    clER_PartsData.append(pGraph);

    // Check if we have some results
    if(nRows == 0)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoResult_0, NULL);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_YieldVsYield_ConstructQuery(
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_ER_Parts & clER_PartsData,
        QString & strQuery)
{
    QString							strSubQuery_X, strSubQuery_Y;
    GexDbPlugin_ER_Parts_SerieDef	*pSerieDef;
    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <fields>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields

    // Check if lot or wafer granularity
    if(clER_PartsData.m_strGenealogy_Granularity.toLower() == "wafer")
    {
        // 1. X-Axis
        pSerieDef = clER_PartsData.m_plistSerieDefs.at(0);
        ER_Genealogy_Yield_ConstructQuery_Core_Wafer(cFilters, pSerieDef, strSubQuery_X, 0);

        // 2. Y-Axis
        pSerieDef = clER_PartsData.m_plistSerieDefs.at(1);
        ER_Genealogy_Yield_ConstructQuery_Core_Wafer(cFilters, pSerieDef, strSubQuery_Y, 1);

        // 3. Build final query
        strQuery = "SELECT\n";
        strQuery += "T.TrackingLot,\n";
        strQuery += "T.Wafer,\n";
        strQuery += "T.Parts_X,\n";
        strQuery += "T.Matching_X,\n";
        strQuery += "T.Parts_Y,\n";
        strQuery += "T.Matching_Y\n";
        strQuery += "FROM\n";
        strQuery += "(\n";
        strQuery += "SELECT\n";
        strQuery += "TX.TrackingLot AS TrackingLot,\n";
        strQuery += "TX.Wafer AS Wafer,\n";
        strQuery += "TX.Parts AS Parts_X,\n";
        strQuery += "TX.Matching AS Matching_X,\n";
        strQuery += "TX.MaxStart AS MaxStart_X,\n";
        strQuery += "TX.MinStart AS MinStart_X,\n";
        strQuery += "TY.Parts AS Parts_Y,\n";
        strQuery += "TY.Matching AS Matching_Y,\n";
        strQuery += "TY.MaxStart AS MaxStart_Y,\n";
        strQuery += "TY.MinStart AS MinStart_Y\n";
        strQuery += "FROM\n";
        strQuery += "(\n";
        strQuery += strSubQuery_X;
        strQuery += ") TX\n";
        strQuery += "INNER JOIN\n";
        strQuery += "(\n";
        strQuery += strSubQuery_Y;
        strQuery += ") TY\n";
        strQuery += "ON TX.TrackingLot=TY.TrackingLot AND TX.Wafer=TY.Wafer\n";
        strQuery += ") T\n";
        strQuery += "WHERE\n";
        strQuery += "(MaxStart_X >= " + QString::number(cFilters.tQueryFrom);
        strQuery += " AND MinStart_X <= " + QString::number(cFilters.tQueryTo);
        strQuery += ") OR\n";
        strQuery += "(MaxStart_Y >= " + QString::number(cFilters.tQueryFrom);
        strQuery += " AND MinStart_Y <= " + QString::number(cFilters.tQueryTo);
        strQuery += ")\n";
        strQuery += "ORDER BY TrackingLot, convert(Wafer, unsigned) ASC\n";
    }
    else
    {
        // 1. X-Axis
        pSerieDef = clER_PartsData.m_plistSerieDefs.at(0);
        ER_Genealogy_Yield_ConstructQuery_Core_Lot(cFilters, pSerieDef, strSubQuery_X, 0);

        // 2. Y-Axis
        pSerieDef = clER_PartsData.m_plistSerieDefs.at(1);
        ER_Genealogy_Yield_ConstructQuery_Core_Lot(cFilters, pSerieDef, strSubQuery_Y, 1);

        // 3. Build final query
        strQuery = "SELECT\n";
        strQuery += "T.TrackingLot,\n";
        strQuery += "T.Parts_X,\n";
        strQuery += "T.Matching_X,\n";
        strQuery += "T.Parts_Y,\n";
        strQuery += "T.Matching_Y\n";
        strQuery += "FROM\n";
        strQuery += "(\n";
        strQuery += "SELECT\n";
        strQuery += "TX.TrackingLot AS TrackingLot,\n";
        strQuery += "TX.Parts AS Parts_X,\n";
        strQuery += "TX.Matching AS Matching_X,\n";
        strQuery += "TX.MaxStart AS MaxStart_X,\n";
        strQuery += "TX.MinStart AS MinStart_X,\n";
        strQuery += "TY.Parts AS Parts_Y,\n";
        strQuery += "TY.Matching AS Matching_Y,\n";
        strQuery += "TY.MaxStart AS MaxStart_Y,\n";
        strQuery += "TY.MinStart AS MinStart_Y\n";
        strQuery += "FROM\n";
        strQuery += "(\n";
        strQuery += strSubQuery_X;
        strQuery += ") TX\n";
        strQuery += "INNER JOIN\n";
        strQuery += "(\n";
        strQuery += strSubQuery_Y;
        strQuery += ") TY\n";
        strQuery += "ON TX.TrackingLot=TY.TrackingLot\n";
        strQuery += ") T\n";
        strQuery += "WHERE\n";
        strQuery += "(MaxStart_X >= " + QString::number(cFilters.tQueryFrom);
        strQuery += " AND MinStart_X <= " + QString::number(cFilters.tQueryTo);
        strQuery += ") OR\n";
        strQuery += "(MaxStart_Y >= " + QString::number(cFilters.tQueryFrom);
        strQuery += " AND MinStart_Y <= " + QString::number(cFilters.tQueryTo);
        strQuery += ")\n";
        strQuery += "ORDER BY TrackingLot ASC\n";
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_Yield_ConstructQuery_Core_Lot(
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
        QString & strQuery,
        int nIndex)
{
    if(pSerieDef->m_bWorkOnBinnings)
        return ER_Genealogy_Yield_ConstructQuery_Core_Lot_Binning(cFilters, pSerieDef, strQuery, nIndex);

    return ER_Genealogy_Yield_ConstructQuery_Core_Lot_NoBinning(cFilters, pSerieDef, strQuery);
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_Yield_ConstructQuery_Core_Lot_Binning(
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
        QString & strQuery,
        int nIndex)
{
    QString			strSubQuery1, strSubQuery2;
    QString			strFieldSpec, strDbField, strDbTable;

    strQuery = "";

    // First part of the Join is the core query without binning specification
    // (some fields will be ignored, ie Matching)
    ER_Genealogy_Yield_ConstructQuery_Core_Lot_NoBinning(cFilters, pSerieDef, strSubQuery1);

    // Second part of the Join is the results corresponding to selected binnings
    Query_Empty();
    // Set Testing stage (default to FT)
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
    m_strTablePrefix = "ft_";
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = "et_";
    }
    else if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = "wt_";
    }
    // Lot field
    strFieldSpec = "Field=Lot|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "lot.lot_id");
    // Matching parts field
    strFieldSpec = "Function=Matching|";
    strFieldSpec += m_strTablePrefix + "lot_sbin.nb_parts|SUM";
    m_strlQuery_Fields.append(strFieldSpec);

    // Add conditions
    strFieldSpec = m_strTablePrefix + "lot.lot_id|";
    strFieldSpec += m_strTablePrefix + "lot_sbin.lot_id";
    m_strlQuery_LinkConditions.append(strFieldSpec);
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "lot.tracking_lot_id|NotString|");
    // GCORE-1200: Checked [SC]
    // Filter on one product
    strFieldSpec = m_strTablePrefix + "lot.product_name|String|";
    strFieldSpec += pSerieDef->m_strProduct;
    m_strlQuery_ValueConditions.append(strFieldSpec);
    strFieldSpec = m_strTablePrefix + "lot_sbin.sbin_no|Numeric|";
    strFieldSpec += pSerieDef->m_strBinnings;
    m_strlQuery_ValueConditions.append(strFieldSpec);
    m_strlQuery_GroupFields.append(m_strTablePrefix + "lot_sbin.lot_id");
    Query_AddFilters(cFilters);
    Query_BuildSqlString(strSubQuery2, false);

    // Build final query
    strFieldSpec = "T" + QString::number(nIndex);
    strQuery = "SELECT\n";
    strQuery += strFieldSpec + "_1.TrackingLot,\n";
    strQuery += strFieldSpec + "_1.Parts as Parts,\n";
    strQuery += strFieldSpec + "_2.Matching as Matching,\n";
    strQuery += strFieldSpec + "_1.MinStart as MinStart,\n";
    strQuery += strFieldSpec + "_1.MaxStart as MaxStart\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += strSubQuery1;
    strQuery += ") " + strFieldSpec;
    strQuery += "_1\n";
    strQuery += "INNER JOIN\n";
    strQuery += "(\n";
    strQuery += strSubQuery2;
    strQuery += ") " + strFieldSpec;
    strQuery += "_2\n";
    strQuery += "ON " + strFieldSpec;
    strQuery += "_1.Lot=" + strFieldSpec;
    strQuery += "_2.Lot\n";

    return true;
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_Yield_ConstructQuery_Core_Lot_NoBinning(
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
        QString & strQuery)
{
    QString			strFieldSpec, strDbField, strDbTable;

    Query_Empty();
    strQuery = "";

    // Set Testing stage (default to FT)
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
    m_strTablePrefix = "ft_";
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = "et_";
    }
    else if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = "wt_";
    }
    // Tracking Lot field
    strFieldSpec  = GEXDB_PLUGIN_DBFIELD_TRACKING_LOT;
    strFieldSpec += "=TrackingLot";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // Lot field
    strFieldSpec  = m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT];
    strFieldSpec += "=Lot";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // Total parts field
    strFieldSpec = "Function=Parts|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "lot.nb_parts|SUM");
    // Matching parts field
    if(pSerieDef->m_strBinnings.toLower().indexOf("fail") == -1)
    {
        strFieldSpec = "Function=Matching|" + m_strTablePrefix;
        m_strlQuery_Fields.append(strFieldSpec + "lot.nb_parts_good|SUM");
    }
    else
    {
        strFieldSpec = "Function=Matching";
        strFieldSpec += "|" + m_strTablePrefix;
        strFieldSpec += "lot.nb_parts";
        strFieldSpec += "|(" + m_strTablePrefix;
        strFieldSpec += "lot.nb_parts-";
        strFieldSpec += m_strTablePrefix + "lot.nb_parts_good)|SUM";
        m_strlQuery_Fields.append(strFieldSpec);
    }
    // Min and Max Start_t
    strFieldSpec = "Function=MinStart|";
    strFieldSpec += m_strTablePrefix + "splitlot.start_t|MIN";
    m_strlQuery_Fields.append(strFieldSpec);
    strFieldSpec = "Function=MaxStart|";
    strFieldSpec += m_strTablePrefix + "splitlot.start_t|MAX";
    m_strlQuery_Fields.append(strFieldSpec);
    // Add conditions
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "splitlot.valid_splitlot|NotString|N");
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "lot.tracking_lot_id|NotString|");
    // GCORE-1200: Checked [SC]
    // Filter on one product
    strFieldSpec = m_strTablePrefix + "lot.product_name|String|";
    strFieldSpec += pSerieDef->m_strProduct;
    m_strlQuery_ValueConditions.append(strFieldSpec);
    m_strlQuery_GroupFields.append(m_strTablePrefix + "lot.tracking_lot_id");
    m_strlQuery_GroupFields.append(m_strTablePrefix + "lot.lot_id");
    Query_AddFilters(cFilters);
    Query_BuildSqlString(strQuery, false);

    return true;
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_Yield_ConstructQuery_Core_Wafer(
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
        QString & strQuery,
        int nIndex)
{
    if(pSerieDef->m_bWorkOnBinnings)
        return ER_Genealogy_Yield_ConstructQuery_Core_Wafer_Binning(cFilters, pSerieDef, strQuery, nIndex);

    return ER_Genealogy_Yield_ConstructQuery_Core_Wafer_NoBinning(cFilters, pSerieDef, strQuery, nIndex);
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_Yield_ConstructQuery_Core_Wafer_Binning(
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
        QString & strQuery,
        int nIndex)
{
    QString			strSubQuery1, strSubQuery2;
    QString			strFieldSpec, strDbField, strDbTable;

    strQuery = "";

    // First part of the Join is the core query without binning specification
    // (some fields will be ignored, ie Matching)
    ER_Genealogy_Yield_ConstructQuery_Core_Wafer_NoBinning(cFilters, pSerieDef, strSubQuery1, nIndex);

    // Second part of the Join is the results corresponding to selected binnings
    Query_Empty();
    // Set Testing stage (default to FT)
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
    m_strTablePrefix = "ft_";
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = "et_";
    }
    else if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = "wt_";
    }
    // Lot field
    strFieldSpec = "Field=Lot|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "lot.lot_id");
    // Wafer field
    strFieldSpec = "Field=Wafer|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "wafer_sbin.wafer_id");
    // Matching parts field
    strFieldSpec = "Function=Matching|";
    strFieldSpec += m_strTablePrefix + "wafer_sbin.nb_parts|SUM";
    m_strlQuery_Fields.append(strFieldSpec);

    // Add conditions
    strFieldSpec = m_strTablePrefix + "lot.lot_id|";
    strFieldSpec += m_strTablePrefix + "wafer_sbin.lot_id";
    m_strlQuery_LinkConditions.append(strFieldSpec);
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "lot.tracking_lot_id|NotString|");
    // GCORE-1200: Checked [SC]
    // Filter on one product
    strFieldSpec = m_strTablePrefix + "lot.product_name|String|";
    strFieldSpec += pSerieDef->m_strProduct;
    m_strlQuery_ValueConditions.append(strFieldSpec);
    strFieldSpec = m_strTablePrefix + "wafer_sbin.sbin_no|Numeric|";
    strFieldSpec += pSerieDef->m_strBinnings;
    m_strlQuery_ValueConditions.append(strFieldSpec);
    m_strlQuery_GroupFields.append(m_strTablePrefix + "wafer_sbin.lot_id");
    m_strlQuery_GroupFields.append(m_strTablePrefix + "wafer_sbin.wafer_id");
    Query_AddFilters(cFilters);
    Query_BuildSqlString(strSubQuery2, false);

    // Build final query
    strFieldSpec = "T" + QString::number(nIndex);
    strQuery = "SELECT\n";
    strQuery += strFieldSpec + "_1.TrackingLot,\n";
    strQuery += strFieldSpec + "_1.Wafer,\n";
    strQuery += strFieldSpec + "_1.Parts as Parts,\n";
    strQuery += strFieldSpec + "_2.Matching as Matching,\n";
    strQuery += strFieldSpec + "_1.MinStart as MinStart,\n";
    strQuery += strFieldSpec + "_1.MaxStart as MaxStart\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += strSubQuery1;
    strQuery += ") " + strFieldSpec;
    strQuery += "_1\n";
    strQuery += "INNER JOIN\n";
    strQuery += "(\n";
    strQuery += strSubQuery2;
    strQuery += ") " + strFieldSpec;
    strQuery += "_2\n";
    strQuery += "ON " + strFieldSpec;
    strQuery += "_1.Lot=" + strFieldSpec;
    strQuery += "_2.Lot";
    strQuery += " AND " + strFieldSpec;
    strQuery += "_1.Wafer=" + strFieldSpec;
    strQuery += "_2.Wafer\n";

    return true;
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_Yield_ConstructQuery_Core_Wafer_NoBinning(
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
        QString & strQuery,
        int nIndex)
{
    QString			strSubQuery1, strSubQuery2;
    QString			strFieldSpec, strDbField, strDbTable;

    strQuery = "";

    // First part of the query is on Lot, Splitlot
    Query_Empty();

    // Set Testing stage (default to FT)
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
    m_strTablePrefix = "ft_";
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = "et_";
    }
    else if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = "wt_";
    }
    // Tracking Lot field
    strFieldSpec  = GEXDB_PLUGIN_DBFIELD_TRACKING_LOT;
    strFieldSpec += "=TrackingLot";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // Lot field
    strFieldSpec  = m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT];
    strFieldSpec += "=Lot";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // Min and Max Start_t
    strFieldSpec = "Function=MinStart|";
    strFieldSpec += m_strTablePrefix + "splitlot.start_t|MIN";
    m_strlQuery_Fields.append(strFieldSpec);
    strFieldSpec = "Function=MaxStart|";
    strFieldSpec += m_strTablePrefix + "splitlot.start_t|MAX";
    m_strlQuery_Fields.append(strFieldSpec);
    // Add conditions
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "splitlot.valid_splitlot|NotString|N");
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "lot.tracking_lot_id|NotString|");
    // GCORE-1200: Checked [SC]
    // Filter on one product
    strFieldSpec = m_strTablePrefix + "lot.product_name|String|";
    strFieldSpec += pSerieDef->m_strProduct;
    m_strlQuery_ValueConditions.append(strFieldSpec);
    m_strlQuery_GroupFields.append(m_strTablePrefix + "lot.tracking_lot_id");
    Query_AddFilters(cFilters);
    Query_BuildSqlString(strSubQuery1, false);

    // Second part of the Join is on Wafer
    Query_Empty();
    // Set Testing stage (default to FT)
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
    m_strTablePrefix = "ft_";
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = "et_";
    }
    else if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = "wt_";
    }
    // Lot field
    strFieldSpec = "Field=Lot|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.lot_id");
    // Wafer field
    strFieldSpec = "Field=Wafer|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.wafer_id");
    // Parts field
    strFieldSpec = "Field=Parts|" + m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.nb_parts");
    // Matching parts field
    if(pSerieDef->m_strBinnings.toLower().indexOf("fail") == -1)
    {
        strFieldSpec = "Field=Matching|" + m_strTablePrefix;
        m_strlQuery_Fields.append(strFieldSpec + "wafer_info.nb_parts_good");
    }
    else
    {
        strFieldSpec = "Expression=Matching";
        strFieldSpec += "|" + m_strTablePrefix;
        strFieldSpec += "wafer_info.nb_parts";
        strFieldSpec += "|(" + m_strTablePrefix;
        strFieldSpec += "wafer_info.nb_parts-";
        strFieldSpec += m_strTablePrefix + "wafer_info.nb_parts_good)";
        m_strlQuery_Fields.append(strFieldSpec);
    }

    // Add conditions
    strFieldSpec = m_strTablePrefix + "lot.lot_id|";
    strFieldSpec += m_strTablePrefix + "wafer_info.lot_id";
    m_strlQuery_LinkConditions.append(strFieldSpec);
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "lot.tracking_lot_id|NotString|");
    // GCORE-1200: Checked [SC]
    // Filter on one product
    strFieldSpec = m_strTablePrefix + "lot.product_name|String|";
    strFieldSpec += pSerieDef->m_strProduct;
    m_strlQuery_ValueConditions.append(strFieldSpec);
    m_strlQuery_GroupFields.append(m_strTablePrefix + "wafer_info.lot_id");
    m_strlQuery_GroupFields.append(m_strTablePrefix + "wafer_info.wafer_id");
    Query_AddFilters(cFilters);
    Query_BuildSqlString(strSubQuery2, false);

    // Build final query
    strFieldSpec = "T" + QString::number(nIndex);
    strQuery = "SELECT\n";
    strQuery += strFieldSpec + "_1_1.TrackingLot,\n";
    strQuery += strFieldSpec + "_1_1.Lot,\n";
    strQuery += strFieldSpec + "_1_2.Wafer,\n";
    strQuery += strFieldSpec + "_1_2.Parts as Parts,\n";
    strQuery += strFieldSpec + "_1_2.Matching as Matching,\n";
    strQuery += strFieldSpec + "_1_1.MinStart as MinStart,\n";
    strQuery += strFieldSpec + "_1_1.MaxStart as MaxStart\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += strSubQuery1;
    strQuery += ") " + strFieldSpec;
    strQuery += "_1_1\n";
    strQuery += "INNER JOIN\n";
    strQuery += "(\n";
    strQuery += strSubQuery2;
    strQuery += ") " + strFieldSpec;
    strQuery += "_1_2\n";
    strQuery += "ON " + strFieldSpec;
    strQuery += "_1_1.Lot=" + strFieldSpec;
    strQuery += "_1_2.Lot\n";

    return true;
}

//////////////////////////////////////////////////////////////////////
// Compute data for Enterprise Report graphs (Genealogy - Yield vs Parameter)
// (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_YieldVsParameter_GetParts(
        GexDbPlugin_Filter & cFilters,
        GexDbPlugin_ER_Parts & clER_PartsData)
{
    int errorMessageMaxSize=2048;

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate)
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Get E-Test parameter name (this is Ok only for single E-Test parameter)
    if(clER_PartsData.m_plistSerieDefs.count() > 0)
    {
        GexDbPlugin_Filter cEtestFilters = cFilters;	// Use a copy to overwrite testing stage with E-Test
        cEtestFilters.strDataTypeQuery = GEXDB_PLUGIN_GALAXY_ETEST;
        QStringList strlParameters;
        GexDbPlugin_ER_Parts_SerieDef *pSerieDef = clER_PartsData.m_plistSerieDefs.at(0);	// E-Test parameter is in serie 0
        if(QueryTestlist(cEtestFilters, strlParameters, true))
        {
            // Go through list and find matching E-Test parameter
            // Each test uses 3 elements in the list: test #, test name, test type (P,F,M)
            int		nIndex=0;
            bool	bFound=false;
            while(!bFound && (nIndex < strlParameters.size()))
            {
                QString strTestNb = strlParameters[nIndex];
                if(strTestNb == pSerieDef->m_strParameter)
                {
                    pSerieDef->m_strParameterName = strlParameters[nIndex+1];
                    bFound=true;
                }
                nIndex+=3;
            }
        }
    }

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // Construct SQL query for Yield
    QString strQuery;
    if(!ER_Genealogy_YieldVsParameter_ConstructQuery(cFilters, clER_PartsData, strQuery))
    {
        WriteDebugMessageFile("GexDbPlugin_Galaxy::ER_Genealogy_YieldVsParameter_GetParts: failed to construct query !");
        return false;
    }

    // Save query
    clER_PartsData.m_strYieldQuery = strQuery;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        WriteDebugMessageFile("GexDbPlugin_Galaxy::ER_Genealogy_YieldVsParameter_GetParts: query execution failed !");
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(errorMessageMaxSize).toLatin1().constData(), clGexDbQuery.lastError().text().left(errorMessageMaxSize).toLatin1().constData());
        return false;
    }

    // Fill Parts data object
    QString						strAggregate;
    QStringList					strlEmptySplit;
    int							nRows=0, nIndex;
    GexDbPlugin_ER_Parts_Graph	*pGraph=NULL;
    GexDbPlugin_ER_Parts_Layer	*pLayer=NULL;
    unsigned int				uiParameterExecs, uiTotalParts, uiMatchingParts;
    double						lfParameterSum;

    // Create new graph/layer
    pGraph = new GexDbPlugin_ER_Parts_Graph(strlEmptySplit);
    pLayer = new GexDbPlugin_ER_Parts_Layer(clER_PartsData.m_uiSeries, strlEmptySplit);

    while(clGexDbQuery.Next())
    {
        nRows++;

        // Get query data
        nIndex = 0;
        strlEmptySplit.clear();

        strAggregate = clGexDbQuery.value(nIndex++).toString();
        if(clER_PartsData.m_strGenealogy_Granularity.toLower() == "wafer")
            strAggregate += "/" + clGexDbQuery.value(nIndex++).toString();
        uiParameterExecs = clGexDbQuery.value(nIndex++).toUInt();
        lfParameterSum = clGexDbQuery.value(nIndex++).toDouble();
        uiTotalParts = clGexDbQuery.value(nIndex++).toUInt();
        uiMatchingParts = clGexDbQuery.value(nIndex++).toUInt();

        // Add data to layer
        pLayer->Add(strAggregate, uiParameterExecs, lfParameterSum, uiTotalParts, uiMatchingParts);
    }

    // Add last layer/graph
    pGraph->append(pLayer);
    clER_PartsData.append(pGraph);

    // Check if we have some results
    if(nRows == 0)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoResult_0, NULL);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts/parameter
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_YieldVsParameter_ConstructQuery(
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_ER_Parts & clER_PartsData,
        QString & strQuery)
{
    QString							strSubQuery_X, strSubQuery_Y;
    GexDbPlugin_ER_Parts_SerieDef	*pSerieDef;

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <fields>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields

    // Check if lot or wafer granularity
    if(clER_PartsData.m_strGenealogy_Granularity.toLower() == "wafer")
    {
        // 1. X-Axis
        pSerieDef = clER_PartsData.m_plistSerieDefs.at(0);
        ER_Genealogy_Mean_ConstructQuery_Core_Wafer(cFilters, pSerieDef, strSubQuery_X, 0);
        WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::ER_Genealogy_YieldVsParameter_ConstructQuery: strSubQuery_X= ").append(strSubQuery_X.replace('\n'," ")));

        // 2. Y-Axis
        pSerieDef = clER_PartsData.m_plistSerieDefs.at(1);
        ER_Genealogy_Yield_ConstructQuery_Core_Wafer(cFilters, pSerieDef, strSubQuery_Y, 1);
        WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::ER_Genealogy_YieldVsParameter_ConstructQuery: strSubQuery_Y= ").append(strSubQuery_Y.replace('\n'," ")));

        // 3. Build final query
        strQuery = "SELECT\n";
        strQuery += "T.TrackingLot,\n";
        strQuery += "T.Wafer,\n";
        strQuery += "T.ParameterExecs_X,\n";
        strQuery += "T.ParameterSum_X,\n";
        strQuery += "T.Parts_Y,\n";
        strQuery += "T.Matching_Y\n";
        strQuery += "FROM\n";
        strQuery += "(\n";
        strQuery += "SELECT\n";
        strQuery += "TX.TrackingLot AS TrackingLot,\n";
        strQuery += "TX.Wafer AS Wafer,\n";
        strQuery += "TX.ParameterExecs AS ParameterExecs_X,\n";
        strQuery += "TX.ParameterSum AS ParameterSum_X,\n";
        strQuery += "TX.MaxStart AS MaxStart_X,\n";
        strQuery += "TX.MinStart AS MinStart_X,\n";
        strQuery += "TY.Parts AS Parts_Y,\n";
        strQuery += "TY.Matching AS Matching_Y,\n";
        strQuery += "TY.MaxStart AS MaxStart_Y,\n";
        strQuery += "TY.MinStart AS MinStart_Y\n";
        strQuery += "FROM\n";
        strQuery += "(\n";
        strQuery += strSubQuery_X;
        strQuery += ") TX\n";
        strQuery += "INNER JOIN\n";
        strQuery += "(\n";
        strQuery += strSubQuery_Y;
        strQuery += ") TY\n";
        strQuery += "ON TX.TrackingLot=TY.TrackingLot AND TX.Wafer=TY.Wafer\n";
        strQuery += ") T\n";
        strQuery += "WHERE\n";
        strQuery += "(MaxStart_X >= " + QString::number(cFilters.tQueryFrom);
        strQuery += " AND MinStart_X <= " + QString::number(cFilters.tQueryTo);
        strQuery += ") OR\n";
        strQuery += "(MaxStart_Y >= " + QString::number(cFilters.tQueryFrom);
        strQuery += " AND MinStart_Y <= " + QString::number(cFilters.tQueryTo);
        strQuery += ")\n";
        strQuery += "ORDER BY TrackingLot, convert(Wafer, unsigned) ASC\n";
    }
    else
    {
        // 1. X-Axis
        pSerieDef = clER_PartsData.m_plistSerieDefs.at(0);
        ER_Genealogy_Mean_ConstructQuery_Core_Lot(cFilters, pSerieDef, strSubQuery_X, 0);

        // 2. Y-Axis
        pSerieDef = clER_PartsData.m_plistSerieDefs.at(1);
        ER_Genealogy_Yield_ConstructQuery_Core_Lot(cFilters, pSerieDef, strSubQuery_Y, 1);

        // 3. Build final query
        strQuery = "SELECT\n";
        strQuery += "T.TrackingLot,\n";
        strQuery += "T.ParameterExecs_X,\n";
        strQuery += "T.ParameterSum_X,\n";
        strQuery += "T.Parts_Y,\n";
        strQuery += "T.Matching_Y\n";
        strQuery += "FROM\n";
        strQuery += "(\n";
        strQuery += "SELECT\n";
        strQuery += "TX.TrackingLot AS TrackingLot,\n";
        strQuery += "TX.ParameterExecs AS ParameterExecs_X,\n";
        strQuery += "TX.ParameterSum AS ParameterSum_X,\n";
        strQuery += "TX.MaxStart AS MaxStart_X,\n";
        strQuery += "TX.MinStart AS MinStart_X,\n";
        strQuery += "TY.Parts AS Parts_Y,\n";
        strQuery += "TY.Matching AS Matching_Y,\n";
        strQuery += "TY.MaxStart AS MaxStart_Y,\n";
        strQuery += "TY.MinStart AS MinStart_Y\n";
        strQuery += "FROM\n";
        strQuery += "(\n";
        strQuery += strSubQuery_X;
        strQuery += ") TX\n";
        strQuery += "INNER JOIN\n";
        strQuery += "(\n";
        strQuery += strSubQuery_Y;
        strQuery += ") TY\n";
        strQuery += "ON TX.TrackingLot=TY.TrackingLot\n";
        strQuery += ") T\n";
        strQuery += "WHERE\n";
        strQuery += "(MaxStart_X >= " + QString::number(cFilters.tQueryFrom);
        strQuery += " AND MinStart_X <= " + QString::number(cFilters.tQueryTo);
        strQuery += ") OR\n";
        strQuery += "(MaxStart_Y >= " + QString::number(cFilters.tQueryFrom);
        strQuery += " AND MinStart_Y <= " + QString::number(cFilters.tQueryTo);
        strQuery += ")\n";
        strQuery += "ORDER BY TrackingLot ASC\n";
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts/parameter
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_Mean_ConstructQuery_Core_Lot(
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
        QString & strQuery,
        int nIndex)
{
    QString			strSubQuery1, strSubQuery2;
    QString			strFieldSpec, strDbField, strDbTable;

    strQuery = "";

    // Set Testing stage (default to FT)
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
    m_strTablePrefix = "ft_";
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = "et_";
    }
    else if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = "wt_";
    }

    // First part of the union is on samples
    if(pSerieDef->m_strTestingStage != GEXDB_PLUGIN_GALAXY_ETEST)
    {
        Query_Empty();
        // Tracking Lot field
        strFieldSpec  = GEXDB_PLUGIN_DBFIELD_TRACKING_LOT;
        strFieldSpec += "=TrackingLot";
        Query_AddField(strFieldSpec, strDbField, strDbTable);
        // Lot field
        strFieldSpec  = m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT];
        strFieldSpec += "=Lot";
        Query_AddField(strFieldSpec, strDbField, strDbTable);
        // Start_t
        strFieldSpec  = GEXDB_PLUGIN_DBFIELD_STARTDATETIME;
        strFieldSpec += "=Start";
        Query_AddField(strFieldSpec, strDbField, strDbTable);
        // NbExecs
        ER_Query_AddTestCount(pSerieDef, "stats_ptest_execs", "ParameterExecs", true, false);
        // Sum
        ER_Query_AddTestCount(pSerieDef, "stats_ptest_sum", "ParameterSum", true, false);
        // Add conditions
        m_strlQuery_ValueConditions.append(m_strTablePrefix + "splitlot.valid_splitlot|NotString|N");
        m_strlQuery_ValueConditions.append(m_strTablePrefix + "lot.tracking_lot_id|NotString|");
        // GCORE-1200: Checked [SC]
        // Filter on one product
        strFieldSpec = m_strTablePrefix + "lot.product_name|String|";
        strFieldSpec += pSerieDef->m_strProduct;
        m_strlQuery_ValueConditions.append(strFieldSpec);
        Query_AddFilters(cFilters);
        Query_BuildSqlString(strSubQuery1, false);
    }

    // Second part of the union is on summary
    Query_Empty();
    // Tracking Lot field
    strFieldSpec  = GEXDB_PLUGIN_DBFIELD_TRACKING_LOT;
    strFieldSpec += "=TrackingLot";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // Lot field
    strFieldSpec  = m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT];
    strFieldSpec += "=Lot";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // Start_t
    strFieldSpec  = GEXDB_PLUGIN_DBFIELD_STARTDATETIME;
    strFieldSpec += "=Start";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // NbExecs
    ER_Query_AddTestCount(pSerieDef, "stats_ptest_execs", "ParameterExecs", false, false);
    // Sum
    ER_Query_AddTestCount(pSerieDef, "stats_ptest_sum", "ParameterSum", false, false);
    // Add conditions
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "splitlot.valid_splitlot|NotString|N");
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "lot.tracking_lot_id|NotString|");
    // GCORE-1200: Checked [SC]
    // Filter on one product
    strFieldSpec = m_strTablePrefix + "lot.product_name|String|";
    strFieldSpec += pSerieDef->m_strProduct;
    m_strlQuery_ValueConditions.append(strFieldSpec);
    Query_AddFilters(cFilters);
    Query_BuildSqlString(strSubQuery2, false);

    // Build final query
    strFieldSpec = "T" + QString::number(nIndex);
    strQuery = "SELECT\n";
    strQuery += "TrackingLot,\n";
    strQuery += "Lot,\n";
    strQuery += "SUM(ParameterExecs) AS ParameterExecs,\n";
    strQuery += "SUM(ParameterSum) AS ParameterSum,\n";
    strQuery += "MIN(Start) AS MinStart,\n";
    strQuery += "MAX(Start) AS MaxStart\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        strQuery += strSubQuery2;
    else
    {
        strQuery += strSubQuery1;
        strQuery += ")\n";
        strQuery += "UNION\n";
        strQuery += "(\n";
        strQuery += strSubQuery2;
    }
    strQuery += ") " + strFieldSpec + "\n";
    strQuery += "GROUP BY TrackingLot, Lot\n";

    return true;
}

///////////////////////////////////////////////////////////
// Query: Construct query to retrieve all data for ER Production reports on parts/parameter
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ER_Genealogy_Mean_ConstructQuery_Core_Wafer(
        GexDbPlugin_Filter &cFilters,
        GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
        QString & strQuery, int nIndex)
{
    QString			strSubQuery1, strSubQuery2;
    QString			strFieldSpec, strDbField, strDbTable;

    strQuery = "";

    // Set Testing stage (default to FT)
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
    m_strTablePrefix = "ft_";
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = "et_";
    }
    else if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = "wt_";
    }

    // First part of the union is on samples
    if(pSerieDef->m_strTestingStage != GEXDB_PLUGIN_GALAXY_ETEST)
    {
        Query_Empty();
        // Tracking Lot field
        strFieldSpec  = GEXDB_PLUGIN_DBFIELD_TRACKING_LOT;
        strFieldSpec += "=TrackingLot";
        Query_AddField(strFieldSpec, strDbField, strDbTable);
        // Lot field
        strFieldSpec  = m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT];
        strFieldSpec += "=Lot";
        Query_AddField(strFieldSpec, strDbField, strDbTable);
        // Wafer field
        strFieldSpec  = m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID];
        strFieldSpec += "=Wafer";
        Query_AddField(strFieldSpec, strDbField, strDbTable);
        // Start_t
        strFieldSpec  = GEXDB_PLUGIN_DBFIELD_STARTDATETIME;
        strFieldSpec += "=Start";
        Query_AddField(strFieldSpec, strDbField, strDbTable);
        // NbExecs
        ER_Query_AddTestCount(pSerieDef, "stats_ptest_execs", "ParameterExecs", true, false);
        // Sum
        ER_Query_AddTestCount(pSerieDef, "stats_ptest_sum", "ParameterSum", true, false);
        // Add conditions
        m_strlQuery_ValueConditions.append(m_strTablePrefix + "splitlot.valid_splitlot|NotString|N");
        m_strlQuery_ValueConditions.append(m_strTablePrefix + "lot.tracking_lot_id|NotString|");
        // GCORE-1200: Checked [SC]
        // Filter on one product
        strFieldSpec = m_strTablePrefix + "lot.product_name|String|";
        strFieldSpec += pSerieDef->m_strProduct;
        m_strlQuery_ValueConditions.append(strFieldSpec);
        Query_AddFilters(cFilters);
        Query_BuildSqlString(strSubQuery1, false);
    }

    // Second part of the union is on summary
    Query_Empty();
    // Tracking Lot field
    strFieldSpec  = GEXDB_PLUGIN_DBFIELD_TRACKING_LOT;
    strFieldSpec += "=TrackingLot";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // Lot field
    strFieldSpec  = m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT];
    strFieldSpec += "=Lot";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // Wafer field
    strFieldSpec  = m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID];
    strFieldSpec += "=Wafer";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // Start_t
    strFieldSpec  = GEXDB_PLUGIN_DBFIELD_STARTDATETIME;
    strFieldSpec += "=Start";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    // NbExecs
    ER_Query_AddTestCount(pSerieDef, "stats_ptest_execs", "ParameterExecs", false, false);
    // Sum
    ER_Query_AddTestCount(pSerieDef, "stats_ptest_sum", "ParameterSum", false, false);
    // Add conditions
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "splitlot.valid_splitlot|NotString|N");
    m_strlQuery_ValueConditions.append(m_strTablePrefix + "lot.tracking_lot_id|NotString|");
    // GCORE-1200: Checked [SC]
    // Filter on one product
    strFieldSpec = m_strTablePrefix + "lot.product_name|String|";
    strFieldSpec += pSerieDef->m_strProduct;
    m_strlQuery_ValueConditions.append(strFieldSpec);
    Query_AddFilters(cFilters);
    Query_BuildSqlString(strSubQuery2, false);
    GSLOG(SYSLOG_SEV_DEBUG, QString("ER Genealogy_Mean_ConstructQuery_Core_Wafer: strSubQuery2= ")
                          .append(strSubQuery2.replace('\n', " ")).toLatin1().data() );

    // Build final query
    strFieldSpec = "T" + QString::number(nIndex);
    strQuery = "SELECT\n";
    strQuery += "TrackingLot,\n";
    strQuery += "Lot,\n";
    strQuery += "Wafer,\n";
    strQuery += "SUM(ParameterExecs) AS ParameterExecs,\n";
    strQuery += "SUM(ParameterSum) AS ParameterSum,\n";
    strQuery += "MIN(Start) AS MinStart,\n";
    strQuery += "MAX(Start) AS MaxStart\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    if(pSerieDef->m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        strQuery += strSubQuery2;
    else
    {
        strQuery += strSubQuery1;
        strQuery += ")\n";
        strQuery += "UNION\n";
        strQuery += "(\n";
        strQuery += strSubQuery2;
    }
    strQuery += ") " + strFieldSpec + "\n";
    strQuery += "GROUP BY TrackingLot, Lot, Wafer\n";

    return true;
}

//////////////////////////////////////////////////////////////////////
// Computes data for Advanced Enterprise Report
// (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::AER_GetDataset(const GexDbPluginERDatasetSettings& datasetSettings,
                                        GexDbPluginERDataset& datasetResult)
{
    // Trace message
    GSLOG(SYSLOG_SEV_DEBUG, "AER_GetDataset");

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate)
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    GexDbPluginSQL::GexDbSQLQuery		gexdbSQLQuery(m_pclDatabaseConnector);
    GexDbPluginSQL::GexDbSQLQuery		gexdbSQLQueryOverall(m_pclDatabaseConnector);
    GexDbPluginSQL::GexDbSQLQuery		gexdbSQLQueryEmptyFullTimeLine(m_pclDatabaseConnector);

    // If at least one serie
    if (datasetSettings.series().count() > 0)
    {
        // build the query
        if (AER_buildQuery(datasetSettings, gexdbSQLQuery, false) == false)
            return false;

        // execute the query and get results
        if (AER_executeQuery(datasetSettings, gexdbSQLQuery, datasetResult, false) == false)
            return false;
    }

    // If need an overall query or no serie defined
    if (datasetSettings.overall() || datasetSettings.series().count() == 0)
    {
        // Build an Overall query
        if (AER_buildQuery(datasetSettings, gexdbSQLQueryOverall, true) == false)
            return false;

        // Execute the query and get results
        if (AER_executeQuery(datasetSettings, gexdbSQLQueryOverall, datasetResult, true) == false)
            return false;
    }

    // Check if we have some results
    if(datasetResult.groups().count() == 0)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoResult_0, NULL);
        return false;
    }

    if (datasetSettings.fullTimeLine())
    {
        bool bFullTimeLineCompatible = true;

        // Build an query to get empty aggregate
        if (AER_buildFullTimeLineQuery(datasetSettings, gexdbSQLQueryEmptyFullTimeLine, bFullTimeLineCompatible) == false)
            return false;

        if (bFullTimeLineCompatible)
        {
            // Execute the query and get results
            if (AER_executeFullTimeLineQuery(datasetSettings, gexdbSQLQueryEmptyFullTimeLine, datasetResult) == false)
                return false;
        }
    }

    // Update Order Field label with Database Field Name
    QString strClause;
    QString strField;
    QString strOrderBy;
    bool	bIsBinning = false;

    for (int nOrder = 0; nOrder < datasetSettings.orders().count(); ++nOrder)
    {
        strField	= datasetSettings.orders().at(nOrder).section("|", 0, 0);
        strClause	= datasetSettings.orders().at(nOrder).section("|", 1, 1).toLower();

        // Group is already added to the sql query
        if (datasetSettings.isGroupField(strField) == false)
        {
            // Get field name from metadata table
            GetConsolidatedFieldName(datasetSettings.testingStage(), strField, strField, NULL, &bIsBinning);

            if (strClause.isEmpty() == false)
                strOrderBy = strField + "|" + strClause;
            else
                strOrderBy = strField;

            datasetResult.addOrderBy(strOrderBy);
        }
    }

    datasetResult.applyPostProcessing(datasetSettings);

    return true;
}

bool GexDbPlugin_Galaxy::AER_buildQuery(const GexDbPluginERDatasetSettings &datasetSettings, GexDbPluginSQL::GexDbSQLQuery &gexdbSQLQuery, bool bOverall)
{
    QList<GexDbPluginSQL::GexDbSQLQuery>    queries;
    GexDbPluginSQL::GexDbSQLQuery           gexdbStandardQuery(m_pclDatabaseConnector);
    GexDbPluginSQL::GexDbSQLQuery           gexdbIntermediateQuery(m_pclDatabaseConnector);

    // Build query for physical consolidated data (Standard consolidated data)
    if (AER_buildSubQuery(datasetSettings, gexdbStandardQuery, bOverall, false) == false)
        return false;

    queries.append(gexdbStandardQuery);

    // Build query for intermediate consolidated data
    if (datasetSettings.useIntermediateConsolidatedData())
    {
        if (AER_buildSubQuery(datasetSettings, gexdbIntermediateQuery, bOverall, true) == false)
            return false;

        queries.append(gexdbIntermediateQuery);
    }

    // Generates the whole query, including union between multiple subqueries.
    for (int idx = 0; idx < queries.count(); idx++)
    {
        if (idx == 0)
            gexdbSQLQuery = queries.at(idx);
        else
            gexdbSQLQuery.addUnion(queries.at(idx));
    }

    return true;
}

bool GexDbPlugin_Galaxy::AER_buildSubQuery(const GexDbPluginERDatasetSettings &datasetSettings,
                                           GexDbPluginSQL::GexDbSQLQuery &gexdbSQLQuery, bool bOverall, bool bIntermediate)
{
    GexDbPluginSQL::GexDbSQLTable	gexdbSQLTable;
    GexDbPluginSQL::GexDbSQLTable	gexdbTableProductBin;
    GexDbPluginSQL::GexDbSQLField	gexdbSQLField;
    GexDbPluginSQL::GexDbSQLField	gexdbFieldBinVolume;
    GexDbPluginSQL::GexDbSQLField	gexdbFieldTPBBinNo;
    GexDbPluginSQL::GexDbSQLField	gexdbFieldTPBBinCat;
    GexDbPluginSQL::GexDbSQLField	gexdbFieldTPBBinName;

    QString strConsolidatedTable;
    QString strSuffixTable;

    // Add suffix to consolidated table for Wafer Sort level when querying Intermediate consolidated data
    if ((datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST ||
         datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST) && bIntermediate)
        strSuffixTable = "_inter";

    // Initialize table object
    if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST)
        strConsolidatedTable = "wt_consolidated_wafer";
    else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST)
        strConsolidatedTable = "ft_consolidated_sublot";
    else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_AZ)
        strConsolidatedTable = "az_consolidated_tl";
    else
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_InvalidTestingStage, NULL, datasetSettings.testingStage().toLatin1().constData());
        ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
        return false;
    }

    strConsolidatedTable += strSuffixTable;
    gexdbSQLTable = GexDbPluginSQL::GexDbSQLTable(strConsolidatedTable, "CT", m_pclDatabaseConnector->m_strSchemaName);

    // Query is based on binning, add the join to the query
    if (datasetSettings.binType() != GexDbPluginERDatasetSettings::NoBin)
    {
        if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST || datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST)
        {
            GexDbPluginSQL::GexDbSQLTable	gexdbTableWaferBin;
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTWBBinNo;

            if (datasetSettings.binType() == GexDbPluginERDatasetSettings::HardBin)
            {
                if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST)
                    gexdbTableWaferBin		= GexDbPluginSQL::GexDbSQLTable("wt_wafer_hbin"+strSuffixTable, "TWB",
                                                                            m_pclDatabaseConnector->m_strSchemaName);
                else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST)
                    gexdbTableWaferBin		= GexDbPluginSQL::GexDbSQLTable("ft_sublot_hbin"+strSuffixTable, "TLB",
                                                                            m_pclDatabaseConnector->m_strSchemaName);

                gexdbFieldTWBBinNo		= GexDbPluginSQL::GexDbSQLField("hbin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableWaferBin);
            }
            else if (datasetSettings.binType() == GexDbPluginERDatasetSettings::SoftBin)
            {
                if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST)
                    gexdbTableWaferBin		= GexDbPluginSQL::GexDbSQLTable("wt_wafer_sbin"+strSuffixTable, "TWB",
                                                                            m_pclDatabaseConnector->m_strSchemaName);
                else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST)
                    gexdbTableWaferBin		= GexDbPluginSQL::GexDbSQLTable("ft_sublot_sbin"+strSuffixTable, "TLB",
                                                                            m_pclDatabaseConnector->m_strSchemaName);

                gexdbFieldTWBBinNo		= GexDbPluginSQL::GexDbSQLField("sbin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableWaferBin);
            }
            else
                return false;

            GexDbPluginSQL::GexDbSQLQuery	gexdbQueryProductBin(m_pclDatabaseConnector);

            if (AER_buildQueryProductBinning(datasetSettings, gexdbQueryProductBin, bIntermediate) == false)
                return false;

            gexdbTableProductBin = GexDbPluginSQL::GexDbSQLTable(gexdbQueryProductBin, "TPBin");

            // define binning fields
            gexdbFieldTPBBinNo		= GexDbPluginSQL::GexDbSQLField("bin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_no", gexdbTableProductBin, true);
            gexdbFieldTPBBinCat		= GexDbPluginSQL::GexDbSQLField("bin_cat", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_cat", gexdbTableProductBin);
            gexdbFieldTPBBinName	= GexDbPluginSQL::GexDbSQLField("bin_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_name", gexdbTableProductBin);
            gexdbFieldBinVolume		= GexDbPluginSQL::GexDbSQLField("nb_parts", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_volume", gexdbTableWaferBin, true);
            gexdbFieldBinVolume.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionSum);

            // Left outer join between xx_wafer_xbin and xx_product_xbin
            GexDbPluginSQL::GexDbSQLJoin	sqlJoinTPB_TCW(gexdbTableProductBin, GexDbPluginSQL::GexDbSQLJoin::SqlLeftOuterJoin);
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTPBProduct("product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "product_name", gexdbTableProductBin);
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTCWProduct("product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "product_name", gexdbSQLTable);

            sqlJoinTPB_TCW.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbFieldTPBProduct, gexdbFieldTCWProduct));
            gexdbSQLQuery.addJoin(sqlJoinTPB_TCW);

            // Left outer join between xx_wafer_xbin and xx_product_xbin
            GexDbPluginSQL::GexDbSQLJoin	sqlJoinTWB_TCW(gexdbTableWaferBin, GexDbPluginSQL::GexDbSQLJoin::SqlLeftOuterJoin);
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTWBLotId("lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableWaferBin);
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTCWLotId("lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbSQLTable);

            sqlJoinTWB_TCW.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbFieldTWBLotId, gexdbFieldTCWLotId));

            if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST)
            {
                GexDbPluginSQL::GexDbSQLField	gexdbFieldTWBWaferId("wafer_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableWaferBin);
                GexDbPluginSQL::GexDbSQLField	gexdbFieldTCWWaferId("wafer_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbSQLTable);

                sqlJoinTWB_TCW.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbFieldTWBWaferId, gexdbFieldTCWWaferId));

                GexDbPluginSQL::GexDbSQLField	gexdbFieldTWBConsolidationName("consolidation_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableWaferBin);
                GexDbPluginSQL::GexDbSQLField	gexdbFieldTCWConsolidationName("consolidation_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbSQLTable);

                // On intermediate table, add a join condition on consolidation_name field
                if (bIntermediate)
                    sqlJoinTWB_TCW.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbFieldTWBConsolidationName, gexdbFieldTCWConsolidationName));
            }
            else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST)
            {
                GexDbPluginSQL::GexDbSQLField	gexdbFieldTWBSublotId("sublot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableWaferBin);
                GexDbPluginSQL::GexDbSQLField	gexdbFieldTCLSublotId("sublot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbSQLTable);

                sqlJoinTWB_TCW.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbFieldTWBSublotId, gexdbFieldTCLSublotId));

                GexDbPluginSQL::GexDbSQLField	gexdbFieldTWBConsolidationName("consolidation_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableWaferBin);
                GexDbPluginSQL::GexDbSQLField	gexdbFieldTCLConsolidationName("consolidation_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbSQLTable);

                // On intermediate table, add a join condition on consolidation_name field
                if (bIntermediate)
                    sqlJoinTWB_TCW.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbFieldTWBConsolidationName, gexdbFieldTCLConsolidationName));
            }

            sqlJoinTWB_TCW.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbFieldTPBBinNo, gexdbFieldTWBBinNo));
            gexdbSQLQuery.addJoin(sqlJoinTWB_TCW);

            gexdbSQLQuery.addField(gexdbFieldTPBBinNo);
            gexdbSQLQuery.addField(gexdbFieldTPBBinCat);
            gexdbSQLQuery.addField(gexdbFieldTPBBinName);
            gexdbSQLQuery.addField(gexdbFieldBinVolume);
        }
        else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_AZ)
        {
            GexDbPluginSQL::GexDbSQLQuery gexdbAZQuery(m_pclDatabaseConnector);

            if (AER_buildAZQueryBinning(datasetSettings, gexdbAZQuery) == false)
                return false;

            gexdbSQLTable = GexDbPluginSQL::GexDbSQLTable(gexdbAZQuery, "azbin");

            // Add sum of bin_volume
            gexdbSQLField = GexDbPluginSQL::GexDbSQLField("bin_volume", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_volume", gexdbSQLTable);
            gexdbSQLField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionSum);
            gexdbSQLQuery.addField(gexdbSQLField);

            gexdbSQLQuery.addField(GexDbPluginSQL::GexDbSQLField("bin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_no", gexdbSQLTable));
            gexdbSQLQuery.addField(GexDbPluginSQL::GexDbSQLField("bin_cat", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_cat", gexdbSQLTable));
            gexdbSQLQuery.addField(GexDbPluginSQL::GexDbSQLField("bin_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_name", gexdbSQLTable));
        }
    }

    // Add madatory fields like parts, parts good and gross die
    // Add sum of nb_parts_good
    gexdbSQLField = GexDbPluginSQL::GexDbSQLField("nb_parts_good", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "good_volume", gexdbSQLTable);
    gexdbSQLField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionSum);
    gexdbSQLQuery.addField(gexdbSQLField);

    // Add sum of nb_parts
    gexdbSQLField = GexDbPluginSQL::GexDbSQLField("nb_parts", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "volume", gexdbSQLTable);
    gexdbSQLField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionSum);
    gexdbSQLQuery.addField(gexdbSQLField);

    // Add start time
    gexdbSQLField = GexDbPluginSQL::GexDbSQLField("start_t", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "start_time", gexdbSQLTable);
    gexdbSQLField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionMax);
    gexdbSQLQuery.addField(gexdbSQLField);

    // Add sum of gross die (only Wafer Sort) and wafer count
    if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        gexdbSQLField = GexDbPluginSQL::GexDbSQLField("gross_die", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "gross_volume", gexdbSQLTable);
        gexdbSQLField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionSum);
        gexdbSQLQuery.addField(gexdbSQLField);

        gexdbSQLField = GexDbPluginSQL::GexDbSQLField("1", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "wafer_count", "");
        gexdbSQLField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionSum);
        gexdbSQLQuery.addField(gexdbSQLField);
    }

    if (AER_fillQueryField(datasetSettings, gexdbSQLQuery, bOverall, gexdbSQLTable, gexdbTableProductBin))
    {
        // Add table to the query
        gexdbSQLQuery.addTable(gexdbSQLTable);

        return true;
    }

    return false;
}

bool GexDbPlugin_Galaxy::AER_fillQueryField(const GexDbPluginERDatasetSettings& datasetSettings, GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery, bool bOverall, const GexDbPluginSQL::GexDbSQLTable& mainTable, const GexDbPluginSQL::GexDbSQLTable& binTable)
{
    GexDbPluginSQL::GexDbSQLField	gexdbSQLField;
    QString                         lOrderDirection;
    bool							bIsBinningField	= false;

    // Add group by
    for (int nGroup = 0; nGroup < datasetSettings.groups().count(); ++nGroup)
    {
        if (datasetSettings.groups().at(nGroup).isEmpty() == false)
        {
            QString strField;
            bool	bIsNumeric = false;
            bool	bIsBinning = false;
            if (GetConsolidatedFieldName(datasetSettings.testingStage(), datasetSettings.groups().at(nGroup), strField, &bIsNumeric, &bIsBinning))
            {
                if (bIsBinning)
                    gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", binTable, bIsNumeric);
                else
                    gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", mainTable, bIsNumeric);
            }
            else
            {
                GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, datasetSettings.groups().at(nGroup).toLatin1().constData());
                return false;
            }

            bIsBinningField |= bIsBinning;

            gexdbSQLQuery.addField(gexdbSQLField);
            gexdbSQLQuery.addGroupBy(gexdbSQLField);

            GexDbPluginSQL::GexDbSQLQuery::sqlOrderBy lOrderBy = GexDbPluginSQL::GexDbSQLQuery::SqlOrderByAsc;
            if (datasetSettings.isOrderedField(datasetSettings.groups().at(nGroup), lOrderDirection))
            {
                if (lOrderDirection.toLower() == "desc")
                {
                    lOrderBy = GexDbPluginSQL::GexDbSQLQuery::SqlOrderByDesc;
                }
            }

            gexdbSQLQuery.addOrderBy(gexdbSQLField, lOrderBy);
        }
    }

    // Add serie to fields and group by
    if (bOverall)
    {
        gexdbSQLField = GexDbPluginSQL::GexDbSQLField("Overall", GexDbPluginSQL::GexDbSQLField::SqlFieldText, "Overall", mainTable);
        gexdbSQLQuery.addField(gexdbSQLField);
    }
    else
    {
        for (int nSerie = 0; nSerie < datasetSettings.series().count(); ++nSerie)
        {
            QString strField;
            bool	bIsNumeric = false;
            bool	bIsBinning = false;


            if (GetConsolidatedFieldName(datasetSettings.testingStage(), datasetSettings.series().at(nSerie), strField, &bIsNumeric, &bIsBinning))
            {
                if (bIsBinning)
                    gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", binTable, bIsNumeric);
                else
                    gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", mainTable, bIsNumeric);
            }
            else
            {
                GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, datasetSettings.series().at(nSerie).toLatin1().constData());
                return false;
            }

            bIsBinningField |= bIsBinning;

            gexdbSQLQuery.addField(gexdbSQLField);
            gexdbSQLQuery.addGroupBy(gexdbSQLField);

            GexDbPluginSQL::GexDbSQLQuery::sqlOrderBy lOrderBy = GexDbPluginSQL::GexDbSQLQuery::SqlOrderByAsc;
            if (datasetSettings.isOrderedField(datasetSettings.series().at(nSerie), lOrderDirection))
            {
                if (lOrderDirection.toLower() == "desc")
                {
                    lOrderBy = GexDbPluginSQL::GexDbSQLQuery::SqlOrderByDesc;
                }
            }

            gexdbSQLQuery.addOrderBy(gexdbSQLField, lOrderBy);
        }
    }

    // Add aggregate to fields and group by
    for (int nAggregate = 0; nAggregate < datasetSettings.aggregates().count(); ++nAggregate)
    {
        QString strField;
        bool	bIsNumeric = false;
        bool	bIsBinning = false;

        if (GetConsolidatedFieldName(datasetSettings.testingStage(), datasetSettings.aggregates().at(nAggregate), strField, &bIsNumeric, &bIsBinning))
        {
            if (bIsBinning)
                gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", binTable, bIsNumeric);
            else
                gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", mainTable, bIsNumeric);
        }
        else
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, datasetSettings.aggregates().at(nAggregate).toLatin1().constData());
            return false;
        }

        bIsBinningField |= bIsBinning;

        gexdbSQLQuery.addField(gexdbSQLField);
        gexdbSQLQuery.addGroupBy(gexdbSQLField);

        GexDbPluginSQL::GexDbSQLQuery::sqlOrderBy lOrderBy = GexDbPluginSQL::GexDbSQLQuery::SqlOrderByAsc;
        if (datasetSettings.isOrderedField(datasetSettings.aggregates().at(nAggregate), lOrderDirection))
        {
            if (lOrderDirection.toLower() == "desc")
            {
                lOrderBy = GexDbPluginSQL::GexDbSQLQuery::SqlOrderByDesc;
            }
        }

        gexdbSQLQuery.addOrderBy(gexdbSQLField, lOrderBy);
    }

    // Add time period condition
    if (datasetSettings.timePeriod() != GEX_QUERY_TIMEPERIOD_ALLDATES)
    {
        gexdbSQLField = GexDbPluginSQL::GexDbSQLField("start_t", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", mainTable);
        gexdbSQLQuery.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbSQLField, datasetSettings.timestampFrom(), datasetSettings.timestampTo()));
    }

    // Add all filters
    for (int nFilter = 0; nFilter < datasetSettings.filters().count(); ++nFilter)
    {
        GexDbPluginERDatasetSettings::GexDbPluginERDatasetPrivateFilter filter = datasetSettings.filters().at(nFilter);

        // Add filter if
        //		No Overall Overall is not set
        // Or	Filter is not based on the serie
        if (bOverall == false || datasetSettings.series().contains(filter.field()) == false)
        {
            GexDbPluginSQL::GexDbSQLCondition::sqlCondition eCondition	= GexDbPluginSQL::GexDbSQLCondition::sqlConditionStringEqual;

            QString strField;
            bool	bIsNumeric = false;
            bool	bIsBinning = false;

            // Get field name from metadata table
            if (GetConsolidatedFieldName(datasetSettings.testingStage(), filter.field(), strField, &bIsNumeric, &bIsBinning))
            {
                if (bIsBinning)
                    gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", binTable, bIsNumeric);
                else
                    gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", mainTable, bIsNumeric);
            }
            else
            {
                GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, filter.field().toLatin1().constData());
                return false;
            }

            if (filter.filterOperator() == GexDbPluginERDatasetSettings::OpNotEqual)
                eCondition = GexDbPluginSQL::GexDbSQLCondition::sqlConditionStringNotEqual;

            QStringList lstValue = filter.value().split("|");

            if (lstValue.count() > 0)
            {
                GexDbPluginSQL::GexDbSQLCondition gexSQLCondition(gexdbSQLField, lstValue.at(0), eCondition);

                for (int nValue = 1; nValue < lstValue.count(); ++nValue)
                    gexSQLCondition.addCondition(gexdbSQLField, lstValue.at(nValue), eCondition);

                gexdbSQLQuery.addCondition(gexSQLCondition);
            }

            if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_AZ)
                gexdbSQLQuery.addField(gexdbSQLField);
        }
    }

    if (datasetSettings.testingStage() != GEXDB_PLUGIN_GALAXY_AZ)
    {
        // Add all other fields available from the GUI
        QStringList lstRdbFields;
        GetRdbFieldsList(datasetSettings.testingStage(), lstRdbFields, "Y", "N", "*" , "*", "Y", "N");

        QString strField;
        bool	bIsNumeric = false;
        for(int nField = 0; nField < lstRdbFields.count(); nField++)
        {
            // Get field name from metadata table
            if (GetConsolidatedFieldName(datasetSettings.testingStage(), lstRdbFields.at(nField), strField, &bIsNumeric))
                gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", mainTable, bIsNumeric);
            else
            {
                GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, lstRdbFields.at(nField).toLatin1().constData());
                return false;
            }

            gexdbSQLQuery.addField(gexdbSQLField);
        }
    }

    // If Query on binning, group by bin_no unless bin_no is already parts of Group, aggregate or serie
    if (datasetSettings.binType() != GexDbPluginERDatasetSettings::NoBin && bIsBinningField == false)
        gexdbSQLQuery.addGroupBy(GexDbPluginSQL::GexDbSQLField("bin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_no", binTable));

    return true;
}

bool GexDbPlugin_Galaxy::AER_buildQueryProductBinning(const GexDbPluginERDatasetSettings& datasetSettings,
                                                      GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery,
                                                      bool bIntermediate)
{
    QString strSuffixTable;

    // Add suffix to consolidated table for Wafer Sort level when querying Intermediate consolidated data
    if ((datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST ||
         datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST) && bIntermediate)
        strSuffixTable = "_inter";

    // Query is based on binning, add the join to the query
    if (datasetSettings.binType() != GexDbPluginERDatasetSettings::NoBin)
    {
        if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST || datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST)
        {
            GexDbPluginSQL::GexDbSQLTable	gexdbTableConsolidated;
            GexDbPluginSQL::GexDbSQLTable	gexdbTableProductBin;
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTPBProductName;
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTPBBinNo;
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTPBBinCat;
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTPBBinName;
            GexDbPluginSQL::GexDbSQLField	gexdbSQLField;

            if (datasetSettings.binType() == GexDbPluginERDatasetSettings::HardBin)
            {
                if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST)
                    gexdbTableProductBin	= GexDbPluginSQL::GexDbSQLTable("wt_product_hbin", "TPB",
                                                                            m_pclDatabaseConnector->m_strSchemaName);
                else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST)
                    gexdbTableProductBin	= GexDbPluginSQL::GexDbSQLTable("ft_product_hbin", "TPB",
                                                                            m_pclDatabaseConnector->m_strSchemaName);
            }
            else if (datasetSettings.binType() == GexDbPluginERDatasetSettings::SoftBin)
            {
                if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST)
                    gexdbTableProductBin	= GexDbPluginSQL::GexDbSQLTable("wt_product_sbin", "TPB",
                                                                            m_pclDatabaseConnector->m_strSchemaName);
                else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST)
                    gexdbTableProductBin	= GexDbPluginSQL::GexDbSQLTable("ft_product_sbin", "TPB",
                                                                            m_pclDatabaseConnector->m_strSchemaName);
            }
            else
                return false;

            // Initialize table object
            if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST)
                gexdbTableConsolidated = GexDbPluginSQL::GexDbSQLTable("wt_consolidated_wafer"+strSuffixTable, "TCW",
                                                                       m_pclDatabaseConnector->m_strSchemaName);
            else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST)
                gexdbTableConsolidated = GexDbPluginSQL::GexDbSQLTable("ft_consolidated_sublot"+strSuffixTable, "TCL",
                                                                       m_pclDatabaseConnector->m_strSchemaName);
            else
                return false;

            gexdbFieldTPBProductName= GexDbPluginSQL::GexDbSQLField("product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "product_name", gexdbTableProductBin);
            gexdbFieldTPBBinNo		= GexDbPluginSQL::GexDbSQLField("bin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_no", gexdbTableProductBin, true);
            gexdbFieldTPBBinCat		= GexDbPluginSQL::GexDbSQLField("bin_cat", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_cat", gexdbTableProductBin);
            gexdbFieldTPBBinName	= GexDbPluginSQL::GexDbSQLField("bin_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_name", gexdbTableProductBin);

            gexdbSQLQuery.addField(gexdbFieldTPBProductName);
            gexdbSQLQuery.addField(gexdbFieldTPBBinNo);
            gexdbSQLQuery.addField(gexdbFieldTPBBinCat);
            gexdbSQLQuery.addField(gexdbFieldTPBBinName);

            // Left outer join between xx_wafer_xbin and xx_product_xbin
            GexDbPluginSQL::GexDbSQLJoin	sqlJoinTPB_TCW(gexdbTableProductBin, GexDbPluginSQL::GexDbSQLJoin::SqlLeftOuterJoin);
            // GCORE-1200: Checked [SC]
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTPBProduct("product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableProductBin);
            GexDbPluginSQL::GexDbSQLField	gexdbFieldTCWProduct("product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableConsolidated);

            sqlJoinTPB_TCW.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbFieldTPBProduct, gexdbFieldTCWProduct));
            gexdbSQLQuery.addJoin(sqlJoinTPB_TCW);

            // Add time period condition
            if (datasetSettings.timePeriod() != GEX_QUERY_TIMEPERIOD_ALLDATES)
            {
                gexdbSQLField = GexDbPluginSQL::GexDbSQLField("start_t", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableConsolidated);
                gexdbSQLQuery.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbSQLField, datasetSettings.timestampFrom(), datasetSettings.timestampTo()));
            }

            // Add all filters
            for (int nFilter = 0; nFilter < datasetSettings.filters().count(); ++nFilter)
            {
                GexDbPluginERDatasetSettings::GexDbPluginERDatasetPrivateFilter filter = datasetSettings.filters().at(nFilter);

                GexDbPluginSQL::GexDbSQLCondition::sqlCondition eCondition	= GexDbPluginSQL::GexDbSQLCondition::sqlConditionStringEqual;

                QString strField;
                bool	bIsNumeric = false;
                bool	bIsBinning = false;

                // Get field name from metadata table
                if (GetConsolidatedFieldName(datasetSettings.testingStage(), filter.field(), strField, &bIsNumeric, &bIsBinning))
                {
                    if (bIsBinning)
                        gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableProductBin, bIsNumeric);
                    else
                        gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbTableConsolidated, bIsNumeric);
                }
                else
                {
                    GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, filter.field().toLatin1().constData());
                    return false;
                }

                if (filter.filterOperator() == GexDbPluginERDatasetSettings::OpNotEqual)
                    eCondition = GexDbPluginSQL::GexDbSQLCondition::sqlConditionStringNotEqual;

                QStringList lstValue = filter.value().split("|");

                if (lstValue.count() > 0)
                {
                    GexDbPluginSQL::GexDbSQLCondition gexSQLCondition(gexdbSQLField, lstValue.at(0), eCondition);

                    for (int nValue = 1; nValue < lstValue.count(); ++nValue)
                        gexSQLCondition.addCondition(gexdbSQLField, lstValue.at(nValue), eCondition);

                    gexdbSQLQuery.addCondition(gexSQLCondition);
                }
            }

            gexdbSQLQuery.addTable(gexdbTableConsolidated);
            gexdbSQLQuery.addGroupBy(gexdbFieldTPBProductName);
            gexdbSQLQuery.addGroupBy(gexdbFieldTPBBinNo);
        }
        else
            return false;
    }
    else
        return false;

    return true;
}

bool GexDbPlugin_Galaxy::AER_buildAZQueryBinning(const GexDbPluginERDatasetSettings& datasetSettings, GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery)
{
    GexDbPluginSQL::GexDbSQLTable		sqlTableAzConsolidatedTrackingLot("az_consolidated_tl", "TAZCTL", m_pclDatabaseConnector->m_strSchemaName);
    GexDbPluginSQL::GexDbSQLTable		sqlTableFtLot("ft_lot", "TFTL", m_pclDatabaseConnector->m_strSchemaName);
    GexDbPluginSQL::GexDbSQLTable		sqlTableFtDieTracking("ft_die_tracking", "", m_pclDatabaseConnector->m_strSchemaName);
    GexDbPluginSQL::GexDbSQLTable		sqlTableFtProductBin;
    GexDbPluginSQL::GexDbSQLTable		sqlTableFtSubLotBin;
    GexDbPluginSQL::GexDbSQLTable		sqlTableWtProductBin;
    GexDbPluginSQL::GexDbSQLTable		sqlTableWtWaferBin;
    GexDbPluginSQL::GexDbSQLField		sqlField;
    QString								strFieldBinNo;

    if (datasetSettings.binType() == GexDbPluginERDatasetSettings::HardBin)
    {
        sqlTableFtProductBin	= GexDbPluginSQL::GexDbSQLTable("ft_product_hbin", "TFTPB", m_pclDatabaseConnector->m_strSchemaName);
        sqlTableFtSubLotBin		= GexDbPluginSQL::GexDbSQLTable("ft_sublot_hbin",  "TFTLB", m_pclDatabaseConnector->m_strSchemaName);
        sqlTableWtProductBin	= GexDbPluginSQL::GexDbSQLTable("wt_product_hbin", "TWTPB", m_pclDatabaseConnector->m_strSchemaName);
        sqlTableWtWaferBin		= GexDbPluginSQL::GexDbSQLTable("wt_wafer_hbin",   "TWTWB", m_pclDatabaseConnector->m_strSchemaName);

        strFieldBinNo			= "hbin_no";

    }
    else if (datasetSettings.binType() == GexDbPluginERDatasetSettings::SoftBin)
    {
        sqlTableFtProductBin	= GexDbPluginSQL::GexDbSQLTable("ft_product_sbin", "TFTPB", m_pclDatabaseConnector->m_strSchemaName);
        sqlTableFtSubLotBin		= GexDbPluginSQL::GexDbSQLTable("ft_sublot_sbin",  "TFTLB", m_pclDatabaseConnector->m_strSchemaName);
        sqlTableWtProductBin	= GexDbPluginSQL::GexDbSQLTable("wt_product_sbin", "TWTPB", m_pclDatabaseConnector->m_strSchemaName);
        sqlTableWtWaferBin		= GexDbPluginSQL::GexDbSQLTable("wt_wafer_sbin",   "TWTWB", m_pclDatabaseConnector->m_strSchemaName);

        strFieldBinNo			= "sbin_no";
    }
    else
        return false;

    // Create query for Final Test binning
    GexDbPluginSQL::GexDbSQLQuery sqlQueryFTBin(m_pclDatabaseConnector);

    // Add main table
    sqlQueryFTBin.addTable(sqlTableAzConsolidatedTrackingLot);

    // Add field to return in the query result
    sqlQueryFTBin.addField(GexDbPluginSQL::GexDbSQLField("bin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_no", sqlTableFtProductBin));
    sqlQueryFTBin.addField(GexDbPluginSQL::GexDbSQLField("bin_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_name", sqlTableFtProductBin));
    sqlQueryFTBin.addField(GexDbPluginSQL::GexDbSQLField("bin_cat", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_cat", sqlTableFtProductBin));

    sqlField = GexDbPluginSQL::GexDbSQLField("nb_parts", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_volume", sqlTableFtSubLotBin);
    sqlField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionSum);
    sqlQueryFTBin.addField(sqlField);

    // Add madatory fields like parts, parts good and start_t
    sqlQueryFTBin.addField(GexDbPluginSQL::GexDbSQLField("nb_parts_good", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));
    sqlQueryFTBin.addField(GexDbPluginSQL::GexDbSQLField("nb_parts", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));

    // Add start time
    sqlField = GexDbPluginSQL::GexDbSQLField("start_t", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "start_t", sqlTableAzConsolidatedTrackingLot);
    sqlField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionMax);
    sqlQueryFTBin.addField(sqlField);

    // Add join with ft_lot table with condition
    //	* on ft_lot.tracking_lot_id = az_consolidated_tl.ft_tracking_lot_id
    GexDbPluginSQL::GexDbSQLJoin		sqlJoinFtLot(sqlTableFtLot, GexDbPluginSQL::GexDbSQLJoin::SqlLeftOuterJoin);
    GexDbPluginSQL::GexDbSQLCondition	sqlConditionFtLot(GexDbPluginSQL::GexDbSQLField("tracking_lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableFtLot),
                                                        GexDbPluginSQL::GexDbSQLField("ft_tracking_lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));
    sqlJoinFtLot.addCondition(sqlConditionFtLot);
    sqlQueryFTBin.addJoin(sqlJoinFtLot);

    // Add join with ft_product_xbin table with condition
    //	* on ft_product_xbin.product_name = az_consolidated_tl_data.ft_product_name
    // GCORE-1200: TO BE Checked [SC]
    // the az_consolidated_tl_data.ft_product_name='MULTI';
    GexDbPluginSQL::GexDbSQLJoin		sqlJoinFtProductBin(sqlTableFtProductBin, GexDbPluginSQL::GexDbSQLJoin::SqlLeftOuterJoin);
    GexDbPluginSQL::GexDbSQLCondition	sqlConditionFtProductBin(GexDbPluginSQL::GexDbSQLField("product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableFtProductBin),
                                                        GexDbPluginSQL::GexDbSQLField("ft_product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));
    sqlJoinFtProductBin.addCondition(sqlConditionFtProductBin);
    sqlQueryFTBin.addJoin(sqlJoinFtProductBin);

    // Add join with ft_sublot_xbin table with condition
    //	* on ft_sublot_xbin.lot_id = az_consolidated_tl_data.ft_lot_id
    //		and	ft_product_xbin.bin_no = ft_sublot_xbin.xbin_no
    GexDbPluginSQL::GexDbSQLJoin		sqlJoinFtLotBin(sqlTableFtSubLotBin, GexDbPluginSQL::GexDbSQLJoin::SqlLeftOuterJoin);
    GexDbPluginSQL::GexDbSQLCondition	sqlConditionFtLotBinOnLot(GexDbPluginSQL::GexDbSQLField("lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableFtSubLotBin),
                                                        GexDbPluginSQL::GexDbSQLField("lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableFtLot));
    GexDbPluginSQL::GexDbSQLCondition	sqlConditionFtLotBinOnBinNo(GexDbPluginSQL::GexDbSQLField("bin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableFtProductBin),
                                                        GexDbPluginSQL::GexDbSQLField(strFieldBinNo, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableFtSubLotBin));
    sqlJoinFtLotBin.addCondition(sqlConditionFtLotBinOnLot);
    sqlJoinFtLotBin.addCondition(sqlConditionFtLotBinOnBinNo);
    sqlQueryFTBin.addJoin(sqlJoinFtLotBin);

    // Add condition on bin_no not null
    sqlQueryFTBin.addCondition(GexDbPluginSQL::GexDbSQLCondition(GexDbPluginSQL::GexDbSQLField("bin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableFtProductBin), "", GexDbPluginSQL::GexDbSQLCondition::sqlConditionIsNotNull));
    sqlQueryFTBin.addCondition(GexDbPluginSQL::GexDbSQLCondition(GexDbPluginSQL::GexDbSQLField("production_stage", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot), "FT", GexDbPluginSQL::GexDbSQLCondition::sqlConditionStringEqual));

    // Add group by on ft_tracking_lot_id
    sqlQueryFTBin.addGroupBy(GexDbPluginSQL::GexDbSQLField("ft_tracking_lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));
    sqlQueryFTBin.addGroupBy(GexDbPluginSQL::GexDbSQLField("ft_product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));

    // Add group by field
    if (AER_fillQueryField(datasetSettings, sqlQueryFTBin, false, sqlTableAzConsolidatedTrackingLot, sqlTableFtProductBin) == false)
        return false;

    // Create query for Wafer Test binning
    GexDbPluginSQL::GexDbSQLQuery sqlQueryWTBin(m_pclDatabaseConnector);

    // Add main table
    sqlQueryWTBin.addTable(sqlTableAzConsolidatedTrackingLot);

    // Add field to return in the query result
    sqlQueryWTBin.addField(GexDbPluginSQL::GexDbSQLField("bin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_no", sqlTableWtProductBin));
    sqlQueryWTBin.addField(GexDbPluginSQL::GexDbSQLField("bin_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_name", sqlTableWtProductBin));
    sqlQueryWTBin.addField(GexDbPluginSQL::GexDbSQLField("bin_cat", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_cat", sqlTableWtProductBin));

    sqlField = GexDbPluginSQL::GexDbSQLField("nb_parts", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "bin_volume", sqlTableWtWaferBin);
    sqlField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionSum);
    sqlQueryWTBin.addField(sqlField);

    // Add madatory fields like parts, parts good and start_t
    sqlQueryWTBin.addField(GexDbPluginSQL::GexDbSQLField("nb_parts_good", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));
    sqlQueryWTBin.addField(GexDbPluginSQL::GexDbSQLField("nb_parts", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));

    // Add start time
    sqlField = GexDbPluginSQL::GexDbSQLField("start_t", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "start_t", sqlTableAzConsolidatedTrackingLot);
    sqlField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionMax);
    sqlQueryWTBin.addField(sqlField);

    // Add join with ft_die_tracking table with condition on ft_die_tracking.tracking_lot_id = ft_consolidated_tl_id.tracking_lot_id
    // GCORE-1200: TO BE Checked [SC]
    GexDbPluginSQL::GexDbSQLQuery		sqlQueryFtDieTracking(m_pclDatabaseConnector);
    sqlQueryFtDieTracking.addField(GexDbPluginSQL::GexDbSQLField("ft_tracking_lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableFtDieTracking));
    sqlQueryFtDieTracking.addField(GexDbPluginSQL::GexDbSQLField("wt_tracking_lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableFtDieTracking));
    sqlQueryFtDieTracking.addField(GexDbPluginSQL::GexDbSQLField("wt_product_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableFtDieTracking));
    sqlQueryFtDieTracking.setDistinct(true);
    sqlQueryFtDieTracking.addTable(sqlTableFtDieTracking);

    // Use query on ft_die_tracking as table for the join
    GexDbPluginSQL::GexDbSQLTable		sqlTableQueryFTDieTracking(sqlQueryFtDieTracking, "TFTDT");
    GexDbPluginSQL::GexDbSQLJoin		sqlJoinFtDieTracking(sqlTableQueryFTDieTracking, GexDbPluginSQL::GexDbSQLJoin::SqlLeftOuterJoin);
    GexDbPluginSQL::GexDbSQLCondition	sqlConditionFtDieTracking(GexDbPluginSQL::GexDbSQLField("ft_tracking_lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableQueryFTDieTracking),
                                                        GexDbPluginSQL::GexDbSQLField("ft_tracking_lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));
    sqlJoinFtDieTracking.addCondition(sqlConditionFtDieTracking);
    sqlQueryWTBin.addJoin(sqlJoinFtDieTracking);

    // Add join with wt_product_xbin table with condition
    //	* on wt_product_xbin.product_name = ft_die_tracking.product_id
    // GCORE-1200: Checked [SC]
    GexDbPluginSQL::GexDbSQLJoin		sqlJoinWtProductBin(sqlTableWtProductBin, GexDbPluginSQL::GexDbSQLJoin::SqlLeftOuterJoin);
    GexDbPluginSQL::GexDbSQLCondition	sqlConditionWtProductBin(GexDbPluginSQL::GexDbSQLField("product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableWtProductBin),
                                                        GexDbPluginSQL::GexDbSQLField("wt_product_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableQueryFTDieTracking));
    sqlJoinWtProductBin.addCondition(sqlConditionWtProductBin);
    sqlQueryWTBin.addJoin(sqlJoinWtProductBin);

    // Add join with wt_wafer_xbin table with condition
    //	* on wt_wafer_xbin.lot_id = ft_die_tracking.wt_tracking_lot_id
    //		and	wt_wafer_xbin.xbin_no = wt_product_xbin.bin_no
    GexDbPluginSQL::GexDbSQLJoin		sqlJoinWtWaferBin(sqlTableWtWaferBin, GexDbPluginSQL::GexDbSQLJoin::SqlLeftOuterJoin);
    GexDbPluginSQL::GexDbSQLCondition	sqlConditionWtWaferBinOnLot(GexDbPluginSQL::GexDbSQLField("lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableWtWaferBin),
                                                        GexDbPluginSQL::GexDbSQLField("wt_tracking_lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableQueryFTDieTracking));
    GexDbPluginSQL::GexDbSQLCondition	sqlConditionWtWaferBinOnBinNo(GexDbPluginSQL::GexDbSQLField(strFieldBinNo, GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableWtWaferBin),
                                                        GexDbPluginSQL::GexDbSQLField("bin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableWtProductBin));
    sqlJoinWtWaferBin.addCondition(sqlConditionWtWaferBinOnLot);
    sqlJoinWtWaferBin.addCondition(sqlConditionWtWaferBinOnBinNo);
    sqlQueryWTBin.addJoin(sqlJoinWtWaferBin);

    // Add condition on bin_no not null
    sqlQueryWTBin.addCondition(GexDbPluginSQL::GexDbSQLCondition(GexDbPluginSQL::GexDbSQLField("bin_no", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableWtProductBin), "", GexDbPluginSQL::GexDbSQLCondition::sqlConditionIsNotNull));
    sqlQueryWTBin.addCondition(GexDbPluginSQL::GexDbSQLCondition(GexDbPluginSQL::GexDbSQLField("production_stage", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot), "WT", GexDbPluginSQL::GexDbSQLCondition::sqlConditionStringEqual));

    // Add group by on ft_tracking_lot_id
    sqlQueryWTBin.addGroupBy(GexDbPluginSQL::GexDbSQLField("ft_tracking_lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));
    sqlQueryWTBin.addGroupBy(GexDbPluginSQL::GexDbSQLField("ft_product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));

    // Add group by field
    if (AER_fillQueryField(datasetSettings, sqlQueryWTBin, false, sqlTableAzConsolidatedTrackingLot, sqlTableWtProductBin) == false)
        return false;

    // Add union between the two queries
    sqlQueryFTBin.addUnion(sqlQueryWTBin);

    // Create query for AZ binning
    GexDbPluginSQL::GexDbSQLQuery sqlQueryAZBin(m_pclDatabaseConnector);

    // Add main table
    sqlQueryAZBin.addTable(sqlTableAzConsolidatedTrackingLot);

    // Add field to return in the query result
    sqlQueryAZBin.addField(GexDbPluginSQL::GexDbSQLField("all", GexDbPluginSQL::GexDbSQLField::SqlFieldText, "bin_no", sqlTableWtProductBin));
    sqlQueryAZBin.addField(GexDbPluginSQL::GexDbSQLField("null", GexDbPluginSQL::GexDbSQLField::SqlFieldNumeric, "bin_name", ""));
    sqlQueryAZBin.addField(GexDbPluginSQL::GexDbSQLField("null", GexDbPluginSQL::GexDbSQLField::SqlFieldNumeric, "bin_cat", ""));
    sqlQueryAZBin.addField(GexDbPluginSQL::GexDbSQLField("null", GexDbPluginSQL::GexDbSQLField::SqlFieldNumeric, "bin_volume", ""));

    // Add madatory fields like parts, parts good and start_t
    sqlQueryAZBin.addField(GexDbPluginSQL::GexDbSQLField("nb_parts_good", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));
    sqlQueryAZBin.addField(GexDbPluginSQL::GexDbSQLField("nb_parts", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));

    // Add start time
    sqlField = GexDbPluginSQL::GexDbSQLField("start_t", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "start_t", sqlTableAzConsolidatedTrackingLot);
    sqlField.setSqlFunction(GexDbPluginSQL::GexDbSQLField::SqlFunctionMax);
    sqlQueryAZBin.addField(sqlField);

    // Add group by on ft_tracking_lot_id
    sqlQueryAZBin.addGroupBy(GexDbPluginSQL::GexDbSQLField("ft_tracking_lot_id", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));
    sqlQueryAZBin.addGroupBy(GexDbPluginSQL::GexDbSQLField("ft_product_name", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", sqlTableAzConsolidatedTrackingLot));

    // Add group by field
    if (AER_fillQueryField(datasetSettings, sqlQueryAZBin, false, sqlTableAzConsolidatedTrackingLot, GexDbPluginSQL::GexDbSQLTable()) == false)
        return false;

    // Add union between the two queries
    sqlQueryFTBin.addUnion(sqlQueryAZBin);

    // Create the whole query
    gexdbSQLQuery = sqlQueryFTBin;

    return true;
}

bool GexDbPlugin_Galaxy::AER_buildFullTimeLineQuery(const GexDbPluginERDatasetSettings &datasetSettings, GexDbPluginSQL::GexDbSQLQuery &gexdbSQLQuery, bool& bFullTimeLineCompatible)
{
    QList<GexDbPluginSQL::GexDbSQLQuery>    queries;
    GexDbPluginSQL::GexDbSQLQuery           gexdbStandardQuery(m_pclDatabaseConnector);
    GexDbPluginSQL::GexDbSQLQuery           gexdbIntermediateQuery(m_pclDatabaseConnector);

    // Build query for physical consolidated data (Standard consolidated data)
    if (AER_buildFullTimeLineSubQuery(datasetSettings, gexdbStandardQuery, bFullTimeLineCompatible, false) == false)
        return false;

    queries.append(gexdbStandardQuery);

    // Build query for intermediate consolidated data
    if (datasetSettings.useIntermediateConsolidatedData())
    {
        if (AER_buildFullTimeLineSubQuery(datasetSettings, gexdbIntermediateQuery, bFullTimeLineCompatible, true) == false)
            return false;

        queries.append(gexdbIntermediateQuery);
    }

    // Generates the whole query, including union between multiple subqueries.
    for (int idx = 0; idx < queries.count(); idx++)
    {
        if (idx == 0)
            gexdbSQLQuery = queries.at(idx);
        else
            gexdbSQLQuery.addUnion(queries.at(idx));
    }

    return true;
}

bool GexDbPlugin_Galaxy::AER_buildFullTimeLineSubQuery(const GexDbPluginERDatasetSettings &datasetSettings, GexDbPluginSQL::GexDbSQLQuery &gexdbSQLQuery, bool &bFullTimeLineCompatible, bool bIntermediate)
{
    GexDbPluginSQL::GexDbSQLTable	gexdbSQLTable;
    GexDbPluginSQL::GexDbSQLField	gexdbSQLField;
    QString strConsolidatedTable;
    QString strSuffixTable;
    QString lOrderDirection;

    // Add suffix to consolidated table for Wafer Sort level when querying Intermediate consolidated data
    if ((datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST ||
         datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST) && bIntermediate)
        strSuffixTable = "_inter";

    // Initialize table object
    if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_WTEST)
        strConsolidatedTable = "wt_consolidated_wafer";
    else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_FTEST)
        strConsolidatedTable = "ft_consolidated_sublot";
    else if (datasetSettings.testingStage() == GEXDB_PLUGIN_GALAXY_AZ)
        strConsolidatedTable = "az_consolidated_tl";
    else
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_InvalidTestingStage, NULL, datasetSettings.testingStage().toLatin1().constData());
        ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
        return false;
    }

    strConsolidatedTable += strSuffixTable;
    gexdbSQLTable = GexDbPluginSQL::GexDbSQLTable(strConsolidatedTable, "CT", m_pclDatabaseConnector->m_strSchemaName);

    // Add aggregate to fields and group by
    for (int nAggregate = 0; nAggregate < datasetSettings.aggregates().count(); ++nAggregate)
    {
        QString strField, strAlias;
        bool	bIsNumeric	= false;
        bool	bIsTime		= false;
        if (GetConsolidatedFieldName(datasetSettings.testingStage(), datasetSettings.aggregates().at(nAggregate), strField, &bIsNumeric, NULL, &bIsTime))
        {
            if (bIsTime)
            {
                strAlias = "aggregate_" + strField;

                gexdbSQLField = GexDbPluginSQL::GexDbSQLField(strField, GexDbPluginSQL::GexDbSQLField::SqlFieldName, strAlias, gexdbSQLTable, bIsNumeric);
            }
            else
                bFullTimeLineCompatible = false;
        }
        else
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, datasetSettings.aggregates().at(nAggregate).toLatin1().constData());
            return false;
        }

        gexdbSQLQuery.addField(gexdbSQLField);
        gexdbSQLQuery.addGroupBy(gexdbSQLField);

        GexDbPluginSQL::GexDbSQLQuery::sqlOrderBy lOrderBy = GexDbPluginSQL::GexDbSQLQuery::SqlOrderByAsc;
        if (datasetSettings.isOrderedField(datasetSettings.aggregates().at(nAggregate), lOrderDirection))
        {
            if (lOrderDirection.toLower() == "desc")
            {
                lOrderBy = GexDbPluginSQL::GexDbSQLQuery::SqlOrderByDesc;
            }
        }

        gexdbSQLQuery.addOrderBy(gexdbSQLField, lOrderBy);
    }

    // Add time period condition
    if (datasetSettings.timePeriod() != GEX_QUERY_TIMEPERIOD_ALLDATES)
    {
        gexdbSQLField = GexDbPluginSQL::GexDbSQLField("start_t", GexDbPluginSQL::GexDbSQLField::SqlFieldName, "", gexdbSQLTable);
        gexdbSQLQuery.addCondition(GexDbPluginSQL::GexDbSQLCondition(gexdbSQLField, datasetSettings.timestampFrom(), datasetSettings.timestampTo()));
    }

    // Add table to the query
    gexdbSQLQuery.addTable(gexdbSQLTable);

    return true;
}

bool GexDbPlugin_Galaxy::AER_executeQuery(const GexDbPluginERDatasetSettings &datasetSettings, const GexDbPluginSQL::GexDbSQLQuery &gexdbSQLQuery, GexDbPluginERDataset& datasetResult, bool bOverall)
{
    // Build the SQL string
    QString strQuery = gexdbSQLQuery.buildSQLString();

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clGexDbQuery.lastError().text().left(1024).toLatin1().constData());
        return false;
    }

    int			nIndex;
    QStringList lstRdbFields;
    QStringList	lstValue;
    QString		strField;

    // get all decorated name one time to improve perfs
    QStringList lstAggregates;
    QStringList lstSeries;
    QStringList lstGroups;
    bool		bBinInAggregates	= false;
    bool		bBinInSeries		= false;
    bool		bBinInGroups		= false;
    bool		bIsBinning			= false;

    // Groups decorated name
    for (int nGroup = 0; nGroup < datasetSettings.groups().count(); ++nGroup)
    {
        if (datasetSettings.groups().at(nGroup).isEmpty() == false)
        {
            if (GetConsolidatedFieldName(datasetSettings.testingStage(), datasetSettings.groups().at(nGroup), strField, NULL, &bIsBinning))
            {
                lstGroups.append(strField);

                if (bIsBinning)
                    bBinInGroups = true;
            }
        }
        else
            lstGroups.append("");
    }

    // Serie decorated name
    for (int nSerie = 0; nSerie < datasetSettings.series().count(); ++nSerie)
    {
        if (GetConsolidatedFieldName(datasetSettings.testingStage(), datasetSettings.series().at(nSerie), strField, NULL, &bIsBinning))
        {
            lstSeries.append(strField);

            if (bIsBinning)
                bBinInSeries = true;
        }
        else
        {
            lstSeries.append("");
            GEX_ASSERT(false);
        }
    }

    // Aggregate decorated name
    for (int nAggregate = 0; nAggregate < datasetSettings.aggregates().count(); ++nAggregate)
    {
        if (GetConsolidatedFieldName(datasetSettings.testingStage(), datasetSettings.aggregates().at(nAggregate), strField, NULL, &bIsBinning))
        {
            lstAggregates.append(strField);

            if (bIsBinning)
                bBinInAggregates = true;
        }
        else
        {
            lstAggregates.append("");
            GEX_ASSERT(false);
        }
    }

    GetRdbFieldsList(datasetSettings.testingStage(), lstRdbFields, "Y", "h", "*" , "*", "Y", "N");

    // Iterate on each query result
    while(clGexDbQuery.Next())
    {
        QSqlRecord				sqlRecord(clGexDbQuery.record());
        GexDbPluginERDatasetRow	datasetRow;

        // Set binning field exists in groups
        if (bBinInGroups)
            datasetRow.setBinGroupBy(GexDbPluginERDatasetRow::BinGroupByGroup);

        // Set binning field exists in series
        if (bBinInSeries)
        {
            datasetRow.setBinGroupBy(GexDbPluginERDatasetRow::BinGroupBySerie);

            if (datasetSettings.binType() == GexDbPluginERDatasetSettings::SoftBin)
                datasetRow.setField("bin_type", "S");
            else if (datasetSettings.binType() == GexDbPluginERDatasetSettings::HardBin)
                datasetRow.setField("bin_type", "H");
        }

        // Set binning field exists in aggregates
        if (bBinInAggregates)
            datasetRow.setBinGroupBy(GexDbPluginERDatasetRow::BinGroupByAggregate);

        // Retrieve group fields if any
        lstValue.clear();
        for (int nGroup = 0; nGroup < lstGroups.count(); ++nGroup)
        {
            if (lstGroups.at(nGroup).isEmpty() == false)
            {
                nIndex = sqlRecord.indexOf(lstGroups.at(nGroup));

                lstValue.append(clGexDbQuery.value(nIndex).toString());
            }
            else
                lstValue.append("");
        }

        datasetRow.setGroup(lstValue.join(","));
        datasetRow.setField("GroupName", lstValue.join(","));

        // Retrieve series fields  if any
        lstValue.clear();
        if (bOverall)
        {
            nIndex = sqlRecord.indexOf("Overall");
            lstValue.append(clGexDbQuery.value(nIndex).toString());
        }
        else
        {
            for (int nSerie = 0; nSerie < lstSeries.count(); ++nSerie)
            {
                if (lstSeries.at(nSerie).isEmpty() == false)
                {
                    nIndex = sqlRecord.indexOf(lstSeries.at(nSerie));

                    lstValue.append(clGexDbQuery.value(nIndex).toString());
                }
                else
                    lstValue.append("");

//				// HACK
//				else
//				{
//					nIndex = sqlRecord.indexOf("bin_no");

//					lstValue.append(clGexDbQuery.value(nIndex).toString());

//					datasetRow.setBinGroupBy(GexDbPluginERDatasetRow::BinGroupBySerie);

//				}
            }
        }

        datasetRow.setSerie(lstValue.join(","));
        datasetRow.setField("SerieName", lstValue.join(","));

        // Retrieve aggregate fields  if any
        lstValue.clear();
        for (int nAggregate = 0; nAggregate < lstAggregates.count(); ++nAggregate)
        {
            if (lstAggregates.at(nAggregate).isEmpty() == false)
            {
                nIndex = sqlRecord.indexOf(lstAggregates.at(nAggregate));

                lstValue.append(clGexDbQuery.value(nIndex).toString());
            }
            else
                lstValue.append("");
        }

        datasetRow.setAggregate(lstValue.join(","));
        datasetRow.setField("AggregateName", lstValue.join(","));

        nIndex = sqlRecord.indexOf("volume");
        if (nIndex >= 0)
            datasetRow.setVolume(clGexDbQuery.value(nIndex).toUInt());

        nIndex = sqlRecord.indexOf("bin_volume");
        if (nIndex >= 0)
            datasetRow.setBinVolume(clGexDbQuery.value(nIndex).toUInt());

        nIndex = sqlRecord.indexOf("good_volume");
        if (nIndex >= 0)
            datasetRow.setGoodVolume(clGexDbQuery.value(nIndex).toUInt());

        nIndex = sqlRecord.indexOf("gross_volume");
        if (nIndex >= 0)
            datasetRow.setGrossVolume(clGexDbQuery.value(nIndex).toUInt());

        nIndex = sqlRecord.indexOf("start_time");
        if (nIndex >= 0)
        {
            datasetRow.setStartTime(clGexDbQuery.value(nIndex).toUInt());
            datasetRow.setField("start_time", clGexDbQuery.value(nIndex).toString());
        }

        nIndex = sqlRecord.indexOf("wafer_count");
        if (nIndex >= 0)
            datasetRow.setWaferCount(clGexDbQuery.value(nIndex).toUInt());

        // Get extra fields
        for (int nCount = 0; nCount < sqlRecord.count(); nCount++)
            datasetRow.setField(sqlRecord.fieldName(nCount), clGexDbQuery.value(nCount));

        // Add the row to the dataset
        datasetResult.addRow(datasetRow);
    }

    return true;
}

bool GexDbPlugin_Galaxy::AER_executeFullTimeLineQuery(const GexDbPluginERDatasetSettings &datasetSettings, const GexDbPluginSQL::GexDbSQLQuery &gexdbSQLQuery, GexDbPluginERDataset& datasetResult)
{
    // Build the SQL string
    QString strQuery = gexdbSQLQuery.buildSQLString();

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clGexDbQuery.lastError().text().left(1024).toLatin1().constData());
        return false;
    }

    int			nIndex;
    QStringList	lstValue;
    QString		strField;

    // Iterate on each query result
    while(clGexDbQuery.Next())
    {
        QSqlRecord sqlRecord = clGexDbQuery.record();

        // Retrieve aggregate fields  if any
        lstValue.clear();
        for (int nAggregate = 0; nAggregate < datasetSettings.aggregates().count(); ++nAggregate)
        {
            if (GetConsolidatedFieldName(datasetSettings.testingStage(), datasetSettings.aggregates().at(nAggregate), strField, NULL))
            {
                nIndex = sqlRecord.indexOf("aggregate_" + strField);
                if (nIndex >= 0)
                    lstValue.append(clGexDbQuery.value(nIndex).toString());
            }
        }

        // Add the empty aggregate to the dataset
        datasetResult.addRow(lstValue.join(","));
    }

    return true;
}
