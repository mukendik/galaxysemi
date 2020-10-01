#include <gqtl_log.h>
#include <license_provider.h>
#include <product_info.h>

#include "autoadmin/mo_create_task_auto_admin.h"
#include "autoadmin/autoadmin_task.h"
#include "common_widgets/collapsible_button.h"
#include "common_widgets/file_path_widget.h"
#include "mo_scheduler_gui.h"
#include "gexmo_constants.h"
#include "browser_dialog.h"
#include "gex_shared.h"
#include "message.h"

extern GexMainwindow* pGexMainWindow;

GexMoCreateTaskAutoAdmin::GexMoCreateTaskAutoAdmin(
        QWidget* parent,
        bool modal,
        Qt::WindowFlags fl) :
    QDialog(parent, fl),
    mAutoAdminData(this),
    mSPMShellsButton(0),
    mSYAShellsButton(0),
    mYieldShellsButton(0),
    mPATShellsButton(0)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Create Task AutoAdmin GUI...");
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(buttonBox_OkCancel,			SIGNAL(accepted()),		this, SLOT(OnOk()));
    QObject::connect(buttonBox_OkCancel,            SIGNAL(rejected()),		this, SLOT(OnCancel()));
    QObject::connect(pushButtonMailingList,			SIGNAL(clicked()),		this, SLOT(OnMailingList()));

    // Default Title
    LineEditTitle->setText("Auto Admin");

    // Default Auto-Admin starting time is 23:00
    QTime tStartTime(23,00);
    timeEditStart->setTime(tStartTime);

    // Load list of Auto-Report erase frequencies (1 day, 1 week,, etc...)
    comboBoxEraseReports->clear();

    int nItem = 0;
    while (gexMoLabelTaskReportEraseFrequency[nItem] != 0)
    {
        comboBoxEraseReports->insertItem(nItem, gexMoLabelTaskReportEraseFrequency[nItem]);
        nItem++;
    }

    // Default: Erase reports older than 1 week.
    comboBoxEraseReports->setCurrentIndex(GEXMO_RPTERASE_FREQ_1WEEK);

    // Default: Log errors only
    comboBoxLogContent->setCurrentIndex(GEXMO_AUTOADMIN_LOG_EXCEPTIONS);

    // Default: don't send daily insertion logs
    checkBoxSendLogs->setChecked(false);

    // Focus on 1st Edit field
    LineEditTitle->setFocus();

    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        lineEditEmailFrom->setText(GEX_EMAIL_PAT_MAN);
    else
        lineEditEmailFrom->setText(GEX_EMAIL_YIELD_MAN);

    mSPMShellsButton = CreateCollapsibleArea("SPM tasks", "spm");
    scrollAreaWidgetContents->layout()->addWidget(mSPMShellsButton);

    mSYAShellsButton = CreateCollapsibleArea("SYA tasks", "sya");
    scrollAreaWidgetContents->layout()->addWidget(mSYAShellsButton);

    mYieldShellsButton = CreateCollapsibleArea("Yield monitoring tasks", "yield");
    scrollAreaWidgetContents->layout()->addWidget(mYieldShellsButton);

    mPATShellsButton = CreateCollapsibleArea("PAT tasks", "pat");
    scrollAreaWidgetContents->layout()->addWidget(mPATShellsButton);

    static_cast<QVBoxLayout*>(scrollAreaWidgetContents->layout())->addSpacerItem(
                new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));


    tabWidget2->setCurrentIndex(0);
}

void GexMoCreateTaskAutoAdmin::OnMailingList(void)
{
    // Info message about Mailing list file format.
    GS::Gex::Message::information(
                "", "Mailing list file format:\n\n"
                    " o It is an ASCII file\n"
                    " o It can hold multiple emails per line\n"
                    " o email format is <address>@<domain>\n"
                    " o email addresses separator is ';'\n\n");

    QString strMailingList;
    QFileDialog cFileDialog(this);
    strMailingList = cFileDialog.getOpenFileName(this, "Select mailing list", "", "Mailing list *.txt;;All Files (*.*)");
    if(strMailingList.isEmpty() == true)
        return;	// No mailing list file selected...return!

    // Save folder selected.
    lineEditEmailList->setText(strMailingList);
}

