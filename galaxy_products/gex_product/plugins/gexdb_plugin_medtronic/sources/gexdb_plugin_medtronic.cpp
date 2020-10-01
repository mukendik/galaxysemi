// gexdb_plugin_medtronic.cpp: implementation of the GexDbPlugin_Medtronic class.
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
#include "gexdb_plugin_medtronic.h"
#include "gexdb_plugin_medtronic_cfgwizard.h"
#include "gexdb_plugin_medtronic_rdb_optionswidget.h"
#include "gex_shared.h"
#include "gex_constants.h"
#include "abstract_query_progress.h"
#include "gqtl_log.h"

// Standard includes

// Qt includes
#include <QMainWindow>
#include <QSqlError>
#include <QDir>
#include <QMessageBox>
#include <QListIterator>

// Galaxy modules includes
#include <gqtl_sysutils.h>

////////////////////////////////////////////////////////////////////////////////////
// Constants and Macro definitions
////////////////////////////////////////////////////////////////////////////////////
#define GEXDB_PLUGIN_MEDTRONIC_MERGE_FIRSTTEST_LASTTEST			1
#define GEXDB_PLUGIN_MEDTRONIC_CREATE_TESTLIST_ONTHEFLY			1

#define GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_MODEL				"Unit Model"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_TYPE				"Unit Type"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT				"Unit Lot"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_SUBLOT					"Sublot (WaferID or SublotID)"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_SERIAL				"Unit Serial"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_PIN				"Unit PIN"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PF_STATUS			"Test P/F Status"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PURPOSE			"Test Purpose"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_OPERATION			"Test Operation"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_FACILITY				"Facility"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID				"Test Set ID"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PACKAGE_ID			"Test Package ID"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_OPERATOR_ID				"Operator ID"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SERIAL			"Station Serial"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_PIN				"Station Pin"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_MODEL			"Station Model"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SOFTWARE_REV	"Station Software Rev"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_ID					"Test ID"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_NAME				"Test Name"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_LABEL				"Test Label"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PARAMETER			"Test Parameter"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_FIRST_TEST_FLAG			"First Test Flag"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_LAST_TEST_FLAG			"Last Test Flag"
#define GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE			"Transfer Date"

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
    return true;
}
#endif // defined(_WIN32)

extern "C" GEXDB_PLUGIN_API GexDbPlugin_Base *gexdb_plugin_getobject(
        const QString & strHostName,
        const QString & strApplicationPath,
        const QString & strUserProfile,
        const QString & strLocalFolder,
        const char *gexLabelFilterChoices[],
        CGexSkin * pGexSkin,
        const GexScriptEngine* gse,
        const bool bCustomerDebugMode)
{
    GexDbPlugin_Medtronic *pObject = new GexDbPlugin_Medtronic(
                strHostName, strApplicationPath, strUserProfile, strLocalFolder,
                gexLabelFilterChoices, bCustomerDebugMode, pGexSkin, gse);
    return (GexDbPlugin_Base *)pObject;
}

extern "C" GEXDB_PLUGIN_API void gexdb_plugin_releaseobject(GexDbPlugin_Base *pObject)
{
    delete pObject;
}

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Medtronic class: database plugin class for MEDTRONIC database type
////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexDbPlugin_Medtronic::GexDbPlugin_Medtronic(const QString & strHostName,
                                             const QString & strApplicationPath,
                                             const QString & strUserProfile,
                                             const QString & strLocalFolder,
                                             const char *gexLabelFilterChoices[],
                                             const bool bCustomerDebugMode,
                                             CGexSkin * pGexSkin,
                                             const GexScriptEngine *gse,
                                             GexDbPlugin_Connector *pclDatabaseConnector) :
    GexDbPlugin_Base(strHostName, strApplicationPath, strUserProfile, strLocalFolder,
                     gexLabelFilterChoices, bCustomerDebugMode, pGexSkin, gse, pclDatabaseConnector)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("new Medtronic Plugin on %1").arg(strHostName).toLatin1().constData());
    m_pclRdbOptionsWidget = NULL;
}

GexDbPlugin_Medtronic::~GexDbPlugin_Medtronic()
{
    // Free ressources
    // ALREADY DELETED ?
    //if(m_pclRdbOptionsWidget)
    //    delete m_pclRdbOptionsWidget;
}

bool GexDbPlugin_Medtronic::Init()
{
    if (!GexDbPlugin_Base::Init())
        return false;

    // Variables init
    m_strPluginName = GEXDB_PLUGIN_MEDTRONIC_NAME;
    m_pclRdbOptionsWidget = new GexDbPlugin_Medtronic_Rdb_OptionsWidget(mParentWidget, Qt::FramelessWindowHint);
    if (!m_pclRdbOptionsWidget)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              "Failed to create a new GexDbPlugin_Medtronic_Rdb_OptionsWidget : m_pclRdbOptionsWidget will be NULL !");
        return false;
    }

    // Call initialization functions
    Init_Extraction();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Call plugin configuration wizard
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::ConfigWizard()
{
    // Create Connector, unless not privately owned
    if(m_bPrivateConnector)
    {
        // Create a new connector?
        if(!m_pclDatabaseConnector)
            m_pclDatabaseConnector = new GexDbPlugin_Connector(m_strPluginName);
    }

    // Create Wizard
    GexDbPlugin_Medtronic_CfgWizard clWizard(m_strlGexFields, m_pGexSkin, mParentWidget);
    clWizard.Set(*m_pclDatabaseConnector);
    if(clWizard.exec() == QDialog::Accepted)
    {
        // Retrieve values from wizard
        clWizard.Get(*m_pclDatabaseConnector);
        return true;
    }

    return false;
}

bool GexDbPlugin_Medtronic::LoadMetaData()
{
    return true;
}

///////////////////////////////////////////////////////////
// Get nb of Splitlots matching query
///////////////////////////////////////////////////////////
int GexDbPlugin_Medtronic::GetNbOfSplitlots(GexDbPlugin_Filter &cFilters)
{
    QString					strSubQuery, strQuery, strFieldSpec, strDbField, strDbTable, strQueryFilter;
    GexDbPlugin_Filter		cTempFilters = cFilters;
    QStringList::iterator	it;

    // If unit_serial is part of the filters, remove it. This filter will only be used to filter on specific parts
    // when retrieving the data
    cTempFilters.strlQueryFilters.clear();
    for(it = cFilters.strlQueryFilters.begin(); it != cFilters.strlQueryFilters.end(); it++)
    {
        if((*it).section('=', 0, 0) != GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_SERIAL)
            cTempFilters.strlQueryFilters.append((*it));
    }

    // Construct query string:
    // SELECT count(waf_prober_info_id)
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    Query_Empty();
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_MODEL, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_TYPE, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_SUBLOT, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_PIN, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PURPOSE, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_OPERATION, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_FACILITY, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PACKAGE_ID, strDbField, strDbTable);
    //	Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_OPERATOR_ID, strDbField, strDbTable);
    //	Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SERIAL, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_PIN, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_MODEL, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SOFTWARE_REV, strDbField, strDbTable);
    //	Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE, strDbField, strDbTable);
#if !GEXDB_PLUGIN_MEDTRONIC_MERGE_FIRSTTEST_LASTTEST
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_FIRST_TEST_FLAG, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_LAST_TEST_FLAG, strDbField, strDbTable);
#endif

    // Set filters
    Query_AddFilters(cTempFilters);

    // Add time period condition
    Query_AddTimePeriodCondition(cFilters);

    // Construct query from table and conditions
    Query_BuildSqlString(strSubQuery, true);

    // Construct query string:
    // SELECT count(*)
    // FROM (subquery)
    strQuery = "SELECT\n";
    strQuery += "count(*)\n";
    strQuery += "FROM\n(";
    strQuery += strSubQuery;
    if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        strQuery += ") AS T1\n";
    else
        strQuery += ")\n";

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        return -1;
    }

    clGexDbQuery.Next();

    return clGexDbQuery.value(0).toInt();
}

///////////////////////////////////////////////////////////
// Query: construct query string for Splitlot query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::ConstructSplitlotQuery(GexDbPlugin_Filter &cFilters, QString & strQuery)
{
    QString					strDbField, strDbTable;
    GexDbPlugin_Filter		cTempFilters = cFilters;
    QStringList::iterator	it;

    // If unit_serial is part of the filters, remove it. This filter will only be used to filter on specific parts
    // when retrieving the data
    cTempFilters.strlQueryFilters.clear();
    for(it = cFilters.strlQueryFilters.begin(); it != cFilters.strlQueryFilters.end(); it++)
    {
        if((*it).section('=', 0, 0) != GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_SERIAL)
            cTempFilters.strlQueryFilters.append((*it));
    }

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <testnum, testname>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields
    Query_Empty();
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_MODEL, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_TYPE, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_SUBLOT, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_PIN, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PURPOSE, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_OPERATION, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_FACILITY, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PACKAGE_ID, strDbField, strDbTable);
    //	Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_OPERATOR_ID, strDbField, strDbTable);
    //	Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SERIAL, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_PIN, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_MODEL, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SOFTWARE_REV, strDbField, strDbTable);
    //	Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE, strDbField, strDbTable);
#if !GEXDB_PLUGIN_MEDTRONIC_MERGE_FIRSTTEST_LASTTEST
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_FIRST_TEST_FLAG, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_LAST_TEST_FLAG, strDbField, strDbTable);
#endif

    // Set filters
    Query_AddFilters(cTempFilters);

    // Add time period condition
    Query_AddTimePeriodCondition(cFilters);

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery);

    return true;
}

//////////////////////////////////////////////////////////////////////
// Fill Splitlot info object for specified Splitlot_id
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic::FillSplitlotInfo(const GexDbPlugin_Query & clGexDbQuerySplitlot, GexDbPlugin_Medtronic_SplitlotInfo	*pclSplitlotInfo)
{
    int nIndex = 0;

    pclSplitlotInfo->m_strUnitModel				= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Unit_Model
    pclSplitlotInfo->m_strUnitType				= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Unit_Type
    pclSplitlotInfo->m_strUnitLot				= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Unit_Lot
    pclSplitlotInfo->m_strSublot				= clGexDbQuerySplitlot.value(nIndex++).toString();		// Sublot (WaferID or SublotID) extracted from Unit_Serial
    pclSplitlotInfo->m_strUnitPin				= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Unit_Pin
    pclSplitlotInfo->m_strTestPurpose			= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Test_Purpose
    pclSplitlotInfo->m_strTestOperation			= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Test_Operation
    pclSplitlotInfo->m_strTestFacility			= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Test_Facility
    pclSplitlotInfo->m_strTestSetID				= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Test_Set_ID
    pclSplitlotInfo->m_strTestPackageID			= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Test_Package_ID
    //pclSplitlotInfo->m_strTestOperatorID		= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Test_Operator_ID
    //pclSplitlotInfo->m_strStationSerial		= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Station_Serial
    pclSplitlotInfo->m_strStationPin			= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Station_Pin
    pclSplitlotInfo->m_strStationModel			= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Station_Model
    pclSplitlotInfo->m_strStationSoftwareRev	= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Station_Software_Rev
    //pclSplitlotInfo->m_strTransferDate		= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Transfer_Date
#if !GEXDB_PLUGIN_MEDTRONIC_MERGE_FIRSTTEST_LASTTEST
    pclSplitlotInfo->m_strFirstTestFlag			= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.First_Test_Flag
    pclSplitlotInfo->m_strLastTestFlag			= clGexDbQuerySplitlot.value(nIndex++).toString();		// <splitlot table>.Last_Test_Flag
#endif
}


//////////////////////////////////////////////////////////////////////
// Create a STDF file for specified Splitlot_id
//////////////////////////////////////////////////////////////////////
bool
GexDbPlugin_Medtronic::
CreateStdfFile(GexDbPlugin_Filter& cFilters,
               const GexDbPlugin_Query& clGexDbQuerySplitlot,
               const QString& strTestlist,
               const QString& strDatabasePhysicalPath,
               const QString& /*strLocalDir*/,
               QString& strStdfFileName,
               QString& strStdfFilePath)
{
    GexDbPlugin_Medtronic_SplitlotInfo	clSplitlotInfo;
    QDateTime							clDateTime;
    QString								strMessage, strQueryFilter, strQueryValue;
    QDir								cDir;
    CGexSystemUtils						clSysUtils;

    strStdfFileName = "";
    m_strFirstTestFlag = m_strLastTestFlag = "X";

    // Fill SplitlotInfo object
    FillSplitlotInfo(clGexDbQuerySplitlot, &clSplitlotInfo);

    // Set date constraints
    clSplitlotInfo.m_clMinTestDate.setDate(cFilters.calendarFrom);
    clSplitlotInfo.m_clMaxTestDate.setDate(cFilters.calendarTo);

    // Build file name
    strStdfFileName = "DMAS";
    if(!clSplitlotInfo.m_strUnitType.isEmpty())
    {
        strStdfFileName += "_";
        strStdfFileName += clSplitlotInfo.m_strUnitType;
    }
    if(!clSplitlotInfo.m_strUnitModel.isEmpty())
    {
        strStdfFileName += "_";
        strStdfFileName += clSplitlotInfo.m_strUnitModel;
    }
    if(!clSplitlotInfo.m_strUnitLot.isEmpty())
    {
        strStdfFileName += "_";
        strStdfFileName += clSplitlotInfo.m_strUnitLot;
    }
    if(!clSplitlotInfo.m_strSublot.isEmpty())
    {
        strStdfFileName += "_";
        strStdfFileName += clSplitlotInfo.m_strSublot;
    }
    if(!clSplitlotInfo.m_strTestFacility.isEmpty())
    {
        strStdfFileName += "_";
        strStdfFileName += clSplitlotInfo.m_strTestFacility;
    }
    if(!clSplitlotInfo.m_strTestPurpose.isEmpty())
    {
        strStdfFileName += "_";
        strStdfFileName += clSplitlotInfo.m_strTestPurpose;
    }
    if(!clSplitlotInfo.m_strTestOperation.isEmpty())
    {
        strStdfFileName += "_";
        strStdfFileName += clSplitlotInfo.m_strTestOperation;
    }
    if(!clSplitlotInfo.m_strTestSetID.isEmpty())
    {
        strStdfFileName += "_";
        strStdfFileName += clSplitlotInfo.m_strTestSetID;
    }
    strStdfFileName += "_";
    switch(m_clOptions.m_eTestMergeOption)
    {
    case GexDbPlugin_Medtronic_Options::eKeepFirstResult:
        strStdfFileName += "F";
        break;
    case GexDbPlugin_Medtronic_Options::eKeepAllResults:
        strStdfFileName += "A";
        break;
    case GexDbPlugin_Medtronic_Options::eKeepLastResult:
    default:
        strStdfFileName += "L";
        break;
    }
#if !GEXDB_PLUGIN_MEDTRONIC_MERGE_FIRSTTEST_LASTTEST
    strStdfFileName += "_";
    if(!clSplitlotInfo.m_strFirstTestFlag.isEmpty())
        m_strFirstTestFlag = clSplitlotInfo.m_strFirstTestFlag;
    strStdfFileName += m_strFirstTestFlag;
    if(!clSplitlotInfo.m_strLastTestFlag.isEmpty())
        m_strLastTestFlag = clSplitlotInfo.m_strLastTestFlag;
    strStdfFileName += m_strLastTestFlag;
#endif

    // Add to the filename those filters that are set but not part of the Splitlot definition (ensure unique filename in case of compare of 2 datasets):
    // test_operator_id
    // station_model
    // test_status_code
    // transfer_date
    // first_test_flag
    // last_test_flag
    //
    // DO NOT ADD unit_serial as is, as this can be a huge selection, resulting in a too big file name
    // Instead, if the unit_serial filter is set, use a hash on the filter value.
    //
    QStringList::iterator	it;
    for(it = cFilters.strlQueryFilters.begin(); it != cFilters.strlQueryFilters.end(); it++)
    {
        strQueryFilter = (*it).section('=', 0, 0);
        strQueryValue = (*it).section('=', 1, 1);
        if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_OPERATOR_ID)
        {
            strStdfFileName += "_";
            strStdfFileName += strQueryValue;
        }
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_MODEL)
        {
            strStdfFileName += "_";
            strStdfFileName += strQueryValue;
        }
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SERIAL)
        {
            strStdfFileName += "_";
            strStdfFileName += strQueryValue;
        }
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PF_STATUS)
        {
            strStdfFileName += "_";
            strStdfFileName += strQueryValue;
        }
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE)
        {
            strStdfFileName += "_";
            strStdfFileName += strQueryValue;
        }
