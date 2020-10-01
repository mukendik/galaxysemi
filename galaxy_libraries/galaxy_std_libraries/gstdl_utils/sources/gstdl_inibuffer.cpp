///////////////////////////////////////////////////////////////////////////////////////////
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
//////////////////////////////////////////////////////////////////////////////////////////
// ginibuffer.cpp: implementation of the CGIniBuffer class.
//
//
// Notes :
//
//////////////////////////////////////////////////////////////////////////////////////////

#define _GALAXY_INIBUFFER_EXPORTS_

#include <stdio.h>
#include <stdlib.h>

#include "gstdl_utilsdll.h"
#include "gstdl_inibuffer.h"

#include <gstdl_utils_c.h>
#include <gstdl_md5checksum_c.h>
#include <gstdl_macro.h>

#include <string.h>

using namespace std;

/* --------------------------------------------------------------------------------------- */
/* PRIVATE TYPES AND DEFINES															   */
/* --------------------------------------------------------------------------------------- */
// Error map
GBEGIN_ERROR_MAP(CGIniBuffer)
	GMAP_ERROR(eExternalError,"External error")
	GMAP_ERROR(eMalloc,"Memory allocation failure")
	GMAP_ERROR(eSectionNotFound,"Couldn't find section: %s")
	GMAP_ERROR(eEntryNotFound,"Couldn't find entry: %s")
	GMAP_ERROR(eOverlap,"String overlap reading field from section: %s, entry:%s")
	GMAP_ERROR(eWrongType,"Wrong type reading field from section: %s, entry:%s")
	GMAP_ERROR(eComputeChecksum,"Error computing checksum")
	GMAP_ERROR(eBadChecksum,"Bad checksum")
GEND_ERROR_MAP(CGIniBuffer)

#define INI_SECTION_VALIDATION		"Validation"
#define INI_ENTRY_CHECKSUM			"Checksum"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGIniBuffer::CGIniBuffer()
{
}

