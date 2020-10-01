#include <QMessageBox>
#include "statistical_monitoring_settings_widget.h"
#include "statistical_monitoring_limits_widget.h"
#include "statistical_monitoring_alarms_widget.h"
#include "statistical_monitoring_datapoint_struct.h"
#include "statistical_monitoring_limit_struct.h"
#include "statistical_monitoring_item_desc.h"
#include "ui_statistical_monitoring_dialog.h"
#include "statistical_monitoring_dialog.h"
#include "spm_draft_widget.h"
#include "browser_dialog.h"
#include "spm_prod_widget.h"
#include "admin_engine.h"
#include "product_info.h"
#include "gex_shared.h"
#include "gqtl_log.h"
#include "mo_task.h"
#include "spm_task.h"
#include "statistical_monitoring_taskdata.h"
#include "message.h"

extern GexMainwindow* pGexMainWindow;

namespace GS
{
namespace Gex
{

StatisticalMonitoringDialog::StatisticalMonitoringDialog(CGexMoTaskStatisticalMonitoring *task, QWidget *parent, bool modal) :
    QDialog(parent),
    mUI(new Ui::StatisticalMonitoringDialog),
    mSettingsWidget(0),
    mDraftWidget(0),
    mProdWidget(0),
    mTask(task),
    mProdWidgetInitialized(false),
    mDraftLimitsWidgetInitialized(false)
{
    mUI->setupUi(this);
    setModal(modal);
}

StatisticalMonitoringDialog::~StatisticalMonitoringDialog()
{
    delete mUI;
}


bool StatisticalMonitoringDialog::InitializeUI(const int DatabaseType, bool aIsNewTask)
{
    GexMainwindow::applyPalette(this);

    mDraftWidget       = createDraftLimitsWidget();
    mUI->layoutCompute->addWidget(mDraftWidget);

    mSettingsWidget    = createSettingsWidget();
    if(!mSettingsWidget || !mSettingsWidget->initializeUI(DatabaseType, aIsNewTask))
        return false;
    mUI->layoutSettings->addWidget(mSettingsWidget);

    mProdWidget        = createProdWidget();
    mUI->layoutProd->addWidget(mProdWidget);

    // Signal-Slot connections
    connect(mSettingsWidget,        SIGNAL(sSettingsChanged()),      this, SLOT(OnSettingsChanged()));
    connect(mSettingsWidget,        SIGNAL(sSettingsLoaded()),       this, SLOT(OnSettingsLoaded()), Qt::DirectConnection);
    connect(mSettingsWidget,        SIGNAL(sSaveClicked()),          this, SLOT(OnSave()));
    connect(mSettingsWidget,        SIGNAL(sComputeClicked()),       this, SLOT(OnCompute()));
    connect(mDraftWidget->GetLimitsWidget(), SIGNAL(sUpdateClicked()), this, SLOT(OnUpdate()));
    connect(mDraftWidget->GetAlarmsWidget(), SIGNAL(SimulateClicked(const QDateTime&, const QDateTime&)), this, SLOT(OnSimulate(const QDateTime&, const QDateTime&)));
    connect(mUI->tabWidget,         SIGNAL(currentChanged(int)),     this, SLOT(OnCurrentTabChanged(int)));
    connect(mDraftWidget->GetLimitsWidget(), SIGNAL(sValidationSucceeded()),this, SLOT(OnValidationSucceeded()));
    connect(mProdWidget,            SIGNAL(NewLimitCopied()),        this, SLOT(OnNewLimitCopied()));
    connect(mUI->buttonBox_OkCancel,SIGNAL(accepted()),              this, SLOT(OnOk()));
    connect(mUI->buttonBox_OkCancel,SIGNAL(rejected()),              this, SLOT(OnCancel()));
    connect(mSettingsWidget,        SIGNAL(sCreateLimitsClicked()),  this, SLOT(OnCreateManualLimits()));
    mUI->tabWidget->setCurrentIndex(TAB_SETTINGS);

    mSettingsChanged = false;

    RefreshUI();

    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint| Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    return InitializeSpecificUI();
}


void StatisticalMonitoringDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return ||
            event->key() == Qt::Key_Enter)
    {
        return;
    }
    QDialog::keyPressEvent(event);
}

void StatisticalMonitoringDialog::OnCancel(void)
{
    if(mTask)
    {
        // Reload the task from the database in order to cancel
        // any change dumped from the UI
        mTask->LoadTaskDataFromDb();
    }
    done(-1);
}

void StatisticalMonitoringDialog::OnSettingsChanged()
{
    mSettingsChanged = true;
    RefreshUI();
}

