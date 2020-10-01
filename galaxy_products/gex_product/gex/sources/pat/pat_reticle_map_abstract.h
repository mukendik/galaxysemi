#ifndef PAT_RETICLE_MAP_ABSTRACT
#define PAT_RETICLE_MAP_ABSTRACT

#include "wafermap.h"

namespace GS
{
namespace Gex
{

class PatReticleMapAbstract
{
public:

    PatReticleMapAbstract();
    PatReticleMapAbstract(unsigned int reticleWidth, unsigned int reticleHeight);
    virtual ~PatReticleMapAbstract();

    virtual bool                    Init() = 0;
    virtual bool                    Next() = 0;

    unsigned int                    GetReticleWidth() const;
    unsigned int                    GetReticleHeight() const;
    int                             GetReticlePosX() const;
    int                             GetReticlePosY() const;

    const WaferCoordinate&          GetTopLeftDie() const;
    WaferCoordinate                 GetTopRightDie() const;
    WaferCoordinate                 GetBottomLeftDie() const;
    const WaferCoordinate&          GetBottomRightDie() const;
    int                             GetBin(int x, int y) const;
    virtual int                     GetBin(const WaferCoordinate& waferCoord) const = 0;
    virtual WaferCoordinate         GetReticleCoordinate(const WaferCoordinate& waferCoord) const = 0;
    virtual QList<WaferCoordinate>  GetWaferCoordinates(const WaferCoordinate& reticleCoord) const = 0;
    virtual QList<WaferCoordinate>  GetSurroundingCoordinates(const WaferCoordinate& waferCoord,
                                                              bool ignoreDiagonal) const = 0;
    virtual bool                    IsValidWaferCoord(const WaferCoordinate& waferCoord) const = 0;
    virtual bool                    IsEdgeReticleField() const = 0;
    bool                            IsInReticleField(const WaferCoordinate& waferCoord) const;
    double                          GetReticleFieldYield(const GS::QtLib::Range& badBins) const;

    void                            SetMaskType(CWaferMap::Ring maskType);
    void                            SetMaskWidth(int width);

protected:

    unsigned int            mReticleWidth;
    unsigned int            mReticleHeight;
    int                     mReticlePosX;
    int                     mReticlePosY;

    WaferCoordinate         mUpperLeftDie;
    WaferCoordinate         mLowerRightDie;
    CWaferMap::Ring         mMaskType;
    int                     mMaskWidth;
};

}
}
#endif // PAT_RETICLE_MAP_ABSTRACT

