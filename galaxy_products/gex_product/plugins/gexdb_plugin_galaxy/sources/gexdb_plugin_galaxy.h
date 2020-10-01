// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

#ifndef _GEXDB_PLUGIN_GALAXY_HEADER_
#define _GEXDB_PLUGIN_GALAXY_HEADER_

// Standard includes

// Qt includes

// Galaxy modules includes
#include <stdf.h>
#include <stdfparse.h>
#include <qprocess.h>
#include <QMainWindow>
#include <QProgressDialog>
#include <QThread>
#include <QFile>
#include <gex_scriptengine.h>

// Local includes
#include "gexdb_plugin_base.h"
#include "gexdb_plugin_common.h"
#include "gexdb_plugin_querybuilder.h"
#include "multi_limit_item.h"
#include "rdb_options.h"
#include "tdr_gs_version.h"
#include "HttpChannel.h"
#include "statistical_monitoring_monitored_item_unique_key_rule.h"

extern "C" GEXDB_PLUGIN_API GexDbPlugin_Base *gexdb_plugin_getobject(
        const QString & strHostName,
        const QString & strApplicationPath,
        const QString & strUserProfile,
        const QString & strLocalFolder,
        const char *gexLabelFilterChoices[],
        CGexSkin * pGexSkin,
        const GexScriptEngine*,
        const bool bCustomerDebugMode);
extern "C" GEXDB_PLUGIN_API void gexdb_plugin_releaseobject(GexDbPlugin_Base *pObject);

// Forward declarations
class ConsolidationCenter;
namespace GS
{
namespace DbPluginGalaxy
{
class RdbOptionsWidget;
}
}
struct GsData;

// Name of supported testing stages
#define GEXDB_PLUGIN_GALAXY_ETEST        "E-Test"
#define GEXDB_PLUGIN_GALAXY_WTEST        "Wafer Sort"
#define GEXDB_PLUGIN_GALAXY_FTEST        "Final Test"
#define GEXDB_PLUGIN_GALAXY_AZ           "A to Z"

// Count of Test Conditions columns available
#define GEXDB_PLUGIN_GALAXY_TEST_CONDITIONS_COLUMNS 100

// GEXDB table prefixes
#define GEXDB_PLUGIN_GALAXY_ETEST_TABLE_PREFIX    "et_"
#define GEXDB_PLUGIN_GALAXY_WTEST_TABLE_PREFIX    "wt_"
#define GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX    "ft_"

// Defines for GexDB Database
#define FLAG_SPLITLOT_TSUMMARY_OK        0x01
#define FLAG_SPLITLOT_NOTESTRESULTS      0x02
#define FLAG_SPLITLOT_PARTSFROMSAMPLES   0x04

// Updating DB steps
#define GEXDB_DB_UPDATE_DB              "UPDATING_DATABASE"
#define GEXDB_DB_UPDATE_CONS_TREE       "UPDATING_CONSOLIDATION_TREE"
#define GEXDB_DB_UPDATE_CONS_TRIGGERS   "UPDATING_CONSOLIDATION_TRIGGERS"
#define GEXDB_DB_UPDATE_CONS_TABLES     "UPDATING_CONSOLIDATION_TABLES"
#define GEXDB_DB_UPDATE_CONS_PROCEDURES "UPDATING_CONSOLIDATION_PROCEDURES"
#define GEXDB_DB_UPDATE_CONS_OLD        "UPDATING_CONSOLIDATION"
#define GEXDB_DB_UPDATE_INDEXES         "UPDATING_INDEXES"

#define FLAG_SPLITLOT_VALID              'Y'
#define FLAG_SPLITLOT_INVALID            'N'

// WAFER FLAGS
#define FLAG_WAFERINFO_GROSSDIEOVERLOADED   0x01

#define FLAG_TESTINFO_MULTIPLE_LIMIT_SET    0x01
#define FLAG_TESTINFO_LL_STRICT             0x02
#define FLAG_TESTINFO_HL_STRICT             0x04
#define FLAG_TESTINFO_PAT_ACTIVE            0x08

#define FLAG_TESTRESULT_PASS                0x01
#define FLAG_TESTRESULT_INVALID_RESULT      0x02    // PTEST/MPTEST: no Result but valid Pass/Fail
#define FLAG_TESTRESULT_NOTEXECUTED         0x04    // PTEST/MPTEST/FTEST: Result but Not Executed (not a "TEST"!)
#define FLAG_TESTRESULT_INVALID_PASSFLAG    0x08    // Pass/fail flag is invalid / no pass/fail indication
#define FLAG_TESTRESULT_ALARM               0x10    // Alarm detected during testing

#define FLAG_RUN_PARTRETESTED               0x01
#define FLAG_RUN_NOFULLTOUCHDOWN            0x02

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_Galaxy_WaferInfo
// Holds info on 1 wafer
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_Galaxy_WaferInfo
{
public:
    GexDbPlugin_Galaxy_WaferInfo()
    {
        m_fWaferSize       = 0.0F;
        m_fDieHt           = 0.0F;
        m_fDieWid          = 0.0F;
        m_uiWaferUnits     = 0;
        m_cWaferFlat       = ' ';
        m_nCenterX         = -32768;
        m_nCenterY         = -32768;
        m_cPosX            = ' ';
        m_cPosY            = ' ';
        m_uiGrossDie       = 0;
        m_uiNbParts        = 0;
        m_uiNbParts_Good   = 0;
        m_uiFlags          = 0;
        m_nWaferNb         = -1;
    }

    // Fields from Galaxy DB (et_wafer_info, wt_wafer_info)
    QString        m_strWaferID;         // WAFER_ID
    QString        m_strFabID;           // FAB_ID
    QString        m_strFrameID;         // FRAME_ID
    QString        m_strMaskID;          // MASK_ID
    float          m_fWaferSize;         // WAFER_SIZE
    float          m_fDieHt;             // DIE_HT
    float          m_fDieWid;            // DIE_WID
    unsigned int   m_uiWaferUnits;       // WAFER_UNITS
    char           m_cWaferFlat;         // WAFER_FLAT
    int            m_nCenterX;           // CENTER_X
    int            m_nCenterY;           // CENTER_Y
    char           m_cPosX;              // POS_X
    char           m_cPosY;              // POS_Y
    unsigned int   m_uiGrossDie;         // GROSS_DIE
    unsigned int   m_uiNbParts;          // NB_PARTS
    unsigned int   m_uiNbParts_Good;     // NB_PARTS_GOOD
    unsigned int   m_uiFlags;            // FLAGS
    int            m_nWaferNb;           // WAFER_NB
    QString        m_strEtestSiteConfig; // ETEST_SITE_CONFIG
};

class GexDbPlugin_Galaxy_UpdatedTables: public QStringList
{
public:
    void Add(const QString & strTableName)
    {
        if(!contains(strTableName))
            append(strTableName);
    }
};


class CTReplies;
class ConsolidationTree;
class GexDbPlugin_Galaxy_SplitlotInfo;
class GexDbPlugin_Galaxy_TestFilter;
class GexDbPlugin_Option;
class QEventLoop;

///////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GALAXY database types
///////////////////////////////////////////////////////////
class GexDbPlugin_Galaxy : public GexDbPlugin_Base
{
    /**
     * @brief Base slot type designed to be inherited and not usable outside its inheritance hierarchy. Contains states
     * to be updated when processing a specified wired signal
     */
    class FunctorSlotForNetworkSignalsBase
    {
    protected :
        /**
         * @brief m_status an external state indicating a status. Modified in derived classes' instances
         */
        bool& m_status;

        /**
         * @brief m_local_loop a local event loop in which are registered specific signals and slots, making the
         * processing of these signals and slots synchronous. This event loop is modified in derived classes' instances
         */
        QEventLoop& m_local_loop;

        /**
         * @brief FunctorSlotForNetworkSignalsBase initializes references in this class
         * @param status the reference on the external status to update later in derived classes' instances
         * @param local_loop the local event loop to use in derived classes' instances
         */
        FunctorSlotForNetworkSignalsBase( bool &status, QEventLoop &local_loop );

        /**
         * @brief something_went_wrong is a function to call in derived class to significate that something went wrong
         * when dealing with the received signal, actually a connection timeout or error or a post error lead to
         * something wrong
         */
        void something_went_wrong();

        /**
         * @brief everything_is_alright is a function to call in derived class to significate that all is good
         * when dealing with the received signal, actually a post finished lead to cool things
         */
        void everything_is_alright();

    private :
        /**
         * @brief terminate_local_loop is a private function used in this class' instance
         */
        void terminate_local_loop();
    };

    /**
     * @brief The ConnectionErrorSlot is a slot that is used when a QTcpSocket instance cannot establish a valid
     * connection.
     */
    struct ConnectionErrorSlot :
        public FunctorSlotForNetworkSignalsBase
    {
        /**
         * @brief ConnectionErrorSlot initializes parent's states using the parent's constructor
         * @param status the reference of the external state
         * @param local_loop the reference on a local event loop
         */
        ConnectionErrorSlot( bool &status, QEventLoop &local_loop );

        /**
         * @brief operator () contains the code to execute after the emission of error signal from QTcpSocket class'
         * instance. Has the same signature as the QTcpSocket::error signal
         */
        void operator ()( QTcpSocket::SocketError );
    };

    /**
     * @brief The PostErrorSlot is a slot that is used when a QNetworkAccessManager instance encounters an error during
     * a HTTP post operation.
     */
    struct PostErrorSlot :
        public FunctorSlotForNetworkSignalsBase
    {
        /**
         * @brief PostErrorSlot initializes parent's states using the parent's constructor
         * @param status the reference of the external state
         * @param local_loop the reference on a local event loop
         */
        PostErrorSlot( bool &status, QEventLoop &local_loop );

        /**
         * @brief operator () contains the code to execute after the emission of error signal from QNetworkReply
         * instance. Has the same signature as the QNetworkReply::error signal
         */
        void operator ()( QNetworkReply::NetworkError );
    };

    /**
     * @brief The PostFinishedSlot is a slot that is used when a QNetworkAccessManager instance process a successful
     * HTTP post operation.
     */
    struct PostFinishedSlot :
        public FunctorSlotForNetworkSignalsBase
    {
        /**
         * @brief PostFinishedSlot initializes parent's states using the parent's constructor
         * @param status the reference of the external state
         * @param local_loop the reference on a local event loop
         * @param response reference to an array, that will recive the response of the server
         */
        PostFinishedSlot( bool &status, QEventLoop &local_loop, QByteArray &response );

        /**
         * @brief operator () contains the code to execute after the emission of finished signal from QNetworkReply
         * instance.
         *
         * @param response the response given by the server after a finished post
         */
        void operator ()( QByteArray response );

    private :
        /**
         * @brief m_response is the response given by the server
         */
        QByteArray &m_response;
    };

    /**
     * @brief The GetErrorSlot is a slot that is used when a QNetworkAccessManager instance encounters an error during
     * a HTTP get operation.
     */
    struct GetErrorSlot :
        public FunctorSlotForNetworkSignalsBase
    {
        /**
         * @brief GetErrorSlot initializes parent's states using the parent's constructor
         * @param status the reference of the external state
         * @param local_loop the reference on a local event loop
         */
        GetErrorSlot( bool &status, QEventLoop &local_loop );

        /**
         * @brief operator () contains the code to execute after the emission of error signal from QNetworkReply
         * instance. Has the same signature as the QNetworkReply::error signal
         */
        void operator ()( QNetworkReply::NetworkError );
    };

    /**
     * @brief The GetFinishedSlot is a slot that is used when a QNetworkAccessManager instance process a successful
     * HTTP get operation.
     */
    struct GetFinishedSlot :
        public FunctorSlotForNetworkSignalsBase
    {
        /**
         * @brief GetFinishedSlot initializes parent's states using the parent's constructor
         * @param status the reference of the external state
         * @param local_loop the reference on a local event loop
         */
        GetFinishedSlot( bool &status, QByteArray& response, QEventLoop &local_loop );

        /**
         * @brief operator () contains the code to execute after the emission of finished signal from QNetworkReply
         * instance. Has the same signature as the QNetworkReply::finished signal
         */
        void operator ()(QByteArray response);

    private:
        /**
         * @brief m_response the result of the http GET request.
         */
        QByteArray& m_response;
    };

    /**
     * @brief The ConnectionTimeout is a slot that is used when a QTimer instance reaches its time limit, indicating a
     * connection timeout
     */
    struct ConnectionTimeoutSlot :
        public FunctorSlotForNetworkSignalsBase
    {
        /**
         * @brief ConnectionTimeoutSlot initializes parent's states using the parent's constructor
         * @param status the reference of the external state
         * @param local_loop the reference on a local event loop
         */
        ConnectionTimeoutSlot( bool &status, QEventLoop &local_loop );

        /**
         * @brief operator () contains the code to execute after the emission of finished signal from QTimer
         * instance. Has the same signature as the QTimer::timeout signal
         */
        void operator ()();
    };

    Q_OBJECT

    long mNumOfRecordsRead; // number of records read in InsertStdfFile() function
public slots:
    // called to refresh InsertStdfFile function progress called by an internal QTimer
    void InsertStdfFileRefresh();
    // called to request to stop process if possible
    void StopProcess();

    // Tells if the DB of this plugin has die trace data (FT tables)
    bool HasDieTraceData();
    void         LoadRecordedTdrTypeIfEmpty();
    //! \brief Is a charaterization TDR ?
    bool         IsCharacTdr();
    //! \brief ?
    bool         IsManualProdTdr();
    //! \brief ?
    bool         IsYmProdTdr();
    //! \brief ?
    bool         IsAdr();
    //! \brief ?
    bool         IsLocalAdr();
    //! \brief is insertion supported (some TDR plugins are extraction only)
    bool         IsInsertionSupported() { return true; }
    //! \brief ?
    bool         IsUpdateSupported() { return true; }
    //! \brief ?
    bool         IsTestingStagesSupported() { return true; }
    //! \brief ?
    bool         IsParameterSelectionSupported() { return true; }
    //! \brief ?
    bool         IsEnterpriseReportsSupported() { return true; }
    //! \brief ?
    bool         IsReportsCenterSupported();

public:

    enum    DataBaseType
    {
        eUnknown        = -1,
        eYmAdminDb      = 0,
        eCharacTdrDb    = 1,
        eManualProdDb   = 2,
        eProdTdrDb      = 3,
        eAdrDb          = 4,
        eAdrLocalDb     = 5
    };

