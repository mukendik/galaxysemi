#include "gexftp_core.h"
#include "gexftp_client.h"

#include <gstdl_utils_c.h>
#include <gqtl_sysutils.h>
#include <gex_constants.h>

#include <QNetworkInterface>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>


extern void WriteDebugMessageFile(const QString & strMessage);	// Write debug message too trace file

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexFtpCore::CGexFtpCore(const QString & strUserHome, const QString & strApplicationDir, const QString & strLocalFolder, const QString & strApplicationName) 
				: QObject(), m_dataSettings(strLocalFolder)
{	        
	// Initialize data
	m_strUserHome			= strUserHome;
	m_strLocalFolder		= strLocalFolder;
	m_strApplicationDir		= strApplicationDir;
	m_strApplicationName	= strApplicationName;
	m_pSecTimer				= NULL;
	m_pFtpClient			= NULL;

	CGexSystemUtils::NormalizePath(m_strUserHome);
	CGexSystemUtils::NormalizePath(m_strLocalFolder);
	CGexSystemUtils::NormalizePath(m_strApplicationDir);
	CGexSystemUtils::NormalizePath(m_strApplicationName);

	// At startup, spooling is off, and validated only if Init() successful
	m_bSpoolingTimerON = false;
	m_bSpoolingInProgress = false;

	// For Log file cleanup
	m_pLogCleanupTime = new QTime;

	connect(&m_dataSettings, SIGNAL(sSettingsLoaded(bool, const QString&)),						this,	SLOT(onSettingsLoaded(bool, const QString&)));
	connect(&m_dataSettings, SIGNAL(sSettingsSaved(bool, const QString&)),						this,	SLOT(onSettingsSaved(bool, const QString&)));

#if 0
	// Timers
	m_bFirstTimerEvent = false;
	m_pSecTimer = new QTimer(this);
	connect( m_pSecTimer, SIGNAL(timeout()), this, SLOT(OnSecTimerDone()) );
	m_pSecTimer->start(1000);

	// Create FTP CLient
	m_pFtpClient = new CGexFtpClient(applicationDir(), settings(), this);

	// Update Log File
	Info(iStartApp);


	connect(m_pFtpClient, SIGNAL(sCheckFtpServer(const QString&)),								this,	SLOT(OnCheckFtpServer(const QString&)));
	connect(m_pFtpClient, SIGNAL(sFtpServerDateTime(const QString &, const QDateTime &, const QDateTime &)),		this,	SLOT(OnFtpServerDateTime(const QString &, const QDateTime &, const QDateTime &)));
	connect(m_pFtpClient, SIGNAL(sFtpServerFilesToTransfer(const QString &, unsigned int, unsigned int)),		this,	SLOT(OnFtpServerFilesToTransfer(const QString &, unsigned int, unsigned int)));
	connect(m_pFtpClient, SIGNAL(sFtpServerFileTransferred(const QString &, const QString &)),	this,	SLOT(OnFtpServerFileTransferred(const QString &, const QString &)));
	connect(m_pFtpClient, SIGNAL(sFtpServerChecked(const QString &, unsigned int)),				this,	SLOT(OnFtpServerChecked(const QString &, unsigned int)));
	connect(m_pFtpClient, SIGNAL(sFtpServersChecked()),											this,	SLOT(OnFtpServersChecked()));
	connect(m_pFtpClient, SIGNAL(sTransferError(const QString &, const QString &)),				this,	SLOT(OnFtpTransferError(const QString &, const QString &)));

	emit sStatusChanged(StatusReady);
#endif
}


