#ifdef GCORE15334

#include "pat_outlier_finder_ft_private.h"
#include "pat_info.h"


namespace GS
{
namespace Gex
{

PATOutlierFinderFTPrivate::PATOutlierFinderFTPrivate()
    : PATOutlierFinderPrivate(), mFTMultiSite(NULL), mFTCurrentSite(NULL), mFTAllSites(NULL)
{
}

PATOutlierFinderFTPrivate::~PATOutlierFinderFTPrivate()
{
}

bool PATOutlierFinderFTPrivate::Init(CPatInfo *lContext, QMap<int, SiteTestResults *> *lFTAllSites)
{
    if (lContext == NULL)
        return false;

    if (lFTAllSites == NULL)
        return false;

    mContext    = lContext;
    mFTAllSites = lFTAllSites;

    // Initialisation done successfully
    mInitialized = true;

    return mInitialized;
}

int PATOutlierFinderFTPrivate::GetRequiredSamples() const
{
    return 1;
}

bool PATOutlierFinderFTPrivate::HasRequiredSamples(const CTest &lTestCell) const
{
    return lTestCell.ldSamplesValidExecs >= GetRequiredSamples();
}

void PATOutlierFinderFTPrivate::PreComputeDynamicLimits(CTest *lTestCell)
{
    // reset shape
    lTestCell->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
    // Compute test statistics for this test
    ComputeTestStatistics(lTestCell, mPartFilter);
}

void PATOutlierFinderFTPrivate::PostComputeDynamicLimits(CTest *lTestCell)
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

void PATOutlierFinderFTPrivate::OnHighCPKDetected(CPatDefinition &/*lPatDef*/, CTest &/*lTestCell*/,
                                           GS::PAT::DynamicLimits &/*lDynLimits*/)
{
    // Do nothing at FT stage
}

void PATOutlierFinderFTPrivate::BuildDataset(CTest *lTest, QVector<double> &lDataset)
{
    if (lTest)
    {
        lDataset.reserve(lTest->ldSamplesValidExecs);

        // Extract valid data samples...
        for(int lIdx = 0; lIdx < lTest->m_testResult.count(); ++lIdx)
        {
            if (lTest->m_testResult.isValidResultAt(lIdx))
            {
                if (lTest->m_testResult.isMultiResultAt(lIdx))
                {
                    for (int lCount = 0; lCount < lTest->m_testResult.at(lIdx)->count(); ++lCount)
                    {
                        if(lTest->m_testResult.at(lIdx)->isValidResultAt(lCount))
                            lDataset.append(lTest->m_testResult.at(lIdx)->multiResultAt(lCount));
                    }
                }
                else
                    lDataset.append(lTest->m_testResult.resultAt(lIdx));

            }
        }

        lDataset.squeeze();
    }
}

void PATOutlierFinderFTPrivate::UpdateMultiSiteTestResult(CTest *lTest, QVector<double> &lDataset, double lOffset)
{
    if (lTest)
    {
        int lIndex = lTest->m_testResult.count();

        if (lTest->m_testResult.count() == 0)
            lTest->m_testResult.createResultTable(lDataset.count());
        else
            lTest->m_testResult.resizeResultTable(lTest->m_testResult.count() + lDataset.count());

        foreach(double lValue, lDataset)
        {
            lTest->m_testResult.pushResultAt(lIndex, lValue - lOffset);
            ++lIndex;
        }
    }
}

}
}
#endif
