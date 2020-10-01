///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QFile>
#include <QProgressBar>
#include <QPrinter>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookie>
#include <QNetworkAccessManager>

#ifndef GEX_NO_WEBKIT
#include <QWebFrame>
#include <QWebElementCollection>
#include <QWebElement>
#endif

#include <gqtl_log.h>

#include "engine.h"
#include "gex_web_browser.h"
#include "gex_web_view.h"
#include "message.h"
#include "browser_dialog.h"
#include "browser.h"
#include "gex_options_handler.h"
#include "pat_definition.h"
//#include <gqtl_log.h>

///////////////////////////////////////////////////////////////////////////////////
// System Includes
///////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
  #include <windows.h>
#endif

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern GexMainwindow *	pGexMainWindow;
extern QProgressBar	*	GexProgressBar;	// Handle to progress bar in status bar

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexWebBrowser
//
// Description	:	Class which handles action between Gex and the WebViewer
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexWebBrowser::GexWebBrowser(QWidget * pParent) : QObject(pParent), m_webHistory(this)
{
    m_pWebView = new GexWebView(pParent);

    setObjectName("GSWebBrowser");
    m_bTest = false;

#ifdef GEX_NO_WEBKIT

    m_pWebView->setFrameStyle( QFrame::NoFrame);
    m_pWebView->setOpenLinks(false);

#ifdef __linux__
    // Under linux, the default HTML font looks too big and bold, so force a different font!
    m_pWebView->setFont(QFont("utopia",9));
#endif

    connect(m_pWebView,	SIGNAL(anchorClicked(const QUrl&)),		this,	SLOT(linkClicked(const QUrl&)));
    connect(m_pWebView, SIGNAL(highlighted(QString)),			this,	SIGNAL(sLinkHovered(const QString&)));

#else

    m_pWebView->setContextMenuPolicy(Qt::NoContextMenu);
    m_pWebView->settings()->setFontFamily(QWebSettings::StandardFont,  "Arial");
    m_pWebView->settings()->setFontFamily(QWebSettings::SansSerifFont, "MS Verdana");
    m_pWebView->settings()->setFontSize(QWebSettings::DefaultFontSize, 11);

    if (m_pWebView->settings()->testAttribute(QWebSettings::WebGLEnabled))
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "WebGL Enabled");
    }
    else
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "WebGL not Enabled");
    }

    // Delegate the link processing
    m_pWebView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    // needed for Dashboard project to reuse registered cookie(s)
    //m_pWebView->page()->setNetworkAccessManager(&gGexNetworkAccessManager);
    m_pWebView->page()->setNetworkAccessManager(&GS::Gex::Engine::GetInstance().GetNAM());

    connect(m_pWebView,			SIGNAL(linkClicked(const QUrl&)),				this,	SLOT(linkClicked(const QUrl&)));
    connect(m_pWebView->page(), SIGNAL(linkHovered(QString,QString,QString)),	this,	SIGNAL(sLinkHovered(const QString&, const QString&, const QString&)));
    connect(m_pWebView->page(),	SIGNAL(loadFinished(bool)),						this,	SLOT(loadFinished(bool)));

#endif
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexWebBrowser::~GexWebBrowser()
{

}

bool GexWebBrowser::test()
{
    return m_bTest;
}

