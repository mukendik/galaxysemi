
#ifdef GCORE15334

#include "gqtl_log.h"
#include "pat_outlier_finder.h"
#include "pat_outlier_finder_private.h"
#include "pat_gdbn_engine.h"
#include "pat_potatoclusterfinder.h"
#include "pat_rules.h"
#include "gex_pat_constants_extern.h"
#include "product_info.h"
#include "sitetestresults.h" // used by GTM only
#include "pat_info.h"
#include "pat_definition.h"
#include "gs_qa_dump.h"
#include "engine.h"
#include <QApplication>


extern double       ScalingPower(int iPower);
extern QString      patlib_GetDistributionName(int iDistributionType);
extern QString      patlib_GetSeverityName(int iSeverityType);
extern int          patlib_GetDistributionType(CTest *ptTestCell,
                                               int lCategoryValueCount = 5,
                                               bool lAssumeIntegerCategory = true,
                                               int aMinConfThreshold = 2,
                                               GS::Gex::PATPartFilter *partFilter = NULL);
extern void         patlib_GetTailCutoff(const CTest*  ptTestCell,
                                         bool    bRight,
                                         double* lfLimit,
                                         CPatDefinition *patDef);
extern int          patlib_getDistributionNormalDirection(const CTest *ptTestCell);
extern int          patlib_getDistributionLogNormalDirection(const CTest *ptTestCell);
extern int          patlib_getDistributionBiModalStats(const CTest *ptTestCell, double &lfMean1,
                                                       double &lfSigma1, double &lfMean2,
                                                       double &lfSigma2, double lfExponent,
                                                       GS::Gex::PATPartFilter * lPartFilter = NULL);
extern int          patlib_getDistributionClampedDirection(const CTest *ptTestCell);

namespace GS
{
namespace Gex
{

PATOutlierFinder::PATOutlierFinder(PATOutlierFinderPrivate &lPrivateData, QObject *parent)
    : QObject(parent), mPrivate(&lPrivateData)
{
}

PATOutlierFinder::~PATOutlierFinder()
{
    if (mPrivate)
    {
        delete mPrivate;
    }
}

bool PATOutlierFinder::ComputePatLimits(const QList<int> & Sites, const bool ComputeSPAT)
{
    PAT_PERF_BENCH

    Q_D(PATOutlierFinder);

    if (d->mInitialized == false)
        return false;

    // Compute Dynamic PAT limits
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
    int lSite = -1;

    // Compute DPAT limits for all testing sites...
    for(int lIdx = 0; lIdx < Sites.count(); lIdx++)
    {
        // Site#
        lSite = Sites.at(lIdx);

        // GS_QA: open dump file & write header
        GS::Gex::GsQaDump lQaDump;
        QString lQaFileShortName;
        if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
            lQaFileShortName = QString("gs_gtm_compute_dpatlimits_s%1.csv").arg(lSite);
        else
            lQaFileShortName = QString("gs_gexpat_compute_dpatlimits_s%1.csv").arg(lSite);
        lQaDump.Open(lQaFileShortName, QIODevice::WriteOnly | QIODevice::Append);
        lQaDump.WriteString(QString("Step,Site#,Test#,PinIndex,TestName,LL,HL,ValidSamples,Mean,Sigma,Cpk,CpkL,CpkH,Q1,Q2,Q3"));
        lQaDump.WriteString(QString(",Kurtosis,Skew,Distribution,Outliers (during DPAT computation, before relaxing limits)"));
        lQaDump.WriteString(QString(",FarLL_1,MediumLL_1,NearLL_1,NearHL_1,MediumHL_1,FarHL_1"));
        lQaDump.WriteString(QString(",FarLL_2,MediumLL_2,NearLL_2,NearHL_2,MediumHL_2,FarHL_2\n"));

        // Force to point relevant group & file (based in Site# to process).
        // As calling function may be final-test (where multiple calls can be done in production)
        // Need to always update pointer, even if addressing same site#.
        if (UpdateDatasetPointer(lSite) == false)
            return false;

        // Compute limits for all tests in a given site#
        for(itPATDefinifion = d->mContext->GetUnivariateRules().begin();
            itPATDefinifion != d->mContext->GetUnivariateRules().end(); ++itPATDefinifion)
        {
            // Update progress bar (for GUI mode)
            if(!GS::LPPlugin::ProductInfo::getInstance()->isGTM())
                Engine::GetInstance().UpdateProgressStatus(false, -1, -1);

            // Compute PAT limits for given test & site.
            if(ComputePatLimits(*itPATDefinifion, lSite, lQaDump, ComputeSPAT) == false)
                return false;
        }

        // GS_QA: close dump file
        lQaDump.Close();
    }

    // Empty the label status
    if(!GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        Engine::GetInstance().UpdateLabelStatus("");

    return true;
}

double PATOutlierFinder::GetRelevantCpk(const CTest * lTest, int lOutliersKept) const
{
    if (lTest)
    {
        switch(lOutliersKept)
        {
            case GEX_TPAT_KEEPTYPE_NONEID:
            default:
                return lTest->GetCurrentLimitItem()->lfCpk;

            case GEX_TPAT_KEEPTYPE_LOWID:	// If keep low outliers, then ignore Low limit for Cpk computation, take CpkHigh
                return lTest->GetCurrentLimitItem()->lfCpkHigh;

            case GEX_TPAT_KEEPTYPE_HIGHID:	// If keep high outliers, then ignore High limit for Cpk computation, take CpkLow
                return lTest->GetCurrentLimitItem()->lfCpkLow;
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Invalid CTest pointer.");
        return GEX_C_DOUBLE_NAN;
    }
}

void PATOutlierFinder::SetAlgorithmValues(int lAlgorithm, const CTest * lTestCell,
                                   GS::PAT::DynamicLimits &lDynLimits) const
{
    switch(lAlgorithm)
    {
        case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            lDynLimits.mMean  = lTestCell->lfMean;
            lDynLimits.mSigma = lTestCell->lfSigma;
            break;

        case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
            // Robust sigma = IQR/1.35
            lDynLimits.mMean  = lTestCell->lfSamplesQuartile2;
            lDynLimits.mSigma = (lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1)/1.35;
            break;

        case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+N*IQR
        case GEX_TPAT_SMART_ALGO_CUSTOMLIB: // Custom PAT library.
            break;
    }
}

void PATOutlierFinder::CheckSPATSpecLimits(CPatDefinition *lPatDef, const CTest *lTestCell)
{
    Q_D(PATOutlierFinder);

    // Check if SPAT limits exceed test limits...
    // if so (and if option forcing to stick to test limits), do necessary corrections
    if(d->mContext->GetRecipeOptions().bStickWithinSpecLimits == true)
    {
        if((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
        {
            // If low limits exists...
            if(lPatDef->m_lfLowStaticLimit < lTestCell->GetCurrentLimitItem()->lfLowLimit
               || lPatDef->m_lfLowStaticLimit > lTestCell->GetCurrentLimitItem()->lfHighLimit)
            {
                lPatDef->m_lfLowStaticLimit = lTestCell->GetCurrentLimitItem()->lfLowLimit;
            }
        }

        if((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
        {
            // If high limits exists...
            if(lPatDef->m_lfHighStaticLimit > lTestCell->GetCurrentLimitItem()->lfHighLimit
               || lPatDef->m_lfHighStaticLimit < lTestCell->GetCurrentLimitItem()->lfLowLimit)
            {
                lPatDef->m_lfHighStaticLimit = lTestCell->GetCurrentLimitItem()->lfHighLimit;
            }
        }
    }
}

bool PATOutlierFinder::ComputePatLimits(CPatDefinition * lPatDef, int lSite, GsQaDump &lQaDump, const bool ComputeSPAT)
{
    Q_D(PATOutlierFinder);

    if (lPatDef == NULL)
        return false;

    // If  PAT is disabled for this test, then do not compute any PAT limits!
    if(lPatDef->m_lFailDynamicBin == -1 && lPatDef->m_lFailStaticBin == -1)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("PPAT disabled for test: %1.%2 %3").arg(lPatDef->m_lTestNumber)
              .arg(lPatDef->mPinIndex).arg(lPatDef->m_strTestName).toLatin1().constData());
        return	true;
    }

    CTest * lTestCell = FindTestCell(lPatDef);

    // Check if test found.
    if(lTestCell == NULL)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("No test found for PAT test: %1.%2 %3").arg(lPatDef->m_lTestNumber)
              .arg(lPatDef->mPinIndex).arg(lPatDef->m_strTestName).toLatin1().constData());

        // disable Outlier identification on this test (since not found in our samples)
        lPatDef->m_lFailStaticBin = lPatDef->m_lFailDynamicBin = -1;
        return true;
    }

    // // Call PreComputeDynamicLimits, so each type of PATEngine can do their internal stuff
    d->PreComputeDynamicLimits(lTestCell);

    // Should we compute SPAT limits?
    if(ComputeSPAT)
    {
        // If custom SPAT limits defined, compute it now...unless too few samples in this site!
        lPatDef->ComputeStaticLimits(lTestCell->ldSamplesValidExecs, lTestCell->lfMean, lTestCell->lfSamplesQuartile2);

        // Check if SPAT limits exceed test limits...
        // If so (and if option forcing to stick to test limits), do necessary corrections
        CheckSPATSpecLimits(lPatDef, lTestCell);
    }

    // If  DPAT is disabled, exit now.
    if(lPatDef->m_lFailDynamicBin == -1)
        return true;

    // Compute Dynamic PAT limit
    switch(lPatDef->mOutlierRule)
    {
        case GEX_TPAT_RULETYPE_SIGMAID:
        case GEX_TPAT_RULETYPE_ROBUSTSIGMAID:
        case GEX_TPAT_RULETYPE_Q1Q3IQRID:
        case GEX_TPAT_RULETYPE_LIMITSID:
        case GEX_TPAT_RULETYPE_NEWLIMITSID:
        case GEX_TPAT_RULETYPE_RANGEID:
        case GEX_TPAT_RULETYPE_SMARTID:
        case GEX_TPAT_RULETYPE_GAUSSIANID:
        case GEX_TPAT_RULETYPE_GAUSSIANTAILID:
        case GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID:
        case GEX_TPAT_RULETYPE_LOGNORMALID:
        case GEX_TPAT_RULETYPE_BIMODALID:
        case GEX_TPAT_RULETYPE_MULTIMODALID:
        case GEX_TPAT_RULETYPE_CLAMPEDID:
        case GEX_TPAT_RULETYPE_DUALCLAMPEDID:
        case GEX_TPAT_RULETYPE_CATEGORYID:
            if (ComputeDynamicLimits(lPatDef, lTestCell, lSite, lQaDump) == false)
                return false;
            break;

        case GEX_TPAT_RULETYPE_CUSTOMLIBID:
            // Call DLL if enabled (otherwise disable test)
            if(d->mContext->GetRecipeOptions().bCustomPatLib)
            {
                // Call Custom PAT lib (plugin)
                double lLowLimit;
                double lHighLimit;

                d->mExternalPat.pat_testlimits(lTestCell->lTestNumber,
                                            lTestCell->lPinmapIndex,
                                            lTestCell->strTestName.toLatin1().constData(),
                                            lSite, lTestCell->m_testResult,
                                            &lLowLimit, &lHighLimit);

                // Save custom DLL test limits into Galaxy structures.
                GS::PAT::DynamicLimits	lDynLimits;	// Temporary structure to store the limits

                for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
                    lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
                    ++lIdx)
                {
                    lDynLimits.mLowDynamicLimit2[lIdx]   = lDynLimits.mLowDynamicLimit1[lIdx]     = lLowLimit;
                    lDynLimits.mHighDynamicLimit2[lIdx]  = lDynLimits.mHighDynamicLimit1[lIdx]    = lHighLimit;
                }

                // Saves dynamic PAT limits computed for this testing site + save distribution shape detected
                lDynLimits.mDistributionShape = patlib_GetDistributionType(lTestCell,
                                                                           d->mContext->GetRecipeOptions().iCategoryValueCount,
                                                                           d->mContext->GetRecipeOptions().bAssumeIntegerCategory,
                                                                           d->mContext->GetRecipeOptions().mMinConfThreshold,
                                                                           d->mPartFilter);

                lPatDef->SetDynamicLimits(lSite, lDynLimits, d->mContext->GetRecipeOptions(),
                                             lTestCell->GetCurrentLimitItem()->bLimitFlag, lTestCell->GetCurrentLimitItem()->lfLowLimit,
                                             lTestCell->GetCurrentLimitItem()->lfHighLimit);

                // Check if SPAT limits exceed test limits...if so (and if option forcing to stick to test limits), do necessary corrections
                CheckSPATSpecLimits(lPatDef, lTestCell);
            }
            else
                // Custom DLL algo selected, but no DLL: disable Outlier identification.
                lPatDef->m_lFailStaticBin = lPatDef->m_lFailDynamicBin = -1;

            break;

        case GEX_TPAT_RULETYPE_IGNOREID:
        default:
            // disable Outlier identification.
            lPatDef->m_lFailStaticBin = lPatDef->m_lFailDynamicBin = -1;
            break;
    }

    // Call PostComputeDynamicLimits, so each type of PATEngine can do their internal stuff
    d->PostComputeDynamicLimits(lTestCell);

    return true;
}

bool PATOutlierFinder::ComputeDynamicLimits(CPatDefinition * lPatDef, CTest * lTestCell, int lSite, GsQaDump &lQaDump)
{
    Q_D(PATOutlierFinder);
    // Remove outliers 1 by 1, but not necessairly the farest, because on asymetric
    // distributions with asymetric limits, the farest may not be an outlier, while the extreme on the other
    // side is one. Start with the farest extreme, and if it is not an outlier, check the other extreme.
    // Also, if the 2 extremes are not outliers, no need to go through the testlist.

    bool lForceNewRule =
        (QString(qgetenv("GS_SHAPEDETECTION_LEGACY")) != "1") ? true : false;

    QString lString;
    if(!LPPlugin::ProductInfo::getInstance()->isGTM())
    {
        // Status bar message.
        lString = " Computing PAT limits: site#";
        lString += QString::number(lSite);
        lString += " Test#";
        lString += QString::number(lPatDef->m_lTestNumber);

        if (lPatDef->mPinIndex >= 0)
            lString += "." + QString::number(lPatDef->mPinIndex);

        Engine::GetInstance().UpdateLabelStatus(lString);

        // Force to process events:
        // * to refresh the gui when in GUI mode
        // * to catch stop signal in daemon mode
        QCoreApplication::processEvents();
    }

    int     lIterationCount = 0;
    double	lTestCpk;
    bool	lHighCpk        = false;
    double	lExponent       = ScalingPower(lTestCell->res_scal);
    int		lOutlierCount   = 0;
    GS::PAT::DynamicLimits	lDynLimits;	// Temporary structure to store the two sets of limits to be created
    lPatDef->mComputedOutlierRule = -1; // reset computed outlier rule because can be different for this site

    // GS_QA: write BEFORE_COMPUTE_DPAT data
    int     lPrecision=4;
    QString lLowLimit = (lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) ?
                QString("n/a") : QString("%1").arg(lTestCell->GetCurrentLimitItem()->lfLowLimit,0,'e',lPrecision);
    QString lHighLimit = (lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) ?
                QString("n/a") : QString("%1").arg(lTestCell->GetCurrentLimitItem()->lfHighLimit,0,'e',lPrecision);
    lQaDump.WriteString(QString("BEFORE_COMPUTE_DPAT,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15")
                        .arg(lSite).arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex)
                        .arg(lTestCell->strTestName)
                        .arg(lLowLimit).arg(lHighLimit)
                        .arg(lTestCell->ldSamplesValidExecs).arg(lTestCell->lfMean,0,'e',lPrecision)
                        .arg(lTestCell->lfSigma,0,'e',lPrecision).arg(lTestCell->GetCurrentLimitItem()->lfCpk,0,'e',lPrecision)
                        .arg(lTestCell->GetCurrentLimitItem()->lfCpkLow,0,'e',lPrecision).arg(lTestCell->GetCurrentLimitItem()->lfCpkHigh,0,'e',lPrecision)
                        .arg(lTestCell->lfSamplesQuartile1,0,'e',lPrecision)
                        .arg(lTestCell->lfSamplesQuartile2,0,'e',lPrecision)
                        .arg(lTestCell->lfSamplesQuartile3,0,'e',lPrecision));
    lQaDump.WriteString(QString(",%1,%2,%3,%4")
                        .arg(lTestCell->lfSamplesKurt,0,'e',lPrecision).arg(lTestCell->lfSamplesSkew,0,'e',lPrecision)
                        .arg(patlib_GetDistributionName(
                                 patlib_GetDistributionType(lTestCell,
                                                            d->mContext->GetRecipeOptions().iCategoryValueCount,
                                                            d->mContext->GetRecipeOptions().bAssumeIntegerCategory,
                                                            d->mContext->GetRecipeOptions().mMinConfThreshold,
                                                            d->mPartFilter)))
                        .arg(0));
    lQaDump.WriteString(QString(",n/a,n/a,n/a,n/a,n/a,n/a"));
    lQaDump.WriteString(QString(",n/a,n/a,n/a,n/a,n/a,n/a\n"));

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Computing DPAT limits on Test %1.%2 Site %3")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).arg(lSite).toLatin1().constData());
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Initial distribution shape: %1")
          .arg(patlib_GetDistributionName(
                   patlib_GetDistributionType(lTestCell,
                                              d->mContext->GetRecipeOptions().iCategoryValueCount,
                                              d->mContext->GetRecipeOptions().bAssumeIntegerCategory,
                                              d->mContext->GetRecipeOptions().mMinConfThreshold,
                                              d->mPartFilter)))
          .toLatin1().constData());

