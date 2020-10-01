///////////////////////////////////////////////////////////
// SPEKTRA class : allows to Read SPEKTRA files
///////////////////////////////////////////////////////////

// note: if compiling under 'console' mode include 'stdio.h' instead.
#include <stdio.h>

#if defined __APPLE__&__MACH__
#include <malloc/malloc.h>
#include <cstdlib>
#else
#include <malloc.h>
#endif
#include "cspektra.h"
#include <string.h>

///////////////////////////////////////////////////////////
// Constructor : reset all private variables.
///////////////////////////////////////////////////////////
CSpektra::CSpektra()
{
	m_hSpektra = NULL;
	m_hSpektraMem = NULL;
	m_nReadOffset = 0;
	m_lRecRead = 0;
	m_hReadSpektraCache = NULL;	// Cache buffer for very fast SPEKTRA Read functions !
	m_lCacheReadOffset = 0;		// Move pointer back to origin of cache buffer
	m_lFilePos=0;
}

///////////////////////////////////////////////////////////
// Destructor : Closes file if not done,
// reset all private variables.
///////////////////////////////////////////////////////////
CSpektra::~CSpektra()
{
	Close();	// Closes SPEKTRA file if not already done.
}

///////////////////////////////////////////////////////////
// Rewind SPEKTRA file.
///////////////////////////////////////////////////////////
void CSpektra::RewindFile(void)
{
	// Rewind file, so next 'LoadRecord' gets first record.
	// This function will not work if rewind is used after
	// more than one cache block has been read.
	m_lCacheReadOffset = 0;	// Move pointer back to origin of cache buffer
	m_lFilePos=0;
	//rewind(hSpektra);
}

///////////////////////////////////////////////////////////
// Action : Opens specified SPEKTRA file
//			Default cache size = 2Mega bytes...unless otherwise specified.
// return :	NoError or error code
///////////////////////////////////////////////////////////
int	CSpektra::Open(const char *szSpektraFile,long m_lCacheSizeValue)
{
	if(m_hSpektra != NULL)
		return FileOpened;	// File handle already in use...

	// If no cache size defined, set to default: 2Mega bytes
	if(m_lCacheSizeValue <= 0)
		m_lCacheSize = SPEKTRA_READ_CACHE;	// 2Mbytes.
	else
		m_lCacheSize = m_lCacheSizeValue;

	m_hSpektra = fopen(szSpektraFile,"rb");
	if(m_hSpektra == NULL)
		return ErrorOpen;
	
	return NoError; 
}

///////////////////////////////////////////////////////////
// Action : Tells file size.
// return :	NoError or error code
///////////////////////////////////////////////////////////
long CSpektra::GetFileSize(char *szSpektraFile)
{
	FILE	*stream;
	long	lSize;

	stream = fopen(szSpektraFile, "rb");
	if (stream == NULL)
		return -1;	// error

	fseek(stream, 0L, SEEK_END); // fast forward to EOF
	lSize = ftell(stream);
	fclose(stream);
	return lSize;
}

///////////////////////////////////////////////////////////
// Action : Closes SPEKTRA file
// return :	NoError or error code
///////////////////////////////////////////////////////////
int	CSpektra::Close(void)
{
	// Clear all variables.
	m_nReadOffset = 0;
	m_lRecRead = 0;
	m_lCacheReadOffset = 0;	// Move pointer back to origin of cache buffer
	m_lFilePos=0;

	if(m_hSpektra == NULL)
		return NoError;

	// If Reading a SPEKTRA file, free the cache!
	if(m_hReadSpektraCache != NULL)
		free(m_hReadSpektraCache);

	if(m_hSpektraMem != NULL)
	{
		// Do not free the cache twice (throws exception under NT !)
		// So only free the buffer memory if no cache was involved !
		if(m_hReadSpektraCache == NULL)
			free(m_hSpektraMem);
	}
	fclose(m_hSpektra);
	m_hSpektraMem = NULL;
	m_hReadSpektraCache = NULL;
	m_hSpektra = NULL;

	return	NoError;
}

///////////////////////////////////////////////////////////
// Action : Read SPEKTRA record, manage cache buffer
// return :	Bytes in record to process.
///////////////////////////////////////////////////////////
long CSpektra::SmartFread(BYTE **ptRxBuffer, int nBlockSize)
{
	if(m_hReadSpektraCache == NULL)
	{
		m_hReadSpektraCache = (BYTE *) malloc(m_lCacheSize);	// 2MB cache memory !...unless custom value specified with 'open' 
		if(m_hReadSpektraCache == NULL)
			return -1;	// Failed allocating cache !
		// Read 500K bytes or less if smaller SPEKTRA file.
		m_lBytesInCache = fread((BYTE*)m_hReadSpektraCache,1,m_lCacheSize,m_hSpektra);
		// Reset cache read position
		m_lCacheReadOffset = 0;
		m_lFilePos = 0;
	}
	// Check if full record to read is in cache.
	// Check what memory left in cache hasn't been read yet
	int	iUnreadCache = m_lBytesInCache-m_lCacheReadOffset;

	if(nBlockSize > iUnreadCache)
	{
		// Need to move cache content and read more data from disk !

		// Move it to the beginning of the cache memory
		memcpy((BYTE *)m_hReadSpektraCache,(BYTE *)&m_hReadSpektraCache[m_lCacheReadOffset],iUnreadCache);
		// Refill cache to max. possible size.
		m_lBytesInCache = iUnreadCache+ fread((BYTE *)&m_hReadSpektraCache[iUnreadCache],1,m_lCacheSize-iUnreadCache,m_hSpektra);
		// Reset cache read offest to its first byte.
		m_lCacheReadOffset=0;
		iUnreadCache = m_lBytesInCache;
	}

	// Check if requested record size is ok ?
	if(nBlockSize > iUnreadCache)
		 nBlockSize = iUnreadCache;
	// Returns pointer to SPEKTRA record in cache !
	*ptRxBuffer = &m_hReadSpektraCache[m_lCacheReadOffset];
	// Update cache buffer read offset
	m_lCacheReadOffset += nBlockSize;

	// Update position in file (if no cache was involved !)
	m_lFilePos+= nBlockSize;

	// Tells number of byte copied from cache.
	return nBlockSize;
}

