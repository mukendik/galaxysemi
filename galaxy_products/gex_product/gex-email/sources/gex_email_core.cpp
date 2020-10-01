#include "gex_email_core.h"
#include "gex_email_send.h"
#include "gex_email_smtp.h"

#include <gstdl_utils_c.h>
#include <gqtl_sysutils.h>
#include <gex_constants.h>

#include <QRegExpValidator>


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexEmailCore::CGexEmailCore(const QString & strUserHome, const QString & strApplicationDir, const QString & strLocalFolder, const QString & strApplicationName) : QObject()
{
	// Signals / Slots connections
   // connect( this, SIGNAL( sSettingsChanged() ), this, SLOT( OnSettingsChanged() ) );
	        
	// Initialize data
	m_strUserHome = strUserHome;
	m_strLocalFolder = strLocalFolder;
	m_strApplicationDir = strApplicationDir;
	m_strApplicationName = strApplicationName;
	m_strSettingsFile = m_strLocalFolder + "/.gex_email.conf";
	CGexSystemUtils::NormalizePath(m_strSettingsFile);
	m_nNextSpooling_Sec = -1;

	m_nHours	= 0;
	m_nMinutes	= 0;
	m_nSeconds	= 0;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: creating Email window object", false);
#endif
	
	// SMTP Email
	m_pSmtp = new CGexEmailSmtp(this);

	m_listEmailSystems.append(m_pSmtp);
	connect(m_pSmtp, SIGNAL(sSettingsChanged()), this, SLOT(OnSettingsChanged()));
	
	// Activ email system is Smtp
	m_pEmailSystem = m_pSmtp;

	// Initially retrieve list of .mail files
	m_bRetrieveListOfMailFiles = true;

	// Log files setup
	m_nLogFilter = GEXEMAIL_MSG_ERROR;
	m_uiLogCleanupInterval = 3600*1000;

	// At startup, spooling is off, and validated only if Init() successful
	m_bSpoolingTimerON = false;
	m_bSpoolingInProgress = false;

	// For Log file cleanup
	m_pLogCleanupTime = new QTime;

	// Timers
	m_bFirstTimerEvent = false;
	m_pSecTimer = new QTimer(this);
	connect( m_pSecTimer, SIGNAL(timeout()), this, SLOT(OnSecTimerDone()) );
	m_pSecTimer->start(1000);

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: email window object created", false);
#endif
	
	// Update Log File
	Info(iStartApp);

	emit sStatusChanged(StatusReady);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexEmailCore::~CGexEmailCore()
{
	// no need to delete child widgets, Qt does it all for us
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: destroying Email window object.", false);
#endif	
	
	// Update Log File
	Info(iStopApp);
}

void CGexEmailCore::setDaysToKeepFiles(int nDays)
{
	m_uiDaysToKeepLogFiles = (uint) nDays;

	OnSettingsChanged();
}


void CGexEmailCore::setHours(int nHours)
{
	m_nHours = (uint) nHours;
	
	OnSettingsChanged();
}

void CGexEmailCore::setMinutes(int nMinutes)
{
	m_nMinutes = (uint) nMinutes;

	OnSettingsChanged();
}

void CGexEmailCore::setSeconds(int nSeconds)
{
	m_nSeconds = (uint) nSeconds;

	OnSettingsChanged();
}

void CGexEmailCore::setFullLog(int nFullLog)
{		
	m_bFullLog = (bool) nFullLog;

	OnSettingsChanged();
}

void CGexEmailCore::SetEmailDirectory(const QString& strDirectory)
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: SetEmailDirectory function.", false);
#endif

	m_strEmailDirectory = strDirectory;
	CGexSystemUtils::NormalizePath(m_strEmailDirectory);

	// Emit signal telling a new file has been selected
	emit sEmailDirectoryChanged(m_strEmailDirectory);
	
	OnSettingsChanged();
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
// Initialize window
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailCore::init(bool bStartEmailSpoolerAtLaunch, bool bAllowUserToSelectEmailSpoolingDir, bool bRunAsService, bool *pbExitApplication)
{ 
	*pbExitApplication = false;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	if(bRunAsService)
		Log(GEXEMAIL_MSG_INFO, "", "Debug: initializing Email window (service mode)", false);
	else
		Log(GEXEMAIL_MSG_INFO, "", "Debug: initializing Email window (GUI mode)", false);
#endif
	
	// Save some variables
	m_bRunningAsService = bRunAsService;
	m_bAllowUserToSelectEmailSpoolingDir = bAllowUserToSelectEmailSpoolingDir;
	
	// Read Settings file
	unsigned int uiPageToDisplay;
	int nStatus = LoadSettings(&uiPageToDisplay);

	m_bNeedSave = false;

	// Make sure relevant page is displayed on startup
	emit sMainSelectionChanged(uiPageToDisplay);

	switch(nStatus)
	{
		case 2:	// critical error
#if GEXEMAIL_DEBUGLOG
			// DEBUG Log
			Log(GEXEMAIL_MSG_INFO, "", "Debug: Critical error. Exit.", false);
#endif

			*pbExitApplication = true;
			return false;

		case 1:	// non critical error
#if GEXEMAIL_DEBUGLOG
			// DEBUG Log
			Log(GEXEMAIL_MSG_INFO, "", "Debug: Non Critical error. Display correct page.", false);
#endif

			return false;
	}

	if((m_bRunningAsService == true) || (bStartEmailSpoolerAtLaunch == true))
		OnStartSpooling();

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	if(*pbExitApplication)
		Log(GEXEMAIL_MSG_INFO, "", "Debug: Email window initialized (exit application).", false);
	else
		Log(GEXEMAIL_MSG_INFO, "", "Debug: Email window initialized (start application).", false);
#endif

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Pause execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::pause(void)
{ 
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: Pause function.", false);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
// Resume execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::resume(void)
{ 
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: Resume function.", false);
#endif

	if(m_bSpoolingTimerON == false)
		OnStartSpooling();
}

/////////////////////////////////////////////////////////////////////////////////////
// Start execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::start(void)
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: Start function.", false);
#endif

	if(m_bSpoolingTimerON == false)
		OnStartSpooling();
}

/////////////////////////////////////////////////////////////////////////////////////
// Stop execution
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::stop(void)
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: Stop function.", false);
#endif

	if(m_bSpoolingTimerON == true)
		OnStopSpooling();
}
	
/////////////////////////////////////////////////////////////////////////////////////
// Start spooling
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::OnStartSpooling()
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: OnStartSpooling function.", false);
#endif

	if(m_bSpoolingTimerON == false)
	{
		// Update log
		Info(iStartSpool);

		// Clean log files
		cleanLogFiles();

		// We need to start the timer and change button text
		EnableSpoolingTimer();

		emit sStartSpooling();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Stop spooling
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::OnStopSpooling()
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: OnStopSpooling function.", false);
#endif

	if(m_bSpoolingTimerON == true)
	{
		// Update log
		Info(iStopSpool);

		// We need to stop the timers and change button text
		StopMailFileTimer();
		DisableSpoolingTimer();

		emit sStopSpooling();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Resumes the spooling timer
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::ResumeSpoolingTimer(bool bInitInterval/* = true*/)
{	
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: ResumeSpoolingTimer function.", false);
#endif

	emit sSpoolingTimer(true);

	// Set next spooling time using settings
	if(bInitInterval)
		SetNextEmailSpooling(false);

	// First timer event should be ignored
	m_bFirstTimerEvent = true;
	
	// Resume timer
	m_bSpoolingInProgress = false;
}

/////////////////////////////////////////////////////////////////////////////////////
// Pauses the spooling timer
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::PauseSpoolingTimer()
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: PauseSpoolingTimer function.", false);
#endif

	emit sSpoolingTimer(false);

	m_bSpoolingInProgress = true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Enables the spooling timer: a tick every second
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::EnableSpoolingTimer()
{	
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: EnableSpoolingTimer function.", false);
#endif

	// Set next spooling time to run immediately
	SetNextEmailSpooling(true);

	// Don't ignore first timer event
	m_bFirstTimerEvent = false;

	// Enable timer
	m_bSpoolingTimerON = true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Disables the spooling timer
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::DisableSpoolingTimer()
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: DisableSpoolingTimer function.", false);
#endif

	m_bSpoolingTimerON = false;
}

/////////////////////////////////////////////////////////////////////////////////////
// Starts MailFile timer
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::StartMailFileTimer()
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: StartMailFileTimer function.", false);
#endif

	emit sStatusChanged(StatusCheckingMails);

	// Stop the timer
	PauseSpoolingTimer();

	// Set timer to give a tick asap
	m_bRetrieveListOfMailFiles = true;
	QTimer::singleShot(0, this, SLOT(OnCheckForMailFile()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Stops MailFile timer
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::StopMailFileTimer()
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: StopMailFileTimer function.", false);
#endif

	// Resume spooling timer
	ResumeSpoolingTimer();
}

/////////////////////////////////////////////////////////////////////////////////////
// Check if .mail file to be processed
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::OnCheckForMailFile()
{
	static QStringList	strMailFiles;
	QDir				dir;
	QString				strFileName;
	QString				strFullFileName;
	QStringList			strNameFilters("*.mail");

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: OnCheckForMailFile function.", false);
#endif

	// Get list of files only once
	if(m_bRetrieveListOfMailFiles == true)
	{
		// Get first .mail files if any
		dir.setPath(m_strEmailDirectory);
		dir.setFilter(QDir::Files | QDir::Hidden);
		strMailFiles = dir.entryList(strNameFilters);
		m_bRetrieveListOfMailFiles = false;
	}

	// Check if any file retrieved
	QStringList::iterator it = strMailFiles.begin();
	if(it == strMailFiles.end())
	{
		// No files, end of the mailing process
		StopMailFileTimer();

        int lDelay=-3600;
        char* lDelayEnvVar=getenv("GEXEMAIL_DELETE_FILE_DELAY");
        bool ok=false;
        QString(lDelayEnvVar).toInt(&ok);
        if (lDelayEnvVar && ok)
            lDelay=QString(lDelayEnvVar).toInt(&ok);
        // Check if any files to be deleted after 1 hour delay
        DeleteFiles_Later(lDelay); // 5975

        return;
	}

	// Get first file and process it
	strFileName = *it;
	strMailFiles.erase(it);
	strFullFileName = m_strEmailDirectory + "/" + strFileName;
	CGexSystemUtils::NormalizePath(strFullFileName);

	// Process mail file
	ProcessMailFile(strFileName, strFullFileName);

	// Check for further mail files
	QTimer::singleShot(0, this, SLOT(OnCheckForMailFile()));
}

/////////////////////////////////////////////////////////////////////////////////////
// Rename corrupted files
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::RenameCorruptedFile(const QString& strFullFileName)
{
	QDir				dir;
	QString				strNewName = strFullFileName + ".bad";
	QFile::Permissions	fPermissions = QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadUser | QFile::WriteUser | QFile::ExeUser | QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: RenameCorruptedFile function.", false);
#endif

	QFile::setPermissions(strFullFileName, fPermissions);
	QFile::setPermissions(strNewName, fPermissions);

	dir.rename(strFullFileName, strNewName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Remove list of files
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::RemoveFiles(const QStringList& strlistFiles)
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: RemoveFiles function.", false);
#endif

	// Delete Files
	for(QStringList::ConstIterator it = strlistFiles.begin(); it != strlistFiles.end(); ++it) 
		RemoveFile(*it);
}

/////////////////////////////////////////////////////////////////////////////////////
// Remove specified file
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::RemoveFile(const QString& strFullFileName)
{
	QDir				dir;
    QFile::Permissions	fPermissions
            = QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
            | QFile::ReadUser | QFile::WriteUser | QFile::ExeUser
            | QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: RemoveFile function.", false);
#endif

	QFile::setPermissions(strFullFileName, fPermissions);
	dir.remove(strFullFileName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Process 1 mail file
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailCore::ProcessMailFile(const QString& strFileName, const QString& strFullFileName)
{
	QFile	fileMail(strFullFileName);
	bool	bStatus;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: ProcessMailFile function.", false);
#endif

	// Current mail file
	m_strCurrentMailFile = strFileName;

	// emit new status
	emit sStatusChanged(StatusProcessingMailFile, m_strCurrentMailFile);

	// Open file
	if(fileMail.open(QIODevice::ReadOnly) == false)
	{
		RenameCorruptedFile(strFullFileName);
		Error(eOpenMailFile, m_strCurrentMailFile);
		return false;
	}

	// Initlist of files to remove on success
	m_strlistFilesToRemove.clear();
	m_strlistFilesToRemove.append(strFullFileName);
	
	QTextStream	streamMail(&fileMail);
	QString		strLine, strFieldName, strFieldValue;
    QString		strFrom, strSubject, strBody, strTo;
    QStringList	strlistTo, strlistAttachment, strlistAttachmentCopies, strlistAttachmentsToDeleteLater;
    int			nBodyFormat = GEXEMAIL_BODYFORMAT_TEXT;

	// Read all lines of the file
	strLine = streamMail.readLine();
	while(strLine.isNull() == false)
	{
		strFieldName = strLine.section('=', 0, 0);
		strFieldValue = strLine.section('=', 1);

		// from (ie From=Examinator-Monitoring)
		if((strFieldName.toLower() == "from") && (GetSender(strFieldValue, strFrom) == false))
		{
			fileMail.close();
			// Delete copied attachments
			RemoveFiles(strlistAttachmentCopies);
			// Remove corrupted email file
			RenameCorruptedFile(strFullFileName);
			return false;
		}

		// format (ie Format=HTML)
		if((strFieldName.toLower() == "format") && (strFieldValue.toLower() == "html"))
			nBodyFormat = GEXEMAIL_BODYFORMAT_HTML;

		// subject (ie Subject=Examinator autoreport: Reporting - keep)
		if(strFieldName.toLower() == "subject")
			strSubject = strFieldValue;

		// body (ie Body=C:/Temp/web_status/examinator_monitoring/.emails/20040430_ 172241_510.body)
		if((strFieldName.toLower() == "body") && (ReadBody(strFieldValue, strBody) == false))
		{
			fileMail.close();
			// Delete copied attachments
			RemoveFiles(strlistAttachmentCopies);
			// Remove corrupted email file
			RenameCorruptedFile(strFullFileName);
			return false;
		}

		// to (ie To=gex@galaxysemi.com)
		if((strFieldName.toLower() == "to") && (AddRecipient(strFieldValue, strlistTo, strTo) == false))
		{
			fileMail.close();
			// Delete copied attachments
			RemoveFiles(strlistAttachmentCopies);
			// Remove corrupted email file
			RenameCorruptedFile(strFullFileName);
			return false;
		}

		// attachement (ie Attachment=C:\PROGRA~1\GACA55~1/reports.mo/20040430_17_22_32/index.htm)
		if((strFieldName.toLower() == "attachment"))
		{
			QStringList lstAttachments = strFieldValue.split(QRegExp(",|;"));
			QStringList::iterator it;
			for (it = lstAttachments.begin(); it != lstAttachments.end(); ++it)
			{
				// If error
                if (AddAttachment((*it), strlistAttachment, strlistAttachmentCopies, strlistAttachmentsToDeleteLater) == false)
				{
					fileMail.close();
					// Delete copied attachments
					RemoveFiles(strlistAttachmentCopies);
					// Remove corrupted email file
					RenameCorruptedFile(strFullFileName);
					return false;
				}
			}
		}

		// Read next line
		strLine = streamMail.readLine();
	}

	// Close file
	fileMail.close();

	// Format body string: make sure that line feeds are coded using "\r\n"
	strBody.replace("\r\n", "\n");
	strBody.replace("\r", "\n");
	strBody.replace("\n", "\r\n");

	// Check if required fields have been read
	if(strFrom.isEmpty() == true)
	{
		Error(eNoSource, m_strCurrentMailFile);
		// Delete copied attachments
		RemoveFiles(strlistAttachmentCopies);
		// Remove corrupted email file
		RenameCorruptedFile(strFullFileName);
		return false;
	}
	if(strlistTo.isEmpty() == true)
	{
		Error(eNoRecipient, m_strCurrentMailFile);
		// Delete copied attachments
		RemoveFiles(strlistAttachmentCopies);
		// Remove corrupted email file
		RenameCorruptedFile(strFullFileName);
		return false;
	}
	
	// Send email
	QString strResponse;
	emit sStatusChanged(StatusSendingMail, m_strCurrentMailFile, strTo);
	bStatus = m_pEmailSystem->Send(strFrom, strlistTo, strSubject, strBody, strlistAttachment, strResponse, nBodyFormat);

	// Delete copied attachments
	RemoveFiles(strlistAttachmentCopies);

	// Check status
	if(bStatus == true)
	{
		// Update log file
		Info(iEmailSent, m_strCurrentMailFile, strTo);

		// Delete Files
		RemoveFiles(m_strlistFilesToRemove);

        // Add to list of files to be deleted later (after 1 hour delay)
        m_strlistFilesToRemove_Later += strlistAttachmentsToDeleteLater;
    }
	else
		// Update log file
		Error(eSend, m_strCurrentMailFile, strResponse);

	return bStatus;
}

/////////////////////////////////////////////////////////////////////////////////////
// Check correctness of email address. 
// Ex: toto.dupond@company.com
//	   Toto Dupond <toto.dupond@company.com>
//	   <toto.dupond@company.com> Toto Dupond
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailCore::IsValidEmailAddress(const QString& strEmailAddress)
{
	QString strAddressToCheck;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: IsValidEmailAddress function.", false);
#endif

	// Check if there is an address enclosed in <>
	if(strEmailAddress.startsWith("<"))
	{
		strAddressToCheck = strEmailAddress.right(strEmailAddress.length()-1);
		int nPos = strAddressToCheck.indexOf(">");
		if(nPos == -1)
			return false;
		strAddressToCheck = strAddressToCheck.left(nPos);
	}
	else if(strEmailAddress.endsWith(">"))
	{
		strAddressToCheck = strEmailAddress.left(strEmailAddress.length()-1);
		int nPos = strAddressToCheck.indexOf("<");
		if(nPos == -1)
			return false;
		strAddressToCheck = strAddressToCheck.right(strAddressToCheck.length()-nPos-1);
	}
	else
		strAddressToCheck = strEmailAddress;

	// There are several regular expressions for email addresses available on the Internet.
	// The regular expression used here is the following:
	// ^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$
	// The expression should be changed if experiencing some issues.
	QString				strRegExp = "(([^<>()[\\]\\\\.,;:\\s@\\\"]+(\\.[^<>()[\\]\\\\.,;:\\s@\\\"]+)*)|(\\\".+\\\"))@((\\[[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\])|(([a-zA-Z\\-0-9]+\\.)+[a-zA-Z]{2,}))";
	QRegExp				rx(strRegExp);
    QRegExpValidator	v( rx, 0 );

	int	nPos = 0;
	if(v.validate(strAddressToCheck, nPos) == QValidator::Acceptable)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////
// Add to list of attachments
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailCore::AddAttachment(QString& strFieldValue,
                                  QStringList& strlistAttachment,
                                  QStringList& strlistAttachmentCopies,
                                  QStringList& strlistAttachmentsToDeleteLater)
{
	QString strFile, strFileCopy;
	bool	bCopyAttachment = false;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: AddAttachment function.", false);
#endif

	// Check if the attachment should be renamed (form file1|file2)
	if(strFieldValue.indexOf("|") != -1)
	{
		strFile = strFieldValue.section('|', 0, 0);
		strFileCopy = strFieldValue.section('|', 1, 1);
		bCopyAttachment = true;
	}
	else
		strFile = strFieldValue;

	// Normalize file names
	CGexSystemUtils::NormalizePath(strFile);
	CGexSystemUtils::NormalizePath(strFileCopy);

	// Check if the attachment is a valid file
	QFile		fileAttachment(strFile);
	QByteArray	pString, pString2;
	if(fileAttachment.exists() == false)
	{
		Error(eOpenAttachment, m_strCurrentMailFile, strFile);
		return false;
	}

	// Check if attachment needs to be copied
	if(bCopyAttachment)
	{
		// Copy file
		pString = strFile.toLatin1();
		pString2 = strFileCopy.toLatin1();
		if(ut_CopyFile(pString.data(), pString2.data(), 0) == 0)
		{
			Error(eCopyAttachment, m_strCurrentMailFile, strFile);
			return false;
		}

		// Add file to list of attachments
		strlistAttachment.append(strFileCopy);

        // Update list of files to remove on success (later, with a delay)
        strlistAttachmentsToDeleteLater.append(strFile);

		// Update list of copied files
		strlistAttachmentCopies.append(strFileCopy);
	}
	else
	{
		// Add file to list of attachments
		strlistAttachment.append(strFile);

        // Update list of files to remove on success (later, with a delay)
        strlistAttachmentsToDeleteLater.append(strFile);
    }

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Validate sender email address
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailCore::GetSender(const QString& strFieldValue, QString& strFrom)
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: GetSender function.", false);
#endif

	if(IsValidEmailAddress(strFieldValue) == false)
	{
		Error(eInvalidSenderAddress, m_strCurrentMailFile, strFieldValue);
		return false;
	}

	strFrom = strFieldValue;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Add to list of recipients
// strFieldValue can be:
// * a single email address
// * a ';' separated list of email addresses
// * a file containing a mailing list (each line being a single email address or a ';' separated list of email addresses)
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailCore::AddRecipient(const QString& strFieldValue, QStringList& strlistTo, QString& strTo)
{
	// Check if the strFieldValue parameter is a mailing list file
	QFile	fileRecipient(strFieldValue);

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: AddRecipient function.", false);
#endif

	if(fileRecipient.exists() == false)
		// Add to recipient list
		return AddLineToRecipient(strFieldValue, strlistTo, strTo);

	// Open file
	if(fileRecipient.open(QIODevice::ReadOnly) == false)
	{
		Error(eOpenMaillistFile, m_strCurrentMailFile, strFieldValue);
		return false;
	}
	
	// Read each line and add to recipient list
	QTextStream	streamRecipient(&fileRecipient);
	QString		strLine = streamRecipient.readLine();
	while(strLine.isNull() == false)
	{
		if(AddLineToRecipient(strLine, strlistTo, strTo) == false)
			return false;
		strLine = streamRecipient.readLine();
	}

	// Close file
	fileRecipient.close();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Add to list of recipients
// strlistTo can be:
// * a single email address
// * a ';' or ',' separated list of email addresses
// * a name with a single email address or multiple email addresses in between <>
//   ie:
//   Sales <s1@foo.com>
//   Sales <s1@foo.com;s2@foo.com>
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailCore::AddLineToRecipient(const QString& strToLine, QStringList& strlistTo, QString& strTo)
{
	QString		strAddressToParse = strToLine;
	QString		strAddressName, strAddressSpec;
	int			nBegin, nEnd;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: AddLineToRecipient function.", false);
#endif

		// Remove useless whitespaces
	strAddressToParse = strAddressToParse.simplified();

	// Check if brackets present
	nBegin = strAddressToParse.indexOf('<');
	if(nBegin != -1)
	{
		// Get ending bracket
		nEnd = strAddressToParse.indexOf('>');
		if((nEnd == -1) || (nEnd <= nBegin+1))
		{
			Error(eInvalidRecipientAddress, m_strCurrentMailFile, strToLine);
			return false;
		}

		// Extract address spec inside brackets
		strAddressSpec = strAddressToParse.mid(nBegin+1, nEnd-nBegin-1).trimmed();
		if(strAddressSpec.isEmpty())
		{
			Error(eInvalidRecipientAddress, m_strCurrentMailFile, strToLine);
			return false;
		}

		// Extract address name (before or after the brackets)
		if(nBegin > 0)
			strAddressName = strAddressToParse.left(nBegin).trimmed();
		else if(nEnd < (int)(strAddressToParse.length()-1))
			strAddressName = strAddressToParse.mid(nEnd+1).trimmed();
	}
	else
		strAddressSpec = strAddressToParse;

	// Add all individual addresses to the list
	QStringList strlistToLine = strAddressSpec.split(";", QString::SkipEmptyParts);
	QString		strEmailAddress, strFullAddress;
	
	if(strlistToLine.count() == 1)
		// If no ';' separators found, try the ',' separator
		strlistToLine = strAddressSpec.split(",", QString::SkipEmptyParts);
	
	for(QStringList::Iterator it = strlistToLine.begin(); it != strlistToLine.end(); ++it) 
	{
		// Remove leading and ending white spaces
		strEmailAddress = (*it).trimmed();
		
		// Check if valid email address
		if(IsValidEmailAddress(strEmailAddress) == false)
		{
			Error(eInvalidRecipientAddress, m_strCurrentMailFile, strEmailAddress);
			return false;
		}

		// Append to list
        strFullAddress = strEmailAddress;
		if(!strAddressName.isEmpty())
		{
			strFullAddress = strAddressName + " <";
			strFullAddress += strEmailAddress;
			strFullAddress += ">";
		}
        strlistTo.append(strFullAddress);
		if(strTo.isEmpty() == false)
			strTo += ";";
		strTo += strEmailAddress;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Read Body file into string
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailCore::ReadBody(QString& strBodyFile, QString& strBody)
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: ReadBody function.", false);
#endif

	strBody = "";

	CGexSystemUtils::NormalizePath(strBodyFile);
	if(strBodyFile.isEmpty() == true)
		return true;

	QFile fileBody(strBodyFile);

	// Open file
	if(fileBody.open(QIODevice::ReadOnly) == false)
	{
		Error(eOpenBodyFile, m_strCurrentMailFile, strBodyFile);
		return false;
	}

	// Read all file into the string
	QTextStream streamBody(&fileBody);
	strBody = streamBody.readAll();

	// Close file
	fileBody.close();

	// Add file to list of files to remove on success
	m_strlistFilesToRemove.append(strBodyFile);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Called every second
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::OnSecTimerDone()
{
	if((m_bSpoolingTimerON == true) && (m_bSpoolingInProgress == false))
	{
		// Check if we need to cleanup log files
		if(m_pLogCleanupTime->elapsed() >= (int)m_uiLogCleanupInterval)
			cleanLogFiles();

		// Drop first timer event
		if(m_bFirstTimerEvent == true)
		{
			m_bFirstTimerEvent = false;
			return;
		}

		// Should we check for mail files?
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

		// Start timer to process mail files
		StartMailFileTimer();
	}
	if((m_bSpoolingTimerON == false) && (m_bSpoolingInProgress == false))
		emit sStatusChanged(StatusReady);
}

/////////////////////////////////////////////////////////////////////////////////////
// Log info
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::Info(int nInfoCode, const QString& strMailFile /*= ""*/, const QString& strArg /*= ""*/)
{
	QString strInfoMsg;

	switch(nInfoCode)
	{
		case iStartApp:
			strInfoMsg = "Starting " + m_strApplicationName;
			strInfoMsg += ".";
			Log(GEXEMAIL_MSG_INFO, strMailFile, strInfoMsg, false);
			break;
		case iStopApp:
			strInfoMsg = "Stopping " + m_strApplicationName;
			strInfoMsg += ".";
			Log(GEXEMAIL_MSG_INFO, strMailFile, strInfoMsg, false);
			break;
		case iStartSpool:
			strInfoMsg = "Starting spooler.";
			Log(GEXEMAIL_MSG_INFO, strMailFile, strInfoMsg, false);
			break;
		case iStopSpool:
			strInfoMsg = "Stopping spooler.";
			Log(GEXEMAIL_MSG_INFO, strMailFile, strInfoMsg, false);
			break;
		case iEmailSent:
			strInfoMsg = "Email sent to " + strArg;
			strInfoMsg += ".";
			Log(GEXEMAIL_MSG_INFO_SENT, strMailFile, strInfoMsg);
			break;
		case iLoadSettings:
			strInfoMsg = "Settings successfully loaded.";
			Log(GEXEMAIL_MSG_INFO, strMailFile, strInfoMsg);
			break;
		case iSaveSettings:
			strInfoMsg = "Settings successfully saved.";
			Log(GEXEMAIL_MSG_INFO, strMailFile, strInfoMsg);
			break;
		case iRemoveLogFile:
			strInfoMsg = "Log file deleted: " + strArg;
			strInfoMsg += ".";
			Log(GEXEMAIL_MSG_INFO, strMailFile, strInfoMsg);
			break;
		case iCleanLogFiles:
			strInfoMsg = "Cleaning log files older than " + strArg;
			strInfoMsg += ".";
			Log(GEXEMAIL_MSG_INFO, strMailFile, strInfoMsg);
			break;
		default:
			return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Log error
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::Error(int nErrorCode, const QString& strMailFile /*= ""*/, const QString& strArg /*= ""*/)
{
	QString strErrorMsg;

	switch(nErrorCode)
	{
		case eOpenMailFile:
			strErrorMsg = "Couldn't open mail file.";
			break;
		case eOpenBodyFile:
			strErrorMsg = "Couldn't open body file (" + strArg;
			strErrorMsg += ").";
			break;
		case eOpenMaillistFile:
			strErrorMsg = "Couldn't open maillist file (" + strArg;
			strErrorMsg += ").";
			break;
		case eOpenAttachment:
			strErrorMsg = "Couldn't open attachment (" + strArg;
			strErrorMsg += ").";
			break;
		case eCopyAttachment:
			strErrorMsg = "Couldn't copy attachment (" + strArg;
			strErrorMsg += ").";
			break;
		case eSend:
			strErrorMsg = "Error sending email (" + strArg;
			strErrorMsg += ").";
			break;
		case eNoSource:
			strErrorMsg = "No source address.";
			break;
		case eNoRecipient:
			strErrorMsg = "No recipient.";
			break;
		case eSaveSetttings:
			strErrorMsg = "Error saving settings to file " + strArg;
			strErrorMsg += ".";
			break;
		case eLoadSetttings:
			strErrorMsg = "Error reading settings from file " + strArg;
			strErrorMsg += ".";
			break;
		case eInvalidRecipientAddress:
			strErrorMsg = "Invalid recipient address " + strArg;
			strErrorMsg += ".";
			break;
		case eInvalidSenderAddress:
			strErrorMsg = "Invalid sender address " + strArg;
			strErrorMsg += ".";
			break;
		default:
			return;
	}
	Log(GEXEMAIL_MSG_ERROR, strMailFile, strErrorMsg);
}

/////////////////////////////////////////////////////////////////////////////////////
// Add to log file and log page
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::Log(int nMsgType,
                        const QString& strMailFile,
                        const QString& strMsg, bool bFilter /*=true*/)
{
	// Check if message should be logged
	if((bFilter == false) || (m_nLogFilter & nMsgType))
	{
		QString	strDateTime, strDate, strMsgType;

		// Date
		strDateTime = QDateTime::currentDateTime().toString();
		strDate = QDate::currentDate().toString(Qt::ISODate);

		sLogInfo(nMsgType, strMailFile, strMsg, strDateTime);

		switch(nMsgType)
		{
			case GEXEMAIL_MSG_INFO		:	strMsgType = "Info";
											break;

			case GEXEMAIL_MSG_INFO_SENT	:	strMsgType = "InfoSent";
											break;

			case GEXEMAIL_MSG_WARNING	:	strMsgType = "Warning";
											break;

			case GEXEMAIL_MSG_ERROR		:	strMsgType = "Error";
											break;

			default						:	break;
		}

		// Add to log file
		QString strLogFileName = m_strLocalFolder + "/gex_email_" + strDate;
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
				streamLog << "Mail file";
				streamLog << ",";
				streamLog << "Message";
				streamLog << endl;
			}
			streamLog << strDateTime;
			streamLog << ",";
			streamLog << strMsgType;
			streamLog << ",";
			streamLog << strMailFile;
			streamLog << ",";
			streamLog << strMsg;
			streamLog << endl;
			fileLog.close();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Save application settings
/////////////////////////////////////////////////////////////////////////////////////
bool CGexEmailCore::SaveSettings(void)
{
	FILE*		hSettingsFile;
	QByteArray	pString;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: SaveSettings function.", false);
#endif

	// Open settings file
	pString = m_strSettingsFile.toLatin1();
	hSettingsFile = fopen(pString.constData(), "w+t");
	if(!hSettingsFile)
	{
		Error(eSaveSetttings, "", m_strSettingsFile);
		return false;
	}

	// General section
	if(ut_WriteSectionToIniFile(hSettingsFile,"General") != UT_ERR_OKAY)
	{
		fclose(hSettingsFile);
		Error(eSaveSetttings, "", m_strSettingsFile);
		return false;
	}
	pString = QString::number(m_nHours).toLatin1(); // spinBoxHours->text().toLatin1();
	ut_WriteEntryToIniFile(hSettingsFile,"SpoolInterval_Hours",pString.data());
	pString = QString::number(m_nMinutes).toLatin1(); // spinBoxMinutes->text().toLatin1();
	ut_WriteEntryToIniFile(hSettingsFile,"SpoolInterval_Minutes",pString.data());
	pString = QString::number(m_nSeconds).toLatin1(); // spinBoxSeconds->text().toLatin1();
	ut_WriteEntryToIniFile(hSettingsFile,"SpoolInterval_Seconds",pString.data());
	pString = QString::number(m_uiLogCleanupInterval).toLatin1();
	ut_WriteEntryToIniFile(hSettingsFile,"LogCleanupInterval",pString.data());
	pString = QString::number(m_uiDaysToKeepLogFiles).toLatin1();
	ut_WriteEntryToIniFile(hSettingsFile,"DaysToKeepLogFiles",pString.data());
	pString = m_strEmailDirectory.toLatin1();
	ut_WriteEntryToIniFile(hSettingsFile,"EmailDirectory",pString.data());
	ut_WriteEntryToIniFile(hSettingsFile,"FullLog", m_bFullLog ? "1" : "0");
	fputs("\n",hSettingsFile);

	// Email system pages
	for(QList<CGexEmailSend*>::iterator it = m_listEmailSystems.begin(); it != m_listEmailSystems.end(); ++it)
	{
		if((*it)->SaveSettings(hSettingsFile) == false)
		{
			fclose(hSettingsFile);
			Error(eSaveSetttings, "", m_strSettingsFile);
			return false;
		}
	}

	// Update log
	Info(iSaveSettings);

	// Close settings file
	fclose(hSettingsFile);

	m_bNeedSave = false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Set default settings
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::DefaultSettings(void)
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: loading default settings", false);
#endif

	// General
	SetEmailDirectory(m_strLocalFolder + "/.emails");
	m_nHours	= 0;
	m_nMinutes	= 30;
	m_nSeconds	= 30;
	m_bFullLog	= false;
	m_uiDaysToKeepLogFiles	= 7;
	m_nLogFilter = GEXEMAIL_MSG_ERROR;
	
	// Email system pages
	for(QList<CGexEmailSend*>::iterator it = m_listEmailSystems.begin(); it != m_listEmailSystems.end(); ++it)
		(*it)->DefaultSettings();

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: default settings loaded", false);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
// Read application settings
// Return value:
//	0 = success
//	1 = failed reading non-critical settings
//	2 = failed reading critical settings
/////////////////////////////////////////////////////////////////////////////////////
int CGexEmailCore::LoadSettings(unsigned int* /*puiPageToDisplay*/)
{
	FILE*		hSettingsFile;
	int			iLastCheckedLine = 0;
	int			iBufferSize = 512;
	char		szString[iBufferSize];
	bool		bSuccess = true;
	QByteArray	pString;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	QString strDebugMessage;
	Log(GEXEMAIL_MSG_INFO, "", "Debug: loading settings", false);
#endif
	
	// First set default settings
	DefaultSettings();

	if(!m_bAllowUserToSelectEmailSpoolingDir && !m_bRunningAsService)
	// If not running as a service, get Email diretory from GEX-MO settings file
	// If running as a service, read the Email directory from the Email configuration file, because no user is
	// linked to the service, and we wouldn't be able to get the user home directory where teh GEX-MO settings file is located.
	{
#if GEXEMAIL_DEBUGLOG
		// DEBUG Log
		Log(GEXEMAIL_MSG_INFO, "", "Debug: getting email directory from Yield-Man", false);
#endif
		
		// Get Email directory from GEX-MO settings file
		QString strMonitoringConfFile;
		if(LoadEmailDirectoryFromGEX(strMonitoringConfFile) == false)
		{
#if GEXEMAIL_DEBUGLOG
			// DEBUG Log
			strDebugMessage.sprintf("Debug: ERROR getting email directory from Yield-Man config file %s", strMonitoringConfFile.toLatin1().constData());
			Log(GEXEMAIL_MSG_INFO, "", strDebugMessage, false);
#endif
			
			QString strMessage = "ERROR: Couldn't read Examinator Yield-Man configuration file ";
			strMessage += strMonitoringConfFile;
			strMessage += "\n\nPlease make sure you configured Examinator Yield-Man as specified in the Yield-Man online Help,";
			strMessage += "\nor the support section on our web site at http://www.mentor.com/";
			strMessage += "\n\nThe application will now exit!";

			emit sCriticalMessage(strMessage);
			return 2;
		}

#if GEXEMAIL_DEBUGLOG
		// DEBUG Log
		strDebugMessage.sprintf("Debug: email directory read from Yield-Man config file %s", strMonitoringConfFile.toLatin1().constData());
		Log(GEXEMAIL_MSG_INFO, "", strDebugMessage, false);
#endif
	}

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	strDebugMessage = "Debug: opening settings file " + m_strSettingsFile;
	Log(GEXEMAIL_MSG_INFO, "", strDebugMessage, false);
#endif
	
	// Open settings file
	pString = m_strSettingsFile.toLatin1();
	hSettingsFile = fopen(pString.constData(), "rt");
	if(!hSettingsFile)
	{
		if(m_bRunningAsService && !m_bAllowUserToSelectEmailSpoolingDir)
		{
#if GEXEMAIL_DEBUGLOG
			// DEBUG Log
			strDebugMessage = "Debug: ERROR1 opening settings file " + m_strSettingsFile;
			Log(GEXEMAIL_MSG_INFO, "", strDebugMessage, false);
#endif

			QString strMessage = "ERROR: Couldn't read GEX-MAIL configuration file ";
			strMessage += m_strSettingsFile;
			strMessage += "\n\nPlease make sure you configured GEX-MAIL before using it as a service.";
			strMessage += "\nTo do so:";
			strMessage += "\n\to launch GEX-MAIL using the Windows Start menu (Start->Programs->Galaxy Yield-Man->Email service)";
			strMessage += "\n\to configure all parameters in the 'Genaral' and 'Smtp' tabs";
			strMessage += "\n\to save your settings using the 'Save Settings' button";
			strMessage += "\n\nThe application will now exit!";

			emit sCriticalMessage(strMessage);
			return 2;
		}
		else
		{
#if GEXEMAIL_DEBUGLOG
			// DEBUG Log
			strDebugMessage = "Debug: ERROR2 opening settings file " + m_strSettingsFile;
			Log(GEXEMAIL_MSG_INFO, "", strDebugMessage, false);
#endif
			Error(eLoadSetttings, "", m_strSettingsFile);
			return 1;
		}
	}

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: settings file opened", false);
#endif
	
	// General
	if(ut_ReadFromIniFile(hSettingsFile,"General","SpoolInterval_Hours",szString,iBufferSize,&iLastCheckedLine,0) == UT_ERR_OKAY)
		m_nHours = atoi(szString);
	if(ut_ReadFromIniFile(hSettingsFile,"General","SpoolInterval_Minutes",szString,iBufferSize,&iLastCheckedLine,0) == UT_ERR_OKAY)
		m_nMinutes = atoi(szString);
	if(ut_ReadFromIniFile(hSettingsFile,"General","SpoolInterval_Seconds",szString,iBufferSize,&iLastCheckedLine,0) == UT_ERR_OKAY)
		m_nSeconds = atoi(szString);
	if(ut_ReadFromIniFile(hSettingsFile,"General","LogCleanupInterval",szString,iBufferSize,&iLastCheckedLine,0) == UT_ERR_OKAY)
		sscanf(szString, "%d", &m_uiLogCleanupInterval);
	if(ut_ReadFromIniFile(hSettingsFile,"General","DaysToKeepLogFiles",szString,iBufferSize,&iLastCheckedLine,0) == UT_ERR_OKAY)
		m_uiDaysToKeepLogFiles = atoi(szString);
	if(ut_ReadFromIniFile(hSettingsFile,"General","FullLog",szString,iBufferSize,&iLastCheckedLine,0) == UT_ERR_OKAY)
	{
		m_bFullLog = !strcmp(szString, "1");
		m_nLogFilter = GEXEMAIL_MSG_ALL;
	}
	else
		m_nLogFilter = GEXEMAIL_MSG_ERROR;
	if(m_bAllowUserToSelectEmailSpoolingDir)
	{
		if(ut_ReadFromIniFile(hSettingsFile,"General","EmailDirectory",szString,iBufferSize,&iLastCheckedLine,0) == UT_ERR_OKAY)
			SetEmailDirectory(szString);
	}
	else if(m_bRunningAsService)
	{
		if(ut_ReadFromIniFile(hSettingsFile,"General","EmailDirectory",szString,iBufferSize,&iLastCheckedLine,0) == UT_ERR_OKAY)
			SetEmailDirectory(szString);
		else
		{
#if GEXEMAIL_DEBUGLOG
			// DEBUG Log
			Log(GEXEMAIL_MSG_INFO, "", "Debug: ERROR reading email directory from settings file", false);
#endif
			
			QString strMessage = "ERROR: Couldn't read email spooling directory from GEX-MAIL configuration file ";
			strMessage += m_strSettingsFile;
			strMessage += "\n\nPlease make sure you configured GEX-MAIL before using it as a service.";
			strMessage += "\nTo do so:";
			strMessage += "\n\to launch GEX-MAIL using the Windows Start menu (Start->Programs->Galaxy Yield-Man->Email service)";
			strMessage += "\n\to configure all parameters in the 'Genaral' and 'Smtp' tabs";
			strMessage += "\n\to save your settings using the 'Save Settings' button";
			strMessage += "\n\nThe application will now exit!";

			emit sCriticalMessage(strMessage);
			return 2;
		}
	}

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: loading SMTP settings", false);
#endif
	
	// Email system pages
	for(QList<CGexEmailSend*>::iterator it = m_listEmailSystems.begin(); it != m_listEmailSystems.end(); ++it)
	{
		if((*it)->LoadSettings(hSettingsFile) == false)
		{
#if GEXEMAIL_DEBUGLOG
			// DEBUG Log
			Log(GEXEMAIL_MSG_INFO, "", "Debug: ERROR loading SMTP settings", false);
#endif
			bSuccess = false;
		}
	}

	// Close settings file
	fclose(hSettingsFile);

	// Check status
	if(bSuccess == false)
	{
#if GEXEMAIL_DEBUGLOG
		// DEBUG Log
		Log(GEXEMAIL_MSG_INFO, "", "Debug: ERROR loading settings", false);
#endif

		Error(eLoadSetttings, "", m_strSettingsFile);
		return 1;
	}

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: settings loaded", false);
#endif

	// Update log
	Info(iLoadSettings);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// Clean log files
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::cleanLogFiles()
{
	QStringList		strLogFiles;
	QDir			dir;
	QString			strFileName;
	QString			strFullFileName;
	QDateTime		limitDate;
	QString			strLimitDate;
	QStringList		strNameFilters("*.log");
	bool			bInfoDisplayed=false;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: CleanLogFiles function.", false);
#endif

	// Restart time for next log cleanup
	m_pLogCleanupTime->start();

	// Set limit date
    limitDate = QDateTime::currentDateTime().addDays(0-static_cast<int>(m_uiDaysToKeepLogFiles));
	strLimitDate = limitDate.toString();

	// Get list of log files
	dir.setPath(m_strLocalFolder);
	dir.setFilter(QDir::Files | QDir::Hidden);
	strLogFiles = dir.entryList(strNameFilters);

	// Check each retrieved file
	for(QStringList::iterator it = strLogFiles.begin(); it != strLogFiles.end(); ++it)
	{
		strFileName = *it;
		if(strFileName.startsWith("gex_email_"))
		{
			strFullFileName = m_strLocalFolder + "/" + strFileName;
			CGexSystemUtils::NormalizePath(strFullFileName);

			QFileInfo fileInfo(strFullFileName);
			if(fileInfo.lastModified() < limitDate)
			{
				if(!bInfoDisplayed)
				{
					// Update log
					Info(iCleanLogFiles, "", strLimitDate);
					bInfoDisplayed=true;
				}
				Info(iRemoveLogFile, "", strFileName);
				RemoveFile(strFullFileName);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Delete files after 1 hour delay
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::DeleteFiles_Later(const int &lNumSeconds)
{
    QString			strFileName;
    QDateTime		limitDate;

#if GEXEMAIL_DEBUGLOG
    // DEBUG Log
    Log(GEXEMAIL_MSG_INFO, "", "Debug: DeleteFiles_Later function.", false);
#endif

    // Set limit date
    limitDate = QDateTime::currentDateTime().addSecs(lNumSeconds); // -3600=1h ?

    // Check files in the list of files to be deleted later
    m_strlistFilesToRemove_Later.removeDuplicates();
    QStringList::iterator it = m_strlistFilesToRemove_Later.begin();
    while(it != m_strlistFilesToRemove_Later.end())
    {
        strFileName = *it;
        QFileInfo fileInfo(strFileName);
        if(fileInfo.lastModified() < limitDate)
        {
            RemoveFile(strFileName);
            it = m_strlistFilesToRemove_Later.erase(it);
        }
        else
            it++;
    }
}

///////////////////////////////////////////////////////////
// Gets email directory from GEX settings
///////////////////////////////////////////////////////////
bool CGexEmailCore::LoadEmailDirectoryFromGEX(QString & strMonitoringConfFile)
{
	// Build access path to the .INI file !
	int			iBufferSize = 1024;
	char		szString[iBufferSize];
	FILE*		hSettingsFile;
	QByteArray	pString;

#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: LoadEmailDirectoryFromGEX function.", false);
#endif

	// => Check if the new XML config file exists
	//		If not
	//			=> Check the old config file
	//			If not error
	// => Check if Task_Status exists
	//		If not
	//			=> Check if Gex_email entry exists
	//			If not error

	// Build name of configuration file
	strMonitoringConfFile = m_strLocalFolder + GEXMO_TASKS_XML_FILE;
	CGexSystemUtils::NormalizePath(strMonitoringConfFile);

	// Open settings file
	pString = strMonitoringConfFile.toLatin1();
	hSettingsFile = fopen(pString.constData(), "rt");
	if(!hSettingsFile)
	{
		// XML config file not exist
		// try with the old config file
		strMonitoringConfFile = m_strLocalFolder + GEXMO_TASKS_FILE;
		CGexSystemUtils::NormalizePath(strMonitoringConfFile);

		// Open settings file
		pString = strMonitoringConfFile.toLatin1();
		hSettingsFile = fopen(pString.constData(), "rt");
		if(!hSettingsFile)
		{
			Error(eLoadSetttings, "", strMonitoringConfFile);
			return false;
		}
	}

	QString strEmailDirectory;

	// Read settings
	if(ut_ReadFromXmlFile(hSettingsFile,"gex_email","EmailDirectory",szString,iBufferSize,0) != UT_ERR_OKAY)
	{
		if(ut_ReadFromXmlFile(hSettingsFile,"task_status","IntranetPath",szString,iBufferSize,0) != UT_ERR_OKAY)
		{
			fclose(hSettingsFile);
			Error(eLoadSetttings, "", strMonitoringConfFile);
			return false;
		}

		strEmailDirectory = szString;
		strEmailDirectory += "/";
		strEmailDirectory += GEXMO_AUTOREPORT_FOLDER;
		strEmailDirectory += "/";
		strEmailDirectory += GEXMO_AUTOREPORT_EMAILS;
	}
	else
		strEmailDirectory = szString;

	fclose(hSettingsFile);

	// Set email diretory
	CGexSystemUtils::NormalizePath(strEmailDirectory);

	SetEmailDirectory(strEmailDirectory);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Called when some Email settings have changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::OnSettingsChanged()
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: OnSettingsChanged function.", false);
#endif

	// Enable "Save settings" button
	m_bNeedSave = true;

	// Check 'Full log' settings
	if(m_bFullLog)
		m_nLogFilter = GEXEMAIL_MSG_ALL;
	else
		m_nLogFilter = GEXEMAIL_MSG_ERROR;

	emit sSettingsChanged();
}

/////////////////////////////////////////////////////////////////////////////////////
// Get seconds to next Mail spooling, or -1 if greater than threashhold
/////////////////////////////////////////////////////////////////////////////////////
int CGexEmailCore::GetSecondsToNextFtpSpooling()
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: GetSecondsToNextFtpSpooling function.", false);
#endif

	int nSecondsToNextFtpSpooling = QDateTime::currentDateTime().secsTo(m_dtNextCheck);
	if(nSecondsToNextFtpSpooling < 3600)
		return nSecondsToNextFtpSpooling;
	else
		return -1;
}

/////////////////////////////////////////////////////////////////////////////////////
// Calculate next Mail spooling time
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailCore::SetNextEmailSpooling(bool bRunNow)
{
#if GEXEMAIL_DEBUGLOG
	// DEBUG Log
	Log(GEXEMAIL_MSG_INFO, "", "Debug: SetNextEmailSpooling function.", false);
#endif

	QDateTime dtCurrent = QDateTime::currentDateTime();

	if(bRunNow)
		m_dtNextCheck = dtCurrent;
	else
		m_dtNextCheck = QDateTime::currentDateTime().addSecs(m_nHours*3600 + m_nMinutes*60 + m_nSeconds);
	
	m_nNextSpooling_Sec = GetSecondsToNextFtpSpooling();
}
