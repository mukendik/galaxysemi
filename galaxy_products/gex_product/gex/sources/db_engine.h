#ifndef GEX_DATABASE_CENTER_H
#define GEX_DATABASE_CENTER_H

#include <QString>
#include <QFile>
#include <QDir>
#include <QProgressDialog>
#include <QTextStream>

//#include "db_transactions.h" // needed for GexDatabaseInsertedFilesList
#include "db_inserted_files.h" // for GexDatabaseInsertedFiles
#include <gexdb_plugin_datafile.h> // for tdGexDbPluginDataFileList
#include <gqtl_datakeys.h>   //

typedef QList<GexDatabaseInsertedFiles> GexDatabaseInsertedFilesList;

class GexDatabaseQuery;
class CGexMoTaskItem;
class CGexMoTaskPatPump;
class CGexMoTaskDataPump;
class CGexMoTaskTriggerPatPump;
class GexDatabaseFilter;
class GexDatabaseEntry;
class GexDbPlugin_Filter;
class GexDbPlugin_ID;
class GexDbPlugin_Galaxy;
class GexDbPlugin_Connector;
struct GsData;

namespace GS
{

namespace StdLib
{

class Stdf;

}

namespace Gex
{

class DatabaseEngine : public QObject
{
    friend class EnginePrivate;

    Q_OBJECT

signals:
    void sOnSelectDatabase(GexDatabaseEntry *ptDatabaseEntry);
    void sShowDatabaseList(GexDatabaseEntry *pSelectDatabaseEntry, bool bDetach=false);
    void sDisplayStatusMessage(QString strText);
    void sButtonConnectEnabled(bool Enable);
    void sButtonConnectIsEnabled(bool &Enable);
    void sButtonReloadListEnabled(bool Enable);
    void sButtonReloadListIsEnabled(bool &Enable);

public slots:

    ///
    /// \brief IsConsolidationInProgress checks whether a consolidation is still in progress
    /// for the provided parameters. It first looks for a splitlot flagged BINNING_CONSOLIDATION,
    /// BIN_STATS, TEST_STATS, META, FSR, RNR, YIELD, BIN_SWAP or SUMMARY matching the filters,
    /// then inspects the agent job queue to see if a job is pending.
    /// \param databaseName: the name of the TDR on which the data has been inserted
    /// \param testingStage: the testings stage
    /// \param lot: a semicolon separated list of inserted lots
    /// \param sublots: a semicolon separated list of inserted sublots
    /// \param wafers: a semicolon separated list of inserted wafers
    /// \param consoType: the type of consolidation to check "raw" or "consolidated"
    /// \param testFlow: the test flow to check
    /// \param consoLevel: the consolidation level to check "test_flow" or "test_insertion"
    /// \param testInsertion: should the expected consolidation be done at test_insertion level
    /// this parameter holds the name of the test_insertion consolidation to check
    /// \return No consolidation in progress (0), consolidation in progress (1) or error (2)
    ///
    int IsConsolidationInProgress(
            QString databaseName,
            QString testingStage,
            QString lot,
            QString sublots,
            QString wafers,
            QString consoType,
            QString testFlow,
            QString consoLevel,
            QString testInsertion);

    // \brief Check the SPM of given params for JS script
    // \param DatabaseName : ADR where the data was inserted (mandatory)
    // \param TestingStage : (mandatory)
    // \param Product : (mandatory)
    // \param Lot : (mandatory)
    // \param Sublot : Sublot/Wafer (value or regexp)
    // \param Shell : can contain $ISODate, $Date, $Time, $UserFolder (can be empty)
    // \param RuleName : to execute a specific task (can be empty)
    // \param LogFileName : output message (can be empty)
    // \param TriggerFileName : the trigger requesting this check if any (can be empty)
    // \param FullSourceScript : the source script in order to log it in the SYA log file
    // \return  Passed(0),Failed(1),FailedValidationStep(2),Delay(3)
    int CheckSPMForJS(QString DatabaseName,
            QString TestingStage,
            QString Product,
            QString Lot,
            QString Sublot, QString Wafer,
            QString Shell,
            QString RuleName,
            QString LogFileName,
            QString TriggerFileName,
            QString TriggerSource
            );

