///////////////////////////////////////////////////////////
// Classes to computes statistics on a CTest object
///////////////////////////////////////////////////////////
#include <math.h>

#include "cstats.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_algorithms.h"
#include "gex_report.h"
#include <gqtl_log.h>
//#include "pat_info.h"
#include "pat_engine.h"
#include "pat_part_filter.h"
#include "product_info.h"
#include <gqtl_global.h>

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
//#include "DebugMemory.h"

// In patman_lib.cpp
extern bool patlib_IsDistributionGaussian(CTest *ptTestCell,
                                          int lCategoryValueCount = 5,
                                          bool lAssumeIntegerCategory = true,
                                          int aMinConfThreshold = 2);

// in report_build.cpp
extern CGexReport*      gexReport;			// Handle to report class
extern CReportOptions	ReportOptions;      // Holds options (report_build.h)


CGexStats::CGexStats()
    : m_fOutlierRemovalValue(6.f),
      m_eScalingMode(SCALING_NONE),
      m_bGenGalTst_Compute_AdvStat(false),
      mHistoBarsCount(40)
{
}

/////////////////////////////////////////////////////////////////////////////
// Compute Low level statistics for one test: SumX, SumX^2, etc...
/////////////////////////////////////////////////////////////////////////////
/// No statistics from limits
void	CGexStats::ComputeLowLevelTestStatistics(CTest *ptTestCell,double lfScalingExponent/*=1*/,
                                                 GS::Gex::PATPartFilter * ptPartFilter /*=NULL*/)
{
    ptTestCell->lfTotal = ptTestCell->lfSamplesTotal = 0;
    ptTestCell->lfTotalSquare = ptTestCell->lfSamplesTotalSquare = 0;
    ptTestCell->ldExecs = 0;
    ptTestCell->lSamplesMinCell = ptTestCell->lSamplesMaxCell = 0; // Offset to the cell that holds the 'Min' & 'Max' values of the samplles.

    // If we have to keep values in normalized format, do not rescale !
    //if(ReportOptions.iSmartScaling == GEX_UNITS_RESCALE_NORMALIZED)
    if(m_eScalingMode==SCALING_NORMALIZED)
        lfScalingExponent = 1.0;

    double	lfValue;
    double	lfMin=C_INFINITE;
    double  lfMax=-C_INFINITE;
    ptTestCell->ldSamplesValidExecs = 0;

    for(long lIndex = 0; lIndex < ptTestCell->m_testResult.count(); lIndex++)
    {
        if (ptTestCell->m_testResult.isValidResultAt(lIndex))
        {
            // If parts filter disabled or part matches the filer, we must process it...
            if (ptPartFilter == NULL || ptPartFilter->IsFiltered(lIndex))
            {
                if (ptTestCell->m_testResult.isMultiResultAt(lIndex))
                {
                    for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lIndex)->count(); ++nCount)
                    {
                        if(!ptTestCell->m_testResult.at(lIndex)->isValidResultAt(nCount)) continue;
                        lfValue = ptTestCell->m_testResult.at(lIndex)->multiResultAt(nCount);

                        if (lfValue != GEX_C_DOUBLE_NAN)
                        {
                            // Build correct valid data count
                            ptTestCell->ldSamplesValidExecs++;

                            lfValue *= lfScalingExponent;
                            // Compute 'Min' value
                            if(lfMin > lfValue)
                            {
                                // Keep track of lowest value
                                lfMin = lfValue;

                                // Save its cell Offset in the data samples array
                                ptTestCell->lSamplesMinCell = lIndex;
                            }
                            // Compute 'Max' value
                            if(lfMax < lfValue)
                            {
                                // Keep track of lowest value
                                lfMax = lfValue;

                                // Save its cell Offset in the data samples array
                                ptTestCell->lSamplesMaxCell = lIndex;
                            }

                            ptTestCell->lfSamplesTotal += lfValue;
                            ptTestCell->lfSamplesTotalSquare += lfValue*lfValue;
                        }
                    }
                }
                else
                {
                    lfValue = ptTestCell->m_testResult.resultAt(lIndex);

                    // Build correct valid data count
                    ptTestCell->ldSamplesValidExecs++;

                    lfValue *= lfScalingExponent;
                    // Compute 'Min' value
                    if(lfMin > lfValue)
                    {
                        // Keep track of lowest value
                        lfMin = lfValue;

                        // Save its cell Offset in the data samples array
                        ptTestCell->lSamplesMinCell = lIndex;
                    }
                    // Compute 'Max' value
                    if(lfMax < lfValue)
                    {
                        // Keep track of lowest value
                        lfMax = lfValue;

                        // Save its cell Offset in the data samples array
                        ptTestCell->lSamplesMaxCell = lIndex;
                    }

                    ptTestCell->lfSamplesTotal += lfValue;
                    ptTestCell->lfSamplesTotalSquare += lfValue*lfValue;
                }
            }
        }
    }

    ptTestCell->ldExecs = ptTestCell->ldSamplesValidExecs;
    ptTestCell->lfTotal = ptTestCell->lfSamplesTotal;
    ptTestCell->lfTotalSquare = ptTestCell->lfSamplesTotalSquare;
    ptTestCell->lfSamplesMax = ptTestCell->lfMax = lfMax;
    ptTestCell->lfSamplesMin = ptTestCell->lfMin = lfMin;
}

/////////////////////////////////////////////////////////////////////////////
// returns 10^iPower
/// No statistics from limits
double	ScalingPower(int iPower)
{
    double	lfValue;
    switch(iPower)
    {
        default:
            // Was qPow
            lfValue = GS_POW(10.0,-iPower);
            break;
        case 0:	// no scaling
            lfValue =1;
            break;
        case 253:	// for unsigned -3 'K'
        case -3:
            lfValue =1e3;
            break;
        case 250:	// for unsigned -6 'M'
        case -6:
            lfValue =1e6;
            break;
        case 247:	// for unsigned -9 'G'
        case -9:
            lfValue =1e9;
            break;
        case 244:	// for unsigned -13 'T'
        case -12:
            lfValue =1e12;
            break;
        case 2:		// '%'
            lfValue =1e-2;
            break;
        case 3:		// 'm'
            lfValue =1e-3;
            break;
        case 6:		// 'u'
            lfValue =1e-6;
            break;
        case 9:		// 'n'
            lfValue =1e-9;
            break;
        case 12:	// 'p'
            lfValue =1e-12;
            break;
        case 15:	// 'f'
            lfValue =1e-15;
            break;
    }
    return lfValue;
}

