#ifndef SYA_PROD_WIDGET_H
#define SYA_PROD_WIDGET_H

#include "sya_task.h"
#include "statistical_monitoring_prod_widget.h"
#include "statistical_monitoring_version_widget.h"

namespace GS {
namespace Gex {

class SYAProdWidget : public StatisticalMonitoringProdWidget
{
    Q_OBJECT

public:
    /// \brief Constructor
    explicit SYAProdWidget(CGexMoTaskSYA *syaTask, QWidget *parent = 0);
    /// \brief Destructor
    ~SYAProdWidget();


private :
    /// \brief create a sya LimisWidget
    StatisticalMonitoringLimitsWidget* CreateLimitsWidget();

    /// \ brief create a sya version widget
    StatisticalMonitoringVersionWidget* CreateVersionWidget();
};

} // namespace Gex
} // namespace GS

#endif // SPM_PROD_WIDGET_H