    // \brief Check the SYA of given params for JS script
    // \param DatabaseName : TDR where the data was inserted (mandatory)
    // \param TestingStage : (mandatory)
    // \param Product : (mandatory)
    // \param Lot : (mandatory)
    // \param Sublot : Sublot/Wafer (value or regexp)
    // \param Shell : can contain $ISODate, $Date, $Time, $UserFolder (can be empty)
    // \param RuleName : to execute a specific task (can be empty)
    // \param LogFileName : output message (can be empty)
    // \param TriggerFileName : the trigger requesting this check if any (can be empty)
    // \param FullSourceScript : the source script in order to log it in the SYA log file
    // \return  Passed(0),Failed(1),FailedValidationStep(2),Delay(3)
    int CheckSYAForJS(QString DatabaseName,
            QString TestingStage,
            QString Product,
            QString Lot,
            QString Sublot, QString Wafer,
            QString Shell,
            QString RuleName,
            QString LogFileName,
            QString TriggerFileName,
            QString TriggerSource
            );

    /*
        Imports one and only one file into the specified database
        Returns message : 'error...' or 'ok'
        Use the given GexDatabaseKeysContent for input :
        - ConfigFileName : the full path + filename to the keys file
        - DataBaseName : DB to insert to
        - FileName : test data file to be inserted (full path + filename)
        After insertion, the KC will contain:
        - Product, Lot, .... according to config keys file
        - Status:
            PassedInsertion = 0,        // File inserted with success
            FailedInsertion,            // File failed insertion (eg: file corrupted)
            FailedValidationStep,       // Failed validation step, file not corrupted but doesn't match with the validation step
            DelayInsertion              // File failed insertion but not corrupted (eg: copy problem, etc), so delay insertion to try again later
        - Message : '' or 'error: or 'warning' .....
    */
    QString     ImportFile(GS::QtLib::DatakeysContent *lKC);

    // Import all files from given folder to a database
    int         ImportFolder(QString strDatabaseLogicalName,
                             QString strFolder,
                             bool bRecursive,
                             QStringList &strDataFilesToImport,
                             QStringList *pCorruptedFiles,
                             QString &strErrorMessage,
                             bool bEditHeaderInfo=false,
                             CGexMoTaskDataPump *ptTask=NULL);

    // Execute ONE trigger file
    bool        ExecuteScriptFile(CGexMoTaskTriggerPatPump *ptTask, QString &strSourceFile,
                                  QString &strErrorMessage);

    // Import ONE file into database (caller is always Yield-Man DataPump)
    bool        ImportOneFile(CGexMoTaskDataPump *ptTask, QString &strSourceFile,
                              QString &strErrorMessage, bool bEditHeaderInfo);

    // Import files to a database
    bool        ImportFiles(const QString &strDatabaseLogicalName,
                            const QStringList &sFilesSelected,
                            QStringList *pCorruptedFiles,
                            GexDatabaseInsertedFilesList & listInsertedFiles,
                            QString &strErrorMessage,
                            bool &bEditHeaderInfo,
                            bool bDelete=false, // delete the converted/copy std file
                            CGexMoTaskDataPump *ptTask=NULL,
                            bool bImportFromRemoteDB=false,
                            bool bFilesCreatedInFinalLocation=false);

    // Update log error
    // if bUpdateMoHistoryLog true, update/append the history file
    // if not bUpdateMoHistoryLog, just do a log
    void        UpdateLogError(QString strErrorMessage,
                               const QString strError,
                               bool bUpdateMoHistoryLog=true);

    // Updates status message
    void        UpdateStatusMessage(QString strMessage);

    // Update report log
    void        UpdateReportLog(const QString &strStatus,const QString &strFileName, const QString &strCause,
                                const QString &strDataPump, const QString &strDirectory, bool bUpdateMoReportLog=true,
                                unsigned int uiFileSize=0, unsigned int uiOriginalSize=0,
                                unsigned int uiTotalInsertionTime=0, unsigned int uiUncompressTime=0,
                                unsigned int uiConvertTime=0);

