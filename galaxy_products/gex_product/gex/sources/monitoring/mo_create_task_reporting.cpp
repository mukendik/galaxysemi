#include <gqtl_log.h>
#include <license_provider.h>
#include <product_info.h>

#include "gexmo_constants.h"
#include "gex_shared.h"
#include "engine.h"
#include "message.h"
#include "reporting/reporting_task.h"
#include "browser_dialog.h"
#include "mo_scheduler_gui.h"
#include "mo_task_create_gui.h"

extern GexMainwindow* pGexMainWindow;

GexMoCreateTaskReporting::GexMoCreateTaskReporting( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl), cReportingData(this)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Create Task Reporting...");
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(checkBoxExecutionWindow,	SIGNAL(clicked()),		this, SLOT(OnExecutionWindow()));
    QObject::connect(pushButtonBrowse,			SIGNAL(clicked()),		this, SLOT(OnBrowse()));
    QObject::connect(buttonBox_OkCancel,		SIGNAL(accepted()),		this, SLOT(OnOk()));
    QObject::connect(buttonBox_OkCancel,        SIGNAL(rejected()),		this, SLOT(OnCancel()));
    QObject::connect(pushButtonMailingList,		SIGNAL(clicked()),		this, SLOT(OnMailingList()));
    QObject::connect(comboBoxTaskFrequency,		SIGNAL(activated(int)), this, SLOT(OnTaskFrequency()));

    // Default Title
    LineEditTitle->setText("Reporting");

    // Load list of tesk frequnecies (1minute,2min,3min, 1hour, etc...)
    comboBoxTaskFrequency->clear();
    int nItem = 0;

    while (gexMoLabelTaskFrequency[nItem])
        comboBoxTaskFrequency->addItem(gexMoLabelTaskFrequency[nItem++]);

    // Load list of Days of the week
    comboBoxTaskFrequencyDayOfWeek->clear();

    nItem = 0;
    while (gexMoLabelTaskFrequencyDayOfWeek[nItem])
        comboBoxTaskFrequencyDayOfWeek->addItem(gexMoLabelTaskFrequencyDayOfWeek[nItem++]);

    // Default frequency: every 30 minutes.
    comboBoxTaskFrequency->setCurrentIndex(GEXMO_TASKFREQ_1DAY);

    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        lineEditEmailFrom->setText(GEX_EMAIL_PAT_MAN);
    else
        lineEditEmailFrom->setText(GEX_EMAIL_YIELD_MAN);

    // Show/Hide relevant fields
    OnExecutionWindow();
}

void GexMoCreateTaskReporting::OnTaskFrequency(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Create Task Reporting On Task Frequency...");

    switch(comboBoxTaskFrequency->currentIndex())
    {
        case	GEXMO_TASKFREQ_1MIN:			// Task every: 1 minute
        case	GEXMO_TASKFREQ_2MIN:			// Task every: 2 minutes
        case	GEXMO_TASKFREQ_3MIN:			// Task every: 3 minutes
        case	GEXMO_TASKFREQ_4MIN:			// Task every: 4 minutes
        case	GEXMO_TASKFREQ_5MIN:			// Task every: 5 minutes
        case	GEXMO_TASKFREQ_10MIN:			// Task every: 10 minutes
        case	GEXMO_TASKFREQ_15MIN:			// Task every: 15 minutes
        case	GEXMO_TASKFREQ_30MIN:			// Task every: 30 minutes
        case	GEXMO_TASKFREQ_1HOUR:			// Task every: 1 hour
        case	GEXMO_TASKFREQ_2HOUR:			// Task every: 2 hours
        case	GEXMO_TASKFREQ_3HOUR:			// Task every: 3 hours
        case	GEXMO_TASKFREQ_4HOUR:			// Task every: 4 hours
        case	GEXMO_TASKFREQ_5HOUR:			// Task every: 5 hours
        case	GEXMO_TASKFREQ_6HOUR:			// Task every: 6 hours
        case	GEXMO_TASKFREQ_12HOUR:			// Task every: 12 hours
        case	GEXMO_TASKFREQ_1DAY:			// Task every: 1 day
        case	GEXMO_TASKFREQ_2DAY:			// Task every: 2 days
        case	GEXMO_TASKFREQ_3DAY:			// Task every: 3 days
        case	GEXMO_TASKFREQ_4DAY:			// Task every: 4 days
        case	GEXMO_TASKFREQ_5DAY:			// Task every: 5 days
        case	GEXMO_TASKFREQ_6DAY:			// Task every: 6 days
            TextLabelDayOfWeek->hide();
            comboBoxTaskFrequencyDayOfWeek->hide();
            break;

        case	GEXMO_TASKFREQ_1WEEK:			// Task every: 1 week
        case	GEXMO_TASKFREQ_2WEEK:			// Task every: 2 weeks
        case	GEXMO_TASKFREQ_3WEEK:			// Task every: 3 weeks
        case	GEXMO_TASKFREQ_1MONTH:			// Task every: 1 month
            TextLabelDayOfWeek->show();
            comboBoxTaskFrequencyDayOfWeek->show();
            break;
    }
}

