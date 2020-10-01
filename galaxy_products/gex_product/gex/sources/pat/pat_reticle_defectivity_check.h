#ifndef PAT_RETICLE_DEFECTIVITY_CHECK
#define PAT_RETICLE_DEFECTIVITY_CHECK

#include "pat_reticle_abstract_algorithm.h"

#include <QMap>
#include <QSet>
#include <QJsonArray>

//class CWaferMap;

namespace GS
{
namespace Gex
{

class PatReticleDefectivityCheck : public PatReticleAbstractAlgorithm
{
public:

    PatReticleDefectivityCheck(const PATOptionReticle &reticleSettings);
    virtual ~PatReticleDefectivityCheck();

    bool                    ProcessReticleMap(PatReticleMapAbstract * reticleMap);

private:

    bool                    Init(PatReticleMapAbstract * reticleMap);
    bool                    IsReticleFieldFiltered();
    bool                    ProcessReticle();
    void                    FindOutliers();

    QJsonArray              mReticleFieldsResults;

};

}
}


#endif // PAT_RETICLE_DEFECTIVITY_CHECK