/////////////////////////////////////////////////////////////////////////////
// Compute BASIC statistics for one test: Mean, Sigma, Range, Cp,Cpk
/////////////////////////////////////////////////////////////////////////////
/// No statistics from limits
int CGexStats::CheckIncorrectScale(CTest *ptTestCell,double fSampleData,double *lfTsrData)
{
    // If no sampled data, we can't figure out if scaling factor is correct or not...
    if(ptTestCell->ldSamplesValidExecs <= 0)
        return 0; // Scale is CORRECT

    // If data = 0, we don't mater about scaling factor !
    if(fSampleData == 0)
        return 0;

    //
    double fLog = log10((*lfTsrData)/fSampleData);
    double fScaledTsrData = (*lfTsrData)*ScalingPower(ptTestCell->res_scal);
    double fLogScale = log10(fScaledTsrData/fSampleData);

    // Check which values are closer: without scaling, or with scaling data...
    if(fabs(fLog) < fabs(fLogScale))
    {
        // Log of non scaled value shows value are closer than when scaled...
        // so no change on the data !
        return 0;	// Scale is CORRECT
    }
    else
    {
        *lfTsrData = fScaledTsrData;
        ptTestCell->bTsrIncorrectScale = true;
        return 1;	// Scale is NOT CORRECT
    }
}

