// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

#ifndef _GEXDB_PLUGIN_MEDTRONIC_HEADER_
#define _GEXDB_PLUGIN_MEDTRONIC_HEADER_

// Standard includes

// Qt includes

// Galaxy modules includes
#include <stdf.h>
#include <stdfparse.h>
#include <qprocess.h>

// Local includes
#include "gexdb_plugin_common.h"
#include "gexdb_plugin_medtronic_options.h"
#include "statistical_monitoring_monitored_item_unique_key_rule.h"



// Plugin name
#define GEXDB_PLUGIN_MEDTRONIC_NAME         "Medtronic SQL Database [Plugin V1.1 B7]"
#define GEXDB_PLUGIN_MEDTRONIC_BUILD        7

// Name of supported testing stages
#define GEXDB_PLUGIN_MEDTRONIC_WTEST        "Wafer Sort"
#define GEXDB_PLUGIN_MEDTRONIC_FTEST        "Final Test"

#define FLAG_TESTINFO_LL_STRICT            	0x01
#define FLAG_TESTINFO_HL_STRICT            	0x02
#define FLAG_TESTINFO_LL_INVALID            0x04
#define FLAG_TESTINFO_HL_INVALID            0x08

extern "C" GEXDB_PLUGIN_API GexDbPlugin_Base *gexdb_plugin_getobject(
        const QString & strHostName,
        const QString & strApplicationPath,
        const QString & strUserProfile,
        const QString & strLocalFolder,
        const char *gexLabelFilterChoices[],
        CGexSkin * pGexSkin,
        const GexScriptEngine *gse,
        const bool bCustomerDebugMode);
extern "C" GEXDB_PLUGIN_API void gexdb_plugin_releaseobject(GexDbPlugin_Base *pObject);

class GexDbPlugin_Medtronic_Rdb_OptionsWidget;

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_Medtronic_SplitlotInfo
// Holds info on 1 splitlot
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_Medtronic_SplitlotInfo
{
public:
    GexDbPlugin_Medtronic_SplitlotInfo()
    {
        m_uiNbParts = 0;
        m_uiNbParts_Good = 0;
    }

    // Fields from Medtronic DB (unit_test table)
    QString             m_strUnitModel;                 // Unit_Model
    QString             m_strUnitType;                  // Unit_Type
    QString             m_strUnitLot;                   // Unit_Lot
    QString             m_strSublot;                    // Sublot (WaferID or SublotID)
    QString             m_strUnitPin;                   // Unit_Pin
    QString             m_strTestPurpose;               // Test_Purpose
    QString             m_strTestOperation;             // Test_Operation
    QString             m_strTestFacility;              // Test_Facility
    QString             m_strTestSetID;                 // Test_Set_ID
    QString             m_strTestPackageID;             // Test_Package_ID
    QString             m_strTestOperatorID;            // Test_Operator_ID
    QString             m_strStationSerial;             // Station_Serial
    QString             m_strStationPin;                // Station_PIN
    QString             m_strStationModel;              // Station_Model
    QString             m_strStationSoftwareRev;        // Station_Software_Rev
    QDateTime           m_clMinTestDate;                // Minimum test date
    QDateTime           m_clMaxTestDate;                // Maximum test date
    QString             m_strTransferDate;              // Transfer_Date
    QString             m_strFirstTestFlag;             // First_Test_Flag
    QString             m_strLastTestFlag;              // Last_Test_Flag

    // Fields computed during extraction
    unsigned int        m_uiNbParts;
    unsigned int        m_uiNbParts_Good;
};


///////////////////////////////////////////////////////////
// GexDbPlugin_Medtronic class: database plugin class for MDTRONIC database types
///////////////////////////////////////////////////////////
class GexDbPlugin_Medtronic : public GexDbPlugin_Base
{
    // Constructor / Destructor
public:
    GexDbPlugin_Medtronic(const QString & strHostName,
                          const QString & strApplicationPath,
                          const QString & strUserProfile,
                          const QString & strLocalFolder,
                          const char *gexLabelFilterChoices[],
                          const bool bCustomerDebugMode,
                          CGexSkin * pGexSkin,
                          const GexScriptEngine *gse,
                          GexDbPlugin_Connector *pclDatabaseConnector=NULL);
    virtual ~GexDbPlugin_Medtronic();

