#include "gexftp_download.h"
#include "gexftp_settings.h"
#include "gexftp_constants.h"

#include <gqtl_sysutils.h>


#include <qhostaddress.h>
#include <qdir.h>
#include <qtimer.h>

#ifdef _WIN32
#include <winsock.h>
#endif

#if defined(unix) || __MACH__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif


extern void WriteDebugMessageFile(const QString & strMessage);	// Write debug message too trace file

#define GEXFTP_READBACK_TIMESTAMP_FILE			1

/////////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////////
#define GEXFTP_SEQUENCE_CONNECT					1
#define GEXFTP_SEQUENCE_LOGIN					2
#define GEXFTP_SEQUENCE_CHDIR					3
#define GEXFTP_SEQUENCE_LIST1					4
#define GEXFTP_SEQUENCE_REMOVETIMESTAMPFILE		5
#define GEXFTP_SEQUENCE_PUTTIMESTAMPFILE		6
#define GEXFTP_SEQUENCE_LIST2					7
#define GEXFTP_SEQUENCE_VERIFY					8
#define GEXFTP_SEQUENCE_GET						9
#define GEXFTP_SEQUENCE_CLEANUP					10
#define GEXFTP_SEQUENCE_REMOTEREMOVE			11
#define GEXFTP_SEQUENCE_REMOTERENAME_1			12
#define GEXFTP_SEQUENCE_REMOTERENAME_2			13
#define GEXFTP_SEQUENCE_REMOTELEAVE				14
#define GEXFTP_SEQUENCE_SYSTEM					15
#define GEXFTP_SEQUENCE_GETTIMESTAMPFILE		16
#define GEXFTP_SEQUENCE_CHECKFILE				17
#define GEXFTP_SEQUENCE_PWD						18

#define GEXFTP_COMMAND_TIMEOUT					30

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexFtpDownload::CGexFtpDownload(CGexFtpSettings * pDataSettings, QObject * pParent) : QObject(pParent), m_pDataSettings(pDataSettings)
{
	// Create timer
	m_pTimer = new QTimer(this);
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(OnTimer()));
	m_pTimer->start(500);
	// Initialize the sequence timer
	m_pSequenceTimer = new QTimer(this);
	connect(m_pSequenceTimer, SIGNAL(timeout()), this, SLOT(onSequenceTimeout()));
	m_pSequenceTimer->setSingleShot(true);
	m_pSequenceTimer->setInterval(30*60*1000); // 30 min
	// Initialize FTP object
	m_pclFtp = NULL;
	m_pTimestampFile = NULL;
	m_strCDPaths.clear();
	m_bIsConnectionOK = true;
	// Initialize members variables
	reset();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexFtpDownload::~CGexFtpDownload()
{
	if (m_pTimestampFile)
	{
		delete m_pTimestampFile;
		m_pTimestampFile = NULL;
	}

	if (m_pclFtp)
	{
        m_pclFtp->deleteLater();
		m_pclFtp = NULL;
	}
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
// Start the ftp sequence
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::start(const CGexFtpServer& ftpServer)
{
	// Start the sequence timer
	m_pSequenceTimer->start();
	QString	strTransferredListFileName_old, strTransferredListFileName_old_old;

	// Save ref to Ftp Server object
	m_clFtpServer = ftpServer;

	// Init last error code
	m_bIsConnectionOK = true;

	// Init some file names
	m_strTimestampFileLocalName = m_clFtpServer.localDir();
	m_strTimestampFileLocalName += "/gex_ftp_timestampfile.temp.txt.gex";
	CGexSystemUtils::NormalizePath(m_strTimestampFileLocalName);
	m_strTimestampFileRemoteName = "gex_ftp_timestampfile.temp.txt.gex";

	// Init Search mode : recursive or not
	m_bRecurseList = m_clFtpServer.recursiveSearch();

	// Name of the file containing the list of files already transferred
	// This file now contains the name of the profile
	// To be compatible with previous versions, rename older file not containing the profile name.
	strTransferredListFileName_old_old = m_clFtpServer.localDir();
	strTransferredListFileName_old_old += "/.gex_ftp_transferred.txt.gex";
	CGexSystemUtils::NormalizePath(strTransferredListFileName_old_old);
	QString strSettingsName = m_clFtpServer.settingsName();
	strSettingsName.replace('/', ' ');
	strSettingsName.replace('\\', ' ');
	strSettingsName.replace(',', ' ');
	strSettingsName.replace(';', ' ');
	strSettingsName.replace('.', ' ');
	strSettingsName.replace('&', ' ');
	strSettingsName.replace('*', ' ');
	strSettingsName.replace(':', ' ');
	strSettingsName.replace('!', ' ');
	strSettingsName.replace('?', ' ');
	strSettingsName.replace('$', ' ');
	strSettingsName = strSettingsName.simplified();
	strSettingsName.replace(' ', '_');
	strTransferredListFileName_old = m_clFtpServer.localDir();
	strTransferredListFileName_old += "/.gex_ftp_transferred_";
	strTransferredListFileName_old += strSettingsName;
	strTransferredListFileName_old += ".txt";

	CGexSystemUtils::NormalizePath(strTransferredListFileName_old);
	m_strTransferredListFileName = strTransferredListFileName_old + ".gex";

	QDir clDir;
	if(clDir.exists(strTransferredListFileName_old_old) && !clDir.exists(m_strTransferredListFileName))
		clDir.rename(strTransferredListFileName_old_old, m_strTransferredListFileName);
	if(clDir.exists(strTransferredListFileName_old) && !clDir.exists(m_strTransferredListFileName))
		clDir.rename(strTransferredListFileName_old, m_strTransferredListFileName);

	// Remove temporary files if any (.__tmp__, gex_ftp_timestampfile.temp.txt)
	QStringList				strlTmpFiles;
	QDir					dir;
	QString					strFullFileName;
	QStringList				strFilters("*.__tmp__");
	QStringList::iterator	it;

	dir.setPath(m_clFtpServer.localDir());
	dir.setFilter(QDir::Files | QDir::Hidden);
	strlTmpFiles = dir.entryList(strFilters);
	for(it = strlTmpFiles.begin(); it != strlTmpFiles.end(); it++)
	{
		strFullFileName = m_clFtpServer.localDir() + "/" + *it;
		CGexSystemUtils::NormalizePath(strFullFileName);
                dir.remove(strFullFileName);
	}
        dir.remove(m_strTimestampFileLocalName);

	// Create FTP object and connect slots
	m_pclFtp = new GexFtpQFtp();
	connect(m_pclFtp, SIGNAL(commandStarted(int)),						this,	SLOT(ftp_commandStarted(int)));
	connect(m_pclFtp, SIGNAL(commandFinished(int,bool)),				this,	SLOT(ftp_commandFinished(int,bool)));
	connect(m_pclFtp, SIGNAL(done(bool)),								this,	SLOT(ftp_done(bool)));
	connect(m_pclFtp, SIGNAL(stateChanged(int)),						this,	SLOT(ftp_stateChanged(int)));
	connect(m_pclFtp, SIGNAL(listInfo(const QUrlInfo &)),				this,	SLOT(ftp_listInfo(const QUrlInfo &)));
	connect(m_pclFtp, SIGNAL(rawCommandReply(int, const QString &)),	this,	SLOT(ftp_rawCommandReply(int, const QString &)));
	connect(m_pclFtp, SIGNAL(dataTransferProgress(qint64, qint64)),		this,	SLOT(OnTransferProgress(qint64, qint64)) );

	// Trace message
	QString strMessage = "########## Starting profile " + m_clFtpServer.settingsName() + " ##########";
	WriteDebugMessageFile(strMessage);

	// Create a timer to launch the first FTP sequence asap
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_connect()));

	QTimer::singleShot(0, this, SLOT(ftp_sequencer()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Stop the ftp sequence
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::stop()
{
	WriteDebugMessageFile("Stop sequence");
	// no need to delete child widgets, Qt does it all for us
	// Disconnect all signals of the Ftp object!!!!!!!
	disconnect(m_pclFtp, 0, 0, 0);
	m_pSequenceTimer->stop();
//	m_pclFtp->close();
	ftp_startCommand(GexFtpQFtp::Close);

	reset();
}

/****************************************************************************
**
** Slots connected to signals of the GexFtpQFtp class
**
*****************************************************************************/

void CGexFtpDownload::ftp_commandStarted(int id)
{
    WriteDebugMessageFile(QString("Command started: id=%1, sequence id=%2").arg(id).arg(m_nCurrentSequence));
    // restart the sequence timer
	m_pSequenceTimer->start();
	LogInfo(iCommandStarted);
}

void CGexFtpDownload::ftp_commandFinished(int id, bool error)
{
	// Reset 'command in progress' flag
	m_bCommandInProgress = false;
	emit sChangeTimeout(-1);

	// Close current download file handle if opened
	CloseLocalFile(true);

	// Close current timestamp file handle if opened
	if(m_pTimestampFile)
	{
		m_pTimestampFile->close();
		delete m_pTimestampFile;
		m_pTimestampFile = NULL;
	}

	// Check if we have files with invalid date/time have been retrieved
	if(	((m_nCurrentSequence == GEXFTP_SEQUENCE_LIST1) || (m_nCurrentSequence == GEXFTP_SEQUENCE_LIST2))
		&& m_uiNbFilesCorruptedDate > 0)
	{
		LogError(eInvalidDateTime_RemoteFiles);
	}

	// Check for errors
	if(error)
	{
        WriteDebugMessageFile(QString("Command finished: id=%1, sequence id=%2, error=%3").arg(id).arg(m_nCurrentSequence).arg(m_pclFtp->errorString()));
        LogInfo(iCommandFinishedNOK);
	}
	else
    {
        WriteDebugMessageFile(QString("Command finished: id=%1, sequence id=%2, no error").arg(id).arg(m_nCurrentSequence));
        LogInfo(iCommandFinishedOK);
    }
}

void CGexFtpDownload::ftp_done(bool error)
{
	QDir				clDir;

	if (error)
        WriteDebugMessageFile(QString("Command done: sequence id=%1, error=%2").arg(m_nCurrentSequence).arg(m_pclFtp->errorString()));
    else
        WriteDebugMessageFile(QString("Command done: sequence id=%1, no error").arg(m_nCurrentSequence));

	// If error during last sequence: log error message, unless RENAME_1 state (normal error if dest file doesn't exist)
	if(error && (m_nCurrentSequence != GEXFTP_SEQUENCE_REMOTERENAME_1))
	{
		LogError(eFtpError);
	}

	emit sDownloadProgress(0, 0);

	// Check if we are in Cleanup state
	if(m_nCurrentSequence == GEXFTP_SEQUENCE_CLEANUP)
	{
		// Disconnect all signals of the Ftp object!!!!!!!
		disconnect(m_pclFtp, 0, 0, 0);
		// Emit 'Transfer done' signal on next timer tick
		m_bEmitTransferDone = true;
		return;
	}

	// Check if we were in the PUTTIMESTAMPFILE sequence
	if((m_nCurrentSequence == GEXFTP_SEQUENCE_PUTTIMESTAMPFILE) && !error)
	{
		// Set flag specifying the remote timestamp file has been created
		m_bRemoteTimestampFileCreated = true;
	}

	// If an error occured during a GET sequence, setup transfer status, and next sequence
	if(error && (m_nCurrentSequence == GEXFTP_SEQUENCE_GET))
	{
		WriteDebugMessageFile("Error during GET sequence");
		emit sTransferStatus(GEXFTP_FILESTATUS_ERROR, m_strFileInTransferRemoteName.section('/', -1, -1));

		// Remove local file
        clDir.remove(m_strFileInTransferLocalName);

		// Disconnect from current Ftp sequence
		disconnect(this, SIGNAL(sNextSequence()), 0, 0);
		// Connect next Ftp sequence
		connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_get()));
	}

	// If we were in a REMOVE/RENAME_2 sequence, setup transfer status
	if((m_nCurrentSequence == GEXFTP_SEQUENCE_REMOTEREMOVE) || (m_nCurrentSequence == GEXFTP_SEQUENCE_REMOTERENAME_2))
	{
		if(error)
		{
			WriteDebugMessageFile("Error during REMOVE/RENAME_2 sequence");
			emit sTransferStatus(GEXFTP_FILESTATUS_ERROR, m_strFileInTransferRemoteName.section('/', -1, -1));

			// Remove local file
            clDir.remove(m_strFileInTransferLocalName);
		}
		else
		{
			// Rename local file
            clDir.rename(m_strFileInTransferLocalName, m_strFinalLocalName);

			emit sTransferStatus(GEXFTP_FILESTATUS_TRANSFERRED, m_strFileInTransferRemoteName.section('/', -1, -1));

			// Emit signal for file transferred
			emit sFileTransferred(m_clFtpServer.settingsName(), m_strFileInTransferRemoteName.section('/', -1, -1));

			// Update file count
			m_uiNbFilesTransferred++;
		}
	}

	// If not logged in anymore, exit
	if(m_pclFtp->state() != GexFtpQFtp::LoggedIn)
	{
		WriteDebugMessageFile("User not logged anymore");
		Cleanup();
		return;
	}

	// If an error occured, exit, unless it happened during a GET/REMOTEREMOVE/REMOTERENAME sequence, in which
	// case we can pursue with next file
	if(error && (m_nCurrentSequence != GEXFTP_SEQUENCE_GET) && (m_nCurrentSequence != GEXFTP_SEQUENCE_REMOTEREMOVE) &&
				(m_nCurrentSequence != GEXFTP_SEQUENCE_REMOTERENAME_1) && (m_nCurrentSequence != GEXFTP_SEQUENCE_REMOTERENAME_2))
	{
		Cleanup();
		return;
	}

	// No errors, no Abort. If logged in, create a timer to launch the next FTP sequence asap
	if(m_pclFtp->state() == GexFtpQFtp::LoggedIn)
		QTimer::singleShot(0, this, SLOT(ftp_sequencer()));
}

