#include "pat_reticle_corner_rule.h"
#include "pat_reticle_map_abstract.h"
#include "gqtl_log.h"

#include <qglobal.h>
#include <QJsonArray>

namespace GS
{
namespace Gex
{

uint qHash(const WaferCoordinate& coordinate)
{
    return qHash(QString::number(coordinate.GetX()) + QString::number(coordinate.GetY()));
}

PatReticleCornerRule::PatReticleCornerRule(const PATOptionReticle &reticleSettings)
    : PatReticleAbstractAlgorithm(reticleSettings)
{

}

PatReticleCornerRule::~PatReticleCornerRule()
{

}

bool PatReticleCornerRule::ProcessReticleMap(PatReticleMapAbstract *reticleMap)
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

        // Compute results for report purpose
        ComputeResults();

        return true;
    }

    return false;
}

bool PatReticleCornerRule::Init(PatReticleMapAbstract *reticleMap)
{
    if (reticleMap)
    {
        mReticleMap = reticleMap;
        mReticleResults.remove("values");

        return mReticleMap->Init();
    }

    return false;
}

void PatReticleCornerRule::AddOutliers(const WaferCoordinate &die)
{
    if (mOutliers.contains(die) == false)
    {
        WaferCoordinate lReticleCoord = mReticleMap->GetReticleCoordinate(die);

        // Compute a map index
        int lMapIndex = lReticleCoord.GetX() + lReticleCoord.GetY() * mReticleMap->GetReticleWidth();  // YYXX

        // Keep track of #of dies over the wafer for this reticle location
        mOutlierDieCount[lMapIndex]++;

        // Keep track of the oultier detected
        mOutliers.append(die);
    }
}

void PatReticleCornerRule::InkX(const WaferCoordinate &cornerDie, int lLeft, int lRight)
{
    WaferCoordinate lDie = cornerDie;

    while (lLeft > 0)
    {
        lDie.Translate(-1, 0);
        if (mReticleMap->IsValidWaferCoord(lDie) && mReticleMap->IsInReticleField(lDie))
            AddOutliers(lDie);

        --lLeft;
    }

    lDie = cornerDie;
    while (lRight > 0)
    {
        lDie.Translate(1, 0);
        if (mReticleMap->IsValidWaferCoord(lDie) && mReticleMap->IsInReticleField(lDie))
            AddOutliers(lDie);

        --lRight;
    }
}

void PatReticleCornerRule::InkY(const WaferCoordinate &cornerDie, int lTop, int lBottom)
{
    WaferCoordinate lDie = cornerDie;

    while (lTop > 0)
    {
        lDie.Translate(0, -1);
        if (mReticleMap->IsValidWaferCoord(lDie) && mReticleMap->IsInReticleField(lDie))
            AddOutliers(lDie);

        --lTop;
    }

    lDie = cornerDie;
    while (lBottom > 0)
    {
        lDie.Translate(0, 1);
        if (mReticleMap->IsValidWaferCoord(lDie) && mReticleMap->IsInReticleField(lDie))
            AddOutliers(lDie);

        --lBottom;
    }
}

void PatReticleCornerRule::InkDiag(const WaferCoordinate &cornerDie, int lDiag, int lXOff, int lYOff)
{
    WaferCoordinate lDieTR = cornerDie;
    WaferCoordinate lDieTL = cornerDie;
    WaferCoordinate lDieBR = cornerDie;
    WaferCoordinate lDieBL = cornerDie;

    while (lDiag > 0)
    {
        lDieTR.Translate(1, -1);
        if (mReticleMap->IsValidWaferCoord(lDieTR) && mReticleMap->IsInReticleField(lDieTR))
        {
            AddOutliers(lDieTR);
            InkX(lDieTR, lXOff, lXOff);
            InkY(lDieTR, lYOff, lYOff);
        }

        lDieTL.Translate(-1, -1);
        if (mReticleMap->IsValidWaferCoord(lDieTL) && mReticleMap->IsInReticleField(lDieTL))
        {
            AddOutliers(lDieTL);
            InkX(lDieTL, lXOff, lXOff);
            InkY(lDieTL, lYOff, lYOff);
        }

        lDieBR.Translate(1, 1);
        if (mReticleMap->IsValidWaferCoord(lDieBR) && mReticleMap->IsInReticleField(lDieBR))
        {
            AddOutliers(lDieBR);
            InkX(lDieBR, lXOff, lXOff);
            InkY(lDieBR, lYOff, lYOff);
        }

        lDieBL.Translate(-1, 1);
        if (mReticleMap->IsValidWaferCoord(lDieBL) && mReticleMap->IsInReticleField(lDieBL))
        {
            AddOutliers(lDieBL);
            InkX(lDieBL, lXOff, lXOff);
            InkY(lDieBL, lYOff, lYOff);
        }

        --lDiag;
    }
}

void PatReticleCornerRule::FindOutliers(const WaferCoordinate& cornerDie)
{
    int lXInk       = mReticleSettings.GetXInk();
    int lYInk       = mReticleSettings.GetYInk();
    int lDiagInk    = mReticleSettings.GetDiagInk();


    // Apply x-ink
    if (lXInk > 0)
    {
        InkX(cornerDie, lXInk, lXInk);
    }

    // Apply y-ink
    if (lYInk > 0)
    {
        InkY(cornerDie, lYInk, lYInk);
    }

    // Apply diag Ink
    if (lDiagInk > 0)
    {
        InkDiag(cornerDie, lDiagInk, mReticleSettings.GetXOffDiag(), mReticleSettings.GetYOffDiag());
    }
}

