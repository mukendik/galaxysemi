// gcrypto.h: interface for the CGCrypto class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GALAXY_CRYPTO_HEADER_
#define _GALAXY_CRYPTO_HEADER_

#include "gstdl_membuffer.h"

#include <gstdl_errormgr.h>
#include <gstdl_blowfish.h>
/*
#if defined(_WIN32)
#	if defined(_GALAXY_CRYPTO_EXPORTS_)
#		define GCRYPTO_API __declspec(dllexport)
#	elif defined _GALAXY_STDUTILS_DLL_MODULE
#		define GCRYPTO_API
#	else	// _GALAXY_CRYPTO_EXPORTS_
#		define GCRYPTO_API __declspec(dllimport)
#	endif // _GALAXY_CRYPTO_EXPORTS_
#else
#	define GCRYPTO_API
#endif // _MSC_VER
*/

#define GCRYPTO_API

//////////////////////////////////////////////////////////////////////
// CGCrypto class

class GCRYPTO_API CGCrypto  
{
// Constructor / Destructor
public:
	CGCrypto(PCSTR szKey);
	virtual ~CGCrypto();

// Attributes
public:
	GDECLARE_ERROR_MAP(CGCrypto)
	{
		eCrypto						// Error during encryption/decryption
	}
	GDECLARE_END_ERROR_MAP(CGCrypto)

protected:	
	char		m_szKey[GBL_MAXKEYBYTES+1];

// Operations
public:
	// Encrypt
	BOOL		Encrypt(const CGMemBuffer &cmbSrcBuffer, CGMemBuffer &mbDestBuffer);
	// Decrypt
	BOOL		Decrypt(const CGMemBuffer &cmbSrcBuffer, CGMemBuffer &mbDestBuffer);

// Implementation
protected:	
};

#endif // #ifndef _GALAXY_CRYPTO_HEADER_

