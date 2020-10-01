#ifndef PAT_RECIPE_H
#define PAT_RECIPE_H

#include "pat_mv_rule.h"

#include <QHash>

class COptionsPat;
class CPatDefinition;

namespace GS
{
namespace Gex
{

class PATRecipePrivate;

class PATRecipe
{
public:

    PATRecipe();
    ~PATRecipe();

    COptionsPat &                            GetOptions();
    const COptionsPat &                      GetOptions() const;
    QList<PATMultiVariateRule>&              GetMultiVariateRules();
    const QList<PATMultiVariateRule>&        GetMultiVariateRules() const;
    QHash<QString, CPatDefinition*>&         GetUniVariateRules();
    const QHash<QString, CPatDefinition *>&  GetUniVariateRules() const;

//    void                                SetOptions(COptionsPat);
    void                                SetMultiVariateRules(QList<PATMultiVariateRule>& multivariate);
    void                                SetUniVariateRules(QHash<QString, CPatDefinition*>&);

    /*!
      @brief    Returns the PAT definition of the test using the \a lTestNumber, \a lPinIndex
                and \a lTestName.

       @param   lTestNumber     The number of the test seeked
       @param   lPinIndex       The pin index of the test seeked
       @param   lTestName       The name of the test seeked

       @return  Pointer on the CPATDefinition found, NULL if no PAT definition found
      */
    CPatDefinition *                    FindUniVariateRule(long lTestNumber, long lPinIndex,
                                                           const QString& lTestName) const;

    /*!
      @brief    Determines whether or no the recipe use parametric algorithms

      @return   true if the recipe use parametric algorithms, otherwise false
      */
    bool                                IsUsingParametric();

    /*!
      @brief    Reset the recipe data to its default values

      @param    lRecipeType     Recipe type (Wafer Sort or Final Test)
      */
    void                                Reset(GS::Gex::PAT::RecipeType lRecipeType);

private:

    PATRecipePrivate *                  mPrivate;
};

}   // namespace Gex
}   // namespace GS

#endif // PAT_RECIPE_H
