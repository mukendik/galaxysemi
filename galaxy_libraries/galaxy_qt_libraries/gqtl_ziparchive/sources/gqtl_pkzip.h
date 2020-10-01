
//////////////////////////////////////////////////////////////////////
// PkZip.h: interface for the CPkZip class.
//

#if !defined(_PkZip_H__A7F528A6_1872_4071_BE66_D56CC2DDE0E6__INCLUDED_)
#define _PkZip_H__A7F528A6_1872_4071_BE66_D56CC2DDE0E6__INCLUDED_

// Project headers
#include "gqtl_pkcentraldir.h"
#include "gqtl_pkinternalinfo.h"
#include "gqtl_pkstorage.h"	

// Standard headers

// QT headers
#include <QList>
#include <QStringList>


//////////////////////////////////////////////////////////////////////////
/////////////////////  CPkZip   /////////////////////////////////////

class CPkZip  
{
public:
	static int SingleToWide(CPkAutoBuffer &szSingle, QString& szWide);
	static int WideToSingle(const char* lpWide, CPkAutoBuffer &szSingle);

//	archive open modes	
	enum {open, openReadOnly, create, createSpan};

    CPkZip(QString strWorkDir = "", bool bFullPath = true, bool bUniqueName = false);
	virtual ~CPkZip();

    bool            ExtractFiles(QString strZipFile, QStringList& lstToFile, bool lListOnly = false);
    static bool     IsPkZipFile(QString strZipFile);

	QString	m_strWorkDir;	// the directory where compressed files are extracted
							// or where compressed file are create
	bool	m_bFullPath;	// false if all file has to be extracted in the same dir
	bool	m_bUniqueName;	// if bFullPath then create new file name like subdir_filename.ext

	bool	m_bError;
	QString	GetLastErrorMsg() { return m_strErrorMsg; }
	
protected:
	QString m_strErrorMsg;
	
	bool Open(const char* szPathName);
	bool Open(CAnsiFile& mf);
	bool ExtractFile(WORD uIndex, const char* lpszPath, const char* lpszNewName = NULL, DWORD nBufSize = 65535);
	bool OpenFile(WORD uIndex);
	DWORD ReadFile(void *pBuf, DWORD iSize);
	int CloseFile(CAnsiFile &file);
	int CloseFile(const char* lpszFilePath = NULL, bool bAfterException = false);
	int GetCurrentDisk();
	bool IsFileDirectory(WORD uIndex);
	bool GetFileInfo(CPkFileHeader & fhInfo, WORD uIndex);
	int GetNoEntries();
	void Close(bool bAfterException = false);
	bool IsClosed(bool bArchive = true);
	bool m_bDetectZlibMemoryLeaks;

	void EmptyPtrList();
	DWORD m_keys[3];

	bool IsDirectory(DWORD uAttr);
	CPkFileHeader* CurrentFile();
	bool CheckForError(int iErr);
	CPkInternalInfo m_info;
	CPkStorage m_storage;
	QList<void*> m_list;
	static void* myalloc(void* opaque, UINT items, UINT size);
	static void myfree(void* opaque, void* address);
	enum {extract = -1, nothing, compress};
	char m_iFileOpened;
	bool Error(char* msg = (char*)"");
	bool Error(QString strMsg);
	CPkCentralDir m_centralDir;

	CAnsiFile		m_cFileUtils;
};

#endif // !defined(_PkZip_H__A7F528A6_1872_4071_BE66_D56CC2DDE0E6__INCLUDED_)