    // Check if we have to ignore some data samples...if so, remove such data points so we don't have incorrect Sigma and Mean!
    if (ApplySamplesFiltering(lPatDef, lTestCell) == false)
        return false;

    // GS_QA: write AFTER_FILTER_SAMPLES data
    lLowLimit = (lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) ?
                QString("n/a") : QString("%1").arg(lTestCell->GetCurrentLimitItem()->lfLowLimit,0,'e',lPrecision);
    lHighLimit = (lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) ?
                QString("n/a") : QString("%1").arg(lTestCell->GetCurrentLimitItem()->lfHighLimit,0,'e',lPrecision);
    lQaDump.WriteString(QString("AFTER_FILTER_SAMPLES,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15")
                        .arg(lSite).arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex)
                        .arg(lTestCell->strTestName)
                        .arg(lLowLimit).arg(lHighLimit)
                        .arg(lTestCell->ldSamplesValidExecs).arg(lTestCell->lfMean,0,'e',lPrecision)
                        .arg(lTestCell->lfSigma,0,'e',lPrecision).arg(lTestCell->GetCurrentLimitItem()->lfCpk,0,'e',lPrecision)
                        .arg(lTestCell->GetCurrentLimitItem()->lfCpkLow,0,'e',lPrecision).arg(lTestCell->GetCurrentLimitItem()->lfCpkHigh,0,'e',lPrecision)
                        .arg(lTestCell->lfSamplesQuartile1,0,'e',lPrecision)
                        .arg(lTestCell->lfSamplesQuartile2,0,'e',lPrecision)
                        .arg(lTestCell->lfSamplesQuartile3,0,'e',lPrecision));
    lQaDump.WriteString(QString(",%1,%2,%3,%4")
                        .arg(lTestCell->lfSamplesKurt,0,'e',lPrecision).arg(lTestCell->lfSamplesSkew,0,'e',lPrecision)
                        .arg(patlib_GetDistributionName(
                                 patlib_GetDistributionType(lTestCell,
                                                            d->mContext->GetRecipeOptions().iCategoryValueCount,
                                                            d->mContext->GetRecipeOptions().bAssumeIntegerCategory,
                                                            d->mContext->GetRecipeOptions().mMinConfThreshold,
                                                            d->mPartFilter)))
                        .arg(0));
    lQaDump.WriteString(QString(",n/a,n/a,n/a,n/a,n/a,n/a"));
    lQaDump.WriteString(QString(",n/a,n/a,n/a,n/a,n/a,n/a\n"));

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Minimum samples required: %1").arg(d->GetRequiredSamples())
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Valid samples found: %1").arg(lTestCell->ldSamplesValidExecs)
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Samples start after noise: %1").arg(lTestCell->lfSamplesStartAfterNoise)
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Samples end before noise: %1").arg(lTestCell->lfSamplesEndBeforeNoise)
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Mean: %1").arg(lTestCell->lfMean).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Sigma: %1").arg(lTestCell->lfSigma).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Cpk: %1").arg(lTestCell->GetCurrentLimitItem()->lfCpk).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Cpk Low: %1").arg(lTestCell->GetCurrentLimitItem()->lfCpkLow).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Cpk High: %1").arg(lTestCell->GetCurrentLimitItem()->lfCpkHigh).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Kurtosis: %1").arg(lTestCell->lfSamplesKurt).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Skew: %1").arg(lTestCell->lfSamplesSkew).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Q1: %1").arg(lTestCell->lfSamplesQuartile1).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Q2: %1").arg(lTestCell->lfSamplesQuartile2).toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG, QString("Q3: %1").arg(lTestCell->lfSamplesQuartile3).toLatin1().constData());

    // Check if enough samples to compute limits.
    if(d->HasRequiredSamples(*lTestCell) == false)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Unable to compute limits for test %1.%2 (Valid samples (%3) < Required samples (%4))")
              .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex)
              .arg(lTestCell->ldSamplesValidExecs)
              .arg(d->GetRequiredSamples()).toLatin1().constData());
        goto save_dyn_limits;	// No data for this test!
    }

    // Check which Cpk to use...CpkH, CpkL, or min(CpkH,CpkL): depends if outliers on one side to ignore (in such case, ignore limit of that side too!)
    lTestCpk = GetRelevantCpk(lTestCell, lPatDef->m_OutliersToKeep);

    if (d->mContext->GetRecipeOptions().mSmart_IgnoreHighCpkEnabled)
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("High Cpk limit: %1").arg(d->mContext->GetRecipeOptions().lfSmart_IgnoreHighCpk)
              .toLatin1().constData());
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, "High Cpk limit disabled");
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Cpk computed: %1").arg(lTestCpk).toLatin1().constData());

    // Leave infinite limits if option to ignore test with very high Cpk is set and Cpk above limit.
    if((d->mContext->GetRecipeOptions().mSmart_IgnoreHighCpkEnabled == false) || d->mContext->GetRecipeOptions().lfSmart_IgnoreHighCpk < 0 ||
        (fabs(lTestCpk) < d->mContext->GetRecipeOptions().lfSmart_IgnoreHighCpk) ||
        (lTestCell->GetCurrentLimitItem()->lfCpk == C_NO_CP_CPK))
    {
        // Compute low levels stats (so to find the Min & Max cell offset)
        d->mStatsEngine.ComputeLowLevelTestStatistics(lTestCell, lExponent, d->mPartFilter);

        // Update PAT limits, depending on rule selected.
        if (UpdateDynamicLimits(lPatDef, lTestCell, lExponent, lDynLimits,lForceNewRule) == false)
            return false;

        // Loop until all outliers removed, then compute the N*Sigma.
        double          lOutlierValue   = GEX_C_DOUBLE_NAN;
        bool            lOutlierFound   = false;
        QElapsedTimer	lTimer;

        // Starts timer in order to refresh the GUI
        lTimer.start();

        do
        {
            GSLOG(SYSLOG_SEV_DEBUG,
                  QString("Iteration# %1").arg(++lIterationCount).toLatin1().constData());

            // Reset outlier flag
            lOutlierFound = false;

            // Check if more than 1 second elapsed since last call to GUI.
            if(lTimer.elapsed() > 1000)
            {
                QCoreApplication::processEvents();

                // Get current date
                lTimer.start();
            }

            lOutlierValue = FindOutlierValue(lTestCell, lDynLimits);

            // Remove all data samples matching the OutlierValue to remove.
            if(lOutlierValue != GEX_C_DOUBLE_NAN)
            {
                lOutlierFound = true;
                // keep track of shape reset by m_testResult.invalidateResultAt
                int lDistrib = lTestCell->GetDistribution();
                // Go through testlist and remove samples equal to outlier value
                for(int lIdx = 0; lIdx < lTestCell->m_testResult.count(); ++lIdx)
                {
                    if (mPrivate->mPartFilter == NULL ||
                        mPrivate->mPartFilter->IsFiltered(lIdx))
                    {
                        // Check if sample equal to outlier to remove.
                        if(lTestCell->m_testResult.resultAt(lIdx) == lOutlierValue)
                        {
                            // Keep track of total outliers removed
                            lOutlierCount++;

                            // Outlier data...need to remove it: insert a NaN
                            lTestCell->m_testResult.invalidateResultAt(lIdx);

                            // Fewer samples to scan
                            if(lTestCell->ldSamplesValidExecs > 0)
                                lTestCell->ldSamplesValidExecs--;
                        }
                    }
                }
                // re-use identified shape, only if new shape detection because we rely on it
                if (lForceNewRule)
                    lTestCell->SetDistribution(lDistrib);


                GSLOG(SYSLOG_SEV_INFORMATIONAL,
                      QString("Outliers detected (%1)").arg(lOutlierCount).toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Outlier value: %1").arg(lOutlierValue).toLatin1().constData());

                // Recompute stats (sigma)...
                // Compute & update statistics (min, max, SumX, SumX2, sigma, mean, quartiles, etc...)
                d->ComputeTestStatistics(lTestCell, d->mPartFilter);

                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Valid samples remaining: %1").arg(lTestCell->ldSamplesValidExecs)
                      .toLatin1().constData());

                // Check which Cpk to use...CpkH, CpkL, or min(CpkH,CpkL): depends if outliers
                // on one side to ignore (in such case, ignore limit of that side too!)
                lTestCpk = GetRelevantCpk(lTestCell, lPatDef->m_OutliersToKeep);

                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Cpk computed: %1").arg(lTestCpk).toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Kurtosis computed: %1").arg(lTestCell->lfSamplesKurt)
                      .toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Skew computed: %1").arg(lTestCell->lfSamplesSkew)
                      .toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Skew without noise computed: %1")
                      .arg(lTestCell->lfSamplesSkewWithoutNoise).toLatin1().constData());

                // Check if option to ignore test with very high Cpk is set, and Cpk above the specified limit.
                if(( d->mContext->GetRecipeOptions().mSmart_IgnoreHighCpkEnabled && d->mContext->GetRecipeOptions().lfSmart_IgnoreHighCpk >= 0) &&
                    (fabs(lTestCpk) >= d->mContext->GetRecipeOptions().lfSmart_IgnoreHighCpk) &&
                    (lTestCell->GetCurrentLimitItem()->lfCpk != C_NO_CP_CPK))
                {
                    lHighCpk = true;
                }
                else
                {
                    // Recompute outlier limits based on new sigma
                    if (UpdateDynamicLimits(lPatDef, lTestCell, lExponent, lDynLimits) == false)
                        return false;
                }
            }
            else
                GSLOG(SYSLOG_SEV_INFORMATIONAL, "No outliers detected");

            // Keep looping until no more outliers!
        }
        while(lOutlierFound && !lHighCpk && (lTestCell->ldSamplesValidExecs > 0));

        // No more sigma, then update the outlier limits with the last values identified (Normalized values)
        for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
            ++lIdx)
        {
            lDynLimits.mLowDynamicLimit1[lIdx]  *= lExponent;
            lDynLimits.mHighDynamicLimit1[lIdx] *= lExponent;
            lDynLimits.mLowDynamicLimit2[lIdx]  *= lExponent;
            lDynLimits.mHighDynamicLimit2[lIdx] *= lExponent;
        }
    }
    else
    {
        if(lTestCell->GetCurrentLimitItem()->lfCpk == C_NO_CP_CPK)
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "No Outliers detection: No cpk");
        }
        else
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("No Outliers detection: Cpk (%1) exceed the cpk limit (%2)").arg(lTestCpk)
                  .arg(d->mContext->GetRecipeOptions().lfSmart_IgnoreHighCpk).toLatin1().constData());
        }
    }

    // If high Cpk...
    if(lHighCpk)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("High Cpk detected (%1)").arg(lTestCpk).toLatin1().constData());

        d->OnHighCPKDetected(*lPatDef, *lTestCell, lDynLimits);
    }

