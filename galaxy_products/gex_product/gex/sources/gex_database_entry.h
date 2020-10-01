#ifndef GEX_DATABASE_ENTRY_H
#define GEX_DATABASE_ENTRY_H

#define DB_LEGACY_ENTRY_FILE    ".gexdb_entry"  // LEGACY Database Entry definition file
#define DB_LEGACY_EXTERNAL_FILE ".gexdb_extern" // LEGACY external database file
#define DB_INI_FILE             ".gexdb_ini.xml"// Database definition

// Database storage mode
#define STATUS_CONNECTED                0x00001 // Connection available
#define STATUS_COMPRESS_PROTOCOL        0x00002 // Connection is opened in COMPRESS protocol
#define STATUS_SECURED_MODE             0x00004 // Restricted security mode activated
#define STATUS_DBUPTODATE               0x00008 // TDR or ADR UpToDate
#define STATUS_DBCOMPATIBLE             0x00010 // TDR or ADR not UpToDate but compatible

#define STATUS_MANUALLY_DISCONNECTED    0x00100 // The user manually disconnectes this database, do not automatically reconnect
#define STATUS_INTERNAL_REMOVE          0x00200 // When reloading the database list, all DB are marked for delete.At the end of the reload, if the tag always setted => delete

#define STATUS_EDIT                     0x01000 // Edit properties available
#define STATUS_REMOVE                   0x02000 // Delete available
#define STATUS_INSERTION                0x04000 // Insertion available
#define STATUS_HOUSEKEEPING             0x08000 // HouseKeeping available

#define GUI_READONLY                    0x00001 // Edit properties available but in read only mode
#define BUTTON_EDIT                     0x00002 // Button Edit properties available
#define BUTTON_REMOVE                   0x00004 // Button Delete available
#define BUTTON_INSERTION                0x00008 // Button Insertion available
#define BUTTON_HOUSEKEEPING             0x00010 // Button HouseKeeping available
#define BUTTON_EDIT_HEADER              0x00020 // Button HouseKeeping available

#include "gstdl_type.h"

#include <QObject>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <QDomElement>

class GexRemoteDatabase;
class GexDbPlugin_ID;

class GexDatabaseEntry: public QObject
{
    Q_OBJECT
public:
    GexDatabaseEntry(QObject* parent=0);
    // Copy constructor needed for JS access
    GexDatabaseEntry(const GexDatabaseEntry&);
    ~GexDatabaseEntry();

    GexDatabaseEntry& operator=(const GexDatabaseEntry& source);    // assignment operator

