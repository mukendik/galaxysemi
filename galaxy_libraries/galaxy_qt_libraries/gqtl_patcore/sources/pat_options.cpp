#include <QVariant>
#include "pat_options.h"
#include "pat_rules.h"

#include "gqtl_utils.h"

COptionsPat::COptionsPat(QObject * parent)
    : QObject(parent)
{
    pGoodSoftBinsList = NULL;
    pGoodHardBinsList = NULL;
    pBadBinsZPAT_List = NULL;
    mFT_CriticalBinsList = NULL;

    clear(GS::Gex::PAT::RecipeWaferSort);
}

COptionsPat::~COptionsPat()
{
    // Delete objects allocated in memory
    qDeleteAll(mMaskRules);

    // Destroy CGexRange objects
    if(pGoodSoftBinsList != NULL)
        delete pGoodSoftBinsList; pGoodSoftBinsList=0;
    if(pGoodHardBinsList != NULL)
        delete pGoodHardBinsList; pGoodHardBinsList=0;
    if(mFT_CriticalBinsList != NULL)
        delete mFT_CriticalBinsList; mFT_CriticalBinsList=0;
    if(pBadBinsZPAT_List != NULL)
        delete pBadBinsZPAT_List; pBadBinsZPAT_List=0;
//    if(pBadBinsReticleList != NULL)
//        delete pBadBinsReticleList; pBadBinsReticleList=0;
//    if(ptReticleYieldResults != NULL)
//        delete []ptReticleYieldResults;
}

GS::Gex::PAT::RecipeType COptionsPat::GetRecipeType() const
{
    return mRecipeType;
}

int COptionsPat::GetPATSoftBinFailType(int lBin) const
{
    int lFailType = 0;

    if (mPATSoftBinFailType.contains(lBin))
        lFailType = mPATSoftBinFailType.value(lBin);

    return lFailType;
}

int COptionsPat::GetPATHardBinFailType(int lBin) const
{
    int lFailType = 0;

    if (mPATHardBinFailType.contains(lBin))
        lFailType = mPATHardBinFailType.value(lBin);

    return lFailType;
}

void COptionsPat::SetRecipeType(GS::Gex::PAT::RecipeType lType)
{
    mRecipeType = lType;
}

void COptionsPat::resetRulePrecedence(void)
{
    strRulePrecedence.clear();
    strRulePrecedence.append("NNR");
    strRulePrecedence.append("IDDQ-Delta");
    strRulePrecedence.append("GDBN");
    strRulePrecedence.append("Clustering");
    strRulePrecedence.append("Reticle");
}

