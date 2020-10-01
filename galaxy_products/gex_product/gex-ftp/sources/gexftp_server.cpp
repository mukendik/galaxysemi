/****************************************************************************
** class CGexFtpServer
****************************************************************************/
#include <gqtl_sysutils.h>


#include "gexftp_server.h"


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexFtpServer.
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpServer::CGexFtpServer()
{
	m_uiFtpPortNb = 0;
	m_bProfileEnabled = true;
	m_bUseDateTimeWindow = false;
	m_bRecursiveSearch = false;
	m_bDisabledDateTimeCheck = false;
	m_iMaxStringSize = 1023;
}


/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpServer::~CGexFtpServer()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// Setters...
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtpServer::setSettingsName(const QString& strSettingsName, QString &strErrorMessage)
{
	if (strSettingsName.size() >= m_iMaxStringSize)
	{
		strErrorMessage = "Maximum length exceeded for setting name field! "
						  "Max. set to " + QString::number(m_iMaxStringSize) +" characters.";
		return false;
	}


	m_strSettingsName = strSettingsName;
	return true;
}

bool CGexFtpServer::setFtpSiteURL(const QString& strFtpSiteURL, QString &strErrorMessage)
{
	if (strFtpSiteURL.size() >= m_iMaxStringSize)
	{
		strErrorMessage = "Maximum length exceeded for site url field! "
						  "Max. set to " + QString::number(m_iMaxStringSize) +" characters.";
		return false;
	}

	m_strFtpSiteURL = strFtpSiteURL.trimmed();
	return true;
}

bool CGexFtpServer::setLogin(const QString& strLogin, QString &strErrorMessage)
{
	if (strLogin.size() >= m_iMaxStringSize)
	{
		strErrorMessage = "Maximum length exceeded for login field! "
						  "Max. set to " + QString::number(m_iMaxStringSize) +" characters.";
		return false;
	}

	m_strLogin = strLogin.trimmed();
	return true;
}

bool CGexFtpServer::setPassword(const QString& strPassword, QString &strErrorMessage)
{
	if (strPassword.size() >= m_iMaxStringSize)
	{
		strErrorMessage = "Maximum length exceeded for password field! "
						  "Max. set to " + QString::number(m_iMaxStringSize) +" characters.";
		return false;
	}

	m_strPassword = strPassword.trimmed();
	return true;
}

bool CGexFtpServer::setLocalDir(const QString& strLocalDir, QString &strErrorMessage)
{
	if (strLocalDir.size() >= m_iMaxStringSize)
	{
		strErrorMessage = "Maximum length exceeded for local dir field! "
						  "Max. set to " + QString::number(m_iMaxStringSize) +" characters.";
		return false;
	}

	m_strLocalDir = strLocalDir;
	return true;
}

bool CGexFtpServer::setFileExtensions(const QString& strFileExtensions, QString &strErrorMessage)
{
	if (strFileExtensions.size() >= m_iMaxStringSize)
	{
		strErrorMessage = "Maximum length exceeded for file extensions field! "
						  "Max. set to " + QString::number(m_iMaxStringSize) +" characters.";
		return false;
	}

	m_strFileExtensions = strFileExtensions;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Normalize path
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtpServer::setRemoteDir(const QString& strRemoteDir, QString &strErrorMessage)
{
	if (m_strSettingsName.size() >= m_iMaxStringSize)
	{
		strErrorMessage = "Maximum length exceeded for setting remote dir field! "
						  "Max. set to " + QString::number(m_iMaxStringSize) +" characters.";
		return false;
	}

	m_strRemoteDir = strRemoteDir;
	while(m_strRemoteDir.startsWith('/') || m_strRemoteDir.startsWith('\\'))
	{
		m_strRemoteDir.remove(0,1);
	}

	while(m_strRemoteDir.endsWith('/') || m_strRemoteDir.endsWith('\\'))
	{
		m_strRemoteDir.remove(m_strRemoteDir.length() - 1,1);
	}

	m_strRemoteDir.replace("\\", "/");
	m_strRemoteDir = "/" + m_strRemoteDir;

	return true;
}
