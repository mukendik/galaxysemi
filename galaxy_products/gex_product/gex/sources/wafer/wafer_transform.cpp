#include "wafer_transform.h"

namespace GS
{
namespace Gex
{

WaferTransform::WaferTransform()
    : mTX(0), mTY(0), mRX(false), mRY(false), mQuarters(0)
{
}

WaferTransform::WaferTransform(const WaferTransform &other)
{
    *this = other;
}

WaferTransform &WaferTransform::operator =(const WaferTransform &other)
{
    if (this != &other)
    {
        mOrigin     = other.mOrigin;
        mTX         = other.mTX;
        mTY         = other.mTY;
        mRX         = other.mRX;
        mRY         = other.mRY;
        mQuarters   = other.mQuarters;
    }

    return *this;
}

void WaferTransform::SetOrigin(const WaferCoordinate &origin)
{
    mOrigin = origin;
}

void WaferTransform::Rotate(int quarters)
{
    mQuarters += quarters;

    mQuarters = mQuarters % 4;
}

void WaferTransform::Translate(int dX, int dY)
{
    mTX += dX;
    mTY += dY;
}

void WaferTransform::Reflect(bool lXAxis, bool lYAxis)
{
    // Reflection over Y axis, x value changes
    if (lXAxis)
        mRX = !mRX;

    // Reflection over X axis, y value changes
    if (lYAxis)
        mRY = !mRY;
}

WaferCoordinate WaferTransform::Map(const WaferCoordinate &die) const
{
    int lX;
    int lY;

    // Map the coordinate
    Map(die.GetX(), die.GetY(), lX, lY);

    return WaferCoordinate(lX, lY);
}

void WaferTransform::Map(int x, int y, int&tX, int& tY) const
{
    int sina;
    int cosa;

    // Compute rotation if any
    switch(mQuarters)
    {
        default:
        case 0:     cosa = 1;
                    sina = 0;
                        break;

        case 1:
        case -3:    cosa = 0;
                    sina = 1;
                    break;

        case 2:
        case -2:    cosa = -1;
                    sina = 0;
                    break;

        case 3:
        case -1:    cosa = 0;
                    sina = -1;
                    break;
    }

    // Execute reflection over the origin
    // Relection over Y Axis, x changes
    if (mRY && mOrigin.IsValid())
        x = mOrigin.GetX() + (mOrigin.GetX() - x);

    // Relection over X Axis, y changes
    if (mRX && mOrigin.IsValid())
        y = mOrigin.GetY() + (mOrigin.GetY() - y);

    if (mOrigin.IsValid())
    {
        x -= mOrigin.GetX();
        y -= mOrigin.GetY();
    }

    // Execute rotation over the origin
    tX = x * cosa - y * sina + mTX;
    tY = x * sina + y * cosa + mTY;

    if (mOrigin.IsValid())
    {
        tX += mOrigin.GetX();
        tY += mOrigin.GetY();
    }
}

WaferTransform WaferTransform::Inverted() const
{
    WaferTransform wTransform;

    wTransform.SetOrigin(mOrigin.Translated(mTX, mTY));
    wTransform.Reflect(mRX, mRY);
    wTransform.Rotate(-mQuarters);
    wTransform.Translate(-mTX, -mTY);

    return wTransform;
}

int WaferTransform::GetRotation() const
{
    return mQuarters;
}

int WaferTransform::GetXTranslation() const
{
    return mTX;
}

int WaferTransform::GetYTranslation() const
{
    return mTY;
}

bool WaferTransform::GetXReflection() const
{
    return mRX;
}

bool WaferTransform::GetYReflection() const
{
    return mRY;
}

} // end namespace Gex
} // end namespace GS
