// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

#ifndef _GEXDB_PLUGIN_ST_TDB_HEADER_
#define _GEXDB_PLUGIN_ST_TDB_HEADER_

// Standard includes

// Qt includes

// Galaxy modules includes

// Local includes
#include "gexdb_plugin_base.h"

#define GEXDB_PLUGIN_ST_TDB_NAME		"ST-TDB GEX Database [Plugin V1.1 B2]"
#define GEXDB_PLUGIN_ST_TDB_BUILD		2

///////////////////////////////////////////////////////////
// GexDbPlugin_StTdb class: database plugin class for ST TDB database type
// This class is exported from the gexdb_plugin_st_tdb.dll
///////////////////////////////////////////////////////////
class GexDbPlugin_StTdb : public GexDbPlugin_Base
{
// Constructor / Destructor
public:
	GexDbPlugin_StTdb(QApplication* qApplication, QMainWindow *pMainWindow, const QString & strHostName, const QString & strApplicationPath, const QString & strUserProfile, const QString & strLocalFolder, const char *gexLabelFilterChoices[], const bool bCustomerDebugMode, CGexSkin * pGexSkin, GexDbPlugin_Connector *pclDatabaseConnector=NULL);
	virtual ~GexDbPlugin_StTdb();

// Member functions
public:
	// Common functions
	void			GetPluginName(QString & strPluginName) { strPluginName = GEXDB_PLUGIN_ST_TDB_NAME; }
	unsigned int	GetPluginBuild(void) { return GEXDB_PLUGIN_ST_TDB_BUILD; }
	void			GetSupportedTestingStages(QString& /*strSupportedTestingStages*/) { return; }
	bool			SetTestingStage(const QString& /*strTestingStage*/) { return true; }
	bool			IsInsertionSupported() { return false; }
	bool			IsUpdateSupported() { return false; }
	bool			IsTestingStagesSupported() { return false; }
	bool			IsParameterSelectionSupported() { return false; }
	bool			IsEnterpriseReportsSupported() { return false; }
	bool			IsTestingStage_FinalTest(const QString& /*strTestingStage*/) { return false; }
	bool			IsTestingStage_Foundry(const QString& /*strTestingStage*/) { return false; }
	bool			GetTestingStageName_Foundry(QString & strTestingStage) { strTestingStage = ""; return true; }
	void			GetLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices);

	// Plugin configuration
	bool				ConfigWizard();
	bool				LoadSettings(QFile *pSettingsFile);			// Load settings from file and init the calls variables
	bool				WriteSettings(QTextStream *phFile);			// Write settings to file using informations loaded in the class variables
	
	// Data insertion (not supported by this plugin)

	// Data extraction for GEX standard reports
	bool			QueryField(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bSoftBin=false,bool bClearQueryFirst=true,bool bDistinct=true, QuerySort eQuerySort=eQuerySort_Asc);		// Return all valid values for a field (given filters on other fields)
	bool			QueryTestlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues, bool bParametricOnly);		// Return all tests (given filters on other fields)
	bool			QueryDataFiles(GexDbPlugin_Filter & cFilters, const QString & strTestlist, GexDbPlugin_DataFileList & cMatchingFiles, const QString & strDatabasePhysicalPath, const QString & strLocalDir, bool *pbFilesCreatedInFinalLocation, GexDbPlugin_Base::StatsSource eStatsSource);	// Return all valid Data files (given filters on several fields)

// Member variables
public:

// Implementation
protected:
	// Variables
	GexDbPlugin_Mapping_FieldMap	m_mapFields_GexToRemote;			// Fields mapping: GEX <-> Custom DB (E-Test)
	QString							m_strTableName;						// Table to use in DB
	QMap<QString,QString>			m_cFieldsMapping;					// Holds mapping Fields from Remote DB to Galaxy DB
	GexDbPlugin_FtpSettings			m_clFtpSettings;					// Ftp settings
	QStringList						m_strlLabelFilterChoices;

	// Functions
	bool			ConstructStdfFilesQuery(GexDbPlugin_Filter &cFilters, QString & strQuery);	// Construct query to retrieve all STDF files corresponding to specified filters
	bool			ConnectToCorporateDb();											// Connect to corporate DB and validate current settings (make sure current table exists, make sure fields used in mapping exists)
	void			UpdateBaseMapping();
	bool			Query_AddTimePeriodCondition(GexDbPlugin_Filter &cFilters);		// Add condition if user selected a time period
};

extern "C" GEXDB_PLUGIN_API GexDbPlugin_Base *gexdb_plugin_getobject(QApplication* qApplication, QMainWindow *pMainWindow, const QString & strHostName, const QString & strApplicationPath, const QString & strUserProfile, const QString & strLocalFolder, const char *gexLabelFilterChoices[], CGexSkin * pGexSkin, const bool bCustomerDebugMode);
extern "C" GEXDB_PLUGIN_API void gexdb_plugin_releaseobject(GexDbPlugin_Base *pObject);

#endif // _GEXDB_PLUGIN_ST_TDB_HEADER_
