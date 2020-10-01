///////////////////////////////////////////////////////////
// STDF class HEADER : allows to Read / Write STDF V3 / V4
///////////////////////////////////////////////////////////

#if !defined(QT_CSTDF_H__INCLUDED_)
#define QT_CSTDF_H__INCLUDED_

#define	STDF_READ	1	// Read STDF file
#define	STDF_WRITE	2	// Write STDF file
#define	STDF_APPEND	3	// Append to STDF file

#define	GOFF_REC_DATA	4	// STDF offset to reach first data in STDF record (skip header)

#include "gstdl_type.h"

//typedef unsigned char	BYTEFLAG;
//#if !defined(_WINDOWS_H) && !defined(_WINDOWS_)
//typedef void			VOID;
//typedef void*			PVOID;
//typedef void*			LPVOID;
//typedef char			CHAR;
//typedef unsigned char	BYTE;
////typedef unsigned char	BYTEFLAG;
//typedef unsigned short	WORD;
//typedef unsigned long	DWORD;
//typedef unsigned int	UINT;
//typedef short			SHORT;
//typedef int				INT;
//typedef int				INT32;
//#if defined(WIN32)
//#ifdef __GNUC__
//typedef long long               INT64;
//#else
//typedef __int64			INT64;
//#endif
//#endif
//typedef int				BOOL;
//typedef long			LONG;
//typedef float			FLOAT;
//typedef char*			PSTR;
//typedef char*			LPSTR;
//typedef const char*		PCSTR;

//#if !defined(NULL)
//#define NULL    0
//#endif
//#endif // _WINDOWS_H

//typedef double			DOUBLE;

#include <qfile.h>

// Maximum Value a 4bytes Floating point number can reach.
#define FLT_MAX_4BYTES 	3.402823466e+38F
#define DBL_MAX_8BYTES 	1.7976931348623158e+308

// STDF Read buffer cache memory
#define	STDF_READ_CACHE 2000000L
#define	STDF_READWRITE_BLOCK 1000000

// Record info : tells information about Record Read
struct	StdfRecordReadInfo
{
    int	iRecordSize;			// STDF Record size
    int	iRecordType;			// STDF Record Type (eg: 0 for FAR)
    int	iRecordSubType;			// STDF Record sub-type (eg: 10 for FAR)
    int	iCpuType;				// CPU type that generated the STDF file
    int	iStdfVersion;			// File STDF Version

    StdfRecordReadInfo& operator=(const StdfRecordReadInfo& source)
    {
        iRecordSize = source.iRecordSize;
        iRecordType = source.iRecordType;
        iRecordSubType = source.iRecordSubType;
        iCpuType = source.iCpuType;
        iStdfVersion = source.iStdfVersion;
        return *this;
    }
};
typedef StdfRecordReadInfo* LPStdfRecordReadInfo;

class CStdf
{
public:
	// Constructor / destructor functions
	CStdf();
	~CStdf();

	static bool IsCompatible(const char *szStdfFile);
	bool IsCompatible();

	// STDF File init.
	int	 Open(const char *szStdfFile,int iAccessMode,long lCacheSizeValue=-1,bool bValidityCheck=true);// Opens STDF file in STDF_READ or STDF_WRITE mode
    int	 Close(void);                           // Closes STDF file
	long GetFileSize(const char *szStdfFile);	// Tells file size (read file only)
    long GetPos();                              // Tells current position in file devided by 1024!
    long GetReadRecordPos(void);                // Tells current position in STDF record READ
    long GetReadRecordNumber(void);             // Tells record# read.
    int	 SetReadRecordPos(long);                // Overwrite current READ position in STDF record
    long GetWriteRecordPos(void);               // Tells current position in STDF record WRITE
	int	 SetWriteRecordPos(long nNewWriteOffset);// Overwrite current WRITE position in STDF record
	int	 IsStdfFormat(const char *szStdfFile, bool *pbValidFileStart);		// Checks if file contains valid STDF records
	bool RepairStdfFile(const char *szStdfFile, const char *szStdfFile_Repaired, int nCpuType);	// Try to repair STDF file

	BYTE*	GetMemoryHandle(void) { return hStdfMem; }

	void RewindFile(void);
	// Return STDF version
	int GetStdfVersion(void)  { return iVersion;  };
	// Return STDF cpu type
	int GetStdfCpuType(void)  { return iCpuType;  };
	void SetStdfCpuType(int iWriteCpuType);	// Define STDF output CPU format (to call just AFTER the Open (write mode) function)
	BYTE GetComputerCpuType(void);	// Returns 1 if running under Sparc Solaris, 2 if Intel PC platform.
	long GetStdfRecordsRead(void) { return lRecRead; };

