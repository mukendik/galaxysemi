#ifndef SPM_PROD_WIDGET_H
#define SPM_PROD_WIDGET_H

#include "statistical_monitoring_prod_widget.h"

class CGexMoTaskSPM;

namespace GS
{
namespace Gex
{

class SPMProdWidget : public StatisticalMonitoringProdWidget
{
    Q_OBJECT

public:
    /// \brief Constructor
    explicit SPMProdWidget(CGexMoTaskSPM *spmTask, QWidget *parent = 0);
    /// \brief Destructor
    ~SPMProdWidget();

private :
    /// \brief create a spm LimisWidget
    StatisticalMonitoringLimitsWidget* CreateLimitsWidget();

    /// \ brief create a spm version widget
    StatisticalMonitoringVersionWidget* CreateVersionWidget();
};

} // namespace Gex
} // namespace GS

#endif // SPM_PROD_WIDGET_H
