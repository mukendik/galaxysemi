// PkCentralDir.cpp: implementation of the CPkCentralDir class.
////////////////////////////////////////////////////////////////////////////////

// Project headers
#include "gqtl_pkcentraldir.h"
#include "gqtl_pkzip.h"

// Standard headers

// QT headers

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
char CPkCentralDir::m_gszSignature[] = {0x50, 0x4b, 0x05, 0x06};
CPkCentralDir::CPkCentralDir()
{
	m_bConvertAfterOpen  = true;
	m_pStorage = NULL;
	m_pOpenedFile = NULL;
	m_iBufferSize = 32768;
	m_bError = false;
	
}

//////////////////////////////////////////////////////////////////////
void CPkCentralDir::Init()
{
	m_bOnDisk = false;
	m_uBytesBeforeZip = m_uCentrDirPos = 0;
	m_pOpenedFile = NULL;
	m_pszComment.Release();
	m_bError = false;
}

//////////////////////////////////////////////////////////////////////
CPkCentralDir::~CPkCentralDir()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
bool CPkCentralDir::Read()
{
	if(m_bError) return false;

	Q_ASSERT(m_pStorage);
	WORD uCommentSize;
	m_uCentrDirPos = Locate();
	m_pStorage->m_pFile->Seek(m_uCentrDirPos, CAnsiFile::begin);
	CPkAutoBuffer buf(WINCENTRALDIRSIZE);
	DWORD uRead = m_pStorage->m_pFile->Read(buf, WINCENTRALDIRSIZE );
	if (uRead != WINCENTRALDIRSIZE)
		return Error(m_pStorage->m_pFile->GetLastErrorMsg().toLatin1().data());

	memcpy(&m_szSignature,		buf, 4);

	READ_WORD(m_uThisDisk,buf,4);
	READ_WORD(m_uDiskWithCD,buf,6);
	READ_WORD(m_uDiskEntriesNo,buf,8);
	READ_WORD(m_uEntriesNumber,buf,10);
	READ_DWORD(m_uSize,buf,12);
	READ_DWORD(m_uOffset,buf,16);
	READ_WORD(uCommentSize,buf,20);

	buf.Release();


	m_pStorage->UpdateSpanMode(m_uThisDisk);
	// if m_uThisDisk is not zero, it is enough to say that it is multi archive
	Q_ASSERT((!m_uThisDisk && (m_uEntriesNumber == m_uDiskEntriesNo) && !m_uDiskWithCD) || m_uThisDisk);
	if((m_uThisDisk == 0) && (m_uDiskWithCD == 0) && (m_uDiskEntriesNo == 0) && (m_uEntriesNumber == 0) && (m_uSize == 0) && (m_uOffset == 0) && (uCommentSize == 0))
	{
		// No error
		// Is an empty compressed file
		return true;
	}
			

	if (uCommentSize)
	{
		m_pszComment.Allocate(uCommentSize);
		uRead = m_pStorage->m_pFile->Read(m_pszComment, uCommentSize);
		if (uRead != uCommentSize)
			return Error(m_pStorage->m_pFile->GetLastErrorMsg().toLatin1().data());
	}
	
	m_uBytesBeforeZip = m_uCentrDirPos - m_uSize - m_uOffset;

	if ((!m_uSize && m_uEntriesNumber) || (!m_uEntriesNumber && m_uSize))
		return Error();

	m_bOnDisk = true;
	m_pStorage->ChangeDisk(m_uDiskWithCD);

	if (!m_uSize)
		return false;

	ReadHeaders();
	return true;
}

//////////////////////////////////////////////////////////////////////
// return the location of the beginning of the "end" record in the file
DWORD CPkCentralDir::Locate()
{

	if(m_bError) return 0;

	// maximum size of end of central dir record
	long uMaxRecordSize = 0xffff + WINCENTRALDIRSIZE;
	DWORD uFileSize = m_pStorage->m_pFile->GetLength();

	if ((DWORD)uMaxRecordSize > uFileSize)
		uMaxRecordSize = uFileSize;

	CPkAutoBuffer buf(m_iBufferSize);

	long uPosInFile = 0;
	int uRead = 0;
	// backward reading
	while (uPosInFile < uMaxRecordSize)
	{
		uPosInFile = uRead + m_iBufferSize;
		if (uPosInFile > uMaxRecordSize)
			uPosInFile = uMaxRecordSize;

		int iToRead = uPosInFile - uRead;

		m_pStorage->m_pFile->Seek(-uPosInFile, CAnsiFile::end);
		int iActuallyRead = m_pStorage->m_pFile->Read(buf, iToRead);
		if (iActuallyRead != iToRead)
			return Error(m_pStorage->m_pFile->GetLastErrorMsg().toLatin1().data());

		// search from the very last bytes to prevent an error if inside archive 
		// there are packed other arhives
		for (int i = iToRead - 4; i >=0 ; i--)
			if (!memcmp((char*)buf + i, m_gszSignature, 4))	
				return uFileSize - (uPosInFile - i);

		uRead += iToRead - 3;

	}
	
	return Error();
}

