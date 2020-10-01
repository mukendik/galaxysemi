// gfilemap.h : Allows to map a file into memory for Windows/Unix
// ----------------------------------------------------------------------------------------------------------

#ifndef _GALAXY_FILE_MAPPING_HEADER_
#define _GALAXY_FILE_MAPPING_HEADER_

#include <gstdl_errormgr.h>
/*
#if defined(_WIN32)
#	if defined(_GALAXY_FILEMAP_EXPORTS_)
#		define GFILEMAP_API __declspec(dllexport)
#	elif defined(_GALAXY_STDUTILS_DLL_MODULE)	// _GALAXY_ERRORMGR_EXPORTS_
#		define	GFILEMAP_API
#	else
#		define GFILEMAP_API __declspec(dllimport)
#	endif // _GALAXY_ERRORMGR_EXPORTS_
#else
#	define GFILEMAP_API
#endif*/

#define GFILEMAP_API

// ----------------------------------------------------------------------------------------------------------
// class CGFileMap

class GFILEMAP_API CGFileMap
{
// Constructor / Destructor
public:
	CGFileMap();
	virtual ~CGFileMap();

// Attrbiutes
public:
	GDECLARE_ERROR_MAP(CGFileMap)
	{
		eOpenFile,				// Cannot open file
		eMapFile,				// Cannot map file to memory
		eUnmapFile,				// Cannot unmap file to memory
		eFlush,					// Cannot flush the memory to file
		eInvalidMode,			// Invalid mode used
		eGetFileSize			// Cannot retrieve file size
	}
	GDECLARE_END_ERROR_MAP(CGFileMap)

	enum EMapMode	{	eRead,
						eWrite
	};

	PVOID		GetWriteMapData() const;// Retrieve a pointer to write to the mapped file
	PVOID	GetReadMapData() const;	// Retrieve a pointer to read from the mapped file
    unsigned long long		GetFileSize() const;	// Retrieve file size

// Operations
public:
	BOOL	MapFile(PCSTR szFileName, EMapMode eMode = eRead);
	BOOL	Flush();
	BOOL	CloseMap();

// Implementation
private:
	PVOID		m_pMapData;		// Pointer mapped to a file
    unsigned long long		m_lFileSize;	// Size of the opened mapped file
	EMapMode	m_eMapMode;		// The current opened mode

#if defined(_WIN32)
	PVOID	m_hFile;	// Handle to the opened file
	PVOID	m_hMap;		// Handle to the current map
#elif defined __unix__ || __APPLE__&__MACH__
	INT32	m_iFile;	// Handle ro the opened file
#endif
};

#endif // _GALAXY_FILE_MAPPING_HEADER_

