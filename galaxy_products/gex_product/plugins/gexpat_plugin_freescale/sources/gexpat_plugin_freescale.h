// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

#ifndef _GEXPAT_PLUGIN_FREESCALE_
#define _GEXPAT_PLUGIN_FREESCALE_

// Standard includes

// Local includes
#include "gexpat_plugin_base.h"

#define GEXPAT_PLUGIN_NAME			"Freescale custom PAT demo [Plugin V1.0 B1]"
#define GEXPAT_PLUGIN_BUILD			1

///////////////////////////////////////////////////////////
// GexPatPlugin_Freescale class: PAT plugin class for 
// This class is exported from the gexpat_plugin_freescale.dll
///////////////////////////////////////////////////////////
class GexPatPlugin_Freescale : public GexPatPlugin_Base
{
// Constructor / Destructor
public:
	GexPatPlugin_Freescale(const char *szApplicationPath, const char *szLocalFolder);
	virtual ~GexPatPlugin_Freescale();

// Member functions
public:
	// Common functions

	// PAT functions
	int		pat_wafermap(GexPatPlugin_WaferMap *pWaferMap, int nBadClusterBin, int nDPatBin);	// Apply PAT to specified wafermap
	int		pat_testlimits(int nTestNumber, int nPinNumber, const char *szTestName, int nSiteNumber, double *plfTestResults, int nNbSamples, double *plfLowLimit, double *plfHighLimit);	// Compute PAT limits for specified test

// Member variables
public:

// Implementation
protected:
	// Variables

	// Functions
};

extern "C" GEXPAT_PLUGIN_API GexPatPlugin_Base *gexpat_plugin_getobject(const char *szApplicationPath, const char *szLocalFolder);
extern "C" GEXPAT_PLUGIN_API void gexpat_plugin_releaseobject(GexPatPlugin_Base *pObject);

#endif // _GEXPAT_PLUGIN_FREESCALE_