void COptionsPat::clear(GS::Gex::PAT::RecipeType lRecipeType)
{
    mRecipeType = lRecipeType;

    mPATSoftBinFailType.clear();
    mPATHardBinFailType.clear();

    iRecipeVersion  = -1;				// Disable recipe
    iRecipeBuild    = 0;
    bStaticPAT      = false;				// false if disabled
    bDynamicPAT     = true;					// false if disabled

    if (mRecipeType == GS::Gex::PAT::RecipeFinalTest)
    {
        // Soft/Hard Bin to assign if fail static PAT limits...unless customized on a per test basis
        iFailStatic_SBin    = iFailStatic_HBin  = 11;
        // Soft/Hard Bin to assign if fail static PAT limits...unless customized on a per test basis
        iFailDynamic_SBin   = iFailDynamic_HBin = 10;
    }
    else
    {
        // Soft/Hard Bin to assign if fail static PAT limits...unless customized on a per test basis
        iFailStatic_SBin    = iFailStatic_HBin  = 140;
        // Soft/Hard Bin to assign if fail static PAT limits...unless customized on a per test basis
        iFailDynamic_SBin   = iFailDynamic_HBin = 141;
    }
    // Good Soft Bins list (default: 1)
    if(pGoodSoftBinsList != NULL)
        delete pGoodSoftBinsList;
    pGoodSoftBinsList = new GS::QtLib::Range("1");	// Bin 1 is the default for 'good Soft bins'

    // Good Hard Bins list (default: 1)
    if(pGoodHardBinsList != NULL)
        delete pGoodHardBinsList;
    pGoodHardBinsList = new GS::QtLib::Range("1");	// Bin 1 is the default for 'good Hard bins'

    // Minimum samples required in a test to check its outliers.
    iMinimumSamples = 50;

    iPatLimitsFromBin   = GEX_TPAT_BUILDLIMITS_GOODSOFTBINS;    // Compute PAT limits from GOOD Bins
    mOptionsTestKey     = GEX_TBPAT_KEY_TESTNUMBER;				// Key is test#
    mTestKey            = GEX_TBPAT_KEY_TESTNUMBER;				// Key is test#

    // Minimum number of outliers to detect in a part to issue a PAT binning
    iMinimumOutliersPerPart = 1;

    bIgnoreIQR0 = true;					// Ignore Static PAT limits for tests with robust sigma = 0
    bStopOnFirstFail = false;			// If set to 'true': only identify first PAT failure in flow.
    lfIgnoreHistoricalCpk = -1;			// 'true' if ignore static PAT for tests with Cpk >= xxx
    // 'true' if ignore static PAT for tests with historical distribution shape = Categories
    bIgnoreHistoricalCategories = false;
    mGPAT_IgnorePPatBins = true;		// Default: Do NOT include PPAT bins into GPAT

    // Report options
    bReport_Stats = true;               // PAT report include Stats for failing tests.
    bReport_Histo = true;               // PAT report include Histogram for failing tests.

    // PAT-71: PAT report include Histogram tests with no outliers
    bReport_Histo_NoOutliers = false;
    bReport_Wafermap = true;            // PAT report include Wafermap
    iReport_WafermapType = 0;           // SBIN=0 or HBIN=1
    bReport_Pareto = true;              // PAT report include Pareto
    bReport_Binning = true;             // PAT report include Binning
    bReport_SPAT_Limits = true;         // PAT report includes SPAT limits computed
    bReport_DPAT_Limits_Outliers = true;	// PAT report lists DPAT limits for tests with outliers

    if (mRecipeType == GS::Gex::PAT::RecipeFinalTest)
        bReport_DPAT_Limits_NoOutliers = true;	// PAT report lists DPAT limits for tests without outlier
    else
        bReport_DPAT_Limits_NoOutliers = false;	// PAT report lists DPAT limits for tests without outlier

    // Do not allow PAT limits to be outside of spec limits.
    if (mRecipeType == GS::Gex::PAT::RecipeFinalTest)
        bStickWithinSpecLimits = false;
    else
        bStickWithinSpecLimits = true;

    // Search outliers in Good parts only (default)
    bScanGoodPartsOnly = true;

    // 'Category' distribution options
    bAssumeIntegerCategory = true;	// Assume integer-only distributions to be categories
    iCategoryValueCount = 5;		// 5 classes or less makes a distribution a 'Category'

    // Default color for Static & Dynamic PAT failure
    cDynamicFailColor = cStaticFailColor = QColor("red");

    // STDF output: as per GUI settings (default)
    iSTDF_Output = GEX_PAT_STDF_OUTPUT_DEFAULT;

    bCustomPatLib = false;			// Default: dsabled custom PAT libs.

    ///////////////////////////////////
    // Wafer sort
    //////////////////////////////////

    // WAFER SORT options
    iUpdateMap_FlowNameID = 0;		// Default Update MAP flow

    // Rule precedence: force to default.
    resetRulePrecedence();

    // NNR
    mNNRIsEnabled = false;					// true if NNR rule enabled
    if (mRecipeType == GS::Gex::PAT::RecipeFinalTest)
        // Soft/HardBin to assign to good parts forced to fail because of NNR algorithm
        mNNRSBin = mNNRHBin = 1522;
    else
        // Soft/HardBin to assign to good parts forced to fail because of NNR algorithm
        mNNRSBin = mNNRHBin = 142;
    // Color for the fail bin due to NNR
    mNNRColor= QColor(255,202,181);
    mNNRRules.clear();

    // IDDQ-Delta
    // true if IDDQ-Delta rule enabled
    mIsIDDQ_Delta_enabled = false;
    if (mRecipeType == GS::Gex::PAT::RecipeFinalTest)
        // Soft/HardBin to assign to good parts forced to fail because of IDDQ-Delta algorithm
        mIDDQ_Delta_SBin = mIDDQ_Delta_HBin = 1523;
    else
        // Soft/HardBin to assign to good parts forced to fail because of IDDQ-Delta algorithm
        mIDDQ_Delta_SBin = mIDDQ_Delta_HBin = 143;
    mIDDQ_Delta_Color= QColor(255,202,181);		// Color for the fail bin due to IDDQ-Delta
    mIDDQ_Delta_Rules.clear();

    // GDBN
    if (mRecipeType == GS::Gex::PAT::RecipeFinalTest)
        // Soft/HardBin to assign to good parts forced to fail because of bad clustere (too many bad dies around)
        mGDBNPatSBin=mGDBNPatHBin=1541;
    else
        // Soft/HardBin to assign to good parts forced to fail because of bad clustere (too many bad dies around)
        mGDBNPatSBin=mGDBNPatHBin=143;

    mGDBNColor = QColor(255,202,181);
    mIsGDBNEnabled = false;			// false if disabled
    mGDBNCustomLib=false;			// Disabled custom PAT lib WaferMap algorithms.
    mGDBNRules.clear();

    // Reticle
    if (mRecipeType == GS::Gex::PAT::RecipeFinalTest)
    {
        mReticleSoftBin = 1543;
        mReticleHardBin = 1543;
    }
    else
    {
        mReticleSoftBin = 144;
        mReticleHardBin = 144;
    }

    mReticleColor   = QColor(255,0,255);
    mReticleEnabled = false;
    mReticleRules.clear();

    mReticleSizeX       = 6;
    mReticleSizeY       = 3;
    mReticleSizeSource  = PATOptionReticle::RETICLE_SIZE_FIXED;

    // Clustering Potato rules
    if (mRecipeType == GS::Gex::PAT::RecipeFinalTest)
    {
        mClusteringPotato       = true;
        mClusteringPotatoSBin   = mClusteringPotatoHBin = 1542;
        mClusteringPotatoColor  = QColor(183, 0, 255);
    }
    else
    {
        mClusteringPotato       = false;
        mClusteringPotatoSBin   = mClusteringPotatoHBin = 145;
        mClusteringPotatoColor  = QColor(255,0,255);
    }

    mClusterPotatoRules.clear();	// Delete Potato rules.

    // Masks
    qDeleteAll(mMaskRules);
    mMaskRules.clear(); // Delete masks

    // Z-PAT
    if(pBadBinsZPAT_List != NULL)
        delete pBadBinsZPAT_List;
    pBadBinsZPAT_List = new GS::QtLib::Range("0;2-65535");	// Default bins for ZPAT
    cZpatColor = QColor(255,0,255);
    bZPAT_SoftBin = true;
    bMergeEtestStdf = false; // True if merge Etest & STDF data (overload STDF data).

    // Failing bin associated with GoodSTDF bin that are rejects in E-test file.
    iCompositeEtestStdf_SBin = iCompositeEtestStdf_HBin = 146;
    lfCompositeExclusionZoneYieldThreshold = 0.0;
    mExclusionZonesEnabled = false;
    if (mRecipeType == GS::Gex::PAT::RecipeFinalTest)
        iCompositeZone_SBin = iCompositeZone_HBin = 1561;
    else
        iCompositeZone_SBin = iCompositeZone_HBin = 148;

    bZPAT_GDBN_Enabled = false;
    bZPAT_Reticle_Enabled = false;
    bZPAT_Clustering_Enabled = false;

    // pat yield alarm
    mEnableYALimit = false;
    mOveralPatYALimit = 0;

    // FINAL TEST Options
    mFT_BaseLine                = FT_PAT_BASELINE_DEFAULT_SIZE;
    mFT_SubsequentBL            = FT_PAT_SUBSEQUENT_BL_DEFAULT_SIZE;
    mFT_BaseLineAlgo            = FT_PAT_MERGED_SITE_ALGO;
    mFT_MinSamplesPerSite       = FT_PAT_MINIMUM_SAMPLES_PER_SITE;
    mFT_RunsPerPacket           = 1;
    mFT_Traceability            = GEX_TPAT_TRACEABILITY_HTML;
    mFT_BaseLineMaxOutliers     = 5;
    mFT_TuningIsEnabled         = true;
    mFT_Tuning                  = FT_PAT_TUNING_ON;
    mFT_TuningType              = FT_PAT_TUNING_EVERY_N_OUTLIERS;
    mFT_TuningSamples           = FT_PAT_TUNING_DEFAULT_SIZE;
    mFT_MaxOutlierParts         = 50;
    mFT_YieldLevel              = 0;
    mFT_CriticalBinsYield       = 100;
    if(mFT_CriticalBinsList != NULL)
        delete mFT_CriticalBinsList;
    mFT_CriticalBinsList        = NULL;             // List of critical bins
    mFT_EmailFormat             = 0;                // HTML format

    // Final test: Alarm timeouts
    // 7103 : default critical (-1) ?
    setProperty(ALARM_SEVERITY_TOO_MUCH_OUTLIERS_IN_BASELINE, QVariant(1) );
    setProperty(ALARM_SEVERITY_DISTRIB_SHAPE_CHANGED, -1);
    setProperty(ALARM_SEVERITY_PAT_DRIFT, -1);
    setProperty(ALARM_SEVERITY_LOW_GOOD_YIELD, -1);
    setProperty(ALARM_SEVERITY_YIELD_LOSS, -1);
    setProperty(ALARM_SEVERITY_PARAM_DRIFT, -1);
    setProperty(ALARM_SEVERITY_PARAM_CPK, -1);
    /*
    iFT_Alarm_Timeout_Outliers=1;
    iFT_Alarm_Timeout_Shapes=1;
    iFT_Alarm_Timeout_PatDrift=1;
    iFT_Alarm_Timeout_LowGoodYield=1;
    iFT_Alarm_Timeout_YieldLoss=1;
    iFT_Alarm_Timeout_ParamDrift=1;
    iFT_Alarm_Timeout_ParamCpk=1;
    */
    // Final test: Alarm emails
    iFT_Alarm_Email_Outliers=0;
    iFT_Alarm_Email_Shapes=0;
    iFT_Alarm_Email_PatDrift=0;
    iFT_Alarm_Email_LowGoodYield=0;
    iFT_Alarm_Email_YieldLoss=0;
    iFT_Alarm_Email_ParamDrift=0;
    iFT_Alarm_Email_ParamCpk=0;

    //////////// SMART MODE options
    // Stop outlier detection in a test as soon as its Cpk is >= to given value
    if (mRecipeType == GS::Gex::PAT::RecipeFinalTest)
    {
        lfSmart_IgnoreHighCpk = -1;
        mSmart_IgnoreHighCpkEnabled = false;
    }
    else
    {
        mSmart_IgnoreHighCpkEnabled = true;
        lfSmart_IgnoreHighCpk = 20;
    }

    // -H*Sigma limit for Gaussian distribution
    lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
    // +T*Sigma limit for Gaussian distribution
    lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
    // -H*Sigma limit for Medium outliers
    lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
    // +T*Sigma limit for Medium outliers
    lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
    // -H*Sigma limit for Far outliers
    lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
    // +T*Sigma limit for Far outliers
    lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;

    // -H*Sigma limit for Gaussian distribution+tail
    lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
    // +T*Sigma limit for Gaussian distribution+tail
    lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_7_SIGMA;
    // -H*Sigma limit for Medium outliers
    lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
    // +T*Sigma limit for Medium outliers
    lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_10_SIGMA;
    // -H*Sigma limit for Far outliers
    lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
    // +T*Sigma limit for Far outliers
    lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_14_SIGMA;

    // -H*Sigma limit for Gaussian distribution+ 2 tails
    lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_7_SIGMA;
    // +T*Sigma limit for Gaussian distribution+ 2 tails
    lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_7_SIGMA;
    // -H*Sigma limit for Medium outliers
    lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_10_SIGMA;
    // +T*Sigma limit for Medium outliers
    lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_10_SIGMA;
    // -H*Sigma limit for Far outliers
    lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_14_SIGMA;
    // +T*Sigma limit for Far outliers
    lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_14_SIGMA;

    // -H*Sigma limit for LogNormal distribution
    lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;
    // +T*Sigma limit for LogNormal distribution
    lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_7_SIGMA;
    // -H*Sigma limit for Medium outliers
    lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_4_SIGMA;
    // +T*Sigma limit for Medium outliers
    lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_10_SIGMA;
    // -H*Sigma limit for Far outliers
    lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_6_SIGMA;
    // +T*Sigma limit for Far outliers
    lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_14_SIGMA;

    // -H*Sigma limit for Multi-Modal distribution
    lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
    // +T*Sigma limit for Multi-Modal distribution
    lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
    // -H*Sigma limit for Medium outliers
    lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
    // +T*Sigma limit for Medium outliers
    lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
    // -H*Sigma limit for Far outliers
    lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
    // +T*Sigma limit for Far outliers
    lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;

    // -H*Sigma limit for Clamped distribution
    lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;
    // +T*Sigma limit for Clamped distribution
    lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
    // -H*Sigma limit for Medium outliers
    lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_4_SIGMA;
    // +T*Sigma limit for Medium outliers
    lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
    // -H*Sigma limit for Far outliers
    lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_6_SIGMA;
    // +T*Sigma limit for Far outliers
    lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;

    // -H*Sigma limit for Double Clamped distribution
    lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;
    // +T*Sigma limit for Double Clamped distribution
    lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;
    // -H*Sigma limit for Medium outliers
    lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_4_SIGMA;
    // +T*Sigma limit for Medium outliers
    lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_4_SIGMA;
    // -H*Sigma limit for Far outliers
    lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_6_SIGMA;
    // +T*Sigma limit for Far outliers
    lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_6_SIGMA;

    // -H*Range limit for Category distribution
    lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
    // +T*Range limit for Category distribution
    lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
    // -H*Sigma limit for Medium outliers
    lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
    // +T*Sigma limit for Medium outliers
    lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
    // -H*Sigma limit for Far outliers
    lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
    // +T*Sigma limit for Far outliers
    lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;

    // -H*Range limit for Unknown distribution
    lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
    // +T*Range limit for Unknown distribution
    lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
    // -H*Sigma limit for Medium outliers
    lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
    // +T*Sigma limit for Medium outliers
    lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
    // -H*Sigma limit for Far outliers
    lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
    // +T*Sigma limit for Far outliers
    lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;

    // // PAT limits Algorithm used
    iAlgo_Gaussian = GEX_TPAT_SMART_ALGO_MEAN;				// Mean +/- N*Sigma
    iAlgo_GaussianTailed = GEX_TPAT_SMART_ALGO_MEAN;		// Mean +/- N*Sigma
    iAlgo_GaussianDoubleTailed = GEX_TPAT_SMART_ALGO_MEAN;	// Mean +/- N*Sigma
    // Mean +/- N*Sigma (could also be Median +/- N*RobustSigma)
    iAlgo_LogNormal = GEX_TPAT_SMART_ALGO_MEAN;
    iAlgo_MultiModal = GEX_TPAT_SMART_ALGO_MEAN;			// Mean +/- N*Sigma
    // Mean +/- N*Sigma (could also be Median +/- N*RobustSigma)
    iAlgo_Clamped = GEX_TPAT_SMART_ALGO_MEAN;
    // Mean +/- N*Sigma (could also be Median +/- N*RobustSigma)
    iAlgo_DoubleClamped = GEX_TPAT_SMART_ALGO_MEAN;
    iAlgo_Category = GEX_TPAT_SMART_ALGO_MEAN;				// Mean +/- N*Sigma
    iAlgo_Unknown = GEX_TPAT_SMART_ALGO_MEAN;				// Mean +/- N*Sigma

    bPAT_Gaussian = true;				// 'false' if disabled PAT on gaussian distributions.
    bPAT_GaussianTailed = true;			// 'false' if disabled PAT on Tailed-gaussian distributions.
    bPAT_GaussianDoubleTailed = true;	// 'false' if disabled PAT on double-tailed-gaussian distributions.
    bPAT_LogNormal = true;				// 'false' if disabled PAT on LogNormal distributions.
    bPAT_MultiModal = true;				// 'false' if disabled PAT on MultiModal distributions.
    bPAT_Clamped = true;				// 'false' if disabled PAT on Clamped distributions.
    bPAT_DoubleClamped = true;			// 'false' if disabled PAT on DoubleClamped distributions.
    bPAT_Category = true;				// 'false' if disabled PAT on Category distributions.
    bPAT_Unknown = true;				// 'false' if disabled PAT on Unknown distributions.

    mRecipeVersion = GS::Gex::PAT::sRecipeVersion;
    mMVPATEnabled                   = false;
    mMVPATAutomaticGroupCreation    = true;
    mMVPATColor                     = QColor("red");
    mMVPATGroupCorrelation          = 0.8;
    mMVPATHardBin                   = 150;
    mMVPATSoftBin                   = 150;
    mMVPATIgnorePPATBins            = true;
    mMVPATNormalShapeOnly           = false;
    mMVPATReportStdCharts           = true;
    mMVPATReportCorrCharts          = false;
    mMVPATReportPCAProjection       = false;
    mMVPATReportPairs               = GS::Gex::PAT::ConsecutivePairs;
    mMVPATReportMaxCharts           = 20;
    mMVPATDistance.insert(GS::Gex::PAT::Near,   6);
    mMVPATDistance.insert(GS::Gex::PAT::Medium, 9);
    mMVPATDistance.insert(GS::Gex::PAT::Far,    12);
    mDefaultDynamicRule = smart_adaptive;

    mAllSitesMerged                 = false;

    mMinConfThreshold = 2; // Min threshold for recognised shapes is 2
}


