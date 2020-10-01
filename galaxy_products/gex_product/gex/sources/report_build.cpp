///////////////////////////////////////////////////////////
// All classes to create a report
///////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>

// #define used to readback computer paper size
#ifndef LOCALE_IPAPERSIZE
#define LOCALE_IPAPERSIZE             0x0000100A
#endif

#endif

#include <QProgressBar>
#include <qfile.h>
#include <qregexp.h>
#include <qapplication.h>
#include <QClipboard>
#include <QInputDialog>
#include "message.h"
#include <ctype.h>
#include <math.h>
#include "browser_dialog.h"
#include "browser.h"
#include "cbinning.h"
#include "engine.h"
#include "command_line_options.h"
#include "settings_dialog.h"
#include "gex_file_in_group.h"
#include "gex_shared.h"
#include "gex_version.h"
#include "gex_errors.h"
#include "gex_group_of_files.h"
#include "db_engine.h"
#include "report_build.h"
#include "report_options.h"
#include "drill_what_if.h"
#include "drill_chart.h"
#include "drill_table.h"
#include "drillDataMining3D.h"
#include "onefile_wizard.h"
#include "comparefiles_wizard.h"
#include "cpart_info.h"
#include "mergefiles_wizard.h"
#include "mixfiles_wizard.h"
#include "db_onequery_wizard.h"
#include "compare_query_wizard.h"
#include "gex_web.h"				// ExaminatorWEB
#include "import_all.h"				// ATDF, WAT, PCM, CSV, etc...=> to STDF converter
#include "report_classes_sorting.h"
#include "license_provider_profile.h"
#include "tb_pat_outlier_removal.h"
#include "patman_lib.h"
#include "gex_pat_constants_extern.h"
#include "scheduler_engine.h"
#include "status/status_task.h"
#include "status/status_taskdata.h"
#include "mo_task.h"
#include "chart_director.h"
#include "cstats.h"
#include "auto_repair_stdf.h"
#include "temporary_files_manager.h"
#include "gex_test_creator.h"
#include "reports_center_widget.h"
#include "gex_advanced_enterprise_report.h"
#include <gqtl_global.h>
#include <gqtl_log.h>
#include "gexperformancecounter.h"
#include "product_info.h"
#include "gqtl_datakeys.h"
#include "gex_database_entry.h"
#include "script_wizard.h"
#include "csl/csl_engine.h"
#include "report_template.h"
#include "report_template_io.h"
#include "admin_engine.h"
#include "read_system_info.h"
#include "pat_widget_ft.h"
#include "pat_report_ft_gui.h"
#include "ofr_controller.h"


#if defined unix || __MACH__
#include <stdlib.h>
#include <unistd.h>
#endif

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
#include "DebugMemory.h"

// main.cpp
#ifndef GSDAEMON
extern GexMainwindow *			pGexMainWindow;
#endif
extern QString					strIniClientSection;

// In main.cpp
extern QString workingFolder();

// browser_dialog.cpp
extern char *FindPath(char	*szString);
extern char *FindExt(char	*szString);

// In cstats.cpp
extern double	ScalingPower(int iPower);

// csl/ZcGexLib.cpp
extern CGexTestRange *createTestsRange(QString strParameterList,bool bAcceptRange, bool bIsAdvancedReport);

// All report generation options
CReportOptions	ReportOptions;

CGexReport		*gexReport=NULL;			// Handle to report class

// Initialize static member
//int CPartBinning::m_nGroupRunOffset = 0;

///////////////////////////////////////////////////////////
// Constructor: Custom Test settings (allows script to change
// test limits, add markers, etc...)
///////////////////////////////////////////////////////////
CTestCustomize::CTestCustomize()
{
    lTestNumber=(unsigned)-1;			// TestNumber
    bLimitWhatIfFlag= CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;	// bit0=1 (no low limit forced), bit1=1 (no high limit forced)
    ptMarker = NULL;		// No marker defined (yet)
    ptChartOptions=NULL;	// No custom viewport (yet)
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CTestCustomize::~CTestCustomize()
{
    if(ptMarker)
        delete ptMarker;
    ptMarker=0;
}

///////////////////////////////////////////////////////////
// Global data mostly comming from the STDF MIR record
///////////////////////////////////////////////////////////
CMir::CMir()
{
    // Ensures all variables are well initialized
    lSetupT					= 0;
    lStartT					= 0;
    lEndT					= 0;
    bStation				= 0;
    bModeCode				= 0;
    bRtstCode				= 0;
    bProdCode				= 0;
    iBurnTime				= 0;
    cModeCode				= 0;
    lFirstPartResult		= 0;
    *szLot=*szPartType=*szNodeName				= 0;
    *szTesterType=*szJobName=*szJobRev			= 0;
    *szSubLot=*szOperator=*szExecType			= 0;
    *szExecVer=*szTestCode=*szTestTemperature	= 0;
    *szUserText=*szAuxFile=*szPkgType			= 0;
    *szFamilyID=*szDateCode=*szFacilityID		= 0;
    *szFloorID=*szProcID=*szperFrq				= 0;
    *szSpecName=*szSpecVersion=*szFlowID		= 0;
    *szSetupID=*szDesignRev=*szEngID			= 0;
    *szROM_Code=*szSerialNumber=*szSuprName		= 0;
    *szProbeCardID = *szHandlerProberID = 0;
     bMrrRecord					= false;
    // PCR data
   /* bMergedPcrDone              = false;
    bFirstPcrRecord				= true;

    lPartCount					= 0;
    lRetestCount=lAbortCount	= -1;
    lGoodCount=lFuncCount		= -1;*/

}

CPcr::CPcr()
{
    bMergedPcrDone              = false;
    bFirstPcrRecord				= true;
    //bMrrRecord					= false;
    lPartCount					= 0;
    lRetestCount=lAbortCount	= -1;
    lGoodCount=lFuncCount		= -1;
}


///////////////////////////////////////////////////////////
// return the result for the given part number
///////////////////////////////////////////////////////////
const BinCell& CPartBinning::resultsAt(int nPartNumber) const
{
    if (nPartNumber < 0 || m_BinCellMap.find(nPartNumber) == m_BinCellMap.end())
        GSLOG(SYSLOG_SEV_ERROR, "Part number not found in bin cell map");

    return m_BinCellMap.at(nPartNumber);
}

///////////////////////////////////////////////////////////
// Remove all custom markers (all tests & all groups)
///////////////////////////////////////////////////////////
void	CGexReport::ClearCustomMarkers(QString lMarkerName/*=""*/)
{
    int					iGroupID	= 0;
    CGexGroupOfFiles *	pGroup		= (iGroupID < 0 || iGroupID >= getGroupsList().size()) ? NULL : getGroupsList().at(iGroupID);
    CGexFileInGroup *	pFile;
    CTest *				ptTestCell;
    TestMarker *		ptMark;

    // Scan all groups
    while(pGroup)
    {
        if (pGroup->pFilesList.isEmpty() == false)
        {
            pFile = pGroup->pFilesList.at(0);

            // For each test list
            ptTestCell = pFile->ptTestList;
            while(ptTestCell)
            {
                for (int nMarker = 0; nMarker < ptTestCell->ptCustomMarkers.size();)
                {
                    ptMark = ptTestCell->ptCustomMarkers.at(nMarker);

                    if (ptMark->strLabel.contains(lMarkerName, Qt::CaseInsensitive))
                    {
                        ptTestCell->ptCustomMarkers.removeAt(nMarker);
                        delete ptMark;
                    }
                    else
                        nMarker++;
                }

                // Next test in list
                ptTestCell = ptTestCell->GetNextTest();
            }
        }
        // Next group
        iGroupID++;
        pGroup= (iGroupID >= getGroupsList().size()) ? NULL : getGroupsList().at(iGroupID);
    };
}

bool CGexReport::HasCustomMarkers(const QString & lMarkerName)
{
    CGexGroupOfFiles *	lGroup		= (getGroupsList().size() > 0) ? getGroupsList().first() : NULL;
    CGexFileInGroup *	lFile;
    CTest *				lTestCell;
    TestMarker *		lMark;

    // Scan all groups
    for (int lIdx = 0; lIdx < pGroupsList.count(); ++lIdx)
    {
        lGroup = pGroupsList.at(lIdx);

        if (lGroup && lGroup->pFilesList.isEmpty() == false)
        {
            lFile = lGroup->pFilesList.at(0);

            if (lFile)
            {
                // For each test list
                lTestCell = lFile->ptTestList;
                while(lTestCell)
                {
                    for (int lMarkerIdx = 0; lMarkerIdx < lTestCell->ptCustomMarkers.size(); ++lMarkerIdx)
                    {
                        lMark = lTestCell->ptCustomMarkers.at(lMarkerIdx);

                        if (lMark && lMark->strLabel.contains(lMarkerName, Qt::CaseInsensitive))
                            return true;
                    }

                    // Next test in list
                    lTestCell = lTestCell->GetNextTest();
                }
            }
        }
    };

    return false;
}

///////////////////////////////////////////////////////////
// Checks if a given HTML section was created...
// Return : true if section created, false otherwise.
///////////////////////////////////////////////////////////
bool 	CGexReport::IfHtmlSectionCreated(int iSection)
{
    // No handle to HTML file...
    if(hReportFile == NULL)
        return false;

    // This section was market as 'not to create'
    if(m_pReportOptions->iHtmlSectionsToSkip & iSection)
        return false;

    // Section created.
    return true;
}

///////////////////////////////////////////////////////////
// Performs computes limits to be used in Sigma outlier mode.
// Return : none
///////////////////////////////////////////////////////////
void	CGexReport::ComputeOutlierSigmaLimits(CTest *ptTestCell, double fRatio)
{
    double	fSigma,fMean;

    fMean = ptTestCell->lfTmpSamplesTotal/ptTestCell->ldSamplesValidExecs;
    if(ptTestCell->ldSamplesValidExecs > 1)
    {
        fSigma = sqrt(fabs((((double)ptTestCell->ldSamplesValidExecs*ptTestCell->lfTmpSamplesTotalSquare)
                            - GS_POW(ptTestCell->lfTmpSamplesTotal,2.0))/
                           ((double)ptTestCell->ldSamplesValidExecs*((double)ptTestCell->ldSamplesValidExecs-1))));
    }
    else
        fSigma=0;

    ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier = fMean - (fSigma*fRatio);
    ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier = fMean + (fSigma*fRatio);
    ptTestCell->bOutlierLimitsSet = true;

    // Prebuilt histogram will only cover the Xsigma window defined.
    ptTestCell->GetCurrentLimitItem()->lfHistogramMin = ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier;
    ptTestCell->GetCurrentLimitItem()->lfHistogramMax = ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier;

    // Samples Min and SamplesMax will be re-computed during pass#2
    ptTestCell->lfSamplesMin = C_INFINITE;
    ptTestCell->lfSamplesMax = -C_INFINITE;
}

///////////////////////////////////////////////////////////
// Performs intermediate pass work...for faster data processing!
// Return : none
///////////////////////////////////////////////////////////
void	CGexReport::PrepareSecondPass(CGexGroupOfFiles *pGroup,CReportOptions *pReportOptions)
{
    CGexFileInGroup		*pFile;
    CTest	*ptTestCell;
    long	lTotalParameters=0;			// Will hold number of parameters
    long	lTotalValidParameters=0;	// Will hold number of parameters that have samples.

    QString orm = pReportOptions->GetOption("dataprocessing", "data_cleaning_mode").toString();
    double orv=pReportOptions->GetOption("dataprocessing","data_cleaning_value").toDouble();

    // First file in group...
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // Scan all list of tests, and build some fields: makes pass 2 faster!
    ptTestCell = pGroup->cMergedData.ptMergedTestList;
    while(ptTestCell != NULL)
    {
        // Keep track of the nb of parameters and nb of parameters with data samples (needed by the Plugins)
        lTotalParameters++;

        if(ptTestCell->ldSamplesValidExecs > 0)
        {
            // Saves test definition position in test list
            ptTestCell->lTestID = lTotalValidParameters;

            // Keep track of total valid Parameters in the flow.
            lTotalValidParameters++;
        }
        else
            // Saves test definition position in test list: use -1 for tests with no execution
            ptTestCell->lTestID = -1;

        // If Outlier is based on Sigma, compute it.
        if ( (orm=="n_sigma") || (orm=="exclude_n_sigma") )
        {
            ComputeOutlierSigmaLimits(ptTestCell, (float)orv);
        }

        // Point to next test cell
        ptTestCell = ptTestCell->GetNextTest();
    };	// Loop until all test cells read.

    // Ensure wafermap buffer allocated/setup for each file in group that needs one.
    long	lSizeX,lSizeY;
    QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

    while (itFilesList.hasNext())
    {
        pFile = itFilesList.next();

        // No wafer map (Final test data)...then "Strip map" (Final test Wafermap) if at least one sample available
        if(pFile->getWaferMapData().bWaferMapExists == false && pFile->ldTotalPartsSampled)
        {
            pFile->getWaferMapData().bWaferMapExists = true;
            pFile->getWaferMapData().bStripMap		= true;
            pFile->getWaferMapData().iLowDieX= 0;
            pFile->getWaferMapData().iLowDieY = 0;
            pFile->getWaferMapData().iHighDieX = pFile->getWaferMapData().iHighDieY = (int) sqrt((double)(pFile->ldTotalPartsSampled));

            // See if we've allocted one extra line (if so, decrement line count)
            if(pFile->getWaferMapData().iHighDieX*pFile->getWaferMapData().iHighDieY == pFile->ldTotalPartsSampled)
            {
                pFile->getWaferMapData().iHighDieX--;
                pFile->getWaferMapData().iHighDieY--;
            }
            else
                if(pFile->getWaferMapData().iHighDieX*(pFile->getWaferMapData().iHighDieY+1) >= pFile->ldTotalPartsSampled)
                    pFile->getWaferMapData().iHighDieX--;

            // Build a WaferID based on LoitID and SublotID
            strcpy(pFile->getWaferMapData().szWaferID,pFile->getMirDatas().szLot);
            if(*pFile->getMirDatas().szSubLot)
            {
                strcat(pFile->getWaferMapData().szWaferID,"-");
                strcat(pFile->getWaferMapData().szWaferID,pFile->getMirDatas().szSubLot);
            }
        }

        // If Wafermap, compute its size, and allocate buffer to store it!
        if(pFile->getWaferMapData().bWaferMapExists == true)
        {
            lSizeX = pFile->getWaferMapData().iHighDieX-pFile->getWaferMapData().iLowDieX+1;
            lSizeY = pFile->getWaferMapData().iHighDieY-pFile->getWaferMapData().iLowDieY+1;
            pFile->getWaferMapData().setWaferMap(CWafMapArray::allocate(lSizeX*lSizeY));
            pFile->getWaferMapData().allocCellTestCounter(lSizeX*lSizeY);
            /* HTH - case 4156 - Catch memory allocation exception
            pFile->getWaferMapData().getWafMap() = new CWafMapArray[lSizeX*lSizeY];
            */
            if(pFile->getWaferMapData().getWafMap()==NULL)
            {
                // Failed allocating memory for wafermap===> corrupted wafermap, ignore it!
                pFile->getWaferMapData().bWaferMapExists = false;
            }
            else
            {
                // Save wafermap size
                pFile->getWaferMapData().SizeX = lSizeX;
                pFile->getWaferMapData().SizeY = lSizeY;

                /* HTH - case 4156 - Catch memory allocation exception - ALREADY DONE IN THE ALLOCATE(...) METHOD
                // Fill wafer with 'Empty cell' IDs.
                pFile->getWaferMapData().clearWafer();
                */
            }
        }

        pFile->onPrepareSecondPass();
    };
}

// Classed used to sort Test list based on FlowID.
class CRetest
{
public:
    long	lTestFlowID;
    CTest	*ptTest;

    inline bool operator < (const CRetest &t) const
    {
        return (lTestFlowID < t.lTestFlowID);
    }
};

/////////////////////////////////////////////////////////////////////////////
// Virtual retest: reprocess each data record & overload pass/fail status
/////////////////////////////////////////////////////////////////////////////
void CGexReport::VirtualRetest(bool bStopOnFail,bool bUpdateReport, virtualRetestParts eComputeOn)
{
    // Get dataset handle.
    CGexGroupOfFiles *	pGroup			= NULL;
    CGexFileInGroup *	pFile			= NULL;
    int					nWhatIfPassBin	= m_pReportOptions->GetOption("adv_what_if", "pass_bin").toString().toInt();
    int					nWhatIfFailBin	= m_pReportOptions->GetOption("adv_what_if", "fail_bin").toString().toInt();
    int					nWhatIfUknownFailBin	= m_pReportOptions->GetOption("adv_what_if", "unknown_fail_bin").toString().toInt();

    for(int nGroupID = 0; nGroupID < getGroupsList().count(); nGroupID++)
    {
        pGroup = getGroupsList().at(nGroupID);

        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        if(pFile == NULL)
            continue;

        // Get handle to SoftBin & HardBin parameters
        CTest *	ptSoftBin;
        CTest *	ptHardBin;
        CTest * ptDieX	= NULL;
        CTest * ptDieY	= NULL;

        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN, GEX_PTEST, &ptSoftBin, true,false) != 1)
            continue;
        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN, GEX_PTEST, &ptHardBin, true,false) != 1)
            continue;

        pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &ptDieX, true, false);
        pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &ptDieY, true, false);

        // Create parameter list sorted by flow index.
        // Necessary for the Stop on Fail option
        CRetest	cRetest;
        QList <CRetest> cRetestList;
        CTest *ptTestCell = pGroup->cMergedData.ptMergedTestList;
        while(ptTestCell)
        {
            // Get pointer to test cell
            cRetest.lTestFlowID = ptTestCell->lTestFlowID;
            cRetest.ptTest = ptTestCell;

            // Add valid test entry to list
            if((cRetest.lTestFlowID >= 0) && (ptTestCell->bTestType != '-'))
                cRetestList.append(cRetest);

            // Move to next cell
            ptTestCell = ptTestCell->GetNextTest();
        };
        qSort(cRetestList.begin(), cRetestList.end());

        // Check each part run
        bool		bGoodPart,bTestFail,bWhatIfFailure, bHasTestExecuted;
        int			iSampleOffset,iTestInFlow,iTotalFailures;
        int			lSoftFailBin, lHardFailBin;
        double		lfValue;
        CPartInfo *	ptrPartInfo		= NULL;
        CTest *		pLastFailedTest = NULL;
        double		dFailingValue	= GEX_C_DOUBLE_NAN;

        // Clear total parts fail count
        bool	bUpdatedMap=false;
        iTotalFailures = 0;

        int nSamplesExecs = gex_min(ptHardBin->m_testResult.count(), ptSoftBin->m_testResult.count());

        for(iSampleOffset = 0; iSampleOffset < nSamplesExecs; iSampleOffset++)
        {
            ptrPartInfo = NULL;
            lSoftFailBin = lHardFailBin = -1;
            // Check if part is good
            bGoodPart = pGroup->isPassingBin(true, (int) ptSoftBin->m_testResult.resultAt(iSampleOffset));

            // Check if we have to do the virtual retest on this part
            if (eComputeOn == CGexReport::allParts || (eComputeOn == CGexReport::goodParts && bGoodPart) || (eComputeOn == CGexReport::failedParts && !bGoodPart))
            {
                // For each part focused by the WhatIf
                // Keep the original Binning value in case we have to roll back
                lHardFailBin = (int) ptHardBin->m_testResult.resultAt(iSampleOffset);
                lSoftFailBin = (int) ptSoftBin->m_testResult.resultAt(iSampleOffset);

                // Reset the Fail counter
                if(!bGoodPart)
                {
                    // But keep also the original Part status
                    // bGoodPart = true;
                    // Reset all Failed part to WhatIf Pass
                    ptHardBin->m_testResult.forceResultAt(iSampleOffset, nWhatIfPassBin);
                    ptSoftBin->m_testResult.forceResultAt(iSampleOffset, nWhatIfPassBin);
                }

                if (ptDieX && ptDieY)
                {
                    int idxPartInfo = iSampleOffset;
                    for (int idxFile = 0; idxFile < pGroup->pFilesList.count() && ptrPartInfo == NULL; ++idxFile){

                        CGexFileInGroup *pTmpFile = pGroup->pFilesList.at(idxFile);
                        if (pTmpFile){
                            if (idxPartInfo < pTmpFile->pPartInfoList.count())
                                ptrPartInfo = pTmpFile->pPartInfoList.at(idxPartInfo);
                            else
                                idxPartInfo -= pTmpFile->pPartInfoList.count();
                        }
                    }
                    if(!ptrPartInfo)
                        continue;
                }

                // Clear variables
                bTestFail			= false;
                bWhatIfFailure		= false;
                bHasTestExecuted	= false;
                pLastFailedTest		= NULL;

                if (ptrPartInfo && ptrPartInfo->iTestsExecuted > 0)
                {
                    // Check all parameters in run flow.
                    for (iTestInFlow = 0; iTestInFlow < cRetestList.size() ; ++iTestInFlow)
                    {
                        // Get test handle
                        cRetest = cRetestList.at(iTestInFlow);
                        ptTestCell = cRetest.ptTest;
                        // If parametric test, check its result over its limits
                        // Check if have a result for this part
                        if(ptTestCell->m_testResult.isValidIndex(iSampleOffset))
                        {
                            // Check if the result is valid
                            if( ptTestCell->m_testResult.isValidResultAt(iSampleOffset) )
                            {
                                // At least one result valid
                                bHasTestExecuted = true;

                                // Extrat the result
                                lfValue = ptTestCell->m_testResult.resultAt(iSampleOffset);
                                // Format result
                                lfValue *= ScalingPower(ptTestCell->res_scal);

                                // Check if the result is Pass or Fail
                                // The passFailStatus return Pass or False if no new WahtIf limit
                                // The passFailStatus return Undefined if new WahtIf limit
                                // The isFailingValue return the Status for (Pass or False)
                                // The isFailingValue check the value according to the new WhatIf limits if Status Undefined
                                if(ptTestCell->isFailingValue(lfValue, ptTestCell->m_testResult.passFailStatus(iSampleOffset)))
                                {
                                    // The test is failed
                                    // But we don't know if it is due to the new WhatIf limits
                                    bTestFail = true;

                                    // Check if failure occured on a what-if or regular limit!
                                    if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
                                    {
                                        // NO new WhatIf definition
                                        // Regular limit
                                        bWhatIfFailure	= false;	// Regular test limit

                                        // Test Flags was good or fail
                                        // In iFailBin, only the FAIL bin value
                                        // When the Status is PASS, iFailBin = -1
                                        bool bIsPassBin = pGroup->isPassingBin(true, ptTestCell->iFailBin);
                                        if(bIsPassBin)
                                        {
                                            // Test Flags was good
                                            // The Test result is fail but ignored
                                            if(bGoodPart)
                                            {
                                                // Consider the Part as GOOD
                                                // Keep the original GOOD Soft/Hard Bin
                                                bTestFail = false;
                                                continue;
                                            }
                                            else
                                            {
                                                // the Part was already forced to WhatIfPassBin
                                                bTestFail = true;
                                            }
                                        }
                                        else
                                        {
                                            // Test Flags was fail
                                            // Roll back to the original binning
                                            // The iFailBin = lSoftFailBin but not overwrite the HardBin with this value !!!
                                            // We need to do this here because of the bUpdateReport that can be false ?
                                            ptHardBin->m_testResult.forceResultAt(iSampleOffset, lHardFailBin);//ptTestCell->iFailBin);
                                            ptSoftBin->m_testResult.forceResultAt(iSampleOffset, lSoftFailBin);//ptTestCell->iFailBin);
                                        }
                                    }
                                    else
                                    {
                                        // The Test Fail was due to the new WhatIf limits
                                        bWhatIfFailure = true;	// WhatIf limit
                                    }
                                    pLastFailedTest = ptTestCell;
                                    dFailingValue	= lfValue;
                                }
                                else
                                {
                                    // The test is not fail
                                    // due to the original limits
                                    // or  due to the new WhatIf limits
                                    // If the Part was fail, it was already forced to WhatIfPassBin
                                    // Else the Bin is the original GOOD bin
                                }
                            }
                        }

                        // If 'Stop on first fail'...
                        if(bStopOnFail & bTestFail)
                        {
                            // Exit on first test Failure: remove
                            // ALL samples after this test in the data flow!

                            // Case 6575: remove ALL samples even if the fail
                            // is not a What-If fail.
#if 0
                            // Remove ALL samples after this test in the data
                            // flow only if What-If failure.
                            if(bWhatifFailure == false)
                                break;
#endif

                            // Stop on Fail
                            // Invalidate all the results after the current test fail
                            iTestInFlow++;	// Move to next test in flow
                            while(iTestInFlow < cRetestList.size())
                            {
                                // Get test handle
                                cRetest = cRetestList.at(iTestInFlow);
                                ptTestCell = cRetest.ptTest;

                                // If parametric test exists, remove its result for this flow.
                                if(ptTestCell->m_testResult.count() > 0)
                                    ptTestCell->m_testResult.invalidateResultAt(iSampleOffset);

                                // Next test in flow
                                iTestInFlow++;
                            };
                            break;
                        }
                    }
                }

                if(bUpdateReport)
                {
                    // Check if test failure in this flow (if so overload the Binning)
                    if(bTestFail)
                    {
                        // Keep track of total failures
                        iTotalFailures++;

                        // Is this a WhatIf or Regular failure ?
                        if(bWhatIfFailure)
                        {
                            // Overload BIN as we have a What-If failure!
                            ptHardBin->m_testResult.forceResultAt(iSampleOffset, nWhatIfFailBin);
                            ptSoftBin->m_testResult.forceResultAt(iSampleOffset, nWhatIfFailBin);

                            // Flag map changed
                            bUpdatedMap = true;
                        }
                        else if(lSoftFailBin != -1)
                        {
                            // RollBack to the original Bin
                            ptHardBin->m_testResult.forceResultAt(iSampleOffset, lHardFailBin);
                            ptSoftBin->m_testResult.forceResultAt(iSampleOffset, lSoftFailBin);
                        }
                        else if(lSoftFailBin == -1)
                        {
                            // Unknown Bin
                            ptHardBin->m_testResult.forceResultAt(iSampleOffset, nWhatIfUknownFailBin);
                            ptSoftBin->m_testResult.forceResultAt(iSampleOffset, nWhatIfUknownFailBin);
                        }
                    }
                    // If no test failure in this flow and part was good or no test has been executed, don't change the binning
                    else if (!bGoodPart && bHasTestExecuted)
                    {
                        // The Part was FAIL but overwrited to nWhatIfPassBin
                        // This part is PASS
                        if(ptSoftBin->m_testResult.resultAt(iSampleOffset) != nWhatIfPassBin)
                        {
                            // Toggle this part from FAIL to PASS!...yield recovering!
                            ptHardBin->m_testResult.forceResultAt(iSampleOffset, nWhatIfPassBin);
                            ptSoftBin->m_testResult.forceResultAt(iSampleOffset, nWhatIfPassBin);

                            // Flag map changed
                            bUpdatedMap = true;
                        }
                    }

                    // Overwrite the PartInfo data
                    if (ptrPartInfo)
                    {
                        ptrPartInfo->iHardBin = (int) ptHardBin->m_testResult.resultAt(iSampleOffset);
                        ptrPartInfo->iSoftBin = (int) ptSoftBin->m_testResult.resultAt(iSampleOffset);

                        if (pLastFailedTest)
                        {
                            ptrPartInfo->iFailTestNumber	= pLastFailedTest->lTestNumber;
                            ptrPartInfo->iFailTestPinmap	= pLastFailedTest->lPinmapIndex;
                            ptrPartInfo->fFailingValue		= dFailingValue;
                        }
                        else if (bUpdatedMap)
                            // Reset failed test number
                            ptrPartInfo->iFailTestNumber	= (unsigned)-1;
                    }
                }
            }
        }

        // If Updated report asked
        if(bUpdateReport)
        {
            // Clear SOFT Bin count
            CBinning	*ptBinCell = pGroup->cMergedData.ptMergedSoftBinList;
            while(ptBinCell)
            {
                ptBinCell->ldTotalCount = 0;
                ptBinCell = ptBinCell->ptNextBin;
            }
            // Clear HARD Bin count
            ptBinCell = pGroup->cMergedData.ptMergedHardBinList;
            while(ptBinCell)
            {
                ptBinCell->ldTotalCount = 0;
                ptBinCell = ptBinCell->ptNextBin;
            }

            // Load Wafermap array with relevant data
            FillWaferMap(pGroup,pFile,ptSoftBin,m_pReportOptions->iWafermapType);

            // Ensure Yield computed from wafermap
            EndDataCollection();
        }
    }

    // Compute test statistics
    ComputeDataStatistics();
}

