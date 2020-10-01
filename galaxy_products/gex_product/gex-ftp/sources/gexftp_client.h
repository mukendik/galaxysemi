#ifndef CGexFtpClient_H
#define CGexFtpClient_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-FTP Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexftp_settings.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <qobject.h>

class CGexFtpServer;
class CGexFtpDownload;

class CGexFtpClient : public QObject
{
    Q_OBJECT

public:
    
	CGexFtpClient(const QString & strApplicationPath, CGexFtpSettings * pDataSettings, QObject * pParent);
    ~CGexFtpClient();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////
	
	const CGexFtpDownload *		downloader() const			{ return m_pFtpDownload; }

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

signals:

    void		sCheckFtpServer(const QString&);
	void		sFtpServerDateTime(const QString&, const QDateTime&, const QDateTime&);
	void		sFtpServerFilesToTransfer(const QString&, unsigned int, unsigned int);
	void		sFtpServerFileTransferred(const QString&, const QString&);
	void		sFtpServerChecked(const QString&, unsigned int);
	void		sFtpServersChecked();
    void		sTransferError(const QString &, const QString &);

public slots:

	void		OnCheckFtpServers();
	void		OnFtpServerDateTime(const QString & strProfileName, const QDateTime & clFtpServerDateTime, const QDateTime & clServerTimeLimit);
	void		OnFilesToTransfer(const QString & strProfileName, unsigned int uiNbFilesToTransfer, unsigned int uiNbFilesIgnored);
	void		OnFileTransferred(const QString & strProfileName, const QString & strFileName);
	void		OnTransferDone(const QString & strProfileName, unsigned int uiNbFilesTransferred);
    void		OnFtpTransferError(const QString & strProfileName, const QString & strError);

protected:

	QString								m_strApplicationPath;		// Application path
	td_constitProfile					m_itServerIterator;			// Iterator on the server which is checking
	CGexFtpSettings *					m_pDataSettings;			// Application settings
	CGexFtpDownload *					m_pFtpDownload;				// Pointer on the downloader object
	int									m_iConsecutiveTries;			// Hold the number of consecutive tries on the same profile
};

#endif // CGexFtpClient_H
