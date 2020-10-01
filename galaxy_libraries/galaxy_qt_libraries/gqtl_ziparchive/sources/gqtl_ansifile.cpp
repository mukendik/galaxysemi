// AnsiFile.cpp
//
//////////////////////////////////////////////////////////////////////

// Project headers
#include "gqtl_ansifile.h"

// Standard headers
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#if defined __unix__ || __APPLE__&__MACH__
#ifndef __linux__
#include <sys/conf.h>
#include <unistd.h>
#endif
//#include <stropts.h>
#include <sys/uio.h>
#include <utime.h>
#else
#include <io.h>
#include <sys/utime.h>
#include <direct.h>
#include <share.h>
#endif

// QT headers
#include <QtGui>
#include <QString>
#include <QFileInfo>

////////////////////////////////////////////////////////////////////////////
// CAnsiFile implementation

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAnsiFile::CAnsiFile()
{
	m_bError = false;
	m_strErrorMsg = "";
	m_hFile = (UINT) hFileNull;
	m_bCloseOnDelete = false;
}


CAnsiFile::~CAnsiFile()
{
	if (m_hFile != (UINT)hFileNull && m_bCloseOnDelete)
		Close();
}

//////////////////////////////////////////////////////////////////////
// Check if iRes return code is OK. If not, retrieve system error 
// and append specified texts
//////////////////////////////////////////////////////////////////////
bool CAnsiFile::CheckError(int iRes, QString strFileName, QString strFunction)
{
	if(iRes == -1)
	{
		m_bError = true;
		m_strErrorMsg = strerror(errno);
		m_strErrorMsg += ": ";
		m_strErrorMsg += strFileName;
		m_strErrorMsg += " (Function ";
		m_strErrorMsg += strFunction;
		m_strErrorMsg += ")";

		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Return last error message
//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetLastErrorMsg()
{
	m_bError = false; 
	return m_strErrorMsg;
}

//////////////////////////////////////////////////////////////////////
bool CAnsiFile::Open(QString strFileName, UINT nOpenFlags)
{
	return Open( strFileName.toLatin1().constData(), nOpenFlags);
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
bool CAnsiFile::Open(const char* lpszFileName, UINT nOpenFlags)
{
	m_bError = false;
	m_strErrorMsg = "";

	m_strFileName = GetAbsFileName(lpszFileName);
	m_bCloseOnDelete = false;
	m_hFile = (UINT)hFileNull;

	// map read/write mode
	DWORD dwAccess = 0;
	switch (nOpenFlags & 3)
	{
	case modeRead:
		dwAccess = S_IREAD;
		break;
	case modeWrite:
		dwAccess = S_IWRITE;
		break;
	case modeReadWrite:
		dwAccess = S_IREAD | S_IWRITE;
		break;
	}

	// map creation flags
	DWORD dwCreateFlag = 0;
	
#if !defined __unix__ && !defined __MACH__
	// map share mode
	DWORD dwShareMode = 0;
	switch (nOpenFlags & 0x70)    // map compatibility mode to exclusive
	{
	case shareCompat:
	case shareExclusive:
		dwShareMode = SH_DENYRW;
		break;
	case shareDenyWrite:
		dwShareMode = SH_DENYWR;
		break;
	case shareDenyRead:
		dwShareMode = SH_DENYRD;
		break;
	case shareDenyNone:
		dwShareMode = SH_DENYNO;
		break;
	}

	// map creation flags
	dwCreateFlag = O_BINARY;
	if(nOpenFlags & modeNoInherit)
		dwCreateFlag |= O_NOINHERIT;
#endif

	if (nOpenFlags & modeCreate)
	{
		if (nOpenFlags & modeNoTruncate)
			dwCreateFlag |= O_CREAT | O_RDWR;
		else
			dwCreateFlag |= O_CREAT | O_RDWR | O_TRUNC;
	}
	else
		dwCreateFlag |= O_RDONLY;


	// attempt file creation
	struct stat s;
	if((dwAccess & S_IWRITE) && (stat(m_strFileName.toLatin1().data(), &s) == 0) && !(s.st_mode & S_IWRITE))
		if(!CheckError(chmod(m_strFileName.toLatin1().data(),  s.st_mode | S_IWRITE), m_strFileName, "Open/chmod"))
			return false;

	int hFile;

#if defined __unix__ || __APPLE__&__MACH__
	hFile = open(m_strFileName.toLatin1().data(), dwCreateFlag, dwAccess);
#else
	hFile = sopen(m_strFileName.toLatin1().data(), dwCreateFlag, dwShareMode, dwAccess);
#endif

	if(!CheckError(hFile, m_strFileName, "Open/open"))
		return false;
	m_hFile = hFile;
	m_bCloseOnDelete = true;

	return true;
}

//////////////////////////////////////////////////////////////////////
UINT CAnsiFile::Read(void* lpBuf, UINT nCount)
{
	if(m_bError) return false;

	if (nCount == 0)
		return 0;

	UINT iRead = 0;
	if(!CheckError((iRead = read(m_hFile, lpBuf, nCount)), m_strFileName, "Read"))
		return 0;
	return iRead;
}


//////////////////////////////////////////////////////////////////////
void CAnsiFile::Write(const void* lpBuf, UINT nCount)
{
	if(m_bError) return;

	if (nCount == 0)
		return; 

	CheckError(write(m_hFile, lpBuf, nCount), m_strFileName, "Write");
}


//////////////////////////////////////////////////////////////////////
long CAnsiFile::Seek(long lOff, UINT nFrom)
{
	if(m_bError) return false;

	int nPos = 0;
	if(nFrom == begin)
		nPos = SEEK_SET;
	if(nFrom == current)
		nPos = SEEK_CUR;
	if(nFrom == end)
		nPos = SEEK_END;
	long lVal;
	lVal = lseek(m_hFile, lOff, nPos);
	if(!CheckError(lVal, m_strFileName, "Seek"))
		return 0;
	return lVal;
}


//////////////////////////////////////////////////////////////////////
DWORD CAnsiFile::GetPosition()
{
	if(m_bError) return false;

#if defined __unix__ || __APPLE__&__MACH__
	DWORD dVal = lseek(m_hFile, SEEK_CUR, 0);
#else
	DWORD dVal = tell(m_hFile);
#endif
	if(!CheckError(dVal, m_strFileName, "GetPosition"))
		return 0;
	return dVal;
}


//////////////////////////////////////////////////////////////////////
void CAnsiFile::Flush()
{
	if(m_bError) return;

	if (m_hFile == (UINT)hFileNull)
		return;

#if defined __unix__ || __APPLE__&__MACH__
//	CheckError(ioctl(m_hFile, I_FLUSH, FLUSHRW), m_strFileName, "Flush");
#else
	CheckError(_commit(m_hFile), m_strFileName, "Flush");
#endif
}


//////////////////////////////////////////////////////////////////////
void CAnsiFile::Close()
{
	if (m_hFile != (UINT)hFileNull)
		close(m_hFile);

	m_hFile = (UINT) hFileNull;
	m_bCloseOnDelete = false;
	m_strFileName = "";
	m_bError = false;
}


//////////////////////////////////////////////////////////////////////
void CAnsiFile::Abort()
{
	if (m_hFile != (UINT)hFileNull)
	{
		// close but ignore errors
		close(m_hFile);
		m_hFile = (UINT)hFileNull;
	}
	m_strFileName = "";
	m_bError = false;
}


//////////////////////////////////////////////////////////////////////
DWORD CAnsiFile::GetLength()

{
	if(m_bError) return false;
	DWORD dwLen, dwCur;

	CAnsiFile* pFile = (CAnsiFile*)this;
	dwCur = pFile->Seek(0L, current);
	dwLen = pFile->SeekToEnd();
	pFile->Seek(dwCur, begin);

	return dwLen;
}


//////////////////////////////////////////////////////////////////////
void CAnsiFile::Rename(const char* lpszOldName, const char* lpszNewName)
{
	m_bError = false;
	m_strErrorMsg = "";
	CheckError(rename((char*)lpszOldName, (char*)lpszNewName), lpszOldName, "Rename");
}


//////////////////////////////////////////////////////////////////////
void CAnsiFile::Remove(const char* lpszFileName)
{
	m_bError = false;
	m_strErrorMsg = "";
	CheckError(remove((char*)lpszFileName), lpszFileName, "Remove");
}



//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetFilePath()
{
	return m_strFileName;
}


//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetFileName()
{
	return m_strFileName;
}



//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetFileName(const char* lpszFilePath)
{
	QFileInfo qFile(lpszFilePath);
	return qFile.fileName();
}

//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetFilePath(const char* lpszFilePath)
{
	QFileInfo qFileInfo(lpszFilePath);
	return NormalizePath(qFileInfo.path().toLatin1().constData());
}


//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetFileExt(const char* lpszFilePath)
{
	QFileInfo qFile(lpszFilePath);
	return qFile.suffix();
}


//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetFileTitle(const char* lpszFilePath)
{
	QFileInfo qFile(lpszFilePath);
	return qFile.baseName();
}


//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetFileTitle()
{
	return GetFileTitle(m_strFileName.toLatin1().data());
}

//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetAbsFileName(const char* lpszFileName)
{
	QString strFileName = lpszFileName, strAbsFileName;

	if(strFileName.isEmpty())
		return strFileName;
	
#if 0
	char	szBuffer[_MAX_PATH];
#if defined __unix__ || __APPLE__&__MACH__
	strFileName = NormalizePath(lpszFileName);
	if(realpath(strFileName.toLatin1().data(), szBuffer) != NULL)
		strFileName = szBuffer;
#else
	strFileName = _fullpath( szBuffer, lpszFileName, _MAX_PATH );
#endif
#endif
	QFileInfo qFileInfo(strFileName);
	strAbsFileName = NormalizePath(qFileInfo.absoluteFilePath().toLatin1().constData());

	return strAbsFileName;
}

//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetFileDirAndName(const char* lpszFilePath)
{
	QString	strFile = NormalizePath(lpszFilePath);
	if((!strFile.isEmpty()) && (strFile.left(2) == ":"))
		strFile = strFile.right(strFile.length() - 2);
	while((!strFile.isEmpty()) && (strFile.at(0) == CHAR_SEP))
		strFile = strFile.right(strFile.length() - 1);
	return  strFile;
}

//////////////////////////////////////////////////////////////////////
QString CAnsiFile::PathCat(const char* lpszFilePath, const char* lpszFileName)
{
	QString strPath = NormalizePath(lpszFilePath);
	QFileInfo qFileInfo(lpszFileName);
	if(qFileInfo.isRelative())
	{
		if(strPath.at(strPath.length()-1) != CHAR_SEP)
			strPath += CHAR_SEP;
		strPath += lpszFileName;
		qFileInfo.setFile(strPath);
		strPath = NormalizePath(qFileInfo.filePath().toLatin1().constData());
	}
	else
		strPath = NormalizePath(lpszFileName);
	return strPath;
}


//////////////////////////////////////////////////////////////////////
QString CAnsiFile::NormalizePath(const char* lpszFilePath)
{
	QString strPath = lpszFilePath;
	strPath.replace(CHAR_SEP_OLD,CHAR_SEP);
	return strPath;
}


//////////////////////////////////////////////////////////////////////
int CAnsiFile::FileExists(const char* lpszName)
{
	QString strAbsFileName;

	m_bError = false;
	m_strErrorMsg = "";

	strAbsFileName = GetAbsFileName(lpszName);
	
	QFileInfo qFileInfo(strAbsFileName);
	if(qFileInfo.exists())
	{
		if(qFileInfo.isFile())
			return 1;
		if(qFileInfo.isDir())
			return -1;
	}
	int hFile;
	CheckError((hFile = open(strAbsFileName.toLatin1().data(),O_RDONLY)), strAbsFileName.toLatin1().data(), "FileExists");
	return 0;
}

//////////////////////////////////////////////////////////////////////
bool CAnsiFile::DirectoryExists(const char* lpszDir)
{
	return FileExists(lpszDir) == -1;
}


//////////////////////////////////////////////////////////////////////
QString CAnsiFile::GetDrive(const char* lpszFilePath)
{
	QString	strDrive= lpszFilePath;
	if((!strDrive.isEmpty()) && (strDrive.left(2) == ":"))
		strDrive = strDrive.left(2);
	else
		strDrive = "";
	return  strDrive;
}


/////////////////////////////////////////////////////////////////////////////
// CAnsiFile Status implementation

bool CAnsiFile::GetStatus(CAnsiFileStatus& rStatus)
{
	return GetStatus(m_strFileName.toLatin1().constData(), rStatus);
}


//////////////////////////////////////////////////////////////////////
bool CAnsiFile::GetStatus(const char* lpszFileName, CAnsiFileStatus& rStatus)
{
	// attempt to fully qualify path first
	if(FileExists(lpszFileName) < 1)
	{
		rStatus.m_strFullName = "";
		return false;
	}
	rStatus.m_strFullName = GetAbsFileName(lpszFileName);
	stat((const char*) lpszFileName, &rStatus.m_status);
	return true;
}



//////////////////////////////////////////////////////////////////////
void CAnsiFile::SetStatus(const char* lpszFileName, const CAnsiFileStatus& status)
{
	m_bError = false;
	m_strErrorMsg = "";

	struct stat s;
	stat((char*)lpszFileName, &s);
	
	if (!(s.st_mode & S_IWRITE))
		CheckError(chmod((char*)lpszFileName,  s.st_mode | S_IWRITE), lpszFileName, "SetStatus/chmod1");

	if(s.st_mode & S_IFDIR)
		CheckError(chmod((char*)lpszFileName,  (s.st_mode | 0111)), lpszFileName, "SetStatus/chmod2");
	else
	{
		struct utimbuf u;
		u.actime = status.m_status.st_atime;
		u.modtime = status.m_status.st_mtime;
		CheckError(utime(lpszFileName, &u), lpszFileName, "SetStatus/utime");
		CheckError(chmod((char*)lpszFileName,  status.m_status.st_mode), lpszFileName, "SetStatus/chmod3");
	}
}



//////////////////////////////////////////////////////////////////////
DWORD CAnsiFile::SeekToEnd()
{ 
	return Seek(0, CAnsiFile::end); 
}


//////////////////////////////////////////////////////////////////////
void CAnsiFile::SeekToBegin()
{ 
	Seek(0, CAnsiFile::begin); 
}


//////////////////////////////////////////////////////////////////////
void CAnsiFile::SetFilePath(const char* lpszNewName)
{
	m_strFileName = lpszNewName;
}



//////////////////////////////////////////////////////////////////////
bool CAnsiFile::CreateDirectory(const char* lpDirectory)
{
	m_bError = false;
	m_strErrorMsg = "";

	Q_ASSERT(lpDirectory != NULL);
	QString strDirectory = lpDirectory;
	if(strDirectory.at(strDirectory.length() - 1) == CHAR_SEP)
		strDirectory = strDirectory.left(strDirectory.length() - 1);
	if ((GetFilePath(strDirectory.toLatin1().constData()) == strDirectory) ||
		(FileExists(strDirectory.toLatin1().constData()) == -1))
		return true;
	if (!CreateDirectory(GetFilePath(strDirectory.toLatin1().constData()).toLatin1().constData()))
		return false;
	m_bError = false;
	m_strErrorMsg = "";
	return CheckError(MkDir(strDirectory.toLatin1().constData(), 0755), lpDirectory, "CreateDirectory");
}

