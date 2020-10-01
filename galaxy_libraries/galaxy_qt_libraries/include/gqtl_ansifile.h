// CAnsiFile.h
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CAnsiFile_H__79E0E6BD_25D6_4B82_85C5_AB397D9EC368__INCLUDED_)
#define _CAnsiFile_H__79E0E6BD_25D6_4B82_85C5_AB397D9EC368__INCLUDED_

// Project headers

// Standard headers
#include <sys/types.h>
#include <sys/stat.h>	/* mkdir() */
#if defined __unix__ || __APPLE__&__MACH__
#include <unistd.h>		/* access(), unlink() */
#include <utime.h>
#else
#include <direct.h>
#include <io.h>
#endif

// QT headers
#include <QString>

// Macros
#if defined __unix__ || __APPLE__&__MACH__
#define MkDir(dirname,mode) mkdir(dirname,mode)
#define UnLink(fn)          unlink(fn)
#define Access(path,mode)   access(path,mode)
#else
#define MkDir(dirname,mode) _mkdir(dirname)
#define UnLink(fn)          _unlink(fn)
#define Access(path,mode)   _access(path,mode)
#endif

#include "gstdl_type.h"
//#if !defined(_WINDOWS_H)
//typedef unsigned short	WORD;
//typedef unsigned long	DWORD;
//typedef unsigned int	UINT;

//#if !defined(NULL)
//#define NULL    0
//#endif
//#endif // _WINDOWS_H

#ifndef _MAX_PATH
#define _MAX_PATH		2048
#endif
#ifndef USHRT_MAX
#define USHRT_MAX		0xffff
#endif

typedef double			DOUBLE;

#ifndef FILE_ATTRIBUTE_READONLY
#define FILE_ATTRIBUTE_READONLY   	0x0000001
#endif
#ifndef FILE_ATTRIBUTE_HIDDEN
#define FILE_ATTRIBUTE_HIDDEN   	0x0000002
#endif
#ifndef FILE_ATTRIBUTE_SYSTEM
#define FILE_ATTRIBUTE_SYSTEM   	0x0000004
#endif
#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY	0x0000010
#endif
#ifndef FILE_ATTRIBUTE_ARCHIVE
#define FILE_ATTRIBUTE_ARCHIVE		0x0000020
#endif
#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL		0x0000080
#endif
#ifndef FILE_ATTRIBUTE_TEMPORARY
#define FILE_ATTRIBUTE_TEMPORARY    0x0000100
#endif
#ifndef FILE_ATTRIBUTE_COMPRESSED
#define FILE_ATTRIBUTE_COMPRESSED	0x0000800
#endif

#if defined __unix__ || __APPLE__&__MACH__
#define CHAR_SEP_OLD '\\'
#define CHAR_SEP '/'
#else
#define CHAR_SEP_OLD '/'
#define CHAR_SEP '\\'
#endif

#if defined __unix__ || __APPLE__&__MACH__
#ifndef _MAX_PATH
#define _MAX_PATH MAXPATHLEN
#endif
#endif

//case 5480
//#if defined(__unix__) || defined(__sun__)
#ifdef __sparc__
#define	_BIGENDIAN
#endif

#ifdef _BIGENDIAN
#define READ_WORD(var,buf,offset) \
	*(((char*)&var)+0) = (char)(*(buf + offset+1)); \
	*(((char*)&var)+1) = (char)(*(buf + offset+0));
#define READ_DWORD(var,buf,offset) \
	*(((char*)&var)+0) = (char)(*(buf + offset+3)); \
	*(((char*)&var)+1) = (char)(*(buf + offset+2)); \
	*(((char*)&var)+2) = (char)(*(buf + offset+1)); \
	*(((char*)&var)+3) = (char)(*(buf + offset+0));
#else
#define READ_WORD(var,buf,offset) \
	*(((char*)&var)+0) = (char)(*(buf + offset+0)); \
	*(((char*)&var)+1) = (char)(*(buf + offset+1));
#define READ_DWORD(var,buf,offset) \
	*(((char*)&var)+0) = (char)(*(buf + offset+0)); \
	*(((char*)&var)+1) = (char)(*(buf + offset+1)); \
	*(((char*)&var)+2) = (char)(*(buf + offset+2)); \
	*(((char*)&var)+3) = (char)(*(buf + offset+3));
#endif

struct CAnsiFileStatus
{
	struct stat m_status;
	QString m_strFullName; // absolute path name
};

class CAnsiFile
{

public:
// Flag values
	enum OpenFlags {
		modeRead =          0x0000,
		modeWrite =         0x0001,
		modeReadWrite =     0x0002,
		shareCompat =       0x0000,
		shareExclusive =    0x0010,
		shareDenyWrite =    0x0020,
		shareDenyRead =     0x0030,
		shareDenyNone =     0x0040,
		modeNoInherit =     0x0080,
		modeCreate =        0x1000,
		modeNoTruncate =    0x2000,
		typeText =          0x4000, // typeText and typeBinary are used in
		typeBinary =   (int)0x8000 // derived classes only
		};

	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
		};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	enum { hFileNull = -1 };

// Constructors
	CAnsiFile();

// Attributes
	UINT m_hFile;

	DWORD GetPosition();
	bool GetStatus(CAnsiFileStatus& rStatus);
	QString GetFileName();
	QString GetFileTitle();
	QString GetFilePath();
	void SetFilePath(const char* lpszNewName);

// Operations
	bool Open(const char* lpszFileName, UINT nOpenFlags = modeRead);
	bool Open(QString lpszFileName, UINT nOpenFlags = modeRead);

// utils function
	QString GetAbsFileName(const char* lpszFileName);
	QString GetFileDirAndName(const char* lpszFilePath);
	QString GetFileTitle(const char* lpszFilePath);
	QString GetDrive(const char* lpszFilePath);
	bool DirectoryExists(const char* lpszDir);
	int FileExists(const char* lpszName);
	QString GetFilePath(const char* lpszFilePath);
	QString GetFileExt(const char* lpszFilePath);
	QString GetFileName(const char* lpszFilePath);
	void Rename(const char* lpszOldName,	const char* lpszNewName);
	void Remove(const char* lpszFileName);
	bool GetStatus(const char* lpszFileName, CAnsiFileStatus& rStatus);
	void SetStatus(const char* lpszFileName, const CAnsiFileStatus& status);
	QString PathCat(const char* lpszFilePath, const char* lpszFileName);
	QString NormalizePath(const char* lpszFilePath);

	bool CreateDirectory(const char* lpDirectory);

	DWORD SeekToEnd();
	void SeekToBegin();

	// backward compatible ReadHuge and WriteHuge
	DWORD ReadHuge(void* lpBuffer, DWORD dwCount);
	void WriteHuge(const void* lpBuffer, DWORD dwCount);


	long Seek(long lOff, UINT nFrom);
	DWORD GetLength();

	UINT Read(void* lpBuf, UINT nCount);
	void Write(const void* lpBuf, UINT nCount);

	void Abort();
	void Flush();
	void Close();

	bool		m_bError;
	QString		GetLastErrorMsg();

// Implementation
public:
	~CAnsiFile();
	enum BufferCommand { bufferRead, bufferWrite, bufferCommit, bufferCheck };

protected:
	bool CheckError(int iRes, QString strFileName, QString strFunction);
	QString	m_strErrorMsg;
	bool m_bCloseOnDelete;
	QString m_strFileName;
};

#endif // !defined(_CAnsiFile_H__79E0E6BD_25D6_4B82_85C5_AB397D9EC368__INCLUDED_)
