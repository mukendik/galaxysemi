// PkAutoBuffer.cpp: implementation of the CPkAutoBuffer class.
// Part of the PkZip library
////////////////////////////////////////////////////////////////////////////////

// Project headers
#include "gqtl_pkautobuffer.h"

// Standard headers
#include <memory.h> 

// QT headers

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPkAutoBuffer::CPkAutoBuffer()
{
	m_iSize = 0;
	m_pBuffer = NULL;	
}

//////////////////////////////////////////////////////////////////////
CPkAutoBuffer::CPkAutoBuffer(DWORD iSize, bool bZeroMemory)
{
	m_iSize = 0;
	m_pBuffer = NULL;
	Allocate(iSize, bZeroMemory);
}

//////////////////////////////////////////////////////////////////////
CPkAutoBuffer::CPkAutoBuffer(const CPkAutoBuffer& buffer)
{
	m_pBuffer = NULL;
	m_iSize = 0;

	if (buffer.m_pBuffer)
	{
		Allocate(buffer.m_iSize);
		memcpy(m_pBuffer, buffer.m_pBuffer, buffer.m_iSize);
	}	
}

//////////////////////////////////////////////////////////////////////
CPkAutoBuffer::~CPkAutoBuffer()
{
	Release();
}


//////////////////////////////////////////////////////////////////////
void CPkAutoBuffer::Release()
{
	if (m_pBuffer)
	{
		delete [] m_pBuffer;
		m_iSize = 0;
		m_pBuffer = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
char* CPkAutoBuffer::Allocate(DWORD iSize, bool bZeroMemory)
{
	if (iSize != m_iSize)
		Release();
	else
	{
		if (bZeroMemory)
			memset(m_pBuffer, 0, iSize); // zerowanie bufora
		return m_pBuffer;
	}

	if (iSize > 0)
	{
			m_pBuffer = new char [iSize];
			if (bZeroMemory)
				memset(m_pBuffer, 0, iSize); // zerowanie bufora
			m_iSize = iSize;
	}
	else 
		m_pBuffer = NULL;

	return m_pBuffer;
}


//////////////////////////////////////////////////////////////////////
CPkAutoBuffer& CPkAutoBuffer::operator=(const CPkAutoBuffer& buffer)
{
	if (this == &buffer)
		return *this;
	Release();
	if (buffer.m_pBuffer)
	{
		Allocate(buffer.m_iSize);
		memcpy(m_pBuffer, buffer.m_pBuffer, buffer.m_iSize);
	}
	return *this;
}



//////////////////////////////////////////////////////////////////////
CPkAutoBuffer::operator char*()
{
	return m_pBuffer;
}


//////////////////////////////////////////////////////////////////////
const char* CPkAutoBuffer::GetBuffer() const 
{
	return m_pBuffer;
}


//////////////////////////////////////////////////////////////////////
DWORD CPkAutoBuffer::GetSize() const 	
{
	return m_iSize;
}


//////////////////////////////////////////////////////////////////////
bool CPkAutoBuffer::IsAllocated() const	
{
	return (m_pBuffer != 0);
}

//////////////////////////////////////////////////////////////////////
void CPkAutoBuffer::SlashChange()	
{
	DWORD i;

	for(i=0; i<m_iSize; i++)
	{
		if(m_pBuffer[i] == CHAR_SEP_OLD)
			m_pBuffer[i] = CHAR_SEP;
	}
}

