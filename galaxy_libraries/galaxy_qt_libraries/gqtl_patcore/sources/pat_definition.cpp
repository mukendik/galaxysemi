#include "pat_definition.h"
#include "pat_options.h"
#include "multi_limit_item.h"
#include "gqtl_log.h"

unsigned CPatDefinition::sTotalNumOfInstances = 0;

QMap<CPatDefinition::TestType, char> CPatDefinition::sTestTypeLegacyMapping;

CPatDefinition::CPatDefinition()
{
    sTotalNumOfInstances++;
    // Reset variables
    clear();
}

CPatDefinition::CPatDefinition(const CPatDefinition & cPatDef)
{
    sTotalNumOfInstances++;

    // Reset variables
    clear();

    // Load with default
    mTailMngtRuleType = cPatDef.mTailMngtRuleType;
    m_lSequenceID = cPatDef.m_lSequenceID;
    m_lfLowLimit = cPatDef.m_lfLowLimit;
    m_lfHighLimit = cPatDef.m_lfHighLimit;
    m_lFailStaticBin = cPatDef.m_lFailStaticBin;
    m_lFailDynamicBin = cPatDef.m_lFailDynamicBin;
    m_lTestNumber = cPatDef.m_lTestNumber;
    mPinIndex = cPatDef.mPinIndex;
    m_strTestName = cPatDef.m_strTestName;
    m_strUnits  = cPatDef.m_strUnits;
    m_llm_scal  = cPatDef.m_llm_scal;
    m_hlm_scal  = cPatDef.m_hlm_scal;
    m_lfMedian  = cPatDef.m_lfMedian;
    m_lfRobustSigma = cPatDef.m_lfRobustSigma;
    m_lfMean = cPatDef.m_lfMean;
    m_lfSigma = cPatDef.m_lfSigma;
    m_lfRange = cPatDef.m_lfRange;
    m_lfLowStaticLimit = cPatDef.m_lfLowStaticLimit;
    m_lfHighStaticLimit = cPatDef.m_lfHighStaticLimit;
    m_iTailDirection = cPatDef.m_iTailDirection;
    m_iDistributionShape = cPatDef.m_iDistributionShape;	// Distribution shape (gaussian, lognormal, etc...)
    mDynamicLimits = cPatDef.mDynamicLimits;
    // Tells if there are type of outliers to keep: none, low values or high values (GEX_TPAT_KEEPTYPE_XXX
    m_OutliersToKeep = cPatDef.m_OutliersToKeep;
    // Tells if consider all data samples in distribution analysis,
    // or ignore some data when computing PAT limits (eg: ignore negative values, etc...)
    m_SamplesToIgnore = cPatDef.m_SamplesToIgnore;
    // Tells the outlier limits set to use: 'near outliers & higher', 'medium & higher' 'far outliers only'
    m_iOutlierLimitsSet = cPatDef.m_iOutlierLimitsSet;
    mOutlierRule = cPatDef.mOutlierRule;	// GEX_TPAT_RULETYPE_XXX)
    mComputedOutlierRule = cPatDef.mComputedOutlierRule;
    // SPAT rule computation (AEC, or other custom rules)
    m_SPATRuleType = cPatDef.m_SPATRuleType;
    // Outlier space (or Head limit). eg: 6 for +/- 6xSigma, etc...
    m_lfOutlierNFactor = cPatDef.m_lfOutlierNFactor;
    // Outlier Tail space: E.g: +9 Sigma.
    m_lfOutlierTFactor = cPatDef.m_lfOutlierTFactor;
    // SPAT: Outlier space (or Head limit). eg: 6 for +/- 6xSigma, etc...
    m_lfSpatOutlierNFactor = cPatDef.m_lfSpatOutlierNFactor;
    // SPAT: Outlier Tail space: E.g: +9 Sigma.
    m_lfSpatOutlierTFactor = cPatDef.m_lfSpatOutlierTFactor;
    // NNR rule type.
    m_iNrrRule = cPatDef.m_iNrrRule;

    // MEdian drift value allowed (Static vs. Dynamic)
    m_SPC_PatMedianDriftAlarm = cPatDef.m_SPC_PatMedianDriftAlarm;
    // MEdian drift value allowed (Static vs. Dynamic)
    m_SPC_PatMedianDriftAlarmUnits = cPatDef.m_SPC_PatMedianDriftAlarmUnits;
    // Test Mean drift limit ( positive value if valid, negative value if disabled)
    m_SPC_TestMeanDriftAlarm = cPatDef.m_SPC_TestMeanDriftAlarm;
    // Test Mean Drift units (% of limits, space, etc....)
    m_SPC_TestMeanDriftAlarmUnits = cPatDef.m_SPC_TestMeanDriftAlarmUnits;
    // Cpk alarm threshold
    m_SPC_CpkAlarm = cPatDef.m_SPC_CpkAlarm;

    // Copy Dynamic limits
    int	iSeverityLimit;
    for(iSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        iSeverityLimit < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        iSeverityLimit++)
    {
        m_lDynamicFailuresLow_AllParts[iSeverityLimit]  = cPatDef.m_lDynamicFailuresLow_AllParts[iSeverityLimit];
        m_lDynamicFailuresHigh_AllParts[iSeverityLimit] = cPatDef.m_lDynamicFailuresHigh_AllParts[iSeverityLimit];
        m_lDynamicFailuresLow[iSeverityLimit]   = cPatDef.m_lDynamicFailuresLow[iSeverityLimit];
        m_lDynamicFailuresHigh[iSeverityLimit]  = cPatDef.m_lDynamicFailuresHigh[iSeverityLimit];
    }

    m_lHistoricalCPK    = cPatDef.m_lHistoricalCPK;

    mTestType = cPatDef.mTestType;

    m_lStaticFailuresLow_AllParts   = cPatDef.m_lStaticFailuresLow_AllParts;
    m_lStaticFailuresHigh_AllParts  = cPatDef.m_lStaticFailuresHigh_AllParts;
    m_TotalFailures_AllParts        = cPatDef.m_TotalFailures_AllParts;
    m_lStaticFailuresLow            = cPatDef.m_lStaticFailuresLow;
    m_lStaticFailuresHigh           = cPatDef.m_lStaticFailuresHigh;
    m_DynamicFailCountPerSite       = cPatDef.m_DynamicFailCountPerSite;
    m_TotalFailures                 = cPatDef.m_TotalFailures;
    m_lSeverityScore                = cPatDef.m_lSeverityScore;
    m_NewPTR_PerSite                = cPatDef.m_NewPTR_PerSite;
    mDPATNote                       = cPatDef.mDPATNote;
    mSPATNote                       = cPatDef.mSPATNote;
}

