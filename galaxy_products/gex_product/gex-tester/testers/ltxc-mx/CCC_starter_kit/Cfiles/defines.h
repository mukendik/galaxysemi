#ifndef DEFINES_H
#define DEFINES_H



#define STS_OK                  0
#define BAD_FFT_LEN             1
#define MALLOC_FAILED           2
#define ARRAY_NOT_MODULO2       3
#define ARRAY_SIZE_MISMATCH     4
#define ARRAY_SIZE_NOT_VALID    5
#define DEBUG_LOG_ERROR         6
#define FFT_ERR                 7
#define INVALID_STRING          8
#define STRING_SIZE_ERROR       9
#define LOG_OPEN_ERR            10
#define LOG_FILE_ALREADY_OPEN   11
#define LOG_CLOSE_ERR           12
#define FUNCTION_UNAVAILABLE    13


#define TRUE 1
#define false 0
#define NO 0
#define  LOG_2_MAX_FFT_LEN  ( 20 )
#define  MAX_FFT_LEN        ( 1 << LOG_2_MAX_FFT_LEN )
#define DEG_TO_RAD          ( M_PI / 180.0 )

#define MATH_IMPULSE 1
#define RETURN_TIME  1

typedef int boolean;
typedef unsigned long LWORD;
typedef long LCOUNT;
typedef float FLOAT;

#define MAXSTRLEN                  256 //This is maximum size of Cadence string variable
#endif
