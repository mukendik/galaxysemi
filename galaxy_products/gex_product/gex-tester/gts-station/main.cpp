#include <QApplication>
#include <QMessageBox>
#include <gstdl_systeminfo.h>
#include <gqtl_log.h>

#ifdef unix
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <gstdl_utils_c.h>

#include "gts_station_mainwindow.h"
#include "gtl_core.h"


// Some global functions
bool GetApplicationPath(QString & strApplicationPath);									// Retrieve application path
void NormalizePath(QString& strPath);													// Normalize path (unix/windows)

// Main window
GtsStationMainwindow *pMainWindow;

// Application name
#if 0
#define GTS_STATION_APP				"Galaxy Tester Simulator V3.6"
const char* lAppFullName = GTS_STATION_APP;
#endif
// Application name
#define GETSTRING(x) #x
#define GETSTRING2(x) GETSTRING(x)
#define GTS_STATION_APP	"Galaxy Tester Simulator V" GETSTRING2(GTL_VERSION_MAJOR) "." GETSTRING2(GTL_VERSION_MINOR)
QString lAppFullName = GTS_STATION_APP;

int main( int argc, char ** argv )
{
    GSLOG(5, "Entering GTS main...");
    QString lApplicationPath;

    GSLOG(5, "Creating QApplication...");
    // Application object
    QApplication app( argc, argv );

    // Get application path
    if(!GetApplicationPath(lApplicationPath))
    {
        GSLOG(SYSLOG_SEV_EMERGENCY, "Error retrieving application path. The application will exit.");
		return 0;
    }

    GSLOG(5, "Creating main window...");
	// Create main window
    //lRunningMode = GtsStationMainwindow::Normal;
    pMainWindow = new GtsStationMainwindow(&app, lApplicationPath, lAppFullName);

    // Init GTS
    if(!pMainWindow->InitGts(argc, argv))
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Error parsing arguments. the application will exit");
        return EXIT_FAILURE;
    }

    // Show main window
    if(!pMainWindow->isHidden())
        pMainWindow->showMaximized();

    bool bStatus = true;
    if(pMainWindow->isAutoRun())
    {
        // GS_QA: simulate load button click to execute all runs
        bStatus = pMainWindow->RunQA();
        if(pMainWindow->isAutoClose())
            pMainWindow->OnButtonExit();
        else
            app.exec();
    }
    else
        // Execute application
        app.exec();

	// Delete main widget
	delete pMainWindow;

	// Return status
    return bStatus ? EXIT_SUCCESS : EXIT_FAILURE;
}

///////////////////////////////////////////////////////////
// Retrieve the application absolute path.
///////////////////////////////////////////////////////////
bool GetApplicationPath(QString & strApplicationPath)
{
#if defined __unix__ || __MACH__
    char *ptChar=0;

	// Check if GTS_PATH defined...if not ABORT!
	if((ptChar = getenv("GTS_PATH")) == NULL)
	{
        QMessageBox::critical(NULL, lAppFullName,
        "ERROR: The environment variable GTS_PATH is not set...\n"
        "you must set it to the path of the Galaxy Examinator application.\n"
        "The application will now exit!");
		return false;
	}
	strApplicationPath = ptChar;
#endif
#ifdef _WIN32
	// Under windows,
	char	szFullName[UT_MAX_PATH], szPath[UT_MAX_PATH], szFileName[UT_MAX_FNAME], szFileExt[UT_MAX_EXT];

	if(GetModuleFileNameA(NULL, szFullName, UT_MAX_PATH) == 0)
	{
        QMessageBox::critical( NULL, lAppFullName,"ERROR: The application path could not be retrieved...\nThe application will now exit!");
		return false;
	}
	ut_SplitPathName(szFullName, szPath, szFileName, szFileExt);
	strApplicationPath = szPath;
#endif

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Normalize path
// o unix: replace all '\' with '/'
// o windows: replace all '/' with \'
// o make sure there is no ending '/' or '\'
/////////////////////////////////////////////////////////////////////////////////////
void NormalizePath(QString& strPath)
{
#ifdef unix
	strPath.replace('\\', '/');
	if(strPath.right(1) == "/")
		strPath.truncate(strPath.length()-1);
#else
	strPath.replace('/', '\\');
	if(strPath.right(1) == "\\")
		strPath.truncate(strPath.length()-1);
#endif
}

