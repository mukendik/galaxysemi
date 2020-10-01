#include "pat_dynamic_limits.h"

namespace GS
{
namespace PAT
{

DynamicLimits::DynamicLimits()
{
    for(int lSeverity = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        lSeverity < GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES;
        lSeverity++)
    {
        mLowDynamicLimit1[lSeverity]  = mLowDynamicLimit2[lSeverity]  = -GEX_TPAT_DOUBLE_INFINITE;
        mHighDynamicLimit1[lSeverity] = mHighDynamicLimit2[lSeverity] = GEX_TPAT_DOUBLE_INFINITE;
    }

    // Relaxed limits (in case trailing tail detected)
    mHighRelaxed = -GEX_TPAT_DOUBLE_INFINITE;
    mLowRelaxed  = GEX_TPAT_DOUBLE_INFINITE;

    // Statistics
    mMean       = 0;
    mSigma      = 0;
    mDynMean    = 0;
    mDynSigma   = 0;
    mDynQ1      = GEX_TPAT_DOUBLE_INFINITE;
    mDynQ2      = GEX_TPAT_DOUBLE_INFINITE;
    mDynQ3      = GEX_TPAT_DOUBLE_INFINITE;

    // Distribution shape detected for given site#
    mDistributionShape = PATMAN_LIB_SHAPE_GAUSSIAN;	// Default: gaussian
}

DynamicLimits::~DynamicLimits()
{

}

}
}
