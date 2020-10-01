/* MD5Checksum.cpp: implementation of the MD5Checksum class. */
#define _MD5FileModule_

/****************************************************************************************
This software is derived from the RSA Data Security, Inc. MD5 Message-Digest Algorithm. 
Incorporation of this statement is a condition of use; please see the RSA
Data Security Inc copyright notice below:-

Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
rights reserved.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.
License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.
License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.
RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
*****************************************************************************************/

/****************************************************************************************
This implementation of the RSA MD5 Algorithm was written by Langfine Ltd 
(www.langfine.com).

Langfine Ltd makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

In addition to the above, Langfine make no warrant or assurances regarding the 
accuracy of this implementation of the MD5 checksum algorithm nor any assurances regarding
its suitability for any purposes.

This implementation may be used freely provided that Langfine is credited
in a copyright or similar notices (eg, RSA MD5 Algorithm implemented by Langfine
Ltd.) and provided that the RSA Data Security notices are complied with.
*/


#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "gstdl_md5checksum_c.h"
#include "gstdl_md5checksumdefines_c.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BYTE  m_lpszBuffer[64];		/*input buffer*/
ULONG m_nCount[2];				/*number of bits, modulo 2^64 (lsb first)*/
ULONG m_lMD5[4];				/*MD5 checksum*/

/*****************************************************************************************/
/* private functions                                                                     */
/*****************************************************************************************/

/*constructor/destructor */
void Initialize();

#if defined(ANSI)
void Transform(BYTE Block[64]);
void Update(BYTE* Input,ULONG nInputLen);
int Final(char* szResult);
DWORD RotateLeft(DWORD x, int n);
void gstd_FF(DWORD* A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);
void gstd_GG(DWORD *A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);
void gstd_HH(DWORD* A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);
void gstd_II(DWORD* A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);

/*utility functions*/
void DWordToByte(BYTE* Output,DWORD*  Input,UINT nLength);
void ByteToDWord(DWORD* Output, BYTE* Input, UINT nLength);
#else

/*RSA MD5 implementation*/
void Transform();
void Update();
int Final();
DWORD RotateLeft();
void gstd_FF();
void gstd_GG();
void gstd_HH();
void gstd_II();

/*utility functions*/
void DWordToByte();
void ByteToDWord();
#endif


/*****************************************************************************************
FUNCTION:		GetMD5File
DETAILS:		static, public
DESCRIPTION:	Gets the MD5 checksum for a specified file
RETURNS:		CString : the hexadecimal MD5 checksum for the specified file
ARGUMENTS:		CString& strFilePath : the full pathname of the specified file
NOTES:			Provides an interface to the CMD5Checksum class. 'strFilePath' name should 
				hold the full pathname of the file, eg C:\My Documents\Arcticle.txt.
				NB. If any problems occur with opening or reading this file, a CFileException
				will be thrown; callers of this function should be ready to catch this 
				exception.
*****************************************************************************************/
#if defined(ANSI)
int GetMD5(char* szFilePath, char* szResult)
#else
int GetMD5(szFilePath, szResult)
char* szFilePath;
char* szResult;
#endif
{
	/*open the file as a binary file in readonly mode, denying write access */
	FILE* pFile;
	//int	  bResult;
	pFile = fopen(szFilePath, "rb");
	if(pFile == NULL)
	{
		/* If the file does not exist */
		if(errno == ENOENT)
			return MD5_FILE_ERROR_EXIST;
		else
		if(errno == EACCES)
			return MD5_FILE_ERROR_ACCES;
		else
			return MD5_FILE_ERROR;
	}
	/*the file has been successfully opened, so now get and return its checksum*/ 
	/*bResult =*/ GetMD5File(pFile, szResult);
	if(fclose(pFile))
		return MD5_FILE_ERROR_CLOSE;

	return MD5_OK;  //TODO: Should return md5 value ?
}


