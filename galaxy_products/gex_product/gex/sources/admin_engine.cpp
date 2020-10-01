///////////////////////////////////////////////////////////
// Database admin: Create/Delete database, insert files
#include <QShortcut>
#include <QMenu>
#include <QInputDialog>
#include <QSqlError>
#include <QSqlRecord>
#include <QSpinBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QRadioButton>
#include <QToolButton>
#include <QProcess>
#include <QHostInfo>

#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "dir_access_base.h"
#include "db_external_database.h"
#include "admin_engine.h"  // YieldManDB
#include "gex_shared.h"
#include "db_engine.h"
#include "scheduler_engine.h"
#include "mo_task.h"
#include "browser_dialog.h"
#include "report_options.h"
#include "gex_constants.h"
#include "product_info.h"
#include "gex_database_entry.h"
#include "engine.h"
#include "libgexoc.h"
#include "read_system_info.h"
#include "command_line_options.h"

namespace GS
{
namespace Gex
{
///////////////////////////////////////////////////////////
// AdminEngine class: database connection...
///////////////////////////////////////////////////////////

// Error map
GBEGIN_ERROR_MAP(AdminEngine)
// General
GMAP_ERROR(eMarkerNotFound,"Marker not found: %s.")
// Database errors
GMAP_ERROR(eDB_Connect,"Error connecting to database %s on host (%s, %s):\n%s.")
GMAP_ERROR(eDB_CreateConnector,"Error creating database connector for driver %s, using connection name %s.")
GMAP_ERROR(eDB_NotUpToDate,"%s.")
GMAP_ERROR(eDB_UnderMaintenance,"%s.")
GMAP_ERROR(eDB_NoConnection,"Lost connection to database %s.")
GMAP_ERROR(eDB_NoFields,"Table %s has no fields.")
GMAP_ERROR(eDB_InvalidTable,"Table %s does not exist.")
GMAP_ERROR(eDB_InvalidField,"Field %s does not exist in table %s.")
GMAP_ERROR(eDB_InvalidEntry,"%s.")
GMAP_ERROR(eDB_Query,"Error executing SQL query.\nQUERY=%s\nERRORNB=%s\nERRORMSG=%s")
GEND_ERROR_MAP(AdminEngine)

// PAT-55
// Enter here all NODE options
// syntax:
// OPTION_NAME [#TYPE=<TYPE>] [#DEFAULT=DefaultValue] [#DESCRIPTION=Description]
// TYPE: NUMBER, BOOLEAN, STRING, CHAR, <LIST>
// LIST: <TYPE> [|<TYPE>]+ or STRING [|STRING]+
const QStringList AdminEngine::sNodeSettingsString=QStringList()
        << "LOADBALANCING_FILE_SELECTION_PRIORITY"\
           "#TYPE=NUMBER|NUMBER|NUMBER"\
           "#DEFAULT=1|10|100"\
           "#DESCRIPTION=LOAD BALANCING PRIORITY OPTION\n ex: 1|10|100\n ° 1   file  for Priority 0 (Low)\n ° 10  files for Priority 1 (Medium)\n ° 100 files for Priority 2 (High)\n -1 = no limit ex: 1|100|-1 "
        << "LOADBALANCING_FILE_SELECTION_AGE_EXCLUSION"\
           "#TYPE=NUMBER"\
           "#DEFAULT=15"\
           "#DESCRIPTION=Files oldest than X seconds can be processed "
        << "LOADBALANCING_QUARANTINE_HOUSEKEEPING_OPTION"\
           "#TYPE=STRING"\
           "#DESCRIPTION= Quarantine after a Gex unexpected result\n ex: \n ° *.quarantine         => rename datapump/file.stdf to datapump/file.stdf.quarantine\n ° /quarantineFolder/*  => move datapump/file.stdf to datapump/quarantineFolder/file.stdf\n ° // C:/quarantineFolder/* => move datapump/file.stdf to C:/quarantineFolder/file.stdf"
        << "LOADBALANCING_QUARANTINE_RETRY_OPTION"\
           "#TYPE=NUMBER"\
           "#DESCRIPTION= The number of time we have to retry to process this file after a Gex unexpected result. The default value is 0"
        << "LOADBALANCING_QUARANTINE_EMAIL_OPTION"\
           "#TYPE=STRING"\
           "#DESCRIPTION= The mail to send after a Gex unexpected result"
        << "MONITORING_LOGS_FOLDER";

const QStringList AdminEngine::sSettingsString=QStringList()
        << "DB_VERSION_NAME"\
           "#TYPE=READONLY"\
           "#DESCRIPTION=Yield-Man Administration Server name"
        << "DB_VERSION_NB"\
           "#TYPE=READONLY"\
           "#DESCRIPTION=Yield-Man Administration Server version"
        << "DB_BUILD_NB"\
           "#TYPE=READONLY"\
           "#DESCRIPTION=Yield-Man Administration Server build"
        << "DB_MN_BEFORE_DISCONNECT"\
           "#TYPE=NUMBER"\
           "#DESCRIPTION=Number of minutes before to auto-disconnect a YieldMan user inactivity. -1 to disabled this option"
        << "BI_SERVER"
        << "LDAP_SERVER"
        << "DB_LOADBALANCING_OPTION"\
           "#TYPE=ENABLED|DISABLED"\
           "#DEFAULT=ENABLED"\
           "#DESCRIPTION=Disable the Load Balancing process"
        << "TDR_CONNECT_LOCALHOST"\
           "#TYPE=ENABLED|DISABLED"\
           "#DEFAULT=DISABLED"\
           "#DESCRIPTION=Force to use 'localhost' for TDR connection"
        << "ADMIN_CONNECT_LOCALHOST"\
           "#TYPE=ENABLED|DISABLED"\
           "#DEFAULT=DISABLED"\
           "#DESCRIPTION=Force to use local 'HostName' for ym_admin_db connection";

const QStringList AdminEngine::sUserSettingsString=QStringList()
        << "DB_MN_BEFORE_DISCONNECT"\
           "#TYPE=NUMBER"\
           "#DEFAULT=1"\
           "#DESCRIPTION=Number of minutes before to auto-disconnect a YieldMan user inactivity. -1 to disabled this option"
        << "SHOW_ALL_USERS_RULES"\
           "#TYPE=BOOLEAN"\
           "#DEFAULT=TRUE"\
           "#DESCRIPTION=Use to hide or show rules from other users"
        << "SHOW_ONLY_ENABLED_RULES"\
           "#TYPE=BOOLEAN"\
           "#DEFAULT=FALSE"\
           "#DESCRIPTION=Use to hide or show disabled rules";

//////////////////////////////////////////////////////////////////////
// AdminEngine
//////////////////////////////////////////////////////////////////////
// Connect YieldMan (not PAT-Man ?) to the Database (which one ?)
// Manage all information about :
//    - users
//    - groups
//    - profiles
//    - tasks
//////////////////////////////////////////////////////////////////////
AdminEngine::AdminEngine(QObject* parent):QObject(parent)
{
    setObjectName("GSAdminEngine");

    m_nNodeId = -1;
    mIsLoadBalancingActive = false;

    mValidityCheckInProgress = false;
    mValidityCheckTriggerConnected = false;

    mDirAccessPlugin = NULL;
    m_pCurrentUser = NULL;
    m_pCurrentGroup = NULL;
    m_pCurrentProfile = NULL;

    m_bFirstActivation = false;

    m_pDatabaseConnector = NULL;

    m_nServerMajorVersion = YIELDMANDB_VERSION_MAJOR;
    m_nServerMinorVersion = YIELDMANDB_VERSION_MINOR;
    m_nServerBuild = YIELDMANDB_BUILD_NB;
    m_strServerVersionName = QString(YIELDMANDB_VERSION_NAME);

    m_LastDbUsersUpdateChecksum = 0;
    m_LastDbUsersIdChecksum = 0;

}

AdminEngine::~AdminEngine()
{
    if (mDirAccessPlugin)
    {
        delete mDirAccessPlugin;
        mDirAccessPlugin = NULL;
    }

    while(m_mapUsers.count() > 0)
        delete m_mapUsers.take(m_mapUsers.begin().key());

    while(m_mapGroups.count() > 0)
        delete m_mapGroups.take(m_mapGroups.begin().key());

    while(m_mapProfiles.count() > 0)
        delete m_mapProfiles.take(m_mapProfiles.begin().key());

    delete m_pDatabaseConnector;
    m_pDatabaseConnector = NULL;
}

void AdminEngine::SetLastError(QSqlQuery &clQuery, bool RetryOnTimeOut/*=false*/)
{

    SetLastError(eDB_Query,
                 clQuery.lastQuery().left(1024),
                 QString::number(clQuery.lastError().number()),
                 clQuery.lastError().text());

    // DeadLock catch
    if((clQuery.lastError().number() == 1213)
            || (clQuery.lastError().number() == 1205))
    {
        // DeadLock Error: 1213 SQLSTATE: 40001 (ER_LOCK_DEADLOCK)
        // Deadlock found when trying to get lock; try restarting transaction

        // ERROR 1205 SQLSTATE:HY000 (ER_LOCK_WAIT_TIMEOUT)
        // Lock wait timeout exceeded; try restarting transaction
        // Message: Result consisted of more than one row
        // DML: Data manipulation language, a set of SQL statements for performing insert, update, and delete operations
        // DML statements for an InnoDB table operate in the context of a transaction, so their effects can be committed or rolled back as a single unit.
        // AutoCommit: A setting that causes a commit operation after each SQL statement.
        // By default, MySQL uses the autocommit setting, which automatically issues a commit following each SQL statement.

        if(clQuery.lastError().number() == 1205)
            GSLOG(SYSLOG_SEV_ERROR,"LOCK WAIT TIMEOUT DETECTED");
        if(clQuery.lastError().number() == 1213)
            GSLOG(SYSLOG_SEV_ERROR,"DEADLOCK DETECTED");

        // For each Deadlock or Timeout, query need to be restarted
        QString lExecutedQuery = clQuery.lastQuery();
        // Dump MySql DeadLock Status
        QString lValues;
        clQuery.exec("show engine innodb status");
        clQuery.first();
        if(!clQuery.value(2).toString().isEmpty())
        {
            GSLOG(SYSLOG_SEV_DEBUG, (QString("QUERY=%1 STATUS=%2").arg(
                                          clQuery.lastQuery().toLatin1().constData())
                                      .arg(clQuery.value(2).toString().toLatin1().constData())).toLatin1().constData());
        }
        clQuery.exec("show variables like '%timeout%'");
        while(clQuery.next())
            lValues += "\n"+clQuery.value(0).toString()+"="+clQuery.value(1).toString();
        clQuery.exec("show variables like '%commit%'");
        while(clQuery.next())
            lValues += "\n"+clQuery.value(0).toString()+"="+clQuery.value(1).toString();
        GSLOG(SYSLOG_SEV_DEBUG, (QString("QUERY=%1 STATUS=%2").arg("show variables")
                                  .arg(lValues.toLatin1().constData())).toLatin1().constData());
        // Clear to not affect the clQuery
        clQuery.clear();
        if(RetryOnTimeOut)
        {
            // This query must be executed
            // Retry to execute the query
            while(true)
            {
                // Check if the query was correctly executed
                // Then exit on Success
                {
                    GSLOG(SYSLOG_SEV_ERROR,"try restarting transaction - QUERY PASS")
                    break;
                }
                // Check if the query failed for some other reason than a TIMEOUT
                // Then exit on Fail
                if((clQuery.lastError().number() != 1213)
                        && (clQuery.lastError().number() != 1205))
                    break;
                // Else try again
                GSLOG(SYSLOG_SEV_ERROR,"try restarting transaction - QUERY FAIL AGAIN")
            }
        }
    }
    else if(clQuery.lastError().number() == 1172)
    {
        // Error: 1172 SQLSTATE: 42000 (ER_TOO_MANY_ROWS)
        // Message: Result consisted of more than one row
        GSLOG(SYSLOG_SEV_ERROR,"TOO MANY ROWS DETECTED");
    }
    else if(clQuery.lastError().number() == 1329)
    {
        // Error: 1329 SQLSTATE: 02000 (ER_SP_FETCH_NO_DATA)
        // Message: No data - zero rows fetched, selected, or processed
        GSLOG(SYSLOG_SEV_ERROR,"NO DATA DETECTED");
    }
    else if((clQuery.lastError().number() == 2006)
            || (clQuery.lastError().number() == 2013))
    {
        // Error: 2006 (CR_SERVER_GONE_ERROR)
        // Message: MySQL server has gone away
        // Error: 2013
        // Message: Lost connection to MySQL server during query
        GSLOG(SYSLOG_SEV_ERROR,"SERVER HAS GONE AWAY");
    }
}

void AdminEngine::SetLastError(int nErrorType,QString strArg1,QString strArg2,
                               QString strArg3,QString strArg4)
{
    switch(nErrorType)
    {
    case eMarkerNotFound:
        GSET_ERROR1(AdminEngine, eMarkerNotFound, NULL,strArg1.toLatin1().constData());
        break;
    case eDB_Connect:
        GSET_ERROR4(AdminEngine, eDB_Connect, NULL,strArg1.toLatin1().constData(),
                    strArg2.toLatin1().constData(),strArg3.toLatin1().constData(),strArg4.toLatin1().constData());
        break;
    case eDB_CreateConnector:
        GSET_ERROR2(AdminEngine, eDB_CreateConnector, NULL,strArg1.toLatin1().constData(),
                    strArg2.toLatin1().constData());
        break;
    case eDB_NotUpToDate:
        GSET_ERROR1(AdminEngine, eDB_NotUpToDate, NULL,strArg1.toLatin1().constData());
        break;
    case eDB_UnderMaintenance:
        GSET_ERROR1(AdminEngine, eDB_UnderMaintenance, NULL,strArg1.toLatin1().constData());
        break;
    case eDB_NoConnection:
        GSET_ERROR1(AdminEngine, eDB_NoConnection, NULL,strArg1.toLatin1().constData());
        break;
    case eDB_NoFields:
        GSET_ERROR1(AdminEngine, eDB_NoFields, NULL,strArg1.toLatin1().constData());
        break;
    case eDB_InvalidTable:
        GSET_ERROR1(AdminEngine, eDB_InvalidTable, NULL,strArg1.toLatin1().constData());
        break;
    case eDB_InvalidField:
        GSET_ERROR2(AdminEngine, eDB_InvalidField, NULL,strArg1.toLatin1().constData(),
                    strArg2.toLatin1().constData());
        break;
    case eDB_InvalidEntry:
        GSET_ERROR1(AdminEngine, eDB_InvalidEntry, NULL,strArg1.toLatin1().constData());
        break;
    case eDB_Query:
        GSET_ERROR3(AdminEngine, eDB_Query, NULL,
                    strArg1.toLatin1().constData(),
                    strArg2.toLatin1().constData(),
                    strArg3.toLatin1().constData());
        break;
    }

    // LOG the error
    QString lError;
    GetLastError(lError);
    GSLOG(SYSLOG_SEV_ERROR, lError.toLatin1().constData());
}

void AdminEngine::GetLastError(QString & strError)
{
    strError = GGET_LASTERRORMSG(AdminEngine,this);
}

/******************************************************************************!
 * \fn GetAdminServerConnector
 * \return pointer
 ******************************************************************************/
GexDbPlugin_Connector* AdminEngine::GetAdminServerConnector()
{
    if (m_pDatabaseConnector == NULL)
    {
        m_pDatabaseConnector =
                new GexDbPlugin_Connector("YieldManDb Admin Connector");
    }

    return m_pDatabaseConnector;
}


void AdminEngine::InitAdminServerConnector()
{
    if(m_pDatabaseConnector == NULL)
        m_pDatabaseConnector = new GexDbPlugin_Connector("YieldManDb Admin Connector");

    if(m_pDatabaseConnector)
    {
        m_pDatabaseConnector->m_bAdminUser = true;
        m_pDatabaseConnector->m_strHost_Name = "localhost";
        m_pDatabaseConnector->m_strHost_Unresolved = "localhost";
        m_pDatabaseConnector->m_strHost_IP = "127.0.0.1";
        m_pDatabaseConnector->m_strDriver = "QMYSQL3";
        m_pDatabaseConnector->m_strDatabaseName = "ym_admin_db";
        m_pDatabaseConnector->m_strUserName_Admin = "ym_admin_db";
        m_pDatabaseConnector->m_strPassword_Admin = "";
        m_pDatabaseConnector->m_strUserName = "ym_admin_db";
        m_pDatabaseConnector->m_strPassword = "";
        m_pDatabaseConnector->m_strSchemaName = "ym_admin_db";
        m_pDatabaseConnector->m_uiPort = 3306;
    }
}


/******************************************************************************!
 * \fn DeleteAdminServerConnector
 * \return pointer
 ******************************************************************************/
void AdminEngine::DeleteAdminServerConnector()
{
    delete m_pDatabaseConnector;
    m_pDatabaseConnector = NULL;
}

bool AdminEngine::CreationAdminServerDb()
{
    // Read file content + copy into our memory buffer list.
    GexDatabaseEntry clNewDatabaseEntry(this);
    clNewDatabaseEntry.SetLocalDB(true);
    clNewDatabaseEntry.SetCacheSize(0.0);  // Empty database.

    // build XML file path to External database settings (login, mappinng tables...)
    clNewDatabaseEntry.m_pExternalDatabase = new GexRemoteDatabase();

    QString strPluginFile = "gexdb_plugin_galaxy.dll";
    QString strPluginName = "YIELDMANDB ";
    if(!clNewDatabaseEntry.m_pExternalDatabase->LoadPluginID(strPluginFile, 1, strPluginName))
    {
        return false;  // Corrupted file.
    }
    // Initialize and Check Remote connection
    GexDbPlugin_ID *pPlugin = clNewDatabaseEntry.m_pExternalDatabase->GetPluginID();
    if(pPlugin && pPlugin->m_pPlugin)
    {
        QStringList strlAllowedDrivers;
        // GCORE-1151 : Remove Oracle traces from V7.3 package
        //        strlAllowedDrivers.append("QOCI8");
        //        strlAllowedDrivers.append("QOCI");
        strlAllowedDrivers.append("QMYSQL3");

        //m_pDatabaseConnector->AddAllowedDrivers(strlAllowedDrivers);
        pPlugin->m_pPlugin->m_pclDatabaseConnector = m_pDatabaseConnector;
        QMap<QString, QString>      lProductInfoMap;
        QMap<QString, QString>      lGuiOptionsMap;

        pPlugin->m_pPlugin->SetTdrLinkName(clNewDatabaseEntry.LogicalName());
        // Set  product info
        lProductInfoMap.insert("product_id",
                               QString::number(GS::LPPlugin::ProductInfo::getInstance()->getProductID()));
        lProductInfoMap.insert("optional_modules",
                               QString::number(GS::LPPlugin::ProductInfo::getInstance()->getOptionalModules()));
        // Set GUI options
        lGuiOptionsMap.insert("open_mode","creation");
        lGuiOptionsMap.insert("read_only","no");
        lGuiOptionsMap.insert("allowed_database_type",GEXDB_ADMIN_DB_KEY);
        if(!pPlugin->m_pPlugin->ConfigWizard(lProductInfoMap, lGuiOptionsMap))
        {
            QString strMessage;
            pPlugin->m_pPlugin->GetLastError(strMessage);

            if(!strMessage.isEmpty())
            {
                strMessage = "Database Creation error !\n\n" + strMessage;
                pPlugin->m_pPlugin->m_pclDatabaseConnector = NULL;

                SetLastError(eDB_InvalidEntry,strMessage);
                return false;
            }
        }
        pPlugin->m_pPlugin->m_pclDatabaseConnector = NULL;

    }

    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return false;
    }