COptionsPat::GexDefaultRuleSet COptionsPat::GetDefaultDynamicRule() const
{
    return mDefaultDynamicRule;
}

void COptionsPat::SetDefaultDynamicRule(const GexDefaultRuleSet defaultDynamicRule)
{
    mDefaultDynamicRule = defaultDynamicRule;
}


double COptionsPat::GetDefaultFactor() const
{
    return mDefaultFactor;
}

void COptionsPat::SetDefaultFactor(const double defaultFactor)
{
    mDefaultFactor = defaultFactor;
}


bool COptionsPat::GetExclusionZoneEnabled() const
{
    return mExclusionZonesEnabled;
}

void COptionsPat::SetExclusionZoneEnabled(const bool enabled)
{
    mExclusionZonesEnabled = enabled;
}

void COptionsPat::SetRecipeVersion(const QString &version)
{
    mRecipeVersion = version;
}

QString COptionsPat::GetRecipeVersion() const
{
    return mRecipeVersion;
}

void COptionsPat::SetMVPATEnabled(const bool lEnable)
{
    mMVPATEnabled = lEnable;
}

void COptionsPat::SetMVPATIgnorePPATBins(bool lIgnore)
{
    mMVPATIgnorePPATBins = lIgnore;
}

void COptionsPat::SetMVPATAutomaticGroupCreation(bool lAutomatic)
{
    mMVPATAutomaticGroupCreation = lAutomatic;
}

