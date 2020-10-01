// PkStorage.h: interface for the CPkStorage class.
////////////////////////////////////////////////////////////////////////////////

#if !defined(_WINSTORAGE_H__941824FE_3320_4794_BDE3_BE334ED8984B__INCLUDED_)
#define _WINSTORAGE_H__941824FE_3320_4794_BDE3_BE334ED8984B__INCLUDED_


// Project headers
#include "gqtl_ansifile.h"	// Added by ClassView
#include "gqtl_pkautobuffer.h"	// Added by ClassView

// Standard headers

// QT headers
#include <QString>

class CPkStorage  
{
public:
	void Open(CAnsiFile& mf);
// return the position in the file, taking into account the bytes in the write buffer
	DWORD GetPosition();

// only called by CPkCentralDir when opening an existing archive
	void UpdateSpanMode(WORD uLastDisk);
// the preset size of the write buffer
	int m_iWriteBufferSize;

// user data to be passed to the callback function
	void* m_pCallbackData;

	void Close();

// return the numer of the current disk
	int GetCurrentDisk();

	void SetCurrentDisk(int iNumber);

// change the disk during extract operations
	void ChangeDisk(int iNumber);

	void Open(const char* szPathName);
	DWORD Read(void* pBuf, DWORD iSize);
	CAnsiFile  m_internalfile;
	CAnsiFile* m_pFile;
	CPkStorage();
	virtual ~CPkStorage();
	enum {noSpan, pkzipSpan, tdSpan, suggestedAuto, suggestedTd};
	int m_iSpanMode;
	static char m_gszExtHeaderSignat[];

// open tdspan: last disk number, create tdspan: volume size
// create pkspan: not used
	int m_iTdSpanData;

	bool	m_bError;
	QString	GetLastErrorMsg()	{ return m_strErrorMsg; } ;

protected:
	QString m_strErrorMsg;
	// how many bytes left free in the write buffer
	DWORD GetFreeInBuffer();
	friend class CPkCentralDir;
// numer of bytes available in the write buffer
	DWORD m_uBytesInWriteBuffer;

//  tdSpan : the total size of the current volume, pkSpan : free space on the current volume
	DWORD m_uCurrentVolSize;

// number of bytes left free in the write buffer
	DWORD m_uVolumeFreeInBuffer;

	CPkAutoBuffer m_pWriteBuffer;

// only disk spanning creation: tells how many bytes have been written physically to the current volume
	DWORD m_iBytesWritten;

// construct the name of the volume in tdSpan mode
	QString GetTdVolumeName(bool bLast, const char* lpszZipName = NULL);

// change the disk in tdSpan mode
	QString ChangeTdRead();

//  you can only add a new files to the new disk spanning archive and only extract 
//	them from the existing one
	bool m_bNewSpan;

	int m_iCurrentDisk;
	bool OpenFile(const char* lpszName, UINT uFlags);
	void Error(char* msg = (char*)"");
	
};

#endif // !defined(_WINSTORAGE_H__941824FE_3320_4794_BDE3_BE334ED8984B__INCLUDED_)