    m_bFirstActivation = true;
    m_pDatabaseConnector->Disconnect();
    return ConnectToAdminServer();
}


bool AdminEngine::ConnectToAdminServer()
{
    // Connect to the Admin Database Server
    if(!ConnectToAdminServerDb())
    {
        DisconnectCurrentUser();
        return false;
    }
    // Then Connect the DA plugin
    if(!ConnectToDaServer())
    {
        DisconnectCurrentUser();
        return false;
    }

    return true;
}

bool AdminEngine::ConnectToDaServer()
{
    // Load the Da
    if (!mDirAccessPlugin)
        LoadDirectoryAccessPlugin();

    // Check if DirAccess is also alive
    if (mDirAccessPlugin &&
            mDirAccessPlugin->GetConnector() &&
            ! mDirAccessPlugin->GetConnector()->IsConnected())
    {
        // Try to reconnect
        if (! LoadDirectoryAccessPlugin())
            return false;
    }

    return true;
}

bool AdminEngine::UpdateAdminServerDb()
{
    /////////////////////
    // Is Up to Date
    if (!IsServerUpToDate())
    {
        // Must start the DA to allow the user connection
        if (!ConnectToDaServer())
            return false;

        // For first installation, m_bFirstActivation = true
        // DB is not uptodate
        // until the B7 - minor update - do not tell to the user
        // for the B8 - major update - load-balancing -
        // display the WelcomeDialog if not a new installation
        m_strUpdateDbLogFile = "";

        if (! AdminServerUpdate())
            return false;
    }

    return true;
}

/******************************************************************************!
 * \fn ConnectToAdminServer
 * \return false on error
 ******************************************************************************/
bool AdminEngine::ConnectToAdminServerDb()
{
    if (m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return false;
    }

    // Check if the connector is alive
    if (m_pDatabaseConnector->IsConnected())
        return true;

    // Close All
    // Force to disconnect the current user if not already done
    DisconnectCurrentUser();
    m_pDatabaseConnector->Disconnect();

    // Open a Sql connection
    if (! m_pDatabaseConnector->Connect())
    {
        QString strError;
        m_pDatabaseConnector->GetLastError(strError);
        SetLastError(eDB_Connect,
                     m_pDatabaseConnector->m_strDatabaseName,
                     m_pDatabaseConnector->m_strHost_Name,
                     QString::number(m_pDatabaseConnector->m_uiPort), strError);
        return false;
    }

    // System identification

    // All system info: hostname, host id, board id, etc.
    ReadSystemInfo cSystemInfo;
    if (m_strAllCpu.isEmpty()
            && cSystemInfo.isSystemInfoAvailable() == READ_SYS_INFO_NOERROR)
    {
        m_strAllCpu = QString::fromLatin1(cSystemInfo.strNetworkBoardsIDs.
                                          toLower().toLatin1());
        m_strCpu = QString::fromLatin1(cSystemInfo.strNetworkBoardID.
                                       toLower().toLatin1());
        m_strHostName = QString::fromLatin1(cSystemInfo.strHostName.toLatin1());
        m_strHostId = QString::fromLatin1(cSystemInfo.strHostID.toLatin1());
        m_strOs = QString::fromLatin1(cSystemInfo.strPlatform.toLatin1());
        m_strOsLogin =
                QString::fromLatin1(cSystemInfo.strAccountName.toLatin1());

        // If we cannot retrieve the HostIp
        // Try with this method
        if (m_strHostId.startsWith("-") ||
                m_strHostId == "0" ||
                m_strHostId == "?")
        {
            QHostInfo clHostInfo = QHostInfo::fromName(m_strHostName);
            QHostAddress clHostAddress(m_strHostName);
            if (clHostInfo.error() == QHostInfo::NoError)
            {
                m_strHostId = "";
                // Get first IPV4 address
                for (int i = 0; i < clHostInfo.addresses().size(); ++i)
                {
                    clHostAddress = clHostInfo.addresses().at(i);
                    if (clHostAddress.protocol() ==
                            QAbstractSocket::IPv4Protocol)
                    {
                        if (! clHostAddress.toString().startsWith("127"))
                        {
                            m_strHostId = clHostAddress.toString();
                            break;
                        }
                    }
                }
            }
        }

        GSLOG(SYSLOG_SEV_DEBUG,
              QString("All CPU=%1").arg(m_strAllCpu).toLatin1().data());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("CPU=%1").arg(m_strCpu).toLatin1().data());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("HostName=%1").arg(m_strHostName).toLatin1().data());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("HostId=%1").arg(m_strHostId).toLatin1().data());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Os=%1").arg(m_strOs).toLatin1().data());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("OsLogin=%1").arg(m_strOsLogin).toLatin1().data());
    }

    QString lQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    QString strValue;
    // Load Global info (Settings Options)

    /// For spider environement ///
    // Force the connection to the localhost
    // and use the localHostName
    // We need first a connection to have the setting value
    if (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() &&
            GetSettingsValue("ADMIN_CONNECT_LOCALHOST", strValue) &&
            (strValue.toUpper() == "ENABLED"))
    {

        SetAttribute("ADMIN_CONNECT_LOCALHOST", QVariant("ENABLED"));
        // Force the connection to use the local HostName connection
        // To force the connection to HostName, use the LastUsed field
        if (m_pDatabaseConnector->m_strHost_LastUsed != "localhost")
        {
            m_pDatabaseConnector->m_strHost_LastUsed = "localhost";

            m_pDatabaseConnector->Disconnect();
            m_pDatabaseConnector->Connect();
            // The m_strHost_LastUsed connect the last name used
            // for a success connection
            // If failed, don't try with the original connection
            if (! m_pDatabaseConnector->IsConnected() ||
                    (m_pDatabaseConnector->m_strHost_LastUsed != "localhost"))
            {
                m_pDatabaseConnector->Disconnect();
                SetLastError(eDB_Connect,
                             m_pDatabaseConnector->m_strDatabaseName,
                             m_pDatabaseConnector->m_strHost_Name,
                             QString::number(m_pDatabaseConnector->m_uiPort),
                             QString("Cannot connect to the SPIDER ADMIN "
                                     "using LOCALHOST for %1").
                             arg(m_strHostName));
                return false;
            }
        }
    }

    // Now get the version of the database
    GetServerVersion();

    /////////////////////
    // Get Mutex
    lQuery = "SELECT CONCAT(ID,':',HOST), @@hostname ";
    lQuery += "FROM information_schema.PROCESSLIST ";
    lQuery += " WHERE USER='" + m_pDatabaseConnector->m_strUserName_Admin + "'";
    lQuery += " AND ID=connection_id()";
    if (! clQuery.exec(lQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    clQuery.first();

    // Construct the new mutex that contains the SQL session ID and the HOST
    QString lMutex = clQuery.value(0).toString().section(":", 0, 1);
    lMutex += ":" + clQuery.value(1).toString();

    // After a disconnect (MySql server issue OR Admin Server maintenance)
    // if mMutex is not empty and diff than new mutex => lost connection => CHECK WHY

    // Check if the database is accessible
    // mMutex is empty (the application just starts)
    // Status is not empty (update in progress)
    // => reject directly the connection

    // Check if the database is accessible
    // mMutex is not empty (the application is already started and was disconnected)
    // => disconnect the user
    // Status is not empty (update in progress)
    // => reject new user connection
    if (!mMutex.isEmpty() && (lMutex != mMutex))
    {
        GSLOG(SYSLOG_SEV_CRITICAL,
              QString("Lost connection opened on Mutex=%1").arg(mMutex).toLatin1().constData());

        // Clean some Yield-Man tables
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            // Lost the preview connection !!!
            // It is possible to lose the connection when
            // the plugin frezes the application
            // In this case, the user was disconnected
            // but the node must be automatically reconnected
            // and his mutex must be updated
            // This mutex is used only on ym_nodes and ym_actions
            // Need to update all Mutex column if associated with the old Mutex
            QStringList lTables;
            lTables << "ym_nodes" << "ym_actions";
            while (! lTables.isEmpty())
            {
                lQuery = "UPDATE " + lTables.takeFirst();
                lQuery += " SET mutex='" + lMutex + "'";
                lQuery += " WHERE mutex='" + mMutex + "'";
                clQuery.exec(lQuery);
            }
        }
    }

    mMutex = lMutex;

    // Check if have the LoadBalancing

    mIsLoadBalancingActive = (m_nServerBuild > 7);
    if (mIsLoadBalancingActive)
    {
        QString strValue;
        GetSettingsValue("DB_LOADBALANCING_OPTION", strValue);
        if (strValue == "DISABLED")
        {
            mIsLoadBalancingActive = false;
            GSLOG(SYSLOG_SEV_ALERT, "DB_LOADBALANCING_OPTION = DISABLED");
            GSLOG(SYSLOG_SEV_ALERT, "Load balancing was disabled by the user!");
        }
    }

    GSLOG(SYSLOG_SEV_NOTICE,
          QString("SqlUser '%1' connected on Mutex %2").
          arg(m_pDatabaseConnector->m_strUserName_Admin).
          arg(mMutex).toLatin1().constData());

    // Check if need an update here
    // UpdateAdminServerDb();

    // For Yield-Man
    // Do not accept minor version modification
    QString lCompatibleInfo;
    if (!IsServerCompatible(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring(),
                            lCompatibleInfo))
    {
        // Check the current version supported
        QString strError;
        strError = lCompatibleInfo;
        strError += "\n\n";
        strError += "Your version is " +
                GetServerVersionName(GS::LPPlugin::ProductInfo::getInstance()->
                                     isMonitoring()) + ".\n";
        strError += "The current version supported is " +
                GetCurrentVersionName(GS::LPPlugin::ProductInfo::getInstance()->
                                      isMonitoring());
        if(m_strServerStatus.isEmpty())
            SetLastError(eDB_NotUpToDate, strError);
        else
            SetLastError(eDB_UnderMaintenance, strError);
        return false;
    }

    return true;
}

