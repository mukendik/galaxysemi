#include "pat_reticle_map_recipe.h"
#include "gqtl_log.h"

namespace GS
{
namespace Gex
{

PatReticleMapRecipe::PatReticleMapRecipe(unsigned int reticleWidth, unsigned int reticleHeight, int defaultBin)
    : PatReticleMapAbstract(reticleWidth, reticleHeight), mDefaultBin(defaultBin)
{

}

PatReticleMapRecipe::~PatReticleMapRecipe()
{

}

bool PatReticleMapRecipe::Init()
{
    if (mReticleWidth > 0 && mReticleHeight > 0)
    {
        mUpperLeftDie.SetX(0);
        mUpperLeftDie.SetY(0);

        mLowerRightDie.SetX(mReticleWidth - 1);
        mLowerRightDie.SetY(mReticleHeight - 1);

        mReticlePosX = 1;
        mReticlePosY = 1;

        return true;
    }

    return false;
}

bool PatReticleMapRecipe::Next()
{
    return false;
}

int PatReticleMapRecipe::GetBin(const WaferCoordinate &waferCoord) const
{
    if (waferCoord.GetX() >= mUpperLeftDie.GetX() && waferCoord.GetX() <= mLowerRightDie.GetX() &&
        waferCoord.GetY() >= mUpperLeftDie.GetY() && waferCoord.GetY() <= mLowerRightDie.GetY())
    {
        if (mReticle.contains(waferCoord))
            return mReticle.value(waferCoord);

        return mDefaultBin;
    }

    return -1;
}

WaferCoordinate PatReticleMapRecipe::GetReticleCoordinate(const WaferCoordinate &waferCoord) const
{
    return waferCoord;
}

QList<WaferCoordinate> PatReticleMapRecipe::GetWaferCoordinates(const WaferCoordinate &reticleCoord) const
{
    QList<WaferCoordinate> lCoordinates;

    lCoordinates.append(reticleCoord);

    return lCoordinates;
}

void PatReticleMapRecipe::SetBin(const WaferCoordinate &waferCoord, int lBin)
{
    mReticle.insert(waferCoord, lBin);
}

QList<WaferCoordinate> PatReticleMapRecipe::GetSurroundingCoordinates(const WaferCoordinate &waferCoord,
                                                                      bool ignoreDiagonal) const
{
    QList<WaferCoordinate> lSurroundingDie;

    if (waferCoord.GetX() > mUpperLeftDie.GetX())
        lSurroundingDie.append(waferCoord.Translated(-1, 0));

    if (waferCoord.GetX() < mLowerRightDie.GetX())
        lSurroundingDie.append(waferCoord.Translated(1, 0));

    if (waferCoord.GetY() > mUpperLeftDie.GetY())
        lSurroundingDie.append(waferCoord.Translated(0, -1));

    if (waferCoord.GetY() < mLowerRightDie.GetY())
        lSurroundingDie.append(waferCoord.Translated(0, 1));

    if (ignoreDiagonal == false)
    {
        // Upper Left diag
        WaferCoordinate lDiag = waferCoord.Translated(-1, -1);

        if (lDiag.GetX() >= mUpperLeftDie.GetX() && lDiag.GetY() >= mUpperLeftDie.GetY())
            lSurroundingDie.append(lDiag);

        // Upper Right diag
        lDiag.Translate(2, 0);

        if (lDiag.GetX() <= mLowerRightDie.GetX() && lDiag.GetY() >= mUpperLeftDie.GetY())
            lSurroundingDie.append(lDiag);

        // Lower Right diag
        lDiag.Translate(0, 2);

        if (lDiag.GetX() <= mLowerRightDie.GetX() && lDiag.GetY() <= mLowerRightDie.GetY())
            lSurroundingDie.append(lDiag);

        // Lower Left diag
        lDiag.Translate(-2, 0);

        if (lDiag.GetX() >= mUpperLeftDie.GetX() && lDiag.GetY() <= mLowerRightDie.GetY())
            lSurroundingDie.append(lDiag);
    }

    return lSurroundingDie;
}

bool PatReticleMapRecipe::IsValidWaferCoord(const WaferCoordinate &waferCoord) const
{
    if (waferCoord.GetX() >= mUpperLeftDie.GetX() && waferCoord.GetX() <= mLowerRightDie.GetX() &&
        waferCoord.GetY() >= mUpperLeftDie.GetY() && waferCoord.GetY() <= mLowerRightDie.GetY())
        return true;

    return false;
}

bool PatReticleMapRecipe::IsEdgeReticleField() const
{
    return false;
}

}
}
