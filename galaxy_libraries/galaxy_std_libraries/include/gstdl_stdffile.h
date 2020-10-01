// ----------------------------------------------------------------------------------------------------------
// CGStdfFile class HEADER : allows to Read / Write STDF V3 / V4
//
// ----------------------------------------------------------------------------------------------------------
//
// Notes: 
//
// ----------------------------------------------------------------------------------------------------------

#ifndef _GALAXY_STDF_FILE_HEADER_
#define _GALAXY_STDF_FILE_HEADER_
/*
#if defined(_WIN32)
#	if defined(_GALAXY_STDF_FILE_EXPORTS_)
#		define GSTDFFILE_API __declspec(dllexport)
#	elif defined(_GALAXY_STDF_DLL_MODULE)
#		define GSTDFFILE_API
#	else	// _GALAXY_STDF_FILE_EXPORTS_
#		define GSTDFFILE_API __declspec(dllimport)
#	endif // _GALAXY_STDF_FILE_EXPORTS_
#else
#	define GSTDFFILE_API
#endif*/

#define GSTDFFILE_API

// ----------------------------------------------------------------------------------------------------------
// PUBLIC DEFINITIONS
// ----------------------------------------------------------------------------------------------------------

#define	STDF_READ				1
#define	STDF_WRITE				2
#define	GOFF_REC_DATA			4	// STDF offset to reach first data in STDF record (skip header)
#define FLT_MAX_4BYTES			3.402823466e+38F
#define FLT_MAX_8BYTES			1.7e+308
// STDF Read buffer cache memory
#define	STDF_READ_CACHE			2000000L

// ----------------------------------------------------------------------------------------------------------
// INCLUDES 
// ----------------------------------------------------------------------------------------------------------
#include <gstdl_errormgr.h>
#include <stdio.h>
#include <string.h>

#include "gstdl_stdftype.h"
#include "gstdl_stdfrectype.h"

#ifdef __unix__
    #define inline
#endif

// ----------------------------------------------------------------------------------------------------------
// Record info : tells information about Record Read
struct GSTDFFILE_API StdfRecordReadInfo
{
	StdfRecordReadInfo()
	{
		iRecordType = 0;
		iRecordSubType = 0;
		iCpuType = 1;
		iStdfVersion = 4;
	}

	int	iRecordType;			// STDF Record Type (eg: 0 for FAR)
	int	iRecordSubType;			// STDF Record sub-type (eg: 10 for FAR)
	int	iCpuType;				// CPU type that generated the STDF file
	int	iStdfVersion;			// File STDF Version
};
typedef StdfRecordReadInfo* LPStdfRecordReadInfo;

// ----------------------------------------------------------------------------------------------------------
// class CGStdfFile

class GSTDFFILE_API CGStdfFile
{
// Constructor/Destructor
public:
	CGStdfFile();
	virtual ~CGStdfFile();

// Attributes
public:
	GDECLARE_ERROR_MAP(CGStdfFile)
	{
		eFileClosed,	// File not opened (so access not allowed)
		eErrorOpen,		// Error opening/creating file
		eErrorMode,		// Invalid access mode specified
		eErrorMemory,	// Memory allocation error.
		eEndOfFile,		// End of file reached.
		eCorrupted,		// File corrupted...unexpected end of file in record
		eEndOfBuffer,	// End of buffer reached reading STDF record
		eWriteError,	// Error writing (disk full ?)
		eReportFile		// Can't create report file (output file)
	}
	GDECLARE_END_ERROR_MAP(CGStdfFile)


    LONG GetFileSize(PCSTR szStdfFile) const;	// Tells file size (read file only)
    LONG GetReadRecordPos(void) const;// Tells current position in STDF record READ
    LONG GetPos(void) const;				// Tells current position in file devided by 1024!
    int GetCurrentBufferSize() const { return m_nReadBufferSize;	}

// Operations
public:
	// STDF File init.
    // Opens STDF file in STDF_READ or STDF_WRITE mode
    BOOL Open(PCSTR szStdfFile,int iAccessMode,long lCacheSizeValue=-1);
    BOOL Close(void);	// Closes STDF file
    BOOL SetReadRecordPos(LONG);// Overwrite current READ position in STDF record

	// Functions to extract fields from STDF record.
    BOOL LoadRecord(LPStdfRecordReadInfo); // Loads one STDF record from STDF file.
    BOOL ReadByte(BYTE *);		// read 1 char. (U*1)
    BOOL ReadString(char *);		// read string	(U*n)
    BOOL ReadWord(uint16_t*);		// read word	(U*2)
    BOOL ReadDword(uint32_t*);		// read Dword	(U*4)
    BOOL ReadFloat(float *);		// read float	(R*4)
    BOOL ReadDouble(double *);	// read double	(R*8)
    BOOL ReadBitField(BITFIELD& bitField);

	// Functions to write a STDF record
	BOOL	Flush(); // Flush file buffer to current opened file
	void	WriteFloat(float fData);
	void	WriteDouble(double dfData);
	void	WriteBitField(const BITFIELD& bitField);
    void	WriteFixedString(PSTR szString,const INT iFixedLength);
	BOOL	DumpRecord(LPStdfRecordReadInfo pRecordReadInfo,CGStdfFile *pStdfRecord);
	// Force small functions to be inlined
        void	WriteHeader(LPStdfRecordReadInfo pRecordReadInfo);
        // Write stdf record to disk...
        BOOL	WriteRecord(void);
        // Copy in stdf buffer a 1 byte number.
        void	WriteByte(BYTE bData);
        void	WriteWord(WORD wData);
        void	WriteDword(DWORD dwData);
        void	WriteString(PCSTR szString);

protected:
	FILE*	m_hStdf;				// Handle to STDF file
	int		m_iFileAccessMode;	// file access mode : STDF_READ or STDF_WRITE
	int		m_iCpuType;			// 0=PDP11,1=Sun,2=PC
	int		m_iVersion;			// STDF version (3,4)
	int		m_iRecType;			// STDF record type
	int		m_iRecSub;			// STDF record sub type
	BYTE	*m_hStdfMem;			// Handle top buffer for reading/writing STDF
	BYTE	*m_hReadStdfCache;	// Cache memory for very fast STDF Read functions !
	long	SmartFread(BYTE **ptRxBuffer, int iBlockSize);	// Supports Read Cache function !
	void	RewindFile(void);
	long	m_lCacheSize;			// Read/Write buffer Cache memory size
	long	m_iBytesInCache;		// Keeps track of valid block size in cache
	long	m_nCacheReadOffset;	// Current read index in cache buffer
	char	m_szErrorMsg[128];

	// Variables used when READING a STDF file
	int		m_nReadOffset;		// current read index in buffer
	int		m_nReadBufferSize;	// STDF READ Buffer size (header + data record size)
	long	m_lRecRead;			// Total records read.
	long	m_lFilePos;			// Position in file, takes into account Read cache buffer

	// Variables used when WRITING a STDF file
    int	m_nWriteOffset;		// current read index in buffer
    int	m_nWriteBlockSize;	// Size of Write buffer

    const char*	GetErrorMessage(int iErrorCode);
};

#endif // _GALAXY_STDF_FILE_HEADER_
