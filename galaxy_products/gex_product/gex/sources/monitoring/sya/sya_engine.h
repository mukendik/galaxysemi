#ifndef SYAENGINE_H
#define SYAENGINE_H

#include "statisticalMonitoring/statistical_monitoring_engine.h"

class CGexMoTaskSYA;

namespace GS
{
    namespace Gex
    {
        ///
        /// \brief The SYAEngine class provides high-level access to basic SYA features
        ///
        class SYAEngine : public StatisticalMonitoringEngine
        {
        public:
            friend class EnginePrivate;

            SYAEngine();

        public:
            const QMap<QString, QString>& GetSupportedMonitoredItemTypes() const;
            const QMap<QString, QString>& GetSupportedStats() const;

            ///
            /// \brief Creates a new SPM task instance and the draft of its first version
            /// then automatically saves it
            /// \param The task recipicent emails
            /// \return a pointer to the created task, NULL if failed
            ///
            CGexMoTaskSYA* CreateSYATask(const QString &Email);

        private:
            int GetSupportedTaskType();
        };
    }
}

#endif // SYAENGINE_H
