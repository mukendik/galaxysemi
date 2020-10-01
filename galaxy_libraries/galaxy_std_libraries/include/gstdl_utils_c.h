/*======================================================================*/
/* Galaxy                                                               */
/* Copyright 1996-98, Softlink                                          */
/* This computer program is protected by copyrigt law                   */
/* and international treaties. Unauthorized reproduction or             */
/* distribution of this program, or any portion of it,may               */
/* result in severe civil and criminal penalties, and will be           */
/* prosecuted to the maximum extent possible under the low.             */
/*======================================================================*/

#include <time.h>

/*======================================================================*/
/*========================== UTILS.H HEADER ============================*/
/*======================================================================*/
#ifndef _Utils_h_

#define _Utils_h_

#include <stdio.h>
#if defined __unix__ || __APPLE__&__MACH__
#include <dirent.h>   /* for directory functions : opendir(), readdir(), closedir() */
#include <string.h>
#else
#include <io.h>       /* for directory functions : _findfirst(), _findnext() */
#endif

/*=============================================================================*/
/* Error Codes																   */
/*=============================================================================*/
#define UT_ERR_OKAY				0   /* No error                                */
#define UT_ERR_NOSECTION		1   /* Couldn't find section in .ini style file*/
#define UT_ERR_NOENTRY			2   /* Couldn't find entry in .ini style file  */
#define UT_ERR_READ				3   /* Couldn't read file                      */
#define UT_ERR_WRITE			4	/* Couldn't write file					   */
#define UT_ERR_OVERFLOW			5	/* Buffer overflow						   */
#define UT_ERR_LINE_TRUNCATED	6	/* Line too long > UT_MAX_BUFFERLENGTH	   */

/*======================================================================*/
/* Definitions.                                                         */
/*======================================================================*/
#define UT_MAX_BUFFERLENGTH   1024
#define UT_MAX_PATH           512
#define UT_MAX_SECTIONLENGTH  128
#define UT_MAX_TIMESTAMP_LEN  100
#define UT_MAX_EXT            32
#define UT_MAX_FNAME          256
/* Defines for ut_FindFiles() */
#define UT_FINDFIRST            0
#define UT_FINDNEXT             1
#define UT_CLOSEFIND            2
#define UT_ALLENTRIES			0
#define UT_DIRONLY				1
#define UT_FILEONLY				2
/* Defines for ut_CopyFile() */
#define UT_COPY_OVERWRITE       1
#define UT_COPY_USETEMPNAME     2
#define UT_COPY_UPDATEMTIME     4
#define UT_COPY_USELOCK			8

/*======================================================================*/
/* Definitions.                                                         */
/*======================================================================*/
/* types */
struct tagUT_FindFileHandle
{
#if defined __unix__ || __APPLE__&__MACH__
	DIR		*pDir;
#else
	long	lHandle;
#endif
};
typedef struct tagUT_FindFileHandle UT_FindFileHandle;
typedef struct tagUT_FindFileHandle *PT_UT_FindFileHandle;

/*======================================================================*/
/* Macros to support both PC and UNIX platforms                         */
/*======================================================================*/
#if defined __unix__ || __APPLE__&__MACH__
/* UNIX macros */
#define	UT_strcmp(a,b)		strcmp(a,b)
#define	UT_stricmp(a,b) 	strcasecmp(a,b)
/* LIVERETEST */
#define	UT_strncmp(a,b,n)	strncmp(a,b,n)
#define	UT_strnicmp(a,b,n) 	strncasecmp(a,b,n)
#else
/* PC macros   */
#define	UT_strcmp(a,b)		strcmp(a,b)
#define	UT_stricmp(a,b)	  stricmp(a,b)
/* LIVERETEST */
#define	UT_strncmp(a,b,n)	strncmp(a,b,n)
#define	UT_strnicmp(a,b,n)	strnicmp(a,b,n)
#endif

/* Define ANSI under SOLARIS */
#if (defined(__STDC__) && !defined(ANSI)) || defined(_WIN32)
#define ANSI
#endif

/* Need to export functions? */
#if defined(_UtilsModule_)
#define _GUTILSC_EXPORT
#else
#define _GUTILSC_EXPORT	extern
#endif

/*======================================================================*/
/* Exported Functions.                                                  */
/*======================================================================*/
#if !defined(__cplusplus) && !defined(ANSI)

