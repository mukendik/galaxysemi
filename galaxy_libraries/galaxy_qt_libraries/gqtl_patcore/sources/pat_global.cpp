#include "gex_constants.h"
#include "gex_pat_constants.h"
#include "pat_global.h"
#include "pat_defines.h"
#include "gqtl_log.h"

namespace GS
{
namespace Gex
{
namespace PAT
{

QString GetDefaultRecipeName(RecipeType lType)
{
    QString lFileName;

    switch(lType)
    {
        case RecipeWaferSort:
            lFileName = GEX_PAT_CONFIG;
            break;

        case RecipeFinalTest:
            lFileName = GEX_PAT_FT_CONFIG;
            break;

        case RecipeUnknown:
            GSLOG(SYSLOG_SEV_WARNING, "Unknown recipe type.")
            break;
    }

    return lFileName;
}

/*
QString GetDefaultSharedRecipeName()
{
    QString lFileName   =   Engine::GetInstance().Get("UserFolder").toString() +
                            GEX_DEFAULT_DIR + "/temp/" + GEX_PAT_CONFIG_TEMP;

    return lFileName;
}
*/

QString GetOutlierTypeName(unsigned int lFailType)
{
    switch(lFailType)
    {
        case GEX_TPAT_BINTYPE_STATICFAIL:
            return "SPAT";

        case GEX_TPAT_BINTYPE_DYNAMICFAIL:
            return "DPAT";

        case GEX_TPAT_BINTYPE_NNR:
            return "NNR";

        case GEX_TPAT_BINTYPE_IDDQ_DELTA:
            return "IDDQ";

        case GEX_TPAT_BINTYPE_BADNEIGHBORS:
            return "GDBN";

        case GEX_TPAT_BINTYPE_RETICLE:
            return "Reticle";

        case GEX_TPAT_BINTYPE_BADCLUSTER:
            return "Clustering";

        case GEX_TPAT_BINTYPE_ZPAT:
            return "ZPAT";

        case GEX_TPAT_BINTYPE_MVPAT:
            return "MVPAT";

        default:
            return "Unknown";
    }
}

QString GetOutlierBinName(unsigned int lFailType)
{
    switch(lFailType)
    {
        case GEX_TPAT_BINTYPE_STATICFAIL:
            return "SPAT: Outliers";

        case GEX_TPAT_BINTYPE_DYNAMICFAIL:
            return "DPAT Outliers";

        case GEX_TPAT_BINTYPE_NNR:
            return "NNR Outliers";

        case GEX_TPAT_BINTYPE_IDDQ_DELTA:
            return "IDDQ-Delta Outliers";

        case GEX_TPAT_BINTYPE_BADNEIGHBORS:
            return "PAT: GDBN";

        case GEX_TPAT_BINTYPE_RETICLE:
            return "PAT: Reticle";

        case GEX_TPAT_BINTYPE_BADCLUSTER:
            return "PAT: Clustering";

        case GEX_TPAT_BINTYPE_ZPAT:
            return "Z-PAT";

        case GEX_TPAT_BINTYPE_MVPAT:
            return "MV-PAT";

        default:
            return "PAT";
    }
}

int GetDistributionID(const QString &lDistributionName)
{
    if(lDistributionName == "Gaussian")
        return PATMAN_LIB_SHAPE_GAUSSIAN;
    if(lDistributionName == "Gaussian_L")
        return PATMAN_LIB_SHAPE_GAUSSIAN_LEFT;
    if(lDistributionName == "Gaussian_R")
        return PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT;
    if(lDistributionName == "Log_L")
        return PATMAN_LIB_SHAPE_LOGNORMAL_LEFT;
    if(lDistributionName == "Log_R")
        return PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT;
    if(lDistributionName == "B_Modal")
        return PATMAN_LIB_SHAPE_BIMODAL;
    if(lDistributionName == "M_Modal")
        return PATMAN_LIB_SHAPE_MULTIMODAL;
    if(lDistributionName == "Clamp_L")
        return PATMAN_LIB_SHAPE_CLAMPED_LEFT;
    if(lDistributionName == "Clamp_R")
        return PATMAN_LIB_SHAPE_CLAMPED_RIGHT;
    if(lDistributionName == "Clamp_2")
        return PATMAN_LIB_SHAPE_DOUBLECLAMPED;
    if(lDistributionName == "Categories")
        return PATMAN_LIB_SHAPE_CATEGORY;
    if(lDistributionName == "Unknown")
        return PATMAN_LIB_SHAPE_UNKNOWN;

    return PATMAN_LIB_SHAPE_UNKNOWN;
}

int GetOutlierToKeepIndex(const QString & lOutlierToKeep, bool * lValid /*= NULL*/)
{
    int lID = 0;

    // Flag to tell if invalid string entry specified
    if(lValid != NULL)
        *lValid = true;

    // check if this selection name is in a GUI format (full name with spaces, etc...)
    while(gexKeepOutliersSetItemsGUI[lID])
    {
        if(lOutlierToKeep.startsWith(gexKeepOutliersSetItemsGUI[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Outliers to keep' possible selection
        lID++;
    };


    // No...then check name in the list of rule keywords (compact mode)
    lID = 0;
    while(gexKeepOutliersSetItemsKeywords[lID])
    {
        if(lOutlierToKeep.startsWith(gexKeepOutliersSetItemsKeywords[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Outliers to keep' possible selection
        lID++;
    };

    // entry specified is not valid!
    if(lValid != NULL)
        *lValid = false;

    // String not found...force selection to ignore all outliers (keep = none)!
    return GEX_TPAT_KEEPTYPE_NONEID;
}

int GetSPATRuleIndex(const QString & lRuleName, bool * lValid /*= NULL*/)
{
    int     lID = 0;
    QString lKeyword;

    // Flag to tell if invalid string entry specified
    if(lValid != NULL)
        *lValid = true;

    // check if this rule name is in a GUI format (full name with spaces, etc...)
    while(gexSpatRuleSetItemsGUI[lID])
    {
        lKeyword = gexSpatRuleSetItemsGUI[lID];

        // Check if the text matches the rule name?
        if(lKeyword.startsWith(lRuleName, Qt::CaseInsensitive))
            return lID;

        // Move to next rule
        lID++;
    };

    // No...then check name in the list of rule keywords (compact mode)
    lID = 0;
    while(gexSpatRuleSetItemsKeywords[lID])
    {
        lKeyword = gexSpatRuleSetItemsKeywords[lID];

        // Check if the text matches the rule name?
        if(lKeyword.startsWith(lRuleName, Qt::CaseInsensitive))
            return lID;

        // Move to next rule
        lID++;
    };

    // entry specified is not valid!
    if(lValid != NULL)
        *lValid = false;

    // Rule name not found....default to AEC rule
    return GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA;
}

int GetRuleIndexForJson(const QString &lRuleName, bool * lValid /*= NULL*/)
{
    int     lID = 0;
    QString	lKeyword;

    // Flag to tell if invalid string entry specified
    if(lValid != NULL)
        *lValid = true;

    // check if this rule name is in a GUI format (full name with spaces, etc...)
    while(gexRuleSetItemsKeywordsForJson[lID])
    {
        // Get rule from rule list
        lKeyword = gexRuleSetItemsKeywordsForJson[lID];
        // Check if the text matches the rule name?
        if(lKeyword.startsWith(lRuleName, Qt::CaseInsensitive))
            return lID;

        // Move to next rule
        lID++;
    };

    // Rule name not found....ignore entry!
    return GEX_TPAT_RULETYPE_SIGMAID;
}

int GetDefaultRule(const QString &lRuleName, bool * lValid /*= NULL*/)
{
    int     lID = 0;
    QString	lKeyword;

    // Flag to tell if invalid string entry specified
    if(lValid != NULL)
        *lValid = true;

    // check if this rule name is in a GUI format (full name with spaces, etc...)
    while(gexDefaultRuleSet[lID])
    {
        // Get rule from rule list
        lKeyword = gexDefaultRuleSet[lID];
        // Check if the text matches the rule name?
        if(lKeyword.startsWith(lRuleName, Qt::CaseInsensitive))
            return lID;

        // Move to next rule
        lID++;
    };

    // Rule name not found....ignore entry!
    return GEX_TPAT_RULETYPE_SIGMAID;
}

int GetRuleIndex(const QString &lRuleName, bool * lValid /*= NULL*/)
{
    int     lID = 0;
    QString	lKeyword;

    // Flag to tell if invalid string entry specified
    if(lValid != NULL)
        *lValid = true;

    // check if this rule name is in a GUI format (full name with spaces, etc...)
    while(gexRuleSetItemsLongNamesGUI[lID])
    {
        // Get rule from rule list
        lKeyword = gexRuleSetItemsLongNamesGUI[lID];
        // Check if the text matches the rule name?
        if(lKeyword.startsWith(lRuleName, Qt::CaseInsensitive))
            return lID;

        // Move to next rule
        lID++;
    };

    // No...check short names then
    lID = 0;
    while(gexRuleSetItemsGUI[lID])
    {
        // Get rule from rule list
        lKeyword = gexRuleSetItemsGUI[lID];
        // Check if the text matches the rule name?
        if(lKeyword.startsWith(lRuleName, Qt::CaseInsensitive))
            return lID;

        // Move to next rule
        lID++;
    };

    // No...then check name in the list of rule keywords (compact mode)
    lID = 0;
    while(gexRuleSetItemsKeywords[lID])
    {
        // Get rule from rule list
        lKeyword = gexRuleSetItemsKeywords[lID];
        // Check if the text matches the rule name?
        if(lKeyword.startsWith(lRuleName, Qt::CaseInsensitive))
            return lID;

        // Move to next rule
        lID++;
    };

    // entry specified is not valid!
    if(lValid != NULL)
        *lValid = false;

    // Rule name not found....ignore entry!
    return GEX_TPAT_RULETYPE_IGNOREID;
}

int GetNNRRuleIndex(const QString & lRuleName, bool * lValid /*= NULL*/)
{
    int lID = 0;

    // Flag to tell if invalid string entry specified
    if(lValid != NULL)
        *lValid = true;

    // check if this selection name is in a GUI format (full name with spaces, etc...)
    while(gexNnrRuleSetItemsGUI[lID])
    {
        if(lRuleName.startsWith(gexNnrRuleSetItemsGUI[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Outliers to keep' possible selection
        lID++;
    };


    // No...then check name in the list of rule keywords (compact mode)
    lID = 0;
    while(gexNnrRuleSetItemsKeywords[lID])
    {
        if(lRuleName.startsWith(gexNnrRuleSetItemsKeywords[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Outliers to keep' possible selection
        lID++;
    };

    // entry specified is not valid!
    if(lValid != NULL)
        *lValid = false;

    // String not found...force selection to 'NNR rule: Enabled'
    return GEX_TPAT_NNR_ENABLED;
}

int GetSamplesToIgnoreIndex(const QString & lSamplesToIgnore, bool * lValid /*= NULL*/)
{
    int lID = 0;

    // Flag to tell if invalid string entry specified
    if(lValid != NULL)
        *lValid = true;

    // check if this selection name is in a GUI format (full name with spaces, etc...)
    while(gexIgnoreSamplesSetItemsGUI[lID])
    {
        if(lSamplesToIgnore.startsWith(gexIgnoreSamplesSetItemsGUI[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Samples to ignore' possible selection
        lID++;
    };


    // No...then check name in the list of rule keywords (compact mode)
    lID = 0;
    while(gexIgnoreSamplesSetItemsKeywords[lID])
    {
        if(lSamplesToIgnore.startsWith(gexIgnoreSamplesSetItemsKeywords[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Samples to Ignore' possible selection
        lID++;
    };

    // entry specified is not valid!
    if(lValid != NULL)
        *lValid = false;

    // String not found...force selection to process all samples (ignore=none)
    return GEX_TPAT_IGNOREDATA_NONEID;
}

int GetOutlierLimitsSetIndex(const QString & lOutlierLimits, bool * lValid /*= NULL*/)
{
    int lID = 0;

    // Flag to tell if invalid string entry specified
    if(lValid != NULL)
        *lValid = true;

    // check if this selection name is in a GUI LONG format (full name with spaces, etc...)
    while(gexOutlierLimitsSetItemsLongNamesGUI[lID])
    {
        if(lOutlierLimits.startsWith(gexOutlierLimitsSetItemsLongNamesGUI[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Outlier Limits set' possible selection
        lID++;
    };

    // No...check if this selection name is in a GUI format (full name with spaces, etc...)
    lID = 0;
    while(gexOutlierLimitsSetItemsGUI[lID])
    {
        if(lOutlierLimits.startsWith(gexOutlierLimitsSetItemsGUI[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Outlier Limits set' possible selection
        lID++;
    };


    // No...then check name in the list of rule keywords (compact mode)
    lID = 0;
    while(gexOutlierLimitsSetItemsKeywords[lID])
    {
        if(lOutlierLimits.startsWith(gexOutlierLimitsSetItemsKeywords[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Outliers Limits set' possible selection
        lID++;
    };

    // entry specified is not valid!
    if(lValid != NULL)
        *lValid = false;

    // String not found...force selection to use the smallest limits set
    // (all outliers to be removed: near, medium and far)
    return GEX_TPAT_LIMITSSET_NEAR;
}

int GetMedianDriftUnitsIndex(const QString & lMeanDrifts, bool * lValid /*= NULL*/)
{
    int lID = 0;

    // Flag to tell if invalid string entry specified
    if(lValid != NULL)
        *lValid = true;

    // Check if this selection name is in a GUI format (full name with spaces, etc...)
    while(gexMedianDriftUnitsSetItemsGUI[lID])
    {
        if(lMeanDrifts.startsWith(gexMedianDriftUnitsSetItemsGUI[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Outlier Limits set' possible selection
        lID++;
    };


    // No...then check name in the list of keywords (compact mode)
    lID = 0;
    while(gexMedianDriftUnitsSetItemsKeywords[lID])
    {
        if(lMeanDrifts.startsWith(gexMedianDriftUnitsSetItemsKeywords[lID], Qt::CaseInsensitive))
            return lID;

        // Move to next 'Outliers Limits set' possible selection
        lID++;
    };

    // entry specified is not valid!
    if(lValid != NULL)
        *lValid = false;

    // String not found...force selection to default (N*Sigma)
    return GEX_TPAT_DRIFT_UNITS_NONE;
}

QString GetNNRAlgorithmName(int lAlgorithmIndex)
{
    QString lName = "Unknown";

    switch(lAlgorithmIndex)
    {
        case GEX_TPAT_NNR_ALGO_LOCAL_SIGMA:
            lName = "Mean +/- N*Sigma";
            break;
        case GEX_TPAT_NNR_ALGO_LOCAL_MEDIAN:
            lName = "Median +/- N*RSigma";
            break;
        case GEX_TPAT_NNR_ALGO_LOCAL_Q1Q3IQR:
            lName = "IQR: Q1-N*IQR, Q3+N*IQR";
            break;
        default:
            break;
    }

    return lName;
}

int GetTailMngtRuleIndex(const QString &ruleName, bool *valid)
{
    int     lID = 0;
    QString lKeyword;

    // Flag to tell if invalid string entry specified
    if(valid != NULL)
        *valid = true;

    // check if this rule name is in a GUI format (full name with spaces, etc...)
    while(gexTailMngtRuleSetItemsGUI[lID])
    {
        lKeyword = gexTailMngtRuleSetItemsGUI[lID];
        // Check if the text matches the rule name?
        if(lKeyword.startsWith(ruleName, Qt::CaseInsensitive))
            return lID;
        // Move to next rule
        ++lID;
    };

    // No...then check name in the list of rule keywords (compact mode)
    lID = 0;
    while(gexTailMngtRuleSetItemsKeywords[lID])
    {
        lKeyword = gexTailMngtRuleSetItemsKeywords[lID];
        // Check if the text matches the rule name?
        if(lKeyword.startsWith(ruleName, Qt::CaseInsensitive))
            return lID;
        // Move to next rule
        ++lID;
    };

    // entry specified is not valid!
    if(valid != NULL)
        *valid = false;

    // Rule name not found... error
    return -1;
}

}
}
}
