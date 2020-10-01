#ifndef GEX_FTP_CORE_H
#define GEX_FTP_CORE_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-FTP Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexftp_constants.h"
#include "gexftp_settings.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QtCore>

class QTimer;
class CGexFtpClient;

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CGexFtpCore
//
// Description	:	This class provides the core methods in order to check, and get
//					the file from Ftp servers.
//
///////////////////////////////////////////////////////////////////////////////////
class CGexFtpCore : public QObject
{

	Q_OBJECT

public:

	enum Status
	{
		StatusReady,
		StatusCountDown,
		StatusCheckingFtpServers,
		StatusCheckingFtpServer
	};

	CGexFtpCore(const QString & strUserHome, const QString & strApplicationDir, const QString & strLocalFolder, const QString & strApplicationName);
	~CGexFtpCore();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////
	
	CGexFtpSettings *			settings()							{ return &m_dataSettings; }
	const CGexFtpClient *		ftpClient()	const					{ return m_pFtpClient;	}
	const QString&				applicationName() const				{ return m_strApplicationName; }
	const QString&				applicationDir() const				{ return m_strApplicationDir; }
	int							nextSpooling() const				{ return m_nNextSpooling_Sec; }
	const QDateTime&			nextCheck() const					{ return m_dtNextCheck;	}
	bool						runAsService() const				{ return m_bRunningAsService; }
	bool						running() const						{ return m_bSpoolingTimerON; }

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	bool	init(bool bStartFtpSpoolerAtLaunch, bool bRunAsService, bool *pbExitApplication);
	void	start(void);																				// Starts execution
	void	pause(void);																				// Suspends execution
	void	resume(void);																				// Resumes execution
	void	stop(void);																					// Stop execution
	void	cleanLogFiles();
	void	Log(int nMsgType, const QString& strFtpServer, const QString& strMsg, bool bFilter=true);	// Add to log file and log page

protected:

	void	ResumeSpoolingTimer(bool bInitInterval = true);					// Resumes the spooling timer
	void	PauseSpoolingTimer(void);										// Pauses the spooling timer
	void	EnableSpoolingTimer(void);										// Enables the spooling timer: a tick every second
	void	DisableSpoolingTimer(void);										// Disables the spooling timer
	void	StartFtpCheckTimer(void);										// Starts timer to check Ftp servers
	void	RemoveFile(const QString& strFullFileName);						// Removes specified file
	void	RenameCorruptedFile(const QString& strFullFileName);			// Renames corrupted files
	void	SetNextFtpSpooling(bool bRunNow);								// Calculates next Ftp server spooling time
	int		GetSecondsToNextFtpSpooling();									// Gets seconds to next Ftp server spooling, or -1 if greater than threashhold

	void	Error(int nErrorCode, const QString& strFtpServer = "", const QString& strArg = "");			// Log error
	void	Info(int nInfoCode, const QString& strFtpServer = "", const QString& strArg = "");				// Log info
	void	LogInfo(int nMsgType, const QString& strMailFile, const QString& strMsg, const QString& strDateTime);

	void	onStartSpooling();																				// Start spooling 
    void	onStopSpooling();																				// Stop spooling

protected slots:
	
    void	OnSecTimerDone();																				// Called every second
	void	OnCheckFtpServer(const QString& strServerName);													// The Ftp client is checking specified ftp server for files to download
    void	OnFtpServersChecked();																			// Received a signal specifying that all Ftp servers have been checked
	void	OnFtpTransferError(const QString & strProfileName, const QString & strError);					// Received a signal specifying that an error occured during Ftp transfer
	void	OnFtpServerChecked(const QString & strProfileName, unsigned int uiNbFilesTransferred);			// Received a signal specifying that one Ftp server has been checked
	void	OnFtpServerDateTime(const QString & strProfileName, const QDateTime & clFtpServerDateTime, const QDateTime & clServerTimeLimit);	// Received a signal specifying the Ftp server date/time, and limit date/time (more recent files ignored)
    void	OnFtpServerFilesToTransfer(const QString & strProfileName, unsigned int uiNbFilesToTransfer, unsigned int uiNbFilesIgnored);		// Received a signal specifying the nb of files to be transferred on one Ftp server
    void	OnFtpServerFileTransferred(const QString & strProfileName, const QString & strFileName);		// Received a signal specifying that a file has been transferred on one Ftp server

	void	onSettingsLoaded(bool bResult, const QString& strSettingsFile);									// Received a signal that settings have been loaded
	void	onSettingsSaved(bool bResult, const QString& strSettingsFile);									// Received a signal that settings have been saved
	void	onHeartbeatDone();
	
signals:
	
	void	sStartSpooling();
    void	sStopSpooling();
	void	sStatusChanged(CGexFtpCore::Status, const QString& strArg1 = "", const QString& strArg2 = "");
	void	sLogInfo(int nMsgType, const QString& strMailFile, const QString& strMsg, const QString& strDateTime);
	void	sSpoolingTimer(bool);
	void	sStartDownload(const QString&);
	void	sStopDownload();
	void	sCriticalMessage(const QString&);

protected:

	enum ErrorCode
	{
		eNoError,
		eSaveSetttings,
		eLoadSetttings,
		eTransferError
	};
	enum InfoCode
	{
		iStartApp,
		iStopApp,
		iStartSpool,
		iStopSpool,
		iLoadSettings,
		iSaveSettings,
		iRemoveLogFile,
		iCleanLogFiles,
		iTransferDone,
		iServerDateTime,
		iFilesToTransfer,
		iFileTransferred
	};
	
	QString						m_strLocalFolder;								// Folder to write config and log files
	QString						m_strUserHome;									// Folder user
	QString						m_strApplicationDir;							// Folder application
	QString						m_strApplicationName;							// Name of the application

	QTimer *					m_pSecTimer;									// Main timer (every second)
	QTimer *					m_pHeartbeatTimer;								// Heart beat timer
	QTime *						m_pLogCleanupTime;								// Timer used to clean up the log files

	bool						m_bRunningAsService;							// Set to true when gex-ftp is running as service
	bool						m_bFirstTimerEvent;								// Set to true to drop the first timer event
	bool						m_bSpoolingTimerON;								// Set to true when spooling timer is ON
	bool						m_bSpoolingInProgress;							// Set to true when a Ftp server spooling is in progress
	int							m_nNextSpooling_Sec;							// Time remaining up to the next spooling (seconds)
	QDateTime					m_dtNextCheck;									// Next check of Ftp servers


	CGexFtpSettings				m_dataSettings;									// Settings
	CGexFtpClient *				m_pFtpClient;									// FTP client	
};

#endif // GEX_FTP_CORE_H