void StatisticalMonitoringDialog::OnSettingsLoaded()
{
    mSettingsChanged = false;
    RefreshUI();
}

void StatisticalMonitoringDialog::OnNewLimitCopied()
{
    if (!mDraftLimitsWidgetInitialized)
    {
        mDraftWidget->InitUI();
        mDraftLimitsWidgetInitialized = true;
    }
    mDraftWidget->LoadLimitsFromDatabase();
    mUI->tabWidget->setCurrentIndex(TAB_DRAFT);
}

void StatisticalMonitoringDialog::OnCurrentTabChanged(int newIndex)
{
    switch (newIndex) {
        case TAB_DRAFT:
            if (!mDraftLimitsWidgetInitialized)
            {
                mDraftWidget->InitUI();
                mDraftLimitsWidgetInitialized = true;
                mDraftWidget->LoadLimitsFromDatabase();
            }
        break;
        case TAB_VERSIONS:
            if (!mProdWidgetInitialized)
            {
                mProdWidget->InitializeUI();
                mProdWidgetInitialized = true;
                mProdWidget->LoadLimitsFromDatabase();
                mProdWidget->LoadVersionsFromDatabase();
            }
        break;
        default:
        break;
    }
}

void StatisticalMonitoringDialog::OnOk()
{
    if(!SaveTask())
    {
        return;
    }
    done(1);
}

void StatisticalMonitoringDialog::OnCompute()
{
    if(!OnSave())
    {
        return;
    }

    if(mTask && mDraftWidget)
    {
        if(!mTask->ComputeLimits())
        {
            return;
        }

        if(mTask->HasComputedLimits())
        {
            if (!mDraftLimitsWidgetInitialized)
            {
                mDraftWidget->InitUI();
                mDraftLimitsWidgetInitialized = true;
            }
            mDraftWidget->LoadLimitsFromDatabase();
            mUI->tabWidget->setCurrentIndex(TAB_DRAFT);
            RefreshUI();
        }
        else
        {
            QMessageBox::warning(this,"No limits computed",
                                 "No limits could be computed provided the monitored items rules you specified. Please make sure at least one item exists in the range you selected");
        }
    }
}

void StatisticalMonitoringDialog::OnUpdate()
{
    if(mTask && mDraftWidget)
    {
        if(mSettingsChanged)
        {
            bool lOk;
            GS::Gex::Message::request("Pending changes in the settings",
                                      "You have made changes in your settings which may affect the update result. "
                                      "Do you want to save them ?", lOk);
            if (lOk)
            {
                if(!SaveTask())
                {
                    return;
                }
            }
        }

        if(!mTask->ComputeLimits(true))
        {
            return;
        }

        mDraftWidget->LoadLimitsFromDatabase();
        mUI->tabWidget->setCurrentIndex(TAB_DRAFT);
        RefreshUI();
    }
}

void StatisticalMonitoringDialog::OnSimulate(const QDateTime& dateFrom, const QDateTime& dateTo)
{
    if (!GS::LPPlugin::ProductInfo::getInstance()->isSYLSBLAllowed())
    {
        GSLOG(SYSLOG_SEV_ERROR, (QString("No SYA license \nPlease contact %1")
                                 .arg(GEX_EMAIL_SALES).toLatin1().constData()));
        QMessageBox::warning(this,"SYA license error",
                             "SYA license has expired \nContact Quantix support at " +
                             QString(GEX_EMAIL_SUPPORT) +
                             " for more details.");
        return;
    }

    if(mTask && mDraftWidget)
    {
        if(mSettingsChanged)
        {
            bool lOk;
            GS::Gex::Message::request("Pending changes in the settings",
                                      "You have made changes in your settings which may affect the update result. "
                                      "Do you want to save them ?", lOk);
            if (lOk)
            {
                if(!OnSave())
                {
                    return;
                }
            }
        }

        setCursor(Qt::WaitCursor);

        QMap<QString, QVariant> lLogSummary;
        QList<StatMonAlarm> lAlarms;
        QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > > lDataPoints;
        QHash<MonitoredItemDesc, QMap<int, StatMonLimit> > lItemsLimits;
        mTask->CheckAgainstLimits(mTask->GetAttribute(C_PRODUCTREGEXP).toString(),
                                  "",
                                  "",
                                  "",
                                  -1,
                                  'S',
                                  lLogSummary,
                                  lAlarms,
                                  lDataPoints,
                                  lItemsLimits,
                                  &dateFrom,
                                  &dateTo);

        double threshold = mTask->GetAttribute(C_THRESHOLD).toDouble();
        if(threshold > 0.)
        {
            lLogSummary["threshold"] = threshold;
        }

        mDraftWidget->UpdateSimulateResult(lLogSummary, lAlarms);

        setCursor(Qt::ArrowCursor);
    }
}

