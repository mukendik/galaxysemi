/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Notes : Use this library to crypt a buffer:                                          */
/*          1) You need to create yourself a buffer in memory.                          */
/*          2) Use gbl_EncryptBuffer() to encrypt the buffer you have created.          */
/*          3) Use gbl_DecryptBuffer() to decrypt the buffer you have encrypted with    */
/*                 this module.                                                         */
/*                                                                                      */
/****************************************************************************************/
#define _GALAXY_BLOWFISH_MODULE_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined __APPLE__&__MACH__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <memory.h>

#include "gstdl_blowfish.h"
#include "gstdl_blowfishconst.h"

/* --------------------------------------------------------------------------------------- */
/* PRIVATE TYPES AND DEFINES															   */
/* --------------------------------------------------------------------------------------- */
#if defined __unix__ || __APPLE__&__MACH__
#ifndef RAND_MAX
#define RAND_MAX 0x7fff
#endif
#endif

// BG 20110623: case 4882 (cross-platform decryption)
// Remove bit-field structure in union, and replace with getByte_N()/setByte_N() accessors
// Decryption will try normal and reverse mode, as we don't know on which platform the string was crypted
union aword {
  DWORD	word;
  BYTE	byte[4];
};

#ifndef __sparc__
BYTE getByte_0(union aword aw, int bReverse){if(bReverse) return aw.byte[3]; return aw.byte[0];}
BYTE getByte_1(union aword aw, int bReverse){if(bReverse) return aw.byte[2]; return aw.byte[1];}
BYTE getByte_2(union aword aw, int bReverse){if(bReverse) return aw.byte[1]; return aw.byte[2];}
BYTE getByte_3(union aword aw, int bReverse){if(bReverse) return aw.byte[0]; return aw.byte[3];}
void setByte_0(union aword* paw, BYTE bValue, int bReverse){if(bReverse) paw->byte[3]=bValue; else paw->byte[0]=bValue;}
void setByte_1(union aword* paw, BYTE bValue, int bReverse){if(bReverse) paw->byte[2]=bValue; else paw->byte[1]=bValue;}
void setByte_2(union aword* paw, BYTE bValue, int bReverse){if(bReverse) paw->byte[1]=bValue; else paw->byte[2]=bValue;}
void setByte_3(union aword* paw, BYTE bValue, int bReverse){if(bReverse) paw->byte[0]=bValue; else paw->byte[3]=bValue;}
#else
BYTE getByte_0(union aword aw, int bReverse){if(bReverse) return aw.byte[0]; return aw.byte[3];}
BYTE getByte_1(union aword aw, int bReverse){if(bReverse) return aw.byte[1]; return aw.byte[2];}
BYTE getByte_2(union aword aw, int bReverse){if(bReverse) return aw.byte[2]; return aw.byte[1];}
BYTE getByte_3(union aword aw, int bReverse){if(bReverse) return aw.byte[3]; return aw.byte[0];}
void setByte_0(union aword* paw, BYTE bValue, int bReverse){if(bReverse) paw->byte[0]=bValue; else paw->byte[3]=bValue;}
void setByte_1(union aword* paw, BYTE bValue, int bReverse){if(bReverse) paw->byte[1]=bValue; else paw->byte[2]=bValue;}
void setByte_2(union aword* paw, BYTE bValue, int bReverse){if(bReverse) paw->byte[2]=bValue; else paw->byte[1]=bValue;}
void setByte_3(union aword* paw, BYTE bValue, int bReverse){if(bReverse) paw->byte[3]=bValue; else paw->byte[0]=bValue;}
#endif

/* --------------------------------------------------------------------------------------- */
/* PRIVATE MACROS																		   */
/* --------------------------------------------------------------------------------------- */
// BG 20110623: case 4882 (cross-platform decryption)
// Add reverse argument to macros
#define S0(x,r) (bf_S[0][getByte_0(x,r)])
#define S1(x,r) (bf_S[1][getByte_1(x,r)])
#define S2(x,r) (bf_S[2][getByte_2(x,r)])
#define S3(x,r) (bf_S[3][getByte_3(x,r)])
#define bf_F(x,r) (((S0(x,r) + S1(x,r)) ^ S2(x,r)) + S3(x,r))
#define ROUND(a,b,n,r) (a.word ^= bf_F(b,r) ^ bf_P[n])

