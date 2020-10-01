#include <QMessageBox>
#include <QScriptValue>
#include <QProgressBar>

#include "browser.h"
#include "browser_dialog.h"
#include "db_transactions.h"
#include "engine.h"
#include "command_line_options.h"
#include "script_wizard.h"	// CGexScripting
#include "settings_dialog.h"	// GexSettings
#include "gex_pixmap_extern.h"	// pixStandalone,...
#include "gex_options_handler.h"
#include "gex_scriptengine.h"
#include "db_engine.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include "onefile_wizard.h"	// GexOneFileWizard
#include "scripting_io.h"	// WaitUntil...
#include "options_center_widget.h"
#include "reports_center_widget.h"
#include "report_options.h"
#include "scheduler_engine.h"
#include "mo_task.h"
#include "product_info.h"
#include "libgexpb.h"
#include "gtm_mainwidget.h"
#include "csl/csl_engine.h"
#include "license_provider_profile.h"
#include "gex_version.h"
#include "message.h"
#include <gqtl_skin.h>

extern CGexSkin*                pGexSkin;			// holds skin settings
extern CReportOptions           ReportOptions;		// Holds options (report_build.h)
extern QProgressBar  *          GexProgressBar; // Handle to progress bar in status bar
extern QLabel        *          GexScriptStatusLabel; // Handle to script status text in status bar

///////////////////////////////////////////////////////////
// GEX application ready: Load environment...
///////////////////////////////////////////////////////////
void GexMainwindow::OnEngineStarting()
{
    connect(&GS::Gex::CSLEngine::GetInstance(), SIGNAL(scriptStarted(QString, bool)),
            this, SLOT(OnCSLScriptStarted()));
    connect(&GS::Gex::CSLEngine::GetInstance(), SIGNAL(scriptFinished(GS::Gex::CSLStatus)),
            this, SLOT(OnCSLScriptFinished(GS::Gex::CSLStatus)));
    connect(&GS::Gex::Engine::GetInstance(), SIGNAL(sProgressStatus(bool,int,int,bool)),
            this, SLOT(UpdateProgressStatus(bool,int,int,bool)));

    connect(&GS::Gex::Engine::GetInstance(), SIGNAL(sHideProgress()),
            this, SLOT(HideProgress()));
    connect(&GS::Gex::Engine::GetInstance(), SIGNAL(sLabelStatus(QString)),
            this, SLOT(UpdateLabelStatus(QString)));

    // build the path where get html file for navigation
    if (pGexSkin)
        strNavigationSkinDir = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + pGexSkin->path();
    else
        GSLOG(SYSLOG_SEV_WARNING, "cant set NavigationSkinDir");

    // Reload the HTML navigation bar (is difference between ExaminatorMonitoring and other modules)
    LoadNavigationTabs(GS::LPPlugin::ProductInfo::getInstance()->getProductID(),true,true);
    // Default 'Data' tab points to 'Single file analysis'...unless running ExaminatorDB
    SetWizardType(GEX_ONEFILE_WIZARD);
    // iWizardType= GEX_ONEFILE_WIZARD;		// PYC, 27/05/2011
    iWizardPage = -1;

    // If not running in Client mode, display immediatly
//    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_CLIENT)
//        ReloadProductSkin(true);	// Client node: show Splash screen while waiting server reply...
//    else
//        ReloadProductSkin(false);	// Standalone or Eval. No splash screen, directly load correct HTML page
    ReloadProductSkin(true);

    // Show Reports Center icon in toolbar if necessary
    if (m_pReportsCenter &&
            (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
             GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() ||
             GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus()))
        actionReportsCenter->setVisible(true);

    // Check if must show window
    if(GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden() == false)
        show();
    else
        hide();

    // If using very small screen relosulion (640x480), display warning (unless running in Hidden mode)
    if(iScreenSize == GEX_SCREENSIZE_SMALL)
        GS::Gex::Message::information(
            "", "Your screen resolution (640x480) is too low\n"
            "for a comfortable usage of the Examinator.\n"
            "Best resolution is 1024x768 or higher.");

    // Set text for the 'build report' button in Settings+Options page...
    BuildReportButtonTitle("Build report!");

    // Initial size & position at startup
    ResizeGexApplication(1);

    GSLOG(SYSLOG_SEV_INFORMATIONAL, " Main window on start end");
}

