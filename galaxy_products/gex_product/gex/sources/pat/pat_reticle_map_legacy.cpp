#include "pat_reticle_map_legacy.h"
#include "wafermap.h"
#include "gqtl_log.h"

namespace GS
{
namespace Gex
{

PatReticleMapLegacy::PatReticleMapLegacy(const CWaferMap *waferMap, int reticleWidth, int reticleHeight)
    : PatReticleMapAbstract(reticleWidth, reticleHeight), mWafermap(waferMap)
{

}

PatReticleMapLegacy::~PatReticleMapLegacy()
{

}

bool PatReticleMapLegacy::Init()
{
    if (mWafermap)
    {
        const CWafMapArray * lWafMapArray = mWafermap->getWafMap();

        if (lWafMapArray == NULL)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to get wafermap array");
            return false;
        }

        int     lCoordX;
        int     lCoordY;
        int     lBin            = GEX_WAFMAP_EMPTY_CELL;
        bool    lFindOrigin     = false;
        int     lWafermapSize   = mWafermap->GetSizeX() * mWafermap->GetSizeY();

        for(int lIdx = 0; lIdx < lWafermapSize && lFindOrigin == false; ++lIdx)
        {
            // Get die value SoftBin#
            lBin = lWafMapArray[lIdx].getBin();

            // Keep track of valid dies at given reticle location
            if(lBin != GEX_WAFMAP_EMPTY_CELL)
            {
                // If first die in wafer, this is where the reticle origin is!
                // Save reticle origine in array
                if (mWafermap->coordFromIndex(lIdx, lCoordX, lCoordY))
                {
                    mOrigin.SetX(lCoordX);
                    mOrigin.SetY(lCoordY);

                    lFindOrigin = true;
                }
            }
        }

        if (lFindOrigin == false)
            return false;

        while (mOrigin.GetX() > mWafermap->GetLowDieX())
            mOrigin.Translate(-mReticleWidth, 0);

        while (mOrigin.GetY() > mWafermap->GetLowDieY())
            mOrigin.Translate(0, -mReticleHeight);

        mUpperLeftDie   = mOrigin;
        mLowerRightDie  = mOrigin.Translated(mReticleWidth-1, mReticleHeight-1);
        mReticlePosX    = 1;
        mReticlePosY    = 1;
    }

