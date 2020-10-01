//////////////////////////////////////////////////////
// CGexStats: Classe to computes statistics on a CTest object
//////////////////////////////////////////////////////

#if !defined(GEX_CTEST_STATS_H__INCLUDED_)
#define GEX_CTEST_STATS_H__INCLUDED_
#include "ctest.h"
#include "classes.h"
#include "gex_constants.h"

///////////////////////////////////////////////////////////
#ifndef gex_max
#define gex_max(a,b)	((a) < (b) ? (b) :(a))
#endif
#ifndef gex_min
#define gex_min(a,b)	((a) > (b) ? (b) :(a))
#endif
#ifndef gex_maxAbs
#define gex_maxAbs(a,b)	((fabs(a)) < (fabs(b)) ? (b) :(a))
#endif
#ifndef gex_minAbs
#define gex_minAbs(a,b)	((fabs(a)) > (fabs(b)) ? (b) :(a))
#endif

namespace GS
{
namespace Gex
{

class PATPartFilter;

}
}

class CGexStats
{
    // options
    float   m_fOutlierRemovalValue;
    enum    ScalingMode { SCALING_NONE, SCALING_SMART, SCALING_TO_LIMITS, SCALING_NORMALIZED } m_eScalingMode;
    bool    m_bGenGalTst_Compute_AdvStat;
    int     mHistoBarsCount;
    bool    m_stopOnFail;//false if mode is (default) 'Continue on Fail'
    bool    m_passFailFlag;//false if mode is (default) 'use limits only'
    bool    mKeepOneOption;

public:
    CGexStats();
    // update cached options like removalvalue,...
    bool    UpdateOptions(class CReportOptions*);
    //
    void	ComputeLowLevelTestStatistics(CTest * ptTestCell, double lfScalingExponent = 1.0,
                                          GS::Gex::PATPartFilter * ptPartFilter = NULL);
    void	ComputeBasicTestStatistics(CTest * ptTestCell, bool bSamplesOnly = false,
                                       GS::Gex::PATPartFilter * ptPartFilter = NULL);
    void	RebuildHistogramArray(CTest * ptTestCell, int nDataRange = GEX_HISTOGRAM_OVERDATA,
                                  GS::Gex::PATPartFilter *ptPartFilter = NULL);
    void	ComputeAdvancedDataStatistics(CTest * ptTestCell, bool bForceCompute = false,
                                          bool bStatsCpCpkPercentileFormula = false, bool bIqrOutlierRemoval = false,
                                          GS::Gex::PATPartFilter * ptPartFilter =  NULL);
    int		CheckIncorrectScale(CTest *ptTestCell,double fSampleData,double *lfTsrData);

    void	ComputeAdvancedDataStatistics_Skew(CTest * ptTestCell, bool bForceCompute = false,
                                               GS::Gex::PATPartFilter * ptPartFilter = NULL);
    void	ComputeAdvancedDataStatistics_Kurtosis(CTest * ptTestCell, bool bForceCompute = false,
                                                   GS::Gex::PATPartFilter * ptPartFilter = NULL);
    void	ComputeAdvancedDataStatistics_Quartiles(CTest * ptTestCell, bool bForceCompute = false,
                                                    bool bStatsCpCpkPercentileFormula = false,
                                                    bool bIqrOutlierRemoval = false,
                                                    GS::Gex::PATPartFilter * ptPartFilter = NULL);
};

#endif	// #ifdef GEX_CTEST_STATS_H__INCLUDED_