    // Load all databases entries link from local folder, server folder and admin DB
    void        LoadDatabaseEntries();
    // Load all databases from admon DB
    void        LoadDatabaseEntriesFromAdminDb();
    // Load all database links found in strDatabasePath
    void        LoadDatabaseEntriesFromFiles(QString strDatabasesPath);
    // Read Local+server folders to list available databases
    int         ReloadDatabasesList(QString strLocalDatabasesPath,
                                    QString strServerDatabasesPath);
    // Check Database list (connection + access) - update GUI if found modif
    void        CheckDatabasesList(bool ForceUpdateGui=false);
    // Alternate function...to be called ONLY if ReportOptions already initialized as will use ReportOptions db pathes !
    int         LoadDatabasesListIfEmpty(bool bForceReload=false);
    // Update a Database entry file (entry MUST already exist)
    bool        UpdateDatabaseEntry(QString strDatabaseName);
    // Delete a Database entry (folder and all files in it...)
    bool        DeleteDatabaseEntry(QString strName);

    // Connect Database with given name : exemple : "gexdb*"
    // If several databases with same name, connect them all
    bool    ConnectDatabase(QString DatabaseName);
    // Disconnect Database with given name : exemple : "gexdb*"
    // If several databases with same name, disconnect them all
    bool    DisconnectDatabase(QString DatabaseName);

    // Return the database 'Import files' password protection
    QString     GetImportPassword(QString strDatabaseLogicalName);
    // Return the database 'Delete' password protection
    QString     GetDeletePassword(QString strDatabaseLogicalName);
    // Returns true if IsYmProdTdr
    bool        isYMDB(const QString &strDB );

    // Check if the folder is a Root folder C:/, D:/, unix root folder
    static bool IsRootFolder(QString strFolder);

    // Update Filter value table if a new value is found.
    bool        UpdateFilterTableFile(QString strDatabaseFolder,int iFilterID, QString strValue);

    // Recursive function to erase all EMPTY &sub-folders
    static bool DeleteEmptySubFolders(QString strFolder,bool bEnteringSubFolder=false);
    // Recursive function to erase folder &sub-folders
    static bool DeleteFolderContent(QString strFolder);
    // Recursive function to copy folder &sub-folders
    static bool CopyFolderContent(QString strOldFolder, QString strNewFolder);
    // List of files matching with extensions (can be a RegExp)
    static QStringList    GetListOfFiles(
            const QString & strRootPath, const QString & strExtensions="*",
            bool bScanSubFolders=false,
            QDir::SortFlags eSortFlags=QDir::Time,int  Priority=-1);

    QString GetLastMessage() {return m_strInsertionShortErrorMessage;}

public:
    GDECLARE_ERROR_MAP(DatabaseEngine)
    {
        // IMPORT ERRORS
        eWrongLicense = 100,            // Wrong license for importing file
        eDatabaseNotFound,              // DB not found
        eReadOnlyDatabase,              // Read only DB
        eFileNotFound,                  // Could not find file to insert
        eFailedToCreateUnzipFolder,     // Could not create unzip folder
        eFailedToUnzipFile,             // Could not unzip file
        eFailedToConvertFile,           // Failed to convert file to STDF
        eMultipleFilesInArchive,        // Multiple files compressed in archive
        eFailedToCopyFile,              // Could not copy file
        eFailedToLoadDataKeys,          // Could not load datakeys with STDF file
        eFailedToLoadConfigKeysFile,    // Could not load/evaluate config keys file
        eFailedToExecPreInsertionJs,    // Failed to execute pre-insertion JS
        eErrorInPreInsertionJs,         // Error in pre-insertion JS
        ePreInsertionJsReject,          // Pre-insertion JS rejected or delayed the file
        eConfigkeysSyntaxError,         // Syntax error in configkeys file
        eStdfCorrupted,                 // STDF file corrupted
        eTooFewParts,                   // File has too few parts
        eUnexpectedPassHardBins,        // Unexpected PASS Hard bins (not part of specified list)
        eTdrInsertionFail,              // TDR insertion with Fail status
        eTdrInsertionDelay,             // TDR insertion with Delay status
        eFailedToAnalyzeFile,           // Failed to analyze file
        eFailedToCreateScriptFile,      // Failed to create script file
        eFailedToWriteScriptFunction,   // Failed to write a function into script file
        eFailedToReadIndexFile,         // Failed to read index file (file based insertion)
        eFailedToCreateIndexFile,       // Failed to create index file (file based insertion)
        eFailedToCreateEmail,           // Failed to create email
        eIncompleteFile,                // Incomplete file
        eFailedToMoveFile,              // Could not move file
        eFailedToCompressFile           // Could not compress file
    }
    GDECLARE_END_ERROR_MAP(DatabaseEngine)

