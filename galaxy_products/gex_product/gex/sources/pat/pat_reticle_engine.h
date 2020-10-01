#ifndef PAT_RETICLE_ENGINE
#define PAT_RETICLE_ENGINE

#include <QHash>
#include <QScopedPointer>

#include "pat_option_reticle.h"

class CPatDieCoordinates;
class CWaferMap;
class CPatInfo;

namespace GS
{
namespace QtLib
{
    class Range;
}

namespace Gex
{

class PatReticleMapAbstract;
class WaferCoordinate;

class PatReticleEngine
{
public:

    PatReticleEngine();
    ~PatReticleEngine();

    bool processWafer(const CWaferMap * wafermap, const GS::QtLib::Range * goodBinList,
                      const PATOptionReticle& reticleSettings, QHash<QString, CPatDieCoordinates> &patOutliers);

protected:

    PatReticleMapAbstract * createReticleMap();
    bool init(const CWaferMap * wafermap, const GS::QtLib::Range * goodBinList, const PATOptionReticle&reticleSettings);
    bool inkOut(const QList<WaferCoordinate> &outlierDie,  QHash<QString, CPatDieCoordinates> &patOutliers);

    PATOptionReticle                            mReticleSettings;
    const CWaferMap *                           mWafermap;
    const GS::QtLib::Range *                    mGoodBinList;
    CPatInfo *                                  mContext;
    int                                         mWaferSize;						// wafermap size
};

}
}
#endif // PAT_RETICLE_ENGINE