///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexFtpCore::~CGexFtpCore()
{
	// Update Log File
	Info(iStopApp);
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
// Initialize data
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtpCore::init(bool bStartFtpSpoolerAtLaunch, bool bRunAsService, bool *pbExitApplication)
{ 

	*pbExitApplication = false;

	// Save service flags
	m_bRunningAsService	= bRunAsService;
			
	m_dataSettings.setRunAsAService(bRunAsService);

	// Update Log File
	Info(iStartApp);

	// Read Settings file
	int nStatus = m_dataSettings.load();

	
	QMap<int, QString> mapSettingsErrors = m_dataSettings.errors();
	if (!mapSettingsErrors.isEmpty() && m_dataSettings.isNewVersion())
		Log(GEXFTP_MSG_WARNING, "", "New version detected", 0);
	QMapIterator<int, QString> it(mapSettingsErrors);
	while (it.hasNext())
	{
		it.next();
		Log(it.key(), "", it.value(), 0/*QDateTime::currentDateTime().toString()*/);
	}

	switch(nStatus)
	{
		case 2:	// critical error
			*pbExitApplication = true;
			return false;

		case 1:	// non critical error
			return false;
	}

	// Timers
	m_bFirstTimerEvent = false;
	m_pSecTimer = new QTimer(this);
	connect( m_pSecTimer, SIGNAL(timeout()), this, SLOT(OnSecTimerDone()) );
	m_pSecTimer->start(1000);
	
	m_pHeartbeatTimer = new QTimer(this);
	connect(m_pHeartbeatTimer, SIGNAL(timeout()), this, SLOT(onHeartbeatDone()));
	m_pHeartbeatTimer->start(600000); // 10 min
	onHeartbeatDone();
	
	// Create FTP CLient
	m_pFtpClient = new CGexFtpClient(applicationDir(), settings(), this);

	connect(m_pFtpClient, SIGNAL(sCheckFtpServer(const QString&)),								this,	SLOT(OnCheckFtpServer(const QString&)));
	connect(m_pFtpClient, SIGNAL(sFtpServerDateTime(const QString &, const QDateTime &, const QDateTime &)),		this,	SLOT(OnFtpServerDateTime(const QString &, const QDateTime &, const QDateTime &)));
	connect(m_pFtpClient, SIGNAL(sFtpServerFilesToTransfer(const QString &, unsigned int, unsigned int)),		this,	SLOT(OnFtpServerFilesToTransfer(const QString &, unsigned int, unsigned int)));
	connect(m_pFtpClient, SIGNAL(sFtpServerFileTransferred(const QString &, const QString &)),	this,	SLOT(OnFtpServerFileTransferred(const QString &, const QString &)));
	connect(m_pFtpClient, SIGNAL(sFtpServerChecked(const QString &, unsigned int)),				this,	SLOT(OnFtpServerChecked(const QString &, unsigned int)));
	connect(m_pFtpClient, SIGNAL(sFtpServersChecked()),											this,	SLOT(OnFtpServersChecked()));
	connect(m_pFtpClient, SIGNAL(sTransferError(const QString &, const QString &)),				this,	SLOT(OnFtpTransferError(const QString &, const QString &)));

	emit sStatusChanged(StatusReady);

	if((m_bRunningAsService == true) || (bStartFtpSpoolerAtLaunch == true))
	{
		// Start spooling if running as a service
		onStartSpooling();
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Resumes the spooling timer
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::ResumeSpoolingTimer(bool bInitInterval/* = true*/)
{	
	// Enable Start/Stop button
	emit sSpoolingTimer(true);
		
	// Set next spooling time using settings
	if(bInitInterval)
		SetNextFtpSpooling(false);

	// First timer event should be ignored
	m_bFirstTimerEvent = true;
	
	// Resume timer
	m_bSpoolingInProgress = false;
}

/////////////////////////////////////////////////////////////////////////////////////
// Pauses the spooling timer
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::PauseSpoolingTimer()
{
	emit sSpoolingTimer(false);

	m_bSpoolingInProgress = true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Enables the spooling timer: a tick every second
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::EnableSpoolingTimer()
{	
	// Set next spooling time to run immediately
	SetNextFtpSpooling(true);

	// Don't ignore first timer event
	m_bFirstTimerEvent = false;

	// Enable timer
	m_bSpoolingTimerON = true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Disables the spooling timer
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::DisableSpoolingTimer()
{
	m_bSpoolingTimerON = false;
}

/////////////////////////////////////////////////////////////////////////////////////
// Starts timer to check Ftp servers
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::StartFtpCheckTimer()
{
	// Check for files to download on Ftp servers
	emit sStatusChanged(StatusCheckingFtpServers);

	// Stop the timer
	PauseSpoolingTimer();

	// Set timer to give a tick asap
	QTimer::singleShot(0, m_pFtpClient, SLOT(OnCheckFtpServers()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Rename corrupted files
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::RenameCorruptedFile(const QString& strFullFileName)
{
	QDir	dir;
	QString	strNewName = strFullFileName + ".bad";

#if defined(unix) || __MACH__
    chmod(strFullFileName.toLatin1().constData(), 0777);
    chmod(strNewName.toLatin1().constData(), 0777);
#else
    _chmod(strFullFileName.toLatin1().constData(), S_IREAD | S_IWRITE);
    _chmod(strNewName.toLatin1().constData(), S_IREAD | S_IWRITE);
#endif

	dir.rename(strFullFileName, strNewName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Remove specified file
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::RemoveFile(const QString& strFullFileName)
{
	QDir dir;

#if defined(unix) || __MACH__
    chmod(strFullFileName.toLatin1().constData(), 0777);
#else
    _chmod(strFullFileName.toLatin1().constData(), S_IREAD | S_IWRITE);
#endif

    dir.remove(strFullFileName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Calculate next Ftp server spooling time
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::SetNextFtpSpooling(bool bRunNow)
{
	QDateTime dtCurrent = QDateTime::currentDateTime();

	if(bRunNow)
	{
		m_dtNextCheck = dtCurrent;
	}
	else
	{
		if(m_dataSettings.dataGeneral().daily())
		{
			if(dtCurrent.time() < m_dataSettings.dataGeneral().dailyAt())
				m_dtNextCheck = dtCurrent;
			else
				m_dtNextCheck = dtCurrent.addDays(m_dataSettings.dataGeneral().dailyEveryNDays());

			m_dtNextCheck.setTime(m_dataSettings.dataGeneral().dailyAt());
		}
		else if(m_dataSettings.dataGeneral().weekly())
		{
			if((dtCurrent.date().dayOfWeek() == (m_dataSettings.dataGeneral().weeklyOn()+1)) && (dtCurrent.time() < m_dataSettings.dataGeneral().weeklyAt()))
				m_dtNextCheck = dtCurrent;
			else
				m_dtNextCheck = dtCurrent.addDays(1);

			while(m_dtNextCheck.date().dayOfWeek() != (m_dataSettings.dataGeneral().weeklyOn()+1))
				m_dtNextCheck = m_dtNextCheck.addDays(1);
		
			m_dtNextCheck.setTime(m_dataSettings.dataGeneral().weeklyAt());
		}
		else
			m_dtNextCheck = QDateTime::currentDateTime().addSecs(m_dataSettings.dataGeneral().everyHour()*3600 + m_dataSettings.dataGeneral().everyMinute()*60 + m_dataSettings.dataGeneral().everySecond());
	}
	
	m_nNextSpooling_Sec = GetSecondsToNextFtpSpooling();
}

/////////////////////////////////////////////////////////////////////////////////////
// Get seconds to next Ftp server spooling, or -1 if greater than threashhold
/////////////////////////////////////////////////////////////////////////////////////
int CGexFtpCore::GetSecondsToNextFtpSpooling()
{
	int nSecondsToNextFtpSpooling = QDateTime::currentDateTime().secsTo(m_dtNextCheck);

	if(nSecondsToNextFtpSpooling < 3600)
		return nSecondsToNextFtpSpooling;
	else
		return -1;
}

/////////////////////////////////////////////////////////////////////////////////////
// Log info
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::Info(int nInfoCode, const QString& strFtpServer /*= ""*/, const QString& strArg /*= ""*/)
{
	QString strInfoMsg;

	switch(nInfoCode)
	{
		case iStartApp:
			strInfoMsg = "Starting "+ QString(m_bRunningAsService ? " service " : " application ") + m_strApplicationName;
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg, false);
			break;
		case iStopApp:
			strInfoMsg = "Stopping " + m_strApplicationName;
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg, false);
			break;
		case iStartSpool:
			strInfoMsg = "Starting spooler";
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg, false);
			break;
		case iStopSpool:
			strInfoMsg = "Stopping spooler";
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg, false);
			break;
		case iLoadSettings:
			strInfoMsg = "Settings successfully loaded from: " + strArg;
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg);
			break;
		case iSaveSettings:
			strInfoMsg = "Settings successfully saved to: " + strArg;
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg);
			break;
		case iRemoveLogFile:
			strInfoMsg = "Log file deleted: " + strArg;
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg);
			break;
		case iCleanLogFiles:
			strInfoMsg = "Cleaning log files older than " + strArg;
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg);
			break;
		case iTransferDone:
			strInfoMsg = "Files successfully transferred: " + strArg;
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg, false);
			break;
		case iServerDateTime:
			strInfoMsg = "Server date/time: " + strArg;
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg, false);
			break;
		case iFilesToTransfer:
			strInfoMsg = "Files to transfer: " + strArg;
			Log(GEXFTP_MSG_INFO, strFtpServer, strInfoMsg, false);
			break;
		case iFileTransferred:
			strInfoMsg = "File transferred: " + strArg;
			Log(GEXFTP_MSG_INFO_XFER, strFtpServer, strInfoMsg);
			break;
		default:
			return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Log error
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::Error(int nErrorCode, const QString& strFtpServer /*= ""*/, const QString& strArg /*= ""*/)
{
	QString strErrorMsg;

	switch(nErrorCode)
	{
		case eSaveSetttings:
			strErrorMsg = "Error saving settings to file " + strArg + ".";
			break;
		case eLoadSetttings:
			strErrorMsg = "Error reading settings from file " + strArg + ".";
			break;
		case eTransferError:
			strErrorMsg = "Transfer error: " + strArg + ".";
			break;
		default:
			return;
	}
	Log(GEXFTP_MSG_ERROR, strFtpServer, strErrorMsg);
}

/////////////////////////////////////////////////////////////////////////////////////
// Add to log file and log page
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::Log(int nMsgType, const QString& strFtpServer, const QString& strMsg, bool bFilter /*=true*/)
{
	// Check if message should be logged
	if((bFilter == false) || (m_dataSettings.logFilter() & nMsgType))
	{
		QString	strDateTime, strDate, strMsgType;

		// Date
		strDateTime = QDateTime::currentDateTime().toString();
		strDate = QDate::currentDate().toString(Qt::ISODate);

		LogInfo(nMsgType, strFtpServer, strMsg.simplified(), strDateTime);

		// Set error type
		switch(nMsgType)
		{
			case GEXFTP_MSG_INFO:
				strMsgType = "Info";
				break;
			case GEXFTP_MSG_INFO_XFER:
				strMsgType = "InfoTransfer";
				break;
			case GEXFTP_MSG_WARNING:
				strMsgType = "Warning";
				break;
			case GEXFTP_MSG_ERROR:
				strMsgType = "Error";
				break;
			default:
				break;
		}

		// Add to log file
		QString strLogFileName;
        strLogFileName = m_strLocalFolder + "/GalaxySemi/logs/" + QDate::currentDate().toString(Qt::ISODate);
        QDir d(strLogFileName);
        if (!d.exists(strLogFileName))
            if (!d.mkpath(strLogFileName))
                return;
        strLogFileName += QDir::separator() + QString("gex_ftp_") + strDate;
        strLogFileName += "_" + QString::number(QCoreApplication::applicationPid());
        strLogFileName += ".log";

		CGexSystemUtils::NormalizePath(strLogFileName);

		QFile	fileLog(strLogFileName);
		bool	bWriteHeader=false;

		if(fileLog.exists() == false)
			bWriteHeader = true;

		if(fileLog.open(QIODevice::WriteOnly | QIODevice::Append) == true)
		{
			QTextStream streamLog(&fileLog);
			if(bWriteHeader == true)
			{
				streamLog << "Date";
				streamLog << ",";
				streamLog << "Type";
				streamLog << ",";
				streamLog << "Ftp server";
				streamLog << ",";
				streamLog << "Message";
				streamLog << endl;
			}
			streamLog << strDateTime;
			streamLog << ",";
			streamLog << strMsgType;
			streamLog << ",";
			streamLog << strFtpServer;
			streamLog << ",";
			streamLog << strMsg.simplified();
			streamLog << endl;
			fileLog.close();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Add to log GUI
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::LogInfo(int nMsgType, const QString& strMailFile, const QString& strMsg, const QString& strDateTime)
{
	emit sLogInfo(nMsgType, strMailFile, strMsg, strDateTime);
}

/////////////////////////////////////////////////////////////////////////////////////
// Called every second
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::OnSecTimerDone()
{
	if((m_bSpoolingTimerON == true) && (m_bSpoolingInProgress == false))
	{
		// Check if we need to cleanup log files
		if(m_pLogCleanupTime->elapsed() >= m_dataSettings.logCleanupInterval())
			cleanLogFiles();

		// Drop first timer event
		if(m_bFirstTimerEvent == true)
		{
			m_bFirstTimerEvent = false;
			return;
		}

		// Should we check ftp servers for files to download?
		if(m_nNextSpooling_Sec == -1)
		{
			emit sStatusChanged(StatusCountDown);

			m_nNextSpooling_Sec = GetSecondsToNextFtpSpooling();
			return;
		}
		else if(m_nNextSpooling_Sec > 0)
		{
			emit sStatusChanged(StatusCountDown);

			m_nNextSpooling_Sec--;
			return;
		}
		
		// Trace message 
		WriteDebugMessageFile("############ Start checking FTP servers ############");
		// Start timer to check Ftp servers asap
		StartFtpCheckTimer();
	}
	if((m_bSpoolingTimerON == false) && (m_bSpoolingInProgress == false))
	{
		// Update status bar
		emit sStatusChanged(StatusReady);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Clean log files
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::cleanLogFiles()
{
	QStringList		strLogFiles;
	QDir			dir;
	QString			strFileName;
	QString			strFullFileName;
	QDateTime		limitDate;
	QString			strLimitDate;
	QStringList		strListFilters("*.log");
	bool			bLogUpdated = false;

	// Restart time for next log cleanup
	m_pLogCleanupTime->start();

	// Set limit date
	limitDate = QDateTime::currentDateTime().addDays(0 - m_dataSettings.dataGeneral().nbDaysToKeepLog());
	strLimitDate = limitDate.toString();

	// Get list of log files
	dir.setPath(m_strLocalFolder);
	dir.setFilter(QDir::Files | QDir::Hidden);
	strLogFiles = dir.entryList(strListFilters);

	// Check each retrieved file
	for(QStringList::iterator it = strLogFiles.begin(); it != strLogFiles.end(); ++it)
	{
		strFileName = *it;
		if(strFileName.startsWith("gex_ftp_"))
		{
			strFullFileName = m_strLocalFolder + "/" + strFileName;
			CGexSystemUtils::NormalizePath(strFullFileName);

			QFileInfo fileInfo(strFullFileName);
			if(fileInfo.lastModified() < limitDate)
			{
				if(!bLogUpdated)
				{
					// Update log
					Info(iCleanLogFiles, "", strLimitDate);
					bLogUpdated = true;
				}

				Info(iRemoveLogFile, "", strFileName);
				RemoveFile(strFullFileName);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
// The Ftp client is checking specified ftp server for files to download
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::OnCheckFtpServer(const QString& strServerName)
{
	emit sStatusChanged(StatusCheckingFtpServer, strServerName);

	emit sStartDownload(strServerName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying the Ftp server date/time
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::OnFtpServerDateTime(const QString & strProfileName, const QDateTime & clFtpServerDateTime, const QDateTime & clServerTimeLimit)
{
	// Update Log
	QString strArg = clFtpServerDateTime.toString("dd MMM yyyy hh:mm:ss") + " (files more recent than " + clServerTimeLimit.toString("dd MMM yyyy hh:mm:ss") + " will be ignored)";
	Info(iServerDateTime, strProfileName, strArg);
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying the nb of files to be transferred on one Ftp server
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::OnFtpServerFilesToTransfer(const QString & strProfileName, unsigned int uiNbFilesToTransfer, unsigned int uiNbFilesIgnored)
{
	// Update Log
	QString strArg = QString::number(uiNbFilesToTransfer) + " (" + QString::number(uiNbFilesIgnored) + " recent files ignored";
	if (m_dataSettings.dataGeneral().enableMaxFilePerProfile())
		strArg += ", Max. nb. of files to download per profile: " + QString::number(m_dataSettings.dataGeneral().maxFilePerProfile());
	strArg += ")";
	Info(iFilesToTransfer, strProfileName, strArg);
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying that a file has been transferred on one Ftp server
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::OnFtpServerFileTransferred(const QString & strProfileName, const QString & strFileName)
{
	// Update Log
	Info(iFileTransferred, strProfileName, strFileName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying that one Ftp server has been checked
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::OnFtpServerChecked(const QString & strProfileName, unsigned int uiNbFilesTransferred)
{
	// Update Log
	Info(iTransferDone, strProfileName, QString::number(uiNbFilesTransferred));
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying that an error occured during Ftp transfer
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::OnFtpTransferError(const QString & strProfileName, const QString & strError)
{
	// Update Log
	Error(eTransferError, strProfileName, strError);
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal specifying that all Ftp servers have been checked
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::OnFtpServersChecked()
{
	emit sStopDownload();
	
	// Resume spooling timer
	ResumeSpoolingTimer();
}

/////////////////////////////////////////////////////////////////////////////////////
// Start spooling
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::onStartSpooling()
{
	if(m_bSpoolingTimerON == false)
	{
		// Update log
		Info(iStartSpool);

		// Clean log files
		cleanLogFiles();

		// We need to start the timer
		EnableSpoolingTimer();

		// emit a signal that the spooling is starting
		emit sStartSpooling();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Stop spooling
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::onStopSpooling()
{
	if(m_bSpoolingTimerON == true)
	{
		// Update log
		Info(iStopSpool);

		// Resume and stop spooling timer
		ResumeSpoolingTimer();
		DisableSpoolingTimer();

		emit sStopSpooling();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Pause execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::pause(void)
{ 
}

/////////////////////////////////////////////////////////////////////////////////////
// Resume execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::resume(void)
{ 
	if(m_bSpoolingTimerON == false)
		onStartSpooling();
}

/////////////////////////////////////////////////////////////////////////////////////
// Start execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::start(void)
{
	if(m_bSpoolingTimerON == false)
		onStartSpooling();
}

/////////////////////////////////////////////////////////////////////////////////////
// Stop execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::stop(void)
{
	if(m_bSpoolingTimerON == true)
		onStopSpooling();
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal that settings have been loaded
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::onSettingsLoaded(bool bResult, const QString& strSettingsFile)
{
	if (bResult)
		Info(iLoadSettings, "", strSettingsFile);
	 else
		Error(eLoadSetttings, "", strSettingsFile);	
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal that settings have been saved
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::onSettingsSaved(bool bResult, const QString& strSettingsFile)
{
	if (bResult)
		Info(iSaveSettings, "", strSettingsFile);
	else
		Error(eSaveSetttings, "", strSettingsFile);
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal that settings have been saved
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpCore::onHeartbeatDone()
{
	WriteDebugMessageFile("-----------------------------------------------------------------");
	WriteDebugMessageFile("--------------------------- Heartbeat ---------------------------");
	WriteDebugMessageFile("-----------------------------------------------------------------");
	
	WriteDebugMessageFile("Detected interfaces:");
	QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
	for (int i = 0; i < interfaces.size() ; ++i)
	{
		QString strMsg = interfaces[i].humanReadableName();
		
		QList<QNetworkAddressEntry> hosts = interfaces[i].addressEntries();
		strMsg += " [ips: ";
		for (int j = 0; j < hosts.size(); ++j)
			strMsg += hosts[j].ip().toString() + ",";
		strMsg += "] [flags: ";
		if (interfaces[i].flags() & QNetworkInterface::IsUp)
			strMsg += "IsUp, ";
		if (interfaces[i].flags() & QNetworkInterface::IsRunning)
			strMsg += "IsRunning, ";
		if (interfaces[i].flags() & QNetworkInterface::CanBroadcast)
			strMsg += "CanBroadcast, ";
		if (interfaces[i].flags() & QNetworkInterface::IsLoopBack)
			strMsg += "IsLoopBack, ";
		if (interfaces[i].flags() & QNetworkInterface::IsPointToPoint)
			strMsg += "IsPointToPoint, ";
		if (interfaces[i].flags() & QNetworkInterface::CanMulticast)
			strMsg += "CanMulticast";
		strMsg += "]";
		WriteDebugMessageFile(strMsg);
	}

	WriteDebugMessageFile("-----------------------------------------------------------------");
}


