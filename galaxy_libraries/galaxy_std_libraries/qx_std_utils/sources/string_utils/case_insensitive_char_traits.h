#ifndef _CASE_INSENSITIVE_CHAR_TRAITS_H_
#define _CASE_INSENSITIVE_CHAR_TRAITS_H_

#include "qx_std_utils_api.h"

#include <string>

namespace Qx
{
/**
 * @brief Char traits class allowing the creation of a basic string that is case
 * insensitive
 */
struct CaseInsensitiveCharTraits : public std::char_traits< char >
{
    /**
     * @brief equality between 2 chars
     * @param c1 first char
     * @param c2 second char
     * @return true if equal, false otherwise
     */
    static QX_STD_UTILS_API_DECL bool eq( char c1, char c2 );

    /**
     * @brief comparison (less than) between 2 chars
     * @param c1 first char
     * @param c2 second char
     * @return true if first char is lesser than the second one, false otherwise
     */
    static bool lt( char c1, char c2 );

    /**
     * @brief comparison (lexicographical) between 2 strings
     * @param s1 first string
     * @param s2 second string
     * @return -1 if first string is lexicographically lesser than the
     * second one, 0 if the are equal, 1 if the first string is
     * lexicographically greater than the second one
     */
    static QX_STD_UTILS_API_DECL int compare(const char* s1, const char* s2, size_t n);

    /**
     * @brief find the first substring beginning with the specified char
     * @param s the string
     * @param n size of the string
     * @param a the char to find
     * @return a substring beginning by a, NULL if a is not found in the
     * original string
     */
    static QX_STD_UTILS_API_DECL const char * find(const char* s, int n, char a);
};
}

#endif // _CASE_INSENSITIVE_CHAR_TRAITS_H_
