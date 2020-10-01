#include "statistical_monitoring_engine.h"
#include "gexdb_plugin_base.h"
#include "engine.h"
#include "scheduler_engine.h"

namespace GS
{
namespace Gex
{
StatisticalMonitoringEngine::StatisticalMonitoringEngine()
{
    algorithms.insert(C_OUTLIERRULE_MEAN_N_SIGMA, C_OUTLIERRULE_MEAN_N_SIGMA_D);
    algorithms.insert(C_OUTLIERRULE_MEDIAN_N_ROBUSTSIGMA, C_OUTLIERRULE_MEDIAN_N_ROBUSTSIGMA_D);
    algorithms.insert(C_OUTLIERRULE_MEDIAN_N_IQR, C_OUTLIERRULE_MEDIAN_N_IQR_D);
    algorithms.insert(C_OUTLIERRULE_PERCENTILE_N, C_OUTLIERRULE_PERCENTILE_N_D);
    algorithms.insert(C_OUTLIERRULE_MANUAL, C_OUTLIERRULE_MANUAL_D);
}

const QMap<QString, QString>& StatisticalMonitoringEngine::GetSupportedOutlierAlgorithms() const
{
    return algorithms;
}

bool StatisticalMonitoringEngine::DeleteTask(CGexMoTaskStatisticalMonitoring* task)
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().DeleteDbTask(task);
}

// Saves the specified SPM task into both the ym_admin_db and tdr databases
bool StatisticalMonitoringEngine::SaveTask(CGexMoTaskStatisticalMonitoring* task)
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveDbTasks(task);
}

QPair<DuplicateStatus, CGexMoTaskStatisticalMonitoring *> StatisticalMonitoringEngine::FindDuplicate(CGexMoTaskStatisticalMonitoring *aTask)
{
    QPair<DuplicateStatus, CGexMoTaskStatisticalMonitoring *> lDuplicate;
    lDuplicate.first = NOTDUPLICATED;
    lDuplicate.second = NULL;
    if(!aTask) return lDuplicate;

    // Go through task list
    QListIterator<CGexMoTaskItem*>  lTaskIt(GS::Gex::Engine::GetInstance().GetSchedulerEngine().getTasksList());
    CGexMoTaskItem*                  lCurTask = NULL;
    CGexMoTaskStatisticalMonitoring* lSmTask = NULL;
    DuplicateStatus                  lDuplicateStatus = NOTDUPLICATED;


    while(lTaskIt.hasNext())
    {
        // Get task
        lCurTask = lTaskIt.next();

        // Make sure this is a SPM/SYA task
        if(lCurTask->GetTaskType() != GetSupportedTaskType())
        {
            continue;
        }

        // The provided task is not a duplicate of itself...
        if(lCurTask->GetID() == aTask->GetID())
        {
            continue;
        }

        // Now we have an SPM task, cast it
        lSmTask = dynamic_cast<CGexMoTaskStatisticalMonitoring*> (lCurTask);

        // Check if duplicates
        lDuplicateStatus = lSmTask->GetDuplicatedParam(*aTask);
        if(lDuplicateStatus != NOTDUPLICATED)
        {
            lDuplicate.first = lDuplicateStatus;
            lDuplicate.second = lSmTask;
            return lDuplicate;
        }
    }
    return lDuplicate;
}
}
}