void CGexFtpDownload::ftp_stateChanged(int state)
{
	switch ((GexFtpQFtp::State)state)
	{
		case GexFtpQFtp::Unconnected:
			LogInfo(iStateUnconnected);
			WriteDebugMessageFile("Ftp state changed to unconnected (sequence id=" + QString::number(m_nCurrentSequence) + ")");
			// Check current sequence
			if(m_nCurrentSequence == GEXFTP_SEQUENCE_GET)
			{
				LogError(eFtpConnectionBroken);

				emit sDownloadProgress(0, 0);

				emit sTransferStatus(GEXFTP_FILESTATUS_ERROR, m_strFileInTransferRemoteName.section('/', -1, -1));
			}

			if(m_nCurrentSequence == GEXFTP_SEQUENCE_CONNECT)
			{
				// Could not connect to server
				LogError(eConnect);
			}

			// Check if we are in Cleanup state. If not, do it!
			if(m_nCurrentSequence == GEXFTP_SEQUENCE_CLEANUP)
			{
				// Disconnect all signals of the Ftp object!!!!!!!
				disconnect(m_pclFtp, 0, 0, 0);
				// Emit 'Transfer done' signal on next timer tick
				m_bEmitTransferDone = true;
			}
			else
				Cleanup();
			break;
		case GexFtpQFtp::HostLookup:
			// Trace message
			WriteDebugMessageFile(m_clFtpServer.ftpSiteURL() + " look up...");
			LogInfo(iStateHostLookup);
			break;
		case GexFtpQFtp::Connecting:
			// Trace message
			WriteDebugMessageFile("Connecting to " + m_clFtpServer.ftpSiteURL() + "...");
			LogInfo(iStateConnecting);
			break;
		case GexFtpQFtp::Connected:
			// Trace message
			WriteDebugMessageFile("Connected to " + m_clFtpServer.ftpSiteURL());
			LogInfo(iStateConnected);
			break;
		case GexFtpQFtp::LoggedIn:
			// Trace message
			WriteDebugMessageFile("Logged in (User=" + m_clFtpServer.login() + ")");
			LogInfo(iStateLoggedIn);
			break;
		case GexFtpQFtp::Closing:
			// Trace message
			WriteDebugMessageFile("Closing connection...");
			LogInfo(iStateClosing);
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Slot called when a list entry (FTP list command) is retrieved
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_rawCommandReply(int nReplyCode, const QString & strDetail)
{
	switch(nReplyCode)
	{
		case 215:
			// 215 <System name>
			// Reply from the SYST command
			m_strSystem = strDetail;
			m_bDisplaySystem = true;
			if(strDetail == "Windows_NT")
			{
				m_strRawCommand = "SITE DIRSTYLE\r\n";
//				m_pclFtp->rawCommand(m_strRawCommand);
				QList<QVariant> commandArgs;
				commandArgs.append(QVariant(m_strRawCommand));
				ftp_startCommand(GexFtpQFtp::RawCommand, commandArgs);
			}
			break;
		case 257:
			// 257 <pwd>
			m_strCurrentDirectory = strDetail.section('"', 1, 1) + "/";
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Slot called when a list entry (FTP list command) is retrieved
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_listInfo(const QUrlInfo &urlInfo)
{
	QUrlInfo info = urlInfo;
	// If files retrieved, reset command start time, to avoid automatic timeout
	m_clCommandStartDateTime = QDateTime::currentDateTime();
	// Is this entry a directory?
	if(info.isDir())
	{
		if ((m_bRecurseList) && (info.name() != ".") && (info.name() != ".."))
		{
			m_strCDPaths << m_strCurrentDirectory + info.name();
//			m_pclFtp->cd(m_strCDPaths.last());
			QList<QVariant> commandArgs;
			commandArgs.append(QVariant(m_strCDPaths.last()));
			ftp_startCommand(GexFtpQFtp::Cd, commandArgs);
			pwdAndList();
		}
		return;
	}

	QString		strAbsoluteFileName = m_strCurrentDirectory + info.name();
        QString		strRemoteDirAndSlash = m_clFtpServer.remoteDir() + "/";
        QString		strFileName = strAbsoluteFileName.remove(strRemoteDirAndSlash,
                                                                 m_pDataSettings->dataGeneral().caseInsensitive() ? Qt::CaseInsensitive : Qt::CaseSensitive);
	QDateTime	clLastModified = info.lastModified();
	QString		strLastModified = clLastModified.toString("dd MMM yyyy hh:mm:ss");
	QString		strOwner = info.owner();
	bool		bValidDateTime;


	// Check date/time validity (done in 2 steps for easier debug)
	bValidDateTime = clLastModified.isValid();
	if(bValidDateTime)
		bValidDateTime = !clLastModified.isNull();

	// Check if the file is the Gex-Ftp timestamp file
	if ((info.name() == m_strTimestampFileRemoteName) && (m_strCurrentDirectory == strRemoteDirAndSlash))
	{
		if(m_nCurrentSequence == GEXFTP_SEQUENCE_LIST1)
			m_bRemoteTimestampFileExists = true;
		else if(m_nCurrentSequence == GEXFTP_SEQUENCE_LIST2)
		{
			m_clServerCurrentDateTime = clLastModified;
			// Correct server date/time (Due to a QT bug, the lastModified() is 1 year wrong on some FTP servers)
			// for the timestamp file just uploaded
			CorrectServerTime(m_clServerCurrentDateTime);
		}
		return;
	}

	m_uiNbTotalFiles++;

	// Display nb files
	emit sListFileInfo(m_uiNbFilesToTransfer, m_uiNbTotalFiles);

	// Check if file's date/time is correct
	if (!m_clFtpServer.disabledDateTimeCheck() && !bValidDateTime)
	{
		m_uiNbFilesCorruptedDate++;
		m_strFileInTransferRemoteName = strAbsoluteFileName;
		LogError(eInvalidDateTime_RemoteFile, true);
		return;
	}

	// Check if file is in the correct DateTime window
	if((!m_clFtpServer.disabledDateTimeCheck() && m_clFtpServer.useDateTimeWindow()) && ((clLastModified < m_clFtpServer.dateFrom()) || (clLastModified > m_clFtpServer.dateTo())))
		return;

	// If file has the correct extension, add it to list of files retrieved
	QRegExp					clRegExp("[ \t]");
	QStringList				listExtensions = m_clFtpServer.fileExtensions().remove(clRegExp).toLower().split(";", QString::SkipEmptyParts);
	QStringList::iterator	itExtension;

	clRegExp.setPatternSyntax(QRegExp::Wildcard);
	clRegExp.setCaseSensitivity(Qt::CaseInsensitive);
	for(itExtension = listExtensions.begin(); itExtension!=listExtensions.end(); itExtension++)
	{
		clRegExp.setPattern(*itExtension);
		if(clRegExp.exactMatch(info.name()))
		{
			info.setName(strFileName);
			if(m_nCurrentSequence == GEXFTP_SEQUENCE_LIST1)
			{
				// Insert at correct position, ordered from oldest to most recent file
				m_itFilesToGet = m_listFiles_1.begin();
                if(m_clFtpServer.disabledDateTimeCheck() == true)
                    // Date/Time check disabled, current entry will be appended
                    m_itFilesToGet = m_listFiles_1.end();
                else if(m_clFtpServer.downloadOrder() == GEXFTP_DOWNLOAD_NEWEST_TO_OLDEST)
                {
                    // Get position for 'Newest to Oldest' order
                    while(  (m_itFilesToGet != m_listFiles_1.end()) &&
                            (info.lastModified() < (*m_itFilesToGet).lastModified()))
                        m_itFilesToGet++;
                }
                else
                {
                    // Get position for 'Oldest to Newest' order
                    while(  (m_itFilesToGet != m_listFiles_1.end()) &&
                            (info.lastModified() > (*m_itFilesToGet).lastModified()))
                        m_itFilesToGet++;
                }

                // Insert at determined position
				if(m_itFilesToGet == m_listFiles_1.end())
					m_listFiles_1.append(info);
				else
					m_listFiles_1.insert(m_itFilesToGet, info);
			}
			else if(m_nCurrentSequence == GEXFTP_SEQUENCE_LIST2)
				m_listFiles_2.append(info);

			// increase nb files to transfer
			m_uiNbFilesToTransfer++;

			emit sListFileInfo(m_uiNbFilesToTransfer, m_uiNbTotalFiles);

			break;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: CONNECT
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_connect()
{
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_pwd()));

	// Connect to Ftp server
	LogInfo(iSequenceConnect);
	m_nCurrentSequence = GEXFTP_SEQUENCE_CONNECT;
	ResolveHost();
//	m_pclFtp->connectToHost(m_strFtpServer, m_clFtpServer.ftpPortNb());
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant(m_strFtpServer));commandArgs.append(QVariant(m_clFtpServer.ftpPortNb()));
	ftp_startCommand(GexFtpQFtp::ConnectToHost, commandArgs);
//	m_pclFtp->login(m_clFtpServer.login(), m_clFtpServer.password());
	commandArgs.clear();
	commandArgs.append(QVariant(m_clFtpServer.login()));commandArgs.append(QVariant(m_clFtpServer.password()));
	ftp_startCommand(GexFtpQFtp::Login, commandArgs);
}


/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: PWD
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_pwd()
{
	WriteDebugMessageFile("Get login directory...");
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_system()));

	// Connect to Ftp server
	LogInfo(iSequenceConnect);
	m_nCurrentSequence = GEXFTP_SEQUENCE_PWD;
//	m_pclFtp->rawCommand("pwd");
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant("pwd"));
	ftp_startCommand(GexFtpQFtp::RawCommand, commandArgs);

}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: SYSTEM
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_system()
{
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	if(m_clFtpServer.remoteDir().isEmpty())
		connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_list1()));
	else
		connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_chdir()));

