// gexdb_plugin_dialogsemi_ft.cpp: implementation of the final test related members of the 
// GexDbPlugin_Dialogsemi_Ft class.
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
#include <QSqlError>
#include <QMessageBox>

// Galaxy modules includes
#include <cstdf.h>
#include <cstdfparse_v4.h>
#include <gqtl_sysutils.h>
#include <QApplication>



//////////////////////////////////////////////////////////////////////
// Write STDF MIR record for specified Job
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteMIR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo)
{
	CStdf_MIR_V4 clMIR;

	// Set MIR fields
	clMIR.SetSETUP_T(clJobInfo.m_lStartDate);
	clMIR.SetSTART_T(clJobInfo.m_lEndDate);
	clMIR.SetSTAT_NUM(1);
	clMIR.SetMODE_COD('P');
	clMIR.SetRTST_COD(' ');
	clMIR.SetPROT_COD(' ');
	clMIR.SetBURN_TIM(65535);
	clMIR.SetCMOD_COD(' ');
	clMIR.SetLOT_ID(clJobInfo.m_strBatchNumber.latin1());
	clMIR.SetPART_TYP(clJobInfo.m_strDeviceID.latin1());
	clMIR.SetNODE_NAM(clJobInfo.m_strTestSys.latin1());
	clMIR.SetTSTR_TYP("????");
	clMIR.SetJOB_NAM(clJobInfo.m_strProgName);
	clMIR.SetJOB_REV(clJobInfo.m_strProgRevision);

	// Write record
	return clStdfParse.WriteRecord(&clMIR);
}

//////////////////////////////////////////////////////////////////////
// Write STDF MRR record for specified Job
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteMRR(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo)
{
	CStdf_MRR_V4 clMRR;

	// Set MRR fields
	clMRR.SetFINISH_T(clJobInfo.m_lEndDate);
	clMRR.SetDISP_COD(' ');

	// Write record
	return clStdfParse.WriteRecord(&clMRR);
}

