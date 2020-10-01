#include "pat_reticle_repeating_pattern.h"
#include "pat_reticle_map_abstract.h"
#include "wafermap.h"
#include "gqtl_log.h"

#include <QJsonArray>

namespace GS
{
namespace Gex
{

PatReticleRepeatingPattern::PatReticleRepeatingPattern(const PATOptionReticle &reticleSettings)
    : PatReticleAbstractAlgorithm(reticleSettings)
{
}

PatReticleRepeatingPattern::~PatReticleRepeatingPattern()
{

}

bool PatReticleRepeatingPattern::Init(PatReticleMapAbstract *reticleMap)
{
    if (reticleMap)
    {
        mReticleMap = reticleMap;
        mReticleResults.remove("values");

        return mReticleMap->Init();
    }

    return false;
}

bool PatReticleRepeatingPattern::ProcessReticleMap(PatReticleMapAbstract * reticleMap)
{
    // Loop over all individual reticle
    if (Init(reticleMap))
    {
        do
        {
            // process single reticle
            if (ProcessReticle() == false)
                return false;
        }
        while (reticleMap->Next());

        // Identify all outliers
        FindOutliers();

        return true;
    }

    return false;
}

void PatReticleRepeatingPattern::FindOutliers()
{
    // Prepare buffer to save Reticle yield levels
    unsigned int lReticleSize = mReticleMap->GetReticleWidth() * mReticleMap->GetReticleHeight();

    // Check which reticle locations are below the yield limit
    double          lYield;
    WaferCoordinate lReticleCoord;
    QJsonArray      lReticleBadYield;
    QJsonObject     lDieBadYield;

    for (unsigned int lMapIndex = 0; lMapIndex < lReticleSize; lMapIndex++)
    {
        // Compute yield level for reticle location
        if (mTotalDieCount[lMapIndex])
        {
            lYield = 100.0 * ((double) mGoodDieCount[lMapIndex] / (double) mTotalDieCount[lMapIndex]);
        }
        else
        {
            lYield = -1;	// No die for this location, force yield level under any possible alarm threshold
        }

        // Compute X and Y location in the reticle
        lReticleCoord.SetX(lMapIndex % mReticleMap->GetReticleWidth());
        lReticleCoord.SetY(lMapIndex / mReticleMap->GetReticleWidth());

        // Compute Bad-Bins yield level
        if (lYield >= 0)
        {
            lYield = 100-lYield;

            // Save Bad Yield per reticle location
            lDieBadYield.insert("x", QJsonValue(lReticleCoord.GetX()));
            lDieBadYield.insert("y", QJsonValue(lReticleCoord.GetY()));
            lDieBadYield.insert("z", QJsonValue(lYield));
            lReticleBadYield.append(lDieBadYield);
        }

        // Low-yield location: Need to reject this die location all over the wafer...
        // Scan Hard-Bin wafermap (to ensure Bin1 is all Good bins (can be multiple soft bins)
        if(lYield > mReticleSettings.GetReticleYieldThreshold())
        {
            mOutliers.append(mReticleMap->GetWaferCoordinates(lReticleCoord));
        }
    }

    mReticleResults.insert("values", lReticleBadYield);
}

bool PatReticleRepeatingPattern::ProcessReticle()
{
    int lBin;
    int lMapIndex;

    for (int lYCoord = mReticleMap->GetTopLeftDie().GetY(); lYCoord <= mReticleMap->GetBottomRightDie().GetY(); ++lYCoord)
    {
        for (int lXCoord = mReticleMap->GetTopLeftDie().GetX(); lXCoord <= mReticleMap->GetBottomRightDie().GetX(); ++lXCoord)
        {
            // Get die value SoftBin#
            lBin = mReticleMap->GetBin(lXCoord, lYCoord);

            // Keep track of valid dies at given reticle location
            if(lBin != GEX_WAFMAP_EMPTY_CELL)
            {
                WaferCoordinate lReticleCoord = mReticleMap->GetReticleCoordinate(WaferCoordinate(lXCoord, lYCoord));

                // Compute a map index
                lMapIndex = lReticleCoord.GetX() + lReticleCoord.GetY() * mReticleMap->GetReticleWidth();  // YYXX

                // Keep track of #of dies over the wafer for this reticle location
                mTotalDieCount[lMapIndex]++;

                // We've got a good bin, then get total fail bins in the cluster zone.
                if(mReticleSettings.GetBadBinsReticleList().Contains(lBin) == false)
                {
                    // Keep track of total good dies at reticle location.
                    mGoodDieCount[lMapIndex]++;
                }
            }
        }
    }

    return true;
}


}
}