#if 0
	// Get system info
	LogInfo(iSequenceSystem);
	m_nCurrentSequence = GEXFTP_SEQUENCE_SYSTEM;
	m_strRawCommand = "SYST\r\n";
	m_pclFtp->rawCommand(m_strRawCommand);
#else
	// Force the timer to issue signal for next state
	QTimer::singleShot(0, this, SLOT(ftp_sequencer()));
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: CHDIR
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_chdir()
{
	WriteDebugMessageFile("Move to " + m_clFtpServer.remoteDir() + "...");
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_list1()));

	// Change directory on server
	LogInfo(iSequenceChdir);
	m_nCurrentSequence = GEXFTP_SEQUENCE_CHDIR;
	m_strCDPaths << m_clFtpServer.remoteDir();
//	m_pclFtp->cd(m_strCDPaths.last());
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant(m_strCDPaths.last()));
	ftp_startCommand(GexFtpQFtp::Cd, commandArgs);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: LIST1
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_list1()
{
	WriteDebugMessageFile("List 1...");
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_postlist1()));

	// Get list of files from server
	m_uiNbFilesCorruptedDate = 0;
	m_uiNbTotalFiles = 0;
	m_uiNbFilesToTransfer = 0;
	m_uiNbFilesIgnored = 0;
	LogInfo(iSequenceGetList);
	m_nCurrentSequence = GEXFTP_SEQUENCE_LIST1;
	pwdAndList();
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: post LIST1
// The purpose of this state is just to select next state depending on some parameters
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_postlist1()
{
	WriteDebugMessageFile(QString::number(m_uiNbTotalFiles) + " file(s) checked");
	WriteDebugMessageFile(QString::number(m_uiNbFilesToTransfer) + " extension matching");
	WriteDebugMessageFile(QString::number(m_uiNbFilesCorruptedDate) + " with corrupted date");
	WriteDebugMessageFile(QString::number(m_uiNbFilesIgnored) + " ignored");

	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);

	// Check if list contains some files
	if(m_listFiles_1.isEmpty())
	{
		Cleanup();
		return;
	}

	// If we are in the mode where the files should be left as-is on the server,
	// update local file with list of transferred files, and remove those files that
	// are not in the list retrieved from the server
	if(m_clFtpServer.filePolicy() == GEXFTP_FILEPOLICY_LEAVE_TEXT)
		UpdateListOfFilesToTransfer();

	// Check if list contains some files
	if(m_listFiles_1.isEmpty())
	{
		Cleanup();
		return;
	}

	// Connect next Ftp sequence
	if (m_clFtpServer.disabledDateTimeCheck())
	{
		connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_list2()));
	}
	else
	{
		if(m_bRemoteTimestampFileExists)
		{
			connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_removetimestampfile()));
			m_bRemoteTimestampFileExists = false;
		}
		else
		{
			connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_puttimestampfile()));
		}
	}

	// Force the timer to issue signal for next state
	QTimer::singleShot(0, this, SLOT(ftp_sequencer()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: REMOVETIMESTAMPFILE
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_removetimestampfile()
{
	WriteDebugMessageFile("Remove timestamp...");
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_puttimestampfile()));

	// Remove timestamp file in case it exists
	LogInfo(iSequenceRemoveTimestampFile);
	m_nCurrentSequence = GEXFTP_SEQUENCE_REMOVETIMESTAMPFILE;
	m_strFileInTransferRemoteName = m_strTimestampFileRemoteName;
	QString strRemoteDirAndSlash = m_clFtpServer.remoteDir() + "/";
//	m_pclFtp->remove(strRemoteDirAndSlash + m_strTimestampFileRemoteName);
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant(strRemoteDirAndSlash + m_strTimestampFileRemoteName));
	ftp_startCommand(GexFtpQFtp::Remove, commandArgs);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: PUTTIMESTAMPFILE
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_puttimestampfile()
{
	WriteDebugMessageFile("Put timestamp...");
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);

	// Create the local timestamp file
	m_pTimestampFile = new QFile(m_strTimestampFileLocalName);
	LogInfo(iSequencePutTimestampFile);
	m_nCurrentSequence = GEXFTP_SEQUENCE_PUTTIMESTAMPFILE;
	if(m_pTimestampFile->open(QIODevice::WriteOnly) == false)
	{
		LogError(eCreateTimeStampFile);
		delete m_pTimestampFile;
		m_pTimestampFile = NULL;
		Cleanup();
		return;
	}
	// Set flag specifying the local timestamp file has been created
	m_bLocalTimestampFileCreated = true;
	QTextStream clStream(m_pTimestampFile);
	clStream << "GEX Ftp date test file." << endl;
	m_pTimestampFile->close();

	// Open local timestamp file
	if(m_pTimestampFile->open(QIODevice::ReadOnly) == false)
	{
		LogError(eReadTimeStampFile);
		delete m_pTimestampFile;
		m_pTimestampFile = NULL;
		Cleanup();
		return;
	}

	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_gettimestampfile()));

	// Put just created local file to the server to get the server's current time
	m_strFileInTransferLocalName = m_strTimestampFileLocalName;
	m_strFileInTransferRemoteName = m_strTimestampFileRemoteName;
	QString strRemoteDirAndSlash = m_clFtpServer.remoteDir() + "/";
