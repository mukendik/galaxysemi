#include <QSqlRecord>
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_base.h"
#include "test_filter.h"

#include "gex_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>

#include "QSqlError"

///////////////////////////////////////////////////////////
// Query: add a field to query selection, eventually based
// on an expression (day, week...)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::Query_AddFieldExpression(const QString & strMetaField, const QString & strAs, QString & strDbField, QString & strDbTable, bool bUseMaxStartT/*=false*/, bool bUseSamples/*=false*/, int nBinning/*=-1*/, bool bSoftBin/*=false*/)
{
    QString strField, strFieldSpec, strDbField0, strDbField1, strDbLink0, strCondition, strBinType;

    // Set bin table
    if(bSoftBin)
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
            if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
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
            else
            {
                strFieldSpec += "|(case when bitand(";
                strFieldSpec += strDbField1;
                strFieldSpec += ",4)=4 then 0 else ";
            }
            strFieldSpec += strDbField0;
            strFieldSpec += " end)";
            if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                m_strlQuery_Fields.append(strFieldSpec);

            // Link condition
            strDbLink0 = m_strTablePrefix +"parts_stats_summary.splitlot_id|";
            strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
            if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
                m_strlQuery_LinkConditions.append(strDbLink0);

            // Value condition
            strCondition = m_strTablePrefix+"parts_stats_summary.site_no";
            strCondition += "|Numeric|0-9999";
            if(m_strlQuery_ValueConditions.indexOf(strCondition))
                m_strlQuery_ValueConditions.append(strCondition);
        }
    }
    else if(strField == "stats_nbmatching")
    {
        if(nBinning == -1)
        {
            if(bUseSamples)
            {
                // Field
                strDbField0 = m_strTablePrefix+"parts_stats_samples.nb_parts_good";
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
                if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
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
                strDbField0 = m_strTablePrefix+"parts_stats_summary.nb_good";
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
                if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                    m_strlQuery_Fields.append(strFieldSpec);

                // Link condition
                strDbLink0 = m_strTablePrefix +"parts_stats_summary.splitlot_id|";
                strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
                if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
                    m_strlQuery_LinkConditions.append(strDbLink0);

                // Value condition
                strCondition = m_strTablePrefix+"parts_stats_summary.site_no";
            strCondition += "|Numeric|0-9999";
                if(m_strlQuery_ValueConditions.indexOf(strCondition))
                    m_strlQuery_ValueConditions.append(strCondition);
            }
        }
        else
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
                    strFieldSpec += "|(case when (";
                    strFieldSpec += strDbField1;
                    strFieldSpec += " & 4)=4 then ";
                }

                strFieldSpec += strDbField0;
                strFieldSpec += " else 0 end)";
                if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                    m_strlQuery_Fields.append(strFieldSpec);

                // Link condition
                strDbLink0 = m_strTablePrefix +strBinType+"_stats_samples.splitlot_id|";
                strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
                if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
                    m_strlQuery_LinkConditions.append(strDbLink0);

                // Value condition
                strCondition = m_strTablePrefix+strBinType+"_stats_samples.site_no";
                strCondition += "|Numeric|0-9999";
                if(m_strlQuery_ValueConditions.indexOf(strCondition))
                    m_strlQuery_ValueConditions.append(strCondition);
                strCondition = m_strTablePrefix+strBinType+"_stats_samples.";
                strCondition += strBinType+"_no";
                strCondition += "|Numeric|";
                strCondition += QString::number(nBinning);
                if(m_strlQuery_ValueConditions.indexOf(strCondition))
                    m_strlQuery_ValueConditions.append(strCondition);
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
                    strFieldSpec += "|(case when (";
                    strFieldSpec += strDbField1;
                    strFieldSpec += " & 4)=4 then 0 else ";
                }

                strFieldSpec += strDbField0;
                strFieldSpec += " end)";
                if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                    m_strlQuery_Fields.append(strFieldSpec);

                // Link condition
                strDbLink0 = m_strTablePrefix +strBinType+"_stats_summary.splitlot_id|";
                strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
                if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
                    m_strlQuery_LinkConditions.append(strDbLink0);

                // Value condition
                strCondition = m_strTablePrefix+strBinType+"_stats_summary.site_no";
                strCondition += "|Numeric|0-9999";
                if(m_strlQuery_ValueConditions.indexOf(strCondition))
                    m_strlQuery_ValueConditions.append(strCondition);
                strCondition = m_strTablePrefix+strBinType+"_stats_summary.";
                strCondition += strBinType+"_no";
                strCondition += "|Numeric|";
                strCondition += QString::number(nBinning);
                if(m_strlQuery_ValueConditions.indexOf(strCondition))
                    m_strlQuery_ValueConditions.append(strCondition);
            }
        }
    }
    else if(strField == QString(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR]).toLower())
    {
        if(nBinning == -1)
        {
            if(bUseSamples)
            {
                strDbField0 = m_strTablePrefix+"parts_stats_samples.site_no";
                strDbLink0 = m_strTablePrefix +"parts_stats_samples.splitlot_id|";
                strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
            }
            else
            {
                strDbField0 = m_strTablePrefix+"parts_stats_summary.site_no";
                strDbLink0 = m_strTablePrefix +"parts_stats_summary.splitlot_id|";
                strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
            }
        }
        else
        {
            if(bUseSamples)
            {
                strDbField0 = m_strTablePrefix+strBinType+"_stats_samples.site_no";
                strDbLink0 = m_strTablePrefix +strBinType+"_stats_samples.splitlot_id|";
                strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
            }
            else
            {
                strDbField0 = m_strTablePrefix+strBinType+"_stats_summary.site_no";
                strDbLink0 = m_strTablePrefix +strBinType+"_stats_summary.splitlot_id|";
                strDbLink0 += m_strTablePrefix +"splitlot.splitlot_id";
            }
        }
        strFieldSpec = "Field=";
        strFieldSpec += strAs;
        strFieldSpec += "|";
        strFieldSpec += strDbField0;
        if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
            m_strlQuery_Fields.append(strFieldSpec);
        if(m_strlQuery_LinkConditions.indexOf(strDbLink0) == -1)
            m_strlQuery_LinkConditions.append(strDbLink0);
        strCondition = strDbField0;
        strCondition += "|Numeric|0-9999";
        if(m_strlQuery_ValueConditions.indexOf(strCondition))
            m_strlQuery_ValueConditions.append(strCondition);
    }
    else if(strField == "day")
    {
        strDbField0 = m_strTablePrefix+"splitlot.start_t";
        if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strFieldSpec += "|date_format(convert_tz(from_unixtime(";
            if(bUseMaxStartT)
            {
                strFieldSpec += "max(";
                strFieldSpec += strDbField0;
                strFieldSpec += ")";
            }
            else
                strFieldSpec += strDbField0;
            strFieldSpec += "),'SYSTEM','+0:00'),'%Y-%m-%d')";
            if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                m_strlQuery_Fields.append(strFieldSpec);
        }
        else
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strFieldSpec += "|to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMaxStartT)
            {
                strFieldSpec += "max(";
                strFieldSpec += strDbField0;
                strFieldSpec += ")";
            }
            else
                strFieldSpec += strDbField0;
            strFieldSpec += "/(24*3600),'YYYY-MM-DD')";
            if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                m_strlQuery_Fields.append(strFieldSpec);
        }
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
            strFieldSpec += "|date_format(convert_tz(from_unixtime(";
            if(bUseMaxStartT)
            {
                strFieldSpec += "max(";
                strFieldSpec += strDbField0;
                strFieldSpec += ")";
            }
            else
                strFieldSpec += strDbField0;
            strFieldSpec += "),'SYSTEM','+0:00'),'%Y-%v')";
            if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                m_strlQuery_Fields.append(strFieldSpec);
        }
        else
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strFieldSpec += "|to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMaxStartT)
            {
                strFieldSpec += "max(";
                strFieldSpec += strDbField0;
                strFieldSpec += ")";
            }
            else
                strFieldSpec += strDbField0;
            strFieldSpec += "/(24*3600),'YYYY-IW')";
            if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                m_strlQuery_Fields.append(strFieldSpec);
        }
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
            strFieldSpec += "|date_format(convert_tz(from_unixtime(";
            if(bUseMaxStartT)
            {
                strFieldSpec += "max(";
                strFieldSpec += strDbField0;
                strFieldSpec += ")";
            }
            else
                strFieldSpec += strDbField0;
            strFieldSpec += "),'SYSTEM','+0:00'),'%Y-%m')";
            if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                m_strlQuery_Fields.append(strFieldSpec);
        }
        else
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strFieldSpec += "|to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMaxStartT)
            {
                strFieldSpec += "max(";
                strFieldSpec += strDbField0;
                strFieldSpec += ")";
            }
            else
                strFieldSpec += strDbField0;
            strFieldSpec += "/(24*3600),'YYYY-MM')";
            if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                m_strlQuery_Fields.append(strFieldSpec);
        }
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
            strFieldSpec += "|date_format(convert_tz(from_unixtime(";
            if(bUseMaxStartT)
            {
                strFieldSpec += "max(";
                strFieldSpec += strDbField0;
                strFieldSpec += ")";
            }
            else
                strFieldSpec += strDbField0;
            strFieldSpec += "),'SYSTEM','+0:00'),'%Y')";
            if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                m_strlQuery_Fields.append(strFieldSpec);
        }
        else
        {
            strFieldSpec = "Expression=";
            strFieldSpec += strAs;
            strFieldSpec += "|";
            strFieldSpec += strDbField0;
            strFieldSpec += "|to_char(trunc(to_date('01-01-1970','DD-MM-YYYY'))+";
            if(bUseMaxStartT)
            {
                strFieldSpec += "max(";
                strFieldSpec += strDbField0;
                strFieldSpec += ")";
            }
            else
                strFieldSpec += strDbField0;
            strFieldSpec += "/(24*3600),'YYYY')";
            if(m_strlQuery_Fields.indexOf(strFieldSpec) == -1)
                m_strlQuery_Fields.append(strFieldSpec);
        }
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


