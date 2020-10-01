// ----------------------------------------------------------------------------------------------------------
// gstdl_info.h: GSTDL library information
// ----------------------------------------------------------------------------------------------------------
//
// Purpose: Retrieve GSTDL library information (version, build).
//
// ----------------------------------------------------------------------------------------------------------
//
// Notes:
//
// ----------------------------------------------------------------------------------------------------------

#ifndef _GSTDL_INFO_HEADER_
#define _GSTDL_INFO_HEADER_

#if defined(_WIN32)
#ifdef _MSC_VER
#pragma warning(disable : 4251)
#pragma warning(disable : 4786)
#endif
#endif

// Include STL string header, and...
#include <string>
/*
#if defined(_WIN32)
#	if defined(_GSTDL_INFO_EXPORTS_)
#		define GSTDL_INFO_API __declspec(dllexport)
#	elif !defined(_GSTDL_INFO_DLL_MODULE)
#		define GSTDL_INFO_API
#	else
#		define GSTDL_INFO_API __declspec(dllimport)
#	endif // _GSTDL_INFO_EXPORTS_
#	pragma	warning (disable : 4251) // Due to STL <map> include
#else
#	define GSTDL_INFO_API
#endif*/

#define GSTDL_INFO_API

#define GSTDL_VERSION_X					2
#define GSTDL_VERSION_Y					0
#define GSTDL_BUILD						7

#define GSTDL_NAME						"Quantix Std Libraries"
#define GSTDL_NAME_WITHVERSION			"Quantix Std Libraries V2.0 B7"
#define GSTDL_SHORTNAME					"Quantix Std Libraries"
#define GSTDL_SHORTNAME_WITHVERSION		"Quantix Std Libraries V2.0 B7"

// ----------------------------------------------------------------------------------------------------------
// GSTDL_Info class declaration

class GSTDL_INFO_API GSTDL_Info
{
// Constructor / Destructor
public:
	GSTDL_Info();
	virtual ~GSTDL_Info();
	void GetVersion(unsigned int *puiVersion_X, unsigned int *puiVersion_Y, unsigned int *puiBuild, std::string & strName, std::string & strName_WithVersion, std::string & strShortName, std::string & strShortName_WithVersion);
};

#endif // _GSTDL_INFO_HEADER_
