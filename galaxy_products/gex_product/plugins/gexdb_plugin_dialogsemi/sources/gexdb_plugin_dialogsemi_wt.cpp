// gexdb_plugin_dialogsemi_wt.cpp: implementation of the Wafer sort related members of the 
// GexDbPlugin_Dialogsemi class.
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

// Standard includes

// Qt includes
#include <QMessageBox>
#include <QSqlError>

// Galaxy modules includes
#include <cstdf.h>
#include <cstdfparse_v4.h>
#include <gqtl_sysutils.h>



//////////////////////////////////////////////////////////////////////
// Write STDF MIR record for specified Wafer
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteMIR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo)
{
	CStdf_MIR_V4 clMIR;

	// Set MIR fields
	clMIR.SetSETUP_T(clWaferInfo.m_lStartDate);
	clMIR.SetSTART_T(clWaferInfo.m_lStartDate);
	clMIR.SetSTAT_NUM(1);
	clMIR.SetMODE_COD('P');
	clMIR.SetRTST_COD(' ');
	clMIR.SetPROT_COD(' ');
	clMIR.SetBURN_TIM(65535);
	clMIR.SetCMOD_COD(' ');
	clMIR.SetLOT_ID(clWaferInfo.m_strBatchNumber.latin1());
	clMIR.SetPART_TYP(clWaferInfo.m_strDeviceID.latin1());
	clMIR.SetNODE_NAM(clWaferInfo.m_strTestSys.latin1());
	clMIR.SetTSTR_TYP("????");
	clMIR.SetJOB_NAM(clWaferInfo.m_strProgName);
	clMIR.SetJOB_REV(clWaferInfo.m_strProgRevision);

	// Write record
	return clStdfParse.WriteRecord(&clMIR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF MRR record for specified Wafer
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteMRR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo)
{
	CStdf_MRR_V4 clMRR;

	// Set MRR fields
	clMRR.SetFINISH_T(clWaferInfo.m_lEndDate);
	clMRR.SetDISP_COD(' ');

	// Write record
	return clStdfParse.WriteRecord(&clMRR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF WIR record for specified Wafer
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteWIR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo)
{
	CStdf_WIR_V4 clWIR;

	// Set WIR fields
	clWIR.SetHEAD_NUM(0);
	clWIR.SetSITE_GRP(0);
	clWIR.SetSTART_T(clWaferInfo.m_lStartDate);
	clWIR.SetWAFER_ID(clWaferInfo.m_strWaferID);

	// Write record
	return clStdfParse.WriteRecord(&clWIR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF WRR record for specified Wafer
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteWRR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo)
{
	CStdf_WRR_V4 clWRR;

	// Set WRR fields
	clWRR.SetHEAD_NUM(0);
	clWRR.SetSITE_GRP(0);
	clWRR.SetFINISH_T(clWaferInfo.m_lEndDate);
	clWRR.SetPART_CNT(clWaferInfo.m_nTotalTested);
	clWRR.SetRTST_CNT(4294967295);
	clWRR.SetABRT_CNT(4294967295);
	clWRR.SetGOOD_CNT(clWaferInfo.m_nTotalPassed);

	// Write record
	return clStdfParse.WriteRecord(&clWRR);
}

//////////////////////////////////////////////////////////////////////
// Write all test results for specified Wafer_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteTestResults(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo, const QString & strTestlist)
{
	QString								strQuery, strCondition;
	QSqlQuery							clQuery(QString::null, m_pclDatabaseConnector->m_sqlDatabase);
	int									nCurrentX=-1, nCurrentY=-1, nX, nY;
	double								lfTestResult;
	GexDbPlugin_RunInfo					*pCurrentPart;
	CStdf_PTR_V4						clPTR;
	unsigned int						uiNbRuns=0;
	bool								bStatus;
	QMap<Q_LLONG, GexDbPlugin_TestInfo>::Iterator itTest;
	Q_LLONG								lTestID;
	QTime								tGlobal, tQuery, tIteration;
	unsigned int						uiGlobalTime=0, uiQueryTime=0, uiIterationTime=0, uiNbTestResults=0; 

	if(m_bProfilerON)
		// For performance measurements
		tGlobal.start();

	if(!strTestlist.isEmpty())
	{
		// Empty query lists
		Query_Empty();

		// Query all test results
		m_strlQuery_Fields.append("Field|waf_test_maps.waf_test_info_id");
		m_strlQuery_Fields.append("Field|waf_test_maps.x_coord");
		m_strlQuery_Fields.append("Field|waf_test_maps.y_coord");
		m_strlQuery_Fields.append("Field|waf_test_maps.testvalue");
		strCondition = "waf_test_maps.waf_prober_info_id|Numeric|" + QString::number(clWaferInfo.m_lWaferInfoID);
		m_strlQuery_ValueConditions.append(strCondition);

		// Narrow on tests specified in testlist (testlist format is "10,20-30,40-50,65")
		if(strTestlist != "*")
			Query_AddTestlistCondition(strTestlist);

		// add sort instruction
		m_strlQuery_OrderFields.append("waf_test_maps.x_coord");
		m_strlQuery_OrderFields.append("waf_test_maps.y_coord");

		// Build query
		Query_BuildSqlString(strQuery, false);

		if(m_bProfilerON)
			tQuery.start();

        QApplication::setOverrideCursor(Qt::WaitCursor);
		clQuery.setForwardOnly(true);
		bStatus = clQuery.exec(strQuery);
        QApplication::restoreOverrideCursor();
		if(!bStatus)
		{
			GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQuery.lastError().text().latin1());
			return false;
		}

		if(m_bProfilerON)
		{
			uiQueryTime = tQuery.elapsed();
			tIteration.start();		// Measure iteration time
		}

		// Iterate through test results
		while(clQuery.next())
		{
			// Retrieve columns from row
			lTestID = clQuery.value(0).toLongLong();
			nX = clQuery.value(1).toInt();
			nY = clQuery.value(2).toInt();
			lfTestResult = clQuery.value(3).toDouble();

			if(m_bProfilerON)
			{
				uiIterationTime += tIteration.elapsed();
				uiNbTestResults++;
			}

			m_uiTotalTestResults++;

			// Any PIR/PRR to write?
			if(nCurrentX == -1)
			{
				uiNbRuns++;
				m_uiTotalRuns++;
				nCurrentX = nX;
				nCurrentY = nY;
				pCurrentPart = m_pPartInfoMatrix + ((nY + m_nWaferOffsetY)*m_nWaferNbColumns + (nX + m_nWaferOffsetX));
				WritePIR(clStdfParse, pCurrentPart->m_nSiteNb);
				pCurrentPart->m_bWrittenToStdf = true;

				// Update query progress dialog
				m_pQueryProgressDlg->UpdateTestCounter(uiNbRuns, m_uiTotalTestResults);
			}

			else if((nCurrentX != nX) || (nCurrentY !=  nY))
			{
				WritePRR(clStdfParse, pCurrentPart->m_nSiteNb, pCurrentPart->m_nSoftBin, pCurrentPart->m_nX, pCurrentPart->m_nY);
				uiNbRuns++;
				m_uiTotalRuns++;
				nCurrentX = nX;
				nCurrentY = nY;
				pCurrentPart = m_pPartInfoMatrix + ((nY + m_nWaferOffsetY)*m_nWaferNbColumns + (nX + m_nWaferOffsetX));
				WritePIR(clStdfParse, pCurrentPart->m_nSiteNb);
				pCurrentPart->m_bWrittenToStdf = true;

				// Update query progress dialog
				m_pQueryProgressDlg->UpdateTestCounter(uiNbRuns, m_uiTotalTestResults);
			}

			// Write PTR record
			itTest = m_mapTestlist.find(lTestID);
			if(itTest != m_mapTestlist.end())
			{
				clPTR.SetRESULT(lfTestResult);
				clPTR.SetTEST_NUM((*itTest).m_uiTestNumber);
				clPTR.SetHEAD_NUM(0);
				clPTR.SetSITE_NUM(pCurrentPart->m_nSiteNb);
				clPTR.SetTEST_FLG(0);
				clPTR.SetPARM_FLG(0);
			}
			clStdfParse.WriteRecord(&clPTR);

			if(m_bProfilerON)
				tIteration.start();
		}

		// Write last PRR
		if(nCurrentX != -1)
			WritePRR(clStdfParse, pCurrentPart->m_nSiteNb, pCurrentPart->m_nSoftBin, pCurrentPart->m_nX, pCurrentPart->m_nY);
	}

	// Write parts with no test executed
	if(uiNbRuns < m_uiWaferNbParts)
	{
		int nMatrixIndex;
		int nMatrixSize = m_nWaferNbColumns*m_nWaferNbRows;

		for(nMatrixIndex=0; nMatrixIndex<nMatrixSize; nMatrixIndex++)
		{
			pCurrentPart = m_pPartInfoMatrix + nMatrixIndex;
			if((pCurrentPart->m_nSoftBin != -1) && (pCurrentPart->m_bWrittenToStdf == false))
			{
				uiNbRuns++;
				m_uiTotalRuns++;
				WritePIR(clStdfParse, pCurrentPart->m_nSiteNb);
				WritePRR(clStdfParse, pCurrentPart->m_nSiteNb, pCurrentPart->m_nSoftBin, pCurrentPart->m_nX, pCurrentPart->m_nY);

				// Update query progress dialog
				m_pQueryProgressDlg->UpdateTestCounter(uiNbRuns, m_uiTotalTestResults);
			}
		}
	}

	// Update query progress dialog (force data update)
	m_pQueryProgressDlg->UpdateTestCounter(uiNbRuns, m_uiTotalTestResults, true);

	if(m_bProfilerON)
	{
		uiGlobalTime = tGlobal.elapsed();

		if(uiGlobalTime >= 10000)
			m_strDebugString.sprintf("DB query = %d ms\nQuery iteration = %d seconds\nDataset creation = %d seconds\nNb of runs = %d\nNb of test results = %d", uiQueryTime, uiIterationTime/1000, uiGlobalTime/1000, uiNbRuns, uiNbTestResults);
		else
			m_strDebugString.sprintf("DB query = %d ms\nQuery iteration = %d ms\nDataset creation = %d ms\nNb of runs = %d\nNb of test results = %d", uiQueryTime, uiIterationTime, uiGlobalTime, uiNbRuns, uiNbTestResults);
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Init different maps: array with part results, binning map,... for specified Wafer_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::InitMaps(GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo)
{
	QString										strQuery, strCondition;
	QSqlQuery									clQuery(QString::null, m_pclDatabaseConnector->m_sqlDatabase);
	int											nMinX, nMaxX, nMinY, nMaxY;
	int											nX, nY, nMatrixSize, nMatrixOffset;
	int											nBinning, nSiteNb;
	QMap<int, GexDbPlugin_BinInfo>				mapBinnings;
	QMap<int, GexDbPlugin_BinInfo>::Iterator	itBinning;

	// Delete matrix if allocated
	if(m_pPartInfoMatrix)
	{
		delete [] m_pPartInfoMatrix;
		m_pPartInfoMatrix = NULL;
	}

	// Clear maps
	m_mapSites.clear();
	m_uiWaferNbParts = 0;

	// Initialize local binning map
	Query_Empty();
	m_strlQuery_Fields.append("Field|waf_prober_info_bin.bin_no");
	m_strlQuery_Fields.append("Field|waf_prober_info_bin.bin_name");
	m_strlQuery_Fields.append("Field|waf_prober_info_bin.bin_cat");
	strCondition = "waf_prober_info_bin.waf_prober_info_id|Numeric|" + QString::number(clWaferInfo.m_lWaferInfoID);
	m_strlQuery_ValueConditions.append(strCondition);
	Query_BuildSqlString(strQuery, false);

	clQuery.setForwardOnly(true);
	if(!clQuery.exec(strQuery))
	{
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQuery.lastError().text().latin1());
		return false;
	}

	// Iterate through binnings
	while(clQuery.next())
	{
		nBinning = clQuery.value(0).toInt();
		mapBinnings[nBinning].m_nBinCount = 0;
		mapBinnings[nBinning].m_cBinCat = clQuery.value(2).toString()[0];
		mapBinnings[nBinning].m_strBinName = clQuery.value(1).toString();
	}
	
	// Create part results matrix
	Query_Empty();
	m_strlQuery_Fields.append("Function|waf_test_fail_map.x_coord|MIN");
	m_strlQuery_Fields.append("Function|waf_test_fail_map.x_coord|MAX");
	m_strlQuery_Fields.append("Function|waf_test_fail_map.y_coord|MIN");
	m_strlQuery_Fields.append("Function|waf_test_fail_map.y_coord|MAX");
	strCondition = "waf_test_fail_map.waf_prober_info_id|Numeric|" + QString::number(clWaferInfo.m_lWaferInfoID);
	m_strlQuery_ValueConditions.append(strCondition);
	Query_BuildSqlString(strQuery, false);

	clQuery.setForwardOnly(true);
	if((!clQuery.exec(strQuery)) || (!clQuery.next()))
	{
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQuery.lastError().text().latin1());
		return false;
	}

	nMinX = clQuery.value(0).toInt();
	nMaxX = clQuery.value(1).toInt();
	nMinY = clQuery.value(2).toInt();
	nMaxY = clQuery.value(3).toInt();

	// Set Wafer dimension variables
	m_nWaferNbRows = (nMaxY-nMinY)+1;
	m_nWaferNbColumns = (nMaxX - nMinX)+1;
	m_nWaferOffsetY = -nMinY;
	m_nWaferOffsetX = -nMinX;

	// Allocate XY matrix
	nMatrixSize = m_nWaferNbColumns*m_nWaferNbRows;
	m_pPartInfoMatrix = new GexDbPlugin_RunInfo[nMatrixSize];
	if(!m_pPartInfoMatrix)
	{
		return false;
	}
	
	// Init matrix
	for(nMatrixOffset=0; nMatrixOffset<nMatrixSize; nMatrixOffset++)
	{
		m_pPartInfoMatrix[nMatrixOffset].m_nRunID = 0;
		m_pPartInfoMatrix[nMatrixOffset].m_nX = -32768;
		m_pPartInfoMatrix[nMatrixOffset].m_nY = -32768;
		m_pPartInfoMatrix[nMatrixOffset].m_nSiteNb = -1;
		m_pPartInfoMatrix[nMatrixOffset].m_nSoftBin = -1;
		m_pPartInfoMatrix[nMatrixOffset].m_nHardBin = -1;
		m_pPartInfoMatrix[nMatrixOffset].m_bWrittenToStdf = false;
	}

	// Query part information
	Query_Empty();
	m_strlQuery_Fields.append("Field|waf_test_fail_map.x_coord");
	m_strlQuery_Fields.append("Field|waf_test_fail_map.y_coord");
	m_strlQuery_Fields.append("Field|waf_test_fail_map.site");
	m_strlQuery_Fields.append("Field|waf_test_fail_map.bin_no");
	strCondition = "waf_test_fail_map.waf_prober_info_id|Numeric|" + QString::number(clWaferInfo.m_lWaferInfoID);
	m_strlQuery_ValueConditions.append(strCondition);
	Query_BuildSqlString(strQuery, false);

	clQuery.setForwardOnly(true);
	if(!clQuery.exec(strQuery))
	{
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQuery.lastError().text().latin1());
		return false;
	}

	while(clQuery.next())
	{
		nX = clQuery.value(0).toInt();
		nY = clQuery.value(1).toInt();
		nSiteNb = clQuery.value(2).toInt();
		nBinning = clQuery.value(3).toInt();

		// Update XY part matrix
		m_uiWaferNbParts++;
		m_uiTotalRuns++;
		nMatrixOffset = (nY + m_nWaferOffsetY)*m_nWaferNbColumns + nX + m_nWaferOffsetX;
		m_pPartInfoMatrix[nMatrixOffset].m_nX = nX;
		m_pPartInfoMatrix[nMatrixOffset].m_nY = nY;
		m_pPartInfoMatrix[nMatrixOffset].m_nSiteNb = nSiteNb;
		m_pPartInfoMatrix[nMatrixOffset].m_nSoftBin = nBinning;

		// Update query progress dialog
		m_pQueryProgressDlg->UpdateRunCounter(m_uiTotalRuns);

		// Update Site map
		m_mapSites[nSiteNb].m_nPartCount++;
		if(clWaferInfo.m_nNumberOfSites > 1)
			m_mapSites[255].m_nPartCount++;

		// Update binning in maps
		itBinning = mapBinnings.find(nBinning);
		if(itBinning != mapBinnings.end())
		{
			if((*itBinning).m_cBinCat.upper() == 'P')
			{
				m_mapSites[nSiteNb].m_nGoodCount++;
				if(clWaferInfo.m_nNumberOfSites > 1)
					m_mapSites[255].m_nGoodCount++;
			}
			m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_cBinCat = (*itBinning).m_cBinCat;
			m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_strBinName = (*itBinning).m_strBinName;
			m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_nBinCount++;
			if(clWaferInfo.m_nNumberOfSites > 1)
			{
				m_mapSites[255].m_mapSoftBinnings[nBinning].m_cBinCat = (*itBinning).m_cBinCat;
				m_mapSites[255].m_mapSoftBinnings[nBinning].m_strBinName = (*itBinning).m_strBinName;
				m_mapSites[255].m_mapSoftBinnings[nBinning].m_nBinCount++;
			}
		}
		else
		{
			// If no BinInfo available, cosider Bin1=P, others=F
			if(nBinning == 1)
			{
				m_mapSites[nSiteNb].m_nGoodCount++;
				m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_cBinCat = 'P';
				if(clWaferInfo.m_nNumberOfSites > 1)
				{
					m_mapSites[255].m_nGoodCount++;
					m_mapSites[255].m_mapSoftBinnings[nBinning].m_cBinCat = 'P';
				}
			}
			else
			{
				m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_cBinCat = 'F';
				if(clWaferInfo.m_nNumberOfSites > 1)
					m_mapSites[255].m_mapSoftBinnings[nBinning].m_cBinCat = 'F';
			}
			m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_strBinName = "?";
			m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_nBinCount++;
			if(clWaferInfo.m_nNumberOfSites > 1)
			{
				m_mapSites[255].m_mapSoftBinnings[nBinning].m_strBinName = "?";
				m_mapSites[255].m_mapSoftBinnings[nBinning].m_nBinCount++;
			}
		}
	}
	
	// Update query progress dialog
	m_pQueryProgressDlg->UpdateRunCounter(m_uiTotalRuns, true);

	return true;
}

//////////////////////////////////////////////////////////////////////
// Create TestList for specified Wafer_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::CreateTestlist(GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo)
{
	QString		strQuery, strCondition;
	QSqlQuery	clQueryTests(QString::null, m_pclDatabaseConnector->m_sqlDatabase);

	// First clear the list
	m_mapTestlist.clear();

	// Query test info for specified wafer
	Query_Empty();
	m_strlQuery_Fields.append("Field|waf_test_info.waf_test_info_id");
	m_strlQuery_Fields.append("Field|waf_test_info.testnum");
	m_strlQuery_Fields.append("Field|waf_test_info.testname");
	m_strlQuery_Fields.append("Field|waf_test_info.unit");
	m_strlQuery_Fields.append("Field|waf_test_info.lolim");
	m_strlQuery_Fields.append("Field|waf_test_info.hilim");
	strCondition = "waf_test_info.waf_prober_info_id|Numeric|" + QString::number(clWaferInfo.m_lWaferInfoID);
	m_strlQuery_ValueConditions.append(strCondition);
	Query_BuildSqlString(strQuery, false);

	clQueryTests.setForwardOnly(true);
	if(!clQueryTests.exec(strQuery))
	{
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQueryTests.lastError().text().latin1());
		return false;
	}

	// Add all retrieved tests to testlist
	GexDbPlugin_TestInfo	clTestInfo;
	Q_LLONG					lTestInfoID;
	while(clQueryTests.next())
	{
		clTestInfo.Reset();
		lTestInfoID									= clQueryTests.value(0).toLongLong();
		clTestInfo.m_uiTestNumber					= clQueryTests.value(1).toUInt();
		clTestInfo.m_strTestName					= clQueryTests.value(2).toString();
		clTestInfo.m_pTestInfo_PTR					= new GexDbPlugin_TestInfo_PTR;
		clTestInfo.m_pTestInfo_PTR->m_strTestUnit	= clQueryTests.value(3).toString();
		clTestInfo.m_pTestInfo_PTR->m_fLL			= clQueryTests.value(4).toDouble();
		clTestInfo.m_pTestInfo_PTR->m_fHL			= clQueryTests.value(5).toDouble();
		m_mapTestlist[lTestInfoID]					= clTestInfo;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Write static test information
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteStaticTestInfo(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Wt_WaferInfo & clWaferInfo)
{
	CStdf_PTR_V4	clPTR;

	QMap<Q_LLONG, GexDbPlugin_TestInfo>::Iterator it;

	for(it = m_mapTestlist.begin(); it != m_mapTestlist.end(); it++)
	{
		// Set PTR fields
		clPTR.SetTEST_NUM((*it).m_uiTestNumber);
		clPTR.SetHEAD_NUM(0);
		clPTR.SetSITE_NUM(0);
		clPTR.SetTEST_FLG(0x10);		// Test not executed
		clPTR.SetPARM_FLG(0);
		clPTR.SetRESULT(0.0F);
		clPTR.SetTEST_TXT((*it).m_strTestName);
		clPTR.SetALARM_ID("");
		clPTR.SetOPT_FLAG(0);
		clPTR.SetRES_SCAL(0);
		clPTR.SetLLM_SCAL(0);
		clPTR.SetHLM_SCAL(0);
		clPTR.SetLO_LIMIT((*it).m_pTestInfo_PTR->m_fLL);
		clPTR.SetHI_LIMIT((*it).m_pTestInfo_PTR->m_fHL);
		clPTR.SetUNITS((*it).m_pTestInfo_PTR->m_strTestUnit);

		// Write record
		clStdfParse.WriteRecord(&clPTR);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Create a STDF file for specified Wafer_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::CreateStdfFile_Wt(const QSqlQuery & clQueryWafer, const QString & strTestlist, const QString & strLocalDir, QString & strStdfFileName)
{
	GexDbPlugin_Dialogsemi_Wt_WaferInfo	clWaferInfo;
	QDateTime							clDateTime;
	QString								strMessage;

	clWaferInfo.m_lWaferInfoID		= clQueryWafer.value(0).toLongLong();					// WAF_PROBER_INFO.WAF_PROBER_INFO_ID
	clWaferInfo.m_strDeviceID		= clQueryWafer.value(1).toString();						// WAF_PROBER_INFO.DEVICE_ID
	clWaferInfo.m_strBatchNumber	= clQueryWafer.value(2).toString();						// WAF_PROBER_INFO.BATCHNUMBER
	clWaferInfo.m_strWaferID		= clQueryWafer.value(3).toString();						// WAF_PROBER_INFO.WAFERID
	clWaferInfo.m_strTestSys		= clQueryWafer.value(4).toString();						// WAF_PROBER_INFO.TESTSYS
	clDateTime = QDateTime::fromString(clQueryWafer.value(5).toString(), Qt::ISODate);
	clWaferInfo.m_lStartDate		= clDateTime.toTime_t();								// WAF_PROBER_INFO.TEST_STARTDATE
	clDateTime = QDateTime::fromString(clQueryWafer.value(6).toString(), Qt::ISODate);
	clWaferInfo.m_lEndDate			= clDateTime.toTime_t();								// WAF_PROBER_INFO.TEST_ENDDATE
	clWaferInfo.m_nTotalTested		= clQueryWafer.value(7).toInt();						// WAF_PROBER_INFO.TOTAL_TESTED
	clWaferInfo.m_nTotalPassed		= clQueryWafer.value(8).toInt();						// WAF_PROBER_INFO.TOTAL_PASSED
	clWaferInfo.m_strTestMode		= clQueryWafer.value(9).toString();						// WAF_PROBER_INFO.TESTMODE
	clWaferInfo.m_strTemp			= clQueryWafer.value(10).toString();					// WAF_PROBER_INFO.TEMP
	clWaferInfo.m_strAcceptBin		= clQueryWafer.value(11).toString();					// WAF_PROBER_INFO.ACCEPTBIN
	clWaferInfo.m_nNumberOfSites	= clQueryWafer.value(12).toInt();						// WAF_PROBER_INFO.NUMBER_OF_SITES
	clWaferInfo.m_strLoadboard		= clQueryWafer.value(13).toString();					// WAF_PROBER_INFO.LOADBOARD
	clWaferInfo.m_strProbeCard		= clQueryWafer.value(14).toString();					// WAF_PROBER_INFO.PROBECARD
	clWaferInfo.m_strProgName		= clQueryWafer.value(15).toString();					// WAF_PROBER_INFO.PROGNAME
	clWaferInfo.m_strProgRevision	= clQueryWafer.value(16).toString();					// WAF_PROBER_INFO.PROG_REVISION
	clWaferInfo.m_strProgRevDate	= clQueryWafer.value(17).toString();					// WAF_PROBER_INFO.PROG_REVDATE
	clWaferInfo.m_strSiteCd			= clQueryWafer.value(18).toString();					// WAF_PROBER_INFO.SITE_CD
	clWaferInfo.m_strFlatCd			= clQueryWafer.value(19).toString();					// WAF_PROBER_INFO.FLAT_CD
	clWaferInfo.m_strHandlingsys	= clQueryWafer.value(20).toString();					// WAF_PROBER_INFO.HANDLINGSYS
	clWaferInfo.m_cValidFlag		= clQueryWafer.value(21).toChar().toLatin1();			// WAF_PROBER_INFO.VALID_FLAG
	clWaferInfo.m_strFacility		= clQueryWafer.value(22).toString();					// WAF_PROBER_INFO.FACILITY

	strStdfFileName.sprintf("%s_%s_%s_%s.stdf",	clWaferInfo.m_strDeviceID.latin1(), clWaferInfo.m_strTestSys.latin1(),
												clWaferInfo.m_strBatchNumber.latin1(), clWaferInfo.m_strWaferID.latin1());

	// Update query progress dialog
	m_pQueryProgressDlg->UpdateFileInfo(clWaferInfo.m_strDeviceID, clWaferInfo.m_strBatchNumber, "", clWaferInfo.m_strWaferID);

	// Create STDF file
	CGexSystemUtils	clSysUtils;
	QString			strStdfFullFileName = strLocalDir + strStdfFileName;
	clSysUtils.NormalizePath(strStdfFullFileName);

	// Create STDF records
	CStdfParse_V4 clStdfParse;
	
	// Open STDF file
    if(!clStdfParse.Open((char *)strStdfFullFileName.latin1(), STDF_WRITE))
	{
		return false;
	}

	// Init diferent maps: X,Y matrix with part results, binning map,...
	if(!InitMaps(clWaferInfo))
	{
		clStdfParse.Close();
		return false;
	}

	// Create testlist
	if(!CreateTestlist(clWaferInfo))
	{
		clStdfParse.Close();
		return false;
	}

	// Start progress bar for current file
	m_pQueryProgressDlg->StartFileProgress(m_uiWaferNbParts);
	
	// Write MIR
	if(!WriteMIR(clStdfParse, clWaferInfo))
	{
		clStdfParse.Close();
		return false;
	}

	// Write WIR
	if(!WriteWIR(clStdfParse, clWaferInfo))
	{
		clStdfParse.Close();
		return false;
	}

	// Write static test info
	WriteStaticTestInfo(clStdfParse, clWaferInfo);

	// Write test results
	if(!WriteTestResults(clStdfParse, clWaferInfo, strTestlist))
	{
		clStdfParse.Close();
		return false;
	}

	// Write WRR
	if(!WriteWRR(clStdfParse, clWaferInfo))
	{
		clStdfParse.Close();
		return false;
	}

	// Write summary records
	if(!WriteSummaryRecords(clStdfParse))
	{
		clStdfParse.Close();
		return false;
	}

	// Write MRR
	if(!WriteMRR(clStdfParse, clWaferInfo))
	{
		clStdfParse.Close();
		return false;
	}

	// Close STDF file
	clStdfParse.Close();

	// Stop progress bar for current file
	m_pQueryProgressDlg->StopFileProgress();
	
	return true;
}

///////////////////////////////////////////////////////////
// Query: construct query string for STDF files query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::ConstructStdfFilesQuery_Wt(GexDbPlugin_Filter &cFilters, QString & strQuery)
{
	QString strCondition;

	// Clear query string
	strQuery = "";

	// Construct query string:
	// SELECT <testnum, testname>
	// FROM <all required tables>
	// WHERE <link conditions> AND <value conditions>	(optional)
	Query_Empty();
	m_strlQuery_Fields.append("Field|waf_prober_info.waf_prober_info_id");
	m_strlQuery_Fields.append("Field|waf_prober_info.device_id");
	m_strlQuery_Fields.append("Field|waf_prober_info.batchnumber");
	m_strlQuery_Fields.append("Field|waf_prober_info.waferid");
	m_strlQuery_Fields.append("Field|waf_prober_info.testsys");
	m_strlQuery_Fields.append("Field|waf_prober_info.test_startdate");
	m_strlQuery_Fields.append("Field|waf_prober_info.test_enddate");
	m_strlQuery_Fields.append("Field|waf_prober_info.total_tested");
	m_strlQuery_Fields.append("Field|waf_prober_info.total_passed");
	m_strlQuery_Fields.append("Field|waf_prober_info.testmode");
	m_strlQuery_Fields.append("Field|waf_prober_info.temp");
	m_strlQuery_Fields.append("Field|waf_prober_info.acceptbin");
	m_strlQuery_Fields.append("Field|waf_prober_info.number_of_sites");
	m_strlQuery_Fields.append("Field|waf_prober_info.loadboard");
	m_strlQuery_Fields.append("Field|waf_prober_info.probecard");
	m_strlQuery_Fields.append("Field|waf_prober_info.progname");
	m_strlQuery_Fields.append("Field|waf_prober_info.prog_revision");
	m_strlQuery_Fields.append("Field|waf_prober_info.prog_revdate");
	m_strlQuery_Fields.append("Field|waf_prober_info.site_cd");
	m_strlQuery_Fields.append("Field|waf_prober_info.flat_cd");
	m_strlQuery_Fields.append("Field|waf_prober_info.handlingsys");
	m_strlQuery_Fields.append("Field|waf_prober_info.valid_flag");
	m_strlQuery_Fields.append("Field|waf_prober_info.facility");
	strCondition = "waf_prober_info.valid_flag|String|Y";
	m_strlQuery_ValueConditions.append(strCondition);
	// Set filters
	Query_AddFilters(cFilters);

	// Add time period condition
	Query_AddTimePeriodCondition(cFilters);

	// Construct query from table and conditions
	Query_BuildSqlString(strQuery, false);

	return true;
}

//////////////////////////////////////////////////////////////////////
// Return all valid Data files (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::QueryDataFiles_Wt(GexDbPlugin_Filter & cFilters, const QString & strTestlist, GexDbPlugin_DataFileList & cMatchingFiles, const QString & strLocalDir)
{
	QTime	tTimer;
	QString	strMessage;

	// Clear returned list of matching files
	cMatchingFiles.Clear();

	// Check database connection
	if(!ConnectToCorporateDb())
		return false;

	// Get nb of files to generate
	int nNbFiles = GetNbOfFilesToGenerate(cFilters);
	if(nNbFiles == -1)
		return false;

	// Construct SQL query
	QString strQuery;
	if(!ConstructStdfFilesQuery_Wt(cFilters, strQuery))
		return false;

	// Execute query
	QSqlQuery	clQuery(QString::null, m_pclDatabaseConnector->m_sqlDatabase);
	clQuery.setForwardOnly(true);
	if(!clQuery.exec(strQuery))
	{
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQuery.lastError().text().latin1());
		return false;
	}

	// Show query progress dialog
	m_pQueryProgressDlg->Start(nNbFiles);
	m_uiTotalRuns = 0;
	m_uiTotalTestResults = 0;
	
	// Create 1 STDF file for each wafer retrieved
	GexDbPlugin_DataFile	*pStdfFile;
	QString					strStdfFilename;
	while(clQuery.next())
	{
		tTimer.start();
		if(CreateStdfFile_Wt(clQuery, strTestlist, strLocalDir, strStdfFilename))
		{
			// STDF file successfully created
			pStdfFile = new GexDbPlugin_DataFile;
			pStdfFile->m_strFileName = strStdfFilename;
			pStdfFile->m_strFilePath = strLocalDir;
			pStdfFile->m_bRemoteFile = false;

			// Append to list
			cMatchingFiles.append(pStdfFile);
		}
		if(m_bProfilerON)
		{
			strMessage.sprintf("STDF file %s created in %d seconds:\n\n%s", strStdfFilename.latin1(), tTimer.elapsed()/1000, m_strDebugString.latin1());
			QMessageBox::information(NULL, "SQL debug", strMessage, QMessageBox::Ok);
		}
	}

	// Hide Query progress dialog
	m_pQueryProgressDlg->Stop();

	return true;
}

