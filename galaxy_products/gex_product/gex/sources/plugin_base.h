///////////////////////////////////////////////////////////
// Plug-in Base Class. 
// Interface layer between Examinator and 
// third-party applications
///////////////////////////////////////////////////////////

#ifndef GEX_PLUGIN_BASE_H
#define GEX_PLUGIN_BASE_H

// Standard includes
#include <stdio.h>

// QT includes
#include <qmap.h>

// Galaxy modules includes
#include <gstdl_errormgr.h>

///////////////////////////////////////////////////////////
// Constant definitions
///////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////
// Class for display of event contents (for DEBUG)
///////////////////////////////////////////////////////////
class GP_EventDebug
{
public:
	GP_EventDebug();
	~GP_EventDebug() { }

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
class GP_PluginDef
{
// Constructor/Destructor
public:
	GP_PluginDef();	// Default constructor
	GP_PluginDef( const GP_PluginDef *source );			// copy constructor
	~GP_PluginDef() { }

// Operators
public:
	GP_PluginDef& operator=( const GP_PluginDef &s );	// assignment operator

// Implementation
public:
	long		m_lPluginModuleID;	// Unique plugin module ID: Specified and maintained by Galaxy. (GEX_PLUGIN_MODULE_xxxxx)
	QString		m_strPluginName;	// Plugin name + version string, returned from third-party plugin DLL
};

///////////////////////////////////////////////////////////
// Interactive command Definition
///////////////////////////////////////////////////////////
class GP_InteractiveCmd
{
// Constructor/Destructor
public:
	GP_InteractiveCmd();	// Default constructor
	GP_InteractiveCmd( const GP_InteractiveCmd *source );  // copy constructor
	~GP_InteractiveCmd() { }

// Operators
public:
	GP_InteractiveCmd& operator=( const GP_InteractiveCmd &s );  // assignment operator

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
class GP_DatasetDef
{
// Constructor/Destructor
public:
	GP_DatasetDef();	// Default constructor
	GP_DatasetDef( const GP_DatasetDef *source );		// copy constructor
	~GP_DatasetDef() { }

// Operators
public:
	GP_DatasetDef& operator=( const GP_DatasetDef &s );	// assignment operator

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
class GP_ParameterDef
{
// Constructor/Destructor
public:
	GP_ParameterDef();	// Default constructor
	GP_ParameterDef( const GP_ParameterDef *source );  // copy constructor
	~GP_ParameterDef() { }

// Operators
public:
	GP_ParameterDef& operator=( const GP_ParameterDef &s );  // assignment operator

// Implementation
public:
	void		ResetData();							// Reset object data
	void		GetDump(QString & strFields) const;	

	BYTE		m_bTestType;			// Test type ('-' if custom parameter added by GEX)
	int			m_nParameterIndex;		// Parameter index in TestList (0,1,2,...)
	int			m_nParameterNumber;		// Parameter/ Test number in TestList (0,1,2,...)
	QString		m_strName;				// Parameter name
	QString		m_strUnits;				// Parameter units
	double		m_lfLowL;				// Parameter Low Limit
	double		m_lfHighL;				// Parameter High Limit
	bool		m_bHasLowL;				// true if test has a low limit
	bool		m_bHasHighL;			// true if test has a high limit
	bool		m_bStrictLowL;			// true if low limit is strict (test fail if result == low limit)
	bool		m_bStrictHighL;			// true if high limit is strict (test fail if result == high limit)
	int			m_nLowLScaleFactor;		// Scale factor for LL
	int			m_nHighLScaleFactor;	// Scale factor for HL
	int			m_nResultScaleFactor;	// Scale factor for results
};

///////////////////////////////////////////////////////////
// Parameter statistics
///////////////////////////////////////////////////////////
class GP_ParameterStats
{
// Constructor/Destructor
public:
	GP_ParameterStats();	// Default constructor
	GP_ParameterStats( const GP_ParameterStats *source );  // copy constructor
	~GP_ParameterStats() { }

// Operators
public:
	GP_ParameterStats& operator=( const GP_ParameterStats &s );  // assignment operator

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
class GP_SiteDescription
{
public:
// Constructor/Destructor
	GP_SiteDescription();
	~GP_SiteDescription();

// Operators
public:
	GP_SiteDescription& operator+=(const GP_SiteDescription& aSite);
	GP_SiteDescription& operator=(const GP_SiteDescription& aSite);

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

class GP_SiteDescriptionMap: public QMap<int, GP_SiteDescription>
{
};

///////////////////////////////////////////////////////////
// Lot Definition
///////////////////////////////////////////////////////////
class GP_LotDef
{
// Constructor/Destructor
public:
	GP_LotDef();	// Default constructor
	GP_LotDef( const GP_LotDef *source );  // copy constructor
	~GP_LotDef() { }

// Operators
public:
	GP_LotDef& operator=( const GP_LotDef &s );  // assignment operator

// Implementation
public:
	void GetDump(QString & strFields) const;

