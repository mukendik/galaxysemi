#include "wafer_coordinate.h"
//#include "wafer_global.h"

///////////////////////////////////////////////////////////////////////////////////
// STD Includes
///////////////////////////////////////////////////////////////////////////////////
#include <limits>

#define WAFER_MIN_COORDINATE		-32767
#define WAFER_MAX_COORDINATE		32767

namespace GS
{
namespace Gex
{

WaferCoordinate::WaferCoordinate()
    : mX(std::numeric_limits<int>::max()),
      mY(std::numeric_limits<int>::max())
{
}

WaferCoordinate::WaferCoordinate(int x, int y)
    : mX(x), mY(y)
{
}

WaferCoordinate::WaferCoordinate(const WaferCoordinate &other)
{
    *this = other;
}

WaferCoordinate::~WaferCoordinate()
{

}

WaferCoordinate& WaferCoordinate::operator =(const WaferCoordinate& other)
{
    if (this != &other)
    {
        mX = other.mX;
        mY = other.mY;
    }

    return *this;
}

bool WaferCoordinate::operator ==(const WaferCoordinate& other) const
{
    return (mX == other.mX && mY == other.mY);
}

bool WaferCoordinate::operator !=(const WaferCoordinate &other) const
{
    return (mX != other.mX || mY != other.mY);
}

WaferCoordinate &WaferCoordinate::operator -=(const WaferCoordinate &other)
{
    mX -= other.mX;
    mY -= other.mY;

    return *this;
}

WaferCoordinate &WaferCoordinate::operator +=(const WaferCoordinate &other)
{
    mX += other.mX;
    mY += other.mY;

    return *this;
}

bool WaferCoordinate::operator<(const WaferCoordinate &other) const
{
    if (mX < other.mX)
        return true;

    if (mX == other.mX && mY < other.mY)
        return true;

    return false;
}

WaferCoordinate WaferCoordinate::operator-(const WaferCoordinate &other)
{
    WaferCoordinate coord;

    coord.SetX(mX - other.mX);
    coord.SetY(mY - other.mY);

    return coord;
}

WaferCoordinate WaferCoordinate::operator+(const WaferCoordinate &other)
{
    WaferCoordinate coord;

    coord.SetX(mX + other.mY);
    coord.SetY(mX + other.mY);

    return coord;
}

bool WaferCoordinate::IsValid() const
{
    return (IsValidX() && IsValidY());
}

bool WaferCoordinate::IsValidX() const
{
    return (mX >= WAFER_MIN_COORDINATE && mX <= WAFER_MAX_COORDINATE);
}

bool WaferCoordinate::IsValidY() const
{
    return (mY >= WAFER_MIN_COORDINATE && mY <= WAFER_MAX_COORDINATE);
}

int WaferCoordinate::GetX() const
{
    return mX;
}

int WaferCoordinate::GetY() const
{
    return mY;
}

void WaferCoordinate::SetX(int x)
{
    mX = x;
}

void WaferCoordinate::SetY(int y)
{
    mY = y;
}

void WaferCoordinate::Translate(int x, int y)
{
    mX += x;
    mY += y;
}

WaferCoordinate WaferCoordinate::Translated(int x, int y) const
{
    WaferCoordinate wCoordinate(*this);

    wCoordinate.Translate(x, y);

    return wCoordinate;
}

} // end namespace Gex
} // end namespace GS