void GexMoCreateTaskReporting::OnBrowse(void)
{
    // Prompt user to select the script file of choice.
    QString fn = QFileDialog::getOpenFileName(this, "Select script file", LineEditPath->text(), "Script File (*.csl)");

    // If no file selected, ignore command and return to home page.
    if(fn.isEmpty())
        return;

    // Save file selected.
    LineEditPath->setText(fn);
}

void GexMoCreateTaskReporting::OnExecutionWindow(void)
{
    if(checkBoxExecutionWindow->isChecked() == true)
    {
        timeEditStart->show();
        TextLabelStopWindow->show();
        timeEditStop->show();
    }
    else
    {
        timeEditStart->hide();
        TextLabelStopWindow->hide();
        timeEditStop->hide();
    }

    // Refresh Frequency & Day of Week shown/hiddem status
    OnTaskFrequency();
}

void GexMoCreateTaskReporting::OnMailingList(void)
{
    // Info message about Mailing list file format.
    GS::Gex::Message::information(
        "", "Mailing list file format:\n\n o It is an ASCII file\n"
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


void GexMoCreateTaskReporting::OnCancel()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Cancelling creation of reporting task...");
    done(-1);
}

void GexMoCreateTaskReporting::OnOk(void)
{
    // Get Task title.
    cReportingData.strTitle = LineEditTitle->text();
    if(cReportingData.strTitle.isEmpty())
    {
        // Need to enter a title!
        GS::Gex::Message::warning("", "You must specify a task title!");
        LineEditTitle->setFocus();
        return;
    }

    // Get Script path
    cReportingData.strScriptPath = LineEditPath->text();
    if(cReportingData.strScriptPath.isEmpty())
    {
        // Need to enter a script name!
        GS::Gex::Message::
            warning("", "You must specify the script file to execute!");
        LineEditPath->setFocus();
        return;
    }

    // Get task Frequency
    cReportingData.iFrequency = comboBoxTaskFrequency->currentIndex();

    // Get day of week
    cReportingData.iDayOfWeek = comboBoxTaskFrequencyDayOfWeek->currentIndex();

    // Get Time window info.
    if(checkBoxExecutionWindow->isChecked())
        cReportingData.bExecutionWindow = true;
    else
        cReportingData.bExecutionWindow = false;
    cReportingData.cStartTime = timeEditStart->time();
    cReportingData.cStopTime = timeEditStop->time();

    cReportingData.iNotificationType = comboBoxGeneration->currentIndex();
    cReportingData.strEmailFrom = lineEditEmailFrom->text();
    if ( cReportingData.strEmailFrom.isEmpty() == true )
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            cReportingData.strEmailFrom = GEX_EMAIL_PAT_MAN;
        else
            cReportingData.strEmailFrom = GEX_EMAIL_YIELD_MAN;
    }
    cReportingData.strEmailNotify = lineEditEmailList->text();
    if((cReportingData.strEmailNotify.isEmpty() == true) && (cReportingData.iNotificationType != 2))
    {
        // Need to specify an email address if an email notification is requested
        if(QMessageBox::question( this,
                                  GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                  "No email specified...\nTask won't send email notifications!\n\nDo you confirm this choice?",QMessageBox::Yes,QMessageBox::No | QMessageBox::Default) != QMessageBox::Yes)
        {
            lineEditEmailList->setFocus();
            return;
        }
    }

    cReportingData.bHtmlEmail = (comboBoxMailFormat->currentIndex() == 0) ? true : false;

    if(comboBoxGeneration->currentIndex() == 0 && LineEditPath->isEnabled()){
        GS::Gex::Message::information(
            "", "You chose to attach the report to the email notification.\n"
            "Appropriate format to use is PDF/Word format.\n"
            "Please make sure that the CSL script doesn't "
            "generate a HTML report.");
    }

    done(1);
}

void GexMoCreateTaskReporting::LoadFields(CGexMoTaskReporting *ptTaskItem)
{
    // Task Title
    LineEditTitle->setText(ptTaskItem->GetProperties()->strTitle);

    // Script file
    LineEditPath->setText(ptTaskItem->GetProperties()->strScriptPath);

    // Task frequency
    comboBoxTaskFrequency->setCurrentIndex(ptTaskItem->GetProperties()->iFrequency);

    // Day of week
    comboBoxTaskFrequencyDayOfWeek->setCurrentIndex(ptTaskItem->GetProperties()->iDayOfWeek);

    // Starting time window enabled?
    checkBoxExecutionWindow->setChecked(ptTaskItem->GetProperties()->bExecutionWindow);

    // Start, stop time
    timeEditStart->setTime(ptTaskItem->GetProperties()->cStartTime);
    timeEditStop->setTime(ptTaskItem->GetProperties()->cStopTime);

    // Report generation type: zip+email, or email URL
    comboBoxGeneration->setCurrentIndex(ptTaskItem->GetProperties()->iNotificationType);

    // Email from
    lineEditEmailFrom->setText(ptTaskItem->GetProperties()->strEmailFrom);

    // Emails to notify
    lineEditEmailList->setText(ptTaskItem->GetProperties()->strEmailNotify);

    // Email format: HTML or ASCII
    if(ptTaskItem->GetProperties()->bHtmlEmail)
        comboBoxMailFormat->setCurrentIndex(0);	// HTML (default)
    else
        comboBoxMailFormat->setCurrentIndex(1);	// TEXT

    // Show/Hide relevant fields
    OnExecutionWindow();
}