/////////////////////////////////////////////////////////////////////////////
// Compute BASIC statistics for one test: Mean, Sigma, Range, Cp,Cpk
/////////////////////////////////////////////////////////////////////////////
/// calculated for Cp and cpk
void	CGexStats::ComputeBasicTestStatistics(
        CTest *ptTestCell,
        bool bSamplesOnly,
        GS::Gex::PATPartFilter * ptPartFilter/*=NULL*/)
{
    long	lIndex=0;
    long	ldTotalExecs = ptTestCell->ldExecs;
    double	lfMean	= GEX_C_DOUBLE_NAN;
    double	lfSigma	= 0.0;
    double	lfValue,CpkL,CpkH,Cpk,Cp;
    bool	bTestFail=true;

    double lfExponent = ScalingPower(ptTestCell->res_scal);

    //if(ReportOptions.iSmartScaling == GEX_UNITS_RESCALE_NORMALIZED)
    if(m_eScalingMode==SCALING_NORMALIZED)
        lfExponent = 1.0;

    if (bSamplesOnly)
        ptTestCell->GetCurrentLimitItem()->ldSampleFails = 0;

    if(bSamplesOnly && ptTestCell->ldSamplesValidExecs)
    {
        // If we have to keep values in normalized format, do not rescale!
        //if(ReportOptions.iSmartScaling == GEX_UNITS_RESCALE_NORMALIZED)
        if(m_eScalingMode==SCALING_NORMALIZED)
            lfExponent = 1.0;

        // compute updated number of samples failures...so new yield can be computed
        ptTestCell->ldSamplesValidExecs = 0;
        ptTestCell->lfSamplesTotal = 0;
        ptTestCell->lfSamplesTotalSquare = 0;

        GS::Core::MultiLimitItem* lOldMl = ptTestCell->GetCurrentLimitItem();

        bool lIsParametricItem = true;
        bool lAreFailsCounted = false;
        bool lWarningLoopedPtrs = true;
        // We don't impact the PAT process.
        GS::LPPlugin::LicenseProvider::GexProducts lPorduct = GS::LPPlugin::ProductInfo::getInstance()->getProductID();
        if(( lPorduct != GS::LPPlugin::LicenseProvider::eExaminatorPAT
             && lPorduct != GS::LPPlugin::LicenseProvider::ePATMan
             && lPorduct != GS::LPPlugin::LicenseProvider::eGTM
             && lPorduct != GS::LPPlugin::LicenseProvider::ePATManEnterprise)
             && (lOldMl != NULL))
        {
            lIsParametricItem = lOldMl->isParametricItem();
        }

        QString strFailCountMode = ReportOptions.GetOption("dataprocessing", "fail_count").toString();
        CGexFileInGroup::FailCountMode lMode = CGexFileInGroup::FAILCOUNT_FIRST;
        if (strFailCountMode == "all")
            lMode = CGexFileInGroup::FAILCOUNT_ALL;
        else if (strFailCountMode == "first")
            lMode = CGexFileInGroup::FAILCOUNT_FIRST;

        // Add missing invalid multi-limit set
        ptTestCell->CompleteWithInvalideLimit(GS::Core::MultiLimitItem::sNbMaxLimits);

        for (int i=0; i<ptTestCell->MultiLimitItemCount(); ++i)
        {
            if (!ptTestCell->setCurrentLimitItem(i))
            {
                continue;
            }

            if (bSamplesOnly)
                ptTestCell->GetCurrentLimitItem()->ldSampleFails = 0;

            for(lIndex=0; lIndex < ptTestCell->m_testResult.count(); lIndex++)
            {
                if (ptTestCell->m_testResult.isValidResultAt(lIndex))
                {
                    // If parts filter disabled or part matches the filer, we must process it...
                    if(ptPartFilter == NULL || ptPartFilter->IsFiltered(lIndex))
                    {
                        bTestFail = false;

                        if (ptTestCell->m_testResult.isMultiResultAt(lIndex))
                        {
                            CTestResult::PassFailStatus ePassFailStatus = ptTestCell->m_testResult.passFailStatus(lIndex);
                            // This section is for the looped PTR. We can't use the pass/fail flag
                            if(!lIsParametricItem)
                            {
                                //undefined by default, for looped PTR (so that limits will be used)
                                ePassFailStatus = CTestResult::statusUndefined;
                                if(lWarningLoopedPtrs
                               && m_passFailFlag
                               && !mKeepOneOption
                               && lMode == CGexFileInGroup::FAILCOUNT_FIRST)
                                {
                                    lWarningLoopedPtrs = false;
                                    QString warningMsg = QString("Test " + ptTestCell->GetTestName() + " has multiple results per run. Examinator has the possibility to store multiple results per run for a given test, but only one P/F flag. You have set option \"Data Processing options...\"->\"Parameter pass/fail rule\" to \"Use pass/fail flag if valid (otherwise use limits)\". To avoid computing inconsistent statistics, the parameter pass/fail decision will be based on limits for this test.");
                                    gexReport->GetReportLogList().addReportLog(warningMsg, GS::Gex::ReportLog::ReportWarning);
                                    GSLOG(SYSLOG_SEV_WARNING, warningMsg.toLatin1().constData());
                                }
                            }
                            for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lIndex)->count(); nCount++)
                            {
                                //GCORE-17470
                                if(!lIsParametricItem)//means we have PTRs that have been merged -> fail count must be treated a little bit differently from the MPR
                                    bTestFail = false;
                                if(!ptTestCell->m_testResult.at(lIndex)->isValidResultAt(nCount)) continue;
                                lfValue = ptTestCell->m_testResult.at(lIndex)->multiResultAt(nCount);

                                if (lfValue != GEX_C_DOUBLE_NAN)
                                {
                                    // Update exact amount of valid samples in array only for the first element
                                    // otherwise, we will do the some of all execs for all ML
                                    if (i == 0)
                                    {
                                        ptTestCell->ldSamplesValidExecs++;
                                        ptTestCell->lfSamplesTotal += lfValue*lfExponent;
                                        ptTestCell->lfSamplesTotalSquare +=  (lfValue*lfExponent) * (lfValue*lfExponent);
                                    }
                                    if (ptTestCell->bTestType == 'F')
                                    {
                                        bTestFail |= (lfValue == 0.0) ? true: false;
                                    }
                                    else
                                    {
                                        // Format result
                                        lfValue *= lfExponent;

                                        // Check if test failure
                                        bTestFail |= ptTestCell->isFailingValue(lfValue, ePassFailStatus);
                                    }
                                }
                                //GCORE-17470
                                if(bTestFail && !lIsParametricItem)
                                {
                                    ptTestCell->GetCurrentLimitItem()->ldSampleFails++;
                                    lAreFailsCounted = true;
                                }
                            }
                        }
                        else
                        {
                            lfValue = ptTestCell->m_testResult.resultAt(lIndex);

                            // Update exact amount of valid samples in array only for the first element
                            // otherwise, we will do the some of all execs for all ML
                            if (i == 0)
                            {
                                ptTestCell->ldSamplesValidExecs++;
                                ptTestCell->lfSamplesTotal += lfValue*lfExponent;
                                ptTestCell->lfSamplesTotalSquare +=  (lfValue*lfExponent) * (lfValue*lfExponent);
                            }
                            if (ptTestCell->bTestType == 'F')
                            {
                                bTestFail |= (lfValue == 0.0) ? true: false;
                            }
                            else
                            {
                                // Format result
                                lfValue *= lfExponent;
                                // Check if test failure
                                bTestFail |= ptTestCell->isFailingValue(lfValue, ptTestCell->m_testResult.passFailStatus(lIndex));
                            }
                        }

                        if(bTestFail && !lAreFailsCounted)
                        {
                            ptTestCell->GetCurrentLimitItem()->ldSampleFails++;
                        }
                    }
                }
            }


            // Compute mean from samples.only for the first element
            // otherwise, we will do the some of all execs for all ML
            if (i == 0)
            {
                if (ptTestCell->ldSamplesValidExecs > 0)
                    lfMean = ptTestCell->lfSamplesTotal/ptTestCell->ldSamplesValidExecs;
            }

            // If must only use samples for statistics...
            ptTestCell->GetCurrentLimitItem()->ldFailCount = ptTestCell->GetCurrentLimitItem()->ldSampleFails;
        }
        ptTestCell->setCurrentLimitItem(lOldMl);
        ptTestCell->ldExecs = ptTestCell->ldSamplesValidExecs;
        // Flag test statisics comming from Samples
        ptTestCell->bStatsFromSamples = true;
    }
    else
    {
        // Ensure we have the accurate number of samples.
        ptTestCell->ldSamplesValidExecs = 0;
        ptTestCell->lfSamplesTotal = 0;
        ptTestCell->lfSamplesTotalSquare = 0;
        if(ptTestCell->m_testResult.count() > 0)
        {
            for(lIndex=0; lIndex < ptTestCell->m_testResult.count(); lIndex++)
            {
                if (ptTestCell->m_testResult.isValidResultAt(lIndex))
                {
                    // If parts filter disabled or part matches the filer, we must process it...
                    if(ptPartFilter == NULL || ptPartFilter->IsFiltered(lIndex))
                    {
                        if (ptTestCell->m_testResult.isMultiResultAt(lIndex))
                        {
                            for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lIndex)->count(); ++nCount){
                                if(ptTestCell->m_testResult.at(lIndex)->isValidResultAt(nCount)){
                                    ptTestCell->ldSamplesValidExecs++;
                                    double dTemp = ptTestCell->m_testResult.at(lIndex)->multiResultAt(nCount)*lfExponent;
                                    ptTestCell->lfSamplesTotal += dTemp ;
                                    ptTestCell->lfSamplesTotalSquare += dTemp * dTemp;
                                }
                            }
                        }
                        else{
                            ptTestCell->ldSamplesValidExecs++;
                            double dTemp = ptTestCell->m_testResult.resultAt(lIndex)*lfExponent;
                            ptTestCell->lfSamplesTotal += dTemp;
                            ptTestCell->lfSamplesTotalSquare += dTemp * dTemp;
                        }
                    }
                }
            }
        }

        // Mean computation.
        if (ldTotalExecs > 0 && ptTestCell->lfTotal != -C_INFINITE)
            lfMean = ptTestCell->lfTotal/ldTotalExecs;
    }

    if(ptTestCell->ldExecs > 1)
    {
        if((ptTestCell->lfTotal == -C_INFINITE) || (ptTestCell->lfTotalSquare == -C_INFINITE) || (ptTestCell->lfMax != -C_INFINITE && ptTestCell->lfMin != C_INFINITE &&(ptTestCell->lfMax -ptTestCell->lfMin)<=0.0))
            lfSigma = 0.0;
        else
            lfSigma = sqrt(fabs((((double)ldTotalExecs*ptTestCell->lfTotalSquare) - (ptTestCell->lfTotal*ptTestCell->lfTotal))
                                /
                                ((double)ldTotalExecs*((double)ldTotalExecs-1))));
    }

    if(ptTestCell->bTsrIncorrectScale)
    {
        // In case TSR info need scaling, current Mean, Sigma computed are not OK !
        float	fSampleMean;
        float fSampleSigma = 0.0;
        //FIXME: not used ?
        //float fSampleRange;

        fSampleMean = ptTestCell->lfSamplesTotal/ptTestCell->ldSamplesValidExecs;

        if(ptTestCell->ldSamplesValidExecs > 1)
            // was pow (qPow behaves strangely in threads)
            fSampleSigma = sqrt(fabs((((double)ptTestCell->ldSamplesValidExecs*ptTestCell->lfSamplesTotalSquare)
                                      - GS_POW(ptTestCell->lfSamplesTotal,2.0))/
                                     ((double)ptTestCell->ldSamplesValidExecs*((double)ptTestCell->ldSamplesValidExecs-1))));
        //FIXME: not used ?
        //fSampleRange= ptTestCell->lfSamplesMax - ptTestCell->lfSamplesMin;

        CheckIncorrectScale(ptTestCell,fSampleMean,&lfMean);
        CheckIncorrectScale(ptTestCell,fSampleSigma,&lfSigma);
    }

    // One final checking...Sigma may be wrong if computed from TSR with incorrectly scaled values.
    // so one way to detect it is to verify that Sigma < Range
    if(bSamplesOnly && (ptTestCell->ldSamplesValidExecs > 0))
        ptTestCell->lfRange = ptTestCell->lfSamplesMax- ptTestCell->lfSamplesMin;
    else
    {
        if (ptTestCell->lfMax != -C_INFINITE && ptTestCell->lfMin != C_INFINITE)
            ptTestCell->lfRange = ptTestCell->lfMax - ptTestCell->lfMin;
    }

    if((bSamplesOnly && (ptTestCell->ldSamplesValidExecs > 1)) || ((lfSigma > ptTestCell->lfRange) && (ptTestCell->lfRange > 0)))
    {
        if(ptTestCell->ldSamplesValidExecs > 1 && (ptTestCell->lfMax != -C_INFINITE && ptTestCell->lfMin != C_INFINITE && (ptTestCell->lfMax -ptTestCell->lfMin)>0.0))
        {
            // was pow
            double lPow=GS_POW(ptTestCell->lfSamplesTotal,2.0);
            double lV1=((double)ptTestCell->ldSamplesValidExecs*ptTestCell->lfSamplesTotalSquare);
            double lDeno=((double)ptTestCell->ldSamplesValidExecs*((double)ptTestCell->ldSamplesValidExecs-1));
            double lFabs=fabs((lV1 - lPow)/ lDeno );
            lfSigma = sqrt( lFabs );
        }
        else
            lfSigma  = 0.0;
    }

    ptTestCell->lfSigma = lfSigma;
    ptTestCell->lfMean = lfMean;

    ///////////////////////////////////////////////////////////////
    // Compute for each ML: Cp, Cpk
    ///////////////////////////////////////////////////////////////
    //   BYTE	bLimitFlag;	// bit0=1 (no low limit), bit1=1 (no high limit)
    // Compute Cpk
    for (int i=0; i<ptTestCell->MultiLimitItemCount(); ++i)
    {
        GS::Core::MultiLimitItem* lMultiLimit = ptTestCell->GetMultiLimitItem(i);
        if (!lMultiLimit)
        {
            continue;
        }

        if(lMultiLimit->bLimitFlag & CTEST_LIMITFLG_NOLTL)
        {
            CpkL = lMultiLimit->lfCp = C_NO_CP_CPK;	// No Low Limit
        }
        else
        {
            if(lfSigma == 0.0)
            {
                CpkL = C_NO_CP_CPK;
                //CpkL = C_INFINITE;	case 3740, pyc, 12/04/2011
            }
            else
            {
                CpkL = (lfMean-lMultiLimit->lfLowLimit)/(3.0*lfSigma);
            }
            if(CpkL > 1e6)
                CpkL = C_INFINITE;
        }

        if(lMultiLimit->bLimitFlag & CTEST_LIMITFLG_NOHTL)
        {
            CpkH = C_NO_CP_CPK;	// No High Limit.
        }
        else
        {
            if(lfSigma == 0.0)
            {
                CpkH = C_NO_CP_CPK;
                // CpkH = C_INFINITE;		case 3740, pyc, 12/04/2011
            }
            else
            {
                CpkH = (lMultiLimit->lfHighLimit-lfMean)/(3.0*lfSigma);
            }
            if(CpkH > 1e6)
            {
                CpkH = C_INFINITE;
            }
            if( (CpkH < -1e6) && (CpkH!=C_NO_CP_CPK))
            {
                CpkH = -C_INFINITE;		// case 3740, pyc, 12/04/2011
            }
        }

        // Compute Cp
        if(CpkL == C_NO_CP_CPK)
        {
            if(CpkH == C_NO_CP_CPK)			// case 3740, pyc, 12/04/2011
                lMultiLimit->lfCpk=C_NO_CP_CPK;
            else
                lMultiLimit->lfCpk= CpkH;
        }
        else
        {
            if(CpkH == C_NO_CP_CPK)
                lMultiLimit->lfCpk = CpkL;
            else
            {
                if(lfSigma > 0.0)
                {
                    Cpk = gex_min(CpkL,CpkH);
                    if(Cpk > 1e6) Cpk = C_INFINITE;
                    if(Cpk < -1e6) Cpk = -C_INFINITE;
                    lMultiLimit->lfCpkLow  = CpkL;
                    lMultiLimit->lfCpkHigh = CpkH;
                    lMultiLimit->lfCpk = Cpk;
                    Cp = fabs(lMultiLimit->lfHighLimit - lMultiLimit->lfLowLimit)/(6.0*lfSigma);
                    if(Cp > 1e6)
                        Cp = C_INFINITE;
                    lMultiLimit->lfCp = Cp;
                }
                else
                {
                    // Sigma = 0, No valid Cp, Cpk!
                    // ptTestCell->lfCp = ptTestCell->lfCpk = C_INFINITE;		case 3740, pyc, 12/04/2011
                    lMultiLimit->lfCp = lMultiLimit->lfCpk = C_NO_CP_CPK;
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Rebuild histogram array
/////////////////////////////////////////////////////////////////////////////
/// This function is going to be called only after the definition of the limit set used
/// So we can let it like this
void CGexStats::RebuildHistogramArray(CTest * ptTestCell,int nDataRange /*=GEX_HISTOGRAM_OVERDATA*/,
                                      GS::Gex::PATPartFilter * ptPartFilter /*=NULL*/)
{
    int		iCell;
    double	lfValue,lfMin,lfMax;
    bool	bUseAllDataset=false;

    // Reset histogram bars.
    memset(&ptTestCell->lHistogram[0],0,TEST_HISTOSIZE*sizeof(int));

    // Make sure we have some results
    if(ptTestCell->ldSamplesValidExecs == 0)
        return;

    // Histogram over limits or over data?
    switch(nDataRange)
    {
        default:
        case GEX_HISTOGRAM_DISABLED:	// disabled
        case GEX_HISTOGRAM_OVERLIMITS:	// Histogram chart over test limits.
        case GEX_HISTOGRAM_CUMULLIMITS: // Cumul over test limits
            bUseAllDataset = false;
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            {
                // Low limit exists
                lfMin = ptTestCell->GetCurrentLimitItem()->lfHistogramMin = ptTestCell->GetCurrentLimitItem()->lfLowLimit;
            }
            else
            {
                // No low limit.
                lfMin = ptTestCell->GetCurrentLimitItem()->lfHistogramMin = ptTestCell->lfSamplesMin;
            }

            // Check if high limit exists
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                // High limit exists
                lfMax = ptTestCell->GetCurrentLimitItem()->lfHistogramMax = ptTestCell->GetCurrentLimitItem()->lfHighLimit;
            }
            else
            {
                // No high limit
                lfMax = ptTestCell->GetCurrentLimitItem()->lfHistogramMax = ptTestCell->lfSamplesMax;
            }
            break;

        case GEX_HISTOGRAM_OVERDATA:	// Histogram chart over data sample
        case GEX_HISTOGRAM_CUMULDATA:	// Cumul over data sample
            bUseAllDataset = true;
            lfMin = ptTestCell->GetCurrentLimitItem()->lfHistogramMin = ptTestCell->lfSamplesMin;
            lfMax = ptTestCell->GetCurrentLimitItem()->lfHistogramMax = ptTestCell->lfSamplesMax;
            break;

        case GEX_HISTOGRAM_DATALIMITS:	// adaptive: show data & limits
            // Chart has to be done over maxi of both datapoints & limits
            bUseAllDataset = true;
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                lfMin = ptTestCell->GetCurrentLimitItem()->lfHistogramMin = gex_min(ptTestCell->lfSamplesMin,ptTestCell->GetCurrentLimitItem()->lfLowLimit);
            else
                lfMin = ptTestCell->GetCurrentLimitItem()->lfHistogramMin = ptTestCell->lfSamplesMin;
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                lfMax = ptTestCell->GetCurrentLimitItem()->lfHistogramMax = gex_max(ptTestCell->lfSamplesMax,ptTestCell->GetCurrentLimitItem()->lfHighLimit);
            else
                lfMax = ptTestCell->GetCurrentLimitItem()->lfHistogramMax = ptTestCell->lfSamplesMax;
            break;
    }

    double lfScalingExponent = ScalingPower(ptTestCell->res_scal);

    // If we have to keep values in normalized format, do not rescale!
    //if(ReportOptions.iSmartScaling == GEX_UNITS_RESCALE_NORMALIZED)
    if(m_eScalingMode==SCALING_NORMALIZED)
        lfScalingExponent = 1.0;

    GS::Gex::HistogramData lHistoData(mHistoBarsCount, lfMin, lfMax);

    for(long lIndex = 0; lIndex < ptTestCell->m_testResult.count(); lIndex++)
    {
        if (ptTestCell->m_testResult.isValidResultAt(lIndex))
        {
            // If parts filter disabled or part matches the filer, we must process it...
            if(ptPartFilter == NULL || ptPartFilter->IsFiltered(lIndex))
            {
                if (ptTestCell->m_testResult.isMultiResultAt(lIndex))
                {
                    // Compute on all result for this run
                    for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lIndex)->count(); ++nCount)
                    {
                        if(!ptTestCell->m_testResult.at(lIndex)->isValidResultAt(nCount)) continue;

                        lfValue = ptTestCell->m_testResult.at(lIndex)->multiResultAt(nCount) * lfScalingExponent;

                        // Rebuild histogram, make sure value is within Histogram limits
                        if(bUseAllDataset || ((lfValue >= lfMin) && (lfValue <= lfMax)))
                        {
                            lHistoData.AddValue(lfValue, bUseAllDataset);
                            iCell = (int)( (TEST_HISTOSIZE*(lfValue-lfMin))/(lfMax-lfMin));
                            if(iCell >= TEST_HISTOSIZE)
                                iCell = TEST_HISTOSIZE-1;	// Security to avoid overflowing array.
                            if(iCell < 0) iCell = 0;
                            ptTestCell->lHistogram[iCell]++;
                        }
                    }
                }
                else
                {
                    lfValue = ptTestCell->m_testResult.resultAt(lIndex) * lfScalingExponent;

                    // Rebuild histogram, make sure value is within Histogram limits
                    if(bUseAllDataset || ((lfValue >= lfMin) && (lfValue <= lfMax)))
                    {
                        lHistoData.AddValue(lfValue, bUseAllDataset);
                        iCell = (int)( (TEST_HISTOSIZE*(lfValue-lfMin))/(lfMax-lfMin));
                        if(iCell >= TEST_HISTOSIZE)
                            iCell = TEST_HISTOSIZE-1;	// Security to avoid overflowing array.
                        if(iCell < 0) iCell = 0;
                        ptTestCell->lHistogram[iCell]++;
                    }
                }
            }
        }
    }

    ptTestCell->mHistogramData = lHistoData;
}

/////////////////////////////////////////////////////////////////////////////
// Compute Advanced statistics
/// No statistics from limits
void	CGexStats::ComputeAdvancedDataStatistics(
        CTest *ptTestCell,
        bool bForceCompute/*=false*/,
        bool bStatsCpCpkPercentileFormula/*=false*/,
        bool bIqrOutlierRemoval/*=false*/,
        GS::Gex::PATPartFilter * ptPartFilter /*=NULL*/)
{
    if(ptTestCell->ldSamplesValidExecs <= 0)
        return;	// No samples!

    if( (ptTestCell->bTestType == '-') && (!m_bGenGalTst_Compute_AdvStat) )
        return; // don't compute advanced stats for galaxy custom tests

    // If no data available
    if(ptTestCell->m_testResult.count() > 0)
    {
        // Compute Skewness (Population degree of asymmetry of a distribution around its mean)
        ComputeAdvancedDataStatistics_Skew(ptTestCell,bForceCompute,ptPartFilter);

        // Compute Kurtosis (Population Kurtosis or relative peakedness or flatness of a distribution compared with the normal distribution)
        ComputeAdvancedDataStatistics_Kurtosis(ptTestCell,bForceCompute,ptPartFilter);

        // Compute Quartiles related statistics (Population in 1st & 3rd quartile : Q1,Q3, standard dev between Q1 and Q3: IRQ SD)
        ComputeAdvancedDataStatistics_Quartiles(ptTestCell,bForceCompute,bStatsCpCpkPercentileFormula,bIqrOutlierRemoval,ptPartFilter);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Advanced stats: Compute Skewness (Population degree of asymmetry of a distribution around its mean)
//
// Biased Skew = Sum[pow(Xi - Mean,3)] / [N*pow(Sigma, 3)]
//
// Unbiased Skew = {N/[(N-1)*(N-2)]} * Sum(pow((X - Mean)/Sigma,3))
//
/////////////////////////////////////////////////////////////////////////////
/// No statistics from limits
void	CGexStats::ComputeAdvancedDataStatistics_Skew(CTest * ptTestCell, bool bForceCompute/*=false*/,
                                                      GS::Gex::PATPartFilter * ptPartFilter/*=NULL*/)
{
    // Check if already computed!
    if((bForceCompute == false) && ptTestCell->lfSamplesSkew != -C_INFINITE)
        return;

    // Check if we can compute the skew
    if((ptTestCell->ldSamplesValidExecs < 3) || (ptTestCell->lfSigma == 0.0))
        return;

    // Get Mean & Sigma and scale to be in same scale as data.
    double lfValue=0.0;
    double lfSkewSum = 0.0;
    double lfMean = ptTestCell->lfMean/ScalingPower(ptTestCell->res_scal);
    double lfSigma = ptTestCell->lfSigma/ScalingPower(ptTestCell->res_scal);

    if (lfSigma==0)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Sigma equal 0, no skew computation possible. Aborting");
        ptTestCell->lfSamplesSkew = -C_INFINITE;
        return;
    }

    // Go through list of values, and compute required sums
    for(long lIndex = 0; lIndex < ptTestCell->m_testResult.count(); lIndex++)
    {
        if(ptTestCell->m_testResult.isValidResultAt(lIndex))
        {
            // If parts filter disabled or part matches the filer, we must process it...
            if(ptPartFilter == NULL || ptPartFilter->IsFiltered(lIndex))
            {
                if (ptTestCell->m_testResult.isMultiResultAt(lIndex))
                {
                    for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lIndex)->count(); ++nCount)
                    {
                        if(ptTestCell->m_testResult.at(lIndex)->isValidResultAt(nCount))
                            lfSkewSum += GS_POW( (ptTestCell->m_testResult.at(lIndex)->multiResultAt(nCount) - lfMean) / lfSigma, 3.0);
                    }
                }
                else
                    lfSkewSum += GS_POW( (ptTestCell->m_testResult.resultAt(lIndex) - lfMean) / lfSigma, 3.0);
            }
        }
    }

    // Compute Skew indicator
    double r=((double)(ptTestCell->ldSamplesValidExecs-1)*(double)(ptTestCell->ldSamplesValidExecs-2));

    if (r==0)
    {
        GSLOG(SYSLOG_SEV_WARNING, "ratio division equal zero, no skew computation possible, aborting.");
        ptTestCell->lfSamplesSkew = -C_INFINITE;
        return;
    }

    lfValue = (double)(ptTestCell->ldSamplesValidExecs)/r;

    ptTestCell->lfSamplesSkew = lfValue*lfSkewSum;
}

