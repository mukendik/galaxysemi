#include <QProgressBar>
#include <QDragEnterEvent>
#include <QMimeData>

#ifndef GEX_NO_WEBKIT
#include <QWebSettings>
#include <QWebFrame>
#include <QWebElement>
#endif

#include "gex_web_browser.h"
#include "gex_web_view.h"

#include "browser_dialog.h"
#include <gqtl_log.h>
#include "gex_report.h"

extern QProgressBar*	GexProgressBar;	// Handle to progress bar in status bar
extern GexMainwindow*	pGexMainWindow;
extern CGexReport* gexReport;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexWebView
//
// Description	:	Web viewer based on QWebView
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexWebView::GexWebView(QWidget *pParent) :
#ifdef GEX_NO_WEBKIT
        QTextBrowser(pParent)
#else
        QWebView(pParent)
#endif
{
    setObjectName("GSWebView");
    mLoadFinished=false;
    // Support for Drag&Drop
    setAcceptDrops(true);
    #ifndef GEX_NO_WEBKIT
        //GSLOG(SYSLOG_SEV_DEBUG, QString("WebKit max pages in cache = %1").arg( settings()->maximumPagesInCache());
        //GSLOG(SYSLOG_SEV_DEBUG, QString("WebKit local storage path : %1").arg( settings()->localStoragePath()).toLatin1().constData());
        //GSLOG(SYSLOG_SEV_DEBUG, QString("WebKit offline storage path : %1").arg( settings()->offlineStoragePath()).toLatin1().constData() );

        // Check me :
        // Specifies whether images are automatically loaded in web pages. This is enabled by default.
        settings()->globalSettings()->setAttribute(QWebSettings::AutoLoadImages, true); //
        // Specifies whether QtWebkit will try to pre-fetch DNS entries to speed up browsing. This only works as a global attribute. Only for Qt 4.6 and later. This is disabled by default.
        settings()->globalSettings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true); // default disabled

        settings()->globalSettings()->setAttribute(QWebSettings::JavascriptEnabled, true); //
        settings()->globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
        settings()->globalSettings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);

        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);
        settings()->globalSettings()->setAttribute(QWebSettings::PrintElementBackgrounds, false);
        // Specifies whether support for the HTML 5 offline storage feature is enabled or not. This is disabled by default.
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, false); //
        // Specifies whether support for the HTML 5 web application cache feature is enabled or not. This is disabled by default.
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
        // Specifies whether support for the HTML 5 local storage feature is enabled or not. This is disabled by default.
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::LocalStorageEnabled, true);

        // pecifies whether locally loaded documents are allowed to access other local urls. This is enabled by default. For more information about security origins and local vs. remote content see QWebSecurityOrigin.
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
        // specifies whether locally loaded documents are allowed to access remote urls. This is disabled by default. For more information about security origins and local vs. remote content see QWebSecurityOrigin.
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);

        // Specifies whether load requests should be monitored for cross-site scripting attempts. Suspicious scripts will be blocked and reported in the inspector's JavaScript console. Enabling this feature might have an impact on performance and it is disabled by default.
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::XSSAuditingEnabled, true);

        // This feature, when used in conjunction with QGraphicsWebView, accelerates animations of web content. CSS animations of the transform and opacity properties will be rendered by composing the cached content of the animated elements. This is enabled by default.
        //settings()->globalSettings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);

          //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::SpatialNavigationEnabled, true);

        // This setting enables the tiled backing store feature for a QGraphicsWebView. With the tiled backing store enabled, the web page contents in and around the current visible area is speculatively cached to bitmap tiles. The tiles are automatically kept in sync with the web page as it changes. Enabling tiling can significantly speed up painting heavy operations like scrolling. Enabling the feature increases memory consumption. It does not work well with contents using CSS fixed positioning (see also resizesToContents property). tiledBackingStoreFrozen property allows application to temporarily freeze the contents of the backing store. This is disabled by default.
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::TiledBackingStoreEnabled, true);

        // With this setting each subframe is expanded to its contents. On touch devices, it is desired to not have any scrollable sub parts of the page as it results in a confusing user experience, with scrolling sometimes scrolling sub parts and at other times scrolling the page itself. For this reason iframes and framesets are barely usable on touch devices. This will flatten all the frames to become one scrollable page. This is disabled by default.
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::FrameFlatteningEnabled, true);

        // This setting enables WebKit's workaround for broken sites. It is enabled by default.
        //m_pWebView->settings()->globalSettings()->setAttribute(QWebSettings::SiteSpecificQuirksEnabled, true);

        QObject::connect(this, SIGNAL(linkClicked(QUrl)), this, SLOT(onLinkClicked(QUrl)) );
        QObject::connect(this, SIGNAL(loadStarted()), this,SLOT(onLoadStarted()) );
        QObject::connect(this, SIGNAL(loadProgress(int)), this, SLOT(onLoadProgress(int)));
        QObject::connect(this, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
        QObject::connect(this, SIGNAL(statusBarMessage(QString)), this, SLOT(onStatusBarMessage(QString)) );
        QObject::connect(this, SIGNAL(titleChanged(QString)), this, SLOT(onTitleChanged(QString)) );
        QObject::connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(onUrlChanged(QUrl)) );
    #endif
}

