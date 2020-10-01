///////////////////////////////////////////////////////////
// Database transactions I/O handling
///////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#endif

// QT
#include <QRegExp>
#include <QSqlError>
#include <QApplication>
#include <QHostAddress>
#include <QListIterator>
#include <QProgressBar>

#include <gqtl_sysutils.h>

// Galaxy modules includes
#include "stdfrecords_v4.h"
#include "gqtl_datakeys.h"
#include "db_keys_editor.h"

#include "gex_shared.h"
#include "admin_engine.h"
#include "cbinning.h"
#include "db_datakeys_dialog.h"
#include "db_transactions.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "db_engine.h"
#include "gex_database_filter.h"
#include "cpart_info.h"
#include "import_csv.h"
#include "import_constants.h"
#include "patman_lib.h"
#include "gex_database_entry.h"
#include "engine.h"
#include "message.h"
#include "admin_gui.h"

// Monitoring classes.
#include "mo_email.h"
#include "datapump/datapump_taskdata.h"
#include "yield/yield_taskdata.h"
#include "status/status_taskdata.h"
#include "scheduler_engine.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "mo_y123.h"
#include "mo_task.h"
#include "datapump/datapump_task.h"
#include "gexmo_constants.h"
#include <gqtl_log.h>
#include "product_info.h"

// Support of Compressed files.
#include <gqtl_archivefile.h>

/////// FILE LOCK
#include <stdio.h>
#include <stdlib.h>
#if !defined __MACH__ && !defined unix
#include <io.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#if defined(unix) || __MACH__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#elif defined(WIN32)
#include <windows.h>
#include <io.h>
#include <sys/locking.h>
#endif

#if defined(WIN32)
#	define GFILELOCK_PERMISSIONS      S_IWRITE | S_IREAD
#elif defined(unix) || __MACH__
#	define GFILELOCK_PERMISSIONS      S_IWUSR | S_IRUSR
#endif
#define GFILELOCK_INVALID_HANDLE      -1
#define GFILELOCK_LOCKERROR           -1

////////
// main.cpp
extern GexMainwindow *            pGexMainWindow;
extern QProgressBar	*             GexProgressBar;         // Handle to progress bar in status bar

// report_build.cpp
extern CGexReport *               gexReport;              // Handle to report class
extern CReportOptions             ReportOptions;          // Holds options (report_build.h)

// Chtmlpng.cpp
extern double       ScalingPower(int iPower);

namespace GS
{

namespace Gex
{

// Error map
GBEGIN_ERROR_MAP(DatabaseEngine)
GMAP_ERROR_EX(eWrongLicense,
              "Insertion not allowed with PAT-Man or GTM - No data file inserted.",
              "Wrong license")
GMAP_ERROR_EX(eDatabaseNotFound,
              "Failed to find database (invalid path or missing database) - No data file inserted.",
              "Couldn't find database.")
GMAP_ERROR_EX(eReadOnlyDatabase,
              "Data insertion rejected for database '%s' (READ ONLY database) - No data file inserted.",
              "READ ONLY database.")
GMAP_ERROR_EX(eFileNotFound,
              "File not exists '%s' (invalid path or missing database) - No data file inserted.",
              "Couldn't find file.")
GMAP_ERROR_EX(eFailedToCreateUnzipFolder,
              "Failed to create unzip folder\n\tFolder to create: %s\n\tFile to process : %s",
              "Unable to create unzip folder.")
GMAP_ERROR_EX(eFailedToUnzipFile,
              "Failed to unzip file\n\tUnzip to folder: %s\n\tFile to unzip  : %s\n\t%s",
              "Failed to unzip file.")
GMAP_ERROR_EX(eFailedToConvertFile,
              "Failed parsing data file.\n\tFile : %s\n\tDetails: %s",
              "Failed parsing data file.")
GMAP_ERROR_EX(eMultipleFilesInArchive,
              "Multiple files compressed in archive\n\tArchive: %s\n\tIt is not supported to insert an archive with multiple files in it.",
              "Multiple files compressed in archive.")
GMAP_ERROR_EX(eFailedToCopyFile,
              "Failed to copy file.\n\tSource: %s\n\tDest. : %s",
              "Failed to copy file.")
GMAP_ERROR_EX(eFailedToLoadDataKeys,
              "Failed to load DataKeys with file.\n\tFile   : %s\n\tDetails: %s",
              "Failed to load DataKeys.")
GMAP_ERROR_EX(eFailedToLoadConfigKeysFile,
              "Failed to load/evaluate configkeys for file.\n\tFile   : %s\n\tDetails: %s",
              "Failed to load/evaluate configkeys file.")
GMAP_ERROR_EX(eFailedToExecPreInsertionJs,
              "Failed to execute pre-insertion JavaScript.\n\tDetails: %s",
              "Failed to execute pre-insertion JavaScript.")
GMAP_ERROR_EX(eErrorInPreInsertionJs,
              "Error in pre-insertion JavaScript.\n\tDetails: %s",
              "Error in pre-insertion JavaScript.")
GMAP_ERROR_EX(ePreInsertionJsReject,
              "Pre-insertion JavaScript rejected/delayed the file.\n\tDetails: %s",
              "Pre-insertion JavaScript rejected/delayed the file.")
GMAP_ERROR_EX(eConfigkeysSyntaxError,
              "Syntax error in configkeys file.\n\tConfigkeys file: %s\n\tSTDF File      : %s\n\tSource         : %s\n\tError          : %s",
              "Syntax error in configkeys file.")
GMAP_ERROR_EX(eStdfCorrupted,
              "STDF file corrupted.\n\tSTDF File: %s\n\tSource   : %s\n\tDetails  : %s",
              "STDF file corrupted.")
GMAP_ERROR_EX(eTooFewParts,
              "File has too few parts.\n\tSTDF File: %s\n\tSource   : %s\n\tDetails  : %s\n\tInfo     : check corresponding setting in your DataPump task.",
              "File has too few parts.")
GMAP_ERROR_EX(eUnexpectedPassHardBins,
              "File contains PASS Hard bins not part of specified list.\n\tSTDF File: %s\n\tSource   : %s\n\tDetails  : %s\n\tInfo     : check corresponding setting in your DataPump task.",
              "File contains PASS Hard bins not part of specified list.")
GMAP_ERROR_EX(eTdrInsertionFail,
              "TDR insertion with Fail status.\n\tSTDF File: %s\n\tSource   : %s\n\tDetails  : %s",
              "TDR insertion with Fail status.")
GMAP_ERROR_EX(eTdrInsertionDelay,
              "TDR insertion with Delay status.\n\tSTDF File: %s\n\tSource   : %s\n\tDetails  : %s",
              "TDR insertion with Delay status.")
GMAP_ERROR_EX(eFailedToAnalyzeFile,
              "Failed to analyze file.\n\tSTDF File: %s\n\tSource   : %s\n\tDetails  : %s\n\tPossible cause: script exception / invalid script keyword or parameter",
              "Failed to analyze file.")
GMAP_ERROR_EX(eFailedToCreateScriptFile,
              "Failed to create script file: %s",
              "Failed to create script file.")
GMAP_ERROR_EX(eFailedToWriteScriptFunction,
              "Failed to write %s section into file: %s",
              "Failed to write options section.")
GMAP_ERROR_EX(eFailedToReadIndexFile,
              "Failed to read index file: %s",
              "Failed to read index file.")
GMAP_ERROR_EX(eFailedToCreateIndexFile,
              "Failed to create index file: %s",
              "Failed to create index file.")
GMAP_ERROR_EX(eFailedToCreateEmail,
              "Failed to create email : no StatusTask defined.",
              "Failed to create email.")
GMAP_ERROR_EX(eIncompleteFile,
              "Insertion failed: Incomplete file (no STDF.MRR record found).\n\tFile: %s\n\tSource: %s\n\tInfo: To accept such files, toggle Monitoring 'Options/Data processing options/Handling STDF compliancy' to 'Flexible'",
              "Incomplete file.")
GMAP_ERROR_EX(eFailedToMoveFile,
              "Failed to move file.\n\tSource: %s\n\tDest. : %s",
              "Failed to move file.")
GMAP_ERROR_EX(eFailedToCompressFile,
              "Failed to compress file\n\tSource: %s\n\tDest. : %s",
              "Failed to compress file.")
GEND_ERROR_MAP(DatabaseEngine)

///////////////////////////////////////////////////////////
// Constructor: Class for Database Transactions.
///////////////////////////////////////////////////////////
DatabaseEngine::DatabaseEngine(QObject* parent):QObject(parent)
{
    setObjectName("GSDatabaseEngine");
    mAllowed = false;

    // Lowest file index not used yet. Used to create unique file name in the database
    // format is : fyymmddhhmmss_<index>
    iDatabaseFileIndex    =0;
    m_nIndexLocalDatabase = -1;
    m_bDatabasesLoaded    = false;

}

///////////////////////////////////////////////////////////
// Destructor: Class for Database Transactions.
///////////////////////////////////////////////////////////
DatabaseEngine::~DatabaseEngine()
{
    // Empty list and free ressources
    while(!mDatabaseEntries.isEmpty())
    {
        GexDatabaseEntry* lDataBase = mDatabaseEntries.takeFirst();
        if (lDataBase)
            delete lDataBase;
    }
}

///////////////////////////////////////////////////////////
// Build a string that shows the databse size (eg: 14.5 MB)
// fSize is in Mo
///////////////////////////////////////////////////////////
QString DatabaseEngine::BuildDatabaseSize(double fSize,bool bRemoteDatabase)
{
    QString strSize;

    // fSize is in Mo
    double fOneByte = (1.0/1024.0/1024.0);
    double fOneMByte = 1.0;
    double fOneGByte = 1024.0;

    if(fSize <= fOneByte)
        strSize = "Empty";
    else if(fSize < fOneMByte)
        strSize = strSize.sprintf("%.1f KB",fSize * 1024.0);
    else if(fSize < fOneGByte)
        strSize = strSize.sprintf("%.2f MB",fSize);
    else
        strSize = strSize.sprintf("%.2f GB",fSize / 1024.0);

    // If remote database, then this size is the local cache size, not the real database size!
    if(bRemoteDatabase)
        strSize += " (local Cache)";

    return strSize;
}


int DatabaseEngine::LoadDatabasesListIfEmpty(bool bForceReload/*=false*/)
{
    if(!mAllowed) return 0;

    // Get number of databases listed
    int	iCount = mDatabaseEntries.count();

    // None, or force reload?...then rescan disk!
    if(bForceReload || iCount <= 0)
        iCount = ReloadDatabasesList(ReportOptions.GetLocalDatabaseFolder(),
                                     ReportOptions.GetServerDatabaseFolder(true));

    // Return total databases present on disk.
    return iCount;
}


void DatabaseEngine::LoadDatabaseEntries()
{
    if(!mAllowed) return;

    GSLOG(SYSLOG_SEV_DEBUG, QString("Scan databases : %1 db entries already in cache.")
          .arg(mDatabaseEntries.count())
          .toLatin1().data() );
    // Have to wait the connection with GexServeur
    if((GS::Gex::Engine::GetInstance().GetClientState() != GS::Gex::Engine::eState_LicenseGranted)
            && (GS::Gex::Engine::GetInstance().GetClientState() != GS::Gex::Engine::eState_NodeReady))
    {
        GSLOG(SYSLOG_SEV_DEBUG, " Client State not ready.");
        return;
    }

    // Do not reload database if have task running
    if(Engine::GetInstance().HasTasksRunning())
        return;

    // Load DBs from Admin DB
    LoadDatabaseEntriesFromAdminDb();

    QString lLocalDbFolder = ReportOptions.GetLocalDatabaseFolder();
    QString lServerDbFolder = ReportOptions.GetServerDatabaseFolder(true);
    // Load DBs from [Local] database folder
    LoadDatabaseEntriesFromFiles(lLocalDbFolder);

    // If local and Server folders are the same, do not read twice the path!
    if(lLocalDbFolder != lServerDbFolder)
        // Load DBs from [Server] database folder
        LoadDatabaseEntriesFromFiles(lServerDbFolder);

    // Delete my first Galaxy Black-Hole name
    if(mDatabaseEntries.count() > 0)
        DeleteDatabaseEntry("Galaxy Black-Hole");

}


void DatabaseEngine::LoadDatabaseEntriesFromAdminDb()
{
    if(!mAllowed) return;

    GSLOG(SYSLOG_SEV_DEBUG, QString("Scan databases from admin DB: %1 db entries already in cache.")
          .arg(mDatabaseEntries.count())
          .toLatin1().data() );

    if (!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        return ;

    // Check if yieldman is connected
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
    {
        GSLOG(SYSLOG_SEV_DEBUG, " connected with YieldManDB.");
        return;
    }

    // Read database entry from the YieldManDatabase table
    // If YieldMan node, only load accredited databases
    QString       strQuery, ymDatabasesPath;
    QSqlQuery     clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    GexDatabaseEntry *pDatabaseEntry;

    // Check if have Internal BlackHole into ym_databases
    // For first load and with YM
    if((mDatabaseEntries.count() == 0) && GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        strQuery = "SELECT * FROM ym_databases WHERE name='"
                +GS::Gex::Engine::GetInstance().GetAdminEngine().GetDatabaseGalaxyBlackHoleName()+"'";
        if(clQuery.exec(strQuery))
        {
            if(!clQuery.first())
            {
                // Auto insert the INTERNAL BLACKHOLE
                GexDatabaseEntry cEntry;
                cEntry.SetBlackHole(true);
                cEntry.SetLogicalName(GS::Gex::Engine::GetInstance().GetAdminEngine().GetDatabaseGalaxyBlackHoleName());
                cEntry.SetDescription("Internal Quantix Black-Hole");
                GS::Gex::Engine::GetInstance().GetAdminEngine().SaveDatabase(&cEntry);
            }
        }
    }

    // Extract the list of database ID
    // Start with the type "adr" then "ym_prod_tdr"
    strQuery = "SELECT DB.database_id, DB.name, DB.description ";
    strQuery+= " FROM ym_databases DB";
    strQuery+= " ORDER BY DB.type,DB.database_id";

    if(clQuery.exec(strQuery))
    {

        while(clQuery.next())
        {
            UpdateStatusMessage("Loading database "+clQuery.value(1).toString()+"...");
            QCoreApplication::processEvents();
            // Load databases from YieldManDb
            pDatabaseEntry = NULL;

            // Check if entry already in the list. If so, update existing entry, else append new entry to the list.
            if(mDatabaseEntries.count() > 0)
                pDatabaseEntry = FindDatabaseEntry(clQuery.value(0).toInt());

            // Get option
            // If empty, create it and save
            if(ymDatabasesPath.isEmpty())
                ymDatabasesPath = GS::Gex::Engine::GetInstance().GetAdminEngine().GetDatabaseFolder();

            if(pDatabaseEntry == NULL)
            {
                GexDatabaseEntry * pNewDatabaseEntry = new GexDatabaseEntry(this);
                QDomElement domElt = GS::Gex::Engine::GetInstance().GetAdminEngine().GetDatabaseEntryDomElt(clQuery.value(0).toInt());
                if (pNewDatabaseEntry->LoadDomElt(ymDatabasesPath, domElt))
                {
                    if(pNewDatabaseEntry->GetPluginID())
                        pNewDatabaseEntry->GetPluginID()->setConfigured(true);
                    QString settingFile = pNewDatabaseEntry->PhysicalPath() +
                            QDir::separator() + QString(DB_INI_FILE);
                    pNewDatabaseEntry->SaveToXmlFile(settingFile);
                    mDatabaseEntries.append(pNewDatabaseEntry);
                }
                else
                {
                    delete pNewDatabaseEntry;
                    pNewDatabaseEntry = NULL;
                }
            }
            else
            {
                // Get the uploaded id if the original comes from Local Folder
                // Reload the database
                QDomElement domElt = GS::Gex::Engine::GetInstance().GetAdminEngine().GetDatabaseEntryDomElt(pDatabaseEntry->m_nDatabaseId);
                if (pDatabaseEntry->LoadDomElt(ymDatabasesPath, domElt))
                    // This database still exists
                    // Do not remove
                    pDatabaseEntry->UnsetStatusFlags(STATUS_INTERNAL_REMOVE);
            }
        }
    }
}

void DatabaseEngine::LoadDatabaseEntriesFromFiles(QString strDatabasesPath)
{
    if(!mAllowed) return;

    GSLOG(SYSLOG_SEV_DEBUG, QString("Scan %1: %2 db entries already in cache.")
          .arg(strDatabasesPath)
          .arg(mDatabaseEntries.count())
          .toLatin1().data() );
    QString     dbFolderPath;

    // Read all the database folders to enumerate databases available.
    QDir             dDir;
    QStringList      strDatabaseEntries;
    bool             bIsDbEntryAdded, bIsDbUploaded;
    GexDatabaseEntry *pNewDatabaseEntry = NULL;
    GexDatabaseEntry *pDatabaseEntry = NULL;

    // Scan local path...
    dDir.setPath(strDatabasesPath);
    dDir.setFilter(QDir::NoDotAndDotDot|QDir::Dirs);
    strDatabaseEntries = dDir.entryList(QStringList() << "*");
    GSLOG(SYSLOG_SEV_DEBUG, QString(" %1 entries found in %2")
          .arg(strDatabaseEntries.count()).arg(strDatabasesPath)
          .toLatin1().data() );
    // Check all DB folders
    for ( QStringList::Iterator it = strDatabaseEntries.begin(); it != strDatabaseEntries.end(); ++it )
    {
        QCoreApplication::processEvents();
        bIsDbEntryAdded = false;
        // Build DB folder path
        dbFolderPath = strDatabasesPath + QDir::separator() + *it;

        pNewDatabaseEntry = new GexDatabaseEntry(this);
        // Try to load DB entry based on the xml file
        if (pNewDatabaseEntry->LoadFromDbFolder(dbFolderPath))
        {
            UpdateStatusMessage("Loading database "+pNewDatabaseEntry->LogicalName()+"...");
            pDatabaseEntry = NULL;
            if(mDatabaseEntries.count() > 0)
                pDatabaseEntry = FindDatabaseEntry(pNewDatabaseEntry->LogicalName(),false);

            // No DB found, append the new one!
            if (!pDatabaseEntry)
            {
                pNewDatabaseEntry->SetDbRestrictions(); // TODO TDR ensure is well defined
                mDatabaseEntries.append(pNewDatabaseEntry);
                bIsDbEntryAdded = true;
                bIsDbUploaded = false;

                // DB is defined in local folder + DB is not uploaded
                // If DB is external && mode is monitoring && admin db is defined
                if (pNewDatabaseEntry ->IsExternal() &&
                        GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() &&
                        GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
                {
                    // If DB is YM PROD then DB must be uploaded automatically
                    if(pNewDatabaseEntry->IsUnknownTdr())
                    {
                        // Connect only if needed
                        // Try to connect to corporate DB to check type
                        if (pNewDatabaseEntry->GetPluginID() &&
                                pNewDatabaseEntry->GetPluginID()->m_pPlugin)
                            pNewDatabaseEntry->GetPluginID()->m_pPlugin->ConnectToCorporateDb();
                    }
                    if(pNewDatabaseEntry->IsYmProdTdr()||pNewDatabaseEntry->IsAdr())
                    {
                        // Then if the DB type is YM TDR, try to upload the DB into admin db!
                        UploadDatabaseEntry(pNewDatabaseEntry);
                        if(pNewDatabaseEntry->IsStoredInDb())
                            bIsDbUploaded = true;
                    }
                }
                // DB not uploaded set a negative ID
                if (!bIsDbUploaded)
                    pNewDatabaseEntry->m_nDatabaseId = --m_nIndexLocalDatabase;
            }
            // DB already loaded
            else
            {
                // Clean the new DB folder if needed [Only with YieldManDb]
                if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                        && (pDatabaseEntry->PhysicalPath() != pNewDatabaseEntry->PhysicalPath()))
                    DeleteFolderContent(pNewDatabaseEntry->PhysicalPath());
                // Do not remove already loaded DB from the list, it still exists in records!
                pDatabaseEntry->UnsetStatusFlags(STATUS_INTERNAL_REMOVE);
            }
        }
        // Nothing to add, delete new DB loaded
        if (!bIsDbEntryAdded)
        {
            delete pNewDatabaseEntry;
            pNewDatabaseEntry = NULL;
        }
    }
}

///////////////////////////////////////////////////////////
// Update Databases list according to new databases paths.
///////////////////////////////////////////////////////////
int DatabaseEngine::ReloadDatabasesList(QString strLocalDatabasesPath,QString strServerDatabasesPath)
{
    if(!mAllowed) return 0;

    GSLOG(SYSLOG_SEV_DEBUG, QString(" localpath=%1 serverpath=%2").arg(
              strLocalDatabasesPath.toLatin1().data()).arg( strServerDatabasesPath.toLatin1().data()).toLatin1().constData());
    // Debug message
    QString strString = " reload ";
    strString += strLocalDatabasesPath;
    GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().data() );

    int		iDatabasesEntries = mDatabaseEntries.count();

    if(iDatabasesEntries > 0)
        emit sOnSelectDatabase(NULL);

    m_bDatabasesLoaded = false;
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().setTaskBeingEdited(true);

    // Disable the connect button during the load of databases
    emit sButtonConnectEnabled(false);
    emit sButtonReloadListEnabled(false);

    QString strMessage = "Loading databases...                         ";
    UpdateStatusMessage(strMessage);

    // Load database entries: local, server and admin DB
    LoadDatabaseEntries();

    ///////////////////////////////////////////////////////////

    // Then for each pDatabaseEntry, test the DB connection
    ///////////////////////////////////////////////////////////
    m_bDatabasesLoaded = true;

    CheckDatabasesList(true);

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().setTaskBeingEdited(false);

    emit sButtonConnectEnabled(true);
    emit sButtonReloadListEnabled(true);

    // Tasks are disabled when database not connected
    // Reload Tasks list to recheck task status
    if(!mDatabaseEntries.isEmpty())
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTasksList();

    UpdateStatusMessage("");

    // Total number of Databases entries in the LOCAL+SERVER paths
    iDatabasesEntries = mDatabaseEntries.count();

    if(iDatabasesEntries == 0)
        emit sShowDatabaseList(NULL);

    return iDatabasesEntries;

}

///////////////////////////////////////////////////////////
// Check Databases list - connection/access.
///////////////////////////////////////////////////////////
void DatabaseEngine::CheckDatabasesList(bool ForceUpdateGui)
{
    if(mDatabaseEntries.count() <= 0)
        return;

    bool                bFoundNewStatus = false;
    GexDatabaseEntry    *pDatabaseEntry;
    QList<GexDatabaseEntry*>::iterator itBegin	= mDatabaseEntries.begin();
    QList<GexDatabaseEntry*>::iterator itEnd	= mDatabaseEntries.end();
    int nProgessBar = 0;

    if(ForceUpdateGui && GexProgressBar)
    {
        GexProgressBar->show();
        GexProgressBar->reset();
        GexProgressBar->setMaximum(mDatabaseEntries.count()*2 + 1);
        GexProgressBar->setValue(nProgessBar++);
    }

    bool bDisplayPopUpWindows = false;
    QString strPopUpValue = ReportOptions.GetOption("databases","database_popup").toString().toLower();

    while(itBegin != itEnd)
    {
        if(ForceUpdateGui)
            UpdateStatusMessage("Loading databases...");

        pDatabaseEntry = (*itBegin);

        if(ForceUpdateGui && GexProgressBar)
            GexProgressBar->setValue(nProgessBar++);

        if(pDatabaseEntry->StatusFlags() & STATUS_INTERNAL_REMOVE)
        {
            // INTERNAL_REMOVE
            // Used when reload all databases when a database was deleted from another Gex
            // instead to delete all databases pointers
            // Delete database pointer only if necessary
            //delete pDatabaseEntry;
            //m_pDatabaseEntries.erase(itBegin);
        }
        // Never check ADR database which will be checked and connected by the TDR
        else //if(!pDatabaseEntry->IsAdr())
        {

            UINT StatusFlags = pDatabaseEntry->StatusFlags();
            bool lIsError = false;
            // Manual or Automatic connection
            // if the option is set to Manual
            // Do not connect to the database
            if(!(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)			// if not connected
                    && (pDatabaseEntry->m_pExternalDatabase
                        && !pDatabaseEntry->m_pExternalDatabase->IsAutomaticStartup()))	// and if not auto startup
            {
                // do not connect to this database
                pDatabaseEntry->SetStatusFlags(STATUS_MANUALLY_DISCONNECTED);
            }
            CheckDatabaseEntryStatus(pDatabaseEntry,!(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED));
            // Display Wizard for the first load
            if(!(pDatabaseEntry->StatusFlags() & STATUS_MANUALLY_DISCONNECTED))
            {
                // If not manual disconnection (forced by Application)
                // If not connected
                if(((!(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED))
                    || (!(pDatabaseEntry->IsUpToDate())))
                        && (GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                            && !GS::Gex::Engine::GetInstance().GetAdminEngine().m_bFirstActivation))
                    lIsError = true;
                // If Error connection - force to disconnect (then reload doesn't retest this db)
                if(!(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
                        && !pDatabaseEntry->m_strLastInfoMsg.isEmpty())
                {
                    // do not reconnect to this database
                    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
                        pDatabaseEntry->SetStatusFlags(STATUS_MANUALLY_DISCONNECTED);
                }
            }

            if(!ForceUpdateGui)
                bDisplayPopUpWindows = false;
            else
            {
                if(strPopUpValue == "never")
                    bDisplayPopUpWindows = false;
                else if(strPopUpValue == "always")
                    bDisplayPopUpWindows = true;
                else if(strPopUpValue == "onerror" && lIsError)
                    bDisplayPopUpWindows = true;
                else
                    bDisplayPopUpWindows = false;
            }

            if(StatusFlags != pDatabaseEntry->StatusFlags())
            {
                // Update line only if status changed
                bFoundNewStatus = true;
                emit sShowDatabaseList(pDatabaseEntry, bDisplayPopUpWindows);
            }
            else if(ForceUpdateGui)
                emit sShowDatabaseList(pDatabaseEntry);

        }

        // Move to next entry.
        itBegin++;

        if(ForceUpdateGui && GexProgressBar)
            GexProgressBar->setValue(nProgessBar++);

    }

    if(ForceUpdateGui && GexProgressBar)
        GexProgressBar->setValue(GexProgressBar->maximum());

    if(ForceUpdateGui)
        emit sOnSelectDatabase(NULL);;

    if(GexProgressBar)
        GexProgressBar->hide();

    UpdateStatusMessage("");

    if(bFoundNewStatus)
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTasksList();

}

bool    DatabaseEngine::CreateDatabaseEntry(GexDbPlugin_ID* currentPlugin, const QString& lLogicalName, QString &error)
{

    if((currentPlugin == NULL)
            || (currentPlugin->m_pPlugin == NULL)
            || (currentPlugin->m_pPlugin->m_pclDatabaseConnector == NULL))
    {
        error = "Invalid internal plugin";
        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, " cCreateDatabase.exec() ok");

    // Valid Data entered: try creating database entry
    GexDatabaseEntry    cEntry(0);
    QString             strStorage;
    bool                bStatus;


    // Check if a GexDb database not already referenced(plugin,host,port,shema,databse)
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        // If External database
        GexDatabaseEntry *pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(currentPlugin->pluginName(),
                                                                                                                currentPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name,
                                                                                                                currentPlugin->m_pPlugin->m_pclDatabaseConnector->m_uiPort,
                                                                                                                currentPlugin->m_pPlugin->m_pclDatabaseConnector->m_strDriver,
                                                                                                                currentPlugin->m_pPlugin->m_pclDatabaseConnector->m_strSchemaName,
                                                                                                                currentPlugin->m_pPlugin->m_pclDatabaseConnector->m_strDatabaseName);

        // Only one reference is allowed
        if(pDatabaseEntry)
        {
            error = "*Error* Database '"+currentPlugin->m_pPlugin->m_pclDatabaseConnector->m_strSchemaName;
            error+= "' already referenced\nUse this one '"+pDatabaseEntry->LogicalName() + "'!";
            return false;
        }
    }
    // Read Database creation info
    cEntry.SetLogicalName(lLogicalName);
    cEntry.m_strDatabaseRef = cEntry.LogicalName();
    cEntry.SetDescription(lLogicalName);
    cEntry.SetLocalDB(false);

    cEntry.SetCompressed(false);
    cEntry.SetHoldsFileCopy(false);
    cEntry.SetSummaryOnly(false);				// true if Database only holds the summary file
    cEntry.SetBlackHole(false);				// true if NO storage to do in the Database
    cEntry.SetExternal(false);
    cEntry.SetReadOnly(!(currentPlugin->m_pPlugin->IsInsertionSupported()));

    cEntry.SetHoldsFileCopy(true);
    cEntry.SetExternal(true);
    strStorage = "Corporate ";
    if(cEntry.IsReadOnly())
        strStorage += "(Read only)";
    else
        strStorage += "(Read+Write)";

    // For the DatabaseEntry creation
    // Will be removed
    cEntry.SetExternalDbPluginIDPTR(currentPlugin);

    //cEntry.setTdrTypeRecorded(QString(GEXDB_ADR_KEY));

    cEntry.IsUnknownTdr();
    // Request Database Transaction module to create Database Entry (will try to create new db folder in databases)
    cEntry.TraceUpdate("CREATE","START","Database creation");
    bStatus = GS::Gex::Engine::GetInstance().GetDatabaseEngine().SaveDatabaseEntry(&cEntry);
    cEntry.TraceUpdate("CREATE",(bStatus?"PASS":"FAIL"),(bStatus?"Created":cEntry.m_strLastInfoMsg));

    /// TODO remove : too dangerous !
    cEntry.SetExternalDbPluginIDPTR(NULL);

    if(bStatus == false)
    {
        // Failed creating the database entry...
        if(cEntry.m_strLastInfoMsg.isEmpty())
            error = "*Error* Failed creating database entry\nYou probably don't have write access to the database folder\nor this database already exists!";
        else
            error = "*Error* Failed creating database entry\n"+cEntry.m_strLastInfoMsg;
        return false;
    }

    // Only the Folder is created
    // Have to ReloadDatabaseList to have the new database in the list
    // Force Full reload of the database list from disk
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().LoadDatabasesListIfEmpty(true);

    return true;

}


///////////////////////////////////////////////////////////
// Create a Database entry.
///////////////////////////////////////////////////////////
bool DatabaseEngine::SaveDatabaseEntry(GexDatabaseEntry *pDatabaseEntry)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" %1 %2")
          .arg( pDatabaseEntry->Description().toLatin1().constData())
          .arg( pDatabaseEntry->PhysicalName().toLatin1().constData() )
          .toLatin1().constData());

    QString             strPath,strDatabaseFolder;
    int                 iStatus;

    // Initialises the list in case not done yet.
    LoadDatabasesListIfEmpty();

    pDatabaseEntry->SetCacheSize(0.0);
    pDatabaseEntry->m_strLastInfoMsg = "";

    QString lLocalDbFolder = ReportOptions.GetLocalDatabaseFolder();
    QString lServerDbFolder = ReportOptions.GetServerDatabaseFolder(true);

    // Create Database folder
    if(pDatabaseEntry->IsStoredInFolderLocal())
        strPath = lLocalDbFolder;
    else
        strPath = lServerDbFolder;

    if(pDatabaseEntry->PhysicalName().isEmpty())
    {
        // If physical name not defined yet, do it now!
        pDatabaseEntry->SetPhysicalName(pDatabaseEntry->LogicalName());
    }

    strDatabaseFolder = pDatabaseEntry->PhysicalName();

    QDir cDir;
    // Create databases folder first if not exist
    cDir.mkpath(strPath);

    strPath += "/";
    strPath += strDatabaseFolder;
    strPath = QDir::cleanPath(strPath);

    // Update Database physical path
    pDatabaseEntry->SetPhysicalPath(strPath);

    if(cDir.exists(strPath))
    {
        pDatabaseEntry->m_strLastInfoMsg = "Failed creating folder "+strPath;
        pDatabaseEntry->m_strLastInfoMsg+= "\nFolder already exists !";
        return false;	// Failed creating folder.
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString(" mkdir '%1' ...")
          .arg( strPath.toLatin1().constData())
          .toLatin1().constData());

