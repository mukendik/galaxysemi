// PkFileHeader.cpp: implementation of the CPkFileHeader class.
////////////////////////////////////////////////////////////////////////////////

// Project headers
#include "gqtl_pkautobuffer.h"
#include "gqtl_pkfileheader.h"
#include "gqtl_pkzip.h"
//#include "gqtl_zlib.h"
#include <zlib.h>

// Standard headers

// QT headers

#define PkFileHeaderSIZE	46
#define LOCALPkFileHeaderSIZE	30

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
char CPkFileHeader::m_gszSignature[] = {0x50, 0x4b, 0x01, 0x02};
char CPkFileHeader::m_gszLocalSignature[] = {0x50, 0x4b, 0x03, 0x04};
CPkFileHeader::CPkFileHeader()
{
	m_uExternalAttr = FILE_ATTRIBUTE_ARCHIVE;
	m_uModDate = m_uModTime = m_uInternalAttr = 0;
	m_uMethod = Z_DEFLATED;

	m_bError = false;
}


//////////////////////////////////////////////////////////////////////
CPkFileHeader::~CPkFileHeader()
{

}

//////////////////////////////////////////////////////////////////////
bool CPkFileHeader::Error(char* msg)
{
	if((msg != NULL) && (strlen(msg) != 0))
		m_strErrorMsg = msg;
	else
		m_strErrorMsg = "Bad header";
	m_bError = true;
	return false;
}


//////////////////////////////////////////////////////////////////////
// read the header from the central dir
bool CPkFileHeader::Read(CPkStorage *pStorage)
{
	WORD uFileNameSize, uCommentSize, uExtraFieldSize;
	CPkAutoBuffer buf(PkFileHeaderSIZE);
    pStorage->Read(buf, PkFileHeaderSIZE);
    memcpy(&m_szSignature,		buf, 4);

	READ_WORD(m_uVersionMadeBy,buf,4);
	READ_WORD(m_uVersionNeeded,buf,6);
	READ_WORD(m_uFlag,buf,8);
	READ_WORD(m_uMethod,buf,10);
	READ_WORD(m_uModTime,buf,12);
	READ_WORD(m_uModDate,buf,14);
	READ_DWORD(m_uCrc32,buf,16);
	READ_DWORD(m_uComprSize,buf,20);
	READ_DWORD(m_uUncomprSize,buf,24);
	READ_WORD(uFileNameSize,buf,28);
	READ_WORD(uExtraFieldSize,buf,30);
	READ_WORD(uCommentSize,buf,32);
	READ_WORD(m_uDiskStart,buf,34);
	READ_WORD(m_uInternalAttr,buf,36);
	READ_DWORD(m_uExternalAttr,buf,38);
	READ_DWORD(m_uOffset,buf,42);

	// Check compatibility between Unix and Dos system
	// if the 3 first Bytes empty then convert
	#if 0  /// TO FIX OLD HIDEN BUG VIEW WITH GCC...
		if((m_uExternalAttr & 0x00000FFF) == 0)
			m_uExternalAttr = m_uExternalAttr >> 16;
	#endif

    buf.Release();

    if (memcmp(m_szSignature, m_gszSignature, 4) != 0)
        return Error();

	int iCurDsk = pStorage->GetCurrentDisk();
	m_pszFileName.Allocate(uFileNameSize); // don't add NULL at the end
	DWORD iRead = 0;
	iRead = pStorage->m_pFile->Read(m_pszFileName, uFileNameSize);
	if(iRead != uFileNameSize)
		return Error(pStorage->m_pFile->GetLastErrorMsg().toLatin1().data());
	if (uExtraFieldSize)
	{
		m_pExtraField.Allocate(uExtraFieldSize);
		iRead = pStorage->m_pFile->Read(m_pExtraField, uExtraFieldSize);
		if(iRead != uExtraFieldSize)
			return Error(pStorage->m_pFile->GetLastErrorMsg().toLatin1().data());
	}
	if (uCommentSize)
	{
		m_pszComment.Allocate(uCommentSize);
		iRead = pStorage->m_pFile->Read(m_pszComment, uCommentSize);
		if(iRead != uCommentSize)
			return Error(pStorage->m_pFile->GetLastErrorMsg().toLatin1().data());
	}

	return pStorage->GetCurrentDisk() == iCurDsk; // check that the while header is on the one disk
}


//////////////////////////////////////////////////////////////////////
// return CTime representation of m_uModDate, m_uModTime
QDateTime CPkFileHeader::GetTime()
{
	int day, month, year;
	int seconde, minute, hour;

	year = m_uModDate >> 9;
	month = (m_uModDate - (year << 9)) >> 5;
	day = (m_uModDate - (year << 9) - (month << 5));
	year += 1980;

	hour = m_uModTime >> 11;
	minute = (m_uModTime - (hour << 11)) >> 5;
	seconde = (m_uModTime - (hour << 11) - (minute << 5)) << 1;
	
	return QDateTime(QDate(year,month,day),QTime(hour,minute,seconde));
}



