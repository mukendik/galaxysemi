///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
#include <QtGui>
#include <QtCore>

#include <gqtl_sysutils.h>
#include <gex_constants.h>
#include <gex_version.h>
#include <gex_shared.h>

#include "gexlm.h"
#include "gexlm_server.h"
#include "gexlm_mainobject.h"
#include "activation_serverkey.h"

#include "gexlm_service.h"


///////////////////////////////////////////////////////////
// Global functions
///////////////////////////////////////////////////////////
void	g_event(QString strEvent,QString strMessage, QDateTime dateTimeStamp = QDateTime::currentDateTime());	// Log message to log file.
QString g_normalize_text(QString &strParameter);
void	LogMessage(QString strMessage, bool bQWarning);
void	ParseArguments(int *pargc, char **argv, bool *pbUseQtService);
int		StartServer();
int		LicenseDialog();
void	WriteDebugMessageFile(const QString & strMessage);

///////////////////////////////////////////////////////////
// Global variables
///////////////////////////////////////////////////////////
#define GEX_APP_GEX_LM				"GEX-LM - " GEX_APP_VERSION
const char*		szAppFullName = GEX_APP_GEX_LM;
char			szServiceName[100];
char			szServiceDescription[100];
const char*		szDefaultServiceName = "Examinator GEX-LM";
const char*		szDefaultServiceDescription = "Examinator license manager (visit http://www.mentor.com)";
int				portnumber = 4242;								// Socket Port#
long			lPlatformRestriction=GEX_DATATYPE_ALLOWED_ANY;	// 0=Gex, 4=Gex-DB, etc...
bool			bEnableGUI=true;
bool			bLicenseOK = false;
bool			bDisabledSupportPerpetualLicense = false;
QApplication	*pclApplication=NULL;		// Ptr on QApplication object
QString			strApplicationDir;			// Application directory
QString			strUserHome;				// User Home directory
QString			strLocalFolder;				// Folder to store configuration files...
bool			bCustomerDebugMode = false;	// Set to true if application started with -D arg, and used to activate certain debug features without re-compiling

// from gexlm_server.cpp
extern int		iProductID;			// 0=Examinator, 1=Examinator for credence, ...,4=ExaminatorDB

// windows only: Used for Microsft Windows bug workaround( to avoid very long HTML page loading)
#if 0
#ifdef _WIN32
extern Q_EXPORT int qt_ntfs_permission_lookup;
#endif
#endif

///////////////////////////////////////////////////////////
// For debug purpose, append string to file
///////////////////////////////////////////////////////////
void WriteDebugMessageFile(const QString & strMessage)
{
#ifdef QT_DEBUG
  qDebug(strMessage.toLatin1().constData());
#endif
  if(bCustomerDebugMode)
  {
    FILE			*hFile;
    CGexSystemUtils	clSysUtils;
    QString			strTraceFile;

    // Construct file name
    strTraceFile = strLocalFolder + "/gexlm_trace_";
    strTraceFile += QDate::currentDate().toString(Qt::ISODate);
    strTraceFile += ".txt";
    clSysUtils.NormalizePath(strTraceFile);

    // Write message to file
    hFile = fopen(strTraceFile.toLatin1().constData() ,"a");
    if(hFile == NULL)
      return;
    fprintf(hFile,"[%s %s] %s\n",QDate::currentDate().toString(Qt::ISODate).toLatin1().constData(), QTime::currentTime().toString(Qt::ISODate).toLatin1().constData(), strMessage.toLatin1().constData());
    fclose(hFile);
  }
}

///////////////////////////////////////////////////////////
// Start license server
///////////////////////////////////////////////////////////
int StartServer()
{
  // Port used message
  QString strMessage= "Socket Port used: ";
  strMessage += QString::number(portnumber);
  LogMessage(strMessage, true);

  // Instanciate server object
  GexLicenseManager gexServer(portnumber);

  if(bEnableGUI)
    CGexSystemUtils::SetGuiStyle();

  // Start application
  return pclApplication->exec();
}

///////////////////////////////////////////////////////////
// License dialog
///////////////////////////////////////////////////////////
int LicenseDialog()
{
  // Show license dialog
  ActivationServerKeyDialog ActivationKeyDialog(strUserHome, strApplicationDir, 0);
  ActivationKeyDialog.show();

  return pclApplication->exec();
}

///////////////////////////////////////////////////////////
// Start license server as application
///////////////////////////////////////////////////////////
int StartServerAsApplication(int argc, char **argv)
{
  int iStatus = 0;

  // Instanciate application object
  QApplication clApplication(argc, argv, bEnableGUI);
  pclApplication = &clApplication;

  // A valid license was found, run the application as service or as standard application
  if (bLicenseOK)
    iStatus = StartServer();
  else
  {
    QString strMessage;
    if(bDisabledSupportPerpetualLicense)
      strMessage = "Your maintenance contract has expired. You can't run this recent release.\nContact the Galaxy sales team to upgrade your contract.";
    else
      strMessage = "The license file was not found or is invalid. Please contact "+QString(GEX_EMAIL_SALES)+"\n";

    // Log message
    LogMessage(strMessage, true);
    iStatus = 0;

    // if the GUI is enabled, display the activation key dialog
    if(bEnableGUI)
    {
      QMessageBox::information(NULL,szAppFullName,strMessage);
      iStatus = LicenseDialog();
    }
  }

  return iStatus;
}

