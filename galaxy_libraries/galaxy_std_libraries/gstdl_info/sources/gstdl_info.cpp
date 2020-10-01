// gstdl_info.cpp : Implementation file
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

#define _GSTDL_INFO_EXPORTS_

#include "gstdl_infodll.h"
#include "gstdl_info.h"

using namespace std;

// ----------------------------------------------------------------------------------------------------------
// Class GSTDL_Info

GSTDL_Info::GSTDL_Info()
{
}

GSTDL_Info::~GSTDL_Info()
{

}

void GSTDL_Info::GetVersion(unsigned int *puiVersion_X, unsigned int *puiVersion_Y, unsigned int *puiBuild, string & strName, string & strName_WithVersion,	string & strShortName, string & strShortName_WithVersion)
{
	*puiVersion_X				= GSTDL_VERSION_X;
	*puiVersion_Y				= GSTDL_VERSION_Y;
	*puiBuild					= GSTDL_BUILD;

	strName						= GSTDL_NAME;
	strName_WithVersion			= GSTDL_NAME_WITHVERSION;
	strShortName				= GSTDL_SHORTNAME;
	strShortName_WithVersion	= GSTDL_SHORTNAME_WITHVERSION;
}