	// Functions to extract fields from STDF record.
	int	LoadRecord(LPStdfRecordReadInfo);					// Loads one STDF record from STDF file.
	bool IsValidStdfRecordType(StdfRecordReadInfo *StdfRecordHeader);
	int	ReadByte(BYTE *);									// read 1 char. (U*1)
	int	ReadString(char *);									// read string	(C*n)
    int	ReadSNString(char *);								// read string	(C*n)
    int	ReadCFString(int iSize,char *);						// read string	(C*n)

	int	ReadBitField(BYTE *pbLength, char *pBitField);		// read bit-encoded field
	int	ReadDBitField(unsigned int *pwLength, char *pBitField);		// read double bit-encoded field
	int	ReadWord(int *);									// read word	(U*2)
	int	ReadDword(long *);									// read Dword	(U*4)
	int	ReadQword( unsigned long long *);									// read Dword	(U*8)
	int	ReadFloat(float *);									// read float	(R*4)
	int	ReadFloat(float *, bool *);							// read float	(R*4), and set a flag in the float is a NAN
	int	ReadDouble(double *ptDouble);						// read double  (R*8)
	int	OverwriteReadByte(BYTE);
	int	OverwriteReadWord(WORD);
	int	OverwriteReadDword(DWORD);

	// Functions to write a STDF record
	int	WriteHeader(LPStdfRecordReadInfo pRecordReadInfo);
	int	WriteRecord(void);
	int	WriteRecord(LPStdfRecordReadInfo pclStdfRecordReadInfo, CStdf *pclStdfSource);
	int	WriteDword(DWORD dwData);
	int	WriteWord(WORD wData);
	int	WriteQWord(unsigned long long wData);
	int	WriteWordOffset(WORD wData, BYTE *hBuffer, int nBufferOffset, bool bWriteSameCpu);
	int	WriteByte(BYTE bData);
	int	WriteFloat(float fData);
	int	WriteString(const char * szString);
	int	WriteSNString(const char * szString);
	int	WriteCFString(const char * szString);
	int	WriteBitField(BYTE bLength, char *pBitField);
	int	WriteDBitField(unsigned int wLength, char *pBitField);
	int	WriteFixedString(LPSTR szString,int iFixedLength);
	int	DumpRecord(LPStdfRecordReadInfo pRecordReadInfo,CStdf *pStdfRecord,bool bFullRecord=false);

	// Error codes returned by CStdf functions
	enum ErrorCode {NoError,		// No error
						FileOpened,	// File already opened.
						FileClosed,	// File not opened (so access not allowed)
						ErrorOpen,	// Error opening/creating file
						ErrorMode,	// Invalid access mode specified
						ErrorMemory,// Memory allocation error.
						EndOfFile,	// End of file reached.
						SkipRecord,	// Found record to ignore.
						Corrupted,	// File corrupted...unexpected end of file in record
						EndOfBuffer,// End of buffer reached reading STDF record
						WriteError,	// Error writing (disk full ?)
						ReportFile, // Can't create report file (output file)
						PlatformRestriction, // STDF file generated by a tester GEX license doesn't allow.
						RunningRestriction,  // GEX running in a mode that doesn't allow to run user scripts!
						TooRecent // STDF file date is too recent : refuse to work on it !
	};

private:
	FILE*	hStdf;				// Handle to STDF file
	int		iFileAccessMode;	// file access mode : STDF_READ or STDF_WRITE
	int		iCpuType;			// 0=PDP11,1=Sun,2=PC
	int		iVersion;			// STDF version (3,4)
	int		iRecType;			// STDF record type
	int		iRecSub;			// STDF record sub type
	BYTE	*hStdfMem;			// Handle top buffer for reading/writing STDF
	BYTE	*hReadStdfCache;	// Cache memory for very fast STDF Read functions !
	long	SmartFread(BYTE **ptRxBuffer, int iBlockSize);	// Supports Read Cache function !
	long	lCacheSize;			// Read/Write buffer Cache memory size
	long	iBytesInCache;		// Keeps track of valid block size in cache
	long	nCacheReadOffset;	// Current read index in cache buffer
	bool	bStdfValidityCheck;	// 'true' (default) if check STDF validity prior to read it (check that starts with FAR)

	// Variables used when READING a STDF file
	int		nReadOffset;		// current read index in buffer
	int		nReadBufferSize;	// STDF READ Buffer size (header + data record size)
	long	lRecRead;			// Total records read.
	long	lFilePos;			// Position in file, takes into account Read cache buffer

	// Variables used when WRITING a STDF file
	int		nWriteOffset;		// current read index in buffer
	int		nWriteBlockSize;	// Size of Write buffer
	bool	bWriteSameCpu;		// 'true' if write STDF record under a CPU same as the targeted STDF output CPU format.
};



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDFCMPDLG_H__41990006_0EE6_11D4_A1ED_0080C8FE85CF__INCLUDED_)
