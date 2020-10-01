/******************************************************************************!
 * \file main.cpp
 * \brief parser
 ******************************************************************************/
#include <unistd.h>
#include <iostream>
#include <QTemporaryDir>
#include "parserFactory.h"
#include "parserAbstract.h"

#include "import_verigy_edl.h"



/******************************************************************************!
 * \fn OldConvertToStdf
 * \brief Convert the input file to the stdf format
 ******************************************************************************/
int OldConvertToStdf(const char* lInputFile, const char* lOutputFile)
{
    if (CGVERIGY_EDLtoSTDF::IsCompatible(lInputFile))
    {
        CGVERIGY_EDLtoSTDF EdlConvert;
        QStringList lStdfList(lOutputFile);
        if (EdlConvert.Convert(lInputFile, lStdfList, false, NULL) ==
            CGVERIGY_EDLtoSTDF::eConvertSuccess)
        {
            if (EdlConvert.GetLastErrorCode() ==
                CGVERIGY_EDLtoSTDF::errWarning)
            {
                std::cout << EdlConvert.GetLastError().toUtf8().constData()
                          << std::endl;
            }
            return EXIT_SUCCESS;
        }
        std::cout << EdlConvert.GetLastError().toUtf8().constData()
                  << std::endl;
    }
    return EXIT_FAILURE;
}

/******************************************************************************!
 * \fn ConvertToStdf
 * \brief Convert the input file to the stdf format
 ******************************************************************************/
int ConvertToStdf(const char* lInputFile)
{
    const char* lOutputFile = "out.stdf";

    // Delete
    ::unlink(lOutputFile);

    // Factory
    GS::Parser::ParserFactory* lFactory =
        GS::Parser::ParserFactory::GetInstance();
    if (lFactory == NULL)
    {
        std::cout << "lFactory == NULL" << std::endl;
        return EXIT_FAILURE;
    }

    // Parser
    GS::Parser::ParserAbstract* lParser = lFactory->CreateParser(lInputFile);
    if (lParser == NULL)
    {
        std::cout << "lParser == NULL" << std::endl;
        GS::Parser::ParserFactory::ReleaseInstance();
        return OldConvertToStdf(lInputFile, lOutputFile);
    }
    lParser->SetProgressHandler(NULL);

    // Create a temporary directory to store parameters definition
    QTemporaryDir lTempDir;
    lParser->SetParameterFolder(lTempDir.path().toStdString());

    // Convert
    GS::Parser::ConverterStatus lStatus =
        lParser->Convert(lInputFile, lOutputFile, 2014, 12, 10);

    // Release
    GS::Parser::ParserFactory::ReleaseInstance();

    // Return
    return (lStatus == GS::Parser::ConvertSuccess) ?
        EXIT_SUCCESS : EXIT_FAILURE;
}

/******************************************************************************!
 * \fn main
 ******************************************************************************/
int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <infile>" << std::endl;
        return EXIT_FAILURE;
    }

    if (ConvertToStdf(argv[1]) != EXIT_SUCCESS)
    {
        std::cout << "Failure" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Success" << std::endl;
    return EXIT_SUCCESS;
}
