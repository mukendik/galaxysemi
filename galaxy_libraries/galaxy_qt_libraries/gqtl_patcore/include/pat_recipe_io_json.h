#ifndef PATRECIPEIOJSON
#define PATRECIPEIOJSON

#include "pat_recipe_io.h"
#include "pat_mv_rule.h"

namespace GS
{
namespace Gex
{

#define PIN_INDEX_KEY "pin_index"
#define TEST_NAME_KEY "test_name"
#define TEST_NUMBER_KEY "test_number"
#define MEAN_N_SIGMA_KEY "mean_n_sigma"
#define MEAN_ROBUST_SIGMA_KEY "median_robust_sigma"
#define IQR_KEY "iqr"
#define CUSTOMER_PAT_LIB_KEY "custom_pat_lib"
#define FORMULA_KEY "formula"
#define APPLY_PAT_KEY "apply_pat"
#define SOFT_BIN_KEY "soft_bin"
#define HARD_BIN_KEY "hard_bin"
#define ENABLED_KEY "enabled"
#define COLOR_KEY "color"
#define RULE_KEY "rules"
#define ALGORITHM_KEY "algorithm"
#define STDF_SOFT_BIN_KEY "stdf_soft_bin"
#define STDF_HARD_BIN_KEY "stdf_hard_bin"
#define EXTERNAL_KEY "external"
#define BAD_BINS_KEY "bad_bins"
#define NAME_KEY "name"
#define OVERALL_PAT_YIELD_ALARM_LIMIT_KEY "overall_pat_yield_alarm_limit"
#define RETICLE_SIZE_X      "x_size"
#define RETICLE_SIZE_Y      "y_size"
#define RETICLE_SIZE_SOURCE "size_source"

class PATRecipe;
class PATRecipeIOJson : public PATRecipeIO
{
public:
    PATRecipeIOJson();
    ~PATRecipeIOJson();

    /// \brief Write the recipe in the file
    /// \param lPATRecipe contains the recipe informations
    ///        recipeName Contains the name of the file in witch the recipe will be wrote
    bool Write(PATRecipe &lPATRecipe, const QString& recipeName = "");

    bool Write(COptionsPat& lPatOptions, const QString& recipeName);

protected:
    /// \brief Read the input stream and set recipe options, univariates parameters and multivariates parameters
    bool Read(QTextStream &lRecipeStream, PATRecipe& lPATRecipe);

    /// \brief Convert recipe options from json to COptionsPat options
    bool readOptions(QJsonObject outlierOptions, COptionsPat& lPATRecipeOptions);
    /// \brief Convert recipe univarite part from json to QHash
    bool readUnivariateRules(QJsonArray univariateRules,
                             COptionsPat &lPATRecipeOptions,
                             QHash<QString, CPatDefinition *> &lPATUnivariate);
    /// \brief Convert smart rules from json to COptionsPat options (smart rules)
    bool readSmartRules(QJsonObject aSmartRules, COptionsPat &aPATRecipeOptions);
    /// \brief Convert wafer sort rules from json to COptionsPat options (wafer sort rules)
    bool readWaferSort(QJsonObject wafer, COptionsPat &lPATRecipeOptions);
    /// \brief Read wafer sort Reticle rules from json to COptionsPat options (wafer sort rules)
    bool readReticleMultiRule(QJsonObject reticle, COptionsPat &lPATRecipeOptions);
    /// \brief Read wafer sort Reticle rules from json to COptionsPat options (wafer sort rules)
    bool readReticleSingleRule(QJsonObject jsonRule, COptionsPat &lPATRecipeOptions);
    /// \brief Convert final test rules from json to COptionsPat options (final test)
    bool readFinalTest(QJsonObject finalTest, COptionsPat &lPATRecipeOptions);
    /// \brief Convert smart rules from json to COptionsPat options (settings)
    bool readSettings(QJsonObject settings, COptionsPat &lPATRecipeOptions);
    /// \brief Convert recipe multivariate part from json to QList
    bool readMultivariateRules(QJsonArray multivariateRules, QList<PATMultiVariateRule>& lPATMultivariate);

    /// \brief write recipe options
    bool WriteOptions(COptionsPat& options, QJsonObject &lOptions);
    ///\ write smart rules in the output json object
    QJsonObject WriteSmartRules(COptionsPat& aPATOptions);
    ///\ write wafer sort in the output json object
    QJsonObject WriteWaferSort(COptionsPat& options);
    ///\ write final test in the output json object
    QJsonObject WriteFinalTest(COptionsPat& options);
    ///\ write setting in the output json object
    QJsonObject WriteSettings(COptionsPat& options);

    ///\ write univariate in json object
    bool WriteUnivariates(QHash<QString, CPatDefinition *> & tests, QJsonObject& univariates);
    ///\ write multivariate in json object
    bool WriteMultivariates(QList<PATMultiVariateRule>& tests, QJsonObject& multivariates);

    ///\ Converts a JsonArray containing a bin list to a string with semi-colons as separator
    QString      JsonArrayBinsListToString(QJsonArray bins, const QString &defaultBins);
    ///\ Converts a String containing a bin list with semi-colons as separator to a JsonArray
    QJsonArray   StringBinsListToJsonArray(const QString& bins);

    ///\ Converts a JsonArray containing RGB colot to QColor object
    QColor      JsonArrayRGBToColor(QJsonArray colors, const QColor& defaultColor);
    ///\ Converts a QColor to a JsonArray
    QJsonArray  ColorRGBToJsonArray(const QColor& color);

private:
    Q_DISABLE_COPY(PATRecipeIOJson)
};

}   // namespace Gex
}   // namespace GS
#endif // PATRECIPEIOJSON