save_dyn_limits:

    // Relax limits if tails detected (doesn't apply to bi-modal distributions)
    if(lPatDef->m_iTailDirection != GEX_PAT_TAIL_NONE)
    {
        for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
            ++lIdx)
        {
            // GCORE-4585: Always apppy relaxed limits if computed.
//            if(lPatDef->m_iTailDirection & GEX_PAT_TAIL_RIGHT)
//            {
                if (lDynLimits.mHighRelaxed > lDynLimits.mHighDynamicLimit1[lIdx])
                {
                    GSLOG(SYSLOG_SEV_INFORMATIONAL,
                          QString("DPAT high limit[%1] relaxed to %2").arg(patlib_GetSeverityName(lIdx))
                          .arg(lDynLimits.mHighRelaxed).toLatin1().constData());
                }
                lDynLimits.mHighDynamicLimit2[lIdx]  = gex_max(lDynLimits.mHighDynamicLimit1[lIdx],
                                                                  lDynLimits.mHighRelaxed);
                lDynLimits.mHighDynamicLimit1[lIdx]  = lDynLimits.mHighDynamicLimit2[lIdx];
//            }
//            if(lPatDef->m_iTailDirection & GEX_PAT_TAIL_LEFT)
//            {
                if (lDynLimits.mLowRelaxed < lDynLimits.mLowDynamicLimit1[lIdx])
                {
                    GSLOG(SYSLOG_SEV_INFORMATIONAL,
                          QString("DPAT low limit[%1] relaxed to %2").arg(patlib_GetSeverityName(lIdx))
                          .arg(lDynLimits.mLowRelaxed).toLatin1().constData());
                }
                lDynLimits.mLowDynamicLimit2[lIdx]   = qMin(lDynLimits.mLowDynamicLimit1[lIdx],
                                                               lDynLimits.mLowRelaxed);
                lDynLimits.mLowDynamicLimit1[lIdx]   = lDynLimits.mLowDynamicLimit2[lIdx];
//            }
        }
    }

    // Save advanced statisics for this site
    lDynLimits.mDynMean  = lTestCell->lfMean; // Mean value used when computing limits
    lDynLimits.mDynSigma = lTestCell->lfSigma;	// Sigma  used when computing limits
    lDynLimits.mDynQ1    = lTestCell->lfSamplesQuartile1;	// Q1
    lDynLimits.mDynQ2    = lTestCell->lfSamplesQuartile2;	// Q2: median
    lDynLimits.mDynQ3    = lTestCell->lfSamplesQuartile3;	// Q3

    // Saves dynamic PAT limits computed for this testing site + save distribution shape detected
    lDynLimits.mDistributionShape = patlib_GetDistributionType(
                lTestCell,
                d->mContext->GetRecipeOptions().iCategoryValueCount,
                d->mContext->GetRecipeOptions().bAssumeIntegerCategory,
                d->mContext->GetRecipeOptions().mMinConfThreshold,
                d->mPartFilter);

    lPatDef->SetDynamicLimits(lSite, lDynLimits, d->mContext->GetRecipeOptions(),
                                 lTestCell->GetCurrentLimitItem()->bLimitFlag, lTestCell->GetCurrentLimitItem()->lfLowLimit,
                                 lTestCell->GetCurrentLimitItem()->lfHighLimit);

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Final distribution shape for test %1.%2: %3").arg(lTestCell->lTestNumber)
          .arg(lTestCell->lPinmapIndex)
          .arg(patlib_GetDistributionName(lPatDef->mDynamicLimits[lSite].mDistributionShape))
          .toLatin1().constData());

    // GS_QA: write AFTER_COMPUTE_DPAT data
    lLowLimit = (lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) ?
                QString("n/a") : QString("%1").arg(lTestCell->GetCurrentLimitItem()->lfLowLimit,0,'e',lPrecision);
    lHighLimit = (lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) ?
                QString("n/a") : QString("%1").arg(lTestCell->GetCurrentLimitItem()->lfHighLimit,0,'e',lPrecision);
    lQaDump.WriteString(QString("AFTER_COMPUTE_DPAT,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15")
                        .arg(lSite).arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex)
                        .arg(lTestCell->strTestName)
                        .arg(lLowLimit).arg(lHighLimit)
                        .arg(lTestCell->ldSamplesValidExecs).arg(lTestCell->lfMean,0,'e',lPrecision)
                        .arg(lTestCell->lfSigma,0,'e',lPrecision).arg(lTestCell->GetCurrentLimitItem()->lfCpk,0,'e',lPrecision)
                        .arg(lTestCell->GetCurrentLimitItem()->lfCpkLow,0,'e',lPrecision).arg(lTestCell->GetCurrentLimitItem()->lfCpkHigh,0,'e',lPrecision)
                        .arg(lTestCell->lfSamplesQuartile1,0,'e',lPrecision)
                        .arg(lTestCell->lfSamplesQuartile2,0,'e',lPrecision)
                        .arg(lTestCell->lfSamplesQuartile3,0,'e',lPrecision));
    lQaDump.WriteString(QString(",%1,%2,%3,%4")
                        .arg(lTestCell->lfSamplesKurt,0,'e',lPrecision).arg(lTestCell->lfSamplesSkew,0,'e',lPrecision)
                        .arg(patlib_GetDistributionName(lPatDef->mDynamicLimits[lSite].mDistributionShape))
                        .arg(lOutlierCount));
    lQaDump.WriteString(QString(",%1,%2,%3,%4,%5,%6")
                        .arg(lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR],0,'e',lPrecision)
                        .arg(lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM],0,'e',lPrecision)
                        .arg(lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],0,'e',lPrecision)
                        .arg(lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],0,'e',lPrecision)
                        .arg(lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM],0,'e',lPrecision)
                        .arg(lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR],0,'e',lPrecision));
    lQaDump.WriteString(QString(",%1,%2,%3,%4,%5,%6\n")
                        .arg(lDynLimits.mLowDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR],0,'e',lPrecision)
                        .arg(lDynLimits.mLowDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM],0,'e',lPrecision)
                        .arg(lDynLimits.mLowDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],0,'e',lPrecision)
                        .arg(lDynLimits.mHighDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],0,'e',lPrecision)
                        .arg(lDynLimits.mHighDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM],0,'e',lPrecision)
                        .arg(lDynLimits.mHighDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR],0,'e',lPrecision));

    return true;
}

