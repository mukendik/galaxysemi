#ifndef SPM_DIALOG_H
#define SPM_DIALOG_H

#include "statisticalMonitoring/statistical_monitoring_dialog.h"


// Forward declarations
class CGexMoTaskSPM;        // spm_task.h

namespace GS
{
namespace Gex
{

class SPMDialog : public StatisticalMonitoringDialog
{
    Q_OBJECT

public:
    //! \brief Constructor
    explicit SPMDialog(CGexMoTaskSPM  *spmTask, QWidget *parent = 0, bool modal = false);
    //! \brief Destructor
    ~SPMDialog();

    //! \brief Load dialog box fields with specified data structure
    bool LoadFields();


private:

    //! \brief Initializes SPM task GUI
    bool InitializeSpecificUI();

    //! \brief disable if needed tabs
    void DisableTab(); // to remove whn refacto finished

    bool AnalyseDuplicatedTasks();
    bool    SaveSpecifTask();

    StatisticalMonitoringSettingsWidget*    createSettingsWidget();
    StatisticalMonitoringDraftWidget*       createDraftLimitsWidget();
    StatisticalMonitoringProdWidget*        createProdWidget();
};


} // namespace Gex
} // namespace GS

#endif // SPM_DIALOG_H
