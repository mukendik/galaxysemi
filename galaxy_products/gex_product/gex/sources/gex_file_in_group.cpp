/******************************************************************************!
 * \file gex_file_in_group.cpp
 ******************************************************************************/
#include <gqtl_log.h>

#include "cmerged_results.h"
#include "cpart_info.h"
#include "engine.h"
#include "gex_file_in_group.h"
#include "gex_test_creator.h"
#include <gqtl_global.h>
#include "gex_algorithms.h"
#include "gex_group_of_files.h"
#include "cbinning.h"
#include "report_options.h"
#include "import_all.h"
#include "auto_repair_stdf.h"
#include "browser_dialog.h"
#include "temporary_files_manager.h"
#include "gex_report.h"
#include "product_info.h"
#include "csl/csl_engine.h"
#include "message.h"
#include "plugin_base.h"
#include "stdf_data_file.h"

#include "gexperformancecounter.h"

extern GexMainwindow* pGexMainWindow;
extern CGexReport*    gexReport;  // Handle to report class
extern CReportOptions ReportOptions;
extern double ScalingPower(int iPower);

int TestSiteLimits::m_iLimitIndex = 0;

#define C_INFINITE_FLOAT  (float) 9e37

typedef int (CPartInfo::* fctComparePart)(const CPartInfo* pOther);

const QString CGexFileInGroup::sConcatenatedJavaScriptDTRs("JSDTRs");

/******************************************************************************!
 * \fn CGexFileInGroup
 * \brief This class holds information about a file
 ******************************************************************************/
CGexFileInGroup::
CGexFileInGroup(CGexGroupOfFiles* parent,
                int lGroup,
                QString FileName,
                int iSite, int iPartBin,
                const char* szRangeList,
                QString strFileMapTests,
                QString strWaferIdToExtract,
                double lfTemperature,
                QString strDataName,
                QString sampleGroup)
    : QObject(0),
      mStdfDataFileParent(0),
      m_eStdfCompliancy(FLEXIBLE),
      mTestMergeRule(TEST_NO_MERGE),
      mTestSortOrder(TEST_SORT_ORDER_BY_NUM),
      mUsedLimits(STANDART_LIMIT_ONLY)
{
    PartProcessed.setGroupOfFile(parent);

    // File to process (STDF, ATDF, GDF, etc.)
    strFileName = FileName;
    // STDF file to analyze (results from above file converted to STDF)
    strFileNameSTDF = FileName;
    // Holds the dataset name (if any defined). Can be used as a custom marker
    strDatasetName    = strDataName;
    mSampleGroup      = sampleGroup;
    strRangeList      = szRangeList;
    iProcessSite      = iSite;
    iProcessBins      = iPartBin;
    lGroupID          = lGroup;
    strMapTests       = strFileMapTests;
    strWaferToExtract = strWaferIdToExtract.trimmed();
    m_lfTemperature   = lfTemperature;
    m_ptCellFound     = NULL;    // Internally used in 'FindTestCell'
    m_ParentGroup     = parent;  // Handle to parent (CGexGroupOfFiles *)

    bool bAllowExtendedCsv = false;
    bool bAllowWizard      = false;

    // Construct site desription objects
    // (avoids to include plugin_base.h in report_build.h)
    m_pGlobalEquipmentID  = new GP_SiteDescription;
    m_pSiteEquipmentIDMap = new GP_SiteDescriptionMap;

    // cached options default values
    m_OutputFormat    = GEX_OPTION_OUTPUT_HTML;
    m_eBinComputation = SUMMARY;
    if (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
    {
        // Force to compute statistics from samples only
        m_eStatsComputation = FROM_SAMPLES_ONLY;
    }
    else
    {
        m_eStatsComputation = FROM_SAMPLES_THEN_SUMMARY;
    }
    m_eDatalogFormat   = ONE_ROW;
    m_eRetestPolicy    = LAST_BIN;
    m_eScalingMode     = SCALING_NONE;
    m_bFullWafermap    = false;
    m_bUsePassFailFlag = false;
    mPartIdentifiation = AUTO;
    mHistoBarsCount    = 40;
    mIndexWafermap     = -1;

    // Pointer to all report generation options
    pReportOptions = &ReportOptions;

    // Ensure we use latest options set
    UpdateOptions(pReportOptions);
    m_cStats.UpdateOptions(pReportOptions);

    bAllowWizard = true;

    // Check if data access type if FILES or DATABASE
    switch (GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:   // OEM-Examinator for LTXC
        case GS::LPPlugin::LicenseProvider::eSzOEM:     // Examinator: Only SZ data files allowed
            bAllowExtendedCsv = false;
            break;

         // DATABASE type
        case GS::LPPlugin::LicenseProvider::eYieldMan:
        case GS::LPPlugin::LicenseProvider::eYieldManEnterprise:
            // ExaminatorMonitoring
        case GS::LPPlugin::LicenseProvider::ePATMan:
        case GS::LPPlugin::LicenseProvider::ePATManEnterprise:
            // Galaxy PAT-Man
        case GS::LPPlugin::LicenseProvider::eExaminatorPAT:
            // ExaminatorDB: Database access, any file type
        case GS::LPPlugin::LicenseProvider::eExaminatorPro:
            bAllowExtendedCsv = true;
            break;
        default:
            break;
    }

    // Convert file to STDF if needed
    // (if not a STDF file. e.g: PCM, WAT, CSV, etc.)
    // unless Must use SUMMARY files only
    GS::Gex::ConvertToSTDF StdfConvert;
    QString strErrorMessage;
    bool    bFileCreated;
    int     nConvertStatus;

    // Convert file to STDF unless we have to use the Summary (ExaminatorDB only)
    if (QFile::exists(FileName))
    {
        if (ReportOptions.bSpeedUseSummaryDB == false)
        {
            // STDF file to be created...
            // append custom extension so we can erase it later on
            QStringList lstFileNameSTDF;
            nConvertStatus =
                    StdfConvert.Convert(false, bAllowWizard,
                                        false, bAllowExtendedCsv,
                                        FileName, lstFileNameSTDF,
                                        GEX_TEMPORARY_STDF, bFileCreated, strErrorMessage);
            if (lstFileNameSTDF.size() > 0)
                strFileNameSTDF = lstFileNameSTDF.first();
            if (nConvertStatus == GS::Gex::ConvertToSTDF::eConvertError)
            {
                CGexAutoRepairStdf gexAutoRepair;
                QString strRepairedFileName;

                switch (gexAutoRepair.repairStdf(strFileNameSTDF, strRepairedFileName))
                {
                    case CGexAutoRepairStdf::repairSuccessful:
                        strFileNameSTDF = strRepairedFileName;
                        break;
                    case CGexAutoRepairStdf::repairFailed:
                        GS::Gex::Message::information(
                            "", "Failed to repair the file " + strFileNameSTDF);
                        GS::Gex::CSLEngine::GetInstance().AbortScript(); // was pGexMainWindow->OnStop();
                        break;
                    case CGexAutoRepairStdf::repairCancelled:
                        GS::Gex::CSLEngine::GetInstance().AbortScript(); // was pGexMainWindow->OnStop();
                        break;
                    case CGexAutoRepairStdf::repairUnavailable:
                        GS::Gex::Message::information("", strErrorMessage);
                        GS::Gex::CSLEngine::GetInstance().AbortScript(); // was pGexMainWindow->OnStop();
                        break;
                    default:
                        GSLOG(SYSLOG_SEV_NOTICE, "unmanaged auto repair option...");
                        GS::Gex::CSLEngine::GetInstance().AbortScript(); //pGexMainWindow->OnStop();
                        break;
                }
            }
            if (nConvertStatus == GS::Gex::ConvertToSTDF::eConvertWarning)
            {
                GS::Gex::Message::information("", strErrorMessage);
            }

            // Update list of temporary STDF files created if needed.
            if (bFileCreated == true)
            {
                // For next file, have to add to FileList
                if (lstFileNameSTDF.count() > 1)
                {
                    strFileNameSTDF = lstFileNameSTDF.last();
                    // If have to keep the original name, delete this line
                    strFileName = lstFileNameSTDF.last();
                }
                GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strFileNameSTDF, TemporaryFile::BasicCheck);

                for (int i = 0; i < lstFileNameSTDF.count() - 1; i++)
                {
                    GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(lstFileNameSTDF[i], TemporaryFile::BasicCheck);
                    // Add file to group
                    if (gexReport != NULL)
                    {
                        gexReport->
                                addFile(lGroup, lstFileNameSTDF[i],
                                        iProcessSite,
                                        iProcessBins,
                                        szRangeList, strMapTests,
                                        strWaferToExtract, lfTemperature, strDatasetName,mSampleGroup);
                        // If have to keep the original name
                        // CGexFileInGroup *pFile = parent->pFilesList.at(iPos);
                        // if (pFile != NULL) pFile->strFileName = strFileName;
                    }
                }
            }
        }

    }
    // Creates list of structures that hold one range each.
    if (iProcessBins == GEX_PROCESSPART_PARTSINSIDE || iProcessBins ==
        GEX_PROCESSPART_PARTSOUTSIDE)
    {
        m_pFilterRangeCoord =
                CGexFilterRangeCoord::createFilterRangeCoord(iProcessBins,
                                                             QString(szRangeList));
        pGexRangeList = new GS::QtLib::Range(NULL);
    }
    else
    {
        m_pFilterRangeCoord = NULL;
        pGexRangeList       = new GS::QtLib::Range(szRangeList);
    }

    PartProcessed.setRangeList(pGexRangeList);

    // Initialization
    //ptPinmapList = NULL;  // Pointing to list of Pinmap definitions
    ptTestList   = NULL;  // Pointing to test list.
    //mMapTestsByNumber.clear();

    lAverageTestTime_Good = 0;  // Counts total execution time of GOOD parts
    // Number of parts used to compute test time of GOOD parts
    lTestTimeParts_Good  = 0;
    lAverageTestTime_All = 0;   // Counts total execution time of ALL parts
    // Number of parts used to compute test time of ALL parts
    lTestTimeParts_All = 0;
    imapZoningTestValue.clear();  // Zonning value for each die tested.
    fmapZoningTestValue.clear();

    lTestsInDatalog = 0;     // Reset count of tests in a part datalog.
    lProcessDone    = 0;
    lRecordIndex    = 0;
    // After pass#1, holds number of parts dataloged (PRR found)
    // Used to decide if no sample available (in which case, use summary)
    ldTotalPartsSampled = 0;
    bBinningSummary     = false; // set to true in ReadSBR();
    bMergeSoftBin       = false; // 'true' if need to merge all SBR sites in Pass2
    bMergeHardBin       = false; // 'true' if need to merge all SBR sites in Pass2
    // Keeps track of # of SBR found (matching site# filter)
    iSoftBinRecords = 0;
    // Keeps track of # of HBR found (matching site# filter)
    iHardBinRecords = 0;
    // At startup, no filter on WaferID to process is set
    bIgnoreThisSubLot  = false;
    bFirstWafer        = true;   // First wafer after MIR
    strPreviousWaferID = "";     // Wafer ID of last wafer read (WIR)
    // Variable for auto-increment wafer ID's (if WaferID missing in STDF)
    uiAutoWaferID = 0;

    // Holds STDF tester type if information defined in a ATR record,
    // holds -1 otherwise
    iStdfAtrTesterBrand = -1;

    // Keeps track of last failing test in flow. Reset after each run (in PRR)
    m_mapFailTestNumber.clear();
    m_mapFailTestPinmap.clear();
    m_mapfFailingValue.clear();
    m_map_ptFailedTestGoodResult.clear();
    m_map_ptPassedTestBadResult.clear();

    // Used for Charting Binning Trend.
    lAdvBinningTrendTotalMatch = 0;  // Total Bins matching the Trend filter
    // Total Bins found (Yield = 100 * lAdvBinningTrendTotalMatch /
    // lAdvBinningTrendTotal)
    lAdvBinningTrendTotal = 0;

    // Init gross die count
    m_nGrossDieCount = 0;

    // Init number of samples for this sublot
    m_lSamplesInSublot = 0;

    // Read datatset config file if any
    m_datasetConfig.load(strMapTests);

    QStringList qslFormatTestName = (ReportOptions.
                                     GetOption(QString("dataprocessing"),
                                               QString("format_test_name"))).
                                    toString().split(QString("|"));
    m_bRemoveSeqName =
            qslFormatTestName.contains(QString("remove_sequencer_name"));
    m_bRemovePinName = qslFormatTestName.contains(QString("remove_pin_name"));
    mTestCorrespondance.clear();

    mOffsetResult = -1;
}


