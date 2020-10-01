// gexdb_plugin_st_tdb.cpp: implementation of the GexDbPlugin_StTdb class.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes: defines the entry point for the DLL application.
//
// ----------------------------------------------------------------------------------------------------------

// Local includes
#include "gexdb_plugin_st_tdb.h"
#include "gexdb_plugin_st_tdb_cfgwizard.h"
#include "gexdb_plugin_st_tdb_constants.h"
#include "gexdb_plugin_common.h"

// Standard includes

// Qt includes
#include <QSqlQuery>
#include <QMainWindow>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>


// Galaxy modules includes

// ----------------------------------------------------------------------------------------------------------
// Only declare a DLL entry point under Windows.
// Under UNIX system, nothing needed for a shared library.
// ----------------------------------------------------------------------------------------------------------
#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#	include <windows.h>

BOOL APIENTRY DllMain( HANDLE /*hModule*/,
                       DWORD  ul_reason_for_call, 
					   LPVOID /*lpReserved*/
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif // defined(_WIN32)

extern "C" GEXDB_PLUGIN_API GexDbPlugin_Base *gexdb_plugin_getobject(QMainWindow *pMainWindow, const QString & strHostName, const QString & strApplicationPath, const QString & strUserProfile, const QString & strLocalFolder, const char *gexLabelFilterChoices[], CGexSkin * pGexSkin, const bool bCustomerDebugMode)
{
    GexDbPlugin_StTdb *pObject = new GexDbPlugin_StTdb(pMainWindow, strHostName, strApplicationPath, strUserProfile, strLocalFolder, gexLabelFilterChoices, bCustomerDebugMode, pGexSkin);
	return (GexDbPlugin_Base *)pObject;
}

extern "C" GEXDB_PLUGIN_API void gexdb_plugin_releaseobject(GexDbPlugin_Base *pObject)
{
	delete pObject;
}

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_StTdb class: database plugin class for ST TDB database type
////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexDbPlugin_StTdb::GexDbPlugin_StTdb(QMainWindow *pMainWindow, const QString & strHostName, const QString & strApplicationPath, const QString & strUserProfile, const QString & strLocalFolder, const char *gexLabelFilterChoices[], const bool bCustomerDebugMode, CGexSkin * pGexSkin, GexDbPlugin_Connector *pclDatabaseConnector) :
GexDbPlugin_Base(pMainWindow, strHostName, strApplicationPath, strUserProfile, strLocalFolder, gexLabelFilterChoices, bCustomerDebugMode, pGexSkin, pclDatabaseConnector)
{
	// Update list of GEX fields: add some fields specific to this plugin
	m_strlGexFields.append(GEXDB_PLUGIN_ST_TDB_FIELD_NONE);
	m_strlGexFields.append(GEXDB_PLUGIN_ST_TDB_FIELD_DATAFILE);
	m_strlGexFields.append(GEXDB_PLUGIN_ST_TDB_FIELD_HOSTNAME);	
	m_strlGexFields.append(GEXDB_PLUGIN_DBFIELD_STARTDATETIME);

	// Variables init
	m_strPluginName = GEXDB_PLUGIN_ST_TDB_NAME;
}

GexDbPlugin_StTdb::~GexDbPlugin_StTdb()
{
}

//////////////////////////////////////////////////////////////////////
// Call plugin configuration wizard
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_StTdb::ConfigWizard()
{
	// Create Connector, unless not privately owned
	if(m_bPrivateConnector)
	{
		// Create a new connector?
		if(!m_pclDatabaseConnector)
			m_pclDatabaseConnector = new GexDbPlugin_Connector(m_strPluginName);
	}

	// Create Wizard
	GexDbPlugin_StTdb_CfgWizard clWizard(m_strlGexFields, m_pGexSkin, m_pMainWindow);
	clWizard.Set(*m_pclDatabaseConnector, m_clFtpSettings, m_strTableName, m_cFieldsMapping);
	if(clWizard.exec() == QDialog::Accepted)
	{
		// Retrieve values from wizard
		clWizard.Get(*m_pclDatabaseConnector, m_clFtpSettings, m_strTableName, m_cFieldsMapping);

		// Update base mapping
		UpdateBaseMapping();

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// Load plugin settings from file
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_StTdb::LoadSettings(QFile *pSettingsFile)
{
	GexDbPlugin_Base::LoadSettings(pSettingsFile);

	// Load settings specific to this DB plugin here...
	if(!m_clFtpSettings.LoadSettings(pSettingsFile))
	{
		GSET_ERROR0(GexDbPlugin_Base, eReadSettings, GGET_LASTERROR(GexDbPlugin_FtpSettings, &m_clFtpSettings));
		return false;
	}

	// Load Mapping
	QString		strString;
	QString		strKeyword;
	QString		strParameter;
	QTextStream hFile(pSettingsFile);

	// Rewind file first
	pSettingsFile->reset();
	
	// Clear any previous mapping list that exists
	m_cFieldsMapping.clear();

	// Search for marker used by this object's settings
	while(!hFile.atEnd())
	{
		strString = hFile.readLine().stripWhiteSpace();
		if(strString.lower() == "<mapping>")
			break;
	}
	if(hFile.atEnd())
	{
		GSET_ERROR1(GexDbPlugin_Base, eMarkerNotFound, NULL, "<Mapping>");
		return false;	// Failed reading file.
	}

	while(!hFile.atEnd())
	{
		// Read line.
		strString = hFile.readLine().stripWhiteSpace();
		strKeyword = strString.section('=',0,0);
		strParameter = strString.section('=',1);

		if(strString.lower() == "</mapping>")
			break;

		else if(strKeyword.lower() == "tablename")
			m_strTableName = strParameter;

		// Save mapping
		else
			m_cFieldsMapping[strKeyword] = strParameter;
	}

	// Update base mapping
	UpdateBaseMapping();

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////
// Write plugin settings to file
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_StTdb::WriteSettings(QTextStream *phFile)
{
	GexDbPlugin_Base::WriteSettings(phFile);

	// Write Ftp settings
	if(!m_clFtpSettings.WriteSettings(phFile))
	{
		GSET_ERROR0(GexDbPlugin_Base, eWriteSettings, GGET_LASTERROR(GexDbPlugin_FtpSettings, &m_clFtpSettings));
		return false;
	}

	// Write Mapping
	*phFile << endl;
	*phFile << "<Mapping>" << endl;
	*phFile << "TableName=" << m_strTableName << endl;
	QMap<QString,QString>::Iterator itMappingField;
	for(itMappingField = m_cFieldsMapping.begin(); itMappingField != m_cFieldsMapping.end(); ++itMappingField)
	{
		*phFile << itMappingField.key().latin1();	// Remote database Field
		*phFile << "=";
		*phFile << itMappingField.data().latin1();	// Galaxy mapped Field
		*phFile << endl;
	}
	// Close mapping section
	*phFile << "</Mapping>" << endl;

	return true;
}

//////////////////////////////////////////////////////////////////////
// Return all valid Data files (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool
GexDbPlugin_StTdb::
QueryDataFiles(GexDbPlugin_Filter& cFilters,
			   const QString& /*strTestlist*/,
			   GexDbPlugin_DataFileList& cMatchingFiles,
			   const QString& /*strDatabasePhysicalPath*/,
			   const QString& /*strLocalDir*/,
			   bool* /*pbFilesCreatedInFinalLocation*/,
			   GexDbPlugin_Base::StatsSource /*eStatsSource*/)
{
	// Clear returned list of matching files
	cMatchingFiles.Clear();

	// Check database connection
	if(!ConnectToCorporateDb())
		return false;

	// Compute query date constraints
	Query_ComputeDateConstraints(cFilters);

	// Construct SQL query
	QString strQuery;
	if(!ConstructStdfFilesQuery(cFilters, strQuery))
		return false;

	// Execute query
	QSqlQuery	clQuery(QString::null, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
	clQuery.setForwardOnly(true);
	if(!clQuery.exec(strQuery))
	{
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQuery.lastError().text().latin1());
		strQuery = clQuery.lastError().text();
		return false;
	}

	// Populate matching file list
	GexDbPlugin_DataFile	*pStdfFile;
	QString					strFilename, strHostname;
	QFileInfo				clFileInfo;
	while(clQuery.next())
	{
		strFilename = clQuery.value(0).toString().trimmed();
		if(m_clFtpSettings.m_bHostnameFromDbField)
			strHostname = clQuery.value(1).toString().trimmed();
		else 
			strHostname = m_clFtpSettings.m_strHostName;
		if(!strFilename.isEmpty() && !strHostname.isEmpty())
		{
			pStdfFile = new GexDbPlugin_DataFile;
			clFileInfo.setFile(strFilename);
			pStdfFile->m_uiPort = m_clFtpSettings.m_uiPort;
			pStdfFile->m_strFileName = clFileInfo.fileName();
			pStdfFile->m_strFilePath = clFileInfo.dirPath();
			pStdfFile->m_strHostName = strHostname;
			pStdfFile->m_strPassword = m_clFtpSettings.m_strPassword;
			pStdfFile->m_strUserName = m_clFtpSettings.m_strUserName;

			// Append to list
			cMatchingFiles.append(pStdfFile);
		}
	}

	return true;
}

///////////////////////////////////////////////////////////
// Query: construct query string for STDF files query
///////////////////////////////////////////////////////////
bool GexDbPlugin_StTdb::ConstructStdfFilesQuery(GexDbPlugin_Filter &cFilters, QString & strQuery)
{
	QString strDbField, strDbTable;

	// Clear query string
	strQuery = "";

	// Make sure we have a field to query on
	// Get DB field names for STDF filename field and Ftp Hostname field
	QString						strMissingFields = "";
	GexDbPlugin_Mapping_Field	clFieldMapping_DataFile, clFieldMapping_HostName;
	if(!m_mapFields_GexToRemote.ContainsGexField(GEXDB_PLUGIN_ST_TDB_FIELD_DATAFILE, clFieldMapping_DataFile) )
	{
		strMissingFields += "'";
		strMissingFields += GEXDB_PLUGIN_ST_TDB_FIELD_DATAFILE;
		strMissingFields += "' ";
	}
	if(m_clFtpSettings.m_bHostnameFromDbField && (!m_mapFields_GexToRemote.ContainsGexField(GEXDB_PLUGIN_ST_TDB_FIELD_HOSTNAME, clFieldMapping_HostName)))
	{
		strMissingFields += "'";
		strMissingFields += GEXDB_PLUGIN_ST_TDB_FIELD_HOSTNAME;
		strMissingFields += "' ";
	}
	if(!strMissingFields.isEmpty())
	{
		GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, strMissingFields.latin1());
		return false;
	}

	// Construct query string:
	// SELECT <testnum, testname>
	// FROM <all required tables>
	// WHERE <link conditions> AND <value conditions>	(optional)
	Query_Empty();
	Query_AddField(clFieldMapping_DataFile.m_strMetaDataName, strDbField, strDbTable);
	if(m_clFtpSettings.m_bHostnameFromDbField)
		Query_AddField(clFieldMapping_HostName.m_strMetaDataName, strDbField, strDbTable);
	// Set filters
	Query_AddFilters(cFilters);

	// Add time period condition
	Query_AddTimePeriodCondition(cFilters);

	// Construct query from table and conditions
	Query_BuildSqlString(strQuery);

	return true;
}

///////////////////////////////////////////////////////////
// Query: add time period condition to query
///////////////////////////////////////////////////////////
bool GexDbPlugin_StTdb::Query_AddTimePeriodCondition(GexDbPlugin_Filter &cFilters)
{
	// Check if time period should be used
	if(!cFilters.bUseTimePeriod)
		return true;

	// Add condition based on dates computed from time_t from and to values
	QDateTime	clDateTimeFrom, clDateTimeTo;
	QString		strFilterCondition;

	clDateTimeFrom.setTime_t(cFilters.tQueryFrom);
	clDateTimeTo.setTime_t(cFilters.tQueryTo);

	strFilterCondition =	"to_date('";
	strFilterCondition +=	clDateTimeFrom.toString("yyyy_MM_dd_hh:mm:ss");
	strFilterCondition +=	"', 'YYYY_MM_DD_HH24:MI:SS')";
	strFilterCondition +=	GEXDB_PLUGIN_DELIMITER_FROMTO;
	strFilterCondition +=	"to_date('";
	strFilterCondition +=	clDateTimeTo.toString("yyyy_MM_dd_hh:mm:ss");
	strFilterCondition +=	"', 'YYYY_MM_DD_HH24:MI:SS')";

	return Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, strFilterCondition, true);
}

///////////////////////////////////////////////////////////
// Connect to corporate DB...
///////////////////////////////////////////////////////////
bool GexDbPlugin_StTdb::ConnectToCorporateDb()
{
	if(!GexDbPlugin_Base::ConnectToCorporateDb())
		return false;

	// Retrieve list of tables
	QStringList strlTables;
	if(!m_pclDatabaseConnector->EnumTables(strlTables))
	{
		// Error: set error and return false
		GSET_ERROR1(GexDbPlugin_Base,
					eDB_EnumTables,
					GGET_LASTERROR(GexDbPlugin_Connector,
								   m_pclDatabaseConnector),
					m_pclDatabaseConnector->
					m_strDatabaseName.toStdString().c_str());
		return false;
	}

	// Update field mapping
	int nItemsRemoved, nItemsAdded;
	if(!m_pclDatabaseConnector->UpdateMapping(&nItemsRemoved, &nItemsAdded, m_strTableName, m_strlGexFields, m_cFieldsMapping))
	{
		// Error: set error and return false
		GSET_ERROR1(GexDbPlugin_Base,
					eDB_UpdateMapping,
					GGET_LASTERROR(GexDbPlugin_Connector,
								   m_pclDatabaseConnector),
					m_pclDatabaseConnector->
					m_strDatabaseName.toStdString().c_str());
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Return all tests (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_StTdb::QueryTestlist(GexDbPlugin_Filter& /*cFilters*/,
									  QStringList& cMatchingValues,
									  bool /*bParametricOnly*/)
{
	// Clear returned stringlist
	cMatchingValues.clear();

	return true;
}

//////////////////////////////////////////////////////////////////////
// Return all valid values for a field (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_StTdb::QueryField(GexDbPlugin_Filter& cFilters,
								   QStringList& cMatchingValues,
								   bool bSoftBin /*= false*/,
								   bool bClearQueryFirst /*= true*/,
								   bool bDistinct /*= true*/,
								   QuerySort /*eQuerySort = eQuerySort_Asc*/)
{
	m_pmapFields_GexToRemote = &m_mapFields_GexToRemote;
	m_pmapLinks_Remote = NULL;
	return GexDbPlugin_Base::QueryField(cFilters, cMatchingValues, bSoftBin, bClearQueryFirst, bDistinct, eQuerySort_None);
}

///////////////////////////////////////////////////////////
// Update Field Map of Base class
// (map used in base class to generate SQL statements)
///////////////////////////////////////////////////////////
void GexDbPlugin_StTdb::UpdateBaseMapping()
{
	m_mapFields_GexToRemote.clear();

#if 1
	// Gex fields <-> Medtronic DB fields mapping
	// 1. Fields exported in the GEX GUI
	m_mapFields_GexToRemote["CAM_PRODUCT"] =
		GexDbPlugin_Mapping_Field("CAM_PRODUCT", "", 
		m_strTableName,		m_strTableName+ ".CAM_PRODUCT",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["CERTIFICATION_FLAG"] =
		GexDbPlugin_Mapping_Field("CERTIFICATION_FLAG", "", 
		m_strTableName,		m_strTableName+ ".CERTIFICATION_FLAG",		"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["CMOD_CODE"] =
		GexDbPlugin_Mapping_Field("CMOD_CODE", "", 
		m_strTableName,		m_strTableName+ ".CMOD_CODE",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["DATATYPE"] =
		GexDbPlugin_Mapping_Field("DATATYPE", "", 
		m_strTableName,		m_strTableName+ ".DATATYPE",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["FAMLY_ID"] =
		GexDbPlugin_Mapping_Field("FAMLY_ID", "", 
		m_strTableName,		m_strTableName+ ".FAMLY_ID",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["FILE_HOST"] =
		GexDbPlugin_Mapping_Field("FILE_HOST", "Ftp Hostname", 
		m_strTableName,		m_strTableName+ ".FILE_HOST",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["FILE_NAME"] =
		GexDbPlugin_Mapping_Field("FILE_NAME", "Test Data Filename", 
		m_strTableName,		m_strTableName+ ".FILE_NAME",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["FLOW_ID"] =
		GexDbPlugin_Mapping_Field("FLOW_ID", "", 
		m_strTableName,		m_strTableName+ ".FLOW_ID",					"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["JOB_NAME"] =
		GexDbPlugin_Mapping_Field("JOB_NAME", "Program name", 
		m_strTableName,		m_strTableName+ ".JOB_NAME",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["JOB_REV"] =
		GexDbPlugin_Mapping_Field("JOB_REV", "Program rev.", 
		m_strTableName,		m_strTableName+ ".JOB_REV",					"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["LAST"] =
		GexDbPlugin_Mapping_Field("LAST", "", 
		m_strTableName,		m_strTableName+ ".LAST",					"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["LOAD_ERROR"] =
		GexDbPlugin_Mapping_Field("LOAD_ERROR", "", 
		m_strTableName,		m_strTableName+ ".LOAD_ERROR",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["LOT_ID"] =
		GexDbPlugin_Mapping_Field("LOT_ID", "Lot ID", 
		m_strTableName,		m_strTableName+ ".LOT_ID",					"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["MODE_CODE"] =
		GexDbPlugin_Mapping_Field("MODE_CODE", "", 
		m_strTableName,		m_strTableName+ ".MODE_CODE",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["NODE_NAME"] =
		GexDbPlugin_Mapping_Field("NODE_NAME", "Tester name", 
		m_strTableName,		m_strTableName+ ".NODE_NAME",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["OPERATOR_NAME"] =
		GexDbPlugin_Mapping_Field("OPERATOR_NAME", "Operator name", 
		m_strTableName,		m_strTableName+ ".OPERATOR_NAME",			"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["PART_TYP"] =
		GexDbPlugin_Mapping_Field("PART_TYP", "Product ID", 
		m_strTableName,		m_strTableName+ ".PART_TYP",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote[GEXDB_PLUGIN_DBFIELD_STARTDATETIME] =
		GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, GEXDB_PLUGIN_DBFIELD_STARTDATETIME, 
		m_strTableName,		m_strTableName+ ".START_T",					"",			false, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["SUBLOT_ID"] =
		GexDbPlugin_Mapping_Field("SUBLOT_ID", "Sublot ID", 
		m_strTableName,		m_strTableName+ ".SUBLOT_ID",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["TEST_AREA"] =
		GexDbPlugin_Mapping_Field("TEST_AREA", "", 
		m_strTableName,		m_strTableName+ ".TEST_AREA",				"",			true, "N", false, false, false, false, false, false, false);
	m_mapFields_GexToRemote["TESTER_TYPE"] =
		GexDbPlugin_Mapping_Field("TESTER_TYPE", "Tester type", 
		m_strTableName,		m_strTableName+ ".TESTER_TYPE",				"",			true, "N", false, false, false, false, false, false, false);

	// List of available filters
	GexDbPlugin_Mapping_FieldMap::Iterator itMapping;
	for(itMapping = m_mapFields_GexToRemote.begin(); itMapping != m_mapFields_GexToRemote.end(); itMapping++)
	{
		if((*itMapping).m_bDisplayInQueryGui)
			m_strlLabelFilterChoices.append((*itMapping).m_strMetaDataName);
	}
#else
	// Go through local map, and update base map
	QString							strDbFullField, strStTdbName, strGexName, strMetaDataName;
	QMap<QString,QString>::Iterator	itMappingField;
	for(itMappingField = m_cFieldsMapping.begin(); itMappingField != m_cFieldsMapping.end(); ++itMappingField)
	{
		if(!itMappingField.data().isEmpty())
		{
			strStTdbName = itMappingField.key();
			strGexName = itMappingField.data();
			strMetaDataName = strGexName;
			strDbFullField = m_strTableName;
			strDbFullField += ".";
			strDbFullField += strStTdbName;
			if(strGexName == GEXDB_PLUGIN_DBFIELD_STARTDATETIME)
				m_mapFields_GexToRemote[strGexName] = GexDbPlugin_Mapping_Field(strGexName, strGexName, m_strTableName, strDbFullField, "", false);
			else
			{
				m_mapFields_GexToRemote[strMetaDataName] = GexDbPlugin_Mapping_Field(strMetaDataName, strGexName, m_strTableName, strDbFullField, "", true);
				m_strlLabelFilterChoices.append(strMetaDataName);
			}
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////
// Return mapped filter labels
//////////////////////////////////////////////////////////////////////
void
GexDbPlugin_StTdb::GetLabelFilterChoices(const QString& /*strDataTypeQuery*/,
										 QStringList& strlLabelFilterChoices)
{
	strlLabelFilterChoices.clear();
	strlLabelFilterChoices = m_strlLabelFilterChoices;
}

