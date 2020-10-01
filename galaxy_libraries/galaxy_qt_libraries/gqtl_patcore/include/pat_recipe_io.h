#ifndef PAT_RECIPE_IO_H
#define PAT_RECIPE_IO_H

#include <QString>
#include <QTextStream>
#include <QSet>
#include "pat_mv_rule.h"
class COptionsPat;
class CPatDefinition;

namespace GS
{
namespace Gex
{

class PATRecipe;

class PATRecipeIO
{
public:

    enum RecipeFormat
    {
        CSV,
        JSON
    };

    PATRecipeIO(PATRecipeIO::RecipeFormat lFormat);
    virtual ~PATRecipeIO();

    const QString&  GetErrorMessage() const;
    RecipeFormat    GetRecipeFormat() const;
    void            SetRecipeName(const QString& lRecipeName);
    void            SetReadOption(const QString& lOption);

    bool            HasReadOption(const QString& lOptionName) const;

    virtual bool    Write(PATRecipe &lPATRecipe, const QString& recipeName= "");
    virtual bool    Write(COptionsPat& lPatOptions, const QString &recipeName);

    bool            Read(PATRecipe& lPATRecipe);
    bool            Read(const QByteArray &buffer, PATRecipe& lPATRecipe);

    static PATRecipeIO* CreateRecipeIo(QTextStream & lRecipeStream);
    static PATRecipeIO* CreateRecipeIo(const QString &lRecipeFileName);
    static PATRecipeIO* CreateRecipeIo(const RecipeFormat type);

protected:

    void BuildPATBinsFailType(COptionsPat &lPATRecipe,
                              const QHash<QString, CPatDefinition *> &lPATDefinitions,
                              const QList<PATMultiVariateRule> &lPATMultivariate);

    virtual bool    Read(QTextStream & lRecipeStream,
                         PATRecipe& lPATRecipe) = 0;

    QString         mRecipeName;
    QString         mErrorMessage;
    QSet<QString>   mReadOptions;

private:
    Q_DISABLE_COPY(PATRecipeIO)

    RecipeFormat    mFormat;

};

}   // namespace Gex
}   // namespace GS

#endif // PAT_RECIPE_IO_H
