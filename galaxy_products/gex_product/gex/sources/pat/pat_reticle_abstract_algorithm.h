#ifndef PAT_RETICLE_ABSTRACT_ALGORITHM
#define PAT_RETICLE_ABSTRACT_ALGORITHM

#include "pat_option_reticle.h"
#include "wafer_coordinate.h"
#include <QJsonObject>

namespace GS
{
namespace Gex
{

class PatReticleMapAbstract;

class PatReticleAbstractAlgorithm
{
public:

    PatReticleAbstractAlgorithm(const PATOptionReticle &reticleSettings);
    virtual ~PatReticleAbstractAlgorithm();

    static PatReticleAbstractAlgorithm *Create(const PATOptionReticle& reticleSettings);

    const QList<WaferCoordinate>&   GetOutliers() const;
    const QJsonObject&              GetReticleResults() const;
    virtual bool                    ProcessReticleMap(PatReticleMapAbstract *  reticle) = 0;

protected:

    virtual bool                    Init(PatReticleMapAbstract * reticleMap) = 0;

    PATOptionReticle        mReticleSettings;
    PatReticleMapAbstract * mReticleMap;
    QList<WaferCoordinate>  mOutliers;
    QJsonObject             mReticleResults;
};

}
}
#endif // PAT_RETICLE_ABSTRACT_ALGORITHM

