/////////////////////////////////////////////////////////////////////////////
// Codes to handle PAT outlier filtering
/////////////////////////////////////////////////////////////////////////////
#ifndef SOURCEFILE_PAT_SHAPE_ANALYSIS_CPP
#define SOURCEFILE_PAT_SHAPE_ANALYSIS_CPP

#include <math.h>

#include "pat_info.h"
#include "pat_definition.h"
#include "pat_part_filter.h"
#include "patman_lib.h"
#include "browser.h"
#include "report_build.h"
#include "cstats.h"
#include <gqtl_log.h>

#include <QProgressDialog>
// Galaxy QT libraries
#include "gqtl_sysutils.h"

#include "tb_toolbox.h"	// Examinator ToolBox
#include "report_build.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include <gtm_testerwindow.h>
//#include "gex_pat_constants_extern.h"
#include "stats_engine.h"
#include "r_data.h"
#include "r_vector.h"
#include "shape_identifier.h"
#include "engine.h"

/////////////////////////////////////////////////////////////////////////////
// Compute space used by each quartile (dropping few percent noise at each extremity
/////////////////////////////////////////////////////////////////////////////
void patlib_ComputeQuartilePercentage(const CTest *ptTestCell,double *lfQ1Pct,double *lfQ2Pct,double *lfQ3Pct,double *lfQ4Pct)
{
    *lfQ1Pct = (ptTestCell->lfSamplesQuartile1-ptTestCell->lfSamplesStartAfterNoise) * 100 /
            (ptTestCell->lfSamplesEndBeforeNoise-ptTestCell->lfSamplesStartAfterNoise);

    if (ptTestCell->lfSamplesQuartile2 != ptTestCell->lfSamplesQuartile1)
    {
        *lfQ2Pct = (ptTestCell->lfSamplesQuartile2-ptTestCell->lfSamplesQuartile1) * 100 /
            (ptTestCell->lfSamplesEndBeforeNoise-ptTestCell->lfSamplesStartAfterNoise);
    }
    else
    {
        *lfQ2Pct = 0.0;
    }

    if (ptTestCell->lfSamplesQuartile3 != ptTestCell->lfSamplesQuartile2)
    {
        *lfQ3Pct = (ptTestCell->lfSamplesQuartile3-ptTestCell->lfSamplesQuartile2) * 100 /
            (ptTestCell->lfSamplesEndBeforeNoise-ptTestCell->lfSamplesStartAfterNoise);
    }
    else
    {
        *lfQ3Pct = 0.0;
    }

    *lfQ4Pct = (ptTestCell->lfSamplesEndBeforeNoise-ptTestCell->lfSamplesQuartile3) * 100 /
            (ptTestCell->lfSamplesEndBeforeNoise-ptTestCell->lfSamplesStartAfterNoise);

    // If one quartile is empty, at least give it a tiny value!
    if(!(*lfQ1Pct)) *lfQ1Pct = 1e-6;
    if(!(*lfQ2Pct)) *lfQ2Pct = 1e-6;
    if(!(*lfQ3Pct)) *lfQ3Pct = 1e-6;
    if(!(*lfQ4Pct)) *lfQ4Pct = 1e-6;
}

