#include <QSqlQuery>
#include <QSqlError>

#include "statistical_monitoring_task.h"
#include "statistical_monitoring_taskdata.h"
#include "statistical_monitoring_item_desc.h"
#include "sm_sql_table_model.h"

#include "statistical_monitoring_tables.h"
#include "gexdb_plugin_monitorstat.h"
#include "gexdb_plugin_monitordatapoint.h"
#include "statistical_monitoring_datapoint_struct.h"
#include "statistical_monitoring_alarm_struct.h"
#include "statistical_monitoring_limit_struct.h"

#include "engine.h"
#include "admin_engine.h"
#include "scheduler_engine.h"
#include "db_engine.h"
#include "gex_database_entry.h"
#include "db_external_database.h"

#include "product_info.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include "message.h"

GBEGIN_ERROR_MAP(CGexMoTaskStatisticalMonitoring)
    GMAP_ERROR(eErrNullPointer,"Null pointer provided")
    GMAP_ERROR(eErrInvalidTask,"The provided task is not in a valid state")
    GMAP_ERROR(eErrUnexpectedTaskState,"The provided task is in an unexpected validation state")
    GMAP_ERROR(eErrMailSendFailed,"Expiration warning mail failed to be sent")
    GMAP_ERROR(eErrRenewalCheckFailed,"An unexpected error occured during renewal check")
    GMAP_ERROR(eErrCloneLimitsForRenewalFailed,"An error occured while creating new limits for renewal: %s")
    GMAP_ERROR(eErrComputeLimitRulesError,"Limits computing failed according to business rules: %s")
    GMAP_ERROR(eErrTaskExpired,"The task expired")
    GMAP_ERROR(eErrPluginError,"The plugin returned an error: %s")
    GMAP_ERROR(eErrDataBaseError,"Error executing SQL query.\nQUERY=%s\nERROR=%s")
    GMAP_ERROR(eErrNoDataPointsError,"Check against limits error: %s")
GEND_ERROR_MAP(CGexMoTaskStatisticalMonitoring)

CGexMoTaskStatisticalMonitoring::CGexMoTaskStatisticalMonitoring(QObject* parent)
    : CGexMoTaskItem(parent)
{
    mProperties = NULL;
    mLoadedFromDb = false;
    mLightWeightOnly = false;
}

CGexMoTaskStatisticalMonitoring::CGexMoTaskStatisticalMonitoring(CGexMoTaskStatisticalMonitoring *orig)
    : CGexMoTaskItem(orig)
{
    if(orig->mProperties != NULL)
    {
        SetProperties(new GexMoStatisticalMonitoringTaskData(this));
        *mProperties = *(orig->mProperties);
    }
    mLoadedFromDb = orig->mLoadedFromDb;
    mLightWeightOnly = orig->mLightWeightOnly;
}

CGexMoTaskStatisticalMonitoring::~CGexMoTaskStatisticalMonitoring()
{
}

void CGexMoTaskStatisticalMonitoring::CheckTaskStatusInternal(bool checkDatabaseOption, bool /*checkFolderOption*/)
{
    m_strName = mProperties->GetAttribute(C_TITLE).toString();

    if (!GS::LPPlugin::ProductInfo::getInstance()->isSYLSBLAllowed())
    {
        // The user doesn't have an SPM/SYA license
        m_iStatus = MO_TASK_STATUS_ERROR;
        m_strLastInfoMsg = QString("Your license does not allow the usage of the SPM or SYA feature. Please contact ") + GEX_EMAIL_SALES;
        return;
    }

    this->CheckDatabase(checkDatabaseOption);
    if(this->m_iStatus != MO_TASK_STATUS_NOERROR)
    {
        return;
    }
    if(!mLoadedFromDb)
    {
        // The database connection is OK but the task have not been loaded from the TDR yet
        if(!LoadTaskDetails())
        {
            m_iStatus = MO_TASK_STATUS_ERROR;
            m_strLastInfoMsg = QString("The task details cannot be loaded from the database");
        }
    }

    if(mProperties && !mProperties->GetAttribute(C_ACTIVEONINSERT).toBool() && !mProperties->GetAttribute(C_ACTIVEONTRIGGER).toBool())
    {
        // This task is not active
        m_iStatus = MO_TASK_STATUS_ERROR;
        m_strLastInfoMsg =  "Your task is neither activated on insertion nor trigger. Please change at least one of these settings to make the task executable.)";
        return;
    }

    int activeId, latestId;
    QDateTime activeExpirationDate, activeWarningDate, latestStartDate;
    QDateTime currentDate = GS::Gex::Engine::GetInstance().GetServerDateTime();
    QString errorMsg;

    if(!GetActiveProdVersionExpirationDate(activeId, activeExpirationDate, activeWarningDate, errorMsg))
    {
        m_iStatus = MO_TASK_STATUS_ERROR;
        m_strLastInfoMsg =  errorMsg;
        return;
    }

    if(!GetLatestProdVersionExpirationDate(latestId, latestStartDate, errorMsg))
    {
        m_iStatus = MO_TASK_STATUS_ERROR;
        m_strLastInfoMsg =  errorMsg;
        return;
    }

    if(activeExpirationDate < currentDate)
    {
        m_iStatus = MO_TASK_STATUS_ERROR;
        if(latestId != activeId)
        {
            m_strLastInfoMsg =  "Your task is expired: New limits have been computed but are not active yet.";
        }
        else
        {
            m_strLastInfoMsg =  "Your task is expired: Please compute new limits.";
        }
        return;
    }

    if(activeExpirationDate.addDays(-1) < currentDate)
    {
        if(latestId != activeId)
        {
            if(latestStartDate > activeExpirationDate)
            {
                m_iStatus = MO_TASK_STATUS_WARNING;
                m_strLastInfoMsg =  "Your task expires today: New limits have been computed but won't take effect immediately after this task expiration.";
            }
            else
            {
                m_strLastInfoMsg =  "Your task expires today: New limits have been computed and will take effect thereafter.";
            }
        }
        else
        {
            m_iStatus = MO_TASK_STATUS_WARNING;
            m_strLastInfoMsg =  "Your task expires today: Please compute new limits.";
        }
        return;
    }

    if(activeWarningDate < currentDate)
    {
        if(latestId != activeId)
        {
            if(latestStartDate > activeExpirationDate)
            {
                m_iStatus = MO_TASK_STATUS_WARNING;
                m_strLastInfoMsg =  "Your task will expire on "
                                    + activeExpirationDate.date().toString("yyyy MM dd")
                                    + ": New limits have been computed but won't take effect immediately after this task expiration.";
            }
            else
            {
                m_strLastInfoMsg =  "Your task will expire on "
                                    + activeExpirationDate.date().toString("yyyy MM dd")
                                    + ": New limits have been computed and will take effect thereafter.";
            }
        }
        else
        {
            m_iStatus = MO_TASK_STATUS_WARNING;
            m_strLastInfoMsg =  "Your task will expire on "
                                + activeExpirationDate.date().toString("yyyy MM dd")
                                + ": Please compute new limits.";
        }
    }
}

bool CGexMoTaskStatisticalMonitoring::GetActiveProdVersionExpirationDate(int& id, QDateTime& expirationDate, QDateTime& warningDate, QString& errorMsg)
{
    id = GetAttribute(C_VERSION_ACTIVE_PROD_ID).toInt();
    if(id == -1)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Warning: no active prod version found for task id %1").arg(this->GetAttribute(C_TASKID).toString()).toLatin1().constData());
        expirationDate = QDateTime::fromMSecsSinceEpoch(0);
        warningDate = QDateTime::fromMSecsSinceEpoch(0);
        errorMsg = "No limits have been validated yet. Please, compute & validate limits.";
        return false;
    }
    expirationDate = GetAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONDATE).toDateTime();
    warningDate = GetAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDATE).toDateTime();
    errorMsg = "";
    return true;
}

bool CGexMoTaskStatisticalMonitoring::GetLatestProdVersionExpirationDate(int& id, QDateTime& startDate, QString& errorMsg)
{
    id = GetAttribute(C_VERSION_LATEST_PROD_ID).toInt();
    if(id == -1)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Warning: no prod version found for task id %1").arg(this->GetAttribute(C_TASKID).toString()).toLatin1().constData());
        startDate = QDateTime::fromMSecsSinceEpoch(0);
        errorMsg = "No limits have been validated yet. Please, compute & validate limits.";
        return false;
    }
    startDate = GetAttribute(C_VERSION_LATEST_PROD_STARTDATE).toDateTime();
    errorMsg = "";
    return true;
}

int CGexMoTaskStatisticalMonitoring::GetActiveProdVersionId()
{
    return GetAttribute(C_VERSION_ACTIVE_PROD_ID).toInt();
}

bool CGexMoTaskStatisticalMonitoring::IsUsable(bool /*bCheckExecutionWindows*/, bool /*bCheckFrequency*/)
{
    // If the task is disabled by the user
    if(!m_bEnabledState)
    {
        return false;
    }

    int id;
    QDateTime expirationDate, warningDate;
    QString errorMsg;
    if(!GetActiveProdVersionExpirationDate(id, expirationDate, warningDate, errorMsg))
    {
        // no active prod version
        return false;
    }

    // return true if either the task is valid or the renewal warnings haven't been sent yet
    return (
        (GS::Gex::Engine::GetInstance().GetServerDateTime() <= expirationDate)
        ||
        (GetAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDONE).toInt() < 3)
    );
}

void CGexMoTaskStatisticalMonitoring::SetProperties(GexMoStatisticalMonitoringTaskData *properties)
{
    mTaskProperties = properties;
    mProperties = properties;
}

GexMoStatisticalMonitoringTaskData* CGexMoTaskStatisticalMonitoring::GetProperties()
{
    return mProperties;
}

QString CGexMoTaskStatisticalMonitoring::GetDatabaseName()
{
    QString DatabaseName = CGexMoTaskItem::GetDatabaseName();
    if(DatabaseName.isEmpty() && mProperties)
    {
        DatabaseName = mProperties->GetAttribute(C_DATABASE).toString();
    }
    return DatabaseName;
}

bool CGexMoTaskStatisticalMonitoring::SetDatabaseName(QString DatabaseName)
{
    CGexMoTaskItem::SetDatabaseName(DatabaseName);
    if(mProperties)
    {
        mProperties->UpdateAttribute(C_DATABASE, DatabaseName);
    }
    return true;
}

bool CGexMoTaskStatisticalMonitoring::LoadTaskDataFromDb()
{
    bool bAllocate = false;
    GexMoStatisticalMonitoringTaskData* props = mProperties;
    if(props == NULL)
    {
        bAllocate = true;
        props = new GexMoStatisticalMonitoringTaskData(this);
        SetProperties(props);
    }

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
    {
        if(bAllocate)
        {
            delete props;
            SetProperties(NULL);
        }
        return false;
    }

    QMap<QString, QString> taskOptions;
    if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().DumpTaskOptions(this->GetID(), taskOptions))
    {
        return false;
    }

    // Try loading the task in the new (ym_admin) format
    if(ParseTaskOptions(taskOptions))
    {
        if(LoadTaskLightWeightDetails())
        {
            return true;
        }
    }
    // Try loading the task in the old (ym_admin + TDR) format
    else if(ParseOldTaskOptions(taskOptions))
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Task %1 needs to be migrated to the 7.7+ format")
                                        .arg(QString::number(this->GetID())).toLatin1().constData());
        if(MigrateTaskDetailsFromTdrToYmAdminDb())
        {
            if(LoadTaskDetails())
            {
                // Migration has updated the task options,
                // save the task immediately to store the changes in the relevant tables
                if(SaveTaskDataToDb())
                {
                    return true;
                }
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Task %1 is unreadable after migration, rolling back")
                                                .arg(QString::number(this->GetID())).toLatin1().constData());

                // Cleanup the potentially orphaned rows
                DeleteTaskDetails();
            }
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Task %1 migration failed")
                                            .arg(QString::number(this->GetID())).toLatin1().constData());

            // Cleanup the potentially orphaned rows
            DeleteTaskDetails();
        }
    }

    // At that point, the load failed. Corrupted task
    if(bAllocate)
    {
        delete props;
        SetProperties(NULL);
    }
    return false;
}

bool CGexMoTaskStatisticalMonitoring::LoadTaskLightWeightDetails()
{
    // handle null values
    const QMap<QString, QString>& lightWeightOptions = GetTaskLightWeightOptions();

    this->UpdateAttribute(C_TESTINGSTAGE, lightWeightOptions["testing_stage"]);
    this->UpdateAttribute(C_PRODUCTREGEXP, lightWeightOptions["product_regexp"]);
    this->UpdateAttribute(C_STATSTOMONITOR, lightWeightOptions["stats_to_monitor"]);
    this->UpdateAttribute(C_MINLOTS, lightWeightOptions["min_lots"].toInt());
    this->UpdateAttribute(C_DEFAULTALGO, lightWeightOptions["default_algorithm"]);
    this->UpdateAttribute(C_EMAILTO, lightWeightOptions["emails"]);
    this->UpdateAttribute(C_N1, lightWeightOptions["n1"].toFloat());
    this->UpdateAttribute(C_N2, lightWeightOptions["n2"].toFloat());
    if(lightWeightOptions["active_version_id"] != "-1")
    {
        this->UpdateAttribute(C_VERSION_ACTIVE_PROD_ID, lightWeightOptions["active_version_id"].toInt());
        this->UpdateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONDATE, QDateTime::fromString(lightWeightOptions["active_expiration_date"], "yyyy-MM-ddThh:mm:ss"));
        this->UpdateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDATE, QDateTime::fromString(lightWeightOptions["active_expiration_warning_date"], "yyyy-MM-ddThh:mm:ss"));
        this->UpdateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDONE, lightWeightOptions["active_expiration_warning_done"].toInt());
    }
    if(lightWeightOptions["last_version_id"] != "-1")
    {
        this->UpdateAttribute(C_VERSION_LATEST_PROD_ID, lightWeightOptions["last_version_id"].toInt());
        this->UpdateAttribute(C_VERSION_LATEST_PROD_STARTDATE, QDateTime::fromString(lightWeightOptions["last_start_date"], "yyyy-MM-ddThh:mm:ss"));
    }

    mLoadedFromDb = true;
    mLightWeightOnly = true;

    return true;
}