///////////////////////////////////////////////////////////
// When alls groups relate to same dataset but filtered
// by site#, tell which group holds a given site
// Note: this is a non-sense to call this when groups are not
// the same dataset or not filtered by site#
///////////////////////////////////////////////////////////
int	CGexReport::getGroupForSite(int iSite)
{
    // If only one group, then this is a single site dataset!
    if(getGroupsList().count() <= 1)
        return 0;

    // Get dataset handle.
    CGexGroupOfFiles *	pGroup	= NULL;
    CGexFileInGroup *	pFile	= NULL;

    for(int uGroupID = 0; uGroupID < getGroupsList().count(); uGroupID++)
    {
        pGroup = getGroupsList().at(uGroupID);
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.at(0);
        if(pFile && pFile->iProcessSite == iSite)
            return uGroupID;	// Return group#
    }

    // This site doesn't exist!
    return -1;
}

///////////////////////////////////////////////////////////
// When alls groups relate to same dataset but filtered
// by site#, tell which group holds a given DieX,Y
// Note: this is a non-sense to call this when groups are not
// the same dataset or not filtered by site#
///////////////////////////////////////////////////////////
int	CGexReport::getSiteForDie(int iDieX,int iDieY)
{
    // If only one group, then this is a single site dataset!
    if(getGroupsList().count() <= 1)
        return 0;

    // Get dataset handle.
    CGexGroupOfFiles *	pGroup	= NULL;
    CGexFileInGroup *	pFile	= NULL;

    // Get handle to Die X,Y info
    CTest *				ptDieX	= NULL;
    CTest *				ptDieY	= NULL;
    long				lPartId;

    int nSamplesExecs = 0;

    for(int uGroupID = 0; uGroupID < getGroupsList().count(); uGroupID++)
    {
        pGroup = getGroupsList().at(uGroupID);
        if (!pGroup->pFilesList.isEmpty())
        {
            pFile = pGroup->pFilesList.at(0);

            if (pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &ptDieX, true, false) == 1 &&
                pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &ptDieY, true, false) == 1)
            {
                nSamplesExecs = gex_min(ptDieX->m_testResult.count(), ptDieY->m_testResult.count());

                for(lPartId = 0; lPartId < nSamplesExecs; lPartId++)
                {
                    // Found a die location matching our selection
                    if((iDieX == (int)ptDieX->m_testResult.resultAt(lPartId)) && (iDieY == (int) ptDieY->m_testResult.resultAt(lPartId)))
                        return pFile->iProcessSite;
                }
            }
        }
    }

    // die location doesn't exist!
    return -1;
}

///////////////////////////////////////////////////////////
// Performs end-of-multi-passes (pass#2) work on dataset...
// for faster data processing!
// Return : none
///////////////////////////////////////////////////////////
void	CGexReport::FinishPasses(CGexGroupOfFiles *pGroup,CReportOptions *pReportOptions)
{
    CGexFileInGroup *	pFile		= NULL;
    CTest *				ptTestCell	= NULL;
    QString strOutputFormatOption;
    strOutputFormatOption=pReportOptions->GetOption("output", "format").toString();

    // Process operation on result
    ptTestCell = pGroup->cMergedData.ptMergedTestList;
    BYTE mrc=0;
    QString mpc=pReportOptions->GetOption( "dataprocessing","multi_parametric_merge_criteria").toString();
    if(mpc=="first") { mrc=GEX_MULTIRESULT_USE_FIRST; } else
        if(mpc=="last") { mrc=GEX_MULTIRESULT_USE_LAST; } else
            if(mpc=="min") { mrc=GEX_MULTIRESULT_USE_MIN; } else
                if(mpc=="max") { mrc=GEX_MULTIRESULT_USE_MAX; } else
                    if(mpc=="mean") { mrc=GEX_MULTIRESULT_USE_MEAN; } else
                        if(mpc=="median") { mrc=GEX_MULTIRESULT_USE_MEDIAN; } else
                            GSLOG(SYSLOG_SEV_WARNING, QString(" error : unsupported multi_parametric_merge_criteria option '%1' !").arg( mpc).toLatin1().constData());

    bool keepone=pReportOptions->GetOption("dataprocessing","multi_parametric_merge_option")
                 .toString()=="keep_one"?true:false;

    QString scaling=pReportOptions->GetOption("dataprocessing","scaling").toString();
    bool isScalingNormalized=(scaling=="normalized");
    bool isScalingToLimits=(scaling=="to_limits");

    while (ptTestCell)
    {
        ptTestCell->m_testResult.processMultiResult(
                    mrc, //pReportOptions->cMultiResultValue,
                    keepone //pReportOptions->bMultiParametricAlwaysKeepOne
                    );
        ptTestCell = ptTestCell->GetNextTest();
    };

    // Apply filter
    pGroup->postLoadProcessing();

    // Update Pass/Fail info for each SoftBin
    CBinning *ptSoftBinList = pGroup->cMergedData.ptMergedSoftBinList;
    while(ptSoftBinList != NULL)
    {
        if((ptSoftBinList->cPassFail != 'P') && (ptSoftBinList->cPassFail != 'F') &&
           pGroup->cMergedData.mSBinPassFailFlag.contains(ptSoftBinList->iBinValue))
        {
            if(pGroup->cMergedData.mSBinPassFailFlag[ptSoftBinList->iBinValue])
                ptSoftBinList->cPassFail = 'P';
            else
                ptSoftBinList->cPassFail = 'F';
        }

        // Assign a color for this soft bin
        cDieColor.assignBinColor(ptSoftBinList->iBinValue, (ptSoftBinList->cPassFail == 'P'));

        // Check nexy bin cell
        ptSoftBinList = ptSoftBinList->ptNextBin;
    };

    // Update Pass/Fail info for each HardBin
    CBinning * ptHardBinList = pGroup->cMergedData.ptMergedHardBinList;
    while(ptHardBinList != NULL)
    {
        if((ptHardBinList->cPassFail != 'P') && (ptHardBinList->cPassFail != 'F') &&
           pGroup->cMergedData.mHBinPassFailFlag.contains(ptHardBinList->iBinValue))
        {
            if(pGroup->cMergedData.mHBinPassFailFlag[ptHardBinList->iBinValue])
                ptHardBinList->cPassFail = 'P';
            else
                ptHardBinList->cPassFail = 'F';
        }

        // Assign a color for this hard bin
        cDieColor.assignBinColor(ptHardBinList->iBinValue, (ptHardBinList->cPassFail == 'P'));

        // Check nexy bin cell
        ptHardBinList = ptHardBinList->ptNextBin;
    };


    // Do some post action
    QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

    while (itFilesList.hasNext())
    {
        pFile = itFilesList.next();

        // Check for Swap-die action
        pFile->SwapWaferMapDies();

        // Set the gross die count defined in the options
        //if (pReportOptions->iGrossDieCount > 0)
        QVariant gd=pReportOptions->GetOption("wafer", "gross_die");
        bool ok=false;
        int gdc=gd.toInt(&ok);
        if ( (ok) && (gdc>0) )
            pFile->setGrossDieCount(gdc); //pFile->setGrossDieCount(pReportOptions->iGrossDieCount);

        // Cumulate gross die count for the group
        pGroup->cMergedData.addGrossDieCount(pFile->grossDieCount());
    };

    // Build 'Stacked wafermap' data array (by merging all individual wafers in this group.
    // + compute physical dies tested per wafer.
    pGroup->BuildStackedWaferMap(pReportOptions);

    // First file in group...
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // Scan all list of tests, and compute last-minute info
    ptTestCell = pGroup->cMergedData.ptMergedTestList;

    while(ptTestCell != NULL)
    {

#if 0
        // If Outlier is based on Sigma, compute it.
        switch(pReportOptions->iOutlierRemoveMode)
        {
            case GEX_OPTION_OUTLIER_SIGMA:
            case GEX_OPTION_INLINER_SIGMA:
                // Remove outlier count as during pass1, ALL parts are counted
                // and the xSigma filter in only activated in pass2!
                if(ptTestCell->ldOutliers > 0)
                    ptTestCell->ldSamplesExecs -= ptTestCell->ldOutliers;
                break;

            default:
                // Make sure the samples exec is updated to only include data filtered
                if(ptTestCell->ldSamplesExecs > ptTestCell->ldCurrentSample)
                    ptTestCell->ldSamplesExecs = ptTestCell->ldCurrentSample;
                break;
        }
#endif // 0

        //QString of=pReportOptions->GetOption("output", "format").toString();
        // pyc, 23/01/12, speed optim

        // Replace any 'tab' or coma from test names so to avoid conflict sending to CSV file.
        if(strOutputFormatOption=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            ptTestCell->strTestName.replace('\t',' ');
            ptTestCell->strTestName.replace(',',';');
        }

        // Normalize scaling factors in case of negative scale coded as a unsigned int.
        if(ptTestCell->res_scal >= 0x80)
            ptTestCell->res_scal = ptTestCell->res_scal - 256;
        if(ptTestCell->llm_scal >= 0x80)
            ptTestCell->llm_scal = ptTestCell->llm_scal - 256;
        if(ptTestCell->hlm_scal >= 0x80)
            ptTestCell->hlm_scal = ptTestCell->hlm_scal - 256;

        // Set result scale according the scaling option
        //if (pReportOptions->iSmartScaling == GEX_UNITS_RESCALE_TO_LIMITS)
        if (isScalingToLimits)
            ptTestCell->res_scal = ptTestCell->hlm_scal;
        else //if (pReportOptions->iSmartScaling == GEX_UNITS_RESCALE_NORMALIZED)
            if(isScalingNormalized)
                ptTestCell->res_scal = 0;

        // Point to next test cell
        ptTestCell = ptTestCell->GetNextTest();
    };

    //QString of=pReportOptions->GetOption("output", "format").toString();

    // If creating a FLAT HTML file, and Datalog is enabled...then we must insert a page break now as next section will be Test statistics!
    if((ReportOptions.getAdvancedReport() == GEX_ADV_DATALOG)
       && ( (strOutputFormatOption=="DOC")||(strOutputFormatOption=="PDF")||(strOutputFormatOption=="PPT")||strOutputFormatOption=="ODT" )
       //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
       )
    {
        // Close Datalog table
        fprintf(hAdvancedReport,"</table>\n");
        // Write page break (ignored if not writing a flat HTML document)
        WritePageBreak();
    }
}

QString	CGexReport::buildDisplayName(QString strString, bool bUseOutputFormat /* = true */)
{
    QString strName = strString;
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    QString tn=m_pReportOptions->GetOption("output", "truncate_names").toString();

    // For Non HTML report (Word, PDF, PowerPoint), check if have to truncate name at 32 chars. max.
    if  (m_pReportOptions->isReportOutputHtmlBased())
        //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        bool ok=false; int tl=tn.toInt(&ok); if (!ok) tl=-1;
        if (tn=="yes") tl=32;
        if((tl > 0) && ((int)strName.length() >= tl))
        {
            // Do not truncate in the middle of a '<>' string!
            strName.truncate(tl);
            strName += "...";
        }

        // Filter characters that can trouble PDF converter
        if(bUseOutputFormat && (of==QString("PDF")))		// PYC, 17/05/2011
        {
            strName.replace("<", "&lt;");
            strName.replace(">", "&gt;");
        }
    }

    return strName;
}