#if defined unix || __MACH__
    iStatus = mkdir(strPath.toLatin1().constData(),0777);
#else
    iStatus = _mkdir(strPath.toLatin1().constData());
#endif

    if(iStatus)
    {
        pDatabaseEntry->m_strLastInfoMsg = "Failed creating folder "+strPath;
        return false;	// Failed creating folder.
    }

    strPath += QDir::separator() + QString(DB_INI_FILE);

    return pDatabaseEntry->SaveToXmlFile(strPath);
    //  return UpdateDatabaseEntry(strPath,pDatabaseEntry);
}

///////////////////////////////////////////////////////////
// Create a Database entry.
///////////////////////////////////////////////////////////
bool DatabaseEngine::CheckDatabaseEntryStatus(GexDatabaseEntry *pDatabaseEntry, bool GetDatabaseSize)
{
    QString strText = "checking DB entry : " + pDatabaseEntry->PhysicalName();
    GSLOG(SYSLOG_SEV_DEBUG, strText.toLatin1().constData() );

    UINT StatusFlags = pDatabaseEntry->StatusFlags();
    // Check if can connect the database - force disconnection
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsInRestrictedMode(pDatabaseEntry))
    {
        if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToRead(pDatabaseEntry))
        {
            pDatabaseEntry->UnsetStatusFlags(STATUS_CONNECTED);
            pDatabaseEntry->SetStatusFlags(STATUS_MANUALLY_DISCONNECTED);
            pDatabaseEntry->UnsetStatusFlags(STATUS_EDIT);
            pDatabaseEntry->UnsetStatusFlags(STATUS_INSERTION);
            pDatabaseEntry->UnsetStatusFlags(STATUS_REMOVE);
            pDatabaseEntry->UnsetStatusFlags(STATUS_HOUSEKEEPING);

            if(StatusFlags != pDatabaseEntry->StatusFlags())
            {
                GSLOG(SYSLOG_SEV_NOTICE, QString(" Restriction Mode for %1").arg(
                          pDatabaseEntry->LogicalName().toLatin1().constData() )
                      .toLatin1().constData());
                pDatabaseEntry->m_LastStatusUpdate =
                        pDatabaseEntry->m_LastStatusChecked = GS::Gex::Engine::GetInstance().GetServerDateTime();
            }
            return true;
        }
    }

    // Ignore non YM db in monitoring - force disconnection
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
            && (pDatabaseEntry->IsCharacTdr() || pDatabaseEntry->IsManualProdTdr())
            && pDatabaseEntry->m_pExternalDatabase)
    {
        pDatabaseEntry->UnsetStatusFlags(STATUS_CONNECTED);
        pDatabaseEntry->SetStatusFlags(STATUS_MANUALLY_DISCONNECTED);
        pDatabaseEntry->UnsetStatusFlags(STATUS_EDIT);
        pDatabaseEntry->UnsetStatusFlags(STATUS_INSERTION);
        pDatabaseEntry->UnsetStatusFlags(STATUS_REMOVE);
        pDatabaseEntry->UnsetStatusFlags(STATUS_HOUSEKEEPING);
        if(StatusFlags != pDatabaseEntry->StatusFlags())
        {
            GSLOG(SYSLOG_SEV_NOTICE, QString(" Ignore non YM db for %1").arg(
                      pDatabaseEntry->LogicalName().toLatin1().constData() )
                  .toLatin1().constData());
            pDatabaseEntry->m_LastStatusUpdate =
                    pDatabaseEntry->m_LastStatusChecked = GS::Gex::Engine::GetInstance().GetServerDateTime();
        }
        return true;
    }


    if(pDatabaseEntry->IsExternal() && pDatabaseEntry->m_pExternalDatabase)
    {
        GexDbPlugin_ID *pPlugin = pDatabaseEntry->GetPluginID();

        pDatabaseEntry->SetStatusFlags(STATUS_REMOVE);
        pDatabaseEntry->SetStatusFlags(STATUS_EDIT);

        if(pPlugin && pPlugin->m_pPlugin && pPlugin->m_pPlugin->m_pclDatabaseConnector)
        {

            // Case 5787 - Make YmAdminDb server mandatory for RDB
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            {
                // case 6095 - Allow creating a local link to a RDB under Examinator-Pro
                // If no YieldManAdminDb
                // Allow Examinator-PRO to create a link for a GexDB only in ReadOnly mode
                // pPlugin->m_pPlugin->SetAutomaticStartup(true);

                pDatabaseEntry->SetReadOnly(pDatabaseEntry->IsYmProdTdr());
                if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
                {
                    pDatabaseEntry->m_strLastInfoMsg = "Your installation doesn't support database plug-ins:";
                    pDatabaseEntry->m_strLastInfoMsg+= "- Plugin not supported -";
                    return true;
                }
            }
            else
                pDatabaseEntry->SetReadOnly( !pDatabaseEntry->m_pExternalDatabase->IsInsertionSupported());

            // Get Connection Time out from ReportOptions
            //pPlugin->m_pPlugin->m_nConnectionTimeOut = ReportOptions.GetOption("databases","database_timeout").toInt();

            if(pPlugin->pluginFileName().contains("plugin_galaxy")
                    && !pDatabaseEntry->IsReadOnly())
            {
                pDatabaseEntry->SetStatusFlags(STATUS_HOUSEKEEPING);
                if(!(pDatabaseEntry->IsUpToDate()))
                    pDatabaseEntry->UnsetStatusFlags(STATUS_INSERTION);
            }
            else
            {
                pDatabaseEntry->UnsetStatusFlags(STATUS_HOUSEKEEPING);
            }

            // Force to disconnect the DB if manually disconnected
            if(!(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
                    && (pDatabaseEntry->StatusFlags() & STATUS_MANUALLY_DISCONNECTED)
                    && pPlugin->m_pPlugin->m_pclDatabaseConnector->IsConnected())
                pPlugin->m_pPlugin->m_pclDatabaseConnector->Disconnect();

            pPlugin->m_pPlugin->m_strDBFolder = pDatabaseEntry->m_pExternalDatabase->m_strDatabasePhysicalPath = pDatabaseEntry->PhysicalPath();
            pDatabaseEntry->UnsetStatusFlags(STATUS_CONNECTED);

            // If database is DUPLICATED must be disconnected (ignore this DB for insertion)
            if(pDatabaseEntry->LogicalName() == pDatabaseEntry->m_strDatabaseRef)
            {
                // Connected or not
                if(!(pDatabaseEntry->StatusFlags() & STATUS_MANUALLY_DISCONNECTED) || pPlugin->m_pPlugin->m_pclDatabaseConnector->IsConnected())
                {
                    QString TdrTypeRecorded = pDatabaseEntry->TdrTypeRecorded();
                    // For Spider Monitoring server
                    QString lStorageEngineRecorded = pDatabaseEntry->StorageEngineRecorded();
                    QString lConnectLocalHost;
                    // For Spider Monitoring server and only for YM
                    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
                        GS::Gex::Engine::GetInstance().GetAdminEngine().GetSettingsValue("TDR_CONNECT_LOCALHOST",lConnectLocalHost);

                    // The StorageEngine is saved into the XML description
                    // The StorageEngine from the TDR cannot be empty
                    // The StorageEngine from the XML can be empty at the begining
                    // If the XML StorageEngine is empty, we need first to connect to the TDR
                    // to retrieve the one saved into the TDR
                    if(lStorageEngineRecorded.isEmpty()
                            && (lConnectLocalHost == "ENABLED"))
                    {
                        // Connect using the standard connection
                        // and check if it is not 'localhost' and 'spider'
                        if(pPlugin->m_pPlugin->ConnectToCorporateDb())
                        {
                            // Update the local XML
                            pDatabaseEntry->SetStorageEngine(pDatabaseEntry->m_pExternalDatabase->GetTdrStorageEngine());
                            // Retrieve the StorageEngine
                            lStorageEngineRecorded = pDatabaseEntry->StorageEngineRecorded();
                        }
                        // For Spider, the connection must be force to 'localhost'
                        // Disconnect the TDR is not 'localhost'
                        if((lStorageEngineRecorded.toLower() == "spider")
                                && GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                                && !pPlugin->m_pPlugin->m_pclDatabaseConnector->m_bIsLocalHost)
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->Disconnect();
                    }


                    // Check the connection
                    if(!pPlugin->m_pPlugin->m_pclDatabaseConnector->IsConnected()
                            || !pPlugin->m_pPlugin->m_pclDatabaseConnector->m_bAdminUser)
                    {
                        UpdateStatusMessage("Loading database "+pDatabaseEntry->LogicalName()+"...");
                        pPlugin->m_pPlugin->SetAdminLogin(true);

                        // For Spider Monitoring server and only for YM
                        // The StorageEngine = 'SPIDER'
                        if((lStorageEngineRecorded.toLower() == "spider")
                                && (lConnectLocalHost == "ENABLED"))
                        {
                            // Force to connect to the TDR using a local connection
                            // Save the original settings before to try localhost
                            QString lHosIP = pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_IP;
                            QString lHostName = pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name;
                            QString lHostUnresolved = pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Unresolved;
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_IP =
                                    pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name =
                                    pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Unresolved =
                                    pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_LastUsed =
                                    "localhost";

                            pPlugin->m_pPlugin->ConnectToCorporateDb();
                            // The m_strHost_LastUsed connect the last name used for a success connection
                            // Just keep this info and Restore original connection
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_IP = lHosIP;
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name = lHostName;
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Unresolved = lHostUnresolved;
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_LastUsed = "localhost";
                            // If failed, don't try with the original connection
                            if(!pPlugin->m_pPlugin->m_pclDatabaseConnector->IsConnected())
                            {
                                pDatabaseEntry->m_strLastInfoMsg= QString("Cannot connect to the SPIDER TDR using LOCALHOST for %1").arg(
                                            pDatabaseEntry->LogicalName().toLatin1().constData());
                                GSLOG(SYSLOG_SEV_NOTICE, pDatabaseEntry->m_strLastInfoMsg.toLatin1().constData());
                                pDatabaseEntry->m_LastStatusUpdate =
                                        pDatabaseEntry->m_LastStatusChecked = GS::Gex::Engine::GetInstance().GetServerDateTime();
                                UpdateStatusMessage("");
                                return true;
                            }
                            pDatabaseEntry->SaveToXmlFile(QString());
                        }
                        else
                        {
                            // Connect using the standard connection
                            if(pPlugin->m_pPlugin->ConnectToCorporateDb())
                            {
                                //pDatabaseEntry->SetStorageEngine(pDatabaseEntry->m_pExternalDatabase->GetTdrStorageEngine());
                                // Update the XML file
                                //pDatabaseEntry->SaveToXmlFile(QString());
                            }
                        }

                    }
                    if(pPlugin->m_pPlugin->m_pclDatabaseConnector->IsConnected())
                    {
                        // if da_galaxy not loaded for any reason
                        // need to check here the Restriction Mode
                        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsInRestrictedMode(pDatabaseEntry))
                        {
                            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToRead(pDatabaseEntry))
                            {
                                pDatabaseEntry->UnsetStatusFlags(STATUS_CONNECTED);
                                pDatabaseEntry->SetStatusFlags(STATUS_MANUALLY_DISCONNECTED);
                                pDatabaseEntry->UnsetStatusFlags(STATUS_EDIT);
                                pDatabaseEntry->UnsetStatusFlags(STATUS_INSERTION);
                                pDatabaseEntry->UnsetStatusFlags(STATUS_REMOVE);
                                pDatabaseEntry->UnsetStatusFlags(STATUS_HOUSEKEEPING);
                                pPlugin->m_pPlugin->m_pclDatabaseConnector->Disconnect();

                                if(StatusFlags != pDatabaseEntry->StatusFlags())
                                {
                                    GSLOG(SYSLOG_SEV_NOTICE, QString(" Restriction Mode for %1").arg(
                                              pDatabaseEntry->LogicalName().toLatin1().constData() )
                                          .toLatin1().constData());
                                    pDatabaseEntry->m_LastStatusUpdate =
                                            pDatabaseEntry->m_LastStatusChecked = GS::Gex::Engine::GetInstance().GetServerDateTime();
                                }
                                UpdateStatusMessage("");
                                QString settingFile = pDatabaseEntry->PhysicalPath() +
                                        QDir::separator() + QString(DB_INI_FILE);
                                // Overwrite the old XML file
                                pDatabaseEntry->SaveToXmlFile(settingFile);

                                return true;
                            }
                        }

                        pDatabaseEntry->SetStatusFlags(STATUS_CONNECTED);

                        pDatabaseEntry->m_strLastInfoMsg = "";

                        if(GetDatabaseSize)
                        {
                            int nSize;
                            pDatabaseEntry->SetDbSize(0.0);
                            if(pDatabaseEntry->m_pExternalDatabase->GetTotalSize(nSize))
                                pDatabaseEntry->SetDbSize((float) nSize);
                        }

                        pDatabaseEntry->UnsetStatusFlags(STATUS_DBCOMPATIBLE);
                        pDatabaseEntry->UnsetStatusFlags(STATUS_DBUPTODATE);
                        pDatabaseEntry->UnsetStatusFlags(STATUS_COMPRESS_PROTOCOL);
                        pDatabaseEntry->UnsetStatusFlags(STATUS_INSERTION);

                        bool bDbUpToDate;
                        QString strVersion, strValue;
                        UINT	nBuildVersion,nBuildSupported;

                        // Force to set Tdr
                        pDatabaseEntry->IsUnknownTdr();
                        bool lForExtractionOnly = false;
                        // If MANUAL/CHARAC TDR and GexPRO => Insertion is allowed
                        // If MANUAL/CHARAC TDR and Monitoring => NOT POSSIBLE
                        // If PROD TDR and GexPRO => Extraction only
                        // If PROD TDR and Monitoring  => Insertion is allowed
                        if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                            &&
                            (pDatabaseEntry->IsYmProdTdr() || pDatabaseEntry->IsAdr()))
                        {
                            lForExtractionOnly = true;
                        }

                        if(lForExtractionOnly)
                            pDatabaseEntry->m_pExternalDatabase->IsDbUpToDateForExtraction(&bDbUpToDate, strVersion, &nBuildVersion, strValue, &nBuildSupported);
                        else
                            pDatabaseEntry->m_pExternalDatabase->IsDbUpToDateForInsertion(&bDbUpToDate, strVersion, &nBuildVersion, strValue, &nBuildSupported);

                        pDatabaseEntry->SetDatabaseVersion(strVersion);

                        // Update StatusFlag
                        // Feature for new B70
                        // For TDR PRODUCTION database
                        // Check if the associated ADR exists and is UpToDate
                        // By default the ADR is up-to-date even if not needed

                        // TDR version is up-to-date or compatible
                        // Need to check if the ADR is up-to-date
                        if(bDbUpToDate && pDatabaseEntry->m_pExternalDatabase->MustHaveAdrLink())
                        {
                            GexDatabaseEntry* lAdrDatabaseEntry = NULL;

                            lAdrDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(pDatabaseEntry->m_pExternalDatabase->GetAdrLinkName(), false);

                            // search for a DB entry with the given name, connected or not
                            if(lAdrDatabaseEntry == NULL)
                            {
                                bDbUpToDate = false;
                                pDatabaseEntry->m_strLastInfoMsg = "No "+QString(GEXDB_ADR_NAME)+" associated";
                            }
                            else
                            {
                                // Store the ADR related information for further processing
                                GexDbPlugin_Connector* adminConnector = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector;
                                GexDbPlugin_Connector* tdrConnector = pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_pclDatabaseConnector;
                                GexDbPlugin_Connector* adrConnector = lAdrDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_pclDatabaseConnector;

                                tdrConnector->mAdminDbHostName = adminConnector->m_strHost_Name;
                                tdrConnector->mAdminDbPort = adminConnector->m_uiPort;
                                tdrConnector->mAdminDbDatabaseName = adminConnector->m_strDatabaseName;
                                tdrConnector->mAdminDbUser = adminConnector->m_strUserName_Admin;
                                tdrConnector->mAdminDbPwd = adminConnector->m_strPassword_Admin;
                                tdrConnector->m_linkedAdrHostName = adrConnector->m_strHost_Name;
                                tdrConnector->m_linkedAdrPort = adrConnector->m_uiPort;
                                tdrConnector->m_linkedAdrDatabaseName = adrConnector->m_strDatabaseName;
                                tdrConnector->m_linkedAdrUser = adrConnector->m_strUserName_Admin;
                                tdrConnector->m_linkedAdrPwd = adrConnector->m_strPassword_Admin;

                                // If TDR Connected
                                // Associated ADR must be OK too
                                // If ADR is not Connected => force the connection
                                // Check the connection
                                if(!adrConnector->IsConnected())
                                {
                                    // Force the connection
                                    // Connect using the standard connection
                                    if(lAdrDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->ConnectToCorporateDb())
                                    {
                                        lAdrDatabaseEntry->SetStatusFlags(STATUS_CONNECTED);
                                    }
                                }
                                if(!(lAdrDatabaseEntry->StatusFlags() & STATUS_CONNECTED))
                                {
                                    bDbUpToDate = false;
                                    pDatabaseEntry->m_strLastInfoMsg = "ADR "+QString(GEXDB_ADR_NAME)+" is not connected";
                                }
                            }
                        }

                        if(bDbUpToDate)
                            pDatabaseEntry->SetStatusFlags(STATUS_DBCOMPATIBLE);
                        else
                        {
                            if(pDatabaseEntry->m_strLastInfoMsg.isEmpty())
                                pDatabaseEntry->m_strLastInfoMsg = GGET_LASTERRORMSG(GexDbPlugin_Base, pPlugin->m_pPlugin);
                            if(pDatabaseEntry->m_strLastInfoMsg.count(":")>1)
                                pDatabaseEntry->m_strLastInfoMsg =
                                        pDatabaseEntry->m_strLastInfoMsg.section(":",pDatabaseEntry->m_strLastInfoMsg.count(":")-1);
                        }

                        if(bDbUpToDate && (nBuildVersion == nBuildSupported))
                            pDatabaseEntry->SetStatusFlags(STATUS_DBUPTODATE);
                        if(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_bCompressionProtocol)
                            pDatabaseEntry->SetStatusFlags(STATUS_COMPRESS_PROTOCOL);

                        if(!pDatabaseEntry->IsReadOnly())
                            pDatabaseEntry->SetStatusFlags(STATUS_INSERTION);

                        // Check if the TdrType was setted with the connection
                        if(TdrTypeRecorded.isEmpty() && !pDatabaseEntry->IsUnknownTdr())
                        {
                            // Update the XML file
                            if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
                            {
                                if(pDatabaseEntry->IsStoredInDb())
                                    GS::Gex::Engine::GetInstance().GetAdminEngine().SaveDatabase(pDatabaseEntry);
                            }
                        }
                    }
                    else
                    {
                        QString strError;
                        pDatabaseEntry->m_pExternalDatabase->GetLastError(strError);
                        if(strError.isEmpty())
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->GetLastError(strError);
                        if(strError.isEmpty())
                            strError += " [Unknown error]";

                        pDatabaseEntry->m_strLastInfoMsg = "Database Connection Error: "+strError;
                        pDatabaseEntry->TraceInfo();
                    }
                    pDatabaseEntry->m_LastStatusChecked = GS::Gex::Engine::GetInstance().GetServerDateTime();
                }
                else
                {
                    // Disconnect connector
                    if(pPlugin->m_pPlugin->m_pclDatabaseConnector->IsConnected())
                        pPlugin->m_pPlugin->m_pclDatabaseConnector->Disconnect();
                    // Even if the database is not connected
                    // We know if support insertion
                    if(!pDatabaseEntry->IsReadOnly())
                        pDatabaseEntry->SetStatusFlags(STATUS_INSERTION);
                }
            }
        }
        else
        {
            pDatabaseEntry->UnsetStatusFlags(STATUS_INSERTION);
            pDatabaseEntry->UnsetStatusFlags(STATUS_HOUSEKEEPING);

            QString strError;
            pDatabaseEntry->m_pExternalDatabase->GetLastError(strError);
            if(strError.isEmpty() && pPlugin)
                pPlugin->m_pPlugin->m_pclDatabaseConnector->GetLastError(strError);
            if(strError.isEmpty())
                strError += " [Unknown error]";

            pDatabaseEntry->m_strLastInfoMsg = "Plugin Error: "+strError;
            pDatabaseEntry->TraceInfo();

        }
    }
    else
    {
        pDatabaseEntry->SetStatusFlags(STATUS_DBUPTODATE);
        pDatabaseEntry->SetStatusFlags(STATUS_DBCOMPATIBLE);
        pDatabaseEntry->SetStatusFlags(STATUS_REMOVE);
        if(pDatabaseEntry->IsReadOnly())
            pDatabaseEntry->UnsetStatusFlags(STATUS_INSERTION);
        else
            pDatabaseEntry->SetStatusFlags(STATUS_INSERTION);
        pDatabaseEntry->SetStatusFlags(STATUS_HOUSEKEEPING);

    }

    UpdateStatusMessage("");

    if(StatusFlags != pDatabaseEntry->StatusFlags())
    {
        pDatabaseEntry->m_LastStatusUpdate =
                pDatabaseEntry->m_LastStatusChecked = GS::Gex::Engine::GetInstance().GetServerDateTime();
        pDatabaseEntry->SaveToXmlFile(QString());

    }

    strText = "DB entry : " + pDatabaseEntry->PhysicalName() + " checked";
    GSLOG(SYSLOG_SEV_DEBUG, strText.toLatin1().constData() );

    return true;
}

