// gmembuffer.h: interface for the CGMemBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMEMBUFFER_H__FAB69F61_6685_4910_B2BB_ED8B03B92245__INCLUDED_)
#define AFX_GMEMBUFFER_H__FAB69F61_6685_4910_B2BB_ED8B03B92245__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gstdl_errormgr.h>
/*
#if defined(_WIN32)
#	if defined(_GALAXY_MEMBUFFER_EXPORTS_)
#		define GMEMBUFFER_API __declspec(dllexport)
#	elif defined _GALAXY_STDUTILS_DLL_MODULE
#		define GMEMBUFFER_API
#	else
#		define GMEMBUFFER_API __declspec(dllimport)
#	endif // _GALAXY_ERRORMGR_EXPORTS_
#else
#	define GMEMBUFFER_API
#endif // _MSC_VER
*/

#define GMEMBUFFER_API
#define GMB_DEFAULT_INCREMENT		256

// ----------------------------------------------------------------------------------------------------------
// class CGMemBuffer

class GMEMBUFFER_API CGMemBuffer  
{
// Constructor / Destructor
public:
	CGMemBuffer();	// Default constructor
	CGMemBuffer(const CGMemBuffer& source); // Initialize this buffer with another buffer object
	CGMemBuffer(const BYTE* pbSourceBuffer, UINT nAllocatedSize, UINT nBufferSize = 0); // Initialize this buffer with a 
	virtual ~CGMemBuffer();

// Initialization
public:
	void			Attach(BYTE* pbBuffer, UINT nAllocatedSize, UINT nBufferSize = 0);
	BYTE*			Detach();	

// Attributes
public:
	enum EDataMode
	{
		eModeDefault = 0x0,			// The buffer will read/write data using default machine endian
		eModeBigEndian = 0x1,		// The buffer will read/write data using big endian
		eModeLittleEndian = 0x2,	// The buffer will read/write data using little endian
		eModeText = 0x4,			// All strings written in text mode ('\n' append at the end of the string)
	};


	GDECLARE_ERROR_MAP(CGMemBuffer)
	{
		eOverflow,			// Overflow buffer
		eNoBuffer,			// No buffer created
		eEOB,				// Reach End Of Buffer
		eNoSpace,			// No enough space to store data read
		eInvalidMode,		// Invalid mode to write this type of data
		eInvalidModeComb	// Invalid mode combination
	}
	GDECLARE_END_ERROR_MAP(CGMemBuffer)
	
	const BYTE*		GetData() const { return m_pbBuffer; };
	UINT			GetBufferSize() const { return m_nBufferSize; };

	// Current position in the buffer
	void			Rewind() { m_nCurrentPos = 0; };
	void			ToEnd() { m_nCurrentPos = m_nBufferSize; }
	UINT			GetCurrentPos() const { return m_nCurrentPos; };
	BOOL			SetCurrentPos(UINT nPosition);
	
	BOOL			IsEmpty();

	void			SetSizeIncrement(UINT nSize) { m_nSizeIncrement = nSize; }
	UINT			GetSizeIncrement() const { return m_nSizeIncrement;	}

	BOOL			SetDataMode(EDataMode eMode);

// Operations
public:
	void			CreateBuffer(UINT nInitialSize, UINT nSizeIncrement = GMB_DEFAULT_INCREMENT);
	void			Close();

	void			CopyIn(BYTE* pbBuffer, UINT nAllocatedSize, UINT nBufferSize = 0);


	// Write operations
	BOOL			WriteByte(BYTE bData);
	BOOL			WriteWord(WORD wData);
	BOOL			WriteDWord(DWORD dwData);
	BOOL			WriteInt(INT nData);
	BOOL			WriteFloat(FLOAT fData);
	BOOL			WriteDouble(DOUBLE dfData);
	BOOL			WriteString(PCSTR szData);
	BOOL			WriteBuffer(BYTE* pbBuffer, UINT nLength);
	BOOL			WriteBuffer(const CGMemBuffer& clBuffer);
	// Read operations
	BOOL			ReadByte(BYTE& bData);
	BOOL			ReadWord(WORD& rwData);
	BOOL			ReadDWord(DWORD& rdwData);
	BOOL			ReadInt(INT& rnData);
	BOOL			ReadFloat(FLOAT& rfData);
	BOOL			ReadDouble(DOUBLE& rdfData);
	BOOL			ReadString(PSTR szData, UINT nMaxRead);
	BOOL			ReadBuffer(BYTE* pbBuffer, UINT& nLength, UINT nMaxLength);
	BOOL			ReadBuffer(CGMemBuffer& clBuffer);
	// Debug (BG case 4882)
	enum ERawBufferToStringMode
	{
		eStringModeAscii = 0x0,		// The returned string will contain 1 ascii character per byte, and '?<hex code>?' if byte < 32 or byte >= 127
		eStringModeHex	 = 0x1		// The returned string will contain the hexadecimal value per byte
	};
	std::string		GetRawBufferToString(ERawBufferToStringMode eMode=eStringModeHex);

// Overridable
protected:
	virtual BYTE*	AllocBuffer(UINT nSizeToAlloc);
	virtual void	FreeBuffer(BYTE* pBuffer);

// Operators
public:
	CGMemBuffer& operator=(const CGMemBuffer& source);
	BOOL operator==(const CGMemBuffer& source);

// Implementation
protected:

	BYTE*		m_pbBuffer;	// Pointer to current data buffer
	UINT		m_nAllocatedBufferSize; // Size of memory allocated for the buffer
	UINT		m_nSizeIncrement; // Value to increment buffer size each time it's necessary
	UINT		m_nCurrentPos; // The current position in the buffer
	UINT		m_nBufferSize; // The size of datas written in the buffer
	EDataMode	m_eDataMode;


	void		ReallocBuffer();
	inline void	WriteData(BYTE* pbData, UINT nDataSize);
	inline void	ReadData(BYTE* pbData, UINT nDataSize);
};

#endif // !defined(AFX_GMEMBUFFER_H__FAB69F61_6685_4910_B2BB_ED8B03B92245__INCLUDED_)
