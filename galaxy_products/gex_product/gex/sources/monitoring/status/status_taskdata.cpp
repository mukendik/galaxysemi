#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "status_taskdata.h"
#include "scheduler_engine.h"
#include "status/status_task.h"
#include "yield/yield_task.h"
#include "yield/yield_taskdata.h"
#include "gexmo_constants.h"
#include "db_engine.h"
#include "gex_database_entry.h"
#include "engine.h"
#include "product_info.h"
#include "message.h"

#ifdef WIN32
#undef CopyFile
#endif

// main.cpp
extern void				WriteDebugMessageFile(const QString & strMessage);
extern GexMainwindow *	pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// Images used to draw charts
#define GEXMO_RED_BAR	"bar2.png"		// FAIL
#define GEXMO_GREEN_BAR	"bar1.png"		// PASS
// Background color if PASS or FAIL
#define GEXMO_FAIL_YIELD	"#ff0000"		// FAIL
#define GEXMO_PASS_YIELD	"#F8F8F8"		// PASS

// Function to compare two lots by product/time
bool sortByProduct(const CGexMoLotsProcessed * pLot1, const CGexMoLotsProcessed * pLot2)
{
    // Sorting done over ProductName & date
    if(pLot1->strProductID < pLot2->strProductID)
        return true;

    if(pLot1->strProductID == pLot2->strProductID)
    {
        if(pLot1->tStartTime < pLot2->tStartTime)
            return true;
    }

    return false;
}

// Function to compare two lots by tester name/time
bool sortByTester(const CGexMoLotsProcessed * pLot1, const CGexMoLotsProcessed * pLot2)
{
    // Sorting done over ProductName & date
    if(pLot1->strNodeName < pLot2->strNodeName)
        return true;

    if(pLot1->strNodeName == pLot2->strNodeName)
    {
        if(pLot1->tStartTime < pLot2->tStartTime)
            return true;
    }

    return false;
}

namespace GS
{
namespace Gex
{
void SchedulerEngine::ExecuteStatusTask(void)
{
    CGexMoTaskStatus *ptTask = GetStatusTask();

    // If 'Status' task not enabled, quietly return.
    if(ptTask == NULL)
        return;

    // Check if the task can be executed
    if(!ptTask->GetEnabledState())
        return;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Execute status task...");

    // Read config file so to know if need to update report or not...
    if(ReadConfigFile() != true)
        return;

    // Execute task: have Report updated if needed.
    GexMoStatusTaskData pTaskStatus(ptTask->parent());
    onStartTask(ptTask);
    pTaskStatus.ExecuteStatusTask(ptTask,&mFilesToRename);
    onStopTask(ptTask);

    // If we have one or more files to rename, do it!
    OnCheckRenameFiles();
}

bool SchedulerEngine::ReadConfigFile(void)
{
    QString		strMonitoringPath;
    QString		strString;

    // Build path to the Config list.
    strMonitoringPath = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    strMonitoringPath += GEX_DATABASE_FOLDER;
    strMonitoringPath += GEXMO_STATUS_FOLDER;
    strMonitoringPath += GEXMO_STATUS_FILE;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Reading Config File %1...").arg( strMonitoringPath).toLatin1().constData());
    QFile file(strMonitoringPath); // Read the config file

    // If first execution ever, file doesn't exist, then we definitely need to update the Web pages!
    if (file.exists() == false)
    {
        mLastDataReceived = 1;
        return true;
    }

    if (file.open(QIODevice::ReadOnly) == false)
        return false;	// Failed.

    // Read config File
    QTextStream hStatus(&file);	// Assign file handle to data stream

    // Check if valid header...or empty!
    strString = hStatus.readLine();
    if(strString != "<monitoring_status>")
        return false;

    // Read all config file...
    do
    {
        // Read one line from file
        strString = hStatus.readLine();

        // Time of last data file received.
        if(strString.startsWith("LastDataReceived=") == true)
        {
            strString = strString.section('=',1);
            mLastDataReceived = strString.toLong();
        }
    }
    while(hStatus.atEnd() == false);
    file.close();
    return true;
}

///////////////////////////////////////////////////////////
// Write Monitoring config file
///////////////////////////////////////////////////////////
bool SchedulerEngine::WriteConfigFile(void)
{
    QString		strMonitoringPath;
    QDir		cDir;

    // Build path to the Config list.
    strMonitoringPath = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    strMonitoringPath += GEX_DATABASE_FOLDER;
    cDir.mkdir(strMonitoringPath);

    strMonitoringPath += GEXMO_STATUS_FOLDER;
    cDir.mkdir(strMonitoringPath);

    strMonitoringPath += GEXMO_STATUS_FILE;

    QFile file(strMonitoringPath); // Write the config file
    if (file.open(QIODevice::WriteOnly) == false)
        return false;	// Failed.

    // Read config File
    QTextStream hStatus(&file);	// Assign file handle to data stream

    // Fill file with Database details...
    hStatus << "<monitoring_status>" << endl;	// Start definition marker
    hStatus << "LastDataReceived=" << QString::number(time(NULL)) << endl;	// Last time a new data stram was received and processed.
    file.close();

    return true;
}
}
}


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexMoStatusTaskData::GexMoStatusTaskData(QObject* parent): TaskProperties(parent)
{
    m_strReportURL="default";	// Report's path as displayed in emails (hyperlink). Default is to give the absolute path, unless this string includes a URL like '\\server\folder'
    // Clear variables.
    m_hReportPage = NULL;

}
GexMoStatusTaskData::~GexMoStatusTaskData()
{
    // Clear list before filling it with list of lots inserted in the databases
    while (m_lstGexMoLotsList.isEmpty() == false)
        delete m_lstGexMoLotsList.takeFirst();	// Empty list!
}

///////////////////////////////////////////////////////////
// Task title setter
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::setTitle(const QString &strTitle)
{
    m_strTitle = strTitle.trimmed();
}

///////////////////////////////////////////////////////////
// True if create one child-web site per database, false otherwise.
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::setOneWebPerDatabase(bool bOneWebPerDB)
{
    m_bOneWebPerDatabase = bOneWebPerDB;
}

///////////////////////////////////////////////////////////
// Set where to publish the HTML status pages.
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::setIntranetPath(const QString &strIntranetPath)
{
    m_strIntranetPath = strIntranetPath.trimmed();
}

///////////////////////////////////////////////////////////
// Set Home page to create in Intranet.
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::setHomePage(const QString &strHomePage)
{
    m_strHomePage = strHomePage.trimmed();
}

///////////////////////////////////////////////////////////
// Set report's URL name, use to display in Emails if strReportHTTPURL is empty (hyperlink)
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::setReportURL(const QString &strReportURL)
{
    m_strReportURL = strReportURL.trimmed();
}

///////////////////////////////////////////////////////////
// Set http Report's URL name to display in Emails (hyperlink)
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::setReportHttpURL(const QString &strReportHttpURL)
{
    m_strReportHttpURL = strReportHttpURL.trimmed();
    while (m_strReportHttpURL.endsWith("/") || m_strReportHttpURL.endsWith("\\"))
        m_strReportHttpURL.truncate(m_strReportHttpURL.length() - 1);
}


///////////////////////////////////////////////////////////
// Execute Status task...Update HTML report pages (if enabled)
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::ExecuteStatusTask(CGexMoTaskStatus *ptTask,QStringList *ptFilesToRename)
{
    QString		strDatabaseMonitoringLotsFolder,strString,strDateString;
    QString		strWebChildFolder,strWebChildName;
    QDateTime	cCurrentDateTime;

    // Save pointer to list of temporary files to rename
    m_pFilesToRename = ptFilesToRename;

    cCurrentDateTime = GS::Gex::Engine::GetInstance().GetClientDateTime();
    strDateString = cCurrentDateTime.toString("yyyyMMdd");

    // Debug message
    strString = "Execute status task '";
    strString += ptTask->GetProperties()->title();
    strString += "', ";
    strString += cCurrentDateTime.toString("hh:mm:ss")+"): ";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strString.toLatin1().data());

    // Clear list before filling it with list of lots inserted in the databases
    while (m_lstGexMoLotsList.isEmpty() == false)
        delete m_lstGexMoLotsList.takeFirst();	// Empty list!

    // list all databases folders, find all lots insertion event info.
    GexDatabaseEntry *pDatabaseEntry;

#if 0
    int	iDatabaseEntry=0;
    // Old style FindDatabaseEntry(int) : use now an iterator on DB entry QList
    // Remove that when validated
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(iDatabaseEntry++);

    // Check if we have databases!
    if(pDatabaseEntry == NULL)
        return;	// No.

    while(pDatabaseEntry != NULL)
    {
        //  Read list of lots inserted in this database + merge info.
        strDatabaseMonitoringLotsFolder = pDatabaseEntry->strDatabasePhysicalPath + "/";
        strDatabaseMonitoringLotsFolder += GEXMO_STATUS_FOLDER;
        ReadLotsInserted(strDatabaseMonitoringLotsFolder,strDateString);
        if(ptTask->ptStatus->bOneWebPerDatabase)
        {
            // Create child web site: one per database
            strWebChildName = pDatabaseEntry->strDatabaseLogicalName;
            strWebChildFolder = pDatabaseEntry->strDatabasePhysicalName;
            CreateStatusWebSite(ptTask,strWebChildFolder,strWebChildName,strDateString);

            // Clear list for next child-web creation with next database info.
            while (m_lstGexMoLotsList.isEmpty() == false)
                delete m_lstGexMoLotsList.takeFirst();	// Empty list!
        }

        // Find next database entry (if any)
        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(iDatabaseEntry++);
    };