bool CPatDefinition::SortOnTestNumber(const CPatDefinition *lPatDefLeft, const CPatDefinition *lPatDefRight)
{
    if (lPatDefLeft->m_lTestNumber < lPatDefRight->m_lTestNumber)
    {
        return true;
    }
    else if (lPatDefLeft->m_lTestNumber == lPatDefRight->m_lTestNumber)
    {
        if (lPatDefLeft->mPinIndex < lPatDefRight->mPinIndex)
            return true;
        else if (lPatDefLeft->mPinIndex == lPatDefRight->mPinIndex)
        {
            if (lPatDefLeft->m_strTestName.compare(lPatDefRight->m_strTestName) < 0)
                return true;
        }
    }

    return false;
}

void CPatDefinition::clear(void)
{
    // Test sequence ID in execution flow
    m_lSequenceID = 0;

    // If Fail Static bin is set to -1 in PAT config file, then Test is NOT tested over its STATIC limits
    m_lFailStaticBin = -1;

    // If Fail Dynamic bin is set to -1 in PAT config file, then Test is NOT tested over its DYNAMIC limits
    m_lFailDynamicBin = -1;

    // Test definition (default)
    m_lTestNumber   = 0;
    mPinIndex       = -1;
    m_strTestName.clear();

    // Clear units
    m_strUnits = "";

    // Statitics per test
    int	iSeverityLimit;
    for(iSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        iSeverityLimit < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        iSeverityLimit++)
    {
        // Clear fail count variables
        m_lDynamicFailuresLow_AllParts[iSeverityLimit] = 0;
        m_lDynamicFailuresHigh_AllParts[iSeverityLimit] = 0;
        m_lDynamicFailuresLow[iSeverityLimit] = 0;
        m_lDynamicFailuresHigh[iSeverityLimit] = 0;
    }

    m_lStaticFailuresLow_AllParts = 0;      // For all parts
    m_lStaticFailuresHigh_AllParts = 0;     // For all parts
    m_lStaticFailuresLow    = 0;			// For selected parts (usually Bin#1)
    m_lStaticFailuresHigh   = 0;			// For selected parts (usually Bin#1)
    m_TotalFailures_AllParts = 0;
    m_TotalFailures     = 0;
    m_lSeverityScore    = 0;
    m_DynamicFailCountPerSite.clear();	// Holds DPAT fail count per site.
    m_NewPTR_PerSite.clear();
    mDynamicLimits.clear();

    // No tail direction identified yet
    m_iTailDirection        = GEX_PAT_TAIL_NONE;

    // Distribution shape.
    m_iDistributionShape    = PATMAN_LIB_SHAPE_GAUSSIAN;

    // No limits by default. Specify infinite values.
    m_lfLowStaticLimit  = -GEX_TPAT_DOUBLE_INFINITE;
    m_lfHighStaticLimit = GEX_TPAT_DOUBLE_INFINITE;
    m_lfLowLimit        = -GEX_TPAT_DOUBLE_INFINITE;
    m_lfHighLimit       = GEX_TPAT_DOUBLE_INFINITE;
    m_llm_scal          = 0;
    m_hlm_scal          = 0;

    // SPC variables
    m_SPC_PatMedianDriftAlarm       = -1;	// Drift disabled
    m_SPC_PatMedianDriftAlarmUnits  = GEX_TPAT_DRIFT_UNITS_NONE;
    // Test Mean drift limit ( positive value if valid, negative value if disabled)
    m_SPC_TestMeanDriftAlarm        = -1;
    m_SPC_TestMeanDriftAlarmUnits   = GEX_TPAT_DRIFT_UNITS_NONE;
    m_SPC_CpkAlarm  = 0.0;	// Cpk alarm threshold (default is disabled)

    m_lfMedian      = -1;
    m_lfMean        = -1;
    m_lfSigma       = -1;
    m_lfRange       = -1;
    m_lfRobustSigma = -1;

    m_lHistoricalCPK        = 0;

    m_OutliersToKeep        = GEX_TPAT_KEEPTYPE_NONEID;
    m_SamplesToIgnore       = GEX_TPAT_IGNOREDATA_NONEID;
    m_iOutlierLimitsSet     = GEX_TPAT_LIMITSSET_NEAR;
    // Default Static PAT rule (limits computation): AEC (median +/-6*RobustSigma)
    m_SPATRuleType          = GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA;
    m_lfSpatOutlierNFactor  = 6;
    m_lfSpatOutlierTFactor  = 6;
    mOutlierRule       = GEX_TPAT_RULETYPE_SMARTID;
    mComputedOutlierRule  = -1;
    m_lfOutlierNFactor      = 0;
    m_lfOutlierTFactor      = 0;

    m_iNrrRule              = GEX_TPAT_NNR_ENABLED;

    mSPATNote.clear();
    mDPATNote.clear();

    mTestType   = ParametricTest;

    if(sTestTypeLegacyMapping.isEmpty())
    {
        sTestTypeLegacyMapping.insert(ParametricTest, 'P');
        sTestTypeLegacyMapping.insert(MultiParametricTest,'M');
        sTestTypeLegacyMapping.insert(FunctionalTest,'F');
        sTestTypeLegacyMapping.insert(UnknownTest,' ');
        sTestTypeLegacyMapping.insert(InvalidTest,'I');
    }
    m_llm_scal = 0;
    m_hlm_scal = 0;

    mTailMngtRuleType = GEX_TPAT_TAIL_MNGT_LIBERAL;
}

