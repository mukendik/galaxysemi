// gcsvfile.h: interface for the CGCsvFile class.
//
// ----------------------------------------------------------------------------------------------------------

#if !defined(AFX_GCSVFILE_H__983D0795_470F_4060_B902_DA96AD07D82D__INCLUDED_)
#define AFX_GCSVFILE_H__983D0795_470F_4060_B902_DA96AD07D82D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gstdl_errormgr.h>
/*
#if defined(_WIN32)
#	ifdef _MSC_VER
#		pragma warning (disable : 4275)
#	endif // _MSC_VER
#	if defined(_GALAXY_CSV_FILE_EXPORTS_)
#		define GCSVFILE_API __declspec(dllexport)
#	elif defined _GALAXY_STDUTILS_DLL_MODULE
#		define GCSVFILE_API
#	else	// _GALAXY_CRYPTO_EXPORTS_
#		define GCSVFILE_API __declspec(dllimport)
#	endif // _GALAXY_CRYPTO_EXPORTS_
#else
#	define GCSVFILE_API
#endif
*/

#define GCSVFILE_API

#define GCSV_DEFAULT_SEPARATOR		','

// std class already instantiated in gerrormgr.h
typedef std::vector<std::string>	CGCsvFieldList;

// ----------------------------------------------------------------------------------------------------------
// class CGCsvLine ( define what is a line in a .csv file )

class GCSVFILE_API CGCsvLine : public CGCsvFieldList
{
// Constructor / Destructor
public:
	CGCsvLine();
	CGCsvLine(const CGCsvLine& source);
	CGCsvLine(const std::string& strLine, CHAR cSeparator = GCSV_DEFAULT_SEPARATOR);
	CGCsvLine(PCSTR szLine, CHAR cSeparator = GCSV_DEFAULT_SEPARATOR);
	virtual ~CGCsvLine();

	// Initialization
	void	Set(const std::string& strLine, CHAR cSeparator = GCSV_DEFAULT_SEPARATOR);
	void	Set(PCSTR szLine, CHAR cSeparator = GCSV_DEFAULT_SEPARATOR);

// Attributes
public:
	INT		GetLineNumber() const;	// Retrieve the 1 base indexed line number in the file (set to -1 for a line created out of a file)
	BOOL	GetField(INT iFieldIndex, PSTR szFieldValue, INT iMaxSize) const;
	BOOL	GetField(INT iFieldIndex, std::string& strFieldValue) const;
	void	GetLineString(std::string& strLine, UINT nStartField = 0, UINT nEndField = 0, CHAR cSeparator = GCSV_DEFAULT_SEPARATOR) const;

// Operations
public:
	void	AppendFields(PCSTR szFields, CHAR cSeparator = GCSV_DEFAULT_SEPARATOR);
	BOOL	ReplaceField(INT iFieldIndex, PCSTR szNewFieldValue);
	void	Clear();

// Operators
public:
	CGCsvLine&	operator=(const CGCsvLine& source);
	BOOL		operator==(const CGCsvLine& source);

// Implementation
protected:
	INT				m_iLineNumber;

	friend class CGCsvFile;
};

typedef std::vector<CGCsvLine>	CGCsvLineList;

class CGFileMap;

// ----------------------------------------------------------------------------------------------------------
// class CGCsvFile 

class GCSVFILE_API CGCsvFile : public CGCsvLineList
{
// Constructor / Destructor
public:
	CGCsvFile(CHAR cSeparator = GCSV_DEFAULT_SEPARATOR);
	virtual ~CGCsvFile();

// Attributes
public:
	GDECLARE_ERROR_MAP(CGCsvFile)
	{
		eAlreadyOpened,		// Try to open a file already opened
		eInvalidMode,		// Invalid opening mode combination
		eOpenFile,			// Cannot open file
		eMapFile,			// Cannot map file
		eEOF,				// Reach end of file
		eWrongMode,			// Wrong open mode for the current operation
		eWrite,				// Write error
		eNoFileInMem,		// No file loaded in memory; operation cannot be completed
		eGetField,			// Cannot get the specified field
		eBufferTooSmall,	// The buffer is too small to receive data
		eRetrieveHeader,	// Cannot retrieve the specified header
		eLineNotFound,		// The specified line cannot be found
		// The following error code are not used in this base class, but
		// can be used in derived class. As the Galaxy Error Management is
		// not perfect and can not handle "derived class" error problem for
		// the moment, we need to put all error messages in the base class.
		eLockFile			// Cannot lock a file

	}
	GDECLARE_END_ERROR_MAP(CGCsvFile)

	enum EOpenMode {	eModeRead	=	0x0001, // Can not be combined with other flags
						eModeWrite	=	0x0002,
						eModeAppend	=	0x0004,
						eModeCreate =	0x0008
	};
	
// Operations
public:
	// Memory operations
		// Add lines to the .csv
	void	InsertLine(INT iLineNumber, const CGCsvLine& line);
	void	InsertLine(INT iLineNumber, const std::string& strLine);
		// Remove lines from the .csv
	BOOL	RemoveLine(INT iLineNumber);
		// Query
	BOOL	QueryLine(INT iFieldIndex, PCSTR szFieldValue, BOOL bStrict, CGCsvLineList& lstLines) const;
	BOOL	QueryLine(PCSTR szHeaderName, PCSTR szFieldValue, BOOL bStrict, CGCsvLineList& lstLines) const;

	// File operations
	BOOL	Open(PCSTR szFileName, DWORD dwOpenMode);
	void	Close();
		// Read a line from the file at the current position
	BOOL	ReadLine(CGCsvLine& line);	
	BOOL	ReadLine(std::string& strLine);
	BOOL	ReadLine(PSTR szLine, UINT nMaxLength);
		// Write a line to the file at the current position
	BOOL	WriteLine(const CGCsvLine& line);
	BOOL	WriteLine(const std::string& strLine);
	BOOL	WriteLine(PCSTR szLine);
		
	BOOL	ReadFile(); // Read the entire file to memory
	BOOL	WriteFile();// Write all lines from memory to the file

	void	Rewind(); // Go to the beginning of the file

// Implementation
protected:
	CHAR			m_cSeparator; // The current separator used for the .csv file (usually a ',' character)

	// File operations
	INT				m_iCurrentLine; // 1 based indexed line number
		// Writting operations (Use standard C library)
	FILE*			m_pFile;

		// Reading operations (Use a map for clarity)
	CGFileMap		*m_pFileMap;
	PCSTR			m_pMapBegin; // Pointer to the beginning of the file map
	PCSTR			m_pMapData;	 // Pointer to the current position in the file map
	PCSTR			m_pMapEnd;	 // Pointer to the end of the file map

	// The only object allowed to set internaly the line number for a specific line
	void			SetLineNumber(CGCsvLine& line, int iLineNumber) { line.m_iLineNumber = iLineNumber;	}
};

#endif // !defined(AFX_GCSVFILE_H__983D0795_470F_4060_B902_DA96AD07D82D__INCLUDED_)
