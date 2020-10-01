#ifndef PAT_EXTERNAL_MAP_DETAILS_H
#define PAT_EXTERNAL_MAP_DETAILS_H

#include <QMap>

namespace GS
{
namespace Gex
{
namespace PAT
{

class ExternalMapDetails
{
public:

    ExternalMapDetails();
    ~ExternalMapDetails();

    void    Clear();

    bool    mComputed;
    int		mTotalDies;                     // Total dies in MAP file
    int		mTotalMatchingMapDiesLoc;       // Holds total matching dies between STDF and MAP files
    int		mTotalMismatchingMapPassBin;
    int		mTotalMismatchingMapFailBin;
    int		mGoodPartsMapDPATFailures;      // number of good die from the input wafer map that become bad die (dpat failures) in the output wafer map.
    int		mGoodPartsMapSTDFMissing;       // number of good die from the input wafer map that do not have matching STDF data.

    QMap<int,int> mBinCountBeforePAT;       // Holds list of bin count of the input map Before PAT
    QMap<int,int> mBinCountAfterPAT;        // Holds list of bin count of the input map After PAT

};

}   // namespace PAT
}   // namespace Gex
}   // namespace GS
#endif // PAT_EXTERNAL_MAP_DETAILS_H