void CPatDefinition::computeSeverity(void)
{
    // Add counts of lowest Severity: Static PAT failures
    m_lSeverityScore = m_lStaticFailuresLow + m_lStaticFailuresHigh;

    // Add counts of next Severity levels: Dynamic PAT
    int	iSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
    for(iSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        iSeverityLimit < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        iSeverityLimit++)
    {
        m_lSeverityScore += (iSeverityLimit+1)*2*m_lDynamicFailuresLow[iSeverityLimit];
        m_lSeverityScore += (iSeverityLimit+1)*2*m_lDynamicFailuresHigh[iSeverityLimit];
    }
}

QList<int> CPatDefinition::GetDynamicSites() const
{
    return mDynamicLimits.keys();
}

GS::PAT::DynamicLimits CPatDefinition::GetDynamicLimits(int iSite) const
{
    if (mDynamicLimits.contains(iSite) == false && mDynamicLimits.count() == 1 && mDynamicLimits.contains(-1))
        iSite = -1;

    if (mDynamicLimits.contains(iSite))
        return mDynamicLimits.value(iSite);
    else
        return GS::PAT::DynamicLimits();
}

bool CPatDefinition::IsTestDisabled(void)
{
    if( m_lFailStaticBin < 0 &&
        (m_lFailDynamicBin < 0 || mOutlierRule == GEX_TPAT_RULETYPE_IGNOREID) &&
        m_SPC_CpkAlarm <= 0.0 &&
        m_SPC_PatMedianDriftAlarm < 0.0 &&
        m_SPC_TestMeanDriftAlarm < 0.0)
        return true;	// Test totally disabled!
    else
        return false;	// Test needs to be monitored (either PAT enabled and/or SPC monitoring, etc...)
}