    DatabaseEngine(QObject* parent);
    ~DatabaseEngine();

    void Activate()     { mAllowed=true; }
    bool isActivated()  {return mAllowed;}

    bool        CreateDatabaseEntry(GexDbPlugin_ID *currentPlugin, const QString &lLogicalName, QString &error);

    bool        SaveDatabaseEntry(GexDatabaseEntry *pDatabaseEntry);
    bool        CheckDatabaseEntryStatus(GexDatabaseEntry *pDatabaseEntry, bool GetDatabaseSize=false);
    bool        UpdateDatabaseEntry(QString strPath, GexDatabaseEntry *pDatabaseEntry);
    bool        UploadDatabaseEntry(GexDatabaseEntry *pDatabaseEntry);

    // If CSV file with multiple lots, split it in smaller temporary CSV files (one lot per CSV)
    bool        SplitMultiLotCsvFiles(QStringList &sFilesSelected);

    // search for a DB entry with the given name, connected or not
    GexDatabaseEntry *FindDatabaseEntry(QString strDatabaseLogicalName, bool bOnlyConnected=true);

    // Maps Galaxy query to GexDB plugin query (for file selection: first 5 filters)
    void        MapQueryToCorporateFilters(GexDbPlugin_Filter& dbPluginFilter,
                                           const GexDatabaseQuery& dbQuery);

    // Maps Galaxy query to GexDB plugin query (for field selection)
    void        MapQueryToCorporateFilters(GexDbPlugin_Filter& dbPluginFilter,
                                           const GexDatabaseQuery& dbQuery,
                                           const QStringList &strFilterName);

    void        QueryFillPluginFilter(GexDbPlugin_Filter & clPluginFilter,
                                      const GexDatabaseFilter& dbFilter);

    QStringList QuerySelectFiles(GexDatabaseQuery *pQuery, QString &aErrorMessage, unsigned uFileLimit=0);

    bool        PurgeDataBase(GexDatabaseQuery *pQuery, QProgressBar *poProgress=0);
    bool        ExportCSVCondition(GexDatabaseQuery *pQuery, const QMap<QString, QString>& oConditions, QProgressDialog *poProgress=0);

    QStringList QuerySelectFilter(const GexDatabaseFilter &dbFilter);

    QStringList QuerySelectFilter(const GexDatabaseQuery & dbQuery, const QStringList &filtersName);
    QStringList QueryGetParameterList(const GexDatabaseFilter& dbFilter, bool bParametricOnly);
    QStringList QueryGetBinningList(const GexDatabaseFilter& dbFilter, bool bSoftBin=false);
    QStringList QueryGetProductList_Genealogy(const GexDatabaseFilter& dbFilter,
                                              bool bAllTestingStages);
    QStringList QueryGetProductList(const GexDatabaseFilter& dbFilter,QString strValue="");

    //! \brief Retrieve all test conditions (using current filter)
    //! \return 'ok' or 'error...'
    QString QueryTestConditionsList(const GexDatabaseFilter& dbFilter, QStringList &output);

    QList<GexDatabaseQuery> QuerySplit(const GexDatabaseQuery& dbQuery, QString& errorMessage);

    QString  BuildDatabaseSize(double fSize, bool bRemoteDatabase);

    //! Sandrine ?
    // Extract Keys from data file (MIR data mostly).
    Q_INVOKABLE bool ExtractFileKeys(QString strDataFile,
                         QtLib::DatakeysContent &dbKeysContent);
    //! \brief Script compatible interface. Returns "error..." or "ok".
    Q_INVOKABLE QString ExtractFileKeys(const QString &lDataFile, QtLib::DatakeysContent *lKeysContent);

