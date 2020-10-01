// gexdb_plugin_dialogsemi.cpp: implementation of the GexDbPlugin_Dialogsemi class.
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
#include "gexdb_plugin_dialogsemi.h"
#include "gexdb_plugin_dialogsemi_cfgwizard.h"
#include "gex_constants.h"

// Standard includes

// Qt includes
#include <qregexp.h>
#include <QMainWindow>
#include <QSqlError>


// Galaxy modules includes



////////////////////////////////////////////////////////////////////////////////////
// Constants and Macro definitions
////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------------------------------------
// Only declare a DLL entry point under Windows.
// Under UNIX system, nothing needed for a shared library.
// ----------------------------------------------------------------------------------------------------------
#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#	include <windows.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
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
    GexDbPlugin_Dialogsemi *pObject = new GexDbPlugin_Dialogsemi(pMainWindow, strHostName, strApplicationPath, strUserProfile, strLocalFolder, gexLabelFilterChoices, bCustomerDebugMode, pGexSkin);
	return (GexDbPlugin_Base *)pObject;
}

extern "C" GEXDB_PLUGIN_API void gexdb_plugin_releaseobject(GexDbPlugin_Base *pObject)
{
	delete pObject;
}

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Dialogsemi class: database plugin class for DIALOGSEMI database type
////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexDbPlugin_Dialogsemi::GexDbPlugin_Dialogsemi(QMainWindow *pMainWindow, const QString & strHostName, const QString & strApplicationPath, const QString & strUserProfile, const QString & strLocalFolder, const char *gexLabelFilterChoices[], const bool bCustomerDebugMode, CGexSkin * pGexSkin, GexDbPlugin_Connector *pclDatabaseConnector) :
GexDbPlugin_Base(pMainWindow, strHostName, strApplicationPath, strUserProfile, strLocalFolder, gexLabelFilterChoices, bCustomerDebugMode, pGexSkin, pclDatabaseConnector)
{
	// Variables init
	m_strPluginName = GEXDB_PLUGIN_DIALOGSEMI_NAME;
#if PROFILERON
	m_bProfilerON = true;
#else
	m_bProfilerON = false;
#endif
	
	GexDbPlugin_Mapping_FieldMap::Iterator itMapping;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Wafer Sort
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	m_pPartInfoMatrix = NULL;
	m_uiWaferNbParts = 0;

	// Gex fields <-> Dialogsemi DB fields mapping
	// 1. Fields exported in the GEX GUI
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME], 
		"waf_prober_info",	"waf_prober_info.loadboard",		"",										true);
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], 
		"waf_prober_info",	"waf_prober_info.batchnumber",		"",										true);
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], 
		"waf_prober_info",	"waf_prober_info.waferid",			"",										true);
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME], 
		"waf_prober_info",	"waf_prober_info.handingsys",		"",										true);
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY], 
		"waf_prober_info",	"waf_prober_info.facility",			"",										true);
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], 
		"waf_prober_info",	"waf_prober_info.device_id",		"",										true);
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME], 
		"waf_prober_info",	"waf_prober_info.progname",			"",										true);
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION], 
		"waf_prober_info",	"waf_prober_info.prog_revision",	"",										true);
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE], 
		"waf_prober_info",	"waf_prober_info.temp",				"",										true);
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME], 
		"waf_prober_info",	"waf_prober_info.testsys",			"",										true);
	m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR], 
		"waf_test_fail_map","waf_test_fail_map.site",		"waf_test_fail_map-waf_prober_info",		true);

	// 2. Fields not exported in the GEX GUI, but needed in our queries
	m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_TESTNAME] =
		GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNAME, GEXDB_PLUGIN_DBFIELD_TESTNAME, 
		"waf_test_info",	"waf_test_info.testname",		"waf_test_info-waf_prober_info",			false);
	m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_TESTNUM] =
		GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNUM, GEXDB_PLUGIN_DBFIELD_TESTNUM, 
		"waf_test_info",	"waf_test_info.testnum",		"waf_test_info-waf_prober_info",			false);
	m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_STARTDATETIME] =
		GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, GEXDB_PLUGIN_DBFIELD_STARTDATETIME, 
		"waf_prober_info",	"waf_prober_info.test_startdate","",										false);

	// Links in Dialogsemi DB
	m_mapLinks_Remote_Wt["waf_test_fail_map-waf_prober_info"] = 
		GexDbPlugin_Mapping_Link("waf_test_fail_map-waf_prober_info", "waf_test_fail_map", "waf_test_fail_map.waf_prober_info_id", "waf_prober_info", "waf_prober_info.waf_prober_info_id", "");
	m_mapLinks_Remote_Wt["waf_test_info-waf_prober_info"] = 
		GexDbPlugin_Mapping_Link("waf_test_info-waf_prober_info", "waf_test_info", "waf_test_info.waf_prober_info_id", "waf_prober_info", "waf_prober_info.waf_prober_info_id", "");

	// List of available filters
	for(itMapping = m_mapFields_GexToRemote_Wt.begin(); itMapping != m_mapFields_GexToRemote_Wt.end(); itMapping++)
	{
		if((*itMapping).m_bDisplayInQueryGui)
			m_strlLabelFilterChoices_Wt.append((*itMapping).m_strMetaDataName);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Final Test
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	m_pPartInfoArray = NULL;
	m_uiLotNbParts = 0;

	// Gex fields <-> Dialogsemi DB fields mapping
	// 1. Fields exported in the GEX GUI
	m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME], 
		"ft_job_info",		"ft_job_info.loadboard",			"",										true);
	m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], 
		"ft_job_info",		"ft_job_info.batchnumber",			"",										true);
	m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME], 
		"ft_job_info",		"ft_job_info.handingsys",			"",										true);
	m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], 
		"ft_job_info",		"ft_job_info.device_id",			"",										true);
	m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME], 
		"ft_job_info",		"ft_job_info.progname",				"",										true);
	m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION], 
		"ft_job_info",		"ft_job_info.prog_revision",		"",										true);
	m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE], 
		"ft_job_info",		"ft_job_info.temp",					"",										true);
	m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME], 
		"ft_job_info",		"ft_job_info.testsys",				"",										true);
	m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR]] =
		GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR], 
		"ft_test_fail",		"ft_test_fail.site",				"ft_test_fail-ft_job_info",				true);
	// 2. Fields not exported in the GEX GUI, but needed in our queries
	m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TESTNAME] =
		GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNAME, GEXDB_PLUGIN_DBFIELD_TESTNAME, 
		"ft_test_info",		"ft_test_info.testname",			"ft_test_info-ft_job_info",				true);
	m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TESTNUM] =
		GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNUM, GEXDB_PLUGIN_DBFIELD_TESTNUM, 
		"ft_test_info",		"ft_test_info.testnum",				"ft_test_info-ft_job_info",				true);
	m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_STARTDATETIME] =
		GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, GEXDB_PLUGIN_DBFIELD_STARTDATETIME, 
		"ft_job_info",		"ft_job_info.test_startdate",		"",										true);

	// Links in Dialogsemi DB
	m_mapLinks_Remote_Ft["ft_test_fail-ft_job_info"] = 
		GexDbPlugin_Mapping_Link("ft_test_fail-ft_job_info", "ft_test_fail", "ft_test_fail.ft_job_info_id", "ft_job_info", "ft_job_info.ft_job_info_id", "");
	m_mapLinks_Remote_Ft["ft_test_info-ft_job_info"] = 
		GexDbPlugin_Mapping_Link("ft_test_info-ft_job_info", "ft_test_info", "ft_test_info.ft_job_info_id", "ft_job_info", "ft_job_info.ft_job_info_id", "");

	// List of available filters
	for(itMapping = m_mapFields_GexToRemote_Ft.begin(); itMapping != m_mapFields_GexToRemote_Ft.end(); itMapping++)
	{
		if((*itMapping).m_bDisplayInQueryGui)
			m_strlLabelFilterChoices_Ft.append((*itMapping).m_strMetaDataName);
	}
}

