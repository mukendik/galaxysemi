#include "spm_engine.h"

#include "spm_task.h"
#include "statisticalMonitoring/statistical_monitoring_taskdata.h"

#include "gexdb_plugin_base.h"
#include "gexmo_constants.h"

namespace GS
{
    namespace Gex
    {
        SPMEngine::SPMEngine()
        {
            itemTypes.insert("P", "Parametric");
            // Force to use only Parametric
            // itemTypes.insert("M", "Multi-parametric");

            stats.insert(C_STATS_MEAN, C_STATS_MEAN_D);
            stats.insert(C_STATS_P0, C_STATS_P0_D);
            stats.insert(C_STATS_P25, C_STATS_P25_D);
            stats.insert(C_STATS_P50, C_STATS_P50_D);
            stats.insert(C_STATS_P75, C_STATS_P75_D);
            stats.insert(C_STATS_P100, C_STATS_P100_D);
            stats.insert(C_STATS_RANGE, C_STATS_RANGE_D);
            stats.insert(C_STATS_CP, C_STATS_CP_D);
            stats.insert(C_STATS_CPK, C_STATS_CPK_D);
            stats.insert(C_STATS_CR, C_STATS_CR_D);
            stats.insert(C_STATS_SIGMA, C_STATS_SIGMA_D);
            stats.insert(C_STATS_YIELD, C_STATS_YIELD_D);
            stats.insert(C_STATS_FAIL_COUNT, C_STATS_FAIL_COUNT_D);
            stats.insert(C_STATS_EXEC_COUNT, C_STATS_EXEC_COUNT_D);
        }

        const QMap<QString, QString>& SPMEngine::GetSupportedMonitoredItemTypes() const
        {
            return itemTypes;
        }

        const QMap<QString, QString>& SPMEngine::GetSupportedStats() const
        {
            return stats;
        }

        CGexMoTaskSPM* SPMEngine::CreateSPMTask(const QString & Email)
        {
            return new CGexMoTaskSPM(Email);
        }

        int SPMEngine::GetSupportedTaskType()
        {
            return GEXMO_TASK_SPM;
        }
    }
}
