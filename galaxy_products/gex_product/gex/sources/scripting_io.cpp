//////////////////////////////////////////////////////////
// File: scripting_io.cpp
// Includes mecanism so Script functions I/O messages
// are sent to the right console window
//////////////////////////////////////////////////////////

#include "scripting_io.h"
#include "script_wizard.h"
#include "browser.h"
#include "gex_web.h"	// ExaminatorWEB
#include "report_build.h"
#include "report_options.h"
#include <gqtl_log.h>
#include "product_info.h"

// main.cpp
extern CReportOptions	ReportOptions;	// Holds options (report_build.h)
static GexScripting		* pGexWizardConsole = NULL;

///////////////////////////////////////////////////////////
// Wait until script is completed (doesn't means execution was a success)
///////////////////////////////////////////////////////////
//void WaitUntilScriptCompleted(void)
//{
//    GSLOG(SYSLOG_SEV_DEBUG, "Wait Until Script Completed... ");
//    if(pGexMainWindow == NULL)
//        return;
//    if(pGexMainWindow->pWizardScripting == NULL)
//        return;

//    // Loop until script completed...
//    while(1)
//    {
//        qApp->processEvents();

//        if(!pGexMainWindow->pWizardScripting)
//            GSLOG(SYSLOG_SEV_ERROR, "No pWizardScripting!");

//        if(pGexMainWindow->pWizardScripting->IsScriptRunning() == false)
//            return;
//    };
//}

///////////////////////////////////////////////////////////
// Wait until script is completed or timeout (doesn't means execution was a success)
// return: 'true' if done 'false' if timout and script still not done
///////////////////////////////////////////////////////////
//bool WaitUntilScriptCompleted_Timeout(int /*iTimeout*/)
//{
//    if(pGexMainWindow == NULL)
//        return true;
//    if(pGexMainWindow->pWizardScripting == NULL)
//        return true;

//    // Compute timeout time = current time + timeout length (in msec.)
//    QTime cExpireTime = QTime::currentTime();
//    cExpireTime = cExpireTime.addMSecs(iTimeout);
//    // Loop until script completed...
//    while(1)
//    {
//        qApp->processEvents();

//        // Check for script completed
//        if(pGexMainWindow->pWizardScripting->IsScriptRunning() == false)
//            return true;

//        // Check for timeout
//        if(QTime::currentTime() > cExpireTime)
//            return false;
//    };

//    return true;
//}
///////////////////////////////////////////////////////////
// Scripting I/O init: saves handle to GUI application
///////////////////////////////////////////////////////////
void Scripting_IO_Init(GexScripting *pt)
{
    // Saves GUI pointer so we can later send messages
    // to the script window.
    pGexWizardConsole = pt;

    // Hide HTML script page
    pt->ShowHtmlConsole(false);

    // Also empty the HTML page...
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        pt->ShowHtmlPage(GEXMO_BROWSER_ELSE_TOPLINK);	// Give invalid HTML page to empty the window!
//    else if(GS::LPPlugin::ProductInfo::getInstance()->isToolBox())
//        pt->ShowHtmlPage(GEXTB_BROWSER_ELSE_TOPLINK);	// Give invalid HTML page to empty the window!
    else
        pt->ShowHtmlPage(GEX_BROWSER_ELSE_TOPLINK);	// Give invalid HTML page to empty the window!
}

///////////////////////////////////////////////////////////
// Scripting message: sent to the GUI window.
///////////////////////////////////////////////////////////
void WriteScriptMessage(ZString strZtext,bool bEOL)
{
    QString strQText(strZtext);

    WriteScriptMessage(strQText, bEOL);
}

///////////////////////////////////////////////////////////
// Scripting message: sent to the GUI window.
///////////////////////////////////////////////////////////
void WriteScriptMessage(QString strQText, bool bEOL)
{
    if (pGexWizardConsole)
    {
        if(bEOL)
            strQText += "\n";

        pGexWizardConsole->WriteMessage(strQText);

        // If time-sharing mode (not multi-thread), need to call EventLoop to update GUI.
        QCoreApplication::processEvents();

        GSLOG(SYSLOG_SEV_INFORMATIONAL, strQText.toLatin1().data());
    }
}

///////////////////////////////////////////////////////////
// Scripting: Erase the list of 'favorite Scripts'
///////////////////////////////////////////////////////////
void EraseFavoriteList(void)
{
    if (pGexWizardConsole)
    {
        pGexWizardConsole->EraseFavoriteList();

        // If time-sharing mode (not multi-thread), need to call EventLoop to update GUI.
        QCoreApplication::processEvents();
    }
}

///////////////////////////////////////////////////////////
// Scripting: Insert script to the list of 'favorite Scripts'
///////////////////////////////////////////////////////////
void InsertFavoriteList(const char *szPath, const char *szTitle)
{
    if (pGexWizardConsole)
    {
        pGexWizardConsole->InsertFavoriteList(szPath,szTitle);

        // If time-sharing mode (not multi-thread), need to call EventLoop to update GUI.
        QCoreApplication::processEvents();
    }
}

///////////////////////////////////////////////////////////
// Get message from scripting console
///////////////////////////////////////////////////////////
QString GetScriptConsoleMessage(QString strField)
{
    if (pGexWizardConsole)
    {
        // If time-sharing mode (not multi-thread), need to call EventLoop to update GUI.
        QCoreApplication::processEvents();

        return pGexWizardConsole->GetScriptConsoleMessage(strField);
    }

    return "";
}

///////////////////////////////////////////////////////////
// Tells that Report page will have to be launched once script
// is completed.
///////////////////////////////////////////////////////////
void ShowReportOnCompletion(const char *szShowPage)
{
    if (pGexWizardConsole)
    {
        // Set flag: GEX will launch Report page after script is finished.
        pGexWizardConsole->strShowReportPage = szShowPage;

        // If time-sharing mode (not multi-thread), need to call EventLoop to update GUI.
        QCoreApplication::processEvents();
    }
}

///////////////////////////////////////////////////////////
// Scripting: Show/Hide HTML console window
///////////////////////////////////////////////////////////
void ShowHtmlConsole(bool bShow)
{
    if (pGexWizardConsole)
    {
        pGexWizardConsole->ShowHtmlConsole(bShow);

        // If time-sharing mode (not multi-thread), need to call EventLoop to update GUI.
        QCoreApplication::processEvents();
    }
}

///////////////////////////////////////////////////////////
// Scripting: Load page into HTML console window
///////////////////////////////////////////////////////////
void ShowHtmlPage(char *szPage)
{
    if (pGexWizardConsole)
    {
        pGexWizardConsole->ShowHtmlPage(szPage);

        // If time-sharing mode (not multi-thread), need to call EventLoop to update GUI.
        QCoreApplication::processEvents();
    }
}

///////////////////////////////////////////////////////////
// Scripting: Set a GEX environment variable
///////////////////////////////////////////////////////////
void SetConfigurationField(char *szSection,char *szField,char *szValue)
{
    if (pGexWizardConsole)
    {
        pGexWizardConsole->SetConfigurationField(szSection,szField,szValue);

        // If time-sharing mode (not multi-thread), need to call EventLoop to update GUI.
        QCoreApplication::processEvents();
    }
}
