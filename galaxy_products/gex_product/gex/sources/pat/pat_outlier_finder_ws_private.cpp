#ifdef GCORE15334
#include "pat_outlier_finder_ws_private.h"
#include "pat_info.h"
#include "pat_definition.h"
#include "pat_gdbn_engine.h"
#include "pat_potatoclusterfinder.h"
#include "pat_rules.h"
#include "gex_report.h"
#include "pat_mv_rule.h"
#include "stats_engine.h"
#include "r_data.h"
#include "r_matrix.h"
#include "mv_outliers_finder.h"
#include "cpart_info.h"
#include "engine.h"
#include "wafermap_parametric.h"
#include "pat_reticle_engine.h"

#include "gqtl_log.h"


extern CGexReport * gexReport;

extern double       ScalingPower(int iPower);

namespace GS
{
namespace Gex
{

PATOutlierFinderWSPrivate::PATOutlierFinderWSPrivate()
    : PATOutlierFinderPrivate(), mCurrentSite(-1), mGroup(NULL), mFile(NULL), mTestSoftBin(NULL),
      mTestHardBin(NULL), mTestSite(NULL)
{
}

PATOutlierFinderWSPrivate::~PATOutlierFinderWSPrivate()
{
}

bool PATOutlierFinderWSPrivate::Init(CPatInfo *lContext)
{
    if (lContext == NULL)
        return false;

    mContext    = lContext;
    mPartFilter = new PATPartFilter();

    if (gexReport == NULL)
        return false;

    QString lCpkOption = ReportOptions.GetOption("statistics", "cp_cpk_computation").toString();

    if (lCpkOption == "percentile")
        mUsePercentileCPK = true;

    // Get pointer to first group & first file (we always have them exist)
    if (gexReport->getGroupsList().count() == 1)
    {
        mGroup = gexReport->getGroupsList().first();
        mFile  = (mGroup == NULL || mGroup->pFilesList.isEmpty()) ? NULL : mGroup->pFilesList.first();

        if (mFile)
        {
            if (mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN, GEX_PTEST,  &mTestSoftBin, true, false) != 1)
            {
                return false;
            }

            if (mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN, GEX_PTEST,  &mTestHardBin, true, false) != 1)
            {
                return false;
            }

            if (mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_TESTING_SITE, GEX_PTEST,  &mTestSite, true, false) != 1)
            {
                return false;
            }
        }
        else
            return false;
    }
    else
        return false;

    // Initialisation done successfully
    mInitialized = true;

    return mInitialized;
}

bool PATOutlierFinderWSPrivate:: FillPartFilter(PATPartFilter *lPartFilter, int lSite)
{
    if (lPartFilter)
    {
        // Remove any previous filter
        lPartFilter->RemoveAllFilters();

        switch(mContext->GetRecipeOptions().iPatLimitsFromBin)
        {
            case GEX_TPAT_BUILDLIMITS_ALLBINS:
                break;

            case GEX_TPAT_BUILDLIMITS_GOODSOFTBINS:
                lPartFilter->AddFilterElement(PATPartFilterElement(mTestSoftBin, PATPartFilterElement::InFilter,
                                                                   QtLib::Range(*mContext->GetRecipeOptions().pGoodSoftBinsList)));
                break;

            case GEX_TPAT_BUILDLIMITS_LISTSOFTBINS:
                lPartFilter->AddFilterElement(PATPartFilterElement(mTestSoftBin, PATPartFilterElement::InFilter,
                                                                   QtLib::Range(mContext->GetRecipeOptions().strPatLimitsFromBin)));
                break;

            case GEX_TPAT_BUILDLIMITS_EXCEPTSOFTBINS:
                lPartFilter->AddFilterElement(PATPartFilterElement(mTestSoftBin, PATPartFilterElement::OutFilter,
                                                                   QtLib::Range(mContext->GetRecipeOptions().strPatLimitsFromBin)));
                break;

            case GEX_TPAT_BUILDLIMITS_GOODHARDBINS:
                lPartFilter->AddFilterElement(PATPartFilterElement(mTestHardBin, PATPartFilterElement::InFilter,
                                                                   QtLib::Range(*mContext->GetRecipeOptions().pGoodHardBinsList)));
                break;

            case GEX_TPAT_BUILDLIMITS_LISTHARDBINS:
                lPartFilter->AddFilterElement(PATPartFilterElement(mTestHardBin, PATPartFilterElement::InFilter,
                                                                   QtLib::Range(mContext->GetRecipeOptions().strPatLimitsFromBin)));
                break;

            case GEX_TPAT_BUILDLIMITS_EXCEPTHARDBINS:
                lPartFilter->AddFilterElement(PATPartFilterElement(mTestHardBin, PATPartFilterElement::OutFilter,
                                                                   QtLib::Range(mContext->GetRecipeOptions().strPatLimitsFromBin)));
                break;
        }

        if (lSite >= 0)
        {
            lPartFilter->AddFilterElement(PATPartFilterElement(mTestSite, PATPartFilterElement::InFilter,
                                                               QtLib::Range(QString::number(lSite))));
        }

        // Sucessfully filled the part filter
        return true;
    }

    return false;
}

CTest *PATOutlierFinderWSPrivate::FindTestCell(unsigned long testNumber, long pinIndex, const QString& testName)
{
    CTest * lTestFound = NULL;

    switch(mContext->GetRecipeOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            // Find test based on Test#
            mFile->FindTestCell(testNumber, pinIndex, &lTestFound);
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            // Find test based on Test# & test name
            mFile->FindTestCell(testNumber, pinIndex, &lTestFound,
                                   true, false, testName);
            // If failed to find test while it was read in STDF file,
            // then probably we have trailing spaces that corrupt the compare function...
            if(lTestFound == NULL)
                mFile->FindTestCellName(testName, &lTestFound);
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            mFile->FindTestCellName(testName, &lTestFound);
            break;
    }

    return lTestFound;
}

