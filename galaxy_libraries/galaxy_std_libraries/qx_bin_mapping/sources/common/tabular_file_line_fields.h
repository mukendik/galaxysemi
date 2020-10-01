#ifndef BIN_MAPPING_FILE_LINE_FIELDS_H
#define BIN_MAPPING_FILE_LINE_FIELDS_H

#include "bin_mapping_api.h"
#include "string_manipulations.h"
#include "bin_mapping_exceptions.h"

#include <vector>
#include <string>
#include <sstream>

namespace Qx
{
namespace BinMapping
{

class TabularFileLineFields
{
public :
    explicit TabularFileLineFields( const std::string &aLine );
    std::size_t Count() const;

    template< typename ValueType >
    ValueType GetValueAt( std::size_t aFieldIndex ) const
    {
        CheckFieldIndex(aFieldIndex);

        return FieldValueExtractor< ValueType >( mFields ).GetValueAt( aFieldIndex );
    }

private :
    typedef std::vector< std::string > FieldContainer;

    template< typename ValueType, typename DummyType = void >
    class FieldValueExtractor
    {
    public :
        FieldValueExtractor( const FieldContainer &aFields ) : mFields( aFields ) {}

        ValueType GetValueAt( std::size_t aFieldIndex )
        {
            const std::string &lFieldStringValue = StringManipulations::GetSpaceTrimmedString( mFields[ aFieldIndex ] );

            if( lFieldStringValue.empty() )
                return ValueType();

            std::stringstream lSs;
            lSs << lFieldStringValue;

            ValueType lOutputValue;
            lSs >> lOutputValue;

            if( ! lSs )
                throw BadFieldFormat();

            return lOutputValue;
        }

    private :
        const FieldContainer &mFields;
    };

    template< typename DummyType >
    class FieldValueExtractor< std::string, DummyType >
    {
    public :
        FieldValueExtractor( const FieldContainer &aFields ) : mFields( aFields ) {}

        std::string GetValueAt( std::size_t aFieldIndex )
        {
            return StringManipulations::GetSpaceTrimmedString( mFields[ aFieldIndex ] );
        }

    private :
        const FieldContainer &mFields;
    };

private :
    void ExtractFields();
    void CheckFieldIndex( std::size_t aFieldIndex ) const;
    std::string MakeFieldWithoutEndOfLine( std::string::const_iterator aBegin,
                                            std::string::const_iterator aEnd ) const;

private :
    const std::string &mLine;
    FieldContainer mFields;
};

}
}

#endif // BIN_MAPPING_FILE_LINE_FIELDS_H