///////////////////////////////////////////////////////////
// Start license server as Service
///////////////////////////////////////////////////////////
int StartServerAsService(int argc, char **argv)
{
  QtServiceBase * pGexService = NULL;

  // Instanciate the service object
  if (bEnableGUI)
    pGexService = new GEXLMServiceGui(argc, argv);
  else
    pGexService = new GEXLMServiceCore(argc, argv);

  int iStatus = pGexService->exec();

#ifdef _WIN32
  // Service has not be able to start, try to launch it in a standard mode
  if(iStatus == -4)
    iStatus = StartServerAsApplication(argc, argv);
#endif

  // delete the service object
  delete pGexService;

  return iStatus;
}

///////////////////////////////////////////////////////////
// Main()
///////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
  int		iStatus;
  bool	bUseQtService=true;
  char	*ptChar;
  QString	strEnv;

#if 0
#ifdef _WIN32
  // Disables a NTFS lookup permission so QT (and our application) doesn't suffer from various Windows bugs.
  --qt_ntfs_permission_lookup;
#endif
#endif

  // Get user home directory
  QString strError;
  if(CGexSystemUtils::CheckGexEnvironment(strUserHome, strApplicationDir, strError) == false)
  {
    strError += "Please contact Galaxy support at ";
    strError += GEX_EMAIL_SUPPORT;
    strError += " specifying your system type and OS.\n";
    strError += "GEX-LM will now exit!";
    LogMessage(strError, true);
    // Exit!
    exit(0);
  }

  QString strOriginalUserHome = strUserHome;

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

  // Check if port specified in env var
  ptChar = getenv("GEXLM_TRACE");
  if(ptChar != NULL)
  {
    strEnv = QString(ptChar).toLower();
    if((strEnv == "1") || (strEnv == "true"))
      bCustomerDebugMode = true;
  }

#ifdef __unix__
  strLocalFolder = strUserHome + GEX_LOCAL_UNIXFOLDER;
  CGexSystemUtils::NormalizePath(strLocalFolder);

  // Create .examinator folder to save all local info.
  QDir cDir;
  if(!cDir.exists(strLocalFolder))
    cDir.mkdir(strLocalFolder);
#else
  strLocalFolder = strUserHome;
#endif

  // Check for -D debug argument
  for(int iIndex = 1; iIndex < argc; iIndex++)
  {
    // Check for: Customer debug mode
    if((strcmp(argv[iIndex], "-D") == 0) || (strcmp(argv[iIndex], "-d") == 0))
      bCustomerDebugMode = true;
  }

  if(bCustomerDebugMode)
    WriteDebugMessageFile("Debug mode was activated ");
  if(strOriginalUserHome != strUserHome)
    WriteDebugMessageFile("System UserHome folder = " + strOriginalUserHome);
  WriteDebugMessageFile("UserHome folder = " + strUserHome);
  WriteDebugMessageFile("Local folder = " + strLocalFolder);

  // Check the time stamp from the configuration file when starting
  // Add a g_close_abnormal message if there has been a problem when previous session was finished
  GexLicenseManager::checkTimeStamp();

  // Log: License manager starting.
  g_event("g_start",szAppFullName);

  // Init service name and description
  strcpy(szServiceName, szDefaultServiceName);
  strcpy(szServiceDescription, szDefaultServiceDescription);

  // Load license file
  LicenseFile lic(strUserHome, strApplicationDir);
  bLicenseOK = lic.IsCorrectLicenseFile();

  WriteDebugMessageFile("Checking for GEXLM_PORT environment variable...");
  // Check if port specified in env var
  ptChar = getenv("GEXLM_PORT");
  if(ptChar != NULL)
  {
    int nTmpPortNb;
    // Get port nb
    if(sscanf(ptChar,"%d",&nTmpPortNb) == 1)
    {
      portnumber = nTmpPortNb;
      WriteDebugMessageFile("Read GEXLM_PORT environment variable = " + QString::number(portnumber));
    }
  }

  // Allow to define a second port (eg: if two license managers must run on same server)
  // If a specific port is specified from Examinator-Pro (GEXLM_PORT_PROD), and the license is
  // for Examinator Pro, rename the service name and description, so that 2 services can be installed
  // side by side.
  // !!!! IMPORTANT !!!!
  // gex-lm must have been started manually for both packages to activate and install the license file.
  // !!!! IMPORTANT !!!!
  if(bLicenseOK)
  {
    char	*ptChar=NULL;
    int		nTmpPortNb;
    switch(iProductID)
    {
      case GEX_DATATYPE_ALLOWED_DATABASE:	// Examinator-Pro: Allow any files from database.
        WriteDebugMessageFile("Checking for GEXLM_PORT_PROD environment variable...");
        ptChar = getenv("GEXLM_PORT_PROD");
        if(ptChar == NULL)
          break;

        // Get port nb
        if(sscanf(ptChar,"%d",&nTmpPortNb) == 1)
        {
          portnumber = nTmpPortNb;
          strcat(szServiceName, " [PROD]");
          strcat(szServiceDescription, " [PROD]");
          WriteDebugMessageFile("Read GEXLM_PORT_PROD environment variable = " + QString::number(portnumber));
        }
        break;
    }
  }

  // Parse arguments
  WriteDebugMessageFile("Parsing arguments...");
  ParseArguments(&argc, argv, &bUseQtService);

  // Should the GUI be enabled?
    #ifdef __unix__
      bEnableGUI = !bUseQtService;
    #endif

  // On Windows Vista, gui must not be enabled if gex-email is running as service
    #ifdef _WIN32
      QSysInfo::WinVersion winVersion = QSysInfo::windowsVersion();

      if (winVersion == QSysInfo::WV_VISTA)
        bEnableGUI = !bUseQtService;
    #endif

  // Startup Message
  QString strMessage= "Starting GEX License Manager: ";
  strMessage += szAppFullName;
  strMessage += " (port " + QString::number(portnumber) + ")";
  LogMessage(strMessage, true);

  // Check if we should use QtService class
  if(bUseQtService)
    iStatus = StartServerAsService(argc, argv);
  else
    iStatus = StartServerAsApplication(argc, argv);

  // Exit message.
  strMessage= "Closing GEX License Manager: ";
  strMessage += szAppFullName;
  LogMessage(strMessage, true);

  // Log: License manager starting.
  g_event("g_stop",szAppFullName);

  // write a null time stamp in the config file
  GexLicenseManager::writeTimeStamp(QDateTime());

  return iStatus;
}