///////////////////////////////////////////////////////////
// Update a Database entry. Create .gexdb_entry file
///////////////////////////////////////////////////////////
bool DatabaseEngine::UpdateDatabaseEntry(QString strPath, GexDatabaseEntry *pDatabaseEntry)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg( strPath.toLatin1().constData())
          .toLatin1().constData());

    // Create database information file
    strPath += GEX_DATABASE_ENTRY_DEFINITION;
    QFile f( strPath );
    if(!f.open( QIODevice::WriteOnly ))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("failed to open %1 in WriteOnly mode").arg( strPath.toLatin1().constData())
              .toLatin1().constData());
        pDatabaseEntry->m_strLastInfoMsg = QString("failed to open %s in WriteOnly mode").arg(strPath);
        return false;	// Failed creating Description file.
    }
    // Assign file I/O stream
    QTextStream hEntryFile(&f);

    // Fill file with Database details...
    hEntryFile << "<database>" << endl;			// Start definition marker
    hEntryFile << "Name=" << pDatabaseEntry->LogicalName() << endl;			// Database name
    hEntryFile << "Description=" << pDatabaseEntry->Description() << endl;		// Description
    hEntryFile << "Storage=" << pDatabaseEntry->StorageType() << endl;		// Storage mode= Copy

    if(pDatabaseEntry->IsExternal())
    {
        if(pDatabaseEntry->IsReadOnly())
            hEntryFile << "ExternalDatabaseReadOnly=1" << endl;		// Read ONLY database
        else
            hEntryFile << "ExternalDatabaseReadOnly=0" << endl;		// Read & Write database

        hEntryFile << "ExternalDatabase=1" << endl;		// External database
        if(pDatabaseEntry->IsStoredInDb())
            hEntryFile << "Location=Server" << endl;
    }
    else
    {
        hEntryFile << "ExternalDatabase=0" << endl;				// External database
        hEntryFile << "ExternalDatabaseReadOnly=0" << endl;		// Read & Write database
    }


    hEntryFile << "Size=" << QString::number(pDatabaseEntry->CacheSize()*(1024.0*1024.0)) << endl;	// Database size.
    hEntryFile << "ImportPwd="  << pDatabaseEntry->ImportFilePassword() << endl;	// 'Import file' password
    hEntryFile << "DeletePwd="  << pDatabaseEntry->DeletePassword() << endl;	// 'Import file' password
    hEntryFile << "</database>" << endl;		// end definition marker
    f.close();
    hEntryFile.setDevice(0);

    // Update list of valid databases
    //ReloadDatabasesList(strLocalDatabase,strServerDatabase);
    return true;	// Success
}

bool DatabaseEngine::UpdateDatabaseEntry(QString strDatabaseName)
{
    // Find database path to the logical name given...
    GexDatabaseEntry *pDatabaseEntry;
    pDatabaseEntry = FindDatabaseEntry(strDatabaseName,false);
    if(pDatabaseEntry == NULL)
        return false;	// Failed finding entry!

    return pDatabaseEntry->SaveToXmlFile(QString());
    //  return UpdateDatabaseEntry(pDatabaseEntry->DbPhysicalPath(), pDatabaseEntry);
}

///////////////////////////////////////////////////////////
// Save a Database entry into the YieldMan Admin Database
///////////////////////////////////////////////////////////
bool DatabaseEngine::UploadDatabaseEntry(GexDatabaseEntry *pDatabaseEntry)
{
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        return false;

    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return false;

    if(!pDatabaseEntry || !pDatabaseEntry->IsExternal() || (pDatabaseEntry->m_pExternalDatabase==NULL))
        return false;

    if(!pDatabaseEntry->IsYmProdTdr() && !pDatabaseEntry->IsAdr())
        return false;

    // If external database, Upload it
    GexDbPlugin_ID *pPluginID = pDatabaseEntry->m_pExternalDatabase->GetPluginID();

    if(pPluginID == NULL) return false;

    // Check if it is a duplicated database
    if(pDatabaseEntry->LogicalName() != pDatabaseEntry->m_strDatabaseRef)
        return false;

    QString				strYieldManDatabasesPath;
    // Get option
    // If empty, create it and save
    strYieldManDatabasesPath = GS::Gex::Engine::GetInstance().GetAdminEngine().GetDatabaseFolder();

    // Upload this DataBase in YieldManDb
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().SaveDatabase(pDatabaseEntry))
        return false;

    // Check if folder is GalaxySemi/databases
    if(!pDatabaseEntry->PhysicalPath().startsWith(strYieldManDatabasesPath,Qt::CaseInsensitive))
    {
        // Then have to Move folder to the good location before to upload it
        QDir	cDir;
        QString strOldDir = pDatabaseEntry->PhysicalPath();
        pDatabaseEntry->SetPhysicalPath(QDir::cleanPath(strYieldManDatabasesPath + QDir::separator() +  pDatabaseEntry->PhysicalName()));

        // Copy
        if(CopyFolderContent(strOldDir,pDatabaseEntry->PhysicalPath()))
            DeleteFolderContent(strOldDir);

        // Force update and upload
        // Create folder if not exist.
        cDir.mkpath(pDatabaseEntry->PhysicalPath());

        QString strSettingsFile = pDatabaseEntry->PhysicalPath() + GEX_DATABASE_EXTERNAL_DB_DEF;
        GexDbPlugin_ID *pPlugin = pDatabaseEntry->m_pExternalDatabase->GetPluginID();
        if(pPlugin == NULL) return false;
        if(pPlugin->m_pPlugin == NULL) return false;

        pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strSettingsFile = strSettingsFile;
        pPlugin->setConfigured(true);
        pDatabaseEntry->SaveToXmlFile(QString());
        //     UpdateDatabaseEntry(pLocalDatabaseEntry->DbPhysicalPath(), pLocalDatabaseEntry);
    }

    // if scheduler is active, update Tasks view
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTasksList(false);

    return true;
}

///////////////////////////////////////////////////////////
// GexAdminGui: Connect database
///////////////////////////////////////////////////////////
bool DatabaseEngine::ConnectDatabase(QString DatabaseName)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Connect Database '%1'").arg( DatabaseName).toLatin1().constData());
    bool isDatabaseFound =false;
    // Check if database is refenced
    GexDatabaseEntry *pDatabaseEntry = FindDatabaseEntry(DatabaseName,false);
    if(pDatabaseEntry)
    {
        isDatabaseFound = true;
        if(!(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED))
        {
            // Database not connected
            // Force the auto connection
            pDatabaseEntry->UnsetStatusFlags(STATUS_MANUALLY_DISCONNECTED);
            CheckDatabaseEntryStatus(pDatabaseEntry,false);
        }
    }
    if (!isDatabaseFound)
        GSLOG(SYSLOG_SEV_ERROR, QString("Database '%1' not found!").arg( DatabaseName).toLatin1().constData());

    return isDatabaseFound;
}

///////////////////////////////////////////////////////////
// GexAdminGui: Disconnect database
///////////////////////////////////////////////////////////
bool DatabaseEngine::DisconnectDatabase(QString DatabaseName)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Disconnect Database '%1'").arg( DatabaseName).toLatin1().constData());
    bool isDatabaseFound =false;
    // Check if database is refenced
    GexDatabaseEntry *pDatabaseEntry = FindDatabaseEntry(DatabaseName,false);
    if(pDatabaseEntry)
    {
        isDatabaseFound = true;
        if(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
        {
            // Database not connected
            // Force the auto connection
            pDatabaseEntry->SetStatusFlags(STATUS_MANUALLY_DISCONNECTED);
            pDatabaseEntry->UnsetStatusFlags(STATUS_CONNECTED);
            CheckDatabaseEntryStatus(pDatabaseEntry,false);
        }
    }
    if (!isDatabaseFound)
        GSLOG(SYSLOG_SEV_ERROR, QString("Database '%1' not found!").arg( DatabaseName).toLatin1().constData());

    return isDatabaseFound;
}

///////////////////////////////////////////////////////////
// Delete ALL empty Sub-folders under specified path.
///////////////////////////////////////////////////////////
bool DatabaseEngine::DeleteEmptySubFolders(QString strFolder, bool bEnteringSubFolder)
{
    bool		bStatus = true;
    QDir		clDir;
    int			nIndex;
    QString		strSubFolder;
    QStringList	strlItems;

    // Any subfolders to process?
    clDir.setPath(strFolder);

    // Check if it is a root Dir like C:, D:, /tmp/root_link
    if(IsRootFolder(strFolder) || strFolder.isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "REMOVING A ROOT FOLDER DISABLED : DeleteEmptySubFolders action cancelled");
        return false;
    }

    strlItems = clDir.entryList(QStringList() << "*", QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
    if(strlItems.size() > 0)
    {
        // Delete all empty subfolders recursively
        for(nIndex=0; nIndex<strlItems.size(); nIndex++)
        {
            strSubFolder = QDir::cleanPath(strFolder + "/" + strlItems.at(nIndex));
            if(DeleteEmptySubFolders(strSubFolder, true))
            {
                // Remove folder as all sub-folders and files have been erased...
                // Make sure file attributes allow delete!
#if defined unix || __MACH__
                chmod(strSubFolder.toLatin1().constData(),0777);
#else
                _chmod(strSubFolder.toLatin1().constData(),_S_IREAD | _S_IWRITE);
#endif
                if(!clDir.rmdir(strSubFolder))
                    bStatus = false;
            }
            else
                bStatus = false;
        }
        return bStatus;
    }

    // No subfolders: if not in a subfolder, success
    if(!bEnteringSubFolder)
        return true;

    // No subfolders, and we are in a subfolder: check if files are present
    strlItems = clDir.entryList(QStringList() << "*", QDir::Files | QDir::Hidden);
    return strlItems.isEmpty();
}

///////////////////////////////////////////////////////////
// Delete the content of a folder
///////////////////////////////////////////////////////////
bool DatabaseEngine::DeleteFolderContent(QString strFolder)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" DeleteFolderContent %1 ").arg( strFolder).toLatin1().constData() );
    QDir d;
    QString		strCleanFolder;
    QString		strFilePath;
    QStringList strDatabaseEntries;
    QStringList::Iterator it;

    strCleanFolder = d.cleanPath(strFolder);
    d.setPath(strCleanFolder);

    // Check if it is a root Dir like C:, D:, /tmp/root_link
    if(IsRootFolder(strCleanFolder) || strCleanFolder.isEmpty())
    {
        GSLOG(SYSLOG_SEV_CRITICAL, " TRY TO REMOVE A ROOT FOLDER : DeleteFolderContent action cancelled !");
        return false;
    }

    // Erase all files in the database tree (if database is Links to file, then
    // files are elsewhere...and we still need to erase internal database files!)

    d.setFilter(QDir::Files | QDir::Hidden);
    strDatabaseEntries = d.entryList(QStringList() << "*");

    for(it = strDatabaseEntries.begin(); it != strDatabaseEntries.end(); ++it )
    {
        // Erase file...
        strFilePath = strCleanFolder + "/" + *it;
        if (!GS::Gex::Engine::RemoveFileFromDisk(strFilePath))
            GSLOG(SYSLOG_SEV_WARNING, QString(" cant remove %1").arg( strFilePath).toLatin1().constData() );
    }

    // Remove all sub-folders
    d.setPath(strCleanFolder);
    d.setFilter(QDir::Dirs | QDir::Hidden);
    strDatabaseEntries = d.entryList(QStringList() << "*");
    for (it = strDatabaseEntries.begin(); it != strDatabaseEntries.end(); ++it )
    {
        // Erase filder: recurssive call...
        strFilePath = strCleanFolder + "/" + *it;
        // If folder is '.' or '..', ignore!
        if((*it != ".") && (*it != ".."))
            if(!DeleteFolderContent(strFilePath))
                GSLOG(SYSLOG_SEV_ERROR, QString(" cant remove %1 ").arg( strFilePath).toLatin1().constData() );
    }

    // Remove folder as all sub-folders and files have been erased...
    // Make sure file attributes allow delete!
#if defined unix || __MACH__
    chmod(strCleanFolder.toLatin1().constData(),0777);
#else
    _chmod(strCleanFolder.toLatin1().constData(),_S_IREAD | _S_IWRITE);
#endif

    bool bStatus = d.rmdir(strCleanFolder);
    if(!bStatus)
        GSLOG(SYSLOG_SEV_WARNING, QString(" cant remove %1 ").arg( strCleanFolder).toLatin1().constData() );

    return bStatus;
}

///////////////////////////////////////////////////////////
// Copy the content of a folder
///////////////////////////////////////////////////////////
bool DatabaseEngine::CopyFolderContent(QString strOldFolder, QString strNewFolder)
{
    GSLOG(SYSLOG_SEV_ERROR, QString("copy %1 to %2 ")
          .arg(strOldFolder.toLatin1().constData())
          .arg(strNewFolder.toLatin1().constData() )
          .toLatin1().constData());

    bool        bStatus = true;
    QDir        d;
    QString     strCleanFolder;
    QString     strFilePath;
    QStringList strDatabaseEntries;
    QStringList::Iterator it;

    strCleanFolder = d.cleanPath(strOldFolder);
    d.setPath(strCleanFolder);

    // Check if it is a root Dir like C:, D:, /tmp/root_link
    if(IsRootFolder(strCleanFolder) || strCleanFolder.isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, " TRY TO COPY A ROOT FOLDER : CopyFolderContent action cancelled !");
        return false;
    }

    if(IsRootFolder(strNewFolder) || strNewFolder.isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "TRY TO DELETE A ROOT FOLDER : CopyFolderContent action cancelled !");
        return false;
    }

    // Make sure the NewFolder exist
    d.mkdir(strNewFolder);

    // Copy all files in the database tree (if database is Links to file, then
    // files are elsewhere...and we still need to copy internal database files!)

    d.setFilter(QDir::Files | QDir::Hidden);
    strDatabaseEntries = d.entryList(QStringList() << "*");

    for(it = strDatabaseEntries.begin(); it != strDatabaseEntries.end(); ++it )
    {
        // Erase file...
        strFilePath = strCleanFolder + "/" + *it;
        if (!CGexSystemUtils::CopyFile(strFilePath,strNewFolder+"/"+*it))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString(" cant copy %1").arg( strFilePath).toLatin1().constData() );
            bStatus = false;
        }
    }

    // Remove all sub-folders
    d.setPath(strCleanFolder);
    d.setFilter(QDir::Dirs | QDir::Hidden);
    strDatabaseEntries = d.entryList(QStringList() << "*");
    for (it = strDatabaseEntries.begin(); it != strDatabaseEntries.end(); ++it )
    {
        // Erase filder: recurssive call...
        strFilePath = strCleanFolder + "/" + *it;
        // If folder is '.' or '..', ignore!
        if((*it != ".") && (*it != ".."))
        {
            if(!CopyFolderContent(strFilePath,strNewFolder + "/" + *it))
            {
                GSLOG(SYSLOG_SEV_ERROR, QString(" cant copy %1 ").arg( strFilePath).toLatin1().constData() );
                bStatus = false;
            }
        }
    }

    return bStatus;
}

bool DatabaseEngine::IsRootFolder(QString strFolder)
{
    QDir		clDir;
    QString		strFolderPath;

    strFolderPath = strFolder.trimmed();

    // Check with initial Value
    clDir.setPath(strFolderPath);
    clDir = clDir.canonicalPath();
    if(clDir.isRoot())
        return true;

    // Check with optionnal '/'
    strFolderPath = QDir::fromNativeSeparators(strFolderPath);
    if(!strFolderPath.endsWith('/'))
    {
        strFolderPath += '/';
        clDir.setPath(strFolderPath);
        clDir = clDir.canonicalPath();
        if(clDir.isRoot())
            return true;
    }

    // Check VarEnv from Windows
    char			*pEnv;
    QString			strSysFolder;
    QStringList		lstVarEnv;
    lstVarEnv.append("GEX_SERVER_PROFILE");
    lstVarEnv.append("GTM_SERVER_PROFILE");
    lstVarEnv.append("YM_SERVER_PROFILE");
    lstVarEnv.append("PM_SERVER_PROFILE");
    lstVarEnv.append("ALLUSERSPROFILE");
    lstVarEnv.append("PROGRAMDATA");
    lstVarEnv.append("APPDATA");
    lstVarEnv.append("HOMEPATH");
    lstVarEnv.append("USERPROFILE");
    lstVarEnv.append("HOMEDRIVE");
    lstVarEnv.append("ProgramFiles");
    lstVarEnv.append("CommonProgramFiles");
    lstVarEnv.append("SystemDrive");
    lstVarEnv.append("SystemRoot");
    lstVarEnv.append("SystemRoot");
    lstVarEnv.append("HOME");

    //ALLUSERSPROFILE=C:\Documents and Settings\All Users
    //APPDATA=C:\Documents and Settings\SandrineC\Application Data
    //CommonProgramFiles=C:\Program Files\Fichiers communs
    //GEX_SERVER_PROFILE=S:\DiskWolfson\Galaxy\serverprofile
    //HOMEDRIVE=C:
    //HOMEPATH=\Documents and Settings\SandrineC
    //ProgramFiles=C:\Program Files
    //SystemDrive=C:
    //SystemRoot=C:\WINDOWS
    //USERPROFILE=C:\Documents and Settings\SandrineC
    //windir=C:\WINDOWS

    while(!lstVarEnv.isEmpty())
    {
        pEnv = getenv(lstVarEnv.takeFirst().toLatin1().constData());
        if(pEnv != NULL)
        {
            // strFolderPath can only be a subfolder
            strSysFolder = pEnv;
            strSysFolder = QDir::fromNativeSeparators(strSysFolder);
            if(!strSysFolder.endsWith('/'))
                strSysFolder += '/';
            if(strSysFolder.startsWith(strFolderPath,Qt::CaseInsensitive))
                return true;

        }
    }

    // Check also ApplicationDir
    strSysFolder = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    strSysFolder = QDir::fromNativeSeparators(strSysFolder);
    if(!strSysFolder.endsWith('/'))
        strSysFolder += '/';
    if(strSysFolder.startsWith(strFolderPath,Qt::CaseInsensitive))
        return true;

    return false;
}



///////////////////////////////////////////////////////////
// Get list of file
// Match with Extension
// Sortby
// with SubFolder
///////////////////////////////////////////////////////////
void listOfFiles(const QString & strPath,  const QRegExp & RegExpExtensions,
                 QFileInfoList & FilesInfo,bool bScanSubFolders,int nCheckAge,
                 QDir::SortFlags eSortFlags, int MaxFiles)
{
    QDir                    dir;
    CGexSystemUtils         clSysUtils;
    QFileInfo               FileInfo;
    QFileInfoList           Files;
    QFileInfoList::iterator it;

    QString Path = strPath;
    // Normalize path
    clSysUtils.NormalizePath(Path);

    // RegExp
    QStringList Extensions  = RegExpExtensions.pattern().split(";",QString::SkipEmptyParts);
    QRegExp     RegExp      = RegExpExtensions;
    bool        bUseRegExp  = false;


    if(!RegExp.isValid())
    {
        // RegExp not already checked
        if((Extensions.count() == 1) && (Extensions.first().simplified().isEmpty()))
            Extensions = QStringList("*");

        RegExp.setPattern(Extensions.join(";"));
        RegExp.setPatternSyntax(QRegExp::Wildcard);

        if(Extensions.count() == 1)
        {
            // Check if it is a RegExp
            RegExp.setPatternSyntax(QRegExp::RegExp);
            if(!RegExp.isValid())
                RegExp.setPatternSyntax(QRegExp::Wildcard);
        }
    }

    if(RegExp.patternSyntax() == QRegExp::RegExp)
    {
        Extensions = QStringList("*");
        bUseRegExp = true;
        // RegExp must be CaseSensitive for unix
        RegExp.setCaseSensitivity(Qt::CaseInsensitive);
#if defined unix || __MACH__
        RegExp.setCaseSensitivity(Qt::CaseSensitive);
#endif
    }


    // Get all files in the directory and add them to the list
    dir.setPath(Path);
    dir.setFilter(QDir::Files | QDir::Hidden);
    dir.setSorting(eSortFlags);
    Files = dir.entryInfoList(Extensions);


    int nbFiles = 0;
    // Do not keep files too recent (15 secs old or less)
    for(it=Files.begin(); it!=Files.end(); it++)
    {
        // Check file age
        if(nCheckAge > 0)
        {
            if((*it).lastModified().secsTo(GS::Gex::Engine::GetInstance().GetClientDateTime()) < nCheckAge)
            {
                GSLOG(SYSLOG_SEV_INFORMATIONAL,
                      QString("File %1 is too recent (less than %2 sec) and will be ignored for now")
                      .arg((*it).absoluteFilePath())
                      .arg(QString::number(nCheckAge))
                      .toLatin1().data() );
                continue;
            }
        }
        if(bUseRegExp)
        {
            QString file = (*it).fileName();
            if(!RegExp.exactMatch(file))
                continue;
        }
        QString file = (*it).fileName();
        if(file.endsWith(".read",Qt::CaseInsensitive) ||
                file.endsWith(".bad",Qt::CaseInsensitive) ||
                file.endsWith(".delay",Qt::CaseInsensitive) ||
                file.endsWith(".quarantine",Qt::CaseInsensitive))
            continue;

        FilesInfo.append(*it);
        nbFiles++;

        if(MaxFiles < 0)
            continue;

        // Stop when have enough files for this Folder
        if(MaxFiles <= nbFiles)
            break;
    }

    // Go recursively through all directories
    if(bScanSubFolders)
    {
        dir.setPath(Path);
        dir.setFilter(QDir::Dirs | QDir::Hidden);
        Files = dir.entryInfoList();
        for(it=Files.begin(); it!=Files.end(); it++)
        {
            FileInfo = *it;
            //for the special entries "." and ".." on Unix
            if(!FileInfo.filePath().endsWith(".")
                    && !FileInfo.filePath().endsWith(".."))
            {
                listOfFiles(FileInfo.filePath(), RegExp, FilesInfo,
                            bScanSubFolders,nCheckAge,eSortFlags, MaxFiles);
            }
        }
    }
}

bool sortFileName(const QFileInfo &s1, const QFileInfo &s2)
{
    return s1.fileName() < s2.fileName();
}

bool sortFileTime(const QFileInfo &s1, const QFileInfo &s2)
{
    // QDir::entryInfoList order file
    // on Newest to Oldest with QDir::Time option
    if(s1.lastModified() > s2.lastModified())
        return true;
    if(s1.lastModified() < s2.lastModified())
        return false;
    // If same time, sort by name
    return s1.fileName() < s2.fileName();
}

bool sortFileSize(const QFileInfo &s1, const QFileInfo &s2)
{
    if(s1.size() < s2.size())
        return true;
    if(s1.size() > s2.size())
        return false;
    // If same size, sort by name
    return s1.fileName() < s2.fileName();
}

bool sortFileType(const QFileInfo &s1, const QFileInfo &s2)
{
    if(s1.suffix() < s2.suffix())
        return true;
    if(s1.suffix() > s2.suffix())
        return false;
    // If same type, sort by name
    return s1.fileName() < s2.fileName();
}


