///////////////////////////////////////////////////////////////////////////////////////////
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
//////////////////////////////////////////////////////////////////////////////////////////
// gcrypto.cpp: implementation of the CGCrypto class.
//
//
// Notes :
//
//////////////////////////////////////////////////////////////////////////////////////////

#define _GALAXY_CRYPTO_EXPORTS_

#include "gstdl_utilsdll.h"
#include "gstdl_crypto.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#if defined(__unix__)
    #include <unistd.h>
#endif

#include <gstdl_utils.h>

using namespace std;

/* --------------------------------------------------------------------------------------- */
/* PRIVATE TYPES AND DEFINES															   */
/* --------------------------------------------------------------------------------------- */
// Error map
GBEGIN_ERROR_MAP(CGCrypto)
	GMAP_ERROR(eCrypto,"Error during encryption/decryption operation")
GEND_ERROR_MAP(CGCrypto)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGCrypto::CGCrypto(PCSTR szKey)
{
	// Initialize key
	strncpy(m_szKey, szKey, GBL_MAXKEYBYTES);
	// Just to make it a null-terminated string if original key was greater 
	// than GBL_MAXKEYBYTES
	m_szKey[GBL_MAXKEYBYTES] = '\0';
}

CGCrypto::~CGCrypto()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Encrypts the source buffer into a destination buffer
//
// Argument(s) :
//      const CGMemBuffer &cmbSrcBuffer : source buffer to encrypt
//      CGMemBuffer *pmbDestBuffer : ptr to destination buffer
//
// Return      : TRUE if successful, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGCrypto::Encrypt(const CGMemBuffer &cmbSrcBuffer, CGMemBuffer &mbDestBuffer)
{
	BOOL		bReturn = TRUE;
	UINT		nSrcBufferSize, nDestBufferSize;
	const char	*pSrcBuffer =  NULL;
	char		*pDestBuffer = NULL;

	// Retrieve source buffer and buffer size
	nSrcBufferSize = cmbSrcBuffer.GetBufferSize();
	pSrcBuffer = (char *)cmbSrcBuffer.GetData();

	// Encrypt source buffer to destination buffer
	if(gbl_EncryptBuffer(pSrcBuffer, nSrcBufferSize, &pDestBuffer, &nDestBufferSize, m_szKey) == 0)
	{
		GSET_ERROR(CGCrypto,eCrypto);
		return FALSE;
	}

	// Create final buffer
	mbDestBuffer.Close();
	mbDestBuffer.CopyIn((unsigned char *)pDestBuffer, nDestBufferSize, nDestBufferSize);

	if(pDestBuffer != NULL)	gbl_FreeBuffer(&pDestBuffer);
	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Decrypts the source buffer into a destination buffer
//
// Argument(s) :
//      const CGMemBuffer &cmbSrcBuffer : source buffer to encrypt
//      CGMemBuffer *pmbDestBuffer : ptr to destination buffer
//
// Return      : TRUE if successful, FALSE else
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGCrypto::Decrypt(const CGMemBuffer &cmbSrcBuffer, CGMemBuffer &mbDestBuffer)
{
	BOOL		bReturn = TRUE;
	UINT		nSrcBufferSize, nDestBufferSize;
	char		*pSrcBuffer =  NULL;
	char		*pDestBuffer = NULL;

	// Retrieve source buffer and buffer size
	nSrcBufferSize = cmbSrcBuffer.GetBufferSize();
	pSrcBuffer = (char *)cmbSrcBuffer.GetData();

	// Decrypt source buffer to destination buffer
	if(gbl_DecryptBuffer(pSrcBuffer, nSrcBufferSize, &pDestBuffer, &nDestBufferSize, m_szKey) == 0)
	{
		GSET_ERROR(CGCrypto,eCrypto);
		return FALSE;
	}

	// Set buffer in MemBuffer
	mbDestBuffer.Close();
	mbDestBuffer.CopyIn((unsigned char *)pDestBuffer, nDestBufferSize, nDestBufferSize);

	if(pDestBuffer != NULL)	gbl_FreeBuffer(&pDestBuffer);
	return bReturn;
}