#if 1
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_SERIAL)
        {
            strStdfFileName += "_";
            strStdfFileName += QString::number(qHash(strQueryValue));
        }
#endif
#if GEXDB_PLUGIN_MEDTRONIC_MERGE_FIRSTTEST_LASTTEST
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_FIRST_TEST_FLAG)
        {
            m_strFirstTestFlag = strQueryValue;
            strStdfFileName += "_";
            strStdfFileName += strQueryValue;
        }
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_LAST_TEST_FLAG)
        {
            m_strLastTestFlag = strQueryValue;
            strStdfFileName += "_";
            strStdfFileName += strQueryValue;
        }
#endif
    }

    clSysUtils.NormalizeString(strStdfFileName);
    strStdfFileName += ".stdf";

    // Update query progress dialog
    if (mQueryProgress)
    {
        if((clSplitlotInfo.m_strUnitType.toLower() == "wafer") || (clSplitlotInfo.m_strUnitType.toLower() == "keithley"))
            mQueryProgress->SetFileInfo(clSplitlotInfo.m_strUnitModel, clSplitlotInfo.m_strUnitLot, "", clSplitlotInfo.m_strSublot);
        else
            mQueryProgress->SetFileInfo(clSplitlotInfo.m_strUnitModel, clSplitlotInfo.m_strUnitLot, clSplitlotInfo.m_strSublot, "");
    }

    // Create STDF file path: <Database Phys path>/DMAS_CACHE/<unit_model>/<unit_lot>
    strStdfFilePath = strDatabasePhysicalPath + "/DMAS_CACHE/";
    clSysUtils.NormalizePath(strStdfFilePath);
    cDir.mkdir(strStdfFilePath);

    // Add unit_model to path and create dir
    if(clSplitlotInfo.m_strUnitModel.isEmpty())
        strStdfFilePath += "UNKNOWN_UNIT_MODEL";
    else
        strStdfFilePath += clSplitlotInfo.m_strUnitModel;
    clSysUtils.NormalizePath(strStdfFilePath);
    cDir.mkdir(strStdfFilePath);

    // Add unit_lot to path and create dir
    strStdfFilePath += "/";
    if(clSplitlotInfo.m_strUnitLot.isEmpty())
        strStdfFilePath += "UNKNOWN_UNIT_LOT";
    else
        strStdfFilePath += clSplitlotInfo.m_strUnitLot;
    clSysUtils.NormalizePath(strStdfFilePath);
    cDir.mkdir(strStdfFilePath);

    // Create full STDF file name: <STDF file path>/<STDF file>
    QString	strStdfFullFileName = strStdfFilePath + "/" + strStdfFileName;
    clSysUtils.NormalizePath(strStdfFullFileName);

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        strMessage = "-------- Extracting splitlot: ";
        strMessage += strStdfFileName;
        WriteDebugMessageFile(strMessage, true);
        m_clExtractionPerf.Start(true);
    }

    // Create STDF records
    GQTL_STDF::StdfParse clStdfParse;

    // Open STDF file
    if(!clStdfParse.Open((char *)strStdfFullFileName.toLatin1().constData(), STDF_WRITE))
    {
        return false;
    }

    // Init diferent maps: X,Y matrix with part results, binning map,...
    if(!InitMaps(cFilters, clSplitlotInfo))
    {
        clStdfParse.Close();
        return false;
    }

    // Create testlist
    if(!CreateTestlist(cFilters, clSplitlotInfo, strTestlist))
    {
        clStdfParse.Close();
        return false;
    }

    // Start progress bar for current file
    if (mQueryProgress)
        mQueryProgress->StartFileProgress(m_uiSplitlotNbRuns);

    // Write MIR
    if(!WriteMIR(clStdfParse, clSplitlotInfo))
    {
        clStdfParse.Close();
        return false;
    }

    // Write WIR
    if((clSplitlotInfo.m_strUnitType.toLower() == "wafer") || (clSplitlotInfo.m_strUnitType.toLower() == "keithley"))
    {
        if(!WriteWIR(clStdfParse, clSplitlotInfo))
        {
            clStdfParse.Close();
            return false;
        }
    }

    // Write static test info
    WriteStaticTestInfo(clStdfParse, clSplitlotInfo);

    // Write test results
    if(!WriteTestResults(cFilters, clStdfParse, clSplitlotInfo, strTestlist))
    {
        clStdfParse.Close();
        return false;
    }

    // Write WRR
    if((clSplitlotInfo.m_strUnitType.toLower() == "wafer") || (clSplitlotInfo.m_strUnitType.toLower() == "keithley"))
    {
        if(!WriteWRR(clStdfParse, clSplitlotInfo))
        {
            clStdfParse.Close();
            return false;
        }
    }

#if 0
    // Write WCR
    if(pclWaferInfo && !WriteWCR(clStdfParse, pclWaferInfo))
    {
        delete pclWaferInfo;
        clStdfParse.Close();
        return false;
    }

    // Write summary records
    if(!WriteSummaryRecords(clStdfParse, clSplitlotInfo))
    {
        clStdfParse.Close();
        return false;
    }
#endif

    // Write MRR
    if(!WriteMRR(clStdfParse, clSplitlotInfo))
    {
        clStdfParse.Close();
        return false;
    }

    // Free ressources
    clStdfParse.Close();
    m_clTestList.ClearData(true);

    // Stop progress bar for current file
    if (mQueryProgress)
        mQueryProgress->EndFileProgress();

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        QString strMessage, strPerformance;
        m_clExtractionPerf.Stop();

        if(m_clExtractionPerf.m_fTime_Total >= 1000000.0F)
        {
            strPerformance =  "-------- Execution time           = " + QString::number(m_clExtractionPerf.m_fTime_Total/1000000.0F, 'f', 2) + " s\n";
            strPerformance += "-------- SQL query execution      = " + QString::number(m_clExtractionPerf.m_fTimer_DbQuery_Total/1000000.0F, 'f', 2) + " s\n";
            strPerformance += "-------- SQL query iteration      = " + QString::number(m_clExtractionPerf.m_fTimer_DbIteration_Total/1000000.0F, 'f', 2) + " s\n";
            strPerformance += "-------- SQL query extracted rows = " + QString::number(m_clExtractionPerf.m_ulNbRows_Total) + " \n";
            strPerformance += "-------- Nb of runs               = " + QString::number(m_clExtractionPerf.m_ulNbRuns_Total) + " \n";
            strPerformance += "-------- Nb of test results       = " + QString::number(m_clExtractionPerf.m_ulNbTestResults_Total) + " \n";
            strMessage.sprintf(	"-------- Splitlot %s extracted in %.2f seconds:\n%s\n",
                                strStdfFileName.toLatin1().constData(),
                                m_clExtractionPerf.m_fTime_Total/1000000.0F,
                                strPerformance.toLatin1().constData());
        }
        else
        {
            strPerformance =  "-------- Execution time           = " + QString::number(m_clExtractionPerf.m_fTime_Total/1000.0F, 'f', 2) + " ms\n";
            strPerformance += "-------- SQL query execution      = " + QString::number(m_clExtractionPerf.m_fTimer_DbQuery_Total/1000.0F, 'f', 2) + " ms\n";
            strPerformance += "-------- SQL query iteration      = " + QString::number(m_clExtractionPerf.m_fTimer_DbIteration_Total/1000.0F, 'f', 2) + " ms\n";
            strPerformance += "-------- SQL query extracted rows = " + QString::number(m_clExtractionPerf.m_ulNbRows_Total) + " \n";
            strPerformance += "-------- Nb of runs               = " + QString::number(m_clExtractionPerf.m_ulNbRuns_Total) + " \n";
            strPerformance += "-------- Nb of test results       = " + QString::number(m_clExtractionPerf.m_ulNbTestResults_Total) + " \n";
            strMessage.sprintf(	"-------- Splitlot %s extracted in %d ms:\n%s\n",
                                strStdfFileName.toLatin1().constData(),
                                (int)(m_clExtractionPerf.m_fTime_Total/1000.0F),
                                strPerformance.toLatin1().constData());
        }

        WriteDebugMessageFile(strMessage, true);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all valid Data files (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool
GexDbPlugin_Medtronic::
QueryDataFiles(GexDbPlugin_Filter& cFilters,
               const QString& strTestlist,
               tdGexDbPluginDataFileList & cMatchingFiles,
               const QString& strDatabasePhysicalPath,
               const QString& strLocalDir,
               bool* pbFilesCreatedInFinalLocation,
               GexDbPlugin_Base::StatsSource /*eStatsSource*/)
{
    QString				strQuery;
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QDir				cDir;
    CGexSystemUtils		clSysUtils;

    // Debug message
    WriteDebugMessageFile("**** QueryDataFiles()...");

    // Init plug-in options
    m_clOptions.Init(cFilters.strOptionsString);

    // Clear returned list of matching files
    qDeleteAll(cMatchingFiles);
    cMatchingFiles.clear();

    *pbFilesCreatedInFinalLocation = true;
    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote;
    m_pmapLinks_Remote = &m_mapLinks_Remote;

    // Check database connection
    if(!ConnectToCorporateDb())
    {
        DisplayErrorMessage();
        return false;
    }

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // Get nb of files to generate
    int nNbFiles = GetNbOfSplitlots(cFilters);
    if(nNbFiles == -1)
    {
        DisplayErrorMessage();
        return false;
    }

    // Construct SQL query
    if(!ConstructSplitlotQuery(cFilters, strQuery))
    {
        DisplayErrorMessage();
        return false;
    }

    // Execute query
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        DisplayErrorMessage();
        return false;
    }

    // Show query progress dialog
    if (mQueryProgress)
        mQueryProgress->Start(nNbFiles);
    m_uiTotalRuns = 0;
    m_uiTotalTestResults = 0;

    // Create 1 STDF file for each Splitlot retrieved
    GexDbPlugin_DataFile	*pStdfFile;
    QString					strStdfFileName, strStdfFilePath;
    while(clGexDbQuery.Next())
    {
        if(CreateStdfFile(cFilters, clGexDbQuery, strTestlist, strDatabasePhysicalPath, strLocalDir, strStdfFileName, strStdfFilePath))
        {
            // STDF file successfully created
            pStdfFile = new GexDbPlugin_DataFile;
            pStdfFile->m_strFileName = strStdfFileName;
            pStdfFile->m_strFilePath = strStdfFilePath;
            pStdfFile->m_bRemoteFile = false;

            // Append to list
            cMatchingFiles.append(pStdfFile);
        }
        else
        {
            // Remove STDF file if exist
            QString	strStdfFullFileName = strStdfFilePath + "/" + strStdfFileName;
            clSysUtils.NormalizePath(strStdfFullFileName);
            if(cDir.exists(strStdfFullFileName))
                cDir.remove(strStdfFullFileName);

            // Display error message if not user abort
            if(!mQueryProgress->IsAbortRequested())
                DisplayErrorMessage();

            // Clear matching files
            qDeleteAll(cMatchingFiles);
            cMatchingFiles.clear();

            // Hide Query progress dialog
            if (mQueryProgress)
                mQueryProgress->Stop();

            return false;
        }
    }

    // Hide Query progress dialog
    if (mQueryProgress)
        mQueryProgress->Stop();

    // Debug message
    WriteDebugMessageFile("************************");

    return true;
}

