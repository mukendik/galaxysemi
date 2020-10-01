#ifndef PAT_GDBN_ENGINE_H
#define PAT_GDBN_ENGINE_H

#include <QHash>

class PatGdbnAbstractAlgorithm;
class CPatDieCoordinates;
class CGDBN_Rule;
class CWaferMap;

namespace GS
{
namespace QtLib
{
class Range;
}
}

class PatGdbnEngine
{
public:

    PatGdbnEngine();
    ~PatGdbnEngine();

    void setAlgorithm(const CGDBN_Rule& gdbnRule);

    void setAlgorithm(PatGdbnAbstractAlgorithm * pAlgorithm);

    bool processWafer(const CWaferMap * pWafermap, const GS::QtLib::Range * pGoodBinList,
                      QHash<QString, CPatDieCoordinates> &lGDBNOutliers);

protected:

    bool init(const CWaferMap * pWafermap,
              const GS::QtLib::Range * pGoodBinList);
    bool excludeDie(int index) const;

    PatGdbnAbstractAlgorithm *          m_pGdbnAlgorithm;
    const CWaferMap *                   m_pWafermap;
    const GS::QtLib::Range *            m_pGoodBinList;
    int                                 m_nWaferSize;						// wafermap size
};

#endif // PAT_GDBN_ENGINE_H
