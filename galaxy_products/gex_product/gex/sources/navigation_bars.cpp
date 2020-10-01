///////////////////////////////////////////////////////////
// ALL Browser navigation bar messages handled here
///////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
#endif

#include "browser_dialog.h"
#include "browser.h"
#include "script_wizard.h"
#include "settings_dialog.h"
#include "drillDataMining3D.h"
#include "gex_web.h"	// ExaminatorWEB
#include "report_build.h"
#include "report_options.h"
#include "gex_constants.h"
#include "gex_skins.h"
#include "gex_shared.h"
#include "gex_report.h"
#include "db_transactions.h"
#include "temporary_files_manager.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "admin_engine.h"
#include <qpixmap.h>
#include <QDesktopServices>
#include "message.h"
#include "command_line_options.h"
#include <QMessageBox>

// build_report.cpp
extern CGexReport*	gexReport;			// Handle to report class
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

///////////////////////////////////////////////////////////
// Empty function, does nothing: needed to fill unused pointers
// in array of functions to call based on lins clicked.
///////////////////////////////////////////////////////////
void GexMainwindow::DoNothing(void)
{
}

///////////////////////////////////////////////////////////
// Display ToolBox home page
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb(void)
{
    // Update the URL edit box
    QString strString = GEX_HTMLPAGE_HOME_TOOLBOX;
    AddNewUrl(strString);

    // HTML skin pages Sub-folder .
    strString = navigationSkinDir();
    strString += "pages/";
    strString += GEX_HTMLPAGE_HOME_TOOLBOX;

    LoadUrl(strString);
}

///////////////////////////////////////////////////////////
// HOME link pressed
///////////////////////////////////////////////////////////
void GexMainwindow::HomeLink(void)
{
    // Keep track of wizard page selected
    iWizardPage = -1;

    // Updates page in browser window
    OnHome();
}

///////////////////////////////////////////////////////////
// Open a file with the correct application
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_OpenReportFile(const QString &strFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg( strFileName).toLatin1().constData() );

    // Do not open report file if running in hidden mode
    if(!GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden())
        GexMainwindow::OpenFileInEditor(strFileName, GexMainwindow::eDocUnknown);

    //    if (ReportOptions.GetOption("preferences","use_os_default").toBool() && QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(strFileName)))) //Case 4619 - Use OS's default program to open file
    //      return;

    //#ifdef _WIN32
    //    // Make file path 'windows' compatible (no '/', only '\\'...otherwise, Excel won't start!).
    //    QString strString;
    //    strString = strFileName;
    //    strString = strString.replace("/","\\");	// Replace '/' with '\\'
    //    // Launch excel
    //    ShellExecuteA(NULL,
    //           "open",
    //           strString.toLatin1().constData(),
    //           NULL,
    //           NULL,
    //           SW_SHOWNORMAL);
    //#else
    //    char	szString[2048];
    //    QString	strReportViewer;

    //    // If Report to open is a PDF file...launch PDF Viewer: acroread!
    //    if(strFileName.endsWith(".pdf",false) == true)
    //    {
    //        // Check the PDF Viewer assigned...
    //#ifdef __linux__
    //        // Check if environment variable is set
    //        char *ptEnv = getenv("GEX_PDF_VIEWER");
    //        if(ptEnv)
    //            strReportViewer = ptEnv;
    //        else
    //            // if no system variable, use the one in ReportOptions
    //            strReportViewer = ReportOptions.GetOption("preferences","pdf_editor").toString(); //ReportOptions.m_PrefMap["pdf_editor"];
    //#else
    //        strReportViewer = "acroread";	// Windows & Solaris PDF reader
    //#endif
    //    }
    //    else
    //    // If Report to open is a CSV file...launch OpenOffice Calc (if linux)!
    //    if(strFileName.endsWith(".csv",false) == true)
    //    {
    //#ifdef __linux__
    //        strReportViewer = ReportOptions.GetOption("preferences","ssheet_editor").toString();	//ReportOptions.m_PrefMap["ssheet_editor"];
    //#else
    //        // Use text editor
    //        strReportViewer = ReportOptions.GetOption("preferences", "text_editor").toString();	//ReportOptions.strTextEditor;
    //#endif
    //    }
    //    else
    //        strReportViewer = ReportOptions.GetOption("preferences", "text_editor").toString(); //ReportOptions.strTextEditor;

    //    sprintf(szString,"%s %s&",
    //        strReportViewer.toLatin1().constData(),
    //        strFileName.toLatin1().constData());
    //  if (system(szString) == -1) {
    //    //FIXME: send error
    //  }
    //#endif
}