QString CGexMoTaskStatisticalMonitoring::GetTDRTablePrefix()
{
    if(this->GetAttribute(C_TESTINGSTAGE).toString() == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        return "et";
    }
    else if(this->GetAttribute(C_TESTINGSTAGE).toString() == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        return "wt";
    }
    else if(this->GetAttribute(C_TESTINGSTAGE).toString() == GEXDB_PLUGIN_GALAXY_FTEST)
    {
        return "ft";
    }
    return "";
}

bool CGexMoTaskStatisticalMonitoring::LoadTaskDetails(int version)
{
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    //////////////////
    // ym_xxx table //
    //////////////////

    QString taskId = this->GetAttribute(C_TASKID).toString();
    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    queryString = "SELECT testing_stage,product_regexp,monitored_item_type,monitored_item_regexp,test_flow,consolidation_type,consolidation_aggregation_level,consolidation_name_regexp,site_merge_mode,stats_to_monitor,min_lots,min_datapoints,use_gross_die,threshold,default_algorithm,remove_outliers,validity_period,days_before_expiration,send_email_before_expiration,auto_recompute,auto_recompute_method,auto_recompute_period,email_format,email_from,emails,email_report_type";
    queryString += " FROM " + tablePrefix + SM_TABLE_MAIN;
    queryString += " WHERE task_id=" + taskId;

    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    if(!query.first())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: No entry found for id %1").arg(this->GetAttribute(C_TASKID).toString()).toLatin1().constData());
        return false;
    }

    this->UpdateAttribute(C_TESTINGSTAGE, query.value("testing_stage").toString());
    this->UpdateAttribute(C_PRODUCTREGEXP, query.value("product_regexp").toString());
    this->UpdateAttribute(C_MONITOREDITEMTYPE, query.value("monitored_item_type").toString());
    this->UpdateAttribute(C_MONITOREDITEMREGEXP, query.value("monitored_item_regexp").toString());
    this->UpdateAttribute(C_TESTFLOW, query.value("test_flow").toString());
    this->UpdateAttribute(C_CONSOLIDATION_TYPE, query.value("consolidation_type").toString());
    this->UpdateAttribute(C_CONSOLIDATION_LEVEL, query.value("consolidation_aggregation_level").toString());
    this->UpdateAttribute(C_INSERTION, query.value("consolidation_name_regexp").toString());
    if(!query.value("site_merge_mode").toString().isEmpty())
    {
        this->UpdateAttribute(C_SITEMERGEMODE, query.value("site_merge_mode").toString());
    }
    this->UpdateAttribute(C_STATSTOMONITOR, query.value("stats_to_monitor").toString());
    this->UpdateAttribute(C_MINLOTS, query.value("min_lots").toInt());
    this->UpdateAttribute(C_MINDATAPOINTS, query.value("min_datapoints").toInt());
    this->UpdateAttribute(C_GROSSDIE, QVariant(query.value("use_gross_die").toBool()));
    this->UpdateAttribute(C_THRESHOLD, QVariant(query.value("threshold").toDouble()));
    this->UpdateAttribute(C_DEFAULTALGO, query.value("default_algorithm").toString());
    this->UpdateAttribute(C_DATACLEANING, QVariant(query.value("remove_outliers").toBool()));
    this->UpdateAttribute(C_VALIDITYPERIOD, query.value("validity_period").toInt());
    this->UpdateAttribute(C_DAYSBEFOREEXPIRATION, query.value("days_before_expiration").toInt());
    this->UpdateAttribute(C_SENDEMAILBEFOREEXPIRATION, QVariant(query.value("send_email_before_expiration").toBool()));
    this->UpdateAttribute(C_AUTORECOMPUTE, QVariant(query.value("auto_recompute").toBool()));
    this->UpdateAttribute(C_AUTORECOMPUTEMETHOD, query.value("auto_recompute_method").toString());
    this->UpdateAttribute(C_AUTORECOMPUTEPERIOD, query.value("auto_recompute_period").toInt());
    this->UpdateAttribute(C_EMAILFORMAT, query.value("email_format").toString());
    this->UpdateAttribute(C_EMAILFROM, query.value("email_from").toString());
    this->UpdateAttribute(C_EMAILTO, query.value("emails").toString());
    this->UpdateAttribute(C_EXCEPTIONTYPE, query.value("email_report_type").toString());

    //////////////////////////
    // ym_xxx_filters table //
    //////////////////////////
    this->GetProperties()->filtersMetaData.clear();

    queryString = "SELECT field, value FROM " + tablePrefix + SM_TABLE_FILTERS;
    queryString += " WHERE task_id=" + this->GetAttribute(C_TASKID).toString();
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }
    while(query.next())
    {
        this->GetProperties()->filtersMetaData.insert(query.value("field").toString(), query.value("value").toString());
    }

    /////////////////////////////////
    // ym_xxx_default_params table //
    /////////////////////////////////

    for(int n = 1; n<= 2; ++n)
    {
        queryString = "SELECT param_value FROM " + tablePrefix + SM_TABLE_DEF_PARAMS;
        queryString += " WHERE task_id=" + taskId;
        queryString += " AND criticity_level=" + QString::number(n);
        queryString += " AND param_name='N'";
        if(!query.exec(queryString))
        {
            GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
            return false;
        }

        if(!query.first())
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error: incomplete db entry for id %1").arg(this->GetAttribute(C_TASKID).toString()).toLatin1().constData());
            return false;
        }

        this->UpdateAttribute(C_N + QString::number(n), query.value("param_value").toFloat());
    }

    //////////////////////////
    // ym_xxx_version table //
    //////////////////////////

    bool success = false;

    while(!success)
    {
        queryString = "SELECT version_id, draft_version,version_label,matched_products,creation_date,start_date,expiration_date,expiration_warning_date,expiration_warning_done,computation_fromdate,computation_todate";
        queryString += " FROM " + tablePrefix + SM_TABLE_VERSION;
        queryString += " WHERE task_id=" + taskId;
        if(version == 0)
        {
            queryString += " ORDER BY version_id DESC LIMIT 1";
        }
        else
        {
            queryString += " AND version_id=" + QString::number(version);
        }

        if(!query.exec(queryString))
        {
            GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
            return false;
        }

        if(!query.first())
        {
            if(version != 0)
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Warning: requested version %1 not found, loading latest").arg(QString::number(version)).toLatin1().constData());
                version = 0;
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Error: no version found for id %1").arg(this->GetAttribute(C_TASKID).toString()).toLatin1().constData());
                return false;
            }
        }
        else
        {
            success = true;
            this->UpdateAttribute(C_VERSION_ID, query.value("version_id").toInt());
            this->UpdateAttribute(C_VERSION_DRAFT, query.value("draft_version").toInt() == 1 ? true : false);
            this->UpdateAttribute(C_VERSION_LABEL, query.value("version_label").toString());
            this->UpdateAttribute(C_VERSION_PRODUCTS, query.value("matched_products").toString());
            this->UpdateAttribute(C_VERSION_CREATIONDATE, query.value("creation_date").toDateTime());
            this->UpdateAttribute(C_VERSION_STARTDATE, query.value("start_date").toDateTime());
            this->UpdateAttribute(C_VERSION_EXPIRATIONDATE, query.value("expiration_date").toDateTime());
            this->UpdateAttribute(C_VERSION_EXPIRATIONWARNINGDATE, query.value("expiration_warning_date").toDateTime());
            this->UpdateAttribute(C_VERSION_EXPIRATIONWARNINGDONE, query.value("expiration_warning_done").toInt());
            this->UpdateAttribute(C_VERSION_COMPUTEFROM, query.value("computation_fromdate").toDateTime());
            this->UpdateAttribute(C_VERSION_COMPUTETO, query.value("computation_todate").toDateTime());
        }
    }

    ///////////////////////////////////////
    // ym_xxx_version table: active prod //
    ///////////////////////////////////////

    queryString = "SELECT version_id, expiration_date, expiration_warning_date, expiration_warning_done";
    queryString += " FROM " + tablePrefix + SM_TABLE_VERSION;
    queryString += " WHERE task_id=" + GetAttribute(C_TASKID).toString();
    queryString += " AND draft_version=0";
    queryString += " AND start_date<=now()";
    queryString += " ORDER BY version_id DESC LIMIT 1";
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    if(!query.first())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Warning: no active prod version found for entry id %1").arg(this->GetAttribute(C_TASKID).toString()).toLatin1().constData());
    }
    else
    {
        this->UpdateAttribute(C_VERSION_ACTIVE_PROD_ID, query.value("version_id").toInt());
        this->UpdateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONDATE, query.value("expiration_date").toDateTime());
        this->UpdateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDATE, query.value("expiration_warning_date").toDateTime());
        this->UpdateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDONE, query.value("expiration_warning_done").toInt());
    }

    ///////////////////////////////////////
    // ym_xxx_version table: latest prod //
    ///////////////////////////////////////

    queryString = "SELECT version_id, start_date";
    queryString += " FROM " + tablePrefix + SM_TABLE_VERSION;
    queryString += " WHERE task_id=" + GetAttribute(C_TASKID).toString();
    queryString += " AND draft_version=0";
    queryString += " ORDER BY version_id DESC LIMIT 1";
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    if(!query.first())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Warning: no prod version found for entry id %1").arg(this->GetAttribute(C_TASKID).toString()).toLatin1().constData());
    }
    else
    {
        this->UpdateAttribute(C_VERSION_LATEST_PROD_ID, query.value("version_id").toInt());
        this->UpdateAttribute(C_VERSION_LATEST_PROD_STARTDATE, query.value("start_date").toDateTime());
    }

    mLoadedFromDb = true;
    mLightWeightOnly = false;

    return true;
}

