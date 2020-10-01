#include "spm_task.h"
#include "spm_version_widget.h"

namespace GS {
namespace Gex {


SPMVersionWidget::SPMVersionWidget(CGexMoTaskSPM *spmTask, const QString& backupFile, QWidget *parent) :
    StatisticalMonitoringVersionWidget(spmTask, backupFile, parent)

{
}

SPMVersionWidget::~SPMVersionWidget()
{
}

}
}