///////////////////////////////////////////////////////////
// Reopen a non HTML report (launch appropriate application)
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_OpenReport(void)
{
    // No report available...
    if(gexReport == NULL)
        return;

    Wizard_OpenReportFile(gexReport->reportAbsFilePath());
}



///////////////////////////////////////////////////////////
// Data link pressed
///////////////////////////////////////////////////////////
void GexMainwindow::DataLink(void)
{
    bool	bIgnoreDatabase=true;
    QString strFilesPage = GEX_BROWSER_ACTIONLINK;

    // Make sure HTML browser is visible (so it can process our new link)...
    // Show Web Browser, hide assistants
    ShowHtmlBrowser(true);

    // Check if this Examinator product supports the 'Database' mode
    if(GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
    {
        // ExaminatorMonitoring or DB or Web: Let's check which 'Files' page to display...depends of report type!
        switch(mWizardType)
        {
            case GEX_ONEFILE_WIZARD:
                // Report type is 'Single file'
                strFilesPage += GEX_BROWSER_ONEFILE_WIZARD;
                bIgnoreDatabase = true;
                break;

            case GEX_CMPFILES_WIZARD:
                // Report type is 'Compare X files'
                strFilesPage += GEX_BROWSER_CMPFILES_WIZARD;
                bIgnoreDatabase = true;
                break;

            case GEX_SHIFT_WIZARD:
                // Report type is 'Shift analysis'
                strFilesPage += GEX_BROWSER_SHIFT_WIZARD;
                bIgnoreDatabase = true;
                break;

            case GEX_ADDFILES_WIZARD:
                // Report type is 'merge file'
                strFilesPage += GEX_BROWSER_ADDFILES_WIZARD;
                bIgnoreDatabase = true;
                break;

            case GEX_MIXFILES_WIZARD:
                // Report type is 'merge and/or compare file'
                strFilesPage += GEX_BROWSER_MIXFILES_WIZARD;
                bIgnoreDatabase = true;
                break;

            case GEX_ONEQUERY_WIZARD:
                // Report type is 'Single Query'
                strFilesPage += GEX_BROWSER_ONEQUERY_WIZARD;
                bIgnoreDatabase = false;
                break;

            case GEX_MIXQUERY_WIZARD:
                // Report type is 'Single Query'
                strFilesPage += GEX_BROWSER_MIXQUERY_WIZARD;
                bIgnoreDatabase = false;
                break;

            case GEX_CMPQUERY_WIZARD:
                // Report type is 'Single Query' ?
                strFilesPage += GEX_BROWSER_CMPQUERY_WIZARD;
                bIgnoreDatabase = false;
                break;

            case GEX_SQL_QUERY_WIZARD:
                // Report type is 'SQL Prodcution reports/Templates and SQL GUI'
                strFilesPage += GEX_BROWSER_SQL_QUERY_WIZARD;
                bIgnoreDatabase = false;
                break;

            case GEX_CHAR_QUERY_WIZARD:
                // Report type is 'Characterization'
                strFilesPage += GEX_BROWSER_CHAR_QUERY_WIZARD;
                bIgnoreDatabase = false;
                break;

            case GEX_FT_PAT_FILES_WIZARD:
                // Report type is 'FT PAT report'
                strFilesPage += GEX_BROWSER_FT_PAT_FILES_WIZARD;
                bIgnoreDatabase = false;
                break;

            default:
                GEX_ASSERT(false);
                GSLOG(SYSLOG_SEV_WARNING, "Invalid wizard type");
                break;
        }

        // If No database available...display the HTML message instead...unless we don't care (when selecting files for example)
        if((bIgnoreDatabase == false) && (ReloadDatabasesList(false) == 0))
        {
            // No report available

            // Display relevant message
            if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() ||
                    GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
                    GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() ||
                    GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
            {
                strFilesPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
                strFilesPage += GEX_HELP_FOLDER;
                strFilesPage += GEX_HTMLPAGE_DB_NODATABASE;
            }
        }
    }
    else
    {
        // Examinator product ONLY working on files (no support for database): Let's check which 'Files' page to display...depends of report type!
        switch(mWizardType)
        {
        default:
        case GEX_ONEFILE_WIZARD:
            // Report type is 'Single file'
            strFilesPage += GEX_BROWSER_ONEFILE_WIZARD;
            break;

        case GEX_CMPFILES_WIZARD:
            // Report type is 'Compare X files'
            strFilesPage += GEX_BROWSER_CMPFILES_WIZARD;
            break;

        case GEX_SHIFT_WIZARD:
            // Report type is 'Shift analysis'
            strFilesPage += GEX_BROWSER_SHIFT_WIZARD;
            break;

        case GEX_ADDFILES_WIZARD:
            // Report type is 'merge file'
            strFilesPage += GEX_BROWSER_ADDFILES_WIZARD;
            break;

        case GEX_MIXFILES_WIZARD:
            // Report type is 'merge and/or compare file'
            strFilesPage += GEX_BROWSER_MIXFILES_WIZARD;
            break;

        case GEX_FT_PAT_FILES_WIZARD:
            // Report type is 'FT PAT report'
            strFilesPage += GEX_BROWSER_FT_PAT_FILES_WIZARD;
        }
    }
    LoadUrl(strFilesPage);
}

void GexMainwindow::ViewSettings(void)
{
    // Keep track of wizard page selected
    iWizardPage = GEX_WIZARD_SETTINGS;
    if (    pWizardSettings
         && pWizardSettings->comboAdvancedReport->currentData() == GEX_ADV_SHIFT
         && pWizardSettings->comboBoxOutputFormat->currentIndex() == GEX_SETTINGS_OUTPUT_INTERACTIVE)
    {
        QMessageBox::warning(this, "Can't display the Shift report",
                                    "You have chosen to generate a Shift Report. This report is displayed in\n\
the More Reports section of the Examinator Report which is not supported in the chosen mode.\n\
Please change the report Type to generate the Shift Report.");
    }


    QString strSettingsPage = GEX_BROWSER_ACTIONLINK;
    strSettingsPage += GEX_BROWSER_SETTINGS;

    LoadUrl(strSettingsPage);

    // If full report to be created (either first run or new files to process)
    // Then Erase the list of temporary files...
    // Reset HTML sections to create flag: ALL pages to create.
    if(iHtmlSectionsToSkip == 0)
    {
        // Will erase all temporary files created so far...unless
        // too few created (e.g: less than 10 in total)
        GS::Gex::Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck);
    }

}