bool AdminEngine::DisconnectToAdminServer()
{
    // Stop the ValidityCheck timer
    disconnect(&mValidityCheckTrigger, SIGNAL(timeout()), this, SLOT(OnValidityCheck()));
    mValidityCheckTrigger.stop();

    if(m_pDatabaseConnector)
    {
        // Stop requested
        if(m_nNodeId > 0)
        {
            QString        lQuery;
            QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

            QString Date = "SYSDATE";
            if(m_pDatabaseConnector->IsMySqlDB())
                Date = "now()";

            lQuery = "UPDATE ym_nodes SET ";
            lQuery+= "  status='STOP_REQUESTED'";
            lQuery+= ", last_update="+Date;
            lQuery+= ", mutex=null";
            lQuery+= "  WHERE node_id="+QString::number(m_nNodeId);
            clQuery.exec(lQuery);

            clQuery.exec("COMMIT"); // To unlock any tables
        }

        DisconnectCurrentUser();

        // Stop the scheduler if running
        if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerRunning())
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().RunTaskScheduler(false);

        DisconnectNode();

        m_pDatabaseConnector->Disconnect();
    }
    if (mDirAccessPlugin)
    {
        if(mDirAccessPlugin->GetConnector()
                && mDirAccessPlugin->GetConnector()->IsConnected())
            mDirAccessPlugin->GetConnector()->Disconnect();

        delete mDirAccessPlugin;
        mDirAccessPlugin = NULL;
    }
    mMutex = "";

    // No database access configured
    // Work offline
    return true;
}

