/******************************************************************************
 *
 *  LTX Corporation (C) 2001
 *
 *  fftw_mgr.h
 *
 *  FFTW manager classes
 * 
 *  Revisions:
 *  
 *  2003-02-13  ll      Completely reorganized
 *  2001-09-20  ll      Created
 *
 ******************************************************************************/
#if ( defined(FFTW_ENABLE_FLOAT) && !defined(SP_FFTW_MGR_H )  ) || ( !defined(FFTW_ENABLE_FLOAT) && !defined(DP_FFTW_MGR_H) )

#if defined(FFTW_ENABLE_FLOAT)
#  define SP_FFTW_MGR_H
#else
#  define DP_FFTW_MGR_H
#endif

#include    <stdlib.h>
#include    "ltx_std.h"

#define FFTW_MANAGER_ARR_LEN    (32)

#if defined( NS )
#  define NS_BEGIN(NS) namespace NS {
#  define NS_END }
#  define NS_PFX(NS) NS::
#else
#  define NS_BEGIN(NS)
#  define NS_END
#  define NS_PFX(NS)
#endif
//
// #define FFTW_MGR_INTERFACE(NS)
 NS_BEGIN(NS) 
class FFTWManager
{
private:
   fftw_plan _forward_in_place_plan_arr[FFTW_MANAGER_ARR_LEN];
   fftw_plan _inverse_in_place_plan_arr[FFTW_MANAGER_ARR_LEN];
   fftw_plan _forward_out_place_plan_arr[FFTW_MANAGER_ARR_LEN];
   fftw_plan _inverse_out_place_plan_arr[FFTW_MANAGER_ARR_LEN];
   LWORD     _length;

public:
    FFTWManager();
    ~FFTWManager();

    LWORD   forwardFFT( LWORD length, fftw_complex *in_arr, fftw_complex *out_arr = NULL );
    LWORD   inverseFFT( LWORD length, fftw_complex *in_arr, fftw_complex *out_arr = NULL );
    VOID    reset(VOID);

    static  LWORD verifyLen( LWORD len, LWORD &log2_len );
    static  LWORD get_log_2_len( LWORD len, LWORD &log2_len );
};

class RFFTWManager
{
private:
    rfftw_plan _forward_plan_arr[FFTW_MANAGER_ARR_LEN];
    rfftw_plan _inverse_plan_arr[FFTW_MANAGER_ARR_LEN];
    LWORD      _length;
 
public:
    RFFTWManager();
   ~RFFTWManager();

    LWORD   forwardFFT( LWORD length, fftw_real *in_arr, fftw_real *out_arr = NULL );
    LWORD   inverseFFT( LWORD length, fftw_real *in_arr, fftw_real *out_arr = NULL );
    VOID    reset(VOID);

    static  LWORD verifyLen( LWORD len, LWORD &log2_len );
    static  LWORD get_log_2_len( LWORD len, LWORD &log2_len );
};

extern    FFTWManager     CmplxFFT;
extern    RFFTWManager    RealFFT;

NS_END

#endif