    GexDbPlugin_Galaxy(const QString & strHostName,
                       const QString & strApplicationPath,
                       const QString & strUserProfile,
                       const QString & strLocalFolder,
                       const char *gexLabelFilterChoices[],
                       const bool bCustomerDebugMode,
                       CGexSkin * pGexSkin,
                       const GexScriptEngine *gse,
                       GexDbPlugin_Connector *pclDatabaseConnector=NULL);

    virtual ~GexDbPlugin_Galaxy();

    // Member functions
public:

    QString GetCreationScriptName(const DataBaseType&type);
    QString GetUpdateScriptName(const DataBaseType&type);
    // Common functions
    bool         Init();
    bool         InitCore();
    bool         InitGui();
    void         GetPluginName(QString & strPluginName) const { strPluginName = GEXDB_PLUGIN_GALAXY_NAME; }
    Q_INVOKABLE unsigned int GetPluginBuild(void) { return GEXDB_PLUGIN_GALAXY_BUILD; }
    void         GetSupportedTestingStages(QString & strSupportedTestingStages);
    void         GetTdrTypeName(QString & strTdrType);
    bool         SetTestingStage(const QString & strTestingStage);
    bool         IsTestingStage_FinalTest(const QString & strTestingStage);
    bool         IsTestingStage_Foundry(const QString & strTestingStage);
    bool         GetTestingStageName_Foundry(QString & strTestingStage);
    void         GetLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);
    void         GetConsolidatedLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);
    void         GetLabelFilterChoicesWaferLevel(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);
    void         GetLabelFilterChoicesLotLevel(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);
    // Returns a pointer on an options GUI
    bool         InitRdbOptionsWidget();
    QWidget*     GetRdbOptionsWidget();
    bool         InitConsolidationWidget();
    QWidget*     GetConsolidationWidget();
    // Returns an options string, if the plugin supports custom options
    bool         GetRdbOptionsString(QString & strRdBOptionsString);
    // Sets the plug-in options GUI according to options string passed as argument
    bool         SetRdbOptionsString(const QString & strRdBOptionsString);

    // Get the (raw) bin counts per site for the given splitlot.
    QString      GetSplitlotBinCountsPerSite(const QString &TestingStage, long lSplitlotID,
                                             QMap<int, GexDbPlugin_BinList> &mapBinsPerSite);
    // Get the (raw) bin counts per group by for the given filter.
    QString      GetBinCounts(GexDbPlugin_Filter & cFilters,
                              QStringList lGroupBy,
                              bool bSoftBin,
                              QMap<QString, GexDbPlugin_BinList> &mapBinsPerKey);

    // Return supported binning types.
    // ConsolidatedType: "Y" or "N"
    bool         GetSupportedBinTypes(const QString &TestingStage, QStringList & strlBinTypes, const QString &ConsolidatedType="Y");

    // Retrieve list of available fields for the given testing stage. output will contains the matching fields.
    void         GetRdbFieldsList(    const QString &TestingStage, QStringList &output,
                                      const QString &In_GUI="Y",             // Y, N or *
                                      const QString &BinType="N",            // BinType can be H, S or N (or even *)
                                      const QString &Custom="*",             // Custom can be Y, N or *
                                      const QString &TimeType="*",           // TimeType can be Y, N or *
                                      const QString &ConsolidatedType="Y",   // ConsolidatedType can be Y, N or *
                                      const QString &Facts="N",              // Facts can be Y, N, or *
                                      bool  OnlyStaticMetaData=false
                                                           );
    bool        GetRdbFieldProperties(const QString& TestingStage, const QString& MetaDataName, QMap<QString, QString> &Properties);
    // Get consolidated field name corresponding to specified decorated field name
    bool         GetConsolidatedFieldName(const QString & strTestingStage,
                                          const QString & strDecoratedField,
                                          QString & strConsolidatedField,
                                          bool *pbIsNumeric = NULL,
                                          bool * pbIsBinning = NULL,
                                          bool * pbIsTime = NULL);

    void         GetWaferIdFieldName(QString & strFieldName);
    void         GetLotIdFieldName(QString & strFieldName);

    // Database administration functions
    bool         IsDbUpToDate(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name,
                              unsigned int *puiCurrentDbVersion_Build,
                              QString & strLatestSupportedDbVersion_Name,
                              unsigned int *puiLatestSupportedDbVersion_Build);
    bool         IsDbUpToDateForInsertion(bool *pbDbIsUpToDate,
                                          QString & strCurrentDbVersion_Name,
                                          unsigned int *puiCurrentDbVersion_Build,
                                          QString & strLatestSupportedDbVersion_Name,
                                          unsigned int *puiLatestSupportedDbVersion_Build);
    bool         IsDbUpToDateForExtraction(bool *pbDbIsUpToDate,
                                           QString & strCurrentDbVersion_Name,
                                           unsigned int *puiCurrentDbVersion_Build,
                                           QString & strLatestSupportedDbVersion_Name,
                                           unsigned int *puiLatestSupportedDbVersion_Build);
    bool         IsCompatibleForStdExtraction(unsigned int uiGexDbBuild,
                                              unsigned int uiBuildSupportedByPlugin);
    bool         IsSecuredDb(bool *pbDbSecured);
    bool         UpdateDbSecuredMode(bool bSecured);
    // update the DB folowing the 'command'. If command empty, simply upgrade to the next version.
    bool         UpdateDb(QString command="");

    // Incremental Update methods
    // flag specified splitlots ready to be flagged for a incremental update
    bool FlagSplitlotsForIncrementalUpdate( const QStringList &splitlots, const QString &incrementalKey);
    bool UnFlagSplitlotsForIncrementalUpdate( const QStringList &splitlots, const QString &incrementalKey);
    bool SwitchFlagSplitlotsForIncrementalUpdate( const QStringList &splitlots, const QString &incrementalOldKey, const QString &incrementalNewKey);

    // Run a specific incremental update
    bool         IncrementalUpdate(QString incrementalName, QString testingStage, QString target, QMap< QString, QString >  &summary);
    // Get the next incremental update for Schedule for ALL according to the Frequency
    bool         GetNextAutomaticIncrementalUpdatesList(QMap< QString,QMap< QString,QStringList > > & incrementalUpdatesList);
    // Get the next incremental update for Schedule for ALL or specific for manual update
    bool         GetFirstIncrementalUpdatesList(QString incrementalName, QMap< QString,QMap< QString,QStringList > > & incrementalUpdatesList);
    // Get/Update the total count of remaining splitlots for incremental update
    bool         GetIncrementalUpdatesCount(bool checkDatabase,int &incrementalSplitlots);
    // Incremental updates settings
    bool         GetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > &incrementalUpdates);
    bool         SetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > &incrementalUpdates);
    bool         IsIncrementalUpdatesSettingsValidValue(QString &name, QString &value);

    bool                mBackgroundTransferActivated;
    int                 mBackgroundTransferInProgress;
    QMap<QString, long> mBackgroundTransferCurrentIndex;
    bool                BackgroundTransferLazyMode(QString aTableName, long aSplitlotID);
    bool                BackgroundTransferExcludeForIncremental(QString aPrefixTable, QString aTarget, QString &aExcludeClause);

    int IsConsolidationInProgress(
            QString testingStage,
            QString lot,
            QString sublots,
            QString wafers,
            QString consoType,
            QString testFlow,
            QString consoLevel,
            QString testInsertion);

    // eTestingStage: 0 : to update all testingstage else update only one testing stage
    bool         UpdateConsolidationProcess(int eTestingStage=0);
    bool         UpdateConsolidatedTableStartTimeFields(const QString &consolidatedTable,
                                                        int eTestingStage,
                                                        const QString &startDateConsolidationRule);
    bool         UpdateAllConsolidatedTableStartTimeFields(int eTestingStage,
                                                           const QString& startDateConsolidationRule);
    bool         IsAutomaticIncrementalUpdatesEnabled() { return m_bEnableAutomaticIncrementalUpdates; }
    void         InsertIntoUpdateLog(const QString & strMessage, bool bAddCR=true, bool bPlainText=false);
    // Reset the database (on SQLite, overwrite DB file gexdb.sqlite)
    bool         Reset(const QString &strDB_Folder);
    // Purge selected splitlots
    bool         PurgeSplitlots(QStringList & strlSplitlots, QString & strTestingStage, QString & strCaseTitle,
                                QString *pstrLog=NULL);
    // Consolidate specified wafer
    bool         ConsolidateWafer(QString & strLotID, QString & strWaferID, QString & strCaseTitle,
                                  QString *pstrLog=NULL);
    // Consolidate specified wafer
    bool         ConsolidateWafers(QString & strLotID, QString & strCaseTitle,
                                   QString *pstrLog=NULL);
    bool         ConsolidateLot(QString & strLotID, bool bConsolidateOnlySBinTable=false, bool bCallConsolidationFunction=true);
    bool         GetGlobalOptionName(int nOptionNb, QString &strValue);
    bool         GetGlobalOptionTypeValue(int nOptionNb, QString &strValue);
    bool         GetGlobalOptionValue(int nOptionNb, QString &strValue);
    bool         GetGlobalOptionValue(int nOptionNb, QString &strValue, bool &bIsDefined);
    bool         GetGlobalOptionDefaultValue(int nOptionNb, QString &strValue);
    bool         GetGlobalOptionDescription(int nOptionNb, QString &strValue);
    bool         GetGlobalOptionReadOnly(int nOptionNb, bool &bIsReadOnly);
    bool         IsGlobalOptionValidValue(int nOptionNb, QString &strValue);
    // Retrieve an option value simply from his name
    bool         GetGlobalOptionValue(QString strOptionName, QString &strValue);
    // Save an option value simply from his name
    bool         SetGlobalOptionValue(QString strOptionName, QString &strValue);
    bool         SetGlobalOptionValue(int nOptionNb, QString &strValue);

    // Get name of storage engine
    bool         GetStorageEngineName(QString & strStorageEngine,QString & strStorageFormat);
    // get current total size of the DB
    bool         GetTotalSize(int &size);

    // Database restriction security
    bool         GetSecuredMode(QString &strSecuredMode);
    bool         UpdateSecuredMode(QString strSecuredMode);


    // Plugin configuration
    bool         ConfigWizard();
    bool         ConnectToCorporateDb();
    bool         LoadStartupTypeFromDom(const QDomElement &node);
    // Load settings from dom elt
    bool         LoadSettingsFromDom(const QDomElement &node);
    // Load settings from file
    bool         LoadSettings(QFile *pSettingsFile);
    // Load meta-data (mapping + links)
    bool         LoadMetaData();
    // Load meta-data (mapping + links)
    bool         LoadMetaData(unsigned int uiDbVersion_Build);
    // Load static meta-data (mapping + links)
    void         LoadStaticMetaData();
    // Load static meta-data (mapping + links)
    void         LoadStaticMetaData(unsigned int uiDbVersion_Build);
    // Load dynamic meta-data from meta-data tables (mapping + links)
    bool         LoadDynamicMetaData();
    // Load dynamic meta-data from meta-data tables (mapping + links)
    bool         LoadDynamicMetaData(unsigned int uiDbVersion_Build);
    QDomElement  GetStartupTypeDom(QDomDocument &doc);
    QDomElement  GetSettingsDom(QDomDocument &doc);
    // Write settings to file using informations loaded in the class variables
    bool         WriteSettings(QTextStream *phFile);
    // Retrieve list of insertion warnings
    void         GetWarnings(QStringList & strlWarnings);
    bool         SetUpdateDbLogFile(QString strLogFileName);
    bool         GetUpdateDbLogFileName(QString&strLogFileName)    {strLogFileName = m_strUpdateDbLogFile; return true;}

    /*!
     * \fn InsertDataFile
     */
    virtual bool InsertDataFile(struct GsData* lGsData,
                                int lSqliteSplitlotId,
                                const QString& strDataFileName,
                                GS::QtLib::DatakeysEngine& dbKeysEngine,
                                bool* pbDelayInsertion,
                                long* plSplitlotID,
                                int* pnTestingStage);
    /*!
     * \fn InsertDataFileSqlite
     */
    bool InsertDataFileSqlite(struct GsData* lGsData,
                              int lSqliteSplitlotId,
                              const QString& strDataFileName);
    /*!
     * \fn InsertDataFromJson
     */
    bool InsertDataFromJson(GexDbPlugin_Query& dbQuery,
                            const char* json,
                            const char* table,
                            unsigned int tdrSplitlotId,
                            unsigned int gtlSplitlotId);
    /*!
     * \fn CheckFTRunFromJson
     */
    bool CheckFTRunFromJson(GexDbPlugin_Query& dbQuery,
                            const char* json,
                            unsigned int tdrSplitlotId);
    /*!
     * \fn CheckFtTestInfoFromJson
     */
    QMap<int,int>* CheckFtTestInfoFromJson(GexDbPlugin_Query& dbQuery,
                                           const char* json,
                                           const char* tablePrefix,
                                           unsigned int tdrSplitlotId);
    /*!
     * \fn CheckFtBinFromJson
     */
    bool CheckFtBinFromJson(GexDbPlugin_Query& dbQuery,
                            const char* json,
                            const char* tablePrefix,
                            unsigned int tdrSplitlotId);
    /*!
     * \fn InsertGlobalFileFromJson
     */
    bool InsertGlobalFileFromJson(GexDbPlugin_Query& dbQuery,
                                  std::string& fileInfos,
                                  const char* json,
                                  unsigned int tdrSplitlotId);
    /*!
     * \fn GetTDRSplitlotFromSP
     */
    int GetTDRSplitlotFromSP(GexDbPlugin_Query& dbQuery,
                             unsigned int sqliteSplitlotId,
                             const QString& strDataFileName);
    /*!
     * \fn DeleteGtlSplitlot
     */
    int DeleteGtlSplitlot(GexDbPlugin_Query& dbQuery,
                          unsigned int tdrSplitlotId);
    /*!
     * \fn InsertGtlSplitlot
     */
    int InsertGtlSplitlot(GexDbPlugin_Query& dbQuery,
                          struct GsGtlTraceability* tr,
                          unsigned int sqliteSplitlotId,
                          unsigned int tdrSplitlotId);
    /*!
     * \fn ValidGtlSplitlot
     */
    int ValidGtlSplitlot(GexDbPlugin_Query& dbQuery,
                         unsigned int tdrSplitlotId,
                         unsigned int sqliteSplitlotId);
    /*!
     * \fn UpdateEventIdAndRunId
     */
    struct GsBuffer* UpdateEventIdAndRunId(struct GsBuffer* src,
                                           unsigned int tdrSplitlotId,
                                           const std::string& lFtRunJson);
    /*!
     * \fn UpdateWithTdrRunId
     */
    struct GsBuffer* UpdateWithTdrRunId(struct GsBuffer* src,
                                        unsigned int tdrSplitlotId,
                                        const std::string& lFtRunJson,
                                        const std::string& fieldName);
    /*!
     * \fn GetTdrRunId
     */
    int GetTdrRunId(unsigned int tdrSplitlotId,
                    int sqliteRunId,
                    const std::string& lFtRunJson);
    /*!
     * \fn UpdateTestId
     */
    struct GsBuffer* UpdateTestId(struct GsBuffer* src,
                                  QMap<int,int>* testId);
    /*!
     * \fn InsertWyrDataFile
     */
    bool         InsertWyrDataFile(const QString & strDataFileName, const QString & strSiteName,
                                   const QString & strTestingStage, unsigned int uiWeekNb, unsigned int uiYear,
                                   bool *pbDelayInsertion);
    bool         InsertAlarm(long lSplitlotID, int nTestingStage, GexDbPlugin_Base::AlarmCategories eAlarmCat,
                             GexDbPlugin_Base::AlarmLevels eAlarmLevel, long lItemNumber, QString & strItemName,
                             unsigned int uiFlags, float fLCL, float fUCL, float fValue, QString strUnits);
    bool         InsertAlarm_Wafer(long lSplitlotID, GexDbPlugin_Base::AlarmCategories eAlarmCat,
                                   GexDbPlugin_Base::AlarmLevels eAlarmLevel, long lItemNumber, QString & strItemName,
                                   unsigned int uiFlags, float fLCL, float fUCL, float fValue, QString strUnits);

    // Generic data extraction functions
    // Execute Query and return results
    bool         QuerySQL(QString & strQuery, QList<QStringList> & listResults);
    // Return all splitlots (given filters on several fields)
    bool         QuerySplitlots(GexDbPlugin_Filter & cFilters,
                                GexDbPlugin_SplitlotList & clSplitlotList,
                                bool bPurge=false);
    // Return all binnings in a list, given filters on other fields
    bool         QueryBinlist(GexDbPlugin_Filter & cFilters,
                              QStringList & cMatchingValues,bool bSoftBin=false,
                              bool bClearQueryFirst=true,
                              bool bIncludeBinName=false, bool bProdDataOnly=false);
    // Return all binnings in a list, given filters on other fields (consolidated mode)
    bool         QueryBinlist_Consolidated(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,
                                           bool bSoftBin=false,bool bClearQueryFirst=true,bool bIncludeBinName=false);
    // Return all products (use only date in the filter)
    bool         QueryProductList(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,QString strProductName="");
    // Return all products for genealogy reports, with data for at least 2 testing stages (use only date in the filter)
    bool         QueryProductList_Genealogy(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bAllTestingStages);

    // Data extraction for GEX standard reports
    // Return all valid values for a field (given filters on other fields)
    bool         QueryField(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bSoftBin=false,
                            bool bDistinct=true, QuerySort eQuerySort=eQuerySort_Asc);
    // Return all valid values for a field (given filters on other fields)
    bool         QueryField_Consolidated(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,
                                         bool bSoftBin=false,bool bDistinct=true,
                                         QuerySort eQuerySort=eQuerySort_Asc);
    // Return the query to get all valid values for a field (given filters on other fields) from intermediate consolidation table or physical
    bool         BuildQueryField_Consolidated(GexDbPlugin_Filter& cFilters, QString &strQuery,
                                              bool bSoftBin=false, bool bQueryOnIntermediateTable=false);
    // Return all tests (given filters on other fields)
    bool         QueryTestlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues, bool bParametricOnly);
    // Return all valid Data files (given filters on several fields)
    bool         QueryDataFiles(GexDbPlugin_Filter & cFilters, const QString & strTestlist,
                                tdGexDbPluginDataFileList & cMatchingFiles, const QString & strDatabasePhysicalPath,
                                const QString & strLocalDir, bool *pbFilesCreatedInFinalLocation,
                                GexDbPlugin_Base::StatsSource eStatsSource);

    // Extracts and returns list of Data files given filters on several fields for etest per die granularity extraction mode
    QString QueryDataFilesForETestPerDie(GexDbPlugin_Filter & cFilters,
                                         GexDbPlugin_Galaxy_TestFilter & clTestFilter,
                                         tdGexDbPluginDataFileList & cMatchingFiles,
                                         const QString & strDatabasePhysicalPath,
                                         const QString & strLocalDir,
                                         bool *pbFilesCreatedInFinalLocation,
                                         GexDbPlugin_Base::StatsSource eStatsSource);

    // Extracts FT data using die trace data grouping by wafer
    QString QueryDataFilesForFTwithDieTrace(
            GexDbPlugin_Filter & cFilters,
            GexDbPlugin_Galaxy_TestFilter & TestFilter,
            tdGexDbPluginDataFileList & MatchingFiles,
            const QString & DatabasePhysicalPath,
            const QString & LocalDir,
            bool *lFilesCreatedInFinalLocation,
            GexDbPlugin_Base::StatsSource eStatsSource,
            bool &hasResult);

    // Get the list of distinct ProductLotWafers for given filters
    // Will look at ft_dietrace_config
    QString GetListOfDieTracedProductLotWafers(GexDbPlugin_Filter & cFilters,
                                               QList< QMap <QString, QVariant> > &list);

    // Get list of FT splitlots containing at least 1 die of given PLW
    QString GetListOfFTSplitlotsContainingWSProductLotWafer(
            const QMap<QString,QVariant> &PLW,
            GexDbPlugin_Filter & cFilters,
            QList<GexDbPlugin_Galaxy_SplitlotInfo> &list);

    // Extract given Die Traced ProductLotWafer
    QString ExtractDieTracedPLW(const QMap<QString,QVariant> &PLW,
                                GexDbPlugin_Filter &cFilters,
                                GexDbPlugin_Galaxy_TestFilter &lTestFilter,
                                tdGexDbPluginDataFileList &lMatchingFiles,
                                const QString &lLocalDir);

    // Extract splitlot using given filters and write runs and testresults to given file
    QString ExtractSplitlotIntoFile(const GexDbPlugin_SplitlotInfo &lSplitlotInfo,
                                    const QMap<QString,QVariant> &PLW,
                                    GexDbPlugin_Filter &lFilters,
                                    GexDbPlugin_Galaxy_TestFilter &lTestFilter,
                                    GQTL_STDF::StdfParse &lStdfParse);

    // Data extraction for GEX enterprise reports
    // Return all binnings in a string, given filters on other fields, and corresponding to specified bin type ("all", "pass", "fail")
    bool QueryBinlist(GexDbPlugin_Filter & cFilters,QString & strBinlist,const QString & strBintype,
                      bool bSoftBin=false,bool bClearQueryFirst=true,bool bIncludeBinName=false,
                      bool bProdDataOnly=false);
    // Compute data for Production - UPH graph
    bool         GetDataForProd_UPH(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList);
    // Compute data for Production - Yield graph
    bool         GetDataForProd_Yield(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList,
                                      int nBinning, bool bSoftBin);
    // Compute data for Production - Consolidated Yield graph
    bool         GetDataForProd_ConsolidatedYield(GexDbPlugin_Filter & cFilters,
                                                  GexDbPlugin_XYChartList & clXYChartList);
    // Compute data for WYR - standard report
    bool         GetDataForWyr_Standard(GexDbPlugin_Filter & cFilters, GexDbPlugin_WyrData & cWyrData);
    bool         Wyr_DataRow_Getvalue(QString & strValue, GexDbPlugin_WyrFormatColumn *pWyrFormatColumn,
                                      GexDbPlugin_Query & clGexDbQuery);
    // Compute data for Enterprise Report graphs (Yield, UPH)
    bool         ER_Prod_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData);
    // Compute data for Enterprise Report graphs (Genealogy - Yield vs Yield)
    bool         ER_Genealogy_YieldVsYield_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData);
    // Compute data for Enterprise Report graphs (Genealogy - Yield vs Parameter)
    bool         ER_Genealogy_YieldVsParameter_GetParts(
                        GexDbPlugin_Filter & cFilters,
                        GexDbPlugin_ER_Parts & clER_PartsData);
    // Get bin counts for Enterprise Report graphs (Yield, UPH)
    bool         ER_Prod_GetBinnings(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData,
                                     GexDbPlugin_ER_Parts_Graph *pGraph, GexDbPlugin_ER_Parts_Layer *pLayer,
                                     const QString & strAggregateLabel, GexDbPlugin_BinList & clBinList);

    // Data extraction for GEX Advanced Enterprise Reports
    bool         AER_GetDataset(const GexDbPluginERDatasetSettings& datasetSettings, GexDbPluginERDataset& datasetResult);

    // Data extraction for SYA (SYL/SBL)
    // Get bin counts and yield per lot or wafer
    bool         SYA_GetBinnings(GexDbPlugin_Filter & cFilters, class GexDbPlugin_ListBinYieldStats* listBinYieldStats,
                                 QString & strProductName, const QHash<QPair<QString, QString>, int> &lExcludedBinCount,
                                 class GexDbPlugin_BinYieldStats *pclBinYieldStats);
    // Get PASS yield per lot or wafer
    bool         SYA_GetYield(GexDbPlugin_Filter & cFilters, class GexDbPlugin_ListBinYieldStats* listBinYieldStats,
                              QString & strProductName, const QHash<QPair<QString, QString>, int> &lExcludedBinCount);
    // Get Excluded Bins parts count per lot or wafer
    bool        GetSYAExcludedBinParts(GexDbPlugin_Filter & cFilters,
                                       QHash<QPair<QString, QString>, int> &lExcludedBinCount,
                                       const QString &strProductName);
    ////////////////////////////////////////////////////////
    // Statistical monitoring related members and methods //
    ////////////////////////////////////////////////////////

