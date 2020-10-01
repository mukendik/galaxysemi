/*======================================================================*/
/* Galaxy                                                               */
/* Copyright 1996-98, Softlink                                          */
/* This computer program is protected by copyrigt law                   */
/* and international treaties. Unauthorized reproduction or             */
/* distribution of this program, or any portion of it,may               */
/* result in severe civil and criminal penalties, and will be           */
/* prosecuted to the maximum extent possible under the low.             */
/*======================================================================*/

/*======================================================================*/
/*=========================== GLOGFIE.H HEADER =========================*/
/*======================================================================*/
#ifndef _GALAXY_LOGFILE_HEADER_
#define _GALAXY_LOGFILE_HEADER_

/*======================================================================*/
/* Error code definitions												*/
/*======================================================================*/
#define	GLG_ERR_OKAY			0	/* No error							*/
#define GLG_ERR_TIMESTAMP		1	/* A time stamp cannot be formatted	*/
#define GLG_ERR_STRINGTOOLONG	2	/* The specified string is too long	*/
#define GLG_ERR_INVALID_PREF	3	/* The specified preferences are not*/
									/* valid.							*/
#define GLG_ERR_OPENFILE		4	/* Cannot open the specified file.	*/
#define GLG_ERR_WRITEFILE		5	/* Cannot write to file.			*/
#define GLG_ERR_CLOSEFILE		6	/* Cannot close file.				*/
#define GLG_ERR_MEMORY			7	/* Out of memory.					*/
#define GLG_ERR_NOTFOUND		8	/* The specified log file cannot be */
									/* found.							*/
#define GLG_ERR_MEMCORRUPTED	9	/* The specified log file handle is	*/
									/* corrupted. Only available under	*/
									/* Windows.							*/
#define GLG_ERR_NULLHANDLE		10	/* The specified handle is NULL		*/
#define GLG_ERR_LOCK			11	/* Error locking the list of log 	*/
									/* file handles.					*/
#define GLG_ERR_ALREADYINIT		12	/* The log file module has been		*/
									/* already initialized.				*/
#define GLG_ERR_NOTINITIALIZED	13	/* The log file module is not initi.*/

/*======================================================================*/
/* Flags that define log module behavior								*/
/*======================================================================*/
#define GLG_FLAG_FILE		0x1	/* Log messages are sent to a file		*/
#define GLG_FLAG_SCREEN		0x2	/* Log messages are sent to the screen	*/
#define GLG_FLAG_NOHEADER	0x4	/* No header added to log messages		*/
#define GLG_FLAG_EXTRALINE	0x8	/* Add an extra '\n' after each msg		*/

/*======================================================================*/
/* Public contants														*/
/*======================================================================*/
#define GLG_MAX_PREFIXLENGTH	16
#define GLG_MAX_APPNAME			32

typedef unsigned int	glg_handle;

/* Define ANSI under SOLARIS */
#if (defined(__STDC__) && !defined(ANSI)) || defined(_WIN32)
#define ANSI
#endif

/* Need to export functions? */
#if defined(_GALAXY_LOGFILE_MODULE_)
#define _GLOGFILE_EXPORT
#else
#define _GLOGFILE_EXPORT	extern
#endif


#if !defined(__cplusplus) && !defined(ANSI)
	
	_GLOGFILE_EXPORT	int			glg_Init();	/* This function must be called before to use this module! */
	_GLOGFILE_EXPORT	int			glg_SetLogPreferences(); /* This function must be called when this module is not required anymore! */
	_GLOGFILE_EXPORT	int			glg_WriteInfoMessage();
	_GLOGFILE_EXPORT	int			glg_WriteWarningMessage();
	_GLOGFILE_EXPORT	int			glg_WriteErrorMessage();
	_GLOGFILE_EXPORT	int			glg_CreateLogFileHandle();
	_GLOGFILE_EXPORT	int			glg_DeleteLogFileHandle();
	_GLOGFILE_EXPORT	int			glg_IsValidLogFile();
	_GLOGFILE_EXPORT	char*		glg_GetLastErrorMessage();

#else /* !defined(__cplusplus) && !defined(ANSI) */

#if defined(__cplusplus) /* If __cplusplus defined, it means this module is exported! */
extern "C"
{
#endif /* defined(__cplusplus) */
	
	int			glg_Init();	/* This function must be called before to use this module! */
	int			glg_Cleanup(); /* This function must be called when this module is not required anymore! */
	int			glg_SetLogPreferences(glg_handle hLogFile,unsigned int nFlag);
	int			glg_WriteInfoMessage(glg_handle hLogFile,const char* szMessage,...);
	int			glg_WriteWarningMessage(glg_handle hLogFile,const char* szMessage,...);
	int			glg_WriteErrorMessage(glg_handle hLogFile,const char* szMessage,...);
	int			glg_CreateLogFileHandle(glg_handle* pLogFileHandle,const char*szFilePath,const char*szPrefix,const char* szAppName);
	int			glg_DeleteLogFileHandle(glg_handle hLogFile);
	int			glg_IsValidLogFile(glg_handle hLogFile);
	char*		glg_GetLastErrorMessage();

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* !defined(__cplusplus) && !defined(ANSI) */

#endif /* _GALAXY_LOGFILE_HEADER_ */

