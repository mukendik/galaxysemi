#ifndef SYA_VERSION_WIDGET_H
#define SYA_VERSION_WIDGET_H

#include "statistical_monitoring_version_widget.h"

class CGexMoTaskSYA;

namespace GS {
namespace Gex {

class SYAVersionWidget : public StatisticalMonitoringVersionWidget
{
    Q_OBJECT
public:
    explicit SYAVersionWidget(CGexMoTaskSYA *syaTask, const QString &backupFile, QWidget *parent = 0);
    ~SYAVersionWidget();
};

}
}

#endif // SYA_VERSION_WIDGET_H
