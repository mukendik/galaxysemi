#ifndef SPM_DRAFT_WIDGET_H
#define SPM_DRAFT_WIDGET_H

#include "statistical_monitoring_draft_widget.h"

class CGexMoTaskSPM;

namespace GS
{
namespace Gex
{

class SPMDraftWidget : public StatisticalMonitoringDraftWidget
{
    Q_OBJECT

public:
     /// \brief Constructor
     SPMDraftWidget(CGexMoTaskSPM *spmTask = 0, QWidget *parent = 0);
     /// \brief Destructor
    ~SPMDraftWidget();
};

} // namespace Gex
} // namespace GS

#endif // SPM_DRAFT_WIDGET_H
