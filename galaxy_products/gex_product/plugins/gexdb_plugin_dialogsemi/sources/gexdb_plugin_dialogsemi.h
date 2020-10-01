// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

#ifndef _GEXDB_PLUGIN_DIALOGSEMI_HEADER_
#define _GEXDB_PLUGIN_DIALOGSEMI_HEADER_

// Standard includes

// Qt includes

// Galaxy modules includes
#include <cstdf.h>
#include <cstdfparse_v4.h>
#include <qprocess.h> 

// Local includes
#include "gexdb_plugin_common.h"

// Plugin name
#define GEXDB_PLUGIN_DIALOGSEMI_NAME		"Dialogsemi SQL Database [Plugin V1.1 B2]"
#define GEXDB_PLUGIN_DIALOGSEMI_BUILD		2

// Name of supported testing stages
#define GEXDB_PLUGIN_DIALOGSEMI_WTEST		"Wafer Sort"
#define GEXDB_PLUGIN_DIALOGSEMI_FTEST		"Final Test"

extern "C" GEXDB_PLUGIN_API GexDbPlugin_Base * gexdb_plugin_getobject(QMainWindow *pMainWindow, const QString & strHostName, const QString & strApplicationPath, const QString & strUserProfile, const QString & strLocalFolder, const char *gexLabelFilterChoices[], CGexSkin * pGexSkin, const bool bCustomerDebugMode);
extern "C" GEXDB_PLUGIN_API void gexdb_plugin_releaseobject(GexDbPlugin_Base *pObject);

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_Dialogsemi_Wt_WaferInfo
// Holds info on 1 wafer
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_Dialogsemi_Wt_WaferInfo
{
public:
        GexDbPlugin_Dialogsemi_Wt_WaferInfo()
	{
		m_lWaferInfoID = -1;
		m_nTotalTested = -1;
		m_nTotalPassed = -1;
	}

	// Fields from DialogSemi DB (waf_prober_info table)
	Q_LLONG			m_lWaferInfoID;			// WAF_PROBER_INFO.WAF_PROBER_INFO_ID
	QString			m_strDeviceID;			// WAF_PROBER_INFO.DEVICE_ID
	QString			m_strBatchNumber;		// WAF_PROBER_INFO.BATCHNUMBER
	QString			m_strWaferID;			// WAF_PROBER_INFO.WAFERID
	QString			m_strTestSys;			// WAF_PROBER_INFO.TESTSYS
	time_t			m_lStartDate;			// WAF_PROBER_INFO.TEST_STARTDATE
	time_t			m_lEndDate;				// WAF_PROBER_INFO.TEST_ENDDATE
	int				m_nTotalTested;			// WAF_PROBER_INFO.TOTAL_TESTED
	int				m_nTotalPassed;			// WAF_PROBER_INFO.TOTAL_PASSED
	QString			m_strTestMode;			// WAF_PROBER_INFO.TESTMODE
	QString			m_strTemp;				// WAF_PROBER_INFO.TEMP
	QString			m_strAcceptBin;			// WAF_PROBER_INFO.ACCEPTBIN
	int				m_nNumberOfSites;		// WAF_PROBER_INFO.NUMBER_OF_SITES
	QString			m_strLoadboard;			// WAF_PROBER_INFO.LOADBOARD
	QString			m_strProbeCard;			// WAF_PROBER_INFO.PROBECARD
	QString			m_strProgName;			// WAF_PROBER_INFO.PROGNAME
	QString			m_strProgRevision;		// WAF_PROBER_INFO.PROG_REVISION
	QString			m_strProgRevDate;		// WAF_PROBER_INFO.PROG_REVDATE
	QString			m_strSiteCd;			// WAF_PROBER_INFO.SITE_CD
	QString			m_strFlatCd;			// WAF_PROBER_INFO.FLAT_CD
	QString			m_strHandlingsys;		// WAF_PROBER_INFO.HANDLINGSYS
	char			m_cValidFlag;			// WAF_PROBER_INFO.VALID_FLAG
	QString			m_strFacility;			// WAF_PROBER_INFO.FACILITY
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_Dialogsemi_Ft_JobInfo
// Holds info on 1 wafer
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_Dialogsemi_Ft_JobInfo
{
public:
        GexDbPlugin_Dialogsemi_Ft_JobInfo()
	{
		m_lJobInfoID = -1;
		m_nTotalTested = -1;
		m_nTotalPassed = -1;
	}

	// Fields from DialogSemi DB (ft_job_info table)
	Q_LLONG			m_lJobInfoID;			// FT_JOB_INFO.FT_JOB_INFO_ID
	QString			m_strDeviceID;			// FT_JOB_INFO.DEVICE_ID
	QString			m_strBatchNumber;		// FT_JOB_INFO.BATCHNUMBER
	QString			m_strDateCode;			// FT_JOB_INFO.DATECODE
	QString			m_strTestSys;			// FT_JOB_INFO.TESTSYS
	time_t			m_lStartDate;			// FT_JOB_INFO.TEST_STARTDATE
	time_t			m_lEndDate;				// FT_JOB_INFO.TEST_ENDDATE
	int				m_nTotalTested;			// FT_JOB_INFO.TOTAL_TESTED
	int				m_nTotalPassed;			// FT_JOB_INFO.TOTAL_PASSED
	QString			m_strTestMode;			// FT_JOB_INFO.TESTMODE
	QString			m_strTemp;				// FT_JOB_INFO.TEMP
	int				m_nNumberOfSites;		// FT_JOB_INFO.NUMBER_OF_SITES
	QString			m_strLoadboard;			// FT_JOB_INFO.LOADBOARD
	QString			m_strProgName;			// FT_JOB_INFO.PROGNAME
	QString			m_strHandingSys;		// FT_JOB_INFO.HANDINGSYS
	QString			m_strProgRevision;		// FT_JOB_INFO.PROG_REVISION
	QString			m_strProgRevDate;		// FT_JOB_INFO.PROG_REVDATE
	QString			m_strAcceptBin;			// FT_JOB_INFO.ACCEPTBIN
	char			m_cValidFlag;			// FT_JOB_INFO.VALID_FLAG
};

///////////////////////////////////////////////////////////
// GexDbPlugin_DialogSemi class: database plugin class for DIALOGSEMI database types
///////////////////////////////////////////////////////////
class GexDbPlugin_Dialogsemi : public GexDbPlugin_Base
{
// Constructor / Destructor
public:
    GexDbPlugin_Dialogsemi(QMainWindow *pMainWindow, const QString & strHostName, const QString & strApplicationPath, const QString & strUserProfile, const QString & strLocalFolder, const char *gexLabelFilterChoices[], const bool bCustomerDebugMode, CGexSkin * pGexSkin, GexDbPlugin_Connector *pclDatabaseConnector=NULL);
	virtual ~GexDbPlugin_Dialogsemi();

// Member functions
public:
	// Common functions
	void			GetPluginName(QString & strPluginName) { strPluginName = GEXDB_PLUGIN_DIALOGSEMI_NAME; }
	unsigned int	GetPluginBuild(void) { return GEXDB_PLUGIN_DIALOGSEMI_BUILD; }
	void			GetSupportedTestingStages(QString & strSupportedTestingStages);
	bool			IsInsertionSupported() { return false; }
	bool			IsUpdateSupported() { return false; }
	bool			IsTestingStagesSupported() { return true; }
	bool			IsParameterSelectionSupported() { return false; }
	bool			IsEnterpriseReportsSupported() { return false; }
	bool			IsTestingStage_FinalTest(const QString& /*strTestingStage*/) { return false; }
	bool			IsTestingStage_Foundry(const QString& /*strTestingStage*/) { return false; }
	bool			GetTestingStageName_Foundry(QString & strTestingStage) { strTestingStage = ""; return true; }
	void			GetLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);

	// Plugin configuration
	bool			ConfigWizard();
	
	// Data extraction for GEX standard reports
	bool			QueryField(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bClearQueryFirst=true,bool bDistinct=true);		// Return all valid values for a field (given filters on other fields)
	bool			QueryTestlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues, bool bParametricOnly);		// Return all tests (given filters on other fields)
	bool			QueryDataFiles(GexDbPlugin_Filter & cFilters, const QString & strTestlist, GexDbPlugin_DataFileList & cMatchingFiles, const QString & strDatabasePhysicalPath, const QString & strLocalDir, bool *pbFilesCreatedInFinalLocation, GexDbPlugin_Base::StatsSource eStatsSource);	// Return all valid Data files (given filters on several fields)

