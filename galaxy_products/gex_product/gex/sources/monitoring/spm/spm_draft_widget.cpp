
#include <QSortFilterProxyModel>
#include <QInputDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QMenu>

#include "statistical_monitoring_alarms_widget.h"
#include "statistical_monitoring_taskdata.h"
#include "spm_draft_widget.h"
#include "common_widgets/filterable_table_view.h"
#include "sm_legend_widget.h"
#include "spm_limits_widget.h"
#include "common_widgets/collapsible_button.h"
#include "limits_model.h"
#include "common_widgets/table_view.h"
#include "spm_task.h"
#include "engine.h"
#include "gex_version.h"

namespace GS
{
namespace Gex
{

SPMDraftWidget::SPMDraftWidget(CGexMoTaskSPM *spmTask, QWidget *parent) :
    StatisticalMonitoringDraftWidget(parent)
{
    mLimitsWidget = new SPMLimitsWidget(spmTask, Engine::GetInstance().
                                        Get("TempFolder").toString() +
                                        QDir::separator() + "spm_draft_limits_" +
                                        QString::number(GEX_APP_VERSION_MAJOR) + "." +
                                        QString::number(GEX_APP_VERSION_MINOR) + "." +
                                        QString::number(GEX_APP_VERSION_PATCH),
                                        parent);
    mAlarmsWidget = new StatisticalMonitoringAlarmsWidget(spmTask, Engine::GetInstance().
                                        Get("TempFolder").toString() +
                                        QDir::separator() + "spm_draft_alarms_" +
                                        QString::number(GEX_APP_VERSION_MAJOR) + "." +
                                        QString::number(GEX_APP_VERSION_MINOR) + "." +
                                        QString::number(GEX_APP_VERSION_PATCH),
                                        parent);
}

SPMDraftWidget::~SPMDraftWidget()
{
    // -- if parent == 0, means thant this is an unpin window
    // --  and that we need to delete it manually
    if(mAlarmsArea && mAlarmsArea->parent() == 0)
    {
       mAlarmsArea->hide();
       delete mAlarmsArea;
    }

    // -- if unpin we need to delete it manually
    if(mLimitsArea && mLimitsArea->parent() == 0)
    {
       mLimitsArea->hide();
       delete mLimitsArea;
    }
}

} // namespace Gex
} // namespace GS
