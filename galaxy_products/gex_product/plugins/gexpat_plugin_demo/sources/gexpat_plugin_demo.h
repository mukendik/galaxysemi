// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

#ifndef _GEXPAT_PLUGIN_DEMO_
#define _GEXPAT_PLUGIN_DEMO_

// Standard includes
#include <list>
#include <vector>

// Local includes
#include "gexpat_plugin_base.h"

#define GEXPAT_PLUGIN_NAME			"Galaxy custom PAT demo [Plugin V1.0 B1]"
#define GEXPAT_PLUGIN_BUILD			1
#define GEXPAT_MIN_BAD_DIE			4

///////////////////////////////////////////////////////////
// GexPatPlugin_Demo class: PAT plugin class for 
// This class is exported from the gexpat_plugin_demo.dll
///////////////////////////////////////////////////////////
class GexPatPlugin_Demo : public GexPatPlugin_Base
{
// Constructor / Destructor
public:
	GexPatPlugin_Demo(const char *szApplicationPath, const char *szLocalFolder);
	virtual ~GexPatPlugin_Demo();

// Member functions
public:
	// Common functions

	// PAT functions
	int		pat_wafermap(GexPatPlugin_WaferMap *pWaferMap, int nBadClusterBin, int nDPatBin);		// Apply PAT to specified wafermap
	int		pat_testlimits(int nTestNumber, int nPinNumber, const char *szTestName, int nSiteNumber, double *plfTestResults, int nNbSamples, double *plfLowLimit, double *plfHighLimit);	// Compute PAT limits for specified test

// Member variables
public:

// Implementation
protected:
	// Variables
	std::list<int>			m_lstBadDies;
	bool *					m_pArrayProcessedDies;

	GexPatPlugin_WaferMap *	m_pWaferMap;
	int						m_nBadClusterBin;
	int						m_nDPatBin;
	int						m_nWaferSize;
	
	// Functions
	bool				init(GexPatPlugin_WaferMap *pWaferMap, int nBadClusterBin, int nDPatBin);
	bool				processDie(int nIndex);
	void				findOutBadDiesAround(std::list<int>::iterator itBadDie);
	void				applyBadClusterBin();
	void				setBadClusterBin(int nIndex);
	std::vector<int>	aroundDieCoordinate(int nIndex);
	bool				verifyBadClusteringRules();
};

extern "C" GEXPAT_PLUGIN_API GexPatPlugin_Base *gexpat_plugin_getobject(const char *szApplicationPath, const char *szLocalFolder);
extern "C" GEXPAT_PLUGIN_API void gexpat_plugin_releaseobject(GexPatPlugin_Base *pObject);

#endif // _GEXPAT_PLUGIN_DEMO_
