#include "pat_reticle_map_abstract.h"
#include "gqtl_utils.h"

namespace GS
{
namespace Gex
{

PatReticleMapAbstract::PatReticleMapAbstract()
    : mReticleWidth(0), mReticleHeight(0), mReticlePosX(0), mReticlePosY(0), mMaskType(CWaferMap::NoRing), mMaskWidth(-1)
{

}

PatReticleMapAbstract::PatReticleMapAbstract(unsigned int reticleWidth, unsigned int reticleHeight)
    : mReticleWidth(reticleWidth), mReticleHeight(reticleHeight), mReticlePosX(0), mReticlePosY(0),
      mMaskType(CWaferMap::NoRing), mMaskWidth(-1)
{

}

PatReticleMapAbstract::~PatReticleMapAbstract()
{

}

unsigned int PatReticleMapAbstract::GetReticleWidth() const
{
    return mReticleWidth;
}

unsigned int PatReticleMapAbstract::GetReticleHeight() const
{
    return mReticleHeight;
}

int PatReticleMapAbstract::GetReticlePosX() const
{
    return mReticlePosX;
}

int PatReticleMapAbstract::GetReticlePosY() const
{
    return mReticlePosY;
}

const WaferCoordinate &PatReticleMapAbstract::GetTopLeftDie() const
{
    return mUpperLeftDie;
}

WaferCoordinate PatReticleMapAbstract::GetTopRightDie() const
{
    return WaferCoordinate(mLowerRightDie.GetX(), mUpperLeftDie.GetY());
}

WaferCoordinate PatReticleMapAbstract::GetBottomLeftDie() const
{
    return WaferCoordinate(mUpperLeftDie.GetX(), mLowerRightDie.GetY());
}

const WaferCoordinate &PatReticleMapAbstract::GetBottomRightDie() const
{
    return mLowerRightDie;
}

int PatReticleMapAbstract::GetBin(int x, int y) const
{
    return GetBin(WaferCoordinate(x, y));
}

bool PatReticleMapAbstract::IsInReticleField(const WaferCoordinate &waferCoord) const
{
    if (waferCoord.GetX() >= GetTopLeftDie().GetX() && waferCoord.GetX() <= GetBottomRightDie().GetX() &&
        waferCoord.GetY() >= GetTopLeftDie().GetY() && waferCoord.GetY() <= GetBottomRightDie().GetY())
        return true;

    return false;
}

double PatReticleMapAbstract::GetReticleFieldYield(const QtLib::Range &badBins) const
{
    double  lYield      = -1.0;
    int     lTestedDie  = 0;
    int     lGoodDie    = 0;
    int     lBin        = GEX_WAFMAP_EMPTY_CELL;

    for (int lX = mUpperLeftDie.GetX(); lX < mLowerRightDie.GetX(); ++lX)
    {
        for (int lY = mUpperLeftDie.GetY(); lY < mLowerRightDie.GetY(); ++lY)
        {
            lBin = GetBin(lX, lY);

            if (lBin != GEX_WAFMAP_EMPTY_CELL)
            {
                ++lTestedDie;

                if (badBins.Contains(lBin) == false)
                    ++lGoodDie;
            }
        }
    }

    if (lTestedDie > 0)
        lYield = (double) lGoodDie / (double) lTestedDie;

    return lYield;
}

void PatReticleMapAbstract::SetMaskType(CWaferMap::Ring maskType)
{
    mMaskType = maskType;
}

void PatReticleMapAbstract::SetMaskWidth(int width)
{
    mMaskWidth = width;
}




}
}