/////////////////////////////////////////////////////////////////////////////
// Advanced stats: Compute Kurtosis (Population Kurtosis or relative peakedness or flatness of a distribution compared with the normal distribution)
//
// Biased Kurtosis = {Sum[pow(Xi - Mean,4)] / [N*pow(Sigma, 4)]} - 3
//
// Unbiased Kurtosis = {[N*(N+1)/(N-1)*(N-2)*(N-3)] * Sum(pow((X - Mean)/Sigma,4))} - [3*pow(N-1,2)]/[(N-2)/(N-3)]
//
/////////////////////////////////////////////////////////////////////////////
/// No statistics from limits
void	CGexStats::ComputeAdvancedDataStatistics_Kurtosis(CTest * ptTestCell, bool bForceCompute/*=false*/,
                                                          GS::Gex::PATPartFilter * ptPartFilter /*=NULL*/)
{
    // Check if already computed!
    if((bForceCompute == false) && ptTestCell->lfSamplesKurt != -C_INFINITE)
        return;

    // Check if we can compute the kurtosis
    if((ptTestCell->ldSamplesValidExecs < 4) || (ptTestCell->lfSigma == 0.0))
        return;

    // Get Mean & Sigma and scale to be in same scale as data.
    double lfMean = ptTestCell->lfMean/ScalingPower(ptTestCell->res_scal);
    double lfSigma = ptTestCell->lfSigma/ScalingPower(ptTestCell->res_scal);

    // Go through list of values, and compute required sums
    double lfKurtosisSum = 0.0;
    long	lIndex;
    for(lIndex = 0; lIndex < ptTestCell->m_testResult.count(); lIndex++)
    {
        if(ptTestCell->m_testResult.isValidResultAt(lIndex))
        {
            // If parts filter disabled or part matches the filer, we must process it...
            if(ptPartFilter == NULL || ptPartFilter->IsFiltered(lIndex))
            {
                if (ptTestCell->m_testResult.isMultiResultAt(lIndex))
                {
                    for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lIndex)->count(); ++nCount){
                        if(ptTestCell->m_testResult.at(lIndex)->isValidResultAt(nCount))
                            lfKurtosisSum += GS_POW((ptTestCell->m_testResult.at(lIndex)->multiResultAt(nCount) - lfMean) / lfSigma, 4.0);
                    }
                }
                else
                    lfKurtosisSum += GS_POW((ptTestCell->m_testResult.resultAt(lIndex) - lfMean) / lfSigma, 4.0);
            }
        }
    }

    // Compute Kurtosis indicator
    double lfValue1 = ((double)(ptTestCell->ldSamplesValidExecs)*(double)(ptTestCell->ldSamplesValidExecs+1))/((double)(ptTestCell->ldSamplesValidExecs-1)*(double)(ptTestCell->ldSamplesValidExecs-2)*(double)(ptTestCell->ldSamplesValidExecs-3));
    double lfValue2 = (3.0*GS_POW((double)(ptTestCell->ldSamplesValidExecs-1),2.0))/((double)(ptTestCell->ldSamplesValidExecs-2)*(double)(ptTestCell->ldSamplesValidExecs-3));
    ptTestCell->lfSamplesKurt = lfValue1*lfKurtosisSum - lfValue2;
}