///////////////////////////////////////////////////////////
// Returns test name formated for output device (eg: truncated string, etc...)
///////////////////////////////////////////////////////////
QString	CGexReport::buildDisplayName(CTest *ptTestCell)
{
    if (!ptTestCell)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Build Test display name : error : TestCell NULL");
        return "?";
    }

    QString strString = ptTestCell->strTestName;
    return buildDisplayName(strString);
}

void CGexReport::CreateNewHtmlStatisticPage()
{
    // close table
    fprintf(hReportFile,"</table>\n");
    WritePageBreak();
    // Reopen table + write header
    WriteHtmlOpenTable(98,0);	// HTML code to open table, Width 98%, cell spacing=0
}

///////////////////////////////////////////////////////////
// Action link clicked, relative to Advanced Enterprise Reports
///////////////////////////////////////////////////////////
void CGexReport::ProcessActionLink_AdvEnterpriseReports(const QString & strGexHtmlToolbarAction)
{
    QString strToken, strAction;

    // Get action
    strToken = strGexHtmlToolbarAction.section("--",1,1);
    if(!strToken.toLower().startsWith("action="))
        return;

    strAction = strToken.section("=", 1, 1);

    // Dispatch action
    if (strAction.toLower() == "modify_params")
    {
        ReportsCenterWidget::ShowLatestParamsWindow();
    }
    else if (strAction.toLower() == "savecsvfile")
    {
        int		nSectionID = -1;
        int		nDatasetID = -1;
        QString strSection;

        // get the section ID
        strSection = strGexHtmlToolbarAction.section("--", 2, 2);

        if (strSection.toLower().startsWith("section="))
            nSectionID = strSection.section("=", 1, 1).toInt();

        // get the dataset ID
        strSection = strGexHtmlToolbarAction.section("--", 3, 3);

        if (strSection.toLower().startsWith("dataset="))
            nDatasetID = strSection.section("=", 1, 1).toInt();

        if (nSectionID != -1 && nDatasetID != -1)
        {
            // Build file name
            QString strFileName = m_pAdvancedEnterpriseReport->name() + ".csv";

            // Let's user tell where to save the configuration.
#           ifndef GSDAEMON
            if (pGexMainWindow == NULL)
            {
                return;
            }
            strFileName = QFileDialog::getSaveFileName(pGexMainWindow,
                                                       "Save Advanced Enterprise Report as...",
                                                       strFileName,
                                                       "Advanced Enterprise Report (*.csv)");
#           else
            return;
#           endif

            // If no file selected, ignore command.
            if(strFileName.isEmpty())
                return;

            // Check if destination exists...
            if (QFile::exists(strFileName))
            {
                bool nCode;
                GS::Gex::Message::request(
                    "", "File already exists. Overwrite it ?", nCode);
                if (! nCode)
                {
                    return;
                }
            }

            QFile fileCsv(strFileName);

            if (!fileCsv.open(QIODevice::WriteOnly | QIODevice::Text))
                return;

            QTextStream txtStream(&fileCsv);
            m_pAdvancedEnterpriseReport->exportToCsv(txtStream, nSectionID, nDatasetID);

            fileCsv.close();
        }
    }
    else if (strAction.toLower() == "saveexcelclipboard")
    {
        int		nSectionID = -1;
        int		nDatasetID = -1;
        QString strSection;

        // get the section ID
        strSection = strGexHtmlToolbarAction.section("--", 2, 2);

        if (strSection.toLower().startsWith("section="))
            nSectionID = strSection.section("=", 1, 1).toInt();

        // get the dataset ID
        strSection = strGexHtmlToolbarAction.section("--", 3, 3);

        if (strSection.toLower().startsWith("dataset="))
            nDatasetID = strSection.section("=", 1, 1).toInt();

        if (nSectionID != -1 && nDatasetID != -1)
        {
            QString strData;
            QTextStream txtStream(&strData);

            m_pAdvancedEnterpriseReport->exportToSpreadSheet(txtStream, nSectionID, nDatasetID);

            // Copy data to clipboard
            QClipboard * lClipBoard = QGuiApplication::clipboard();
            if (!lClipBoard)
            {
                GSLOG(SYSLOG_SEV_WARNING, "failed to retrieve application clipboard");
            }
            else
            {
                lClipBoard->setText(strData, QClipboard::Clipboard );
            }

            QString		strMessage = "Table saved to clipboard in Spreadsheet format,\nready to paste!\nLaunch Spreadsheet application now?";
            bool lOk;
            GS::Gex::Message::request("", strMessage, lOk);
            if (! lOk)
            {
                return;
            }
#ifdef _WIN32
            // Launch spreadsheet editor
            ShellExecuteA(NULL,
                          "open",
                          m_pReportOptions->GetOption("preferences", "ssheet_editor").toString().toLatin1().constData(),
                          NULL,
                          NULL,
                          SW_SHOWNORMAL);
#else
            //system(pReportOptions->m_PrefMap["ssheet_editor"].
            //       toLatin1().constData());
            if (system(m_pReportOptions->GetOption("preferences",
                                                   "ssheet_editor").
                       toString().toLatin1().constData()) == -1) {
                //FIXME: send error
            }
#endif
        }
    }
}


///////////////////////////////////////////////////////////

// Create a group
///////////////////////////////////////////////////////////
int CGexReport::addGroup(QString strGroupName,GexDatabaseQuery *pQuery)
{
    // Creates the new group
    CGexGroupOfFiles *pGroup = new CGexGroupOfFiles(strGroupName, getGroupsList().count());

    // In case group results from a Query, save its filters.
    if(pQuery !=NULL)
        pGroup->cQuery = *pQuery;

    // Add group to the group list
    getGroupsList().append(pGroup);

    // Return group ID
    return pGroup->GetGroupId();
}

///////////////////////////////////////////////////////////
// Build report folder into database structure.
///////////////////////////////////////////////////////////
bool
CGexReport::CreateDatabaseReportFolder(CReportOptions* pReportOptions,
                                       const QString& /*strReportFileName*/,
                                       QString& strReportType)
{
    QDir cDir;

    QString strLegacyReportName = reportAbsFilePath();

    // Refresh Output report format (if needed)
    overloadOutputFormat(strReportType);

    QString	strReportURL;
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        CGexMoTaskStatus *ptStatusTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetStatusTask();
        if(ptStatusTask)
            strReportURL = ptStatusTask->GetProperties()->reportURL();
    }

    // Report Path = <database>/reports.mo/time_stamp
    // time_stamp format:
    if(strReportURL.isEmpty() || strReportURL.toLower() == "default" || strReportURL.toLower() == "(default)")
    {
        QString strOutputLocation=outputLocation(pReportOptions);
        if (strOutputLocation.isEmpty() || strReportURL.toLower() == "default" || strReportURL.toLower() == "(default)")
        {
            // No Web tesk defined or no custom report folder defined: use default report folder
            strLegacyReportName = pReportOptions->GetServerDatabaseFolder(true);

            // Make sure folder path doesn't end with a '/' or '\\'
            if((strLegacyReportName.endsWith("/") == true) || (strLegacyReportName.endsWith("\\")==true))
                strLegacyReportName.truncate(strLegacyReportName.length()-1);
            cDir.mkpath(strLegacyReportName);
            strLegacyReportName += GEX_DATABASE_REPORTS_MO;
        }
        else
        {
            strLegacyReportName =	strOutputLocation;
            if((strLegacyReportName.endsWith("/") == false) && (strLegacyReportName.endsWith("\\")==false))
                strLegacyReportName += "/";
        }
    }
    else
    {
        // Web status task exists: use report URL
        strLegacyReportName = strReportURL;
        if((strLegacyReportName.endsWith("/") == false) && (strLegacyReportName.endsWith("\\")==false))
            strLegacyReportName += "/";
    }
    setLegacyReportRootFolder(strLegacyReportName);
    cDir.mkdir(m_strLegacyReportRootFolder);

    // Clean-up: erase any empty folder in 'GEX_DATABASE_REPORTS_MO' path.
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteEmptySubFolders(m_strLegacyReportRootFolder);

    // Now create subfolder required for this new report.
    QDateTime cCurrentDateTime = QDateTime::currentDateTime();
    strLegacyReportName += cCurrentDateTime.toString("yyyyMMdd_h_mm_ss.zzz");
    strLegacyReportName += "/";
    // Return report file name created
    setLegacyReportName(strLegacyReportName);

    QString strOutputFormat = outputFormat();
    // Build folder if needed
    if(strOutputFormat!="CSV")
        return cDir.mkdir(reportAbsFilePath());
    else
        return true;
}

void	CGexReport::overloadOutputFormat(QString &strReportType)
{
    // Check if overwrite report output format...Done if caller is Monitoring and custom report to be attached to email notification.
    if (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        if(strReportType.isEmpty() == false)
        {
            m_pReportOptions->SetOption(QString("report"), QString("build"), QString("true"));
        }
        if(strReportType == "excel")
            m_pReportOptions->SetOption("output","format", "CSV");
        if(strReportType == "word")
            m_pReportOptions->SetOption("output","format", "DOC");
        if(strReportType == "ppt")
            m_pReportOptions->SetOption("output","format", "PPT");
        if(strReportType == "pdf")
            m_pReportOptions->SetOption("output","format", "PDF");
        if(strReportType == "odt")
            m_pReportOptions->SetOption("output","format", "ODT");
    }
    else
    {
        QString of=m_pReportOptions->GetOption("output", "format").toString();
        // If Examinator, refuse to execute a script in argument with format not HTML or CSV...
        // so to push Monitoring to be used!
        if(GS::Gex::CSLEngine::GetInstance().IsRunning(GS::Gex::CSLEngine::CSL_ARGUMENT))
        {
            if(	(of!="CSV")
                && (of!="HTML")
                )
                m_pReportOptions->SetOption("output","format", "HTML");
        }
    }

    m_strOutputFormat = ReportOptions.GetOption("output","format").toString();
}




///////////////////////////////////////////////////////////
// Copy necessary default image files in report's folder
///////////////////////////////////////////////////////////
bool CGexReport::legacyLoadImagesReportFolder(
        CReportOptions* pReportOptions,
        QString strFirstFileName,
        bool bCreateReportFolder,
        bool bCreateReportFile,
        bool /*bAllowPopup*/)
{
    QDir		cDir;
    QString		strHtmlPage;	// HTML home page created for ExaminatorWeb CSV file creation.

    // ensure the path ends with a '/'
    if((reportAbsFilePath().endsWith("/") == false) && (reportAbsFilePath().endsWith("\\")==false))
        setLegacyReportName(reportAbsFilePath() + '/');

    QString		strOutputFormat = outputFormat();
    CHtmlPng	cWebResources;
    bool		bStatus;
    FILE		*hCSVHtmlHome=NULL;
    QString		strErrMessage;
    QString		strPagesFolder;
    if (strOutputFormat=="CSV")
    {
        pReportOptions->strReportDirectory = QDir::toNativeSeparators(reportAbsFilePath());
        if(!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
            if(pReportOptions->iFiles == 1)
                setLegacyReportName(reportAbsFilePath() + strFirstFileName);
            else
                setLegacyReportName(reportAbsFilePath() + DEFAULT_REPORT_NAME);
        }
        // Make sure folder path doesn't end with a '/' or '\\'
        if((reportAbsFilePath().endsWith("/") == true) || (reportAbsFilePath().endsWith("\\")==true))
            setLegacyReportName(reportAbsFilePath().left(reportAbsFilePath().length()-1));

        QString strReportAbsFilePathStorage = reportAbsFilePath();
        if(!getGroupsList().isEmpty())
        {
            CGexGroupOfFiles	*pGroupListElement = getGroupsList().first();
            if(!(pGroupListElement->pFilesList.isEmpty()))
            {
                CGexFileInGroup	*pFileListElement = pGroupListElement->pFilesList.first();
                if(pFileListElement->strFileName == (strReportAbsFilePathStorage+QString(".csv")))
                    strReportAbsFilePathStorage += QString("_report");
            }
        }
        setLegacyReportName(strReportAbsFilePathStorage + QString(".csv"));

        // Create HTML home page to CSV (ONLY if ExaminatorWeb)
        if(hCSVHtmlHome != NULL)
        {
            fprintf(hCSVHtmlHome,"<align=\"left\">Your report is ready: <a href=\"%s.csv\">%s.csv</a><br>\n",
                    pReportOptions->strReportNormalizedTitle.toLatin1().constData(),
                    pReportOptions->strReportNormalizedTitle.toLatin1().constData());
            // Close HTML page
            fprintf(hCSVHtmlHome,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
            fprintf(hCSVHtmlHome,"</body>\n");
            fprintf(hCSVHtmlHome,"</html>\n");
            fclose(hCSVHtmlHome);
        }

        hReportFile = fopen(reportAbsFilePath().toLatin1().constData(),"w");
        if(hReportFile == NULL)
        {
            strErrMessage = "Failed to create report file:\n" + reportAbsFilePath();
            strErrMessage += "\n\nPossible cause: Read/Write access issue,\nor a file has the same name as the folder we tried to create.";
            GS::Gex::Message::information("", strErrMessage);
            return false;	// Write error...Stdf::ErrorOpen;
        }
        // Generating .CSV report file.
        fprintf(hReportFile,"******************************************************");
        fprintf(hReportFile,"******************************************************\n");
        fprintf(hReportFile,"* %s - www.mentor.com\n", GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
        fprintf(hReportFile,"* %s. All rights reserved.\n",C_STDF_COPYRIGHT);
        fprintf(hReportFile,"******************************************************");
        fprintf(hReportFile,"******************************************************\n");

        // ExaminatorWeb only: Will display HTML link page to CSV file when report is ready...
        if(hCSVHtmlHome != NULL)
            setLegacyReportName( strHtmlPage );
    }
    else if ( strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PPT"||strOutputFormat=="PDF"||strOutputFormat=="ODT" )
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
            // Make sure folder path doesn't en with a '/' or '\\'
            if((reportAbsFilePath().endsWith("/") == true) || (reportAbsFilePath().endsWith("\\")==true))
                setLegacyReportName(reportAbsFilePath().left(reportAbsFilePath().length()-1));
            // Create folder where report will be created...unless we process data file without building a report!
            if(bCreateReportFolder)
                cDir.mkdir(reportAbsFilePath());
        }
        else
        {
            // If we have only one STDF file...create a folder with his name!...unless we only have to analyze the file, with no report creation!
            if(bCreateReportFolder)
                cDir.mkdir(reportAbsFilePath());
            if(pReportOptions->iFiles == 1)
                setLegacyReportName(reportAbsFilePath() + strFirstFileName);
            else
                setLegacyReportName(reportAbsFilePath() + DEFAULT_REPORT_NAME);
            if(bCreateReportFolder)
                cDir.mkdir(reportAbsFilePath());
        }

        // Keeps location where HTML pages to be created
        strPagesFolder = reportAbsFilePath() + "/pages/";

        pReportOptions->strReportDirectory = QDir::toNativeSeparators(reportAbsFilePath());
        if( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||(strOutputFormat=="ODT") )
            //(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
        {
            // Flat HTML as ALL sections will be written in same HTML flat file prior to convert to WORD or PDF
            setLegacyReportName(reportAbsFilePath() + "/pages/");
            cDir.mkdir(reportAbsFilePath());
            setLegacyReportName(reportAbsFilePath() + "indexf.htm");

            // Also, we ALWAYS generate all the sections
            pReportOptions->iHtmlSectionsToSkip = 0;

            // Define HTML Font size to use in reports.
            if(strOutputFormat=="PPT")	//(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_PPT)
            {
                // PowerPoint tables with Percentage bar (pareto) doesn't hold as many lines as under the other FLAT formats (word, PDF)
                iLinesPercentagePerFlatPage = MAX_PERCENTAGE_LINES_PER_PPT_PAGE;
            }
        }
        else
        {
            // Standard HTML pages to create (with folders, etc...)
            setLegacyReportName(reportAbsFilePath() + "/index.htm");	// Standard report home page
        }

        // If we only analyze data files without creating a Examinator report, then do not copy the HTML files.
        if(bCreateReportFolder)
        {

            GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteFolderContent(strPagesFolder);	// Erase target folder (<report/pages>)

            // Create all images used in the HTML reports.
            bStatus = cWebResources.CreateWebReportImages(pReportOptions->strReportDirectory.toLatin1().constData());
            if(bStatus != true)
            {
                strErrMessage = "Failed to prepare report folder:\n" + pReportOptions->strReportDirectory;
                strErrMessage += "\n\nPossible cause: Read/Write access issue,\nor a file has the same name as the folder we tried to create.";
                GS::Gex::Message::information("", strErrMessage);
                return false;	// Error creating HTML static images
            }

            bStatus = cWebResources.CreateWebReportPages(pReportOptions->strReportDirectory.toLatin1().constData());
            if(bStatus != true)
            {
                strErrMessage = "Failed to prepare report folder:\n" + pReportOptions->strReportDirectory;
                strErrMessage += "\n\nPossible cause: Read/Write access issue,\nor a file has the same name as the folder we tried to create.";
                GS::Gex::Message::information("", strErrMessage);
                return false;	// Error creating HTML static pages
            }

            // If we do not build a custom home page, exit now!
            if(bCreateReportFile)
            {
                // Create HTML file now as ALL sections will be written in same HTML flat file prior to convert to WORD or PDF
                GS::Gex::Engine::RemoveFileFromDisk(reportAbsFilePath());	// Erase target file

                // create target file
                hReportFile = fopen(reportAbsFilePath().toLatin1().constData(),"w");
                if(hReportFile == NULL)
                {
                    strErrMessage = "Failed to create report file:\n" + reportAbsFilePath();
                    strErrMessage += "\n\nPossible cause: Read/Write access issue,\nor a file has the same name as the folder we tried to create.";
                    GS::Gex::Message::information("", strErrMessage);
                    return false;	// Write error...Stdf::ErrorOpen;
                }
            }
        }
    }

    // success
    return true;
}

///////////////////////////////////////////////////////////
// Build report name + create necessary folders & copy files!
///////////////////////////////////////////////////////////
bool	CGexReport::legacyBuildReportFolderName(CReportOptions *pReportOptions, QString strReportType)
{
    char				szString[2048];
    CGexGroupOfFiles	*pGroup;
    CGexFileInGroup		*pFile;

    // Refresh Output report format (if needed)
    overloadOutputFormat(strReportType);

    QString strOutputFormat = outputFormat();

    // See if we create the report file or not...
    bool	bCreateReportFolder=false;
    bool	bCreateReportFile=false;
    {
        // Create file only if report has to be created.
        bCreateReportFolder = (pReportOptions->GetOption(QString("report"), QString("build"))).toBool();

        // If we create a CSV file, or a flat HTML report, we now have to create + open the file to write in.
        // Else (if Interactive or standard HTML), we don't create the file yet.
        if(	(strOutputFormat=="CSV") //(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            || ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
            )
            bCreateReportFile = true;	// We'll have to open & create the file when creating
        else
            bCreateReportFile = false;
    }

    // Get name of first file STDF file to process...
    QString strFirstFileName("");
    char	*ptChar;
    ptChar = NULL;
    // Rewind to first group
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first(); //pGroup = getGroupsList().first();
    if(pGroup!=NULL)
    {
        pFile = (pGroup->pFilesList).first();
        strcpy(szString,pFile->strFileNameSTDF.toLatin1().constData());
        ptChar = FindExt(szString);
        if(ptChar != NULL)
            *ptChar = 0;	// Remove extension
        ptChar = FindPath(szString);
        if(ptChar != NULL)
        {
            *ptChar = 0;	// so 'szstring' only has path to STDF file.
            strFirstFileName = (ptChar+1);	// Save file name.
            // If Report name includes a '#'...then replace it with a '-' to avoid HTML bookmark issue.
            int iIndex;
            iIndex = strFirstFileName.indexOf('#');
            while(iIndex >= 0)
            {
                strFirstFileName = strFirstFileName.replace(iIndex,1,"-");
                // Look for next '#' character...
                iIndex = strFirstFileName.indexOf('#');
            };
        }
        else
            strFirstFileName = DEFAULT_REPORT_NAME;
    }
    else
    {
        // No Group defined: only valid for Enterprise report or Reports Center
        if( (QFile::exists(pReportOptions->strTemplateFile) == false)
            && (QFile::exists(pReportOptions->strReportCenterTemplateFile) == false) )
        {
            GSLOG(SYSLOG_SEV_ERROR, "No group defined and no report template!");
            return false;	// Fatal error: no group defined, and not template report!
        }
        // Ensure building name for Query request
        pReportOptions->bQueryDataset = true;
    }

    // Special case for Monitoring: if the output path is forced, then ignore default folder creation Path = <database>/reports.mo/time_stamp
    QDir cDir;
    bool bAllowPopup = true;

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // under monitoring, do not popup error message as it would hang the process!
        bAllowPopup = false;
    }

    // QString strOutputLocation = ReportOptions.GetOption("output", "location").toString();
    QString strOutputLocation = outputLocation(pReportOptions);

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        CreateDatabaseReportFolder(pReportOptions, m_strLegacyReportAbsFilePath,strReportType);
    }
    else if(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() ||
            GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
            GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
    {
        // Check if Dataset source results from inserting files (not a Query).
        if(pReportOptions->bQueryDataset == false)
        {
            // Report results from processing files, not a query...then report has to be in the relevant folder
            m_strLegacyReportAbsFilePath = strOutputLocation;

            if((m_strLegacyReportAbsFilePath == "default") || (m_strLegacyReportAbsFilePath == "(default)"))
            {
                // Use the default <Examinator>/databases one !
                m_strLegacyReportAbsFilePath = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
            }

            // If no path specified: we must use the path of the first Data file
            if(m_strLegacyReportAbsFilePath.isEmpty() == true)
                m_strLegacyReportAbsFilePath = szString;	// STDF full path
            if(m_strLegacyReportAbsFilePath.endsWith("/") == false)
                m_strLegacyReportAbsFilePath += "/";
            cDir.mkdir(m_strLegacyReportAbsFilePath);

            // If Examinator-PAT, ensure report root name ALWAYS start with first test file name (needed for cosistency when making multi-sote file audit: report needs to use file name instead of 'exminator-report')
            if(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
            {
                m_strLegacyReportAbsFilePath += strFirstFileName;
                if(pReportOptions->iFiles > 1)
                    m_strLegacyReportAbsFilePath += QString("_") + DEFAULT_REPORT_NAME;
            }
            else
            {
                if(pReportOptions->iFiles == 1)
                    m_strLegacyReportAbsFilePath += strFirstFileName;
                else
                    m_strLegacyReportAbsFilePath += DEFAULT_REPORT_NAME;
            }

            // Create folder where report will be created...unless we've got a CSV report to create.
            if(bCreateReportFolder)
            {
                if(	(strOutputFormat!="CSV")			//(ReportOptions.iOutputFormat != GEX_OPTION_OUTPUT_CSV)
                    && (strOutputFormat!="INTERACTIVE")	//(ReportOptions.iOutputFormat != GEX_OPTION_OUTPUT_INTERACTIVEONLY)
                    )
                    cDir.mkdir(m_strLegacyReportAbsFilePath);
            }
        }
        else
        {

            // Dataset results from running a query...then create report in the database reports folder.
            if((strOutputLocation.isEmpty() == true) || (strOutputLocation == "default") || (strOutputLocation == "(default)"))
            {
                // Use default path: <ExaminatorPath>/reports
                m_strLegacyReportAbsFilePath = pReportOptions->GetLocalDatabaseFolder();	//ReportOptions.strDatabasesLocalPath;

                // Make sure folder path doesn't end with a '/' or '\\'
                if((m_strLegacyReportAbsFilePath.endsWith("/") == true) || (m_strLegacyReportAbsFilePath.endsWith("\\")==true))
                    m_strLegacyReportAbsFilePath = m_strLegacyReportAbsFilePath.left(m_strLegacyReportAbsFilePath.length()-1);
                cDir.mkpath(m_strLegacyReportAbsFilePath);
                m_strLegacyReportAbsFilePath += GEX_DATABASE_REPORTS;
                cDir.mkdir(m_strLegacyReportAbsFilePath);
                // Sub folder is user node name (if specified, usually: computer name)
                m_strLegacyReportAbsFilePath +=
                        GS::Gex::Engine::GetInstance().GetSystemInfo().strHostName;
                if(m_strLegacyReportAbsFilePath.endsWith("/") == false)
                    m_strLegacyReportAbsFilePath += "/";
                cDir.mkdir(m_strLegacyReportAbsFilePath);
            }
            else
            {
                m_strLegacyReportAbsFilePath = strOutputLocation;
                    cDir.mkdir(m_strLegacyReportAbsFilePath);
                }

            // ensure the path ends with a '/'
            if((m_strLegacyReportAbsFilePath.endsWith("/") == false) && (m_strLegacyReportAbsFilePath.endsWith("\\")==false))
                m_strLegacyReportAbsFilePath += '/';

            // add query title specified, as the sub-folder name...unless we generate a CSV file...
            if(pReportOptions->strReportNormalizedTitle.isEmpty() == false)
                m_strLegacyReportAbsFilePath += pReportOptions->strReportNormalizedTitle;
        }
    }
    else
    {
        m_strLegacyReportAbsFilePath =	strOutputLocation;
        // If no path specified: we must use the path of the first Data file
        if(m_strLegacyReportAbsFilePath.isEmpty() == true)
            m_strLegacyReportAbsFilePath = szString;		// STDF full path
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Load images into the folder %1...")
          .arg(m_strLegacyReportAbsFilePath).toLatin1().constData());

    // Load images into the fodler
    bool bStatus = legacyLoadImagesReportFolder(pReportOptions, strFirstFileName,bCreateReportFolder,bCreateReportFile,bAllowPopup);

    GSLOG(SYSLOG_SEV_DEBUG, QString("Build folder name... DONE").toLatin1().constData());

    return bStatus;
}


QString CGexReport::buildLegacyMonitoringReportName(QString &strReportName, const QString &strExtension)
{
    CGexGroupOfFiles *pGroup=0;
    CGexFileInGroup *pFile=0;
    int iFolDerPath = 0;
    pGroup= getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if (!pGroup)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Group null");
        return strReportName;
    }
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
       && m_pReportOptions->iFiles == 1 && pGroup != NULL && pFile != NULL)
    {
        // Monitoring creates a report with the name <parttype_lot_date_time>...ONLY if we analyzed a single file!
        GSLOG(SYSLOG_SEV_DEBUG, QString("ReportName = %1").arg( strReportName).toLatin1().constData());
        iFolDerPath = strReportName.lastIndexOf('/');
        if(!(iFolDerPath < 0))
        {
            strReportName.truncate(iFolDerPath+1);

            strReportName += pFile->getMirDatas().szPartType;
            strReportName += "_";
            strReportName += pFile->getMirDatas().szLot;

            strReportName += QDateTime::currentDateTime().toString("_yyyyMMdd_h_mm_ss.zzz");
            strReportName += strExtension;
        }
    }

    return strReportName;
}

