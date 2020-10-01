// ginibuffer.h: interface for the CGCrypto class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GALAXY_INIBUFFER_HEADER_
#define _GALAXY_INIBUFFER_HEADER_

#include <string>

#include "gstdl_membuffer.h"

#include <gstdl_errormgr.h>
/*
#if defined(_WIN32)
#	if defined(_GALAXY_INIBUFFER_EXPORTS_)
#		define GINIBUFFER_API __declspec(dllexport)
#	elif defined _GALAXY_STDUTILS_DLL_MODULE
#		define GINIBUFFER_API
#	else	// _GALAXY_INIBUFFER_EXPORTS_
#		define GINIBUFFER_API __declspec(dllimport)
#	endif // _GALAXY_INIBUFFER_EXPORTS_
#else
#	define GINIBUFFER_API
#endif
*/

#define GINIBUFFER_API

//////////////////////////////////////////////////////////////////////
// CGIniBuffer class

class GINIBUFFER_API CGIniBuffer  
{
// Constructor / Destructor
public:
	CGIniBuffer();
	virtual ~CGIniBuffer();

// Attributes
public:
	GDECLARE_ERROR_MAP(CGIniBuffer)
	{
		eExternalError,			// Error coming from other classes used in objects derived from this
		eMalloc,				// Memory allocation failure
		eSectionNotFound,		// Section mot found
		eEntryNotFound,			// Entry not found
		eOverlap,				// String overlap reading field
		eWrongType,				// Wrong type in field
		eComputeChecksum,		// Error computing checksum
		eBadChecksum			// Bad checksum
	}
	GDECLARE_END_ERROR_MAP(CGIniBuffer)

protected:	

// Operations
public:
	// Read ini fields
	INT		ReadString(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry, PSTR szField, UINT nFieldSize, UINT nSectionsToSkip = 0);
	BOOL	ReadString(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry, std::string &strField, UINT nSectionsToSkip = 0);
	BOOL	ReadInt(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry, INT *piField, UINT nSectionsToSkip = 0);
	BOOL	ReadDouble	(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry, DOUBLE *pdField, UINT nSectionsToSkip = 0);
	BOOL	ReadLong(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry, long *plField, UINT nSectionsToSkip = 0);

	// Write ini fields
	BOOL	WriteString(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry, PCSTR szField, UINT nSectionsToSkip = 0);
	BOOL	WriteString(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry, const std::string strField, UINT nSectionsToSkip = 0);
	BOOL	WriteInt(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry, const INT iField, UINT nSectionsToSkip = 0);
	BOOL	WriteDouble(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry, const DOUBLE dField, UINT nSectionsToSkip = 0);
	BOOL	WriteLong(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry, const long lField, UINT nSectionsToSkip = 0);
	BOOL	AddChecksum(CGMemBuffer &clBuffer);
	BOOL	CheckAndRemoveChecksum(CGMemBuffer &clBuffer);

// Implementation
protected:	
	BOOL	ToNextSection(CGMemBuffer &clSrcBuffer, CGMemBuffer &clDestBuffer, PSTR pBuffer, UINT nBufferSize, BOOL bCopyContent);
	BOOL	FindEntry(CGMemBuffer &clSrcBuffer, PSTR pBuffer, UINT nBufferSize, PCSTR pEntry, UINT &nPosition);
};

#endif // #ifndef _GALAXY_INIBUFFER_HEADER_