/*****************************************************************************************
FUNCTION:		GetMD5File
DETAILS:		static, public
DESCRIPTION:	Gets the MD5 checksum for a specified file
RETURNS:		CString : the hexadecimal MD5 checksum for the specified file
ARGUMENTS:		CFile& File : the specified file
NOTES:			Provides an interface to the CMD5Checksum class. 'File' should be open in 
				binary readonly mode before calling this function. 
				NB. Callers of this function should be ready to catch any CFileException
				thrown by the CFile functions
*****************************************************************************************/
#if defined(ANSI)
int GetMD5File(FILE* pFile, char* szResult)
#else
int GetMD5File(pFile, szResult)
FILE* pFile;
char* szResult;
#endif
{
	int nLength = 0;				/*number of bytes read from the file*/
	int nBufferSize = 1024;	/*checksum the file in blocks of 1024 bytes*/
	BYTE Buffer[1024];		/*buffer for data read from the file*/

	if(pFile == NULL)
		return MD5_FILE_ERROR;

	Initialize();					/*checksum object	*/
	/*checksum the file in blocks of 1024 bytes*/
	while ((nLength = fread(Buffer, sizeof(BYTE), nBufferSize, pFile)) > 0 )
	{
		Update( Buffer, nLength );
	}

	/*finalise the checksum and return it*/ 
	return Final(szResult);
}


/*****************************************************************************************
FUNCTION:		GetMD5File
DETAILS:		static, public
DESCRIPTION:	Gets the MD5 checksum for data in a unsigned char array
RETURNS:		CString : the hexadecimal MD5 checksum for the specified data
ARGUMENTS:		unsigned char* pBuf  :	pointer to the unsigned char array
				unsigned int nLength :	number of unsigned chars of data to be checksumed
NOTES:			Provides an interface to the CMD5Checksum class. Any data that can
				be cast to a unsigned char array of known length can be checksummed by this
				function. Typically, CString and char arrays will be checksumed, 
				although this function can be used to check the integrity of any unsigned char array. 
				A buffer of zero length can be checksummed; all buffers of zero length 
				will return the same checksum. 
*****************************************************************************************/
#if defined(ANSI)
int GetMD5Buffer(BYTE* pBuf, UINT nLength, char *szResult)
#else
int GetMD5Buffer(pBuf, nLength, szResult)
BYTE* pBuf;
UINT nLength;
char* szResult;
#endif
{
	/*calculate and return the checksum*/
	Initialize();
	Update( pBuf, nLength );
	return Final(szResult);
}


/*****************************************************************************************
FUNCTION:		RotateLeft
DETAILS:		private
DESCRIPTION:	Rotates the bits in a 32 bit DWORD left by a specified amount
RETURNS:		The rotated DWORD 
ARGUMENTS:		DWORD x : the value to be rotated
				int n   : the number of bits to rotate by
*****************************************************************************************/
#if defined(ANSI)
DWORD RotateLeft(DWORD x, int n)
#else
DWORD RotateLeft(x, n)
DWORD x;
int n;
#endif
{
	/*rotate and return x*/
	return (x << n) | (x >> (32-n));
}


/*****************************************************************************************
FUNCTION:		gstd_FF
DETAILS:		protected
DESCRIPTION:	Implementation of basic MD5 transformation algorithm
RETURNS:		none
ARGUMENTS:		DWORD A, B, C, D : Current (partial) checksum
				DWORD X           : Input data
				DWORD S			  : MD5_SXX Transformation constant
				DWORD T			  :	MD5_TXX Transformation constant
NOTES:			None
*****************************************************************************************/
#if defined(ANSI)
void gstd_FF(DWORD* A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T)
#else
void gstd_FF(A, B, C, D, X, S, T)
DWORD* A;
DWORD B;
DWORD C;
DWORD D;
DWORD X;
DWORD S;
DWORD T;
#endif
{
	DWORD F = (B & C) | (~B & D);
	*A += F + X + T;
	*A = RotateLeft(*A, S);
	*A += B;
}


/*****************************************************************************************
FUNCTION:		gstd_GG
DETAILS:		protected
DESCRIPTION:	Implementation of basic MD5 transformation algorithm
RETURNS:		none
ARGUMENTS:		DWORD &A, B, C, D : Current (partial) checksum
				DWORD X           : Input data
				DWORD S			  : MD5_SXX Transformation constant
				DWORD T			  :	MD5_TXX Transformation constant
NOTES:			None
*****************************************************************************************/
#if defined(ANSI)
void gstd_GG(DWORD *A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T)
#else
void gstd_GG(A, B, C, D, X, S, T)
DWORD *A;
DWORD B;
DWORD C;
DWORD D;
DWORD X;
DWORD S;
DWORD T;
#endif
{
	DWORD G = (B & D) | (C & ~D);
	*A += G + X + T;
	*A = RotateLeft(*A, S);
	*A += B;
}


