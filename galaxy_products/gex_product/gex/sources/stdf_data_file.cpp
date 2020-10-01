#include "stdfrecord.h"
#include "stdfrecords_v4.h"
#include "stdf_head_and_site_number_decipher.h"
#include "abstract_csv_converter.h"
#include "report_options.h"
#include "gex_group_of_files.h"
#include "report_build.h"
#include "gexabstractdatalog.h"
#include <QMap>
#include "gqtl_log.h"
#include "csl/csl_engine.h"
#include <QString>
#include "license_provider.h"
#include "product_info.h"
#include "engine.h"
#include "gex_oem_constants.h"
#include "gs_types.h"
#include "gex_report.h"

#include "stdf_data_file.h"

extern CGexReport*    gexReport;  // Handle to report class
extern CReportOptions ReportOptions;

StdfDataFile::StdfDataFile(QString & fileName,
             FILE*  advFile,
             CReportOptions* globalReportOptions,
             const QString & waferToExtract):
                mReportOptions(globalReportOptions),
                mMRRCount(0),
                mAdvFile(advFile),
                mTestDef(true),
                mPTRCount(0),
                mFileName(fileName),
                mWaferToExtract(waferToExtract),
                mRecordIndex(0),
                mIgnoreNextWaferRecord(false),
                ptPinmapList(0),
                mCurrentIndexWaferMap(-1),
                mMultiWaferMap(false)
{ }

StdfDataFile::~StdfDataFile()
{
    mStdfFile.Close();

    // Destroy Pinmap list
    CPinmap* ptNextPinmap = 0;
    CPinmap* ptPinmapCell = ptPinmapList;
    while (ptPinmapCell != NULL)
    {
        ptNextPinmap = ptPinmapCell->ptNextPinmap;
        delete ptPinmapCell;
        ptPinmapCell = ptNextPinmap;
    }
    ptPinmapList = 0;

}


void StdfDataFile::loopOverFileInGroup(int siteNumber, T_Record type, int lPass, CPartInfo *partInfo)
{
    if(mFileInGroupsBySite.contains(siteNumber) == false || siteNumber == -1)
    {
        //-- Loop over all pFile
        FileInGroupContainerIter lIter(mFileInGroupsBySite.begin()), lIterEnd(mFileInGroupsBySite.end());
        for(;lIter != lIterEnd;++lIter)
        {
            QList<CGexFileInGroup*> lList = lIter.value();
            QList<CGexFileInGroup*>::iterator lIterList(lList.begin()), lIterListEnd(lList.end());
            for(; lIterList != lIterListEnd; ++lIterList)
            {
                 if((*lIterList)->bIgnoreThisSubLot)
                     continue;
                 initFileInGroup(*lIterList, type, lPass, partInfo);
            }
        }
    }
    else
    {
        //-- Loop over pFile with the same site
        QList<CGexFileInGroup*> lList = mFileInGroupsBySite[siteNumber];
        QList<CGexFileInGroup*>::iterator lIterList(lList.begin()), lIterListEnd(lList.end());
        for(; lIterList != lIterListEnd; ++lIterList)
        {
            if((*lIterList)->bIgnoreThisSubLot)
                continue;
            initFileInGroup(*lIterList, type, lPass, partInfo);
        }
    }
}

void StdfDataFile::initFileInGroup(CGexFileInGroup *lFile, T_Record type, int lPass,  CPartInfo *partInfo)
{
     //-- take the test, hardbinning and softbinning from the parent group
    lFile->initPtrFromParentGroup();

    switch(type)
    {
        case T_PCR: initFileInGroupPCR(lFile); break;
        case T_PIR: initFileInGroupPIR(lFile, *partInfo); break;
        case T_PRR: initFileInGroupPRR(lFile, lPass); break;
        case T_PTR: initFileInGroupPTR(lFile, lPass); break;
        case T_WIR :
                    initFileInGroupWIR(lFile);
                    lFile->m_iWIRCount++;
                    break;
        case T_WRR:
                    initFileInGroupWRR(lFile, lPass);
                    lFile->m_iWRRCount++;
                    break;
        case T_WCR: initFileInGroupWCR(lFile); break;
        case T_PMR: initFileInGroupPMR(lFile);break;
        case T_TSR: initFileInGroupTSR(lFile, lPass); break;
        case T_MPR: initFileInGroupMPR(lFile, lPass); break;
        case T_FTR: initFileInGroupFTR(lFile, lPass);break;
       default: break;
    }
    //-- update the test, hardbinning and softbinning from the parent group
    lFile->initPtrParentGroup();
}

//!-------------------------------!//
//!     initFileInGroupPIR        !//
//!-------------------------------!//
void StdfDataFile::initFileInGroupPIR(CGexFileInGroup *lFile, CPartInfo &PartInfo)
{
    lFile->m_iPIRCount++;
    CTest		*ptTestCell;
        // Structure to hold part info: DieXY, Exec time, Tests executed, partID, etc...
    unsigned	uMapIndex;

    // Clear Run failing type
    lFile->getMapFailingSiteDetails()[PartInfo.m_site] = 0;

    uMapIndex = (PartInfo.bHead << 16) | PartInfo.m_site;	// Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR

    lFile->getMappingSiteHead()[uMapIndex]= 1;	// Tells we enter in a PIR/PRR block

    // If filter is: First/Last testing instance (Wafer sort only)
    // then keep track of Part# for this given site...
    //TOCHECK
    switch(lFile->iProcessBins)
    {
    case GEX_PROCESSPART_FIRSTINSTANCE:
    case GEX_PROCESSPART_LASTINSTANCE:

        // First time call: reset buffer
        if(lFile->getPartProcessed().partNumber(PartInfo.m_site) == 1)
        {
            lFile->getFirstInstanceDies().clear();
            lFile->getLastInstanceDies().clear();
            lFile->getFirstInstance().clear();
            lFile->getLastInstance().clear();
        }
    }

    // Initializes the mapping offset to know which site is mapped to which Part#...
    lFile->getPartProcessed().addSite(PartInfo.m_site);

    // If we reach this point, this means we process this type of part (site# & head#)
    // then do pre-processing cleanup...unless we already did it (in previous PIR if we have multiple PIR nested)
    if(lFile->getPartProcessed().PIRNestedLevel() == 1)
    {
        ptTestCell = lFile->getTestList();
        // Reset 'tested' flag for each test in the test list
        while(ptTestCell != NULL)
        {
            ptTestCell->bTestExecuted = false;
            ptTestCell->ldTmpSamplesExecs=0;
            ptTestCell = ptTestCell->GetNextTest();
        };
    }
}

//!-------------------------------!//
//!     initFileInGroupPRR        !//
//!-------------------------------!//
void StdfDataFile::initFileInGroupPRR(CGexFileInGroup *lFile, int lPass)
{
    CTest       *ptTestCell=0;
    CTest       *ptFailTestCell = NULL;
    CPartInfo   PartInfo;		// Structure to hold part info: DieXY, Exec time, Tests executed, partID, etc...
    BYTE        cPassFail;
    int         iBin=0;
    bool        bIgnoreSite=false;
    bool        bIgnorePart=false;	// Will remain true if site# filter not matching, or if part must not be processed
    bool        bPartFailed=false;
    unsigned    uMapIndex=0;
    long        lPartID				= -1;		// Will receive PRR partID number (unless not available).
    long        lInternalPartID		= -1;
    bool        bNumericalPartID	= false;

    PartInfo.bHead  = mPRRRecord.mHead;
    PartInfo.m_site = mPRRRecord.mSite;

    if(lFile == 0)
        return ;

    lFile->m_iPRRCount++;

    uMapIndex = (int)(PartInfo.bHead << 16) | PartInfo.m_site;	// Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR

    lFile->getMappingSiteHead()[uMapIndex]= 0;		// Reset the flag saying we enter in a PIR/PRR block
    //mTestDefinition = true;

    // Check if a test failied in this flow.
    if(lFile->getMapFailTestCell().contains(PartInfo.m_site))
        ptFailTestCell = lFile->getMapFailTestCell()[PartInfo.m_site];

    bPartFailed = (int)mPRRRecord.partFlag & 0x0008;
    if(bPartFailed)
    {
        PartInfo.bPass = false;	// Run was FAIL
        cPassFail = 'F';
    }
    else
    {
        PartInfo.bPass = true;	// Run was PASS
        cPassFail = 'P';
    }

    PartInfo.iTestsExecuted = mPRRRecord.numberOfTests;
    if(PartInfo.iTestsExecuted == 65535)	// Some testers (ie Eagle) set this field to 65535 instead of 0 when no tests were executed (only limits defined)
        PartInfo.iTestsExecuted = 0;
    PartInfo.iHardBin = mPRRRecord.hbin;
    PartInfo.iSoftBin = mPRRRecord.sbin;

    if(lPass ==1)
        lFile->getSBinHBinMap().insertMulti(PartInfo.iSoftBin , PartInfo.iHardBin );

    // Get bin result
    if(PartInfo.iSoftBin != 65535)
        iBin = PartInfo.iSoftBin;
    else
        iBin = PartInfo.iHardBin;

    // Save part #
    PartInfo.lPartNumber = lFile->getPartProcessed().partNumber(PartInfo.m_site);

    // Update part counter, etc...
    bIgnorePart = lFile->UpdateBinResult(&PartInfo);

    // Check if Have to Collect Binning Trend data (normally done in HBR, S BR, but if Summary is disabled, we have to do it within PRRs!)
    if((bIgnorePart==false) && lFile->getBinComputation()== CGexFileInGroup::SAMPLES )
    {
        lFile->AdvCollectBinningTrend(&lFile->m_ParentGroup->cMergedData.ptMergedSoftBinList,PartInfo.iSoftBin,1);
        lFile->AdvCollectBinningTrend(&lFile->m_ParentGroup->cMergedData.ptMergedHardBinList,PartInfo.iHardBin,1);
    }

    // Save Soft-Bin parameter info.
    if(PartInfo.iSoftBin != 65535)
    {
        // Save failing bin into relevant test structure if this run failed.
        if(ptFailTestCell != NULL)
        {
            ptFailTestCell->iFailBin = PartInfo.iSoftBin;
            ptFailTestCell = NULL;
        }
    }

    // Save Hard-Bin parameter info.
    if(PartInfo.iHardBin != 65535)
    {
        // Save failing bin into relevant test structure if this run failed.
        if(ptFailTestCell != NULL)
        {
            ptFailTestCell->iFailBin = PartInfo.iHardBin;
            ptFailTestCell = NULL;
        }
    }

    PartInfo.iDieX = mPRRRecord.dieX;
    PartInfo.iDieY = mPRRRecord.dieY;

    // 2-Aug-2004 PhL: Test removed as ITS9000 testers sometimes have this field set to 0 and still valid wafermap/Bin info.
#if 0
    // If no tests performed, force Die location = Unrelevant
    //if(PartInfo.iTestsExecuted <= 0)
    //	PartInfo.iDieX = PartInfo.iDieY = -32768;
#endif
    switch(mReportOptions->iWafermapType)
    {
    case GEX_WAFMAP_SOFTBIN:
    case GEX_WAFMAP_STACK_SOFTBIN:
    case GEX_WAFMAP_ZONAL_SOFTBIN:
        // Wafer map on: SOFT  bin
        iBin = PartInfo.iSoftBin;
        break;
    case GEX_WAFMAP_HARDBIN:
    case GEX_WAFMAP_STACK_HARDBIN:
    case GEX_WAFMAP_ZONAL_HARDBIN:
        // Wafer map on: HARD  bin
        iBin = PartInfo.iHardBin;
        break;
    }

    // Read test time, and get test time info if GOOD binning (bit3,4=0)
    if(mPRRRecord.timeRes)
    {
        // Save execution time
        PartInfo.lExecutionTime = mPRRRecord.time;

        lFile->lAverageTestTime_All += mPRRRecord.time;	// Counts in Ms.
        lFile->lTestTimeParts_All++;			// Number of parts used to compute test time.

        // If Passing part, then use this exec time to compute the everage testing time of good devices.
        if((lPass == 1) && ((mPRRRecord.partFlag & 0x18) == 0) && (mPRRRecord.time > 0))
        {
            lFile->lAverageTestTime_Good += mPRRRecord.time;	// Counts in Ms.
            lFile->lTestTimeParts_Good++;			// Number of parts used to compute test time.
        }
    }
    else
    {
        // No more data in record
        PartInfo.lExecutionTime=0;
    }

    // Create Parameter entry for Testing site
    lFile->UpdateCustomParameter(bIgnorePart, "Testing_Site parameter",GEX_TESTNBR_OFFSET_EXT_TESTING_SITE,(float)PartInfo.m_site,&PartInfo);

    // Create Parameter entry for TestTemperature (if a custom value was specified from the 'File Properties' page)
    if(lFile->m_lfTemperature > C_NO_TEMPERATURE)
        lFile->UpdateCustomParameter(bIgnorePart, "Test_Temperature parameter",GEX_TESTNBR_OFFSET_EXT_TEMPERATURE,(float)lFile->m_lfTemperature,&PartInfo);

    // Create Parameter entry for Soft_Bin
    if(PartInfo.iSoftBin != 65535 || (lFile->getStdCompliancy()==CGexFileInGroup::FLEXIBLE))
        lFile->UpdateCustomParameter(bIgnorePart, "Soft_Bin parameter",GEX_TESTNBR_OFFSET_EXT_SBIN,(float)PartInfo.iSoftBin,&PartInfo);

    // Create Parameter entry for Hard_Bin
    if(PartInfo.iHardBin != 65535 || (lFile->getStdCompliancy()==CGexFileInGroup::FLEXIBLE))
        lFile->UpdateCustomParameter(bIgnorePart, "Hard_Bin parameter",GEX_TESTNBR_OFFSET_EXT_HBIN,(float)PartInfo.iHardBin,&PartInfo);

    // Create Parameter entry for Test time (in sec.).
    if(mPRRRecord.time != 0)
        lFile->UpdateCustomParameter(bIgnorePart, "Test_Time parameter",GEX_TESTNBR_OFFSET_EXT_TTIME,((float)mPRRRecord.time)/1e3,&PartInfo);

    PartInfo.setPartID(mPRRRecord.partId);

    lPartID = PartInfo.getPartID().toLong(&bNumericalPartID);

    if (lFile->getMapPartId().contains(PartInfo.getPartID()))
        lInternalPartID = lFile->getMapPartId().value(PartInfo.getPartID());
    else
    {
        lInternalPartID = lFile->m_lCurrentInternalPartID++;

        if (PartInfo.getPartID().isEmpty() == false)
            lFile->getMapPartId().insert(PartInfo.getPartID(), lInternalPartID);
    }

    // Update partID list into results list (if PartID list is enabled)
    if(bNumericalPartID == false)
        lPartID = lInternalPartID;	// If no PartID defined, then force it to run# instead.

    if (PartInfo.getPartID().isEmpty())
        PartInfo.setPartID(QString::number(lPartID));

    // Save PartID
    lFile->UpdateCustomParameter(bIgnorePart, "Part_ID parameter",GEX_TESTNBR_OFFSET_EXT_PARTID,lInternalPartID,&PartInfo);

    PartInfo.setPartText(mPRRRecord.partTxt);
    if(mPRRRecord.partTxt[0])
        lFile->m_strPart_TXT += mPRRRecord.partTxt;		// Keep list of PartTXT strings.

    PartInfo.strPartRepairInfo = mPRRRecord.partFix;

    // Update wafermap info if part note filtered, or if wafermap must be FULL wafermap (then ignore part filtering!)
    if((bIgnorePart==false) || lFile->getFullWafermap())
        lFile->UpdateWaferMap(iBin,&PartInfo, lInternalPartID);

    // If no binning summary found, or we must use PRR for Bin count, we have to build it now!
    if( (bIgnorePart==false) && (lPass==2) &&
            ( (lFile->bBinningSummary == false)
              || (lFile->getBinComputation()==CGexFileInGroup::SAMPLES)
              || (lFile->getBinComputation()==CGexFileInGroup::WAFER_MAP)
              )
            )
    {
        lFile->AddBinCell(&lFile->m_ParentGroup->cMergedData.ptMergedSoftBinList, PartInfo.m_site,PartInfo.iDieX,PartInfo.iDieY,PartInfo.iSoftBin,PartInfo.iHardBin,cPassFail,1,true,true,"",false);	// Add Soft Bin inf to list
        lFile->AddBinCell(&lFile->m_ParentGroup->cMergedData.ptMergedHardBinList, PartInfo.m_site,PartInfo.iDieX,PartInfo.iDieY,PartInfo.iSoftBin,PartInfo.iHardBin,cPassFail,1,true,true,"",true);	// Add Hard Bin inf to list
    }
    float fData;
    // Create Parameter entry for Die_X
    if(PartInfo.iDieX >= 32768)
        fData = PartInfo.iDieX - 65536.0;
    else
        fData = PartInfo.iDieX;
    lFile->UpdateCustomParameter(bIgnorePart, "Die_X parameter",GEX_TESTNBR_OFFSET_EXT_DIEX,fData,&PartInfo);

    // Create Parameter entry for Die_Y
    if(PartInfo.iDieY >= 32768)
        fData = PartInfo.iDieY - 65536.0;
    else
        fData = PartInfo.iDieY;
    lFile->UpdateCustomParameter(bIgnorePart, "Die_Y parameter",GEX_TESTNBR_OFFSET_EXT_DIEY,fData,&PartInfo);

    // If Wafersort & filtering over First test instance or Last test instance...
    if(lPass == 2 && (lFile->iProcessBins == GEX_PROCESSPART_FIRSTINSTANCE || lFile->iProcessBins == GEX_PROCESSPART_LASTINSTANCE))
    {
        int nRunIndex = lFile->getPartProcessed().runIndexFromSite(PartInfo.m_site);

        if(nRunIndex >= 0)
        {
            QString strKey;

            CGexFileInGroup::PartIdentification lPartIdentification = lFile->getPartIdentification();

            if (lFile->getPartIdentification() == CGexFileInGroup::AUTO)
            {
                if (lFile->getWaferMapData().bWirExists)
                    lPartIdentification = CGexFileInGroup::XY;
                else
                    lPartIdentification = CGexFileInGroup::PARTID;
            }

            if (lPartIdentification == CGexFileInGroup::PARTID)
                strKey = PartInfo.getPartID();
            else if (lPartIdentification == CGexFileInGroup::XY)
            {
                if ((PartInfo.iDieX != 32768) && (PartInfo.iDieY != 32768))
                    strKey = QString::number(PartInfo.iDieX) + "-" + QString::number(PartInfo.iDieY);
            }

            if (strKey.isEmpty() == false)
            {
                // First instance
                if(lFile->getFirstInstanceDies().contains(strKey) == false)
                    lFile->getFirstInstanceDies()[strKey] = nRunIndex;	// Run Index for first instance of this die.

                // Instance already present, overload it.
                lFile->getLastInstanceDies()[strKey] = nRunIndex;	// Run Index for last instance of this die.
            }
        }
    }

    // If this site must be ignored, simply do not save temporary Min/Max/Execs values for this site...
    if(bIgnoreSite == false && bIgnorePart == false)
    {
        // Datalog Binning if activated.
        if (lPass == 2)
        {
            if (GexAbstractDatalog::existInstance())
                GexAbstractDatalog::pushPartInfo(&PartInfo);
            else if((mReportOptions->getAdvancedReport() == GEX_ADV_DATALOG) && (lFile->lTestsInDatalog > 0) && (mAdvFile != NULL))
                lFile->WriteDatalogBinResult(iBin,&PartInfo);// Bin + Part ID+DieLocation
        }

        // Keep track of the number of samples in this sub-lot
        if (lPass == 2)
            lFile->m_lSamplesInSublot++;
    }

    if(lPass == 1)
    {
        ptTestCell = lFile->getTestList();
        // If part is an outlier...
        if(bIgnorePart == true)
        {
            // Don't reinitialize parameters until parsing the last PRR in the case of nested PRR
            if (lFile->getPartProcessed().PIRNestedLevel() == 1)
            {
                // Reset 'tested' flag for each test in the test list
                while(ptTestCell != NULL)
                {
                    ptTestCell->bTestExecuted = false;
                    ptTestCell->ldTmpSamplesExecs=0;
                    ptTestCell->lfTmpSamplesMin = ptTestCell->lfSamplesMin;
                    ptTestCell->lfTmpSamplesMax = ptTestCell->lfSamplesMax;

                    // If we exit from the last PRR in the PIR/PRR block, clear the multi result count for this test
                    if (lFile->getPartProcessed().PIRNestedLevel() == 1)
                        ptTestCell->clearMultiResultCount();

                    ptTestCell = ptTestCell->GetNextTest();
                };
            }
        }
        else
        {
            // This part belongs to our filter...so update min/max and count values
            long	lTestsInPart=0;
            while(ptTestCell != NULL)
            {
                // check if update fields or not !
                if(ptTestCell->bTestExecuted == true)
                {
                    // count total of datalogs in this run
                    lTestsInPart++;

                    // This test was executed...if this part matches the filter
                    ptTestCell->ldSamplesValidExecs   += ptTestCell->ldTmpSamplesExecs;	// Updates sample count for this test...test may have been executed more than once in a run!
                    ptTestCell->ldTmpSamplesExecs     = 0;
                    ptTestCell->GetCurrentLimitItem()->lfHistogramMin        = ptTestCell->lfTmpHistogramMin;
                    ptTestCell->lfSamplesMin          = ptTestCell->lfTmpSamplesMin;
                    ptTestCell->GetCurrentLimitItem()->lfHistogramMax        = ptTestCell->lfTmpHistogramMax;
                    ptTestCell->lfSamplesMax          = ptTestCell->lfTmpSamplesMax;
                    ptTestCell->mHistogramData        = GS::Gex::HistogramData(lFile->mHistoBarsCount,
                                                                               ptTestCell->GetCurrentLimitItem()->lfHistogramMin,
                                                                               ptTestCell->GetCurrentLimitItem()->lfHistogramMax) ;

                    // Reset 'tested' flag.
                    ptTestCell->bTestExecuted = false;
                }
                else
                {
                    // No valid result for this run, only increment buffer size for NaN value
                    ptTestCell->ldSamplesExecs++;
                }

                // If we exit from the last PRR in the PIR/PRR block, clear the multi result count for this test
                if (lFile->getPartProcessed().PIRNestedLevel() == 1)
                    ptTestCell->clearMultiResultCount();

                // Resets temporary samples buffer...to make sure to restrat from clean situation
                ptTestCell = ptTestCell->GetNextTest();
            };

            // If datalogs in this run OR PRR says tests in run, then consider this as a valid sample...
            if(lTestsInPart || PartInfo.iTestsExecuted)
                lFile->ldTotalPartsSampled++;
        }

        // Save Pass/Fail flag so we can create the lilst of PASS Bins and FAIL bins
        lFile->getParentGroup()->cMergedData.mSBinPassFailFlag[PartInfo.iSoftBin] = (int) PartInfo.bPass;
        lFile->getParentGroup()->cMergedData.mHBinPassFailFlag[PartInfo.iHardBin] = (int) PartInfo.bPass;
    }

    if(lPass == 2)
    {
        lFile->getParentGroup()->cMergedData.UpdateBinSiteCounters(PartInfo);
    }

    if (bIgnoreSite == false)
        // Increment the part number
        lFile->getPartProcessed().nextPartNumber();
}