///////////////////////////////////////////////////////////
// Set the report generation mode: legacy or normalized
///////////////////////////////////////////////////////////
#ifdef GEX_NO_JS_SOLARIS
void CGexReport::setReportGenerationMode(const QString& )
{
    m_strReportGenerationMode = "legacy";
}
#else
void CGexReport::setReportGenerationMode(const QString &strGenerationMode)
{
    if (strGenerationMode.toLower() == "legacy")
        m_strReportGenerationMode = strGenerationMode.toLower();
    else if (strGenerationMode.toLower() == "normalized")
        m_strReportGenerationMode = strGenerationMode.toLower();
    else
    {
        QString strErrorMsg = "Error: undefined generation mode: " + strGenerationMode;
        GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
    }
}
#endif

bool CGexReport::WriteToReportFile(QString s)
{
    if (!getReportFile())
        return false;
    fprintf(getReportFile(), "%s", s.toLatin1().data());
    return true;
}

QVariant CGexReport::GetOption(QString s, QString o)
{
    if (!m_pReportOptions)
        return QVariant();
    return m_pReportOptions->GetOption(s,o);
}

///////////////////////////////////////////////////////////
// Return report name without extension
///////////////////////////////////////////////////////////
QString CGexReport::reportName()
{
    QString strReportName = "";

    // If have to be overloaded
    if (!m_strEvaluatedReportName.isEmpty())
        strReportName = m_strEvaluatedReportName;

    if (strReportName.isEmpty())
        strReportName = reportType();

    return strReportName;
}

///////////////////////////////////////////////////////////
// Return report name with extension
///////////////////////////////////////////////////////////
QString CGexReport::reportFileName()
{
    if (!m_pReportOptions)
    {
        GSLOG(SYSLOG_SEV_ERROR, "No report option available!");
        return "undefined";
    }

    bool isInServerMode = GS::LPPlugin::ProductInfo::getInstance()->isMonitoring();
    if(!isInServerMode && m_bUserAlreadyAsked && !m_strUserDefRepotFileName.isEmpty())
        return m_strUserDefRepotFileName;

    // If legacy mode
    if (reportGenerationMode() == "legacy")
        return m_strLegacyReportAbsFilePath;

    QString strReportFileName = "";
    QString strReportName = "";
    QString strOutputFormat = "";

    strReportName = reportName();
    strOutputFormat = outputFormat();

    if (strOutputFormat == "HTML")
    {
        strReportFileName = strReportName + QDir::separator();
        // my report or reports center
        if (!m_pReportOptions->strReportCenterTemplateFile.isEmpty()
            || !m_pReportOptions->strTemplateFile.isEmpty())
            strReportFileName += QString("pages") + QDir::separator();
        strReportFileName += "index.htm";
    }
    else if( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
    {
        strReportFileName = strReportName + "." + strOutputFormat.toLower();
    }
    else if (strOutputFormat == "CSV")
    {
        strReportFileName = strReportName + ".csv";
    }

    QString hypotheticFilePath =
            reportFolderAbsPath() + QDir::separator() + strReportFileName;
    if (! isInServerMode &&
        (strOutputFormat != "HTML" &&
         strOutputFormat != "INTERACTIVE") && ! m_bUserAlreadyAsked)
    {
        bool bRes = checkIfReportExist(hypotheticFilePath,
                                       m_strUserDefRepotFileName);
        m_bUserAlreadyAsked = true;
        if (bRes)
        {
            return m_strUserDefRepotFileName;
        }
    }
    return strReportFileName;
}

///////////////////////////////////////////////////////////
// Return flat HTML absolute path (only for doc,pdf,ppt)
///////////////////////////////////////////////////////////
QString CGexReport::reportFlatHtmlAbsPath()
{
    // If legacy mode
    if (reportGenerationMode() == "legacy")
        return m_strLegacyReportAbsFilePath;

    QString strOutputFormat = "";
    QString strReportFlatHtmlAbsPath = "";

    strOutputFormat = outputFormat();
    if ((strOutputFormat=="DOC") || (strOutputFormat=="PDF") || (strOutputFormat=="PPT") || strOutputFormat=="ODT")
    {
        strReportFlatHtmlAbsPath = QDir::cleanPath(reportPagesFolderAbsPath() + QDir::separator() + "indexf.htm");
    }

    return strReportFlatHtmlAbsPath;
}

///////////////////////////////////////////////////////////
// Return pages folder absolute path
///////////////////////////////////////////////////////////
QString CGexReport::reportPagesFolderAbsPath()
{
    return QDir::cleanPath(reportFolderAbsPath() + QDir::separator() + reportName() + QDir::separator() + "pages");
}

///////////////////////////////////////////////////////////
// Return images folder absolute path
///////////////////////////////////////////////////////////
QString CGexReport::reportImagesFolderAbsPath()
{
    return QDir::cleanPath(reportFolderAbsPath() + QDir::separator() + reportName() + QDir::separator() + "images");
}

///////////////////////////////////////////////////////////
// Return report file absolute path
///////////////////////////////////////////////////////////
QString CGexReport::reportAbsFilePath()
{
    QString strAbsoluteFilePath;

    // If legacy mode
    if (reportGenerationMode() == "legacy")
        strAbsoluteFilePath = m_strLegacyReportAbsFilePath;
    else
    {
        strAbsoluteFilePath = reportFolderAbsPath() + QDir::separator() + reportFileName();
        strAbsoluteFilePath = QDir::cleanPath(strAbsoluteFilePath);
    }

    // LE FIX DE LA MORT QUI TUE LA MORT
    // Without following trick, the step-by-step mode in debug is corrupted!!
    // Even if replacing the whole function with just {QString strAbsoluteFilePath="legacy"; return strAbsoluteFilePath;}
    QChar * pChar = strAbsoluteFilePath.data();
    return QString(pChar);
}

///////////////////////////////////////////////////////////
// Return report file relative path (relative to the root folder)
///////////////////////////////////////////////////////////
QString CGexReport::reportRelativeFilePath()
{
    if (!m_strEvaluatedReportRelativeFilePath.isEmpty())
        return m_strEvaluatedReportRelativeFilePath;

    if (reportGenerationMode() == "legacy")
    {
        if ((!m_pReportOptions->strReportCenterTemplateFile.isEmpty()
             || !m_pReportOptions->strTemplateFile.isEmpty())
            && (outputFormat() == "HTML"))
            m_strEvaluatedReportRelativeFilePath = reportAbsFilePath().
                                                   remove(0, QDir::cleanPath(reportRootFolderAbsPath()).size() - QDir(reportRootFolderAbsPath()).dirName().size());
        else
            m_strEvaluatedReportRelativeFilePath = reportAbsFilePath().remove(0, QDir::cleanPath(reportRootFolderAbsPath()).size() + 1);
    }
    else
    {
        m_strEvaluatedReportRelativeFilePath = reportAbsFilePath().remove(0, QDir::cleanPath(reportRootFolderAbsPath()).size() + 1);
    }

    return m_strEvaluatedReportRelativeFilePath;
}

///////////////////////////////////////////////////////////
// Return report folder absolute path
///////////////////////////////////////////////////////////
QString CGexReport::reportFolderAbsPath()
{
    // If legacy mode
    if (reportGenerationMode() == "legacy")
        return m_strLegacyReportRootFolder;

    QString strAbsoluteFolderPath = reportRootFolderAbsPath() + QDir::separator() + reportSubFolders();
    strAbsoluteFolderPath = QDir::cleanPath(strAbsoluteFolderPath);

    return strAbsoluteFolderPath;
}

///////////////////////////////////////////////////////////
// Return report subfolders
///////////////////////////////////////////////////////////
QString CGexReport::reportSubFolders()
{
    QString strSubFolders = "";

    // If it has already been evaluated, give it
    if (!m_strEvaluatedReportSubFolders.isEmpty())
        strSubFolders = m_strEvaluatedReportSubFolders;

    return strSubFolders;
}

///////////////////////////////////////////////////////////
// Return report root folder absolute path
///////////////////////////////////////////////////////////
QString CGexReport::reportRootFolderAbsPath()
{
    if (!m_pReportOptions)
    {
        GSLOG(SYSLOG_SEV_ERROR, "No report option available!");
        return "undefined";
    }

    // If legacy mode
    if (reportGenerationMode() == "legacy")
        return m_strLegacyReportRootFolder;

    CGexGroupOfFiles	*pGroup = NULL;
    CGexFileInGroup		*pFile = NULL;
    QString				strAbsFileFolderPath = "";
    QString				strOutputLocation = "";
    QString				strReportRootFolderAbsPath = "";

    // If it has already been evaluated, give it
    if (!m_strEvaluatedReportRootFolderAbsPath.isEmpty())
        return m_strEvaluatedReportRootFolderAbsPath;

    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first(); //pGroup = getGroupsList().first();
    if(pGroup)
    {
        pFile = pGroup->pFilesList.isEmpty()?NULL:pGroup->pFilesList.first();
        if(pFile)
            strAbsFileFolderPath = QDir::cleanPath(QFileInfo(pFile->strFileNameSTDF).absoluteDir().path());
    }

    if (!pGroup || !pFile)
    {
        // No Group defined: only valid for Enterprise report or Reports Center
        if(QFile::exists(m_pReportOptions->strTemplateFile) == false
           && QFile::exists(m_pReportOptions->strReportCenterTemplateFile) == false)
        {
            // Fatal error: no group defined, and not template report!
            if (!pGroup)
                GSLOG(SYSLOG_SEV_ERROR, "pGroup NULL!");
            if (!pFile)
                GSLOG(SYSLOG_SEV_ERROR, "pFile NULL!");
        }
        // Ensure building name for Query request
        m_pReportOptions->bQueryDataset = true;
    }

    // If not in legacy mode root folder depend on if the report is manually launch or automatically

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // Auto report
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            CGexMoTaskStatus *ptStatusTask = GS::Gex::Engine::GetInstance().GetSchedulerEngine().GetStatusTask();
            if(ptStatusTask)
            {
                strReportRootFolderAbsPath = ptStatusTask->GetProperties()->reportURL();
                strOutputLocation = strReportRootFolderAbsPath;
            }
        }

        if (strOutputLocation.isEmpty() || strOutputLocation == "default" || strOutputLocation == "(default)")
            strReportRootFolderAbsPath = workingFolder() + QDir::separator() + "reports";

    } else
    {
        // Manual report
        strOutputLocation = outputLocation(m_pReportOptions);
        // Check if Dataset source results from inserting files (not a Query).
        if (strOutputLocation.isEmpty() && m_pReportOptions->bQueryDataset == false)
            strReportRootFolderAbsPath = strAbsFileFolderPath;
        else if ((strOutputLocation == "default") || (strOutputLocation == "(default)") || (strOutputLocation.isEmpty() && m_pReportOptions->bQueryDataset == true))
            strReportRootFolderAbsPath = workingFolder() + QDir::separator() + "reports";
        else
            strReportRootFolderAbsPath = strOutputLocation;
    }

    strReportRootFolderAbsPath = QDir::cleanPath(strReportRootFolderAbsPath);

    return strReportRootFolderAbsPath;
}

