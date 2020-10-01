#include "sya_task.h"
#include "statisticalMonitoring/statistical_monitoring_taskdata.h"
#include "statistical_monitoring_tables.h"
#include "statistical_monitoring_alarm_struct.h"
#include "statistical_monitoring_datapoint_struct.h"
#include "statistical_monitoring_limit_struct.h"
#include "statistical_monitoring_item_desc.h"

#include <QSqlError>
#include <QListIterator>

#include <gqtl_log.h>
#include "gexmo_constants.h"
#include "gex_constants.h"
#include "gqtl_sysutils.h"
#include "engine.h"
#include "scheduler_engine.h"
#include "admin_engine.h"
#include "db_engine.h"
#include "gex_database_entry.h"
#include "db_external_database.h"
#include "mo_scheduler_gui.h"
#include "message.h"
#include "mo_email.h"
#include "scheduler_engine.h"
#include "gqtl_datakeys_content.h"

CGexMoTaskSYA::CGexMoTaskSYA(QString email, QObject *parent)
    : CGexMoTaskStatisticalMonitoring(parent)
{
    SetProperties(new GexMoStatisticalMonitoringTaskData);
    SetPrivateAttribute(C_EMAILTO, email);
    SetPrivateAttribute(C_STATSTOMONITOR, C_STATS_RATIO);
    SetPrivateAttribute(C_GROSSDIE, false);
    mUniqueKeyRule = useNum;
}

CGexMoTaskSYA::CGexMoTaskSYA(CGexMoTaskSYA *orig)
    : CGexMoTaskStatisticalMonitoring(orig)
{
    mUniqueKeyRule = useNum;
}

CGexMoTaskItem* CGexMoTaskSYA::Duplicate(QString copyName)
{
    this->LoadTaskDetails();
    CGexMoTaskSYA* clone = new CGexMoTaskSYA(this);
    clone->UpdateAttribute(C_TITLE, copyName);
    clone->UpdateAttribute(C_TASKID, -1);
    clone->UpdateAttribute(C_VERSION_ID, -1);
    clone->UpdateAttribute(C_VERSION_DRAFT, true);
    clone->UpdateAttribute(C_VERSION_ACTIVE_PROD_ID, -1);
    clone->UpdateAttribute(C_VERSION_LATEST_PROD_ID, -1);
    return clone;
}

CGexMoTaskStatisticalMonitoring* CGexMoTaskSYA::Clone()
{
    return new CGexMoTaskSYA(this);
}

CGexMoTaskSYA::~CGexMoTaskSYA()
{
}

int CGexMoTaskSYA::GetTaskType()
{
    return GEXMO_TASK_SYA;
}

QString CGexMoTaskSYA::GetTypeName()
{
    return "SYA";
}

bool CGexMoTaskSYA::ParseTaskOptions(const QMap<QString, QString>& taskOptions)
{

    QString lField;
    int lNbreField = 0;

    for(QMap<QString, QString>::const_iterator iter = taskOptions.constBegin(); iter != taskOptions.constEnd(); ++iter)
    {
        lField = iter.key();

        if( lField == C_TITLE || lField == C_DATABASE)
        {
            ++lNbreField;
            UpdateAttribute(lField, iter.value());
        }
        else if(lField == C_TASKID)
        {
            ++lNbreField;
            UpdateAttribute(lField, iter.value().toInt());
        }
        else if(lField == C_ACTIVEONTRIGGER || lField == C_ACTIVEONINSERT)
        {
            ++lNbreField;
            UpdateAttribute(lField, iter.value().toLower() == "true" ? true : false);
        }
        else if(lField == C_VERSION_LASTEXEC)
        {
            ++lNbreField;
            UpdateAttribute(lField, QDateTime::fromMSecsSinceEpoch(iter.value().toInt()));
        }
    }

    return (lNbreField == 6);
}

bool CGexMoTaskSYA::ParseOldTaskOptions(const QMap<QString, QString>& taskOptions)
{
    // the old method was LoadDbTaskSectionYieldMonitoring_OneRule

    mOldSYATaskParam = new CGexOldSYAParameters();
    QString lField;
    QString lValue;
    int lNbreField = 0;

    for(QMap<QString, QString>::const_iterator iter = taskOptions.constBegin(); iter != taskOptions.constEnd(); ++iter)
    {
        lField = iter.key();
        lValue  = iter.value();
        ++lNbreField;

        // Read Title
        if(lField == C_TITLE)
        {
            UpdateAttribute(lField, lValue);
        }

        // Read ProductID : 7170
        else if(lField == "ProductID")
        {
            UpdateAttribute(C_PRODUCTREGEXP, lValue);
        }

        else if(lField == "RenewalDateRange")
        {
            mOldSYATaskParam->mRenewalDateRange = lValue.toInt();
        }

        else if(lField == "RenewalDateRangeUnit")
        {
            mOldSYATaskParam->mRenewalDateRangeUnit = lValue.toInt();
        }

        // Read Yield Bin list
        else if(lField == "YieldBins")
        {
        }

        // Bin list type: 0=Good bins, 1=Failing bins.
        else if(lField == "BiningType")
        {
            mOldSYATaskParam->mBiningType = lValue.toInt();
        }

        // Read Alarm level (0-100%)
        else if(lField == "AlarmLevel")
        {
            mOldSYATaskParam->mAlarmLevel = lValue.toInt();
        }

        // Read Flag: Check if Yield OVER or UNDER the limit.
        else if(lField == "AlarmDirection")
        {
            mOldSYATaskParam->mAlarmIfOverLimit = lValue.toInt();
        }

        // Read Minimum parts to have a valid file
        else if(lField == "MinimumParts")
        {
            mOldSYATaskParam->mMinimumyieldParts = lValue.toLong();
        }

        // Read SBL/YBL data file (if exists)
        else if((lField == "SblFile")) //&& QFile::exists(strValue))
        {
            GSLOG(SYSLOG_SEV_WARNING, "SBL file no more supported.");  //ptYield->strSblFile = strValue;
        }

        else if(lField == C_EMAILFROM)
        {
            UpdateAttribute(lField, lValue);
        }

        // Read Email notification list
        else if(lField == "Emails")
        {
            UpdateAttribute(C_EMAILTO, lValue);
        }

        // Read Email format: HTML or TXT
        else if(lField == C_EMAILFORMAT)
        {
            if(lValue == "HTML")
            {
                UpdateAttribute(lField, "html");
            }
            else
            {
                UpdateAttribute(lField, "plain_text");
            }
        }

        // Read Email message contents type to send
        else if(lField == "EmailReportType")
        {
            mOldSYATaskParam->mEmailReportType = lValue.toInt();
        }

        // Read Email report notification type: send as attachment or leave on server
        else if(lField == "NotificationType")
        {
            mOldSYATaskParam->mNotificationType = lValue.toInt();
        }

        // Read Alarm type: Standard, Critical...
        else if(lField == "ExceptionLevel")
        {
            mOldSYATaskParam->mExceptionLevel = lValue.toInt();
        }

        /////////////////////////////////////////////////////////////////////////////
        // SYL-SBL specifics
        /////////////////////////////////////////////////////////////////////////////
        else if(lField == "ActiveOnDatafileInsertion")
        {
            UpdateAttribute(C_ACTIVEONINSERT, lValue.toLower() == "1" ? true : false);
        }

        else if(lField == "ActiveOnTriggerFile")
        {
            UpdateAttribute(C_ACTIVEONTRIGGER, lValue.toLower() == "1" ? true : false);
        }

        else if(lField == "BinRule")
        {
            int nBinNo;
            int nRuleType;

            bool bIsNumber;
            QStringList lRules = lValue.split(";");
            while(!lRules.isEmpty())
            {
                lValue = lRules.takeFirst();

                nBinNo = lValue.section("|",0,0).toInt(&bIsNumber);

                // If no error
                if(bIsNumber)
                {
                    nRuleType = lValue.section("|",1,1).toInt(&bIsNumber);

                    // Add the rule bin
                    mOldSYATaskParam->mMapBins_rules[nBinNo]["RuleType"] = nRuleType;
                    if(nRuleType == eManual)
                    {
                        mOldSYATaskParam->mMapBins_rules[nBinNo]["LL1"] = lValue.section("|",2,2).toFloat();
                        mOldSYATaskParam->mMapBins_rules[nBinNo]["HL1"] = lValue.section("|",3,3).toFloat();
                        mOldSYATaskParam->mMapBins_rules[nBinNo]["LL2"] = lValue.section("|",4,4).toFloat();
                        mOldSYATaskParam->mMapBins_rules[nBinNo]["HL2"] = lValue.section("|",5,5).toFloat();
                    }
                    else
                    {
                        mOldSYATaskParam->mMapBins_rules[nBinNo]["N1"] = lValue.section("|",2,2).toFloat();
                        mOldSYATaskParam->mMapBins_rules[nBinNo]["N2"] = lValue.section("|",3,3).toFloat();
                    }
                }
            }
        }

        else if(lField == "SYABinExclusion")
        {
            mOldSYATaskParam->mExcludedBins = lValue;
        }

        else if(lField == C_DATABASE)
        {
            UpdateAttribute(lField, lValue);
        }

        else if(lField == C_TESTINGSTAGE)
        {
            if(lValue.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Error: cannot migrate task %1 with an empty testing stage")
                                                .arg(QString::number(this->GetID())).toLatin1().constData());
                return false;
            }

            UpdateAttribute(lField, lValue);
        }

        else if(lField == "RuleType")
        {
            mOldSYATaskParam->mSYA_Rule = (OutlierRule)lValue.toInt();      // Rule: 0=N*Sigma, 1=N*IQR
        }

        else if(lField == "RuleTypeString")       // Rule string: N*Sigma, N*IQR
        {
            UpdateAttribute(C_DEFAULTALGO, ConvertRuleType(lValue));
        }

        else if(lField == "N_Parameter")
        {
            UpdateAttribute(C_N1, lValue);
        }

        else if(lField == "N1_Parameter")
        {
            UpdateAttribute(C_N1, lValue);
        }

        else if(lField == "N2_Parameter")
        {
            UpdateAttribute(C_N2, lValue);
        }

        else if(lField == "MinimumLotsRequired")
        {
            UpdateAttribute(C_MINLOTS, lValue);
        }

        else if(lField == "ValidityPeriod")
        {
            int lValidityPeriod;
            switch(lValue.toInt())
            {
                case 0: lValidityPeriod= 7;     // One week
                    break;
                case 1: lValidityPeriod= 30;    // One month
                    break;
                case 2: lValidityPeriod= 60;    // 2 lonths
                    break;
                case 3: lValidityPeriod= 90;    // 3 months
                    break;
                case 4: lValidityPeriod= 180;    // 6 months
                    break;
                case 5: lValidityPeriod= 365;   // 1 year
                    break;
                default: lValidityPeriod= 7;
                    break;
            }
            UpdateAttribute(C_VALIDITYPERIOD, lValidityPeriod);
        }

        else if(lField == "ExpirationAlgo")
        {
            mOldSYATaskParam->mExpirationAlgo = lValue.toInt();
        }

        else if(lField == "BinType")
        {
            if(lValue == "soft")
            {
                UpdateAttribute(C_MONITOREDITEMTYPE, "S");
            }
            else
            {
                UpdateAttribute(C_MONITOREDITEMTYPE, "H");
            }
        }


        else if(lField == "ExpirationDate")
        {
            // Historically, dates where stored in French format d M yyyy
            // While mutating ExpirationDate to Attribute, the QVariant defaulty stringify date in ISO format : yyyy-MM-dd
            // datetime in yyyy-MM-ddThh:mm:ss
            // Consequently, for the moment, lets support all formats.
            QDateTime lDate;
            lDate=lDate.fromString(lValue, "yyyy-MM-dd"); // ISO
            if (!lDate.isValid())
            {
                lDate=lDate.fromString(lValue, "yyyy-MM-ddThh:mm:ss");
                if (!lDate.isValid())
                {
                    // French parsing ?
                    int iDay,iMonth,iYear;
                    iDay = lValue.section(' ',0,0).trimmed().toInt();
                    iMonth = lValue.section(' ',1,1).trimmed().toInt();
                    iYear = lValue.section(' ',2,2).trimmed().toInt();
                    //ptYield->cExpiration.setDate(iYear,iMonth,iDay);
                    lDate = QDateTime(QDate(iYear,iMonth,iDay));
                }
            }
            mOldSYATaskParam->mExpirationDate = lDate.date();
        }

        else if(lField == "ExpirationWarned")
        {
            UpdateAttribute(C_VERSION_EXPIRATIONWARNINGDONE, (lValue == "true" ? 2 : 0));
        }

        else if(lField == "IgnoreDataPointsWithNullSigma")
        {
            mOldSYATaskParam->mSYA_IgnoreDataPointsWithNullSigma = (lValue == "1");
        }

        else if(lField == "IgnoreOutliers")
        {
            UpdateAttribute(C_DATACLEANING, (lValue == "1"));
        }

        else if(lField == "UseGrossDie")
        {
            UpdateAttribute(C_GROSSDIE, (lValue == "1"));
        }

        else if(lField == "MinDataPoints")
        {
            UpdateAttribute(C_MINDATAPOINTS, lValue);
        }

        // Read Last time task was executed...
        else if(lField == "LastExecuted")
        {
            // Saved as "yyyy-MM-dd hh:mm:ss"
            QDateTime lDateTime = QDateTime::fromString(lValue,"yyyy-MM-dd hh:mm:ss");
            UpdateAttribute(C_VERSION_LASTEXEC, lDateTime.toTime_t());
        }
    }

    return (lNbreField > 7);
}

