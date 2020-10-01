#ifndef GEX_WEB_BROWSER_H
#define GEX_WEB_BROWSER_H

#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>

class GexWebBrowser;

//! \brief Description	:	Class which handled history of the urls visited
class GexWebHistory
{

public:
    //! \brief Constructor
	GexWebHistory(GexWebBrowser * pParent);
    //! \brief Destructor
    ~GexWebHistory();

	void				addUrl(const QUrl&);
	void				clear();
	void				back();
	void				forward();

	bool				canGoBack() const;
	bool				canGoForward() const;

	const QUrl&			currentUrl() const;

private:

	int					m_nCurrentPage;
	QList<QUrl>			m_lstUrl;
	QUrl				m_urlCurrent;
	GexWebBrowser *		m_pWebBrowser;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexWebBrowserAction
//
// Description	:	Class which defines action to do when loading an url
//
///////////////////////////////////////////////////////////////////////////////////
class GexWebBrowserAction
{
public:
	enum Action
	{
		HtmlAction = 0,
		BookmarkDrill3dAction,
		BookmarkDrillTableAction,
		BookmarkDrillChartAction,
		BookmarkERAction,
		BookmarkAdvERAction,
		OpenUrlAction,
        LinkAction, // 7
        CsvAction,
        FTRExportAction
	};

    GexWebBrowserAction(Action eAction, const QString& strLink, int nIndex = -1, bool forwardBack = false);
	GexWebBrowserAction(const GexWebBrowserAction&);
	~GexWebBrowserAction();

    Action			action  () const							{ return m_eAction; }
    int				index   () const							{ return m_nIndex; }
    const QString&	link    () const							{ return m_strLink; }
    bool            forwardBack () const                         { return m_forwardBack; }

	GexWebBrowserAction& operator=(const GexWebBrowserAction&);

    QUrl            m_DesiredUrl;

private:

	Action			m_eAction;
	int				m_nIndex;
    QString			m_strLink;
    bool            m_forwardBack;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexWebBrowser
//
// Description	:	Class which handles action between Gex and the WebViewer
//
///////////////////////////////////////////////////////////////////////////////////
class GexWebBrowser : public QObject
{
	Q_OBJECT

public:
	GexWebBrowser(QWidget * pParent = NULL);
	~GexWebBrowser();

	Q_PROPERTY(bool m_bTest READ test WRITE setTest)

    // Load the url in the page
	void			loadUrl(const QUrl& url);
    const QUrl&		GetLastUrlClicked();

    // Return a widget pointer on the WebView
	QWidget	*		widget() const;

	// test
	bool			test(); //								{ return test; }
	void			setTest(bool bTest); //					{ test = bTest; }

public slots:
    // Description	:	Return the current url
    const QUrl&		url() const;
    //
    QString currentUrl();
    // Load the url in the page
    void			loadUrl(const QString& strUrl);
    // Find the next occurence of the string in the current page
    bool			find(const QString&);
    // Go to the previous url in the history
	void			back();
    // Go to the next url in the history
	void			forward();
    // Called when a link is clicked
    void			linkClicked(const QUrl&, bool forwardBack = false);
    // Reloads the current url
    void			reload();
    // Stops loading the document
    void			stop();
    // Capture the given url into an image : type is defined by extension (.png,...)
    // return 'ok' or 'error...'
    QString         capture(const QString& url, const QString &pre_capture_script_file, const QString& targetfile);

private slots:
	void			urlChanged(const QUrl&);
	void			loadFinished(bool);

signals:

	void			sActionTriggered(const GexWebBrowserAction&);
	void			sBackAvailable(bool);
	void			sForwardAvailable(bool);
#ifdef GEX_NO_WEBKIT
	void			sLinkHovered(const QString&);
#else
	void			sLinkHovered(const QString&, const QString&, const QString&);
#endif
	void			sUrlChanged(const QUrl&);

private:
    // Description	:	Parse the link to find out if it is a gex action
    // full Url is needed to keep a trace on the original desired Url
    bool			processActionLink(const QString&, const QUrl&, bool forwardBack = false);
    // Description	:	Parse the link to find out if it is a gex action
    bool			processActionLink(const QUrl&, bool forwardBack = false);
    // Description	:	Parse the bookmark to find out the action to do
    void			processActionBookmark(const QString&, bool forward);

    bool			m_bTest;
	GexWebHistory	m_webHistory;
    class GexWebView *	m_pWebView;
};

#endif // GEX_WEB_BROWSER_H