///////////////////////////////////////////////////////////
// Return report type, used to build default report name
///////////////////////////////////////////////////////////
QString CGexReport::reportType()
{
    if (!m_pReportOptions)
    {
        GSLOG(SYSLOG_SEV_WARNING, "No report option available!");
        return "undefined";
    }

    if (!m_strEvaluatedReportType.isEmpty())
        return m_strEvaluatedReportType;

    QString strReportType = "undefined";

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // Auto report
        strReportType = "ym_report_" + QDateTime::currentDateTime().toString("yyyyMMdd_h_mm_ss.zzz");
    }
    else
    {
        // If query over data set add prefix "db_"
        if (m_pReportOptions->bQueryDataset)
            strReportType = "db_";
        else
            strReportType = "";

        // Analyze or merge
        if (getGroupsList().count() == 1)
        {
            if(getGroupsList().first()->pFilesList.count() == 1 || m_pReportOptions->bQueryDataset)
            {
                strReportType += "analyze_report";
                if (!m_pReportOptions->bQueryDataset)
                    strReportType += QString("_") + QFileInfo(getGroupsList().first()->pFilesList.first()->strFileNameSTDF).baseName();
            }
            else
                strReportType += "merge_report";
        }
        // Compare or compare groups
        else
        {
            strReportType += "compare";
            QListIterator<CGexGroupOfFiles*> iter(getGroupsList());

            while (iter.hasNext() && strReportType == "compare" && !m_pReportOptions->bQueryDataset)
            {
                if (iter.next()->pFilesList.count() > 1)
                    strReportType += "_groups";
            }
            strReportType += "_report";
        }
        if (m_pReportOptions->bQueryDataset)
            strReportType += QString("_") + m_pReportOptions->strReportNormalizedTitle;

        if (!m_pReportOptions->strReportCenterTemplateFile.isEmpty())
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("Report type: RC: %1").arg(m_pReportOptions->strReportCenterTemplateFile).toLatin1().constData());
            strReportType = "rc_report";
        }
        else if (m_pReportOptions->getAdvancedReport() == GEX_ADV_TEMPLATE
                 && !m_pReportOptions->strTemplateFile.isEmpty())
        {
            strReportType = "my_report_" + m_pReportOptions->strReportNormalizedTitle;
        }
        else if(m_pReportOptions->getAdvancedReport() == GEX_ADV_REPORT_BUILDER
                && !m_pReportOptions->strTemplateFile.isEmpty())
        {
            if (!m_pReportOptions->strReportNormalizedTitle.isEmpty())
            {
                strReportType = "report_builder_" + m_pReportOptions->strReportNormalizedTitle;
            }
            else
            {
                strReportType = "Quantix_Report_Builder_"
                                + QDate::currentDate().toString("yyyy-MM-dd")
                                + "@" + QDateTime::currentDateTime().toString("HHmmss");
            }
        }
    }

    m_strEvaluatedReportType = strReportType;
    return strReportType;
}

///////////////////////////////////////////////////////////
// Return report generation mode
///////////////////////////////////////////////////////////
QString CGexReport::reportGenerationMode()
{
#ifdef GEX_NO_JS_SOLARIS
    m_strReportGenerationMode = "legacy";
#else
    if (m_strReportGenerationMode.isEmpty() && m_pReportOptions)
        m_strReportGenerationMode = m_pReportOptions->GetOption("output", "generation_mode").toString().toLower();

    if ((m_strReportGenerationMode != "legacy") && (m_strReportGenerationMode != "normalized"))
    {
        GSLOG(SYSLOG_SEV_WARNING, "No valid generation mode, default value set to LEGACY");
        m_strReportGenerationMode = "legacy";
    }
#endif

    // LE FIX DE LA MORT QUI TUE LA MORT
    // Without following trick, the step-by-step mode in debug is corrupted!!
    // Even if replacing the whole function with just {QString strReportGenerationMode="legacy"; return strReportGenerationMode;}
    QChar * pChar = m_strReportGenerationMode.data();
    return QString(pChar);
}


///////////////////////////////////////////////////////////
// Return report output format
///////////////////////////////////////////////////////////
QString	CGexReport::outputFormat()
{
    if (m_strOutputFormat.isEmpty())
        m_strOutputFormat = m_pReportOptions->GetOption("output", "format").toString();

    return m_strOutputFormat;
}


///////////////////////////////////////////////////////////
// Add global object to QScript engine
// To use it we will have to read MIR data when adding the
// file to the group
///////////////////////////////////////////////////////////
bool CGexReport::setQScriptEngineProperties()
{
    if (!m_pScriptEngine)
        return false;

    CGexGroupOfFiles *pGroup = 0;
    CGexFileInGroup *pFile = 0;
    QString strProductId, strLotId, strWaferId, strSubLotId, strProgName,
            strTesterName, strTesterType, strFacilityId, strPackageType,
            strFamilyId;

    strProductId = strLotId = strWaferId = strSubLotId = strProgName =
            strTesterName = strTesterType = strFacilityId = strPackageType =
            strFamilyId = "";

    foreach(pGroup, getGroupsList())
    {
        if(pGroup)
        {
            foreach(pFile, pGroup->pFilesList)
            {
                if (pFile)
                {
                    // Set product ID
                    if (!strProductId.isEmpty() && strProductId != QString(pFile->getMirDatas().szPartType))
                        strProductId = "MultiProduct";
                    else
                        strProductId = QString(pFile->getMirDatas().szPartType);
                    // Set lot ID
                    if (!strLotId.isEmpty() && strLotId != QString(pFile->getMirDatas().szLot))
                        strLotId= "MultiLot";
                    else
                        strLotId = QString(pFile->getMirDatas().szLot);
                    // Set wafer ID
                    if ((pFile->getMirDatas().m_lstWaferID.count() != 1) ||
                        (!strWaferId.isEmpty() && strWaferId != pFile->getMirDatas().m_lstWaferID.first()))
                        strWaferId = "MultiWafer";
                    else
                        strWaferId = pFile->getMirDatas().m_lstWaferID.first();
                    // Set sublot ID
                    if (!strSubLotId.isEmpty() && strSubLotId != QString(pFile->getMirDatas().szSubLot))
                        strSubLotId = "MultiSubLot";
                    else
                        strSubLotId = QString(pFile->getMirDatas().szSubLot);
                    // Set programme name
                    if (!strProgName.isEmpty() && strProgName != QString(pFile->getMirDatas().szJobName))
                        strProgName = "MultiProgramme";
                    else
                        strProgName = QString(pFile->getMirDatas().szJobName);
                    // Set tester name
                    if (!strTesterName.isEmpty() && strTesterName != QString(pFile->getMirDatas().szNodeName))
                        strTesterName = "MultiTesterName";
                    else
                        strTesterName = QString(pFile->getMirDatas().szNodeName);
                    // Set tester type
                    if (!strTesterType.isEmpty() && strTesterType != QString(pFile->getMirDatas().szTesterType))
                        strTesterType = "MultiTesterType";
                    else
                        strTesterType = QString(pFile->getMirDatas().szTesterType);
                    // Set facility ID
                    if (!strFacilityId.isEmpty() && strFacilityId != QString(pFile->getMirDatas().szFacilityID))
                        strFacilityId = "MultiFacility";
                    else
                        strFacilityId = QString(pFile->getMirDatas().szFacilityID);
                    // Set package type
                    if (!strPackageType.isEmpty() && strPackageType != QString(pFile->getMirDatas().szPkgType))
                        strPackageType = "MultiPackage";
                    else
                        strPackageType = QString(pFile->getMirDatas().szPkgType);
                    // Set family ID
                    if (!strFamilyId.isEmpty() && strFamilyId != QString(pFile->getMirDatas().szFamilyID))
                        strFamilyId = "MultiFamiliy";
                    else
                        strFamilyId = QString(pFile->getMirDatas().szFamilyID);
                }
            }
        }
    }

    // Set product ID
    m_pScriptEngine->globalObject().setProperty("$PRODUCT_ID",strProductId);
    // Set lot ID
    m_pScriptEngine->globalObject().setProperty("$LOT_ID",strLotId);
    // Set wafer ID
    m_pScriptEngine->globalObject().setProperty("$WAFER_ID",strWaferId);
    // Set sublot ID
    m_pScriptEngine->globalObject().setProperty("$SUBLOT_ID",strSubLotId);
    // Set programme name
    m_pScriptEngine->globalObject().setProperty("$PROGRAM_NAME",strProgName);
    // Set tester name
    m_pScriptEngine->globalObject().setProperty("$TESTER_NAME",strTesterName);
    // Set tester type
    m_pScriptEngine->globalObject().setProperty("$TESTER_TYPE",strTesterType);
    // Set facility ID
    m_pScriptEngine->globalObject().setProperty("$FACILITY_ID",strFacilityId);
    // Set package type
    m_pScriptEngine->globalObject().setProperty("$PACKAGE_TYPE",strPackageType);
    // Set family ID
    m_pScriptEngine->globalObject().setProperty("$FAMILY_ID",strFamilyId);

    return true;
}

///////////////////////////////////////////////////////////
// Evaluate if needed report name, folder and subfolder
///////////////////////////////////////////////////////////
QString CGexReport::evaluateReportJsAttributes()
{
    QString strErrorMsg = "";
    m_strEvaluatedReportName = "";
    m_strEvaluatedReportSubFolders = "";
    m_strEvaluatedReportRootFolderAbsPath = "";

    //	setQScriptEngineProperties();

    // Evaluates file name
    if (m_strJsReportName.isEmpty())
        m_strEvaluatedReportName = jsToFileName(m_pReportOptions->GetOption("output", "file_name").toString(), strErrorMsg);
    else
        m_strEvaluatedReportName = jsToFileName(m_strJsReportName, strErrorMsg);

    // Evaluates sub folders
    if (m_strJsReportSubFolders.isEmpty())
        m_strEvaluatedReportSubFolders = jsToSubFolders(m_pReportOptions->GetOption("output", "sub_folders").toString(), strErrorMsg);
    else
        m_strEvaluatedReportSubFolders = jsToSubFolders(m_strJsReportSubFolders, strErrorMsg);

    // Evaluates root folder
    if (m_strJsReportRootFolderAbsPath.isEmpty())
        m_strEvaluatedReportRootFolderAbsPath = jsToRootFolderPath(m_pReportOptions->GetOption("output", "root_folder").toString(), strErrorMsg);
    else
        m_strEvaluatedReportRootFolderAbsPath = jsToRootFolderPath(m_strJsReportRootFolderAbsPath, strErrorMsg);

    return strErrorMsg;
}

///////////////////////////////////////////////////////////
// Return evaluated file name:
// - if the JS expression is valid file name and error msg is empty
// - else "" and error msg contains msg
///////////////////////////////////////////////////////////
QString	CGexReport::jsToFileName(const QString &strJSExpression, QString &strErrorMsg)
{
    if (!m_pScriptEngine || strJSExpression.isEmpty())
        return QString();
    QString strFileName = "";
    if (m_pScriptEngine->canEvaluate(strJSExpression))
        strFileName = m_pScriptEngine->evaluate(strJSExpression).toString();

    if (m_pScriptEngine->hasUncaughtException())
    {
        strErrorMsg = "Error evaluating: " + strJSExpression;
        GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
        // GCORE-499
        m_pScriptEngine->collectGarbage();
        return QString();
    }

    // Remove windows forbiden char
    strFileName.remove(QRegExp("[\\/:*?<>|]"));
    // Begins with space,-,dot
    strFileName.remove(QRegExp("^[ -.]+"));
    // Ends with space,-,dot
    strFileName.remove(QRegExp("[ -.]+$"));

    return strFileName;
}

///////////////////////////////////////////////////////////
// Return evaluated sub folder:
// - if the JS expression is valid file name and error msg is empty
// - else "" and error msg contains msg
///////////////////////////////////////////////////////////
QString	CGexReport::jsToSubFolders(const QString &strJSExpression, QString &strErrorMsg)
{
    if (!m_pScriptEngine || strJSExpression.isEmpty())
        return QString();
    QString strSubFolders = "";
    if (m_pScriptEngine->canEvaluate(strJSExpression))
        strSubFolders = m_pScriptEngine->evaluate(strJSExpression).toString();

    if (m_pScriptEngine->hasUncaughtException())
    {
        strErrorMsg = "Error evaluating: " + strJSExpression;
        GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
        return QString();
    }

    // Remove windows forbiden char
    strSubFolders.remove(QRegExp("[:*?<>|]"));
    // Begins with space,-,dot
    strSubFolders.remove(QRegExp("^[ -.]+"));
    // Ends with space,-,dot
    strSubFolders.remove(QRegExp("[ -.]+$"));

    return strSubFolders;
}

///////////////////////////////////////////////////////////
// Return evaluated folder path:
// - if the JS expression is valid file name and error msg is empty
// - else "" and error msg contains msg
///////////////////////////////////////////////////////////
QString CGexReport::jsToRootFolderPath(const QString &strJSExpression, QString &strErrorMsg)
{
    if (!m_pScriptEngine || strJSExpression.isEmpty())
        return QString();
    QString strFolderAbsPath = "";

    if (m_pScriptEngine->canEvaluate(strJSExpression))
        strFolderAbsPath = m_pScriptEngine->evaluate(strJSExpression).toString();

    if (m_pScriptEngine->hasUncaughtException())
    {
        strErrorMsg = "Error evaluating: " + strJSExpression;
        GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
        // GCORE-499
        m_pScriptEngine->collectGarbage();
        return QString();
    }

    // Remove windows forbiden char
    strFolderAbsPath.remove(QRegExp("[*?<>|]"));
    // Begins with space,-,dot
    strFolderAbsPath.remove(QRegExp("^[ -.]+"));
    // Ends with space,-,dot
    strFolderAbsPath.remove(QRegExp("[ -.]+$"));

    return strFolderAbsPath;
}



///////////////////////////////////////////////////////////
// Define report root folder, sub-folders, report file name
///////////////////////////////////////////////////////////
bool CGexReport::buildReportArborescence(CReportOptions *pReportOptions,QString strReportType)
{
    // If legacy mode
    if (reportGenerationMode() == "legacy")
        return legacyBuildReportFolderName(pReportOptions,strReportType);

    QString strErrorMsg = evaluateReportJsAttributes();
    if (!strErrorMsg.isEmpty())
        GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
    // Refresh Output report format (if needed)
    overloadOutputFormat(strReportType);

    QString strAbsoluteFolderPath = "";
    QString strOutputFormat = outputFormat();
    CGexGroupOfFiles	*pGroup;


    bool bAllowPopup = true;
    if (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        // Also: under monitoring, do not popup error message as it would hang the process!
        bAllowPopup = false;

    // See if we create the report file or not...
    bool	bCreateReportFolder=false;
    bool	bCreateReportFile=false;

    // Create file only if report has to be created.
    bCreateReportFolder = (pReportOptions->GetOption(QString("report"), QString("build"))).toBool();

    // If we create a CSV file, or a flat HTML report, we now have to create + open the file to write in.
    // Else (if Interactive or standard HTML), we don't create the file yet.
    if ( (strOutputFormat=="CSV") || ((strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||(strOutputFormat=="ODT") ))
        bCreateReportFile = true;	// We'll have to open & create the file when creating
    else
        bCreateReportFile = false;

    // Rewind to first group
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first(); //pGroup = getGroupsList().first();
    if(pGroup == NULL)
    {
        // No Group defined: only valid for Enterprise report or Reports Center
        if(QFile::exists(pReportOptions->strTemplateFile) == false
           && QFile::exists(pReportOptions->strReportCenterTemplateFile) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "No group defined, and no report template!");
            return false;	// Fatal error: no group defined, and not template report!
        }
        // Ensure building name for Query request
        pReportOptions->bQueryDataset = true;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Root Folder: %1").arg(reportRootFolderAbsPath()).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Sub-Folder: %1").arg(reportSubFolders()).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("File Name: %1").arg(reportFileName().replace('%', "%%")).toLatin1().constData());

    strAbsoluteFolderPath = reportFolderAbsPath();
    QDir root(strAbsoluteFolderPath);
    if (!root.exists())
        root.mkpath(strAbsoluteFolderPath);

    GSLOG(SYSLOG_SEV_DEBUG, QString("Load images into the folder %1...")
          .arg(reportFolderAbsPath()).toLatin1().constData());

    // Load images into the fodler
    bool bStatus = loadImagesReportFolder(pReportOptions,bCreateReportFolder,bCreateReportFile,bAllowPopup);

    GSLOG(SYSLOG_SEV_DEBUG, "DONE");

    return bStatus;
}