void AdminEngine::OnValidityCheck()
{
    if(m_pDatabaseConnector==NULL)
        return;

    // Check if already running
    if(mValidityCheckInProgress)
        return;

    mValidityCheckInProgress = true;
    // Check Admin connection
    if(!ValidityCheck())
    {
        DisconnectCurrentUser();
        if(mDirAccessPlugin)
            mDirAccessPlugin->GetConnector()->Disconnect();
    }
    // Then check the DA plugin connection
    else if(mDirAccessPlugin
            && mDirAccessPlugin->GetConnector()
            && !mDirAccessPlugin->GetConnector()->ValidityCheck())
    {
        DisconnectCurrentUser();
        mDirAccessPlugin->GetConnector()->Disconnect();
    }

    // Refresh the GUI and the status
    emit sEnableGexAccess();

    mValidityCheckInProgress = false;

    // Schedule another validity check in 5s
    mValidityCheckTrigger.setSingleShot(true);
    mValidityCheckTrigger.setInterval(5000);

    if(!mValidityCheckTriggerConnected)
    {
        connect(&mValidityCheckTrigger, SIGNAL(timeout()), this, SLOT(OnValidityCheck()), Qt::UniqueConnection);
        mValidityCheckTriggerConnected = true;
    }

    mValidityCheckTrigger.start();
}

