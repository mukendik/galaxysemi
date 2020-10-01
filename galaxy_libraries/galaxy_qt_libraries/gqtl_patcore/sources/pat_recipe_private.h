#ifndef PAT_RECIPE_PRIVATE_H
#define PAT_RECIPE_PRIVATE_H

#include "pat_options.h"
#include "pat_mv_rule.h"
#include "pat_definition.h"

#include <QHash>

namespace GS
{
namespace Gex
{

class PATRecipePrivate
{
public:

    PATRecipePrivate();
    ~PATRecipePrivate();

    COptionsPat                     mOptions;
    QList<PATMultiVariateRule>      mMVRules;           // Holds the list of Multi-variate rules
    QHash<QString, CPatDefinition*> mUniVariateRules;   // Holds the list of Uni-variate rules
};
}   // namespace Gex
}   // namespace GS

#endif // PAT_RECIPE_PRIVATE_H