GexWebView::~GexWebView()
{

}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexWebView::dragEnterEvent(QDragEnterEvent *pDragEnterEvent)
//
// Description	:	Called when mouse enters this widget. Accept or reject the drag
//					according the drag format
//
///////////////////////////////////////////////////////////////////////////////////
void GexWebView::dragEnterEvent(QDragEnterEvent * pDragEnterEvent)
{
    // Accept Drag if files list dragged over.
    if (pDragEnterEvent->mimeData()->hasFormat("text/uri-list") == false)
        pDragEnterEvent->ignore();
}
///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexWebView::dragMoveEvent(QDragMoveEvent *pDragMoveEvent)
//
// Description	:	Called when mouse moves within this widget. Accept or reject the drag
//					according the drag format
//
///////////////////////////////////////////////////////////////////////////////////
void GexWebView::dragMoveEvent(QDragMoveEvent *pDragMoveEvent)
{
    // Accept Drag if files list dragged over.
    if (pDragMoveEvent->mimeData()->hasFormat("text/uri-list"))
        pDragMoveEvent->acceptProposedAction();
    else
        pDragMoveEvent->ignore();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexWebView::dropEvent(QDropEvent *pDropEvent)
//
// Description	:	Called when the drag is dropped on this widget.
//
///////////////////////////////////////////////////////////////////////////////////
void GexWebView::dropEvent(QDropEvent * pDropEvent)
{
    // Have the drag&drop request processe by the main window.
    if(pGexMainWindow != NULL)
        pGexMainWindow->dropEvent(pDropEvent);
}

#ifndef GEX_NO_WEBKIT

    void GexWebView::onLinkClicked ( const QUrl & url )
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("on Link Clicked %1").arg( url.toString()).toLatin1().constData());
        mLastUrlClicked=url;
    }

    void GexWebView::onLoadProgress( int progress )
    {
        if (!GexProgressBar)
            return;

        //GSLOG(SYSLOG_SEV_DEBUG, QString("onLoadProgress %1").arg( progress));
        GexProgressBar->show();
        GexProgressBar->setValue(progress);
        //if (progress==100)
        //GexProgressBar->hide();
        GexProgressBar->setWindowTitle( QString("Loading...(%1 %)").arg(progress) );
    }

    void GexWebView::onLoadStarted( )
    {
        QWebElement webElt = page()->currentFrame()->findFirstElement("img#AA");

        if (webElt.isNull() == false)
            webElt.setAttribute("src", "../images/home007-b.png");

        GSLOG(SYSLOG_SEV_DEBUG, "LoadStarted...");
        mLoadFinished=false;
    }

    void GexWebView::onLoadFinished( bool ok )
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("LoadFinished : %1").arg( ok?"ok":"error").toLatin1().constData());

    // GCORE-11713
    if (gexReport != NULL)
    {
        int lMessageCount;
        QString lMessageString;
        lMessageCount = gexReport->GetReportLogList()
            .filter(GS::Gex::ReportLog::ReportError).count();
        if (lMessageCount)
        {
            lMessageString = QString("<font color=#FF0000>%1</font>")
                             .arg(lMessageCount);
        }
        else
        {
            lMessageString = QString("%1").arg(lMessageCount);
        }
        this->page()->mainFrame()
        ->evaluateJavaScript(QString("document.getElementById('errorCount')."
                                     "innerHTML = '%1';").arg(lMessageString));
        lMessageCount = gexReport->GetReportLogList()
            .filter(GS::Gex::ReportLog::ReportWarning).count();
        if (lMessageCount)
        {
            lMessageString = QString("<font color=#FF0000>%1</font>")
                             .arg(lMessageCount);
        }
        else
        {
            lMessageString = QString("%1").arg(lMessageCount);
        }
        this->page()->mainFrame()
        ->evaluateJavaScript(QString("document.getElementById('warnCount')."
                                     "innerHTML = '%1';").arg(lMessageString));
        lMessageCount = gexReport->GetReportLogList()
            .filter(GS::Gex::ReportLog::ReportInformation).count();
        if (lMessageCount)
        {
            lMessageString = QString("<font color=#FF0000>%1</font>")
                             .arg(lMessageCount);
        }
        else
        {
            lMessageString = QString("%1").arg(lMessageCount);
        }
        this->page()->mainFrame()
        ->evaluateJavaScript(QString("document.getElementById('infoCount')."
                                     "innerHTML = '%1';").arg(lMessageString));
    }

        mLoadFinished=true;

        QWebElement webElt = page()->currentFrame()->findFirstElement("img#AA");

        if (webElt.isNull() == false)
            webElt.setAttribute("src", "../images/home007-b.png");

        if (!GexProgressBar)
            return;

        if (ok)
           GexProgressBar->hide();
        else
            GexProgressBar->setWindowTitle("Finished (error)");
    }

    void GexWebView::onStatusBarMessage( const QString & text )
    {
        if (!text.isEmpty())
            GSLOG(SYSLOG_SEV_DEBUG, QString("Status bar message : '%1'").arg( text).toLatin1().constData());
    }

    void GexWebView::onTitleChanged( const QString & title )
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Title changed : '%1'").arg( title).toLatin1().constData());
    }

    void GexWebView::onUrlChanged( const QUrl & url )
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Url changed to %1").arg( url.toString()).toLatin1().constData());
    }

#endif
