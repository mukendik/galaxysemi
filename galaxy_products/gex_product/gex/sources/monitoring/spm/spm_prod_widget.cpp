
#include <QDir>
#include "gex_version.h"
#include "engine.h"
#include "spm_task.h"
#include "spm_limits_widget.h"
#include "spm_prod_widget.h"
#include "spm_version_widget.h"

namespace GS
{
namespace Gex
{

SPMProdWidget::SPMProdWidget(CGexMoTaskSPM *spmTask, QWidget *parent) :
    StatisticalMonitoringProdWidget(spmTask, parent)
{
}

SPMProdWidget::~SPMProdWidget()
{

}


StatisticalMonitoringLimitsWidget* SPMProdWidget::CreateLimitsWidget()
{
    return new SPMLimitsWidget(0, Engine::GetInstance().
                                  Get("TempFolder").toString() +
                                  QDir::separator() + "spm_limits_version_" +
                                  QString::number(GEX_APP_VERSION_MAJOR) + "." +
                                  QString::number(GEX_APP_VERSION_MINOR) + "." +
                                  QString::number(GEX_APP_VERSION_PATCH));
}

StatisticalMonitoringVersionWidget* SPMProdWidget::CreateVersionWidget()
{
    return new SPMVersionWidget(static_cast<CGexMoTaskSPM*>(mTask),
                                GS::Gex::Engine::GetInstance().
                                Get("TempFolder").toString() +
                                QDir::separator() + "spm_versions_" +
                                QString::number(GEX_APP_VERSION_MAJOR) + "." +
                                QString::number(GEX_APP_VERSION_MINOR) + "." +
                                QString::number(GEX_APP_VERSION_PATCH));
}


}
}
