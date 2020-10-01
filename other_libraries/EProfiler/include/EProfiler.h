/***************************************************************************************************************
 *
 * Copyright (c) 2012, Eccam s.r.o.
 *
 * All rights reserved.
 *
 */

#ifndef __EPROFILER_H__
#define __EPROFILER_H__

#include <stddef.h>
#include <stdint.h>

#include "EProfilerDllPublic.h"

#ifdef __cplusplus
extern "C"
{
#endif // ifdef __cplusplus

/** Enumeration for result codes from public embedded profiler functions. */
typedef enum
{
    EProfilerFailure = 0,      /**< Failure. */
    EProfilerSuccess = 1       /**< Success. */
} TEProfilerResult;

/**
 * Configure profiler file name for the current thread.
 *
 * This method can be called only before EProfilerStart() or after EProfilerStop(). If this method is not
 * called, profiler will use default file name.
 *
 * \param[in] p_prof_file_name Pointer to the profiler file name to use or NULL to use default file name.
 *
 * \retval EProfilerSuccess Success.
 * \retval EProfilerFailure Failure, the profiler has been already started.
 */
DLL_PUBLIC TEProfilerResult EProfilerConfigureFile(const char* p_prof_file_name);

/**
 * Configure profiler memory for the current thread.
 *
 * This method can be called only before EProfilerStart() or after EProfilerStop(). If this method is not
 * called, profiler memory will have default size and it will be allocated on heap.
 *
 * Default profiler memory size in call tree mode is 1MB (1048576 bytes).
 * Default profiler memory size in function list mode is 40KB (40960 bytes).
 *
 * \param[in] prof_memory_size Profiler memory size in bytes to use or 0 to use default memory size.
 * \param[in] p_prof_memory    Pointer to memory to use or NULL to allocate memory on heap.
 *
 * \retval EProfilerSuccess Success.
 * \retval EProfilerFailure Failure, the profiler has been already started or given memory size is too small.
 */
DLL_PUBLIC TEProfilerResult EProfilerConfigureMemory(uint32_t prof_memory_size, void* p_prof_memory);

/**
 * Starts profiling for the current thread.
 */
DLL_PUBLIC void EProfilerStart();

/**
 * Flushes profiling memory to the profiler file for the current thread.
 *
 * In call tree mode, this method stores entry and exit of virtual function 'InternalFlushing' to measure
 * consumed time by flushing. It can be called only between EProfilerStart() and EProfilerStop().
 */
DLL_PUBLIC void EProfilerFlush();

/**
 * Stops profiling for the current thread.
 */
DLL_PUBLIC void EProfilerStop();

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // __EPROFILER_H__