///////////////////////////////////////////////////////////
// User clicked link to see 'On-line context help' page
///////////////////////////////////////////////////////////
void GexMainwindow::OnShowWizardHelp(void)
{
    QString	strString;

    // Make sure HTML browser is visible (so it can process our new link)...
    // Show Web Browser, hide assistants
    ShowHtmlBrowser(true);

    // Build HTML page path
    strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + GEX_HELP_FOLDER;

    // Let's check which Wizard help page to display...depends of report type!
    switch(mWizardType)
    {
    case GEX_ONEFILE_WIZARD:
    default:
        // Report type is 'Single file'
        strString += GEX_HTMLPAGE_HLP_ONEWIZ;
        break;

    case GEX_CMPFILES_WIZARD:
        // Report type is 'Compare X files'
        strString += GEX_HTMLPAGE_HLP_CMPWIZ;
        break;

    case GEX_SHIFT_WIZARD:
        // Report type is 'Shift analysis'
        strString += GEX_HTMLPAGE_HLP_CMPWIZ;
        break;

    case GEX_ADDFILES_WIZARD:
        // Report type is 'merge file'
        strString += GEX_HTMLPAGE_HLP_ADDWIZ;
        break;

    case GEX_MIXFILES_WIZARD:
        // Report type is 'merge and/or compare file'
        strString += GEX_HTMLPAGE_HLP_MIXWIZ;
        break;
    }
    // Activate 'HELP' tab on navigation bar!
    QString strTopBarPage = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
    strTopBarPage += GEX_BROWSER_HELP_TOPLINK;
    pageHeader->setSource(QUrl::fromLocalFile(strTopBarPage));

    LoadUrl(strString);
}


///////////////////////////////////////////////////////////
// Show What-If wizard page.
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_DrillWhatif(void)
{
    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eSzOEM)
    {
//    case GEX_DATATYPE_ALLOWED_SZ:			// OEM-Examinator for Credence SZ: doesn't support WHAT-IF
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
//        return;
    }

    // Show WhatIf page...
    ShowWizardDialog(GEX_DRILL_WHATIF_WIZARD_P1);
}