//////////////////////////////////////////////////////////////////////
// Add filter on tests to query
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::Query_AddTestlistCondition(
        const GexDbPlugin_Galaxy_TestFilter & clTestFilter,
        const char cTestType,
        enum TestlistConditionTarget eTarget/*=eOnTestResults*/)
{
    QString	strFilterValues, strCondition;

    // If testlist is '*', no condition to add!
    if(clTestFilter.extractAllTests())
        return;

    // Check filter target (test info or test results)
    if(eTarget == eOnTestResults)
    {
        // If filter on test results, check if we have tests in the testlist
        if(m_clTestList.m_uiNbTests == 0)
            return;

        // Check test type
        switch(cTestType)
        {
        case 'P':
        case 'p':
            strFilterValues = m_clTestList.getTestIdList_PTR();
            if(strFilterValues.isEmpty())
                return;
            strCondition = m_strTablePrefix + "ptest_results.ptest_info_id|Numeric|" + strFilterValues;
            break;

        case 'm':
        case 'M':
            strFilterValues = m_clTestList.getTestIdList_MPR();
            if(strFilterValues.isEmpty())
                return;
            strCondition = m_strTablePrefix + "mptest_results.mptest_info_id|Numeric|" + strFilterValues;
            break;

        case 'f':
        case 'F':
            strFilterValues = m_clTestList.getTestIdList_FTR();
            if(strFilterValues.isEmpty())
                return;
            strCondition = m_strTablePrefix + "ftest_results.ftest_info_id|Numeric|" + strFilterValues;
            break;

        default:
            return;
        }
    }