///////////////////////////////////////////////////////////
// // Load default images into report's folder.
///////////////////////////////////////////////////////////
bool CGexReport::loadImagesReportFolder(
        CReportOptions* pReportOptions,
        bool bCreateReportFolder,
        bool bCreateReportFile,
        bool /*bAllowPopup*/)
{
    QString		strOutputFormat = outputFormat();
    CHtmlPng	cWebResources;
    bool		bStatus;
    QString		strErrMessage;

    if (strOutputFormat=="CSV")
    {
        pReportOptions->strReportDirectory = reportFolderAbsPath();

        // Create CSV file
        hReportFile = fopen(reportAbsFilePath().toLatin1().constData(),"w");
        if (hReportFile == NULL)
        {
            strErrMessage = "Failed to create report file:\n" + reportAbsFilePath();
            strErrMessage += "\n\nPossible cause: Read/Write access issue,\nor a file has the same name as the folder we tried to create.";
            GS::Gex::Message::information("", strErrMessage);
            return false;	// Write error...Stdf::ErrorOpen;
        }
        // Generating .CSV report file.
        fprintf(hReportFile,"******************************************************");
        fprintf(hReportFile,"******************************************************\n");
        fprintf(hReportFile,"* %s - www.mentor.com\n", GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
        fprintf(hReportFile,"* %s. All rights reserved.\n",C_STDF_COPYRIGHT);
        fprintf(hReportFile,"******************************************************");
        fprintf(hReportFile,"******************************************************\n");
    }
    else if ( strOutputFormat=="HTML" || strOutputFormat=="DOC" || strOutputFormat=="PPT" || strOutputFormat=="PDF" || strOutputFormat=="ODT" )
    {
        pReportOptions->strReportDirectory = QDir::cleanPath(reportFolderAbsPath() + QDir::separator() + reportName());
        if( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
        {
            // Also, we ALWAYS generate all the sections
            pReportOptions->iHtmlSectionsToSkip = 0;

            // Define HTML Font size to use in reports.
            if(strOutputFormat=="PPT")	//(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_PPT)
            {
                // PowerPoint tables with Percentage bar (pareto) doesn't hold as many lines as under the other FLAT formats (word, PDF)
                iLinesPercentagePerFlatPage = MAX_PERCENTAGE_LINES_PER_PPT_PAGE;
            }
        }

        // If we only analyze data files without creating a Examinator report, then do not copy the HTML files.
        if(bCreateReportFolder)
        {
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteFolderContent(reportPagesFolderAbsPath());	// Erase target folder (<report/pages>)
            // Create all images used in the HTML reports.
            bStatus = cWebResources.CreateWebReportImages(pReportOptions->strReportDirectory.toLatin1().constData());
            if(bStatus != true)
            {
                strErrMessage = "Failed to prepare report folder:\n" + pReportOptions->strReportDirectory;
                strErrMessage += "\n\nPossible cause: Read/Write access issue,\nor a file has the same name as the folder we tried to create.";
                GS::Gex::Message::information("", strErrMessage);
                return false;	// Error creating HTML static images
            }

            bStatus = cWebResources.CreateWebReportPages(pReportOptions->strReportDirectory.toLatin1().constData());
            if(bStatus != true)
            {
                strErrMessage = "Failed to prepare report folder:\n" + pReportOptions->strReportDirectory;
                strErrMessage += "\n\nPossible cause: Read/Write access issue,\nor a file has the same name as the folder we tried to create.";
                GS::Gex::Message::information("", strErrMessage);
                return false;	// Error creating HTML static pages
            }

            // If we do not build a custom home page, exit now!
            if(bCreateReportFile)
            {
                QString strAbsFilePath = "";
                // Create HTML file now as ALL sections will be written in same HTML flat file prior to convert to WORD or PDF
                if (strOutputFormat=="HTML")
                    strAbsFilePath = reportAbsFilePath();
                else
                    strAbsFilePath = reportFlatHtmlAbsPath();

                GS::Gex::Engine::RemoveFileFromDisk(strAbsFilePath);	// Erase target file

                // create target file
                hReportFile = fopen(strAbsFilePath.toLatin1().constData(),"w");
                if(hReportFile == NULL)
                {
                    strErrMessage = "Failed to create report file:\n" + strAbsFilePath;
                    strErrMessage += "\n\nPossible cause: Read/Write access issue,\nor a file has the same name as the folder we tried to create.";
                    GS::Gex::Message::information("", strErrMessage);
                    return false;	// Write error...Stdf::ErrorOpen;
                }
            }
        }
    }

    // success
    return true;
}



bool	CGexReport::CheckIfUseSummary()
{
    // If not processing a Query, we must process the files 'as-is' (there are no summary files in such case)
    if(ReportOptions.bQueryDataset == false)
        return false;

    // If not running a Examinator release supporting the database access, then we must process files 'as-is'
    if(!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        return false;

    ReportOptions.bSpeedUseSummaryDB = false;

    // If summary to be used...then Disable stats computation from "samples only", and force it to "summary & samples" instead.
    if(ReportOptions.bSpeedUseSummaryDB)
        ReportOptions.SetOption("statistics","computation","summary_then_samples");

    return ReportOptions.bSpeedUseSummaryDB;
}

///////////////////////////////////////////////////////////
// Rebuild report pages (without reading data files)
// Assumes a previous report was already created!
///////////////////////////////////////////////////////////
void CGexReport::RebuildReport(void)
{
    // check if a previous report was built
    if(gexReport == NULL)
        return;

    // Check if script executed, but no data available! (No Group or no files)
    // Rebuild the report if we are going to regenerate an Adv. enterprise report
    if((getGroupsList().isEmpty() || ReportOptions.iFiles <= 0)
       && ReportOptions.strReportCenterTemplateFile.isEmpty())
        return;

    // If not report to be created, ignore!
    bool bReportBuildOption = (ReportOptions.GetOption(QString("report"), QString("build"))).toBool();
    if(!bReportBuildOption)
        return;

    // Clear interactive test filter.
    // Emit a signal caught by the connected gui
    clearInteractiveTestFilter();

    // Reset page counter
    lReportPageNumber = 0;				// Holds report page# being written in report file
    lSlideIndex = 0;					// Variables used for PowerPoint slides creation

    // Reuild Report name + copy image & HTML files if need be & open HTM file
    if(!buildReportArborescence(&ReportOptions))
        return;	// Failure while creating report name + folders & copy images...

    // Shows process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,7,1);	// Show process bar...step 1 of 7 (there are 7 steps in the repor tcreation: Histo pages, Wafer pages, Binning pages, Pareto pages, etc...)

    // Rebuild pages now (no data analysis required)!
    bool bNeedPostProcessing = false;
    CreateTestReportPages(&bNeedPostProcessing);

    // Check if report needs post-processing (convert to Word, Ppt, Pdf...)
    if(bNeedPostProcessing == true)
    {
        QString lRes=ReportPostProcessingConverter();
        GSLOG(5, QString("ReportPostProcessingConverter: %1").arg(lRes).toLatin1().data() );
    }

    QString of=ReportOptions.GetOption("output", "format").toString();
    // Reload the current HTML page pointed!
#   ifndef GSDAEMON
    if(pGexMainWindow != NULL)
    {
        pGexMainWindow->reloadUrl();
        // Hide progress bar as it reached 100%
        if (GexProgressBar)
            GexProgressBar->hide();
        // Display report (or launch PDF/WORD/PPT, depending of the report format)
        if(of!="HTML")	//(pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_HTML)
            pGexMainWindow->pWizardScripting->strShowReportPage = "reload_home";	// For NON-HTML report (Word, PDF,...): allows to relaunch file editor!
        pGexMainWindow->ShowReportPage(false);
    }
#   endif
    GS::Gex::Engine::GetInstance().UpdateLabelStatus("");
}

///////////////////////////////////////////////////////////
// Do all prep-wrok before building report
///////////////////////////////////////////////////////////
void CGexReport::BuildReportPrepWork(void)
{
    // Load the Gage R&R Warning file if it exists
    int iLine = 1;

    // Clean up gage warning list
    m_lstGageWarning.clear();

    if (m_pReportOptions->GetOption("adv_boxplot","r&r_alarms").toString()=="dont_check")
    {
        //GSLOG(SYSLOG_SEV_DEBUG, " will not use any Gage RnR warning and alarms rules file.");
        return;
    }

    QString strFilePath;
    strFilePath = m_pReportOptions->GetOption("adv_boxplot","r&r_file").toString();

    if(!QFile::exists(strFilePath))
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Build report prep work : trying to use an unfindable Gage RnR rules file '%1' !")
              .arg(strFilePath).toLatin1().constData());
        return;
    }

    // Read file and extract all lines starting with the IF statement. Ignore all other lines.
    QString			strString;
    CGageWarning	gageWarning;
    QFile			file(strFilePath); // open file

    GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString(" Opening %1...").arg(strFilePath) );

    if (file.open(QIODevice::ReadOnly) == false)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString(" error : cant open '%1' !").arg( strFilePath).toLatin1().constData() );
        return;	// Failed reading file.
    }

    QTextStream hFile(&file);

    QString strExpression,strColor;
    do
    {
        // Read one line from file
        strString = hFile.readLine();
        if(strString.isEmpty() || strString.startsWith("#"))
            goto next_rr_line;
        if(strString.startsWith("If", Qt::CaseInsensitive) == false)
            goto next_rr_line;

        // Convert to lowercase
        strString = strString.toLower();

        // Valid line found. eg: IF Cpk > 1.0 AND Cpk < 1.33 AND R_R < 25 THEN Color = Green
        strExpression = strString.section("then",0,0);	// All string before the 'Then'
        strExpression = strExpression.section("if",1);  // All string after the 'If'
        strColor = strString.section("=",-1).trimmed();	// Last string after the '='

        // Save expression structure
        gageWarning = CGageWarning();
        gageWarning.setFileLine(iLine);
        gageWarning.setExpression(strExpression);

        if(strColor == "default")
            gageWarning.setColor(QColor(0xff,0xff,0xcc));	// Default data background color as defined in .h files.
        else if(strColor == "red")
            gageWarning.setColor(Qt::red);
        else if(strColor == "yellow")
            gageWarning.setColor(Qt::yellow);
        else if(strColor == "white")
            gageWarning.setColor(Qt::white);
        else if(strColor == "green")
            gageWarning.setColor(Qt::green);

        // Add entry to our list
        m_lstGageWarning.append(gageWarning);

next_rr_line:
        // Keep track of line#
        iLine++;
    }
    while(hFile.atEnd() == false);

    file.close();
}