/*****************************************************************************************
FUNCTION:		gstd_HH
DETAILS:		protected
DESCRIPTION:	Implementation of basic MD5 transformation algorithm
RETURNS:		none
ARGUMENTS:		DWORD &A, B, C, D : Current (partial) checksum
				DWORD X           : Input data
				DWORD S			  : MD5_SXX Transformation constant
				DWORD T			  :	MD5_TXX Transformation constant
NOTES:			None
*****************************************************************************************/
#if defined(ANSI)
void gstd_HH(DWORD* A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T)
#else
void gstd_HH(A, B, C, D, X, S, T)
DWORD* A;
DWORD B;
DWORD C;
DWORD D;
DWORD X;
DWORD S;
DWORD T;
#endif
{
	DWORD H = (B ^ C ^ D);
	*A += H + X + T;
	*A = RotateLeft(*A, S);
	*A += B;
}


/*****************************************************************************************
FUNCTION:		gstd_II
DETAILS:		protected
DESCRIPTION:	Implementation of basic MD5 transformation algorithm
RETURNS:		none
ARGUMENTS:		DWORD &A, B, C, D : Current (partial) checksum
				DWORD X           : Input data
				DWORD S			  : MD5_SXX Transformation constant
				DWORD T			  :	MD5_TXX Transformation constant
NOTES:			None
*****************************************************************************************/
#ifdef ANSI
void gstd_II(DWORD* A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T)
#else
void gstd_II(A, B, C, D, X, S, T)
DWORD* A;
DWORD B;
DWORD C;
DWORD D;
DWORD X;
DWORD S;
DWORD T;
#endif
{
	DWORD I = (C ^ (B | ~D));
	*A += I + X + T;
	*A = RotateLeft(*A, S);
	*A += B;
}


/*****************************************************************************************
FUNCTION:		ByteToDWord
DETAILS:		private
DESCRIPTION:	Transfers the data in an 8 bit array to a 32 bit array
RETURNS:		void
ARGUMENTS:		DWORD* Output : the 32 bit (unsigned long) destination array 
				unsigned char* Input	  : the 8 bit (unsigned char) source array
				unsigned int nLength  : the number of 8 bit data items in the source array
NOTES:			Four unsigned charS from the input array are transferred to each unsigned int entry
				of the output array. The first unsigned char is transferred to the bits (0-7) 
				of the output unsigned int, the second unsigned char to bits 8-15 etc. 
				The algorithm assumes that the input array is a multiple of 4 bytes long
				so that there is a perfect fit into the array of 32 bit words.
*****************************************************************************************/
#if defined(ANSI)
void ByteToDWord(DWORD* Output, BYTE* Input, UINT nLength)
#else
void ByteToDWord(Output, Input, nLength)
DWORD* Output;
BYTE* Input;
UINT nLength;
#endif
{
	/*initialisations*/
	UINT i=0;	/*index to Output array*/
	UINT j=0;	/*index to Input array*/

	/*transfer the data by shifting and copying*/
	for ( ; j < nLength; i++, j += 4)
	{
		Output[i] = (ULONG)Input[j]			| 
					(ULONG)Input[j+1] << 8	| 
					(ULONG)Input[j+2] << 16 | 
					(ULONG)Input[j+3] << 24;
	}
}