bool PATOutlierFinderWSPrivate::AnalyzeWaferSurfaceGDBN(CWaferMap *lExternWafermap, bool lIgnoreYieldThreshold)
{
    PAT_PERF_BENCH

    // If rule disabled, return now!
    if(mContext->GetRecipeOptions().mIsGDBNEnabled == false)
        return true;

    QList <CGDBN_Rule>::iterator lIterGDBN;
    GS::QtLib::Range *  ptGoodBinList = NULL;
    CGDBN_Rule          lGdbnRule;
    PatGdbnEngine       gdbnEngine;

    for(lIterGDBN = mContext->GetRecipeOptions().mGDBNRules.begin(); lIterGDBN != mContext->GetRecipeOptions().mGDBNRules.end(); ++lIterGDBN)
    {
        // Handle to rule
        lGdbnRule = *lIterGDBN;

        // If rule disabled, find next one!
        if(lGdbnRule.mIsEnabled == false)
            continue;	// Move to next rule

        // Check if GDBN analysis over SoftBin or HardBin
        CWaferMap	* ptWafermap;
        // Get handle to relevant wafermap (STDF soft bin or STDF Hard bin or Prober map)
        switch(lGdbnRule.mWafermapSource % 3)
        {
            case GEX_PAT_WAFMAP_SRC_SOFTBIN:
                ptWafermap = &mContext->m_AllSitesMap_Sbin;
                ptGoodBinList = mContext->GetRecipeOptions().pGoodSoftBinsList;	// Checking over Good SOFT bins
                break;
            case GEX_PAT_WAFMAP_SRC_HARDBIN:
                ptWafermap = &mContext->m_AllSitesMap_Hbin;
                ptGoodBinList = mContext->GetRecipeOptions().pGoodHardBinsList;	// Checking over Good HARD bins
                break;
            case GEX_PAT_WAFMAP_SRC_PROBER:
                ptWafermap = &mContext->m_ProberMap;
                ptGoodBinList = mContext->GetRecipeOptions().pGoodHardBinsList;	// Checking over Good HARD bins
                break;
            default:
                // Unsupported flow: don't do anything!
                return false;
        }

        // If specific map defined by caller, use it instead!
        if(lExternWafermap != NULL)
            ptWafermap = lExternWafermap;

        // Check if this wafer (all sites merged) has a yield low enough to screen good bin in bad neighborhood.
        bool	bStandardBadNeighbors=false;
        long	lTotalGoodBins=0;
        long	lTotalBins = 0;
        double	lfYieldRate;
        int		i,iBin;

        // Compute wafer yield level...
        for(i=0;i<ptWafermap->SizeX*ptWafermap->SizeY;i++)
        {
            // Get die.
            iBin = ptWafermap->getWafMap()[i].getBin();
            if(iBin >= 0)
            {
                lTotalBins++;
                // Check if this bin belongs to our list of 'good bins'
                if(ptGoodBinList->Contains(iBin))
                    lTotalGoodBins++;	// Counts total Good Bins.
            }
        }

        // Compute Percentage for all groups merged (all testing sites)
        if(lTotalBins)
            lfYieldRate = 100.0 * ((double) lTotalGoodBins) / (double) lTotalBins;
        else
            lfYieldRate = 0;

        // If Yield threshold must be ignored...ignore it! (eg: if GDBN applied to Z-PAT composite map)
        if(lIgnoreYieldThreshold)
            lfYieldRate = 0;

        // Decide which 'Bad cluster' modes to enable
        if((mContext->GetRecipeOptions().mIsGDBNEnabled == true) && (lfYieldRate <= lGdbnRule.mYieldThreshold))
            bStandardBadNeighbors = true;	// Perform bad neighorhood detection (internal algorithms)

        // If Custom wafermap (Z-PAT) to process, ignore the yield threshold!
        if(lExternWafermap != NULL)
            bStandardBadNeighbors = true;

        // Scan all Good parts and see if enough failing neighbours to fail part.
        if(bStandardBadNeighbors && ptWafermap->bWaferMapExists)
        {
            gdbnEngine.setAlgorithm(lGdbnRule);
            gdbnEngine.processWafer(ptWafermap, ptGoodBinList, mContext->mGDBNOutliers);
        }
    }

    // Check if Custom PAT Library to be called.
    if(mContext->GetRecipeOptions().bCustomPatLib && mContext->GetRecipeOptions().mGDBNCustomLib)
    {
        // Call Custom PAT lib (plugin)
        mExternalPat.pat_wafermap(&mContext->m_AllSitesMap_Sbin, mContext);
    }

    // ensure we overload master maps bin (Force to Fail die on wafermap! So next rules can inherit & process this failure)
    tdIterGPATOutliers itGDBNOutlier(mContext->mGDBNOutliers);
    while (itGDBNOutlier.hasNext())
    {
        itGDBNOutlier.next();

        mContext->OverloadRefWafermap(itGDBNOutlier.value().mDieX, itGDBNOutlier.value().mDieY,
                                      itGDBNOutlier.value().mPatSBin,
                                      itGDBNOutlier.value().mPatHBin);
    }

    return true;
}