    // Setters
    void                SetPhysicalPath(QString value); // set the path of the db folder
    void                SetPhysicalName(QString value); // set the name of the db folder
    void                SetLogicalName(QString value);
    void                SetDescription(QString value);
    void                SetStorageEngine(QString value);
    void                SetDatabaseVersion(QString value) {m_strDatabaseVersion = value;}
    void                SetLocalDB(bool value);
    void                SetHoldsFileCopy(bool value);
    void                SetCompressed(bool value);
    void                SetSummaryOnly(bool value);
    void                SetBlackHole(bool value);
    void                SetExternal(bool value);
    void                SetReadOnly(bool value);
    void                SetCacheSize(double value);
    void                SetDbSize(double value);
    void                SetDeletePasssword(QString value);
    void                SetImportFilePasssword(QString value);
    void                SetStatusFlags(const UINT flag) {mStatusFlags |= flag;}
    void                UnsetStatusFlags(const UINT flag) {mStatusFlags &= ~flag;}
    void                SetGuiFlags(const UINT flag)    {mGuiFlags |= flag;}
    void                UnsetGuiFlags(const UINT flag)  {mGuiFlags &= ~flag;}
    bool                IsUpToDate() {return (mStatusFlags & STATUS_DBUPTODATE);}
    bool                IsCompatible() {return (mStatusFlags & STATUS_DBCOMPATIBLE);}
    // Getters
    QString             PhysicalPath() const; // return the path of the db folder
    QString             PhysicalName() const; // return the name of the db folder
    QString             LogicalName() const{return m_strLogicalName;}
    QString             Description() const {return m_strDescription;}
    QString             StorageType() const;
    QString             IconName();
    bool                IsStoredInDb() const {return (m_nDatabaseId > 0);}  // return true if the entry is stored in the admin db
    bool                IsStoredInFolderLocal() const {return m_bIsLocalDatabase;}
    bool                IsStoredInFolderServer() const {return !IsStoredInFolderLocal();}
    bool                HoldsFileCopy() const {return m_bHoldsFileCopy;} // true if Database holds a file copy, 'false' if only holds a link.
    bool                IsCompressed() const {return m_bIsCompressed;} // true if insert data in .gz format.
    bool                IsSummaryOnly() const {return m_bIsSummaryOnly;} // true if Database only holds the summary file
    bool                IsBlackHole() const {return m_bIsBlackHole;} // true if NO storage to do in the Database
    bool                IsFileBased() const {return !m_bIsExternal && !m_bIsBlackHole;}
    bool                IsExternal() const {return m_bIsExternal;} // true if External database (not a file based: remote / Corporate db).
    bool                IsReadOnly() const {return m_bIsReadOnly;}// true if database is READ ONLY (no insertion allowed)
    double              CacheSize() const {return m_fCacheSize;} // Database size. <0 if empty.
    double              DbSize() const {return m_fDatabaseSize;}
    QString             DeletePassword() const {return m_strDeletePassword;}
    QString             ImportFilePassword() const {return m_strImportFilePassword;}
    QString             TdrTypeRecorded() const {return mTdrTypeRecorded;}
    void                setTdrTypeRecorded(const QString& tdrType) {mTdrTypeRecorded = tdrType;}
    QString             StorageEngineRecorded() const {return m_strStorageEngine;}
    QString             GetTdrTypeName() const; // from plugin if connected or from TdrRecorded
    QString             DatabaseVersion() const {return m_strDatabaseVersion;}
    UINT                StatusFlags()   const {return mStatusFlags;}
    UINT                GuiFlags()   const {return mGuiFlags;}
    bool                IsCharacTdr();
    bool                IsManualProdTdr();
    bool                IsYmProdTdr();
    bool                IsUnknownTdr();
    bool                IsAdr();
    bool                IsLocalAdr();
    bool                IsSecuredDatabase();
    bool                MustHaveAdrLink();
    bool                HasAdrLink();


    GexDbPlugin_ID *    GetPluginID() const;    // return GexDbPluginID held by remotedb

    // IO functions
    static bool         ConvertFromLegacy(QString dbFilesFolder);   // will convert from Galaxy struct to xml following same order
    bool                LoadFromXmlFile(QString iniFilePath);       // Load database entry from .xml config file
    bool                LoadFromDbFolder(QString dbFolderPath);     // Search default xml file to load db entry, convert to default format if the lagacy format is found
    bool                SaveToXmlFile(const QString &destFile) const;   // Save database entry to .xml config file into destination folder
    bool                LoadDomElt(const QString &databasesPath, const QDomElement &node); // Load the object using the dom element
    QDomElement         GetDomElt(QDomDocument &domDoc) const ;     // return the dom element with the object description
    bool                CopyToDatabasesFolder(const QString &filePath, const QString &databasesFolder);

    bool                SetDbRestrictions(); // Define database restrictions depending on running mode, license type...
    void                SetExternalDbPluginIDPTR(GexDbPlugin_ID *pluginId);

    // Fields from ym_databases
    QString             m_strDatabaseRef;           // Name of the reference database if duplicated
    QString             m_strLastInfoMsg;           // Can contains error creation or error connection
    int                 m_nDatabaseId;              // ym_databases primary key
    int                 m_nNodeId;                  // node key that create this entry
    QMap<UINT,UINT>     m_mapGroupPermissions;      // Permissions for Read and Write
    QString             m_lstAllowedProducts;       // Products permissions for extraction
    QString             m_lstAllowedLots;           // Lots permissions for extraction
    QDateTime           m_clLastUpdate;
    QDateTime           m_LastStatusUpdate;
    QDateTime           m_LastStatusChecked;

    // for load-balancing
    bool    IncrementalUpdate(QString incrementalName, QString testingStage, QString target, QMap< QString, QString >  &summary);
    bool    TraceExecution(QString Command,QString Status,QString Summary);
    bool    TraceUpdate(QString Command,QString Status,QString Summary);
    bool    TraceInfo();

    // Update ym_actions to indicate step DONE
    // when task is DataPump and have Yieldtasks associated
    bool    SetActionMng(int ActionId,QString Command);
    bool    UpdateActionMng(QString Summary);
    QString GetActionMngAttribute(QString key);
    QMap<QString, QString> GetActionMngAttributes() {return mActionMngAttributes;}

