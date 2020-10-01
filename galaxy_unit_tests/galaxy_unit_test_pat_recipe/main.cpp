//#include <stdlib.h>
#include <stdio.h>
#include <QString>
#include <QFile>

#include "pat_recipe_io.h"
#include "pat_recipe.h"

int main(int argc, char** argv)
{
    printf("main: %d args: %s\n", argc, argv[0]?argv[0]:"?");

    // Read a correct recipe
    QString ErrorMsg("");
    QFile file;
    QString lInputFile("correct_recipe.json");
    file.setFileName(lInputFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ErrorMsg = "Error : File " + lInputFile + " doesn't exist or error opening it";
        printf("Error : File %s doesn't exist or error opening it \n", lInputFile.toLatin1().constData());
        return EXIT_FAILURE;
    }
    file.close();

    // Create recipe IO from file
    GS::Gex::PATRecipeIO* lPatRecipeIO = GS::Gex::PATRecipeIO::CreateRecipeIo(lInputFile);

    if(lPatRecipeIO == NULL)
    {
        printf("Error : The input file %s is not in supported formats (csv/xml or JSON)\n",
               lInputFile.toLatin1().constData());
        return EXIT_FAILURE;
    }
    GS::Gex::PATRecipe lPatRecipe;
    bool status =lPatRecipeIO->Read(lPatRecipe);
    ErrorMsg = lPatRecipeIO->GetErrorMessage();
    delete lPatRecipeIO;
    lPatRecipeIO = NULL;

    if (status != true && ErrorMsg != "")
    {
        printf("%s \n", ErrorMsg.toLatin1().constData());
        return EXIT_FAILURE;
    }


    ////////////////////////////////////////////////////////////////////////////:::
    ///         read a wrong recipe
    lInputFile = "wrong_recipe.json";
    file.setFileName(lInputFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ErrorMsg = "Error : File " + lInputFile + " doesn't exist or error opening it";
        printf("Error : File %s doesn't exist or error opening it \n", lInputFile.toLatin1().constData());
        return EXIT_FAILURE;
    }
    file.close();

    // Create recipe IO from file
    lPatRecipeIO = GS::Gex::PATRecipeIO::CreateRecipeIo(lInputFile);

    if(lPatRecipeIO == NULL)
    {
        printf("Error : The input file %s is not in supported formats (csv/xml or JSON)\n",
               lInputFile.toLatin1().constData());
        return EXIT_FAILURE;
    }

    status =lPatRecipeIO->Read(lPatRecipe);
    ErrorMsg = lPatRecipeIO->GetErrorMessage();
    delete lPatRecipeIO;
    lPatRecipeIO = NULL;

    if (status != true && ErrorMsg != "")
    {
        printf("The file is correctly incorrect ;). This is the error message %s \n", ErrorMsg.toLatin1().constData());
    }
    else
    {
        printf("The input file %s must have an error \n", lInputFile.toLatin1().constData());
        return EXIT_FAILURE;
    }

    printf("Success\n");
    return EXIT_SUCCESS;
}
