// PkZip.cpp: implementation of the CPkZip class.
////////////////////////////////////////////////////////////////////////////////

// Project headers
#include "gqtl_pkzip.h"

// Standard headers
#include <time.h>
#include <stdlib.h>
#include <stdio.h> // now mandatory for remove(file) method

// QT headers
#include <QApplication>
#include <QDateTime>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CPkZip::CPkZip(QString strWorkDir, bool bFullPath, bool bUniqueName)
{
	m_strWorkDir = strWorkDir;
	m_bFullPath = bFullPath;
	m_bUniqueName = !m_bFullPath && bUniqueName;

	m_bDetectZlibMemoryLeaks = false;
	m_centralDir.m_pStorage= &m_storage;
	m_info.m_stream.zalloc = (alloc_func)myalloc;
	m_info.m_stream.zfree = (free_func)myfree;
	m_iFileOpened = nothing;

	m_bError = false;
}



//////////////////////////////////////////////////////////////////////
CPkZip::~CPkZip()
{
    EmptyPtrList();
}

//////////////////////////////////////////////////////////////////////
bool CPkZip::IsPkZipFile(QString strZipFile)
{
	bool bIsZip = false;
	char buf[32768];
	CAnsiFile cFile;
	if(!cFile.Open(strZipFile, CAnsiFile::modeNoTruncate | CAnsiFile::modeRead))
		return false;

	if((cFile.Read(buf, 2) < 2)
	|| (memcmp((char*)buf, CPkCentralDir::m_gszSignature, 2) != 0))
	{
		cFile.Close();
		return false;
	}

	// maximum size of end of central dir record
	long uMaxRecordSize = 0xffff + WINCENTRALDIRSIZE;
	DWORD uFileSize = cFile.GetLength();

	if ((DWORD)uMaxRecordSize > uFileSize)
		uMaxRecordSize = uFileSize;

	long uPosInFile = 0;
	int uRead = 0;
	// backward reading
	while((uPosInFile < uMaxRecordSize) && !bIsZip)
	{
		uPosInFile = uRead + 32768;
		if (uPosInFile > uMaxRecordSize)
			uPosInFile = uMaxRecordSize;

		int iToRead = uPosInFile - uRead;

		cFile.Seek(-uPosInFile, CAnsiFile::end);
		int iActuallyRead = cFile.Read(buf, iToRead);
		if (iActuallyRead != iToRead)
			break;
		// search from the very last bytes to prevent an error if inside archive 
		// there are packed other arhives
		for (int i = iToRead - 4; i >=0 ; i--)
			if (!memcmp((char*)buf + i, CPkCentralDir::m_gszSignature, 4))	
			{
				bIsZip = true;
				break;
			}

		uRead += iToRead - 3;

	}
	cFile.Close();
	return bIsZip;
}


//////////////////////////////////////////////////////////////////////
bool CPkZip::ExtractFiles(QString strZipFile, QStringList& lstToFile, bool lListOnly /* = false */)
{
	QString strWorkDir = m_strWorkDir;

	if((strWorkDir.isEmpty())
	|| (m_cFileUtils.FileExists(strWorkDir.toLatin1().constData()) > -1))
		strWorkDir = m_cFileUtils.GetFilePath(strZipFile.toLatin1().constData());

	if(!Open(strZipFile.toLatin1().constData()))
		return false;

	WORD i;
	
	for (i = 0; i < GetNoEntries(); i++)
	{
		if(m_bError) return false;
		
		CPkFileHeader fh;
		GetFileInfo(fh, i);
		QString strFileName = m_cFileUtils.NormalizePath(fh.GetFileName().toLatin1().constData());

        if (lListOnly)
        {
            if(!IsFileDirectory(i))
                lstToFile.append(strFileName);
        }
        else
        {
            if(!m_bFullPath)
            {
                if(m_bUniqueName)
                    strFileName.replace(CHAR_SEP,'_');
                else
                    strFileName = m_cFileUtils.GetFileName(strFileName.toLatin1().constData());
            }

            if(ExtractFile(i,strWorkDir.toLatin1().constData(),strFileName.toLatin1().constData()))
            {
                if(!IsFileDirectory(i))
                    lstToFile.append(strFileName);
            }
            else
                return false;
        }
	}
	Close();

	return true;
}


//////////////////////////////////////////////////////////////////////
bool CPkZip::Open(const char* szPathName)
{
	if (!IsClosed())
		return false;

	m_iFileOpened = nothing;
	m_storage.Open(szPathName);
	if(m_storage.m_bError)
		return Error(m_storage.GetLastErrorMsg());
	m_centralDir.Init();
	if(!m_centralDir.Read())
		return Error(m_centralDir.GetLastErrorMsg());
	return true;
}


