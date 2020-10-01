/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Notes :                                                                              */
/*                                                                                      */
/****************************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////
// Implementation of classes to compute Yield-123 stats.
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
/////////////////////////////////////////////////////////////////////////////////////////
// Galaxy libraries includes

// Standard includes
#include <stdlib.h>
#include <math.h>
#include <QFile>

// Local includes
#include "gate_data_stats.h"
#include "gate_event_constants.h"


/////////////////////////////////////////////////////////////////////////////////////////
// EXTERN DATA
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// STATS OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
// ERROR MAP
GBEGIN_ERROR_MAP(C_Gate_DataStats)
	GMAP_ERROR(eNotEnoughSamples,"There are too few runs (%d) to compute reliable statistics")
	GMAP_ERROR(eNoParameterSet,"Attempt to compute ststistics on a parameter, but no parameters are defined")
	GMAP_ERROR(eMalloc,"Memory allocation failure")
GEND_ERROR_MAP(C_Gate_DataStats)

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataStats::C_Gate_DataStats(C_Gate_DataModel_Progress *pProgress)
{
	m_pProgress = pProgress;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
C_Gate_DataStats::~C_Gate_DataStats()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Computes basic statistics on given dataset (Mean, Sigma, Cp, Cpk...)
//				 This function goes through the complete dataset, and for each batch/site,
//				 it computes first level statistics.
//
// Argument(s) :
//
//	const C_Gate_DataModel *pclData
//		Ptr on data set
//
// Return type : TRUE if statistics successfully computed, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataStats::ComputeFirstLevelStats(C_Gate_DataModel *pclData, bool bComputeWaferStats, bool bOutliersRemoval)
{
	C_Gate_DataModel_TestingStage		*pTestingStage;
	C_Gate_DataModel_Lot				*pLot;
	C_Gate_DataModel_Sublot				*pSublot;
	C_Gate_DataModel_Batch				*pBatch;
	C_Gate_DataModel_Site				*pSite;


	pTestingStage = pclData->GetFirstTestingStage();
	while(pTestingStage)
	{
		// Lot loop
		pLot = pTestingStage->GetFirstLot();
		while(pLot)
		{
			// Sublot loop
			pSublot = pLot->GetFirstSublot();
			while(pSublot)
			{
				// Batch loop
				pBatch = pSublot->GetFirstBatch();
				while(pBatch)
				{
					// Initialize WaferMap information only if we need this info
					if(bComputeWaferStats)
						pBatch->InitWaferMap();

					unsigned int uiNbElements;
					// for all parameters on all sites
					uiNbElements = pBatch->NbSites()*pBatch->m_uiNbParameters;
					// and all parameters for wafermap
					if((pBatch->m_pWaferMap)
					&& (pBatch->m_pWaferMap->m_pWaferZones))
					{
						uiNbElements +=
								(pBatch->NbSites()+1)
								*pBatch->m_pWaferMap->m_nNbZones
								*pBatch->m_uiNbRuns;
						uiNbElements += (pBatch->NbSites()+1)*pBatch->m_uiNbRuns*2;
					}

					if(m_pProgress)
						m_pProgress->Init("Statistics in progress ...", uiNbElements);

					// Site loop
					pSite = pBatch->GetFirstSite();
					while(pSite)
					{
						ComputeFirstLevelStats(pSite, bOutliersRemoval);
						pSite = pBatch->GetNextSite();
					}

					// WaferMap stat
					if((pBatch->m_pWaferMap)
					&& (pBatch->m_pWaferMap->m_pWaferZones))
						ComputeWaferMapStats(pBatch);
					pBatch = pSublot->GetNextBatch();
				}
				pSublot = pLot->GetNextSublot();
			}
			pLot = pTestingStage->GetNextLot();
		}
		pTestingStage = pclData->GetNextTestingStage();
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Computes wafermap statistics for a given batch
// Argument(s) :
//
//	C_Gate_DataModel_Batch *pBatch
//		Ptr on batch
//
// Return type : TRUE if statistics successfully computed, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataStats::ComputeWaferMapStats(C_Gate_DataModel_Batch *pBatch)
{
	if(pBatch->m_pWaferMap == NULL)
		return true;

	int nZoneIndex, nSiteIndex;
	for(nZoneIndex=0 ; nZoneIndex<=pBatch->m_pWaferMap->m_nNbZones ; nZoneIndex++)
	{
		for(nSiteIndex=-1 ; nSiteIndex<pBatch->m_pWaferMap->m_nNbSites ; nSiteIndex++)
		{
			// Update progress dialog
			if(m_pProgress != NULL)
				m_pProgress->Increment();

			pBatch->m_pWaferMap->UpdateWaferZone(pBatch, nZoneIndex, nSiteIndex);
		}
	}

	pBatch->m_pWaferMap->UpdateWaferPoints(pBatch);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Computes basic statistics for a given site (Mean, Sigma, Cp, Cpk...)
//				 This function does following computations for each parameter:
//				 1) compute basic statistics using all data stats array
//				 2) mark outliers in result arrays
//
//				 IF SOME OUTLIERS FOUND
//
//				 3) update execs, fails, good, SumX, SumX2 for non-outlier stats and
//					outlier stats
//				 4) compute basic statistics on non-outlier data and on outlier data
//
// Argument(s) :
//
//	C_Gate_DataModel_Site *pSite
//		Ptr on site
//
// Return type : TRUE if statistics successfully computed, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////
bool C_Gate_DataStats::ComputeFirstLevelStats(C_Gate_DataModel_Site *pSite, bool bOutliersRemoval)
{
	C_Gate_DataModel_ParameterSet *pParameterSet = pSite->GetParent()->m_clBatchID.m_pParameterSet;

	// Make sure we have at least n runs
	if(pSite->m_uiNbRuns < YIELD123_STATS_MINIMUMSAMPLES)
	{
		GSET_ERROR1(C_Gate_DataStats, eNotEnoughSamples, NULL, pSite->m_uiNbRuns);
		return FALSE;
	}

	// Make sure parameter set is valid
	if((pParameterSet == NULL) || (pParameterSet->m_uiNbParameters == 0))
	{
		GSET_ERROR0(C_Gate_DataStats, eNoParameterSet, NULL);
		return FALSE;
	}

	unsigned int	uiParameterIndex;
	unsigned int	uiNbHistoClasses = 10;
	float			fValue;

#if 0
	QTime	clTimer;
	clTimer.start();
#endif

	// Parameter stat loop
	for(uiParameterIndex = 0; uiParameterIndex < pSite->m_uiNbParameters; uiParameterIndex++)
	{
		// Update progress dialog
		if(m_pProgress != NULL)
			m_pProgress->Increment();

		if((pParameterSet->m_pclParameters[uiParameterIndex].m_bType == 'P') || (pParameterSet->m_pclParameters[uiParameterIndex].m_bType == 'M') || (pParameterSet->m_pclParameters[uiParameterIndex].m_bType == '-'))
		{
#if 1
			// COMPUTE QUARTILES

			C_Gate_DataModel_TestResultVector TestResultVector;

			///////////////////////////////////////////////////////////////////////////////////////
			// STATS ON ALL DATA
			///////////////////////////////////////////////////////////////////////////////////////
			// Construct a vector of test results for sorting: use all data
			TestResultVector.FillFromParameter(pSite, uiParameterIndex, eResult_Any);
			TestResultVector.Sort(C_Gate_DataModel_TestResultVector::eSortOnResult, TRUE);

			// Compute basic stats on all data
			ComputeBasicStats(pSite->m_pParameterStats_AllData[uiParameterIndex], (pParameterSet->m_pclParameters) + uiParameterIndex);

			// Compute quartiles
			ComputeQuartiles(pSite->m_pParameterStats_AllData[uiParameterIndex], TestResultVector);

			// Compute the number of bars you need to have the best representation of the data
			// Compute the 2 histobars, one with 100 bars and one with a limited nb bars
			uiNbHistoClasses = 10;
			C_Gate_DataModel_ParameterDef *pParameterDef = (pParameterSet->m_pclParameters + uiParameterIndex);

			if(pSite->m_pParameterStats_AllData[uiParameterIndex].m_fIQR > 0.0)
			{
				// smooth variation
				// minimum of 5 bars to draw the interquartiles
				uiNbHistoClasses = (unsigned int)(pSite->m_pParameterStats_AllData[uiParameterIndex].m_fRange * 5.0 / pSite->m_pParameterStats_AllData[uiParameterIndex].m_fIQR);

				// if HL and LL available
				// consider the limit space and adjust the NbHistoClasses
				if((pParameterDef->m_fLL > -GEX_PLUGIN_INVALID_VALUE_FLOAT)
				&& (pParameterDef->m_fHL <  GEX_PLUGIN_INVALID_VALUE_FLOAT))
				{
					// if data take less than 10% of the limit space, use only 10 HistoBar
					fValue = (pSite->m_pParameterStats_AllData[uiParameterIndex].m_fRange/(pParameterDef->m_fHL-pParameterDef->m_fLL))*100;
					if(fValue < uiNbHistoClasses)
						uiNbHistoClasses = (unsigned int)fValue;
				}
				if(uiNbHistoClasses < 10)
					uiNbHistoClasses = 10;
				if(uiNbHistoClasses > YIELD123_STATS_MAXHISTOCLASSES)
					uiNbHistoClasses = YIELD123_STATS_MAXHISTOCLASSES;
			}

			// For Binning parameter (only integer), have to used the range
			if(pParameterSet->m_pclParameters[uiParameterIndex].m_bType == '-')
			{
				uiNbHistoClasses = (unsigned int) pSite->m_pParameterStats_AllData[uiParameterIndex].m_fRange;
				if(uiNbHistoClasses < 10)
					uiNbHistoClasses = 10;
			}

			// Compute histogram classes on all data
			ComputeHistoClasses(pSite->m_pParameterStats_AllData[uiParameterIndex], uiNbHistoClasses, pSite->m_pTestResults, uiParameterIndex, pSite->m_uiNbRuns, eResult_Any);

			// Set some stats flags (artificial limits...)
			SetStatFlags(pSite->m_pParameterStats_AllData[uiParameterIndex], (pParameterSet->m_pclParameters) + uiParameterIndex);

			// Compute Shape Distribution
			ComputeShapeDistribution(pSite->m_pParameterStats_AllData[uiParameterIndex], TestResultVector);

			// Compute X-Bar R stats
			ComputeXBarRStats(pSite->m_pParameterStats_AllData[uiParameterIndex], pSite->m_pTestResults, pSite->m_pPartResults, uiParameterIndex, pSite->m_uiNbRuns);

			// ONLY IF HAVE TO COMPUTE OUTLIERS REMOVAL STATISTICS
			if(bOutliersRemoval)
			{
				// Mark outliers on all data: use standard sigma outlier removal
				if(pParameterSet->m_pclParameters[uiParameterIndex].m_bType != '-')
				{
	#if 0
					if(pSite->m_pParameterStats_AllData[uiParameterIndex].m_fRobustSigma != 0.0F)
						MarkOutliers_RobustSigma(pSite, TestResultVector, uiParameterIndex);
					else if(pSite->m_pParameterStats_AllData[uiParameterIndex].m_fSigma != 0.0F)
						MarkOutliers_Standard(pSite, uiParameterIndex);
	#else
					if(pSite->m_pParameterStats_AllData[uiParameterIndex].m_fSigma != 0.0F)
						MarkOutliers_Standard(pSite, uiParameterIndex);
	#endif
				}
			}

			///////////////////////////////////////////////////////////////////////////////////////
			// STATS ON NON-OUTLIER DATA
			///////////////////////////////////////////////////////////////////////////////////////
			// Init Stat array on non-outlier data
			ComputeStatCumuls(pSite->m_pParameterStats_Distribution[uiParameterIndex], pSite->m_pTestResults, pSite->m_pPartResults, uiParameterIndex, pSite->m_uiNbRuns, eResult_NotOutlier);

			// Copy some stat flags from the stat structure on all data to the stat structure on non-outlier data
			CopyStatFlags((pSite->m_pParameterStats_AllData) + uiParameterIndex, (pSite->m_pParameterStats_Distribution) + uiParameterIndex);

			// Compute basic stats on non-outlier data
			ComputeBasicStats(pSite->m_pParameterStats_Distribution[uiParameterIndex], (pParameterSet->m_pclParameters) + uiParameterIndex);

			// Compute quartiles on non-outlier data: no need to create a new vector with non-outlier data and sort it,
			// we just tell how many outliers to remove on low and high values
			ComputeQuartiles(pSite->m_pParameterStats_Distribution[uiParameterIndex], TestResultVector, pSite->m_pParameterStats_AllData[uiParameterIndex].m_uiOutliers_L, pSite->m_pParameterStats_AllData[uiParameterIndex].m_uiOutliers_H);

			// Compute the number of bars you need to have the best representation of the data
			// Compute the 2 histobars, one with 100 bars and one with a limited nb bars
			if(pSite->m_pParameterStats_Distribution[uiParameterIndex].m_fIQR > 0.0)
			{
				// smooth variation
				// minimum of 5 bars to draw the interquartiles
				uiNbHistoClasses = (unsigned int)(pSite->m_pParameterStats_Distribution[uiParameterIndex].m_fRange * 5.0 / pSite->m_pParameterStats_Distribution[uiParameterIndex].m_fIQR);

				// if HL and LL available
				// consider the limit space and adjust the NbHistoClasses
				if((pParameterDef->m_fLL > -GEX_PLUGIN_INVALID_VALUE_FLOAT)
				&& (pParameterDef->m_fHL <  GEX_PLUGIN_INVALID_VALUE_FLOAT))
				{
					// if data take less than 10% of the limit space, use only 10 HistoBar
					fValue = (pSite->m_pParameterStats_Distribution[uiParameterIndex].m_fRange/(pParameterDef->m_fHL-pParameterDef->m_fLL))*100;
					if(fValue < uiNbHistoClasses)
						uiNbHistoClasses = (unsigned int)fValue;
				}
				if(uiNbHistoClasses < 10)
					uiNbHistoClasses = 10;
				if(uiNbHistoClasses > YIELD123_STATS_MAXHISTOCLASSES)
					uiNbHistoClasses = YIELD123_STATS_MAXHISTOCLASSES;
			}

			// For Binning parameter (only integer), have to used the range
			if(pParameterSet->m_pclParameters[uiParameterIndex].m_bType == '-')
			{
				uiNbHistoClasses = (unsigned int) pSite->m_pParameterStats_Distribution[uiParameterIndex].m_fRange;
				if(uiNbHistoClasses < 10)
					uiNbHistoClasses = 10;
			}

			// Compute histogram classes on non-outlier data
			ComputeHistoClasses(pSite->m_pParameterStats_Distribution[uiParameterIndex], uiNbHistoClasses, pSite->m_pTestResults, uiParameterIndex, pSite->m_uiNbRuns, eResult_NotOutlier);

			// Compute Shape Distribution on non-outlier data
			ComputeShapeDistribution(pSite->m_pParameterStats_Distribution[uiParameterIndex], TestResultVector, pSite->m_pParameterStats_AllData[uiParameterIndex].m_uiOutliers_L, pSite->m_pParameterStats_AllData[uiParameterIndex].m_uiOutliers_H);

			// Compute X-Bar R stats
			ComputeXBarRStats(pSite->m_pParameterStats_Distribution[uiParameterIndex], pSite->m_pTestResults, pSite->m_pPartResults, uiParameterIndex, pSite->m_uiNbRuns);

			///////////////////////////////////////////////////////////////////////////////////////
			// STATS ON OUTLIER DATA
			///////////////////////////////////////////////////////////////////////////////////////
			// Init Stat array on outlier data
			ComputeStatCumuls(pSite->m_pParameterStats_Outliers[uiParameterIndex], pSite->m_pTestResults, pSite->m_pPartResults, uiParameterIndex, pSite->m_uiNbRuns, eResult_Outlier);

			// Copy some stat flags from the stat structure on all data to the stat structure on outlier data
			CopyStatFlags((pSite->m_pParameterStats_AllData) + uiParameterIndex, (pSite->m_pParameterStats_Outliers) + uiParameterIndex);

			// Compute basic stats on outlier data
			ComputeBasicStats(pSite->m_pParameterStats_Outliers[uiParameterIndex], (pParameterSet->m_pclParameters) + uiParameterIndex);

			// For Binning parameter (only integer), have to used the range
			if(pParameterSet->m_pclParameters[uiParameterIndex].m_bType == '-')
			{
				uiNbHistoClasses = (unsigned int) pSite->m_pParameterStats_Outliers[uiParameterIndex].m_fRange;
				if(uiNbHistoClasses < 10)
					uiNbHistoClasses = 10;
			}

			// Compute histogram classes on outlier data
			ComputeHistoClasses(pSite->m_pParameterStats_Outliers[uiParameterIndex], uiNbHistoClasses, pSite->m_pTestResults, uiParameterIndex, pSite->m_uiNbRuns, eResult_Outlier);
#endif
#if 0
			// NO QUARTILES COMPUTED

			///////////////////////////////////////////////////////////////////////////////////////
			// STATS ON ALL DATA
			///////////////////////////////////////////////////////////////////////////////////////
			// Compute basic stats on all data
			ComputeBasicStats(pSite->m_pParameterStats_AllData[uiParameterIndex], (pParameterSet->m_pclParameters) + uiParameterIndex);

			// Set some stats flags (artificial limits...)
			SetStatFlags(pSite->m_pParameterStats_AllData[uiParameterIndex], (pParameterSet->m_pclParameters) + uiParameterIndex);

			// Mark outliers on all data using standard sigma
			if(pParameterSet->m_pclParameters[uiParameterIndex].m_bType != '-')
			{
				MarkOutliers_Standard(pSite, uiParameterIndex);
			}

			///////////////////////////////////////////////////////////////////////////////////////
			// STATS ON NON-OUTLIER DATA
			///////////////////////////////////////////////////////////////////////////////////////
			// Init Stat array on non-outlier data
			ComputeStatCumuls(pSite->m_pParameterStats_Distribution[uiParameterIndex], pSite->m_pTestResults, pSite->m_pPartResults, uiParameterIndex, pSite->m_uiNbRuns, eResult_NotOutlier);

			// Copy some stat flags from the stat structure on all data to the stat structure on non-outlier data
			CopyStatFlags((pSite->m_pParameterStats_AllData) + uiParameterIndex, (pSite->m_pParameterStats_Distribution) + uiParameterIndex);

			// Compute basic stats on non-outlier data
			ComputeBasicStats(pSite->m_pParameterStats_Distribution[uiParameterIndex], (pParameterSet->m_pclParameters) + uiParameterIndex);

			///////////////////////////////////////////////////////////////////////////////////////
			// STATS ON OUTLIER DATA
			///////////////////////////////////////////////////////////////////////////////////////
			// Init Stat array on non-outlier data
			ComputeStatCumuls(pSite->m_pParameterStats_Outliers[uiParameterIndex], pSite->m_pTestResults, pSite->m_pPartResults, uiParameterIndex, pSite->m_uiNbRuns, eResult_Outlier);

			// Copy some stat flags from the stat structure on all data to the stat structure on outlier data
			CopyStatFlags((pSite->m_pParameterStats_AllData) + uiParameterIndex, (pSite->m_pParameterStats_Outliers) + uiParameterIndex);

			// Compute basic stats on outlier data
			ComputeBasicStats(pSite->m_pParameterStats_Outliers[uiParameterIndex], (pParameterSet->m_pclParameters) + uiParameterIndex);
#endif
		}
	}

    // Message::information(NULL, "Timer",
    //                      QString::number(clTimer.elapsed()) + " msec");

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Computes basic statistics on one parameter (Mean, Sigma, Cp, Cpk...)
//
//				 Mean	= SumX/Execs
//				 Sigma	= sqrt((Execs*SumX2 - SumX*SumX)/(Execs*(Execs-1)))
//				 CpkL	= (Mean-LL)/3*Sigma
//				 CpkH	= (HL-Mean)/3*Sigma
//				 Cpk	= min(|CpkL|, |CpkH|)
//				 Cp		= (HL-LL)/6*Sigma
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		Reference to object holding statistical values for this parameter
//
//	const C_Gate_DataModel_ParameterDef *pParameterDef
//		Ptr to parameter definition object
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::ComputeBasicStats(C_Gate_DataModel_ParameterStat & clParameterStat, const C_Gate_DataModel_ParameterDef *pParameterDef)
{
	// Make sure parameter has at leat 1 results
	if(clParameterStat.m_uiExecs < 1)
		return;

	double	lfExecs = clParameterStat.m_uiExecs;
	double	lfMean, lfSigma;
	double	lfNum, lfDenom;

	// Compute Mean
	lfMean = clParameterStat.m_lfSumX/lfExecs;
	clParameterStat.m_fMean = lfMean;

	if(clParameterStat.m_uiExecs > 0)
		clParameterStat.m_lDifferentValues = 1;

	// Make sure parameter has at least 2 results
	if(clParameterStat.m_uiExecs < 2)
		return;


	// Compute Sigma
	if(clParameterStat.m_fRange == 0.0)
		lfSigma = 0.0;
	else
	{
		lfNum = lfExecs*clParameterStat.m_lfSumX2 - pow(clParameterStat.m_lfSumX, 2.0);
		lfDenom = lfExecs*(lfExecs-1.0);
		lfSigma = sqrt(fabs(lfNum/lfDenom));
	}
	clParameterStat.m_fSigma = lfSigma;

	if(pParameterDef != NULL)
	{
		double	lfLL = pParameterDef->m_fLL;
		double	lfHL = pParameterDef->m_fHL;
		// Compute CpkL, CpkH, Cpk, Cp
		if((pParameterDef->m_uiLimitFlags & YIELD123_PARAMETERDEF_HASLL)
		&& (pParameterDef->m_uiLimitFlags & YIELD123_PARAMETERDEF_HASHL)
		&& (lfLL < lfHL))
		{
			// Parameter has 2 limits
			// CpkL
			lfNum = lfMean - lfLL;
			lfDenom = lfSigma*3.0;
			if(lfDenom == 0.0)
				clParameterStat.m_fCpkL = lfNum < 0.0 ? -GEX_PLUGIN_INFINITE_VALUE_FLOAT : GEX_PLUGIN_INFINITE_VALUE_FLOAT;
			else
				clParameterStat.m_fCpkL = lfNum/lfDenom;
			// CkpH
			lfNum = lfHL - lfMean;
			lfDenom = lfSigma*3.0;
			if(lfDenom == 0.0)
				clParameterStat.m_fCpkH = lfNum < 0.0 ? -GEX_PLUGIN_INFINITE_VALUE_FLOAT : GEX_PLUGIN_INFINITE_VALUE_FLOAT;
			else
				clParameterStat.m_fCpkH = lfNum/lfDenom;
			// Cpk
			clParameterStat.m_fCpk = gex_min(clParameterStat.m_fCpkL, clParameterStat.m_fCpkH);
			// Cp
			lfNum = lfHL - lfLL;
			lfDenom = lfSigma*6.0;
			if(lfDenom == 0.0)
				clParameterStat.m_fCp = GEX_PLUGIN_INFINITE_VALUE_FLOAT;
			else
				clParameterStat.m_fCp = lfNum/lfDenom;
		}
		else
		if((pParameterDef->m_uiLimitFlags & YIELD123_PARAMETERDEF_HASLL)
		&& !(pParameterDef->m_uiLimitFlags & YIELD123_PARAMETERDEF_HASHL))
		{
			// Parameter has only a LL
			// CpkL
			lfNum = lfMean - lfLL;
			lfDenom = lfSigma*3.0;
			if(lfDenom == 0.0)
				clParameterStat.m_fCpkL = lfNum < 0.0 ? -GEX_PLUGIN_INFINITE_VALUE_FLOAT : GEX_PLUGIN_INFINITE_VALUE_FLOAT;
			else
				clParameterStat.m_fCpkL = lfNum/lfDenom;
			// Cpk
			clParameterStat.m_fCpk = clParameterStat.m_fCpkL;
		}
		else
		if(!(pParameterDef->m_uiLimitFlags & YIELD123_PARAMETERDEF_HASLL)
		&& (pParameterDef->m_uiLimitFlags & YIELD123_PARAMETERDEF_HASHL))
		{
			// Parameter has only a HL
			// CkpH
			lfNum = lfHL - lfMean;
			lfDenom = lfSigma*3.0;
			if(lfDenom == 0.0)
				clParameterStat.m_fCpkH = lfNum < 0.0 ? -GEX_PLUGIN_INFINITE_VALUE_FLOAT : GEX_PLUGIN_INFINITE_VALUE_FLOAT;
			else
				clParameterStat.m_fCpkH = lfNum/lfDenom;
			// Cpk
			clParameterStat.m_fCpk = clParameterStat.m_fCpkH;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description :
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		Reference to object holding statistical values for this parameter
//
//	const C_Gate_DataModel_ParameterDef *pParameterDef
//		Ptr to parameter definition object
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void
C_Gate_DataStats::
SetStatFlags(C_Gate_DataModel_ParameterStat& /*clParameterStat*/,
			 const C_Gate_DataModel_ParameterDef* /*pParameterDef*/)
{
	// Artificial LL if:
	// o (LL != HL) OR no HL
	// o LL = 0
	// o all results >= 0 and LL = 0
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Computes quartiles (Q1, Q2, Q3) on one parameter. This function also
//				 computes the IQR (Inter Quartile Range), IQS (Interquartile Sigma),
//				 robust mean and robust sigma
//
//				 Q1           = the value of the sorted series of observations having the position x = round(0.25*(N+1))
//				 Q2           = if N is even, Q2 is the mean of the two values at the positions N/2 and N/2+1; if N is odd, Q2 is the value at the position (N+1)/2
//				 Q3           = the value of the sorted series having the position x = round(0.75*(N+1))
//				 IQR          = Q3-Q1
//				 IQS		  = Sigma of values in [Q1-Q3] space
//				 Robust Mean  = Q2
//				 Robust Sigma = IQR/1.35
//
//				 Note: the array starting at position 0, 1 must be retrieved from any above position
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::ComputeQuartiles(C_Gate_DataModel_ParameterStat & clParameterStat, C_Gate_DataModel_TestResultVector & TestResultVector, unsigned int uiRemoveLow/* = 0*/, unsigned int uiRemoveHigh/* = 0*/)
{
	unsigned int	uiExecs = TestResultVector.CountValues() - uiRemoveHigh - uiRemoveLow;

	// Make sure parameter has at leat 4 results
	if(uiExecs < 4)
		return;

	unsigned int	uiRunIndex;
	unsigned int	uiPositionQ1, uiPositionQ2, uiPositionQ3;
	double			lfExecs = uiExecs;

	// Set indexes
	uiPositionQ1 = (uiExecs >= 3 ? (int)(0.25*(lfExecs+1.0)) - 1 : 0) + uiRemoveLow;
	uiPositionQ2 = (uiExecs/2) + uiRemoveLow;
	uiPositionQ3 = (uiExecs >= 1 ? (int)(0.75*(lfExecs+1.0)) - 1 : 0) + uiRemoveLow;

	// Q1 (25th percentile)
	clParameterStat.m_fQ1 = TestResultVector[uiPositionQ1]->m_pTestResult->m_fResult;

	// Q2 (50th percentile = median)
	if((uiExecs/2)*2 == uiExecs)
		// Even nb of samples
		clParameterStat.m_fQ2 = (TestResultVector[uiPositionQ2-1]->m_pTestResult->m_fResult+TestResultVector[uiPositionQ2]->m_pTestResult->m_fResult)/2.0;
	else
		// Odd nb of samples
		clParameterStat.m_fQ2 = TestResultVector[uiPositionQ2]->m_pTestResult->m_fResult;

	// Q3 (75th percentile)
	clParameterStat.m_fQ3 = TestResultVector[uiPositionQ3]->m_pTestResult->m_fResult;

	// Compute IQR (Interquartile range), robust Mean and robust Sigma
	clParameterStat.m_fIQR = clParameterStat.m_fQ3-clParameterStat.m_fQ1;
	clParameterStat.m_fRobustMean = clParameterStat.m_fQ2;
	clParameterStat.m_fRobustSigma = clParameterStat.m_fIQR/1.35;

	// Compute IQS (Interquartile sigma)
	if(clParameterStat.m_fIQR == 0.0)
		clParameterStat.m_fIQS = 0.0;
	else
	{
		double	lfIQSSumX=0.0, lfIQSSumX2=0.0;
		double	lfIQSExecs = uiPositionQ3 - uiPositionQ1 + 1;
		double	lfResult;
		double	lfNum, lfDenom;
		for(uiRunIndex = uiPositionQ1; uiRunIndex <= uiPositionQ3; uiRunIndex++)
		{
			lfResult = (double)(TestResultVector[uiRunIndex]->m_pTestResult->m_fResult);
			lfIQSSumX	+= lfResult;
			lfIQSSumX2	+= pow(lfResult, 2.0);
		}
		lfNum = lfIQSExecs*lfIQSSumX2 - pow(lfIQSSumX, 2.0);
		lfDenom = lfIQSExecs*(lfIQSExecs-1.0);
		clParameterStat.m_fIQS = sqrt(fabs(lfNum/lfDenom));
	}


}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Computes shape distribution on one parameter. This function also
//				 computes the Skew, Kurtosis, lDifferentValues, bIntegerValues
//
//				 Skew         = Population degree of asymmetry of a distribution around its mean
//				 Kurtosis     = Population Kurtosis or relative peakedness or flatness of a distribution compared with the normal distribution
//
//				 Note: the array starting at position 0, 1 must be retrieved from any above position
//
// Argument(s) :
//
//    C_Gate_DataModel_ParameterStat & clParameterStat
//
//    C_Gate_DataModel_TestResultVector & TestResultVector
//
//    unsigned int uiRemoveLow
//         for remove outliers data
//
//    unsigned int uiRemoveHigh
//         for remove outliers data
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::ComputeShapeDistribution(C_Gate_DataModel_ParameterStat & clParameterStat, C_Gate_DataModel_TestResultVector & TestResultVector, unsigned int uiRemoveLow/* = 0*/, unsigned int uiRemoveHigh/* = 0*/)
{
	unsigned int	uiStartPosition = uiRemoveLow;
	unsigned int	uiEndPosition = TestResultVector.CountValues() - uiRemoveHigh;
	unsigned int	uiExecs = uiEndPosition - uiStartPosition;
	unsigned int	uiRunIndex;

	if ((unsigned int) TestResultVector.count() <= uiStartPosition)
		return;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Compute number of different values (used to detect distributions with few indexed values)
	// Detect if serie of integer numbers.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	long	lDifferentValues=1;
	float	fSampleValue;
	float	fValue = TestResultVector[uiStartPosition]->m_pTestResult->m_fResult;
	for(uiRunIndex = uiStartPosition+1; uiRunIndex < uiEndPosition; uiRunIndex++)
	{
		fSampleValue = TestResultVector[uiRunIndex]->m_pTestResult->m_fResult;
		if(fValue < fSampleValue)
		{
			lDifferentValues++;
			fValue = fSampleValue;
		}
	}
	clParameterStat.m_lDifferentValues = lDifferentValues;


	// Make sure parameter has at leat 4 results
	// Check if we can compute Shape Distribution
	if((uiExecs < 4)
	|| (clParameterStat.m_fSigma == 0.0))
	{
		if(uiExecs > 0)
			clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeCategory);
		return;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Compute Skewness (Population degree of asymmetry of a distribution around its mean)
	// Unbiased Skew = {N/[(N-1)*(N-2)]} * Sum(pow((X - Mean)/Sigma,3))
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Go through list of values, and compute required sums
	double	lfValue = 0.0;
	for(uiRunIndex = uiStartPosition; uiRunIndex < uiEndPosition; uiRunIndex++)
		lfValue += pow(((double)(TestResultVector[uiRunIndex]->m_pTestResult->m_fResult)-(double)(clParameterStat.m_fMean))/(double)(clParameterStat.m_fSigma) , 3.0);

	// Compute Skew indicator
	lfValue *= (double)(uiExecs)/(double)((uiExecs-1)*(uiExecs-2));
	clParameterStat.m_lfSamplesSkew = lfValue;


	/////////////////////////////////////////////////////////////////////////////
	// Compute Kurtosis (Population Kurtosis or relative peakedness or flatness of a distribution compared with the normal distribution)
	// Unbiased Kurtosis = {[N*(N+1)/(N-1)*(N-2)*(N-3)] * Sum(pow((X - Mean)/Sigma,4))} - [3*pow(N-1,2)]/[(N-2)/(N-3)]
	/////////////////////////////////////////////////////////////////////////////
	// Go through list of values, and compute required sums
	double lfKurtosisSum = 0.0;
	for(uiRunIndex = uiStartPosition; uiRunIndex < uiEndPosition; uiRunIndex++)
		lfKurtosisSum += pow(((double)(TestResultVector[uiRunIndex]->m_pTestResult->m_fResult)-(double)(clParameterStat.m_fMean))/(double)(clParameterStat.m_fSigma), 4.0);

	// Compute Kurtosis indicator
	double lfValue1 = ((double)(uiExecs)*(double)(uiExecs+1))/((double)(uiExecs-1)*(double)(uiExecs-2)*(double)(uiExecs-3));
	double lfValue2 = (3.0*pow((double)(uiExecs-1),2))/((double)(uiExecs-2)*(double)(uiExecs-3));
	clParameterStat.m_lfSamplesKurtosis = lfValue1*lfKurtosisSum - lfValue2;

	/////////////////////////////////////////////////////////////////////////////
	// Compute the Shape Distribution
	/////////////////////////////////////////////////////////////////////////////
	Shape_UpdateDistributionType(clParameterStat, true);

}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Computes X-Bar R stats
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	C_Gate_DataModel_TestResult *pTestResults
//		<description>
//
//	unsigned int uiParameterIndex
//		<description>
//
//	unsigned int uiNbRuns
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void
C_Gate_DataStats::
ComputeXBarRStats(C_Gate_DataModel_ParameterStat& clParameterStat,
				  C_Gate_DataModel_TestResult* pTestResults,
				  C_Gate_DataModel_PartResult* /*pPartResults*/,
				  unsigned int uiParameterIndex,
				  unsigned int uiNbRuns)
{

	unsigned int	uiRunIndex;
	long			lIndex;
	unsigned int	uiNbValue = 0;
	unsigned int	uiNbStep = 0;

	float			fXBar = 0.0;
	float			fXBarBar = 0.0;
	float			fRange = 0.0;
	float			fRangeBar = 0.0;
	float			fXMin = GEX_PLUGIN_INVALID_VALUE_FLOAT;
	float			fXMax = GEX_PLUGIN_INVALID_VALUE_FLOAT;
  //FIXME: not used ?
  //float fXUWL = GEX_PLUGIN_INVALID_VALUE_FLOAT;
  //float fXLWL = GEX_PLUGIN_INVALID_VALUE_FLOAT;
	float			fRMin = GEX_PLUGIN_INVALID_VALUE_FLOAT;
	float			fRMax = GEX_PLUGIN_INVALID_VALUE_FLOAT;
	float			fMinValue = GEX_PLUGIN_INVALID_VALUE_FLOAT;
	float			fMaxValue = GEX_PLUGIN_INVALID_VALUE_FLOAT;

	//constants declaration
	float			tabA2[11] = {0.000F,0.000F,1.880F,1.023F,0.729F,0.577F,0.483F,0.419F,0.373F,0.337F,0.308F};
	//TODO: unused tabD2 ?
	//float			tabD2[11] = {0.000F,0.000F,1.128F,1.693F,2.059F,2.326F,2.534F,2.704F,2.847F,2.970F,3.078F};
	float			tabD3[11] = {0.000F,0.000F,0.000F,0.000F,0.000F,0.000F,0.000F,0.076F,0.136F,0.184F,0.223F};
	float			tabD4[11] = {0.000F,0.000F,3.267F,2.574F,2.282F,2.114F,2.004F,1.924F,1.864F,1.816F,1.777F};

	// Adjust the Nb points per SubGroup
	// depend of the NbRuns
	// must be included between 2 and 10 values
	unsigned int	uiNbPts;

	uiNbPts = (unsigned int)gex_max(gex_min((float)(uiNbRuns)/10.0 , 10.0),2.0);

	for(uiRunIndex=0 ; uiRunIndex<uiNbRuns ; uiRunIndex++)
	{
		lIndex = uiParameterIndex*uiNbRuns + uiRunIndex;
		if( (pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_EXECUTED)
//		select only the 95% of the value between +/- 3 sigma
//		&& ((clParameterStat.m_fMean-(clParameterStat.m_fSigma*3.0) <= pTestResults[lIndex].m_fResult) && (pTestResults[lIndex].m_fResult <= clParameterStat.m_fMean+(clParameterStat.m_fSigma*3.0)))
		&& !(pTestResults[lIndex].m_uiFlags & (YIELD123_TESTRESULT_OUTLIER_L | YIELD123_TESTRESULT_OUTLIER_H)))
		{
			// collect information only for non-outliers tests
			if((fMinValue == GEX_PLUGIN_INVALID_VALUE_FLOAT)
			|| (fMaxValue == GEX_PLUGIN_INVALID_VALUE_FLOAT))
				fMinValue = fMaxValue = pTestResults[lIndex].m_fResult;

			fMinValue = gex_min(fMinValue,pTestResults[lIndex].m_fResult);
			fMaxValue = gex_max(fMaxValue,pTestResults[lIndex].m_fResult);
			fXBar += pTestResults[lIndex].m_fResult;
			uiNbValue++;
		}

		if((uiNbValue == uiNbPts)
		|| (uiRunIndex == uiNbRuns-1))
		{
			// verify if we have the 10 value else no result for this last step
			if(uiNbValue == uiNbPts)
			{
				// have collected all information for one step
				// compute stat
				fXBar /= (float)(uiNbValue);
				fXBarBar += fXBar;
				fRange = fMaxValue - fMinValue;
				fRangeBar += fRange;

				if((fXMin == GEX_PLUGIN_INVALID_VALUE_FLOAT)
				|| (fXMax == GEX_PLUGIN_INVALID_VALUE_FLOAT))
				{
					fXMin = fMinValue;
					fXMax = fMaxValue;
				}
				fXMin = gex_min(fMinValue,fXMin);
				fXMax = gex_max(fMaxValue,fXMax);

				if((fRMin == GEX_PLUGIN_INVALID_VALUE_FLOAT)
				|| (fRMax == GEX_PLUGIN_INVALID_VALUE_FLOAT))
				{
					fRMin = fRMax = fRange;
				}
				fRMin = gex_min(fRange,fRMin);
				fRMax = gex_max(fRange,fRMax);

				uiNbStep++;
				uiNbValue = 0;
				fXBar = 0.0;
				fRange = 0.0;
				fMinValue = fMaxValue = GEX_PLUGIN_INVALID_VALUE_FLOAT;
			}
		}

	}

	// final step
	// compute stat
	fXBarBar /=(float)uiNbStep;
	fRangeBar /= (float)uiNbStep;
	clParameterStat.m_fXBarBar = fXBarBar;
	clParameterStat.m_fXBarUCL = fXBarBar + tabA2[uiNbPts]*fRangeBar;
	clParameterStat.m_fXBarLCL = fXBarBar - tabA2[uiNbPts]*fRangeBar;
  //FIXME: not used ?
  //fXUWL = fXBarBar + tabA2[uiNbPts]*fRangeBar*2/3;
  //fXLWL = fXBarBar - tabA2[uiNbPts]*fRangeBar*2/3;
	clParameterStat.m_fRBar = fRangeBar;
	clParameterStat.m_fRUCL = tabD4[uiNbPts]*fRangeBar;
	clParameterStat.m_fRLCL = tabD3[uiNbPts]*fRangeBar;
	clParameterStat.m_uiNbPointsPerSubGroup = uiNbPts;


}



/////////////////////////////////////////////////////////////////////////////////////////
// Description : Identify distribution type: NORMAL
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	bool bUseSmoothHisto
//		true if we want to use the HistoClasses computed in a restricted space
//
// Return type : true if NORMAL
/////////////////////////////////////////////////////////////////////////////////////////
bool
C_Gate_DataStats::
Shape_isDistributionNormal(C_Gate_DataModel_ParameterStat& clParameterStat,
						   bool /*bUseSmoothHisto*/)
{
	// Check if close to ideal Gaussian
	if(fabs(clParameterStat.m_lfSamplesSkew) < 0.5)
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeGaussian);
	}
	// Check if Left tailed Gaussian
	else if(clParameterStat.m_lfSamplesSkew < 0.0)
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeGaussianLeft);
	}
	// Then can only be Right tailed Gaussian
	else
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeGaussianRight);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Identify distribution type: Log normal (Left tailed) distribution
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	double lfQ1,lfQ2,lfQ3,lfQ4
//		quartiles
//
//	bool bUseSmoothHisto
//		true if we want to use the HistoClasses computed in a restricted space
//
// Return type : true if Log normal (Left tailed) distribution
/////////////////////////////////////////////////////////////////////////////////////////
bool
C_Gate_DataStats::
Shape_isDistributionLogNormalLeftTailed(C_Gate_DataModel_ParameterStat&
										clParameterStat,
										double lfQ1, double lfQ2,
										double lfQ3, double lfQ4,
										bool /*bUseSmoothHisto*/)
{
	// Not enough tail for a LogNormal
	if(lfQ1 < 45)
		return false;

	// Head's slope not high enough for a Lognormal.
	if(lfQ4 > 15)
		return false;

	// If each quartile gets smaller in range space, then yes it's a log normal!
	if(lfQ1 > lfQ2 && lfQ2 > lfQ3 && lfQ3 > lfQ4)
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeLogNormalLeft);
		return true;
	}

	double lfRatio = lfQ2/lfQ3;
	if(lfRatio < 1) lfRatio = 1/lfRatio;

	// If Last quartile used a very small range, then yes it's definitely a LogNormal!
	if(lfQ4 < 2 && (lfRatio < 1.8))
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeLogNormalLeft);
		return true;
	}

	if(lfQ1 > lfQ2 &&  lfQ2 > lfQ3)
	{
		// Lognormal
		if(lfRatio > 1.8)
		{
			clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeLogNormalLeft);
			return true;
		}
		// If ratio not high enough, but still Q2 & Q3 very small, then this is fine too.
		if(gex_max(lfQ2,lfQ3) <= 5)
		{
			clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeLogNormalLeft);
			return true;
		}
	}

	// Not log normal-left tailed.
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Identify distribution type: Log normal (Right tailed) distribution
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	double lfQ1,lfQ2,lfQ3,lfQ4
//		quartiles
//
//	bool bUseSmoothHisto
//		true if we want to use the HistoClasses computed in a restricted space
//
// Return type : true if Log normal (Right tailed) distribution
/////////////////////////////////////////////////////////////////////////////////////////
bool	C_Gate_DataStats::Shape_isDistributionLogNormalRightTailed(C_Gate_DataModel_ParameterStat & clParameterStat,double lfQ1,double lfQ2,double lfQ3,double lfQ4, bool bUseSmoothHisto)
{
	// Swap Quartiles and call function to detect a left tail!
	double lfSwap = lfQ1;
	lfQ1 = lfQ4;
	lfQ4 = lfSwap;

	lfSwap = lfQ2;
	lfQ2 = lfQ3;
	lfQ3  = lfSwap;

	if(Shape_isDistributionLogNormalLeftTailed(clParameterStat,lfQ1,lfQ2,lfQ3,lfQ4, bUseSmoothHisto))
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeLogNormalRight);
		return true;
	}

	// Not Lognormal right-tailed
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Identify distribution type: Log normal (Left or Right tailed) distribution
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	double lfQ1,lfQ2,lfQ3,lfQ4
//		quartiles
//
//	bool bUseSmoothHisto
//		true if we want to use the HistoClasses computed in a restricted space
//
// Return type : true if Log normal (Left or Right tailed) distribution
/////////////////////////////////////////////////////////////////////////////////////////
bool	C_Gate_DataStats::Shape_isDistributionLogNormal(C_Gate_DataModel_ParameterStat & clParameterStat,double lfQ1,double lfQ2,double lfQ3,double lfQ4, bool bUseSmoothHisto)
{
	// First check if Noraml Left-tailed
	if(Shape_isDistributionLogNormalLeftTailed(clParameterStat,lfQ1,lfQ2,lfQ3,lfQ4, bUseSmoothHisto))
		return true;

	// NO...then check if Normal Right-tailed
	return Shape_isDistributionLogNormalRightTailed(clParameterStat,lfQ1,lfQ2,lfQ3,lfQ4, bUseSmoothHisto);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Identify distribution type: Multi-Modal distribution
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	double lfQ1,lfQ2,lfQ3,lfQ4
//		quartiles
//
//	bool bUseSmoothHisto
//		true if we want to use the HistoClasses computed in a restricted space
//
// Return type : true if Multi-Modal distribution
/////////////////////////////////////////////////////////////////////////////////////////
bool	C_Gate_DataStats::Shape_isDistributionMultiModal(C_Gate_DataModel_ParameterStat & clParameterStat,double lfQ1,double lfQ2,double lfQ3,double lfQ4, bool bUseSmoothHisto)
{

	C_Gate_DataModel_Histogram	*pHistogram;
	if(bUseSmoothHisto)
		pHistogram = clParameterStat.m_pHistogramSmooth;
	else
		pHistogram = clParameterStat.m_pHistogram;

	if(pHistogram== NULL)
	{
		return false;
	}

	float	fPercent=0;
	float	fMax=0;
	float	fMin=0;
	int		iSmallCell=0;
	int		iPickFound=0;
	bool	bPickFound=false;
	int		iIndex;

	// INITIALIZATION
	float	fPickValue=3.9F;	// to consider a bar like a pick must be larger than 3.9
	float	fHoleValue=2.1F;	// to consider a bar like a hole must be smaller than 2.1
	int		iNbCell= 2;			// adaptive : Nb cell used to consider a hole

	iNbCell = pHistogram->m_uiNbClasses/20; // between 2 and 5
	if(iNbCell < 2)
		iNbCell = 2;

	fMax = fPickValue;
	fMin = fHoleValue;

	for(iIndex=0;iIndex<(int)pHistogram->m_uiNbClasses;iIndex++)
	{

		fPercent = (100.0*pHistogram->m_pHistoClasses[iIndex].m_uiSamples/clParameterStat.m_uiExecs);
		if(fPercent > fMax)
		{
			if(!bPickFound)
			{
				iPickFound ++;
				iSmallCell = 0;
				bPickFound = true;
				fMax = fPercent;
			}
			fMax = gex_max(fMax, fPercent);
			fMin = gex_max(fMax/2,fHoleValue);;
		}
		else if(fPercent < fMax/2)
		{
			if(bPickFound)
			{
				iSmallCell ++;
				if(iSmallCell >= iNbCell)
					bPickFound = false;
			}
			if(fPercent < fMin)
			{
				fMin = gex_min(fMin,fPercent);
				fMax = gex_max(fMin*2,fPickValue);
			}
		}
	}

	if((iPickFound == 2)
	&& (lfQ1 < 30 && lfQ4 < 30 && (lfQ2 + lfQ3) > 50))
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeBiModal);
		return true;
	}
	if(iPickFound >2)
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeMultiModal);
		return true;
	}


	// Not Multi-Modal
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Identify distribution type: Clamped distribution
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	bool bUseSmoothHisto
//		true if we want to use the HistoClasses computed in a restricted space
//
// Return type : true if Clamped distribution
/////////////////////////////////////////////////////////////////////////////////////////
bool	C_Gate_DataStats::Shape_isDistributionClamped(C_Gate_DataModel_ParameterStat & clParameterStat, bool bUseSmoothHisto)
{
	// Compute the highest bar in the histogram.
	int	iHighestRank = 0;
	int	iIndex;

	C_Gate_DataModel_Histogram	*pHistogram;
	if(bUseSmoothHisto)
		pHistogram = clParameterStat.m_pHistogramSmooth;
	else
		pHistogram = clParameterStat.m_pHistogram;


	for(iIndex=0;iIndex<(int)pHistogram->m_uiNbClasses;iIndex++)
		iHighestRank = gex_max((int)pHistogram->m_pHistoClasses[iIndex].m_uiSamples,iHighestRank);

	// No data!
	if(iHighestRank == 0)
		return false;

	// Check if clamped LEFT
	bool	bLeftClamped = false;
	double	lfValue = pHistogram->m_pHistoClasses[0].m_uiSamples*100.0/(double)iHighestRank;
	if(lfValue >= 50)
		bLeftClamped = true;

	// Check if clamped RIGHT
	bool	bRightClamped = false;
	lfValue = pHistogram->m_pHistoClasses[pHistogram->m_uiNbClasses-1].m_uiSamples*100.0/(double)iHighestRank;
	if(lfValue >= 50)
		bRightClamped = true;

	if(bLeftClamped)
	{
		if(bRightClamped)
			clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeClampedDouble);
		else
			clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeClampedLeft);
		return true;
	}
	if(bRightClamped)
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeClampedRight);
		return true;
	}

	// Not Clamped
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Tells type of distribution based on percentage of samples in each quartile
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	bool bUseSmoothHisto
//		true if we want to use the HistoClasses computed in a restricted space
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::Shape_UpdateDistributionType(C_Gate_DataModel_ParameterStat & clParameterStat, bool bUseSmoothHisto)
{
	if( clParameterStat.m_fQ1 >= GEX_PLUGIN_INVALID_VALUE_FLOAT &&
		clParameterStat.m_fQ2 >= GEX_PLUGIN_INVALID_VALUE_FLOAT &&
		clParameterStat.m_fQ3 >= GEX_PLUGIN_INVALID_VALUE_FLOAT &&
		clParameterStat.m_fMin >= GEX_PLUGIN_INVALID_VALUE_FLOAT &&
		clParameterStat.m_fMax >= GEX_PLUGIN_INVALID_VALUE_FLOAT)
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeUnknow);
		return;
	}

	// Check if 'categories' or 'unknown'
	if((clParameterStat.m_pHistogram == NULL))
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeUnknow);

		return;
	}

	// Check if 'categories' or 'unknown'
	if((clParameterStat.m_uiExecs <= 5)
	|| !(clParameterStat.m_uiFlags & YIELD123_PARAMETERSTATS_HASFRESULTS)
	|| (clParameterStat.m_lDifferentValues < 5.0))
	{
		clParameterStat.UpdateShapeDistribution(C_Gate_DataModel_ParameterStat::eShapeCategory);

		return;
	}

	double lfQ1 = (clParameterStat.m_fQ1-clParameterStat.m_fMin) * 100 / (clParameterStat.m_fRange);
	double lfQ2 = (clParameterStat.m_fQ2-clParameterStat.m_fQ1) * 100 / (clParameterStat.m_fRange);
	double lfQ3 = (clParameterStat.m_fQ3-clParameterStat.m_fQ2) * 100 / (clParameterStat.m_fRange);
	double lfQ4 = (clParameterStat.m_fMax -clParameterStat.m_fQ3) * 100 / (clParameterStat.m_fRange);

	// If one quartile is empty, at least give it a tiny value!
	if(!lfQ1) lfQ1 = 1e-6;
	if(!lfQ2) lfQ2 = 1e-6;
	if(!lfQ3) lfQ3 = 1e-6;
	if(!lfQ4) lfQ4 = 1e-6;

	// Check for Multi-modal
	if(Shape_isDistributionMultiModal(clParameterStat,lfQ1,lfQ2,lfQ3,lfQ4, bUseSmoothHisto))
		return;

	// Check for LogNormal
	if(Shape_isDistributionLogNormal(clParameterStat,lfQ1,lfQ2,lfQ3,lfQ4, bUseSmoothHisto))
		return;

	// Not dual clamped...then maybe single-Clamped
	if(Shape_isDistributionClamped(clParameterStat, bUseSmoothHisto))
		return;

	// Assume Gaussian if all other shape identification failed!
	Shape_isDistributionNormal(clParameterStat, bUseSmoothHisto);
}




