#include "gexdb_plugin_galaxy.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include <QSqlError>
#include <QSqlRecord>

//////////////////////////////////////////////////////////////////////
// Return all products for genealogy reports, with data for at least
// 2 testing stages (use only date in the filter)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::QueryProductList_Genealogy(GexDbPlugin_Filter & cFilters,
                                                    QStringList & cMatchingValues,bool bAllTestingStages)
{
    // Debug message
    WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::QueryProductList_Genealogy: %1 (at least on %2)...")
                          .arg(bAllTestingStages?"for all TS":"for a single TS")
                          .arg(cFilters.strDataTypeQuery) );

    // TD-78

    // Clear list
    cMatchingValues.clear();

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        WriteDebugMessageFile("GexDbPlugin_Galaxy::QueryProductList_Genealogy: cant get DB version !\n");
        return false;
    }
    // Allow extraction on GEXDB Bx with a client supporting GEXDB By (is y>x ?)
    if(!bDbUpToDate && !IsCompatibleForStdExtraction(uiGexDbBuild, uiBuildSupportedByPlugin))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        WriteDebugMessageFile("GexDbPlugin_Galaxy::QueryProductList_Genealogy: error : DB not uptodate or compatible.\n");
        return false;
    }

    // Construct SQL query
    QString strSubQuery_Et, strSubQuery_Wt, strSubQuery_Ft, strQuery;
    QString strFieldSpec, strDbField, strDbTable;

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // E-Test testing stage
    Query_Empty();
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
    strFieldSpec = GEXDB_PLUGIN_DBFIELD_TRACKING_LOT;
    strFieldSpec += "=tracking_lot_id";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    m_strlQuery_Fields.append("Function=max_start_t|et_splitlot.start_t|MAX");
    m_strlQuery_Fields.append("Function=min_start_t|et_splitlot.start_t|MIN");
    m_strlQuery_ValueConditions.append("et_splitlot.valid_splitlot|NotString|N");
    m_strlQuery_ValueConditions.append("et_lot.tracking_lot_id|NotString|");
    m_strlQuery_GroupFields.append("et_lot.tracking_lot_id");
    Query_AddFilters(cFilters);
    Query_BuildSqlString(strSubQuery_Et, false);

    // Wafer Sort testing stage
    Query_Empty();
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
    strFieldSpec = GEXDB_PLUGIN_DBFIELD_TRACKING_LOT;
    strFieldSpec += "=tracking_lot_id";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    m_strlQuery_Fields.append("Function=max_start_t|wt_splitlot.start_t|MAX");
    m_strlQuery_Fields.append("Function=min_start_t|wt_splitlot.start_t|MIN");
    m_strlQuery_ValueConditions.append("wt_splitlot.valid_splitlot|NotString|N");
    m_strlQuery_ValueConditions.append("wt_lot.tracking_lot_id|NotString|");
    m_strlQuery_GroupFields.append("wt_lot.tracking_lot_id");
    Query_AddFilters(cFilters);
    Query_BuildSqlString(strSubQuery_Wt, false);

    // Final Test testing stage
    Query_Empty();
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
    strFieldSpec = GEXDB_PLUGIN_DBFIELD_TRACKING_LOT;
    strFieldSpec += "=tracking_lot_id";
    Query_AddField(strFieldSpec, strDbField, strDbTable);
    m_strlQuery_Fields.append("Function=max_start_t|ft_splitlot.start_t|MAX");
    m_strlQuery_Fields.append("Function=min_start_t|ft_splitlot.start_t|MIN");
    m_strlQuery_ValueConditions.append("ft_splitlot.valid_splitlot|NotString|N");
    m_strlQuery_ValueConditions.append("ft_lot.tracking_lot_id|NotString|");
    m_strlQuery_GroupFields.append("ft_lot.tracking_lot_id");
    Query_AddFilters(cFilters);
    Query_BuildSqlString(strSubQuery_Ft, false);

    // Build final query
    // GCORE-1200: Checked [SC]
    // Return the distinct product per testing_stage from xx_lot table
    strQuery =  "SELECT DISTINCT E.product_name, W.product_name, F.product_name\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += "(\n";
    strQuery += "(\n";
    strQuery += "SELECT tracking_lot_id\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += "SELECT tracking_lot_id, MAX(max_start_t) AS max_start_t, MIN(min_start_t) AS min_start_t\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += "SELECT *\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += strSubQuery_Et;
    strQuery += ") T1\n";
    strQuery += "UNION\n";
    strQuery += "SELECT *\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += strSubQuery_Wt;
    strQuery += ") T2\n";
    strQuery += "UNION\n";
    strQuery += "SELECT *\n";
    strQuery += "FROM\n";
    strQuery += "(\n";
    strQuery += strSubQuery_Ft;
    strQuery += ") T3\n";
    strQuery += ") L1\n";
    strQuery += "GROUP BY tracking_lot_id\n";
    strQuery += ") L2\n";
    strQuery += "WHERE (max_start_t >= " + QString::number(cFilters.tQueryFrom);
    strQuery += " AND min_start_t <= " + QString::number(cFilters.tQueryTo);
    strQuery += ")\n";
    strQuery += ") L3\n";
    strQuery += "LEFT OUTER JOIN et_lot E ON L3.tracking_lot_id=E.tracking_lot_id\n";
    strQuery += ")\n";
    strQuery += "LEFT OUTER JOIN wt_lot W ON L3.tracking_lot_id=W.tracking_lot_id\n";
    strQuery += ")\n";
    strQuery += "LEFT OUTER JOIN ft_lot F ON L3.tracking_lot_id=F.tracking_lot_id\n";

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        // Display error
        qDebug("GexDbPlugin_Galaxy::QueryProductList_Genealogy: error : %s !", clGexDbQuery.lastError().text().toLatin1().data());
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        WriteDebugMessageFile("GexDbPlugin_Galaxy::QueryProductList_Genealogy: error "+clGexDbQuery.lastError().text());
        return false;
    }

    // Populate stringlist (first item is the name of the testing stages)
    QString strMatchingProducts;
    int		nNbTestingStages;
    bool	bValidProduct;
    strMatchingProducts = GEXDB_PLUGIN_GALAXY_ETEST;
    strMatchingProducts += ",";
    strMatchingProducts += GEXDB_PLUGIN_GALAXY_WTEST;
    strMatchingProducts += ",";
    strMatchingProducts += GEXDB_PLUGIN_GALAXY_FTEST;
    cMatchingValues.append(strMatchingProducts);
    while(clGexDbQuery.Next())
    {
        bValidProduct = true;
        nNbTestingStages = 0;
        strMatchingProducts = "";
        if(!clGexDbQuery.isNull(0))
        {
            strMatchingProducts += clGexDbQuery.value(0).toString();
            nNbTestingStages++;
        }
        else if(bAllTestingStages && (cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST))
            bValidProduct = false;
        strMatchingProducts += ",";
        if(!clGexDbQuery.isNull(1))
        {
            strMatchingProducts += clGexDbQuery.value(1).toString();
            nNbTestingStages++;
        }
        else if(bAllTestingStages && (cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST))
            bValidProduct = false;
        strMatchingProducts += ",";
        if(!clGexDbQuery.isNull(2))
        {
            strMatchingProducts += clGexDbQuery.value(2).toString();
            nNbTestingStages++;
        }
        else if(bAllTestingStages && (cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST))
            bValidProduct = false;

        // Append only those valid products having data for at least 2 testing stages
        if(bValidProduct && (nNbTestingStages >= 2))
            cMatchingValues.append(strMatchingProducts);
    }

    // Debug message
    WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::QueryProductList_Genealogy: %1 products retrieved by the query.")
                          .arg(cMatchingValues.count()-1));

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all products (use only date in the filter)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::QueryProductList(GexDbPlugin_Filter & cFilters,
                                          QStringList & cMatchingValues,
                                          QString strProductName/*=""*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" QueryProductList on %1")
          .arg( cFilters.strDataTypeQuery).toLatin1().constData() );

    // Clear list
    cMatchingValues.clear();

    // Set product_id field as the field to query, + eventually a restriction on the value (ie 'PGSM%')
    cFilters.SetFields(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT]);

    if(!strProductName.isEmpty())
    {
        QString strFilter = m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT];
        strFilter += "=" + strProductName;
        cFilters.strlQueryFilters.clear();
        cFilters.strlQueryFilters += strFilter;
    }

    Query_Empty();
    bool bStatus = QueryField(cFilters, cMatchingValues, true, true);

    return bStatus;
}