//	m_pclFtp->put(m_pTimestampFile, strRemoteDirAndSlash + m_strTimestampFileRemoteName);
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant::fromValue<void *>(m_pTimestampFile));
	commandArgs.append(QVariant(strRemoteDirAndSlash + m_strTimestampFileRemoteName));
	ftp_startCommand(GexFtpQFtp::Put, commandArgs);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: GETTIMESTAMPFILE
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_gettimestampfile()
{
	WriteDebugMessageFile("Get timestamp...");
	// First sleep for 5 sec
	LogInfo(iSequenceSleep);
#if defined unix || __MACH__
	sleep(5);
#else
	Sleep(5000);
#endif

	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_list2()));

	// Close current timestamp file handle if opened
	if(m_pTimestampFile)
	{
		m_pTimestampFile->close();
		delete m_pTimestampFile;
		m_pTimestampFile = NULL;
	}

#if GEXFTP_READBACK_TIMESTAMP_FILE
	// Read back the timestamp file, to workaround a QT bug, where if just listing it, the last modification date or time can be corrupted
	LogInfo(iSequenceGetTimestampFile);
	m_nCurrentSequence = GEXFTP_SEQUENCE_GETTIMESTAMPFILE;
	m_strFinalLocalName = m_strTimestampFileLocalName;
	m_strFileInTransferLocalName = m_strTimestampFileLocalName;

	// Try to remove local file if it exists
	if((QFile::exists(m_strFinalLocalName)) && (QFile::remove(m_strFinalLocalName) == false))
	{
		LogError(eRemoveLocalFile);
		Cleanup();
		return;
	}

	// Open local file
	m_pTimestampFile = new QFile(m_strFileInTransferLocalName);
	if(m_pTimestampFile->open(QIODevice::WriteOnly) == false)
	{
		LogError(eCreateLocalFile);
		delete m_pTimestampFile;
		m_pTimestampFile = NULL;
		Cleanup();
		return ;
	}
	QString strRemoteDirAndSlash = m_clFtpServer.remoteDir() + "/";
//	m_pclFtp->get(strRemoteDirAndSlash + m_strFileInTransferRemoteName, m_pTimestampFile);
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant(strRemoteDirAndSlash + m_strFileInTransferRemoteName));
	commandArgs.append(QVariant::fromValue<void *>(m_pTimestampFile));
	ftp_startCommand(GexFtpQFtp::Get, commandArgs);
#else
	// Don't read back the timestamp file
	// Force the timer to issue signal for next state
	QTimer::singleShot(0, this, SLOT(ftp_sequencer()));
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: LIST2
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_list2()
{
	WriteDebugMessageFile("List 2...");
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_verify()));

	// Get list of files from server
	m_uiNbFilesCorruptedDate = 0;
	m_uiNbTotalFiles = 0;
	m_uiNbFilesToTransfer = 0;
	m_uiNbFilesIgnored = 0;
	LogInfo(iSequenceGetList);
	m_nCurrentSequence = GEXFTP_SEQUENCE_LIST2;
	m_strCDPaths << m_clFtpServer.remoteDir();
//	m_pclFtp->cd(m_strCDPaths.last()); // to ensure put it in remote folder
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant(m_strCDPaths.last()));
	ftp_startCommand(GexFtpQFtp::Cd, commandArgs);
	pwdAndList();
}

/////////////////////////////////////////////////////////////////////////////////////
// Correct server time
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::CorrectServerTime(QDateTime & clDateTime)
{
	QDateTime	clCurrentDateTime = QDateTime::currentDateTime();
	int			nTimeshift_Days;
	QString		strDateTime;

	strDateTime = clCurrentDateTime.toString("dd MMM yyyy hh:mm:ss");
	strDateTime = clDateTime.toString("dd MMM yyyy hh:mm:ss");

#ifdef QT_DEBUG
	bool bTest = clDateTime.date().isNull();
	bTest = clDateTime.date().isValid();
	bTest = clDateTime.time().isNull();
	bTest = clDateTime.time().isValid();
    Q_UNUSED(bTest)
#endif

	// If Date/Time not valid, use UTC date/time -12h
	if(clDateTime.isNull() || !clDateTime.isValid())
	{
		LogError(eReadServerTime);
		clDateTime = clCurrentDateTime.toUTC().addSecs(-12*3600);
		clDateTime.setTimeSpec(Qt::LocalTime);
		return;
	}

	// Get difference in days between server time and current time
	if(clDateTime < clCurrentDateTime)
		nTimeshift_Days = clDateTime.daysTo(clCurrentDateTime);
	else
		nTimeshift_Days = clCurrentDateTime.daysTo(clDateTime);

	// If difference > 7 days, make correction
	if(nTimeshift_Days <= 7)
		return;

	// Log error
	LogError(eCorruptedServerTime);

	// Correct server time
	QDate	clServerDate = clDateTime.date();
	QDate	clCurrentDate = clCurrentDateTime.date();
	int		nServerDay = clServerDate.day(), nServerMonth = clServerDate.month(), nServerYear = clServerDate.year();
	int		nCurrentMonth = clCurrentDate.month(), nCurrentYear = clCurrentDate.year();

	if((nCurrentMonth == 12) && (nServerMonth == 1))
		nServerYear = nCurrentYear + 1;
	else if((nCurrentMonth == 1) && (nServerMonth == 12))
		nServerYear = nCurrentYear -1;
	else
		nServerYear = nCurrentYear;

	clServerDate.setDate(nServerYear, nServerMonth, nServerDay);
	clDateTime.setDate(clServerDate);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: VERIFY
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_verify()
{
	WriteDebugMessageFile("Verify...");
	WriteDebugMessageFile(QString::number(m_uiNbTotalFiles) + " file(s) checked");
	WriteDebugMessageFile(QString::number(m_uiNbFilesToTransfer) + " extension matching");
	WriteDebugMessageFile(QString::number(m_uiNbFilesCorruptedDate) + " with corrupted date");
	WriteDebugMessageFile(QString::number(m_uiNbFilesIgnored) + " ignored");

	if (m_pDataSettings->dataGeneral().enableMaxFilePerProfile())
	{
		emit sMaxFileToDownLoadInfo(m_pDataSettings->dataGeneral().maxFilePerProfile());
		WriteDebugMessageFile("Max. nb. of files to download per profile: " + QString::number(m_pDataSettings->dataGeneral().maxFilePerProfile()));
	}

	bool				bStatus;
	QUrlInfo			clUrlInfo;
	QString				strFileTime, strTimeLimit, strMessage;

	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);

	// Add matching files to listview
	LogInfo(iSequenceVerify);

	// Datetime limit is server datetime minus 5 minutes
	int nFileAgeBeforeTransfer_Min = m_pDataSettings->dataGeneral().fileAgeBeforeTransfer_Min();
	m_clServerTimeLimit = m_clServerCurrentDateTime.addSecs(-nFileAgeBeforeTransfer_Min*60);
	strTimeLimit = m_clServerTimeLimit.toString("dd MMM yyyy hh:mm:ss");

	// Emit signal to display ftp server date/time
	emit sFtpServerDateTime(m_clFtpServer.settingsName(), m_clServerCurrentDateTime, m_clServerTimeLimit);

	// Check files matching in both lists with modification date older than limit
	m_itFilesToGet = m_listFiles_1.begin();
	while(m_itFilesToGet != m_listFiles_1.end())
	{
		clUrlInfo = *m_itFilesToGet;
		strFileTime = clUrlInfo.lastModified().toString("dd MMM yyyy hh:mm:ss");
		// If file is in the both lists
        if((m_listFiles_2.contains(*m_itFilesToGet)) &&
					((m_clFtpServer.disabledDateTimeCheck() == true) ||
					((m_clFtpServer.disabledDateTimeCheck() == false) && ((*m_itFilesToGet).lastModified() < m_clServerTimeLimit))))
			m_itFilesToGet++;
		else
		{
			m_uiNbFilesIgnored++;
			m_itFilesToGet = m_listFiles_1.erase(m_itFilesToGet);
		}
	}

	// Check if list contains some files
	if(m_listFiles_1.isEmpty())
	{
		WriteDebugMessageFile("No new file to transfer...");
		Cleanup();
		return;
	}

	// Insert files into list view
	m_uiNbFilesToTransfer = 0;
	m_uiNbFilesProcessed = 0;
	for(m_itFilesToGet = m_listFiles_1.begin(); m_itFilesToGet != m_listFiles_1.end(); m_itFilesToGet++)
	{
		emit sFileInfo(*m_itFilesToGet);
		m_uiNbFilesToTransfer++;
	}

	// Emit signal to display nb of files to transfer
	m_bEmitFilesToTransfer = true;

	// First file in the list
	m_itFilesToGet = m_listFiles_1.begin();

	// Start file transfer
	while((bStatus = StartFileTransfer()) == false)
	{
		if (m_pDataSettings->dataGeneral().enableMaxFilePerProfile() && (m_uiNbFilesProcessed >= (unsigned int ) m_pDataSettings->dataGeneral().maxFilePerProfile()))
		{
			Cleanup();
			return;
		}

		// Check if some more files to get
		if(++m_itFilesToGet == m_listFiles_1.end())
		{
			Cleanup();
			return;
		}
	}

	// Check if a file transfer was started
	if(bStatus == false)
	{
		Cleanup();
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: GET
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_get()
{
	bool bStatus;

	// Start next file transfer
	do
	{
		// Check if some more files to get
		if(++m_itFilesToGet == m_listFiles_1.end())
		{
			Cleanup();
			return;
		}
	}
	while((bStatus = StartFileTransfer()) == false);

	// Check if a file transfer was started
	if(bStatus == false)
	{
		Cleanup();
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: REMOTEREMOVE
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_remoteremove()
{
	WriteDebugMessageFile("Remote remove...");
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_get()));

	// Enter REMOTEREMOVE sequence
	LogInfo(iSequenceRemoteRemove);
	m_nCurrentSequence = GEXFTP_SEQUENCE_REMOTEREMOVE;

	// Schedule FTP Remove command
//	m_pclFtp->remove(m_strFileInTransferRemoteName);
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant(m_strFileInTransferRemoteName));
	ftp_startCommand(GexFtpQFtp::Remove, commandArgs);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: REMOTERENAME (step 1: remove dest file on remote server in case it exists)
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_remoterename_1()
{
	WriteDebugMessageFile("Remote rename 1...");
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_remoterename_2()));

	// Enter REMOTERENAME sequence
	LogInfo(iSequenceRemoteRename_1);
	m_nCurrentSequence = GEXFTP_SEQUENCE_REMOTERENAME_1;

	// Schedule FTP Remove command (remove of destination file)
	m_strFileInTransferRemoteName += ".gex";
//	m_pclFtp->remove(m_strFileInTransferRemoteName);
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant(m_strFileInTransferRemoteName));
	ftp_startCommand(GexFtpQFtp::Remove, commandArgs);
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: REMOTERENAME (step 2: rename file on remote server)
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_remoterename_2()
{
    WriteDebugMessageFile("Remote rename 2...");
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);
	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_get()));

	// Enter REMOTERENAME sequence
	LogInfo(iSequenceRemoteRename_2);
	m_nCurrentSequence = GEXFTP_SEQUENCE_REMOTERENAME_2;

	// Schedule FTP Rename command
	m_strFileInTransferRemoteName = m_strFileInTransferRemoteName.left(m_strFileInTransferRemoteName.length()-4);
