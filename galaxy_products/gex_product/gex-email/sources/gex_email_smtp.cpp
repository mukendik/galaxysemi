/////////////////////////////////////////////////////////////////////////////////////
// Email sending object using smtp
/////////////////////////////////////////////////////////////////////////////////////
#include <gstdl_jwsmtp.h>
#include <gstdl_utils_c.h>

#include "gex_email_smtp.h"

#include <QStringList>


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexEmailSmtp object
/////////////////////////////////////////////////////////////////////////////////////
CGexEmailSmtp::CGexEmailSmtp(QObject * pParent) : CGexEmailSend(CGexEmailSend::sendType_smtp, pParent)
{
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexEmailSmtp::~CGexEmailSmtp()
{
}

void CGexEmailSmtp::setPort(const QString& strPort)
{
	m_uiPort = strPort.toUInt();

	emit sSettingsChanged();
}

void CGexEmailSmtp::setSmtpServer(const QString& strSmptServer)
{
	m_strSmtpServer = strSmptServer;

	emit sSettingsChanged();
}

/////////////////////////////////////////////////////////////////////////////////////
// Send email
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailSmtp::Send(const QString& strFrom, const QStringList& strlistTo, const QString& strSubject, const QString& strBody, const QStringList& strlistAttachement, QString& strResponse, int nBodyFormat /*= GEXEMAIL_BODYFORMAT_TEXT*/)
{
	strResponse = "";

	jwsmtp::mailer mail(false, m_uiPort);

	// Set server
	QByteArray	pString = m_strSmtpServer.toLatin1();
	if((pString.isEmpty() == true) || (mail.setserver(pString.constData()) == false))
	{
		strResponse = "Couldn't set server to " + m_strSmtpServer;
		strResponse += ".";
		return false;
	}

	// Set From field
	pString = strFrom.toLatin1();
	if((pString.isEmpty() == true) || (mail.setsender(pString.constData()) == false))
	{
		strResponse = "Couldn't set source address to " + strFrom;
		strResponse += ".";
		return false;
	}

	// Set subject field
	pString = strSubject.toLatin1();
	if((pString.isEmpty() == false) && (mail.setsubject(pString.constData()) == false))
	{
		strResponse = "Couldn't set subject to " + strSubject;
		strResponse += ".";
		return false;
	}

	// Set body
	pString = strBody.toLatin1();
	if(strBody.isEmpty() == false)
	{
		bool bStatus;
		if(nBodyFormat == GEXEMAIL_BODYFORMAT_HTML)
			bStatus = mail.setmessage(pString.constData(), MAILER_BODYTYPE_HTML);
		else
			bStatus = mail.setmessage(pString.constData(), MAILER_BODYTYPE_TEXT);
		if(bStatus == false)
		{
			strResponse = "Couldn't set body to " + strBody;
			strResponse += ".";
			return false;
		}
	}

	// Add attachements
	QStringList::const_iterator it;
	for(it = strlistAttachement.begin(); it != strlistAttachement.end(); ++it)
	{
		pString = (*it).toLatin1();
		if(mail.attach(pString.constData()) == false)
		{
			strResponse = "Couldn't add attchment " + *it;
			strResponse += ".";
			return false;
		}
	}

	// Add recipients
	if(strlistTo.isEmpty() == true)
	{
		strResponse = "No recipients.";
		return false;
	}
	for(it = strlistTo.begin(); it != strlistTo.end(); ++it)
	{
		pString = (*it).toLatin1();
		if(mail.addrecipient(pString.constData()) == false)
		{
			strResponse = "Couldn't add recipient " + *it;
			strResponse += ".";
			return false;
		}
	}

	// Set username to "" to avoid authentication
	mail.username("");

	// Send email
	mail.send();

	// Check response
	strResponse = mail.response().c_str();
	strResponse.remove('\r');
	strResponse.remove('\n');
	return strResponse.startsWith("250");
}

/////////////////////////////////////////////////////////////////////////////////////
// Set default settings
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailSmtp::DefaultSettings(void)
{
	m_strSmtpServer = "<your smtp server>";
	m_uiPort		= 25;
}

/////////////////////////////////////////////////////////////////////////////////////
// Save settings
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailSmtp::SaveSettings(FILE* hSettingsFile)
{
	// Smtp section
	if(ut_WriteSectionToIniFile(hSettingsFile,"Smtp") != UT_ERR_OKAY)
		return false;

	QByteArray pString = m_strSmtpServer.toLatin1();
	ut_WriteEntryToIniFile(hSettingsFile,"SmtpServer",pString.data());
	
	pString = QString::number(m_uiPort).toLatin1();
	ut_WriteEntryToIniFile(hSettingsFile,"SmtpPort",pString.data());
	
	fputs("\n",hSettingsFile);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Read settings
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailSmtp::LoadSettings(FILE* hSettingsFile)
{
	int		iBufferSize = 512;
	int		iLastCheckedLine = 0;
	char	szString[iBufferSize];

	// Smtp section
	if(ut_ReadFromIniFile(hSettingsFile,"Smtp","SmtpServer",szString,iBufferSize,&iLastCheckedLine,0) != UT_ERR_OKAY)
		return false;
	else
		m_strSmtpServer = szString;

	if(ut_ReadFromIniFile(hSettingsFile,"Smtp","SmtpPort",szString,iBufferSize,&iLastCheckedLine,0) != UT_ERR_OKAY)
		return false;
	else
	{
		QString strPort(szString);
		m_uiPort = strPort.toInt();
	}
	
	return true;
}