    // Member functions
public:
    // Common functions
    bool            Init();
    void            GetPluginName(QString & strPluginName) const { strPluginName = GEXDB_PLUGIN_MEDTRONIC_NAME; }
    void            GetTdrTypeName(QString & strTdrType) {strTdrType = QString(GEXDB_PLUGIN_MEDTRONIC_NAME).section("[",0,0).simplified(); }
    unsigned int    GetPluginBuild(void) { return GEXDB_PLUGIN_MEDTRONIC_BUILD; }
    void            GetSupportedTestingStages(QString& /*strSupportedTestingStages*/)	{ return; }
    bool            SetTestingStage(const QString& /*strTestingStage*/) { return true; }
    int             GetTestingStageEnum(const QString & /*strTestingStage*/) { return 0; }
    bool            IsInsertionSupported() { return false; }
    bool            IsUpdateSupported() { return false; }
    bool            IsTestingStagesSupported() { return false; }
    bool            IsParameterSelectionSupported() { return true; }
    bool            IsEnterpriseReportsSupported() { return false; }
    bool            IsReportsCenterSupported() { return false; }
    bool            IsTestingStage_FinalTest(const QString& /*strTestingStage*/) { return false; }
    bool            IsTestingStage_Foundry(const QString& /*strTestingStage*/) { return false; }
    bool            GetTestingStageName_Foundry(QString & strTestingStage) { strTestingStage = ""; return true; }
    void            GetLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);
    void            GetConsolidatedLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);
    QWidget        *GetRdbOptionsWidget();                                                // Returns a pointer on an options GUI
    bool            GetRdbOptionsString(QString & strRdBOptionsString);            		// Returns an options string, if the plugin supports custom options
    bool            SetRdbOptionsString(const QString & strRdBOptionsString);            // Sets the plug-in options GUI according to options string passed as argument

    // Plugin configuration
    bool            ConfigWizard();
    bool            LoadMetaData();

    // Data extraction for GEX standard reports
    bool            QueryField(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bSoftBin=false,bool bDistinct=true, QuerySort eQuerySort=eQuerySort_Asc);		// Return all valid values for a field (given filters on other fields)
    bool            QueryTestlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues, bool bParametricOnly);		// Return all tests (given filters on other fields)
    bool            QueryDataFiles(GexDbPlugin_Filter & cFilters, const QString & strTestlist, tdGexDbPluginDataFileList & cMatchingFiles, const QString & strDatabasePhysicalPath, const QString & strLocalDir, bool *pbFilesCreatedInFinalLocation, GexDbPlugin_Base::StatsSource eStatsSource);	// Return all valid Data files (given filters on several fields)

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
                                                  const QMap<QString, QString> &filtersMetaData, //in
                                                  QStringList& waferKeyList); //out

    bool            SPM_GetConditionsFromFilters(QString testingStage, //in
                                                 const QMap<QString,QString>& filtersMetaData, //in
                                                 QMap<QString,QStringList>& filtersConditions); //out

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
                                                         MonitoredItemUniqueKeyRule uniqueKeyRule, //in
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
                                                           MonitoredItemUniqueKeyRule uniqueKeyRule, //in
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
                                                         MonitoredItemUniqueKeyRule uniqueKeyRule, //in
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

    int IsConsolidationInProgress(
                QString /*testingStage*/,
                QString /*lot*/,
                QString /*sublots*/,
                QString /*wafers*/,
                QString /*consoType*/,
                QString /*testFlow*/,
                QString /*consoLevel*/,
                QString /*testInsertion*/) {return 2;}

    // Member variables
public:

    // Implementation
