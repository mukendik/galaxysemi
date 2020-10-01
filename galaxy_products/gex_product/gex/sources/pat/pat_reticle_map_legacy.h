#ifndef PAT_RETICLE_LEGACY
#define PAT_RETICLE_LEGACY

#include "pat_reticle_map_abstract.h"

class CWaferMap;

namespace GS
{
namespace Gex
{

class PatReticleMapLegacy : public PatReticleMapAbstract
{
public:

    PatReticleMapLegacy(const CWaferMap * waferMap, int reticleWidth, int reticleHeight);
    virtual ~PatReticleMapLegacy();

    bool                    Init();
    bool                    Next();

    int                     GetBin(const WaferCoordinate& waferCoord) const;
    WaferCoordinate         GetReticleCoordinate(const WaferCoordinate& waferCoord) const;
    QList<WaferCoordinate>  GetWaferCoordinates(const WaferCoordinate& reticleCoord ) const;
    QList<WaferCoordinate>  GetSurroundingCoordinates(const WaferCoordinate& waferCoord, bool ignoreDiagonal) const;
    bool                    IsValidWaferCoord(const WaferCoordinate& waferCoord) const;
    bool                    IsEdgeReticleField() const;

private:

    const CWaferMap *       mWafermap;
    WaferCoordinate         mOrigin;
};

}
}

#endif // PAT_RETICLE_LEGACY