///////////////////////////////////////////////////////////
// Get list of files
///////////////////////////////////////////////////////////
QStringList DatabaseEngine::GetListOfFiles(const QString & strRootPath, const QString & strExtensions,
                                           bool bScanSubFolders, QDir::SortFlags eSortFlags,int Priority)
{
    int MaxFilesToSelect = -1;
    if(Priority >= 0)
    {
        // PRIORITY_OPTION = "1|10|100"
        // 1   file  for Priority 0 (Low)
        // 10  files for Priority 1 (Medium)
        // 100 files for Priority 2 (High)
        // Option can be customized into ym_nodes_options
        // -1 = no limit ex: "1|100|-1"
        QString MaxFilesPerPriorityOptions;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeSettingsValue("LOADBALANCING_FILE_SELECTION_PRIORITY",
                                                                                 MaxFilesPerPriorityOptions);

        MaxFilesToSelect = MaxFilesPerPriorityOptions.section("|",Priority,Priority).simplified().toInt();

        if(MaxFilesToSelect == 0)
            MaxFilesToSelect = (int) pow(10, (Priority));

        // For manual insertion
        // just get the first
        if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
            MaxFilesToSelect = 1;
    }

    QRegExp     RegExp;
    QStringList Extensions = strExtensions.split(";",QString::SkipEmptyParts);
    // remove space
    QString Value, NewExtensions;
    foreach(Value , Extensions)
        NewExtensions += Value.simplified()+";";
    Extensions = NewExtensions.split(";",QString::SkipEmptyParts);

    // RegExp
    RegExp.setPattern(Extensions.join(";"));
    QStringList FilesList;
    QFileInfoList FilesInfo;
    int nCheckAge = (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()?15:-1);
    if((nCheckAge > 0) && GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()){
        bool bIsNumber;
        QString Var;
        GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeSettingsValue("LOADBALANCING_FILE_SELECTION_AGE_EXCLUSION",Var);
        Var.toInt(&bIsNumber);
        if(bIsNumber)
            nCheckAge = Var.toInt();
    }

    QDir dir(strRootPath);
    GSLOG(SYSLOG_SEV_DEBUG, QString("Get files from Path[%1] Ext[%2] Age[%3]...")
          .arg(dir.absolutePath()).arg(Extensions.join(";")).arg(nCheckAge)
          .toLatin1().data());

    listOfFiles(strRootPath,RegExp,FilesInfo,bScanSubFolders,
                nCheckAge,eSortFlags,MaxFilesToSelect);


    // Have the list of file for all Folder and SubFolders
    if(eSortFlags & QDir::Time)
        qSort(FilesInfo.begin(), FilesInfo.end(), sortFileTime);
    else if(eSortFlags & QDir::Size)
        qSort(FilesInfo.begin(), FilesInfo.end(), sortFileSize);
    else if(eSortFlags & QDir::Type)
        qSort(FilesInfo.begin(), FilesInfo.end(), sortFileType);
    else
        qSort(FilesInfo.begin(), FilesInfo.end(), sortFileName);

    QDir    Dir;
    QString FileName;
    QFileInfo FileInfo;

    Dir.setPath(strRootPath);
    while(!FilesInfo.isEmpty())
    {
        if(eSortFlags & QDir::Reversed)
            FileInfo = FilesInfo.takeLast();
        else
            FileInfo = FilesInfo.takeFirst();

        //FileName = Dir.relativeFilePath(FileInfo.absoluteFilePath());
        //FileName = FileInfo.absoluteFilePath();
        // Returns the canonical path/file name, absolute path without symbolic links or redundant "." or ".." elements.
        FileName = FileInfo.canonicalFilePath();

        if(FilesList.contains(FileName))
            continue;

        FilesList.append(FileName);

        if(MaxFilesToSelect < 0)
            continue;

        // Stop when have enough files for this Folder
        if(MaxFilesToSelect <= FilesList.count())
            break;
    }

    return FilesList;
}

///////////////////////////////////////////////////////////
// Find Database structure entry in the list...
///////////////////////////////////////////////////////////
GexDatabaseEntry *DatabaseEngine::FindDatabaseEntry(QString strDatabaseLogicalName, bool bOnlyConnected)
{
    if(!mAllowed) return NULL;
    if(strDatabaseLogicalName.isEmpty()) return NULL;

    // Debug message
    //QString strMessage = " ";
    //strMessage+= "DatabaseLogicalName="+strDatabaseLogicalName;
    //strMessage+= " bOnlyConnected="+QString::number(bOnlyConnected);
    //GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data());

    // If list of database not already loaded, do it now!
    LoadDatabasesListIfEmpty();

    if(mDatabaseEntries.count() <= 0) return NULL;

    GexDatabaseEntry		*pDatabaseEntry;
    QList<GexDatabaseEntry*>::iterator itBegin	= mDatabaseEntries.begin();
    QList<GexDatabaseEntry*>::iterator itEnd	= mDatabaseEntries.end();

    QString strDatabaseName = strDatabaseLogicalName;
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();
    while(itBegin != itEnd)
    {
        pDatabaseEntry = (*itBegin);

#if defined unix || __MACH__
        if(pDatabaseEntry->LogicalName() == strDatabaseName)
#else
        if(pDatabaseEntry->LogicalName().toLower() == strDatabaseName.toLower())
#endif
        {
            if(!bOnlyConnected || (pDatabaseEntry->StatusFlags() & STATUS_CONNECTED))
                return pDatabaseEntry;	// Correct entry found
            else
            {
                QString strMessage = QString("Database '%1' not accessible : STATUS_CONNECTED = %2").arg(strDatabaseName)
                        .arg( (pDatabaseEntry->StatusFlags() & STATUS_CONNECTED) ? "yes":"no" );
                GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data() );
                return NULL;
            }
        }

        // Move to next enry.
        itBegin++;
    };

    // Debug message
    QString strMessage = "Find Database Entry: "; strMessage+=strDatabaseName; strMessage += " not found !";
    GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data() );
    return NULL;
}

///////////////////////////////////////////////////////////
// Find Database structure entry# in the list...
///////////////////////////////////////////////////////////
GexDatabaseEntry *DatabaseEngine::FindDatabaseEntry(int iDatabaseEntry, bool bOnlyConnected)
{
    if(!mAllowed) return NULL;

    // If list of database not already loaded, do it now!
    LoadDatabasesListIfEmpty();

    GexDatabaseEntry		*pDatabaseEntry;
    QList<GexDatabaseEntry*>::iterator itBegin	= mDatabaseEntries.begin();
    QList<GexDatabaseEntry*>::iterator itEnd	= mDatabaseEntries.end();

    while(itBegin != itEnd)
    {
        pDatabaseEntry = (*itBegin);
        if(pDatabaseEntry->m_nDatabaseId == iDatabaseEntry)
        {
            if(!bOnlyConnected || (pDatabaseEntry->StatusFlags() & STATUS_CONNECTED))
                return pDatabaseEntry;	// Correct entry found
            else
            {
                QString strMessage = QString("Database '%1' not accessible : STATUS_CONNECTED = %2").arg(pDatabaseEntry->LogicalName())
                        .arg( (pDatabaseEntry->StatusFlags() & STATUS_CONNECTED) ? "yes":"no" );
                GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data() );
                return NULL;
            }
        }

        // Move to next enry.
        itBegin++;
    };

    QString strMessage=QString("Find db : database %1 not found").arg(iDatabaseEntry);
    GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data() );
    return NULL;
}

///////////////////////////////////////////////////////////
// Find Database structure entry# in the list...
///////////////////////////////////////////////////////////
GexDatabaseEntry *DatabaseEngine::FindDatabaseEntry(QString strPluginName, QString strHost, int nPort, QString strDriver, QString strSchema, QString strDatabase)
{
    if(!mAllowed) return NULL;

    QString strMessage = "Find DB Entry for ";
    strMessage+= "PluginName="+strPluginName;
    strMessage+= ",Host_Name="+strHost;
    strMessage+= ",Port="+QString::number(nPort);
    strMessage+= ",Driver="+strDriver;
    strMessage+= ",SchemaName="+strSchema;
    strMessage+= ",DatabaseName="+strDatabase;
    strMessage+=")";
    GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data());

    // If list of database not already loaded, do it now!
    LoadDatabasesListIfEmpty();

    GexDatabaseEntry		*pDatabaseEntry;
    GexDbPlugin_ID			*pPlugin;
    GexDbPlugin_Connector	*pPluginConnector;
    QList<GexDatabaseEntry*>::iterator itBegin	= mDatabaseEntries.begin();
    QList<GexDatabaseEntry*>::iterator itEnd	= mDatabaseEntries.end();

    for(itBegin=mDatabaseEntries.begin(); itBegin != itEnd; itBegin++)
    {
        pDatabaseEntry = (*itBegin);

        // Return only the original (YieldManDb) database
        if(pDatabaseEntry->LogicalName() != pDatabaseEntry->m_strDatabaseRef)
            continue;

        if(pDatabaseEntry->m_pExternalDatabase)
        {
            pPlugin = pDatabaseEntry->m_pExternalDatabase->GetPluginID();
            if(pPlugin && pPlugin->m_pPlugin && pPlugin->m_pPlugin->m_pclDatabaseConnector)
                pPluginConnector = pPlugin->m_pPlugin->m_pclDatabaseConnector;
            else
                pPluginConnector = NULL;

            // If the plugin for this database could not be loaded, we have no access to connection IDs
            if(!pPlugin || !pPluginConnector)
                continue;

            if(pPlugin && (pPlugin->pluginName() != strPluginName))
                continue;
            if(pPluginConnector && (pPluginConnector->m_strHost_Name != strHost))
                continue;
            if(pPluginConnector && (pPluginConnector->m_uiPort != nPort))
                continue;
            if(pPluginConnector && (pPluginConnector->m_strDriver != strDriver))
                continue;
            if(pPluginConnector && (pPluginConnector->m_strSchemaName != strSchema))
                continue;
            if(pPluginConnector && (pPluginConnector->m_strDatabaseName != strDatabase))
                continue;

            // Find the good entry
            //strMessage=QString("DatabaseEngine::FindDatabaseEntry: database %1 found ! ").arg(pDatabaseEntry->strDatabaseLogicalName);

            return pDatabaseEntry;
        }
    };

    // Debug message
    strMessage=QString(" database '%1' not found ! ").arg(strSchema);
    GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data() );
    return NULL;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
QString	DatabaseEngine::GetImportPassword(QString strDatabaseLogicalName)
{
    GexDatabaseEntry *pDatabaseEntry = FindDatabaseEntry(strDatabaseLogicalName,false);
    if(pDatabaseEntry != NULL)
        return pDatabaseEntry->ImportFilePassword();
    else
        return "";
}

QString	DatabaseEngine::GetDeletePassword(QString strDatabaseLogicalName)
{
    GexDatabaseEntry *pDatabaseEntry = FindDatabaseEntry(strDatabaseLogicalName,false);
    if(pDatabaseEntry != NULL)
        return pDatabaseEntry->DeletePassword();
    else
        return "";
}

bool	DatabaseEngine::DeleteDatabaseEntry(QString strDatabaseName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Delete database %1")
          .arg( strDatabaseName.toLatin1().constData())
          .toLatin1().constData());
    // Find database path to the logical name given...
    GexDatabaseEntry *pDatabaseEntry;
    pDatabaseEntry = FindDatabaseEntry(strDatabaseName,false);
    if(pDatabaseEntry == NULL)
        return false;	// Failed finding entry!

    // If the database is a Yield man production, delete the ADR associated with it.
    if (pDatabaseEntry->MustHaveAdrLink())
    {
        QString lADRDatabaseEntry = GetADRDBNameFromTDRDBName(pDatabaseEntry->LogicalName());
        if(!lADRDatabaseEntry.isEmpty() && !DeleteDatabaseEntry(lADRDatabaseEntry))
            return false;
    }

    // If YieldManDb is activated
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            &&  GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer()
            && (pDatabaseEntry->IsStoredInDb()))
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Delete database entry %1")
              .arg( QString::number(pDatabaseEntry->m_nDatabaseId).toLatin1().constData())
              .toLatin1().constData());
        // Delete database entry into the YieldManDatabase table
        QString strQuery;
        QSqlQuery clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
        strQuery = "SELECT DB.database_id FROM ym_databases DB WHERE DB.database_id="+QString::number(pDatabaseEntry->m_nDatabaseId);

        if(clQuery.exec(strQuery))
        {
            // Delete databases into YieldManDb
            pDatabaseEntry->TraceUpdate("DELETE","START","Delete database");
            if(clQuery.first())
            {
                // Then delete current database
                strQuery = "DELETE FROM ym_databases_options";
                strQuery+= " WHERE database_id="+QString::number(pDatabaseEntry->m_nDatabaseId);
                if(!clQuery.exec(strQuery))
                {
                    pDatabaseEntry->TraceUpdate("DELETE","FAIL",clQuery.lastError().text());
                    GSLOG(SYSLOG_SEV_DEBUG, QString("Delete options fail: %1")
                          .arg( clQuery.lastError().text().toLatin1().constData())
                          .toLatin1().constData());
                    return false;
                }

                // Then delete current database
                strQuery = "DELETE FROM ym_databases";
                strQuery+= " WHERE database_id="+QString::number(pDatabaseEntry->m_nDatabaseId);
                if(!clQuery.exec(strQuery))
                {
                    pDatabaseEntry->TraceUpdate("DELETE","FAIL",clQuery.lastError().text());
                    GSLOG(SYSLOG_SEV_DEBUG, QString("Delete database fail: %1")
                          .arg( clQuery.lastError().text().toLatin1().constData())
                          .toLatin1().constData());
                    return false;
                }
                pDatabaseEntry->TraceUpdate("DELETE","PASS","Deleted");
            }
        }
    }

    if (pDatabaseEntry->m_pExternalDatabase)
    {
        GSLOG(SYSLOG_SEV_DEBUG, " try to disconnect before deleting...");
        if (!pDatabaseEntry->m_pExternalDatabase->GetPluginID())
        {
            GSLOG(SYSLOG_SEV_DEBUG, " ...cant get PluginID !");
        }
        else if (!pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin)
        {
            GSLOG(SYSLOG_SEV_DEBUG, " ...cant get Plugin !");
        }
        else if (!pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_pclDatabaseConnector)
        {
            GSLOG(SYSLOG_SEV_DEBUG, " ...cant get Connector !");
        }
        else if (!pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_pclDatabaseConnector->Disconnect())
        {
            GSLOG(SYSLOG_SEV_DEBUG, " failed to disconnect");
        }
    }

    // Recurrsive function to delete folder and all sub-folders & files
    GSLOG(SYSLOG_SEV_DEBUG, QString("Delete database folder %1")
          .arg( pDatabaseEntry->PhysicalPath().toLatin1().constData())
          .toLatin1().constData());
    bool bStatus = DeleteFolderContent(pDatabaseEntry->PhysicalPath());

    if(bStatus == false)
        return false;	// Failed removing folder

    // Success removing database...remove entry from our list.
    mDatabaseEntries.removeAll(pDatabaseEntry);
    if (pDatabaseEntry)
        delete pDatabaseEntry;

    GSLOG(SYSLOG_SEV_DEBUG, QString("Delete database %1 DONE")
          .arg( strDatabaseName.toLatin1().constData())
          .toLatin1().constData());

    return true;
}

void DatabaseEngine::ReadStringToField(GS::StdLib::Stdf *pStdfFile,char *szField)
{
    char	szString[257];	// A STDF string is 256 bytes long max!
    QString strValue;

    // Empties string.
    *szField=0;

    // Read string from STDF file.
    if(pStdfFile->ReadString(szString)  != GS::StdLib::Stdf::NoError)
        return;

    // Security: ensures we do not overflow destination buffer !
    szString[256] = 0;
    strValue = szString;
    strValue = strValue.trimmed();
    strcpy(szField,strValue.toLatin1().constData());
}

///////////////////////////////////////////////////////////
// Replaces ',' with '.' in the string
///////////////////////////////////////////////////////////
char *DatabaseEngine::FormatIdxString(char *szString)
{
    char *ptChar;

    // Replaces comas ',' by ';'.
    do
    {
        ptChar = strchr(szString,',');
        if(ptChar != NULL) *ptChar = ';';
    }
    while(ptChar != NULL);
    return szString;
}

///////////////////////////////////////////////////////////
// Replaces ',' with '.' in the string
///////////////////////////////////////////////////////////
QString DatabaseEngine::FormatIdxString(const QString &data)
{
    QString strString = data;
    if(strString.isEmpty() == true)
        return "";

    strString.replace( QChar(','), ";");

    return strString;
}

///////////////////////////////////////////////////////////
// Add " for argument script when necessary (space)
///////////////////////////////////////////////////////////
QString DatabaseEngine::FormatArgumentScriptString(const QString &strString)
{
    if(strString.isEmpty() == true)
        return "";

    QString lArgument = strString;

    lArgument = lArgument.replace("\n"," ");
    lArgument = lArgument.replace("\"","'");

    if(lArgument.contains(QRegExp("\\s")))
        lArgument = "\"" + lArgument + "\"";

#ifdef _WIN32
    // Replace '/' to '\' to avoid MS-DOS compatibility issues
    lArgument = lArgument.replace('/','\\');
#endif

    return lArgument;
}

bool	DatabaseEngine::UpdateFilterTableFile(QString strDatabaseFilterFolder,int iFilterID, QString strValue)
{
    if(strValue.isEmpty() == true)
        return true;	// Silent ignore empty strings!

    QString strFilerTableFile = strDatabaseFilterFolder + gexFilterChoices[iFilterID];

    QFile f(strFilerTableFile);
    if(f.open(QIODevice::ReadOnly))
    {
        // Filter table file exists...read it to see if matching entry found.
        QString strString;
        QTextStream hFilterTableFile(&f);	// Assign file handle to data stream

        do
        {
            strString = hFilterTableFile.readLine();
            if(strString == strValue)
                return true;	// Value already in file...no need to insert it again.
        }
        while(hFilterTableFile.atEnd() == false);

        // Close file
        f.close();
    }

    // We reach this point if the file doesn't exist or doesn't have a matching value.
    if(f.open(QIODevice::WriteOnly | QIODevice::Append) == false)
        return false;	// Failed adding value to file...disk full?

    // Update Filter table definition File
    QTextStream hUpdateFilterTableFile(&f);	// Assign file handle to data stream

    // Fill file with Database details...
    hUpdateFilterTableFile << strValue << endl;
    f.close();

    return true;
}

bool DatabaseEngine::CheckFileForCopy(QString strFileName,
                                      CGexMoTaskItem *ptTask,
                                      QString &cause)
{
#if defined(unix) || __MACH__
    struct flock    stFileLock;
    int              iStatus;
#endif
    int             hFile;
    struct stat     stFileInfo, stTestFileInfo;
    QString         strTestFile;
    time_t          tCurrent, tFile;

    // Compute test file name
    strTestFile = strFileName + ".stdf_examinator_filespool_timedate";

    ///// FOR DEBUG ONLY: Allows to immediatly import files, no 1minute delay!
    //return true;
    ///// FOR DEBUG ONLY

    // Check FileName
    if(strFileName.isEmpty())
    {
        cause="Check File For Copy: This file is not a valid file path (empty name)";
        return false;
    }

    // Get file last modification time
    if(stat(strFileName.toLatin1().constData(), &stFileInfo) != 0)
    {
        cause="Check File For Copy: Cannot have access to file priority";
        return false;
    }
    tFile = stFileInfo.st_mtime > stFileInfo.st_ctime ? stFileInfo.st_mtime : stFileInfo.st_ctime;

    // Get current system time
    remove(strTestFile.toLatin1().constData());
    // Check if can create a file into this folder
#if defined unix || __MACH__
    hFile = open(strTestFile.toLatin1().constData(), O_RDWR | O_CREAT, GFILELOCK_PERMISSIONS);
#else
    hFile = open(strTestFile.toLatin1().constData(), O_RDWR | O_CREAT | O_TEMPORARY, GFILELOCK_PERMISSIONS);
#endif
    if(hFile == GFILELOCK_INVALID_HANDLE)
    {
        cause="Check File For Copy: Cannot create a temporary file into "+QFileInfo(strTestFile).canonicalPath();
        return false;
    }
    if(fstat(hFile, &stTestFileInfo) != 0)
    {
        close(hFile);
        remove(strTestFile.toLatin1().constData());
        cause="Check File For Copy: Cannot create a temporary file "+strTestFile;
        return false;
    }
    close(hFile);
    remove(strTestFile.toLatin1().constData());
    tCurrent = stTestFileInfo.st_ctime;

    // Check if last modification more than 1 min ago
    if(ptTask == NULL)
    {
        // Check if last modification more than 1 min ago
        if((tCurrent-tFile) < 60)
        {
            // ignore
            //cause="Check File For Copy: This file was modified less than 1 min ago";
            //return false;
        }
    }

    // Open file
    // case 7474 - file permission
    // If the Folder is ReadWrite, check only if the file is ReadOnly
    // Can rename the file
    //hFile = open(strFileName.toLatin1().constData(), O_RDWR);
    hFile = open(strFileName.toLatin1().constData(), O_RDONLY);
    if(hFile == GFILELOCK_INVALID_HANDLE)
    {
        cause="Check File For Copy: Cannot open file "+strFileName;
        return false;
    }

#if defined(unix) || __MACH__
    // On unix, the file creation mask can affect the file mode. Make sure the file has the expected mode with chmod
    chmod(strFileName.toLatin1().constData(), GFILELOCK_PERMISSIONS);

    // Check if lock would succeed
    // case 7474 - file permission
    //stFileLock.l_type = F_WRLCK;
    stFileLock.l_type = F_RDLCK;
    stFileLock.l_whence = SEEK_SET;
    stFileLock.l_start = 0;
    stFileLock.l_len = 0;

    iStatus = fcntl(hFile, F_GETLK, &stFileLock);
    close(hFile);

    if(iStatus == GFILELOCK_LOCKERROR)
    {
        cause="Check File For Copy: File "+strFileName+" is locked";
        return false;
    }
    if(stFileLock.l_type == F_UNLCK)
        // File exists, is writable and not locked
        return true;

    cause="Check File For Copy: File "+strFileName+" is locked";
    return false;

#elif defined(WIN32)
    // Check if lock would succeed
    if(_locking(hFile, _LK_NBLCK, 1) == 0)
    {
        _locking(hFile, _LK_UNLCK, 1);
        close(hFile);
        return true;
    }
    close(hFile);

    cause="Check File For Copy: File "+strFileName+" is locked";
    return true;	// Change to 'true' as it seems locking doesn't work!

#endif
}