//////////////////////////////////////////////////////////////////////
bool CPkZip::Open(CAnsiFile& mf)
{
	if (!IsClosed())
		return false;

	m_iFileOpened = nothing;
	m_storage.Open(mf);
	if(m_storage.m_bError)
		return Error(m_storage.GetLastErrorMsg());
	m_centralDir.Init();
	if(!m_centralDir.Read())
		return Error(m_centralDir.GetLastErrorMsg());
	return true;
}



//////////////////////////////////////////////////////////////////////
bool CPkZip::IsClosed(bool bArchive)
{
  return ! bArchive ?
    (m_storage.GetCurrentDisk() == -1) :
    (!m_storage.m_pFile ||
     m_storage.m_pFile->m_hFile == (UINT) CAnsiFile::hFileNull);
}



//////////////////////////////////////////////////////////////////////
bool CPkZip::Error(char* msg)
{
	if((msg != NULL) && (strlen(msg) != 0))
		m_strErrorMsg = msg;
	else
		m_strErrorMsg = "Bad zip file";
	m_bError = true;
	return false;
}


//////////////////////////////////////////////////////////////////////
bool CPkZip::Error(QString strMsg)
{
	if(!strMsg.isEmpty())
		m_strErrorMsg = strMsg;
	else
		m_strErrorMsg = "Bad zip file";
	m_bError = true;
	return false;
}

//////////////////////////////////////////////////////////////////////
int CPkZip::GetNoEntries()
{
	return m_centralDir.m_headers.count();
}



//////////////////////////////////////////////////////////////////////
bool CPkZip::GetFileInfo(CPkFileHeader & fhInfo, WORD uIndex)
{
	if (IsClosed())
		return false;
	
	if (!m_centralDir.IsValidIndex(uIndex))
		return false;
	
	fhInfo = *(m_centralDir.m_headers.at(uIndex));
	m_centralDir.ConvertFileName(true, false, &fhInfo);
	return true;
}


//////////////////////////////////////////////////////////////////////
bool CPkZip::OpenFile(WORD uIndex)
{
	if(m_bError) return false;

	if (!m_centralDir.IsValidIndex(uIndex))
		return false;
	
	if (m_iFileOpened)
		return false;

	m_info.Init();

	m_centralDir.OpenFile(uIndex);

    // debug case 5480
    CPkFileHeader* cpfhPtrCurrentFile = CurrentFile();
    if(!cpfhPtrCurrentFile)
        return false;

    //if (CurrentFile()->IsEncrypted())
    if (cpfhPtrCurrentFile->IsEncrypted())
		return Error((char*)"Encrypted file");
	
    //WORD uMethod = CurrentFile()->m_uMethod;
    WORD uMethod = cpfhPtrCurrentFile->m_uMethod;
	if ((uMethod != 0) &&(uMethod != Z_DEFLATED))
		return Error();
			
	if (uMethod == Z_DEFLATED)
	{
		m_info.m_stream.opaque =  m_bDetectZlibMemoryLeaks ? &m_list : 0;
		int err = inflateInit2(&m_info.m_stream, -MAX_WBITS);
		if(!CheckForError(err))
			return false;
	}
//	m_info.m_uComprLeft = CurrentFile()->m_uComprSize;
//	m_info.m_uUncomprLeft = CurrentFile()->m_uUncomprSize;
    m_info.m_uComprLeft = cpfhPtrCurrentFile->m_uComprSize;
    m_info.m_uUncomprLeft = cpfhPtrCurrentFile->m_uUncomprSize;
	m_info.m_uCrc32 = 0;
	m_info.m_stream.total_out = 0;
	m_info.m_stream.avail_in = 0;
	
	m_iFileOpened = extract;
	return true;
}



//////////////////////////////////////////////////////////////////////
void* CPkZip::myalloc(void* opaque, UINT items, UINT size)
{
	void* p = new char[size * items];
	if (opaque)
	{
		QList<void*>* list  = (QList<void*>*) opaque;
		list->append(p);
	}
	return p;
}


//////////////////////////////////////////////////////////////////////
void CPkZip::myfree(void* opaque, void* address)
{
	if (opaque)
	{
		QList<void*>* list  = (QList<void*>*) opaque;
		int iPos = list->indexOf(address);
		if (iPos)
			list->removeAt(iPos);
	}
	delete[] (char*) address;
}


//////////////////////////////////////////////////////////////////////
bool CPkZip::CheckForError(int iErr)
{
	if ((iErr == Z_OK) ||(iErr == Z_NEED_DICT))
		return true;
	
	return Error(m_info.m_stream.msg);
}


//////////////////////////////////////////////////////////////////////
CPkFileHeader* CPkZip::CurrentFile()
{
	return m_centralDir.m_pOpenedFile;
}


