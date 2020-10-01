#ifndef SPM_VERSION_WIDGET_H
#define SPM_VERSION_WIDGET_H

#include "statistical_monitoring_version_widget.h"

class CGexMoTaskSPM;

namespace GS {
namespace Gex {

class SPMVersionWidget : public StatisticalMonitoringVersionWidget
{
    Q_OBJECT
public:
    explicit SPMVersionWidget(CGexMoTaskSPM *spmTask,
                              const QString &backupFile,
                              QWidget *parent = 0);
    ~SPMVersionWidget();
};

} // namespace Gex
} // namespace GS

#endif // SPM_VERSION_WIDGET_H