unsigned long	DatabaseEngine::BuildSummarySTDF(QString /*strSourceArchive*/,
                                                 bool /*bMakeLocalCopy*/, QString strFileNameSTDF,
                                                 QtLib::DatakeysContent &cKeyContent)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Building summary stdf for %1").arg(strFileNameSTDF).toLatin1().data() );
    GS::StdLib::Stdf				StdfSumFile;
    GS::StdLib::StdfRecordReadInfo	StdfSumRecordHeader;
    QString strSumFile = strFileNameSTDF + ".sum";
    StdfSumRecordHeader.iCpuType = 1;       // SUN formay
    StdfSumRecordHeader.iStdfVersion = 4;   // STDF V4

    // Check if the database mode is 'Links only'...in such case, the summary must be created in the file home location!
    //	if(bMakeLocalCopy == false)
    //	{
    //		// Build summary file name
    //		strSumFile = strSourceArchive + ".sum";
    //	}

    int iStatus = StdfSumFile.Open(strSumFile.toLatin1().constData(),STDF_WRITE,1000000L);
    if(iStatus != GS::StdLib::Stdf::NoError)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Cannot write open %1").arg(strSumFile).toLatin1().data() );
        return 0;	// Error. Can't create STDF Summary file!
    }

    // Write FAR
    StdfSumRecordHeader.iRecordType = 0;
    StdfSumRecordHeader.iRecordSubType = 10;
    StdfSumFile.WriteHeader(&StdfSumRecordHeader);
    StdfSumFile.WriteByte(StdfSumRecordHeader.iCpuType);
    StdfSumFile.WriteByte(StdfSumRecordHeader.iStdfVersion);
    StdfSumFile.WriteRecord();

    // Write MIR
    StdfSumRecordHeader.iRecordType = 1;
    StdfSumRecordHeader.iRecordSubType = 10;
    StdfSumFile.WriteHeader(&StdfSumRecordHeader);
    StdfSumFile.WriteDword(cKeyContent.Get("SetupTime").toUInt());    // Setup_T
    StdfSumFile.WriteDword(cKeyContent.Get("StartTime").toUInt());    // Start_T
    StdfSumFile.WriteByte((BYTE)cKeyContent.Get("Station").toUInt());  // station #
    StdfSumFile.WriteByte(cKeyContent.Get("DataType").toString().at(0).toLatin1());// mode_code
    StdfSumFile.WriteByte(0);           // rtst_code
    StdfSumFile.WriteByte(0);           // prot_cod #
    StdfSumFile.WriteWord((WORD)cKeyContent.Get("BurninTime").toInt()); // burn_time
    StdfSumFile.WriteByte(0);           // cmode_code
    StdfSumFile.WriteString(cKeyContent.Get("Lot").toString().toLatin1().constData());
    StdfSumFile.WriteString(cKeyContent.Get("Product").toString().toLatin1().constData());
    StdfSumFile.WriteString(cKeyContent.Get("TesterName").toString().toLatin1().constData());
    StdfSumFile.WriteString(cKeyContent.Get("TesterType").toString().toLatin1().constData());
    StdfSumFile.WriteString(cKeyContent.Get("ProgramName").toString().toLatin1().constData());
    StdfSumFile.WriteString(cKeyContent.Get("ProgramRevision").toString().toLatin1().constData());
    StdfSumFile.WriteString(cKeyContent.Get("SubLot").toString().toLatin1().constData());
    StdfSumFile.WriteString(cKeyContent.Get("Operator").toString().toLatin1().constData());
    StdfSumFile.WriteRecord();

    ///////////////////////////////////////////////////////////////
    // Write PTRs: REQUIRED for specifying test limits!
    ///////////////////////////////////////////////////////////////
    // Pointer to file/structure processed
    CGexGroupOfFiles    *pGroup=0;
    CGexFileInGroup     *pFile=0;
    CTest               *ptTestCell=0;
    BYTE                bData;

    if (!gexReport)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Null report");
        return 0;
    }
    pGroup = gexReport->getGroupsList().size()>0?gexReport->getGroupsList().first():NULL;
    // anti crash: seems to happen sometimes when inserting inside file based DB
    if (!pGroup)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Null group");
        return 0;
    }
    pFile  = ( pGroup->pFilesList.isEmpty()) ? NULL : (pGroup->pFilesList.first()) ;
    if (!pFile)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Null file");
        return 0;
    }
    // Get pointer to parameters list (rewind)
    ptTestCell = pFile->ptTestList;

    // WIR: If a wafer was tested in this file, then insert the WIR/WRR records...
    if(pFile->getWaferMapData().bWaferMapExists)
    {
        // Write WIR .
        StdfSumRecordHeader.iRecordType = 2;
        StdfSumRecordHeader.iRecordSubType = 10;
        StdfSumFile.WriteHeader(&StdfSumRecordHeader);
        StdfSumFile.WriteByte(1);                             // Test head
        StdfSumFile.WriteByte(255);                           // Tester site (all)
        StdfSumFile.WriteDword(pFile->getWaferMapData().lWaferStartTime);  // Start time
        StdfSumFile.WriteString(pFile->getWaferMapData().szWaferID);       // WaferID
        StdfSumFile.WriteRecord();
    }

    // Write the PIR/PRR blocs so to log the Die location & Bin results
    QListIterator<CPartInfo*> lstIteratorPartInfo(pFile->pPartInfoList);
    CPartInfo * ptrPartInfo = NULL;

    lstIteratorPartInfo.toFront();

    while(lstIteratorPartInfo.hasNext())
    {
        ptrPartInfo = lstIteratorPartInfo.next();

        // Write PIR for parts in this Wafer site
        StdfSumRecordHeader.iRecordType = 5;
        StdfSumRecordHeader.iRecordSubType = 10;
        StdfSumFile.WriteHeader(&StdfSumRecordHeader);
        StdfSumFile.WriteByte(ptrPartInfo->bHead);            // Test head
        StdfSumFile.WriteByte(ptrPartInfo->m_site & 0xff);            // Tester site
        StdfSumFile.WriteRecord();

        // Write PRR
        StdfSumRecordHeader.iRecordType = 5;
        StdfSumRecordHeader.iRecordSubType = 20;
        StdfSumFile.WriteHeader(&StdfSumRecordHeader);
        StdfSumFile.WriteByte(ptrPartInfo->bHead);            // Test head
        StdfSumFile.WriteByte(ptrPartInfo->m_site & 0xff);            // Tester site
        if(ptrPartInfo->bPass)
            bData = 0;	// Pass flag
        else
            bData = 8;	// Fail flag
        StdfSumFile.WriteByte(bData);                 // PART_FLG : PASSED/Fail info

        StdfSumFile.WriteWord(ptrPartInfo->iTestsExecuted);     // NUM_TEST
        StdfSumFile.WriteWord(ptrPartInfo->iHardBin);           // HARD_BIN
        StdfSumFile.WriteWord(ptrPartInfo->iSoftBin);           // SOFT_BIN
        StdfSumFile.WriteWord(ptrPartInfo->iDieX);              // X_COORD
        StdfSumFile.WriteWord(ptrPartInfo->iDieY);              // Y_COORD
        StdfSumFile.WriteDword(ptrPartInfo->lExecutionTime);    // No testing time known...
        StdfSumFile.WriteString(ptrPartInfo->getPartID().toLatin1().constData());// PART_ID
        StdfSumFile.WriteString("");              // PART_TXT
        StdfSumFile.WriteString("");              // PART_FIX
        StdfSumFile.WriteRecord();
    };

    // WRR: If a wafer was tested in this file, then insert the WIR/WRR records...
    if(pFile->getWaferMapData().bWaferMapExists)
    {
        StdfSumRecordHeader.iRecordType = 2;
        StdfSumRecordHeader.iRecordSubType = 20;
        StdfSumFile.WriteHeader(&StdfSumRecordHeader);
        StdfSumFile.WriteByte(1);                         // Test head
        StdfSumFile.WriteByte(255);                       // Tester site (all)
        StdfSumFile.WriteDword(pFile->getWaferMapData().lWaferEndTime);  // Time of last part tested
        StdfSumFile.WriteDword(pFile->getPcrDatas().lPartCount);          // Parts tested
        StdfSumFile.WriteDword(0);                              // Parts retested
        StdfSumFile.WriteDword(0);                              // Parts Aborted
        StdfSumFile.WriteDword(4294967295UL);                   // Good Parts
        StdfSumFile.WriteDword(4294967295UL);                   // Functionnal Parts
        StdfSumFile.WriteString(pFile->getWaferMapData().szWaferID); // WaferID
        StdfSumFile.WriteRecord();
    }

    // Find ALL parameters matching our list...and write their PTR
    while(ptTestCell != NULL)
    {
        StdfSumRecordHeader.iRecordType = 15;
        StdfSumRecordHeader.iRecordSubType = 10;
        StdfSumFile.WriteHeader(&StdfSumRecordHeader);
        StdfSumFile.WriteDword(ptTestCell->lTestNumber);      // Test Number
        StdfSumFile.WriteByte(1);                             // Test head
        StdfSumFile.WriteByte(1);                             // Tester site#
        StdfSumFile.WriteByte(2);                             // TEST_FLG: Invalid test result
        StdfSumFile.WriteByte(0);                             // PARAM_FLG
        StdfSumFile.WriteFloat(0.0);                          // Test result
        StdfSumFile.WriteString(ptTestCell->strTestName.toLatin1().constData());	// TEST_TXT
        StdfSumFile.WriteString("");                          // ALARM_ID
        bData = 2;
        if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL)
            bData |= 0x10;	// NO low limit
        if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL)
            bData |= 0x20;	// NO high limit

        StdfSumFile.WriteByte(bData);                           // OPT_FLAG
        StdfSumFile.WriteByte(0);                               // RES_SCALE
        StdfSumFile.WriteByte(0);                               // LLM_SCALE
        StdfSumFile.WriteByte(0);                               // HLM_SCALE
        StdfSumFile.WriteFloat(ptTestCell->GetCurrentLimitItem()->lfLowLimit);         // LOW Limit
        StdfSumFile.WriteFloat(ptTestCell->GetCurrentLimitItem()->lfHighLimit);        // HIGH Limit
        StdfSumFile.WriteString(ptTestCell->szTestUnits);       // Units
        StdfSumFile.WriteRecord();

        // Next cell.
        ptTestCell = ptTestCell->GetNextTest();
    };

    ///////////////////////////////////////////////////////////////
    // Write TSRs: REQUIRED for specifying Min, Max, Sum(x) and SumSQR(x) (for computing Cp, Cpk)
    ///////////////////////////////////////////////////////////////

    pGroup = gexReport->getGroupsList().size()>0?gexReport->getGroupsList().first():NULL;
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    // Get pointer to parameters list (rewind)
    ptTestCell = pFile->ptTestList;

    // Find ALL parameters matching our list...and write their TSR...
    while(ptTestCell != NULL)
    {
        StdfSumRecordHeader.iRecordType = 10;
        StdfSumRecordHeader.iRecordSubType = 30;
        StdfSumFile.WriteHeader(&StdfSumRecordHeader);
        StdfSumFile.WriteByte(255);                       // head_num: merged sites
        StdfSumFile.WriteByte(0);                         // site_num
        StdfSumFile.WriteByte(ptTestCell->bTestType);     // test_type
        StdfSumFile.WriteDword(ptTestCell->lTestNumber);  // test_num
        StdfSumFile.WriteDword(ptTestCell->bStatsFromSamples ? ptTestCell->ldSamplesValidExecs : ptTestCell->ldExecs);// exec_count
        StdfSumFile.WriteDword(ptTestCell->bStatsFromSamples ? ptTestCell->GetCurrentLimitItem()->ldSampleFails : ptTestCell->GetCurrentLimitItem()->ldFailCount);// fail_count
        StdfSumFile.WriteDword(4294967295UL);             // alarm_count
        StdfSumFile.WriteString(ptTestCell->strTestName.toLatin1().constData());
        StdfSumFile.WriteString(0);                       // seq_name
        StdfSumFile.WriteString(0);                       // test_labl

        BYTE cOptFlag = (128|64|8);
        cOptFlag |= ((ptTestCell->bStatsFromSamples ? ptTestCell->lfSamplesMin : ptTestCell->lfMin) == C_INFINITE ) ? 1 : 0;
        cOptFlag |= ((ptTestCell->bStatsFromSamples ? ptTestCell->lfSamplesMax : ptTestCell->lfMax) == -C_INFINITE ) ? 2 : 0;
        cOptFlag |= ((ptTestCell->bStatsFromSamples ? ptTestCell->lfSamplesTotal : ptTestCell->lfTotal) == -C_INFINITE ) ? 16 : 0;
        cOptFlag |= ((ptTestCell->bStatsFromSamples ? ptTestCell->lfSamplesTotalSquare : ptTestCell->lfTotalSquare) == -C_INFINITE ) ? 32 : 0;

        StdfSumFile.WriteByte(cOptFlag);                  // opt_flag
        StdfSumFile.WriteFloat(0.0);                      // test_time
        double lfExponent = ScalingPower(ptTestCell->res_scal);
        StdfSumFile.WriteFloat(ptTestCell->bStatsFromSamples ? ptTestCell->lfSamplesMin / lfExponent : ptTestCell->lfMin);    // test_min
        StdfSumFile.WriteFloat(ptTestCell->bStatsFromSamples ? ptTestCell->lfSamplesMax / lfExponent : ptTestCell->lfMax);    // test_max
        StdfSumFile.WriteFloat(ptTestCell->bStatsFromSamples ? ptTestCell->lfSamplesTotal  / lfExponent: ptTestCell->lfTotal);// test_sum
        StdfSumFile.WriteFloat(ptTestCell->bStatsFromSamples ? ptTestCell->lfSamplesTotalSquare / (lfExponent*lfExponent) : ptTestCell->lfTotalSquare); // test_sqrs
        StdfSumFile.WriteRecord();

        // Next cell.
        ptTestCell = ptTestCell->GetNextTest();
    };


    // Write HBRs
    CBinning	*ptBinCell;	// Pointer to Bin cell
    ptBinCell= pGroup->cMergedData.ptMergedHardBinList;	// Point to Hard bin list
    while(ptBinCell != NULL)
    {
        StdfSumRecordHeader.iRecordType = 1;
        StdfSumRecordHeader.iRecordSubType = 40;
        StdfSumFile.WriteHeader(&StdfSumRecordHeader);
        StdfSumFile.WriteByte(255);                   // Test Head = ALL
        StdfSumFile.WriteByte(255);                   // Test sites = ALL
        StdfSumFile.WriteWord(ptBinCell->iBinValue);  // HBIN = 0
        StdfSumFile.WriteDword(ptBinCell->ldTotalCount);// Total Bins
        StdfSumFile.WriteByte(ptBinCell->cPassFail);  // 'P' / 'F' type.
        StdfSumFile.WriteRecord();

        ptBinCell = ptBinCell->ptNextBin;
    };

    // Write SBRs
    ptBinCell= pGroup->cMergedData.ptMergedSoftBinList;	// Point to Soft bin list
    while(ptBinCell != NULL)
    {
        StdfSumRecordHeader.iRecordType = 1;
        StdfSumRecordHeader.iRecordSubType = 50;
        StdfSumFile.WriteHeader(&StdfSumRecordHeader);
        StdfSumFile.WriteByte(255);                   // Test Head = ALL
        StdfSumFile.WriteByte(255);                   // Test sites = ALL
        StdfSumFile.WriteWord(ptBinCell->iBinValue);  // HBIN = 0
        StdfSumFile.WriteDword(ptBinCell->ldTotalCount);// Total Bins
        StdfSumFile.WriteByte(ptBinCell->cPassFail);  // 'P' / 'F' type.
        StdfSumFile.WriteRecord();

        ptBinCell = ptBinCell->ptNextBin;
    };

    // Write PCR
    StdfSumRecordHeader.iRecordType = 1;
    StdfSumRecordHeader.iRecordSubType = 30;
    StdfSumFile.WriteHeader(&StdfSumRecordHeader);
    StdfSumFile.WriteByte(255);                     // Test Head = ALL
    StdfSumFile.WriteByte(255);                     // Test sites = ALL
    StdfSumFile.WriteDword(pFile->getPcrDatas().lPartCount);// Total Parts tested
    StdfSumFile.WriteDword(4294967295UL);           // Total Parts re-tested
    StdfSumFile.WriteDword(4294967295UL);           // Total Parts aborted
    StdfSumFile.WriteDword(pFile->getPcrDatas().lGoodCount);// Total GOOD Parts
    StdfSumFile.WriteRecord();

    // Write MRR
    StdfSumRecordHeader.iRecordType = 1;
    StdfSumRecordHeader.iRecordSubType = 20;
    StdfSumFile.WriteHeader(&StdfSumRecordHeader);
    if(pFile->getMirDatas().lEndT > cKeyContent.Get("FinishTime").toLongLong())
        StdfSumFile.WriteDword(pFile->getMirDatas().lEndT);     // Timestamp of last part tested
    else
        StdfSumFile.WriteDword(cKeyContent.Get("FinishTime").toLongLong()); // Finish_T
    StdfSumFile.WriteRecord();

    StdfSumFile.Close();

    // Return file size
    return QFile(strSumFile).size();
}

QString DatabaseEngine::ExtractFileKeys(const QString &lDataFile,
                                        QtLib::DatakeysContent *lKeysContent)
{
    if (!lKeysContent)
        return "error: null KeysContent";
    bool lRes=ExtractFileKeys(lDataFile, *lKeysContent);
    if (!lRes)
        return "error: extraction failed";
    return "ok";
}

bool DatabaseEngine::ExtractFileKeys(QString strDataFile,GS::QtLib::DatakeysContent& dbKeysContent)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Extract file keys for '%1'").arg(strDataFile).toLatin1().data() );
    QString strFileUncompressed;
    bool	bEraseOnExit=false;
    bool	bStatus=true;

    // Make sure the database /.temp/ folder exists!
    QDir			cDir;
    QString			strUnzipPath = GS::Gex::Engine::GetInstance().Get("TempFolder").toString()+QDir::separator();
    CArchiveFile	clZip(strUnzipPath);
    cDir.mkdir(strUnzipPath);

    // Check if it is a compressed file....
    if(clZip.IsCompressedFile(strDataFile))
    {
        // Uncompress file...
        QStringList	strUncompressedFiles;

        if(clZip.Uncompress(strDataFile, strUncompressedFiles) == false)
            return false;	// Error uncompressing

        // If more than one file in the archive, display warning...
        if(strUncompressedFiles.count() > 1)
            GS::Gex::Message::information(
                        "", "Warning: You have multiple files compressed "
                            "in one archive.\nOnly first file will be processed.");

        // Build full path to zip file.
        strFileUncompressed = GS::Gex::Engine::GetInstance().Get("TempFolder").toString();
        strFileUncompressed += QDir::separator()+strUncompressedFiles.first();

        // Erase uncompressed file when done
        bEraseOnExit = true;
    }
    else
        strFileUncompressed = strDataFile;

    //    GS::StdLib::Stdf               StdfFile;
    //    GS::StdLib::StdfRecordReadInfo  StdfRecordHeader;
    //    int                 iStatus;
    //    BYTE    bData;
    //    int     wData;
    //    long    lData;
    //    char    szString[GEX_MAX_PATH];
    //    int     nNbPIR = 0;
    QString lDatakeysLoaderError;

    bool lResult= GS::QtLib::DatakeysLoader::Load(strFileUncompressed, dbKeysContent, lDatakeysLoaderError);
    if(!lResult)
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Error when loading %1 a,d error : %2").arg(strFileUncompressed).arg(lDatakeysLoaderError)
              .toLatin1().constData());
        bStatus = false;
    }

    // Remove uncompressed file in temporary folder file...
    if(bEraseOnExit)
        GS::Gex::Engine::RemoveFileFromDisk(strFileUncompressed);

    return bStatus;
}

int DatabaseEngine::ImportWyrFile(GexDatabaseEntry *pDatabaseEntry,
                                  const QString & strWyrFile,
                                  QString &strErrorMessage,
                                  QtLib::DatakeysEngine &dbKeysEngine,
                                  unsigned int *puiFileSize,
                                  CGexMoTaskDataPump *ptTask/*=NULL*/)
{
    bool	bDelayInsertion = false;
    QString	strLocalErrorMessage;

    // Debug message
    QString strMessage = " Import Wyr File ";
    strMessage += strWyrFile;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data() );

    // Init variables
    m_strInsertionShortErrorMessage = "";
    *puiFileSize = 0;

    if((ptTask != NULL) && (pDatabaseEntry->m_pExternalDatabase != NULL))
    {
        // Init keycontent structure
        dbKeysEngine.dbKeysContent().SetInternal("FileName",strWyrFile);
        dbKeysEngine.dbKeysContent().SetInternal("FileSize",QFileInfo(strWyrFile).size());
        dbKeysEngine.dbKeysContent().SetInternal("StdfFileName",strWyrFile);
        dbKeysEngine.dbKeysContent().SetInternal("StdfFileSize",QFileInfo(strWyrFile).size());
        dbKeysEngine.dbKeysContent().SetInternal("SourceArchive",strWyrFile);

        // Caller is Examinator-Monitoring...then check for a Key configuration file. + erase it if it is a file-specific one.
        QString strError;
        bool	bStatus,bFailedValidationStep;
        int		nLineNb=0;
        QString strMoTaskSpoolingDir = (ptTask->GetProperties()) ? ptTask->GetProperties()->strDataPath : "";

        // Init Db Keys Engine
        QString                                 configFileName;

        if (dbKeysEngine.findConfigDbKeysFile(configFileName, dbKeysEngine.dbKeysContent(), strMoTaskSpoolingDir))
        {
            if (dbKeysEngine.loadConfigDbKeysFile(configFileName, nLineNb, strError))
            {
                bStatus = dbKeysEngine.evaluateStaticDbKeys(bFailedValidationStep, nLineNb, strError);
                if(!bStatus)
                {
                    // Set short error message
                    if(bFailedValidationStep)
                        m_strInsertionShortErrorMessage = "Validation failed (gexdbkeys) ";
                    else
                        m_strInsertionShortErrorMessage = "Syntax error (gexdbkeys) ";

                    // Set complete error msg for log file
                    // Report validation error type
                    strLocalErrorMessage += m_strInsertionShortErrorMessage;

                    // Report file name
                    strLocalErrorMessage += "\n\tConfig file: " + configFileName;
                    strLocalErrorMessage += " (line ";
                    strLocalErrorMessage += QString::number(nLineNb);
                    strLocalErrorMessage += ")";
                    strLocalErrorMessage += "\n\tError: " + strError;
                    strLocalErrorMessage += "\n\tWYR File: " + strWyrFile;
                    UpdateLogError(strErrorMessage,strLocalErrorMessage);

                    if(bFailedValidationStep)
                        return FailedValidationStep;
                    else
                        return Delay;
                }
            }
            else
            {
                m_strInsertionShortErrorMessage = "Syntax error (gexdbkeys) ";

                // Set complete error msg for log file
                // Report validation error type
                strLocalErrorMessage += m_strInsertionShortErrorMessage;

                // Report file name
                strLocalErrorMessage += "\n\tConfig file: " + configFileName;
                strLocalErrorMessage += " (line ";
                strLocalErrorMessage += QString::number(nLineNb);
                strLocalErrorMessage += ")";
                strLocalErrorMessage += "\n\tError: " + strError;
                strLocalErrorMessage += "\n\tWYR File: " + strWyrFile;
                UpdateLogError(strErrorMessage,strLocalErrorMessage);

                return Delay;
            }
        }

        bool Status = pDatabaseEntry->m_pExternalDatabase->InsertWyrDataFile(strWyrFile,
                                                                             dbKeysEngine.dbKeysContent().Get("Facility").toString(),
                                                                             ptTask->GetProperties()->strTestingStage,
                                                                             dbKeysEngine.dbKeysContent().Get("WeekNb").toUInt(),
                                                                             dbKeysEngine.dbKeysContent().Get("Year").toUInt(),
                                                                             &bDelayInsertion);
        if(!Status)
        {
            // Failed inserting file in remote/corporate database file...
            QString		strError;

            // Get error message from plug-in
            pDatabaseEntry->m_pExternalDatabase->GetLastError(strError);

            // Set short error message
            if(bDelayInsertion)
                m_strInsertionShortErrorMessage = "Insertion delayed";
            else
                m_strInsertionShortErrorMessage = "Insertion failed";

            // Set complete error msg for log file
            // Build error message to display
            strLocalErrorMessage = "Insertion failed: " + strError;
            strLocalErrorMessage += "\n\tFile: " + strWyrFile;
            UpdateLogError(strErrorMessage,strLocalErrorMessage);

            if(bDelayInsertion)
                return Delay;	// Don't rename file to '.bad'
            return Failed;	// Rename file to '.bad'
        }
        else
        {
            // Insertion successful, write warnings to Log file if any
            QStringList strlWarnings;
            pDatabaseEntry->m_pExternalDatabase->GetWarnings(strlWarnings);
            if(!strlWarnings.empty())
            {
                QString					strWarningMessage, strLocalWarningMessage;
                QStringList::Iterator	itWarnings;

                // Build warning message to display
                strLocalWarningMessage = "Insertion warning(s): ";
                strLocalWarningMessage += "\n\tFile: " + strWyrFile;

                for(itWarnings = strlWarnings.begin(); itWarnings != strlWarnings.end(); ++itWarnings)
                {
                    strLocalWarningMessage += "\n\tWarning: " + *itWarnings;
                }
                UpdateLogError(strWarningMessage,strLocalWarningMessage);
            }
        }
    }

    return Passed;
}

bool DatabaseEngine::LoadConfigKeys(
        GS::QtLib::DatakeysEngine &dbKeysEngine,
        CGexMoTaskItem *ptTask,
        bool &bEditHeaderInfo,
        bool &bFailedValidationStep,
        QString &strLocalErrorMessage,
        QString &strShortErrorMsg,
        QString & strFileNameSTDF)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Load config keys file for Product '%1'...").
          arg(dbKeysEngine.dbKeysContent().Get("Product").toString()).toLatin1().data());
    bFailedValidationStep = false;
    QString configFileName;

    if((ptTask == NULL) && (bEditHeaderInfo == true))
    {
        if (!dbKeysEngine.findConfigDbKeysFile(configFileName, dbKeysEngine.dbKeysContent(), ""))
        {
            // Are we sure this is daemon compatible ?
            // Launch config keys editor
            GS::Gex::DbKeysEditor keysEditor(dbKeysEngine.dbKeysContent(),
                                             mFilesToImport,
                                             GS::Gex::DbKeysEditor::FILEINSERTION,
                                             pGexMainWindow);
            if (keysEditor.exec() == QDialog::Rejected)
                return false;
        }
    }

    // Caller is Examinator-Monitoring...then check for a Key configuration file. + erase it if it is a file-specific one.

    QString strError;
    QString strMoTaskSpoolingDir = (ptTask && ptTask->GetDataPumpData()) ? ptTask->GetDataPumpData()->strDataPath : "";
    bool	bStatus;
    int		nLineNb=0;

    if (dbKeysEngine.findConfigDbKeysFile(configFileName, dbKeysEngine.dbKeysContent(), strMoTaskSpoolingDir))
    {
        if (dbKeysEngine.loadConfigDbKeysFile(configFileName, nLineNb, strError))
        {
            bStatus = dbKeysEngine.evaluateStaticDbKeys(bFailedValidationStep, nLineNb, strError);

            if(!bStatus)
            {
                // Set short error message
                if(bFailedValidationStep)
                    strShortErrorMsg = "Validation failed (gexdbkeys) ";
                else
                    strShortErrorMsg = "Syntax error (gexdbkeys) ";

                // Set complete error msg for log file
                // Report validation error type
                strLocalErrorMessage += strShortErrorMsg;

                // Report file name
                strLocalErrorMessage += "\n\tConfig file: " + dbKeysEngine.dbKeysContent().Get("ConfigFileName").toString();
                strLocalErrorMessage += " (line ";
                strLocalErrorMessage += QString::number(nLineNb);
                strLocalErrorMessage += ")";
                strLocalErrorMessage += "\n\tError: " + strError;
                strLocalErrorMessage += "\n\tSTDF File: " + strFileNameSTDF;
                strLocalErrorMessage += "\n\tSource: " + dbKeysEngine.dbKeysContent().Get("SourceArchive").toString();
                m_strInsertionShortErrorMessage = strShortErrorMsg;

                return false;
            }
        }
        else
        {
            strShortErrorMsg = "Syntax error (gexdbkeys) ";

            // Set complete error msg for log file
            // Report validation error type
            strLocalErrorMessage += strShortErrorMsg;

            // Report file name
            strLocalErrorMessage += "\n\tConfig file: " + dbKeysEngine.dbKeysContent().Get("ConfigFileName").toString();
            strLocalErrorMessage += " (line ";
            strLocalErrorMessage += QString::number(nLineNb);
            strLocalErrorMessage += ")";
            strLocalErrorMessage += "\n\tError: " + strError;
            strLocalErrorMessage += "\n\tSTDF File: " + strFileNameSTDF;
            strLocalErrorMessage += "\n\tSource: " + dbKeysEngine.dbKeysContent().Get("SourceArchive").toString();
            m_strInsertionShortErrorMessage = strShortErrorMsg;

            return false;
        }
    }

    // Make sure we have no \n,\r in strings
    QStringList lCleanChar = QStringList() << "\n" << "\t";
    QStringList lKeys = dbKeysEngine.dbKeysContent().allowedStaticDbKeys();
    while(!lKeys.isEmpty())
        dbKeysEngine.dbKeysContent().CleanKeyData(lKeys.takeFirst(),lCleanChar);

    return true;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool	DatabaseEngine::SmartOpenFile(QFile &f, QIODevice::OpenMode openMode)
{
    QTime cTimeoutTime = QTime::currentTime();
    cTimeoutTime = cTimeoutTime.addSecs(60);
    QTime cTime,cWait;

    if((openMode == QIODevice::ReadOnly) && (f.exists() == false))
        return false;	// Can't read as file doesn't exists!

    do
    {
        // Open Index file in Append mode to append the reference to our new STDF file.
        if(f.open(openMode) == true)
            return true;	// Success accessing to file

        // Failed accessing to file....then wait 150msec & retry.
        cWait = cTime.addMSecs(150);
        do
        {
            cTime = QTime::currentTime();
        }
        while(cTime < cWait);
    }
    while(cTime < cTimeoutTime);

    // Failed having access to the resource for over 1 minute....
    return false;
}