int CGexFileInGroup::getWIRCount() const {
  return m_iWRRCount;
}
/*!
 * \fn getWRRCount
 */
int CGexFileInGroup::getWRRCount() const {
  return m_iWRRCount;
}

/******************************************************************************!
 * \fn ~CGexFileInGroup
 * \brief Destructor : Closes file if not done,
 *        reset all private variables
 ******************************************************************************/
CGexFileInGroup::~CGexFileInGroup()
{
    // Destroy site desription objects
    delete m_pGlobalEquipmentID; m_pGlobalEquipmentID   = 0;
    delete m_pSiteEquipmentIDMap; m_pSiteEquipmentIDMap = 0;

    // Destroy lists
    /*CPinmap* ptPinmapCell, * ptNextPinmap;

    // Destroy Pinmap list
    ptPinmapCell = ptPinmapList;
    while (ptPinmapCell != NULL)
    {
        ptNextPinmap = ptPinmapCell->ptNextPinmap;
        delete ptPinmapCell;
        ptPinmapCell = ptNextPinmap;
    }

    ptPinmapList = NULL;*/

    // Delete wafermap array.
    if (getWaferMapData().getWafMap() != NULL)
    {
        delete[] getWaferMapData().getWafMap();
    }
    getWaferMapData().setWaferMap(NULL);

    // Test list ALREADY destroyed in the group of files (see ~CMergedResults())
    ptTestList = NULL;
    //mMapTestsByNumber.clear();

    // Destroy part info list
    while (! pPartInfoList.isEmpty())
    {
        delete pPartInfoList.takeFirst();
    }

    if (m_pFilterRangeCoord)
    {
        delete m_pFilterRangeCoord;
        m_pFilterRangeCoord = NULL;
    }

    if (pGexRangeList)
    {
        delete pGexRangeList;
        pGexRangeList = NULL;
    }
}

/******************************************************************************!
 * \fn isMatchingWafer
 * \brief Checks if a WaferID matches a filter
 *        return 'true' if true
 ******************************************************************************/
bool CGexFileInGroup::isMatchingWafer(QString strFilter, QString strWaferID)
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
    if (rx.exactMatch(strWaferID) == true)
    {
        return true;  // Yes: WaferID matches the filter
    }
    else
    {
        return false;
    }
}

// this function will determine the offset for the same file under the same group
int CGexFileInGroup::CalculOffsetIndexResult(const QString& fileName)
{
   /* if(mOffsetResult != -1)
        return mOffsetResult;

    if(lFileID == 0)
    {
        mOffsetResult = 0;
    }
    else
    {
        if(fileName == m_ParentGroup->pFilesList[lFileID - 1]->strFileName)
            mOffsetResult = m_ParentGroup->pFilesList[lFileID - 1]->ldTotalPartsSampled;
        else
            // for the file with different name, the offset will be set in the GroupOffset
            // during the end pass
            mOffsetResult = 0;


        mOffsetResult += m_ParentGroup->pFilesList[lFileID - 1]->CalculOffsetIndexResult(fileName);
    }

    return mOffsetResult;
    */

    if(mOffsetResult != -1)
        return mOffsetResult;

    //-- the offset is calculated for all the pFileInTheGroup associated with the same file
    int lOffsetResult = 0;
    QList<CGexFileInGroup*>::iterator lIter(m_ParentGroup->pFilesList.begin()), lIterEnd(m_ParentGroup->pFilesList.end());
    for( ; lIter != lIterEnd; ++lIter)
    {
        if(fileName == (*lIter)->strFileName)
        {
            if((*lIter)->mOffsetResult != -1)
            {
                break;
            }
            else
            {
                (*lIter)->mOffsetResult = lOffsetResult;
                lOffsetResult += (*lIter)->ldTotalPartsSampled;
            }
        }
    }

    return mOffsetResult;

}

/******************************************************************************!
 * \fn fillUnfilteredPartsList
 * \brief Extacts the list of parts unfiltered
 ******************************************************************************/
void CGexFileInGroup::fillUnfilteredPartsList(QList<long>& lstUnfilteredParts)
{
    if (m_pFilterRangeCoord ||
        iProcessBins == GEX_PROCESSPART_FIRSTINSTANCE ||
        iProcessBins == GEX_PROCESSPART_LASTINSTANCE ||
        m_datasetConfig.partFilter().isActivated())
    {
        // Find the X,Y die coordinates for this part...
        CTest* pTestCellDieX = NULL;
        CTest* pTestCellDieY = NULL;
        CTest* pTestCellPartFilter = NULL;

        int lR=FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &pTestCellDieX, true, false);
        if (lR != 1)
        {
            if (lR==-1)
            {
                GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
                return;
            }
            return;
        }

        if (FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &pTestCellDieY,
                         true, false) != 1)
        {
            return;
        }

        // Get the test for the part filtering
        if (m_datasetConfig.partFilter().isActivated())
        {
            FindTestCell(m_datasetConfig.partFilter().partFilterPrivate()->testNumber(),
                         GEX_PTEST, &pTestCellPartFilter,
                         true, false);
        }

        QMap<QString, long>::Iterator it;
        for (it  = m_First_InstanceDies.begin();
             it != m_First_InstanceDies.end(); ++it)
        {
            m_First_Instance.append(*it);
        }
        for (it = m_Last_InstanceDies.begin();
             it != m_Last_InstanceDies.end();
             ++it)
        {
            m_Last_Instance.append(*it);
        }

        int    nBegin;
        int    nEnd;
        double dPartValue;

        // Find begin and end offset for this sublot
        pTestCellDieX->findSublotOffset(nBegin, nEnd, lFileID);

        for (long lSample = nBegin; lSample < nEnd; lSample++)
        {
            if (m_pFilterRangeCoord &&
                m_pFilterRangeCoord->
                isFilteredCoordinate((int) pTestCellDieX->
                                     m_testResult.resultAt(lSample),
                                     (int) pTestCellDieY->
                                     m_testResult.resultAt(lSample)) == false)
            {
                lstUnfilteredParts.append(lSample);
            }
            else if (iProcessBins == GEX_PROCESSPART_FIRSTINSTANCE &&
                     m_First_Instance.contains(lSample) == false)
            {
                lstUnfilteredParts.append(lSample);
            }
            else if (iProcessBins == GEX_PROCESSPART_LASTINSTANCE &&
                     m_Last_Instance.contains(lSample) == false)
            {
                lstUnfilteredParts.append(lSample);
            }
            else if (pTestCellPartFilter)
            {
                if (pTestCellPartFilter->m_testResult.isValidResultAt(lSample))
                {
                    dPartValue = pTestCellPartFilter->m_testResult.resultAt(lSample);
                    if (m_eScalingMode == SCALING_NONE ||
                        m_eScalingMode == SCALING_SMART)
                    {
                        dPartValue *= ScalingPower(pTestCellPartFilter->res_scal);
                    }
                    else if (m_eScalingMode == SCALING_TO_LIMITS)
                    {
                        dPartValue *= ScalingPower(pTestCellPartFilter->hlm_scal);
                    }

                    if (m_datasetConfig.partFilter().isFiltered(dPartValue) == false)
                    {
                        lstUnfilteredParts.append(lSample);
                    }
                }
            }
        }
    }
}

/******************************************************************************!
 * \fn applyTestFilter
 * \brief Apply filter defined in file setup
 ******************************************************************************/