//!-------------------------------!//
//!         initFileInGroupPTR    !//
//!-------------------------------!//
bool StdfDataFile::initFileInGroupPTR(CGexFileInGroup* lFile, int lPass)
{
    int		test_flg;
    float	fResult;
    double	lfLowLimit=-C_INFINITE,lfHighLimit=C_INFINITE;
    bool	bValideResult=true;
    bool	bTestIsFail=false, bOutsideLimit=false;
    bool	bAlarmResult=false;
    bool	bIgnoreSite=false;
    bool	bIsOutlier;
    bool	bStrictLL=true, bStrictHL=true;
    unsigned uMapIndex;
    int iProcessSite = lFile->iProcessSite;
    PTRFileRecord* lPTRMemory = &mPTRRecord;

    CTestResult::PassFailStatus	ePassFailStatus	= CTestResult::statusUndefined;
    CTest *						ptTestCell		= NULL;	// Pointer to test cell to receive STDF info.

    // Check if this PTR is within a valid PIR/PRR block.
    uMapIndex = (lPTRMemory->mHead << 16) | lPTRMemory->mSite;	// Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR

    if((lFile->getStdCompliancy()== CGexFileInGroup::STRINGENT) && lFile->getMappingSiteHead()[uMapIndex] != 1)
        bIgnoreSite = true;

    // Ignore PTR if filtering set to 'Bin data only'
    if(lFile->iProcessBins == GEX_PROCESSPART_NO_SAMPLES)
        return true;

    // Check if current PTR (test result) belongs to a part that we filter ?
    // If this part has a binning the User doesn't want to process, then
    // we skip this record...
    if((lPass == 2) && (lFile->getPartProcessed().IsFilteredPart(mReportOptions, lPTRMemory->mSite,
                                                                 false, lFile->iProcessBins) == true))
        return true;

    // Ignore test if we are in 'Stop on Fail' mode, and current site already failed!
    if((lPass == 2) && (lFile->getFailCountMode() == CGexFileInGroup::FAILCOUNT_FIRST) &&
            (lFile->getMapFailTestCell() .contains(lPTRMemory->mSite)))
        return true;

    test_flg = (int) lPTRMemory->testFlag & 0xff;
    fResult = lPTRMemory->testResult;

    // Check if test limits are strict
    if(lPTRMemory->paramFlg & 0100)
        bStrictLL = false;
    if(lPTRMemory->paramFlg & 0200)
        bStrictHL = false;

    // Check if Pass/Fail indication is valid (bit 6 cleared)
    // Only if Pass/Fail rule option is checked
    if (lFile->getUsePassFailFlag() && (test_flg & 0x40) == 0x00)
    {
        // If bit 7 set, then test is fail, otherwise test is pass
        ePassFailStatus = (test_flg & 0x80) ? CTestResult::statusFail : CTestResult::statusPass;
    }

    // Check if test result is valid
    if(lFile->getStdCompliancy()==CGexFileInGroup::STRINGENT)
    {
        // In stringent mode (Data processing options->Handling STDF Compliancy issues = Stringent),
        // consider test result is valid based on STDF specs:
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        // The RESULT value is considered useful only if all the following bits from TEST_FLG and PARM_FLG are 0:
        //
        // TEST_FLG	bit 0 = 0	no alarm
        // bit 1 = 0	value in result field is valid
        // bit 2 = 0	test result is reliable
        // bit 3 = 0	no timeout
        // bit 4 = 0	test was executed
        // bit 5 = 0	no abort
        // PARM_FLG	bit 0 = 0	no scale error
        // bit 1 = 0	no drift error
        // bit 2 = 0	no oscillation
        //
        // If any one of these bits is 1, then the PTR result should not be used.
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        if((test_flg & 077) || (lPTRMemory->paramFlg & 07))
            bValideResult = false;
        else
            bValideResult = true;
    }
    else
    {
        // In flexible mode (Data processing options->Handling STDF Compliancy issues = Flexible),
        // consider test result is valid only based on TEST_FLG.bit1
        bValideResult = ((test_flg & 0x12) == 0);
    }

    if(lPTRMemory->testResultNan)
        bValideResult = false;

    // Ignore test result if must compute statistics from summary only
    if (lFile->getStatsComputation() == CGexFileInGroup::FROM_SUMMARY_ONLY)
        bValideResult = false;

    // Check if test has an alarm
    if(test_flg & 1)
        bAlarmResult = true;

    // Read Test definition: name, Limits, etc...
    if(lPTRMemory->testDef.isEmpty() == false)
    {
        // Replace ',' with ';' for csv output
        lFile->FormatTestName(lPTRMemory->testDef);

        if (lFile->getTestMergeRule() != TEST_MERGE_NUMBER)
        {
            if (/*lPass == 1 &&*/ lPTRMemory->testDef.isEmpty() == false)
            {

                QMap<int, QString>::iterator lIter = lFile->getTestCorrespondance().lowerBound(lPTRMemory->testNumber);
                if(lIter == lFile->getTestCorrespondance().end()  || lIter.key() != lPTRMemory->testNumber)
                    lFile->getTestCorrespondance().insert(lIter, lPTRMemory->testNumber, lPTRMemory->testDef);
            }
            else
            {
               // if (lFile->getTestCorrespondance().count() > 0)
                {
                    QMap<int, QString>::iterator it = lFile->getTestCorrespondance().find(lPTRMemory->testNumber);
                    if (it != lFile->getTestCorrespondance().end())
                    {
                        lPTRMemory->testDef = it.value();
                    }
                }
            }
        }

        // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
        if (lFile->FindTestCell(lPTRMemory->testNumber, GEX_PTEST, &ptTestCell, true,true,  lPTRMemory->testDef) !=1)
            return true;  // Error

        // Formats test name (in case to long, or leading spaces)..only take non-empty strings (since at init. time it is already set to empty)

        //GCORE-17470 to distinguish MPR from cases where several PTRs will be merged because they have same nb AND same name
        if(ptTestCell->GetCurrentLimitItem() != NULL)
            ptTestCell->GetCurrentLimitItem()->setIsParametricItem(false);

     //   if(firstReadTest == 0)
       //     firstReadTest = ptTestCell;

        if (ptTestCell->strTestName.isEmpty() == true)
        {
            // Overwrite Cell name only if it was not previously initialized...
            if(lPTRMemory->testDef.isEmpty() == false)
            {
                /*if(firstReadTest)
                {
                   ptTestCell->strTestName =  (firstReadTest)->strTestName;
                }
                else
                {*/
                    // Trim test name to remove unnecessary spaces at
                    // beginning and end (for QA) [BG 08/27/2012]
                    ptTestCell->strTestName = lPTRMemory->testDef.trimmed();
               // }
            }
        }
    }
    else
    {
        if (lFile->getTestMergeRule() != TEST_MERGE_NUMBER)
        {
            if (lPTRMemory->testDef.isEmpty())
            {

                QMap<int, QString>::iterator it = lFile->getTestCorrespondance().find(lPTRMemory->testNumber);
                if (it != lFile->getTestCorrespondance().end())
                {
                    lPTRMemory->testDef = it.value();
                }
            }

            if(lFile->FindTestCell(lPTRMemory->testNumber, GEX_PTEST, &ptTestCell, true, true, lPTRMemory->testDef) !=1)
                return true;  // Error
        }
        else
        {
            if(lFile->FindTestCell(lPTRMemory->testNumber, GEX_PTEST, &ptTestCell) !=1)
                return true;
        }
    }

    // Add a count for this site
    ptTestCell->addMultiResultCount(lPTRMemory->mSite);

    // OPT_FLAG
    if( lPTRMemory->optFlagRes)
    {
        // RES_SCALE
        if(lPTRMemory->resScalRes)
        {
            // res-scal is valid and never defined before, save it in test definition
            if(((lPTRMemory->optFlag & 0x1) == 0) &&
                    ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)))
                ptTestCell->res_scal = lPTRMemory->resScal;

        }
    }

    if(bIgnoreSite || ((iProcessSite >=0) && (iProcessSite != lPTRMemory->mSite) ) )
    {
        bIgnoreSite = true;
        goto ReadTestDefinitionV4;	// read test static info (but not test limits).
    }

    if((bValideResult == true) && (lPass == 2))
    {
        // PASS2:

        // Check if this test is an outlier
        bIsOutlier = lFile->PartIsOutlier(ptTestCell,fResult);

        // Ignore STDF FAIL flag if What-If limits defined!
        if(ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
            ePassFailStatus = CTestResult::statusUndefined;

        // Check if test is fail
        bTestIsFail = lFile->IsTestFail(ptTestCell, fResult, ePassFailStatus, bOutsideLimit);

        if (bTestIsFail && !bOutsideLimit)
            lFile->getMapFailedTestGoodResult()[lPTRMemory->mSite].append(ptTestCell);
        else if (!bTestIsFail && bOutsideLimit)
            lFile->getMapPassedTestBadResult()[lPTRMemory->mSite].append(ptTestCell);

        // Keep track of failing count (unless it's an outlier)
        if((bIsOutlier == false) && lFile->CountTestAsFailure(lPTRMemory->mSite,ptTestCell,GEX_PTEST,fResult,bTestIsFail) == true)
        {
            // Update failing count
            ptTestCell->GetCurrentLimitItem()->ldSampleFails++;
            ptTestCell->UpdateSampleFails(lPTRMemory->mSite);
            lFile->getMapFailingSiteDetails()[lPTRMemory->mSite] |= GEX_RUN_FAIL_TEST;

            // If failure and What-If limits enabled, make sure to force a failing bin!
            if(ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
                lFile->getMapFailingSiteDetails()[lPTRMemory->mSite] |= GEX_RUN_FAIL_WHATIFTEST;
        }

        // If failure, make sure we mark it.
        if(bTestIsFail)
            lFile->CountTrackFailingTestInfo(lPTRMemory->mSite,ptTestCell,GEX_PTEST,fResult);

        // Advanced report is Enabled, and test in filter limits.
        switch(mReportOptions->getAdvancedReport())
        {
        case GEX_ADV_DISABLED:
        case GEX_ADV_PAT_TRACEABILITY:
        default:
            // If Gex-Advanced edition, ALWAYS collect test results!
            if(bIsOutlier)
                return true;
            // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
            if(mReportOptions->bSpeedCollectSamples)
            {
                if (lFile->AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, lPTRMemory->mSite, true) == false)
                    return false;
            }
            break;

        case GEX_ADV_DATALOG:
            if (GexAbstractDatalog::existInstance())
            {
                if (GexAbstractDatalog::isDataloggable(ptTestCell, bTestIsFail, bIsOutlier))
                {
                    int nRunIndex = lFile->getPartProcessed().runIndexFromSite(lPTRMemory->mSite);
                    if (nRunIndex >= 0 && nRunIndex < lFile->getPartInfoList().count())
                        GexAbstractDatalog::pushParametricResult(
                                    lFile, ptTestCell, lFile->getPartInfoList().at(nRunIndex), fResult, bTestIsFail, bAlarmResult, lPTRMemory->mSite);
                }
            }
            else
                lFile->AdvCollectDatalog(ptTestCell,fResult,bTestIsFail,bAlarmResult,bIsOutlier, lPTRMemory->mSite,
                                         (int)lPTRMemory->mHead, lPTRMemory->mSite);

            // Check for outlier AFTER checking for datalog...in case datalog is on outlier!
            if(bIsOutlier)
                return true;
            // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
            if(mReportOptions->bSpeedCollectSamples)
            {
                if (lFile->AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, lPTRMemory->mSite, true) == false)
                    return false;
            }
            break;
        case GEX_ADV_HISTOGRAM:
        case GEX_ADV_TREND:
        case GEX_ADV_CANDLE_MEANRANGE:
        case GEX_ADV_GO_DIAGNOSTICS:	// Optimizer Diagnostic takes ALL Trend results!
            if(bIsOutlier)
                return true;
            if (lFile->AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, lPTRMemory->mSite,
                                     mReportOptions->bSpeedCollectSamples) == false)
            {
                return false;
            }
            break;
        case GEX_ADV_CORRELATION:
            if(bIsOutlier)
                return true;
            if (lFile->AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, lPTRMemory->mSite,
                                            mReportOptions->bSpeedCollectSamples) == false)
                return false;
            break;
        }

        // If this is an outlier (data to totally IGNORE)...don't take it into account!
        if(bIsOutlier)
            return true;

#if 0 // Herve Thomy : // OBSOLETE MEMBER Not used anymore,since we hold the runIndex in the CPartBinning class except with MASA plugins
        // Keeps tracking the test# result.
        ptTestCell->ldCurrentSample++;
#endif

        // Build Cpk
        ptTestCell->lfSamplesTotal += fResult;				// Sum of X
        ptTestCell->lfSamplesTotalSquare += (fResult*fResult);// Sum of X*X


        ptTestCell->mHistogramData.AddValue(fResult);
        if((fResult >= ptTestCell->GetCurrentLimitItem()->lfHistogramMin)
                && (fResult <= ptTestCell->GetCurrentLimitItem()->lfHistogramMax))
        {
            // Incluse result if it in into the viewing window.
            int iCell;
            if (ptTestCell->GetCurrentLimitItem()->lfHistogramMax == ptTestCell->GetCurrentLimitItem()->lfHistogramMin)
                iCell = 0;
            else
            {
                iCell = (int)( (TEST_HISTOSIZE*(fResult - ptTestCell->GetCurrentLimitItem()->lfHistogramMin))
                               / (ptTestCell->GetCurrentLimitItem()->lfHistogramMax-ptTestCell->GetCurrentLimitItem()->lfHistogramMin));
            }

            // Incluse result if it in into the viewing window.
            if( (iCell >= 0) && (iCell < TEST_HISTOSIZE) )
                ptTestCell->lHistogram[iCell]++;
            else if (iCell >= TEST_HISTOSIZE)
                ptTestCell->lHistogram[TEST_HISTOSIZE-1]++;   /// TO REVIEW : is it normal that iCell can exceed than TEST_HISTOSIZE
            else
            {
                /*GSLOG(SYSLOG_SEV_ERROR,
                      QString("Invalid value detected to build histogram for test %1 [%2]")
                      .arg(ptTestCell->lTestNumber)
                      .arg(ptTestCell->strTestName).toLatin1().constData());

                GSLOG(SYSLOG_SEV_DEBUG, QString("\tCell Found: %1").arg(iCell).toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG, QString("\tPTR result: %1").arg( fResult).toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Min: %1").arg(
                          ptTestCell->GetCurrentLimitItem()->lfHistogramMin).toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Max: %1").arg(
                          ptTestCell->lfHistogramMax).toLatin1().constData());*/
            }
        }
    }
