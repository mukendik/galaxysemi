#include "statistical_monitoring_taskdata.h"
#include "gex_shared.h"
#include "engine.h"
#include "gexdb_plugin_base.h"

GexMoStatisticalMonitoringTaskData::GexMoStatisticalMonitoringTaskData(QObject *parent)
    : TaskProperties(parent)
{
    Clear();
}

GexMoStatisticalMonitoringTaskData::~GexMoStatisticalMonitoringTaskData()
{
}

GexMoStatisticalMonitoringTaskData& GexMoStatisticalMonitoringTaskData::operator=(const GexMoStatisticalMonitoringTaskData& copy)
{
    if(this != &copy)
    {
        TaskProperties::operator =(copy);

        // Duplicate the filters
        filtersMetaData = copy.filtersMetaData;
    }
    return *this;
}

void GexMoStatisticalMonitoringTaskData::ResetDefault(const QString &aTestingStage)
{
    if (aTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        SetPrivateAttribute(C_N1, 6);
        SetPrivateAttribute(C_N2, 8);
        SetPrivateAttribute(C_MONITOREDITEMTYPE, "S");
        SetPrivateAttribute(C_MINDATAPOINTS, 120);
        SetPrivateAttribute(C_MINLOTS, 8);
        SetPrivateAttribute(C_DEFAULTALGO, C_OUTLIERRULE_MEAN_N_SIGMA);
    }
    else if (aTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
    {
        SetPrivateAttribute(C_N1, 3);
        SetPrivateAttribute(C_N2, 4);
        SetPrivateAttribute(C_MONITOREDITEMTYPE, "H");
        SetPrivateAttribute(C_MINDATAPOINTS, 8);
        SetPrivateAttribute(C_MINLOTS, 8);
        SetPrivateAttribute(C_DEFAULTALGO, C_OUTLIERRULE_MEDIAN_N_ROBUSTSIGMA);
    }
    else
    {
        SetPrivateAttribute(C_N1, 4);
        SetPrivateAttribute(C_N2, 6);
        SetPrivateAttribute(C_MONITOREDITEMTYPE, "H");
        SetPrivateAttribute(C_MINDATAPOINTS, 30);
        SetPrivateAttribute(C_MINLOTS, 8);
        SetPrivateAttribute(C_DEFAULTALGO, C_OUTLIERRULE_MEAN_N_SIGMA);
    }

    SetPrivateAttribute(C_TESTINGSTAGE, aTestingStage);
}

void GexMoStatisticalMonitoringTaskData::UpdatePrivateAttributes()
{

}

void GexMoStatisticalMonitoringTaskData::Clear()
{
    ResetPrivateAttributes();

    QDateTime serverDateTime = GS::Gex::Engine::GetInstance().GetServerDateTime();

    SetPrivateAttribute(C_TITLE, "New Statistical Monitoring task");
    SetPrivateAttribute(C_DATABASE, "");
    SetPrivateAttribute(C_TESTINGSTAGE, GEXDB_PLUGIN_GALAXY_WTEST);
    SetPrivateAttribute(C_TASKID, -1);
    SetPrivateAttribute(C_ACTIVEONTRIGGER, true);
    SetPrivateAttribute(C_ACTIVEONINSERT, true);
    SetPrivateAttribute(C_PRODUCTREGEXP, "");
    SetPrivateAttribute(C_MONITOREDITEMREGEXP, "");
    SetPrivateAttribute(C_N1, 6);
    SetPrivateAttribute(C_N2, 8);
    SetPrivateAttribute(C_MONITOREDITEMTYPE, "S");
    SetPrivateAttribute(C_MINDATAPOINTS, 120);
    SetPrivateAttribute(C_MINLOTS, 8);
    SetPrivateAttribute(C_DEFAULTALGO, C_OUTLIERRULE_MEAN_N_SIGMA);
    SetPrivateAttribute(C_TESTFLOW, "");
    SetPrivateAttribute(C_CONSOLIDATION_TYPE, GEXDB_CONSO_TYPE_CONSO);
    SetPrivateAttribute(C_CONSOLIDATION_LEVEL, GEXDB_CONSO_LEVEL_INSERT);
    SetPrivateAttribute(C_INSERTION, "");
    SetPrivateAttribute(C_SITEMERGEMODE, GEX_MERGE_BOTH);
    SetPrivateAttribute(C_STATSTOMONITOR, "");
    SetPrivateAttribute(C_DATACLEANING, true);
    SetPrivateAttribute(C_VALIDITYPERIOD, 180);
    SetPrivateAttribute(C_DAYSBEFOREEXPIRATION, 7);
    SetPrivateAttribute(C_SENDEMAILBEFOREEXPIRATION, true);
    SetPrivateAttribute(C_THRESHOLD, 0);
    SetPrivateAttribute(C_AUTORECOMPUTE, true);
    SetPrivateAttribute(C_AUTORECOMPUTEMETHOD, "duplicateIfRecomputeFailed");
    SetPrivateAttribute(C_AUTORECOMPUTEPERIOD, 180);
    SetPrivateAttribute(C_EMAILFORMAT, "plain_text");
    SetPrivateAttribute(C_EMAILFROM, GEX_EMAIL_NOREPLY);
    SetPrivateAttribute(C_EMAILTO, "");
    SetPrivateAttribute(C_EXCEPTIONTYPE, GEX_EXCEPTION_STANDARD);
    SetPrivateAttribute(C_VERSION_ID, -1);
    SetPrivateAttribute(C_VERSION_DRAFT, true);
    SetPrivateAttribute(C_VERSION_LABEL, "");
    SetPrivateAttribute(C_VERSION_PRODUCTS, "");
    SetPrivateAttribute(C_VERSION_CREATIONDATE, serverDateTime);
    SetPrivateAttribute(C_VERSION_STARTDATE, serverDateTime);
    SetPrivateAttribute(C_VERSION_EXPIRATIONDATE, serverDateTime.addDays(30));
    SetPrivateAttribute(C_VERSION_EXPIRATIONWARNINGDATE, serverDateTime.addDays(23));
    SetPrivateAttribute(C_VERSION_EXPIRATIONWARNINGDONE, 0);
    SetPrivateAttribute(C_VERSION_LASTEXEC, serverDateTime.addYears(-10));
    SetPrivateAttribute(C_VERSION_COMPUTEFROM, serverDateTime.addDays(-180));
    SetPrivateAttribute(C_VERSION_COMPUTETO, serverDateTime);
    SetPrivateAttribute(C_VERSION_ACTIVE_PROD_ID, -1);
    SetPrivateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONDATE, serverDateTime.addDays(30));
    SetPrivateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDATE, serverDateTime.addDays(23));
    SetPrivateAttribute(C_VERSION_ACTIVE_PROD_EXPIRATIONWARNINGDONE, 0);
    SetPrivateAttribute(C_VERSION_LATEST_PROD_ID, -1);
    SetPrivateAttribute(C_VERSION_LATEST_PROD_STARTDATE, serverDateTime);

    ResetDefault(GEXDB_PLUGIN_GALAXY_WTEST);

    filtersMetaData.clear();
}