void StatisticalMonitoringDialog::OnValidationSucceeded()
{
    if(mProdWidget)
    {
        if (!mProdWidgetInitialized)
        {
            mProdWidget->InitializeUI();
            mProdWidgetInitialized = true;
        }
        mProdWidget->LoadLimitsFromDatabase();
        mProdWidget->LoadVersionsFromDatabase(true);
        mUI->tabWidget->setCurrentIndex(TAB_VERSIONS);
        RefreshUI();
    }
}

void StatisticalMonitoringDialog::RefreshUI()
{
    QString lSettingsTabTitle = "Settings";
    if (mSettingsChanged)
    {
        lSettingsTabTitle.append("*");
    }

    mUI->tabWidget->setTabText(mUI->tabWidget->indexOf(mUI->tabSettings),lSettingsTabTitle);

    // disable tab if needed
    bool lEnableCompute = mTask->HasComputedLimits() || !mDraftWidget ||!mDraftWidget->IsLimitsModelEmpty();
    mUI->tabWidget->setTabEnabled(TAB_DRAFT, lEnableCompute);
    bool lEnableProd = (mTask->GetActiveProdVersionId() > 0) || !mProdWidget ||!mProdWidget->IsDataModelEmpty();
    mUI->tabWidget->setTabEnabled(TAB_VERSIONS, lEnableProd);
}

bool StatisticalMonitoringDialog::SaveTask()
{
    // check the license
    if(!(GS::LPPlugin::ProductInfo::getInstance()->isSYLSBLAllowed()))
    {
        QMessageBox::warning(this,
                             QString("Monitoring task editor"),
                             QString("No monitoring License\n"
                                     "No monitoring license available to execute the save of the monitoring task.\n"
                                     "Please contact %1").arg(GEX_EMAIL_SALES),
                             QMessageBox::Ok);
        return false;
    }

    if(mTask && mSettingsWidget && mDraftWidget)
    {
        // Make sure all mandatory fields are populated
        if(!mSettingsWidget->onCheckMandatoryFields())
        {
            return false;
        }

        mSettingsWidget->DumpFields(mTask->GetProperties());
        mDraftWidget->DumpFields();

        if (AnalyseDuplicatedTasks() == false) return false;

        if(SaveSpecifTask() == false)
        {
            // Display error and ask the user for a confirmation
            QString lError;
            if(Engine::GetInstance().GetAdminEngine().IsActivated() &&
                    Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector)
            {
                Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->GetLastError(lError);
            }
            if(Engine::GetInstance().GetAdminEngine().IsActivated() && lError.isEmpty())
            {
                Engine::GetInstance().GetAdminEngine().GetLastError(lError);
            }
            GSLOG(SYSLOG_SEV_ERROR, QString("SaveDbTasks return error %1").arg( lError).toLatin1().constData() );

            QMessageBox::warning(
                        this,
                        QString("Task editor"),
                        QString("Error saving the task.\n\n%1\n\nYour changes will not be saved.").arg(lError),
                        QMessageBox::Ok);
            return false;
        }
        return true;
    }
    return false;
}


bool StatisticalMonitoringDialog::OnSave()
{
    if(!SaveTask())
    {
        return false;
    }
    mSettingsChanged = false;
    RefreshUI();
    return true;
}

void StatisticalMonitoringDialog::OnCreateManualLimits()
{
    if(!OnSave())
    {
        return;
    }

    if(mTask && mDraftWidget && mSettingsWidget)
    {
        if(mSettingsWidget->GetNewTests().isEmpty())
            return;

        if(!mTask->ComputeLimits(false, &(mSettingsWidget->GetNewTests())))
        {
            return;
        }

        if (!mDraftLimitsWidgetInitialized)
        {
            mDraftWidget->InitUI();
            mDraftLimitsWidgetInitialized = true;
        }

        if(!mDraftWidget->LoadLimitsFromDatabase())
        {
            // Error reloading limits, stay on current tab
            return;
        }

        // Tests RegExp
        if(!mSettingsWidget->loadFields(mTask->GetProperties()))
        {
            // Error loading settings fields, stay on current tab
            return;
        }

        mUI->tabWidget->setTabEnabled(TAB_DRAFT, true);
        mUI->tabWidget->setCurrentIndex(TAB_DRAFT);

        RefreshUI();
    }
}


}
}
