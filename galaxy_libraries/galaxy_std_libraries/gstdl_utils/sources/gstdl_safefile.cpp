///////////////////////////////////////////////////////////////////////////////////////////
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
//////////////////////////////////////////////////////////////////////////////////////////
// gsafefile.cpp: implementation of the CGSafeFile class.
//
//
// Notes :
//
//////////////////////////////////////////////////////////////////////////////////////////

#define _GALAXY_SAFEFILE_EXPORTS_

#include "gstdl_utilsdll.h"
#include "gstdl_safefile.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#if defined(__unix__)
#include <unistd.h>
#elif defined(_WIN32)
#endif

#include <gstdl_utils_c.h>

using namespace std;

/* --------------------------------------------------------------------------------------- */
/* PRIVATE TYPES AND DEFINES															   */
/* --------------------------------------------------------------------------------------- */
// Error map
GBEGIN_ERROR_MAP(CGSafeFile)
	GMAP_ERROR(eOpenFile,"Couldn't open file: %s (%s)")
	GMAP_ERROR(eWriteFile,"Error writing to file: %s (%s)")
	GMAP_ERROR(eReadFile,"Error reading from file: %s (%s)")
	GMAP_ERROR(eFileSize,"File does not have the correct size or size could not be retrieved: %s")
	GMAP_ERROR(eMalloc,"Memory allocation failure")
GEND_ERROR_MAP(CGSafeFile)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSafeFile::CGSafeFile()
{
}