//	m_pclFtp->rename(m_strFileInTransferRemoteName, m_strFileInTransferRemoteName+".gex");
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant(m_strFileInTransferRemoteName));
	commandArgs.append(QVariant(m_strFileInTransferRemoteName+".gex"));
	ftp_startCommand(GexFtpQFtp::Rename, commandArgs);
}

/////////////////////////////////////////////////////////////////////////////////////
// Make sure transferred file is same size as remote file
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtpDownload::CheckFileSize(qint64 & ui64_LocalSize, qint64 & ui64_RemoteSize)
{
	// Check if file size is identical
#ifdef QT_DEBUG
	qint64		ui64_BytesAvailable;
#endif
	QFileInfo	clLocalFile(m_strFileInTransferLocalName);

	ui64_LocalSize = clLocalFile.size();
	ui64_RemoteSize = (*m_itFilesToGet).size();

	if(ui64_LocalSize != ui64_RemoteSize)
	{
#ifdef QT_DEBUG
		ui64_BytesAvailable = m_pclFtp->bytesAvailable();	// For debug
        Q_UNUSED(ui64_BytesAvailable)
#endif
        // Trace message
        QString strMessage = QString("CheckfileSize() reported local and remote files differ in size.");
        strMessage += QString(" Local file name=%1, Local file size=%1, Remote file size=%1")
                .arg(m_strFileInTransferLocalName).arg(ui64_LocalSize).arg(ui64_RemoteSize);
        WriteDebugMessageFile(strMessage);
        return false;
	}

    // Trace message
    QString strMessage = QString("CheckfileSize() reported local and remote files have same size.");
    strMessage += QString(" Local file name=%1, Local file size=%2, Remote file size=%3")
            .arg(m_strFileInTransferLocalName).arg(ui64_LocalSize).arg(ui64_RemoteSize);
    WriteDebugMessageFile(strMessage);
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Add transferred file to list of files transferred
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtpDownload::AddToTransferredFilesList()
{
	// Open file with list of transferred files
	QFile clFile(m_strTransferredListFileName);
	if(!clFile.open(QIODevice::WriteOnly | QIODevice::Append))
		return false;

	// Add to file
	QTextStream strFileStream(&clFile);
	QString strFileToTransfer = m_strFileInTransferRemoteName;
	QString strRemoteDirAndSlash = m_clFtpServer.remoteDir() + "/";
        strFileStream << strFileToTransfer.remove(strRemoteDirAndSlash,
                                                  m_pDataSettings->dataGeneral().caseInsensitive() ? Qt::CaseInsensitive : Qt::CaseSensitive) << endl;
	clFile.close();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: REMOTELEAVE
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_remoteleave()
{
    WriteDebugMessageFile("Remote leave...");
	bool	bStatus=false;
	QDir	clDir;

	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);

	// Enter REMOTELEAVE sequence (next sequence decided by StartFileTransfer)
	LogInfo(iSequenceRemoteLeave);
	m_nCurrentSequence = GEXFTP_SEQUENCE_REMOTELEAVE;

	// Add file in list of files transferred, set bStatus to true if file correctly inserted, to false else
	bStatus = AddToTransferredFilesList();
	if(!bStatus)
	{
		emit sTransferStatus(GEXFTP_FILESTATUS_ERROR, m_strFileInTransferRemoteName.section('/', -1, -1));
		// Remove local file
        clDir.remove(m_strFileInTransferLocalName);
	}
	else
	{
		// Rename local file
		clDir.rename(m_strFileInTransferLocalName, m_strFinalLocalName);
		emit sTransferStatus(GEXFTP_FILESTATUS_TRANSFERRED, m_strFileInTransferRemoteName.section('/', -1, -1));

		// Emit signal for file transferred
		emit sFileTransferred(m_clFtpServer.settingsName(), m_strFileInTransferRemoteName.section('/', -1, -1));

		// Update file count
		m_uiNbFilesTransferred++;
	}

	// Get next file
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_get()));
	QTimer::singleShot(0, this, SLOT(ftp_sequencer()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence: CHECK SIZE OF DOWNLOADED FILE
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequence_checkfile()
{
	QDir	clDir;

	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);

	// Enter CHECKFILE sequence
	LogInfo(iSequenceCheckFile);
	m_nCurrentSequence = GEXFTP_SEQUENCE_CHECKFILE;

	// Check if file size is identical
    qint64 ui64_LocalSize, ui64_RemoteSize;
    if(!CheckFileSize(ui64_LocalSize, ui64_RemoteSize))
	{
		// Transfer error signal
		emit sTransferStatus(GEXFTP_FILESTATUS_ERROR, m_strFileInTransferRemoteName.section('/', -1, -1));
		// Remove local file
        clDir.remove(m_strFileInTransferLocalName);

		// Get next file
		connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_get()));
	}
	else
	{
		// Transferred file is OK: connect next Ftp sequence
		if(m_clFtpServer.filePolicy() == GEXFTP_FILEPOLICY_LEAVE_TEXT)
			connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_remoteleave()));
		else if(m_clFtpServer.filePolicy() == GEXFTP_FILEPOLICY_REMOVE_TEXT)
			connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_remoteremove()));
		else
			connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_remoterename_1()));
	}

	// Force the timer to issue signal for next state
	QTimer::singleShot(0, this, SLOT(ftp_sequencer()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Ftp sequence control
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_sequencer()
{
	emit sNextSequence();
}

/////////////////////////////////////////////////////////////////////////////////////
// Log info
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::LogInfo(int nInfoCode)
{
	QString strInfoMsg;

	switch(nInfoCode)
	{
		case iStateUnconnected:
			LogMessage(GEXFTP_MSG_INFO, "Unconnected");
			break;
		case iStateHostLookup:
			LogMessage(GEXFTP_MSG_INFO, "Host lookup");
			break;
		case iStateConnecting:
			strInfoMsg = "Connecting to Ftp server (Name=" + m_strFtpSiteName + ", IP=" + m_strFtpSiteIP;
			strInfoMsg += ", Port=" + QString::number(m_clFtpServer.ftpPortNb()) + ")";
			LogMessage(GEXFTP_MSG_INFO, strInfoMsg);
			break;
		case iStateConnected:
			LogMessage(GEXFTP_MSG_INFO, "Connected");
			break;
		case iStateLoggedIn:
			strInfoMsg = "Logged in (User=" + m_clFtpServer.login() + ")";
			LogMessage(GEXFTP_MSG_INFO, strInfoMsg);
			break;
		case iStateClosing:
			LogMessage(GEXFTP_MSG_INFO, "Closing connection");
			break;
		case iSequenceConnect:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Connection");
			break;
		case iSequenceLogin:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Login");
			break;
		case iSequenceSystem:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: System");
			break;
		case iSequenceChdir:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Chdir");
			break;
		case iSequenceRemoveTimestampFile:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Removing timestamp file");
			break;
		case iSequenceGetList:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Getting directory list");
			break;
		case iSequenceSleep:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Sleep");
			break;
		case iSequenceVerify:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Verify");
			break;
		case iSequencePutTimestampFile:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Transferring timestamp file");
			break;
		case iSequenceGetTimestampFile:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Reading back timestamp file");
			break;
		case iSequenceGet:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Getting files");
			break;
		case iSequenceRemoteRemove:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Removing remote file");
			break;
		case iSequenceRemoteRename_1:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Renaming remote file (step 1: remove destination file of the rename, in case it exists)");
			break;
		case iSequenceRemoteRename_2:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Renaming remote file (step 2: rename file)");
			break;
		case iSequenceRemoteLeave:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Adding file to list of transferred files");
			break;
		 case iSequenceCheckFile:
			 LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Checking transferred file");
			 break;
		case iSequenceCleanup:
			LogMessage(GEXFTP_MSG_INFO_SEQ, "SEQUENCE: Cleanup");
			break;
		case iCommandStarted:
			switch(m_pclFtp->currentCommand())
			{
				case GexFtpQFtp::ConnectToHost:
					LogMessage(GEXFTP_MSG_INFO, "COMMAND connectToHost");
					break;
				case GexFtpQFtp::Login:
					LogMessage(GEXFTP_MSG_INFO, "COMMAND login");
					break;
				case GexFtpQFtp::Close:
					LogMessage(GEXFTP_MSG_INFO, "COMMAND close");
					break;
				case GexFtpQFtp::List:
					LogMessage(GEXFTP_MSG_INFO, "COMMAND list");
					break;
				case GexFtpQFtp::Cd:
					if (!m_strCDPaths.isEmpty())
						strInfoMsg = "COMMAND cd (" + m_strCDPaths.takeFirst() + ")";
					LogMessage(GEXFTP_MSG_INFO, strInfoMsg);
					break;
				case GexFtpQFtp::Get:
					strInfoMsg = "COMMAND get (" + m_strFileInTransferLocalName + " <- " + m_strFileInTransferRemoteName + ")";
					LogMessage(GEXFTP_MSG_INFO, strInfoMsg);
					break;
				case GexFtpQFtp::Put:
					strInfoMsg = "COMMAND put (" + m_strFileInTransferLocalName + " -> " + m_strFileInTransferRemoteName + ")";
					LogMessage(GEXFTP_MSG_INFO, strInfoMsg);
					break;
				case GexFtpQFtp::Remove:
					strInfoMsg = "COMMAND remove (" + m_strFileInTransferRemoteName + ")";
					LogMessage(GEXFTP_MSG_INFO, strInfoMsg);
					break;
				case GexFtpQFtp::Mkdir:
					LogMessage(GEXFTP_MSG_INFO, "COMMAND mkdir");
					break;
				case GexFtpQFtp::Rmdir:
					LogMessage(GEXFTP_MSG_INFO, "COMMAND rmdir");
					break;
				case GexFtpQFtp::Rename:
					strInfoMsg = "COMMAND rename (" + m_strFileInTransferRemoteName + ", " + m_strFileInTransferRemoteName + ".gex)";
					LogMessage(GEXFTP_MSG_INFO, strInfoMsg);
					break;
				case GexFtpQFtp::RawCommand:
					strInfoMsg = "COMMAND rawCommand (" + m_strRawCommand.trimmed() + ")";
					LogMessage(GEXFTP_MSG_INFO, strInfoMsg);
					break;
				case GexFtpQFtp::None:
				default:
					break;
			}
			break;
			case iCommandFinishedOK:
				if(m_bDisplaySystem)
				{
					m_bDisplaySystem = false;
					strInfoMsg = "OK (" + m_strSystem + ")";
					LogMessage(GEXFTP_MSG_INFO, strInfoMsg);
				}
				else
					LogMessage(GEXFTP_MSG_INFO, "OK");
				break;
			case iCommandFinishedNOK:
				LogMessage(GEXFTP_MSG_INFO, "NOK");
				break;
		default:
			return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Log error
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::LogError(int nErrorCode, bool bLocal/*=false*/)
{
	QString strErrorMsg;
	QRegExp	clRegExp;
	QString strFtpError;

	switch(nErrorCode)
	{
		case eFtpError:
			clRegExp.setPattern("[\n\r\t]");
			strFtpError = m_pclFtp->errorString().replace(clRegExp, " ");
			if(m_nCurrentSequence == GEXFTP_SEQUENCE_GET)
				strErrorMsg = "FTP error during transfer of " + m_strFileInTransferRemoteName + ": " + strFtpError;
			else if(m_nCurrentSequence == GEXFTP_SEQUENCE_REMOTEREMOVE)
				strErrorMsg = "FTP error during remote remove of " + m_strFileInTransferRemoteName + ": " + strFtpError;
			else if(m_nCurrentSequence == GEXFTP_SEQUENCE_REMOTERENAME_2)
				strErrorMsg = "FTP error during remote rename of " + m_strFileInTransferRemoteName + ": " + strFtpError;
			else if(m_nCurrentSequence == GEXFTP_SEQUENCE_CHDIR)
				strErrorMsg = "FTP error: " + strFtpError + "(<" + m_clFtpServer.remoteDir() +
						  ">: make sure the \'Remote folder\' specified in your profile is relative to your Ftp login home directory).";
			else
				strErrorMsg = "FTP error: " + strFtpError;
			break;
		case eCreateTimeStampFile:
			strErrorMsg = "Error creating timestamp file: " + m_strTimestampFileLocalName;
			break;
		case eReadServerTime:
			strErrorMsg = "Error reading FTP server date/time, using UTC-12 hours";
			break;
		case eCorruptedServerTime:
			strErrorMsg = "Read corrupted FTP server date/time (" + m_clServerCurrentDateTime.toString("dd MMM yyyy hh:mm:ss") + "), trying to repair";
			break;
		case eReadTimeStampFile:
			strErrorMsg = "Error reading timestamp file: " + m_strTimestampFileLocalName;
			break;
		case eCreateLocalFile:
			strErrorMsg = "Error creating local file: " + m_strFileInTransferLocalName;
			break;
		case eRemoveLocalFile:
			strErrorMsg = "Error removing local file: " + m_strFinalLocalName;
			break;
		case eOperationAborted:
			strErrorMsg = "Operation aborted";
			break;
		case eOperationTimedOut:
			strErrorMsg = "Operation timed out";
			m_bIsConnectionOK = false;
			break;
		case eFtpConnectionBroken:
			strErrorMsg = "Ftp connection lost";
			m_bIsConnectionOK = false;
			break;
		case eConnect:
			strErrorMsg = "Error connecting to ftp server: " + m_strFtpServer;
			m_bIsConnectionOK = false;
			break;
		case eInvalidDateTime_RemoteFile:
			strErrorMsg = "Read corrupted file date/time property: " + m_strFileInTransferRemoteName;
			break;
		case eInvalidDateTime_RemoteFiles:
			strErrorMsg = "Read " + QString::number(m_uiNbFilesCorruptedDate) + " files with corrupted date/time property";
			if(m_nCurrentSequence == GEXFTP_SEQUENCE_LIST1)
				strErrorMsg += " (list1)";
			else
				strErrorMsg += " (list2)";
			break;
		default:
			return;
	}
	WriteDebugMessageFile("Log error: " + strErrorMsg + " (sequence id=" + QString::number(m_nCurrentSequence) +")");
	LogMessage(GEXFTP_MSG_ERROR, strErrorMsg);

	if(!bLocal)
		emit sTransferError(m_clFtpServer.settingsName(), strErrorMsg);

	qDebug() << strErrorMsg;
}

/////////////////////////////////////////////////////////////////////////////////////
// Log info
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::LogMessage(int nErrorCode, const QString& strMessage)
{
	emit sLogMessage(nErrorCode, strMessage);
}

/////////////////////////////////////////////////////////////////////////////////////
// Cleanup
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::Cleanup(bool bNoFtpActions/* = false*/)
{
	WriteDebugMessageFile("Cleanup...");
	bool bFtpAction = false;

	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);

	// Disable Abort button
	emit sCleanup();

	// Try to remove temporary local file
	if(m_nCurrentSequence == GEXFTP_SEQUENCE_GET)
		QFile::remove(m_strFileInTransferLocalName);

	// Switch to Cleanup mode?
	if(m_nCurrentSequence != GEXFTP_SEQUENCE_CLEANUP)
	{
		LogInfo(iSequenceCleanup);
		m_nCurrentSequence = GEXFTP_SEQUENCE_CLEANUP;
	}

	// Remove local timestamp file?
	if(m_bLocalTimestampFileCreated == true)
		QFile::remove(m_strTimestampFileLocalName);

	// Remove temporary remote file?
	if(!bNoFtpActions)
	{
		if(m_bRemoteTimestampFileCreated && (m_pclFtp->state() == GexFtpQFtp::LoggedIn))
		{
			bFtpAction = true;
			m_strFileInTransferRemoteName = m_clFtpServer.remoteDir() + "/" + m_strTimestampFileRemoteName;
//			m_pclFtp->remove(m_strFileInTransferRemoteName);
			QList<QVariant> commandArgs;
			commandArgs.append(QVariant(m_strFileInTransferRemoteName));
			ftp_startCommand(GexFtpQFtp::Remove, commandArgs);
		}

		// Close the Ftp connection
		if(m_pclFtp->state() != GexFtpQFtp::Unconnected)
		{
			bFtpAction = true;
//			m_pclFtp->close();
			ftp_startCommand(GexFtpQFtp::Close);
		}
	}

	if(!bFtpAction)
	{
		// Disconnect all signals of the Ftp object!!!!!!!
		disconnect(m_pclFtp, 0, 0, 0);

		// Emit 'Transfer done' signal on next timer tick
		m_bEmitTransferDone = true;
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Try to resolve host name domain<->IP
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ResolveHost()
{
	// Make sure server name is in IP format
	m_strFtpSiteName = "???";
	m_strFtpSiteIP = "???";
	m_strFtpServer = m_clFtpServer.ftpSiteURL();

	QHostAddress clHostAddr;
	if(clHostAddr.setAddress(m_strFtpServer) == false)
	{
		// The server name is in domain name format. Try to retrieve the IP address.
		m_strFtpSiteName = m_strFtpServer;
		hostent* host = 0;
        host = gethostbyname(m_strFtpServer.toLatin1().constData());
		if(host)
		{
			unsigned long ul_HostAddress;
			memcpy(&(ul_HostAddress), host->h_addr, host->h_length);
			ul_HostAddress = htonl(ul_HostAddress);
			clHostAddr.setAddress((quint32)ul_HostAddress);
			m_strFtpSiteIP = clHostAddr.toString();
			m_strFtpServer = m_strFtpSiteIP;
		}
	}
	else
	{
		// The server name is in IP address format. Try to retrieve the server name.
		m_strFtpSiteIP = m_strFtpServer;
		unsigned long ul_HostAddress = (unsigned long)clHostAddr.toIPv4Address();
		ul_HostAddress = ntohl(ul_HostAddress);
		char *pHostAddress = (char *)&ul_HostAddress;
		struct hostent *host = 0;
		host = gethostbyaddr(pHostAddress, 4, AF_INET);
		if(host)
			m_strFtpSiteName = host->h_name;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Timer function
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::OnTimer()
{
	int	nSecondsToTimeout;

    // If GET command, update download status controls
    if(m_nCurrentSequence == GEXFTP_SEQUENCE_GET)
    {
        int nDuration_Seconds	= m_clGetStartDateTime.secsTo(QDateTime::currentDateTime());
        double dSpeedBytePerSec = (m_nTransferredData / 1000.0) / nDuration_Seconds;
        int nSpeed				= (int)dSpeedBytePerSec;

        emit sDownloadInfo(m_nTransferredData, m_nTotalDataToTransfer, nSpeed);
    }

	// Check command timeout
	if(m_bCommandInProgress && m_pDataSettings->dataGeneral().ftpTimeout())
	{
		nSecondsToTimeout = m_pDataSettings->dataGeneral().ftpTimeoutAfter() - m_clCommandStartDateTime.secsTo(QDateTime::currentDateTime());
        if (nSecondsToTimeout <= 0)
        {
            QString strMessage = QString("Timeout occured while in sequence %1!").arg(m_nCurrentSequence);
            WriteDebugMessageFile(strMessage);
            if((m_nCurrentSequence == GEXFTP_SEQUENCE_GET) || (m_nCurrentSequence == GEXFTP_SEQUENCE_CHECKFILE))
            {
                qint64 ui64_LocalSize, ui64_RemoteSize;
                // Called to write trace message on file size check
                CheckFileSize(ui64_LocalSize, ui64_RemoteSize);
            }
			// Abort
			abort(eOperationTimedOut);
        }
		else if(nSecondsToTimeout != m_nSecondsToTimeout)
		{
			emit sChangeTimeout(nSecondsToTimeout);

			m_nSecondsToTimeout = nSecondsToTimeout;
		}
	}

	// Check if some signals should be emitted
	if(m_bEmitTransferDone)
	{
		m_bEmitTransferDone = false;
        emit sTransferDone(m_clFtpServer.settingsName(),
                           m_uiNbFilesTransferred);
	}
	else if(m_bEmitFilesToTransfer)
	{
		m_bEmitFilesToTransfer = false;
		emit sFilesToTransfer(m_clFtpServer.settingsName(), m_uiNbFilesToTransfer, m_uiNbFilesIgnored);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Abort operations
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::abort(int nAbortType)
{
    WriteDebugMessageFile("Abort");
	// Reset 'command in progress' flag
	m_bCommandInProgress = false;
	emit sChangeTimeout(-1);

	// Command not completed message
	LogInfo(iCommandFinishedNOK);

	// Close current file handle if opened
	CloseLocalFile(false);

	// Delete ftp object
    m_pclFtp->disconnect();
    m_pclFtp->deleteLater();
    m_pclFtp = new GexFtpQFtp();
	LogError(nAbortType);

	// Cleanup?
	if(nAbortType == eOperationTimedOut)
		Cleanup(true);
	else
		Cleanup();
}

/////////////////////////////////////////////////////////////////////////////////////
// Called during Ftp transfer to notify transfer progress
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::OnTransferProgress(qint64 nTransferred, qint64 nTotalToTransfer)
{
	// restart the sequence timer
	m_pSequenceTimer->start();

	// Update progress only for downloads in the GET sequence
	if(m_nCurrentSequence != GEXFTP_SEQUENCE_GET)
		return;

	// If transfer progressed, reset command start time, to avoid automatic timeout
    if(nTransferred > m_nTransferredData)
		m_clCommandStartDateTime = QDateTime::currentDateTime();

    if (nTotalToTransfer <= 0)
        nTotalToTransfer = (*m_itFilesToGet).size();

	// Save progress
    m_nTransferredData = nTransferred;
    m_nTotalDataToTransfer = nTotalToTransfer;

	// Update progress bar
    emit sDownloadProgress(nTransferred, nTotalToTransfer);
}

/////////////////////////////////////////////////////////////////////////////////////
// Start file transfer to get remote file
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtpDownload::StartFileTransfer()
{
	// Disconnect from current Ftp sequence
	disconnect(this, SIGNAL(sNextSequence()), 0, 0);

	// Before starting download ensure max number of file per
	if (m_pDataSettings->dataGeneral().enableMaxFilePerProfile() && (m_uiNbFilesProcessed >= (unsigned int ) m_pDataSettings->dataGeneral().maxFilePerProfile()))
	{
		return false;
	}

	// Update File counter
	m_uiNbFilesProcessed++;

	m_strFinalLocalName = m_clFtpServer.localDir();
	m_strFinalLocalName += "/";
	m_strFinalLocalName += (*m_itFilesToGet).name().section('/', -1, -1);
	CGexSystemUtils::NormalizePath(m_strFinalLocalName);
	m_strFileInTransferLocalName = m_strFinalLocalName + ".__tmp__";
	m_strFileInTransferRemoteName = m_clFtpServer.remoteDir() + "/" + (*m_itFilesToGet).name();

	WriteDebugMessageFile("Transfer " + (*m_itFilesToGet).name());
	emit sStartFileTransfer(m_strFinalLocalName.section('/', -1, -1), m_uiNbFilesProcessed, m_uiNbFilesToTransfer);

	// Try to remove local file if it exists
	if((QFile::exists(m_strFinalLocalName)) && (QFile::remove(m_strFinalLocalName) == false))
	{
		LogError(eRemoveLocalFile);

		emit sTransferStatus(GEXFTP_FILESTATUS_ERROR, m_strFinalLocalName, true);

		return false;
	}

	// Open local file
	if(!OpenLocalFile())
		return false;

	// Enter GET sequence (next sequence decided by StartFileTransfer)
	LogInfo(iSequenceGet);
	m_nCurrentSequence = GEXFTP_SEQUENCE_GET;
    m_nTransferredData = 0;
    m_nTotalDataToTransfer = 0;

	emit sDownloadInfo(0, (*m_itFilesToGet).size(), 0);

	emit sTransferStatus(GEXFTP_FILESTATUS_TRANSFERRING, m_strFileInTransferRemoteName.section('/', -1, -1), true);

	// Connect next Ftp sequence
	connect(this, SIGNAL(sNextSequence()), this, SLOT(ftp_sequence_checkfile()));

	// Schedule FTP Get command
//	m_pclFtp->get(m_strFileInTransferRemoteName, m_pFile);
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant(m_strFileInTransferRemoteName));
	commandArgs.append(QVariant::fromValue<void *>(m_pFile));
	ftp_startCommand(GexFtpQFtp::Get, commandArgs);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Update list of files to transfer and file containg list of transferred files
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::UpdateListOfFilesToTransfer()
{
	// Trace message
	QString strMessage = "Checking files already transferred using local history file " + m_strTransferredListFileName;
	strMessage += "...";
	WriteDebugMessageFile(strMessage);

	// Open file with list of transferred files
	QFile clFile(m_strTransferredListFileName);
	if(!clFile.open(QIODevice::ReadOnly))
		return;

	// List of files retrieved from the server
	QStringList strlFilesOnServer;
	for(m_itFilesToGet = m_listFiles_1.begin(); m_itFilesToGet != m_listFiles_1.end(); m_itFilesToGet++)
		strlFilesOnServer << (*m_itFilesToGet).name();

	// Trace message
	strMessage = QString::number(strlFilesOnServer.count()) + " files on remote server.";
	WriteDebugMessageFile(strMessage);

	// Read list of files already transferred, and check if those files are still on the server
	bool			bRewriteFile = false;
	QStringList		strFilesFromList;
	QString			strLine;
	QTextStream		strFileStream(&clFile);
	unsigned int	uiLinesRead=0, uiLinesRemoved=0;

	strLine = strFileStream.readLine();
	while(!strLine.isNull())
	{
		if(!strLine.isEmpty())
		{
			uiLinesRead++;

			if(strlFilesOnServer.contains(strLine))
				strFilesFromList += strLine;
			else
			{
				bRewriteFile = true;
				uiLinesRemoved++;
			}
		}
		strLine = strFileStream.readLine();
	}

	// Close file
	clFile.close();

	// Trace message
	strMessage = QString::number(uiLinesRead) + " lines read in local history file.";
	WriteDebugMessageFile(strMessage);
	strMessage = QString::number(uiLinesRemoved) + " lines removed from local history file.";
	WriteDebugMessageFile(strMessage);

	// Rewrite file?
	if(bRewriteFile)
	{
		clFile.open(QIODevice::WriteOnly);
		strFileStream.setDevice(&clFile);
		for(QStringList::Iterator it = strFilesFromList.begin(); it != strFilesFromList.end(); ++it)
			strFileStream << *it << endl;
		clFile.close();
	}

	// Remove files already transferred from list of files to transfer
	uiLinesRemoved	= 0;
	m_itFilesToGet	= m_listFiles_1.begin();
	while(m_itFilesToGet != m_listFiles_1.end())
	{
		if(strFilesFromList.contains((*m_itFilesToGet).name()))
		{
			uiLinesRemoved++;
			m_itFilesToGet = m_listFiles_1.erase(m_itFilesToGet);
		}
		else
			m_itFilesToGet++;
	}

	// Trace message
	strMessage = QString::number(uiLinesRemoved) + " files removed from files to transfer.";
	WriteDebugMessageFile(strMessage);
	strMessage = QString::number(m_listFiles_1.count()) + " files remaining to be transferred.";
	WriteDebugMessageFile(strMessage);
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal abort from the user
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::onUserAbort()
{
	abort(CGexFtpDownload::eOperationAborted);
}

/////////////////////////////////////////////////////////////////////////////////////
// Reset variable members used during the downloading
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::reset()
{
	WriteDebugMessageFile("Reset...");

	m_listFiles_1.clear();
	m_listFiles_2.clear();

	m_bLocalTimestampFileCreated	= false;
	m_bRemoteTimestampFileCreated	= false;
	m_bRemoteTimestampFileExists	= false;
	m_bEmitTransferDone				= false;
	m_bEmitFilesToTransfer			= false;
	m_uiNbFilesTransferred			= 0;
	m_uiNbFilesToTransfer			= 0;
	m_uiNbFilesIgnored				= 0;
	m_uiNbFilesProcessed			= 0;
	m_bCommandInProgress			= false;
	m_pFile							= NULL;
	m_bDisplaySystem				= false;
	m_bRecurseList					= true;
    m_nTransferredData						= 0;
    m_nTotalDataToTransfer					= 0;

	if (m_pTimestampFile)
	{
		delete m_pTimestampFile;
		m_pTimestampFile = NULL;
	}

	if (m_pclFtp)
	{
        m_pclFtp->deleteLater();
		m_pclFtp = NULL;
	}
}


/////////////////////////////////////////////////////////////////////////////////////
// Launch pwd raw command then list()
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::pwdAndList()
{
//	m_pclFtp->rawCommand("pwd");
	QList<QVariant> commandArgs;
	commandArgs.append(QVariant("pwd"));
	ftp_startCommand(GexFtpQFtp::RawCommand, commandArgs);
//	m_pclFtp->list();
	ftp_startCommand(GexFtpQFtp::List);
}
/////////////////////////////////////////////////////////////////////////////////////
// Opens local file for get command
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtpDownload::OpenLocalFile()
{
	// Open local file
	m_pFile = new QFile(m_strFileInTransferLocalName);
	if(m_pFile->open(QIODevice::WriteOnly) == false)
	{
		LogError(eCreateLocalFile);
		delete m_pFile;
		m_pFile = NULL;

		emit sTransferStatus(GEXFTP_FILESTATUS_ERROR, m_strFileInTransferRemoteName.section('/', -1, -1), true);

		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Closes local file after get completed
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::CloseLocalFile(bool bWriteMissingData)
{
	if(!m_pFile)
		return;

	// Flush the file
	m_pFile->flush();

	// Should we check for missing data?
	if(bWriteMissingData)
	{
		// Check if still some data available in the socket buffer
		// Since QT4, the commandFinished is received after a get is complete, despite some data is still available in
		// the socket buffer, not written to the file
		qint64		ui64_LocalSize, ui64_RemoteSize, ui64_BytesAvailable;
		QFileInfo	clLocalFile(m_strFileInTransferLocalName);

		ui64_LocalSize = clLocalFile.size();
		ui64_RemoteSize = (*m_itFilesToGet).size();
		if(ui64_LocalSize != ui64_RemoteSize)
		{
			// If the remaining data corresponds to the size difference, write it!
			ui64_BytesAvailable = m_pclFtp->bytesAvailable();
			if(ui64_BytesAvailable == (ui64_RemoteSize - ui64_LocalSize))
			{
				QByteArray clSocketBuffer = m_pclFtp->readAll();
				m_pFile->write(clSocketBuffer);
				m_pFile->flush();
			}
		}
	}

	// Close the file
	m_pFile->close();
	delete m_pFile;
	m_pFile = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
// Launch Ftp command method and set the timeout interval
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpDownload::ftp_startCommand(GexFtpQFtp::Command command, const QList<QVariant> &args)
{
	QFile* pFile = NULL;
	switch (command)
	{
	// None
	case GexFtpQFtp::None:
		break;
	// Set transfer mode
	case GexFtpQFtp::SetTransferMode:
		if (args.size() == 1)
			m_pclFtp->setTransferMode((GexFtpQFtp::TransferMode)args[0].toInt());
		break;
	// Set proxy
	case GexFtpQFtp::SetProxy:
		if (args.size() == 2)
			m_pclFtp->setProxy(args[0].toString(), args[1].toInt());
		break;
	// Connect to host
	case GexFtpQFtp::ConnectToHost:
		if (args.size() == 2)
			m_pclFtp->connectToHost(args[0].toString(), args[1].toInt());
		break;
	// Login
	case GexFtpQFtp::Login:
		if (args.isEmpty())
			m_pclFtp->login();
		else if (args.size() == 2)
			m_pclFtp->login(args[0].toString(), args[1].toString());
		break;
	// Close
	case GexFtpQFtp::Close:
		m_pclFtp->close();
		break;
	// List
	case GexFtpQFtp::List:
		if (args.isEmpty())
			m_pclFtp->list();
		else
			m_pclFtp->list(args[0].toString());
		break;
	// Cd
	case GexFtpQFtp::Cd:
		if (args.size() == 1)
			m_pclFtp->cd(args[0].toString());
		break;
	// Get
	case GexFtpQFtp::Get:
		if (args.size() >= 2)
			pFile = static_cast<QFile *>(args[1].value<void *>());
		if (pFile != NULL)
		{
			if (args.size() == 2)
				m_pclFtp->get(args[0].toString(), pFile);
			else
				m_pclFtp->get(args[0].toString(), pFile, (GexFtpQFtp::TransferType)args[2].toInt());
		}
		break;
	// Put
	case GexFtpQFtp::Put:
		if (args.size() >= 2)
			pFile = static_cast<QFile *>(args[0].value<void *>());
		if (pFile != NULL)
		{
			if (args.size() == 2)
				m_pclFtp->put(pFile, args[1].toString());
			else
				m_pclFtp->put(pFile, args[1].toString(), (GexFtpQFtp::TransferType)args[2].toInt());
		}
		break;
	// Remove
	case GexFtpQFtp::Remove:
		if (args.size() == 1)
			m_pclFtp->remove(args[0].toString());
		break;
	// Mkdir
	case GexFtpQFtp::Mkdir:
		if (args.size() == 1)
			m_pclFtp->mkdir(args[0].toString());
		break;
	// Rmdir
	case GexFtpQFtp::Rmdir:
		if (args.size() == 1)
			m_pclFtp->rmdir(args[0].toString());
		break;
	// Rename
	case GexFtpQFtp::Rename:
		if (args.size() == 2)
			m_pclFtp->rename(args[0].toString(), args[1].toString());
		break;
	// Raw command
	case GexFtpQFtp::RawCommand:
		if (args.size() == 1)
		m_pclFtp->rawCommand(args[0].toString());
		break;
	}

	m_clCommandStartDateTime = QDateTime::currentDateTime();
	m_bCommandInProgress = true;

	// Init start date/time of GET command
	if(m_nCurrentSequence == GEXFTP_SEQUENCE_GET)
		m_clGetStartDateTime = QDateTime::currentDateTime();

	if(m_pDataSettings->dataGeneral().ftpTimeout())
	{
		m_nSecondsToTimeout = m_pDataSettings->dataGeneral().ftpTimeoutAfter();
		emit sChangeTimeout(m_nSecondsToTimeout);
	}
}


void CGexFtpDownload::onSequenceTimeout()
{
	WriteDebugMessageFile("SEQUENCE TIMEOUT!");
	abort(CGexFtpDownload::eOperationAborted);
}


