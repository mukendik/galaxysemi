#ifdef GCORE15334

#ifndef PAT_OUTLIER_FINDER_FT_PRIVATE_H
#define PAT_OUTLIER_FINDER_FT_PRIVATE_H

#include "pat_outlier_finder_private.h"

class CTest;

namespace GS
{
namespace Gex
{

class SiteTestResults;

class PATOutlierFinderFTPrivate : public PATOutlierFinderPrivate
{
public:

    PATOutlierFinderFTPrivate();
    virtual ~PATOutlierFinderFTPrivate();

    SiteTestResults *              mFTMultiSite;   // To store pointer for Multi-site baseline
    SiteTestResults *              mFTCurrentSite; // To store pointer to a given site structure (thaholding CTest structures & samples)
    QMap<int, SiteTestResults*> *  mFTAllSites;	// Pointer to ALL sites structures with samples.

    bool            Init(CPatInfo * lContext, QMap<int, SiteTestResults*> *  lFTAllSites);

    int             GetRequiredSamples() const;
    bool            HasRequiredSamples(const CTest& lTestCell) const;
    void            PreComputeDynamicLimits(CTest * lTestCell);
    void            PostComputeDynamicLimits(CTest * lTestCell);
    void            OnHighCPKDetected(CPatDefinition& lPatDef, CTest& lTestCell,
                                      GS::PAT::DynamicLimits& lDynLimits);

    void            BuildDataset(CTest * lTest, QVector<double>& lDataset);
    void            UpdateMultiSiteTestResult(CTest * lTest, QVector<double>& lDataset, double lOffset);
};

}
}
#endif // PAT_OUTLIER_FINDER_FT_PRIVATE_H
#endif
