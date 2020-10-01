#ifndef SPFFTW_H
#define SPFFTW_H

#if !defined(FFTW_ENABLE_FLOAT)
#define     FFTW_ENABLE_FLOAT
#endif

#undef      FFTW_NS

#include    "fftw.h"
#include    "rfftw.h"

#undef      NS
#if defined(FFTW_NS)
#  define NS FFTW_NS
#endif

#include    "fftw_mgr.h"

#undef      FFTW_ENABLE_FLOAT

#endif
