/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Notes :                                                                              */
/*                                                                                      */
/****************************************************************************************/
#ifndef _GALAXY_BLOWFISH_HEADER_
#define _GALAXY_BLOWFISH_HEADER_

/* $Id: blowfish.h,v 1.3 1995/01/23 12:38:02 pr Exp pr $*/
#define GBL_MAXKEYBYTES	56          /* 448 bits */


/*///////////////////////////////////////////////////////////////////////////////////*/
/* defines */

/* Define ANSI under SOLARIS */
#if (defined(__STDC__) && !defined(ANSI)) || defined(_WIN32)
#define ANSI
#endif

#if (!defined(__STDC__) && !defined(ANSI) && !defined(_WIN32))
#define const
#endif

/* Need to export functions? */
#if defined(_GALAXY_LOGFILE_MODULE_)
#define _GBLOWFISH_EXPORT
#else
#define _GBLOWFISH_EXPORT	extern
#endif


#if !defined(__cplusplus) && !defined(ANSI)

	_GBLOWFISH_EXPORT	int		gbl_EncryptBuffer();
	_GBLOWFISH_EXPORT	int		gbl_DecryptBuffer();
	_GBLOWFISH_EXPORT	void	gbl_FreeBuffer();

#else /* !defined(__cplusplus) && !defined(ANSI) */

#if defined(__cplusplus) /* If __cplusplus defined, it means this module is exported! */
extern "C"
{
#endif /* defined(__cplusplus) */
	
	int		gbl_EncryptBuffer(const char *szSrcBuffer, const unsigned int nSrcBufferSize, 
							  char **pszDestBuffer, unsigned int *pnDestBufferSize,
							  const char *szKey);
	int		gbl_DecryptBuffer(const char *szSrcBuffer, const unsigned int nSrcBufferSize,
							  char **pszDestBuffer, unsigned int *pnDestBufferSize,
							  const char *szKey);
	void	gbl_FreeBuffer(char **pszBuffer);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* !defined(__cplusplus) && !defined(ANSI) */

#endif /* _GALAXY_BLOWFISH_HEADER_ */
