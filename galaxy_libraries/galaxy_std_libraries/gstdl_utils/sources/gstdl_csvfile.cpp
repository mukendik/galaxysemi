// gcsvfile.cpp: implementation of the CGCsvFile class.
//
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes :
//
// ----------------------------------------------------------------------------------------------------------

#include "gs_types.h"
#define _GALAXY_CSV_FILE_EXPORTS_
#include "gstdl_utilsdll.h"
#include "gstdl_filemap.h"
#include "gstdl_csvfile.h"

#include <gstdl_macro.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

using namespace std;

// ----------------------------------------------------------------------------------------------------------
// class CGCsvLine
// Define what is a line in a .csv file.


// Constructors / Destructor
CGCsvLine::CGCsvLine() : m_iLineNumber(-1)
{
}

CGCsvLine::CGCsvLine(const CGCsvLine& source)
  :CGCsvFieldList()
{
	operator=(source);
}

CGCsvLine::CGCsvLine(const string& strLine, CHAR cSeparator /* = GCSV_DEFAULT_SEPARATOR */) :
	m_iLineNumber(-1)
{
	Set(strLine.c_str(), cSeparator);
}

CGCsvLine::CGCsvLine(PCSTR szLine, CHAR cSeparator /* = GCSV_DEFAULT_SEPARATOR */) :
		m_iLineNumber(-1)
{
	Set(szLine, cSeparator);
}

CGCsvLine::~CGCsvLine()
{
	Clear();
}