/*****************************************************************************************
FUNCTION:		Transform
DETAILS:		protected
DESCRIPTION:	MD5 basic transformation algorithm;  transforms 'm_lMD5'
RETURNS:		void
ARGUMENTS:		unsigned char Block[64]
NOTES:			An MD5 checksum is calculated by four rounds of 'Transformation'.
				The MD5 checksum currently held in m_lMD5 is merged by the 
				transformation process with data passed in 'Block'.  
*****************************************************************************************/
#if defined(ANSI)
void Transform(BYTE Block[64])
#else
void Transform(Block)
BYTE Block[64];
#endif
{
	/*initialise local data with current checksum*/
	ULONG a = m_lMD5[0];
	ULONG b = m_lMD5[1];
	ULONG c = m_lMD5[2];
	ULONG d = m_lMD5[3];

	/*copy unsigned charS from input 'Block' to an array of ULONGS 'X'*/
	ULONG X[16];
	ByteToDWord( X, Block, 64 );

	/*Perform Round 1 of the transformation*/
	gstd_FF (&a, b, c, d, X[ 0], MD5_S11, MD5_T01); 
	gstd_FF (&d, a, b, c, X[ 1], MD5_S12, MD5_T02); 
	gstd_FF (&c, d, a, b, X[ 2], MD5_S13, MD5_T03); 
	gstd_FF (&b, c, d, a, X[ 3], MD5_S14, MD5_T04); 
	gstd_FF (&a, b, c, d, X[ 4], MD5_S11, MD5_T05); 
	gstd_FF (&d, a, b, c, X[ 5], MD5_S12, MD5_T06); 
	gstd_FF (&c, d, a, b, X[ 6], MD5_S13, MD5_T07); 
	gstd_FF (&b, c, d, a, X[ 7], MD5_S14, MD5_T08); 
	gstd_FF (&a, b, c, d, X[ 8], MD5_S11, MD5_T09); 
	gstd_FF (&d, a, b, c, X[ 9], MD5_S12, MD5_T10); 
	gstd_FF (&c, d, a, b, X[10], MD5_S13, MD5_T11); 
	gstd_FF (&b, c, d, a, X[11], MD5_S14, MD5_T12); 
	gstd_FF (&a, b, c, d, X[12], MD5_S11, MD5_T13); 
	gstd_FF (&d, a, b, c, X[13], MD5_S12, MD5_T14); 
	gstd_FF (&c, d, a, b, X[14], MD5_S13, MD5_T15); 
	gstd_FF (&b, c, d, a, X[15], MD5_S14, MD5_T16); 

	/*Perform Round 2 of the transformation*/
	gstd_GG (&a, b, c, d, X[ 1], MD5_S21, MD5_T17); 
	gstd_GG (&d, a, b, c, X[ 6], MD5_S22, MD5_T18); 
	gstd_GG (&c, d, a, b, X[11], MD5_S23, MD5_T19); 
	gstd_GG (&b, c, d, a, X[ 0], MD5_S24, MD5_T20); 
	gstd_GG (&a, b, c, d, X[ 5], MD5_S21, MD5_T21); 
	gstd_GG (&d, a, b, c, X[10], MD5_S22, MD5_T22); 
	gstd_GG (&c, d, a, b, X[15], MD5_S23, MD5_T23); 
	gstd_GG (&b, c, d, a, X[ 4], MD5_S24, MD5_T24); 
	gstd_GG (&a, b, c, d, X[ 9], MD5_S21, MD5_T25); 
	gstd_GG (&d, a, b, c, X[14], MD5_S22, MD5_T26); 
	gstd_GG (&c, d, a, b, X[ 3], MD5_S23, MD5_T27); 
	gstd_GG (&b, c, d, a, X[ 8], MD5_S24, MD5_T28); 
	gstd_GG (&a, b, c, d, X[13], MD5_S21, MD5_T29); 
	gstd_GG (&d, a, b, c, X[ 2], MD5_S22, MD5_T30); 
	gstd_GG (&c, d, a, b, X[ 7], MD5_S23, MD5_T31); 
	gstd_GG (&b, c, d, a, X[12], MD5_S24, MD5_T32); 

	/*Perform Round 3 of the transformation*/
	gstd_HH (&a, b, c, d, X[ 5], MD5_S31, MD5_T33); 
	gstd_HH (&d, a, b, c, X[ 8], MD5_S32, MD5_T34); 
	gstd_HH (&c, d, a, b, X[11], MD5_S33, MD5_T35); 
	gstd_HH (&b, c, d, a, X[14], MD5_S34, MD5_T36); 
	gstd_HH (&a, b, c, d, X[ 1], MD5_S31, MD5_T37); 
	gstd_HH (&d, a, b, c, X[ 4], MD5_S32, MD5_T38); 
	gstd_HH (&c, d, a, b, X[ 7], MD5_S33, MD5_T39); 
	gstd_HH (&b, c, d, a, X[10], MD5_S34, MD5_T40); 
	gstd_HH (&a, b, c, d, X[13], MD5_S31, MD5_T41); 
	gstd_HH (&d, a, b, c, X[ 0], MD5_S32, MD5_T42); 
	gstd_HH (&c, d, a, b, X[ 3], MD5_S33, MD5_T43); 
	gstd_HH (&b, c, d, a, X[ 6], MD5_S34, MD5_T44); 
	gstd_HH (&a, b, c, d, X[ 9], MD5_S31, MD5_T45); 
	gstd_HH (&d, a, b, c, X[12], MD5_S32, MD5_T46); 
	gstd_HH (&c, d, a, b, X[15], MD5_S33, MD5_T47); 
	gstd_HH (&b, c, d, a, X[ 2], MD5_S34, MD5_T48); 

	/*Perform Round 4 of the transformation*/
	gstd_II (&a, b, c, d, X[ 0], MD5_S41, MD5_T49); 
	gstd_II (&d, a, b, c, X[ 7], MD5_S42, MD5_T50); 
	gstd_II (&c, d, a, b, X[14], MD5_S43, MD5_T51); 
	gstd_II (&b, c, d, a, X[ 5], MD5_S44, MD5_T52); 
	gstd_II (&a, b, c, d, X[12], MD5_S41, MD5_T53); 
	gstd_II (&d, a, b, c, X[ 3], MD5_S42, MD5_T54); 
	gstd_II (&c, d, a, b, X[10], MD5_S43, MD5_T55); 
	gstd_II (&b, c, d, a, X[ 1], MD5_S44, MD5_T56); 
	gstd_II (&a, b, c, d, X[ 8], MD5_S41, MD5_T57); 
	gstd_II (&d, a, b, c, X[15], MD5_S42, MD5_T58); 
	gstd_II (&c, d, a, b, X[ 6], MD5_S43, MD5_T59); 
	gstd_II (&b, c, d, a, X[13], MD5_S44, MD5_T60); 
	gstd_II (&a, b, c, d, X[ 4], MD5_S41, MD5_T61); 
	gstd_II (&d, a, b, c, X[11], MD5_S42, MD5_T62); 
	gstd_II (&c, d, a, b, X[ 2], MD5_S43, MD5_T63); 
	gstd_II (&b, c, d, a, X[ 9], MD5_S44, MD5_T64); 

	/*add the transformed values to the current checksum*/
	m_lMD5[0] += a;
	m_lMD5[1] += b;
	m_lMD5[2] += c;
	m_lMD5[3] += d;
}


