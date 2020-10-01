// gfilemap.cpp : Allows to map a file into memory for Windows/Unix
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes : * Write operations need to be implemented.
//
// ----------------------------------------------------------------------------------------------------------

#define _GALAXY_FILEMAP_EXPORTS_
#include "gstdl_utilsdll.h"
#include "gstdl_filemap.h"
#include "gstdl_utils.h"
#include <gstdl_macro.h>

#include <errno.h>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>


#endif // defined(_WIN32)

using namespace std;


GBEGIN_ERROR_MAP(CGFileMap)
	GMAP_ERROR(eOpenFile,"Cannot open file: %s\n%s")
	GMAP_ERROR(eMapFile,"Cannot map the file: %s to memory.\n%s")
	GMAP_ERROR(eUnmapFile,"Cannot unmap file.\n%s")
	GMAP_ERROR(eFlush,"%s")
	GMAP_ERROR(eInvalidMode,"Using an invalid map mode for this operation");
	GMAP_ERROR(eGetFileSize,"Cannot retrieve the size of file: %s\n%s")
GEND_ERROR_MAP(CGFileMap)

// ----------------------------------------------------------------------------------------------------------
// Constructor / Destructor
// ----------------------------------------------------------------------------------------------------------

CGFileMap::CGFileMap() : m_pMapData(NULL),m_lFileSize(0)
{
#if defined(_WIN32)
	m_hFile = NULL;
	m_hMap = NULL;
#elif defined (__unix__)
	m_iFile = 0;
#endif
}

CGFileMap::~CGFileMap()
{
	CloseMap();
}