void GexMoCreateTaskAutoAdmin::OnOk(void)
{
    // Get Task title.
    mAutoAdminData.mTitle = LineEditTitle->text();
    if(mAutoAdminData.mTitle.isEmpty())
    {
        // Need to enter a title!
        GS::Gex::Message::warning("", "You must specify a task title!");
        LineEditTitle->setFocus();
        return;
    }

    mAutoAdminData.mStartTime = timeEditStart->time();
    mAutoAdminData.mKeepReportDuration = comboBoxEraseReports->currentIndex();	// How long to keep reports on server
    mAutoAdminData.mLogContents = comboBoxLogContent->currentIndex();	// Default: Log errors only
    mAutoAdminData.mSendInsertionLogs = checkBoxSendLogs->isChecked();	// Default false

    mAutoAdminData.mEmailFrom = lineEditEmailFrom->text();
    if ( mAutoAdminData.mEmailFrom.isEmpty())
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            mAutoAdminData.mEmailFrom = GEX_EMAIL_PAT_MAN;
        else
            mAutoAdminData.mEmailFrom = GEX_EMAIL_YIELD_MAN;
    }
    mAutoAdminData.mEmailNotify = lineEditEmailList->text();
    if(mAutoAdminData.mEmailNotify.isEmpty())
    {
        // Need to specify an email address
        GS::Gex::Message::warning("", "No Sys. Admin email defined...\n"
                                      "you may miss important notifications!");
    }

    // Ensure to save current Shell settings if edited and not saved yet
    SaveShellSettings();

    done(1);
}

void GexMoCreateTaskAutoAdmin::OnCancel(void)
{
    done(-1);
}