ReadTestDefinitionV4:

    // Test is Parametric
    //GEX_ASSERT(ptTestCell->bTestType == 'P');

    if(lPass == 2)
        return true;

  /*  if(firstReadTest)
    {
        ptTestCell->GetCurrentLimitItem()->bLimitFlag      = firstReadTest->bLimitFlag;
        ptTestCell->llm_scal        = firstReadTest->llm_scal;
        ptTestCell->hlm_scal        = firstReadTest->hlm_scal;
        ptTestCell->GetCurrentLimitItem()->lfLowLimit      = firstReadTest->lfLowLimit;
        ptTestCell->GetCurrentLimitItem()->lfLowLimit      = firstReadTest->lfLowLimit;
        strcpy( ptTestCell->szTestUnits, firstReadTest->szTestUnits);
        ptTestCell->lfLowSpecLimit  = firstReadTest->lfLowSpecLimit;
        ptTestCell->lfHighSpecLimit = firstReadTest->lfHighSpecLimit;
    }
    else {*/
        // If we already have the limits set...ignore any NEW limits defined.
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
                != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
        {
            lfLowLimit = ptTestCell->GetCurrentLimitItem()->lfLowLimit;
            lfHighLimit = ptTestCell->GetCurrentLimitItem()->lfHighLimit;
        }
        else
        {
            lfLowLimit = -C_INFINITE;
            lfHighLimit = C_INFINITE;
        }

        // Are test limits strict??
        if(!bStrictLL)
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_LTLNOSTRICT;
        if(!bStrictHL)
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_HTLNOSTRICT;

        // If not in 'What-if' mode, save limits flag seen in STDF file
        if((lPTRMemory->optFlag & 0x10) == 0 && lFile->getUsedLimits() != CGexFileInGroup::SPEC_LIMIT_ONLY)
        {
            // LowLimit+low_scal flag is valid...
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
            if(lPTRMemory->optFlag & 0x40)	// No Low Limit
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
        }
        // If What-if has forced a LL
        else if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL) == 0)
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);

        // If not in 'What-if' mode, save limits flag seen in STDF file
        if((lPTRMemory->optFlag & 0x20) == 0 && lFile->getUsedLimits() != CGexFileInGroup::SPEC_LIMIT_ONLY)
        {
            // HighLimit+high_scal flag is valid...
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);
            if(lPTRMemory->optFlag & 0x80)	// No High Limit
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
        }
        // If What-if has forced a HL
        else if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL) == 0)
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);

        if(lPTRMemory->loLimScalRes == false)
            goto ExitBeforeLimitsV4;	// llm_scal

        if((lPTRMemory->optFlag & 0x50) == 0)	// llm-scal is valid
            ptTestCell->llm_scal = lPTRMemory->loLimScal;

         if(lPTRMemory->hiLimScalRes == false)
            goto ExitBeforeLimitsV4;	// hlm_scal

        if((lPTRMemory->optFlag & 0xA0) == 0)	// hlm-scal is valid
            ptTestCell->hlm_scal = lPTRMemory->hiLimScal;


        // Read low and High limits

        if(lPTRMemory->loLimitRes == false)
            goto ExitBeforeLimitsV4;	// low limit

        if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                && ((lPTRMemory->optFlag & 0x10) == 0)
                && (lFile->getUsedLimits() != CGexFileInGroup::SPEC_LIMIT_ONLY))
            lfLowLimit = M_CHECK_INFINITE((double)lPTRMemory->loLimit); // Low limit exists: keep value


        if(lPTRMemory->hiLimitRes == false)
        {
            // if not "Use spec limits only"
            if (lFile->getUsedLimits() != CGexFileInGroup::SPEC_LIMIT_ONLY)
                ptTestCell->GetCurrentLimitItem()->lfLowLimit = lPTRMemory->loLimit;
            goto ExitAfterLimitsV4;	// High limit doesn't exist
        }

        if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                && ((lPTRMemory->optFlag & 0x20) == 0)
                && (lFile->getUsedLimits() != CGexFileInGroup::SPEC_LIMIT_ONLY))
            lfHighLimit = M_CHECK_INFINITE((double)lPTRMemory->hiLimit);// High limit exists: keep value


        // Read test units, C_RESFMT, C_LLMFMT and C_HLMFMT
        if(lPTRMemory->testUnitRes == false)
            goto CalculateLimits;

        //std::strncpy(szString, ptrRecord.m_cnUNITS.toStdString().c_str(), 257);
        //lPTRMemory->testUnit[GEX_UNITS-1] = 0;
        // If unit specified (and using printable ascii characters), then use it.

        if(*lPTRMemory->testUnit.toStdString().c_str() >= ' ')
            strcpy(ptTestCell->szTestUnits, lPTRMemory->testUnit.toStdString().c_str());

        // C_RESFMT, C_LLMFMT, C_HLMFMT
        if( lPTRMemory->fmtRes == false)
            goto CalculateLimits;

        /// These inforamtions have to be in the new limits
        // LO_SPEC
        if(lPTRMemory->loSpecRes == false)
            goto CalculateLimits;

        if(lPTRMemory->optFlag & 4)	// LoSpec is Invalid
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
        else
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLSL;	// Valid Low Spec limit

        ptTestCell->lfLowSpecLimit = M_CHECK_INFINITE((double)lPTRMemory->loSpec);

        // If not "Use standard limits only"
        if(lFile->getUsedLimits() == CGexFileInGroup::SPEC_LIMIT_IF_ANY)
        {
            // Flag test limit validity depending on spec limits flag
            if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
            {
                ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;
                lfLowLimit = M_CHECK_INFINITE((double)lPTRMemory->loSpec);

                // No low specs limits as there are kept in std limits
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
            }
        }
        else if(lFile->getUsedLimits() == CGexFileInGroup::SPEC_LIMIT_ONLY)
        {
            lfLowLimit = M_CHECK_INFINITE((double)lPTRMemory->loSpec);

            // Flag test limit validity depending on spec limits flag
            if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
            {
                ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;

                // No low specs limits as there are kept in std limits
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
            }
            else
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
        }

        // HI_SPEC
        if(lPTRMemory->hiSpecRes == false)
            goto CalculateLimits;

        if(lPTRMemory->optFlag & 8)	// HiSpec is Invalid
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
        else
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHSL;	// Valid High Spec limit

        ptTestCell->lfHighSpecLimit = M_CHECK_INFINITE((double)lPTRMemory->hiSpec);

        // If not "Use standard limits only"
        if(lFile->getUsedLimits() == CGexFileInGroup::SPEC_LIMIT_IF_ANY)
        {
            // Flag test limit validity depending on spec limits flag
            if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
            {
                ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;
                lfHighLimit = M_CHECK_INFINITE((double)lPTRMemory->hiSpec);

                // No low specs limits as there are kept in std limits
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
            }
        }
        else if(lFile->getUsedLimits() == CGexFileInGroup::SPEC_LIMIT_ONLY)
        {
            lfHighLimit = M_CHECK_INFINITE((double)lPTRMemory->hiSpec);

            // Flag test limit validity depending on spec limits flag
            if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
            {
                ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;

                // No low specs limits as there are kept in std limits
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
            }
            else
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
        }

   // }
CalculateLimits:

    // Store the test limits for this site
    if (lPass == 1 && lFile->getParentGroup()->addTestSiteLimits(ptTestCell, iProcessSite, lPTRMemory->mSite, lfLowLimit,
                                         lfHighLimit, lPTRMemory->optFlag, lFile->getMirDatas().lStartT) == false)
        return false;

     //beforeLowLimit = lfLowLimit;
     //beforeHighLimit = lfHighLimit;


    // Retrieve the min and max tests limits for this site
    //lFile->getParentGroup()->getTestSiteLimits(ptTestCell, iProcessSite, siteNumber, lfLowLimit, lfHighLimit);

  //  if(beforeLowLimit != lfLowLimit || beforeHighLimit != lfHighLimit )
    //    beforeLowLimit = lfLowLimit;

    if (bIgnoreSite == false || (ptTestCell->GetCurrentLimitItem()->lfHighLimit == C_INFINITE && ptTestCell->GetCurrentLimitItem()->lfLowLimit == -C_INFINITE))
    {
        // If High limit not forced by What-If...
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL)
            ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighLimit;

        // If Low limit not forced by What-If...
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL)
            ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;
    }

    if(lFile->getDataSetConfig().testToUpdateList().isActivated())
    {
        GexTestToUpdate *poUpdate = lFile->getDataSetConfig().testToUpdateList().getTestToUpdate(ptTestCell->lTestNumber);
        if(poUpdate)
        {
            ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag = CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;
            if(!poUpdate->highLimit().isEmpty())
            {
                ptTestCell->GetCurrentLimitItem()->lfHighLimit = poUpdate->highLimit().toDouble();
                if(poUpdate->source() == "whatif")
                    ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag	&= ~CTEST_LIMITFLG_NOHTL;
            }
            if(!poUpdate->lowLimit().isEmpty())
            {
                ptTestCell->GetCurrentLimitItem()->lfLowLimit = poUpdate->lowLimit().toDouble();
                if(poUpdate->source() == "whatif")
                    ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag	&= ~CTEST_LIMITFLG_NOLTL;
            }
        }
    }

    // All info read, including test limits
ExitAfterLimitsV4:

    // Avoid failing if LL=HL
    if(ptTestCell->GetCurrentLimitItem()->lfLowLimit == ptTestCell->GetCurrentLimitItem()->lfHighLimit)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= (CTEST_LIMITFLG_LTLNOSTRICT | CTEST_LIMITFLG_HTLNOSTRICT);

    // If data result to be ignoreed...
    if(bIgnoreSite)
        return true;
    // We have limits...so check if this is an outlier
    if(lFile->PartIsOutlier(ptTestCell,fResult) == true)
        return true;
    // Updates Min/Max values
    if(bValideResult == true)
        lFile->UpdateMinMaxValues(ptTestCell,fResult);
    return true;

    // All info read...but not until limits (may have been read in previous PTR)
ExitBeforeLimitsV4:
    // If data result to be ignoreed...
    if(bIgnoreSite)
        return true;

    if (lFile->getParentGroup()->getTestSiteLimits(ptTestCell, iProcessSite, lPTRMemory->mSite, lfLowLimit, lfHighLimit))
    {
        // If High limit not forced by What-If...
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL)
            ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighLimit;

        // If Low limit not forced by What-If...
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL)
            ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;

        // Avoid failing if LL=HL
        if(ptTestCell->GetCurrentLimitItem()->lfLowLimit == ptTestCell->GetCurrentLimitItem()->lfHighLimit)
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= (CTEST_LIMITFLG_LTLNOSTRICT | CTEST_LIMITFLG_HTLNOSTRICT);
    }
    else
    {
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
        ptTestCell->GetCurrentLimitItem()->lfHighLimit	= C_INFINITE;
        ptTestCell->GetCurrentLimitItem()->lfLowLimit	= -C_INFINITE;
    }

    // Check if test is an outlier
    if(lFile->PartIsOutlier(ptTestCell,fResult) == true)
        return true;
    // Updates Min/Max values..either because not an outlier, or no limits to check it!
    if(bValideResult == true)
       lFile->UpdateMinMaxValues(ptTestCell,fResult);

    return true;  // ?
}

//!-------------------------------!//
//!         insertFileInGroup     !//
//!-------------------------------!//
void StdfDataFile::insertFileInGroup(int site, CGexFileInGroup* file)
{

    if(mFileInGroupsBySite.contains(site) == false)
        mFileInGroupsBySite.insert(site, QList<CGexFileInGroup* >());

    mFileInGroupsBySite[site].push_back(file);
     file->setParentStdfDataFile(this);
}

//!-------------------------------!//
//!         isMatchingWafer       !//
//!-------------------------------!//
bool StdfDataFile::isMatchingWafer(const QString& strFilter, const QString& strWaferID)
{
    QRegExp rx(strFilter, Qt::CaseInsensitive);  // NOT case sensitive

    // If wildcar used, set its support
    if (strFilter.indexOf("*") >= 0 || strFilter.indexOf("?") >= 0)
    {
        rx.setPatternSyntax(QRegExp::Wildcard);
    }
    else
    {
        rx.setPatternSyntax(QRegExp::RegExp);
    }

    // Check if WaferID matches the filter
    return rx.exactMatch(strWaferID);
}

//!-------------------------------!//
//!         loadFile              !//
//!-------------------------------!//
bool StdfDataFile::loadFile()
{
    int iProcessStatus = mStdfFile.Open(mFileName.toLatin1().constData(), STDF_READ);
    if (iProcessStatus != GS::StdLib::Stdf::NoError)
    {
       mStdfFile.Close();
       return false;
    }

    return true;
}

//!-------------------------------!//
//!         processPCR            !//
//!-------------------------------!//
bool StdfDataFile::processPCR(int lPass)
{
    if(lPass == 2)
        return true;

    mPCRRecord.reset();
    mPCRRecord.readPCR(mStdfFile);

   /* if( mFileInGroupsBySite.contains(-1) == false &&   // not in merge file mode
       (mFileInGroupsBySite.contains(mPCRRecord.mSite) == false || // no fileInGroup with the same site
        mPCRRecord.mHead == 255   )) // or head equl 255
        return true; // ignore this record*/

    loopOverFileInGroup(mPCRRecord.mSite, T_PCR, lPass);
    return true;
}

//!---------------------------------------!//
//!         initFileInGroupPCR            !//
//!---------------------------------------!//
void StdfDataFile::initFileInGroupPCR(CGexFileInGroup *lFile)
{
    CPcr& lPcr = lFile->getPcrDatas();
    if(lPcr.bFirstPcrRecord == true)
    {
        // First PCR record...reset all PCR related variables. WRR may have loaded them (in case no PCR present in file!).
        lPcr.bFirstPcrRecord    = false;
        lPcr.lPartCount         = 0;
        lPcr.lRetestCount       = -1;
        lPcr.lAbortCount        = -1;
        lPcr.lGoodCount         = -1;
        lPcr.lFuncCount         = -1;
    }

    if((lFile->iProcessSite >=0) && ((mPCRRecord.mSite != lFile->iProcessSite) || (mPCRRecord.mHead == 255)))
        return;	// Site filter not matching.

    // If PCR of merged sites found, and filter=all sites, ignore other PCRs!
    // Sometimes PCRs of specific sites appear AFTER PCR of merged sites!
    if(lPcr.bMergedPcrDone == true)
        return;	// Ignore all other PCRs for this file since we have read the 'Merged' record!

    if(mPCRRecord.mPartCountRes == false)
        return;// PART_CNT

    if(mPCRRecord.mHead == 255)
    {
        // Tells we will ignore any future PCRs...as we only want the ALL sites!
        if(lFile->iProcessSite <0)
            lPcr.bMergedPcrDone = true;
        lPcr.lPartCount = mPCRRecord.mPartCount; // This is a merge of all sites
    }
    else
        lPcr.lPartCount+= mPCRRecord.mPartCount; // Cumulate all sites.

    if(mPCRRecord.mRetestPartCountRes  == false)
       return;// RTST_CNT

    if (mPCRRecord.mRetestPartCount >= 0 && mPCRRecord.mRetestPartCount != GS_MAX_UINT32)
    {
        if(mPCRRecord.mHead == 255)
            lPcr.lRetestCount = mPCRRecord.mRetestPartCount; // This is a merge of all sites
        else
        {
            if (lPcr.lRetestCount == -1)
                lPcr.lRetestCount = mPCRRecord.mRetestPartCount;
            else
                lPcr.lRetestCount+= mPCRRecord.mRetestPartCount; // Cumulate all sites.
        }
    }

    if(mPCRRecord.mAbortCountRes  == false)
        return;// ABRT_CNT

    if (mPCRRecord.mAbortCount >= 0 && mPCRRecord.mAbortCount != GS_MAX_UINT32)
    {
        if(mPCRRecord.mHead == 255)
            lPcr.lAbortCount = mPCRRecord.mAbortCount; // This is a merge of all sites
        else
        {
            if (lPcr.lAbortCount == -1)
                lPcr.lAbortCount = mPCRRecord.mAbortCount;
            else
                lPcr.lAbortCount+= mPCRRecord.mAbortCount; // Cumulate all sites.
        }
    }

    if(mPCRRecord.mGoodCountRes == false)
        return;// GOOD_CNT

    if (mPCRRecord.mGoodCount  >= 0 && mPCRRecord.mGoodCount  != GS_MAX_UINT32)
    {
        if(mPCRRecord.mHead == 255)
            lPcr.lGoodCount = mPCRRecord.mGoodCount; // This is a merge of all sites
        else
        {
            if (lPcr.lGoodCount == -1)
                lPcr.lGoodCount = mPCRRecord.mGoodCount;
            else
                lPcr.lGoodCount+= mPCRRecord.mGoodCount; // Cumulate all sites.
        }
    }

    if(mPCRRecord.mFunctCountRes == false)
        return;// FUNC_CNT

    if (mPCRRecord.mFunctCount >= 0 && mPCRRecord.mFunctCount != GS_MAX_UINT32)
    {
        if(mPCRRecord.mHead == 255)
            lPcr.lFuncCount = mPCRRecord.mFunctCount; // This is a merge of all sites
        else
        {
            if (lPcr.lFuncCount == -1)
                lPcr.lFuncCount = mPCRRecord.mFunctCount;
            else
                lPcr.lFuncCount+= mPCRRecord.mFunctCount; // Cumulate all sites.
        }
    }
}

//!-------------------------------!//
//!         processPIR            !//
//!-------------------------------!//
void StdfDataFile::processPIR()
{
    BYTE        lSite = 0;
    BYTE        lHead = 0;
    CPartInfo	lPartInfo;

    mStdfFile.ReadByte(&lHead) ;
    mStdfFile.ReadByte(&lSite)  ;	// site number

    lPartInfo.m_site    = lSite;
    lPartInfo.bHead     = lHead;

    // get the deciphering mode
    if(mStdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        lPartInfo.m_site = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(lHead, lSite);
        lPartInfo.bHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(lHead);
    }
    else
    {
        lPartInfo.m_site    = lSite;
        lPartInfo.bHead     = lHead;
    }

    // This code will manage the scpecific case:
    // split by site and the first PIR is only to incapsulate PTR definition and no valid data
    // * Condition 1: means merge sites (no split)
    // * Condition 2: means no site in all groups for this physical file
    // This PIR has to be ignored
    if (mFileInGroupsBySite.contains(-1) == false
        && mFileInGroupsBySite.contains(lPartInfo.m_site) == false)
        return;

    loopOverFileInGroup(lPartInfo.m_site, T_PIR, -1, &lPartInfo);
}

//!-------------------------------!//
//!         processATR            !//
//!-------------------------------!//
void StdfDataFile::processATR(int lPass)
{
    if(lPass == 2)
        return;

    // ATR: STDF V4 only
    long    lData;
    char	szString[257];	// A STDF string is 256 bytes long max!

    mStdfFile.ReadDword((long *)&lData);			// MOD_TIM
    if(mStdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        return;	// CMD_LINE

    //  Flag to tell 'Reject this file'...cleared below if file is fine!
    int iStdfAtrTesterBrand = -2;

    // ATR may include the tester signature. If this is the case, parse it!
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:	// OEM-Examinator for LTXC
        {
            // Parse ATR string and check if it includes the Sapphire STDF signature.
            QString strString = szString;
            // E.G: Credence Systems Sapphire D Series - Release 1
            if(strString.startsWith("Credence Systems Sapphire D Series - Release") == false)
                break;

            //  Flag to tell 'Reject this file'...cleared below if file is fine!
           // int iStdfAtrTesterBrand = -2;

            // Clean the Revision string in case it starts with string "Rev" or Revision"
            QString strExecVersion = strString.section(' ',-1);	// Get Exec version info
            strExecVersion = strExecVersion.trimmed();
            if(strExecVersion.isEmpty())
                break;	// Missing OS revision...error

            // Check if this release accepts this version of Credence Sapphire OS...
            // Check that OS version is in format X.Y
            int iMajorRelease;
            int iStatus = sscanf(strExecVersion.toLatin1().constData(),"%d",&iMajorRelease);
            if(iStatus != 1)
                break;	// OS version string not valid

            // Check OS version is more recent than the one accepted by Examinator: user needs to upgrade Examinator!
            if(iMajorRelease > GEX_OEM_SAPPHIRE_OSVERSION)
                break;

            iStdfAtrTesterBrand = GS::LPPlugin::LicenseProvider::eLtxcOEM;

            break;
        }

        default:
            break;
    }

    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:	// OEM-Examinator for LTXC
        {

            FileInGroupContainerIter lIter(mFileInGroupsBySite.begin()), lIterEnd(mFileInGroupsBySite.end());
            for(;lIter != lIterEnd;++lIter)
            {
                QList<CGexFileInGroup*> lList = lIter.value();
                QList<CGexFileInGroup*>::iterator lIterList(lList.begin()), lIterListEnd(lList.end());
                for(; lIterList != lIterListEnd; ++lIterList)
                {
                    (*lIterList)->iStdfAtrTesterBrand = iStdfAtrTesterBrand;
                }
            }
            break;
        }
    default:
        break;
    }
}

//!-------------------------------!//
//!         readStringToField     !//
//!-------------------------------!//
bool	StdfDataFile::readStringToField(char *szField)
{
  *szField=0;

  if(mStdfFile.ReadString(szField)  != GS::StdLib::Stdf::NoError)
  {
    *szField=0;
    return false;
  }
  // Security: ensures we do not overflow destination buffer !
  szField[MIR_STRING_SIZE-1] = 0;
  return true;
}

//!-------------------------------!//
//!         readMIR               !//
//!-------------------------------!//
void StdfDataFile::readMIR(int pass)
{

    if(pass == 2)
        return;

    long lData;
    mStdfFile.ReadDword(&lData);                        // Setup_T
    mMirData.lSetupT = (time_t)lData;
    mStdfFile.ReadDword(&lData);                        // Start_T
    mMirData.lStartT = (time_t)lData;
    mMirData.lEndT = mMirData.lStartT;                  // Just in case the 'end time' info is missing...
    mStdfFile.ReadByte(&mMirData.bStation);             // stat #
    mStdfFile.ReadByte(&mMirData.bModeCode);			// mode_code
    mStdfFile.ReadByte(&mMirData.bRtstCode);			// rtst_code
    mStdfFile.ReadByte(&mMirData.bProdCode);			// prot_cod #
    mStdfFile.ReadWord(&mMirData.iBurnTime);			// burn_time
    mStdfFile.ReadByte(&mMirData.cModeCode);			// cmode_code

    if(readStringToField(mMirData.szLot) == false)
        return;
    if(readStringToField(mMirData.szPartType) == false)
        return;
    if(readStringToField(mMirData.szNodeName) == false)
        return;
    if(readStringToField(mMirData.szTesterType) == false)
        return;
    if(readStringToField(mMirData.szJobName) == false)
        return;
    if(readStringToField(mMirData.szJobRev) == false)
        return;
    if(readStringToField(mMirData.szSubLot) == false)
        return;
    if(readStringToField(mMirData.szOperator) == false)
        return;
    if(readStringToField(mMirData.szExecType) == false)
        return;
    if(readStringToField(mMirData.szExecVer) == false)
        return;
    if(readStringToField(mMirData.szTestCode) == false)
        return;
    if(readStringToField(mMirData.szTestTemperature) == false)
        return;
    if(readStringToField(mMirData.szUserText) == false)
        return;
    if(readStringToField(mMirData.szAuxFile) == false)
        return;
    if(readStringToField(mMirData.szPkgType) == false)
        return;
    if(readStringToField(mMirData.szFamilyID) == false)
        return;
    if(readStringToField(mMirData.szDateCode) == false)
        return;
    if(readStringToField(mMirData.szFacilityID) == false)
        return;
    if(readStringToField(mMirData.szFloorID) == false)
        return;
    if(readStringToField(mMirData.szProcID) == false)
        return;
    if(readStringToField(mMirData.szperFrq) == false)
        return;
    if(readStringToField(mMirData.szSpecName) == false)
        return;
    if(readStringToField(mMirData.szSpecVersion) == false)
        return;
    if(readStringToField(mMirData.szFlowID) == false)
        return;
    if(readStringToField(mMirData.szSetupID) == false)
        return;
    if(readStringToField(mMirData.szDesignRev) == false)
        return;
    if(readStringToField(mMirData.szEngID) == false)
        return;
    if(readStringToField(mMirData.szROM_Code) == false)
        return;
    if(readStringToField(mMirData.szSerialNumber) == false)
        return;
    if(readStringToField(mMirData.szSuprName) == false)
        return;
}

