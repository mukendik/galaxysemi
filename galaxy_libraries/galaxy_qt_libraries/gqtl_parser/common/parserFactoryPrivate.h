#ifndef PARSERFACTORYPRIVATE_H
#define PARSERFACTORYPRIVATE_H

#include "gqtlParserGlobal.h"
#include <QHash>

namespace GS
{
namespace Parser
{

class ParserAbstract;

class ParserFactoryItemBase
{
public:
    ParserFactoryItemBase() {}
    virtual ~ParserFactoryItemBase() {}

    virtual ParserAbstract *    Create() = 0;
    virtual bool                IsCompatible(const QString& lFileName) = 0;
    virtual ParserType          GetType() const = 0;
    virtual bool                IsCompressedFormat() const = 0;
};

template <class TParser>
class ParserFactoryItem : public ParserFactoryItemBase
{
public:
    /**
     * \fn ParserFactoryItemBase()
     * \brief Constructor
     */
    ParserFactoryItem() : ParserFactoryItemBase()
    {
    }

    /**
     * \fn ~ParserFactoryItemBase()
     * \brief Destructor
     */
    ~ParserFactoryItem()
    {
    }

    /**
     * \fn ParserAbstract* Create()
     * \brief Create a parser using the class passed in the template
     * \return A pointer to the created parser
     */
    ParserAbstract *  Create()
    {
        return new TParser();
    }

    /**
     * \fn bool IsCompatible(const QString& FileName)
     * \brief Check if the input file is compatible with the parser
     * \param FileName: input file
     * \return True if compatible, else false.
     */
    bool        IsCompatible(const QString& FileName)
    {
        return TParser::IsCompatible(FileName);
    }

    /**
     * \fn ParserType  GetType() const
     * \return The parser type
     */
    ParserType  GetType() const
    {
        TParser lParser;
        return lParser.GetType();
    }

    /**
     * \fn bool  IsCompressedFormat() const
     * \return Returns true if the data file supported by this parser is compressed
     */
    bool  IsCompressedFormat() const
    {
        TParser lParser;
        return lParser.IsCompressedFormat();
    }

};

class ParserFactoryPrivate
{
public:

    /**
     * \fn ParserFactoryPrivate()
     * \brief Constructor
     */
    ParserFactoryPrivate();
    /**
     * \fn ~ParserFactoryPrivate()
     * \brief Destructor
     */
    ~ParserFactoryPrivate();

    /**
     * \fn ParserAbstract * CreateParser(const QString& lFileName)
     * \brief Create a parser using the file name
     * \param lFileName: Input file name
     * \return The created parser
     */
    ParserAbstract *    CreateParser(const QString& lFileName);
    /**
     * \fn ParserAbstract * CreateParser(ParserType lType)
     * \brief Create a parser using the parser type
     * \param lType: Input parser type
     * \return The created parser
     */
    ParserAbstract *    CreateParser(ParserType lType);

    /**
     * \fn ParserType FindParserType(const QString& lFileName)
     * \brief   Find the parser type correspoding to the given file name
     * \param   lFileName: Input file name
     * \return  The parser type corresponding to the given file if supported, otherwise typeUnknown type
     */
    ParserType          FindParserType(const QString& lFileName);

    /**
     * \fn void Initialize()
     * \brief Initialize the parsers in the factory
     */
    void                Initialize();

    /**
     * \fn bool RegisterParser(ParserFactoryItemBase * lProperty)
     * \brief Add the parser to the list of parsers (mParsers)
     * \param lProperty: Input parser factory item
     * \return True if the parser has been correctly registred
     */
    bool                RegisterParser(ParserFactoryItemBase * lProperty);

    /**
     * \fn bool UnregisterParser(ParserType lType)
     * \brief Delete the parser from the list of parsers (mParsers)
     * \param lType: The type of the parser
     * \return True if the parser has been correctly unregistred
     */
    bool                UnregisterParser(ParserType lType);

private:

    /// \param List of parser type, parser
    QHash<ParserType, ParserFactoryItemBase * >  mParsers;
};

}
}

#endif // PARSERFACTORYPRIVATE_H
