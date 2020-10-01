/* ----------------------------------------------------------------------------------------------------------*/
/* gstdmacro.h                                                                                               */
/* ----------------------------------------------------------------------------------------------------------*/
/*                                                                                                           */
/* Purpose: Define standard types for all libraries and client software.                                     */
/*                                                                                                           */
/* ----------------------------------------------------------------------------------------------------------*/

#ifndef _GALAXY_STANDARD_MACRO_HEADERS_
#define _GALAXY_STANDARD_MACRO_HEADERS_

/* ----------------------------------------------------------------------------------------------------------*/
/* Assertion macros                                                                                          */
#if !defined(ASSERT)
#include <assert.h>

#if defined(_DEBUG) || defined(DEBUG)

#define ASSERT(f)		assert(f)
#define VERIFY(f)		ASSERT(f)

#else /* defined(_DEBUG) || defined(DEBUG) */

#define ASSERT(f)		((void)0)
#define VERIFY(f)		((void)(f))

#endif /* defined(_DEBUG) || defined(DEBUG) */
#endif /* !defined(ASSERT) */

// ----------------------------------------------------------------------------------------------------------
/* Byte,word and long manipulation */
#if !defined(_WINDOWS_H) && !defined(_WINDOWS_)

#define MAKEWORD(bLow, bHigh)      ((WORD)(((BYTE)(bLow)) | ((WORD)((BYTE)(bHigh))) << 8))
#define MAKELONG(bLow, bHigh)      ((LONG)(((WORD)(bLow)) | ((DWORD)((WORD)(bHigh))) << 16))
#define LOWORD(l)				   ((WORD)(l))
#define HIWORD(l)				   ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)				   ((BYTE)(w))
#define HIBYTE(w)				   ((BYTE)(((WORD)(w) >> 8) & 0xFF))

#endif /* !defined(_WINDOWS_H) */

/* ----------------------------------------------------------------------------------------------------------*/
/* Inline definition                                                                                         */
#if defined(DEBUG) || defined(_DEBUG)
#define GINLINE
#else
#define GINLINE		inline
#endif


#define GMULTI8(x)		   ( (x)<<3 )
#define GMULTI10(x)        ( (((x)<<2) + (x))<<1 )	/* Multiply x by ten */
#define GMULTI16(x)		   ( (x)<<4 )

#endif /* _GALAXY_STANDARD_MACRO_HEADERS_ */