//!-------------------------------!//
//!         readMRR               !//
//!-------------------------------!//
void StdfDataFile::readMRR(int lPass)
{
    mMRRCount++;
    // MRR
    if(lPass == 2)
        return;

    // Flags that this file is complete (as the MRR is ALWAYS the last record in a STDF file).
    mMirData.bMrrRecord = true;

    long data;
    mStdfFile.ReadDword(&data);	// End_T
    mMirData.lEndT = (time_t)data;
}

//!-------------------------------!//
//!         processPRR            !//
//!-------------------------------!//
void StdfDataFile::processPRR(int lPass)
{
    mPRRRecord.reset();
    mPRRRecord.readPRR(mStdfFile);

    // This code will manage the scpecific case:
    // split by site and the first PRR is only to incapsulate PTR definition and no valid data
    // * Condition 1: means merge sites (no split)
    // * Condition 2: means no site in all groups for this physical file

    // This PRR has to be ignored
    if (mFileInGroupsBySite.contains(-1) == false
        && mFileInGroupsBySite.contains(mPRRRecord.mSite) == false)
        return;

    loopOverFileInGroup(mPRRRecord.mSite, T_PRR, lPass);
}

//!-------------------------------!//
//!         processPTR            !//
//!-------------------------------!//
bool StdfDataFile::processPTR(int lPass, bool itsATestDefinition)
{
    mPTRRecord.reset();
    mPTRRecord.readPTR(mStdfFile);

    int lSiteNumber = mPTRRecord.mSite;

    //-- for to all sites
    if(mPTRRecord.mContainTestDefinition || itsATestDefinition)
        lSiteNumber = -1;

     loopOverFileInGroup(lSiteNumber, T_PTR, lPass);

    return true;
}

//!-------------------------------!//
//!         processBinning        !//
//!-------------------------------!//
void StdfDataFile::processBinning(int lPass, bool isHBR)
{

    mBinRecord.reset();
    mBinRecord.readBin(mStdfFile);

    FileInGroupContainerIter lIter(mFileInGroupsBySite.begin()), lIterEnd(mFileInGroupsBySite.end());
    for(;lIter != lIterEnd;++lIter)
    {
        QList<CGexFileInGroup*> lList = lIter.value();
        QList<CGexFileInGroup*>::iterator lIterList(lList.begin()), lIterListEnd(lList.end());
        for(; lIterList != lIterListEnd; ++lIterList)
        {
            if((*lIterList)->bIgnoreThisSubLot)
                continue;

            if(isHBR)
            {
                initFileInGroupBin((*lIterList),
                                   lPass,
                                   &(*lIterList)->m_ParentGroup->cMergedData.ptMergedHardBinList,
                                   &(*lIterList)->iHardBinRecords,
                                   &(*lIterList)->bMergeHardBin);
            }
            else
            {
                initFileInGroupBin((*lIterList),
                                   lPass,
                                   &(*lIterList)->m_ParentGroup->cMergedData.ptMergedSoftBinList,
                                   &(*lIterList)->iSoftBinRecords,
                                   &(*lIterList)->bMergeSoftBin);
            }
        }
    }
}


//!-------------------------------!//
//!         processWIR            !//
//!-------------------------------!//
void StdfDataFile::processWIR(int lPass)
{
    ++mCurrentIndexWaferMap;
    mWIRRecord.readWIR(mStdfFile);

    if(lPass == 1)
    {
        QList<CGexFileInGroup*> lList = mFileInGroupsBySite.begin().value();
        CGexFileInGroup* lFile = (*lList.begin());

        // a file may contain several wafermpa description (between WIR/WRR records).
        // for each of them, a pFile is created and identify with an unique mIndexWafermap.
        if(mCurrentIndexWaferMap > 0)
        {
            mMultiWaferMap = true;
            int iFileID = gexReport->addFile(lFile->lGroupID,
                                             lFile->strFileName,
                                             lFile->iProcessSite,
                                             lFile->iProcessBins,
                                             lFile->strRangeList.toLatin1().data(),
                                             lFile->strMapTests,
                                             lFile->strWaferToExtract,
                                             lFile->m_lfTemperature,
                                             lFile->strDatasetName,
                                             lFile->mSampleGroup);

            // Retrive the created FileInGroup and Set WaferID filter for this file.
            CGexGroupOfFiles *pGroup = lFile->m_ParentGroup;

            if (!pGroup->pFilesList.isEmpty() && iFileID < pGroup->pFilesList.size())
            {
                CGexFileInGroup *pFile = pGroup->pFilesList.at(iFileID);
                pFile->Init(lFile->pReportOptions, &pGroup->cMergedData, lFile->hAdvancedReport, lPass);
                strcpy(pFile->getWaferMapData().szWaferIDFilter, mWIRRecord.mWaferID);
                pFile->getWaferMapData().bFirstSubLotInFile = false;
                pFile->mIndexWafermap = mCurrentIndexWaferMap;
            }
        }
        else
        {
            lFile->mIndexWafermap = mCurrentIndexWaferMap;
        }
    }

    // only in the case of mutli wafermap inside the same file, we need to update the flag
    // bIgnoreThisSubLot in order to associate the record between the WIR/WRR record to the correct
    // GexFileInGroup
    if(mMultiWaferMap)
    {
        // Loop over all the pFile in order to active only the one corresponding to the wafermap section read
        // (WIR, PRR, PTR, WRR).
        // This flag enable to read PTR record only for the dedicated wafermap
        // that is to say with the related pFile having a flag bIgnoreThisSubLot set to false
        FileInGroupContainerIter lIter(mFileInGroupsBySite.begin()), lIterEnd(mFileInGroupsBySite.end());
        for(;lIter != lIterEnd;++lIter)
        {
            QList<CGexFileInGroup*>::iterator lIterList(lIter.value().begin()), lIterListEnd(lIter.value().end());
            for(; lIterList != lIterListEnd; ++lIterList)
            {
                if((*lIterList)->mIndexWafermap != mCurrentIndexWaferMap)
                {
                    (*lIterList)->bIgnoreThisSubLot = true;
                }
            }
       }
    }

    loopOverFileInGroup(-1, T_WIR, lPass);
}


//!---------------------------------------!//
//!         initFileInGroupWIR            !//
//!---------------------------------------!//
void StdfDataFile::initFileInGroupWIR(CGexFileInGroup *lFile)
{

    // LTX-STDF bug fix: if empty WaferID, assume it is the same as previous one found!
    if(*(mWIRRecord.mWaferID) == 0)
    {
      strcpy(mWIRRecord.mWaferID, lFile->getWaferMapData().szWaferIDFilter);
    }

    // Check if we read this wafer or not...
    if(*(lFile->getWaferMapData().szWaferIDFilter) == 0)
    {
      // No filter defined...so this is the first WaferID we find in the file...
      //bIgnoreThisSubLot = false;	// Set Filter to read all data for this wafer.
      // Mark that this is the first Wafer/Sub-lot processed in this file
      lFile->getWaferMapData().bFirstSubLotInFile=true;
    }
    else
    {
      // A filter is defined...so probably we must not read this Wafer data...
      if(qstricmp(mWIRRecord.mWaferID, lFile->getWaferMapData().szWaferIDFilter))
      {
        mIgnoreNextWaferRecord = true;	// Mismatch with filter: must ignore this wafer and its following related records
      }
      else
      {
        mIgnoreNextWaferRecord = false;	// Filter match: read next records (related to this waferID)
      }
    }

    if((mWaferToExtract.isEmpty() == false) && (isMatchingWafer(mWaferToExtract, mWIRRecord.mWaferID) == false))
      mIgnoreNextWaferRecord = true;	// Query requested to only read one specific WaferID...do so!

    // If we read this wafer, save the TimeStamp & WaferID
    if(mIgnoreNextWaferRecord == false)
    {
      lFile->getWaferMapData().lWaferStartTime = mWIRRecord.mStart_T;			// Save TimeStamp
      strcpy(lFile->getWaferMapData().szWaferID, mWIRRecord.mWaferID);		// Save WaferID
      strcpy(lFile->getWaferMapData().szWaferIDFilter, mWIRRecord.mWaferID);	// Wafer Filter = This WaferID
      // Overwrite MIR Time/date data with Wafer Time info (other wise all wafers in file would have same Time info!).
      getMirDatas().lStartT = mWIRRecord.mStart_T;
    }

    // Flag this Data file if it has a WIR record
    lFile->getWaferMapData().bWirExists = true;
    lFile->m_iWIRCount++;
}


//!-------------------------------!//
//!         processWRR            !//
//!-------------------------------!//
void StdfDataFile::processWRR(int lPass)
{
    mWRRRecord.reset();
    mWRRRecord.readWRR(mStdfFile);

    loopOverFileInGroup(mWRRRecord.mSite, T_WRR, lPass);

    // Records outside WIR/WRR record, may have to be apply to all GexFileInGroup.
    //  For that we need to reset the flag bIgnoreThisSubLot to false
    FileInGroupContainerIter lIter(mFileInGroupsBySite.begin()), lIterEnd(mFileInGroupsBySite.end());
    for(;lIter != lIterEnd;++lIter)
    {
        QList<CGexFileInGroup*> lList = lIter.value();
        QList<CGexFileInGroup*>::iterator lIterList(lList.begin()), lIterListEnd(lList.end());
        for(; lIterList != lIterListEnd; ++lIterList)
        {
            (*lIterList)->bIgnoreThisSubLot = false;
        }
    }
}


//!-------------------------------!//
//!         processWCR            !//
//!-------------------------------!//
void StdfDataFile::processWCR(int lPass)
{
    if(lPass == 2)
        return;

    mWCRRecord.reset();
    mWCRRecord.readWCR(mStdfFile);

    loopOverFileInGroup(-1, T_WCR);
}

//!-------------------------------------!//
//!       initFileInGroupWCR            !//
//!-------------------------------------!//
void StdfDataFile::initFileInGroupWCR(CGexFileInGroup *lFile)
{
    if(mWCRRecord.mWaferSizeRes == false)
        return;	// WaferSize.
    lFile->getWaferMapData().SetDiameter(mWCRRecord.mWaferSize);

    if(mWCRRecord.mDieHeightRes == false)
        return;	// DIE Heigth in WF_UNITS.
    lFile->getWaferMapData().SetDieHeight(mWCRRecord.mDieHeight);

    if(mWCRRecord.mDieWidthRes == false)
        return;	// DIE Width in WF_UNITS.
    lFile->getWaferMapData().SetDieWidth(mWCRRecord.mDieWidth);

    if(mWCRRecord.mUnitRes == false)
        return;	// WF_UNITS
    lFile->getWaferMapData().bWaferUnits = mWCRRecord.mUnit;

    if(mWCRRecord.mFlatRes == false)
        return; // WF_FLAT
    lFile->getWaferMapData().cWaferFlat_Stdf = mWCRRecord.mFlat;

    if(mWCRRecord.mCenterXRes == false)
        return; // CENTER_X

    GS::Gex::WaferCoordinate lCenterDie;
    lCenterDie.SetX(mWCRRecord.mCenterX);
    lFile->getWaferMapData().SetCenterDie(lCenterDie);

    if(mWCRRecord.mCenterYRes == false)
        return; // CENTER_Y
    lCenterDie.SetY(mWCRRecord.mCenterX);
    lFile->getWaferMapData().SetCenterDie(lCenterDie);

    if(mWCRRecord.mPosXRes == false)
        return; // POS_X
    lFile->getWaferMapData().cPos_X = mWCRRecord.mPosX;

    if(mWCRRecord.mPosYRes == false)
        return; // POS_Y
    lFile->getWaferMapData().cPos_Y = mWCRRecord.mPosY;
}

//!-------------------------------!//
//!         initFileInGroupWRR    !//
//!-------------------------------!//
void StdfDataFile::initFileInGroupWRR(CGexFileInGroup *lFile, int lPass)
{
    // record below this one won't be apply to this pFile
    //lFile->bIgnoreThisSubLot = true;
    if(lPass == 2)
    {
        return;
    }

    lFile->getWaferMapData().lWaferEndTime = mWRRRecord.mFinish_T;

    // Overwrite MIR Time/date data with Wafer Time info (other wise all wafers in file would have same Time info!).
    getMirDatas().lEndT = lFile->getWaferMapData().lWaferEndTime;

    // PART_CNT
    if(mWRRRecord.mPartCountRes == false)
        return;

    if((mWRRRecord.mPartCount >= 0) && (mWRRRecord.mPartCount != GS_MAX_UINT32))
    {
        if((lFile->getPcrDatas().lPartCount <= 0) || (lFile->getPcrDatas().lPartCount == GS_MAX_UINT32))
            lFile->getPcrDatas().lPartCount = mWRRRecord.mPartCount;
        else if (lFile->getPcrDatas().bFirstPcrRecord)
        {
            // We have already seen another WRR in the file...merge data
            lFile->getPcrDatas().lPartCount += mWRRRecord.mPartCount;
        }
    }

    // RTST_CNT
    if(mWRRRecord.mRstCountRes == false)
        return;

    if((mWRRRecord.mRstCount >= 0) && (mWRRRecord.mRstCount != GS_MAX_UINT32))
    {
        if((lFile->getPcrDatas().lRetestCount <= 0) || (lFile->getPcrDatas().lRetestCount == GS_MAX_UINT32))
            lFile->getPcrDatas().lRetestCount = mWRRRecord.mRstCount;
        else if (lFile->getPcrDatas().bFirstPcrRecord)
        {
            // We have already seen another WRR in the file...merge data
            lFile->getPcrDatas().lRetestCount += mWRRRecord.mRstCount;
        }
    }

    // ABRT_CNT
    if(mWRRRecord.mAbrCountRes == false)
        return;

    if((mWRRRecord.mAbrCount >= 0) && (mWRRRecord.mAbrCount != GS_MAX_UINT32))
    {
        if((lFile->getPcrDatas().lAbortCount <= 0) || (lFile->getPcrDatas().lAbortCount == GS_MAX_UINT32))
            lFile->getPcrDatas().lAbortCount = mWRRRecord.mAbrCount;
        else if (lFile->getPcrDatas().bFirstPcrRecord)
        {
            // We have already seen another WRR in the file...merge data
            lFile->getPcrDatas().lAbortCount += mWRRRecord.mAbrCount;
        }
    }

    // GOOD_CNT
    if(mWRRRecord.mGoodCountRes == false)
        return;

    if((mWRRRecord.mGoodCount >= 0) && (mWRRRecord.mGoodCount != GS_MAX_UINT32))
    {
        if((lFile->getPcrDatas().lGoodCount <= 0) || (lFile->getPcrDatas().lGoodCount == GS_MAX_UINT32))
            lFile->getPcrDatas().lGoodCount = mWRRRecord.mGoodCount;
        else if (lFile->getPcrDatas().bFirstPcrRecord)
        {
            // We have already seen another WRR in the file...merge data
            lFile->getPcrDatas().lGoodCount += mWRRRecord.mGoodCount;
        }
    }

    // FUNC_CNT
    if(mWRRRecord.mFuncCountRes == false)
        return;

    if((mWRRRecord.mFuncCount >= 0) && (mWRRRecord.mFuncCount != GS_MAX_UINT32))
    {
        if((lFile->getPcrDatas().lFuncCount <= 0) || (lFile->getPcrDatas().lFuncCount == GS_MAX_UINT32))
            lFile->getPcrDatas().lFuncCount = mWRRRecord.mFuncCount;
        else if (lFile->getPcrDatas().bFirstPcrRecord)
        {
            // We have already seen another WRR in the file...merge data
            lFile->getPcrDatas().lFuncCount += mWRRRecord.mFuncCount;
        }
    }


    if(mWRRRecord.mWaferID[0] == 0)
        sprintf(lFile->getWaferMapData().szWaferID, "%s", "n/a");
}

//!-------------------------------!//
//!         initFileInGroupBin    !//
//!-------------------------------!//
void StdfDataFile::initFileInGroupBin(CGexFileInGroup *pFile,
                                      int lPass,
                                      CBinning **ptBinList,
                                      int *piRecords,
                                      bool *pbMerge)
{
    //-- take the test, hardbinning and softbinning from the parent group
    pFile->initPtrFromParentGroup();

    switch(mRecordHeader.iStdfVersion)
    {

    case GEX_STDFV4:
        bool	bBreak=false;
        bool	bCreateEntryOnly=false;

        // If filter is specific site, only read relevant SBR/HBR
        if((pFile->iProcessSite >=0) && ((lPass == 2) || (mBinRecord.mSite != pFile->iProcessSite) ||
                                         (mBinRecord.mHead == 255)))
            bCreateEntryOnly = true;	// Site filter not matching.

        // If looking for merged binning summary
        if(pFile->iProcessSite < 0)
        {
            if((lPass == 1) && (mBinRecord.mHead != 255))
                bBreak = true;	// In pass1, only process HBR/SBR giving merged results
            else
                if(lPass == 2)
                {
                    if((*piRecords) && (*pbMerge == false))
                        break;	// In pass2, only take records if we have to merge ourselves...
                    else
                        *pbMerge = true;	// Trigger to merge ourselves!
                }
        }

        long lData = mBinRecord.mBinCount;
        if((unsigned)lData & 0xC0000000)
            break;	// Too high number to be valid...also fixes LTX-Fusion bug!

        //-- Check if Have to Collect Binning Trend data
        if(	(pFile->m_eBinComputation == CGexFileInGroup::SUMMARY)
                && (mBinRecord.mBinNum != 65535 || (pFile->m_eStdfCompliancy == CGexFileInGroup::FLEXIBLE)))
            pFile->AdvCollectBinningTrend(ptBinList, mBinRecord.mBinNum, lData);

        //-- Check if should exit now
        if(bBreak)
            break;

        //-- If only to use PRR to compute Binning., then only create empty Bin celle (so we save its name)
        if(	(pFile->m_eBinComputation == CGexFileInGroup::SAMPLES)    ||
                (pFile->m_eBinComputation == CGexFileInGroup::WAFER_MAP)  ||
                bCreateEntryOnly
                )
            lData = -1;

        if (mBinRecord.mBinNum != 65535 || (pFile->m_eStdfCompliancy == CGexFileInGroup::FLEXIBLE)){
            int iSoftBinValue = mBinRecord.mBinNum ,iHardBinValue = mBinRecord.mBinNum;

            //--bModeUsed to specify if the fct insert a soft bin or a hard bin
            bool bMode = false;

            if(mRecordHeader.iRecordSubType == 40)//HBIN
            {
                iSoftBinValue = pFile->m_oSBinHBinMap.key(mBinRecord.mBinNum, mBinRecord.mBinNum);
                QList<int> oFoundSoftBin = pFile->m_oSBinHBinMap.keys(mBinRecord.mBinNum);
                if(oFoundSoftBin.count() == 1)
                    iSoftBinValue  = oFoundSoftBin.first();
                else {
                    foreach (int iCSoft, oFoundSoftBin) {
                        if(pFile->iProcessBins == GEX_PROCESSPART_SBINLIST &&
                                pFile->pGexRangeList->Contains(iCSoft))
                        {
                            iSoftBinValue = iCSoft;
                            break;
                        }
                        if(pFile->iProcessBins == GEX_PROCESSPART_EXSBINLIST &&
                                !pFile->pGexRangeList->Contains(iCSoft))
                        {
                            iSoftBinValue = iCSoft;
                            break;
                        }
                    }
                }
                bMode = true;
            }else{//SBIN
                iHardBinValue = pFile->m_oSBinHBinMap.value(mBinRecord.mBinNum, mBinRecord.mBinNum);
                bMode = false;
            }

            // Add Bin inf to list
            pFile->AddBinCell(ptBinList, -1, 32768, 32768,
                              iSoftBinValue, iHardBinValue,
                              mBinRecord.mPassFailInfo, lData, true, true, mBinRecord.mBinLabel, bMode);
        }

        //-- If only to use PRR to compute Binning, exit now.
        if(	(pFile->m_eBinComputation==CGexFileInGroup::SAMPLES) ||
                (pFile->m_eBinComputation==CGexFileInGroup::WAFER_MAP) ||
                bCreateEntryOnly
                )
            return;

        // Keeps track of total number of records SBR and HBR processed.
        (*piRecords)++;
        break;
    }

    // Flags we have a binning summary, then no need in pass2 to create it from sampled data
    pFile->bBinningSummary = true;

    //-- take the test, hardbinning and softbinning from the parent group
    pFile->initPtrParentGroup();
}