bool CGexMoTaskStatisticalMonitoring::SaveTaskDataToDb()
{
    // The task details should only be saved if at some point they have been fully loaded
    // (for edition, execution...)
    // Otherwise nothing needs to be saved, and nothing SHOULD be saved based on a lightweight loaded task
    if(mLightWeightOnly)
    {
        return true;
    }

    if(!SaveTaskDetails())
    {
        return false;
    }

    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
    {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    // Delete all entries from this task
    queryString = "DELETE FROM ym_tasks_options WHERE task_id="+QString::number(this->GetID());
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "INSERT INTO ym_tasks_options (task_id,field,value) VALUES";

    // Update Private attribute from members
    this->UpdatePrivateAttributes();

    queryString += BuildTaskOptionsQuery();

    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::SaveTaskDetails()
{
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    // Check if the task has already been created
    queryString = "SELECT task_id FROM " + tablePrefix + SM_TABLE_MAIN;
    queryString += " WHERE task_id=" + this->GetAttribute(C_TASKID).toString();
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    if(!query.first())
    {
        // the task has yet to be created

        // Get the next AutoIncrement
        QTime clTime = QTime::currentTime();
        queryString = "INSERT INTO " + tablePrefix + SM_TABLE_MAIN + "(testing_stage,product_regexp,monitored_item_type,monitored_item_regexp,test_flow,consolidation_type,consolidation_aggregation_level,consolidation_name_regexp,site_merge_mode,stats_to_monitor,min_lots,min_datapoints,use_gross_die,threshold,default_algorithm,remove_outliers,validity_period,days_before_expiration,send_email_before_expiration,auto_recompute,auto_recompute_method,auto_recompute_period)";
        queryString += " VALUES('','" + clTime.toString("hhmmsszz") + "','','','','','','','','',0,0,false,0,'',false,0,0,false,false,'',0)";
        if(!query.exec(queryString))
        {
            GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
            return false;
        }

        // Retrieve the task_id
        queryString = "SELECT task_id FROM " + tablePrefix + SM_TABLE_MAIN;
        queryString += " WHERE product_regexp='" + clTime.toString("hhmmsszz")+"'";
        if(!query.exec(queryString))
        {
            GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
            return false;
        }

        query.first();
        this->UpdateAttribute(C_TASKID, query.value(0).toInt());
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Save DB tasks : new task id = %1").arg(this->GetAttribute(C_TASKID).toString()).toLatin1().data() );
    }

    // update
    queryString = "UPDATE " + tablePrefix + SM_TABLE_MAIN + " SET";
    queryString += " testing_stage='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_TESTINGSTAGE).toString(),false) +"'";
    queryString += ",product_regexp='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_PRODUCTREGEXP).toString(),false) +"'";
    queryString += ",monitored_item_type='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_MONITOREDITEMTYPE).toString(),false) +"'";
    queryString += ",monitored_item_regexp='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_MONITOREDITEMREGEXP).toString(),false) +"'";
    queryString += ",test_flow='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_TESTFLOW).toString(),false) +"'";
    queryString += ",consolidation_type='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_CONSOLIDATION_TYPE).toString(),false) +"'";
    queryString += ",consolidation_aggregation_level='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_CONSOLIDATION_LEVEL).toString(),false) +"'";
    queryString += ",consolidation_name_regexp='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_INSERTION).toString(),false) +"'";
    queryString += ",consolidation_name_regexp='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_INSERTION).toString(),false) +"'";
    queryString += ",site_merge_mode='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_SITEMERGEMODE).toString(),false) +"'";
    queryString += ",stats_to_monitor='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_STATSTOMONITOR).toString(),false) +"'";
    queryString += ",min_lots="
                + this->GetAttribute(C_MINLOTS).toString();
    queryString += ",min_datapoints="
                + this->GetAttribute(C_MINDATAPOINTS).toString();
    queryString += ",use_gross_die="
                + QString(this->GetAttribute(C_GROSSDIE).toBool() ? "1" : "0");
    queryString += ",threshold="
                + QString(this->GetAttribute(C_THRESHOLD).toString());
    queryString += ",default_algorithm='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_DEFAULTALGO).toString(),false) + "'";
    queryString += ",remove_outliers="
                + QString(this->GetAttribute(C_DATACLEANING).toBool() ? "1" : "0");
    queryString += ",validity_period="
                + this->GetAttribute(C_VALIDITYPERIOD).toString();
    queryString += ",days_before_expiration="
                + this->GetAttribute(C_DAYSBEFOREEXPIRATION).toString();
    queryString += ",send_email_before_expiration="
                + QString(this->GetAttribute(C_SENDEMAILBEFOREEXPIRATION).toBool() ? "1" : "0");
    queryString += ",auto_recompute="
                + QString(this->GetAttribute(C_AUTORECOMPUTE).toBool() ? "1" : "0");
    queryString += ",auto_recompute_method='"
                + this->GetAttribute(C_AUTORECOMPUTEMETHOD).toString() + "'";
    queryString += ",auto_recompute_period="
                + this->GetAttribute(C_AUTORECOMPUTEPERIOD).toString();
    queryString += ",email_format='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_EMAILFORMAT).toString(),false) +"'";
    queryString += ",email_from='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_EMAILFROM).toString(),false) +"'";
    queryString += ",emails='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_EMAILTO).toString(),false) +"'";
    queryString += ",email_report_type='"
                + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_EXCEPTIONTYPE).toString(),false) +"'";
    queryString += " WHERE task_id=" + this->GetAttribute(C_TASKID).toString();
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    // Update the filters
    // First remove the old filters before inserting the new ones
    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_FILTERS;
    queryString += " WHERE task_id=" + this->GetAttribute(C_TASKID).toString();
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }
    // Then insert the new name/value pairs
    for(QMap<QString, QString>::iterator it = GetProperties()->filtersMetaData.begin(); it != GetProperties()->filtersMetaData.end(); ++it)
    {
        if(!it.value().isEmpty())
        {
            queryString = "INSERT INTO " + tablePrefix + SM_TABLE_FILTERS + "(task_id,field,value) VALUES(";
            queryString += this->GetAttribute(C_TASKID).toString() + ",";
            queryString += GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(it.key())+",";
            queryString += GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(it.value()) + ")";
            if(!query.exec(queryString))
            {
                GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
                return false;
            }
        }
    }

    // Update the default params
    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_DEF_PARAMS;
    queryString += " WHERE task_id=" + this->GetAttribute(C_TASKID).toString();
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }
    queryString = "INSERT INTO " + tablePrefix + SM_TABLE_DEF_PARAMS + "(task_id,criticity_level,param_name,param_value) VALUES";
    QStringList paramQueries;
    for(int n = 1; n<= 2; ++n)
    {
        paramQueries.append("(" + this->GetAttribute(C_TASKID).toString() + ","
                            + QString::number(n) + ","
                            + "'N',"
                            + this->GetAttribute(C_N + QString::number(n)).toString() + ")");
    }
    queryString += paramQueries.join(',');
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }


    // Check if the version has already been created
    if(this->GetAttribute(C_VERSION_ID).toInt() == -1)
    {
        // The version has to be created, get the next available version number
        queryString = " SELECT IFNULL(MAX(version_id),0)+1 as version_id FROM " + tablePrefix + SM_TABLE_VERSION + " WHERE task_id=" + this->GetAttribute(C_TASKID).toString();
        if(!query.exec(queryString))
        {
            GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
            return false;
        }
        query.first();
        UpdateAttribute(C_VERSION_ID, query.value("version_id").toInt());
    }

    queryString = "SELECT task_id FROM " + tablePrefix + SM_TABLE_VERSION;
    queryString += " WHERE task_id=" + this->GetAttribute(C_TASKID).toString();
    queryString += " AND version_id=" + this->GetAttribute(C_VERSION_ID).toString();
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    if(!query.first())
    {
        // Create the version
        queryString = "INSERT INTO " + tablePrefix + SM_TABLE_VERSION + "(task_id,version_id,draft_version,version_label,matched_products,creation_date,start_date,expiration_date,expiration_warning_date,expiration_warning_done,computation_fromdate,computation_todate)";
        queryString += " VALUES(";
        queryString += this->GetAttribute(C_TASKID).toString() + ",";
        queryString += this->GetAttribute(C_VERSION_ID).toString() + ",";
        queryString += QString(this->GetAttribute(C_VERSION_DRAFT).toBool() ? "1" : "0") + ",";
        queryString += GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_VERSION_LABEL).toString()) + ",";
        queryString += GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_VERSION_PRODUCTS).toString()) + ",";
        queryString += GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_CREATIONDATE).toDateTime()) + ",";
        queryString += GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_STARTDATE).toDateTime()) + ",";
        queryString += GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_EXPIRATIONDATE).toDateTime()) + ",";
        queryString += GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_EXPIRATIONWARNINGDATE).toDateTime()) + ",";
        queryString += QString::number(this->GetAttribute(C_VERSION_EXPIRATIONWARNINGDONE).toInt()) + ",";
        queryString += GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_COMPUTEFROM).toDateTime()) + ",";
        queryString += GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_COMPUTETO).toDateTime()) + ")";
        if(!query.exec(queryString))
        {
            GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
            return false;
        }
    }
    else
    {
        queryString = "UPDATE " + tablePrefix + SM_TABLE_VERSION + " SET";
        queryString += " draft_version=" + QString(this->GetAttribute(C_VERSION_DRAFT).toBool() ? "1" : "0");
        queryString += ",version_label=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_VERSION_LABEL).toString()) + "";
        queryString += ",matched_products=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeStringToSql(this->GetAttribute(C_VERSION_PRODUCTS).toString()) + "";
        queryString += ",creation_date=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_CREATIONDATE).toDateTime()) + "";
        queryString += ",start_date=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_STARTDATE).toDateTime()) + "";
        queryString += ",expiration_date=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_EXPIRATIONDATE).toDateTime()) + "";
        queryString += ",expiration_warning_date=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_EXPIRATIONWARNINGDATE).toDateTime()) + "";
        queryString += ",expiration_warning_done=" + QString::number(this->GetAttribute(C_VERSION_EXPIRATIONWARNINGDONE).toInt());
        queryString += ",computation_fromdate=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_COMPUTEFROM).toDateTime()) + "";
        queryString += ",computation_todate=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(this->GetAttribute(C_VERSION_COMPUTETO).toDateTime()) + "";
        queryString += " WHERE task_id=" + this->GetAttribute(C_TASKID).toString();
        queryString += " AND version_id=" + this->GetAttribute(C_VERSION_ID).toString();
        if(!query.exec(queryString))
        {
            GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
            return false;
        }
    }

    return true;
}

void CGexMoTaskStatisticalMonitoring::SaveTaskToXML(QTextStream& XMLStream)
{
    return;
}

bool CGexMoTaskStatisticalMonitoring::DeleteTaskDetails()
{
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    QString taskId = this->GetAttribute(C_TASKID).toString();
    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();


    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_ALARM + " WHERE task_id=" + taskId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LOG + " WHERE task_id=" + taskId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT_PARAM + " WHERE task_id=" + taskId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM + " WHERE task_id=" + taskId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT + " WHERE task_id=" + taskId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_VERSION + " WHERE task_id=" + taskId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_MONITORED_ITEM + " WHERE task_id=" + taskId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_DEF_PARAMS + " WHERE task_id=" + taskId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_MAIN + " WHERE task_id=" + taskId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::DeleteVersionFromDB(int version)
{
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    QString taskId = this->GetAttribute(C_TASKID).toString();
    QString versionId = QString::number(version);
    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_ALARM + " WHERE task_id=" + taskId + " AND version_id=" + versionId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LOG + " WHERE task_id=" + taskId + " AND version_id=" + versionId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT_PARAM + " WHERE task_id=" + taskId + " AND version_id=" + versionId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM + " WHERE task_id=" + taskId + " AND version_id=" + versionId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT + " WHERE task_id=" + taskId + " AND version_id=" + versionId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_VERSION + " WHERE task_id=" + taskId + " AND version_id=" + versionId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::ValidateCurrentDraft()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("Validating draft for the statistical monitoring task: %1").arg(GetAttribute(C_TITLE).toString()).toLatin1().constData()));

    // Check if the version is a draft
    if(!GetAttribute(C_VERSION_DRAFT).toBool())
    {
        return false;
    }

    // Check if the draft is eligible as a production version
    if(!HasComputedLimits())
    {
        return false;
    }

    // Set the current draft as a production version
    UpdateAttribute(C_VERSION_DRAFT, QVariant(false));
    // update its validity range
    QDateTime current = GS::Gex::Engine::GetInstance().GetServerDateTime();
    QDateTime expirat = current.addDays(GetAttribute(C_VALIDITYPERIOD).toInt());
    QDateTime warning = expirat.addDays(-GetAttribute(C_DAYSBEFOREEXPIRATION).toInt());
    UpdateAttribute(C_VERSION_CREATIONDATE, current);
    UpdateAttribute(C_VERSION_STARTDATE, current);
    UpdateAttribute(C_VERSION_EXPIRATIONDATE, expirat);
    UpdateAttribute(C_VERSION_EXPIRATIONWARNINGDATE, warning);
    // and save it to the TDR
    if(!SaveTaskDetails())
    {
        return false;
    }

    // copy this validated prod data into the prod "cache"
    UpdateAttribute(C_VERSION_LATEST_PROD_ID, GetAttribute(C_VERSION_ID));
    UpdateAttribute(C_VERSION_LATEST_PROD_STARTDATE, current);
    UpdateAttribute(C_VERSION_ACTIVE_PROD_ID, GetAttribute(C_VERSION_ID));
    UpdateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONDATE, expirat);
    UpdateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDATE, warning);
    UpdateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDONE, 0);

    // then create and save a new draft
    UpdateAttribute(C_VERSION_ID, -1);
    UpdateAttribute(C_VERSION_DRAFT, QVariant(true));
    UpdateAttribute(C_VERSION_CREATIONDATE, GS::Gex::Engine::GetInstance().GetServerDateTime());
    // We use the scheduler saving method so that the ym_task's last_modification date
    // is updated and this task is reloaded by whichever other yieldman node has the former in its cache.
    if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveDbTasks(this))
    {
        return false;
    }
    return DbDuplicateLimits(GetAttribute(C_VERSION_ACTIVE_PROD_ID).toInt(), GetAttribute(C_VERSION_ID).toInt());
}

bool CGexMoTaskStatisticalMonitoring::DuplicateVersionToDraft(int version)
{
    if(!GetAttribute(C_VERSION_DRAFT).toBool())
    {
        return false;
    }
    return DbDuplicateLimits(version, GetAttribute(C_VERSION_ID).toInt());
}

bool CGexMoTaskStatisticalMonitoring::DbDuplicateLimits(int srcVersionId, int destVersionId)
{
    // duplicate the source version's limits and limit_params into the destination version
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    int taskId =  GetAttribute(C_TASKID).toInt();
    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    // Step 1: Clean the existing limits
    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT;
    queryString += " WHERE task_id=" + QString::number(taskId);
    queryString += " AND version_id=" + QString::number(destVersionId);
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    // Step 2: Copy the new ones
    queryString = "INSERT INTO " + tablePrefix + SM_TABLE_LIMIT;
    queryString += "(task_id,version_id,limit_id,site_no,criticity_level,stat_name,has_unit,ll_enabled,ll,hl_enabled,hl,algorithm,computation_datapoints,computation_outliers,enabled,recompute,"
                   "mean,sigma,median,q3,q1,percent_n,percent_100_min_n) ";
    queryString += "SELECT ";
    queryString += "task_id, ";
    queryString += QString::number(destVersionId) + ", ";
    queryString += "limit_id, ";
    queryString += "site_no, ";
    queryString += "criticity_level, ";
    queryString += "stat_name, ";
    queryString += "has_unit, ";
    queryString += "ll_enabled, ";
    queryString += "ll, ";
    queryString += "hl_enabled, ";
    queryString += "hl, ";
    queryString += "algorithm, ";
    queryString += "computation_datapoints, ";
    queryString += "computation_outliers, ";
    queryString += "enabled, ";
    queryString += "recompute, ";
    queryString += "mean, ";
    queryString += "sigma, ";
    queryString += "median, ";
    queryString += "q3, ";
    queryString += "q1, ";
    queryString += "percent_n, ";
    queryString += "percent_100_min_n ";
    queryString += "FROM " + tablePrefix + SM_TABLE_LIMIT + " ";
    queryString += "WHERE task_id=" + QString::number(taskId) + " ";
    queryString += "AND version_id=" + QString::number(srcVersionId);
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    // Step 3: Clean the existing limit-to-monitored-items associations
    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM;
    queryString += " WHERE task_id=" + QString::number(taskId);
    queryString += " AND version_id=" + QString::number(destVersionId);
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    // Step 4: Copy the new ones
    queryString = "INSERT INTO " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM;
    queryString += "(task_id,version_id,limit_id,monitored_item_id) ";
    queryString += "SELECT ";
    queryString += "task_id, ";
    queryString += QString::number(destVersionId) + ", ";
    queryString += "limit_id, ";
    queryString += "monitored_item_id ";
    queryString += "FROM " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM + " ";
    queryString += "WHERE task_id=" + QString::number(taskId) + " ";
    queryString += "AND version_id=" + QString::number(srcVersionId);
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    // Step 5: Clean the existing limit parameterss
    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT_PARAM;
    queryString += " WHERE task_id=" + QString::number(taskId);
    queryString += " AND version_id=" + QString::number(destVersionId);
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    // Step 6: Copy the new ones
    queryString = "INSERT INTO " + tablePrefix + SM_TABLE_LIMIT_PARAM;
    queryString += "(task_id,version_id,limit_id,param_name,param_value) ";
    queryString += "SELECT ";
    queryString += "task_id, ";
    queryString += QString::number(destVersionId) + ", ";
    queryString += "limit_id, ";
    queryString += "param_name, ";
    queryString += "param_value ";
    queryString += "FROM " + tablePrefix + SM_TABLE_LIMIT_PARAM + " ";
    queryString += "WHERE task_id=" + QString::number(taskId) + " ";
    queryString += "AND version_id=" + QString::number(srcVersionId);
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::HasComputedLimits(int version)
{
    if(version == 0)
    {
        version = this->GetAttribute(C_VERSION_ID).toInt();
    }

    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    queryString = "SELECT task_id FROM " + GetStatisticalMonitoringTablesPrefix() + SM_TABLE_LIMIT + " ";
    queryString += "WHERE task_id=" + this->GetAttribute(C_TASKID).toString() + " ";
    queryString += "AND version_id=" + QString::number(version);
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }
    return query.first();
}

