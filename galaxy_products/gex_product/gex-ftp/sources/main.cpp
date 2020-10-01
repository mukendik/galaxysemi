#include <QtCore>
#include <QtWidgets>
#include <iostream>
#ifdef unix
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <gstdl_utils_c.h>
#include <gqtl_sysutils.h>
#include <gqtl_log.h>
#include <gqtl_filelock.h>
#include <gex_version.h>
#include <gex_constants.h>
#include <gex_shared.h>

#include "gexftp_mainconsole.h"
#include "gexftp_mainwindow.h"
#include "gexftp_service.h"



// Gex Ftp Application name
#define GEX_APP_GEX_FTP				"GEX-FTP - " GEX_APP_VERSION
const char *szAppFullName			= GEX_APP_GEX_FTP;
const char *szServiceName			= "Examinator GEX-FTP";
const char *szServiceDescription	= "Examinator ftp client (visit http://www.mentor.com)";

// Some global functions
void ParseArguments(int argc, char **argv, bool *pbUseQtService, bool *pbExit);	// Parse arguments

bool	bStartFtpSpoolerAtLaunch	= false;		// Set to true if the spooler should be started at launch time
bool	bEnableGui					= true;
bool	bTraceModeON				= false;		// Set to true if trace mode activated
QString	gUserProfile, gLocalFolder;

// windows only: Used for Microsft Windows bug workaround( to avoid very long HTML page loading)
#if 0
#ifdef _WIN32
extern Q_EXPORT int qt_ntfs_permission_lookup;
#endif
#endif

///////////////////////////////////////////////////////////
// For debug purpose, append string to trace file
///////////////////////////////////////////////////////////
void WriteDebugMessageFile(const QString & strMessage)
{
	if(bTraceModeON)
	{
        FILE			*hFile=0;
		CGexSystemUtils	clSysUtils;
		QString			strTraceFile;

		// Construct file name
        strTraceFile = gLocalFolder + "/GalaxySemi/logs/" + QDate::currentDate().toString(Qt::ISODate);
		QDir d(strTraceFile);
		if (!d.exists(strTraceFile))
			if (!d.mkpath(strTraceFile))
				return;

        strTraceFile += QDir::separator() + QString("gex-ftp_trace_");
		strTraceFile += QDate::currentDate().toString(Qt::ISODate);
        strTraceFile += "_" + QString::number(QCoreApplication::applicationPid());
        strTraceFile += ".txt";
		clSysUtils.NormalizePath(strTraceFile);

		// Write message to file
        hFile = fopen(strTraceFile.toLatin1().constData(), "a");
		if(hFile == NULL)
			return;
		fprintf(hFile,"[%s %s] %s\n",QDate::currentDate().toString(Qt::ISODate).toLatin1().data(), QTime::currentTime().toString(Qt::ISODate).toLatin1().data(), strMessage.toLatin1().data());
		fclose(hFile);
	}
}

