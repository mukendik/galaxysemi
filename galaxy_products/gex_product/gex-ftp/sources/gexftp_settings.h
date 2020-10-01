#ifndef GEX_FTP_SETTINGS_H
#define GEX_FTP_SETTINGS_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-FTP Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexftp_server.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QObject>
#include <QTime>
#include <QMap>

class CGexFtpSettingsDataGeneral : public QObject
{

	Q_OBJECT

public:

	CGexFtpSettingsDataGeneral();
	~CGexFtpSettingsDataGeneral();

	bool			every() const							{ return m_bEvery; }
	bool			daily() const							{ return m_bDaily; }
	bool			weekly() const							{ return m_bWeekly; }
	int				everyHour() const						{ return m_nEveryHour; }
	int				everyMinute() const						{ return m_nEveryMinute; }
	int				everySecond() const						{ return m_nEverySecond; }
	const QTime&	dailyAt() const							{ return m_timeDailyAt;	}
	int				dailyEveryNDays() const					{ return m_nDailyEveryNDays; }
	const QTime&	weeklyAt() const						{ return m_timeWeeklyAt; }
	int				weeklyOn() const						{ return m_nWeeklyOn; }
	int				nbDaysToKeepLog() const					{ return m_nNbDaysToKeepLog; }
	bool			fullLog() const							{ return m_bFullLog; }
	bool			ftpTimeout() const						{ return m_bFtpTimeout;	}
	int				ftpTimeoutAfter() const					{ return m_nFtpTimeoutAfter; }
	int				fileAgeBeforeTransfer_Min() const		{ return m_nFileAgeBeforeTransfer_Min; }
	const QString	version() const							{ return m_strVersion; }
	bool			isNewVersion() const					{ return m_bIsNewVersion; }
	int				maxFilePerProfile() const				{ return m_iMaxFilePerProfile; }
	int				maxReconnectionAttempts() const			{ return m_iMaxReconnectionAttempts; }
        bool                    caseInsensitive () const                                { return m_bCaseInsensitive;}
	bool			enableMaxFilePerProfile() const;

	void			reset();

public slots:

	void			setVersion(const QString &strVersion);
	void			checkVersion(const QString &strVersion);
	void			setEvery(bool bEvery);
	void			setDaily(bool bDaily);
	void			setWeekly(bool bWeekly);
	void			setEveryHour(int nHour);
	void			setEveryMinute(int nMinute);
	void			setEverySecond(int nSecond);
	void			setDailyAt(const QTime& timeAt);
	void			setDailyEveryNDays(int nDays);
	void			setWeeklyAt(const QTime& timeAt);
	void			setWeeklyOn(int nDay);
	void			setNbDaysToKeepLog(int nDays);
	void			setFullLog(bool bFullLog);
	void			setFtpTimeout(int nFtpTimeout);
	void			setFtpTimeoutAfter(int nSecond);
	void			setFileAgeBeforeTransfer_Min(int nMinutes);
	void			setEnableMaxFilePerProfile(bool bUse);
	void			setMaxFilePerProfile(int iMaxFile);
	void			setMaxReconnectionAttempts(int iMaxReconnect);
        void			setCaseInsensitive(bool bCaseInsensitive);

private:
	
	QString			m_strVersion;
	bool			m_bEvery;					// Check ftp servers every x hours, minutes, seconds
	bool			m_bDaily;					// Check ftp servers daily
	bool			m_bWeekly;					// Check ftp servers weekly
	int				m_nEveryHour;				// 
	int				m_nEveryMinute;
	int				m_nEverySecond;
	QTime			m_timeDailyAt;
	int				m_nDailyEveryNDays;
	QTime			m_timeWeeklyAt;
	int				m_nWeeklyOn;
	int				m_nNbDaysToKeepLog;
	bool			m_bFullLog;
	bool			m_bFtpTimeout;
	int				m_nFtpTimeoutAfter;
	int				m_nFileAgeBeforeTransfer_Min;
	bool			m_bIsNewVersion;
	bool			m_bEnableMaxFilePerProfile;	// Holds if the max file download per profile has to used or not
	int				m_iMaxFilePerProfile;		// Holds the max file download per profile
	int				m_iMaxReconnectionAttempts;	// Holds the number of reconnection attempts before switching to next profile
        bool                    m_bCaseInsensitive;

signals:

	void			sDataGeneralChanged();
};

typedef	QList<CGexFtpServer>					td_lstProfile;
typedef	td_lstProfile::iterator					td_itProfile;
typedef	td_lstProfile::const_iterator			td_constitProfile;

class CGexFtpSettingsDataFtp : public QObject
{
	Q_OBJECT

public:

	enum addType
	{
		addFromFile = 0,
		addFromGui
	};

	CGexFtpSettingsDataFtp();
	~CGexFtpSettingsDataFtp();

	const td_lstProfile&		lstProfile() const			{ return m_lstProfile; }

	void						addProfile(const CGexFtpServer& ftpServer, addType eType = addFromGui);
	void						removeProfile(const QString& strProfileName);
	bool						findProfile(const QString& strProfileName, td_itProfile& profileFound);
	void						reset();
	
private:

	td_lstProfile				m_lstProfile;

signals:

	void						sDataFtpChanged();
};


class CGexFtpSettings : public QObject
{
	Q_OBJECT
		
public:

	CGexFtpSettings(const QString& strLocalFolder);
	~CGexFtpSettings();

	CGexFtpSettingsDataGeneral&			dataGeneral()				{ return m_dataGeneral; }
	const CGexFtpSettingsDataGeneral&	dataGeneral() const			{ return m_dataGeneral; }
	CGexFtpSettingsDataFtp&				dataFtp()					{ return m_dataFtp; }
	const CGexFtpSettingsDataFtp&		dataFtp() const				{ return m_dataFtp; }
	const QString&						serviceHomeDir() const		{ return m_strServiceHomeDir; }
	const QString&						serviceSettingsFile() const	{ return m_strServiceSettingsFile; }
	const QString&						settingsFile() const		{ return m_strSettingsFile;	}
	bool								dataChanged() const			{ return m_bDataChanged; }
	int									logFilter() const			{ return m_nLogFilter; }
	int									logCleanupInterval() const	{ return m_nLogCleanupInterval;	}
	QMap<int, QString>					errors()					{ return m_mapErrTypeErrMsg;}
	bool								isNewVersion()				{ return m_dataGeneral.isNewVersion();}
	
	void								setServiceHomeDir(const QString& strHomeDir);
	
	void								reset();
	void								beginProfileChange();
	void								endProfileChange();
	void								setRunAsAService(const bool bRunAsAService);

public slots:

	void								save();
	int									load();
	void								saveService();
	int									loadService();

private:

	void								save(const QString& strSettingsFile);
	int									load(const QString strSettingsFile);

	void								CryptPassword(const QString & strPassword, QString & strCryptedPasswordHexa);
	void								DecryptPassword(const QString & strCryptedPasswordHexa, QString & strPassword);	
	void								checkRemoteFolderValidity(CGexFtpServer &clFtpServer);
	int									readIniFile(FILE *hFile, QString strFile, QString strSection, QString strEntry, QString &strValue);
	int									manageReadError(QString strFile, QString strSection, QString strEntry, int iErrorStatus, int iLastCheckedLine, int iBufferSize);

	bool								m_bDataChanged;
	bool								m_bRunAsAService;
	
	QString								m_strSettingsFile;			// 
	QString								m_strServiceHomeDir;
	QString								m_strServiceSettingsFile;
	QMap<int, QString>					m_mapErrTypeErrMsg;
	int									m_nLogCleanupInterval;
	int									m_nLogFilter;

	CGexFtpSettingsDataGeneral			m_dataGeneral;
	CGexFtpSettingsDataFtp				m_dataFtp;

protected slots:

	void								onDataChanged();

signals:

	void								sSettingsChanged();
	void								sSettingsLoaded(bool, const QString&);
	void								sSettingsSaved(bool, const QString&);
	void								sProfileInChange(bool);	
};

#endif // GEX_FTP_SETTINGS_H


