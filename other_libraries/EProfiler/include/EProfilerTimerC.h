/***************************************************************************************************************
 *
 * Copyright (c) 2012, Eccam s.r.o.
 *
 * All rights reserved.
 *
 */

#ifndef __EPROFILER_TIMER_C_H__
#define __EPROFILER_TIMER_C_H__

// stdint.h workaround - MSVC compilers older than MSVC 2010 lack C99 stdint.h.
#if (!defined(_MSC_VER) || _MSC_VER > 1700)
    #include <stdint.h>
#else
    typedef unsigned __int64 uint64_t;
#endif // if (!defined(_MSC_VER) || _MSC_VER > 1700)

#include "EProfilerDllPublic.h"

#ifdef __cplusplus
extern "C"
{
#endif // ifdef __cplusplus

/** Invalid handle for Embedded Profiler timer. */
#define INVALID_EPROFILER_TIMER_HANDLE   ((void *) 0)

/** Type of handle for Embedded Profiler timer. */
typedef void* TEProfilerTimerHandle;

/**
 * Creates handle for Embedded Profiler timer.
 *
 * \return Handle for Embedded Profiler timer or INVALID_EPROFILER_TIMER_HANDLE in case of error.
 */
DLL_PUBLIC TEProfilerTimerHandle EProfilerCreateTimerHandle();

/**
 * Destroys handle for Embedded Profiler timer.
 *
 * \param[in] handle Handle for Embedded Profiler timer to destroy.
 */
DLL_PUBLIC void EProfilerDestroyTimerHandle(TEProfilerTimerHandle handle);

/**
 * Starts internal profiler timer to measure time in processor cycles.
 *
 * \param[in] handle Handle for Embedded Profiler timer.
 */
DLL_PUBLIC void EProfilerTimerStart(TEProfilerTimerHandle handle);

/**
 * Stops internal profiler timer.
 *
 * \param[in] handle Handle for Embedded Profiler timer.
 *
 * \return Number of elapsed processor cycles from last calling of Start() method.
 */
DLL_PUBLIC uint64_t EProfilerTimerStop(TEProfilerTimerHandle handle);

/**
 * Gets number of processor cycles per one second.
 *
 * This value can be used for calculation of elapsed time from processor cycles to time. Be aware that
 * this value will be not valid if processor frequency is not fixed during whole measuring.
 *
 * \return Number of processor cycles per one second.
 */
DLL_PUBLIC uint64_t EProfilerGetCyclesPerSecond();

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // __EPROFILER_TIMER_C_H__