bool CGexMoTaskStatisticalMonitoring::ComputeLimits(bool update/*=false*/, const QList<QPair<int, QString> >* const manualItemsNumNames /*=NULL*/, int version/*=-1*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("Computing limits for the statistical monitoring task: %1").arg(GetAttribute(C_TITLE).toString()).toLatin1().constData()));

    if (!GS::LPPlugin::ProductInfo::getInstance()->isSYLSBLAllowed())
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrLicenseError, NULL,
                   QString("No SPM/SYA license \nPlease contact %1").arg(GEX_EMAIL_SALES).toLatin1().constData());
        GSLOG(SYSLOG_SEV_ERROR, (QString("No SPM/SYA license \nPlease contact %1")
                                 .arg(GEX_EMAIL_SALES).toLatin1().constData()));
        GS::Gex::Message::critical(
            "No SPM/SYA license",
            QString("Your license does not include the SPM and SYA features, Please contact ") + GEX_EMAIL_SALES);
        return false;
    }

    this->LoadTaskDetails();

    int taskId = GetAttribute(C_TASKID).toInt();
    int versionId = version == -1 ? GetAttribute(C_VERSION_ID).toInt() : version;

    QString productRegexp = GetAttribute(C_PRODUCTREGEXP).toString();
    if(productRegexp.isEmpty())
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                    QString("No product selected for monitoring").toLatin1().constData());
        GS::Gex::Message::warning(
            "Compute limits",
            "Please select at least one product to monitor");
        return false;
    }

    QString itemType = GetAttribute(C_MONITOREDITEMTYPE).toString();

    QString itemRegexp = GetAttribute(C_MONITOREDITEMREGEXP).toString();
    if(itemRegexp.isEmpty() && (manualItemsNumNames == NULL || manualItemsNumNames->size() == 0))
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                    QString("No item selected for monitoring").toLatin1().constData());
        GS::Gex::Message::warning(
            "Compute limits",
            "Please select at least one item (test/binning) to monitor");
        return false;
    }

    QString testFlow = GetAttribute(C_TESTFLOW).toString();

    QString consoType = GetAttribute(C_CONSOLIDATION_TYPE).toString();

    QString consoLevel = GetAttribute(C_CONSOLIDATION_LEVEL).toString();

    QString testInsertion = GetAttribute(C_INSERTION).toString();

    QStringList statsToMonitor = GetAttribute(C_STATSTOMONITOR).toString().split('|');
    if(statsToMonitor.size() == 0)
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                    QString("No statistic selected for monitoring").toLatin1().constData());
        GS::Gex::Message::warning(
            "Compute limits",
            "Please select at least one stat to monitor");
        return false;
    }

    if(!CheckComputeSpecificRequirements())
    {
        return false;
    }

    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    // Get version specific properties
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    queryString = "SELECT computation_fromdate, computation_todate"
               " FROM " + tablePrefix + SM_TABLE_VERSION +
               " WHERE task_id = " + QString::number(taskId) + " AND version_id=" + QString::number(versionId);
    if(!query.exec(queryString))
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                    QString("Error retrieveing the provided version properties").toLatin1().constData());
        GS::Gex::Message::warning(
            "Compute limits",
            "Error getting the version properties");
        return false;
    }
    if(!query.first())
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                    QString("The version for which computation was requested does not exist").toLatin1().constData());
        GS::Gex::Message::warning(
            "Compute limits",
            "The version for which limits computation was requested does not exist");
        return false;
    }

    //////////////////////////////////////
    // Parse the monitored items regexp //
    //////////////////////////////////////

    QList<MonitoredItemRule> monitoredItemRules;
    QStringList excludedItems;
    MonitoredItemRule monitoredItemRule;
    if(!itemRegexp.isEmpty())
    {
        QListIterator<QString> iter(itemRegexp.split(QRegExp("[,;]")));
        while(iter.hasNext())
        {
            monitoredItemRule.ruleItems.clear();

            QString itemReg = iter.next().trimmed();
            if(itemReg == "*")
            {
                monitoredItemRule.ruleType = all;
                monitoredItemRules.append(monitoredItemRule);
            }
            else if(itemReg.contains('-') && !itemReg.startsWith('-'))
            {
                monitoredItemRule.ruleType = range;
                QStringList itemRange = itemReg.split('-');
                monitoredItemRule.ruleItems.append(itemRange.first());
                monitoredItemRule.ruleItems.append(itemRange.last());
                monitoredItemRules.append(monitoredItemRule);
            }
            else if(itemReg.contains('&'))
            {
                monitoredItemRule.ruleType = groupOfItems;
                QListIterator<QString> iter2(itemReg.split('&'));
                while(iter2.hasNext())
                {
                    monitoredItemRule.ruleItems.append(iter2.next());
                }
                monitoredItemRules.append(monitoredItemRule);
            }
            else if(itemReg.contains('+'))
            {
                monitoredItemRule.ruleType = mergeOfItems;
                QListIterator<QString> iter2(itemReg.split('+'));
                while(iter2.hasNext())
                {
                    monitoredItemRule.ruleItems.append(iter2.next());
                }
                monitoredItemRules.append(monitoredItemRule);
            }
            else if(itemReg.startsWith('!'))
            {
                excludedItems.append(itemReg.remove('!'));
            }
            else
            {
                monitoredItemRule.ruleType = singleItem;
                monitoredItemRule.ruleItems.append(itemReg);
                monitoredItemRules.append(monitoredItemRule);
            }
        }
    }

    //////////////////////////
    // Fetch the datapoints //
    //////////////////////////

    QStringList productList;
    int numLots, numDataPoints;
    QSet<int> siteList;
    QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > > monitoredItemToSiteToStatToValues;

    if(monitoredItemRules.size() != 0)
    {
        if(!FetchDataPointsForComputing(GetAttribute(C_TESTINGSTAGE).toString(),
                                        productRegexp,
                                        GetProperties()->filtersMetaData,
                                        itemType,
                                        monitoredItemRules,
                                        excludedItems,
                                        mUniqueKeyRule,
                                        testFlow,
                                        consoType,
                                        consoLevel,
                                        testInsertion,
                                        statsToMonitor,
                                        GetAttribute(C_SITEMERGEMODE).toString(),
                                        GetAttribute(C_GROSSDIE).toBool(),
                                        query.value("computation_fromdate").toDateTime(),
                                        query.value("computation_todate").toDateTime(),
                                        productList,
                                        numLots,
                                        numDataPoints,
                                        siteList,
                                        monitoredItemToSiteToStatToValues))
        {
            return false;
        }
    }

    if(manualItemsNumNames != NULL)
        {
    //////////////////////////////////////////////
        // Handle the manual creation dialog inputs //
    //////////////////////////////////////////////

        QStringList itemNums;
        if(siteList.isEmpty())
        {
            siteList.insert(-1);
        }

        monitoredItemRule.manualRule = true;
        monitoredItemRule.ruleType = singleItem;
        QListIterator<QPair<int, QString> > iterManualItems(*manualItemsNumNames);
        while(iterManualItems.hasNext())
        {

            QPair<int, QString> manualItemNumName = iterManualItems.next();
            itemNums.append(QString::number(manualItemNumName.first));

            MonitoredItemDesc item = CreateItemDesc(
                                         itemType,
                                         QString::number(manualItemNumName.first),
                                         manualItemNumName.second);
            QSetIterator<int> iterSite(siteList);
            while(iterSite.hasNext())
            {
                int siteNumber = iterSite.next();
                QListIterator<QString> iterStatsToMonitor(statsToMonitor);
                while(iterStatsToMonitor.hasNext())
                {
                    monitoredItemToSiteToStatToValues[item][siteNumber].insert(iterStatsToMonitor.next(), 0);
                }
            }

            monitoredItemRule.ruleItems.clear();
            monitoredItemRule.ruleItems.append(QString::number(manualItemNumName.first));
            monitoredItemRules.append(monitoredItemRule);
        }

        itemRegexp += (itemRegexp.isEmpty() ? "" : ",") + itemNums.join(',');
        UpdateAttribute(C_MONITOREDITEMREGEXP, itemRegexp);

        queryString = "UPDATE " + tablePrefix + SM_TABLE_MAIN +
                      " SET monitored_item_regexp='" + itemRegexp + "'"
                      " WHERE task_id = " + QString::number(taskId);
        if(!query.exec(queryString))
        {
            GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                        QString("Error writing the updated item regexp in the task").toLatin1().constData());
            GS::Gex::Message::warning(
                "Compute limits",
                "Error writing the updated item regexp in the task");
            return false;
        }
    }
    else
    {
    ////////////////////////////////////////////////////////
    // Check the datapoints against the task requirements //
    ////////////////////////////////////////////////////////

        // Check the products
        if(productList.isEmpty())
        {
            GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                        QString("No products were found matching your criteria within the provided date range").toLatin1().constData());
            GS::Gex::Message::warning(
                "Compute limits",
                "No products were found matching your criteria within the provided date range");
            return false;
        }

        // Check the minimum number of lots
        if(numLots < GetAttribute(C_MINLOTS).toInt())
        {
            GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                        QString("Not enough lots (" + QString::number(numLots) + ") were found matching your criteria within the provided date range").toLatin1().constData());
            GS::Gex::Message::warning(
                "Compute limits",
                "Not enough lots (" + QString::number(numLots) + ") were found matching your criteria within the provided date range");
            return false;
        }

        // Check the minimum number of datapoints
        if(numDataPoints < GetAttribute(C_MINDATAPOINTS).toInt())
        {
            GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                        QString("Not enough data points (" + QString::number(numDataPoints) + ") were found matching your criteria within the provided date range").toLatin1().constData());
            GS::Gex::Message::warning(
                "Compute limits",
                "Not enough data points (" + QString::number(numDataPoints) + ") were found matching your criteria within the provided date range");
            return false;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////
    // Update the ym_xxx_monitored_item table with newly discovered monitoredItems //
    /////////////////////////////////////////////////////////////////////////////////

    QHash<MonitoredItemDesc, int> registeredItemsIds;
    if(!CreateNewMonitoredItems(itemType, monitoredItemToSiteToStatToValues, registeredItemsIds))
    {
        return false;
    }

    /////////////////////////////////////////////////////////////////////////
    // Clear existing limits and create new ones if calculating new limits //
    /////////////////////////////////////////////////////////////////////////

    if(!update)
    {
        if(!CreateNewLimits(versionId, monitoredItemRules, registeredItemsIds, siteList, statsToMonitor, GetAttribute(C_DEFAULTALGO).toString()))
        {
            return false;
        }
    }

    ////////////////////////////////////////////////////////////////
    // Fetch limits that needs to be recomputed from the database //
    ////////////////////////////////////////////////////////////////

    queryString = "SELECT L.limit_id, MI.monitored_item_type, MI.monitored_item_cat, GROUP_CONCAT(CONCAT(MI.monitored_item_num, '|', MI.monitored_item_name) SEPARATOR ',') as items, L.site_no, L.stat_name, L.algorithm, LP.param_value"
               " FROM " + tablePrefix + SM_TABLE_LIMIT + " L"
               " INNER JOIN " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM + " LMI ON LMI.task_id=L.task_id AND LMI.version_id=L.version_id AND LMI.limit_id=L.limit_id"
               " INNER JOIN " + tablePrefix + SM_TABLE_MONITORED_ITEM + " MI ON MI.task_id=L.task_id AND MI.monitored_item_id=LMI.monitored_item_id"
               " INNER JOIN " + tablePrefix + SM_TABLE_LIMIT_PARAM + " LP ON LP.task_id=L.task_id AND LP.version_id=L.version_id AND LP.limit_id=L.limit_id AND LP.param_name='N'"
               " WHERE L.task_id = " + QString::number(taskId) + " AND L.version_id=" + QString::number(versionId) + " AND L.recompute=true"
               " GROUP BY L.limit_id";
    if(!query.exec(queryString))
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                    QString("Error getting the limits which need recomputing").toLatin1().constData());
        GS::Gex::Message::warning(
            "Compute limits",
            "Error getting the limits which need recomputing");
        return false;
    }

    //////////////////////////////////
    // Recompute the fetched limits //
    //////////////////////////////////

    QSqlQuery                      subQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    GexDbPlugin_MonitorStat        stat;
    double ll = 0;
    bool llEnabled = true;
    double hl = 0;
    bool hlEnabled = true;
    double mean;
    double sigma;
    double median;
    double Q1;
    double Q3;
    double percentN;
    double percent100MinusN;
    int numOutliers;
    while(query.next())
    {
        double nParam = query.value("param_value").toFloat();
        int limitId = query.value("limit_id").toInt();
        QString statName = query.value("stat_name").toString();
        QString itemType = query.value("monitored_item_type").toString();
        QString itemCat = query.value("monitored_item_cat").toString();
        QString limitAlgo = query.value("algorithm").toString();
        llEnabled = true;
        hlEnabled = true;

        if(nParam != -1)
        {
            QList<double> dataPoints;
            QListIterator<QString> iter(query.value("items").toString().split(','));
            while(iter.hasNext())
            {
                QStringList itemValues = iter.next().split('|');
                dataPoints.append(monitoredItemToSiteToStatToValues[CreateItemDesc(itemType.toUpper(), itemValues[0], itemValues[1])][query.value("site_no").toInt()].values(statName));
            }
            // Compute the limits
            if(limitAlgo != C_OUTLIERRULE_MANUAL
               &&
               stat.GetLimitsAndParams(dataPoints,
                                        GetAttribute(C_DATACLEANING).toBool(),
                                        GexDbPlugin_Base::GetOutlierAlgorithmType(limitAlgo),
                                        nParam,
                                        ll,
                                        hl,
                                        mean,
                                        sigma,
                                        median,
                                        Q1,
                                        Q3,
                                        percentN,
                                        percent100MinusN,
                                        numOutliers))
            {
                // Clean limits values regarding the monitored stat
                if(statName == C_STATS_YIELD
                        || statName == C_STATS_RATIO)
                {
                    ll = (ll < 0 ? 0 : (ll > 100 ? 100 : ll));
                    hl = (hl < 0 ? 0 : (hl > 100 ? 100 : hl));
                }
                else if (statName == C_STATS_EXEC_COUNT
                         || statName == C_STATS_FAIL_COUNT
                         || statName == C_STATS_RANGE
                         || statName == C_STATS_SIGMA
                         || statName == C_STATS_CP
                         || statName == C_STATS_CPK
                         || statName == C_STATS_CR)
                {
                    ll = (ll < 0 ? 0 : ll);
                    hl = (hl < 0 ? 0 : hl);
                }

                // Clean the limits values/enabled state regarding the specificities of the task
                ProcessSpecificCleanLimits(itemCat, ll, llEnabled, hl, hlEnabled);

                // Update the stat in the db
                queryString ="UPDATE " + tablePrefix + SM_TABLE_LIMIT +
                             " SET "
                             "ll_enabled=" + (llEnabled ? "true" : "false") + ", "
                             "ll=" + QString::number(ll, 'f', 10) + ", "
                             "hl_enabled=" + (hlEnabled ? "true" : "false") + ", "
                             "hl=" + QString::number(hl, 'f', 10) + ", "
                             "mean=" + QString::number(mean, 'f', 10) + ", "
                             "sigma=" + QString::number(sigma, 'f', 10) + ", "
                             "median=" + QString::number(median, 'f', 10) + ", "
                             "q1=" + QString::number(Q1, 'f', 10) + ", "
                             "q3=" + QString::number(Q3, 'f', 10) + ", "
                             "percent_n=" + QString::number(percentN, 'f', 10) + ", "
                             "percent_100_min_n=" + QString::number(percent100MinusN, 'f', 10) + ", "
                             "computation_datapoints=" + QString::number(dataPoints.count()) + ", "
                             "computation_outliers=" + QString::number(numOutliers) + ", "
                             "recompute=" + (update?"2":"0") + " "
                             "WHERE task_id=" + QString::number(taskId) +
                             " AND version_id=" + QString::number(versionId) +
                             " AND limit_id="+QString::number(limitId);
            }
            else
            {
                // Update the stats in the db with a zero, manual limit
                queryString = "UPDATE " + tablePrefix + SM_TABLE_LIMIT +
                              " SET "
                              "algorithm='" + C_OUTLIERRULE_MANUAL + "', "
                              "ll=0, "
                              "hl=0, "
                              "mean=0, "
                              "sigma=0, "
                              "median=0, "
                              "q1=0, "
                              "q3=0, "
                              "percent_n=0, "
                              "percent_100_min_n=0, "
                              "computation_datapoints=0, "
                              "computation_outliers=0, "
                              "recompute=0 "
                              "WHERE task_id=" + QString::number(taskId) +
                              " AND version_id=" + QString::number(versionId) +
                              " AND limit_id=" + QString::number(limitId);
            }
        }
        else
        {
            // disable the stat limits in the db
            queryString ="UPDATE " + tablePrefix + SM_TABLE_LIMIT +
                         " SET "
                         "enabled=false, "
                         "ll=NULL, "
                         "hl=NULL, "
                         "mean=NULL, "
                         "sigma=NULL, "
                         "median=NULL, "
                         "q1=NULL, "
                         "q3=NULL, "
                         "percent_n=NULL, "
                         "percent_100_min_n=NULL, "
                         "computation_datapoints=NULL, "
                         "computation_outliers=NULL, "
                         "recompute=0 "
                         "WHERE task_id=" + QString::number(taskId) +
                         " AND version_id=" + QString::number(versionId) +
                         " AND limit_id=" + QString::number(limitId);
        }

        if(!subQuery.exec(queryString))
        {
            GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                        QString("Error writing a limit").toLatin1().constData());
            GS::Gex::Message::warning(
                "Compute limits",
                "Error writing a limit");
            return false;
        }
    }

    //////////////////////////////////////////////////
    // Update the task properties after computation //
    //////////////////////////////////////////////////

    // If limits were computed for the current version,
    // update locally loaded attribute:
    if(version == -1)
    {
        UpdateAttribute(C_VERSION_PRODUCTS, productList.join('|'));
    }

    // In any case, update the database value
    queryString = "UPDATE " + tablePrefix + SM_TABLE_VERSION +
                  " SET matched_products='" + productList.join('|') + "'"
                  " WHERE task_id = " + QString::number(taskId) + " AND version_id=" + QString::number(versionId);
    if(!query.exec(queryString))
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrComputeLimitRulesError, NULL,
                    QString("Error writing the matched products in the version").toLatin1().constData());
        GS::Gex::Message::warning(
            "Compute limits",
            "Error writing the matched products in the version");
        return false;
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::CreateNewMonitoredItems(
        const QString& monitoredItemType,
        const QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& monitoredItemToSiteToStatToValues,
        QHash<MonitoredItemDesc, int>& registeredItemsIds)
{
    int lTaskId = GetAttribute(C_TASKID).toInt();
    QString lTablePrefix = GetStatisticalMonitoringTablesPrefix();

    GexDbPlugin_Connector* lCon = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector;
    if (!lCon)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Invalid connector!");
        return false;
    }

    QSqlQuery lQuery(QSqlDatabase::database(lCon->m_strConnectionName));

    QString lQueryString = "SELECT IFNULL(MAX(monitored_item_id),0) as item_id FROM " +
            lTablePrefix + SM_TABLE_MONITORED_ITEM + " WHERE task_id=" + QString::number(lTaskId);

    if(!lQuery.exec(lQueryString))
    {
        GS::Gex::Message::warning(
            "Compute limits",
            "Error getting the monitored item list from the database");
        return false;
    }
    lQuery.first();
    int lItemId = lQuery.value("item_id").toInt();

    // Get the existing items
    lQueryString = "SELECT monitored_item_id, monitored_item_type, monitored_item_num, "
                    "monitored_item_name FROM " + lTablePrefix + SM_TABLE_MONITORED_ITEM +
                    " WHERE task_id=" + QString::number(lTaskId) +
                    " AND monitored_item_type='" + monitoredItemType + "'";

    if(!lQuery.exec(lQueryString))
    {
        GS::Gex::Message::warning(
            "Compute limits",
            "Error getting the monitored item list from the database");
        return false;
    }
    while(lQuery.next())
    {
        int lMonItemId = lQuery.value("monitored_item_id").toInt();
        registeredItemsIds[CreateItemDesc(lQuery.value("monitored_item_type").toString().toUpper(),
                                          lQuery.value("monitored_item_num").toString(),
                                          lQuery.value("monitored_item_name").toString())] = lMonItemId;
    }

    // Iterate through all the current items

    lQueryString = "INSERT INTO "  + lTablePrefix + SM_TABLE_MONITORED_ITEM +
            " (task_id,monitored_item_id,monitored_item_type,monitored_item_num,monitored_item_name,"
            "monitored_item_unit,monitored_item_scale,monitored_item_cat) VALUES ";

    QStringList values;
    QListIterator<MonitoredItemDesc> iter(monitoredItemToSiteToStatToValues.uniqueKeys());
    while(iter.hasNext())
    {
        MonitoredItemDesc item = iter.next();
        if(!registeredItemsIds.contains(item))
        {
            // Get a monitored_item_id for that new item
            registeredItemsIds[item] = ++lItemId;

            // Create the missing ones in db
            values.append("("
                           + QString::number(lTaskId) + ","
                           + QString::number(lItemId) + ","
                           + lCon->TranslateStringToSqlVarChar(item.type.toUpper()) + ","
                           + lCon->TranslateStringToSqlVarChar(item.num) + ","
                           + lCon->TranslateStringToSqlVarChar(item.name) + ","
                           + lCon->TranslateStringToSqlVarChar(item.unit) + ","
                           + QString::number(item.scale) + ","
                           + lCon->TranslateStringToSqlVarChar(item.cat) + ")");

            if(values.length() > 1000)
            {
                if(!lQuery.exec(lQueryString + values.join(',')))
                {
                    GS::Gex::Message::warning("Compute limits",
                        "Error saving the monitored item list to the database");
                    return false;
                }
                values.clear();
            }
        }
    }
    if(!values.empty())
    {
        if(!lQuery.exec(lQueryString + values.join(',')))
        {
            GS::Gex::Message::warning("Compute limits",
                "Error saving the monitored item list to the database");
            return false;
        }
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::CreateNewLimits(const int version,
                                                      const QList<MonitoredItemRule>& monitoredItemRules,
                                                      const QHash<MonitoredItemDesc, int>& registeredItemsIds,
                                                      const QSet<int>& sites,
                                                      const QStringList& stats,
                                                      const QString& defaultAlgorithm)
{
    if(!CleanLimits())
    {
        GS::Gex::Message::warning(
            "Compute limits",
            "Error deleting previous limits");
        return false;
    }

    int taskId = GetAttribute(C_TASKID).toInt();
    int versionId = version == -1 ? GetAttribute(C_VERSION_ID).toInt() : version;
    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    // Get the criticity levels default params
    QList<int> criticityLevels;
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString = "SELECT DISTINCT criticity_level FROM " + tablePrefix + SM_TABLE_DEF_PARAMS + " WHERE task_id=" + QString::number(taskId);
    if(!query.exec(queryString))
    {
        GS::Gex::Message::warning(
            "Compute limits",
            "Error getting critical/standard default parameters from the database");
        return false;
    }
    while(query.next())
    {
        criticityLevels.append(query.value("criticity_level").toInt());
    }

    int newLimitId = 0;
    QStringList values1;
    QString queryString1 = "INSERT INTO " + tablePrefix + SM_TABLE_LIMIT + " (task_id,version_id,limit_id,site_no,criticity_level,stat_name,has_unit,ll_enabled,ll,hl_enabled,hl,algorithm,computation_datapoints,computation_outliers,enabled,recompute) VALUES ";
    QStringList values2;
    QString queryString2 = "INSERT INTO " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM + " (task_id,version_id,limit_id,monitored_item_id) VALUES ";

    QListIterator<MonitoredItemDesc> iterItem(registeredItemsIds.uniqueKeys());
    QListIterator<MonitoredItemRule> iterRule(monitoredItemRules);
    MonitoredItemRule currentRule;
    QSetIterator<int> iterSite(sites);
    int currentSite;
    QListIterator<QString> iterStat(stats);
    QString currentStat;

    bool matched;
    bool manual;

    ////////////
    // STEP 1 //
    ////////////

    // Iterate through the monitored items and create a limit set
    // for every matching rule which is not a group of items

    while(iterItem.hasNext())
    {
        const MonitoredItemDesc& currentItem = iterItem.next();

        matched = false;
        manual = false;
        iterRule.toFront();
        while(iterRule.hasNext())
        {
            currentRule = iterRule.next();
            if(currentRule.ruleType == groupOfItems)
            {
                continue;
            }

            if(
                (
                    (currentRule.ruleType == singleItem)
                    &&
                    !currentItem.num.contains('+')
                    &&
                    (currentRule.ruleItems.first() == currentItem.num)
                )
                ||
                (
                    (currentRule.ruleType == range)
                    &&
                    !currentItem.num.contains('+')
                    &&
                    (
                        currentRule.ruleItems.first().toInt() <= currentItem.num.toInt()
                        &&
                        currentRule.ruleItems.last().toInt() >= currentItem.num.toInt()
                    )
                )
                ||
                (
                    currentRule.ruleType == all
                    &&
                    !currentItem.num.contains('+')
                )
                ||
                (
                    (currentRule.ruleType == mergeOfItems)
                    &&
                    (currentRule.ruleItems.join('+') == currentItem.num)
                )
            )
            {
                matched = true;
                manual |= currentRule.manualRule;
                // Do not break out of the loop as another rule may
                // cover this item as well. In case several rules cover
                // the same item, the manual one prevails
            }
        }

        if(matched)
        {
            // Loop through the sites
            iterSite.toFront();
            while(iterSite.hasNext())
            {
                currentSite = iterSite.next();

                // Loop through the stats
                iterStat.toFront();
                while(iterStat.hasNext())
                {
                    currentStat = iterStat.next();

                    // Loop through the criticity levels
                    for(int i=0; i<criticityLevels.size(); ++i)
                    {
                        // create the limit entries
                        values1.append("("
                                      + QString::number(taskId) + ","
                                      + QString::number(versionId) + ","
                                      + QString::number(++newLimitId) + ","
                                      + QString::number(currentSite) + ","
                                      + QString::number(criticityLevels.at(i)) + ","
                                      + "'" + currentStat + "',"
                                      + HasUnit(currentStat) + ","
                                      "true,"
                                      "null,"
                                      "true,"
                                      "null,"
                                      + "'" + (manual ? C_OUTLIERRULE_MANUAL : defaultAlgorithm) + "',"
                                      "null,"
                                      "null,"
                                      "true,"
                                      "true)");

                        values2.append("("
                                       + QString::number(taskId) + ","
                                       + QString::number(versionId) + ","
                                       + QString::number(newLimitId) + ","
                                       + QString::number(registeredItemsIds[currentItem]) + ")");

                        if(values2.length() > 1000)
                        {
                            if(!query.exec(queryString1 + values1.join(',')))
                            {
                                GS::Gex::Message::warning(
                                    "Compute limits",
                                    "Error creating the default limit set");
                                return false;
                            }
                            if(!query.exec(queryString2 + values2.join(',')))
                            {
                                GS::Gex::Message::warning(
                                    "Compute limits",
                                    "Error creating the default limit set");
                                return false;
                            }
                            values1.clear();
                            values2.clear();
                        }
                    }
                }
            }
        }
    }
    if(!values2.empty())
    {
        if(!query.exec(queryString1 + values1.join(',')))
        {
            GS::Gex::Message::warning(
                "Compute limits",
                "Error creating the default limit set");
            return false;
        }
        if(!query.exec(queryString2 + values2.join(',')))
        {
            GS::Gex::Message::warning(
                "Compute limits",
                "Error creating the default limit set");
            return false;
        }
    }
    values1.clear();
    values2.clear();

    ////////////
    // STEP 2 //
    ////////////

    // Iterate through the groupOfItems rules and create a limit set
    // for each rules of which all items have been fetched

    iterRule.toFront();
    while(iterRule.hasNext())
    {
        currentRule = iterRule.next();
        if(currentRule.ruleType != groupOfItems)
        {
            continue;
        }

        // Loop through the sites
        iterSite.toFront();
        while(iterSite.hasNext())
        {
            currentSite = iterSite.next();

            // Loop through the stats
            iterStat.toFront();
            while(iterStat.hasNext())
            {
                currentStat = iterStat.next();

                // Loop through the criticity levels
                for(int i=0; i<criticityLevels.size(); ++i)
                {
                    // create the stat entries
                    values1.append("("
                                  + QString::number(taskId) + ","
                                  + QString::number(versionId) + ","
                                  + QString::number(++newLimitId) + ","
                                  + QString::number(currentSite) + ","
                                  + QString::number(criticityLevels.at(i)) + ","
                                  + "'" + currentStat + "',"
                                  + HasUnit(currentStat) + ","
                                  "true,"
                                  "null,"
                                  "true,"
                                  "null,"
                                  + "'" + (manual ? C_OUTLIERRULE_MANUAL : defaultAlgorithm) + "',"
                                  "null,"
                                  "null,"
                                  "true,"
                                  "true)");

                    iterItem.toFront();
                    while(iterItem.hasNext())
                    {
                        const MonitoredItemDesc& currentItem = iterItem.next();

                        if(
                            !currentItem.num.contains('+')
                            &&
                            currentRule.ruleItems.contains(currentItem.num)
                        )
                        {
                            values2.append("("
                                           + QString::number(taskId) + ","
                                           + QString::number(versionId) + ","
                                           + QString::number(newLimitId) + ","
                                           + QString::number(registeredItemsIds[currentItem]) + ")");
                        }
                    }
                }
            }
        }
        if(values2.length() == currentRule.ruleItems.length()
                             * sites.size()
                             * stats.size()
                             * criticityLevels.size())
        {
            if(!query.exec(queryString1 + values1.join(',')))
            {
                GS::Gex::Message::warning(
                    "Compute limits",
                    "Error creating the default limit set");
                return false;
            }
            if(!query.exec(queryString2 + values2.join(',')))
            {
                GS::Gex::Message::warning(
                    "Compute limits",
                    "Error creating the default limit set");
                return false;
            }
        }
        values1.clear();
        values2.clear();
    }

    ////////////
    // STEP 3 //
    ////////////

    // insert parameters for all the created limits

    queryString = "INSERT INTO " + tablePrefix + SM_TABLE_LIMIT_PARAM + "("
               + "SELECT SL.task_id, SL.version_id, SL.limit_id, SDP.param_name, SDP.param_value"
               + " FROM " + tablePrefix + SM_TABLE_LIMIT +" SL INNER JOIN " + tablePrefix + SM_TABLE_DEF_PARAMS + " SDP ON SL.task_id=SDP.task_id AND SL.criticity_level=SDP.criticity_level"
               + " WHERE SL.task_id=" + QString::number(taskId) + " AND SL.version_id=" + QString::number(versionId) + ")";
    if(!query.exec(queryString))
    {
        GS::Gex::Message::warning(
            "Compute limits",
            "Error creating the default limit set param values");
        return false;
    }

    return true;
}

QString CGexMoTaskStatisticalMonitoring::HasUnit(QString stat)
{
    if(stat == "fail_count")
    {
        return "false";
    }
    if(stat == "exec_count")
    {
        return "false";
    }
    if(stat == "range")
    {
        return "true";
    }
    if(stat == "yield")
    {
        return "false";
    }
    if(stat == "mean")
    {
        return "true";
    }
    if(stat == "sigma")
    {
        return "true";
    }
    if(stat == "cp")
    {
        return "false";
    }
    if(stat == "cpk")
    {
        return "false";
    }
    if(stat == "ratio")
    {
        return "true";
    }
    return "false";
}

bool CGexMoTaskStatisticalMonitoring::CleanLimits()
{
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    QString taskId = this->GetAttribute(C_TASKID).toString();
    QString versionId = this->GetAttribute(C_VERSION_ID).toString();
    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT_PARAM + " WHERE task_id=" + taskId + " AND version_id=" + versionId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM + " WHERE task_id=" + taskId + " AND version_id=" + versionId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    queryString = "DELETE FROM " + tablePrefix + SM_TABLE_LIMIT + " WHERE task_id=" + taskId + " AND version_id=" + versionId;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }
    return true;
}

bool CGexMoTaskStatisticalMonitoring::ExceedLimits(double value, double lowLimit, bool lowLimitEnabled,  double highLimit, bool highLimitEnabled, double /*threshold*/, const QString& /*itemCat*/)
{
    return ((lowLimitEnabled && (value < lowLimit)) || (highLimitEnabled && (value > highLimit)));
}