bool AdminEngine::ValidityCheck()
{
    // Check connection
    if (m_pDatabaseConnector->IsConnected())
        return true;

    // Try to connect and repair
    if (ConnectToAdminServerDb())
        return true;

    return false;
}

bool AdminEngine::WelcomeDialog(QString Options)
{
    bool Result=false;
    emit sWelcomeDialog(Options,Result);
    return Result;
}

void AdminEngine::SummaryDialog()
{
    emit sSummaryDialog();
}

void AdminEngine::ConnectionErrorDialog(QString &strRootCause, bool bCriticalError)
{
    emit sConnectionErrorDialog(strRootCause,bCriticalError);

    // Display an error message
    QString strError;
    QString strMessage;

    GetLastError(strError);
    strMessage = "Yield-Man Administration Server is not accessible.";
    strMessage+= "\n";
    strMessage+= (bCriticalError?"CRITICAL ERROR\n":"");
    if(!strError.isEmpty())
        strMessage+= strError + "\n";
    strMessage+= strRootCause+"\n";

    GSLOG(SYSLOG_SEV_EMERGENCY,strMessage.toLatin1().constData());
}

QString AdminEngine::GetServerVersionName(bool bWithBuildNb)
{
    QString strServerName;

    strServerName = m_strServerVersionName;
    strServerName += " V"+QString::number(m_nServerMajorVersion)+"."+QString::number(m_nServerMinorVersion);

    if(bWithBuildNb)
        strServerName += " B"+QString::number(m_nServerBuild);

    return strServerName;
}

