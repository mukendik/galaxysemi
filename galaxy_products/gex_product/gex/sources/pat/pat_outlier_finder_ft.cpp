#ifdef GCORE15334

#include "pat_outlier_finder_ft.h"
#include "pat_outlier_finder_ft_private.h"
#include "pat_info.h"
#include "pat_definition.h"
#include "sitetestresults.h"
#include "gqtl_log.h"
#include "gex_algorithms.h"


extern int          patlib_GetDistributionType(CTest *ptTestCell,
                                               int lCategoryValueCount = 5,
                                               bool lAssumeIntegerCategory = true,
                                               int aMinConfThreshold = 2,
                                               GS::Gex::PATPartFilter *partFilter = NULL);
extern double       ScalingPower(int iPower);

namespace GS
{
namespace Gex
{

PATOutlierFinderFT::PATOutlierFinderFT(CPatInfo *lContext, QMap<int, SiteTestResults *> *lFTAllSites, QObject *parent)
    : PATOutlierFinder(*new PATOutlierFinderFTPrivate, parent)
{
    Q_D(PATOutlierFinderFT);

    d->Init(lContext, lFTAllSites);
}

PATOutlierFinderFT::PATOutlierFinderFT(PATOutlierFinderFTPrivate &lPrivateData, QMap<int, SiteTestResults *> *lFTAllSites,
                         CPatInfo *lContext, QObject *parent)
    : PATOutlierFinder(lPrivateData, parent)
{
    Q_D(PATOutlierFinderFT);

    d->Init(lContext, lFTAllSites);
}


PATOutlierFinderFT::~PATOutlierFinderFT()
{
}

CTest *PATOutlierFinderFT::FindTestCell(CPatDefinition * lPatDef)
{
    Q_D(PATOutlierFinderFT);

    CTest * lTestFound = NULL;

    // Final-Test PAT: Find Test cell (with samples)
    // 7208 [BG]: No need to check PatInfo->cOptionsPat.mTestKey anymore. Checked inside the site.
    lTestFound = d->mFTCurrentSite->FindTestCell((unsigned int)lPatDef->m_lTestNumber,
                                                 lPatDef->mPinIndex, false, true,
                                                 lPatDef->m_strTestName);
#if 0
    switch(d->mContext->cOptionsPat.mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
        case GEX_TBPAT_KEY_TESTMIX:
            // Find by testnumber / test name
            lTestFound = d->mFTCurrentSite->FindTestCell(lPatDef->m_lTestNumber, false, true);
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            // Find by test name.
            lTestFound = d->mFTCurrentSite->FindTestCellName(lPatDef->m_lTestNumber, false, true,
                                                             lPatDef->m_strTestName.toLatin1().data());
            break;
    }
#endif

    return lTestFound;
}

bool PATOutlierFinderFT::UpdateDatasetPointer(int lSite)
{
    Q_D(PATOutlierFinderFT);

    // Final-Test (real time) PAT
    if (d->mFTMultiSite && lSite == 255)
    {
        d->mFTCurrentSite = d->mFTMultiSite;
        return true;
    }
    else if (d->mFTAllSites && d->mFTAllSites->contains(lSite))
    {
        d->mFTCurrentSite = d->mFTAllSites->value(lSite);
        return true;
    }
    else
    {
        d->mFTCurrentSite = NULL;
        return false;
    }
}

bool PATOutlierFinderFT::ComputeTestStats(const QList<int> &lSites)
{
    Q_D(PATOutlierFinderFT);

    if (d->mInitialized)
    {
        QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;

        // Compute stats for all testing sites...
        CPatDefinition *    lPatDef     = NULL;
        CTest          *    lTestCell   = NULL;

        for(int lIdx = 0; lIdx < lSites.count(); ++lIdx)
        {
            // Force to point relevant group & file (based in Site# to process).
            // As calling function may be final-test (where multiple calls can be done in production)
            // Need to always update pointer, even if addressing same site#.
            if (UpdateDatasetPointer(lSites[lIdx]) == false)
                return false;

            // Compute stats for all tests in a given site#
            for(itPATDefinifion = d->mContext->GetUnivariateRules().begin();
                itPATDefinifion != d->mContext->GetUnivariateRules().end(); ++itPATDefinifion)
            {
                lPatDef     = *itPATDefinifion;
                lTestCell   = NULL;

                if(lPatDef)
                {
                    lTestCell = FindTestCell(lPatDef);

                    if(lTestCell)
                        d->ComputeTestStatistics(lTestCell, d->mPartFilter);
                }
            }
        }

        return true;
    }

    return false;
}

bool PATOutlierFinderFT::ComputeMultiSiteDynamicLimits(const QList<int> &lSites,
                                                const QList<int> &lSitesFrom)
{
    Q_D(PATOutlierFinderFT);

    if (d->mInitialized == false)
        return false;

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Computing MultiSite Dynamic limits for %1 sites using %2 sites")
          .arg(lSites.count()).arg(lSitesFrom.count()).toLatin1().constData());

