// PkAutoBuffer.h: Interface for the CPkAutoBuffer class.
// Part of the PkZip library
////////////////////////////////////////////////////////////////////////////////


#if !defined(_WINAUTOBUFFER_H__DEC28C20_83FE_11D3_B7C3_EDEC47A8A86C__INCLUDED_)
#define _WINAUTOBUFFER_H__DEC28C20_83FE_11D3_B7C3_EDEC47A8A86C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Project headers
#include "gqtl_ansifile.h"

// Standard headers

// QT headers

//	A smart buffer freeing its contents on destruction.
class CPkAutoBuffer  
{
public:
	operator char*();

	const char*	GetBuffer() const;
	char*		Allocate(DWORD iSize, bool bZeroMemory = false);
	void		Release();
	DWORD		GetSize() const;
	bool		IsAllocated() const;
	void		SlashChange();	

	CPkAutoBuffer(DWORD iSize, bool bZeroMemory = false);
	CPkAutoBuffer();
	CPkAutoBuffer(const CPkAutoBuffer& buffer);
	virtual ~CPkAutoBuffer();
	
	CPkAutoBuffer& operator=(const CPkAutoBuffer& buffer);
protected:
	char* m_pBuffer;
	DWORD m_iSize;
};

#endif // !defined(_WINAUTOBUFFER_H__DEC28C20_83FE_11D3_B7C3_EDEC47A8A86C__INCLUDED_)