bool PATOutlierFinderWSPrivate::AnalyzeWaferSurfaceIDDQ_Delta()
{
    PAT_PERF_BENCH

    // If rule disabled, return now!
    if(mContext->GetRecipeOptions().mIsIDDQ_Delta_enabled == false)
        return true;

    // Point to data structure
    bool                lOutlier;
    int                 lDieX;
    int                 lDieY;
    int                 lOutlierBin;
    double              lExponent;
    double              lValue          = GEX_C_DOUBLE_NAN;
    double              lLowLimit;
    double              lHighLimit;
    QString             lKey;
    CPatDieCoordinates  lGoodBin;
    CTest *             lTestTmp        = NULL;
    CTest *             lTestPreStress  = NULL;
    CTest *             lTestPostStress = NULL;
    CTest *             lTestDieX       = NULL;
    CTest *             lTestDieY       = NULL;

    if(mFile->getWaferMapData().bWaferMapExists == false)
        return false;

    // Get handle to DieX and DieY parameters
    if(mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &lTestDieX, true, false) != 1)
        return false;
    if(mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &lTestDieY, true, false) != 1)
        return false;

    // Identify list of IDDQ tests matching naming convention
    QString	lPreStressName;
    QString lPostStressName;

    CGexStats lStatsEngine;
    lStatsEngine.UpdateOptions(&ReportOptions);

    // Set global NNR options
    QList <CIDDQ_Delta_Rule>::iterator itIDDQ_Delta_Rule;
    CIDDQ_Delta_Rule lRule;

    // Create a part filter on good soft bins
    PATPartFilter lPartFilter;
    lPartFilter.AddFilterElement(PATPartFilterElement(mTestSoftBin, PATPartFilterElement::InFilter,
                                                      *mContext->GetRecipeOptions().pGoodSoftBinsList));

    for(itIDDQ_Delta_Rule = mContext->GetRecipeOptions().mIDDQ_Delta_Rules.begin();
        itIDDQ_Delta_Rule != mContext->GetRecipeOptions().mIDDQ_Delta_Rules.end(); ++itIDDQ_Delta_Rule)
    {
        // Handle to rule
        lRule = *itIDDQ_Delta_Rule;

        // If rule disabled, find next one!
        if(lRule.IsEnabled() == false)
            continue;	// Move to next rule

        // Rewind test list.
        lTestTmp     = mGroup->cMergedData.ptMergedTestList;

        // Case sensitive search?
        Qt::CaseSensitivity	lCaseSensitivity;
        if(lRule.GetCaseSensitive())
            lCaseSensitivity = Qt::CaseSensitive;
        else
            lCaseSensitivity = Qt::CaseInsensitive;

        while(lTestTmp    )
        {
            // Clear variables
            lTestPreStress    = NULL;
            lTestPostStress   = NULL;

            // Is is a Pre-stress Iddq test?
            if(lTestTmp    ->strTestName.indexOf(lRule.GetPreStress(), 0, lCaseSensitivity) != -1)
            {
                // Save handle to test
                lTestPreStress = lTestTmp    ;
                lPreStressName = lTestTmp    ->strTestName;

                // Keep test name...without Pre-Stress sub-string
                lPreStressName = lPreStressName.replace(lRule.GetPreStress(), "",
                                                            lCaseSensitivity);

                // Find matching Post-Stress test -- Rewind test list.
                lTestPostStress = mGroup->cMergedData.ptMergedTestList;
                while(lTestPostStress)
                {
                    // Is is a Pre-stress Iddq test?
                    if(lTestPostStress->strTestName.indexOf(lRule.GetPostStress(), 0,
                                                                  lCaseSensitivity) != -1)
                    {
                        // Save handle to test
                        lPostStressName = lTestPostStress->strTestName;

                        // Keep test name...without Pre-Stress sub-string
                        lPostStressName = lPostStressName.replace(lRule.GetPostStress(),
                                                                      "", lCaseSensitivity);

                        // Check if Pre & Post stress test found & Compare if test names (without stress sub-string are matching)
                        if(lPreStressName.compare(lPostStressName, lCaseSensitivity) == 0)
                            break;
                    }

                    // Post-Stress test not found yet...keep searching.
                    lTestPostStress = lTestPostStress->GetNextTest();
                };

                // Check if a Pre/Post-stress pair has been identified.
                if(lTestPreStress && lTestPostStress)
                {
                    // Pre/Post-Stress pair identified!
                    lPreStressName = lTestPreStress->strTestName;
                    lPostStressName = lTestPostStress->strTestName;

                    // Fill Test cell with delta values + compute statistics.
                    CTest lDeltaTest;
                    lDeltaTest.ldSamplesExecs = lTestPreStress->m_testResult.count();

                    QString lRes = lDeltaTest.m_testResult.createResultTable(lDeltaTest.ldSamplesExecs);
                    if (lRes.startsWith("error"))
                    {
                        GSLOG(SYSLOG_SEV_ERROR,
                              QString("Analyze wafer surface : createResultTable failed : %1")
                              .arg(lRes).toLatin1().data());

                        return false;
                    }

                    lDeltaTest.res_scal = lTestPreStress->res_scal;
                    lDeltaTest.hlm_scal = lTestPreStress->hlm_scal;

                    lExponent = ScalingPower(lDeltaTest.res_scal);

                    // Loop over test results to merge parametric value
                    double	lDeltaValue;

                    // Fill tested dies with tested dies for the given parameter
                    for(int lIdx = 0; lIdx < lTestPreStress->m_testResult.count(); lIdx++)
                    {
                        if( (lTestPreStress->m_testResult.isValidResultAt(lIdx))  &&
                                (lTestPostStress->m_testResult.isValidResultAt(lIdx)))
                        {
                            // Check if we must only load parametric values for dies that are tested GOOD (Good Bin Softbin), or load all values
                            if(lPartFilter.IsFiltered(lIdx))
                            {
                                // Normalize value
                                lDeltaValue    = lTestPreStress->m_testResult.scaledResultAt(lIdx, lTestPreStress->res_scal) -
                                                    lTestPostStress->m_testResult.scaledResultAt(lIdx, lTestPostStress->res_scal);

                                lDeltaTest.m_testResult.pushResultAt(lIdx, lDeltaValue/lExponent);
                            }
                        }
                    }

                    // Update all statistics fields as so far only samples and lows stats are accurate (mean, sigma, quartiles, histo not initialized yet)
                    lStatsEngine.ComputeLowLevelTestStatistics(&lDeltaTest,lExponent);
                    lStatsEngine.ComputeBasicTestStatistics(&lDeltaTest,true);
                    lStatsEngine.RebuildHistogramArray(&lDeltaTest,GEX_HISTOGRAM_OVERDATA);
                    lStatsEngine.ComputeAdvancedDataStatistics(&lDeltaTest,true,false);

                    // Compute DPAT limits
                    switch(lRule.GetAlgorithm())
                    {
                        default:
                        case GEX_TPAT_IDDQ_DELTA_SIGMA:	// Mean+N*Sigma
                            lLowLimit = lDeltaTest.lfMean - lRule.GetNFactor()*lDeltaTest.lfSigma;
                            lHighLimit = lDeltaTest.lfMean + lRule.GetNFactor()*lDeltaTest.lfSigma;
                            break;

                        case GEX_TPAT_IDDQ_DELTA_ROBUSTS:	// Median+N*RobustSigma
                            lLowLimit = lDeltaTest.lfSamplesQuartile2 - lRule.GetNFactor()*((lDeltaTest.lfSamplesQuartile3-lDeltaTest.lfSamplesQuartile1)/1.35);
                            lHighLimit = lDeltaTest.lfSamplesQuartile2 + lRule.GetNFactor()*((lDeltaTest.lfSamplesQuartile3-lDeltaTest.lfSamplesQuartile1)/1.35);
                            break;

                        case GEX_TPAT_IDDQ_Q1Q3IQR:	// Q1-N*IQR, Q3+N*IQR
                            lLowLimit = lDeltaTest.lfSamplesQuartile1 - lRule.GetNFactor()*((lDeltaTest.lfSamplesQuartile3-lDeltaTest.lfSamplesQuartile1)/1.35);
                            lHighLimit = lDeltaTest.lfSamplesQuartile3 + lRule.GetNFactor()*((lDeltaTest.lfSamplesQuartile3-lDeltaTest.lfSamplesQuartile1)/1.35);
                            break;
                    }

                    // Scan parametric wafermap array to detect DPAT outliers
                    for(int lIdx = 0; lIdx < lDeltaTest.m_testResult.count(); ++lIdx)
                    {
                        if(lDeltaTest.m_testResult.isValidResultAt(lIdx))
                        {
                            // Compute die location
                            lDieX = (int) lTestDieX->m_testResult.resultAt(lIdx);
                            lDieY = (int) lTestDieY->m_testResult.resultAt(lIdx);

                            // Die coordinates are valid, so
                            if (lDieX != GEX_WAFMAP_INVALID_COORD && lDieY != GEX_WAFMAP_INVALID_COORD)
                            {
                                // If die already outlier, do not check IDDQ-Delta rule over it!
                                lOutlier = false;

                                if(mContext->isDieOutlier(lDieX,lDieY,lOutlierBin) == false)
                                {
                                    // Get delta value
                                    lValue = lDeltaTest.m_testResult.resultAt(lIdx) * lExponent;

                                    // IDDQ-Delta outlier?
                                    if( (lValue < lLowLimit) || (lValue > lHighLimit) )
                                        lOutlier = true;
                                }

                                // Outlier die identified: This good bin passes the conditions to be failed
                                if(lOutlier)
                                {
                                    // ensure we overload master maps bin (Force to Fail die on wafermap! So next rules can inherit & process this failure)
                                    mContext->OverloadRefWafermap(lDieX, lDieY,
                                                                  lRule.GetSoftBin(),
                                                                  lRule.GetHardBin());

                                    lKey = QString::number(lDieX) + "." + QString::number(lDieY);

                                    // Save NNR test details
                                    CPatOutlierIDDQ_Delta *ptOutlierIDDQ_Delta = new CPatOutlierIDDQ_Delta;

                                    ptOutlierIDDQ_Delta->iDieX = lDieX;
                                    ptOutlierIDDQ_Delta->iDieY = lDieY;
                                    ptOutlierIDDQ_Delta->iSite = gexReport->getDieTestingSite(-1,0,lDieX,lDieY);	// Testing site
                                    ptOutlierIDDQ_Delta->lTestNumber1 = lTestPreStress->lTestNumber;
                                    ptOutlierIDDQ_Delta->lPinmapIndex1 = lTestPreStress->lPinmapIndex;
                                    ptOutlierIDDQ_Delta->lTestNumber2 = lTestPostStress->lTestNumber;
                                    ptOutlierIDDQ_Delta->lPinmapIndex2 = lTestPostStress->lPinmapIndex;
                                    ptOutlierIDDQ_Delta->lfValue = lValue;				// Holds outlier data point value (same units as samples).
                                    mContext->pIDDQ_Delta_OutlierTests.append(ptOutlierIDDQ_Delta);	// Add test details to global list

                                    // Check if This die location is already flagged as NNR failure...
                                    if(mContext->mIDDQOutliers.contains(lKey) == false)
                                    {
                                        lGoodBin.mDieX      = lDieX;
                                        lGoodBin.mDieY      = lDieY;
                                        lGoodBin.mSite      = ptOutlierIDDQ_Delta->iSite;	// Testing site
                                        lGoodBin.mFailType  = GEX_TPAT_BINTYPE_IDDQ_DELTA;
                                        lGoodBin.mRuleName  = lRule.GetRuleName();
                                        lGoodBin.mPatHBin   = lRule.GetHardBin();
                                        lGoodBin.mPatSBin   = lRule.GetSoftBin();
                                        lGoodBin.mOrigHBin  = mContext->GetOriginalBin(false, lDieX, lDieY);
                                        lGoodBin.mOrigSBin  = mContext->GetOriginalBin(true, lDieX, lDieY);

                                        if (mGroup->FindPartId(lDieX, lDieY, lGoodBin.mPartId) == false)
                                            lGoodBin.mPartId = "?";

                                        // Save NNR Part definition in our list
                                        mContext->mIDDQOutliers.insert(lKey, lGoodBin);	// Holds the Good Die X,Y position to fail
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Search for next pre-stress test
            lTestTmp    = lTestTmp->GetNextTest();
        }
    }// Executed for each IDDQ-Delta rule.

    return true;
}

bool PATOutlierFinderWSPrivate::AnalyzeWaferSurfaceNNR()
{
    PAT_PERF_BENCH

    // If rule disabled, return now!
    if(mContext->GetRecipeOptions().IsNNREnabled() == false)
        return true;

    // Point to data structure
    if(mFile->getWaferMapData().bWaferMapExists == false)
        return false;

    // Identify NNR tests on each tested site
    CParametricWaferMap     cParamWafMap;		// To hold Parametric wafermap
    CParametricWaferMap     cParamWafMap_NNR;	// To hold NNR wafermap (delta values)
    CParametricWaferMapNNR  cNNR;
    CPatDieCoordinates      lGoodBin;
    CTest *                 ptTestCell  = NULL;
    CPatDefinition	*       ptPatDef    = NULL;
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
    int                     i,iLine,iCol,iDieX,iDieY,iOutlierBin;

    // Get wafermap visual options
    QStringList lstVisualOptions	= ReportOptions.GetOption("wafer", "visual_options").toString().split("|");
    bool		bWafmapMirrorX		= lstVisualOptions.contains("mirror_x");
    bool		bWafmapMirrorY		= lstVisualOptions.contains("mirror_y");

    // Check all tests in the recipe and keep only those which are enabled.
    QList<CPatDefinition*> lNNRTest;

    for(itPATDefinifion = mContext->GetUnivariateRules().begin();
        itPATDefinifion != mContext->GetUnivariateRules().end(); ++itPATDefinifion)
    {
        ptPatDef = *itPATDefinifion;

        // Chekc if NNR enabled for this test.
        if(ptPatDef->m_iNrrRule == GEX_TPAT_NNR_ENABLED)
            lNNRTest.append(ptPatDef);
    }

    // Sort pat test definition by test number
    qSort(lNNRTest.begin(), lNNRTest.end(), CPatDefinition::SortOnTestNumber);

    // Set global NNR options
    QList<CNNR_Rule >::iterator		itNNR_Rule;
    CNNR_Rule						lRule;
    for(itNNR_Rule = mContext->GetRecipeOptions().GetNNRRules().begin(); itNNR_Rule != mContext->GetRecipeOptions().GetNNRRules().end(); ++itNNR_Rule)
    {
        // Handle to rule
        lRule = *itNNR_Rule;

        // If rule disabled, find next one!
        if(lRule.IsEnabled() == false)
            continue;	// Move to next rule

        cNNR.iMatrixSize = lRule.GetClusterSize();
        cNNR.iAlgorithm = lRule.GetAlgorithm();
        cNNR.lfN_Factor = lRule.GetNFactor();
        cNNR.lf_LA = static_cast<int>(lRule.GetLA());

        // Check all tests in the recipe editor....
        for(int lIdx = 0; lIdx < lNNRTest.count(); ++lIdx)
        {
            ptPatDef    = lNNRTest.at(lIdx);
            ptTestCell  = FindTestCell(ptPatDef->m_lTestNumber, ptPatDef->mPinIndex, ptPatDef->m_strTestName);

            // Get handle to Test
            if(ptTestCell != NULL)
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("Compute NNR, T%1").arg(ptPatDef->m_lTestNumber).toLatin1().data());

                // Load Parametric wafermap: all sites merged
                if(cParamWafMap.FillWaferMapFromSingleDataset(ptTestCell, mContext->GetSiteList(),
                                                              mContext->GetRecipeOptions().pGoodSoftBinsList) == false)
                    goto next_test;

                // Compute NNR Parametric wafermap: for each die
                if(cParamWafMap_NNR.NNR_BuildMap(cParamWafMap, cNNR) == false)
                {
                    // Failed to compute NNR: not enough data
                    GSLOG(SYSLOG_SEV_WARNING,
                          QString("Failed to compute NNR (not enough data): T%1")
                          .arg(ptPatDef->m_lTestNumber).toLatin1().data());

                    mContext->logPatWarning("NRR",ptPatDef->m_lTestNumber,ptPatDef->mPinIndex,"Failed to compute NNR (not enough data)");
                }

                // If limits have been computed, look for outliers
                if (cParamWafMap_NNR.mHighLimit != C_INFINITE && cParamWafMap_NNR.mLowLimit != -C_INFINITE)
                {
                    // Check if parametric wafermap has any NNR over the map surface?
                    int lSize = cParamWafMap.GetSizeX()*cParamWafMap.GetSizeY();
                    for(i=0;i < lSize;i++)
                    {
                        // Compute array location
                        iCol = i % cParamWafMap.GetSizeX();
                        iLine = i / cParamWafMap.GetSizeX();

                        // Compute die location
                        cNNR.iDieX = iDieX = iCol + cParamWafMap.GetLowDieX();
                        cNNR.iDieY = iDieY = iLine + cParamWafMap.GetLowDieY();
                        cNNR.lfValue = cParamWafMap_NNR.cParametricWafMap[i].lfValue;

                        if((mContext->isDieOutlier(iDieX,iDieY,iOutlierBin) == false) && (cParamWafMap_NNR.isNNR(cNNR)))
                        {

                            // ensure we overload master maps bin (Force to Fail die on wafermap! So next rules can inherit & process this failure)
                            mContext->OverloadRefWafermap(iDieX, iDieY,
                                                          lRule.GetSoftBin(),
                                                          lRule.GetHardBin());

                            // This good bin passes the conditions to be failed
                            QString strKey;

                            // Check if X-axis swap option enabled...
                            if(bWafmapMirrorX == true)
                                iDieX = mFile->getWaferMapData().iHighDieX - iCol;
                            else
                                iDieX = iCol+mFile->getWaferMapData().iLowDieX;

                            // Check if Y-axis swap option enabled...
                            if(bWafmapMirrorY == true)
                                iDieY = mFile->getWaferMapData().iHighDieY - iLine;
                            else
                                iDieY = iLine+mFile->getWaferMapData().iLowDieY;

                            strKey = QString::number(iDieX) + "." + QString::number(iDieY);

                            // Save NNR test details
                            CPatOutlierNNR *ptOutlierNNR = new CPatOutlierNNR;

                            ptOutlierNNR->mDieX         = iDieX;
                            ptOutlierNNR->mDieY         = iDieY;
                            ptOutlierNNR->mSite         = gexReport->getDieTestingSite(-1,0,iDieX,iDieY);	// Testing site
                            ptOutlierNNR->mTestNumber   = ptPatDef->m_lTestNumber;
                            ptOutlierNNR->mPinmap       = ptPatDef->mPinIndex;
                            ptOutlierNNR->mTestName     = ptPatDef->m_strTestName;
                            ptOutlierNNR->mValue        = cNNR.lfValue;				// Holds outlier data point value (same units as samples).
                            ptOutlierNNR->mNNRRule      = lRule;
                            mContext->pNNR_OutlierTests.append(ptOutlierNNR);	// Add test details to global list

                            // Check if This die location is already flagged as NNR failure...
                            if(mContext->mNNROutliers.contains(strKey) == false)
                            {
                                lGoodBin.mDieX          = iDieX;
                                lGoodBin.mDieY          = iDieY;
                                lGoodBin.mSite          = ptOutlierNNR->mSite;	// Testing site
                                lGoodBin.mFailType      = GEX_TPAT_BINTYPE_NNR;
                                lGoodBin.mRuleName      = lRule.GetRuleName();
                                lGoodBin.mPatHBin       = lRule.GetSoftBin();
                                lGoodBin.mPatSBin       = lRule.GetSoftBin();
                                lGoodBin.mOrigHBin      = mContext->GetOriginalBin(false, iDieX, iDieY);
                                lGoodBin.mOrigSBin      = mContext->GetOriginalBin(true, iDieX, iDieY);

                                if (mGroup->FindPartId(iDieX, iDieY, lGoodBin.mPartId) == false)
                                    lGoodBin.mPartId = "?";

                                // Save NNR Part definition in our list
                                mContext->mNNROutliers.insert(strKey, lGoodBin);	// Holds the Good Die X,Y position to fail
                            }

                        }
                    }
                }
            }
            next_test:;
        }
    } // Executed for each NNR rule.

    return true;
}

bool PATOutlierFinderWSPrivate::AnalyzeWaferSurfacePotatoCluster(CWaferMap *lExternWafermap)
{
    PAT_PERF_BENCH

    // If rule disabled, return now!
    if(mContext->GetRecipeOptions().mClusteringPotato == false)
        return true;

    // Compute the Cluster size (based on user rules)
    CWaferMap	*           ptWafermap;
    GS::QtLib::Range *      ptGoodBinList;
    QList <CClusterPotatoRule>::iterator itClusterPotato;
    CClusterPotatoRule      lClusteringRule;
    long                    iClusterSize = -1;
    for(itClusterPotato = mContext->GetRecipeOptions().mClusterPotatoRules.begin();
        itClusterPotato != mContext->GetRecipeOptions().mClusterPotatoRules.end(); ++itClusterPotato)
    {
        // Handle to rule
        lClusteringRule = *itClusterPotato;

        // If rule disabled, find next one!
        if(lClusteringRule.mIsEnabled == false)
            continue;	// Move to next rule

        // Get handle to relevant wafermap (STDF soft bin or STDF Hard bin or Prober map)
        switch(lClusteringRule.mWaferSource % 3)
        {
            case GEX_PAT_WAFMAP_SRC_SOFTBIN:
                ptWafermap = &mContext->m_AllSitesMap_Sbin;
                ptGoodBinList = mContext->GetRecipeOptions().pGoodSoftBinsList;	// Checking over Good SOFT bins
                break;
            case GEX_PAT_WAFMAP_SRC_HARDBIN:
                ptWafermap = &mContext->m_AllSitesMap_Hbin;
                ptGoodBinList = mContext->GetRecipeOptions().pGoodHardBinsList;	// Checking over Good HARD bins
                break;
            case GEX_PAT_WAFMAP_SRC_PROBER:
                ptWafermap = &mContext->m_ProberMap;
                ptGoodBinList = mContext->GetRecipeOptions().pGoodHardBinsList;	// Checking over Good HARD bins
                break;
            default:
                // Unsupported flow: don't do anything!
                return false;
        }
        // If specific map defined by caller, use it instead!
        if(lExternWafermap != NULL)
            ptWafermap = lExternWafermap;

        // Check if rule matching criteria
        if(lClusteringRule.mClusterSize < 0)
            iClusterSize = (long) (((long)ptWafermap->iTotalPhysicalDies*(0-lClusteringRule.mClusterSize))/100L);	// Rule is X% of wafer size
        else
            iClusterSize = (long) lClusteringRule.mClusterSize;	// Rule is X dies is cluster size.

        // Check if Mask exists (if so, load it)
        CWaferMap::Ring eMaskRegion = CWaferMap::NoRing;
        int				iMaskRadius = -1;

        if(lClusteringRule.mMaskName.isEmpty() == false)
        {
            CMask_Rule *ptMask = mContext->GetMaskDefinition(lClusteringRule.mMaskName);
            if(ptMask)
            {
                iMaskRadius = ptMask->mRadius;

                switch(ptMask->mWorkingArea)
                {
                    case 0:	// Outer ring
                        eMaskRegion = CWaferMap::RingFromEdge;
                        break;
                    case 1:	// Inner ring
                        eMaskRegion = CWaferMap::RingFromCenter;
                        break;
                }
            }
        }

        // Define settings for potato cluster algorithm
        CPatPotatoClusterSettings	potatoClusterSettings(iClusterSize);

        potatoClusterSettings.setRingRadius(iMaskRadius);
        potatoClusterSettings.setRingArea(eMaskRegion);
        potatoClusterSettings.setEdgeDieType(lClusteringRule.mEdgeDieType);
        potatoClusterSettings.setEdgeDieWeighting(lClusteringRule.mEdgeDieWeighting);
        potatoClusterSettings.setEdgeDieWeightingScale(lClusteringRule.mEdgeDieWeightingScale);
        potatoClusterSettings.setOutlineWidth(lClusteringRule.mOutlineWidth);
        potatoClusterSettings.enableIgnoreVerticalScratches(lClusteringRule.mIgnoreScratchLines);
        potatoClusterSettings.enableIgnoreHorizontalScratches(lClusteringRule.mIgnoreScratchRows);
        potatoClusterSettings.enableIgnoreDiagonalBadDies(lClusteringRule.mIgnoreDiagonalBadDies);
        potatoClusterSettings.setFailType(GEX_TPAT_BINTYPE_BADCLUSTER);
        potatoClusterSettings.setRuleName(lClusteringRule.mRuleName);
        potatoClusterSettings.SetSoftBin(lClusteringRule.mSoftBin);
        potatoClusterSettings.SetHardBin(lClusteringRule.mHardBin);

        // Below is the code to enable the bad neighbours algorithm into potatoclusterfinder
        potatoClusterSettings.enableUseBadNeighboursAlgorithm(lClusteringRule.mIsLightOutlineEnabled);
        if(lClusteringRule.mIsLightOutlineEnabled)
        {
            // extract only weight list matching the cluster size
            const int lListSize = qMin(lClusteringRule.mDiagWeightLst.size(), lClusteringRule.mOutlineMatrixSize + 1);
            potatoClusterSettings.setAdjacentWeighting(
                        lClusteringRule.mAdjWeightLst.mid(0, lListSize));
            potatoClusterSettings.setDiagonalWeighting(
                        lClusteringRule.mDiagWeightLst.mid(0, lListSize));
            potatoClusterSettings.setMinimumWeighting(lClusteringRule.mFailWeight);
        }

        // Apply potato cluster algorithm
        CPatPotatoClusterFinder		potatoClusterFinder(potatoClusterSettings);

        // Scan wafer, and add list of good bin rejected to bin list
        potatoClusterFinder.processWafer(ptWafermap, ptGoodBinList,
          lClusteringRule.mBadBinIdentifyList, lClusteringRule.mBadBinInkingList,
          &mContext->mClusteringOutliers);

        // ensure we overload master maps bin (Force to Fail die on wafermap! So next rules can inherit & process this failure)
        tdIterGPATOutliers itClustering(mContext->mClusteringOutliers);
        while (itClustering.hasNext())
        {
            itClustering.next();

            mContext->OverloadRefWafermap(itClustering.value().mDieX, itClustering.value().mDieY,
                                          itClustering.value().mPatSBin,
                                          itClustering.value().mPatHBin);
        }
    }

    return true;
}

bool PATOutlierFinderWSPrivate::AnalyzeWaferSurfaceReticle(CWaferMap *lExternWafermap)
{
    PAT_PERF_BENCH

    // If rule disabled, return now!
    if(mContext->GetRecipeOptions().GetReticleEnabled() == false)
        return true;

    for (int lIdx = 0; lIdx < mContext->GetRecipeOptions().GetReticleRules().count(); ++lIdx)
    {
        if (mContext->GetRecipeOptions().GetReticleRules().at(lIdx).IsReticleEnabled())
        {
            // Check if Reticle analysis over SoftBin or HardBin
            CWaferMap	* lWafermap;

            // Get handle to relevant wafermap (soft bin or Hard bin)
            switch(mContext->GetRecipeOptions().GetReticleRules().at(lIdx).GetReticle_WafermapSource() % 3)
            {
                case GEX_PAT_WAFMAP_SRC_SOFTBIN:
                    lWafermap = &mContext->m_AllSitesMap_Sbin;
                    // Checking over Good SOFT bins
                    break;
                case GEX_PAT_WAFMAP_SRC_HARDBIN:
                    lWafermap = &mContext->m_AllSitesMap_Hbin;
                    // Checking over Good HARD bins
                    break;
                case GEX_PAT_WAFMAP_SRC_PROBER:
                    lWafermap = &mContext->m_ProberMap;
                    // Checking over Good HARD bins
                    break;
                default:
                    // Unsupported flow: don't do anything!
                    return false;
            }

            // If specific map defined by caller, use it instead!
            if(lExternWafermap != NULL)
                lWafermap = lExternWafermap;

            PatReticleEngine lReticleEngine;

            if (lReticleEngine.processWafer(lWafermap, mContext->GetRecipeOptions().pGoodHardBinsList,
                                            mContext->GetRecipeOptions().GetReticleRules().at(lIdx),
                                            mContext->mReticleOutliers) == false)
            {
                return false;
            }

            // ensure we overload master maps bin (Force to Fail die on wafermap! So next rules can inherit & process this failure)
            tdIterGPATOutliers itReticle(mContext->mReticleOutliers);
            while (itReticle.hasNext())
            {
                itReticle.next();

                mContext->OverloadRefWafermap(itReticle.value().mDieX, itReticle.value().mDieY,
                                              itReticle.value().mPatSBin,
                                              itReticle.value().mPatHBin);
            }
        }
    }

    return true;
}

bool PATOutlierFinderWSPrivate::BuildMVPATCorrelationCharts(const QString& lRuleName,
                                                     int lIndex,
                                                     SE::MVOutliersFinder& lAlgo,
                                                     SE::RMatrix& lMatrix,
                                                     SE::RVector &lLabels,
                                                     const QString& lTitle,
                                                     const QStringList& lAxisLabels /*= QStringList()*/,
                                                     double lChi /*= 0*/, double lSigma /*= 0*/)
{
    QSize   lSize(400, 300);
    QString lImagePath;
    QString lYLabel;
    QString lXLabel;
    int     lChartCount = 0;

    if (mContext->GetRecipeOptions().GetMVPATReportPairs() == PAT::ConsecutivePairs)
    {
        for (int lIdx = 0; lIdx < lMatrix.GetCols()-1 && lChartCount < mContext->GetRecipeOptions().GetMVPATReportMaxCharts(); lIdx += 2)
        {
            lImagePath = Engine::GetInstance().Get("TempFolder").toString() + QDir::separator();
            lImagePath += QString("mvrule_%1_corr_%2_vs_%3.png").arg(lIndex).arg(lIdx).arg(lIdx+1);

            if (lIdx < lAxisLabels.count())
                lXLabel = lAxisLabels.at(lIdx);
            else
                lXLabel = QString("Element# %1").arg(lIdx);

            if (lIdx+1 < lAxisLabels.count())
                lYLabel = lAxisLabels.at(lIdx+1);
            else
                lYLabel = QString("Element# %1").arg(lIdx+1);

            if (lAlgo.BuildCorrelationChart(lMatrix, lIdx, lIdx+1, lImagePath,
                                            lLabels, lSize, lTitle, lXLabel, lYLabel,
                                            lChi, lSigma))
            {
                mContext->AddMVPATRuleChartPath(lRuleName, lImagePath);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Error while building correlation chart: %1")
                      .arg(lAlgo.GetLastError()).toLatin1().data());
                return false;
            }

            ++lChartCount;
        }
    }
    else if (mContext->GetRecipeOptions().GetMVPATReportPairs() == PAT::AllPairs)
    {
        for (int lIdx = 0; lIdx < lMatrix.GetCols() && lChartCount < mContext->GetRecipeOptions().GetMVPATReportMaxCharts(); ++lIdx)
        {
            for (int lOther = lIdx+1; lOther < lMatrix.GetCols(); ++lOther)
            {
                lImagePath = Engine::GetInstance().Get("TempFolder").toString() + QDir::separator();
                lImagePath += QString("mvrule_%1_corr_%2_vs_%3.png").arg(lIndex).arg(lIdx).arg(lOther);

                if (lIdx < lAxisLabels.count())
                    lXLabel = lAxisLabels.at(lIdx);
                else
                    lXLabel = QString("Element# %1").arg(lIdx);

                if (lOther < lAxisLabels.count())
                    lYLabel = lAxisLabels.at(lOther);
                else
                    lYLabel = QString("Element# %1").arg(lOther);

                if (lAlgo.BuildCorrelationChart(lMatrix, lIdx, lOther, lImagePath,
                                                lLabels, lSize, lTitle, lXLabel, lYLabel,
                                                lChi, lSigma))
                {
                    mContext->AddMVPATRuleChartPath(lRuleName, lImagePath);
                }
                else
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Error while building correlation chart: %1")
                          .arg(lAlgo.GetLastError()).toLatin1().data());
                    return false;
                }
            }

            ++lChartCount;
        }
    }

    return true;
}

bool PATOutlierFinderWSPrivate::ComputeMVPATOutlier(const PATMultiVariateRule &lRule,
                                                    int lIndex,
                                                    GS::SE::StatsEngine *statsEngine)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("[Begin] Compute MVPAT Outliers for Rule %1").arg(lRule.GetName()).toLatin1().constData());

    if (!statsEngine)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Stats Engine NULL");
        return false;
    }
    PAT_PERF_BENCH

    SE::MVOutliersFinder * lAlgo = static_cast<SE::MVOutliersFinder*>(
                statsEngine->GetAlgorithm(SE::StatsAlgo::MV_OUTLIERS_FINDER));

    if (lAlgo == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Outliers finder algorithm not loaded");
        return false;
    }

    // Get pointer to first group & first file (we always have them exist)
    CTest *                             lTestCell   = NULL;
    double                              lMedianRef  = 0.0;
    double                              lSiteOffset = 0.0;
    int                                 lCol        = 0;
    int                                 lRowCount   = 0;

    // Count the number of samples
    lRowCount = mTestSoftBin->m_testResult.count();

    // Build labels for histograms
    GS::SE::RVector lLabels;
    int             lLabelIdx = 0;
    lLabels.Build("histolabels", lRowCount, GS::SE::RVector::V_STD);

    for (int lPart = 0; lPart < mFile->pPartInfoList.count(); ++lPart)
    {
        lLabels.FillStd(lLabelIdx, mFile->pPartInfoList.at(lPart)->getPartID());
        ++lLabelIdx;
    }

    QSharedPointer<SE::RData> lRData(new SE::RData());
    SE::RMatrix* lRMatrix = lRData.data()->AllocateMatrix(SE::StatsData::MVOUTLIER_IN_1,
                                                          lRowCount,
                                                          lRule.GetMVTestData().count());

    if (lRMatrix == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Unable to allocate data matrix");
        return false;
    }

    // Iterate over all parameters
    QStringList     lAxisLabels;
    PATPartFilter   lPartFilter;
    for (int lIdxTest = 0; lIdxTest < lRule.GetMVTestData().count(); ++lIdxTest)
    {
        // Loop over each site to fill the R structure
        for (int lIdxSite = 0; lIdxSite < mContext->GetSiteList().count(); ++lIdxSite)
        {
            lTestCell = FindTestCell(lRule.GetMVTestData().at(lIdxTest).GetTestNumber(),
                                     lRule.GetMVTestData().at(lIdxTest).GetPinIdx(),
                                     lRule.GetMVTestData().at(lIdxTest).GetTestName());

            if (lTestCell == NULL)
            {
                return false;
            }

            if (FillPartFilter(&lPartFilter, mContext->GetSiteList().at(lIdxSite)) == false)
            {
                return false;
            }

            // First site would be used as the reference
            if (lIdxSite == 0)
            {
                QString lTestLabel = QString::number(lTestCell->lTestNumber);

                if (lTestCell->lPinmapIndex >= 0)
                    lTestLabel += "." + QString::number(lTestCell->lPinmapIndex);

                lTestLabel += "." + lTestCell->strTestName;

                lAxisLabels.append(lTestLabel);
            }

            // Compute test statistics for this parameter
            if (ComputeTestStatistics(lTestCell, &lPartFilter) == false)
                return false;

            // Compute site offset
            if (lTestCell->lfSamplesQuartile2 != -C_INFINITE)
            {
                // First site would be used as the reference
                if (lIdxSite == 0)
                {
                    lMedianRef  = lTestCell->lfSamplesQuartile2;
                    lSiteOffset = 0.0;
                }
                else
                    lSiteOffset = lTestCell->lfSamplesQuartile2 - lMedianRef;
            }
            else
                return false;

            // Fill the R Matrix with offseted results
            PATPartFilter lPartFilterSite;
            lPartFilterSite.AddFilterElement(PATPartFilterElement(mTestSite,
                                                                  PATPartFilterElement::InFilter,
                                                                  QtLib::Range(QString::number(mContext->GetSiteList().at(lIdxSite)))));
            for(int lIdx = 0; lIdx < lTestCell->m_testResult.count(); ++lIdx)
            {
                if (lPartFilterSite.IsFiltered(lIdx))
                {
                    if(lPartFilter.IsFiltered(lIdx) && lTestCell->m_testResult.isValidResultAt(lIdx))
                    {
                        lRMatrix->Fill(lIdx, lCol,
                                       lTestCell->m_testResult.resultAt(lIdx) - lSiteOffset);
                    }
                    else
                        lRMatrix->Fill(lIdx, lCol, nan(""));
                }
            }
        };

        // Increment the col number (correspond to a parameter)
        ++lCol;
    }

    // Prepare Statistical Engine with data and parameters
    QMap<SE::StatsAlgo::Parameter, QVariant>    lParam;
    double                                      lOutlierDistance;

    if (lRule.GetOutlierDistanceMode() != PAT::Custom)
        lOutlierDistance = mContext->GetRecipeOptions().GetMVPATDistance(lRule.GetOutlierDistanceMode());
    else
        lOutlierDistance = lRule.GetCustomDistance();

    lParam.insert(SE::StatsAlgo::SIGMA, QVariant(lOutlierDistance));

    if (lAlgo->Execute(lParam, lRData.data()) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error while executing Outliers Finder algorithm: %1")
              .arg(lAlgo->GetLastError()).toLatin1().data());
        return false;
    }

    // Search for outliers
    bool            lOk         = false;
    int             lOutlierIdx;
    double          lZScore;
    CPartInfo *     lPartInfo   = NULL;

    SE::RVector lOutliers   = lAlgo->GetOutliers(lOk);

    if (!lOk)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error while retrieving outliers: %1")
              .arg(lAlgo->GetLastError()).toLatin1().data());
        return false;
    }

    SE::RVector lZScores    = lAlgo->GetZScores(lOk);

    if (!lOk)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error while retrieving zscore: %1")
              .arg(lAlgo->GetLastError()).toLatin1().data());
        return false;
    }

    if (!lOutliers.IsNull() && !lOutliers.IsEmpty())
    {
        for (int lIdx = 0; lIdx < lOutliers.GetSize(); ++lIdx)
        {
            lOutlierIdx = lOutliers.GetDoubleItem(lIdx, lOk);

            if (!lOk)
                return false;

            lZScore = lZScores.GetDoubleItem(lOutlierIdx, lOk);

            if (!lOk)
                return false;

            // Find the CPartInfo object corresponding to the outlier
            lPartInfo   = NULL;

            for (int idxFile = 0; idxFile < mGroup->pFilesList.count() && lPartInfo == NULL; ++idxFile)
            {
                CGexFileInGroup * pTmpFile = mGroup->pFilesList.at(idxFile);

                if (pTmpFile)
                {
                    if (lOutlierIdx < pTmpFile->pPartInfoList.count())
                        lPartInfo = pTmpFile->pPartInfoList.at(lOutlierIdx);
                    else
                        lOutlierIdx -= pTmpFile->pPartInfoList.count();
                }
            }

            if (lPartInfo == NULL)
                return false;

            WaferCoordinate lCoord(lPartInfo->iDieX, lPartInfo->iDieY);

            // Add MV Outlier
            if (mContext->IsMVOutlier(lCoord) == false)
            {
                PATMVOutlier lMVOutlier;

                lMVOutlier.SetCoordinate(lCoord);
                lMVOutlier.SetPartID(lPartInfo->getPartID());
                lMVOutlier.SetHardBin(lRule.GetBin());
                lMVOutlier.SetSoftBin(lRule.GetBin());
                lMVOutlier.SetSite(lPartInfo->m_site);
                lMVOutlier.SetOriginalHardBin(mContext->GetOriginalBin(false, lCoord.GetX(), lCoord.GetY()));
                lMVOutlier.SetOriginalSoftBin(mContext->GetOriginalBin(true, lCoord.GetX(), lCoord.GetY()));

                mContext->AddMVOutlier(lMVOutlier);
            }

            // Add MV failing rule
            PATMVFailingRule lMVFailingRule;

            lMVFailingRule.SetRuleName(lRule.GetName());
            lMVFailingRule.SetZScore(lZScore);

            for (int lIdx = 0; lIdx < mContext->GetMultiVariateRules().count(); ++lIdx)
            {
                if (lZScore >= mContext->GetRecipeOptions().GetMVPATDistance(PAT::Far))
                    lMVFailingRule.SetSeverity(PAT::Far);
                else if (lZScore >= mContext->GetRecipeOptions().GetMVPATDistance(PAT::Medium))
                    lMVFailingRule.SetSeverity(PAT::Medium);
                else
                    lMVFailingRule.SetSeverity(PAT::Near);
            }

            if (mContext->AddMVFailingRule(lCoord, lMVFailingRule) == false)
                return false;
        }

        // Create standard charts here
        if (mContext->GetRecipeOptions().GetMVPATReportStdCharts())
        {
            QSize lSize(400, 300);
            QString lImagePath;

            lImagePath = Engine::GetInstance().Get("TempFolder").toString() + QDir::separator();
            lImagePath += QString("mvrule_%1_histo.png").arg(lIndex);
            if (lAlgo->BuildZScoresHisto(lImagePath, lSize))
                mContext->AddMVPATRuleChartPath(lRule.GetName(), lImagePath);
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Error while building zscore histogram: %1")
                      .arg(lAlgo->GetLastError()).toLatin1().data());
                return false;
            }

            lImagePath = Engine::GetInstance().Get("TempFolder").toString() + QDir::separator();
            lImagePath += QString("mvrule_%1_qqplot.png").arg(lIndex);
            if (lAlgo->BuildZScoresQQPlot(lImagePath, lSize))
                mContext->AddMVPATRuleChartPath(lRule.GetName(), lImagePath);
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Error while building zscore qqplot: %1")
                      .arg(lAlgo->GetLastError()).toLatin1().data());
                return false;
            }

            lImagePath = Engine::GetInstance().Get("TempFolder").toString() + QDir::separator();
            lImagePath += QString("mvrule_%1_trend.png").arg(lIndex);
            if (lAlgo->BuildZScoresTrend(lImagePath, lLabels, lSize))
                mContext->AddMVPATRuleChartPath(lRule.GetName(), lImagePath);
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Error while building zscore trend chart: %1")
                      .arg(lAlgo->GetLastError()).toLatin1().data());
                return false;
            }
        }

        // Create correlation charts here
        if (mContext->GetRecipeOptions().GetMVPATReportCorrCharts())
        {
            if (mContext->GetRecipeOptions().GetMVPATReportPCAProjection())
            {
                double      lChi    = lAlgo->GetChi(lOk);
                if (!lOk)
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Error while retrieving Chi value: %1")
                          .arg(lAlgo->GetLastError()).toLatin1().data());
                    return false;
                }

                double      lSigma  = lAlgo->GetSigma(lOk);
                if (!lOk)
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Error while retrieving Sigma value: %1")
                          .arg(lAlgo->GetLastError()).toLatin1().data());
                    return false;
                }

                SE::RMatrix lMatrix = lAlgo->GetPCA(lOk);
                if (!lOk)
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Error while retrieving PCA matrix: %1")
                          .arg(lAlgo->GetLastError()).toLatin1().data());
                    return false;
                }

                lAxisLabels.clear();

                for (int lIdx = 0; lIdx < lMatrix.GetCols(); ++lIdx)
                    lAxisLabels.append(QString("PC %1").arg(lIdx));

                if (BuildMVPATCorrelationCharts(lRule.GetName(), lIndex, *lAlgo,
                                                lMatrix, lLabels, "PC Correlation Chart",
                                                lAxisLabels, lChi, lSigma) == false)
                    return false;
            }
            else
                if (BuildMVPATCorrelationCharts(lRule.GetName(), lIndex, *lAlgo,
                                                *lRMatrix, lLabels, "Parameter Correlation Chart",
                                                lAxisLabels) == false)
                    return false;

        }
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("[End] Compute MVPAT Outliers for Rule %1").arg(lRule.GetName()).toLatin1().constData());

    return true;
}