const QMap<QString, QString>& CGexMoTaskSYA::GetTaskLightWeightOptions()
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetSYALightWeightOptions(this->GetAttribute(C_TASKID).toInt());
}

QString CGexMoTaskSYA::BuildTaskOptionsQuery()
{
    QMap<QString, QVariant> mapAttributes = this->GetAttributes();
    QMap<QString, QVariant>::const_iterator itAttribute;
    QStringList values;
    for(itAttribute = mapAttributes.begin(); itAttribute != mapAttributes.end(); itAttribute++)
    {
        if(
                itAttribute.key() == C_TITLE
                || itAttribute.key() == C_DATABASE
                || itAttribute.key() == C_TASKID
                || itAttribute.key() == C_ACTIVEONTRIGGER
                || itAttribute.key() == C_ACTIVEONINSERT
                || itAttribute.key() == C_VERSION_LASTEXEC
        )
        {
            values.append("(" + QString::number(this->GetID())
                          + ",'"+itAttribute.key()+"',"
                          + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSqlLob(itAttribute.value().toString())
                          + ")");
        }
    }
    return values.join(',');
}

bool CGexMoTaskSYA::MigrateTaskDetailsFromTdrToYmAdminDb()
{
    int lRecomputePeriod;

    switch(mOldSYATaskParam->mRenewalDateRangeUnit)
    {
    case 0: lRecomputePeriod = mOldSYATaskParam->mRenewalDateRange * 30;
        break;
    case 1: lRecomputePeriod = mOldSYATaskParam->mRenewalDateRange * 7;
        break;
    default:
        lRecomputePeriod = mOldSYATaskParam->mRenewalDateRange;
    }

    QSqlQuery ymQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString lQueryString;

    //////////////////////////////////////////////////
    // Get the next available task_id in ym_admin_db //
    //////////////////////////////////////////////////
    lQueryString = "SELECT IFNULL(MAX(task_id), 0)+1 as task_id FROM ym_sya" + SM_TABLE_MAIN;
    if(!ymQuery.exec(lQueryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     lQueryString,
                                                                     QString::number(ymQuery.lastError().number()),
                                                                     ymQuery.lastError().text());
        return false;
    }

    if(!ymQuery.first())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: query %1 returned no result").arg(lQueryString).toLatin1().constData());
        return false;
    }

    int lNewSyaId = ymQuery.value("task_id").toInt();
    UpdateAttribute(C_TASKID, lNewSyaId);
    QString lDBName = GetAttribute(C_DATABASE).toString();
    GexDatabaseEntry* lTDREntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().
                                    FindDatabaseEntry(lDBName,false);
    if (!lTDREntry)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: No database with this name %1")
                                .arg(lDBName).toLatin1().constData());
        return false;
    }
    QSqlQuery lTDRQuery(QSqlDatabase::database(lTDREntry->m_pExternalDatabase->GetPluginID()->
                                               m_pPlugin->m_pclDatabaseConnector->m_strConnectionName));

    // Get the database name to query the xx_sya_set and xx_SBL

    QString lRuleNameQuery = "lower(RULE_NAME)='"+ GetAttribute(C_TITLE).toString().toLower() + "'";


    QString lQuery =  "SELECT sya_id,ll_1, hl_1, ll_2, hl_2, n1_parameter, n2_parameter, creation_date,user_comment,start_date,expiration_date,rule_type,computation_fromdate,computation_todate,min_lots_required,min_data_points,rule_name "
                      "FROM " + GetTDRTablePrefix() + "_sya_set";
    lQuery += " WHERE lower(PRODUCT_ID)='"+ GetAttribute(C_PRODUCTREGEXP).toString() + "'";
    lQuery += " AND "+lRuleNameQuery;
    lQuery += " ORDER BY CREATION_DATE DESC";

    if(!lTDRQuery.exec(lQuery))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     lQuery,
                                                                     QString::number(lTDRQuery.lastError().number()),
                                                                     lTDRQuery.lastError().text());
        return false;
    }

    if(!lTDRQuery.first())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Warning: No limits validated for task %1, creating task without limits")
                                         .arg(QString::number(this->GetID())).toLatin1().constData());

        // Create task definition without limits

        // ym_sya
        lQueryString = "INSERT INTO ym_sya" + SM_TABLE_MAIN + "(task_id,testing_stage,product_regexp,monitored_item_type,monitored_item_regexp,test_flow,consolidation_type,consolidation_aggregation_level,consolidation_name_regexp,site_merge_mode,stats_to_monitor,min_lots,min_datapoints,use_gross_die,default_algorithm,remove_outliers,validity_period,days_before_expiration,send_email_before_expiration,auto_recompute,auto_recompute_method,auto_recompute_period,email_format,email_from,email_report_type,emails) VALUES (";
        lQueryString += QString::number(lNewSyaId) + ",";
        lQueryString += "'" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(GetAttribute(C_TESTINGSTAGE).toString(), false) + "',";
        lQueryString += "'" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(GetAttribute(C_PRODUCTREGEXP).toString(), false) + "',";
        lQueryString += "'" + GetAttribute(C_MONITOREDITEMTYPE).toString() + "',";
        lQueryString += "'',";
        lQueryString += "'P',";
        lQueryString += "'" + QString(GEXDB_CONSO_TYPE_CONSO) + "',";
        lQueryString += "'" + QString(GEXDB_CONSO_LEVEL_INSERT) + "',";
        lQueryString += "'*',";
        lQueryString += "'" + QString(GEX_MERGE_MERGE) + "',";
        lQueryString += "'" + C_STATS_RATIO + "',";
        lQueryString += QString::number(GetAttribute(C_MINLOTS).toInt()) + ",";
        lQueryString += QString::number(GetAttribute(C_MINDATAPOINTS).toInt()) + ",";
        lQueryString += QString(GetAttribute(C_GROSSDIE).toBool() ? "1" : "0") + ",";
        lQueryString += "'" + GetAttribute(C_DEFAULTALGO).toString() + "',";
        lQueryString += QString(GetAttribute(C_DATACLEANING).toBool() ? "1" : "0") + ",";
        lQueryString += QString::number(GetAttribute(C_VALIDITYPERIOD).toInt()) + ",";
        lQueryString += QString::number(mOldSYATaskParam->mExpirationWarningReprieve) + ",";
        lQueryString += QString((mOldSYATaskParam->mExpirationAlgo >= 1)? "1" : "0") + ",";
        lQueryString += QString((mOldSYATaskParam->mExpirationAlgo == 2)? "1" : "0") + ",";
        lQueryString += "'duplicateIfRecomputeFails',";
        lQueryString += QString::number(lRecomputePeriod) + ",";
        lQueryString += "'" + this->GetAttribute(C_EMAILFORMAT).toString() + "',";
        lQueryString += "'" + this->GetAttribute(C_EMAILFROM).toString() + "',";
        lQueryString += "'" + QString((mOldSYATaskParam->mExceptionLevel == 0)? "standard" : "critical") + "',";
        lQueryString += "'" + GetAttribute(C_EMAILTO).toString() + "')";
        if(!ymQuery.exec(lQueryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         lQueryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }

        // xx_sya_default_params
        QStringList values;
        for(int criticityLevel = 1; criticityLevel <= 2; ++ criticityLevel)
        {
            values.append("("
                          + QString::number(lNewSyaId) + ","
                          + QString::number(criticityLevel) + ","
                          + "'N',"
                          + QString::number(GetAttribute(C_N + QString::number(criticityLevel)).toInt())
                          + ")");
        }
        if(values.length() > 0)
        {
            lQueryString = "INSERT INTO ym_sya" + SM_TABLE_DEF_PARAMS
                        + "(task_id,criticity_level,param_name,param_value) VALUES "
                        + values.join(',');
            if(!ymQuery.exec(lQueryString))
            {
                GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                             lQueryString,
                                                                             QString::number(ymQuery.lastError().number()),
                                                                             ymQuery.lastError().text());
                return false;
            }
        }

        lQueryString = "INSERT INTO ym_sya" + SM_TABLE_VERSION
                    + "(task_id,version_id,draft_version,version_label,matched_products,creation_date,start_date,expiration_date,expiration_warning_date,expiration_warning_done,computation_fromdate,computation_todate) VALUES ("
                      + QString::number(lNewSyaId) + ","
                      "1,"
                      "1,"
                      + "'migrated from previous SYA task without computed limits',"
                      + "'" + GetAttribute(C_PRODUCTREGEXP).toString() + "',"
                      + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_CREATIONDATE).toDateTime()) + ","
                      + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_STARTDATE).toDateTime()) + ","
                      + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_EXPIRATIONDATE).toDateTime()) + ","
                      + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_EXPIRATIONWARNINGDATE).toDateTime()) + ","
                      + QString::number(GetAttribute(C_VERSION_EXPIRATIONWARNINGDONE).toInt()) + ","
                      + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_COMPUTEFROM).toDateTime()) + ","
                      + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_COMPUTETO).toDateTime())
                      + ")";
        if(!ymQuery.exec(lQueryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         lQueryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }

        return true;
    }

    QStringList lBins;
    int lLastSYAVersion               = lTDRQuery.value("sya_id").toInt();
    QString lCreationDate             = lTDRQuery.value("creation_date").toString();
    QString lStartDate                = lTDRQuery.value("start_date").toString();
    QDateTime lExpirationDate         = lTDRQuery.value("expiration_date").toDateTime();
    QDateTime lExpirationWarningDate  = lExpirationDate.addDays(-GetAttribute(C_DAYSBEFOREEXPIRATION).toInt());
    QString lComputeFromDate          = lTDRQuery.value("computation_fromdate").toString();
    QString lComputeToDate            = lTDRQuery.value("computation_todate").toString();
    QString monitoredItemType         = GetAttribute(C_MONITOREDITEMTYPE).toString().toLower();

    double ll[2];
    bool llEnab[2];
    double hl[2];
    bool hlEnab[2];
    if(lTDRQuery.value("ll_1").toDouble() == -1)
    {
        ll[0] = 0;
        llEnab[0] = false;
    }
    else
    {
        ll[0] = lTDRQuery.value("ll_1").toDouble();
        llEnab[0] = true;
    }
    if(lTDRQuery.value("ll_2").toDouble() == -1)
    {
        ll[1] = 0;
        llEnab[1] = false;
    }
    else
    {
        ll[1] = lTDRQuery.value("ll_2").toDouble();
        llEnab[1] = true;
    }
    if(lTDRQuery.value("hl_1").toDouble() == -1)
    {
        hl[0] = 100;
        hlEnab[0] = false;
    }
    else
    {
        hl[0] = lTDRQuery.value("hl_1").toDouble();
        hlEnab[0] = true;
    }
    if(lTDRQuery.value("hl_2").toDouble() == -1)
    {
        hl[1] = 100;
        hlEnab[1] = false;
    }
    else
    {
        hl[1] = lTDRQuery.value("hl_2").toDouble();
        hlEnab[1] = true;
    }
    double nParam[2];
    nParam[0] = lTDRQuery.value("n1_parameter").toDouble();
    nParam[1] = lTDRQuery.value("n2_parameter").toDouble();
    QString ruleType = ConvertRuleType(lTDRQuery.value("rule_type").toString());
    QString userComment = lTDRQuery.value("user_comment").toString();

    int lNewMonitoringItemId = 0;
    QStringList lLimits;
    QStringList lLimitMonitoredItems;
    QStringList lMonitoredItems;
    QStringList lLimitParams;
    int lLimitId = 0;

    // Prepare a monitored item and some limits for the global yield

    lBins.append("-1");

    lMonitoredItems.append(
                "("
                + QString::number(lNewSyaId) + ","
                + QString::number(++lNewMonitoringItemId) + ","
                + "'" + monitoredItemType + "',"
                + "'-1',"
                + "'Yield',"
                + "'%',"
                + "0,"
                + "'P'"
                + ")"
            );
    for(int criticityLevel = 1; criticityLevel <= 2; ++criticityLevel)
    {
        ++lLimitId;
        for(int versionId = 1; versionId <= 2; ++versionId)
        {
            lLimits.append(
                        "("
                        + QString::number(lNewSyaId) + ","
                        + QString::number(versionId) + ","
                        + QString::number(lLimitId) + ","
                        + "-1,"// merged site
                        + QString::number(criticityLevel) + ","
                        + "'" + C_STATS_RATIO + "',"
                        + "1,"
                        + (llEnab[criticityLevel - 1] ? "1," : "0,")
                        + QString::number(ll[criticityLevel - 1]) + ","
                        + (hlEnab[criticityLevel - 1] ? "1," : "0,")
                        + QString::number(hl[criticityLevel - 1]) + ","
                        + "'" + ruleType + "',"
                        + "1," // enabled
                        + "0" // recompute
                        + ")");
            lLimitMonitoredItems.append(
                        "("
                        + QString::number(lNewSyaId) + ","
                        + QString::number(versionId) + ","
                        + QString::number(lLimitId) + ","
                        + QString::number(lNewMonitoringItemId)
                        + ")");
            lLimitParams.append("("
                                + QString::number(lNewSyaId) + ","
                                + QString::number(versionId) + ","
                                + QString::number(lLimitId) + ","
                                + "'N',"
                                + QString::number(nParam[criticityLevel - 1])
                                + ")");
        }
    }

    // Get the previous bins from the SBL table to fill the new sya_limit_parameter and sya_limit

    lQuery = "SELECT binL.bin_no, binL.bin_name, bin.bin_cat, binL.ll_1, binL.hl_1, binL.ll_2, binL.hl_2, binL.rule_type, binL.n1_parameter, binL.n2_parameter";
    lQuery += " FROM " + GetTDRTablePrefix() + "_sbl binL";
    lQuery += " INNER JOIN (SELECT DISTINCT "+ monitoredItemType + "bin_no AS bin_no, "+ monitoredItemType + "bin_cat AS bin_cat FROM " + GetTDRTablePrefix() + "_lot_" + monitoredItemType + "bin WHERE product_name='" + GetAttribute(C_PRODUCTREGEXP).toString() + "') bin";
    lQuery += " ON binL.bin_no = bin.bin_no";
    lQuery += " WHERE binL.sya_id = " + QString::number(lLastSYAVersion);

    if(!lTDRQuery.exec(lQuery))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     lQuery,
                                                                     QString::number(lTDRQuery.lastError().number()),
                                                                     lTDRQuery.lastError().text());
        return false;
    }

    while(lTDRQuery.next())
    {
        int lBinNo = lTDRQuery.value("bin_no").toInt();
        lBins.append(QString::number(lBinNo));

        if(lTDRQuery.value("ll_1").toDouble() == -1)
        {
            ll[0] = 0;
            llEnab[0] = false;
        }
        else
        {
            ll[0] = lTDRQuery.value("ll_1").toDouble();
            llEnab[0] = true;
        }
        if(lTDRQuery.value("ll_2").toDouble() == -1)
        {
            ll[1] = 0;
            llEnab[1] = false;
        }
        else
        {
            ll[1] = lTDRQuery.value("ll_2").toDouble();
            llEnab[1] = true;
        }
        if(lTDRQuery.value("hl_1").toDouble() == -1)
        {
            hl[0] = 100;
            hlEnab[0] = false;
        }
        else
        {
            hl[0] = lTDRQuery.value("hl_1").toDouble();
            hlEnab[0] = true;
        }
        if(lTDRQuery.value("hl_2").toDouble() == -1)
        {
            hl[1] = 100;
            hlEnab[1] = false;
        }
        else
        {
            hl[1] = lTDRQuery.value("hl_2").toDouble();
            hlEnab[1] = true;
        }
        int nSpecificParam[2];
        nSpecificParam[0] = lTDRQuery.value("n1_parameter").isNull() ? nParam[0] : lTDRQuery.value("n1_parameter").toDouble();
        nSpecificParam[1] = lTDRQuery.value("n2_parameter").isNull() ? nParam[1] : lTDRQuery.value("n2_parameter").toDouble();
        QString specificRuleType = lTDRQuery.value("rule_type").isNull() ? ruleType : ConvertRuleType(lTDRQuery.value("rule_type").toString());

        lMonitoredItems.append(
            "("
            + QString::number(lNewSyaId) + ","
            + QString::number(++lNewMonitoringItemId) + ","
            + "'" + monitoredItemType + "',"
            + "'" + QString::number(lBinNo) + "',"
            + "'" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(lTDRQuery.value("bin_name").toString(), false) + "',"
            + "'%',"
            + "0,"
            + "'" + lTDRQuery.value("bin_cat").toString().toUpper() + "'"
            + ")"
        );

        for(int criticityLevel = 1; criticityLevel <= 2; ++criticityLevel)
        {
            ++lLimitId;
            for(int versionId = 1; versionId <= 2; ++versionId)
            {
                lLimits.append(
                            "("
                            + QString::number(lNewSyaId) + ","
                            + QString::number(versionId) + ","
                            + QString::number(lLimitId) + ","
                            + "-1,"// merged site
                            + QString::number(criticityLevel) + ","
                            + "'" + C_STATS_RATIO + "',"
                            + "1,"
                            + (llEnab[criticityLevel - 1] ? "1," : "0,")
                            + QString::number(ll[criticityLevel - 1]) + ","
                            + (hlEnab[criticityLevel - 1] ? "1," : "0,")
                            + QString::number(hl[criticityLevel - 1]) + ","
                            + "'" + specificRuleType + "',"
                            + "1," // enabled
                            + "0" // recompute
                            + ")");
                lLimitMonitoredItems.append(
                            "("
                            + QString::number(lNewSyaId) + ","
                            + QString::number(versionId) + ","
                            + QString::number(lLimitId) + ","
                            + QString::number(lNewMonitoringItemId)
                            + ")");
                lLimitParams.append("("
                                    + QString::number(lNewSyaId) + ","
                                    + QString::number(versionId) + ","
                                    + QString::number(lLimitId) + ","
                                    + "'N',"
                                    + QString::number(nSpecificParam[criticityLevel - 1])
                                    + ")");
            }
        }
    }

    //////////////////////////////////////////////////////////////
    // Duplicate the rows from tdr tables to ym_admin_db tables //
    //////////////////////////////////////////////////////////////
    // xx_sya

    QString lBinList = lBins.join(',');
    if(!mOldSYATaskParam->mExcludedBins.isEmpty())
    {
        lBinList += ",!" + mOldSYATaskParam->mExcludedBins.replace(',', ",!");
    }

    lQueryString = "INSERT INTO ym_sya" + SM_TABLE_MAIN + "(task_id,testing_stage,product_regexp,monitored_item_type,monitored_item_regexp,test_flow,consolidation_type,consolidation_aggregation_level,consolidation_name_regexp,site_merge_mode,stats_to_monitor,min_lots,min_datapoints,use_gross_die,default_algorithm,remove_outliers,validity_period,days_before_expiration,send_email_before_expiration,auto_recompute,auto_recompute_method,auto_recompute_period,email_format,email_from,email_report_type,emails) VALUES (";
    lQueryString += QString::number(lNewSyaId) + ",";
    lQueryString += "'" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(GetAttribute(C_TESTINGSTAGE).toString(), false) + "',";
    lQueryString += "'" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(GetAttribute(C_PRODUCTREGEXP).toString(), false) + "',";
    lQueryString += "'" + monitoredItemType + "',";
    lQueryString += "'" + lBinList + "',";
    lQueryString += "'P',";
    lQueryString += "'" + QString(GEXDB_CONSO_TYPE_CONSO) + "',";
    lQueryString += "'" + QString(GEXDB_CONSO_LEVEL_INSERT) + "',";
    lQueryString += "'*',";
    lQueryString += "'" + QString(GEX_MERGE_MERGE) + "',";
    lQueryString += "'" + C_STATS_RATIO + "',";
    lQueryString += QString::number(GetAttribute(C_MINLOTS).toInt()) + ",";
    lQueryString += QString::number(GetAttribute(C_MINDATAPOINTS).toInt()) + ",";
    lQueryString += QString(GetAttribute(C_GROSSDIE).toBool() ? "1" : "0") + ",";
    lQueryString += "'" + GetAttribute(C_DEFAULTALGO).toString() + "',";
    lQueryString += QString(GetAttribute(C_DATACLEANING).toBool() ? "1" : "0") + ",";
    lQueryString += QString::number(GetAttribute(C_VALIDITYPERIOD).toInt()) + ",";
    lQueryString += QString::number(mOldSYATaskParam->mExpirationWarningReprieve) + ",";
    lQueryString += QString((mOldSYATaskParam->mExpirationAlgo >= 1)? "1" : "0") + ",";
    lQueryString += QString((mOldSYATaskParam->mExpirationAlgo == 2)? "1" : "0") + ",";
    lQueryString += "'duplicateIfRecomputeFails',";
    lQueryString += QString::number(lRecomputePeriod) + ",";
    lQueryString += "'" + this->GetAttribute(C_EMAILFORMAT).toString() + "',";
    lQueryString += "'" + this->GetAttribute(C_EMAILFROM).toString() + "',";
    lQueryString += "'" + QString((mOldSYATaskParam->mExceptionLevel == 0)? "standard" : "critical") + "',";
    lQueryString += "'" + GetAttribute(C_EMAILTO).toString() + "')";
    if(!ymQuery.exec(lQueryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     lQueryString,
                                                                     QString::number(ymQuery.lastError().number()),
                                                                     ymQuery.lastError().text());
        return false;
    }

    // xx_sya_default_params
    QStringList values;
    for(int criticityLevel = 1; criticityLevel <= 2; ++ criticityLevel)
    {
        values.append("("
                      + QString::number(lNewSyaId) + ","
                      + QString::number(criticityLevel) + ","
                      + "'N',"
                      + QString::number(GetAttribute(C_N + QString::number(criticityLevel)).toInt())
                      + ")");
    }
    if(values.length() > 0)
    {
        lQueryString = "INSERT INTO ym_sya" + SM_TABLE_DEF_PARAMS
                    + "(task_id,criticity_level,param_name,param_value) VALUES "
                    + values.join(',');
        if(!ymQuery.exec(lQueryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         lQueryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    // xx_sya_filters
    // No filter to migrate

    // xx_sya_version
    values.clear();
    for(int versionId = 1; versionId <= 2; ++versionId)
    {
        values.append(
                    "("
                    + QString::number(lNewSyaId) + ","
                    + QString::number(versionId) + ","
                    + (versionId == 1 ? "0," : "1,")
                    + "'migrated from previous SYA format: " + userComment + "',"
                    + "'" + GetAttribute(C_PRODUCTREGEXP).toString() + "',"
                    + "'" + lCreationDate + "',"
                    + "'" + lStartDate + "',"
                    + "'" + lExpirationDate.toString("yyyy-MM-dd hh:mm:ss") + "',"
                    + "'" + lExpirationWarningDate.toString("yyyy-MM-dd hh:mm:ss") + "',"
                    + QString::number(GetAttribute(C_VERSION_EXPIRATIONWARNINGDONE).toInt()) + ","
                    + "'" + QDateTime::fromString(lComputeFromDate, "yyyy-MM-dd").toString("yyyy-MM-dd hh:mm:ss") + "',"
                    + "'" + QDateTime::fromString(lComputeToDate, "yyyy-MM-dd").toString("yyyy-MM-dd hh:mm:ss") + "'"
                    + ")"
                    );
    }

    if(values.length() > 0)
    {
        lQueryString = "INSERT INTO ym_sya" + SM_TABLE_VERSION
                    + "(task_id,version_id,draft_version,version_label,matched_products,creation_date,start_date,expiration_date,expiration_warning_date,expiration_warning_done,computation_fromdate,computation_todate) VALUES "
                    + values.join(',');
        if(!ymQuery.exec(lQueryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         lQueryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    // xx_sya_monitored_item

    if(lMonitoredItems.length() > 0)
    {
        lQueryString = "INSERT INTO ym_sya" + SM_TABLE_MONITORED_ITEM
                    + "(task_id,monitored_item_id,monitored_item_type,monitored_item_num,monitored_item_name,monitored_item_unit,monitored_item_scale,monitored_item_cat) VALUES "
                    + lMonitoredItems.join(',');
        if(!ymQuery.exec(lQueryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         lQueryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    // xx_sya_limit

    if(lLimits.length() > 0)
    {
        lQueryString = "INSERT INTO ym_sya" + SM_TABLE_LIMIT
                    + "(task_id,version_id,limit_id,site_no,criticity_level,stat_name,has_unit,"
                    + "ll_enabled,ll,hl_enabled,hl,algorithm,enabled,recompute) VALUES "
                    + lLimits.join(',');
        if(!ymQuery.exec(lQueryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         lQueryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    // xx_sya_limit_monitored_item

    if(lLimitMonitoredItems.length() > 0)
    {
        lQueryString = "INSERT INTO ym_sya" + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM
                    + "(task_id,version_id,limit_id,monitored_item_id) VALUES "
                    + lLimitMonitoredItems.join(',');
        if(!ymQuery.exec(lQueryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         lQueryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    // xx_sya_limit_param
    if(lLimitParams.length() > 0)
    {
        lQueryString = "INSERT INTO ym_sya" + SM_TABLE_LIMIT_PARAM
                    + "(task_id,version_id,limit_id,param_name,param_value) VALUES "
                    + lLimitParams.join(',');
        if(!ymQuery.exec(lQueryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         lQueryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    return true;
}

QString CGexMoTaskSYA::GetStatisticalMonitoringTablesPrefix()
{
    return "ym_sya";
}

bool CGexMoTaskSYA::CheckComputeSpecificRequirements()
{
    return true;
}

bool CGexMoTaskSYA::FetchDataPointsForComputing(
        QString testingStage,
        QString productRegexp,
        QMap<QString, QString> filters,
        QString monitoredItemType,
        QList<MonitoredItemRule> monitoredItemRules,
        QStringList excludedItems,
        MonitoredItemUniqueKeyRule uniqueKeyRule,
        QString testFlow,
        QString consolidationType,
        QString consolidationLevel,
        QString testInsertion,
        QStringList statsToMonitor,
        QString siteMergeMode,
        bool useGrossDie,
        QDateTime computeFrom,
        QDateTime computeTo,
        QStringList& productList,
        int& numLots,
        int& numDataPoints,
        QSet<int>& siteList,
        QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& monitoredItemToSiteToStatToValues)
{
    GexDatabaseEntry* pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(GetAttribute(C_DATABASE).toString(),false);
    // Do nothing if database not found
    if(!pDatabaseEntry)
    {
        // Cannot find database name
        return false;
    }
    if(!pDatabaseEntry->m_pExternalDatabase->SYA_FetchDataPointsForComputing(testingStage,
                                                                             productRegexp,
                                                                             filters,
                                                                             monitoredItemType,
                                                                             monitoredItemRules,
                                                                             excludedItems,
                                                                             uniqueKeyRule,
                                                                             testFlow,
                                                                             consolidationType,
                                                                             consolidationLevel,
                                                                             testInsertion,
                                                                             statsToMonitor,
                                                                             siteMergeMode,
                                                                             useGrossDie,
                                                                             computeFrom,
                                                                             computeTo,
                                                                             productList,
                                                                             numLots,
                                                                             numDataPoints,
                                                                             siteList,
                                                                             monitoredItemToSiteToStatToValues))
    {
        return false;
    }

    return true;
}

void CGexMoTaskSYA::ProcessSpecificCleanLimits(const QString &itemCat, double &ll, bool &llEnabled, double &hl, bool &hlEnabled)
{
    if(itemCat == "P")
    {
        //hl = 100;
        hlEnabled = false;
    }
    else if(itemCat == "F")
    {
        //ll = 0;
        llEnabled = false;
    }
}

bool CGexMoTaskSYA::FetchDataPointsForCheck(QChar ExecutionType,
                                            QString testingStage,
                                            QString productName,
                                            QString lot,
                                            QString sublot,
                                            QString wafer,
                                            int splitlot,
                                            QMap<QString, QString> filters,
                                            QList<MonitoredItemDesc> monitoredItemList,
                                            QStringList excludedItems,
                                            MonitoredItemUniqueKeyRule uniqueKeyRule,
                                            QString testFlow,
                                            QString consolidationType,
                                            QString consolidationLevel,
                                            QString testInsertion,
                                            QList<int> siteList,
                                            QStringList statList,
                                            bool useGrossDie,
                                            const QDateTime* dateFrom,
                                            const QDateTime* dateTo,
                                            QString& resolvedProductList,
                                            QString& resolvedLotList,
                                            QString& resolvedSublotList,
                                            QString& resolvedWaferList,
                                            int& resolvedNumParts,
                                            QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > > &monitoredItemToSiteToStatToDataPoint)
{
    GexDatabaseEntry* pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(GetAttribute(C_DATABASE).toString(),false);
    // Do nothing if database not found
    if(!pDatabaseEntry)
    {
        // Cannot find database name
        return false;
    }
    if(ExecutionType != 'I')
    {
        if(!pDatabaseEntry->m_pExternalDatabase->SYA_FetchDataPointsForCheckOnTrigger(testingStage,
                                                                                      productName,
                                                                                      lot,
                                                                                      sublot,
                                                                                      wafer,
                                                                                      filters,
                                                                                      monitoredItemList,
                                                                                      excludedItems,
                                                                                      uniqueKeyRule,
                                                                                      testFlow,
                                                                                      consolidationType,
                                                                                      consolidationLevel,
                                                                                      testInsertion,
                                                                                      siteList,
                                                                                      statList,
                                                                                      useGrossDie,
                                                                                      dateFrom,
                                                                                      dateTo,
                                                                                      resolvedProductList,
                                                                                      resolvedLotList,
                                                                                      resolvedSublotList,
                                                                                      resolvedWaferList,
                                                                                      resolvedNumParts,
                                                                                      monitoredItemToSiteToStatToDataPoint))
        {
            GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrNoDataPointsError, NULL, GGET_LASTERRORMSG(GexRemoteDatabase, pDatabaseEntry->m_pExternalDatabase));

            return false;
        }
    }
    else
    {
        if(!pDatabaseEntry->m_pExternalDatabase->SYA_FetchDataPointsForCheckOnInsertion(testingStage,
                                                                                        splitlot,
                                                                                        filters,
                                                                                        monitoredItemList,
                                                                                        excludedItems,
                                                                                        uniqueKeyRule,
                                                                                        siteList,
                                                                                        statList,
                                                                                        useGrossDie,
                                                                                        resolvedProductList,
                                                                                        resolvedLotList,
                                                                                        resolvedSublotList,
                                                                                        resolvedWaferList,
                                                                                        resolvedNumParts,
                                                                                        monitoredItemToSiteToStatToDataPoint))
        {
            GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrNoDataPointsError, NULL, GGET_LASTERRORMSG(GexRemoteDatabase, pDatabaseEntry->m_pExternalDatabase));

            return false;
        }
    }

    return true;
}

CGexMoTaskItem* CGexMoTaskSYA::CreateTask(SchedulerGui *gui, bool readOnly)
{
    this->LoadTaskDetails();
    return gui->CreateTaskSYA(this, readOnly);
}

bool CGexMoTaskSYA::ExceedLimits(double value, double lowLimit, bool lowLimitEnabled, double highLimit, bool highLimitEnabled, double threshold , const QString &categorie)
{
    // Do not raise an alarm for fail bins if a min alarm threshold is defined
    if(!categorie.contains('P')
       && threshold > 0.
       && value < threshold)
    {
        return false;
    }

    return ((lowLimitEnabled && (value < lowLimit)) || (highLimitEnabled && (value > highLimit)));
}

void CGexMoTaskSYA::UpdateListView(SchedulerGui *gui, int nCurrentRow, bool allowEdit)
{
    if(gui)
    {
        gui->UpdateListViewSYA(this, nCurrentRow, allowEdit);
    }
}

QString CGexMoTaskSYA::ExecuteTask(GS::QtLib::DatakeysContent &aDbKeysContent, const QString& aMailFilePath)
{
    QString lProductName = aDbKeysContent.Get("Product").toString();
    QString lLot = aDbKeysContent.Get("Lot").toString();
    QString lSubLotId = aDbKeysContent.Get("SubLot").toString();
    QString lWaferId = aDbKeysContent.Get("Wafer").toString();
    QChar lExecutionType = QChar('T'); // for Trigger execution
    if(aDbKeysContent.Get("InsertionStatus").toString() == "PASS")
    {
        lExecutionType = QChar('I'); // for Datapump Insertion execution
    }

    // On insertion, do the check on the current splitlot_id
    long lSplitlotId = -1;
    if(lExecutionType == QChar('I'))
    {
        lSplitlotId = aDbKeysContent.Get("SplitlotId").toLongLong();
    }

    QString lMessageStatus;
    // Check task for this product
    QMap<QString, QVariant> logSummary;
    QList<StatMonAlarm> alarms;
    QHash<MonitoredItemDesc, QMap<int, StatMonLimit> > lItemsLimits;
    QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > > lMonitoredItemToSiteToStatToDataPoint;
    if(!CheckAgainstLimits(
                lProductName,
                lLot,
                lSubLotId,
                lWaferId,
                lSplitlotId,
                lExecutionType,
                logSummary,
                alarms,
                lMonitoredItemToSiteToStatToDataPoint,
                lItemsLimits))
    {
        lMessageStatus = GGET_LASTERRORMSG(CGexMoTaskStatisticalMonitoring, this);
        GSLOG(SYSLOG_SEV_ERROR, lMessageStatus.toLatin1().data() );
        lMessageStatus = "delay: "+lMessageStatus;
        return lMessageStatus;
    }

    aDbKeysContent.Set(GetTypeName() + "Status","PASS");
    aDbKeysContent.Set(GetTypeName() + "Info","");
    GexMoBuildEmailString lEmailString;

    QString lTaskDataSet;
    if(lExecutionType == QChar('I'))
        lTaskDataSet += "Testing date       : " + QString(TimeStringUTC_F(aDbKeysContent.Get("StartTime").toUInt(), "d MMMM yyyy h:mm:ss")) + "\n";
    lTaskDataSet += "Product            : " + lProductName + "\n";
    lTaskDataSet += "Lot                : " + lLot + "\n";
    lTaskDataSet += "Sublot             : " + lSubLotId + "\n";
    lTaskDataSet += "Wafer              : " + lWaferId + "\n";
    if(lExecutionType == QChar('I'))
    {
        lTaskDataSet += "Splitlot           : " + QString::number(lSplitlotId) + "\n";
        lTaskDataSet += "Tester             : " + (aDbKeysContent.Get("TesterName").toString().isEmpty() ?
                                                       QString("n/a\n") : QString(aDbKeysContent.Get("TesterName").toString() + "\n"));
        lTaskDataSet += "Operator           : " + (aDbKeysContent.Get("Operator").toString().isEmpty() ?
                                                       QString("n/a\n") : QString(aDbKeysContent.Get("Operator").toString() + "\n"));
        lTaskDataSet += "Program name       : " + (aDbKeysContent.Get("ProgramName").toString().isEmpty() ?
                                                       QString("n/a\n") : QString(aDbKeysContent.Get("ProgramName").toString() + "\n"));
        lTaskDataSet += "Program revision   : " + (aDbKeysContent.Get("ProgramRevision").toString().isEmpty() ?
                                                       QString("n/a\n") : QString(aDbKeysContent.Get("ProgramRevision").toString() + "\n"));
    }
    else
    {
        // On trigger
        lTaskDataSet += "Matched lots       : " + logSummary["matched_lots"].toString() + "\n";
        lTaskDataSet += "Matched sublots    : " + logSummary["matched_sublots"].toString() + "\n";
        lTaskDataSet += "Matched wafers     : " + logSummary["matched_wafers"].toString() + "\n";
    }
    lTaskDataSet += "Total parts tested : " + logSummary["nb_parts"].toString() + "\n";
    lTaskDataSet += "Alarm level        : " + logSummary["summary"].toString() + "\n";

    QString lTaskDetails;
    // Table with task details
    lTaskDetails += "Task name          : " + GetAttribute(C_TITLE).toString() + "\n";
    lTaskDetails += "Execution date     : " + logSummary["execution_date"].toDateTime().toString("d MMMM yyyy h:mm:ss") + "\n";
    lTaskDetails += "Version            : " + logSummary["version_id"].toString() + "\n";
    lTaskDetails += "Version comment    : " + logSummary["_sm_version_label"].toString() + "\n";
    lTaskDetails += "Version expiration : " + logSummary["_sm_version_expiration"].toDateTime().toString("d MMMM yyyy") + "\n";
    lTaskDetails += "Default algorithm  : " + GetAttribute(C_DEFAULTALGO).toString() + "\n";
    lTaskDetails += "N1-Value           : " + QString::number(GetAttribute(C_N1).toInt()) + "\n";
    lTaskDetails += "N2-Value           : " + QString::number(GetAttribute(C_N2).toInt()) + "\n";
    lTaskDetails += "Data cleaning      : " + (GetAttribute(C_DATACLEANING).toBool() ? QString("yes") : QString("no")) + "\n";
    lTaskDetails += "Use gross die      : " + (GetAttribute(C_GROSSDIE).toBool() ? QString("yes") : QString("no")) + "\n";


    QString lTextBody;
    // Always generates the TEXT Email Body for Log file
    lTextBody = "#### DATASET ##################################################\n";
    lTextBody+= lTaskDataSet;

    // Table with task details
    lTextBody += "\n#### " + GetTypeName() + " ######################################################\n";
    lTextBody += lTaskDetails;

    int lMaxCriticity = 0;

    if(alarms.count() != 0)
    {
        aDbKeysContent.Set(GetTypeName() + "Status","ALARM");

        QString lAlarmInfo;
        QStringList lAlarmsInfoList;
        foreach(const StatMonAlarm& alarm, alarms)
        {
            lMaxCriticity = qMax(alarm.criticityLevel, lMaxCriticity);

            lAlarmInfo  = "Product            : " + alarm.productName + "\n";
            lAlarmInfo += "Lot                : " + alarm.lotId + "\n";
            lAlarmInfo += "Sublot             : " + alarm.sublotId + "\n";
            lAlarmInfo += "Wafer              : " + alarm.waferId + "\n";

            if(lExecutionType == QChar('I'))
            {
                lAlarmInfo += "Splitlot           : " + QString::number(alarm.splitlotId) + "\n";
            }
            lAlarmInfo += GetMonitoredItemDesignation() + " type          : " + alarm.monitoredItemType + "\n";
            lAlarmInfo += GetMonitoredItemDesignation() + " number        : " + alarm.monitoredItemNum + "\n";
            lAlarmInfo += GetMonitoredItemDesignation() + " name          : " + alarm.monitoredItemName + "\n";
            lAlarmInfo += "Unit               : " + alarm.monitoredItemUnit + "\n";
            lAlarmInfo += "Stat name          : " + alarm.statName + "\n";
            lAlarmInfo += "Site number        : " + (alarm.siteNum == -1 ? "All" : QString::number(alarm.siteNum)) + "\n";
            lAlarmInfo += "Alarm criticity    : " + QString(alarm.criticityLevel == 2 ? "critical" : "standard") + "\n";
            lAlarmInfo += "Low limit          : " + QString::number(alarm.lowLimit) + "\n";
            lAlarmInfo += "Outlier value      : " + QString::number(alarm.outlierValue) + "\n";
            lAlarmInfo += "High limit         : " + QString::number(alarm.highLimit) + "\n";
            lAlarmInfo += "Exec count         : " + QString::number(alarm.execCount) + "\n";
            lAlarmInfo += "Fail count         : " + QString::number(alarm.failCount) + "\n";
            lAlarmsInfoList += lAlarmInfo;
        }

        GexMoSendEmail Email;
        bool bHtmlEmail = (GetAttribute(C_EMAILFORMAT).toString() == "html");
        QString strFrom = GetAttribute(C_EMAILFROM).toString();
        QString strTo = GetAttribute(C_EMAILTO).toString();

        QString alarmLevel = lMaxCriticity == 2 ? "critical" : "standard";

        QString strTitle = "** " + GetTypeName() + " ALARM (" + alarmLevel + ") **";
        aDbKeysContent.Set(GetTypeName() + "Info",QString("%1 ALARM").arg(alarmLevel.toUpper()));
        lMessageStatus = "ok: " + GetTypeName() + " task [" + m_strName +"] generates "+alarmLevel+" alarm(s)";

        QString strSubject = strTitle
                + " (product="
                + lProductName
                + " ,lot="
                + lLot
                + " ,sublot="
                + lSubLotId
                + " ,wafer="
                + lWaferId
                + ")";

        QString lEmailBody;

        // Alarms
        lTextBody += "\n#### Alarms: " + QString::number(lAlarmsInfoList.size()) + " ##################################################\n";
        lTextBody += lAlarmsInfoList.join("\n");

        lEmailBody = lTextBody;

        if(bHtmlEmail)
        {
            // HTML Header
            lEmailString.CreatePage(strTitle);

            // Table with task details
            lEmailString.AddHtmlString("<h2><font color=\"#000080\">" + GetTypeName() + " rule</font></h2>\n");
            lEmailString.WriteHtmlOpenTable();
            foreach(const QString &line, lTaskDetails.split("\n",QString::SkipEmptyParts))
            {
                lEmailString.WriteInfoLine(line.section(":",0,0).trimmed(), (line.section(":",1).trimmed().isEmpty()?"n/a":line.section(":",1).trimmed()), false, false, false);
            }
            lEmailString.WriteHtmlCloseTable();


            // Build list of wafers with alarms detected
            QMap<QString, StatMonAlarm > lWaferWithAlarm;
            foreach (const StatMonAlarm& alarm, alarms)
            {
                QString lKey = alarm.productName + "|" +
                        alarm.lotId + "|" +
                        alarm.sublotId + "|" +
                        alarm.waferId + "|" +
                        alarm.splitlotId;

                lWaferWithAlarm.insertMulti(lKey, alarm);
            }


            QString lTaskDataSet;

            QString lDBName = GetAttribute(C_DATABASE).toString();
            GexDatabaseEntry* lTDREntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().
                    FindDatabaseEntry(lDBName,false);
            if (!lTDREntry)
            {
                lMessageStatus = QString("Error: No database with this name %1")
                        .arg(lDBName).toLatin1().constData();
                GSLOG(SYSLOG_SEV_ERROR, lMessageStatus.toLatin1().data() );
                lMessageStatus = "delay: "+lMessageStatus;
                return lMessageStatus;
            }
            QSqlQuery lTDRQuery(QSqlDatabase::database(lTDREntry->m_pExternalDatabase->GetPluginID()->
                                                       m_pPlugin->m_pclDatabaseConnector->m_strConnectionName));

            // ///////////////////////////////////////////
            // Iterate on wafers with alarms
            foreach(const QString &lWaferKey, lWaferWithAlarm.uniqueKeys())
            {
                lEmailString.AddHtmlString("<br>\n");
                lEmailString.AddHtmlString("<br>\n");
                lEmailString.AddHtmlString("<hr>\n");
                // ////////////////////////////////////////////////////////////////
                // Build Dataset summary

                QList<StatMonAlarm> lAlarmsForThisWafer = lWaferWithAlarm.values(lWaferKey);
                StatMonAlarm lFirstAlarm = lAlarmsForThisWafer.first();

                QString lTestingDate, lTesterName, lOperator, lProgramName, lProgramRevision, lNbParts, lGrossDie, lQuery;

                // wafer sort: query wt_consolidated_wafer
                if (GetAttribute(C_TESTINGSTAGE).toString().simplified() == GEXDB_PLUGIN_GALAXY_WTEST)
                {
                    // on insertion, check splitlot table
                    if(lExecutionType == QChar('I'))
                    {
                        lQuery =  "SELECT "
                                  "wt_splitlot.lot_id, wt_splitlot.wafer_id, start_t, tester_name, oper_nam, "
                                  "job_nam, job_rev, gross_die, wt_splitlot.nb_parts "
                                  "FROM wt_splitlot JOIN wt_wafer_info "
                                  "ON (wt_splitlot.lot_id=wt_wafer_info.lot_id AND wt_splitlot.wafer_id=wt_wafer_info.wafer_id) "
                                  "WHERE splitlot_id='" + QString::number(lSplitlotId) + "'";
                    }
                    else
                    {
                        lQuery =  "SELECT lot_id, wafer_id, start_t, tester_name, oper_nam, job_nam, job_rev, gross_die, nb_parts "
                                  "FROM wt_consolidated_wafer "
                                  "WHERE lot_id='" + lFirstAlarm.lotId + "' AND wafer_id='" + lFirstAlarm.waferId + "'";
                    }
                }
                // final test: query ft_consolidated_sublot
                else if (GetAttribute(C_TESTINGSTAGE).toString().simplified() == GEXDB_PLUGIN_GALAXY_FTEST)
                {
                    // on insertion, check splitlot table
                    if(lExecutionType == QChar('I'))
                    {
                        lQuery =  "SELECT "
                                  "ft_splitlot.lot_id, ft_splitlot.sublot_id, start_t, tester_name, oper_nam, "
                                  "job_nam, job_rev, ft_splitlot.nb_parts "
                                  "FROM ft_splitlot JOIN ft_sublot_info "
                                  "ON (ft_splitlot.lot_id=ft_sublot_info.lot_id AND ft_splitlot.sublot_id=ft_sublot_info.sublot_id) "
                                  "WHERE splitlot_id='" + QString::number(lSplitlotId) + "'";
                    }
                    else
                    {
                        lQuery =  "SELECT lot_id, sublot_id, start_t, tester_name, oper_nam, job_nam, job_rev, nb_parts "
                                  "FROM ft_consolidated_sublot "
                                  "WHERE lot_id='" + lFirstAlarm.lotId + "' AND sublot_id='" + lFirstAlarm.sublotId + "'";
                    }
                }
                // elec test: query et_splitlot
                else if (GetAttribute(C_TESTINGSTAGE).toString().simplified() == GEXDB_PLUGIN_GALAXY_ETEST)
                {
                    lQuery =  "SELECT splitlot_id, start_t, tester_name, oper_nam, job_nam, job_rev, gross_die, et_splitlot.nb_parts "
                              "FROM et_splitlot INNER JOIN et_wafer_info "
                              "ON (et_splitlot.lot_id = et_wafer_info.lot_id AND et_splitlot.wafer_id = et_wafer_info.wafer_id) "
                              "WHERE splitlot_id=" + QString::number(lFirstAlarm.splitlotId);
                }

                if(!lTDRQuery.exec(lQuery))
                {
                    lMessageStatus = lTDRQuery.lastError().text();
                    GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                                 lQuery,
                                                                                 QString::number(lTDRQuery.lastError().number()),
                                                                                 lMessageStatus);
                    GSLOG(SYSLOG_SEV_ERROR, lMessageStatus.toLatin1().data() );
                    lMessageStatus = "delay: "+lMessageStatus;
                    return lMessageStatus;
                }

                if(lTDRQuery.first())
                {
                    lTestingDate = lTDRQuery.value("start_t").toString();
                    lTesterName = lTDRQuery.value("tester_name").toString();
                    lOperator = lTDRQuery.value("oper_nam").toString();
                    lProgramName = lTDRQuery.value("job_nam").toString();
                    lProgramRevision = lTDRQuery.value("job_rev").toString();
                    lNbParts = lTDRQuery.value("nb_parts").toString();
                    if (GetAttribute(C_TESTINGSTAGE).toString().simplified() != GEXDB_PLUGIN_GALAXY_FTEST)
                    {
                        lGrossDie = lTDRQuery.value("gross_die").toString();
                    }
                }

                lTaskDataSet = "";
                if(lExecutionType == QChar('I'))
                {
                    lTaskDataSet += "Testing date       : " + QString(TimeStringUTC_F(lTestingDate.toUInt(), "d MMMM yyyy h:mm:ss")) + "\n";
                }
                lTaskDataSet += "Product            : " + lFirstAlarm.productName + "\n";
                lTaskDataSet += "Lot                : " + lFirstAlarm.lotId + "\n";
                lTaskDataSet += "Sublot             : " + lFirstAlarm.sublotId + "\n";
                lTaskDataSet += "Wafer              : " + lFirstAlarm.waferId + "\n";
                lTaskDataSet += "Splitlot           : " + QString::number(lFirstAlarm.splitlotId) + "\n";

                lTaskDataSet += "Tester             : " + (lTesterName.isEmpty() ? QString("n/a") : lTesterName) + "\n";
                lTaskDataSet += "Operator           : " + (lOperator.isEmpty() ? QString("n/a") : lOperator) + "\n";
                lTaskDataSet += "Program name       : " + (lProgramName.isEmpty() ? QString("n/a") : lProgramName) + "\n";
                lTaskDataSet += "Program revision   : " + (lProgramRevision.isEmpty() ? QString("n/a") : lProgramRevision) + "\n";
                lTaskDataSet += "Total parts tested : " + (lNbParts.isEmpty() ? QString("n/a") : lNbParts) + "\n";
                lTaskDataSet += "GrossDie           : " + (lGrossDie.isEmpty() ? QString("n/a") : lGrossDie) + "\n";;

                // ADD Table with dataset details
                lEmailString.AddHtmlString("<h2><font color=\"#000080\">Dataset</font></h2>\n");
                lEmailString.WriteHtmlOpenTable();
                foreach(const QString &line, lTaskDataSet.split("\n",QString::SkipEmptyParts))
                {
                    lEmailString.WriteInfoLine(
                                line.section(":",0,0).trimmed(),
                                (line.section(":",1).trimmed().isEmpty() ? "n/a" : line.section(":",1).trimmed()), false, false, false);
                }
                lEmailString.WriteHtmlCloseTable();


                // ADD table with bins summary
                QString lTitle = "<h2><font color=\"#000080\">%1 bins summary</font></h2>\n";
                if (GetAttribute(C_MONITOREDITEMTYPE) == "H")
                    lTitle = QString("<h2><font color=\"#000080\">%1 bins summary</font></h2>\n").arg("Hardware");
                else if (GetAttribute(C_MONITOREDITEMTYPE) == "S")
                    lTitle = QString("<h2><font color=\"#000080\">%1 bins summary</font></h2>\n").arg("Software");

                lEmailString.AddHtmlString(lTitle);

                lEmailString.WriteHtmlOpenTable();
                // Header
                QStringList lTableHeader;
                lTableHeader <<  "Bin#" << "Bin Name" << "P/F" << "Parts" << "LL2" << "LL1" << "Percent" << "HL1" << "HL2";
                lEmailString.WriteLabelLineList(lTableHeader);

                // Build line by line Bins summary table

                // Iterate on monitored items for this Wafer
                QList<MonitoredItemDesc> lItemsListFromDataPoints = lMonitoredItemToSiteToStatToDataPoint.uniqueKeys();

                qSort(lItemsListFromDataPoints.begin(), lItemsListFromDataPoints.end(), lessThan);
                foreach(const MonitoredItemDesc& lItemDesc, lItemsListFromDataPoints)
                {
                    // Check if an alarm exists for this item and store its criticity
                    int lAlarmCriticity = -1;
                    foreach (const StatMonAlarm& lAlarm, lAlarmsForThisWafer)
                    {
                        if (lAlarm.monitoredItemNum == lItemDesc.num)
                        {
                            lAlarmCriticity = lAlarm.criticityLevel;
                            break;
                        }
                    }

                    QString lBinNumber = lItemDesc.num;
                    QString lBinName = lItemDesc.name;
                    QString lBinCat = lItemDesc.cat;
                    // Get the right datapoint
                    QString lParts = "0", lLL2 = "-", lLL1 = "-", lPercent = "0%", lHL1 = "-", lHL2 = "-";

                    QList<StatMonDataPoint> lDataPoints = lMonitoredItemToSiteToStatToDataPoint[lItemDesc][-1].values("ratio");
                    foreach (const StatMonDataPoint& lPoint, lDataPoints)
                    {
                        QString lKey = lPoint.mProduct + "|" +
                                lPoint.mLot + "|" +
                                lPoint.mSublot + "|" +
                                lPoint.mWafer + "|" +
                                lPoint.mSplitlot;
                        if (lKey == lWaferKey)
                        {
                            if (lBinCat == "F")
                            {
                                lParts = QString::number(lPoint.mNumFails);
                            }
                            else
                            {
                                lParts = QString::number(lPoint.mNumGood);
                            }
                            lPercent.sprintf(" %6.2f%% ",  lPoint.mValue);
                            break;
                        }
                    }

                    // We only want to write the bin if it is represented in the wafer (nb parts <> 0) or if it is in alarm
                    if(lParts != "0" || lAlarmCriticity != -1)
                    {
                        QString lTmpValue;
                        if (lItemsLimits.contains(lItemDesc))
                        {
                            lTmpValue.sprintf(" %6.2f%% ", lItemsLimits[lItemDesc].value(2).mLowl);
                            lLL2 = lItemsLimits[lItemDesc].value(2).mLowlIsEnabled ? lTmpValue : "-";

                            lTmpValue.sprintf(" %6.2f%% ", lItemsLimits[lItemDesc].value(1).mLowl);
                            lLL1 = lItemsLimits[lItemDesc].value(1).mLowlIsEnabled ? lTmpValue : "-";

                            lTmpValue.sprintf(" %6.2f%% ", lItemsLimits[lItemDesc].value(1).mHighl);
                            lHL1 = lItemsLimits[lItemDesc].value(1).mHighlIsEnabled ? lTmpValue : "-";

                            lTmpValue.sprintf(" %6.2f%% ", lItemsLimits[lItemDesc].value(2).mHighl);
                            lHL2 = lItemsLimits[lItemDesc].value(2).mHighlIsEnabled ? lTmpValue : "-";
                        }

                        QStringList lLine;
                        lLine << lBinNumber << lBinName << lBinCat << lParts << lLL2 << lLL1 << lPercent << lHL1 << lHL2;
                        QList<bool> lErrorList;

                        bool lIsAlarm = false;
                        QColor	lErrorBkgColor = QColor("#FF0000");
                        if(lAlarmCriticity == 2)
                        {
                            lIsAlarm = true;
                            lErrorBkgColor = QColor("#FF0000");
                        }
                        else if(lAlarmCriticity == 1)
                        {
                            lIsAlarm = true;
                            lErrorBkgColor = QColor("#FF8000");
                        }
                        lErrorList << false << false << false << false << false << false << lIsAlarm << false << false;
                        lEmailString.WriteInfoLineList(lLine, lErrorList, lErrorBkgColor);
                    }
                } // end foreach item monitored
                lEmailString.WriteHtmlCloseTable();

            }// end foreach wafer


            lEmailBody = lEmailString.ClosePage();
        }

        // Send email with Yield Monitoring alarm message + report.
        Email.Send(aMailFilePath,strFrom,strTo,strSubject,lEmailBody,bHtmlEmail,"");

    } // end alarms > 0
    else // If one or more alarms detected for this ProductID, launch the shell
    {
        // For SYA the SeverityLevel is 0:PASS, 1:STANDARD, 2:CRITICAL
        lMaxCriticity = 0;
    }

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().LaunchAlarmShell(GS::Gex::SchedulerEngine::ShellSya,
                     lMaxCriticity,
                     alarms.size(),
                     lProductName,
                     lLot,
                     lSubLotId,
                     lWaferId,
                     aDbKeysContent.Get("TesterName").toString(),
                     aDbKeysContent.Get("Operator").toString(),
                     aDbKeysContent.Get("FileName").toString(),
                     "?" );

    // Trace the ALARMS info
    // with the execution of the task into YmAdminDb
    TraceExecution("",(alarms.count() != 0 ?"WARNING":"INFO"),lTextBody);
    // Propagate the email into (TaskType)Summary for Trigger execution
    if(lExecutionType == QChar('T'))
        aDbKeysContent.Set(GetTypeName() + "Summary",lTextBody.replace("|",";"));

    return lMessageStatus;
}

// Check if the 2 SYA tasks should be considered duplicate
DuplicateStatus CGexMoTaskSYA::GetDuplicatedParam(CGexMoTaskStatisticalMonitoring & other)
{
    if(other.GetTaskType() != GEXMO_TASK_SYA) return NOTDUPLICATED;

    GexMoStatisticalMonitoringTaskData* lThisProp = GetProperties();
    GexMoStatisticalMonitoringTaskData* lOtherProp = other.GetProperties();
    if(!lThisProp || !lOtherProp)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: Unable to get Task properties").toUtf8().constData());
        return NOTDUPLICATED;
    }

    // it's not allowed to use same task name
    if (lThisProp->GetAttribute(C_TITLE).toString().simplified() ==
            lOtherProp->GetAttribute(C_TITLE).toString().simplified()) return TITLEDUPLICATED;

    if (lThisProp->GetAttribute(C_DATABASE).toString().simplified() !=
            lOtherProp->GetAttribute(C_DATABASE).toString().simplified()) return NOTDUPLICATED;

    if (lThisProp->GetAttribute(C_TESTINGSTAGE).toString().simplified() !=
            lOtherProp->GetAttribute(C_TESTINGSTAGE).toString().simplified()) return NOTDUPLICATED;

    if (lThisProp->GetAttribute(C_PRODUCTREGEXP).toString().simplified() !=
            lOtherProp->GetAttribute(C_PRODUCTREGEXP).toString().simplified()) return NOTDUPLICATED;

    // if both are active on insertion check filters
    if ((lThisProp->GetAttribute(C_ACTIVEONINSERT).toBool() == true) &&
             (lOtherProp->GetAttribute(C_ACTIVEONINSERT).toBool() == true))
    {
        if (GetProperties()->filtersMetaData == other.GetProperties()->filtersMetaData) return FILTERSDULPLICATEDONINSERTION;
    }

    return NOTDUPLICATED;
}


QString CGexMoTaskSYA::ConvertRuleType(QString oldRuleType)
{
    if(oldRuleType.toLower() == QStringLiteral("None").toLower())
        return C_OUTLIERRULE_MEAN_N_SIGMA;
    else if(oldRuleType.toLower() == QStringLiteral("Mean ± N*Sigma").toLower())
        return C_OUTLIERRULE_MEAN_N_SIGMA;
    else if(oldRuleType.toLower() == QStringLiteral("Median ± N*RobustSigma").toLower())
        return C_OUTLIERRULE_MEDIAN_N_ROBUSTSIGMA;
    else if(oldRuleType.toLower() == QStringLiteral("Median ± N*IQR").toLower())
        return C_OUTLIERRULE_MEDIAN_N_IQR;
    else if(oldRuleType.toLower() == QStringLiteral("Percentile(N)|Percentile(100-N)").toLower())
        return C_OUTLIERRULE_PERCENTILE_N;
    else if(oldRuleType.toLower() == QStringLiteral("Q1/Q3 ± N*IQR").toLower())
        return C_OUTLIERRULE_Q1Q3_N_IQR;
    else if(oldRuleType.toLower() == QStringLiteral("Default").toLower())
        return C_OUTLIERRULE_MEAN_N_SIGMA;
    else if(oldRuleType.toLower() == QStringLiteral("Manual").toLower())
        return C_OUTLIERRULE_MANUAL;
    return C_OUTLIERRULE_MEAN_N_SIGMA;
}

// Yield-Monitoring Rule: reset variables
CGexOldSYAParameters::CGexOldSYAParameters()
{
    clear();
}

void CGexOldSYAParameters::clear(void)
{
    mBiningType             = 0;                    // Binning type: 0=Good bins, 1=Failing bins
    mAlarmLevel             = 100;                  // Ranges in 0-100
    mAlarmIfOverLimit       = 0;                    // 0= alarm if Yield Under Limit, 1= alarm if Yield Over limit
    mMinimumyieldParts      = 50;                   // Minimum parts required to check the yield.
    mSblFile                = "";                   // Full path to SBL/YBL data file (for Statistical Bin Limits monitoring)
    mEmailReportType        = 0;                    // GEXMO_YIELD_EMAILREPORT_xxx  :ASCII text in BODY, CSV, Word,  PPT, PDF
    mNotificationType       = 0;                    // 0= Report attached to email, 1= keep on server, only email its URL
    mExceptionLevel         = 0;                    // 0=Standard, 1=Critical...


    mSYA_Rule               = eNone;                // Rule: None means SYL/SBL disabled, only Fixed limit defined
    mSYA_IgnoreDataPointsWithNullSigma = true;      // Set to true if datapoints with null sigma should be ignored
    mMapBins_rules.clear();                         //
    mExpirationWarningReprieve = 7;
    mExpirationAlgo         = 0;
    mRenewalDateRange       = 0;
    mRenewalDateRangeUnit   = 0;
    mExcludedBins           = "";
}
