// gexpat_plugin_demo.cpp: implementation of the GexPatPlugin_Demo class.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes: defines the entry point for the DLL application.
//
// ----------------------------------------------------------------------------------------------------------

// Standard includes

// Local includes
#include "gexpat_plugin_demo.h"
#include <QtGlobal>


// ----------------------------------------------------------------------------------------------------------
// Only declare a DLL entry point under Windows.
// Under UNIX system, nothing needed for a shared library.
// ----------------------------------------------------------------------------------------------------------
#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#	include <windows.h>

BOOL APIENTRY DllMain( HANDLE /*hModule*/,
                       DWORD  ul_reason_for_call, 
					   LPVOID /*lpReserved*/
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif // defined(_WIN32)

extern "C" GEXPAT_PLUGIN_API GexPatPlugin_Base *gexpat_plugin_getobject(const char *szApplicationPath, const char *szLocalFolder)
{
	GexPatPlugin_Demo *pObject = new GexPatPlugin_Demo(szApplicationPath, szLocalFolder);
	return (GexPatPlugin_Base *)pObject;
}

extern "C" GEXPAT_PLUGIN_API void gexpat_plugin_releaseobject(GexPatPlugin_Base *pObject)
{
	delete pObject;
}

const char *szPluginName = GEXPAT_PLUGIN_NAME;

////////////////////////////////////////////////////////////////////////////////////
// GexPatPlugin_Demo class: PAT plugin demo class
////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexPatPlugin_Demo::GexPatPlugin_Demo(const char *szApplicationPath, const char *szLocalFolder) : 
GexPatPlugin_Base(szPluginName, GEXPAT_PLUGIN_BUILD, szApplicationPath, szLocalFolder)
{
	m_pArrayProcessedDies	= NULL;
	m_pWaferMap				= NULL;
	m_nBadClusterBin		= -1;
	m_nDPatBin				= -1;
	m_nWaferSize			= 0;
}

GexPatPlugin_Demo::~GexPatPlugin_Demo()
{
}

//////////////////////////////////////////////////////////////////////
// Apply PAT to specified wafermap
//////////////////////////////////////////////////////////////////////
int GexPatPlugin_Demo::pat_wafermap(GexPatPlugin_WaferMap *pWaferMap, int nBadClusterBin, int nDPatBin)
{
	if (init(pWaferMap, nBadClusterBin, nDPatBin))
	{
		for(int nIndex=0; nIndex < m_nWaferSize; nIndex++)
		{
			// Check if we have to process this die
			if (processDie(nIndex))
			{
				// Find out bad die around this one
				findOutBadDiesAround(m_lstBadDies.begin());

				// Apply PAT binning
				applyBadClusterBin();
			}
		}

		delete [] m_pArrayProcessedDies;
		m_pArrayProcessedDies = NULL;

		return pat_ok();
	}

	char szErrorMessage[GEXPAT_MAX_STRING];

	sprintf(szErrorMessage, "Invalid wafer map in function pat_wafermap() for this plugin (%s)", m_szPluginName);

	return pat_nok(szErrorMessage);
}

//////////////////////////////////////////////////////////////////////
// Compute PAT limits for specified test
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
int GexPatPlugin_Demo::pat_testlimits(int /*nTestNumber*/,
									  int /*nPinNumber*/,
									  const char* /*szTestName*/,
									  int /*nSiteNumber*/,
									  double* plfTestResults,
									  int nNbSamples,
									  double* plfLowLimit,
									  double* plfHighLimit)
{
	// Check if samples available
	if(nNbSamples <= 1)
	{
		*plfLowLimit = GEXPAT_DOUBLE_INFINITE;
		*plfHighLimit = GEXPAT_DOUBLE_INFINITE;
		return pat_ok();
	}
		
	double	lfMin=plfTestResults[0], lfMax=plfTestResults[0];
	int		nIndex;

	// Go through test results
	for(nIndex = 0; nIndex < nNbSamples; nIndex++)
	{
		lfMin = qMin(lfMin, plfTestResults[nIndex]);
		lfMax = qMax(lfMax, plfTestResults[nIndex]);
	}

	// Check if we have a non zero range
	if(lfMax == lfMin)
	{
		*plfLowLimit = GEXPAT_DOUBLE_INFINITE;
		*plfHighLimit = GEXPAT_DOUBLE_INFINITE;
		return pat_ok();
	}

	// Compute limits: LL = Min+10%(range), HL = Max-10%(range)
	*plfLowLimit = lfMin + (lfMax-lfMin)/10.0;
	*plfHighLimit = lfMax - (lfMax-lfMin)/10.0;

	return pat_ok();
}

bool GexPatPlugin_Demo::init(GexPatPlugin_WaferMap *pWaferMap, int nBadClusterBin, int nDPatBin)
{
	if (pWaferMap)
	{
		m_pWaferMap			= pWaferMap;
		m_nBadClusterBin	= nBadClusterBin;
		m_nDPatBin			= nDPatBin;
		m_nWaferSize		= m_pWaferMap->m_nSizeX * pWaferMap->m_nSizeY;

		m_pArrayProcessedDies = new bool[m_nWaferSize];
		memset(m_pArrayProcessedDies, false, sizeof(bool) * (m_nWaferSize));

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// Process a die
// if the bin of the die is failed, add it into the list of bad die
//////////////////////////////////////////////////////////////////////
bool GexPatPlugin_Demo::processDie(int nIndex)
{
	// add die into list of bad dies
	if (m_pWaferMap->m_pnBinnings[nIndex] >= 0 && m_pWaferMap->m_pnBinnings[nIndex] != 1 && 
		m_pWaferMap->m_pnBinnings[nIndex] != m_nDPatBin && m_pArrayProcessedDies[nIndex] == false)
	{
		m_lstBadDies.push_back(nIndex);
		m_pArrayProcessedDies[nIndex] = true;

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// Looking for bad die around another one
//////////////////////////////////////////////////////////////////////
void GexPatPlugin_Demo::findOutBadDiesAround(std::list<int>::iterator itBadDie)
{
	if (itBadDie != m_lstBadDies.end())
	{
		int nIndex = (*itBadDie);

		std::vector<int>			vecIndexDie = aroundDieCoordinate(nIndex);
		std::vector<int>::iterator	itBegin		= vecIndexDie.begin();
		std::vector<int>::iterator	itEnd		= vecIndexDie.end();

		while (itBegin != itEnd)
		{
			processDie(*itBegin);
			itBegin++;
		}

		itBadDie++;

		findOutBadDiesAround(itBadDie);
	}
}

//////////////////////////////////////////////////////////////////////
// Check if we found enough bad die to apply bad cluster bin
//////////////////////////////////////////////////////////////////////
void GexPatPlugin_Demo::applyBadClusterBin()
{
	// At least GEXPAT_MIN_BAD_DIE 
	if (verifyBadClusteringRules())
	{
		std::list<int>::iterator itDieBegin	= m_lstBadDies.begin();
		std::list<int>::iterator itDieEnd	= m_lstBadDies.end();

		while (itDieBegin != itDieEnd)
		{
			int nIndex = (*itDieBegin);

			std::vector<int>			vecIndexDie = aroundDieCoordinate(nIndex);
			std::vector<int>::iterator	itBegin		= vecIndexDie.begin();
			std::vector<int>::iterator	itEnd		= vecIndexDie.end();

			while (itBegin != itEnd)
			{
				setBadClusterBin(*itBegin);
				itBegin++;
			}
		
			itDieBegin++;
		}
	}

	m_lstBadDies.clear();
}

//////////////////////////////////////////////////////////////////////
// Set bad cluster bin on each die around the zone
//////////////////////////////////////////////////////////////////////
void GexPatPlugin_Demo::setBadClusterBin(int nIndex)
{
	// add die into list of bad dies
	if (m_pWaferMap->m_pnBinnings[nIndex] == 1)
	{
		m_pWaferMap->m_pnBinnings[nIndex]	= m_nBadClusterBin;
		m_pArrayProcessedDies[nIndex]		= true;
	}
}

//////////////////////////////////////////////////////////////////////
// Get valid coordinate around a die
//////////////////////////////////////////////////////////////////////
std::vector<int> GexPatPlugin_Demo::aroundDieCoordinate(int nIndex)
{
	std::vector<int>	vecCoordinate;
	int					nDieIndex;
	int					nPos = nIndex % m_pWaferMap->m_nSizeX;

	// Not the first on a line
	if (nPos > 0)
	{
		// up left die
		nDieIndex = nIndex - 1 - m_pWaferMap->m_nSizeX;
		if (nDieIndex >= 0 )
			vecCoordinate.push_back(nDieIndex);

		// left die
		nDieIndex = nIndex - 1;
		vecCoordinate.push_back(nDieIndex);

		// down left die
		nDieIndex = nIndex - 1 + m_pWaferMap->m_nSizeX;
		if (nDieIndex < m_nWaferSize)
			vecCoordinate.push_back(nDieIndex);
	}
	
	// up die
	nDieIndex = nIndex - m_pWaferMap->m_nSizeX;
	if (nDieIndex >= 0)
		vecCoordinate.push_back(nDieIndex);

	// down die
	nDieIndex = nIndex + m_pWaferMap->m_nSizeX;
	if (nDieIndex < m_nWaferSize)
		vecCoordinate.push_back(nDieIndex);

	// not the last on a line
	if (nPos < m_pWaferMap->m_nSizeX - 1)
	{
		// up right die
		nDieIndex = nIndex + 1 - m_pWaferMap->m_nSizeX;
		if (nDieIndex >= 0 )
			vecCoordinate.push_back(nDieIndex);

		// right die
		nDieIndex = nIndex + 1;
		vecCoordinate.push_back(nDieIndex);

		// down right die
		nDieIndex = nIndex + 1 + m_pWaferMap->m_nSizeX;
		if (nDieIndex < m_nWaferSize)
			vecCoordinate.push_back(nDieIndex);
	}

	return vecCoordinate;	
}

//////////////////////////////////////////////////////////////////////
// Verify that the die list verify all rules
//////////////////////////////////////////////////////////////////////
bool GexPatPlugin_Demo::verifyBadClusteringRules()
{
	bool bBadClustering = false;

	if (m_lstBadDies.size() >= GEXPAT_MIN_BAD_DIE)
	{
		int nMinX	= m_nWaferSize;
		int nMaxX	= 0;
		int nMinY	= m_nWaferSize;
		int nMaxY	= 0;
		int nCoordX = -1;
		int nCoordY = -1;

		std::list<int>::iterator itDieBegin	= m_lstBadDies.begin();
		std::list<int>::iterator itDieEnd	= m_lstBadDies.end();

		while (itDieBegin != itDieEnd)
		{
			nCoordX = (*itDieBegin) % m_pWaferMap->m_nSizeX;
			nCoordY = (*itDieBegin) / m_pWaferMap->m_nSizeX;

			nMinX = qMin(nMinX, nCoordX);
			nMaxX = qMax(nMaxX, nCoordX);

			nMinY = qMin(nMinY, nCoordY);
			nMaxY = qMax(nMaxY, nCoordY);

			itDieBegin++;
		}

		if ( (nMaxY - nMinY) != 0 && (nMaxX - nMinX) != 0)
			bBadClustering = true;
	}

	return bBadClustering;
}	