//////////////////////////////////////////////////////////////////////
DWORD CPkZip::ReadFile(void *pBuf, DWORD iSize)
{
	if (m_iFileOpened != extract)
		return 0;
	
	if (!pBuf || !iSize)
		return 0;
	
	m_info.m_stream.next_out = (Bytef*)pBuf;
	m_info.m_stream.avail_out = iSize > m_info.m_uUncomprLeft ? m_info.m_uUncomprLeft : iSize;
	
	
	DWORD iRead = 0;
	
	// may happen when the file is 0 sized
	bool bForce = m_info.m_stream.avail_out == 0 && m_info.m_uComprLeft > 0;
	while (m_info.m_stream.avail_out > 0 || (bForce && m_info.m_uComprLeft > 0))
	{
		if ((m_info.m_stream.avail_in == 0) &&
			(m_info.m_uComprLeft > 0))
		{
			DWORD uToRead = m_info.m_pBuffer.GetSize();
			if (m_info.m_uComprLeft < uToRead)
				uToRead = m_info.m_uComprLeft;
			
			if (uToRead == 0)
				return 0;
			
			m_storage.Read(m_info.m_pBuffer, uToRead);
			m_info.m_uComprLeft -= uToRead;
			
			m_info.m_stream.next_in = (unsigned char*)(char*)m_info.m_pBuffer;
			m_info.m_stream.avail_in = uToRead;
		}
		
        //if (CurrentFile()->m_uMethod == 0)        // case 5480
        CPkFileHeader* cpfhPtrCurrentFile = CurrentFile();
        if(!cpfhPtrCurrentFile)
            return 0;

        if (cpfhPtrCurrentFile->m_uMethod == 0)
		{
			DWORD uToCopy = m_info.m_stream.avail_out < m_info.m_stream.avail_in 
				? m_info.m_stream.avail_out : m_info.m_stream.avail_in;
			
			memcpy(m_info.m_stream.next_out, m_info.m_stream.next_in, uToCopy);
			
			m_info.m_uCrc32 = crc32(m_info.m_uCrc32, m_info.m_stream.next_out, uToCopy);
			
			m_info.m_uUncomprLeft -= uToCopy;
			m_info.m_stream.avail_in -= uToCopy;
			m_info.m_stream.avail_out -= uToCopy;
			m_info.m_stream.next_out += uToCopy;
			m_info.m_stream.next_in += uToCopy;
            m_info.m_stream.total_out += uToCopy;
			iRead += uToCopy;
		}
		else
		{
			DWORD uTotal = m_info.m_stream.total_out;
			Bytef* pOldBuf =  m_info.m_stream.next_out;
			int err = inflate(&m_info.m_stream, Z_SYNC_FLUSH);
			DWORD uToCopy = m_info.m_stream.total_out - uTotal;
			
			m_info.m_uCrc32 = crc32(m_info.m_uCrc32, pOldBuf, uToCopy);
			
			m_info.m_uUncomprLeft -= uToCopy;
			iRead += uToCopy;
            
			if (err == Z_STREAM_END)
				return iRead;
			
			if(!CheckForError(err))
				return 0;
		}
	}
	
	return iRead;
}


//////////////////////////////////////////////////////////////////////
void CPkZip::Close(bool bAfterException)
{
	if (IsClosed() && (!bAfterException || IsClosed(false)))
		return;
	
	if (!bAfterException)
	{
		if (m_iFileOpened == extract)
			CloseFile(NULL);
	}
	else
	{
		m_info.m_pBuffer.Release();
		m_iFileOpened = nothing;
		EmptyPtrList();
	}

	m_centralDir.Clear();
	m_storage.Close();
}


//////////////////////////////////////////////////////////////////////
int CPkZip::CloseFile(CAnsiFile &file)
{
	QString temp = file.GetFilePath();
	file.Close();
	return CloseFile(temp.toLatin1().constData());
}


//////////////////////////////////////////////////////////////////////
int CPkZip::CloseFile(const char* lpszFilePath, bool bAfterException)
{
	if (m_iFileOpened != extract)
		return false;

    // case 5480
    CPkFileHeader* cpfhPtrCurrentFile = CurrentFile();
    if(!cpfhPtrCurrentFile)
        return false;

	int iRet = 1;
	if (!bAfterException)
	{
		if (m_info.m_uUncomprLeft == 0)
		{
//			if (m_info.m_uCrc32 != CurrentFile()->m_uCrc32)
            if (m_info.m_uCrc32 != cpfhPtrCurrentFile->m_uCrc32)
				return Error();
		}
		else
			iRet = -1;

				
        //if (CurrentFile()->m_uMethod == Z_DEFLATED)
        if (cpfhPtrCurrentFile->m_uMethod == Z_DEFLATED)
			inflateEnd(&m_info.m_stream);
		
		
		if (lpszFilePath)
		{
			time_t lTime;
			time(&lTime);
			CAnsiFileStatus fs;
			fs.m_status.st_ctime = fs.m_status.st_atime = lTime;
//			fs.m_status.st_mode = CurrentFile()->GetFileAttribute();
//			fs.m_status.st_mtime = CurrentFile()->GetTime().toTime_t();
            fs.m_status.st_mode = cpfhPtrCurrentFile->GetFileAttribute();
            fs.m_status.st_mtime = cpfhPtrCurrentFile->GetTime().toTime_t();
            m_cFileUtils.SetStatus(lpszFilePath, fs);
		}
	}
	m_centralDir.CloseFile();
	m_iFileOpened = nothing;
	m_info.m_pBuffer.Release();
	EmptyPtrList();
	return iRet;
}


