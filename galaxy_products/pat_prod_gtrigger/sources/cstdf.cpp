///////////////////////////////////////////////////////////
// STDF class : allows to Read / Write STDF V3 / V4

// note: if compiling under 'console' mode include 'stdio.h' instead.
#include "stdio.h"

#ifdef __MACH__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif // __MACH__

#include "cstdf.h"
#include <string.h>


///////////////////////////////////////////////////////////
// Constructor : reset all private variables.
///////////////////////////////////////////////////////////
CStdf::CStdf()
{
	hStdf = NULL;
	hStdfMem = NULL;
	nReadOffset = 0;
	lRecRead = 0;
	hReadStdfCache = NULL;	// Cache buffer for very fast STDF Read functions !
	nCacheReadOffset = 0;	// Move pointer back to origin of cache buffer
	lFilePos=0;
	bStdfValidityCheck=true;
	lCacheSize = STDF_READ_CACHE;

	// Default CPU & STDF format
	SetStdfCpuType(1);
	iVersion = 3;
}

///////////////////////////////////////////////////////////
// Destructor : Closes file if not done,
// reset all private variables.
///////////////////////////////////////////////////////////
CStdf::~CStdf()
{
	Close();	// Closes STDF file if not already done.
}

///////////////////////////////////////////////////////////
// Only works for STDF file opened in READ mode !
///////////////////////////////////////////////////////////
void	CStdf::RewindFile(void)
{
	// Rewind file, so next 'LoadRecord' gets first record.
	// This function will not work if rewind is used after
	// more than one cache block has been read.
	nCacheReadOffset = 0;	// Move pointer back to origin of cache buffer
	lFilePos=0;
}

///////////////////////////////////////////////////////////
// Action : Opens specified file and verify if it's a STDF format
// return :	false if not
///////////////////////////////////////////////////////////
bool CStdf::IsCompatible(const char *szStdfFile)
{

	CStdf				clStdf;
	StdfRecordReadInfo	ReadInfo;

	// Open CStdf
	int nStatus = clStdf.Open(szStdfFile, STDF_READ, 1000000L);
	if(nStatus != CStdf::NoError)
	{
		clStdf.Close();
		return false;
	}
	nStatus = clStdf.LoadRecord(&ReadInfo);
	if(nStatus != CStdf::NoError)
	{
		clStdf.Close();
		return false;
	}
	if(!clStdf.IsValidStdfRecordType(&ReadInfo))
	{
		clStdf.Close();
		return false;
	}
	nStatus= clStdf.GetStdfVersion();
	if((nStatus != 3) && (nStatus != 4))
	{
		clStdf.Close();
		return false;
	}
	clStdf.Close();
	return true;
}



///////////////////////////////////////////////////////////
// Action : Opens specified STDF file, into given mode,
//			either STDF_READ, or STDF_WRITE or STDF_APPEND
//			Default cache size = 2Mega bytes...unless otherwise specified.
// return :	NoError or error code
///////////////////////////////////////////////////////////
int	CStdf::Open(const char *szStdfFile,int iAccessMode,long lCacheSizeValue,bool bValidityCheck/*=true*/)
{
	int		iStatus;

	if(hStdf != NULL)
		return FileOpened;	// File handle already in use...
	
	// Reset SameCpu flag
	bWriteSameCpu = true;

	// Save validity flag
	bStdfValidityCheck = bValidityCheck;

	// Saves file access mode. Used in all access function to validate
	// accesses of Read or Write functions.
	StdfRecordReadInfo ReadInfo;
	iFileAccessMode = iAccessMode;

	switch(iAccessMode)
	{
		// Access to STDF file in READ mode
		case STDF_READ:
			// If no cache size defined, set to default: 2Mega bytes
			if(lCacheSizeValue <= 0)
				lCacheSize = STDF_READ_CACHE;	// 2Mbytes.
			else
				lCacheSize = lCacheSizeValue;

			hStdf = fopen(szStdfFile,"rb");
			if(hStdf == NULL)
				return ErrorOpen;
			do
			{
				// Load first record...then check it is the FAR or MIR
				iStatus = LoadRecord(&ReadInfo);
				switch(iStatus)
				{
					case NoError:
					case Corrupted:
						RewindFile();
						break;		// Success
					
					default:
					case EndOfFile:	// Failed reading STDF record
						break;

					case SkipRecord:
						break;	// Found record to ignore...
				}
			}
			while(iStatus == SkipRecord);
			break;

		// Creating STDF file (access in WRITE mode)
		case STDF_WRITE:
			// If no cache size defined, set to default: 5K bytes
			if(lCacheSizeValue <= 0)
				lCacheSize = STDF_READWRITE_BLOCK;	// 5Kbytes.
			else
				lCacheSize = lCacheSizeValue;
			
			hStdf = fopen(szStdfFile,"wb");
			if(hStdf == NULL)
				return ErrorOpen;

			// Unless application defines the output CPU format, force it to SUN format.
			SetStdfCpuType(1);
			break;

		case STDF_APPEND:	// Same as STDF_WRITE, except that appends to existing file!
			// Force next calls to be handled as normal WRITE!
			iFileAccessMode = STDF_WRITE;
			
			// If no cache size defined, set to default: 5K bytes
			if(lCacheSizeValue <= 0)
				lCacheSize = STDF_READWRITE_BLOCK;	// 5Kbytes.
			else
				lCacheSize = lCacheSizeValue;
			
			hStdf = fopen(szStdfFile,"ab");
			if(hStdf == NULL)
				return ErrorOpen;

			// Unless application defines the output CPU format, force it to SUN format.
			SetStdfCpuType(1);
			break;
 
		// Invalid STDF file access mode.
		default:
			return ErrorMode;
	}
	return NoError; 
}

///////////////////////////////////////////////////////////
// Action : Checks if file contains valid STDF records
// return :	o STDF version if valid STDF file
//			o 1 if file in STDF format, but no FAR/MIR found
//			o 0 if file not in STDF format
//			o -1 if failed reading the file
///////////////////////////////////////////////////////////
int CStdf::IsStdfFormat(const char *szStdfFile, bool *pbValidFileStart)
{
	int					nStatus;
	StdfRecordReadInfo	ReadInfo;
	bool				bValidRecord;

	// Set output booleans
	*pbValidFileStart = false;

	// If file already opened, close it
	if(hStdf != NULL)
		Close();

	// Open file
	hStdf = fopen(szStdfFile,"rb");
	if(hStdf == NULL)
		return -1;

	// Set some global variables (Check validity during loads, access mode...)
	bStdfValidityCheck = true;
	iFileAccessMode = STDF_READ;

	// Try to load first STDF record
	nStatus = LoadRecord(&ReadInfo);
	bValidRecord = IsValidStdfRecordType(&ReadInfo);
	if(nStatus == NoError)
	{
		*pbValidFileStart = true;
		Close();
		return iVersion;
	}
	// Go through STDF records until found FAR/MIR, or EOF, or Corrupted
	while((nStatus == SkipRecord) && bValidRecord)
	{
		nStatus = LoadRecord(&ReadInfo);
		bValidRecord = IsValidStdfRecordType(&ReadInfo);
	}

	// If FAR/MIR found, exit
	if(nStatus == NoError)
	{
		Close();
		return iVersion;
	}

	// If records valid, exit
	if(bValidRecord)
	{
		Close();
		return 1;
	}

	// If no FAR/MIR found, try with different CPU type, unless non-standard CPU type
	if(ReadInfo.iCpuType == 1 )
	{
		SetStdfCpuType(2);
		Close();
	}
	else if(ReadInfo.iCpuType == 2)
	{
		SetStdfCpuType(1);
		Close();
	}
	else
	{
		Close();
		return 0;
	}

	// Now try with different CPU type
	// Open file
	hStdf = fopen(szStdfFile,"rb");
	if(hStdf == NULL)
		return -1;

	// Try to load first STDF record
	nStatus = LoadRecord(&ReadInfo);
	bValidRecord = IsValidStdfRecordType(&ReadInfo);
	if(nStatus == NoError)
	{
		*pbValidFileStart = true;
		Close();
		return iVersion;
	}
	// Go through STDF records until found FAR/MIR, or EOF, or Corrupted
	while((nStatus == SkipRecord) && bValidRecord)
	{
		nStatus = LoadRecord(&ReadInfo);
		bValidRecord = IsValidStdfRecordType(&ReadInfo);
	}

	// If FAR/MIR found, exit
	if(nStatus == NoError)
	{
		Close();
		return iVersion;
	}

	// If records valid, exit
	if(bValidRecord)
	{
		Close();
		return 1;
	}

	// Not valid STDF format
	Close();
	return 0;
}

