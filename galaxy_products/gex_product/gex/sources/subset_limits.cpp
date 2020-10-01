///////////////////////////////////////////////////////////////////////////////////
// GEX includes
///////////////////////////////////////////////////////////////////////////////////
#include "subset_limits.h"

namespace GS
{
namespace Gex
{


SubsetLimits::~SubsetLimits()
{
    mLimits.clear();
}



bool SubsetLimits::GetLimits(int runId, RunIdLimits& limits)
{
    QMap<int, RunIdLimits>::const_iterator elt = mLimits.find(runId);
    if (elt != mLimits.end())
    {
        limits = elt.value();
        return true;
    }
    else
        return false;
}



void SubsetLimits::AddLimits(int runId, double ll, double hl)
{
    CGexSiteLimits limit(ll, hl);
    AddSubsetLimitsByRunId(runId, limit);
}



void SubsetLimits::AddSubsetLimitsByRunId(int runId, CGexSiteLimits limit)
{
    RunIdLimits limits;
    TestSubsetLimits::const_iterator it = mLimits.find(runId);
    // the first time we insert an element with runId
    if (it == mLimits.end())
    {
        limits.append(limit);
        mLimits.insert(runId, limits);
    }
    else    // The map contains an element with runId as key
    {
        limits = it.value();
        limits.push_back(limit);
        mLimits.insert(runId, limits);
    }
}



void SubsetLimits::AddSubsetLimitsByRunRange(int firstRange, int lastRange, CGexSiteLimits limit)
{
    for (int id=firstRange; id<=lastRange; id++)
    {
        AddSubsetLimitsByRunId(id, limit);
    }

}


void SubsetLimits::RemoveAllLimits()
{
    mLimits.clear();
}


} // Gex
} // GS
