#include "sya_task.h"
#include "sya_version_widget.h"

namespace GS {
namespace Gex {


SYAVersionWidget::SYAVersionWidget(CGexMoTaskSYA *syaTask, const QString& backupFile, QWidget *parent) :
    StatisticalMonitoringVersionWidget(syaTask, backupFile, parent)

{
}

SYAVersionWidget::~SYAVersionWidget()
{
}

}
}
