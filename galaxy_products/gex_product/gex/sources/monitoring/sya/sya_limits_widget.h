#ifndef SPMLIMITSWIDGET_H
#define SPMLIMITSWIDGET_H


#include "statistical_monitoring_limits_widget.h"

class QSqlQuery;
class CGexMoTaskStatisticalMonitoring;

namespace GS
{
namespace Gex
{

class TableView;

class SYALimitsWidget : public StatisticalMonitoringLimitsWidget
{
public:
    /// \brief Constructor
    explicit SYALimitsWidget(CGexMoTaskStatisticalMonitoring *syaTask,
                             const QString backupFile,
                             QWidget* parent = 0);
    /// \brief Destructor
    virtual ~SYALimitsWidget();
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

#endif // SYALIMITSWIDGET_H