/* --------------------------------------------------------------------------------------- */
/* PRIVATE FUNCTIONS																	   */
/* --------------------------------------------------------------------------------------- */
// BG 20110623: case 4882 (cross-platform decryption)
// Add gbl_DecryptBuffer_Internal() containing the core decryption code
// Used to be in the public gbl_DecryptBuffer() function, wich will now simply call
// gbl_DecryptBuffer_Internal() in normal mode, then in reverse mode if decryption unsuccessful.
#ifdef ANSI
static void				gbl_BlowfishEncipher(DWORD *xl, DWORD *xr, DWORD bf_P[bf_N + 2], DWORD bf_S[4][256], unsigned int bReverse);
static void				gbl_Blowfish_decipher(DWORD *xl, DWORD *xr, DWORD bf_P[bf_N + 2], DWORD bf_S[4][256], unsigned int bReverse);
static void				gbl_InitializeBlowfish(BYTE key[], int keybytes, DWORD bf_P[bf_N + 2], DWORD bf_S[4][256], unsigned int bReverse);
static int				gbl_GetRandomNumber();
static DWORD			gbl_convert_to_32(unsigned char str[4]);
static char				gbl_convert_from_32(DWORD X, int bit);
static int				gbl_DecryptBuffer_Internal(const char *szSrcBuffer, const unsigned int nSrcBufferSize, char **pszDestBuffer, unsigned int *pnDestBufferSize, const char *szKey, unsigned int bReverse);
#else
static void				gbl_BlowfishEncipher();
static void				gbl_Blowfish_decipher();
static void				gbl_InitializeBlowfish();
static int				gbl_GetRandomNumber();
static DWORD			gbl_convert_to_32();
static char				gbl_convert_from_32();
static int				gbl_DecryptBuffer_Internal();
#endif

/* --------------------------------------------------------------------------------------- */
/* PRIVATE DATA																			   */
/* --------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------- */
/* PUBLIC FUNCTIONS IMPLEMENTATION														   */
/* --------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------- */
/* Description: Free the specified buffer.												   */
/*																						   */
/* Parameters:                                                                             */
/*             pszDestBuffer: ptr to the buffer to free.								   */
/*																						   */
/* Return value:                                                                           */
/*             None.																	   */
/* --------------------------------------------------------------------------------------- */
#ifdef ANSI
void gbl_FreeBuffer(char **pszBuffer)
#else
void gbl_FreeBuffer(pszBuffer)
char	**pszBuffer;
#endif
{
	free(*pszBuffer);
	*pszBuffer = NULL;
}

/* --------------------------------------------------------------------------------------- */
/* Description: Encrypt the specified source buffer to the specified destination buffer.   */
/*																						   */
/* Parameters:                                                                             */
/*             szSrcBuffer: buffer to crypt.									           */
/*             nSrcBufferSize: The size of the buffer to crypt.                            */
/*             pszDestBuffer: ptr to the destination buffer.                               */
/*             pnDestBufferSize: ptr to an unsigned int to receive the size of the         */
/*                               destination buffer.                                       */
/*             szKey: key used to crypt the buffer (must also be use to decrypt it!)       */
/*             nKeySize: The lenght of the specified key                                   */
/*																						   */
/* Return value:                                                                           */
/*             1 if successful; otherwise return 0.                                        */
/*																						   */
/* Notes: The destination buffer is allocated in this function with a malloc() function.   */
/*        The user of this function must free the destination buffer, using				   */
/*		  gbl_FreeBuffer()																   */
/* --------------------------------------------------------------------------------------- */
#ifdef ANSI
int gbl_EncryptBuffer(const char *szSrcBuffer, const unsigned int nSrcBufferSize,
					  char **pszDestBuffer, unsigned int *pnDestBufferSize,
					  const char *szKey)
