#ifndef SPM_ENGINE_H
#define SPM_ENGINE_H

#include "statisticalMonitoring/statistical_monitoring_engine.h"

class CGexMoTaskSPM;

namespace GS
{
    namespace Gex
    {
        ///
        /// \brief The SPMEngine class provides high-level access to basic SPM features
        ///
        class SPMEngine : public StatisticalMonitoringEngine
        {
            friend class EnginePrivate;

            SPMEngine();

        public:
            const QMap<QString, QString>& GetSupportedMonitoredItemTypes() const;
            const QMap<QString, QString>& GetSupportedStats() const;

            ///
            /// \brief Creates a new SPM task instance and the draft of its first version
            /// then automatically saves it
            /// \param The task recipicent emails
            /// \return a pointer to the created task, NULL if failed
            ///
            CGexMoTaskSPM* CreateSPMTask(const QString &Email);

        private:
            int GetSupportedTaskType();
        };
    }
}

#endif // SPM_ENGINE_H