// Retrieve the corresponding file line number, if this line
// object has been read from a .csv file.
INT	CGCsvLine::GetLineNumber() const
{
	return m_iLineNumber;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Initialize the object with the specified string. This string must be
//				in .csv line format. Each field are separated by the specified character.
//
// Arguments:
//		strLine: The string used to initialize the object.
//		cSeparator: The character used to delimite each field.
//
// ----------------------------------------------------------------------------------------------------------
void CGCsvLine::Set(const string& strLine, CHAR cSeparator /* = GCSV_DEFAULT_SEPARATOR */)
{
	Set(strLine.c_str(),cSeparator);
}

// Same function than above, with a PCSTR argument
void CGCsvLine::Set(PCSTR szLine, CHAR cSeparator /* = GCSV_DEFAULT_SEPARATOR */)
{
	ASSERT(szLine != NULL);
	Clear(); // The line must be empty before to initialize it

	AppendFields(szLine,cSeparator);
}

// ----------------------------------------------------------------------------------------------------------
// Description: Append the specified list of fields to the current line.
//
// Arguments:
//		szFields: The list of fields (each field is delimited by the specified character).
//		cSeparator: The character used to delimite each field.
//
// ----------------------------------------------------------------------------------------------------------
void CGCsvLine::AppendFields(PCSTR szFields, CHAR cSeparator /*= GCSV_DEFAULT_SEPARATOR*/)
{
	ASSERT(szFields != NULL);

	if(strlen(szFields) == 0)
		return;	// Nothing to append

	// Don't use strtok, because strtok skip 2 consecutive ',' character.
	// For example, if we have szFields = "field1,field2,,field4", we will see
	// only 3 fields, and we will miss the empty one.

	// Considere a field can not be longer that the list of fields itself!
	// (In most case, it will used more memory that needed, but it is completly safe)
	PSTR	szToken = new CHAR[strlen(szFields)+1];
	PCSTR	pszFields = szFields;
	PSTR	pszToken = szToken;

	while(*pszFields != '\0')
	{
		if(*pszFields == cSeparator)
		{
			*pszToken = '\0';
			push_back(string(szToken));
			pszToken = szToken;
			pszFields++; // Skip the separator character
		}
		else
			*pszToken++ = *pszFields++;
	}

	// Last field
	*pszToken = '\0';
	push_back(string(szToken));

	delete [] szToken;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Replace the current field, with the specified index, by the specified value.
//
// Arguments:
//		iFieldIndex: The index of the field to replace
//		szNewFieldValue: The new field value
//
// Return:
//	TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvLine::ReplaceField(INT iFieldIndex, PCSTR szNewFieldValue)
{
	INT			iCurIndex = 0;
	iterator	itField = begin();

	while(itField != end())
	{
		if(iCurIndex == iFieldIndex)
		{
			// Remove old field value, and get an iterator to the next field
			iterator itNextField = erase(itField);
			string strNewField(szNewFieldValue);
			// Insert the new field before itNextField
			(void)insert(itNextField,strNewField);
			return TRUE;
		}
		itField++;
		iCurIndex++;
	}

	return FALSE; // The specified field index does not exist
}

// Copy the specified line content to this line.
CGCsvLine& CGCsvLine::operator=(const CGCsvLine& source)
{
	clear();

	for(UINT n=0;n<source.size();n++)
		push_back(source[n]);

	m_iLineNumber = source.m_iLineNumber;

	return *this;
}

// Return TRUE if this line is equal to the specified one
// (All fields must be equals)
BOOL CGCsvLine::operator==(const CGCsvLine& source)
{
	if(size() != source.size())
		return FALSE;

	for(UINT n=0;n<source.size();n++)
	{
		if((*this)[n] != source[n])
			return FALSE;
	}
	return TRUE;
}

// Remove all fields from the list
void CGCsvLine::Clear()
{
	clear();
	m_iLineNumber = -1;
}


// ----------------------------------------------------------------------------------------------------------
// Description: Retrieve the field with the specified index.
//
// Arguments:
//			iFieldIndex: Index of the field to retrieve.
//			szFieldValue: The value of the retrieved field.
//			iMaxSize: The maximum size of the field string to retrieve.
//
// Return:
//		TRUE if successful; FALSE if the specified field index does not exist.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvLine::GetField(INT iFieldIndex, PSTR szFieldValue, INT iMaxSize) const
{
	string	strField;

	if(iFieldIndex >= (INT)size() )
		return FALSE;

	strField = (*this)[iFieldIndex];
	strncpy(szFieldValue,strField.c_str(),iMaxSize-1);
	return TRUE;
}

// Same function than above, but with a STL string argument.
BOOL CGCsvLine::GetField(INT iFieldIndex, string& strFieldValue) const
{
	if(iFieldIndex >= (INT)size() )
		return FALSE;

	strFieldValue = (*this)[iFieldIndex];
	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Retrieve the line content in a string, starting at the specified field index position
//
// Arguments:
//		strLine: A STL string that will receive the .csv line.
//		nStartField: The index of the first field to put in the string
//		nEndField: The index of the last field to put in the string (if set to 0, all fields from nStartField
//				   to the last field in the line will be retrieved).
//		cSeparator: The separator used to delimite each field.
//
// ----------------------------------------------------------------------------------------------------------
void CGCsvLine::GetLineString(string& strLine,
							  UINT nStartField /* = 0 */,
							  UINT nEndField /* = 0 */,
							  CHAR cSeparator /* = GCSV_DEFAULT_SEPARATOR */) const
{
	strLine = "";

	UINT	nLastFieldToRead = nEndField == 0 ? size()-1 : nEndField;
	if(nLastFieldToRead >= size())
		nLastFieldToRead = size()-1;

	if(nLastFieldToRead <= nStartField)
		return; // Nothing to retrieve

	for(UINT n=nStartField;n<=nLastFieldToRead;n++)
	{
		strLine += (*this)[n];
		if(n <  nLastFieldToRead)
			strLine += cSeparator;
	}
}

// ----------------------------------------------------------------------------------------------------------
// class CGCsvFile

GBEGIN_ERROR_MAP(CGCsvFile)
	GMAP_ERROR(eAlreadyOpened,"The file is already opened")
	GMAP_ERROR(eInvalidMode,"Invalid opening mode combination")
	GMAP_ERROR(eOpenFile,"%s")
	GMAP_ERROR(eMapFile,"Cannot map file")
	GMAP_ERROR(eEOF,"Reach end of file")
	GMAP_ERROR(eWrongMode,"The file is not opened in the correct mode to support this operation.")
	GMAP_ERROR(eWrite,"%s")
	GMAP_ERROR(eNoFileInMem,"No file loaded into memory; the operation cannot be completed")
	GMAP_ERROR(eGetField,"Cannot get field index %d from current line")
	GMAP_ERROR(eBufferTooSmall,"The buffer is too small to receive data")
	GMAP_ERROR(eRetrieveHeader,"Cannot retrieve a column with the header: %s")
	GMAP_ERROR(eLineNotFound,"The line number %d cannot be found")
	GMAP_ERROR(eLockFile,"Cannot lock the file: %s")
GEND_ERROR_MAP(CGCsvFile)

// ----------------------------------------------------------------------------------------------------------
// Construction/Destruction

CGCsvFile::CGCsvFile(CHAR cSeparator /*= ','*/) :
	m_cSeparator(cSeparator), m_iCurrentLine(0), m_pFile(NULL), m_pFileMap(NULL)
{

}

CGCsvFile::~CGCsvFile()
{
	Close();
}

// ----------------------------------------------------------------------------------------------------------
// Description: Open a .csv file in reading or writing mode.
//
// Arguments:
//		szFileName: The full file name of the file to open.
//		dwOpenMode: A combination of flags used to open the file.
//
// Return: TRUE if successful; therwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::Open(PCSTR szFileName, DWORD dwOpenMode)
{
	if(dwOpenMode & eModeRead)
	{
		if(dwOpenMode != eModeRead) // This flag cannot be combined with other flags
		{
			GSET_ERROR(CGCsvFile,eInvalidMode);
			return FALSE;
		}

		if(m_pFileMap != NULL)
		{
			GSET_ERROR(CGCsvFile,eAlreadyOpened);
			return FALSE;
		}

		m_pFileMap = new CGFileMap();

		if(m_pFileMap->MapFile(szFileName,CGFileMap::eRead) == FALSE)
		{
			GSET_ERROR0(CGCsvFile,eMapFile,GGET_LASTERROR(CGFileMap,m_pFileMap));
			return FALSE;
		}
		m_pMapBegin = m_pMapData = (PCSTR)m_pFileMap->GetReadMapData();
		m_pMapEnd = m_pMapData + m_pFileMap->GetFileSize();
	}
	else
	{
		// We must have eModeWrite flag here
		if ((dwOpenMode & eModeWrite) == 0)
		{
			GSET_ERROR(CGCsvFile,eInvalidMode);
			return FALSE;
		}

		if(m_pFile != NULL)
		{
			GSET_ERROR(CGCsvFile,eAlreadyOpened);
			return FALSE;
		}

		CHAR	szFlag[16];
		int		iIndex = 0;

		szFlag[iIndex++] = 'w';

		if(dwOpenMode & eModeCreate)
			szFlag[iIndex++] = '+';

		if(dwOpenMode & eModeAppend)
		{
			szFlag[0] = 'a';
			szFlag[1] = '+';
			iIndex = 2;
		}

		szFlag[iIndex++] = 't';
		szFlag[iIndex] = '\0';

		m_pFile = fopen(szFileName,szFlag);
		if(m_pFile == NULL)
		{
			GSET_ERROR1(CGCsvFile,eOpenFile,NULL,strerror(errno));
			return FALSE;
		}
	}

	return TRUE;
}

void CGCsvFile::Close()
{
	if(m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
	if(m_pFileMap != NULL)
	{
		m_pFileMap->CloseMap();
		delete m_pFileMap;
		m_pFileMap = NULL;
		m_pMapData = NULL;
	}
	m_iCurrentLine = 0;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read a line from the .csv file at the current position.
//
// Argument:
//		line: A reference to a CGCsvLine object that is populated with line content.
//
// Return: TRUE if successful; therwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::ReadLine(CGCsvLine& line)
{
	if(m_pFileMap == NULL)
	{
		GSET_ERROR(CGCsvFile,eWrongMode);
		return FALSE;
	}
	if(m_pMapData == m_pMapEnd)
	{
		GSET_ERROR(CGCsvFile,eEOF);
		return FALSE;
	}

	string		strLine;
	PCSTR		pCurData = m_pMapData;

	// Retrieve the position of the end of the line
	while(*pCurData != '\n' && *pCurData != '\r' && pCurData < m_pMapEnd)
	{
		strLine += *pCurData;
		pCurData++;
	}

	line.Set(strLine,m_cSeparator);

	if(pCurData < m_pMapEnd)
	{
		m_pMapData = pCurData;
		while(*m_pMapData == '\n' || *m_pMapData == '\r')
			m_pMapData++;
	}
	else
		m_pMapData = m_pMapEnd; // Reach end of file

	m_iCurrentLine++;
	line.m_iLineNumber = m_iCurrentLine;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read a line from the .csv file at the current position.
//
// Argument:
//		line: A reference to a string object that will received line content.
//
// Return: TRUE if successful; therwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::ReadLine(string& strLine)
{
	if(m_pFileMap == NULL)
	{
		GSET_ERROR(CGCsvFile,eWrongMode);
		return FALSE;
	}

	if(m_pMapData == m_pMapEnd)
	{
		GSET_ERROR(CGCsvFile,eEOF);
		return FALSE;
	}

	strLine = "";

	PCSTR		pCurData = m_pMapData;
	// Retrieve the position of the end of the line
	while(*pCurData != '\n' && *pCurData != '\r' && pCurData < m_pMapEnd)
	{
		strLine += *pCurData;
		pCurData++;
	}

	if(pCurData < m_pMapEnd)
	{
		m_pMapData = pCurData;
		while(*m_pMapData == '\n' || *m_pMapData == '\r')
			m_pMapData++;
	}
	else
		m_pMapData = m_pMapEnd; // Reach end of file

	m_iCurrentLine++;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read a line from the .csv file at the current position.
//
// Argument:
//		szLine: A pointer to a string buffer that will received the line.
//		nMaxLength: The maximum size of the line to read. If the line from the file exceed
//					this size, the function will return FALSE.
//
// Return: TRUE if successful; therwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::ReadLine(PSTR szLine, UINT nMaxLength)
{
	if(m_pFileMap == NULL)
	{
		GSET_ERROR(CGCsvFile,eWrongMode);
		return FALSE;
	}

	PCSTR		pCurData = m_pMapData;
	// Retrieve the position of the end of the line
	while(*pCurData != '\n' && *pCurData != '\r' && pCurData < m_pMapEnd)
		pCurData++;

	UINT	nLineSize = pCurData - m_pMapData;
	// If the size of the line if NULL
	if(nLineSize == 0)
	{
		// Reach the end of the file ?
		if(pCurData >= m_pMapEnd)
		{
			GSET_ERROR(CGCsvFile,eEOF);
			return FALSE;
		}
		while(*m_pMapData == '\n' || *m_pMapData == '\r')
			m_pMapData++;
		return TRUE;
	}
	else
	if(nLineSize >= nMaxLength) // Must considere a byte left for the '\0'
	{
		GSET_ERROR(CGCsvFile,eBufferTooSmall);
		return FALSE;
	}
	// Allocate a temporary string butffer to receive the line from the map
	strncpy(szLine,m_pMapData,nLineSize);
	*(szLine+nLineSize) = '\0';

	m_pMapData += nLineSize;
	while(*m_pMapData == '\n' || *m_pMapData == '\r')
		m_pMapData++;

	m_iCurrentLine++;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write a line to the .csv file at the current position.
//
// Argument:
//		line: A reference to a CGCsvLine object to write in the file.
//
// Return: TRUE if successful; therwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::WriteLine(const CGCsvLine& line)
{
	if(m_pFile == NULL)
	{
		GSET_ERROR(CGCsvFile,eWrongMode);
		return FALSE;
	}
	INT	iFieldCount = line.size();

	for(INT iLoop=0;iLoop<iFieldCount;iLoop++)
	{
		string		strField;
		line.GetField(iLoop,strField);

		if(fputs(strField.c_str(),m_pFile) < 0)
		{
			GSET_ERROR1(CGCsvFile,eWrite,NULL,strerror(errno));
			return FALSE;
		}
		// Reach the last field ?
		if(iLoop == iFieldCount-1)
		{
			// Close the line
			if(fputc('\n',m_pFile) < 0)
			{
				GSET_ERROR1(CGCsvFile,eWrite,NULL,strerror(errno));
				return FALSE;
			}
		}
		else // Other fields to write,
		{	 // so add a separator to the line.
			if(fputc(m_cSeparator,m_pFile) < 0)
			{
				GSET_ERROR1(CGCsvFile,eWrite,NULL,strerror(errno));
				return FALSE;
			}
		}
	}

	m_iCurrentLine++;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write a line to the .csv file at the current position.
//				*** Note that a '\n' is automatically added at the end of the line.
//
// Argument:
//		strLine: A reference to a string object to write in the file.
//
// Return: TRUE if successful; therwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::WriteLine(const string& strLine)
{
	if(m_pFile == NULL)
	{
		GSET_ERROR(CGCsvFile,eWrongMode);
		return FALSE;
	}
	// Write the line
	if(fputs(strLine.c_str(),m_pFile) < 0)
	{
		GSET_ERROR1(CGCsvFile,eWrite,NULL,strerror(errno));
		return FALSE;
	}
	// Close the line
	if(fputc('\n',m_pFile) < 0)
	{
		GSET_ERROR1(CGCsvFile,eWrite,NULL,strerror(errno));
		return FALSE;
	}

	m_iCurrentLine++;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write a line to the .csv file at the current position.
//				*** Note that a '\n' is automatically added at the end of the line.
//
// Argument:
//		szLine: A NULL terminated string buffer to write in the file.
//
// Return: TRUE if successful; therwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::WriteLine(PCSTR szLine)
{
	if(m_pFile == NULL)
	{
		GSET_ERROR(CGCsvFile,eWrongMode);
		return FALSE;
	}
	// Write the line
	if(fputs(szLine,m_pFile) < 0)
	{
		GSET_ERROR1(CGCsvFile,eWrite,NULL,strerror(errno));
		return FALSE;
	}
	// Close the line
	if(fputc('\n',m_pFile) < 0)
	{
		GSET_ERROR1(CGCsvFile,eWrite,NULL,strerror(errno));
		return FALSE;
	}

	m_iCurrentLine++;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read the entire opened file to memory.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::ReadFile()
{
	CGCsvLine	line;

	while(ReadLine(line) == TRUE)
		push_back(line);

	// We should have reach the end of the file; if not, return FALSE
	if(GGET_LASTERRORCODE(CGCsvFile,this) != eEOF)
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write the entire list of lines in the current opened file.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::WriteFile()
{
	for(UINT n=0;n<size();n++)
	{
		if(WriteLine((*this)[n]) == FALSE)
			return FALSE;
	}
	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Retrieve the list of line where a specific field is equal (or contains) a specific string.
//				Note that you must call ReadFile() member function before to call this one. Querys are
//				only operate in memory, not from the file in the disk.
//
// Arguments:
//			iFieldIndex: The index of the field to check.
//			szFieldValue: The value to compare to the field.
//			bStrict: TRUE if szFieldValue must be equal to the field value.
//					 FALSE if the field can contains szFieldValue to add the line to the list.
//			lstLines: Receive the list of lines where the field index iFieldIndex is equal (or contains)
//					  the string szFieldValue.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::QueryLine(INT iFieldIndex, PCSTR szFieldValue, BOOL bStrict, CGCsvLineList& lstLines) const
{
	if(empty())
	{
		GSET_ERROR(CGCsvFile,eNoFileInMem);
		return FALSE;
	}

	string	strFieldValue;
	// Go through the line list
	for(UINT n=0;n<size();n++)
	{
		if((*this)[n].GetField(iFieldIndex, strFieldValue) == FALSE)
		{
			GSET_ERROR1(CGCsvFile,eGetField,NULL,iFieldIndex);
			return FALSE;
		}

		if(bStrict == FALSE)
		{
			if(strFieldValue.find(szFieldValue) != string::npos)
				lstLines.push_back((*this)[n]);
		}
		else
		if(strFieldValue == szFieldValue)
			lstLines.push_back((*this)[n]);
	}
	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Retrieve the list of line where a specific field is equal (or contains) a specific string.
//				Note that you must call ReadFile() member function before to call this one. Queries are
//				only operate in memory, not from the file in the disk.
//				*** Of course, the .csv file must contains a first line where each field is the name of the
//				column.
//
// Arguments:
//			szHeaderName: The name of the column header of the field to check.
//			szFieldValue: The value to compare to the field.
//			bStrict: TRUE if szFieldValue must be equal to the field value.
//					 FALSE if the field can contains szFieldValue to add the line to the list.
//			lstLines: Receive the list of lines where the field index iFieldIndex is equal (or contains)
//					  the string szFieldValue.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGCsvFile::QueryLine(PCSTR szHeaderName, PCSTR szFieldValue, BOOL bStrict, CGCsvLineList& lstLines) const
{
	if(empty())
	{
		GSET_ERROR(CGCsvFile,eNoFileInMem);
		return FALSE;
	}

	// First, retrieve the index of the column coresponding to szHeaderName
	CGCsvLine	line((*this)[0]);
	BOOL		bFound = FALSE;
	int			i;
	// Go through the list of field for the first line in the file (that should be the 'header' line)
	for(i=0;i<int(line.size());i++)
	{
		string	strCurField;

		if(!line.GetField(i,strCurField))
		{
			GSET_ERROR1(CGCsvFile,eGetField,NULL,i);
			return FALSE;
		}
		// Find the correct header?
		if(strCurField == szHeaderName)
		{
			bFound = TRUE;
			break;
		}
	}
	// The header cannot be retrieved
	if(bFound == FALSE)
	{
		GSET_ERROR1(CGCsvFile,eRetrieveHeader,NULL,szHeaderName);
		return FALSE;
	}

	return QueryLine(i,szFieldValue,bStrict,lstLines);
}


// Go to the beginning of the file
void CGCsvFile::Rewind()
{
	// If the file is opened in reading mode
	if(m_pMapData != NULL)
		m_pMapData = m_pMapBegin;
	else
		(void)fseek(m_pFile, 0, SEEK_SET);

	m_iCurrentLine = 0;
}

// Remove the specified line from the list
BOOL CGCsvFile::RemoveLine(INT iLineNumber)
{
	iterator it = begin();
	while(it < end())
	{
		if(it->GetLineNumber() == iLineNumber)
		{
			it = erase(it);
			// For the moment, it the next line is not the end, we
			// must have the next line number == line number + 1
			if(it != end())
				ASSERT(it->m_iLineNumber == iLineNumber+1);
			// Update all following line numbers
			while(it < end())
			{
				it->m_iLineNumber--;
				it++;
			}

			return TRUE;
		}
		it++;
	}

	GSET_ERROR1(CGCsvFile,eLineNotFound,NULL,iLineNumber);
	return FALSE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Insert a line at the specified position (line number) in the file.
//
// Arguments:
//			iLineNumber: The position of the line to insert. If -1, insert at the end.
//			line: The line to insert.
//
// ----------------------------------------------------------------------------------------------------------
void CGCsvFile::InsertLine(INT iLineNumber, const CGCsvLine& line)
{
	iterator it = begin();
	while(it < end())
	{
		if((iLineNumber != -1) && (it->m_iLineNumber == iLineNumber))
		{
			it = insert(it,line);
			it->m_iLineNumber = iLineNumber;

			it++;
			// For the moment, the next line must have the same number
			ASSERT(it->m_iLineNumber == iLineNumber);
			// Update all following line numbers
			while(it < end())
			{
				it->m_iLineNumber++;
				it++;
			}
			return;
		}
		else
		// Missing the line number ot insert?
		if((iLineNumber != -1) && (it->m_iLineNumber > iLineNumber))
		{
			it = insert(it,line);
			it->m_iLineNumber = iLineNumber;
			return;
		}
		it++;
	}

	// If the line number does not already exists, insert the line
	// at the end of the file
	push_back(line);

	it = end();
	it--;

	iterator	itOldLast = it;
	itOldLast--;
	it->m_iLineNumber = itOldLast->m_iLineNumber+1;
}

// Same function than above, with string parameter
void CGCsvFile::InsertLine(INT iLineNumber, const string& strLine)
{
	CGCsvLine	line(strLine,m_cSeparator);
	InsertLine(iLineNumber, line);
}