void COptionsPat::SetMVPATGroupCorrelation(double lCorrelation)
{
    mMVPATGroupCorrelation = lCorrelation;
}

void COptionsPat::SetMVPATNormalShapeOnly(bool lNormalOnly)
{
    mMVPATNormalShapeOnly = lNormalOnly;
}

void COptionsPat::SetMVPATSoftBin(int lSoftBin)
{
    mMVPATSoftBin = lSoftBin;
}

void COptionsPat::SetMVPATHardBin(int lHardBin)
{
    mMVPATHardBin = lHardBin;
}

void COptionsPat::SetMVPATColor(const QColor &lColor)
{
    mMVPATColor = lColor;
}

void COptionsPat::SetMVPATDistance(GS::Gex::PAT::OutlierDistance lDistance, double lValue)
{
    mMVPATDistance.insert(lDistance, lValue);
}

void COptionsPat::SetMVPATReportStdCharts(bool lEnabled)
{
    mMVPATReportStdCharts = lEnabled;
}

void COptionsPat::SetMVPATReportCorrCharts(bool lEnabled)
{
    mMVPATReportCorrCharts = lEnabled;
}

void COptionsPat::SetMVPATReportPCAProjection(bool lEnabled)
{
    mMVPATReportPCAProjection = lEnabled;
}

