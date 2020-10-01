#ifdef GCORE15334

#ifndef PAT_OUTLIER_FINDER_PRIVATE_H
#define PAT_OUTLIER_FINDER_PRIVATE_H

#include "cstats.h"
#include "pat_plugins.h"
#include "pat_part_filter.h"

class CPatInfo;
class CPatDefinition;

namespace GS
{

namespace PAT
{
class DynamicLimits;
}

namespace Gex
{

class PATPartFilter;

class PATOutlierFinderPrivate
{
public:

    PATOutlierFinderPrivate();
    virtual ~PATOutlierFinderPrivate();

    bool                    mInitialized;
    bool                    mUsePercentileCPK;
    CPatInfo *              mContext;
    PATPartFilter *         mPartFilter;
    GexExternalPat          mExternalPat;
    CGexStats               mStatsEngine;

    bool                    ComputeTestStatistics(CTest * lTestCell, PATPartFilter * lPartFilter = NULL);
    virtual int             GetRequiredSamples() const = 0;
    virtual bool            HasRequiredSamples(const CTest &lTestCell) const = 0;
    virtual void            PreComputeDynamicLimits(CTest * lTestCell) = 0;
    virtual void            PostComputeDynamicLimits(CTest * lTestCell) = 0;
    virtual void            OnHighCPKDetected(CPatDefinition& lPatDef, CTest& lTestCell,
                                              GS::PAT::DynamicLimits&	lDynLimits) = 0;
};

}   // namespace Gex
}   // namespace GS

#endif // PAT_OUTLIER_FINDER_PRIVATE_H
#endif
