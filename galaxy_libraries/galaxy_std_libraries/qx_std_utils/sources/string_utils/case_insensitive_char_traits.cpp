#include "case_insensitive_char_traits.h"

#include <cctype>

namespace Qx
{

bool CaseInsensitiveCharTraits::eq( char c1, char c2 )
{
    return std::toupper( c1 ) == std::toupper( c2 );
}

bool CaseInsensitiveCharTraits::lt( char c1, char c2 )
{
     return std::toupper( c1 ) <  std::toupper( c2 );
}

int CaseInsensitiveCharTraits::compare( const char *s1, const char *s2, size_t n )
{
    while ( n-- != 0 )
    {
        if ( lt( *s1, *s2 ) ) return -1;
        if ( lt( *s2, *s1 ) ) return 1;
        ++s1; ++s2;
    }

    return 0;
}

const char * CaseInsensitiveCharTraits::find(const char* s, int n, char a)
{
    const char ua = std::toupper( a );

    while ( n-- != 0 )
    {
        if ( std::toupper( *s ) == ua )
            return s;
        s++;
    }

    return NULL;
}
}