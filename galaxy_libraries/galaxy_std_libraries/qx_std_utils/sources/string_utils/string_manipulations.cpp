#include "string_manipulations.h"

#include <locale>
#include <iterator>

namespace
{
struct IsSpace
{
    bool operator()( char aLeftChar, char aRightChar ) const
    { return std::isspace( aLeftChar, std::locale() ) && std::isspace( aRightChar, std::locale() ); }
};
}

namespace Qx
{

std::string StringManipulations::GetSpaceTrimmedString(const std::string &aString)
{
    typedef std::string::const_iterator iterator;

    iterator lIterator = aString.begin(), lEnd = aString.end();
    for( ; lIterator != lEnd && *lIterator == ' '; ++lIterator );

    typedef std::reverse_iterator< iterator > reverse_iterator;

    reverse_iterator lReverseIterator = aString.rbegin(), lReverseEnd = reverse_iterator( lIterator );
    for( ; lReverseIterator != lReverseEnd && *lReverseIterator == ' '; ++lReverseIterator );

    return std::string( lIterator, lReverseIterator.base() );
}

std::string StringManipulations::UniqueConsecutiveWhitespaces(const std::string &aString)
{
    std::string lString = aString;
    std::string::iterator lNewEnd = std::unique(lString.begin(), lString.end(), IsSpace() );

    lString.erase( lNewEnd, lString.end() );
    return lString;
}

std::string StringManipulations::ToLowerCase(const std::string &aString)
{
    std::string lCopy = aString;
    for( std::string::iterator lIterator = lCopy.begin(), lEnd = lCopy.end(); lIterator != lEnd; ++ lIterator )
      ( *lIterator ) = std::tolower( ( *lIterator ), std::locale() );

    return lCopy;
}

std::string StringManipulations::ToUpperCase(const std::string &aString)
{
    std::string lCopy = aString;
    for( std::string::iterator lIterator = lCopy.begin(), lEnd = lCopy.end(); lIterator != lEnd; ++ lIterator )
      ( *lIterator ) = std::toupper( ( *lIterator ), std::locale() );

    return lCopy;
}

}
