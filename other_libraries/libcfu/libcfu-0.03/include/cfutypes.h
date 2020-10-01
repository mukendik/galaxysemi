/* Creation date: 2005-07-30 16:44:11
 * Authors: Don
 * Change log:
 */

#ifndef _CFU_TYPES_H_
#define _CFU_TYPES_H_

#include <sys/types.h>

#ifndef u_int32_t // mingw does not seem to define this one
    // #warning u_int32_t not defined in sys/types.h
    #include <stdint.h> // does not contain any def of u_int32 but has a uint32_t
    typedef uint32_t u_int32_t;
#endif

#ifndef u_int
    #include <stdint.h>
    typedef unsigned u_int; // under mingw, unsigned is a uint_least32_t;
#endif

/* u_int is now supposed to be defined */

#endif
