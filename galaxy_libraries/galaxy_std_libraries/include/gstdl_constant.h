/* ----------------------------------------------------------------------------------------------------------*/
/* gstdconstant.h                                                                                            */
/* ----------------------------------------------------------------------------------------------------------*/
/*                                                                                                           */
/* Purpose: Define standard constants for all libraries and client software.                                 */
/*                                                                                                           */
/* ----------------------------------------------------------------------------------------------------------*/

#ifndef _GALAXY_STANDARD_CONSTANT__HEADERS_
#define _GALAXY_STANDARD_CONSTANT__HEADERS_

#if defined(_WIN32)
#include <stdlib.h>
#define GMAX_PATH _MAX_PATH
#elif defined(__unix__) || __MACH__
#include <limits.h>
#if defined(PATH_MAX)
#define GMAX_PATH PATH_MAX
#else
#define GMAX_PATH 255
#endif
#endif

#endif /* _GALAXY_STANDARD_CONSTANT__HEADERS_ */