bool PATOutlierFinder::UpdateDynamicLimits(CPatDefinition * lPatDef, CTest * lTestCell,
                                    double lExponent, GS::PAT::DynamicLimits &lDynLimits, bool forceNewRule) const
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("DPAT outlier rule: %1").arg(gexRuleSetItemsGUI[lPatDef->mOutlierRule])
          .toLatin1().constData());

    // Compute limits (according to rule set)
    switch(lPatDef->mOutlierRule)
    {
        case GEX_TPAT_RULETYPE_SIGMAID:
            UpdateSigmaLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_ROBUSTSIGMAID:
            UpdateRobustSigmaLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_Q1Q3IQRID:
            UpdateQ1Q3IQRSigmaLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_LIMITSID:
            UpdateLimitPctLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_NEWLIMITSID:
            UpdateForceLimitsPctLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_RANGEID:
            UpdateRangeLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_SMARTID:
        case GEX_TPAT_RULETYPE_GAUSSIANID:
        case GEX_TPAT_RULETYPE_GAUSSIANTAILID:
        case GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID:
        case GEX_TPAT_RULETYPE_LOGNORMALID:
        case GEX_TPAT_RULETYPE_BIMODALID:
        case GEX_TPAT_RULETYPE_MULTIMODALID:
        case GEX_TPAT_RULETYPE_CLAMPEDID:
        case GEX_TPAT_RULETYPE_DUALCLAMPEDID:
        case GEX_TPAT_RULETYPE_CATEGORYID:
            if (UpdateSmartLimits(lPatDef, lTestCell, lDynLimits, forceNewRule) == false)
                return false ;
            break;

        case GEX_TPAT_RULETYPE_CUSTOMLIBID:
        default:
            break;
    }

    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        lDynLimits.mHighDynamicLimit1[lIdx] /= lExponent;
        lDynLimits.mLowDynamicLimit1[lIdx]  /= lExponent;
        lDynLimits.mHighDynamicLimit2[lIdx] /= lExponent;
        lDynLimits.mLowDynamicLimit2[lIdx]  /= lExponent;
    }

    // Save advanced statisics for this site
    lDynLimits.mDynMean  = lTestCell->lfMean;                // Mean value used when computing limits
    lDynLimits.mDynSigma = lTestCell->lfSigma;               // Sigma  used when computing limits
    lDynLimits.mDynQ1    = lTestCell->lfSamplesQuartile1;	// Q1
    lDynLimits.mDynQ2    = lTestCell->lfSamplesQuartile2;	// Q2: median
    lDynLimits.mDynQ3    = lTestCell->lfSamplesQuartile3;	// Q3

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Near low limit = %1")
          .arg(lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Near High limit = %1")
          .arg(lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Medium low limit = %1")
          .arg(lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM])
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Medium High limit = %1")
          .arg(lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM])
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Far low limit = %1")
          .arg(lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR])
          .toLatin1().constData());
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Far High limit = %1")
          .arg(lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR])
          .toLatin1().constData());

    if (lPatDef->mComputedOutlierRule == GEX_TPAT_RULETYPE_BIMODALID)
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Near low limit mode 2 = %1")
              .arg(lDynLimits.mLowDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])
              .toLatin1().constData());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Near High limit mode 2 = %1")
              .arg(lDynLimits.mHighDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])
              .toLatin1().constData());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Medium low limit mode 2 = %1")
              .arg(lDynLimits.mLowDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM])
              .toLatin1().constData());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Medium High limit mode 2 = %1")
              .arg(lDynLimits.mHighDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM])
              .toLatin1().constData());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Far low limit mode 2 = %1")
              .arg(lDynLimits.mLowDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR])
              .toLatin1().constData());
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Far High limit mode 2 = %1")
              .arg(lDynLimits.mHighDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR])
              .toLatin1().constData());
    }

    return true;
}

void PATOutlierFinder::UpdateForceLimitsPctLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                           GS::PAT::DynamicLimits &lDynLimits) const
{
    lDynLimits.mMean = lTestCell->lfMean;
    lDynLimits.mSigma= lTestCell->lfSigma;

    // Set custom limits
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        lDynLimits.mLowDynamicLimit1[lIdx]  = lPatDef->m_lfOutlierNFactor;
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
        lDynLimits.mHighDynamicLimit1[lIdx] = lPatDef->m_lfOutlierTFactor;
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

double PATOutlierFinder::FindOutlierValue(const CTest *lTest, const GS::PAT::DynamicLimits &lDynLimits) const
{
    double lOutlierValue = GEX_C_DOUBLE_NAN;

    if (lTest == NULL)
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Invalid Test pointer given in arguments.");
    }
    else if (lTest->ldSamplesValidExecs > 0)
    {
        double	lFarestExtreme  = GEX_C_DOUBLE_NAN;	// Holds farest from mean betweem min and max
        double	lNearestExtreme = GEX_C_DOUBLE_NAN;	// Holds nearest from mean betweem min and max

        // Get farest and nearest extremes
        if(fabs(lTest->lfMin - lDynLimits.mMean) > fabs(lTest->lfMax - lDynLimits.mMean))
        {
            if(lTest->m_testResult.isValidIndex(lTest->lSamplesMinCell))
                lFarestExtreme = lTest->m_testResult.resultAt(lTest->lSamplesMinCell);	// Get 'Min' value from the data samples array
            else
                GSLOG(SYSLOG_SEV_CRITICAL, "Invalid run index for samples min");

            if(lTest->m_testResult.isValidIndex(lTest->lSamplesMaxCell))
                lNearestExtreme = lTest->m_testResult.resultAt(lTest->lSamplesMaxCell);	// Get 'Max' value from the data samples array
            else
                GSLOG(SYSLOG_SEV_CRITICAL, "Invalid run index for samples max");
        }
        else
        {
            if(lTest->m_testResult.isValidIndex(lTest->lSamplesMaxCell))
                lFarestExtreme = lTest->m_testResult.resultAt(lTest->lSamplesMaxCell);	// Get 'Max' value from the data samples array
            else
                GSLOG(SYSLOG_SEV_CRITICAL, "Invalid run index for samples max");

            if(lTest->m_testResult.isValidIndex(lTest->lSamplesMinCell))
                lNearestExtreme = lTest->m_testResult.resultAt(lTest->lSamplesMinCell);	// Get 'Min' value from the data samples array
            else
                GSLOG(SYSLOG_SEV_CRITICAL, "Invalid run index for samples min");
        }

        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Farest points found: [%1] [%2]").arg(lFarestExtreme).arg(lNearestExtreme)
              .toLatin1().constData());

        // Check if farest extreme is an outlier
        if(((lFarestExtreme < lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]) ||
            (lFarestExtreme > lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])) &&
           ((lFarestExtreme < lDynLimits.mLowDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]) ||
            (lFarestExtreme > lDynLimits.mHighDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])))
        {
            lOutlierValue = lFarestExtreme;
        }
        // If not, check if nearest extreme is an outlier
        else if(((lNearestExtreme < lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]) ||
                 (lNearestExtreme > lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])) &&
                ((lNearestExtreme < lDynLimits.mLowDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]) ||
                 (lNearestExtreme > lDynLimits.mHighDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR])))
        {
            lOutlierValue = lNearestExtreme;
        }
    }

    return lOutlierValue;
}

bool PATOutlierFinder::ApplySamplesFiltering(CPatDefinition * lPatDef, CTest * lTestCell)
{
    Q_D(PATOutlierFinder);

    if (lPatDef == NULL)
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Invalid pointer on PAT definition.");
        return false;
    }

    if (lTestCell == NULL)
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Invalid pointer on Test structure.");
        return false;
    }

    // Check if we have the minimum required number of samples to go further...
    if(d->HasRequiredSamples(*lTestCell) == false)
        return true;

    long    lSamplesRemoved = 0;

    switch(lPatDef->m_SamplesToIgnore)
    {
        // Do not ignore any data sample...
        case GEX_TPAT_IGNOREDATA_NONEID:
            break;

        // Ignore NEGATIVE samples: remove any negative data point from data samples array.
        case GEX_TPAT_IGNOREDATA_NEGATIVEID:

            GSLOG(SYSLOG_SEV_INFORMATIONAL,"Removing negative data point from the dataset");
            for(int lIdx = 0; lIdx < lTestCell->m_testResult.count(); ++lIdx)
            {
                if((lTestCell->m_testResult.resultAt(lIdx) < 0) &&
                   lTestCell->m_testResult.isValidResultAt(lIdx))
                {
                    lTestCell->m_testResult.invalidateResultAt(lIdx);

                    // Fewer samples to scan
                    lSamplesRemoved++;
                }
            }
            break;

        // Ignore POSITIVE samples
        case GEX_TPAT_IGNOREDATA_POSITIVEID:

            GSLOG(SYSLOG_SEV_INFORMATIONAL,"Removing positive data point from the dataset");
            for(int lIdx = 0; lIdx < lTestCell->m_testResult.count(); ++lIdx)
            {
                if((lTestCell->m_testResult.resultAt(lIdx) > 0) &&
                   lTestCell->m_testResult.isValidResultAt(lIdx))
                {
                    lTestCell->m_testResult.invalidateResultAt(lIdx);

                    // Fewer samples to scan
                    lSamplesRemoved++;
                }
            }
            break;

        default:
            GSLOG(SYSLOG_SEV_WARNING, "Undefined type of data to filter on.");
            break;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("%1 sample(s) removed from the dataset").arg(lSamplesRemoved)
          .toLatin1().constData());

    // If no data samples removed, return now.
    if(lSamplesRemoved == 0)
        return true;

    return d->ComputeTestStatistics(lTestCell, d->mPartFilter);
}

