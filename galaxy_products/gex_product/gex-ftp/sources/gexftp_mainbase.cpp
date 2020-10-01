#include "gexftp_mainbase.h"


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexFtpMainBase::CGexFtpMainBase(const QString & strUserHome, const QString & strApplicationDir, const QString & strLocalFolder, const QString & strApplicationName)
{
	m_pCore = new CGexFtpCore(strUserHome, strApplicationDir, strLocalFolder, strApplicationName);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexFtpMainBase::~CGexFtpMainBase()
{
	if (m_pCore)
	{
		delete m_pCore;
		m_pCore = NULL;
	}
}