void COptionsPat::SetMVPATReportPairs(GS::Gex::PAT::MVPairs lMode)
{
    mMVPATReportPairs = lMode;
}

void COptionsPat::SetMVPATReportMaxCharts(int lMaxCharts)
{
    mMVPATReportMaxCharts = lMaxCharts;
}

void COptionsPat::SetEnableYALimit(const bool enabled)
{
   mEnableYALimit = enabled;
}

void COptionsPat::SetOveralPatYALimit(const double YALimit)
{
    mOveralPatYALimit = YALimit;
}

void COptionsPat::SetReticleSoftBin(int softBin)
{
    mReticleSoftBin = softBin;
}

void COptionsPat::SetReticleHardBin(int hardBin)
{
    mReticleHardBin = hardBin;
}

void COptionsPat::SetReticleColor(const QColor &color)
{
    mReticleColor = color;
}

void COptionsPat::SetReticleEnabled(bool isEnabled)
{
    mReticleEnabled = isEnabled;
}

void COptionsPat::SetReticleSizeX(int sizeX)
{
    mReticleSizeX = sizeX;
}

void COptionsPat::SetReticleSizeY(int sizeY)
{
    mReticleSizeY = sizeY;
}

void COptionsPat::SetReticleSizeSource(PATOptionReticle::ReticleSizeSource source)
{
    mReticleSizeSource = source;
}