bool CGexMoTaskStatisticalMonitoring::CheckAgainstLimits(
        const QString& productName,
        const QString& lot,
        const QString& sublot,
        const QString& wafer,
        const int& splitlot,
        const QChar &executionType,
        QMap<QString, QVariant> &logSummary,
        QList<StatMonAlarm> &alarmsOutput,
        QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > > &monitoredItemToSiteToStatToDataPoint,
        QHash<MonitoredItemDesc, QMap<int, StatMonLimit> > &aItemsLimits,
        const QDateTime *dateFrom,
        const QDateTime *dateTo)
{
    if (!GS::LPPlugin::ProductInfo::getInstance()->isSYLSBLAllowed())
    {
        GSLOG(SYSLOG_SEV_ERROR, (QString("No SPM/SYA license \nPlease contact %1")
                                 .arg(GEX_EMAIL_SALES).toLatin1().constData()));
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrLicenseError, NULL,
                   QString("No SPM/SYA license \nPlease contact %1").arg(GEX_EMAIL_SALES).toLatin1().constData());
        return false;
    }

    int taskId = GetAttribute(C_TASKID).toInt();

    int versionToExecute;
    if(executionType != 'S')
    {
        // On trigger and insertion:

        // Check if rule disabled
        if(!IsUsable())
        {
            // There is no prod version currently active
            GSET_ERROR0(CGexMoTaskStatisticalMonitoring,eErrInvalidTask, NULL);
            return false;
        }

        // Check the renewal status
        if(!CheckForRenewal())
        {
            return false;
        }

        // Execute the active prod version
        versionToExecute = GetActiveProdVersionId();
    }
    else
    {
        // On simulate:

        // Make sure this is a draft version and has limits
        if((GetAttribute(C_VERSION_DRAFT).toInt() != 1) || !HasComputedLimits())
        {
            return false;
        }

        // Execute the current draft version
        versionToExecute = GetAttribute(C_VERSION_ID).toInt();
    }

    ///////////////////
    // Start the log //
    ///////////////////

    QDateTime logDate;
    QString cnxId;
    int logId;

    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    QString queryString;
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));

    queryString ="SELECT NOW() AS log_date, CONNECTION_ID() as cnxId";
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }
    query.first();
    logDate = query.value("log_date").toDateTime();
    cnxId = query.value("cnxId").toString();

    // Create the log
    logSummary["task_id"] = taskId;
    logSummary["version_id"] = versionToExecute;
    logSummary["execution_date"] = logDate;
    logSummary["execution_type"] = executionType;
    logSummary["status"] = "IN PROGRESS";
    logSummary["summary"] = cnxId;

    queryString = "SELECT version_label,expiration_date FROM " + tablePrefix + SM_TABLE_VERSION
                  + " WHERE task_id=" + QString::number(taskId)
                  + " AND version_id=" + QString::number(versionToExecute);
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }
    query.first();
    logSummary["_sm_version_label"] = query.value("version_label").toString();
    logSummary["_sm_version_expiration"] = query.value("expiration_date").toDateTime();

    // Write it in the database
    if(!WriteLog(logSummary))
    {
        return false;
    }

    // Retrieve the log_id
    queryString = "SELECT log_id FROM " + tablePrefix + SM_TABLE_LOG
                + " WHERE task_id=" + QString::number(taskId)
                + " AND version_id=" + QString::number(versionToExecute)
                + " AND execution_date='" + logDate.toString("yyyy-MM-dd hh:mm:ss") + "'"
                + " AND summary='" + cnxId + "'";
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }
    query.first();
    logId = query.value("log_id").toInt();

    /////////////////////////////////////////////////////////////
    // Get the list of excluded items from the task definition //
    /////////////////////////////////////////////////////////////

    QString itemRegexp = GetAttribute(C_MONITOREDITEMREGEXP).toString();
    QStringList excludedItems;
    if(!itemRegexp.isEmpty())
    {
        QListIterator<QString> iter(itemRegexp.split(','));
        while(iter.hasNext())
        {
            QString itemReg = iter.next().trimmed();
            if(itemReg.startsWith('!'))
            {
                excludedItems.append(itemReg.remove('!'));
            }
        }
    }

    //////////////////////////
    // Fetch the datapoints //
    //////////////////////////

    QString whereClause = " WHERE L.task_id=" + QString::number(taskId)
                          + " AND L.version_id=" + QString::number(versionToExecute)
                          + " AND enabled=1 AND (ll_enabled=1 OR hl_enabled=1)";

    // Query the list of monitored items, sites and stats monitored by a limit
    queryString = "SELECT DISTINCT monitored_item_type,monitored_item_num,monitored_item_name,monitored_item_cat"
                  " FROM "
                       + tablePrefix + SM_TABLE_LIMIT + " L"
                       + " INNER JOIN " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM + " LMI "
                          " ON LMI.task_id=L.task_id"
                          " AND LMI.version_id=L.version_id"
                          " AND LMI.limit_id=L.limit_id"
                       + " INNER JOIN " + tablePrefix + SM_TABLE_MONITORED_ITEM + " MI "
                          " ON MI.task_id=L.task_id"
                          " AND MI.monitored_item_id=LMI.monitored_item_id"
                  + whereClause;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());

        logSummary["status"] = "FAIL";
        logSummary["summary"] = "Error getting monitored item list: " + query.lastError().text() + "\nUsing query: " + query.lastQuery();
        UpdateLog(logId, logSummary);

        return false;
    }

    QList<MonitoredItemDesc> lItemList;
    while(query.next())
    {
        lItemList.append(CreateItemDesc(query.value("monitored_item_type").toString(),
                                        query.value("monitored_item_num").toString(),
                                        query.value("monitored_item_name").toString(),
                                        query.value("monitored_item_cat").toString()));
    }
    if(lItemList.length() == 0)
    {
        logSummary["status"] = "PASS";
        logSummary["summary"] = "No activated limits needs to be checked";
        UpdateLog(logId, logSummary);

        return true;
    }

    queryString = "SELECT DISTINCT(site_no)"
                  " FROM "
                       + tablePrefix + SM_TABLE_LIMIT + " L "
                  + whereClause;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());

        logSummary["status"] = "FAIL";
        logSummary["summary"] = "Error getting site list: " + query.lastError().text() + "\nUsing query: " + query.lastQuery();
        UpdateLog(logId, logSummary);

        return false;
    }
    QList<int> siteList;
    while(query.next())
    {
        siteList.append(query.value("site_no").toInt());
    }

    queryString = "SELECT DISTINCT(stat_name)"
                  " FROM "
                       + tablePrefix + SM_TABLE_LIMIT + " L "
                  + whereClause;
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());

        logSummary["status"] = "FAIL";
        logSummary["summary"] = "Error getting stat list: " + query.lastError().text() + "\nUsing query: " + query.lastQuery();
        UpdateLog(logId, logSummary);

        return false;
    }
    QStringList statList;
    while(query.next())
    {
        statList.append(query.value("stat_name").toString());
    }

    // Get the datapoints
    QString resolvedProductList;
    QString resolvedLotList;
    QString resolvedSublotList;
    QString resolvedWaferList;
    int resolvedNumParts;

    if(!FetchDataPointsForCheck(executionType,
                                GetAttribute(C_TESTINGSTAGE).toString(),
                                productName,
                                lot,
                                sublot,
                                wafer,
                                splitlot,
                                GetProperties()->filtersMetaData,
                                lItemList,
                                excludedItems,
                                mUniqueKeyRule,
                                GetAttribute(C_TESTFLOW).toString(),
                                GetAttribute(C_CONSOLIDATION_TYPE).toString(),
                                GetAttribute(C_CONSOLIDATION_LEVEL).toString(),
                                GetAttribute(C_INSERTION).toString(),
                                siteList,
                                statList,
                                GetAttribute(C_GROSSDIE).toBool(),
                                dateFrom,
                                dateTo,
                                resolvedProductList,
                                resolvedLotList,
                                resolvedSublotList,
                                resolvedWaferList,
                                resolvedNumParts,
                                monitoredItemToSiteToStatToDataPoint))
    {
        logSummary["status"] = "FAIL";
        logSummary["summary"] = "Error fetching the datapoints from the database";
        UpdateLog(logId, logSummary);

        return false;
    }

    if(monitoredItemToSiteToStatToDataPoint.size() == 0)
    {
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrNoDataPointsError, NULL, QString("[%1] No valid datapoints found for the time period and parameters specified").arg(m_strName).toLatin1().constData());

        logSummary["status"] = "FAIL";
        logSummary["summary"] = "[" + m_strName + "] No valid datapoints found to check with the provided parameters";
        UpdateLog(logId, logSummary);

        return false;
    }

    logSummary["matched_products"] = resolvedProductList;
    logSummary["matched_lots"] = resolvedLotList;
    logSummary["matched_sublots"] = resolvedSublotList;
    logSummary["matched_wafers"] = resolvedWaferList;
    logSummary["nb_parts"] = resolvedNumParts;
    logSummary["status"] = "IN PROGRESS";
    logSummary["summary"] = "No Alarm";
    if(!UpdateLog(logId, logSummary))
    {
        return false;
    }

    /////////////////////////
    // Get the task limits //
    /////////////////////////

    // Iterate through the critical limits
    queryString = "SELECT L.limit_id, MI.monitored_item_type, GROUP_CONCAT(CONCAT(MI.monitored_item_id, '|', MI.monitored_item_num, '|', MI.monitored_item_name, '|', MI.monitored_item_cat, '|', MI.monitored_item_unit) SEPARATOR ',') as items, L.site_no, L.stat_name, L.ll, L.ll_enabled, L.hl, L.hl_enabled, T.threshold"
                  " FROM " + tablePrefix + SM_TABLE_LIMIT + " L"
                  " INNER JOIN " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM + " LMI ON LMI.task_id=L.task_id AND LMI.version_id=L.version_id AND LMI.limit_id=L.limit_id"
                  " INNER JOIN " + tablePrefix + SM_TABLE_MONITORED_ITEM + " MI ON MI.task_id=L.task_id AND MI.monitored_item_id=LMI.monitored_item_id"
                  " INNER JOIN " + tablePrefix + SM_TABLE_MAIN + " T ON T.task_id=L.task_id"
                  + whereClause + " AND L.criticity_level=2"
                  " GROUP BY L.limit_id";

    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());

        logSummary["status"] = "FAIL";
        logSummary["summary"] = "Error getting the spm task critical limits: " + query.lastError().text() + "\nUsing query: " + query.lastQuery();
        UpdateLog(logId, logSummary);

        return false;
    }
    while(query.next())
    {
        double taskThreshold = query.value("threshold").toDouble();
        int limitId = query.value("limit_id").toInt();
        int site = query.value("site_no").toInt();
        QString statName = query.value("stat_name").toString();
        double ll = query.value("ll").toDouble();
        bool ll_enabled = query.value("ll_enabled").toBool();
        double hl = query.value("hl").toDouble();
        bool hl_enabled = query.value("hl_enabled").toBool();
        QString itemType = query.value("monitored_item_type").toString();

        QListIterator<QString> iterItems(query.value("items").toString().split(','));
        while(iterItems.hasNext())
        {
            QStringList itemValues = iterItems.next().split('|');
            int itemId = itemValues[0].toInt();
            QString itemNum =  itemValues[1];
            QString itemName = itemValues[2];
            QString itemCat = itemValues[3];
            QString itemUnit = itemValues[4];
            MonitoredItemDesc itemUniqueKey = CreateItemDesc(itemType.toUpper(), itemNum, itemName, itemCat, itemUnit);
            QList<StatMonDataPoint> dataPoints = monitoredItemToSiteToStatToDataPoint[itemUniqueKey][site].values(statName);
            QListIterator<StatMonDataPoint> iterDataPoints(dataPoints);

            QMap<int, StatMonLimit> lLimitMap;
            lLimitMap.insert(2, StatMonLimit(limitId, site, statName, ll, ll_enabled, hl, hl_enabled, 2));
            aItemsLimits.insert(itemUniqueKey, lLimitMap);

            while(iterDataPoints.hasNext())
            {
                StatMonDataPoint dataPoint = iterDataPoints.next();
                if(ExceedLimits(dataPoint.mValue, ll, ll_enabled, hl, hl_enabled, taskThreshold, itemCat))
                {
                    alarmsOutput.append(StatMonAlarm(logDate,
                                                     dataPoint.mProduct,
                                                     dataPoint.mLot,
                                                     dataPoint.mSublot,
                                                     dataPoint.mWafer,
                                                     dataPoint.mSplitlot,
                                                     2,
                                                     itemId,
                                                     itemType,
                                                     itemNum,
                                                     itemName,
                                                     itemCat,
                                                     itemUnit,
                                                     limitId,
                                                     statName,
                                                     site,
                                                     dataPoint.mNumExecutions,
                                                     dataPoint.mNumFails,
                                                     ll,
                                                     dataPoint.mValue,
                                                     hl));
                }
            }
        }
    }

    bool criticalAlarmRaised = false;
    if(alarmsOutput.size() != 0)
    {
        criticalAlarmRaised = true;
        logSummary["summary"] = "Critical Alarm";
        if(!UpdateLog(logId, logSummary))
        {
            return false;
        }
    }

    // Write the alarms
    if(!WriteAlarms(logId, taskId, versionToExecute, alarmsOutput))
    {
        logSummary["status"] = "FAIL";
        logSummary["summary"] = "Error writing alarms in the database";
        UpdateLog(logId, logSummary);

        return false;
    }

    // Iterate through the standard limits with no critical alarm
    queryString = "SELECT L.limit_id, MI.monitored_item_type, GROUP_CONCAT(CONCAT(MI.monitored_item_id, '|', MI.monitored_item_num, '|', MI.monitored_item_name, '|', MI.monitored_item_cat, '|', MI.monitored_item_unit) SEPARATOR ',') as items, L.site_no, L.stat_name, L.ll, L.ll_enabled, L.hl, L.hl_enabled, T.threshold"
                  " FROM " + tablePrefix + SM_TABLE_LIMIT + " L"
                  " INNER JOIN " + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM + " LMI ON LMI.task_id=L.task_id AND LMI.version_id=L.version_id AND LMI.limit_id=L.limit_id"
                  " INNER JOIN " + tablePrefix + SM_TABLE_MONITORED_ITEM + " MI ON MI.task_id=L.task_id AND MI.monitored_item_id=LMI.monitored_item_id"
                  " INNER JOIN " + tablePrefix + SM_TABLE_MAIN + " T ON T.task_id=L.task_id"
                  + whereClause + " AND L.criticity_level=1"
                  " GROUP BY L.limit_id";
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());

        logSummary["status"] = "FAIL";
        logSummary["summary"] = "Error getting the spm task standard limits: " + query.lastError().text() + "\nUsing query: " + query.lastQuery();
        UpdateLog(logId, logSummary);

        return false;
    }
    while(query.next())
    {
        double taskThreshold = query.value("threshold").toDouble();
        int limitId = query.value("limit_id").toInt();
        int site = query.value("site_no").toInt();
        QString statName = query.value("stat_name").toString();
        double ll = query.value("ll").toDouble();
        bool ll_enabled = query.value("ll_enabled").toBool();
        double hl = query.value("hl").toDouble();
        bool hl_enabled = query.value("hl_enabled").toBool();
        QString itemType = query.value("monitored_item_type").toString();

        QListIterator<QString> iterItems(query.value("items").toString().split(','));
        while(iterItems.hasNext())
        {
            QStringList itemValues = iterItems.next().split('|');
            int itemId = itemValues[0].toInt();
            QString itemNum =  itemValues[1];
            QString itemName = itemValues[2];
            QString itemCat = itemValues[3];
            QString itemUnit = itemValues[4];
            MonitoredItemDesc itemUniqueKey = CreateItemDesc(itemType.toUpper(), itemNum, itemName, itemCat, itemUnit);
            QList<StatMonDataPoint> dataPoints = monitoredItemToSiteToStatToDataPoint[itemUniqueKey][site].values(statName);
            QListIterator<StatMonDataPoint> iterDataPoints(dataPoints);

            aItemsLimits[itemUniqueKey].insert(1, StatMonLimit(limitId, site, statName, ll, ll_enabled, hl, hl_enabled, 1));

            while(iterDataPoints.hasNext())
            {
                StatMonDataPoint dataPoint = iterDataPoints.next();
                if(ExceedLimits(dataPoint.mValue, ll, ll_enabled, hl, hl_enabled, taskThreshold, itemCat))
                {
                    // only insert an alarm if the same datapoint hasn't already triggered a critical alarm
                    // for the same monitored item
                    bool criticalAlarm = false;
                    for(QList<StatMonAlarm>::const_iterator lIter = alarmsOutput.begin(), lEnd = alarmsOutput.end(); lIter != lEnd; ++lIter)
                    {
                        const StatMonAlarm& alarm = *lIter;
                        if(
                            alarm.productName == dataPoint.mProduct
                            &&
                            alarm.lotId == dataPoint.mLot
                            &&
                            alarm.sublotId == dataPoint.mSublot
                            &&
                            alarm.waferId == dataPoint.mWafer
                            &&
                            alarm.MonitoredItemId == itemId
                        )
                        {
                            criticalAlarm = true;
                            break;
                        }
                    }
                    if(!criticalAlarm)
                    {
                        alarmsOutput.append(StatMonAlarm(logDate,
                                                         dataPoint.mProduct,
                                                         dataPoint.mLot,
                                                         dataPoint.mSublot,
                                                         dataPoint.mWafer,
                                                         dataPoint.mSplitlot,
                                                         1,
                                                         itemId,
                                                         itemType,
                                                         itemNum,
                                                         itemName,
                                                         itemCat,
                                                         itemUnit,
                                                         limitId,
                                                         statName,
                                                         site,
                                                         dataPoint.mNumExecutions,
                                                         dataPoint.mNumFails,
                                                         ll,
                                                         dataPoint.mValue,
                                                         hl));
                    }
                }
            }
        }
    }

    if(alarmsOutput.size() != 0 && !criticalAlarmRaised)
    {
        logSummary["summary"] = "Standard Alarm";
        if(!UpdateLog(logId, logSummary))
        {
            return false;
        }
    }

    // Write the alarms
    if(!WriteAlarms(logId, taskId, versionToExecute, alarmsOutput))
    {
        logSummary["status"] = "FAIL";
        logSummary["summary"] = "Error writing alarms in the database";
        UpdateLog(logId, logSummary);

        return false;
    }

    // Finalize the log
    logSummary["nb_alarms"] = alarmsOutput.size();
    logSummary["status"] = "PASS";

    // Write the log
    if(!UpdateLog(logId, logSummary))
    {
        return false;
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::CheckForRenewal()
{
    RenewalStatus status;
    bool sendMail;
    bool renew;
    bool executeAnyway = false;
    if(!CheckRenewalStatus(status, sendMail, renew))
    {
        GSET_ERROR0(CGexMoTaskStatisticalMonitoring,eErrRenewalCheckFailed, NULL);
        return false;
    }

    if(status != OK)
    {
        // The task requires emailing and perhaps renewal, let's lock
        // it to prevent concurrency during this process
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().Lock(this))
        {
            QString mailTitle;
            QString mailBody;

            switch(status)
            {
                case Warning:
                    mailTitle = GetTypeName() + " task will expire soon";
                    mailBody = "Your " + GetTypeName() + " task titled \"" + GetName() +
                               "\" is set to expire on " + GetAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONDATE).toDateTime().toString("d MMMM yyyy") + ".";
                break;
                case Warning24h:
                    mailTitle = GetTypeName() + " task will expire in 24h";
                    mailBody = "Your " + GetTypeName() + " task titled \"" + GetName() +
                               "\" is set to expire in 24 hours.";
                break;
                case Expired:
                    mailTitle = GetTypeName() + " task has expired";
                    mailBody = "Your " + GetTypeName() + " task titled \"" + GetName() +
                               "\" has expired and can't be executed anymore.";
                break;
                default:
                    // unexpected situation
                    GSET_ERROR0(CGexMoTaskStatisticalMonitoring,eErrUnexpectedTaskState, NULL);
                    return false;
            }

            if(renew)
            {
                QString internalError;
                switch(RenewActiveVersion())
                {
                    case RenewalOK:
                        mailBody += " It was automatically renewed according to your configuration.\n";
                        executeAnyway = true;
                    break;
                    case RenewalDuplicated:
                        internalError = GGET_LASTERRORMSG(CGexMoTaskStatisticalMonitoring,this);
                        mailBody += " It was automatically renewed using previous limits due to an issue in new limits computation:\n\""
                                 + internalError + "\"\n";
                        executeAnyway = true;
                    break;
                    default:
                        mailBody += " Automatic renewal failed. Please log into the task scheduler to"
                                    " make the eventual adjustments for renewal.\n";
                    break;
                }
            }
            else
            {
                mailBody += " Please log into the task scheduler to"
                            " make the eventual adjustments for renewal.\n";
            }

            if(sendMail)
            {
                if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().SendEmail(
                        GetAttribute(C_EMAILFROM).toString(),
                        GetAttribute(C_EMAILTO).toString(),
                        mailTitle,
                        mailBody,
                        false,
                        "") != "ok")
                {
                    // mail send failed
                    GSET_ERROR0(CGexMoTaskStatisticalMonitoring,eErrMailSendFailed, NULL);
                    return false;
                }
            }

            // Save using Scheduler to update ym_tasks' "update_date" and force other
            // yieldman nodes to reload
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveDbTasks(this);

            // And unlock the task
            GS::Gex::Engine::GetInstance().GetAdminEngine().Unlock(this);
        }
    }

    if(status == Expired)
    {
        if(!executeAnyway)
        {
            GSET_ERROR0(CGexMoTaskStatisticalMonitoring,eErrTaskExpired, NULL);
            return false;
        }
        // The task is expired, but new limits have been set. Load them and run
        // the task.
        if(!LoadTaskDetails())
        {
            GSET_ERROR0(CGexMoTaskStatisticalMonitoring,eErrInvalidTask, NULL);
            return false;
        }
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::CheckRenewalStatus(RenewalStatus& status, bool& sendMail, bool& renew)
{
    status = OK;
    sendMail = GetAttribute(C_SENDEMAILBEFOREEXPIRATION).toBool();
    renew = GetAttribute(C_AUTORECOMPUTE).toBool();

    // Retrieve the active prod version data

    int activeId, latestId;
    QDateTime activeExpirationDate, activeWarningDate, latestStartDate;
    QDateTime currentDate = GS::Gex::Engine::GetInstance().GetServerDateTime();
    QString errorMsg;
    if(
        !GetActiveProdVersionExpirationDate(activeId, activeExpirationDate, activeWarningDate, errorMsg)
        ||
        !GetLatestProdVersionExpirationDate(latestId, latestStartDate, errorMsg)
    )
    {
        // error getting version information
        return false;
    }

    // Check the task status

    if((currentDate < activeWarningDate) && (currentDate < activeExpirationDate))
    {
        // The active task is valid, there's nothing to do
        return true;
    }

    // The task needs attention

    // Because the renewal status could have been modified by another
    // yieldman node since the task was last loaded (concurrency situation)
    // we reload the data from the DB, to make sure the task is up to date

    if(!LoadTaskDetails())
    {
        // error
        return false;
    }

    // Now, let's check if the active prod version has changed

    int oldActiveId = activeId;
    if(!GetActiveProdVersionExpirationDate(activeId, activeExpirationDate, activeWarningDate, errorMsg))
    {
        // error getting version information
        return false;
    }
    if(oldActiveId != activeId)
    {
        // A new prod version is active, is this new version expired?
        if((currentDate < activeWarningDate) && (currentDate < activeExpirationDate))
        {
            // The active task is valid, there's nothing to do
            return true;
        }
    }

    // The active task requires attention

    if(latestId != activeId)
    {
        // A more recent version is waiting to replace the active one.
        // Disregarding whether it's a user created or automatically
        // renewed one, there's nothing to do
        return true;
    }

    // No new version is pending for activation

    int warningDone = GetAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDONE).toInt();
    if((activeExpirationDate < currentDate) && (warningDone < 3))
    {
        status = Expired;
        return SetWarningDone(3);
    }
    else if((activeExpirationDate.addDays(-1) < currentDate) && (warningDone < 2))
    {
        status = Warning24h;
        return SetWarningDone(2);
    }
    else if(warningDone < 1)
    {
        status = Warning;
        return SetWarningDone(1);
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::SetWarningDone(int value)
{
    if(GetAttribute(C_VERSION_ACTIVE_PROD_ID).toInt() != -1)
    {
        QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
        QString queryString;
        queryString = "UPDATE " + GetStatisticalMonitoringTablesPrefix() + SM_TABLE_VERSION + " SET ";
        queryString += "expiration_warning_done=" + QString::number(value);
        queryString += " WHERE task_id=" + this->GetAttribute(C_TASKID).toString();
        queryString += " AND version_id=" + this->GetAttribute(C_VERSION_ACTIVE_PROD_ID).toString();
        if(!query.exec(queryString))
        {
            GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
            return false;
        }
        UpdateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDONE, value);
        return true;
    }

    return false;
}

RenewalResult CGexMoTaskStatisticalMonitoring::RenewActiveVersion()
{
    int newProdVersion;

    // duplicate the last prod's version and stat values into a new draft
    if(!DbCloneActiveProdVersionForRenewal(newProdVersion))
    {
        return RenewalFailed;
    }

    if(GetAttribute(C_AUTORECOMPUTEMETHOD).toString() != "duplicate")
    {
        // Launch a limit update
        if(!ComputeLimits(true, NULL, newProdVersion))
        {
            if(GetAttribute(C_AUTORECOMPUTEMETHOD).toString() == "duplicateIfRecomputeFails")
            {
                return RenewalDuplicated;
            }
            else
            {
                DeleteVersionFromDB(newProdVersion);

                return RenewalFailed;
            }
        }
    }

    return RenewalOK;
}

bool CGexMoTaskStatisticalMonitoring::DbCloneActiveProdVersionForRenewal(int& newProdVersion)
{
    // Load the latest version
    LoadTaskDetails();

    // Check if the version is a draft
    if(!GetAttribute(C_VERSION_DRAFT).toBool())
    {
        // This is not supposed to happen
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrCloneLimitsForRenewalFailed, NULL,
                   QString("The loaded version is not a draft").toLatin1().constData());
        return false;
    }

    int activeProdVersion = GetAttribute(C_VERSION_ACTIVE_PROD_ID).toInt();
    if(activeProdVersion == 0)
    {
        // There is no production version to clone
        GSET_ERROR1(CGexMoTaskStatisticalMonitoring, eErrCloneLimitsForRenewalFailed, NULL,
                   QString("There is no production version available for cloning").toLatin1().constData());
        return false;
    }
    int currentDraftVersion = GetAttribute(C_VERSION_ID).toInt();

    // Duplicate the draft
    UpdateAttribute(C_VERSION_ID, -1);
    UpdateAttribute(C_VERSION_CREATIONDATE, GS::Gex::Engine::GetInstance().GetServerDateTime());
    if(!SaveTaskDetails())
    {
        return false;
    }
    int newDraftVersion = GetAttribute(C_VERSION_ID).toInt();
    if(!DbDuplicateLimits(currentDraftVersion, newDraftVersion))
    {
        return false;
    }

    // Clone the active prod into the current draft
    newProdVersion = currentDraftVersion;

    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString;

    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    QDateTime currentDateTime = GS::Gex::Engine::GetInstance().GetServerDateTime();
    QDateTime creationDate = currentDateTime;
    QDateTime fromDate = currentDateTime.addDays(-GetAttribute(C_AUTORECOMPUTEPERIOD).toInt());
    QDateTime toDate = currentDateTime;
    QDateTime startDate = qMax(GetAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONDATE).toDateTime(), currentDateTime);
    QDateTime expirationDate = startDate.addDays(GetAttribute(C_VALIDITYPERIOD).toInt());
    QDateTime warningDate = expirationDate.addDays(-GetAttribute(C_DAYSBEFOREEXPIRATION).toInt());

    queryString = "UPDATE " + tablePrefix + SM_TABLE_VERSION + " as dest "
               "INNER JOIN " + tablePrefix + SM_TABLE_VERSION + " as src "
                    "ON dest.task_id=src.task_id "
                    "AND dest.version_id=" + QString::number(newProdVersion) + " "
                    "AND src.version_id=" + QString::number(activeProdVersion) + " "
               "SET "
                    "dest.draft_version=0,"
                    "dest.version_label=src.version_label,"
                    "dest.creation_date=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(creationDate) + ","
                    "dest.computation_fromdate=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(fromDate) + ","
                    "dest.computation_todate=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(toDate) + ","
                    "dest.start_date=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(startDate) + ","
                    "dest.expiration_date=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(expirationDate) + ","
                    "dest.expiration_warning_date=" + GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeDateToSql(warningDate) + " "
               "WHERE "
                    "src.task_id=" + QString::number(GetAttribute(C_TASKID).toInt());
    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    // Clone the draft's limits using the latest prod
    if(!DbDuplicateLimits(activeProdVersion, newProdVersion))
    {
        return false;
    }

    // Update ym_admin_db task's "last_update" to force reloading by other nodes
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveDbTasks(this);
}