void CGexFileInGroup::applyTestFilter(CMergedResults* pMergedResults)
{
    ptTestList = pMergedResults->ptMergedTestList;

    CTest* pPrevTest = NULL;
    CTest* pCurrent  = ptTestList;
    CTest* pTmpTest  = NULL;

    while (pCurrent)
    {
        if (m_datasetConfig.testFilter().isFiltered(pCurrent->lTestNumber) == false)
        {
            if (pPrevTest)
            {
                pPrevTest->SetNextTest(pCurrent->GetNextTest());
            }
            else
            {
                ptTestList = pCurrent->GetNextTest();
            }

            // temporary storage
            pTmpTest = pCurrent->GetNextTest();

            // delete unused testcell and update the list
            unsigned int lTestNumber = pCurrent->GetTestNumber().toUInt();
            GetCTests().remove(lTestNumber);
            QString lFormattedTestName = pCurrent->GetTestName();
            formatSequencerAndPinTestName(lFormattedTestName);
            delete pCurrent;

            // set current test
            pCurrent = pTmpTest;
        }
        else
        {
            pPrevTest = pCurrent;
            pCurrent  = pCurrent->GetNextTest();
        }
    }

    pMergedResults->ptMergedTestList = ptTestList;
}

/******************************************************************************!
 * \fn orderRun
 ******************************************************************************/
bool CGexFileInGroup::orderRun(const QString& lOptionsSublotOrder)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
             QString("Ordering sublots by %1")
             .arg(lOptionsSublotOrder).toLatin1().constData());

    fctComparePart pCompareMethod;

    if (lOptionsSublotOrder == "xy")
    {
        pCompareMethod = &CPartInfo::compareXY;
    }
    else if (lOptionsSublotOrder == "partid")
    {
        pCompareMethod = &CPartInfo::comparePartID;
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR,
                 QString("Unknown sublot ordering option: %s")
                 .arg(lOptionsSublotOrder).toLatin1().constData());
        return false;
    }

    if (pPartInfoList.isEmpty() == false)
    {
        int nFileBeginRunOffset;
        int nFileEndRunOffset;

        // Retrieve begin offset in the test result for this file
        ptTestList->findSublotOffset(nFileBeginRunOffset,
                                     nFileEndRunOffset,
                                     lFileID);

        // Combsort algorithm found on http://en.wikipedia.org/wiki/Combsort
        //
        // function combsort(array input)
        //  gap := input.size //initialize gap size
        //
        //  loop until gap = 1 and swaps = 0
        //    //update the gap value for a next comb. Below is an example
        //    gap := int(gap / 1.247330950103979)
        //    if gap < 1
        //      //minimum gap is 1
        //      gap := 1
        //    end if
        //
        //    i := 0
        //    swaps := 0 //see bubblesort for an explanation
        //
        //    //a single "comb" over the input list
        //    loop until i + gap >= input.size //see shellsort for similar idea
        //      if input[i] > input[i+gap]
        //        swap(input[i], input[i+gap])
        //        swaps := 1 // Flag a swap has occurred, so the
        //               // list is not guaranteed sorted
        //      end if
        //      i := i + 1
        //    end loop
        //
        //  end loop
        // end function

        float shrink_factor = 1.247330950103979;
        int   gap           = pPartInfoList.size();
        int   swapped       = 1;
        int   i;

        while ((gap > 1) || swapped)
        {
            if (gap > 1)
            {
                gap = static_cast<int>(gap / shrink_factor);
            }

            swapped = 0;
            i = 0;

            while ((gap + i) < pPartInfoList.size())
            {
                if ((pPartInfoList.at(i)->*pCompareMethod)(pPartInfoList.at(i + gap)) >
                    0)
                {
                    pPartInfoList.swap(i, i + gap);
                    CTest* pTmp = ptTestList;

                    while (pTmp)
                    {
                        if (pTmp->m_testResult.count() > 0)
                        {
                            pTmp->m_testResult.swapResults(nFileBeginRunOffset, i, i + gap);
                        }

                        pTmp = pTmp->GetNextTest();
                    }
                    swapped = 1;
                }
                ++i;
            }
        }

        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "No parts to order");
    }

    return false;
}

int CGexFileInGroup::getMRRCount() const
{
    return mStdfDataFileParent->mMRRCount  /*m_iMRRCount*/;
}

/******************************************************************************!
 * \fn applyTestCreation
 * \brief Apply list of tests creation
 ******************************************************************************/
void CGexFileInGroup::applyTestCreation(CTest **ptMergedTestList)
{
    GexTestToCreate* testToCreate = NULL;
    GexTestCreator   testCreator(this);
    for (int i = 0; i < this->m_datasetConfig.testToCreateList().m_lstTestToCreate.size(); ++i)
    {
        testToCreate = this->m_datasetConfig.testToCreateList().m_lstTestToCreate.at(i);
        if (!testToCreate)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Invalid test to create at index %1")
                  .arg(i).toLatin1().constData());
            continue;
        }
        // If one of tests in formula are an MPR Test, create as many tests as the number of pin in test
        testCreator.setFormula(testToCreate->formula());
        QList<CTest*> testInFormula = testCreator.GetTestsFromFormula();
        int pinCount;
        int testMaxPin = 0;
        if (testInFormula.size() > 0)
        {
            pinCount = testInFormula[0]->getPinCount();
            for( int i=1; i < testInFormula.size(); ++i)
            {
                // If the test is MPR and haven't the same pin cout => return and don't create the new test
                // If the first test is not an MPR => pinCount=0
                if ( pinCount != testInFormula[i]->getPinCount()
                     && testInFormula[i]->getPinCount() != 0
                     && pinCount != 0)
                {
                    // Message d'erreur
                    QString errorMessage("the tests that you are using in this xml mapping file are both MPR test and have different pin counts therefore the new test cannot be created.");
                    gexReport->GetReportLogList().addReportLog(errorMessage, GS::Gex::ReportLog::ReportError);
                    return ;
                }
                // The first elements are not MPR
                else if (pinCount == 0)
                {
                    pinCount = testInFormula[i]->getPinCount();
                    testMaxPin = i;
                }
            }

            // the list of tests in formula has at least one MPR test
            if (pinCount > 0)
            {
                for(int pin=0; pin<testInFormula[testMaxPin]->getPinCount(); ++pin)
                {
                    // If succes to create or update test
                    if (testCreator.createTestInFileGroup(testToCreate, ptMergedTestList, pin))
                    {
                        testCreator.applyFormula(this->lFileID, testInFormula, pin);
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_ERROR, QString("Unable to create or update test: %1 #%2")
                              .arg(testToCreate->name())
                              .arg(testToCreate->number())
                              .toLatin1().constData());
                    }
                }
            }
            else
            {
                // Check if the test already exists in the merged list (Ex: attached the same file to 2 files in the merge files)
                // If succes to create or update test
                if (testCreator.createTestInFileGroup(testToCreate, ptMergedTestList))
                    testCreator.applyFormula(this->lFileID, testInFormula);
                else
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Unable to create or update test: %1 #%2")
                          .arg(testToCreate->name())
                          .arg(testToCreate->number())
                          .toLatin1().constData());
                }
            }
        }
    }
}

/******************************************************************************!
 * \fn operator<
 * \brief Less than operator, used to sort files within a group
 ******************************************************************************/
bool CGexFileInGroup::operator<(const CGexFileInGroup& other) const
{
    if (getMirDatas().lStartT == other.getMirDatas().lStartT)
    {
        int nCompare = qstricmp(getMirDatas().szLot, other.getMirDatas().szLot);

        if (nCompare == 0)
        {
            nCompare =
                    algorithms::gexCompareIntegerString(getMirDatas().szSubLot,
                                                        other.getMirDatas().szSubLot);

            if (nCompare == 0)
            {
                nCompare =
                        algorithms::gexCompareIntegerString(getWaferMapData().szWaferID,
                                                            getWaferMapData().szWaferID);
            }
        }

        return nCompare < 0;
    }
    else
    {
        return getMirDatas().lStartT < other.getMirDatas().lStartT;
    }
}

/******************************************************************************!
 * \fn GetWafermapYieldInfo
 * \brief Return PASS/Fail info over a wafermap
 *        If strBinList is empty: checks for P/F bins,
 *        otherwise focus on binList
 *        iMatch: number of bins matching criteria (strBinList or 'Pass' bins)
 *        iUnmatch: number of bins not-matching criteria
 *        (not in strBinList or 'Fail' bins)
 ******************************************************************************/
void
CGexFileInGroup::GetWafermapYieldInfo(bool    bSoftBin,
                                      int&    iMatch,
                                      int&    iUnmatch,
                                      QString strBinList  /*=""*/)
{
    // If no specific binlist given, focus is on Pass/Fail bins
    if (strBinList.isEmpty())
    {
        // Get handle to group
        CGexGroupOfFiles* pGroup =
                (lGroupID < 0 || lGroupID >= gexReport->getGroupsList().size()) ? NULL :
                                                                                  gexReport->getGroupsList().at(lGroupID);

        CBinning*      pBinning;   // Used to scan list of binning
        if (bSoftBin)
        {
            pBinning = pGroup->cMergedData.ptMergedSoftBinList;
        }
        else
        {
            pBinning = pGroup->cMergedData.ptMergedHardBinList;
        }

        // Scan Bin list and identify list of Good and Fail bins
        while (pBinning)
        {
            if (pBinning->cPassFail == 'P')
            {
                strBinList += QString::number(pBinning->iBinValue) + ",";
            }

            pBinning = pBinning->ptNextBin;
        }


        // If no Pass/Fail info available, then assume
        // Pass = Bin1, Fail = other bins
        if (strBinList.isEmpty())
        {
            strBinList = "1";
        }
    }

    // Create Range object for fast bin classification
    GS::QtLib::Range* ptBinRange = new GS::QtLib::Range(strBinList.toLatin1().constData());

    // Clear variables
    iMatch   = 0;
    iUnmatch = 0;

    // Scan all the wafermap buffer, see how many time bin 'iBin' is found.
    int iBinCode;
    int iWaferSizeX = getWaferMapData().SizeX;
    int iWaferSizeY = getWaferMapData().SizeY;
    int iLine, iCol;
    for (iLine = 0; iLine < iWaferSizeY; iLine++)
    {
        // Processing a wafer line
        for (iCol = 0; iCol < iWaferSizeX; iCol++)
        {
            iBinCode =
                    getWaferMapData().getWafMap()[(iCol + (iLine * iWaferSizeX))].getBin();
            switch (iBinCode)
            {
                case GEX_WAFMAP_EMPTY_CELL:  // -1: Die not tested
                    break;

                default:  // valid bin
                    if (ptBinRange->Contains(iBinCode))
                    {
                        iMatch++;
                    }
                    else
                    {
                        iUnmatch++;
                    }
                    break;
            }
        }
    }

    // Free memory
    delete ptBinRange; ptBinRange = 0;
}

