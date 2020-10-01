#include <QNetworkRequest>
#include <QNetworkReply>

#include <gqtl_sqlbrowser.h>
#include "gqtl_sysutils.h"

#include "engine.h"
#include <gqtl_log.h>
#include "browser_dialog.h"
#include "browser.h"
#include "javascriptcenter.h"
#include "report_options.h"
#include "product_info.h"
#include "message.h"

extern CReportOptions	ReportOptions;

void GexMainwindow::ShowGtmWidget(void)
{
    if (mGtmWidget)
    {
        // Show GTM widget
        ShowWizardDialog(GEX_GTM_TESTERS_WIDGET);

        // Display relevant top navigation bar
        QString strString;

        // HTML skin pages Sub-folder .
        strString += navigationSkinDir();
        strString += "pages/";
        strString += GEX_BROWSER_ACTIONLINK;
        strString += GEXGTM_BROWSER_TESTERS_TOPLINK;
        pageHeader->setSource( QUrl::fromLocalFile(strString) );
    }
    else
        GS::Gex::Message::warning("Error",
                                  "GTM widget has not yet been allocated!");
}

void GexMainwindow::ShowOptionsCenter(void)
{
    if (m_pOptionsCenter)
    {
        // PYC, 20/06/2011, source code optimization
        if(!RefreshOptionsCenter())
        {
            GSLOG(SYSLOG_SEV_ERROR, "RefreshOptionsCenter failed");
            GEX_ASSERT(false);
        }

        // Show Option page
        ShowWizardDialog(GEX_OPTIONS_CENTER);

        // Reset HTML sections to create flag: ALL pages to create.
        // pGexMainWindow->iHtmlSectionsToSkip = 0;
        iHtmlSectionsToSkip = 0;

        // Display relevant top navigation bar
        QString strPageName, strString;

        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            strPageName = GEXMO_BROWSER_OPTIONS_TOPLINK;
        else if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
            strPageName = GEXGTM_BROWSER_OPTIONS_TOPLINK;
        else
            strPageName = GEX_BROWSER_OPTIONS_TOPLINK;

        // HTML skin pages Sub-folder .
        strString += navigationSkinDir();
        strString += "pages/";
        strString += GEX_BROWSER_ACTIONLINK;
        strString += strPageName;

        // GCORE-7369 : reload top page properly for OEM version
        ReloadHeaderPage( QUrl::fromLocalFile(strString) );
    }
    else
        GS::Gex::Message::warning("Error",
                                  "OptionsCenter library has not been found "
                                  "or is still not loaded !");
}

void GexMainwindow::ShowLogsCenter(void)
{
    //ShowWizardDialog(GEX_LOGS_CENTER);
    if (m_pLogsCenter)
        m_pLogsCenter->show();
}

void GexMainwindow::ShowJavaScriptCenter(void)
{
    if (!m_pJavaScriptCenter)
        m_pJavaScriptCenter=new JavaScriptCenter(0);
    m_pJavaScriptCenter->showMaximized();
    //ShowWizardDialog(GEX_JAVASCRIPT_CENTER);
}

void GexMainwindow::ShowSqlBrowser(void)
{
    if (!m_pSqlBrowser)
    {
        QString o;
        SQLBROWSER_GET_INSTANCE(m_pSqlBrowser, NULL, o);
        GSLOG(SYSLOG_SEV_NOTICE, QString("SQLBROWSER_GET_INSTANCE : '%1' : %2")
              .arg(m_pSqlBrowser?m_pSqlBrowser->objectName().toLatin1().data():"NULL")
              .arg(o).toLatin1().data());
        if (m_pSqlBrowser)
          QObject::connect(this, SIGNAL(RefreshDB()), m_pSqlBrowser, SLOT(Refresh()) );
     }
     if (m_pSqlBrowser)
        m_pSqlBrowser->showMaximized(); // or show() ?
    //ShowWizardDialog(GEX_SQLBROWSER);
}
