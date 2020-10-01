#ifndef PAT_RECIPE_IO_CSV_H
#define PAT_RECIPE_IO_CSV_H
#include <QString>
#include <QTextStream>
#include "pat_recipe_io.h"

class COptionsPat;
class CPatDefinition;

namespace GS
{
namespace Gex
{

class PATRecipe;
class PATRecipeIOCsv : public PATRecipeIO
{
public:

    PATRecipeIOCsv();
    ~PATRecipeIOCsv();

    /// \brief Write the recipe in the file
    /// \param lPATRecipe contains the recipe informations
    ///        recipeName Contains the name of the file in witch the recipe will be wrote
    bool            Write(PATRecipe &lPATRecipe, const QString& recipeName);

    bool            Write(COptionsPat& lPatOptions, const QString& recipeName);

protected:
    bool            Read(QTextStream &lRecipeStream, PATRecipe& lPATRecipe);
    bool            ReadOptions(QTextStream &lRecipeStream, COptionsPat& lPATRecipe);
    bool            ReadRules(QTextStream &lRecipeStream,
                              COptionsPat& lPATRecipe,
                              QHash<QString, CPatDefinition *>& lPATDefinitions);
    void            UpdateGeographicalRules(COptionsPat &lPATRecipe);

private:
    Q_DISABLE_COPY(PATRecipeIOCsv)
    int         mCurrentLine;
};

}   // namespace Gex
}   // namespace GS


#endif // PAT_RECIPE_IO_CSV_H