    QMap<QString, QString> mActionMngAttributes;

    GexRemoteDatabase   *m_pExternalDatabase;       // Used if external database (oracle, mySQL, ...)

signals :
    void sLogRichMessage(QString,bool);

private:
    static QString NormalizeDatabaseFolderName(QString &strDatabaseFolder);                     // Build folder name from database name...replace any invalid char with a '_'

    // LOAD QDomElement Functions
    // Load main TAGS
    bool LoadAdminDbFromDom(const QDomElement &node);                                           // admindb TAG
    bool LoadDatabaseFromDom(const QDomElement &node);                                          // database TAG
    bool LoadExternalDbFromDom(const QDomElement &node);   // externaldatabase TAG
    // database TAGS
    bool LoadNameFromDom(const QDomElement &node);
    bool LoadDescriptionFromDom(const QDomElement &node);
    bool LoadStorageModeFromDom(const QDomElement &node);
    bool LoadStorageEngineFromDom(const QDomElement &node);
    bool LoadIsExtDbReadOnlyFromDom(const QDomElement &node);
    bool LoadIsExternalDbFromDom(const QDomElement &node);
    bool LoadSizeFromDom(const QDomElement &node);
    bool LoadImportPwdFromDom(const QDomElement &node);
    bool LoadDeletePwdFromDom(const QDomElement &node);
    bool LoadTdrTypeFromDom(const QDomElement &node);
    // admindb TAG
    bool LoadDbIdFromDom(const QDomElement &node);
    bool LoadNodeIdFromDom(const QDomElement &node);

    // DUMP QDomElement Functions
    // Dump main TAGS
    QDomElement GetAdminDbDom(QDomDocument &doc) const;                                         // admindb TAG
    QDomElement GetDatabaseDom(QDomDocument &doc) const;                                        // database TAG
    QDomElement GetExternalDbDom(QDomDocument &doc) const;                                      // externaldatabase TAG
    // database TAGS
    QDomElement GetNameDom(QDomDocument &doc) const;
    QDomElement GetDescriptionDom(QDomDocument &doc) const;
    QDomElement GetStorageModeDom(QDomDocument &doc) const;
    QDomElement GetStorageEngineDom(QDomDocument &doc) const;
    QDomElement GetIsExtDbReadOnlyDom(QDomDocument &doc) const;
    QDomElement GetIsExternalDbDom(QDomDocument &doc) const;
    QDomElement GetSizeDom(QDomDocument &doc) const;
    QDomElement GetImportPwdDom(QDomDocument &doc) const;
    QDomElement GetDeletePwdDom(QDomDocument &doc) const;
    QDomElement GetTdrTypeDom(QDomDocument &doc) const;

    bool SetConnected(); // set the entry as connected

    UINT                mGuiFlags;                  // Holds GUI flags
    UINT                mStatusFlags;               // Holds Status flags

    // !!!! IMPORTANT: update the assignment operator each time a field is added !!!!
    double              m_fDatabaseSize;
    double              m_fCacheSize;               // Database size. <0 if empty.
    bool                m_bIsReadOnly;              // true if database is READ ONLY (no insertion allowed)
    bool                m_bIsExternal;              // true if External database (remote / Corporate db). /// FIXME why sometime check the ptr sometime this var ?
    bool                m_bIsBlackHole;             // true if NO storage to do in the Database
    bool                m_bIsSummaryOnly;           // true if Database only holds the summary file
    bool                m_bIsCompressed;            // true if insert data in .gz format.
    bool                m_bHoldsFileCopy;           // true if Database holds a file copy, 'false' if only holds a link.
    bool                m_bIsLocalDatabase;         // true if stored in Local DB folder
    QString             m_strPhysicalPath;          // Full path to the database entry
    QString             m_strPhysicalName;          // Normalized name (no white char), used for folder creation!
    QString             m_strLogicalName;           // Database name
    QString             m_strDescription;
    QString             m_strDataTypeQuery;         // Type of data to query on in SQL-DB (wafer sort, final test,...)
    QString             m_strDeletePassword;
    QString             m_strImportFilePassword;
    QString             mTdrTypeRecorded;           // Holds the db type read from the ini file
    QString             m_strDatabaseVersion;
    QString             m_strStorageEngine;
};

#endif // GEX_DATABASE_ENTRY_H
