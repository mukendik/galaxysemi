#include <QSqlError>
#include <QSqlRecord>
#include <gqtl_log.h>
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_base.h"
#include "gex_constants.h"
#include "gex_shared.h"

//////////////////////////////////////////////////////////////////////
// Return all valid values for a field (given filters on other fields)
// Working on consolidated data
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::QueryField_Consolidated(GexDbPlugin_Filter& cFilters,
                        QStringList& cMatchingValues,
                        bool bSoftBin /*= false*/,
                        bool /*bDistinct = true*/,
                        QuerySort /*eQuerySort = eQuerySort_Asc*/)
{
    QString strQuery;
    bool bQueryOnIntermediateTable = false;

    if (!BuildQueryField_Consolidated(cFilters, strQuery, bSoftBin, bQueryOnIntermediateTable))
        return false;

    bQueryOnIntermediateTable = true; /// TODO to change depending on the filters field if it's consolidation data type do not union
    // Add union on intermediate table
    if (bQueryOnIntermediateTable)
    {
        QString strQuery2;
        BuildQueryField_Consolidated(cFilters, strQuery2, bSoftBin, bQueryOnIntermediateTable);
        strQuery += " UNION\n";
        strQuery += strQuery2;
    }


    // 5) Execute query
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);

    if(!clGexDbQuery.Execute(strQuery))
    {
        // Display error
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // 6) Populate stringlist
    QString fieldValue;
    QString matchingValues;

    while(clGexDbQuery.Next())
    {
        matchingValues.clear();
        bool validValue = true;

        // extract values for all requested fields (concatenated with '|' as separator)
        for (int fieldIdx = 0; fieldIdx < clGexDbQuery.record().count(); ++fieldIdx)
        {
            if(!clGexDbQuery.value(fieldIdx).isNull())
                fieldValue = clGexDbQuery.value(fieldIdx).toString();
            else
                fieldValue.clear();

            if(!fieldValue.isEmpty())
            {
                if (fieldIdx != 0)
                    matchingValues += "|";

                matchingValues += fieldValue;
            }
            else
                validValue = false;

        }

        // Add results if all values are valid
        if (validValue)
          cMatchingValues.append(matchingValues);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all valid values for a field (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::QueryField(GexDbPlugin_Filter & cFilters,
                                    QStringList & cMatchingValues,
                                    bool bSoftBin/*=false*/,
                                    bool bDistinct/*=true*/,
                                    QuerySort eQuerySort/*=eQuerySort_Asc*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Fields to query: '%1'...").arg(cFilters.mQueryFields.join(";"))
           .toLatin1().data()
           );

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        WriteDebugMessageFile("GexDbPlugin_Galaxy::QueryField: error : DB not uptodate !");
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate && !IsCompatibleForStdExtraction(uiGexDbBuild, uiBuildSupportedByPlugin))
    {
        WriteDebugMessageFile(
            QString("GexDbPlugin_Galaxy::QueryField: error : DB not uptodate and not compatible for extraction ! (Build=%1 BuildSupportedByPlugin=%2)")
                .arg(uiGexDbBuild)
                .arg(uiBuildSupportedByPlugin) );
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }
    if (SetTestingStage(cFilters.strDataTypeQuery) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("unknown testing stage %1")
              .arg( cFilters.strDataTypeQuery).toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_InvalidTestingStage, NULL,
                    "unknown testing stage %s", cFilters.strDataTypeQuery.toLatin1().data());
        return false;
    }


    Query_Empty();

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // For all query, add a VALID_SPLITLOT<>'N'
    m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N");

    // Check if working on consolidated data
    if(cFilters.bConsolidatedData)
        return QueryField_Consolidated(cFilters, cMatchingValues, bSoftBin, bDistinct, eQuerySort);

    // Raw data
    return GexDbPlugin_Base::QueryField(cFilters, cMatchingValues, bSoftBin, bDistinct, eQuerySort);
}

///////////////////////////////////////////////////////////////////////////////
// Return the query to get all valid values for a field (given filters on other
// fields) from intermediate consolidation table or physical
///////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::BuildQueryField_Consolidated(GexDbPlugin_Filter& cFilters,
                        QString &strQuery,
                        bool bSoftBin /*= false*/,
                        bool bQueryOnIntermediateTable /*=false*/)
{
    QString fieldsSelect;
    QString strField, strSelect, strWhere, strFrom, strTable1, strTable2;
    QString strProductDecoratedField, strProductDbField;
    QString	strQueryFilter, strQueryField, strQueryValue;
    bool	bIsNumeric, bIsBinning, bIsTime;
    bool	bJoinBinTable=false;

    strQuery = "";

    QStringList::const_iterator itQueryField = cFilters.mQueryFields.constBegin();

    // 1) Field to query (strSelect)
    strSelect = "SELECT DISTINCT \n";

    while (itQueryField != cFilters.mQueryFields.constEnd())
    {
        strQueryField = (*itQueryField);

        if(!GetConsolidatedFieldName(cFilters.strDataTypeQuery, strQueryField, strField, &bIsNumeric, &bIsBinning, &bIsTime))
        {
            WriteDebugMessageFile(
                    QString("GexDbPlugin_Galaxy::QueryField: error in 'GetConsolidatedFieldName' for '%1' !")
                        .arg(strQueryField)
                                  );
            return false;
        }

        // Add comma and new line character if there is already a field in the select.
        if (!fieldsSelect.isEmpty())
            fieldsSelect += ",\n";

        // Add field from bin table or consolidated table depending on data type
        if(bIsBinning)
        {
            bJoinBinTable=true;
            fieldsSelect += "BT." + strField;
        }
        else
            fieldsSelect += "CT." + strField;

        ++itQueryField;
    }

    strSelect += fieldsSelect + "\n";

    // 2) Add time period condition
    if(cFilters.bUseTimePeriod)
    {
        strWhere = "WHERE\n";
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

        // Ignore binning filters
        if(bIsBinning)
            continue;
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

    // 4) If have to join with bin tables, get consolidated name for ProductID
    if(bJoinBinTable)
    {
        strProductDecoratedField = m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT];
        if(!GetConsolidatedFieldName(cFilters.strDataTypeQuery, strProductDecoratedField, strProductDbField))
        {
            WriteDebugMessageFile(
                    QString("GexDbPlugin_Galaxy::QueryField: error in 'GetConsolidatedFieldName' for '%1' !")
                        .arg(strProductDecoratedField)
                                  );
            return false;
        }
    }

    // 5) Check production stage and set strFrom
    QString	strDataType = cFilters.strDataTypeQuery.toLower();
    if(strDataType == QString(GEXDB_PLUGIN_GALAXY_AZ).toLower())
    {
        strFrom = "FROM az_consolidated_tl CT\n";
        if(bJoinBinTable)
        {
            WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::QueryField: error binning field not supported in AZ stage !"));
            return false;
        }
    }
    else
    if(strDataType == QString(GEXDB_PLUGIN_GALAXY_WTEST).toLower())
    {
        // GCORE-1200: Checked [SC]
        strTable1 = "wt_consolidated_wafer";
        if (bQueryOnIntermediateTable)
            strTable1 += "_inter";
        strFrom = "FROM " + strTable1 + " CT\n";
        if(bJoinBinTable)
        {
            strTable2 = "wt_product_" + QString(bSoftBin?"sbin":"hbin");
            strFrom += "INNER JOIN " + strTable2 + " BT\n";
            strFrom += "ON BT.product_name=CT." + strProductDbField + "\n";
        }
    }
    else if(strDataType == QString(GEXDB_PLUGIN_GALAXY_FTEST).toLower())
    {// GCORE-1200: Checked [SC]
        strTable1 = "ft_consolidated_sublot";
        if (bQueryOnIntermediateTable)
            strTable1 += "_inter";
        strFrom = "FROM " + strTable1 + " CT\n";
        if(bJoinBinTable)
        {
            strTable2 = "ft_product_" + QString(bSoftBin?"sbin":"hbin");
            strFrom += "INNER JOIN " + strTable2 + " BT\n";
            strFrom += "ON BT.product_name=CT." + strProductDbField + "\n";
        }
    }
    else
    {
        WriteDebugMessageFile(QString("GexDbPlugin_Galaxy::QueryField: error unsupported stage '%1' !")
                              .arg(cFilters.strDataTypeQuery)
                              );
        return false;
    }

    strQuery = strSelect + strFrom + strWhere;

    return true;
}

