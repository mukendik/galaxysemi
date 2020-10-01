
#include <QSortFilterProxyModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QMenu>

#include "statistical_monitoring_limits_widget.h"
#include "statistical_monitoring_alarms_widget.h"
#include "statistical_monitoring_draft_widget.h"
#include "statistical_monitoring_taskdata.h"
#include "statistical_monitoring_task.h"
#include "sm_legend_widget.h"
#include "limits_model.h"
#include "common_widgets/table_view.h"
#include "admin_engine.h"
#include "engine.h"
#include "common_widgets/collapsible_button.h"

namespace GS
{
namespace Gex
{

StatisticalMonitoringDraftWidget::StatisticalMonitoringDraftWidget(QWidget* parent)
    :SplittedWindowWidget(parent),
      mAlarmsWidget(0),
      mLimitsWidget(0),
      mLegend(0),
      mAlarmsArea(0),
      mLimitsArea(0),
      mAlarmsSummaryArea(0)
{
}

StatisticalMonitoringDraftWidget::~StatisticalMonitoringDraftWidget()
{
}

bool StatisticalMonitoringDraftWidget::IsLimitsModelEmpty()
{
    return mLimitsWidget->IsDataModelEmpty();
}

bool StatisticalMonitoringDraftWidget::DumpFields()
{
    return mLimitsWidget->DumpFields();
}

bool StatisticalMonitoringDraftWidget::LoadLimitsFromDatabase()
{
    return mLimitsWidget->LoadLimitsFromDatabase();
}

StatisticalMonitoringLimitsWidget *StatisticalMonitoringDraftWidget::
GetLimitsWidget()
{
    return mLimitsWidget;
}

StatisticalMonitoringAlarmsWidget *StatisticalMonitoringDraftWidget::
GetAlarmsWidget()
{
    return mAlarmsWidget;
}

void StatisticalMonitoringDraftWidget::CustomizeLeftBannerUI(QLayout *)
{
    // Limits area
    QFrame *lLimitsFrame = new QFrame();
    lLimitsFrame->setLayout(new QVBoxLayout());
    mLimitsWidget->CreateDataUI(lLimitsFrame->layout());
    mLimitsArea = CreateUnfoldWidgetInLeftBanner("Limits", lLimitsFrame);
    // Alarms area
    QFrame *lAlarmsFrame = new QFrame();
    lAlarmsFrame->setLayout(new QVBoxLayout());
    mAlarmsWidget->CreateDataUI(lAlarmsFrame->layout());
    mAlarmsArea = CreateUnfoldWidgetInLeftBanner("Simulate", lAlarmsFrame);
}

void StatisticalMonitoringDraftWidget::CustomizeRightBannerUI(QLayout* layout)
{
    // Send to prod button
    mLimitsWidget->CreateSendButton(layout);

    // Comment area
    QFrame *lComment = new QFrame();
    lComment->setLayout(new QVBoxLayout());
    mLimitsWidget->CreateCommentArea(lComment->layout());
    CreateUnfoldWidgetInRightBanner("Comment", lComment);

    // Legend area
    mLegend = new SMLegendWidget();
    mLegend->InitUI();
    CreateUnfoldWidgetInRightBanner("Legend", mLegend);

    // Summary area
    QFrame *lAlarmSumFrame = new QFrame();
    lAlarmSumFrame->setFrameShape(QFrame::NoFrame);
    lAlarmSumFrame->setContentsMargins(0,0,0,0);
    lAlarmSumFrame->setLayout(new QVBoxLayout());
    mAlarmsWidget->CreateSummaryWidget(lAlarmSumFrame->layout());
    mAlarmsSummaryArea = CreateUnfoldWidgetInRightBanner("Simulation summary",
                                                         lAlarmSumFrame);
}

void StatisticalMonitoringDraftWidget::UpdateSimulateResult(QMap<QString, QVariant> logSummary, QList<StatMonAlarm> alarms)
{
    mAlarmsWidget->UpdateSimulateResult(logSummary, alarms);

    mAlarmsArea->open();
    mAlarmsSummaryArea->open();
    LimitsModel* lModel = dynamic_cast<LimitsModel*>(mLimitsWidget->GetModel());
    bool lEnableButtons = lModel &&
            (lModel->rowCount() > 0) &&
            !lModel->hasNonManualLimitsToUpdate();
    mAlarmsWidget->RefreshUI(lEnableButtons);
}

} // namespace Gex
} // namespace GS
