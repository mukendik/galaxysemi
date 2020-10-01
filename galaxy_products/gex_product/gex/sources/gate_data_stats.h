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
// This file defines classes to compute Yield-123 stats.
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef GEX_PLUGIN_YIELD123_STATS_H
#define GEX_PLUGIN_YIELD123_STATS_H

/////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
/////////////////////////////////////////////////////////////////////////////////////////
// Standard includes

// QT includes
// Local includes
#include "gate_data_model.h"

// Galaxy libraries includes
#include <gstdl_errormgr.h>

/////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS
/////////////////////////////////////////////////////////////////////////////////////////
// Minimum samples required to compute statistics
#define YIELD123_STATS_MINIMUMSAMPLES		5
#define YIELD123_STATS_MAXHISTOCLASSES		50

/////////////////////////////////////////////////////////////////////////////////////////
// DATA TYPES
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: C_Gate_DataStats
// Class holding statistical computation methods on Yield123 data
/////////////////////////////////////////////////////////////////////////////////////////
class C_Gate_DataStats
{
// ERROR MAP
public:
	GDECLARE_ERROR_MAP(C_Gate_DataStats)
	{
		eNotEnoughSamples,			// Not enough samples to compute reliable statistics
		eNoParameterSet,			// Attempt to compute stats, but no Parameter set is available
		eMalloc						// Memory allocation failure
	}
	GDECLARE_END_ERROR_MAP(C_Gate_DataStats)

// CONSTRUCTOR/DESTRUCTOR
public:
	C_Gate_DataStats(C_Gate_DataModel_Progress *pProgress);
	~C_Gate_DataStats();

// PUBLIC DATA
public:

// PUBLIC METHODS
public:
	bool	ComputeFirstLevelStats(C_Gate_DataModel *pclData, bool bComputeWaferStats = false, bool bOutliersRemoval = false);
	void	ComputeStatCumuls(C_Gate_DataModel_ParameterStat & clParameterStat, C_Gate_DataModel_TestResult *pTestResults, C_Gate_DataModel_PartResult *pPartResults, unsigned int uiParameterIndex, unsigned int uiNbRuns, ResultFilter eResultFilter = eResult_Any, RunFilter eRunFilter = eRun_Any);
	void	ComputeStatCumuls(C_Gate_DataModel_ParameterStat & clParameterStat, C_Gate_DataModel_TestResultVector & TestResultVector);
	void	ComputeBasicStats(C_Gate_DataModel_ParameterStat & clParameterStat, const C_Gate_DataModel_ParameterDef *pParameterDef);
	void	ComputeQuartiles(C_Gate_DataModel_ParameterStat & clParameterStat, C_Gate_DataModel_TestResultVector & TestResultVector, unsigned int uiRemoveLow = 0, unsigned int uiRemoveHigh = 0);
	void	ComputeShapeDistribution(C_Gate_DataModel_ParameterStat & clParameterStat, C_Gate_DataModel_TestResultVector & TestResultVector, unsigned int uiRemoveLow = 0, unsigned int uiRemoveHigh = 0);
	void	ComputeXBarRStats(C_Gate_DataModel_ParameterStat & clParameterStat, C_Gate_DataModel_TestResult *pTestResults, C_Gate_DataModel_PartResult *pPartResults, unsigned int uiParameterIndex, unsigned int uiNbRuns);


// PRIVATE METHODS
private:
	bool	ComputeFirstLevelStats(C_Gate_DataModel_Site *pSite, bool bOutliersRemoval = false);
	void	MarkOutliers_RobustSigma(C_Gate_DataModel_Site *pSite, C_Gate_DataModel_TestResultVector & TestResultVector, unsigned int uiParameterIndex);
	void	MarkOutliers_Standard(C_Gate_DataModel_Site *pSite, unsigned int uiParameterIndex);
	int		MarkOutliers_Standard(C_Gate_DataModel_Site *pSite, float fLCL, float fHCL, unsigned int uiParameterIndex);
	void	UpdateStatCumuls(C_Gate_DataModel_ParameterStat & clParameterStat, unsigned int uiFlags, float fResult);
	void	CopyStatFlags(C_Gate_DataModel_ParameterStat *pSource, C_Gate_DataModel_ParameterStat *pDest);
	void	SetStatFlags(C_Gate_DataModel_ParameterStat & clParameterStat, const C_Gate_DataModel_ParameterDef *pParameterDef);
	void	ComputeHistoClasses(C_Gate_DataModel_ParameterStat & clParameterStat, unsigned int uiNbClasses, C_Gate_DataModel_TestResult *pTestResults, unsigned int uiParameterIndex, unsigned int uiNbRuns, ResultFilter eResultFilter = eResult_Any);
	void	Shape_UpdateDistributionType(C_Gate_DataModel_ParameterStat & clParameterStat, bool bUseSmoothHisto);

// FUNCTION TO COMPUTE THE SHAPE DISTRIBUTION
	bool	Shape_isDistributionNormal(C_Gate_DataModel_ParameterStat & clParameterStat, bool bUseSmoothHisto);
	bool	Shape_isDistributionLogNormalLeftTailed(C_Gate_DataModel_ParameterStat & clParameterStat,double lfQ1,double lfQ2,double lfQ3,double lfQ4, bool bUseSmoothHisto);
	bool	Shape_isDistributionLogNormalRightTailed(C_Gate_DataModel_ParameterStat & clParameterStat,double lfQ1,double lfQ2,double lfQ3,double lfQ4, bool bUseSmoothHisto);
	bool	Shape_isDistributionLogNormal(C_Gate_DataModel_ParameterStat & clParameterStat,double lfQ1,double lfQ2,double lfQ3,double lfQ4, bool bUseSmoothHisto);
	bool	Shape_isDistributionMultiModalOld(C_Gate_DataModel_ParameterStat & clParameterStat,double lfQ1,double lfQ2,double lfQ3,double lfQ4, bool bUseSmoothHisto);
	bool	Shape_isDistributionMultiModal(C_Gate_DataModel_ParameterStat & clParameterStat,double lfQ1,double lfQ2,double lfQ3,double lfQ4, bool bUseSmoothHisto);
	bool	Shape_isDistributionClamped(C_Gate_DataModel_ParameterStat & clParameterStat, bool bUseSmoothHisto);

// FUNCTION TO COMPUTE WAFERMAP INFORMATION
	bool	ComputeWaferMapStats(C_Gate_DataModel_Batch *pBatch);
// PRIVATE DATA
private:
	C_Gate_DataModel_Progress*				m_pProgress;				// Progress dialog when analyzes in progress ...
	
};

#endif // #ifndef GEX_PLUGIN_YIELD123_STATS_H
