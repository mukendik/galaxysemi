#ifndef PARSERABSTRACT_H
#define PARSERABSTRACT_H

#include "gqtlParserGlobal.h"
#include "progressHandlerAbstract.h"
#include <QString>
#include <QDate>
#include <string>
#include <list>


namespace GS
{
namespace Parser
{

class /*GQTL_PARSERSHARED_EXPORT*/ ParserAbstract
{
public:

    /**
     * \fn ParserAbstract()
     * \brief Constructor
     */
    ParserAbstract() {}

    /**
     * \fn ~ParserAbstract()
     * \brief Destructor
     */
    virtual ~ParserAbstract() {}



    /**
     * \fn IsCompatibleMapUpdater Convert()
     * \return Return true if the parser is compatoble with the map updater. (Default = false)
     */
    virtual bool IsCompatibleMapUpdater() const = 0;


    /**
     * \fn ConverterStatus Convert()
     * \brief Convert the input file from the input type to the stdf format.
     * \param fileName     input file
     *        stdfFileName outpu stdf file
     * \return Return the status of the convertion
     */
    virtual ConverterStatus         Convert(const std::string &fileName, std::string &stdfFileName) = 0;

    /**
     * \brief Getter
     */
    virtual ParserType                      GetType() const = 0;
    virtual std::string                     GetName() const = 0;
    virtual const std::list<std::string>&   GetExtensions() const = 0;
    virtual bool                            IsCompressedFormat() const = 0;

    virtual std::list<std::string>          GetListStdfFiles() const = 0;

    /**
     * \fn void SetProgressHandler(ProgressHandlerAbstract* progress)
     * \brief Setter of the progress into the member parameter
     */
    virtual void SetProgressHandler(ProgressHandlerAbstract* progress) = 0;

    virtual void SetParameterFolder(const std::string& parameterFolder) = 0;

    /**
     * \fn int GetLastError(std::string & ErrorMsg)
     * \brief return the last error code and message.
     * \return last error code.
     */
    virtual int     GetLastError(std::string & ErrorMsg) const = 0;

    protected :

    virtual     bool ConvertoStdf(const QString&, QString&) = 0;
};

}
}

#endif // PARSERABSTRACT_H