//////////////////////////////////////////////////////////////////////
// Write all test results for specified Job_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteTestResults(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo, const QString & strTestlist)
{
	QString								strQuery, strCondition;
	QSqlQuery							clQuery(QString::null, m_pclDatabaseConnector->m_sqlDatabase);
	int									nCurrentSerNo=-1, nSerNo;
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
		m_strlQuery_Fields.append("Field|ft_test_results_l.ft_test_info_id");
		m_strlQuery_Fields.append("Field|ft_test_results_l.ser_no");
		m_strlQuery_Fields.append("Field|ft_test_results_l.testvalue");
		strCondition = "ft_test_results_l.ft_job_info_id|Numeric|" + QString::number(clJobInfo.m_lJobInfoID);
		m_strlQuery_ValueConditions.append(strCondition);

		// Narrow on tests specified in testlist (testlist format is "10,20-30,40-50,65")
		// SQL syntax: "AND ((testnum IN (100,101,102)) OR (testnum BETWEEN 160 AND 165))"
		if(strTestlist != "*")
			Query_AddTestlistCondition(strTestlist);

		//add sort instruction
		m_strlQuery_OrderFields.append("ft_test_results_l.ser_no");

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
			nSerNo = clQuery.value(1).toInt();
			lfTestResult = clQuery.value(2).toDouble();

			if(m_bProfilerON)
			{
				uiIterationTime += tIteration.elapsed();
				uiNbTestResults++;
			}

			m_uiTotalTestResults++;

			// Any PIR/PRR to write?
			if(nCurrentSerNo == -1)
			{
				uiNbRuns++;
				nCurrentSerNo = nSerNo;
				pCurrentPart = m_pPartInfoArray + nSerNo + m_nRunOffset;
				WritePIR(clStdfParse, pCurrentPart->m_nSiteNb);
				pCurrentPart->m_bWrittenToStdf = true;

				// Update query progress dialog
				m_pQueryProgressDlg->UpdateTestCounter(uiNbRuns, m_uiTotalTestResults);
			}

			else if(nCurrentSerNo !=  nSerNo)
			{
				WritePRR(clStdfParse, pCurrentPart->m_nSiteNb, pCurrentPart->m_nSoftBin, -32768, -32768, pCurrentPart->m_nRunID);
				uiNbRuns++;
				nCurrentSerNo = nSerNo;
				pCurrentPart = m_pPartInfoArray + nSerNo + m_nRunOffset;
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
		if(nCurrentSerNo != -1)
			WritePRR(clStdfParse, pCurrentPart->m_nSiteNb, pCurrentPart->m_nSoftBin, -32768, -32768, pCurrentPart->m_nRunID);
	}

	// Write parts with no test executed
	if(uiNbRuns < m_uiLotNbParts)
	{
		int nArrayIndex;

		for(nArrayIndex=0; nArrayIndex<m_nNbRuns; nArrayIndex++)
		{
			pCurrentPart = m_pPartInfoArray + nArrayIndex;
			if((pCurrentPart->m_nSoftBin != -1) && (pCurrentPart->m_bWrittenToStdf == false))
			{
				uiNbRuns++;
				m_uiTotalRuns++;
				WritePIR(clStdfParse, pCurrentPart->m_nSiteNb);
				WritePRR(clStdfParse, pCurrentPart->m_nSiteNb, pCurrentPart->m_nSoftBin, -32768, -32768, pCurrentPart->m_nRunID);

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
// Init different maps: array with part results, binning map,... for specified Job_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::InitMaps(GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo)
{
	QString										strQuery, strCondition;
	QSqlQuery									clQuery(QString::null, m_pclDatabaseConnector->m_sqlDatabase);
	int											nMin, nMax;
	int											nArrayOffset;
	int											nBinning, nSerialNb, nSiteNb;
	QMap<int, GexDbPlugin_BinInfo>				mapBinnings;
	QMap<int, GexDbPlugin_BinInfo>::Iterator	itBinning;

	// Delete part array if allocated
	if(m_pPartInfoArray)
	{
		delete [] m_pPartInfoArray;
		m_pPartInfoArray = NULL;
	}

	// Clear maps
	m_mapSites.clear();
	m_uiLotNbParts = 0;

	// Initialize local binning map
	Query_Empty();
	m_strlQuery_Fields.append("Field|ft_info_bin.bin_no");
	m_strlQuery_Fields.append("Field|ft_info_bin.bin_name");
	m_strlQuery_Fields.append("Field|ft_info_bin.bin_cat");
	strCondition = "ft_info_bin.ft_job_info_id|Numeric|" + QString::number(clJobInfo.m_lJobInfoID);
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
	
	// Create part results array
	Query_Empty();
	m_strlQuery_Fields.append("Function|ft_test_fail.ser_no|MIN");
	m_strlQuery_Fields.append("Function|ft_test_fail.ser_no|MAX");
	strCondition = "ft_test_fail.ft_job_info_id|Numeric|" + QString::number(clJobInfo.m_lJobInfoID);
	m_strlQuery_ValueConditions.append(strCondition);
	Query_BuildSqlString(strQuery, false);

	clQuery.setForwardOnly(true);
	if((!clQuery.exec(strQuery)) || (!clQuery.next()))
	{
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQuery.lastError().text().latin1());
		return false;
	}

	nMin = clQuery.value(0).toInt();
	nMax = clQuery.value(1).toInt();

	// Allocate array
	m_nNbRuns = (nMax-nMin)+1;
	m_nRunOffset = -nMin;
	m_pPartInfoArray = new GexDbPlugin_RunInfo[m_nNbRuns];
	if(!m_pPartInfoArray)
	{
		return false;
	}
	
	// Init array
	for(nArrayOffset=0; nArrayOffset<m_nNbRuns; nArrayOffset++)
	{
		m_pPartInfoArray[nArrayOffset].m_nRunID = 0;
		m_pPartInfoArray[nArrayOffset].m_nX = -32768;
		m_pPartInfoArray[nArrayOffset].m_nY = -32768;
		m_pPartInfoArray[nArrayOffset].m_nSiteNb = -1;
		m_pPartInfoArray[nArrayOffset].m_nSoftBin = -1;
		m_pPartInfoArray[nArrayOffset].m_nHardBin = -1;
		m_pPartInfoArray[nArrayOffset].m_bWrittenToStdf = false;
	}

	// Query part information
	Query_Empty();
	m_strlQuery_Fields.append("Field|ft_test_fail.ser_no");
	m_strlQuery_Fields.append("Field|ft_test_fail.site");
	m_strlQuery_Fields.append("Field|ft_test_fail.bin_no");
	strCondition = "ft_test_fail.ft_job_info_id|Numeric|" + QString::number(clJobInfo.m_lJobInfoID);
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
		nSerialNb = clQuery.value(0).toInt();
		nSiteNb = clQuery.value(1).toInt();
		nBinning = clQuery.value(2).toInt();

		// Update part array
		m_uiLotNbParts++;
		m_uiTotalRuns++;
		nArrayOffset = nSerialNb + m_nRunOffset;
		m_pPartInfoArray[nArrayOffset].m_nRunID = nSerialNb;
		m_pPartInfoArray[nArrayOffset].m_nSiteNb = nSiteNb;
		m_pPartInfoArray[nArrayOffset].m_nSoftBin = nBinning;

		// Update query progress dialog
		m_pQueryProgressDlg->UpdateRunCounter(m_uiTotalRuns);

		// Update Site map
		itBinning = mapBinnings.find(nBinning);
		m_mapSites[nSiteNb].m_nPartCount++;
		if(clJobInfo.m_nNumberOfSites > 1)
			m_mapSites[255].m_nPartCount++;
		if(itBinning != mapBinnings.end())
		{
			if((*itBinning).m_cBinCat.upper() == 'P')
			{
				m_mapSites[nSiteNb].m_nGoodCount++;
				if(clJobInfo.m_nNumberOfSites > 1)
					m_mapSites[255].m_nGoodCount++;
			}
			m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_cBinCat = (*itBinning).m_cBinCat;
			m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_strBinName = (*itBinning).m_strBinName;
			m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_nBinCount++;
			if(clJobInfo.m_nNumberOfSites > 1)
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
				if(clJobInfo.m_nNumberOfSites > 1)
				{
					m_mapSites[255].m_nGoodCount++;
					m_mapSites[255].m_mapSoftBinnings[nBinning].m_cBinCat = 'P';
				}
			}
			else
			{
				m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_cBinCat = 'F';
				if(clJobInfo.m_nNumberOfSites > 1)
					m_mapSites[255].m_mapSoftBinnings[nBinning].m_cBinCat = 'F';
			}
			m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_strBinName = "?";
			m_mapSites[nSiteNb].m_mapSoftBinnings[nBinning].m_nBinCount++;
			if(clJobInfo.m_nNumberOfSites > 1)
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
// Create TestList for specified FtJob_id
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::CreateTestlist(GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo)
{
	QString		strQuery, strCondition;
	QSqlQuery	clQueryTests(QString::null, m_pclDatabaseConnector->m_sqlDatabase);

	// First clear the list
	m_mapTestlist.clear();

	// Query test info for specified job
	Query_Empty();
	m_strlQuery_Fields.append("Field|ft_test_info.ft_test_info_id");
	m_strlQuery_Fields.append("Field|ft_test_info.testnum");
	m_strlQuery_Fields.append("Field|ft_test_info.testname");
	m_strlQuery_Fields.append("Field|ft_test_info.unit");
	m_strlQuery_Fields.append("Field|ft_test_info.lolim");
	m_strlQuery_Fields.append("Field|ft_test_info.hilim");
	strCondition = "ft_test_info.ft_job_info_id|Numeric|" + QString::number(clJobInfo.m_lJobInfoID);
	m_strlQuery_ValueConditions.append(strCondition);
	Query_BuildSqlString(strQuery, false);

	clQueryTests.setForwardOnly(true);
	if(!clQueryTests.exec(strQuery))
	{
		GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.latin1(), clQueryTests.lastError().text().latin1());
		return false;
	}

	// Add all retrieved tests to testlist
	GexDbPlugin_TestInfo		clTestInfo;
	Q_LLONG						lTestInfoID;
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
		m_mapTestlist[lTestInfoID]	= clTestInfo;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Write static test information
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::WriteStaticTestInfo(CStdfParse_V4 & clStdfParse, GexDbPlugin_Dialogsemi_Ft_JobInfo & clJobInfo)
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
// Create a STDF file for specified Job ID
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Dialogsemi::CreateStdfFile_Ft(const QSqlQuery & clQueryJob, const QString & strTestlist, const QString & strLocalDir, QString & strStdfFileName)
{
	GexDbPlugin_Dialogsemi_Ft_JobInfo	clJobInfo;
	QDateTime							clDateTime;
	QString								strMessage;

	clJobInfo.m_lJobInfoID		= clQueryJob.value(0).toLongLong();						// FT_JOB_INFO.FT_JOB_INFO_ID
	clJobInfo.m_strDeviceID		= clQueryJob.value(1).toString();						// FT_JOB_INFO.DEVICE_ID
	clJobInfo.m_strBatchNumber	= clQueryJob.value(2).toString();						// FT_JOB_INFO.BATCHNUMBER
	clJobInfo.m_strDateCode		= clQueryJob.value(3).toString();						// FT_JOB_INFO.DATECODE
	clJobInfo.m_strTestSys		= clQueryJob.value(4).toString();						// FT_JOB_INFO.TESTSYS
	clDateTime = QDateTime::fromString(clQueryJob.value(5).toString(), Qt::ISODate);
	clJobInfo.m_lStartDate		= clDateTime.toTime_t();								// FT_JOB_INFO.TEST_STARTDATE
	clDateTime = QDateTime::fromString(clQueryJob.value(6).toString(), Qt::ISODate);
	clJobInfo.m_lEndDate		= clDateTime.toTime_t();								// FT_JOB_INFO.TEST_ENDDATE
	clJobInfo.m_nTotalTested	= clQueryJob.value(7).toInt();							// FT_JOB_INFO.TOTAL_TESTED
	clJobInfo.m_nTotalPassed	= clQueryJob.value(8).toInt();							// FT_JOB_INFO.TOTAL_PASSED
	clJobInfo.m_strTestMode		= clQueryJob.value(9).toString();						// FT_JOB_INFO.TESTMODE
	clJobInfo.m_strTemp			= clQueryJob.value(10).toString();						// FT_JOB_INFO.TEMP
	clJobInfo.m_nNumberOfSites	= clQueryJob.value(11).toInt();							// FT_JOB_INFO.NUMBER_OF_SITES
	clJobInfo.m_strLoadboard	= clQueryJob.value(12).toString();						// FT_JOB_INFO.LOADBOARD
	clJobInfo.m_strProgName		= clQueryJob.value(13).toString();						// FT_JOB_INFO.PROGNAME
	clJobInfo.m_strHandingSys	= clQueryJob.value(14).toString();						// FT_JOB_INFO.HANDLINGSYS
	clJobInfo.m_strProgRevision	= clQueryJob.value(15).toString();						// FT_JOB_INFO.PROG_REVISION
	clJobInfo.m_strProgRevDate	= clQueryJob.value(16).toString();						// FT_JOB_INFO.PROG_REVDATE
	clJobInfo.m_strAcceptBin	= clQueryJob.value(17).toString();						// FT_JOB_INFO.ACCEPTBIN
	clJobInfo.m_cValidFlag		= clQueryJob.value(18).toChar().toLatin1();		// FT_JOB_INFO.VALID_FLAG

	strStdfFileName.sprintf("%s_%s_%s_%s.stdf",	clJobInfo.m_strDeviceID.latin1(), clJobInfo.m_strTestSys.latin1(),
												clJobInfo.m_strBatchNumber.latin1(), clJobInfo.m_strDateCode.latin1());

	// Update query progress dialog
	m_pQueryProgressDlg->UpdateFileInfo(clJobInfo.m_strDeviceID, clJobInfo.m_strBatchNumber, "", "");

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

	// Init diferent maps: array with part results, binning map,...
	if(!InitMaps(clJobInfo))
	{
		clStdfParse.Close();
		return false;
	}

	// Create testlist
	if(!CreateTestlist(clJobInfo))
	{
		clStdfParse.Close();
		return false;
	}

	// Start progress bar for current file
	m_pQueryProgressDlg->StartFileProgress(m_uiLotNbParts);
	
	// Write MIR
	if(!WriteMIR(clStdfParse, clJobInfo))
	{
		clStdfParse.Close();
		return false;
	}

	// Write static test info
	WriteStaticTestInfo(clStdfParse, clJobInfo);

	// Write test results
	if(!WriteTestResults(clStdfParse, clJobInfo, strTestlist))
	{
		clStdfParse.Close();
		return false;
	}

	// Write summary records (SBR, HBR, PCR, TSR)
	if(!WriteSummaryRecords(clStdfParse))
	{
		clStdfParse.Close();
		return false;
	}

	// Write MRR
	if(!WriteMRR(clStdfParse, clJobInfo))
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
bool GexDbPlugin_Dialogsemi::ConstructStdfFilesQuery_Ft(GexDbPlugin_Filter &cFilters, QString & strQuery)
{
	// Clear query string
	strQuery = "";

	// Construct query string:
	// SELECT <testnum, testname>
	// FROM <all required tables>
	// WHERE <link conditions> AND <value conditions>	(optional)
	Query_Empty();
	m_strlQuery_Fields.append("Field|ft_job_info.ft_job_info_id");
	m_strlQuery_Fields.append("Field|ft_job_info.device_id");
	m_strlQuery_Fields.append("Field|ft_job_info.batchnumber");
	m_strlQuery_Fields.append("Field|ft_job_info.datecode");
	m_strlQuery_Fields.append("Field|ft_job_info.testsys");
	m_strlQuery_Fields.append("Field|ft_job_info.test_startdate");
	m_strlQuery_Fields.append("Field|ft_job_info.test_enddate");
	m_strlQuery_Fields.append("Field|ft_job_info.total_tested");
	m_strlQuery_Fields.append("Field|ft_job_info.total_passed");
	m_strlQuery_Fields.append("Field|ft_job_info.testmode");
	m_strlQuery_Fields.append("Field|ft_job_info.temp");
	m_strlQuery_Fields.append("Field|ft_job_info.number_of_sites");
	m_strlQuery_Fields.append("Field|ft_job_info.loadboard");
	m_strlQuery_Fields.append("Field|ft_job_info.progname");
	m_strlQuery_Fields.append("Field|ft_job_info.handingsys");
	m_strlQuery_Fields.append("Field|ft_job_info.prog_revision");
	m_strlQuery_Fields.append("Field|ft_job_info.prog_revdate");
	m_strlQuery_Fields.append("Field|ft_job_info.acceptbin");
	m_strlQuery_Fields.append("Field|ft_job_info.valid_flag");
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
bool GexDbPlugin_Dialogsemi::QueryDataFiles_Ft(GexDbPlugin_Filter & cFilters, const QString & strTestlist, GexDbPlugin_DataFileList & cMatchingFiles, const QString & strLocalDir)
{
	QTime							tTimer;
	QString							strMessage;

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
	if(!ConstructStdfFilesQuery_Ft(cFilters, strQuery))
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
		if(CreateStdfFile_Ft(clQuery, strTestlist, strLocalDir, strStdfFilename))
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