void PATOutlierFinder::UpdateSigmaLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                  GS::PAT::DynamicLimits &lDynLimits) const
{
    lDynLimits.mMean     = lTestCell->lfMean;
    lDynLimits.mSigma    = lTestCell->lfSigma;

    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        lDynLimits.mLowDynamicLimit1[lIdx]  = (lDynLimits.mMean -
                                                  lPatDef->m_lfOutlierNFactor * lDynLimits.mSigma);
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
        lDynLimits.mHighDynamicLimit1[lIdx] = (lDynLimits.mMean +
                                                  lPatDef->m_lfOutlierNFactor * lDynLimits.mSigma);
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

bool PATOutlierFinder::UpdateSmartLimits(CPatDefinition *lPatDef, CTest *lTestCell,
                                  GS::PAT::DynamicLimits &lDynLimits, bool forceNewRule) const
{

    // For gaussian with tail, allow smart & adaptive not tio cut both sides of the tail (as we may not have detected the right side over guassina, noise over detection can be hogh!)
    // GCORE-4585: Always compute relaxed limits for the side where a tail is detected.
//    bool    lAssumeTail         = false;
    int     lDistributionShape  = -1;
    int     lRuleToApply        = -1;

    // if new rule can be computed and has been computed, use it
    if (forceNewRule && (lPatDef->mComputedOutlierRule != -1))
        lRuleToApply = lPatDef->mComputedOutlierRule;
    else
        lRuleToApply = lPatDef->mOutlierRule;

    switch(lRuleToApply)
    {
        case GEX_TPAT_RULETYPE_SMARTID:
            lDistributionShape = ComputeAutoDetectSmartLimits(lPatDef, lTestCell, lDynLimits);
            if (forceNewRule)
            {
                switch (lDistributionShape)
                {
                case PATMAN_LIB_SHAPE_GAUSSIAN:
                    lPatDef->mComputedOutlierRule = GEX_TPAT_RULETYPE_GAUSSIANID;
                    break;
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                    lPatDef->mComputedOutlierRule = GEX_TPAT_RULETYPE_GAUSSIANTAILID;
                    break;
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                    lPatDef->mComputedOutlierRule = GEX_TPAT_RULETYPE_LOGNORMALID;
                    break;
                case PATMAN_LIB_SHAPE_BIMODAL:
                    lPatDef->mComputedOutlierRule = GEX_TPAT_RULETYPE_BIMODALID;
                    break;
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                    lPatDef->mComputedOutlierRule = GEX_TPAT_RULETYPE_MULTIMODALID;
                    break;
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                    lPatDef->mComputedOutlierRule = GEX_TPAT_RULETYPE_CLAMPEDID;
                    break;
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                    lPatDef->mComputedOutlierRule = GEX_TPAT_RULETYPE_DUALCLAMPEDID;
                    break;
                case PATMAN_LIB_SHAPE_CATEGORY:
                    lPatDef->mComputedOutlierRule = GEX_TPAT_RULETYPE_CATEGORYID;
                    break;
                case PATMAN_LIB_SHAPE_ERROR:
                    return false;

                default:
                    lPatDef->mComputedOutlierRule = -1;
                    break;
                }
            }
//            switch(lDistributionShape)
//            {
//                case PATMAN_LIB_SHAPE_GAUSSIAN:		// Gaussian
//                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:	// Gaussian: Left tailed
//                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:	// Gaussian: Right tailed
//                    lAssumeTail = true;
//                    break;
//            }
            break;

        case GEX_TPAT_RULETYPE_GAUSSIANID:
            ComputeGaussianSmartLimits(lPatDef, lTestCell, lDynLimits);
//            lAssumeTail = true;
            break;

        case GEX_TPAT_RULETYPE_GAUSSIANTAILID:
            ComputeTailedGaussianSmartLimits(lPatDef, lTestCell, lDynLimits);
//            lAssumeTail = true;
            break;

        case GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID:
            ComputeDoubleTailedGaussianSmartLimits(lPatDef, lTestCell, lDynLimits);
//            lAssumeTail = true;
            break;

        case GEX_TPAT_RULETYPE_LOGNORMALID:
            ComputeLogNormalSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_BIMODALID:
            ComputeBiModalSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_MULTIMODALID:
            ComputeMultiModalSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_CLAMPEDID:
            ComputeClampedSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_DUALCLAMPEDID:
            ComputeDoubleClampedSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case GEX_TPAT_RULETYPE_CATEGORYID:
            ComputeCategorySmartLimits(lPatDef, lTestCell, lDynLimits);
            break;
    }

    // Compute relaxed oultier limits based on tail-detection
    if(lPatDef->m_iTailDirection == GEX_PAT_TAIL_NONE)
    {
        lDynLimits.mHighRelaxed = -GEX_TPAT_DOUBLE_INFINITE;
        lDynLimits.mLowRelaxed  = GEX_TPAT_DOUBLE_INFINITE;
    }

    int	lCellsUsed = 0;

    for(int lIdx = 0; lIdx < TEST_HISTOSIZE; lIdx++)
    {
        if(lTestCell->lHistogram[lIdx])
            lCellsUsed++;
    }

    // If we do not already have computed the high limit, and far outliers are removed:
    if(((lPatDef->m_iTailDirection & GEX_PAT_TAIL_RIGHT) /*|| lAssumeTail*/) &&
       (lDynLimits.mHighRelaxed == -GEX_TPAT_DOUBLE_INFINITE) &&
       (lCellsUsed > (TEST_HISTOSIZE/4)))
    {
        // If a High Spec limit exists, and Histogram Max. higher than High Limit, do not relax anything!
        if(((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) &&
           (lTestCell->GetCurrentLimitItem()->lfHighLimit < lTestCell->GetCurrentLimitItem()->lfHistogramMax))
        {
            // Do nothing: Do NOT relax limit as histogram resolution bar of 18 cells is probably too small for accurate gap computation
        }
        else
        {
            patlib_GetTailCutoff(lTestCell, true, &lDynLimits.mHighRelaxed, lPatDef);
        }
    }

    // If we do not already have computed the low limit, and far outliers are removed:
    if(((lPatDef->m_iTailDirection & GEX_PAT_TAIL_LEFT) /*|| lAssumeTail*/) &&
       (lDynLimits.mLowRelaxed == GEX_TPAT_DOUBLE_INFINITE) &&
       (lCellsUsed > (TEST_HISTOSIZE/4)))
    {
        // If a Low Spec limit exists, and Histogram Min. lower than Low Limit, do not relax anything!
        if(((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                && (lTestCell->GetCurrentLimitItem()->lfLowLimit > lTestCell->GetCurrentLimitItem()->lfHistogramMin))
        {
            // Do nothing: Do NOT relax limit as histogram resolution bar of 18 cells is probably too small for accurate gap computation
        }
        else
        {
            patlib_GetTailCutoff(lTestCell, false, &lDynLimits.mLowRelaxed, lPatDef);
        }
    }

    // Save advanced statistics for given testing site
    lDynLimits.mDynMean  = lTestCell->lfMean;
    lDynLimits.mDynSigma = lTestCell->lfSigma;
    lDynLimits.mDynQ1    = lTestCell->lfSamplesQuartile1;
    lDynLimits.mDynQ2    = lTestCell->lfSamplesQuartile2;
    lDynLimits.mDynQ3    = lTestCell->lfSamplesQuartile3;

    return true;
}

int PATOutlierFinder::ComputeAutoDetectSmartLimits(CPatDefinition *lPatDef, CTest *lTestCell,
                                            GS::PAT::DynamicLimits &lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    int     lDistributionShape  = patlib_GetDistributionType(
                lTestCell,
                d->mContext->GetRecipeOptions().iCategoryValueCount,
                d->mContext->GetRecipeOptions().bAssumeIntegerCategory,
                d->mContext->GetRecipeOptions().mMinConfThreshold,
                d->mPartFilter);
    bool	lPatEnabled         = false;

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Auto detected distribution: %1")
          .arg(patlib_GetDistributionName(lDistributionShape)).toLatin1().data());

    switch(lDistributionShape)
    {
        case PATMAN_LIB_SHAPE_GAUSSIAN:
            // 'false' if disabled PAT on Gaussian distributions.
            lPatEnabled = d->mContext->GetRecipeOptions().bPAT_Gaussian;

            // Compute PAT Limits
            ComputeGaussianSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
        case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
            // 'false' if disabled PAT on Tailed-Gaussian distributions.
            lPatEnabled = d->mContext->GetRecipeOptions().bPAT_GaussianTailed;

            // Compute PAT Limits
            ComputeTailedGaussianSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
        case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
            // 'false' if disabled PAT on LogNormal distributions.
            lPatEnabled = d->mContext->GetRecipeOptions().bPAT_LogNormal;

            // Compute PAT Limits
            ComputeLogNormalSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case PATMAN_LIB_SHAPE_BIMODAL:	// Bi-Modal with each mode clearely detached from the other
            // 'false' if disabled PAT on Multi-modal distributions.
            lPatEnabled = d->mContext->GetRecipeOptions().bPAT_MultiModal;

            // Compute PAT Limits
            ComputeBiModalSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case PATMAN_LIB_SHAPE_MULTIMODAL:	// Bi-modal or more modes, not clearely appart.
            // 'false' if disabled PAT on Multi-modal distributions.
            lPatEnabled = d->mContext->GetRecipeOptions().bPAT_MultiModal;

            // Compute PAT Limits
            ComputeMultiModalSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
        case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
            // 'false' if disabled PAT on clamped distributions.
            lPatEnabled = d->mContext->GetRecipeOptions().bPAT_Clamped;

            // Compute PAT Limits
            ComputeClampedSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
            // 'false' if disabled PAT on double-clamped distributions.
            lPatEnabled = d->mContext->GetRecipeOptions().bPAT_DoubleClamped;

            // Compute PAT Limits
            ComputeDoubleClampedSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case PATMAN_LIB_SHAPE_CATEGORY:
            // 'false' if disabled PAT on category distributions.
            lPatEnabled = d->mContext->GetRecipeOptions().bPAT_Category;

            // Compute PAT Limits
            ComputeCategorySmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case PATMAN_LIB_SHAPE_UNKNOWN:
            // 'false' if disabled PAT on Unknown distributions.
            lPatEnabled = d->mContext->GetRecipeOptions().bPAT_Unknown;

            // Compute PAT Limits
            ComputeUnknownSmartLimits(lPatDef, lTestCell, lDynLimits);
            break;

        case PATMAN_LIB_SHAPE_ERROR:
        default:
            lPatEnabled = false;
            return lDistributionShape;
    }

    // If PAT Limits disabled for the distribution detected, then specify infinite limits!
    if(lPatEnabled == false)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "PAT limits disabled for the distribution detected");

        // If PAT disabled for this distribution...
        for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
            ++lIdx)
        {
            lDynLimits.mLowDynamicLimit1[lIdx]  = -GEX_TPAT_DOUBLE_INFINITE;
            lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
            lDynLimits.mHighDynamicLimit1[lIdx] = GEX_TPAT_DOUBLE_INFINITE;
            lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
        }
    }

    // Return distribution shape detected
    return lDistributionShape;
}

