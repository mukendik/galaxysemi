///////////////////////////////////////////////////////////
// External Database: Access functions.
///////////////////////////////////////////////////////////

#ifndef GEX_EXTERNAL_DB_H
#define GEX_EXTERNAL_DB_H

// Standard includes

// QT includes
#include <QStringList>
#include <QDomElement>

// Galaxy modules includes
#include <gstdl_errormgr.h>

// Local includes
#include "gexdb_plugin_base.h"
#include "gexdb_plugin_er_dataset.h"
#include "statistical_monitoring_monitored_item_unique_key_rule.h"

// Forward declarations
class QTextEdit;
class QProgressBar;
class GexMainwindow;
struct GsData;
struct StatMonAlarm;
struct StatMonDataPoint;
class GexDatabaseFilter;

///////////////////////////////////////////////////////////
// GexDbPlugin_ID class: plugin identification...
///////////////////////////////////////////////////////////
class GexDbPlugin_ID
{
public:
    GexDbPlugin_ID();
    ~GexDbPlugin_ID();

    bool                LoadPlugin(QString &pluginFilePath, QMainWindow *pMainWindow);
    QString             pluginName();
    QString             pluginFileName();
    QString             pluginFilePath();
    unsigned int        pluginBuild();
    void                setConfigured(bool value);
    bool                isConfigured();

    GexDbPlugin_Base    *m_pPlugin;

private:
    QLibrary            *m_pLibrary;
    QString             m_strFilePath;
    QString             m_strFileName;
    bool                m_bPluginConfigured;
};

// Remote database login & mapping details
class GexRemoteDatabase : public QObject
{
    Q_OBJECT

public slots:
    void StopProcess();

signals:
    //void NewMessage(const QString&);

public:
    GDECLARE_ERROR_MAP(GexRemoteDatabase)
    {
        eLoadPlugin,                // Error loading plugin
        eNoPluginLoaded,            // No plugin loaded
        ePluginNotConfigured,       // Plugin not configured
        eOpenSettingsFile,          // Failed opening settings file
        eMarkerNotFound,            // XML marker not found in settings file
        eReadField,                 // Error reading field in settings file
        eMissingPlugin,             // Specified plugin not found in plgugin directory
        ePluginSettings,            // Error reading plugin settings
        eCreateSettingsFile,        // Failed creating settings file
        ePlugin                     // Error in plugin function
    }
    GDECLARE_END_ERROR_MAP(GexRemoteDatabase)

    // Constructor / destructor functions
    GexRemoteDatabase();            // Consturctor
    ~GexRemoteDatabase();           // Destructor

    // Common functions
    QString         GetPluginDirectory(void) { return m_strPluginDirectory;	}
    void            GetAvailablePlugins(QList<GexDbPlugin_ID*> & pPluginList);
    void            UnloadPlugins(QList<GexDbPlugin_ID*> & pPluginList);
    void            ConfigurePlugin(GexDbPlugin_ID *pPluginID);
    void            GetLastError(QString & strError);
    void            SetPluginIDPTR(GexDbPlugin_ID *pPluginID);                          // Load plugin
    bool            LoadPluginFromDom(const QDomElement &node);                         // Load plugin specified from Dom elt
    bool            LoadPlugin(const QString &strSettingsFile);                         // Load plugin specified in settings file
    bool            LoadPluginID(QString &strPluginFileName, unsigned int uiPluginBuild, QString &strPluginName);	// Load plugin
    bool            IsInsertionSupported();                                             // Returns true if insertion supported, false else
    bool            IsUpdateSupported();                                                // Returns true if DB update supported, false else
    bool            IsTestingStagesSupported();                                         // Returns true if testing stages are supported, false else
    bool            IsParameterSelectionSupported();                                    // Returns true if parameter selection supported (for filtering), false else
    bool            IsEnterpriseReportsSupported();                                     // Returns true if Enterprise Reports supported, false else
    bool            IsReportsCenterSupported();                                         // Returns true if Reports Center supported, false else
    bool            IsTestingStage_FinalTest(const QString & strTestingStage);          // Returns true if the specified testing stage is Final Test
    bool            IsTestingStage_Foundry(const QString & strTestingStage);            // Returns true if the specified testing stage is Foundry (E-Test)
    bool            GetTestingStageName_Foundry(QString & strTestingStage);             // Returns the name of Foundry testing stage (E-Test)
    void            GetWarnings(QStringList & strlWarnings);                            // Retrieve list of insertion warnings
    // Returns the list of supported testing stages by the plugin
    void            GetSupportedTestingStages(QStringList & strlSupportedTestingStages);
    bool            SetTestingStage(const QString & strTestingStage);
    int             GetTestingStageEnum(const QString & strTestingStage);