public:
    // Fetches a list of wafer_id from the TDR matching the provided criterion
    void            SPM_SetQueryError(GexDbPlugin_Query &lQuery);
    void            SPM_GetOwnerVersion(QStringList &lOwnerVersion);
    bool            SPM_GetMetadatasList(QString metaName,
                                         QString testingStage,
                                         QString productRegexp,
                                         QString flowRegexp,
                                         QStringList & cMatchingValues);
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
    bool            SPM_GetConditionsFromFilters(QString testingStage, //in
                                                 const QMap<QString,QString>& filtersMetaData, //in
                                                 QMap<QString,QStringList>& filtersConditions); //out

    bool            SPM_FetchWaferKeysFromFilters(QString testingStage, //in
                                                  QString productRegexp, //in
                                                  QString lotId, //in
                                                  QString sublotId, //in
                                                  QString waferId, //in
                                                  const QMap<QString, QString> &filtersMetaData, //in
                                                  QStringList& waferKeyList); //out

    // Fetches the SPM datapoints from the ADR
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
                                                         MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                         QString testFlow, //in
                                                         QString consolidationType, //in
                                                         QString consolidationLevel, //in
                                                         QString testInsertion, //in
                                                         const QList<int>& siteList, //in
                                                         const QList<QString> &statsList, //in
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
                                                           const QList<MonitoredItemDesc> &testList, //in
                                                           MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                           const QList<int> &siteList, //in
                                                           const QList<QString> &statsList, //in
                                                           QString& productList, //out
                                                           QString& lotList, //out
                                                           QString& sublotList, //out
                                                           QString& waferList, //out
                                                           int& numParts, //out
                                                           QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& testToSiteToStatToDataPoint); //out

    // Fetches the SYA datapoints from the TDR for a compute
    bool            SYA_FetchDataPointsForComputing(QString testingStage, //in
                                                    QString productRegexp, //in
                                                    const QMap<QString,QString> &filtersMetaData, //in
                                                    QString monitoredItemType, //in
                                                    const QList<MonitoredItemRule> &monitoredItemRules, //in
                                                    const QStringList& binsToExclude, //in
                                                    MonitoredItemUniqueKeyRule uniqueKeyRule,
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
                                                    QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& binToSiteToStatToValues); //out

    // Fetches the SYA datapoints from the TDR for a check
    bool            SYA_FetchDataPointsForCheckOnTrigger(QString testingStage, //in
                                                         QString productRegexp, //in
                                                         QString lotId, //in
                                                         QString sublotId, //in
                                                         QString waferId, //in
                                                         const QMap<QString,QString> &filtersMetaData, //in
                                                         const QList<MonitoredItemDesc> &binList, //in
                                                         const QStringList& binsToExclude, //in
                                                         MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                         QString testFlow, //in
                                                         QString consolidationType, //in
                                                         QString consolidationLevel, //in
                                                         QString testInsertion, //in
                                                         const QList<int> &siteList, //in
                                                         const QList<QString> &statsList, //in
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
                                                           const QMap<QString,QString> &filtersMetaData, //in
                                                           const QList<MonitoredItemDesc> &binList, //in
                                                           const QStringList& binsToExclude, //in
                                                           MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                           const QList<int> &siteList, //in
                                                           const QList<QString> &statsList, //in
                                                           bool useGrossDie, //in
                                                           QString& productList, //out
                                                           QString& lotList, //out
                                                           QString& sublotList, //out
                                                           QString& waferList, //out
                                                           int& numParts, //out
                                                           QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& binToSiteToStatToDataPoint); //out

    ///////////////////////////////////////////////////////////////
    // end of statistical monitoring related members and methods //
    ///////////////////////////////////////////////////////////////

    // Retrieve the numerical value of the testing stage corresponding to the string value of the testing stage
    int          GetTestingStageEnum(const QString & strTestingStage);

    // Retrieve the consolidation tree stored in DB or the default one if none is found
    // Return true on success
    // CTReplies contain the details error/warning
    bool         GetConsolidationTree(QString &s, CTReplies &r);

    // Validate the given tree in xml format
    // Returns true on success, false on error
    // The detail of error/warning is in CTReplies
    bool         ValidateConsolidationTree(const QString &xml_string, CTReplies &r);

    // Try to send the given xml consolidation tree
    // Returns true on success, false on error
    // The detail of error/warning is in CTReplies
    // On success, the tree will be saved in DB then a pass will be done to check if some data has to be reconsolidated.
    bool         SendConsolidationTree(const QString &xml_string, CTReplies &r);

    // Insert the default xml consolidation file into the DB
    bool         InsertDefaultConsolidationTree( );

    // Tdr Logincal Name reference
    void          SetTdrLinkName(const QString& link) {mTdrLinkName = link;}
    QString       GetTdrLinkName() {return mTdrLinkName;}

    // Implementation
    bool          MustHaveAdrLink() { return (IsYmProdTdr() || !mAdrLinkName.isEmpty());}
    QDomElement   GetAdrLinkDom(QDomDocument &doc);
    bool          LoadAdrNameFromDOM(const QDomElement &node);

    // Link the AdrLinkName to the TDR Prod
    void          SetAdrLinkName(const QString& link) {mAdrLinkName = link;}
    QString       GetAdrLinkName() {return ((mAdrLinkName.contains("@"))?mAdrLinkName:QString());}

    bool CreateDatabase(const GexDbPlugin_Connector settingConnector, const DataBaseType &dbType,
                        const QString &rootName, const QString &rootPwd,
                        QStringList &lstUninstall,
                        QProgressBar *progressBar = NULL,
                        bool lActiveKeepComment= false);
    // Create or update user for DatabaseType
    bool CreateUpdateDatabaseUsers(const GexDbPlugin_Connector settingConnector, const QString rootConnection,
                                   const DataBaseType &dbType,
                                   QStringList &lstUninstall, QString &errorMessage);