/////////////////////////////////////////////////////////////////////////////////////////
// Description : Mark outliers in test result vector
//				 !!!! THIS ALGO IS OPTIMIZED. !!!!
//				 !!!! IT IS VERY IMPORTANT THAT THE RESULT VECTOR IS SORTED !!!!
//
// Argument(s) :
//
//	C_Gate_DataModel_Site *pSite
//		<description>
//
//	C_Gate_DataModel_TestResultVector & TestResultVector
//		<description>
//
//	unsigned int uiParameterIndex
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::MarkOutliers_RobustSigma(C_Gate_DataModel_Site *pSite, C_Gate_DataModel_TestResultVector & TestResultVector, unsigned int uiParameterIndex)
{
	// Make sure parameter has at leat 4 results
	if(TestResultVector.CountValues() == 0)
		return;

	C_Gate_DataModel_TestResult_Item *pTestResultItem;
	C_Gate_DataModel_ParameterStat	*pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
	int							iIndex;
	float						fLCL, fUCL;

	if(	(pParameterStat->m_fRobustSigma != GEX_PLUGIN_INVALID_VALUE_FLOAT) && (pParameterStat->m_fRobustMean != GEX_PLUGIN_INVALID_VALUE_FLOAT) &&
		(pParameterStat->m_fRobustSigma != 0.0))
	{
		fLCL = pParameterStat->m_fRobustMean - 6.0*pParameterStat->m_fRobustSigma;
		fUCL = pParameterStat->m_fRobustMean + 6.0*pParameterStat->m_fRobustSigma;

		// As the result vector is sorted, we can optimize the outlier marking going through the vector in both
		// direction until limit is reached
		iIndex = 0;
		pTestResultItem = TestResultVector[iIndex];
		while((iIndex < (int)TestResultVector.CountValues()) && (pTestResultItem->m_pTestResult->m_fResult < fLCL))
		{
			pTestResultItem->m_pTestResult->m_uiFlags |= YIELD123_TESTRESULT_OUTLIER_L;
			(pParameterStat->m_uiOutliers_L)++;
			if(!(pSite->m_pPartResults[pTestResultItem->m_uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_FAIL))
				(pParameterStat->m_uiOutliers_L_GoodPart)++;
			if(!(pSite->m_pPartResults[pTestResultItem->m_uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_HASBEENRETESTED))
				(pParameterStat->m_uiOutliers_L_LastRetest)++;
			pSite->m_pPartResults[pTestResultItem->m_uiRunIndex].m_uiFlags |= YIELD123_PARTRESULT_HASOUTLIERS;
			iIndex++;
			pTestResultItem = TestResultVector[iIndex];
		}
		iIndex = TestResultVector.CountValues()-1;
		pTestResultItem = TestResultVector[iIndex];
		while((iIndex >= 0) && (pTestResultItem->m_pTestResult->m_fResult > fUCL))
		{
			pTestResultItem->m_pTestResult->m_uiFlags |= YIELD123_TESTRESULT_OUTLIER_H;
			(pParameterStat->m_uiOutliers_H)++;
			if(!(pSite->m_pPartResults[pTestResultItem->m_uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_FAIL))
				(pParameterStat->m_uiOutliers_H_GoodPart)++;
			if(!(pSite->m_pPartResults[pTestResultItem->m_uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_HASBEENRETESTED))
				(pParameterStat->m_uiOutliers_H_LastRetest)++;
			pSite->m_pPartResults[pTestResultItem->m_uiRunIndex].m_uiFlags |= YIELD123_PARTRESULT_HASOUTLIERS;
			iIndex--;
			pTestResultItem = TestResultVector[iIndex];
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Mark outliers in test result array using the standard sigma. This function
//				 does a recursive outlier removal until no more outliers to remove
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	unsigned int uiParameterIndex
//		<description>
//
//	unsigned int uiNbRuns
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::MarkOutliers_Standard(C_Gate_DataModel_Site *pSite, unsigned int uiParameterIndex)
{
	float						fMean, fSigma, fLCL, fUCL;
	unsigned int				uiOutliers, uiPasses=0;
	C_Gate_DataModel_ParameterStat	clParameterStat;

	fMean	= pSite->m_pParameterStats_AllData[uiParameterIndex].m_fMean;
	fSigma	= pSite->m_pParameterStats_AllData[uiParameterIndex].m_fSigma;

	if(	(fSigma != GEX_PLUGIN_INVALID_VALUE_FLOAT) && (fMean != GEX_PLUGIN_INVALID_VALUE_FLOAT) &&
		(fSigma != 0.0))
	{
		fLCL = fMean - 6.0*fSigma;
		fUCL = fMean + 6.0*fSigma;
		uiOutliers = MarkOutliers_Standard(pSite, fLCL, fUCL, uiParameterIndex);
		uiPasses++;

		while(uiOutliers != 0)
		{
			ComputeStatCumuls(clParameterStat, pSite->m_pTestResults, pSite->m_pPartResults, uiParameterIndex, pSite->m_uiNbRuns, eResult_NotOutlier);
			ComputeBasicStats(clParameterStat, NULL);
			fMean	= clParameterStat.m_fMean;
			fSigma	= clParameterStat.m_fSigma;
			fLCL = fMean - 6.0*fSigma;
			fUCL = fMean + 6.0*fSigma;
			uiOutliers = MarkOutliers_Standard(pSite, fLCL, fUCL, uiParameterIndex);
			uiPasses++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Mark outliers in test result array using the standard sigma
//
// Argument(s) :
//
//	C_Gate_DataModel_Site *pSite
//		<description>
//
//	float fLCL
//		<description>
//
//	float fHCL
//		<description>
//
//	unsigned int uiParameterIndex
//		<description>
//
// Return type : int
/////////////////////////////////////////////////////////////////////////////////////////
int C_Gate_DataStats::MarkOutliers_Standard(C_Gate_DataModel_Site *pSite, float fLCL, float fHCL, unsigned int uiParameterIndex)
{
	C_Gate_DataModel_ParameterStat	*pParameterStat = pSite->m_pParameterStats_AllData + uiParameterIndex;
	unsigned int				uiNbRuns = pSite->m_uiNbRuns;
	unsigned int				uiRunIndex, uiOutliersMarked=0;
	long						lIndex;

	for(uiRunIndex = 0; uiRunIndex < uiNbRuns; uiRunIndex++)
	{
		lIndex = uiParameterIndex*uiNbRuns+uiRunIndex;
		if(	(pSite->m_pTestResults[lIndex].m_uiFlags & YIELD123_TESTRESULT_EXECUTED) &&
			!(pSite->m_pTestResults[lIndex].m_uiFlags & (YIELD123_TESTRESULT_OUTLIER_L | YIELD123_TESTRESULT_OUTLIER_H)))
		{
			if(pSite->m_pTestResults[lIndex].m_fResult < fLCL)
			{
				uiOutliersMarked++;
				pSite->m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_OUTLIER_L;
				(pParameterStat->m_uiOutliers_L)++;
				if(!(pSite->m_pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_FAIL))
					(pParameterStat->m_uiOutliers_L_GoodPart)++;
				if(!(pSite->m_pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_HASBEENRETESTED))
					(pParameterStat->m_uiOutliers_L_LastRetest)++;
				pSite->m_pPartResults[uiRunIndex].m_uiFlags |= YIELD123_PARTRESULT_HASOUTLIERS;
			}
			if(pSite->m_pTestResults[lIndex].m_fResult > fHCL)
			{
				uiOutliersMarked++;
				pSite->m_pTestResults[lIndex].m_uiFlags |= YIELD123_TESTRESULT_OUTLIER_H;
				(pParameterStat->m_uiOutliers_H)++;
				if(!(pSite->m_pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_FAIL))
					(pParameterStat->m_uiOutliers_H_GoodPart)++;
				if(!(pSite->m_pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_HASBEENRETESTED))
					(pParameterStat->m_uiOutliers_H_LastRetest)++;
				pSite->m_pPartResults[uiRunIndex].m_uiFlags |= YIELD123_PARTRESULT_HASOUTLIERS;
			}
		}
	}

	return uiOutliersMarked;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Copy flags from the stat structure holding stats on all data to the stats
//				 structure holding stats on data without outliers
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat *pSource
//		Source stat structure
//
//	C_Gate_DataModel_ParameterStat *pDest
//		Dest stat structure
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::CopyStatFlags(C_Gate_DataModel_ParameterStat *pSource, C_Gate_DataModel_ParameterStat *pDest)
{
	if(pSource->m_uiFlags & YIELD123_PARAMETERSTATS_HASVALIDRESULTS)
		pDest->m_uiFlags |= YIELD123_PARAMETERSTATS_HASVALIDRESULTS;
	if(pSource->m_uiFlags & YIELD123_PARAMETERSTATS_HASFRESULTS)
		pDest->m_uiFlags |= YIELD123_PARAMETERSTATS_HASFRESULTS;
	if(pSource->m_uiFlags & YIELD123_PARAMETERSTATS_ARTIFICIAL_LL)
		pDest->m_uiFlags |= YIELD123_PARAMETERSTATS_ARTIFICIAL_LL;
	if(pSource->m_uiFlags & YIELD123_PARAMETERSTATS_ARTIFICIAL_HL)
		pDest->m_uiFlags |= YIELD123_PARAMETERSTATS_ARTIFICIAL_HL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Computes cumuls for statistical computations
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	C_Gate_DataModel_TestResultVector & TestResultVector
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::ComputeStatCumuls(C_Gate_DataModel_ParameterStat & clParameterStat, C_Gate_DataModel_TestResultVector & TestResultVector)
{
	unsigned int uiIndex;

	clParameterStat.ClearData();

	for (uiIndex = 0; uiIndex < TestResultVector.CountValues(); uiIndex++) {
		UpdateStatCumuls(clParameterStat,
						 TestResultVector[uiIndex]->m_pTestResult->m_uiFlags,
						 TestResultVector[uiIndex]->m_pTestResult->m_fResult);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Computes cumuls for statistical computations
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	C_Gate_DataModel_TestResult *pTestResults
//		<description>
//
//	unsigned int uiParameterIndex
//		<description>
//
//	unsigned int uiNbRuns
//		<description>
//
//	ResultFilter eResultFilter /*= eResult_Any*/
//		<description>
//
//	RunFilter eRunFilter /*= eRun_Any*/
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::ComputeStatCumuls(C_Gate_DataModel_ParameterStat & clParameterStat, C_Gate_DataModel_TestResult *pTestResults, C_Gate_DataModel_PartResult *pPartResults, unsigned int uiParameterIndex, unsigned int uiNbRuns, ResultFilter eResultFilter /*= eResult_Any*/, RunFilter eRunFilter/* = eRun_Any*/)
{
	unsigned int	uiRunIndex, uiFlags;
	long			lIndex;
	float			fResult;

	clParameterStat.ClearData();

	for(uiRunIndex = 0; uiRunIndex < uiNbRuns; uiRunIndex++)
	{
		if(	(eRunFilter == eRun_Any) ||
			((eRunFilter == eRun_Pass) && !(pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_FAIL)) ||
			((eRunFilter == eRun_Fail) && (pPartResults[uiRunIndex].m_uiFlags & YIELD123_PARTRESULT_FAIL)))
		{
			lIndex = uiParameterIndex*uiNbRuns+uiRunIndex;
			uiFlags = (pTestResults + lIndex)->m_uiFlags;
			fResult = (pTestResults + lIndex)->m_fResult;
			if(uiFlags & YIELD123_TESTRESULT_EXECUTED)
			{
				switch(eResultFilter)
				{
					case eResult_NotOutlier:
						if(!(uiFlags & (YIELD123_TESTRESULT_OUTLIER_H | YIELD123_TESTRESULT_OUTLIER_L)))
							UpdateStatCumuls(clParameterStat, uiFlags, fResult);
						break;

					case eResult_Outlier:
						if(uiFlags & (YIELD123_TESTRESULT_OUTLIER_H | YIELD123_TESTRESULT_OUTLIER_L))
							UpdateStatCumuls(clParameterStat, uiFlags, fResult);
						break;

					case eResult_LowOutlier:
						if(uiFlags & YIELD123_TESTRESULT_OUTLIER_L)
							UpdateStatCumuls(clParameterStat, uiFlags, fResult);
						break;

					case eResult_HighOutlier:
						if(uiFlags & YIELD123_TESTRESULT_OUTLIER_H)
							UpdateStatCumuls(clParameterStat, uiFlags, fResult);
						break;

					case eResult_Pass:
						if(!(uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED))
							UpdateStatCumuls(clParameterStat, uiFlags, fResult);
						break;

					case eResult_Fail:
						if(uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED)
							UpdateStatCumuls(clParameterStat, uiFlags, fResult);
						break;

					case eResult_LowFail:
						if((uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED) && (uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_L))
							UpdateStatCumuls(clParameterStat, uiFlags, fResult);
						break;

					case eResult_HighFail:
						if((uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED) && (uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_H))
							UpdateStatCumuls(clParameterStat, uiFlags, fResult);
						break;

					case eResult_Any:
					default:
						UpdateStatCumuls(clParameterStat, uiFlags, fResult);
						break;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Computes histogram classes
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	C_Gate_DataModel_TestResult *pTestResults
//		<description>
//
//	unsigned int uiParameterIndex
//		<description>
//
//	unsigned int uiNbRuns
//		<description>
//
//	ResultFilter eResultFilter /*= eResult_Any*/
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::ComputeHistoClasses(C_Gate_DataModel_ParameterStat & clParameterStat, unsigned int uiNbClasses, C_Gate_DataModel_TestResult *pTestResults, unsigned int uiParameterIndex, unsigned int uiNbRuns, ResultFilter eResultFilter /*= eResult_Any*/)
{
	unsigned int	uiRunIndex, uiFlags;
	long			lIndex;
	float			fResult;

	if(clParameterStat.InitHistogram(uiNbClasses) == FALSE)
		return;

	for(uiRunIndex = 0; uiRunIndex < uiNbRuns; uiRunIndex++)
	{
		lIndex = uiParameterIndex*uiNbRuns+uiRunIndex;
		uiFlags = (pTestResults + lIndex)->m_uiFlags;
		fResult = (pTestResults + lIndex)->m_fResult;
		if(uiFlags & YIELD123_TESTRESULT_EXECUTED)
		{
			switch(eResultFilter)
			{
				case eResult_NotOutlier:
					if(!(uiFlags & (YIELD123_TESTRESULT_OUTLIER_H | YIELD123_TESTRESULT_OUTLIER_L)))
						clParameterStat.UpdateHistogram(fResult);
					break;

				case eResult_Outlier:
					if(uiFlags & (YIELD123_TESTRESULT_OUTLIER_H | YIELD123_TESTRESULT_OUTLIER_L))
						clParameterStat.UpdateHistogram(fResult);
					break;

				case eResult_LowOutlier:
					if(uiFlags & YIELD123_TESTRESULT_OUTLIER_L)
						clParameterStat.UpdateHistogram(fResult);
					break;

				case eResult_HighOutlier:
					if(uiFlags & YIELD123_TESTRESULT_OUTLIER_H)
						clParameterStat.UpdateHistogram(fResult);
					break;

				case eResult_Pass:
					if(!(uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED))
						clParameterStat.UpdateHistogram(fResult);
					break;

				case eResult_Fail:
					if(uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED)
						clParameterStat.UpdateHistogram(fResult);
					break;

				case eResult_LowFail:
					if((uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED) && (uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_L))
						clParameterStat.UpdateHistogram(fResult);
					break;

				case eResult_HighFail:
					if((uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED) && (uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_H))
						clParameterStat.UpdateHistogram(fResult);
					break;

				case eResult_Any:
				default:
					clParameterStat.UpdateHistogram(fResult);
					break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update cumuls for statistical computations
//
// Argument(s) :
//
//	C_Gate_DataModel_ParameterStat & clParameterStat
//		<description>
//
//	unsigned int uiFlags
//		<description>
//
//	float fResult
//		<description>
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void C_Gate_DataStats::UpdateStatCumuls(C_Gate_DataModel_ParameterStat & clParameterStat, unsigned int uiFlags, float fResult)
{
	// Update execution count
	clParameterStat.m_uiExecs++;
	// Update Pass/Fail counters
	if(uiFlags & YIELD123_TESTRESULT_FINALSTATUS_FAILED)
	{
		(clParameterStat.m_uiFail)++;
		if(uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_H)
			(clParameterStat.m_uiFail_H)++;
		else if(uiFlags & YIELD123_TESTRESULT_VALUE_FAILED_L)
			(clParameterStat.m_uiFail_L)++;
	}
	else
	{
		(clParameterStat.m_uiPass)++;
	}
	// Update cumuls, min, max...
	double lfResult = (double)fResult;
	clParameterStat.m_lfSumX += lfResult;
	clParameterStat.m_lfSumX2 += pow(lfResult, 2.0);
	if(clParameterStat.m_fMin == GEX_PLUGIN_INVALID_VALUE_FLOAT)
		clParameterStat.m_fMin = fResult;
	else
		clParameterStat.m_fMin = gex_min(clParameterStat.m_fMin, fResult);
	if(clParameterStat.m_fMax == GEX_PLUGIN_INVALID_VALUE_FLOAT)
		clParameterStat.m_fMax = fResult;
	else
		clParameterStat.m_fMax = gex_max(clParameterStat.m_fMax, fResult);
	clParameterStat.m_fRange = clParameterStat.m_fMax - clParameterStat.m_fMin;
}