    // Return supported binning types.
    // ConsolidatedType: "Y" or "N"
    bool            GetSupportedBinTypes(const QString &TestingStage, QStringList & strlBinTypes, const QString &ConsolidatedType="Y");

    // Retrieve list of available fields for the given testing stage
    // output will contains all the fields
    void            GetRdbFieldsList(	const QString &TestingStage, QStringList &output,
                                        const QString &In_GUI="Y",              // Y, N or *
                                        const QString &BinType="N",             // BinType can be H, S or N (or even *)
                                        const QString &Custom="*",              // Custom can be Y, N or *
                                        const QString &TimeType="*",            // TimeType can be Y, N or *
                                        const QString &ConsolidatedType="Y",    // ConsolidatedType can be Y, N or *
                                        const QString &Facts="N" ,               // Facts can be Y, N, or *
                                        bool  OnlyStaticMetaData=false
            );
    bool            GetRdbFieldProperties(const QString& TestingStage, const QString& MetaDataName, QMap<QString,QString>& Properties);

    // Get consolidated field name corresponding to specified decorated field name
    bool            GetConsolidatedFieldName(const QString & strTestingStage, const QString & strDecoratedField, QString & strConsolidatedField, bool *pbIsNumeric = NULL, bool * pbIsBinning = NULL, bool * pbIsTime = NULL);