// Member variables
public:
	
// Implementation
protected:
	/////////////////////////////////////////////////
	// VARIABLES DEFINITION
	/////////////////////////////////////////////////
	// Database mapping
	GexDbPlugin_Mapping_FieldMap			m_mapFields_GexToRemote_Et;	// Fields mapping: GEX <-> Custom DB (E-Test)
	GexDbPlugin_Mapping_LinkMap				m_mapLinks_Remote_Et;		// Table links mapping (E-Test)
	GexDbPlugin_Mapping_FieldMap			m_mapFields_GexToRemote_Wt;	// Fields mapping: GEX <-> Custom DB (Wafer Sort)
	GexDbPlugin_Mapping_LinkMap				m_mapLinks_Remote_Wt;		// Table links mapping (Wafer Sort)
	GexDbPlugin_Mapping_FieldMap			m_mapFields_GexToRemote_Ft;	// Fields mapping: GEX <-> Custom DB (Final Test)
	GexDbPlugin_Mapping_LinkMap				m_mapLinks_Remote_Ft;		// Table links mapping (Final Test)
	QStringList								m_strlLabelFilterChoices_Et;
	QStringList								m_strlLabelFilterChoices_Ft;
	QStringList								m_strlLabelFilterChoices_Wt;

	// Wacfer Sort
	GexDbPlugin_RunInfo						*m_pPartInfoMatrix;			// X*Y matrix of RunInfo pointers
	int										m_nWaferNbRows;				// Nb of wafer rows (Y)
	int										m_nWaferNbColumns;			// Nb of wafer columns (X)
	int										m_nWaferOffsetX;			// Offset in X to access XY matrix
	int										m_nWaferOffsetY;			// Offset in Y to access XY matrix
	unsigned int							m_uiWaferNbParts;			// Nb of parts on wafer

	// Final Test
	GexDbPlugin_RunInfo						*m_pPartInfoArray;			// array of RunInfo pointers
	int										m_nNbRuns;					// Nb of runs
	int										m_nRunOffset;				// Offset to access RunInfo array
	unsigned int							m_uiLotNbParts;				// Nb of parts in lot

	// Other
	QString									m_strDebugString;
	QString									m_strTestingStage;			// Testing stage for current query
	QMap<Q_LLONG, GexDbPlugin_TestInfo>		m_mapTestlist;				// Testlist mapping
	QMap<int, GexDbPlugin_SiteInfo>			m_mapSites;					// Site mapping
	
	/////////////////////////////////////////////////
	// FUNCTIONS DEFINITION
	/////////////////////////////////////////////////
	int		GetNbOfFilesToGenerate(GexDbPlugin_Filter &cFilters);
	Q_LLONG	GetTestID(unsigned int uiTestNum);
	bool	WriteSummaryRecords(CStdfParse_V4 & clStdfParse);
	void	Query_AddTestlistCondition(const QString & strTestlist);
	bool	WritePIR(CStdfParse_V4 & clStdfParse, int nSiteNum);
	bool	WritePRR(CStdfParse_V4 & clStdfParse, int nSiteNum, int nBinning, int nX, int nY, int nSerialNumber=-1);
	bool	Query_AddTimePeriodCondition(GexDbPlugin_Filter &cFilters);		// Add condition if user selected a time period

	// Wafer Sort
	bool	QueryDataFiles_Wt(GexDbPlugin_Filter & cFilters, const QString & strTestlist, GexDbPlugin_DataFileList & cMatchingFiles, const QString & strLocalDir);	// Return all valid Data files (given filters on several fields)
	bool	ConstructStdfFilesQuery_Wt(GexDbPlugin_Filter &cFilters, QString & strQuery);							// Construct query to retrieve all STDF files corresponding to specified filters
	bool	CreateStdfFile_Wt(const QSqlQuery & clQueryWafer, const QString & strTestlist, const QString & strLocalDir, QString & strStdfFileName);	// Create a STDF file for specified Splitlot_id
	bool	CreateTestlist(GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo);
	bool	WriteStaticTestInfo(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo);
	bool	InitMaps(GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo);
	bool	WriteTestResults(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo, const QString & strTestlist);
	bool	WriteMIR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo);
	bool	WriteMRR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo);
	bool	WriteWIR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo);
	bool	WriteWRR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo);

	// Final Test
	bool	QueryDataFiles_Ft(GexDbPlugin_Filter & cFilters, const QString & strTestlist, GexDbPlugin_DataFileList & cMatchingFiles, const QString & strLocalDir);	// Return all valid Data files (given filters on several fields)
	bool	ConstructStdfFilesQuery_Ft(GexDbPlugin_Filter &cFilters, QString & strQuery);							// Construct query to retrieve all STDF files corresponding to specified filters
	bool	CreateStdfFile_Ft(const QSqlQuery & clQueryJob, const QString & strTestlist, const QString & strLocalDir, QString & strStdfFileName);	// Create a STDF file for specified Splitlot_id
	bool	CreateTestlist(GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo);
	bool	WriteStaticTestInfo(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo);
	bool	InitMaps(GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo);
	bool	WriteTestResults(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo, const QString & strTestlist);
	bool	WriteMIR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo);
	bool	WriteMRR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo);
};

#endif // _GEXDB_PLUGIN_DIALOGSEMI_HEADER_