/******************************************************************************!
 * \fn GetWafermapBinOccurance
 * \brief Return bin occurance in wafermap
 ******************************************************************************/
int CGexFileInGroup::GetWafermapBinOccurance(int iBin)
{
    int iBinCount = 0;

    // Scan all the wafermap buffer, see how many time bin 'iBin' is found.
    int iBinCode;
    int iWaferSizeX = getWaferMapData().SizeX;
    int iWaferSizeY = getWaferMapData().SizeY;
    int iLine, iCol;
    for (iLine = 0; iLine < iWaferSizeY; iLine++)
    {
        // Processing a wafer line.
        for (iCol = 0; iCol < iWaferSizeX; iCol++)
        {
            iBinCode =
                    getWaferMapData().getWafMap()[(iCol + (iLine * iWaferSizeX))].getBin();
            switch (iBinCode)
            {
                case GEX_WAFMAP_EMPTY_CELL:  // -1: Die not tested
                    break;

                default:  // valid bin
                    if (iBinCode == iBin)
                    {
                        iBinCount++;
                    }
                    break;
            }
        }
    }

    // Return count
    return iBinCount;
}

/******************************************************************************!
 * \fn findRunNumber
 * \brief Find the run number associated with the die coord
 ******************************************************************************/
long
CGexFileInGroup::findRunNumber(int    nXDie,
                               int    nYDie,
                               CTest* pTestCell,
                               int    iStartFrom  /*=0*/)
{
    long lRunNumber = -1;

    // Get handle to Die X,Y info
    CTest* ptDieX = NULL;
    CTest* ptDieY = NULL;

    if (FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &ptDieX, true, false) == 1 &&
        FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &ptDieY, true, false) == 1)
    {
        int nStartOffset = 0;
        int nEndOffset   = 0;

        // Find begin and end offset for this sublot
        pTestCell->findSublotOffset(nStartOffset, nEndOffset, lFileID);
        if ((iStartFrom > nStartOffset) &&
            (iStartFrom <= nEndOffset))
        {
            nStartOffset = iStartFrom;
        }

        for (long lPartId = nStartOffset; lPartId < nEndOffset; lPartId++)
        {
            if ((nXDie == (int) ptDieX->m_testResult.resultAt(lPartId)) &&
                (nYDie == (int) ptDieY->m_testResult.resultAt(lPartId)))
            {
                // Found a die location matching our selection
                lRunNumber = lPartId;
            }
        }
    }

    return lRunNumber;
}

/******************************************************************************!
 * \fn onPrepareSecondPass
 * \brief Called before the second pass
 ******************************************************************************/
void CGexFileInGroup::onPrepareSecondPass()
{
    bIgnoreThisSubLot = false;
}


void CGexFileInGroup::Init(CReportOptions* ptReportOptions,
                           CMergedResults* ptMergedData,
                           FILE*           hAdvFile,
                           int iPass)
{
        setProperty(sConcatenatedJavaScriptDTRs.toLatin1().data(), QVariant());
    // Updates Parts/Bin Filter
        //if(iPass == 1)
        //{
            pReportOptions = ptReportOptions;
            //pReportOptions = new CReportOptions(*ptReportOptions);
            // If GO_DIAGS...make sure ONLY BIN1 parts are processed, ignore Bad parts!
            if (pReportOptions->getAdvancedReport() == GEX_ADV_GO_DIAGNOSTICS)
            {
                iProcessBins = GEX_PROCESSPART_GOOD;
            }

            if (pReportOptions->pGexRangeList)
                delete pReportOptions->pGexRangeList;

            pReportOptions->pGexRangeList = new GS::QtLib::Range(*(pGexRangeList));  // Fom/To list of bins/parts

            // Handle to HTML Advanced page if Adv repor tto be created
            hAdvancedReport = hAdvFile;
            // Pointing to list of Tests
            ptTestList = ptMergedData->ptMergedTestList;

            UpdateOptions(pReportOptions);
        //}

        lPass = iPass;


        // Do miscelaneous prep. work before each pass
        PreparePass();

        if (iPass == 1)
        {
            m_iPIRCount = 0;
            m_iPRRCount = 0;
            m_iMRRCount = 0;
            m_iInvalidPartFoundCount.clear();
            m_iWIRCount = 0;
            m_iWRRCount = 0;
            mMaxMultiLimitItems = 1;
        }

    PartProcessed.resetPart();
}

/******************************************************************************!
 * \fn PreparePass
 * \brief Prepare pass
 ******************************************************************************/
void CGexFileInGroup::PreparePass()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Preparing pass %1...").arg(lPass).toLatin1().data());

    // Erase some global variables used during file analysis
    // Pointer to last parameter failing in a test flow
    m_map_ptFailTestCell.clear();
    m_mapFailTestNumber.clear();
    m_mapFailTestPinmap.clear();
    m_mapfFailingValue.clear();
    m_map_ptFailedTestGoodResult.clear();
    m_map_ptPassedTestBadResult.clear();

    m_lSamplesInSublot = 0;  // Holds the number of samples for this sublot
    m_lCurrentInternalPartID = 0;  // Holds current internal part ID.
    m_mapPartID.clear();
    m_strPart_TXT.clear();

    // Check Wafermap orientation options (only for pass 2)
    if (lPass == 2)
    {
        QString strOptionPositiveX =
                ReportOptions.GetOption("wafer", "positive_x").toString();
        QString strOptionPositiveY =
                ReportOptions.GetOption("wafer", "positive_y").toString();

        // Check if Wafermap probing direction must be overloaded
        // (from global 'Options)
        if (strOptionPositiveX == "right")
        {
            getWaferMapData().SetPosXDirection(true);  // No mirror along X line
        }
        else if (strOptionPositiveX == "left")
        {
            getWaferMapData().SetPosXDirection(false);  // DO mirror along X line
        }
        else
        {
            // Keep direction as stated in the STDF file.
            if (toupper(getWaferMapData().cPos_X) == 'L')
            {
                getWaferMapData().SetPosXDirection(false);  // DO mirror along X line
            }
            else
            {
                getWaferMapData().SetPosXDirection(true);  // No mirror along X line
            }
        }

        if (strOptionPositiveY == "up")
        {
            getWaferMapData().SetPosYDirection(false);  // DO mirror along Y line
        }
        else if (strOptionPositiveY == "down")
        {
            getWaferMapData().SetPosYDirection(true);  // No mirror along Y line
        }
        else
        {
            // Keep direction as stated in the STDF file.
            if (toupper(getWaferMapData().cPos_Y) == 'U')
            {
                getWaferMapData().SetPosYDirection(false);  // DO mirror along Y line
            }
            else
            {
                getWaferMapData().SetPosYDirection(true);  // No mirror along Y line
            }
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Pass %1 prepared...").arg(lPass).toLatin1().data());
}

/******************************************************************************!
 * \fn EndPass
 ******************************************************************************/
void CGexFileInGroup::EndPass(CMergedResults* ptMergedData)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 grossDie")
          .arg( ptMergedData ? ptMergedData->grossDieCount() : 0).toLatin1().constData());

    // Clean close
    StdfFile.Close();

    // End of file reached, or error (we don't care!)...ready for next pass !
    iProcessStatus = GS::StdLib::Stdf::NoError;

    // Pass1 done
    if (lPass == 1)
    {
        lRecordIndex = 0;  // Resets record count, only between passes!

    }
    // Update parts count
    CTest* ptTestCell;
    long   lTotalDevices = 0;
    if (lPass == 1)
    {
        // Get handle to HardBin, so to know how many runs in dataset
        int lR=FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN, GEX_PTEST, &ptTestCell, true, false);
        if ( lR == 1)
        {
            lTotalDevices = ptTestCell->ldSamplesValidExecs;
        }
        else if (lR==-1)
        {
            GS::Gex::Engine::GetInstance().Exit(EXIT_FAILURE);
            return;
        }

        // Counts total execution time of GOOD parts
        ptMergedData->lMergedAverageTestTime_Good += lAverageTestTime_Good;
        // Number of parts used to compute test time of GOOD parts
        ptMergedData->lMergedTestTimeParts_Good += lTestTimeParts_Good;
        // Counts total execution time of ALL parts
        ptMergedData->lMergedAverageTestTime_All += lAverageTestTime_All;
        // Number of parts used to compute test time of ALL parts
        ptMergedData->lMergedTestTimeParts_All += lTestTimeParts_All;

        // Clean reset of Test structures in case STDF file not complete
        // (eg: missing last PRR)
        ptTestCell = ptTestList;
        while (ptTestCell != NULL)
        {
            // With aligned data, all tests have same buffer size!
            ptTestCell->ldSamplesExecs = lTotalDevices;

            // check if update fields or not !
            if (ptTestCell->bTestExecuted == true)
            {
                // This test was executed...if this part matches the filter
                ptTestCell->ldTmpSamplesExecs = 0;
                ptTestCell->GetCurrentLimitItem()->lfHistogramMin    = ptTestCell->lfTmpHistogramMin;
                ptTestCell->lfSamplesMin      = ptTestCell->lfTmpSamplesMin;
                ptTestCell->GetCurrentLimitItem()->lfHistogramMax    = ptTestCell->lfTmpHistogramMax;
                ptTestCell->lfSamplesMax      = ptTestCell->lfTmpSamplesMax;
                ptTestCell->mHistogramData    = GS::Gex::HistogramData(mHistoBarsCount,
                                                                       ptTestCell->GetCurrentLimitItem()->lfHistogramMin,
                                                                       ptTestCell->GetCurrentLimitItem()->lfHistogramMax) ;

                // Reset 'tested' flag.
                ptTestCell->bTestExecuted = false;
            }

            // Make sure scale factors are integer numbers
            // (eg: change 253 = -3, 250 = -6, etc.)
            ptTestCell->res_scal = NormalizeScaleFactor(ptTestCell->res_scal);
            ptTestCell->hlm_scal = NormalizeScaleFactor(ptTestCell->hlm_scal);
            ptTestCell->llm_scal = NormalizeScaleFactor(ptTestCell->llm_scal);

            // Do some process on test with Percent as units
            ptTestCell->uniformizePercentUnits();

            // Resets temporary samples buffer...
            // to make sure to restrat from clean situation
            ptTestCell = ptTestCell->GetNextTest();
        }

    }
    else
    {
        // Keep track of number of samples for each test in this given sub-lot
        SaveLotSamplesOffsets();

        // Update the offset of the run index for this group
        this->PartProcessed.updateGroupRunOffset(PartProcessed.runCount());
       // CPartBinning::updateGroupRunOffset(PartProcessed.runCount());
    }

    // If what-if enabled, check if What-if bins names definitions already exist
    // (if not, create them)
    if ((pReportOptions->getAdvancedReport() == GEX_ADV_GUARDBANDING) &&
        (lPass == 1))
    {
        int nWhatIfPassBin = pReportOptions->GetOption("adv_what_if", "pass_bin").toString().toInt();
        int nWhatIfFailBin = pReportOptions->GetOption("adv_what_if", "fail_bin").toString().toInt();
        int nWhatIfUnknownFailBin = pReportOptions->GetOption("adv_what_if", "unknown_fail_bin").toString().toInt();

        // What-if Pass Bin
        AddBinCell(&m_ParentGroup->cMergedData.ptMergedSoftBinList, -1, 32768, 32768,
                   nWhatIfPassBin, nWhatIfPassBin, 'P',
                   -1, true, true, "What-If Pass");  // Add Soft Bin inf to list
        AddBinCell(&m_ParentGroup->cMergedData.ptMergedHardBinList, -1, 32768, 32768,
                   nWhatIfPassBin, nWhatIfPassBin, 'P',
                   -1, true, true, "What-If Pass");  // Add Hard Bin inf to list

        // What-if Fail Bin
        char cPassType = 'F';
        if(pReportOptions->GetOption("adv_what_if", "fail_bin_is_pass").toString() == "true")
            cPassType = 'P';

        AddBinCell(&m_ParentGroup->cMergedData.ptMergedSoftBinList, -1, 32768, 32768,
                   nWhatIfFailBin, nWhatIfFailBin, cPassType,
                   -1, true, true, "What-If Fail");  // Add Soft Bin inf to list
        AddBinCell(&m_ParentGroup->cMergedData.ptMergedHardBinList, -1, 32768, 32768,
                   nWhatIfFailBin, nWhatIfFailBin, cPassType,
                   -1, true, true, "What-If Fail");  // Add Hard Bin inf to list

        AddBinCell(&m_ParentGroup->cMergedData.ptMergedSoftBinList, -1, 32768, 32768,
                   nWhatIfUnknownFailBin, nWhatIfUnknownFailBin, cPassType,
                   -1, true, true, "What-If Fail");  // Add Soft Bin inf to list
        AddBinCell(&m_ParentGroup->cMergedData.ptMergedHardBinList, -1, 32768, 32768,
                   nWhatIfUnknownFailBin, nWhatIfUnknownFailBin, cPassType,
                   -1, true, true, "What-If Fail");  // Add Hard Bin inf to list
    }

    if (! PartProcessed.bIgnorePartsFiltered)
    {
        // true if some parts excluded from analysis
        ptMergedData->bPartsFiltered = PartProcessed.bPartsFiltered;
    }

  /*  CGexGroupOfFiles* pGroup =
            (lGroupID < 0 || lGroupID >= gexReport->getGroupsList().size()) ? NULL :
                                                                              gexReport->getGroupsList().at(lGroupID);*/
    //pGroup->SetMapTestsByNumber(mMapTestsByNumber);
}

