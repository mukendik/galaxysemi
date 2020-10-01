/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Notes :                                                                              */
/*                                                                                      */
/****************************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////
// This file defines the YIELD-123 Plugin Class.
// Interface layer between Examinator and YIELD-123 Plugin.
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef GATE_EVENT_MANAGER_H
#define GATE_EVENT_MANAGER_H

// Local includes

// Standard includes
#include <QMap>
#include <QList>
#include <QString>

// Galaxy modules includes
#include <gstdl_errormgr.h>

/////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS
/////////////////////////////////////////////////////////////////////////////////////////
// YIELD-123 Plugin name
#define GEX_PLUGINNAME_YIELD123					"GEX Yield-123 Plugin V1.0"

// YIELD-123 command IDs
#define GEX_YIELD123_COMMAND_ID_NONE			0	// No command
#define GEX_YIELD123_COMMAND_ID_YIELD			1	// Perform a yield analysis
#define GEX_YIELD123_COMMAND_ID_QUALITY			2	// Perform a quality analysis
#define GEX_YIELD123_COMMAND_ID_REPEATABILITY	3	// Perform a repeatability analysis

// YIELD-123 command names
#define GEX_YIELD123_COMMAND_NAME_NONE			""								// No command
#define GEX_YIELD123_COMMAND_NAME_YIELD			"YIELD123_CHECKYIELD"			// Perform a yield analysis
#define GEX_YIELD123_COMMAND_NAME_QUALITY		"YIELD123_CHECKQUALITY"			// Perform a quality analysis
#define GEX_YIELD123_COMMAND_NAME_REPEATABILITY	"YIELD123_CHECKREPEATABILITY"	// Perform a repeatability analysis

// Name of Html report pages
#define GEX_YIELD123_HTML_PAGE_INDEX			"plugin_index"
#define GEX_YIELD123_HTML_PAGE_GLOBAL			"plugin_masa_mde_global"

// Title in Html report pages
#define GEX_YIELD123_HTML_TITLE_GLOBAL			"YIELD-123 Global Info"

// Common Plugin Function Names
#define PLUGIN_FUNCTION_GETPLUGINNAME		"GetPluginName"
#define PLUGIN_FUNCTION_GETPLUGINID			"GetPluginID"

// Plugin Event ID's
#define PLUGIN_EVENTID_BEGINDATA			0
#define PLUGIN_EVENTID_BEGINDATASET			1
#define PLUGIN_EVENTID_DEFINEPARAMETER		2
#define PLUGIN_EVENTID_LOT					3
#define PLUGIN_EVENTID_PARAMETERRESULT		4
#define PLUGIN_EVENTID_SETCOMMAND			5
#define PLUGIN_EVENTID_EXECCOMMAND			6
#define PLUGIN_EVENTID_ACTION				7
#define PLUGIN_EVENTID_GENERATEREPORT		8
#define PLUGIN_EVENTID_SETSETTING			9
#define PLUGIN_EVENTID_ABORT				10
#define PLUGIN_EVENTID_ENDDATASET			11
#define PLUGIN_EVENTID_ENDDATA				12
#define PLUGIN_EVENTID_PARTRESULT			13
#define PLUGIN_EVENTID_INTERACTIVECMD		14
#define PLUGIN_EVENTID_MAX					14

// Plugin Event names
#define PLUGIN_EVENTNAME_BEGINDATA			"BeginData"
#define PLUGIN_EVENTNAME_BEGINDATASET		"BeginDataset"
#define PLUGIN_EVENTNAME_DEFINEPARAMETER	"DefineParameter"
#define PLUGIN_EVENTNAME_LOT				"Lot"
#define PLUGIN_EVENTNAME_PARAMETERRESULT	"ParameterResult"
#define PLUGIN_EVENTNAME_SETCOMMAND			"SetCommand"
#define PLUGIN_EVENTNAME_EXECCOMMAND		"ExecCommand"
#define PLUGIN_EVENTNAME_ACTION				"Action"
#define PLUGIN_EVENTNAME_GENERATEREPORT		"GenerateReport"
#define PLUGIN_EVENTNAME_SETSETTING			"SetSetting"
#define PLUGIN_EVENTNAME_ABORT				"Abort"
#define PLUGIN_EVENTNAME_ENDDATASET			"EndDataset"
#define PLUGIN_EVENTNAME_ENDDATA			"EndData"
#define PLUGIN_EVENTNAME_PARTRESULT			"PartResult"
#define PLUGIN_EVENTNAME_INTERACTIVECMD		"InteractiveAction"

///////////////////////////////////////////////////////////
// Type definitions
///////////////////////////////////////////////////////////

// Types to access functions common to all plugins
typedef long (*FP_GetPluginID)();
typedef const char* (*FP_GetPluginName)();

class C_Gate_DataModel_Progress;
class C_Gate_DataModel;
class C_Gate_DataModel_ParameterSet;
class C_Gate_DataModel_TestingStage;
class C_Gate_DataModel_Batch;
class C_Gate_DataModel_Sublot;
class C_Gate_DataModel_Lot;
class C_Y123_Engine_Generic;

/////////////////////////////////////////////////////////////////////////////////////////
// DATA TYPES
/////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Class for display of event contents (for DEBUG)
///////////////////////////////////////////////////////////
class Gate_EventDebug
{
public:
	Gate_EventDebug();
	~Gate_EventDebug() { }

	void	Reset();
	void	NewLot();
	void	DisplayEvent(int nEventID, const QString & strEventName, const QString & strEventDump);
	bool	DisplayEvent_Enabled(int nEventID) { return (m_bDisplay_AllEvents && m_bDisplay_SingleEvent[nEventID]); }

private:
	bool	m_bDisplay_SingleEvent[PLUGIN_EVENTID_MAX+1];
	bool	m_bDiscard_SingleEvent[PLUGIN_EVENTID_MAX+1];
	bool	m_bDisplay_AllEvents;
	bool	m_bDiscard_AllEvents;
};

///////////////////////////////////////////////////////////
// Data Classes
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Plugin Info Details
///////////////////////////////////////////////////////////
class Gate_PluginDef
{
// Constructor/Destructor
public:
	Gate_PluginDef();	// Default constructor
	Gate_PluginDef( const Gate_PluginDef *source );			// copy constructor
	~Gate_PluginDef() { }

// Operators
public:
	Gate_PluginDef& operator=( const Gate_PluginDef &s );	// assignment operator

// Implementation
public:
	long		m_lPluginModuleID;	// Unique plugin module ID: Specified and maintained by Galaxy. (GEX_PLUGIN_MODULE_xxxxx)
	QString		m_strPluginName;	// Plugin name + version string, returned from third-party plugin DLL
};

///////////////////////////////////////////////////////////
// Interactive command Definition
///////////////////////////////////////////////////////////
class Gate_InteractiveCmd
{
// Constructor/Destructor
public:
	Gate_InteractiveCmd();	// Default constructor
	Gate_InteractiveCmd( const Gate_InteractiveCmd *source );  // copy constructor
	~Gate_InteractiveCmd() { }

// Operators
public:
	Gate_InteractiveCmd& operator=( const Gate_InteractiveCmd &s );  // assignment operator

// Implementation
public:
	void GetDump(QString & strFields) const;

	unsigned int	m_uiSiteIndex;					// Site index of parameter to display
	int				m_nSiteNb;						// Site nb of parameter to display
	unsigned int	m_uiParameterIndex;				// Parameter index of parameter to display
	int				m_uiParameterNumber;			// Parameter number of parameter to display
	QString			m_strParameterName;				// Parameter name of parameter to display
	int				m_nChartType;					// Chart type (histo, trend, scatter)
	int				m_nAdvancedReportSettings;		// Adv settings (ie over limits, adaptive...)
};

///////////////////////////////////////////////////////////
// Dataset Definition
///////////////////////////////////////////////////////////
class Gate_DatasetDef
{
// Constructor/Destructor
public:
	Gate_DatasetDef();	// Default constructor
	Gate_DatasetDef( const Gate_DatasetDef *source );		// copy constructor
	~Gate_DatasetDef() { }

// Operators
public:
	Gate_DatasetDef& operator=( const Gate_DatasetDef &s );	// assignment operator

// Implementation
public:
	void GetDump(QString & strFields) const;

	long		m_lGroupID;				// Dataset Group ID (when comparing  datasets)
	QString		m_strGroupName;			// Dataset label
	long		m_lTotalParameters;		// Total number of parameters (entries)
	long		m_lTotalBatches;		// Total batches in dataset (nb of Wafers in WaferTest, 1 if FinalTest)
};

///////////////////////////////////////////////////////////
// Parameter Definition
///////////////////////////////////////////////////////////
class Gate_ParameterDef
{
// Constructor/Destructor
public:
	Gate_ParameterDef();	// Default constructor
	Gate_ParameterDef( const Gate_ParameterDef *source );  // copy constructor
	~Gate_ParameterDef() { }

// Operators
public:
	Gate_ParameterDef& operator=( const Gate_ParameterDef &s );  // assignment operator

// Implementation
public:
	void		ResetData();							// Reset object data
	void		GetDump(QString & strFields) const;

	BYTE		m_bTestType;			// Test type ('-' if custom parameter added by GEX)
	int			m_nParameterIndex;		// Parameter index in TestList (0,1,2,...)
	int			m_nParameterNumber;		// Parameter/ Test number in TestList (0,1,2,...)
	QString		m_strName;				// Parameter name
	int			m_nPinArrayIndex;		// Parameter/ Test number in TestList (0,1,2,...)
	QString		m_strUnits;				// Parameter units
	double		m_lfLowL;				// Parameter Low Limit
	double		m_lfHighL;				// Parameter High Limit
	bool		m_bHasLowL;				// TRUE if test has a low limit
	bool		m_bHasHighL;			// TRUE if test has a high limit
	bool		m_bStrictLowL;			// TRUE if low limit is strict (test fail if result == low limit)
	bool		m_bStrictHighL;			// TRUE if high limit is strict (test fail if result == high limit)
	int			m_nLowLScaleFactor;		// Scale factor for LL
	int			m_nHighLScaleFactor;	// Scale factor for HL
	int			m_nResultScaleFactor;	// Scale factor for results
	int			m_nId;					// Test ID
	uint		m_uiFlags;				// Specific Flags (Duplicated test, ...)
	QList<int>	m_lstnInfoId;			// List of test info ID
};

///////////////////////////////////////////////////////////
// Binning Definition
///////////////////////////////////////////////////////////
class Gate_BinningDef
{
// Constructor/Destructor
public:
	Gate_BinningDef();	// Default constructor
	Gate_BinningDef( const Gate_BinningDef *source );  // copy constructor
	~Gate_BinningDef() { }

// Operators
public:
	Gate_BinningDef& operator=( const Gate_BinningDef &s );  // assignment operator

// Implementation
public:
	void		ResetData();							// Reset object data

	BYTE		m_bBinType;				// Bin type ('H' if HBIN, 'S' if SBIN)
	int			m_nBinNumber;			// Bin number in BinList (0,1,2,...)
	QString		m_strBinName;			// Bin name
	BYTE		m_bBinCat;				// 'P' if Good Bin
};

///////////////////////////////////////////////////////////
// Parameter statistics
///////////////////////////////////////////////////////////
class Gate_ParameterStats
{
// Constructor/Destructor
public:
	Gate_ParameterStats();	// Default constructor
	Gate_ParameterStats( const Gate_ParameterStats *source );  // copy constructor
	~Gate_ParameterStats() { }

// Operators
public:
	Gate_ParameterStats& operator=( const Gate_ParameterStats &s );  // assignment operator

// Implementation
public:
	void			ResetData();							// Reset object data
	void			GetDump(QString & strFields) const;

	unsigned int	m_uiNbExecs;	// Nb executions
	unsigned int	m_uiNbOutliers;	// Nb of outliers
	unsigned int	m_uiNbFails;	// Nb of failures
	double			m_lfSumX;		// Sum of result values
	double			m_lfSumX2;		// Sum of square of result values
	float			m_fMin;			// Min value
	float			m_fMax;			// Max value
	float			m_fRange;		// Range
	float			m_fMean;		// Mean
	float			m_fSigma;		// Sigma
	float			m_fCp;			// Cp
	float			m_fCpk;			// Cpk
};

///////////////////////////////////////////////////////////
// Site description information (read from SDR record)
///////////////////////////////////////////////////////////
class Gate_SiteDescription
{
public:
// Constructor/Destructor
	Gate_SiteDescription();
	~Gate_SiteDescription();

// Operators
public:
	Gate_SiteDescription& operator+=(const Gate_SiteDescription& aSite);
	Gate_SiteDescription& operator=(const Gate_SiteDescription& aSite);

// Implementation
public:
	void GetDump(QString & strFields) const;

	int				m_nSiteNum;					// Site nb, or 255 for all sites
	int				m_nHeadNum;					// Head nb, or 255 for all heads
	QString			m_strHandlerType;			// Handler type
	QString			m_strHandlerProberID;		// Handler/Prober ID
	QString			m_strProbeCardType;			// Prober card type
	QString			m_strProbeCardID;			// Prober card ID
	QString			m_strLoadBoardType;			// Loadboard type
	QString			m_strLoadBoardID;			// Loadboard ID
	QString			m_strDibBoardID;			// DibBoard ID
	QString			m_strInterfaceCableID;		// Interface cable ID
	QString			m_strHandlerContactorID;	// Handler Contactor ID
	QString			m_strLaserID;				// Laser ID
	QString			m_strExtraEquipmentID;		// Extra equipment ID
};

class Gate_SiteDescriptionMap: public QMap<int, Gate_SiteDescription>
{
};

///////////////////////////////////////////////////////////
// Lot Definition
///////////////////////////////////////////////////////////
class Gate_LotDef
{
// Constructor/Destructor
public:
	Gate_LotDef();	// Default constructor
	Gate_LotDef( const Gate_LotDef *source );  // copy constructor
	~Gate_LotDef() { };

// Operators
public:
	Gate_LotDef& operator=( const Gate_LotDef &s );  // assignment operator

// Implementation
public:
	void GetDump(QString & strFields) const;

	QString			m_strDatasetOrigin;		// Origin of dataset (ie "MASA-CSV" if file has been generated by MASA converter)
	QString			m_strProductID;			// Product ID of dataset (must be unique through all testing stages)
	QString			m_strLotID;				// LotID string
	QString			m_strProcId;			// Process ID
	QString			m_strSubLotID;			// SublotID string
	QString			m_strWaferID;			// WaferID string
	QString			m_strTesterName;		// Tester name
	QString			m_strTesterType;		// Tester type
	QString			m_strOperatorName;		// Operator name
	QString			m_strJobName;			// Job name
	QString			m_strJobRevision;		// Job revision
	QString			m_strExecType;			// Tester Exec type
	QString			m_strExecVersion;		// Tester Exec version
	QString			m_strTestCode;			// Test code
	QString			m_strTestTemperature;	// Test temperature

	QList<int>		m_lstGoodHardBins;		// list of all good hard binnings
	QList<int>		m_lstGoodSoftBins;		// list of all good soft binnings

	long			m_lStartTime;			// Time of first part tested
	long			m_lFinishTime;			// Time of last part tested
	long			m_lTotalParts;			// Total parts in wafer
	unsigned int	m_uiProgramRuns;		// Nb of program runs
	unsigned int	m_uiNbSites;			// Nb of sites in the original file (not merged)
	unsigned int*	m_puiPartsPerSite;		// Nb of parts per site	(array of 255 unsigned)
};

///////////////////////////////////////////////////////////
// Data Result Definition
///////////////////////////////////////////////////////////
class Gate_DataResult
{
// Constructor/Destructor
public:
	Gate_DataResult();	// Default constructor
	Gate_DataResult( const Gate_DataResult *source );  // copy constructor
	~Gate_DataResult() { }

// Operators
public:
	Gate_DataResult& operator=( const Gate_DataResult &s );  // assignment operator

// Implementation
public:
	void GetDump(QString & strFields) const;

	unsigned int	m_uiProgramRunIndex;	// Test program run index
	int				m_nFlowId;				// Flow id
	int				m_nHeadNum;				// Head number
	int				m_nSiteNum;				// Site number
	int				m_nStepNum;				// Step number
	BYTE			m_bTestType;			// Test type ('-' if custom parameter added by GEX)
	int				m_nParameterIndex;		// Parameter index
	double			m_lfValue;				// Data result
	QString			m_strValue;				// String result
	bool			m_bValidValue;			// Data result is valid (ex: false for functional)
	bool			m_bIsOutlier;			// TRUE if Data is an outlier
	bool			m_bPassFailStatusValid;	// TRUE if m_bTestFailed is valid
	bool			m_bTestFailed;			// TRUE if test FAILED, FALSE if test PASSED (valid only if m_bFailStatusValid is TRUE)
};

class Gate_DataResultList: public QList<Gate_DataResult *>
{
public:
	Gate_DataResultList() { m_bAutoDelete = TRUE; };
	~Gate_DataResultList() {clearItems();};

	void clearItems() {	if(m_bAutoDelete)	{ while(!isEmpty())	delete takeFirst();	}	clear();};

// PROTECTED DATA
protected:
	bool			m_bAutoDelete;
};

///////////////////////////////////////////////////////////
// Part Result Definition
///////////////////////////////////////////////////////////
class Gate_PartResult
{
// Constructor/Destructor
public:
	Gate_PartResult();	// Default constructor
	Gate_PartResult( const Gate_PartResult *source );  // copy constructor
	~Gate_PartResult() { }

// Operators
public:
	Gate_PartResult& operator=( const Gate_PartResult &s );  // assignment operator

// Implementation
public:
	void GetDump(QString & strFields) const;

	unsigned int	m_uiProgramRunIndex;		// Test program run nb
	int				m_nHeadNum;					// Head Nb
	int				m_nSiteNum;					// Site Nb
	int				m_nPart_X;					// X coordinate of part
	int				m_nPart_Y;					// Y coordinate of part
	int				m_nHardBin;					// Hardware binning of part
	int				m_nSoftBin;					// Software binning of part
	QString			m_strPartID;				// Part identification string
	bool			m_bPassFailStatusValid;		// TRUE if m_bPartFailed is valid
	bool			m_bPartFailed;				// TRUE if part FAILED, FALSE if part PASSED (valid only if m_bPartStatusValid is TRUE)
};

///////////////////////////////////////////////////////////
// CLASS: CGate_EventManager
// Yield123 Plugin class
///////////////////////////////////////////////////////////
class CGate_EventManager
{
public:
	GDECLARE_ERROR_MAP(CGate_EventManager)
	{
		eLibraryNotFound,			// Plugin library not found
		eUnresolvedFunctions,		// Unresolved functions in library
		eInit						// Error initializing plug-in library
	}
	GDECLARE_END_ERROR_MAP(CGate_EventManager)

	// Status of yield analysis
	enum Status
	{
		eStatusOK,									// Analysis OK
		eStatus_Error_Malloc,						// Memory allocation failure
		eStatus_Error_ParameterSet_Init,			// Error initializing parameter set
		eStatus_Error_ParameterDefinition,			// Error setting parameter values (limits...)
		eStatus_Error_InvalidParameterSet,			// Invalid Paramater set
		eStatus_Error_NoParameterSet,				// No Paramater set for current lot
		eStatus_Error_AddBatch,						// Error adding batch to dataset
		eStatus_Error_SetTestResult,				// Error setting test result
		eStatus_Error_SetPartResult,				// Error setting part result
		eStatus_Error_GetTestingStage,				// Error retrieving a testing stage to add coming data
		eStatus_Error_InitEngine,					// Error initializing nalysis engine
		eStatus_Error_RunEngine,					// Error running analysis engine
		eStatus_Error_TryCatch,						// Error running crash
		eStatus_Error_CommandNotSupported			// The command is not supported
	};

	CGate_EventManager();
	~CGate_EventManager();

	// Common plug-in functions (specialized for YIELD123 plug-in)
	bool	Init(void);
	bool	ClearData();
	bool	SetErrorMessage(Status eStatus, const char *szError);
	bool	eventBeginData();
	bool	eventBeginDataset(const Gate_DatasetDef & DatasetDef);
	bool	eventDefineBinning(const Gate_BinningDef & BinDefinition);	// Called one time per parameter definition
	bool	eventDefineParameter(const Gate_ParameterDef & ParameterDefinition);	// Called one time per parameter definition
	bool	eventLot(const Gate_LotDef & LotDefinition, const Gate_SiteDescription& clGlobalEquipmentID, const Gate_SiteDescriptionMap* pSiteEquipmentIDMap, bool bIgnoreThisSubLot);	// Lot, sub-lot (or waferID if wafer test) change notification
	bool	eventParameterResult(const Gate_DataResult & DataResult);		// Data result
	bool	eventPartResult(const Gate_PartResult & PartResult);			// Result for 1 part (binning, x, y...)
	bool	eventEndDataset();											// End of current Dataset
	bool	eventEndData();												// End of ALL Datasets
	bool	eventSetCommand(const QString & strCommand);				// Set next plugin command
	bool	eventSetSetting(const QString & strSetting);				// Set plugin setting

	bool	eventDumpData(const QString & strReportFile);
	bool	eventInitAnalyzeDataModel();
	bool	eventRunAnalyzeDataModel();

	// Status
	Status						m_eStatus;					// Status of yield analysis
	QString						m_strError;					// Error message
	bool						m_bInvalidData;
	QString						m_strFileName;				// Original file name from user
	QString						m_strAnalyzeName;			// Analyze name

	// Data
	C_Gate_DataModel*			m_pclData;					// Data to be analyzed
	// Yield123 analysis engine
	C_Y123_Engine_Generic*		m_pEngineGeneric;
	C_Gate_DataModel_Progress*	m_pProgress;

private:
	// Report


private:

	// Data
	C_Gate_DataModel_ParameterSet*	m_pTmpParameterSet;			// Temporary parameter set (with parameter definition)
	C_Gate_DataModel_ParameterSet*	m_pCurrentParameterSet;		// Cureent parameter set
	C_Gate_DataModel_TestingStage*	m_pCurrentTestingStage;		// Ptr on current testing stage
	C_Gate_DataModel_Lot*			m_pCurrentLot;				// Ptr on current lot
	C_Gate_DataModel_Sublot*		m_pCurrentSublot;			// Ptr on current sublot
	C_Gate_DataModel_Batch*			m_pCurrentBatch;			// Ptr on current data batch

	// Other members used internally
	QString						m_strGroupName;				// Dataset group name

	int							m_nPluginCommandStatus;		// Status of last plugin command executed
	int							m_nPluginCommandID;			// ID of next command to be executed by plugin
	Gate_PluginDef				m_clPluginDef;

private:

};


class TestKey
{
public:
	TestKey()
	{
		m_iNumber	= 0;
		m_strName	= "";
		m_lfLowL	= 0.0;
		m_lfHighL	= 0.0;
	}
	
	TestKey(const Gate_ParameterDef &paramInfo)
	{
		setNumber(paramInfo.m_nParameterNumber);
		setName(paramInfo.m_strName);
		setLowL(paramInfo.m_lfLowL);
		setHighL(paramInfo.m_lfHighL);
	}
	
	TestKey(const int &iNumber, const QString &strName, const double &lfLowL = 0.0, const double &lfHighL = 0.0)
	{
		setNumber(iNumber);
		setName(strName);
		setLowL(lfLowL);
		setHighL(lfHighL);
	}
	
	int				number()	const				{return m_iNumber;}
	QString			name()		const				{return m_strName;}
	double			lowL()		const				{return m_lfLowL;}
	double			highL()		const				{return m_lfHighL;}
	void			setNumber(const int &iNumber)	{ m_iNumber = iNumber;}
	void			setName(const QString &strName)	{ m_strName = strName;}
	void			setLowL(const double lfLowL)	{ m_lfLowL= lfLowL;}
	void			setHighL(const double lfHighL)	{ m_lfHighL= lfHighL;}
	void			setTest(const Gate_ParameterDef &param)
	{
		setNumber(param.m_nParameterNumber);
		setName(param.m_strName);
		setLowL(param.m_lfLowL);
		setHighL(param.m_lfHighL);
	}
	
private:
	int		m_iNumber;
	QString m_strName;
	double	m_lfLowL;
	double	m_lfHighL;
};


inline bool operator<(const TestKey &t1, const TestKey &t2)
{			
	if (t1.number() == t2.number())
	{
		if (t1.name() == t2.name())
		{
			if (t1.highL() == t2.highL())
			{
				return t1.lowL() < t2.lowL();
			}
			else
				return t1.highL() < t2.highL();
		}
		else if (t1.name().trimmed().isEmpty() || t2.name().trimmed().isEmpty())
			return false;
		else
			return t1.name() < t2.name();
	}
	
	return t1.number() < t2.number();
}

#endif // ifdef GATE_EVENT_MANAGER_H