    if (lSites.isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("No sites given in order to compute baseline").toLatin1().constData());
        return false;
    }

    foreach(int lSite, lSites)
    {
        if (lSitesFrom.contains(lSite) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Site# %1 is not included in the list of site to use for Multi-site " \
                          "baseline computation.").arg(lSite).toLatin1().constData());
            return false;
        }
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Site %1 used as reference site").arg(lSites.first()).toLatin1().constData());

    // 7208 [BG]: CSiteTestResults->SiteTestResults (different constructor)
//    d->mFTMultiSite = new SiteTestResults(NULL, 255, 0);
    d->mFTMultiSite = new SiteTestResults(-1, 255, 0, d->mContext->GetRecipeOptions().mTestKey);

    // Compute Dynamic PAT limits
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
    QMap<QString, double>                       lOffsets;
    CPatDefinition *    lPatDef         = NULL;
    CTest *             lTestCell       = NULL;
    CTest *             lTestCellMulti  = NULL;
    int                 lSite           = -1;
    QString             lKey;

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Building merged datasets by adjusting all sites together").toLatin1().constData());

    // Compute DPAT limits for all testing sites...
    // Compute median for all site
    for(itPATDefinifion = d->mContext->GetUnivariateRules().begin();
        itPATDefinifion != d->mContext->GetUnivariateRules().end(); ++itPATDefinifion)
    {
        double lMedianRef   = 0.0;
        double lMedianSite  = 0.0;

        lPatDef = *itPATDefinifion;

        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Creating merged datasets for Test %1.%2 '%3'...")
              .arg(lPatDef->m_lTestNumber).arg(lPatDef->mPinIndex).arg(lPatDef->m_strTestName)
              .toLatin1().data() );

        for(int lIdx = 0; lIdx < lSitesFrom.count(); lIdx++)
        {
            QVector<double> lDataset;
            double          lOffset = 0.0;

            // Site#
            lSite = lSitesFrom.at(lIdx);

            // Force to point relevant dataset (based in Site# to process).
            // As calling function may be final-test (where multiple calls can be done in production)
            // Need to always update pointer, even if addressing same site#.
            if (UpdateDatasetPointer(lSite) == false)
                return false;

            lTestCell = FindTestCell(lPatDef);

            // Build a QVector with all valid execution for this site
            d->BuildDataset(lTestCell, lDataset);

            qSort(lDataset);

            // Get Median of the site
            if (lIdx == 0)
            {
                lMedianRef = GEX_C_DOUBLE_NAN;
                if (lDataset.size()>0)
                    lMedianRef = algorithms::gexMedianValue(lDataset);

                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Median value for reference site %1 on Test %2.%3: %4")
                      .arg(lSite).arg(lTestCell->lTestNumber).arg(lPatDef->mPinIndex)
                      .arg(lMedianRef).toLatin1().constData());
            }
            else
            {
                lMedianSite = GEX_C_DOUBLE_NAN;
                if (lDataset.size()>0)
                    lMedianSite = algorithms::gexMedianValue(lDataset);
                lOffset = lMedianSite - lMedianRef;

                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Median value for site %1 on Test %2.%3: %4")
                      .arg(lSite).arg(lTestCell->lTestNumber).arg(lPatDef->mPinIndex).arg(lMedianSite)
                      .toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("Offets computed for site %1 on Test %2.%3: %4")
                      .arg(lSite).arg(lTestCell->lTestNumber).arg(lPatDef->mPinIndex).arg(lOffset)
                      .toLatin1().constData());
            }

            // Keep value of the median for the current pair test/PinIndex/site
            lKey = QString::number(lTestCell->lTestNumber);
            lKey += "." + QString::number(lTestCell->lPinmapIndex);
            lKey += ":" + QString::number(lSite);

            lOffsets.insert(lKey, lOffset);

            // 7208 [BG]: No need to check PatInfo->cOptionsPat.mTestKey anymore. Checked inside the site.
            lTestCellMulti = d->mFTMultiSite->FindTestCell(lTestCell->lTestNumber,
                                                           lTestCell->lPinmapIndex, true, false,
                                                           lTestCell->strTestName);

            GSLOG(SYSLOG_SEV_DEBUG,
                  QString("Inserting data results from site %2").arg(lSite).toLatin1().constData());

            // Update Test result
            d->UpdateMultiSiteTestResult(lTestCellMulti, lDataset, lOffset);
        }
    }

    // Compute DPAT limits on mutli site test result
    QList<int> lMultiSite;

    lMultiSite.append(255);

    if (ComputePatLimits(lMultiSite, false) == false)
    {
        if (d->mFTMultiSite)
        {
            delete d->mFTMultiSite;
            d->mFTMultiSite = NULL;
        }

        return false;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Adjusting DPAT limits per site").toLatin1().constData());

    // Set DPAT limits for all site adjusted with the median offset
    for(int lIdx = 0; lIdx < lSites.count(); lIdx++)
    {
        // Site#
        lSite = lSites.at(lIdx);

        double lOffset = 0.0;

        // Force to point relevant dataset (based in Site# to process).
        // As calling function may be final-test (where multiple calls can be done in production)
        // Need to always update pointer, even if addressing same site#.
        if (UpdateDatasetPointer(lSite) == false)
        {
            if (d->mFTMultiSite)
            {
                delete d->mFTMultiSite;
                d->mFTMultiSite = NULL;
            }

            return false;
        }

        GS::PAT::DynamicLimits  lDynLimits;

        for(itPATDefinifion = d->mContext->GetUnivariateRules().begin();
            itPATDefinifion != d->mContext->GetUnivariateRules().end(); ++itPATDefinifion)
        {
            lPatDef     = *itPATDefinifion;
            lTestCell   = FindTestCell(lPatDef);

            if (lTestCell)
            {
                GSLOG(SYSLOG_SEV_DEBUG,
                      QString("For Test %1.%2 on site %3 with offset %4")
                      .arg(lTestCell->lTestNumber).arg(lPatDef->mPinIndex).arg(lSite)
                      .arg(lOffset).toLatin1().constData());

                lKey        = QString::number(lTestCell->lTestNumber);
                lKey        += "." + QString::number(lTestCell->lPinmapIndex);
                lKey        += ":" + QString::number(lSite);

                lOffset     = lOffsets.value(lKey);
                lDynLimits  = lPatDef->mDynamicLimits.value(255);

                // Adjust limits using median offset
                for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
                    lIdx < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
                    ++lIdx)
                {
                    if (lDynLimits.mLowDynamicLimit1[lIdx] != -GEX_TPAT_DOUBLE_INFINITE)
                        lDynLimits.mLowDynamicLimit1[lIdx]   += lOffset;

                    if (lDynLimits.mLowDynamicLimit2[lIdx] != -GEX_TPAT_DOUBLE_INFINITE)
                        lDynLimits.mLowDynamicLimit2[lIdx]   += lOffset;

                    if (lDynLimits.mHighDynamicLimit1[lIdx] != GEX_TPAT_DOUBLE_INFINITE)
                        lDynLimits.mHighDynamicLimit1[lIdx]  += lOffset;

                    if (lDynLimits.mHighDynamicLimit2[lIdx] != GEX_TPAT_DOUBLE_INFINITE)
                        lDynLimits.mHighDynamicLimit2[lIdx]  += lOffset;
                }

                // Saves dynamic PAT limits computed for this testing site + save distribution shape detected
                lPatDef->SetDynamicLimits(lSite, lDynLimits, d->mContext->GetRecipeOptions(),
                                             lTestCell->GetCurrentLimitItem()->bLimitFlag, lTestCell->GetCurrentLimitItem()->lfLowLimit,
                                             lTestCell->GetCurrentLimitItem()->lfHighLimit);
            }
        }
    }

    if (d->mFTMultiSite)
    {
        delete d->mFTMultiSite;
        d->mFTMultiSite = NULL;
    }

    return true;
}

}
}

#endif