// ----------------------------------------------------------------------------------------------------------
// Description: Retrieve a pointer to the block of memory mapped to the opened file for read operations.
// ----------------------------------------------------------------------------------------------------------
PVOID CGFileMap::GetReadMapData() const
{
	if(m_eMapMode != eRead)
	{
		GSET_ERROR(CGFileMap,eInvalidMode);
		return NULL;
	}
	return m_pMapData;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Retrieve a pointer to the block of memory mapped to the opened file for write operations.
// ----------------------------------------------------------------------------------------------------------
PVOID CGFileMap::GetWriteMapData() const
{
	if(m_eMapMode != eWrite)
	{
		GSET_ERROR(CGFileMap,eInvalidMode);
		return NULL;
	}
	return m_pMapData;
}


unsigned long long CGFileMap::GetFileSize() const
{
	return m_lFileSize;
}


// ----------------------------------------------------------------------------------------------------------
// Description: Map the specified file to memory.
//
// Argument(s):
//      LPCSTR szFileName: The name of the file to map.
//      EMapMode eMode /*= eRead*/: The map mode
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGFileMap::MapFile(PCSTR szFileName, EMapMode eMode /*= eRead*/)
{
	ASSERT(eMode == eRead || eMode == eWrite);
	m_eMapMode = eMode;
#if defined(_WIN32)
    GS::CGStdUtils	utils;
	CHAR		szBuffer[256];

	if(eMode == eRead)
	{
		// Open the file in reading mode
		m_hFile = ::CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
							   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(m_hFile == NULL)
		{
			utils.GetSystemErrorMsg(szBuffer,256);
			GSET_ERROR2(CGFileMap,eOpenFile,NULL,szFileName,szBuffer);
			return FALSE;
		}
		// Retrieve its size
		m_lFileSize = ::GetFileSize(m_hFile, NULL);

		// Map the file to memory
		m_hMap = ::CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if(m_hMap == NULL)
		{
			utils.GetSystemErrorMsg(szBuffer,256);
			GSET_ERROR2(CGFileMap,eMapFile,NULL,szFileName,szBuffer);
			return FALSE;
		}
		// Retrieve a pointer to mapped file data
		m_pMapData = ::MapViewOfFile(m_hMap, FILE_MAP_READ, 0, 0, 0);
		if(m_pMapData == NULL)
		{
			utils.GetSystemErrorMsg(szBuffer,256);
			GSET_ERROR2(CGFileMap,eMapFile,NULL,szFileName,szBuffer);
			return FALSE;
		}
	}
	else
	{
		ASSERT(FALSE);
		return FALSE; // Need to be implemented
		// Open the file in reading mode
		m_hFile = ::CreateFile(szFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
							   OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(m_hFile == NULL)
		{
			utils.GetSystemErrorMsg(szBuffer,256);
			GSET_ERROR2(CGFileMap,eOpenFile,NULL,szFileName,szBuffer);
			return FALSE;
		}
		// Retrieve its size
		m_lFileSize = ::GetFileSize(m_hFile, NULL);

		// Map the file to memory
		m_hMap = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 256, NULL);
		if(m_hMap == NULL)
		{
			utils.GetSystemErrorMsg(szBuffer,256);
			GSET_ERROR2(CGFileMap,eMapFile,NULL,szFileName,szBuffer);
			return FALSE;
		}
		// Retrieve a pointer to mapped file data
		m_pMapData = ::MapViewOfFile(m_hMap, FILE_MAP_WRITE, 0, 0, 0);
		if(m_pMapData == NULL)
		{
			utils.GetSystemErrorMsg(szBuffer,256);
			GSET_ERROR2(CGFileMap,eMapFile,NULL,szFileName,szBuffer);
			return FALSE;
		}
	}


#elif defined __unix__ || __APPLE__&__MACH__

	struct stat		attributes;
	// Try to open the file in reading mode
	m_iFile = open(szFileName, O_RDONLY);
	if(m_iFile < 0)
	{
		GSET_ERROR2(CGFileMap,eOpenFile,NULL,szFileName,strerror(errno));
		return FALSE;
	}
	// Retrieve file size
	if(fstat(m_iFile, &attributes) == -1)
	{
		close(m_iFile);
		m_iFile = 0;
		GSET_ERROR2(CGFileMap,eGetFileSize,NULL,szFileName,strerror(errno));
		return FALSE;
	}
	m_lFileSize = attributes.st_size;

    m_pMapData = mmap((caddr_t) 0, m_lFileSize, (PROT_READ | PROT_WRITE),MAP_PRIVATE, m_iFile, 0);
	if(m_pMapData == MAP_FAILED)
	{
		close(m_iFile);
		m_iFile = 0;
		GSET_ERROR2(CGFileMap,eMapFile,NULL,szFileName,strerror(errno));
		return FALSE;
	}

#endif

	return TRUE;
}

BOOL CGFileMap::Flush()
{
#if defined(_WIN32)
	if(!::FlushViewOfFile(m_pMapData,256))
	{
        GS::CGStdUtils	utils;
		CHAR		szBuffer[256];
		utils.GetSystemErrorMsg(szBuffer,256);
		GSET_ERROR1(CGFileMap,eFlush,NULL,szBuffer);
		return FALSE;
	}
#endif // defined(_WIN32)
	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Close the memory mapping and the corresponding file.
//
// ----------------------------------------------------------------------------------------------------------
BOOL CGFileMap::CloseMap()
{
#if defined(_WIN32)
	if(m_pMapData != NULL)
	{
		::UnmapViewOfFile(m_pMapData);
		m_pMapData = NULL;
	}

	if(m_hMap != NULL)
	{
		::CloseHandle(m_hMap);
		m_hMap = NULL;
	}

	if(m_hFile != NULL)
	{
		::CloseHandle(m_hFile);
		m_hFile = NULL;
		m_lFileSize = 0;
	}
	return TRUE;

#elif defined __unix__ || __APPLE__&__MACH__

	if(m_pMapData != NULL)
	{
		int iResult = munmap((char*)m_pMapData, m_lFileSize);
		if(iResult != 0)
		{
			GSET_ERROR1(CGFileMap,eUnmapFile,NULL,strerror(errno));
			return FALSE;
		}
		m_pMapData = NULL;
		m_lFileSize = 0;
	}

	if(m_iFile != 0)
	{
		(void)close(m_iFile);
		m_iFile = 0;
	}
	return TRUE;

#endif
}