//!-------------------------------!//
//!         processPMR            !//
//!-------------------------------!//
void StdfDataFile::processPMR(int lPass)
{
    // Only read in PASS 1
    if(lPass == 2)
        return;

    mPMRRecord.reset();
    mPMRRecord.readPMR(mStdfFile);

    //-- as the pinMap list is for now not linked to a pFile
    //-- we call that function only for the first one that will initialize the
    //-- pinMapList shared between all the pFileInGroup instance
    initFileInGroupPMR(mFileInGroupsBySite.begin()->at(0));

    //loopOverFileInGroup(-1, T_PMR);
}


//!-------------------------------!//
//!         processPGR            !//
//!-------------------------------!//
void StdfDataFile::processPGR(int lPass)
{
    // Only read in PASS 1
    if(lPass == 2)
        return;

    mPGRRecord.readPGR(mStdfFile);
}

//!-------------------------------!//
//!         initFileInGroupPMR    !//
//!-------------------------------!//
void StdfDataFile::initFileInGroupPMR(CGexFileInGroup *lFile)
{

    CPinmap	*ptPinmapCell=0;	// Pointer to a pinmap cell in pinmap list

    switch(mRecordHeader.iStdfVersion)
    {
        default :
            break;

        case GEX_STDFV4:
        {

            if( lFile->FindPinmapCell(&ptPinmapCell, mPMRRecord.mIndex)!=1)
                return;	// some error creating the pinmap cell, then ignore record

            if(mPMRRecord.mChannelNameRes == false)
                return;	// CHAN_NAM
            ptPinmapCell->strChannelName = lFile->FormatTestName(mPMRRecord.mChannelName);

            if(mPMRRecord.mPysicalNameRes == false)
                return;	// PHY_NAM
            ptPinmapCell->strPhysicName =  lFile->FormatTestName(mPMRRecord.mPysicalName);

            if(mPMRRecord.mLogicalNameRes == false)
                return;	// LOG_NAM
            ptPinmapCell->strLogicName =  lFile->FormatTestName(mPMRRecord.mLogicalName);

            ptPinmapCell->bHead = mPMRRecord.mHead;
            ptPinmapCell->m_site = mPMRRecord.mSite;

            break;
        }
    }
}

//!-------------------------------!//
//!         processPLR            !//
//!-------------------------------!//
void StdfDataFile::processPLR(int lPass)
{
       // Only read in PASS 1
    if(lPass == 2)
        return;

    char	szString[257];	// A STDF string is 256 bytes long max!

    int wData;
    mStdfFile.ReadWord(&wData);	// GRP_CNT
    int iGrpCnt,iCount;
    iGrpCnt = iCount = wData;
    while(iCount)
    {
        mStdfFile.ReadWord(&wData);	// GRP_INDX
        --iCount;
    };

    iCount = iGrpCnt;
    while(iCount)
    {
        mStdfFile.ReadWord(&wData);	// GRP_MODE
        --iCount;
    };

    iCount = iGrpCnt;
    BYTE bData;
    while(iCount)
    {
        mStdfFile.ReadByte(&bData);	// GRP_RADX
        --iCount;
    };

    iCount = iGrpCnt;
    while(iCount)
    {
        mStdfFile.ReadString(szString);	// PGM_CHAR
        --iCount;
    };
    iCount = iGrpCnt;
    while(iCount)
    {
        mStdfFile.ReadString(szString);	// RTN_CHAR
        --iCount;
    };
    iCount = iGrpCnt;
    while(iCount)
    {
        mStdfFile.ReadString(szString);	// PGM_CHAL
        --iCount;
    };
    iCount = iGrpCnt;
    while(iCount)
    {
        mStdfFile.ReadString(szString);	// RTN_CHAL
        --iCount;
    };
}

//!-------------------------------!//
//!         processGDR            !//
//!-------------------------------!//
void StdfDataFile::processGDR(void)
{
    GQTL_STDF::Stdf_GDR_V4 gdr;
    gdr.Read( mStdfFile );

    // modify the deciphering mode for encountered site number in other records
    GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInStdf( gdr, mStdfFile );
}

//!-------------------------------!//
//!         processTSR            !//
//!-------------------------------!//
void StdfDataFile::processTSR(int lPass)
{
    mTSRRecord.reset();
    mTSRRecord.readTSR(mStdfFile);

    loopOverFileInGroup(mTSRRecord.mSite, T_TSR, lPass);
}

//!-----------------------------------!//
//!        initFileInGroupTSR         !//
//!-----------------------------------!//
void StdfDataFile::initFileInGroupTSR(CGexFileInGroup *pFile, int lPass)
{
    CTest	*ptTestCell=0;	// Pointer to test cell to receive STDF info.
    bool	bSingleTestEntry=true;
    int		iPinmapMergeIndex;

    // Trim test name to remove unnecessary spaces at beginning and end
    // (for QA) [BG 08/27/2012]
   // clTSR_V4.m_cnTEST_NAM = clTSR_V4.m_cnTEST_NAM.trimmed();

    // Replace ',' with ';' for csv output
    pFile->FormatTestName(mTSRRecord.mTestName);

    // Check site filter
   //  if((iProcessSite >=0) && ( (siteNumber != iProcessSite) || (clTSR_V4.m_u1HEAD_NUM == 255)))
     //   break;	// Site filter not matching.

    // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
  /*  char szTestName[257];	// A STDF string is 256 bytes long max!
    if(mTSRRecord.mTestName.isEmpty() == false)
    {
        strcpy(szTestName, mTSRRecord.mTestName.left(256).toLatin1().constData());
        szTestName[256] = 0;
    }
    else
        szTestName[0] = 0;*/

    switch(mTSRRecord.mTestType)
    {
    // First try to find the test without specifying the test name, and without creating a new test.
    // If not found, specify the test name, and create test if not exist.
    case 'F':
        if(	(pFile->FindTestCell(mTSRRecord.mTestNum,GEX_FTEST,&ptTestCell,true,false) !=1) &&
                (pFile->FindTestCell(mTSRRecord.mTestNum,GEX_FTEST,&ptTestCell,true,true,mTSRRecord.szTestName) !=1))
            return;	// Error
        break;

    case 'M':
        if(pFile->m_eMPRMergeMode == CGexFileInGroup::MERGE)
            iPinmapMergeIndex = GEX_PTEST;	// Merge Multiple parametric pins
        else
            iPinmapMergeIndex = GEX_MPTEST;	// NO Merge Multiple parametric pins
        if(	(pFile->FindTestCell(mTSRRecord.mTestNum,iPinmapMergeIndex,&ptTestCell,true,false) !=1) &&
                (pFile->FindTestCell(mTSRRecord.mTestNum,iPinmapMergeIndex,&ptTestCell,true,true,mTSRRecord.szTestName) !=1))
            return;	// Error
        break;
    case 'P':
        if(	(pFile->FindTestCell(mTSRRecord.mTestNum,GEX_PTEST,&ptTestCell,true,false) !=1) &&
                (pFile->FindTestCell(mTSRRecord.mTestNum,GEX_PTEST,&ptTestCell,true,true,mTSRRecord.szTestName) !=1))
            return;	// Error
        break;
    default:
        // No test type specified, we try to attach the TSR to an existing
        // PTR/MPR/FTR
        if(	(pFile->FindTestCell(mTSRRecord.mTestNum,GEX_PTEST,&ptTestCell,true,false) !=1) &&
                (pFile->FindTestCell(mTSRRecord.mTestNum,GEX_PTEST,&ptTestCell,true,false,mTSRRecord.szTestName) !=1) &&
                (pFile->FindTestCell(mTSRRecord.mTestNum,GEX_MPTEST,&ptTestCell,true,false) !=1) &&
                (pFile->FindTestCell(mTSRRecord.mTestNum,GEX_MPTEST,&ptTestCell,true,false,mTSRRecord.szTestName) !=1) &&
                (pFile->FindTestCell(mTSRRecord.mTestNum,GEX_FTEST,&ptTestCell,true,false) !=1) &&
                (pFile->FindTestCell(mTSRRecord.mTestNum,GEX_FTEST,&ptTestCell,true,false,mTSRRecord.szTestName) !=1))
        {
            // TSR cannot be attached to an existing test,
            // force creation of a new test with Unknown or Invalid
            // test type.
            int lTestType = GEX_INVALIDTEST;

            if (mTSRRecord.mTestType == ' ')
                lTestType = GEX_UNKNOWNTEST;

            if (pFile->FindTestCell(mTSRRecord.mTestNum, lTestType,
                             &ptTestCell, true, true,
                             mTSRRecord.szTestName) !=1)
                return;	// Error
        }

        break;
    }

    // Flag if multiple tests have this same test#
    if(ptTestCell->GetNextTest() != NULL && ptTestCell->lTestNumber == ptTestCell->GetNextTest()->lTestNumber)
        bSingleTestEntry = false;

    // Check if Test type info not initialized yet (occures if no datalog for this test, only summary)
    if(ptTestCell->bTestType == ' ')
        ptTestCell->bTestType = mTSRRecord.mTestType;

    // In case test name was not specified in the PTR but is found in
    // the TSR, let's get it !
    // Also, if this string is more detailed, use it instead.
    // Note: only applies if unique test#
    if(bSingleTestEntry && !ptTestCell->bTestNameMapped && mTSRRecord.mTestNameRes && (ptTestCell->strTestName.length() <mTSRRecord.mTestName.length()))
    {
        // Formats test name (in case to long, or leading spaces)
        if(pFile->mTestMergeRule == TEST_MERGE_NUMBER || ptTestCell->strTestName.isEmpty())
        {
            // For MPR, propagate the new TestName for all pin
            CTest	*lMPTest=0;
            for (int lPin = 0; lPin < ptTestCell->getPinCount(); ++lPin)
            {
                pFile->FindTestCell(mTSRRecord.mTestNum, lPin, &lMPTest, true,false);
                if(lMPTest)
                {
                    lMPTest->strTestName = mTSRRecord.mTestName;
                }
            }

            ptTestCell->strTestName = mTSRRecord.mTestName;
        }
    }

    // Check if this TSR should be used to override test statistics.
    // This depends on the test statistics computation option, and the nb of samples available for the test
    bool bUseTsrForStats = true;
    if(pFile->m_eStatsComputation == CGexFileInGroup::FROM_SUMMARY_ONLY)
    {
        bUseTsrForStats = true;
        ptTestCell->bStatsFromSamples = false;
    }
    if(pFile->m_eStatsComputation == CGexFileInGroup::FROM_SUMMARY_THEN_SAMPLES)
    {
        if(mTSRRecord.mExecCountRes &&  mTSRRecord.mFailCountRes)
        {
            bUseTsrForStats = true;
            ptTestCell->bStatsFromSamples = false;
        }
        else
        {
            bUseTsrForStats = false;
            ptTestCell->bStatsFromSamples = true;
        }
    }
    if (pFile->m_eStatsComputation == CGexFileInGroup::FROM_SAMPLES_ONLY)
    {
        bUseTsrForStats = false;
        ptTestCell->bStatsFromSamples = true;
    }
    if( pFile->m_eStatsComputation == CGexFileInGroup::FROM_SAMPLES_THEN_SUMMARY)
    {
        if(ptTestCell->ldSamplesValidExecs > 0)
        {
            bUseTsrForStats = false;
            ptTestCell->bStatsFromSamples = true;
        }
        else
        {
            bUseTsrForStats = true;
            ptTestCell->bStatsFromSamples = false;
        }
    }

    // Check if special TSR from GEXDB holding sample data (in this case, no samples are available for the test, but TSR has been created from sample stats)
    if(mTSRRecord.mSeqName == GEXDB_PLUGIN_TSR_STATS_SOURCE_SAMPLES)
    {
        bUseTsrForStats = true;
        ptTestCell->bStatsFromSamples = true;
    }

    // If TSR should be ignored, just return
    if(!bUseTsrForStats)
        return;

    // If looking for merged result only...flag if such record exist!
    if(lPass == 1)
    {
        if(((int)mTSRRecord.mHead) == 255)
            ptTestCell->bMergedTsrExists = true;
        return;
    }

    // We're here in pass2 only...check if need to merge TSR of all sites ourselves?
    if((pFile->iProcessSite < 0) && (ptTestCell->bMergedTsrExists == true) && (((int)mTSRRecord.mHead) != 255))
        return;

    // Cumul count if multiple files are merged.
    if(ptTestCell->ldExecs>0)
        ptTestCell->ldExecs += mTSRRecord.mExecCount;
    else
        ptTestCell->ldExecs = mTSRRecord.mExecCount;

    // Do not take Summary Failure count if Count must be done from samples
    {
        if((pFile->m_eFailCountMode == CGexFileInGroup::FAILCOUNT_ALL) || (pFile->pReportOptions->bSpeedUseSummaryDB))
        {
            // Cumul count if multiple files are merged
            if(ptTestCell->GetCurrentLimitItem()->ldFailCount>0)
                ptTestCell->GetCurrentLimitItem()->ldFailCount += mTSRRecord.mFailCount;
            else
                ptTestCell->GetCurrentLimitItem()->ldFailCount = mTSRRecord.mFailCount;
        }
    }

    double tmp_double;
    if(mTSRRecord.mTestMinRes )
    {
        if(!(mTSRRecord.mOptFlag & 1)){
            tmp_double = mTSRRecord.mTestMin;
            pFile->m_cStats.CheckIncorrectScale(ptTestCell,
                                         ptTestCell->lfSamplesMin,
                                         &tmp_double);
            ptTestCell->lfMin = gex_min( mTSRRecord.mTestMin,ptTestCell->lfMin);	// TEST_MIN is valid
        }else{
            ptTestCell->lfMin = C_INFINITE;
        }
    } else
        ptTestCell->lfMin = C_INFINITE;

    if(mTSRRecord.mTestMaxRes   )
    {
        if(!(mTSRRecord.mOptFlag & 2)){

            tmp_double = mTSRRecord.mTestMax;
            pFile->m_cStats.CheckIncorrectScale(ptTestCell,
                                         ptTestCell->lfSamplesMax,
                                         &tmp_double);
            ptTestCell->lfMax = gex_max(mTSRRecord.mTestMax,ptTestCell->lfMax);	// TEST_MAX is valid
        }else
            ptTestCell->lfMax = -C_INFINITE;
    }else
        ptTestCell->lfMax = -C_INFINITE;

    if(mTSRRecord.mTestSumRes)
    {
        if(!(mTSRRecord.mOptFlag & 16)){

            // Check if scaling is needed on test_sum
            // TEST_SUMS is valid
            // Cumul Sum if multiple files merged
            if(ptTestCell->lfTotal != -C_INFINITE)
                ptTestCell->lfTotal += mTSRRecord.mTestSum;
            else
                ptTestCell->lfTotal = mTSRRecord.mTestSum;
        }else{
            ptTestCell->lfTotal = -C_INFINITE;
        }
    }else
        ptTestCell->lfTotal = -C_INFINITE;

    if(mTSRRecord.mTestSquareRes && !(mTSRRecord.mOptFlag & 32))
    {
        if(!(mTSRRecord.mOptFlag & 32)){
            // Check if scaling is needed on test_sqr
            // TEST_SQRS is valid
            // Cumul SumSquare if multiple files merged
            if(ptTestCell->lfTotalSquare != -C_INFINITE)
                ptTestCell->lfTotalSquare += mTSRRecord.mTestSquare;
            else
                ptTestCell->lfTotalSquare = mTSRRecord.mTestSquare;
        }else
            ptTestCell->lfTotalSquare = -C_INFINITE;
    }else
        ptTestCell->lfTotalSquare = -C_INFINITE;

}