void COptionsPat::SetNNRSoftBin(int softBin)
{
    mNNRSBin = softBin;
}

void COptionsPat::SetNNRHardBin(int hardBin)
{
    mNNRHBin = hardBin;
}

void COptionsPat::SetNNRColor(const QColor &color)
{
    mNNRColor = color;
}

void COptionsPat::SetNNREnabled(bool isEnabled)
{
    mNNRIsEnabled = isEnabled;
}

void COptionsPat::SetAllSitesMerged(bool lAllSitesMerged)
{
    mAllSitesMerged = lAllSitesMerged;
}

bool COptionsPat::GetMVPATEnabled() const
{
    return mMVPATEnabled;
}

bool COptionsPat::GetMVPATIgnorePPATBins() const
{
    return mMVPATIgnorePPATBins;
}

bool COptionsPat::GetMVPATAutomaticGroupCreation() const
{
    return mMVPATAutomaticGroupCreation;
}

double COptionsPat::GetMVPATGroupCorrelation() const
{
    return mMVPATGroupCorrelation;
}

bool COptionsPat::GetMVPATNormalShapeOnly() const
{
    return mMVPATNormalShapeOnly;
}

int COptionsPat::GetMVPATSoftBin() const
{
    return mMVPATSoftBin;
}

int COptionsPat::GetMVPATHardBin() const
{
    return mMVPATHardBin;
}

