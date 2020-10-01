#ifndef PROMIS_INTERPRETER_BASE_H
#define PROMIS_INTERPRETER_BASE_H

#include "bin_mapping_api.h"
#include "tabular_file_line_fields.h"

#include <utility>
#include <string>

namespace Qx
{
namespace BinMapping
{

struct PromisItemBase;

class QX_BIN_MAPPING_API_DECL PromisInterpreterBase
{
public :
    typedef const PromisItemBase *PromisItemPointer;
    typedef const PromisItemBase &PromisItemReference;

public :
    virtual ~PromisInterpreterBase();

    virtual PromisItemReference GetPromisItem() const = 0;

protected :
    PromisInterpreterBase(const std::string &aKey, const std::string &mPromisFilePath, const std::string &aConverterExternalFilePath );
    PromisInterpreterBase( const PromisInterpreterBase & );
    PromisInterpreterBase & operator =( const PromisInterpreterBase & );

    void TrySetPromisItemWithStream(std::ifstream &aFileStream );
    bool GetNextTrimmedLineFromStream(std::ifstream &aFileStream, std::string &aOutputReadLine ) const;
    virtual void ProcessReadLine( const std::string &aLine ) = 0;
    bool ReadAndTrimLineFromStream(std::ifstream &aFileStream, std::string &aOutputReadLine ) const;
    bool IsRelevantLine(const std::string &aLine) const;
    virtual bool IsComment(const std::string &aLine) const = 0;
    virtual bool IsHeader( const std::string &aLine ) const = 0;
    bool IsEmpty( const std::string &aLine ) const;

    template< typename ExtractionException >
        TabularFileLineFields ExtractNFieldsFromLine( const std::string &aLine,
                                                      unsigned int aFieldCount ) const
    {
        TabularFileLineFields lFields( aLine );

        if( lFields.Count() != aFieldCount )
            throw ExtractionException( aLine.c_str() );

        return lFields;
    }

    template< typename ExtractionException >
        TabularFileLineFields ExtractRangeOfFieldsFromLine( const std::string &aLine,
                                                            unsigned int aFieldCountLow,
                                                            unsigned int aFieldCountHigh ) const
    {
        TabularFileLineFields lFields( aLine );

        if( aFieldCountHigh < aFieldCountLow )
            std::swap( aFieldCountHigh, aFieldCountLow );

        if( ( lFields.Count() < aFieldCountLow ) && ( lFields.Count() > aFieldCountHigh ) )
            throw ExtractionException( aLine.c_str() );

        return lFields;
    }

protected :
    PromisItemPointer mPromisItem;
    std::string mKey;
    std::string mPromisFilePath;
    std::string mConverterExternalFilePath;
};

}
}

#endif // PROMIS_INTERPRETER_BASE_H