/******************************************************************************!
 * \fn NormalizeScaleFactor
 * \brief Format scale factor to be signed values instead of unsigned
 *        eg: 253 changes to -3, etc...(signed char)
 ******************************************************************************/
int CGexFileInGroup::NormalizeScaleFactor(int iPower)
{
    if (iPower & 0200)
    {
        return (-1 - (~iPower & 0177));
    }
    else
    {
        return iPower;
    }
}

/******************************************************************************!
 * \fn GetPartsFromPartID
 ******************************************************************************/
bool CGexFileInGroup::GetPartsFromPartID(QString partID, QList<CPartInfo*>& l)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("searching for part id '%1' (already %2)").arg(
             partID).arg(l.size()).toLatin1().constData());
    foreach(CPartInfo * pi, pPartInfoList)
    {
        if (pi)
        {
            if (pi->getPartID() == partID)
            {
                l.push_back(pi);
            }
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//FIXME
////////////////////////////////////////////////////////////////////////////////
char CGexFileInGroup::szBuffer[50];

/******************************************************************************!
 * \fn FormatTestLimit
 ******************************************************************************/
void CGexFileInGroup::FormatTestLimit(CTest* ptTestCell)
{
    // LL
    if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
    {
        FormatTestLimit(ptTestCell,
                        ptTestCell->GetCurrentLimitItem()->szLowL,
                        ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                        ptTestCell->llm_scal);
    }
    // HL
    if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
    {
        FormatTestLimit(ptTestCell,
                        ptTestCell->GetCurrentLimitItem()->szHighL,
                        ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                        ptTestCell->hlm_scal);
    }

    // L Spec
    if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
    {
        FormatTestLimit(ptTestCell,
                        ptTestCell->szLowSpecL,
                        ptTestCell->lfLowSpecLimit,
                        ptTestCell->llm_scal);
    }
    // H Spec
    if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
    {
        FormatTestLimit(ptTestCell,
                        ptTestCell->szHighSpecL,
                        ptTestCell->lfHighSpecLimit,
                        ptTestCell->hlm_scal);
    }
}

  const CMir& CGexFileInGroup::getMirDatas() const   { return /*&MirData;*/ mStdfDataFileParent->getMirDatas();}
  CMir& CGexFileInGroup::getMirDatas()   { return /*&MirData;*/ mStdfDataFileParent->getMirDatas();}
  CPcr& CGexFileInGroup::getPcrDatas()   { return mPCRData;}

/******************************************************************************!
 * \fn initDynamicTest
 * \brief Prepare a clear CTest structure ready to fill samples
 *        Note: Ctest struture must have been created calling 'FindTestCell(..)'
 ******************************************************************************/
bool
CGexFileInGroup::initDynamicTest(CTest* ptTestCell,
                                 double lfLowL,
                                 int    iLlScale,
                                 double lfHighL,
                                 int    iHlScale,
                                 int    iResScale)
{
    // Low & High limits exist
    ptTestCell->GetCurrentLimitItem()->bLimitFlag  = ~(CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL);
    ptTestCell->GetCurrentLimitItem()->lfLowLimit  = lfLowL;
    ptTestCell->llm_scal    = iLlScale;
    ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighL;
    ptTestCell->hlm_scal    = iHlScale;
    ptTestCell->res_scal    = iResScale;

    // Get handle to HBin test
    CTest* ptTestCell_HBin;
    if (FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN, GEX_PTEST, &ptTestCell_HBin, true, false) != 1)
    {
        return false;
    }

    // Allocate buffer size (matching same size as Binning buffer in this group,
    // as all data are aligned)
    ptTestCell->ldSamplesExecs      = ptTestCell_HBin->ldSamplesExecs;
    ptTestCell->ldSamplesValidExecs = 0;
    QString lRes=ptTestCell->m_testResult.
                 createResultTable(ptTestCell->ldSamplesExecs, false);
    if ( lRes.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Init dynamic test : failed to create result table: %1").arg( lRes).toLatin1().constData());
        return false;
    }
    // Save samples in this splitlot.
    ptTestCell->pSamplesInSublots.append(ptTestCell->ldSamplesExecs);

    // Build test limits strings
    FormatTestLimit(ptTestCell);

    // Build test# string
    BuildTestNumberString(ptTestCell);

    // Success
    return true;
}

/******************************************************************************!
 * \fn computeAllStatistics
 * \brief Recompute all statistics for the test
 ******************************************************************************/
void
CGexFileInGroup::computeAllStatistics(CTest* ptTestCell,
                                      bool   bPercentile,
                                      int    iHistogramType)
{
    m_cStats.UpdateOptions(gexReport->getReportOptions());
    m_cStats.ComputeLowLevelTestStatistics(ptTestCell,
                                           ScalingPower(ptTestCell->res_scal));
    m_cStats.ComputeBasicTestStatistics(ptTestCell, true);
    m_cStats.RebuildHistogramArray(ptTestCell, iHistogramType);
    m_cStats.ComputeAdvancedDataStatistics(ptTestCell, true, bPercentile);
}

/******************************************************************************!
 * \fn FormatTestLimit
 ******************************************************************************/
char*
CGexFileInGroup::FormatTestLimit(CTest* ptTestCell,
                                 char*  szBuffer,
                                 double lfValue,
                                 int    iPower,
                                 bool   bUseOutputFormat  /* = true */)
{
    char szTmpBuffer[512];

    // Rescale to better range. eg: 1500mV becomes 1.5V...
    // unless we have to keep limits normalized!
    if (m_eScalingMode != SCALING_NORMALIZED)
    {
        // Overload result-scale with HighLimit scale
        if (m_eScalingMode == SCALING_TO_LIMITS)
        {
            iPower = ptTestCell->hlm_scal;
        }

        // Scale limit
        lfValue *= GS_POW(10.0, iPower);

        switch (ptTestCell->szTestUnits[0])
        {
            case '%':
            case '\0':  // No unit defined
                break;
            default:
                // No adaptive re-scaling if must stick to the units
                if (m_eScalingMode == SCALING_TO_LIMITS)
                {
                    break;
                }

                // If value really too big (eg: +- INF!), do not bother changing format!
                if (lfValue >= C_INFINITE_FLOAT || lfValue <= -C_INFINITE_FLOAT)
                {
                    break;
                }
                // Rescale only on some types of units !
                while (fabs(lfValue) >= 1000)
                {
                    lfValue /= 1000;
                    iPower  -= 3;
                }
                while (lfValue && (fabs(lfValue) < 0.1))
                {
                    lfValue *= 1000;
                    iPower  += 3;
                }
                break;
        }
    }

    // To avoid Spreadsheet Editor to convert a limit to a Time/Date field,
    // make sure we display a floating point number
    double fractpart, intpart;
    fractpart = modf(lfValue, &intpart);
    if (fractpart == 0 && ! iPower && *ptTestCell->szTestUnits)
    {
        // Make sure we add one leading 0!
        sprintf(szTmpBuffer, "%.1f", lfValue);
        QString strFormatDouble =
                gexReport->getNumberFormat()->
                formatNumericValue(lfValue, true, QString(szTmpBuffer));
        sprintf(szTmpBuffer, "%s", strFormatDouble.toLatin1().constData());

    }
    else
    {
        sprintf(szTmpBuffer, "%g", lfValue);
        QString strFormatDouble =
                gexReport->getNumberFormat()->
                formatNumericValue(lfValue, true, QString(szTmpBuffer));
        sprintf(szTmpBuffer, "%s", strFormatDouble.toLatin1().constData());
    }

    // Case 7214: Add separator (',' for CSV, ' ' else) AFTER formatNumericValue()
    // Add units to sting.
    // Under Spreadsheet editor, ensures units are not in same column as data
    if (bUseOutputFormat && (m_OutputFormat == GEX_OPTION_OUTPUT_CSV))
    {
        strcat(szTmpBuffer, ",");
    }
    else
    {
        strcat(szTmpBuffer, " ");
    }

    // If we have to keep limits in normalized format, do not rescale!
    if (m_eScalingMode != SCALING_NORMALIZED)
    {
        switch (iPower)
        {
            case 0:
            default:
                break;
            case 253:  // for unsigned -3
            case -3:
                strcat(szTmpBuffer, "K");
                break;
            case 250:  // for unsigned -6
            case -6:
                strcat(szTmpBuffer, "M");
                break;
            case 247:  // for unsigned -9
            case -9:
                strcat(szTmpBuffer, "G");
                break;
            case 244:  // for unsigned -13
            case -12:
                strcat(szTmpBuffer, "T");
                break;
            case 2:
                if (ptTestCell->szTestUnits[0] != '%')
                {
                    strcat(szTmpBuffer, "%");
                }
                break;
            case 3:
            case 4:
                // Should never occur: ...but present because of bug in J750 STDF files
            case 5:
                // Should never occur: ...but present because of bug in J750 STDF files
                strcat(szTmpBuffer, "m");
                break;
            case 6:
            case 7:
                // Should never occur: ...but present because of bug in J750 STDF files
            case 8:
                // Should never occur: ...but present because of bug in J750 STDF files
                strcat(szTmpBuffer, "u");
                break;
            case 9:
                strcat(szTmpBuffer, "n");
                break;
            case 12:
                strcat(szTmpBuffer, "p");
                break;
            case 15:
                strcat(szTmpBuffer, "f");
                break;
        }
    }
    strcat(szTmpBuffer, ptTestCell->szTestUnits);
    // Make sure we do not overflw the reception buffer
    szTmpBuffer[GEX_LIMIT_LABEL - 1] = 0;
    strcpy(szBuffer, szTmpBuffer);
    return szBuffer;
}

/******************************************************************************!
 * \fn GetMaxTestnumber
 * \brief Return max test number in Test List
 ******************************************************************************/
int CGexFileInGroup::GetMaxTestnumber()
{
    CTest* ptTestCell;
    unsigned int nMaxTestNumber = 0;

    // Check if test list empty
    if (ptTestList == NULL)
    {
        return 0;
    }

    // Start from first test in list
    ptTestCell = ptTestList;
    ptPrevCell = NULL;

    // Check from the beginning of the list if can find the test
    while (ptTestCell != NULL)
    {
        if (ptTestCell->lTestNumber > nMaxTestNumber)
        {
            nMaxTestNumber = ptTestCell->lTestNumber;
        }

        ptTestCell = ptTestCell->GetNextTest();
    }

    return nMaxTestNumber;
}

/******************************************************************************!
 * \fn FindTestCellName
 * \brief Find Test Cell structure matching a given Test List
 ******************************************************************************/
int CGexFileInGroup::FindTestCellName(const QString &lName,
                                      CTest** ptTestCellFound)
{
    CTest* ptTestCell = ptTestList;
    *ptTestCellFound = NULL;

    if (lName.isNull() || lName.isEmpty())
        return 0;

    // Remove trailing spaces as this may lead to problems
    // when comparing test names!
    QString strName = lName.trimmed();
    QString strTestName;
    while (ptTestCell != NULL)
    {
        strTestName = ptTestCell->strTestName.trimmed();
        if (strTestName == strName)
        {
            *ptTestCellFound = ptTestCell;
            return 1;  // Test found, pointer to it returned
        }

        // Next cell
        ptTestCell = ptTestCell->GetNextTest();
    }

    return 0;

}


CGexFileInGroup::T_Container& CGexFileInGroup::GetCTests()
{
    return m_ParentGroup->GetMapTestsByNumber();

}


void CGexFileInGroup::initPtrFromParentGroup()
{
   // if(ptTestList == 0)
        ptTestList = m_ParentGroup->cMergedData.ptMergedTestList;
}

void CGexFileInGroup::initPtrParentGroup()
{
    //if(m_ParentGroup->cMergedData.ptMergedTestList == 0)
        m_ParentGroup->cMergedData.ptMergedTestList = ptTestList;
}

/******************************************************************************!
 * \fn FindTestCell : using the 1 std::multimap<int, CTest*>
 ******************************************************************************/
int
CGexFileInGroup::FindTestCell(unsigned int  lTestNumber,
                              int           lPinmapIndex,
                              CTest**       ptTestCellFound,
                              bool          bAllowMapping ,
                              bool          bCreateIfNew ,
                              const QString &lTestName,
                              bool          enablePinmapIndex)
{
    CTest*  ptNewCell   = 0;
    QString lFormattedTestName = lTestName;
    QString strStrippedTestName;
    bool    bTestNameMapped     = false;
    bool    lIsFormatedTNameEmpty = true;
    // Check if the test name need to be modified
    // to be aligned with format test name
    formatSequencerAndPinTestName(lFormattedTestName);

    // Clear pointer that keeps track of previous test.
    ptPrevCell = NULL;
    // In the case of searching test and don't know if it is an MPR or PTR test. Search from the beginning of the list
    if (enablePinmapIndex == false)
    {
        m_ptCellFound = NULL;
    }

    // Check if we have to mapp the test# of cells we create...
    if (m_datasetConfig.testMapping().isActivated())
    {
        // Get mapping test number (if it exists)
        // Check on stripped test name (whitespaces removed)
        strStrippedTestName = lFormattedTestName.trimmed();
        if (bAllowMapping == true)
        {
            if (m_datasetConfig.testMapping().isMappedNumber(lTestNumber))
            {
                lTestNumber = m_datasetConfig.testMapping().mapNumber(lTestNumber);
            }

            if (m_datasetConfig.testMapping().mapName(strStrippedTestName))
            {
                lFormattedTestName     = strStrippedTestName;
                bTestNameMapped = true;
            }
        }
    }

    lIsFormatedTNameEmpty = lFormattedTestName.isEmpty() ? 1 : 0;


    // If test# must be ignored
    if (mTestMergeRule == TEST_MERGE_NAME)
    {
        lTestNumber = gexReport->findMappedTestName((lPinmapIndex > 0) ? GEX_MPTEST : lPinmapIndex,
                                         lTestNumber,
                                         lFormattedTestName.toLatin1().data());
    }

    T_Container& mMapTestsByNumber  = GetCTests();
    QPair <T_Iter, T_Iter> lRange = mMapTestsByNumber.equal_range(lTestNumber);
    for (T_Iter lIt=lRange.first; lIt!=lRange.second; ++lIt)
    {
        CTest* lTestFound = lIt.value();
        if(
                lTestFound
                && (lPinmapIndex == lTestFound->lPinmapIndex)
                && (lIsFormatedTNameEmpty || (lTestFound->strTestName == lFormattedTestName) || (mTestMergeRule == TEST_MERGE_NUMBER))
        )
        {
            *ptTestCellFound = lTestFound;
            m_ptCellFound    = lTestFound;
            return 1;  // Test found, pointer to it returned
        }
    }

    // If we are here, means, not found => create a new test
    if (bCreateIfNew == false)
    {
        *ptTestCellFound = NULL;
        return 0;  // Test not in current list...
    }

    // Test not in list: insert in list
    try
    {
        m_ptCellFound = ptNewCell = new CTest();
    }
    catch(const std::bad_alloc &e)
    {
        printf("\nFindTestCell: bad_alloc. line %d of file %s.\n", __LINE__, __FILE__);
        //GSLOG(SYSLOG_SEV_ERROR, QString("Currently %1 instances of CTest")
          //    .arg(CTest::GetNumberOfInstances()).toLatin1().data() );
        //GSLOG(SYSLOG_SEV_ERROR, e.what() );
        return -1;
    }
    catch(...)
    {
        printf("\nFindTestCell: unknown exception\n");
        return -1; // ?
    }

    ptNewCell->bMergedTsrExists = false;
    ptNewCell->lTestNumber      = lTestNumber;
    ptNewCell->strTestName      = lFormattedTestName;
    ptNewCell->bTestNameMapped  = bTestNameMapped;
    ptNewCell->lPinmapIndex     = lPinmapIndex;

    if (lPinmapIndex >= 0)
    {
        ptNewCell->bTestType = 'M';
    }
    else
    {
        switch(lPinmapIndex)
        {
            case GEX_PTEST  :
                ptNewCell->bTestType = 'P';
                break;

            case GEX_FTEST  :
                ptNewCell->bTestType = 'F';
                break;

            case GEX_MPTEST :
                ptNewCell->bTestType = 'M';
                break;

            case GEX_UNKNOWNTEST:
                ptNewCell->bTestType = ' ';
                break;

            case GEX_INVALIDTEST:
                ptNewCell->bTestType = 'I';
                break;

            default         :
                ptNewCell->bTestType = 'I';
                //GEX_ASSERT(false);
                GSLOG(SYSLOG_SEV_ERROR, QString("Failed to create test# %1 with test name %2")
                      .arg(lTestNumber)
                      .arg(lFormattedTestName).toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG, QString("\tInvalid Pinmap Index %1").arg(lPinmapIndex).toLatin1().constData());
                break;
        }
    }
    // Default is no test array result, std parametric test
    ptNewCell->lResultArraySize = 0;

    // Update TestFlowID
    if (isGexTest(lTestNumber))
    {
        ptNewCell->lTestFlowID = -1;
    }
    else
    {
        m_ParentGroup->IncrementFlowId();
        ptNewCell->lTestFlowID = m_ParentGroup->GetFlowId();
    }

    T_Iter lItInsert;
    // To be the first element in the list
    if (mMapTestsByNumber.empty())
    {
        ptPrevCell = NULL;
        ptNewCell->SetNextTest(NULL);
        ptTestList  = ptNewCell;
        lItInsert = mMapTestsByNumber.begin();
    }
    else
    {
        T_Iter lLowerFoundIt = mMapTestsByNumber.lowerBound(lTestNumber);
        lItInsert = lLowerFoundIt;
        // to be also the first elt
        if (lLowerFoundIt == mMapTestsByNumber.begin())
        {
            ptPrevCell  = NULL;
            ptTestList  = ptNewCell;
            ptNewCell->SetNextTest(lLowerFoundIt.value());
            lLowerFoundIt.value()->SetPreviousTest(ptNewCell);
        }
        // to be in the middle or the end of the list
        else
        {
            if (lLowerFoundIt != mMapTestsByNumber.end())
            {
                ptPrevCell = lLowerFoundIt.value()->GetPreviousTest();

                // create link next "ptNewCell -> lLowerFoundIt.value()"
                ptNewCell->SetNextTest(lLowerFoundIt.value());


                // create link previous "ptNewCell <- lLowerFoundIt.value()"
                lLowerFoundIt.value()->SetPreviousTest(ptNewCell);

                // create link next "ptPrevCell -> ptNewCell"
                ptPrevCell->SetNextTest(ptNewCell);

            }
            else
            {
                ptPrevCell = mMapTestsByNumber.last();
                ptPrevCell->SetNextTest(ptNewCell);
                ptNewCell->SetNextTest(NULL);
            }
        }
    }

    if(ptPrevCell != 0)
        ptNewCell->SetPreviousTest(ptPrevCell);

    mMapTestsByNumber.insert(lItInsert, lTestNumber, ptNewCell);

    *ptTestCellFound = ptNewCell;

    return 1;  // Success
}


/******************************************************************************!
 * \fn CompareTestFlowID
 ******************************************************************************/
bool CompareTestFlowID(CTest* a, CTest* b)
{
    if (a->lTestFlowID == b->lTestFlowID)
    {
        return (a->lTestNumber < b->lTestNumber);
    }
    else
    {
        return (a->lTestFlowID < b->lTestFlowID);
    }
}

/******************************************************************************!
 * \fn PrevTest
 ******************************************************************************/
CTest* CGexFileInGroup::PrevTest(CTest* ref)
{
    // TEST_SORT_ORDER_BY_NUM
    if ( mTestSortOrder == TEST_SORT_ORDER_BY_NUM)
    {
        if(ptPrevCell == 0)
            ptPrevCell = ref->GetPreviousTest();

        if(ptPrevCell == 0)
        {
            T_Iter lPos = GetCTests().find(ref->GetTestNumber().toInt());
            if(lPos != GetCTests().begin())
            {
                --lPos;
                ptPrevCell = lPos.value();
                ref->SetPreviousTest(ptPrevCell);
            }
        }
        return ptPrevCell;
    }

    // TEST_SORT_ORDER_BY_FLOW
    if (ptTestList == NULL ||
        ptTestList->GetNextTest() == NULL)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Too few items");
        return NULL;
    }

    QList<CTest*> lst;
    CTest* cur = ptTestList;
    while (cur)
    {
        lst.push_back(cur);
        cur = cur->GetNextTest();
    }

    qSort(lst.begin(), lst.end(), CompareTestFlowID);

    int i = lst.indexOf(ref);
    if (i == -1)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Item not found");
        return NULL;
    }
    if (i > 0)
    {
        return lst[i - 1];
    }
    else
    {
        return NULL;
    }
}

