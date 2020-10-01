// gmembuffer.cpp: implementation of the CGMemBuffer class.
//
// ----------------------------------------------------------------------------------------------------------
//
// Notes:
//		* This module manages a memory buffer. User can write formatted data to this buffer and read it back.
//			There is 2 mode to read/write data:
//				Binary mode (the default one), where you can read and write bytes, words, float, strings...
//				Text mode(need to be specified), where you can only read and write strings. In this mode,
//				each time you write a string in the buffer, a '\n' character is added at the end of the string.
//				Then when you read the string, the ReadString function will read character until it reaches
//				a '\n'.
//		* Optionaly, you can use this class to write in a buffer, data formatted in bigendian.
//		* You can provide your own function to allocate and free memory by subclassing AllocBuffer() and
//		  FreeBuffer() virtual functions (see notes on top of these 2 functions)
//
// Restrictions:
//		* Unlike a memory file class where several mem file can be opened and manage the same block of memory,
//		  you should not use several CGMemBuffer objects to access the same buffer.
//		* Also, this class is not thread safe, so several thread should not share and use the same
//		  CGMemBuffer object.
//
// Example:
//			1)
//				CGMemBuffer		buffer;
//
//				buffer.CreateBuffer(32);
//				buffer.WriteString("Hello the world!");
//
//				CHAR	szBuffer[256];
//				buffer.Rewind();
//				buffer.ReadString(szBuffer);
//				printf("String= %s\n",szBuffer);
//
//			The display text will be: String= Hello the world!);
//
//			2)
//				BYTE*			pbBuffer = new BYTE[256];
//				CGMemBuffer		buffer;
//
//				buffer.Attach(pbBuffer,256);
//				buffer.SetDataMode(eModeText);
//				buffer.WriteString("Hello the world!");
//				...
//				buffer.Close(); // Memory free
//
//
// ----------------------------------------------------------------------------------------------------------

#define _GALAXY_MEMBUFFER_EXPORTS_

#include "gstdl_utilsdll.h"
#include "gstdl_membuffer.h"

#include <gstdl_macro.h>

#include <string.h>
#include <iostream>
#include <sstream>

using namespace std;

#ifdef __sparc__
#define _BIGENDIAN
#endif

// ----------------------------------------------------------------------------------------------------------
// PRIVATE MACRO DEFINITION
// ----------------------------------------------------------------------------------------------------------
#define _CHECK_BINARY_VALIDITY()		if(m_pbBuffer == NULL)	{\
											GSET_ERROR(CGMemBuffer,eNoBuffer);\
											return FALSE;\
										}\
										if(m_eDataMode & eModeText)	{\
											GSET_ERROR(CGMemBuffer,eInvalidMode);\
											return FALSE;\
										}



GBEGIN_ERROR_MAP(CGMemBuffer)
	GMAP_ERROR(eOverflow,"Overflow memory buffer")
	GMAP_ERROR(eNoBuffer,"No buffer created")
	GMAP_ERROR(eEOB,"End of buffer reached")
	GMAP_ERROR(eNoSpace,"There is not enough space left to store read data")
	GMAP_ERROR(eInvalidModeComb,"Invalide mode combination")
	GMAP_ERROR(eInvalidMode,"You are not allowed to perform this operation in this mode")
GEND_ERROR_MAP(CGMemBuffer)

// ----------------------------------------------------------------------------------------------------------
// Construction/Destruction
// ----------------------------------------------------------------------------------------------------------

// Default constructor
CGMemBuffer::CGMemBuffer() : m_pbBuffer(NULL),m_nAllocatedBufferSize(0),
							 m_nSizeIncrement(GMB_DEFAULT_INCREMENT),m_nCurrentPos(0),
							 m_nBufferSize(0),m_eDataMode(eModeDefault)
{
}

// Copy constructor
CGMemBuffer::CGMemBuffer(const CGMemBuffer& source)
{
	operator=(source);
}

