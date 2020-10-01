// gsafefile.h: interface for the CGSafeFile class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GALAXY_SAFEFILE_HEADER_
#define _GALAXY_SAFEFILE_HEADER_

#include "gstdl_membuffer.h"

#include <gstdl_errormgr.h>

/*
#if defined(_WIN32)
#	if defined(_GALAXY_SAFEFILE_EXPORTS_)
#		define GSAFEFILE_API __declspec(dllexport)
#	elif defined _GALAXY_STDUTILS_DLL_MODULE
#		define GSAFEFILE_API
#	else	// _GALAXY_SAFEFILE_EXPORTS_
#		define GSAFEFILE_API __declspec(dllimport)
#	endif // _GALAXY_SAFEFILE_EXPORTS_
#else
#	define GSAFEFILE_API
#endif // _MSC_VER
*/

#define GSAFEFILE_API

//////////////////////////////////////////////////////////////////////
// CGSafeFile class

class GSAFEFILE_API CGSafeFile  
{
// Constructor / Destructor
public:
	CGSafeFile();
	virtual ~CGSafeFile();

// Attributes
public:
	GDECLARE_ERROR_MAP(CGSafeFile)
	{
		eOpenFile,					// Cannot open file
		eWriteFile,					// Cannot write to file
		eReadFile,					// Cannot read from file
		eFileSize,					// The file doesn't have the correct size
		eMalloc						// Memory allocation failure
	}
	GDECLARE_END_ERROR_MAP(CGSafeFile)

protected:	

// Operations
public:
	BOOL		GetFileType(PCSTR szSrcFileName, UINT *pnFileType);
	// Write buffer to file
	BOOL		WriteBinary(const CGMemBuffer &cmbSrcBuffer, PCSTR szDestFileName, UINT nFileType);
	BOOL		WriteText(const CGMemBuffer &cmbSrcBuffer, PCSTR szDestFileName);
	// Read buffer from file
	BOOL		ReadBinary(PCSTR szSrcFileName, CGMemBuffer &mbDestBuffer, UINT *pnFileType);
	BOOL		ReadText(PCSTR szSrcFileName, CGMemBuffer &mbDestBuffer);

// Implementation
protected:	
	void		UINTToFromBigEndian(UINT *pnData);
};

#endif // #ifndef _GALAXY_SAFEFILE_HEADER_

