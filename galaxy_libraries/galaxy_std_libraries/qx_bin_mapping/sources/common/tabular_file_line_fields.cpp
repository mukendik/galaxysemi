#include "tabular_file_line_fields.h"

#include <iterator>

namespace Qx
{
namespace BinMapping
{

TabularFileLineFields::TabularFileLineFields(const std::string &aLine) :
    mLine( aLine )
{
    ExtractFields();
}

std::size_t TabularFileLineFields::Count() const
{
    return mFields.size();
}

void TabularFileLineFields::ExtractFields()
{
    typedef std::string::const_iterator iterator;
    const iterator lBegin = mLine.begin();
    const iterator lEnd = mLine.end();

    iterator lCurrent = lBegin;
    for( iterator delimiter = std::find( lBegin, lEnd, ',' );
         delimiter != lEnd;
         lCurrent = ++delimiter,
         delimiter = std::find( delimiter, lEnd, ',' ) )
        mFields.push_back( MakeFieldWithoutEndOfLine( lCurrent, delimiter ) );

    mFields.push_back( MakeFieldWithoutEndOfLine( lCurrent, lEnd ) );
}

void TabularFileLineFields::CheckFieldIndex(std::size_t aFieldIndex) const
{
    if( aFieldIndex >= Count() )
        throw FieldIndexOutOfRange();
}

std::string TabularFileLineFields::MakeFieldWithoutEndOfLine( std::string::const_iterator aBegin,
                                                                  std::string::const_iterator aEnd ) const
{
    std::string lResult( aBegin, aEnd );

    lResult.erase( std::remove( lResult.begin(), lResult.end(), '\n' ), lResult.end() );
    lResult.erase( std::remove( lResult.begin(), lResult.end(), '\r' ), lResult.end() );

    return lResult;
}

}
}