protected:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // For Extraction
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////
    // VARIABLES DEFINITION
    /////////////////////////////////////////////////
    // Database mapping
    GexDbPlugin_Mapping_FieldMap            m_mapFields_GexToRemote_Lookup;	// Fields mapping: GEX <-> Custom DB (E-Test, Wafer Sot, Final Test) lookup tables
    GexDbPlugin_Mapping_LinkMap            	m_mapLinks_Remote_Lookup;		// Table links mapping (E-Test, Wafer Sort, Final Test) lookup tables
    GexDbPlugin_Mapping_FieldMap            m_mapFields_GexToRemote;		// Fields mapping: GEX <-> Custom DB (E-Test, Wafer Sort, Final Test)
    GexDbPlugin_Mapping_LinkMap            	m_mapLinks_Remote;            	// Table links mapping (E-Test, Wafer Sort, Final Test)
    QStringList                        		m_strlLabelFilterChoices;

    // Other
    QString                                    m_strDebugString;
    QMap<long, GexDbPlugin_RunInfo>            m_mapRunInfo;            	// RUN info map (key is the Ut_Id)
    unsigned int                        	m_uiSplitlotNbRuns;            // Nb of runs for on Splitlot
    GexDbPlugin_TestInfoList            	m_clTestList;            	// Testlist for extraction
    QString                                    m_strFirstTestFlag;            // Value of 'first_test_flag' (or 'X' if no filter applies)
    QString                                    m_strLastTestFlag;            // Value of 'last_test_flag' (or 'X' if no filter applies)
    GexDbPlugin_Medtronic_Rdb_OptionsWidget	*m_pclRdbOptionsWidget;		// Widget with Medtronic-custom options GUI
    GexDbPlugin_Medtronic_Options            m_clOptions;

    /////////////////////////////////////////////////
    // FUNCTIONS DEFINITION
    /////////////////////////////////////////////////
    // Init variables used for data extraction
    void	Init_Extraction();
    int		GetNbOfSplitlots(GexDbPlugin_Filter &cFilters);
    bool	ConstructSplitlotQuery(GexDbPlugin_Filter &cFilters, QString & strQuery);	// Construct query to retrieve all Splitlots corresponding to specified filters
    void	FillSplitlotInfo(const GexDbPlugin_Query & clGexDbQuerySplitlot, GexDbPlugin_Medtronic_SplitlotInfo	*pclSplitlotInfo);	// Fill Galaxy SplitlotInfo object with query result
    bool	CreateStdfFile(GexDbPlugin_Filter & cFilters, const GexDbPlugin_Query & clGexDbQuerySplitlot, const QString & strTestlist, const QString & strDatabasePhysicalPath, const QString & strLocalDir, QString & strStdfFileName, QString & strStdfFilePath);	// Create a STDF file for specified Splitlot_id
    bool	InitMaps(GexDbPlugin_Filter &cFilters, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo);
    bool	CreateTestlist(GexDbPlugin_Filter &cFilters, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo, const QString & strTestlist);
    bool	WriteMIR(GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo);
    bool	WriteMRR(GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo);
    bool	WriteStaticTestInfo(GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo);
    bool	WriteTestResults(GexDbPlugin_Filter &cFilters, GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo, const QString & strTestlist);
    bool	WritePIR(GQTL_STDF::StdfParse & clStdfParse, int nSiteNum);
    bool	WritePRR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_RunInfo & clCurrentPart);
    bool	WriteWIR(GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo);
    bool	WriteWRR(GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo);
    bool	Query_AddValueCondition(const QString & strQueryFilter, const QString & strQueryValue, bool bExpressionRange=false, bool bUseLinkConditions=true);
    bool	Query_AddField(const QString & strQueryField, QString & strDbField, QString & strDbTable, bool bUseLinkConditions=true, bool bConsolidated=false);                        	// Add a select field to the query
    bool	Query_AddFilters(GexDbPlugin_Filter &cFilters);                                                                        // Add filters to current query
    void	Query_AddTestlistCondition(const QString & strTestlist);
    bool	ConstructTestlistQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, QString & strTestNumField, QString & strTestNameField, QString strTestTypeField = "");	// Construct query string to retrieve all tests matching filters
    bool	Query_AddTimePeriodCondition(GexDbPlugin_Filter &cFilters);            	// Add condition if user selected a time period
    bool	Query_AddTimePeriodCondition(QDateTime & clFrom, QDateTime & clTo);		// Add condition on test_date
    void	AddSplitlotConditions(GexDbPlugin_Filter &cFilters, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo, bool bUseSplitlotDateWindow = false);
    bool	GetStaticTestInfo(GexDbPlugin_Query & clGexDbQuery, GexDbPlugin_Filter &cFilters, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo, GexDbPlugin_TestInfo **ppTestInfo, unsigned int uiTestID, unsigned int uiTestSeq);
    bool	WriteTestInfo(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_RunInfo & clCurrentPart);
    bool    LoadDatabaseArchitecture();
};

#endif // _GEXDB_PLUGIN_MEDTRONIC_HEADER_