//////////////////////////////////////////////////////////////////////
// read local header
bool CPkFileHeader::ReadLocal(CPkStorage *pStorage, WORD& iLocExtrFieldSize)
{
	char buf[LOCALPkFileHeaderSIZE];
	pStorage->Read(buf, LOCALPkFileHeaderSIZE);
	if (memcmp(buf, m_gszLocalSignature, 4) != 0)
		return Error();

	bool bIsDataDescr = (((WORD)*(buf + 6)) & 8) != 0;

	WORD uFileNameSize = GetFileNameSize();
	WORD uLocalFlag, uLocalMethod, uLocalFileNameSize;

	READ_WORD(uLocalFlag,buf,6);
	READ_WORD(uLocalMethod,buf,8);
	READ_WORD(uLocalFileNameSize,buf,26);

	if ((m_uFlag != uLocalFlag)
		||(m_uMethod != uLocalMethod)
		|| (m_uMethod && (m_uMethod != Z_DEFLATED))
		|| (uFileNameSize != uLocalFileNameSize))
		return Error();

	if (!bIsDataDescr)
		if (!CheckCrcAndSizes(buf + 14))
			return Error();

	READ_WORD(iLocExtrFieldSize,buf,28);

	pStorage->m_pFile->Seek(uFileNameSize, CAnsiFile::current);

	return true;
}


//////////////////////////////////////////////////////////////////////
// set the m_uModDate, m_uModTime values using CTime object
void CPkFileHeader::SetTime(const QDateTime &time)
{
    WORD year = (WORD)time.date().year();
    if (year <= 1980)
		year = 0;
	else
		year -= 1980;
    m_uModDate = (WORD) (time.date().day() + (time.date().month() << 5) + (year << 9));
    m_uModTime = (WORD) ((time.time().second() >> 1) + (time.time().minute() << 5) + 
		(time.time().hour() << 11));
}


//////////////////////////////////////////////////////////////////////
// set the m_uModDate, m_uModTime values using CTime object
void CPkFileHeader::SetTime(time_t time)
{
	QDateTime qDateTime;
	qDateTime.setTime_t(time);
    WORD year = (WORD)qDateTime.date().year();
    if (year <= 1980)
		year = 0;
	else
		year -= 1980;
    m_uModDate = (WORD) (qDateTime.date().day() + (qDateTime.date().month() << 5) + (year << 9));
    m_uModTime = (WORD) ((qDateTime.time().second() >> 1) + (qDateTime.time().minute() << 5) + 
		(qDateTime.time().hour() << 11));
}



//////////////////////////////////////////////////////////////////////
//	the buffer contains crc32, compressed and uncompressed sizes to be compared 
//	with the actual values
bool CPkFileHeader::CheckCrcAndSizes(char *pBuf)
{
	DWORD uLocalCrc32, uLocalComprSize;

	READ_DWORD(uLocalCrc32,pBuf,0);
	READ_DWORD(uLocalComprSize,pBuf,4);
	READ_DWORD(m_uUncomprSize,pBuf,8);

    return (m_uCrc32 == uLocalCrc32) && (m_uComprSize == uLocalComprSize);
}


//////////////////////////////////////////////////////////////////////
DWORD CPkFileHeader::GetSize()
{
	return PkFileHeaderSIZE + GetExtraFieldSize() + GetFileNameSize() + GetCommentSize();
}



//////////////////////////////////////////////////////////////////////
bool CPkFileHeader::IsEncrypted()
{
	return ((m_uFlag & ((WORD)1)) != 0);
}


//////////////////////////////////////////////////////////////////////
bool CPkFileHeader::IsDataDescr()
{
	return ((m_uFlag & ((WORD)8)) != 0);
}


//////////////////////////////////////////////////////////////////////
bool CPkFileHeader::SetComment(const char* lpszComment)
{
	return CPkZip::WideToSingle(lpszComment, m_pszComment)	!= -1;
}


//////////////////////////////////////////////////////////////////////
QString CPkFileHeader::GetComment()
{
	QString temp;
	CPkZip::SingleToWide(m_pszComment, temp);
	return temp;

}


//////////////////////////////////////////////////////////////////////
bool CPkFileHeader::SetFileName(const char* lpszFileName)
{
	return CPkZip::WideToSingle(lpszFileName, m_pszFileName) != -1;
}


//////////////////////////////////////////////////////////////////////
UINT CPkFileHeader::GetFileAttribute()
{
	UINT nAccessFlag = 0;
	nAccessFlag |= 0000444;//S_IREAD;
	if(!(m_uExternalAttr & FILE_ATTRIBUTE_READONLY))
		nAccessFlag |= 0000222;//S_IWRITE;
	if(m_uExternalAttr & FILE_ATTRIBUTE_NORMAL)
		nAccessFlag |= S_IFREG;
	if(m_uExternalAttr & FILE_ATTRIBUTE_DIRECTORY)
		nAccessFlag |= S_IFDIR;
	else
		nAccessFlag |= S_IFREG;
	return nAccessFlag;
}


//////////////////////////////////////////////////////////////////////
QString CPkFileHeader::GetFileName()
{
	QString temp;
	CPkZip::SingleToWide(m_pszFileName, temp);
	return temp;
}



//////////////////////////////////////////////////////////////////////
void CPkFileHeader::SlashChange()
{
	m_pszFileName.SlashChange();
}


//////////////////////////////////////////////////////////////////////
void CPkFileHeader::AnsiOem(bool bAnsiToOem)
{
	if (bAnsiToOem)
		return;//CharToOemBuffA(m_pszFileName, m_pszFileName, m_pszFileName.GetSize());
	else
		return;//OemToCharBuffA(m_pszFileName, m_pszFileName, m_pszFileName.GetSize());
}