#else
int gbl_EncryptBuffer(szSrcBuffer, nSrcBufferSize, pszDestBuffer, pnDestBufferSize, szKey)
const char			*szSrcBuffer;
const unsigned int	nSrcBufferSize;
char				**pszDestBuffer;
unsigned int		*pnDestBufferSize;
const char			*szKey;
#endif
{
	DWORD			bf_P[bf_N + 2];
	DWORD			bf_S[4][256];
	DWORD			Xl,Xr;
	unsigned char	c[5];
	unsigned char	overflow[9];
  int i, bytesleft;
  //FIXME: not used ?
  //int nBytesToWrite;
	int				reading = 1;
	long int		nCurPos_Srce=0, nCurPos_Dest=0;

	unsigned int	nKeySize = strlen(szKey);
	int				nNewSize, nOverflow, nBytesAdded;
	char			*szTmpSrcBuffer;
	
	/* Check input parameters */
	if((szSrcBuffer == NULL) || (nSrcBufferSize == 0))
		return 0;
	if((szKey == NULL) || (nKeySize < 1))
		return 0;

	/* Key size must be between 1 and GBL_MAXKEYBYTES */
	if(nKeySize > GBL_MAXKEYBYTES)
		nKeySize = GBL_MAXKEYBYTES;

	/* Create temp buffer: buffer size must be aligned to multiple of 8 bytes */
	nOverflow = (nSrcBufferSize+1) - ((nSrcBufferSize >> 3) << 3);
	nBytesAdded = 8-nOverflow;
	nNewSize = (nSrcBufferSize+1) + nBytesAdded;
	szTmpSrcBuffer = (char*)malloc(nNewSize*sizeof(char));
	if(szTmpSrcBuffer == NULL)
		return 0;

	/* Fill aligned temp buffer:  */
	/* Byte 0 of buffer will contain nb of bytes added for alignment */
	/* Then we insert the original source buffer content */
	/* Then we fill with random values if required */
	szTmpSrcBuffer[0] = (char)nBytesAdded;
	memcpy(szTmpSrcBuffer+1, szSrcBuffer, nSrcBufferSize);
	for(i=1 ; i<=nBytesAdded ; i++)
		szTmpSrcBuffer[nSrcBufferSize+i] = (char)gbl_GetRandomNumber();

	/* Alloc destination buffer */
	*pszDestBuffer = (char*)malloc((nNewSize)*sizeof(char));
	if(*pszDestBuffer == NULL)
	{
		free(szTmpSrcBuffer);
		return 0;
	}
	*pnDestBufferSize = nNewSize;  

	/* Initialize blowfish data */ 
	gbl_InitializeBlowfish((unsigned char*)szKey, nKeySize, bf_P, bf_S, 0);
	
	/* Crypt Buffer */
	while(reading)
	{
		bytesleft = nNewSize-nCurPos_Srce;
		if ((bytesleft) < 8)
		{
			for (i=0; i<bytesleft;i++)
			{
				overflow[i] = szTmpSrcBuffer[nCurPos_Srce++];
			}
			overflow[bytesleft + 1] = '\0';
			reading = 0;
                  //FIXME: not used ?
                  //nBytesToWrite = bytesleft;
		}
		else
		{
			c[0] = szTmpSrcBuffer[nCurPos_Srce++];
			c[1] = szTmpSrcBuffer[nCurPos_Srce++];
			c[2] = szTmpSrcBuffer[nCurPos_Srce++];
			c[3] = szTmpSrcBuffer[nCurPos_Srce++];
			c[4] = '\0';

			Xl = gbl_convert_to_32(c);

			c[0] = szTmpSrcBuffer[nCurPos_Srce++];
			c[1] = szTmpSrcBuffer[nCurPos_Srce++];
			c[2] = szTmpSrcBuffer[nCurPos_Srce++];
			c[3] = szTmpSrcBuffer[nCurPos_Srce++];
			c[4] = '\0';

			Xr = gbl_convert_to_32(c);
			reading = 1;
                  //FIXME: not used ?
                  //nBytesToWrite = 8;
		}

		if(reading == 0)
		{
			for (i=0; i<bytesleft;i++)
			{
				(*pszDestBuffer)[nCurPos_Dest++] = overflow[i];
			}

		}
		else 
		{
			gbl_BlowfishEncipher(&Xl, &Xr, bf_P, bf_S, 0);

			for(i=0;i<4;i++) overflow[i] = gbl_convert_from_32(Xl,i);

			(*pszDestBuffer)[nCurPos_Dest++] = overflow[0];
			(*pszDestBuffer)[nCurPos_Dest++] = overflow[1];
			(*pszDestBuffer)[nCurPos_Dest++] = overflow[2];
			(*pszDestBuffer)[nCurPos_Dest++] = overflow[3];


			for(i=0;i<4;i++) overflow[i] = gbl_convert_from_32(Xr,i);
			
			(*pszDestBuffer)[nCurPos_Dest++] = overflow[0];
			(*pszDestBuffer)[nCurPos_Dest++] = overflow[1];
			(*pszDestBuffer)[nCurPos_Dest++] = overflow[2];
			(*pszDestBuffer)[nCurPos_Dest++] = overflow[3];
		}
	}

  free(szTmpSrcBuffer);
  return 1;
}

  /* --------------------------------------------------------------------------------------- */
  /* Description: Decrypt the specified source to the specified destination buffer.		   */
  /*																						   */
  /* Parameters:                                                                             */
  /*             szSrcBuffer: buffer to decrypt.									           */
  /*             nSrcBufferSize: The size of the buffer to decrypt.                          */
  /*             pszDestBuffer: ptr to the destination buffer.                               */
  /*             pnDestBufferSize: ptr to an unsigned int to receive the size of the         */
  /*                               destination buffer.                                       */
  /*             szKey: key used to decrypt the buffer (must be the same than to encrypt it!)*/
  /*             nKeySize: The lenght of the specified key                                   */
  /*																						   */
  /* Return value:                                                                           */
  /*             1 if successful; otherwise return 0.                                        */
  /*																						   */
  /* Notes: The destination buffer is allocated in this function with a malloc() function.   */
  /*        The user of this function must free the destination buffer, using				   */
  /*		  gbl_FreeBuffer()																   */
  /* --------------------------------------------------------------------------------------- */
  #ifdef ANSI
  int gbl_DecryptBuffer(const char *szSrcBuffer, const unsigned int nSrcBufferSize,
						char **pszDestBuffer, unsigned int *pnDestBufferSize,
						const char *szKey)
  #else
  int gbl_DecryptBuffer(szSrcBuffer, nSrcBufferSize, pszDestBuffer, pnDestBufferSize, szKey)
  const char			*szSrcBuffer;
  const unsigned int	nSrcBufferSize;
  char				**pszDestBuffer;
  unsigned int		*pnDestBufferSize;
  const char			*szKey;
  #endif
  {
	  if(gbl_DecryptBuffer_Internal(szSrcBuffer, nSrcBufferSize, pszDestBuffer, pnDestBufferSize, szKey, 0)) return 1;
	  return gbl_DecryptBuffer_Internal(szSrcBuffer, nSrcBufferSize, pszDestBuffer, pnDestBufferSize, szKey, 1);
  }

  /* --------------------------------------------------------------------------------------- */
  /* Description: Decrypt the specified source to the specified destination buffer.		   */
  /*																						   */
  /* Parameters:                                                                             */
  /*             szSrcBuffer: buffer to decrypt.									           */
  /*             nSrcBufferSize: The size of the buffer to decrypt.                          */
  /*             pszDestBuffer: ptr to the destination buffer.                               */
  /*             pnDestBufferSize: ptr to an unsigned int to receive the size of the         */
  /*                               destination buffer.                                       */
  /*             szKey: key used to decrypt the buffer (must be the same than to encrypt it!)*/
  /*             nKeySize: The lenght of the specified key                                   */
  /*             bReverse: if > 0, reverse byte ordering when decrypting                     */
  /*																						   */
  /* Return value:                                                                           */
  /*             1 if successful; otherwise return 0.                                        */
  /*																						   */
  /* Notes: The destination buffer is allocated in this function with a malloc() function.   */
  /*        The user of this function must free the destination buffer, using				   */
  /*		  gbl_FreeBuffer()																   */
  /* --------------------------------------------------------------------------------------- */
  #ifdef ANSI
  int gbl_DecryptBuffer_Internal(const char *szSrcBuffer, const unsigned int nSrcBufferSize,
						char **pszDestBuffer, unsigned int *pnDestBufferSize,
						const char *szKey, unsigned int bReverse)
  #else
  int gbl_DecryptBuffer_Internal(szSrcBuffer, nSrcBufferSize, pszDestBuffer, pnDestBufferSize, szKey, bReverse)
  const char			*szSrcBuffer;
  const unsigned int	nSrcBufferSize;
  char				**pszDestBuffer;
  unsigned int		*pnDestBufferSize;
  const char			*szKey;
  unsigned int		bReverse;
  #endif
  {
	DWORD			bf_P[bf_N + 2];
	DWORD			bf_S[4][256];
	DWORD			Xl,Xr;
	unsigned char	c[5];
	unsigned char	overflow[9];
    int i,bytesleft;
    //FIXME: not used ?
    //int nBytesToWrite;
	int				reading = 1;
	long int		nCurPos_Srce=0, nCurPos_Dest=0;

	unsigned int	nKeySize = strlen(szKey);
	int				nNewSize, nBytesAdded;
	char			*szTmpDestBuffer;
	
	/* Check input parameters */
	if((szSrcBuffer == NULL) || (nSrcBufferSize == 0))
		return 0;
	if((szKey == NULL) || (nKeySize < 1))
		return 0;

	/* Key size must be between 1 and GBL_MAXKEYBYTES */
	if(nKeySize > GBL_MAXKEYBYTES)
		nKeySize = GBL_MAXKEYBYTES;

	/* Allocate memory for temp destination buffer */
	szTmpDestBuffer = (char*)malloc(nSrcBufferSize*sizeof(char));
	if(szTmpDestBuffer == NULL)
		return 0;

	/* Initialize blowfish data */ 
	gbl_InitializeBlowfish((unsigned char*)szKey, nKeySize, bf_P, bf_S, bReverse);
	
	/* Decrypt Buffer */
	while(reading)
	{
		bytesleft = nSrcBufferSize-nCurPos_Srce;
		if ((bytesleft) < 8)
		{
			for (i=0; i<bytesleft;i++)
			{
				overflow[i] = (szSrcBuffer)[nCurPos_Srce++];
			}
			overflow[bytesleft + 1] = '\0';
			reading = 0;
                  //FIXME: not used ?
                  //nBytesToWrite = bytesleft;
		}
		else
		{
			c[0] = (szSrcBuffer)[nCurPos_Srce++];
			c[1] = (szSrcBuffer)[nCurPos_Srce++];
			c[2] = (szSrcBuffer)[nCurPos_Srce++];
			c[3] = (szSrcBuffer)[nCurPos_Srce++];
			c[4] = '\0';
			Xl = gbl_convert_to_32(c);

			c[0] = (szSrcBuffer)[nCurPos_Srce++];
			c[1] = (szSrcBuffer)[nCurPos_Srce++];
			c[2] = (szSrcBuffer)[nCurPos_Srce++];
			c[3] = (szSrcBuffer)[nCurPos_Srce++];
			c[4] = '\0';
			Xr = gbl_convert_to_32(c);
			reading = 1;
                  //FIXME: not used ?
                  //nBytesToWrite = 8;
		}

		if(reading == 0)
		{
			for (i=0; i<bytesleft;i++)
			{
				szTmpDestBuffer[nCurPos_Dest++] = overflow[i];
			}
		}
		else 
		{
			gbl_Blowfish_decipher(&Xl, &Xr, bf_P, bf_S, bReverse);

			for(i=0;i<4;i++) overflow[i] = gbl_convert_from_32(Xl,i);
			szTmpDestBuffer[nCurPos_Dest++] = overflow[0];
			szTmpDestBuffer[nCurPos_Dest++] = overflow[1];
			szTmpDestBuffer[nCurPos_Dest++] = overflow[2];
			szTmpDestBuffer[nCurPos_Dest++] = overflow[3];
			

			for(i=0;i<4;i++) overflow[i] = gbl_convert_from_32(Xr,i);
			szTmpDestBuffer[nCurPos_Dest++] = overflow[0];
			szTmpDestBuffer[nCurPos_Dest++] = overflow[1];
			szTmpDestBuffer[nCurPos_Dest++] = overflow[2];
			szTmpDestBuffer[nCurPos_Dest++] = overflow[3];
		}
	}
	
	/* Restore original buffer (removing data added for alignment) */
	nBytesAdded = (int)szTmpDestBuffer[0];
	nNewSize = nSrcBufferSize-(nBytesAdded+1);
	
	/* Check consistency of nNewSize: if encrypted data has been modified or altered, nNewSize may be negative */
	/* It may also be positive, but then the resulting data will be invalid */
	if(nNewSize < 0)
	{
		free(szTmpDestBuffer);
		return 0;
	}
	*pszDestBuffer = (char*)malloc(nNewSize*sizeof(char));
	if(*pszDestBuffer == NULL)
	{
		free(szTmpDestBuffer);
		return 0;
	}
	memcpy(*pszDestBuffer, szTmpDestBuffer+1, nNewSize);
	*pnDestBufferSize = nNewSize;

	free(szTmpDestBuffer);
	return 1;
}

