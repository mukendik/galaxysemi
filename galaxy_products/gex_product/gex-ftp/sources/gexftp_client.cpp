/////////////////////////////////////////////////////////////////////////////////////
// Ftp client object wrapper
/////////////////////////////////////////////////////////////////////////////////////

#include "gexftp_client.h"
#include "gexftp_settings_widget.h"
#include "gexftp_constants.h"
#include "gexftp_download.h"

#include <gstdl_utils_c.h>
#include <gstdl_membuffer.h>
#include <gstdl_crypto.h>


extern void WriteDebugMessageFile(const QString & strMessage);	// Write debug message too trace file

/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexFtpClient object
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpClient::CGexFtpClient(const QString & strApplicationPath, CGexFtpSettings * pDataSettings, QObject * pParent)
	: QObject(pParent)
{
	m_strApplicationPath	= strApplicationPath;
	m_pDataSettings			= pDataSettings;
	m_iConsecutiveTries		= 0;
	m_pFtpDownload			= new CGexFtpDownload(m_pDataSettings, this);

	connect(m_pFtpDownload, SIGNAL(sTransferError(const QString &, const QString &)),	this, SLOT(OnFtpTransferError(const QString &, const QString &)));
	connect(m_pFtpDownload, SIGNAL(sTransferDone(const QString &, unsigned int)),		this, SLOT(OnTransferDone(const QString &, unsigned int)));
	connect(m_pFtpDownload, SIGNAL(sFilesToTransfer(const QString &, unsigned int, unsigned int)),	this, SLOT(OnFilesToTransfer(const QString &, unsigned int, unsigned int)));
	connect(m_pFtpDownload, SIGNAL(sFtpServerDateTime(const QString &, const QDateTime &, const QDateTime &)),	this, SLOT(OnFtpServerDateTime(const QString &, const QDateTime &, const QDateTime &)));
	connect(m_pFtpDownload, SIGNAL(sFileTransferred(const QString &, const QString &)), this, SLOT(OnFileTransferred(const QString &, const QString &)));
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpClient::~CGexFtpClient()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp server date/time
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpClient::OnFtpServerDateTime(const QString & strProfileName, const QDateTime & clFtpServerDateTime, const QDateTime & clServerTimeLimit)
{
	// Emit signal specifying the Ftp server date/time
	emit sFtpServerDateTime(strProfileName, clFtpServerDateTime, clServerTimeLimit);
}

/////////////////////////////////////////////////////////////////////////////////////
// Nb of files to transfer on current Ftp server
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpClient::OnFilesToTransfer(const QString & strProfileName, unsigned int uiNbFilesToTransfer, unsigned int uiNbFilesIgnored)
{
	// Emit signal specifying the nb of files to be downloaded from current Ftp server
	if(uiNbFilesToTransfer > 0)
		emit sFtpServerFilesToTransfer(strProfileName, uiNbFilesToTransfer, uiNbFilesIgnored);
}

/////////////////////////////////////////////////////////////////////////////////////
// File transferred on current Ftp server
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpClient::OnFileTransferred(const QString & strProfileName, const QString & strFileName)
{
	// Emit signal specifying a file has been transferred from current Ftp server
	emit sFtpServerFileTransferred(strProfileName, strFileName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp transfer done on current Ftp server
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpClient::OnTransferDone(const QString & strProfileName, unsigned int uiNbFilesTransferred)
{
	// Emit signal specifying that files have been downloaded from current Ftp server
	if(uiNbFilesTransferred > 0)
		emit sFtpServerChecked(strProfileName, uiNbFilesTransferred);
	
	m_pFtpDownload->stop();

	// If no connection problem, lets go to next profile
	if (m_pFtpDownload->isConnectionOK() || m_iConsecutiveTries >= m_pDataSettings->dataGeneral().maxReconnectionAttempts())
	{
		m_iConsecutiveTries = 0;
		// Get next enabled profile
		m_itServerIterator++;
		while((m_itServerIterator != m_pDataSettings->dataFtp().lstProfile().constEnd()) && ((*m_itServerIterator).profileEnabled() == false))
			// Next Ftp server
			m_itServerIterator++;

		if(m_itServerIterator == m_pDataSettings->dataFtp().lstProfile().constEnd())
		{
			// Emit signals specifying that all Ftp servers have been checked
			emit sFtpServersChecked();
			return;
		}
	}
	else
	{
		m_iConsecutiveTries++;
		WriteDebugMessageFile("Connection retry #" + QString::number(m_iConsecutiveTries) + "...");
	}

	// Emit a signal to specify that we are checking current server
	emit sCheckFtpServer((*m_itServerIterator).settingsName());

	// Start transfer for this server
	m_pFtpDownload->start(*m_itServerIterator);
}

/////////////////////////////////////////////////////////////////////////////////////
// Check all Ftp servers for files to get
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpClient::OnCheckFtpServers()
{
	m_itServerIterator = m_pDataSettings->dataFtp().lstProfile().constBegin();

	// Get next enabled profile
	while((m_itServerIterator != m_pDataSettings->dataFtp().lstProfile().constEnd()) && ((*m_itServerIterator).profileEnabled() == false))
		// Next Ftp server
		m_itServerIterator++;

	if(m_itServerIterator == m_pDataSettings->dataFtp().lstProfile().constEnd())
	{
		// Emit signals specifying that all Ftp servers have been checked
		emit sFtpServersChecked();
		return;
	}

	// Emit a signal to specify that we are checking current server
	emit sCheckFtpServer((*m_itServerIterator).settingsName());

	// Start transfer for this server
	m_pFtpDownload->start(*m_itServerIterator);
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying that en error occured during Ftp transfer
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpClient::OnFtpTransferError(const QString & strProfileName, const QString & strError)
{
	// Emit signal advising the main window of the error
	emit sTransferError(strProfileName, strError);
}


