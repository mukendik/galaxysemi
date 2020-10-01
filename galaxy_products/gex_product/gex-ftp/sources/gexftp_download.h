#ifndef GEX_FTP_DOWNLOAD_H
#define GEX_FTP_DOWNLOAD_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-FTP Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexftp_server.h"
#include "gexftp_qftp.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QtCore>

class CGexFtpSettings;

class CGexFtpDownload : public QObject
{

	Q_OBJECT

public:

	CGexFtpDownload(CGexFtpSettings * pDataSettings, QObject * pParent);
	~CGexFtpDownload();

	void			start(const CGexFtpServer& ftpServer);					// Start the ftp transfer for this server
	void			stop();													// Close the ftp transfer
	void			abort(int nAbortType);									// Abort the current ftp transfer
	bool			isConnectionOK()	{ return m_bIsConnectionOK; }		// Return true if connection is OK

	enum ErrorCode
	{
		eNoError,
		eFtpError,
		eCreateTimeStampFile,
		eReadServerTime,
		eCorruptedServerTime,
		eReadTimeStampFile,
		eCreateLocalFile,
		eRemoveLocalFile,
		eOperationAborted,
		eOperationTimedOut,
		eFtpConnectionBroken,
		eConnect,
		eInvalidDateTime_RemoteFile,
		eInvalidDateTime_RemoteFiles
	};

protected:

	enum InfoCode
	{
		iStateUnconnected,
		iStateHostLookup,
		iStateConnecting,
		iStateConnected,
		iStateLoggedIn,
		iStateClosing,
		iSequenceConnect,
		iSequenceSystem,
		iSequenceLogin,
		iSequenceChdir,
		iSequenceRemoveTimestampFile,
		iSequenceGetList,
		iSequenceSleep,
		iSequenceVerify,
		iSequencePutTimestampFile,
		iSequenceGetTimestampFile,
		iSequenceGet,
		iSequenceRemoteRemove,
		iSequenceRemoteRename_1,
		iSequenceRemoteRename_2,
		iSequenceRemoteLeave,
		iSequenceCleanup,
		iSequenceCheckFile,
		iCommandStarted,
		iCommandFinishedOK,
		iCommandFinishedNOK
	};

	void						LogInfo(int nInfoCode);									// Logs informations
	void						LogError(int nErrorCode, bool bLocal=false);			// Logs errors
	void						LogMessage(int nErrorCode, const QString& strMessage);	// Log message

	void						ResolveHost();									// Try to resolve host name domain<->IP
	void						Cleanup(bool bNoFtpActions = false);			// Clean up the ftp transfer
	void						CorrectServerTime(QDateTime & clDateTime);		// Correct the server time
	void						UpdateListOfFilesToTransfer();					// Update list of files to transfer and file containg list of transferred files
	bool						AddToTransferredFilesList();					// Add transferred file to list of files transferred
    bool						CheckFileSize(qint64 &ui64_LocalSize, qint64 &ui64_RemoteSize);		// Make sure transferred file is same size as remote file
	bool						StartFileTransfer();							// Begins the files transfer's
	bool						OpenLocalFile();								// Opens local file for get command
	void						CloseLocalFile(bool bWriteMissingData);			// Closes local file after get completed
	void						reset();										// Reset information
	void						pwdAndList();									// Launch pwd raw command then list
	void						ftp_startCommand(GexFtpQFtp::Command, const QList<QVariant> &args = QList<QVariant>());// Launch Ftp command method and set the timeout interval
	