    // Reads a string from STDF file...and remove any leading space.
    static void ReadStringToField(GS::StdLib::Stdf *pStdfFile,char *szField);

    QList<GexDatabaseEntry*> mDatabaseEntries;			// List of Databases found (local & server)

    // Status code returned when inserting a data file
    enum ExecutionStatus
    {
        Passed = 0,        // File inserted with success
        Failed,            // File failed insertion (eg: file corrupted)
        FailedValidationStep, // Failed validation step, file not corrupted but doesn't match with the validation step
        Delay              // File failed insertion but not corrupted (eg: copy problem, etc), so delay insertion to try again later
    };

    // For Admin Server
    bool                m_bDatabasesLoaded;
    int                 m_nIndexLocalDatabase;
    GexDatabaseEntry*   FindDatabaseEntry(int iDatabaseEntry, bool bOnlyConnected=false);
    GexDatabaseEntry*   FindDatabaseEntry(QString strPluginName, QString strHost, int nPort, QString strDriver, QString strSchema, QString strDatabase);

    static int      GetFilterIndex(const QString &filterName);
    static int      GetLabelFilterIndex(const QString &filterLabel);

    // Functions to handle files that have already been processed (only used if DataPump mode is : "leave files on server, maintain list of files processed")
    // Description : Check if file is ready to be copied
    // Argument(s) : string strFileName	: name of the file is ready
    // Return      : true if file ready, false else
    static bool     CheckFileForCopy(QString strFileName,
                                     CGexMoTaskItem *ptTask,
                                     QString &cause);
    // Tell if this file must be processed or not...
    static bool     CheckIfFileToProcess(QString strFileName,
                                         CGexMoTaskItem *ptTask,
                                         QString &cause);

    // \brief Check the SPM of given params
    // \param dbKeysContent :
    //  o DatabaseName : TDR where the data was inserted (mandatory)
    //  o TestingStage : (mandatory)
    //  o Product : (mandatory)
    //  o Lot : (mandatory)
    //  o Sublot : Sublot/Wafer (value or regexp)
    //  o SpmTask : to execute a specific task (can be empty)
    // \param Shell : can contain $ISODate, $Date, $Time, $UserFolder (can be empty)
    // \param LogFileName : output message (can be empty)
    // \param TriggerFileName : the trigger requesting this check if any (can be empty)
    // \param FullSourceScript : the source script in order to log it in the SYA log file
    // \return  Passed(0),Failed(1),FailedValidationStep(2),Delay(3)
    int CheckStatMon(QtLib::DatakeysContent& dbKeysContent, QString& Shell, QString& LogFileName, QString& TriggerFileName, QString& TriggerSource);

    /// Brief return the ADR database name from the TDR Database name
    const QString GetADRDBNameFromTDRDBName(const QString &tdrDatabaseName);
    GexDatabaseEntry *GetADRDatabaseFromTDR(const QString &tdrDatabaseName);
    bool UpdateAdrDatabaseConnector(GexDbPlugin_Connector &adrSettingConnector, const GexDbPlugin_Connector *tdrSettingConnector);

private:
    // database is allowed
    bool            mAllowed;
    QString         m_strInsertionShortErrorMessage;		// Short error message in case of insertion error (for MO report)

    char*           FormatIdxString(char *szString);
    QString         FormatIdxString(const QString &data);
    QString         FormatArgumentScriptString(const QString &strString);
    QStringList     cFilesProcess;	// Filled with the list of files impoorted in a database.
    QStringList     mFilesToImport; // Keep track of files to import

    // Build complete list of files matching insertion criteria (recursive mode supported)
    void            SearchMatchingFilesInFolder(const QString &strSourceFolder,
                                                bool bRecursive,
                                                QStringList &strDataFilesToImport,
                                                CGexMoTaskItem *ptTask);

