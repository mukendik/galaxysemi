#include "pat_reticle_map_extern.h"
#include "gqtl_log.h"

namespace GS
{
namespace Gex
{

PatReticleMapExtern::PatReticleMapExtern(const CWaferMap *waferMap, const CWaferMap *waferReticle)
    : PatReticleMapAbstract(), mWafermapData(waferMap), mWafermapReticle(waferReticle)
{

}

PatReticleMapExtern::~PatReticleMapExtern()
{

}

bool PatReticleMapExtern::Init()
{
    if (mWafermapData && mWafermapReticle && mWafermapReticle->HasReticle())
    {
        const CWafMapArray * lWafMapArray = mWafermapData->getWafMap();

        if (lWafMapArray == NULL)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to get wafermap array");
            return false;
        }

        int     lCoordX;
        int     lCoordY;
        int     lBin            = GEX_WAFMAP_EMPTY_CELL;
        bool    lFindOrigin     = false;
        int     lWafermapSize   = mWafermapData->GetSizeX() * mWafermapData->GetSizeY();

        for(int lIdx = 0; lIdx < lWafermapSize && lFindOrigin == false; ++lIdx)
        {
            // Get die value SoftBin#
            lBin = lWafMapArray[lIdx].getBin();

            // Keep track of valid dies at given reticle location
            if(lBin != GEX_WAFMAP_EMPTY_CELL)
            {
                // If first die in wafer, this is where the reticle origin is!
                // Save reticle origine in array
                if (mWafermapData->coordFromIndex(lIdx, lCoordX, lCoordY))
                {
                    int lArrayIndex = -1;
                    if (mWafermapReticle->indexFromCoord(lArrayIndex, lCoordX, lCoordY))
                    {
                        // Reticle pos start to 1 not 0
                        mOrigin.SetX(lCoordX - mWafermapReticle->getWafMapDie(lArrayIndex).GetReticleDieX() + 1);
                        mOrigin.SetY(lCoordY - mWafermapReticle->getWafMapDie(lArrayIndex).GetReticleDieY() + 1);

                        mReticlePosX = mWafermapReticle->getWafMapDie(lArrayIndex).GetReticlePosX();
                        mReticlePosY = mWafermapReticle->getWafMapDie(lArrayIndex).GetReticlePosY();

                        lFindOrigin = true;
                    }
                }
            }
        }

        if (lFindOrigin == false)
            return false;

        mReticleWidth   = mWafermapReticle->GetReticleWidth();
        mReticleHeight  = mWafermapReticle->GetReticleHeight();

        while (mOrigin.GetX() > mWafermapData->GetLowDieX())
            mOrigin.Translate(-mReticleWidth, 0);

        while (mOrigin.GetY() > mWafermapData->GetLowDieY())
            mOrigin.Translate(0, -mReticleHeight);

        mUpperLeftDie   = mOrigin;
        mLowerRightDie  = mOrigin.Translated(mReticleWidth-1, mReticleHeight-1);

        return true;
    }

    GSLOG(SYSLOG_SEV_ERROR, "Unable to retrieve Reticle Information from specified external file");