///////////////////////////////////////////////////////////
// Check argumants.
// *pbUseQtService is set to true if the QtService object should be used
///////////////////////////////////////////////////////////
void ParseArguments(int *pargc, char **argv, bool *pbUseQtService)
{
  QString strArg;

  *pbUseQtService = true;

  // No arguments?
  if(*pargc <= 1)
  {
    // Windows: if no argument, the application is being launched:
    // o by the user by typing gex-lm
    // o by the user by typing gex-lm -e
    // o by the service control manager

                return;
        }

  // 1 argument
  if(*pargc == 2)
  {
    // Check argument
    strArg = argv[1];

                if(strArg == "-i" || strArg == "-install" || strArg == "-u" || strArg == "-uninstall" ||
                    strArg == "-e" || strArg == "-exec" || strArg == "-t" || strArg == "-terminate" )
    {
      return;
    }

    if((strArg == "-GEXDB") || (strArg == "-GEXPRO"))
    {
      *pbUseQtService = false;
      lPlatformRestriction = GEX_DATATYPE_ALLOWED_DATABASE;
      return;
    }
  }

  // We are not in service mode: check if port nb specified
  *pbUseQtService = false;
  if(*pargc == 3)
  {
    int nTmpPortNb;
    strArg = argv[1];
    if((strArg == "-port") && (sscanf(argv[3],"%d",&nTmpPortNb) == 1))
      portnumber = nTmpPortNb;
  }

  return;
}

///////////////////////////////////////////////////////////
// Log event message (used for License manager statistics)
///////////////////////////////////////////////////////////
void g_event(QString strEvent,QString strMessage, QDateTime dateTimeStamp)
{
  // Create log file name: <user_folder>//gex-lm_<year>_<month>.log
  QString		strFile = strLocalFolder + "/gex-lm_" + dateTimeStamp.toString("yyyy_MM") + ".log";

  // Open file
  QFile fLogFile(strFile);
    if(fLogFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text) == false)
    return;
  QTextStream hLogFile(&fLogFile);

  // Write event string: <event> <time-stamp dd-mmm-yy_hh:mm:ss> <strMessage>
  hLogFile << strEvent << dateTimeStamp.toString(" dd-MM-yyyy_hh:mm:ss ") << strMessage << endl;
  fLogFile.close();

  // Cleanup: delete log files older than 1 year (13 to 24 months old)
  dateTimeStamp.addMonths(-13);
  int	iMonth;
  for(iMonth= 0; iMonth < 12; iMonth++)
  {
    dateTimeStamp.addMonths(-1);
    strFile = strLocalFolder + dateTimeStamp.toString("/gex-lm_yyyy_MM") + ".log";
    QFile::remove(strFile);
  }
}

///////////////////////////////////////////////////////////
// Format parameter to be log-complient (no spaces)
///////////////////////////////////////////////////////////
QString g_normalize_text(QString &strParameter)
{
  QRegExp cRegExp(" \t");
  return strParameter.replace(cRegExp," ");
}

///////////////////////////////////////////////////////////
// Writes message to log file + display popup (if requested)
///////////////////////////////////////////////////////////
void LogMessage(QString strMessage, bool /*bQWarning*/)
{
  // Check if message should be displayed in debug window (or error stream under unix)
#if 0
  if(bQWarning)
    qWarning(strMessage.toLatin1().constData());
#endif

  // Write message to event log file
  g_event("g_message",strMessage);
}

