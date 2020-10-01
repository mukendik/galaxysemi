#include "pat_recipe.h"
#include "pat_recipe_private.h"

namespace GS
{
namespace Gex
{

PATRecipe::PATRecipe() : mPrivate(new PATRecipePrivate)
{
}

PATRecipe::~PATRecipe()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = NULL;
    }
}

COptionsPat &PATRecipe::GetOptions()
{
    return mPrivate->mOptions;
}

const COptionsPat &PATRecipe::GetOptions() const
{
    return mPrivate->mOptions;
}

QList<PATMultiVariateRule> &PATRecipe::GetMultiVariateRules()
{
    return mPrivate->mMVRules;
}

const QList<PATMultiVariateRule> &PATRecipe::GetMultiVariateRules() const
{
    return mPrivate->mMVRules;
}

QHash<QString, CPatDefinition *> &PATRecipe::GetUniVariateRules()
{
    return mPrivate->mUniVariateRules;
}

const QHash<QString, CPatDefinition *> &PATRecipe::GetUniVariateRules() const
{
    return mPrivate->mUniVariateRules;
}

//void PATRecipe::SetOptions(COptionsPat options)
//{
//    mPrivate->mOptions = options;
//}

void PATRecipe::SetMultiVariateRules(QList<PATMultiVariateRule>& multivariate)
{
    mPrivate->mMVRules = multivariate;
}

void PATRecipe::SetUniVariateRules(QHash<QString, CPatDefinition*>& univariate)
{
    mPrivate->mUniVariateRules = univariate;
}


CPatDefinition *PATRecipe::FindUniVariateRule(long lTestNumber, long lPinIndex,
                                              const QString &lTestName) const
{
    CPatDefinition *    lUVRule = NULL;
    QString             lKey;

    switch(mPrivate->mOptions.mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            lKey = QString::number(lTestNumber);
            if(lPinIndex >= 0)
                lKey += "." + QString::number(lPinIndex);
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            lKey = lTestName.trimmed();
            if(lPinIndex >= 0)
                lKey += "." + QString::number(lPinIndex);
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            lKey = lTestName.trimmed();
            lKey += "." + QString::number(lTestNumber);
            if(lPinIndex >= 0)
                lKey += "." + QString::number(lPinIndex);

            break;
    }

    if (mPrivate->mUniVariateRules.contains(lKey))
        lUVRule = mPrivate->mUniVariateRules.value(lKey);

    // Holds the Static PAT limits for each test
    return lUVRule;
}

bool PATRecipe::IsUsingParametric()
{
    // Scan all recipe rules to identify is STDF file required
    if(mPrivate->mOptions.bStaticPAT || mPrivate->mOptions.bDynamicPAT)
    {
        QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
        CPatDefinition	*                           ptPatDef = NULL;

        for(itPATDefinifion = mPrivate->mUniVariateRules.begin();
            itPATDefinifion != mPrivate->mUniVariateRules.end(); ++itPATDefinifion)
        {
            ptPatDef = *itPATDefinifion;

            // Check if SPAT/PPAT enabled for this test.
            if((ptPatDef->m_lFailStaticBin > 0) || (ptPatDef->m_lFailDynamicBin > 0))
                return true;
        }
    }

    // Check Flows used in NNR rules (rules always use STDF files)
    QList <CNNR_Rule>::iterator itNNR;
    CNNR_Rule lNNR_Rule;
    int iRuleID=0;
    for(itNNR = mPrivate->mOptions.GetNNRRules().begin();
        itNNR != mPrivate->mOptions.GetNNRRules().end(); ++itNNR, iRuleID++)
    {
        lNNR_Rule = *itNNR;
        if(lNNR_Rule.IsEnabled() && mPrivate->mOptions.IsNNREnabled())
            return true;	// NNR enabled: requires STDF input file!
    }

    // Check Flows used in IDDQ-Delta rules (rules always use STDF files)
    QList <CIDDQ_Delta_Rule>::iterator lItIDDQ_Delta;
    CIDDQ_Delta_Rule lIDDQ_Delta_Rule;
    iRuleID=0;
    for(lItIDDQ_Delta = mPrivate->mOptions.mIDDQ_Delta_Rules.begin();
        lItIDDQ_Delta != mPrivate->mOptions.mIDDQ_Delta_Rules.end(); ++lItIDDQ_Delta, iRuleID++)
    {
        lIDDQ_Delta_Rule = *lItIDDQ_Delta;
        if(lIDDQ_Delta_Rule.IsEnabled() && mPrivate->mOptions.mIsIDDQ_Delta_enabled)
            return true;	// IDDQ enabled: requires STDF input file!
    }

    // NO STDF parametric data required!
    return false;
}

void PATRecipe::Reset(PAT::RecipeType lRecipeType)
{
    mPrivate->mOptions.clear(lRecipeType);

    qDeleteAll(mPrivate->mUniVariateRules);
    mPrivate->mUniVariateRules.clear();

    mPrivate->mMVRules.clear();
}

}   // namespace Gex
}   // namespace GS

