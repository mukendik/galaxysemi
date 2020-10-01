#ifndef PAT_DYNAMIC_LIMITS_H
#define PAT_DYNAMIC_LIMITS_H

#include "pat_defines.h"

namespace GS
{
namespace PAT
{

//! \class Structure that holds the two sets of limits to apply to samples.
//! \brief Most of the time, only the first set is used, the second set is only used when processing a clear bi_modal distribution (with each mode apart)
class DynamicLimits
{
public:
    DynamicLimits();	// Constructor
    ~DynamicLimits();

    double mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];
    double mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];
    double mLowDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];
    double mHighDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];
    double mHighRelaxed;	// Relaxed High limit (in case trailing tail detected)
    double mLowRelaxed;	// Relaxed High limit (in case trailing tail detected)
    double mMean;	// Mean (or median) value used when computing limits
    double mSigma;	// Sigma (or robust sigma) used when computing limits
    double mDynMean;	// Mean (or median) value used when computing limits
    double mDynSigma;	// Sigma used when computing limits
    double mDynQ1;	// Q1
    double mDynQ2;	// Q2: median
    double mDynQ3;	// Q3
    int	   mDistributionShape;	// Distribution shape detected for a given distribution & site#
};

}
}
#endif // PAT_DYNAMIC_LIMITS_H