//////////////////////////////////////////////////////////////////////
bool CPkCentralDir::Error(char * msg)
{
	if((msg != NULL) && (strlen(msg) != 0))
		m_strErrorMsg = msg;
	else 
		m_strErrorMsg = "Bad zip file";
	m_bError = true;
	return false;
}


//////////////////////////////////////////////////////////////////////
bool CPkCentralDir::ReadHeaders()
{
	if(m_bError) return false;

	m_pStorage->m_pFile->Seek(m_uOffset + m_uBytesBeforeZip, CAnsiFile::begin);
	if(m_pStorage->m_pFile->m_bError)
		return Error(m_pStorage->m_pFile->GetLastErrorMsg().toLatin1().data());

	RemoveHeaders();
	for (int i = 0; i < m_uEntriesNumber; i++)
	{
		CPkFileHeader* pHeader = new CPkFileHeader;
		m_headers.append(pHeader);

		if (!pHeader->Read(m_pStorage))
			return Error(pHeader->GetLastErrorMsg().toLatin1().data());
		ConvertFileName(true, true, pHeader);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// remove all headers from the central dir
void CPkCentralDir::Clear(bool bEverything)
{
	m_pOpenedFile = NULL;
	m_pLocalExtraField.Release();

	if (bEverything)
	{
		RemoveHeaders();
		m_pszComment.Release();
		
	}
}


//////////////////////////////////////////////////////////////////////
bool CPkCentralDir::IsValidIndex(WORD uIndex)
{

	return uIndex < m_headers.count();
}

//////////////////////////////////////////////////////////////////////
// open the file for extracting
bool CPkCentralDir::OpenFile(WORD uIndex)
{
	if(m_bError) return false;

	WORD uLocalExtraFieldSize;
	m_pOpenedFile = m_headers.at(uIndex);
	m_pStorage->ChangeDisk(m_pOpenedFile->m_uDiskStart);
	m_pStorage->m_pFile->Seek(m_pOpenedFile->m_uOffset + m_uBytesBeforeZip, CAnsiFile::begin);	
	if (!m_pOpenedFile->ReadLocal(m_pStorage, uLocalExtraFieldSize))
		return Error(m_pOpenedFile->GetLastErrorMsg().toLatin1().data());


	m_pLocalExtraField.Release(); // just in case
	if (uLocalExtraFieldSize)
	{
		int iCurrDsk = m_pStorage->GetCurrentDisk();
		m_pLocalExtraField.Allocate(uLocalExtraFieldSize);
		m_pStorage->Read(m_pLocalExtraField, uLocalExtraFieldSize);
		if (m_pStorage->GetCurrentDisk() != iCurrDsk)
			return Error(m_pStorage->GetLastErrorMsg().toLatin1().data());
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CPkCentralDir::CloseFile()
{
	if(m_bError) return false;

	if (!m_pOpenedFile)	return false;
	
	m_pLocalExtraField.Release();
	if (m_pOpenedFile->IsDataDescr())
	{
		CPkAutoBuffer buf(12);
		m_pStorage->Read(buf, 4);
		// in span mode, files that are divided between disks have bit 3 of flag set
		// which tell about the presence of the data descriptor after the compressed data
		// This signature may be in the disk spanning archive that is one volume only
		// (it is detected as a non disk spanning archive)
		if (memcmp(buf, CPkStorage::m_gszExtHeaderSignat, 4) != 0) // there is no signature
				m_pStorage->m_pFile->Seek(-4, CAnsiFile::current);

		
		m_pStorage->Read(buf, 12);
		if (!m_pOpenedFile->CheckCrcAndSizes(buf))
			return Error(m_pOpenedFile->GetLastErrorMsg().toLatin1().data());
	}
	m_pOpenedFile = NULL;
	return true;
}

//////////////////////////////////////////////////////////////////////
DWORD CPkCentralDir::GetSize(bool bWhole)
{
	DWORD uHeaders = 0;
	if (bWhole)
	{
		for (int i = 0; i < (int)m_headers.count(); i++)
			uHeaders += m_headers.at(i)->GetSize();
	}
	return WINCENTRALDIRSIZE + m_pszComment.GetSize() + uHeaders;
}


//////////////////////////////////////////////////////////////////////
void CPkCentralDir::RemoveHeaders()
{
		for (int i = 0; i < (int)m_headers.count(); i++)
			delete m_headers.at(i);
		m_headers.clear();
}

//////////////////////////////////////////////////////////////////////
void CPkCentralDir::ConvertAll()
{
	Q_ASSERT(!m_bConvertAfterOpen);
	for (int i = 0; i < (int)m_headers.count(); i++)
		ConvertFileName(true, false, m_headers.at(i));
	m_bConvertAfterOpen = true;
}


