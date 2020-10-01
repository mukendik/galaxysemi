#include "gex_email_mainbase.h"


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexEmailMainBase::CGexEmailMainBase(const QString & strUserHome, const QString & strApplicationDir, const QString & strLocalFolder, const QString & strApplicationName)
{
	m_pCore = new CGexEmailCore(strUserHome, strApplicationDir, strLocalFolder, strApplicationName);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexEmailMainBase::~CGexEmailMainBase()
{
	if (m_pCore)
	{
		delete m_pCore;
		m_pCore = NULL;
	}
}
