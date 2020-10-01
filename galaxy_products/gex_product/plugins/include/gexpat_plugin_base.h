// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

#ifndef _GEXPAT_PLUGIN_BASE_HEADER_
#define _GEXPAT_PLUGIN_BASE_HEADER_

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Local includes

// The following ifdef block is the standard way of creating macros which make 
// exporting  from a DLL simpler. All files within a plugin DLL are compiled 
// without the GEXPAT_PLUGIN_IMPORTS symbol defined on the command line. 
// This symbol should be defined on any project that uses this DLL. 
// This way any other project whose source files include this file see 
// GEXPAT_PLUGIN_API functions as being imported from a DLL, wheras the plugin 
// DLL sees symbols defined with this macro as being exported.
#if defined(WIN32)
#	ifdef GEXPAT_PLUGIN_IMPORTS
#		define GEXPAT_PLUGIN_API __declspec(dllimport)
#	else
#		define GEXPAT_PLUGIN_API __declspec(dllexport)
#	endif
#else
#	define GEXPAT_PLUGIN_API
#endif

///////////////////////////////////////////////////////////
// Defines...
///////////////////////////////////////////////////////////
// Name of factory function (to get ptr on plugin object)
#define GEXPAT_PLUGIN_GETOBJECT_NAME		"gexpat_plugin_getobject"
#define GEXPAT_PLUGIN_RELEASEOBJECT_NAME	"gexpat_plugin_releaseobject"

// Global constants
#define GEXPAT_MAX_STRING			1024
#define	GEXPAT_DOUBLE_INFINITE		1e308
#define	GEXPAT_FLOAT_INFINITE		1e38F

class GexPatPlugin_Base;

typedef GexPatPlugin_Base* (*FP_gexpat_plugin_getobject)(const char *szApplicationPath, const char *szLocalFolder);
typedef void (*FP_gexpat_plugin_releaseobject)(GexPatPlugin_Base *pObject);

///////////////////////////////////////////////////////////
// GexPatPlugin_WaferMap class: PAT plugin WaferMap class...
///////////////////////////////////////////////////////////
class GexPatPlugin_WaferMap
{
public:
	// Constructor / Destructor
	GexPatPlugin_WaferMap(int nSizeX, int nSizeY)
	{
		m_pnBinnings = new int[nSizeX*nSizeY];
	}
	~GexPatPlugin_WaferMap()
	{
		delete [] m_pnBinnings;
	}
	// Wafermap corners
	int		m_nLowDieX;
	int		m_nHighDieX;
	int		m_nLowDieY;
	int		m_nHighDieY;
	// Wafermap size (cells in X, and Y)
	int		m_nSizeX;
	int		m_nSizeY;
	// Pointer to wafermap BIN results array
	int		*m_pnBinnings;
};

///////////////////////////////////////////////////////////
// GexPatPlugin_Base class: PAT plugin base class...
///////////////////////////////////////////////////////////
class GexPatPlugin_Base
{
// Constructor / Destructor
public:
	enum ErrorCodes
	{
		// General
		eNoError,							// No error
		ePluginError						// Error in the plugin function
	};

	GexPatPlugin_Base(const char *szPluginName, unsigned int uiPluginBuild, const char *szApplicationPath, const char *szLocalFolder);
	virtual ~GexPatPlugin_Base();

// Member functions
public:
	// Common functions
	const char				*GetPluginName(void) { return m_szPluginName; }			// Returns plugin name
	unsigned int			GetPluginBuild(void) { return m_uiPluginBuild; }		// Returns plugin build
	const char				*GetLastErrorMessage(void);								// Returns last error message

	// PAT functions
	virtual int				pat_wafermap(GexPatPlugin_WaferMap *pWaferMap, int nBadClusterBin, int nDPatBin);	// Apply PAT to specified wafermap
	virtual int				pat_testlimits(int nTestNumber, int nPinNumber, const char *szTestName, int nSiteNumber, double *plfTestResults, int nNbSamples, double *plfLowLimit, double *plfHighLimit);	// Compute PAT limits for specified test

// Member variables
public:

// Implementation
protected:
	// Variables
	const char				*m_szPluginName;										// Plugin name
	unsigned int			m_uiPluginBuild;										// Plugin build
	const char				*m_szApplicationPath;									// Application path
	const char				*m_szLocalFolder;										// Local folder (user directory)
	char					m_szLastErrorMessage[GEXPAT_MAX_STRING];				// Last error message

	// Functions
	int						pat_ok(void);
	int						pat_nok(char * szErrorMessage);
};

#endif // _GEXPAT_PLUGIN_BASE_HEADER_