bool	StdfDataFile::readStaticDataMPR( CGexFileInGroup *pFile,
                                     CTest ** ptParamTestCell,
                                       unsigned long lTestNumber,
                                       long iPinmapMergeIndex,
                                       long nJcount,
                                       long nKcount,
                                       int *piCustomScaleFactor,
                                       bool bStrictLL, bool bStrictHL, int lPass)
{
   // long lRecordReadOffset          = 0;
   // long lRecordReadOffsetRtnIndx   = 0;
    bool bIgnoreSite                = false;

    // If more results than Pin indexes...ignore the leading results
    if(iPinmapMergeIndex != GEX_PTEST)
    {
        // Multiple results: we need the individual pinmaps.
        if((nKcount==0) && (nJcount==0))
            return true;	// No Result index list...then ignore this MPR!
    }

    if(((pFile->iProcessSite >=0) && (pFile->iProcessSite != mMPRRecord.mSite)) )
    {
        bIgnoreSite = true;
    }

    CTest *		ptTestCell = NULL;
    CTest	*	ptMPTestCell = NULL;	// Pointer to test cell to create
    double		lfLowLimit=-C_INFINITE, lfHighLimit=C_INFINITE;
    //char		szString[257]="";	// A STDF string is 256 bytes long max!
   // int			iPinIndex=0,iCount=0,iPinCount=0;
    //BYTE		bOptFlag=0;
    //BYTE		bRes_scal=0;
    bool        isFirstRtnIndxOccurence = false;
    int iPinCount = 0, iPinIndex  = 0;
    QString szString;

    // Saves current READ offset in the STDF record.
   // lRecordReadOffset = StdfFile.GetReadRecordPos();

    // Skip the RTN_RSLT fields ('nKcount' fields of 4bytes to skip)
    // so we point : TEST_TXT
   // if(StdfFile.SetReadRecordPos(lRecordReadOffset + 4*nKcount) != GS::StdLib::Stdf::NoError)
//        goto exit_function;	// Record is not that big !

    if(mMPRRecord.mTestTxtRes == false)
        goto exit_function;	// TEST_TXT text description (test name)

    // Formats test name (in case to long, or leading spaces)
    szString = mMPRRecord.mTestTxt;
    pFile->FormatTestName(szString);

    // Returns pointer to correct cell. If cell doesn't exist ; it's created...Test# mapping enabled.
    if(pFile->FindTestCell(lTestNumber, iPinmapMergeIndex, &ptTestCell, true, true,szString.toStdString().c_str()) !=1)
        return false;	// Error

    // Get the pointer on the test cell created
    *ptParamTestCell = ptTestCell;

    if(mMPRRecord.mAlarmIdRes == false)
        goto exit_function;	// ALARM_ID


    // Optional fields
    *piCustomScaleFactor = 0;
    if(mMPRRecord.mOptFlagRes == false)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto read_RTNINDX;	// OPT_FLG
    }

    // Read result-scale factor in case it is a custom one for this record
    if((mMPRRecord.mResScalRes == false) && (!(mMPRRecord.mOptFlag & 1)))
    {
        // Compute custom scale factor.: WILL BE INCORRECT ON 1st execution of PASS1
        // ... as ptTestCell->res_scal is not defined yet!...this is why this code
        // appears few lines below as well!
        *piCustomScaleFactor = (int)mMPRRecord.mResScal - ptTestCell->res_scal;
    }

    if((lPass == 2) || (ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_GOTTL))
    {
        // Pass1: ignore test definition limits if already defined!
        // Pass2: only refresh the list of pinmap indexes to use

        // Compute offset to seek to RTN_INDX offset: current offset+18 bytes
       // lRecordReadOffsetRtnIndx = StdfFile.GetReadRecordPos()+18;
        goto read_RTNINDX;
    }

    // Are test limits strict??
    if(!bStrictLL)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_LTLNOSTRICT;
    if(!bStrictHL)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_HTLNOSTRICT;

    // If not in 'What-if' mode, save limits flag seen in STDF file
    if((mMPRRecord.mOptFlag & 0x10) == 0 && pFile->mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY)
    {
        // LowLimit+low_scal flag is valid...
        ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
        if(mMPRRecord.mOptFlag & 0x10)	// Low Limit in first PTR
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
        else if(mMPRRecord.mOptFlag & 0x40)	// No Low Limit
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
    }
    // If not in 'What-if' mode, save limits flag seen in STDF file
    if((mMPRRecord.mOptFlag & 0x20) == 0 && pFile->mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY)
    {
        // HighLimit+high_scal flag is valid...
        ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);
        if(mMPRRecord.mOptFlag & 0x20)	// High Limit in first PTR
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
        else if(mMPRRecord.mOptFlag & 0x80)	// No High Limit
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
    }
    // If What-if has forced a LL
    if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL) == 0)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
    // If What-if has forced a HL
    if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL) == 0)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);

    // If reached this point, it means test limits have been defined!
    if (bIgnoreSite == false)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_GOTTL;

    if((mMPRRecord.mOptFlag & 0x1) == 0)	// res-scal is valid
    {
        ptTestCell->res_scal = mMPRRecord.mResScal;
        // // Compute custom scale factor as first computation few lines above is incorrect in 1st call in pass#1!
        *piCustomScaleFactor = (int)mMPRRecord.mResScal - ptTestCell->res_scal;
    }

    if(mMPRRecord.mLowLimScalRes == false)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto read_RTNINDX;
    }

    if(((mMPRRecord.mLowLimScal & 0x50) == 0) && ((mMPRRecord.mOptFlag & 0x10) == 0))	// llm-scal is valid
        ptTestCell->llm_scal = mMPRRecord.mLowLimScal;

    if(mMPRRecord.mHighLimScalRes == false)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto read_RTNINDX;// hlm_scal
    }
    if(((mMPRRecord.mHighLimScal & 0x120) == 0) && ((mMPRRecord.mOptFlag & 0x20) == 0))	// hlm-scal is valid
        ptTestCell->hlm_scal = mMPRRecord.mHighLimScal;


    ///////////////////Read Low Limit
    if(mMPRRecord.mLoLimitRes == false)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto read_RTNINDX;// low limit
    }
    if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            && ((mMPRRecord.mOptFlag & 0x10) == 0)
            && (pFile->mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY))
        lfLowLimit = M_CHECK_INFINITE((double)mMPRRecord.mLoLimit); // Low limit exists: keep value

    if(mMPRRecord.mHiLimitRes == false)
    {
        // if not "Use spec limits only"
        if (pFile->mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY)
            ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto read_RTNINDX;// Low limit
    }
    if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            && ((mMPRRecord.mOptFlag & 0x20) == 0)
            && (pFile->mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY))
        lfHighLimit = M_CHECK_INFINITE((double)mMPRRecord.mHiLimit);// High limit exists: keep value


    if(mMPRRecord.mStartInRes == false)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto CalculateMPLimits;// start_in
    }
    if(mMPRRecord.mOptFlag & 0x1)
        ptTestCell->lfStartIn = mMPRRecord.mStartIn;

    if(mMPRRecord.mIncrInRes == false)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto CalculateMPLimits;// incr_in
    }
    if(mMPRRecord.mOptFlag & 0x1)
        ptTestCell->lfIncrementIn = mMPRRecord.mIncrIn;

    // Skip Pinmap index list for now...but will be read later in this function!
    // Skip 'nJcount' fields of 2bytes!
    // Saves current READ offset in the STDF record.
   // lRecordReadOffsetRtnIndx = StdfFile.GetReadRecordPos();
    //if(StdfFile.SetReadRecordPos(lRecordReadOffsetRtnIndx+2*nJcount) != GS::StdLib::Stdf::NoError)
     //   goto CalculateMPLimits;	// Record is not that big !

    // UNITS
    //StdfFile.ReadString(szString);
    //szString[GEX_UNITS-1] = 0;
    // If unit specified (and using printable ascii characters), then use it.

    if(mMPRRecord.mUnits[0] >= ' ')
        strcpy(ptTestCell->szTestUnits, mMPRRecord.mUnits.trimmed().toLatin1().constData());

    // UNITS_IN
   // StdfFile.ReadString(szString);
    //szString[GEX_UNITS-1] = 0;
    if(mMPRRecord.mUnitsInRes)
        strcpy(ptTestCell->szTestUnitsIn,mMPRRecord.mUnitsIn.toStdString().c_str());

    //Read unused info
    /*if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        goto CalculateMPLimits; // C_RESFMT
    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        goto CalculateMPLimits; // C_LLMFMT
    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        goto CalculateMPLimits; // C_HLMFMT*/

    // LO_SPEC
    if(mMPRRecord.mLowSpecRes == false)
        goto CalculateMPLimits;

    ptTestCell->lfLowSpecLimit = M_CHECK_INFINITE((double)mMPRRecord.mLowSpec);

    // If not "Use standard limits only"
    if(pFile->mUsedLimits == CGexFileInGroup::SPEC_LIMIT_IF_ANY)
    {
        // Flag test limit validity depending on spec limits flag
        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
        {
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;
            lfLowLimit = M_CHECK_INFINITE((double)mMPRRecord.mLowSpec);

            // No low specs limits as there are kept in std limits
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
        }
    }
    else if(pFile->mUsedLimits == CGexFileInGroup::SPEC_LIMIT_ONLY)
    {
        lfLowLimit = M_CHECK_INFINITE((double)mMPRRecord.mLowSpec);

        // Flag test limit validity depending on spec limits flag
        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
        {
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;

            // No low specs limits as there are kept in std limits
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
        }
        else
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
    }

    // HI_SPEC
    if(mMPRRecord.mHighSpecRes == false)
        goto CalculateMPLimits;

    ptTestCell->lfHighSpecLimit = M_CHECK_INFINITE((double)mMPRRecord.mHighSpec);

    // If not "Use standard limits only"
    if(pFile->mUsedLimits == CGexFileInGroup::SPEC_LIMIT_IF_ANY)
    {
        // Flag test limit validity depending on spec limits flag
        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
        {
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;
            lfHighLimit = M_CHECK_INFINITE((double)mMPRRecord.mHighSpec);

            // No low specs limits as there are kept in std limits
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
        }
    }
    else if(pFile->mUsedLimits == CGexFileInGroup::SPEC_LIMIT_ONLY)
    {
        lfHighLimit = M_CHECK_INFINITE((double)mMPRRecord.mHighSpec);

        // Flag test limit validity depending on spec limits flag
        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
        {
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;

            // No low specs limits as there are kept in std limits
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
        }
        else
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
    }

CalculateMPLimits:

    if(pFile->m_datasetConfig.testToUpdateList().isActivated()){
        GexTestToUpdate *poUpdate = pFile->m_datasetConfig.testToUpdateList().getTestToUpdate(ptTestCell->lTestNumber);
        if(poUpdate){
            ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag = CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;
            if(!poUpdate->highLimit().isEmpty()){
                lfHighLimit = poUpdate->highLimit().toDouble();
                if(poUpdate->source() == "whatif")
                    ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag	&= ~CTEST_LIMITFLG_NOHTL;
            }
            if(!poUpdate->lowLimit().isEmpty()){
                lfLowLimit = poUpdate->lowLimit().toDouble();
                if(poUpdate->source() == "whatif")
                    ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag	&= ~CTEST_LIMITFLG_NOLTL;
            }
        }
    }

    // Store the test limits for this site
    if (pFile->m_ParentGroup->addTestSiteLimits(ptTestCell, pFile->iProcessSite, mMPRRecord.mSite, lfLowLimit, lfHighLimit,
                                         mMPRRecord.mOptFlag, getMirDatas().lStartT) == false)
        return false;

    // Retrieve the min and max tests limits for this site
    pFile->m_ParentGroup->getTestSiteLimits(ptTestCell, pFile->iProcessSite, mMPRRecord.mSite, lfLowLimit, lfHighLimit);

    // If High limit not forced by What-If...
    if (!bIgnoreSite && (ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL)
        ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighLimit;

    // If Low limit not forced by What-If...
    if(!bIgnoreSite && (ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL)
        ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;

    // Avoid failing if LL=HL
    if(ptTestCell->GetCurrentLimitItem()->lfLowLimit == ptTestCell->GetCurrentLimitItem()->lfHighLimit)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= (CTEST_LIMITFLG_LTLNOSTRICT | CTEST_LIMITFLG_HTLNOSTRICT);


    // Here, we read the list of indexes: it may be possible that for each run,
    // the list changes for a same test!
read_RTNINDX:
    // Now, create as many test cells as Pins tested in this test!
    // Seek into record offset to read the RTN_INDX table!
   // if(StdfFile.SetReadRecordPos(lRecordReadOffsetRtnIndx) != GS::StdLib::Stdf::NoError)
     //   goto exit_function;	// Record is not that big !...then we will use the latest pinmap indexes read for that test.
    if(ptTestCell->ptResultArrayIndexes != NULL)
        delete [] ptTestCell->ptResultArrayIndexes;	// erase previous buffer if any.
    // If current mode is to merge multiple parametric values...then exit now!
    if(ptTestCell->lPinmapIndex == GEX_PTEST)
    {
        ptTestCell->ptResultArrayIndexes = NULL;
        ptTestCell->bTestType = 'M';
        goto exit_function;
    }

    iPinCount = nJcount;
    isFirstRtnIndxOccurence = false;
    if(nJcount==0)
        iPinCount = nKcount;
    // Stores only first valid occurence of RTN_INDX
    else if (ptTestCell->ptFirstResultArrayIndexes == NULL)
    {
        ptTestCell->ptFirstResultArrayIndexes = new WORD[iPinCount];
        if(ptTestCell->ptFirstResultArrayIndexes == NULL)
            return false;	// Error: memory alloc. failure...ignore this MPR
        isFirstRtnIndxOccurence = true;
    }

    ptTestCell->ptResultArrayIndexes = new WORD[iPinCount];
    ptTestCell->setPinCount(iPinCount);
    if(ptTestCell->ptResultArrayIndexes == NULL)
        return false;	// Error: memory alloc. failure...ignore this MPR

    for(int iCount=0;iCount<iPinCount;iCount++)
    {
        // Retrieve Pinmap index if any
        int wData = iCount + 1; // case 5329 legal range starts at 1
        if(nJcount > 0 && mMPRRecord.mPMRINdexes.size() > iCount)
            wData = mMPRRecord.mPMRINdexes[iCount];
        // Check if we have a reference array
        else if (!isFirstRtnIndxOccurence && ptTestCell->ptFirstResultArrayIndexes)
            wData = ptTestCell->ptFirstResultArrayIndexes[iCount];
        iPinIndex = wData;

        // save array of pinmap indexes.
        ptTestCell->ptResultArrayIndexes[iCount] = iPinIndex;
        if (isFirstRtnIndxOccurence)
            ptTestCell->ptFirstResultArrayIndexes[iCount] = iPinIndex;
        if(lPass == 1)
        {
            // Pass1: Save it into the test master record..Test# mapping enabled.
            if (pFile->FindTestCell(lTestNumber, iCount, &ptMPTestCell,
                             true, true,
                             ptTestCell->strTestName.toLatin1().data()) != 1)
            {
                goto exit_function;  // Error
            }
            ptMPTestCell->bTestType = 'M';			// Test type: Parametric multi-results
            // Copy test static info from master test definition
            ptMPTestCell->strTestName = ptTestCell->strTestName;
            ptMPTestCell->GetCurrentLimitItem()->bLimitFlag = ptTestCell->GetCurrentLimitItem()->bLimitFlag;
            ptMPTestCell->res_scal = ptTestCell->res_scal;
            ptMPTestCell->llm_scal = ptTestCell->llm_scal;
            ptMPTestCell->hlm_scal = ptTestCell->hlm_scal;
            ptMPTestCell->lfStartIn = ptTestCell->lfStartIn;
            ptMPTestCell->lfIncrementIn = ptTestCell->lfIncrementIn;
            strcpy(ptMPTestCell->szTestUnits, ptTestCell->szTestUnits);
            strcpy(ptMPTestCell->szTestUnitsIn, ptTestCell->szTestUnitsIn);

            // Retrieve the min and max tests limits for this site
            pFile->m_ParentGroup->getTestSiteLimits(ptTestCell, pFile->iProcessSite, mMPRRecord.mSite, lfLowLimit, lfHighLimit);

            // If What-if has forced a LL
            if((ptMPTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL) == 0)
                ptMPTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
            else
                ptMPTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;

            // If What-if has forced a HL
            if((ptMPTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL) == 0)
                ptMPTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);
            else
                ptMPTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighLimit;
        }
    }

    // Keeps track of test result array size
    ptTestCell->lResultArraySize = gex_max(ptTestCell->lResultArraySize,iPinCount);

    // MergeMultiParametricTests
    //ptTestCell->lResultArraySize=0;

    // Before returning to MPR processing, seekback into right record offset!
exit_function:
    // 'rewind' offset so we can now read test result fields!
    //StdfFile.SetReadRecordPos(lRecordReadOffset);
    return true;	// OK
}

//!-------------------------------!//
//!         processMPR            !//
//!-------------------------------!//
void StdfDataFile::processMPR(int lPass)
{
    mMPRRecord.reset();
    mMPRRecord.readMPR(mStdfFile);


   /* int lSiteNumber = mPTRRecord.mSite;

    //-- for to all sites
    if(mMPRRecord.isTestDef())
        lSiteNumber = -1;
    else
        lSiteNumber = mPTRRecord.mSite;*/

    // force to all pFile
    loopOverFileInGroup(-1, T_MPR, lPass);
}