const QColor &COptionsPat::GetMVPATColor() const
{
    return mMVPATColor;
}

double COptionsPat::GetMVPATDistance(GS::Gex::PAT::OutlierDistance lDistance) const
{
    if (mMVPATDistance.contains(lDistance))
        return mMVPATDistance.value(lDistance);

    return 0.0;
}

bool COptionsPat::GetMVPATReportStdCharts() const
{
    return mMVPATReportStdCharts;
}

bool COptionsPat::GetMVPATReportCorrCharts() const
{
    return mMVPATReportCorrCharts;
}

bool COptionsPat::GetMVPATReportPCAProjection() const
{
    return mMVPATReportPCAProjection;
}

GS::Gex::PAT::MVPairs COptionsPat::GetMVPATReportPairs() const
{
    return mMVPATReportPairs;
}

int COptionsPat::GetMVPATReportMaxCharts() const
{
    return mMVPATReportMaxCharts;
}

bool COptionsPat::GetEnableYALimit() const
{
    return mEnableYALimit;
}
double COptionsPat::GetOveralPatYALimit() const
{
    return mOveralPatYALimit;
}

int COptionsPat::GetReticleSoftBin() const
{
    return mReticleSoftBin;
}

int COptionsPat::GetReticleHardBin() const
{
    return mReticleHardBin;
}