QString AdminEngine::GetCurrentVersionName(bool bWithBuildNb)
{
    QString strValue;
    strValue = QString(YIELDMANDB_VERSION_NAME);
    strValue+= " V"+QString::number(YIELDMANDB_VERSION_MAJOR);
    strValue+= "."+QString::number(YIELDMANDB_VERSION_MINOR);
    if(bWithBuildNb)
        strValue += " B"+QString::number(YIELDMANDB_BUILD_NB);

    return strValue;
}

QMap<QString,QString> AdminEngine::GetServerVersion()
{
    QMap<QString,QString> Values;
    QString lValue;

    // General information
    GetSettingsValue("DB_VERSION_NB", lValue);
    if (lValue == "10")
        lValue = "1.0";

    Values["DB_VERSION_NB"] = lValue;
    m_nServerMajorVersion = lValue.section(".", 0, 0).toInt();
    m_nServerMinorVersion = lValue.section(".", 1).toInt();

    GetSettingsValue("DB_BUILD_NB", lValue);
    m_nServerBuild = lValue.toInt();
    Values["DB_BUILD_NB"] = lValue;

    GetSettingsValue("DB_VERSION_NAME", lValue);
    m_strServerVersionName = lValue;
    Values["DB_VERSION_NAME"] =  lValue;

    // New Server Status
    lValue = "";
    if(m_nServerBuild > 18)
        GetSettingsValue("DB_STATUS", lValue);
    m_strServerStatus = lValue;
    Values["DB_STATUS"] =  lValue;

    // Settings information
    Values["DB_CONNECTION_DATABASE"] = m_pDatabaseConnector->m_strDatabaseName;
    Values["DB_CONNECTION_DRIVER"] = m_pDatabaseConnector->m_strDriver;
    Values["DB_CONNECTION_IP"] = m_pDatabaseConnector->m_strHost_IP;
    Values["DB_CONNECTION_HOST"] = m_pDatabaseConnector->m_strHost_Name;
    Values["DB_CONNECTION_USER"] = m_pDatabaseConnector->m_strUserName;
    Values["DB_CONNECTION_PORT"] = QString::number(m_pDatabaseConnector->m_uiPort);

    return Values;
}

QMap<QString,QString> AdminEngine::GetCurrentVersion()
{
    QMap<QString,QString> Values;
    Values["DB_VERSION_NAME"] = QString(YIELDMANDB_VERSION_NAME);
    Values["DB_VERSION_MAJOR"] = QString::number(YIELDMANDB_VERSION_MAJOR);
    Values["DB_VERSION_MINOR"] = QString::number(YIELDMANDB_VERSION_MINOR);
    Values["DB_BUILD_NB"] = QString::number(YIELDMANDB_BUILD_NB);

    return Values;
}

QDateTime AdminEngine::GetServerDateTime()
{
    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return QDateTime::currentDateTime();
    }
    return m_pDatabaseConnector->ServerDateTime();
}


bool AdminEngine::IsServerUpToDate()
{
    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

    if(!ConnectToAdminServer())
        return false;

    GetServerVersion();

    /////////////////////////
    // CHECK IF ALL UPDATES ARE DONE
    if(!m_strServerStatus.isEmpty())    // need some pending update
        return false;

    if(m_nServerMajorVersion != YIELDMANDB_VERSION_MAJOR)
        return false;

    if(m_nServerMinorVersion != YIELDMANDB_VERSION_MINOR)
        return false;

    if(m_nServerBuild != YIELDMANDB_BUILD_NB)
        return false;

    return true;
}

