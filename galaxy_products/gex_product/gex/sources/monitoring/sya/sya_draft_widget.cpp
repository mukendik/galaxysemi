
#include <QWidget>

#include <QPushButton>

#include "statistical_monitoring_alarms_widget.h"
#include "sya_limits_widget.h"
#include "sya_draft_widget.h"
#include "common_widgets/collapsible_button.h"
#include "sya_task.h"
#include "engine.h"
#include "gex_version.h"

namespace GS
{
namespace Gex
{

SYADraftWidget::SYADraftWidget(CGexMoTaskSYA *syaTask, QWidget *parent) :
    StatisticalMonitoringDraftWidget(parent)
{
    mLimitsWidget = new SYALimitsWidget(syaTask, Engine::GetInstance().
                                        Get("TempFolder").toString() +
                                        QDir::separator() + "sya_draft_limits_" +
                                        QString::number(GEX_APP_VERSION_MAJOR) + "." +
                                        QString::number(GEX_APP_VERSION_MINOR) + "." +
                                        QString::number(GEX_APP_VERSION_PATCH),
                                        parent);
    mAlarmsWidget = new StatisticalMonitoringAlarmsWidget(syaTask, Engine::GetInstance().
                                        Get("TempFolder").toString() +
                                        QDir::separator() + "sya_draft_alarms_" +
                                        QString::number(GEX_APP_VERSION_MAJOR) + "." +
                                        QString::number(GEX_APP_VERSION_MINOR) + "." +
                                        QString::number(GEX_APP_VERSION_PATCH),
                                        parent);
}

SYADraftWidget::~SYADraftWidget()
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
