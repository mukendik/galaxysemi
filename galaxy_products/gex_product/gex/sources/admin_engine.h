///////////////////////////////////////////////////////////
// Database Admin class
///////////////////////////////////////////////////////////

#ifndef GEX_ADMIN_ENGINE_H
#define GEX_ADMIN_ENGINE_H

#include <QDateTime>
#include <QFile>
#include <QStack>
#include <QDomElement>
#include <QString>
#include <QDir>
#include <QTime>
#include <QMap>
#include <QVariant>
#include <QSqlQuery>
#include <QTimer>

#include "gexdb_plugin_base.h"

#define YIELDMANDB_ADMIN_MIN_BEFORE_DISCONNECT        1        // Nb Minutes before disconnect the admin login (-1 if no disconnection)

#define YIELDMANDB_LOADBALANCING_RESERVED_ACTION_ROWS 10

#define YIELDMANDB_USERTYPE_USER            1
#define YIELDMANDB_USERTYPE_MASTER_ADMIN    2

// Updating ym_admin_db
#define YIELDMANDB_BUILD_NB 28
#define YIELDMANDB_VERSION_MAJOR 6
#define YIELDMANDB_VERSION_MINOR 1
#define YIELDMANDB_VERSION_NAME     "Yield-Man Administration Server"

// Galaxy modules includes
#include <gstdl_errormgr.h>
class GexDbPlugin_Connector;
class GexDatabaseEntry;
class CGexMoTaskItem;

namespace GS
{
    namespace DAPlugin
    {
        class DirAccessBase;
    }
}

class AdminUserProfile : public QObject
{
    Q_OBJECT

public:
    // User profile
    int          m_nProfileId;
    int          m_nUserId;
    QString      m_strName;
    QString      m_strDescription;
    QString      m_strOsLogin;
    int          m_nPermissions;
    QString      m_strScriptName;
    QString      m_strScriptContent;

    QDateTime    m_clCreationDate;
    QDateTime    m_clUpdateDate;
};

class AdminUser : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString Email READ GetEmail)
    Q_PROPERTY(QString Password READ GetPassword)

public slots:
    QString GetEmail() { return m_strEmail; }
    QString GetPassword() { return m_strPwd; }
    // Get an attribute : example :  "Title" > "Coucou"
    QVariant  GetAttribute(const QString &key) { return mAttributes.value(key); }
    //! \brief Reset ?
    void      ResetAllAttributes()             { mAttributes.clear(); }

public:
    // User identification
    int        m_nUserId;
    int        m_nGroupId;        // Primary group
    QString    m_strLogin;
    QString    m_strPwd;
    QString    m_strName;
    QString    m_strEmail;
    int        m_nType;
    QString    m_strOsLogin;
    int        m_nProfileId;

    QDateTime  m_clCreationDate;
    QDateTime  m_clAccessDate;
    QDateTime  m_clUpdateDate;

    // Functions
    // mAttributes.insert( "ReportIf", "JavaScript" );
    //! \brief Set ?
    Q_INVOKABLE void SetAttribute(const QString &key, QVariant value);

    const QMap<QString, QVariant> GetAttributes() { return mAttributes; }

    bool    TraceUpdate(QString Command,QString Status,QString Summary);

private:
    // Map containing the parameters of this task
    // exple : "CheckType"="FixedYieldTreshold", "Title"="Coucou",...
    QMap<QString, QVariant> mAttributes;
};

class AdminUserGroup
{
public:
    // Group definition
    int            m_nGroupId;
    int            m_nUserId;        // Manager
    QString        m_strName;
    QString        m_strDescription;
    int            m_nProfileId;
    QStringList    m_lstAllowedProducts;
    QStringList    m_lstAllowedLots;

    // Users associated with this group
    QList<int>    m_lstUserIds;

};

namespace GS
{
namespace Gex
{
class AdminEngine: public QObject
{
    friend class EnginePrivate;

    Q_OBJECT
    AdminEngine(QObject* parent);
    //! \brief Destructor
    ~AdminEngine();

signals:
    void sShowUserList();
    void sPopupMessage(QString strMessage);

    void sWelcomeDialog(QString Options, bool &Result);
    bool sSummaryDialog();
    void sConnectionErrorDialog(QString &strRootCause, bool bCriticalError);
    void sDisplayStatusMessage(QString strText);
    void sEnableGexAccess();

public slots:

    bool    LoadServerSettings();
    bool    ConnectNode();
    bool    DisconnectNode();
    bool    UpdateNodeStatus();

