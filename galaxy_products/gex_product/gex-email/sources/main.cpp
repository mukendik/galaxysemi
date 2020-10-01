
#include <QtCore>


    #include <QtWidgets>


#include <gqtl_sysutils.h>
#include <gqtl_filelock.h>
#include <gqtl_log.h>
#include <gex_constants.h>
#include <gex_version.h>
#include <gex_shared.h>

#include "gex_email_mainwindow.h"
#include "gex_email_mainconsole.h"

#include "gex_email_service.h"


// Gex Email Dispatcher Application name
#define GEX_APP_GEX_EMAIL				"GEX-MAIL - " GEX_APP_VERSION
char szAppFullName[100];
const char *szDefaultAppFullName	= GEX_APP_GEX_EMAIL;
const char *szServiceName			= "Examinator GEX-MAIL";
const char *szServiceDescription	= "Examinator email dispatcher (visit http://www.mentor.com)";

// Some global functions
void ParseArguments(int argc, char **argv, bool *pbUseQtService, bool *pbExit);	// Parse arguments

// Set to true if the application can run with GUI
bool bEnableGUI = true;

// Set to true if we allow the user to select the email spooling directory (used when using gex-email with licman)
bool bAllowUserToSelectEmailSpoolingDir = false;

// Set to true if the spooler should be started at launch time
bool bStartEmailSpoolerAtLaunch = false;

#if 0
// windows only: Used for Microsft Windows bug workaround( to avoid very long HTML page loading)
#ifdef _WIN32
extern Q_EXPORT int qt_ntfs_permission_lookup;
#endif
#endif

#define GEXEMAIL_DEBUGLOG 1

///////////////////////////////////////////////////////////
// Check if environment is ok
///////////////////////////////////////////////////////////
bool CheckGexEmailEnvironment(QString& strUserHome, QString& strApplicationDir, QString& strLocalFolder, QString& strError)
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
		strError += "GEX-EMAIL will now exit!";
		return false;
	}

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

#ifdef __unix__
    strLocalFolder = strUserHome;   //+ GEX_LOCAL_UNIXFOLDER;  // case 5470
	CGexSystemUtils::NormalizePath(strLocalFolder);

	// Create .examinator folder to save all local info.
	QDir cDir;
	if(!cDir.exists(strLocalFolder))
		cDir.mkdir(strLocalFolder);
#else
	strLocalFolder = strUserHome;
#endif

	// Lock lock file
	strLockFile = strLocalFolder + "/.gex_email_lockfile.txt";
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
// Start GEX-EMAIL as application
///////////////////////////////////////////////////////////
int StartAsApplication(int argc, char **argv)	
{
	QString			strUserHome, strApplicationDir, strLocalFolder, strError;
	QApplication	emailApplication(argc, argv, bEnableGUI);
	int				nStatus = 0;

	if (CheckGexEmailEnvironment(strUserHome, strApplicationDir, strLocalFolder, strError))
	{
		CGexEmailMainBase * pEmailMain		= NULL;
		
		if (bEnableGUI)
		{
			// Set GUI style
            CGexSystemUtils::SetGuiStyle();

			pEmailMain		= new CGexEmailMainwindow(&emailApplication, strUserHome, strApplicationDir, strLocalFolder, szAppFullName);
		}	
		else
			pEmailMain		= new CGexEmailMainConsole(strUserHome, strApplicationDir, strLocalFolder, szAppFullName);

		// Lock successful: continue...
		bool bExitApplication;
		if((pEmailMain->init(bStartEmailSpoolerAtLaunch, bAllowUserToSelectEmailSpoolingDir, false, &bExitApplication) == false))
		{
			if(bExitApplication)
				return nStatus;
		}

		// Execute application
		nStatus = emailApplication.exec();

		// Stop email window
		pEmailMain->stop();

		// Free the memory
		delete pEmailMain;
	}
	else
		QMessageBox::critical(NULL, szAppFullName, strError);
	
	return nStatus;
}

///////////////////////////////////////////////////////////
// Start GEX-EMAIL as service
///////////////////////////////////////////////////////////
int StartAsService(int argc, char **argv)	
{
	QtServiceBase * pService = NULL;

	if (bEnableGUI)
		pService = new GEXEMailServiceGui(argc,argv, szServiceName, szServiceDescription);
	else
		pService = new GEXEMailServiceCore(argc,argv, szServiceName, szServiceDescription);
	
	// Start the application using QtService
	int nStatus = pService->exec();

	// Free the memory
	delete pService;

	return nStatus;
}

