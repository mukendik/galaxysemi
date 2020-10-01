///////////////////////////////////////////////////////////
// SPEKTRA class HEADER : allows to Read SPEKTRA files
///////////////////////////////////////////////////////////

#ifndef _CSpektra_h_
#define _CSpektra_h_

#include "gstdl_type.h"
//typedef unsigned char	BYTE;
//typedef unsigned short	WORD;
//typedef unsigned long	DWORD;
//typedef float			FLOAT;

#include <stdio.h>

// Maximum Value a 4bytes Floating point number can reach.
#define FLT_MAX_4BYTES 	3.402823466e+38F
#define DBL_MAX_8BYTES 	1.7976931348623158e+308

// SPEKTRA Read buffer cache memory
#define	SPEKTRA_READ_CACHE 2000000L
#define	SPEKTRA_READWRITE_BLOCK 5000

class CSpektra
{
public:
	// Constructor / destructor functions
	CSpektra();
	~CSpektra();

	// SPEKTRA File init.
	int	 Open(const char* szSpektraFile,long lCacheSizeValue=-1);		// Opens SPEKTRA file
	int	 Close(void);											// Closes SPEKTRA file
	void RewindFile(void);
	long GetFileSize(char *szSpektraFile);	// Tells file size (read file only)

	// Functions to extract fields from SPEKTRA record.
	int	LoadRecord(int nRecorSize);					// Loads one SPEKTRA record from SPEKTRA file.
	int	ReadByte(BYTE* pbData);						// read 1 char. (1 byte)
	int	ReadWord(WORD* pwData);						// read word	(2 bytes)
	int	ReadDword(DWORD* pdwData);					// read Dword	(4 bytes)
	int	ReadFloat(FLOAT* pfData);					// read float	(4 bytes)
	int	ReadString(char* szString, int nLength);	// read string	(nLength * char)


	// Error codes returned by CSpektra functions
	enum ErrorCode {	NoError,		// No error
						FileOpened,		// File already opened.
						FileClosed,		// File not opened (so access not allowed)
						ErrorOpen,		// Error opening/creating file
						ErrorMode,		// Invalid access mode specified
						ErrorMemory,	// Memory allocation error.
						EndOfFile,		// End of file reached.
						Corrupted,		// File corrupted...unexpected end of file in record
						EndOfBuffer,	// End of buffer reached reading STDF record
						WriteError		// Error writing (disk full ?)
	};

private:
	long	SmartFread(BYTE** ptRxBuffer, int nBlockSize);	// Supports Read Cache function !

	FILE*	m_hSpektra;				// Handle to SPEKTRA file
	BYTE*	m_hSpektraMem;			// Handle top buffer for reading/writing SPEKTRA
	BYTE*	m_hReadSpektraCache;	// Cache memory for very fast SPEKTRA Read functions !
	long	m_lCacheSize;			// Read/Write buffer Cache memory size
	long	m_lBytesInCache;		// Keeps track of valid block size in cache
	long	m_lCacheReadOffset;		// Current read index in cache buffer

	// Variables used when READING a SPEKTRA file
	int		m_nReadOffset;			// current read index in buffer
	int		m_nReadBufferSize;		// SPEKTRA READ Buffer size (header + data record size)
	long	m_lRecRead;			// Total records read.
	long	m_lFilePos;				// Position in file, takes into account Read cache buffer

};

#endif // #ifndef _CSpektra_h_
