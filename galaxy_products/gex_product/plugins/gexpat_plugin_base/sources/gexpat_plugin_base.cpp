// gexpat_plugin_base.cpp: implementation of the GexPatPlugin_Base class.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes:
//
// ----------------------------------------------------------------------------------------------------------

// Standard includes

// Local includes
#include "gexpat_plugin_base.h"


///////////////////////////////////////////////////////////
// GexPatPlugin_Base class: PAT plugin base class...
///////////////////////////////////////////////////////////
GexPatPlugin_Base::GexPatPlugin_Base(const char *szPluginName, unsigned int uiPluginBuild, const char *szApplicationPath, const char *szLocalFolder)
{
	// Init some variables
	m_szPluginName		= szPluginName;
	m_uiPluginBuild		= uiPluginBuild;
	m_szApplicationPath = szApplicationPath;
	m_szLocalFolder		= szLocalFolder;
}

GexPatPlugin_Base::~GexPatPlugin_Base()
{
}

//////////////////////////////////////////////////////////////////////
// Return last error message
//////////////////////////////////////////////////////////////////////
const char *GexPatPlugin_Base::GetLastErrorMessage(void)
{
	return m_szLastErrorMessage;
}

//////////////////////////////////////////////////////////////////////
// Apply PAT to specified wafermap
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
int GexPatPlugin_Base::pat_wafermap(GexPatPlugin_WaferMap* /*pWaferMap*/,
									int /*nBadClusterBin*/,
									int /*nDPatBin*/)
{
	char szErrorMessage[GEXPAT_MAX_STRING];

	sprintf(szErrorMessage, "Function pat_wafermap() is not supported in this plugin (%s)", m_szPluginName);

	return pat_nok(szErrorMessage);
}

//////////////////////////////////////////////////////////////////////
// Compute PAT limits for specified test
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
int GexPatPlugin_Base::pat_testlimits(int /*nTestNumber*/,
									  int /*nPinNumber*/,
									  const char* /*szTestName*/,
									  int /*nSiteNumber*/,
									  double* /*plfTestResults*/,
									  int /*nNbSamples*/,
									  double* /*plfLowLimit*/,
									  double* /*plfHighLimit*/)
{
	char szErrorMessage[GEXPAT_MAX_STRING];

	sprintf(szErrorMessage, "Function pat_testlimits() is not supported in this plugin (%s)", m_szPluginName);

	return pat_nok(szErrorMessage);
}

//////////////////////////////////////////////////////////////////////
// Return ok status and clear error message
//////////////////////////////////////////////////////////////////////
int GexPatPlugin_Base::pat_ok(void)
{
	strcpy(m_szLastErrorMessage, "");
	return eNoError;
}

//////////////////////////////////////////////////////////////////////
// Return nok status and set error message
//////////////////////////////////////////////////////////////////////
int GexPatPlugin_Base::pat_nok(char * szErrorMessage)
{
	strncpy(m_szLastErrorMessage, szErrorMessage, GEXPAT_MAX_STRING);
	m_szLastErrorMessage[GEXPAT_MAX_STRING] = 0;

	return ePluginError;
}