//!-----------------------------------!//
//!        initFileInGroupMPR         !//
//!-----------------------------------!//
void StdfDataFile::initFileInGroupMPR(CGexFileInGroup *pFile, int lPass)
{
    int		test_flg=0,param_flg=0;
    bool	bValideResult=false;
    bool	bTestIsFail=false, bOutsideLimit=false;
    bool	bAlarmResult=false;
    bool	bIsOutlier=false;
    bool	bStrictLL=true, bStrictHL=true;
    CTest	*ptTestCell=NULL;	// Pointer to test cell to receive STDF info.
    CTest	*ptMPTestCell=NULL;	// Pointer to test cell to create
    int		iPinIndex=0;
    int		iPinmapMergeIndex=0;

    CTestResult::PassFailStatus	ePassFailStatus	= CTestResult::statusUndefined;

    int		nJcount=0,nKcount=0;//,iCount=0;
    int		iCustomScaleFactor;

    // Ignore MPR if filtering set to 'Bin data only'
    if(pFile->iProcessBins == GEX_PROCESSPART_NO_SAMPLES)
        return;

    // Check if current MPR (test result) belongs to a part that we filter ?
    // If this part has a binning the User doesn't want to process, then
    // we skip this record...
    if(lPass == 2 && pFile->PartProcessed.IsFilteredPart(pFile->pReportOptions, mMPRRecord.mSite, false, pFile->iProcessBins) == true)
        return;

    // Ignore test if we are in 'Stop on Fail' mode, and current site already failed!
    if((lPass == 2) && (pFile->m_eFailCountMode == CGexFileInGroup::FAILCOUNT_FIRST) && (pFile->m_map_ptFailTestCell.contains(mMPRRecord.mSite)))
        return;

    switch(mRecordHeader.iStdfVersion)
    {
            break;

        case GEX_STDFV4:
            //StdfFile.ReadByte(&bData);		// test_flg
            test_flg = (int) mMPRRecord.mTestFlag & 0xff;
            //StdfFile.ReadByte(&bData);		// parm_flg
            param_flg = (int) mMPRRecord.mParamFlag & 0xff;

            // RTN_ICNT: Count (j) of PMR indexes
            if(mMPRRecord.mIndexCountRes == false)
                return ;
            nJcount = mMPRRecord.mIndexCount;

            // RSLT_CNT: Count (k) of returned results
            if(mMPRRecord.mReturnResultCountRes == false)
                return ;
            nKcount = mMPRRecord.mReturnResultCount;

            // Check if test limits are strict
            if(param_flg & 0100)
                bStrictLL = false;
            if(param_flg & 0200)
                bStrictHL = false;

            // Check if Pass/Fail indication is valid (bit 6 cleared)
            // Only if Pass/Fail rule option is checked
            if (pFile->m_bUsePassFailFlag && (test_flg & 0x40) == 0x00)
            {
                // If bit 7 set, then test is fail, otherwise test is pass
                ePassFailStatus = (test_flg & 0x80) ? CTestResult::statusFail : CTestResult::statusPass;
            }

            // Check if test result is valid
            if((test_flg & 16) || ((mMPRRecord.mParamFlag & 07) & (pFile->m_eStdfCompliancy==CGexFileInGroup::STRINGENT)))
                bValideResult = false;
            else
                bValideResult = true;

            // Ignore test result if must compute statistics from summary only
            if (pFile->m_eStatsComputation == CGexFileInGroup::FROM_SUMMARY_ONLY)
                bValideResult = false;

            // Check if test has an alarm
            if(test_flg & 1)
                bAlarmResult = true;

            // Read array of states( Nibbles: 8 bits = 2 states of 4 bits!).
            /*iCount = nJcount;
            while(iCount>0)
            {
                StdfFile.ReadByte(&bData);		// State[j]
                iCount-=2;
            }*/

            // Merge Pins under single test name or not?
            if(pFile->m_eMPRMergeMode== CGexFileInGroup::MERGE)
                iPinmapMergeIndex = GEX_PTEST;	// Merge Multiple parametric pins
            else
                iPinmapMergeIndex = GEX_MPTEST;	// NO Merge Multiple parametric pins

            // Read Static information (located at the end of the record)
            ptTestCell = NULL;
            if(readStaticDataMPR(pFile, &ptTestCell, mMPRRecord.mTestNumber, iPinmapMergeIndex, nJcount,nKcount,
                                 &iCustomScaleFactor, bStrictLL, bStrictHL, lPass) == false)
                return;  // some kind of error: ignore this record

            if((pFile->iProcessSite >=0) && ((pFile->iProcessSite != (int)mMPRRecord.mSite)))
                return;	// Site filter not matching.

            // Check if record is very short and doesn't even include the Parameter name
            if(ptTestCell == NULL)
            {
                // No Parameter name found in this record...then rely on Parameter Number only...
                if(pFile->FindTestCell(mMPRRecord.mTestNumber, iPinmapMergeIndex, &ptTestCell) !=1)
                    return;  // Error
            }

            //GCORE-17470 to distinguish MPR from cases where several PTRs will be merged because they have same nb AND same name
            if(ptTestCell->GetCurrentLimitItem() != NULL)
                ptTestCell->GetCurrentLimitItem()->setIsParametricItem(true);

            // Update Test# as mapping is activated...
            mMPRRecord.mTestNumber = ptTestCell->lTestNumber;

            // If more results than Pin indexes...ignore the leading results
            if(iPinmapMergeIndex != GEX_PTEST)
            {
                // Multiple results: we need the individual pinmaps.
                if((nJcount>0) && (nKcount > nJcount))
                    nKcount = nJcount;
            }
            // Read each result in array...and process it!
            for(int iCount=0;iCount<nKcount;iCount++)
            {
                if(iPinmapMergeIndex == GEX_PTEST)
                {
                    // If merging Multiple parametric results.
                    iPinIndex = GEX_PTEST;
                    ptMPTestCell = ptTestCell;
                }
                else
                {
                    // Find Test cell matching current pin result
                    if(ptTestCell->ptResultArrayIndexes)
                        iPinIndex = ptTestCell->ptResultArrayIndexes[iCount];
                    else
                        iPinIndex = 1;

                    // DO NOT allow test# mapping as it was already done in above call to 'FindTestCell'
                    if (pFile->FindTestCell(mMPRRecord.mTestNumber, iCount, &ptMPTestCell,
                                     false, true,
                                     ptTestCell->strTestName.
                                     toLatin1().data()) != 1)
                    {
                        return;  // Error
                    }
                }

                // Read Pin Parametric test result.
               // mStdfFile.ReadFloat(&fResult, &bIsNAN);
                if(mMPRRecord.mReturnResultsRes[iCount])
                    goto NextTestResult;

                if(bValideResult != true)
                    goto NextTestResult;

                if(lPass == 1)
                {
                    // Check if this test is an outlier
                    if(pFile->PartIsOutlier(ptMPTestCell,mMPRRecord.mReturnResults[iCount]) == true)
                        goto NextTestResult;

                    // Updates Min/Max values
                    pFile->UpdateMinMaxValues(ptMPTestCell,mMPRRecord.mReturnResults[iCount]);

                    ptMPTestCell->addMultiResultCount(mMPRRecord.mSite);
                }
                else
                {
                    // PASS2:

                    // Ignore STDF FAIL flag if What-If limits defined!
                    if(ptMPTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
                        ePassFailStatus = CTestResult::statusUndefined;

                    // Check if this test is an outlier
                    bIsOutlier = pFile->PartIsOutlier(ptMPTestCell,mMPRRecord.mReturnResults[iCount]);

                    // Check if test is fail
                    bTestIsFail = pFile->IsTestFail(ptTestCell, mMPRRecord.mReturnResults[iCount], ePassFailStatus, bOutsideLimit);

                    // Keep track of failing count (unless it's an outlier)
                    if((bIsOutlier == false) && pFile->CountTestAsFailure(mMPRRecord.mSite,ptTestCell,iPinIndex,mMPRRecord.mReturnResults[iCount],bTestIsFail) == true)
                    {
                        // Update failing count
                        ptMPTestCell->GetCurrentLimitItem()->ldSampleFails++;
                        pFile->m_bFailingSiteDetails[mMPRRecord.mSite] |= GEX_RUN_FAIL_TEST;

                        // If failure and What-If limits enabled, make sure to force a failing bin!
                        if(ptMPTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
                            pFile->m_bFailingSiteDetails[mMPRRecord.mSite] |= GEX_RUN_FAIL_WHATIFTEST;
                    }

                    // If failure, make sure we mark it.
                    if(bTestIsFail)
                        pFile->CountTrackFailingTestInfo(mMPRRecord.mSite,ptTestCell,iPinIndex,mMPRRecord.mReturnResults[iCount]);

                    // Check if test doesn't have a name...if so, build it
                    // using the master test name (probably read in TSR at
                    // end of pass 1), and add to it the Pinmap logical name.
                    if(ptMPTestCell->strTestName.isEmpty() == true)
                        pFile->BuildMultiResultParamTestName(ptTestCell, ptMPTestCell);

                    // Advanced report is Enabled, and test in filter limits.
                    switch(pFile->pReportOptions->getAdvancedReport())
                    {
                        case GEX_ADV_DISABLED:
                        case GEX_ADV_PAT_TRACEABILITY:
                        default:
                            // If Gex-Advanced edition, ALWAYS collect test results!
                            if(bIsOutlier)
                                goto NextTestResult;
                            // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                            if(pFile->pReportOptions->bSpeedCollectSamples)
                            {
                                if (pFile->AdvCollectTestResult(ptMPTestCell, mMPRRecord.mReturnResults[iCount], ePassFailStatus, mMPRRecord.mSite, true) == false)
                                    return;
                            }
                            break;

                        case GEX_ADV_DATALOG:
                            if (GexAbstractDatalog::existInstance())
                            {
                                if (GexAbstractDatalog::isDataloggable(ptTestCell, bTestIsFail, bIsOutlier))
                                {
                                    int nRunIndex = pFile->PartProcessed.runIndexFromSite(mMPRRecord.mSite);
                                    if (nRunIndex >= 0 && nRunIndex < pFile->pPartInfoList.count())
                                        GexAbstractDatalog::pushParametricResult(pFile,
                                                                                 ptTestCell,
                                                                                 pFile->pPartInfoList.at(nRunIndex),
                                                                                 mMPRRecord.mReturnResults[iCount],
                                                                                 bTestIsFail,
                                                                                 bAlarmResult,
                                                                                 mMPRRecord.mSite);
                                }
                            }
                            else
                                pFile->AdvCollectDatalog(ptMPTestCell,
                                                         mMPRRecord.mReturnResults[iCount],
                                                         bTestIsFail,
                                                         bAlarmResult,
                                                         bIsOutlier,
                                                         mMPRRecord.mSite,
                                                         (int)mMPRRecord.mHead,
                                                         (int)mMPRRecord.mSite);

                            // Check if outlier only after datalog, in case we collect outliers!
                            if(bIsOutlier)
                                goto NextTestResult;
                            // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                            if(pFile->pReportOptions->bSpeedCollectSamples)
                                pFile->AdvCollectTestResult(ptMPTestCell, mMPRRecord.mReturnResults[iCount] ,ePassFailStatus, mMPRRecord.mSite, true);

                            break;
                        case GEX_ADV_HISTOGRAM:
                        case GEX_ADV_TREND:
                        case GEX_ADV_CANDLE_MEANRANGE:
                        case GEX_ADV_GO_DIAGNOSTICS:	// Optimizer Diagnostic takes ALL Trend results!
                            if(bIsOutlier)
                                goto NextTestResult;
                            pFile->AdvCollectTestResult(ptMPTestCell, mMPRRecord.mReturnResults[iCount], ePassFailStatus, mMPRRecord.mSite, pFile->pReportOptions->bSpeedCollectSamples);

                            break;
                        case GEX_ADV_CORRELATION:
                            if(bIsOutlier)
                                goto NextTestResult;

                            pFile->AdvCollectTestResult(ptMPTestCell, mMPRRecord.mReturnResults[iCount], ePassFailStatus, mMPRRecord.mSite, pFile->pReportOptions->bSpeedCollectSamples);
                            break;
                    }
                    // If this is an outlier (data to totally IGNORE)...don't take it into account!
                    if(bIsOutlier)
                        return ;

                    // Build Cpk
                    ptMPTestCell->lfSamplesTotal += mMPRRecord.mReturnResults[iCount];				// Sum of X
                    ptMPTestCell->lfSamplesTotalSquare += (mMPRRecord.mReturnResults[iCount]*mMPRRecord.mReturnResults[iCount]);// Sum of X*X

                    ptTestCell->mHistogramData.AddValue(mMPRRecord.mReturnResults[iCount]);
                    if((mMPRRecord.mReturnResults[iCount] >= ptMPTestCell->GetCurrentLimitItem()->lfHistogramMin) && (mMPRRecord.mReturnResults[iCount] <= ptMPTestCell->GetCurrentLimitItem()->lfHistogramMax))
                    {
                        double lfCellHistogramMax = ptMPTestCell->GetCurrentLimitItem()->lfHistogramMax;
                        double lfCellHistogramMin = ptMPTestCell->GetCurrentLimitItem()->lfHistogramMin;

                        int iCell = 0;
                        if(lfCellHistogramMin != lfCellHistogramMax)        // Comment : lfCellHistogramMin <= mMPRRecord.mReturnResults[iCount] <= lfCellHistogramMax already tested
                        {
                            iCell = (unsigned int)( (TEST_HISTOSIZE*(mMPRRecord.mReturnResults[iCount] - lfCellHistogramMin)) / (lfCellHistogramMax-lfCellHistogramMin));
                        }
                        // else, only one bar iCell = 0

                        // Incluse result if it in into the viewing window.
                        if( (iCell>=0) && (iCell<TEST_HISTOSIZE) )      // check iCell correspond to a valid table index (ptMPTestCell->lHistogram[] size = TEST_HISTOSIZE)
                            ptMPTestCell->lHistogram[iCell]++;
                        else if (iCell >= TEST_HISTOSIZE)
                            ptMPTestCell->lHistogram[TEST_HISTOSIZE-1]++;   /// TO REVIEW : is it normal that iCell can exceed than TEST_HISTOSIZE
                        else
                        {
                            GSLOG(SYSLOG_SEV_ERROR,
                                     QString("Invalid value detected to build histogram for test %1 [%2]")
                                     .arg(ptMPTestCell->lTestNumber)
                                     .arg(ptMPTestCell->strTestName).toLatin1().constData());

                            GSLOG(SYSLOG_SEV_DEBUG, QString("\tCell Found: %1").arg(iCell).toLatin1().constData());
                            GSLOG(SYSLOG_SEV_DEBUG, QString("\tMPR result: %1").arg( mMPRRecord.mReturnResults[iCount]).toLatin1().constData());
                            GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Min: %1").arg(
                                     ptMPTestCell->GetCurrentLimitItem()->lfHistogramMin).toLatin1().constData());
                            GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Max: %1").arg(
                                     ptMPTestCell->GetCurrentLimitItem()->lfHistogramMax).toLatin1().constData());
                        }
                    }
                } // In pass2 only
                // Loop over all tests in array.
NextTestResult:;
            }	// Loop for as many test results as Pins in the pin list...

    }// STDF version switch.
    return ;
}

//!-------------------------------!//
//!         processFTR            !//
//!-------------------------------!//
void StdfDataFile::processFTR(int lPass)
{
    mFTRRecord.reset();
    mFTRRecord.readFTR(mStdfFile);

    loopOverFileInGroup(mFTRRecord.mSite, T_FTR, lPass);
}

//!-------------------------------!//
//!         processSDR            !//
//!-------------------------------!//
void StdfDataFile::processSDR(int lPass)
{
    if(lPass == 2)
        return;

    mSDRRecord.reset();
    mSDRRecord.readSDR(mStdfFile);

    //-- Loop over all pFile
    FileInGroupContainerIter lIter(mFileInGroupsBySite.begin()), lIterEnd(mFileInGroupsBySite.end());
    for(;lIter != lIterEnd;++lIter)
    {
        QList<CGexFileInGroup*> lList = lIter.value();
        QList<CGexFileInGroup*>::iterator lIterList(lList.begin()), lIterListEnd(lList.end());
        for(; lIterList != lIterListEnd; ++lIterList)
        {

            CGexFileInGroup* lpFile = *lIterList;

            if (lpFile->bIgnoreThisSubLot)
                continue;

            // Update global equipment object
            lpFile->m_pGlobalEquipmentID->m_nHeadNum = 255;
            lpFile->m_pGlobalEquipmentID->m_nSiteNum = 255;
            *lpFile->m_pGlobalEquipmentID += mSDRRecord.mSiteDescription;

            // Update individual sites
            for(int nSite=0; nSite<mSDRRecord.mSiteCount; ++nSite)
            {
                mSDRRecord.mSiteDescription.m_nHeadNum = mSDRRecord.mHead;
                mSDRRecord.mSiteDescription.m_nSiteNum = mSDRRecord.mSiteNumbers[nSite];
                lpFile->m_pSiteEquipmentIDMap->insert(mSDRRecord.mSiteDescription.m_nSiteNum,
                                                      mSDRRecord.mSiteDescription);
            }
        }
    }
}

//!-----------------------------------!//
//!        initFileInGroupFTR         !//
//!-----------------------------------!//
void StdfDataFile::initFileInGroupFTR(CGexFileInGroup *pFile, int lPass)
{
    bool 	bValideResult	= false;
    bool	bFailResult		= false;
    CTest	*ptTestCell		= NULL;			// Pointer to test cell to receive STDF info.
    unsigned uMapIndex;
    float	fResult;

    CTestResult::PassFailStatus	ePassFailStatus	= CTestResult::statusUndefined;

    // Check if FTR processing is disabled
    if(pFile->m_eIgnoreFunctionalTests == CGexFileInGroup::FTR_DISABLED)
        return;

    // Ignore FTR if filtering set to 'Bin data only'
    if(pFile->iProcessBins == GEX_PROCESSPART_NO_SAMPLES)
        return;

    switch(mRecordHeader.iStdfVersion)
    {
    default :
            break;

        case GEX_STDFV4:

            // Check if current FTR (functional test result) belongs to a part that we filter ?
            // If this part has a binning the User doesn't want to process, then
            // we skip this record...
            if(lPass == 2 && pFile->PartProcessed.IsFilteredPart(pFile->pReportOptions, mFTRRecord.mSite, false, pFile->iProcessBins) == true)
                return;

            // Ignore test if we are in 'Stop on Fail' mode, and current site already failed!
            if((lPass == 2) && (pFile->m_eFailCountMode == CGexFileInGroup::FAILCOUNT_FIRST) && (pFile->m_map_ptFailTestCell.contains(mFTRRecord.mSite)))
                return;

//            if((pFile->iProcessSite >=0) && (pFile->iProcessSite != (int)mFTRRecord.mSite))
//                break;	// Site filter not matching.

            // Check if this FTR is within a valid PIR/PRR block.
            // Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR
            uMapIndex = (mFTRRecord.mHead << 16) | mFTRRecord.mSite;
            if((pFile->m_eStdfCompliancy==CGexFileInGroup::STRINGENT) && pFile->lMappingSiteHead[uMapIndex] != 1)
                break;

            // Check if this test is marked 'fail' (bits 6 cleared, bit 7 set) ?
            // Check if Pass/Fail indication is valid (bit 6 cleared)
            if ((mFTRRecord.mTestFlag & 0x40) == 0x00)
            {
                // If bit 7 set, then test is fail, otherwise test is pass
                ePassFailStatus = (mFTRRecord.mTestFlag & 0x80) ? CTestResult::statusFail : CTestResult::statusPass;
                bFailResult		= (ePassFailStatus == CTestResult::statusFail) ? true : false;
            }

            // Check if test result is valid
            if((mFTRRecord.mTestFlag & 0100) || (mFTRRecord.mTestFlag & 16))
                bValideResult = false;
            else
                bValideResult = true;

            // Ignore test result if must compute statistics from summary only
            if(pFile->m_eStatsComputation == CGexFileInGroup::FROM_SUMMARY_ONLY)
                bValideResult = false;

            QString strVectorName	= mFTRRecord.mVectorName;
            QString strTestName		= mFTRRecord.mTestTxt;

            if(strVectorName.isNull() || strVectorName.isEmpty())
                strVectorName = "-";		// Force a name if none defined.

            // Formats test name (in case to long, or leading spaces)
            pFile->FormatTestName(strTestName);

            // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
            if(pFile->FindTestCell(mFTRRecord.mTestNumber, GEX_FTEST, &ptTestCell, true, true, strTestName.toLatin1().data()) !=1)
                return;	// Error

            if(strTestName.isEmpty() == false && (ptTestCell->strTestName.isEmpty() == true))
                ptTestCell->strTestName = strTestName;

            // Test is Functionnal
            GEX_ASSERT(ptTestCell->bTestType == 'F');

            if (bValideResult)
            {
                ptTestCell->bTestExecuted = true;

                // Update tmp buffer sample count: number of time this test is executed in a run.
                ptTestCell->ldTmpSamplesExecs++;
            }

            if((bValideResult == true) && (lPass == 2))
            {
                // PASS2:
                // Keep track of failing count (unless it's an outlier)
                if(bFailResult)
                {
                    // Update failing count
                    ptTestCell->GetCurrentLimitItem()->ldSampleFails++;
                    // Keep failure history.
                    pFile->m_bFailingSiteDetails[mFTRRecord.mSite] |= GEX_RUN_FAIL_TEST;
                    // If failure, make sure we mark it.
                    pFile->CountTrackFailingTestInfo(mFTRRecord.mSite, ptTestCell, GEX_FTEST, 0);
                }

                if(bFailResult)
                    fResult = 0.0;	// Functionnal Failure
                else
                    fResult = 1.0;	// Functionnal PAss.

                // Save vector info
                CFunctionalTest cVectorResult;
                if(ptTestCell->mVectors.find(strVectorName) == ptTestCell->mVectors.end())
                {
                    // First failing vector in this test
                    cVectorResult.lExecs = 1;
                    cVectorResult.lFails = (bFailResult) ? 1: 0;
                    cVectorResult.strVectorName = strVectorName;
                }
                else
                {
                    // This vector already in list, then increment Exec count & update fail count
                    cVectorResult = ptTestCell->mVectors[strVectorName];
                    // Increment exec count
                    cVectorResult.lExecs++;
                    // Update fail count
                    if(bFailResult)
                        cVectorResult.lFails++;
                }

                GexVectorFailureInfo vectorFailure(mFTRRecord.mCycleCount, mFTRRecord.mRelativeVecAdress);

                // Add vector information on pin failed
                if(pFile->pReportOptions->getAdvancedReport() == GEX_ADV_DATALOG
                   || pFile->pReportOptions->getAdvancedReport() == GEX_ADV_FUNCTIONAL)
                {
                    if (bFailResult && mFTRRecord.mNumberPinFailure > 0)
                    {
                        uint		uiFailPinCount	= 0;
                        char		cBitField		= 0;
                        CPinmap *	pPinmap			= NULL;

                        // Looking for every pin failed
                        for (uint nIndex = 0; nIndex < mFTRRecord.mFailingPin.m_uiLength  && uiFailPinCount < mFTRRecord.mNumberPinFailure; nIndex++)
                        {
                            cBitField = mFTRRecord.mFailingPin.m_pBitField[nIndex];
                            if(!cBitField) continue;

                            for (int nBit = 0; nBit < 8; nBit++)
                            {
                                if (cBitField & 0x01)
                                {
                                    if (pFile->FindPinmapCell(&pPinmap, (nIndex*8) + nBit))
                                        vectorFailure.addPinmap(pPinmap);
                                    uiFailPinCount++;
                                }
                                cBitField = cBitField >> 1;
                            }
                        }
                        cVectorResult.m_lstVectorInfo.append(vectorFailure);
                    }
                }

                // Save updated vector info
                ptTestCell->mVectors[strVectorName] = cVectorResult;

                // Advanced report is Enabled, and test in filter limits.
                switch(pFile->pReportOptions->getAdvancedReport())
                {
                    case GEX_ADV_DISABLED:
                    default:
                        break;

                    case GEX_ADV_DATALOG:

                        if (GexAbstractDatalog::existInstance())
                        {
                            if (GexAbstractDatalog::isDataloggable(ptTestCell, bFailResult, false))
                            {

                                int nRunIndex = pFile->PartProcessed.runIndexFromSite(mFTRRecord.mSite);
                                if (nRunIndex >= 0 && nRunIndex < pFile->pPartInfoList.count())
                                    GexAbstractDatalog::pushFunctionalResult(
                                                pFile, ptTestCell, pFile->pPartInfoList.at(nRunIndex),
                                                fResult, bFailResult, strVectorName, vectorFailure, mFTRRecord.mSite);
                            }
                        }
                        else
                            pFile->AdvCollectDatalog(ptTestCell,
                                                     fResult,
                                                     bFailResult,
                                                     false,
                                                     false,
                                                     mFTRRecord.mSite,
                                                     mFTRRecord.mHead, mFTRRecord.mSite);
                        break;
                }

                // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                {
                    if(pFile->pReportOptions->bSpeedCollectSamples)
                    {
                        if (pFile->AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, mFTRRecord.mSite, true) == false)
                            return;
                    }
                }

            }

            break;
    }
}


//!-----------------------------------!//
//!        processDTR                 !//
//!-----------------------------------!//
void StdfDataFile::processDTR(int lPass)
{

    mDTRRecord.reset();
    mDTRRecord.readDTR(mStdfFile);

    //-- Loop over all pFile
    FileInGroupContainerIter lIter(mFileInGroupsBySite.begin()), lIterEnd(mFileInGroupsBySite.end());
    for(;lIter != lIterEnd;++lIter)
    {
        QList<CGexFileInGroup*> lList = lIter.value();
        QList<CGexFileInGroup*>::iterator lIterList(lList.begin()), lIterListEnd(lList.end());
        for(; lIterList != lIterListEnd; ++lIterList)
        {
            if ((*lIterList)->bIgnoreThisSubLot)
                continue;

            (*lIterList)->initPtrFromParentGroup();
            initFileInGroupDTR(*lIterList, lPass);
            (*lIterList)->initPtrParentGroup();
        }
    }
}