void PATOutlierFinder::UpdateRangeLimits(CPatDefinition *lPatDef, const CTest * lTestCell,
                                  GS::PAT::DynamicLimits &lDynLimits) const
{
    lDynLimits.mMean     = lTestCell->lfMean;
    lDynLimits.mSigma    = lTestCell->lfSigma;

    // Dynamic Limits are identical for all testing sites (% of limits space)
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        lDynLimits.mLowDynamicLimit1[lIdx]  = lDynLimits.mMean - lPatDef->m_lfOutlierNFactor;
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
        lDynLimits.mHighDynamicLimit1[lIdx] = lDynLimits.mMean + lPatDef->m_lfOutlierNFactor;
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

void PATOutlierFinder::UpdateRobustSigmaLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                        GS::PAT::DynamicLimits &lDynLimits) const
{
    lDynLimits.mMean = lTestCell->lfSamplesQuartile2;
    // Robust sigma = IQR/1.35
    lDynLimits.mSigma= (lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1) / 1.35;

    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        lDynLimits.mLowDynamicLimit1[lIdx]  = (lDynLimits.mMean -
                                                  lPatDef->m_lfOutlierNFactor * lDynLimits.mSigma);
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];

        lDynLimits.mHighDynamicLimit1[lIdx] = (lDynLimits.mMean +
                                                  lPatDef->m_lfOutlierNFactor * lDynLimits.mSigma);
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

void PATOutlierFinder::UpdateLimitPctLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                     GS::PAT::DynamicLimits &lDynLimits) const
{
    lDynLimits.mMean     = lTestCell->lfMean;
    lDynLimits.mSigma    = lTestCell->lfSigma;

    // Set limits only if test has 2 limits, otherwise leave the infinite limits
    if((lTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0)
    {
        // Dynamic Limits are identical for all testing sites (% of limits space)
        double lValue = (lTestCell->GetCurrentLimitItem()->lfHighLimit - lTestCell->GetCurrentLimitItem()->lfLowLimit) *
                        lPatDef->m_lfOutlierNFactor / 100.0;
        for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
            lIdx++)
        {
            lDynLimits.mLowDynamicLimit1[lIdx]  = lDynLimits.mMean - lValue;
            lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
            lDynLimits.mHighDynamicLimit1[lIdx] = lDynLimits.mMean + lValue;
            lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
        }
    }
}

void PATOutlierFinder::UpdateQ1Q3IQRSigmaLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                         GS::PAT::DynamicLimits &lDynLimits) const
{
    double	lIQR = lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1;

    lDynLimits.mMean = lTestCell->lfSamplesQuartile2;
    lDynLimits.mSigma= (lIQR)/1.35;	// Robust sigma = IQR/1.35

    // Security if IQR = 0
    if(lIQR == 0)
        lIQR = (lTestCell->lfSamplesQuartile3) * 1e-6;

    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        lIdx++)
    {
        lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 -
                                                 lIQR * lPatDef->m_lfOutlierNFactor;
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
        lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 +
                                                 lIQR * lPatDef->m_lfOutlierNFactor;
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

void PATOutlierFinder::ComputeGaussianSmartLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                           GS::PAT::DynamicLimits &lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Computing Gaussian Smart Limit on Test %1.%2 ")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).toLatin1().constData());

    // Get Mean/sigma values to use (or Median/RobustSigma)
    double	lIQR;

    SetAlgorithmValues(d->mContext->GetRecipeOptions().iAlgo_Gaussian, lTestCell, lDynLimits);

    // Clear tail direction.
    lPatDef->m_iTailDirection = GEX_PAT_TAIL_NONE;

    bool lDoNotUseLegacyHack =
        (QString(qgetenv("GS_SHAPEDETECTION_LEGACY")) != "1") ? true : false;
    lDoNotUseLegacyHack = false;

    // Compute all limits sets
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        switch(d->mContext->GetRecipeOptions().iAlgo_Gaussian)
        {
            case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    // No custom N*Sigma specified for this test, then use the global one!
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lDynLimits.mMean -
                                                             (lDynLimits.mSigma *
                                                              d->mContext->GetRecipeOptions().lfSmart_HeadGaussian[lIdx]);
                    lDynLimits.mHighDynamicLimit1[lIdx] = lDynLimits.mMean +
                                                             (lDynLimits.mSigma *
                                                              d->mContext->GetRecipeOptions().lfSmart_TailGaussian[lIdx]);

                    // Allow Right & Left tail processing (relaxing outlier limits if need be.
                    if (!lDoNotUseLegacyHack)
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT | GEX_PAT_TAIL_RIGHT;
                }
                else
                {
                    // Custom N*Sigma specified for this test, use it!
                    lDynLimits.mLowDynamicLimit1[lIdx] = lDynLimits.mMean -
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                    lDynLimits.mHighDynamicLimit1[lIdx]= lDynLimits.mMean +
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                }
                break;

            case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+N*IQR
                lIQR = lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1;
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 - lIQR *
                                                             d->mContext->GetRecipeOptions().lfSmart_HeadGaussian[lIdx];
                    lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 + lIQR *
                                                             d->mContext->GetRecipeOptions().lfSmart_TailGaussian[lIdx];

                    // Allow Right & Left tail processing (relaxing outlier limits if need be.
                    if (!lDoNotUseLegacyHack)
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT | GEX_PAT_TAIL_RIGHT;
                }
                else
                {
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 -
                                                             lIQR * lPatDef->m_lfOutlierNFactor;
                    lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 +
                                                             lIQR * lPatDef->m_lfOutlierNFactor;
                }
                break;

            case GEX_TPAT_SMART_ALGO_CUSTOMLIB:	// User custom PAT lib
                break;
        }

        // Second set of limits only different if Bi-Modal distribution.
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

void PATOutlierFinder::ComputeTailedGaussianSmartLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                                 GS::PAT::DynamicLimits &lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Computing Tailed Gaussian Smart Limit on Test %1.%2 ")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).toLatin1().constData());

    double	lHead   = 0.0;
    double  lTail   = 0.0;
    double  lSwap;
    double  lIQR;

    SetAlgorithmValues(d->mContext->GetRecipeOptions().iAlgo_GaussianTailed, lTestCell, lDynLimits);

    // If distribution is Left tailed, then swap the Tail & Head limits!
    bool lLeftTailed = (patlib_getDistributionNormalDirection(lTestCell) < 0) ? true: false;

    // Clear tail direction.
    lPatDef->m_iTailDirection = GEX_PAT_TAIL_NONE;

    // Compute all limits sets
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        lIdx++)
    {
        switch(d->mContext->GetRecipeOptions().iAlgo_GaussianTailed)
        {
            case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
                if(lPatDef->m_lfOutlierNFactor == 0 && lPatDef->m_lfOutlierTFactor == 0)
                {
                    // Allow Right / Left tail processing (relaxing outlier limits if need be)
                    if(lLeftTailed)
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT;
                    else
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_RIGHT;
                }

                if (lPatDef->m_lfOutlierNFactor == 0)  // No custom specified for this test, then use the global one!
                    lHead  = (lDynLimits.mSigma *
                              d->mContext->GetRecipeOptions().lfSmart_HeadGaussianTailed[lIdx]);
                else
                    // Custom specified for this test, use it!
                    lHead  = (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);

                if (lPatDef->m_lfOutlierTFactor == 0)
                    // No custom specified for this test, then use the global one!
                    lTail  = (lDynLimits.mSigma *
                              d->mContext->GetRecipeOptions().lfSmart_TailGaussianTailed[lIdx]);
                else
                    // Custom specified for this test, use it!
                    lTail  = (lDynLimits.mSigma * lPatDef->m_lfOutlierTFactor);

                // If distribution is Left tailed, then swap the Tail & Head limits!
                if(lLeftTailed)
                {
                    lSwap = lHead;
                    lHead = lTail;
                    lTail = lSwap;
                }

                // Second set of limits only different if Bi-Modal distribution.
                lDynLimits.mLowDynamicLimit1[lIdx]   = lDynLimits.mMean - lHead;
                lDynLimits.mLowDynamicLimit2[lIdx]   = lDynLimits.mLowDynamicLimit1[lIdx];
                lDynLimits.mHighDynamicLimit1[lIdx]  = lDynLimits.mMean + lTail;
                lDynLimits.mHighDynamicLimit2[lIdx]  = lDynLimits.mHighDynamicLimit1[lIdx];

            break;

            case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+N*IQR
                lIQR = lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1;

                if(lPatDef->m_lfOutlierNFactor == 0 && lPatDef->m_lfOutlierTFactor == 0)
                {
                    // Allow Right / Left tail processing (relaxing outlier limits if need be)
                    if(lLeftTailed)
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT;
                    else
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_RIGHT;
                }

                if(lPatDef->m_lfOutlierNFactor == 0) // No custom specified for this test, then use the global one!
                    lHead = d->mContext->GetRecipeOptions().lfSmart_HeadGaussianTailed[lIdx];
                else                                  // Custom specified for this test, use it!
                    lHead  = lPatDef->m_lfOutlierNFactor;

                if(lPatDef->m_lfOutlierTFactor == 0) // No custom specified for this test, then use the global one!
                    lTail = d->mContext->GetRecipeOptions().lfSmart_TailGaussianTailed[lIdx];
                else                                  // Custom specified for this test, use it!
                    lTail  = lPatDef->m_lfOutlierTFactor;

                if(lLeftTailed)
                {
                    lSwap = lHead;
                    lHead = lTail;
                    lTail = lSwap;
                }

                lDynLimits.mLowDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile1 - lIQR * lHead;
                lDynLimits.mLowDynamicLimit2[lIdx] = lDynLimits.mLowDynamicLimit1[lIdx];

                lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 + lIQR * lTail;
                lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
                break;

            case GEX_TPAT_SMART_ALGO_CUSTOMLIB:	// User custom PAT lib
                break;
        }
    }
}

void PATOutlierFinder::ComputeDoubleTailedGaussianSmartLimits(CPatDefinition * lPatDef,
                                                       const CTest * lTestCell,
                                                       GS::PAT::DynamicLimits & lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Computing Double Tailed Gaussian Smart Limit on Test %1.%2 ")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).toLatin1().constData());

    double	lIQR;

    // Set Mean/sigma values to use (or Median/RobustSigma)
    SetAlgorithmValues(d->mContext->GetRecipeOptions().iAlgo_GaussianDoubleTailed, lTestCell, lDynLimits);

    // Clear tail direction.
    lPatDef->m_iTailDirection = GEX_PAT_TAIL_NONE;

    // Compute all limits sets
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        switch(d->mContext->GetRecipeOptions().iAlgo_GaussianDoubleTailed)
        {
            case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    // No custom N*Sigma specified for this test, then use the global one!
                    lDynLimits.mLowDynamicLimit1[lIdx] = lDynLimits.mMean -
                                                            (lDynLimits.mSigma *
                                                             d->mContext->GetRecipeOptions().lfSmart_HeadGaussianDoubleTailed[lIdx]);
                    lDynLimits.mHighDynamicLimit1[lIdx]= lDynLimits.mMean +
                                                            (lDynLimits.mSigma *
                                                             d->mContext->GetRecipeOptions().lfSmart_TailGaussianDoubleTailed[lIdx]);

                    // Allow Right & Left tail processing (relaxing outlier limits if need be)
                    lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT | GEX_PAT_TAIL_RIGHT;
                }
                else
                {
                    // Custom N*Sigma specified for this test, use it!
                    lDynLimits.mLowDynamicLimit1[lIdx] = lDynLimits.mMean -
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                    lDynLimits.mHighDynamicLimit1[lIdx]= lDynLimits.mMean +
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                }
                break;

            case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+N*IQR
                lIQR = lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1;

                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    lDynLimits.mLowDynamicLimit1[lIdx]   = lTestCell->lfSamplesQuartile1 -
                                                              lIQR *
                                                              d->mContext->GetRecipeOptions().lfSmart_HeadGaussianDoubleTailed[lIdx];
                    lDynLimits.mHighDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile3 +
                                                              lIQR *
                                                              d->mContext->GetRecipeOptions().lfSmart_TailGaussianDoubleTailed[lIdx];

                    // Allow Right & Left tail processing (relaxing outlier limits if need be)
                    lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT | GEX_PAT_TAIL_RIGHT;
                }
                else
                {
                    // Custom factore
                    lDynLimits.mLowDynamicLimit1[lIdx]   = lTestCell->lfSamplesQuartile1 -
                                                              lIQR * lPatDef->m_lfOutlierNFactor;
                    lDynLimits.mHighDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile3 +
                                                              lIQR * lPatDef->m_lfOutlierNFactor;
                }
                break;

            case GEX_TPAT_SMART_ALGO_CUSTOMLIB:	// User custom PAT lib
                break;
        }

        // Second set of limits only different if Bi-Modal distribution.
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

