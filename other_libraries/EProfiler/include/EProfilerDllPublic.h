/***************************************************************************************************************
 *
 * Copyright (c) 2012, Eccam s.r.o.
 *
 * All rights reserved.
 *
 */

#ifndef __EPROFILER_DLL_PUBLIC_H__
#define __EPROFILER_DLL_PUBLIC_H__

// DLL_PUBLIC definition according to the platform
#ifdef _WIN32
    #ifdef _BUILDING_DLL
        #define DLL_PUBLIC __declspec(dllexport)
    #else
        #define DLL_PUBLIC __declspec(dllimport)
    #endif
#else
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

#endif // __EPROFILER_DLL_PUBLIC_H__
