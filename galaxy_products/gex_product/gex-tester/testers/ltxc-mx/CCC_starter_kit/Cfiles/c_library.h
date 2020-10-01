/*******************************************************************************
 *             Copyright 2003, LTX Corporation, Westwood, MA
 *
 * Author: Bob Nardi
 * c_library.h  -- definitions needed for external C libraries
 *                 to interface with call_c_library bif
 * 
 *******************************************************************************
 */

#ifndef C_LIBRARY_H
#define C_LIBRARY_H

#define LTX_MAX_C_PARMS 20

/* Following are bit definitions used in parameters flag word*/
#define LTX_IS_ARRAY 0x0100
#define LTX_IS_VALUE     0x0200
#define LTX_IS_POINTER   0x0400

#define LTX_IS_INTEGER   0x0002
#define LTX_IS_ST_WORD   0x0008
#define LTX_IS_UNSIGNED  0x0010
#define LTX_IS_FLOAT     0x0020
#define LTX_IS_DOUBLE    0x0040

/* Following flag is set by C library function if an array or pointer is passed 
 * to it to indicate if it used address for input data or return data
 */
#define LTX_RETURN_DATA 0x1000

typedef struct parameters LTX_C_PARMS;
struct parameters
{
    unsigned short flag;
    unsigned long size; /* contains array size in bytes*/
    union{
        void * ptr;
        float * fptr;
        double * dptr;
        int *    iptr;
        long *   lptr;
        short * sptr;
    }p; 
    unsigned short w_const;
    unsigned long  l_const;
    float f_const;
    double d_const;

  union{
    long lcon;
  }c;
};

struct cad_c_struct
{
    int temp;
    LTX_C_PARMS param[LTX_MAX_C_PARMS];
};
typedef struct cad_c_struct LTX_CADENCE_C_STRUCT;

#endif