bool	DatabaseEngine::CheckIfFileToProcess(QString strFileName,CGexMoTaskItem *ptTask, QString &cause)
{
    // Add any valid file to the list...
    if((strFileName == ".")  || (strFileName == ".."))
    {
        cause="This file is not a valid file path";
        return false;	// That's not a file, ignore it!
    }

    if(strFileName.endsWith(".gexdbkeys",Qt::CaseInsensitive))
    {
        cause="This file is a key definition not a data file";
        return false;	// That's a Key definition file (to overwrite MIR records info): ignore it!
    }

    if(strFileName.endsWith(".read",Qt::CaseInsensitive)
            || strFileName.endsWith(".bad",Qt::CaseInsensitive)
            || strFileName.endsWith(".delay",Qt::CaseInsensitive)
            || strFileName.endsWith(".quarantine",Qt::CaseInsensitive))
    {
        cause="This file was already processed (reserved extension)";
        return false;	// That's a processed file: ignore it!
    }

    if(ptTask == NULL)
    {
        cause="Monitoring mode is not running";
        return true;	// Not running monitorinng...so process file anyway
    }

    cause="ok";
    return true;	// Files processed do not stay on server unchanged....so process this file.
}

void DatabaseEngine::SearchMatchingFilesInFolder(
        const QString &strSourceFolder,
        bool bRecursive,
        QStringList &strDataFilesToImport,
        CGexMoTaskItem *ptTask)
{

    bool            ScanSubFolder = bRecursive;
    QDir::SortFlags eSortFlags=QDir::Time;
    int             Priority=-1;
    QString         Extensions;
    QFileInfo       File;
    QFileInfoList   Files;
    QFileInfoList::Iterator it;

    if(ptTask != NULL)
    {
        // Import ALL files, no matter extension. (RegExp or WildCard)
        Extensions = ptTask->GetDataFileExtension();// ->ptDataPump->strImportFileExtensions;
        ScanSubFolder = ptTask->IsDataFileScanSubFolder();
        eSortFlags = ptTask->GetDataFileSort();
        Priority = ptTask->GetPriority();
    }
    else
        // Files extensions to look for...
        Extensions = ConvertToSTDF::GetListOfSupportedFormat().join(";");

    GetListOfFiles(strSourceFolder,Extensions,ScanSubFolder,eSortFlags,Priority);

    // Build absolute path to each file to import
    QString strFileShortName;
    QString		strFile;
    QDir Dir;
    Dir.setPath(strSourceFolder);
    for(it = Files.begin(); it != Files.end(); ++it )
    {
        File = *it;
        strFileShortName = Dir.relativeFilePath(File.fileName());

        // Check if file to be processed (check name, and if need be: see if belongs to the list of files already processed
        QString cause;
        if(CheckIfFileToProcess(strFileShortName,ptTask, cause))
        {
            // Build long name: path + file name
            strFile = File.absoluteFilePath();

            // If File imported automatically (Examinator-Monitoring DataPump),
            // quietly ignore files not available (currently Written by another application)
            if(ptTask != NULL)
            {
                // if file NOT in use, process it now...else process it later!
                if(CheckFileForCopy(strFile,ptTask, cause) == true)
                    strDataFilesToImport += strFile;
            }
            else
                strDataFilesToImport += strFile;	// Manual import.
        }
    }
}

int	DatabaseEngine::ImportFolder(QString strDatabaseLogicalName,
                                 QString strSourceFolder,
                                 bool bRecursive,
                                 QStringList &strDataFilesToImport,
                                 QStringList *pCorruptedFiles,
                                 QString &strErrorMessage, bool bEditHeaderInfo,
                                 CGexMoTaskDataPump *ptTask)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg( strSourceFolder).toLatin1().constData() );
    bool                            bResult;
    GexDatabaseInsertedFilesList    listInsertedFiles;

    m_strInsertionShortErrorMessage = "";

    // Check if folder exists!
    if(QFile::exists(strSourceFolder) == false)
    {
        // Request to import files from a missing folder! Report error
        QString strMessage = "Failed to access spooling folder: " + strSourceFolder;
        strMessage += "\n\tDetails: Folder doesn't exist, not mounted or not available.";
        UpdateLogError(strErrorMessage,strMessage);
    }

    // Update GUI in case long process...
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        QString strStatusMessage = "Scanning folder: " + strSourceFolder;
        emit sDisplayStatusMessage(strStatusMessage);
        QCoreApplication::processEvents();
    }

    // Find all files matching the insertion criteria
    SearchMatchingFilesInFolder(strSourceFolder,bRecursive,strDataFilesToImport,ptTask);


    // Update GUI in case long process...
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        QString strStatusMessage = "Listing files to process: " + QString::number(strDataFilesToImport.count());
        emit sDisplayStatusMessage(strStatusMessage);
        QCoreApplication::processEvents();
    }

    // If one or more files to retrieve, display DataPump header in log file
    if(strDataFilesToImport.count() > 0)
    {
        QString strErrorTitle = (ptTask ? ptTask->GetProperties()->strTitle : "");
        QString strErrorTask = "DataPump";
        QString strLocalMessage = "\tTotal files to process: " + QString::number(strDataFilesToImport.count()) + "\n";
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().AppendMoHistoryLog(strLocalMessage,
                                                                               strErrorTask,
                                                                               strErrorTitle);
    }

    // Import all files found in the folder(s)...
    bool bEditHeaderMode = bEditHeaderInfo;	// Need to pass a variable as this flag may be cleared in the 'ImportFiles' function.
    bResult = ImportFiles(strDatabaseLogicalName,strDataFilesToImport,
                          pCorruptedFiles,listInsertedFiles,
                          strErrorMessage,bEditHeaderMode,false, ptTask);

    // Update GUI in case long process...
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        emit sDisplayStatusMessage("");

    return bResult;	// return 'file import' completion status
}

bool DatabaseEngine::ImportOneFile(CGexMoTaskDataPump* ptTask,
                                   QString& strSourceFile,
                                   QString& strErrorMessage,
                                   bool /*bEditHeaderInfo*/)
{
    if(ptTask == NULL)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Import one file failed because task is NULL");
        return false;
    }

    QString strMessage = "Import one file ";
    strMessage += strSourceFile;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data() );

    QString strDatabaseLogicalName = ptTask->m_strDatabaseName;
    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();

    // Check if file already imported
    /*
    QFileInfo cFileInfo(strSourceFile);
    QString cause;
    if(CheckIfFileToProcess(cFileInfo.fileName(),ptTask, cause) == false)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("This file does not have to be processed: %1").arg( cause).toLatin1().constData());
        return false;	// No insertion done.
    }
    */

    // Import specific file
    bool                            bEditHeaderMode = false;
    QStringList                     strDataFilesToImport(strSourceFile);
    QStringList                     strlCorruptedFiles;
    GexDatabaseInsertedFilesList    listInsertedFiles;

    bool bResult = ImportFiles(
                strDatabaseLogicalName,strDataFilesToImport,&strlCorruptedFiles,
                listInsertedFiles,strErrorMessage,bEditHeaderMode, false, ptTask);

    // Update GUI in case long process...
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        emit sDisplayStatusMessage("");

    return bResult;	// return 'file import' completion status
}

void DatabaseEngine::UpdateStatusMessage(QString strMessage)
{
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(strMessage);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool	DatabaseEngine::SplitMultiLotCsvFiles(QStringList &sFilesSelected)
{
    QString			strFileName;
    QStringList::Iterator it;
    CGCSVtoSTDF	CsvInput;
    QStringList	strCsvOriginalFiles;
    QStringList	strSplitFiles;

    for (it = sFilesSelected.begin(); it != sFilesSelected.end(); ++it )
    {
        strFileName = *it;
        if(strFileName.endsWith(".csv",Qt::CaseInsensitive))
        {
            // In case CSV file holds multiple lots, split it!
            if(CsvInput.SplitLots(strFileName,strSplitFiles))
            {
                // Sucessfully split the files, then insert the new split files in the list of data files to insert in the database
                strCsvOriginalFiles += strFileName;
            }
        }
    }

    // a) Remove ALL .CSV files that have been detected as Multi-lots files.
    for (it = strCsvOriginalFiles.begin(); it != strCsvOriginalFiles.end(); ++it )
        sFilesSelected.removeAll(*it);

    // b) Add ALL .CSV split files created from the a) files and to insert in the database
    sFilesSelected += strSplitFiles;
    return true;
}

void	DatabaseEngine::UpdateLogError(
        QString strErrorMessage,
        const QString strError,
        bool bUpdateMoHistoryLog/*=true*/)
{
    QString strMessage = " Update Log Error: ";
    strMessage += strError;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data() );

    // Check if MO History log should be updated
    if(bUpdateMoHistoryLog)
    {
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            GS::Gex::Engine::GetInstance().GetAdminEngine().AddNewEvent("","",
                                                                        "INFO",strError,"UpdateLogError");
        QDateTime cCurrentDateTime=GS::Gex::Engine::GetInstance().GetClientDateTime();
        strErrorMessage = cCurrentDateTime.toString("[d MMMM yyyy h:mm:ss] ");
        strErrorMessage	+= strError + "\n";
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().AppendMoHistoryLog(strErrorMessage);
    }
}

void	DatabaseEngine::UpdateReportLog(
        const QString &strStatus,
        const QString &strFileName,
        const QString &strCause,
        const QString &strDataPump, const QString &strDirectory,
        bool bUpdateMoReportLog/*=true*/,
        unsigned int uiFileSize/*=0*/, unsigned int uiOriginalSize/*=0*/,
        unsigned int uiTotalInsertionTime/*=0*/, unsigned int uiUncompressTime/*=0*/,
        unsigned int uiConvertTime/*=0*/)
{
    QString strMessage = "Update report log: ";
    strMessage += strStatus;
    if (!strFileName.isEmpty())
        strMessage += " file "+strFileName;
    if(!strCause.isEmpty())
        strMessage += " (" + strCause + ") ";
    if (uiTotalInsertionTime!=0)
        strMessage += " in " + QString("%1").arg(uiTotalInsertionTime) + " sec";
    GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data() );

    // Check if MO Report log should be updated
    if(bUpdateMoReportLog)
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().AppendMoReportLog(strStatus,
                                                                              strFileName,
                                                                              strCause,
                                                                              strDataPump,
                                                                              strDirectory,
                                                                              uiFileSize,
                                                                              uiOriginalSize,
                                                                              uiTotalInsertionTime,
                                                                              uiUncompressTime,
                                                                              uiConvertTime);
}



///////////////////////////////////////////////////////////
// Get list of data events entries matching a filter...for the given date
///////////////////////////////////////////////////////////
void	DatabaseEngine::FindMatchingFilter(QStringList &cEntriesMatching,
                                           const QDate& currentDate,
                                           const GexDatabaseFilter& dbFilter,
                                           const GexDatabaseEntry * pDatabaseEntry)
{
    if (pDatabaseEntry)
    {
        QString     strFolderPath;
        QString     strFilePath;
        QString     strString;
        QString     strMatchingData;
        QStringList strCells;

        // Build database folder sub-string: /YYYY/MM/DD
        strFilePath = currentDate.toString("/yyyy");
        strFilePath += currentDate.toString("/MM");
        strFilePath += currentDate.toString("/dd");

        // Build sub-path from database home llocation.
        strFolderPath = pDatabaseEntry->PhysicalPath() + strFilePath;

        // Build full path to the database index file.
        QFile f;
        strString = strFolderPath + GEX_DATABASE_INDEX_DEFINITION;
        f.setFileName(strString);

        // Open Index file in Read to see list of STDF files in this folder.
        if(SmartOpenFile(f, QIODevice::ReadOnly) == false)
            return;	// Failed opening index file...maybe folder doesn't exist!

        // Read Database definition File
        QTextStream hDefinitionFile(&f);	// Assign file handle to data stream

        // Check if valid header...
        strString = hDefinitionFile.readLine();
        if(strString != "<indexes>")
            return;

        // Read all entries...and find files/data sets matching filter criteria.
        QRegExp rx("",Qt::CaseInsensitive);	// NOT case sensitive.
        //FIXME: not used ?
        //bool bFilterMatching;
        QString filterValue;
        int		iFilterID=0;

        do
        {
            //FIXME: not used ?
            //bFilterMatching=true;

            // Read one line: one STDF file header definition.
            strString = hDefinitionFile.readLine();

            // Check if valid line
            if(!strString.isEmpty() && !strString.startsWith("#"))
            {
                // Split line
                strCells = SplitFileBasedDbIndexLine(strString);

                for (int idx = 0; idx < dbFilter.narrowFilters().count(); ++idx)
                {
                    QPair<QString,QString> filter = dbFilter.narrowFilters().at(idx);

                    iFilterID   = DatabaseEngine::GetLabelFilterIndex(filter.first);
                    filterValue = filter.second;

                    rx.setPattern(filterValue);

                    // If wildcar used, set its support.
                    if(filterValue.indexOf("*") >= 0 || filterValue.indexOf("?") >= 0)
                        rx.setPatternSyntax(QRegExp::Wildcard);
                    else
                        rx.setPatternSyntax(QRegExp::RegExp);

                    // If valid filter entry...check it
                    if(iFilterID)
                    {
                        iFilterID         = FiltersMapping[iFilterID];
                        strMatchingData   = strCells[iFilterID];

                        if(rx.indexIn(strMatchingData) < 0)
                            goto next_entry;	// Data found not matching criteria
                    }
                }	// 4 loops as we have 4 Narrowing filters to check.

                // File line is matching the query criteria..save its value into the list
                // Avoid to list multiple times the same data file!
                iFilterID         = DatabaseEngine::GetLabelFilterIndex(dbFilter.queryFilter());
                iFilterID         = FiltersMapping[iFilterID];
                strMatchingData   = strCells[iFilterID];

                if(iFilterID)
                {
                    // If valid mapping found...save new matching entry into our list.
                    if(cEntriesMatching.indexOf(strMatchingData) < 0)
                        cEntriesMatching += strMatchingData;
                }
            }
next_entry:;
        }
        while(hDefinitionFile.atEnd() == false);
    }
}

///////////////////////////////////////////////////////////
// Checks if a given string matches the filter specified
///////////////////////////////////////////////////////////
bool DatabaseEngine::isMatchingFilter(const QString &strFilter, const QString &strString)
{
    // strFilter = "5|13";	// What we're looking for
    // strString = "13";	// File pointed has this.

    QRegExp rx(strFilter,Qt::CaseInsensitive);	// NOT case sensitive

    // If wildcar used, set its support.
    if(strFilter.indexOf("*") >= 0 || strFilter.indexOf("?") >= 0)
        rx.setPatternSyntax(QRegExp::Wildcard);
    else
        rx.setPatternSyntax(QRegExp::RegExp);

    return rx.exactMatch(strString);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Splits a line read from the .gexdb_index index file
// Handles a bug introduced in V7.3 where 21 cells were duplicated starting at cell 24.
//////////////////////////////////////////////////////////////////////////////////////////
QStringList DatabaseEngine::SplitFileBasedDbIndexLine(const QString & aIndexLine)
{
    QStringList lCells;

    lCells = aIndexLine.split(",",QString::KeepEmptyParts);
    if(lCells.count() == 57)
    {
        // Remove cells [24-44]
        QStringList lFixedCells;
        for(int lIndex=0; lIndex<=23; ++lIndex)
        {
            lFixedCells.append(lCells[lIndex]);
        }
        for(int lIndex=45; lIndex<lCells.size(); ++lIndex)
        {
            lFixedCells.append(lCells[lIndex]);
        }
        return lFixedCells;
    }
    return lCells;
}

///////////////////////////////////////////////////////////
// Get list of files matching a query...for the given date
///////////////////////////////////////////////////////////
void DatabaseEngine::FindMatchingFilter(QStringList& cEntriesMatching, const QDate& currentDate,
                                        const GexDatabaseQuery& dbQuery, const QStringList &filtersName,
                                        const GexDatabaseEntry * pDatabaseEntry)
{
    QString       strFolderPath;
    QString       strFilePath;
    QString       strString;
    QStringList   strCells;

    if (pDatabaseEntry)
    {
        // Build database folder sub-string: /YYYY/MM/DD
        strFilePath = currentDate.toString("/yyyy");
        strFilePath += currentDate.toString("/MM");
        strFilePath += currentDate.toString("/dd");

        // Build sub-path from database home llocation.
        strFolderPath = pDatabaseEntry->PhysicalPath() + strFilePath;

        // Build full path to the database index file.
        QFile f;
        strString = strFolderPath + GEX_DATABASE_INDEX_DEFINITION;
        f.setFileName(strString);

        // Open Index file in Read to see list of STDF files in this folder.
        if(SmartOpenFile(f, QIODevice::ReadOnly) == false)
            return;	// Failed opening index file...maybe folder doesn't exist!

        // Read Database definition File
        QTextStream hDefinitionFile(&f);	// Assign file handle to data stream

        // Check if valid header...
        strString = hDefinitionFile.readLine();
        if(strString != "<indexes>")
            return;

        // Read all entries...and find files matching query criteria.
        QString     lStrNumber;
        QString     lFieldData;
        QString     lMatchingData;
        bool        lFilterMatching;
        long        lMinimumPartsInFile;
        int         lFilterID=0;

        do
        {
            lFilterMatching=true;

            // Read one line: one STDF file header definition.
            strString = hDefinitionFile.readLine();

            // Check if valid line
            if(!strString.isEmpty() && !strString.startsWith("#"))
            {
                // Split line
                strCells = SplitFileBasedDbIndexLine(strString);

                // Check if 'LotID' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_LOT)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strLotID,strCells[2]);
                }

                // Check if 'subLotID' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_SUBLOT)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strSubLotID,strCells[3]);
                }

                // Check if 'Program name' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_PROGRAMNAME)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strJobName,strCells[4]);
                }

                // Check if 'Program revision' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_PROGRAMREVISION)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strJobRev,strCells[5]);
                }

                // Check if 'Operator' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_OPERATOR)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strOperator,strCells[6]);
                }

                // Check if 'Node/tester name' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_TESTERNAME)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strNodeName,strCells[7]);
                }

                // Check if 'Node/tester type' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_TESTERTYPE)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strTesterType,strCells[8]);
                }

                // Check if 'test code' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_TESTCODE)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strTestingCode,strCells[9]);
                }

                // Check if 'temperature' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_TEMPERATURE)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strTemperature,strCells[10]);
                }

                // Check if 'Package type' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_PACKAGE)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strPackageType,strCells[11]);
                }

                // Check if 'ProductID' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_PRODUCT)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strProductID,strCells[12]);
                }

                // Check if 'FamilyID' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_FAMILY)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strFamilyID,strCells[13]);
                }

                // Check if 'FacilityID' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_FACILITY)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strFacilityID,strCells[14]);
                }

                // Check if 'FloorID' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_FLOOR)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strFloorID,strCells[15]);
                }

                // Check if 'ProcessID' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_PROCESS)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strProcessID,strCells[16]);
                }

                // Check if 'Frequency/Step' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_FREQUENCYSTEP)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strFrequencyStep,strCells[17]);
                }

                // Check if 'Burning' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_BURNIN)
                {
                    lStrNumber = QString::number(dbQuery.iBurninTime);
                    lFilterMatching &= isMatchingFilter(lStrNumber,strCells[18]);
                }

                // Check if 'Prober Type' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_PROBERTYPE)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strProberType,strCells[19]);
                }

                // Check if 'Prober Name' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_PROBERNAME)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strProberName,strCells[20]);
                }

                // Check if 'LoadBoard Type' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_LOADBOARDTYPE)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strLoadBoardType,strCells[21]);
                }

                // Check if 'LoadBoard Name' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_LOADBOARDNAME)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strLoadBoardName,strCells[22]);
                }

                // Check if 'DIB Name' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_DIBNAME)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strDibName,strCells[33]);
                }

                // Check if 'DIB Type' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_DIBTYPE)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strDibType,strCells[34]);
                }

                // Check if 'Data Origin Name' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_ORIGIN)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strDataOrigin,strCells[23]);
                }

                // Check if 'WaferID' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_WAFERID)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strWaferID,strCells[24]);
                }

                // Check if 'User1' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_USER1)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strUser1,strCells[25]);
                }

                // Check if 'User2' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_USER2)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strUser2,strCells[26]);
                }

                // Check if 'User3' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_USER3)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strUser3,strCells[27]);
                }

                // Check if 'User4' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_USER4)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strUser4,strCells[28]);
                }

                // Check if 'User5' matching filter
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_USER5)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strUser5,strCells[29]);
                }

                // Check if Minimum of parts in file is reached...
                lMinimumPartsInFile = strCells[30].toLong();
                if(lMinimumPartsInFile < dbQuery.lMinimumPartsInFile)
                    lFilterMatching = false;  // File has too few data...ignore it!

                // Skip strCells[31], holds (long) lTotalGoodBins

                // Check if 'Retest count' filter is reached
                if(dbQuery.uFilterFlag & GEX_QUERY_FLAG_RETESTNBR)
                {
                    lFilterMatching &= isMatchingFilter(dbQuery.strRetestNbr,strCells[32]);
                }

                if(lFilterMatching)
                {
                    bool validFilters = true;

                    lMatchingData.clear();

                    // Filter is matching the query criteria... save value for filter of interest
                    // File line is matching the query criteria... save value for filter of interest
                    // Avoid to list multiple times the same data file!
                    for(int idxFilter = 0; idxFilter < filtersName.count(); ++idxFilter)
                    {
                        lFilterID   = GetFilterIndex(filtersName.at(idxFilter));
                        lFilterID   = FiltersMapping[lFilterID];
                        lFieldData   = strCells[lFilterID];

                        if (lFilterID && !lFieldData.isEmpty())
                        {
                            if (idxFilter != 0)
                                lMatchingData += "|";

                            lMatchingData += lFieldData;
                        }
                        else
                            validFilters = false;
                    }

                    if(validFilters && (cEntriesMatching.indexOf(lMatchingData) < 0))
                        cEntriesMatching += lMatchingData;
                }
            }
        }
        while(hDefinitionFile.atEnd() == false);
    }
}