private:
    struct JobDefinition
    {
        QString lotId;
        QString sublotId;
        QString waferId;
        QString testFlow;
        QString testInsertion;
    };

    GS::Gex::HttpChannel* mHttpChannel;

    GS::Gex::HttpChannel* CreateHttpChannel(const std::string &address,
                                            unsigned short port,
                                            const std::string &route,
                                            QObject *parent = NULL);

protected:

    enum TestingStage
    {
        // DO NOT CHANGE THE ASSOCIATED NUMBERS!!
        eUnknownStage   = 0,    // Unknown
        eFinalTest      = 1,    // For stdf final test
        eWaferTest      = 2,    // For stdf wafer test
        eElectTest      = 3     // For stdf electronic test
    };

    enum DbUpdateStep
    {
        eUpdateUnknown        = 0x001,
        eUpdateDb             = 0x002,
        eUpdateConsTree       = 0x004,
        eUpdateConsTriggers   = 0x008,
        eUpdateConsTables     = 0x010,
        eUpdateConsProcedures = 0x020,
        eUpdateIndexes        = 0x040,
        eUpdateConsOld        = 0x100
    };

    enum TestlistConditionTarget
    {
        eOnTestInfo,
        eOnTestResults
    };

    QStringList mListHBinNUmber;
    QStringList mListSBinNUmber;
    inline bool isInTheHBinSBinListNumber(const QString& number);

    ConsolidationTree * m_pConsolidationTree;

    // Set to true if automatic increamental updates should be enabled.
    bool    m_bEnableAutomaticIncrementalUpdates;
    // eFinalTest, eWaferTest or eElectTest found directly from the SDTF file
    int     m_eTestingStage;
    // Time used to measure execution time from start of InsertDataFile function
    QTime   m_clInsertDataFileTime;
    // Time used to measure execution time from start of InsertStdfFile function
    QTime   m_clInsertStdfFileTime;

    bool    LoadDatabaseArchitecture();

    // Function to call the qApplication->processEvents function (make sure the qApplication->processEvent function is not called too often)
    void    ProcessEvents();

    // Data extraction for GEX Advanced Enterprise Reports
    // Build Consolidated query for Advanced  Enterprise Report
    bool    AER_buildQuery(const GexDbPluginERDatasetSettings& datasetSettings,
                           GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery, bool bOverall);
    // Build Consolidated sub-query for Advanced  Enterprise Report
    bool    AER_buildSubQuery(const GexDbPluginERDatasetSettings& datasetSettings,
                              GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery, bool bOverall, bool bIntermediate);
    // Build query for Advanced  Enterprise Report
    bool    AER_fillQueryField(const GexDbPluginERDatasetSettings& datasetSettings,
                               GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery, bool bOverall,
                               const GexDbPluginSQL::GexDbSQLTable& mainTable,
                               const GexDbPluginSQL::GexDbSQLTable& binTable);
    // Build query for Advanced  Enterprise Report (find all aggregate for the period)
    bool    AER_buildFullTimeLineQuery(const GexDbPluginERDatasetSettings& datasetSettings,
                                       GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery, bool& bFullTimeLineCompatible);
    // Build sub-query for Advanced  Enterprise Report (find all aggregate for the period)
    bool    AER_buildFullTimeLineSubQuery(const GexDbPluginERDatasetSettings& datasetSettings,
                                          GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery,
                                          bool& bFullTimeLineCompatible, bool bIntermediate);
    // Execute query for Enterprise Report
    bool    AER_executeQuery(const GexDbPluginERDatasetSettings &datasetSettings,
                             const GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery,
                             GexDbPluginERDataset& datasetResult, bool bOverall);
    // Execute query for Enterprise Report ( get all aggregate for the period)
    bool    AER_executeFullTimeLineQuery(const GexDbPluginERDatasetSettings &datasetSettings,
                                         const GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery,
                                         GexDbPluginERDataset& datasetResult);
    bool    AER_buildAZQueryBinning(const GexDbPluginERDatasetSettings& datasetSettings,
                                    GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery);
    bool    AER_buildQueryProductBinning(const GexDbPluginERDatasetSettings& datasetSettings,
                                         GexDbPluginSQL::GexDbSQLQuery& gexdbSQLQuery, bool bIntermediate);

    // Database administration functions
    QFile           m_hUpdateDbLogFile;
    QString         m_strUpdateDbLogFile;

    unsigned int    m_uiDbVersionBuild;
    unsigned int    m_uiDbVersionMajor;
    unsigned int    m_uiDbVersionMinor;
    QString         m_strDbVersionName;
    QString         m_strDbStatus;
    QString         m_strDbHashType;
    QString         m_strSecuredMode;
    DataBaseType    m_eDbType;
    QString          mTdrLinkName;
    QString          mAdrLinkName;


    QMap<QString,QString> m_mapDbStatusMessage;
    QMap<int,GexDbPlugin_Option> mDbGlobalOptions;
    QMap<QString,int> mDbGlobalOptionsMapping;
    //! \brief List of standard incremental update keys
    QMap< QString,QMap< QString,QString > > mIncrementalUpdateName;

    // Map containing the parameters of this task
    // exple : "CheckType"="FixedYieldTreshold", "Title"="Coucou",...
    QMap<QString, QVariant> mAttributes;
    // mAttributes.insert( "PostImportCrash", 1 );
    // mAttributes.insert( "PostImportCrashMoveFolder", "path" );
    QVariant GetAttribute(const QString &key)                   { return mAttributes.value(key.toLower()); }
    void     SetAttribute(const QString &key, QVariant value)   { mAttributes.insert(key.toLower(), value); }

    // Add DB type (MySQL, Oracle...) to base DB name
    QString     GetDbName(const QString & strDbName_Base=QString());
    QString     GetDbStatus();
    // Load Options Definition
    bool        LoadGlobalOptions();
    // Create Options Definition
    bool        CreateGlobalOptions(int optionNumber,GexDbPlugin_Option optionDef);
    // Load Standard Incremental Update ames
    //! \brief List of standard incremental update keys
    bool        LoadIncrementalUpdateNames();


    // Returns the test number associated with the test name for the given test type.
    // If not test number already assigned, create an auto-incremented test number
    // WORKS ONLY FOR AUTO-INCREMENT TEST NUMBER FEATURE
    qint64      GetAutoIncrementTestNumber(int lTestType,
                                           const QString& lOriginalTestName);

    bool        SetTestingStage(int eTestingStage);
    void        SetTestingStage();
    void        GetTestingStageName(int eTestingStage, QString & strTestingStageName);
    void        GetCurrentTestingStageName(QString & strTestingStageName);
    bool        AddConsolidatedFields(const QString & strCastNumber);
    bool        CreateConsolidatedTables(enum TestingStage eTestingStage=eUnknownStage);
    bool        CreateConsolidatedUpdateProcedures(enum TestingStage eTestingStage=eUnknownStage);
    bool        CreateConsolidationTriggers(QString *pstrRootUser=NULL, QString *pstrRootPassword=NULL,enum TestingStage eTestingStage=eUnknownStage);
    bool        DropConsolidationTriggers(enum TestingStage eTestingStage=eUnknownStage, QString *pstrRootUser=NULL,QString *pstrRootPassword=NULL);
    QStringList GetIndexForConsolidatedTables(enum TestingStage eTestingStage=eUnknownStage);

    bool        AddConsolidatedField_TL(QString & strQuery_Fields, const QString & strTableAlias,
                                        const QString & strMetaDataName, const QString & strCastNumber);
    bool        CreateConsolidatedSelectQuery_TL(QString & strQuery, const QString & strFromTableName,
                                                 const QString & strTableAlias, const QString & strFtTrackingLotID,
                                                 const QString & strCastNumber);
    bool        CreateConsolidatedTable_TL(
            const QString & strTableName, const QString & strTableAlias,
            const QString & strFromTableName, const QString & strCastNumber,
            const QString & strTableEngine, const QString & strIndexSuffix);
    bool        CreateConsolidatedTables_TL(
            enum TestingStage eTestingStage=eUnknownStage);
    bool        CreateConsolidatedUpdateProcedure_TL(
            const QString & strTableName, const QString & strTableAlias,
            const QString & strFromTableName, const QString & strCastNumber,
            const QString & strTableEngine, const QString & strIndexSuffix);
    bool        CreateConsolidatedUpdateProcedures_TL(
            enum TestingStage eTestingStage=eUnknownStage);
    bool        AddConsolidatedField_AZ(QString & strQuery_Fields, const QString & strTableAlias,
                                        const QString & strMetaDataName, const QString & strCastNumber,
                                        bool bAddPrefix=false);
    bool        CreateConsolidatedSelectQuery_AZ_Data(QString & strQuery, QString & strTableAlias_FT,
                                                      QString & strTableAlias_WT, const QString & strFtTrackingLotID,
                                                      const QString & strCastNumber);
    bool        CreateConsolidatedTable_AZ_Data(
            const QString & strCastNumber, const QString & strTableEngine,
            const QString & strIndexSuffix);
    bool        CreateConsolidatedSelectQuery_AZ_Facts(QString & strQuery, const QString & strFtTrackingLotID,
                                                       const QString & strCastNumber);
    bool        CreateConsolidatedTable_AZ_Facts(
            const QString & strCastNumber, const QString & strTableEngine,
            const QString & strIndexSuffix);
    bool        CreateConsolidatedSelectQuery_AZ(QString & strQuery, const QString & strFtTrackingLotID);
    bool        CreateConsolidatedTable_AZ(
            const QString & strCastNumber, const QString & strTableEngine,
            const QString & strIndexSuffix);
    bool        CreateConsolidatedTables_AZ(   );
    bool        CreateConsolidatedUpdateProcedure_AZ(   );
#if 0
    bool        CreateAzUpdateProcedures(
            enum TestingStage eTestingStage=eUnknownStage);
    bool        CreateAzTriggers(
            QString *pstrRootUser=NULL, QString *pstrRootPassword=NULL,
            enum TestingStage eTestingStage=eUnknownStage);
    bool        DropAzTriggers(   QString *pstrRootUser=NULL,
                                  QString *pstrRootPassword=NULL);
