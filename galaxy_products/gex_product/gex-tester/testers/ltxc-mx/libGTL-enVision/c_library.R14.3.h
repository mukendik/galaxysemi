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


    //This is structure for STRING inside TC

    #define MAX_STR_LEN 255
    typedef struct
        {
        unsigned char maxlen ;               /* maximum string length in bytes */
        unsigned char len ;                  /* actual string length in bytes */
        char txt [MAX_STR_LEN+1] ;  /* String data.  Will always contain Null terminator*/
	                            /* Library should NOT null terminate */
        }  STRING ;

#define LTX_MAX_C_PARMS 20
typedef unsigned short CADENCE_BOOLEAN; //This is what boolean is defined as in Cadence
                                        //Use this definition in library functions

/* Following are bit definitions used in parameters flag word*/
#define LTX_IS_QSAFE     0x2000

#define LTX_IS_ARRAY     0x0100
#define LTX_IS_VALUE     0x0200//Set if constant specified in addition to one of the type bits below
#define LTX_IS_POINTER   0x0400

#define LTX_IS_BOOLEAN   0x0001  //16-bits
#define LTX_IS_INTEGER   0x0002  //Signed 32-bits
#define LTX_IS_SHORT     0x0004  //Signed 16-bits
#define LTX_IS_ST_WORD   0x0008  //Unsigned 16-bits
#define LTX_IS_UNSIGNED  0x0010  //Unsigned 32-bits
#define LTX_IS_FLOAT     0x0020
#define LTX_IS_DOUBLE    0x0040
#define LTX_IS_STRING    0x0080

/* Following flag is set by C library function if an array or pointer is passed 
 * to it to indicate if it used address for input data or return data
 */
#define LTX_RETURN_DATA 0x1000

typedef struct parameters LTX_C_PARMS;
struct parameters
{
    unsigned short flag;
    unsigned long size; /* contains array size in bytes*/
    union{ /* no endian issue */
        void            * ptr;
        CADENCE_BOOLEAN * bptr;
        float           * fptr;
        double          * dptr;
        int             * iptr;
        unsigned long   * lptr; //Cadence lword
        short * sptr;
        unsigned short  * wptr; //Cadence Word
        STRING  *strptr; //See STRING definition above
    }p; 
//Support for old libraries
    unsigned short w_const;
    unsigned long  l_const;
    float f_const;
    double d_const;

//Never implemented beyond this. Replace with data union below
#ifdef OBSOLETE
  union{  /* no endian issue */
    long lcon;
  }c;
#endif 
 
//New recommended data format
  union{
    float           f_data;
    double          d_data;
    int             i_data;
    short           s_data;
    unsigned short  w_data;  //Cadence word
    unsigned long   lw_data;  //Cadence lword
    CADENCE_BOOLEAN b_data;
  }data;
};

struct cad_c_struct
{
    int temp;
    LTX_C_PARMS param[LTX_MAX_C_PARMS];
};
typedef struct cad_c_struct LTX_CADENCE_C_STRUCT;


#endif