int PATOutlierFinderWSPrivate::GetRequiredSamples() const
{
    return mContext->GetRecipeOptions().iMinimumSamples;
}

bool PATOutlierFinderWSPrivate::HasRequiredSamples(const CTest &lTestCell) const
{
    return lTestCell.ldSamplesValidExecs >= mContext->GetRecipeOptions().iMinimumSamples;
}

void PATOutlierFinderWSPrivate::PreComputeDynamicLimits(CTest *lTestCell)
{
    // reset shape
    lTestCell->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
    // Compute test statistics for this test
    ComputeTestStatistics(lTestCell, mPartFilter);
}

void PATOutlierFinderWSPrivate::PostComputeDynamicLimits(CTest *lTestCell)
{
    PAT_PERF_BENCH

    // If MV PAT outlier detection is activated, make all run temporary invalidated
    // during PPAT oultier detection valid.
    if (mContext->GetRecipeOptions().GetMVPATEnabled() ||
        mContext->GetRecipeOptions().IsNNREnabled() ||
        mContext->GetRecipeOptions().mIsIDDQ_Delta_enabled)
    {
        int lValidatedResults = 0;

        for(int lIdx = 0; lIdx < lTestCell->m_testResult.count(); lIdx++)
        {
            // Check if value is invalid
            if(!lTestCell->m_testResult.isValidResultAt(lIdx))
            {
                // Revalidate value, and increment valid samples (if value is not NAN)
                lTestCell->m_testResult.validateResultAt(lIdx);

                if(lTestCell->m_testResult.isValidResultAt(lIdx))
                {
                    lTestCell->ldSamplesValidExecs++;
                    lValidatedResults++;
                }
            }
        }

        // Update all statistics fields after re-validating results invalidated during dpat limits computation
        if(lValidatedResults > 0)
            ComputeTestStatistics(lTestCell, mPartFilter);
    }
}