// Second copy constructor
// For nAllocatedSize and nBufferSize parameter, see notes for Attach() function.
CGMemBuffer::CGMemBuffer(const BYTE* pbSourceBuffer, UINT nAllocatedSize, UINT nBufferSize)
{
	m_pbBuffer = AllocBuffer(nAllocatedSize);
	memcpy(m_pbBuffer,pbSourceBuffer,nBufferSize);
	m_nBufferSize = nBufferSize;
	m_nAllocatedBufferSize = nAllocatedSize;
	m_nSizeIncrement = GMB_DEFAULT_INCREMENT;
	m_nCurrentPos = 0;
	m_eDataMode = eModeDefault;
}

CGMemBuffer::~CGMemBuffer()
{
	Close();
}

// Close this buffer: Free memory allocated for it, and reset data members
void CGMemBuffer::Close()
{
	if(m_pbBuffer != NULL)
	{
		FreeBuffer(m_pbBuffer);
		m_pbBuffer = NULL;
		m_nCurrentPos = 0;
		m_nBufferSize = m_nAllocatedBufferSize = 0;
	}
}

// Test if the buffer is empty or not
BOOL CGMemBuffer::IsEmpty()
{
	if(m_pbBuffer != NULL &&
	   m_nBufferSize > 0)
	   return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Attach the specified buffer to the current internal one, so
//				this buffer will be managed by the CGMemBuffer object.
//
// Arguments:
//		pbBuffer: A pointer to the memory buffer to attach to this object.
//		nAllocatedSize: Contains the size of the memory allocated for this buffer.
//						It could not be zero, and it is very important to specify
//						the correct value for this parameter.
//		nBufferSize: The size of datas in the buffer. It could be different than
//					 the total size of memory allocated for the buffer. For example
//					 you can have only X bytes of data really valid (initialized)
//					 in the buffer, but X*2 bytes of allocated memory, where the
//					 X other bytes are not valid data).
//					 If no data have been written in this buffer, you can put
//					 0 for this parameter: In this case, you could not call directly
//					 a ReadXXX function, because there is nothing to read in the buffer.
//					 The ReadXXX function will return FALSE. You will be only allowed
//					 to write something in the buffer.
//
// ----------------------------------------------------------------------------------------------------------
void CGMemBuffer::Attach(BYTE* pbBuffer, UINT nAllocatedSize, UINT nBufferSize /* = 0*/)
{
	ASSERT(nAllocatedSize > 0); // Memory should be allocated for this buffer
	ASSERT(m_pbBuffer == NULL); // Better to close the buffer before to create a new one to be clean
	Close(); // Juste in case m_pbBuffer is not NULL (situation not seen in debug mode during tests)

	m_pbBuffer = pbBuffer,
	m_nCurrentPos = 0;
	m_nBufferSize = nBufferSize;
	m_nAllocatedBufferSize = nAllocatedSize;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Detach the internal managed buffer from this object. This function
//				does not free memory. It just reset all internal data member and
//				return a pointer to the internal buffer, so it will not have any
//				link between the buffer and the object. So after to have call this
//				function, it will be the job of the user to free himself the buffer.
// ----------------------------------------------------------------------------------------------------------
BYTE* CGMemBuffer::Detach()
{
	BYTE*	pbReturnValue = m_pbBuffer;

	m_pbBuffer = NULL;
	m_nCurrentPos = 0;
	m_nBufferSize = m_nAllocatedBufferSize = 0;

	return pbReturnValue;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Change the current buffer mode
//
// Argument:
//		eMode: A combination of EDataMode flag.
//			   Note that for the moment, you can not combine any of the flags.
//			   Future version of this class will probably use other flags that could be combined.
// Return:
//		TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::SetDataMode(EDataMode eMode)
{
	if((eMode != eModeDefault) && (eMode != eModeBigEndian) && (eMode != eModeLittleEndian) && (eMode != eModeText))
	{
		GSET_ERROR(CGMemBuffer,eInvalidModeComb);
		return FALSE;
	}
	m_eDataMode = eMode;
	return TRUE;
}

// Change the current position in the buffer
BOOL CGMemBuffer::SetCurrentPos(UINT nPosition)
{
	if(m_pbBuffer == NULL)
	{
		GSET_ERROR(CGMemBuffer,eNoBuffer);
		return FALSE;
	}

	if(nPosition > m_nBufferSize)
	{
		GSET_ERROR(CGMemBuffer,eOverflow);
		return FALSE;
	}

	m_nCurrentPos = nPosition;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Create a new buffer.
//
// Argument(s) :
//      nInitialSize: The initial size of the created buffer
//		nSizeIncrement: The value to increment buffer size each time it's necessary.
//
// ----------------------------------------------------------------------------------------------------------
void CGMemBuffer::CreateBuffer(UINT nInitialSize, UINT nSizeIncrement /*= GMB_DEFAULT_INCREMENT*/)
{
	ASSERT(m_pbBuffer == NULL); // Better to close the buffer before to create a new one to be clean
	Close(); // Juste in case m_pbBuffer is not NULL (situation not seen in debug mode during tests)

	m_pbBuffer = AllocBuffer(nInitialSize);
	memset(m_pbBuffer,0,nInitialSize); // Reset data in buffer
	m_nAllocatedBufferSize = nInitialSize;
	m_nSizeIncrement = nSizeIncrement;
	m_nBufferSize = m_nCurrentPos = 0;
}

// Copy a memory buffer object to this one
CGMemBuffer& CGMemBuffer::operator=(const CGMemBuffer& source)
{
	Close(); // Destroy current buffer if necessary

	m_pbBuffer = AllocBuffer(source.m_nAllocatedBufferSize);
	memcpy(m_pbBuffer,source.m_pbBuffer,source.m_nAllocatedBufferSize);
	m_nBufferSize = source.m_nBufferSize;
	m_nAllocatedBufferSize = source.m_nAllocatedBufferSize;
	m_nSizeIncrement = source.m_nSizeIncrement;
	m_nCurrentPos = source.m_nCurrentPos;
	m_eDataMode = source.m_eDataMode;

	return *this;
}

void CGMemBuffer::CopyIn(BYTE* pbBuffer,
						 UINT nAllocatedSize,
						 UINT nBufferSize /* = 0 */)
{
	Close(); // Destroy current buffer if necessary

	m_pbBuffer = AllocBuffer(nAllocatedSize);
	memcpy(m_pbBuffer,pbBuffer,nAllocatedSize);
	m_nBufferSize = nBufferSize;
	m_nAllocatedBufferSize = nAllocatedSize;
}

// Compare this buffer with the specified one.
// Note that if the allocated size is different but all data in these
// buffers are identical, this operator return TRUE.
BOOL CGMemBuffer::operator==(const CGMemBuffer& source)
{
	if(m_nBufferSize != source.m_nBufferSize)
		return FALSE;

	if(memcmp(m_pbBuffer,source.m_pbBuffer,m_nBufferSize) != 0)
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write a byte value to the current buffer.
//
// Argument:
//		bData: The value to write in the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::WriteByte(BYTE bData)
{
	_CHECK_BINARY_VALIDITY();

	// Test if we've got enough empty space to add the data
	while(m_nCurrentPos + sizeof(BYTE) >= m_nAllocatedBufferSize)
		ReallocBuffer();

	*(m_pbBuffer+m_nCurrentPos) = bData;
	m_nCurrentPos += sizeof(BYTE);
	if(m_nCurrentPos > m_nBufferSize)
		m_nBufferSize = m_nCurrentPos;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write a word value to the current buffer.
//
// Argument:
//		wData: The value to write in the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::WriteWord(WORD wData)
{
	UINT	nWordSize = sizeof(WORD);

	_CHECK_BINARY_VALIDITY();

	// Test if we've got enough empty space to add the data
	while(m_nCurrentPos + nWordSize >= m_nAllocatedBufferSize)
		ReallocBuffer();

	WriteData((BYTE*)&wData,nWordSize);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write a dword value to the current buffer.
//
// Argument:
//		dwData: The value to write in the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::WriteDWord(DWORD dwData)
{
	UINT	nDWordSize = sizeof(DWORD);

	_CHECK_BINARY_VALIDITY();

	// Test if we've got enough empty space to add the data
	while(m_nCurrentPos + nDWordSize >= m_nAllocatedBufferSize)
		ReallocBuffer();

	WriteData((BYTE*)&dwData,nDWordSize);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write an integer value to the current buffer.
//
// Argument:
//		iData: The value to write in the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::WriteInt(INT iData)
{
	UINT	nIntSize = sizeof(INT);

	_CHECK_BINARY_VALIDITY();

	// Test if we've got enough empty space to add the data
	while(m_nCurrentPos + nIntSize >= m_nAllocatedBufferSize)
		ReallocBuffer();

	WriteData((BYTE*)&iData,nIntSize);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write a float value to the current buffer.
//
// Argument:
//		fData: The value to write in the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::WriteFloat(FLOAT fData)
{
	UINT	nFloatSize = sizeof(FLOAT);

	_CHECK_BINARY_VALIDITY();

	// Test if we've got enough empty space to add the data
	while(m_nCurrentPos + nFloatSize >= m_nAllocatedBufferSize)
		ReallocBuffer();

	WriteData((BYTE*)&fData,nFloatSize);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write a double value to the current buffer.
//
// Argument:
//		dfData: The value to write in the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::WriteDouble(DOUBLE dfData)
{
	UINT	nDoubleSize = sizeof(DOUBLE);

	_CHECK_BINARY_VALIDITY();

	// Test if we've got enough empty space to add the data
	while(m_nCurrentPos + nDoubleSize >= m_nAllocatedBufferSize)
		ReallocBuffer();

	WriteData((BYTE*)&dfData,nDoubleSize);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write a string to the current buffer.
//
// Argument:
//		szData: The value to write in the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::WriteString(PCSTR szData)
{
	UINT		nStringSize = strlen(szData),nLoop;

	if(m_pbBuffer == NULL)
	{
		GSET_ERROR(CGMemBuffer,eNoBuffer);
		return FALSE;
	}

	// Write the string in text mode ?
	if(m_eDataMode & eModeText)
	{
		// Test if we've got enough empty space to add the data
		while(m_nCurrentPos + nStringSize + 1 >= m_nAllocatedBufferSize)
			ReallocBuffer();

		for(nLoop=0;nLoop<nStringSize;nLoop++)
			*(m_pbBuffer+m_nCurrentPos+nLoop) = (BYTE)*(szData+nLoop);

		m_nCurrentPos += nStringSize;
		// Append a '\n' at the end of the string (it is the delimiter)
		*(m_pbBuffer+m_nCurrentPos) = (BYTE)'\n';
		m_nCurrentPos++;
	}
	else // Write string in binary mode
	{
		// Add string size
		if(!WriteDWord((DWORD)nStringSize))
			return FALSE;

		// Test if we've got enough empty space to add the data
		while(m_nCurrentPos + nStringSize >= m_nAllocatedBufferSize)
			ReallocBuffer();

		for(nLoop=0;nLoop<nStringSize;nLoop++)
			*(m_pbBuffer+m_nCurrentPos+nLoop) = (BYTE)*(szData+nLoop);

		m_nCurrentPos += nStringSize;
	}

	if(m_nCurrentPos > m_nBufferSize)
		m_nBufferSize = m_nCurrentPos;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Write a binary buffer to the current buffer.
//
// Argument:
//		pbBuffer: The binary buffer to write.
//		nLength: The length of the buffer to write.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::WriteBuffer(BYTE* pbBuffer, UINT nLength)
{
	_CHECK_BINARY_VALIDITY();

	// Add buffer size information
	if(!WriteDWord((DWORD)nLength))
		return FALSE;

	// Test if we've got enough empty space to add the data
	while(m_nCurrentPos + nLength >= m_nAllocatedBufferSize)
		ReallocBuffer();

	for(UINT nLoop=0;nLoop < nLength;nLoop++)
		*(m_pbBuffer+m_nCurrentPos+nLoop) = *(pbBuffer+nLoop);

	m_nCurrentPos += nLength;
	if(m_nCurrentPos > m_nBufferSize)
		m_nBufferSize = m_nCurrentPos;

	return TRUE;
}

// Same function, but the source is another CGMembuffer object
BOOL CGMemBuffer::WriteBuffer(const CGMemBuffer& clBuffer)
{
	_CHECK_BINARY_VALIDITY();

	if(clBuffer.m_pbBuffer == NULL)
	{
		GSET_ERROR(CGMemBuffer,eNoBuffer);
		return FALSE;
	}

	// Add buffer size information
	if(!WriteDWord((DWORD)clBuffer.m_nBufferSize))
		return FALSE;

	// Test if we've got enough empty space to add the data
	while(m_nCurrentPos + clBuffer.m_nBufferSize >= m_nAllocatedBufferSize)
		ReallocBuffer();

	for(UINT nLoop=0;nLoop < clBuffer.m_nBufferSize;nLoop++)
		*(m_pbBuffer+m_nCurrentPos+nLoop) = *(clBuffer.m_pbBuffer+nLoop);

	m_nCurrentPos += clBuffer.m_nBufferSize;
	if(m_nCurrentPos > m_nBufferSize)
		m_nBufferSize = m_nCurrentPos;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read a byte value from the current buffer at the current position.
//
// Argument:
//		rbData: Received the value read from the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::ReadByte(BYTE& rbData)
{
	_CHECK_BINARY_VALIDITY();

	if(m_nCurrentPos+sizeof(BYTE) > m_nBufferSize)
	{
		GSET_ERROR(CGMemBuffer,eEOB);
		return FALSE;
	}

	rbData = *(m_pbBuffer+m_nCurrentPos);
	m_nCurrentPos += sizeof(BYTE);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read a word value from the current buffer at the current position.
//
// Argument:
//		rwData: Received the value read from the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::ReadWord(WORD& rwData)
{
    UINT		nWordSize = sizeof(WORD);

	_CHECK_BINARY_VALIDITY();

	if(m_nCurrentPos+nWordSize > m_nBufferSize)
	{
		GSET_ERROR(CGMemBuffer,eEOB);
		return FALSE;
	}

	ReadData((BYTE*)&rwData,nWordSize);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read a dword value from the current buffer at the current position.
//
// Argument:
//		rdwData: Received the value read from the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::ReadDWord(DWORD& rdwData)
{
    UINT		nDWordSize = sizeof(DWORD);

	_CHECK_BINARY_VALIDITY();

	if(m_nCurrentPos+nDWordSize > m_nBufferSize)
	{
		GSET_ERROR(CGMemBuffer,eEOB);
		return FALSE;
	}

	ReadData((BYTE*)&rdwData,nDWordSize);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read an integer value from the current buffer at the current position.
//
// Argument:
//		riData: Received the value read from the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::ReadInt(INT& riData)
{
	UINT		nIntSize = sizeof(INT);

	_CHECK_BINARY_VALIDITY();

	if(m_nCurrentPos+nIntSize > m_nBufferSize)
	{
		GSET_ERROR(CGMemBuffer,eEOB);
		return FALSE;
	}

	ReadData((BYTE*)&riData,nIntSize);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read a float value from the current buffer at the current position.
//
// Argument:
//		rfData: Received the value read from the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::ReadFloat(FLOAT& rfData)
{
	UINT		nFloatSize = sizeof(FLOAT);

	_CHECK_BINARY_VALIDITY();

	if(m_nCurrentPos+nFloatSize > m_nBufferSize)
	{
		GSET_ERROR(CGMemBuffer,eEOB);
		return FALSE;
	}
	ReadData((BYTE*)&rfData,nFloatSize);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read a double value from the current buffer at the current position.
//
// Argument:
//		rdfData: Received the value read from the buffer.
//
// Return: TRUE if successful; otherwise return FALSE.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::ReadDouble(DOUBLE& rdfData)
{
	UINT		nDoubleSize = sizeof(DOUBLE);

	_CHECK_BINARY_VALIDITY();

	if(m_nCurrentPos+nDoubleSize > m_nBufferSize)
	{
		GSET_ERROR(CGMemBuffer,eEOB);
		return FALSE;
	}
	ReadData((BYTE*)&rdfData,nDoubleSize);

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read a string from the current buffer at the current position.
//
// Argument:
//		szData: Received the string read from the buffer.
//		nMaxRead: The maximum number of characters to store in szData.
//
// Return: TRUE if successful; FALSE if:
//				1) The string to read exceed the maximum size specified in iMaxRead.
//				2) The buffer is NULL or no data can be read at the current position.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::ReadString(PSTR szData, UINT nMaxRead)
{
    UINT  nStringSizeNoRef;
	UINT&	nStringSize = nStringSizeNoRef;
    UINT  nRead = 0;

	if(m_pbBuffer == NULL)
	{
		GSET_ERROR(CGMemBuffer,eNoBuffer);
		return FALSE;
	}

	// Read the string in text mode?
	if(m_eDataMode & eModeText)
	{
		// While we have not reach end of line, and we have not reach end of read buffer,
		// and we have not reach end of destination buffer
		while(*(m_pbBuffer+m_nCurrentPos+nRead) != '\n' &&
			  m_nCurrentPos + nRead < m_nBufferSize &&
			  nRead < nMaxRead)
		{
			*(szData+nRead) = *(m_pbBuffer+m_nCurrentPos+nRead);
			nRead++;
		}
		if(nRead == nMaxRead)
		{
			GSET_ERROR(CGMemBuffer,eNoSpace);
			return FALSE;
		}
		if(m_nCurrentPos + nRead == m_nBufferSize)
		{
			GSET_ERROR(CGMemBuffer,eEOB);
			return FALSE;
		}
		m_nCurrentPos += nRead;
		*(szData+nRead) = '\0';
		m_nCurrentPos++; // Skip '\n'
	}
	else // Read string in binary mode
    {
        if(ReadDWord((DWORD&)nStringSize) == FALSE)
            return FALSE;

		if(m_nCurrentPos+nStringSize > m_nBufferSize)
		{
			GSET_ERROR(CGMemBuffer,eEOB);
			return FALSE;
		}

		for(UINT nLoop=0;nLoop<nStringSize;nLoop++)
			*(szData+nLoop) = *(m_pbBuffer+m_nCurrentPos+nLoop);

		m_nCurrentPos += nStringSize;

		*(szData+nStringSize) = '\0'; // Close the string
	}

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Read a binary buffer from the current buffer at the current position.
//
// Argument:
//		pbBuffer: Received the buffer read from the buffer.
//		nLength: Received the length of the buffer read
//		nMaxLength: The maximum length of the buffer to read.
//
// Return: TRUE if successful; FALSE if:
//				1) The buffer to read exceed the maximum size specified in nMaxLength.
//				2) The buffer is NULL or no data can be read at the current position.
// ----------------------------------------------------------------------------------------------------------
BOOL CGMemBuffer::ReadBuffer(BYTE* pbBuffer, UINT& nLength, UINT nMaxLength)
{
	_CHECK_BINARY_VALIDITY();
	// Retrieve the buffer length
	if(ReadDWord((DWORD&)nLength) == FALSE)
		return FALSE;

	// Test if the buffer to read does not exceed the maximum length allowed
	if(nLength > nMaxLength)
	{
		GSET_ERROR(CGMemBuffer,eNoSpace);
		return FALSE;
	}
	// Reach end of buffer? (Should not happen)
	if(m_nCurrentPos+nLength > m_nBufferSize)
	{
		GSET_ERROR(CGMemBuffer,eEOB);
		return FALSE;
	}

	for(UINT nLoop=0;nLoop<nLength;nLoop++)
		*(pbBuffer+nLoop) = *(m_pbBuffer+m_nCurrentPos+nLoop);

	m_nCurrentPos += nLength;

	return TRUE;
}

// Same function, the the buffer read is put in another CGMemBuffer object
BOOL CGMemBuffer::ReadBuffer(CGMemBuffer& clBuffer)
{
  UINT  nLengthNoRef;
	UINT&	nLength = nLengthNoRef;

	_CHECK_BINARY_VALIDITY();
	// Retrieve the buffer length
	if(ReadDWord((DWORD&)nLength) == FALSE)
		return FALSE;

	// Reach end of buffer? (Should not happen)
	if(m_nCurrentPos+nLength > m_nBufferSize)
	{
		GSET_ERROR(CGMemBuffer,eEOB);
		return FALSE;
	}

	clBuffer.Close();
	clBuffer.CreateBuffer(nLength);

	for(UINT nLoop=0;nLoop<nLength;nLoop++)
		*(clBuffer.m_pbBuffer+nLoop) = *(m_pbBuffer+m_nCurrentPos+nLoop);

	m_nCurrentPos += nLength;
	clBuffer.m_nBufferSize = nLength;

	return TRUE;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Reallocate memory for the new buffer.
// ----------------------------------------------------------------------------------------------------------
void CGMemBuffer::ReallocBuffer()
{
	BYTE*	pbNewBuffer;

	pbNewBuffer = AllocBuffer(m_nAllocatedBufferSize + m_nSizeIncrement);
	memcpy(pbNewBuffer,m_pbBuffer,m_nAllocatedBufferSize);

	FreeBuffer(m_pbBuffer);
	m_pbBuffer = pbNewBuffer;

	m_nAllocatedBufferSize += m_nSizeIncrement; // New buffer size
}

// ----------------------------------------------------------------------------------------------------------
// Description : Write bytes in big endian format.
//
// Argument(s) :
//      BYTE* pbData : A pointer to the data buffer.
//      UINT nDataSize : The number of bytes to write.
//
// ----------------------------------------------------------------------------------------------------------
void CGMemBuffer::WriteData(BYTE* pbData, UINT nDataSize)
{
	UINT	 i;

	if(m_eDataMode & eModeBigEndian)
	{
		// If big endian mode is set, write data in big endian format regarding current platform
		for(i=0;i<nDataSize;i++)
#ifdef _BIGENDIAN
			*(m_pbBuffer+m_nCurrentPos+i) = *(pbData+i);
#else
			*(m_pbBuffer+m_nCurrentPos+i) = *(pbData+nDataSize-i-1);
#endif
	}
	else if(m_eDataMode & eModeLittleEndian)
	{
		// If little endian mode is set, write data in little endian format regarding current platform
		for(i=0;i<nDataSize;i++)
#ifdef _BIGENDIAN
			*(m_pbBuffer+m_nCurrentPos+i) = *(pbData+nDataSize-i-1);
#else
			*(m_pbBuffer+m_nCurrentPos+i) = *(pbData+i);
#endif
	}
	else
	{	// No big endian mode, write data as it is
		for(i=0;i<nDataSize;i++)
			*(m_pbBuffer+m_nCurrentPos+i) = *(pbData+i);
	}

	m_nCurrentPos += nDataSize;

	if(m_nCurrentPos > m_nBufferSize)
		m_nBufferSize = m_nCurrentPos;
}


// ----------------------------------------------------------------------------------------------------------
// Description : Read bytes in big endian format.
//
// Argument(s) :
//      BYTE* pbData : A pointer to the data buffer.
//      UINT nDataSize : The number of bytes to read.
//
// ----------------------------------------------------------------------------------------------------------
void CGMemBuffer::ReadData(BYTE* pbData, UINT nDataSize)
{
	UINT	 i;

	if(m_eDataMode & eModeBigEndian)
	{
		for(i=0;i<nDataSize;i++)
#ifdef _BIGENDIAN
			*(pbData+i) = *(m_pbBuffer+m_nCurrentPos+i);
#else
			*(pbData+i) = *(m_pbBuffer+m_nCurrentPos+nDataSize-i-1);
#endif
	}
	if(m_eDataMode & eModeLittleEndian)
	{
		for(i=0;i<nDataSize;i++)
#ifdef _BIGENDIAN
			*(pbData+i) = *(m_pbBuffer+m_nCurrentPos+nDataSize-i-1);
#else
			*(pbData+i) = *(m_pbBuffer+m_nCurrentPos+i);
#endif
	}
	else
	{
		for(i=0;i<nDataSize;i++)
			*(pbData+i) = *(m_pbBuffer+m_nCurrentPos+i);
	}
	m_nCurrentPos += nDataSize;
}

// ----------------------------------------------------------------------------------------------------------
// Virual function to manage memory allocation.
// The default implementation use new and delete operators to allocate the array of byte.
// If you want, you can derive a new class from CGMemBuffer, and overwrite AllocBuffer and FreeBuffer to
// use malloc and free functions for example.
//
// Example:
//		CMyBuffer is derived from CGMemBuffer; you also need to overwrite the destructor like this:
//
//			CMyBuffer::~CMyBuffer()
//			{
//				Close();
//			}
//
//		Then, subclass AllocBuffer and FreeBuffer:
//
//			BYTE* CMyBuffer::AllocBuffer(UINT nSizeToAlloc)
//			{
//				return (BYTE*)malloc(nSizeToAlloc);
//			}
//
//			void CMyBuffer::FreeBuffer(BYTE* pBuffer)
//			{
//				free(pBuffer);
//			}
// ----------------------------------------------------------------------------------------------------------

BYTE* CGMemBuffer::AllocBuffer(UINT nSizeToAlloc)
{
	return new BYTE[nSizeToAlloc];
}

void CGMemBuffer::FreeBuffer(BYTE* pBuffer)
{
	delete [] pBuffer;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Return a string representing the raw data.
// The returned string has following format:
// [Size=<size>][Mode=<mode>]: <string>
//
// Argument(s) :
//      ERawBufferToStringMode eMode : the mode in which each raw data byte should be represented in the string.
// ----------------------------------------------------------------------------------------------------------
string CGMemBuffer::GetRawBufferToString(ERawBufferToStringMode eMode/*=eStringModeHex*/)
{
	stringstream	sstm;
	unsigned int	uiIndex=0;
	char			szToken[10];

	// Init string
	sstm << "[Size=";
	sstm << m_nBufferSize;
	sstm << "][Mode =";
	switch(eMode)
	{
		case eStringModeAscii:
			sstm << "Ascii]: ";
			for(uiIndex=0; uiIndex<m_nBufferSize; uiIndex++)
			{
				if(((int)(m_pbBuffer[uiIndex]&0xff) < 32) || ((int)(m_pbBuffer[uiIndex]&0xff) >= 127))
					sprintf(szToken, "?%02x?", m_pbBuffer[uiIndex]&0xff);
				else
					sprintf(szToken, "%c", m_pbBuffer[uiIndex]&0xff);
				sstm << szToken;
			}
			break;
		default:
		case eStringModeHex:
			sstm << "Hex]  : ";
			for(uiIndex=0; uiIndex<m_nBufferSize; uiIndex++)
			{
				sprintf(szToken, "%02x", m_pbBuffer[uiIndex]&0xff);
				sstm << szToken;
			}
			break;
	}
	return sstm.str();
}
