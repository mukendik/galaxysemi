// PkFileHeader.h: interface for the CPkFileHeader class.
////////////////////////////////////////////////////////////////////////////////

#if !defined(_PkFileHeader_H__0081FC65_C9C9_4D48_AF72_DBF37DF5E0CF__INCLUDED_)
#define _PkFileHeader_H__0081FC65_C9C9_4D48_AF72_DBF37DF5E0CF__INCLUDED_

// Project headers
#include "gqtl_pkstorage.h"

// Standard headers

// QT headers
#include <QDateTime>

class CPkFileHeader  
{
public:	
// convert characters in the filename from oem to ansi or vice-versa	
	void AnsiOem(bool bAnsiToOem);
// change slash to backslash or vice-versa	
	void SlashChange();
// return the filename size in characters (without NULL);	
	WORD GetFileNameSize(){return (WORD)m_pszFileName.GetSize();}
// return the comment size in characters (without NULL);		
	WORD GetCommentSize(){return (WORD)m_pszComment.GetSize();}
// return the extra field size in characters (without NULL);		
	WORD GetExtraFieldSize(){return (WORD)m_pExtraField.GetSize();}

	UINT GetFileAttribute();

	QString GetFileName();
// return true if confersion from unicode to single byte was successful	
	bool SetFileName(const char* lpszFileName);
	QString GetComment();
// return true if confersion from unicode to single byte was successful	
	bool SetComment(const char* lpszComment);
// return true if the data descriptor is present	
	bool IsDataDescr();
//	return true if the file is encrypted
	bool IsEncrypted();
//         central file header signature   4 bytes  (0x02014b50)
    char m_szSignature[4];
//         version made by                 2 bytes
	WORD m_uVersionMadeBy;
//         version needed to extract       2 bytes
	WORD m_uVersionNeeded;
//         general purpose bit flag        2 bytes
	WORD m_uFlag;
//         compression method              2 bytes
	WORD m_uMethod;
//         last mod file time              2 bytes
	WORD m_uModTime;
//         last mod file date              2 bytes
	WORD m_uModDate;
//         crc-32                          4 bytes
	DWORD m_uCrc32;
//         compressed size                 4 bytes
	DWORD m_uComprSize;
//         uncompressed size               4 bytes
	DWORD m_uUncomprSize;
//         disk number start               2 bytes
	WORD m_uDiskStart;
//         internal file attributes        2 bytes
	WORD m_uInternalAttr;
//         external file attributes        4 bytes
	DWORD m_uExternalAttr;
//         relative offset of local header 4 bytes
	DWORD m_uOffset;	
//         extra field (variable size)
	CPkAutoBuffer m_pExtraField;

	CPkFileHeader();
	virtual ~CPkFileHeader();
	QDateTime GetTime();
	void SetTime(const QDateTime& time);
	void SetTime(time_t time);
	static char m_gszSignature[];
	static char m_gszLocalSignature[];
// return the total size of the structure as stored in the central directory	
	DWORD GetSize();

	bool	m_bError;
	QString	GetLastErrorMsg()	{ return m_strErrorMsg; };

protected:
	QString	m_strErrorMsg;
//         filename (variable size)
	CPkAutoBuffer m_pszFileName;	
//         file comment (variable size)
	CPkAutoBuffer m_pszComment;

	bool CheckCrcAndSizes(char* pBuf);
	friend class CPkCentralDir;
	friend class CPkZip;
	bool Read(CPkStorage *pStorage);
	bool ReadLocal(CPkStorage *pStorage, WORD& iLocExtrFieldSize);
	DWORD Write(CPkStorage *pStorage);

	bool	Error(char* msg = (char*)"");

};

#endif // !defined(_PkFileHeader_H__0081FC65_C9C9_4D48_AF72_DBF37DF5E0CF__INCLUDED_)
