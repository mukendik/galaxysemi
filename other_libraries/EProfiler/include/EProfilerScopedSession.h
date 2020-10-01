/***************************************************************************************************************
 *
 * Copyright (c) 2012, Eccam s.r.o.
 *
 * All rights reserved.
 *
 */

#ifndef __EPROFILER_SCOPED_SESSION_H__
#define __EPROFILER_SCOPED_SESSION_H__

#include <stddef.h>
#include <stdint.h>

#include "EProfilerDllPublic.h"

/**
 * Embedded Profiler scoped session class.
 *
 * This class wraps embedded profiler EProfilerStart() and EProfilerStop() methods. It can call Start() in its
 * constructor and it calls Stop() in its destructor.
 */
class EProfilerScopedSession
{
public:
    /**
     * Constructor from starting flag.
     *
     * \param[in] start_flag True if profiling should be started in this constructor.
     */
    DLL_PUBLIC explicit EProfilerScopedSession(bool start_flag = true);

    /**
     * Constructor from profiler file name.
     *
     * \param[in] p_prof_file_name Pointer to profiler file name to use or NULL to use default file name.
     * \param[in] start_flag       True if profiling should be started in this constructor.
     */
    DLL_PUBLIC explicit EProfilerScopedSession(const char* p_prof_file_name, bool start_flag = true);

    /**
     * Constructor from profiler memory size and memory for profiler.
     *
     * \param[in] prof_memory_size Profiler memory size in bytes to use or 0 to use default memory size.
     * \param[in] p_prof_memory    Pointer to memory for profiler or NULL to allocate memory on heap.
     * \param[in] start_flag       True if profiling should be started in this constructor.
     */
    DLL_PUBLIC EProfilerScopedSession(uint32_t prof_memory_size, void* p_prof_memory, bool start_flag = true);

    /**
     * Constructor from profiler file name and profiler memory size and memory for profiler.
     *
     * \param[in] p_prof_file_name Pointer to profiler file name to use or NULL to use default file name.
     * \param[in] prof_memory_size Profiler memory size in bytes to use or 0 to use default memory size.
     * \param[in] p_prof_memory    Pointer to memory for profiler or NULL to allocate memory on heap.
     * \param[in] start_flag       True if profiling should be started in this constructor.
     */
    DLL_PUBLIC EProfilerScopedSession(const char* p_prof_file_name, uint32_t prof_memory_size,
                                      void* p_prof_memory, bool start_flag = true);

    /**
     * Destructor.
     *
     * This method stops profiling for the current thread if profiling is active.
     */
    DLL_PUBLIC ~EProfilerScopedSession();

    /**
     * Starts profiling for the current thread.
     */
    DLL_PUBLIC void Start();

    /**
     * Flushes profiling memory to the profiler file for the current thread.
     */
    DLL_PUBLIC void Flush();

    /**
     * Stops profiling for the current thread.
     */
    DLL_PUBLIC void Stop();

private:
    // disable copy constructor and copy assignment operator
    EProfilerScopedSession(const EProfilerScopedSession&);
    EProfilerScopedSession& operator=(const EProfilerScopedSession&);
};

#endif // __EPROFILER_SCOPED_SESSION_H__
