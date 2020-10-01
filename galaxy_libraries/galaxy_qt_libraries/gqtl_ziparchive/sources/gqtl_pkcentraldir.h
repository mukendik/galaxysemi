// PkCentralDir.h: interface for the CPkCentralDir class.
////////////////////////////////////////////////////////////////////////////////

#if !defined(_PkCentralDir_H__859029E8_8927_4717_9D4B_E26E5DA12BAE__INCLUDED_)
#define _PkCentralDir_H__859029E8_8927_4717_9D4B_E26E5DA12BAE__INCLUDED_

// Project headers
#include "gqtl_pkfileheader.h"
#include "gqtl_pkautobuffer.h"

// Standard headers

// QT headers
#include <QList>

#define WINCENTRALDIRSIZE	22

class CPkCentralDir  
{

public:
	
//		end of central dir signature    4 bytes  (0x06054b50)
	char m_szSignature[4];
//		number of this disk             2 bytes
	WORD m_uThisDisk;
//		number of the disk with the
//		start of the central directory  2 bytes
	WORD m_uDiskWithCD;
//		total number of entries in
//		the central dir on this disk    2 bytes
	WORD m_uDiskEntriesNo;
//		total number of entries in
//		the central dir                 2 bytes
	WORD m_uEntriesNumber;
//		size of the central directory   4 bytes
	DWORD m_uSize;
//		offset of start of central
//		directory with respect to
//		the starting disk number        4 bytes
	DWORD m_uOffset;
//		zipfile comment length          2 bytes

	CPkAutoBuffer m_pszComment;
	bool m_bFindFastEnabled;
	CPkFileHeader* m_pOpenedFile;
	void Clear(bool bEverything = true);
	CPkStorage* m_pStorage;
	DWORD m_uCentrDirPos;
	DWORD m_uBytesBeforeZip;
	CPkCentralDir();
	virtual ~CPkCentralDir();
	bool CloseFile();
	bool OpenFile(WORD uIndex);
	bool IsValidIndex(WORD uIndex);
	bool Read();
	void Init();
	int m_iBufferSize;
	bool m_bOnDisk;
	static char m_gszSignature[];
	QList<CPkFileHeader *> m_headers;
	CPkAutoBuffer m_pLocalExtraField;
	void RemoveFromDisk();
	DWORD GetSize(bool bWhole = false);
	bool m_bConvertAfterOpen;

	void ConvertFileName(bool bFromZip, bool bAfterOpen, CPkFileHeader* pHeader = NULL)
	{
		if (bAfterOpen != m_bConvertAfterOpen)
			return;
		if (!pHeader)
		{
			pHeader = m_pOpenedFile;
			Q_ASSERT(pHeader != NULL);
		}
		pHeader->AnsiOem(!bFromZip);
		pHeader->SlashChange();
	}
	void ConvertAll();

	bool	m_bError;
	QString	GetLastErrorMsg()	{ return m_strErrorMsg;	}

protected:
	QString m_strErrorMsg;
	void RemoveHeaders();
	bool RemoveDataDescr(bool bFromBuffer);
	bool ReadHeaders();
	bool Error(char* msg = (char*)"");
	DWORD Locate();	
};

#endif // !defined(_PkCentralDir_H__859029E8_8927_4717_9D4B_E26E5DA12BAE__INCLUDED_)