#endif
    bool        ConsolidateWafer(QString & strLotID, QString & strWaferID,
                                 bool bCallConsolidationFunction=true);
    bool        ConsolidateLot(QString & strProductName, QString & strTrackingLotID, QString & strLotID,
                               bool bCallConsolidationFunction=true);
    bool        purgeDataBase(GexDbPlugin_Filter &roFilters,
                              GexDbPlugin_SplitlotList &oSplitLotList);
    bool exportCSVCondition(const QString &strCSVFileName, const QMap<QString, QString>& oConditions, GexDbPlugin_Filter &roFilters, GexDbPlugin_SplitlotList &oSplitLotList, QProgressDialog *poProgress);

    QStringList FieldToDelete(const QString &strField, const QString &strTable,
                              const QString &strConnectionName);
    bool        GetConsolidationRetestPhases(const QString &LotId, const QString &SubLotId,
                                             QString &ProdData, QStringList &lstPhases);
    bool        GetConsolidationRetestInfo(const QString &LotId, const QString &SubLotId,
                                             QString &ProdData, QStringList &lstPhases,
                                           QMap<QString, QMap<int,QStringList> > &mapPhaseRetestHBins);
    bool        GetDbUpdateSteps(unsigned int & uiFlags);
    bool        AddDbUpdateSteps(unsigned int uiFlags);
    bool        RemoveDbUpdateSteps(unsigned int uiFlags);
    bool        UpdateDb_B11_to_B12(   );
    bool        UpdateDb_B11_to_B12_UpdateMapping(   );
    bool        UpdateDb_B11_to_B12_UpdateProductBinTables(   );
    bool        UpdateDb_B11_to_B12_UpdateIndexes(   );
    bool        UpdateDb_B12_to_B13(   );
    bool        UpdateDb_B12_to_B13_UpdateSyaTables();
    bool        UpdateDb_B12_to_B13_UpdateTablesForGenealogy();
    bool        UpdateDb_B12_to_B13_UpdateTablesForNullableValue();
    bool        UpdateDb_B13_to_B14(   );
    bool        UpdateDb_B13_to_B14_CreateCustomIncrementalUpdateProcedure(   );
    bool        UpdateDb_B14_to_B15(   );
    bool        UpdateDb_B14_to_B15_UpdateIncrementalUpdateTable(   );
    bool        UpdateDb_B14_to_B15_UpdateGlobalInfoTable(   );
    bool        UpdateDb_B15_to_B16(   );
    bool        UpdateDb_B15_to_B16_UpdateWtWaferInfoTable(   );
    bool        UpdateDb_B15_to_B16_CreateGlobalFilesTable(   );
    bool        UpdateDb_B15_to_B16_CreateWtIntermediateTables(   );
    bool        UpdateDb_B15_to_B16_UpdateGlobalInfoTable(   );
    bool        UpdateDb_B16_to_B17(   );
    bool        UpdateDb_B17_to_B18(   );
    bool        UpdateDb_B17_to_B18_UpdateGlobalInfoTable(   );
    bool        UpdateDb_B17_to_B18_CreateTestConditionsTable(TestingStage testingStage);
    bool        UpdateDb_B18_to_B19(   );
    bool        UpdateDb_B18_to_B19_UpdateSizeScheduler(   );
    bool        UpdateDb_B18_to_B19_CreateDieTraceabilityTable(   );
    bool        UpdateDb_B19_to_B20(   );
    bool        UpdateDb_B20_to_B21(   );
    bool        UpdateDb_B21_to_B22(   );
    bool        UpdateDb_B22_to_B23(   );
    bool        UpdateDb_B23_to_B24(   );
    bool        UpdateDb_B24_to_B25(   );
    bool        UpdateDb_B24_to_B25_CreatePinMapsTables(TestingStage testingStage);
    bool        UpdateDb_B14_Incremental(   QString & strLog);
    bool        UpdateDb_FromSqlScript(UINT fromBuild, UINT toBuild, const GexDbPlugin_Connector &rootConnector);
    bool        UpdateDb_CheckServerVersion(DataBaseType databaseType, QString lSqlConnection=QString());
    bool        UpdateDb_CreateSchedulerSize(   );
    bool        UpdateDb_DropTableIfExists(const QString & strTableName);
    bool        UpdateDb_UpdateIndexes(   QStringList lstIndexesToCheck=QStringList());

    // update nearly all the tables to InnoDB engine.
    bool        UpdateDb_To_InnoDB(   bool bToBarracudaCompressed=false);

    // Load Test conditions meta-data
    bool LoadDynamicTCMetaData(TestingStage eTestingStage, unsigned int uiDbVersion_Build);


    // Update db architecture with remotes tables found
    bool         UpdateDbArchitectureWithExternalLink(
            const GexDbPlugin_Mapping_LinkMap &linksMap,
            TestingStage testingStage);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // For Extraction
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////
    // VARIABLES DEFINITION
    /////////////////////////////////////////////////
    // Database mapping
    // Fields mapping: GEX <-> Custom DB (E-Test)
    GexDbPlugin_Mapping_FieldMap m_mapFields_GexToRemote_Et;
    // Table links mapping (E-Test)
    GexDbPlugin_Mapping_LinkMap  m_mapLinks_Remote_Et;
    // Fields mapping: GEX <-> Custom DB (Wafer Sort)
    GexDbPlugin_Mapping_FieldMap m_mapFields_GexToRemote_Wt;
    // Table links mapping (Wafer Sort)
    GexDbPlugin_Mapping_LinkMap  m_mapLinks_Remote_Wt;
    // Fields mapping: GEX <-> Custom DB (Final Test)
    GexDbPlugin_Mapping_FieldMap m_mapFields_GexToRemote_Ft;
    // Table links mapping (Final Test)
    GexDbPlugin_Mapping_LinkMap  m_mapLinks_Remote_Ft;
    QStringList     m_strlLabelFilterChoices_Et;
    QStringList     m_strlLabelFilterChoices_Ft;
    QStringList     m_strlLabelFilterChoices_Wt;
    QStringList     m_strlLabelFilterChoices_Consolidated_Et;
    QStringList     m_strlLabelFilterChoices_Consolidated_Ft;
    QStringList     m_strlLabelFilterChoices_Consolidated_Wt;
    QStringList     m_strlLabelFilterChoices_WaferLevel_Et;
    QStringList     m_strlLabelFilterChoices_WaferLevel_Wt;
    QStringList     m_strlLabelFilterChoices_LotLevel_Et;
    QStringList     m_strlLabelFilterChoices_LotLevel_Wt;
    QStringList     m_strlLabelFilterChoices_LotLevel_Ft;

    // Other
    // RUN matrix of RunInfo pointers
    GexDbPlugin_RunInfo *m_pRunInfoArray;
    // Nb of items in RUN array
    int             m_nRunInfoArray_NbItems;
    // Nb of runs for on Splitlot
    unsigned int    m_uiSplitlotNbRuns;
    // Testlist for extraction
    GexDbPlugin_TestInfoList m_clTestList;
    // Testing stage for current query
    QString         m_strTestingStage;
    // Prefix for table names corresponding to given testing stage
    QString         m_strTablePrefix;
    // Build of current GexDb
    unsigned int    m_uiCurrentGexdbBuild;
    // Software Bin mapping
    GexDbPlugin_BinMap m_mapSoftBins;
    // Hardware binning map
    GexDbPlugin_BinMap m_mapHardBins;
    // Skip all index dialogs (Oracle when index unusable)
    bool            m_bSkipAllIndexDialogs;
    // Set to true if extraction should be aborted due to unusable index
    bool            m_bAbortForUnusableIndex;
    // Widget with Galaxy RDB-custom options GUI
    GS::DbPluginGalaxy::RdbOptionsWidget *mRdbOptionsWidget;
    GS::DbPluginGalaxy::RdbOptions m_clOptions;

    /////////////////////////////////////////////////
    // FUNCTIONS DEFINITION
    /////////////////////////////////////////////////
    // Standard Extraction (for standard Examinator Reports)
    void Init_Extraction();
    // Get nb of Splitlots matching query
    int  GetNbOfSplitlots(GexDbPlugin_Filter &cFilters);
    // Get list of Splitlots ID (int) matching Lot Wafer
    QString GetSplitlotsList(QString LotID, QString WaferID, QString TS, QList<int> &l);
    // To Do ?
    //QString  GetSplitlotsList(GexDbPlugin_Filter &cFilters, QStringList &sl);
    // Get list of lots/sublots concerning the filters
    int     GetLotSublotsList(GexDbPlugin_Filter &cFilters,
                              QList< QMap< QString,QString > > &lFlowToExtract);

    // retrieves the TrackingLotID from given LotID and TS
    // TS must be GEXDB_PLUGIN_GALAXY_ETEST,...
    // returns "ok" or "error..."
    QString GetTrackingLotID(const QString LotID, const QString TestingStage, QString &TLID);

    // Search and try to retrieve from the given TLID and WID the corresponding LotWafer
    // for the given TestingStage
    // the WaferID found should be the same but sometimes not really : exemple : "08" vs "8"
    // TrackingWaferNB is a number !
    // returns "ok" or "error ..."
    QString GetMatchingLotWafer(const QString TrackingLotID,
                                const int TrackingWaferNB,
                                const QString targetedTestingStage,
                                QString &LotID, QString &WaferID);

    // Search for site config for given lot wafer (through wafer_info table)
    // returns "error..." or "ok"
    QString GetETestSiteConfig(const QString &lotid,
                               const int &wafer_number, QString site_config);

    // Get distinct XY for the given LotWafer
    // result will contain for each dsitinct XY
    // a Map of key-value : "splitlot_id" = 34534563, "hbin_no" = 4, "sbin_no" = 2, "run_id" = 1, ...
    bool GetDistinctXY(const QString &Lot, const QString &Wafer, GexDbPlugin_Filter &cFilters,
                          QMap< QPair<int, int>, QMap<QString, QVariant> > &r);


    // Construct query to retrieve all Splitlots corresponding to specified filters
    // ordering by lot_id, start_t
    bool    ConstructSplitlotQuery(GexDbPlugin_Filter &cFilters,
                                   QString & strQuery,    // will contain the query to execute
                                   unsigned int uiGexDbBuild,
                                   bool bClearQueryFirst=true, bool bPurge=false);
    // Construct query to retrieve all Splitlots corresponding to specified SplitlotID
    bool    ConstructSplitlotQuery(long lSplitlotID, QString &strQuery, bool bClearQueryFirst=true);
    // Add all splitlot fields
    void    ConstructSplitlotQuery_Fields(bool bPurge=false);
    // Fill Galaxy SplitlotInfo object with query result
    QString    FillSplitlotInfo(GexDbPlugin_Query & clGexDbQuery_Splitlot,
                                GexDbPlugin_Galaxy_SplitlotInfo    *pclSplitlotInfo,
                                const GexDbPlugin_Filter & cFilters,
                                const QVector<int>* pvHbinsToExclude=NULL);

    // Create a STDF file for the specified Splitlot_id.
    // strStdfFileName will contain the name of the generated stdf if success.
    // m_vHbinsToExclude can contain a list of hbins to exclude but can be NULL
    // sampling controls the extraction sample rate : 1 = 1 out of 1 = all samples, 10 = 1 out of 10 = 1 test results each 10 results
    //
    bool    CreateStdfFile(
            GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
            GexDbPlugin_Galaxy_TestFilter & clTestFilter,
            const QString & strLocalDir,
            QString & strStdfFileName,
            const GexDbPlugin_Filter & cFilters,
            const int &sampling=1);

    // Create a STDF file name: TS, Product, Tester, Lot, Wafer, Site, Splitlot ID
    // Exple: GEXDB_XT_PA_T1_L1_W1_S1_1230600033
    bool    CreateStdfFileName(QString& fileName,
                               const GexDbPlugin_Galaxy_SplitlotInfo& clSplitlotInfo,
                               const GexDbPlugin_Galaxy_TestFilter & clTestFilter,
                               const GexDbPlugin_Filter &dbFilters,
                               const GexDbPlugin_Galaxy_WaferInfo *gdpgwiPtrWaferInfo
                               ) const;

    // Create a stdf file at Etest Testing Stage but with the die granularity
    QString    CreateStdfFileForETestDieGranularity(GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
                                                    GexDbPlugin_Galaxy_TestFilter & clTestFilter, const QString & strLocalDir,
                                                    QString & strStdfFileName,
                                                    const QString &WSWaferID, const QString &WSLotID,
                                                    const QMap<QPair<int, int>, QMap<QString, QVariant> > &xys,
                                                    const QString& waferzonefilepath,
                                                    const QString desired_et_notch, const QString desired_ws_notch,
                                                    QMap<QRgb, int> color_to_site_no,
                                                    const GexDbPlugin_Filter & cFilters
                                                    );

    bool    WriteMIR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);
    bool    WriteSDR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);
    bool    WriteMRR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);
    bool    WriteWIR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
                     const GexDbPlugin_Galaxy_WaferInfo *pclWaferInfo);
    bool    WriteWRR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
                     const GexDbPlugin_Galaxy_WaferInfo *pclWaferInfo);
    bool    WriteWCR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_WaferInfo *pclWaferInfo);
    bool    WritePIR(GQTL_STDF::StdfParse & clStdfParse, int nSiteNum);
    // Write STDF PRR record for specified Splitlot
    bool    WritePRR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_RunInfo *pCurrentPart);

    // Construct query string to retrieve all tests matching filters
    bool   ConstructTestlistQueryBACK(GexDbPlugin_Filter &cFilters, QString & strQuery,
                                      QString & strTestNumField, QString & strTestNameField);

    // Create TestList for specified Splitlot_id
    bool    CreateTestlist(
            const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
            GexDbPlugin_Galaxy_TestFilter & clTestFilter,
            const GexDbPlugin_Filter & cFilters);

    // Extract multi limits from xx_ptest_static_limits for a given splitlot
    bool    ExtractMultiLimits(QMap<int, QList<QVariantMap> > &multiLimits,
                                int splitlotId,
                                const QString& tablePrefix);

    qint64    GetTestID(unsigned int uiTestNum);

    bool    WritePinMapRecords(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);

    // Write static test information
    bool    WriteStaticTestInfo(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);

    // Init different maps: array with part results, binning map,... for specified Splitlot_id
    // Filter is now mandatory because run query will be filtered if any filters on run level is found in metadatas
    // xys can list all XY to consider : the QMap<QString, QVariant> contains "splitlot_id", "run_id",...
    // sampling : 1 or upper
    bool    InitMaps(
            const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
            const GexDbPlugin_Filter& cFilters,
            bool bClearBinningMaps=true,
            QMap< QPair<int, int>, QMap<QString, QVariant> > *xys=NULL,
            unsigned int sampling=1
                                  );

    bool  InsertFilters(QStringList queryFilters);

    // Init maps: array with part results, binning map,... for specified Splitlot_id
    // in the purpose of generating an Etest Wafer with all dies of WaferSort
    // SplitlotInfo must be the ETest Splitlot
    // WSWaferID and WSLotID are the corresponding Lot Wafer at WS testing stage
    // xys are the distincts XY that will be considered
    QString InitMapsForETestDieGranularity(const GexDbPlugin_Galaxy_SplitlotInfo & clETSplitlotInfo,
                                           const QString &WSWaferID,
                                           const QString &WSLotID,
                                           const QMap< QPair<int, int>, QMap<QString, QVariant> > &xys,
                                           const QString &waferzonefilepath,
                                           int rotation_in_degree,
                                           const QMap<QRgb, int> color_to_site_no,
                                           bool bClearBinningMaps=true);

    // Write all test results for specified Splitlot_id
    QString WriteTestResults(GQTL_STDF::StdfParse & clStdfParse,
                             const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
                             GexDbPlugin_Galaxy_TestFilter & clTestFilter,
                             QMap< QPair<int, int>, QMap<QString, QVariant> > *xys=NULL,
                             bool autowrite_parts_with_same_runid=false,
                             const int &sampling=1 );

    // Write test information for current run
    bool    WriteTestInfo(GQTL_STDF::StdfParse & clStdfParse,
                          const GexDbPlugin_RunInfo *pCurrentPart);

    // Write summary records (SBR, HBR , PCR, TSR)
    // if run_filtered then some records wont be written in order to not induce misunderstanding for customers
    bool    WriteSummaryRecords(GQTL_STDF::StdfParse & clStdfParse,
                                const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
                                const bool run_filtered,
                                const GexDbPlugin_Filter& dbFilters);

    // Write summary records (SBR, HBR , PCR, TSR)
    bool    WriteSummaryRecords_Consolidated(
            GQTL_STDF::StdfParse & clStdfParse,
            const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);

    // Write Part : WritePIR, WriteTestResult(PTR,MPR,FTR), WritePRR
    // for given GexDbPlugin_RunInfo* part
    // write_all_other_RunInfo_with_same_run_id will write too all other parts with same run_id
    // Will set pCurrentPart->m_bWrittenToStdf = true;
    bool    WritePart(GQTL_STDF::StdfParse & clStdfParse,
                      GexDbPlugin_RunInfo *pCurrentPart,
                      bool write_all_other_RunInfo_with_same_run_id=false );

    // returns the very first RunInfo with the desired run_id in  m_pRunInfoArray
    // It will check if table + desired_run_id is a good one
    // returns NULL if not found
    // search in m_pRunInfoArray of size m_nRunInfoArray_NbItems !
    // Warning : can be time consuming !
    GexDbPlugin_RunInfo* GetPart(int desired_run_id);

    void    Query_AddTestlistCondition(
            const GexDbPlugin_Galaxy_TestFilter & clTestFilter,
            const char cTestType,
            enum TestlistConditionTarget eTarget=eOnTestResults);
    GexDbPlugin_Galaxy_WaferInfo    *GetWaferInfo(
            const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);
    // Construct query to retrieve UPH corresponding to specified filters
    bool    ConstructUPHQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, const QString & strSplitField,
                              const QString & strCumulField);
    // Construct query to retrieve Yield corresponding to specified filters
    bool    ConstructYieldQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, const QString & strSplitField,
                                const QString & strCumulField, int nBinning, bool bSoftBin);
    // Construct query to retrieve Yield corresponding to specified filters
    bool    ConstructYieldQuery_NoSite_NoBinning(GexDbPlugin_Filter &cFilters, QString & strQuery,
                                                 const QString & strSplitField, const QString & strCumulField);
    // Construct query to retrieve Yield corresponding to specified filters
    bool    ConstructYieldQuery_Site_NoBinning(GexDbPlugin_Filter &cFilters, QString & strQuery,
                                               const QString & strSplitField, const QString & strCumulField);
    // Construct query to retrieve Yield corresponding to specified filters
    bool    ConstructYieldQuery_Binning(GexDbPlugin_Filter &cFilters, QString & strQuery,
                                        const QString & strSplitField, const QString & strCumulField,
                                        int nBinning, bool bSoftBin);
    // Construct query to retrieve Consolidated Yield corresponding to specified filters
    bool    ConstructConsolidatedYieldQuery(GexDbPlugin_Filter &cFilters, QString & strQuery,
                                            const QString & strSplitField, const QString & strCumulField);
    // Add a field to query selection, eventually based on an expression (day, week...)
    bool    Query_AddFieldExpression(const QString & strMetaField, const QString & strAs, QString & strDbField,
                                     QString & strDbTable, bool bUseMaxStartT=false, bool bUseSamples=false,
                                     int nBinning=-1, bool bSoftBin=false);
    bool    GetTestStats_P(const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);
    bool    GetTestStats_MP(const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);
    bool    GetTestStats_F(const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);
    bool    CreateBinningMaps(
            const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
            bool bClearBinningMaps=true);
    bool    WritePatDtrRecords(GQTL_STDF::StdfParse & clStdfParse,
                               const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo);

    // Extraction for Enterprise reports
    // Construct query to retrieve all data for ER Production reports on parts
    bool    ER_Prod_ConstructPartQuery(GexDbPlugin_Filter &cFilters,
                                       GexDbPlugin_ER_Parts & clER_PartsData,
                                       QString & strQuery);
    // Construct query to retrieve all data for ER Production reports on parts
    bool    ER_Prod_ConstructPartQuery_Binning(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData,
                                               QString & strQuery);
    // Construct query to retrieve all data for ER Production reports on parts
    bool    ER_Prod_ConstructPartQuery_NoBinning(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData,
                                                 QString & strQuery);
    // Query: Construct query to retrieve bincount data for ER Production reports
    bool    ER_Prod_ConstructBinCountQuery(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData,
                                           GexDbPlugin_ER_Parts_Graph *pGraph, GexDbPlugin_ER_Parts_Layer *pLayer,
                                           const QString & strAggregateLabel, QString & strQuery);
    bool    ER_Prod_ConstructSplitCountQuery(GexDbPlugin_Filter &cFilters, GexDbPlugin_ER_Parts & clER_PartsData,
                                             QString & strQuery);
    // Add a field to query selection, eventually based on an expression (day, week...)
    bool    ER_Query_AddFieldExpression(const QString & strMetaField, const QString & strAs, QString & strDbField,
                                        QString & strDbTable, bool bUseMax=false);
    // Add a part count to query selection
    bool    ER_Query_AddPartCount(const QString & strMetaField, const QString & strAs, bool bUseSamples,
                                  GexDbPlugin_ER_Parts & clER_PartsData, const QString & strBinspec, bool bSumParts);
    // Add a test count to query selection (execs, fails, sum)
    bool    ER_Query_AddTestCount(GexDbPlugin_ER_Parts_SerieDef *pSerieDef, const QString & strMetaField,
                                  const QString & strAs, bool bUseSamples, bool bSumTests);
    bool    ER_Query_AddConditionExpression(const QString & strMetaField, const QString & strValue,
                                            QString & strDbField, QString & strDbTable, bool bUseMax/*=false*/);

    // Construct query to retrieve all data for ER Production reports on parts
    bool    ER_Genealogy_YieldVsYield_ConstructQuery(
            GexDbPlugin_Filter &cFilters,
            GexDbPlugin_ER_Parts & clER_PartsData,
            QString & strQuery);
    // Construct core query to retrieve all data for ER Production reports on parts
    bool    ER_Genealogy_Yield_ConstructQuery_Core_Lot(
            GexDbPlugin_Filter &cFilters,
            GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
            QString & strQuery,
            int nIndex);
    // Construct core query to retrieve all data for ER Production reports on parts
    bool    ER_Genealogy_Yield_ConstructQuery_Core_Lot_NoBinning(
            GexDbPlugin_Filter &cFilters,
            GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
            QString & strQuery);
    // Construct core query to retrieve all data for ER Production reports on parts
    bool    ER_Genealogy_Yield_ConstructQuery_Core_Lot_Binning(
            GexDbPlugin_Filter &cFilters,
            GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
            QString & strQuery,
            int nIndex);
    // Construct core query to retrieve all data for ER Production reports on parts
    bool    ER_Genealogy_Yield_ConstructQuery_Core_Wafer(
            GexDbPlugin_Filter &cFilters,
            GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
            QString & strQuery,
            int nIndex);
    // Construct core query to retrieve all data for ER Production reports on parts
    bool    ER_Genealogy_Yield_ConstructQuery_Core_Wafer_NoBinning(GexDbPlugin_Filter &cFilters,
            GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
            QString & strQuery,
            int nIndex);
    // Construct core query to retrieve all data for ER Production reports on parts
    bool    ER_Genealogy_Yield_ConstructQuery_Core_Wafer_Binning(
            GexDbPlugin_Filter &cFilters,
            GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
            QString & strQuery,
            int nIndex);
    // Construct query to retrieve all data for ER Production reports on parts/parameter
    bool    ER_Genealogy_YieldVsParameter_ConstructQuery(
            GexDbPlugin_Filter &cFilters,
            GexDbPlugin_ER_Parts & clER_PartsData,
            QString & strQuery);
    // Construct core query to retrieve all data for ER Production reports on parts/parameter
    bool    ER_Genealogy_Mean_ConstructQuery_Core_Lot(
            GexDbPlugin_Filter &cFilters,
            GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
            QString & strQuery, int nIndex);
    // Construct core query to retrieve all data for ER Production reports on parts/parameter
    bool    ER_Genealogy_Mean_ConstructQuery_Core_Wafer(
            GexDbPlugin_Filter &cFilters,
            GexDbPlugin_ER_Parts_SerieDef *pSerieDef,
            QString & strQuery, int nIndex);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // For Insertion
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////
    // VARIABLES DEFINITION
    /////////////////////////////////////////////////
    // Ftp settings to use for configuration wizard
    GexDbPlugin_FtpSettings  m_clFtpSettings;
    // STDF V4 parser
    GQTL_STDF::StdfParse m_clStdfParse;
    // STDF File to convert
    QString                  m_strStdfFile;
    // STDF file size for progress bar
    int                      m_nFileSize;
    // 2 passes are required to process the STDF file.
    int                      m_lPass;
    // when an error occurs, have stop all processes
    bool                     m_bStopProcess;
    // When stop requested by user/service
    bool                     mStopRequested;
    // List of warning messages issued during the insertion of a data file
    QStringList              m_strlWarnings;
    // Holds MIR information (eventually overloaded from data key file)
    GS::QtLib::DatakeysEngine * mpDbKeysEngine;
    // Holds the die info for each die in a FT part
    QMap<QString,QString>    m_mapDieTrackingInfo;
    // Holds the list of updated tables during 1 insertion
    GexDbPlugin_Galaxy_UpdatedTables  m_clUpdatedTables;
    // Index of DTR inside a run
    unsigned int             m_uiDtrIndex_InRun;
    // Index of DTR inside a splitlot
    unsigned int             m_uiDtrIndex_InSplitlot;

    // PROGRAM INFORMATION
    // product name
    QString         m_strProductName;
    // FileHost_ID for reference
    int             m_nFileHostId;
    // Tracking_Lot_ID for reference
    QString         m_strTrackingLotId;
    // Lot_ID for reference
    QString         m_strLotId;
    // SubLot_ID for reference
    QString         m_strSubLotId;
    // Wafer_ID for reference
    QString         m_strWaferId;
    int             m_nSplitLotFlags;
    QString         m_strDataProvider;
    QString         m_strDataType;
    int             m_nReworkCode;

    // GLOBAL INFORMATION
    int             m_nNbSites;
    // Nb parts for current SPLITLOT
    int             m_nNbParts;
    // Nb good parts for current SPLITLOT
    int             m_nNbPartsGood;
    // Nb runs for current SPLITLOT (for retest)
    QMap<int,int>   m_mapNbRuns;
    // Nb good runs for current SPLITLOT (for restest)
    QMap<int,int>   m_mapNbRunsGood;
    // Set to true if insertion should be delayed
    bool            *m_pbDelayInsertion;
    // List of splitlots inserted (>1 in case of multi-wafer data files)
    QList<int>      m_nlInsertedSplitlots;

    // FOR MULTI WAFER
    // Status after load a STDF Record
    int   m_iStatus;
    int   m_nRecordType;
    // false if record allready loaded
    bool  m_bLoadNextRecord;
    // current wafer in the stdf file
    int   m_nCurrentWaferIndex;
    // number of head per wafer (1 in general cas)
    int   m_nNbHeadPerWafer;
    // number of head per wafer (1 in general cas)
    int   m_bHaveMultiHeadWafer;
    // read only information for the first wafer record
    int   m_nWaferIndexToProcess;
    // skeep all result not corresponding to this wafer
    // Total wafer to process
    int   m_nTotalWaferToProcess;

    // PCR information (validation function)
    // Nb parts for current SPLITLOT
    QMap<int,int>   m_mapPCRNbParts;
    // Nb parts restested for current SPLITLOT (for retest)
    QMap<int,int>   m_mapPCRNbRetestParts;
    // Nb good runs for current SPLITLOT (for restest)
    QMap<int,int>   m_mapPCRNbPartsGood;
    // If have good summary, use it for splitlot and lot
    bool            m_bUsePcrForSummary;

    // RUN INFORMATION
    // HAVE TO SPLIT INFORMATION PER SITE
    // Run_ID for reference
    int           m_nRunId;
    // Run_ID for reference
    QMap<int,int> m_mapRunId;
    // for each run, contain ...
    // the number of tests executed (is a global info - no split per site)
    int               m_nTestsSequence;
    // the number of tests executed
    QMap<int,int>     m_mapTestsExecuted;
    // the number of tests failed
    QMap<qint64,int>     m_mapTestsFailed;
    // the type of the first test failed
    QMap<int,QString> m_mapFirstFailTType;
    // the id of the first test failed
    QMap<int,qint64>  m_mapFirstFailTId;
    // the type of the last test failed
    QMap<int,QString> m_mapLastFailTType;
    // the id of the last test failed
    QMap<int,qint64>  m_mapLastFailTId;

    // INVALID HBIN and INVALID PRR
    // applied when the first PRR is invalid (HBin and SBin invalid)
    bool    m_bIgnoreFirstRun;
    // All SBin must be associated with a specific and unique HBin
    QMap<int, int>  mSBinToHBin;
    // All invalid HBin must be associated with a specific and unique SBin
    QMap<int, int>  mInvalidHBinToSBin;
    // All invalid SBin must be associated with a specific and unique HBin
    QMap<int, int>  mInvalidSBinToHBin;

    // BIN INFORMATION
    struct structBinInfo
    {
        int                 m_nBinNum;
        QMap<int,int>       m_mapBinNbParts; // 255 for all sites, next for each site
        QMap<int,int>       m_mapBinNbRuns;  // 255 for all sites, next for each site
        QMap<int,int>       m_mapBinCnt;     // from HBR or SBR
        QString             m_strBinName;
        char                m_cBinCat;
    };

    // list of HBin
    QMap<int,structBinInfo>    m_mapHBinInfo;
    // list if SBin
    QMap<int,structBinInfo>    m_mapSBinInfo;
    // list of PinMap Definition
    // The key is the PRIMARY KEY
    // The value is the QueryString to insert
    QMap<QString,QString>     m_mapPinDef;

    // TEST INFORMATION
    struct structTestInfo
    {
        // During the insertion process
        int                    m_nTestInfoIndex;      // current index into m_listTestInfo
        qint64                 m_nOriginalTestNum;    // test definition - number
        QString                m_strOriginalTestName; // name

        int                    m_nInfoId;             // reference in the database
        // PTR, FTR, MPR
        int                    m_nTestType;            // Stdf Record Type
        qint64                 m_nTestNum;            // formated test number (customized from user)
        QString                m_strTestName;         // formated name (customized from user)
        int                    m_nTestPinIndex;       // pin index  - an pin = an entry
        int                    m_nTestPin;            // pin number - an pin = an entry
        int                    m_nTestSeq;            // executed order
        QString                m_strUnits;            // units
        char                   m_cFlags;              // flag information
        QMap<int,float>        m_mapfLL;
        QMap<int,float>        m_mapfHL;
        // From Samples
        // info for each sites (255 for all sites)
        QMap<int,unsigned int> m_mapExecCount;
        QMap<int,unsigned int> m_mapFailCount;
        QMap<int,float>        m_mapMinValue;
        QMap<int,float>        m_mapMaxValue;
        QMap<int,float>        m_mapSum;
        QMap<int,float>        m_mapSquareSum;
        // From Summary
        // info for each sites (255 for all sites)
        QList<int>             mTsrSites;
        QMap<int,unsigned int> mTsrExecCount;
        QMap<int,unsigned int> mTsrFailCount;
        QMap<int,float>        mTsrMinValue;
        QMap<int,float>        mTsrMaxValue;
        QMap<int,float>        mTsrSum;
        QMap<int,float>        mTsrSquareSum;
        QMap<int,float>        mTsrTTime;

        // FOR LIMITS SPECIFICATION
        bool                 m_bHaveSpecLL;
        bool                 m_bHaveSpecHL;
        bool                 m_bHaveSpecTarget;
        float                m_fSpecLL;
        float                m_fSpecHL;
        float                m_fSpecTarget;

        // FOR LIMITS SPECIFICATION
        bool                 m_bHaveScalRes;
        bool                 m_bHaveScalLL;
        bool                 m_bHaveScalHL;
        int                  m_nScalRes;
        int                  m_nScalLL;
        int                  m_nScalHL;

        // FOR TEST CONDITIONS
        QMap<QString, QString>  mTestConditions;

        // FOR STATS LIMITS
        QList<GS::Core::MultiLimitItem> mMultiLimits;       ///< List of multi limit items
    };

    // list of all tests info encountered in the STDF file
    QList<structTestInfo*>    m_lstTestInfo;

    // current index on m_listTestInfo
    int         m_lstTestInfo_Index;

    // Search optimization
    // list of all test number encountered in the STDF file (first filter before to parse m_lstTest)
    // QList<qint64>   mTestNumbers;
    QMultiMap<qint64, structTestInfo*>   mTestNumbers;
    // list of all test names encountered in the STDF file (second filter before to parse m_lstTest)
    // append the TestNum and the TestName used to find a pTest
    // QList<QString>  mTestNames;
    QMultiMap<QString, structTestInfo*>  mTestNames;

    // Internal counters to store count info for anything
    // Search Inconsistent Test Numbers
    // Store the number of test created for the current run
    // Store the number of test executed for the current run
    // Store the number of inconsistent test number detected

    // Check per site (MERGE_SITE for all)
    int       mConsistentPartsFlow;
    int       mInconsistentPartsFlow;
    int       mConsistentTests;
    int       mInconsistentTests;

    int       mTotalTestsCreated;                   // Total tests into the Internal Test list
    int       mTotalTestsCreatedForReferenceFlows;  // Count the total tests executed/created for all reference flows
    QMap<QString,int> mReferenceFlows;              // Count the number of execution for each reference flow (site + category)

    // For the current Runs
    QMap<int,int>       mTotalTestsExecuted;
    QMap<int,int>       mTotalNewTests;
    QMap<int,int>       mTotalInconsistentTests;
    QMap<int,QString>   mLastInconsistentTestNumber;
    QMap<int,QString>   mLastInconsistentTestName;

    // Auto-increment Test map (used with auto-increment test number only)
    QMap<QString, qint64>  mAutoIncrementTestMap;
    QMap<int, qint64>      mAIDummyMaxTestNumber;

    // list of all test number encountered in the STDF file associated with a test name
    // PTR are associated with the test number, the test name is optional
    // If we have to ignore the test number, we must retrieve for each records (test number)
    // the original test name
    // <PTR><test number> = <test name>
    QMap<QString,QString>  mAssociatedTestNames;

    // Specifies if TestNumber should be ignored (auto increment)
    // This option allow duplicated test number only is the TestName always presents in each records
    // When this option is true, use the original TestNumber and the original TestName to find test in TestInfo list
    // With this option, we have to force mAllowedDuplicateTestNumbers to true
    bool        mAutoincrementTestNumbers;

    // The Merge by TestNumber will ignore the TestName to agregate results to each specific TestNumber
    bool        mMergeByTestNumber;
    // The Merge by TestName will ignore the TestNumber to agregate results to each specific TestName
    bool        mMergeByTestName;

    // Specifies if duplicate test number are allowed to be inserted
    bool        mAllowedDuplicateTestNumbers;

    // Define if SequencerName should be removed or not, when the SequencerName appears into the TestName
    bool        m_bRemoveSequencerName;
    bool        m_bRemovePinName;
    bool        mIgnoreDtrTestConditions;
    int         mDtrTestConditionsCount;

    // case 6390 : timout (in ms) to force the SQL insertion even if test resuts not fully read in order to keep sql connection alive
    int         mForceSqlInsertionTimeout;

    // SQL OPTIMIZATION : merge query results for each run or a lot of runs
    // contains
    // for partition <table_name|active_partition , partition_name|MAXVALUE>,
    // for partition <table_name|last_partition , partition_name|MAXVALUE>,
    // for partition <table_name|last_tablespace , tablespace_name>,
    // for partition <table_name|partition_name , partition_position|MAXVALUE>,
    // for table_sequence <table_name,auto_increment>,
    // for table columns <table_name, columns>
    QMap<QString,QString> m_mapTablesDesc;
    // "ft", "wt", "et" for FinalTest, WaferTest or ElectTest
    QString     m_strPrefixTable;
    // "GexDB.\"" for Oracle
    QString     m_strPrefixDB;
    // "\"" for Oracle
    QString     m_strSuffixDB;
    int         m_nMaxPacketSize;
    // Total Run processed
    int         m_nNbRunProcessed;
    // Total Run in the current m_strQueryRun
    int         m_nNbRunInQuery;
    // current m_strQueryRun
    QString     m_strQueryRun;

    // all PTest results for run in current m_strQueryRun
    QString     m_strPTestResults;
    QString     m_strPTestResultsTableColumns;
    // all FTest results for run in current m_strQueryRun
    QString     m_strFTestResults;
    QString     m_strFTestResultsTableColumns;
    // all MPTest results for run in current m_strQueryRun
    QString     m_strMPTestResults;
    QString     m_strMPTestResultsTableColumns;

    int         m_nNbPTestResults;
    int         m_nNbFTestResults;
    int         m_nNbMPTestResults;

    bool        m_bPTestResultsBySqlLoader;
    bool        m_bMPTestResultsBySqlLoader;
    bool        m_bFTestResultsBySqlLoader;
    QFile       m_clPTestResultsDataFile;
    QFile       m_clMPTestResultsDataFile;
    QFile       m_clFTestResultsDataFile;

    // for Load Data Infile : decimal separator ',' or '.'
    QString     m_strLoadDataInfileDecSepValues;
    // for Load Data Infile : NULL value is '' or '\N'
    QString     m_strLoadDataInfileNullValues;
    QString     m_strLoadDataInfileBeginValues;
    QString     m_strLoadDataInfileSepValues;
    QString     m_strLoadDataInfileEndValues;

    QString     mQuerySamplesPTest;
    QString     mQuerySamplesFTest;
    QString     mQuerySamplesMPTest;
    QString     mQuerySummaryPTest;
    QString     mQuerySummaryFTest;
    QString     mQuerySummaryMPTest;
    QString     mQueryPTestLimits;
    QString     mQueryPTestStaticLimits;
    QString     mQueryMPTestLimits;
    QString     mQueryMPTestStaticLimits;

    // STDF analyses
    // Array of records to display
    int         m_nStdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_COUNT];
    GQTL_STDF::Stdf_FAR_V4            m_clStdfFAR;
    GQTL_STDF::Stdf_VUR_V4            m_clStdfVUR;
    GQTL_STDF::Stdf_ATR_V4            m_clStdfATR;
    GQTL_STDF::Stdf_MIR_V4            m_clStdfMIR;
    GQTL_STDF::Stdf_MRR_V4            m_clStdfMRR;
    GQTL_STDF::Stdf_PCR_V4            m_clStdfPCR;
    GQTL_STDF::Stdf_HBR_V4            m_clStdfHBR;
    GQTL_STDF::Stdf_SBR_V4            m_clStdfSBR;
    GQTL_STDF::Stdf_PMR_V4            m_clStdfPMR;
    GQTL_STDF::Stdf_PGR_V4            m_clStdfPGR;
    GQTL_STDF::Stdf_PLR_V4            m_clStdfPLR;
    GQTL_STDF::Stdf_RDR_V4            m_clStdfRDR;
    GQTL_STDF::Stdf_SDR_V4            m_clStdfSDR;
    GQTL_STDF::Stdf_WIR_V4            m_clStdfWIR;
    GQTL_STDF::Stdf_WRR_V4            m_clStdfWRR;
    GQTL_STDF::Stdf_WCR_V4            m_clStdfWCR;
    GQTL_STDF::Stdf_PIR_V4            m_clStdfPIR;
    GQTL_STDF::Stdf_PRR_V4            m_clStdfPRR;
    GQTL_STDF::Stdf_TSR_V4            m_clStdfTSR;
    GQTL_STDF::Stdf_PTR_V4            m_clStdfPTR;
    GQTL_STDF::Stdf_MPR_V4            m_clStdfMPR;
    GQTL_STDF::Stdf_FTR_V4            m_clStdfFTR;
    GQTL_STDF::Stdf_BPS_V4            m_clStdfBPS;
    GQTL_STDF::Stdf_EPS_V4            m_clStdfEPS;
    GQTL_STDF::Stdf_GDR_V4            m_clStdfGDR;
    GQTL_STDF::Stdf_DTR_V4            m_clStdfDTR;
    GQTL_STDF::Stdf_RESERVED_IMAGE_V4 m_clStdfRESERVED_IMAGE;
    GQTL_STDF::Stdf_RESERVED_IG900_V4 m_clStdfRESERVED_IG900;
    GQTL_STDF::Stdf_UNKNOWN_V4        m_clStdfUNKNOWN;

    /////////////////////////////////////////////////
    // FUNCTIONS DEFINITION
    /////////////////////////////////////////////////
    // Insert Stdf File into the current GALAXY database
    bool    InsertStdfFile(const QString & strStdfFileName, int nWaferIndexToProcess=1);

    // Validation function
    // here all verification before insert data in the GALAXY database
    //////////////////////////////////////////////////////////////////////
    // to verify if the stdf file is already inserted in the DB
    // have to call this function at the first PIR parsed with bOnlyHeaderValidation = true
    //////////////////////////////////////////////////////////////////////
    // to verify more validity
    // call this function after the first parse end
    /////////////////////////////////////////////////////////////////////
    // 3 CALLS
    // * Pass1 - after the first PIR :
    //    - to set the TestingStage
    //    - to check if have one MIR
    //    - to check DateTime
    // * Pass1 - all records read and stored procedures called:
    //    - to check DateTime for multi-wafer
    //    - to check WaferId for multi-wafer
    //    - to check WIR/WRR count
    //    - to check WaferId into WIR and WRR
    // * Pass2 - after the MIR
    //    - to check Product
    //    - to check Lot
    //    - to check WaferId
    //    - to check already inserted
    bool    ValidationFunction(bool bOnlyHeaderValidation=false);

    // to display or save warning message
    void    WarningMessage(QString strMessage);
    // to display or save error message
    void    ErrorMessage(QString strMessage);
    // Update progress bar (if m_pProgressBar not null)
    bool    UpdateInsertionProgress(int nProg=0);
    bool    InitStdfFields();
    bool    Clear(bool bFirstWafer = false);
    void    CleanInvalidSplitlot();

    bool    IsTestFail(const GQTL_STDF::Stdf_PTR_V4 &clRecord, structTestInfo* pTest);
    bool    IsTestFail(const GQTL_STDF::Stdf_FTR_V4 &clRecord, structTestInfo* pTest);
    bool    IsTestFail(const GQTL_STDF::Stdf_MPR_V4 &clRecord, structTestInfo* pTest);

    bool    IsTestResultInvalid(const GQTL_STDF::Stdf_PTR_V4 &clRecord);
    bool    IsTestResultInvalid(const GQTL_STDF::Stdf_FTR_V4 &clRecord);
    bool    IsTestResultInvalid(const GQTL_STDF::Stdf_MPR_V4 &clRecord);

    bool    IsTestNotExecuted(const GQTL_STDF::Stdf_PTR_V4 &clRecord);
    bool    IsTestNotExecuted(const GQTL_STDF::Stdf_FTR_V4 &clRecord);
    bool    IsTestNotExecuted(const GQTL_STDF::Stdf_MPR_V4 &clRecord);

    // Update variables abd list information (bin, test)
    bool    UpdatePCRInfo(const GQTL_STDF::Stdf_PCR_V4 &clRecord);

    bool    UpdateHBinInfo(const GQTL_STDF::Stdf_HBR_V4 &clRecord);
    bool    UpdateHBinInfo(const GQTL_STDF::Stdf_PRR_V4 &clRecord);
    bool    UpdateSBinInfo(const GQTL_STDF::Stdf_SBR_V4 &clRecord);
    bool    UpdateSBinInfo(const GQTL_STDF::Stdf_PRR_V4 &clRecord);

    bool    UpdateTestInfo(const GQTL_STDF::Stdf_PTR_V4 &clRecord);
    bool    UpdateTestInfo(const GQTL_STDF::Stdf_FTR_V4 &clRecord);
    bool    UpdateTestInfo(const GQTL_STDF::Stdf_MPR_V4 &clRecord);
    // Create new TestInfo pointer
    structTestInfo* CreateTestInfo(int nTestType, qint64 nTestNumber, qint64 nOriginalTestNumber,
                                           QString lTestName, QString lOriginalTestName,
                                           int nTestPinIndex=-1, int nTestPin=-1);
    bool MatchingTestInfo(structTestInfo* pTest, int nTestType, qint64 nTestNumber,
                                           QString lTestName, int nTestPinIndex);
    structTestInfo* FindTestInfo(int nTestType, qint64 nTestNumber, QString lTestName="", int nTestPinIndex=-1);
    // if test like "test_name <> seq_name" return "test_name"
    QString NormalizeTestName(const QString &lTestName);
    // if fValue is inf return "1e+38" (QString::number() return "inf") , bUseLocalFormat for Data File
    QString NormalizeNumber(const float &fValue, QString strDecSepValue=".");
    // for MySQL table = ft_table, for Oracle table = GexDB."ft_table"
    QString NormalizeTableName(const QString &strValue, bool bAddPrefixTable=true);
    // for MySQL table = ft_table, for Oracle table = GexDB."ft_table"
    QString NormalizeTableName(const QString &strValue, const QString &strPrefixTable, bool bAddPrefixTable=true);
    // for MySQL and Oracle table = LocalPath + FileName
    QString NormalizeFileName(const QString &strFile, const QString strExt);
    // for MySql and Oracle partition (use the old or the new convention)
    QString GetPartitionName(int *LimitSequence=NULL);

    // translate string value for SQL query (ex: ' is not supported in a field)
    QString TranslateStringToSqlVarChar(const QString &strValue, bool bAddQuote=true);
    // apply a sql date format to translate a time
    QString TranslateUnixTimeStampToSqlDateTime(const QString strUnixTimeStamp, enum SqlUnixDateFormat eFormat, enum SqlUnixDateFormat eConcatFormat=eEmpty);

    bool    InitLoadDataInfile(GQTL_STDF::Stdf_Record::RecordTypes nRecordType);
    bool    AddInLoadDataInfile(GQTL_STDF::Stdf_Record::RecordTypes nRecordType, QString &strAllValues);
    bool    ExecuteLoadDataInfile(GQTL_STDF::Stdf_Record::RecordTypes nRecordType);
    // called by ExecuteLoadDataInfile() if SQLite
    bool    ExecuteLoadDataInfile_SQLite(QString* pstrTestResults, int* pNbResults);

    bool    AddInMultiInsertQuery(const QString & strTableName,
                                  QString &strHeader,
                                  QString &strNewValue,
                                  QString &strAllValues);
    bool    ExecuteMultiInsertQuery(const QString & strTableName, const QString & strQuery);
    // special sqlite version. Called by ExecuteMultiInsertQuery().
    bool    ExecuteMultiInsertQuery_SQLite(const QString & strTableName, const QString &strQuery);

    bool    ExecuteSqlScript(const QString & fileName);

    // add new partition in result table for each splitlot_id
    bool    UpdateTablespacePartitionsSequence();
    // CALL SUB-FUNCTIONS
    bool    UpdateTablespacePartitionsSequenceBy();
    bool    CheckAndUpdateTablespace(QString& strWeek, QString &strTablespaceName);
    bool    CheckAndUpdateSequence(int nFirstSplitlotSequence);

    // AutoIncrement SplitlotId Maintenance
    bool    GetTemporarySequenceIndex();

    // when encountered an STDF record
    bool    ProcessVUR();
    bool    ProcessMIR();
    bool    ProcessMRR();
    bool    ProcessPCR();
    bool    ProcessHBR();
    bool    ProcessSBR();
    bool    ProcessRDR();
    bool    ProcessSDR();
    bool    ProcessWIR();
    bool    ProcessWRR();
    bool    ProcessWCR();
    bool    ProcessPIR();
    bool    ProcessPRR();
    bool    ProcessTSR();
    bool    ProcessPTR();
    bool    ProcessMPR();
    bool    ProcessPMR();
    bool    ProcessFTR();
    bool    ProcessDTR();


    // ONLY FOR DATABASE UPDATE
    bool    UpdateWaferInfoTable_B6();

    // MULTI INSERTION MULTI SQL_THREAD
    // SplitLot_ID used for reference
    int     m_nSplitLotId;
    // SplitLot_ID used for MULTI INSERTION MULTI SQL_THREAD
    int     m_nTemporarySplitLotId;
    // SID used for this insertion
    int     m_nInsertionSID;
    // HostName used for this insertion
    QString m_strInsertionHostName;
    // Check if a specific SID is active
    bool IsSessionIdActive(QString & SessionId);

    // SPLITLOT INSERTION
    bool    StartSplitlotInsertion(bool bTemporarySplitlot=false );
    // with temporary splitlot_id for validation_function and MULTI INSERTION MULTI SQL_THREAD
    // with good splitlot_id for data insertion
    bool    StopSplitlotInsertion();
    // delete temporary splitlot_id and validate current splitlot_id for MULTI INSERTION MULTI SQL_THREAD

    // Call the preprocessing stored procedure
    bool    PreprocessingProcedure(int lSplitlotId);

    // GexDbKeys access for Stored Procedures
    bool    KeysContentDumpToTempTable();
    bool    KeysContentUpdateFromTempTable(int SplitlotId);
    QString KeysContentToSplitlotColumn(const QString &lKey);

    // TOKEN METHOD
    bool InitTokens(QString Name=QString());
    bool GetToken(QString Name, QString Key, int Time);
    void ReleaseToken(QString Name, QString Key);
    void ReleaseTokens();

    // CONSOLIDATION PHASE
    // Use transaction for MySql/InnoDb and Oracle
    bool    StartTransactionMode(QString strQueryForUpdate);
    // Check if AUTOCOMMIT is OFF (for transaction)
    bool    StopTransactionMode(bool bCommit=true);
    // Check if AUTOCOMMIT is ON (for non transaction)
    // Use Lock function session for MySql/MyISAM
    QString m_strCurrentLockSession;
    // Locked session
    bool    GetLock(QString strLockName);
    // Lock session for MySql
    bool    ReleaseLock(bool bForce=false);
    // Unlock session for MySql

    // SEQUENCER/PARTITION/TABLESPACE UPDATE
    // Lock exclusive lock to stop all other thread during this phase
    bool    LockTables(QString strTableName, bool bRowMode);
    // Lock table for Oracle or MySql
    bool    UnlockTables();
    // Unlock tables for Oracle or MySql

    QMap<QString,int> mTimersTrace;
    bool TimersTraceReset(const QString &TimerName);
    bool TimersTraceStart(const QString &TimerName);
    bool TimersTraceStop(const QString &TimerName);
    bool TimersTraceDump(const QString &TimerName);

    // TestConditions definition update
    bool    checkMetaDataForTestConditions();
    QString getTestConditionColumn(const QString& testConditionName) const;

    // SQL Query for update tables in GALAXY database
    bool    UpdateProductTable(QString &ProductName);
    bool    UpdateProductConsolidation(QString &ProductName);
    bool    UpdateFtDieTrackingTable();
    bool    UpdateFileHostTable();
    // GCORE-187 - SubLot consolidation
    bool    UpdateSubLotInfoTable();
    bool    UpdateSubLotConsolidation(const QString &LotId, const QString &SubLotId);
    bool    UpdateSubLotBinningConsolidationTable(const QString &LotId, const QString &SubLotId, const QString &ProdData, QString &PhysicalConsolidationName, QString &ConsolidationSummary);
    bool    UpdateSubLotRunConsolidationTable(const QString &LotId, const QString &SubLotId, const QString &ProdData, QStringList &lstPhases, QMap<QString, QMap<int,QStringList> > &mapPhaseRetestHBins, QMap<QString, int> &mapPhaseMissingParts);
    bool    UpdateLotTable();
    bool    UpdateLotConsolidation(const QString &LotId);

    bool    UpdateSplitLotTable(bool bTemporaryInsertion=false );
    // return the next usable splitlot_id, -1 if error
    int     GetNextSplitlotIndex_SQLite();
    bool    UpdateWaferInfoTable();
    bool    UpdateWaferConsolidation(const QString &LotId, const QString &WaferId);
    bool    UpdateWaferBinningConsolidationTable(const QString &LotId, const QString &WaferId, const QString &ProdData, QString &PhysicalConsolidationName, QString &ConsolidationSummary);
    bool    UpdateWaferRunConsolidationTable(const QString &LotId, const QString &WaferId, const QString &ProdData, QString &PhysicalConsolidationName, QString &ConsolidationSummary);
    bool    UpdatePartsStatsSamplesTable();
    bool    UpdatePartsStatsSummaryTable();
    bool    UpdateHBinTable();
    bool    UpdateSBinTable();
    bool    UpdateHBinStatsSamplesTable();
    bool    UpdateHBinStatsSummaryTable();
    bool    UpdateSBinStatsSamplesTable();
    bool    UpdateSBinStatsSummaryTable();
    bool    UpdatePMRInfoTable();
    bool    UpdateRunTable();
    bool    UpdateRunTablePartFlags(long lSplitlotID=0);
    bool    UpdatePTestInfoTable();
    bool    UpdatePTestLimitsTable(structTestInfo *pTest);
    bool    UpdatePTestStaticLimitsTable(structTestInfo &cTest);
    bool    UpdateMPTestInfoTable();
    bool    UpdateMPTestLimitsTable(structTestInfo *pTest);
    bool    UpdateMPTestStaticLimitsTable(structTestInfo &cTest);
    bool    UpdateFTestInfoTable();
    bool    UpdatePTestStatsSummaryTable(structTestInfo *pTest);
    bool    UpdatePTestStatsSamplesTable(structTestInfo *pTest);
    bool    UpdateMPTestStatsSummaryTable(structTestInfo *pTest);
    bool    UpdateMPTestStatsSamplesTable(structTestInfo *pTest);
    bool    UpdateFTestStatsSummaryTable(structTestInfo *pTest);
    bool    UpdateFTestStatsSamplesTable(structTestInfo *pTest);
    bool    UpdatePTestResultsTable();
    bool    UpdateMPTestResultsTable();
    bool    UpdateFTestResultsTable();
    bool    UpdateTestConditionTable();
    bool    InsertSplitlotDataIntoGexdb(const QString & strTableName, const QString & strInsertQuery);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // For Weekly Week Report
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    QString        m_strSiteName;
    unsigned int   m_uiWeekNb;
    unsigned int   m_uiYear;

    bool    InsertWyrFile(const QString & strDataFileName);
    bool    ReadNextWyrLine(QTextStream& hFile, QString & strLine);
    bool    CheckWyrTables();
    QString ExtractFieldValue(
            char* szDataType, QStringList& lstField,
            QMap<int,QString>    &mapColNb_DataType,
            QMap<int,int> &mapColNb_ColId,
            bool bSplitDataType=false, bool bNumericalSum=false);