/*****************************************************************************************
CONSTRUCTOR:	CMD5Checksum
DESCRIPTION:	Initialises member data
ARGUMENTS:		None
NOTES:			None
*****************************************************************************************/
void Initialize()
{
	/* zero members*/
	memset( m_lpszBuffer, 0, 64 );
	m_nCount[0] = m_nCount[1] = 0;

	/* Load magic state initialization constants*/
	m_lMD5[0] = MD5_INIT_STATE_0;
	m_lMD5[1] = MD5_INIT_STATE_1;
	m_lMD5[2] = MD5_INIT_STATE_2;
	m_lMD5[3] = MD5_INIT_STATE_3;
}

/*****************************************************************************************
FUNCTION:		DWordToByte
DETAILS:		private
DESCRIPTION:	Transfers the data in an 32 bit array to a 8 bit array
RETURNS:		void
ARGUMENTS:		unsigned char* Output  : the 8 bit destination array 
				DWORD* Input  : the 32 bit source array
				unsigned int nLength  : the number of 8 bit data items in the source array
NOTES:			One unsigned int from the input array is transferred into four unsigned charS 
				in the output array. The first (0-7) bits of the first unsigned int are 
				transferred to the first output unsigned char, bits bits 8-15 are transferred from
				the second unsigned char etc. 
				
				The algorithm assumes that the output array is a multiple of 4 bytes long
				so that there is a perfect fit of 8 bit unsigned charS into the 32 bit unsigned ints.
*****************************************************************************************/
#if defined(ANSI)
void DWordToByte(BYTE* Output,DWORD*  Input,UINT nLength)
#else
void DWordToByte(Output, Input, nLength)
BYTE* Output;
DWORD* Input;
UINT nLength;
#endif
{
	/*transfer the data by shifting and copying*/
	UINT i = 0;
	UINT j = 0;
	for (j=0 ; j < nLength; i++, j += 4) 
	{
		Output[j] =   (UCHAR)(Input[i] & 0xff);
		Output[j+1] = (UCHAR)((Input[i] >> 8) & 0xff);
		Output[j+2] = (UCHAR)((Input[i] >> 16) & 0xff);
		Output[j+3] = (UCHAR)((Input[i] >> 24) & 0xff);
	}
}