///////////////////////////////////////////////////////////
// Query: add condition string for a specified filter/value
///////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::Query_AddValueCondition(const QString & strQueryFilter, const QString & strQueryValue, bool bExpressionRange/*=false*/, bool bUseLinkConditions/*=true*/)
{
    if(m_pmapFields_GexToRemote == NULL)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoMappingTable, NULL);
        return false;
    }

    // Make sure filter is not empty
    if(strQueryFilter.isEmpty() || (strQueryValue == "*"))
        return true;

    // Check if mapping available for specified field
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping = m_pmapFields_GexToRemote->find(strQueryFilter);
    if(itMapping == m_pmapFields_GexToRemote->end())
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, strQueryFilter.toLatin1().constData());
        return false;
    }

    // Get mapping fields
    // Syntax is: <table>.<field>[|<link_name>]
    // Example1: "ft_splitlot.job_nam"
    // Example2: "product.product_name|product-ft_lot"
    // Example3: "GEXDB.wt_splitlot.product_name"
    QString	strDbLink, strQueryCondition;
    bool	bNumeric = (*itMapping).m_bNumeric;
    QString strValue = strQueryValue;

    // If the field to filter is the LotID or WaferID, use the Unit_Serial field
    if((*itMapping).m_strMetaDataName == GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT)
    {
        strValue.replace("|",",").replace(",",GEXDB_PLUGIN_DELIMITER_OR);
        strQueryCondition = "unit_test.unit_serial|FieldExpression_String|" + strValue;
        strQueryCondition += "|NVL(unit_lot, substr(unit_serial,1,instr(unit_serial,'.')-1))";
    }
    else if((*itMapping).m_strMetaDataName == GEXDB_PLUGIN_MEDTRONIC_METADATA_SUBLOT)
    {
        strValue.replace("|",",").replace(",",GEXDB_PLUGIN_DELIMITER_OR);
        strQueryCondition = "unit_test.unit_serial|FieldExpression_String|" + strValue;
        strQueryCondition += "|substr(unit_serial,instr(unit_serial,'.')+1,2)";
    }
    else if((*itMapping).m_strMetaDataName == GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE)
    {
        strValue.replace("|",",").replace(",",GEXDB_PLUGIN_DELIMITER_OR);
        strQueryCondition = "unit_test.transfer_date|FieldExpression_String|" + strValue;
        strQueryCondition += "|to_char(transfer_date,'mmddyyyyhhmiss')";
    }
    else if(bNumeric)
    {
        // In the filter condition, replace '|' with ',', because we use '|' in our internal
        // query syntax
        strValue.replace("|", ",");
        strQueryCondition = (*itMapping).m_strSqlFullField + "|Numeric|" + strValue;
    }
    else if(bExpressionRange)
        strQueryCondition = (*itMapping).m_strSqlFullField + "|ExpressionRange|" + strValue;
    else
    {
        // In the filter condition, replace '|' with GEXDB_PLUGIN_DELIMITER_OR, because we use '|' in our internal
        // query syntax
        strValue.replace("|",",").replace(",",GEXDB_PLUGIN_DELIMITER_OR);
        strQueryCondition = (*itMapping).m_strSqlFullField + "|String|" + strValue;
    }

    // Add condition to the condition list
    if(m_strlQuery_ValueConditions.indexOf(strQueryCondition) == -1)
        m_strlQuery_ValueConditions.append(strQueryCondition);

    // Link conditions
    strDbLink = (*itMapping).m_strSqlLinkName;
    if(bUseLinkConditions && !strDbLink.isEmpty())
        return Query_AddLinkCondition(strDbLink);

    return true;
}

///////////////////////////////////////////////////////////
// Query: add select string for a specified field
///////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::Query_AddField(const QString& strQueryField,
                                           QString& strDbField,
                                           QString& strDbTable,
                                           bool bUseLinkConditions /*= true*/,
                                           bool /*bConsolidated = false*/)
{
    // Make sure field is not empty
    if(strQueryField.isEmpty())
        return true;

    if(m_pmapFields_GexToRemote == NULL)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoMappingTable, NULL);
        return false;
    }

    // Check if alias is specified
    QStringList	strlElements = strQueryField.split("=");
    QString		strField = strlElements[0];

    // Check if mapping available for specified field
    GexDbPlugin_Mapping_FieldMap::Iterator itMapping = m_pmapFields_GexToRemote->find(strField);
    if(itMapping == m_pmapFields_GexToRemote->end())
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, strField.toLatin1().constData());
        return false;
    }

    // Get mapping fields
    // Syntax is: <table>.<field>[|<link_name>]
    // Example1: "ft_splitlot.job_nam"
    // Example2: "product.product_name|product-ft_lot"
    // Example3: "GEXDB.wt_splitlot.product_name"
    QString		strDbLink, strQueryCondition, strExpressionPrefix;

    strDbField = (*itMapping).m_strSqlFullField;
    strDbTable = (*itMapping).m_strSqlTable;
    strDbLink = (*itMapping).m_strSqlLinkName;

    strExpressionPrefix = "Expression";
    if(strlElements.count() > 1)
    {
        strExpressionPrefix += "=";
        strExpressionPrefix += strlElements[1];
    }

    // If the field to query is the LotID or WaferID, extract it from the Unit_Serial field
    if((*itMapping).m_strMetaDataName == GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT)
    {
        strField = strExpressionPrefix + "|unit_test.unit_serial|NVL(unit_lot, substr(unit_serial,1,instr(unit_serial,'.')-1))";
        if(m_strlQuery_Fields.indexOf(strField) == -1)
            m_strlQuery_Fields.append(strField);
        return true;
    }
    if((*itMapping).m_strMetaDataName == GEXDB_PLUGIN_MEDTRONIC_METADATA_SUBLOT)
    {
        strField = strExpressionPrefix + "|unit_test.unit_serial|substr(unit_serial,instr(unit_serial,'.')+1,2)";
        if(m_strlQuery_Fields.indexOf(strField) == -1)
            m_strlQuery_Fields.append(strField);
        return true;
    }
    if((*itMapping).m_strMetaDataName == GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE)
    {
        strField = strExpressionPrefix + "|unit_test.transfer_date|to_char(transfer_date,'mmddyyyyhhmiss')";
        if(m_strlQuery_Fields.indexOf(strField) == -1)
            m_strlQuery_Fields.append(strField);
        return true;
    }

    // Add field
    strField = "Field";
    if(strlElements.count() > 1)
    {
        strField += "=";
        strField += strlElements[1];
    }
    strField += "|";
    strField += strDbField;
    if(m_strlQuery_Fields.indexOf(strField) == -1)
        m_strlQuery_Fields.append(strField);

    // Link conditions
    if(bUseLinkConditions && !strDbLink.isEmpty())
        return Query_AddLinkCondition(strDbLink);

    return true;
}

///////////////////////////////////////////////////////////
// Query: add filters to query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::Query_AddFilters(GexDbPlugin_Filter &cFilters)
{
    QString						strQueryFilter, strQueryValue;
    QStringList::ConstIterator	it;
    for(it = cFilters.strlQueryFilters.begin(); it != cFilters.strlQueryFilters.end(); it++)
    {
        strQueryFilter = (*it).section('=', 0, 0);
        strQueryValue = (*it).section('=', 1, 1);
        if(!Query_AddValueCondition(strQueryFilter, strQueryValue))
            return false;
    }

    // Add condition to exclude Test_Set_ID values ending with 'SUMM'
    // Get mapping for Test_Set_ID field
    QString									strQueryCondition;
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID);
    GexDbPlugin_Mapping_Field				clMappingEntry;
    if(itMapping == m_pmapFields_GexToRemote->end())
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID);
        return false;
    }
    clMappingEntry = itMapping.value();
    strQueryCondition = clMappingEntry.m_strSqlFullField;
    strQueryCondition += "|NotString|%SUMM";
    if(m_strlQuery_ValueConditions.indexOf(strQueryCondition) == -1)
        m_strlQuery_ValueConditions.append(strQueryCondition);

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all valid values for a field (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool
GexDbPlugin_Medtronic::QueryField(GexDbPlugin_Filter& cFilters,
                                  QStringList& cMatchingValues,
                                  bool bSoftBin /*= false*/,
                                  bool bDistinct /*= true*/,
                                  QuerySort /*eQuerySort = eQuerySort_Asc*/)
{
    GexDbPlugin_Filter						cTempFilters = cFilters;
    bool									bIsLinkedToUnitTest=false, bFilterOnUnitLot=false;
    QString									strQueryFilter, strQueryValue, strUnitType, strMessage;
    QStringList::ConstIterator				it;
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping;
    GexDbPlugin_Mapping_Field				clMappingEntry, clMappingEntry_QueryField;

    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Lookup;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Lookup;

    Query_Empty();

    // Check field to query and filters to avoid a full scan over the huge Unit_Test table without using the defined partitions:
    // o If the field to query has about a distinct value for each row in Unit_Test table (ie unit_serial), make sure a filter is set on Unit_Lot
    // o If the field to query or any fields having a filter is linked to Unit_Test table:
    //		1) make sure date restriction is set to avoid querying over the whole table
    //		2) don't use the lookup maps
    // o If neither the field to query nor any fields having a filter are linked to Unit_Test table, remove any eventual date restrictions
    //   to avoid querying on the Unit_Test table

    // Check field to query
    foreach(QString queryField, cFilters.mQueryFields)
    {
        itMapping = m_pmapFields_GexToRemote->find(queryField);
        if(itMapping == m_pmapFields_GexToRemote->end())
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, queryField.toLatin1().constData());
            return false;
        }
        clMappingEntry_QueryField = itMapping.value();
        if(m_pmapLinks_Remote->IsLinkedToTable(clMappingEntry_QueryField, "unit_test"))
            bIsLinkedToUnitTest = true;
    }

    // Go through filters already set
    for(it=cFilters.strlQueryFilters.begin(); it != cFilters.strlQueryFilters.end(); it++)
    {
        strQueryFilter = (*it).section('=', 0, 0);
        strQueryValue = (*it).section('=', 1, 1);
        if(!strQueryValue.isEmpty() && (strQueryValue != "*"))
        {
            itMapping = m_pmapFields_GexToRemote->find(strQueryFilter);
            if(itMapping == m_pmapFields_GexToRemote->end())
            {
                GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, strQueryFilter.toLatin1().constData());
                return false;
            }
            clMappingEntry = itMapping.value();
            if(m_pmapLinks_Remote->IsLinkedToTable(clMappingEntry, "unit_test"))
                bIsLinkedToUnitTest = true;
            if(clMappingEntry.m_strMetaDataName == GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT)
                bFilterOnUnitLot = true;
        }
    }

    // If the field to query is directly linked to the unit_test table, make sure date restrictions are set
    if((bIsLinkedToUnitTest) && (cFilters.iTimePeriod == GEX_QUERY_TIMEPERIOD_ALLDATES))
    {
        cMatchingValues.append("No date restrictions set:");
        cMatchingValues.append("to avoid a full scan on the unit_test table, please set some date restrictions.");
        return true;
    }

    // If the field has about one distinct value per row, make sure a filter on unit_lot is set
    if((clMappingEntry_QueryField.m_strMetaDataName == GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_SERIAL) && !bFilterOnUnitLot)
    {
        strMessage =	"No filter is set on Unit_Lot.\n";
        strMessage +=	"This might result in a scan of a big portion of the unit_test table,\n";
        strMessage +=	"unless other restrictive filters are set.\n\n";
        strMessage +=	"Do you want to continue?";
        QString logMessage("No filter is set on Unit_Lot: to avoid too many "
                           "results to be listed, please set a filter on Unit_Lot");
        if (!mGexScriptEngine->property("GS_DAEMON").toBool())
        {
            if (QMessageBox::warning(mParentWidget, "Field selection", strMessage, QMessageBox::Yes, QMessageBox::No) ==
                    QMessageBox::No)
            {
                cMatchingValues.append(logMessage);
                return true;
            }
        }
        else
        {
            GSLOG(SYSLOG_SEV_WARNING, logMessage.toLatin1().constData());
            cMatchingValues.append(logMessage);
            return true;
        }
    }

    // Select appropriate Meta-Data mapping
    if(bIsLinkedToUnitTest)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote;
        m_pmapLinks_Remote = &m_mapLinks_Remote;
    }
    else
    {
        // To avoid using unit_test table if no fields are directly linked to it, remove date constraints
        cTempFilters.iTimePeriod = GEX_QUERY_TIMEPERIOD_ALLDATES;
    }

    // Call base class QueryField function (Medtronic specificities implemented in overloaded functions called by the base class QueryField function)
    return GexDbPlugin_Base::QueryField(cTempFilters, cMatchingValues, bSoftBin, bDistinct, eQuerySort_None);
}