void GexMoCreateTaskAutoAdmin::SaveShellSettings()
{
    // Shells for: Pass
    mAutoAdminData.SetAttribute(C_ShellYieldPass,
                                mShellPathWidgets.value(C_ShellPassKey + C_YieldTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellSpmPass,
                                mShellPathWidgets.value(C_ShellPassKey + C_SpmTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellSyaPass,
                                mShellPathWidgets.value(C_ShellPassKey + C_SyaTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellPatPass,
                                mShellPathWidgets.value(C_ShellPassKey + C_PatTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellYieldPassIsEnabled,
                                mShellPathWidgets.value(C_ShellPassKey + C_YieldTask)->isChecked());
    mAutoAdminData.SetAttribute(C_ShellSpmPassIsEnabled,
                                mShellPathWidgets.value(C_ShellPassKey + C_SpmTask)->isChecked());
    mAutoAdminData.SetAttribute(C_ShellSyaPassIsEnabled,
                                mShellPathWidgets.value(C_ShellPassKey + C_SyaTask)->isChecked());
    mAutoAdminData.SetAttribute(C_ShellPatPassIsEnabled,
                                mShellPathWidgets.value(C_ShellPassKey + C_PatTask)->isChecked());

    // Shells for: Standard alarms
    mAutoAdminData.SetAttribute(C_ShellYieldAlarm,
                                mShellPathWidgets.value(C_ShellStdKey + C_YieldTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellSpmAlarm,
                                mShellPathWidgets.value(C_ShellStdKey + C_SpmTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellSyaAlarm,
                                mShellPathWidgets.value(C_ShellStdKey + C_SyaTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellPatAlarm,
                                mShellPathWidgets.value(C_ShellStdKey + C_PatTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellYieldAlarmIsEnabled,
                                mShellPathWidgets.value(C_ShellStdKey + C_YieldTask)->isChecked());
    mAutoAdminData.SetAttribute(C_ShellSpmAlarmIsEnabled,
                                mShellPathWidgets.value(C_ShellStdKey + C_SpmTask)->isChecked());
    mAutoAdminData.SetAttribute(C_ShellSyaAlarmIsEnabled,
                                mShellPathWidgets.value(C_ShellStdKey + C_SyaTask)->isChecked());
    mAutoAdminData.SetAttribute(C_ShellPatAlarmIsEnabled,
                                mShellPathWidgets.value(C_ShellStdKey + C_PatTask)->isChecked());

    // Shells for: Critical alarms
    mAutoAdminData.SetAttribute(C_ShellYieldAlarmCritical,
                                mShellPathWidgets.value(C_ShellCritKey + C_YieldTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellSpmAlarmCritical,
                                mShellPathWidgets.value(C_ShellCritKey + C_SpmTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellSyaAlarmCritical,
                                mShellPathWidgets.value(C_ShellCritKey + C_SyaTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellPatAlarmCritical,
                                mShellPathWidgets.value(C_ShellCritKey + C_PatTask)->GetFilePath());
    mAutoAdminData.SetAttribute(C_ShellYieldAlarmCriticalIsEnabled,
                                mShellPathWidgets.value(C_ShellCritKey + C_YieldTask)->isChecked());
    mAutoAdminData.SetAttribute(C_ShellSpmAlarmCriticalIsEnabled,
                                mShellPathWidgets.value(C_ShellCritKey + C_SpmTask)->isChecked());
    mAutoAdminData.SetAttribute(C_ShellSyaAlarmCriticalIsEnabled,
                                mShellPathWidgets.value(C_ShellCritKey + C_SyaTask)->isChecked());
    mAutoAdminData.SetAttribute(C_ShellPatAlarmCriticalIsEnabled,
                                mShellPathWidgets.value(C_ShellCritKey + C_PatTask)->isChecked());
}

CollapsibleButton* GexMoCreateTaskAutoAdmin::CreateCollapsibleArea(
        const QString& title,
        const QString &key)
{
    // Create button
    CollapsibleButton* lCollapsibleArea = new CollapsibleButton(title, T_Header, 12 );
    // with its frame
    QFrame* lFrame = new QFrame(this);
    lFrame->setLayout(new QVBoxLayout());
    lFrame->layout()->setContentsMargins(0, 0, 0, 0);

    QString lCaption = "Select shell...";
    QString lFilter = "Shell applications/Executables (*)";

    // Add Pass script area
    FilePathWidget* lPathWidget = new FilePathWidget("on pass:", lCaption, lFilter, "", true, this);
    mShellPathWidgets.insert(C_ShellPassKey + key, lPathWidget);
    lFrame->layout()->addWidget(lPathWidget);

    // Add Standard alarm area
    lPathWidget = new FilePathWidget("on standard alarm:", lCaption, lFilter, "", true, this);
    mShellPathWidgets.insert(C_ShellStdKey + key, lPathWidget);
    lFrame->layout()->addWidget(lPathWidget);

    // Add Critical alarm area
    lPathWidget = new FilePathWidget("on critical alarm:", lCaption, lFilter, "", true, this);
    mShellPathWidgets.insert(C_ShellCritKey + key, lPathWidget);
    lFrame->layout()->addWidget(lPathWidget);

    lCollapsibleArea->addWidget(lFrame);

    return lCollapsibleArea;
}

void GexMoCreateTaskAutoAdmin::LoadFields(CGexMoTaskAutoAdmin *ptTaskItem)
{
    if (!ptTaskItem)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Empty task item, unable to load fields");
        return;
    }

    GexMoAutoAdminTaskData* lTaskProperty = ptTaskItem->GetProperties();
    if (!lTaskProperty)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Empty task property, unable to load fields");
        return;
    }
    // Task Title
    LineEditTitle->setText(lTaskProperty->mTitle);

    // Start, Auto-Admin time
    timeEditStart->setTime(lTaskProperty->mStartTime);

    // How long to keep reports on server before removing them.
    comboBoxEraseReports->setCurrentIndex(lTaskProperty->mKeepReportDuration);

    // Log file contents
    comboBoxLogContent->setCurrentIndex(lTaskProperty->mLogContents);

    // Email 'From'
    lineEditEmailFrom->setText(lTaskProperty->mEmailFrom);

    // Emails to notify
    lineEditEmailList->setText(lTaskProperty->mEmailNotify);

    // Send daily insertion logs
    checkBoxSendLogs->setChecked(lTaskProperty->mSendInsertionLogs);

    // Shells for: Pass
    mShellPathWidgets.value(C_ShellPassKey + C_YieldTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellYieldPass).toString());
    if (!lTaskProperty->GetAttribute(C_ShellYieldPassIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellPassKey + C_YieldTask)->setChecked(
                    lTaskProperty->GetAttribute(C_ShellYieldPassIsEnabled).toBool());
    }
    mShellPathWidgets.value(C_ShellPassKey + C_SpmTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellSpmPass).toString());
    if (!lTaskProperty->GetAttribute(C_ShellSpmPassIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellPassKey + C_SpmTask)->setChecked(
                    lTaskProperty->GetAttribute(C_ShellSpmPassIsEnabled).toBool());
    }
    mShellPathWidgets.value(C_ShellPassKey + C_SyaTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellSyaPass).toString());
    if (!lTaskProperty->GetAttribute(C_ShellSyaPassIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellPassKey + C_SyaTask)->setChecked(
                    lTaskProperty->GetAttribute(C_ShellSyaPassIsEnabled).toBool());
    }
    mShellPathWidgets.value(C_ShellPassKey + C_PatTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellPatPass).toString());
    if (!lTaskProperty->GetAttribute(C_ShellPatPassIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellPassKey + C_PatTask)->setChecked(
                lTaskProperty->GetAttribute(C_ShellPatPassIsEnabled).toBool());
    }
    // Shells for: Standard alarms
    mShellPathWidgets.value(C_ShellStdKey + C_YieldTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellYieldAlarm).toString());
    if (!lTaskProperty->GetAttribute(C_ShellYieldAlarmIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellStdKey + C_YieldTask)->setChecked(
                lTaskProperty->GetAttribute(C_ShellYieldAlarmIsEnabled).toBool());
    }
    mShellPathWidgets.value(C_ShellStdKey + C_SpmTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellSpmAlarm).toString());
    if (!lTaskProperty->GetAttribute(C_ShellSpmAlarmIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellStdKey + C_SpmTask)->setChecked(
                lTaskProperty->GetAttribute(C_ShellSpmAlarmIsEnabled).toBool());
    }
    mShellPathWidgets.value(C_ShellStdKey + C_SyaTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellSyaAlarm).toString());
    if (!lTaskProperty->GetAttribute(C_ShellSyaAlarmIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellStdKey + C_SyaTask)->setChecked(
                lTaskProperty->GetAttribute(C_ShellSyaAlarmIsEnabled).toBool());
    }
    mShellPathWidgets.value(C_ShellStdKey + C_PatTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellPatAlarm).toString());
    if (!lTaskProperty->GetAttribute(C_ShellPatAlarmIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellStdKey + C_PatTask)->setChecked(
                lTaskProperty->GetAttribute(C_ShellPatAlarmIsEnabled).toBool());
    }
    // Shells for: Critical alarms
    mShellPathWidgets.value(C_ShellCritKey + C_YieldTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellYieldAlarmCritical).toString());
    if (!lTaskProperty->GetAttribute(C_ShellYieldAlarmCriticalIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellCritKey + C_YieldTask)->setChecked(
                lTaskProperty->GetAttribute(C_ShellYieldAlarmCriticalIsEnabled).toBool());
    }
    mShellPathWidgets.value(C_ShellCritKey + C_SpmTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellSpmAlarmCritical).toString());
    if (!lTaskProperty->GetAttribute(C_ShellSpmAlarmCriticalIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellCritKey + C_SpmTask)->setChecked(
                lTaskProperty->GetAttribute(C_ShellSpmAlarmCriticalIsEnabled).toBool());
    }
    mShellPathWidgets.value(C_ShellCritKey + C_SyaTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellSyaAlarmCritical).toString());
    if (!lTaskProperty->GetAttribute(C_ShellSyaAlarmCriticalIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellCritKey + C_SyaTask)->setChecked(
                lTaskProperty->GetAttribute(C_ShellSyaAlarmCriticalIsEnabled).toBool());
    }
    mShellPathWidgets.value(C_ShellCritKey + C_PatTask)->SetFilePath(
                lTaskProperty->GetAttribute(C_ShellPatAlarmCritical).toString());
    if (!lTaskProperty->GetAttribute(C_ShellPatAlarmCriticalIsEnabled).isNull())
    {
        mShellPathWidgets.value(C_ShellCritKey + C_PatTask)->setChecked(
                lTaskProperty->GetAttribute(C_ShellPatAlarmCriticalIsEnabled).toBool());
    }
}

GexMoAutoAdminTaskData &GexMoCreateTaskAutoAdmin::GetData()
{
    return mAutoAdminData;
}