    // Process Script (ECMA JavaScript .js) file
    int             ProcessScriptFile(QString strFileName, bool *pbDeleteTrigger,
                                      QString &strErrorMessage, QString &strShortErrorMsg);
    // Trigger file processing
    // Process trigger file
    bool    IsTriggerFile(QString strTriggerFileName);
    //! \brief Sandrine ?
    int     ProcessTriggerFile(CGexMoTaskTriggerPatPump *ptTask, QString strTriggerFileName, bool *pbDeleteTrigger,
                                       QString &strErrorMessage, QString &strShortErrorMsg);
    //! \brief Process xml compliant trigger files
    int     ProcessTriggerFile_XML(CGexMoTaskItem *ptTask,
                                           QString strTriggerFileName, bool *pbDeleteTrigger,
                                           QString &strErrorMessage, QString &strShortErrorMsg);

#ifdef GCORE15334

    //! \brief Process trigger file: PAT action: Single wafer processing
    //! \return "ok" or "error: ..."
    QString ProcessTrigger_PAT(QString &strTriggerFileName, QTextStream &hTriggerFile, CGexMoTaskPatPump *PATPump);
    //! \brief Process trigger file: Composite PAT action (pre-process a full lot)
    bool    ProcessTrigger_CompositePAT(QString &strTriggerFileName,QTextStream &hTriggerFile);
#endif

    //! \brief Process trigger file: WAFER_EXPORT action: Single wafer processing
    bool    ProcessTrigger_WAFER_EXPORT(CGexMoTaskTriggerPatPump *ptTask, QString &strTriggerFileName, QTextStream &hTriggerFile);
    //! \brief Process trigger file: Y123 action: insertion for Y123
    int     ProcessTrigger_Y123(CGexMoTaskItem *ptTask,GexDatabaseEntry *pDatabaseEntry,
                                QString &strTriggerFileName,QTextStream &hTriggerFile,
                                bool *pbDeleteTrigger, QString &strErrorMessage, QString &strShortErrorMsg);

    //! \brief Read/Append database Index file. Manages retry mode.
    bool            SmartOpenFile(QFile &f, QIODevice::OpenMode openMode);

    //! \brief Create a summary file from a STDF file: Merges all sites info.
    //! \return 0 on error, else the summary file size ? Sandrine ?
    unsigned long   BuildSummarySTDF(QString strSourceArchive,bool bMakeLocalCopy,QString strFileNameSTDF,
                                     GS::QtLib::DatakeysContent &cKeyContent);

    //! \brief On import error, make sure error code and messages all get set (Keys, members...).
    void            SetImportError(GS::QtLib::DatakeysEngine &dbKeysEngine, QString &ErrorMsg, const int ImportStatus);

    //! \brief Add last error to import warnings (after insertion itself is passed).
    void            AddLastErrorToImportWarnings(GS::QtLib::DatakeysEngine &dbKeysEngine, QString &LastErrorMsg);

    //! \brief Add a list of warnings to import warnings (after insertion itself is passed).
    void            AddWarningsToImportWarnings(GS::QtLib::DatakeysEngine &dbKeysEngine, const QStringList & Warnings);

    // Import one file into the database
    int ImportFile(GexDatabaseEntry* pDatabaseEntry,
                   QString strSourceArchive,
                   bool bFromArchive,
                   QString strFileName,
                   QString& strFileInserted,
                   QString& strErrorMessage,
                   bool& bEditHeaderInfo,
                   GS::QtLib::DatakeysEngine& dbKeysEngine,
                   unsigned int* puiFileSize,
                   unsigned int* puiUncompressTime,
                   unsigned int* puiConvertTime,
                   CGexMoTaskDataPump* ptTask = NULL,
                   bool bImportFromRemoteDB = false,
                   bool bFilesCreatedInFinalLocation = false);

    // Import one WYR file into the database
    int             ImportWyrFile(GexDatabaseEntry *pDatabaseEntry,
                                  const QString & strWyrFile,
                                  QString &strErrorMessage,
                                  GS::QtLib::DatakeysEngine& dbKeysEngine,
                                  unsigned int *puiFileSize,
                                  CGexMoTaskDataPump *ptTask=NULL);
    QStringList     SplitFileBasedDbIndexLine(const QString & aIndexLine);
    QStringList     FindMatchingFiles(QDate *pCurrentDate,GexDatabaseQuery *pQuery,GexDatabaseEntry *pDatabaseEntry);
    bool            isMatchingFilter(const QString &strFilter, const QString &strString);
    void            FindMatchingFilter(QStringList &cEntriesMatching, const QDate& currentDate,
                                       const GexDatabaseFilter& dbFilter,
                                       const GexDatabaseEntry * pDatabaseEntry);
    void            FindMatchingFilter(QStringList& cEntriesMatching, const QDate& currentDate,
                                       const GexDatabaseQuery& dbQuery, const QStringList & filtersName,
                                       const GexDatabaseEntry * pDatabaseEntry);
    long            iDatabaseFileIndex;	// Index used to create unique file name in the database.