bool CPatDefinition::ComputeStaticLimits(const int Samples, const double Mean, const double Median)
{
    // If dynamic data provided with enough samples, use mean/median from dynamic data
    double lMean = m_lfMean;
    double lMedian = m_lfMedian;
    if(Samples >= 33)
    {
        lMean = Mean;
        lMedian = Median;
    }

    // Init SPAT limits with original limits, which will be used if SPAT limits cannot be computed despite
    // them being enabled (ie in case of 0 sigma)
    m_lfLowStaticLimit  = m_lfLowLimit;
    m_lfHighStaticLimit = m_lfHighLimit;

    // If custom SPAT limits defined, compute it now...unless too few samples in this site!
    switch(m_SPATRuleType)
    {
        // AEC standard SPAT: Median +/- 6*RobustSigma rule
        case  GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA:
            if(m_lfRobustSigma != 0.0)
            {
                m_lfLowStaticLimit  = lMedian - 6*m_lfRobustSigma;
                m_lfHighStaticLimit = lMedian + 6*m_lfRobustSigma;
            }
            break;

        // Custom SPAT limits: Mean +/- N*Sigma rule
        case GEX_TPAT_SPAT_ALGO_SIGMA:
            if(m_lfSigma != 0.0)
            {
                // Head and tail ratio are identical
                m_lfLowStaticLimit  = lMean - m_lfSpatOutlierNFactor*m_lfSigma;
                m_lfHighStaticLimit = lMean + m_lfSpatOutlierNFactor*m_lfSigma;
            }
            break;

        // Custom SPAT limits (ignore it if too few samples)
        case GEX_TPAT_SPAT_ALGO_NEWLIMITS:
            m_lfLowStaticLimit     = m_lfSpatOutlierNFactor;
            m_lfHighStaticLimit    = m_lfSpatOutlierTFactor;
            break;

        // Custom SPAT limits: Mean +/-N (Range rule: centered on mean)
        case GEX_TPAT_SPAT_ALGO_RANGE:
            // Compute Mean +/- half range
            m_lfLowStaticLimit     = lMean - m_lfSpatOutlierNFactor;
            m_lfHighStaticLimit    = lMean + m_lfSpatOutlierNFactor;
            break;

        case GEX_TPAT_SPAT_ALGO_IGNORE:	// SPAT disabled
        default:
            // Give infinite limits: Static PAT disabled!
            m_lfLowStaticLimit = -GEX_TPAT_DOUBLE_INFINITE;
            m_lfHighStaticLimit = GEX_TPAT_DOUBLE_INFINITE;
            break;
    }

    return true;
}

