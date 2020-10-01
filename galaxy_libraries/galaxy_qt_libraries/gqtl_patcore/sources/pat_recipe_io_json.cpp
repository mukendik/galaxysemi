#include "pat_recipe_io_json.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRegularExpression>

#include <gqtl_log.h>
#include "pat_global.h"
#include "pat_definition.h"
#include "pat_options.h"
#include "pat_recipe_io.h"
#include "pat_rules.h"
#include "pat_recipe.h"
#include "test_defines.h"
#include "gex_pat_constants_extern.h"
#include "gs_types.h"

#include "gqtl_utils.h"

#include <QFile>
#include <QFileInfo>

#include <math.h>

namespace GS
{
namespace Gex
{
PATRecipeIOJson::PATRecipeIOJson()
    : PATRecipeIO(PATRecipeIO::JSON)
{

}
PATRecipeIOJson::~PATRecipeIOJson()
{
}

bool PATRecipeIOJson::Write(PATRecipe &lPATRecipe, const QString &recipeName /*= ""*/ )
{
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Writing recipe file %1").arg(recipeName).toLatin1().constData());

    QByteArray recipe ;
    QJsonObject lRecipeObject;
    QFile file(recipeName);
    file.open(QIODevice::WriteOnly);

    // Make sure Major and Minor version are valid, otherwise force to default one
    if (lPATRecipe.GetOptions().iRecipeVersion < DEFAULT_RECIPE_VERSION)
    {
        lPATRecipe.GetOptions().iRecipeVersion    = DEFAULT_RECIPE_VERSION;
        lPATRecipe.GetOptions().iRecipeBuild      = DEFAULT_RECIPE_BUILD;
    }
    if (lPATRecipe.GetOptions().iRecipeBuild < DEFAULT_RECIPE_BUILD)
        lPATRecipe.GetOptions().iRecipeBuild = DEFAULT_RECIPE_VERSION;

    // PAT-71
    lRecipeObject.insert("version", QJsonValue(QString(PAT::sRecipeVersion)));

    // write parts of the recie
    WriteOptions(lPATRecipe.GetOptions(), lRecipeObject);
    WriteUnivariates(lPATRecipe.GetUniVariateRules(), lRecipeObject);
    if (lPATRecipe.GetOptions().GetRecipeType() == PAT::RecipeWaferSort)
    {
        lRecipeObject.insert("testing_stage", QJsonValue(QString("wafer_sort")));
        WriteMultivariates(lPATRecipe.GetMultiVariateRules(), lRecipeObject);
    }
    else
    {
        lRecipeObject.insert("testing_stage", QJsonValue(QString("final_test")));
    }

    // convert the json object to a
    QJsonDocument d;
    d.setObject(lRecipeObject);
    recipe = d.toJson();
    file.write(recipe);
    file.close();
    return true;
}

bool PATRecipeIOJson::Write(COptionsPat &lPatOptions, const QString &recipeName)
{
    QByteArray recipe ;
    QJsonObject lRecipeObject;
    QFile file(recipeName);
    file.open(QIODevice::WriteOnly);

    // write parts of the recie
    WriteOptions(lPatOptions, lRecipeObject);

    // convert the json object to a
    QJsonDocument d;
    d.setObject(lRecipeObject);
    recipe = d.toJson();
    file.write(recipe);
    file.close();
    return true;
}

bool PATRecipeIOJson::WriteOptions(COptionsPat& options, QJsonObject& lOptions)
{
    QJsonObject outlierOptions;
    QJsonObject smartRules = WriteSmartRules(options);
    outlierOptions.insert("smart_rules", smartRules);

    if (options.GetRecipeType() == PAT::RecipeWaferSort)
    {
        QJsonObject waferSort = WriteWaferSort(options);
        outlierOptions.insert("wafer_sort", waferSort);
    }
    else
    {
        QJsonObject finalTest = WriteFinalTest(options);
        outlierOptions.insert("final_test", finalTest);
    }

    QJsonObject settings = WriteSettings(options);
    outlierOptions.insert("settings", settings);

    lOptions.insert("outlier_options", QJsonValue(outlierOptions));
    return true;
}

QJsonObject PATRecipeIOJson::WriteSmartRules(COptionsPat& aPATOptions)
{
    QString str;
    QJsonObject smartRules;

    QJsonObject shapes;
    QJsonObject shape;
    QJsonObject limitFactors;
    QJsonArray limitFactorNear;
    QJsonArray limitFactorMedium;
    QJsonArray limitFactorFar;

    ////////////// Gaussian //////////////////
    shape.insert(APPLY_PAT_KEY, QJsonValue(aPATOptions.bPAT_Gaussian));
    switch (aPATOptions.iAlgo_Gaussian) {
    case 0:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 1:
        str = MEAN_ROBUST_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 2:
        str = IQR_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 3:
        str = CUSTOMER_PAT_LIB_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    default:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    }
    // set limit factors
    limitFactorNear.insert(0, QJsonValue(aPATOptions.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactorNear.insert(1, QJsonValue(aPATOptions.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactors.insert("near", QJsonValue(limitFactorNear));

    limitFactorMedium.insert(0, QJsonValue(aPATOptions.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactorMedium.insert(1, QJsonValue(aPATOptions.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactors.insert("medium", QJsonValue(limitFactorMedium));

    limitFactorFar.insert(0, QJsonValue(aPATOptions.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactorFar.insert(1, QJsonValue(aPATOptions.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactors.insert("far", QJsonValue(limitFactorFar));

    shape.insert("limit_factors", QJsonValue(limitFactors));
    shapes.insert("gaussian", QJsonValue(shape));

    ////////////// Gaussian Tailed //////////////////
    shape.insert(APPLY_PAT_KEY, QJsonValue(aPATOptions.bPAT_GaussianTailed));
    switch (aPATOptions.iAlgo_GaussianTailed) {
    case 0:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 1:
        str = MEAN_ROBUST_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 2:
        str = IQR_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 3:
        str = CUSTOMER_PAT_LIB_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    default:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    }
    // set limit factors
    limitFactorNear.removeFirst();limitFactorNear.removeFirst();
    limitFactorNear.insert(0, QJsonValue(aPATOptions.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactorNear.insert(1, QJsonValue(aPATOptions.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactors.insert("near", QJsonValue(limitFactorNear));

    limitFactorMedium.removeFirst();limitFactorMedium.removeFirst();
    limitFactorMedium.insert(0, QJsonValue(aPATOptions.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactorMedium.insert(1, QJsonValue(aPATOptions.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactors.insert("medium", QJsonValue(limitFactorMedium));

    limitFactorFar.removeFirst();limitFactorFar.removeFirst();
    limitFactorFar.insert(0, QJsonValue(aPATOptions.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactorFar.insert(1, QJsonValue(aPATOptions.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactors.insert("far", QJsonValue(limitFactorFar));

    shape.insert("limit_factors", QJsonValue(limitFactors));
    shapes.insert("gaussian_tailed", QJsonValue(shape));

    ////////////// gaussian Doubl Tailed //////////////////
    shape.insert(APPLY_PAT_KEY, QJsonValue(aPATOptions.bPAT_GaussianDoubleTailed));
    switch (aPATOptions.iAlgo_GaussianDoubleTailed) {
    case 0:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 1:
        str = MEAN_ROBUST_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 2:
        str = IQR_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 3:
        str = CUSTOMER_PAT_LIB_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    default:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    }
    // set limit factors
    limitFactorNear.removeFirst();limitFactorNear.removeFirst();
    limitFactorNear.insert(0,
                         QJsonValue(aPATOptions.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactorNear.insert(1,
                         QJsonValue(aPATOptions.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactors.insert("near", QJsonValue(limitFactorNear));

    limitFactorMedium.removeFirst();limitFactorMedium.removeFirst();
    limitFactorMedium.insert(0,
                         QJsonValue(aPATOptions.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactorMedium.insert(1,
                         QJsonValue(aPATOptions.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactors.insert("medium", QJsonValue(limitFactorMedium));

    limitFactorFar.removeFirst();limitFactorFar.removeFirst();
    limitFactorFar.insert(0,
                        QJsonValue(aPATOptions.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactorFar.insert(1,
                        QJsonValue(aPATOptions.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactors.insert("far", QJsonValue(limitFactorFar));

    shape.insert("limit_factors", QJsonValue(limitFactors));
    shapes.insert("gaussian_double_tailed", QJsonValue(shape));

    ////////////// Log Normal //////////////////
    shape.insert(APPLY_PAT_KEY, QJsonValue(aPATOptions.bPAT_LogNormal));
    switch (aPATOptions.iAlgo_LogNormal) {
    case 0:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 1:
        str = MEAN_ROBUST_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 2:
        str = IQR_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 3:
        str = CUSTOMER_PAT_LIB_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    default:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    }
    // set limit factors
    limitFactorNear.removeFirst();limitFactorNear.removeFirst();
    limitFactorNear.insert(0, QJsonValue(aPATOptions.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactorNear.insert(1, QJsonValue(aPATOptions.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactors.insert("near", QJsonValue(limitFactorNear));

    limitFactorMedium.removeFirst();limitFactorMedium.removeFirst();
    limitFactorMedium.insert(0, QJsonValue(aPATOptions.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactorMedium.insert(1, QJsonValue(aPATOptions.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactors.insert("medium", QJsonValue(limitFactorMedium));

    limitFactorFar.removeFirst();limitFactorFar.removeFirst();
    limitFactorFar.insert(0, QJsonValue(aPATOptions.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactorFar.insert(1, QJsonValue(aPATOptions.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactors.insert("far", QJsonValue(limitFactorFar));

    shape.insert("limit_factors", QJsonValue(limitFactors));
    shapes.insert("log_normal", QJsonValue(shape));

    ////////////// Multi Modal //////////////////
    shape.insert(APPLY_PAT_KEY, QJsonValue(aPATOptions.bPAT_MultiModal));
    switch (aPATOptions.iAlgo_MultiModal) {
    case 0:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 1:
        str = MEAN_ROBUST_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 2:
        str = IQR_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 3:
        str = CUSTOMER_PAT_LIB_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    default:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    }
    // set limit factors
    limitFactorNear.removeFirst();limitFactorNear.removeFirst();
    limitFactorNear.insert(0, QJsonValue(aPATOptions.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactorNear.insert(1, QJsonValue(aPATOptions.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactors.insert("near", QJsonValue(limitFactorNear));

    limitFactorMedium.removeFirst();limitFactorMedium.removeFirst();
    limitFactorMedium.insert(0, QJsonValue(aPATOptions.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactorMedium.insert(1, QJsonValue(aPATOptions.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactors.insert("medium", QJsonValue(limitFactorMedium));

    limitFactorFar.removeFirst();limitFactorFar.removeFirst();
    limitFactorFar.insert(0, QJsonValue(aPATOptions.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactorFar.insert(1, QJsonValue(aPATOptions.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactors.insert("far", QJsonValue(limitFactorFar));

    shape.insert("limit_factors", QJsonValue(limitFactors));
    shapes.insert("multi_modal", QJsonValue(shape));

    ////////////// Clamped //////////////////
    shape.insert(APPLY_PAT_KEY, QJsonValue(aPATOptions.bPAT_Clamped));
    switch (aPATOptions.iAlgo_Clamped) {
    case 0:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 1:
        str = MEAN_ROBUST_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 2:
        str = IQR_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 3:
        str = CUSTOMER_PAT_LIB_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    default:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    }
    // set limit factors
    limitFactorNear.removeFirst();limitFactorNear.removeFirst();
    limitFactorNear.insert(0, QJsonValue(aPATOptions.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactorNear.insert(1, QJsonValue(aPATOptions.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactors.insert("near", QJsonValue(limitFactorNear));

    limitFactorMedium.removeFirst();limitFactorMedium.removeFirst();
    limitFactorMedium.insert(0, QJsonValue(aPATOptions.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactorMedium.insert(1, QJsonValue(aPATOptions.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactors.insert("medium", QJsonValue(limitFactorMedium));

    limitFactorFar.removeFirst();limitFactorFar.removeFirst();
    limitFactorFar.insert(0, QJsonValue(aPATOptions.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactorFar.insert(1, QJsonValue(aPATOptions.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactors.insert("far", QJsonValue(limitFactorFar));

    shape.insert("limit_factors", QJsonValue(limitFactors));
    shapes.insert("clamped", QJsonValue(shape));

    ////////////// Double Clamped //////////////////
    shape.insert(APPLY_PAT_KEY, QJsonValue(aPATOptions.bPAT_DoubleClamped));
    switch (aPATOptions.iAlgo_DoubleClamped) {
    case 0:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 1:
        str = MEAN_ROBUST_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 2:
        str = IQR_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 3:
        str = CUSTOMER_PAT_LIB_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    default:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    }
    // set limit factors
    limitFactorNear.removeFirst();limitFactorNear.removeFirst();
    limitFactorNear.insert(0, QJsonValue(aPATOptions.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactorNear.insert(1, QJsonValue(aPATOptions.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactors.insert("near", QJsonValue(limitFactorNear));

    limitFactorMedium.removeFirst();limitFactorMedium.removeFirst();
    limitFactorMedium.insert(0, QJsonValue(aPATOptions.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactorMedium.insert(1, QJsonValue(aPATOptions.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactors.insert("medium", QJsonValue(limitFactorMedium));

    limitFactorFar.removeFirst();limitFactorFar.removeFirst();
    limitFactorFar.insert(0, QJsonValue(aPATOptions.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactorFar.insert(1, QJsonValue(aPATOptions.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactors.insert("far", QJsonValue(limitFactorFar));

    shape.insert("limit_factors", QJsonValue(limitFactors));
    shapes.insert("double_clamped", QJsonValue(shape));

    ////////////// Category //////////////////
    shape.insert(APPLY_PAT_KEY, QJsonValue(aPATOptions.bPAT_Category));
    switch (aPATOptions.iAlgo_Category) {
    case 0:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 1:
        str = MEAN_ROBUST_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 2:
        str = IQR_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    case 3:
        str = CUSTOMER_PAT_LIB_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    default:
        str = MEAN_N_SIGMA_KEY;
        shape.insert(FORMULA_KEY, QJsonValue(str));
        break;
    }
    // set limit factors
    limitFactorNear.removeFirst();limitFactorNear.removeFirst();
    limitFactorNear.insert(0, QJsonValue(aPATOptions.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactorNear.insert(1, QJsonValue(aPATOptions.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactors.insert("near", QJsonValue(limitFactorNear));

    limitFactorMedium.removeFirst();limitFactorMedium.removeFirst();
    limitFactorMedium.insert(0, QJsonValue(aPATOptions.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactorMedium.insert(1, QJsonValue(aPATOptions.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactors.insert("medium", QJsonValue(limitFactorMedium));

    limitFactorFar.removeFirst();limitFactorFar.removeFirst();
    limitFactorFar.insert(0, QJsonValue(aPATOptions.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactorFar.insert(1, QJsonValue(aPATOptions.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactors.insert("far", QJsonValue(limitFactorFar));

    shape.insert("limit_factors", QJsonValue(limitFactors));
    shapes.insert("category", QJsonValue(shape));

    ////////////// Unknown //////////////////
    shape.insert(APPLY_PAT_KEY, QJsonValue(aPATOptions.bPAT_Unknown));
    switch (aPATOptions.iAlgo_Unknown)
    {
        case 0:
            str = MEAN_N_SIGMA_KEY;
            break;
        case 1:
            str = MEAN_ROBUST_SIGMA_KEY;
            break;
        case 2:
            str = IQR_KEY;
            break;
        case 3:
            str = CUSTOMER_PAT_LIB_KEY;
            break;
        default:
            str = MEAN_N_SIGMA_KEY;
            break;
    }
    shape.insert(FORMULA_KEY, QJsonValue(str));
    // set limit factors
    limitFactorNear.removeFirst();limitFactorNear.removeFirst();
    limitFactorNear.insert(0, QJsonValue(aPATOptions.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactorNear.insert(1, QJsonValue(aPATOptions.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]));
    limitFactors.insert("near", QJsonValue(limitFactorNear));

    limitFactorMedium.removeFirst();limitFactorMedium.removeFirst();
    limitFactorMedium.insert(0, QJsonValue(aPATOptions.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactorMedium.insert(1, QJsonValue(aPATOptions.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]));
    limitFactors.insert("medium", QJsonValue(limitFactorMedium));

    limitFactorFar.removeFirst();limitFactorFar.removeFirst();
    limitFactorFar.insert(0, QJsonValue(aPATOptions.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactorFar.insert(1, QJsonValue(aPATOptions.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]));
    limitFactors.insert("far", QJsonValue(limitFactorFar));

    shape.insert("limit_factors", QJsonValue(limitFactors));
    shapes.insert("unknown", QJsonValue(shape));


    smartRules.insert("shapes", QJsonValue(shapes));

    ////////////// MIN CONF THRESHOLD /////////
    smartRules.insert("min_confidence_threshold", QJsonValue(aPATOptions.mMinConfThreshold));
    return smartRules;
}


QJsonObject PATRecipeIOJson::WriteWaferSort(COptionsPat& options)
{
    QString str;
    QJsonObject waferSort;

    /////////////////// NNR ///////////////////////////////////
    QJsonObject nnr;
    nnr.insert(ENABLED_KEY, QJsonValue(options.IsNNREnabled()));
    nnr.insert(HARD_BIN_KEY, QJsonValue(options.GetNNRHardBin()));
    nnr.insert(SOFT_BIN_KEY, QJsonValue(options.GetNNRSoftBin()));
    nnr.insert(COLOR_KEY, ColorRGBToJsonArray(options.GetNNRColor()));

    QJsonArray nnrRules;
    CNNR_Rule lNNRRule;
    int i=0;
    for(QList <CNNR_Rule>::iterator itNNR = options.GetNNRRules().begin();
        itNNR != options.GetNNRRules().end();
        ++itNNR)
    {
        QJsonObject lJsonNNRRule;
        lNNRRule = *itNNR;
        lJsonNNRRule.insert(NAME_KEY, QJsonValue(lNNRRule.GetRuleName()));
        lJsonNNRRule.insert("location_averaging", QJsonValue(lNNRRule.GetLA()));

        switch(lNNRRule.GetAlgorithm())
        {
            case 0:
                str = MEAN_N_SIGMA_KEY;
                break;
            case 1:
                str = MEAN_ROBUST_SIGMA_KEY;
                break;
            case 2:
                str = IQR_KEY;
                break;
            default:
                str = MEAN_N_SIGMA_KEY;
                break;
        }
        lJsonNNRRule.insert(ALGORITHM_KEY, QJsonValue(str));
        lJsonNNRRule.insert("factor", QJsonValue(lNNRRule.GetNFactor()));
        lJsonNNRRule.insert("cluster_size", QJsonValue(lNNRRule.GetClusterSize()));
        lJsonNNRRule.insert(ENABLED_KEY, QJsonValue(lNNRRule.IsEnabled()));
        lJsonNNRRule.insert(SOFT_BIN_KEY, QJsonValue(lNNRRule.GetSoftBin()));
        lJsonNNRRule.insert(HARD_BIN_KEY, QJsonValue(lNNRRule.GetHardBin()));
        lJsonNNRRule.insert(COLOR_KEY, ColorRGBToJsonArray(lNNRRule.GetFailBinColor()));
        nnrRules.insert(i, lJsonNNRRule);
        ++i;
    }
    nnr.insert(RULE_KEY, QJsonValue(nnrRules));
    waferSort.insert("nnr",  QJsonValue(nnr));

    /////////////////// IDDQ Delta ///////////////////////////////////
    QJsonObject iddqDelta;
    iddqDelta.insert(ENABLED_KEY, QJsonValue(options.mIsIDDQ_Delta_enabled));
    iddqDelta.insert(HARD_BIN_KEY, QJsonValue(options.mIDDQ_Delta_HBin));
    iddqDelta.insert(SOFT_BIN_KEY, QJsonValue(options.mIDDQ_Delta_SBin));
    iddqDelta.insert(COLOR_KEY, ColorRGBToJsonArray(options.mIDDQ_Delta_Color));

    QJsonArray iddqRules;
    CIDDQ_Delta_Rule lIddqRule;
    i=0;
    for(QList <CIDDQ_Delta_Rule>::iterator itIddq = options.mIDDQ_Delta_Rules.begin();
        itIddq != options.mIDDQ_Delta_Rules.end();
        ++itIddq)
    {
        QJsonObject lJsonIddqRule;
        lIddqRule = *itIddq;
        lJsonIddqRule.insert(NAME_KEY, QJsonValue(lIddqRule.GetRuleName()));
        lJsonIddqRule.insert("pre_stress", QJsonValue(lIddqRule.GetPreStress()));
        lJsonIddqRule.insert("post_stress", QJsonValue(lIddqRule.GetPostStress()));
        lJsonIddqRule.insert("case_sensitive", QJsonValue(lIddqRule.GetCaseSensitive()));
        lJsonIddqRule.insert("factor", QJsonValue(lIddqRule.GetNFactor()));

        switch(lIddqRule.GetAlgorithm())
        {
            case 0:
                str = MEAN_N_SIGMA_KEY;
                break;
            case 1:
                str = MEAN_ROBUST_SIGMA_KEY;
                break;
            case 2:
                str = IQR_KEY;
                break;
            default:
                str = MEAN_N_SIGMA_KEY;
                break;
        }
        lJsonIddqRule.insert(ALGORITHM_KEY, QJsonValue(str));
        lJsonIddqRule.insert(ENABLED_KEY, QJsonValue(lIddqRule.IsEnabled()));
        lJsonIddqRule.insert(SOFT_BIN_KEY, QJsonValue(lIddqRule.GetSoftBin()));
        lJsonIddqRule.insert(HARD_BIN_KEY, QJsonValue(lIddqRule.GetHardBin()));
        lJsonIddqRule.insert(COLOR_KEY, ColorRGBToJsonArray(lIddqRule.GetFailBinColor()));
        iddqRules.insert(i, lJsonIddqRule);
        ++i;
    }
    iddqDelta.insert(RULE_KEY, QJsonValue(iddqRules));
    waferSort.insert("iddq_delta",  QJsonValue(iddqDelta));

    /////////////////// GDBN ///////////////////////////////////
    QJsonObject gdbn;
    gdbn.insert(ENABLED_KEY, QJsonValue(options.mIsGDBNEnabled));
    gdbn.insert(HARD_BIN_KEY, QJsonValue(options.mGDBNPatHBin));
    gdbn.insert(SOFT_BIN_KEY, QJsonValue(options.mGDBNPatSBin));
    gdbn.insert(COLOR_KEY, ColorRGBToJsonArray(options.mGDBNColor));

    QJsonArray gdbnRules;
    CGDBN_Rule lGdbnRule;
    i=0;
    for(QList <CGDBN_Rule>::iterator lGdbnIter = options.mGDBNRules.begin();
        lGdbnIter != options.mGDBNRules.end();
        ++lGdbnIter)
    {
        QJsonObject lJsonGdbnRule;
        lGdbnRule = *lGdbnIter;
        lJsonGdbnRule.insert(NAME_KEY, QJsonValue(lGdbnRule.mRuleName));
        lJsonGdbnRule.insert("yield_threshold", QJsonValue(lGdbnRule.mYieldThreshold));

        QJsonArray  lBinsArray  = StringBinsListToJsonArray(lGdbnRule.mBadBinList->GetRangeList());

        lJsonGdbnRule.insert(BAD_BINS_KEY, QJsonValue(lBinsArray));

        switch(lGdbnRule.mWafermapSource)
        {
            case GEX_PAT_WAFMAP_SRC_SOFTBIN:
                str = STDF_SOFT_BIN_KEY;
                break;
            case GEX_PAT_WAFMAP_SRC_HARDBIN:
                str = STDF_HARD_BIN_KEY;
                break;
            case GEX_PAT_WAFMAP_SRC_PROBER:
                str = EXTERNAL_KEY;
                break;
            default:
                str = STDF_SOFT_BIN_KEY;
                break;
        }
        lJsonGdbnRule.insert("map_source", QJsonValue(str));

        switch(lGdbnRule.mAlgorithm)
        {
            case 0:
                str = "squeeze";
                break;
            case 1:
                str = "weighting";
                break;
            default:
                str = "squeeze";
                break;
        }
        lJsonGdbnRule.insert(ALGORITHM_KEY, QJsonValue(str));

        lJsonGdbnRule.insert("fail_wafer_edge", QJsonValue(lGdbnRule.mFailWaferEdges));
        lJsonGdbnRule.insert("cluster_size", QJsonValue(lGdbnRule.mClusterSize));
        lJsonGdbnRule.insert("squeeze_count", QJsonValue(lGdbnRule.mFailCount));

        QJsonArray adjWeight;
        for (int compt=0; compt < lGdbnRule.mAdjWeightLst.size(); ++compt)
        {
            adjWeight.insert(compt, lGdbnRule.mAdjWeightLst[compt]);
        }
        lJsonGdbnRule.insert("adjacent_weight", QJsonValue(adjWeight));

        QJsonArray diagWeight;
        for (int compt=0; compt < lGdbnRule.mDiagWeightLst.size(); ++compt)
        {
            diagWeight.insert(compt, lGdbnRule.mDiagWeightLst[compt]);
        }
        lJsonGdbnRule.insert("diag_weight", QJsonValue(diagWeight));

        lJsonGdbnRule.insert("fail_weight", QJsonValue(lGdbnRule.mMinimumWeighting));
        switch(lGdbnRule.mEdgeDieWeighting)
        {
            case 0:
                str = "ignored";
                break;
            case 1:
                str = "good";
                break;
            case 2:
                str = "bad";
                break;
            case 3:
                str = "scaled";
                break;
            default:
                str = "ignored";
                break;
        }
        lJsonGdbnRule.insert("edge_die_weight", QJsonValue(str));
        lJsonGdbnRule.insert("weight_scale", QJsonValue(lGdbnRule.mEdgeDieWeightingScale));
        lJsonGdbnRule.insert(ENABLED_KEY, QJsonValue(lGdbnRule.mIsEnabled));
        lJsonGdbnRule.insert("mask_name", QJsonValue(lGdbnRule.mMaskName));

        switch(lGdbnRule.mEdgeDieType)
        {
            case 3:
                str = "all";
                break;
            case 1:
                str = "adjacent";
                break;
            case 2:
                str = "corner";
                break;
            default:
                str = "all";
                break;
        }
        lJsonGdbnRule.insert("edge_die_type", QJsonValue(str));
        lJsonGdbnRule.insert(SOFT_BIN_KEY, QJsonValue(lGdbnRule.mSoftBin));
        lJsonGdbnRule.insert(HARD_BIN_KEY, QJsonValue(lGdbnRule.mHardBin));
        lJsonGdbnRule.insert(COLOR_KEY, ColorRGBToJsonArray(lGdbnRule.mFailBinColor));
        gdbnRules.insert(i, lJsonGdbnRule);
        ++i;
    }
    gdbn.insert(RULE_KEY, QJsonValue(gdbnRules));
    waferSort.insert("gdbn",  QJsonValue(gdbn));

    /////////////////// Clustering ///////////////////////////////////
    QJsonObject clustering;
    clustering.insert(ENABLED_KEY, QJsonValue(options.mClusteringPotato));
    clustering.insert(HARD_BIN_KEY, QJsonValue(options.mClusteringPotatoHBin));
    clustering.insert(SOFT_BIN_KEY, QJsonValue(options.mClusteringPotatoSBin));
    clustering.insert(COLOR_KEY, ColorRGBToJsonArray(options.mClusteringPotatoColor));

    QJsonArray clusteringRules;
    CClusterPotatoRule lClusteringRule;
    i=0;
    for(QList <CClusterPotatoRule>::iterator lClusteringIter = options.mClusterPotatoRules.begin();
        lClusteringIter != options.mClusterPotatoRules.end();
        ++lClusteringIter)
    {
        QJsonObject lJsonClusteringRule;
        lClusteringRule = *lClusteringIter;
        lJsonClusteringRule.insert(NAME_KEY, QJsonValue(lClusteringRule.mRuleName));

        QJsonArray  lBinsArray  = StringBinsListToJsonArray(lClusteringRule.mBadBinIdentifyList->GetRangeList());
        lJsonClusteringRule.insert("cluster_bins", QJsonValue(lBinsArray));

        lBinsArray  = StringBinsListToJsonArray(lClusteringRule.mBadBinInkingList->GetRangeList());
        lJsonClusteringRule.insert(BAD_BINS_KEY, QJsonValue(lBinsArray));

        switch(lClusteringRule.mWaferSource)
        {
            case GEX_PAT_WAFMAP_SRC_SOFTBIN:
                str = STDF_SOFT_BIN_KEY;
                break;
            case GEX_PAT_WAFMAP_SRC_HARDBIN:
                str = STDF_HARD_BIN_KEY;
                break;
            case GEX_PAT_WAFMAP_SRC_PROBER:
                str = EXTERNAL_KEY;
                break;
            default:
                str = STDF_SOFT_BIN_KEY;
                break;
        }
        lJsonClusteringRule.insert("map_source", QJsonValue(str));

        lJsonClusteringRule.insert("cluster_threshold", QJsonValue(fabs(lClusteringRule.mClusterSize)));
        if (lClusteringRule.mClusterSize < 0)
            str = "gross_die_percent";
        else
            str = "dies";
        lJsonClusteringRule.insert("threshold_type", QJsonValue(str));

        lJsonClusteringRule.insert("outline_width", QJsonValue(lClusteringRule.mOutlineWidth));
        lJsonClusteringRule.insert(ENABLED_KEY, QJsonValue(lClusteringRule.mIsEnabled));
        lJsonClusteringRule.insert("ignore_scratch_lines", QJsonValue(lClusteringRule.mIgnoreScratchLines));
        lJsonClusteringRule.insert("ignore_scratch_rows", QJsonValue(lClusteringRule.mIgnoreScratchRows));
        lJsonClusteringRule.insert("ignore_diag_bad_dies", QJsonValue(lClusteringRule.mIgnoreDiagonalBadDies));
        lJsonClusteringRule.insert("mask_name", QJsonValue(lClusteringRule.mMaskName));


        QJsonObject lightOutline;
        lightOutline.insert(ENABLED_KEY, QJsonValue(lClusteringRule.mIsLightOutlineEnabled));
        QJsonArray adjWeight;
        for (int compt=0; compt < lClusteringRule.mAdjWeightLst.size(); ++compt)
        {
            adjWeight.insert(compt, lClusteringRule.mAdjWeightLst[compt]);
        }
        lightOutline.insert("adj_weight", QJsonValue(adjWeight));

        QJsonArray diagWeight;
        for (int compt=0; compt < lClusteringRule.mDiagWeightLst.size(); ++compt)
        {
            diagWeight.insert(compt, lClusteringRule.mDiagWeightLst[compt]);
        }
        lightOutline.insert("diag_weight", QJsonValue(diagWeight));
        lightOutline.insert("fail_weight", QJsonValue(lClusteringRule.mFailWeight));
        lightOutline.insert("outline_matrix_size", QJsonValue(lClusteringRule.mOutlineMatrixSize));

        switch(lClusteringRule.mEdgeDieWeighting)
        {
            case 0:
                str = "ignored";
                break;
            case 1:
                str = "good";
                break;
            case 2:
                str = "bad";
                break;
            case 3:
                str = "scaled";
                break;
            default:
                str = "ignored";
                break;
        }
        lightOutline.insert("edge_die_rule_type", QJsonValue(str));
        lightOutline.insert("edge_die_factor", QJsonValue(lClusteringRule.mEdgeDieWeightingScale));
        lightOutline.insert("edge_die_type", QJsonValue(lClusteringRule.mEdgeDieType));
        lJsonClusteringRule.insert("light_outline", QJsonValue(lightOutline));
        lJsonClusteringRule.insert(SOFT_BIN_KEY, QJsonValue(lClusteringRule.mSoftBin));
        lJsonClusteringRule.insert(HARD_BIN_KEY, QJsonValue(lClusteringRule.mHardBin));
        lJsonClusteringRule.insert(COLOR_KEY, ColorRGBToJsonArray(lClusteringRule.mFailBinColor));
        clusteringRules.insert(i, lJsonClusteringRule);
        ++i;
    }
    clustering.insert(RULE_KEY, QJsonValue(clusteringRules));
    waferSort.insert("clustering",  QJsonValue(clustering));

    /////////////////// Reticle ///////////////////////////////////
    QJsonObject reticle;
    reticle.insert(ENABLED_KEY, QJsonValue(options.GetReticleEnabled()));
    reticle.insert(HARD_BIN_KEY, QJsonValue(options.GetReticleHardBin()));
    reticle.insert(SOFT_BIN_KEY, QJsonValue(options.GetReticleSoftBin()));
    reticle.insert(COLOR_KEY, ColorRGBToJsonArray(options.GetReticleColor()));

    if (options.GetReticleSizeSource() == PATOptionReticle::RETICLE_SIZE_FILE)
        reticle.insert(RETICLE_SIZE_SOURCE, QJsonValue(QString("from_file")));
    else if (options.GetReticleSizeSource() == PATOptionReticle::RETICLE_SIZE_FIXED)
        reticle.insert(RETICLE_SIZE_SOURCE, QJsonValue(QString("fixed")));

    reticle.insert(RETICLE_SIZE_X, QJsonValue(options.GetReticleSizeX()));
    reticle.insert(RETICLE_SIZE_Y, QJsonValue(options.GetReticleSizeY()));

    QJsonArray          lJsonReticleRules;
    PATOptionReticle    lReticleRule;
    int                 lRuleIndex = 0;

    for(QList<PATOptionReticle>::iterator itReticle = options.GetReticleRules().begin();
        itReticle != options.GetReticleRules().end();
        ++itReticle)
    {
        QJsonObject lJsonReticleRule;
        lReticleRule = *itReticle;

        QJsonArray lBinsArray   = StringBinsListToJsonArray(lReticleRule.GetBadBinsReticleList().GetRangeList());
        lJsonReticleRule.insert("bad_bins_list", QJsonValue(lBinsArray));

        switch(lReticleRule.GetReticle_WafermapSource())
        {
            case 0:
                str = STDF_SOFT_BIN_KEY;
                break;
            case 1:
                str = STDF_HARD_BIN_KEY;
                break;
            case 2:
                str = EXTERNAL_KEY;
                break;
            default:
                str = STDF_SOFT_BIN_KEY;
                break;
        }

        lJsonReticleRule.insert(ENABLED_KEY, QJsonValue(lReticleRule.IsReticleEnabled()));
        lJsonReticleRule.insert(HARD_BIN_KEY, QJsonValue(lReticleRule.GetReticleHBin()));
        lJsonReticleRule.insert(SOFT_BIN_KEY, QJsonValue(lReticleRule.GetReticleSBin()));
        lJsonReticleRule.insert(COLOR_KEY, ColorRGBToJsonArray(lReticleRule.GetReticleColor()));

        lJsonReticleRule.insert("map_source", QJsonValue(str));
        lJsonReticleRule.insert("bad_bins_percent", QJsonValue(lReticleRule.GetReticleYieldThreshold()));
        lJsonReticleRule.insert("mask_name", QJsonValue(lReticleRule.GetReticleMaskName()));
        // reticle corners
        lJsonReticleRule.insert("name", QJsonValue(lReticleRule.GetRuleName()));
        lJsonReticleRule.insert("rule", QJsonValue(lReticleRule.GetRuleString()));
        lJsonReticleRule.insert("x_ink", QJsonValue(lReticleRule.GetXInk()));
        lJsonReticleRule.insert("y_ink", QJsonValue(lReticleRule.GetYInk()));
        lJsonReticleRule.insert("diag_ink", QJsonValue(lReticleRule.GetDiagInk()));
        lJsonReticleRule.insert("x_off_diag", QJsonValue(lReticleRule.GetXOffDiag()));
        lJsonReticleRule.insert("y_off_diag", QJsonValue(lReticleRule.GetYOffDiag()));
        lJsonReticleRule.insert("ignore_diag_bad", QJsonValue(lReticleRule.IgnoreDiagonalBadDies()));
        QJsonArray lActivatedCorner;
        lActivatedCorner.insert(0, QJsonValue(lReticleRule.IsActivatedCorner(PATOptionReticle::CORNER_TOP_LEFT)));
        lActivatedCorner.insert(1, QJsonValue(lReticleRule.IsActivatedCorner(PATOptionReticle::TOP)));
        lActivatedCorner.insert(2, QJsonValue(lReticleRule.IsActivatedCorner(PATOptionReticle::CORNER_TOP_RIGHT)));
        lActivatedCorner.insert(3, QJsonValue(lReticleRule.IsActivatedCorner(PATOptionReticle::RIGHT)));
        lActivatedCorner.insert(4, QJsonValue(lReticleRule.IsActivatedCorner(PATOptionReticle::CORNER_BOTTOM_RIGHT)));
        lActivatedCorner.insert(5, QJsonValue(lReticleRule.IsActivatedCorner(PATOptionReticle::BOTTOM)));
        lActivatedCorner.insert(6, QJsonValue(lReticleRule.IsActivatedCorner(PATOptionReticle::CORNER_BOTTOM_LEFT)));
        lActivatedCorner.insert(7, QJsonValue(lReticleRule.IsActivatedCorner(PATOptionReticle::LEFT)));
        lJsonReticleRule.insert("activated_corner", QJsonArray(lActivatedCorner));
        lJsonReticleRule.insert("defectivity_check_bad_Bin_level", lReticleRule.GetFieldThreshold());

        QJsonArray  lFieldCoordinates;
        QJsonObject lFieldCoord;

        for (int lCoordIdx = 0; lCoordIdx < lReticleRule.GetFieldCoordinates().count(); ++lCoordIdx)
        {
            lFieldCoord.insert("X", QJsonValue(lReticleRule.GetFieldCoordinates().at(lCoordIdx).first));
            lFieldCoord.insert("Y", QJsonValue(lReticleRule.GetFieldCoordinates().at(lCoordIdx).second));
            lFieldCoordinates.insert(lCoordIdx, lFieldCoord);
        }

        lJsonReticleRule.insert("field_coordinates", lFieldCoordinates);

        QString lFieldSelected;
        switch (lReticleRule.GetFieldSelection())
        {
            case PATOptionReticle::ALL_RETICLE_FIELDS:
                lFieldSelected = "all_reticle_fields";
                break;
            case PATOptionReticle::LIST_RETICLE_FIELDS:
                lFieldSelected = "list_reticle_fields";
                break;
            default:
                lFieldSelected = "edge_reticle_fields";
        }

        lJsonReticleRule.insert("field_selected", lFieldSelected);
        lJsonReticleRules.insert(lRuleIndex, lJsonReticleRule);
        ++lRuleIndex;
    }

    reticle.insert(RULE_KEY, QJsonValue(lJsonReticleRules));
    waferSort.insert("reticle",  QJsonValue(reticle));

    /////////////////// z_pat ///////////////////////////////////
    QJsonObject z_pat;
    z_pat.insert(COLOR_KEY, ColorRGBToJsonArray(options.cZpatColor));

    QJsonObject compareToEtest;
    compareToEtest.insert(ENABLED_KEY, QJsonValue(options.bMergeEtestStdf));
    compareToEtest.insert(HARD_BIN_KEY, QJsonValue(options.iCompositeEtestStdf_HBin));
    compareToEtest.insert(SOFT_BIN_KEY, QJsonValue(options.iCompositeEtestStdf_SBin));
    z_pat.insert("compare_to_etest", QJsonValue(compareToEtest));

    QJsonObject exclusionZones;
    exclusionZones.insert(ENABLED_KEY, QJsonValue((options.GetExclusionZoneEnabled()) ? true: false));
    if (options.bZPAT_SoftBin == true)
        str = STDF_SOFT_BIN_KEY;
    else
        str = STDF_HARD_BIN_KEY;
    exclusionZones.insert("map_source", QJsonValue(str));
    exclusionZones.insert("yield_threshold", QJsonValue(options.lfCompositeExclusionZoneYieldThreshold));
    exclusionZones.insert(HARD_BIN_KEY, QJsonValue(options.iCompositeZone_HBin));
    exclusionZones.insert(SOFT_BIN_KEY, QJsonValue(options.iCompositeZone_SBin));

    QJsonArray lBinsArray  = StringBinsListToJsonArray(options.pBadBinsZPAT_List->GetRangeList());

    exclusionZones.insert(BAD_BINS_KEY, QJsonValue(lBinsArray));

    exclusionZones.insert("apply_gdbn", QJsonValue(options.bZPAT_GDBN_Enabled));
    exclusionZones.insert("apply_reticle", QJsonValue(options.bZPAT_Reticle_Enabled));
    exclusionZones.insert("apply_clustering", QJsonValue(options.bZPAT_Clustering_Enabled));
    z_pat.insert("exclusion_zones", QJsonValue(exclusionZones));
    waferSort.insert("z_pat",  QJsonValue(z_pat));


    /////////////////// mask ///////////////////////////////////
    QJsonArray masks;
    for(int iRuleID=0; iRuleID<options.mMaskRules.count();++iRuleID)
    {
        QJsonObject mask;
        CMask_Rule *ptMaskRule = options.mMaskRules[iRuleID];
        mask.insert(NAME_KEY, QJsonValue(ptMaskRule->mRuleName));
        mask.insert(ENABLED_KEY, QJsonValue(ptMaskRule->mIsEnabled));
        if (ptMaskRule->mWorkingArea == 0)
            str = "outer_ring";
        else
            str = "inner_ring";
        mask.insert("working_area", QJsonValue(str));
        mask.insert("size", QJsonValue(ptMaskRule->mRadius));
        masks.insert(iRuleID, QJsonObject(mask));
    }
    waferSort.insert("mask",  QJsonValue(masks));

    /////////////////// precedence ///////////////////////////////////
    QJsonArray precedence;
    for (int iPrec=0; iPrec<options.strRulePrecedence.size(); ++iPrec)
    {
        precedence.insert(iPrec, QJsonValue(options.strRulePrecedence[iPrec]));
    }
    waferSort.insert("precedence",  QJsonValue(precedence));

    /////////////////// pat yield limit //////////////////////////////
    QJsonObject yieldAlarm;
    yieldAlarm.insert(ENABLED_KEY, QJsonValue(options.GetEnableYALimit()));
    yieldAlarm.insert(OVERALL_PAT_YIELD_ALARM_LIMIT_KEY, QJsonValue(options.GetOveralPatYALimit()));
    waferSort.insert("pat_yield_alarm",  QJsonValue(yieldAlarm));

    return waferSort;
}


QJsonObject PATRecipeIOJson::WriteFinalTest(COptionsPat& options)
{
    QString str;
    QJsonObject finalTest;
    /////////////////// pat_limits ///////////////////////////////////
    QJsonObject patLimits;
    patLimits.insert("buffer_size", QJsonValue(options.mFT_TuningSamples));

    // baseline
    QJsonObject baseline;
    if(options.mFT_BaseLineAlgo == 0)
        str = "standard";
    else
        str = "merged_sites";
    baseline.insert(ALGORITHM_KEY, QJsonValue(str));
    baseline.insert("nb_parts", QJsonValue(options.mFT_BaseLine));
    baseline.insert("min_parts_per_site", QJsonValue(options.mFT_MinSamplesPerSite));
    QJsonObject maxOutliers;
    bool lEnabled(false);
    if (options.mFT_BaseLineMaxOutliers>0) lEnabled = true;
    maxOutliers.insert(ENABLED_KEY, QJsonValue(lEnabled));

    maxOutliers.insert("threshold", QJsonValue(options.mFT_BaseLineMaxOutliers));
    baseline.insert("max_outliers", QJsonValue(maxOutliers));
    patLimits.insert("baseline",  QJsonValue(baseline));

    // tuning
    QJsonObject tuning;
    tuning.insert(ENABLED_KEY, QJsonValue(options.mFT_TuningIsEnabled));
    if(options.mFT_TuningType == 0)
        str = "devices";
    else
        str = "outliers";
    tuning.insert("type", QJsonValue(str));
    tuning.insert("frequency", QJsonValue(options.mFT_Tuning));
    patLimits.insert("tuning",  QJsonValue(tuning));

    finalTest.insert("pat_limits",  QJsonValue(patLimits));

    /////////////////// alarms ///////////////////////////////////
    QJsonArray alarms;
    QJsonObject alarm;
    alarm.insert("type", options.m_FT_AlarmType);
///    lPATRecipe.getProperty(QString("FT_Alarm_Severity_Outliers").toLatin1().constData(),
///                                   QVariant(alarm["severity"]) );
    alarms.insert(0, QJsonValue(alarm));
    finalTest.insert("alarms",  QJsonValue(alarms));

    /////////////////// misc ///////////////////////////////////
    QJsonObject misc;
    misc.insert("runs_per_packet", options.mFT_RunsPerPacket);
    if (options.mFT_Traceability == 0)
        str = "html";
    else
        str = "disabled";
    misc.insert("execution_log", QJsonValue(str));
    finalTest.insert("misc",  QJsonValue(misc));

    return finalTest;
}


QJsonObject PATRecipeIOJson::WriteSettings(COptionsPat& options)
{
    QString str;
    QJsonObject settings;

    /////////////////// recipe versioning ///////////////////////////////////
    QJsonObject recipeVersioning;
    recipeVersioning.insert("major", QJsonValue(options.iRecipeVersion));
    recipeVersioning.insert("minor", QJsonValue(options.iRecipeBuild));
    recipeVersioning.insert("product", QJsonValue(options.strProductName));
    settings.insert("recipe_versioning", QJsonValue(recipeVersioning));

    /////////////////// general ///////////////////////////////////
    QJsonObject general;
    // Write good hard bins
    QJsonArray  lBinArray = StringBinsListToJsonArray(options.pGoodHardBinsList->GetRangeList());
    general.insert("good_hard_bins", QJsonValue(lBinArray));

    // Write good soft bins
    lBinArray   = StringBinsListToJsonArray(options.pGoodSoftBinsList->GetRangeList());
    general.insert("good_soft_bins", QJsonValue(lBinArray));

    switch (options.mOptionsTestKey)
    {
    case GEX_TBPAT_KEY_TESTNUMBER:
        str = "number";
        break;
    case GEX_TBPAT_KEY_TESTNAME:
        str = NAME_KEY;
        break;
    case GEX_TBPAT_KEY_TESTMIX:
        str = "number_name";
        break;
    default:
        str = "number";
        break;
    }
    general.insert("test_key", QJsonValue(str));
    if (options.bScanGoodPartsOnly == true)
        str = "good_parts";
    else
        str = "all_parts";
    general.insert("bin_outliers", QJsonValue(str));
    general.insert("all_sites_merged", QJsonValue(options.GetAllSitesMerged()));
    settings.insert("general", QJsonValue(general));

    /////////////////// static pat ///////////////////////////////////
    QJsonObject staticPat;
    staticPat.insert(ENABLED_KEY, QJsonValue(options.bStaticPAT));
    staticPat.insert(HARD_BIN_KEY, QJsonValue(options.iFailStatic_HBin));
    staticPat.insert(SOFT_BIN_KEY, QJsonValue(options.iFailStatic_SBin));
    staticPat.insert(COLOR_KEY, ColorRGBToJsonArray(options.cStaticFailColor));
    staticPat.insert("ignore_when_iqr_null", QJsonValue(options.bIgnoreIQR0));
    staticPat.insert("ignore_when_categories", QJsonValue(options.bIgnoreHistoricalCategories));
    staticPat.insert("ignore_when_cpk_greater", QJsonValue(options.lfIgnoreHistoricalCpk));
    settings.insert("static_pat", QJsonValue(staticPat));

    /////////////////// dynamic pat ///////////////////////////////////
    QJsonObject dynamicPat;
    dynamicPat.insert(ENABLED_KEY, QJsonValue(options.bDynamicPAT));
    dynamicPat.insert(HARD_BIN_KEY, QJsonValue(options.iFailDynamic_HBin));
    dynamicPat.insert(SOFT_BIN_KEY, QJsonValue(options.iFailDynamic_SBin));
    dynamicPat.insert(COLOR_KEY, ColorRGBToJsonArray(options.cDynamicFailColor));
    QString lDefaultRule = QString(gexDefaultRuleSet[options.GetDefaultDynamicRule()]);
    dynamicPat.insert("default_rule", QJsonValue(lDefaultRule));

    // limits
    QJsonObject limits;
    switch(options.iPatLimitsFromBin)
    {
        case GEX_TPAT_BUILDLIMITS_ALLBINS:
            str = "all";
            break;
        case GEX_TPAT_BUILDLIMITS_GOODSOFTBINS:
            str = "good_softbins";
            break;
        case GEX_TPAT_BUILDLIMITS_LISTSOFTBINS:
            str = "including_softbins";
            break;
        case GEX_TPAT_BUILDLIMITS_EXCEPTSOFTBINS:
            str = "excluding_softbins";
            break;
        case GEX_TPAT_BUILDLIMITS_GOODHARDBINS:
            str = "good_hardbins";
            break;
        case GEX_TPAT_BUILDLIMITS_LISTHARDBINS:
            str = "including_hardbins";
            break;
        case GEX_TPAT_BUILDLIMITS_EXCEPTHARDBINS:
            str = "excluding_hardbins";
            break;
        default:
            str = "all";
            break;
    }
    limits.insert("bin_filter", QJsonValue(str));
    limits.insert("bins", QJsonValue(options.strPatLimitsFromBin));
    limits.insert("within_original_limits", QJsonValue(options.bStickWithinSpecLimits));
    limits.insert("ignore_high_cpk", QJsonValue(options.lfSmart_IgnoreHighCpk));
    limits.insert("ignore_high_cpk_enabled", QJsonValue(options.mSmart_IgnoreHighCpkEnabled));
    limits.insert("minimum_samples", QJsonValue((double)options.iMinimumSamples));
    limits.insert("minimum_outliers_to_fail", QJsonValue((double)options.iMinimumOutliersPerPart));
    limits.insert("stop_on_first_fail", QJsonValue(options.bStopOnFirstFail));
    limits.insert("category_value_count", QJsonValue(options.iCategoryValueCount));
    limits.insert("assume_integer_category", QJsonValue(options.bAssumeIntegerCategory));
    dynamicPat.insert("limits", QJsonValue(limits));
    settings.insert("dynamic_pat", QJsonValue(dynamicPat));

    /////////////////// multi variate pat ///////////////////////////////////
    QJsonObject multiVariatePat;
    multiVariatePat.insert(ENABLED_KEY, QJsonValue(options.GetMVPATEnabled()));
    multiVariatePat.insert(HARD_BIN_KEY, QJsonValue(options.GetMVPATHardBin()));
    multiVariatePat.insert(SOFT_BIN_KEY, QJsonValue(options.GetMVPATSoftBin()));
    multiVariatePat.insert("fail_color", ColorRGBToJsonArray(options.GetMVPATColor()));
    multiVariatePat.insert("auto_group_creation", QJsonValue(options.GetMVPATAutomaticGroupCreation()));
    multiVariatePat.insert("normal_shape_only", QJsonValue(options.GetMVPATNormalShapeOnly()));
    multiVariatePat.insert("correlation_threshold", QJsonValue(options.GetMVPATGroupCorrelation()));
    QJsonObject lOutlierDistance;
    lOutlierDistance.insert("near", options.GetMVPATDistance(PAT::Near));
    lOutlierDistance.insert("medium", options.GetMVPATDistance(PAT::Medium));
    lOutlierDistance.insert("far", options.GetMVPATDistance(PAT::Far));
    multiVariatePat.insert("outlier_distance", QJsonValue(lOutlierDistance));
    multiVariatePat.insert("ignore_ppat_bins", QJsonValue(options.GetMVPATIgnorePPATBins()));
    settings.insert("multi_variate_pat", QJsonValue(multiVariatePat));

    /////////////////// wafer ///////////////////////////////////
    QJsonObject geographicPat;
    geographicPat.insert("ignore_ppat_bins", QJsonValue(options.mGPAT_IgnorePPatBins));

    settings.insert("geographic_pat", QJsonValue(geographicPat));

    /////////////////// report ///////////////////////////////////
    QJsonObject report;
    report.insert("stats", QJsonValue(options.bReport_Stats));
    report.insert("histo", QJsonValue(options.bReport_Histo));
    // PAT-71
    report.insert("histo_no_outliers", QJsonValue(options.bReport_Histo_NoOutliers));
    report.insert("wafermap", QJsonValue(options.bReport_Wafermap));
    if (options.iReport_WafermapType == 0)
        str = SOFT_BIN_KEY;
    else
        str = HARD_BIN_KEY;
    report.insert("wafermap_type", QJsonValue(str));
    report.insert("pareto", QJsonValue(options.bReport_Pareto));
    report.insert("binning", QJsonValue(options.bReport_Binning));
    report.insert("spat_limits", QJsonValue(options.bReport_SPAT_Limits));
    report.insert("dpat_limits_outliers", QJsonValue(options.bReport_DPAT_Limits_Outliers));
    report.insert("dpat_limits_no_outliers", QJsonValue(options.bReport_DPAT_Limits_NoOutliers));

    // Mvpat tests
    report.insert("mvpat_standard_charts", QJsonValue(options.GetMVPATReportStdCharts()));
    QJsonObject lMvPatCorrelationChart;
    lMvPatCorrelationChart.insert(ENABLED_KEY, QJsonValue(options.GetMVPATReportCorrCharts()));
    if (options.GetMVPATReportPCAProjection() == true)
        lMvPatCorrelationChart.insert("dataset", QJsonValue(QString("pca")));
    else
        lMvPatCorrelationChart.insert("dataset", QJsonValue(QString("tests")));

    if (options.GetMVPATReportPairs() == PAT::AllPairs)
        lMvPatCorrelationChart.insert("charts", QJsonValue(QString("all_pairs")));
    else
        lMvPatCorrelationChart.insert("charts", QJsonValue(QString("consecutive_pairs")));
    lMvPatCorrelationChart.insert("max_charts", QJsonValue(options.GetMVPATReportMaxCharts()));

    report.insert("mvpat_correlation_charts", QJsonValue(lMvPatCorrelationChart));

    settings.insert("report", QJsonValue(report));

    return settings;
}

/******************************************************************************!
 * \fn patdefinitioncompare
 ******************************************************************************/
bool patdefinitioncompare(const CPatDefinition* a, const CPatDefinition* b)
{
    if (a->m_lTestNumber == b->m_lTestNumber)
    {
        if (a->m_strTestName == b->m_strTestName)
        {
            return a->mPinIndex < b->mPinIndex;
        }
        else
        {
            return a->m_strTestName < b->m_strTestName;
        }
    }
    else
    {
        return a->m_lTestNumber < b->m_lTestNumber;
    }
}

/******************************************************************************!
 * \fn WriteUnivariates
 ******************************************************************************/
bool PATRecipeIOJson::WriteUnivariates(QHash<QString, CPatDefinition *> & tests, QJsonObject& univariates)
{
    QJsonArray univariatesArray;
    int i = 0;

    QList<CPatDefinition*> lTestList = tests.values();
    QList<CPatDefinition*>::const_iterator lTestIter;
    std::sort(lTestList.begin(), lTestList.end(), patdefinitioncompare);
    for (lTestIter = lTestList.begin();
         lTestIter != lTestList.end(); ++lTestIter, ++i)
    {
        CPatDefinition* patDef = *lTestIter;
        QJsonObject test;
        test.insert("static_bin", QJsonValue((double)patDef->m_lFailStaticBin));
        test.insert("dynamic_bin", QJsonValue((double)patDef->m_lFailDynamicBin));
        test.insert(TEST_NUMBER_KEY, QJsonValue((double)patDef->m_lTestNumber));
        test.insert("test_type", QJsonValue(QString(patDef->GetTestTypeLegacy())));
        test.insert(PIN_INDEX_KEY, QJsonValue((double)patDef->mPinIndex));
        test.insert(TEST_NAME_KEY, QJsonValue(patDef->m_strTestName));
        QString lLowLimit = QString::number(patDef->m_lfLowLimit) + QString(" ") + patDef->m_strUnits;
        test.insert("low_limit", QJsonValue(lLowLimit));
        QString lHighLimit = QString::number(patDef->m_lfHighLimit) + QString(" ") + patDef->m_strUnits;
        test.insert("high_limit", QJsonValue(lHighLimit));

        /// dpat_settings
        QJsonObject dpatSettings;
        QString lTmpStr;
        switch (patDef->m_iDistributionShape)
        {
            case PATMAN_LIB_SHAPE_GAUSSIAN :        lTmpStr = "Gaussian";   break;
            case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT :   lTmpStr = "Gaussian_L"; break;
            case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT :  lTmpStr = "Gaussian_R"; break;
            case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT :  lTmpStr = "Log_L";      break;
            case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT : lTmpStr = "Log_R";      break;
            case PATMAN_LIB_SHAPE_BIMODAL :         lTmpStr = "B_Modal";    break;
            case PATMAN_LIB_SHAPE_MULTIMODAL :      lTmpStr = "M_Modal";    break;
            case PATMAN_LIB_SHAPE_CLAMPED_LEFT :    lTmpStr = "Clamp_L";    break;
            case PATMAN_LIB_SHAPE_CLAMPED_RIGHT :   lTmpStr = "Clamp_R";    break;
            case PATMAN_LIB_SHAPE_DOUBLECLAMPED :   lTmpStr = "Clamp_2";    break;
            case PATMAN_LIB_SHAPE_CATEGORY :        lTmpStr = "Categories"; break;
            case PATMAN_LIB_SHAPE_ERROR :           lTmpStr = "Error"; break;
            case PATMAN_LIB_SHAPE_UNKNOWN :         lTmpStr = "Unknown";    break;
            default  :                              lTmpStr = "Unknown";    break;
        }
        dpatSettings.insert("shape", QJsonValue(lTmpStr));

        switch (patDef->m_SamplesToIgnore)
        {
            case GEX_TPAT_IGNOREDATA_NONEID :        lTmpStr = "none";   break;
            case GEX_TPAT_IGNOREDATA_NEGATIVEID :    lTmpStr = "negative"; break;
            case GEX_TPAT_IGNOREDATA_POSITIVEID :    lTmpStr = "positive"; break;
            default  :                               lTmpStr = "none";    break;
        }
        dpatSettings.insert("samples_to_ignore", QJsonValue(lTmpStr));

        switch (patDef->m_OutliersToKeep)
        {
            case GEX_TPAT_KEEPTYPE_NONEID :    lTmpStr = "none";   break;
            case GEX_TPAT_KEEPTYPE_LOWID :     lTmpStr = "low"; break;
            case GEX_TPAT_KEEPTYPE_HIGHID :    lTmpStr = "high"; break;
            default  :                         lTmpStr = "none";    break;
        }
        dpatSettings.insert("outliers_to_keep", QJsonValue(lTmpStr));

        switch (patDef->m_iOutlierLimitsSet)
        {
            case GEX_TPAT_LIMITSSET_NEAR :    lTmpStr = "near";   break;
            case GEX_TPAT_LIMITSSET_MEDIUM :  lTmpStr = "medium"; break;
            case GEX_TPAT_LIMITSSET_FAR :     lTmpStr = "far"; break;
            default  :                        lTmpStr = "near";    break;
        }
        dpatSettings.insert("outlier_limits_set", QJsonValue(lTmpStr));

        lTmpStr = QString(gexRuleSetItemsKeywordsForJson[patDef->mOutlierRule]);
        dpatSettings.insert("outlier_rule", QJsonValue(lTmpStr));

        dpatSettings.insert("head_factor", QJsonValue(patDef->m_lfOutlierNFactor));
        dpatSettings.insert("tail_factor", QJsonValue(patDef->m_lfOutlierTFactor));
        dpatSettings.insert("dpat_notes", QJsonValue(patDef->mDPATNote));

        lTmpStr = QString(gexTailMngtRuleSetItemsKeywords[patDef->mTailMngtRuleType]);
        dpatSettings.insert("tail_management", QJsonValue(lTmpStr));

        test.insert("dpat_settings", QJsonValue(dpatSettings));

        /// spat_settings
        QJsonObject spatSettings;
        switch (patDef->m_SPATRuleType)
        {
            case GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA :   lTmpStr = "aec";        break;
            case GEX_TPAT_SPAT_ALGO_SIGMA :         lTmpStr = "relax";      break;
            case GEX_TPAT_SPAT_ALGO_NEWLIMITS :     lTmpStr = "new";        break;
            case GEX_TPAT_SPAT_ALGO_RANGE :         lTmpStr = "range";      break;
            case GEX_TPAT_SPAT_ALGO_IGNORE :        lTmpStr = "disabled";   break;
            default :                               lTmpStr = "aec";        break;
        }
        spatSettings.insert("outlier_limits_set", QJsonValue(lTmpStr));
        spatSettings.insert("head_factor", QJsonValue(patDef->m_lfSpatOutlierNFactor));
        spatSettings.insert("tail_factor", QJsonValue(patDef->m_lfSpatOutlierTFactor));
        spatSettings.insert("spat_notes", QJsonValue(patDef->mSPATNote));
        test.insert("spat_settings", QJsonValue(spatSettings));

        /// nnr_settings
        QJsonObject nnrSettings;
        nnrSettings.insert(ENABLED_KEY,
                           QJsonValue((patDef->m_iNrrRule==GEX_TPAT_NNR_ENABLED) ? true : false));
        test.insert("nnr_settings", QJsonValue(nnrSettings));

        /// historical_stats
        QJsonObject hsitoricalStats;
        hsitoricalStats.insert("cpk", QJsonValue((isinf(patDef->m_lHistoricalCPK))?GS_INFINITE_VALUE_DOUBLE:patDef->m_lHistoricalCPK));
        hsitoricalStats.insert("robust_mean", QJsonValue((isinf(patDef->m_lfMedian))?GS_INFINITE_VALUE_DOUBLE:patDef->m_lfMedian));
        hsitoricalStats.insert("robust_sigma", QJsonValue((isinf(patDef->m_lfRobustSigma))?GS_INFINITE_VALUE_DOUBLE:patDef->m_lfRobustSigma));
        hsitoricalStats.insert("mean", QJsonValue((isinf(patDef->m_lfMean))?GS_INFINITE_VALUE_DOUBLE:patDef->m_lfMean));
        hsitoricalStats.insert("sigma", QJsonValue((isinf(patDef->m_lfSigma))?GS_INFINITE_VALUE_DOUBLE:patDef->m_lfSigma));
        hsitoricalStats.insert("range", QJsonValue((isinf(patDef->m_lfRange))?GS_INFINITE_VALUE_DOUBLE:patDef->m_lfRange));
        hsitoricalStats.insert("ll_scale", QJsonValue(patDef->m_llm_scal));
        hsitoricalStats.insert("hl_scale", QJsonValue(patDef->m_hlm_scal));
        test.insert("historical_stats", QJsonValue(hsitoricalStats));

        univariatesArray.insert(i, QJsonValue(test));
    }
    univariates.insert("univariate_rules", univariatesArray);
    return true;
}


bool PATRecipeIOJson::WriteMultivariates(QList<PATMultiVariateRule>& tests, QJsonObject& multivariates)
{
    QJsonArray multivariatesArray;

    for (int i=0; i<tests.size(); ++i)
    {
        PATMultiVariateRule testDef = tests[i];
        QJsonObject test;
        test.insert(ENABLED_KEY, QJsonValue(testDef.GetEnabled()));
        test.insert(NAME_KEY, QJsonValue(testDef.GetName()));
        test.insert(HARD_BIN_KEY, QJsonValue(testDef.GetBin()));
        test.insert(SOFT_BIN_KEY, QJsonValue(testDef.GetBin()));
        test.insert("custom_distance", QJsonValue(testDef.GetCustomDistance()));
        test.insert("principal_components", QJsonValue(testDef.GetPrincipalComponents()));
        test.insert("outlier_distance_mode", QJsonValue(testDef.GetOutlierDistanceModeString().toLower()));
        test.insert("type", QJsonValue(testDef.GetTypeString().toLower()));

        const QList<PATMultiVariateRule::MVTestData> mvTestData = testDef.GetMVTestData();
        QJsonArray mvTestDataArray;
        for (int j=0; j<mvTestData.size(); ++j)
        {
            QJsonObject mvTestDataObject;
            PATMultiVariateRule::MVTestData data = mvTestData[j];
            mvTestDataObject.insert(TEST_NAME_KEY, QJsonValue(data.GetTestName()));
            mvTestDataObject.insert(TEST_NUMBER_KEY, QJsonValue(data.GetTestNumber()));
            mvTestDataObject.insert(PIN_INDEX_KEY, QJsonValue(data.GetPinIdx()));
            mvTestDataArray.insert(j, QJsonValue(mvTestDataObject));
        }
        test.insert("test_data", QJsonValue(mvTestDataArray));
        multivariatesArray.insert(i, QJsonValue(test));
    }
    multivariates.insert("multivariate_rules", multivariatesArray);
    return true;
}

QString PATRecipeIOJson::JsonArrayBinsListToString(QJsonArray bins, const QString& defaultBins)
{
    QString lBins;

    if (bins.isEmpty())
        lBins = defaultBins;
    else
    {
        for(int lIdx = 0; lIdx < bins.size(); ++lIdx)
        {
            if (lBins.isEmpty() == false)
                lBins += ";";

            lBins += bins[lIdx].toString();
        }
    }

    return lBins;
}

QJsonArray PATRecipeIOJson::StringBinsListToJsonArray(const QString& bins)
{
    QStringList lBinsList = bins.split(QRegExp("[,;]"), QString::SkipEmptyParts);

    return QJsonArray::fromStringList(lBinsList);
}

QColor PATRecipeIOJson::JsonArrayRGBToColor(QJsonArray colors, const QColor &defaultColor)
{
    QColor lColor = defaultColor;

    if(colors.size() == 3)
    {
        if (colors[0].isDouble())
            lColor.setRed(static_cast<int>(colors[0].toDouble()));

        if (colors[1].isDouble())
            lColor.setGreen(static_cast<int>(colors[1].toDouble()));

        if (colors[2].isDouble())
            lColor.setBlue(static_cast<int>(colors[2].toDouble()));
    }

    return lColor;
}

QJsonArray PATRecipeIOJson::ColorRGBToJsonArray(const QColor &color)
{
    QJsonArray lcolor;

    lcolor.insert(0, QJsonValue(color.red()));
    lcolor.insert(1, QJsonValue(color.green()));
    lcolor.insert(2, QJsonValue(color.blue()));

    return lcolor;
}

bool PATRecipeIOJson::Read(QTextStream & lRecipeStream, PATRecipe &lPATRecipe)
{
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("PATRecipeIO::Read from device %1")
          .arg(lRecipeStream.device()->metaObject()->className()).toLatin1().data());

    QString recipe = lRecipeStream.readAll();

    QJsonParseError error;
    QJsonDocument d = QJsonDocument::fromJson(recipe.toUtf8(), &error);
    if (error.error != error.NoError)
    {
        mErrorMessage = QString("The recipe file %1 is not JSON format: %2")
                .arg(mRecipeName)
                .arg(error.errorString());
        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
        return false;
    }
    QJsonObject recipeObject = d.object();

    QJsonValue lVersion = recipeObject[QString("version")];
    if(lVersion.isUndefined() || lVersion.isNull())
    {
        lPATRecipe.GetOptions().SetRecipeVersion(GS::Gex::PAT::sRecipeVersion);
    }
    else
    {
        // GCORE-3666 - HT
        // Version can be with 1 or 2 decimals. (ie: 2.00 or 2.0)
        QRegExp lRegExpFull("^\\d.\\d{1,2}$");
        float   lJSONFormat;

        if (lVersion.isDouble())
        {
            lJSONFormat = (float) lVersion.toDouble();
        }
        // GCORE XXXX - HTH
        // With older GEX version, some recipes were created with an empty version. When happening, we should consider
        // the JSON version is the lower supported. (2.00)
        else if (lVersion.isString() && lVersion.toString().isEmpty())
        {
            lVersion    = QJsonValue(QString("2.00"));
            lJSONFormat = MIN_JSON_FORMAT;
        }
        else if (lRegExpFull.exactMatch(lVersion.toString()))
        {
            lJSONFormat = lVersion.toString().toFloat();
        }
        else
        {
            mErrorMessage = QString("Unable to detect the Recipe version: %1").arg(lVersion.toString());
            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
            return false;
        }


        if (lJSONFormat < MIN_JSON_FORMAT || lJSONFormat > MAX_JSON_FORMAT)
        {
            mErrorMessage = QString("Unsupported Recipe version detected: %1. Supported Recipe version are from %2 to %3").arg(lJSONFormat).arg(MIN_JSON_FORMAT).arg(MAX_JSON_FORMAT);
            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
            return false;
        }

        lPATRecipe.GetOptions().SetRecipeVersion(lVersion.toString());
    }

    QJsonValue lTestingStage = recipeObject["testing_stage"];
    if(!lVersion.isUndefined() && !lVersion.isNull())
    {
        if(lTestingStage.toString() == QString("wafer_sort"))
        {
            lPATRecipe.GetOptions().SetRecipeType(PAT::RecipeWaferSort);
        }
        else if(lTestingStage.toString() == QString("final_test"))
        {
            lPATRecipe.GetOptions().SetRecipeType(PAT::RecipeFinalTest);
        }
    }

    QJsonObject outlierOptions = recipeObject["outlier_options"].toObject();
    // Section: <Outlier_Options>
    if (readOptions(outlierOptions, lPATRecipe.GetOptions()) == false)
        return false;

    QJsonArray univariateRules = recipeObject["univariate_rules"].toArray();
    // Section: <univariate_rules>
    if (readUnivariateRules(univariateRules, lPATRecipe.GetOptions(), lPATRecipe.GetUniVariateRules()) == false)
        return false;

    QJsonArray multivariateRules = recipeObject["multivariate_rules"].toArray();
    // Section: <multivariate_rules>
    if (readMultivariateRules(multivariateRules, lPATRecipe.GetMultiVariateRules()) == false)
        return false;

    // Build outlier failure type for PAT soft and hard bins
    BuildPATBinsFailType(lPATRecipe.GetOptions(), lPATRecipe.GetUniVariateRules(), lPATRecipe.GetMultiVariateRules());

    return true;
}

bool PATRecipeIOJson::readOptions(QJsonObject outlierOptions, COptionsPat &lPATRecipeOptions)
{
    // Ensure we purge all rules (in case we read twice the recipe file!)
    lPATRecipeOptions.GetNNRRules().clear();
    lPATRecipeOptions.SetNNREnabled(false);
    lPATRecipeOptions.mIDDQ_Delta_Rules.clear();
    lPATRecipeOptions.mIsIDDQ_Delta_enabled = false;
    lPATRecipeOptions.mGDBNRules.clear();
    lPATRecipeOptions.mIsGDBNEnabled = false;
    lPATRecipeOptions.mClusterPotatoRules.clear();
    lPATRecipeOptions.mClusteringPotato = false;
    qDeleteAll(lPATRecipeOptions.mMaskRules);
    lPATRecipeOptions.mMaskRules.clear();
    lPATRecipeOptions.GetReticleRules().clear();

    // Rule precedence: force to default.
    lPATRecipeOptions.resetRulePrecedence();

    if (!outlierOptions.isEmpty())
    {
        //////////////////////////   Smart rules   ///////////////////////////////
        QJsonObject smartRules = outlierOptions["smart_rules"].toObject();
        if (readSmartRules(smartRules, lPATRecipeOptions) == false)
            return false;

        //////////////////////////   wafer sort   ///////////////////////////////
        QJsonObject wafer = outlierOptions["wafer_sort"].toObject();
        if (readWaferSort(wafer, lPATRecipeOptions) == false)
            return false;

        //////////////////////////////// final test ////////////////////////
        QJsonObject finalTest = outlierOptions["final_test"].toObject();
        if (readFinalTest(finalTest, lPATRecipeOptions) == false)
            return false;
        ///////////////////////////////////////////////////////////////////////////////
        /// settings
        ///////////////////////////////////////////////////////////////////////////////
        QJsonObject settings = outlierOptions["settings"].toObject();
        if(readSettings(settings, lPATRecipeOptions) == false)
            return false;
        // Success
        return true;
    }
    return false;
}

bool PATRecipeIOJson::readSmartRules(QJsonObject aSmartRules, COptionsPat &aPATRecipeOptions)
{
    if (!aSmartRules.isEmpty())
    {
        QJsonObject shapes = aSmartRules["shapes"].toObject();
        if (!shapes.isEmpty())
        {
            ////////////// Gaussian //////////////////
            QJsonObject shape = shapes["gaussian"].toObject();
            if (!shape.isEmpty())
            {
                if (shape[APPLY_PAT_KEY].isBool())
                    aPATRecipeOptions.bPAT_Gaussian = shape[APPLY_PAT_KEY].toBool();
                else
                {
                    mErrorMessage = QString("The value of gaussian->apply_pat is not a boolean");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                QString algoString = shape[FORMULA_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY) algoIndex = 0;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY) algoIndex = 1;
                else if (algoString == IQR_KEY) algoIndex = 2;
                else if (algoString == CUSTOMER_PAT_LIB_KEY) algoIndex = 3;
                else
                {
                    mErrorMessage = QString("Value %1 for key formula is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.iAlgo_Gaussian =  algoIndex;

                // Get outlier limits
                aPATRecipeOptions.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                aPATRecipeOptions.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                QJsonObject limitFactors = shape["limit_factors"].toObject();
                if (!limitFactors.isEmpty())
                {
                    QJsonArray oulierLimitsArray;
                    // Get outlier limits: NEAR outliers
                    if (limitFactors["near"].isArray())
                    {
                        oulierLimitsArray = limitFactors["near"].toArray();
                        if (oulierLimitsArray.size() == 2)
                        {
                            aPATRecipeOptions.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                               (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_6_SIGMA);
                            aPATRecipeOptions.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                               (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_6_SIGMA);
                        }
                    }
                    else
                    {
                        mErrorMessage = QString("The value of gaussian->near is not an array");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }


                    // Get outlier limits: MEDIUM outliers
                    if (!limitFactors["medium"].isArray())
                    {
                        mErrorMessage = QString("The value of gaussian->medium is not an array");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                    oulierLimitsArray = limitFactors["medium"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                               (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_9_SIGMA);
                        aPATRecipeOptions.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                               (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_9_SIGMA);
                    }

                    // Get outlier limits: FAR outliers
                    if (!limitFactors["far"].isArray())
                    {
                        mErrorMessage = QString("The value of gaussian->far is not an array");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                    oulierLimitsArray = limitFactors["far"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_12_SIGMA);
                        aPATRecipeOptions.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_12_SIGMA);
                    }
                }
            }

            ////////////// GaussianTailedPAT //////////////////
            shape = shapes["gaussian_tailed"].toObject();
            if (!shape.isEmpty())
            {
                if (!shape[APPLY_PAT_KEY].isBool())
                {
                    mErrorMessage = QString("The value of gaussian_tailed->apply_pat is not a boolean");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.bPAT_GaussianTailed = shape[APPLY_PAT_KEY].toBool();
                QString algoString = shape[FORMULA_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY) algoIndex = 0;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY) algoIndex = 1;
                else if (algoString == IQR_KEY) algoIndex = 2;
                else if (algoString == CUSTOMER_PAT_LIB_KEY) algoIndex = 3;
                else
                {
                    mErrorMessage = QString("Value %1 for key formula is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.iAlgo_GaussianTailed =  algoIndex;

                // Get outlier limits
                aPATRecipeOptions.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_7_SIGMA;
                aPATRecipeOptions.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_10_SIGMA;
                aPATRecipeOptions.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                aPATRecipeOptions.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_14_SIGMA;
                QJsonObject limitFactors = shape["limit_factors"].toObject();
                if (!limitFactors.isEmpty())
                {
                    // Get outlier limits: NEAR outlierss
                    if(!limitFactors["near"].isArray())
                    {
                        mErrorMessage = QString("The value of gaussian_tailed->near is not an array");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                    QJsonArray oulierLimitsArray = limitFactors["near"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_6_SIGMA);
                        aPATRecipeOptions.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_7_SIGMA);
                    }

                    // Get outlier limits: MEDIUM outliers
                    oulierLimitsArray = limitFactors["medium"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_9_SIGMA);
                        aPATRecipeOptions.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_10_SIGMA);
                    }

                    // Get outlier limits: FAR outliers
                    oulierLimitsArray = limitFactors["far"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_12_SIGMA);
                        aPATRecipeOptions.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_14_SIGMA);
                    }
                }
            }

            ////////////// gaussian_double_tailed //////////////////
            shape = shapes["gaussian_double_tailed"].toObject();
            if (!shape.isEmpty())
            {
                if (!shape[APPLY_PAT_KEY].isBool())
                {
                    mErrorMessage = QString("The value of gaussian_double_tailed->apply_pat is not a boolean");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.bPAT_GaussianDoubleTailed = shape[APPLY_PAT_KEY].toBool();
                QString algoString = shape[FORMULA_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY) algoIndex = 0;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY) algoIndex = 1;
                else if (algoString == IQR_KEY) algoIndex = 2;
                else if (algoString == CUSTOMER_PAT_LIB_KEY) algoIndex = 3;
                else
                {
                    mErrorMessage = QString("Value %1 for key formula is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.iAlgo_GaussianDoubleTailed =  algoIndex;

                aPATRecipeOptions.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]
                        = GEX_TPAT_7_SIGMA;
                aPATRecipeOptions.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]
                        = GEX_TPAT_7_SIGMA;
                aPATRecipeOptions.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]
                        = GEX_TPAT_10_SIGMA;
                aPATRecipeOptions.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]
                        = GEX_TPAT_10_SIGMA;
                aPATRecipeOptions.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]
                        = GEX_TPAT_14_SIGMA;
                aPATRecipeOptions.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]
                        = GEX_TPAT_14_SIGMA;
                QJsonObject limitFactors = shape["limit_factors"].toObject();
                if (!limitFactors.isEmpty())
                {
                    if(!limitFactors["near"].isArray())
                    {
                        mErrorMessage = QString("The value of gaussian_double_tailed->near is not an array");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                    QJsonArray oulierLimitsArray = limitFactors["near"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                               (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_7_SIGMA);
                        aPATRecipeOptions.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                               (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_7_SIGMA);
                    }

                    // Get outlier limits: MEDIUM outliers
                    oulierLimitsArray = limitFactors["medium"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_10_SIGMA);
                        aPATRecipeOptions.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_10_SIGMA);
                    }

                    // Get outlier limits: FAR outliers
                    oulierLimitsArray = limitFactors["far"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_14_SIGMA);
                        aPATRecipeOptions.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_14_SIGMA);
                    }
                }
            }

            ////////////// log_normal //////////////////
            shape = shapes["log_normal"].toObject();
            if (!shape.isEmpty())
            {
                if (!shape[APPLY_PAT_KEY].isBool())
                {
                    mErrorMessage = QString("The value of log_normal->apply_pat is not a boolean");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.bPAT_LogNormal = shape[APPLY_PAT_KEY].toBool();
                QString algoString = shape[FORMULA_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY) algoIndex = 0;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY) algoIndex = 1;
                else if (algoString == IQR_KEY) algoIndex = 2;
                else if (algoString == CUSTOMER_PAT_LIB_KEY) algoIndex = 3;
                else
                {
                    mErrorMessage = QString("Value %1 for key formula is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.iAlgo_LogNormal =  algoIndex;

                aPATRecipeOptions.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;
                aPATRecipeOptions.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_7_SIGMA;
                aPATRecipeOptions.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_4_SIGMA;
                aPATRecipeOptions.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_10_SIGMA;
                aPATRecipeOptions.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_3_SIGMA;
                aPATRecipeOptions.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_14_SIGMA;
                QJsonObject limitFactors = shape["limit_factors"].toObject();
                if (!limitFactors.isEmpty())
                {
                    // Get outlier limits: Near outliers
                    QJsonArray oulierLimitsArray = limitFactors["near"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_3_SIGMA);
                        aPATRecipeOptions.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_7_SIGMA);
                    }

                    // Get outlier limits: MEDIUM outliers
                    oulierLimitsArray = limitFactors["medium"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_4_SIGMA);
                        aPATRecipeOptions.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_10_SIGMA);
                    }

                    // Get outlier limits: FAR outliers
                    oulierLimitsArray = limitFactors["far"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_3_SIGMA);
                        aPATRecipeOptions.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_14_SIGMA);
                    }
                }
            }

            ////////////// multi_modal //////////////////
            shape = shapes["multi_modal"].toObject();
            if (!shape.isEmpty())
            {
                if (!shape[APPLY_PAT_KEY].isBool())
                {
                    mErrorMessage = QString("The value of multi_modal->apply_pat is not a boolean");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.bPAT_MultiModal = shape[APPLY_PAT_KEY].toBool();
                QString algoString = shape[FORMULA_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY) algoIndex = 0;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY) algoIndex = 1;
                else if (algoString == IQR_KEY) algoIndex = 2;
                else if (algoString == CUSTOMER_PAT_LIB_KEY) algoIndex = 3;
                else {
                    mErrorMessage = QString("Value %1 for key formula is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.iAlgo_MultiModal = algoIndex;

                aPATRecipeOptions.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                aPATRecipeOptions.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                QJsonObject limitFactors = shape["limit_factors"].toObject();
                if (!limitFactors.isEmpty())
                {
                    // Get outlier limits: Near outliers
                    QJsonArray oulierLimitsArray = limitFactors["near"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_6_SIGMA);
                        aPATRecipeOptions.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_6_SIGMA);
                    }

                    // Get outlier limits: MEDIUM outliers
                    oulierLimitsArray = limitFactors["medium"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_9_SIGMA);
                        aPATRecipeOptions.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_9_SIGMA);
                    }

                    // Get outlier limits: FAR outliers
                    oulierLimitsArray = limitFactors["far"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_12_SIGMA);
                        aPATRecipeOptions.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_12_SIGMA);
                    }
                }
            }

            ////////////// clamped //////////////////
            shape = shapes["clamped"].toObject();
            if (!shape.isEmpty())
            {
                if (!shape[APPLY_PAT_KEY].isBool())
                {
                    mErrorMessage = QString("The value of clamped->apply_pat is not a boolean");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.bPAT_Clamped = shape[APPLY_PAT_KEY].toBool();
                QString algoString = shape[FORMULA_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY) algoIndex = 0;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY) algoIndex = 1;
                else if (algoString == IQR_KEY) algoIndex = 2;
                else if (algoString == CUSTOMER_PAT_LIB_KEY) algoIndex = 3;
                else
                {
                    mErrorMessage = QString("Value %1 for key formula is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.iAlgo_Clamped = algoIndex;

                aPATRecipeOptions.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                aPATRecipeOptions.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                QJsonObject limitFactors = shape["limit_factors"].toObject();
                if (!limitFactors.isEmpty())
                {
                    // Get outlier limits: Near outliers
                    QJsonArray oulierLimitsArray = limitFactors["near"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_6_SIGMA);
                        aPATRecipeOptions.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_6_SIGMA);
                    }

                    // Get outlier limits: MEDIUM outliers
                    oulierLimitsArray = limitFactors["medium"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_9_SIGMA);
                        aPATRecipeOptions.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_9_SIGMA);
                    }

                    // Get outlier limits: FAR outliers
                    oulierLimitsArray = limitFactors["far"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_12_SIGMA);
                        aPATRecipeOptions.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_12_SIGMA);
                    }
                }
            }

            ////////////// double_clamped //////////////////
            shape = shapes["double_clamped"].toObject();
            if (!shape.isEmpty())
            {
                if (!shape[APPLY_PAT_KEY].isBool())
                {
                    mErrorMessage = QString("The value of double_clamped->apply_pat is not a boolean");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.bPAT_DoubleClamped = shape[APPLY_PAT_KEY].toBool();
                QString algoString = shape[FORMULA_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY) algoIndex = 0;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY) algoIndex = 1;
                else if (algoString == IQR_KEY) algoIndex = 2;
                else if (algoString == CUSTOMER_PAT_LIB_KEY) algoIndex = 3;
                else
                {
                    mErrorMessage = QString("Value %1 for key formula is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.iAlgo_DoubleClamped = algoIndex;

                aPATRecipeOptions.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;
                aPATRecipeOptions.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;
                aPATRecipeOptions.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_4_SIGMA;
                aPATRecipeOptions.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_4_SIGMA;
                aPATRecipeOptions.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_6_SIGMA;
                QJsonObject limitFactors = shape["limit_factors"].toObject();
                if (!limitFactors.isEmpty())
                {
                    // Get outlier limits: Near outliers
                    QJsonArray oulierLimitsArray = limitFactors["near"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_3_SIGMA);
                        aPATRecipeOptions.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_3_SIGMA);
                    }

                    // Get outlier limits: MEDIUM outliers
                    oulierLimitsArray = limitFactors["medium"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_4_SIGMA);
                        aPATRecipeOptions.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_4_SIGMA);
                    }

                    // Get outlier limits: FAR outliers
                    oulierLimitsArray = limitFactors["far"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_6_SIGMA);
                        aPATRecipeOptions.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_6_SIGMA);
                    }
                }
            }

            ////////////// category //////////////////
            shape = shapes["category"].toObject();
            if (!shape.isEmpty())
            {
                if (!shape[APPLY_PAT_KEY].isBool())
                {
                    mErrorMessage = QString("The value of category->apply_pat is not a boolean");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.bPAT_Category = shape[APPLY_PAT_KEY].toBool();
                QString algoString = shape[FORMULA_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY) algoIndex = 0;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY) algoIndex = 1;
                else if (algoString == IQR_KEY) algoIndex = 2;
                else if (algoString == CUSTOMER_PAT_LIB_KEY) algoIndex = 3;
                else
                {
                    mErrorMessage = QString("Value %1 for key formula is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.iAlgo_Category = algoIndex;

                aPATRecipeOptions.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                aPATRecipeOptions.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                QJsonObject limitFactors = shape["limit_factors"].toObject();
                if (!limitFactors.isEmpty())
                {
                    // Get outlier limits: Near outliers
                    QJsonArray oulierLimitsArray = limitFactors["near"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_6_SIGMA);
                        aPATRecipeOptions.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_6_SIGMA);
                    }

                    // Get outlier limits: MEDIUM outliers
                    oulierLimitsArray = limitFactors["medium"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_9_SIGMA);
                        aPATRecipeOptions.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_9_SIGMA);
                    }

                    // Get outlier limits: FAR outliers
                    oulierLimitsArray = limitFactors["far"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_12_SIGMA);
                        aPATRecipeOptions.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_12_SIGMA);
                    }
                }
            }

            ////////////// unknown //////////////////
            shape = shapes["unknown"].toObject();
            if (!shape.isEmpty())
            {
                if (!shape[APPLY_PAT_KEY].isBool())
                {
                    mErrorMessage = QString("The value of unknown->apply_pat is not a boolean");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.bPAT_Unknown = shape[APPLY_PAT_KEY].toBool();
                QString algoString = shape[FORMULA_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY) algoIndex = 0;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY) algoIndex = 1;
                else if (algoString == IQR_KEY) algoIndex = 2;
                else if (algoString == CUSTOMER_PAT_LIB_KEY) algoIndex = 3;
                else
                {
                    mErrorMessage = QString("Value %1 for key formula is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                aPATRecipeOptions.iAlgo_Unknown = algoIndex;

                aPATRecipeOptions.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
                aPATRecipeOptions.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]= GEX_TPAT_9_SIGMA;
                aPATRecipeOptions.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                aPATRecipeOptions.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR]= GEX_TPAT_12_SIGMA;
                QJsonObject limitFactors = shape["limit_factors"].toObject();
                if (!limitFactors.isEmpty())
                {
                    // Get outlier limits: Near outliers
                    QJsonArray oulierLimitsArray = limitFactors["near"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_6_SIGMA);
                        aPATRecipeOptions.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_6_SIGMA);
                    }

                    // Get outlier limits: MEDIUM outliers
                    oulierLimitsArray = limitFactors["medium"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_9_SIGMA);
                        aPATRecipeOptions.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]=
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_9_SIGMA);
                    }

                    // Get outlier limits: FAR outliers
                    oulierLimitsArray = limitFactors["far"].toArray();
                    if (oulierLimitsArray.size() == 2)
                    {
                        aPATRecipeOptions.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[0].isDouble()?fabs(oulierLimitsArray[0].toDouble()):GEX_TPAT_12_SIGMA);
                        aPATRecipeOptions.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] =
                              (oulierLimitsArray[1].isDouble()?fabs(oulierLimitsArray[1].toDouble()):GEX_TPAT_12_SIGMA);
                    }
                }
            }
        }


        // inserted in 2.13 but hidden option so if unable to read
        // so don't check the version at the moment just check if readable or not
        if (aSmartRules.contains("min_confidence_threshold"))
        {
            int lThreshold = aSmartRules["min_confidence_threshold"].toInt();
            // has to be between 1 and 5, if not read/written correctly set to default
            if ((lThreshold <= 0) || (lThreshold > 5))
            {
                mErrorMessage = QString("Wrong value for minimum confidence threshold (%1), "
                                        "it has to be between 1 and 5").
                                        arg(aSmartRules["min_confidence_threshold"].toInt());
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            aPATRecipeOptions.mMinConfThreshold = lThreshold;
        }
        else
        {
            // by default we use a value of 2
            int lMinThreshold =2;

            // Check if the confident level has been modify trhough an env variable
            QString lStrEnv =  QString(qgetenv("QX_PAT_MIN_CONFIDENCE_LEVEL")).simplified();
            if(lStrEnv.isEmpty() == false)
            {
                int lResConversion =  lStrEnv.toInt();
                // the number must be between 1 and 5
                if(lResConversion >= 1 && lResConversion <=5)
                {
                    lMinThreshold = lResConversion;
                }
                else
                {
                    mErrorMessage = QString("Wrong value for minimum confidence threshold (%1) coming from  "
                                            "the environment variable [QX_PAT_MIN_CONFIDENCE_LEVEL]. "
                                            "It has to be between 1 and 5").
                                            arg(lStrEnv);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                }
            }

            aPATRecipeOptions.mMinConfThreshold = lMinThreshold;
        }
    }
    return true;

}

bool PATRecipeIOJson::readWaferSort(QJsonObject wafer, COptionsPat &lPATRecipeOptions)
{
    if (!wafer.isEmpty())
    {
        lPATRecipeOptions.SetRecipeType(PAT::RecipeWaferSort);

        // NNR: Nearest Neighbor Residual (NNR), identify parametric outliers within geographic clusters
        QJsonObject nnr = wafer["nnr"].toObject();
        if (!nnr.isEmpty())
        {
            if (nnr[ENABLED_KEY].isBool())
                lPATRecipeOptions.SetNNREnabled(nnr[ENABLED_KEY].toBool());
            else
                lPATRecipeOptions.SetNNREnabled(false);

            // NRR Cluster bin#
            if (!nnr[HARD_BIN_KEY].isDouble())
            {
                mErrorMessage = QString("The value of nnr->hard_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.SetNNRHardBin(static_cast<int>(nnr[HARD_BIN_KEY].toDouble()));

            if (!nnr[SOFT_BIN_KEY].isDouble())
            {
                mErrorMessage = QString("The value of nnr->soft_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.SetNNRSoftBin(static_cast<int>(nnr[SOFT_BIN_KEY].toDouble()));

            // NNR Bin RGB color
            lPATRecipeOptions.SetNNRColor(JsonArrayRGBToColor(nnr[COLOR_KEY].toArray(), QColor(255,202,181)));

            // NNR rules list
            QJsonArray  nnrRules = nnr[RULE_KEY].toArray();
            QString     lRuleName;
            QString     lBaseRuleName;
            int         lBaseIndex = 0;
            for (int i=0; i<nnrRules.size(); ++i)
            {
                // Format: Rulename,
                CNNR_Rule lRule;
                QJsonObject lJsonRule = nnrRules[i].toObject();
                lRuleName       = lJsonRule[NAME_KEY].toString();
                lBaseRuleName   = lRuleName;
                lBaseIndex      = 1;

                // Ensure the rule name is unique
                while(lPATRecipeOptions.FindNNRRuleByName(lRuleName) != -1)
                {
                    lRuleName = lBaseRuleName + "-" + QString::number(lBaseIndex);
                    ++lBaseIndex;
                }
                lRule.SetRuleName(lRuleName);

                if (!lJsonRule["location_averaging"].isDouble())
                {
                    mErrorMessage = QString("The value of nnr->rules->location_averaging is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.SetLA(lJsonRule["location_averaging"].toDouble());	// Test density
                // NNR Algo.
                QString algoString = lJsonRule[ALGORITHM_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY)
                    algoIndex = GEX_TPAT_NNR_ALGO_LOCAL_SIGMA;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY)
                    algoIndex = GEX_TPAT_NNR_ALGO_LOCAL_MEDIAN;
                else if (algoString == IQR_KEY)
                    algoIndex = GEX_TPAT_NNR_ALGO_LOCAL_Q1Q3IQR;
                else
                {
                    mErrorMessage = QString("Value %1 for key algorithm is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.SetAlgorithm(algoIndex);

                if (!lJsonRule["factor"].isDouble())
                {
                    mErrorMessage = QString("The value of nnr->rules->factor is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.SetNFactor(lJsonRule["factor"].toDouble());		// N factor

                if (!lJsonRule["cluster_size"].isDouble())
                {
                    mErrorMessage = QString("The value of nnr->rules->cluster_size is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.SetClusterSize(lJsonRule["cluster_size"].toDouble());
                bool bStatus = false;
                switch(lRule.GetClusterSize())
                {
                    case -1:
                    case 5:
                    case 7:
                    case 9:
                    case 11:
                    case 13:
                    case 15:    bStatus = true;
                                break;

                    default:    bStatus = false;
                                break;
                }
                if(!bStatus)
                {
                    // Invalid line...
                    mErrorMessage = "Outliers configuration file parsing error :\nFile:\n" + mRecipeName;
                    mErrorMessage += " : invalid 'NNR cluster size' (";
                    mErrorMessage += QString::number(lRule.GetClusterSize()) + ").";
                    return false;
                }
                // Rule Enabled
                lRule.SetIsEnabled(lJsonRule[ENABLED_KEY].isBool()?lJsonRule[ENABLED_KEY].toBool():true);
                if (lPATRecipeOptions.GetRecipeVersion().toDouble() >= 2.12)
                {
                    lRule.SetSoftBin(lJsonRule[SOFT_BIN_KEY].toInt());
                    lRule.SetHardBin(lJsonRule[HARD_BIN_KEY].toInt());
                    lRule.SetFailBinColor(JsonArrayRGBToColor(lJsonRule[COLOR_KEY].toArray(), QColor(255,202,181)));
                }
                else
                {
                    lRule.SetSoftBin(lPATRecipeOptions.GetNNRSoftBin());
                    lRule.SetHardBin(lPATRecipeOptions.GetNNRHardBin());
                    lRule.SetFailBinColor(lPATRecipeOptions.GetNNRColor());
                }
                lPATRecipeOptions.GetNNRRules().append(lRule);
            }
        }

        // IDDQ-Delta: IDDQ-Delta outlier checks (compute PostStress-PreStress values & identify outliers)
        QJsonObject iddqDelta = wafer["iddq_delta"].toObject();
        if (!iddqDelta.isEmpty())
        {
            // IDDQ-Delta enabled?
            if (iddqDelta[ENABLED_KEY].isBool())
                lPATRecipeOptions.mIsIDDQ_Delta_enabled = iddqDelta[ENABLED_KEY].toBool();
            else
                lPATRecipeOptions.mIsIDDQ_Delta_enabled = false;

            // IDDQ-Delta bin#
            if (!iddqDelta[HARD_BIN_KEY].isDouble())
            {
                mErrorMessage = QString("The value of wafer->iddqDelta->hard_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.mIDDQ_Delta_HBin = static_cast<int>(iddqDelta[HARD_BIN_KEY].toDouble());

            if (!iddqDelta[SOFT_BIN_KEY].isDouble())
            {
                mErrorMessage = QString("The value of wafer->iddqDelta->soft_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.mIDDQ_Delta_SBin = static_cast<int>(iddqDelta[SOFT_BIN_KEY].toDouble());

            // IDDQ-Delta Bin RGB color
            lPATRecipeOptions.mIDDQ_Delta_Color = JsonArrayRGBToColor(iddqDelta[COLOR_KEY].toArray(), QColor(255,202,181));

            // iddq delta rules list
            QJsonArray iddqDeltaRules = iddqDelta[RULE_KEY].toArray();
            for (int i=0; i<iddqDeltaRules.size(); ++i)
            {
                // Format: Rulename,
                CIDDQ_Delta_Rule lRule;
                QJsonObject lJsonRule = iddqDeltaRules[i].toObject();
                lRule.SetRuleName(lJsonRule[NAME_KEY].toString());
                lRule.SetPreStress(lJsonRule["pre_stress"].toString());
                lRule.SetPostStress(lJsonRule["post_stress"].toString());
                lRule.SetCaseSensitive(lJsonRule["case_sensitive"].toBool());
                QString algoString = lJsonRule[ALGORITHM_KEY].toString();
                int algoIndex;
                if (algoString == MEAN_N_SIGMA_KEY) algoIndex = 0;
                else if (algoString == MEAN_ROBUST_SIGMA_KEY) algoIndex = 1;
                else if (algoString == IQR_KEY) algoIndex = 2;
                else
                {
                    mErrorMessage = QString("Value %1 for key iddq_delta->algorithm is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.SetAlgorithm(algoIndex);
                if (!iddqDelta[SOFT_BIN_KEY].isDouble())
                {
                    mErrorMessage = QString("The value of wafer->iddqDelta->->rules->factor is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.SetNFactor(lJsonRule["factor"].toDouble());
                // Rule Enabled
                lRule.SetIsEnabled(lJsonRule[ENABLED_KEY].isBool()?lJsonRule[ENABLED_KEY].toBool():true);
                if (lPATRecipeOptions.GetRecipeVersion().toDouble() >= 2.12)
                {
                    lRule.SetSoftBin(lJsonRule[SOFT_BIN_KEY].toInt());
                    lRule.SetHardBin(lJsonRule[HARD_BIN_KEY].toInt());
                    lRule.SetFailBinColor(JsonArrayRGBToColor(lJsonRule[COLOR_KEY].toArray(), QColor(255,202,181)));
                }
                else
                {
                    lRule.SetSoftBin(lPATRecipeOptions.mIDDQ_Delta_SBin);
                    lRule.SetHardBin(lPATRecipeOptions.mIDDQ_Delta_HBin);
                    lRule.SetFailBinColor(lPATRecipeOptions.mIDDQ_Delta_Color);
                }
                lPATRecipeOptions.mIDDQ_Delta_Rules.append(lRule);
            }
        }

        ///////////////////////////////////////////////////////////////////////////////
        // GDBN settings
        ///////////////////////////////////////////////////////////////////////////////
        QJsonObject gdbn = wafer["gdbn"].toObject();
        if (!gdbn.isEmpty())
        {
            if (gdbn[ENABLED_KEY].isBool())
                lPATRecipeOptions.mIsGDBNEnabled = gdbn[ENABLED_KEY].toBool();
            else
                lPATRecipeOptions.mIsGDBNEnabled = false;

            // Cluster bin#
            lPATRecipeOptions.mGDBNPatHBin = static_cast<int>(gdbn[HARD_BIN_KEY].toDouble());
            lPATRecipeOptions.mGDBNPatSBin = static_cast<int>(gdbn[SOFT_BIN_KEY].toDouble());

            // RGB color for the Bad cluster failure on a good bin
            lPATRecipeOptions.mGDBNColor = JsonArrayRGBToColor(gdbn[COLOR_KEY].toArray(), QColor(255,202,181));

            // iddq delta rules list
            QJsonArray gdbnRules = gdbn[RULE_KEY].toArray();
            for (int i=0; i<gdbnRules.size(); ++i)
            {
                // Format: Rulename,
                CGDBN_Rule lRule;
                QJsonObject lJsonRule = gdbnRules[i].toObject();
                lRule.mRuleName = lJsonRule[NAME_KEY].toString();

                if (!lJsonRule["yield_threshold"].isDouble())
                {
                    mErrorMessage = QString("The value of wafer->gndb->yield_threshold is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.mYieldThreshold = lJsonRule["yield_threshold"].toDouble();

                if(lRule.mBadBinList != NULL)
                    delete lRule.mBadBinList;

                QString badBins = JsonArrayBinsListToString(lJsonRule[BAD_BINS_KEY].toArray(), "0;2-65535");

                lRule.mBadBinList = new GS::QtLib::Range(badBins);

                QString algoString = lJsonRule["map_source"].toString();
                int algoIndex;
                if (algoString == STDF_SOFT_BIN_KEY) algoIndex = 0;
                else if (algoString == STDF_HARD_BIN_KEY) algoIndex = 1;
                else if (algoString == EXTERNAL_KEY) algoIndex = 2;
                else
                {
                    mErrorMessage = QString("Value %1 for key rules->map_source is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.mWafermapSource =  algoIndex;

                algoString = lJsonRule[ALGORITHM_KEY].toString();
                if (algoString == "squeeze") algoIndex = 0;
                else if (algoString == "weighting") algoIndex = 1;
                else
                {
                    mErrorMessage = QString("Value %1 for key rules->algorithm is incorrect").arg(algoString);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.mAlgorithm =  algoIndex;

                lRule.mFailWaferEdges = lJsonRule["fail_wafer_edge"].toBool();

                if (!lJsonRule["cluster_size"].isDouble())
                {
                    mErrorMessage = QString("The value of wafer->gndb->cluster_size is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.mClusterSize = static_cast<int>(lJsonRule["cluster_size"].toDouble());

                if (!lJsonRule["squeeze_count"].isDouble())
                {
                    mErrorMessage = QString("The value of wafer->gndb->squeeze_count is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.mFailCount = static_cast<int>(lJsonRule["squeeze_count"].toDouble());

                // Adjacent weights for bad dies
                lRule.mAdjWeightLst.clear();
                QJsonArray weight = lJsonRule["adjacent_weight"].toArray();
                for (int i=0; i<weight.size(); ++i)
                {
                    if (!weight[i].isDouble())
                    {
                        mErrorMessage = QString("A value in wafer->gndb->adjacent_weight is not a number");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                    lRule.mAdjWeightLst.append(static_cast<int>(weight[i].toDouble()));
                }
                // Diagonal weight for bad dies
                lRule.mDiagWeightLst.clear();
                weight = lJsonRule["diag_weight"].toArray();
                for (int i=0; i<weight.size(); ++i)
                {
                    if (!weight[i].isDouble())
                    {
                        mErrorMessage = QString("A value in wafer->gndb->diag_weight is not a number");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                    lRule.mDiagWeightLst.append(static_cast<int>(weight[i].toDouble()));
                }

                if (!lJsonRule["fail_weight"].isDouble())
                {
                    mErrorMessage = QString("The value of wafer->gndb->fail_weight is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.mMinimumWeighting = static_cast<int>(lJsonRule["fail_weight"].toDouble());

                QString edgeDieWeight = lJsonRule["edge_die_weight"].toString();
                if (edgeDieWeight == "ignored") algoIndex = 0;
                else if (edgeDieWeight == "good") algoIndex = 1;
                else if (edgeDieWeight == "bad") algoIndex = 2;
                else if (edgeDieWeight == "scaled") algoIndex = 3;
                else
                {
                    mErrorMessage = QString("Value %1 for key rules->edge_die_weight is incorrect").arg(edgeDieWeight);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.mEdgeDieWeighting =  algoIndex;

                if (!lJsonRule["weight_scale"].isDouble())
                {
                    mErrorMessage = QString("The value of wafer->gndb->weight_scale is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lRule.mEdgeDieWeightingScale = lJsonRule["weight_scale"].toDouble();
                lRule.mIsEnabled = lJsonRule[ENABLED_KEY].isBool()?lJsonRule[ENABLED_KEY].toBool():true;

                if (lJsonRule.contains("mask"))
                    lRule.mMaskName = lJsonRule["mask"].toString();
                else
                    lRule.mMaskName = lJsonRule["mask_name"].toString();

                QString edgeDieType = lJsonRule["edge_die_type"].toString();
                if (edgeDieType == "all")
                    algoIndex = GEX_TPAT_EDGE_DIE_BOTH;
                else if (edgeDieType == "adjacent")
                    algoIndex = GEX_TPAT_EDGE_DIE_ADJACENT;
                else if (edgeDieType == "corner")
                    algoIndex = GEX_TPAT_EDGE_DIE_CORNER;
                else
                {
                    mErrorMessage = QString("Value %1 for key rules->edge_die_type is incorrect").arg(edgeDieType);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }

                lRule.mEdgeDieType =  algoIndex;
                if (lPATRecipeOptions.GetRecipeVersion().toDouble() >= 2.12)
                {
                    lRule.mSoftBin = lJsonRule[SOFT_BIN_KEY].toInt();
                    lRule.mHardBin = lJsonRule[HARD_BIN_KEY].toInt();
                    lRule.mFailBinColor = JsonArrayRGBToColor(lJsonRule[COLOR_KEY].toArray(), QColor(255,202,181));
                }
                else
                {
                    lRule.mSoftBin = lPATRecipeOptions.mGDBNPatSBin;
                    lRule.mHardBin = lPATRecipeOptions.mGDBNPatHBin;
                    lRule.mFailBinColor = lPATRecipeOptions.mGDBNColor;
                }
                lPATRecipeOptions.mGDBNRules.append(lRule);
            }
        }

       //////////////////////////////////////////////////////////////////////
       /// CLUSTERING POTATO Options
       /////////////////////////////////////////////////////////////////////
       QJsonObject clustering = wafer["clustering"].toObject();
       if (!clustering.isEmpty())
       {
           if (clustering[ENABLED_KEY].isBool())
               lPATRecipeOptions.mClusteringPotato = clustering[ENABLED_KEY].toBool();
           else
               lPATRecipeOptions.mClusteringPotato = false;

          // Clustering Potato bin#
          if (!clustering[HARD_BIN_KEY].isDouble())
          {
              mErrorMessage = QString("The value of wafer->clustering->hard_bin is not a number");
              GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
              return false;
          }
          lPATRecipeOptions.mClusteringPotatoHBin = static_cast<int>(clustering[HARD_BIN_KEY].toDouble());

          if (!clustering[SOFT_BIN_KEY].isDouble())
          {
              mErrorMessage = QString("The value of wafer->clustering->soft_bin is not a number");
              GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
              return false;
          }
          lPATRecipeOptions.mClusteringPotatoSBin = static_cast<int>(clustering[SOFT_BIN_KEY].toDouble());

          // RGB color for the Clustering Potato good bins rejected
          lPATRecipeOptions.mClusteringPotatoColor = JsonArrayRGBToColor(clustering[COLOR_KEY].toArray(), QColor(255,0,255));

          // Clustering rule
          QJsonArray clusteringRules = clustering[RULE_KEY].toArray();
          for (int i=0; i<clusteringRules.size(); ++i)
          {
              CClusterPotatoRule lRule;
              QJsonObject lJsonRule = clusteringRules[i].toObject();
              lRule.mRuleName = lJsonRule[NAME_KEY].toString();

              // Binlist to Identify a Cluster
              if(lRule.mBadBinIdentifyList != NULL)
              {
                  delete lRule.mBadBinIdentifyList;
                  lRule.mBadBinIdentifyList = NULL;
              }
              QString badBins = JsonArrayBinsListToString(lJsonRule["cluster_bins"].toArray(), "0;2-65535");
              lRule.mBadBinIdentifyList = new GS::QtLib::Range(badBins);

              // Binlist Inking a Cluster
              if(lRule.mBadBinInkingList != NULL)
                  delete lRule.mBadBinInkingList;

              badBins = JsonArrayBinsListToString(lJsonRule[BAD_BINS_KEY].toArray(), "0;2-65535");
              lRule.mBadBinInkingList = new GS::QtLib::Range(badBins);

              // Wafer source: stdf_soft_bin (0) stdf_hard_bin (1), external (2)
              QString algoString = lJsonRule["map_source"].toString();
              int algoIndex;
              if (algoString == STDF_SOFT_BIN_KEY) algoIndex = 0;
              else if (algoString == STDF_HARD_BIN_KEY) algoIndex = 1;
              else if (algoString == EXTERNAL_KEY) algoIndex = 2;
              else
              {
                  mErrorMessage =QString("Value %1 for key clustering->rules->map_source is incorrect").arg(algoString);
                  GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                  return false;
              }
              lRule.mWaferSource = algoIndex;

              // Cluster size (Note: negative number is '%' of wafer die count)
              if (lJsonRule["threshold_type"].toString() == "gross_die_percent")
              {
                  if (!lJsonRule["cluster_threshold"].isDouble())
                  {
                      mErrorMessage = QString("The value of wafer->clustering->cluster_threshold is not a number");
                      GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                      return false;
                  }
                  lRule.mClusterSize = 0 - fabs(lJsonRule["cluster_threshold"].toDouble());
              }
              else
              {
                  if (!lJsonRule["cluster_threshold"].isDouble())
                  {
                      mErrorMessage = QString("The value of wafer->clustering->cluster_threshold is not a number");
                      GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                      return false;
                  }
                  lRule.mClusterSize = fabs(lJsonRule["cluster_threshold"].toDouble());
              }

              // Outline width
              if (!lJsonRule["outline_width"].isDouble())
              {
                  mErrorMessage = QString("The value of wafer->clustering->outline_width is not a number");
                  GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                  return false;
              }
              lRule.mOutlineWidth = static_cast<int>(lJsonRule["outline_width"].toDouble());

              // Rule Enabled
              lRule.mIsEnabled = lJsonRule[ENABLED_KEY].isBool()?lJsonRule[ENABLED_KEY].toBool():true;
              // Ignore one-die scratch lines?
              lRule.mIgnoreScratchLines = lJsonRule["ignore_scratch_lines"].toBool();
              // Ignore one-die scratch rows?
              lRule.mIgnoreScratchRows = lJsonRule["ignore_scratch_rows"].toBool();
              // Ignore diagonal bad dies?
              lRule.mIgnoreDiagonalBadDies = lJsonRule["ignore_diag_bad_dies"].toBool();

              if (lJsonRule.contains("mask"))
                  lRule.mMaskName = lJsonRule["mask"].toString();
              else
                  lRule.mMaskName = lJsonRule["mask_name"].toString();

              QJsonObject lightOutline = lJsonRule["light_outline"].toObject();
              if (!lightOutline.isEmpty())
              {
                   // Light outline ?
                   lRule.mIsLightOutlineEnabled = lightOutline[ENABLED_KEY].toBool();

                   // Adjacent weights for bad dies
                   lRule.mAdjWeightLst.clear();
                   QJsonArray weight = lightOutline["adj_weight"].toArray();
                   for (int i=0; i<weight.size(); ++i)
                   {
                       if (!weight[i].isDouble())
                       {
                           mErrorMessage = QString("The value of wafer->clustering->adj_weight->weight["+
                                                   QString::number(i)+"] is not a number");
                           GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                           return false;
                       }
                       lRule.mAdjWeightLst.append(static_cast<int>(weight[i].toDouble()));
                   }

                   // Diagonal weight for bad dies
                   lRule.mDiagWeightLst.clear();
                   weight = lightOutline["diag_weight"].toArray();
                   for (int i=0; i<weight.size(); ++i)
                   {
                       if (!weight[i].isDouble())
                       {
                           mErrorMessage = QString("The value of wafer->clustering->diag_weight->weight["+
                                                   QString::number(i)+"] is not a number");
                           GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                           return false;
                       }
                       lRule.mDiagWeightLst.append(static_cast<int>(weight[i].toDouble()));
                   }

                   // Fail weight threshold of bad dies.
                   if (!lightOutline["fail_weight"].isDouble())
                   {
                       mErrorMessage = QString("The value of wafer->clustering->fail_weight is not a number");
                       GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                       return false;
                   }
                   lRule.mFailWeight = static_cast<int>(lightOutline["fail_weight"].toDouble());

                   // Outline matrix size
                   if (!lightOutline["outline_matrix_size"].isDouble())
                   {
                       mErrorMessage = QString("The value of wafer->clustering->outline_matrix_size is not a number");
                       GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                       return false;
                   }
                   lRule.mOutlineMatrixSize = static_cast<int>(lightOutline["outline_matrix_size"].toDouble());

                   // Edge-die rule type
                   QString edgeDieWeight = lightOutline["edge_die_rule_type"].toString();
                   if (edgeDieWeight == "ignored") algoIndex = 0;
                   else if (edgeDieWeight == "good") algoIndex = 1;
                   else if (edgeDieWeight == "bad") algoIndex = 2;
                   else if (edgeDieWeight == "scaled") algoIndex = 3;
                   else
                   {
                       mErrorMessage =
                        QString("Value %1 for key light_outline->edge_die_rule_type is incorrect").arg(edgeDieWeight);
                       GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                       return false;
                   }
                    lRule.mEdgeDieWeighting =  algoIndex;

                    // Edge-die factor
                    if (!lightOutline["edge_die_factor"].isDouble())
                    {
                        mErrorMessage = QString("The value of wafer->clustering->edge_die_factor is not a number");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                    lRule.mEdgeDieWeightingScale = lightOutline["edge_die_factor"].toDouble();

                    // Edge-die type
                    if (!lightOutline["edge_die_type"].isDouble())
                    {
                        mErrorMessage = QString("The value of wafer->clustering->edge_die_type is not a number");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                    lRule.mEdgeDieType = static_cast<int>(lightOutline["edge_die_type"].toDouble());

                }
                if (lPATRecipeOptions.GetRecipeVersion().toDouble() >= 2.12)
                {
                    lRule.mSoftBin = lJsonRule[SOFT_BIN_KEY].toInt();
                    lRule.mHardBin = lJsonRule[HARD_BIN_KEY].toInt();
                    lRule.mFailBinColor = JsonArrayRGBToColor(lJsonRule[COLOR_KEY].toArray(), QColor(255,0,255));
                }
                else
                {
                    lRule.mSoftBin = lPATRecipeOptions.mClusteringPotatoSBin;
                    lRule.mHardBin = lPATRecipeOptions.mClusteringPotatoHBin;
                    lRule.mFailBinColor = lPATRecipeOptions.mClusteringPotatoColor;
                }

                lPATRecipeOptions.mClusterPotatoRules.append(lRule);
            }
        }

        /////////////////////////////////////////////////////////////////////
        /// Reticle fields
        /////////////////////////////////////////////////////////////////////
        QJsonObject reticle = wafer["reticle"].toObject();
        if (!reticle.isEmpty())
        {
            if (reticle[ENABLED_KEY].isBool())
                lPATRecipeOptions.SetReticleEnabled(reticle[ENABLED_KEY].toBool());
            else
                lPATRecipeOptions.SetReticleEnabled(false);

            // Reticle error bin#
            if (!reticle[HARD_BIN_KEY].isDouble())
            {
                mErrorMessage = QString("The value of wafer->reticle->hard_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            lPATRecipeOptions.SetReticleHardBin(static_cast<int>(reticle[HARD_BIN_KEY].toDouble()));

            if (!reticle[SOFT_BIN_KEY].isDouble())
            {
                mErrorMessage = QString("The value of wafer->reticle->soft_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.SetReticleSoftBin(static_cast<int>(reticle[SOFT_BIN_KEY].toDouble()));

            // RGB color for the Reticle error on a good bin
            lPATRecipeOptions.SetReticleColor(JsonArrayRGBToColor(reticle[COLOR_KEY].toArray(), QColor(255,0,255)));

            // Get Reticle Size X and Y
            if (!reticle[RETICLE_SIZE_X].isDouble())
            {
                mErrorMessage = QString("The value of wafer->reticle->x_size is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            lPATRecipeOptions.SetReticleSizeX(reticle[RETICLE_SIZE_X].toInt());

            if (!reticle[RETICLE_SIZE_Y].isDouble())
            {
                mErrorMessage = QString("The value of wafer->reticle->y_size is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            lPATRecipeOptions.SetReticleSizeY(reticle[RETICLE_SIZE_Y].toInt());

            // Reticle Size Source (Fixed or from File)
            QString lReticleSizeSource = reticle[RETICLE_SIZE_SOURCE].toString();

            if (lReticleSizeSource == "from_file")
                lPATRecipeOptions.SetReticleSizeSource(PATOptionReticle::RETICLE_SIZE_FILE);
            else if (lReticleSizeSource == "fixed" || lReticleSizeSource.isEmpty())
                lPATRecipeOptions.SetReticleSizeSource(PATOptionReticle::RETICLE_SIZE_FIXED);
            else
            {
                mErrorMessage = QString("The value of wafer->reticle->size_source is invalid");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            if (lPATRecipeOptions.GetRecipeVersion().toDouble() >= 2.11)
            {
                if (readReticleMultiRule(reticle, lPATRecipeOptions) == false)
                    return false;
            }
            else
            {
                if (readReticleSingleRule(reticle, lPATRecipeOptions) == false)
                    return false;
            }
        }
        /////////////////////////////////////////////////////////////////////
        /// ZPAT
        /////////////////////////////////////////////////////////////////////
        QJsonObject zpat = wafer["z_pat"].toObject();
        if (!zpat.isEmpty())
        {
            // RGB color for the Z-PAT bins
            lPATRecipeOptions.cZpatColor = JsonArrayRGBToColor(reticle[COLOR_KEY].toArray(), QColor(255,0,255));

            /// Compare to E-Test
            QJsonObject compare_to_etest = zpat["compare_to_etest"].toObject();
            if(!compare_to_etest.isEmpty())
            {
                // Enable/Disable Overload of STDF good bins with identified rejects in E-test
                lPATRecipeOptions.bMergeEtestStdf = compare_to_etest[ENABLED_KEY].isBool() ?
                                                        compare_to_etest[ENABLED_KEY].toBool() : false;

                // Bin associated with Good STDF bins that are rejects in E-Test maps.
                if (!compare_to_etest[HARD_BIN_KEY].isDouble())
                {
                    mErrorMessage = QString("The value of wafer->zpat->compare_to_etest->hard_bin is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.iCompositeEtestStdf_HBin
                        = static_cast<int>(compare_to_etest[HARD_BIN_KEY].toDouble());

                if (!compare_to_etest[SOFT_BIN_KEY].isDouble())
                {
                    mErrorMessage = QString("The value of wafer->zpat->compare_to_etest->soft_bin is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.iCompositeEtestStdf_SBin
                        = static_cast<int>(compare_to_etest[SOFT_BIN_KEY].toDouble());
            }

            /// Exclusion Zones
            QJsonObject exclusionZones = zpat["exclusion_zones"].toObject();
            if(!exclusionZones.isEmpty())
            {
                if (exclusionZones[ENABLED_KEY].isBool())
                    lPATRecipeOptions.SetExclusionZoneEnabled(exclusionZones[ENABLED_KEY].toBool());
                else
                    lPATRecipeOptions.SetExclusionZoneEnabled(false);

                // Use Softbin?
                if (exclusionZones["map_source"].toString() == STDF_HARD_BIN_KEY)
                    lPATRecipeOptions.bZPAT_SoftBin = false;
                else
                    lPATRecipeOptions.bZPAT_SoftBin = true;

                // Yield threshold to enable this rule
                if (!exclusionZones["yield_threshold"].isDouble())
                {
                    mErrorMessage=QString("The value of wafer->zpat->exclusion_zones->yield_threshold is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }

                lPATRecipeOptions.lfCompositeExclusionZoneYieldThreshold =
                        exclusionZones["yield_threshold"].toDouble();

                // Composite exclusion zone error bin#
                if (!exclusionZones[HARD_BIN_KEY].isDouble())
                {
                    mErrorMessage = QString("The value of wafer->zpat->exclusion_zones->hard_bin is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.iCompositeZone_HBin = static_cast<int>(exclusionZones[HARD_BIN_KEY].toDouble());

                if (!exclusionZones[SOFT_BIN_KEY].isDouble())
                {
                    mErrorMessage = QString("The value of wafer->zpat->exclusion_zones->soft_bin is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.iCompositeZone_SBin = static_cast<int>(exclusionZones[SOFT_BIN_KEY].toDouble());

                // ZPAT Bad Bin list
                if(lPATRecipeOptions.pBadBinsZPAT_List != NULL)
                    delete lPATRecipeOptions.pBadBinsZPAT_List;

                QString badBins = JsonArrayBinsListToString(exclusionZones[BAD_BINS_KEY].toArray(), "0;2-65535");
                lPATRecipeOptions.pBadBinsZPAT_List = new GS::QtLib::Range(badBins);

                // Enable/Disable GDBN rule over Z-PAT composite map.
                lPATRecipeOptions.bZPAT_GDBN_Enabled = exclusionZones["apply_gdbn"].toBool();
                // Enable/Disable Reticle rule over Z-PAT composite map.
                lPATRecipeOptions.bZPAT_Reticle_Enabled = exclusionZones["apply_reticle"].toBool();
                // Enable/Disable Clustering rule over Z-PAT composite map.
                lPATRecipeOptions.bZPAT_Clustering_Enabled =exclusionZones["apply_clustering"].toBool();
            }
        }

        /////////////////////////////////////////////////////////////////////
        /// Mask rule
        /////////////////////////////////////////////////////////////////////
        QJsonArray masks = wafer["mask"].toArray();
        QJsonObject mask;
        for (int i=0; i<masks.size(); ++i)
        {
            mask = masks[i].toObject();
            /// Format: Rulename, Enabled, Areatype, Radius
            CMask_Rule *ptRule = new CMask_Rule();
            // Mask Rule name
            ptRule->mRuleName = mask[NAME_KEY].toString();
            // Enabled?
            ptRule->mIsEnabled = mask[ENABLED_KEY].toBool();
            // Mask area type
            if (mask["working_area"].toString() == "outer_ring")
                ptRule->mWorkingArea = 0;
            else
                ptRule->mWorkingArea = 1;

            // Radius
            if (!mask["size"].isDouble())
            {
                mErrorMessage = QString("The value of wafer->mask->size is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            ptRule->mRadius = static_cast<int>(mask["size"].toDouble());
            lPATRecipeOptions.mMaskRules.append(ptRule);
        }

        /////////////////////////////////////////////////////////////////////
        /// Precedence
        /////////////////////////////////////////////////////////////////////
        // Define in which order 'Wafer' rules are executed
        lPATRecipeOptions.strRulePrecedence.clear();
        QJsonArray precedence = wafer["precedence"].toArray();
        for (int i=0; i<precedence.size(); ++i)
        {
            lPATRecipeOptions.strRulePrecedence.append(precedence[i].toString());
        }

        /////////////////////////////////////////////////////////////////////
        /// Yield Alarm
        /////////////////////////////////////////////////////////////////////
        lPATRecipeOptions.SetEnableYALimit(false);
        QJsonObject yieldAlarm = wafer["pat_yield_alarm"].toObject();
        if (!yieldAlarm.isEmpty())
        {
            if (!yieldAlarm[OVERALL_PAT_YIELD_ALARM_LIMIT_KEY].isDouble())
            {
                mErrorMessage =
                        QString("The value of wafer->pat_yield_alarm->overall_pat_yield_alarm_limit is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            if (yieldAlarm[ENABLED_KEY].isBool())
                lPATRecipeOptions.SetEnableYALimit(yieldAlarm[ENABLED_KEY].toBool());
            else
                lPATRecipeOptions.SetEnableYALimit(false);
            lPATRecipeOptions.SetOveralPatYALimit(yieldAlarm[OVERALL_PAT_YIELD_ALARM_LIMIT_KEY].toDouble());
        }
    }
    return true;
}

bool PATRecipeIOJson::readReticleMultiRule(QJsonObject reticle, COptionsPat &lPATRecipeOptions)
{
    QJsonArray lJsonArray = reticle[RULE_KEY].toArray();

    for (int lIdx = 0; lIdx < lJsonArray.size(); ++lIdx)
    {
        PATOptionReticle    lRule;
        // Set default rule name in case this json was created by converting an old csv file where the rule name
        // didn't exist
        lRule.SetRuleName(QString("Rule-%1").arg(lIdx));
        QJsonObject         lJsonRule = lJsonArray[lIdx].toObject();

        QString badBins = JsonArrayBinsListToString(lJsonRule["bad_bins_list"].toArray(), "0;2-65535");
        lRule.SetBadBinsReticleList(GS::QtLib::Range(badBins));

        if (lJsonRule[ENABLED_KEY].isBool())
            lRule.SetReticleEnabled(lJsonRule[ENABLED_KEY].toBool());
        else
            lRule.SetReticleEnabled(false);

        if (lPATRecipeOptions.GetRecipeVersion().toDouble() >= 2.12)
        {
            // Reticle error bin#
            if (!lJsonRule[HARD_BIN_KEY].isDouble())
            {
                mErrorMessage = QString("The value of wafer->reticle->hard_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lRule.SetReticleHBin(static_cast<int>(lJsonRule[HARD_BIN_KEY].toDouble()));

            if (!lJsonRule[SOFT_BIN_KEY].isDouble())
            {
                mErrorMessage = QString("The value of wafer->reticle->soft_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lRule.SetReticleSBin(static_cast<int>(lJsonRule[SOFT_BIN_KEY].toDouble()));

            // RGB color for the Reticle error on a good bin
            lRule.SetReticleColor(JsonArrayRGBToColor(lJsonRule[COLOR_KEY].toArray(), QColor(255,0,255)));
        }
        else
        {
            lRule.SetReticleHBin(lPATRecipeOptions.GetReticleHardBin());
            lRule.SetReticleSBin(lPATRecipeOptions.GetReticleSoftBin());

            // RGB color for the Reticle error on a good bin
            lRule.SetReticleColor(lPATRecipeOptions.GetReticleColor());
        }

        // EnReticle Bin source: stdf_soft_bin (0) stdf_hard_bin (1), external (2)
        QString algoString = lJsonRule["map_source"].toString();
        int algoIndex;
        if (algoString == STDF_SOFT_BIN_KEY)
            algoIndex = GEX_PAT_WAFMAP_SRC_SOFTBIN;
        else if (algoString == STDF_HARD_BIN_KEY)
            algoIndex = GEX_PAT_WAFMAP_SRC_HARDBIN;
        else if (algoString == EXTERNAL_KEY)
            algoIndex = GEX_PAT_WAFMAP_SRC_PROBER;
        else
        {
            mErrorMessage = QString("Value %1 for key reticle->map_source is incorrect").arg(algoString);
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        lRule.SetReticle_WafermapSource(algoIndex);

        // Reticle Yield threshold under which die location rejected
        if (!lJsonRule["bad_bins_percent"].isDouble())
        {
            mErrorMessage = QString("The value of wafer->reticle->bad_bins_percent is not a number");
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        lRule.SetReticleYieldThreshold(lJsonRule["bad_bins_percent"].toDouble());

        // reticle corners
        lRule.SetRule(lJsonRule["rule"].toString());
        lRule.SetRuleName(lJsonRule["name"].toString());
        lRule.SetXInk(lJsonRule["x_ink"].toInt());
        lRule.SetYInk(lJsonRule["y_ink"].toInt());
        lRule.SetDiagInk(lJsonRule["diag_ink"].toInt());
        lRule.SetXOffDiag(lJsonRule["x_off_diag"].toInt());
        lRule.SetYOffDiag(lJsonRule["y_off_diag"].toInt());
        lRule.SetIgnoreDiagonalBadDies(lJsonRule["ignore_diag_bad"].toBool());
        QJsonArray lActivatedCorner = lJsonRule["activated_corner"].toArray();
        if (lActivatedCorner.size() < 8)
        {
            mErrorMessage = QString("The size of wafer->reticle->activated_corner is not enough. it has to be equal to 8");
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        lRule.SetActivatedCorners(PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[0].toBool() ? PATOptionReticle::CORNER_TOP_LEFT : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[1].toBool() ? PATOptionReticle::TOP : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[2].toBool() ? PATOptionReticle::CORNER_TOP_RIGHT : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[3].toBool() ? PATOptionReticle::RIGHT : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[4].toBool() ? PATOptionReticle::CORNER_BOTTOM_RIGHT : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[5].toBool() ? PATOptionReticle::BOTTOM : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[6].toBool() ? PATOptionReticle::CORNER_BOTTOM_LEFT : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[7].toBool() ? PATOptionReticle::LEFT : PATOptionReticle::NO_CORNER);

        lRule.SetFieldThreshold(lJsonRule["defectivity_check_bad_Bin_level"].toDouble());

        QList<QPair<int, int> > lFieldCoordinates;
        QPair<int,int>          lFieldCoord;
        QJsonArray lJsonFieldCoordinates = lJsonRule["field_coordinates"].toArray();
        QJsonObject lJsonFieldCoord;

        for (int lCoordIdx = 0; lCoordIdx < lJsonFieldCoordinates.count(); ++lCoordIdx)
        {
            lJsonFieldCoord     = lJsonFieldCoordinates.at(lCoordIdx).toObject();
            lFieldCoord.first   = lJsonFieldCoord["X"].toInt();
            lFieldCoord.second  = lJsonFieldCoord["Y"].toInt();

            lFieldCoordinates.append(lFieldCoord);
        }

        lRule.SetFieldCoordinates(lFieldCoordinates);

        QString lFieldSelected = lJsonRule["field_selected"].toString();
        if (lFieldSelected == "all_reticle_fields")
            lRule.SetFieldSelection(PATOptionReticle::ALL_RETICLE_FIELDS);
        else if (lFieldSelected == "list_reticle_fields")
            lRule.SetFieldSelection(PATOptionReticle::LIST_RETICLE_FIELDS);
        else if (lFieldSelected == "edge_reticle_fields")
            lRule.SetFieldSelection(PATOptionReticle::EDGE_RETICLE_FIELDS);

        // Reticle mask name
        QString lMaskName = lJsonRule["mask_name"].toString();

        if (lMaskName.isEmpty() == false)
            lRule.SetReticleMaskName(lMaskName);

        // Add rule
        lPATRecipeOptions.GetReticleRules().append(lRule);
    }

    return true;
}

bool PATRecipeIOJson::readReticleSingleRule(QJsonObject jsonRule, COptionsPat &lPATRecipeOptions)
{
    PATOptionReticle lRule;

    // Set default rule name in case this json was created by converting an old csv file where the rule name
    // didn't exist
    lRule.SetRuleName("Rule-1");

    if (jsonRule[ENABLED_KEY].isBool())
        lRule.SetReticleEnabled(jsonRule[ENABLED_KEY].toBool());
    else
        lRule.SetReticleEnabled(false);

    lRule.SetReticleHBin(lPATRecipeOptions.GetReticleHardBin());
    lRule.SetReticleSBin(lPATRecipeOptions.GetReticleSoftBin());

    // RGB color for the Reticle error on a good bin
    lRule.SetReticleColor(lPATRecipeOptions.GetReticleColor());

    // Reticle Bad Bin list
    QString badBins = JsonArrayBinsListToString(jsonRule["bad_bins_list"].toArray(), "0;2-65535");
    lRule.SetBadBinsReticleList(GS::QtLib::Range(badBins));

    // EnReticle Bin source: stdf_soft_bin (0) stdf_hard_bin (1), external (2)
    QString algoString = jsonRule["map_source"].toString();
    int algoIndex;
    if (algoString == STDF_SOFT_BIN_KEY)
        algoIndex = GEX_PAT_WAFMAP_SRC_SOFTBIN;
    else if (algoString == STDF_HARD_BIN_KEY)
        algoIndex = GEX_PAT_WAFMAP_SRC_HARDBIN;
    else if (algoString == EXTERNAL_KEY)
        algoIndex = GEX_PAT_WAFMAP_SRC_PROBER;
    else
    {
        mErrorMessage = QString("Value %1 for key reticle->map_source is incorrect").arg(algoString);
        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
        return false;
    }
    lRule.SetReticle_WafermapSource(algoIndex);

    // Reticle Yield threshold under which die location rejected
    if (!jsonRule["bad_bins_percent"].isDouble())
    {
        mErrorMessage = QString("The value of wafer->reticle->bad_bins_percent is not a number");
        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
        return false;
    }
    lRule.SetReticleYieldThreshold(jsonRule["bad_bins_percent"].toDouble());

    if (lPATRecipeOptions.GetRecipeVersion().toDouble() >= 2.10)
    {
        // reticle corners
        lRule.SetRule(jsonRule["rule"].toString());
        lRule.SetRuleName(jsonRule["name"].toString());
        lRule.SetXInk(jsonRule["x_ink"].toInt());
        lRule.SetYInk(jsonRule["y_ink"].toInt());
        lRule.SetDiagInk(jsonRule["diag_ink"].toInt());
        lRule.SetXOffDiag(jsonRule["x_off_diag"].toInt());
        lRule.SetYOffDiag(jsonRule["y_off_diag"].toInt());
        lRule.SetIgnoreDiagonalBadDies(jsonRule["ignore_diag_bad"].toBool());
        QJsonArray lActivatedCorner = jsonRule["activated_corner"].toArray();
        if (lActivatedCorner.size() < 8)
        {
            mErrorMessage = QString("The size of wafer->reticle->activated_corner is not enough. it has to be equal to 8");
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        lRule.SetActivatedCorners(PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[0].toBool() ? PATOptionReticle::CORNER_TOP_LEFT : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[1].toBool() ? PATOptionReticle::TOP : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[2].toBool() ? PATOptionReticle::CORNER_TOP_RIGHT : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[3].toBool() ? PATOptionReticle::RIGHT : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[4].toBool() ? PATOptionReticle::CORNER_BOTTOM_RIGHT : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[5].toBool() ? PATOptionReticle::BOTTOM : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[6].toBool() ? PATOptionReticle::CORNER_BOTTOM_LEFT : PATOptionReticle::NO_CORNER);
        lRule.AddActivatedCorner(lActivatedCorner[7].toBool() ? PATOptionReticle::LEFT : PATOptionReticle::NO_CORNER);
    }

    // Reticle mask name
    QString lMaskName = jsonRule["mask_name"].toString();

    if (lMaskName.isEmpty() == false)
        lRule.SetReticleMaskName(lMaskName);

    // Add rule
    lPATRecipeOptions.GetReticleRules().append(lRule);

    return true;
}

bool PATRecipeIOJson::readFinalTest(QJsonObject finalTest, COptionsPat &lPATRecipeOptions)
{
    if (!finalTest.isEmpty())
    {
        lPATRecipeOptions.SetRecipeType(PAT::RecipeFinalTest);
        QJsonObject patLimits = finalTest["pat_limits"].toObject();
        if (!patLimits.isEmpty())
        {
            /// buffer size
            if (!patLimits["buffer_size"].isDouble())
            {
                mErrorMessage = QString("Value %1 for key buffer_size is not numerical")
                        .arg(patLimits["buffer_size"].toString());
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            lPATRecipeOptions.mFT_TuningSamples = static_cast<int>(patLimits["buffer_size"].toDouble());
            if (lPATRecipeOptions.mFT_TuningSamples < FT_PAT_TUNING_MIN_SIZE)
            {
                mErrorMessage=QString("Tuning Samples value (%1) is lesser than the minimum value allowed (%2)")
                                 .arg(lPATRecipeOptions.mFT_TuningSamples).arg(FT_PAT_TUNING_MIN_SIZE);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            /// Base Line
            QJsonObject baseline = patLimits["baseline"].toObject();
            if (!baseline.isEmpty())
            {
                // Algorithm
                if (baseline[ALGORITHM_KEY].toString() != "standard"
                    && baseline[ALGORITHM_KEY].toString() != "merged_sites")
                {
                    mErrorMessage = QString("Value %1 for key algorithm must be standard or merged_sites")
                            .arg(baseline[ALGORITHM_KEY].toString());
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                if(baseline[ALGORITHM_KEY].toString() == "standard")
                {
                    lPATRecipeOptions.mFT_BaseLineAlgo = FT_PAT_SINGLE_SITE_ALGO;
                }
                else
                {
                    lPATRecipeOptions.mFT_BaseLineAlgo = FT_PAT_MERGED_SITE_ALGO;
                }

                // Total parts in base line
                lPATRecipeOptions.mFT_BaseLine = static_cast<long>(baseline["nb_parts"].toDouble());

                // min sample per site
                if (!baseline["min_parts_per_site"].isDouble())
                {
                    mErrorMessage = QString("The value of baseline->min_parts_per_site is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.mFT_MinSamplesPerSite = static_cast<int>(baseline["min_parts_per_site"].toDouble());
                if (lPATRecipeOptions.mFT_MinSamplesPerSite < FT_PAT_MINIMUM_SAMPLES_PER_SITE)
                {
                    mErrorMessage =
                            QString("Minimum Sample per Site value (%1) is lesser than the minimum value allowed (%2)")
                                    .arg(lPATRecipeOptions.mFT_MinSamplesPerSite).arg(FT_PAT_MINIMUM_SAMPLES_PER_SITE);
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }

                // Alarm level on outliers detected in baseline
                QJsonObject maxOutliers = baseline["max_outliers"].toObject();
                if (!maxOutliers.empty() && maxOutliers[ENABLED_KEY].toBool() == true)
                {
                    if (!maxOutliers["threshold"].isDouble())
                    {
                        mErrorMessage = QString("The value of baseline->max_outliers->threshold is not a number");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                    lPATRecipeOptions.mFT_BaseLineMaxOutliers =
                            static_cast<long>(maxOutliers["threshold"].toDouble());
                }
            }

            /// Tuning
            QJsonObject tuning = patLimits["tuning"].toObject();
            if (!tuning.isEmpty())
            {
                 lPATRecipeOptions.mFT_TuningIsEnabled = tuning[ENABLED_KEY].toBool();
                // Tuning type: (0) = devices, (1) = outliers
                if(tuning["type"].toString() == "devices")
                {
                    lPATRecipeOptions.mFT_TuningType = 0;
                }
                else if (tuning["type"].toString() == "outliers")
                {
                    lPATRecipeOptions.mFT_TuningType = 1;
                }
                else
                {
                    mErrorMessage = QString("Value %1 for key tuning->type must be devices or outliers")
                                           .arg(tuning["type"].toString());
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }

                // PAT dynamic limits tuning frequency
                if (!tuning["frequency"].isDouble())
                {
                    mErrorMessage = QString("The value of baseline->tuning->frequency is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.mFT_Tuning = static_cast<long>(tuning["frequency"].toDouble());
            }
        }

            /// Alarms
            QJsonArray alarms = finalTest["alamrs"].toArray();
            for (int i=0; i<alarms.size(); ++i)
            {
                QJsonObject alarm = alarms[i].toObject();
                lPATRecipeOptions.m_FT_AlarmType = alarm["type"].toString();
                // generic key loader:
                int serverityInt;
                QString severityString = alarm["severity"].toString();
                if (severityString == "critical") serverityInt = 0;
                else if(severityString == "warning") serverityInt = 1;
                else if(severityString == "notice") serverityInt = 2;
                else if(severityString == "ignore") serverityInt = 3;
                else
                {
                    mErrorMessage
                      = QString("Value %1 for key alamrs->severity must be one of critical, warning, notice or ignores")
                                           .arg(alarm["type"].toString());
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.setProperty(QString("FT_Alarm_Severity_Outliers").toLatin1().constData(),
                                       serverityInt);

            }

        /// Misc
        QJsonObject misc = finalTest["misc"].toObject();
        if (!misc.isEmpty())
        {
            // Number of test program runs per packet sent from tester to GTM server
            if (!misc["runs_per_packet"].isDouble())
            {
                mErrorMessage = QString("The value of misc->runs_per_packet is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.mFT_RunsPerPacket = static_cast<int>(misc["runs_per_packet"].toDouble());

            // Traceability mode: disabled, save PAT limits in ASCII file, or STDF file
            if (misc["execution_log"].toString() == "html")
                lPATRecipeOptions.mFT_Traceability = 0;
            else
                lPATRecipeOptions.mFT_Traceability = 1;
        }
    }
    return true;
}

bool PATRecipeIOJson::readSettings(QJsonObject settings, COptionsPat &lPATRecipeOptions)
{
    if (!settings.isEmpty())
    {
        /// Genaral settings
        QJsonObject general = settings["general"].toObject();
        if (!general.isEmpty())
        {
            // GoodHardBins list
            if(lPATRecipeOptions.pGoodHardBinsList != NULL)
                delete lPATRecipeOptions.pGoodHardBinsList;
            QString goodBins = JsonArrayBinsListToString(general["good_hard_bins"].toArray(), "1");

            lPATRecipeOptions.pGoodHardBinsList = new GS::QtLib::Range(goodBins);

            // GoodSoftBins list
            if(lPATRecipeOptions.pGoodSoftBinsList != NULL)
                delete lPATRecipeOptions.pGoodSoftBinsList;

            goodBins = JsonArrayBinsListToString(general["good_soft_bins"].toArray(), "1");
            lPATRecipeOptions.pGoodSoftBinsList = new GS::QtLib::Range(goodBins);

            // Test key (how is a test identified: based on test#, test name, etc...)
            QString testKey = general["test_key"].toString();
            long index = 0;
            if (testKey == "number")
                index = GEX_TBPAT_KEY_TESTNUMBER;
            else if (testKey == NAME_KEY)
                index = GEX_TBPAT_KEY_TESTNAME;
            else if (testKey == "number_name")
                index = GEX_TBPAT_KEY_TESTMIX;
            else
            {
                mErrorMessage = QString("Value %1 for key general->test_key is incorrect").arg(testKey);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.mOptionsTestKey = index;
            lPATRecipeOptions.mTestKey        = index;

            // Only apply PAT Bin to Good Parts (Good Bins only)?
            if(general["bin_outliers"].toString() == "good_parts")
                lPATRecipeOptions.bScanGoodPartsOnly = true;
            else
                lPATRecipeOptions.bScanGoodPartsOnly = false;

            if (general["all_sites_merged"].toBool() == true)
                lPATRecipeOptions.SetAllSitesMerged(true);
            else
                lPATRecipeOptions.SetAllSitesMerged(false);
        }

        /// static_pat settings
        QJsonObject staticPatSettings = settings["static_pat"].toObject();
        if(!staticPatSettings.isEmpty())
        {
            lPATRecipeOptions.bStaticPAT = staticPatSettings[ENABLED_KEY].toBool();
            if (!staticPatSettings[HARD_BIN_KEY].isDouble())
            {
                mErrorMessage = QString("The value of static_pat->staticPatSettings->hard_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.iFailStatic_HBin = static_cast<int>(staticPatSettings[HARD_BIN_KEY].toDouble());

            if (!staticPatSettings[SOFT_BIN_KEY].isDouble())
            {
                mErrorMessage = QString("The value of static_pat->staticPatSettings->soft_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.iFailStatic_SBin = static_cast<int>(staticPatSettings[SOFT_BIN_KEY].toDouble());

            // RGB color for the Static PAT failure
            lPATRecipeOptions.cStaticFailColor = JsonArrayRGBToColor(staticPatSettings[COLOR_KEY].toArray(), QColor(255, 0, 0));

            // Ignore Static PAT limits for tests whith IQR=0
            lPATRecipeOptions.bIgnoreIQR0 = staticPatSettings["ignore_when_iqr_null"].toBool();
            // Ignore static PAT for tests with historical distribution shape = Categories
            lPATRecipeOptions.bIgnoreHistoricalCategories =
                    staticPatSettings["ignore_when_categories"].toBool();

            // Ignore static PAT for tests with Cpk >= xxx
            if (!staticPatSettings["ignore_when_cpk_greater"].isDouble())
            {
                mErrorMessage =
                        QString("The value of static_pat->staticPatSettings->ignore_when_cpk_greater is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.lfIgnoreHistoricalCpk=staticPatSettings["ignore_when_cpk_greater"].toDouble();
        }

        /// dynamic_pat settings
        QJsonObject dynamicPatSettings = settings["dynamic_pat"].toObject();
        if(!dynamicPatSettings.isEmpty())
        {
            lPATRecipeOptions.bDynamicPAT = dynamicPatSettings[ENABLED_KEY].toBool();
            if (!dynamicPatSettings[HARD_BIN_KEY].isDouble())
            {
                mErrorMessage =
                        QString("The value of static_pat->dynamic_pat->hard_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.iFailDynamic_HBin=static_cast<int>(dynamicPatSettings[HARD_BIN_KEY].toDouble());

            if (!dynamicPatSettings[SOFT_BIN_KEY].isDouble())
            {
                mErrorMessage =
                        QString("The value of static_pat->dynamic_pat->soft_bin is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lPATRecipeOptions.iFailDynamic_SBin=static_cast<int>(dynamicPatSettings[SOFT_BIN_KEY].toDouble());

            // RGB color for the dynamic PAT failure
            lPATRecipeOptions.cDynamicFailColor = JsonArrayRGBToColor(dynamicPatSettings[COLOR_KEY].toArray(), QColor(255, 0, 0));

            /// default rule
            bool bStatus;
            int lRule= GS::Gex::PAT::GetDefaultRule(dynamicPatSettings["default_rule"].toString() ,&bStatus);
            lPATRecipeOptions.SetDefaultDynamicRule((COptionsPat::GexDefaultRuleSet)lRule);
            /// Limits
            QJsonObject limits = dynamicPatSettings["limits"].toObject();
            if(!limits.isEmpty())
            {
                QString strSection;
                // Build PAT limits from given Bins list
                strSection = limits["bin_filter"].toString();

                if(strSection.isEmpty())
                    strSection = "good_softbins";	// Force good bin

                if(strSection == "all")
                    lPATRecipeOptions.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_ALLBINS;
                else if(strSection == "good" || strSection == "good_softbins")
                    lPATRecipeOptions.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_GOODSOFTBINS;
                else if(strSection == "including" || strSection == "including_softbins")
                    lPATRecipeOptions.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_LISTSOFTBINS;
                else if(strSection == "excluding" || strSection == "excluding_softbins")
                    lPATRecipeOptions.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_EXCEPTSOFTBINS;
                else if(strSection == "good_hardbins")
                    lPATRecipeOptions.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_GOODHARDBINS;
                else if(strSection == "including_hardbins")
                    lPATRecipeOptions.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_LISTHARDBINS;
                else if(strSection == "excluding_hardbins")
                    lPATRecipeOptions.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_EXCEPTHARDBINS;

                lPATRecipeOptions.strPatLimitsFromBin = limits["bins"].toString();

                // Do not allow PAT limits to be outside of spec limits.
                lPATRecipeOptions.bStickWithinSpecLimits = limits["within_original_limits"].isBool()?
                            limits["within_original_limits"].toBool():true;

                // Stop outlier identification & removal on a parameter if its Cpk >= to a given value.
                if (!limits["ignore_high_cpk"].isDouble())
                {
                    mErrorMessage =
                            QString("The value of static_pat->dynamic_pat->limits->ignore_high_cpk is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.lfSmart_IgnoreHighCpk = limits["ignore_high_cpk"].isDouble()?
                            limits["ignore_high_cpk"].toDouble():-1;

                lPATRecipeOptions.mSmart_IgnoreHighCpkEnabled = limits["ignore_high_cpk_enabled"].isBool()?
                            limits["ignore_high_cpk_enabled"].toBool():false;


                // Minimum samples  required per test (otherwise ignore)
                if (!limits["minimum_samples"].isDouble())
                {
                    mErrorMessage =
                            QString("The value of static_pat->dynamic_pat->limits->minimum_samples is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.iMinimumSamples = static_cast<long>(limits["minimum_samples"].toDouble());

                // Minimum number of Outliers to fail in  a flow to issue a failing PAT bin.
                if (!limits["minimum_outliers_to_fail"].isDouble())
                {
                    mErrorMessage =
                      QString("The value of static_pat->dynamic_pat->limits->minimum_outliers_to_fail is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.iMinimumOutliersPerPart =
                        static_cast<long>(limits["minimum_outliers_to_fail"].toDouble());

                // Stop on first PAT failure in flow?
                lPATRecipeOptions.bStopOnFirstFail = limits["stop_on_first_fail"].toBool();

                if (!limits["category_value_count"].isDouble())
                {
                    mErrorMessage =
                          QString("The value of static_pat->dynamic_pat->limits->category_value_count is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.iCategoryValueCount =
                        static_cast<int>(limits["category_value_count"].toDouble());

                // Category distribution definition
                lPATRecipeOptions.bAssumeIntegerCategory = limits["assume_integer_category"].toBool();
            }

            ///  multivariate pat
            QJsonObject lMultivariatePat = settings["multi_variate_pat"].toObject();
            if(!lMultivariatePat.isEmpty())
            {
                lPATRecipeOptions.SetMVPATEnabled(lMultivariatePat[ENABLED_KEY].toBool());
                if (!lMultivariatePat[HARD_BIN_KEY].isDouble())
                {
                    mErrorMessage =
                    QString("The value of settings->multi_variate_pat->category_value_count->hard_bin is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.SetMVPATHardBin(static_cast<int>(lMultivariatePat[HARD_BIN_KEY].toDouble()));

                if (!lMultivariatePat[SOFT_BIN_KEY].isDouble())
                {
                    mErrorMessage =
                    QString("The value of settings->multi_variate_pat->category_value_count->soft_bin is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.SetMVPATSoftBin(static_cast<int>(lMultivariatePat[SOFT_BIN_KEY].toDouble()));

                lPATRecipeOptions.SetMVPATColor(JsonArrayRGBToColor(lMultivariatePat["fail_color"].toArray(), QColor(255, 0, 0)));
                lPATRecipeOptions.SetMVPATAutomaticGroupCreation(lMultivariatePat["auto_group_creation"].toBool());
                lPATRecipeOptions.SetMVPATNormalShapeOnly(lMultivariatePat["normal_shape_only"].toBool());

                if (!lMultivariatePat["correlation_threshold"].isDouble())
                {
                    mErrorMessage =
                 QString("The value of multi_variate_pat->category_value_count->correlation_threshold is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.SetMVPATGroupCorrelation(lMultivariatePat["correlation_threshold"].toDouble());

                QJsonObject outlierDistance = lMultivariatePat["outlier_distance"].toObject();

                if (!outlierDistance["near"].isDouble())
                {
                    mErrorMessage = QString("The value of multi_variate_pat->outlier_distance->near is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.SetMVPATDistance(PAT::Near, outlierDistance["near"].toDouble());

                if (!outlierDistance["medium"].isDouble())
                {
                    mErrorMessage = QString("The value of multi_variate_pat->outlier_distance->medium is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.SetMVPATDistance(PAT::Medium, outlierDistance["medium"].toDouble());

                if (!outlierDistance["far"].isDouble())
                {
                    mErrorMessage = QString("The value of multi_variate_pat->outlier_distance->far is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.SetMVPATDistance(PAT::Far, outlierDistance["far"].toDouble());

                lPATRecipeOptions.SetMVPATIgnorePPATBins(lMultivariatePat["ignore_ppat_bins"].toBool());
            }

            /// Geographic PAT
            QJsonObject geographicPat = settings["geographic_pat"].toObject();
            if (geographicPat.isEmpty() == false)
            {
                lPATRecipeOptions.mGPAT_IgnorePPatBins = (geographicPat["ignore_ppat_bins"].isBool() ?
                                                            geographicPat["ignore_ppat_bins"].toBool() : true);
            }

            /// report
            QJsonObject report = settings["report"].toObject();
            if (!report.isEmpty())
            {
                lPATRecipeOptions.bReport_Stats =
                    (report["stats"].isBool()?report["stats"].toBool():true);
                lPATRecipeOptions.bReport_Histo =
                    (report["histo"].isBool()?report["histo"].toBool():true);
                // PAT-71
                lPATRecipeOptions.bReport_Histo_NoOutliers =
                    (report["histo_no_outliers"].isBool()?report["histo_no_outliers"].toBool():false);
                lPATRecipeOptions.bReport_Wafermap =
                    (report["wafermap"].isBool()?report["wafermap"].toBool():true);
                if(report["wafermap_type"].toString() == HARD_BIN_KEY)
                    lPATRecipeOptions.iReport_WafermapType = 1;
                else
                    lPATRecipeOptions.iReport_WafermapType = 0;
                lPATRecipeOptions.bReport_Pareto =
                   (report["pareto"].isBool()?report["pareto"].toBool():true);
                lPATRecipeOptions.bReport_Binning =
                   (report["binning"].isBool()?report["binning"].toBool():true);
                lPATRecipeOptions.bReport_SPAT_Limits =
                   (report["spat_limits"].isBool()?report["spat_limits"].toBool():true);
                lPATRecipeOptions.bReport_DPAT_Limits_Outliers =
                   (report["dpat_limits_outliers"].isBool()?report["dpat_limits_outliers"].toBool():true);
                lPATRecipeOptions.bReport_DPAT_Limits_NoOutliers =
                  (report["dpat_limits_no_outliers"].isBool()?report["dpat_limits_no_outliers"].toBool():false);

                // Mvpat tests
                lPATRecipeOptions.SetMVPATReportStdCharts(
                  report["mvpat_standard_charts"].isBool()?report["mvpat_standard_charts"].toBool():false);
                QJsonObject lMvPatCorrelationChart = report["mvpat_correlation_charts"].toObject();
                if ((!lMvPatCorrelationChart.isEmpty()) && (lMvPatCorrelationChart[ENABLED_KEY].toBool()==true))
                {
                    lPATRecipeOptions.SetMVPATReportCorrCharts(lMvPatCorrelationChart[ENABLED_KEY].toBool());
                    if (lMvPatCorrelationChart["dataset"].toString() == "pca")
                        lPATRecipeOptions.SetMVPATReportPCAProjection(true);
                    else
                        lPATRecipeOptions.SetMVPATReportPCAProjection(false);
                    if (lMvPatCorrelationChart["charts"].toString() == "all_pairs")
                        lPATRecipeOptions.SetMVPATReportPairs(PAT::AllPairs);
                    else
                        lPATRecipeOptions.SetMVPATReportPairs(PAT::ConsecutivePairs);

                    if (!lMvPatCorrelationChart["max_charts"].isDouble())
                    {
                        mErrorMessage = QString("The value of mvpat_correlation_charts->max_charts is not a number");
                        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                    lPATRecipeOptions.SetMVPATReportMaxCharts(lMvPatCorrelationChart["max_charts"].toDouble());
                }
            }

            /// Recipe versioning
            QJsonObject recipeVersioning = settings["recipe_versioning"].toObject();
            if (!recipeVersioning.isEmpty())
            {
                if (!recipeVersioning["major"].isDouble())
                {
                    mErrorMessage = QString("The value of recipe_versioning->major is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.iRecipeVersion = static_cast<int>(recipeVersioning["major"].toDouble());

                if (!recipeVersioning["minor"].isDouble())
                {
                    mErrorMessage = QString("The value of recipe_versioning->minor is not a number");
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                lPATRecipeOptions.iRecipeBuild = static_cast<int>(recipeVersioning["minor"].toDouble());

                QFileInfo lFile(mRecipeName);

                // Do not read Product ID from default config File (5951)
                if (lFile.fileName() !=  GS::Gex::PAT::GetDefaultRecipeName(GS::Gex::PAT::RecipeWaferSort))
                    lPATRecipeOptions.strProductName = recipeVersioning["product"].toString();
                else
                    lPATRecipeOptions.strProductName.clear();
            }
        }
    }
    return true;
}


bool PATRecipeIOJson::readUnivariateRules(QJsonArray univariateRules, COptionsPat &lPATRecipeOptions,
                                      QHash<QString, CPatDefinition *> &lPATUnivariate)
{
    // Read Static PAT limits file...created manually or with the ToolBox!
    CPatDefinition cPatDef;	// To hold a PAT definition
    QString	strString, lSection;
    QString	strKey;
    bool	bStatus,bDisableStaticPAT;
    int		iStatus             = 0;
    int		iKey_TestNumber     = 0;
    int		iKey_TestName       = 0;
    double	lfHistoricalCpk     = 0.0;
    long	lTestSequenceID     = 0;
    bool    lRemoveSeqName      = mReadOptions.contains("TESTNAME_REMOVE_SEQUENCER");

    int nbRules = univariateRules.size();
    for (int i=0; i < nbRules; ++i)
    {
        QJsonObject testObject = univariateRules[i].toObject();

        if(testObject.isEmpty())
            continue;
        // Clear stucture to hold definition
        cPatDef.clear();

        // Extract Static Fail Bin info from line
        if(!testObject["static_bin"].isDouble())
        {
            // Invalid line...
            mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += " : invalid 'univariate_rules->static_bin' value";
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        cPatDef.m_lFailStaticBin = static_cast<long>(testObject["static_bin"].toDouble());


        // Extract Dynamic Fail Bin info from line
        cPatDef.m_lFailDynamicBin = static_cast<long>(testObject["dynamic_bin"].toDouble());
        if(!testObject["dynamic_bin"].isDouble())
        {
            // Invalid line...
            mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += " : invalid 'univariate_rules->dynamic_bin' value";
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }

        // Extract Testnumber info from line
        if(!testObject[TEST_NUMBER_KEY].isDouble())
        {
            // Invalid line...
            mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += " : invalid univariate_rules->" + QString(TEST_NUMBER_KEY);
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        cPatDef.m_lTestNumber = static_cast<unsigned long>(testObject[TEST_NUMBER_KEY].toDouble());
        iKey_TestNumber++;

        // Extract type info from line
        cPatDef.SetTestTypeLegacy(((testObject["test_type"]).toString().toLatin1().constData())[0]);

        // Extract type info from line
        if(!testObject[PIN_INDEX_KEY].isDouble())
        {
            // Invalid line...
            mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += " : invalid univariate_rules->" + QString(PIN_INDEX_KEY);
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        cPatDef.mPinIndex = static_cast<long>(testObject[PIN_INDEX_KEY].toDouble());

        //  Extract test name.
        strString = (testObject[TEST_NAME_KEY]).toString();
        // Replace the _; with the comma  (comma being a separator, it can't be written as-is in the file!
        strString = strString.replace("_;",",");

        // Only applies to Real-time (where test program such as image reads test names without sequencer name)
        if(lRemoveSeqName)
        {
            // check if Teradyne test name with sequencer string at the end. If so, remove it!
            iStatus = strString.indexOf(" <>");
            if(iStatus >= 0)
                strString.truncate(iStatus);
        }

        cPatDef.m_strTestName = strString.replace("__","_");				// Replace the  double '_' with the '_'
        // Test list key is on 'test name'
        if(!cPatDef.m_strTestName.isEmpty())
            iKey_TestName++;

        if((cPatDef.m_lTestNumber == (unsigned)GEX_PTEST) && (cPatDef.m_strTestName.isEmpty()))
        {
            // Invalid line...
            mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += " : need to define at least a valid test number or test name";
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }

        // Get the Low Limit
        strString = (testObject["low_limit"]).toString();
        cPatDef.m_lfLowLimit = -GEX_TPAT_DOUBLE_INFINITE;
        if(strString.isEmpty() == false)
        {
            lSection = strString.section(' ',0,0);
            if(!lSection.isEmpty())
            {
                // Test LL
                bool lOk=false;
                double lLL = lSection.toDouble(&lOk);
                if(lOk)
                    cPatDef.m_lfLowLimit = lLL;
                else
                    GSLOG(SYSLOG_SEV_CRITICAL,
                          QString("Error extracting LL for test %1 from line %2 when loading recipe")
                          .arg(cPatDef.m_lTestNumber).arg(strString).toLatin1().data());
            }
            lSection = strString.section(' ',1,1);
            if(!lSection.isEmpty())
                // Test units
                cPatDef.m_strUnits = lSection;
        }

        // Get the High Limit
        strString = (testObject["high_limit"]).toString();
        cPatDef.m_lfHighLimit = GEX_TPAT_DOUBLE_INFINITE;
        if(strString.isEmpty() == false)
        {
            lSection = strString.section(' ',0,0);
            if(!lSection.isEmpty())
            {
                // Test HL
                bool lOk=false;
                double lHL = lSection.toDouble(&lOk);
                if(lOk)
                    cPatDef.m_lfHighLimit = lHL;
                else
                    GSLOG(SYSLOG_SEV_CRITICAL,
                          QString("Error extracting HL for test %1 from line %2 when loading recipe")
                          .arg(cPatDef.m_lTestNumber).arg(strString).toLatin1().data());
            }
            lSection = strString.section(' ',1,1);
            if(!lSection.isEmpty())
                // Test units
                cPatDef.m_strUnits = lSection;
        }

        // Get dpat settings
        QJsonObject dpatSetting = testObject["dpat_settings"].toObject();
        if (!dpatSetting.isEmpty())
        {
            // Get Historical shape
            cPatDef.m_iDistributionShape = PAT::GetDistributionID(dpatSetting["shape"].toString());

            // Extract 'Samples to ignore' Rule ("none", "Negative values", "Positive values")
            strString = dpatSetting["samples_to_ignore"].toString();
            cPatDef.m_SamplesToIgnore= GS::Gex::PAT::GetSamplesToIgnoreIndex(strString,&bStatus);
            if(bStatus == false)
            {
                // Invalid rule specified, not in our list...
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid 'univariate_rules->samples_to_ignore'";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            // Extract 'Outliers to keep' Rule ("none", "low values", "high values")
            strString = dpatSetting["outliers_to_keep"].toString();
            cPatDef.m_OutliersToKeep = GS::Gex::PAT::GetOutlierToKeepIndex(strString,&bStatus);
            if(bStatus == false)
            {
                // Invalid rule specified, not in our list...
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid 'univariate_rules->Outliers To Keep'";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            // Extract 'Outlier Limits set': near, medium, far
            strString = dpatSetting["outlier_limits_set"].toString();
            cPatDef.m_iOutlierLimitsSet = GS::Gex::PAT::GetOutlierLimitsSetIndex(strString,&bStatus);
            if(bStatus == false)
            {
                // Invalid rule specified, not in our list...
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid 'univariate_rules->outlier_limits_set'";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            // Extract Outlier Rule (% of limits, N*Sigma, Smart, disabled...)
            strString = dpatSetting["outlier_rule"].toString();
            cPatDef.mOutlierRule = GS::Gex::PAT::GetRuleIndexForJson(strString,&bStatus);
            if(bStatus == false)
            {
                // Invalid rule specified, not in our list...
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid 'univariate_rules->Outlier Rule' selected";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            // Extract Outlier space number (N factor / Head factor)...if it exists!
            if(dpatSetting["head_factor"].isDouble())
            {
                cPatDef.m_lfOutlierNFactor = dpatSetting["head_factor"].toDouble();
                // Unless N factor is a cutom limit, remove its sign.
                if(cPatDef.mOutlierRule != GEX_TPAT_RULETYPE_NEWLIMITSID)
                    cPatDef.m_lfOutlierNFactor = fabs(cPatDef.m_lfOutlierNFactor);
                bStatus = true;	// Good value read.
            }
            else
            {
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid 'univariate_rules->head_factor'";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                bStatus = false;
            }

            if(!bStatus &&
                ((cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_SIGMAID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_ROBUSTSIGMAID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_Q1Q3IQRID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_NEWLIMITSID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_RANGEID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_LIMITSID)))
            {
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid/missing 'Outlier N*Factor' value";
                return false;
            }

            // Extract T factor (Tail)
            if(dpatSetting["tail_factor"].isDouble())
            {
                cPatDef.m_lfOutlierTFactor = dpatSetting["tail_factor"].toDouble();
                // Unless T factor is a cutom limit, remove its sign.
                if(cPatDef.mOutlierRule != GEX_TPAT_RULETYPE_NEWLIMITSID)
                    cPatDef.m_lfOutlierTFactor = fabs(cPatDef.m_lfOutlierTFactor);
                bStatus = true;	// Good value read.
            }
            else
            {
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid 'univariate_rules->tail_factor'";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                bStatus = false;
            }

            // Extract tail management rule only if version if version is high enough
            if (lPATRecipeOptions.GetRecipeVersion().toDouble() >= 2.04)
            {
                strString = dpatSetting["tail_management"].toString();
                cPatDef.mTailMngtRuleType = GS::Gex::PAT::GetTailMngtRuleIndex(strString,&bStatus);
                if(bStatus == false)
                {
                    // Invalid rule specified, not in our list...
                    // Invalid line...
                    mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                    mErrorMessage += " : invalid 'univariate_rules->Tail management Rule' selected";
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
            }

            // If 'T' factor not specified while a 'N' factor was, then force 'T' = 'N'
            if(!bStatus && (cPatDef.m_lfOutlierNFactor != 0))
                cPatDef.m_lfOutlierTFactor = cPatDef.m_lfOutlierNFactor;

            if(!bStatus && ((cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_GAUSSIANID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_GAUSSIANTAILID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_LOGNORMALID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_BIMODALID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_MULTIMODALID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_CLAMPEDID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_DUALCLAMPEDID) ||
                (cPatDef.mOutlierRule == GEX_TPAT_RULETYPE_CATEGORYID)) )
            {
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid/missing 'Outlier T*Factor' value";
                return false;
            }
            cPatDef.mDPATNote = dpatSetting["dpat_notes"].toString();
        }
        ///////////////////// end of getting of dpat_settings parameters /////////////////////////////////////


        /////////////////////////// getting spat_settings parameters /////////////////////////////////////////
        QJsonValue spatSettingsVal = testObject["spat_settings"];
        if (!spatSettingsVal.isNull())
        {
            QJsonObject spatSettings = spatSettingsVal.toObject();
            // Read SPAT rule: AEC (default), or Sigma or Range, etc
            if(!spatSettings["outlier_limits_set"].isNull())
                strString = spatSettings["outlier_limits_set"].toString();
            else
                strString = "0";	// Default to AEC SPAT rule

            cPatDef.m_SPATRuleType = GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA;
            if(strString.isEmpty() == true)
                bStatus = true;
            else
            {
                cPatDef.m_SPATRuleType = GS::Gex::PAT::GetSPATRuleIndex(strString,NULL);
                bStatus = true;	// Good value read.
            }
            if(!bStatus)
            {
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid/missing 'univariate_rules->SPAT rule' value";
                return false;
            }

            // Extract Outlier space number (N factor / Head factor)...if it exists!
            double factor = 6;
            if(!spatSettings["head_factor"].isNull())
            {
                if(!spatSettings["head_factor"].isDouble())
                {
                    // Invalid line...
                    mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                    mErrorMessage += " : invalid univariate_rules->head_factor";
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                factor = spatSettings["head_factor"].toDouble();
            }
            cPatDef.m_lfSpatOutlierNFactor = factor;
            if(cPatDef.m_SPATRuleType != GEX_TPAT_SPAT_ALGO_NEWLIMITS)
                cPatDef.m_lfSpatOutlierNFactor = fabs(cPatDef.m_lfSpatOutlierNFactor);

            // Extract T factor (Tail)
            if(!spatSettings["tail_factor"].isNull())
            {
                if(!spatSettings["tail_factor"].isDouble())
                {
                    // Invalid line...
                    mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                    mErrorMessage += " : invalid univariate_rules->tail_factor";
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }
                factor = spatSettings["tail_factor"].toDouble();
            }
            cPatDef.m_lfSpatOutlierTFactor = factor;
            if(cPatDef.m_SPATRuleType != GEX_TPAT_SPAT_ALGO_NEWLIMITS)
                cPatDef.m_lfSpatOutlierTFactor = fabs(cPatDef.m_lfSpatOutlierTFactor);

            cPatDef.mSPATNote = spatSettings["spat_notes"].toString();
        }
        ///////////////////// end of reading of spat_settings parameters /////////////////////////////////////

        /////////////////////////// reading nnr_settings parameters /////////////////////////////////////////
        QJsonObject nnrSettings = testObject["nnr_settings"].toObject();
        if (!nnrSettings.isEmpty())
        {
            // Extract NNR rule
            if(!nnrSettings[ENABLED_KEY].isNull())
            {
                if (nnrSettings[ENABLED_KEY].isBool() == false)
                {
                    // Invalid rule specified, not in our list...
                    // Invalid line...
                    mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                    mErrorMessage += " : invalid 'univariate_rules->NNR Rule'";
                    GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                    return false;
                }

                if (nnrSettings[ENABLED_KEY].toBool() == true)
                {
                    cPatDef.m_iNrrRule = GEX_TPAT_NNR_ENABLED;
                }
                else
                {
                    cPatDef.m_iNrrRule = GEX_TPAT_NNR_DISABLED;
                }
            }
            else
            {
                cPatDef.m_iNrrRule = GEX_TPAT_NNR_DISABLED;
            }
        }
        ///////////////////// end of reading of nnr_settings parameters /////////////////////////////////////

        /////////////////////////// reading historical_stats parameters /////////////////////////////////////
        QJsonObject historicalStats = testObject["historical_stats"].toObject();
        if (!historicalStats.isEmpty())
        {
            // Get Historical Cpk
            if(!historicalStats["cpk"].isDouble())
            {
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid univariate_rules->cpk";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            lfHistoricalCpk = historicalStats["cpk"].toDouble();
            cPatDef.m_lHistoricalCPK = lfHistoricalCpk;

            // Extract Median value
            if(!historicalStats["robust_mean"].isDouble())
            {
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid univariate_rules->robust_mean";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            cPatDef.m_lfMedian = historicalStats["robust_mean"].toDouble();
            if(!historicalStats["robust_mean"].isDouble())
            {
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid Robust mean (Median)";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            // Extract Robust sigma value
            if(!historicalStats["robust_sigma"].isDouble())
            {
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid univariate_rules->robust_sigma";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            cPatDef.m_lfRobustSigma = historicalStats["robust_sigma"].toDouble();

            // Extract Mean value
            if(!historicalStats["mean"].isDouble())
            {
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid univariate_rules->mean";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            cPatDef.m_lfMean = historicalStats["mean"].toDouble();

            // Extract Sigma value
            if(!historicalStats["sigma"].isDouble())
            {
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid univariate_rules->sigma";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            cPatDef.m_lfSigma = historicalStats["sigma"].toDouble();

            // Extract Range value
            if(!historicalStats["range"].isDouble())
            {
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid univariate_rules->range";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            cPatDef.m_lfRange = historicalStats["range"].toDouble();

            // Read Low limit scale factor
            if(historicalStats["ll_scale"].isNull())
            {
                cPatDef.m_llm_scal= 0;
            }
            else if (historicalStats["ll_scale"].isDouble())
            {
                cPatDef.m_llm_scal = static_cast<int>(historicalStats["ll_scale"].toDouble());
            }
            else
            {
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid/missing 'univariate_rules->ll_scale' value";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            // Read High limit scale factor
            if(historicalStats["hl_scale"].isDouble())
            {
                cPatDef.m_hlm_scal = static_cast<int>(historicalStats["hl_scale"].toDouble());
            }
            else if(historicalStats["hl_scale"].isNull())
            {
                cPatDef.m_hlm_scal = 0;
            }
            else
            {
                // Invalid line...
                mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += " : invalid/missing 'univariate_rules->hl_scale' value";
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
        }
        /////////////////////////// end of reading historical_stats parameters ///////////////////////////////////

        //        // Check if test number or test name or both
        //        if(iKey_TestName && iKey_TestNumber && (iKey_TestName != iKey_TestNumber))
        //        {
        //            // Invalid file...
        //            mErrorMessage = "*Error* Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
        //            mErrorMessage += "Some lines provide test numbers/names and some don't."
        //              +"You have to chose either one for all tests";
        //            return false;
        //        }

        // Compute key access mode
        if(lPATRecipeOptions.mOptionsTestKey == GEX_TBPAT_KEY_DETECT)
        {
            if(iKey_TestName == iKey_TestNumber)
                lPATRecipeOptions.mTestKey = GEX_TBPAT_KEY_TESTMIX;
            else
            if(iKey_TestName)
                lPATRecipeOptions.mTestKey = GEX_TBPAT_KEY_TESTNAME;
            else
                lPATRecipeOptions.mTestKey = GEX_TBPAT_KEY_TESTNUMBER;
        }
        else
            lPATRecipeOptions.mTestKey = lPATRecipeOptions.mOptionsTestKey;

        // See if we consider tests based on the 'Test name' or 'test number' (the access key)
        switch(lPATRecipeOptions.mTestKey)
        {
            case GEX_TBPAT_KEY_TESTNUMBER:
                strKey = QString::number(cPatDef.m_lTestNumber);
                if(cPatDef.mPinIndex >= 0)
                    strKey += "." + QString::number(cPatDef.mPinIndex);
                break;
            case GEX_TBPAT_KEY_TESTNAME:
                strKey = cPatDef.m_strTestName.trimmed();
                if(cPatDef.mPinIndex >= 0)
                    strKey += "." + QString::number(cPatDef.mPinIndex);
                break;
            case GEX_TBPAT_KEY_TESTMIX:
                strKey = cPatDef.m_strTestName.trimmed();
                strKey += "." + QString::number(cPatDef.m_lTestNumber);
                if(cPatDef.mPinIndex >= 0)
                    strKey += "." + QString::number(cPatDef.mPinIndex);
                break;
        }

        // Compute Static PAT limits...see if disabled test or not
        bDisableStaticPAT = false;

        // If IQR is 0 (means loads of data are identical),
        // then we may simply ignore PAT on this test (if option is set to do so)
        if(cPatDef.m_lfRobustSigma == 0 && lPATRecipeOptions.bIgnoreIQR0)
            bDisableStaticPAT = true;

        // Check If Cpk high enough, and option set to ignore tests with high cpk...
        if(lPATRecipeOptions.lfIgnoreHistoricalCpk >=0
                && lfHistoricalCpk >= lPATRecipeOptions.lfIgnoreHistoricalCpk)
            bDisableStaticPAT = true;

        // Check if historical distribution shape is 'Categories' and option set to ignore such tests
        if(lPATRecipeOptions.bIgnoreHistoricalCategories && cPatDef.m_iDistributionShape == PATMAN_LIB_SHAPE_CATEGORY)
            bDisableStaticPAT = true;

        if(bDisableStaticPAT)
        {
            // Give infinite limits: Static PAT disabled!
            cPatDef.m_lfLowStaticLimit = -GEX_TPAT_DOUBLE_INFINITE;
            cPatDef.m_lfHighStaticLimit = GEX_TPAT_DOUBLE_INFINITE;
        }
        else
        {
            // Compute SPAT limits
            cPatDef.ComputeStaticLimits();
            // Check SPAT limits (exceed std limits?)
            cPatDef.CheckStaticLimits(lPATRecipeOptions);
        }

        // Keep track of test order in configuration file (as it is the execution flow order)
        cPatDef.m_lSequenceID = lTestSequenceID++;

        // Save PAT definition in our list
        // Holds the Static PAT limits for each test
        if (lPATUnivariate.contains(strKey) == false)
            lPATUnivariate.insert(strKey, new CPatDefinition(cPatDef));
        else
        {
            // Invalid file...
            mErrorMessage = "Outlier Configuration file parsing error :\n" + mRecipeName + "\n";
            mErrorMessage += "Duplicated test rules for";
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

            switch(lPATRecipeOptions.mTestKey)
            {
                case GEX_TBPAT_KEY_TESTNUMBER:
                    mErrorMessage += " test number " + QString::number(cPatDef.m_lTestNumber);
                    if(cPatDef.mPinIndex >= 0)
                        mErrorMessage += " and pin index " + QString::number(cPatDef.mPinIndex);
                    break;
                case GEX_TBPAT_KEY_TESTNAME:
                    mErrorMessage += " test name '" + cPatDef.m_strTestName.trimmed() + "'";
                    if(cPatDef.mPinIndex >= 0)
                        mErrorMessage += " and pin index " + QString::number(cPatDef.mPinIndex);
                    break;
                case GEX_TBPAT_KEY_TESTMIX:
                    mErrorMessage += " test number " + QString::number(cPatDef.m_lTestNumber);
                    mErrorMessage += " with test name '" + cPatDef.m_strTestName.trimmed() + "'";
                    if(cPatDef.mPinIndex >= 0)
                        mErrorMessage += " and pin index" + QString::number(cPatDef.mPinIndex);
                    break;
            }
            return false;
        }
    }
    // success
    return true;
}

bool PATRecipeIOJson::readMultivariateRules(QJsonArray multivariateRules,
                                            QList<PATMultiVariateRule>& lPATMultivariate)
{
    int nbRules = multivariateRules.size();
    for (int i=0; i < nbRules; ++i)
    {
        PATMultiVariateRule multiVariateRule;
        QJsonObject testObject = multivariateRules[i].toObject();
        if(testObject.isEmpty())
            continue;

        multiVariateRule.SetEnabled(testObject[ENABLED_KEY].toBool());
        multiVariateRule.SetName(testObject[NAME_KEY].toString());

        if (!testObject[HARD_BIN_KEY].isDouble())
        {
            mErrorMessage = QString("The value of multivariate_rules->hard_bin is not a number");
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        multiVariateRule.SetBin(static_cast<int>(testObject[HARD_BIN_KEY].toDouble()));

        if (!testObject[SOFT_BIN_KEY].isDouble())
        {
            mErrorMessage = QString("The value of multivariate_rules->soft_bin is not a number");
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        multiVariateRule.SetBin(static_cast<int>(testObject[SOFT_BIN_KEY].toDouble()));

        if (!testObject["custom_distance"].isDouble())
        {
            mErrorMessage = QString("The value of multivariate_rules->custom_distance is not a number");
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        multiVariateRule.SetCustomDistance((testObject["custom_distance"].toDouble()));

        if (!testObject["principal_components"].isDouble())
        {
            mErrorMessage = QString("The value of multivariate_rules->principal_components is not a number");
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
        multiVariateRule.SetPrincipalComponents(static_cast<int>(testObject["principal_components"].toDouble()));
        QString str = testObject["outlier_distance_mode"].toString();
        if (str == "near") multiVariateRule.SetOutlierDistanceMode(PAT::Near);
        else if (str == "medium") multiVariateRule.SetOutlierDistanceMode(PAT::Medium);
        else if (str == "far") multiVariateRule.SetOutlierDistanceMode(PAT::Far);
        else if (str == "custom") multiVariateRule.SetOutlierDistanceMode(PAT::Custom);
        else
        {
            mErrorMessage =
                    QString("Value %1 for key multivariate_rules->outlier_distance_mode is incorrect").arg(str);
            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }

        str = testObject["type"].toString();
        if (str == "generated") multiVariateRule.SetRule(PATMultiVariateRule::Generated);
        else if (str == "manual") multiVariateRule.SetRule(PATMultiVariateRule::Manual);
        else return false;

        QJsonArray testData = testObject["test_data"].toArray();
        int testDataSize = testData.size();
        for (int j=0; j<testDataSize; ++j)
        {
            QJsonObject test = testData[j].toObject();

            if (!test[TEST_NUMBER_KEY].isDouble())
            {
                mErrorMessage = QString("The value of multivariate_rules->test_data->"
                                        + QString(TEST_NUMBER_KEY) + " is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            int number = static_cast<int>(test[TEST_NUMBER_KEY].toDouble());

            if (!test[PIN_INDEX_KEY].isDouble())
            {
                mErrorMessage = QString("The value of multivariate_rules->test_data->"
                                        + QString(PIN_INDEX_KEY) + " is not a number");
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
            int index = static_cast<int>(test[PIN_INDEX_KEY].toDouble());
            QString name = test[TEST_NAME_KEY].toString();
            multiVariateRule.AddTestData(PATMultiVariateRule::MVTestData(name, number, index));
        }

        lPATMultivariate.append(multiVariateRule);
    }
    return true;
}

}   // namespace Gex
}   // namespace GS
