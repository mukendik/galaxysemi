
#include "statistical_monitoring_taskdata.h"
#include "spm_settings_widget.h"
#include "spm_draft_widget.h"
#include "spm_prod_widget.h"
#include "browser_dialog.h"
#include "admin_engine.h"
#include "spm_engine.h"
#include "spm_task.h"
#include "spm_dialog.h"
#include "gqtl_log.h"

#include <QMessageBox>

namespace GS
{
namespace Gex
{

SPMDialog::SPMDialog(CGexMoTaskSPM *spmTask, QWidget *parent, bool modal) :
    StatisticalMonitoringDialog(spmTask, parent, modal)
{
}


StatisticalMonitoringProdWidget *SPMDialog::createProdWidget()
{
    return new SPMProdWidget(static_cast<CGexMoTaskSPM*>(mTask), mUI->tabVersions);
}

StatisticalMonitoringDraftWidget *SPMDialog::createDraftLimitsWidget()
{
    return new SPMDraftWidget(static_cast<CGexMoTaskSPM*>(mTask), mUI->tabDraftLimits);
}

StatisticalMonitoringSettingsWidget *SPMDialog::createSettingsWidget()
{
    return new SPMSettingsWidget(mUI->tabSettings);
}

bool SPMDialog::InitializeSpecificUI()
{
    mDraftWidget->setObjectName(QStringLiteral("SPMComputeWidget"));
    mSettingsWidget->setObjectName(QStringLiteral("SPMSettingsWidget"));
    mProdWidget->setObjectName(QStringLiteral("SPMProdWidget"));

    mUI->label_3->setText("<html><head/><body><p><span style=\" font-family:'Ms Sans Serif'; "
                          "font-size:14pt; font-weight:600;\"/><span style=\" font-family:'Ms Sans Serif'; "
                          "font-size:14pt;\"> SPM task:</span><span style=\" font-family:'Ms Sans Serif'; "
                          "font-size:12pt;\"> configure, compute, simulate, update your SPM limits</span></p>"
                          "</body></html>");

    setWindowTitle("SPM dialog");
   // setWindowIcon();

    return true;
}

void SPMDialog::DisableTab()
{
    bool lEnableCompute = mTask->HasComputedLimits() ||!mDraftWidget->IsLimitsModelEmpty();
    mUI->tabWidget->setTabEnabled(TAB_DRAFT, lEnableCompute);
    bool lEnableProd = (mTask->GetActiveProdVersionId() > 0) ||!mProdWidget->IsDataModelEmpty();
    mUI->tabWidget->setTabEnabled(TAB_VERSIONS, lEnableProd);
}

SPMDialog::~SPMDialog()
{
    }

bool SPMDialog::LoadFields()
{
    if(!mTask || !mSettingsWidget)
        return false;

    return mSettingsWidget->loadFields(mTask->GetProperties());
}

bool SPMDialog::AnalyseDuplicatedTasks()
{
    QPair<DuplicateStatus, CGexMoTaskStatisticalMonitoring *> lDuplicate =
            Engine::GetInstance().GetSPMEngine().FindDuplicate(mTask);

    QString lMessage;
    bool lIsDuplicateOK = true;
    if (lDuplicate.first == NOTDUPLICATED) //OK
    {
        lIsDuplicateOK = true;
    }
    else if (lDuplicate.first == MAININFODUPLICATEDONINSERTION) // ERROR
    {
        lIsDuplicateOK = false;
        lMessage = "ERROR\n\n"
                   "This task is a duplicate of: \"" +
                    lDuplicate.second->GetAttribute(C_TITLE).toString() + "\"\n"
                   "\n"
                   "Only one task is allowed on insertion for a given Database, Testing Stage, and Product.";
    }
    else if (lDuplicate.first == TITLEDUPLICATED) // ERROR
    {
        lIsDuplicateOK = false;
        lMessage = "ERROR\n\n"
                   "A task with the same name already exists.";
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("ERROR: Unknown duplicate status!").toLatin1().constData());
        GEX_ASSERT(false);
    }

    // pop message if any
    if (lMessage.isEmpty() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Task id %1 is a duplicate considering business rules - %2")
              .arg(mTask->GetID())
              .arg(lMessage)
              .toLatin1().constData());
        QMessageBox::warning(
                    this,
                    QString("Monitoring task editor"),
                    lMessage,
                    QMessageBox::Ok);
    }

    return lIsDuplicateOK;
}

bool SPMDialog::SaveSpecifTask()
{
    return Engine::GetInstance().GetSPMEngine().SaveTask(mTask);
}


} // namespace Gex
} // namespace GS