///////////////////////////////////////////////////////////
// Build all report!
///////////////////////////////////////////////////////////
QString CGexReport::BuildReport(CReportOptions *pOptions)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Starting to create report...").toLatin1().constData());
    GexPerformanceCounter perfCounter(true);

    // Read file headers...
    CGexGroupOfFiles	*pGroup=0;
    CGexFileInGroup		*pFile=0;
    int lReturn=0;

    // Status bar message.
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Reading data...");

    // Keep pointer to 'Options'
    m_pReportOptions = pOptions;

    UpdateOptions(pOptions);

    int iProductID = GS::LPPlugin::ProductInfo::getInstance()->getProductID();

    // Do all prep-wrok before building report. Eg: Load configuration files (if any. eg: Gage R&R alarm thresholds, etc...)
    BuildReportPrepWork();

    GSLOG(SYSLOG_SEV_DEBUG, QString("PrepWork done...").toLatin1().constData());

    // Hardcode restrictions for some Examinator releases
    pOptions->bSpeedCollectSamples = false;
    QString of=ReportOptions.GetOption("output", "format").toString();
    switch(iProductID)
    {
        case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:		// OEM-Examinator for LTXC
            // Only accept HTML, CSV and PDF formats.
            if(of=="HTML"||of=="CSV"||of=="PDF")	//(pOptions->iOutputFormat & (~(GEX_OPTION_OUTPUT_HTML | GEX_OPTION_OUTPUT_CSV | GEX_OPTION_OUTPUT_PDF)))
            {
                pOptions->SetOption("output","format", "HTML"); //pOptions->iOutputFormat = GEX_OPTION_OUTPUT_HTML;
                of="HTML";
            }
            pOptions->bSpeedCollectSamples = true; // required to generate histograms under PDF output format
            break;
        default:
            break;
    }

    // If no report to create (only interactive mode, or Yield 123, etc..), then make sure we flag it!
    if(of=="INTERACTIVE")
    {
        pOptions->SetOption(QString("report"), QString("build"), QString("false"));
    }

    // For Guardbanding, never use the pass/fail flag
    if (pOptions->getAdvancedReport() == GEX_ADV_GUARDBANDING)
        pOptions->SetOption("dataprocessing", "param_passfail_rule", "limits_only");

    // Reset counter, variables
    pOptions->iGroups=0;	// Each file analyzed need to know if other groups exist...
    pOptions->iFiles=0;
    iScriptExitCode = GS::StdLib::Stdf::NoError;	// Holds the error code if any during script execution.
    lReportPageNumber = 0;				// Holds report page# being written in report file
    lSlideIndex = 0;					// Variables used for PowerPoint slides creation

    // Scan all groups and files to compute total size to process
    QFileInfo	cFileInfo;
    QList<CGexGroupOfFiles*>::iterator itGroup = getGroupsList().begin();
    bool lEmptyDataset = true;

    while(itGroup != getGroupsList().end())
    {
        lEmptyDataset = false;
        pFile = NULL;
        pGroup	= (*itGroup);
        if (!pGroup)
        {
            GSLOG(SYSLOG_SEV_ERROR, "error : pGroup NULL !");
            return "error : found a NULL CGexGroupOfFiles !";
        }

        if (pGroup->pFilesList.isEmpty())
        {
          QString m=QString("this group (%1) has no data ").arg(pGroup->strGroupName);
          GSLOG(SYSLOG_SEV_ERROR, m.toLatin1().data() );
          GetReportLogList().addReportLog(m,GS::Gex::ReportLog::ReportWarning);
        }
        else
        {
            pFile = (pGroup->pFilesList).first();
        }

        // Keeps track of number of valid groups (not empty)
        if(pFile != NULL)
        {
            // Keeps track of number of files
            pOptions->iFiles += pGroup->pFilesList.count();
            pOptions->iGroups++;

            // Review files in group to analyze (compute overall size!)...If the group is a query, then use query size info.
            if(pGroup->cQuery.lfQueryFileSize > 0)
            {
                // This group is a query: get file size from the query structure.
                lfTotalFileSize += pGroup->cQuery.lfQueryFileSize;
            }
            else
            {
                // This group is made of individual files (not a query), then compte total size...
                QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

                while (itFilesList.hasNext())
                {
                    pFile = itFilesList.next();
                    // Compute size of ALL files to process (STDF files in all cases as other
                    // format are converted to STDF automatically)
                    cFileInfo.setFile(pFile->strFileNameSTDF);
                    lfTotalFileSize += (double) cFileInfo.size();
                };
            }

            ++itGroup;
        }
        else
        {
            // Empty group: delete it!
            delete (pGroup);
            itGroup = getGroupsList().erase(itGroup);

            // If all groups are deleted
            if(getGroupsList().isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR, "Error while generating the report - all datasets are empty");
                GS::Gex::Message::information(
                            "", "Error while generating the report.\n"
                                "It seems that all datasets are empty.\n"
                                "Please verify your dataset definitions");
                return "error : all datasets are empty";
            }
        }
    }
    if(lEmptyDataset && getGroupsList().isEmpty() && ReportOptions.strReportCenterTemplateFile.isEmpty() == true)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Error while generating the report - all datasets are empty");
        GS::Gex::Message::information(
                    "", "Error while generating the report.\n"
                        "It seems that all datasets are empty.\n"
                        "Please verify your dataset definitions");
        return "error : all datasets are empty";
    }
    ;


    GSLOG(SYSLOG_SEV_DEBUG, "Group and files scanned for size computation...");

    ReportOptions.bSpeedCollectSamples = true;		// Always collect samples for 2D/3D Interactive Charting

    // If Report template selected, overload appropriate settings (eg: type of wafermap data to collect, etc...)
    if(QFile::exists(pOptions->strTemplateFile) && pOptions->getAdvancedReport() == GEX_ADV_TEMPLATE)
    {
        // Make sure no report section will be skipped as this is a 'My Report' we always force to rebuild sections!
        m_pReportOptions->iHtmlSectionsToSkip = 0;

        // Load template file if already defined
        GS::Gex::ReportTemplate   reportTemplate;
        GS::Gex::ReportTemplateIO reportTemplateIO;
        reportTemplateIO.ReadTemplateFromDisk(&reportTemplate,
                                              pOptions->strTemplateFile);

        // Get Wafermap settings (if any)
        GS::Gex::CGexCustomReport_WafMap_Section* pWafmap =
                reportTemplate.GetWafmapSectionDetails();
        if(pWafmap != NULL)
        {
            // Import settings
            pOptions->iWafermapType = pWafmap->iWafermapType+1;	// Set wafermap type to collect (increment counter to align #define values)

            switch(pOptions->iWafermapType)
            {
                case GEX_WAFMAP_SOFTBIN:// Standard Wafer map: Software Binning
                case GEX_WAFMAP_STACK_SOFTBIN:
                case GEX_WAFMAP_ZONAL_SOFTBIN:
                case GEX_WAFMAP_HARDBIN:// Standard Wafer map: Hardware Binning
                case GEX_WAFMAP_STACK_HARDBIN:
                case GEX_WAFMAP_ZONAL_HARDBIN:
                case GEX_WAFMAP_TESTOVERLIMITS:  // zoning on test limits
                case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                case GEX_WAFMAP_TESTOVERDATA:			// zoning on test values range
                case GEX_WAFMAP_TEST_PASSFAIL:			// zoning on test values range
                case GEX_WAFMAP_STACK_TESTOVERDATA:
                case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                    pOptions->pGexWafermapRangeList = createTestsRange(pWafmap->strRangeList,true,false);
                    break;
            }
        }

    }

    GSLOG(SYSLOG_SEV_DEBUG, "Settings overloaded if template set...");

    // Prepare the process bar, conpute total steps
    // a) Total steps = 2xiFiles+8 ...before we know how many steps in 'Advanced' report section
    //    7 in the formula comes from the starting step + 1 empty step+6 steps for creating sections:global,stats,hito,wafmap,bin,pareto
    // b) Once 'advanced' report is ready for generation, we know how many charts still to
    //    be created (X) and add X more steps to do in the process bar.
    int iTotalSteps = ((2*pOptions->iFiles)+7);
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString(" Reading data (%1 steps)...").arg(iTotalSteps));
    if(ReportOptions.getAdvancedReport() != GEX_ADV_DISABLED)
        iTotalSteps*=2;	// If advanced report is required, then keep 50% of the progress gauge for it!

    int iGaugeStep=1;	// Starting step: 1
    // Shows process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true,iTotalSteps,iGaugeStep);	// Show process bar...step 1

    // Build Report name + copy image & HTML files if need be
    if(!buildReportArborescence(pOptions))
    {
        GSLOG(SYSLOG_SEV_ERROR, "error while generating report folder name...");
        GS::Gex::Message::information(
                    "", "Error while generating the report.\n"
                        "The report folder hierarchy could not be created");
        return "error : could not create the report folder hierarchy";	// Failure while creating report name + folders & copy images...
    }
    GSLOG(SYSLOG_SEV_DEBUG, "Report arborescence created");

    // Before analysis files, check if need to set custom test limits (gexOptions('output','GuardBanding Analysis) or markers, etc...
    CTestCustomize	*pTest=0;
    CTest			*ptTestCell=0;	// Pointer to test cell to receive STDF info.

    // Rewind to first group & first file...
    QListIterator<CGexGroupOfFiles*> itGroupsList(getGroupsList());

    while(itGroupsList.hasNext())
    {
        pGroup	= itGroupsList.next();
        if (!pGroup)
            continue;
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString(" Reading data (group %1)...").arg(pGroup->strGroupName) );
        pFile = (pGroup->pFilesList).first();

        QListIterator<CTestCustomize*> itTest(m_pTestCustomFieldsList);

        while(itTest.hasNext())
        {
            pTest = itTest.next();

            // Create each test cell (no mapping on test#!)
            int lR=pFile->FindTestCell(pTest->lTestNumber, pTest->lPinmapIndex, &ptTestCell, false, true,
                                       pTest->strTestName.toLatin1().data());
            if(lR==1)
            {
                // If custom marker, add it to the test structure
                if(pTest->ptMarker != NULL)
                    ptTestCell->ptCustomMarkers.append(pTest->ptMarker);

                // If custom viewport definition, add it to the test structure
                if(pTest->ptChartOptions != NULL)
                    ptTestCell->ptChartOptions = pTest->ptChartOptions;
            }
            else if (lR==-1) // bad_alloc
            {
                GSLOG(3, QString("Failed to create a new test cell (%1 instances of CTest)")
                      .arg(CTest::GetNumberOfInstances()).toLatin1().data() );
                return "error: failed to create a new test cell (low memory,...)";
            }
        };

        // Ensure to update the MergedGroup pointer so we don't lose the short list of test just created...
        pGroup->cMergedData.ptMergedTestList = pFile->ptTestList;
    };

    // Read all groups & files. Merge data for each group, compare groups
    // Rewind to first group & first file...
    itGroupsList.toFront();
    TestSiteLimits::reset();

    QList<CGexGroupOfFiles*>::iterator itGroupsListIter(getGroupsList().begin()), itGroupsListEnd(getGroupsList().end());
    for(;itGroupsListIter != itGroupsListEnd; ++itGroupsListIter) {
         (*itGroupsListIter)->resetIgnoreMRR();
        (*itGroupsListIter)->Init( (*itGroupsListIter), &ReportOptions,hAdvancedReport , 1);
    }


            if(ReportOptions.getAdvancedReport() == GEX_ADV_DATALOG)
            {
                int iStatus = PrepareSection_Advanced(true);
                if(iStatus != GS::StdLib::Stdf::NoError)
                    return QString("error : Stdf error %1").arg(iStatus);
            }

    // -------PASS 1
    QList<StdfDataFile*>::iterator lIter(mOrderedStdfDataFiles.begin()), lIterEnd(mOrderedStdfDataFiles.end());
    for(;lIter != lIterEnd; ++lIter)
    {
        StdfDataFile* lStdfDataFile= *lIter;
        if(lStdfDataFile)
        {
            lStdfDataFile->loadFile();
            lStdfDataFile->mReportOptions = m_pReportOptions;
            lStdfDataFile->mAdvFile = hAdvancedReport;
            lStdfDataFile->initPass();
            lStdfDataFile->processFile(1);
            lStdfDataFile->closeFile();
        }
        if(GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
            return "error : script aborted by user !";
    }

    // -------PASS 2
    // Prepare second pass...notify Plugin, etc...
    QList<CGexGroupOfFiles*>::iterator itGroupsListIter2(getGroupsList().begin());
    for(;itGroupsListIter2 != itGroupsListEnd; ++itGroupsListIter2) {
        PrepareSecondPass((*itGroupsListIter2),&ReportOptions);
        (*itGroupsListIter2)->Init( (*itGroupsListIter2), &ReportOptions,hAdvancedReport , 2);
    }

    QString strDatasetSorting = (gexReport->GetOption("dataprocessing", "sorting")).toString();

    //-- sort the file by date for the second pass if needed
    if(strDatasetSorting=="date")
    {
        // Sort files by date in MIR
        if (!pGroup->pFilesList.isEmpty())
            qSort(pGroup->pFilesList.begin(), pGroup->pFilesList.end(), CGexFileInGroup::lessThan);

        for (int nIndex = 0; nIndex < pGroup->pFilesList.count(); nIndex++)
            pGroup->pFilesList.at(nIndex)->lFileID = nIndex;

        QList<StdfDataFile*> lListDataFile;

        QSet<QString> filterOneTime;
        QList<CGexFileInGroup*>::iterator lIter(pGroup->pFilesList.begin()), lIterEnd(pGroup->pFilesList.end());
        for(; lIter != lIterEnd; ++lIter)
        {
            //-- avoid to insert twice
            if( filterOneTime.insert((*lIter)->strFileName) != filterOneTime.end() )
            {
                StdfDataFile* lStdfDataFile = FindStdfDataFile((*lIter)->strFileName);
                if(lStdfDataFile)
                    lListDataFile.append(lStdfDataFile);
            }
        }
        if(lListDataFile.isEmpty() == false)
        mOrderedStdfDataFiles = lListDataFile;
    }


    lIterEnd = mOrderedStdfDataFiles.end();
    for(lIter = mOrderedStdfDataFiles.begin();lIter != lIterEnd; ++lIter)
    {
        StdfDataFile* lStdfDataFile= *lIter;
        if(lStdfDataFile)
        {
            lStdfDataFile->initPass();
            lStdfDataFile->loadFile();
#           ifndef GSDAEMON
            if (pGexMainWindow != NULL)
            {
                pGexMainWindow->ResetProgress(false);
            }
#           endif
            lStdfDataFile->processFile(2);
            lStdfDataFile->closeFile();
        }
    }

        QDir oTempDir(GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString() + QDir::separator()+ "temp" + QDir::separator()+ "datalog");
        if(oTempDir.exists())
        {
            oTempDir.rmdir(GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString() + QDir::separator()+ "temp" + QDir::separator()+ "datalog");
        }

    QList<CGexGroupOfFiles*>::iterator itGroupsListIterFinish(getGroupsList().begin());
    for(;itGroupsListIterFinish != itGroupsListEnd; ++itGroupsListIterFinish) {
          FinishPasses(*itGroupsListIterFinish, &ReportOptions);
    }


    // Recomputes the exact total number of files...as it may have increased
    // if files had multiple WaferMaps inside! (number of groups never changes)
    itGroupsList.toFront();
    pOptions->iFiles = 0;

    while(itGroupsList.hasNext())
    {
        // Add number of files in each group
        pGroup	= itGroupsList.next();
        pFile	= (pGroup->pFilesList).first();
        pOptions->iFiles += (pGroup->pFilesList).count();
    };

    //QList<CGexGroupOfFiles*>  pGroupList = gexReport->getGroupsList();
   // QString strDatasetSorting = (gexReport->GetOption("dataprocessing", "sorting")).toString();
    if(strDatasetSorting=="date"){
        qSort(getGroupsList().begin(), getGroupsList().end(), CGexGroupOfFiles::lessThan);
    }

    // End of data collection. Check if valid licence.
    iScriptExitCode = EndDataCollection();
    if(iScriptExitCode != GS::StdLib::Stdf::NoError)
        return QString("error : iScriptExitCode = %1").arg(iScriptExitCode);	// Failure (license expired?).

    // If multi-groups, ensure each group has same tests list...
    if(getGroupsList().count() >= 2)
    {
        CGexGroupOfFiles	*pGroup2 = NULL;
        CGexFileInGroup		*pFile2 = NULL;
        CTest				*ptMergedGroupsTest = NULL;
        int					iGroupIndex1=0,iGroupIndex2=0;
        QString lLabelStatus = QString(" Building merged tests list of group %1/%2...");
        QString lErrorMsg = QString("Building merged tests list for group %1. %2 instances of CTest...");

        // For each group: Create all tests only present in other groups.
        for(iGroupIndex1 = 0;iGroupIndex1<getGroupsList().count();iGroupIndex1++)
        {
            // Handle to testlist
            pGroup = getGroupsList().at(iGroupIndex1);
            if (!pGroup)
            {
                GSLOG(SYSLOG_SEV_ERROR, "pGroup NULL !");
                continue;
            }
            GSLOG(SYSLOG_SEV_NOTICE,
                  lErrorMsg
                  .arg(iGroupIndex1)
                  .arg(CTest::GetNumberOfInstances())
                  .toLatin1().data() );

            GS::Gex::Engine::GetInstance().UpdateLabelStatus( lLabelStatus
                                                              .arg(iGroupIndex1)
                                                              .arg(getGroupsList().count() )
                                                              );

            pFile = (pGroup->pFilesList).first();
            if (!pFile)
                return "error: found a null file";

            bool lNewTestAdded = false;
            for(iGroupIndex2 = iGroupIndex1 + 1; iGroupIndex2<getGroupsList().count(); iGroupIndex2++)
            {
                if(iGroupIndex1 != iGroupIndex2)
                {
                    // -- groupIndex1 <> grouIndex2
                    ptTestCell = pFile->ptTestList;

                    pGroup2 = getGroupsList().at(iGroupIndex2);
                    pFile2 = (pGroup2->pFilesList).first();
                    size_t lNbTests = pFile2->GetCTests().size();
                    while(ptTestCell)
                    {
                        lReturn=pFile2->FindTestCell(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex, &ptMergedGroupsTest,
                                                      false,true, ptTestCell->strTestName.toLatin1().data());

                        if(pFile2->GetCTests().size() != lNbTests)
                            lNewTestAdded = true;

                        if (lReturn==-1)
                        {
                            printf("\nBuildReport: FindTestCell() failed (-1:bad_alloc)\n");
                            return "error: cannot create new Test (low mem,...)";
                        }

                        if(lReturn!=1)
                            break;	// Error

                        ptTestCell = ptTestCell->GetNextTest();

                        if(GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
                            return "error : script aborted.";
                    }
                    pGroup2->cMergedData.ptMergedTestList = pFile2->ptTestList;

                    //-- this test check that we have to different list. Otherwise this is skiped
                    // -- groupIndex2 <> grouIndex1
                    if(lNewTestAdded || pFile->GetCTests().size() != pFile2->GetCTests().size())
                    {
                        ptTestCell = pFile2->ptTestList;
                        while(ptTestCell)
                        {
                            lReturn=pFile->FindTestCell(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex, &ptMergedGroupsTest,
                                                          false,true, ptTestCell->strTestName.toLatin1().data());
                            if (lReturn==-1)
                            {
                                printf("\nBuildReport: FindTestCell() failed (-1:bad_alloc)\n");
                                return "error: cannot create new Test (low mem,...)";
                            }

                            if(lReturn!=1)
                                break;	// Error

                            ptTestCell = ptTestCell->GetNextTest();

                            if(GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
                                return "error : script aborted.";
                }
                        pGroup->cMergedData.ptMergedTestList = pFile->ptTestList;
                    }
            }
            }

            // Ensure to update the MergedGroup pointer so we don't lose the short list of test just created...
            pGroup->cMergedData.ptMergedTestList = pFile->ptTestList;
        }
    }

    this->UpdateUserTestNameAndTestNumber();

    // Compute statistics based on all data collected
    ComputeDataStatistics(true);

    // Create standard EXAMINATOR report (first: build test limits strings, then create reports)
    bool bNeedPostProcessing = false;

    // Create a report of report builder
    // Build the OFR if needed
    if (ReportOptions.getAdvancedReport() == GEX_ADV_REPORT_BUILDER)
    {
        QMap< QString,  QString> lParams;
        QString lFormat("pdf");
        lParams.insert("report_format", lFormat);
        if (!GS::Gex::OFR_Controller::GetInstance()->GenerateReport(ReportOptions.strTemplateFile,
                                                                   ReportOptions.strReportDirectory+".pdf",
                                                                   lParams))
            return "error generating report";
        else
            return "ok";
    }
    else
        iScriptExitCode = CreateTestReportPages(&bNeedPostProcessing);

    // Does the report need post-processing
    if(bNeedPostProcessing == true)
    {
        QString lRes=ReportPostProcessingConverter();
        if (lRes.startsWith("err"))
            return lRes;
    }

    // todo : update gex_report_db now with bins, tests, parts ?
    //QString r=UpdateGexReportDb();

    GSLOG(SYSLOG_SEV_DEBUG, QString("Build Report in %1 ms.")
           .arg(perfCounter.elapsedTime() / 1000).toLatin1().data()
           );

#   ifndef GSDAEMON
    if (pGexMainWindow != NULL)
    {
        pGexMainWindow->m_bDatasetChanged	= false;
        pGexMainWindow->iHtmlSectionsToSkip	= GEX_HTMLSECTION_ALL;
    }
#   endif

    GEX_BENCHMARK_DUMP();
    return "ok";
}

/////////////////////////////////////////////////////////////////////////////
// Create Report file
/////////////////////////////////////////////////////////////////////////////
FILE	*CGexReport::CreateReportFile(QString strFileName)
{
    hReportFile = fopen(strFileName.toLatin1().constData(),"wt");
    return hReportFile;
}

/////////////////////////////////////////////////////////////////////////////
// Close the Report file
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::CloseReportFile(FILE *hFile)
{
    if(hFile != NULL)
    {
        // Close handle given in parameter...then check if it's one of the internal report handle!
        fclose(hFile);
        if(hFile == hAdvancedReport)
            hAdvancedReport = NULL;
        if(hFile == hReportFile)
            hReportFile = NULL;
    }
    else
    {
        // Close standard report file handles.
        hFile = hReportFile;

        // just in case!
        if(hReportFile != NULL)
            fclose(hReportFile);

        // Check if this handle is a duplicate of the Advanced report one...
        if(hReportFile == hAdvancedReport)
            hAdvancedReport = hReportFile = NULL;
        else
            hReportFile = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Set the list of test filtered in interactive mode
/////////////////////////////////////////////////////////////////////////////
void CGexReport::setInteractiveTestFilter(const QString& strInteractiveTestFilter)
{
    m_lstInteractiveTestFilter =
        strInteractiveTestFilter.split(',', QString::SkipEmptyParts);

    emit interactiveFilterChanged(hasInteractiveTestFilter());
}

/////////////////////////////////////////////////////////////////////////////
// Clear the list of test filtered in interactive mode
/////////////////////////////////////////////////////////////////////////////
void CGexReport::clearInteractiveTestFilter()
{
    m_lstInteractiveTestFilter.clear();

    emit interactiveFilterChanged(hasInteractiveTestFilter());
}

/////////////////////////////////////////////////////////////////////////////
// Return true is a interactive test filter has been activated, otherwise false
/////////////////////////////////////////////////////////////////////////////
bool CGexReport::hasInteractiveTestFilter() const
{
    return m_lstInteractiveTestFilter.size();
}

/////////////////////////////////////////////////////////////////////////////
// Return true is a current test belong to the filter
/////////////////////////////////////////////////////////////////////////////
bool CGexReport::isInteractiveTestFiltered(CTest * pTest)
{
    bool bFound = true;

    if (hasInteractiveTestFilter())
    {
        bFound = false;

        for (int nFilter = 0; nFilter < m_lstInteractiveTestFilter.size() && !bFound; nFilter++)
        {
            QString strFilter	= m_lstInteractiveTestFilter.at(nFilter);

            int		nPinmap		= -1;
            int		nTestNumber = strFilter.section(".", 0, 0).toInt();
            QString	strField	= strFilter.section(".", 1, 1);

            if (!strField.isEmpty())
                nPinmap	 = strField.toInt();

            if (pTest && pTest->lTestNumber == (unsigned)nTestNumber && pTest->lPinmapIndex == nPinmap)
                bFound = true;
        }
    }

    return bFound;
}

void CGexReport::SetProcessingFile(bool lProcessingFile)
{
    mProcessingFile = lProcessingFile;
}

void	CGexReport::setLegacyReportName(const QString &strName)
{
    m_strLegacyReportAbsFilePath = strName;
}

///////////////////////////////////////////////////////////
// Script header for Script-based report generation
///////////////////////////////////////////////////////////
FILE *GexMainwindow::CreateGexScript(int iGexScriptName)
{
    QString	strScriptName;
    FILE *hFile=0;

    // Get correct script path+name to create
    switch(iGexScriptName)
    {
        case GEX_SCRIPT_CONFIG:
            strScriptName = GS::Gex::Engine::GetInstance().GetStartupScript();
            break;

        case GEX_SCRIPT_ASSISTANT:
            strScriptName = GS::Gex::Engine::GetInstance().GetAssistantScript();
            break;
    }

    //FIXME: should not need to do that
    //if (strScriptName.isEmpty())
    //{
    //    GS::Gex::Engine::GetInstance().InitScriptsPath();
    //    switch (iGexScriptName)
    //    {
    //    case GEX_SCRIPT_CONFIG:
    //        strScriptName =
    //            GS::Gex::Engine::GetInstance().GetStartupScript();
    //        break;
    //    case GEX_SCRIPT_ASSISTANT:
    //        strScriptName =
    //            GS::Gex::Engine::GetInstance().GetAssistantScript();
    //        break;
    //    }
    //}

    GSLOG(6, QString("Lets try to open file in write mode '%1'...").arg(strScriptName).toLatin1().data() );
    hFile = fopen(strScriptName.toLatin1().constData(),"w");
    if(hFile == NULL)
    {
        QString strErrMessage = "Failed to create script file:\n" + strScriptName;
        strErrMessage += "\n\nPossible cause: Read/Write access issue.";
        GS::Gex::Message::information("", strErrMessage);
        return NULL;	// Failed creating script
    }

    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"// Welcome to Quantix Examinator Script\n");
    fprintf(hFile,"// Find more on www.mentor.com\n");
    fprintf(hFile,"// File created by: %s\n", GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
    fprintf(hFile,"//////////////////////////////////////////\n\n");
    return hFile;
}

///////////////////////////////////////////////////////////
// Script function to request GEX to build the report
///////////////////////////////////////////////////////////
void GexMainwindow::WriteBuildReportSection(
        FILE *hFile, QString strLoadPage, bool bCreateReport)
{
    if (!hFile)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Write BuildReport Section : hFile NULL & LoadPage = %1")
              .arg( strLoadPage).toLatin1().constData());
        return;
    }
    GSLOG(SYSLOG_SEV_DEBUG, QString("Write BuildReport Section : LoadPage=%1 bCreateReport=%2")
             .arg(strLoadPage)
             .arg(bCreateReport?"true":"false" ).toLatin1().constData());

    // Writes 'BuildReport' section
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"// GEX to analyze data + create report\n");
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"BuildReport()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n");
    fprintf(hFile,"  // Have Quantix Examinator build report, then show it!\n");
    if(bCreateReport == false)
        fprintf(hFile,"  gexOptions('report','build','false');\n");	// Only data analysis, no report created!
    // Script instruction to trigger report generation + list of HTML sections NOT to create.
    fprintf(hFile,"  gexBuildReport('%s','%d');\n",
            (char *)strLoadPage.toLatin1().constData(), iHtmlSectionsToSkip);
    fprintf(hFile,"\n\n  sysLog('* Data analysis and report completed ! *');\n\n");

    // Close function
    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");
}

///////////////////////////////////////////////////////////
// Script function to define Data file groups
///////////////////////////////////////////////////////////
void GexMainwindow::WriteProcessFilesSection(FILE *hFile)
{
    if (!hFile)
    {
        GSLOG(SYSLOG_SEV_ERROR, "File null");
        return;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Write process files section: wizard type=%1")
          .arg( mWizardType).toLatin1().constData());

    // Writes 'SetProcessData' section
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"// List Queries or files groups to process\n");
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"// BEGIN PROCESS_DATA \n");
    fprintf(hFile,"  var group_id;  // Holds group_id when creating groups\n\n");

    switch(mWizardType)
    {
        case GEX_ONEFILE_WIZARD:
            pWizardOneFile->WriteProcessFilesSection(hFile);
            break;
        case GEX_SHIFT_WIZARD:
        case GEX_CMPFILES_WIZARD:
            pWizardCmpFiles->WriteProcessFilesSection(hFile);
            break;
        case GEX_FT_PAT_FILES_WIZARD:
            if (mPATReportFTGui)
                mPATReportFTGui->WriteProcessFilesSection(hFile);
            else
                GSLOG(SYSLOG_SEV_WARNING, "FT PAT report GUI not allocated");
            break;
        case GEX_ADDFILES_WIZARD:
            pWizardAddFiles->WriteProcessFilesSection(hFile);
            break;
        case GEX_MIXFILES_WIZARD:
            pWizardMixFiles->WriteProcessFilesSection(hFile);
            break;
        case GEX_ONEQUERY_WIZARD:
            pWizardOneQuery->WriteProcessFilesSection(hFile);
            break;
        case GEX_CMPQUERY_WIZARD:
            pWizardCmpQueries->WriteProcessFilesSection(hFile);
            break;
        case GEX_MIXQUERY_WIZARD:
            pWizardCmpQueries->WriteProcessFilesSection(hFile);
            break;
        case GEX_SQL_QUERY_WIZARD:
            pWizardOneQuery->WriteProcessFilesSection(hFile);
            break;
        case GEX_CHAR_QUERY_WIZARD:
            pWizardOneQuery->WriteProcessFilesSection(hFile);
            break;
        default:
            GEX_ASSERT(false);
            GSLOG(SYSLOG_SEV_WARNING, "Invalid wizard type");
            break;
    }

    fprintf(hFile,"// END PROCESS_DATA \n");
    // Close function
    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");
}

///////////////////////////////////////////////////////////
// Script footer for Script-based report generation
///////////////////////////////////////////////////////////
void GexMainwindow::CloseGexScript(FILE *hFile,int iSections)
{
    GSLOG(SYSLOG_SEV_DEBUG, " closing script...");
    // Writes 'main' section of the script+calls to relevant sections
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"// Script entry point\n");
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");

    // Inform the Csl format version
    // Nowadays, Gex will write only the latest version of the csl format.
    fprintf(hFile,"  gexCslVersion('%1.2f');\n\n", GEX_MAX_CSL_VERSION);

    // Call to 'Favorite Scripts'
    if(iSections & GEX_SCRIPT_SECTION_FAVORITES)
    {
        fprintf(hFile,"  // Setup the GEX 'Preferences'\n");
        fprintf(hFile,"  SetPreferences();\n\n");
    }

    // Call to 'SetOptions'
    if(iSections & GEX_SCRIPT_SECTION_OPTIONS)
    {
        fprintf(hFile,"  // Setup the GEX Options\n");
        fprintf(hFile,"  SetOptions();\n\n");
    }

    // Call to 'SetProcessData'...must be before SetSettings, because it does a RESET
    if(iSections & GEX_SCRIPT_SECTION_GROUPS)
    {
        fprintf(hFile,"  // Setup the list of data Queries or files to process\n");
        fprintf(hFile,"  SetProcessData();\n\n");
    }

    // Call to 'SetDrill' (this call must stay AFTER 'SetProcessData')
    if(iSections & GEX_SCRIPT_SECTION_DRILL)
    {
        fprintf(hFile,"  // Setup the GEX DataMining Settings\n");
        fprintf(hFile,"  SetDataMining();\n\n");
    }

    // Call to 'SetSettings'
    if(iSections & GEX_SCRIPT_SECTION_SETTINGS)
    {
        fprintf(hFile,"  // Setup the GEX Report type (Settings)\n");
        fprintf(hFile,"  SetReportType();\n\n");
    }

    // If Plugin activated, Call Plugin section
    if(iSections & GEX_SCRIPT_SECTION_PLUGIN)
    {
        fprintf(hFile,"  // Call Plugin setup function\n");
        fprintf(hFile,"  SetPluginOptions();\n\n");
    }

    // Call to 'BuildReport'
    if(iSections & GEX_SCRIPT_SECTION_REPORT)
    {
        fprintf(hFile,"  // Analyze data and build the report, do not show report page\n");
        fprintf(hFile,"  BuildReport();\n\n");
    }

    // Closes the 'main' section
    fprintf(hFile,"}\n");

    // Close file
    fclose(hFile);
}

///////////////////////////////////////////////////////////
// Build report from 'What-If' assistant...ensure Drill
// parameters are listed in script file
///////////////////////////////////////////////////////////
void GexMainwindow::BuildReportSetDrill(void)
{
    // If we have to compute a what-if and some pages have to be created...
    // ..this means environment changed too much to allow this action,
    // ...and user needs to go back to the Settings page...

    // Get Report output format requested in what-if page, force it in settings page
    if(pWizardWhatIf->comboBoxOutputFormat->isVisible())
    {
        // Copy Report outut format selected under what-if, into Settings page.
        pWizardSettings->comboBoxOutputFormat->setCurrentIndex(pWizardWhatIf->comboBoxOutputFormat->currentIndex());

        // Detach What-if page
        pWizardWhatIf->ForceAttachWindow(false);

        // Make Settings page visible
        Wizard_Settings();
    }

    // Get report built
    BuildReportNow(true);

    // Display report page in case the what-if page is detached.
    if(pWizardWhatIf->comboBoxOutputFormat->isVisible())
        ReportLink();
}