///////////////////////////////////////////////////////////////////////
// Query: construct query string to retrieve all tests matching filters
///////////////////////////////////////////////////////////////////////
bool
GexDbPlugin_Medtronic::ConstructTestlistQuery(GexDbPlugin_Filter& cFilters,
                                              QString& strQuery,
                                              QString& /*strTestNumField*/,
                                              QString& /*strTestNameField*/,
                                              QString /*strTestTypeField = ""*/)
{
    QString strDbField, strDbTable;

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <testnum, testname>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    Query_Empty();

    // Set fields to query
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_ID, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_NAME, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_LABEL, strDbField, strDbTable);
    Query_AddField(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PARAMETER, strDbField, strDbTable);

    // Set filters
    Query_AddFilters(cFilters);

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery);

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all tests (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::QueryTestlist(GexDbPlugin_Filter& cFilters,
                                          QStringList& cMatchingValues,
                                          bool /*bParametricOnly*/)
{
    GexDbPlugin_Filter cTempFilters = cFilters;

    m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Lookup;
    m_pmapLinks_Remote = &m_mapLinks_Remote_Lookup;

    Query_Empty();

    // Clear returned stringlist
    cMatchingValues.clear();

    // Keep only filter on Test_Set_ID.
    QStringList::ConstIterator	it;
    cTempFilters.strlQueryFilters.clear();
    for(it = cFilters.strlQueryFilters.begin(); it != cFilters.strlQueryFilters.end(); it++)
    {
        if((*it).section('=', 0, 0) == GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID)
            cTempFilters.strlQueryFilters.append((*it));
    }

    // Make sure a filter on Test_Set_ID is defined to avoid querying about a potentially huge table with a full scan
    if(cTempFilters.strlQueryFilters.isEmpty())
    {
        // No filter on Test_Set_ID, don't execute the query, give a message for the user
        cMatchingValues.append("No filter defined on Test Set ID:");
        cMatchingValues.append("to avoid a full scan on the test information table,");
        cMatchingValues.append("please define a filter on Test Set ID.");
        return true;
    }

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Construct SQL query
    QString strQuery, strTestNumField, strTestNameField;
    strTestNumField = GEXDB_PLUGIN_DBFIELD_TESTNUM;
    strTestNameField = GEXDB_PLUGIN_DBFIELD_TESTNAME;
    ConstructTestlistQuery(cTempFilters, strQuery, strTestNumField, strTestNameField);

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        // Display error
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Populate stringlist
    QString strTestName;
    int nTestID;
    while(clGexDbQuery.Next())
    {
        nTestID = clGexDbQuery.value(0).toInt();
        strTestName = clGexDbQuery.value(1).toString();
        strTestName += "," + clGexDbQuery.value(2).toString();
        strTestName += "," + clGexDbQuery.value(3).toString();
        cMatchingValues.append(QString::number(nTestID));
        cMatchingValues.append(strTestName);
        cMatchingValues.append("P");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return mapped filter labels
//////////////////////////////////////////////////////////////////////
void
GexDbPlugin_Medtronic::
GetLabelFilterChoices(const QString& /*strDataTypeQuery*/,
                      QStringList& strlLabelFilterChoices)
{
    strlLabelFilterChoices.clear();
    strlLabelFilterChoices = m_strlLabelFilterChoices;
}

void
GexDbPlugin_Medtronic::
GetConsolidatedLabelFilterChoices(const QString& /*strDataTypeQuery*/,
                      QStringList& strlLabelFilterChoices)
{
    strlLabelFilterChoices.clear();
    strlLabelFilterChoices = m_strlLabelFilterChoices;
}

void GexDbPlugin_Medtronic::Init_Extraction()
{
    m_mapRunInfo.clear();
    m_uiSplitlotNbRuns = 0;
#if PROFILERON
    m_bProfilerON = true;
#else
    m_bProfilerON = false;
#endif

    // A. Field maps and links used for the filters in the query page (mostly using Lookup tables)
    // A1. Gex fields <-> Medtronic DB mapping (lookup tables)
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_MODEL] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_MODEL, "",
                                                                                                           "w_unit_model__test_set_id",	"w_unit_model__test_set_id.unit_model",		"",
                                                                                                           true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID, "",
                                                                                                            "w_unit_model__test_set_id",	"w_unit_model__test_set_id.test_set_id",	"",
                                                                                                            true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_TYPE] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_TYPE, "",
                                                                                                          "w_unit_type__unit_model",		"w_unit_type__unit_model.unit_type",		"lt10-lt11",
                                                                                                          true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_MODEL] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_MODEL, "",
                                                                                                              "w_station_serial",				"w_station_serial.station_model",			"",
                                                                                                              true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SERIAL] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SERIAL, "",
                                                                                                               "w_station_serial",				"w_station_serial.station_serial",			"",
                                                                                                               true, "N", false, false, false, false, false, false, false);
    // A4. Gex fields <-> Medtronic DB mapping (directly linked to unit_test table)
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PURPOSE] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PURPOSE, "",
                                                                                                             "test_purpose",					"test_purpose.test_purpose",				"lt6-t1",
                                                                                                             true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_FACILITY] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_FACILITY, "",
                                                                                                         "facility",						"facility.facility",						"lt8-t1",
                                                                                                         true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_OPERATION] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_OPERATION, "",
                                                                                                               "test_operation",				"test_operation.test_operation",			"lt3-t1",
                                                                                                               true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_PIN] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_PIN, "",
                                                                                                            "unit_test",					"unit_test.station_pin",					"",
                                                                                                            false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SOFTWARE_REV] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SOFTWARE_REV, "",
                                                                                                                     "unit_test",					"unit_test.station_software_rev",			"",
                                                                                                                     false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT, "",
                                                                                                         "unit_test",					"unit_test.unit_serial",					"",
                                                                                                         true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_SUBLOT] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_SUBLOT, "",
                                                                                                       "unit_test",					"unit_test.unit_serial",					"",
                                                                                                       true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_OPERATOR_ID] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_OPERATOR_ID, "",
                                                                                                            "unit_test",					"unit_test.test_operator_id",				"",
                                                                                                            true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_SERIAL] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_SERIAL, "",
                                                                                                            "unit_test",					"unit_test.unit_serial",					"",
                                                                                                            true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_PIN] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_PIN, "",
                                                                                                         "unit_test",					"unit_test.unit_pin",						"",
                                                                                                         true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PACKAGE_ID] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PACKAGE_ID, "",
                                                                                                         "unit_test",					"unit_test.test_package_id",				"",
                                                                                                         false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PF_STATUS] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PF_STATUS, "",
                                                                                                               "unit_test",					"unit_test.test_status_code",				"",
                                                                                                               true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_FIRST_TEST_FLAG] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_FIRST_TEST_FLAG, "",
                                                                                                                "unit_test",					"unit_test.first_test_flag",				"",
                                                                                                                true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_LAST_TEST_FLAG] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_LAST_TEST_FLAG, "",
                                                                                                               "unit_test",					"unit_test.last_test_flag",				"",
                                                                                                               true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE, "",
                                                                                                              "unit_test",					"unit_test.transfer_date",				"",
                                                                                                              true, "N", false, false, false, false, false, false, false);
    // A3. Fields not exported in the GEX GUI, but needed in our queries
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_DBFIELD_STARTDATETIME] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, GEXDB_PLUGIN_DBFIELD_STARTDATETIME,
                                                                                                   "unit_test",					"unit_test.test_date",						"",
                                                                                                   false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_ID] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_ID, "",
                                                                                                        "test_id",						"test_id.test_id",							"t2-lt9-1",
                                                                                                        false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_NAME] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_NAME, "",
                                                                                                          "w_test_set_id__test_id",		"w_test_set_id__test_id.test_name",			"lt9-lt11",
                                                                                                          false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_LABEL] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_LABEL, "",
                                                                                                           "w_test_set_id__test_id",		"w_test_set_id__test_id.test_label",		"lt9-lt11",
                                                                                                           false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Lookup[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PARAMETER] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PARAMETER, "",
                                                                                                               "w_test_set_id__test_id",		"w_test_set_id__test_id.test_parameter",	"lt9-lt11",
                                                                                                               false, "N", false, false, false, false, false, false, false);
    // A4. Links in Medtronic DB
    m_mapLinks_Remote_Lookup["t2-lt9-1"] =
            GexDbPlugin_Mapping_Link("t2-lt9-1", "test_id", "test_id.test_name", "w_test_set_id__test_id", "w_test_set_id__test_id.test_name", "t2-lt9-2");
    m_mapLinks_Remote_Lookup["t2-lt9-2"] =
            GexDbPlugin_Mapping_Link("t2-lt9-2", "test_id", "test_id.test_label", "w_test_set_id__test_id", "w_test_set_id__test_id.test_label", "t2-lt9-3");
    m_mapLinks_Remote_Lookup["t2-lt9-3"] =
            GexDbPlugin_Mapping_Link("t2-lt9-3", "test_id", "test_id.test_parameter", "w_test_set_id__test_id", "w_test_set_id__test_id.test_parameter", "lt9-lt11");
    m_mapLinks_Remote_Lookup["lt10-lt11"] =
            GexDbPlugin_Mapping_Link("lt10-lt11", "w_unit_type__unit_model", "w_unit_type__unit_model.unit_model", "w_unit_model__test_set_id", "w_unit_model__test_set_id.unit_model", "");
    m_mapLinks_Remote_Lookup["lt9-lt11"] =
            GexDbPlugin_Mapping_Link("lt9-lt11", "w_test_set_id__test_id", "w_test_set_id__test_id.test_set_id", "w_unit_model__test_set_id", "w_unit_model__test_set_id.test_set_id", "");
    m_mapLinks_Remote_Lookup["lt6-t1"] =
            GexDbPlugin_Mapping_Link("lt6-t1", "test_purpose", "test_purpose.test_purpose_code", "unit_test", "unit_test.test_purpose_code", "");
    m_mapLinks_Remote_Lookup["lt8-t1"] =
            GexDbPlugin_Mapping_Link("lt8-t1", "facility", "facility.facility_code", "unit_test", "unit_test.test_facility_code", "");
    m_mapLinks_Remote_Lookup["lt3-t1"] =
            GexDbPlugin_Mapping_Link("lt3-t1", "test_operation", "test_operation.test_operation_code", "unit_test", "unit_test.test_operation_code", "");
    // A5 List of available filters
    GexDbPlugin_Mapping_FieldMap::Iterator itMapping;
    for(itMapping = m_mapFields_GexToRemote_Lookup.begin(); itMapping != m_mapFields_GexToRemote_Lookup.end(); itMapping++)
    {
        if((*itMapping).m_bDisplayInQueryGui)
            m_strlLabelFilterChoices.append((*itMapping).m_strMetaDataName);
    }

    // B. Field maps and links for dataset extraction
    // B1. Gex fields <-> Medtronic DB mapping
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_MODEL] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_MODEL, "",
                                                                                                    "unit_test",					"unit_test.unit_model",						"",
                                                                                                    false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID, "",
                                                                                                     "unit_test",					"unit_test.test_set_id",					"",
                                                                                                     false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_TYPE] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_TYPE, "",
                                                                                                   "unit_test",					"unit_test.unit_type",						"",
                                                                                                   false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_MODEL] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_MODEL, "",
                                                                                                       "unit_test",					"unit_test.station_model",					"",
                                                                                                       false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SERIAL] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SERIAL, "",
                                                                                                        "unit_test",					"unit_test.station_serial",					"",
                                                                                                        false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PURPOSE] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PURPOSE, "",
                                                                                                      "test_purpose",					"test_purpose.test_purpose",				"lt6-t1",
                                                                                                      false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_FACILITY] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_FACILITY, "",
                                                                                                  "facility",						"facility.facility",						"lt8-t1",
                                                                                                  false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_OPERATION] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_OPERATION, "",
                                                                                                        "test_operation",				"test_operation.test_operation",			"lt3-t1",
                                                                                                        false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_PIN] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_PIN, "",
                                                                                                     "unit_test",					"unit_test.station_pin",					"",
                                                                                                     false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SOFTWARE_REV] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SOFTWARE_REV, "",
                                                                                                              "unit_test",					"unit_test.station_software_rev",			"",
                                                                                                              false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT, "",
                                                                                                  "unit_test",					"unit_test.unit_serial",					"",
                                                                                                  false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_SUBLOT] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_SUBLOT, "",
                                                                                                "unit_test",					"unit_test.unit_serial",					"",
                                                                                                false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_OPERATOR_ID] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_OPERATOR_ID, "",
                                                                                                     "unit_test",					"unit_test.test_operator_id",				"",
                                                                                                     false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_SERIAL] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_SERIAL, "",
                                                                                                     "unit_test",					"unit_test.unit_serial",					"",
                                                                                                     false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_PIN] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_PIN, "",
                                                                                                  "unit_test",					"unit_test.unit_pin",						"",
                                                                                                  false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PACKAGE_ID] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PACKAGE_ID, "",
                                                                                                         "unit_test",					"unit_test.test_package_id",				"",
                                                                                                         false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PF_STATUS] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PF_STATUS, "",
                                                                                                        "unit_test",					"unit_test.test_status_code",				"",
                                                                                                        false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_FIRST_TEST_FLAG] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_FIRST_TEST_FLAG, "",
                                                                                                         "unit_test",					"unit_test.first_test_flag",				"",
                                                                                                         false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_LAST_TEST_FLAG] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_LAST_TEST_FLAG, "",
                                                                                                        "unit_test",					"unit_test.last_test_flag",				"",
                                                                                                        false, "N", false, false, false, false, false, false, false);
    // B2. Fields not exported in the GEX GUI, but needed in our queries
    m_mapFields_GexToRemote[GEXDB_PLUGIN_DBFIELD_STARTDATETIME] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, GEXDB_PLUGIN_DBFIELD_STARTDATETIME,
                                                                                            "unit_test",					"unit_test.test_date",						"",
                                                                                            false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote[GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE] = GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE, "",
                                                                                                       "unit_test",					"unit_test.transfer_date",				"",
                                                                                                       false, "N", false, false, false, false, false, false, false);
    // B3. Links in Medtronic DB
    m_mapLinks_Remote["lt6-t1"] =
            GexDbPlugin_Mapping_Link("lt6-t1", "test_purpose", "test_purpose.test_purpose_code", "unit_test", "unit_test.test_purpose_code", "");
    m_mapLinks_Remote["lt3-t1"] =
            GexDbPlugin_Mapping_Link("lt3-t1", "test_operation", "test_operation.test_operation_code", "unit_test", "unit_test.test_operation_code", "");
    m_mapLinks_Remote["lt8-t1"] =
            GexDbPlugin_Mapping_Link("lt8-t1", "facility", "facility.facility_code", "unit_test", "unit_test.test_facility_code", "");
}

//////////////////////////////////////////////////////////////////////
// Add splitlot conditions to query
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic::AddSplitlotConditions(GexDbPlugin_Filter &cFilters, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo, bool bUseSplitlotDateWindow/* = false*/)
{
    QStringList::iterator	it;
    QString					strQueryFilter, strQueryValue;
    QString					strUnitType = clSplitlotInfo.m_strUnitType.toLower();

    if(!clSplitlotInfo.m_strUnitModel.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_MODEL, clSplitlotInfo.m_strUnitModel);
    if(!clSplitlotInfo.m_strUnitType.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_TYPE, clSplitlotInfo.m_strUnitType);
    if(!clSplitlotInfo.m_strUnitLot.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_LOT, clSplitlotInfo.m_strUnitLot);
    if(!clSplitlotInfo.m_strSublot.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_SUBLOT, clSplitlotInfo.m_strSublot);
    if(!clSplitlotInfo.m_strUnitPin.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_PIN, clSplitlotInfo.m_strUnitPin);
    if(!clSplitlotInfo.m_strTestPurpose.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PURPOSE, clSplitlotInfo.m_strTestPurpose);
    if(!clSplitlotInfo.m_strTestOperation.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_OPERATION, clSplitlotInfo.m_strTestOperation);
    if(!clSplitlotInfo.m_strTestFacility.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_FACILITY, clSplitlotInfo.m_strTestFacility);
    if(!clSplitlotInfo.m_strTestSetID.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID, clSplitlotInfo.m_strTestSetID);
    if(!clSplitlotInfo.m_strTestPackageID.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PACKAGE_ID, clSplitlotInfo.m_strTestPackageID);
    if(!clSplitlotInfo.m_strTestOperatorID.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_OPERATOR_ID, clSplitlotInfo.m_strTestOperatorID);
    if(!clSplitlotInfo.m_strStationPin.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_PIN, clSplitlotInfo.m_strStationPin);
    if(!clSplitlotInfo.m_strStationModel.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_MODEL, clSplitlotInfo.m_strStationModel);
    if(!clSplitlotInfo.m_strStationSoftwareRev.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SOFTWARE_REV, clSplitlotInfo.m_strStationSoftwareRev);
#if !GEXDB_PLUGIN_MEDTRONIC_MERGE_FIRSTTEST_LASTTEST
    if(!clSplitlotInfo.m_strFirstTestFlag.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_FIRST_TEST_FLAG, clSplitlotInfo.m_strFirstTestFlag);
    if(!clSplitlotInfo.m_strLastTestFlag.isEmpty())
        Query_AddValueCondition(GEXDB_PLUGIN_MEDTRONIC_METADATA_LAST_TEST_FLAG, clSplitlotInfo.m_strLastTestFlag);
#endif

    if(bUseSplitlotDateWindow)
        Query_AddTimePeriodCondition(clSplitlotInfo.m_clMinTestDate, clSplitlotInfo.m_clMaxTestDate);
    else
        Query_AddTimePeriodCondition(cFilters);

    // Add to the conditions those filters that are set but not part of the Splitlot definition:
    // test_operator_id
    // station_model
    // test_status_code
    // transfer_date
    // unit_serial
    // first_test_flag
    // last_test_flag
    for(it = cFilters.strlQueryFilters.begin(); it != cFilters.strlQueryFilters.end(); it++)
    {
        strQueryFilter = (*it).section('=', 0, 0);
        strQueryValue = (*it).section('=', 1, 1);
        if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_OPERATOR_ID)
            Query_AddValueCondition(strQueryFilter, strQueryValue);
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_MODEL)
            Query_AddValueCondition(strQueryFilter, strQueryValue);
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_STATION_SERIAL)
            Query_AddValueCondition(strQueryFilter, strQueryValue);
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_PF_STATUS)
            Query_AddValueCondition(strQueryFilter, strQueryValue);
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_TRANSFER_DATE)
            Query_AddValueCondition(strQueryFilter, strQueryValue);
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_UNIT_SERIAL)
            Query_AddValueCondition(strQueryFilter, strQueryValue);