///////////////////////////////////////////////////////////
// Check if environment is ok
///////////////////////////////////////////////////////////
bool CheckGexFtpEnvironment(QString& strUserHome, QString& strApplicationDir, QString& strLocalFolder, QString& strError)
{
	QString			strLockFile;
	CGFileLock		clFileLock;
	QByteArray		pString;
	
	// Get user home directory
	if(CGexSystemUtils::CheckGexEnvironment(strUserHome, strApplicationDir, strError) == false)
	{
		strError += "Please contact Galaxy support at ";
		strError += GEX_EMAIL_SUPPORT;
		strError += " specifying your system type and OS.\n";
		strError += "GEX-FTP will now exit!";
		return false;
	}

	// Save user profile before eventually overloaded by env variable
    gUserProfile = strUserHome;

	// Server applications: check if env set to overwrite user profile directory
	QDir	clDir;
	char *	pszEnv = getenv("GEX_SERVER_PROFILE");
	if(pszEnv != NULL)
	{
		// Get env variable
		QString strFolder = pszEnv;
		CGexSystemUtils::NormalizePath(strFolder);

		// Check if dir exists or can be created
		if(QFile::exists(strFolder) == true)
			strUserHome = strFolder;
		else if(clDir.mkdir(strFolder))
			strUserHome = strFolder;
	}

#ifdef unix

        strLocalFolder = strUserHome + GEX_LOCAL_UNIXFOLDER;
	// Create .examinator folder to save all local info.
	QDir cDir;
	if(!cDir.exists(strLocalFolder))
		cDir.mkdir(strLocalFolder);
#else
	strLocalFolder = strUserHome;
#endif

    // Set global variable
    gLocalFolder = strLocalFolder;
    GSLOG(SYSLOG_SEV_NOTICE, QString("Local folder will be '%1'").arg(gLocalFolder).toLatin1().data() );

    // If in debug mode, write startup message (don't write any debug message before, as the debug file is created in the gLocalFolder)
    QDateTime	cCurrentDateTime	= QDateTime::currentDateTime();
    QString		strMessage			= "Starting ";
    strMessage += szAppFullName;
    strMessage += "...";
    WriteDebugMessageFile("---------------------------");
    WriteDebugMessageFile(strMessage);

    // Lock lock file
	strLockFile = strUserHome + "/.gex_ftp_lockfile.txt";
	CGexSystemUtils::NormalizePath(strLockFile);
	pString = strLockFile.toLatin1();
	clFileLock.SetLockFileName(pString.data());
	if(clFileLock.Lock() == 0)
	{
		// Couldn't lock lock file, an instance of the application is already running
		strError = "ERROR: An instance of the application is already running...\nThe application will now exit!";
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////
// Start GEX-FTP as application
///////////////////////////////////////////////////////////
int StartAsApplication(int argc, char **argv)	
{
	QString			strUserHome, strApplicationDir, strLocalFolder, strError;
	QApplication	ftpApplication(argc, argv, bEnableGui);
	int				nStatus = 0;

	if (CheckGexFtpEnvironment(strUserHome, strApplicationDir, strLocalFolder, strError))
	{
		CGexFtpMainBase *		pFtpMain			= NULL;
		QtServiceController *	pServiceController	= NULL;
		if (bEnableGui)
		{
			// Instanciate the service controller
			pServiceController = new QtServiceController(szServiceName);

			// Set GUI style
            //CGexSystemUtils::SetGuiStyle();
            pFtpMain		= new CGexFtp_mainwindow(&ftpApplication, strUserHome, strApplicationDir,
                                                     strLocalFolder, szAppFullName);
		}	
		else
			pFtpMain		= new CGexFtpMainConsole(strUserHome, strApplicationDir, strLocalFolder, szAppFullName);

		// Lock successful: continue...
		bool bExitApplication;
		if((pFtpMain->init(bStartFtpSpoolerAtLaunch, false, &bExitApplication, pServiceController) == false))
		{
			if(bExitApplication)
				return nStatus;
		}

		// Execute application
		nStatus = ftpApplication.exec();

		// Stop email window
		pFtpMain->stop();

		// Free the memory
		delete pFtpMain;

		if (pServiceController)
			delete pServiceController;
	}
	else
		QMessageBox::critical(NULL, szAppFullName, strError);
	
	return nStatus;
}

///////////////////////////////////////////////////////////
// Start GEX-FTP as service
///////////////////////////////////////////////////////////
int StartAsService(int argc, char **argv)	
{
	QtServiceBase * pService = NULL;

	if (bEnableGui)
		pService = new GEXFtpServiceGui(argc,argv, szServiceName, szServiceDescription);
	else
		pService = new GEXFtpServiceCore(argc,argv, szServiceName, szServiceDescription);
	
	// Start the application using QtService
	int nStatus = pService->exec();

	// Free the memory
	delete pService;

	return nStatus;
}

///////////////////////////////////////////////////////////
// Main()
int main(int argc, char **argv)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Entering in main: %1 args...").arg(argc).toLatin1().data() );
	bool	bUseQtService=false, bExit=false;
	int		nStatus=0;

#if 0
#ifdef _WIN32
	// Disables a NTFS lookup permission so QT (and our application) doesn't suffer from various Windows bugs.
	--qt_ntfs_permission_lookup;
#endif
#endif

	// First check argumants for compliance
	ParseArguments(argc, argv, &bUseQtService, &bExit);

	if(bExit)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Parse Arguments requested to exit...");
		return 0;
    }

		// Should the GUI be enabled?
#ifdef __unix__
	bEnableGui = !bUseQtService;
#endif

	// On Windows Vista, gui must not be enabled if gex-email is running as service
#ifdef _WIN32
	QSysInfo::WinVersion winVersion = QSysInfo::windowsVersion();

	if (winVersion == QSysInfo::WV_VISTA)
		bEnableGui = !bUseQtService;
#endif

	// Check if trace mode activated
	char *ptChar = getenv("GEX_FTP_TRACE");
	if(ptChar && !strcmp(ptChar, "1"))
		bTraceModeON = true;

	if(bUseQtService)
		nStatus = StartAsService(argc, argv);
	else
		nStatus = StartAsApplication(argc, argv);

	QString strMessage = "Stopping ";
	strMessage += szAppFullName;
	strMessage += ".";
	WriteDebugMessageFile(strMessage);
	WriteDebugMessageFile("---------------------------\n");

	return nStatus;
}