    return true;
}

bool PatReticleMapLegacy::Next()
{
    if (mWafermap)
    {
        if ((mUpperLeftDie.GetX() + (int) mReticleWidth) <= mWafermap->GetHighDieX())
        {
            mUpperLeftDie.Translate(mReticleWidth, 0);
            mLowerRightDie.Translate(mReticleWidth, 0);
            mReticlePosX += 1;
        }
        else if (mUpperLeftDie.GetY() <= mWafermap->GetHighDieY())
        {
            mUpperLeftDie.SetX(mOrigin.GetX());
            mUpperLeftDie.Translate(0, mReticleHeight);
            mLowerRightDie.SetX(mOrigin.GetX() + (int) mReticleWidth - 1);
            mLowerRightDie.Translate(0, mReticleHeight);
            mReticlePosX = 1;
            mReticlePosY += 1;
        }

        // We reach the end of the reticle map
        if (mUpperLeftDie.GetY() > mWafermap->GetHighDieY())
            return false;

        return true;
    }

    return false;
}

int PatReticleMapLegacy::GetBin(const WaferCoordinate &waferCoord) const
{
    if (waferCoord.GetX() >= mUpperLeftDie.GetX() && waferCoord.GetX() <= mLowerRightDie.GetX() &&
        waferCoord.GetY() >= mUpperLeftDie.GetY() && waferCoord.GetY() <= mLowerRightDie.GetY() &&
        mWafermap && mWafermap->isDieInsideRing(mMaskType, mMaskWidth, waferCoord.GetX(), waferCoord.GetY()))
    {
        int lArrayIndex = -1;
        if (mWafermap->indexFromCoord(lArrayIndex, waferCoord.GetX(), waferCoord.GetY()))
        {
            return mWafermap->getWafMapDie(lArrayIndex).getBin();
        }
    }

    return GEX_WAFMAP_EMPTY_CELL;
}

WaferCoordinate PatReticleMapLegacy::GetReticleCoordinate(const WaferCoordinate &waferCoord) const
{
    return waferCoord.Translated(-mUpperLeftDie.GetX(), -mUpperLeftDie.GetY());
}

QList<WaferCoordinate> PatReticleMapLegacy::GetWaferCoordinates(const WaferCoordinate &reticleCoord) const
{
    int                     lX;
    int                     lY;
    WaferCoordinate         lRefCoord = mOrigin.Translated(reticleCoord.GetX(), reticleCoord.GetY());
    QList<WaferCoordinate>  lCoordinates;

    if (mWafermap)
    {
        for (int lIdx = 0; lIdx < mWafermap->GetSizeX() * mWafermap->GetSizeY(); ++lIdx)
        {
            if (mWafermap->getWafMapDie(lIdx).getBin() != GEX_WAFMAP_EMPTY_CELL &&
                mWafermap->coordFromIndex(lIdx, lX, lY))
            {
                if (((lX - lRefCoord.GetX()) % (int) mReticleWidth == 0) &&
                    ((lY - lRefCoord.GetY()) % (int) mReticleHeight == 0))
                {
                    lCoordinates.append(WaferCoordinate(lX, lY));
                }
            }
        }
    }

    return lCoordinates;
}

QList<WaferCoordinate> PatReticleMapLegacy::GetSurroundingCoordinates(const WaferCoordinate &waferCoord,
                                                                      bool ignoreDiagonal) const
{
    int lArrayIndex = -1;
    QList<WaferCoordinate> lSurroundingDie;

    if (mWafermap && mWafermap->indexFromCoord(lArrayIndex, waferCoord.GetX(), waferCoord.GetY()))
    {
        std::vector<int> lIndexDie = mWafermap->aroundDieCoordinate(lArrayIndex,
                                                                    (ignoreDiagonal) ? CWaferMap::AdjacentDie : CWaferMap::BothDie);
        std::vector<int>::iterator	itBegin = lIndexDie.begin();
        std::vector<int>::iterator	itEnd   = lIndexDie.end();
        int lCoordX, lCoordY;

        while (itBegin != itEnd)
        {
            if (mWafermap->coordFromIndex(*itBegin, lCoordX, lCoordY))
            {
                if (IsInReticleField(WaferCoordinate(lCoordX, lCoordY)))
                    lSurroundingDie.append(WaferCoordinate(lCoordX, lCoordY));
            }
            itBegin++;
        }
    }

    return lSurroundingDie;
}

bool PatReticleMapLegacy::IsValidWaferCoord(const WaferCoordinate &waferCoord) const
{
    return mWafermap->isValidCoord(waferCoord.GetX(), waferCoord.GetY());
}

bool PatReticleMapLegacy::IsEdgeReticleField() const
{
    // Check edge die of the reticle, if at least one die is outside the wafer, the reticle field is considered
    // as an Edge reticle field.
    int lArrayIdx = -1;

    // Check top and bottom die of the reticle field
    for (int lX = mUpperLeftDie.GetX(); lX < mLowerRightDie.GetX(); ++lX)
    {
        if (mWafermap->indexFromCoord(lArrayIdx, lX, mUpperLeftDie.GetY()))
        {
            if (mWafermap->isDieOutOfWafer(lArrayIdx))
                return true;
        }

        if (mWafermap->indexFromCoord(lArrayIdx, lX, mLowerRightDie.GetY()))
        {
            if (mWafermap->isDieOutOfWafer(lArrayIdx))
                return true;
        }
    }

    // Check left and right edge die of the reticle field
    for (int lY = mUpperLeftDie.GetY(); lY < mLowerRightDie.GetY(); ++lY)
    {
        if (mWafermap->indexFromCoord(lArrayIdx, mUpperLeftDie.GetX(), lY))
        {
            if (mWafermap->isDieOutOfWafer(lArrayIdx))
                return true;
        }

        if (mWafermap->indexFromCoord(lArrayIdx, mLowerRightDie.GetX(), lY))
        {
            if (mWafermap->isDieOutOfWafer(lArrayIdx))
                return true;
        }
    }

    return false;
}

}
}