///////////////////////////////////////////////////////////
// Action : Try to repair an STDF file with valid STDF records
// return :	true if file repaired, false else
///////////////////////////////////////////////////////////
bool CStdf::RepairStdfFile(const char *szStdfFile, const char *szStdfFile_Repaired, int nCpuType)
{
	int					nStatus;
	StdfRecordReadInfo	ReadInfo;
	bool				bValidRecord;

	/////////////////////////////////////////////////////////////////
	// 1) Open source STDF file, and skip all records until FAR
	/////////////////////////////////////////////////////////////////

	// If file already opened, close it
	if(hStdf != NULL)
		Close();

	// Open file
	hStdf = fopen(szStdfFile,"rb");
	if(hStdf == NULL)
		return false;

	// Set some global variables (Check validity during loads, access mode...)
	bStdfValidityCheck = true;
	iFileAccessMode = STDF_READ;
	SetStdfCpuType(nCpuType);

	// Try to load first STDF record
	nStatus = LoadRecord(&ReadInfo);
	bValidRecord = IsValidStdfRecordType(&ReadInfo);
	// Go through STDF records until found FAR/MIR, or EOF, or Corrupted
	while((nStatus == SkipRecord) && bValidRecord)
	{
		nStatus = LoadRecord(&ReadInfo);
		bValidRecord = IsValidStdfRecordType(&ReadInfo);
	}

	// If no FAR/MIR found, exit
	if((nStatus != NoError) || !bValidRecord)
	{
		Close();
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// 2) Copy all STDF records into destination file
	/////////////////////////////////////////////////////////////////
	
	// Open destination STDF
	CStdf clDestStdf;
	nStatus = clDestStdf.Open(szStdfFile_Repaired, STDF_WRITE);
	if(nStatus != NoError)
	{
		Close();
		return false;
	}

	// Write current and all remaining records
	clDestStdf.SetStdfCpuType(nCpuType);
	ReadInfo.iCpuType = iCpuType;		// Not set when FAR loaded
	ReadInfo.iStdfVersion = iVersion;	// Not set when FAR loaded
	while((nStatus == NoError) && bValidRecord)
	{
		clDestStdf.WriteRecord(&ReadInfo, this);
		nStatus = LoadRecord(&ReadInfo);
		bValidRecord = IsValidStdfRecordType(&ReadInfo);
	}

	// Close files and return
	Close();
	clDestStdf.Close();

	return true;
}

///////////////////////////////////////////////////////////
// Action : Tells file size.
// return :	NoError or error code
///////////////////////////////////////////////////////////
long	CStdf::GetFileSize(const char *szStdfFile)
{
	FILE	*stream;
	long	lSize;
	stream = fopen(szStdfFile, "rb");
	if (stream == NULL)
		return -1;	// error
	fseek(stream, 0L, SEEK_END); // fast forward to EOF
	lSize = ftell(stream);
	fclose(stream);
	return lSize;
}

///////////////////////////////////////////////////////////
// Action : Tells which record# is being read.
// return :	record#
///////////////////////////////////////////////////////////
long	CStdf::GetReadRecordNumber(void)
{
	return lRecRead;
}

///////////////////////////////////////////////////////////
// Action : Closes STDF file
// return :	NoError or error code
///////////////////////////////////////////////////////////
int	CStdf::Close(void)
{
	// Clear all variables.
	nReadOffset = 0;
	lRecRead = 0;
	nCacheReadOffset = 0;	// Move pointer back to origin of cache buffer
	lFilePos=0;

	if(hStdf == NULL)
		return NoError;
	// If Reading a STDF file, free the cache!
	if(hReadStdfCache != NULL)
	{
		free(hReadStdfCache);
	}
	else if(hStdfMem != NULL)
	{
		// Do not free the cache twice (throws exception under NT !)
		// So only free the buffer memory if no cache was involved !
		free(hStdfMem);
	}
	fclose(hStdf);
	hStdfMem = NULL;
	hReadStdfCache = NULL;
	hStdf = NULL;

	return	NoError;
}

///////////////////////////////////////////////////////////
// Action : none
// return :	position in STDF file processed.
///////////////////////////////////////////////////////////
long	CStdf::GetPos()
{
	return lFilePos;	// returns 
}

///////////////////////////////////////////////////////////
// Action : Read STDF record, manage cache buffer
// return :	Bytes in record to process.
///////////////////////////////////////////////////////////
long	CStdf::SmartFread(BYTE **ptRxBuffer, int iBlockSize)
{
	if(hReadStdfCache == NULL)
	{
		hReadStdfCache = (BYTE *) malloc(lCacheSize);	// 2MB cache memory !...unless custom value specified with 'open' 
		if(hReadStdfCache == NULL)
			return -1;	// Failed allocating cache !
		// Read 500K bytes or less if smaller STDF file.
		iBytesInCache = fread((BYTE*)hReadStdfCache,1,lCacheSize,hStdf);
		// Reset cache read position
		nCacheReadOffset = 0;
		lFilePos = 0;
	}
	// Check if full record to read is in cache.
	// Check what memory left in cache hasn't been read yet
	int	iUnreadCache = iBytesInCache-nCacheReadOffset;

	if(iBlockSize > iUnreadCache)
	{
		// Need to move cache content and read more data from disk !

		// Move it to the beginning of the cache memory
		memcpy((BYTE *)hReadStdfCache,(BYTE *)&hReadStdfCache[nCacheReadOffset],iUnreadCache);
		// Refill cache to max. possible size.
		iBytesInCache = iUnreadCache+ fread((BYTE *)&hReadStdfCache[iUnreadCache],1,lCacheSize-iUnreadCache,hStdf);
		// Reset cache read offest to its first byte.
		nCacheReadOffset=0;
		iUnreadCache = iBytesInCache;
	}

	// Check if requested record size is ok ?
	if(iBlockSize > iUnreadCache)
		 iBlockSize = iUnreadCache;
	// Returns pointer to STDF record in cache !
	*ptRxBuffer = &hReadStdfCache[nCacheReadOffset];
	// Update cache buffer read offset
	nCacheReadOffset += iBlockSize;

	// Update position in file (if no cache was involved !)
	lFilePos+= iBlockSize;

	// Tells number of byte copied from cache.
	return iBlockSize;
}

///////////////////////////////////////////////////////////
// Action : Tells current position in STDF record READ
//			in STDF_READ mode).
// return :	READ offset in record, 0 on error
///////////////////////////////////////////////////////////
long CStdf::GetReadRecordPos(void)
{
	return nReadOffset;
}

///////////////////////////////////////////////////////////
// Action : Overwrite current READ position in STDF record
// return :	NoError or error code
///////////////////////////////////////////////////////////
int	 CStdf::SetReadRecordPos(long nNewReadOffset)
{
  // Checks if new offset is valid (not over buffer size)
  if(nNewReadOffset >= nReadBufferSize)
	 return EndOfBuffer;	// Invalid offset !
  nReadOffset = nNewReadOffset;
  return NoError;
}

///////////////////////////////////////////////////////////
// Action : Gets one STDF record (requires file to be open
//			in STDF_READ mode).
// return :	NoError or error code
///////////////////////////////////////////////////////////
int	CStdf::LoadRecord(LPStdfRecordReadInfo pRecordReadInfo)
{
    int		iStatus=0, iLocalCpuType=0;
    iStatus = 0; iLocalCpuType = 0;

	// Check if file opened.
	if(hStdf == NULL)
		return FileClosed;

	// Check if file opened in READ mode
	if(iFileAccessMode != STDF_READ)
		return ErrorMode;

	// Read first 4 bytes of record from disk/network = Record Header
	iStatus = SmartFread(&hStdfMem,4);
	if(iStatus < 4)
	{
		if(iStatus == 0)
			return EndOfFile;	// End of file reached.
		else
		{
			Close();
			return Corrupted;	// File corrupted.
		}
	}

	int	iLsb = hStdfMem[0];
	int	iMsb = hStdfMem[1];
	pRecordReadInfo->iRecordType = hStdfMem[2];
	pRecordReadInfo->iRecordSubType = hStdfMem[3];
	pRecordReadInfo->iCpuType = iCpuType;
	pRecordReadInfo->iStdfVersion = iVersion;

	// If first record read in file, extract CPU type & STDF version
	if(bStdfValidityCheck && lRecRead == 0)
	{
		if(pRecordReadInfo->iRecordType == 0 && pRecordReadInfo->iRecordSubType == 10)
		{
			// FAR record
			nReadOffset = 0;	// Reset read offset index.
			iStatus = SmartFread(&hStdfMem,2);
			if(iStatus != 2)
				return EndOfFile;	// File corrupted.
			iLocalCpuType = hStdfMem[0];
			// If CPU type is custom, then treat it as a SUN CpuType.
			if(iLocalCpuType > 2)
				iLocalCpuType = 1;
			iVersion = hStdfMem[1];	

			// Use this function to set the CPU type, to make sure the bWriteSameCPU flag is correctly set
			SetStdfCpuType(iLocalCpuType);
			
			// Saves Record information.
			pRecordReadInfo->iRecordSize = 2;

			// Updates record count value
			lRecRead++;
			return	NoError;
		}

		if(pRecordReadInfo->iRecordType == 1 && pRecordReadInfo->iRecordSubType == 10)
		{
			// MIR record
			nReadOffset = 0;	// Reset read offset index.
			iStatus = SmartFread(&hStdfMem,2);
			if(iStatus != 2)
				return EndOfFile;	// File corrupted.
			if(hStdfMem[1] != 3)
				return EndOfFile;	// Do not support STDF file NOT starting with FAR (STDF V3/4) or MIR (STDF V3)

			iLocalCpuType = hStdfMem[0];
			// If CPU type is custom, then treat it as a SUN CpuType.
			if(iLocalCpuType > 2)
				iLocalCpuType = 1;
			iVersion = hStdfMem[1];	

			// Use this function to set the CPU type, to make sure the bWriteSameCPU flag is correctly set
			SetStdfCpuType(iLocalCpuType);

			if(iLocalCpuType == 1)
				nReadBufferSize = (iMsb) | (iLsb  << 8);	// CPU Type = SUN
			else
				nReadBufferSize = (iLsb) | (iMsb  << 8);	// CPU Type = PC,Sun386i,VAX
			nReadBufferSize -=2;

			// Saves Record information.
			pRecordReadInfo->iRecordSize = nReadBufferSize;

			// Reset read offset index.
			nReadOffset = 0;
			
			/* Read rest of record from disk/network */
			BYTE *ptBuffer = &hStdfMem[2];	// Pointing to Offset 2 in Read-Buffer
			if((long)SmartFread(&ptBuffer,nReadBufferSize) != nReadBufferSize)
			{
				Close();
				return Corrupted;		// Unexpected end of file.
			}

			// Updates record count value
			lRecRead++;
			return	NoError;
		}

		// Read record (assuming current CPU type is correct....)
		if(iCpuType == 1)
			nReadBufferSize = (iMsb) | (iLsb  << 8);	// CPU Type = SUN
		else
			nReadBufferSize = (iLsb) | (iMsb  << 8);	// CPU Type = PC,Sun386i,VAX

		// Saves Record information.
		pRecordReadInfo->iRecordSize = nReadBufferSize;

		// Reset read offset index.
		nReadOffset = 0;
		
		/* Read rest of record from disk/network */
		if((long)SmartFread(&hStdfMem,nReadBufferSize) != nReadBufferSize)
		{
			Close();
			return Corrupted;		// Unexpected end of file.
		}

		return SkipRecord;		// STDF file not starting with FAR or MIR....then ignore it and see next one.
	}

	if(iCpuType == 1)
		nReadBufferSize = (iMsb) | (iLsb  << 8);	// CPU Type = SUN
	else
		nReadBufferSize = (iLsb) | (iMsb  << 8);	// CPU Type = PC,Sun386i,VAX

	// Saves Record information.
	pRecordReadInfo->iRecordSize = nReadBufferSize;

	if(lRecRead == 0)
	{
		// Updates record count value
		lRecRead++;
		return NoError;
	}

	// Reset read offset index.
	nReadOffset = 0;
	
	/* Read rest of record from disk/network */
	if((long)SmartFread(&hStdfMem,nReadBufferSize) != nReadBufferSize)
	{
		Close();
		return Corrupted;		// Unexpected end of file.
	}

	// Updates record count value
	lRecRead++;
	return	NoError;
#if 0
	// If first record read in file, extract CPU type & STDF version
	if(lRecRead == 0)
	{
		// Read first 6 bytes of record from disk/network = FAR/MIR Record Header + 2 bytes
		iStatus = SmartFread(&hStdfMem,6);
		if(iStatus != 6)
		{
			Close();
			return Corrupted;	// File corrupted.
		}
		iLocalCpuType = hStdfMem[4];
		// If CPU type is custom, then treat it as a SUN CpuType.
		if(iLocalCpuType > 2)
			iLocalCpuType = 1;
		iVersion = hStdfMem[5];

		// Use this function to set the CPU type, to make sure the bWriteSameCPU flag is correctly set
		SetStdfCpuType(iLocalCpuType);
	}
	else
	{
		// Read first 4 bytes of record from disk/network = Record Header
		iStatus = SmartFread(&hStdfMem,4);
		if(iStatus < 4)
		{
			if(iStatus == 0)
				return EndOfFile;	// End of file reached.
			else
			{
				Close();
				return Corrupted;	// File corrupted.
			}
		}
	}


	// Extracts record size.
	if(iCpuType == 1)
		nReadBufferSize = ((int)hStdfMem[1]) | ((int)hStdfMem[0]  << 8);	// CPU Type = SUN
	else
		nReadBufferSize = ((int)hStdfMem[0]) | ((int)hStdfMem[1]  << 8);	// CPU Type = PC,Sun386i,VAX

	// Saves Record information.
	pRecordReadInfo->iRecordSize = nReadBufferSize;
	pRecordReadInfo->iRecordType = hStdfMem[2];
	pRecordReadInfo->iRecordSubType = hStdfMem[3];
	pRecordReadInfo->iCpuType = iCpuType;
	pRecordReadInfo->iStdfVersion = iVersion;

	if(lRecRead == 0)
	{
		// Updates record count value
		lRecRead++;
		return NoError;
	}

	// Reset read offset index.
	nReadOffset = 0;
	
	/* Read rest of record from disk/network */
	if((long)SmartFread(&hStdfMem,nReadBufferSize) != nReadBufferSize)
	{
		Close();
		return Corrupted;		// Unexpected end of file.
	}

	// Updates record count value
	lRecRead++;
	return	NoError;
#endif
}

///////////////////////////////////////////////////////////
// Check if record loaded is valid (holds valid recorrd type)
///////////////////////////////////////////////////////////
bool CStdf::IsValidStdfRecordType(StdfRecordReadInfo *StdfRecordHeader)
{
	switch(StdfRecordHeader->iRecordType)
	{
		case 0://	Information about the STDF file
			switch(StdfRecordHeader->iRecordSubType)
			{
				case 10://	File Attributes Record (FAR)
				case 20://	Audit Trail Record (ATR)
					return true;	// Valid record
			}
			break;

		case 1://	Data collected on a per lot basis
			switch(StdfRecordHeader->iRecordSubType)
			{
				case 10://	Master Information Record (MIR)
				case 20://	Master Results Record (MRR)
				case 30://	Part Count Record (PCR)
				case 40://	Hardware Bin Record (HBR)
				case 50://	Software Bin Record (SBR)
				case 60://	Pin Map Record (PMR)
				case 62://	Pin Group Record (PGR)
				case 63://	Pin List Record (PLR)
				case 70://	Retest Data Record (RDR)
				case 80://	Site Description Record (SDR)
					return true;	// Valid record
			}
			break;


		case 2://	Data collected per wafer
			switch(StdfRecordHeader->iRecordSubType)
			{
				case 10://	Wafer Information Record (WIR)
				case 20://	Wafer Results Record (WRR)
				case 30://	Wafer Configuration Record (WCR)
					return true;	// Valid record
			}
			break;

		case 5://	Data collected on a per part basis
			switch(StdfRecordHeader->iRecordSubType)
			{
				case 10://	Part Information Record (PIR)
				case 20://  Part Results Record (PRR)
					return true;	// Valid record
			}
			break;

		case 10://	Data collected per test in the test program
			switch(StdfRecordHeader->iRecordSubType)
			{
				case 30://	Test Synopsis Record (TSR)
					return true;	// Valid record
			}
			break;

		case 15://	Data collected per test execution
			switch(StdfRecordHeader->iRecordSubType)
			{
				case 10://	Parametric Test Record (PTR)
				case 15://	Multiple-Result Parametric Record (MPR)
				case 20://	Functional Test Record (FTR)
					return true;	// Valid record
			}
			break;

		case 20://	Data collected per program segment
			switch(StdfRecordHeader->iRecordSubType)
			{
				case 10://	Begin Program Section Record (BPS)
				case 20://	End Program Section Record (EPS)
					return true;	// Valid record
			}
			break;

		case 50://	Generic Data
			switch(StdfRecordHeader->iRecordSubType)
			{
				case 10://	Generic Data Record (GDR)
				case 30://	Datalog Text Record (DTR)
					return true;	// Valid record
			}
			break;


		case 180://	Reserved for use by Image software
		case 181://	Reserved for use by IG900 software	
			return true;	// Valid record
	}

	// Invalid record
	return false;
}

///////////////////////////////////////////////////////////
// Action : Reads one byte from STDF buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::ReadByte(BYTE *ptByte)
{
	if(nReadOffset >= nReadBufferSize)
		return EndOfBuffer;
  *ptByte = hStdfMem[nReadOffset];
  nReadOffset++;
  return NoError;
}

///////////////////////////////////////////////////////////
// Action : Overwrites one byte from STDF READ buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::OverwriteReadByte(BYTE cByte)
{
	if(nReadOffset >= nReadBufferSize)
		return EndOfBuffer;
  hStdfMem[nReadOffset] = cByte;
  nReadOffset++;
  return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one String from STDF buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::ReadString(char *szString)
{
  BYTE	bStringLength,i;

  *szString = 0;
  if(nReadOffset >= nReadBufferSize)
	 return EndOfBuffer;

  ReadByte(&bStringLength);
  bStringLength &= 0xff;
  for(i=0;i<bStringLength;i++){
	 szString[i] = 0;
	 ReadByte((BYTE *)&szString[i]);
  }

  szString[i] = 0;
  return NoError;
}

int	CStdf::ReadSNString(char *szString){
    int	bStringLength;
    unsigned int i;

    *szString = 0;
    if(nReadOffset >= nReadBufferSize)
	   return EndOfBuffer;

    ReadWord(&bStringLength);
    bStringLength &= 0xff;
    for(i=0;i<(unsigned int )bStringLength;i++){
	   szString[i] = 0;
	   ReadByte((BYTE *)&szString[i]);
    }

    szString[i] = 0;
    return NoError;
}

int	CStdf::ReadCFString(int iSize,char *szString){
    int i;

    *szString = 0;
    if(nReadOffset >= nReadBufferSize)
	   return EndOfBuffer;
    for(i=0;i<iSize;i++){
	   ReadByte((BYTE *)&szString[i]);
    }

    szString[i] = 0;
    return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one bit-encoded field from STDF buffer (size on 1 byte)
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::ReadBitField(BYTE *pbLength, char *pBitField)
{
  BYTE	i;

  *pBitField = 0;
  if(nReadOffset >= nReadBufferSize)
	 return EndOfBuffer;

  ReadByte(pbLength);
  *pbLength &= 0xff;
  for(i=0;i<*pbLength;i++)
	 ReadByte((BYTE *)&pBitField[i]);

  return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one double bit-encoded field from STDF buffer (size on 2 bytes)
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::ReadDBitField(unsigned int *pwLength, char *pBitField)
{
  unsigned int	i;
  int			iData;

  *pBitField = 0;
  if(nReadOffset >= nReadBufferSize)
	 return EndOfBuffer;

  ReadWord(&iData);
  if(iData == 0)
    *pwLength = 0;
  else
    *pwLength = (iData-1)/8 + 1;
  for(i=0;i<*pwLength;i++)
	 ReadByte((BYTE *)&pBitField[i]);

  return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one word (2 bytes) from STDF buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::ReadWord(int *ptWord)
{
  BYTE	bByte0=0,bByte1=0;
  int	iWord=0;

  if(nReadOffset >= nReadBufferSize)
	 return EndOfBuffer;

  ReadByte(&bByte0);
  ReadByte(&bByte1);
  bByte0 &= 0xff;
  bByte1 &= 0xff;
	if(iCpuType == 1)
	{
		// CPU Type = SUN
		iWord = ((int)bByte1) | ((int)bByte0  << 8);
	}
	else
	{
		// CPU Type = PC,Sun386i,VAX
		iWord = ((int)bByte0) | ((int)bByte1  << 8);
	}
	*ptWord = iWord;
	return NoError;
}

///////////////////////////////////////////////////////////
// Action : Overwrites two byte from STDF READ buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::OverwriteReadWord(WORD wData)
{
  if(nReadOffset >= nReadBufferSize)
	 return EndOfBuffer;

	LPSTR	ptData;
	ptData = (LPSTR)&wData;
	if(bWriteSameCpu)
	{
		// Examinator and STDF output have same CPU platform
		hStdfMem[nReadOffset++] = ptData[0];
		hStdfMem[nReadOffset++] = ptData[1];
	}
	else
	{
		// GEX running on a CPU different than the STDF output
		// Eg: Examinator running on a PC, and STDF output to be SUN-CPU
		// or Examinator running on a Sparc, and STDF output to be PC,Sun386i,VAX
		hStdfMem[nReadOffset++] = ptData[1];
		hStdfMem[nReadOffset++] = ptData[0];
	}
  return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one dword (4 bytes) from STDF buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::ReadDword(long *ptDword)
{
  int	iWord0=0,iWord1=0;
  long	lWord=0;

  if(nReadOffset >= nReadBufferSize)
	 return EndOfBuffer;

  ReadWord(&iWord0);
  ReadWord(&iWord1);
  iWord0 &= 0xffff;
  iWord1 &= 0xffff;
	if(iCpuType == 1)
	{
		// CPU Type = SUN
		lWord = ((long)iWord1) | ((long)iWord0 << 16);
	}
	else
	{
		// CPU Type = PC,Sun386i,VAX
		lWord = ((long)iWord0) | ((long)iWord1 << 16);
	}
	*ptDword = lWord;
  return NoError;	// Success
}

///////////////////////////////////////////////////////////
// Action : Overwrites two byte from STDF READ buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::OverwriteReadDword(DWORD dwData)
{
  if(nReadOffset >= nReadBufferSize)
	 return EndOfBuffer;

	LPSTR ptData = (LPSTR)&dwData;
	if(bWriteSameCpu)
	{
		// Examinator and STDF output have same CPU platform
		hStdfMem[nReadOffset++] = ptData[0];
		hStdfMem[nReadOffset++] = ptData[1];
		hStdfMem[nReadOffset++] = ptData[2];
		hStdfMem[nReadOffset++] = ptData[3];
	}
	else
	{
		// GEX running on a CPU different than the STDF output
		// Eg: Examinator running on a PC, and STDF output to be SUN-CPU
		// or Examinator running on a Sparc, and STDF output to be PC,Sun386i,VAX
		hStdfMem[nReadOffset++] = ptData[3];
		hStdfMem[nReadOffset++] = ptData[2];
		hStdfMem[nReadOffset++] = ptData[1];
		hStdfMem[nReadOffset++] = ptData[0];
	}
	return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one float (4 bytes) from STDF buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::ReadFloat(float *ptFloat)
{
	float	fData;
	BYTE	*bBuf2;
	BYTE	ptData[4];
	int		iIndex;

  if(nReadOffset >= nReadBufferSize)
	 return EndOfBuffer;

	bBuf2 = (BYTE *)&fData;
	for(iIndex=0;iIndex<4;iIndex++)
	 ReadByte((BYTE *)&ptData[iIndex]);

	switch(iCpuType)
	{
		case 0:	// CPU Type = VAX-PDP
			// VMS format is:
			// Bit 31-----16  15   14--------7 6-------0
			//  Matissa      Sign   Exponent    Fraction

			// Sign=1 means 'negative' number
			// Exponent (Bits 14-7): binary exponent in excess 128 notation 
			//       (binary exponents from -127 to +127 are represented by binary 1 to 255). 
			// Bits 6:0 and 31:16 are a normalized 24-bit fraction with the redundant most significant fraction bit not represented. 
			// The value of data is in the approximate range: 0.293873588E--38 to 1.7014117E38. 

				 // Check for overflow
			bBuf2[0] = ptData[3];	// Mantissa: Bits31-24
			bBuf2[1] = ptData[2];	// Mantissa: Bits23-16
			bBuf2[2] = ptData[1];	// Sign & Exponent
			bBuf2[3] = ptData[0];	// Fraction

			if(bBuf2[2] & 0x80)
			  ptData[3] = 0x80;	// Negative number
			else
			  ptData[3] = 0;	// Positive number
			ptData[3] |= bBuf2[2] & 0x7f;	// Exponent.
			ptData[2] = bBuf2[3];
			ptData[1] = bBuf2[0];
			ptData[0] = bBuf2[1];
			
			bBuf2[0] = ptData[0];
			bBuf2[1] = ptData[1];
			bBuf2[2] = ptData[2];
			bBuf2[3] = ptData[3];
			// Scales value (exponent) to be PC compatible!
			fData /= 4;
			break;

		case 1:	// CPU Type = SUN
#ifdef __sparc__
			// SUN-sparc CPU reading SUN float number
			bBuf2[0] = ptData[0];
			bBuf2[1] = ptData[1];
			bBuf2[2] = ptData[2];
			bBuf2[3] = ptData[3];
#else
			// PC-x86 CPU reading SUN float number
			bBuf2[0] = ptData[3];
			bBuf2[1] = ptData[2];
			bBuf2[2] = ptData[1];
			bBuf2[3] = ptData[0];
#endif
			break;

		case 2:	// CPU Type = PC,Sun386i
#ifdef __sparc__
			// SUN-sparc CPU reading PC float number
			bBuf2[0] = ptData[3];
			bBuf2[1] = ptData[2];
			bBuf2[2] = ptData[1];
			bBuf2[3] = ptData[0];
#else
			// PC-x86 CPU reading PC float number
			bBuf2[0] = ptData[0];
			bBuf2[1] = ptData[1];
			bBuf2[2] = ptData[2];
			bBuf2[3] = ptData[3];
#endif
			break;
	}
	
#ifdef __sparc__
	// Check if the read value is a INF or -INF value (all bits set in E, zero Fraction)
	if( ((bBuf2[0] & 0x7f) == 0x7f) && (bBuf2[1] & 0x80) &&									// E has all bits set
		((bBuf2[1] & 0x7f) == 0) && ((bBuf2[2] & 0xff) == 0) && ((bBuf2[3] & 0xff) == 0) )	// Zero Fraction
	{
		if(bBuf2[0] & 0x80)
			fData = -FLT_MAX_4BYTES;
		else
			fData = FLT_MAX_4BYTES;
	}
#else
	// Check if the read value is a INF or -INF value (all bits set in E, zero Fraction)
	if( ((bBuf2[3] & 0x7f) == 0x7f) && (bBuf2[2] & 0x80) &&									// E has all bits set
		((bBuf2[2] & 0x7f) == 0) && ((bBuf2[1] & 0xff) == 0) && ((bBuf2[0] & 0xff) == 0) )	// Zero Fraction
	{
		if(bBuf2[3] & 0x80)
			fData = -FLT_MAX_4BYTES;
		else
			fData = FLT_MAX_4BYTES;
	}
#endif

	// Added to avoid '-INF value' corrupt computation...& crash under Unix!
	if(fData >= FLT_MAX_4BYTES)
		fData = FLT_MAX_4BYTES;
	else
	if(fData <= -FLT_MAX_4BYTES)
		fData = -FLT_MAX_4BYTES;

	// [Simon]Fix behavior diff of Linux Win7 VS  WinXP
	if (fData== -0.0)
		fData = 0.0;

	*ptFloat = fData;
	return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one float (4 bytes) from STDF buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::ReadFloat(float *ptFloat, bool *pbIsNAN)
{
	float	fData;
	BYTE	*bBuf2;
	BYTE	ptData[4];
	int		iIndex;

	*pbIsNAN = false;

	if(nReadOffset >= nReadBufferSize)
		return EndOfBuffer;

	bBuf2 = (BYTE *)&fData;
	for(iIndex=0;iIndex<4;iIndex++)
		ReadByte((BYTE *)&ptData[iIndex]);

	switch(iCpuType)
	{
		case 0:	// CPU Type = VAX-PDP
			// VMS format is:
			// Bit 31-----16  15   14--------7 6-------0
			//  Matissa      Sign   Exponent    Fraction

			// Sign=1 means 'negative' number
			// Exponent (Bits 14-7): binary exponent in excess 128 notation 
			//       (binary exponents from -127 to +127 are represented by binary 1 to 255). 
			// Bits 6:0 and 31:16 are a normalized 24-bit fraction with the redundant most significant fraction bit not represented. 
			// The value of data is in the approximate range: 0.293873588E--38 to 1.7014117E38. 

				 // Check for overflow
			bBuf2[0] = ptData[3];	// Mantissa: Bits31-24
			bBuf2[1] = ptData[2];	// Mantissa: Bits23-16
			bBuf2[2] = ptData[1];	// Sign & Exponent
			bBuf2[3] = ptData[0];	// Fraction

			if(bBuf2[2] & 0x80)
			  ptData[3] = 0x80;	// Negative number
			else
			  ptData[3] = 0;	// Positive number
			ptData[3] |= bBuf2[2] & 0x7f;	// Exponent.
			ptData[2] = bBuf2[3];
			ptData[1] = bBuf2[0];
			ptData[0] = bBuf2[1];
			
			bBuf2[0] = ptData[0];
			bBuf2[1] = ptData[1];
			bBuf2[2] = ptData[2];
			bBuf2[3] = ptData[3];
			// Scales value (exponent) to be PC compatible!
			fData /= 4;
			break;

		case 1:	// CPU Type = SUN
#ifdef __sparc__
			// SUN-sparc CPU reading SUN float number
			bBuf2[0] = ptData[0];
			bBuf2[1] = ptData[1];
			bBuf2[2] = ptData[2];
			bBuf2[3] = ptData[3];
#else
			// PC-x86 CPU reading SUN float number
			bBuf2[0] = ptData[3];
			bBuf2[1] = ptData[2];
			bBuf2[2] = ptData[1];
			bBuf2[3] = ptData[0];
#endif
			break;

		case 2:	// CPU Type = PC,Sun386i
#ifdef __sparc__
			// SUN-sparc CPU reading PC float number
			bBuf2[0] = ptData[3];
			bBuf2[1] = ptData[2];
			bBuf2[2] = ptData[1];
			bBuf2[3] = ptData[0];
#else
			// PC-x86 CPU reading PC float number
			bBuf2[0] = ptData[0];
			bBuf2[1] = ptData[1];
			bBuf2[2] = ptData[2];
			bBuf2[3] = ptData[3];
#endif
			break;
	}
	
#ifdef __sparc__
	// Check if the read value is a INF or -INF value (all bits set in E, zero Fraction)
	if( ((bBuf2[0] & 0x7f) == 0x7f) && (bBuf2[1] & 0x80) &&									// E has all bits set
		((bBuf2[1] & 0x7f) == 0) && ((bBuf2[2] & 0xff) == 0) && ((bBuf2[3] & 0xff) == 0) )	// Zero Fraction
	{
		if(bBuf2[0] & 0x80)
			fData = -FLT_MAX_4BYTES;
		else
			fData = FLT_MAX_4BYTES;
	}

	// Check if read value is a NAN (all bits set in E and non-zero Fraction)
	if( ((bBuf2[0] & 0x7f) == 0x7f) && (bBuf2[1] & 0x80) &&									// E has all bits set
		((bBuf2[1] & 0x7f) || (bBuf2[2] & 0xff) || (bBuf2[3] & 0xff)) )						// Non-zero Fraction
	{
		*pbIsNAN = true;
	}
#else
	// Check if the read value is a INF or -INF value (all bits set in E, zero Fraction)
	if( ((bBuf2[3] & 0x7f) == 0x7f) && (bBuf2[2] & 0x80) &&									// E has all bits set
		((bBuf2[2] & 0x7f) == 0) && ((bBuf2[1] & 0xff) == 0) && ((bBuf2[0] & 0xff) == 0) )	// Zero Fraction
	{
		if(bBuf2[3] & 0x80)
			fData = -FLT_MAX_4BYTES;
		else
			fData = FLT_MAX_4BYTES;
	}

	// Check if read value is a NAN (all bits set in E and non-zero Fraction)
	if( ((bBuf2[3] & 0x7f) == 0x7f) && (bBuf2[2] & 0x80) &&									// E has all bits set
		((bBuf2[2] & 0x7f) || (bBuf2[1] & 0xff) || (bBuf2[0] & 0xff)) )						// Non-zero Fraction
	{
		*pbIsNAN = true;
	}
#endif

	// Added to avoid '-INF value' corrupt computation...& crash under Unix!
	if(fData >= FLT_MAX_4BYTES)
		fData = FLT_MAX_4BYTES;
	else
	if(fData <= -FLT_MAX_4BYTES)
		fData = -FLT_MAX_4BYTES;

	*ptFloat = fData;
	return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one double (8 bytes) from STDF buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CStdf::ReadDouble(double *ptDouble)
{
	double	lfData;
	BYTE	*bBuf8;
	BYTE	ptData[8];
	int		iIndex;

  if(nReadOffset >= nReadBufferSize)
	 return EndOfBuffer;

	bBuf8 = (BYTE *)&lfData;
	for(iIndex=0;iIndex<8;iIndex++)
	 ReadByte((BYTE *)&ptData[iIndex]);

	switch(iCpuType)
	{
		case 0:	// CPU Type = VAX-PDP
			// VMS double not supported. 
			lfData = 0.0;
			break;

		case 1:	// CPU Type = SUN
#ifdef __sparc__
			// SUN-sparc CPU reading SUN float number
			bBuf8[0] = ptData[0];
			bBuf8[1] = ptData[1];
			bBuf8[2] = ptData[2];
			bBuf8[3] = ptData[3];
			bBuf8[4] = ptData[4];
			bBuf8[5] = ptData[5];
			bBuf8[6] = ptData[6];
			bBuf8[7] = ptData[7];
#else
			// PC-x86 CPU reading SUN float number
			bBuf8[0] = ptData[7];
			bBuf8[1] = ptData[6];
			bBuf8[2] = ptData[5];
			bBuf8[3] = ptData[4];
			bBuf8[4] = ptData[3];
			bBuf8[5] = ptData[2];
			bBuf8[6] = ptData[1];
			bBuf8[7] = ptData[0];
#endif
			break;

		case 2:	// CPU Type = PC,Sun386i
#ifdef __sparc__
			// SUN-sparc CPU reading PC float number
			bBuf8[0] = ptData[7];
			bBuf8[1] = ptData[6];
			bBuf8[2] = ptData[5];
			bBuf8[3] = ptData[4];
			bBuf8[4] = ptData[3];
			bBuf8[5] = ptData[2];
			bBuf8[6] = ptData[1];
			bBuf8[7] = ptData[0];
#else
			// PC-x86 CPU reading PC float number
			bBuf8[0] = ptData[0];
			bBuf8[1] = ptData[1];
			bBuf8[2] = ptData[2];
			bBuf8[3] = ptData[3];
			bBuf8[4] = ptData[4];
			bBuf8[5] = ptData[5];
			bBuf8[6] = ptData[6];
			bBuf8[7] = ptData[7];
#endif
			break;
	}

	// Added to avoid '-INF value' corrupt computation...& crash under Unix!
	if(lfData >= DBL_MAX_8BYTES)
		lfData = DBL_MAX_8BYTES;
	else
	if(lfData <= -DBL_MAX_8BYTES)
		lfData = -DBL_MAX_8BYTES;

	*ptDouble = lfData;
	return NoError;
}

///////////////////////////////////////////////////////////
// Fuctions to Write STDF records
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// Defines the STDF output format.
///////////////////////////////////////////////////////////
void	CStdf::SetStdfCpuType(int iWriteCpuType)
{
	iCpuType = iWriteCpuType;
#ifdef __sparc__
	bWriteSameCpu = (iCpuType == 1) ? true: false;	// Cpu type: SUN-sparc
#else
	bWriteSameCpu = (iCpuType == 2) ? true: false;	// Cpu type: PC-x86
#endif
}

///////////////////////////////////////////////////////////
// Get CPU type
///////////////////////////////////////////////////////////
BYTE	CStdf::GetComputerCpuType(void)
{
#ifdef __sparc__
	return 1;	// Cpu type: SUN-sparc
#else
	return 2;	// Cpu type: PC-x86
#endif
}
///////////////////////////////////////////////////////////
// Initializes header of record.
///////////////////////////////////////////////////////////
int	CStdf::WriteHeader(LPStdfRecordReadInfo pRecordReadInfo)
{
	// Check if file opened.
	if(hStdf == NULL)
		return FileClosed;

	// Check if file opened in WRITE mode
	if(iFileAccessMode != STDF_WRITE)
		return ErrorMode;

	// Reset read offset index.
	nWriteOffset = GOFF_REC_DATA;

	// Default Write buffer size: 5K bytes
	nWriteBlockSize = lCacheSize;

	// Allocates Buffer to write to STDF.
	if(hStdfMem == NULL)
		hStdfMem= (BYTE *) malloc(nWriteBlockSize);
	
	if(hStdfMem == NULL)
	{
		Close();
		return ErrorMemory;	// Failed allocating buffer.
	}

	hStdfMem[2] = pRecordReadInfo->iRecordType;		// REC_TYPE field
	hStdfMem[3] = pRecordReadInfo->iRecordSubType;	// REC_SBTYPE field
	nWriteOffset=4;

	return NoError;
}

///////////////////////////////////////////////////////////
// Write stdf record to disk...
///////////////////////////////////////////////////////////
int	CStdf::WriteRecord(void)
{
	WORD	wRecSize;
    int		nWriteOffsetCopy;

	if(hStdfMem == NULL)
		return WriteError;

	// Record size doens't include REC-EN and REC_TYPE,REC_SUB
	wRecSize = nWriteOffset-4;

	// Computes record size...
   nWriteOffsetCopy = nWriteOffset;
   nWriteOffset = 0;
   WriteWord(wRecSize);	// Record size...check bytes swaping.
   nWriteOffset = nWriteOffsetCopy;

	// Write record to PC disk.
	if(fwrite(hStdfMem,nWriteOffset,1,hStdf) != 1)
		return WriteError;
	else
		return NoError;
}

///////////////////////////////////////////////////////////
// Write stdf record to disk, using data from another STDF object...
///////////////////////////////////////////////////////////
int	CStdf::WriteRecord(LPStdfRecordReadInfo pclStdfRecordReadInfo, CStdf *pclStdfSource)
{
	BYTE*	hSourceBuffer = pclStdfSource->GetMemoryHandle();
	BYTE	hHeaderBuffer[4];

	if(hSourceBuffer == NULL)
		return WriteError;

	// Write header to PC disk
	WriteWordOffset(pclStdfRecordReadInfo->iRecordSize, hHeaderBuffer, 0, bWriteSameCpu);
	hHeaderBuffer[2] = (BYTE)(pclStdfRecordReadInfo->iRecordType);
	hHeaderBuffer[3] = (BYTE)(pclStdfRecordReadInfo->iRecordSubType);
	if(fwrite(hHeaderBuffer,4,1,hStdf) != 1)
		return WriteError;

	// Write record to PC disk.
	if(pclStdfRecordReadInfo->iRecordSize)
	{
		if(fwrite(hSourceBuffer,pclStdfRecordReadInfo->iRecordSize,1,hStdf) != 1)
			return WriteError;
	}
	return NoError;
}

///////////////////////////////////////////////////////////
// Copy in stdf buffer a 4 bytes number.
///////////////////////////////////////////////////////////
int	CStdf::WriteDword(DWORD dwData)
{
	LPSTR	ptData;

	if(hStdfMem == NULL)
		return WriteError;

	ptData = (LPSTR)&dwData;
	if(bWriteSameCpu)
	{
		// Examinator and STDF output have same CPU platform
		hStdfMem[nWriteOffset++] = ptData[0];
		hStdfMem[nWriteOffset++] = ptData[1];
		hStdfMem[nWriteOffset++] = ptData[2];
		hStdfMem[nWriteOffset++] = ptData[3];
	}
	else
	{
		// GEX running on a CPU different than the STDF output
		// Eg: Examinator running on a PC, and STDF output to be SUN-CPU
		// or Examinator running on a Sparc, and STDF output to be PC,Sun386i,VAX
		hStdfMem[nWriteOffset++] = ptData[3];
		hStdfMem[nWriteOffset++] = ptData[2];
		hStdfMem[nWriteOffset++] = ptData[1];
		hStdfMem[nWriteOffset++] = ptData[0];
	}
	return NoError;
}

///////////////////////////////////////////////////////////
// Copy in stdf buffer a 2 bytes number.
///////////////////////////////////////////////////////////
int	CStdf::WriteWord(WORD wData)
{
	LPSTR	ptData;

	if(hStdfMem == NULL)
		return WriteError;

	ptData = (LPSTR)&wData;
	if(bWriteSameCpu)
	{
		// Examinator and STDF output have same CPU platform
		hStdfMem[nWriteOffset++] = ptData[0];
		hStdfMem[nWriteOffset++] = ptData[1];
	}
	else
	{
		// GEX running on a CPU different than the STDF output
		// Eg: Examinator running on a PC, and STDF output to be SUN-CPU
		// or Examinator running on a Sparc, and STDF output to be PC,Sun386i,VAX
		hStdfMem[nWriteOffset++] = ptData[1];
		hStdfMem[nWriteOffset++] = ptData[0];
	}
	return NoError;
}

///////////////////////////////////////////////////////////
// Copy in given buffer a 2 bytes number (specify offset in buffer)
///////////////////////////////////////////////////////////
int	CStdf::WriteWordOffset(WORD wData, BYTE *hBuffer, int nBufferOffset, bool bWriteSameCpu)
{
	LPSTR	ptData;

	if(hBuffer == NULL)
		return WriteError;

	ptData = (LPSTR)&wData;
	if(bWriteSameCpu)
	{
		// Examinator and STDF output have same CPU platform
		hBuffer[nBufferOffset++] = ptData[0];
		hBuffer[nBufferOffset++] = ptData[1];
	}
	else
	{
		// GEX running on a CPU different than the STDF output
		// Eg: Examinator running on a PC, and STDF output to be SUN-CPU
		// or Examinator running on a Sparc, and STDF output to be PC,Sun386i,VAX
		hBuffer[nBufferOffset++] = ptData[1];
		hBuffer[nBufferOffset++] = ptData[0];
	}
	return NoError;
}

///////////////////////////////////////////////////////////
// Action : Tells current position in STDF record WRITE
//			in STDF_READ mode).
// return :	READ offset in record, 0 on error
///////////////////////////////////////////////////////////
long CStdf::GetWriteRecordPos(void)
{
	return nWriteOffset;
}

///////////////////////////////////////////////////////////
// Action : Overwrite current READ position in STDF record
// return :	NoError or error code
///////////////////////////////////////////////////////////
int	 CStdf::SetWriteRecordPos(long nNewWriteOffset)
{
  // Checks if new offset is valid (not over buffer size)
  if(nNewWriteOffset >= lCacheSize)
	 return EndOfBuffer;	// Invalid offset !
  nWriteOffset = nNewWriteOffset;
  return NoError;
}

///////////////////////////////////////////////////////////
// Copy in stdf buffer a 1 byte number.
///////////////////////////////////////////////////////////
int	CStdf::WriteByte(BYTE bData)
{
	if(hStdfMem == NULL)
		return WriteError;

	hStdfMem[nWriteOffset++] = bData;
	return NoError;
}

///////////////////////////////////////////////////////////
// Copy in stdf buffer a 4 bytes float number.
///////////////////////////////////////////////////////////
int	CStdf::WriteFloat(float fData)
{
	BYTE	*ptData;

	if(hStdfMem == NULL)
		return WriteError;

	ptData = (BYTE *)&fData;
	if(bWriteSameCpu)
	{
		// Examinator and STDF output have same CPU platform
		hStdfMem[nWriteOffset++] = ptData[0];
		hStdfMem[nWriteOffset++] = ptData[1];
		hStdfMem[nWriteOffset++] = ptData[2];
		hStdfMem[nWriteOffset++] = ptData[3];
	}
	else
	{
		// GEX running on a CPU different than the STDF output
		// Eg: Examinator running on a PC, and STDF output to be SUN-CPU
		// or Examinator running on a Sparc, and STDF output to be PC,Sun386i,VAX
		hStdfMem[nWriteOffset++] = ptData[3];
		hStdfMem[nWriteOffset++] = ptData[2];
		hStdfMem[nWriteOffset++] = ptData[1];
		hStdfMem[nWriteOffset++] = ptData[0];
	}
	return NoError;
}

///////////////////////////////////////////////////////////
// Copy in stdf buffer a x bytes string (x on 1 byte).
///////////////////////////////////////////////////////////
int	CStdf::WriteString(const char * szString)
{
	BYTE			bLength;
	const char *	ptChar;

	if(hStdfMem == NULL)
		return WriteError;

	if(szString == NULL)
		bLength = 0;
	else
		bLength = strlen(szString);

	hStdfMem[nWriteOffset++] = bLength;	// string length.
	ptChar = szString;

	while(bLength)
	{
		hStdfMem[nWriteOffset++] = *ptChar;
		bLength--;
		ptChar++;
	}
	return NoError;
}


int	CStdf::WriteSNString(const char * szString){
    unsigned int 	bLength;
    const char *	ptChar;

    if(hStdfMem == NULL)
	    return WriteError;

    if(szString == NULL)
	    bLength = 0;
    else
	    bLength = strlen(szString);

    WriteWord((int)bLength);
    ptChar = szString;

    while(bLength)
    {
	    hStdfMem[nWriteOffset++] = *ptChar;
	    bLength--;
	    ptChar++;
    }
    return NoError;

}

int	CStdf::WriteCFString(const char * szString){

    unsigned int 	bLength;
    const char *	ptChar;

    if(hStdfMem == NULL)
	    return WriteError;

    if(szString == NULL)
	    bLength = 0;
    else
	    bLength = strlen(szString);

    ptChar = szString;

    while(bLength)
    {
	    hStdfMem[nWriteOffset++] = *ptChar;
	    bLength--;
	    ptChar++;
    }
    return NoError;
}

///////////////////////////////////////////////////////////
// Copy in stdf buffer a x bytes bit-encoded field (x on 1 byte).
///////////////////////////////////////////////////////////
int	CStdf::WriteBitField(BYTE bLength, char *pBitField)
{
	char *ptChar;

	if(hStdfMem == NULL)
		return WriteError;

	hStdfMem[nWriteOffset++] = bLength;	// string length.
	ptChar = pBitField;

	while(bLength)
	{
		hStdfMem[nWriteOffset++] = *ptChar;
		bLength--;
		ptChar++;
	}
	return NoError;
}

///////////////////////////////////////////////////////////
// Copy in stdf buffer a x bytes bit-encoded field (x on 2 bytes).
///////////////////////////////////////////////////////////
int	CStdf::WriteDBitField(unsigned int wLength, char *pBitField)
{
	char *ptChar;

	if(hStdfMem == NULL)
		return WriteError;

	WriteWord((WORD)wLength);				// String length
	ptChar = pBitField;

	while(wLength)
	{
		hStdfMem[nWriteOffset++] = *ptChar;
		wLength--;
		ptChar++;
	}
	return NoError;
}

///////////////////////////////////////////////////////////
// Copy in stdf buffer a x bytes string.
///////////////////////////////////////////////////////////
int	CStdf::WriteFixedString(LPSTR szString,int iFixedLength)
{
	BYTE	bLength;
	LPSTR	ptChar;

	if(hStdfMem == NULL)
		return WriteError;

	bLength = strlen(szString);
	iFixedLength -= bLength;
	ptChar = szString;

	while(bLength)
	{
		hStdfMem[nWriteOffset++] = *ptChar;
		bLength--;
		ptChar++;
	}

	// Fixed length string, must be completed with spaces.
	while(iFixedLength > 0)
	{
		hStdfMem[nWriteOffset++] = ' ';
		iFixedLength--;
	}
	return NoError;
}

///////////////////////////////////////////////////////////
// Dumps a STDF record (copy a READ buffer to a WRITE one)
///////////////////////////////////////////////////////////
int	CStdf::DumpRecord(LPStdfRecordReadInfo pRecordReadInfo,CStdf *pStdfRecord,bool bFullRecord/*=false*/)
{
	if(bFullRecord)
		return WriteRecord(pRecordReadInfo, pStdfRecord);

	int		iStatus;
	BYTE	bData;	// Temporary buffer when reading STDF record

	// Writes record header
	iStatus = WriteHeader(pRecordReadInfo);
	
	if(iStatus != NoError) return iStatus;
	do
	{
		if(pStdfRecord->ReadByte(&bData) != NoError)
			break;	// Reached end of record
		WriteByte(bData);
	}
	while(1);
	iStatus = WriteRecord();
	return iStatus;
}



int	CStdf::ReadQword( unsigned long long *pullQWData){

    int	iWord0,iWord1,iWord2,iWord3;
    unsigned long long	ullWord;

    if(nReadOffset >= nReadBufferSize)
	   return EndOfBuffer;

    ReadWord(&iWord0);
    ReadWord(&iWord1);
    ReadWord(&iWord2);
    ReadWord(&iWord3);
    iWord0 &= 0xffff;
    iWord1 &= 0xffff;
    iWord2 &= 0xffff;
    iWord3 &= 0xffff;

      if(iCpuType == 1)
      {
	      // CPU Type = SUN
	      ullWord = ((unsigned long long)iWord3) | ((unsigned long long)iWord2<<16) | ((unsigned long long)iWord1<<32) | ((unsigned long long)iWord0 << 48) ;//| ((unsigned long long)iWord1 << 16)  | ((unsigned long long)iWord1 << 16) ;
      }
      else
      {
	      // CPU Type = PC,Sun386i,VAX
	      ullWord = ((unsigned long long)iWord0) | ((unsigned long long)iWord1 << 16) | ((unsigned long long)iWord2 << 32) | ((unsigned long long)iWord3 << 48) ;
      }
	  *pullQWData = ullWord;
    return NoError;	// Success

}

int	CStdf::WriteQWord(unsigned long long ullData){

    LPSTR	ptData;

    if(hStdfMem == NULL)
	    return WriteError;

    ptData = (LPSTR)&ullData;
    if(bWriteSameCpu)
    {
	    // Examinator and STDF output have same CPU platform
	    hStdfMem[nWriteOffset++] = ptData[0];
	    hStdfMem[nWriteOffset++] = ptData[1];
	    hStdfMem[nWriteOffset++] = ptData[2];
	    hStdfMem[nWriteOffset++] = ptData[3];
	    hStdfMem[nWriteOffset++] = ptData[4];
	    hStdfMem[nWriteOffset++] = ptData[5];
	    hStdfMem[nWriteOffset++] = ptData[6];
	    hStdfMem[nWriteOffset++] = ptData[7];
    }
    else
    {
	    // GEX running on a CPU different than the STDF output
	    // Eg: Examinator running on a PC, and STDF output to be SUN-CPU
	    // or Examinator running on a Sparc, and STDF output to be PC,Sun386i,VAX
	    hStdfMem[nWriteOffset++] = ptData[7];
	    hStdfMem[nWriteOffset++] = ptData[6];
	    hStdfMem[nWriteOffset++] = ptData[5];
	    hStdfMem[nWriteOffset++] = ptData[4];
	    hStdfMem[nWriteOffset++] = ptData[3];
	    hStdfMem[nWriteOffset++] = ptData[2];
	    hStdfMem[nWriteOffset++] = ptData[1];
	    hStdfMem[nWriteOffset++] = ptData[0];
    }
    return NoError;
}