	CGexFtpSettings *			m_pDataSettings;
	GexFtpQFtp *				m_pclFtp;						// Ptr on QT FTP object
	CGexFtpServer				m_clFtpServer;					// Settings of current Ftp server
	QString						m_strFileInTransferLocalName;	// Name of the file (on local system) currently being transferred (put/get)
	QString						m_strFileInTransferRemoteName;	// Name of the file (on remote system) currently being transferred (put/get)
	QString						m_strFinalLocalName;			// The file in transfer should be renamed to this name after transfer OK
	QString						m_strTransferredListFileName;	// In leave mode, name of the file with the list of files already transferred
	QString						m_strFtpSiteName;				// Domain name of current Ftp server
	QString						m_strFtpSiteIP;					// IP of Ftp server
	QString						m_strFtpServer;					// String to use to connect to Ftp server (IP if available, domain name else)
	QString						m_strTimestampFileLocalReadbackName;	// Name of file (on local system) used to read back the timestamp
	QString						m_strTimestampFileLocalName;	// Name of file (on local system) used to get Ftp server's current time
	QString						m_strTimestampFileRemoteName;	// Name of file (on remote system) used to get Ftp server's current time
	int							m_nCurrentSequence;				// Current task in Ftp sequence
	QList<QUrlInfo>				m_listFiles_1;					// List of files retrieved from server at first trial
	QList<QUrlInfo>				m_listFiles_2;					// List of files retrieved from server at second trial (after a waiting time)
	QDateTime					m_clServerCurrentDateTime;		// Current date/time on remote server
	QDateTime					m_clServerTimeLimit;			// Files more recent than this date/time will be ignored
	QList<QUrlInfo>::iterator	m_itFilesToGet;					// Iterator through files to get during the Get sequence
	bool						m_bLocalTimestampFileCreated;	// Set to true when the timestampfile has been created on the local server
	bool						m_bRemoteTimestampFileCreated;	// Set to true when the timestampfile has been created on the remote server
	QTimer *					m_pTimer;
	QTimer *					m_pSequenceTimer;
	bool						m_bEmitTransferDone;			// Set to true when the 'transfer done' signal should be emitted during next timer
	bool						m_bEmitFilesToTransfer;			// Set to true when the 'files to transfer' signal should be emitted during next timer
	unsigned int				m_uiNbTotalFiles;				// Total files in directory
	unsigned int				m_uiNbFilesToTransfer;			// Nb of files to transfer on current profile
	unsigned int				m_uiNbFilesIgnored;				// Nb of files ignored on current profile (due to date/time too recent...)
	unsigned int				m_uiNbFilesProcessed;			// Nb of files processed (whether transfer is successful or not)
	unsigned int				m_uiNbFilesTransferred;			// Nb of files sucessfully transferred on current profile
	bool						m_bRemoteTimestampFileExists;	// Set to true if a timestamp file exists on the remote server
	QDateTime					m_clCommandStartDateTime;		// Date/Time the current command is started
	QDateTime					m_clGetStartDateTime;			// Date/Time the current GET is started
	bool						m_bCommandInProgress;			// A Ftp command is in progress
	int							m_nSecondsToTimeout;			// Nb of seconds until command timeout
	QFile *						m_pFile;						// Handle on file to download
	QFile *						m_pTimestampFile;				// Handle on timestamp file (upoad)
	QString						m_strRawCommand;				// Current raw command
	QString						m_strSystem;					// Ftp Server's system (reply to SYST command)
	QString						m_strCurrentDirectory;			// Holds current directory
	bool						m_bDisplaySystem;				// Set to true if system should be displayed at next finish signal
	bool						m_bRecurseList;					// Set to true if the list function should return a recursive list
    int							m_nTransferredData;				// Progress step in a Ftp GET command
    int							m_nTotalDataToTransfer;			// Total steps in a Ftp GET command
	unsigned int				m_uiNbFilesCorruptedDate;		// Nb of files with corrupted date/time
	QStringList					m_strCDPaths;					// Hold the paths of the pending cd command
	bool						m_bIsConnectionOK;				// Holds the last error code

protected slots:

	void						ftp_sequencer();
	void						ftp_sequence_connect();
	void						ftp_sequence_system();
	void						ftp_sequence_pwd();
	void						ftp_sequence_chdir();
	void						ftp_sequence_list1();
	void						ftp_sequence_postlist1();
	void						ftp_sequence_removetimestampfile();
	void						ftp_sequence_puttimestampfile();
	void						ftp_sequence_gettimestampfile();
	void						ftp_sequence_list2();
	void						ftp_sequence_verify();
	void						ftp_sequence_get();
	void						ftp_sequence_remoteremove();
	void						ftp_sequence_remoterename_1();
	void						ftp_sequence_remoterename_2();
	void						ftp_sequence_remoteleave();
	void						ftp_sequence_checkfile();
	void						ftp_commandStarted(int id);
	void						ftp_commandFinished(int id, bool error);
	void						ftp_done(bool error);
	void						ftp_stateChanged(int state);
	void						ftp_listInfo(const QUrlInfo &info);
	void						ftp_rawCommandReply(int, const QString &);
	void						OnTimer();
	void						OnTransferProgress(qint64, qint64);
	void						onUserAbort();
	void						onSequenceTimeout();

signals:

	void						sNextSequence();
	void						sTransferError(const QString &, const QString &);
	void						sTransferDone(const QString &, unsigned int);
	void						sFtpServerDateTime(const QString &, const QDateTime &, const QDateTime &);
	void						sFilesToTransfer(const QString &, unsigned int, unsigned int);
	void						sFileTransferred(const QString &, const QString &);

	void						sLogMessage(int, const QString&);
	void						sChangeTimeout(int);
	void						sDownloadProgress(int, int);
	void						sListFileInfo(int, int);
	void						sDownloadInfo(int, int, int);
	void						sTransferStatus(int, const QString&, bool bStart = false);
	void						sStartFileTransfer(const QString&, int, int);
	void						sCleanup();
	void						sFileInfo(const QUrlInfo&);
	void						sMaxFileToDownLoadInfo(int iMaxFileToTransfer);
};

#endif // GEX_FTP_DOWNLOAD_H
