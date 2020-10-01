#ifndef PAT_RETICLE_MAP_RECIPE
#define PAT_RETICLE_MAP_RECIPE

#include "pat_reticle_map_abstract.h"

#include <QMap>

namespace GS
{
namespace Gex
{

class PatReticleMapRecipe : public PatReticleMapAbstract
{
public:

    PatReticleMapRecipe(unsigned int reticleWidth, unsigned int reticleHeight, int defaultBin);
    virtual ~PatReticleMapRecipe();

    bool                    Init();
    bool                    Next();

    int                     GetBin(const WaferCoordinate& waferCoord) const;
    WaferCoordinate         GetReticleCoordinate(const WaferCoordinate& waferCoord) const;
    QList<WaferCoordinate>  GetWaferCoordinates(const WaferCoordinate& reticleCoord ) const;
    QList<WaferCoordinate>  GetSurroundingCoordinates(const WaferCoordinate &waferCoord, bool ignoreDiagonal) const;

    bool                    IsValidWaferCoord(const WaferCoordinate& waferCoord) const;
    bool                    IsEdgeReticleField() const;

    void                    SetBin(const WaferCoordinate& waferCoord, int lBin);

private:

    int                             mDefaultBin;
    QMap<WaferCoordinate, int>     mReticle;
};

}
}

#endif // PAT_RETICLE_MAP_RECIPE

