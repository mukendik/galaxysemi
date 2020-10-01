#ifndef SYA_DRAFT_LIMITS_WIDGET_H
#define SYA_DRAFT_LIMITS_WIDGET_H

#include "statistical_monitoring_draft_widget.h"

class CGexMoTaskSYA;

namespace GS
{
namespace Gex
{

class SYADraftWidget : public StatisticalMonitoringDraftWidget
{
    Q_OBJECT

public:
     /// \brief Constructor
     SYADraftWidget(CGexMoTaskSYA *syaTask = 0, QWidget *parent = 0);
     /// \brief Destructor
    ~SYADraftWidget();
};

} // namespace Gex
} // namespace GS

#endif // SPM_DRAFT_LIMITS_WIDGET_H