    if(eTarget == eOnTestInfo)
    {
        strFilterValues = clTestFilter.getCleanTestNblist();
        if(strFilterValues.isEmpty())
            return;

        // Check test type
        switch(cTestType)
        {
        case 'P':
        case 'p':
            strCondition = m_strTablePrefix + "ptest_info.tnum|Numeric|" + strFilterValues;
            break;

        case 'm':
        case 'M':
            strCondition = m_strTablePrefix + "mptest_info.tnum|Numeric|" + strFilterValues;
            break;

        case 'f':
        case 'F':
            strCondition = m_strTablePrefix + "ftest_info.tnum|Numeric|" + strFilterValues;
            break;

        default:
            return;
        }
    }

    // Add condition
    if(!strCondition.isEmpty())
        m_strlQuery_ValueConditions.append(strCondition);
}

// Return all test conditions corresponding to the splitlots (according to given filters)
bool GexDbPlugin_Galaxy::QueryTestConditionsList(GexDbPlugin_Filter & dbFilters,
                                                 QStringList & matchingTestConditions)
{
    // Clear returned stringlist
    matchingTestConditions.clear();

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

    // Check database connection
    if(!ConnectToCorporateDb())
    {
      GSLOG(SYSLOG_SEV_ERROR, "cant connect to DB !");
      return false;
    }

    if (SetTestingStage(dbFilters.strDataTypeQuery) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("unknown testing stage %1")
              .arg( dbFilters.strDataTypeQuery).toLatin1().constData());
        GSET_ERROR2(GexDbPlugin_Base, eDB_InvalidTestingStage, NULL,
                    "unknown testing stage %s", dbFilters.strDataTypeQuery.toLatin1().data());
        return false;
    }

    // Varaible to execute the query
    QString fieldExpression;

    // Empty any previous query
    Query_Empty();

    // Compute query date constraints
    Query_ComputeDateConstraints(dbFilters);

    // Add Fields
    fieldExpression = "Field=splitlot_id|" + m_strTablePrefix;
    fieldExpression += "test_conditions.splitlot_id";
    m_strlQuery_Fields.append(fieldExpression);

    GexDbPlugin_Mapping_FieldMap::iterator  itBegin = m_pmapFields_GexToRemote->begin();
    GexDbPlugin_Mapping_FieldMap::iterator  itEnd   = m_pmapFields_GexToRemote->end();

    while (itBegin != itEnd)
    {
        if ((*itBegin).isTestCondition())
        {
            matchingTestConditions.append((*itBegin).m_strMetaDataName);
        }

        ++itBegin;
    }

    // GCORE-17166 to load all TC that are in the DB
    return true;
}

