#include <gqtl_log.h>
#include <license_provider.h>
#include <product_info.h>

#include "gexmo_constants.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "engine.h"
#include "message.h"
#include "status/status_task.h"
#include "browser_dialog.h"
#include "mo_scheduler_gui.h"
#include "mo_task_create_gui.h"

GexMoCreateTaskStatus::GexMoCreateTaskStatus( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl), m_cStatusData(this)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Create a new Status task GUI...");
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(pushButtonBrowse,      SIGNAL(clicked()), this, SLOT(OnBrowse()));
    QObject::connect(buttonBox_OkCancel,    SIGNAL(accepted()), this, SLOT(OnOk()));
    QObject::connect(buttonBox_OkCancel,    SIGNAL(rejected()), this, SLOT(OnCancel()));

    // Default Title
    LineEditTitle->setText("Manufacturing Status");
}

void GexMoCreateTaskStatus::OnBrowse(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "New Status task GUI: on browse intranet folder...");

    QString s;

    // Get current path entered (if any).
    s = LineEditPath->text();

    // If no path define yet, start from default directory.
    if(s.isEmpty() == true)
        s = ".";

    // Popup directory browser
    s = QFileDialog::getExistingDirectory(
                this,
                "Choose a directory (path to the Intranet folder)",
                s,
                QFileDialog::ShowDirsOnly );

    // Check if valid selection
    if(s.isEmpty() == true)
        return;

    // Save folder selected.
    LineEditPath->setText(s);
}

void	GexMoCreateTaskStatus::OnOk(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Create a new Status task GUI: on ok...");

    // Get Task title.
    m_cStatusData.setTitle(LineEditTitle->text());

    if(m_cStatusData.title().isEmpty())
    {
        // Need to enter a title!
        GS::Gex::Message::warning("", "You must specify a task title!");
        LineEditTitle->setFocus();
        return;
    }

    // Get Web organization type to create.
    switch(comboBoxWebStructure->currentIndex())
    {
    case 0:	// ONE web site merging ALL databases status info
        m_cStatusData.setOneWebPerDatabase(false);
        break;
    case 1:	// ONE child-web site for each database.
        m_cStatusData.setOneWebPerDatabase(true);
        break;
    }

    // Get Intranet path
    m_cStatusData.setIntranetPath(LineEditPath->text());

    if(m_cStatusData.intranetPath().isEmpty())
    {
        // Need to enter a folder name!
        GS::Gex::Message::warning(
            "",
            "You must specify the Intranet folder path.\n"
            "It's need for creating Web pages and email events!");
        LineEditPath->setFocus();
        return;
    }

    // Get Home page name
    m_cStatusData.setHomePage(LineEditHomePage->text());

    if(m_cStatusData.homePage().isEmpty())
    {
        // Need to enter a page name
        GS::Gex::Message::warning("", "You must specify the Report home page!");
        LineEditHomePage->setFocus();
        return;
    }

    // Get Report's URL name, use to display in Emails if http URL is empty (hyperlink)
    m_cStatusData.setReportURL(LineEditHomeReportURL->text());
    if(GS::Gex::Engine::GetInstance().GetDatabaseEngine().IsRootFolder(m_cStatusData.reportURL()))
    {
        // Need to enter a title!
        GS::Gex::Message::
            warning("", "You must specify a valid Reports directory!");
        LineEditHomeReportURL->setFocus();
        return;
    }


    // Get Report's http URL name to display in Emails (hyperlink)
    m_cStatusData.setReportHttpURL(LineEditHomeReportHttpURL->text());
    if(GS::Gex::Engine::GetInstance().GetDatabaseEngine().IsRootFolder(m_cStatusData.reportURL()))
    {
        // Need to enter a title!
        GS::Gex::Message::
            warning("", "You must specify a valid Report's http URL!");
        LineEditHomeReportHttpURL->setFocus();
        return;
    }

    // Create <WebSite> & email folders.
    QDir	cDir;
    bool	bStatus;
    QString strMessage;
    QString	strString = m_cStatusData.intranetPath();

    // <WebSite>
    if(QFile::exists(strString) == false)
    {
        bStatus = cDir.mkdir(strString);
        if(bStatus == false)
        {
            strMessage="** Failed creating folder:\n" + strString;
            GS::Gex::Message::warning("", strMessage);
            LineEditPath->setFocus();
            return;
        }
    }
    // ensure the path ends with a '/'
    if((strString.endsWith(":") == false) &&(strString.endsWith("/") == false) && (strString.endsWith("\\")==false))
        strString += '/';

    // <WebSite>/examinator_monitoring
    strString += GEXMO_AUTOREPORT_FOLDER;
    if(QFile::exists(strString) == false)
    {
        bStatus = cDir.mkdir(strString);
        if(bStatus == false)
        {
            strMessage="** Failed creating folder:\n" + strString;
            GS::Gex::Message::warning("", strMessage);
            LineEditPath->setFocus();
            return;
        }
    }

    // <WebSite>/.emails
    strString += QString("/");
    strString += GEXMO_AUTOREPORT_EMAILS;
    if(QFile::exists(strString) == false)
    {
        bStatus = cDir.mkdir(strString);
        if(bStatus == false)
        {
            strMessage="** Failed creating folder:\n" + strString;
            GS::Gex::Message::warning("", strMessage);
            LineEditPath->setFocus();
            return;
        }
    }

    done(1);
}

void GexMoCreateTaskStatus::LoadFields(CGexMoTaskStatus *ptTaskItem)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Create a new Status task GUI: load fields...");

    // Task Title
    LineEditTitle->setText(ptTaskItem->GetProperties()->title());

    // Type of Web site to create
    if(!ptTaskItem->GetProperties()->isOneWebPerDatabase())
        comboBoxWebStructure->setCurrentIndex(0);	// One Web merging all database status info
    else
        comboBoxWebStructure->setCurrentIndex(1);	// One childWeb per database

    // Intranet file
    LineEditPath->setText(ptTaskItem->GetProperties()->intranetPath());

    // Home page name
    LineEditHomePage->setText(ptTaskItem->GetProperties()->homePage());

    // Report's URL name, use to display in Emails if http URL is empty (hyperlink)
    LineEditHomeReportURL->setText(ptTaskItem->GetProperties()->reportURL());

    // Report's http URL name to display in Emails (hyperlink)
    LineEditHomeReportHttpURL->setText(ptTaskItem->GetProperties()->reportHttpURL());

}

void GexMoCreateTaskStatus::OnCancel(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "On cancel...");
    done(-1);
}