	QString			m_strDatasetOrigin;		// Origin of dataset (ie "MASA-CSV" if file has been generated by MASA converter)
	QString			m_strProductID;			// Product ID of dataset (must be unique through all testing stages)
	QString			m_strLotID;				// LotID string
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

	long			m_lStartTime;			// Time of first part tested
	long			m_lFinishTime;			// Time of last part tested
	long			m_lTotalParts;			// Total parts in wafer
	unsigned int	m_uiProgramRuns;		// Nb of program runs
	unsigned int*	m_puiPartsPerSite;		// Nb of parts per site	(array of 255 unsigned)
};

///////////////////////////////////////////////////////////
// Data Result Definition
///////////////////////////////////////////////////////////
class GP_DataResult
{
// Constructor/Destructor
public:
	GP_DataResult();	// Default constructor
	GP_DataResult( const GP_DataResult *source );  // copy constructor
	~GP_DataResult() { }

// Operators
public:
	GP_DataResult& operator=( const GP_DataResult &s );  // assignment operator

// Implementation
public:
	void GetDump(QString & strFields) const;

	unsigned int	m_uiProgramRunIndex;	// Test program run index
	int				m_nHeadNum;				// Head number
	int				m_nSiteNum;				// Site number
	BYTE			m_bTestType;			// Test type ('-' if custom parameter added by GEX)
	int				m_nParameterIndex;		// Parameter index
	double			m_lfValue;				// Data result
	bool			m_bIsOutlier;			// true if Data is an outlier
	bool			m_bPassFailStatusValid;	// true if m_bTestFailed is valid
	bool			m_bTestFailed;			// true if test FAILED, false if test PASSED (valid only if m_bFailStatusValid is true)
};

///////////////////////////////////////////////////////////
// Part Result Definition
///////////////////////////////////////////////////////////
class GP_PartResult
{
// Constructor/Destructor
public:
	GP_PartResult();	// Default constructor
	GP_PartResult( const GP_PartResult *source );  // copy constructor
	~GP_PartResult() { }

// Operators
public:
	GP_PartResult& operator=( const GP_PartResult &s );  // assignment operator

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
	bool			m_bPassFailStatusValid;		// true if m_bPartFailed is valid
	bool			m_bPartFailed;				// true if part FAILED, false if part PASSED (valid only if m_bPartStatusValid is true)
};

///////////////////////////////////////////////////////////
// Data collection functions (notification events)
///////////////////////////////////////////////////////////
class CExaminatorBasePlugin
{
public:
	GDECLARE_ERROR_MAP(CExaminatorBasePlugin)
	{
		eLibraryNotFound,			// Plugin library not found	
		eUnresolvedFunctions,		// Unresolved functions in library
		eInit						// Error initializing plug-in library
	}
	GDECLARE_END_ERROR_MAP(CExaminatorBasePlugin)

	CExaminatorBasePlugin(QString strPluginFullPath, QString strPluginDirectory);
	virtual ~CExaminatorBasePlugin();

	long		GetPluginID(void) { return m_clPluginDef.m_lPluginModuleID; };
	QString		GetPluginName(void) { return m_clPluginDef.m_strPluginName; };
	QString		GetPluginCommand(void) { return m_strPluginCommand; };

