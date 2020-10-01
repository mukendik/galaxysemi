#ifndef GS_TYPES_H
#define GS_TYPES_H

#include <stdint.h>
#include <float.h>

// to define if the compiler is greater than C++11, use #if __cplusplus > 201103L


// Define all int
typedef int8_t gsint8;
typedef int16_t gsint16;
typedef int32_t gsint32;
typedef int64_t gsint64;

// Define all unsigned int
typedef uint8_t gsuint8;
typedef uint16_t gsuint16;
typedef uint32_t gsuint32;
typedef uint64_t gsuint64;

// Define const int (min and max)
#define GS_MIN_INT8 -128
#define GS_MAX_INT8 127
#define GS_MIN_INT16 -32768
#define GS_MAX_INT16 32767
#define GS_MIN_INT32 -2147483648
#define GS_MAX_INT32 2147483647
#define GS_MIN_INT64 -9223372036854775808
#define GS_MAX_INT64 9223372036854775807

#define GS_MAX_UINT8 255U
#define GS_MAX_UINT16 65535U
#define GS_MAX_UINT32 4294967295U       // This value is used as an invalid value in the STDF
#define GS_MAX_UINT64 18446744073709551615U
#define GS_INFINITE_VALUE_DOUBLE		1.6e+308


typedef bool gsbool;
typedef int8_t gschar;
typedef uint8_t gsuchar;

typedef double gsfloat64;
typedef float gsfloat32;

/// Define time_t
// If windows
#ifdef _WIN32
#ifdef _WIN64
typedef int64_t gstime;
#else
typedef int32_t gstime;
#endif
#endif
// if linux
#if __GNUC__
#if __x86_64__ || __ppc64__
typedef int64_t gstime;
#else
typedef int32_t gstime;
#endif
#endif
typedef gstime gstime_t;


#endif // GS_TYPE_H