///////////////////////////////////////////////////////////
// Main()
///////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	bool	bUseQtService=false, bExit=false;
	int		nStatus=0;

    GSLOG(5, "main ");

#if 0
#ifdef _WIN32
	// Disables a NTFS lookup permission so QT (and our application) doesn't suffer from various Windows bugs.
	--qt_ntfs_permission_lookup;
#endif
#endif
	
	// Init variables
	strcpy(szAppFullName, szDefaultAppFullName);

	// First check argumants for compliance
//#ifdef _WIN32
	ParseArguments(argc, argv, &bUseQtService, &bExit);
//#endif
	
    GSLOG(5, szAppFullName);

	if(bExit)
		return 0;

	// Should the GUI be enabled?
#ifdef __unix__
    if (argc == 1)
    {
        bUseQtService = true;
        bEnableGUI = false;
        bStartEmailSpoolerAtLaunch = true;
    }
#endif

	// On Windows Vista, gui must not be enabled if gex-email is running as service
#ifdef _WIN32
	QSysInfo::WinVersion winVersion = QSysInfo::windowsVersion();

	if (winVersion == QSysInfo::WV_VISTA)
		bEnableGUI = !bUseQtService;
#endif

	// Check if we should use QtService class
	if(bUseQtService)
		nStatus = StartAsService(argc, argv);
	else
		nStatus = StartAsApplication(argc, argv);
	
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
	QString strMessage;

	*pbUseQtService = *pbExit = false;

#if defined unix || __MACH__
	// If no argument, just return
	if(argc <= 1)
		return;

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
        if(lArg == "-start")
        {
            bStartEmailSpoolerAtLaunch = true;
            return;
        }

        if(lArg == "-h" || lArg == "-help")
        {
            *pbExit = true;
            strMessage = "This application supports the following arguments:\n\n";
            strMessage += "-start\t\t: start the email spooler at launch\n";
            strMessage += "\nThe application will now exit!";
            QMessageBox::information(NULL, szAppFullName,strMessage);
            return;
        }

        // Not a service argument, check if custom argument
        if(lArg == "-gui")
        {
            return;
        }
    }

	// Found incorrect arguments: display critical error
	*pbExit = true;
	strMessage = "ERROR: unsupported argument. This application supports the following arguments:\n\n";
	strMessage += "-start\t\t: start the email spooler at launch\n";
	strMessage += "\nThe application will now exit!";
	QMessageBox::critical(NULL, szAppFullName,strMessage);
#endif

#ifdef _WIN32
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
        if(lArg == "-licman")
        {
            strcat(szAppFullName, " (licman)");
            bAllowUserToSelectEmailSpoolingDir = true;
            bStartEmailSpoolerAtLaunch = true;
            return;
        }

        if(lArg == "-h" || lArg == "-help")
        {
            *pbExit = true;
            strMessage = "This application supports the following arguments:\n\n";
            strMessage += "-gui\t\t: execute the application in GUI mode (not as a service)\n";
            strMessage += "-i or -install\t: install the application as a service\n";
            strMessage += "-u or -uninstall\t: uninstall the application as a service\n";
            strMessage += "-e or -exec\t: execute the application as a service\n";
            strMessage += "-t or -terminate\t: terminate the application as a service\n";
            strMessage += "-p or -pause\t: pause the service\n";
            strMessage += "-r or -resume\t: resume the service\n";
            strMessage += "\nThe application will now exit!";
            QMessageBox::information(NULL, szAppFullName,strMessage);
            return;
        }
	}

	// Found incorrect arguments: display critical error
	*pbExit = true;
	strMessage = "ERROR: unsupported argument. This application supports the following arguments:\n\n";
	strMessage += "-gui\t\t: execute the application in GUI mode (not as a service)\n";
	strMessage += "-i or -install\t: install the application as a service\n";
	strMessage += "-u or -uninstall\t: uninstall the application as a service\n";
	strMessage += "-e or -exec\t: execute the application as a service\n";
	strMessage += "-t or -terminate\t: terminate the application as a service\n";
	strMessage += "-p or -pause\t: pause the service\n";
	strMessage += "-r or -resume\t: resume the service\n";
	strMessage += "\nThe application will now exit!";
	QMessageBox::critical(NULL, szAppFullName,strMessage);
#endif
	
	return;
}

