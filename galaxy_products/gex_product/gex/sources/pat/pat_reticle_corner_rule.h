#ifndef PAT_RETICLE_CORNER_RULE
#define PAT_RETICLE_CORNER_RULE

#include "pat_reticle_abstract_algorithm.h"

#include <QMap>
#include <QSet>

class CWaferMap;

namespace GS
{
namespace Gex
{

class PatReticleCornerRule : public PatReticleAbstractAlgorithm
{
public:

    PatReticleCornerRule(const PATOptionReticle &reticleSettings);
    virtual ~PatReticleCornerRule();

    bool                    ProcessReticleMap(PatReticleMapAbstract * reticleMap);

private:

    bool                    Init(PatReticleMapAbstract * reticleMap);

    void                    AddOutliers(const WaferCoordinate& die);
    void                    ComputeResults();
    void                    FindOutliers(const WaferCoordinate& cornerDie);
    void                    InkX(const WaferCoordinate& cornerDie, int lLeft, int lRight);
    void                    InkY(const WaferCoordinate& cornerDie, int lTop, int lBottom);
    void                    InkDiag(const WaferCoordinate& cornerDie, int lDiag, int lXOff, int lYOff);
    bool                    ProcessReticle();
    bool                    ProcessDie(const WaferCoordinate& die);

    QSet<WaferCoordinate>   mReticleDieProcessed;
    QMap<int,int>           mOutlierDieCount;   // Outlier die count per reticle location
    QMap<int,int>           mTotalDieCount;     // Total die count per reticle location
};

}
}

#endif // PAT_RETICLE_CORNER_RULE