#else

    // Check if we have databases!
    if(GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.isEmpty())
        return;	// No.

    QList<GexDatabaseEntry*>::iterator itBegin	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.begin();
    QList<GexDatabaseEntry*>::iterator itEnd	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.end();

    for(itBegin=GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.begin(); itBegin != itEnd; itBegin++)
    {
        pDatabaseEntry = *itBegin;
        if(pDatabaseEntry == NULL)
            continue;

        //  Read list of lots inserted in this database + merge info.
        strDatabaseMonitoringLotsFolder = pDatabaseEntry->PhysicalPath() + "/";
        strDatabaseMonitoringLotsFolder += GEXMO_STATUS_FOLDER;
        ReadLotsInserted(strDatabaseMonitoringLotsFolder,strDateString);
        if(ptTask->GetProperties()->isOneWebPerDatabase())
        {
            // Create child web site: one per database
            strWebChildName = pDatabaseEntry->LogicalName();
            strWebChildFolder = pDatabaseEntry->PhysicalName();
            CreateStatusWebSite(ptTask,strWebChildFolder,strWebChildName,strDateString);

            // Clear list for next child-web creation with next database info.
            while (m_lstGexMoLotsList.isEmpty() == false)
                delete m_lstGexMoLotsList.takeFirst();	// Empty list!
        }
    };

#endif

    // Create Web site with merged pages
    if(ptTask->GetProperties()->isOneWebPerDatabase() == false)
        CreateStatusWebSite(ptTask,"all.databases","All databases",strDateString);
    else
        CreateStatusWebMasterHome(ptTask);	// Create master home page to all child-sites.
}