void GexWebBrowser::setTest(bool bTest)
{
    m_bTest = bTest;
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

bool GexWebBrowser::find(const QString& strText)
{
    if (m_pWebView)
    #ifdef GEX_NO_WEBKIT
            return m_pWebView->find(strText);
    #else
            return m_pWebView->findText(strText);
    #endif

    return false;
}

const QUrl& GexWebBrowser::url() const
{
    return m_webHistory.currentUrl();
}

QString GexWebBrowser::currentUrl()
{
    return m_webHistory.currentUrl().toString();
}

const QUrl&	GexWebBrowser::GetLastUrlClicked()
{
    return m_pWebView->GetLastUrlCliked();
}

void GexWebBrowser::loadUrl(const QUrl &url)
{
    GSLOG(7, QString("load Url %1").arg(url.toString()).toLatin1().data() );
    if (m_pWebView)
    {
        //QString lLF=url.toLocalFile(); // stupidly gives file:///C:/...
        //QFile	hLocalFile(lLF);
        QFileInfo lFI(url.toLocalFile()); // 5909
        if (url.scheme()=="file")
        {
            //GSLOG(SYSLOG_SEV_DEBUG, QString("load Url : local file '%1'").arg( lLF).toLatin1().constData());
            //GSLOG(SYSLOG_SEV_DEBUG, QString("load Url : path=%1").arg( url.path()).toLatin1().constData());
            //GSLOG(SYSLOG_SEV_DEBUG, QString("load Url : file name=%1").arg( url.fileName()).toLatin1().constData());
        }
        if (lFI.exists())
        //if(hLocalFile.exists())
        {
            #ifdef GEX_NO_WEBKIT
                m_pWebView->setSource(url);
                loadFinished(true);
            #else
                m_pWebView->load(url);
            #endif
            urlChanged(url);
        }
        else
        {
            if (url.toString().startsWith("http://"))
            {
#ifdef GEX_NO_WEBKIT
                m_pWebView->setSource(url);
                loadFinished(true);
#else
                if (GexProgressBar)
                {
                    GexProgressBar->show();
                    GexProgressBar->setMaximum(100);
                    GexProgressBar->setValue(0);
                }
                m_pWebView->load(url);
#endif
                urlChanged(url);
            }
            else
                processActionLink(url, true);
        }

        m_pWebView->setFocus();

        emit sBackAvailable(m_webHistory.canGoBack());
        emit sForwardAvailable(m_webHistory.canGoForward());
    }
}

void GexWebBrowser::loadUrl(const QString& strUrl)
{
    GSLOG(7, QString("Load Url %1").arg(strUrl).toLatin1().data());
    if (strUrl.startsWith("http://"))
        loadUrl(QUrl(strUrl));
    else
        loadUrl(QUrl::fromLocalFile(strUrl));
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexWebBrowser::loadFinished(bool bOk)
//
// Description	:	Called when the html page is fully loaded
//
///////////////////////////////////////////////////////////////////////////////////
void GexWebBrowser::loadFinished(bool /*bOk*/)
{
    emit sActionTriggered(GexWebBrowserAction(GexWebBrowserAction::HtmlAction, QString()));
}

void GexWebBrowser::linkClicked(const QUrl& urlClicked, bool forwardBack)
{
    GSLOG(6, QString("linkClicked %1").arg(urlClicked.toString()).toLatin1().data());
    QString strLink;
    QString strHtmlToolbarAction;

    if (urlClicked.hasFragment())
    {
        QString lFrag=urlClicked.fragment();
        if (lFrag.contains("tb_patreload")) // 5909
        {
            // There is perhaps a concatenation of several fragment : example : ....html#anchor#_gex_tb_patreload....#1
            strLink	= "#" + urlClicked.fragment().section("#", -2);
        }
        else
            strLink	= "#" + urlClicked.fragment();
        strHtmlToolbarAction = strLink.section(GEX_BROWSER_ACTIONBOOKMARK,1);

        GSLOG(7, QString("link=%1 HtmlToolbarAction=%2").arg(strLink).arg(strHtmlToolbarAction).toLatin1().data());

        if (strHtmlToolbarAction.isEmpty() == false)
        {
            if (processActionLink(strLink, urlClicked) == false)
            {
                processActionBookmark(strHtmlToolbarAction, forwardBack);
                urlChanged(urlClicked);
            }
        }
        else if (lFrag.isEmpty())
        {
            QUrl lUrl = urlClicked;

            lUrl.setFragment(QString());
            loadUrl(lUrl);
        }
        else
            loadUrl(urlClicked);
    }
    else if (processActionLink(urlClicked, forwardBack) == false)
    {
        loadUrl(urlClicked);
    }

    // -- for the interactive 3d, table and chart, since they are in a detach window
    // -- we keep the current windows visible
   /* if( strHtmlToolbarAction.contains("drill_chart") ||
        strHtmlToolbarAction.contains("drill_table") ||
        strHtmlToolbarAction.contains("drill_3d"))
    {
         loadUrl(urlClicked);
         reload();
    }*/

    GSLOG(7, "linkClicked ok");
}

QWidget * GexWebBrowser::widget() const
{
    return m_pWebView;
}

void GexWebBrowser::back()
{
    m_webHistory.back();
}

void GexWebBrowser::forward()
{
    m_webHistory.forward();
}

void GexWebBrowser::reload()
{
    if (m_pWebView)
        m_pWebView->reload();
}

void GexWebBrowser::stop()
{
    #ifndef GEX_NO_WEBKIT
    if (m_pWebView)
        m_pWebView->stop();
    #endif
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexWebBrowser::onUrlChanged(const QUrl& urlChanged)
//
// Description	:	Stops loading the document
//
///////////////////////////////////////////////////////////////////////////////////
void GexWebBrowser::urlChanged(const QUrl& urlChanged)
{
    m_webHistory.addUrl(urlChanged);
    emit sUrlChanged(urlChanged);
}

bool GexWebBrowser::processActionLink(const QUrl& url, bool forwardBack)
{
    GSLOG(7, QString("process Action Link url %1").arg(url.toString()).toLatin1().data() );
    // Check if email link...under Windows only!
    if(url.scheme() == "mailto")
    {
        QString strMail = url.path();

        // Execute 'mailto' if under windows...or show a message under Unix.
        #if defined unix || __MACH__
            QString gexPageName = "Please use your email application to write\na message to: " + strMail;
            GS::Gex::Message::information("", gexPageName);
        #else
            // Rebuild full string 'mailto"' + email address
            strMail = "mailto:" + strMail;

            // Launch email shell
            ShellExecuteA(NULL,
                   "open",
                   strMail.toLatin1().constData(),
                   NULL,
                   NULL,
                   SW_SHOWNORMAL);
        #endif
        return true;
    }

    if (url.toString().startsWith("http://"))
    return processActionLink(url.toString(), url, forwardBack);

    return processActionLink(url.toLocalFile(), url, forwardBack);
}

bool GexWebBrowser::processActionLink(const QString& strLink, const QUrl& url, bool forwardBack)
{
    GSLOG(7, QString("process Action Link '%1' from url %2")
          .arg(strLink).arg(url.toString()).toLatin1().data() );
    // Loop until we find the link in our list...then launch its function!
    int		nIndex		= pGexMainWindow->LookupLinkName(strLink);

    // If this link is not in our list, we simply ignore it!
    if(nIndex == -1)
    {
        // Check if Clicking a file that can be opened (eg: .CSV, .TXT, etc)
        if(strLink.endsWith(".csv", Qt::CaseInsensitive))
        {
            GexWebBrowserAction ba(GexWebBrowserAction::CsvAction, strLink);
            ba.m_DesiredUrl=url;
            emit sActionTriggered(ba);
            return true;
        }

        QString strActionName = strLink.section("--",0,0);
        strActionName = strActionName.remove(0, 1);

        // Check if clicking a link to open an external url
        if (strActionName == GEX_BROWSER_ACT_OPEN_URL)
        {
            QString strUrl = strLink.section("--", 1, 1);
            emit sActionTriggered(GexWebBrowserAction(GexWebBrowserAction::OpenUrlAction, strUrl));
            return true;
        }

        if (strLink.startsWith("http://"))
        {
            //emit sActionTriggered(GexWebBrowserAction(GexWebBrowserAction::OpenUrlAction, strLink)); // open in the external webbrowser !
            emit sActionTriggered(GexWebBrowserAction(GexWebBrowserAction::LinkAction, strLink, nIndex));
            return true;
        }

        // Check if Clicking a file that can be opened (eg: .CSV, .TXT, etc)
        if(strLink.contains("_export_ftr_correlation.html", Qt::CaseInsensitive))
        {
            emit sActionTriggered(GexWebBrowserAction(GexWebBrowserAction::FTRExportAction, strLink,nIndex));
            return true;
        }

        // prevent splash screen from re-appear
        if (strLink.contains(GEX_HTMLPAGE_SPLASHHOME, Qt::CaseInsensitive))
        {
            //do nothing
            return true;
        }

        return false;
    }

    // Make sure selectedURL contains just the action link. Discard action if selectedURL contains
    // the full URL (<file>#<action link>)
    if(!strLink.startsWith("#") && strLink.contains("#"))
        return false;

    // GCORE-9822
    QString lOptionsLinkName = GEX_BROWSER_ACTIONLINK;
    lOptionsLinkName += GEX_BROWSER_OPTIONS_WIZARD;
    if(!forwardBack	&&
       (strLink.contains(GEX_BROWSER_ONEFILE_WIZARD)							||
       strLink.contains(GEX_HTMLPAGE_HOME_ROOT_TOOLBOX, Qt::CaseInsensitive)	||
	   strLink.contains(GEX_BROWSER_FILES_TOPLINK, Qt::CaseInsensitive)         ||
	   strLink.contains("_options.htm", Qt::CaseInsensitive)))

    {
        m_webHistory.addUrl(url);
    }

    // Action performed!
    GexWebBrowserAction ba(GexWebBrowserAction::LinkAction, strLink, nIndex);
    ba.m_DesiredUrl=url;
    emit sActionTriggered(ba);

    GSLOG(7, "processActionLink ok");
    return true;
}

void GexWebBrowser::processActionBookmark(const QString& strBookmark, bool forward)
{
    // Not a known standard command hyperlink: either a Interactive link, or Plugin link
    // Extract action (string format is <url>--field=val--field=val...
    QString strActionName = strBookmark.section("--",0,0);

    if(strActionName == GEX_BROWSER_ACT_DRILL)
    {
        // Extract chart type to Drill into
        QString strField = strBookmark.section("--",1,1).section('=',0,0);

        // Show Tables
        if(strField == "drill_table")
        {
            // Display the inteactive table page
            emit sActionTriggered(GexWebBrowserAction(GexWebBrowserAction::BookmarkDrillTableAction, strBookmark, -1, forward));
        }
        // Show 2D Charting page
        else if(strField == "drill_chart")
        {
            // Display the 2D inteactive page
            emit sActionTriggered(GexWebBrowserAction(GexWebBrowserAction::BookmarkDrillChartAction, strBookmark, -1, forward));
        }
        // Show Wafermap 3D
        else if(strField == "drill_3d")
        {
            // Enter into Wafermap 3D DRILL mode
            emit sActionTriggered(GexWebBrowserAction(GexWebBrowserAction::BookmarkDrill3dAction, strBookmark, -1, forward));
        }
    }
    else if(strActionName == GEX_BROWSER_ACT_ENTERPRISE)
        emit sActionTriggered(GexWebBrowserAction(GexWebBrowserAction::BookmarkERAction, strBookmark));
    else if(strActionName == GEX_BROWSER_ACT_ADV_ENTERPRISE)
        emit sActionTriggered(GexWebBrowserAction(GexWebBrowserAction::BookmarkAdvERAction, strBookmark));
}

QString GexWebBrowser::capture(const QString& url, const QString &pre_capture_script_file, const QString& targetfile)
{
#ifndef GEX_NO_WEBKIT
    QUrl dashboard_url(url);

    QPrinter pr;
    pr.setOutputFormat(QPrinter::PdfFormat);
    //pr.setFullPage(true); // Test me
    pr.setOrientation(QPrinter::Landscape);


    GexWebView wv;
    wv.show();
    if (wv.page())
        wv.page()->setNetworkAccessManager(& GS::Gex::Engine::GetInstance().GetNAM());

    /*
    // directly using the NAM to get the dashboard : does not seems to work
        QNetworkRequest dnr(dashboard_url);
        reply=mNAM.get(dnr);
        while (!reply->isFinished()) // isFinished() or isRunning() ?
        {
            //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("NetworkReply : running:%1, finished:%2").arg(.arg(            //GSLOG(SYSLOG_SEV_INFORMATIONAL, "NetworkReply : running:%1.arg( finished:%2").arg(
              //       reply->isRunning()?"yes":"no", reply->isFinished()?"yes":"no" );
            QCoreApplication::processEvents();
            QThread::currentThread()->wait(100);
        }
        if (reply->error()!=QNetworkReply::NoError)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("NetworkReply : error : %1").arg( reply->errorString()).toLatin1().constData());
        }
        else
        {
            int v= reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (v >= 300 && v < 400) // redirection
            {
                QUrl newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Redirection requested to %1").arg( newUrl.toString()).toLatin1().constData());
            }
            else
            {
                QByteArray dba=reply->readAll();
                //m_pWebView->setContent(dba, "", QUrl("http://prismweb.galaxysemi.com"));
                wv.setContent(dba, "", QUrl("http://prismweb.galaxysemi.com"));
                QCoreApplication::processEvents();
                pr.setOutputFileName(QDir::homePath()+"/GalaxySemi/temp/o1.pdf");
                wv.print(&pr);
            }
        }
    */

    /*
    QScriptValue sc = m_scriptEngine->evaluate("JSON.parse").call(QScriptValue(), QScriptValueList() << QString(rba) );
    QList<QObject*> productList;
    if (sc.property("products").isArray()) { .... }
    */

    QWebSettings* ws=wv.settings()->globalSettings();
    if (ws)
    {
        GSLOG(7, QString("LocalStoragePath=%1").arg(ws->localStoragePath()).toLatin1().data());
        GSLOG(7, QString("MaxPagesInCache=%1").arg(ws->maximumPagesInCache()).toLatin1().data() );
    }

    //wv.sizeHint();
    //wv.settings()
    //wv.setSource(QUrl(url));

    //dashboard_url.set
    //wv.load( dashboard_url ); // load QUrl is only available with QWebView, not with QTextBrowser

    if (wv.page()->mainFrame())
        wv.page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    //nr.setUrl(dashboard_url);
    //wv.load(nr);
    wv.setUrl(QUrl(dashboard_url)); // better ?
    //Example: http://prismweb.galaxyec7.com/dashboards/#f=103&d=321

    //wv.resize(wv.sizeHint());
    wv.resize(wv.maximumSize()); // this line is known to crash on 7.4. Ask Qt why...
    QCoreApplication::processEvents();

    while (!wv.loadFinished())  //(p->bytesReceived()<1)  //wf->frameName().isEmpty()) : do not work
        QCoreApplication::processEvents();

    GSLOG(SYSLOG_SEV_DEBUG, "Webview load finished...");
    QWebPage* p=wv.page();
    if (!p)
        return "error : page null in webview";

    GSLOG(6, QString("Page byte received : %1 ko").arg(p->bytesReceived()/1024).toLatin1().data() );
    QWebFrame* mf=p->mainFrame();
    if (!mf)
        return "error : main frame null";

    if (!pre_capture_script_file.isEmpty())
    {
        if (QFile::exists(pre_capture_script_file))
        {
            QFile scriptFile(pre_capture_script_file);
            if (scriptFile.open(QIODevice::ReadOnly))
            {
                QTextStream stream(&scriptFile);
                QString contents = stream.readAll();
                scriptFile.close();
                QVariant v=mf->evaluateJavaScript(contents);
                GSLOG(5, QString("Execution of pre capture script : %1").arg(v.toString()).toLatin1().data() );
            }
        }
    }

    QWebFrame* frame=mf;
    QWebFrame* cf=p->currentFrame();
    GSLOG(6, QString("CurrentFrame==MainFrame ? %1").arg((cf==mf)?"true":"false").toLatin1().data() );
    QWebFrame* df=p->frameAt(QPoint(100,100));
    if (df)
    {
        GSLOG(6, QString("Frame at 100,100 : %1").arg(df->frameName()).toLatin1().data() );
        frame=df;
    }

    if (frame)
    {
        frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
        frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
        QCoreApplication::processEvents();
        wv.thread()->wait(900); // just to be sure
        QThread::currentThread()->wait(1000);
        GSLOG(6, QString("Frame name=%1 title=%2 baseUrl=%3")
              .arg(frame->frameName())
              .arg(frame->title())
              .arg(frame->baseUrl().toString())
              .toLatin1().data() );
        // Pdf
        pr.setOutputFormat(QPrinter::PdfFormat);
        pr.setOutputFileName(QDir::homePath()+"/GalaxySemi/temp/frame.pdf");
        frame->print(&pr);
        // PS
        pr.setOutputFormat(QPrinter::NativeFormat);
        pr.setOrientation(QPrinter::Portrait); // why ?
        pr.setOutputFileName(QDir::homePath()+"/GalaxySemi/temp/frame.ps");
        frame->print(&pr);
        // XPS
        pr.setOutputFormat(QPrinter::NativeFormat);
        pr.setOrientation(QPrinter::Landscape);
        pr.setOutputFileName(QDir::homePath()+"/GalaxySemi/temp/frame.xps");
        frame->print(&pr);

        /*
            // Try to render to an image
            QImage i(wv.size(), QImage::Format_ARGB32);
            QPainter pi(&i);
            if (pi.begin(&i))
            {
                frame->render(&pi, QWebFrame::ContentsLayer);
                pi.end();
                if (!i.save(QDir::homePath()+"/GalaxySemi/temp/frame_image.bmp"))
                    GSLOG(SYSLOG_SEV_ERROR, "Cannot save frame image");
            }
            else
                GSLOG(SYSLOG_SEV_ERROR, "Cannot begin painting to Image");
        */
        GSLOG(6, QString("frame childs : %1").arg(frame->childFrames().size()).toLatin1().data() );
        GSLOG(6, QString("frame pos : x:%1 y:%2").arg(frame->pos().x()).arg(frame->pos().y()).toLatin1().data() );
        GSLOG(6, QString("frame geom : w:%1 h:%2 ").arg(frame->geometry().width())
              .arg(frame->geometry().height()).toLatin1().data() );
        // Searching for the svg
        QWebElementCollection wec=frame->findAllElements("svg");
        GSLOG(6, QString("Found %1 svg in this frame").arg(wec.count()).toLatin1().data() );
        QWebElement svge;
        for (int i=0; i<wec.count(); i++)
        {
            svge=wec.at(i);
            GSLOG(6, QString("SVG %1 geom w:%2 h:%3").arg(i).arg(svge.geometry().width()).arg(svge.geometry().height()).toLatin1().constData() );
            QPixmap pm(svge.geometry().size()); // wv.size() frame->contentsSize()
            QPainter ppm(&pm);
            svge.render(&ppm);
            pm.save(QDir::homePath()+"/GalaxySemi/temp/svg"+QString::number(i)+".bmp");
        }
        svge=wec.at(0);

        // svge height = 1031 ? frame height = 882 ?
        QPixmap pm( svge.geometry().size().width()-frame->scrollBarGeometry(Qt::Vertical).width(),
                    frame->geometry().height() ); // wv.size() frame->contentsSize() svge.geometry().size().height()
        pm.fill(Qt::white);
        QPainter ppm(&pm);
        frame->render(&ppm, QWebFrame::ContentsLayer,
                      QRegion(0,0, svge.geometry().width(), svge.geometry().height() )); // frame->pos().y()
        pm.save(QDir::homePath()+"/GalaxySemi/temp/frame.bmp");
    }

    QPixmap	pPixmap = QPixmap::grabWidget(&wv);
    if (pPixmap.isNull())
        GSLOG(6, "Pixmap null")
    else
    {
        bool b=pPixmap.save(targetfile);
        if (!b)
            GSLOG(4, QString("Failed to save to file %1").arg(targetfile).toLatin1().data() );
    }

    pr.setOutputFileName(QDir::homePath()+"/GalaxySemi/temp/webview.pdf");
    pr.setOutputFormat(QPrinter::PdfFormat);
    pr.setOrientation(QPrinter::Landscape);
    wv.print(&pr);
#else
    Q_UNUSED(url);
    Q_UNUSED(pre_capture_script_file);
    Q_UNUSED(targetfile);
#endif

    return "ok";
}