#if GEXDB_PLUGIN_MEDTRONIC_MERGE_FIRSTTEST_LASTTEST
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_FIRST_TEST_FLAG)
            Query_AddValueCondition(strQueryFilter, strQueryValue);
        else if(strQueryFilter == GEXDB_PLUGIN_MEDTRONIC_METADATA_LAST_TEST_FLAG)
            Query_AddValueCondition(strQueryFilter, strQueryValue);
#endif
    }

    // Add condition to exclude Test_Set_ID values ending with 'SUMM'
    // Get mapping for Test_Set_ID field
    QString									strQueryCondition;
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping = m_pmapFields_GexToRemote->find(GEXDB_PLUGIN_MEDTRONIC_METADATA_TEST_SET_ID);
    GexDbPlugin_Mapping_Field				clMappingEntry;
    if(itMapping == m_pmapFields_GexToRemote->end())
        return;
    clMappingEntry = itMapping.value();
    strQueryCondition = clMappingEntry.m_strSqlFullField;
    strQueryCondition += "|NotString|%SUMM";
    if(m_strlQuery_ValueConditions.indexOf(strQueryCondition) == -1)
        m_strlQuery_ValueConditions.append(strQueryCondition);
}

//////////////////////////////////////////////////////////////////////
// Init different maps: array with part results, binning map,... for specified Splitlot_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::InitMaps(GexDbPlugin_Filter &cFilters, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo)
{
    QString				strQuery, strCondition, strFieldSpec;
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int					nRunID, nIndex;

    // Reset variables
    m_mapRunInfo.clear();
    m_uiSplitlotNbRuns = 0;

    // Query part information
    Query_Empty();
    m_strlQuery_Fields.append("Field|unit_test.ut_id");
    m_strlQuery_Fields.append("Field|unit_test.test_date");
    m_strlQuery_Fields.append("Field|unit_test.unit_serial");
    m_strlQuery_Fields.append("Field|unit_test.test_status_code");
    m_strlQuery_Fields.append("Field|unit_test.test_bin_code");
    m_strlQuery_Fields.append("Field|unit_test.test_elapsed_time");
    m_strlQuery_OrderFields.append("unit_test.ut_id|ASC");

    // Add splitlot conditions
    AddSplitlotConditions(cFilters, clSplitlotInfo);

    // Build SQL query
    Query_BuildSqlString(strQuery, true);

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                    clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    bool				bFirstRow = true;
    QDateTime			clDateTime;
    QString				strTemp;
    GexDbPlugin_RunInfo	clRunInfo;
    int					nHour=0, nMin=0, nSec=0;
    while(clGexDbQuery.Next())
    {
        // Check for user abort
        if(mQueryProgress->IsAbortRequested())
        {
            GSET_ERROR0(GexDbPlugin_Base, eDB_Abort, NULL);
            return false;
        }

        nIndex = 0;
        nRunID = clGexDbQuery.value(nIndex++).toUInt();
        clRunInfo.Reset();
        clRunInfo.m_nSiteNb = 1;

        // Check test date/time
        clDateTime = clGexDbQuery.value(nIndex++).toDateTime();
        if(bFirstRow)
        {
            clSplitlotInfo.m_clMinTestDate = clSplitlotInfo.m_clMaxTestDate = clDateTime;
            bFirstRow = false;
        }
        else
        {
            if(clDateTime < clSplitlotInfo.m_clMinTestDate)
                clSplitlotInfo.m_clMinTestDate = clDateTime;
            if(clDateTime > clSplitlotInfo.m_clMaxTestDate)
                clSplitlotInfo.m_clMaxTestDate = clDateTime;
        }

        // Update RUN array
        m_uiSplitlotNbRuns++;
        m_uiTotalRuns++;
        clRunInfo.m_nRunID = nRunID;
        clRunInfo.m_strPartID = clGexDbQuery.value(nIndex++).toString();
        strTemp = clSplitlotInfo.m_strUnitType.toLower();
        if((strTemp == "wafer") || (strTemp == "keithley"))
        {
            // Extract X,Y coordinates from Unit_Serial
            strTemp = clRunInfo.m_strPartID;
            QRegExp regExp("[^\\.]+\\.[^+\\-]+([+\\-]\\d+)([\\.+\\-]\\d+)");
            if(regExp.indexIn(strTemp) > -1)
            {
                clRunInfo.m_nX = regExp.cap(1).toInt();
                strTemp = regExp.cap(2);
                if(strTemp.startsWith("."))
                    strTemp = strTemp.mid(1);
                clRunInfo.m_nY = strTemp.toInt();
            }
        }
        else
        {
#if 0
            // Extract PartID from Unit_Serial
            clRunInfo.m_strPartID = clRunInfo.m_strPartID.section(".", 3, 3);
#else
            // Keep Unit Serial as the PartID
#endif
        }
        // Add test_date to Part_ID
        //clRunInfo.m_strPartID += "_" + clDateTime.toString(Qt::ISODate);
        // Save test_date
        clRunInfo.m_tDateTimeOfTest = clDateTime.toTime_t();

        QString strPassFail = clGexDbQuery.value(nIndex++).toString();
        if(strPassFail.toLower() == "f")
            clRunInfo.m_bPartFailed = true;
        if(!clGexDbQuery.isNull(nIndex))
            clRunInfo.m_nSoftBin = clRunInfo.m_nHardBin = clGexDbQuery.value(nIndex).toInt();
        nIndex++;
        if(!clGexDbQuery.isNull(nIndex))
        {
            strTemp = clGexDbQuery.value(nIndex).toString();
            nHour = strTemp.section(':',0,0).toUInt();
            nMin = strTemp.section(':',1,1).toUInt();
            nSec = strTemp.section(':',2,2).toUInt();
            clRunInfo.m_lnTestTime = nHour*3600000+nMin*60000+nSec*1000;
        }
        nIndex++;

        // Add RunInfo to the map
        m_mapRunInfo[nRunID] = clRunInfo;

        // Update part counts
        (clSplitlotInfo.m_uiNbParts)++;
        if(!clRunInfo.m_bPartFailed)
            (clSplitlotInfo.m_uiNbParts_Good)++;

        // Update query progress dialog
        if (mQueryProgress)
            mQueryProgress->SetRetrievedRuns(m_uiTotalRuns);

    }

    // Update query progress dialog
    if (mQueryProgress)
        mQueryProgress->SetRetrievedRuns(m_uiTotalRuns, true);

    return true;
}

//////////////////////////////////////////////////////////////////////
// Create TestList for specified Splitlot_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::CreateTestlist(GexDbPlugin_Filter &cFilters, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo, const QString & strTestlist)
{
    QString				strQuery, strCondition, strFieldSpec;
    GexDbPlugin_Query	clGexDbQueryTests(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // First clear the list
    m_clTestList.ClearData(true);

#if GEXDB_PLUGIN_MEDTRONIC_CREATE_TESTLIST_ONTHEFLY
    // Don't create testlist (jointure too time consuming), create it on the fly when retrieving test results
    return true;
#endif

    if(!strTestlist.isEmpty())
    {
        // Query test info for specified Splitlot
        Query_Empty();
        m_strlQuery_Fields.append("Field|test_id.test_id");
        m_strlQuery_Fields.append("Field|test_id.test_name");
        m_strlQuery_Fields.append("Field|test_id.test_label");
        m_strlQuery_Fields.append("Field|test_id.test_parameter");
        m_strlQuery_Fields.append("Field|test_id.unit_of_measure");
        m_strlQuery_Fields.append("Function|test_data.lower_limit|MIN");
        m_strlQuery_Fields.append("Function|test_data.upper_limit|MAX");

        // Add link conditions
        strCondition = "test_id.test_id|test_data.test_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);
        strCondition = "test_data.ut_id|unit_test.ut_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);
        strCondition = "test_data.test_date|unit_test.test_date";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);

        // Add splitlot conditions
        AddSplitlotConditions(cFilters, clSplitlotInfo, true);

        // Narrow on tests specified in testlist (testlist format is "10,20-30,40-50,65")
        if(strTestlist != "*")
            Query_AddTestlistCondition(strTestlist);

        // Add group conditions
        m_strlQuery_GroupFields.append(QString("test_id.test_id"));
        m_strlQuery_GroupFields.append(QString("test_id.test_name"));
        m_strlQuery_GroupFields.append(QString("test_id.test_label"));
        m_strlQuery_GroupFields.append(QString("test_id.test_parameter"));
        m_strlQuery_GroupFields.append(QString("test_id.unit_of_measure"));

        Query_BuildSqlString(strQuery, true);

        clGexDbQueryTests.setForwardOnly(true);
        if(!clGexDbQueryTests.Execute(strQuery))
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQueryTests.lastError().text().toLatin1().constData());
            return false;
        }

        // Add all retrieved tests to testlist
        GexDbPlugin_TestInfo	*pTestInfo;
        unsigned int			uiTestID;
        QString					strUnitOfMeasure;
        while(clGexDbQueryTests.Next())
        {
            pTestInfo = new GexDbPlugin_TestInfo;
            uiTestID						= clGexDbQueryTests.value(0).toInt();
            pTestInfo->m_uiTestNumber	= uiTestID;
            pTestInfo->m_strTestName	= clGexDbQueryTests.value(1).toString();
            pTestInfo->m_strTestName	+= "," + clGexDbQueryTests.value(2).toString();
            pTestInfo->m_strTestName	+= "," + clGexDbQueryTests.value(3).toString();
            pTestInfo->m_pTestInfo_PTR	= new GexDbPlugin_TestInfo_PTR;

            if(!clGexDbQueryTests.isNull(4) && clGexDbQueryTests.value(4).isValid())
            {
                strUnitOfMeasure = clGexDbQueryTests.value(4).toString();
                if(!strUnitOfMeasure.isEmpty() && (strUnitOfMeasure.toLower() != "na"))
                    pTestInfo->m_pTestInfo_PTR->m_strTestUnit = strUnitOfMeasure;
            }
            if(!clGexDbQueryTests.isNull(5) && clGexDbQueryTests.value(5).isValid())
            {
                pTestInfo->m_pTestInfo_PTR->m_bHasLL	= true;
                pTestInfo->m_pTestInfo_PTR->m_fLL		= clGexDbQueryTests.value(5).toDouble();
            }
            if(!clGexDbQueryTests.isNull(6) && clGexDbQueryTests.value(6).isValid())
            {
                pTestInfo->m_pTestInfo_PTR->m_bHasHL	= true;
                pTestInfo->m_pTestInfo_PTR->m_fHL		= clGexDbQueryTests.value(6).toDouble();
            }
            m_clTestList.Insert(pTestInfo);
        }

        // Add artificial test_date parameter
        pTestInfo = new GexDbPlugin_TestInfo;
        pTestInfo->m_uiTestNumber	= GEX_TESTNBR_OFFSET_EXT_TEST_TIME_T;
        pTestInfo->m_strTestName	= "Test_Time_t parameter";
        pTestInfo->m_pTestInfo_PTR	= new GexDbPlugin_TestInfo_PTR;
        pTestInfo->m_pTestInfo_PTR->m_strTestUnit = "sec";
        m_clTestList.Insert(pTestInfo);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Write static test information
//////////////////////////////////////////////////////////////////////
bool
GexDbPlugin_Medtronic::
WriteStaticTestInfo(GQTL_STDF::StdfParse& clStdfParse,
                    GexDbPlugin_Medtronic_SplitlotInfo& /*clSplitlotInfo*/)
{
    GQTL_STDF::Stdf_PTR_V4	clPTR;
    BYTE			bParmFlag, bOptFlag;

#if GEXDB_PLUGIN_MEDTRONIC_CREATE_TESTLIST_ONTHEFLY
    // Testlist not created before going through test results (jointure too time consuming), but on the fly when retrieving test results
    // Static test info must be written on the fly
    return true;
#endif

    // Write a dummy PIR
    WritePIR(clStdfParse, 255);

    GexDbPlugin_TestInfoContainer	*pContainer = m_clTestList.m_pFirstTest;
    GexDbPlugin_TestInfo			*pTestInfo;
    while(pContainer)
    {
        pTestInfo = pContainer->m_pTestInfo;

        // Set PTR fields
        bParmFlag = 0xc0; // Limits are not strict
        bOptFlag = 0;
        if(pTestInfo->m_pTestInfo_PTR->m_bHasHL == false)
            bOptFlag |= 0x40;
        if(pTestInfo->m_pTestInfo_PTR->m_bHasHL == false)
            bOptFlag |= 0x80;
        clPTR.SetTEST_NUM(pTestInfo->m_uiTestNumber);
        clPTR.SetHEAD_NUM(1);
        clPTR.SetSITE_NUM(255);
        clPTR.SetTEST_FLG(0x10);		// Test not executed
        clPTR.SetPARM_FLG(bParmFlag);
        clPTR.SetRESULT(0.0F);
        clPTR.SetTEST_TXT(pTestInfo->m_strTestName);
        clPTR.SetALARM_ID("");
        clPTR.SetOPT_FLAG(bOptFlag);
        clPTR.SetRES_SCAL(0);
        clPTR.SetLLM_SCAL(0);
        clPTR.SetHLM_SCAL(0);
        clPTR.SetLO_LIMIT(pTestInfo->m_pTestInfo_PTR->m_fLL);
        clPTR.SetHI_LIMIT(pTestInfo->m_pTestInfo_PTR->m_fHL);
        clPTR.SetUNITS(pTestInfo->m_pTestInfo_PTR->m_strTestUnit);

        // Write record
        clStdfParse.WriteRecord(&clPTR);

        pTestInfo->m_bStaticInfoWrittenToStdf = true;
        pContainer = pContainer->m_pNextTest;
    }

    // Write a dummy PRR
    GexDbPlugin_RunInfo clPart;
    clPart.m_nSiteNb = 255;
    clPart.m_nSoftBin = clPart.m_nHardBin = 65535;
    clPart.m_nX = clPart.m_nY = -32768;
    WritePRR(clStdfParse, clPart);

    return true;
}

