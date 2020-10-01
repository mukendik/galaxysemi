#ifndef STATISTICAL_MONITORING_TASKDATA_H
#define STATISTICAL_MONITORING_TASKDATA_H

#include <QDate>
#include <QObject>
#include "task_properties.h"
#include "gex_database_filter.h"

#define GEX_MERGE_NONE  "per_site"
#define GEX_MERGE_MERGE "merged_sites"
#define GEX_MERGE_BOTH  "both"

#define GEX_EXCEPTION_STANDARD  "standard"
#define GEX_EXCEPTION_CRITICAL  "critical"

#define GEXDB_PLUGIN_GALAXY_ETEST "E-Test"
#define GEXDB_PLUGIN_GALAXY_WTEST "Wafer Sort"
#define GEXDB_PLUGIN_GALAXY_FTEST "Final Test"

#define GEXDB_CONSO_TYPE_RAW "raw"
#define GEXDB_CONSO_TYPE_CONSO "consolidated"

#define GEXDB_CONSO_LEVEL_INSERT "test_insertion"
#define GEXDB_CONSO_LEVEL_FLOW "test_flow"

// properties keys
const QString C_TITLE = "Title";
const QString C_DATABASE = "Database";
const QString C_TESTINGSTAGE = "TestingStage";
const QString C_TASKID = "TaskId";
const QString C_SPM_TDRSPMID = "TdrSpmId"; // for backward compatibility
const QString C_SYA_TDRSYAID = "TdrSyaId"; // for backward compatibility
const QString C_STATSTOMONITOR = "StatsToMonitor";
const QString C_ACTIVEONTRIGGER = "ActiveOnTrigger";
const QString C_ACTIVEONINSERT = "ActiveOnInsert";
const QString C_PRODUCTREGEXP = "ProductRegexp";
const QString C_MONITOREDITEMTYPE = "MonitoredItemType";
const QString C_MONITOREDITEMREGEXP = "MonitoredItemRegexp";
const QString C_TESTFLOW = "TestFlow";
const QString C_CONSOLIDATION_TYPE = "ConsolidationType";
const QString C_CONSOLIDATION_LEVEL = "ConsolidationName";
const QString C_INSERTION = "Insertion";
const QString C_SITEMERGEMODE = "siteMergeMode";
const QString C_MINLOTS = "MinLots";
const QString C_MINDATAPOINTS = "MinDatapoints";
const QString C_GROSSDIE = "GrossDie";
const QString C_THRESHOLD = "Threshold";
const QString C_DEFAULTALGO = "DefaultAlgo";
const QString C_VALIDITYPERIOD = "ValidityPeriod";
const QString C_EMAILFORMAT = "EmailFormat";
const QString C_EMAILFROM = "EmailFrom";
const QString C_EMAILTO = "EmailTo";
const QString C_N = "N";
const QString C_N1 = "N1";
const QString C_N2 = "N2";
const QString C_DATACLEANING = "DataCleaning";
const QString C_DAYSBEFOREEXPIRATION = "DaysBeforeExpiration";
const QString C_SENDEMAILBEFOREEXPIRATION = "SendEmailBeforeExpiration";
const QString C_AUTORECOMPUTE = "AutoRecompute";
const QString C_AUTORECOMPUTEMETHOD = "AutoRecomputeMethod";
const QString C_AUTORECOMPUTEPERIOD = "AutoRecomputePeriod";
const QString C_EXCEPTIONTYPE = "ExceptionType";
const QString C_VERSION_ID = "version_id";
const QString C_VERSION_DRAFT = "version_draft";
const QString C_VERSION_LABEL = "version_label";
const QString C_VERSION_PRODUCTS = "version_products";
const QString C_VERSION_CREATIONDATE = "version_creationDate";
const QString C_VERSION_STARTDATE = "version_startDate";
const QString C_VERSION_EXPIRATIONDATE = "version_expirationDate";
const QString C_VERSION_EXPIRATIONWARNINGDATE = "version_expirationWarningDate";
const QString C_VERSION_EXPIRATIONWARNINGDONE = "version_expirationWarningDone";
const QString C_VERSION_LASTEXEC = "version_LastExecutionDate";
const QString C_VERSION_COMPUTEFROM = "version_computeFrom";
const QString C_VERSION_COMPUTETO = "version_computeTo";
const QString C_VERSION_ACTIVE_PROD_ID = "version_active_prod_id";
const QString C_VERSION_ACTIVE_PROD_EXPIRATIONDATE = "version_active_prod_expirationDate";
const QString C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDATE = "version_active_prod_expirationWarningDate";
const QString C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDONE = "version_active_prod_expirationWarningDone";
const QString C_VERSION_LATEST_PROD_ID = "version_latest_prod_id";
const QString C_VERSION_LATEST_PROD_STARTDATE = "version_latest_prod_startDate";

class GexMoStatisticalMonitoringTaskData : public TaskProperties
{
public:
    GexMoStatisticalMonitoringTaskData(QObject* parent=NULL);
    virtual ~GexMoStatisticalMonitoringTaskData();

    GexMoStatisticalMonitoringTaskData& operator= (const GexMoStatisticalMonitoringTaskData& copy);

    void ResetDefault(const QString& aTestingStage);

    // overrides
    void UpdatePrivateAttributes();

    // Contains the filters defined through the Settings GUI
    // <TDR MetaData Name, valueRegExp>
    QMap<QString, QString> filtersMetaData;

private:

    void Clear();
};

#endif // STATISTICAL_MONITORING_TASKDATA_H
