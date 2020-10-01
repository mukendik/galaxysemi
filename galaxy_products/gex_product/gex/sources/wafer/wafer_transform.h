#ifndef WAFER_TRANSFORM_H
#define WAFER_TRANSFORM_H

#include "wafer_coordinate.h"

namespace GS
{
namespace Gex
{

class WaferTransform
{
public:

    WaferTransform();
    WaferTransform(const WaferTransform& other);

    WaferTransform& operator=(const WaferTransform& other);

    void                SetOrigin(const WaferCoordinate& origin);

    int                 GetRotation() const;
    int                 GetXTranslation() const;
    int                 GetYTranslation() const;
    bool                GetXReflection() const;
    bool                GetYReflection() const;

    void                Reflect(bool lXAxis, bool lYAxis);
    void                Rotate(int quarters);
    void                Translate(int dX, int dY);

    WaferCoordinate     Map(const WaferCoordinate& die) const;
    void                Map(int x, int y, int &tX, int &tY) const;

    WaferTransform      Inverted() const;

private:

    WaferCoordinate     mOrigin;
    int                 mTX;
    int                 mTY;
    bool                mRX;
    bool                mRY;
    int                 mQuarters;
};

} // end namespace Gex
} // end namespace GS
#endif // WAFER_TRANSFORM_H