///////////////////////////////////////////////////////////
// Action : Gets one SPEKTRA record (requires file to be open
//			in STDF_READ mode).
// return :	NoError or error code
///////////////////////////////////////////////////////////
int	CSpektra::LoadRecord(int nRecorSize)
{
	long lBytesRead;
	
	// Check if file opened.
	if(m_hSpektra == NULL)
		return FileClosed;

	// Set record size.
	m_nReadBufferSize = nRecorSize;

	// Reset read offset index.
	m_nReadOffset = 0;
	
	// Read rest of record from disk/network
	lBytesRead = SmartFread(&m_hSpektraMem,m_nReadBufferSize);

	if(lBytesRead == 0)
		return EndOfFile;

	if(lBytesRead != m_nReadBufferSize)
	{
		Close();
		return Corrupted;		// Unexpected end of file.
	}

	// Updates record count value
	m_lRecRead++;
	return	NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one byte from SPEKTRA buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CSpektra::ReadByte(BYTE* pbData)
{
	if(m_nReadOffset >= m_nReadBufferSize)
		return EndOfBuffer;

	*pbData = m_hSpektraMem[m_nReadOffset];
	m_nReadOffset++;
	return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one String from SPEKTRA buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CSpektra::ReadString(char* szString, int nLength)
{
	BYTE	i;

	*szString = 0;
	if(m_nReadOffset >= m_nReadBufferSize)
		return EndOfBuffer;

	for(i=0;i<nLength;i++)
		ReadByte((BYTE*)&szString[i]);

	szString[i] = 0;
	return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one word (2 bytes) from SPEKTRA buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CSpektra::ReadWord(WORD* pwData)
{
	BYTE	bByte0,bByte1=0;

	if(m_nReadOffset >= m_nReadBufferSize)
		return EndOfBuffer;

	ReadByte(&bByte0);
	ReadByte(&bByte1);
	bByte0 &= 0xff;
	bByte1 &= 0xff;

	// LSB first (in bByte0)
	*pwData = ((int)bByte0) | ((int)bByte1  << 8);

	return NoError;
}

///////////////////////////////////////////////////////////
// Action : Reads one dword (4 bytes) from SPEKTRA buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CSpektra::ReadDword(DWORD* pdwData)
{
	WORD	wWord0,wWord1;

	if(m_nReadOffset >= m_nReadBufferSize)
		return EndOfBuffer;

	ReadWord(&wWord0);
	ReadWord(&wWord1);
	wWord0 &= 0xffff;
	wWord1 &= 0xffff;

	// LSB first (in wWord0)
	*pdwData = ((long)wWord0) | ((long)wWord1 << 16);

	return NoError;	// Success
}

///////////////////////////////////////////////////////////
// Action : Reads one float (4 bytes) from SPEKTRA buffer
// return :	NoError or EndOfBuffer
///////////////////////////////////////////////////////////
int	CSpektra::ReadFloat(FLOAT* pfData)
{
	BYTE	*bBuf2;
	BYTE	ptData[4];
	int		iIndex;

	if(m_nReadOffset >= m_nReadBufferSize)
		return EndOfBuffer;

	bBuf2 = (BYTE *)pfData;
	for(iIndex=0;iIndex<4;iIndex++)
		ReadByte((BYTE *)&ptData[iIndex]);

#if defined unix || __MACH__
	// SUN CPU reading PC float number
	bBuf2[0] = ptData[3];
	bBuf2[1] = ptData[2];
	bBuf2[2] = ptData[1];
	bBuf2[3] = ptData[0];
#else
	// PC CPU reading PC float number
	bBuf2[0] = ptData[0];
	bBuf2[1] = ptData[1];
	bBuf2[2] = ptData[2];
	bBuf2[3] = ptData[3];
#endif

	// Added to avoid '-INF value' corrupt computation...& crash under Unix!
	if(*pfData >= FLT_MAX_4BYTES)
		*pfData = FLT_MAX_4BYTES;
	else
	if(*pfData <= -FLT_MAX_4BYTES)
		*pfData = -FLT_MAX_4BYTES;

	return NoError;
}

