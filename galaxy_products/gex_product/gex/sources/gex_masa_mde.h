////////////////////////////////////////////////////////////////////////////////////////////////////
// File:		gex_masa_mde.h
// Description:	Data types definition for parameters passed to functions in MASA MDE DLL
// Created:		Tue 09 Mar 2004
// Revision:	1
////////////////////////////////////////////////////////////////////////////////////////////////////
// History:		
//	
// 09 Mar 2004:	created (Revision 1)
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// Notes:
// * Any module including this file needs to have access to QT for QString definition (qstring.h)
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _GEX_MASA_MDE_H_
#define _GEX_MASA_MDE_H_

// MASA MDE functions return codes
#define MDE_STATUS_OK					0
#define MDE_STATUS_USER_CANCELLED		1
#define MDE_STATUS_GENERIC_ERROR		2
#define MDE_STATUS_NOT_ENOUGH_MEMORY    3
#define MDE_STATUS_INVALID_ARGUMENT     4

// Structure for parameter definition
// (1 per parameter)
typedef struct t_GexMde_Parameter
{
	const char*			szParameterName;	// parameter name
	const char*			szUnits;			// parameter units
	long				lParameterIndex;	// parameter index (test number)
	int					nSiteIndex;			// index of measured site
	float				rLowerSpecLimit;	// lower specification limit for parameter
	float				rUpperSpecLimit;	// upper specification limit for parameter
} GexMde_Parameter;

// Structure for wafer identification
// (1 per wafer)
typedef struct t_GexMde_Wafer
{
	const char*			szLotID;			// lot name
	const char*			szWaferID;			// wafer ID
} GexMde_Wafer;

// Structure for measured data
typedef struct t_GexMde_Measurements
{
	int					nSites;				// number of sites
	int					nBaseParameters;	// number of base parameters
	int					nRows;				// number of wafers
	int					nCols;				// number of input parameters (base parameters * sites)
	GexMde_Parameter*	pstParameters;		// array of size nCols
	GexMde_Wafer*		pstWafers;			// array of size nRows
	float*				pfDataMatrix;		// matrix of nRows * nCols
} GexMde_Measurements;

// Structure for univariate outputs
typedef struct t_GexMde_UnivariateOutputs
{
   float rZscore;
   float rReferenceMean;
   float rReferenceStdDev;
   float rOOS;
} GexMde_UnivariateOutputs;

// Types to access plugin functions
///////////////////////////////////////////////////////////
// MDE_Init
///////////////////////////////////////////////////////////
// Initialize a MDE function call sequence
//
// Return values:
//		MDE_STATUS_OK	: Function successful
///////////////////////////////////////////////////////////
typedef int (*FP_MDE_Init)(const char* szPluginPath);

///////////////////////////////////////////////////////////
// MDE_Admin
///////////////////////////////////////////////////////////
// Administrate MDE models created with MDE_Train
//
// Return values:
//		MDE_STATUS_OK	: Function successful
///////////////////////////////////////////////////////////
typedef int (*FP_MDE_Admin)(void);

///////////////////////////////////////////////////////////
// MDE_Train
///////////////////////////////////////////////////////////
// Create a model using GexMde_Measurements structure data
// Requirement on pstMeasurements:
//		- nSites		: Must be equal to nCols divide by nBaseParameters
//		- pstParameters	: * Must be a nCols vector
//						  * szParameterName must be sorted by site like:
//							param1_site1, param1_site2, ... , param2_site1, 
//		- pstWafers		: Must be a nRows vector
//		- pfDataMatrix	: Must be a nRows * nCols vector. It contains row first
//
// Return values:
//		MDE_STATUS_OK	: Function successful
///////////////////////////////////////////////////////////
typedef int (*FP_MDE_Train)(const GexMde_Measurements* pstMeasurements);

///////////////////////////////////////////////////////////
// MDE_Apply
///////////////////////////////////////////////////////////
// Apply a model created with MDE_Train
// Requirement:
//		- pstMeasurements		: Same as train, except some szParameterName can
//								  be missed
//		- pfRiskOutput			: nRow vector of float containing risk
//		- pfRiskThreshold		: Pointer to a float that will be filled with 
//								  risk threshold
//		- pstUnivariateOutput	: nRow * nBaseParameters instances of
//								  t_GexMde_UnivariateOutputs, rows first.
//
// Return values:
//		MDE_STATUS_OK	: Function successful
///////////////////////////////////////////////////////////
typedef int (*FP_MDE_Apply)(const GexMde_Measurements* pstMeasurements, float* pfRiskOutput, float* pfRiskThreshold, GexMde_UnivariateOutputs* pstUnivariateOutput, float* pfZScoreThreshold, float* pfOOSThreshold);

///////////////////////////////////////////////////////////
// MDE_Done
///////////////////////////////////////////////////////////
// Close a MDE function call sequence
//
// Return values:
//		MDE_STATUS_OK	: Function successful
///////////////////////////////////////////////////////////
typedef int (*FP_MDE_Done)(void);

#endif // _GEX_MASA_MDE_H_