_GUTILSC_EXPORT	time_t  ut_GetCompactTimeStamp();
_GUTILSC_EXPORT	time_t  ut_GetFullTextTimeStamp();
_GUTILSC_EXPORT	void    ut_GetPID();
_GUTILSC_EXPORT	int     ut_SplitPathName();
_GUTILSC_EXPORT	int     ut_NormalizePath();
_GUTILSC_EXPORT	int		ut_RemoveDrive();
_GUTILSC_EXPORT	int		ut_NextSubDir();
_GUTILSC_EXPORT	int     ut_Normalize();
_GUTILSC_EXPORT	int     ut_ReadFromXmlFile();
_GUTILSC_EXPORT	int     ut_ReadFromIniFile();
_GUTILSC_EXPORT	int		ut_ReadLineFromIniFile();
_GUTILSC_EXPORT	int		ut_WriteSectionToIniFile();
_GUTILSC_EXPORT	int		ut_WriteEntryToIniFile();
_GUTILSC_EXPORT	int		ut_UpdateValueToIniFile();
_GUTILSC_EXPORT	int     ut_CheckDir();
_GUTILSC_EXPORT	int		ut_CreateDir();
_GUTILSC_EXPORT	int     ut_CheckFile();
_GUTILSC_EXPORT	int     ut_FindFiles();
_GUTILSC_EXPORT	int     ut_GetFileSize();
_GUTILSC_EXPORT	time_t  ut_GetFileMtime();
_GUTILSC_EXPORT	time_t  ut_GetFileCtime();
_GUTILSC_EXPORT	time_t  ut_GetFileAtime();
_GUTILSC_EXPORT	unsigned short ut_GetFileMode();
_GUTILSC_EXPORT	int		ut_CopyFile();
_GUTILSC_EXPORT	int		ut_CheckFileDiff();
_GUTILSC_EXPORT	void	ut_RemoveChar();
_GUTILSC_EXPORT	void	ut_ReplaceChar();
_GUTILSC_EXPORT	unsigned long ut_IPtoUL();
_GUTILSC_EXPORT	int		ut_GetMonthNb();
_GUTILSC_EXPORT	time_t	ut_GetCompactTimeStamp2();
_GUTILSC_EXPORT	time_t	ut_GetFullTextTimeStamp2();
_GUTILSC_EXPORT	void	ut_StringToUpper();
_GUTILSC_EXPORT	int		ut_ReadString_NoEcho();
_GUTILSC_EXPORT float   ut_RoundFloat();

#else /* !defined(__cplusplus) && !defined(ANSI) */

#if defined(__cplusplus) /* If __cplusplus defined, it means this module is exported! */
extern "C"
{
#endif /* defined(__cplusplus) */
	
	time_t  ut_GetCompactTimeStamp(char *szTimeStamp);
	time_t  ut_GetFullTextTimeStamp(char *szTimeStamp);
	void    ut_GetPID(char *szPID);
	int     ut_SplitPathName(char *szFullName, char *szPath, char *szFileName, char *szFileExt);
	int     ut_NormalizePath(char *szPath);
	int		ut_RemoveDrive(char *szPath);
	int		ut_NextSubDir(char *szPath);
	int     ut_RoundToNearest(float fValue);
	int     ut_Normalize(float *fNumber, char *szUnit);
	int     ut_ReadFromXmlFile(FILE *hFile, const char *szSection, const char *szEntry, char *szValue, int buffer_len, int nSectionsToSkip);
	int		ut_ReadFromIniFile(FILE *hFile, const char *szSection, const char *szEntry, char *szValue, int buffer_len, int *nLastCheckedLine, int nSectionsToSkip);
	int		ut_ReadLineFromIniFile(FILE *hFile, const char *szSection, char *szValue, int buffer_len, int nLineNb);
	int		ut_WriteSectionToIniFile(FILE *hFile, const char *szSection);
	int		ut_WriteEntryToIniFile(FILE *hFile, const char *szEntry, const char *szValue);
	int		ut_UpdateValueToIniFile(FILE *hFile, const char *szSection, const char *szEntry, const char *szValue, int nSectionsToSkip);
	int     ut_CheckDir(const char *szDir);
	int		ut_CreateDir(char *szDir, unsigned int uiAccessMode);
	int     ut_CheckFile(const char *szFile);
	int     ut_FindFiles(PT_UT_FindFileHandle ptHandle, char *szExt, char *szDir, char *szFileName, int nMode, int nType);
	int     ut_GetFileSize(const char *szFileName);
	time_t  ut_GetFileMtime(const char *szFileName);
	time_t  ut_GetFileCtime(const char *szFileName);
	time_t  ut_GetFileAtime(const char *szFileName);
	unsigned short ut_GetFileMode(const char *szFileName);
	int		ut_CopyFile(const char *szSourceFile, const char *szDestFile, int nMode);
	int		ut_CheckFileDiff(const char *szFile1, const char *szFile2);
	void	ut_RemoveChar(char *szString, char cChar);
	void	ut_ReplaceChar(char *szString, char cFromChar, char cToChar);
	unsigned long ut_IPtoUL(char *szIP);
	int		ut_GetMonthNb(char *szMonth);
	time_t	ut_GetCompactTimeStamp2(char *szTimeStamp, time_t tTimestamp);
	time_t	ut_GetFullTextTimeStamp2(char *szTimeStamp, time_t tTimestamp);
	void	ut_StringToUpper(char *szString);
	int		ut_ReadString_NoEcho(char *szString);
    float   ut_TruncateFloat(float Value, unsigned int Precision);
#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* !defined(__cplusplus) && !defined(ANSI) */

#endif /* #ifndef _Utils_h_ */