//!-----------------------------------!//
//!        initFileInGroupDTR         !//
//!-----------------------------------!//
void StdfDataFile::initFileInGroupDTR(CGexFileInGroup *pFile, int lPass)
{
    bool lIsDatalogString=true, lIsGexScriptingCommand=false, lIsFilteredPart;
    CTest	*lTestCell = NULL;	// Pointer to test cell to receive STDF info.

    // process reticle infos if any
    if (pFile->ProcessReticleInformationsIn( mDTRRecord.mDTR_V4 ) == false)
        return;


    QString lCommand  = "ML"; // to search Multi-Limit object
    QJsonObject mJson = mDTRRecord.mDTR_V4.GetGsJson(lCommand);

    if(!mJson.isEmpty() && lPass == 1) // if multi limit JSon oject found!
    {
        QString lError;
        GS::Core::MultiLimitItem* lLimitItem = new GS::Core::MultiLimitItem();
        if (!lLimitItem->LoadFromJSon(mJson, lError) || !lLimitItem->IsValid())
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error when reading DTR record: %1 - %2")
                  .arg(mDTRRecord.mDTR_V4.m_cnTEXT_DAT)
                  .arg(lError)
                  .toLatin1().constData());
            return;
        }
        if (!lLimitItem->IsValidLowLimit() && !lLimitItem->IsValidHighLimit())
        {
            GSLOG(SYSLOG_SEV_NOTICE, QString("No Valid limit in DTR: %1")
                  .arg(mDTRRecord.mDTR_V4.m_cnTEXT_DAT)
                  .toLatin1().constData());
            return;
        }

        int lTestNumber = GS::Core::MultiLimitItem::ExtractTestNumber(mJson);
        QString lTestName = GS::Core::MultiLimitItem::ExtractTestName(mJson);

        // if filter on site, check site value
        if (pFile->iProcessSite != -1)
        {
            int lSite = GS::Core::MultiLimitItem::ExtractSiteNumber(mJson);
            // if special site specified in DTR, make sure it matches the selected site
            // if not, do not store the limit
            if ((lSite != -1) && (pFile->iProcessSite != lSite))
            {
                return;
            }
        }

        if (lTestNumber >= 0)
        {
            // find the right test
            if(pFile->FindTestCell(lTestNumber, GEX_PTEST, &lTestCell, true, false, lTestName) != 1)
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Unable to find matching test for %1")
                      .arg(mDTRRecord.mDTR_V4.m_cnTEXT_DAT)
                      .toLatin1().constData());
                return;
            }

            if (lTestCell == NULL)
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Invalid test found for %1")
                      .arg(mDTRRecord.mDTR_V4.m_cnTEXT_DAT)
                      .toLatin1().constData());
                return;
            }
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Invalid test number found for %1")
                  .arg(mDTRRecord.mDTR_V4.m_cnTEXT_DAT)
                  .toLatin1().constData());
            return;
        }


    }

    // Change \n to \r so HTML output (if report type) doesn't include empty lines.
    QString lString = mDTRRecord.mDTR_V4.m_cnTEXT_DAT;
    lString.replace('\n','\r');

    //************************************************
    // Check if scripting line
    // Format: <cmd> set variable = value
    // Ember should modify their test programs to use fommowing command instead:
    lIsGexScriptingCommand = (lString.toLower().startsWith("<cmd>", Qt::CaseInsensitive));
    if(!lIsGexScriptingCommand)
        lIsGexScriptingCommand = (lString.toLower().indexOf("<cmd> gross_die") >= 0);
    if(!lIsGexScriptingCommand)
        lIsGexScriptingCommand = (lString.toLower().indexOf("<cmd> grossdiecount") >= 0);

    // Check if partID filtered.
    lIsFilteredPart = pFile->PartProcessed.IsFilteredPart(pFile->pReportOptions, 0, false, pFile->iProcessBins);

    if(lPass != 2)
        lIsDatalogString = false;	// Datalog is only during pass2!

    // Do not datalog DTR in advanced mode
    if(pFile->hAdvancedReport == NULL || GexAbstractDatalog::existInstance())
        lIsDatalogString = false;	// No file to output the DTR string.

    // Check if Datalog is disabled
    if(pFile->pReportOptions->getAdvancedReport() != GEX_ADV_DATALOG)
        lIsDatalogString = false;

    // Check if datalog comments are activated for display
    if(!(pFile->m_eDatalogTableOptions & CGexFileInGroup::ADF_COMMENTS)	)
        lIsDatalogString = false;	// no!

    // If raw data, do not include DTR strings in datalog
    if(pFile->pReportOptions->getAdvancedReportSettings() == GEX_ADV_DATALOG_RAWDATA)
        lIsDatalogString = false;


    // If Datalog enabled, and part not filtered...
    if(lIsDatalogString && !lIsFilteredPart)
    {
        // If line is too long, write it over few lines...or remove many spaces if possible...
        if (
            (pFile->m_OutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
            &&
            (lString.length() >= 100) )
        {
            // First: remove convert double spaces to single space.
            lString = lString.simplified();
        }

        // First test datalogged in part, and not RAW datalog.
        if(pFile->lTestsInDatalog == 0)
            pFile->WriteDatalogPartHeader();

        if(pFile->m_OutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            // Generating .CSV report file.

            // Datalog current part in the .CSV file.
            if(pFile->lTestsInDatalog % 2)
            {
                // Test in in second column...so tab until on next line
                fprintf(pFile->hAdvancedReport,"\n");
                pFile->lTestsInDatalog++;	// Tells we are now on a even count.
            }
            if(!lString.isEmpty())
                fprintf(pFile->hAdvancedReport,"%s\n",lString.toLatin1().constData());
        }
        else if (pFile->m_OutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        {
            // Datalog current part in the HTML datalog file.
            if(pFile->lTestsInDatalog % 2)
            {
                // Test in in second column...so tab until on next line
                fprintf(pFile->hAdvancedReport,"</tr>\n");
                pFile->lTestsInDatalog++;	// Tells we are now on a even count.
            }
            fprintf(pFile->hAdvancedReport,"</table>\n");

            // Write text in green (comment)
            if(!lString.isEmpty())
            {
                // Check if color control given in the string!
                if(lString.startsWith("<font_cmd>", Qt::CaseInsensitive))
                {
                    // Extract rgb string. line format: '<font_cmd> #XXYYZZ line_to_print
                    lString = lString.trimmed();
                    QString lLine = lString.mid(11);
                    QString lColor = lLine.mid(0,7);
                    lLine = lString.mid(18);

                    fprintf(pFile->hAdvancedReport,
                            "<font color=\"%s\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%s</font>\n",
                            lColor.toLatin1().constData(),lLine.toLatin1().constData());
                }
                else
                    fprintf(pFile->hAdvancedReport,"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%s\n",
                            lString.toLatin1().constData());
            }
            fprintf(pFile->hAdvancedReport,"<table border=\"0\" width=\"98%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
        }
    }

    if(!lIsGexScriptingCommand)
    {
        // GCORE-2133
        if (lString.startsWith("/*GalaxySemiconductor*/") || lString.startsWith("/*GalaxySemiconductorJS*/") )
        {
            QVariant lConcatenatedJSDTR = pFile->property(pFile->sConcatenatedJavaScriptDTRs.toLatin1().data());
            if (lConcatenatedJSDTR.isNull())
            {
                pFile->setProperty(pFile->sConcatenatedJavaScriptDTRs.toLatin1().data(), QVariant(lString));
            }
            else
            {
                pFile->setProperty(pFile->sConcatenatedJavaScriptDTRs.toLatin1().data(), QVariant(lConcatenatedJSDTR.toString()+lString) );
            }
        }
        return;
    }

    // DTR Gex scripting commands
    long	lTestNumber=0;
    double	lfResult;

    // Parse command line (take string after the '<cmd>' prefix)
    QString lAction = lString.mid(6);
    //strAction = strAction.trimmed();
    QString lKeyword = lAction.section(' ',0,0);
    lKeyword = lKeyword.trimmed();

    // Check if PAT datalog string
    if(lKeyword.startsWith("logPAT", Qt::CaseInsensitive))
    {
        // Command line: '<cmd> logPAT <PAT datalog HTML string>'
        if(lPass == 1 && ReportOptions.getAdvancedReport() == GEX_ADV_PAT_TRACEABILITY)
        {
            // Only consider this string during pass1. Ignore'd during pass 2!
            lAction = lAction.mid(7);
            gexReport->strPatTraceabilityReport += lAction;
        }
        return;
    }

    int  lCommandIndex;
   // QString lCommand  ;

    // Check if gross_die command
    if (ReportOptions.
        GetOption("binning",
                  "total_parts_used_for_percentage_computation").toString() ==
        "gross_die_if_available")
    {
        bool bOK;
        int  nGrossDie;
        if (lKeyword.toLower().startsWith("gross_die", Qt::CaseInsensitive))
        {
            bool bOK;
            int  nGrossDie = lAction.section('=', 1, 1).trimmed().toInt(&bOK);
            if (bOK)
            {
                pFile->m_nGrossDieCount = nGrossDie;
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("grossdie set").toLatin1().constData());
                return;
            }
        }

        // Note: Vishay Eagle files have [0,15,"<cmd> GrossDieCount =8290",""]
        // instead of [<cmd> gross_die=8290]
        lCommand    = lString.trimmed().toLower();
        lCommandIndex = lCommand.indexOf("<cmd> gross_die");
        if (lCommandIndex >= 0)
        {
            // Parse command line
            lCommand = lCommand.mid(lCommandIndex);
            nGrossDie  =
                    lCommand.section(QRegExp("[=\"]"), 1, 1).trimmed().toInt(&bOK);
            if (bOK)
            {
                pFile->m_nGrossDieCount = nGrossDie;
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("grossdie set").toLatin1().constData());
                return;
            }
        }
        lCommandIndex = lCommand.indexOf("<cmd> grossdiecount");
        if (lCommandIndex >= 0)
        {
            // Parse command line
            lCommand = lCommand.mid(lCommandIndex);
            nGrossDie  =
                    lCommand.section(QRegExp("[=\"]"), 1, 1).trimmed().toInt(
                        &bOK);
            if (bOK)
            {
                 pFile->m_nGrossDieCount = nGrossDie;
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("grossdie set").toLatin1().constData());
                return;
            }
        }
    }

    // Ignore multi-die command
    lCommandIndex = lCommand.indexOf("<cmd> multi-die");
    if(lCommandIndex >= 0)
        return;

    // Ignore die-tracking command
    lCommandIndex = lCommand.indexOf("<cmd> die-tracking");
    if(lCommandIndex >= 0)
        return;

    // Extract <VariableName> & <TestNumber>
    lAction = lAction.section(' ',1);
    // E.g: string format: 'setValue <VariableName>.<TestNumber> = <value>'
    QString lParameter = lAction.section('.',0,0);
    QString lStrTestNumber = lAction.section('.',1);
    lStrTestNumber = lStrTestNumber.section('=',0,0);
    lStrTestNumber = lStrTestNumber.trimmed();
    bool	lIsValid;
    lTestNumber = lStrTestNumber.toLong(&lIsValid);
    if(!lIsValid)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Failed to extract test number...string parsed may be corrupted")
              .toLatin1().constData());
        return;
    }

    // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
    if(pFile->FindTestCell(lTestNumber,GEX_PTEST, &lTestCell,true,true, lParameter.toLatin1().data()) !=1)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error when searching test: %1 - %2")
              .arg(QString::number(lTestNumber))
              .arg(lParameter)
              .toLatin1().constData());
        return ;	// Error
    }

    // Set test type
    GEX_ASSERT(lTestCell->bTestType == 'P');

    if(lKeyword.startsWith("setString", Qt::CaseInsensitive))
    {
        // String variable
    }
    if(lKeyword.startsWith("setValue", Qt::CaseInsensitive))
    {
        // Parametric variable
        QString lValue = lAction.section('=',1,1);
        lValue = lValue.trimmed();
        lfResult = lValue.toDouble(&lIsValid);
        if(!lIsValid)
        {
            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("Failed to extract test result...string parsed may be corrupted")
                  .toLatin1().constData());
            return;
        }

        // During pass#1: keep track of Min/Max so histogram cells can be computed in pass#2.
        if(lPass == 1)
            pFile->UpdateMinMaxValues(lTestCell,lfResult);

        if(lPass == 2)
        {
            // Command must be after a PIR and before a PRR
            GEX_ASSERT(pFile->PartProcessed.PIRNestedLevel() != 0);

            // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
            if( pFile->pReportOptions->bSpeedCollectSamples)
            {
                if (pFile->AdvCollectTestResult(lTestCell, lfResult, CTestResult::statusUndefined, 0, true) == false)
                {
                    GSLOG(SYSLOG_SEV_ERROR, "Failed to collect test result");
                    return;
                }
            }

#if 0 // Herve Thomy : // OBSOLETE MEMBER Not used anymore,since we hold the runIndex in the CPartBinning class except with MASA plugins
            // Keeps tracking the test# result.
            ptTestCell->ldCurrentSample++;
#endif

            // Build Cpk
            lTestCell->lfSamplesTotal += lfResult;				// Sum of X
            lTestCell->lfSamplesTotalSquare += (lfResult*lfResult);// Sum of X*X


            lTestCell->mHistogramData.AddValue(lfResult);
            if((lfResult >= lTestCell->GetCurrentLimitItem()->lfHistogramMin) && (lfResult <= lTestCell->GetCurrentLimitItem()->lfHistogramMax))
            {
                // Incluse result if it in into the viewing window.
                int lCell;
                if (lTestCell->GetCurrentLimitItem()->lfHistogramMax == lTestCell->GetCurrentLimitItem()->lfHistogramMin)
                {
                    lCell = 0;
                }
                else
                {
                    lCell = (int)( (TEST_HISTOSIZE*(lfResult - lTestCell->GetCurrentLimitItem()->lfHistogramMin)) / (lTestCell->GetCurrentLimitItem()->lfHistogramMax-lTestCell->GetCurrentLimitItem()->lfHistogramMin));
                }

                // Incluse result if it in into the viewing window.
                if( (lCell >= 0) && (lCell < TEST_HISTOSIZE) )
                    lTestCell->lHistogram[lCell]++;
                else if (lCell >= TEST_HISTOSIZE)
                    lTestCell->lHistogram[TEST_HISTOSIZE-1]++;   /// TO REVIEW : is it normal that iCell can exceed than TEST_HISTOSIZE
                else
                {
                    GSLOG(SYSLOG_SEV_ERROR,
                             QString("Invalid value detected to build histogram for test %1 [%2]")
                             .arg(lTestCell->lTestNumber)
                             .arg(lTestCell->strTestName).toLatin1().constData());

                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tCell Found: %1").arg(lCell).toLatin1().constData());
                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tDTR result: %1").arg( lfResult).toLatin1().constData());
                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Min: %1")
                          .arg(lTestCell->GetCurrentLimitItem()->lfHistogramMin).toLatin1().constData());
                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Max: %1").arg(
                             lTestCell->GetCurrentLimitItem()->lfHistogramMax).toLatin1().constData());
                }
            }
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Unsupported command line: %1")
              .arg(lKeyword)
              .toLatin1().constData());
    }

}


//!-------------------------------!//
//!         scanFile              !//
//!-------------------------------!//
//! This method has to be kept until eahc record has been passed
//! in the new algo. Afterward, the loop won't be usefull anymore since it is perfome dirrectly
//! on each record
bool StdfDataFile::processFile(int iPass)
{
    //-- scan the file
    if(readFile( iPass) == false)
        return false;

    //-- then call the end pas for all the associated fileInGroup
    FileInGroupContainerIter lIter(mFileInGroupsBySite.begin()), lIterEnd(mFileInGroupsBySite.end());
    for(;lIter != lIterEnd;++lIter)
    {
        QList<CGexFileInGroup*> lList = lIter.value();
        QList<CGexFileInGroup*>::iterator lIterList(lList.begin()), lIterListEnd(lList.end());
        for(; lIterList != lIterListEnd; ++lIterList)
        {
             (*lIterList)->EndPass(&(*lIterList)->getParentGroup()->cMergedData);
        }
    }
    return true;
}

//!-------------------------------!//
//!         scanFile              !//
//!-------------------------------!//
//! // WHEN ALL the READ WILL PUT HERE THE fileInGroup WON4T BE NECESSARY ANYMORE
//! // THE CALLING LOOP NEITHER
bool StdfDataFile::readFile(int iPass)
{
    // Update process bar
   // GS::Gex::Engine::GetInstance().UpdateProgressStatus(false, -1, -1);  // Increment to next step

    // File size:
    QFileInfo cFielInfoDebug(mFileName);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("File : %1").arg( mFileName).toLatin1().constData());
    if (cFielInfoDebug.exists() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("file %1 does not exist").arg( mFileName).toLatin1().constData());
    }

    // Read all file
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    while (1)
    {
        //TOCHECK
        // Check every 0.5 second if abort clicked...allows to refresh GUI too!
        if (elapsedTimer.elapsed() > 500)
        {
            GS::Gex::Engine::GetInstance().UpdateProgressStatus(false, -1, -1);  // Increment to next step
            QString lMessage = QString("Scan file : pass %1, reading record %2, %3Mo of test "
                                       "results allocated...").arg(iPass).arg(mRecordIndex)
                               .arg(CTestResult::GetTotalAllocated()/(1024*1024));
            GS::Gex::Engine::GetInstance().UpdateLabelStatus(lMessage);

            // At least 0.5 seconds since last check...so check again now!
            if (GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
            {
                GSLOG(SYSLOG_SEV_NOTICE, "Script aborted.");
                return true;
            }

            // Reset timer
            elapsedTimer.restart();
        }

        // Read one record from STDF file
        int iProcessStatus = mStdfFile.LoadRecord(&mRecordHeader);
        // Updates record counter
        mRecordIndex++;

        // If reached the end of file, perform end-of-pass then exit
        if (iProcessStatus != GS::StdLib::Stdf::NoError)
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Reading %1 STDF records...").arg(
                     QString::number(mRecordIndex)).toLatin1().constData());
            return true; // ?
        }

        // Process STDF record read
        switch (mRecordHeader.iRecordType)
        {
            case 0:
                switch (mRecordHeader.iRecordSubType)
                {
                    // Note: FAR (0:10) is analyzed in the lower layer (GS::StdLib::Stdf class)
                    // and is not visible from the STDF record analysis source code
                    // This is not an issue as this record is only needed for extracting
                    // data from the records (LSB/MSB issues)
                    case 20:  // Process ATR records... Type = 0:20
                        processATR(iPass);
                        break;
                }
                break;
            case 1:
                switch (mRecordHeader.iRecordSubType)
                {
                    case 10:  // Process MIR records... Type = 1:10
                        readMIR(iPass);
                        break;
                    case 20:  // Process MRR records... Type = 1:20
                        readMRR(iPass);
                        break;
                    case 30:  // Process PCR records... Type = 1:30
                        processPCR(iPass);
                        break;
                    case 40:  // Process HBR records... Type = 1:40
                        processBinning(iPass, true);
                        break;
                    case 50:  // Process SBR records... Type = 1:50
                        processBinning(iPass, false);
                        break;
                    case 60:  // Process PMR records... Type = 1:60
                        processPMR(iPass);
                        break;
                    case 62:  // Process PGR records... Type = 1:62
                        processPGR(iPass);
                        break;
                    case 63:  // Process PLR records... Type = 1:63
                        processPLR(iPass);
                        break;
                    case 80:  // Process SDR records... Type = 1:80
                        // If we have a SubLot filter activated, skip all these records!
                        processSDR(iPass);
                        break;
                }
                break;
            case 2:
                switch (mRecordHeader.iRecordSubType)
                {
                    case 10:  // Process WIR records... Type = 2:10
                        processWIR(iPass);
                        break;
                    case 20:  // Process WRR records... Type = 2:20
                        // If we have a SubLot filter activated, skip all these records!
                        if (mIgnoreNextWaferRecord == false)
                        {
                            processWRR(iPass);
                        }
                        break;
                    case 30:  // Process WCR records... Type = 2:30
                        // If we have a SubLot filter activated, skip all these records!
                        if (mIgnoreNextWaferRecord == false)
                        {
                            processWCR(iPass);
                        }
                        break;
                }
                break;
            case 5:
                switch (mRecordHeader.iRecordSubType)
                {
                    case 10:  // Process PIR records... Type = 5:10
                        processPIR();
                        if(mPTRCount > 0)
                            mTestDef = false;
                        break;
                    case 20:  // Process PRR records... Type = 5:20
                         processPRR(iPass);
                        break;
                }
                break;
            case 10:
                switch (mRecordHeader.iRecordSubType)
                {
//                    case 10:  // Process PDR records... Type = 10:10 ==== STDF V3 only.
//                        fileInGroup->ReadPDR();
//                        break;
//                    case 20:  // Process FDR records... Type = 10:20 ==== STDF V3 only.
//                        fileInGroup->ReadFDR();
//                        break;
                    case 30:  // Process TSR records... Type = 10:30
                        processTSR(iPass);;
                        break;
                }
                break;
            case 15:
                switch (mRecordHeader.iRecordSubType)
                {
                    case 10:   // Process PTR records... Type = 15:10
                        ++mPTRCount;
                        if(!processPTR(iPass, mTestDef))
                        {
                            QString lMessage = "error: ReadPTR failed";
                            GSLOG(SYSLOG_SEV_WARNING,
                                  lMessage.toLatin1().constData());
                            mStdfFile.Close();
                            return false;
                        }
                    break;

                    case 15:  // Process MPR records... Type = 15:15
                        processMPR(iPass);
                        break;
                    case 20:  // Process FTR records... Type = 15:20
                        processFTR(iPass);
                        break;
                }
                break;
            case 50:
                switch (mRecordHeader.iRecordSubType)
                {
                    case 10:  // Process GDR records... Type = 50:10
                        processGDR();
                        break;
                    case 30:  // Process DTR records... Type = 50:30
                        processDTR(iPass);
                        break;
                }
                break;
        }
    }

    return true;
}


void StdfDataFile::initPass()
{
    mTestDef = true;
    mPTRCount = 0;
    mCurrentIndexWaferMap = -1;
}
