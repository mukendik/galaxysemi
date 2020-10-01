#ifndef DPFFTW_H
#define DPFFTW_H

#undef   FFTW_NS

#include "fftw.h"
#include "rfftw.h"

#undef NS
#if defined( FFTW_NS )
#define NS FFTW_NS
#endif

#include "fftw_mgr.h"

#endif