CGIniBuffer::~CGIniBuffer()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Reads a string from a buffer in ini format, given it's section and entry
//
// Argument(s) :
//      CGMemBuffer &clBuffer	     : buffer to read from
//      PCSTR szSection              : section to look for
//      PCSTR szEntry                : entry to look for
//      string strField              : reference on string to receive the result.
//      UINT nSectionsToSkip         : nb of sections to skip (useful if some sections are present several times)
//
// Return      : TRUE if function sucessfully read the integer in specified entry, FALSE else
//
// Notes       :
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::ReadString(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry,
							 string &strField, UINT nSectionsToSkip /* = 0 */)
{
	BOOL	bSectionFound = FALSE, bEndOfSection = FALSE;
	BOOL	bReturn = FALSE;
	string	strFullSection;
	PSTR	pBuffer = NULL, pScan1;
	UINT	nBufferSize = clBuffer.GetBufferSize();

	// Check parameters
	ASSERT((szSection != NULL) && (szEntry != NULL));

	// Initialize result
	strField = "";

	// Allocate buffer to read each string
	pBuffer = (PSTR)malloc(nBufferSize*sizeof(CHAR));
	if(pBuffer == NULL)
	{
		GSET_ERROR(CGIniBuffer,eMalloc);
		return FALSE;
	}

	// Compute full section name
	strFullSection = "[";
	strFullSection += szSection;
	strFullSection += "]";

	// Look for specified section
	clBuffer.Rewind();
	clBuffer.SetDataMode(CGMemBuffer::eModeText);
	while((bSectionFound == FALSE) && (clBuffer.ReadString(pBuffer, nBufferSize) == TRUE))
	{
		// Section found ?
        if(!UT_strnicmp(pBuffer, strFullSection.c_str(), strFullSection.length()))
        {
            if(nSectionsToSkip == 0)	bSectionFound = TRUE;
            else						nSectionsToSkip--;
        }
    }
	// Check if section found
	if(bSectionFound == FALSE)
	{
		GSET_ERROR1(CGIniBuffer,eSectionNotFound,NULL,szSection);
		goto cleanup;
	}

	// Section found, look for entry
	while((bEndOfSection == FALSE) && (clBuffer.ReadString(pBuffer, nBufferSize) == TRUE))
	{
		// Check if beginning of a new section
		if(pBuffer[0] == '[')
			bEndOfSection = TRUE;
		else
		{
			// Check for '=' character
			pScan1 = strchr(pBuffer, '=');
			if(pScan1 != NULL)
			{
                if(!UT_strnicmp(pBuffer, szEntry, strlen(szEntry)))
                {
                    // Entry found
                    strField = "";
                    strField += (pScan1 + 1);
                    bReturn = TRUE;
                    goto cleanup;
                }
            }
		}
	}

	// Entry not found
	GSET_ERROR1(CGIniBuffer,eEntryNotFound,NULL,szEntry);

cleanup:
	if(pBuffer != NULL)			free(pBuffer);
	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Reads a string from a buffer in ini format, given it's section and entry
//
// Argument(s) :
//      CGMemBuffer &clBuffer	     : buffer to read from
//      PCSTR szSection              : section to look for
//      PCSTR szEntry                : entry to look for
//      PSTR szField                 : ptr on buffer to receive the result.
//      UINT nFieldSize              : size of the buffer to receive the result
//      UINT nSectionsToSkip         : nb of sections to skip (useful if some sections are present several times)
//
// Return      : nb of bytes read not including the terminating null character, -1 if entry
//               was not found, or result buffer too small
//
// Notes       : if the buffer to receive the result is too small to hold the result, -1 is
//               returned, as if the entry was not found.
//
/////////////////////////////////////////////////////////////////////////////////////////////
INT CGIniBuffer::ReadString(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry,
						    PSTR szField, UINT nFieldSize, UINT nSectionsToSkip /* = 0 */)
{
	string strField;

	// Check parameters
	ASSERT((szSection != NULL) && (szEntry != NULL) && (szField != NULL));

	// Initialize result
	strcpy(szField, "");

	// Read into strField
	if(ReadString(clBuffer, szSection, szEntry, strField, nSectionsToSkip) == FALSE)
		return -1;

	// Check if no Overlap
	if((UINT)strField.length() >= nFieldSize)
	{
		GSET_ERROR2(CGIniBuffer,eOverlap,NULL,szSection,szEntry);
		return -1;
	}

	// Copy result
	strcpy(szField, strField.c_str());
	return (INT)strField.length();
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Reads an int from a buffer in ini format, given it's section and entry
//
// Argument(s) :
//      CGMemBuffer &clBuffer        : buffer to read from
//      PCSTR szSection              : section to look for
//      PCSTR szEntry                : entry to look for
//      INT *piField                 : ptr on int to receive the result.
//      UINT nSectionsToSkip         : nb of sections to skip (useful if some sections are present several times)
//
// Return      : TRUE if function sucessfully read the integer in specified entry, FALSE else
//
// Notes       :
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::ReadInt(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry,
						  INT *piField, UINT nSectionsToSkip /* = 0 */)
{
	string	strField;

	// Check parameters
	ASSERT((szSection != NULL) && (szEntry != NULL) && (piField != NULL));

	// Initialize result
	*piField = 0;

	// Get string for section/Entry
	if(ReadString(clBuffer, szSection, szEntry, strField, nSectionsToSkip) == FALSE)
		return FALSE;

	// Get int in field
	if(sscanf(strField.c_str(), "%d", piField) != 1)
	{
		GSET_ERROR2(CGIniBuffer,eWrongType,NULL,szSection,szEntry);
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Reads a long from a buffer in ini format, given it's section and entry
//
// Argument(s) :
//      CGMemBuffer &clBuffer        : buffer to read from
//      PCSTR szSection              : section to look for
//      PCSTR szEntry                : entry to look for
//      long *plField                : ptr on long to receive the result.
//      UINT nSectionsToSkip         : nb of sections to skip (useful if some sections are present several times)
//
// Return      : TRUE if function sucessfully read the long in specified entry, FALSE else
//
// Notes       :
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::ReadLong(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry,
						   long *plField, UINT nSectionsToSkip /* = 0 */)
{
	string	strField;

	// Check parameters
	ASSERT((szSection != NULL) && (szEntry != NULL) && (plField != NULL));

	// Initialize result
	*plField = 0;

	// Get string for section/Entry
	if(ReadString(clBuffer, szSection, szEntry, strField, nSectionsToSkip) == FALSE)
		return FALSE;

	// Get int in field
	if(sscanf(strField.c_str(), "%ld", plField) != 1)
	{
		GSET_ERROR2(CGIniBuffer,eWrongType,NULL,szSection,szEntry);
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Reads a double from a buffer in ini format, given it's section and entry
//
// Argument(s) :
//      CGMemBuffer &clBuffer        : buffer to read from
//      PCSTR szSection              : section to look for
//      PCSTR szEntry                : entry to look for
//      DOUBLE *pdField              : ptr on a double to receive the result.
//      UINT nSectionsToSkip         : nb of sections to skip (useful if some sections are present several times)
//
// Return      : TRUE if function sucessfully read the integer in specified entry, FALSE else
//
// Notes       :
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::ReadDouble(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry,
							 DOUBLE *pdField, UINT nSectionsToSkip /* = 0 */)
{
	string	strField;

	// Check parameters
	ASSERT((szSection != NULL) && (szEntry != NULL) && (pdField != NULL));

	// Initialize result
	*pdField = 0;

	// Get string for section/Entry
	if(ReadString(clBuffer, szSection, szEntry, strField, nSectionsToSkip) == FALSE)
		return FALSE;

	// Get int in field
	if(sscanf(strField.c_str(), "%lf", pdField) != 1)
	{
		GSET_ERROR2(CGIniBuffer,eWrongType,NULL,szSection,szEntry);
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Writes a string to a buffer in ini format, given it's section and entry
//
// Argument(s) :
//      CGMemBuffer &clBuffer        : buffer to read from
//      PCSTR szSection              : section to look for
//      PCSTR szEntry                : entry to look for
//      const string strField        : string to write
//      UINT nSectionsToSkip         : nb of sections to skip (useful if some sections are present several times)
//
// Return      : TRUE if function sucessfully wrote the field in specified entry, FALSE else
//
// Notes       : if szField is NULL, the entry is deleted
//				 if szEntry is NULL, the whole section is deleted
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::WriteString(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry,
							  const string strField, UINT nSectionsToSkip /* = 0 */)
{
	return WriteString(clBuffer, szSection, szEntry, strField.c_str(), nSectionsToSkip);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Writes a string to a buffer in ini format, given it's section and entry
//
// Argument(s) :
//      CGMemBuffer &clBuffer        : buffer to read from
//      PCSTR szSection              : section to look for
//      PCSTR szEntry                : entry to look for
//      PCSTR szField                : string to write
//      UINT nSectionsToSkip         : nb of sections to skip (useful if some sections are present several times)
//
// Return      : TRUE if function sucessfully wrote the field in specified entry, FALSE else
//
// Notes       : if szField is NULL, the entry is deleted
//				 if szEntry is NULL, the whole section is deleted
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::WriteString(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry,
							  PCSTR szField, UINT nSectionsToSkip /* = 0 */)
{
	BOOL		bReturn = FALSE, bSectionFound = FALSE;
	PSTR		pFullSection = NULL, pFullEntryLine = NULL, pBuffer = NULL;
	UINT		nBufferSize = clBuffer.GetBufferSize(), nPosition, nEntryPosition;
	CGMemBuffer	clNewBuffer;

	// Check parameters
	ASSERT(szSection != NULL);

	// Compute full section name
	pFullSection = (PSTR)malloc((strlen(szSection)+3)*sizeof(CHAR));
	if(pFullSection == NULL)
	{
		GSET_ERROR(CGIniBuffer,eMalloc);
		goto cleanup;
	}
	sprintf(pFullSection, "[%s]", szSection);

	// Compute full entry line
	if((szEntry != NULL) && (szField != NULL))
	{
		pFullEntryLine = (PSTR)malloc((strlen(szEntry)+strlen(szField)+2)*sizeof(CHAR));
		if(pFullEntryLine == NULL)
		{
			GSET_ERROR(CGIniBuffer,eMalloc);
			goto cleanup;
		}
		sprintf(pFullEntryLine, "%s=%s", szEntry, szField);
	}

	// Prepare source buffer
	clBuffer.Rewind();
	clBuffer.SetDataMode(CGMemBuffer::eModeText);

	// Check if buffer empty
	if(nBufferSize == 0)
	{
		if(pFullEntryLine != NULL)
		{
			clBuffer.WriteString(pFullSection);
			clBuffer.WriteString(pFullEntryLine);
			clBuffer.WriteString("");
		}
		bReturn = TRUE;
		goto cleanup;
	}

	// Allocate buffer to read each string
	pBuffer = (PSTR)malloc(nBufferSize*sizeof(CHAR));
	if(pBuffer == NULL)
	{
		GSET_ERROR(CGIniBuffer,eMalloc);
		goto cleanup;
	}

	// Write source buffer to destination buffer until section found
	clNewBuffer.CreateBuffer(nBufferSize > 1024 ? nBufferSize : 1024);
	clNewBuffer.SetDataMode(CGMemBuffer::eModeText);
	while((bSectionFound == FALSE) && (ToNextSection(clBuffer, clNewBuffer, pBuffer, nBufferSize, TRUE) == TRUE))
	{
		if(!UT_strnicmp(pBuffer, pFullSection, strlen(pFullSection)) && (nSectionsToSkip-- == 0))
			bSectionFound = TRUE;
		else
			clNewBuffer.WriteString(pBuffer);
	}

	// Check if section found
	if(bSectionFound == FALSE)
	{
		if(pFullEntryLine != NULL)
		{
			// Add section and entry
			clNewBuffer.WriteString(pFullSection);
			clNewBuffer.WriteString(pFullEntryLine);
			clNewBuffer.WriteString("");
			clBuffer = clNewBuffer;
		}
		bReturn = TRUE;
		goto cleanup;
	}

	// Section found
	if(szEntry == NULL)
	{
		// Delete entire section
		if(ToNextSection(clBuffer, clNewBuffer, pBuffer, nBufferSize, FALSE) == FALSE)
		{
			bReturn = TRUE;
			clBuffer = clNewBuffer;
			goto cleanup;
		}
		clNewBuffer.WriteString(pBuffer);
		goto RemainingBuffer;
	}

	// Add/remove entry
	clNewBuffer.WriteString(pBuffer);
	nPosition = clBuffer.GetCurrentPos();
	if(FindEntry(clBuffer, pBuffer, nBufferSize, szEntry, nEntryPosition) == FALSE)
	{
		if(pFullEntryLine != NULL)
			clNewBuffer.WriteString(pFullEntryLine);
		clBuffer.SetCurrentPos(nPosition);
		goto RemainingBuffer;
	}

	// Entry found: replace or remove it
	clBuffer.SetCurrentPos(nPosition);
	while(nPosition != nEntryPosition)
	{
		clBuffer.ReadString(pBuffer, nBufferSize);
		clNewBuffer.WriteString(pBuffer);
		nPosition = clBuffer.GetCurrentPos();
	}
	clBuffer.ReadString(pBuffer, nBufferSize);
	if(pFullEntryLine != NULL)
		clNewBuffer.WriteString(pFullEntryLine);

RemainingBuffer:
	// Write remaining sections
	while(clBuffer.ReadString(pBuffer, nBufferSize) == TRUE)
		clNewBuffer.WriteString(pBuffer);
	clBuffer = clNewBuffer;
	bReturn = TRUE;

cleanup:
	if(pBuffer != NULL)			free(pBuffer);
	if(pFullSection != NULL)	free(pFullSection);
	if(pFullEntryLine != NULL)	free(pFullEntryLine);
	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Writes an integer to a buffer in ini format, given it's section and entry
//
// Argument(s) :
//      CGMemBuffer &clBuffer        : buffer to read from
//      PCSTR szSection              : section to look for
//      PCSTR szEntry                : entry to look for
//      const INT iField             : integer to write
//      UINT nSectionsToSkip         : nb of sections to skip (useful if some sections are present several times)
//
// Return      : TRUE if function sucessfully wrote the field in specified entry, FALSE else
//
// Notes       : if szField is NULL, the entry is deleted
//				 if szEntry is NULL, the whole section is deleted
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::WriteInt(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry,
						   const INT iField, UINT nSectionsToSkip /* = 0 */)
{
	CHAR	szField[200];

	// Check parameters
	ASSERT(szSection != NULL);

	// Convert integer to string
	sprintf(szField, "%d", iField);

	return WriteString(clBuffer, szSection, szEntry, szField, nSectionsToSkip);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Writes a long to a buffer in ini format, given it's section and entry
//
// Argument(s) :
//      CGMemBuffer &clBuffer        : buffer to read from
//      PCSTR szSection              : section to look for
//      PCSTR szEntry                : entry to look for
//      const long lField            : long to write
//      UINT nSectionsToSkip         : nb of sections to skip (useful if some sections are present several times)
//
// Return      : TRUE if function sucessfully wrote the field in specified entry, FALSE else
//
// Notes       : if szField is NULL, the entry is deleted
//				 if szEntry is NULL, the whole section is deleted
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::WriteLong(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry,
							const long lField, UINT nSectionsToSkip /* = 0 */)
{
	CHAR	szField[200];

	// Check parameters
	ASSERT(szSection != NULL);

	// Convert integer to string
	sprintf(szField, "%ld", lField);

	return WriteString(clBuffer, szSection, szEntry, szField, nSectionsToSkip);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Writes an integer to a buffer in ini format, given it's section and entry
//
// Argument(s) :
//      CGMemBuffer &clBuffer        : buffer to read from
//      PCSTR szSection              : section to look for
//      PCSTR szEntry                : entry to look for
//      const INT iField             : integer to write
//      UINT nSectionsToSkip         : nb of sections to skip (useful if some sections are present several times)
//
// Return      : TRUE if function sucessfully wrote the field in specified entry, FALSE else
//
// Notes       : if szField is NULL, the entry is deleted
//				 if szEntry is NULL, the whole section is deleted
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::WriteDouble(CGMemBuffer &clBuffer, PCSTR szSection, PCSTR szEntry,
							  const DOUBLE dField, UINT nSectionsToSkip /* = 0 */)
{
	CHAR	szField[200];

	// Check parameters
	ASSERT(szSection != NULL);

	// Convert integer to string
	sprintf(szField, "%g", dField);

	return WriteString(clBuffer, szSection, szEntry, szField, nSectionsToSkip);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Add checksum of current .ini file to .ini file
//
// Argument(s) :
//      CGMemBuffer &clBuffer :
//
// Return      : TRUE if checksum was sucessfuly added, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::AddChecksum(CGMemBuffer &clBuffer)
{
	CHAR	szCheckSum[MD5_RESULT_SIZE];
	INT		nResult;

	// Remove validation section in case it exists
	WriteString(clBuffer, INI_SECTION_VALIDATION, NULL, NULL);

	// Compute checksum from buffer
	nResult = GetMD5Buffer((unsigned char *)clBuffer.GetData(), clBuffer.GetBufferSize(), szCheckSum);
	if(nResult != MD5_OK)
	{
		GSET_ERROR(CGIniBuffer,eComputeChecksum);
		return FALSE;
	}

	// Add checksum to buffer
	return WriteString(clBuffer, INI_SECTION_VALIDATION, INI_ENTRY_CHECKSUM, szCheckSum);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Read checksum from buffer, remove it, and check if it is correct
//
// Argument(s) :
//      CGMemBuffer &clBuffer : buffer to read from
//
// Return      : TRUE if checksum exact and removed from buffer, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::CheckAndRemoveChecksum(CGMemBuffer &clBuffer)
{
	CHAR	szCheckSum1[MD5_RESULT_SIZE], szCheckSum2[MD5_RESULT_SIZE];
	INT		nResult;

	// Retrieve checksum from ini file
	if(ReadString(clBuffer, INI_SECTION_VALIDATION, INI_ENTRY_CHECKSUM, szCheckSum1, MD5_RESULT_SIZE) == FALSE)
		return FALSE;

	// Remove checksum from ini file
	if(WriteString(clBuffer, INI_SECTION_VALIDATION, NULL, NULL) == FALSE)
		return FALSE;

	// Compute checksum from buffer
	nResult = GetMD5Buffer((unsigned char *)clBuffer.GetData(), clBuffer.GetBufferSize(), szCheckSum2);
	if(nResult != MD5_OK)
	{
		GSET_ERROR(CGIniBuffer,eComputeChecksum);
		return FALSE;
	}

	// Compare checksum
	if(strcmp(szCheckSum1, szCheckSum2) != 0)
	{
		GSET_ERROR(CGIniBuffer,eBadChecksum);
		return FALSE;
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Description :
//
// Argument(s) :
//      CGMemBuffer clSrcBuffer : source buffer to copy from
//      CGMemBuffer clDestBuffer : destination buffer to copy to
//      PSTR pBuffer : temp buffer to use to read from source buffer
//      UINT nBufferSize : allocated size of pBuffer
//      BOOL bCopyContent : must be TRUE if you want the conent to be copied to the destination buffer
//
// Return      : TRUE if successful; FALSE else
//
// Notes :
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::ToNextSection(CGMemBuffer &clSrcBuffer, CGMemBuffer &clDestBuffer,
								PSTR pBuffer, UINT nBufferSize,
								BOOL bCopyContent)
{
	PSTR	pScan;
	BOOL	bSectionFound = FALSE;

	// Check parameters
	ASSERT(pBuffer != NULL);

	// Copy while section not found (the section itself won't be copied if found)
	while(	(bSectionFound == FALSE) &&
			(clSrcBuffer.ReadString(pBuffer, nBufferSize) == TRUE))
	{
		// Check if section begins here
        for(pScan=pBuffer ; ((*pScan==' ' || *pScan=='\t') && (*pScan!='\0')) ; pScan++) ;
		if(*pScan == '[')
			bSectionFound = TRUE;
		else if(bCopyContent == TRUE)
			clDestBuffer.WriteString(pBuffer);
	}

	return bSectionFound;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description :
//
// Argument(s) :
//      CGMemBuffer clSrcBuffer : source buffer to search
//      PSTR pBuffer : temp buffer to use to read from source buffer
//      UINT nBufferSize : allocated size of pBuffer
//      PCSTR pEntry : entry to look for
//      UINT &nPosition : ptr on variable to store the position of entry
//
// Return      : TRUE if successful; FALSE else
//
// Notes :
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGIniBuffer::FindEntry(CGMemBuffer &clSrcBuffer, PSTR pBuffer, UINT nBufferSize, PCSTR pEntry, UINT &nPosition)
{
	PSTR	pScan;
	BOOL	bSectionFound = FALSE, bEntryFound = FALSE;

	// Check parameters
	ASSERT((pBuffer != NULL) && (pEntry != NULL));

	// Read until entry is found
	nPosition = clSrcBuffer.GetCurrentPos();
	while(	(bSectionFound == FALSE) && (bEntryFound == FALSE) &&
			(clSrcBuffer.ReadString(pBuffer, nBufferSize) == TRUE))
	{
		// Check if section begins here
        for(pScan=pBuffer ; (*pScan==' ' || *pScan=='\t') && (*pScan!='\0') ; pScan++) ;
		if(*pScan == '[')
			bSectionFound = TRUE;
		else
		{
			// Check for '=' character
			pScan = strchr(pBuffer, '=');
			if((pScan != NULL) && !UT_strnicmp(pBuffer, pEntry, strlen(pEntry)))
				// Entry found
				bEntryFound = TRUE;
		}
		if(bEntryFound == FALSE)
			nPosition = clSrcBuffer.GetCurrentPos();
	}

	return bEntryFound;
}
