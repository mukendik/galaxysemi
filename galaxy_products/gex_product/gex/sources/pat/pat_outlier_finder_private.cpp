#ifdef GCORE15334

#include "pat_outlier_finder_private.h"
#include "pat_info.h"


extern double       ScalingPower(int iPower);

namespace GS
{
namespace Gex
{

PATOutlierFinderPrivate::PATOutlierFinderPrivate()
    : mInitialized(false), mUsePercentileCPK(false), mContext(NULL), mPartFilter(NULL)
{
}

PATOutlierFinderPrivate::~PATOutlierFinderPrivate()
{
    if (mPartFilter)
    {
        delete mPartFilter;
        mPartFilter = NULL;
    }
}

bool PATOutlierFinderPrivate::ComputeTestStatistics(CTest * lTestCell, PATPartFilter * lPartFilter /*= NULL*/)
{
    PAT_PERF_BENCH

    if (lTestCell)
    {
        // Update all statistics fields as so far only samples and lows stats are accurate (mean, sigma, quartiles, histo not initialized yet)
        double lExponent = ScalingPower(lTestCell->res_scal);

        mStatsEngine.ComputeLowLevelTestStatistics(lTestCell, lExponent, lPartFilter);
        mStatsEngine.ComputeBasicTestStatistics(lTestCell, true, lPartFilter);
        mStatsEngine.RebuildHistogramArray(lTestCell,GEX_HISTOGRAM_OVERDATA, lPartFilter);
        mStatsEngine.ComputeAdvancedDataStatistics(lTestCell, true, mUsePercentileCPK, false, lPartFilter);

        return true;
    }

    return false;
}

}
}
#endif