    return false;
}

bool PatReticleMapExtern::Next()
{
    if (mWafermapData)
    {
        if ((mUpperLeftDie.GetX() + (int) mReticleWidth) <= mWafermapData->GetHighDieX())
        {
            mUpperLeftDie.Translate(mReticleWidth, 0);
            mLowerRightDie.Translate(mReticleWidth, 0);
        }
        else if (mUpperLeftDie.GetY() <= mWafermapData->GetHighDieY())
        {
            mUpperLeftDie.SetX(mOrigin.GetX());
            mUpperLeftDie.Translate(0, mReticleHeight);
            mLowerRightDie.SetX(mOrigin.GetX() + (int) mReticleWidth - 1);
            mLowerRightDie.Translate(0, mReticleHeight);
        }

        // Update reticle pos
        bool    lReticlePosFound    = false;
        int     lArrayIdx           = -1;

        for (int lX = mUpperLeftDie.GetX(); lX < mLowerRightDie.GetX() && lReticlePosFound == false; ++lX)
        {
            for (int lY = mUpperLeftDie.GetY(); lY < mLowerRightDie.GetY() && lReticlePosFound == false; ++lY)
            {
                if (mWafermapReticle->indexFromCoord(lArrayIdx, lX, lY) &&
                    mWafermapReticle->getWafMapDie(lArrayIdx).getBin() != GEX_WAFMAP_EMPTY_CELL)
                {
                    mReticlePosX = mWafermapReticle->getWafMapDie(lArrayIdx).GetReticlePosX();
                    mReticlePosY = mWafermapReticle->getWafMapDie(lArrayIdx).GetReticlePosY();

                    lReticlePosFound = true;
                }
            }
        }

        // We reach the end of the reticle map
        if (mUpperLeftDie.GetY() > mWafermapData->GetHighDieY())
            return false;

        return true;
    }

    return false;
}

int PatReticleMapExtern::GetBin(const WaferCoordinate &waferCoord) const
{
    if (waferCoord.GetX() >= mUpperLeftDie.GetX() && waferCoord.GetX() <= mLowerRightDie.GetX() &&
        waferCoord.GetY() >= mUpperLeftDie.GetY() && waferCoord.GetY() <= mLowerRightDie.GetY() &&
        mWafermapData &&
        mWafermapData->isDieInsideRing(mMaskType, mMaskWidth, waferCoord.GetX(), waferCoord.GetY()))
    {
        int lArrayIndex = -1;
        if (mWafermapData->indexFromCoord(lArrayIndex, waferCoord.GetX(), waferCoord.GetY()))
        {
            return mWafermapData->getWafMapDie(lArrayIndex).getBin();
        }
    }

    return GEX_WAFMAP_EMPTY_CELL;
}

WaferCoordinate PatReticleMapExtern::GetReticleCoordinate(const WaferCoordinate &waferCoord) const
{
    return waferCoord.Translated(-mUpperLeftDie.GetX(), -mUpperLeftDie.GetY());
}

QList<WaferCoordinate> PatReticleMapExtern::GetWaferCoordinates(const WaferCoordinate &reticleCoord) const
{
    int                     lX;
    int                     lY;
    int                     lArrayIndex = -1;
    WaferCoordinate         lRefCoord = mOrigin.Translated(reticleCoord.GetX(), reticleCoord.GetY());
    QList<WaferCoordinate>  lCoordinates;

    if (mWafermapData)
    {
        for (int lIdx = 0; lIdx < mWafermapData->GetSizeX() * mWafermapData->GetSizeY(); ++lIdx)
        {
            if (mWafermapData->getWafMapDie(lIdx).getBin() != GEX_WAFMAP_EMPTY_CELL &&
                mWafermapData->coordFromIndex(lIdx, lX, lY))
            {
                if (mWafermapReticle->indexFromCoord(lArrayIndex, lX, lY))
                {
                    if (((lX - lRefCoord.GetX()) % (int) mReticleWidth == 0) &&
                        ((lY - lRefCoord.GetY()) % (int) mReticleHeight == 0))
                    {
                        lCoordinates.append(WaferCoordinate(lX, lY));
                    }
                }
            }
        }
    }

    return lCoordinates;
}

QList<WaferCoordinate> PatReticleMapExtern::GetSurroundingCoordinates(const WaferCoordinate &waferCoord,
                                                                      bool ignoreDiagonal) const
{
    int lArrayIndex = -1;
    QList<WaferCoordinate> lSurroundingDie;

    if (mWafermapData && mWafermapData->indexFromCoord(lArrayIndex, waferCoord.GetX(), waferCoord.GetY()))
    {
        std::vector<int> lIndexDie = mWafermapData->aroundDieCoordinate(lArrayIndex,
                                                                        (ignoreDiagonal) ? CWaferMap::AdjacentDie : CWaferMap::BothDie);
        std::vector<int>::iterator	itBegin = lIndexDie.begin();
        std::vector<int>::iterator	itEnd   = lIndexDie.end();
        int lCoordX, lCoordY;

        while (itBegin != itEnd)
        {
            if (mWafermapData->coordFromIndex(*itBegin, lCoordX, lCoordY))
            {
                if (IsInReticleField(WaferCoordinate(lCoordX, lCoordY)))
                    lSurroundingDie.append(WaferCoordinate(lCoordX, lCoordY));
            }

            itBegin++;
        }
    }

    return lSurroundingDie;
}

bool PatReticleMapExtern::IsValidWaferCoord(const WaferCoordinate &waferCoord) const
{
    return mWafermapData->isValidCoord(waferCoord.GetX(), waferCoord.GetY());
}

bool PatReticleMapExtern::IsEdgeReticleField() const
{
    // Check edge die of the reticle, if at least one die is outside the wafer, the reticle field is considered
    // as an Edge reticle field.
    int lArrayIdx = -1;

    // Check top and bottom die of the reticle field
    for (int lX = mUpperLeftDie.GetX(); lX < mLowerRightDie.GetX(); ++lX)
    {
        if (mWafermapData->indexFromCoord(lArrayIdx, lX, mUpperLeftDie.GetY()))
        {
            if (mWafermapData->isDieOutOfWafer(lArrayIdx))
                return true;
        }

        if (mWafermapData->indexFromCoord(lArrayIdx, lX, mLowerRightDie.GetY()))
        {
            if (mWafermapData->isDieOutOfWafer(lArrayIdx))
                return true;
        }
    }

    // Check left and right edge die of the reticle field
    for (int lY = mUpperLeftDie.GetY(); lY < mLowerRightDie.GetY(); ++lY)
    {
        if (mWafermapData->indexFromCoord(lArrayIdx, mUpperLeftDie.GetX(), lY))
        {
            if (mWafermapData->isDieOutOfWafer(lArrayIdx))
                return true;
        }

        if (mWafermapData->indexFromCoord(lArrayIdx, mLowerRightDie.GetX(), lY))
        {
            if (mWafermapData->isDieOutOfWafer(lArrayIdx))
                return true;
        }
    }

    return false;
}

}
}
