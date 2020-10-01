#include "spm_task.h"
#include "statisticalMonitoring/statistical_monitoring_taskdata.h"
#include "statistical_monitoring_tables.h"
#include "statistical_monitoring_alarm_struct.h"
#include "statistical_monitoring_datapoint_struct.h"
#include "statistical_monitoring_limit_struct.h"
#include "statistical_monitoring_item_desc.h"

#include <QSqlError>

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

CGexMoTaskSPM::CGexMoTaskSPM(QString email, QObject *parent)
    : CGexMoTaskStatisticalMonitoring(parent)
{
    SetProperties(new GexMoStatisticalMonitoringTaskData);
    SetPrivateAttribute(C_EMAILTO, email);
    SetPrivateAttribute(C_STATSTOMONITOR, C_STATS_MEAN + "|" + C_STATS_SIGMA);
    SetPrivateAttribute(C_MONITOREDITEMTYPE, "P");
    mUniqueKeyRule = useNumAndName;
}

CGexMoTaskSPM::CGexMoTaskSPM(CGexMoTaskSPM *orig)
    : CGexMoTaskStatisticalMonitoring(orig)
{
    mUniqueKeyRule = useNumAndName;
}

CGexMoTaskItem* CGexMoTaskSPM::Duplicate(QString copyName)
{
    this->LoadTaskDetails();
    CGexMoTaskSPM* clone = new CGexMoTaskSPM(this);
    clone->UpdateAttribute(C_TITLE, copyName);
    clone->UpdateAttribute(C_TASKID, -1);
    clone->UpdateAttribute(C_VERSION_ID, -1);
    clone->UpdateAttribute(C_VERSION_DRAFT, true);
    clone->UpdateAttribute(C_VERSION_ACTIVE_PROD_ID, -1);
    clone->UpdateAttribute(C_VERSION_LATEST_PROD_ID, -1);
    return clone;
}

CGexMoTaskStatisticalMonitoring *CGexMoTaskSPM::Clone()
{
    return new CGexMoTaskSPM(this);
}

CGexMoTaskSPM::~CGexMoTaskSPM()
{
}

int CGexMoTaskSPM::GetTaskType()
{
    return GEXMO_TASK_SPM;
}

QString CGexMoTaskSPM::GetTypeName()
{
    return "SPM";
}

bool CGexMoTaskSPM::ParseTaskOptions(const QMap<QString, QString>& taskOptions)
{
    QString lField;
    int nField = 0;

    for(QMap<QString, QString>::const_iterator iter = taskOptions.constBegin(); iter != taskOptions.constEnd(); ++iter)
    {
        lField = iter.key();

        if( lField == C_TITLE || lField == C_DATABASE)
        {
            ++nField;
            UpdateAttribute(lField, iter.value());
        }
        else if(lField == C_TASKID)
        {
            ++nField;
            UpdateAttribute(lField, iter.value().toInt());
        }
        else if(lField == C_ACTIVEONTRIGGER || lField == C_ACTIVEONINSERT)
        {
            ++nField;
            UpdateAttribute(lField, iter.value().toLower() == "true" ? true : false);
        }
        else if(lField == C_VERSION_LASTEXEC)
        {
            ++nField;
            UpdateAttribute(lField, QDateTime::fromMSecsSinceEpoch(iter.value().toInt()));
        }
    }

    return (nField == 6);
}

bool CGexMoTaskSPM::ParseOldTaskOptions(const QMap<QString, QString>& taskOptions)
{
    QString lField;
    int nField = 0;

    for(QMap<QString, QString>::const_iterator iter = taskOptions.constBegin(); iter != taskOptions.constEnd(); ++iter)
    {
        lField = iter.key();

        if(
            lField == C_TITLE
            || lField == C_TESTINGSTAGE
            || lField == C_DATABASE
        )
        {
            ++nField;
            UpdateAttribute(lField, iter.value());
        }
        else if(lField == C_SPM_TDRSPMID)
        {
            ++nField;
            UpdateAttribute(C_TASKID, iter.value().toInt());
        }
        else if(
            lField == C_ACTIVEONTRIGGER
            || lField == C_ACTIVEONINSERT
        )
        {
            ++nField;
            UpdateAttribute(lField, iter.value().toLower() == "true" ? true : false);
        }
        else if(lField == C_VERSION_LASTEXEC)
        {
            ++nField;
            UpdateAttribute(lField, QDateTime::fromMSecsSinceEpoch(iter.value().toInt()));
        }
    }

    return (nField == 7);
}