/// No statistics from limits
bool CGexStats::UpdateOptions(CReportOptions* ro)
{
    if (!ro)
        return false;

    bool ok=false;
    m_fOutlierRemovalValue = ro->GetOption("dataprocessing","data_cleaning_value").toFloat(&ok);
    if (!ok)
        GSLOG(SYSLOG_SEV_WARNING, "cant update (dataprocessing,data_cleaning_value) option");

    m_bGenGalTst_Compute_AdvStat = ro->GetOption(QString("statistics"), QString("generic_galaxy_tests_adv_stats")).toBool();

    QString s=ro->GetOption("dataprocessing","scaling").toString();
    if (s=="none") m_eScalingMode=SCALING_NONE;
    else if (s=="smart") m_eScalingMode=SCALING_SMART;
    else if (s=="to_limits") m_eScalingMode=SCALING_TO_LIMITS;
    else if (s=="normalized") m_eScalingMode=SCALING_NORMALIZED;
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("unknown option 'scaling' '%1' ").arg( s).toLatin1().constData());
        ok=false;
    }

    if (!ok)
        GSLOG(SYSLOG_SEV_WARNING, "cant update (dataprocessing,data_cleaning_value) option !");

    bool lInteger = false;
    mHistoBarsCount = ReportOptions.GetOption("adv_histogram", "total_bars").toInt(&lInteger);

    if (lInteger == false)
    {
        GSLOG(SYSLOG_SEV_WARNING,
               " error : Option adv_histogram/total_bars is not a integer value");
    }

    m_stopOnFail = false;
    if(ReportOptions.GetOption("dataprocessing", "fail_count").toString() == "first")
        m_stopOnFail = true;
    m_passFailFlag = ReportOptions.GetOption("dataprocessing", "param_passfail_rule").toString() == "passfail_flag" ? true : false;
    mKeepOneOption = ReportOptions.GetOption("dataprocessing", "multi_parametric_merge_option").toString() == "keep_one" ? true : false;

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Advanced stats: Quartiles relates statistics.
/////////////////////////////////////////////////////////////////////////////
void	CGexStats::ComputeAdvancedDataStatistics_Quartiles(CTest *ptTestCell,bool bForceCompute/*=false*/,
                                                           bool bStatsCpCpkPercentileFormula/*=false*/,
                                                           bool bIqrOutlierRemoval/*=false*/,
                                                           GS::Gex::PATPartFilter * ptPartFilter /*=NULL*/)
{
    // Check if already computed!
    if((bForceCompute == false) && (ptTestCell->lfSamplesQuartile1 != -C_INFINITE) &&
       (ptTestCell->lfSamplesQuartile2 != -C_INFINITE) &&
       (ptTestCell->lfSamplesQuartile3 != -C_INFINITE))
        return;

    if(ptTestCell->ldSamplesValidExecs == 0)
        return;	// No samples!

    // Fill list with data points (they are NOT normalized, exponent is dropped and is in ptTestCell->res_scal
    QVector<double> vectDataPoints;
    vectDataPoints.reserve(ptTestCell->ldSamplesValidExecs);

    long lIndex;
    for(lIndex = 0; lIndex < ptTestCell->m_testResult.count(); lIndex++)
    {
        if(ptTestCell->m_testResult.isValidResultAt(lIndex))
        {
            // If parts filter disabled or part matches the filer, we must process it...
            if(ptPartFilter == NULL || ptPartFilter->IsFiltered(lIndex))
            {
                if (ptTestCell->m_testResult.isMultiResultAt(lIndex))
                {
                    for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lIndex)->count(); ++nCount)
                    {
                        if(ptTestCell->m_testResult.at(lIndex)->isValidResultAt(nCount))
                            vectDataPoints.append(ptTestCell->m_testResult.at(lIndex)->multiResultAt(nCount));
                    }
                }
                else
                    vectDataPoints.append(ptTestCell->m_testResult.resultAt(lIndex));
            }
        }
    }

    // Sort list
    qSort(vectDataPoints);

    // Get quartiles.
    long	ldSamplesValidExecs = vectDataPoints.count();	// Total valid samples available (considering filtering if any).
    if(ldSamplesValidExecs == 0)
        return;	// No samples!

    lIndex = vectDataPoints.count();
    unsigned int	uP0_5 = lIndex >= 200 ? (int)(0.005*((double)lIndex+1.0)) - 1 : 0;
    unsigned int	uP2_5 = lIndex >= 40 ? (int)(0.025*((double)lIndex+1.0)) - 1 : 0;
    unsigned int	uP10 = lIndex >= 10 ? (int)(0.1*((double)lIndex+1.0)) - 1 : 0;
    unsigned int	uQuartile1 = lIndex >= 3 ? (int)(0.25*((double)lIndex+1.0)) - 1 : 0;
    unsigned int	uQuartile3 = lIndex >= 1 ? (int)(0.75*((double)lIndex+1.0)) - 1 : 0;
    unsigned int	uP90 = lIndex >= 1 ? (int)(0.9*((double)lIndex+1.0)) - 1 : 0;
    unsigned int	uP97_5 = lIndex >= 1 ? (int)(0.975*((double)lIndex+1.0)) - 1 : 0;
    unsigned int	uP99_5 = lIndex >= 1 ? (int)(0.995*((double)lIndex+1.0)) - 1 : 0;
    // Compute index to test at position 99.5%
    unsigned int	uUp = (unsigned int)(0.995*((double)lIndex+1.0));
    if(uUp >= 1) uUp--;
    // Compute index to test at position 0.5%
    unsigned int	uLp = (unsigned int)(0.005*((double)lIndex+1.0));
    if(uLp >= 1) uLp--;

    // Compute exponent to have value normalized.
    double lfExponent = ScalingPower(ptTestCell->res_scal);

    // If we have to keep values in normalized format, do not rescale!
    //if(ReportOptions.iSmartScaling == GEX_UNITS_RESCALE_NORMALIZED)
    if (m_eScalingMode==SCALING_NORMALIZED)
        lfExponent = 1.0;

    // Percentiles values
    ptTestCell->lfSamplesP0_5		= vectDataPoints[uP0_5] * lfExponent;				// P0.5% = Sample value at 0.5% of sorted data.
    ptTestCell->lfSamplesP2_5		= vectDataPoints[uP2_5] * lfExponent;				// P2.5% = Sample value at 2.5% of sorted data.
    ptTestCell->lfSamplesP10		= vectDataPoints[uP10] * lfExponent;				// P10% = Sample value at 10% of sorted data.
    ptTestCell->lfSamplesQuartile1	= vectDataPoints[uQuartile1] * lfExponent;		// P25% - Quartile1 = Sample value at 25% of sorted data.
    ptTestCell->lfSamplesQuartile3	= vectDataPoints[uQuartile3] * lfExponent;		// P75% - Quartile3 = Sample value at 25% of sorted data.
    ptTestCell->lfSamplesP90		= vectDataPoints[uP90] * lfExponent;				// P90% = Sample value at 90% of sorted data.
    ptTestCell->lfSamplesP97_5		= vectDataPoints[uP97_5] * lfExponent;			// P97.5% = Sample value at 97.5% of sorted data.
    ptTestCell->lfSamplesP99_5		= vectDataPoints[uP99_5] * lfExponent;			// P99.5% = Sample value at 99.5% of sorted data.

    // Median: Sample value in the middle of sorted data
    ptTestCell->lfSamplesQuartile2=GEX_C_DOUBLE_NAN;
    if (vectDataPoints.size()>0)
        ptTestCell->lfSamplesQuartile2	= algorithms::gexMedianValue(vectDataPoints) * lfExponent;

    // Get starting & ending data values shaved by X% (typically 1%) of noise on both sides of the distribution!
    double	lfValue;
    lIndex = (3*ldSamplesValidExecs)/100;
    ptTestCell->lfSamplesStartAfterNoise = vectDataPoints[lIndex]*lfExponent;	// Starting point after the first X% (norlmally 1%) if data, aliminated few samples of noise ! (used in PAT-Man
    lIndex = ldSamplesValidExecs-lIndex;
    if(lIndex >= ldSamplesValidExecs)
        lIndex = ldSamplesValidExecs-1;
    if(lIndex < 0)
        lIndex = 0;
    ptTestCell->lfSamplesEndBeforeNoise = vectDataPoints[lIndex]*lfExponent;	// Starting point after the first X% (norlmally 1%) if data, aliminated few samples of noise ! (used in PAT-Man

    // Compute Interquartile sigma: data from quartile 1 to 3
    double	lfTotal=0;
    double	lfTotalSquare=0;
    double	lfTotalExecs = uQuartile3 - uQuartile1 + 1;
    for(lIndex = (int)uQuartile1; lIndex <= (int)uQuartile3; lIndex++)
    {
        lfValue = vectDataPoints[lIndex];
        lfTotal += lfValue;
        lfTotalSquare += (lfValue*lfValue);
    }

    // Sigma formula (only possible if at least 2 samples are available!)
    if(lfTotalExecs >= 2)
    {
        // was pow
        //lfValue = sqrt(fabs(((lfTotalExecs*lfTotalSquare) - GS_POW(lfTotal,2))/(lfTotalExecs*(lfTotalExecs-1))));
        //ptTestCell->lfSamplesSigmaInterQuartiles = lfValue*lfExponent;
        ptTestCell->lfSamplesSigmaInterQuartiles = (ptTestCell->lfSamplesQuartile3-ptTestCell->lfSamplesQuartile1)/1.349;
    }

    // Compute Decatiles (the 10 values for each group of  10% of the sorted data!)
    unsigned long	uDecatile;
    for(lIndex = 1; lIndex <= 10; lIndex++)
    {
        uDecatile = ((ldSamplesValidExecs-1)*lIndex)/10;
        ptTestCell->lfSamplesDecatiles[lIndex-1] = vectDataPoints[uDecatile]*lfExponent;;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // a) Compute number of different values (used to detect distributions with few indexed values)
    // b) Detect if serie of integer numbers.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    long	lDifferentValues=0;
    double	lfSampleValue;
    lfValue = vectDataPoints[0];
    // Reset flag
    bool lIntegerValues = true;
    for(lIndex = 1; lIndex < ldSamplesValidExecs; lIndex++)
    {
        lfSampleValue = vectDataPoints[lIndex];
        if(lfValue < lfSampleValue)
        {
            lDifferentValues++;
            lfValue = lfSampleValue;
        }

        // Check if integer numbers only or floating point values.
        if(lfValue != (double) ((int) lfValue))
            lIntegerValues = false;
    }
    ptTestCell->lDifferentValues    = lDifferentValues;
    ptTestCell->bIntegerValues      = lIntegerValues;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Compute the Skew without the noise (first X% and last X% of the data)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(ldSamplesValidExecs >= 3)
    {
        int iStartAfterNoise = 3*ldSamplesValidExecs/100;
        int iEndBeforeNoise = ldSamplesValidExecs - iStartAfterNoise;
        if(iEndBeforeNoise >= ldSamplesValidExecs)
            iEndBeforeNoise = ldSamplesValidExecs-1;
        if(iEndBeforeNoise < iStartAfterNoise)
            iEndBeforeNoise = iStartAfterNoise;

        lfTotal = lfTotalSquare = 0;
        for(lIndex = iStartAfterNoise; lIndex <= iEndBeforeNoise; lIndex++)
        {
            lfValue = vectDataPoints[lIndex];
            lfTotal += lfValue;
            lfTotalSquare += (lfValue*lfValue);
        }
        // Sigma formula
        lfTotalExecs = iEndBeforeNoise - iStartAfterNoise + 1;
        double lfMeanNoNoise = lfTotal/lfTotalExecs;
        double lfSigmaNoNoise = sqrt(fabs(((lfTotalExecs*lfTotalSquare) - (lfTotal*lfTotal))/
                                    (lfTotalExecs*(lfTotalExecs-1))));
        lfValue = 0.0;	// To hold SkewSum values.
        for(lIndex = iStartAfterNoise; lIndex <= iEndBeforeNoise; lIndex++)
            lfValue += GS_POW((vectDataPoints[lIndex]-lfMeanNoNoise)/lfSigmaNoNoise, 3.0);

        lfValue *= lfTotalExecs/((lfTotalExecs-1)*(lfTotalExecs-2));
        ptTestCell->lfSamplesSkewWithoutNoise = lfValue;
    }

    // Check if need to shave data (if outliers on Q1,Q3 +/-N*IQR is enabled)
    double	lfUp;
    double	lfLp;
    bool    lGaussian = false;

    if(bIqrOutlierRemoval && (gexReport != NULL))
    {
        // Compute Outlier limits
        lfLp= (ptTestCell->lfSamplesQuartile1 - m_fOutlierRemovalValue*(ptTestCell->lfSamplesQuartile3 - ptTestCell->lfSamplesQuartile1));// /lfExponent;
        lfUp= (ptTestCell->lfSamplesQuartile3 + m_fOutlierRemovalValue*(ptTestCell->lfSamplesQuartile3 - ptTestCell->lfSamplesQuartile1));// /lfExponent;

        // Now, remove all samples that are outside of these limits!
        gexReport->RemoveDataPoints(NULL,NULL,ptTestCell,0.0,lfUp,GEX_REMOVE_HIGHER_THAN);	// Remove data points Higher than given value...
        gexReport->RemoveDataPoints(NULL,NULL,ptTestCell,lfLp,0.0,GEX_REMOVE_LOWER_THAN);	// Remove data points Lower than given value...

    }

    ///////////////////////////////////////////////////////////////
    // Compute: Cp, Cpk based on Percentile formula
    ///////////////////////////////////////////////////////////////
    lfUp = vectDataPoints[uUp]*lfExponent;
    lfLp = vectDataPoints[uLp]*lfExponent;
    double	Cp,Cpk,CpkL,CpkH;
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();

    if(bStatsCpCpkPercentileFormula == false)
        goto skip_iqr_cpk;

    // Check the distribution type: we only apply this formula to non-normal distributions.
    if (lPatInfo)
    {
        lGaussian = patlib_IsDistributionGaussian(ptTestCell,
                                                  lPatInfo->GetRecipeOptions().iCategoryValueCount,
                                                  lPatInfo->GetRecipeOptions().bAssumeIntegerCategory,
                                                  lPatInfo->GetRecipeOptions().mMinConfThreshold);
    }
    else
        lGaussian = patlib_IsDistributionGaussian(ptTestCell);

    if (lGaussian)
        goto skip_iqr_cpk;

    for (int i=0; i<ptTestCell->MultiLimitItemCount(); ++i)
    {
        GS::Core::MultiLimitItem* lMultiLimit = ptTestCell->GetMultiLimitItem(i);
        if (!lMultiLimit)
        {
            continue;
        }
        // Reset Cp, Cpk,...
        lMultiLimit->lfCp = C_NO_CP_CPK;
        lMultiLimit->lfCpk = C_NO_CP_CPK;
        lMultiLimit->lfCpkHigh = C_NO_CP_CPK;
        lMultiLimit->lfCpkLow = C_NO_CP_CPK;

        //   BYTE	bLimitFlag;	// bit0=1 (no low limit), bit1=1 (no high limit)
        // Compute Cpk
        if(lMultiLimit->bLimitFlag & CTEST_LIMITFLG_NOLTL)
            CpkL = lMultiLimit->lfCp = C_NO_CP_CPK;	// No Low Limit
        else
        {
            if(ptTestCell->lfSamplesQuartile2 == lfLp)
                CpkL = C_INFINITE;
            else
              CpkL = (ptTestCell->lfSamplesQuartile2-lMultiLimit->lfLowLimit)/(ptTestCell->lfSamplesQuartile2-lfLp);
            if(CpkL > 1e6) CpkL = C_INFINITE;
        }

        if(lMultiLimit->bLimitFlag & CTEST_LIMITFLG_NOHTL)
            CpkH = C_NO_CP_CPK;	// No High Limit.
        else
        {
            if(lfUp == ptTestCell->lfSamplesQuartile2)
                CpkH = C_INFINITE;
            else
                CpkH = (lMultiLimit->lfHighLimit-ptTestCell->lfSamplesQuartile2)/(lfUp-ptTestCell->lfSamplesQuartile2);
            if(CpkH > 1e6) CpkH = C_INFINITE;
            if(CpkH < -1e6) CpkH = -C_INFINITE;
        }

        // Compute Cp
        if(CpkL == C_NO_CP_CPK)
            lMultiLimit->lfCpk= CpkH;
        else
        {
            if(CpkH == C_NO_CP_CPK)
                lMultiLimit->lfCpk = CpkL;
            else
            {
                if(lfUp != lfLp)
                {
                    Cpk = gex_min(CpkL,CpkH);
                    if(Cpk > 1e6) Cpk = C_INFINITE;
                    if(Cpk < -1e6) Cpk = -C_INFINITE;
                    lMultiLimit->lfCpkLow  = CpkL;
                    lMultiLimit->lfCpkHigh = CpkH;
                    lMultiLimit->lfCpk = Cpk;
                    Cp = fabs(lMultiLimit->lfHighLimit - lMultiLimit->lfLowLimit)/(lfUp - lfLp);
                    if(Cp > 1e6) Cp = C_INFINITE;
                    lMultiLimit->lfCp = Cp;
                }
                else
                {
                    // Sigma = 0, No valid Cp, Cpk!
                    lMultiLimit->lfCp = lMultiLimit->lfCpk = C_INFINITE;
                }
            }
        }
    }

skip_iqr_cpk:
    // Cleanup on exit.
    vectDataPoints.clear();
}