///////////////////////////////////////////////////////////
// Get list of files matching a query...for the given date
///////////////////////////////////////////////////////////
QStringList	DatabaseEngine::FindMatchingFiles(
        QDate *pCurrentDate,
        GexDatabaseQuery *pQuery,
        GexDatabaseEntry *pDatabaseEntry)
{
    QString     strFolderPath;
    QString     strFilePath;
    QString     strString,strSection;
    QStringList cFilesMatching;
    QStringList strCells;

    // Build database folder sub-string: /YYYY/MM/DD
    strFilePath = pCurrentDate->toString("/yyyy");
    strFilePath += pCurrentDate->toString("/MM");
    strFilePath += pCurrentDate->toString("/dd");

    // Build sub-path from database home llocation.
    strFolderPath = pDatabaseEntry->PhysicalPath() + strFilePath;

    // Build full path to the database index file.
    QFile f;
    strString = strFolderPath + GEX_DATABASE_INDEX_DEFINITION;
    f.setFileName(strString);

    // Open Index file in Read to see list of STDF files in this folder.
    if(SmartOpenFile(f, QIODevice::ReadOnly) == false)
        return cFilesMatching;	// Failed opening index file...maybe folder doesn't exist!

    // Read Database definition File
    QTextStream hDefinitionFile(&f);	// Assign file handle to data stream

    // Check if valid header...
    strString = hDefinitionFile.readLine();
    if(strString != "<indexes>")
        return cFilesMatching;

    // Read all entries...and find files matching query criteria.
    QRegExp rx("",Qt::CaseInsensitive);	// NOT case sensitive
    QString strFilter, strNumber;
    bool    bFilterMatching;
    long    lMinimumPartsInFile;

    do
    {
        bFilterMatching=true;

        // Read one line: one STDF file header definition.
        strString = hDefinitionFile.readLine();

        // Check if valid line
        if(!strString.isEmpty() && !strString.startsWith("#"))
        {
            // Split line
            strCells = SplitFileBasedDbIndexLine(strString);

            // Check if 'LotID' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_LOT)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strLotID,strCells[2]);
            }

            // Check if 'subLotID' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_SUBLOT)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strSubLotID,strCells[3]);
            }

            // Check if 'Program name' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PROGRAMNAME)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strJobName,strCells[4]);
            }

            // Check if 'Program revision' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PROGRAMREVISION)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strJobRev,strCells[5]);
            }

            // Check if 'Operator' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_OPERATOR)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strOperator,strCells[6]);
            }

            // Check if 'Node/tester name' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_TESTERNAME)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strNodeName,strCells[7]);
            }

            // Check if 'Node/tester type' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_TESTERTYPE)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strTesterType,strCells[8]);
            }

            // Check if 'test code' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_TESTCODE)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strTestingCode,strCells[9]);
            }

            // Check if 'temperature' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_TEMPERATURE)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strTemperature,strCells[10]);
            }

            // Check if 'Package type' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PACKAGE)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strPackageType,strCells[11]);
            }

            // Check if 'ProductID' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PRODUCT)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strProductID,strCells[12]);
            }

            // Check if 'FamilyID' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_FAMILY)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strFamilyID,strCells[13]);
            }

            // Check if 'FacilityID' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_FACILITY)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strFacilityID,strCells[14]);
            }

            // Check if 'FloorID' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_FLOOR)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strFloorID,strCells[15]);
            }

            // Check if 'ProcessID' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PROCESS)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strProcessID,strCells[16]);
            }

            // Check if 'Frequency/Step' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_FREQUENCYSTEP)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strFrequencyStep,strCells[17]);
            }

            // Check if 'Burning' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_BURNIN)
            {
                strNumber = QString::number(pQuery->iBurninTime);
                bFilterMatching &= isMatchingFilter(strNumber,strCells[18]);
            }

            // Check if 'Prober Type' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PROBERTYPE)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strProberType,strCells[19]);
            }

            // Check if 'Prober Name' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PROBERNAME)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strProberName,strCells[20]);
            }

            // Check if 'LoadBoard Type' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_LOADBOARDTYPE)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strLoadBoardType,strCells[21]);
            }

            // Check if 'LoadBoard Name' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_LOADBOARDNAME)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strLoadBoardName,strCells[22]);
            }

            // Check if 'DIB Name' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_DIBNAME)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strDibName,strCells[33]);
            }

            // Check if 'DIB Type' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_DIBTYPE)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strDibType,strCells[34]);
            }

            // Check if 'Data Origin Name' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_ORIGIN)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strDataOrigin,strCells[23]);
            }

            // Check if 'WaferID' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_WAFERID)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strWaferID,strCells[24]);
            }

            // Check if 'User1' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_USER1)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strUser1,strCells[25]);
            }

            // Check if 'User2' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_USER2)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strUser2,strCells[26]);
            }

            // Check if 'User3' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_USER3)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strUser3,strCells[27]);
            }

            // Check if 'User4' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_USER4)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strUser4,strCells[28]);
            }

            // Check if 'User5' matching filter
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_USER5)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strUser5,strCells[29]);
            }

            // Check if Minimum of parts in file is reached...
            lMinimumPartsInFile = strCells[30].toLong();
            if(lMinimumPartsInFile < pQuery->lMinimumPartsInFile)
                bFilterMatching = false;  // File has too few data...ignore it!

            // Skip strCells[31], holds (long) lTotalGoodBins

            // Check if 'Retest count' filter is reached
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_RETESTNBR)
            {
                bFilterMatching &= isMatchingFilter(pQuery->strRetestNbr,strCells[32]);
            }

            if(bFilterMatching)
            {
                // File is matching the query criteria..save its full path in the list!
                if(strCells[0] == "C")
                {
                    // This file is a copy in the dat
                    strFilePath = strFolderPath + strCells[1];
                }
                else
                {
                    // This file a link (full path to file outside the database)
                    strFilePath = strCells[1];
                }
                // Avoid to list multiple times the same data file!
                if(cFilesMatching.indexOf(strFilePath) < 0)
                    cFilesMatching += strFilePath;
            }
        }
    }
    while(hDefinitionFile.atEnd() == false);

    // Return list of files matching the query
    return cFilesMatching;
}

///////////////////////////////////////////////////////////
// Maps Galaxy query to Corporate database query;
///////////////////////////////////////////////////////////
void DatabaseEngine::MapQueryToCorporateFilters(GexDbPlugin_Filter& dbPluginFilter, const GexDatabaseQuery& dbQuery)
{
    dbPluginFilter.iTimePeriod                = dbQuery.iTimePeriod;               // Time period to consider.
    dbPluginFilter.iTimeNFactor               = dbQuery.iTimeNFactor;
    dbPluginFilter.m_eTimeStep                = (GexDbPlugin_Filter::eTimeStep) dbQuery.m_eTimeStep;
    dbPluginFilter.calendarFrom               = dbQuery.calendarFrom;             // Filter: From date
    dbPluginFilter.calendarTo                 = dbQuery.calendarTo;                // Filter: To date
    dbPluginFilter.calendarFrom_Time          = dbQuery.calendarFrom_Time;   // Filter: From time
    dbPluginFilter.calendarTo_Time            = dbQuery.calendarTo_Time;      // Filter: To time
    dbPluginFilter.strDataTypeQuery           = dbQuery.strDataTypeQuery;     // Data type (testing stage) to query
    dbPluginFilter.strTestList                = dbQuery.strTestList;               // List of parameters to extract
    dbPluginFilter.strOptionsString           = dbQuery.strOptionsString;     // Plug-in options string
    dbPluginFilter.SetFilters(dbQuery.strlSqlFilters);              // List of filters/values elements (<filter>=<value>)
    dbPluginFilter.bConsolidatedExtraction    = dbQuery.bConsolidatedExtraction;	// Consolidated extraction or not
    dbPluginFilter.mMinimumPartsInFile        = dbQuery.lMinimumPartsInFile;
    dbPluginFilter.m_gexQueries               = dbQuery.m_gexQueries;             // other options
}

///////////////////////////////////////////////////////////
// Maps Galaxy query to Corporate database query;
///////////////////////////////////////////////////////////
void DatabaseEngine::MapQueryToCorporateFilters(GexDbPlugin_Filter& dbPluginFilter,
                                                const GexDatabaseQuery& dbQuery,
                                                const QStringList & filtersName)
{
    dbPluginFilter.iTimePeriod        = dbQuery.iTimePeriod;              // Time period to consider.

    // GCORE-9201 : missing properties
    dbPluginFilter.iTimeNFactor       = dbQuery.iTimeNFactor;
    dbPluginFilter.m_eTimeStep        = (GexDbPlugin_Filter::eTimeStep) dbQuery.m_eTimeStep;
    // GCORE-9201 : missing properties

    dbPluginFilter.calendarFrom       = dbQuery.calendarFrom;             // Filter: From date
    dbPluginFilter.calendarTo         = dbQuery.calendarTo;               // Filter: To date
    dbPluginFilter.calendarFrom_Time  = dbQuery.calendarFrom_Time;        // Filter: From time
    dbPluginFilter.calendarTo_Time    = dbQuery.calendarTo_Time;          // Filter: To time
    dbPluginFilter.strDataTypeQuery   = dbQuery.strDataTypeQuery;         // Data type (testing stage) to query
    dbPluginFilter.strTestList        = dbQuery.strTestList;              // List of parameters to extract
    dbPluginFilter.strOptionsString   = dbQuery.strOptionsString;         // Plug-in options string
    dbPluginFilter.SetFilters(dbQuery.strlSqlFilters);     // List of filters/values elements (<filter>=<value>)
    dbPluginFilter.SetFields(filtersName);

    dbPluginFilter.m_gexQueries       = dbQuery.m_gexQueries;
}

///////////////////////////////////////////////////////////
// Check if some files needs to be transferred over FTP
///////////////////////////////////////////////////////////
bool DatabaseEngine::HasFtpFiles(const tdGexDbPluginDataFileList &lFiles) const
{
    tdGexDbPluginDataFileListIterator lstIteratorDataFile(lFiles);
    GexDbPlugin_DataFile *	 pFile	= NULL;

    while(lstIteratorDataFile.hasNext())
    {
        pFile = lstIteratorDataFile.next();

        if (pFile)
        {
            // Only consider remote files
            if(pFile->m_bRemoteFile)
                return true;
        }
    };

    return false;
}

///////////////////////////////////////////////////////////
// Remove from the FTP download list all files already present in the local database
///////////////////////////////////////////////////////////
void DatabaseEngine::SimplifyFtpList(GexDatabaseEntry *pDatabaseEntry,tdGexDbPluginDataFileList &cFilesToFtp,QStringList & cMatchingFiles)
{
    // Copy the list of files to FTP locally into a QStringList (so to leverage '.find()' function)
    QStringList				strFtpList;
    bool					bUpdateFtpFile = false;

    tdGexDbPluginDataFileListIteratorMutable lstIteratorDataFile(cFilesToFtp);
    GexDbPlugin_DataFile *	 pFile	= NULL;

    while(lstIteratorDataFile.hasNext())
    {
        pFile = lstIteratorDataFile.next();

        if (pFile)
        {
            // Only consider remote files
            if(pFile->m_bRemoteFile)
                strFtpList += pFile->m_strFileName; // File name (without path)
            else
            {
                // Remove from list if not a remote file
                lstIteratorDataFile.remove();
                delete pFile;
            }
        }
    };

    // Check if list empty
    if(cFilesToFtp.isEmpty())
        return;

    // Read file holding list of files already FTPed over.
    QString strFtpCopies = pDatabaseEntry->PhysicalPath() + GEX_DATABASE_EXTERNAL_FTP_DEF;

    // Read file: if no file exists, then all files listed must be FTPed over.
    QString			strLine, strFtpFilesAlreadyLocal_Short, strFtpFilesAlreadyLocal_GexDbLocation;
    QFile			f(strFtpCopies);
    if(f.open(QIODevice::ReadOnly) == false)
        return;

    // Database definition File exists...read it!
    QTextStream hFile(&f);	// Assign file handle to data stream
    do
    {
        // Get file name already FTPed over
        strLine = hFile.readLine();

        // Each line should have format <short file name>;<full file name in Gex local DB>
        // For compatibility reasons, update file if it contains at least 1 line with the old format (<short file name>), removing all lines with old format
        if(strLine.contains(';') == 0)
            bUpdateFtpFile = true;
        else
        {
            // Extract both fields of the line
            strFtpFilesAlreadyLocal_Short = strLine.section(";", 0, 0);
            strFtpFilesAlreadyLocal_GexDbLocation = strLine.section(";", 1, 1);

            // Check if Database file also in FTP list
            if(strFtpList.indexOf(strFtpFilesAlreadyLocal_Short) > 0)
            {
                // File already exists locally...then remove from the list of files to FTP!
                // and add the full location of the file in the local Gex DB to list of matching files
                lstIteratorDataFile.toFront();
                while(lstIteratorDataFile.hasNext())
                {
                    pFile = lstIteratorDataFile.next();

                    if(pFile->m_strFileName == strFtpFilesAlreadyLocal_Short)
                    {
                        if(QFile::exists(strFtpFilesAlreadyLocal_GexDbLocation))
                        {
                            lstIteratorDataFile.remove();
                            delete pFile;

                            cMatchingFiles.append(strFtpFilesAlreadyLocal_GexDbLocation);
                            break;
                        }
                        else
                            bUpdateFtpFile = true;
                    }
                };
            }
        }
    }
    while(hFile.atEnd() == false);

    // Close file
    f.close();

    // Check if file should be updated
    if(bUpdateFtpFile)
    {
        QStringList				strlLines;
        QStringList::iterator	it;
        if(f.open(QIODevice::ReadOnly) == true)
        {
            hFile.setDevice(&f);
            do
            {
                // Get file name already FTPed over
                strLine = hFile.readLine();

                // For each line, check if correct format (<short file name>;<full file name in Gex local DB>)
                // If so, save the line
                if(strLine.count(';') > 0)
                {
                    strFtpFilesAlreadyLocal_Short = strLine.section(";", 0, 0);
                    strFtpFilesAlreadyLocal_GexDbLocation = strLine.section(";", 1, 1);
                    if(QFile::exists(strFtpFilesAlreadyLocal_GexDbLocation))
                        strlLines.append(strLine);
                }
            }
            while(hFile.atEnd() == false);
            f.close();
            if(f.open(QIODevice::ReadOnly) == true)
            {
                hFile.setDevice(&f);
                for(it=strlLines.begin(); it!=strlLines.end(); it++)
                    hFile << *it << endl;
            }
            f.close();
        }
    }
}

///////////////////////////////////////////////////////////
// Update local FTP file list (holding list of ALL files FTPed over)
///////////////////////////////////////////////////////////
void DatabaseEngine::UpdateFtpList(GexDatabaseEntry *pDatabaseEntry,tdGexDbPluginDataFileList &cDataFiles,GexDatabaseInsertedFilesList &listInsertedFiles,QStringList &strlCorruptedFiles)
{
    // Check if list of files is empty
    if(cDataFiles.isEmpty())
        return;

    // Update file holding list of files already FTPed over.
    QString strFtpCopies = pDatabaseEntry->PhysicalPath() + GEX_DATABASE_EXTERNAL_FTP_DEF;

    QFile f(strFtpCopies);
    if(f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text) == false)
        return;	// Failed adding value to file...disk full?

    // Update Filter table definition File
    QTextStream hFile(&f);	// Assign file handle to data stream

    // Create list of inserted files with short filenames
    // Create list of corrupted files with short filenames
    QStringList                             strlInsertedFiles_Short,strlCorruptedFiles_Short;
    QStringList::iterator                   itCorrupted;
    GexDatabaseInsertedFilesList::iterator  itInserted;
    QFileInfo                               clFileInfo;
    for(itInserted = listInsertedFiles.begin(); itInserted != listInsertedFiles.end(); itInserted++)
    {
        clFileInfo.setFile((*itInserted).m_strSourceFile);
        QString strDebug = clFileInfo.fileName();
        strlInsertedFiles_Short.append(clFileInfo.fileName());
    }
    for(itCorrupted = strlCorruptedFiles.begin(); itCorrupted != strlCorruptedFiles.end(); itCorrupted++)
    {
        clFileInfo.setFile(*itCorrupted);
        strlCorruptedFiles_Short.append(clFileInfo.fileName());
    }

    // Add names of new files FTPed over
    tdGexDbPluginDataFileListIterator   lstIteratorDataFile(cDataFiles);
    GexDbPlugin_DataFile	*             pMatchingFile = NULL;
    int                                 nFileIndex;

    while(lstIteratorDataFile.hasNext())
    {
        pMatchingFile = lstIteratorDataFile.next();

        if (pMatchingFile)
        {
            // File name (without path). Consider only remote files
            if(strlCorruptedFiles_Short.indexOf(pMatchingFile->m_strFileName) < 0 &&
                    pMatchingFile->m_bRemoteFile &&
                    pMatchingFile->m_bTransferOK)
            {
                // File sucessfuly inserted.
                nFileIndex = strlInsertedFiles_Short.indexOf(pMatchingFile->m_strFileName);
                if(nFileIndex != -1)
                {
                    hFile << pMatchingFile->m_strFileName;
                    hFile << ";";
                    hFile << listInsertedFiles[nFileIndex].m_strDestFile;
                    hFile << endl;
                }
            }
        }
    };

    f.close();
}



///////////////////////////////////////////////////////////
// Fill plugin filter object from pFilter
///////////////////////////////////////////////////////////
void DatabaseEngine::QueryFillPluginFilter(GexDbPlugin_Filter & clPluginFilter,
                                           const GexDatabaseFilter &dbFilter)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Fill PluginFilter for query...");


    // Set filters
    QString strField, strFilter;
    clPluginFilter.strlQueryFilters.clear();
    clPluginFilter.mQueryFields.clear();

    // Add query filter ?
    strField = dbFilter.queryFilter();

    // Set field to query
    if (!strField.isEmpty() && strField != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE])
        clPluginFilter.SetFields(QStringList(strField));

    for (int idx = 0; idx < dbFilter.narrowFilters().count(); ++idx)
    {
        QPair<QString,QString> filter = dbFilter.narrowFilters().at(idx);

        if(!filter.first.isEmpty() && (filter.first != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]) && ( ! filter.second.isEmpty() ) &&(filter.second != "*"))
        {
            strFilter = filter.first;
            strFilter += "=";
            strFilter += filter.second ;
            clPluginFilter.strlQueryFilters.append(strFilter);
        }
    }

    // Set other query parameters
    clPluginFilter.strDataTypeQuery   = dbFilter.strDataTypeQuery;
    clPluginFilter.calendarFrom       = dbFilter.calendarFrom;
    clPluginFilter.calendarTo         = dbFilter.calendarTo;
    clPluginFilter.calendarFrom_Time  = dbFilter.calendarFrom_Time;
    clPluginFilter.calendarTo_Time    = dbFilter.calendarTo_Time;
    clPluginFilter.iTimePeriod        = dbFilter.iTimePeriod;
    clPluginFilter.iTimeNFactor       = dbFilter.iTimeNFactor;
    clPluginFilter.m_eTimeStep        = (GexDbPlugin_Filter::eTimeStep) dbFilter.m_eTimeStep;
    clPluginFilter.bConsolidatedData  = dbFilter.bConsolidatedData;
}

///////////////////////////////////////////////////////////
// Get list of parameters found
///////////////////////////////////////////////////////////
QStringList	DatabaseEngine::QueryGetParameterList(const GexDatabaseFilter &dbFilter, bool bParametricOnly)
{
    GexDatabaseEntry    *pDatabaseEntry;
    QStringList         strParameterList;	// Format is pairs of strinngs (test#, test name)

    // Get database object
    pDatabaseEntry = FindDatabaseEntry(dbFilter.strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
        return strParameterList;	// Return empty list

    // Query only applies to External database
    if(!pDatabaseEntry->IsExternal())
        return strParameterList;	// Return empty list

    // Fill external database query object
    GexDbPlugin_Filter clPluginFilter(this);
    QueryFillPluginFilter(clPluginFilter, dbFilter);

    // Execute query on remote database
    pDatabaseEntry->m_pExternalDatabase->QueryTestlist(clPluginFilter,strParameterList,bParametricOnly);

    // Return parameter list matching dataset filter
    return strParameterList;
}

///////////////////////////////////////////////////////////
// Get list of binnings found
///////////////////////////////////////////////////////////
QStringList	DatabaseEngine::QueryGetBinningList(const GexDatabaseFilter &dbFilter,
                                                bool bSoftBin)
{
    GexDatabaseEntry    *pDatabaseEntry;
    QStringList         strBinningList;

    // Get database object
    pDatabaseEntry = FindDatabaseEntry(dbFilter.strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
        return strBinningList;	// Return empty list

    // Query only applies to External database
    if(!pDatabaseEntry->IsExternal())
        return strBinningList;	// Return empty list

    // Fill external database query object
    GexDbPlugin_Filter clPluginFilter(this);
    QueryFillPluginFilter(clPluginFilter, dbFilter);

    // Execute query on remote database
    pDatabaseEntry->m_pExternalDatabase->QueryBinlist(clPluginFilter,strBinningList,bSoftBin,true);

    // Return parameter list matching dataset filter
    return strBinningList;
}

///////////////////////////////////////////////////////////
// Return all products for genealogy reports, with data for
// at least 2 testing stages (use only date in the filter)
///////////////////////////////////////////////////////////
QStringList DatabaseEngine::QueryGetProductList_Genealogy(const GexDatabaseFilter &dbFilter, bool bAllTestingStages)
{
    GexDatabaseEntry    *lDatabaseEntry;
    QStringList         lProductList;

    // Get database object
    lDatabaseEntry = FindDatabaseEntry(dbFilter.strDatabaseLogicalName);
    if(lDatabaseEntry == NULL)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString(" error : DB entry for '%1' null !")
              .arg(dbFilter.strDatabaseLogicalName).toLatin1().data() );
        return lProductList;	// Return empty list
    }

    // Query only applies to External database
    if(!lDatabaseEntry->IsExternal())
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "impossible to query product list on non external DB");
        return lProductList;	// Return empty list
    }

    // Fill external database query object
    GexDbPlugin_Filter lPluginFilter(this);
    lPluginFilter.strlQueryFilters.clear();
    lPluginFilter.strDataTypeQuery   = dbFilter.strDataTypeQuery;
    lPluginFilter.calendarFrom       = dbFilter.calendarFrom;
    lPluginFilter.calendarTo         = dbFilter.calendarTo;
    lPluginFilter.calendarFrom_Time  = dbFilter.calendarFrom_Time;
    lPluginFilter.calendarTo_Time    = dbFilter.calendarTo_Time;
    lPluginFilter.iTimePeriod        = dbFilter.iTimePeriod;

    for (int lIdx = 0; lIdx < dbFilter.narrowFilters().count(); ++lIdx)
    {
        QPair<QString,QString> lFilter = dbFilter.narrowFilters().at(lIdx);
        if(!lFilter.first.isEmpty() && (lFilter.first != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]) && (lFilter.second != "*"))
        {
            QString lFieldValue = lFilter.first;
            lFieldValue += "=";
            lFieldValue += lFilter.second;
            lPluginFilter.strlQueryFilters.append(lFieldValue);
        }
    }

    // Execute query on remote database
    if (!lDatabaseEntry->m_pExternalDatabase->QueryProductList_Genealogy(lPluginFilter,lProductList,bAllTestingStages))
    {
        GS::Gex::Message::warning("",
                                  "Impossible to search for products list.\n"
                                  "Please check you have updated your database "
                                  "to the latest version.");
        return lProductList;
    }

    if (lProductList.empty())
    {
        GS::Gex::Message::warning("",
                                  "No cross testing stage products found !\n"
                                  "Check the TrackingLot field when inserting "
                                  "data.");
        return lProductList;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString(" %1 cross testing stages products found.")
          .arg( lProductList.size())
          .toLatin1().constData());
    // Return product list matching dataset filter
    return lProductList;
}

