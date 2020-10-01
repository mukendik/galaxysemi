#include "pat_reticle_abstract_algorithm.h"
#include "pat_reticle_repeating_pattern.h"
#include "pat_reticle_defectivity_check.h"
#include "pat_reticle_corner_rule.h"

namespace GS
{
namespace Gex
{

PatReticleAbstractAlgorithm::PatReticleAbstractAlgorithm(const PATOptionReticle &reticleSettings)
    : mReticleSettings(reticleSettings), mReticleMap(NULL)
{
    mReticleResults.insert("type", QJsonValue(reticleSettings.GetRuleString()));
}

PatReticleAbstractAlgorithm::~PatReticleAbstractAlgorithm()
{

}

PatReticleAbstractAlgorithm * PatReticleAbstractAlgorithm::Create(const PATOptionReticle &reticleSettings)
{
    PatReticleAbstractAlgorithm * lreticleAlgo = NULL;

    switch(reticleSettings.GetRule())
    {
        case PATOptionReticle::REPEATING_PATTERNS:
            lreticleAlgo = new PatReticleRepeatingPattern(reticleSettings);
            break;

        case PATOptionReticle::CORNER:
            lreticleAlgo = new PatReticleCornerRule(reticleSettings);
            break;

        case PATOptionReticle::STEP_DEFECTIVITY_CHECK:
            lreticleAlgo = new PatReticleDefectivityCheck(reticleSettings);;
            break;
        default:
            break;
    }

    return lreticleAlgo;
}

const QList<WaferCoordinate> &PatReticleAbstractAlgorithm::GetOutliers() const
{
    return mOutliers;
}

const QJsonObject &PatReticleAbstractAlgorithm::GetReticleResults() const
{
    return mReticleResults;
}

}
}