    // TO MANAGE OTHER NODES
    bool    OtherNodesRunning();
    bool    StopAllOtherNodes();

    QString GetOsLogin()      {return m_strOsLogin;}

    ////////////////////////////////////
    // Get server info
    // Get global settings for all users
    bool    GetSettingsValue(const QString &OptionName, QString &strOptionValue);
    // Secondary function mainly for Script engine, use first one in C++
    // return an empty string on error
    QString GetSettingsValue(const QString &OptionName);
    // Get settings for current node_id
    bool    GetNodeSettingsValue(const QString &strOptionName, QString &strOptionValue);
    //! \brief Get Node Setting. Return a null Variant if not found.
    QVariant GetNodeSetting(const QString &OptionName);
    bool    GetNodeSettingsValues(QMap<QString,QString>& mapSettings, bool ForAll=false);
    // Get settings for current user
    bool    GetUserSettingsValue(const QString &strOptionName, QString &strOptionValue);
    bool    GetUserSettingsValues(QMap<QString,QString>& mapSettings, bool ForAll=false);
    // Get the new yieldmandb database folder
    //    - from OptionCenter if exist
    //    - create it if not (HOME/GalaxySemi/Databases/yieldman
    QString GetDatabaseFolder();


    // Check if YieldManDb is up to date
    bool        IsServerUpToDate();
    bool        IsServerCompatible(bool bOnlyCheckMajorVersion, QString &lCompatibleInfo);
    QString     GetServerVersionName(bool bWithBuildNb);    // From ym_admin_db database
    QString     GetCurrentVersionName(bool bWithBuildNb);   // From current code
    QDateTime   GetServerDateTime();

    // return a map for each value
    QMap<QString,QString>     GetServerVersion();           // From ym_admin_db database
    QMap<QString,QString>     GetCurrentVersion();          // From current code

    // Update YieldManDb
    bool    AdminServerUpdate();

    ////////////////////////////////////
    //
    // Check if a specific user is connected through another GEX
    bool    IsUserConnected(AdminUser *pUser);
    // Check if the Current user is connected as user/admin/superadmin
    bool    IsUserConnected(bool asAdmin=false);
    // true if user is a master (with or without DA GALAXY)
    // if NULL, check the current user connected
    bool    IsUserAdmin(AdminUser *pUser);
    bool    IsUserAdmin(bool bWithReadOnly=false);
    bool    HasUserGroupAdminPrivileges(bool bWithReadOnly=false);

    // Disconnect the user for Gex execution
    bool    DisconnectCurrentUser();

    // Synchronize user profiles between YieldmanDB and the local folder.
    void    SynchronizeProfiles(const QString & strDefaultUserName);

    // Enable some Gex button access
    bool    IsNodeConnected(int nNodeId);

    // ?
    bool    LoadUsersList();
    // Load the dir access plugin
    bool    LoadDirectoryAccessPlugin();
    // Check the server (connection, plugin and logged-in) and update the GUI every 5s
    void    OnValidityCheck();

    // Load balancing
    // Functions
    bool      IsLoadBalancingMode()   {return mIsLoadBalancingActive;}
    bool      IsActivated()           {return m_pDatabaseConnector!=NULL;}

    QString       GetDatabaseGalaxyBlackHoleName() {return QString("None (Black Hole mode)");}

    // Current User identification
    // Connect the user for Gex execution
    // Check if :  the user is registred
    //        the pwd is correct
    //        the user not already connected
    // Then disconnect the current user connected if any and connect the new user
    // return what ?
    bool ConnectUser(int nUserId, QString strPwd, bool bAsAdmin);

    /*!
     * \fn Log
     * \brief Log to admin log. Status = "WARNING", "INFO", or "UNEXPECTED"
     */
    void Log(QString Status, QString Summary, QString Comment);

public:
    GDECLARE_ERROR_MAP(AdminEngine)
    {
        // General
        eMarkerNotFound,        // XML marker not found in settings file
                // Database errors
                eDB_CreateConnector,    // Error creating database connector
                eDB_Connect,            // Error connecting to database
                eDB_NotUpToDate,        // Database is not UpToDate
                eDB_UnderMaintenance,    // Database is not UpToDate
                eDB_NoConnection,        // Lost Database connection
                eDB_NoFields,            // No fields for specified table
                eDB_InvalidTable,        // Specified table does not exist in DB
                eDB_InvalidField,        // Specified field does not exist in DB
                eDB_InvalidEntry,        // Specified entry does not exist in DB
                eDB_Query                // Error executing SQL query
    }
    GDECLARE_END_ERROR_MAP(AdminEngine)

