#include "sya_engine.h"

#include "sya_task.h"
#include "statisticalMonitoring/statistical_monitoring_taskdata.h"

#include "gexdb_plugin_base.h"
#include "gexmo_constants.h"

namespace GS
{
    namespace Gex
    {
        SYAEngine::SYAEngine()
        {
            itemTypes.insert("H", "Hard");
            itemTypes.insert("S", "Soft");

            stats.insert(C_STATS_RATIO, C_STATS_RATIO_D);
        }

        const QMap<QString, QString>& SYAEngine::GetSupportedMonitoredItemTypes() const
        {
            return itemTypes;
        }

        const QMap<QString, QString>& SYAEngine::GetSupportedStats() const
        {
            return stats;
        }

        CGexMoTaskSYA* SYAEngine::CreateSYATask(const QString & Email)
        {
            return new CGexMoTaskSYA(Email);
        }

        int SYAEngine::GetSupportedTaskType()
        {
            return GEXMO_TASK_SYA;
        }
    }
}
