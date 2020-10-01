#include "gex_web_browser.h"

///////////////////////////////////////////////////////////////////////////////////
// Name			:	GexWebBrowserAction
// Description	:	Class which defines action to do when loading an url

GexWebBrowserAction::GexWebBrowserAction(Action eAction, const QString &strLink, int nIndex, bool forwardBack)
{
    m_eAction       = eAction;
    m_nIndex        = nIndex;
    m_strLink       = strLink;
    m_forwardBack   = forwardBack;
}

GexWebBrowserAction::GexWebBrowserAction(const GexWebBrowserAction&)
{

}

GexWebBrowserAction::~GexWebBrowserAction()
{

}

GexWebBrowserAction& GexWebBrowserAction::operator=(const GexWebBrowserAction& webBrowserAction)
{
    if (this != &webBrowserAction)
    {
        m_eAction	= webBrowserAction.m_eAction;
        m_nIndex	= webBrowserAction.m_nIndex;
        m_strLink	= webBrowserAction.m_strLink;
    }

    return *this;
}
