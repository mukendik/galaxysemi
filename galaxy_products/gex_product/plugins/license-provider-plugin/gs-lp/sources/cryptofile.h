///////////////////////////////////////////////////////////////////////////////////////////
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
//////////////////////////////////////////////////////////////////////////////////////////
//
// Notes :
//
//////////////////////////////////////////////////////////////////////////////////////////
// CryptoFile.h: interface for the CCryptoFile class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CRYPTO_H
	
#define _CRYPTO_H

/* $Id: blowfish.h,v 1.3 1995/01/23 12:38:02 pr Exp pr $*/
#define MAXKEYBYTES				56          /* 448 bits */
#define bf_N					16
#define noErr					0
#define DATAERROR				-1
#define KEYBYTES				8
#define subkeyfilename			"Blowfish.dat"
#define CRYPTED_EXTENSION		"bin"
#define DECRYPTED_EXTENSION		"ldf"
#define	DEFAULT_NAME_CRYPTED	"cryptedfile"
#define	DEFAULT_NAME_DECRYPTED	"decryptedfile"

#define	W_TEXT_MODE				"wt"
#define	W_BINARY_MODE			"wb"
#define	R_TEXT_MODE				"rt"
#define	R_BINARY_MODE			"rb"


#define UWORD_32bits  unsigned long
#define UWORD_16bits  unsigned short
#define UBYTE_08bits  unsigned char

#if defined __unix__ || __MACH__
#ifndef RAND_MAX
#define RAND_MAX 0x7fff
#endif
#endif

//union aword {
// UWORD_32bits word;
//  UBYTE_08bits byte [4];
//  struct {
//    unsigned int byte3:8;
//    unsigned int byte2:8;
//    unsigned int byte1:8;
//    unsigned int byte0:8;
//  } w;
//};

union aword {
  UWORD_32bits word;
  UBYTE_08bits byte [4];
#if __sparc__
  // Unix-sparc platform, keep definition as before.
  struct {
    unsigned int byte3:8;
    unsigned int byte2:8;
    unsigned int byte1:8;
    unsigned int byte0:8;
  } w;
#else
  // PC-x86 Platform (LITTLE_ENDIAN): swap definition order
  // to behave as UNIX CPU.
  // Unix platform is: BIG_ENDIAN
  struct {
    unsigned int byte0:8;
    unsigned int byte1:8;
    unsigned int byte2:8;
    unsigned int byte3:8;
  } w;
#endif
};
 

//////////////////////////////////////////////////////////////////////////////////////:
// CryptoFile defines

#define	CPF_OKAY				0x0	// Okay, no error
#define CPF_COF					0x1	// Cannot open file
#define	CPF_FNF					0x2	// File not found

class CCryptoFile  
{
public:
	CCryptoFile();
	virtual ~CCryptoFile();

// Operations
	int			Encrypt_Buffer(unsigned char key[], short keybytes);
	int			Decrypt_Buffer(unsigned char key[], short keybytes);
	void		ModifyOutputFileName(char *,char *);
	void		ModifyInputFileName(char *,char *);
	int			SetBufferFromFile(const char *, const char *);
	int			SetFileFromBuffer(const char * szFileName, const char *szMode);
	int			SetBufferFromBuffer(const char *,int);
	void		SetBufferFromHexa(const char *);
	void		GetBuffer(char **);
	void		GetBufferToHexa(char **);
	int			GetBufferSize();
	void		InitConstantes();
	int			GetRandomNumber();

private:
	UWORD_32bits bf_P[bf_N + 2];
	UWORD_32bits bf_S[4][256];
	
	char	*m_szBuffer;
	int		m_nBufferSize;

protected:

// Operation
	short						opensubkeyfile(void);
	UWORD_32bits				F(UWORD_32bits x);
	void Blowfish_encipher		(UWORD_32bits *xl, UWORD_32bits *xr);
	void Blowfish_decipher		(UWORD_32bits *xl, UWORD_32bits *xr);
	short InitializeBlowfish	(unsigned char key[], short keybytes);
	UWORD_32bits convert_to_32	(unsigned char str[4]); 
	char convert_from_32		(UWORD_32bits,int bit); 
	
};

#endif // #ifndef _CRYPTO_H

