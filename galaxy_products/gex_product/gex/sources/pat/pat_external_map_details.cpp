#include "pat_external_map_details.h"

namespace GS
{
namespace Gex
{
namespace PAT
{

ExternalMapDetails::ExternalMapDetails()
{
    Clear();
}

ExternalMapDetails::~ExternalMapDetails()
{

}

void ExternalMapDetails::Clear()
{
    mComputed                   = false;
    mGoodPartsMapDPATFailures   = 0;
    mGoodPartsMapSTDFMissing    = 0;
    mTotalDies                  = 0;
    mTotalMatchingMapDiesLoc    = 0;
    mTotalMismatchingMapPassBin = 0;
    mTotalMismatchingMapFailBin = 0;

    mBinCountAfterPAT.clear();
    mBinCountBeforePAT.clear();
}

}   // namespace PAT
}   // namespace Gex
}   // namespace GS