const QColor &COptionsPat::GetReticleColor() const
{
    return mReticleColor;
}

bool COptionsPat::GetReticleEnabled() const
{
    return mReticleEnabled;
}

int COptionsPat::GetReticleSizeX() const
{
    return mReticleSizeX;
}

int COptionsPat::GetReticleSizeY() const
{
    return mReticleSizeY;
}

PATOptionReticle::ReticleSizeSource COptionsPat::GetReticleSizeSource() const
{
    return mReticleSizeSource;
}

int COptionsPat::FindNNRRuleByName(const QString &name) const
{
    int lRuleIndex = -1;

    for (int lIdx = 0; lIdx < mNNRRules.count() && lRuleIndex == -1; ++lIdx)
    {
        if (name.compare(mNNRRules.at(lIdx).GetRuleName(), Qt::CaseInsensitive) == 0)
        {
            lRuleIndex = lIdx;
        }
    }

    return lRuleIndex;
}

int COptionsPat::GetNNRSoftBin() const
{
    return mNNRSBin;
}

int COptionsPat::GetNNRHardBin() const
{
    return mNNRHBin;
}

const QColor &COptionsPat::GetNNRColor() const
{
    return mNNRColor;
}

bool COptionsPat::IsNNREnabled() const
{
    return mNNRIsEnabled;
}

bool COptionsPat::GetAllSitesMerged() const
{
    return mAllSitesMerged;
}


//QString COptionsPat::GetReticleSizeSourceString()
//{
//    if (mReticleSizeSource == RETICLE_SIZE_STDF)
//        return "reticle_size_stdf";
//    else
//        return "reticle_size_fixed";
//}

//COptionsPat::ReticleSizeSource COptionsPat::GetReticleSizeSource()
//{
//    return mReticleSizeSource;
//}

//void COptionsPat::SetReticleSizeSource(const ReticleSizeSource reticleSource)
//{
//    mReticleSizeSource = reticleSource;
//}

//void COptionsPat::SetReticleSizeSource(const QString reticleSource)
//{
//    if (reticleSource == "reticle_size_stdf")
//        mReticleSizeSource = RETICLE_SIZE_STDF;
//    else
//        mReticleSizeSource = RETICLE_SIZE_FIXED;
//}
