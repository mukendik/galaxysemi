#ifndef PAT_RETICLE_MAP_EXTERN
#define PAT_RETICLE_MAP_EXTERN

#include "pat_reticle_map_abstract.h"

class CWaferMap;

namespace GS
{
namespace Gex
{

class PatReticleMapExtern : public PatReticleMapAbstract
{
public:

    PatReticleMapExtern(const CWaferMap * waferMap, const CWaferMap * waferReticle);
    virtual ~PatReticleMapExtern();

    bool                    Init();
    bool                    Next();

    int                     GetBin(const WaferCoordinate& waferCoord) const;
    WaferCoordinate         GetReticleCoordinate(const WaferCoordinate& waferCoord) const;
    QList<WaferCoordinate>  GetWaferCoordinates(const WaferCoordinate& reticleCoord ) const;
    QList<WaferCoordinate>  GetSurroundingCoordinates(const WaferCoordinate &waferCoord, bool ignoreDiagonal) const;

    bool                    IsValidWaferCoord(const WaferCoordinate& waferCoord) const;
    bool                    IsEdgeReticleField() const;

private:

    const CWaferMap *       mWafermapData;
    const CWaferMap *       mWafermapReticle;
    WaferCoordinate         mOrigin;
};

}
}

#endif // PAT_RETICLE_MAP_EXTERN

