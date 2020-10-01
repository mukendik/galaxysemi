#ifndef GEX_EMAIL_CORE_H
#define GEX_EMAIL_CORE_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-EMAIL Includes
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QtCore>

class CGexEmailSmtp;
class CGexEmailSend;

typedef		const QList<CGexEmailSend*>				td_constlistEmailSystem;
typedef		td_constlistEmailSystem::const_iterator	it_constListEmailSystem; 

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CGexEmailCore
//
// Description	:	This class provides the core methods in order to check, and send
//					the emails.
//
///////////////////////////////////////////////////////////////////////////////////
class CGexEmailCore : public QObject
{
	Q_OBJECT

public:

	enum Status
	{
		StatusReady,
		StatusCountDown,
		StatusCheckingMails,
		StatusProcessingMailFile,
		StatusSendingMail
	};
	
	CGexEmailCore(const QString & strUserHome, const QString & strApplicationDir, const QString & strLocalFolder, const QString & strApplicationName);
	virtual ~CGexEmailCore();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////
	
	const QString&				applicationName() const				{ return m_strApplicationName; }
	const QString&				emailDirectory() const				{ return m_strEmailDirectory; }
	uint						daysToKeepLogFiles() const			{ return m_uiDaysToKeepLogFiles; }
	uint						hours() const						{ return m_nHours; }
	uint						minutes() const						{ return m_nMinutes; }
	uint						seconds() const						{ return m_nSeconds; }
	bool						fullLog() const						{ return m_bFullLog; }
	int							nextSpooling() const				{ return m_nNextSpooling_Sec; }
	const QDateTime&			nextCheck() const					{ return m_dtNextCheck;	}
	bool						runAsService() const				{ return m_bRunningAsService; }
	bool						needSaveSettings() const			{ return m_bNeedSave; }
	td_constlistEmailSystem&	listMailSystem() const				{ return m_listEmailSystems; }
	const CGexEmailSend *		mailSystem() const					{ return m_pEmailSystem; }
	bool						running() const						{ return m_bSpoolingTimerON; }

	void						SetEmailDirectory(const QString& strDirectory);

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////
	
	void	start(void);																				// Starts execution
	void	pause(void);																				// Suspends execution
	void	resume(void);																				// Resumes execution
	void	stop(void);																					// Stop execution
	bool	SaveSettings(void);																			// Save the settings
	void	cleanLogFiles();																			// Clean the log files
	void	Log(int nMsgType, const QString& strMailFile, const QString& strMsg, bool bFilter=true);	// Log message in a file
	bool	init(bool bStartEmailSpoolerAtLaunch, bool bAllowUserToSelectEmailSpoolingDir, bool bRunAsService, bool *pbExitApplication); // Initialize the core object
	

protected:
	
	void	ResumeSpoolingTimer(bool bInitInterval = true);												// Resumes the timer used for the spooling
	void	PauseSpoolingTimer(void);																	// Suspends the timer used for the spooling
	void	EnableSpoolingTimer(void);																	// Enables the spooling timer: a tick every second
	void	DisableSpoolingTimer(void);																	// Disables the spooling timer
	void	StartMailFileTimer(void);																	// Starts MailFile timer
	void	StopMailFileTimer(void);																	// Stops MailFile timer
	bool	ProcessMailFile(const QString& strFileName, const QString& strFullFileName);				// Process 1 mail file
	bool	ReadBody(QString& strBodyFile, QString& strBody);											// Read Body file into string
	bool	GetSender(const QString& strFieldValue, QString& strFrom);									// Validate sender email address
	bool	AddRecipient(const QString& strFieldValue, QStringList& strlistTo, QString& strTo);			// Add to list of recipients
	bool	AddLineToRecipient(const QString& strToLine, QStringList& strlistTo, QString& strTo);		// Add to list of recipients
    bool	AddAttachment(QString& strFieldValue, QStringList& strlistAttachment, QStringList& strlistAttachmentCopies, QStringList& strlistAttachmentsToDeleteLater);	// Add to list of attachments
    void	RemoveFiles(const QStringList& strlistFiles);												// Remove list of files
	void	RemoveFile(const QString& strFullFileName);													// Remove specified file
	void	RenameCorruptedFile(const QString& strFullFileName);										// Rename corrupted files
	void	Error(int nErrorCode, const QString& strMailFile = "", const QString& strArg = "");			// Log error
	void	Info(int nInfoCode, const QString& strMailFile = "", const QString& strArg = "");			// Log info
	
