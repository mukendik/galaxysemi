///////////////////////////////////////////////////////////
// ALL Browser wizard pages for 'Assistant Wizard': NEPTUS
// and
// Flying Neptus: the context help!
#include <time.h>
#include <stdlib.h>
#include <QProcess>
#include <QDesktopServices>
#include <gqtl_sysutils.h>

#include "engine.h"
#include "command_line_options.h"
#include "browser_dialog.h"
#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "script_wizard.h"
#include "assistant_flying.h"
#include "gex_constants.h"
#include "gex_version.h"
#include "gex_shared.h"
#include "gex_skins.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "message.h"

// in gex_pixmap.h
extern  char * pixmap_FlyNeptusIdle[];
extern char * pixmap_FlyNeptusHelp[];

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

///////////////////////////////////////////////////////////
// Flying Neptus to Show-up
///////////////////////////////////////////////////////////
void GexMainwindow::OnContextualHelp(void)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("No user manual supported in V%1.%2. Please use http://support.galaxysemi.com.")
          .arg(GEX_APP_VERSION_MAJOR)
          .arg(GEX_APP_VERSION_MINOR).toLatin1().constData());
    return;

    // User Manual keys
    QString strPDFDocFilePrefix;
    QString strPDFDocFileSuffix = QString("V") + QString::number(GEX_APP_VERSION_MAJOR) +QString(".") + QString::number(GEX_APP_VERSION_MINOR);

    // If Examinator OEM for the Credence tester, do not show Help assistant!
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eSzOEM:
        case GS::LPPlugin::LicenseProvider::eGTM:
            return;
        case GS::LPPlugin::LicenseProvider::ePATMan:
        case GS::LPPlugin::LicenseProvider::ePATManEnterprise:
            strPDFDocFilePrefix = QString("PM_");
            break;
        case GS::LPPlugin::LicenseProvider::eYieldMan:
        case GS::LPPlugin::LicenseProvider::eYieldManEnterprise:
            strPDFDocFilePrefix = QString("YM_");
            break;
        default:
            strPDFDocFilePrefix = QString("GEX_");
            break;
    }

    // Run the Gex User Manual
    QString		strSvgFolder        = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/help/pages/";
    QString		strWorkingFolder	= GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    QString		strPDFDocFile       = strWorkingFolder + "/"+ strPDFDocFilePrefix +"UserManual_"+ strPDFDocFileSuffix +".pdf";	// !! strWorkingFolder doesn't end with '/' (08/07/2010)
    QString		strReferenceDocFile = strSvgFolder + "_gexstd_dbgcore.lib";
    QString		strMessage;

    CGexSystemUtils::NormalizePath(strPDFDocFile);
    CGexSystemUtils::NormalizePath(strReferenceDocFile);
    CGexSystemUtils::NormalizePath(strWorkingFolder);

    // Trace message
    strMessage = "Checking for user manual files: " + strPDFDocFile + ", " + strReferenceDocFile + ".\n";
    GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data());

    if ( (!QFile::exists(strPDFDocFile)) && (!QFile::exists(strReferenceDocFile)) )
    {
        GS::Gex::Message::warning("Quantix User Manual",
                                  "Could not find the user manual.\n\n"
                                  "Please contact Quantix support at " +
                                  QString(GEX_EMAIL_SUPPORT)+".");
        return;
    }

    if (!QFile::exists(strPDFDocFile))
        QFile::copy(strReferenceDocFile,strPDFDocFile);

    strPDFDocFile = "file:///" + strPDFDocFile;

    if (!QDesktopServices::openUrl(QUrl(strPDFDocFile)))
    {
        strMessage =  "Error opening user manual.\n";
        strMessage += "Make sure you have a PDF reader installed on your system,\n";
        strMessage += "and that the .pdf extension is associated with your PDF reader.\n\n";
        strMessage += "If the problem persists, please contact Quantix support at "+QString(GEX_EMAIL_SUPPORT)+".";
        GS::Gex::Message::warning("Quantix User Manual", strMessage);
        return;
    }
}

///////////////////////////////////////////////////////////
// WIZARD PAGE
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_Assistant(void)
{
    QString	strString, strHelpPage;

    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
    {
        strHelpPage = QString("%1pat_help.htm").arg(GEX_HELP_FOLDER);
    }
    else if(GS::LPPlugin::ProductInfo::getInstance()->isYieldMan())
    {
        strHelpPage = QString("%1ym_help.htm").arg(GEX_HELP_FOLDER);
    }
    else if(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
    {
       strHelpPage = QString("%1pat_help.htm").arg(GEX_HELP_FOLDER);
    }
    else if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
    {
        strHelpPage = QString("%1gtm_help.htm").arg(GEX_HELP_FOLDER);
    }
    else if(
            GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus()
            || GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetProduct() == GS::LPPlugin::LicenseProvider::eTerOEM)
    {
        strHelpPage = QString("%1gexter_help.htm").arg(GEX_HELP_FOLDER);
    }
    else
    {
        strHelpPage = QString("%1gexpro_help.htm").arg(GEX_HELP_FOLDER);
    }

    strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + strHelpPage;

    // Display the page specified by the Neptus Assitant
    LoadUrl(strString);

    // Update neptus
    strString = GEX_BROWSER_ACTIONBOOKMARK;
    strString += GEX_BROWSER_ASSISTANT_WIZARD;
    AddNewUrl(strString);
}