    // Load config keys
    // return false on error
    bool            LoadConfigKeys(GS::QtLib::DatakeysEngine& dbKeysEngine,
                                   CGexMoTaskItem *ptTask, bool &bEditHeaderInfo,
                                   bool&bFailedValidationStep, QString &strErrorMessage,
                                   QString &strShortErrorMsg, QString & strFileNameSTDF);
    bool            LaunchPostInsertionShell(int nStatus, QString& strErrorMessage,
                                             GS::QtLib::DatakeysContent& dbKeysContent,
                                             CGexMoTaskItem *ptTask);
    QStringList GetValidateFilter(const QDate comboBoxDate, const GexDatabaseFilter& dbFilter);
    /*!
     * \fn RunPreInsertionScript
     */
    int RunPreInsertionScript(QString& strErrorMessage,
                              GS::QtLib::DatakeysEngine& dbKeysEngine,
                              CGexMoTaskItem* ptTask,
                              bool bImportFromRemoteDB,
                              QString& lLocalErrorMessage);
    /*!
     * \fn ImportFileSqlite
     */
    int ImportFileSqlite(GexDatabaseEntry* pDatabaseEntry,
                         QString strSourceArchive,
                         QString lFileToInsert,
                         QString& strErrorMessage,
                         bool& bEditHeaderInfo,
                         GS::QtLib::DatakeysEngine& dbKeysEngine,
                         CGexMoTaskItem* ptTask,
                         bool bImportFromRemoteDB);
    /*!
     * \fn ImportFileSqliteSplitlot
     */
    int ImportFileSqliteSplitlot(struct GsData* lGsData,
                                 int lSqliteSplitlotId,
                                 GexDatabaseEntry* pDatabaseEntry,
                                 QString lFileToInsert,
                                 QString& strErrorMessage,
                                 bool& bEditHeaderInfo,
                                 GS::QtLib::DatakeysEngine& dbKeysEngine,
                                 CGexMoTaskItem* ptTask,
                                 bool bImportFromRemoteDB,
                                 QString& lErrorMessage);
    /*!
     * \fn DbKeyContentSetFileInfo
     */
    void DbKeyContentSetFileInfo(GS::QtLib::DatakeysEngine& dbKeysEngine,
                                 QString lFileToInsert,
                                 QString lFileNameSTDF,
                                 QString strSourceArchive);
    /*!
     * \fn DatabaseInsertFile
     */
    int DatabaseInsertFile(struct GsData* lGsData,
                           int lSqliteSplitlotId,
                           GexDatabaseEntry* pDatabaseEntry,
                           QString lFileToInsert,
                           QString& strErrorMessage,
                           GS::QtLib::DatakeysEngine& dbKeysEngine,
                           CGexMoTaskItem* ptTask,
                           bool bImportFromRemoteDB);
    /*!
     * \fn CheckOverloadedKeyWords
     */
    void CheckOverloadedKeyWords(long t_SetupTime,
                                 long t_StartTime,
                                 long t_FinishTime,
                                 GS::QtLib::DatakeysEngine& dbKeysEngine,
                                 QString& strError);

    void    SimplifyFtpList(GexDatabaseEntry *pDatabaseEntry,
                            tdGexDbPluginDataFileList &cFilesToFtp,
                            QStringList & cMatchingFiles);

    bool    HasFtpFiles(const tdGexDbPluginDataFileList &lFiles) const;

    void    UpdateFtpList(GexDatabaseEntry *pDatabaseEntry,
                          tdGexDbPluginDataFileList &cDataFiles,
                          GexDatabaseInsertedFilesList &listInsertedFiles,
                          QStringList &strCorruptedFiles);
};
}
}

#endif // GEX_DATABASE_CENTER_H
