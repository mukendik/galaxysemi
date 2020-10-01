#ifndef STATISTICAL_MONITORING_ENGINE_H
#define STATISTICAL_MONITORING_ENGINE_H

#include <QStringList>
#include "statistical_monitoring_task.h"

namespace GS
{
    namespace Gex
    {
        class StatisticalMonitoringEngine
        {
        public:
            StatisticalMonitoringEngine();

            const QMap<QString, QString>& GetSupportedOutlierAlgorithms() const;
            virtual const QMap<QString, QString>& GetSupportedMonitoredItemTypes() const = 0;
            virtual const QMap<QString, QString>& GetSupportedStats() const = 0;

            ///
            /// \brief Deletes the specified statistical monitoring task instance
            /// and removes it from the ym_admin_db
            /// \param the task to delete
            /// \return true if succeeded, false otherwise
            ///
            bool DeleteTask(CGexMoTaskStatisticalMonitoring* task);

            ///
            /// \brief Saves the specified SPM task instance into the ym_admin_db
            /// \param the task to save
            /// \return true if succeeded, false otherwise
            ///
            bool SaveTask(CGexMoTaskStatisticalMonitoring* task);

            QPair<DuplicateStatus, CGexMoTaskStatisticalMonitoring*> FindDuplicate(CGexMoTaskStatisticalMonitoring* aTask);

        private:
            virtual int GetSupportedTaskType() = 0;

            QMap<QString, QString> algorithms;
        protected:
            QMap<QString, QString> itemTypes;
            QMap<QString, QString> stats;
        };
    }
}

#endif // STATISTICAL_MONITORING_ENGINE_H