///////////////////////////////////////////////////////////
// Drill page invoked from HTML hyperlink
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_Drill3D(QString strLink, bool fromHTMLBrowser)
{
    bool	bDataAvailable = false;

    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    if(gexReport == NULL)
    {
        bDataAvailable = false;
    }
    else
    {

        if(gexReport->getReportOptions() == NULL)
        {
            bDataAvailable = false;
        }
        else
        {
            // Set endering mode
            pWizardSettings->iDrillType = GEX_DRILL_WAFERPROBE;
            pWizardSettings->iDrillSubType = pWizardSettings->comboWafer->currentIndex();

            // Show Drill page...if any Drill assistant is active
            ShowWizardDialog(GEX_DRILL_3D_WIZARD_P1, fromHTMLBrowser);

            bDataAvailable = LastCreatedWizardWafer3D()->isDataAvailable();
        }
    }

    // If no data yet...tell user!
    if(bDataAvailable == false)
    {
        GS::Gex::Message::information(
            "", "No data available yet\nSelect your data first ('Home' page)!");

        // Switch to the home page
        OnHome();
        return;
    }

    strLink = strLink;
    // Show 3D plots.
    if(LastCreatedWizardWafer3D())
        LastCreatedWizardWafer3D()->ShowChart(strLink);

    mCanCreateNewDetachableWindow = true;

}

// Needed as table of addresses to function is only of type (void) for parameters...
void GexMainwindow::Wizard_Drill3D(void)
{
    if(LastCreatedWizardWafer3D() == 0)
        Wizard_Drill3D("#_gex_drill--drill_3d=wafer_sbin--g=0--f=0");
    else
        ShowWizardDialog(GEX_DRILL_3D_WIZARD_P1 );
}