private :

   bool UpdateDatabase();

    void buildQueryForFTConsolidateProcedures(QString &strQuery,  QString &strSelectQuery, const QString &strTableName);
    void buildQueryForWConsolidateProcedures(QString &strQuery,  QString &strSelectQuery, const QString &strTableName);

    /// \brief Rteurn true if the database doesn't contain any test_insertion. (Have to be deleted before the 7.4)
    bool isDataBaseContainRetestPhase();

    // Return all valid Data files when Consolidated data (given filters on several fields)
    bool    QueryDataFilesConsolidatedForWaferSort(GexDbPlugin_Filter & cFilters,
                                                   GexDbPlugin_Galaxy_TestFilter & clTestFilter,
                                                   tdGexDbPluginDataFileList & cMatchingFiles,
                                                   const QString & strDatabasePhysicalPath,
                                                   const QString & strLocalDir,
                                                   bool *pbFilesCreatedInFinalLocation,
                                                   GexDbPlugin_Base::StatsSource eStatsSource,
                                                   unsigned int    uiGexDbBuild);

    // Return all valid Data files when Consolidated data (given filters on several fields)
    bool    QueryDataFilesConsolidatedForFinalTest(GexDbPlugin_Filter & cFilters,
                                                   GexDbPlugin_Galaxy_TestFilter & clTestFilter,
                                                   tdGexDbPluginDataFileList & cMatchingFiles,
                                                   const QString & strLocalDir);
    // Retrieve retest indexes and hbins for the given lot/sublot
    bool    QueryRetestMapForLotSublot(const QPair<QString,QString> &lotSublotName,
                                       QMap<int, QStringList> &retestIndexOfSublot);
    // Retrieve all HBIN_NO for given HBINS string ('all', 'all_fail', 'all_pass', '5-8',...)
    QString QueryHBIN_NOListForLotSublotAndHBINS(const QPair<QString, QString> &lotSublot,
                                                 const QString& hbins,
                                                 QVector<int> &v);
    // Retrieve list of splitlots for the given lot and retest_index
    bool    QuerySplitlotsList(const QString& lot, const int& ri,
                               QList<int>& splitlots);
    // Retrieve all test conditions corresponding to the splitlots (according to given filters)
    bool    QueryTestConditionsList(GexDbPlugin_Filter & cFilters,
                                    QStringList & matchingTestConditions);
    // Retrieve distinct list of lotwafers for wafer extraction (order by lotid, waferid, start_t)
    bool    RetrieveDistinctLotsWafersList(GexDbPlugin_Filter &cFilters,
                                           QMap<QString,
                                           QList< QPair<int, QString> > > &lotswafernbs);

    // writeMIR,SDR,DTR,WCR,WIR and keep the stdf opened ! The GQTL_STDF::StdfParse must be already opened !
    bool    BeginStdf(GQTL_STDF::StdfParse &f,
                      GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
                      GexDbPlugin_Galaxy_TestFilter & clTestFilter,
                      const QString & strLocalDir,
                      const QString &desired_notch);

    // write  WRR and MRR. Dont write summary records. Dont close the stdf.
    bool    EndStdf(GQTL_STDF::StdfParse &f,
                    GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
                    GexDbPlugin_Filter &cFilters,
                    GexDbPlugin_Galaxy_TestFilter & clTestFilter,
                    const QString & strLocalDir,
                    const QString& desired_notch);

    // Create 1 (only 1) stdf for the given lotwafer.
    bool CreateStdfForLotWafer(QString lotid, QString waferid,
                               GexDbPlugin_Filter &cFilters,
                               GexDbPlugin_Galaxy_TestFilter & clTestFilter,
                               const QString & strLocalDir,
                               QString &stdfFN, unsigned int    uiGexDbBuild);

    /// \brief Update the database and set it to the right type with a hash s key
    /// \param dbType requested type
    /// \param adminConnector connector to the database
    /// \param sqlQuery object to query the database
    bool    UpdateDbSetTdrType(const QString &dbName, const GexDbPlugin_Galaxy::DataBaseType &dbType,
                               QSqlQuery *sqlQuery);

    bool CheckJob(const QString& jobTitle,
                  const QString& testingStage,
                  const QString& lot,
                  const QStringList& sublots,
                  const QStringList& wafers,
                  const QString& consoType,
                  const QString& testFlow,
                  const QString& consoLevel,
                  const QString& testInsertion);

    ConsolidationCenter* mConsolidationCenter;

    // for MySql and Oracle partition (use Granularity)
    QMap<QString,QString> GetPartitionNameFromGranularity(QString &Granularity,
                                                          UINT MinSequence,
                                                          UINT MaxSequence,
                                                          UINT CurrentSequence);

    // For new options
    QMap<QString,QVariant> mInsertionOptions;

};