bool CPatDefinition::CheckStaticLimits(const COptionsPat & PatOptions)
{
    // Check if SPAT limits exceed test limits...
    // if so (and if option forcing to stick to test limits), do necessary corrections
    if(PatOptions.bStickWithinSpecLimits)
    {
        if((m_lfLowStaticLimit != -GEX_TPAT_DOUBLE_INFINITE)
           && ((m_lfLowStaticLimit < m_lfLowLimit)
               || (m_lfLowStaticLimit > m_lfHighLimit)))
        {
            m_lfLowStaticLimit = m_lfLowLimit;
        }

        if((m_lfHighStaticLimit != GEX_TPAT_DOUBLE_INFINITE)
           && ((m_lfHighStaticLimit > m_lfHighLimit)
               || (m_lfHighStaticLimit < m_lfLowLimit)))
        {
            m_lfHighStaticLimit = m_lfHighLimit;
        }
    }

    return true;
}

void CPatDefinition::SetDynamicLimits(int lSite, GS::PAT::DynamicLimits &lDynLimits, const COptionsPat&  lOptions,
                                         unsigned char lLimitsFlag, double lLowLimit, double lHighLimit)
{
    // If Flag to keep all low outliers, then disable the outlier LOW limit.
    int	lSeverityLimit;
    if(m_OutliersToKeep == GEX_TPAT_KEEPTYPE_LOWID)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Disabling PAT low limit (Outlier to keep option set to low)");
        for(lSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            lSeverityLimit < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
            lSeverityLimit++)
            lDynLimits.mLowDynamicLimit1[lSeverityLimit] = lDynLimits.mLowDynamicLimit2[lSeverityLimit]
                    = -GEX_TPAT_DOUBLE_INFINITE;
    }

    // If Flag to keep all high outliers, then disable the outlier HIGH limit.
    if(m_OutliersToKeep == GEX_TPAT_KEEPTYPE_HIGHID)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Disabling PAT high limit (Outlier to keep option set to high)");
        for(lSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            lSeverityLimit < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
            lSeverityLimit++)
            lDynLimits.mHighDynamicLimit1[lSeverityLimit]
                    = lDynLimits.mHighDynamicLimit2[lSeverityLimit] = GEX_TPAT_DOUBLE_INFINITE;
    }

    // If IQR is 0 (means loads of data are identical),
    // then we may simply ignore PAT on this test (if option is set to do so)
    if(m_lfRobustSigma == 0 && lOptions.bIgnoreIQR0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Disabling PAT limits (Ignore Historical data with IQR == 0)");
        for(lSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            lSeverityLimit < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES; lSeverityLimit++)
        {
            lDynLimits.mLowDynamicLimit1[lSeverityLimit] = lDynLimits.mLowDynamicLimit2[lSeverityLimit]
                    = -GEX_TPAT_DOUBLE_INFINITE;
            lDynLimits.mHighDynamicLimit1[lSeverityLimit] = lDynLimits.mHighDynamicLimit2[lSeverityLimit]
                    = GEX_TPAT_DOUBLE_INFINITE;
        }
    }

    // If PAT limits higher than original limits, then force PAT limits back to original limits!
    if(lOptions.bStickWithinSpecLimits)
    {
        for(lSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            lSeverityLimit < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
            lSeverityLimit++)
        {
            // If low limits exists
            if((lLimitsFlag & CTEST_LIMITFLG_NOLTL) == 0)
            {
                if(lDynLimits.mLowDynamicLimit1[lSeverityLimit] < lLowLimit)
                {
                    // TODO
                    /*
                        GSLOG(SYSLOG_SEV_INFORMATIONAL,
                          QString("DPAT low limits[%1] forced to its original low limit (%2)")
                          .arg(patlib_GetSeverityName(iSeverityLimit))
                          .arg(lLowLimit).toLatin1().constData() );
                          */
                    lDynLimits.mLowDynamicLimit1[lSeverityLimit] = lLowLimit;
                }

                if(lDynLimits.mLowDynamicLimit2[lSeverityLimit] < lLowLimit)
                    lDynLimits.mLowDynamicLimit2[lSeverityLimit] = lLowLimit;
            }

            // If high limits exists
            if((lLimitsFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                if(lDynLimits.mHighDynamicLimit1[lSeverityLimit] > lHighLimit)
                {
                    // TODO
                    /*
                        GSLOG(SYSLOG_SEV_INFORMATIONAL,
                        QString("DPAT high limits[%1] forced to its original high limit (%2)")
                             .arg(patlib_GetSeverityName(iSeverityLimit))
                             .arg(lHighLimit).toLatin1().constData() );
                    */
                    lDynLimits.mHighDynamicLimit1[lSeverityLimit] = lHighLimit;
                }

                if(lDynLimits.mHighDynamicLimit2[lSeverityLimit] > lHighLimit)
                    lDynLimits.mHighDynamicLimit2[lSeverityLimit] = lHighLimit;
            }
        }
    }

    // Update Low & High Dynamic Outlier limits
    for(lSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lSeverityLimit < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        lSeverityLimit++)
    {
        mDynamicLimits[lSite].mLowDynamicLimit1[lSeverityLimit]  = lDynLimits.mLowDynamicLimit1[lSeverityLimit];
        mDynamicLimits[lSite].mHighDynamicLimit1[lSeverityLimit] = lDynLimits.mHighDynamicLimit1[lSeverityLimit];
        mDynamicLimits[lSite].mLowDynamicLimit2[lSeverityLimit]  = lDynLimits.mLowDynamicLimit2[lSeverityLimit];
        mDynamicLimits[lSite].mHighDynamicLimit2[lSeverityLimit] = lDynLimits.mHighDynamicLimit2[lSeverityLimit];
    }

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Near low limit = %1").arg(mDynamicLimits[lSite].mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Near High limit = %1").arg(mDynamicLimits[lSite].mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Medium low limit = %1").arg(mDynamicLimits[lSite].mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM])
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Medium High limit = %1").arg(mDynamicLimits[lSite].mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM])
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Far low limit = %1").arg(mDynamicLimits[lSite].mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR])
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Far High limit = %1").arg(mDynamicLimits[lSite].mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR])
          .toLatin1().constData());

    // Save advanced stats per testing site
    mDynamicLimits[lSite].mDynMean  = lDynLimits.mDynMean;	// Mean value used when computing limits
    mDynamicLimits[lSite].mDynSigma = lDynLimits.mDynSigma;	// Sigma used when computing limits
    mDynamicLimits[lSite].mDynQ1    = lDynLimits.mDynQ1;	// Q1
    mDynamicLimits[lSite].mDynQ2    = lDynLimits.mDynQ2;	// Q2: median
    mDynamicLimits[lSite].mDynQ3    = lDynLimits.mDynQ3;	// Q3

    // Save distribution shape detected for current test# & site#
    mDynamicLimits[lSite].mDistributionShape = lDynLimits.mDistributionShape;
}

CPatDefinition::TestType CPatDefinition::GetTestType() const
{
    return mTestType;
}

void CPatDefinition::SetTestType(const CPatDefinition::TestType &testType)
{
    mTestType = testType;
}

char CPatDefinition::GetTestTypeLegacy() const
{
    return sTestTypeLegacyMapping[mTestType];
}

void CPatDefinition::SetTestTypeLegacy(const char &legacyType)
{
    if(sTestTypeLegacyMapping.keys(legacyType).isEmpty())
        mTestType = InvalidTest;
    else
        mTestType = sTestTypeLegacyMapping.key(legacyType);
}

void CPatDefinition::SetPinmapNumber(long pinmap)
{
    mPinIndex = pinmap;
}