/******************************************************************************!
 * \fn NextTest
 ******************************************************************************/
CTest* CGexFileInGroup::NextTest(CTest* ref) const
{
    // TEST_SORT_ORDER_BY_NUM
    if (mTestSortOrder == TEST_SORT_ORDER_BY_NUM)
    {
        return ref->GetNextTest();
    }

    // TEST_SORT_ORDER_BY_FLOW
    if (ptTestList == NULL ||
        ptTestList->GetNextTest() == NULL)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Too few items");
        return NULL;
    }

    QList<CTest*> lst;
    CTest* cur = ptTestList;
    while (cur)
    {
        lst.push_back(cur);
        cur = cur->GetNextTest();
    }

    qSort(lst.begin(), lst.end(), CompareTestFlowID);

    int i = lst.indexOf(ref);
    if (i == -1)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Item not found");
        return NULL;
    }
    if ((i + 1) < lst.size())
    {
        return lst[i + 1];
    }
    else
    {
        return NULL;
    }
}

/******************************************************************************!
 * \fn ChangeTestSortOrder
 ******************************************************************************/
void CGexFileInGroup::ChangeTestSortOrder()
{
    if (mTestSortOrder == TEST_SORT_ORDER_BY_NUM)
    {
        mTestSortOrder = TEST_SORT_ORDER_BY_FLOW;
    }
    else if (mTestSortOrder == TEST_SORT_ORDER_BY_FLOW)
    {
        mTestSortOrder = TEST_SORT_ORDER_BY_NUM;
    }
}

