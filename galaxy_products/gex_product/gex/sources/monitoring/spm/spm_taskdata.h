#ifndef SPM_TASKDATA_H
#define SPM_TASKDATA_H

#include <QDate>
#include <QObject>
#include "task_properties.h"
#include "gex_database_filter.h"

#define GEX_SPM_MERGE_NONE  "per_site"
#define GEX_SPM_MERGE_MERGE "merged_sites"
#define GEX_SPM_MERGE_BOTH  "both"

#define GEX_SPM_EXCEPTION_STANDARD  "standard"
#define GEX_SPM_EXCEPTION_CRITICAL  "critical"

#define GEXDB_PLUGIN_GALAXY_ETEST "E-Test"
#define GEXDB_PLUGIN_GALAXY_WTEST "Wafer Sort"
#define GEXDB_PLUGIN_GALAXY_FTEST "Final Test"


// properties keys
const QString C_SPM_TITLE = "Title";
const QString C_SPM_ADR = "ADR";
const QString C_SPM_TDR = "TDR";
const QString C_SPM_DATABASE = "Database";
const QString C_SPM_TESTINGSTAGE = "TestingStage";
const QString C_SPM_SPMID = "SpmId";
const QString C_SPM_TDRSPMID = "TdrSpmId";
const QString C_SPM_STATSTOMONITOR = "StatsToMonitor";
const QString C_SPM_ACTIVEONTRIGGER = "ActiveOnTrigger";
const QString C_SPM_ACTIVEONINSERT = "ActiveOnInsert";
const QString C_SPM_PRODUCTREGEXP = "ProductRegexp";
const QString C_SPM_TESTREGEXP = "TestRegexp";
const QString C_SPM_CONSOLIDATIONNAMEREGEXP = "ConsolidationNameRegexp";
const QString C_SPM_MINLOTS = "MinLots";
const QString C_SPM_MINDATAPOINTS = "MinDatapoints";
const QString C_SPM_DEFAULTALGO = "DefaultAlgo";
const QString C_SPM_VALIDITYPERIOD = "ValidityPeriod";
const QString C_SPM_EMAILFORMAT = "EmailFormat";
const QString C_SPM_EMAILFROM = "EmailFrom";
const QString C_SPM_EMAILTO = "EmailTo";
const QString C_SPM_N = "N";
const QString C_SPM_N1 = "N1";
const QString C_SPM_N2 = "N2";
const QString C_SPM_DAYSBEFOREEXPIRATION = "DaysBeforeExpiration";
const QString C_SPM_SENDEMAILBEFOREEXPIRATION = "SendEmailBeforeExpiration";
const QString C_SPM_AUTORECOMPUTE = "AutoRecompute";
const QString C_SPM_AUTORECOMPUTEMETHOD = "AutoRecomputeMethod";
const QString C_SPM_AUTORECOMPUTEPERIOD = "AutoRecomputePeriod";
const QString C_SPM_EXCEPTIONTYPE = "ExceptionType";
const QString C_SPM_VERSION_ID = "version_id";
const QString C_SPM_VERSION_DRAFT = "version_draft";
const QString C_SPM_VERSION_LABEL = "version_label";
const QString C_SPM_VERSION_PRODUCTS = "version_products";
const QString C_SPM_VERSION_SITEMERGEMODE = "version_siteMergeMode";
const QString C_SPM_VERSION_CREATIONDATE = "version_creationDate";
const QString C_SPM_VERSION_STARTDATE = "version_startDate";
const QString C_SPM_VERSION_EXPIRATIONDATE = "version_expirationDate";
const QString C_SPM_VERSION_EXPIRATIONWARNINGDATE = "version_expirationWarningDate";
const QString C_SPM_VERSION_EXPIRATIONWARNINGDONE = "version_expirationWarningDone";
const QString C_SPM_VERSION_LASTEXEC = "version_LastExecutionDate";
const QString C_SPM_VERSION_COMPUTEFROM = "version_computeFrom";
const QString C_SPM_VERSION_COMPUTETO = "version_computeTo";
const QString C_SPM_VERSION_ACTIVE_PROD_ID = "version_active_prod_id";
const QString C_SPM_VERSION_ACTIVE_PROD_EXPIRATIONDATE = "version_active_prod_expirationDate";
const QString C_SPM_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDATE = "version_active_prod_expirationWarningDate";
const QString C_SPM_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDONE = "version_active_prod_expirationWarningDone";
const QString C_SPM_VERSION_LATEST_PROD_ID = "version_latest_prod_id";
const QString C_SPM_VERSION_LATEST_PROD_STARTDATE = "version_latest_prod_startDate";

class GexMoSPMTaskData : public TaskProperties
{
public:
    explicit GexMoSPMTaskData(QObject* parent=NULL);
    virtual ~GexMoSPMTaskData();

    GexMoSPMTaskData& operator= (const GexMoSPMTaskData& copy);

    // overrides
    void UpdatePrivateAttributes();

    // Contains the filters defined through the Settings GUI
    QMap<QString, QString> filters;

private:
    void Clear();

};

#endif // SPM_TASKDATA_H