/* --------------------------------------------------------------------------------------- */
/* PRIVATE FUNCTIONS IMPLEMENTATION														   */
/* --------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------- */
#ifdef ANSI
static void gbl_BlowfishEncipher(DWORD *xl, DWORD *xr, DWORD bf_P[bf_N + 2], DWORD bf_S[4][256], unsigned int bReverse)
#else
static void gbl_BlowfishEncipher(xl, xr, bf_P, bf_S, bReverse)
DWORD	*xl;
DWORD	*xr;
DWORD	bf_P[bf_N + 2];
DWORD	bf_S[4][256];
unsigned int bReverse;
#endif
{
	union aword  Xl;
	union aword  Xr;

	Xl.word = *xl;
	Xr.word = *xr;

	Xl.word ^= bf_P[0];
	ROUND (Xr, Xl, 1, bReverse);  ROUND (Xl, Xr, 2, bReverse);
	ROUND (Xr, Xl, 3, bReverse);  ROUND (Xl, Xr, 4, bReverse);
	ROUND (Xr, Xl, 5, bReverse);  ROUND (Xl, Xr, 6, bReverse);
	ROUND (Xr, Xl, 7, bReverse);  ROUND (Xl, Xr, 8, bReverse);
	ROUND (Xr, Xl, 9, bReverse);  ROUND (Xl, Xr, 10, bReverse);
	ROUND (Xr, Xl, 11, bReverse); ROUND (Xl, Xr, 12, bReverse);
	ROUND (Xr, Xl, 13, bReverse); ROUND (Xl, Xr, 14, bReverse);
	ROUND (Xr, Xl, 15, bReverse); ROUND (Xl, Xr, 16, bReverse);
	Xr.word ^= bf_P[17];

	*xr = Xl.word;
	*xl = Xr.word;
}