///////////////////////////////////////////////////////////
// Query Thread for progressBar
#define QUERYTHREAD_TYPE_DEFAULT        0
#define QUERYTHREAD_TYPE_PARTITIONING   1
#define QUERYTHREAD_TYPE_BARRACUDA      3
#define QUERYTHREAD_TYPE_INNODB         4

class QSqlQuery;
class QueryThread : public QThread
{
    Q_OBJECT

public slots:
    bool exec(QSqlQuery *pQuery,
              int nType=QUERYTHREAD_TYPE_DEFAULT,
              QString strDir=QString(""),
              QString strFile=QString(""));

signals:
    // Signal for progressDialog control
    void progressValue (const int);
    void newText(const QString&);
    void showDlg();

protected:
    virtual void run();

private:
    bool        m_bError;
    QSqlQuery  *m_pQuery;
};





class CSVConditionDumper{
    static QString m_strTempFileSep;
    QFile m_oFile;
    QTextStream m_oStream;

    QStringList m_oConditionHeader;
    QStringList m_oTestHeader;

    QMap<QString, QString> m_oTestsGroup;
    bool m_bFirstEntry;
    QProgressDialog *m_poProgress;
public:


    CSVConditionDumper(){
        m_poProgress = 0;
        m_bFirstEntry = true;

    }
    ~CSVConditionDumper(){
        clear();

    }
    void setProgress(QProgressDialog *poProgress){
        m_poProgress = poProgress;
    }
    bool cancel(){
        if(m_poProgress){
            m_poProgress->wasCanceled();
        }
        return false;
    }

    void clear();
    bool create(const QString &strFileName);

    int addConditionHeader(const QString &strCondition);
    int addTestHeader(const QString &strTest);
    int groupCount() const;
    QString addGroup(const QString &strGroup);
    bool createTempGroupFile(const QString &strGroup, const QMap<QString, QString> &oData);
    bool dumpTempGroupFile (const QStringList &oValuesOrder);

    bool dumpSQL(const QString &strQuery);

    static bool entryToStringList(GexDbPlugin_Query *poObj, QStringList& oTestEntries);

};



#endif // _GEXDB_PLUGIN_GALAXY_HEADER_