    // Get last error
    // Update details about last error
    void    SetLastError(int nErrorType,QString strArg1="",QString strArg2="",QString strArg3="",QString strArg4="");
    void    SetLastError(QSqlQuery &clQuery,bool RetryOnTimeOut=false);

    Q_INVOKABLE void    GetLastError(QString & strError);                        // Returns details about last error

    //! \brief Set global settings for all users
    Q_INVOKABLE bool SetSettingsValue(const QString &strOptionName, QString &strOptionValue);
    //! \brief Set settings for current node_id
    Q_INVOKABLE bool SetNodeSettingsValue(const QString &strOptionName, const QString &strOptionValue, bool ForAll=false);
    //! \brief Set settings for current user
    Q_INVOKABLE bool SetUserSettingsValue(const QString &strOptionName, QString &strOptionValue, bool ForAll=false);

    // System identification
    bool    m_bFirstActivation;
    Q_INVOKABLE int     GetNodeId()       {return m_nNodeId;}
    Q_INVOKABLE bool    IsMutexActive(QString lMutex);

    // Return directory access plugin
    GS::DAPlugin::DirAccessBase* GetDirAccessPlugin() {return mDirAccessPlugin;}

    // Connection to YieldManDatabase
    GexDbPlugin_Connector    *m_pDatabaseConnector;

    //! \brief Get a valid plugin connector for Database admin connection
    GexDbPlugin_Connector    *GetAdminServerConnector();


    void                        InitAdminServerConnector();


    /*!
     * \fn DeleteAdminServerConnector
     */
    void DeleteAdminServerConnector();

    // YieldManDbCreation : Create YieldManDb database
    bool    CreationAdminServerDb();
    bool    UpdateAdminServerDb();
    // Start and configurate the Admin Server
    // Connect the Admin Server (DB and DA plugin)
    bool    ConnectToAdminServer();
    // Open the DB and DA connections
    bool    ConnectToAdminServerDb();
    bool    ConnectToDaServer();
    // Stop and clean the Admin Server (Close DB connection and close DA plugin)
    bool    DisconnectToAdminServer();

    Q_INVOKABLE bool    WelcomeDialog(QString Options);
    //! \brief Display this dialog only after the first ym_admin_db install (?)
    Q_INVOKABLE void    SummaryDialog();
    Q_INVOKABLE void    ConnectionErrorDialog(QString &strRootCause, bool bCriticalError);

    // todo : from name/email
    //bool    ConnectUser(QString email, QString strPwd, bool bAsAdmin);

    Q_INVOKABLE void    DeleteProfiles(const QString & strDefaultUserName);

    AdminUser           *m_pCurrentUser;
    AdminUserGroup      *m_pCurrentGroup;
    AdminUserProfile    *m_pCurrentProfile;

    qlonglong       m_LastDbUsersUpdateChecksum;  // Last CHECKSUM(Date&Time) users list was reloaded from ym_users...
    qlonglong       m_LastDbUsersIdChecksum;      // Last CHECKSUM(user_id) from ym_users...

    // Users and Groups information

    bool    SaveGroup(AdminUserGroup* pGroup);
    bool    SaveUser(AdminUser* pUser = NULL);
    bool    SaveProfile(AdminUserProfile* pProfile);
    bool    DeleteProfile(AdminUserProfile* pProfile);

    QMap<int,AdminUser*>          m_mapUsers;
    QMap<int,AdminUserGroup*>     m_mapGroups;
    QMap<int,AdminUserProfile*>   m_mapProfiles;

    bool    IsAllowedToRead(AdminUser *pUser);           // Read user info
    bool    IsAllowedToModify(AdminUser *pUser);         // Modify user info
    bool    IsAllowedToRead(AdminUserGroup *pGroup);     // Read group info
    bool    IsAllowedToModify(AdminUserGroup *pGroup);   // Modify group info

    // Tasks permission
    bool    IsAllowedToLock(CGexMoTaskItem* pTask);
    bool    IsAllowedToRead(CGexMoTaskItem* pTask);
    bool    IsAllowedToModify(CGexMoTaskItem* pTask);
    bool    IsAllowedToExecute(CGexMoTaskItem* pTask);

    // Databases permission
    bool    IsInRestrictedMode(GexDatabaseEntry *pDatabaseEntry);
    bool    IsAllowedToRead(GexDatabaseEntry *pDatabaseEntry);
    bool    IsAllowedToModify(GexDatabaseEntry *pDatabaseEntry);