void PATOutlierFinder::ComputeLogNormalSmartLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                            GS::PAT::DynamicLimits &lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Computing LogNormal Smart Limit on Test %1.%2 ")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).toLatin1().constData());

    // Compute limits for "LogNormal: right tailed"
    double	lHead   = 0.0;
    double  lTail   = 0.0;
    double  lSwap;
    double  lIQR;

    // Get Mean/sigma values to use (or Median/RobustSigma)
    SetAlgorithmValues(d->mContext->GetRecipeOptions().iAlgo_LogNormal, lTestCell, lDynLimits);

    // If distribution is Left tailed, then swap the Tail & Head limits!
    bool lLeftTailed = (patlib_getDistributionLogNormalDirection(lTestCell) < 0) ? true: false;

    // Clear tail direction.
    lPatDef->m_iTailDirection = GEX_PAT_TAIL_NONE;

    // Compute all limits sets
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        lIdx++)
    {
        switch(d->mContext->GetRecipeOptions().iAlgo_LogNormal)
        {
            case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
                if(lPatDef->m_lfOutlierNFactor == 0 && lPatDef->m_lfOutlierTFactor == 0)
                {
                    // Allow Right / Left tail processing (relaxing outlier limits if need be)
                    if(lLeftTailed)
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT;
                    else
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_RIGHT;
                }

                if(lPatDef->m_lfOutlierNFactor == 0)   // No custom specified for this test, then use the global one!
                    lHead  = (lDynLimits.mSigma * d->mContext->GetRecipeOptions().lfSmart_HeadLogNormal[lIdx]);
                else                                    // Custom specified for this test, use it!
                    lHead  = (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);

                if(lPatDef->m_lfOutlierTFactor == 0)   // No custom specified for this test, then use the global one!
                    lTail  = (lDynLimits.mSigma * d->mContext->GetRecipeOptions().lfSmart_TailLogNormal[lIdx]);
                else                                    // Custom specified for this test, use it!
                    lTail  = (lDynLimits.mSigma * lPatDef->m_lfOutlierTFactor);

                // If distribution is Left tailed, then swap the Tail & Head limits!
                if(lLeftTailed)
                {
                    lSwap = lHead;
                    lHead = lTail;
                    lTail = lSwap;
                }

                // Second set of limits only different if Bi-Modal distribution.
                lDynLimits.mLowDynamicLimit1[lIdx]   = lDynLimits.mMean - lHead;;
                lDynLimits.mLowDynamicLimit2[lIdx]   = lDynLimits.mLowDynamicLimit1[lIdx];
                lDynLimits.mHighDynamicLimit1[lIdx]  = lDynLimits.mMean + lTail;
                lDynLimits.mHighDynamicLimit2[lIdx]  = lDynLimits.mHighDynamicLimit1[lIdx];

                break;

            case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+N*IQR
                lIQR = lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1;
                if(lPatDef->m_lfOutlierNFactor == 0 && lPatDef->m_lfOutlierTFactor == 0)
                {
                    // Allow Right / Left tail processing (relaxing outlier limits if need be)
                    if(lLeftTailed)
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT;
                    else
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_RIGHT;
                }

                if(lPatDef->m_lfOutlierNFactor == 0)   // No custom specified for this test, then use the global one!
                    lHead  = d->mContext->GetRecipeOptions().lfSmart_HeadLogNormal[lIdx];
                else                                    // Custom specified for this test, use it!
                    lHead  = lPatDef->m_lfOutlierNFactor;

                if(lPatDef->m_lfOutlierTFactor == 0)   // No custom specified for this test, then use the global one!
                    lTail = d->mContext->GetRecipeOptions().lfSmart_TailLogNormal[lIdx];
                else                                    // Custom specified for this test, use it!
                    lTail =  lPatDef->m_lfOutlierTFactor;

                // If distribution is Left tailed, then swap the Tail & Head limits!
                if(lLeftTailed)
                {
                    lSwap = lHead;
                    lHead = lTail;
                    lTail = lSwap;
                }

                lDynLimits.mLowDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile1 - lIQR * lHead;
                lDynLimits.mLowDynamicLimit2[lIdx] = lDynLimits.mLowDynamicLimit1[lIdx];
                lDynLimits.mHighDynamicLimit1[lIdx]= lTestCell->lfSamplesQuartile3 + lIQR * lTail;
                lDynLimits.mHighDynamicLimit2[lIdx]= lDynLimits.mHighDynamicLimit1[lIdx];

            break;

            case GEX_TPAT_SMART_ALGO_CUSTOMLIB:	// User custom PAT lib
                break;
        }
    }
}

void PATOutlierFinder::ComputeBiModalSmartLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                          GS::PAT::DynamicLimits &lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Computing BiModal Smart Limit on Test %1.%2 ")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).toLatin1().constData());

    // First, get the Mean and Sigma values of each mode
    double	lMean1;
    double  lMean2;
    double	lSigma1;
    double  lSigma2;
    double  lExponent = ScalingPower(lTestCell->res_scal);

    // If error while extracting the two modes, then call multi-modal instead.
    if(!patlib_getDistributionBiModalStats(lTestCell,lMean1, lSigma1, lMean2, lSigma2,
                                           lExponent, d->mPartFilter))
    {
        ComputeMultiModalSmartLimits(lPatDef, lTestCell, lDynLimits);
        return;
    }

    // Allow Right / Left tail processing (relaxing outlier limits if need be)
    lPatDef->m_iTailDirection = GEX_PAT_TAIL_NONE;

    // Compute limits for "Bi-Modal"
    // Compute all limits sets
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        if(lPatDef->m_lfOutlierNFactor == 0)
        {
            // No custom N*Sigma specified for this test, then use the global one! for each mode
            lDynLimits.mLowDynamicLimit1[lIdx] = lMean1 -
                                                    lSigma1 *
                                                    d->mContext->GetRecipeOptions().lfSmart_HeadMultiModal[lIdx];
            lDynLimits.mHighDynamicLimit1[lIdx]= lMean1 +
                                                    lSigma1 *
                                                    d->mContext->GetRecipeOptions().lfSmart_TailMultiModal[lIdx];

            // Mode 2
            lDynLimits.mLowDynamicLimit2[lIdx] = lMean2 -
                                                    lSigma2 *
                                                    d->mContext->GetRecipeOptions().lfSmart_HeadMultiModal[lIdx];
            lDynLimits.mHighDynamicLimit2[lIdx]= lMean2 +
                                                    lSigma2 *
                                                    d->mContext->GetRecipeOptions().lfSmart_TailMultiModal[lIdx];
        }
        else
        {
            // Custom N*Sigma specified for this test, use it! for each mode
            lDynLimits.mLowDynamicLimit1[lIdx] = lMean1 - lSigma1 * lPatDef->m_lfOutlierNFactor;
            lDynLimits.mHighDynamicLimit1[lIdx]= lMean1 + lSigma1 * lPatDef->m_lfOutlierNFactor;

            lDynLimits.mLowDynamicLimit2[lIdx] = lMean2 - lSigma2 * lPatDef->m_lfOutlierNFactor;
            lDynLimits.mHighDynamicLimit2[lIdx]= lMean2 + lSigma2 * lPatDef->m_lfOutlierNFactor;
        }
    }
}

void PATOutlierFinder::ComputeMultiModalSmartLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                             GS::PAT::DynamicLimits &lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Computing Multi-Modal Smart Limit on Test %1.%2 ")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).toLatin1().constData());

    // Compute limits for "Multi-Modal"
    double	lIQR;

    // Clear tail direction.
    lPatDef->m_iTailDirection = GEX_PAT_TAIL_NONE;

    // Get Mean/sigma values to use (or Median/RobustSigma)
    SetAlgorithmValues(d->mContext->GetRecipeOptions().iAlgo_MultiModal, lTestCell, lDynLimits);

    // Compute all limits sets
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        switch(d->mContext->GetRecipeOptions().iAlgo_MultiModal)
        {
            case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    // No custom N*Sigma specified for this test, then use the global one!
                    lDynLimits.mLowDynamicLimit1[lIdx]   = lDynLimits.mMean -
                                                              (lDynLimits.mSigma * d->mContext->GetRecipeOptions().lfSmart_HeadMultiModal[lIdx]);
                    lDynLimits.mHighDynamicLimit1[lIdx]  = lDynLimits.mMean +
                                                              (lDynLimits.mSigma * d->mContext->GetRecipeOptions().lfSmart_TailMultiModal[lIdx]);

                    // Allow Right / Left tail processing (relaxing outlier limits if need be)
                    lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT | GEX_PAT_TAIL_RIGHT;
                }
                else
                {
                    // Custom N*Sigma specified for this test, use it!
                    lDynLimits.mLowDynamicLimit1[lIdx] = lDynLimits.mMean -
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                    lDynLimits.mHighDynamicLimit1[lIdx]= lDynLimits.mMean +
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                }
                break;

            case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+N*IQR
                lIQR = lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1;
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 -
                                                             lIQR * d->mContext->GetRecipeOptions().lfSmart_HeadMultiModal[lIdx];
                    lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 +
                                                             lIQR * d->mContext->GetRecipeOptions().lfSmart_TailMultiModal[lIdx];

                    // Allow Right / Left tail processing (relaxing outlier limits if need be)
                    lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT | GEX_PAT_TAIL_RIGHT;
                }
                else
                {
                    // Custom factors
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 -
                                                             lIQR * lPatDef->m_lfOutlierNFactor;
                    lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 +
                                                             lIQR * lPatDef->m_lfOutlierNFactor;
                }
            break;

            case GEX_TPAT_SMART_ALGO_CUSTOMLIB:	// User custom PAT lib
                break;
        }

        // Second set of limits only different if Bi-Modal distribution.
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