void PATOutlierFinderWSPrivate::OnHighCPKDetected(CPatDefinition& lPatDef, CTest& lTestCell,
                                           GS::PAT::DynamicLimits &lDynLimits)
{
    //  If resulting in infinite limits, check if need to force guardbands.
    if(lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] == -GEX_TPAT_DOUBLE_INFINITE &&
       lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] == GEX_TPAT_DOUBLE_INFINITE)
    {
        // Compute N*RobustSigma*CpkCutOffLimit
        double lSpace = 3 * qMax(lPatDef.m_lfRobustSigma, lTestCell.lfSigma) *
                        mContext->GetRecipeOptions().lfSmart_IgnoreHighCpk;

        // Allow Right & Left tail processing (relaxing outlier limits if need be).
        lPatDef.m_iTailDirection = GEX_PAT_TAIL_LEFT | GEX_PAT_TAIL_RIGHT;

        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              "Force limits to mean +/- (3 * Robust Sigma) or (3 * Sigma * Cpk limit)");

        // Compute all limits sets
        for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
            ++lIdx)
        {
            lDynLimits.mLowDynamicLimit1[lIdx]   = lTestCell.lfMean - lSpace;
            lDynLimits.mLowDynamicLimit2[lIdx]   = lTestCell.lfMean - lSpace;
            lDynLimits.mHighDynamicLimit1[lIdx]  = lTestCell.lfMean + lSpace;
            lDynLimits.mHighDynamicLimit2[lIdx]  = lTestCell.lfMean + lSpace;
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Force limits to Min-Max space");

        // Stopped outlier removal when high Cpk reached, then new limits are current Min-Max space!
        for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
            ++lIdx)
        {
            // Used to enlarge limits to avoid problems due to loss of precision in float/double convertions
            double lAdjustement = (lTestCell.lfMax - lTestCell.lfMin) / 1000.0;

            lDynLimits.mLowDynamicLimit1[lIdx]   = lTestCell.lfMin - lAdjustement;
            lDynLimits.mLowDynamicLimit2[lIdx]   = lTestCell.lfMin - lAdjustement;
            lDynLimits.mHighDynamicLimit1[lIdx]  = lTestCell.lfMax + lAdjustement;
            lDynLimits.mHighDynamicLimit2[lIdx]  = lTestCell.lfMax + lAdjustement;
        }
    }
}

}
}
#endif