void GexMainwindow::BuildReport(void)
{
    BuildReportNow(false);
}

///////////////////////////////////////////////////////////
// Build report from Assistants selections...now!
///////////////////////////////////////////////////////////
QString GexMainwindow::BuildReportNow(
        bool bFromWhatIf,
        QString strStartupPage,
        bool bCreateReport,
        QString outputFormat,
        QString strRCgrtFile)
{
    CGexGroupOfFiles::resetIgnoreAllMRR();
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("build %1 to %2...").arg(
              strStartupPage).arg(outputFormat).toLatin1().constData());

    // If a script is already running, then the button is a 'abort' instead!
    if(GS::Gex::CSLEngine::GetInstance().IsRunning())
    {
        GS::Gex::CSLEngine::GetInstance().AbortScript();
        return "error : a script is already running. Please wait and retry.";
    }

    // Writes script footer : writes 'main' function+ call to 'Favorites'+SetOptions+SetDrill
    int iFlags = GEX_SCRIPT_SECTION_GROUPS | GEX_SCRIPT_SECTION_REPORT;
    iFlags |= GEX_SCRIPT_SECTION_SETTINGS | GEX_SCRIPT_SECTION_OPTIONS;
    iFlags |= GEX_SCRIPT_SECTION_DRILL;

    // Create header section of the script file
    FILE *hFile=NULL;
    // Creates header of Gex Assistant script file.
    hFile = CreateGexScript(GEX_SCRIPT_ASSISTANT);
    if(hFile == NULL)
        return QString("error : cant create script file '%1' !").arg(GEX_SCRIPT_ASSISTANT);

    // Creates 'SetOptions' section
    if(!(ReportOptions.WriteOptionSectionToFile(hFile)))
    {
        QString strErrorMsg("Error : problem occured when write option section");
        GSLOG(SYSLOG_SEV_WARNING, strErrorMsg.toLatin1().constData());
        GEX_ASSERT(false);
        return strErrorMsg;
    }
    // pWizardOptions->WriteOptionsSection(hFile);

    // Creates 'SetReportType' section.
    if (strRCgrtFile.isNull() || strRCgrtFile.isEmpty()) // was isNull()
        pWizardSettings->WriteScriptReportSettingsSection(hFile,bFromWhatIf,strStartupPage);
    else // the Repoprt Center grtfile is specified
        WriteScriptGalaxyReportCenterSettingsSection(hFile,bFromWhatIf,strStartupPage, outputFormat, strRCgrtFile);


    // GEX-DataMining : Create Drill section...
    //if(bIncludeDrillAction == false)
    pWizardSettings->WriteBuildDrillSection(hFile,bFromWhatIf);

    // Creates 'SetProcessData' section
    WriteProcessFilesSection(hFile);

    // Creates 'BuildReport' section...always write this section last (so HTML section list to exclude is known).
    WriteBuildReportSection(hFile,strStartupPage,bCreateReport);

    CloseGexScript(hFile,iFlags);

    // Set text for the 'build report' button in Settings+Options page...to 'abort report!' until script executed!
    BuildReportButtonTitle("*Abort*", true);

    // Free Drill assistants if not consecutive call
    FreeDrillingAssistants(bFromWhatIf);

    // Launch script!
    GS::Gex::CSLEngine::GetInstance().RunScript(GS::Gex::Engine::GetInstance().GetAssistantScript());

    CGexGroupOfFiles::resetIgnoreAllMRR();

    //-- reset the report builder if any
    OFRManager::GetInstance()->Reset();

    return "ok";
}

///////////////////////////////////////////////////////////
// Deletes Drilling Classes when original call (or from settings page)...
///////////////////////////////////////////////////////////
void GexMainwindow::FreeDrillingAssistants(bool bFromWhatIf)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Free Drilling Assistants...");

#ifdef GCORE15334

    // Delete what-if assistant only if reset was not ordered by what-if wizard itself
    if(pWizardWhatIf != NULL && !bFromWhatIf)
    {
        pScrollArea->layout()->removeWidget(pWizardWhatIf);
        delete pWizardWhatIf;
        pWizardWhatIf = NULL;
    }

    // PAT-Man: Process file (outlier removal)
    if(mWizardWSPAT != NULL)
    {
        pScrollArea->layout()->removeWidget(mWizardWSPAT);
        delete mWizardWSPAT;
        mWizardWSPAT = NULL;
    }

    // PAT-Man: FT Process file (outlier removal)
    if(mWizardFTPAT != NULL)
    {
        pScrollArea->layout()->removeWidget(mWizardFTPAT);
        delete mWizardFTPAT;
        mWizardFTPAT = NULL;
    }
#endif

    // Do NOT delete the object 'pWizardChart' otherwise the gexReport->pPlot would crash !(memory shared)
    // Only empty the chart object to save memory, make sure it's a child of the scrollview !
    if(LastCreatedWizardChart() != NULL)
    {
        pScrollArea->layout()->removeWidget(LastCreatedWizardChart());
        //qDeleteAll(mWizardCharts);
        mWizardsHandler.Clear();
       // delete pWizardChart;
       // pWizardChart = NULL;
    }

    if(LastCreatedWizardTable() != NULL)
    {
        pScrollArea->layout()->removeWidget(LastCreatedWizardTable());
        //qDeleteAll(mWizardTables);
        mWizardsHandler.Clear();
    }

    if(LastCreatedWizardWafer3D() != NULL)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Deleting WizardDrill3D...");
        pScrollArea->layout()->removeWidget(LastCreatedWizardWafer3D());
        //qDeleteAll(mWizardWafer3Ds);
        mWizardsHandler.Clear();
    }
}

void GexMainwindow::OnCSLScriptFinished(const GS::Gex::CSLStatus &lStatus)
{
    // Greyed out abort script button
    actionStop->setEnabled(false);

    // If script just completed was a startup script...refresh the Options list
    if(lStatus.IsStartup() == true)
    {
        // Update menus to reflect settings given in startup script
        if(m_pOptionsCenter)	// refresh method used to update the gui from reportoptions object
        {
            if(!(RefreshOptionsCenter()))
            {
                GSLOG(SYSLOG_SEV_WARNING, "problem occured when refresh option center");
                GEX_ASSERT(false);
            }
        }

        // Startup script completed, ensure all 'FileOpen' will start from last path accessed in previous session
        pWizardOneFile->strWorkingPath = pWizardScripting->strGexConfLastFilePathSelected;
        pWizardCmpFiles->SetWorkingPath(pWizardScripting->strGexConfLastFilePathSelected);
        pWizardAddFiles->strWorkingPath = pWizardScripting->strGexConfLastFilePathSelected;
        pWizardMixFiles->strWorkingPath = pWizardScripting->strGexConfLastFilePathSelected;

        if (mPATReportFTGui)
            mPATReportFTGui->SetWorkingPath(pWizardScripting->strGexConfLastFilePathSelected);
    }

    // Report handle not available yet!
    if(gexReport == NULL)
        return;

    // Reset button's text to its original string (eg: 'build report' ...).
    if(mOnClose == false)
        BuildReportButtonTitle("",false,false);

    // If script not aborted, update current page
    if(lStatus.IsFailed() == false)
        ShowReportPage(lStatus.IsStartup());

    if(lStatus.IsFailed() == true && lStatus.IsStartup() == false)
    {
        if (!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            // Switch Tab view to the HTML scripting page (unless it is the startup script!)
            QString strTopBarPage = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
            strTopBarPage += GEX_BROWSER_SCRIPTING_TOPLINK;
            pageHeader->setSource(QUrl::fromLocalFile(strTopBarPage));
            // Shows Scripting page
            ScriptingLink();
        }
    }

    // If script just completed was a startup script, check plugin...
    if(lStatus.IsStartup() == true)
    {
        // If application not hidden, check for messages to display
        time_t lCurrentTime = time(NULL);
        if(lCurrentTime-pWizardScripting->lGexConfLastReminder >= 2592000L)
        {
            // It's been more than 30 days without a reminder....show it!
            switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
            {
                case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
                    GS::Gex::Message::information(
                                "", "********* Software Upgrade Offer (Credence users "
                                "ONLY) *********\n\nUpgrade your license to do more and "
                                "save money!\n\nVisit www.mentor.com web site or "
                                "contact " + QString(GEX_EMAIL_SALES) +
                                " for more details.");
                    break;
                case GS::LPPlugin::LicenseProvider::eLtxcOEM:			// OEM-Examinator for LTXC
                    GS::Gex::Message::information(
                                "", "********* Software Upgrade Offer (LTXC users ONLY) "
                                "*********\n\nUpgrade your license to do more and save "
                                "money!\n\nVisit www.mentor.com web site or contact "+
                                QString(GEX_EMAIL_SALES) + " for more details.");
                    break;
                default:
                    // Message("********* 30 days REMINDER *********\nWe keep "
                    // "improving our solutions for you.\nReleases and news "
                    // "are periodically\nposted to the www.mentor.com "
                    // "web site.\nYou should have a look today!");
                    break;
            }

            // Update date of last reminder (today!).
            pWizardScripting->lGexConfLastReminder = lCurrentTime;
        }
    }
}

void GexMainwindow::OnCSLScriptStarted()
{
    // Enable abort csl script button
    actionStop->setEnabled(true);
}

void GexMainwindow::UpdateProgressStatus(bool lSet, int lRange, int lStep, bool lShow)
{
    if (GexProgressBar)
    {
        if(lSet)
        {
            // Reset process bar to given range & step
            if(lRange == 0)
                GexProgressBar->hide();	// Hides process bar
            else if(lRange > 0)
            {
                GexProgressBar->show();	// Show progress bar
                GexProgressBar->setMaximum(lRange);
            }
            else
            {
                GexProgressBar->show();	// Show progress bar
                GexProgressBar->setMaximum(GexProgressBar->maximum() - lRange);
            }

            GexProgressBar->setTextVisible(lShow);		// Makes '%' value visible
        }

        // Update step position
        if(lStep < 0)
        {
            // Increment by on step...
            lStep = 1 + GexProgressBar->value();
            GexProgressBar->setValue(lStep);
        }
        else
        {
            // Update to the new given step
            GexProgressBar->setValue(lStep);
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, "Invalid pointer on progress bar widget");
    }
}


void GexMainwindow::HideProgress()
{
    if (GexProgressBar)
    {
        GexProgressBar ->hide();
    }
}


///////////////////////////////////////////////////////////
// Script completed with success: Show Report Page
///////////////////////////////////////////////////////////
void GexMainwindow::ShowReportPage(bool startup)
{
    // No report available!
    if((gexReport == NULL) || (gexReport->getReportOptions() == NULL))
        return;

    // If ExaminatorMonitoring stick to current visible page
    bool	bShowReportPage=false;
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eYieldMan:
        case GS::LPPlugin::LicenseProvider::eYieldManEnterprise:
        case GS::LPPlugin::LicenseProvider::ePATMan:
        case GS::LPPlugin::LicenseProvider::ePATManEnterprise:
            return;	// Do not switch to the report page
        default:						// Other modules rely on script flag.
            bShowReportPage = (ReportOptions.GetOption(QString("report"), QString("build"))).toBool();
            break;
    }

    // Check if script requested to show or not the report on completion
    if(pWizardScripting->strShowReportPage.isEmpty()==true)
        return;	// Do not switch to the report page
    if(pWizardScripting->strShowReportPage == "none")
        return;	// Do not switch to the report page
    if(pWizardScripting->strShowReportPage == "hide")
        return;	// Do not switch to the report page

    // Save path to Report file...in case we have to print it later on.
    strPrintReportPath = gexReport->reportAbsFilePath();

    //	QString of=ReportOptions.GetOption("output", "format").toString();
    QString strReportFormat = CGexReport::reportFormat(strPrintReportPath);

    // Report created manually (not from a command line script), then show it
    if(startup == false)
    {
        // If Interactive mode only (no report created), then display HTML 'Interactive only' home page.
        if(strReportFormat=="INTERACTIVE")	//(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_INTERACTIVEONLY)
        {
            ReportLink();
            return;
        }

        // Check if need to change the page displayed: If data analyzed and no report created....
        // eg: like when creating the test list, etc...!
        if(bShowReportPage == false)
            return;

        if( (strReportFormat=="DOC")||(strReportFormat=="PDF")||(strReportFormat=="PPT")||strReportFormat=="ODT" )
            //(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
            Wizard_OpenReport();
        else
            if(	(strReportFormat=="CSV") //(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_CSV)
                /*&&
                (GS::LPPlugin::ProductInfo::getInstance()->getProductID() != GEX_DATATYPE_ALLOWED_DATABASEWEB)*/)
                Wizard_OpenReport();
    }
    else
    {
        // Switch Tab view to the HTML report page
        QString strTopBarPage = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
        strTopBarPage += GEX_BROWSER_REPORT_TOPLINK;
        pageHeader->setSource(QUrl::fromLocalFile(strTopBarPage));
    }

    // Shows Report page
    ReportLink();
}

unsigned int CGexReport::findMappedTestName(long lTestType, unsigned int nTestNumber, char *szTestName)
{
    unsigned int nMappedTestNumber = nTestNumber;

    if (m_testNameMap.contains(lTestType))
    {
        TestNameMappingPrivate& testNameMap = m_testNameMap[lTestType];

        // Find the mapped test number
        nMappedTestNumber = testNameMap.findTestNumber(nTestNumber, szTestName);
    }
    else
    {
        TestNameMappingPrivate testNameMap;

        // Find the mapped test number
        nMappedTestNumber = testNameMap.findTestNumber(nTestNumber, szTestName);

        // Add to the test type map
        m_testNameMap.insert(lTestType, testNameMap);
    }

    return nMappedTestNumber;
}

unsigned int CGexReport::TestNameMappingPrivate::findTestNumber(unsigned int nTestNumber, char *szTestName)
{
    unsigned int nMappedTestNumber = nTestNumber;

    // Test name specified, see if already in Name list
    if(szTestName && *szTestName)
    {
        if(m_mapTestName.contains(szTestName) == false)
        {
            // No....so add entry!
            m_mapTestName.insert(szTestName, nTestNumber);

            // Test# mapping.
            m_mapTestNumber.insert(nTestNumber, nTestNumber);
        }
        else
        {
            // This test name already in list, get its root test# assigned to it.
            nMappedTestNumber = m_mapTestName.value(szTestName);
            m_mapTestNumber.insert(nTestNumber, nMappedTestNumber);
        }
    }
    else
    {
        if (m_mapTestNumber.contains(nTestNumber) == false)
            nMappedTestNumber = nTestNumber;
        else
            // No test name....specified (run#2 or higher....)....get root test#
            nMappedTestNumber = m_mapTestNumber.value(nTestNumber);
    }

    return nMappedTestNumber;
}

/******************************************************************************!
 * \fn checkIfReportExist
 * \brief Check if a file exists and return a new file name if necessary
 *        return true if the file exist else return false
 ******************************************************************************/
bool CGexReport::checkIfReportExist(const QString& filePath,
                                    QString&       newFilename)
{
    QFileInfo fileInfo(filePath);
    if (! fileInfo.exists())
    {
        return false;
    }

#ifndef GSDAEMON
    bool lOk;
    GS::Gex::Message::request("", QString("The %1 file already exists. Do you want to overwrite it ?")
                                  .arg(filePath), lOk);
    if (! lOk)
    {
        QString oldFileName = fileInfo.baseName();
        bool    ok = false;
        do
        {
            newFilename = QInputDialog::getText(
                              0,
                              GS::Gex::Engine::GetInstance().
                              Get("AppFullName").toString(),
                              "File Name",
                              QLineEdit::Normal,
                              "",
                              &ok);
            if (! ok)
            {
                return false;
            }
        }
        while (newFilename.isEmpty() && newFilename != oldFileName);
        newFilename = newFilename + "." + fileInfo.suffix();
        return true;
    }
#else
    int suffix = 0;
    do
    {
        ++suffix;
        newFilename = filePath + QString(".%1").arg(suffix);
        fileInfo.setFile(newFilename);
    }
    while (fileInfo.exists());
    GSLOG(SYSLOG_SEV_WARNING,
           QString("The %1 file already exists, using %2").
           arg(filePath).arg(newFilename).toLatin1().constData());
#endif

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexSiteLimits
//
// Description	:	This class holds limits for a specific site
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexSiteLimits::CGexSiteLimits(double dLowLimit /*= -C_INFINITE*/, double dHighLimit /*= C_INFINITE*/)
    : m_dLowLimit(-C_INFINITE), m_dHighLimit(C_INFINITE), m_timeStamp(0)
{
    setLimits(dLowLimit, dHighLimit);
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CGexSiteLimits::setLowLimit(double dLowLimit)
//
// Description	:	Sets the lower limit
//
///////////////////////////////////////////////////////////////////////////////////
void CGexSiteLimits::setLowLimit(double dLowLimit)
{
    if (dLowLimit != -C_INFINITE)
    {
        m_dLowLimit = dLowLimit;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CGexSiteLimits::setHighLimit(double dHighLimit)
//
// Description	:	Sets the higher limit
//
///////////////////////////////////////////////////////////////////////////////////
void CGexSiteLimits::setHighLimit(double dHighLimit)
{
    if (dHighLimit != C_INFINITE)
    {
        m_dHighLimit = dHighLimit;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CGexSiteLimits::setLimits(double dLowLimit, double dHighLimit)
//
// Description	:	Sets the higher and lower limits
//
///////////////////////////////////////////////////////////////////////////////////
void CGexSiteLimits::setLimits(double dLowLimit, double dHighLimit)
{
    if (dLowLimit != -C_INFINITE && dHighLimit != C_INFINITE)
    {
        if (dLowLimit > dHighLimit)
        {
            double dTmpLimit = dLowLimit;

            dLowLimit	= dHighLimit;
            dHighLimit	= dTmpLimit;
        }
    }

    setLowLimit(dLowLimit);
    setHighLimit(dHighLimit);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CGexSiteLimits::limits(double& dLowLimit, double& dHighLimit) const
//
// Description	:	Retrieves the higher and lower limits
//
///////////////////////////////////////////////////////////////////////////////////
void CGexSiteLimits::limits(double& dLowLimit, double& dHighLimit) const
{
    dLowLimit	= m_dLowLimit;
    dHighLimit	= m_dHighLimit;
}

void CGexSiteLimits::setLimitTimeStamp(time_t limitTimeStamp)
{
    m_timeStamp = limitTimeStamp;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGageWarning
//
// Description	:	This class holds gage warnings
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGageWarning::CGageWarning() : m_clrColor(Qt::white), m_nFileLine(-1), m_bSyntaxError(false)
{
}

bool CGexReport::UpdateOptions(CReportOptions *lOptions)
{
    bool lOk = true;

    QString lTestMergeRule = lOptions->GetOption("dataprocessing","duplicate_test").toString();

    if (lTestMergeRule == "merge")
        mTestMergeRule = TEST_MERGE_NUMBER;
    else if (lTestMergeRule == "merge_name")
        mTestMergeRule = TEST_MERGE_NAME;
    else if (lTestMergeRule == "no_merge")
        mTestMergeRule = TEST_NO_MERGE;
    else
    {
        lOk = false;
        GSLOG(SYSLOG_SEV_WARNING, QString(" warning : unknown duplicate test merge rule : '%1' !").arg(
                 lTestMergeRule).toLatin1().constData());
    }

    // Update number formatting options
    QString lScientificNotation = lOptions->GetOption("output", "scientific_notation").toString();
    int lPrecision = lOptions->GetOption("output", "precision").toInt();
    mNumberFormat.setOptions((lScientificNotation=="turn_on")?'e':' ', lPrecision);

    return lOk;
}

