/*  ltx_std.h    Standard header for LTX Corporation   
 *  
 *  CONFIDENTIAL and PROPRIETARY INFORMATION
 *  OF LTX CORPORATION, LTX PARK, WESTWOOD MA. 02090
 *
 *  Revision History:
 */

#ifndef LTX_STD_H
#define LTX_STD_H 


#ifdef __cplusplus
#define VOID void 
#define _LTX_PROTOTYPES
#define _LTX_CXX
#else
#    ifdef _cplusplus
#       define VOID void  
#       define _LTX_PROTOTYPES 
#       define _LTX_CXX
#    else
#       ifdef __STDC__
#           define VOID void 
#           define _LTX_PROTOTYPES
#       else
#           ifdef c_plusplus
#               define _LTX_PROTOTYPES  
#               define _LTX_CXX 
#               define VOID void 
#           else
                typedef int VOID;    /* The old Whitesmith's typedef */
#           endif
#       endif
#    endif
#endif

#ifdef _CCXX_COMPILER
#undef _LTX_CXX
#endif

#define STS_OK 0
#define TC_OUT_OF_MEMORY 1 //BOB
#define MATH_BAD_FFT_LEN 2 //BOB

#ifdef _LTX_PROTOTYPES
        /* allow compilation of non ansi from ansi source */
#   ifndef NOT_ANSI
#       define ANSI      
#   endif
#endif

/*  Required for C++ V2.0  */
#ifdef  _LTX_CXX
    extern "C" {
#endif

/*  storage classes 
 */

typedef unsigned long LWORD;



#ifdef  _LTX_CXX
    }
#endif




#endif /* ltx_std_h */