GexDbPlugin_Dialogsemi::~GexDbPlugin_Dialogsemi()
{
	// Free ressources
	if(m_pPartInfoMatrix)
		delete [] m_pPartInfoMatrix;
	if(m_pPartInfoArray)
		delete [] m_pPartInfoArray;
}

void GexDbPlugin_Dialogsemi::GetSupportedTestingStages(QString & strSupportedTestingStages)
{
	strSupportedTestingStages = GEXDB_PLUGIN_DIALOGSEMI_FTEST;
	strSupportedTestingStages += ";";
	strSupportedTestingStages += GEXDB_PLUGIN_DIALOGSEMI_WTEST;
}

//////////////////////////////////////////////////////////////////////
// Call plugin configuration wizard
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::ConfigWizard()
{
	// Create Connector, unless not privately owned
	if(m_bPrivateConnector)
	{
		// Create a new connector?
		if(!m_pclDatabaseConnector)
			m_pclDatabaseConnector = new GexDbPlugin_Connector(m_strPluginName);
	}

	// Create Wizard
	GexDbPlugin_Dialogsemi_CfgWizard clWizard(m_strlGexFields, m_pGexSkin, m_pMainWindow);
	clWizard.Set(*m_pclDatabaseConnector);
	if(clWizard.exec() == QDialog::Accepted)
	{
		// Retrieve values from wizard
		clWizard.Get(*m_pclDatabaseConnector);
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// Write STDF PIR record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WritePIR(CStdfParse_V4 & clStdfParse, int nSiteNum)
{
	CStdf_PIR_V4 clPIR;

	// Set PIR fields
	clPIR.SetHEAD_NUM(0);
	clPIR.SetSITE_NUM(nSiteNum);

	// Write record
	return clStdfParse.WriteRecord(&clPIR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF PRR record
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WritePRR(CStdfParse_V4 & clStdfParse, int nSiteNum, int nBinning, int nX, int nY, int nSerialNumber/*=-1*/)
{
	CStdf_PRR_V4 clPRR;

	// Set PRR fields
	clPRR.SetHEAD_NUM(0);
	clPRR.SetSITE_NUM(nSiteNum);
	clPRR.SetPART_FLG(0);
	clPRR.SetNUM_TEST(0);
	clPRR.SetHARD_BIN(nBinning);
	clPRR.SetSOFT_BIN(nBinning);
	clPRR.SetX_COORD(nX);
	clPRR.SetY_COORD(nY);
	if(nSerialNumber != -1)
	{
		clPRR.SetTEST_T(0);
		clPRR.SetPART_ID(QString::number(nSerialNumber).latin1());
	}

	// Write record
	return clStdfParse.WriteRecord(&clPRR);
}

//////////////////////////////////////////////////////////////////////
// Return TestID for soecified test number, or -1 if test not found
//////////////////////////////////////////////////////////////////////
Q_LLONG	GexDbPlugin_Dialogsemi::GetTestID(unsigned int uiTestNum)
{
	QMap<Q_LLONG, GexDbPlugin_TestInfo>::Iterator it;

	for(it=m_mapTestlist.begin(); it!=m_mapTestlist.end(); it++)
	{
		if((*it).m_uiTestNumber == uiTestNum)
			return it.key();
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////
// Add filter on tests to query
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Dialogsemi::Query_AddTestlistCondition(const QString & strTestlist)
{		
	// Go through all elements of the testlist (syntax is: n1-m1,y1,y2,n2-m2...)
	QStringList::Iterator	itTestlist;
	QString					strFilterValues, strCondition;
	QStringList				strlInterval, strlTestlist = QStringList::split(QRegExp("[,;]"), strTestlist);
	unsigned int			uiBeginValue, uiEndValue, uiIndex;
	Q_LLONG					lTestID;

	for(itTestlist = strlTestlist.begin(); itTestlist != strlTestlist.end(); itTestlist++)
	{
		switch((*itTestlist).count('-'))
		{
			case 0:
				lTestID = GetTestID((*itTestlist).toUInt());
				if(lTestID != -1)
				{
					if(strFilterValues.isEmpty())
						strFilterValues += QString::number(lTestID);
					else
						strFilterValues += "," + QString::number(lTestID);
				}
				break;

			case 1:
				strlInterval = QStringList::split('-', (*itTestlist));
				uiBeginValue = strlInterval[0].toUInt();
				uiEndValue = strlInterval[1].toUInt();
				for(uiIndex=uiBeginValue; uiIndex<=uiEndValue; uiIndex++)
				{
					lTestID = GetTestID(uiIndex);
					if(lTestID != -1)
					{
						if(strFilterValues.isEmpty())
							strFilterValues += QString::number(lTestID);
						else
							strFilterValues += "," + QString::number(lTestID);
					}
				}
				break;

			default:
				break;
		}
	}

	if(m_strTestingStage == GEXDB_PLUGIN_DIALOGSEMI_WTEST)
		strCondition = "waf_test_maps.waf_test_info_id|Numeric|" + strFilterValues;
	if(m_strTestingStage == GEXDB_PLUGIN_DIALOGSEMI_FTEST)
		strCondition = "ft_test_results_l.ft_test_info_id|Numeric|" + strFilterValues;
	m_strlQuery_ValueConditions.append(strCondition);
}

//////////////////////////////////////////////////////////////////////
// Write summary records (SBR, HBE , PCR, TSR)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteSummaryRecords(CStdfParse_V4 & clStdfParse)
{
	CStdf_SBR_V4	clSBR;
	CStdf_HBR_V4	clHBR;
	CStdf_PCR_V4	clPCR;
	int				nSiteNum, nBinning, nBinCount;
	QChar			cBinCat;
	QString			strBinName;
	QMap<int, GexDbPlugin_BinInfo>::Iterator	itBinning;
	QMap<int, GexDbPlugin_SiteInfo>::Iterator	itSite;

	// Go through site map
	for(itSite=m_mapSites.begin(); itSite!=m_mapSites.end(); itSite++)
	{
		nSiteNum = itSite.key();

		// Write PCR for this site
		if(nSiteNum == 255)
			clPCR.SetHEAD_NUM(255);
		else
			clPCR.SetHEAD_NUM(0);
		clPCR.SetSITE_NUM(nSiteNum);
		clPCR.SetPART_CNT((*itSite).m_nPartCount);
		clPCR.SetRTST_CNT((*itSite).m_nRetestCount);
		clPCR.SetABRT_CNT(0);
		clPCR.SetGOOD_CNT((*itSite).m_nGoodCount);
		clStdfParse.WriteRecord(&clPCR);

		// Go through binnings for this site
		for(itBinning=(*itSite).m_mapSoftBinnings.begin(); itBinning!=(*itSite).m_mapSoftBinnings.end(); itBinning++)
		{
			nBinning = itBinning.key();		
			nBinCount = (*itBinning).m_nBinCount;
			cBinCat = (*itBinning).m_cBinCat;
			strBinName = (*itBinning).m_strBinName;

			// Set SBR fields
			if(nSiteNum == 255)
				clSBR.SetHEAD_NUM(255);
			else
				clSBR.SetHEAD_NUM(0);
			clSBR.SetSITE_NUM(nSiteNum);
			clSBR.SetSBIN_NUM(nBinning);
			clSBR.SetSBIN_CNT(nBinCount);
			clSBR.SetSBIN_PF(cBinCat.toLatin1());
			clSBR.SetSBIN_NAM(strBinName);
			// Write record
			clStdfParse.WriteRecord(&clSBR);

			// Set HBR fields
			if(nSiteNum == 255)
				clHBR.SetHEAD_NUM(255);
			else
				clHBR.SetHEAD_NUM(0);
			clHBR.SetSITE_NUM(nSiteNum);
			clHBR.SetHBIN_NUM(nBinning);
			clHBR.SetHBIN_CNT(nBinCount);
			clHBR.SetHBIN_PF(cBinCat.toLatin1());
			clHBR.SetHBIN_NAM(strBinName);
			// Write record
			clStdfParse.WriteRecord(&clHBR);
		}
	}

	return true;
}

///////////////////////////////////////////////////////////
// Get nb of STDF files matching query
///////////////////////////////////////////////////////////
int GexDbPlugin_Dialogsemi::GetNbOfFilesToGenerate(GexDbPlugin_Filter &cFilters)
{
	QString strQuery;

	// Construct query string:
	// SELECT count(waf_prober_info_id)
	// FROM <all required tables>
	// WHERE <link conditions> AND <value conditions>	(optional)
	Query_Empty();
	if(m_strTestingStage == GEXDB_PLUGIN_DIALOGSEMI_WTEST)
		m_strlQuery_Fields.append("Function|waf_prober_info.waf_prober_info_id|COUNT");
	if(m_strTestingStage == GEXDB_PLUGIN_DIALOGSEMI_FTEST)
		m_strlQuery_Fields.append("Function|ft_job_info.ft_job_info_id|COUNT");
	// Set filters
	Query_AddFilters(cFilters);

	// Add time period condition
	Query_AddTimePeriodCondition(cFilters);

	// Construct query from table and conditions
	Query_BuildSqlString(strQuery, false);

	// Execute query
	QSqlQuery	clQuery(QString::null, m_pclDatabaseConnector->m_sqlDatabase);
	clQuery.setForwardOnly(true);
	if(!clQuery.exec(strQuery))
	{
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQuery.lastError().text().latin1());
		return -1;
	}

	clQuery.next();

	return clQuery.value(0).toInt();
}

///////////////////////////////////////////////////////////
// Query: add time period condition to query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::Query_AddTimePeriodCondition(GexDbPlugin_Filter &cFilters)
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
	strFilterCondition +=	clDateTimeFrom.toString("yyyy_mm_dd_hh:mm:ss");
	strFilterCondition +=	"', 'YYYY_MM_DD_HH24:MI:SS')";
	strFilterCondition +=	GEXDB_PLUGIN_DELIMITER_FROMTO;
	strFilterCondition +=	"to_date('";
	strFilterCondition +=	clDateTimeTo.toString("yyyy_mm_dd_hh:mm:ss");
	strFilterCondition +=	"', 'YYYY_MM_DD_HH24:MI:SS')";

	return Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, strFilterCondition, true);
}

//////////////////////////////////////////////////////////////////////
// Return all valid Data files (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::QueryDataFiles(GexDbPlugin_Filter & cFilters, const QString & strTestlist, GexDbPlugin_DataFileList & cMatchingFiles, const QString & strDatabasePhysicalPath, const QString & strLocalDir, bool *pbFilesCreatedInFinalLocation, GexDbPlugin_Base::StatsSource eStatsSource)
{
	// Compute query date constraints
	Query_ComputeDateConstraints(cFilters);

	// Check testingstage to query
	m_strTestingStage = cFilters.strDataTypeQuery;
	if(m_strTestingStage == GEXDB_PLUGIN_DIALOGSEMI_WTEST)
	{
		m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
		m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
		return QueryDataFiles_Wt(cFilters, strTestlist, cMatchingFiles, strLocalDir);
	}
	if(m_strTestingStage == GEXDB_PLUGIN_DIALOGSEMI_FTEST)
	{
		m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
		m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
		return QueryDataFiles_Ft(cFilters, strTestlist, cMatchingFiles, strLocalDir);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// Return all valid values for a field (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::QueryField(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bClearQueryFirst/*=true*/,bool bDistinct/*=true*/)
{
	if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_DIALOGSEMI_WTEST)
	{
		m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
		m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
		return GexDbPlugin_Base::QueryField(cFilters, cMatchingValues, bClearQueryFirst, bDistinct);
	}
	if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_DIALOGSEMI_FTEST)
	{
		m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
		m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
		return GexDbPlugin_Base::QueryField(cFilters, cMatchingValues, bClearQueryFirst, bDistinct);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// Return all tests (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::QueryTestlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues, bool bParametricOnly)
{
	if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_DIALOGSEMI_WTEST)
	{
		m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
		m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
		return GexDbPlugin_Base::QueryTestlist(cFilters, cMatchingValues, bParametricOnly);
	}
	if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_DIALOGSEMI_FTEST)
	{
		m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
		m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
		return GexDbPlugin_Base::QueryTestlist(cFilters, cMatchingValues, bParametricOnly);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// Return mapped filter labels
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Dialogsemi::GetLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices)
{
	strlLabelFilterChoices.clear();

	if(strDataTypeQuery == GEXDB_PLUGIN_DIALOGSEMI_WTEST)
		strlLabelFilterChoices = m_strlLabelFilterChoices_Wt;
	if(strDataTypeQuery == GEXDB_PLUGIN_DIALOGSEMI_FTEST)
		strlLabelFilterChoices = m_strlLabelFilterChoices_Ft;
}
