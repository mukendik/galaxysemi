// gexpat_plugin_freescale.cpp: implementation of the GexPatPlugin_Freescale class.
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
#include "gexpat_plugin_freescale.h"


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
	GexPatPlugin_Freescale *pObject = new GexPatPlugin_Freescale(szApplicationPath, szLocalFolder);
	return (GexPatPlugin_Base *)pObject;
}

extern "C" GEXPAT_PLUGIN_API void gexpat_plugin_releaseobject(GexPatPlugin_Base *pObject)
{
	delete pObject;
}

////////////////////////////////////////////////////////////////////////////////////
// GexPatPlugin_Freescale class: PAT plugin demo class
////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexPatPlugin_Freescale::GexPatPlugin_Freescale(const char *szApplicationPath, const char *szLocalFolder) : 
GexPatPlugin_Base(GEXPAT_PLUGIN_NAME, GEXPAT_PLUGIN_BUILD, szApplicationPath, szLocalFolder)
{
}

GexPatPlugin_Freescale::~GexPatPlugin_Freescale()
{
}

//////////////////////////////////////////////////////////////////////
// Apply PAT to specified wafermap
//////////////////////////////////////////////////////////////////////
int GexPatPlugin_Freescale::pat_wafermap(GexPatPlugin_WaferMap *pWaferMap, int nBadClusterBin, int nDPatBin)
{
	const char* szWaferText[5] = {	"XXX XXX XXX XXX XXX  XX XXX X   XXX",
									"X   X X X   X   X   X   X X X   X  ",
									"XX  XX  XX  XX   X  X   XXX X   XX ",
									"X   X X X   X     X X   X X X   X  ",
									"X   X X XXX XXX XXX  XX X X XXX XXX"
								};
	int	nText_LowX	= (pWaferMap->m_nSizeX/2) - 16;
	int nText_HighX	= (pWaferMap->m_nSizeX/2) + 19;
	int nText_LowY	= (pWaferMap->m_nSizeY/2) - 2;
	int nText_HighY	= pWaferMap->m_nSizeY/2 + 3;

	int nIndex, nX, nY, nXText, nYText;
	for(nIndex=0; nIndex<(pWaferMap->m_nSizeX*pWaferMap->m_nSizeY); nIndex++)
	{
		if((pWaferMap->m_pnBinnings[nIndex] != -1) && (pWaferMap->m_pnBinnings[nIndex] != nDPatBin))
		{
			// Compute die location
			nX = nIndex % pWaferMap->m_nSizeX;
			nY = nIndex / pWaferMap->m_nSizeX;

			// Check Wafer text
			if((nX >= nText_LowX) && (nX < nText_HighX) && (nY < nText_HighY) && (nY >= nText_LowY))
			{
				nXText = nX - nText_LowX;
				nYText = 4 - (nY - nText_LowY);
				if(szWaferText[nYText][nXText] == 'X')
					pWaferMap->m_pnBinnings[nIndex] = nBadClusterBin;
			}
		}
	}

	return pat_ok();
}

//////////////////////////////////////////////////////////////////////
// Compute PAT limits for specified test
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
int GexPatPlugin_Freescale::pat_testlimits(int /*nTestNumber*/,
										   int /*nPinNumber*/,
										   const char* /*szTestName*/,
										   int /*nSiteNumber*/,
										   double* /*plfTestResults*/,
										   int /*nNbSamples*/,
										   double* /*plfLowLimit*/,
										   double* /*plfHighLimit*/)
{
	return pat_ok();
}
