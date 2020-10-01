
#include <QDir>
#include "gex_version.h"
#include "engine.h"
#include "sya_task.h"
#include "sya_limits_widget.h"
#include "sya_prod_widget.h"
#include "sya_version_widget.h"

namespace GS
{
namespace Gex
{

SYAProdWidget::SYAProdWidget(CGexMoTaskSYA *syaTask, QWidget *parent) :
    StatisticalMonitoringProdWidget(syaTask, parent)
{
}

SYAProdWidget::~SYAProdWidget()
{

}


StatisticalMonitoringLimitsWidget* SYAProdWidget::CreateLimitsWidget()
{
    return  new SYALimitsWidget(0, Engine::GetInstance().
                                   Get("TempFolder").toString() +
                                   QDir::separator() + "sya_limits_version_" +
                                   QString::number(GEX_APP_VERSION_MAJOR) + "." +
                                   QString::number(GEX_APP_VERSION_MINOR) + "." +
                                   QString::number(GEX_APP_VERSION_PATCH));
}

StatisticalMonitoringVersionWidget* SYAProdWidget::CreateVersionWidget()
{
    return new SYAVersionWidget(static_cast<CGexMoTaskSYA*>(mTask), GS::Gex::Engine::GetInstance().
                                        Get("TempFolder").toString() +
                                        QDir::separator() + "sya_versions_" +
                                        QString::number(GEX_APP_VERSION_MAJOR) + "." +
                                        QString::number(GEX_APP_VERSION_MINOR) + "." +
                                        QString::number(GEX_APP_VERSION_PATCH));
}


}
}