bool CGexMoTaskStatisticalMonitoring::WriteLog(const QMap<QString, QVariant> &logSummary)
{
    QStringList fields;
    QStringList values;
    QList<QString> lKeys = logSummary.keys();
    QString key;
    for(int i=0; i<lKeys.size(); ++i)
    {
        key = lKeys.at(i);
        if(!key.startsWith('_'))
        {
            fields.append(key);
            QVariant value = logSummary[key];
            switch(value.type())
            {
                case QVariant::String:
                    values.append("'" + value.toString() + "'");
                break;
                case QVariant::Char:
                    values.append('\'' + value.toChar() + '\'');
                break;
                case QVariant::Int:
                    values.append(QString::number(value.toInt()));
                break;
                case QVariant::Double:
                    values.append(QString::number(value.toDouble()));
                break;
                case QVariant::DateTime:
                    values.append("'" + value.toDateTime().toString("yyyy-MM-dd hh:mm:ss") + "'");
                break;
                default:
                    values.append("null");
                break;
            }
        }
    }

    QString queryString = "INSERT INTO " + GetStatisticalMonitoringTablesPrefix() + SM_TABLE_LOG + " (" + fields.join(',') + ") VALUES (" + values.join(',') + ")";
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));

    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::UpdateLog(int logId, const QMap<QString, QVariant> &logSummary)
{
    QStringList fieldsAndValues;
    QList<QString> lKeys = logSummary.keys();
    QString key;
    for(int i=0; i<lKeys.size(); ++i)
    {
        key = lKeys.at(i);
        if(!key.startsWith('_'))
        {
            QVariant value = logSummary[key];
            switch(value.type())
            {
                case QVariant::String:
                    fieldsAndValues.append(key + "='" + value.toString() + "'");
                break;
                case QVariant::Char:
                    fieldsAndValues.append(key + "=\'" + value.toChar() + '\'');
                break;
                case QVariant::Int:
                    fieldsAndValues.append(key + "=" + QString::number(value.toInt()));
                break;
                case QVariant::Double:
                    fieldsAndValues.append(key + "=" + QString::number(value.toDouble()));
                break;
                case QVariant::DateTime:
                    fieldsAndValues.append(key + "='" + value.toDateTime().toString("yyyy-MM-dd hh:mm:ss") + "'");
                break;
                default:
                break;
            }
        }
    }

    QString queryString = "UPDATE " + GetStatisticalMonitoringTablesPrefix() + SM_TABLE_LOG + " SET " + fieldsAndValues.join(',') + " WHERE log_id=" + QString::number(logId);
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));

    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}