void GexMainwindow::ReportLink(void)
{
    if(pWizardScripting == 0)
        return;

    // Shows the report page...or a message if no report built yet!
    QString	strString;

    // Keep track of wizard page selected
    iWizardPage = GEX_BROWSER;

    // Show report...if any!
    if((gexReport == NULL) || (gexReport->reportAbsFilePath().isEmpty())
            || gexReport->isCompleted() == false)
    {
        // No report available
        strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strString += GEX_HELP_FOLDER;

        // Display relevant HTML page (if running Examinator or ExaminatorDB orWeb)
        int nProductId = GS::LPPlugin::ProductInfo::getInstance()->getProductID();
        switch(nProductId)
        {
            case GS::LPPlugin::LicenseProvider::eExaminatorPAT:         // ExaminatorDB
            case GS::LPPlugin::LicenseProvider::eExaminatorPro:
            case GS::LPPlugin::LicenseProvider::eYieldMan:              // Yield-Man
            case GS::LPPlugin::LicenseProvider::eYieldManEnterprise:
            case GS::LPPlugin::LicenseProvider::ePATMan:                // PAT-Man
            case GS::LPPlugin::LicenseProvider::ePATManEnterprise:
                if((gexReport != NULL) && gexReport->isCompleted() && (gexReport->getGroupsList().count() > 0))
                    strString += GEX_HTMLPAGE_DB_EMPTYQUERY;        // Query result is Empty!
                else
                    strString += GEX_HTMLPAGE_DB_NOREPORT;          // No report yet
                break;

            default:
                if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
                    strString += GEX_HTMLPAGE_NOREPORT_LTXC;
                else
                    strString += GEX_HTMLPAGE_NOREPORT;
                break;
        }

        // If ExaminatorWEB is running, write in status file the URL to display in client Web Browser
        if(mGexWeb != NULL)
            mGexWeb->UrlStatus(strString);

        // Update HTML page displayed.
        LoadUrl( strString );
        return;
    }

    // Report exists...show the specified page!
    QString strShowPage;

    // QString of=ReportOptions.GetOption("output", "format").toString();
    QString strReportFormat = CGexReport::reportFormat(gexReport->reportAbsFilePath());
    if(strReportFormat=="CSV")	//(gexReport->getReportOptions()->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Report created in CSV format!
        strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strString += GEX_HELP_FOLDER;
        strString += GEX_HTMLPAGE_CSVFILE;
        LoadUrl( strString );

        // Update navigation tab to highlight the 'Report' tab.: _Gex_ + page name
        strString = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
        strString += GEX_BROWSER_REPORT_TOPLINK;
        pageHeader->setSource(QUrl::fromLocalFile(strString));

        // If ExaminatorWEB is running, write in status file the URL to display in client Web Browser
        if(mGexWeb != NULL)
            mGexWeb->UrlStatus(gexReport->reportAbsFilePath());
    }

    if(strReportFormat=="DOC" || strReportFormat=="ODT")	//(gexReport->getReportOptions()->iOutputFormat == GEX_OPTION_OUTPUT_WORD)
    {
        // Report created in WORD format!
        strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strString += GEX_HELP_FOLDER;

        switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
        {
//        case GEX_DATATYPE_GEX_YIELD123:
//            strString += GEXY123_HTMLPAGE_WORDFILE;
//            break;
        default:
            // Examinator
            strString += strReportFormat=="DOC"?GEX_HTMLPAGE_WORDFILE:GEX_HTMLPAGE_ODTFILE;
            break;
        }

        LoadUrl( strString );

        // Update navigation tab to highlight the 'Report' tab.: _Gex_ + page name
        strString = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
        strString += GEX_BROWSER_REPORT_TOPLINK;
        pageHeader->setSource(QUrl::fromLocalFile(strString));

        // If ExaminatorWEB is running, write in status file the URL to display in client Web Browser
        if(mGexWeb != NULL)
            mGexWeb->UrlStatus(gexReport->reportAbsFilePath());
    }
    else
        if(strReportFormat=="PPT")	//(gexReport->getReportOptions()->iOutputFormat == GEX_OPTION_OUTPUT_PPT)
        {
            // Report created in PowerPoint format!
            strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
            strString += GEX_HELP_FOLDER;

            switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
            {
//            case GEX_DATATYPE_GEX_YIELD123:
//                strString += GEXY123_HTMLPAGE_PPTFILE;
//                break;

            default:
                // Examinator
                strString += GEX_HTMLPAGE_PPTFILE;
                break;
            }

            LoadUrl( strString );

            // Update navigation tab to highlight the 'Report' tab.: _Gex_ + page name
            strString = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
            strString += GEX_BROWSER_REPORT_TOPLINK;
            pageHeader->setSource(QUrl::fromLocalFile(strString));

            // If ExaminatorWEB is running, write in status file the URL to display in client Web Browser
            if(mGexWeb != NULL)
                mGexWeb->UrlStatus(gexReport->reportAbsFilePath());
        }
        else
            if(strReportFormat=="PDF")	//(gexReport->getReportOptions()->iOutputFormat == GEX_OPTION_OUTPUT_PDF)
            {
                // Report created in PDF format!
                strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
                strString += GEX_HELP_FOLDER;

                switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
                {
//                case GEX_DATATYPE_GEX_YIELD123:
//                    strString += GEXY123_HTMLPAGE_PDFFILE;
//                    break;

                default:
                    // Examinator
                    strString += GEX_HTMLPAGE_PDFFILE;
                    break;
                }

                LoadUrl( strString );

                // Update navigation tab to highlight the 'Report' tab.: _Gex_ + page name
                strString = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
                strString += GEX_BROWSER_REPORT_TOPLINK;
                pageHeader->setSource(QUrl::fromLocalFile(strString));

                // If ExaminatorWEB is running, write in status file the URL to display in client Web Browser
                if(mGexWeb != NULL)
                    mGexWeb->UrlStatus(gexReport->reportAbsFilePath());
            }
            else
                if(strReportFormat=="INTERACTIVE")	//(gexReport->getReportOptions()->iOutputFormat == GEX_OPTION_OUTPUT_INTERACTIVEONLY)
                {
                    // NO Report created: Interactive mode only!
                    strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
                    strString += GEX_HELP_FOLDER;
                    strString += GEX_HTMLPAGE_INTERACTIVEONLY;
                    LoadUrl( strString );

                    // Update navigation tab to highlight the 'Report' tab.: _Gex_ + page name
                    strString = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
                    strString += GEX_BROWSER_REPORT_TOPLINK;
                    pageHeader->setSource(QUrl::fromLocalFile(strString));

                    // If ExaminatorWEB is running, write in status file the URL to display in client Web Browser
                    if(mGexWeb != NULL)
                        mGexWeb->UrlStatus(gexReport->reportAbsFilePath());
                }
                else
                {
                    // Check specific page to load at startup
                    strShowPage = gexReport->reportAbsFilePath();

                    // Check if it is a GEX action page...or not...
                    QString gexAction = pWizardScripting->strShowReportPage.section(GEX_BROWSER_ACTIONLINK,1);

                    if(gexAction.isEmpty() == false)
                    {
                        // Not a page to show, but an Action to perform!
                        strShowPage = pWizardScripting->strShowReportPage;

                        // Switch back to Assistants instead of HTML browser!
                        ShowHtmlBrowser(false);
                    }
                    else
                    {
                        // Standard HTML page to show...see which one!
                        int i = strShowPage.lastIndexOf( '/' );     // find where '/index.htm' is in string name!
                        strShowPage.truncate(i);				// Keep path to report, need HTML page to specify...


                        if(pWizardScripting->strShowReportPage == GEX_BROWSER_ACT_DRILL)
                            strShowPage = strShowPage + "/" + GEX_BROWSER_ACTIONLINK + GEX_BROWSER_DRILL_3D;
                        else if(pWizardScripting->strShowReportPage == GEX_BROWSER_ACT_DRILL_CHART)
                            strShowPage = strShowPage + "/" + GEX_BROWSER_ACTIONLINK + GEX_BROWSER_DRILL_CHART;
                        else if(pWizardScripting->strShowReportPage == GEX_BROWSER_ACT_DRILL_TABLE)
                            strShowPage = strShowPage + "/" + GEX_BROWSER_ACTIONLINK + GEX_BROWSER_DRILL_TABLE;
                        else if(pWizardScripting->strShowReportPage == GEX_BROWSER_ACT_WHATIF)
                            strShowPage = strShowPage + "/" + GEX_BROWSER_ACTIONLINK + GEX_BROWSER_DRILL_WHATIF;
                        else if(pWizardScripting->strShowReportPage == "settings")
                            strShowPage = strShowPage + "/" + GEX_BROWSER_ACTIONLINK + GEX_BROWSER_SETTINGS;
                        else if(pWizardScripting->strShowReportPage == "global")
                            strShowPage += "/pages/global.htm";
                        else if(pWizardScripting->strShowReportPage == "statistics")
                            strShowPage += "/pages/stats.htm";	// Stats: table of contents
                        else if(pWizardScripting->strShowReportPage == "statistics_1st")
                            strShowPage += "/pages/stats1.htm";	// Stats: 1st page with table of stats
                        else if(pWizardScripting->strShowReportPage == "histogram")
                            strShowPage += "/pages/histogram.htm";// Histograms: table of contents
                        else if(pWizardScripting->strShowReportPage == "histogram_1st")
                            strShowPage += "/pages/histogram1.htm";// Histograms: 1st page with table of histograms
                        else if(pWizardScripting->strShowReportPage == "wafer"
                                && !(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(
                                           GS::LPPlugin::ProductInfo::waferMap)))
                            strShowPage += "/pages/wafermap.htm";
                        else if(pWizardScripting->strShowReportPage == "binning")
                            strShowPage += "/pages/binning.htm";
                        else if(pWizardScripting->strShowReportPage == "pareto")
                            strShowPage += "/pages/pareto.htm";
                        else if(pWizardScripting->strShowReportPage == "advanced")
                            strShowPage += "/pages/advanced.htm";
                        else if(pWizardScripting->strShowReportPage == "real_html")
                            strShowPage = gexReport->reportAbsFilePath(); // Keep full report name
                        else if(pWizardScripting->strShowReportPage == "interactive_chart")
                        {
                            Wizard_DrillChart("");
                            return;
                        }
                        else if(pWizardScripting->strShowReportPage == "interactive_table")
                        {
                            Wizard_DrillTable("");
                            return;
                        }
                        else if(pWizardScripting->strShowReportPage == "admin")
                        {
                            Wizard_AdminGui();
                            return;
                        }
                        else
                        {
                            strShowPage += "/index.htm";

                            // Unselect all tabs on navigation bar!
                            QString gexPageName = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
                            gexPageName += GEX_BROWSER_REPORT_TOPLINK;

                            // GCORE-7369 : reload top page correctly if OEM
                            ReloadHeaderPage( QUrl::fromLocalFile( gexPageName ) );
                        }
                    }
                }

    if(pWizardScripting->strShowReportPage == "what_if")
        strShowPage = QString(GEX_BROWSER_ACTIONLINK) + GEX_BROWSER_DRILL_WHATIF;

    // Erase temporary home page link so 'Home' button is back to normal home page
    pWizardScripting->strShowReportPage = "";

    // If ExaminatorWEB is running, write in status file the URL to display in client Web Browser
    if(mGexWeb != NULL)
        mGexWeb->UrlStatus(strShowPage);

    // Show report starting on page of interest!
    LoadUrl(strShowPage);
}

