#ifndef SATISTICAL_MONITORING_DRAFT_WIDGET_H
#define SATISTICAL_MONITORING_DRAFT_WIDGET_H

#include "common_widgets/split_widget.h"

struct StatMonAlarm;

namespace GS
{
namespace Gex
{

class StatisticalMonitoringLimitsWidget;
class StatisticalMonitoringAlarmsWidget;
class SMLegendWidget;

/**
 * \brief The SettingsWidget class provides a widget for letting the user customize the settings of a task
 */
class StatisticalMonitoringDraftWidget : public SplittedWindowWidget
{
    Q_OBJECT
public:
    /// \brief Constructor
    StatisticalMonitoringDraftWidget(QWidget* parent);
    /// \brief Destructor
    virtual ~StatisticalMonitoringDraftWidget();
    /// \brief check if datamodel is empty
    bool IsLimitsModelEmpty();
    /// \brief Dump the user defined fields into the SYA task
    bool DumpFields();
    /// \brief Load limits from DB
    bool LoadLimitsFromDatabase();
    /// \brief return ptr to limits widget
    StatisticalMonitoringLimitsWidget* GetLimitsWidget();
    StatisticalMonitoringAlarmsWidget* GetAlarmsWidget();

    /// \brief When simulate is requested on alarms set
    void UpdateSimulateResult(QMap<QString, QVariant> logSummary, QList<StatMonAlarm> alarms);

private:
    /// \brief create widget for the left banner
    void CustomizeLeftBannerUI(QLayout *);
    /// \brief Init specialized UI
    void CustomizeRightBannerUI(QLayout* layout);

protected:
    StatisticalMonitoringAlarmsWidget*  mAlarmsWidget;      ///< Holds ptr to alarms widget
    StatisticalMonitoringLimitsWidget*  mLimitsWidget;      ///< Holds prt to limit widget
    SMLegendWidget*                     mLegend;            ///< Holds ptr to the legend of data's color
    CollapsibleButton*                       mLimitsArea;       ///< Holds ptr to limits table area
    CollapsibleButton*                       mAlarmsArea;        ///< Holds ptr to alarm table area
    CollapsibleButton*                       mAlarmsSummaryArea; ///< Holds ptr to alarm table summary area
};

} // namespace Gex
} // namespace GS

#endif // SATISTICAL_MONITORING_DRAFT_WIDGET_H