const QMap<QString, QString>& CGexMoTaskSPM::GetTaskLightWeightOptions()
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetSPMLightWeightOptions(this->GetAttribute(C_TASKID).toInt());
}

QString CGexMoTaskSPM::BuildTaskOptionsQuery()
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

bool CGexMoTaskSPM::MigrateTaskDetailsFromTdrToYmAdminDb()
{
    GexDatabaseEntry* tdrEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(GetAttribute(C_DATABASE).toString(),false);
    if (tdrEntry == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error: database %1 not found").
              arg(GetAttribute(C_DATABASE).toString()).toUtf8().constData());
        return false;
    }

    if (tdrEntry == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error: database %1 not found").
              arg(GetAttribute(C_DATABASE).toString()).toUtf8().constData());
        return false;
    }
    QSqlQuery tdrQuery(QSqlDatabase::database(tdrEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_pclDatabaseConnector->m_strConnectionName));
    QSqlQuery ymQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    //////////////////////////////////////////////////
    // Get the next available task_id in ym_admin_db //
    //////////////////////////////////////////////////

    queryString = "SELECT IFNULL(MAX(task_id), 0)+1 as task_id FROM ym_spm" + SM_TABLE_MAIN;
    if(!ymQuery.exec(queryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     queryString,
                                                                     QString::number(ymQuery.lastError().number()),
                                                                     ymQuery.lastError().text());
        return false;
    }

    if(!ymQuery.first())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: query %1 returned no result").arg(queryString).toLatin1().constData());
        return false;
    }

    int currentSpmId = GetAttribute(C_TASKID).toInt();
    int newSpmId = ymQuery.value("task_id").toInt();
    UpdateAttribute(C_TASKID, newSpmId);

    //////////////////////////////////////////////////////////////
    // Duplicate the rows from tdr tables to ym_admin_db tables //
    //////////////////////////////////////////////////////////////

    // xx_spm

    queryString = "SELECT product_regexp,test_regexp,site_merge_mode,stats_to_monitor,min_lots,min_datapoints,default_algorithm,validity_period,days_before_expiration,send_email_before_expiration,auto_recompute,auto_recompute_method,auto_recompute_period,email_format,email_from,email_report_type,emails";
    queryString += " FROM " + GetTDRTablePrefix() + "_spm S";
    queryString += " INNER JOIN " + GetTDRTablePrefix() + "_spm_version SV ON S.spm_id=SV.spm_id";
    queryString += " WHERE spm_id=" + QString::number(currentSpmId);
    queryString += " ORDER BY spm_version_id DESC";
    if(!tdrQuery.exec(queryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     queryString,
                                                                     QString::number(tdrQuery.lastError().number()),
                                                                     tdrQuery.lastError().text());
        return false;
    }

    if(!tdrQuery.first())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: No tdr entry found for id %1").arg(QString::number(currentSpmId)).toLatin1().constData());
        return false;
    }

    queryString = "INSERT INTO ym_spm" + SM_TABLE_MAIN + "(task_id,testing_stage,product_regexp,monitored_item_type,monitored_item_regexp,test_flow,consolidation_type,consolidation_aggregation_level,consolidation_name_regexp,site_merge_mode,stats_to_monitor,min_lots,min_datapoints,default_algorithm,validity_period,days_before_expiration,send_email_before_expiration,auto_recompute,auto_recompute_method,auto_recompute_period,email_format,email_from,email_report_type,emails) VALUES (";
    queryString += QString::number(newSpmId) + ",";
    queryString += "'" + GetAttribute(C_TESTINGSTAGE).toString() + "',";
    queryString += "'" + tdrQuery.value("product_regexp").toString() + "',";
    queryString += "'P',";
    queryString += "'" + tdrQuery.value("test_regexp").toString() + "',";
    queryString += "'P',";
    queryString += "'" + QString(GEXDB_CONSO_TYPE_RAW) + "',";
    queryString += "'" + QString(GEXDB_CONSO_LEVEL_INSERT) + "',";
    queryString += "'*',";
    queryString += "'" + tdrQuery.value("site_merge_mode").toString() + "',";
    queryString += "'" + tdrQuery.value("stats_to_monitor").toString() + "',";
    queryString += tdrQuery.value("min_lots").toString() + ",";
    queryString += tdrQuery.value("min_datapoints").toString() + ",";
    queryString += "'" + tdrQuery.value("default_algorithm").toString() + "',";
    queryString += tdrQuery.value("validity_period").toString() + ",";
    queryString += tdrQuery.value("days_before_expiration").toString() + ",";
    queryString += tdrQuery.value("send_email_before_expiration").toString() + ",";
    queryString += tdrQuery.value("auto_recompute").toString() + ",";
    queryString += "'" + tdrQuery.value("auto_recompute_method").toString() + "',";
    queryString += tdrQuery.value("auto_recompute_period").toString() + ",";
    queryString += "'" + tdrQuery.value("email_format").toString() + "',";
    queryString += "'" + tdrQuery.value("email_from").toString() + "',";
    queryString += "'" + tdrQuery.value("email_report_type").toString() + "',";
    queryString += "'" + tdrQuery.value("emails").toString() + "')";
    if(!ymQuery.exec(queryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     queryString,
                                                                     QString::number(ymQuery.lastError().number()),
                                                                     ymQuery.lastError().text());
        return false;
    }

    // xx_spm_default_params

    queryString = "SELECT criticity_level,param_name,param_value";
    queryString += " FROM " + GetTDRTablePrefix() + "_spm_default_params";
    queryString += " WHERE spm_id=" + QString::number(currentSpmId);
    if(!tdrQuery.exec(queryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     queryString,
                                                                     QString::number(tdrQuery.lastError().number()),
                                                                     tdrQuery.lastError().text());
        return false;
    }

    QStringList values;
    while(tdrQuery.next())
    {
        values.append(
            "("
            + QString::number(newSpmId) + ","
            + tdrQuery.value("criticity_level").toString()+ ","
            + "'" + tdrQuery.value("param_name").toString() + "',"
            + tdrQuery.value("param_value").toString()
            + ")"
        );
    }
    if(values.length() > 0)
    {
        queryString = "INSERT INTO ym_spm" + SM_TABLE_DEF_PARAMS
                    + "(task_id,criticity_level,param_name,param_value) VALUES "
                    + values.join(',');
        if(!ymQuery.exec(queryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         queryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    // xx_spm_filters

    queryString = "SELECT field,value";
    queryString += " FROM " + GetTDRTablePrefix() + "_spm_filters";
    queryString += " WHERE spm_id=" + QString::number(currentSpmId);
    if(!tdrQuery.exec(queryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     queryString,
                                                                     QString::number(tdrQuery.lastError().number()),
                                                                     tdrQuery.lastError().text());
        return false;
    }

    values.clear();
    while(tdrQuery.next())
    {
        values.append(
            "("
            + QString::number(newSpmId) + ","
            + "'" + tdrQuery.value("field").toString() + "',"
            + "'" + tdrQuery.value("value").toString() + "'"
            + ")"
        );
    }
    if(values.length() > 0)
    {
        queryString = "INSERT INTO ym_spm" + SM_TABLE_FILTERS
                    + "(task_id,field,value) VALUES "
                    + values.join(',');
        if(!ymQuery.exec(queryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         queryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    // xx_spm_version

    queryString = "SELECT spm_version_id,draft_version,version_label,matched_products,creation_date,start_date,expiration_date,expiration_warning_date,expiration_warning_done,computation_fromdate,computation_todate";
    queryString += " FROM " + GetTDRTablePrefix() + "_spm_version";
    queryString += " WHERE spm_id=" + QString::number(currentSpmId);
    if(!tdrQuery.exec(queryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     queryString,
                                                                     QString::number(tdrQuery.lastError().number()),
                                                                     tdrQuery.lastError().text());
        return false;
    }

    values.clear();
    while(tdrQuery.next())
    {
        values.append(
            "("
            + QString::number(newSpmId) + ","
            + tdrQuery.value("spm_version_id").toString() + ","
            + tdrQuery.value("draft_version").toString() + ","
            + "'" + tdrQuery.value("version_label").toString() + "',"
            + "'" + tdrQuery.value("matched_products").toString() + "',"
            + "'" + tdrQuery.value("creation_date").toDateTime().toString("yyyy-MM-dd hh:mm:ss") + "',"
            + "'" + tdrQuery.value("start_date").toDateTime().toString("yyyy-MM-dd hh:mm:ss") + "',"
            + "'" + tdrQuery.value("expiration_date").toDateTime().toString("yyyy-MM-dd hh:mm:ss") + "',"
            + "'" + tdrQuery.value("expiration_warning_date").toDateTime().toString("yyyy-MM-dd hh:mm:ss") + "',"
            + tdrQuery.value("expiration_warning_done").toString() + ","
            + "'" + tdrQuery.value("computation_fromdate").toDateTime().toString("yyyy-MM-dd hh:mm:ss") + "',"
            + "'" + tdrQuery.value("computation_todate").toDateTime().toString("yyyy-MM-dd hh:mm:ss") + "'"
            + ")"
        );
    }
    if(values.length() > 0)
    {
        queryString = "INSERT INTO ym_spm" + SM_TABLE_VERSION
                    + "(task_id,version_id,draft_version,version_label,matched_products,creation_date,start_date,expiration_date,expiration_warning_date,expiration_warning_done,computation_fromdate,computation_todate) VALUES "
                    + values.join(',');
        if(!ymQuery.exec(queryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         queryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    // xx_spm_test

    queryString = "SELECT test_id,test_type,test_num,test_name,test_unit,test_scale";
    queryString += " FROM " + GetTDRTablePrefix() + "_spm_test";
    queryString += " WHERE spm_id=" + QString::number(currentSpmId);
    if(!tdrQuery.exec(queryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     queryString,
                                                                     QString::number(tdrQuery.lastError().number()),
                                                                     tdrQuery.lastError().text());
        return false;
    }

    values.clear();
    while(tdrQuery.next())
    {
        values.append(
            "("
            + QString::number(newSpmId) + ","
            + tdrQuery.value("test_id").toString() + ","
            + "'" + tdrQuery.value("test_type").toString() + "',"
            + "'" + tdrQuery.value("test_num").toString() + "',"
            + "'" + tdrQuery.value("test_name").toString() + "',"
            + "'" + tdrQuery.value("test_unit").toString() + "',"
            + tdrQuery.value("test_scale").toString()
            + ")"
        );
    }
    if(values.length() > 0)
    {
        queryString = "INSERT INTO ym_spm" + SM_TABLE_MONITORED_ITEM
                    + "(task_id,monitored_item_id,monitored_item_type,monitored_item_num,monitored_item_name,monitored_item_unit,monitored_item_scale) VALUES "
                    + values.join(',');
        if(!ymQuery.exec(queryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         queryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    // xx_spm_stat

    queryString = "SELECT spm_version_id,spm_stat_id,test_id,site_no,criticity_level,stat_name,has_unit,ll_enabled,ll,hl_enabled,hl,algorithm,computation_datapoints,computation_outliers,enabled,recompute";
    queryString += " FROM " + GetTDRTablePrefix() + "_spm_stat";
    queryString += " WHERE spm_id=" + QString::number(currentSpmId);
    if(!tdrQuery.exec(queryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     queryString,
                                                                     QString::number(tdrQuery.lastError().number()),
                                                                     tdrQuery.lastError().text());
        return false;
    }

    values.clear();
    QStringList values2;
    while(tdrQuery.next())
    {
        values.append(
            "("
            + QString::number(newSpmId) + ","
            + tdrQuery.value("spm_version_id").toString() + ","
            + tdrQuery.value("spm_stat_id").toString() + ","
            + tdrQuery.value("site_no").toString() + ","
            + tdrQuery.value("criticity_level").toString() + ","
            + "'" + tdrQuery.value("stat_name").toString() + "',"
            + tdrQuery.value("has_unit").toString() + ","
            + tdrQuery.value("ll_enabled").toString() + ","
            + tdrQuery.value("ll").toString() + ","
            + tdrQuery.value("hl_enabled").toString() + ","
            + tdrQuery.value("hl").toString() + ","
            + "'" + tdrQuery.value("algorithm").toString() + "',"
            + tdrQuery.value("computation_datapoints").toString() + ","
            + tdrQuery.value("computation_outliers").toString() + ","
            + tdrQuery.value("enabled").toString() + ","
            + tdrQuery.value("recompute").toString()
            + ")"
        );
        values2.append(
            "("
            + QString::number(newSpmId) + ","
            + tdrQuery.value("spm_version_id").toString() + ","
            + tdrQuery.value("spm_stat_id").toString() + ","
            + tdrQuery.value("test_id").toString()
            + ")"
        );
        // To prevent request failures due to the request exceeding the server's MAX_ALLOWED_PACKET size,
        // we use this cheap trick to flush the values once every 1000 items:
        if(values.length() > 1000)
        {
            queryString = "INSERT INTO ym_spm" + SM_TABLE_LIMIT
                        + "(task_id,version_id,limit_id,site_no,criticity_level,stat_name,has_unit,ll_enabled,ll,hl_enabled,hl,algorithm,computation_datapoints,computation_outliers,enabled,recompute) VALUES "
                        + values.join(',');
            if(!ymQuery.exec(queryString))
            {
                GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                             queryString,
                                                                             QString::number(ymQuery.lastError().number()),
                                                                             ymQuery.lastError().text());
                return false;
            }
            queryString = "INSERT INTO ym_spm" + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM
                        + "(task_id,version_id,limit_id,monitored_item_id) VALUES "
                        + values2.join(',');
            if(!ymQuery.exec(queryString))
            {
                GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                             queryString,
                                                                             QString::number(ymQuery.lastError().number()),
                                                                             ymQuery.lastError().text());
                return false;
            }
            values.clear();
            values2.clear();
        }
    }
    if(values.length() > 0)
    {
        queryString = "INSERT INTO ym_spm" + SM_TABLE_LIMIT
                    + "(task_id,version_id,limit_id,site_no,criticity_level,stat_name,has_unit,ll_enabled,ll,hl_enabled,hl,algorithm,computation_datapoints,computation_outliers,enabled,recompute) VALUES "
                    + values.join(',');
        if(!ymQuery.exec(queryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         queryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
        queryString = "INSERT INTO ym_spm" + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM
                    + "(task_id,version_id,limit_id,monitored_item_id) VALUES "
                    + values2.join(',');
        if(!ymQuery.exec(queryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         queryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    // xx_spm_stat_param

    queryString = "SELECT spm_version_id,spm_stat_id,param_name,param_value";
    queryString += " FROM " + GetTDRTablePrefix() + "_spm_stat_param";
    queryString += " WHERE spm_id=" + QString::number(currentSpmId);
    if(!tdrQuery.exec(queryString))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     queryString,
                                                                     QString::number(tdrQuery.lastError().number()),
                                                                     tdrQuery.lastError().text());
        return false;
    }

    values.clear();
    while(tdrQuery.next())
    {
        values.append(
            "("
            + QString::number(newSpmId) + ","
            + tdrQuery.value("spm_version_id").toString() + ","
            + tdrQuery.value("spm_stat_id").toString() + ","
            + "'" + tdrQuery.value("param_name").toString() + "',"
            + tdrQuery.value("param_value").toString()
            + ")"
        );
    }
    if(values.length() > 0)
    {
        queryString = "INSERT INTO ym_spm" + SM_TABLE_LIMIT_PARAM
                    + "(task_id,version_id,limit_id,param_name,param_value) VALUES "
                    + values.join(',');
        if(!ymQuery.exec(queryString))
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                         queryString,
                                                                         QString::number(ymQuery.lastError().number()),
                                                                         ymQuery.lastError().text());
            return false;
        }
    }

    return true;
}

QString CGexMoTaskSPM::GetStatisticalMonitoringTablesPrefix()
{
    return "ym_spm";
}

bool CGexMoTaskSPM::CheckComputeSpecificRequirements()
{
    QString testFlow = GetAttribute(C_TESTFLOW).toString();
    if(testFlow.isEmpty())
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                    QString("no test flow specified").toLatin1().constData());
        GS::Gex::Message::warning(
            "Compute limits",
            "You have not chosen any \"Test flow\" to work with. Please set test flow name or regular expression (\"*\" for all the flows together");
        return false;
    }

    QString consoLevel = GetAttribute(C_CONSOLIDATION_LEVEL).toString();

    QString testInsertion = GetAttribute(C_INSERTION).toString();

    if(consoLevel == GEXDB_CONSO_LEVEL_INSERT && testInsertion.isEmpty())
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                    QString("Consolidation level is test_insertion but no insertion name is specified").toLatin1().constData());
        GS::Gex::Message::warning(
            "Compute limits",
            "You have chosen a \"Test insertion\" consolidation aggregation level but specified no insertion name. Please set an insertion name or \"*\" for all the insertions");
        return false;
    }

    return true;
}

bool CGexMoTaskSPM::FetchDataPointsForComputing(
        QString testingStage,
        QString productRegexp,
        QMap<QString, QString> filtersMetaData,
        QString monitoredItemType,
        QList<MonitoredItemRule> monitoredItemRules,
        QStringList /*excludedItems*/, // not used in SPM
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
        QStringList &productList,
        int &numLots,
        int &numDataPoints,
        QSet<int>& siteList,
        QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > > &monitoredItemToSiteToStatToValues)
{
    GexDatabaseEntry *pDatabaseEntry = NULL;
    QMap<QString, QStringList> filtersConditions;

    if(!filtersMetaData.isEmpty())
    {
        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(GetAttribute(C_DATABASE).toString(),false);
        // Do nothing if database not found
        if(!pDatabaseEntry)
        {
            // Cannot find database name
            return false;
        }
        if(!pDatabaseEntry->m_pExternalDatabase->SPM_GetConditionsFromFilters(testingStage,
                                                                              filtersMetaData,
                                                                              filtersConditions))
        {
            QString lMessage;
            pDatabaseEntry->m_pExternalDatabase->GetLastError(lMessage);
            GS::Gex::Message::warning(
                "Compute SPM limits / Resolving filters",
                lMessage);
            return false;
        }
    }

    pDatabaseEntry = GetADREntry(GetAttribute(C_DATABASE).toString());
    if(!pDatabaseEntry)
    {
        // Do nothing if database not found
        return false;
    }

    if(!pDatabaseEntry->m_pExternalDatabase->SPM_FetchDataPointsForComputing(testingStage,
                                                                             productRegexp,
                                                                             monitoredItemType,
                                                                             monitoredItemRules,
                                                                             uniqueKeyRule,
                                                                             testFlow,
                                                                             consolidationType,
                                                                             consolidationLevel,
                                                                             testInsertion,
                                                                             statsToMonitor,
                                                                             siteMergeMode,
                                                                             useGrossDie,
                                                                             filtersConditions,
                                                                             computeFrom,
                                                                             computeTo,
                                                                             productList,
                                                                             numLots,
                                                                             numDataPoints,
                                                                             siteList,
                                                                             monitoredItemToSiteToStatToValues))
    {
        QString lMessage;
        pDatabaseEntry->m_pExternalDatabase->GetLastError(lMessage);
        GS::Gex::Message::warning(
            "Compute SPM limits",
            lMessage);
        return false;
    }

    return true;
}

void CGexMoTaskSPM::ProcessSpecificCleanLimits(const QString &itemCat, double &ll, bool &llEnabled, double &hl, bool &hlEnabled)
{
    return;
}

bool CGexMoTaskSPM::FetchDataPointsForCheck(
        QChar ExecutionType,
        QString testingStage,
        QString productName,
        QString lot,
        QString sublot,
        QString wafer,
        int splitlot,
        QMap<QString, QString> filtersMetaData,
        QList<MonitoredItemDesc> monitoredItemList,
        QStringList /*excludedItems*/, // not used in SPM
        MonitoredItemUniqueKeyRule uniqueKeyRule,
        QString testFlow, QString consolidationType,
        QString consolidationLevel,
        QString testInsertion,
        QList<int> siteList,
        QStringList statList,
        bool useGrossDie,
        const QDateTime *dateFrom,
        const QDateTime *dateTo,
        QString &resolvedProductList,
        QString &resolvedLotList,
        QString &resolvedSublotList,
        QString &resolvedWaferList,
        int &resolvedNumParts,
        QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > > &monitoredItemToSiteToStatToDataPoint)
{
    GexDatabaseEntry* pDatabaseEntry = NULL;

    if(ExecutionType != 'I')
    {
        QMap<QString, QStringList> filtersConditions;

        if(!filtersMetaData.isEmpty())
        {
            pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(GetAttribute(C_DATABASE).toString(),false);
            // Do nothing if database not found
            if(!pDatabaseEntry)
            {
                // Cannot find database name
                return false;
            }
            if(!pDatabaseEntry->m_pExternalDatabase->SPM_GetConditionsFromFilters(testingStage,
                                                                                  filtersMetaData,
                                                                                  filtersConditions))
            {
                QString lMessage;
                pDatabaseEntry->m_pExternalDatabase->GetLastError(lMessage);
                GS::Gex::Message::warning(
                    "Compute SPM limits / Resolving filters",
                    lMessage);
                return false;
            }
        }

        pDatabaseEntry = GetADREntry(GetAttribute(C_DATABASE).toString());
        if(!pDatabaseEntry)
        {
            // Do nothing if database not found
            return false;
        }

        if(!pDatabaseEntry->m_pExternalDatabase->SPM_FetchDataPointsForCheckOnTrigger(testingStage,
                                                                                     productName,
                                                                                     lot,
                                                                                     sublot,
                                                                                     wafer,
                                                                                     monitoredItemList,
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
                                                                                     filtersConditions,
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
        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(GetAttribute(C_DATABASE).toString(),false);

        // Do nothing if database not found
        if(!pDatabaseEntry)
        {
            return false;
        }
        if(!pDatabaseEntry->m_pExternalDatabase->SPM_FetchDataPointsForCheckOnInsertion(testingStage,
                                                                                       splitlot,
                                                                                       filtersMetaData,
                                                                                       monitoredItemList,
                                                                                       uniqueKeyRule,
                                                                                       siteList,
                                                                                       statList,
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

CGexMoTaskItem* CGexMoTaskSPM::CreateTask(SchedulerGui *gui, bool readOnly)
{
    this->LoadTaskDetails();
    return gui->CreateTaskSPM(this, readOnly);
}

void CGexMoTaskSPM::UpdateListView(SchedulerGui *gui, int nCurrentRow, bool allowEdit)
{
    if(gui)
    {
        gui->UpdateListViewSPM(this, nCurrentRow, allowEdit);
    }
}

QString CGexMoTaskSPM::ExecuteTask(GS::QtLib::DatakeysContent &aDbKeysContent, const QString& aMailFilePath)
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
    QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > > lMonitoredItemToSiteToStatToDataPoint;
    QHash<MonitoredItemDesc, QMap<int, StatMonLimit> > lItemsLimits;
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
        // Generate an email now

        QString lAlarm;
        QStringList lAlarms;
        foreach(const StatMonAlarm& alarm, alarms)
        {
            lMaxCriticity = qMax(alarm.criticityLevel, lMaxCriticity);

            lAlarm  = "Product            : " + alarm.productName + "\n";
            lAlarm += "Lot                : " + alarm.lotId + "\n";
            lAlarm += "Sublot             : " + alarm.sublotId + "\n";
            lAlarm += "Wafer              : " + alarm.waferId + "\n";

            if(lExecutionType == QChar('I'))
            {
                lAlarm += "Splitlot           : " + QString::number(alarm.splitlotId) + "\n";
            }
            lAlarm += GetMonitoredItemDesignation() + " type          : " + alarm.monitoredItemType + "\n";
            lAlarm += GetMonitoredItemDesignation() + " number        : " + alarm.monitoredItemNum + "\n";
            lAlarm += GetMonitoredItemDesignation() + " name          : " + alarm.monitoredItemName + "\n";
            lAlarm += "Unit               : " + alarm.monitoredItemUnit + "\n";
            lAlarm += "Stat name          : " + alarm.statName + "\n";
            lAlarm += "Site number        : " + (alarm.siteNum == -1 ? "All" : QString::number(alarm.siteNum)) + "\n";
            lAlarm += "Alarm criticity    : " + QString(alarm.criticityLevel == 2 ? "critical" : "standard") + "\n";
            lAlarm += "Low limit          : " + QString::number(alarm.lowLimit) + "\n";
            lAlarm += "Outlier value      : " + QString::number(alarm.outlierValue) + "\n";
            lAlarm += "High limit         : " + QString::number(alarm.highLimit) + "\n";
            lAlarm += "Exec count         : " + QString::number(alarm.execCount) + "\n";
            lAlarm += "Fail count         : " + QString::number(alarm.failCount) + "\n";
            lAlarms += lAlarm;
        }

        GexMoSendEmail Email;
        GexMoBuildEmailString lEmailString;
        bool bHtmlEmail = (GetAttribute(C_EMAILFORMAT).toString() == "html");
        QString strFrom = GetAttribute(C_EMAILFROM).toString();
        QString strTo = GetAttribute(C_EMAILTO).toString();

        QString alarmLevel = lMaxCriticity == 2 ? "critical" : "standard";

        QString strTitle = "** " + GetTypeName() + " ALARM (" + alarmLevel + ") **";
        aDbKeysContent.Set(GetTypeName() + "Info",QString("%1 ALARM").arg(alarmLevel.toUpper()));
        lMessageStatus = "ok: " + GetTypeName() + " task generates "+alarmLevel+" alarm(s)";

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
        lTextBody += "\n#### Alarms: " + QString::number(lAlarms.size()) + " ##################################################\n";
        lTextBody += lAlarms.join("\n");

        lEmailBody = lTextBody;

        if(bHtmlEmail)
        {
            // HTML Header
            lEmailString.CreatePage(strTitle);

            // Table with dataset details
            lEmailString.AddHtmlString("<h2><font color=\"#000080\">Dataset</font></h2>\n");
            lEmailString.WriteHtmlOpenTable();
            foreach(const QString &line, lTaskDataSet.split("\n",QString::SkipEmptyParts))
            {
                lEmailString.WriteInfoLine(line.section(":",0,0).trimmed(), (line.section(":",1).trimmed().isEmpty()?"n/a":line.section(":",1).trimmed()),
                                           line.section(":",1).contains("critical",Qt::CaseInsensitive),
                                           line.section(":",1).contains("standard",Qt::CaseInsensitive));
            }
            lEmailString.WriteHtmlCloseTable();

            // Table with task details
            lEmailString.AddHtmlString("<h2><font color=\"#000080\">" + GetTypeName() + "</font></h2>\n");
            lEmailString.WriteHtmlOpenTable();
            foreach(const QString &line, lTaskDetails.split("\n",QString::SkipEmptyParts))
            {
                lEmailString.WriteInfoLine(line.section(":",0,0).trimmed(), (line.section(":",1).trimmed().isEmpty()?"n/a":line.section(":",1).trimmed()));
            }
            lEmailString.WriteHtmlCloseTable();

            // Alarms
            lEmailString.AddHtmlString("<h2><font color=\"#000080\">Alarms</font></h2>\n");
            lEmailString.AddHtmlString("<h3><font color=\"#000080\">Alarms count: " + QString::number(lAlarms.size()) + "</font></h3>\n");
            lEmailString.WriteHtmlOpenTable();
            QStringList strList;
            // Extract the alarms lable
            foreach(const QString &alarmInfo, lAlarms.first().split("\n",QString::SkipEmptyParts))
            {
                strList << alarmInfo.section(":",0,0).trimmed();
            }
            lEmailString.WriteLabelLineList(strList);

            foreach(const QString &alarm, lAlarms)
            {
                strList.clear();
                foreach(const QString &alarmInfo, alarm.split("\n",QString::SkipEmptyParts))
                {
                    strList << alarmInfo.section(":",1).trimmed();
                }
                lEmailString.WriteInfoLineList(strList);
            }
            lEmailString.WriteHtmlCloseTable();

            lEmailBody = lEmailString.ClosePage();
        }

        // Send email with Yield Monitoring alarm message + report.
        Email.Send(aMailFilePath,strFrom,strTo,strSubject,lEmailBody,bHtmlEmail,"");

    }

    // If one or more alarms detected for this ProductID, launch the shell
    if(alarms.size() == 0)
    {
        // For SYA and SPM the SeverityLevel is 0:PASS, 1:STANDARD, 2:CRITICAL
        lMaxCriticity = 0;
    }

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().LaunchAlarmShell(GS::Gex::SchedulerEngine::ShellSpm,
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

// Check if the 2 SPM tasks should be considered duplicate
DuplicateStatus CGexMoTaskSPM::GetDuplicatedParam(CGexMoTaskStatisticalMonitoring & other)
{
    if(other.GetTaskType() != GEXMO_TASK_SPM)
    {
        return NOTDUPLICATED;
    }

    GexMoStatisticalMonitoringTaskData* lThisProp = GetProperties();
    GexMoStatisticalMonitoringTaskData* lOtherProp = other.GetProperties();
    if(!lThisProp || !lOtherProp)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: Unable to get Task properties").toUtf8().constData());
        return NOTDUPLICATED;
    }

    // If same title, DB, Testing Stage, and products => duplicate
    if(lThisProp->GetAttribute(C_TITLE).toString().simplified() ==
            lOtherProp->GetAttribute(C_TITLE).toString().simplified())
        return TITLEDUPLICATED;

    if (lThisProp->GetAttribute(C_DATABASE).toString().simplified() !=
            lOtherProp->GetAttribute(C_DATABASE).toString().simplified()) return NOTDUPLICATED;

    if (lThisProp->GetAttribute(C_TESTINGSTAGE).toString().simplified() !=
            lOtherProp->GetAttribute(C_TESTINGSTAGE).toString().simplified()) return NOTDUPLICATED;

    if (lThisProp->GetAttribute(C_PRODUCTREGEXP).toString().simplified() !=
            lOtherProp->GetAttribute(C_PRODUCTREGEXP).toString().simplified()) return NOTDUPLICATED;

    // If same DB, Testing Stage, and products, and both on file insertion => duplicate
    if(lThisProp->GetAttribute(C_ACTIVEONINSERT).toBool() == true &&
            lOtherProp->GetAttribute(C_ACTIVEONINSERT).toBool() == true)
    {
        return MAININFODUPLICATEDONINSERTION;
    }

    return NOTDUPLICATED;
}

GexDatabaseEntry* CGexMoTaskSPM::GetADREntry(const QString& lDBName)
{
    // GCORE-14130: non connected TDR are not blocking
    GexDatabaseEntry* lTDRDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(lDBName, false);
    if (!lTDRDatabaseEntry && !lTDRDatabaseEntry->m_pExternalDatabase)
    {
        return NULL;
    }
    GexDatabaseEntry* lADRDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(lTDRDatabaseEntry->m_pExternalDatabase->GetAdrLinkName(), false);
    if(!lADRDatabaseEntry || !lADRDatabaseEntry->m_pExternalDatabase)
    {
        return NULL;
    }
    return lADRDatabaseEntry;
}