//////////////////////////////////////////////////////////////////////
// Write all test results for specified Splitlot_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::WriteTestResults(GexDbPlugin_Filter &cFilters, GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo, const QString & strTestlist)
{
    QString								strQuery, strCondition, strFieldSpec, strResultCode;
    GexDbPlugin_Query					clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int									nCurrentItem=-1, nRunID;
    unsigned int						uiTestID, uiTestSeq;
    unsigned int						uiNbRuns=0, uiNbTestResults=0;
    bool								bStatus;
    GexDbPlugin_TestInfo				*pTestInfo=NULL;
    QMap<long, GexDbPlugin_RunInfo>::Iterator itCurrentPart;

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        WriteDebugMessageFile("---- WriteTestResults()");
        m_clExtractionPerf.Start();
    }

    if(!strTestlist.isEmpty())
    {
        // Empty query lists
        Query_Empty();
        // Query all test results
        m_strlQuery_Fields.append("Field|test_data.test_id");
        m_strlQuery_Fields.append("Field|test_data.ut_id");
        m_strlQuery_Fields.append("Field|test_data.ut_seq");
        m_strlQuery_Fields.append("Field|test_data.result_code");
        m_strlQuery_Fields.append("Field|test_data.measured_value");
        m_strlQuery_Fields.append("Field|test_data.lower_limit");
        m_strlQuery_Fields.append("Field|test_data.upper_limit");

        // Add link conditions
        strCondition = "test_data.ut_id|unit_test.ut_id";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);
        strCondition = "test_data.test_date|unit_test.test_date";
        if(m_strlQuery_LinkConditions.indexOf(strCondition) == -1)
            m_strlQuery_LinkConditions.append(strCondition);

        // Add splitlot conditions
        AddSplitlotConditions(cFilters, clSplitlotInfo, true);

        // Narrow on tests specified in testlist (testlist format is "10,20-30,40-50,65")
        if(strTestlist != "*")
            Query_AddTestlistCondition(strTestlist);

        // add sort instructions
        m_strlQuery_OrderFields.append("test_data.ut_id");
        m_strlQuery_OrderFields.append("test_data.ut_seq");

        // Build query
        Query_BuildSqlString(strQuery, true);

        clGexDbQuery.setForwardOnly(true);
        bStatus = clGexDbQuery.Execute(strQuery);
        if(!bStatus)
        {
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(),
                        clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // Iterate through test results
        while(clGexDbQuery.Next())
        {
            // Check for user abort
            if(mQueryProgress->IsAbortRequested())
            {
                GSET_ERROR0(GexDbPlugin_Base, eDB_Abort, NULL);
                return false;
            }

            // Retrieve columns from row
            uiTestID = clGexDbQuery.value(0).toUInt();
            nRunID = clGexDbQuery.value(1).toInt();
            uiTestSeq = clGexDbQuery.value(2).toUInt();
            strResultCode = clGexDbQuery.value(3).toString();

            if(m_bCustomerDebugMode)
                uiNbTestResults++;

            // Any PIR/PRR to write?
            if(nCurrentItem == -1)
            {
                uiNbRuns++;
                nCurrentItem = nRunID;
                itCurrentPart = m_mapRunInfo.find(nCurrentItem);
                WritePIR(clStdfParse, itCurrentPart.value().m_nSiteNb);
                itCurrentPart.value().m_bWrittenToStdf = true;
                m_clTestList.ResetDynamicTestInfo();

                // Update query progress dialog
                if (mQueryProgress)
                    mQueryProgress->SetRetrievedResults(uiNbRuns, m_uiTotalTestResults);
            }

            else if(nCurrentItem != nRunID)
            {
                WriteTestInfo(clStdfParse, itCurrentPart.value());
                WritePRR(clStdfParse, itCurrentPart.value());
                uiNbRuns++;
                nCurrentItem = nRunID;
                itCurrentPart = m_mapRunInfo.find(nCurrentItem);
                WritePIR(clStdfParse, itCurrentPart.value().m_nSiteNb);
                itCurrentPart.value().m_bWrittenToStdf = true;
                m_clTestList.ResetDynamicTestInfo();

                // Update query progress dialog
                if (mQueryProgress)
                    mQueryProgress->SetRetrievedResults(uiNbRuns, m_uiTotalTestResults);
            }

            m_uiTotalTestResults++;

            // Get test static info
            GetStaticTestInfo(clGexDbQuery, cFilters, clSplitlotInfo, &pTestInfo, uiTestID, uiTestSeq);

            // Save test result ?
            if(	(m_clOptions.m_eTestMergeOption != GexDbPlugin_Medtronic_Options::eKeepFirstResult) ||
                    (pTestInfo->m_bTestExecuted == false))
            {
                pTestInfo->m_bTestExecuted = true;
                pTestInfo->m_pTestInfo_PTR->m_uiStaticFlags = 0xc0; // Limits are not strict
                if(clGexDbQuery.isNull(4) || !clGexDbQuery.value(4).isValid())
                {
                    pTestInfo->m_pTestInfo_PTR->m_fResultList.append(0.0F);
                    pTestInfo->m_pTestInfo_PTR->m_uiDynamicFlagsList.append((((strResultCode == "P") ? 0x02 : 0x82)));	// RESULT field not valid
                }
                else
                {
                    pTestInfo->m_pTestInfo_PTR->m_fResultList.append(clGexDbQuery.value(4).toDouble());
                    pTestInfo->m_pTestInfo_PTR->m_uiDynamicFlagsList.append((strResultCode == "P") ? 0 : 0x80);
                }
            }

            // Eventually update some part info
            if(	((*itCurrentPart).m_nHardBin == -1) &&
                    (pTestInfo->m_strTestName.toLower().trimmed() == "header,final,bin") &&
                    (!pTestInfo->m_pTestInfo_PTR->m_uiDynamicFlagsList.isEmpty()) &&
                    (!pTestInfo->m_pTestInfo_PTR->m_fResultList.isEmpty()) &&
                    ((pTestInfo->m_pTestInfo_PTR->m_uiDynamicFlagsList.first() & 0x02) == 0))
                (*itCurrentPart).m_nSoftBin = (*itCurrentPart).m_nHardBin = (int)(pTestInfo->m_pTestInfo_PTR->m_fResultList.first());
        }

        // Write last PRR
        if(nCurrentItem != -1)
        {
            WriteTestInfo(clStdfParse, itCurrentPart.value());
            WritePRR(clStdfParse, itCurrentPart.value());
        }
    }

    // Write parts with no test executed
    if(uiNbRuns < m_uiSplitlotNbRuns)
    {
        // Check for user abort
        if(mQueryProgress->IsAbortRequested())
        {
            GSET_ERROR0(GexDbPlugin_Base, eDB_Abort, NULL);
            return false;
        }

        for(itCurrentPart=m_mapRunInfo.begin(); itCurrentPart!=m_mapRunInfo.end(); itCurrentPart++)
        {
            if((itCurrentPart.value().m_nRunID != -1) && (itCurrentPart.value().m_bWrittenToStdf == false))
            {
                uiNbRuns++;
                WritePIR(clStdfParse, itCurrentPart.value().m_nSiteNb);
                WritePRR(clStdfParse, itCurrentPart.value());

                // Update query progress dialog
                if (mQueryProgress)
                    mQueryProgress->SetRetrievedResults(uiNbRuns, m_uiTotalTestResults);
            }
        }
    }

    // Update query progress dialog (force data update)
    if (mQueryProgress)
        mQueryProgress->SetRetrievedResults(uiNbRuns, m_uiTotalTestResults, true);

    // Debug mode??
    if(m_bCustomerDebugMode)
    {
        // Stop partial timer
        m_clExtractionPerf.Stop(clGexDbQuery.m_ulRetrievedRows_Cumul,
                                clGexDbQuery.m_fTimer_DbQuery_Cumul,
                                clGexDbQuery.m_fTimer_DbIteration_Cumul,
                                uiNbRuns, uiNbTestResults);

        // Write partial performance
        WritePartialPerformance("WriteTestResults()");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Write STDF MIR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::WriteMIR(GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo)
{
    GQTL_STDF::Stdf_MIR_V4	clMIR;
    QString			strString;

    // Set MIR fields
    clMIR.SetSETUP_T(clSplitlotInfo.m_clMinTestDate.toTime_t());
    clMIR.SetSTART_T(clSplitlotInfo.m_clMinTestDate.toTime_t());
    clMIR.SetSTAT_NUM(0);
    clMIR.SetMODE_COD('P');
    clMIR.SetRTST_COD('N');
    clMIR.SetPROT_COD(' ');
    clMIR.SetBURN_TIM(65535);
    clMIR.SetCMOD_COD(' ');
    clMIR.SetLOT_ID(clSplitlotInfo.m_strUnitLot);
    clMIR.SetPART_TYP(clSplitlotInfo.m_strUnitModel);
    clMIR.SetNODE_NAM(clSplitlotInfo.m_strStationSerial);
    strString = clSplitlotInfo.m_strStationPin + ":";
    strString += clSplitlotInfo.m_strStationModel;
    clMIR.SetTSTR_TYP(strString);
    clMIR.SetJOB_NAM("");
    clMIR.SetJOB_REV("");
    clMIR.SetSBLOT_ID(clSplitlotInfo.m_strSublot);
    clMIR.SetOPER_NAM(clSplitlotInfo.m_strTestOperatorID);
    clMIR.SetEXEC_TYP("");
    clMIR.SetEXEC_VER(clSplitlotInfo.m_strStationSoftwareRev);
    clMIR.SetTEST_COD("");
    clMIR.SetTST_TEMP("");
    clMIR.SetUSER_TXT("");
    clMIR.SetAUX_FILE("");
    clMIR.SetPKG_TYP(clSplitlotInfo.m_strTestPackageID);
    clMIR.SetFAMLY_ID("");
    clMIR.SetDATE_COD("");
    clMIR.SetFACIL_ID(clSplitlotInfo.m_strTestFacility);
    clMIR.SetFLOOR_ID("");
    clMIR.SetPROC_ID("");
    clMIR.SetOPER_FRQ("");
    clMIR.SetSPEC_NAM("");
    clMIR.SetSPEC_VER("");
    clMIR.SetFLOW_ID("");
    clMIR.SetSETUP_ID(clSplitlotInfo.m_strTestSetID);

    // Write record
    return clStdfParse.WriteRecord(&clMIR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF MRR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::WriteMRR(GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo)
{
    GQTL_STDF::Stdf_MRR_V4 clMRR;

    // Set MRR fields
    clMRR.SetFINISH_T(clSplitlotInfo.m_clMaxTestDate.toTime_t());
    clMRR.SetDISP_COD(' ');

    // Write record
    return clStdfParse.WriteRecord(&clMRR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF PIR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::WritePIR(GQTL_STDF::StdfParse & clStdfParse, int nSiteNum)
{
    GQTL_STDF::Stdf_PIR_V4 clPIR;

    // Set PIR fields
    clPIR.SetHEAD_NUM(1);
    clPIR.SetSITE_NUM(nSiteNum);

    // Write record
    return clStdfParse.WriteRecord(&clPIR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF PRR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::WritePRR(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_RunInfo & clCurrentPart)
{
    GQTL_STDF::Stdf_PRR_V4	clPRR;
    GQTL_STDF::Stdf_DTR_V4	clDTR;
    int				nHardBin, nSoftBin;

    // Write DTR record with test_date -> Time_Of_Test
    QString strDtrString = "<cmd> setValue Time_Of_Test.";
    strDtrString += QString::number(GEX_TESTNBR_OFFSET_EXT_TEST_TIME_T) + " = ";
    strDtrString += QString::number(clCurrentPart.m_tDateTimeOfTest);
    clDTR.SetTEXT_DAT(strDtrString);
    clStdfParse.WriteRecord(&clDTR);

    // If no binning specified, force default one
    if(clCurrentPart.m_nHardBin == -1)
        nHardBin = (clCurrentPart.m_bPartFailed) ? 0 : 1;
    else
        nHardBin = clCurrentPart.m_nHardBin;
    if(clCurrentPart.m_nSoftBin == -1)
        nSoftBin = (clCurrentPart.m_bPartFailed) ? 0 : 1;
    else
        nSoftBin = clCurrentPart.m_nSoftBin;

    // Set PRR fields
    clPRR.SetHEAD_NUM(1);
    clPRR.SetSITE_NUM(clCurrentPart.m_nSiteNb);
    if(clCurrentPart.m_bPartFailed)
        clPRR.SetPART_FLG(0x08);
    else
        clPRR.SetPART_FLG(0);
    clPRR.SetNUM_TEST(0);
    clPRR.SetHARD_BIN(nHardBin);
    clPRR.SetSOFT_BIN(nSoftBin);
    clPRR.SetX_COORD(clCurrentPart.m_nX);
    clPRR.SetY_COORD(clCurrentPart.m_nY);
    clPRR.SetTEST_T(clCurrentPart.m_lnTestTime);
    clPRR.SetPART_ID(clCurrentPart.m_strPartID);

    // Write record
    return clStdfParse.WriteRecord(&clPRR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF WIR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::WriteWIR(GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo)
{
    GQTL_STDF::Stdf_WIR_V4 clWIR;

    // Set WIR fields
    clWIR.SetHEAD_NUM(1);
    clWIR.SetSITE_GRP(0);
    clWIR.SetSTART_T(clSplitlotInfo.m_clMinTestDate.toTime_t());
    clWIR.SetWAFER_ID(clSplitlotInfo.m_strSublot);

    // Write record
    return clStdfParse.WriteRecord(&clWIR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF WRR record for specified Splitlot
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::WriteWRR(GQTL_STDF::StdfParse & clStdfParse, GexDbPlugin_Medtronic_SplitlotInfo & clSplitlotInfo)
{
    GQTL_STDF::Stdf_WRR_V4 clWRR;

    // Set WRR fields
    clWRR.SetHEAD_NUM(1);
    clWRR.SetSITE_GRP(0);
    clWRR.SetFINISH_T(clSplitlotInfo.m_clMaxTestDate.toTime_t());
    clWRR.SetPART_CNT(clSplitlotInfo.m_uiNbParts);
    clWRR.SetRTST_CNT(4294967295u);
    clWRR.SetABRT_CNT(4294967295u);
    clWRR.SetGOOD_CNT(clSplitlotInfo.m_uiNbParts_Good);
    clWRR.SetFUNC_CNT(4294967295u);
    clWRR.SetWAFER_ID(clSplitlotInfo.m_strSublot);

    // Write record
    return clStdfParse.WriteRecord(&clWRR);
}

//////////////////////////////////////////////////////////////////////
// Add filter on tests to query
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic::Query_AddTestlistCondition(const QString & strTestlist)
{
    // Go through all elements of the testlist (syntax is: n1-m1,y1,y2,n2-m2...)
    QStringList::Iterator	itTestlist;
    QString					strFilterValues, strCondition;
    QStringList				strlInterval, strlTestlist = strTestlist.split(QRegExp("[,;]"));
    int						nBeginValue, nEndValue, nIndex;

    for(itTestlist = strlTestlist.begin(); itTestlist != strlTestlist.end(); itTestlist++)
    {
        switch((*itTestlist).count('-'))
        {
        case 0:
            if(strFilterValues.isEmpty())
                strFilterValues += (*itTestlist);
            else
                strFilterValues += "," + (*itTestlist);
            break;

        case 1:
            strlInterval = (*itTestlist).split('-');
            nBeginValue = strlInterval[0].toInt();
            nEndValue = strlInterval[1].toInt();
            for(nIndex=nBeginValue; nIndex<=nEndValue; nIndex++)
            {
                if(strFilterValues.isEmpty())
                    strFilterValues += (*itTestlist);
                else
                    strFilterValues += "," + (*itTestlist);
            }
            break;

        default:
            break;
        }
    }

    strCondition = "test_data.test_id|Numeric|" + strFilterValues;
    m_strlQuery_ValueConditions.append(strCondition);
}

///////////////////////////////////////////////////////////
// Query: add time period condition to query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::Query_AddTimePeriodCondition(GexDbPlugin_Filter &cFilters)
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
// Query: add time period condition to query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::Query_AddTimePeriodCondition(QDateTime & clFrom, QDateTime & clTo)
{
    // Add condition based on dates computed from time_t from and to values
    QString		strFilterCondition;

    strFilterCondition =	"to_date('";
    strFilterCondition +=	clFrom.toString("yyyy_MM_dd_hh:mm:ss");
    strFilterCondition +=	"', 'YYYY_MM_DD_HH24:MI:SS')";
    strFilterCondition +=	GEXDB_PLUGIN_DELIMITER_FROMTO;
    strFilterCondition +=	"to_date('";
    strFilterCondition +=	clTo.toString("yyyy_MM_dd_hh:mm:ss");
    strFilterCondition +=	"', 'YYYY_MM_DD_HH24:MI:SS')";

    return Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, strFilterCondition, true);
}

///////////////////////////////////////////////////////////
// Retrieve static info for specified test.
// Create test structure if not found in map.
///////////////////////////////////////////////////////////
bool
GexDbPlugin_Medtronic::
GetStaticTestInfo(GexDbPlugin_Query& clGexDbQuery,
                  GexDbPlugin_Filter& /*cFilters*/,
                  GexDbPlugin_Medtronic_SplitlotInfo& /*clSplitlotInfo*/,
                  GexDbPlugin_TestInfo** ppTestInfo,
                  unsigned int uiTestID,
                  unsigned int uiTestSeq)
{
    float			fLowLimit, fHighLimit;
    unsigned int	uiTestNumber, uiIndex = 0;
    bool			bTestFound = false;

    // Check option on how to handle tests with multiple executions per run
    if(m_clOptions.m_eTestMergeOption == GexDbPlugin_Medtronic_Options::eKeepFirstResult || m_clOptions.m_eTestMergeOption == GexDbPlugin_Medtronic_Options::eKeepLastResult)
    {
        // First or Last: we keep only 1 test result, so no artificial tests will be created, and we
        // just get the test from the list, or create it if it doesn't exist.
        // We still use uiTestID*100 for the test number to be consistent with the other scenario (create multiple tests)
        uiTestNumber = uiTestID*100;
        *ppTestInfo = m_clTestList.FindTestByNb(uiTestNumber);
        bTestFound = (*ppTestInfo != NULL);
    }
    else
    {
        // Check if test already in TestList
        uiTestNumber = uiTestID*100 + uiIndex;
        *ppTestInfo = m_clTestList.FindTestByNb(uiTestNumber);
        while(*ppTestInfo && !bTestFound)
        {
            // Found a test with same TestID, check if already executed in this run (except for HEADER tests)
            if((*ppTestInfo)->m_strTestName.toLower().startsWith("header,"))
                bTestFound = true;
            else if(((*ppTestInfo)->m_lTestID == uiTestID) && ((*ppTestInfo)->m_bTestExecuted == false))
                bTestFound = true;
            else
            {
                uiTestNumber = uiTestID*100 + ++uiIndex;
                *ppTestInfo = m_clTestList.FindTestByNb(uiTestNumber);
            }
        }
    }

    // Test found ??
    if(bTestFound)
    {
        if(!clGexDbQuery.isNull(5) && clGexDbQuery.value(5).isValid())
        {
            fLowLimit = clGexDbQuery.value(5).toDouble();
            if((*ppTestInfo)->m_pTestInfo_PTR->m_bHasLL == false)
            {
                (*ppTestInfo)->m_pTestInfo_PTR->m_bHasLL = true;
                (*ppTestInfo)->m_pTestInfo_PTR->m_fLL = fLowLimit;
                (*ppTestInfo)->m_bStaticInfoWrittenToStdf = false; // This will force static info (including limits) to be re-written to STDF
            }
            else if(fLowLimit < (*ppTestInfo)->m_pTestInfo_PTR->m_fLL)
            {
                (*ppTestInfo)->m_pTestInfo_PTR->m_fLL = fLowLimit;
                (*ppTestInfo)->m_bStaticInfoWrittenToStdf = false; // This will force static info (including limits) to be re-written to STDF
            }
        }
        if(!clGexDbQuery.isNull(6) && clGexDbQuery.value(6).isValid())
        {
            fHighLimit = clGexDbQuery.value(6).toDouble();
            if((*ppTestInfo)->m_pTestInfo_PTR->m_bHasHL == false)
            {
                (*ppTestInfo)->m_pTestInfo_PTR->m_bHasHL = true;
                (*ppTestInfo)->m_pTestInfo_PTR->m_fHL = fHighLimit;
                (*ppTestInfo)->m_bStaticInfoWrittenToStdf = false; // This will force static info (including limits) to be re-written to STDF
            }
            else if(fHighLimit > (*ppTestInfo)->m_pTestInfo_PTR->m_fHL)
            {
                (*ppTestInfo)->m_pTestInfo_PTR->m_fHL = fHighLimit;
                (*ppTestInfo)->m_bStaticInfoWrittenToStdf = false; // This will force static info (including limits) to be re-written to STDF
            }
        }
        return true;
    }

    // Create test in list
    QString				strQuery, strCondition, strFieldSpec, strUnitOfMeasure, strTestName;
    GexDbPlugin_Query	clGexDbQuery_Tests(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Query test info for specified Splitlot/test
    Query_Empty();
    m_strlQuery_Fields.append("Field|test_id.test_id");
    m_strlQuery_Fields.append("Field|test_id.test_name");
    m_strlQuery_Fields.append("Field|test_id.test_label");
    m_strlQuery_Fields.append("Field|test_id.test_parameter");
    m_strlQuery_Fields.append("Field|test_id.unit_of_measure");

    // Narrow on tests specified
    strCondition = "test_id.test_id|Numeric|" + QString::number(uiTestID);
    m_strlQuery_ValueConditions.append(strCondition);

    Query_BuildSqlString(strQuery, true);

    clGexDbQuery_Tests.setForwardOnly(true);

    *ppTestInfo = new GexDbPlugin_TestInfo;
    (*ppTestInfo)->m_pTestInfo_PTR	= new GexDbPlugin_TestInfo_PTR;
    if(clGexDbQuery_Tests.Execute(strQuery) && clGexDbQuery_Tests.Next())
    {
        // Found some data in Test_ID table for this test, get it
        strTestName = clGexDbQuery_Tests.value(1).toString();
        (*ppTestInfo)->m_lTestID		= uiTestID;
        (*ppTestInfo)->m_uiTestNumber	= uiTestNumber;
        (*ppTestInfo)->m_uiTestSeq		= uiTestSeq;
        (*ppTestInfo)->m_cTestType		= 'P';
        (*ppTestInfo)->m_strTestName	= strTestName;
        (*ppTestInfo)->m_strTestName	+= "," + clGexDbQuery_Tests.value(2).toString();
        (*ppTestInfo)->m_strTestName	+= "," + clGexDbQuery_Tests.value(3).toString();
        if(strTestName.trimmed().toLower() != "header")
            (*ppTestInfo)->m_strTestName	+= "_" + QString::number(uiIndex);
        if(!clGexDbQuery_Tests.isNull(4) && clGexDbQuery_Tests.value(4).isValid())
        {
            strUnitOfMeasure = clGexDbQuery_Tests.value(4).toString();
            if(!strUnitOfMeasure.isEmpty() && (strUnitOfMeasure.toLower() != "na"))
                (*ppTestInfo)->m_pTestInfo_PTR->m_strTestUnit = strUnitOfMeasure;
        }
    }
    else
    {
        // No data in test_id for this test: use data from test_data table
        (*ppTestInfo)->m_lTestID		= uiTestID;
        (*ppTestInfo)->m_uiTestNumber	= uiTestNumber;
        (*ppTestInfo)->m_uiTestSeq		= uiTestSeq;
        (*ppTestInfo)->m_cTestType		= 'P';
        (*ppTestInfo)->m_strTestName	= "T" + QString::number(uiTestID);
        (*ppTestInfo)->m_strTestName	+= "_" + QString::number(uiIndex);
    }

    // Set limits
    if(!clGexDbQuery.isNull(5) && clGexDbQuery.value(5).isValid())
    {
        (*ppTestInfo)->m_pTestInfo_PTR->m_bHasLL = true;
        (*ppTestInfo)->m_pTestInfo_PTR->m_fLL = clGexDbQuery.value(5).toDouble();
    }
    if(!clGexDbQuery.isNull(6) && clGexDbQuery.value(6).isValid())
    {
        (*ppTestInfo)->m_pTestInfo_PTR->m_bHasHL = true;
        (*ppTestInfo)->m_pTestInfo_PTR->m_fHL = clGexDbQuery.value(6).toDouble();
    }

    // Add test to list
    m_clTestList.Insert(*ppTestInfo);

    return true;
}

///////////////////////////////////////////////////////////
// Returns a pointer on an options GUI.
///////////////////////////////////////////////////////////
QWidget *GexDbPlugin_Medtronic::GetRdbOptionsWidget()
{
    return (QWidget *)m_pclRdbOptionsWidget;
}

///////////////////////////////////////////////////////////
// Return a string with plug-in options.
///////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::GetRdbOptionsString(QString & strRdBOptionsString)
{
    m_pclRdbOptionsWidget->GetOptionsString(strRdBOptionsString);
    return true;
}

///////////////////////////////////////////////////////////
// Sets the plug-in options GUI according to options string passed as argument.
///////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::SetRdbOptionsString(const QString & strRdBOptionsString)
{
    // Init plug-in options
    m_clOptions.Init(strRdBOptionsString);

    m_pclRdbOptionsWidget->SetOptionsString(m_clOptions);
    return true;
}

//////////////////////////////////////////////////////////////////////
// Write test information for current run
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Medtronic::WriteTestInfo(GQTL_STDF::StdfParse & clStdfParse, const GexDbPlugin_RunInfo & clCurrentPart)
{
    GQTL_STDF::Stdf_PTR_V4	clPTR;
    GQTL_STDF::Stdf_MPR_V4	clMPR;
    GQTL_STDF::Stdf_FTR_V4	clFTR;
    int				nMPR_MaxArrayIndex;
    BYTE			bOptFlag;

    QList<GexDbPlugin_TestInfo*>	pMPR_Tests;
    GexDbPlugin_TestInfo			*pTestInfo, *pMPR_RefTest;
    GexDbPlugin_TestInfoContainer	*pContainer;

    pContainer = m_clTestList.m_pFirstTest;
    while(pContainer)
    {
        pTestInfo = pContainer->m_pTestInfo;

        if(pTestInfo->m_bTestExecuted && (pTestInfo->m_cTestType == 'P'))
        {
            while(!pTestInfo->m_pTestInfo_PTR->m_uiDynamicFlagsList.isEmpty())
            {
                // Set PTR fields
                clPTR.SetTEST_NUM(pTestInfo->m_uiTestNumber);
                clPTR.SetHEAD_NUM(1);
                clPTR.SetSITE_NUM(clCurrentPart.m_nSiteNb);
                clPTR.SetTEST_FLG(pTestInfo->m_pTestInfo_PTR->m_uiDynamicFlagsList.takeFirst());
                clPTR.SetPARM_FLG(pTestInfo->m_pTestInfo_PTR->m_uiStaticFlags);
                clPTR.SetRESULT(pTestInfo->m_pTestInfo_PTR->m_fResultList.takeFirst());

                if(!pTestInfo->m_bStaticInfoWrittenToStdf)
                {
                    bOptFlag = 0;
                    if(!pTestInfo->m_pTestInfo_PTR->m_bHasLL)
                        bOptFlag |= 0x40;
                    if(!pTestInfo->m_pTestInfo_PTR->m_bHasHL)
                        bOptFlag |= 0x80;

                    clPTR.SetTEST_TXT(pTestInfo->m_strTestName);
                    clPTR.SetALARM_ID("");
                    clPTR.SetOPT_FLAG(bOptFlag);
                    clPTR.SetRES_SCAL(0);
                    clPTR.SetLLM_SCAL(0);
                    clPTR.SetHLM_SCAL(0);
                    clPTR.SetLO_LIMIT(pTestInfo->m_pTestInfo_PTR->m_fLL);
                    clPTR.SetHI_LIMIT(pTestInfo->m_pTestInfo_PTR->m_fHL);
                    clPTR.SetUNITS(pTestInfo->m_pTestInfo_PTR->m_strTestUnit);

                    pTestInfo->m_bStaticInfoWrittenToStdf = true;
                }

                // Write record
                clStdfParse.WriteRecord(&clPTR);
                clPTR.Reset();
            }


            pContainer = pContainer->m_pNextTest;
        }
        else if(pTestInfo->m_bTestExecuted && (pTestInfo->m_cTestType == 'M'))
        {
            pMPR_RefTest = pTestInfo;

            while(!pTestInfo->m_pTestInfo_MPR->m_uiDynamicFlagsList.isEmpty())
            {
                // Save all MPR's with same test nb and test name
                pMPR_Tests.append(pTestInfo);
                nMPR_MaxArrayIndex = pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex;
                pContainer = pContainer->m_pNextTest;
                while(pContainer)
                {
                    pTestInfo = pContainer->m_pTestInfo;
                    if((pTestInfo->m_uiTestNumber != pMPR_RefTest->m_uiTestNumber) || (pTestInfo->m_strTestName != pMPR_RefTest->m_strTestName))
                        break;
                    pMPR_Tests.append(pTestInfo);
                    nMPR_MaxArrayIndex = qMax(nMPR_MaxArrayIndex, pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex);
                    pContainer = pContainer->m_pNextTest;
                }

                // Set MPR fields
                clMPR.SetTEST_NUM(pMPR_RefTest->m_uiTestNumber);
                clMPR.SetHEAD_NUM(1);
                clMPR.SetSITE_NUM(clCurrentPart.m_nSiteNb);
                clMPR.SetTEST_FLG(pMPR_RefTest->m_pTestInfo_MPR->m_uiDynamicFlagsList.takeFirst());
                clMPR.SetPARM_FLG(pMPR_RefTest->m_pTestInfo_MPR->m_uiStaticFlags);
                clMPR.SetRTN_ICNT(nMPR_MaxArrayIndex+1);
                clMPR.SetRSLT_CNT(nMPR_MaxArrayIndex+1);

                QListIterator<GexDbPlugin_TestInfo*> lstIteratorMPRTests(pMPR_Tests);

                lstIteratorMPRTests.toFront();
                while(lstIteratorMPRTests.hasNext())
                {
                    pTestInfo = lstIteratorMPRTests.next();
                    clMPR.SetRTN_STAT(pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex, 0);
                    if(!pTestInfo->m_pTestInfo_MPR->m_fResultList.isEmpty())
                        clMPR.SetRTN_RSLT(pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex, pTestInfo->m_pTestInfo_MPR->m_fResultList.takeFirst());
                    if(!pTestInfo->m_pTestInfo_MPR->m_nTpin_PmrIndexList.isEmpty())
                        clMPR.SetRTN_INDX(pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex, pTestInfo->m_pTestInfo_MPR->m_nTpin_PmrIndexList.takeFirst());
                }

                clMPR.SetTEST_TXT(pMPR_RefTest->m_strTestName);
                clMPR.SetALARM_ID("");
                if(!pMPR_RefTest->m_bStaticInfoWrittenToStdf)
                {
                    bOptFlag = 0;
                    if(!pMPR_RefTest->m_pTestInfo_MPR->m_bHasLL)
                        bOptFlag |= 0x40;
                    if(!pMPR_RefTest->m_pTestInfo_MPR->m_bHasHL)
                        bOptFlag |= 0x80;

                    clMPR.SetOPT_FLAG(bOptFlag);
                    clMPR.SetRES_SCAL(0);
                    clMPR.SetLLM_SCAL(0);
                    clMPR.SetHLM_SCAL(0);
                    clMPR.SetLO_LIMIT(pMPR_RefTest->m_pTestInfo_MPR->m_fLL);
                    clMPR.SetHI_LIMIT(pMPR_RefTest->m_pTestInfo_MPR->m_fHL);
                    clMPR.SetSTART_IN(0.0F);
                    clMPR.SetINCR_IN(0.0F);
                    clMPR.SetUNITS(pMPR_RefTest->m_pTestInfo_MPR->m_strTestUnit);

                    pMPR_RefTest->m_bStaticInfoWrittenToStdf = true;
                }
                else
                    clMPR.SetOPT_FLAG(0x30);	// No LL/HL, use the ones in first MPR for same test

                // Write record
                clStdfParse.WriteRecord(&clMPR);
                pMPR_Tests.clear();
                clMPR.Reset();

                pTestInfo = pMPR_RefTest;
            }
        }
        else if(pTestInfo->m_bTestExecuted && (pTestInfo->m_cTestType == 'F'))
        {
            while(!pTestInfo->m_pTestInfo_FTR->m_uiDynamicFlagsList.isEmpty())
            {
                // Set FTR fields
                clFTR.SetTEST_NUM(pTestInfo->m_uiTestNumber);
                clFTR.SetHEAD_NUM(1);
                clFTR.SetSITE_NUM(clCurrentPart.m_nSiteNb);
                clFTR.SetTEST_FLG(pTestInfo->m_pTestInfo_FTR->m_uiDynamicFlagsList.takeFirst());
                clFTR.SetOPT_FLAG((BYTE)0xdf);		// Only VECT_OFF is valid
                clFTR.SetCYCL_CNT(0);
                clFTR.SetREL_VADR(0);
                clFTR.SetREPT_CNT(0);
                clFTR.SetNUM_FAIL(0);
                clFTR.SetXFAIL_AD(0);
                clFTR.SetYFAIL_AD(0);
                clFTR.SetVECT_OFF(pTestInfo->m_pTestInfo_FTR->m_uiVectorOffsetList.takeFirst());
                clFTR.SetRTN_ICNT(0);
                clFTR.SetPGM_ICNT(0);
                GSLOG(SYSLOG_SEV_WARNING, "check me : FTR set fail pin ?");
                clFTR.SetFAIL_PIN(); //clFTR.SetFAIL_PIN(0); // Zied/Bernard ?
                clFTR.SetVECT_NAM(pTestInfo->m_pTestInfo_FTR->m_strVectorNameList.takeFirst());
                if(!pTestInfo->m_bStaticInfoWrittenToStdf)
                {
                    clFTR.SetTIME_SET("");
                    clFTR.SetOP_CODE("");
                    clFTR.SetTEST_TXT(pTestInfo->m_strTestName);

                    pTestInfo->m_bStaticInfoWrittenToStdf = true;
                }

                // Write record
                clStdfParse.WriteRecord(&clFTR);
                clFTR.Reset();
            }

            pContainer = pContainer->m_pNextTest;
        }
        else
            pContainer = pContainer->m_pNextTest;
    }

    return true;
}


bool GexDbPlugin_Medtronic::LoadDatabaseArchitecture()
{
    return false;
}

bool GexDbPlugin_Medtronic::SPM_GetProductList(QString /*testingStage*/,
                                               QString /*productRegexp*/,
                                               QStringList & /*cMatchingValues*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SPM fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}
bool GexDbPlugin_Medtronic::SPM_GetFlowList(QString /*testingStage*/,
                                            QString /*productRegexp*/,
                                            QStringList & /*cMatchingValues*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SPM fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}
bool GexDbPlugin_Medtronic::SPM_GetInsertionList(QString /*testingStage*/,
                                                 QString /*productRegexp*/,
                                                 QString /*flowRegexp*/,
                                                 QStringList & /*cMatchingValues*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SPM fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}
bool GexDbPlugin_Medtronic::SPM_GetItemsList(QString /*testingStage*/,
                                             QString /*productRegexp*/,
                                             QString /*flowRegexp*/,
                                             QString /*insertionRegexp*/,
                                             QString /*testType*/,
                                             QStringList & /*cMatchingValues*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SPM fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}

bool GexDbPlugin_Medtronic::SPM_FetchWaferKeysFromFilters(QString /*testingStage*/, //in
                                                          QString /*productRegexp*/, //in
                                                          QString /*lotId*/, //in
                                                          QString /*sublotId*/, //in
                                                          QString /*waferId*/, //in
                                                          const QMap<QString, QString> &/*filtersMetaData*/, //in
                                                          QStringList& /*waferKeyList*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SPM fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}

bool GexDbPlugin_Medtronic::SPM_GetConditionsFromFilters(QString /*testingStage*/, //in
                                                         const QMap<QString, QString> &/*filters*/, //in
                                                         QMap<QString, QStringList> & /*conditions*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SPM fetch conditions : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}

bool GexDbPlugin_Medtronic::SPM_FetchDataPointsForComputing(QString /*testingStage*/, //in
                                                            QString /*productRegexp*/, //in
                                                            QString /*monitoredItemType*/, //in
                                                            const QList<MonitoredItemRule>& /*monitoredItemRules*/, //in
                                                            MonitoredItemUniqueKeyRule /*uniqueKeyRule*/, //in
                                                            QString /*testFlow*/, //in
                                                            QString /*consolidationType*/, //in
                                                            QString /*consolidationLevel*/, //in
                                                            QString /*testInsertion*/, //in
                                                            const QStringList& /*statsToMonitor*/, //in
                                                            QString /*siteMergeMode*/, //in
                                                            bool /*useGrossDie*/, //in
                                                            const QMap<QString, QStringList>& /*filtersConditions*/, //in
                                                            QDateTime /*dateFrom*/, //in
                                                            QDateTime /*dateTo*/, //in
                                                            QStringList& /*productsMatched*/, //out
                                                            int& /*numLotsMatched*/, //out
                                                            int& /*numDataPointsMatched*/, //out
                                                            QSet<int>& /*siteList*/, //out
                                                            QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& /*testToSiteToStatToValues*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SPM fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}

bool GexDbPlugin_Medtronic::SPM_FetchDataPointsForCheckOnTrigger(QString /*testingStage*/, //in
                                                                 QString /*productRegexp*/, //in
                                                                 QString /*lotId*/, //in
                                                                 QString /*sublotId*/, //in
                                                                 QString /*waferId*/, //in
                                                                 const QList<MonitoredItemDesc>& /*testList*/, //in
                                                                 MonitoredItemUniqueKeyRule /*uniqueKeyRule*/, //in
                                                                 QString /*testFlow*/, //in
                                                                 QString /*consolidationType*/, //in
                                                                 QString /*consolidationLevel*/, //in
                                                                 QString /*testInsertion*/, //in
                                                                 const QList<int>& /*siteList*/, //in
                                                                 const QList<QString> &/*statsList*/, //in
                                                                 bool /*useGrossDie*/, //in
                                                                 const QDateTime* /*dateFrom*/, //in
                                                                 const QDateTime* /*dateTo*/, //in
                                                                 const QMap<QString, QStringList>& /*filtersConditions*/, //in
                                                                 QString& /*productList*/, //out
                                                                 QString& /*lotList*/, //out
                                                                 QString& /*sublotList*/, //out
                                                                 QString& /*waferList*/, //out
                                                                 int& /*numParts*/, //out
                                                                 QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& /*testToSiteToStatToDataPoint*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SPM fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}

bool GexDbPlugin_Medtronic::SPM_FetchDataPointsForCheckOnInsertion(QString /*testingStage*/, //in
                                                                   int /*splitlotId*/, //in
                                                                   const QMap<QString,QString> &/*filters*/, //in
                                                                   const QList<MonitoredItemDesc> &/*testList*/, //in
                                                                   MonitoredItemUniqueKeyRule /*uniqueKeyRule*/, //in
                                                                   const QList<int> &/*siteList*/, //in
                                                                   const QList<QString> &/*statsList*/, //in
                                                                   QString& , //out
                                                                   QString& , //out
                                                                   QString& , //out
                                                                   QString& , //out
                                                                   int& , //out
                                                                   QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& )
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SPM fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}

bool GexDbPlugin_Medtronic::SYA_FetchDataPointsForComputing(QString /*testingStage*/, //in
                                                            QString /*productRegexp*/, //in
                                                            const QMap<QString,QString>& /*filters*/, //in
                                                            QString /*monitoredItemType*/, //in
                                                            const QList<MonitoredItemRule>& /*monitoredItemRules*/, //in
                                                            const QStringList& /*binsToExclude*/, //in
                                                            MonitoredItemUniqueKeyRule /*uniqueKeyRule*/, //in
                                                            QString /*testFlow*/, //in
                                                            QString /*consolidationType*/, //in
                                                            QString /*consolidationLevel*/, //in
                                                            QString /*testInsertion*/, //in
                                                            const QStringList& /*statsToMonitor*/, //in
                                                            QString /*siteMergeMode*/, //in
                                                            bool /*useGrossDie*/, //in
                                                            QDateTime /*computeFrom*/, //in
                                                            QDateTime /*computeTo*/, //in
                                                            QStringList& /*productsMatched*/, //out
                                                            int& /*numLotsMatched*/, //out
                                                            int& /*numDataPointsMatched*/, //out
                                                            QSet<int>& /*siteList*/, //out
                                                            QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& /*binToSiteToStatToValues*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SYA fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}

bool GexDbPlugin_Medtronic::SYA_FetchDataPointsForCheckOnTrigger(QString /*testingStage*/, //in
                                                                 QString /*productRegexp*/, //in
                                                                 QString /*lotId*/, //in
                                                                 QString /*sublotId*/, //in
                                                                 QString /*waferId*/, //in
                                                                 const QMap<QString,QString>& /*filters*/, //in
                                                                 const QList<MonitoredItemDesc>& /*binList*/, //in
                                                                 const QStringList& /*binsToExclude*/, //in
                                                                 MonitoredItemUniqueKeyRule /*uniqueKeyRule*/, //in
                                                                 QString /*testFlow*/, //in
                                                                 QString /*consolidationType*/, //in
                                                                 QString /*consolidationLevel*/, //in
                                                                 QString /*testInsertion*/, //in
                                                                 const QList<int>& /*siteList*/, //in
                                                                 const QList<QString>& /*statsList*/, //in
                                                                 bool /*useGrossDie*/, //in
                                                                 const QDateTime* /*dateFrom*/, //in
                                                                 const QDateTime* /*dateTo*/, //in
                                                                 QString& /*productList*/, //out
                                                                 QString& /*lotList*/, //out
                                                                 QString& /*sublotList*/, //out
                                                                 QString& /*waferList*/, //out
                                                                 int& /*numParts*/, //out
                                                                 QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& /*binToSiteToStatToDataPoint*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SYA fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}

bool GexDbPlugin_Medtronic::SYA_FetchDataPointsForCheckOnInsertion(QString /*testingStage*/, //in
                                                                   int /*splitlotId*/, //in
                                                                   const QMap<QString,QString>& /*filters*/, //in
                                                                   const QList<MonitoredItemDesc>& /*binList*/, //in
                                                                   const QStringList& /*binsToExclude*/, //in
                                                                   MonitoredItemUniqueKeyRule /*uniqueKeyRule*/, //in
                                                                   const QList<int>& /*siteList*/, //in
                                                                   const QList<QString>& /*statsList*/, //in
                                                                   bool /*useGrossDie*/, //in
                                                                   QString& /*productList*/, //out
                                                                   QString& /*lotList*/, //out
                                                                   QString& /*sublotList*/, //out
                                                                   QString& /*waferList*/, //out
                                                                   int& /*numParts*/, //out
                                                                   QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& /*binToSiteToStatToDataPoint*/)
{
    // Set error and return
    GSLOG(SYSLOG_SEV_ERROR, "SYA fetch datapoints : virtual method not implemented");
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), __FUNCTION__);
    return false;
}
