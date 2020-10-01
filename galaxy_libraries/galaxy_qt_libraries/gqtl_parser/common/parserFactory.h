#ifndef PARSERFACTORY_H
#define PARSERFACTORY_H

#include "gqtlParserGlobal.h"

#include <string>

namespace GS
{
namespace Parser
{

class ParserAbstract;
class ParserFactoryPrivate;

class /*GQTL_PARSERSHARED_EXPORT*/ ParserFactory
{
public:

    /**
     * \fn ParserAbstract * CreateParser(ParserType lType)
     * \brief Creates a parser using the parser type
     * \param lType: Input parser type
     * \return The created parser
     */
    ParserAbstract * CreateParser(ParserType lType);

    /**
     * \fn ParserAbstract * CreateParser(const std::string &lFileName)
     * \brief Creates a parser using the file name
     * \param lFileName: Input file name
     * \return The created parser
     */
    ParserAbstract * CreateParser(const std::string &lFileName);

    /**
     * \fn ParserType FindParserType(const std::string& lFileName)
     * \brief   Find the parser type correspoding to the given file name
     * \param   lFileName: Input file name
     * \return  The parser type corresponding to the given file if supported, otherwise typeUnknown type
     */
    ParserType FindParserType(const std::string& lFileName);

    /**
     * \fn static ParserFactory *  GetInstance()
     * \brief Create an instance of the parser factory or just return the exist one
     * \return A parser factory
     */
    static ParserFactory *  GetInstance();

    /**
     * \fn static void ReleaseInstance()
     * \brief Static function to delete the instance
     */
    static void ReleaseInstance();

protected:

    /**
     * \fn void Initialize()
     * \brief Initialize the parsers in the factory
     */
    void Initialize();

private:

    /**
     * \fn ParserFactory()
     * \brief Constructor
     */
    ParserFactory();
    /**
     * \fn ~ParserFactory()
     * \brief Destructor
     */
    ~ParserFactory();


    static  ParserFactory * mInstance;  /// \param The instance of the parser factory
    ParserFactoryPrivate *  mPrivate;   /// \param A pointer to the private parser factory
};

}
}
#endif // PARSERFACTORY_H
