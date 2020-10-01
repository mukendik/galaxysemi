
#include "statistical_monitoring_taskdata.h"
#include "statistical_monitoring_task.h"
#include "sya_settings_widget.h"
#include "sya_draft_widget.h"
#include "sya_prod_widget.h"
#include "browser_dialog.h"
#include "admin_engine.h"
#include "sya_engine.h"
#include "sya_dialog.h"
#include "sya_task.h"
#include "gqtl_log.h"

#include <QMessageBox>


namespace GS
{
namespace Gex
{

SYADialog::SYADialog(CGexMoTaskStatisticalMonitoring *task, QWidget *parent, bool modal) :
    StatisticalMonitoringDialog(task, parent, modal)
{
}

SYADialog::~SYADialog()
{
}

StatisticalMonitoringProdWidget *SYADialog::createProdWidget()
{
    return new SYAProdWidget(static_cast<CGexMoTaskSYA*>(mTask), mUI->tabVersions);
}

StatisticalMonitoringDraftWidget *SYADialog::createDraftLimitsWidget()
{
    return new SYADraftWidget(static_cast<CGexMoTaskSYA*>(mTask), mUI->tabDraftLimits);
}

StatisticalMonitoringSettingsWidget *SYADialog::createSettingsWidget()
{
    return new SYASettingsWidget(mUI->tabSettings);
}

bool SYADialog::InitializeSpecificUI()
{
    mSettingsWidget->setObjectName(QStringLiteral("SYASettingsWidget"));
    mUI->label_3->setText("<html><head/><body><p><span style=\" font-family:'Ms Sans Serif'; font-size:14pt; "
                          "font-weight:600;\"/><span style=\" font-family:'Ms Sans Serif'; font-size:14pt;\"> "
                          "SYA task:</span><span style=\" font-family:'Ms Sans Serif'; font-size:12pt;\"> "
                          "configure, compute, simulate, update your SYA limits</span></p></body></html>");

    setWindowTitle("SYA dialog");

    //this tooltip differs from SPMDialog's one, let's reset it (bins instead of tests)
    mSettingsWidget->getUI()->buttonPickTests->setToolTip("Pick binnings from list");

    return true;
}



bool SYADialog::LoadFields()
{
    if(!mTask || !mSettingsWidget)
        return false;

    return mSettingsWidget->loadFields(mTask->GetProperties());
}

void SYADialog::DisableTab()
{
    mUI->tabWidget->setTabEnabled(TAB_DRAFT, false);
    mUI->tabWidget->setTabEnabled(TAB_VERSIONS, false);
}

bool SYADialog::AnalyseDuplicatedTasks()
{
    QPair<DuplicateStatus, CGexMoTaskStatisticalMonitoring *> lDuplicate =
            Engine::GetInstance().GetSYAEngine().FindDuplicate(mTask);

    QString lMessage;
    bool lIsDuplicateOK = true;
    if (lDuplicate.first == NOTDUPLICATED) //OK
    {
        lIsDuplicateOK = true;
    }
    else if (lDuplicate.first == MAININFODUPLICATEDONINSERTION) // WARNING [deprecated]
    {
        lIsDuplicateOK = true;
        lMessage = "WARNING\n\n"
                   "This task has same Database, Testing Stage, and Product that: \"" +
                    lDuplicate.second->GetAttribute(C_TITLE).toString() + "\"\n"
                   "\n"
                   "Filters are different but please make sure that on insertion,"
                   "you won't have duplicated alarms.";
    }
    else if (lDuplicate.first == TITLEDUPLICATED) // ERROR
    {
        lIsDuplicateOK = false;
        lMessage = "ERROR\n\n"
                   "A task with the same name already exists.";
    }
    else if (lDuplicate.first == MAININFODUPLICATEDONTRIGGER) // ERROR [deprecated]
    {
        lIsDuplicateOK = false;
        lMessage = "ERROR\n\n"
                   "This task is a duplicate of: \"" +
                    lDuplicate.second->GetAttribute(C_TITLE).toString() + "\"\n"
                   "\n"
                   "Only one task is allowed on trigger for a given Database, Testing Stage, and Product.";
    }
    else if (lDuplicate.first == FILTERSDULPLICATEDONINSERTION) // ERROR
    {
        lIsDuplicateOK = false;
        lMessage = "ERROR\n\n"
                   "This task is a duplicate of: \"" +
                    lDuplicate.second->GetAttribute(C_TITLE).toString() + "\"\n"
                   "\n"
                   "Only one task is allowed on insertion for a given Database, Testing Stage, Product and Filters set.";
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

bool SYADialog::SaveSpecifTask()
{
    return Engine::GetInstance().GetSYAEngine().SaveTask(mTask);
}


} // namespace Gex
} // namespace GS