    // Lock Task in YieldManDb
    bool    Lock(CGexMoTaskItem *ptTask);
    bool    Unlock(CGexMoTaskItem *ptTask);
    bool    Lock(AdminUser *ptUser);
    bool    Unlock(AdminUser *ptUser);
    bool    Lock(GexDatabaseEntry *pDatabaseEntry);
    bool    Unlock(GexDatabaseEntry *pDatabaseEntry);

    // Database entry ?
    bool          SaveDatabase(GexDatabaseEntry* pDatabaseEntry);
    QDomElement   GetDatabaseEntryDomElt(int nDatabaseId);
    // CODE ME
    bool          SaveDatabaseEntry(QDomElement node);

    int     StringToUserType(QString strType);
    QString UserTypeToString(int nType);
    QString NormalizeScriptStringToSql(QString strValue);
    QString NormalizeSqlToScriptString(QString strValue);
    QString NormalizeStringToSql(QString strValue, bool bAddQuote=true);
    QString NormalizeStringToSqlLob(QString strValue);
    QString NormalizeDateToSql(QDateTime Date);

    // Events
    bool      AddNewEvent(QString Category,QString Type,
                          QString Status, QString Summary, QString Comment);
    bool      AddNewEvent(QString Category,QString Type,
                          int Size, int TaskId, int DatabaseId, QString Command,
                          QString Status, QString Summary, QString Comment);

    // Actions
    bool      AddNewAction(QString Category, QString ActionType, int TaskId, int DatabaseId, QString Command, QString& Error);
    bool      CleanActions(QString Options = QString());
    bool      CancelActions();
    bool      GetNextAction(QString Category, QStringList ActionType, int &ActionId);
    bool      GetActionInfo(int ActionId, QString &ActionType, int &TaskId, int &DatabaseId, QString &Command);
    bool      CloseAction(int ActionId, int Size,QString Status, QString Summary, QString Comment);
    bool      UpdateAction(int ActionId, QString AddCommand);

    // Master role
    bool      GetMasterRole();
    bool      ReleaseMasterRole();

    QMap<QString,QString>   SplitCommand(QString Command);
    QString                 JoinCommand(QMap<QString,QString> Command);

    QString m_strAllCpu;
    QString m_strCpu;
    QString m_strHostName;
    QString m_strHostId;
    QString m_strOs;
    QString m_strOsLogin;
    int     m_nNodeId;

    int     m_nServerMajorVersion;
    int     m_nServerMinorVersion;
    int     m_nServerBuild;
    QString m_strServerVersionName;
    QString m_strServerStatus;

    //! \brief List of keys for nodes
    static const QStringList sNodeSettingsString;
    //! \brief List of keys for ym_admin_db settings
    static const QStringList sSettingsString;
    //! \brief List of keys for users
    static const QStringList sUserSettingsString;

    QStringList GetNodeSettingsName();
    QString     GetNodeSettingType(QString optionName);
    bool        IsNodeSettingName(QString optionName);
    bool        IsValidNodeSettingValue(QString optionName, QString optionValue);

    QStringList GetSettingsName();
    QString     GetSettingType(QString optionName);
    bool        IsSettingName(QString optionName);
    bool        IsValidSettingValue(QString optionName, QString optionValue);

    QStringList GetUserSettingsName();
    QString     GetUserSettingType(QString optionName);
    QString     GetUserSettingDescription(QString optionName);
    QString     GetUserSettingDefaultValue(QString optionName);
    bool        IsUserSettingName(QString optionName);
    bool        IsValidUserSettingValue(QString optionName, QString optionValue);

    // Get an attribute : example :  "Title" > "Coucou"
    QVariant  GetAttribute(const QString &key);
    void SetAttribute(const QString &key, QVariant value);
    // Reset all attributes
    void      ResetAllAttributes();

private:

    QStringList GenericGetSettingsName(const QStringList lSettingsList);
    QString     GenericGetSettingAttribute(const QStringList lSettingsList,QString attributeName, QString optionName);
    bool        GenericIsSettingName(const QStringList lSettingsList,QString optionName);
    bool        GenericIsValidSettingValue(const QStringList lSettingsList,QString optionName, QString optionValue);

