#ifndef SYA_DIALOG_H
#define SYA_DIALOG_H

#include "statistical_monitoring_dialog.h"


// Forward declarations
class CGexMoTaskYieldMonitor;        // SYA_task.h

namespace GS
{
namespace Gex
{
class SYADraftWidget;

class SYADialog : public StatisticalMonitoringDialog
{
    Q_OBJECT

public:
    //! \brief Constructor
    explicit SYADialog(CGexMoTaskStatisticalMonitoring *task, QWidget *parent = 0, bool modal = false);
    //! \brief Destructor
    ~SYADialog();

    //! \brief Load dialog box fields with specified data structure
    bool LoadFields();

    void DisableTab();


private:

    //! \brief Initializes SYA task GUI
    bool InitializeSpecificUI();


    bool AnalyseDuplicatedTasks();
    bool    SaveSpecifTask();

    StatisticalMonitoringSettingsWidget*    createSettingsWidget();
    StatisticalMonitoringDraftWidget *      createDraftLimitsWidget();
    StatisticalMonitoringProdWidget*        createProdWidget();

};


} // namespace Gex
} // namespace GS

#endif // SYA_DIALOG_H
