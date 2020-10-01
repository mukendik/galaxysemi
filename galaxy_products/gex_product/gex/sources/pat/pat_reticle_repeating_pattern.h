#ifndef PAT_RETICLE_REPEATING_PATTERN
#define PAT_RETICLE_REPEATING_PATTERN

#include "pat_reticle_abstract_algorithm.h"

#include <QMap>
#include <QHash>
#include <QJsonObject>

class CWaferMap;

namespace GS
{
namespace Gex
{

class PatReticleRepeatingPattern : public PatReticleAbstractAlgorithm
{
public:

    PatReticleRepeatingPattern(const PATOptionReticle &reticleSettings);
    virtual ~PatReticleRepeatingPattern();

    bool                        ProcessReticleMap(PatReticleMapAbstract * reticleMap);

private:

    bool                        Init(PatReticleMapAbstract * reticleMap);

    void                        FindOutliers();
    bool                        ProcessReticle();

    QMap<int,int>               mGoodDieCount;      // Good die count per reticle location
    QMap<int,int>               mTotalDieCount;     // Total die count per reticle location

};

}
}
#endif // PAT_RETICLE_REPEATING_PATTERN