void GexMainwindow::OnEngineReady()
{
    ReloadProductSkin(false); // Reload correct HTML Home page + update application name

    if(pWizardSettings)
    {
        QString of=ReportOptions.GetOption("output", "format").toString();
        if (of=="HTML")
            pWizardSettings->comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_HTML);
        else 	if (of=="DOC")
            pWizardSettings->comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_WORD);
        else 	if (of=="CSV")
            pWizardSettings->comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_CSV);
        else 	if (of=="PPT")
            pWizardSettings->comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_PPT);
        else 	if (of=="PDF")
            pWizardSettings->comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_PDF);
        else 	if (of=="INTERACTIVE")
            // Interactive mode only (no report created)
            pWizardSettings->comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_INTERACTIVE);
        else 	if (of=="ODT")
            pWizardSettings->comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_ODT);
    }

    // Evaluation mode: propose to use Demo data file
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        QString strMessage =
            "Examinator includes a STDF demo data file!\n"
            "(with real production test data)\n\n"
            "Do you want to use it now?\n\n";

        bool lOk;
        GS::Gex::Message::request("", strMessage, lOk);
        if (lOk)
        {
            // Select Tutorial STDF file.
            QString strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/data_samples.std";
            pWizardOneFile->OnSelectFile(strString);

            // Select 'Files' tab in web bar.
            QString gexPageName = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
            gexPageName += GEX_BROWSER_FILES_TOPLINK;
            pageHeader->setSource(QUrl::fromLocalFile(gexPageName));

            // Directly go to the Settings page...
            Wizard_Settings();

            // Update navigation tab to highlight the 'Settings' tab.: _Gex_ + page name
            QString strTopBarPage = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
            strTopBarPage += GEX_BROWSER_SETTINGS_TOPLINK;
            pageHeader->setSource(QUrl::fromLocalFile(strTopBarPage));
        }
    }

    /*
    // Log levels are now driven by env var as GEXPB_LOGLEVEL,...
    */

    if(!m_pOptionsCenter)
    {
        UpdateLabelStatus("Loading Options Center...");

        // ready to build OptionsCenter with correct product/licence/modules
        QString r=LoadOptionsCenter();
        if(r.startsWith(QString("error"), Qt::CaseInsensitive))		// Exit application
        {
            GSLOG(SYSLOG_SEV_EMERGENCY, r.toLatin1().constData());
#           ifdef QT_DEBUG
            GS::Gex::Message::warning("", "Error loading OptionsCenter.");
#           else
            GS::Gex::Message::critical(
                        "", "Error loading OptionsCenter.\n\n"
                        "Please contact Quantix support at " +
                        QString(GEX_EMAIL_SUPPORT) + ".\n\nThe software will now exit!");
            GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
            return;
#           endif
        }


        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Load OptionsCenter result: %1").arg( r).toLatin1().constData());
        if (m_pOptionsCenter)
        {
            m_pOptionsCenter->hide();
            pScrollArea->layout()->addWidget(m_pOptionsCenter);
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, QString("OptionsCenter not loaded: %1 !").arg( r).toLatin1().constData());
    }

    UpdateLabelStatus("");

    // Need to add GTM widget?
#ifdef GCORE15334

    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
    {
        //if( !getenv("WILLIAM_TAMBELLINI") )
        {
            // Widget for GTM (Quantix Tester Monitor). Used for FT PAT.
            // Actually for the moment also do the TcpServer job. Cleanup in progress.
            mGtmWidget = new Gtm_MainWidget(centralWidget());
            if(mGtmWidget)
                pScrollArea->layout()->addWidget(mGtmWidget);
        }
    }
#endif
    // If data file given in argument, process it now
    if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetStartupDataFile().isEmpty() == false)
        onProcessDataFile(QStringList(GS::Gex::Engine::GetInstance().GetCommandLineOptions()
                                      .GetStartupDataFile()), true);
}

void GexMainwindow::OnLicenseGranted()
{
    // Keeps track of the number of connections attempted (so to only insert Widgets in GUI on first connection).
    static int iConnectionID = 0;

    if (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_CLIENT)
    {
        if (GexProgressBar)
            GexProgressBar->hide();
        if (GexScriptStatusLabel)
            GexScriptStatusLabel->hide();

        // Add 'Network' icon to the status bar...unless this is not the first connection attempt!
        if(!iConnectionID)
            AddStatusBarButton(pixNetwork);

        // Keep track of total number of connections performed
        iConnectionID++;
    }
    else if (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION ||
        GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_STANDALONE)
    {
        // Add 'Standalone' icon to the status bar
        AddStatusBarButton(pixStandalone);
    }


    ReloadProductSkin(false);
}