/* --------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------- */
#ifdef ANSI
static void gbl_Blowfish_decipher(DWORD *xl, DWORD *xr, DWORD bf_P[bf_N + 2], DWORD bf_S[4][256], unsigned int bReverse)
#else
static void gbl_Blowfish_decipher(xl, xr, bf_P, bf_S, bReverse)
DWORD	*xl;
DWORD	*xr;
DWORD	bf_P[bf_N + 2];
DWORD	bf_S[4][256];
unsigned int bReverse;
#endif
{
   union aword  Xl;
   union aword  Xr;

   Xl.word = *xl;
   Xr.word = *xr;

   Xl.word ^= bf_P[17];
   ROUND (Xr, Xl, 16, bReverse);  ROUND (Xl, Xr, 15, bReverse);
   ROUND (Xr, Xl, 14, bReverse);  ROUND (Xl, Xr, 13, bReverse);
   ROUND (Xr, Xl, 12, bReverse);  ROUND (Xl, Xr, 11, bReverse);
   ROUND (Xr, Xl, 10, bReverse);  ROUND (Xl, Xr, 9, bReverse);
   ROUND (Xr, Xl, 8, bReverse);   ROUND (Xl, Xr, 7, bReverse);
   ROUND (Xr, Xl, 6, bReverse);   ROUND (Xl, Xr, 5, bReverse);
   ROUND (Xr, Xl, 4, bReverse);   ROUND (Xl, Xr, 3, bReverse);
   ROUND (Xr, Xl, 2, bReverse);   ROUND (Xl, Xr, 1, bReverse);
   Xr.word ^= bf_P[0];

   *xl = Xr.word;
   *xr = Xl.word;
}