    // Mutex Convention
    // old mutex = CONNECTION_ID():HOST
    // - CONNECTION_ID is the # allocated by the Sql Server for this connection
    // - HOST is the hostName or hostIp of the user who creates this connection
    //      * localhost
    //      * 192.168.1.27
    // => with this mutex, this is not possible to have the Host of the Server
    // NEW CONVENTION
    // mutex = CONNECTION_ID():HOST:HOST_CONNECTOR
    // - CONNECTION_ID is the # allocated by the Sql Server for this connection
    // - HOST is the hostName or hostIp of the user who creates this connection
    //      * localhost
    //      * 192.168.1.27
    // - HOST_CONNECTOR is the hostName of the Sql Server
    //      * gx-head
    //      * Mac.local
    // when open a local connection on 'host1' = XX:localhost:host1
    // when open a remote connection on 'host1' from 'host2' = XX:host2_IP:host1
    // YM on comp1, user = XX:localhost:comp1
    // GEX on laptop(192.168.1.55), user = XX:192.168.1.55:head
    QString mMutex;

    /// \brief check if evrything is OK and try repair
    bool    mValidityCheckInProgress;
    bool    ValidityCheck();
    QTimer  mValidityCheckTrigger;  ///< Holds validity check timer
    bool    mValidityCheckTriggerConnected;

    GS::DAPlugin::DirAccessBase* mDirAccessPlugin; ///< hold pointer to directory access plugin

    // LOGLEVEL variables defined into ym_admin_db database
    QMap<QString,QString> m_mapLogLevel;

    // Admin Server settings
    bool    LoadServerOptions();

    bool          InsertIntoUpdateLog(QString Log);
    QString       m_strUpdateDbLogFile;
    QStringList   m_UpdateDbLogContents;
    QFile         m_hUpdateDbLogFile;

    bool          AdminServerUpdate_CloseAllSqlConnection();
    bool          AdminServerUpdate_UpdateIndexes(QStringList lstIndexesToCheck=QStringList());
    bool          AdminServerUpdate_UpdateEvents();
    bool          AdminServerUpdate_loop();
    bool          AdminServerUpdate_B1_to_B2();
    bool          AdminServerUpdate_B2_to_B3();
    // Rename existing DB users corresponding to YM users : 'user<user_id>' to 'ym_admin_user_<user_id>'
    // Rename existing DB users corresponding to YM nodes : 'node<user_id>' to 'ym_admin_node_<node_id>'
    bool          AdminServerUpdate_B3_to_B4();
    bool          AdminServerUpdate_B4_to_B5();
    bool          AdminServerUpdate_B5_to_B6();
    bool          AdminServerUpdate_B6_to_B7();
    bool          AdminServerUpdate_B7_to_B8();
    bool          AdminServerUpdate_B8_to_B9();
    bool          AdminServerUpdate_B9_to_B10();
    bool          AdminServerUpdate_B10_to_B11();
    bool          AdminServerUpdate_B11_to_B12();
    bool          AdminServerUpdate_B12_to_B13();
    bool          AdminServerUpdate_B13_to_B14();
    bool          AdminServerUpdate_FromSqlScript(UINT fromBuild, UINT toBuild);

//#define DA_GALAXY_REGENERATE
#ifdef DA_GALAXY_REGENERATE
    bool          AdminServerUpdate_dagalaxy();
#endif

    // Backup
    bool    AdminServerBackup(QString &BakupFile);

    // Load Balancing
    bool          mIsLoadBalancingActive;

    bool      AddNewEvent(QString Category,QString Type,QDateTime Date,
                          QString Status, QString Summary, QString Comment);
    bool      AddNewEvent(QString Category,QString Type,QDateTime Date,
                          int Size, int TaskId, int DatabaseId, QString Command,
                          QString Status, QString Summary, QString Comment);

    // Stack of event for Category "EXECUTION"
    // EventId=xxx|Date=xxx|File=xxx|FileSize=xxx|...
    QStack<QString>   mEventsExecutionStack;
    // For other events with START PASS FAIL result
    QStack<QString>   mEventsStack;

    // Actions list
    // Contains the information of actions to insert
    QMap< QString,QMap< QString,QString > > mActionsInfoList;
    // List of all actions to insert (sorted list)
    QStringList mActionsList;

    // Map containing the key/value for Admin method
    // exple : "ADMIN_CONNECT_LOCALHOST"="ENABLED", "DB_MYSQL_ENGINE"="SPIDER",...
    QMap<QString, QVariant> mAttributes;

protected:
    //! \brief Database entry
    bool  GrantUserPrivileges(QString strUserName, QString strUserPwd);

};
}
}

#endif
