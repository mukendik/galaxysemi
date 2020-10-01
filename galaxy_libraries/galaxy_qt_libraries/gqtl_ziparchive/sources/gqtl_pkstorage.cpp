// PkStorage.cpp: implementation of the CPkStorage class.
////////////////////////////////////////////////////////////////////////////////

// Project headers
#include "gqtl_pkzip.h"
#include "gqtl_pkstorage.h"
#include "gqtl_ansifile.h"

// Standard headers

// QT headers


//////////////////////////////////////////////////////////////////////
char CPkStorage::m_gszExtHeaderSignat[] = {0x50, 0x4b, 0x07, 0x08};
CPkStorage::CPkStorage()
{
	m_pCallbackData = NULL;
	m_iWriteBufferSize = 65535;
	m_iCurrentDisk = -1;
	m_pFile = NULL;
	m_bError = false;
}


//////////////////////////////////////////////////////////////////////
CPkStorage::~CPkStorage()
{

}


//////////////////////////////////////////////////////////////////////
DWORD CPkStorage::Read(void *pBuf, DWORD iSize)
{
	if(m_bError) return 0;

	if (iSize == 0)
		return 0;
	DWORD iRead = 0;
	iRead = m_pFile->Read(pBuf, iSize);
	if(iRead != iSize)
		Error(m_pFile->GetLastErrorMsg().toLatin1().data());
	return iRead;
}


//////////////////////////////////////////////////////////////////////
void CPkStorage::Open(const char* szPathName)
{
	if(m_bError) return;

	m_pWriteBuffer.Allocate(m_iWriteBufferSize); 
	m_uBytesInWriteBuffer = 0;
	m_bNewSpan = false;
	m_pFile = &m_internalfile;
	m_iSpanMode = suggestedAuto;

	OpenFile(szPathName, CAnsiFile::modeNoTruncate | CAnsiFile::modeRead);
		
}



//////////////////////////////////////////////////////////////////////
void CPkStorage::Open(CAnsiFile& mf)
{
	if(m_bError) return;

	m_pWriteBuffer.Allocate(m_iWriteBufferSize); 
	m_uBytesInWriteBuffer = 0;
	m_bNewSpan = false;
	m_pFile = &mf;

	mf.SeekToBegin();
	m_iSpanMode = suggestedAuto;
}



//////////////////////////////////////////////////////////////////////
void CPkStorage::ChangeDisk(int iNumber)
{
	if(m_bError) return;

	if (iNumber == m_iCurrentDisk)
		return;

	Q_ASSERT(m_iSpanMode != noSpan);
	m_iCurrentDisk = iNumber;
	OpenFile(ChangeTdRead().toLatin1().constData(),CAnsiFile::modeNoTruncate | CAnsiFile::modeRead);
}


//////////////////////////////////////////////////////////////////////
void CPkStorage::Error(char * msg)
{
	if((msg != NULL) && (strlen(msg) != 0))
		m_strErrorMsg = msg;
	else
		m_strErrorMsg = "Bad zip file";
	m_bError = true;
}


//////////////////////////////////////////////////////////////////////
bool CPkStorage::OpenFile(const char* lpszName, UINT uFlags)
{
	if(m_bError) return false;

	if(!m_pFile->Open(lpszName, uFlags | CAnsiFile::shareDenyWrite))
		Error(m_pFile->GetLastErrorMsg().toLatin1().data());
	return !m_bError;
}


//////////////////////////////////////////////////////////////////////
void CPkStorage::SetCurrentDisk(int iNumber)
{
	m_iCurrentDisk = iNumber;
}


//////////////////////////////////////////////////////////////////////
int CPkStorage::GetCurrentDisk()
{
	return m_iCurrentDisk;
}


//////////////////////////////////////////////////////////////////////
QString CPkStorage::ChangeTdRead()
{
	QString szTemp = GetTdVolumeName(m_iCurrentDisk == m_iTdSpanData);
	m_pFile->Close();
	return szTemp;
}


//////////////////////////////////////////////////////////////////////
void CPkStorage::Close()
{
	m_pFile->Close();


	m_pWriteBuffer.Release();
	m_iCurrentDisk = -1;
	m_iSpanMode = noSpan;
	m_pFile = NULL;
}


//////////////////////////////////////////////////////////////////////
QString CPkStorage::GetTdVolumeName(bool bLast, const char* lpszZipName)
{
	CAnsiFile cFileUtils;
        QString strFilePath = lpszZipName ? QString(lpszZipName) : m_pFile->GetFilePath();
	QString strName;
	QString strExt = cFileUtils.GetFileExt(strFilePath.toLatin1().constData());
	strName = strFilePath.left(strFilePath.length() - strExt.length());
	if (bLast)
		strExt = "zip";
	else
		strExt.sprintf("%.3d", m_iCurrentDisk);
	return strName + strExt;
}



//////////////////////////////////////////////////////////////////////
void CPkStorage::UpdateSpanMode(WORD uLastDisk)
{
	m_iCurrentDisk = uLastDisk;
	if (uLastDisk)
	{
		// disk spanning detected

		m_iSpanMode = tdSpan;

		m_iTdSpanData = uLastDisk; // disk with .zip extension ( the last one)
			
		m_pWriteBuffer.Release(); // no need for this in this case
	}
	else 
		m_iSpanMode = noSpan;

}


//////////////////////////////////////////////////////////////////////
DWORD CPkStorage::GetPosition()
{
	if(m_bError) return 0;

	return m_pFile->GetPosition() + m_uBytesInWriteBuffer;
}



//////////////////////////////////////////////////////////////////////
DWORD CPkStorage::GetFreeInBuffer()
{
	return m_pWriteBuffer.GetSize() - m_uBytesInWriteBuffer;
}