void PatReticleCornerRule::ComputeResults()
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
        if (mTotalDieCount.contains(lMapIndex) && mTotalDieCount[lMapIndex] > 0)
        {
            if (mOutlierDieCount.contains(lMapIndex))
                lYield = 100.0 * ((double) mOutlierDieCount[lMapIndex] / (double) mTotalDieCount[lMapIndex]);
            else
                lYield = 0.0;
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
            // Save Bad Yield per reticle location
            lDieBadYield.insert("x", QJsonValue(lReticleCoord.GetX()));
            lDieBadYield.insert("y", QJsonValue(lReticleCoord.GetY()));
            lDieBadYield.insert("z", QJsonValue(lYield));
            lReticleBadYield.append(lDieBadYield);
        }
    }

    mReticleResults.insert("values", lReticleBadYield);
}

bool PatReticleCornerRule::ProcessReticle()
{
    // Clear the list of reticle die processed in other reticles
    mReticleDieProcessed.clear();

    // Compute Die count for this reticle field
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
            }
        }
    }

    // Identify corner and edge rule that needs to be analyzed
    lBin = GEX_WAFMAP_EMPTY_CELL;
    QList<WaferCoordinate> lCornerDie;

    if (mReticleSettings.IsActivatedCorner(PATOptionReticle::CORNER_TOP_LEFT))
    {
        lCornerDie.append(mReticleMap->GetTopLeftDie());
    }

    if (mReticleSettings.IsActivatedCorner(PATOptionReticle::CORNER_TOP_RIGHT))
    {
        lCornerDie.append(mReticleMap->GetTopRightDie());
    }

    if (mReticleSettings.IsActivatedCorner(PATOptionReticle::CORNER_BOTTOM_LEFT))
    {
        lCornerDie.append(mReticleMap->GetBottomLeftDie());
    }

    if (mReticleSettings.IsActivatedCorner(PATOptionReticle::CORNER_BOTTOM_RIGHT))
    {
        lCornerDie.append(mReticleMap->GetBottomRightDie());
    }

    if (mReticleSettings.IsActivatedCorner(PATOptionReticle::TOP) ||
        mReticleSettings.IsActivatedCorner(PATOptionReticle::BOTTOM))
    {
        for (int lXCoord = mReticleMap->GetTopLeftDie().GetX() + 1;
             lXCoord < mReticleMap->GetTopRightDie().GetX(); ++lXCoord)
        {
            if (mReticleSettings.IsActivatedCorner(PATOptionReticle::TOP))
                lCornerDie.append(WaferCoordinate(lXCoord, mReticleMap->GetTopLeftDie().GetY()));

            if (mReticleSettings.IsActivatedCorner(PATOptionReticle::BOTTOM))
                lCornerDie.append(WaferCoordinate(lXCoord, mReticleMap->GetBottomLeftDie().GetY()));
        }
    }

    if (mReticleSettings.IsActivatedCorner(PATOptionReticle::LEFT) ||
        mReticleSettings.IsActivatedCorner(PATOptionReticle::RIGHT))
    {
        for (int lYCoord = mReticleMap->GetTopLeftDie().GetY() + 1;
             lYCoord < mReticleMap->GetBottomLeftDie().GetY(); ++lYCoord)
        {
            if (mReticleSettings.IsActivatedCorner(PATOptionReticle::LEFT))
                lCornerDie.append(WaferCoordinate(mReticleMap->GetTopLeftDie().GetX(), lYCoord));

            if (mReticleSettings.IsActivatedCorner(PATOptionReticle::RIGHT))
                lCornerDie.append(WaferCoordinate(mReticleMap->GetTopRightDie().GetX(), lYCoord));
        }

    }

    // Check bin value for each edge or corner die activated
    for (int lIdx = 0; lIdx < lCornerDie.count(); ++lIdx)
    {
        lBin = mReticleMap->GetBin(lCornerDie.at(lIdx));

        // Find out surrounding dies that are failed
        if (lBin != GEX_WAFMAP_EMPTY_CELL && mReticleSettings.GetBadBinsReticleList().Contains(lBin) == true)
            ProcessDie(lCornerDie.at(lIdx));
    }

    return true;
}

bool PatReticleCornerRule::ProcessDie(const WaferCoordinate& die)
{
    if (mReticleDieProcessed.contains(die) == false)
    {
        mReticleDieProcessed.insert(die);

        int lBin = mReticleMap->GetBin(die);

        if (lBin != GEX_WAFMAP_EMPTY_CELL && mReticleSettings.GetBadBinsReticleList().Contains(lBin) == true)
        {
            // Find die that will be rebinned as PAT die around this one
            FindOutliers(die);

            // Find out surrounding die that could create a cluster from this given die
            QList<WaferCoordinate> lSurroundingDie = mReticleMap->GetSurroundingCoordinates(die,
                                                                                            mReticleSettings.IgnoreDiagonalBadDies());

            for (int lIdx = 0; lIdx < lSurroundingDie.count(); ++lIdx)
            {
                // Find out surrounding dies that are failed
                ProcessDie(lSurroundingDie.at(lIdx));
            }
        }
    }

    return true;
}

}
}
