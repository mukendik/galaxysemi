/****************************************************************************
** class CGexFtpServer
****************************************************************************/

#ifndef CGexFtpServer_H
#define CGexFtpServer_H

#include <qstring.h>
#include <qdatetime.h>

class CGexFtpServer
{
public:
	
	CGexFtpServer();
	~CGexFtpServer();

	unsigned int		ftpPortNb() const							{ return m_uiFtpPortNb; }
	bool				profileEnabled() const						{ return m_bProfileEnabled; }
	bool				useDateTimeWindow() const					{ return m_bUseDateTimeWindow; }
	bool				recursiveSearch() const						{ return m_bRecursiveSearch; }
	bool				disabledDateTimeCheck() const				{ return m_bDisabledDateTimeCheck;}
	QString				settingsName() const						{ return m_strSettingsName; }
	QString				ftpSiteURL() const							{ return m_strFtpSiteURL; }
	QString				login() const								{ return m_strLogin; }
	QString				password() const							{ return m_strPassword; }
	QString				remoteDir() const							{ return m_strRemoteDir; }
	QString				localDir() const							{ return m_strLocalDir; }
	QString				fileExtensions() const						{ return m_strFileExtensions; }
	QString				filePolicy() const							{ return m_strFilePolicy; }
    QString				downloadOrder() const						{ return m_strDownloadOrder; }
    const QDateTime&	dateFrom() const							{ return m_clDateFrom; }
	const QDateTime&	dateTo() const								{ return m_clDateTo; }

	void				setFtpPortNb(unsigned int uiFtpPortNb)				{ m_uiFtpPortNb = uiFtpPortNb; }
	void				setProfileEnabled(bool bProfileEnabled)				{ m_bProfileEnabled = bProfileEnabled; }
	void				setUseDateTimeWindow(bool bUseDateTimeWindow)		{ m_bUseDateTimeWindow = bUseDateTimeWindow; }
	void				setRecursiveSearch(bool bRecursiveSearch)			{ m_bRecursiveSearch = bRecursiveSearch; }
	void				setDisabledDateTimeCheck(bool bDisabledDateTimeCheck){ m_bDisabledDateTimeCheck = bDisabledDateTimeCheck;}
	void				setFilePolicy(const QString& strFilePolicy)			{ m_strFilePolicy = strFilePolicy; }
    void				setDownloadOrder(const QString& strDownloadOrder)	{ m_strDownloadOrder = strDownloadOrder; }
    void				setDateFrom(const QDateTime& dateFrom)				{ m_clDateFrom = dateFrom; }
	void				setDateTo(const QDateTime& dateTo)					{ m_clDateTo = dateTo; }
	bool				setSettingsName(const QString& strSettingsName, QString &strErrorMessage);
	bool				setFtpSiteURL(const QString& strFtpSiteURL, QString &strErrorMessage);
	bool				setLogin(const QString& strLogin, QString &strErrorMessage);
	bool				setPassword(const QString& strPassword, QString &strErrorMessage);
	bool				setRemoteDir(const QString& strRemoteDir, QString &strErrorMessage);
	bool				setLocalDir(const QString& strLocalDir, QString &strErrorMessage);
	bool				setFileExtensions(const QString& strFileExtensions, QString &strErrorMessage);

protected:
	
	QString			m_strSettingsName;
	QString			m_strFtpSiteURL;
	unsigned int	m_uiFtpPortNb;
	QString			m_strLogin;
	QString			m_strPassword;
	QString			m_strRemoteDir;
	QString			m_strLocalDir;
	QString			m_strFileExtensions;
	QString			m_strFilePolicy;
    QString			m_strDownloadOrder;
    bool			m_bProfileEnabled;
	bool			m_bUseDateTimeWindow;
	bool			m_bRecursiveSearch;
	bool			m_bDisabledDateTimeCheck;
	QDateTime		m_clDateFrom;
	QDateTime		m_clDateTo;
	int				m_iMaxStringSize;
};

#endif // CGexFtpServer_H
