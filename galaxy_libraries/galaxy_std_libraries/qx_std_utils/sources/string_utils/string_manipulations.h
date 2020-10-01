#ifndef STRING_MANIPULATIONS_H
#define STRING_MANIPULATIONS_H

#include "qx_std_utils_api.h"

#include <string>
#include <sstream>

namespace Qx
{

struct QX_STD_UTILS_API_DECL StringManipulations
{
    static std::string GetSpaceTrimmedString( const std::string &aString );
    static std::string UniqueConsecutiveWhitespaces( const std::string &aString);
    static std::string ToLowerCase( const std::string &aString );
    static std::string ToUpperCase( const std::string &aString );

    struct BadStringConversion {};

    template< class T >
        static T GetValueFromString( const std::string &aString )
        {
            std::stringstream lStream;
            T lValue;

            lStream << aString;
            lStream >> lValue;

            if( ! lStream )
                throw BadStringConversion();

            return lValue;
        }

    template< std::size_t N >
    static bool StringContains( const std::string &aString, const char ( &aPattern )[ N ] )
    {
      const char * const lPatternBegin = &aPattern[ 0 ];
      const char * const lPatternEnd = &aPattern[ sizeof( aPattern ) - 1 ];

      return std::search( aString.begin(), aString.end(), lPatternBegin, lPatternEnd ) != aString.end();
    }

    template< std::size_t N >
    static bool StringStartsWith( const std::string &aString, const char ( &aPattern )[ N ] )
    {
      const char * const lPatternBegin = &aPattern[ 0 ];
      const char * const lPatternEnd = &aPattern[ sizeof( aPattern ) - 1 ];

      return std::search( aString.begin(), aString.end(), lPatternBegin, lPatternEnd ) == aString.begin();
    }
};

}

#endif // STRING_MANIPULATIONS_H