///////////////////////////////////////////////////////////
// Create HTML page + write header.
///////////////////////////////////////////////////////////
bool	GexMoStatusTaskData::CreateReportPage(GexMoStatusTaskData *pStatusTask,QString strWebChildFolder,QString strWebChildName,int iPageType,QString strChildPage)
{
    QString			strAutoReportPage,strPageName,strReportHome;
    QString			strWebOrganizationName;	// = database name, if one child site per database, 'all.databases' otherwise.
    QString			strImagesPath;
    QString			strIndexHome;
    QDateTime		cCurrentDateTime;
    bool			bIndexPages;
    QDir			cDir;
    QFile			cFile;
    QString			strNormalizedChildPage = strChildPage;

    // Normalize child page (replace special characters with '_')
    CGexSystemUtils::NormalizePath(strNormalizedChildPage);

    // Relative path to Web images is different depending of the caller.
    if(iPageType == GEXMO_REPORTPAGE_MASTERHOME)
    {
        // Master index.htm home page
        strImagesPath  = GEXMO_AUTOREPORT_FOLDER;	// Creating Indexes pages in <WebSite_rootPath>
        strImagesPath += QString("/") + GEXMO_AUTOREPORT_IMAGES_DEST;
        strIndexHome = "";		// Index (home) page is in the same folder as the page to create.
        bIndexPages = true;
    }
    else
    {
        if(strChildPage.isEmpty())
        {
            strImagesPath  = "../../";	// Images are in ../../.images
            strImagesPath += GEXMO_AUTOREPORT_IMAGES_DEST;
            strIndexHome = "";		// Index (home) page is in the same folder as the page to create.
            bIndexPages = true;
        }
        else
        {
            strImagesPath  = "../../../";	// Images are in ../../.images
            strImagesPath += GEXMO_AUTOREPORT_IMAGES_DEST;
            strIndexHome = "../../";
            strIndexHome += GEXMO_AUTOREPORT_HISTORY; // Index (home) is in the history folder, not sub-folder for products or testers.
            strIndexHome += "/";
            bIndexPages = false;
        }
    }

    cCurrentDateTime = GS::Gex::Engine::GetInstance().GetClientDateTime();
    strPageName = cCurrentDateTime.toString("yyyyMMdd");

    // Web site page stored in:
    // <WebSite>/examinator_monitoring/all.databases (if merged info)
    // <WebSite>/examinator_monitoring/<database_name> (if one child site per database)
    cDir.mkdir(pStatusTask->intranetPath());	// <WebSite>
    strAutoReportPage = pStatusTask->intranetPath() + "/" + GEXMO_AUTOREPORT_FOLDER + "/";
    cDir.mkdir(strAutoReportPage);	// <WebSite>/examinator_monitoring
    strAutoReportPage += strWebChildFolder + "/";
    strReportHome = strAutoReportPage;

    // If folder doesn't exist...this means it's the 1st installation...then copy image files+ create home page
    if(cDir.exists(strAutoReportPage) == false)
    {
        QString		strMonitoringPath;
        QString		strDestination,strSrc,strDest;
        QStringList strDataFiles;
        QStringList::Iterator it;

        // Create folders
        cDir.mkdir(strAutoReportPage);	// <WebSite>/examinator_monitoring
        strMonitoringPath = strAutoReportPage + GEXMO_AUTOREPORT_PRODUCTS;
        cDir.mkdir(strMonitoringPath);	// <WebSite>/examinator_monitoring/products
        strMonitoringPath = strAutoReportPage + GEXMO_AUTOREPORT_TESTERS;
        cDir.mkdir(strMonitoringPath);	// <WebSite>/examinator_monitoring/testers

        // Copy images folder 'autoreport' used in HTML pages.

        // Source (<gex_application>/help/autoreport)
        strMonitoringPath = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strMonitoringPath += GEXMO_AUTOREPORT_IMAGES_SRC;

        // Destination (<intranet_path>/examinator_monitoring/.images)
        strDestination = pStatusTask->intranetPath() + "/" + GEXMO_AUTOREPORT_FOLDER + "/" + GEXMO_AUTOREPORT_IMAGES_DEST;
        cDir.mkdir(strDestination);
        strDestination += "/";
        cDir.setPath(strMonitoringPath);
        cDir.setFilter(QDir::Files);
        // Files extensions to look for...: *.png
        strDataFiles = cDir.entryList(QDir::nameFiltersFromString("*.png"));
        for(it = strDataFiles.begin(); it != strDataFiles.end(); ++it )
        {
            // Add any valid file to the list...
            if((*it != ".")  && (*it != ".."))
            {
                // Destination (<intranet_path>/examinator_monitoring/.images)
                strSrc = strMonitoringPath + *it;
                strDest = strDestination + *it;
                CGexSystemUtils::CopyFile(strSrc,strDest);
            }
        }

        // Copy index.htm page in root folder.
        strSrc = strMonitoringPath + "index.htm";
        strDest = pStatusTask->intranetPath() + "/" + pStatusTask->homePage();
        CGexSystemUtils::CopyFile(strSrc,strDest);

        // Create History sub-folder
        strAutoReportPage += GEXMO_AUTOREPORT_HISTORY;
        cDir.mkdir(strAutoReportPage);

        // Copy 'products.htm' and 'testers.htm' pages
        strSrc = strMonitoringPath + "products.htm";
        strDest = strAutoReportPage + "/products.htm";
        CGexSystemUtils::CopyFile(strSrc,strDest);
        strSrc = strMonitoringPath + "testers.htm";
        strDest = strAutoReportPage + "/testers.htm";
        CGexSystemUtils::CopyFile(strSrc,strDest);
    }
    else
        strAutoReportPage += GEXMO_AUTOREPORT_HISTORY;

    switch(iPageType)
    {
    case GEXMO_REPORTPAGE_MASTERHOME:
        // Build web site master index page: 'index.htm'
        strAutoReportPage = pStatusTask->intranetPath() + "/" + pStatusTask->homePage();
        break;

    case GEXMO_REPORTPAGE_HOME:
        // Build home Index page : 'home.htm'
        strAutoReportPage += "/home.htm";
        break;

    case GEXMO_REPORTPAGE_PRODUCTS:
        // Build 'p-<YYYYMMDD>.htm' HTML page name if main 'Products' index page
        // or
        // <WebSite>/<>/products/<ProductID>/p-<YYYYMMDD>.htm if child page.
        if(strChildPage.isEmpty() == false)
        {
            // We have to build a child page that will list all sub-lots tested for  a given Product
            strAutoReportPage = strReportHome + GEXMO_AUTOREPORT_PRODUCTS;
            strAutoReportPage += "/" + strNormalizedChildPage;
            CGexSystemUtils::NormalizePath(strAutoReportPage);
            cDir.mkdir(strAutoReportPage);	// <WebSite>/products/<ProductID>
        }
        // Save folder where the page is created.
        m_strHtmlPageFolder = strAutoReportPage;

        // Html page to create
        strAutoReportPage += "/p-" + strPageName;
        strAutoReportPage += ".htm";

        // If this page is new, then update the past-History page in its folder!
        cFile.setFileName(strAutoReportPage);
        if(cFile.exists() == false)
            UpdatePastHistoryPage(pStatusTask,strWebChildFolder,strWebChildName,bIndexPages,GEXMO_HISTORYPAGE_PRODUCTS,strChildPage);
        break;

    case GEXMO_REPORTPAGE_TESTERS:
        // Build 't-<YYYYMMDD>.htm' HTML page name
        // or
        // <WebSite>/testers/<ProductID>/p-<YYYYMMDD>.htm if child page.
        if(strChildPage.isEmpty() == false)
        {
            // We have to build a child page that will list all sub-lots tested for  a given Product
            strAutoReportPage = strReportHome + GEXMO_AUTOREPORT_TESTERS;
            strAutoReportPage += "/" + strNormalizedChildPage;
            CGexSystemUtils::NormalizePath(strAutoReportPage);
            cDir.mkdir(strAutoReportPage);	// <WebSite>/testers/<TesterName>
        }
        // Save folder where the page is created.
        m_strHtmlPageFolder = strAutoReportPage;

        // Html page to create
        strAutoReportPage += "/t-" + strPageName;
        strAutoReportPage += ".htm";

        // If this page is new, then update the past-History page in its folder!
        cFile.setFileName(strAutoReportPage);
        if(cFile.exists() == false)
            UpdatePastHistoryPage(pStatusTask,strWebChildFolder,strWebChildName,bIndexPages,GEXMO_HISTORYPAGE_TESTERS,strChildPage);
        break;

    case GEXMO_HISTORYPAGE_PRODUCTS:
    case GEXMO_HISTORYPAGE_TESTERS:
        // Build 'x-history.htm' HTML page name if main 'Products' or 'Testers' index page
        // or
        // <WebSite>/products/<ProductID>/x-history.htm if child page.
        if(bIndexPages == false)
        {
            // We have to build a child page History for all sub-lots tested for a given product or tester
            if(iPageType == GEXMO_HISTORYPAGE_PRODUCTS)
                strAutoReportPage = strReportHome + GEXMO_AUTOREPORT_PRODUCTS;
            else
                strAutoReportPage = strReportHome + GEXMO_AUTOREPORT_TESTERS;
            strAutoReportPage += "/" + strNormalizedChildPage;
            CGexSystemUtils::NormalizePath(strAutoReportPage);
            cDir.mkdir(strAutoReportPage);	// <WebSite>/products/<ProductID>
        }
        // Html page to create
        if(iPageType == GEXMO_HISTORYPAGE_PRODUCTS)
            strAutoReportPage += "/p-history.htm";
        else
            strAutoReportPage += "/t-history.htm";
        break;
    }

    // Add temporary extension as real name is used later (timer based) so to avoid read/write issues!
    strAutoReportPage += GEX_TEMPORARY_HTML;
    m_hReportPage = fopen(strAutoReportPage.toLatin1().constData(),"w");
    if(m_hReportPage == NULL)
        return false;	// Failed creating HTML page...

    // Update list of files to rename later.
    *m_pFilesToRename += strAutoReportPage;

    fprintf(m_hReportPage,"<html>\n");
    fprintf(m_hReportPage,"<!-- ***************************************************************************-->\n");
    fprintf(m_hReportPage,"<!-- %s-->\n",
            GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
    fprintf(m_hReportPage,"<!-- %s.   www.mentor.com-->\n",C_STDF_COPYRIGHT);
    fprintf(m_hReportPage,"<!-- All rights reserved. Users of the program must be Registered Users (see-->\n");
    fprintf(m_hReportPage,"<!-- the Quantix examinator license terms).-->\n");
    fprintf(m_hReportPage,"<!-- The Quantix Examinator program (including its HTML pages and .png files)-->\n");
    fprintf(m_hReportPage,"<!-- is protected by copyright law and international treaties. You are no-->\n");
    fprintf(m_hReportPage,"<!-- allowed to alter any HTML page, .png file, or any other part of the-->\n");
    fprintf(m_hReportPage,"<!-- software. Unauthorized reproduction or distribution of this program,-->\n");
    fprintf(m_hReportPage,"<!-- or any portion of it, may result in severe civil and criminal -->\n");
    fprintf(m_hReportPage,"<!-- penalties, and will be prosecuted to the maximum extent possible.-->\n");
    fprintf(m_hReportPage,"<!-- ***************************************************************************-->\n");
    fprintf(m_hReportPage,"<head>\n");
    fprintf(m_hReportPage,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n");

    // If writing the 'index' or 'home' or 'products' or 'testers' master pages (page of the day) force auto-refresh every 1 minute.
    if(strChildPage.isEmpty() == true)
        fprintf(m_hReportPage,"<meta http-equiv=\"Refresh\" CONTENT=\"60\">\n");

    fprintf(m_hReportPage,"<title>Quantix Examinator - the STDF detective</title>\n");
    fprintf(m_hReportPage,"</head>\n");

    // Sets default background color & text color given in argument.
    fprintf(m_hReportPage,"<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n");	// Default color in HTML page!

    // Navigation bar
    fprintf(m_hReportPage,"<table border=\"0\" cellspacing=\"1\" style=\"border-collapse: collapse\" bordercolor=\"#111111\" width=\"100%%\">\n");
    fprintf(m_hReportPage,"<tr>\n");
    fprintf(m_hReportPage,"<td width=\"100%%\"><font color=\"#006699\">\n");

    QString strTitle;
    switch(iPageType)
    {
    case GEXMO_REPORTPAGE_MASTERHOME: // HTML Top bar with 'Home' Highlighted
        strTitle = pStatusTask->title();
        fprintf(m_hReportPage,"<img src=\"%s/home000.png\" border=\"0\">",strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"%s%s\"><img src=\"%s/home001.png\" border=\"0\" alt=\"Yield-Man Home Page\"></a>",strIndexHome.toLatin1().constData(),pStatusTask->homePage().toLatin1().constData(), strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<img src=\"%s/masterhome002.png\" border=\"0\">",strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<img src=\"%s/masterhome003.png\" border=\"0\">",strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<img src=\"%s/home004.png\" border=\"0\"></font></td>\n",strImagesPath.toLatin1().constData());
        break;

    case GEXMO_REPORTPAGE_HOME:	// HTML Top bar with 'Home' Highlighted
        strTitle = pStatusTask->title();
        fprintf(m_hReportPage,"<img src=\"%s/home000.png\" border=\"0\">",strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"%shome.htm\"><img src=\"%s/home001.png\" border=\"0\" alt=\"Yield-Man Home Page\"></a>",strIndexHome.toLatin1().constData(), strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"%sproducts.htm\"><img src=\"%s/home002.png\" border=\"0\"></a>",strIndexHome.toLatin1().constData(), strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"%stesters.htm\"><img src=\"%s/home003.png\" border=\"0\"</a>",strIndexHome.toLatin1().constData(), strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<img src=\"%s/home004.png\" border=\"0\"></font></td>\n",strImagesPath.toLatin1().constData());
        break;

    case GEXMO_REPORTPAGE_PRODUCTS:	// HTML Top bar with 'Products' Highlighted
    case GEXMO_HISTORYPAGE_PRODUCTS:	// HTML Top bar with 'Products' Highlighted
        if(strChildPage.isEmpty() == true)
            strTitle = "Products Status";
        else
            strTitle = "Product: " + strChildPage;
        fprintf(m_hReportPage,"<img src=\"%s/prod_000.png\" border=\"0\">",strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"%shome.htm\"><img src=\"%s/prod_001.png\" border=\"0\" alt=\"Yield-Man Home Page\"></a>",strIndexHome.toLatin1().constData(), strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"%sproducts.htm\"><img src=\"%s/prod_002.png\" border=\"0\"></a>",strIndexHome.toLatin1().constData(), strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"%stesters.htm\"><img src=\"%s/prod_003.png\" border=\"0\"</a>",strIndexHome.toLatin1().constData(), strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<img src=\"%s/home004.png\" border=\"0\"></font></td>\n",strImagesPath.toLatin1().constData());
        break;

    case GEXMO_REPORTPAGE_TESTERS:	// HTML Top bar with 'Testers' Highlighted
    case GEXMO_HISTORYPAGE_TESTERS:	// HTML Top bar with 'Testers' Highlighted
        if(strChildPage.isEmpty() == true)
            strTitle = "Testers Status";
        else
            strTitle = "Tester: " + strChildPage;
        fprintf(m_hReportPage,"<img src=\"%s/test_000.png\" border=\"0\">",strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"%shome.htm\"><img src=\"%s/test_001.png\" border=\"0\" alt=\"Yield-Man Home Page\"></a>",strIndexHome.toLatin1().constData(), strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"%sproducts.htm\"><img src=\"%s/test_002.png\" border=\"0\"></a>",strIndexHome.toLatin1().constData(), strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"%stesters.htm\"><img src=\"%s/test_003.png\" border=\"0\"</a>",strIndexHome.toLatin1().constData(), strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<img src=\"%s/home004.png\" border=\"0\"></font></td>\n",strImagesPath.toLatin1().constData());
        break;
    }
    fprintf(m_hReportPage,"</tr>\n");
    fprintf(m_hReportPage,"</table>\n");

    // Write Page title
    fprintf(m_hReportPage,"<h1 align=\"left\"><font color=\"#006699\">");

    // Show database name...unless write master home page.
    if(iPageType != GEXMO_REPORTPAGE_MASTERHOME)
    {
        QString strString;
        strString = "Database : " + strWebChildName;
        strString += "<br>"+ strTitle;
        strTitle = strString;
    }

    switch(iPageType)
    {
    case GEXMO_REPORTPAGE_MASTERHOME: // HTML Top bar with 'Home' Highlighted
    case GEXMO_REPORTPAGE_HOME:	// HTML Top bar with 'Home' Highlighted
    case GEXMO_REPORTPAGE_PRODUCTS:	// HTML Top bar with 'Products' Highlighted
    case GEXMO_REPORTPAGE_TESTERS:	// HTML Top bar with 'Testers' Highlighted
        fprintf(m_hReportPage,"%s - %s<br>\n",strTitle.toLatin1().constData(), cCurrentDateTime.toString("MMM d, yyyy").toLatin1().constData());
        break;
    case GEXMO_HISTORYPAGE_PRODUCTS:	// HTML Top bar with 'Products' Highlighted
    case GEXMO_HISTORYPAGE_TESTERS:	// HTML Top bar with 'Testers' Highlighted
        fprintf(m_hReportPage,"%s<br>History since %s<br>\n",strTitle.toLatin1().constData(), cCurrentDateTime.toString("MMM d, yyyy").toLatin1().constData());
        break;
    }
    fprintf(m_hReportPage,"<img src=\"%s/ruler.png\" border=\"0\" width=\"616\" height=\"8\"></font><br></h1>\n",strImagesPath.toLatin1().constData());
    fprintf(m_hReportPage,"<p align=\"left\">&nbsp;</p>\n");

    return true;
}

///////////////////////////////////////////////////////////
// Close HTML page (also write footer)
///////////////////////////////////////////////////////////
void	GexMoStatusTaskData::CloseReportPage(int iPageType,bool bIndexPages,int iHistoryLink)
{
    QString		strImagesPath;
    QDateTime	cCurrentDateTime;
    cCurrentDateTime = GS::Gex::Engine::GetInstance().GetClientDateTime();
    QString		strIndexHome;

    // Relative path to Web images is different depending of the
    if(iPageType == GEXMO_REPORTPAGE_MASTERHOME)
    {
        // Master index.htm home page.
        strImagesPath = GEXMO_AUTOREPORT_FOLDER;
        strImagesPath += QString("/") + GEXMO_AUTOREPORT_IMAGES_DEST;
        strIndexHome = "";
    }
    else
    {
        if(bIndexPages)
        {
            strImagesPath = "../../";	// image in ../.images
            strImagesPath += GEXMO_AUTOREPORT_IMAGES_DEST;
            strIndexHome = "";		// Index (home) page is in the same folder as the page to create.
        }
        else
        {
            strImagesPath = "../../../";	// Creating events pages (for each sub-lot).
            strImagesPath += GEXMO_AUTOREPORT_IMAGES_DEST;
            strIndexHome = "../../";
            strIndexHome += GEXMO_AUTOREPORT_HISTORY; // Index (home) is in the history folder, not sub-folder for products or testers.
            strIndexHome += "/";
        }
    }

    fprintf(m_hReportPage,"<font color=\"#006699\" size=\"2\"><br>Last update: %s</font><br>\n", cCurrentDateTime.toString("MMM d, yyyy hh:mm").toLatin1().constData());

    // If index page, tell that auto-refresh occures every minute.
    if(bIndexPages)
        fprintf(m_hReportPage,"<font color=\"#006699\" size=\"2\">Page auto reload: every minute.</font><br>\n");

    // Write notice that table content can be customized from the Monitoring/Options tab
    fprintf(m_hReportPage,"<font color=\"#006699\" size=\"2\"><br><b>Hint:</b> To customize fields listed in tables, run Yield-Man then click the 'Options' tab.</font><br>\n");

    // Write past-History links (if requested)
    if(iHistoryLink != GEXMO_REPORTPAGE_NOHISTORY)
    {
        fprintf(m_hReportPage,"<h1 align=\"left\"><font color=\"#006699\">");
        fprintf(m_hReportPage,"History<br>\n");
        fprintf(m_hReportPage,"<img src=\"%s/ruler.png\" border=\"0\" width=\"616\" height=\"8\"></font><br></h1>\n",strImagesPath.toLatin1().constData());

        QString strMonitorHistoryOption = (ReportOptions.GetOption(QString("monitoring"), QString("history"))).toString();
        if(strMonitorHistoryOption == QString("none"))
        {
            // History is disabled.
            fprintf(m_hReportPage,"<p align=\"left\">Disabled (</p>\n");
        }
        else
        {
            // Create link to History page (to Testers or Products page)
            if(iHistoryLink == GEXMO_REPORTPAGE_PRODUCTS)
                fprintf(m_hReportPage,"<p align=\"left\"><a href=\"p-history.htm\">Visit past history</a></td>\n");
            else
                fprintf(m_hReportPage,"<p align=\"left\"><a href=\"t-history.htm\">Visit past history</a></td>\n");
        }
    }

    fprintf(m_hReportPage,"<hr>\n");
    fprintf(m_hReportPage,"<p><font color=\"#006699\">Report created with: %s -	www.mentor.com</font></p>\n",
            GS::Gex::Engine::GetInstance().Get("AppFullName")
            .toString().toLatin1().data() );
    fprintf(m_hReportPage,"<p><font color=\"#006699\">");

    // If hone page (but not MASTER web home page) define hyperlink.
    if(iPageType != GEXMO_REPORTPAGE_MASTERHOME)
        fprintf(m_hReportPage,"<a href=\"%shome.htm\">\n",strIndexHome.toLatin1().constData());
    fprintf(m_hReportPage,"<img src=\"%s/web_bottom.png\" border=\"0\" width=\"900\" height=\"22\">",strImagesPath.toLatin1().constData());

    if(iPageType != GEXMO_REPORTPAGE_MASTERHOME)
        fprintf(m_hReportPage,"</a>");

    fprintf(m_hReportPage,"</font></p>\n</body>\n");
    fprintf(m_hReportPage,"</html>\n");
    fclose(m_hReportPage);
    m_hReportPage = NULL;
}

///////////////////////////////////////////////////////////
// Create the past history page in given folder
// File: p-history.htm (for a given product history)
// or    t-history.htm (for a given tester history)
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::UpdatePastHistoryPage(GexMoStatusTaskData *pStatusTask,QString strWebChildFolder,QString strWebChildName,bool bIndexPages,int iHistoryPageType,QString strChildPage)
{
    // Create the 'history.htm' page in the relevant folder.
    if(!CreateReportPage(pStatusTask,strWebChildFolder,strWebChildName,iHistoryPageType,strChildPage))
        return;	// failed creating pages

    // Enumerate ALL files in the folder
    QDir		d;
    QString		strFilesToList;
    QString		strFile;
    QStringList strDataFiles;
    QStringList::Iterator it;
    switch(iHistoryPageType)
    {
    case GEXMO_REPORTPAGE_PRODUCTS:
    case GEXMO_HISTORYPAGE_PRODUCTS:
        strFilesToList = "p-*.htm";
        break;
    case GEXMO_REPORTPAGE_TESTERS:
    case GEXMO_HISTORYPAGE_TESTERS:
        strFilesToList = "t-*.htm";
        break;
    }

    d.setPath(m_strHtmlPageFolder);	// Folder where the x-history.htm file is to be created.
    d.setFilter(QDir::Files);
    strDataFiles = d.entryList(QDir::nameFiltersFromString(strFilesToList));	// Files types to look for...
    strDataFiles.sort();	// Sort them so first one is the oldest.

    // Write table hader
    fprintf(m_hReportPage,"<blockquote>\n");
    fprintf(m_hReportPage,"<table style=\"BORDER-COLLAPSE: collapse\" cellSpacing=\"0\" cellPadding=\"0\" width=\"700\" border=\"1\">\n");
    fprintf(m_hReportPage,"<tr>\n");
    fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Date (Month/Year)</b></td>\n");
    fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Day</b></td>\n");
    fprintf(m_hReportPage,"</tr>\n");

    // Read from end so to list from most recent to oldest file...
    int iPrevYear=0,iPrevMonth=0;
    int iYear=0,iMonth=0,iDay=0;
    int	iIndex = strDataFiles.count()-1;	// Start reading from the last name (nost recent)
    QString strString;
    QDate cDate;
    while(iIndex >= 0)
    {
        // Extract YYYY MM DD from file name'x-YYYYMMDD.htm'
        strFile = strDataFiles[iIndex];
        sscanf(strFile.toLatin1().constData(),"%*c%*c%4d%2d%2d",&iYear,&iMonth,&iDay);

        // If new month, create new table line
        if((iYear != iPrevYear) || (iPrevMonth != iMonth))
        {
            // If this is NOT the first line, close previous line
            if(iPrevYear != 0)
                fprintf(m_hReportPage,"</td>\n</tr>\n");

            // Copy Month+Year to buffers.
            iPrevYear  = iYear;
            iPrevMonth = iMonth;

            // Write Month+Year (e.g: November 2004)
            if (iYear < 99) iYear += 1900;
            cDate.setDate(iYear,iMonth,iDay);
            strString = cDate.toString("MMMM yyyy");
            fprintf(m_hReportPage,"<tr>\n");
            fprintf(m_hReportPage,"<td align=\"center\" width=\"150\">%s</td>\n<td width=\"550\">\n",strString.toLatin1().constData());
        }

        // Write valid day with hyperlink to its relevant file
        if(iYear && iDay)
            fprintf(m_hReportPage,"&nbsp;<a href=\"%s\">%d</a>&nbsp;\n",strFile.toLatin1().constData(), iDay);

        // Move to previous item (older)
        iIndex--;
    };

    // Close last Month line written
    fprintf(m_hReportPage,"</td>\n</tr>\n");

    // Close table
    fprintf(m_hReportPage,"</table>\n");
    fprintf(m_hReportPage,"</blockquote>\n");

    // Clean close of the report page. (write footer + close).
    CloseReportPage(iHistoryPageType,bIndexPages,GEXMO_REPORTPAGE_NOHISTORY);

#if 0
    // Monitoring - History size (number of HTML status pages to keep)
    switch(ReportOptions.iMonitorHistory)
    {
    case	GEXMO_OPTION_HISTORY_NONE:
        fprintf(m_hReportPage,"<p align=\"left\">Disabled</p>\n");
        break;
    case	GEXMO_OPTION_HISTORY_1WEEK:
        fprintf(m_hReportPage,"<p align=\"left\">1 Week</p>\n");
        break;
    case	GEXMO_OPTION_HISTORY_2WEEK:
        ptMonitoring_History_2week->setChecked(true);	// Keep 2 weeks of pages
        break;
    case	GEXMO_OPTION_HISTORY_3WEEK:
        ptMonitoring_History_3week->setChecked(true);	// Keep 3 weeks of pages
        break;
    case	GEXMO_OPTION_HISTORY_1MONTH:
        ptMonitoring_History_1month->setChecked(true);	// Keep 1 month of pages
        break;
    case	GEXMO_OPTION_HISTORY_2MONTH:
        ptMonitoring_History_2month->setChecked(true);	// Keep 2 months of pages
        break;
    case	GEXMO_OPTION_HISTORY_3MONTH:
        ptMonitoring_History_3month->setChecked(true);	// Keep 3 months of pages
        break;
    case	GEXMO_OPTION_HISTORY_4MONTH:
        ptMonitoring_History_4month->setChecked(true);	// Keep 4 months of pages
        break;
    case	GEXMO_OPTION_HISTORY_5MONTH:
        ptMonitoring_History_5month->setChecked(true);	// Keep 5 months of pages
        break;
    case	GEXMO_OPTION_HISTORY_6MONTH:
        ptMonitoring_History_6month->setChecked(true);	// Keep 6 months of pages
        break;
    case	GEXMO_OPTION_HISTORY_1YEAR:
        ptMonitoring_History_1year->setChecked(true);	// Keep 1 year of pages
        break;
    case	GEXMO_OPTION_HISTORY_ALL:
        ptMonitoring_History_all->setChecked(true);		// Keep all pages
        break;
    }
#endif
}

///////////////////////////////////////////////////////////
// Read list of lots inserted into a given database
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::ReadLotsInserted(QString strDatabaseMonitoringLotsFolder,QString strDateString)
{
    // File: <Database_physical_path>/.monitor/YYYYMMDD.dat
    QString strScanDatabase = strDatabaseMonitoringLotsFolder;
    strDatabaseMonitoringLotsFolder += "/" + strDateString;
    strDatabaseMonitoringLotsFolder += ".dat";

    // Open file in Read mode
    QFile file(strDatabaseMonitoringLotsFolder); // Read the config file
    if (file.open(QIODevice::ReadOnly) == false)
        return;	// Failed.

    // Read config File
    QTextStream hStatus(&file);	// Assign file handle to data stream

    // Check if valid header...or empty!
    QString strString = hStatus.readLine();
    if(strString != "<monitoring_lots>")
    {
        file.close();
        return;
    }

    // Read info about all lots processed today...
    QStringList strSections;
    CGexMoLotsProcessed	*pLotProcessed;
    long	lCount;

    do
    {
        // Read one line from file
        strString = hStatus.readLine();

        // Extract the N line parameters to parse.
        strSections = strString.split(",",QString::KeepEmptyParts);
        lCount = strSections.count();

        // Check if end of file / invalid line...then exit loop
        if(lCount <= 0)
            break;

        // Create structure to hold data + save into list
        // For each section, make sure it exists!!

        pLotProcessed = new CGexMoLotsProcessed;
        if(strSections.size() > GEXMO_LOT_PROCESSED_STARTTIME)
            pLotProcessed->tStartTime = strSections[GEXMO_LOT_PROCESSED_STARTTIME].toLong();
        if(strSections.size() > GEXMO_LOT_PROCESSED_PRODUCT)
            pLotProcessed->strProductID = strSections[GEXMO_LOT_PROCESSED_PRODUCT];
        if(strSections.size() > GEXMO_LOT_PROCESSED_LOT)
            pLotProcessed->strLot = strSections[GEXMO_LOT_PROCESSED_LOT];
        if(strSections.size() > GEXMO_LOT_PROCESSED_SUBLOT)
            pLotProcessed->strSubLot = strSections[GEXMO_LOT_PROCESSED_SUBLOT];
        if(strSections.size() > GEXMO_LOT_PROCESSED_NODENAME)
            pLotProcessed->strNodeName = strSections[GEXMO_LOT_PROCESSED_NODENAME];
        if(strSections.size() > GEXMO_LOT_PROCESSED_OPERATOR)
            pLotProcessed->strOperator = strSections[GEXMO_LOT_PROCESSED_OPERATOR];
        if(strSections.size() > GEXMO_LOT_PROCESSED_JOBNAME)
            pLotProcessed->strJobName = strSections[GEXMO_LOT_PROCESSED_JOBNAME];
        if(strSections.size() > GEXMO_LOT_PROCESSED_JOBREV)
            pLotProcessed->strJobRev = strSections[GEXMO_LOT_PROCESSED_JOBREV];
        if(strSections.size() > GEXMO_LOT_PROCESSED_GOODPARTS)
            pLotProcessed->lTotalGoodBins = strSections[GEXMO_LOT_PROCESSED_GOODPARTS].toLong();
        if(strSections.size() > GEXMO_LOT_PROCESSED_TOTALPARTS)
            pLotProcessed->lTotalParts = strSections[GEXMO_LOT_PROCESSED_TOTALPARTS].toLong();
        if(strSections.size() > GEXMO_LOT_PROCESSED_WAFER)
            pLotProcessed->strWafer = strSections[GEXMO_LOT_PROCESSED_WAFER];
        if(pLotProcessed->strLot.isEmpty())
            pLotProcessed->strLot = "Unknown";
        if(pLotProcessed->strProductID.isEmpty())
            pLotProcessed->strProductID = "Unknown";
        if(pLotProcessed->strNodeName.isEmpty())
            pLotProcessed->strNodeName = "Unknown";
        if(pLotProcessed->strSubLot.isEmpty())
            pLotProcessed->strSubLot = "-";
        if(pLotProcessed->strWafer.isEmpty())
            pLotProcessed->strWafer = "-";

        m_lstGexMoLotsList.append(pLotProcessed);
    }
    while(hStatus.atEnd() == false);

    // Close file.
    file.close();
}

void GexMoStatusTaskData::UpdatePrivateAttributes()
{
    ResetPrivateAttributes();
    // Insert new values
    SetPrivateAttribute("Title",title());
    SetPrivateAttribute("WebOrganization",(isOneWebPerDatabase() ? "1" : "0"));
    SetPrivateAttribute("IntranetPath",intranetPath());
    SetPrivateAttribute("HomePage",homePage());
    SetPrivateAttribute("ReportURL",reportURL());
    SetPrivateAttribute("ReportHttpURL",reportHttpURL());
}

GexMoStatusTaskData &GexMoStatusTaskData::operator=(const GexMoStatusTaskData &copy)
{
    if(this != &copy)
    {
        m_strTitle              = copy.title();
        m_bOneWebPerDatabase    = copy.m_bOneWebPerDatabase;
        m_strIntranetPath       = copy.m_strIntranetPath;
        m_strHomePage           = copy.m_strHomePage;
        m_strReportURL          = copy.m_strReportURL;
        m_strReportHttpURL      = copy.m_strReportHttpURL;

        TaskProperties::operator =(copy);
        UpdatePrivateAttributes();
    }
    return *this;
}
///////////////////////////////////////////////////////////
// Create Web Status pages
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::CreateStatusWebMasterHome(CGexMoTaskStatus *ptTask)
{
    // Write Page header: 'Home' page
    if(!CreateReportPage(ptTask->GetProperties(),"","",GEXMO_REPORTPAGE_MASTERHOME))
        return;	// failed creating pages

    // Write table hader
    fprintf(m_hReportPage,"<blockquote>\n");
    fprintf(m_hReportPage,"<table style=\"BORDER-COLLAPSE: collapse\" cellSpacing=\"0\" cellPadding=\"0\" width=\"70%%\" border=\"1\">\n");
    fprintf(m_hReportPage,"<tr>\n");
    fprintf(m_hReportPage,"<td width=\"40%%\" align=\"center\" bgColor=\"#CCECFF\"><b>Database name</b></td>\n");
    fprintf(m_hReportPage,"<td width=\"40%%\" align=\"center\" bgColor=\"#CCECFF\"><b>Description</b></td>\n");
    fprintf(m_hReportPage,"<td width=\"20%%\" align=\"center\" bgColor=\"#CCECFF\"><b>Size (Mbytes)</b></td>\n");
    fprintf(m_hReportPage,"</tr>\n");

    // list all databases folders available
    QString strDescription;
    GexDatabaseEntry *pDatabaseEntry;
#if 0

    // Deprecated code. Remove me.
    int	iDatabaseEntry=0;
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(iDatabaseEntry++);

    while(pDatabaseEntry != NULL)
    {
        // Write Database name + description + size
        fprintf(m_hReportPage,"<tr>\n");
        fprintf(m_hReportPage,"<td width=\"40%%\"><font color=\"#006699\"><a href=\"%s/%s/history/home.htm\"><b>%s</b></a></font></td>\n",GEXMO_AUTOREPORT_FOLDER,pDatabaseEntry->strDatabasePhysicalName.toLatin1().constData(), pDatabaseEntry->strDatabaseLogicalName.toLatin1().constData());
        strDescription = pDatabaseEntry->strDatabaseDescription;
        if(strDescription.isEmpty()) strDescription = "-";
        fprintf(m_hReportPage,"<td width=\"40%%\" align=\"center\"><font color=\"#006699\">%s</font></td>\n",strDescription.toLatin1().constData());
        fprintf(m_hReportPage,"<td width=\"20%%\" align=\"center\"><font color=\"#006699\">%.2f Mb</font></td>\n",pDatabaseEntry->fSize/1048576.0);
        fprintf(m_hReportPage,"</tr>\n");

        // Find next database entry (if any)
        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(iDatabaseEntry++);
    };

#else

    QList<GexDatabaseEntry*>::iterator itBegin;
    QList<GexDatabaseEntry*>::iterator itEnd	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.end();

    for(itBegin=GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.begin(); itBegin != itEnd; itBegin++)
    {
        pDatabaseEntry = *itBegin;
        if(pDatabaseEntry == NULL)
            continue;

        // Write Database name + description + size
        fprintf(m_hReportPage,"<tr>\n");
        fprintf(m_hReportPage,"<td width=\"40%%\"><font color=\"#006699\"><a href=\"%s/%s/history/home.htm\"><b>%s</b></a></font></td>\n",
                GEXMO_AUTOREPORT_FOLDER,
                pDatabaseEntry->PhysicalName().toLatin1().constData(),
                pDatabaseEntry->LogicalName().toLatin1().constData());
        strDescription = pDatabaseEntry->Description();
        if(strDescription.isEmpty()) strDescription = "-";
        fprintf(m_hReportPage,"<td width=\"40%%\" align=\"center\"><font color=\"#006699\">%s</font></td>\n",strDescription.toLatin1().constData());
        fprintf(m_hReportPage,"<td width=\"20%%\" align=\"center\"><font color=\"#006699\">%.2f Mb</font></td>\n",pDatabaseEntry->CacheSize());
        fprintf(m_hReportPage,"</tr>\n");
    }
#endif
    // Close table
    fprintf(m_hReportPage,"</table>\n");
    fprintf(m_hReportPage,"</blockquote>\n");

    // Clean close of the report page. (write footer + close).
    CloseReportPage(GEXMO_REPORTPAGE_MASTERHOME,true,GEXMO_REPORTPAGE_NOHISTORY);
}

///////////////////////////////////////////////////////////
// Create Web Status pages
///////////////////////////////////////////////////////////
void GexMoStatusTaskData::CreateStatusWebSite(CGexMoTaskStatus *ptTask,QString strDatabaseName,QString strWebChildName, QString strDateString)
{
    if(pGexMainWindow == NULL)
        return;

    // one folder per database, folder name is database name (normalized - no white chars.)
    QString			strWebChildFolder = strDatabaseName;

    // OPTIONS
    QString strMonitoringProductPageOptions = (ReportOptions.GetOption(QString("monitoring"), QString("product_page"))).toString();
    QStringList qslMonitoringProductPageOptionList = strMonitoringProductPageOptions.split(QString("|"));
    QString strMonitoringTesterPageOption = (ReportOptions.GetOption(QString("monitoring"), QString("tester_page"))).toString();
    QStringList qslMonitoringTesterPageOptionList = strMonitoringTesterPageOption.split(QString("|"));

    ////////////////////////////////////////////////////////////////////////////////////////
    // PRODUCTS PAGE
    ////////////////////////////////////////////////////////////////////////////////////////
    // Create HTML pages: Product pages.
    qSort(m_lstGexMoLotsList.begin(), m_lstGexMoLotsList.end(), sortByProduct);

    // Write Page header: 'Products' page
    if(!CreateReportPage(ptTask->GetProperties(),strWebChildFolder,strWebChildName,GEXMO_REPORTPAGE_PRODUCTS))
        return;	// failed creating pages

    // Write table hader
    fprintf(m_hReportPage,"<table style=\"BORDER-COLLAPSE: collapse\" cellSpacing=\"0\" cellPadding=\"0\" width=\"90%%\" border=\"1\">\n");
    fprintf(m_hReportPage,"<tr>\n");
    fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Product ID</b></td>\n");
    if(qslMonitoringProductPageOptionList.contains(QString("total_parts")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Total parts</b></td>\n");
    if(qslMonitoringProductPageOptionList.contains(QString("total_good")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Good</b></td>\n");
    if(qslMonitoringProductPageOptionList.contains(QString("total_fail")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Fail</b></td>\n");
    if(qslMonitoringProductPageOptionList.contains(QString("yield")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Yield level</b></td>\n");
    if(qslMonitoringProductPageOptionList.contains(QString("alarm_limit")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Alarm level</b></td>\n");
    if(qslMonitoringProductPageOptionList.contains(QString("yield_chart")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Yield Chart</b></td>\n");
    fprintf(m_hReportPage,"</tr>\n");

    // Read list with 'Product' name in Ascending order
    // Create: 'PRODUCTS' page
    QString	strString, strNormalizedString;
    QString strCurrentProduct,strYield,strAlarmLevel;
    QString	strChartImage;	// Either RED (fail) or GREEN (default - pass) bar
    QString	strBackgroundColor;	// Either RED (fail) or yellow (Passs)
    int		iCheckIfHigher=0;	// Tells if we check to be over or under a limit
    long	lTotalProductsTested=0;
    long	lTestersInUse=0;
    long	lTotal,lGood;
    float	fYield,fAlarmYield=0.0F;
    bool	bEntryFound;
    int		iYieldBarSize;	// Size (in pixel) of the Yile level bar. 200 pixels=100%
    // Used for checking Yield on a specific ProductID
    CGexMoTaskYieldMonitor *ptYieldTask;

    CGexMoLotsProcessed		*pLotProcessed=NULL, *pNextLot=NULL;
    int						nIndex=0;

    while(nIndex < m_lstGexMoLotsList.size())
    {
        pLotProcessed		= m_lstGexMoLotsList.at(nIndex);
        strCurrentProduct	= pLotProcessed->strProductID;
        lTotal				= pLotProcessed->lTotalParts;
        lGood				= pLotProcessed->lTotalGoodBins;
        bEntryFound			= true;

        while((nIndex+1) < m_lstGexMoLotsList.size())
        {
            pNextLot=m_lstGexMoLotsList.at(nIndex+1);
            if(strCurrentProduct != pNextLot->strProductID)
                break;
            // Cumulate all entries that belong to the same product.
            lTotal		+= pNextLot->lTotalParts;
            lGood		+= pNextLot->lTotalGoodBins;

            // Move to next entry in sorted list.
            pLotProcessed	= pNextLot;
            nIndex++;
        };

        if(bEntryFound == false)
            break;	// reached en of list!

        // Keep track of total number of products under test
        lTotalProductsTested++;

        // Write Product line: one per product.
        strNormalizedString = strCurrentProduct;
        CGexSystemUtils::NormalizePath(strNormalizedString);
        fprintf(m_hReportPage,"<tr>\n");
        fprintf(m_hReportPage,"<td align=\"middle\" bgColor=\"#CCECFF\"><b>\n");
        fprintf(m_hReportPage,"<a href=\"../products/%s/p-%s.htm\">%s</a></b></td>",strNormalizedString.toLatin1().constData(), strDateString.toLatin1().constData(), strCurrentProduct.toLatin1().constData());
        if(lTotal > 0)
        {
            fYield = (100.0*(float)lGood)/(float)lTotal;
            strYield = QString::number(fYield,'f',2) + " %";
        }
        else
        {
            fYield = 0.0;
            strYield = "-";
        }

        // Get Yield alarm level set in the Scheduler task list
        fAlarmYield = 0.0F;	// Default is NO alarm level defined.
        iCheckIfHigher = 0;	// Default: Check if yield under given limit.
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            // Ignore TestingStage ?
            // Find Yield task matching our Product AND including a list of Good Bins
            ptYieldTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductYieldInfo(strCurrentProduct,strDatabaseName,"Final Test",true);
            if(ptYieldTask == NULL)
                ptYieldTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductYieldInfo(strCurrentProduct,strDatabaseName,"Wafer Sort",true);
            if(ptYieldTask == NULL)
                ptYieldTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductYieldInfo(strCurrentProduct,strDatabaseName,"E-Test",true);
            if(ptYieldTask != NULL)
            {
                fAlarmYield = ptYieldTask->GetProperties()->iAlarmLevel;
                iCheckIfHigher = ptYieldTask->GetProperties()->iAlarmIfOverLimit;
                strAlarmLevel = QString::number(fAlarmYield) + " %";
            }
            else
                strAlarmLevel = "-";
        }
        else
            strAlarmLevel = "-";

        // Compute Yield bar size
        iYieldBarSize = (int)(2.0*fYield);	// 100% = 200 pixels.

        // Check if this level is under the minimum limit expected...
        if((iCheckIfHigher == 0) && (fYield < fAlarmYield))
        {
            strChartImage = GEXMO_RED_BAR;			// RED bar.
            strBackgroundColor = GEXMO_FAIL_YIELD;	// RED background
        }
        else
            if((iCheckIfHigher == 1) && (fYield > fAlarmYield))
            {
                strChartImage = GEXMO_RED_BAR;			// RED bar.
                strBackgroundColor = GEXMO_FAIL_YIELD;	// RED background
            }
            else
            {
                strChartImage = GEXMO_GREEN_BAR;		// GREEN - Pass bar.
                strBackgroundColor = GEXMO_PASS_YIELD;	// YELLOW background
            }

        if(qslMonitoringProductPageOptionList.contains(QString("total_parts")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lTotal);
        if(qslMonitoringProductPageOptionList.contains(QString("total_good")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lGood);
        if(qslMonitoringProductPageOptionList.contains(QString("total_fail")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lTotal-lGood);
        if(qslMonitoringProductPageOptionList.contains(QString("yield")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"%s\">%s</td>\n",strBackgroundColor.toLatin1().constData(), strYield.toLatin1().constData());
        if(qslMonitoringProductPageOptionList.contains(QString("alarm_limit")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",strAlarmLevel.toLatin1().constData());
        if(qslMonitoringProductPageOptionList.contains(QString("yield_chart")))
        {
            fprintf(m_hReportPage,"<td align=\"left\" bgColor=\"#F8F8F8\">\n");
            fprintf(m_hReportPage,"<img border=\"1\" src=\"../../%s/%s\" width=\"%d\" height=\"9\"></td>\n",GEXMO_AUTOREPORT_IMAGES_DEST,strChartImage.toLatin1().constData(), iYieldBarSize);
        }

        fprintf(m_hReportPage,"</tr>\n");

        // Next product
        nIndex++;
    };

    // Close table
    fprintf(m_hReportPage,"</table>\n");

    // Clean close of the report page. (write history links + footer + close).
    CloseReportPage(GEXMO_REPORTPAGE_PRODUCTS,true,GEXMO_REPORTPAGE_PRODUCTS);

    // Read list with 'Product' name in Ascending order
    // Create: each product CHILD page of page 'PRODUCTS'
    pLotProcessed=pNextLot=NULL;
    nIndex=0;
    while(nIndex < m_lstGexMoLotsList.size())
    {
        pLotProcessed		= m_lstGexMoLotsList.at(nIndex);
        strCurrentProduct	= pLotProcessed->strProductID;

        fAlarmYield			= 0;	// Default is NO alarm level defined.
        iCheckIfHigher		= 0;	// Default: Check if yield under given limit.

        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            // Find Yield task matching our Product AND including a list of Good Bins
            ptYieldTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductYieldInfo(strCurrentProduct,strDatabaseName,"Final Test",true);
            if(ptYieldTask == NULL)
                ptYieldTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductYieldInfo(strCurrentProduct,strDatabaseName,"Wafer Sort",true);
            if(ptYieldTask == NULL)
                ptYieldTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductYieldInfo(strCurrentProduct,strDatabaseName,"E-Test",true);
            if(ptYieldTask != NULL)
            {
                iCheckIfHigher = ptYieldTask->GetProperties()->iAlarmIfOverLimit;
                fAlarmYield = ptYieldTask->GetProperties()->iAlarmLevel;
                strAlarmLevel = QString::number(fAlarmYield) + " %";
            }
            else
                strAlarmLevel = "-";
        }
        else
            strAlarmLevel = "-";

        // Write child-Page header: '/products/<productID/>' page
        if(!CreateReportPage(ptTask->GetProperties(),strWebChildFolder,strWebChildName,GEXMO_REPORTPAGE_PRODUCTS,strCurrentProduct))
            return;	// failed creating pages

        // Write table hader
        fprintf(m_hReportPage,"<table style=\"BORDER-COLLAPSE: collapse\" cellSpacing=\"0\" cellPadding=\"0\" width=\"90%%\" border=\"1\">\n");
        fprintf(m_hReportPage,"<tr>\n");
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Date</b></td>\n");
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Tester</b></td>\n");
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Lot</b></td>\n");
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>SubLot</b></td>\n");
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Wafer</b></td>\n");

        if(qslMonitoringProductPageOptionList.contains(QString("total_parts")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Total<br>Parts</b></td>\n");
        if(qslMonitoringProductPageOptionList.contains(QString("total_good")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Good</b></td>\n");
        if(qslMonitoringProductPageOptionList.contains(QString("total_fail")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Fail</b></td>\n");

        if(qslMonitoringProductPageOptionList.contains(QString("yield")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>SubLot<br>Yield</b></td>\n");
        if(qslMonitoringProductPageOptionList.contains(QString("alarm_limit")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Alarm<br>level</b></td>\n");
        if(qslMonitoringProductPageOptionList.contains(QString("yield_chart")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Yield Chart</b></td>\n");
        fprintf(m_hReportPage,"</tr>\n");

        do
        {
            lTotal	= pLotProcessed->lTotalParts;
            lGood	= pLotProcessed->lTotalGoodBins;

            // Write each Sub-lot line for this product.
            if(lTotal > 0)
            {
                fYield = (100.0*(float)lGood)/(float)lTotal;
                strYield = QString::number(fYield,'f',2) + " %";
            }
            else
            {
                strYield = "-";
                fYield = 0.0;
            }

            // Compute Yield bar size
            iYieldBarSize = (int)(2.0*fYield);	// 100% = 200 pixels.

            // Check if this level is under the minimum limit expected...
            if((iCheckIfHigher == 0) && (fYield < fAlarmYield))
            {
                strChartImage = GEXMO_RED_BAR;			// RED bar.
                strBackgroundColor = GEXMO_FAIL_YIELD;	// RED background
            }
            else
                if((iCheckIfHigher == 1) && (fYield > fAlarmYield))
                {
                    strChartImage = GEXMO_RED_BAR;			// RED bar.
                    strBackgroundColor = GEXMO_FAIL_YIELD;	// RED background
                }
                else
                {
                    strChartImage = GEXMO_GREEN_BAR;		// GREEN - Pass bar.
                    strBackgroundColor = GEXMO_PASS_YIELD;	// YELLOW background
                }

            strString = TimeStringUTC_F(pLotProcessed->tStartTime, "d-MMM-yyyy h:mm:ss");

            fprintf(m_hReportPage,"<tr>\n");

            // Sublot testing time.
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",strString.toLatin1().constData());

            // tester
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",pLotProcessed->strNodeName.toLatin1().constData());

            // lot
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",pLotProcessed->strLot.toLatin1().constData());

            // Sublot
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",pLotProcessed->strSubLot.toLatin1().constData());

            // Wafer
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n", (pLotProcessed->strWafer).toLatin1().constData());

            // Part count
            if(qslMonitoringProductPageOptionList.contains(QString("total_parts")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lTotal);

            // Good parts
            if(qslMonitoringProductPageOptionList.contains(QString("total_good")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lGood);

            // Failing parts
            if(qslMonitoringProductPageOptionList.contains(QString("total_fail")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lTotal-lGood);

            // Sublot Yield
            if(qslMonitoringProductPageOptionList.contains(QString("yield")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"%s\">%s</td>\n",strBackgroundColor.toLatin1().constData(), strYield.toLatin1().constData());

            // Alarm level
            if(qslMonitoringProductPageOptionList.contains(QString("alarm_limit")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",strAlarmLevel.toLatin1().constData());

            // Yield chart
            if(qslMonitoringProductPageOptionList.contains(QString("yield_chart")))
            {
                fprintf(m_hReportPage,"<td align=\"left\" width=\"200\" bgColor=\"#F8F8F8\">\n");
                fprintf(m_hReportPage,"<img border=\"1\" src=\"../../../%s/%s\" width=\"%d\" height=\"9\"></td>\n",GEXMO_AUTOREPORT_IMAGES_DEST,strChartImage.toLatin1().constData(),iYieldBarSize);
            }
            fprintf(m_hReportPage,"</tr>\n");

            // If not last item, check if next item is same product
            if((nIndex+1) < m_lstGexMoLotsList.size())
            {
                pNextLot=m_lstGexMoLotsList.at(nIndex+1);
                if(strCurrentProduct != pNextLot->strProductID)
                    break;

                // Go to next item (same product)
                nIndex++;
                pLotProcessed = pNextLot;
            }
            else
                break;
        }
        while(nIndex < m_lstGexMoLotsList.size());

        // Close table
        fprintf(m_hReportPage,"</table>\n");

        // Clean close of the sub-lot list report page. (write history links + footer + close).
        CloseReportPage(GEXMO_REPORTPAGE_PRODUCTS,false,GEXMO_REPORTPAGE_PRODUCTS);

        // Next product
        nIndex++;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    // TESTERS PAGE
    ////////////////////////////////////////////////////////////////////////////////////////
    // Create HTML pages: Testers pages
    // Force to sort again...but on "Tester" name this time
    qSort(m_lstGexMoLotsList.begin(), m_lstGexMoLotsList.end(), sortByTester);

    // Write Page header: 'Testers' page
    if(!CreateReportPage(ptTask->GetProperties(),strWebChildFolder,strWebChildName,GEXMO_REPORTPAGE_TESTERS))
        return;	// failed creating pages

    // Write table hader
    fprintf(m_hReportPage,"<table style=\"BORDER-COLLAPSE: collapse\" cellSpacing=\"0\" cellPadding=\"0\" width=\"90%%\" border=\"1\">\n");
    fprintf(m_hReportPage,"<tr>\n");
    fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Tester</b></td>\n");
    if(qslMonitoringTesterPageOptionList.contains(QString("total_parts")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Total<br>Parts</b></td>\n");
    if(qslMonitoringTesterPageOptionList.contains(QString("total_good")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Good</b></td>\n");
    if(qslMonitoringTesterPageOptionList.contains(QString("total_fail")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Fail</b></td>\n");
    if(qslMonitoringTesterPageOptionList.contains(QString("yield")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Yield<br>level</b></td>\n");
    if(qslMonitoringTesterPageOptionList.contains(QString("yield_chart")))
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Yield Chart</b></td>\n");
    fprintf(m_hReportPage,"</tr>\n");

    // Read list with 'Tester' name in Ascending order
    // Create: 'TESTERS' page
    QString strCurrentTester;

    nIndex=0;
    pLotProcessed=pNextLot=NULL;
    while(nIndex < m_lstGexMoLotsList.size())
    {
        pLotProcessed		= m_lstGexMoLotsList.at(nIndex);
        strCurrentTester	= pLotProcessed->strNodeName;
        lTotal				= pLotProcessed->lTotalParts;
        lGood				= pLotProcessed->lTotalGoodBins;
        bEntryFound			= true;

        while((nIndex+1) < m_lstGexMoLotsList.size())
        {
            pNextLot=m_lstGexMoLotsList.at(nIndex+1);
            if(strCurrentTester != pNextLot->strNodeName)
                break;
            // Cumulate all entries that belong to the same tester.
            lTotal		+= pNextLot->lTotalParts;
            lGood		+= pNextLot->lTotalGoodBins;

            // Move to next entry in sorted list.
            pLotProcessed	= pNextLot;
            nIndex++;
        };

        if(bEntryFound == false)
            break;	// reached end of list!

        // Keep track of total number of testers in use
        lTestersInUse++;

        // Write Tester line: one per tester
        strNormalizedString = strCurrentTester;
        CGexSystemUtils::NormalizePath(strNormalizedString);
        fprintf(m_hReportPage,"<tr>\n");
        fprintf(m_hReportPage,"<td align=\"middle\" bgColor=\"#CCECFF\"><b>\n");
        fprintf(m_hReportPage,"<a href=\"../testers/%s/t-%s.htm\">%s</a></b></td>",strNormalizedString.toLatin1().constData(), strDateString.toLatin1().constData(), strCurrentTester.toLatin1().constData());
        if(lTotal > 0)
        {
            fYield = (100.0*(float)lGood)/(float)lTotal;
            strYield = QString::number(fYield,'f',2) + " %";
        }
        else
        {
            fYield = 0.0;
            strYield = "-";
        }

        // Compute Yield bar size
        iYieldBarSize = (int)(2.0*fYield);	// 100% = 200 pixels.

        // Check if this level is under the minimum limit expected...
        if((iCheckIfHigher == 0) && (fYield < fAlarmYield))
        {
            strChartImage = GEXMO_RED_BAR;			// RED bar.
            strBackgroundColor = GEXMO_FAIL_YIELD;	// RED background
        }
        else
            if((iCheckIfHigher == 1) && (fYield > fAlarmYield))
            {
                strChartImage = GEXMO_RED_BAR;			// RED bar.
                strBackgroundColor = GEXMO_FAIL_YIELD;	// RED background
            }
            else
            {
                strChartImage = GEXMO_GREEN_BAR;		// GREEN - Pass bar.
                strBackgroundColor = GEXMO_PASS_YIELD;	// YELLOW background
            }

        if(qslMonitoringTesterPageOptionList.contains(QString("total_parts")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lTotal);
        if(qslMonitoringTesterPageOptionList.contains(QString("total_good")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lGood);
        if(qslMonitoringTesterPageOptionList.contains(QString("total_fail")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lTotal-lGood);
        if(qslMonitoringTesterPageOptionList.contains(QString("yield")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"%s\">%s</td>\n",strBackgroundColor.toLatin1().constData(), strYield.toLatin1().constData());
        if(qslMonitoringTesterPageOptionList.contains(QString("yield_chart")))
        {
            // Yield chart
            fprintf(m_hReportPage,"<td align=\"left\" bgColor=\"#F8F8F8\">\n");
            fprintf(m_hReportPage,"<img border=\"1\" src=\"../../%s/%s\" width=\"%d\" height=\"9\"></td>\n",GEXMO_AUTOREPORT_IMAGES_DEST,strChartImage.toLatin1().constData(), iYieldBarSize);
        }

        fprintf(m_hReportPage,"</tr>\n");

        // Next tester
        nIndex++;
    };

    // Close table
    fprintf(m_hReportPage,"</table>\n");

    // Clean close of the report page. (write History links + footer + close).
    CloseReportPage(GEXMO_REPORTPAGE_TESTERS,true,GEXMO_REPORTPAGE_TESTERS);

    // Read list with 'Testers' name in Ascending order
    // Create: each tester CHILD page of page 'TESTERS'
    pLotProcessed=pNextLot=NULL;
    nIndex=0;
    while(nIndex < m_lstGexMoLotsList.size())
    {
        pLotProcessed		= m_lstGexMoLotsList.at(nIndex);
        strCurrentTester	= pLotProcessed->strNodeName;

        // Write child-Page header: '/products/<productID/>' page
        if(!CreateReportPage(ptTask->GetProperties(),strWebChildFolder,strWebChildName,GEXMO_REPORTPAGE_TESTERS,strCurrentTester))
            return;	// failed creating pages

        // Write table hader
        fprintf(m_hReportPage,"<table style=\"BORDER-COLLAPSE: collapse\" cellSpacing=\"0\" cellPadding=\"0\" width=\"90%%\" border=\"1\">\n");
        fprintf(m_hReportPage,"<tr>\n");
        fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Date</b></td>\n");
        if(qslMonitoringTesterPageOptionList.contains(QString("product")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Product</b></td>\n");
        if(qslMonitoringTesterPageOptionList.contains(QString("operator")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Operator</b></td>\n");
        if(qslMonitoringTesterPageOptionList.contains(QString("program_name")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Program</b></td>\n");
        if(qslMonitoringTesterPageOptionList.contains(QString("total_parts")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Total<br>Parts</b></td>\n");
        if(qslMonitoringTesterPageOptionList.contains(QString("total_good")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Good</b></td>\n");
        if(qslMonitoringTesterPageOptionList.contains(QString("total_fail")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Fail</b></td>\n");
        if(qslMonitoringTesterPageOptionList.contains(QString("yield")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Yield<br>level</b></td>\n");
        if(qslMonitoringTesterPageOptionList.contains(QString("alarm_limit")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Alarm<br>level</b></td>\n");
        if(qslMonitoringTesterPageOptionList.contains(QString("yield_chart")))
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Yield Chart</b></td>\n");
        fprintf(m_hReportPage,"</tr>\n");

        do
        {
            lTotal	= pLotProcessed->lTotalParts;
            lGood	= pLotProcessed->lTotalGoodBins;

            // Write each Sub-lot line for this product.
            if(lTotal > 0)
            {
                fYield = (100.0*(float)lGood)/(float)lTotal;
                strYield = QString::number(fYield,'f',2) + " %";
            }
            else
            {
                strYield = "-";
                fYield = 0.0;
            }

            // Compute Yield alarm level
            fAlarmYield = 0;	// Default is NO alarm level defined.
            iCheckIfHigher = 0;	// Default: Check if yield under given limit.
            if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            {
                // Find Yield task matching our Product AND including a list of Good Bins
                ptYieldTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductYieldInfo(strCurrentProduct,strDatabaseName,"Final Test",true);
                if(ptYieldTask == NULL)
                    ptYieldTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductYieldInfo(strCurrentProduct,strDatabaseName,"Wafer Sort",true);
                if(ptYieldTask == NULL)
                    ptYieldTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetProductYieldInfo(strCurrentProduct,strDatabaseName,"E-Test",true);
                if(ptYieldTask != NULL)
                {
                    fAlarmYield = ptYieldTask->GetProperties()->iAlarmLevel;
                    strAlarmLevel = QString::number(fAlarmYield) + " %";
                }
                else
                    strAlarmLevel = "-";
            }
            else
                strAlarmLevel = "-";

            // Compute Yield bar size
            iYieldBarSize = (int)(2.0*fYield);	// 100% = 200 pixels.

            // Check if this level is under the minimum limit expected...
            if(fYield < fAlarmYield)
            {
                strChartImage = GEXMO_RED_BAR;			// RED bar.
                strBackgroundColor = GEXMO_FAIL_YIELD;	// RED background
            }
            else
            {
                strChartImage = GEXMO_GREEN_BAR;		// GREEN - Pass bar.
                strBackgroundColor = GEXMO_PASS_YIELD;	// YELLOW background
            }

            strString = TimeStringUTC_F(pLotProcessed->tStartTime, "d-MMM-yyyy h:mm:ss");

            fprintf(m_hReportPage,"<tr>\n");
            // Sublot testing time.
            fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",strString.toLatin1().constData());
            if(qslMonitoringTesterPageOptionList.contains(QString("product")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",pLotProcessed->strProductID.toLatin1().constData());
            if(qslMonitoringTesterPageOptionList.contains(QString("operator")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",pLotProcessed->strOperator.toLatin1().constData());
            if(qslMonitoringTesterPageOptionList.contains(QString("program_name")))
            {
                // Job name + rev.
                strString = pLotProcessed->strJobName + " " + pLotProcessed->strJobRev;
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",strString.toLatin1().constData());
            }
            if(qslMonitoringTesterPageOptionList.contains(QString("total_parts")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lTotal);
            if(qslMonitoringTesterPageOptionList.contains(QString("total_good")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lGood);
            if(qslMonitoringTesterPageOptionList.contains(QString("total_fail")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%ld</td>\n",lTotal-lGood);
            if(qslMonitoringTesterPageOptionList.contains(QString("yield")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"%s\">%s</td>\n",strBackgroundColor.toLatin1().constData(), strYield.toLatin1().constData());
            if(qslMonitoringTesterPageOptionList.contains(QString("alarm_limit")))
                fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#F8F8F8\">%s</td>\n",strAlarmLevel.toLatin1().constData());
            if(qslMonitoringTesterPageOptionList.contains(QString("yield_chart")))
            {
                // Yield chart
                fprintf(m_hReportPage,"<td align=\"left\" width=\"200\" bgColor=\"#F8F8F8\">\n");
                fprintf(m_hReportPage,"<img border=\"1\" src=\"../../../%s/%s\" width=\"%d\" height=\"9\"></td>\n",GEXMO_AUTOREPORT_IMAGES_DEST,strChartImage.toLatin1().constData(), iYieldBarSize);
            }
            fprintf(m_hReportPage,"</tr>\n");

            // If not last item, check if next item is same tester
            if((nIndex+1) < m_lstGexMoLotsList.size())
            {
                pNextLot=m_lstGexMoLotsList.at(nIndex+1);
                if(strCurrentTester != pNextLot->strNodeName)
                    break;

                // Go to next item (same tester)
                nIndex++;
                pLotProcessed = pNextLot;
            }
            else
                break;
        }
        while(nIndex < m_lstGexMoLotsList.size());

        // Close table
        fprintf(m_hReportPage,"</table>\n");

        // Clean close of the sub-lot list report page. (write history links + footer + close).
        CloseReportPage(GEXMO_REPORTPAGE_TESTERS,false,GEXMO_REPORTPAGE_TESTERS);

        // Next tester
        nIndex++;
    };

    ////////////////////////////////////////////////////////////////////////////////////////
    // HOME PAGE
    ////////////////////////////////////////////////////////////////////////////////////////
    // Write Page header: 'Home' page
    if(!CreateReportPage(ptTask->GetProperties(),strWebChildFolder,strWebChildName,GEXMO_REPORTPAGE_HOME))
        return;	// failed creating pages

    // Write table hader
    fprintf(m_hReportPage,"<blockquote>\n");
    fprintf(m_hReportPage,"<table style=\"BORDER-COLLAPSE: collapse\" cellSpacing=\"0\" cellPadding=\"0\" width=\"50%%\" border=\"1\">\n");
    fprintf(m_hReportPage,"<tr>\n");
    fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Status Summary</b></td>\n");
    fprintf(m_hReportPage,"<td align=\"center\" bgColor=\"#CCECFF\"><b>Total</b></td>\n");
    fprintf(m_hReportPage,"</tr>\n");

    fprintf(m_hReportPage,"<tr>\n");
    fprintf(m_hReportPage,"<td> <a href=\"products.htm\">Products under test today</a></td>\n");
    fprintf(m_hReportPage,"<td align=\"center\">%ld</td>\n",lTotalProductsTested);
    fprintf(m_hReportPage,"</tr>\n");

    fprintf(m_hReportPage,"<tr>\n");
    fprintf(m_hReportPage,"<td> <a href=\"testers.htm\">Testers in operation</a></td>\n");
    fprintf(m_hReportPage,"<td align=\"center\">%ld</td>\n",lTestersInUse);
    fprintf(m_hReportPage,"</tr>\n");

    fprintf(m_hReportPage,"<tr>\n");
    fprintf(m_hReportPage,"<td><font color=\"#006699\">Data files processed today</font></td>\n");
    fprintf(m_hReportPage,"<td align=\"center\">%d</td>\n", m_lstGexMoLotsList.count());
    fprintf(m_hReportPage,"</tr>\n");

    // Close table
    fprintf(m_hReportPage,"</table>\n");
    fprintf(m_hReportPage,"</blockquote>\n");

    // If multipe child-sites (one per database), add hyperlink to global master home page over all databases
    if(ptTask->GetProperties()->isOneWebPerDatabase())
    {
        QString strImagesPath  = "../../";	// Creating Indexes pages in <WebSite_rootPath>
        strImagesPath += GEXMO_AUTOREPORT_IMAGES_DEST;
        fprintf(m_hReportPage,"<p>&nbsp;&nbsp;&nbsp;&nbsp;<img src=\"%s/rarrow.png\" border=\"0\">",strImagesPath.toLatin1().constData());
        fprintf(m_hReportPage,"<a href=\"../../../%s\"><font color=\"#006699\">&nbsp;Return to Master home page</font></a></p>\n",ptTask->GetProperties()->homePage().toLatin1().constData());
    }

    // Clean close of the report page. (write footer + close).
    CloseReportPage(GEXMO_REPORTPAGE_HOME,true,GEXMO_REPORTPAGE_NOHISTORY);
}