void GexMainwindow::ViewOptions(void)
{
    QString strOptionsPage = GEX_BROWSER_ACTIONLINK;
    strOptionsPage += GEX_BROWSER_OPTIONS_WIZARD;
    LoadUrl(strOptionsPage);
}

///////////////////////////////////////////////////////////
// HELP link pressed in browser: call 'top bar help' + reload
// home page.
///////////////////////////////////////////////////////////
void GexMainwindow::HelpLink(void)
{
    Wizard_Assistant();
}

///////////////////////////////////////////////////////////
// Footer HTML page highlighted...
///////////////////////////////////////////////////////////
void GexMainwindow::OnHighlightFooterLinks(QString /*url*/)
{
    // Display comment in the Status bar
    statusBar()->showMessage("All about Quantix!");
}

///////////////////////////////////////////////////////////
// Footer HTML page clicked...
///////////////////////////////////////////////////////////
void GexMainwindow::OnFooterLinks(const QUrl& /*url*/)
{
    // Reload footer HTML page (not Header navigation bar).
    LoadNavigationTabs(GS::LPPlugin::ProductInfo::getInstance()->getProductID(),false,true);

    // Update Browser center page.
    QString strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    strString += GEX_HELP_FOLDER;
    strString += GEX_HTMLPAGE_HLP_FOOTER;
    LoadUrl( strString );
}