    void            GetLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);
    void            GetConsolidatedLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);
    void            GetLabelFilterChoicesLotLevel(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);
    void            GetLabelFilterChoicesWaferLevel(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);
    GexDbPlugin_ID  *GetPluginID() { if(m_pPluginID && m_pPluginID->m_pPlugin) m_pPluginID->m_pPlugin->m_strDBFolder = m_strDatabasePhysicalPath; return m_pPluginID; }
    QString         GetPluginName();
    QString         GetPluginErrorMsg();
    QString         GetPluginErrorCode();
    QString         GetPluginErrorDescription();
    QString         GetTdrTypeName();
    QString         GetTdrStorageEngine();
    bool            IsCharacTdr();
    bool            IsManualProdTdr();
    bool            IsYmProdTdr();
    bool            IsAdr();
    bool            IsLocalAdr();
    bool            IsSecuredMode();
    bool            SetSecuredMode(bool toSecured);

    QWidget         *GetRdbOptionsGui();                                        // Returns a pointer on an options GUI, if the plugin supports custom options
    bool            GetRdbOptionsString(QString & strRdBOptionsString);         // Returns an options string, if the plugin supports custom options
    bool            SetRdbOptionsString(const QString & strRdBOptionsString);   // Sets the plug-in options GUI according to options string passed as argument

    // Database administration functions
    bool            IsCustomerDebugModeActivated();
    bool            IsDbConnected();
    bool            IsDbUpToDate(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name, unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name, unsigned int *puiLatestSupportedDbVersion_Build);
    bool            IsDbUpToDateForExtraction(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name, unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name, unsigned int *puiLatestSupportedDbVersion_Build);
    bool            IsDbUpToDateForInsertion(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name, unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name, unsigned int *puiLatestSupportedDbVersion_Build);
    // Update the DB using the command if any.
    bool            UpdateDb(QString command="");
    bool            UpdateConsolidationProcess(int eTestingStage=0); // eTestingStage: 0 : to update all testingstage else update only one testing stage

    // Incremental Update methods

    // Run a specific incremental update
    bool            IncrementalUpdate(QString incrementalName, QString testingStage, QString target, QMap< QString, QString >  &summary);
    // Get the next incremental update for Schedule for ALL according to the Frequency
    bool            GetNextAutomaticIncrementalUpdatesList(QMap< QString,QMap< QString,QStringList > > & IncrementalUpdatesList);
    // Get the next incremental update for Schedule for ALL or specific for manual update
    bool            GetFirstIncrementalUpdatesList(QString incrementalName, QMap< QString,QMap< QString,QStringList > > & incrementalUpdatesList);
    // Get/Update the total count of remaining splitlots for incremental update
    bool            GetIncrementalUpdatesCount(bool checkDatabase,int &incrementalSplitlots);
    // Incremental updates settings
    bool            GetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > &incrementalUpdates);
    bool            SetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > &incrementalUpdates);
    bool            IsIncrementalUpdatesSettingsValidValue(QString &name, QString &value);

    virtual int IsConsolidationInProgress(
            QString testingStage,
            QString lot,
            QString sublots,
            QString wafers,
            QString consoType,
            QString testFlow,
            QString consoLevel,
            QString testInsertion);

    // Try to retrieve the current total size (in octet) used by the DB on his server or on the local HDD
    bool            GetTotalSize(int &size);
    bool            IsAutomaticIncrementalUpdatesEnabled();
    // Check if automatic startup are enabled
    bool            IsAutomaticStartup();
    bool            SetAutomaticStartup(bool bValue);
    const QString   GetTdrLinkName();
    void            SetTdrLinkName(const QString &value);
    const QString   GetAdrLinkName();
    void            SetAdrLinkName(const QString &value);
    bool            MustHaveAdrLink();
    bool            PurgeSplitlots(QStringList & strlSplitlots, QString & strTestingStage, QString & strCaseTitle, QString *pstrLog=NULL);	// Purge selected splitlots
    bool            ConsolidateWafer(QString & strLotID, QString & strWaferID, QString & strCaseTitle, QString *pstrLog=NULL);	// Consolidate specified wafer
    bool            ConsolidateWafers(QString & strLotID, QString & strCaseTitle, QString *pstrLog=NULL);	// Consolidate specified wafer
    bool            ConsolidateLot(QString & strLotID, bool bConsolidateOnlySBinTable=false, bool bCallConsolidationFunction=true);
    // Sandrine : could you add comments in headers. What does all those functions do ?
    bool            GetGlobalOptionName(int nOptionNb, QString &strValue);
    bool            GetGlobalOptionTypeValue(int nOptionNb, QString &strValue);
    bool            GetGlobalOptionValue(int nOptionNb, QString &strValue);
    bool            GetGlobalOptionValue(int nOptionNb, QString &strValue, bool &bIsDefined);
    bool            GetGlobalOptionDefaultValue(int nOptionNb, QString &strValue);
    bool            GetGlobalOptionDescription(int nOptionNb, QString &strValue);
    bool            GetGlobalOptionReadOnly(int nOptionNb, bool &bIsReadOnly);
    bool            IsGlobalOptionValidValue(int nOptionNb, QString &strValue);
    // Retrieve an option value simply from his name
    bool            GetGlobalOptionValue(QString strOptionName, QString &strValue);
    // Save an option value simply from his name
    bool            SetGlobalOptionValue(QString strOptionName, QString &strValue);

    // Data insertion
    void            SetInsertionValidationOptionFlag(unsigned int uiInsertionValidationOptionFlag);
    void            SetInsertionValidationFailOnFlag(unsigned int uiInsertionValidationFailOnFlag);
    unsigned int    GetInsertionValidationOptionFlag();
    unsigned int    GetInsertionValidationFailOnFlag();
    /*!
     * \fn InsertDataFile
     */
    bool InsertDataFile(struct GsData* lGsData,
                        int lSqliteSplitlotId,
                        const QString& strDataFileName,
                        GS::QtLib::DatakeysEngine& dbKeysEngine,
                        bool* pbDelayInsertion);
    bool            InsertWyrDataFile(const QString & strDataFileName,
                                      const QString & strSiteName,
                                      const QString & strTestingStage,
                                      unsigned int uiWeekNb,
                                      unsigned int uiYear,
                                      bool *pbDelayInsertion);
    // Insert an alarm for specified splitlot/testing stage
    // returns false on error
    bool            InsertAlarm(GexDbPlugin_Base::AlarmCategories eAlarmCat, GexDbPlugin_Base::AlarmLevels eAlarmLevel, long lItemNumber, QString & strItemName, unsigned int uiFlags, float fLCL, float fUCL, float fValue, QString strUnits);
    bool            InsertAlarm_Wafer(GexDbPlugin_Base::AlarmCategories eAlarmCat, GexDbPlugin_Base::AlarmLevels eAlarmLevel, long lItemNumber, QString & strItemName, unsigned int uiFlags, float fLCL, float fUCL, float fValue, QString strUnits);

    // Generic data extraction functions
    bool            QuerySQL(QString & strQuery, QList<QStringList> & listResults);		// Execute Query and return results
    bool            QuerySplitlots(GexDbPlugin_Filter & cFilters, GexDbPlugin_SplitlotList & clSplitlotList, bool bPurge=false);						// Return all splitlots (given filters on several fields)
    bool            QueryBinlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bSoftBin=false,bool bIncludeBinName=false, bool bProdDataOnly=false);		// Return all binnings (given filters on other fields)
    bool            QueryProductList(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,QString strProductName="");				// Return all products (use only date in the filter)
    bool            QueryProductList_Genealogy(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bAllTestingStages);	// Return all products for genealogy reports, with data for at least 2 testing stages (use only date in the filter)
    bool            QueryTestConditionsList(GexDbPlugin_Filter & filters,
                                            QStringList & matchingTestConditions);                                                  // Return all test conditions corresponding to the splitlots (according to given filters)

    // Data extraction for GEX standard reports
    // Return all valid values for a field (given filters on other fields)
    bool            QueryField(GexDbPlugin_Filter & cFilters, QStringList & cMatchingValues, bool bSoftBin=false);
    // Returns all tests (given filters on other fields)
    bool            QueryTestlist(GexDbPlugin_Filter & cFilters, QStringList & cMatchingValues, bool bParametricOnly);
    // Return all valid Data files (given filters on several fields)
    bool            QueryDataFiles(GexDbPlugin_Filter & cFilters,
                                   const QString & strTestlist,
                                   tdGexDbPluginDataFileList & cMatchingFiles,
                                   const QString & strDatabasePhysicalPath,
                                   const QString & strLocalDir,
                                   bool *pbFilesCreatedInFinalLocation,
                                   GexDbPlugin_Base::StatsSource eStatsSource);

    // Data extraction for GEX enterprise reports
    bool            GetDataForProd_UPH(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList);                 // Compute data for Production - UPH graph
    bool            GetDataForProd_Yield(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList, int nBinning, bool bSoftBin);	// Compute data for Production - Yield graph
    bool            GetDataForProd_ConsolidatedYield(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList);   // Compute data for Production - Consolidated Yield graph
    bool            GetDataForWyr_Standard(GexDbPlugin_Filter & cFilters, GexDbPlugin_WyrData & cWyrData);                      // Compute data for WYR - standard report
    bool            ER_Prod_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData);                     // Compute data for Enterprise Report graphs (Yield, UPH)
    bool            ER_Genealogy_YieldVsYield_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData);   // Compute data for Enterprise Report graphs (Genealogy - Yield vs Yield)
    // Compute data for Enterprise Report graphs (Genealogy - Yield vs Yield)
    bool            ER_Genealogy_YieldVsParameter_GetParts(
                        GexDbPlugin_Filter & cFilters,
                        GexDbPlugin_ER_Parts & clER_PartsData);
    bool            ER_Prod_GetBinnings(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData, GexDbPlugin_ER_Parts_Graph *pGraph, GexDbPlugin_ER_Parts_Layer *pLayer, const QString & strAggregateLabel, GexDbPlugin_BinList & clBinList);// Get bin counts for Enterprise Report graphs (Yield, UPH)
    bool            ER_Genealogy_QueryProductList(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bAllTestingStages);// Return all products (use only date in the filter)

    // Data extraction for GEX Advanced Enterprise Reports
    bool            AER_GetDataset(const GexDbPluginERDatasetSettings&, GexDbPluginERDataset&);

    bool            SPM_GetProductList(QString testingStage,
                                       QString productRegexp,
                                       QStringList & cMatchingValues);
    bool            SPM_GetFlowList(QString testingStage,
                                    QString productRegexp,
                                    QStringList & cMatchingValues);
    bool            SPM_GetInsertionList(QString testingStage,
                                         QString productRegexp,
                                         QString flowRegexp,
                                         QStringList & cMatchingValues);
    bool            SPM_GetItemsList(QString testingStage,
                                     QString productRegexp,
                                     QString flowRegexp,
                                     QString insertionRegexp,
                                     QString testType,
                                     QStringList & cMatchingValues);
    // Fetches a list of wafer_id from the TDR matching the provided criterion
    bool            SPM_FetchWaferKeysFromFilters(QString testingStage, //in
                                                  QString productRegexp, //in
                                                  QString lotId, //in
                                                  QString sublotId, //in
                                                  QString waferId, //in
                                                  const QMap<QString,QString>& filtersMetaData, //in
                                                  QStringList& waferKeyList); //out
    bool            SPM_GetConditionsFromFilters(QString testingStage, //in
                                                  const QMap<QString,QString>& filtersMetaData, //in
                                                  QMap<QString, QStringList>& filtersConditions); //out

    // Fetches the SPM datapoints from the ADR for a compute
    bool            SPM_FetchDataPointsForComputing(QString testingStage, //in
                                                    QString productRegexp, //in
                                                    QString monitoredItemType, //in
                                                    const QList<MonitoredItemRule>& monitoredItemRules, //in
                                                    MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                    QString testFlow, //in
                                                    QString consolidationType, //in
                                                    QString consolidationLevel, //in
                                                    QString testInsertion, //in
                                                    const QStringList& statsToMonitor, //in
                                                    QString siteMergeMode, //in
                                                    bool useGrossDie, //in
                                                    const QMap<QString, QStringList>& filtersConditions, //in
                                                    QDateTime dateFrom, //in
                                                    QDateTime dateTo, //in
                                                    QStringList& productsMatched, //out
                                                    int& numLotsMatched, //out
                                                    int& numDataPointsMatched, //out
                                                    QSet<int>& siteList, //out
                                                    QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& testToSiteToStatToValues); //out

    // Fetches the SPM datapoints from the ADR for a check on trigger
    bool            SPM_FetchDataPointsForCheckOnTrigger(QString testingStage, //in
                                                         QString productRegexp, //in
                                                         QString lotId, //in
                                                         QString sublotId, //in
                                                         QString waferId, //in
                                                         const QList<MonitoredItemDesc>& testList, //in
                                                         MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                         QString testFlow, //in
                                                         QString consolidationType, //in
                                                         QString consolidationLevel, //in
                                                         QString testInsertion, //in
                                                         const QList<int>& siteList, //in
                                                         const QList<QString>& statsList, //in
                                                         bool useGrossDie, //in
                                                         const QDateTime* dateFrom, //in
                                                         const QDateTime* dateTo, //in
                                                         const QMap<QString, QStringList>& filtersConditions, //in
                                                         QString& productList, //out
                                                         QString& lotList, //out
                                                         QString& sublotList, //out
                                                         QString& waferList, //out
                                                         int& numParts, //out
                                                         QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& testToSiteToStatToDataPoint); //out

    // Fetches the SPM datapoints from the TDR for a check on insertion
    bool            SPM_FetchDataPointsForCheckOnInsertion(QString testingStage, //in
                                                           int splitlotId, //in
                                                           const QMap<QString,QString> &filtersMetaData, //in
                                                           const QList<MonitoredItemDesc>& testList, //in
                                                           MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                           const QList<int>& siteList, //in
                                                           const QList<QString>& statsList, //in
                                                           QString& productList, //out
                                                           QString& lotList, //out
                                                           QString& sublotList, //out
                                                           QString& waferList, //out
                                                           int& numParts, //out
                                                           QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& testToSiteToStatToDataPoint); //out

    // Fetches the SYA datapoints from the TDR for a compute
    bool            SYA_FetchDataPointsForComputing(QString testingStage, //in
                                                    QString productRegexp, //in
                                                    const QMap<QString,QString>& filters, //in
                                                    QString monitoredItemType, //in
                                                    const QList<MonitoredItemRule>& monitoredItemRules, //in
                                                    const QStringList& binsToExclude, //in
                                                    MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                    QString testFlow, //in
                                                    QString consolidationType, //in
                                                    QString consolidationLevel, //in
                                                    QString testInsertion, //in
                                                    const QStringList &statsToMonitor, //in
                                                    QString siteMergeMode, //in
                                                    bool useGrossDie, //in
                                                    QDateTime computeFrom, //in
                                                    QDateTime computeTo, //in
                                                    QStringList& productsMatched, //out
                                                    int& numLotsMatched, //out
                                                    int& numDataPointsMatched, //out
                                                    QSet<int>& siteList, //out
                                                    QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& monitoredItemToSiteToStatToValues); //out

    // Fetches the SYA datapoints from the TDR for a check
    bool            SYA_FetchDataPointsForCheckOnTrigger(QString testingStage, //in
                                                         QString productRegexp, //in
                                                         QString lotId, //in
                                                         QString sublotId, //in
                                                         QString waferId, //in
                                                         const QMap<QString,QString> &filtersMetaData, //in
                                                         const QList<MonitoredItemDesc> &binList, //in
                                                         const QStringList& binsToExclude, //in
                                                         MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                         QString testFlow, //in
                                                         QString consolidationType, //in
                                                         QString consolidationLevel, //in
                                                         QString testInsertion, //in
                                                         const QList<int> &siteList, //in
                                                         const QList<QString>& statsList, //in
                                                         bool useGrossDie, //in
                                                         const QDateTime* dateFrom, //in
                                                         const QDateTime* dateTo, //in
                                                         QString& productList, //out
                                                         QString& lotList, //out
                                                         QString& sublotList, //out
                                                         QString& waferList, //out
                                                         int& numParts, //out
                                                         QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& binToSiteToStatToDataPoint); //out

    // Fetches the SYA datapoints from the TDR for a check
    bool            SYA_FetchDataPointsForCheckOnInsertion(QString testingStage, //in
                                                           int splitlotId, //in
                                                           const QMap<QString,QString>& filters, //in
                                                           const QList<MonitoredItemDesc> &binList, //in
                                                           const QStringList& binsToExclude, //in
                                                           MonitoredItemUniqueKeyRule uniqueKeyRule, //in
                                                           const QList<int> &siteList, //in
                                                           const QList<QString> &statsList, //in
                                                           bool useGrossDie, //in
                                                           QString& productList, //out
                                                           QString& lotList, //out
                                                           QString& sublotList, //out
                                                           QString& waferList, //out
                                                           int& numParts, //out
                                                           QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& binToSiteToStatToDataPoint); //out

    // Data transfer
    bool            TransferDataFiles(tdGexDbPluginDataFileList &cMatchingFiles, const QString & strLocalDir);          // Transfer remote data files to local FS

    // Read/Write GexDb plugin settings.
    GexDbPlugin_ID  *LoadSettings(QList<GexDbPlugin_ID*> & pPluginList, const QString &strFile);    // Read GexDb plugin settings
    bool            WriteSettings(GexDbPlugin_ID *pPluginID, const QString &strFile);               // Write GexDb plugin settings

    QString         m_strDatabasePhysicalPath;

private:
    // Functions
    void          UpdatePluginFileName(QString &strFileName);
    QString       ReadPluginFileNameFromDom(const QDomElement &node);
    QString       ReadPluginNameFromDom(const QDomElement &node);
    QString       ReadPluginBuildFromDom(const QDomElement &node);

    void          SetLastErrorFromPlugin();

    // Variables
    GexMainwindow       *m_pMainWindow;             // Ptr to  app main window
    GexDbPlugin_ID      *m_pPluginID;               // Ptr to loaded plugin
    QString             m_strPluginDirectory;       // Directory where plug-ins should be located
    long                m_lSplitlotID;              // SplitlotID for file just inserted (used for subsequent monitoring tasks to insert alarms into the DB)
    int                 m_nTestingStage;            // Testing stage for file just inserted (used for subsequent monitoring tasks to insert alarms into the DB)
public:
    bool            purgeDataBase(GexDbPlugin_Filter &roFilters, GexDbPlugin_SplitlotList &oSplitLotList);
    bool exportCSVCondition(const QString &strCSVFileName, const QMap<QString, QString>& oConditions, GexDbPlugin_Filter &roFilters, GexDbPlugin_SplitlotList &oSplitLotList, QProgressDialog *poProgress=0);
};

#endif