bool CGexMoTaskStatisticalMonitoring::WriteAlarms(int logId, int taskId, int versionId, QList<StatMonAlarm> &alarms)
{
    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    QString queryString = "INSERT INTO " + GetStatisticalMonitoringTablesPrefix() + SM_TABLE_ALARM
                          + "(log_id, task_id, version_id, limit_id, execution_date, "
                            "lot_id, sublot_id, wafer_id, splitlot_id, product_name, monitored_item_id, site_no, exec_count, fail_count, "
                            "criticity_level, stat_name, ll, hl, outlier_value) VALUES ";
    QStringList alarmsString;

    for(int i = 0; i < alarms.size(); ++i)
    {
        StatMonAlarm* alarm = &alarms[i];
        if(!alarm->alreadyWritten)
        {
            alarm->alreadyWritten = true;
            alarmsString.append("("
                                + QString::number(logId) + ","
                                + QString::number(taskId) + ","
                                + QString::number(versionId) + ","
                                + QString::number(alarm->limitId) + ","
                                + "'" + alarm->execDate.toString("yyyy-MM-dd hh:mm:ss") + "',"
                                + "'" + alarm->lotId + "',"
                                + "'" + alarm->sublotId + "',"
                                + "'" + alarm->waferId + "',"
                                + QString::number(alarm->splitlotId) + ","
                                + "'" + alarm->productName + "',"
                                + QString::number(alarm->MonitoredItemId) + ","
                                + QString::number(alarm->siteNum) + ","
                                + QString::number(alarm->execCount) + ","
                                + QString::number(alarm->failCount) + ","
                                + QString::number(alarm->criticityLevel) + ","
                                + "'" + alarm->statName + "',"
                                + QString::number(alarm->lowLimit) + ","
                                + QString::number(alarm->highLimit) + ","
                                + QString::number(alarm->outlierValue)
                                + ")");
            alarm->alreadyWritten = true;
        }

        if(alarmsString.size() > 1000)
        {
            // Insert the 1000 alarms
            if(!query.exec(queryString + alarmsString.join(',')))
            {
                GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.left(1000).toLatin1().constData(), query.lastError().text().toLatin1().constData());
                return false;
            }
            alarmsString.clear();
        }

    }

    if(alarmsString.size() != 0)
    {
        // Insert the last alarms
        if(!query.exec(queryString + alarmsString.join(',')))
        {
            GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.left(1000).toLatin1().constData(), query.lastError().text().toLatin1().constData());
            return false;
        }
        alarmsString.clear();
    }

    return true;
}