CGSafeFile::~CGSafeFile()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Writes the source buffer into a destination file (binary mode)
//
// Argument(s) :
//      const CGMemBuffer &cmbSrcBuffer : source buffer to write
//      PCSTR szDestFileName : name of destination file
//
// Return      : TRUE if successful, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGSafeFile::WriteBinary(const CGMemBuffer &cmbSrcBuffer, PCSTR szDestFileName, UINT nFileType)
{
	FILE		*fpDestFile = NULL;
	BOOL		bReturn = TRUE;
	UINT		nSrcBufferSize, nByteCount_BigEndian;
	const char	*pSrcBuffer =  NULL;

	// Open destination file in write mode (binary)
	fpDestFile = fopen(szDestFileName, "wb");
	if(fpDestFile == NULL)
	{	
		GSET_ERROR2(CGSafeFile,eOpenFile,NULL,szDestFileName,strerror(errno));
		return FALSE;
	}

	// Retrieve source buffer and buffer size
	nSrcBufferSize = cmbSrcBuffer.GetBufferSize();
	pSrcBuffer = (char *)cmbSrcBuffer.GetData();

	// First write final count of bytes that will be written to the file,
	// and file type (size + filetype + buffer)
	nByteCount_BigEndian = nSrcBufferSize + sizeof(UINT) + sizeof(UINT);
	UINTToFromBigEndian(&nByteCount_BigEndian);
  if (fwrite(&nByteCount_BigEndian, sizeof(UINT), 1, fpDestFile) != 1)
	{
		GSET_ERROR2(CGSafeFile,eWriteFile,NULL,szDestFileName,strerror(errno));
		bReturn = FALSE;
		goto cleanup;
	}
	UINTToFromBigEndian(&nFileType);
  if (fwrite(&nFileType, sizeof(UINT), 1, fpDestFile) != 1)
	{
		GSET_ERROR2(CGSafeFile,eWriteFile,NULL,szDestFileName,strerror(errno));
		bReturn = FALSE;
		goto cleanup;
	}

	// Write buffer to destination file
  if (fwrite(pSrcBuffer, 1, nSrcBufferSize, fpDestFile) != nSrcBufferSize)
	{
		GSET_ERROR2(CGSafeFile,eWriteFile,NULL,szDestFileName,strerror(errno));
		bReturn = FALSE;
		goto cleanup;
	}

cleanup:
	if(fpDestFile != NULL)	
	{
		fflush(fpDestFile);
		fclose(fpDestFile);
	}
	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Writes the source buffer into a destination file (text mode)
//
// Argument(s) :
//      const CGMemBuffer &cmbSrcBuffer : source buffer to write
//      PCSTR szDestFileName : name of destination file
//
// Return      : TRUE if successful, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGSafeFile::WriteText(const CGMemBuffer &cmbSrcBuffer, PCSTR szDestFileName)
{
	FILE		*fpDestFile = NULL;
	BOOL		bReturn = TRUE;
	UINT		nSrcBufferSize;
	const char	*pSrcBuffer =  NULL;

	// Open destination file in write mode (text)
	fpDestFile = fopen(szDestFileName, "wt");
	if(fpDestFile == NULL)
	{	
		GSET_ERROR2(CGSafeFile,eOpenFile,NULL,szDestFileName,strerror(errno));
		return FALSE;
	}

	// Retrieve source buffer and buffer size
	nSrcBufferSize = cmbSrcBuffer.GetBufferSize();
	pSrcBuffer = (char *)cmbSrcBuffer.GetData();

	// Write buffer to destination file
  if (fwrite(pSrcBuffer, 1, nSrcBufferSize, fpDestFile) != nSrcBufferSize)
	{
		GSET_ERROR2(CGSafeFile,eWriteFile,NULL,szDestFileName,strerror(errno));
		bReturn = FALSE;
		goto cleanup;
	}

cleanup:
	if(fpDestFile != NULL)	
	{
		fflush(fpDestFile);
		fclose(fpDestFile);
	}
	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Reads file type stored in file (binary mode)
//
// Argument(s) :
//      PCSTR szSrcFileName : name of source file
//      UINT *pnFileType : pointer on UINT to receive file type
//
// Return type : BOOL 
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGSafeFile::GetFileType(PCSTR szSrcFileName, UINT *pnFileType)
{
	FILE		*fpSrcFile = NULL;
	BOOL		bReturn = TRUE;
	UINT		nSrcFileSize, nByteCount, nRead;
	char		*pSrcBuffer =  NULL;

	// Get size of source file
	nSrcFileSize = ut_GetFileSize((char *)szSrcFileName);
	if(nSrcFileSize <= 0)
	{
		GSET_ERROR1(CGSafeFile, eFileSize, NULL, szSrcFileName);
		return FALSE;
	}
	
	// Open source file in read mode (binary)
	fpSrcFile = fopen(szSrcFileName, "rb");
	if(fpSrcFile == NULL)
	{	
		GSET_ERROR2(CGSafeFile,eOpenFile,NULL,szSrcFileName,strerror(errno));
		return FALSE;
	}

	// Verify if size is correct
	nRead = (UINT)fread(&nByteCount, sizeof(UINT), 1, fpSrcFile);
	if(ferror(fpSrcFile) != 0)
	{
		GSET_ERROR2(CGSafeFile,eReadFile,NULL,szSrcFileName,strerror(errno));
		bReturn = FALSE;
		goto cleanup;
	}
	if(nRead != 1)
	{
		GSET_ERROR1(CGSafeFile, eFileSize, NULL, szSrcFileName);
		bReturn = FALSE;
		goto cleanup;
	}
	UINTToFromBigEndian(&nByteCount);
	if(nByteCount != nSrcFileSize)
	{
		GSET_ERROR1(CGSafeFile, eFileSize, NULL, szSrcFileName);
		bReturn = FALSE;
		goto cleanup;
	}
	
	// Read FileType
	nRead = (UINT)fread(pnFileType, sizeof(UINT), 1, fpSrcFile);
	if(ferror(fpSrcFile) != 0)
	{
		GSET_ERROR2(CGSafeFile,eReadFile,NULL,szSrcFileName,strerror(errno));
		bReturn = FALSE;
		goto cleanup;
	}
	if(nRead != 1)
	{
		GSET_ERROR1(CGSafeFile, eFileSize, NULL, szSrcFileName);
		bReturn = FALSE;
		goto cleanup;
	}
	UINTToFromBigEndian(pnFileType);

cleanup:
	if(fpSrcFile != NULL)	fclose(fpSrcFile);
	if(pSrcBuffer != NULL)	free(pSrcBuffer);
	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Reads content of the source file into the destination buffer (binary mode)
//
// Argument(s) :
//      PCSTR szSrcFileName : name of source file
//      CGMemBuffer *pmbDestBuffer : ptr to destination buffer
//
// Return type : BOOL 
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGSafeFile::ReadBinary(PCSTR szSrcFileName, CGMemBuffer &mbDestBuffer, UINT *pnFileType)
{
	FILE		*fpSrcFile = NULL;
	BOOL		bReturn = TRUE;
	UINT		nSrcFileSize, nSrcBufferSize, nByteCount, nRead;
	char		*pSrcBuffer =  NULL;

	// Get size of source file
	nSrcFileSize = ut_GetFileSize((char *)szSrcFileName);
	if(nSrcFileSize <= 0)
	{
		GSET_ERROR1(CGSafeFile, eFileSize, NULL, szSrcFileName);
		return FALSE;
	}
	
	// Open source file in read mode (binary)
	fpSrcFile = fopen(szSrcFileName, "rb");
	if(fpSrcFile == NULL)
	{	
		GSET_ERROR2(CGSafeFile,eOpenFile,NULL,szSrcFileName,strerror(errno));
		return FALSE;
	}

	// Verify if size is correct
	nRead = (UINT)fread(&nByteCount, sizeof(UINT), 1, fpSrcFile);
	if(ferror(fpSrcFile) != 0)
	{
		GSET_ERROR2(CGSafeFile,eReadFile,NULL,szSrcFileName,strerror(errno));
		bReturn = FALSE;
		goto cleanup;
	}
	if(nRead != 1)
	{
		GSET_ERROR1(CGSafeFile, eFileSize, NULL, szSrcFileName);
		bReturn = FALSE;
		goto cleanup;
	}
	UINTToFromBigEndian(&nByteCount);
	if(nByteCount != nSrcFileSize)
	{
		GSET_ERROR1(CGSafeFile, eFileSize, NULL, szSrcFileName);
		bReturn = FALSE;
		goto cleanup;
	}
	
	// Read FileType
	nRead = (UINT)fread(pnFileType, sizeof(UINT), 1, fpSrcFile);
	if(ferror(fpSrcFile) != 0)
	{
		GSET_ERROR2(CGSafeFile,eReadFile,NULL,szSrcFileName,strerror(errno));
		bReturn = FALSE;
		goto cleanup;
	}
	if(nRead != 1)
	{
		GSET_ERROR1(CGSafeFile, eFileSize, NULL, szSrcFileName);
		bReturn = FALSE;
		goto cleanup;
	}
	UINTToFromBigEndian(pnFileType);

	// Read source file into source buffer
	nSrcBufferSize = nByteCount - sizeof(UINT) - sizeof(UINT);
	pSrcBuffer = (char *)malloc(nSrcBufferSize*sizeof(char));
	if(pSrcBuffer == NULL)
	{
		GSET_ERROR(CGSafeFile,eMalloc);
		bReturn = FALSE;
		goto cleanup;
	}
	nRead = (UINT)fread((char *)pSrcBuffer, 1, nSrcBufferSize, fpSrcFile);
	if((ferror(fpSrcFile) != 0) || (nRead != nSrcBufferSize))
	{
		GSET_ERROR2(CGSafeFile,eReadFile,NULL,szSrcFileName,strerror(errno));
		bReturn = FALSE;
		goto cleanup;
	}
	
	// Create final buffer
	mbDestBuffer.Close();
	mbDestBuffer.CopyIn((unsigned char *)pSrcBuffer, nSrcBufferSize, nSrcBufferSize);

cleanup:
	if(fpSrcFile != NULL)	fclose(fpSrcFile);
	if(pSrcBuffer != NULL)	free(pSrcBuffer);
	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Reads content of the source file into the destination buffer (text mode)
//
// Argument(s) :
//      PCSTR szSrcFileName : name of source file
//      CGMemBuffer *pmbDestBuffer : ptr to destination buffer
//
// Return type : BOOL 
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGSafeFile::ReadText(PCSTR szSrcFileName, CGMemBuffer &mbDestBuffer)
{
	FILE		*fpSrcFile = NULL;
	BOOL		bReturn = TRUE;
	UINT		nSrcFileSize;
	char		*pSrcBuffer =  NULL;

	// Get size of source file
	nSrcFileSize = ut_GetFileSize((char *)szSrcFileName);
	if(nSrcFileSize <= 0)
	{
		GSET_ERROR1(CGSafeFile, eFileSize, NULL, szSrcFileName);
		return FALSE;
	}
	
	// Open source file in read mode (text)
	fpSrcFile = fopen(szSrcFileName, "rt");
	if(fpSrcFile == NULL)
	{	
		GSET_ERROR2(CGSafeFile,eOpenFile,NULL,szSrcFileName,strerror(errno));
		return FALSE;
	}

	// Read source file into source buffer
	pSrcBuffer = (char *)malloc(nSrcFileSize*sizeof(char));
	if(pSrcBuffer == NULL)
	{
		GSET_ERROR(CGSafeFile,eMalloc);
		bReturn = FALSE;
		goto cleanup;
	}
	nSrcFileSize = fread((char *)pSrcBuffer, 1, nSrcFileSize, fpSrcFile);
	if(ferror(fpSrcFile) != 0)
	{
		GSET_ERROR2(CGSafeFile,eReadFile,NULL,szSrcFileName,strerror(errno));
		bReturn = FALSE;
		goto cleanup;
	}
	
	// Create final buffer
	mbDestBuffer.Close();
	mbDestBuffer.CopyIn((unsigned char *)pSrcBuffer, nSrcFileSize, nSrcFileSize);

cleanup:
	if(fpSrcFile != NULL)	fclose(fpSrcFile);
	if(pSrcBuffer != NULL)	free(pSrcBuffer);
	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : convert an UINT to big endian format
//
// Argument(s) :
//      pnData : ptr on UINT to be converted
//
// Return      : none
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __unix__
void CGSafeFile::UINTToFromBigEndian(UINT *pnData)
{
	BYTE	bLowByte0,bLowByte1,bHighByte0,bHighByte1;
	BYTE	*pbData = (BYTE*)pnData;
	
	bLowByte0  = *(pbData+0);
	bLowByte1  = *(pbData+1);
	bHighByte0 = *(pbData+2);
	bHighByte1 = *(pbData+3);

	*(pbData+3) = bLowByte0;
	*(pbData+2) = bLowByte1;
	*(pbData+1) = bHighByte0;
	*(pbData+0) = bHighByte1;

	return;
}
#else
void CGSafeFile::UINTToFromBigEndian(UINT* )
{
}
#endif