void PATOutlierFinder::ComputeClampedSmartLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                          GS::PAT::DynamicLimits &lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Computing Clamped Smart Limit on Test %1.%2 ")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).toLatin1().constData());

    double	lHead   = 0.0;
    double  lTail   = 0.0;
    double  lSwap;
    double  lIQR;

    // Get Mean/sigma values to use (or Median/RobustSigma)
    SetAlgorithmValues(d->mContext->GetRecipeOptions().iAlgo_Clamped, lTestCell, lDynLimits);

    // If distribution is Left clamped, then swap the Tail & Head limits!
    bool lLeftClamped = (patlib_getDistributionClampedDirection(lTestCell) < 0) ? true: false;

    // Clear tail direction.
    lPatDef->m_iTailDirection = GEX_PAT_TAIL_NONE;

    // Compute all limits sets
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        switch(d->mContext->GetRecipeOptions().iAlgo_Clamped)
        {
            case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
                if(lPatDef->m_lfOutlierNFactor == 0 && lPatDef->m_lfOutlierTFactor == 0)
                {
                    // Allow Right / Left tail processing (relaxing outlier limits if need be)
                    if(lLeftClamped)
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_RIGHT;
                    else
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT;
                }

                if(lPatDef->m_lfOutlierNFactor == 0)   // No custom specified for this test, then use the global one!
                    lHead  = (lDynLimits.mSigma * d->mContext->GetRecipeOptions().lfSmart_HeadClamped[lIdx]);
                else                                    // Custom specified for this test, use it!
                    lHead  = (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);

                if(lPatDef->m_lfOutlierTFactor == 0)   // No custom specified for this test, then use the global one!
                    lTail  = (lDynLimits.mSigma * d->mContext->GetRecipeOptions().lfSmart_TailClamped[lIdx]);
                else                                    // Custom specified for this test, use it!
                    lTail  = (lDynLimits.mSigma * lPatDef->m_lfOutlierTFactor);

                // If distribution is right clamped (left tailed), then swap the Tail & Head limits!
                if(lLeftClamped == false)
                {
                    lSwap = lHead;
                    lHead = lTail;
                    lTail = lSwap;
                }

                // Second set of limits only different if Bi-Modal distribution.
                lDynLimits.mLowDynamicLimit1[lIdx]   = lDynLimits.mMean - lHead;
                lDynLimits.mLowDynamicLimit2[lIdx]   = lDynLimits.mLowDynamicLimit1[lIdx];
                lDynLimits.mHighDynamicLimit1[lIdx]  = lDynLimits.mMean + lTail;
                lDynLimits.mHighDynamicLimit2[lIdx]  = lDynLimits.mHighDynamicLimit1[lIdx];

                break;

            case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+N*IQR
                lIQR = lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1;
                if(lPatDef->m_lfOutlierNFactor == 0 && lPatDef->m_lfOutlierTFactor == 0)
                {
                    // Allow Right / Left tail processing (relaxing outlier limits if need be)
                    if(lLeftClamped)
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_RIGHT;
                    else
                        lPatDef->m_iTailDirection = GEX_PAT_TAIL_LEFT;
                }

                if(lPatDef->m_lfOutlierNFactor == 0)   // No custom specified for this test, then use the global one!
                    lHead = d->mContext->GetRecipeOptions().lfSmart_HeadClamped[lIdx];
                else                                    // Custom specified for this test, use it!
                    lHead  = lPatDef->m_lfOutlierNFactor;

                if(lPatDef->m_lfOutlierTFactor == 0)   // No custom specified for this test, then use the global one!
                    lTail = d->mContext->GetRecipeOptions().lfSmart_TailClamped[lIdx];
                else                                    // Custom specified for this test, use it!
                    lTail  = lPatDef->m_lfOutlierTFactor;

                // If distribution is right clamped (left tailed), then swap the Tail & Head limits!
                if(lLeftClamped == false)
                {
                    lSwap = lHead;
                    lHead = lTail;
                    lTail = lSwap;
                }

                lDynLimits.mLowDynamicLimit1[lIdx]   = lTestCell->lfSamplesQuartile1 - lIQR * lHead;
                lDynLimits.mLowDynamicLimit2[lIdx]   = lDynLimits.mLowDynamicLimit1[lIdx];
                lDynLimits.mHighDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile3 + lIQR * lTail;
                lDynLimits.mHighDynamicLimit2[lIdx]  = lDynLimits.mHighDynamicLimit1[lIdx];
                break;

            case GEX_TPAT_SMART_ALGO_CUSTOMLIB:	// User custom PAT lib
                break;
        }
    }
}

void PATOutlierFinder::ComputeDoubleClampedSmartLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                                GS::PAT::DynamicLimits &lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Computing Double Clamped Smart Limit on Test %1.%2 ")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).toLatin1().constData());

    double	lIQR;

    // Get Mean/sigma values to use (or Median/RobustSigma)
    SetAlgorithmValues(d->mContext->GetRecipeOptions().iAlgo_DoubleClamped, lTestCell, lDynLimits);

    // Disable Right / Left tail processing (relaxing outlier limits if need be)
    lPatDef->m_iTailDirection = GEX_PAT_TAIL_NONE;

    // Compute all limits sets
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        switch(d->mContext->GetRecipeOptions().iAlgo_DoubleClamped)
        {
            case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    lDynLimits.mLowDynamicLimit1[lIdx] = lDynLimits.mMean -
                                                            (lDynLimits.mSigma *
                                                             d->mContext->GetRecipeOptions().lfSmart_HeadDoubleClamped[lIdx]);
                    lDynLimits.mHighDynamicLimit1[lIdx]= lDynLimits.mMean +
                                                            (lDynLimits.mSigma *
                                                             d->mContext->GetRecipeOptions().lfSmart_TailDoubleClamped[lIdx]);
                }
                else
                {
                    lDynLimits.mLowDynamicLimit1[lIdx] = lDynLimits.mMean -
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                    lDynLimits.mHighDynamicLimit1[lIdx]= lDynLimits.mMean +
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                }
                break;
            case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+N*IQR
                lIQR = lTestCell->lfSamplesQuartile3  -lTestCell->lfSamplesQuartile1;
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 -
                                                             lIQR * d->mContext->GetRecipeOptions().lfSmart_HeadDoubleClamped[lIdx];
                    lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 +
                                                             lIQR * d->mContext->GetRecipeOptions().lfSmart_TailDoubleClamped[lIdx];
                }
                else
                {
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 -
                                                             lIQR * lPatDef->m_lfOutlierNFactor;
                    lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 +
                                                             lIQR * lPatDef->m_lfOutlierNFactor;
                }
            break;

            case GEX_TPAT_SMART_ALGO_CUSTOMLIB:	// User custom PAT lib
                break;
        }

        // Second set of limits only different if Bi-Modal distribution.
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

void PATOutlierFinder::ComputeCategorySmartLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                           GS::PAT::DynamicLimits &lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Computing Category Smart Limit on Test %1.%2 ")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).toLatin1().constData());

    double	lIQR;

    // Get Mean/sigma values to use (or Median/RobustSigma)
    SetAlgorithmValues(d->mContext->GetRecipeOptions().iAlgo_Category, lTestCell, lDynLimits);

    // Disable Right / Left tail processing (relaxing outlier limits if need be)
    lPatDef->m_iTailDirection = GEX_PAT_TAIL_NONE;

    // Compute all limits sets
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        switch(d->mContext->GetRecipeOptions().iAlgo_Category)
        {
            case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    lDynLimits.mLowDynamicLimit1[lIdx] = lDynLimits.mMean -
                                                            (lDynLimits.mSigma * d->mContext->GetRecipeOptions().lfSmart_HeadCategory[lIdx]);
                    lDynLimits.mHighDynamicLimit1[lIdx]= lDynLimits.mMean +
                                                            (lDynLimits.mSigma * d->mContext->GetRecipeOptions().lfSmart_TailCategory[lIdx]);
                }
                else
                {
                    lDynLimits.mLowDynamicLimit1[lIdx] = lDynLimits.mMean -
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                    lDynLimits.mHighDynamicLimit1[lIdx]= lDynLimits.mMean +
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                }
                break;

            case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+N*IQR
                lIQR = lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1;
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 -
                                                             lIQR * d->mContext->GetRecipeOptions().lfSmart_HeadCategory[lIdx];
                    lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 +
                                                             lIQR * d->mContext->GetRecipeOptions().lfSmart_TailCategory[lIdx];
                }
                else
                {
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 -
                                                             lIQR * lPatDef->m_lfOutlierNFactor;
                    lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 +
                                                             lIQR * lPatDef->m_lfOutlierNFactor;
                }
            break;

            case GEX_TPAT_SMART_ALGO_CUSTOMLIB:	// User custom PAT lib
                break;
        }

        // Second set of limits only different if Bi-Modal distribution.
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

void PATOutlierFinder::ComputeUnknownSmartLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                                          GS::PAT::DynamicLimits &lDynLimits) const
{
    Q_D(const PATOutlierFinder);

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Computing Unknown Smart Limit on Test %1.%2 ")
          .arg(lTestCell->lTestNumber).arg(lTestCell->lPinmapIndex).toLatin1().constData());

    double	lIQR;

    // Get Mean/sigma values to use (or Median/RobustSigma)
    SetAlgorithmValues(d->mContext->GetRecipeOptions().iAlgo_Unknown, lTestCell, lDynLimits);

    // Disable Right / Left tail processing (relaxing outlier limits if need be)
    lPatDef->m_iTailDirection = GEX_PAT_TAIL_NONE;

    // Compute all limits sets
    for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        ++lIdx)
    {
        switch(d->mContext->GetRecipeOptions().iAlgo_Unknown)
        {
            case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    lDynLimits.mLowDynamicLimit1[lIdx] = lDynLimits.mMean -
                                                            (lDynLimits.mSigma * d->mContext->GetRecipeOptions().lfSmart_HeadUnknown[lIdx]);
                    lDynLimits.mHighDynamicLimit1[lIdx]= lDynLimits.mMean +
                                                            (lDynLimits.mSigma * d->mContext->GetRecipeOptions().lfSmart_TailUnknown[lIdx]);
                }
                else
                {
                    lDynLimits.mLowDynamicLimit1[lIdx] = lDynLimits.mMean -
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                    lDynLimits.mHighDynamicLimit1[lIdx]= lDynLimits.mMean +
                                                            (lDynLimits.mSigma * lPatDef->m_lfOutlierNFactor);
                }
                break;
            case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+N*IQR
                lIQR = lTestCell->lfSamplesQuartile3 - lTestCell->lfSamplesQuartile1;
                if(lPatDef->m_lfOutlierNFactor == 0)
                {
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 -
                                                             lIQR * d->mContext->GetRecipeOptions().lfSmart_HeadUnknown[lIdx];
                    lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 +
                                                             lIQR * d->mContext->GetRecipeOptions().lfSmart_TailUnknown[lIdx];
                }
                else
                {
                    lDynLimits.mLowDynamicLimit1[lIdx]  = lTestCell->lfSamplesQuartile1 -
                                                             lIQR * lPatDef->m_lfOutlierNFactor;
                    lDynLimits.mHighDynamicLimit1[lIdx] = lTestCell->lfSamplesQuartile3 +
                                                             lIQR * lPatDef->m_lfOutlierNFactor;
                }
            break;

            case GEX_TPAT_SMART_ALGO_CUSTOMLIB:	// User custom PAT lib
                break;
        }

        // Second set of limits only different if Bi-Modal distribution.
        lDynLimits.mLowDynamicLimit2[lIdx]  = lDynLimits.mLowDynamicLimit1[lIdx];
        lDynLimits.mHighDynamicLimit2[lIdx] = lDynLimits.mHighDynamicLimit1[lIdx];
    }
}

}
}
#endif