/* --------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------- */
#ifdef ANSI
static void gbl_InitializeBlowfish(BYTE key[], int keybytes, DWORD bf_P[bf_N + 2], DWORD bf_S[4][256], unsigned int bReverse)
#else
static void gbl_InitializeBlowfish(key, keybytes, bf_P, bf_S, bReverse)
BYTE	key[];
int		keybytes;
DWORD	bf_P[bf_N + 2];
DWORD	bf_S[4][256];
unsigned int bReverse;
#endif
{
	short			i;             /* FIXME: unsigned int, char? */
	short			j;             /* FIXME: unsigned int, char? */
	DWORD			data;
	DWORD			datal;
	DWORD			datar;
	union aword		temp;
	time_t			tCurTime;

	time(&tCurTime);
	srand((unsigned int)tCurTime);

	memcpy(bf_P,bf_P_Temp,(bf_N + 2)*sizeof(DWORD));
	memcpy(bf_S,bf_S_Temp,(4*256)*sizeof(DWORD));

	j = 0;
	for (i = 0; i < bf_N + 2; ++i) 
	{
		temp.word = 0;
		setByte_0(&temp, key[j], bReverse);
		setByte_1(&temp, key[(j+1)%keybytes], bReverse);
		setByte_2(&temp, key[(j+2)%keybytes], bReverse);
		setByte_3(&temp, key[(j+3)%keybytes], bReverse);
		data = temp.word;
		bf_P[i] = bf_P[i] ^ data;
		j = (j + 4) % keybytes;
	}

	datal = 0x00000000;
	datar = 0x00000000;

	for (i = 0; i < bf_N + 2; i += 2) 
	{
		gbl_BlowfishEncipher(&datal, &datar, bf_P, bf_S, bReverse);
		bf_P[i] = datal;
		bf_P[i + 1] = datar;
	}

	for (i = 0; i < 4; ++i) 
	{
		for (j = 0; j < 256; j += 2) 
		{
			gbl_BlowfishEncipher(&datal, &datar, bf_P, bf_S, bReverse);
   			bf_S[i][j] = datal;
			bf_S[i][j + 1] = datar;
		}
	}
}

static int gbl_GetRandomNumber()
{
	return ((rand()*255)/RAND_MAX);
}

#ifdef ANSI
static DWORD gbl_convert_to_32(unsigned char str[4])
#else
static DWORD gbl_convert_to_32(str)
unsigned char str[4];
#endif
{
	DWORD R = 0x00000000;
	
	R = str[3];
	R = R << 8;
	R += str[2];
	R = R << 8;
	R += str[1];
	R = R << 8;
	R += str[0];
	return R;
}

#ifdef ANSI
static char gbl_convert_from_32(DWORD X, int bit)
#else
static char gbl_convert_from_32(X, bit)
DWORD X;
int bit;
#endif
{
	char c;
	
	switch(bit)
	{
	case 0:
		c = (char) (X & 255);
		break;
	case 1:
		c = (char) ((X >> 8) & 255);
		break;
	case 2:
		c = (char) ((X >> 16) & 255);
		break;
	case 3 :
		c = (char) ((X >> 24) & 255);
		break;
  default:
    c = 0;
    break;
	}

	 return c;
}