/******************************************************************************!
 * \fn AddBinCell
 * \brief Add BIN to BIN List
 *        bMode = false indicate that the funct will insert a soft bin
 ******************************************************************************/
void CGexFileInGroup::
AddBinCell(CBinning**  ptBinList,
           int         iSiteID,
           int         iDieX,
           int         iDieY,
           int         iSoftBinValue,
           int         iHardBinValue,
           BYTE        cPassFail,
           int         ldTotalCount,
           bool        bCumulate,
           bool        bReadingDataFiles,
           const char* szBinName,
           bool        bMode)
{
    CBinning* ptNewCell;
    CBinning* ptBinCell;
    CBinning* ptPrevBinCell;
    int iBinValue = ! bMode ? iSoftBinValue : iHardBinValue;
    if (ldTotalCount > 0)
    {
        // Check if currently scanning database (and not building report pages) &
        // binning must be computed from WaferMap data only
        /*QString strOptionStorageDevice =
       (pReportOptions->GetOption("binning","computation").toString());
       if ((strOptionStorageDevice == "wafer_map") &&
          (bReadingDataFiles==true))*/
        if ((m_eBinComputation == WAFER_MAP) && (bReadingDataFiles == true))
        {
            // As binning is computed later on, do not save count yet,
            // but at least create empt ystructure and save Binning name!
            ldTotalCount = 0;
            // Set 'Cumulate' so we do not alter any existing count!
            bCumulate = true;
        }
    }
    else
    {
        // Count <=0 , then simply create entry if it doesn't exist
        ldTotalCount = 0;
        // Set 'Cumulate' so we do not alter any existing count!
        bCumulate = true;
    }

    // If filter activated, and binning out of filter: ignore
    switch (iProcessBins /*pReportOptions->iProcessBins*/)
    {
        case GEX_PROCESSPART_ALL:  // Range= All bins
            break;
        case GEX_PROCESSPART_GOOD:  // Range= Good bins
            if (cPassFail != 'P')
            {
                return;  // Bin is OUT of range !
            }
            break;
        case GEX_PROCESSPART_FAIL:  // Range = All bins Pass
            if (cPassFail == 'P')
            {
                return;  // Bin is OUT of range !
            }
            break;
        case GEX_PROCESSPART_SBINLIST:  // Range of Soft-Bin is custom:
            if (pGexRangeList->Contains(iSoftBinValue) == false)
            {
                return;  // Bin is OUT of range !
            }
            break;
        case GEX_PROCESSPART_EXSBINLIST:  // All bins except given Range of Soft-Bins
            if (pGexRangeList->Contains(iSoftBinValue) == true)
            {
                return;  // This bin belongs to the exclusion list!
            }
            break;
        case GEX_PROCESSPART_HBINLIST:  // Range of Hard-Bin is custom
            if (pGexRangeList->Contains(iHardBinValue) == false)
            {
                return;  // Bin is OUT of range !
            }
            break;
        case GEX_PROCESSPART_EXHBINLIST:  // All bins except given Range of Hard-Bins
            if (pGexRangeList->Contains(iHardBinValue) == true)
            {
                return;  // This bin belongs to the exclusion list!
            }
            break;
        case GEX_PROCESSPART_PARTLIST:  // Part or parts range
            if (pGexRangeList->Contains(PartProcessed.partNumber(iSiteID)) == false)
            {
                return;  // Part# is out of range !
            }
            break;
        case GEX_PROCESSPART_EXPARTLIST:  // All parts except given parts range
            if (pGexRangeList->Contains(PartProcessed.partNumber(iSiteID)) == true)
            {
                return;  // This Part# belongs to the exclusion list !
            }
            break;
        case GEX_PROCESSPART_ODD:  // Odd parts only (1,3,5,...)
            break;
        case GEX_PROCESSPART_EVEN:  // Even parts only (2,4,6,...)
            break;
        case GEX_PROCESSPART_FIRSTINSTANCE:
        case GEX_PROCESSPART_LASTINSTANCE:
            // Ignore filter if NOT Wafer-Sort data
            if (iDieX == 32768 || iDieY == 32768)
            {
                break;
            }
            break;
        case GEX_PROCESSPART_NO_SAMPLES:  // Only consider bins: Ignore samples
            break;
    }

    if (isspace(cPassFail) || iscntrl(cPassFail) || ! isascii(cPassFail))
    {
        // Avoids to have space for empty fields...Spreadsheet editor may not like it
        cPassFail = '-';
    }
    if (*ptBinList == NULL)
    {
        // First Bin : list is currently empty
        ptNewCell = *ptBinList = new CBinning;
        ptNewCell->iBinValue    = ! bMode ? iSoftBinValue : iHardBinValue;
        ptNewCell->cPassFail    = cPassFail;
        ptNewCell->ldTotalCount = ldTotalCount;
        if (*szBinName)
        {
            ptNewCell->strBinName = szBinName;
        }
        ptNewCell->ptNextBin = NULL;
        return;  // Success
    }

    // Loop until test cell found, or end of list reached
    ptBinCell     = *ptBinList;
    ptPrevBinCell = NULL;
    while (ptBinCell != NULL)
    {
        if (ptBinCell->iBinValue > iBinValue)
        {
            break;
        }
        else if (ptBinCell->iBinValue == iBinValue)
        {
            // Keeps track of total binnings
            if (bCumulate == true)
            {
                // This is when multiple bin call for a same binning is done (like when
                // using wafermap data to recreate the binning table)
                ptBinCell->ldTotalCount += ldTotalCount;
            }
            else
            {
                ptBinCell->ldTotalCount = ldTotalCount;
            }

            if (ptBinCell->strBinName.isEmpty() && (*szBinName))
            {
                ptBinCell->strBinName = szBinName;  // Save BIN name if not already done
            }
            return;
        }
        ptPrevBinCell = ptBinCell;
        ptBinCell     = ptBinCell->ptNextBin;
    }

    // Bin not in list: insert in list
    ptNewCell = new CBinning;
    ptNewCell->ldTotalCount = ldTotalCount;
    ptNewCell->iBinValue    = ! bMode ? iSoftBinValue : iHardBinValue;;
    ptNewCell->cPassFail    = cPassFail;
    if (*szBinName)
    {
        ptNewCell->strBinName = szBinName;
    }
    ptNewCell->ptNextBin = NULL;

    if (ptPrevBinCell == NULL)
    {
        // This cell becomes head of list
        ptNewCell->ptNextBin = *ptBinList;
        *ptBinList           = ptNewCell;
    }
    else
    {
        // Insert cell in list
        ptPrevBinCell->ptNextBin = ptNewCell;
        ptNewCell->ptNextBin     = ptBinCell;
    }
}