///////////////////////////////////////////////////////////
// Check argumants.
// Returns true if arguments correct, false else.
// *pbUseQtService is set to true if the QtService object should be used
// *pbExit is set to true if we should exit the application
///////////////////////////////////////////////////////////
void ParseArguments(int argc, char **argv, bool *pbUseQtService, bool *pbExit)
{
    QString lMessage;

	*pbUseQtService = *pbExit = false;

//#ifdef _WIN32
	// If no argument, the application is being launched by the service control manager
	if(argc <= 1)
	{
		*pbUseQtService = true;
		return;
	}

    QString lArg;
    if (argc > 1)
    {
        if (argc == 2)
        {
            lArg = argv[1];
        }
        else if (argc == 4) // to handle the new -style arg...
        {
            lArg = argv[3];
        }
		// First check if the argument is a service argument
        if(	lArg == "-i" || lArg == "-install" || lArg == "-u" || lArg == "-uninstall" ||
            lArg == "-e" || lArg == "-exec" || lArg == "-t" || lArg == "-terminate" ||
            lArg == "-p" || lArg == "-pause" || lArg == "-r" || lArg == "-resume")
		{
			*pbUseQtService = true;
			return;
		}

		// Not a service argument, check if custom argument
        if(lArg == "-gui")
		{
			return;
		}
        if(lArg == "-h" || lArg == "-help")
		{
			*pbExit = true;
            lMessage = "This application supports the following arguments:\n\n";
            lMessage += "-gui\t\t: execute the application in GUI mode (not as a service)\n";
            lMessage += "-i or -install\t: install the application as a service\n";
            lMessage += "-u or -uninstall\t: uninstall the application as a service\n";
            lMessage += "-e or -exec\t: execute the application as a service\n";
            lMessage += "-t or -terminate\t: terminate the application as a service\n";
            lMessage += "-p or -pause\t: pause the service\n";
            lMessage += "-r or -resume\t: resume the service\n";
            lMessage += "\nThe application will now exit!";
            std::cout << lMessage.toLatin1().data() << std::endl;
            GSLOG(SYSLOG_SEV_CRITICAL, lMessage.toLatin1().data() );
			return;
		}
	}

	// Found incorrect arguments: display critical error
	*pbExit = true;
    lMessage = "ERROR: unsupported argument. This application supports the following arguments:\n\n";
    lMessage += "-gui\t\t: execute the application in GUI mode (not as a service)\n";
    lMessage += "-i or -install\t: install the application as a service\n";
    lMessage += "-u or -uninstall\t: uninstall the application as a service\n";
    lMessage += "-e or -exec\t: execute the application as a service\n";
    lMessage += "-t or -terminate\t: terminate the application as a service\n";
    lMessage += "-p or -pause\t: pause the service\n";
    lMessage += "-r or -resume\t: resume the service\n";
    lMessage += "\nThe application will now exit!";
    std::cout << lMessage.toLatin1().data() << std::endl;
    GSLOG(SYSLOG_SEV_CRITICAL, lMessage.toLatin1().data() );
//#endif

	return;
}

