#ifndef SPMLIMITSWIDGET_H
#define SPMLIMITSWIDGET_H

class QSqlQuery;

#include "statistical_monitoring_limits_widget.h"

namespace GS
{
namespace Gex
{

class SPMLimitsWidget : public StatisticalMonitoringLimitsWidget
{
public:
    /// \brief Constructor
    explicit SPMLimitsWidget(CGexMoTaskStatisticalMonitoring *spmTask,
                             const QString& backupFile,
                             QWidget* parent = 0);
    /// \brief Destructor
    virtual ~SPMLimitsWidget();
    /// \brief Create Table view and return it
    QWidget *CreateTableView();

protected:
    /// \brief Return index of default column to sort
    int GetDefaultColumnToSort() const;
    /// \brief Customize the limit view with an item delegate to generate combo while editing algo
    /// return the list of supported outlier algorithm
    QMap<QString, QString> AddItemDelegateOutlierAlgo();
    /// \brief Update specific table view
    void UpdateSpecificsTableView();
};


} // namespace Gex
} // namespace GS

#endif // SPMLIMITSWIDGET_H
