#ifndef STATISTICALMONITORINGALARMSWIDGET_H
#define STATISTICALMONITORINGALARMSWIDGET_H

#include <QStringList>
#include <QWidget>

#include "statistical_monitoring_alarm_struct.h"

class CGexMoTaskStatisticalMonitoring;
class QLayout;
class QTextEdit;
class QDateEdit;
class QPushButton;

namespace GS
{
namespace Gex
{

class FilterableTableView;
class TableModel;

class StatisticalMonitoringAlarmsWidget : public QWidget
{
    Q_OBJECT
public:
    /// \brief Constructor
    explicit StatisticalMonitoringAlarmsWidget(CGexMoTaskStatisticalMonitoring *spmTask,
                                      const QString&backupFile,
                                      QWidget *parent = 0);
    /// \brief Destructor
    ~StatisticalMonitoringAlarmsWidget();
    /// \brief Create Table view and return it
    virtual QWidget*    CreateTableView();
    /// \brief Refresh view and buttons
    void RefreshUI(bool enableUpdateButtons);
    /// \brief Create table view and filters UI
    void CreateDataUI(QLayout *target);
    /// \brief summary widget
    void CreateSummaryWidget(QLayout *target);

    void UpdateSimulateResult(QMap<QString, QVariant> logSummary, QList<StatMonAlarm> alarms);

signals:
    void SimulateClicked(const QDateTime& dateFrom, const QDateTime& dateTo);

public slots:
    /// \brief run simulation and update UI
    void Simulate();

private:
    /// \brief Load alarms model with data from alarm object
    virtual bool LoadAlarmsModel(const QList<StatMonAlarm> &lAlarms);
    /// \brief Update table view
    void UpdateLimitsTableView();
    /// \brief return default column to sort
    virtual int GetDefaultColumnToSort() const;

protected:
    FilterableTableView*                   mAlarmsView;            ///< Holds ptr alarms table view
    CGexMoTaskStatisticalMonitoring*       mTask;                  ///< Holds ptr to SPM task
    TableModel*                            mModel;                 ///< Holds ptr to model
    QTextEdit*                             mSimulateSummary;       ///< Holds ptr to simulate summary
    QDateEdit*                             mDateSimulateFrom;      ///< Holds ptr to From date of simulate
    QDateEdit*                             mDateSimulateTo;        ///< Holds ptr to To date of simulate
    QStringList                            mShownFields;           ///< Holds shown fields in the model
    QPushButton*                           mRunSimulate;
    QStringList                            mHiddenFields;          ///< Holds hidden fields in the model
};

} // namespace Gex
} // namespace GS

#endif // STATISTICALMONITORINGALARMSWIDGET_H