//////////////////////////////////////////////////////////////////////
bool CPkZip::IsFileDirectory(WORD uIndex)
{
	if (IsClosed())
		return false;
	
	if (!m_centralDir.IsValidIndex(uIndex))
		return false;
	
	return IsDirectory(m_centralDir.m_headers.at(uIndex)->m_uExternalAttr);
}


//////////////////////////////////////////////////////////////////////
bool CPkZip::ExtractFile(WORD uIndex,
						const char* lpszPath,             
						const char* lpszNewName,          
						DWORD nBufSize)
{
	if(m_bError) return false;

	if (!nBufSize && !lpszPath)
		return false;
	
	CPkFileHeader header;
	GetFileInfo(header, uIndex); // to ensure that slash and oem conversions take place
	QString strFile = lpszPath, strFileName = lpszNewName ? QString(lpszNewName) : header.GetFileName();

	strFile = m_cFileUtils.PathCat(strFile.toLatin1().constData(),strFileName.toLatin1().constData());
	
	if (IsFileDirectory(uIndex))
	{
		if(m_bFullPath)
		{
			m_cFileUtils.CreateDirectory(strFile.toLatin1().constData());
			CAnsiFileStatus fs;
			fs.m_status.st_mode = header.GetFileAttribute();
			fs.m_status.st_ctime = fs.m_status.st_atime = 
				fs.m_status.st_mtime = header.GetTime().toTime_t();
			m_cFileUtils.SetStatus(strFile.toLatin1().constData(), fs);
		}
		return true;
	}
	else
	{
		if (!OpenFile(uIndex))
			return false;

		if(m_bFullPath)
			if(!m_cFileUtils.CreateDirectory(m_cFileUtils.GetFilePath(strFile.toLatin1().constData()).toLatin1().constData()))
				return Error(m_cFileUtils.GetLastErrorMsg());

		CAnsiFile f;
		if(!f.Open(strFile, CAnsiFile::modeWrite | CAnsiFile::modeCreate | CAnsiFile::shareDenyWrite))
		{
			CloseFile(f);
			return Error(f.GetLastErrorMsg());
		}
		DWORD iRead, iSoFar = 0;
		CPkAutoBuffer buf(nBufSize);
		do
		{
			iRead = ReadFile(buf, buf.GetSize());
			if (iRead)
			{	
				f.Write(buf, iRead);
				iSoFar += iRead;
			}

			// Process application's events to prevent freezing the GUI
            QCoreApplication::processEvents();
		}
		while (iRead == buf.GetSize());
		if(CloseFile(f) == 1)
			return true;

		remove(strFile.toLatin1().constData());
		return false;
	}	
}


//////////////////////////////////////////////////////////////////////
bool CPkZip::IsDirectory(DWORD uAttr)
{
	return (uAttr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}


//////////////////////////////////////////////////////////////////////
int CPkZip::GetCurrentDisk()
{
	return m_storage.GetCurrentDisk() + 1;
}


//////////////////////////////////////////////////////////////////////
int CPkZip::WideToSingle(const char* lpWide, CPkAutoBuffer &szSingle)
{
	QString strWide;
	size_t wideLen = strWide.length();
	if (wideLen == 0)
	{
		szSingle.Release();
		return 0;
	}

	szSingle.Allocate(wideLen);
	memcpy(szSingle, lpWide, wideLen);
	return wideLen;
}


//////////////////////////////////////////////////////////////////////
int CPkZip::SingleToWide(CPkAutoBuffer &szSingle, QString& szWide)
{
	int singleLen = szSingle.GetSize();
	char string[256];
	memcpy(string, szSingle, singleLen);
	szWide = string;
	szWide = szWide.left(singleLen);
	return singleLen;
}


//////////////////////////////////////////////////////////////////////
void CPkZip::EmptyPtrList()
{
	if (m_list.count())
	{
		for(int i = 0; i < (int)m_list.count(); i++)
			delete (char*) m_list.at(i);
		m_list.clear();
	}

}