/*****************************************************************************************
FUNCTION:		Final
DETAILS:		protected
DESCRIPTION:	Implementation of main MD5 checksum algorithm; ends the checksum calculation.
RETURNS:		CString : the final hexadecimal MD5 checksum result 
ARGUMENTS:		None
NOTES:			Performs the final MD5 checksum calculation ('Update' does most of the work,
				this function just finishes the calculation.) 
*****************************************************************************************/
#if defined(ANSI)
int Final(char* szResult)
#else
int Final(szResult)
char* szResult;
#endif
{
	/*Save number of bits*/
	int i;
	BYTE Bits[8];
	UINT nIndex;
	UINT nPadLen;
	int nMD5Size = 16;
	unsigned char lpszMD5[ 16 ];
	char szMD5[33], szStr[16];
	DWordToByte( Bits, m_nCount, 8 );

	/*Pad out to 56 mod 64.*/
	nIndex = (UINT)((m_nCount[0] >> 3) & 0x3f);
	nPadLen = (nIndex < 56) ? (56 - nIndex) : (120 - nIndex);
	Update( PADDING, nPadLen );

	/*Append length (before padding)*/
	Update( Bits, 8 );

	/*Store final state in 'lpszMD5'*/
	DWordToByte( lpszMD5, m_lMD5, nMD5Size );

	/*Convert the hexadecimal checksum to a CString*/
	strcpy(szMD5, "");
	for ( i=0; i < nMD5Size; i++) 
	{
		if (lpszMD5[i] == 0) {
			strcpy(szStr, "00");
		}
		else if (lpszMD5[i] <= 15) 	{
			sprintf(szStr, "0%x",lpszMD5[i]);
		}
		else {
			sprintf(szStr, "%x",lpszMD5[i]);
		}

		strcat(szMD5, szStr);
	}
	strcpy(szResult, szMD5);
	return MD5_OK;
}


/*****************************************************************************************
FUNCTION:		Update
DETAILS:		protected
DESCRIPTION:	Implementation of main MD5 checksum algorithm
RETURNS:		void
ARGUMENTS:		unsigned char* Input    : input block
				unsigned int nInputLen : length of input block
NOTES:			Computes the partial MD5 checksum for 'nInputLen' bytes of data in 'Input'
*****************************************************************************************/
#if defined(ANSI)
void Update(BYTE* Input,ULONG nInputLen)
#else
void Update(Input, nInputLen)
BYTE* Input;
ULONG nInputLen;
#endif
{
	/*Compute number of bytes mod 64*/
	UINT i;		
	UINT nPartLen;
	UINT nIndex = (UINT)((m_nCount[0] >> 3) & 0x3F);

	/*Update number of bits*/
	if ( ( m_nCount[0] += nInputLen << 3 )  <  ( nInputLen << 3) )
	{
		m_nCount[1]++;
	}
	m_nCount[1] += (nInputLen >> 29);

	/*Transform as many times as possible.*/
	i=0;		
	nPartLen = 64 - nIndex;
	if (nInputLen >= nPartLen) 	
	{
		memcpy( &m_lpszBuffer[nIndex], Input, nPartLen );
		Transform( m_lpszBuffer );
		for (i = nPartLen; i + 63 < nInputLen; i += 64) 
		{
			Transform( &Input[i] );
		}
		nIndex = 0;
	} 
	else 
	{
		i = 0;
	}

	/* Buffer remaining input*/
	memcpy( &m_lpszBuffer[nIndex], &Input[i], nInputLen-i);
}