/******************************************************************************!
 * \fn setBinningType
 ******************************************************************************/
void CGexFileInGroup::setBinningType(bool bSoftBin,
                                     int  iBinValue,
                                     bool bPassBin)
{
    CBinning* ptBinList;

    if (bSoftBin)
    {
        ptBinList = m_ParentGroup->cMergedData.ptMergedSoftBinList;
    }
    else
    {
        ptBinList = m_ParentGroup->cMergedData.ptMergedHardBinList;
    }

    while (ptBinList)
    {
        if (ptBinList->iBinValue == iBinValue)
        {
            // Binning found, set its pass/fail flag
            ptBinList->cPassFail = (bPassBin) ? 'P' : 'F';
            return;
        }

        // Binning entry not found, check next one
        ptBinList = ptBinList->ptNextBin;
    }
}

/******************************************************************************!
 * \fn CountTestAsFailure
 * \brief Check if test result in valid!
 *        return: true if count this Test as a Failure, false otherwise
 ******************************************************************************/
bool
CGexFileInGroup::CountTestAsFailure(int    iSiteID,
                                    CTest* ptTestCell,
                                    int    iPinmapIndex,
                                    float  fValue,
                                    bool   bTestIsFail)
{
    if (bTestIsFail)
    {
        // Keep track of last failing test info (for GEX-DRILL module)
        CountTrackFailingTestInfo(iSiteID, ptTestCell, iPinmapIndex, fValue);

        return true;  // Count test as a FAILURE!
    }
    return false;  // DO NOT count test as a Failure!
}

/******************************************************************************!
 * \fn UpdateMinMaxValues
 * \brief Compares value to current Min, Max, updates Min,Max,
 *        and Temporary SumX, sumX*X
 ******************************************************************************/
void CGexFileInGroup::UpdateMinMaxValues(CTest* ptTestCell, double fValue)
{
    // If part is to be processed (will know at PRR!), this test has new min/max
    ptTestCell->bTestExecuted = true;

    // Calculate new min/max, store it in Temporary field, then at PRR, if
    // part belongs to the binning/part count filter, temporary field will be
    // copied into real min/max fields.
    ptTestCell->lfTmpSamplesMin =
            gex_min(fValue, ptTestCell->lfTmpSamplesMin);
    ptTestCell->lfTmpSamplesMax =
            gex_max(fValue, ptTestCell->lfTmpSamplesMax);
    ptTestCell->lfTmpSamplesTotal += fValue;  // Sum of X (including outliers)
    // Sum of X*X (including outliers)
    ptTestCell->lfTmpSamplesTotalSquare += (fValue * fValue);

    // Histogram limits
    switch (pReportOptions->iHistogramType)
    {
        case GEX_HISTOGRAM_DISABLED:  // disabled
        case GEX_HISTOGRAM_OVERLIMITS:  // Histogram chart over test limits
        case GEX_HISTOGRAM_CUMULLIMITS:  // Cumul over test limits
            if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            {
                // Low limit exists
                ptTestCell->lfTmpHistogramMin = ptTestCell->GetCurrentLimitItem()->lfLowLimit;
            }
            else
            {
                // No low limit
                ptTestCell->lfTmpHistogramMin = ptTestCell->lfTmpSamplesMin;
            }

            // Check if high limit exists
            if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                // High limit exists
                ptTestCell->lfTmpHistogramMax = ptTestCell->GetCurrentLimitItem()->lfHighLimit;
            }
            else
            {
                // No high limit
                ptTestCell->lfTmpHistogramMax = ptTestCell->lfTmpSamplesMax;
            }
            break;

        case GEX_HISTOGRAM_OVERDATA:  // Histogram chart over data sample
        case GEX_HISTOGRAM_CUMULDATA:  // Cumul over data sample
            ptTestCell->lfTmpHistogramMin = ptTestCell->lfTmpSamplesMin;
            ptTestCell->lfTmpHistogramMax = ptTestCell->lfTmpSamplesMax;
            break;

        case GEX_HISTOGRAM_DATALIMITS:  // adaptive: show data & limits
            // Chart has to be done over maxi of both datapoints & limits
            if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            {
                ptTestCell->lfTmpHistogramMin =
                        gex_min(ptTestCell->lfTmpSamplesMin, ptTestCell->GetCurrentLimitItem()->lfLowLimit);
            }
            else
            {
                ptTestCell->lfTmpHistogramMin = ptTestCell->lfTmpSamplesMin;
            }
            if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                ptTestCell->lfTmpHistogramMax =
                        gex_max(ptTestCell->lfTmpSamplesMax, ptTestCell->GetCurrentLimitItem()->lfHighLimit);
            }
            else
            {
                ptTestCell->lfTmpHistogramMax = ptTestCell->lfTmpSamplesMax;
            }
            break;
    }

    // In case we have no TSR, min/max will come from PTR samples!
    ptTestCell->lfMin = gex_min(ptTestCell->lfMin, fValue);
    ptTestCell->lfMax = gex_max(ptTestCell->lfMax, fValue);
    // Update tmp buffer sample count:
    // number of time this test is executed in a run
    ptTestCell->ldTmpSamplesExecs++;
}

/******************************************************************************!
 * \fn formatSequencerAndPinTestName
 ******************************************************************************/
void CGexFileInGroup::formatSequencerAndPinTestName(QString& strTestName)
{
    QString strString;

    for (int iIdx = 0; iIdx < 2; iIdx++)
    {
        // Check if this is a Flex/J750 data file
        // (if so, we MAY need to remove leading pin# in test names!)
        if (m_bRemovePinName)
        {
            // If ends with pin#, remove it eg: xxxxx 11.a30
            strString = strTestName.section(' ', -1, -1);
            if (strString.isEmpty() == false)
            {

                QString lPinName = strString;
                QString lPinPattern = "(\\d+\\.[^\\d]+\\d+|(\\+|-)?\\d+)";
                QRegExp::PatternSyntax lPatternSyntax = QRegExp::RegExp;
                QRegExp  lRegExpPattern("", Qt::CaseInsensitive, lPatternSyntax);
                lRegExpPattern.setPattern(lPinPattern);

                // Check match
                bool bMatch = lRegExpPattern.exactMatch(lPinName);
                if(bMatch)
                {
                    // Drop this pin number !
                    strTestName.truncate(strTestName.length() - lPinName.length());
                    strTestName = strTestName.trimmed();
                }
            }
        }
        if (m_bRemoveSeqName && strTestName.contains("<>"))
        {
            strTestName = strTestName.section("<>", 0, 0).trimmed();
        }
    }

    // Trim test name to remove unnecessary spaces at beginning and end
    // (for QA) [BG 08/27/2012]
    strTestName = strTestName.trimmed();
}

/******************************************************************************!
 * \fn getWaferMapData
 ******************************************************************************/
CWaferMap& CGexFileInGroup::getWaferMapData()
{
    return WaferMapData; /*mStdfDataFileParent->getWaferMapData();*/
}

const CWaferMap& CGexFileInGroup::getWaferMapData() const
{
    return WaferMapData; /* mStdfDataFileParent->getWaferMapData();*/
}