///////////////////////////////////////////////////////////
// Open a bundled pdf file
///////////////////////////////////////////////////////////
void GexMainwindow::OpenBundledPdf(QString fileName)
{
    QString strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    strString += GEX_HELPIMAGES_FOLDER;
    strString += fileName;
    Wizard_OpenReportFile(strString);
}

///////////////////////////////////////////////////////////
// Open the bundled copyright and licence pdf
///////////////////////////////////////////////////////////
void GexMainwindow::OpenCopyrightLicense(void)
{
    OpenBundledPdf(COPYRIGHT_LICENSE_PDF_FILE);
}

///////////////////////////////////////////////////////////
// Open the bundled quick start guide pdf
///////////////////////////////////////////////////////////
void GexMainwindow::OpenExamQuickStart(void)
{
    OpenBundledPdf(EXAM_QUICK_START_FILE);
}

///////////////////////////////////////////////////////////
// Open the bundled installation pdf
///////////////////////////////////////////////////////////
void GexMainwindow::OpenExamInstall(void)
{
    OpenBundledPdf(EXAM_INSTALL_FILE);
}

///////////////////////////////////////////////////////////
// Open the bundled user license pdf
///////////////////////////////////////////////////////////
void GexMainwindow::OpenUserLicense(void)
{
    OpenBundledPdf(USER_LICENSE_FILE);
}

///////////////////////////////////////////////////////////
// Open the bundled release highlights pdf
///////////////////////////////////////////////////////////
void GexMainwindow::OpenRH(void)
{
    OpenBundledPdf(RH_FILE);
}

///////////////////////////////////////////////////////////
// Open the bundled installation pdf
///////////////////////////////////////////////////////////
void GexMainwindow::OpenTerInstall(void)
{
    OpenBundledPdf(TER_INSTALL_FILE);
}

///////////////////////////////////////////////////////////
// Open the bundled release highlights pdf
///////////////////////////////////////////////////////////
void GexMainwindow::OpenTerRH(void)
{
    OpenBundledPdf(TER_RH_FILE);
}

///////////////////////////////////////////////////////////
// Open the bundled quick start guide pdf
///////////////////////////////////////////////////////////
void GexMainwindow::OpenYMQuickStart(void)
{
    OpenBundledPdf(YM_QUICK_START_FILE);
}

///////////////////////////////////////////////////////////
// Open the bundled installation pdf
///////////////////////////////////////////////////////////
void GexMainwindow::OpenYMInstall(void)
{
    OpenBundledPdf(YM_INSTALL_FILE);
}