void CGexMoTaskStatisticalMonitoring::UpdatePrivateAttributes()
{
    return;
}

SMSQLTableModel* CGexMoTaskStatisticalMonitoring::CreateVersionsModel( )
{
    SMSQLTableModel *lModel = 0;

    lModel = new SMSQLTableModel(this, 0, QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    lModel->setTable(GetStatisticalMonitoringTablesPrefix() + SM_TABLE_VERSION);
    lModel->setFilter(QString("task_id=%1 AND draft_version=0").arg(GetAttribute(C_TASKID).toInt()));
    lModel->setHeaderData(0, Qt::Horizontal, "task ID", Qt::DisplayRole);
    lModel->setHeaderData(1, Qt::Horizontal, "version#", Qt::DisplayRole);
    lModel->setHeaderData(2, Qt::Horizontal, "draft", Qt::DisplayRole);
    lModel->setHeaderData(3, Qt::Horizontal, "comment", Qt::DisplayRole);
    lModel->setHeaderData(4, Qt::Horizontal, "matched products", Qt::DisplayRole);
    lModel->setHeaderData(5, Qt::Horizontal, "creation date", Qt::DisplayRole);
    lModel->setHeaderData(6, Qt::Horizontal, "start date", Qt::DisplayRole);
    lModel->setHeaderData(7, Qt::Horizontal, "expiration date", Qt::DisplayRole);
    lModel->setHeaderData(8, Qt::Horizontal, "expiration warning date", Qt::DisplayRole);
    lModel->setHeaderData(9, Qt::Horizontal, "expiration warning status", Qt::DisplayRole);
    lModel->setHeaderData(10, Qt::Horizontal, "computed from", Qt::DisplayRole);
    lModel->setHeaderData(11, Qt::Horizontal, "computed to", Qt::DisplayRole);

    return lModel;

}

QString CGexMoTaskStatisticalMonitoring::GetLimitsExtractionQuery(QStringList visibleFields, QStringList idFields, bool lastProdVersion/* =false */)
{
    QString taskId = this->GetAttribute(C_TASKID).toString().trimmed();
    QString versionId;

    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    if (lastProdVersion)
        versionId = QString::number(this->GetActiveProdVersionId());
    else
        versionId = this->GetAttribute(C_VERSION_ID).toString();

    QStringList lFields;
    lFields << visibleFields;
    QStringList lHiddenFields = idFields;
    for (int lIt = 0; lIt < lHiddenFields.size(); ++lIt)
    {
        lHiddenFields[lIt].prepend("L.");
    }
    lFields << lHiddenFields;

    QString lQueryString = "SELECT "
            "" + lFields.join(",") +
            " FROM "
            "" + tablePrefix + SM_TABLE_LIMIT + " L "
            "    LEFT OUTER JOIN "
            "" + tablePrefix + SM_TABLE_LIMIT + SM_TABLE_MONITORED_ITEM + " LMI "
                "    ON LMI." + SM_ID + " = L." + SM_ID + " "
                "    AND LMI." + SM_VERSION_ID + " = L." + SM_VERSION_ID + " "
                "    AND LMI." + SM_LIMIT_ID + " = L." + SM_LIMIT_ID + " "
            "    LEFT OUTER JOIN "
            "" + tablePrefix + SM_TABLE_MONITORED_ITEM + " MI "
                "    ON MI." + SM_ID + " = L." + SM_ID + " "
                "    AND MI." + SM_MONITORED_ITEM_ID + " = LMI." + SM_MONITORED_ITEM_ID + " "
            "    LEFT OUTER JOIN "
            "" + tablePrefix + SM_TABLE_LIMIT_PARAM + " LP "
                "    ON LP." + SM_ID + " = L." + SM_ID + " "
                "    AND LP." + SM_VERSION_ID + " = L." + SM_VERSION_ID + " "
                "    AND LP." + SM_LIMIT_ID + " = L." + SM_LIMIT_ID + " "
                "    AND LP.param_name='N'"
            " WHERE "
            "    L." + SM_ID + " = " + taskId + " AND "
            "    L." + SM_VERSION_ID + " = " + versionId + " "
            " GROUP BY L." + SM_LIMIT_ID;

    return lQueryString;
}

bool CGexMoTaskStatisticalMonitoring::UpdateStatTable(QString tableName,
                                                      QList<QPair<QString, QString> > updateFieldValuePairs,
                                                      QList<QPair<QString, QString> > filterFieldValuePairs)
{
    if (tableName.isEmpty() || updateFieldValuePairs.isEmpty())
        return false;

    QString tablePrefix = GetStatisticalMonitoringTablesPrefix();

    QSqlQuery query(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));

    QStringList lSetStmt;
    for (int lIt = 0; lIt < updateFieldValuePairs.size(); ++lIt)
    {
        lSetStmt << updateFieldValuePairs.at(lIt).first + "='" + updateFieldValuePairs.at(lIt).second + "'";
    }
    QStringList lWhereStmt;
    for (int lIt = 0; lIt < filterFieldValuePairs.size(); ++lIt)
    {
        lWhereStmt << filterFieldValuePairs.at(lIt).first + "=" + filterFieldValuePairs.at(lIt).second;
    }
    QString queryString = "UPDATE " + tablePrefix + tableName + " "
                            "SET " + lSetStmt.join(",") + " "
                            "WHERE " + lWhereStmt.join(" AND ")+ "";

    if(!query.exec(queryString))
    {
        GSET_ERROR2(CGexMoTaskStatisticalMonitoring, eErrDataBaseError, NULL, queryString.toLatin1().constData(), query.lastError().text().toLatin1().constData());
        return false;
    }

    return true;
}



MonitoredItemDesc CGexMoTaskStatisticalMonitoring::CreateItemDesc(QString itemType, QString itemNum, QString itemName, QString itemCat, QString itemUnit, int itemScale)
{
    return MonitoredItemDesc::CreateItemDesc(mUniqueKeyRule, itemType, itemNum, itemName, itemCat, itemUnit, itemScale);
}