	int		LoadSettings(unsigned int *puiPageToDisplay);												// Read application settings
	bool	LoadEmailDirectoryFromGEX(QString & strMonitoringConfFile);									// Gets email directory from GEX settings
	void	DefaultSettings(void);																		// Set default settings
	bool	IsValidEmailAddress(const QString& strEmailAddress);										// Check correctness of email address
	void	SetNextEmailSpooling(bool bRunNow);															// Calculate next Mail spooling time
    // Get seconds to next Mail spooling, or -1 if greater than threashhold
    int		GetSecondsToNextFtpSpooling();

    // Delete files after a delay
    void	DeleteFiles_Later(const int &lNumSeconds=-3600);

	void	OnStartSpooling();																			// Start spooling 
    void	OnStopSpooling();																			// Stop spooling

// Overrides
protected:
	
	enum ErrorCode
	{
		eNoError,
		eOpenMailFile,
		eOpenBodyFile,
		eOpenAttachment,
		eCopyAttachment,
		eOpenMaillistFile,
		eNoSource,
		eNoRecipient,
		eInvalidSenderAddress,
		eInvalidRecipientAddress,
		eSaveSetttings,
		eLoadSetttings,
		eProcess,
		eSend
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
		iEmailSent
	};

	QString						m_strEmailDirectory;							// directory where the .mail files are saved
	uint						m_uiDaysToKeepLogFiles;							// Number of days to keep log files
	uint						m_nHours;										// Number of hours before next check
	uint						m_nMinutes;										// Number of minutes before next check
	uint						m_nSeconds;										// Number of seconds befoore next check
	bool						m_bFullLog;										// Log all messages or error messages only

	QList<CGexEmailSend*>		m_listEmailSystems;								// List of systems to send mails

	int							m_nLogFilter;									// Level of log (All or error)
	int							m_nNextSpooling_Sec;							// Seconds before next check
	QDateTime					m_dtNextCheck;									// Next check of Ftp servers
	QTimer*						m_pSecTimer;									// Timer
	QTime*						m_pLogCleanupTime;								// Time last clean up has been made
	QString						m_strLocalFolder;								// Folder to write config and log files
	QString						m_strUserHome;									//
	QString						m_strApplicationDir;							// Folder application
	QString						m_strApplicationName;							// Name of the application
	QString						m_strCurrentMailFile;							// Name of the mail faile which is currently processed
	bool						m_bSpoolingTimerON;								// Indicates if the timer is started
	bool						m_bSpoolingInProgress;							// Indicates if spooling is in progress
	bool						m_bRetrieveListOfMailFiles;						// Needs to retrieve the list of mail files if sets to true
	QStringList					m_strlistFilesToRemove;							// List of files to remove
    QStringList					m_strlistFilesToRemove_Later;					// List of files to remove later (after 1 hour)
    QString						m_strSettingsFile;								// File path of the setting file
	uint						m_uiLogCleanupInterval;							// Interval of time to clean up the log files
	bool						m_bRunningAsService;							// Indicates if application is running as service
	bool						m_bAllowUserToSelectEmailSpoolingDir;			// Indicates if the mail directory can be changed
	bool						m_bFirstTimerEvent;								// Indicates if the timer event is the first
	bool						m_bNeedSave;									// Indicates if settings need to be saved

	CGexEmailSmtp*				m_pSmtp;										// SMTP email system
	CGexEmailSend*				m_pEmailSystem;									// Pointer on the active mail system

signals:
    void						sEmailDirectoryChanged(const QString &);
    void						sMainSelectionChanged(int);
    void						sStartSpooling();
    void						sStopSpooling();
    void						sCriticalMessage(const QString&);
	void						sStatusChanged(CGexEmailCore::Status, const QString& strArg1 = "", const QString& strArg2 = "");
	void						sLogInfo(int nMsgType, const QString& strMailFile, const QString& strMsg, const QString& strDateTime);
	void						sSpoolingTimer(bool);
	void						sSettingsChanged();
	
public slots:
	void						setDaysToKeepFiles(int nDays);					// Sets the number of days Gex-email keeps the log files
	void						setHours(int nHours);							// Sets the number of hours of the timer
	void						setMinutes(int nMinutes);						// Sets the number of minutes of the timer
	void						setSeconds(int nSeconds);						// Sets the number of seconds of the timer
	void						setFullLog(int nFullLog);						// Sets if gex-email log all or error message only

protected slots:
   
	void						OnSecTimerDone();								// Called on timer event (every second)
    void						OnCheckForMailFile();							// Check if .mail file to be processed
   	void						OnSettingsChanged();							// Called when some Email settings have changed
};

#endif // GEX_EMAIL_CORE_H