	// Common plug-in functions (specialized for each plug-in)
	// Virtual
	virtual QString	GetHomePage(void) { return ""; };					// Return plugin-specific home page
	virtual int		BuildGEXReport(void);								// Return 1 if report should be created, 0 else
	virtual bool	HTMLOutputOnly(void);								// Return true if only HTML format should be supported by plug-in, false else
	virtual void	eventSetSetting(const QString & strSetting);		// Set plugin setting
	virtual void	eventAbort(void);									// Abort current action.
	virtual void	eventInteractiveCmd(GP_InteractiveCmd & InteractiveCmd);	// Notification event: command from interactive module
	// Pure virtual
	virtual bool	Init(void) = 0;
	virtual void	eventBeginData(void) = 0;
	virtual void	eventBeginDataset(const GP_DatasetDef & DatasetDef) = 0;
	virtual void	eventDefineParameter(const GP_ParameterDef & ParameterDefinition) = 0;	// Called one time per parameter definition
	virtual void	eventLot(const GP_LotDef & LotDefinition, const GP_SiteDescription& clGlobalEquipmentID, const GP_SiteDescriptionMap* pSiteEquipmentIDMap, bool bIgnoreThisSubLot) = 0;	// Lot, sub-lot (or waferID if wafer test) change notification
	virtual void	eventParameterResult(const GP_DataResult & DataResult) = 0;				// Data result
	virtual void	eventPartResult(const GP_PartResult & PartResult) = 0;					// Result for 1 part (binning, x, y...)
	virtual void	eventEndDataset(void) = 0;												// End of current Dataset
	virtual void	eventEndData(void) = 0;
	virtual void	eventSetCommand(const QString & strCommand) = 0;						// Set next plugin command
	virtual void	eventExecCommand(void) = 0;												// Execute Plugin command
	virtual void	eventAction(const QString & strAction) = 0;								// Notification event on Action button / hyperlink clicked
	virtual void	eventGenerateReport(void) = 0;											// Generate Plugin report

protected:
	// For Debug
	void			eventSetSetting_Display(const QString & strSetting);						// Message Box with event details
	void			eventAbort_Display();														// Message Box with event details
	void			eventBeginData_Display();													// Message Box with event details
	void			eventBeginDataset_Display(const GP_DatasetDef & DatasetDef);				// Message Box with event details
	void			eventDefineParameter_Display(const GP_ParameterDef & ParameterDefinition);	// Message Box with event details
	void			eventLot_Display(const GP_LotDef & LotDefinition, const GP_SiteDescription& clGlobalEquipmentID, const GP_SiteDescriptionMap* pSiteEquipmentIDMap);	// Message Box with event details
	void			eventParameterResult_Display(const GP_DataResult & DataResult);				// Message Box with event details
	void			eventPartResult_Display(const GP_PartResult & PartResult);					// Message Box with event details
	void			eventEndDataset_Display();													// Message Box with event details
	void			eventEndData_Display();														// Message Box with event details
	void			eventSetCommand_Display(const QString & strCommand);						// Message Box with event details
	void			eventExecCommand_Display(const QString & strCommand);						// Message Box with event details
	void			eventAction_Display(const QString & strAction);								// Message Box with event details
	void			eventGenerateReport_Display();												// Message Box with event details
	void			eventInteractiveCmd_Display(const GP_InteractiveCmd & InteractiveCmd);		// Message Box with event details

protected:
	QString				m_strPluginFullPath;
	QString				m_strPluginDirectory;
	QString				m_strPluginCommand;			// Command to be executed
	GP_PluginDef		m_clPluginDef;
	FP_GetPluginID		LibGetPluginID;
	FP_GetPluginName	LibGetPluginName;

	// For Debug
	GP_EventDebug		m_clEventDebug;
	bool				m_bPluginDebug_EventPopups;			// Plugin debug / display event popups: set to true if env. GEX_PLUGINDEBUG_EVENTPOPUPS is 'true'
	bool				m_bPluginDebug_DumpData;			// Plugin debug / dump data structures: set to true if env. GEX_PLUGINDEBUG_DUMPDATA is 'true'
	bool				m_bPluginDebug_HtmlReport;			// Plugin debug / create HTML debug: set to true if env. GEX_PLUGINDEBUG_HTMLREPORT is 'true'
};

#endif