/////////////////////////////////////////////////////////////////////////////
// Identify distribution type: NORMAL with left tail
/////////////////////////////////////////////////////////////////////////////
int	patlib_isDistributionNormalLeftTailed(const CTest *ptTestCell)
{
    // Check if it takes more consecutive histograms cells on the left to build up 10% of the histogram
    // Note: assumes histogram array has been done over test results, non-cumulative.
    double	lfLeftPercent=0;
    int		iLeftCellsRequired=0;
    int		lIndex;

    // Compute lIgnoreSpace to handle distributions with a lot of hole (GCORE-2831)
    int lEmptyCount = 0;
    int lMaxContiguousEmpty = 0;
    int lContiguousEmpty = 0;
    bool lIgnoreSpace = false;
    for(lIndex=0; lIndex < TEST_HISTOSIZE; ++lIndex)
    {
        if(ptTestCell->lHistogram[lIndex] == 0)
        {
            ++lEmptyCount;
            ++lContiguousEmpty;
        }
        else
        {
            lContiguousEmpty = 0;
        }
        // keep track of max
        if (lContiguousEmpty > lMaxContiguousEmpty)
            lMaxContiguousEmpty = lContiguousEmpty;
    }

    // If never more than 2 contiguous holes and holes represents at least 1/3 of the distrib
    // then ignore spaces
    if ((lMaxContiguousEmpty <= 2) && (lEmptyCount > (int)(TEST_HISTOSIZE/3)))
        lIgnoreSpace = true;

    for(lIndex=0;lIndex<TEST_HISTOSIZE;lIndex++)
    {
        // Keep track of empty histogram cell.
        if(ptTestCell->lHistogram[lIndex] == 0)
        {
            if (!lIgnoreSpace)
            {
                lfLeftPercent = 0;
                iLeftCellsRequired = 0;
            }
        }
        else
        {
            // Compute cumul of distribution seen until next gap or reach the threshold
            lfLeftPercent += (100.0*ptTestCell->lHistogram[lIndex])/(ptTestCell->ldSamplesValidExecs);
            iLeftCellsRequired++;
        }
        // Stop as soon as we've got the cumul of histogram cells that reached the threshold.
        if(lfLeftPercent > 10)
            break;
    }

    // Now check how many cells are required coming from the right side of the histogram...
    int		iRightCellsRequired=0;
    double	lfRightPercent=0;
    for(lIndex=TEST_HISTOSIZE-1;lIndex >= 0;lIndex--)
    {
        // Keep track of empty histogram cell.
        if(ptTestCell->lHistogram[lIndex] == 0)
        {
            if (!lIgnoreSpace)
            {
                lfRightPercent = 0;
                iRightCellsRequired = 0;
            }
        }
        else
        {
            // Compute cumul of distribution seen until next gap or reach the threshold
            lfRightPercent += (100.0*ptTestCell->lHistogram[lIndex])/(ptTestCell->ldSamplesValidExecs);
            iRightCellsRequired++;
        }
        // Stop as soon as we've got the cumul of histogram cells that reached the threshold.
        if(lfRightPercent > 10)
            break;
    }

    // Check if left-tailed
    if(iLeftCellsRequired > iRightCellsRequired)
        return PATMAN_LIB_SHAPE_GAUSSIAN_LEFT;

    // If we have the same number of cells, then compare the percentages
    if((iLeftCellsRequired == iRightCellsRequired) && (lfLeftPercent < lfRightPercent))
        return PATMAN_LIB_SHAPE_GAUSSIAN_LEFT;


    // Not Gaussian Left tailed
    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// Identify distribution type: NORMAL with right tail
/////////////////////////////////////////////////////////////////////////////
int	patlib_isDistributionNormalRightTailed(const CTest *ptTestCell)
{
    // If not left-tailed, then its right-tailed!
    if(patlib_isDistributionNormalLeftTailed(ptTestCell) < 0)
        return PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT;

    // Not Gaussian Right tailed
    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// Identify distribution direction:
// -1: left tailed, 0: not tailed, +1: right tailed
/////////////////////////////////////////////////////////////////////////////
int	patlib_getDistributionNormalDirection(const CTest *ptTestCell)
{
    // if shape has been computed use it...
    if (ptTestCell->GetDistribution() != PATMAN_LIB_SHAPE_UNSET)
    {
        if (ptTestCell->GetDistribution() == PATMAN_LIB_SHAPE_GAUSSIAN)
            return 0;
        else if (ptTestCell->GetDistribution() == PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT)
            return 1;
        else if (ptTestCell->GetDistribution() == PATMAN_LIB_SHAPE_GAUSSIAN_LEFT)
            return -1;
    }

    // Check if close to ideal Gaussian
    if(fabs(ptTestCell->lfSamplesSkewWithoutNoise) < 0.5)
        return 0;	// Very little skew: not tailed.

    // If not left-tailed, then its right-tailed!
    if(patlib_isDistributionNormalLeftTailed(ptTestCell) < 0)
        return 1;	// Right tailed

    // Left tailed
    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// Identify distribution type: NORMAL
/////////////////////////////////////////////////////////////////////////////
int	patlib_isDistributionNormal(const CTest *ptTestCell)
{
    // If Kurtosis too low, it's not gaussian enough
    if(ptTestCell->lfSamplesKurt < -0.9)
        return PATMAN_LIB_SHAPE_UNKNOWN;

    // Check if close to ideal Gaussian
    if(fabs(ptTestCell->lfSamplesSkewWithoutNoise) < 0.5)
        return PATMAN_LIB_SHAPE_GAUSSIAN;

    // Check if Left tailed Gaussian
    if(patlib_isDistributionNormalLeftTailed(ptTestCell) >= 0)
        return PATMAN_LIB_SHAPE_GAUSSIAN_LEFT;

    // Then can only be Right tailed Gaussian
    return patlib_isDistributionNormalRightTailed(ptTestCell);
}

/////////////////////////////////////////////////////////////////////////////
// Identify distribution type: Log normal (Left tailed) distribution
/////////////////////////////////////////////////////////////////////////////
int	patlib_isDistributionLogNormalLeftTailed(double lfQ1,double lfQ2,double lfQ3,double lfQ4)
{
    // Not enough tail for a LogNormal
    if(lfQ1 < 45)
        return -1;

    // Head's slope not high enough for a Lognormal.
    if(lfQ4 > 15)
        return -1;

    // If each quartile gets smaller in range space, then yes it's a log normal!
    if(lfQ1 > lfQ2 && lfQ2 > lfQ3 && lfQ3 > lfQ4)
        return PATMAN_LIB_SHAPE_LOGNORMAL_LEFT;

    double lfRatio = lfQ2/lfQ3;
    if(lfRatio < 1) lfRatio = 1/lfRatio;

    // If Last quartile used a very small range, then yes it's definitely a LogNormal!
    if(lfQ4 < 2 && (lfRatio < 1.8))
        return PATMAN_LIB_SHAPE_LOGNORMAL_LEFT;

    if(lfQ1 > lfQ2 &&  lfQ2 > lfQ3)
    {
        // Lognormal
        if(lfRatio > 1.8)
            return PATMAN_LIB_SHAPE_LOGNORMAL_LEFT;
        // If ratio not high enough, but still Q2 & Q3 very small, then this is fine too.
        if(gex_max(lfQ2,lfQ3) <= 5)
            return PATMAN_LIB_SHAPE_LOGNORMAL_LEFT;
    }

    // Not log normal-left tailed.
    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// Identify distribution type: Log normal (Right tailed) distribution
/////////////////////////////////////////////////////////////////////////////
int	patlib_isDistributionLogNormalRightTailed(double lfQ1,double lfQ2,double lfQ3,double lfQ4)
{
    // Swap Quartiles and call function to detect a left tail!
    double lfSwap = lfQ1;
    lfQ1 = lfQ4;
    lfQ4 = lfSwap;

    lfSwap = lfQ2;
    lfQ2 = lfQ3;
    lfQ3  = lfSwap;

    if(patlib_isDistributionLogNormalLeftTailed(lfQ1,lfQ2,lfQ3,lfQ4) >= 0)
        return PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT;

    // Not Lognormal right-tailed
    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// Identify LogNormal distribution direction
// -1: left tailed, +1: right tailed
/////////////////////////////////////////////////////////////////////////////
int	patlib_getDistributionLogNormalDirection(const CTest *ptTestCell)
{
    // if shape has been computed use it...
    if (ptTestCell->GetDistribution() != PATMAN_LIB_SHAPE_UNSET)
    {
        if (ptTestCell->GetDistribution() == PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT)
            return 1;
        else if (ptTestCell->GetDistribution() == PATMAN_LIB_SHAPE_LOGNORMAL_LEFT)
            return -1;
    }
    double lfQ1,lfQ2,lfQ3,lfQ4;
    // Compute space used by each quartile (dropping few percent noise at each extremity
    patlib_ComputeQuartilePercentage(ptTestCell,&lfQ1,&lfQ2,&lfQ3,&lfQ4);

    if(lfQ1+lfQ2 > lfQ3+lfQ4)
        return -1;	// Left tailed

    // Right tailed
    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Identify distribution type: Log normal (Left or Right tailed) distribution
/////////////////////////////////////////////////////////////////////////////
int	patlib_isDistributionLogNormal(double lfQ1, double lfQ2,
                                   double lfQ3, double lfQ4)
{
    // First check if Noraml Left-tailed
    if(patlib_isDistributionLogNormalLeftTailed(lfQ1,lfQ2,lfQ3,lfQ4) >= 0)
        return PATMAN_LIB_SHAPE_LOGNORMAL_LEFT;

    // NO...then check if Normal Right-tailed
    return patlib_isDistributionLogNormalRightTailed(lfQ1,lfQ2,lfQ3,lfQ4);
}

/////////////////////////////////////////////////////////////////////////////
// Computes & returns the Mean and sigma of each of the two modes
// (only call if this is a Bi-Modal distribution: PATMAN_LIB_SHAPE_BIMODAL!)
/////////////////////////////////////////////////////////////////////////////
int	patlib_getDistributionBiModalStats(const CTest * ptTestCell, double &lfMean1, double &lfSigma1,
                                       double &lfMean2, double &lfSigma2, double lfExponent,
                                       GS::Gex::PATPartFilter * lPartFilter /*= NULL*/)
{
    // We've got to compute statistics for two modes, the cut-off limit between the two modes is computed

    double lModalSplitPos = ptTestCell->GetBimodalSplitValue();
    // Scale Modal split point
    lModalSplitPos /= lfExponent;

    // First: clear variables.
    lfMean1 = lfMean2 = lModalSplitPos;
    lfSigma1 = lfSigma2 = 0;

    double	lfSumX_Mode1=0;
    double	lfSumX2_Mode1=0;
    int		lTotalSamples_Mode1=0;
    double	lfSumX_Mode2=0;
    double	lfSumX2_Mode2=0;
    int		lTotalSamples_Mode2=0;
    double	lfValue;
    int		iIndex;

    for(iIndex=0;iIndex<ptTestCell->m_testResult.count();iIndex++)
    {
        if(ptTestCell->m_testResult.isValidResultAt(iIndex))
        {
            // If parts filter disabled, or part matches the filter,
            // the run is included in the filtered parts, use it to compute statistics
            if (lPartFilter == NULL || lPartFilter->IsFiltered(iIndex))
            {
                if (ptTestCell->m_testResult.isMultiResultAt(iIndex))
                {
                    for (int nCount = 0; nCount < ptTestCell->m_testResult.at(iIndex)->count(); ++nCount)
                    {
                        lfValue = ptTestCell->m_testResult.at(iIndex)->multiResultAt(nCount);

                        if(lfValue <= lModalSplitPos)
                        {
                            // Mode#1: to the Left of the cut-off point computed as bi-modal middle point
                            lfSumX_Mode1  += lfValue;
                            lfSumX2_Mode1 += (lfValue*lfValue);

                            // Keep track of total samples in this mode
                            lTotalSamples_Mode1++;
                        }
                        else
                        {
                            // Mode#2: to the Right of the cut-off point computed as bi-modal middle point
                            lfSumX_Mode2  += lfValue;
                            lfSumX2_Mode2 += (lfValue*lfValue);

                            // Keep track of total samples in this mode
                            lTotalSamples_Mode2++;
                        }
                    }
                }
                else
                {
                    lfValue = ptTestCell->m_testResult.resultAt(iIndex);

                    if(lfValue <= lModalSplitPos)
                    {
                        // Mode#1: to the Left of the cut-off point computed as bi-modal middle point
                        lfSumX_Mode1  += lfValue;
                        lfSumX2_Mode1 += (lfValue*lfValue);

                        // Keep track of total samples in this mode
                        lTotalSamples_Mode1++;
                    }
                    else
                    {
                        // Mode#2: to the Right of the cut-off point computed as bi-modal middle point
                        lfSumX_Mode2  += lfValue;
                        lfSumX2_Mode2 += (lfValue*lfValue);

                        // Keep track of total samples in this mode
                        lTotalSamples_Mode2++;
                    }
                }
            }
        }
    }

    // Compute Mean & Sigma for Mode#1
    if(lTotalSamples_Mode1 > 0)
        lfMean1 = lfSumX_Mode1/lTotalSamples_Mode1;
    if(lTotalSamples_Mode1 >= 2)
        lfSigma1 = sqrt(fabs((((double)lTotalSamples_Mode1*lfSumX2_Mode1) - pow(lfSumX_Mode1,2.0))
                             /((double)lTotalSamples_Mode1*((double)lTotalSamples_Mode1-1))));

    // Compute Mean & Sigma for Mode#2
    if(lTotalSamples_Mode2 > 0)
        lfMean2 = lfSumX_Mode2/lTotalSamples_Mode2;
    if(lTotalSamples_Mode2 >= 2)
        lfSigma2 = sqrt(fabs((((double)lTotalSamples_Mode2*lfSumX2_Mode2) - pow(lfSumX_Mode2,2.0))
                             /((double)lTotalSamples_Mode2*((double)lTotalSamples_Mode2-1))));

    // Scale statistics to be in line with data.
    lfMean1 *= lfExponent;
    lfMean2 *= lfExponent;
    lfSigma1 *= lfExponent;
    lfSigma2 *= lfExponent;
    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Identify distribution type: Multi-Modal distribution
/////////////////////////////////////////////////////////////////////////////
int	patlib_isDistributionMultiModal(CTest *ptTestCell,double lfQ1,double lfQ2,double lfQ3,double lfQ4)
{
    bool	bLooksBiModal=false;
    int lInterModeMinGap = 5;
    // check if INTER_MODE_MIN_GAP is defined then use it
    bool lOk;
    int lEnv = QString(qgetenv("INTER_MODE_MIN_GAP")).toInt(&lOk);
    if (lOk)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("INTER_MODE_MIN_GAP: %1").arg(lEnv).toLatin1().data());
        lInterModeMinGap = lEnv;
    }
    // Check if clear bi-modal: two distinct modes with wide gap in-between
    if(lfQ1 < 30 && lfQ4 < 30 && (lfQ2 > 50 || lfQ3 > 50))
    {
        // This looks a bi-modal, but we need to confirm over the histogram analysis below!
        bLooksBiModal = true;
    }

    // Check if enough middle histogram classes are empty to show to separate distributions (probably one big mode, and one small mode)
    // Note: assumes histogram array has been done over test results, non-cumulative.
    double	lfPercent=0;
    int		iEmptyCell=0;
    bool	bFirstModeFound=false;
    int		iIndex;
    for(iIndex=0;iIndex<TEST_HISTOSIZE;iIndex++)
    {
        // Keep track of empty histogram cell.
        if(ptTestCell->lHistogram[iIndex] == 0)
        {
            // Keep track of number of consecutive empty cell....to detect separate Bi-modal distribution!
            iEmptyCell++;

            // First mode found, all we need is to find second mode and a gap between them!
            if(lfPercent >= 6.0)
            {
                bFirstModeFound = true;
                iEmptyCell = 0;
            }

            lfPercent = 0;
        }
        else
        {
            // Compute cumul of distribution seen until next gap.
            lfPercent += (100.0*ptTestCell->lHistogram[iIndex])/(ptTestCell->ldSamplesValidExecs);
        }

        // If at least two modes of 6% minimum each, and at least 5 of the 18 consecutive cells are empty, this is a bi-modal!
        if(bFirstModeFound && lfPercent >= 6.0 && iEmptyCell >= lInterModeMinGap)
        {
            if(bLooksBiModal)
            {
                // Saves cut-off limit between the two modes.
                ptTestCell->SetBimodalSplitValue(
                            ptTestCell->GetCurrentLimitItem()->lfHistogramMin +
                            ((iIndex-(iEmptyCell/2))*(ptTestCell->GetCurrentLimitItem()->lfHistogramMax -
                                                      ptTestCell->GetCurrentLimitItem()->lfHistogramMin)/TEST_HISTOSIZE));
                return PATMAN_LIB_SHAPE_BIMODAL;	// Clear bi-modal with each mode apart!
            }
            else
                return PATMAN_LIB_SHAPE_MULTIMODAL;
        }
    }

    // Check if we have a "Down-up" curve with significant slope for a semi-merged bi-modal distribution.
    double	lfMax=0;
    for(iIndex=0;iIndex<TEST_HISTOSIZE;iIndex++)
        lfMax = gex_max(lfMax,ptTestCell->lHistogram[iIndex]);

    // first, get the highest histogram bar
    double	lfValue;
    int		iIndexPick1,iIndexPick2;
    int		iIndexArea;
    double	lfArea=0;
    double	lfHole=0;
    bool	bFoundPick2;

    for(iIndexPick1=0;iIndexPick1<TEST_HISTOSIZE-2;iIndexPick1++)
    {
        lfValue = ptTestCell->lHistogram[iIndexPick1];
        if(lfValue >= lfMax/2)
        {
            // We got the first high bar
            bFoundPick2 = false;
            iIndexPick2=iIndexPick1+2;

            // If got to 0, ignore this down-up cycle, because this algorithm only applies to merged bi-modal.
            if(ptTestCell->lHistogram[iIndexPick1+1] == 0)
                break;
            // We need a Down-up cycle!
            if(ptTestCell->lHistogram[iIndexPick1+1] >= ptTestCell->lHistogram[iIndexPick1])
                goto next_Pick1;

            while(iIndexPick2<TEST_HISTOSIZE)
            {
                lfValue = ptTestCell->lHistogram[iIndexPick2];

                // If got to 0, ignore this down-up cycle, because this algorithm only applies to merged bi-modal.
                if(lfValue == 0)
                    break;

                if(lfValue >= lfMax/2)
                {
                    // We got the first high bar
                    bFoundPick2 = true;
                    break;
                }

                // Next histogram cell
                iIndexPick2++;
            };
            // Didn't find two picks high enough...
            if(bFoundPick2 == false)
                return -1;

            // Need at least 2 cells bewteen the two picks; one cell is not enough to be 100% sure
            if(iIndexPick2-iIndexPick1 <= 2)
                goto next_Pick1;

            // Compute the area between the two picks and see if it is low enough...
            lfArea = 0;
            for(iIndexArea=iIndexPick1+1;iIndexArea<iIndexPick2;iIndexArea++)
                lfArea += ptTestCell->lHistogram[iIndexArea];

            // Compute the hole space
            lfHole = ((iIndexPick2-iIndexPick1-1)*ptTestCell->lHistogram[iIndexPick1]);
            lfHole += ((ptTestCell->lHistogram[iIndexPick2]-ptTestCell->lHistogram[iIndexPick1])*(iIndexPick2-iIndexPick1-1)/2);
            if(lfArea <= lfHole/2)
                return PATMAN_LIB_SHAPE_MULTIMODAL;
        }
next_Pick1:;
    }


    // Not Multi-Modal
    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// Identify Clamped distribution direction
// -1: left tailed, +1: right tailed
/////////////////////////////////////////////////////////////////////////////
int	patlib_getDistributionClampedDirection(const CTest *ptTestCell)
{
    if (ptTestCell->GetDistribution() != PATMAN_LIB_SHAPE_UNSET)
    {
        if (ptTestCell->GetDistribution() == PATMAN_LIB_SHAPE_CLAMPED_RIGHT)
            return 1;
        else if (ptTestCell->GetDistribution() == PATMAN_LIB_SHAPE_CLAMPED_LEFT)
            return -1;
    }
    if(patlib_isDistributionClamped(ptTestCell) == PATMAN_LIB_SHAPE_CLAMPED_LEFT)
        return -1;	// Left clamped

    // Right clamped
    return 1;
}

//////////////////////////////////////////////////////////////////////
// Identify distribution type: Clamped distribution
/////////////////////////////////////////////////////////////////////////////
int	patlib_isDistributionClamped(const CTest *ptTestCell)
{
    // Compute the highest bar in the histogram.
    int	iHighestRank = 0;
    int	iIndex;
    for(iIndex=0;iIndex<TEST_HISTOSIZE;iIndex++)
        iHighestRank = gex_max(ptTestCell->lHistogram[iIndex],iHighestRank);

    // No data!
    if(iHighestRank == 0)
        return -1;

    // Check if clamped LEFT
    double	lfValue = ptTestCell->lHistogram[0]*100.0/(double)iHighestRank;
    if(lfValue >= 50)
        return PATMAN_LIB_SHAPE_CLAMPED_LEFT;

    // Check if clamped RIGHT
    lfValue = ptTestCell->lHistogram[TEST_HISTOSIZE-1]*100.0/(double)iHighestRank;
    if(lfValue >= 50)
        return PATMAN_LIB_SHAPE_CLAMPED_RIGHT;

    // Not Clamped
    return -1;
}

//////////////////////////////////////////////////////////////////////
// Identify distribution type: DualClamped distribution
/////////////////////////////////////////////////////////////////////////////
int	patlib_isDistributionDualClamped(const CTest *ptTestCell,
                                     double /*lfQ1*/, double /*lfQ2*/,
                                     double /*lfQ3*/, double /*lfQ4*/)
{
    // Compute the highest bar in the histogram.
    int	iHighestRank = 0;
    int	iIndex;
    for(iIndex=0;iIndex<TEST_HISTOSIZE;iIndex++)
        iHighestRank = gex_max(ptTestCell->lHistogram[iIndex],iHighestRank);

    // No data!
    if(iHighestRank == 0)
        return -1;

    // Check if clamped LEFT
    bool	bLeftClamped = false;
    double	lfValue = ptTestCell->lHistogram[0]*100.0/(double)iHighestRank;
    if(lfValue >= 50)
        bLeftClamped = true;

    // Check if clamped RIGHT
    bool	bRightClamped = false;
    lfValue = ptTestCell->lHistogram[TEST_HISTOSIZE-1]*100.0/(double)iHighestRank;
    if(lfValue >= 50)
        bRightClamped = true;

    if(bLeftClamped && bRightClamped)
        return PATMAN_LIB_SHAPE_DOUBLECLAMPED;

    // Not Clamped
    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// Tells type of distribution
/////////////////////////////////////////////////////////////////////////////
QString patlib_GetDistributionShortName(int iDistributionType)
{
    switch(iDistributionType)
    {
        case PATMAN_LIB_SHAPE_GAUSSIAN:
            return "Gaussian";
            break;

        case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
            return "Gaussian_L";
            break;

        case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
            return "Gaussian_R";
            break;

        case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
            return "Log_L";
            break;

        case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
            return "Log_R";
            break;

        case PATMAN_LIB_SHAPE_BIMODAL:
            return "B_Modal";
            break;

        case PATMAN_LIB_SHAPE_MULTIMODAL:
            return "M_Modal";
            break;

        case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
            return "Clamp_L";
            break;

        case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
            return "Clamp_R";
            break;

        case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
            return "Clamp_2";
            break;

        case PATMAN_LIB_SHAPE_CATEGORY:
            return "Categories";
            break;

        case PATMAN_LIB_SHAPE_UNKNOWN:
        default:
            return "Unknown";
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Tells type of distribution
/////////////////////////////////////////////////////////////////////////////
int patlib_GetDistributionID(QString strDistributionName)
{
    if(strDistributionName == "Gaussian")
        return PATMAN_LIB_SHAPE_GAUSSIAN;
    if(strDistributionName == "Gaussian_L")
        return PATMAN_LIB_SHAPE_GAUSSIAN_LEFT;
    if(strDistributionName == "Gaussian_R")
        return PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT;
    if(strDistributionName == "Log_L")
        return PATMAN_LIB_SHAPE_LOGNORMAL_LEFT;
    if(strDistributionName == "Log_R")
        return PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT;
    if(strDistributionName == "B_Modal")
        return PATMAN_LIB_SHAPE_BIMODAL;
    if(strDistributionName == "M_Modal")
        return PATMAN_LIB_SHAPE_MULTIMODAL;
    if(strDistributionName == "Clamp_L")
        return PATMAN_LIB_SHAPE_CLAMPED_LEFT;
    if(strDistributionName == "Clamp_R")
        return PATMAN_LIB_SHAPE_CLAMPED_RIGHT;
    if(strDistributionName == "Clamp_2")
        return PATMAN_LIB_SHAPE_DOUBLECLAMPED;
    if(strDistributionName == "Categories")
        return PATMAN_LIB_SHAPE_CATEGORY;
    if(strDistributionName == "Unknown")
        return PATMAN_LIB_SHAPE_UNKNOWN;

    return PATMAN_LIB_SHAPE_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////
// Tells type of distribution
/////////////////////////////////////////////////////////////////////////////
QString patlib_GetDistributionName(int iDistributionType)
{
    switch(iDistributionType)
    {
        case PATMAN_LIB_SHAPE_GAUSSIAN:
            return "Gaussian";
            break;

        case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
            return "Gaussian (Left tailed)";
            break;

        case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
            return "Gaussian (Right tailed)";
            break;

        case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
            return "LogNormal (Left tailed)";
            break;

        case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
            return "LogNormal (Right tailed)";
            break;

        case PATMAN_LIB_SHAPE_BIMODAL:
            return "Bi-Modal (clear modes)";
            break;

        case PATMAN_LIB_SHAPE_MULTIMODAL:
            return "Multi-Modal";
            break;

        case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
            return "Clamped (Left)";
            break;

        case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
            return "Clamped (Right)";
            break;

        case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
            return "Clamped (Both sides)";
            break;

        case PATMAN_LIB_SHAPE_CATEGORY:
            return "Categories";
            break;

        case PATMAN_LIB_SHAPE_ERROR:
            return "Error";
            break;

        case PATMAN_LIB_SHAPE_UNKNOWN:
        default:
            return "Unknown";
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Tells type of severity
/////////////////////////////////////////////////////////////////////////////
QString patlib_GetSeverityName(int iSeverityType)
{
    switch(iSeverityType)
    {
        case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR:
            return "Near";
            break;

        case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM:
            return "Medium";
            break;

        case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR:
            return "Far";
            break;

        default:
            return "Unknown";
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Tells if current distribution mismatches historical one (not a stringent test, otherwise too many false alarms would appear!).
/////////////////////////////////////////////////////////////////////////////
bool	patlib_IsDistributionMatchingHistory(int iDistributionType_History,int iDistributionType_Current)
{
    // Check if perfect match
    if(iDistributionType_History == iDistributionType_Current)
        return true;

    // Check for small mismatch
    bool	bSmallMismatch = false;
    switch(iDistributionType_History)
    {
        case PATMAN_LIB_SHAPE_GAUSSIAN:
            if(iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN_LEFT ||
                iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT ||
                iDistributionType_Current == PATMAN_LIB_SHAPE_MULTIMODAL)
                bSmallMismatch = true;
            break;

        case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
            if(iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN ||
                iDistributionType_Current == PATMAN_LIB_SHAPE_MULTIMODAL)
                bSmallMismatch = true;
            break;

        case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
            if(iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN ||
                iDistributionType_Current == PATMAN_LIB_SHAPE_MULTIMODAL)
                bSmallMismatch = true;
            break;

        case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
            if(iDistributionType_Current == PATMAN_LIB_SHAPE_CLAMPED_RIGHT)
                bSmallMismatch = true;
            break;

        case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
            if(iDistributionType_Current == PATMAN_LIB_SHAPE_CLAMPED_LEFT)
                bSmallMismatch = true;
            break;

        case PATMAN_LIB_SHAPE_BIMODAL:
            break;

        case PATMAN_LIB_SHAPE_MULTIMODAL:
            if(iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN_LEFT ||
                iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT ||
                iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN)
                bSmallMismatch = true;
            break;

        case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
            if(iDistributionType_Current == PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT ||
                iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT ||
                iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN)
                bSmallMismatch = true;
            break;

        case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
            if(iDistributionType_Current == PATMAN_LIB_SHAPE_LOGNORMAL_LEFT ||
                iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN_LEFT ||
                iDistributionType_Current == PATMAN_LIB_SHAPE_GAUSSIAN)
                bSmallMismatch = true;
            break;

        case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
            break;

        case PATMAN_LIB_SHAPE_CATEGORY:
            break;

        case PATMAN_LIB_SHAPE_UNKNOWN:
        default:
            break;
    }

    if(bSmallMismatch)
        return true;	// Small mis-match, soo do not flag it!
    else
        return false;
}

/////////////////////////////////////////////////////////////////////////////
// Tells the outlier Tail or Head factor used for a given distribution
/////////////////////////////////////////////////////////////////////////////
double patlib_GetOutlierFactor(const CPatInfo * lPatContext, CPatDefinition *ptPatDef, int iSite,
                               bool bHeadFactor)
{
    double	lfOutlierFactor;
    if(bHeadFactor)
        lfOutlierFactor = ptPatDef->m_lfOutlierNFactor;
    else
        lfOutlierFactor = ptPatDef->m_lfOutlierTFactor;

    // If custom Head/Tail factor specified, then return it
    if(lfOutlierFactor)
        return lfOutlierFactor;

    int iSeverityLimit = ptPatDef->m_iOutlierLimitsSet;

    // Template Head/Tail factor used, then find out which one!
    int iRuleType = ptPatDef->mOutlierRule;

    do
    {
        switch(iRuleType)
        {
            case GEX_TPAT_RULETYPE_SMARTID:
                // Set rule to shape detected, then return to above 'switch'!
                switch(ptPatDef->mDynamicLimits[iSite].mDistributionShape)
                {
                    case PATMAN_LIB_SHAPE_GAUSSIAN:
                        iRuleType = GEX_TPAT_RULETYPE_GAUSSIANID;
                        break;

                    case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                    case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                        iRuleType = GEX_TPAT_RULETYPE_GAUSSIANTAILID;
                        break;

                    case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                    case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                        iRuleType = GEX_TPAT_RULETYPE_LOGNORMALID;
                        break;

                    case PATMAN_LIB_SHAPE_BIMODAL:
                        iRuleType = GEX_TPAT_RULETYPE_BIMODALID;
                        break;

                    case PATMAN_LIB_SHAPE_MULTIMODAL:
                        iRuleType = GEX_TPAT_RULETYPE_MULTIMODALID;
                        break;

                    case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                    case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                        iRuleType = GEX_TPAT_RULETYPE_CLAMPEDID;
                        break;

                    case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                        iRuleType = GEX_TPAT_RULETYPE_DUALCLAMPEDID;
                        break;

                    case PATMAN_LIB_SHAPE_CATEGORY:
                        iRuleType = GEX_TPAT_RULETYPE_CATEGORYID;
                        break;

                    case PATMAN_LIB_SHAPE_UNKNOWN:
                        if(lfOutlierFactor > 0)
                            return lfOutlierFactor;
                        if(bHeadFactor)
                            return lPatContext->GetRecipeOptions().lfSmart_HeadUnknown[iSeverityLimit];
                        else
                            return lPatContext->GetRecipeOptions().lfSmart_TailUnknown[iSeverityLimit];
                        break;

                    default:
                        iRuleType = GEX_TPAT_RULETYPE_IGNOREID;
                        break;
                }
                break;

            case GEX_TPAT_RULETYPE_SIGMAID:
            case GEX_TPAT_RULETYPE_ROBUSTSIGMAID:
            case GEX_TPAT_RULETYPE_Q1Q3IQRID:
            case GEX_TPAT_RULETYPE_LIMITSID:
            case GEX_TPAT_RULETYPE_NEWLIMITSID:
            case GEX_TPAT_RULETYPE_RANGEID:
                return lfOutlierFactor;
                break;

            case GEX_TPAT_RULETYPE_GAUSSIANID:
                if(lfOutlierFactor>0)
                    return lfOutlierFactor;
                if(bHeadFactor)
                    return lPatContext->GetRecipeOptions().lfSmart_HeadGaussian[iSeverityLimit];
                else
                    return lPatContext->GetRecipeOptions().lfSmart_TailGaussian[iSeverityLimit];

            case GEX_TPAT_RULETYPE_GAUSSIANTAILID:
                if(lfOutlierFactor>0)
                    return lfOutlierFactor;
                if(bHeadFactor)
                    return lPatContext->GetRecipeOptions().lfSmart_HeadGaussianTailed[iSeverityLimit];
                else
                    return lPatContext->GetRecipeOptions().lfSmart_TailGaussianTailed[iSeverityLimit];

            case GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID:
                if(lfOutlierFactor>0)
                    return lfOutlierFactor;
                if(bHeadFactor)
                    return lPatContext->GetRecipeOptions().lfSmart_HeadGaussianDoubleTailed[iSeverityLimit];
                else
                    return lPatContext->GetRecipeOptions().lfSmart_TailGaussianDoubleTailed[iSeverityLimit];

            case GEX_TPAT_RULETYPE_LOGNORMALID:
                if(lfOutlierFactor>0)
                    return lfOutlierFactor;
                if(bHeadFactor)
                    return lPatContext->GetRecipeOptions().lfSmart_HeadLogNormal[iSeverityLimit];
                else
                    return lPatContext->GetRecipeOptions().lfSmart_TailLogNormal[iSeverityLimit];

            case GEX_TPAT_RULETYPE_BIMODALID:
                    return lfOutlierFactor;

            case GEX_TPAT_RULETYPE_MULTIMODALID:
                if(lfOutlierFactor>0)
                    return lfOutlierFactor;
                if(bHeadFactor)
                    return lPatContext->GetRecipeOptions().lfSmart_HeadMultiModal[iSeverityLimit];
                else
                    return lPatContext->GetRecipeOptions().lfSmart_TailMultiModal[iSeverityLimit];

            case GEX_TPAT_RULETYPE_CLAMPEDID:
                if(lfOutlierFactor>0)
                    return lfOutlierFactor;
                if(bHeadFactor)
                    return lPatContext->GetRecipeOptions().lfSmart_HeadClamped[iSeverityLimit];
                else
                    return lPatContext->GetRecipeOptions().lfSmart_TailClamped[iSeverityLimit];

            case GEX_TPAT_RULETYPE_DUALCLAMPEDID:
                if(lfOutlierFactor>0)
                    return lfOutlierFactor;
                if(bHeadFactor)
                    return lPatContext->GetRecipeOptions().lfSmart_HeadDoubleClamped[iSeverityLimit];
                else
                    return lPatContext->GetRecipeOptions().lfSmart_TailDoubleClamped[iSeverityLimit];

            case GEX_TPAT_RULETYPE_CATEGORYID:
                if(lfOutlierFactor>0)
                    return lfOutlierFactor;
                if(bHeadFactor)
                    return lPatContext->GetRecipeOptions().lfSmart_HeadCategory[iSeverityLimit];
                else
                    return lPatContext->GetRecipeOptions().lfSmart_TailCategory[iSeverityLimit];
                break;

            case GEX_TPAT_RULETYPE_IGNOREID:
            case GEX_TPAT_RULETYPE_CUSTOMLIBID:
            default:
                return 0.0;
        }
    }
    while(1);
}

/////////////////////////////////////////////////////////////////////////////
// Detects tail cut-off (in Right or Left direction starting from Median).
// Tail cut-off: limit between valid data and outliers.
/////////////////////////////////////////////////////////////////////////////
void patlib_GetTailCutoff(const CTest *ptTestCell,
                          bool bRight,
                          double *lfLimit,
                          CPatDefinition *patDef)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Compute Tail Cut-off").toLatin1().data());
    if (!ptTestCell || !patDef)
        return;
    // Note: assumes histogram array has been done over test results, non-cumulative.
    double	lRange = ptTestCell->GetCurrentLimitItem()->lfHistogramMax-ptTestCell->GetCurrentLimitItem()->lfHistogramMin;
    // avoid zero div
    if (lRange == 0.0)
        return;

    double	lCellSize = lRange/TEST_HISTOSIZE;
    int		lMedianCell = (int)(((double)TEST_HISTOSIZE*(ptTestCell->lfSamplesQuartile2 - ptTestCell->GetCurrentLimitItem()->lfHistogramMin))/(lRange));

    // Debug check: Ensure we have a valid index
    GEX_ASSERT(lMedianCell >= 0);

    // Compute number of histogram cells for N*IQR
    double lIQRFactor;
    switch(patDef->mTailMngtRuleType)
    {
        case GEX_TPAT_TAIL_MNGT_DISABLED:
            return;
            break;
        case GEX_TPAT_TAIL_MNGT_CONSERVATIVE:
            lIQRFactor = 1.0;
            break;
        case GEX_TPAT_TAIL_MNGT_MEDIUM:
            lIQRFactor = 2.0;
            break;
        case GEX_TPAT_TAIL_MNGT_LIBERAL:
        default:
            lIQRFactor = 3.0;
            break;
    }
    int	lIqrCells = (int) (lIQRFactor*(ptTestCell->lfSamplesQuartile3-ptTestCell->lfSamplesQuartile1)
                           /lCellSize);

    if(lIqrCells <= 0) lIqrCells = 1;

    int	iEmptyCells=0;
    int	iIndex;
    // Define tail analysis direction
    if(bRight)
        iIndex=gex_min(lMedianCell+1,TEST_HISTOSIZE-1);
    else
        iIndex=gex_max(lMedianCell-1,0);

    do
    {
        // Keep track of empty histogram cell.
        if(ptTestCell->lHistogram[iIndex] == 0)
        {
            iEmptyCells++;
        }
        else
        {
            iEmptyCells = 0;
        }
        // Stop as soon as we've got too many empty cells,defines the cut-off limit!
        if(iEmptyCells > lIqrCells)
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("empty %1 / Iqr %2 / index %3")
                  .arg(iEmptyCells)
                  .arg(lIqrCells)
                  .arg(iIndex)
                  .toLatin1().data());

            // Compute cut-off limit position
            if(bRight)
                *lfLimit = ptTestCell->GetCurrentLimitItem()->lfHistogramMin + iIndex*lCellSize + (lCellSize/2);
            else
                *lfLimit = ptTestCell->GetCurrentLimitItem()->lfHistogramMin + iIndex*lCellSize - (lCellSize/2);
            GSLOG(SYSLOG_SEV_DEBUG, QString("Cut-off limit pos %1 (%2 side)").arg(*lfLimit).arg(bRight?"Right":"Left").toLatin1().data());
            return;
        }

        // If analysing Right tail, increment index...otherwise, decrement!
        if(bRight)
            iIndex++;
        else
            iIndex--;
    }
    while(iIndex >=0 && iIndex<TEST_HISTOSIZE);

    // All data to be kept
    if(bRight)
        *lfLimit = ptTestCell->GetCurrentLimitItem()->lfHistogramMax+(lCellSize/2);
    else
        *lfLimit = ptTestCell->GetCurrentLimitItem()->lfHistogramMin - (lCellSize/2);
    GSLOG(SYSLOG_SEV_DEBUG, QString("Limit pos %1 (%2 side)").arg(*lfLimit).arg(bRight?"Right":"Left").toLatin1().data());
}

/////////////////////////////////////////////////////////////////////////////
// Tells if two distributions are of similar type (eg: clamped left and lognormal-right tail)
/////////////////////////////////////////////////////////////////////////////
bool patlib_IsDistributionSimilar(int iShape1,int iShape2)
{
    // If one of the distribution not initialized yet, do not throw exception.
    if(iShape1 < 0 || iShape2 < 0)
        return true;

    bool	bSimilar=false;
    switch(iShape1)
    {
        case PATMAN_LIB_SHAPE_GAUSSIAN:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_CATEGORY:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_CATEGORY:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_CATEGORY:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_CATEGORY:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_CATEGORY:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_BIMODAL:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_BIMODAL:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_CATEGORY:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_MULTIMODAL:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_CATEGORY:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_CATEGORY:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_CATEGORY:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                case PATMAN_LIB_SHAPE_CATEGORY:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_CATEGORY:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_CATEGORY:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = false;
                    break;
            }
            break;

        case PATMAN_LIB_SHAPE_UNKNOWN:
            switch(iShape2)
            {
                case PATMAN_LIB_SHAPE_UNKNOWN:
                    bSimilar = true;
                    break;

                case PATMAN_LIB_SHAPE_GAUSSIAN:
                case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
                case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
                case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
                case PATMAN_LIB_SHAPE_BIMODAL:
                case PATMAN_LIB_SHAPE_MULTIMODAL:
                case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
                case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
                case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
                case PATMAN_LIB_SHAPE_CATEGORY:
                    bSimilar = false;
                    break;
            }
            break;
    }

    return bSimilar;
}

/////////////////////////////////////////////////////////////////////////////
// Tells if distribution is Normal
/////////////////////////////////////////////////////////////////////////////
bool patlib_IsDistributionGaussian(CTest *ptTestCell,
                                   int lCategoryValueCount /*=5*/,
                                   bool lAssumeIntegerCategory /*=true*/,
                                   int aMinConfThreshold /*=2*/)
{
    switch(patlib_GetDistributionType(ptTestCell, lCategoryValueCount, lAssumeIntegerCategory, aMinConfThreshold))
    {
        case PATMAN_LIB_SHAPE_GAUSSIAN:
        case PATMAN_LIB_SHAPE_GAUSSIAN_LEFT:
        case PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT:
            return true;

        case PATMAN_LIB_SHAPE_LOGNORMAL_LEFT:
        case PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT:
        case PATMAN_LIB_SHAPE_BIMODAL:
        case PATMAN_LIB_SHAPE_MULTIMODAL:
        case PATMAN_LIB_SHAPE_CLAMPED_LEFT:
        case PATMAN_LIB_SHAPE_CLAMPED_RIGHT:
        case PATMAN_LIB_SHAPE_DOUBLECLAMPED:
        case PATMAN_LIB_SHAPE_CATEGORY:
        case PATMAN_LIB_SHAPE_UNKNOWN:
        default:
            return false;
    }
}

int GetMatchingPATDistribution(const QString& shape)
{
    QString lShape = shape.trimmed();
    int lPATDistribution = -1;

    if (lShape == "Constant" || lShape == "Categorical")
    {
        lPATDistribution = PATMAN_LIB_SHAPE_CATEGORY;
    }
    else if (lShape == "LeftClamped")
    {
        lPATDistribution = PATMAN_LIB_SHAPE_CLAMPED_LEFT;
    }
    else if (lShape == "RightClamped")
    {
        lPATDistribution = PATMAN_LIB_SHAPE_CLAMPED_RIGHT;
    }
    else if (lShape == "DualClamped")
    {
        lPATDistribution = PATMAN_LIB_SHAPE_DOUBLECLAMPED;
    }
    else if (lShape == "Bimodal")
    {
        lPATDistribution = PATMAN_LIB_SHAPE_BIMODAL;
    }
    else if (lShape == "Normal" || lShape == "Multimodal-Normal")
    {
        lPATDistribution = PATMAN_LIB_SHAPE_GAUSSIAN;
    }
    else if (lShape == "Right-Skewed"/* || lShape == "Multimodal-Right-Skewed"*/)
    {
        lPATDistribution = PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT;
    }
    else if (lShape == "Left-Skewed"/* || lShape == "Multimodal-Left-Skewed"*/)
    {
        lPATDistribution = PATMAN_LIB_SHAPE_LOGNORMAL_LEFT;
    }
    else if (lShape.startsWith("Multimodal"))
    {
        lPATDistribution = PATMAN_LIB_SHAPE_MULTIMODAL;
    }
    else if (lShape.isEmpty()/* || lShape == "Multimodal-Other"*/)
    {
        lPATDistribution = PATMAN_LIB_SHAPE_UNKNOWN;
    }
    else
    {
        lPATDistribution = PATMAN_LIB_SHAPE_UNKNOWN;
    }

    return lPATDistribution;
}

int patlib_IdentifyDistribution(CTest *testCell,
                                GS::SE::StatsEngine * statsEngine,
                                int aMinConfThreshold,
                                GS::Gex::PATPartFilter *partFilter)
{
    if (testCell->GetDistribution() > PATMAN_LIB_SHAPE_UNSET)
        return testCell->GetDistribution();

    GSLOG(SYSLOG_SEV_DEBUG, QString("identify distrib on %1").arg(testCell->lTestNumber).toLatin1().data());
    if (!statsEngine)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Stats engine NULL").toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }

    if (!testCell)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Test NULL").toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }

    GS::SE::ShapeIdentifier * lShapeIdentifier = static_cast<GS::SE::ShapeIdentifier*>
            (statsEngine->GetAlgorithm(GS::SE::StatsAlgo::SHAPE_IDENTIFIER));
    if (!lShapeIdentifier)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Shape identifier Algorithm not loaded").toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }

    // GCORE-5239
    // Use QScopedPointer in order to automatically manage deletion of this pointer at the end of this scope
    QScopedPointer<GS::SE::RData> lData(new GS::SE::RData());

    if (lData.isNull())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Unable to allocate Data").toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }

    GS::SE::RVector* lInputVector = lData->AllocateVector(GS::SE::StatsData::SHAPEIDENTIFIER_IN,
                                                               testCell->ldSamplesValidExecs,
                                                               GS::SE::RVector::V_DOUBLE);
    if (!lInputVector)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Unable to allocate vector").toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }

    int lResultIdx = 0;
    // Fill the R Matrix with offseted results
    for(int lRunIdx = 0; lRunIdx < testCell->m_testResult.count(); ++lRunIdx)
    {
        // ignore invalid results
        if (!testCell->m_testResult.isValidResultAt(lRunIdx) ||
                (partFilter && !partFilter->IsFiltered(lRunIdx)))
            continue;

        // If not multi results
        if (!testCell->m_testResult.isMultiResultAt(lRunIdx))
        {
            lInputVector->FillDouble(lResultIdx,
                                     testCell->m_testResult.resultAt(lRunIdx));
            ++lResultIdx;
        }
        // If multi results
        else
        {
            for (int lCount = 0; lCount < testCell->m_testResult.at(lRunIdx)->count(); ++lCount)
            {
                // ignore invalid results
                if(!testCell->m_testResult.at(lRunIdx)->isValidResultAt(lCount))
                    continue;
                lInputVector->FillDouble(lResultIdx,
                                         testCell->m_testResult.at(lRunIdx)->multiResultAt(lCount));
                ++lResultIdx;
            }
        }
    }

    QMap<GS::SE::StatsAlgo::Parameter,QVariant> lParam;
    lParam.insert(GS::SE::StatsAlgo::DISTINCT_VALUES, QVariant((qlonglong)testCell->lDifferentValues));

    bool lSuccess = lShapeIdentifier->Execute(lParam, lData.data());

    if (!lSuccess)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: Error while executing shape identifier: %1")
              .arg(lShapeIdentifier->GetLastError()).toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }

    lSuccess = false;
    GS::SE::RVector lShape = lShapeIdentifier->GetShapeName(lSuccess);
    if (!lSuccess)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: Unable to get shape").toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }
    GS::SE::RVector lConfidence = lShapeIdentifier->GetConfidenceLevel(lSuccess);
    if (!lSuccess)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: Unable to get confidence level").toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }

    std::string lTmp = lConfidence.GetStringItem(0, lSuccess);
    if (!lSuccess)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: Invalid confidence level (%1)").arg(QString::fromStdString(lTmp)).toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }

    int lConfidenceValue = QString::fromStdString(lTmp).toInt(&lSuccess);
    if(!lSuccess)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error: In confidence level conversion").toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }

    int lDistribution = PATMAN_LIB_SHAPE_UNKNOWN;
    // if confidence is not strong enough
    if (lConfidenceValue < aMinConfThreshold)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Shape identified as %1 for test %2 but confidence level too low, changed to unknown.")
              .arg(QString::fromStdString(lShape.GetStringItem(0, lSuccess)))
              .arg(testCell->lTestNumber)
              .toLatin1().data());
        lDistribution = PATMAN_LIB_SHAPE_UNKNOWN;
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Shape identified as %1 for test %2 (Conf %3).")
              .arg(QString::fromStdString(lShape.GetStringItem(0, lSuccess)))
              .arg(testCell->lTestNumber)
              .arg(lConfidenceValue)
              .toLatin1().data());
        // confidence is good, check distrib
        lDistribution = GetMatchingPATDistribution(QString::fromStdString(lShape.GetStringItem(0, lSuccess)));
        // if Gaussian check if right / left / standard
        if (lDistribution == PATMAN_LIB_SHAPE_GAUSSIAN)
        {
            if (qAbs(testCell->lfSamplesSkewWithoutNoise) < 0.3)
                lDistribution = PATMAN_LIB_SHAPE_GAUSSIAN;
            else if (testCell->lfSamplesSkewWithoutNoise > 0)
                lDistribution = PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT;
            else if (testCell->lfSamplesSkewWithoutNoise < 0)
                lDistribution = PATMAN_LIB_SHAPE_GAUSSIAN_LEFT;
        }
    }

    testCell->SetDistribution(lDistribution);

    // in case of bimodality, store value that splits modes
    if (lDistribution == PATMAN_LIB_SHAPE_BIMODAL)
    {
        GS::SE::RVector lBimodalSplit = lShapeIdentifier->GetBimodalSplitValue(lSuccess);
        if (!lSuccess)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error: Unable to get bimodal split value").toLatin1().data());
            return PATMAN_LIB_SHAPE_ERROR;
        }

        std::string lTmp = lBimodalSplit.GetStringItem(0, lSuccess);

        if (!lSuccess)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error: Invalid bimodal split value").toLatin1().data());
            return PATMAN_LIB_SHAPE_ERROR;
        }

        double lBimodalSplitValue = QString::fromStdString(lTmp).toDouble(&lSuccess);

        if (!lSuccess)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error: In bimodal split value conversion (%1)").arg(QString::fromStdString(lTmp)).toLatin1().data());
            return PATMAN_LIB_SHAPE_ERROR;
        }

        testCell->SetBimodalSplitValue(lBimodalSplitValue);
    }
    //printf("Test %d\n", testCell->lTestNumber);
    //fflush(NULL);



    return lDistribution;
}

int patlib_IdentifyDistributionLegacy(CTest *testCell)
{
    if (!testCell)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Test NULL").toLatin1().data());
        return -1;
    }

    double lQ1,lQ2,lQ3,lQ4;
    patlib_ComputeQuartilePercentage(testCell,&lQ1,&lQ2,&lQ3,&lQ4);

    // Check for Multi-modal
    int lDistribution = patlib_isDistributionMultiModal(testCell,lQ1,lQ2,lQ3,lQ4);
    if(lDistribution >= 0)
        return lDistribution;

    // Check for LogNormal
    lDistribution = patlib_isDistributionLogNormal(lQ1,lQ2,lQ3,lQ4);
    if(lDistribution >= 0)
        return lDistribution;

    // Check if dual-Clamped
    lDistribution = patlib_isDistributionDualClamped(testCell,lQ1,lQ2,lQ3,lQ4);
    if(lDistribution >= 0)
        return lDistribution;

    // Not dual clamped...then maybe single-Clamped
    lDistribution = patlib_isDistributionClamped(testCell);
    if(lDistribution >= 0)
        return lDistribution;

    // Assume Gaussian if all other shape identification failed!
    lDistribution = patlib_isDistributionNormal(testCell);
    if(lDistribution >= 0)
        return lDistribution;


    return PATMAN_LIB_SHAPE_CATEGORY;
}

/////////////////////////////////////////////////////////////////////////////
// Tells type of distribution based on percentage of samples in each quartile
/////////////////////////////////////////////////////////////////////////////
int patlib_GetDistributionType(CTest *testCell,
                               int categoryValueCount /*=5*/,
                               bool assumeIntegerCategory /*=true*/,
                               int aMinConfThreshold /*= 2*/,
                               GS::Gex::PATPartFilter *partFilter /*=NULL*/)
{
    if (!testCell)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Stats engine NULL").toLatin1().data());
        return PATMAN_LIB_SHAPE_ERROR;
    }

    if(testCell->lfSamplesQuartile1 == -C_INFINITE &&
        testCell->lfSamplesQuartile2 == -C_INFINITE &&
        testCell->lfSamplesQuartile3 == -C_INFINITE &&
        testCell->lfSamplesStartAfterNoise == -C_INFINITE &&
        testCell->lfSamplesEndBeforeNoise == -C_INFINITE)
        return PATMAN_LIB_SHAPE_UNKNOWN;

    // Check if 'categories'
    if(testCell->lDifferentValues >= 0 && testCell->lDifferentValues <= categoryValueCount)
        return PATMAN_LIB_SHAPE_CATEGORY;

    if(testCell->bIntegerValues && assumeIntegerCategory)
        return PATMAN_LIB_SHAPE_CATEGORY;

    if (testCell->GetDistribution() > PATMAN_LIB_SHAPE_UNSET)
    {
        return testCell->GetDistribution();
    }

    int lDistributionType = PATMAN_LIB_SHAPE_UNKNOWN;
    if (QString(qgetenv("GS_SHAPEDETECTION_LEGACY")) != "1")
    {
        QString lAppDir = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        QString lError;
        GS::SE::StatsEngine * lStatsEngine = GS::SE::StatsEngine::GetInstance(lAppDir, lError);
        if (lStatsEngine)
        {
            lDistributionType = patlib_IdentifyDistribution(testCell, lStatsEngine, aMinConfThreshold, partFilter);
            GS::SE::StatsEngine::ReleaseInstance();
        }
        else
        {
            GSLOG(SYSLOG_SEV_CRITICAL, QString("Unable to instantiate StatsEngine: %1. Application will now exit.")
                  .arg(lError).toLatin1().data());
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        lDistributionType = patlib_IdentifyDistributionLegacy(testCell);
    }

    return lDistributionType;
}

#endif  // #ifndef SOURCEFILE_PAT_SHAPE_ANALYSIS_CPP