///////////////////////////////////////////////////////////
// Return all products (use only date in the filter)
///////////////////////////////////////////////////////////
QStringList DatabaseEngine::QueryGetProductList(const GexDatabaseFilter &dbFilter,QString strValue/*=""*/)
{
    GexDatabaseEntry    *pDatabaseEntry;
    QStringList         strlProductList;

    // Get database object
    pDatabaseEntry = FindDatabaseEntry(dbFilter.strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
        return strlProductList;	// Return empty list

    // Query only applies to External database
    if(!pDatabaseEntry->IsExternal())
        return strlProductList;	// Return empty list

    // Fill external database query object
    GexDbPlugin_Filter clPluginFilter(this);
    clPluginFilter.strlQueryFilters.clear();
    clPluginFilter.strDataTypeQuery   = dbFilter.strDataTypeQuery;
    clPluginFilter.calendarFrom       = dbFilter.calendarFrom;
    clPluginFilter.calendarTo         = dbFilter.calendarTo;
    clPluginFilter.calendarFrom_Time  = dbFilter.calendarFrom_Time;
    clPluginFilter.calendarTo_Time    = dbFilter.calendarTo_Time;
    clPluginFilter.iTimePeriod        = dbFilter.iTimePeriod;

    QString strProductFilter = strValue;

    // Execute query on remote database
    pDatabaseEntry->m_pExternalDatabase->QueryProductList(clPluginFilter,strlProductList,strProductFilter);

    // Return product list matching dataset filter
    return strlProductList;
}

// GCORE-994
QString DatabaseEngine::QueryTestConditionsList(const GexDatabaseFilter &dbFilter, QStringList &testConditions)
{
    GexDatabaseEntry *  pDatabaseEntry = NULL;
    //QStringList         testConditions;

    // Get database object
    pDatabaseEntry = FindDatabaseEntry(dbFilter.strDatabaseLogicalName);

    if (!pDatabaseEntry)
        return QString("error: cannot find DB entry %1").arg(dbFilter.strDatabaseLogicalName);

    // Only for external DB
    if(!pDatabaseEntry->IsExternal())
        return "error: DB is not external";

    // Fill external database query object
    GexDbPlugin_Filter dbPluginFilter(this);

    QueryFillPluginFilter(dbPluginFilter, dbFilter);

    // Execute query on remote database
    if (!pDatabaseEntry->m_pExternalDatabase->QueryTestConditionsList(dbPluginFilter, testConditions))
    {
        QString lLastError;
        pDatabaseEntry->m_pExternalDatabase->GetLastError(lLastError);
        return "error: query test conditions failed: "+lLastError;
    }

    return "ok";
}

QList<GexDatabaseQuery> DatabaseEngine::QuerySplit(const GexDatabaseQuery &dbQuery, QString &errorMessage)
{
    QList<GexDatabaseQuery>         dbQueries;
    QStringList                     splits;
    QString                         filter;
    QString                         field;
    QString                         value;

    // Clear error message
    errorMessage.clear();

    // Get database entry
    GexDatabaseEntry * pDatabaseEntry = FindDatabaseEntry(dbQuery.strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
    {
        errorMessage = QString("couldn't find database: %1").arg(dbQuery.strDatabaseLogicalName);
        return dbQueries;
    }

    // Check if the dataset has to be split
    if(dbQuery.mSplitFields.isEmpty())
    {
        dbQueries.append(dbQuery);
        return dbQueries;
    }
    else
    {
        splits = QuerySelectFilter(dbQuery, dbQuery.mSplitFields);

        if(splits.isEmpty())
        {
            dbQueries.append(dbQuery);
            return dbQueries;
        }
    }

    // Create a db query for each set of values retrieved
    for(int index = 0; index < splits.count(); ++index)
    {
        GexDatabaseQuery    dbSplitQuery    = dbQuery;
        QStringList         splitValues     = splits.at(index).split("|");

        // Verify we retrieve as many values as requested fields
        if (dbQuery.mSplitFields.count() == splitValues.count())
        {
            // Create a filter for each field/value
            for(int fieldIdx = 0; fieldIdx < dbQuery.mSplitFields.count(); ++fieldIdx)
            {
                // Add filter for dataset split
                if(pDatabaseEntry->IsExternal() && !dbSplitQuery.bOfflineQuery)
                {
                    // RDB database
                    field   = dbQuery.mSplitFields.at(fieldIdx);
                    value   = splitValues.at(fieldIdx);

                    // Format filter string
                    filter  = field + "=" + value;

                    dbSplitQuery.strlSqlFilters.append(filter);	// Add current split filter
                    dbSplitQuery.strTitle += "[";
                    dbSplitQuery.strTitle += filter;
                    dbSplitQuery.strTitle += "]";

                    // If the split field is a test condition, keep the condition value
                    if (gexReport->IsExistingTestConditions(field))
                        dbSplitQuery.mTestConditions.insert(field, value);
                }
                else
                {
                    // File-based database: add filter for this dataset split
                    dbSplitQuery.setQueryFilter(dbQuery.mSplitFields.at(fieldIdx), splitValues.at(fieldIdx));

                    dbSplitQuery.strTitle += "[";
                    dbSplitQuery.strTitle += dbQuery.mSplitFields.at(fieldIdx);
                    dbSplitQuery.strTitle += "=";
                    dbSplitQuery.strTitle += splitValues.at(fieldIdx);
                    dbSplitQuery.strTitle += "]";
                }
            }
        }

        dbQueries.append(dbSplitQuery);
    }

    return dbQueries;
}

///////////////////////////////////////////////////////////
// Get list of Filter strings matching a filter selection.
///////////////////////////////////////////////////////////
QStringList	DatabaseEngine::QuerySelectFilter(const GexDatabaseFilter& dbFilter)
{
    QStringList cMatchingFilters;

    GSLOG(SYSLOG_SEV_DEBUG, QString("for %1 on %2(%3) over timeperiod %4")
          .arg(dbFilter.queryFilter().toLatin1().data())
          .arg(dbFilter.strDatabaseLogicalName.toLatin1().data())
          .arg(dbFilter.strDataTypeQuery.toLatin1().data())
          .arg(dbFilter.iTimePeriod )
          .toLatin1().constData());

    GexDatabaseEntry *    pDatabaseEntry;
    QDate                 currentDate = QDate::currentDate();
    QDate                 filterDate;
    int                   iDays;

    // Check if a field is selected for the query.
    // The index 0 should be "--Select Filter--" but not always if the filter is mandatory.
    // This is safer to test for the string 'gexLabelFilterChoices[GEX_QUERY_FILTER_NONE]'
    //if (pFilter->pQueryFilter->currentIndex() == 0)
    if (dbFilter.queryFilter() == gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] //probably "--Select Filter--"
            || dbFilter.queryFilter().isEmpty())
        return cMatchingFilters;

    // Get database object
    pDatabaseEntry = FindDatabaseEntry(dbFilter.strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "cant find DatabaseEntry !");
        return cMatchingFilters;	// Failed finding entry!
    }

    // If query over External database, then trigger external query...unless 'Work offline' is checked!
    if(pDatabaseEntry->IsExternal() && (dbFilter.bOfflineQuery == false))
    {
        // Fill external database query object
        GexDbPlugin_Filter clPluginFilter(this);
        QueryFillPluginFilter(clPluginFilter, dbFilter);

        // Execute query on remote database
        if (!pDatabaseEntry->m_pExternalDatabase->QueryField(clPluginFilter, cMatchingFilters))
        {
            QString lLastError;
            pDatabaseEntry->m_pExternalDatabase->GetLastError(lLastError);
            GS::Gex::Message::warning("Query field failed", lLastError);
        }

        // Return matching values
        return cMatchingFilters;
    }

    // Check the Calendar period to scan for
    switch(dbFilter.iTimePeriod)
    {
    case GEX_QUERY_TIMEPERIOD_TODAY:
        FindMatchingFilter(cMatchingFilters, currentDate, dbFilter, pDatabaseEntry);
        break;
    case GEX_QUERY_TIMEPERIOD_LAST2DAYS:
        for(iDays=0;iDays > -2 ;iDays--)
        {
            filterDate = currentDate.addDays(iDays);
            FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST3DAYS:
        for(iDays=0;iDays > -3 ;iDays--)
        {
            filterDate = currentDate.addDays(iDays);
            FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST7DAYS:
        for(iDays=0;iDays > -7 ;iDays--)
        {
            filterDate = currentDate.addDays(iDays);
            FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST14DAYS:
        for(iDays=0;iDays > -14 ;iDays--)
        {
            filterDate = currentDate.addDays(iDays);
            FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST31DAYS:
        for(iDays=0;iDays > -31 ;iDays--)
        {
            filterDate = currentDate.addDays(iDays);
            FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_THISWEEK:
        // From 'Monday' to current date.
        filterDate = currentDate;
        FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
        while(filterDate.dayOfWeek() != 1)
        {
            filterDate = filterDate.addDays(-1);
            FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
        };
        break;
    case GEX_QUERY_TIMEPERIOD_THISMONTH:
        // From 1st day of the month to today...
        filterDate = currentDate;
        FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
        while(filterDate.day() != 1)
        {
            filterDate = filterDate.addDays(-1);
            FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
        };
        break;
    case GEX_QUERY_TIMEPERIOD_CALENDAR:
        // Find all valid years/months/days entries matching From-To dates...
        for(filterDate = dbFilter.calendarFrom; filterDate <= dbFilter.calendarTo; )
        {
            FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
            filterDate = filterDate.addDays(1);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST_N_X:
        // code me
        GSLOG(SYSLOG_SEV_ERROR, "code me");
        GEX_ASSERT(false);
        break;

    case GEX_QUERY_HOUSEKPERIOD_TODAY:
    {
        QDate comboBoxDate = currentDate;
        cMatchingFilters = GetValidateFilter(comboBoxDate, dbFilter);
        break;
    }

    case GEX_QUERY_HOUSEKPERIOD_1DAY:
    {
        QDate comboBoxDate = currentDate.addDays(-1);
        cMatchingFilters = GetValidateFilter(comboBoxDate, dbFilter);

        break;
    }
    case GEX_QUERY_HOUSEKPERIOD_2DAYS:
    {
        QDate comboBoxDate = currentDate.addDays(-2);
        cMatchingFilters = GetValidateFilter(comboBoxDate, dbFilter);

        break;
    }
    case GEX_QUERY_HOUSEKPERIOD_3DAYS:
    {
        QDate comboBoxDate = currentDate.addDays(-3);
        cMatchingFilters = GetValidateFilter(comboBoxDate, dbFilter);

        break;
    }
    case GEX_QUERY_HOUSEKPERIOD_4DAYS:
    {
        QDate comboBoxDate = currentDate.addDays(-4);
        cMatchingFilters = GetValidateFilter(comboBoxDate, dbFilter);

        break;
    }
    case GEX_QUERY_HOUSEKPERIOD_1WEEK:
    {
        QDate comboBoxDate = currentDate.addDays(-7);
        cMatchingFilters = GetValidateFilter(comboBoxDate, dbFilter);

        break;
    }
    case GEX_QUERY_HOUSEKPERIOD_2WEEKS:
    {
        QDate comboBoxDate = currentDate.addDays(-14);
        cMatchingFilters = GetValidateFilter(comboBoxDate, dbFilter);

        break;
    }
    case GEX_QUERY_HOUSEKPERIOD_3WEEKS:
    {
        QDate comboBoxDate = currentDate.addDays(-21);
        cMatchingFilters = GetValidateFilter(comboBoxDate, dbFilter);

        break;
    }
    case GEX_QUERY_HOUSEKPERIOD_1MONTH:
    {
        QDate comboBoxDate = currentDate.addDays(-30);
        cMatchingFilters = GetValidateFilter(comboBoxDate, dbFilter);

        break;
    }
    case GEX_QUERY_HOUSEKPERIOD_CALENDAR:
    {
        // Find all valid years/months/days entries matching From-To dates...
        for(filterDate = dbFilter.calendarFrom; filterDate <= dbFilter.calendarTo; )
        {
            FindMatchingFilter(cMatchingFilters, filterDate, dbFilter,pDatabaseEntry);
            filterDate = filterDate.addDays(1);
        }
        break;
    }

    case GEX_QUERY_TIMEPERIOD_ALLDATES:
    default:
    {
        // Scan all valid folders!
        QDate comboBoxDate = currentDate;
        cMatchingFilters = GetValidateFilter(comboBoxDate, dbFilter);
        break;
    }
    }

    return cMatchingFilters;
}





///////////////////////////////////////////////////////////
// Get list of matching file for filter.
// This funtion reads all files in the data base folder and find files that
// have a later date than the retrieved date from combobox
///////////////////////////////////////////////////////////
QStringList DatabaseEngine::GetValidateFilter(const QDate comboBoxDate, const GexDatabaseFilter& dbFilter)
{
    // Scan all valid folders!
    QStringList cMatchingFilters;
    QDate    filterDate;

    QDir d;
    QString		strFolder,strYearFolder,strMonthFolder;
    QStringList strValidYears,strValidMonths,strValidDays;
    QStringList::Iterator itYear,itMonth,itDay;
    int	iYear,iMonth,iDay, year, month, day;
    bool thisYear(false), thisMonth(false);
    GexDatabaseEntry *    pDatabaseEntry;
    QDate                 currentDate = QDate::currentDate();
    year = comboBoxDate.year();
    month = comboBoxDate.month();
    day = comboBoxDate.day();

    // Get database object
    pDatabaseEntry = FindDatabaseEntry(dbFilter.strDatabaseLogicalName);
    // Build sub-path from database home location.
    strFolder = pDatabaseEntry->PhysicalPath();

    // Find all valid YEAR, Month, Day folders!
    d.setPath(strFolder);
    d.setFilter(QDir::Dirs );
    strValidYears = d.entryList(QStringList() << "*");
    for(itYear = strValidYears.begin(); itYear != strValidYears.end(); ++itYear )
    {
        thisYear = false;
        // Skip folders names that start with a '.'
        if(((*itYear).startsWith(".")) || ((*itYear).toInt() > year))
            continue;

        // List of valid YEARS folders
        iYear = (*itYear).toInt();
        if (iYear == currentDate.year())
            thisYear = true;

        // Move to YEAR folder
        strYearFolder = strFolder + "/" + *itYear;
        d.setPath(strYearFolder);
        strValidMonths = d.entryList(QStringList() << "*");
        for(itMonth = strValidMonths.begin(); itMonth != strValidMonths.end(); ++itMonth )
        {
            thisMonth = false;
            // Skip '.' and '..' folders.
            if((*itMonth == ".") || (*itMonth == "..")
                    || ((*itMonth).toInt() > month && thisYear))
                continue;

            // List of valid MONTHS folders
            iMonth = (*itMonth).toInt();

            if (thisYear && iMonth == currentDate.month())
                thisMonth = true;

            // Move to MONTH folder
            strMonthFolder = strYearFolder + "/" + *itMonth;
            d.setPath(strMonthFolder);
            strValidDays = d.entryList(QStringList() << "*");
            for(itDay = strValidDays.begin(); itDay != strValidDays.end(); ++itDay )
            {
                // Skip '.' and '..' folders.
                if((*itDay == ".") || (*itDay == "..")
                        || ((*itDay).toInt() > day && thisMonth))
                    continue;
                // List of valid Days folders
                iDay = (*itDay).toInt();

                if (iYear < 99) iYear += 1900;
                filterDate.setDate(iYear,iMonth,iDay);
                FindMatchingFilter(cMatchingFilters, filterDate, dbFilter, pDatabaseEntry);
            }
        }
    }
    return cMatchingFilters;
}



///////////////////////////////////////////////////////////
// Get list of Filter strings matching a filter selection,
// based on a cQuery object.
///////////////////////////////////////////////////////////
QStringList DatabaseEngine::QuerySelectFilter(const GexDatabaseQuery &dbQuery, const QStringList &filtersName)
{
    GexDatabaseEntry *    pDatabaseEntry=0;
    QStringList           cMatchingFilters;
    QDate                 currentDate    = QDate::currentDate();
    QDate                 filterDate;
    int                   iDays=0;

    // Check if a field is selected for the query
    if(filtersName.isEmpty())
        return cMatchingFilters;

    // Get database object
    pDatabaseEntry = FindDatabaseEntry(dbQuery.strDatabaseLogicalName);
    if(pDatabaseEntry == NULL)
        return cMatchingFilters;	// Failed finding entry!

    // If query over External database, then trigger external query...unless 'Work offline' is checked!
    if(pDatabaseEntry->IsExternal() && (dbQuery.bOfflineQuery == false))
    {
        // Fill external database query object
        GexDbPlugin_Filter clPluginFilter(this);
        MapQueryToCorporateFilters(clPluginFilter, dbQuery, filtersName);

        // Execute query on remote database
        if (!pDatabaseEntry->m_pExternalDatabase->QueryField(clPluginFilter, cMatchingFilters))
        {
            QString lLastError;
            pDatabaseEntry->m_pExternalDatabase->GetLastError(lLastError);
            GS::Gex::Message::warning("Query field failed", lLastError);
        }

        // Return matching values
        return cMatchingFilters;
    }

    // Check the Calendar period to scan for
    switch(dbQuery.iTimePeriod)
    {
    case GEX_QUERY_TIMEPERIOD_TODAY:
        FindMatchingFilter(cMatchingFilters, currentDate, dbQuery, filtersName,pDatabaseEntry);
        break;
    case GEX_QUERY_TIMEPERIOD_LAST2DAYS:
        for(iDays=0;iDays > -2 ;iDays--)
        {
            filterDate = currentDate.addDays(iDays);
            FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST3DAYS:
        for(iDays=0;iDays > -3 ;iDays--)
        {
            filterDate = currentDate.addDays(iDays);
            FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST7DAYS:
        for(iDays=0;iDays > -7 ;iDays--)
        {
            filterDate = currentDate.addDays(iDays);
            FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST14DAYS:
        for(iDays=0;iDays > -14 ;iDays--)
        {
            filterDate = currentDate.addDays(iDays);
            FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST31DAYS:
        for(iDays=0;iDays > -31 ;iDays--)
        {
            filterDate = currentDate.addDays(iDays);
            FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_LAST_N_X:
        GSLOG(SYSLOG_SEV_WARNING, "GEX_QUERY_TIMEPERIOD_LAST_N_X : CheckMe !");
        switch(dbQuery.m_eTimeStep)
        {
        case GexDatabaseQuery::DAYS:        filterDate = currentDate.addDays(-dbQuery.iTimeNFactor);
            break;

        case GexDatabaseQuery::WEEKS:       filterDate = currentDate.addDays(-dbQuery.iTimeNFactor*7);
            break;

        case GexDatabaseQuery::MONTHS:      filterDate = currentDate.addMonths(-dbQuery.iTimeNFactor);
            break;

        case GexDatabaseQuery::QUARTERS:    filterDate = currentDate.addMonths(-dbQuery.iTimeNFactor*4);
            break;

        case GexDatabaseQuery::YEARS:       filterDate = currentDate.addYears(-dbQuery.iTimeNFactor);
            break;
        }
        FindMatchingFilter(cMatchingFilters,  filterDate, dbQuery, filtersName, pDatabaseEntry);
        break;
    case GEX_QUERY_TIMEPERIOD_THISWEEK:
        // From 'Monday' to current date.
        filterDate = currentDate;
        FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
        while(filterDate.dayOfWeek() != 1)
        {
            filterDate = filterDate.addDays(-1);
            FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
        };
        break;
    case GEX_QUERY_TIMEPERIOD_THISMONTH:
        // From 1st day of the month to today...
        filterDate = currentDate;
        FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
        while(filterDate.day() != 1)
        {
            filterDate = filterDate.addDays(-1);
            FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
        };
        break;
    case GEX_QUERY_TIMEPERIOD_CALENDAR:
        // Find all valid years/months/days entries matching From-To dates...
        for(filterDate = dbQuery.calendarFrom; filterDate <= dbQuery.calendarTo; )
        {
            FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
            filterDate = filterDate.addDays(1);
        }
        break;
    case GEX_QUERY_TIMEPERIOD_ALLDATES:
    default:
        // Scan all valid folders!
        QDir d;
        QString		strFolder,strYearFolder,strMonthFolder;
        QStringList strValidYears,strValidMonths,strValidDays;
        QStringList::Iterator itYear,itMonth,itDay;
        int	iYear,iMonth,iDay;

        // Build sub-path from database home location.
        strFolder = pDatabaseEntry->PhysicalPath();

        // Find all valid YEAR, Month, Day folders!
        d.setPath(strFolder);
        d.setFilter(QDir::Dirs );
        strValidYears = d.entryList(QStringList() << "*");
        for(itYear = strValidYears.begin(); itYear != strValidYears.end(); ++itYear )
        {
            // Skip folders names that start with a '.'
            if((*itYear).startsWith("."))
                goto next_Year_Entry;

            // List of valid YEARS folders
            iYear = (*itYear).toInt();

            // Move to YEAR folder
            strYearFolder = strFolder + "/" + *itYear;
            d.setPath(strYearFolder);
            strValidMonths = d.entryList(QStringList() << "*");
            for(itMonth = strValidMonths.begin(); itMonth != strValidMonths.end(); ++itMonth )
            {
                // Skip '.' and '..' folders.
                if((*itMonth == ".") || (*itMonth == ".."))
                    goto next_Month_Entry;

                // List of valid MONTHS folders
                iMonth = (*itMonth).toInt();

                // Move to MONTH folder
                strMonthFolder = strYearFolder + "/" + *itMonth;
                d.setPath(strMonthFolder);
                strValidDays = d.entryList(QStringList() << "*");
                for(itDay = strValidDays.begin(); itDay != strValidDays.end(); ++itDay )
                {
                    // Skip '.' and '..' folders.
                    if((*itDay == ".") || (*itDay == ".."))
                        goto next_Day_Entry;
                    // List of valid Days folders
                    iDay = (*itDay).toInt();

                    if (iYear < 99) iYear += 1900;
                    filterDate.setDate(iYear,iMonth,iDay);
                    FindMatchingFilter(cMatchingFilters, filterDate, dbQuery, filtersName,pDatabaseEntry);
next_Day_Entry: ;
                }
next_Month_Entry: ;
            }
next_Year_Entry: ;
        }
        break;
    }

    return cMatchingFilters;
}

///////////////////////////////////////////////////////////
// Get index of filter in gexFilterChoices, based on filter name
///////////////////////////////////////////////////////////
int DatabaseEngine::GetFilterIndex(const QString & filterName)
{
    int i=0;
    while(gexFilterChoices[i] != 0)
    {
        if( filterName == gexFilterChoices[i])
            return i;
        i++;
    }; // loop until we have found the string entry.

    return 0;
}

///////////////////////////////////////////////////////////
// Get index of filter in gexFilterChoices, based on filter label
///////////////////////////////////////////////////////////
int DatabaseEngine::GetLabelFilterIndex(const QString & filterLabel)
{
    int i=0;
    while(gexLabelFilterChoices[i] != 0)
    {
        if( filterLabel == gexLabelFilterChoices[i])
            return i;
        i++;
    }; // loop until we have found the string entry.

    return 0;
}

///////////////////////////////////////////////////////////
// Execute Post-insertion Shell...
// return: true on success (no error)
///////////////////////////////////////////////////////////
bool DatabaseEngine::LaunchPostInsertionShell(int nStatus, QString& strErrorMessage,
                                              QtLib::DatakeysContent &dbKeysContent,
                                              CGexMoTaskItem *ptTask)
{
    // If no shell (or shell doesn't exist), simply quietly return.
    QString	strShellFile = ptTask->GetDataPumpData()->strBatchToExecuteAfterInsertion;
    if((QFile::exists(strShellFile) == false) || !ptTask)
        return true;

    QString Value;
    QString strArgumentsLine;

    // Arg1: insertion status
    strArgumentsLine = QString::number(nStatus);

    // Arg2: Date & Time argument...
    strArgumentsLine += " " + GS::Gex::Engine::GetInstance().GetClientDateTime().toString(Qt::ISODate);

    // Arg3: ProductID...
    if(dbKeysContent.Get("Product").toString().isEmpty())
        strArgumentsLine += " ?";
    else
        strArgumentsLine += " " + FormatArgumentScriptString(dbKeysContent.Get("Product").toString());

    // Arg4: LotID...
    if(dbKeysContent.Get("Lot").toString().isEmpty())
        strArgumentsLine += " ?";
    else
        strArgumentsLine += " " + FormatArgumentScriptString(dbKeysContent.Get("Lot").toString());

    // Arg5: TrackingLotID...
    if(dbKeysContent.Get("TrackingLot").toString().isEmpty())
        strArgumentsLine += " ?";
    else
        strArgumentsLine += " " + FormatArgumentScriptString(dbKeysContent.Get("TrackingLot").toString());

    // Arg6: SubLotID...
    if(dbKeysContent.Get("SubLot").toString().isEmpty())
        strArgumentsLine += " ?";
    else
        strArgumentsLine += " " + FormatArgumentScriptString(dbKeysContent.Get("SubLot").toString());

    // Arg7: WaferID...
    if(dbKeysContent.Get("Wafer").toString().isEmpty())
        strArgumentsLine += " ?";
    else
        strArgumentsLine += " " + FormatArgumentScriptString(dbKeysContent.Get("Wafer").toString());

    // Arg8: Tester name...
    if(dbKeysContent.Get("TesterName").toString().isEmpty())
        strArgumentsLine += " ?";
    else
        strArgumentsLine += " " + FormatArgumentScriptString(dbKeysContent.Get("TesterName").toString());

    // Arg9: Operator...
    if(dbKeysContent.Get("Operator").toString().isEmpty())
        strArgumentsLine += " ?";
    else
        strArgumentsLine += " " + FormatArgumentScriptString(dbKeysContent.Get("Operator").toString());

    // Arg10: Source Data file name...
    Value = dbKeysContent.Get("FileName").toString();
    if(Value.isEmpty())
        strArgumentsLine += " ?";
    else
        strArgumentsLine += " " + FormatArgumentScriptString(Value);

    // Arg11: Dest Data file name (after move/rename)...
    Value = dbKeysContent.Get("FullDestinationName").toString();
    if(Value.isEmpty())
        strArgumentsLine += " ?";
    else
        strArgumentsLine += " " + FormatArgumentScriptString(Value);

    // Arg12: YM insertion error message...
    if(strErrorMessage.isEmpty())
        strArgumentsLine += " ?";
    else
        strArgumentsLine += " " + FormatArgumentScriptString(strErrorMessage);

    // Launch Shell command (minimized)
#ifdef _WIN32
    // Replace '/' to '\' to avoid MS-DOS compatibility issues
    strArgumentsLine = strArgumentsLine.replace('/','\\');

    ShellExecuteA(NULL,
                  "open",
                  strShellFile.toLatin1().constData(),
                  strArgumentsLine.toLatin1().constData(),
                  NULL,
                  SW_SHOWMINIMIZED);
#else
    strShellFile = strShellFile + " " + strArgumentsLine;
    if (system(strShellFile.toLatin1().constData()) == -1) {
        return false;
    }
#endif

    // Success
    return true;
}

bool DatabaseEngine::UpdateAdrDatabaseConnector(GexDbPlugin_Connector &adrSettingConnector, const GexDbPlugin_Connector *tdrSettingConnector)
{
    // For AdrType
    // Give the convention name from the tdrName
    QString lTdrName = tdrSettingConnector->m_strUserName_Admin;
    QString lAdrName;
    QString lTdrUserName = tdrSettingConnector->m_strUserName;
    QString lAdrUserName;
    if(lTdrName.contains("gexdb"))
        lAdrName = lTdrName.replace("gexdb", "adr");
    else
        lAdrName = "adr_"+lTdrName;
    if(lTdrUserName.contains("gexdb"))
        lAdrUserName = lTdrUserName.replace("gexdb", "adr");
    else
        lAdrUserName = "adr_user_"+lTdrName+"";
    adrSettingConnector.m_bAdminUser = true;
    adrSettingConnector.m_strDatabaseName = lAdrName;
    adrSettingConnector.m_strUserName_Admin = lAdrName;
    adrSettingConnector.m_strPassword_Admin = "adradmin";
    adrSettingConnector.m_strUserName = lAdrUserName;
    adrSettingConnector.m_strPassword = "adruser";
    adrSettingConnector.m_strSchemaName = lAdrName;

    // Then update the connection settings
    adrSettingConnector.m_strHost_Name = tdrSettingConnector->m_strHost_Name;
    adrSettingConnector.m_strHost_Unresolved = tdrSettingConnector->m_strHost_Unresolved;
    adrSettingConnector.m_strHost_IP = tdrSettingConnector->m_strHost_IP;
    adrSettingConnector.m_strDriver = tdrSettingConnector->m_strDriver;
    adrSettingConnector.m_uiPort = tdrSettingConnector->m_uiPort;

    return true;
}

///////////////////////////////////////////////////////////
// Get the ADR dabase from the TDR dabase name
///////////////////////////////////////////////////////////
GexDatabaseEntry* DatabaseEngine::GetADRDatabaseFromTDR(const QString& tdrDatabaseName)
{
    GexDatabaseEntry	*lADRDatabaseEntry = NULL, *lTdrDatabaseEntry = NULL;
    QString lADRDataBAseName;

    // Extract database name
    lTdrDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(tdrDatabaseName,false);
    if(lTdrDatabaseEntry == NULL)
        return NULL;
    if(lTdrDatabaseEntry->m_pExternalDatabase
            && lTdrDatabaseEntry->m_pExternalDatabase->GetPluginID()
            && lTdrDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin)
    {
        lADRDataBAseName = lTdrDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->GetAdrLinkName();
        // Find database ptr
        lADRDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(lADRDataBAseName,false);
    }

    return lADRDatabaseEntry;
}

///////////////////////////////////////////////////////////
// Get the ADR dabase name from the TDR dabase name
///////////////////////////////////////////////////////////
const QString DatabaseEngine::GetADRDBNameFromTDRDBName(const QString& tdrDatabaseName)
{
    GexDatabaseEntry	*lADRDatabaseEntry = NULL;
    lADRDatabaseEntry = GetADRDatabaseFromTDR(tdrDatabaseName);
    if (lADRDatabaseEntry)
        return (lADRDatabaseEntry->LogicalName());
    else
        return (QString());
}


} // namespace GS
} // namespace Gex