bool AdminEngine::IsServerCompatible(bool bOnlyCheckMajorVersion, QString &lCompatibleInfo)
{
    if(m_pDatabaseConnector == NULL)
    {
        // No database access configured
        // Work offline
        return true;
    }

    if(!ConnectToAdminServer())
    {
        lCompatibleInfo = "Your Administration Server Connection is not available.";
        lCompatibleInfo += "\nPlease check your network.";
        return false;
    }

    GetServerVersion();

    /////////////////////////
    // CHECK IF ALL UPDATES ARE DONE
    if(!m_strServerStatus.isEmpty())    // need some pending update
    {
        lCompatibleInfo = "A maintenance is in progress on your Administration Server :";
        lCompatibleInfo += "\n"+m_strServerStatus;
        lCompatibleInfo += "\nPlease wait and try later.";
        return false;
    }

    if(m_nServerMajorVersion != YIELDMANDB_VERSION_MAJOR)
    {
        if(m_nServerMajorVersion > YIELDMANDB_VERSION_MAJOR)
        {
            lCompatibleInfo = "Your application is not compatible with the current Administration Server.";
            lCompatibleInfo += "\nPlease upgrade your application.";
        }
        else
        {
            lCompatibleInfo = "Your Administration Server is not up-to-date.";
            lCompatibleInfo += "\nPlease updade your Server.";
        }
        return false;
    }

    if(bOnlyCheckMajorVersion)
        return true;

    if(m_nServerMinorVersion != YIELDMANDB_VERSION_MINOR)
    {
        lCompatibleInfo = "Your Administration Server is not up-to-date";
        lCompatibleInfo += "\nPlease updade your Server.";
        return false;
    }

    return true;
}

bool AdminEngine::AdminServerBackup(QString &BackupFile)
{
    QString        strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(m_pDatabaseConnector->
                                             m_strConnectionName));

    QProcess        clProc;
    QStringList     Arguments;
    QFileInfo       Prog;
    QString         ErrorMsg;

    if(!m_pDatabaseConnector->IsMySqlDB())
        return true;

    // Backup File
    QDir Dir;
    QStringList Backups;
    Dir.setPath(GS::Gex::Engine::GetInstance().
                Get("GalaxySemiFolder").toString() + QDir::separator());
    Backups = Dir.entryList(QStringList() <<
                            QString(m_pDatabaseConnector->m_strSchemaName+"_"
                                    +QDate::currentDate().toString(Qt::ISODate)
                                    +"*.sql"),QDir::Files,QDir::Time);
    BackupFile = QFileInfo(Dir,m_pDatabaseConnector->m_strSchemaName + "_" +
                           QDate::currentDate().toString(Qt::ISODate)).
            absoluteFilePath();
    if(!Backups.isEmpty())
        BackupFile += "-"+QString::number(Backups.count());
    BackupFile += ".sql";

    QFile::remove(BackupFile);

    // SHOW EVENTS access ?
    strQuery = "SHOW VARIABLES LIKE 'BaseDir'";
    if(!clQuery.exec(strQuery))
    {
        SetLastError(clQuery);
        return false;
    }
    clQuery.first();

    Dir.setPath(clQuery.value(1).toString()+"bin/");
    QStringList env = QProcess::systemEnvironment();
    env << QString("PATH=%PATH%;"+Dir.path());
    clProc.setEnvironment(env);
    clProc.setWorkingDirectory(Dir.path());


    Prog.setFile(Dir,"mysqldump.exe");

    // Check if Dir exists
    if(!Prog.exists())
    {
        strQuery = "No such file or directory";
        SetLastError(eDB_Query,
                     Prog.absoluteFilePath().left(1024),
                     strQuery);
        return false;
    }

    Arguments << QString("--user=" +
                         m_pDatabaseConnector->m_strUserName_Admin);
    Arguments << QString("--password=" +
                         m_pDatabaseConnector->m_strPassword_Admin);
    Arguments << QString("--host=" +
                         m_pDatabaseConnector->m_strHost_Name);
    Arguments << QString("--port=" +
                         QString::number(m_pDatabaseConnector->m_uiPort));
    Arguments << QString("--opt");
    Arguments << QString("--comments");
    Arguments << QString(m_pDatabaseConnector->m_strSchemaName);

    clProc.setStandardOutputFile(BackupFile);
    clProc.setStandardErrorFile(m_strUpdateDbLogFile);
    clProc.start(Prog.absoluteFilePath(), Arguments);
    // Make sure process started
    int nTrials=0;
    while(!clProc.waitForStarted(200) && (nTrials < 10))
    {
        if(clProc.state() == QProcess::NotRunning)
            break;
        nTrials++;
    }
    if(nTrials == 10)
    {
        ErrorMsg =  "Timeout starting MySql Dump (mysqldump.exe).";
        SetLastError(eDB_Query,
                     QString("mysqldump "+Arguments.join(" ")).left(1024),
                     ErrorMsg);
        return false;
    }

    // Synchronization with the end of the process
    while(!clProc.waitForFinished(200))
    {
        if(clProc.state() != QProcess::Running)
            break;
    }

    // Check if file exists
    if(!QFileInfo(BackupFile).exists()
            || (QFileInfo(BackupFile).size() < 100)) // Empty file
    {
        ErrorMsg =  "MySql Dump (mysqldump.exe) - Backup error: ";
        ErrorMsg+= clProc.errorString();
        SetLastError(eDB_Query,
                     QString("mysqldump "+Arguments.join(" ")).left(1024),
                     ErrorMsg);
        return false;
    }

    // create all user/node for connection
    QFile File(BackupFile);
    if(!File.open(QIODevice::ReadWrite|QIODevice::Append))
    {
        SetLastError(eDB_Query,
                     QString("Open file: "+BackupFile).left(1024),
                     File.errorString());
        return false;
    }

    QString User;
    QString Pwd;

    User = m_pDatabaseConnector->m_strUserName_Admin;
    Pwd = m_pDatabaseConnector->m_strPassword_Admin;

    strQuery = "\n\n\n--\n-- CREATE root connection: "+User+"\n--\n\n";
    strQuery += "GRANT ALL PRIVILEGES ON "
            + m_pDatabaseConnector->m_strSchemaName + ".* TO '"
            + User + "'@'localhost' IDENTIFIED BY '" + Pwd + "';\n";
    strQuery += "GRANT ALL PRIVILEGES ON "
            + m_pDatabaseConnector->m_strSchemaName + ".* TO '"
            + User + "'@'%' IDENTIFIED BY '" + Pwd + "';\n";
    strQuery += "GRANT GRANT OPTION ON "
            + m_pDatabaseConnector->m_strSchemaName + ".* TO '"
            + User + "'@'localhost';\n";
    strQuery += "GRANT GRANT OPTION ON "
            + m_pDatabaseConnector->m_strSchemaName + ".* TO '"
            + User + "'@'%';\n";
    strQuery += "GRANT CREATE USER ON *.* TO '"
            + User + "'@'localhost';\n";
    strQuery += "GRANT CREATE USER ON *.* TO '"
            + User + "'@'%';\n\n\n\n";
    File.write(strQuery.toLatin1().data());

    File.close();

    return true;
}

