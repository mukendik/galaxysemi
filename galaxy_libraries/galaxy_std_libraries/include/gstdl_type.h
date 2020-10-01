/* ----------------------------------------------------------------------------------------------------------*/
/* gstdtype.h                                                                                                */
/* ----------------------------------------------------------------------------------------------------------*/
/*                                                                                                           */
/* Purpose: Define standard types for all libraries and client software.                                     */
/*                                                                                                           */
/* ----------------------------------------------------------------------------------------------------------*/

/// TO BE REVIEWED :
// define galaxy standard types
// 20/09/2011, PYC, HT


#ifndef _GALAXY_STANDARD_TYPE__HEADERS_
#define _GALAXY_STANDARD_TYPE__HEADERS_

#include <stdint.h>

#ifdef _WIN32
    #include<windows.h>
#endif

#if (!defined(__STDC__) && !defined(ANSI) && !defined(_WIN32))
#define const
#endif

//#if defined(_WINDOWS_H) || defined(_WINDOWS_)
//typedef unsigned char	BYTEFLAG;
//#endif

/* If Windows.h has not been included */
#if !defined(_WINDOWS_H) && !defined(_WINDOWS_)
typedef void			VOID;
typedef void*			PVOID;
typedef void*			LPVOID;
typedef char			CHAR;
typedef unsigned char	BYTE;
//typedef unsigned char	BYTEFLAG;
//typedef unsigned short	WORD;
//typedef unsigned long	DWORD;
typedef uint16_t    	WORD;
typedef uint32_t    	DWORD;
typedef unsigned int	UINT;
typedef short			SHORT;
typedef int				INT;
typedef int				INT32;
#if defined(_WIN32)
#ifdef __GNUC__
typedef long long               INT64;
#else
typedef __int64			INT64;
#endif
#endif
typedef int				BOOL;
typedef long			LONG;
typedef float			FLOAT;
typedef char*			PSTR;
typedef char*			LPSTR;
typedef const char*		PCSTR;

#if !defined(TRUE)
#define	TRUE			1
#endif
#if !defined(FALSE)
#define FALSE			0
#endif

#if !defined(NULL)
#define NULL			0
#endif

#endif /* _WINDOWS_ */

typedef double			DOUBLE;

typedef unsigned char	BYTEFLAG;
typedef unsigned char   UCHAR;
typedef DWORD           ULONG;



#endif /* _GALAXY_STANDARD_TYPE__HEADERS_ */


