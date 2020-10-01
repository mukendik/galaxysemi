/***************************************************************************************************************
 *
 * Copyright (c) 2012, Eccam s.r.o.
 *
 * All rights reserved.
 *
 */

#ifndef __EPROFILER_TIMER_H__
#define __EPROFILER_TIMER_H__

// stdint.h workaround - MSVC compilers older than MSVC 2010 lack C99 stdint.h.
#if (!defined(_MSC_VER) || _MSC_VER > 1700)
    #include <stdint.h>
#else
    typedef unsigned __int64 uint64_t;
#endif // if (!defined(_MSC_VER) || _MSC_VER > 1700)

#include "EProfilerDllPublic.h"

// forward declaration
class EProfilerTimerImpl;

/**
 * Class to handle internal profiler timer to measure wall time in processor cycles.
 */
class EProfilerTimer
{
public:
    /** Empty constructor. */
    DLL_PUBLIC EProfilerTimer();

    /** Destructor. */
    DLL_PUBLIC ~EProfilerTimer();

    /** Starts internal profiler timer to measure time in processor cycles. */
    DLL_PUBLIC void Start();

    /**
     * Stops internal profiler timer.
     *
     * \return Number of elapsed processor cycles from last calling of Start() method.
     */
    DLL_PUBLIC uint64_t Stop();

    /**
     * Gets number of processor cycles per one second.
     *
     * This value can be used for calculation of elapsed time from processor cycles to time. Be aware that
     * this value will be not valid if processor frequency is not fixed during whole measuring.
     *
     * \return Number of processor cycles per one second.
     */
    DLL_PUBLIC static uint64_t GetCyclesPerSecond();

private:
    // disable copy constructor and copy assignment operator
    EProfilerTimer(const EProfilerTimer&);
    EProfilerTimer& operator=(const EProfilerTimer&);

    EProfilerTimerImpl*     m_pImpl;        /**< Pointer to hidden internal implementation structure. */
};

#endif // __EPROFILER_TIMER_H__