int AdminEngine::StringToUserType(QString strType)
{
    int nType = YIELDMANDB_USERTYPE_MASTER_ADMIN;

    if(strType.isEmpty())
        return nType;

    for (nType = YIELDMANDB_USERTYPE_USER;
         nType <= YIELDMANDB_USERTYPE_MASTER_ADMIN; ++nType)
    {
        if(strType == UserTypeToString(nType))
            break;
    }

    return nType;
}

QString AdminEngine::UserTypeToString(int nType)
{
    QString  strType;

    switch(nType)
    {
    case YIELDMANDB_USERTYPE_MASTER_ADMIN:
        strType = "Yield-Man Server Administrator";
        break;
    default:
        strType = "Examinator-PRO User";
    }

    return strType;
}


/******************************************************************************!
 * \fn NormalizeScriptStringToSql
 ******************************************************************************/
QString AdminEngine::NormalizeScriptStringToSql(QString strValue)
{
    return m_pDatabaseConnector->
            TranslateStringToSqlLob(strValue.
                                    replace("\'", "[BackQuote]").
                                    replace("'", "[Quote]").
                                    replace("\\", "[Back]"));
}

QString AdminEngine::NormalizeSqlToScriptString(QString strValue)
{
    return strValue.
            replace("[Back]","\\").
            replace("[Quote]","'").
            replace("[BackQuote]","\'");
}

/******************************************************************************!
 * \fn NormalizeStringToSql
 ******************************************************************************/
QString AdminEngine::NormalizeStringToSql(QString strValue, bool bAddQuote)
{
    return m_pDatabaseConnector->
            TranslateStringToSqlVarChar(strValue, bAddQuote);
}

QString AdminEngine::NormalizeStringToSqlLob(QString strValue)
{
    return m_pDatabaseConnector->TranslateStringToSqlLob(strValue);
}

/******************************************************************************!
 * \fn NormalizeDateToSql
 ******************************************************************************/
QString AdminEngine::NormalizeDateToSql(QDateTime Date)
{
    return m_pDatabaseConnector->TranslateDateToSqlDateTime(Date);
}

bool AdminEngine::GrantUserPrivileges(QString strUserName, QString strUserPwd)
{

    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(m_pDatabaseConnector->m_strConnectionName));

    if(m_pDatabaseConnector->IsOracleDB())
    {
        // Drop user if exist
        strQuery = "DROP USER "+strUserName+" CASCADE";
        clQuery.exec(strQuery);


        strQuery = "CREATE USER "+strUserName+" PROFILE DEFAULT";
        strQuery+= " IDENTIFIED BY "+strUserPwd;
        strQuery+= " DEFAULT TABLESPACE "+m_pDatabaseConnector->m_strSchemaName;
        strQuery+= " TEMPORARY TABLESPACE "+m_pDatabaseConnector->m_strSchemaName+"_temp ";
        strQuery+= " ACCOUNT UNLOCK";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT CONNECT TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT RESOURCE TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT OEM_MONITOR TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT ALTER TABLESPACE TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT CREATE TABLESPACE TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT DROP TABLESPACE TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT MANAGE TABLESPACE TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT IMP_FULL_DATABASE TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT EXP_FULL_DATABASE TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT SCHEDULER_ADMIN TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "ALTER USER "+strUserName+" DEFAULT ROLE ALL";
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

        strQuery = "GRANT CREATE JOB TO "+strUserName;
        if(!clQuery.exec(strQuery))
        {
            SetLastError(clQuery);
            return false;
        }

    }
    else
    {
        // Drop user if exist
        // on localhost
        strQuery = "DROP USER '"+strUserName+"'@'localhost'";
        clQuery.exec(strQuery);
        // for all other hosts
        strQuery = "DROP USER '"+strUserName+"'@'%'";
        clQuery.exec(strQuery);

        // on localhost
        strQuery = "GRANT SELECT, SHOW VIEW, CREATE VIEW, CREATE TEMPORARY TABLES ON "
                +m_pDatabaseConnector->m_strSchemaName+".* TO '"+strUserName+"'@'localhost'";
        strQuery += " IDENTIFIED BY '"+strUserPwd+"'";
        if(!clQuery.exec(strQuery))
        {
            // if user already exist
            // try without pwd
            // on localhost
            strQuery = "GRANT SELECT, SHOW VIEW, CREATE VIEW, CREATE TEMPORARY TABLES ON "
                    +m_pDatabaseConnector->m_strSchemaName+".* TO '"+strUserName+"'@'localhost'";
            if(!clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
            // for all other hosts
            strQuery = "GRANT SELECT, SHOW VIEW, CREATE VIEW, CREATE TEMPORARY TABLES ON "
                    +m_pDatabaseConnector->m_strSchemaName+".* TO '"+strUserName+"'@'%'";
            if(!clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }
        }
        else
        {
            // for all other hosts
            strQuery = "GRANT SELECT, SHOW VIEW, CREATE VIEW, CREATE TEMPORARY TABLES ON "
                    +m_pDatabaseConnector->m_strSchemaName+".* TO '"+strUserName+"'@'%'";
            strQuery += " IDENTIFIED BY '"+strUserPwd+"'";
            if(!clQuery.exec(strQuery))
            {
                SetLastError(clQuery);
                return false;
            }

        }
    }

    clQuery.exec("COMMIT");
    return true;
}

QMap<QString,QString> AdminEngine::SplitCommand(QString Command)
{
    QMap<QString,QString> Args;
    foreach(const QString& v , Command.split("|",QString::SkipEmptyParts))
        Args[v.section("=",0,0)]=v.section("=",1);

    return Args;
}

QString AdminEngine::JoinCommand(QMap<QString,QString> Command)
{
    QStringList listSummary;

    foreach(const QString& Key, Command.keys())
    {
        // Keep the actionId used from the ym_actions
        // to check if the duplicate insertion
        // comes from the same actionId
        // Skip ActionId
        //if(Key == "ActionId")
        //    continue;
        listSummary << Key+"="+Command[Key];
    }

    return listSummary.join("|");
}

QVariant AdminEngine::GetAttribute(const QString &key)
{
    return mAttributes.value(key.toUpper());
}

void AdminEngine::SetAttribute(const QString &key, QVariant value)
{
    mAttributes.remove(key.toUpper());
    if(!key.isEmpty())
        mAttributes.insert(key.toUpper(), value);
}
// Reset all attributes
void AdminEngine::ResetAllAttributes()
{
    mAttributes.clear();
}

}
}
