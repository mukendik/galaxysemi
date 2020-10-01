#include "browser_dialog.h"
#include "gex_report.h"
#include "gex_word_report.h"
#include "db_engine.h"
#include "product_info.h"
#include <gqtl_log.h>
#include "engine.h"
#include <QMessageBox>
#include "message.h"

extern GexMainwindow *	pGexMainWindow;

int	CGexReport::ConvertHtmlToWord(void)
{
    CGexWordReport	clWordReport;
    QString			strDestination;
    QString			strHtmlReportFolder;
    QDir			cDir;
    int				nStatus=0;

    // HTML flat file is <path>/<query_folder_name>/pages/indexf.htm,
    // Word output must be: <path>/<query_folder_name>.doc
    strDestination	= BuildTargetReportName(strHtmlReportFolder,".doc");

    cDir.remove(strDestination);	// Make sure destination doesn't exist.

    // Retrieve Word Options
    GexWordOptions	stGexWordOptions;

    // Show progress bar & messages?
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        stGexWordOptions.m_bShowProgressBar = false;

    // Show Word?
    stGexWordOptions.m_bShowWordApplication = true;	// show/hide WORD
    stGexWordOptions.m_bMinimizeApplication = true; // normal size / minimized

    // Paper size
    /*
    switch(pReportOptions->iPaperSize)
    {
        case GEX_OPTION_PAPER_SIZE_A4:			// Page size is European A4
            stGexWordOptions.m_nPaperFormat = GexWordOptions::ePaperFormatA4;
            break;
        case GEX_OPTION_PAPER_SIZE_LETTER:		// Page size is US Letter
            stGexWordOptions.m_nPaperFormat = GexWordOptions::ePaperFormatLetter;
            break;
    }
    */
    QString ps=ReportOptions.GetOption("output", "paper_size").toString();
    if (ps=="A4")
        stGexWordOptions.m_nPaperFormat = GexWordOptions::ePaperFormatA4;
    if (ps=="letter")
        stGexWordOptions.m_nPaperFormat = GexWordOptions::ePaperFormatLetter;


    // Paper orientation: portrait or landscape.
    QString pf=ReportOptions.GetOption("output", "paper_format").toString();
    if (pf=="portrait") //if(pReportOptions->bPortraitFormat)
        stGexWordOptions.m_nPaperOrientation = GexWordOptions::ePaperOrientationPortrait;
    else
        stGexWordOptions.m_nPaperOrientation = GexWordOptions::ePaperOrientationLandscape;

    // Margins
    stGexWordOptions.m_lfMargin_Left = 0.2;	// in inches (default: 1.25);
    stGexWordOptions.m_lfMargin_Right = 0.2;	// in inches (default: 1.25);
//	stGexWordOptions.m_lfMargin_Top = 1;		// in inches (default: 1);
//	stGexWordOptions.m_lfMargin_Bottom = 1;	// in inches (default: 1);

    // Header & Footer: Examinator version + URL & Lot/Product info if known.
    BuildHeaderFooterText(stGexWordOptions.m_strHeader,stGexWordOptions.m_strFooter);

#ifndef GSDAEMON
    // Message box to be used if we fail to create the Word file (file locked)
    QMessageBox mb( GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
        "Failed to create Word document...maybe it's already in use?\nIf so, close MS-Word first then try again...",
        QMessageBox::Question,
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No  | QMessageBox::Escape,
        0,
        0 );
    mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    mb.setButtonText( QMessageBox::Yes, "&Try again" );
    mb.setButtonText( QMessageBox::NoAll, "&Cancel" );
#endif

#ifndef GSDAEMON
    build_word_file:
#endif
    // Generate Word document
    nStatus = clWordReport.GenerateDocFromHtml(pGexMainWindow, stGexWordOptions, reportFlatHtmlAbsPath(), strDestination);
    int r=1;

    switch(nStatus)
    {
        case CGexWordReport::Err_RemoveDestFile:

            if(stGexWordOptions.m_bShowProgressBar == false)
            {
                // Running Monitoring: then do not show dialog box, add message to log file instead...
                GS::Gex::Message::information(
                    "", "MS-Word already in use by another process.");
                break;
            }
            #ifndef GSDAEMON
                if(mb.exec() == QMessageBox::Yes)
                    goto build_word_file;	// Try again
            #endif
            break;

        case CGexWordReport::ConversionCancelled:	// Conversion process has been cancelled by the user
            GS::Gex::Message::information(
                "", "Word document generation cancelled.\n\n"
                "A MS-Word instance may still be active and "
                "needs to be closed.");
            break;

        case CGexWordReport::Err_ScriptError:		// Error during script execution (no Word file generated)
            GS::Gex::Message::information(
                "", "Failed to create Word document (script error).\n"
                "Check you have 'Microsoft Office' and 'MS-Word' installed!");
            r=ConvertHtmlToODT();
            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("ConvertHtmlToODT returned %1").arg(r).toLatin1().constData());
            break;

        default:
            GS::Gex::Message::information("Word document",
                                          "Error creating Word document.");
            break;

        case CGexWordReport::NoError:	// No error
            break;
    }


    // Cleanup: erase the HTML folder created for the flat HTML file.
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteFolderContent(strHtmlReportFolder);

    // Update the report file name created (from HTML file name to '.doc' file just created
    setLegacyReportName(strDestination);

    return r;
}


