#include "pat_recipe_private.h"

namespace GS
{
namespace Gex
{

PATRecipePrivate::PATRecipePrivate() : mOptions(NULL)
{
}

PATRecipePrivate::~PATRecipePrivate()
{
    qDeleteAll(mUniVariateRules);
    mUniVariateRules.clear();

    mMVRules.clear();
}



}   // namespace Gex
}   // namespace GS
