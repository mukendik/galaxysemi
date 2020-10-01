#ifndef PAT_GLOBAL_H
#define PAT_GLOBAL_H

#include <QString>
#include <map>

#define MIN_JSON_FORMAT 2.00f
#define MAX_JSON_FORMAT 2.13f
#define DEFAULT_RECIPE_VERSION 1
#define DEFAULT_RECIPE_BUILD 0

namespace GS
{
namespace Gex
{
namespace PAT
{
    enum RecipeType
    {
        RecipeUnknown = 0,
        RecipeWaferSort,
        RecipeFinalTest
    };

    enum OutlierDistance
    {
        Near,
        Medium,
        Far,
        Custom
    };

    enum MVPairs
    {
        AllPairs,
        ConsecutivePairs
    };

    const char * const  sRecipeVersion = "2.13";

    QString GetDefaultRecipeName(RecipeType lType);
//    QString GetDefaultSharedRecipeName();
    QString GetOutlierBinName(unsigned int lFailType);
    QString GetOutlierTypeName(unsigned int lFailType);

    int     GetDistributionID(const QString& lDistributionName);
    QString GetNNRAlgorithmName(int lAlgorithmIndex);

    int     GetOutlierToKeepIndex(const QString & lOutlierToKeep, bool * lValid = NULL);
    int     GetSPATRuleIndex(const QString & lRuleName, bool * lValid = NULL);
    int     GetRuleIndex(const QString &lRuleName, bool * lValid = NULL);
    int     GetRuleIndexForJson(const QString &lRuleName, bool * lValid = NULL);
    int     GetNNRRuleIndex(const QString & lRuleName, bool * lValid = NULL);
    int     GetSamplesToIgnoreIndex(const QString & lSamplesToIgnore, bool * lValid = NULL);
    int     GetOutlierLimitsSetIndex(const QString & lOutlierLimits, bool * lValid = NULL);
    int     GetMedianDriftUnitsIndex(const QString & lMeanDrifts, bool * lValid = NULL);
    int     GetDefaultRule(const QString & lMeanDrifts, bool * lValid = NULL);
    int     GetTailMngtRuleIndex(const QString &ruleName, bool * valid = NULL);
}
}
}

#endif // PAT_GLOBAL_H
