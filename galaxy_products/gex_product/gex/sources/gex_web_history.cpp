#include "gex_web_browser.h"
#include <gqtl_log.h>

GexWebHistory::GexWebHistory(GexWebBrowser * pWebBrowser) : m_nCurrentPage(-1), m_pWebBrowser(pWebBrowser)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "GexWebHistory...");
}

GexWebHistory::~GexWebHistory()
{

}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexWebHistory::clear()
//
// Description	:	Clear Url's history
//
///////////////////////////////////////////////////////////////////////////////////
void GexWebHistory::clear()
{
    // If history is empty, exit now
    if (m_lstUrl.count() == 0)
        return;

    // Back up the current url
    QUrl urlCurrent = currentUrl();

    // Clear the history
    m_nCurrentPage = -1;
    m_lstUrl.clear();

    // add the current url
    addUrl(urlCurrent);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexWebHistory::back()
//
// Description	:	Go to the previous Url
//
///////////////////////////////////////////////////////////////////////////////////
void GexWebHistory::back()
{
    if (canGoBack())
    {
        --m_nCurrentPage;
        m_urlCurrent = m_lstUrl.at(m_nCurrentPage);

        m_pWebBrowser->linkClicked(m_urlCurrent, true);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexWebHistory::forward()
//
// Description	:	Go to the next Url
//
///////////////////////////////////////////////////////////////////////////////////
void GexWebHistory::forward()
{
    if (canGoForward())
    {
        ++m_nCurrentPage;
        m_urlCurrent = m_lstUrl.at(m_nCurrentPage);

        m_pWebBrowser->linkClicked(m_urlCurrent, true);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexWebHistory::canGoBack() const
//
// Description	:	Returns true if there is an item preceding the current item
//					in the history; otherwise returns false.
//
///////////////////////////////////////////////////////////////////////////////////
bool GexWebHistory::canGoBack() const
{
    if (m_nCurrentPage > 0 && m_nCurrentPage < m_lstUrl.count())
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexWebHistory::canGoForward() const
//
// Description	:	Returns true if we have an item to go forward to;
//					otherwise returns false
//
///////////////////////////////////////////////////////////////////////////////////
bool GexWebHistory::canGoForward() const
{
    if (m_nCurrentPage >= 0 && m_nCurrentPage < m_lstUrl.count()-1)
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	const QUrl& GexWebHistory::currentUrl() const
//
// Description	:	Returns the current url
//
///////////////////////////////////////////////////////////////////////////////////
const QUrl& GexWebHistory::currentUrl() const
{
    return m_urlCurrent;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexWebHistory::addUrl(const QUrl& url)
//
// Description	:	Add an url in the history
//
///////////////////////////////////////////////////////////////////////////////////
void GexWebHistory::addUrl(const QUrl& url)
{
    if (url != m_urlCurrent)
    {
        while (m_nCurrentPage < (m_lstUrl.count()-1))
            m_lstUrl.removeLast();

        if(!m_lstUrl.isEmpty())
        {
            QString lUrlToString(url.toString());
            QString lLastUrlToString(m_lstUrl.last().toString());
            if(lUrlToString.contains(lLastUrlToString))
                return;
        }

        // splash screen doens't count
        if (!url.url().contains("_gex_home_splash.htm", Qt::CaseInsensitive))
        {
            m_lstUrl.append(url);
            m_nCurrentPage	= m_lstUrl.count()-1;
            m_urlCurrent	= url;
        }
    }
}

