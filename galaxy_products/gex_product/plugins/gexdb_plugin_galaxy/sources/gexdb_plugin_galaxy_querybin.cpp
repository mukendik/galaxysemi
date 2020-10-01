#include "gexdb_plugin_galaxy.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>

#include <QSqlError>

//////////////////////////////////////////////////////////////////////
// Return all binnigs (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::QueryBinlist(GexDbPlugin_Filter & cFilters,
                                      QStringList & cMatchingValues,
                                      bool bSoftBin/*=false*/,
                                      bool bClearQueryFirst/*=true*/,
                                      bool bIncludeBinName/*=false*/,
                                      bool bProdDataOnly/*=false*/)
{
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Query Bin list on %1 %2 bins").
          arg((cFilters.bConsolidatedData) ? "consolidated" : "raw").
          arg((bSoftBin) ? "soft" : "hard").toLatin1().constData());

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate, bStatus;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate && !IsCompatibleForStdExtraction(uiGexDbBuild, uiBuildSupportedByPlugin))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // Check if working on consolidated data
    if(cFilters.bConsolidatedData)
        return QueryBinlist_Consolidated(cFilters, cMatchingValues, bSoftBin, bClearQueryFirst, bIncludeBinName);

    if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        bStatus = GexDbPlugin_Base::QueryBinlist(cFilters, cMatchingValues, bSoftBin, bClearQueryFirst, bIncludeBinName, bProdDataOnly);
        return bStatus;
    }
    if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        bStatus = GexDbPlugin_Base::QueryBinlist(cFilters, cMatchingValues, bSoftBin, bClearQueryFirst, bIncludeBinName, bProdDataOnly);
        return bStatus;
    }
    if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
        bStatus = GexDbPlugin_Base::QueryBinlist(cFilters, cMatchingValues, bSoftBin, bClearQueryFirst, bIncludeBinName, bProdDataOnly);
        return bStatus;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Return all binnings in a string, given filters on other fields, and corresponding to specified bin type ("all", "pass", "fail")
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::QueryBinlist(GexDbPlugin_Filter & cFilters,QString & strBinlist,const QString & strBintype,bool bSoftBin/*=false*/,bool bClearQueryFirst/*=true*/, bool bIncludeBinName/*=false*/, bool bProdDataOnly/*=false*/)
{
    QString strBinning;

    // Debug message
    WriteDebugMessageFile("**** QueryBinlist()...");

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        WriteDebugMessageFile("***********************\n");
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate && !IsCompatibleForStdExtraction(uiGexDbBuild, uiBuildSupportedByPlugin))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

    // Clear returned string
    strBinlist = "";

    Query_ComputeDateConstraints(cFilters);

    // Construct SQL query
    QString strQuery;
    ConstructBinlistQuery(cFilters, strQuery, strBintype, bSoftBin, bClearQueryFirst, false, bProdDataOnly);

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        // Display error
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Populate stringlist
    while(clGexDbQuery.Next())
    {
        strBinning = QString::number(clGexDbQuery.value(0).toInt());
        if(bIncludeBinName && !clGexDbQuery.isNull(1))
            strBinning += "#GEX#" + clGexDbQuery.value(1).toString();
        if(strBinlist.isEmpty())
            strBinlist = strBinning;
        else
            strBinlist += "," + strBinning;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all binnigs (given filters on other fields)
// Working on consolidated data
//////////////////////////////////////////////////////////////////////
bool
GexDbPlugin_Galaxy::QueryBinlist_Consolidated(GexDbPlugin_Filter& cFilters,
                                              QStringList& cMatchingValues,
                                              bool bSoftBin /*= false*/,
                                              bool /*bClearQueryFirst = true*/,
                                              bool bIncludeBinName /*= false*/)
{
    QString strQuery, strField, strSelect, strWhere, strFrom, strTable1, strTable2;
    QString strProductDecoratedField, strProductDbField;
    QString	strQueryFilter, strQueryField, strQueryValue;
    bool	bIsNumeric, bIsBinning, bIsTime;

    // TD-78

    // 1) Field to query (strSelect)
    strSelect = "select distinct BT.bin_no, BT.bin_name\n";

    // 2) Add time period condition
    if(cFilters.bUseTimePeriod)
    {
        strWhere = "where\n";
        strWhere += "(CT.start_t BETWEEN " + QString::number(cFilters.tQueryFrom);
        strWhere += " AND " + QString::number(cFilters.tQueryTo) + ")\n";
    }

    // 3) Add other filters
    QStringList::ConstIterator	it;
    QString strCondition;
    for(it = cFilters.strlQueryFilters.begin(); it != cFilters.strlQueryFilters.end(); it++)
    {
        strCondition="";
        strQueryFilter = (*it);
        QString lOperator ("=");
        bool lNegativeOperator(false);
        if (strQueryFilter.contains("!="))
        {
            lOperator = "!=";
            lNegativeOperator = true;
        }

        strQueryField = strQueryFilter.section(lOperator, 0, 0);
        strQueryValue = strQueryFilter.section(lOperator, 1, 1);
        if(!GetConsolidatedFieldName(cFilters.strDataTypeQuery, strQueryField, strField, &bIsNumeric, &bIsBinning, &bIsTime))
        {
            WriteDebugMessageFile(
                    QString("GexDbPlugin_Galaxy::QueryField: error in 'GetConsolidatedFieldName' for '%1' !")
                        .arg(strQueryField)
                                  );
            return false;
        }

        // Join with Bin tables?
        if(bIsBinning)
            strField="BT."+strField;
        else
            strField="CT."+strField;

        if(bIsNumeric)
        {
            strQueryValue.replace("|", ",");
            Query_BuildSqlString_NumericFilter(strField, strQueryValue, strCondition, lNegativeOperator);
        }
        else
        {
            strQueryValue.replace("|",",").replace(",",GEXDB_PLUGIN_DELIMITER_OR);
            Query_BuildSqlString_StringFilter(strField, strQueryValue, strCondition, lNegativeOperator);
        }
        if(!strCondition.isEmpty())
        {
            if(strWhere.isEmpty())
            {
                strWhere = "WHERE ";
                strWhere += strCondition + "\n";
            }
            else
            {
                strWhere += "AND (";
                strWhere += strCondition + ")\n";
            }
        }
    }

    // 4) Join with bin tables, get consolidated name for ProductID
    strProductDecoratedField = m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT];
    if(!GetConsolidatedFieldName(cFilters.strDataTypeQuery, strProductDecoratedField, strProductDbField))
    {
        WriteDebugMessageFile(
                QString("GexDbPlugin_Galaxy::QueryField: error in 'GetConsolidatedFieldName' for '%1' !")
                    .arg(strProductDecoratedField)
                              );
        return false;
    }

    // 5) Check production stage and set strFrom
    QString	strTableName, strDataType = cFilters.strDataTypeQuery.toLower();
    if(strDataType == QString(GEXDB_PLUGIN_GALAXY_AZ).toLower())
    {
        WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::QueryField: error binning field not supported in AZ stage !"));
        return false;
    }
    else
    if(strDataType == QString(GEXDB_PLUGIN_GALAXY_WTEST).toLower())
    {
        // GCORE-1200: Checked [SC]
        strTable1 = "wt_consolidated_wafer";
        strFrom = "from " + strTable1 + " CT\n";
        strTable2 = "wt_product_" + QString(bSoftBin?"sbin":"hbin");
        strFrom += "inner join " + strTable2 + " BT\n";
        strFrom += "on BT.product_name=CT." + strProductDbField + "\n";
    }
    else if(strDataType == QString(GEXDB_PLUGIN_GALAXY_FTEST).toLower())
    {
        // GCORE-1200: Checked [SC]
        strTable1 = "ft_consolidated_sublot";
        strFrom = "from " + strTable1 + " CT\n";
        strTable2 = "ft_product_" + QString(bSoftBin?"sbin":"hbin");
        strFrom += "inner join " + strTable2 + " BT\n";
        strFrom += "on BT.product_name=CT." + strProductDbField + "\n";
    }
    else
    {
        WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::QueryField: error unsupported stage '%1' !")
                              .arg(cFilters.strDataTypeQuery)
                              );
        return false;
    }

    // 5) Execute query
    strQuery = strSelect + strFrom + strWhere;
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);

    if(!clGexDbQuery.Execute(strQuery))
    {
        // Display error
        WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::QueryField failed : '%1'")
                              .arg(strQuery.toLatin1().data()) );
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // 6) Populate stringlist
    QString strMatchingValue;
    while(clGexDbQuery.Next())
    {
        strMatchingValue = QString::number(clGexDbQuery.value(0).toInt());
        if(bIncludeBinName && !clGexDbQuery.isNull(1))
            strMatchingValue += "#GEX#" + clGexDbQuery.value(1).toString();
        if(!strMatchingValue.isEmpty())
            cMatchingValues.append(strMatchingValue);
    }

    return true;
}
