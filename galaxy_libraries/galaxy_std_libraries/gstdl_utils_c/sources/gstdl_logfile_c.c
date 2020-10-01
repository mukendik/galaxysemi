/*======================================================================*/
/* Galaxy 																*/
/* Copyright 1996-2001, Galaxy											*/
/* This computer program is protected by copyrigt law 					*/
/* and international treaties. Unauthorized reproduction or 			*/
/* distribution of this program, or any portion of it,may 				*/
/* result in severe civil and criminal penalties, and will be 			*/
/* prosecuted to the maximum extent possible under the low. 			*/
/*======================================================================*/
/*																		*/
/* Notes :																*/
/*		* This library allow an application to write log messages in	*/
/*		  a specified file, and optionaly to the screen.				*/
/*																		*/
/*		  To use it :													*/
/*			1) Call glg_Init() to initialize this module.				*/
/*																		*/
/*			2) Call glg_CreateLogFileHandle() function that will give   */
/*			   you an handle to your log file.							*/
/*																		*/
/*			3) Use the function glg_SetLogPreferences to configure 		*/
/*			   your log file.											*/
/*																		*/
/*			4) Use glg_WriteInfoMessage, glg_WriteWarningMessage and	*/
/*			   glg_WriteErrorMessage to send message to the log.		*/
/*																		*/
/*			5) You need to call glg_DeleteLogFileHandle to free the	    */
/*			   memory allocated by the glg_CreateLogFileHandle()		*/
/*																		*/
/*			6) Call glg_Cleanup() when you don't want to use this		*/
/*			   module anymore in your application.						*/
/*																		*/
/*		* This library is thread safe, and severals thread could access	*/
/*		  to the same log file handle.									*/
/*======================================================================*/

#define _GALAXY_LOGFILE_MODULE_

/*======================================================================*/
/*=========================== GLOGFILE.C LIBRARY =======================*/
/*======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include <time.h>

#include "gstdl_logfile_c.h"

#if defined(ANSI) && defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#define _MAX_PATH		256
#endif /* defined(ANSI) */

/* include file for ellipsis arguments */
#if defined(ANSI)
#include <stdarg.h>
#else
/* Under SunOS4 only */
#include <varargs.h>
#endif
/* Includes for mutex */
#if (defined __unix__ && defined __STDC__ && defined __sun__)
#include <thread.h>
#include <synch.h>
#endif
#if (defined __unix__ && defined __STDC__ && defined __linux__)
#include <pthread.h>
#endif

/*======================================================================*/
/* Private definitions													*/
/*======================================================================*/
#define GLG_MAX_TIMESTAMP_LENGTH	32
#define GLG_MAX_APPNAME_LENGTH		32
#define GLG_MAX_HEADER_LENGTH		64
#define GLG_MAX_ERRORMSG_LENGTH		256
#define	GLG_MAX_MSG_LENGTH			1024

/*======================================================================*/
/* Private structure													*/
/*======================================================================*/

/* Define data associated with a log file */
typedef struct tagLogFile
{
	char				szFilePath[_MAX_PATH]; /* Log file path */
	char				szPrefixName[GLG_MAX_PREFIXLENGTH+1];
	char				szApplicationName[GLG_MAX_APPNAME_LENGTH+1]; /* Name used in log message */
	unsigned int		nFlag; /* Flag that gives log file behavior */
#if (defined __unix__ && defined __STDC__ && defined __sun__)
	mutex_t				mtMutex; /* Use to lock the structure for multi-thread safe */
#elif (defined __unix__ && defined __STDC__ && defined __linux__)
	pthread_mutex_t		mtMutex; /* Use to lock the structure for multi-thread safe */
#elif (defined _WIN32)
	CRITICAL_SECTION	csCriticalSection; 
#endif
	struct tagLogFile*	pNextLogFile; /* Pointer to the next structure */
	struct tagLogFile*	pPrevLogFile; /* Pointer to the previous structure */

} GLG_LOGFILE,*PT_GLG_LOGFILE;

/*======================================================================*/
/* Private global variables	declaration									*/
/*======================================================================*/
static int				glg_iInitialized = 0;
static PT_GLG_LOGFILE	g_pFirstLogFile = NULL;
static PT_GLG_LOGFILE	g_pLastLogFile = NULL;
static char				g_szErrorMessage[GLG_MAX_ERRORMSG_LENGTH];

#if (defined __unix__ && defined __STDC__ && defined __sun__)
static mutex_t			g_mtMutex = DEFAULTMUTEX;
#elif (defined __unix__ && defined __STDC__ && defined __linux__)
static pthread_mutex_t	g_mtMutex = PTHREAD_MUTEX_INITIALIZER;
#elif (defined _WIN32)
CRITICAL_SECTION		g_csCriticalSection; 
#endif

/*======================================================================*/
/* Private functions declaration										*/
/*======================================================================*/
#if defined(ANSI)
static int		glg_CreateHeader(glg_handle hLogFile, char *szHeader, char *szType);
static int		glg_GetFullTimeStamp(char* szTimeStamp);
static int		glg_GetShortTimeStamp(char *szTimeStamp);
static int		glg_WriteMessage(glg_handle hLogFile,const char* szHeader,const char* szMessage);
static int		glg_LockList();
static int		glg_SetFullLogFileName(glg_handle hLogFile, char* szFullFileName);
static void		glg_UnlockList();
static int		glg_LockLogFileHandle(PT_GLG_LOGFILE pLogFile);
static void		glg_UnlockLogFileHandle(PT_GLG_LOGFILE pLogFile);
static void		glg_FormatErrorMessage(int iErrorCode,const char* szMsg1);
#else
static int		glg_CreateHeader();
static int		glg_GetFullTimeStamp();
static int		glg_GetShortTimeStamp();
static int		glg_WriteMessage();
static int		glg_LockList();
static int		glg_SetFullLogFileName();
static void		glg_UnlockList();
static int		glg_LockLogFileHandle();
static void		glg_UnlockLogFileHandle();
static void		glg_FormatErrorMessage();
#endif // defined(ANSI)

/*======================================================================*/
/*=================== Public functions implementation ==================*/
/*======================================================================*/

/*======================================================================*/
/* Initialize the Galaxy LoG file module. This function should be called*/
/* once, when initializing the application that use it.					*/
/* You must not call it for each thread that use glg_X functions.		*/
/*																		*/
/* Return value:														*/
/*					GLG_ERR_OKAY if successful.							*/
/*					GLG_ERR_ALREADYINIT if the module has been already  */
/*								        initialized.					*/
/*======================================================================*/
int glg_Init()
{
	/* Already initialized ? */
	if(glg_iInitialized == 1)
		return GLG_ERR_ALREADYINIT;

#ifdef _WIN32
	InitializeCriticalSection(&g_csCriticalSection);
#endif
	glg_iInitialized = 1;

	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Uninitialize the Galaxy LoG file module. This function should be		*/
/* called once, when exiting the application that use it.				*/
/*																		*/
/* Return value:														*/
/*					GLG_ERR_OKAY if successful.							*/
/*					GLG_ERR_NOTINITIALIZED if the module has not been	*/
/*								           initialized.					*/
/*======================================================================*/
int glg_Cleanup()
{
	if(glg_iInitialized == 0)
		return GLG_ERR_NOTINITIALIZED;

#ifdef _WIN32
	DeleteCriticalSection(&g_csCriticalSection);
#endif

	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Create a new log file handle. This handle will be used by client to	*/
/* access the log file.													*/
/*																		*/
/* Parameter :															*/
/*		pLogFileHandle : A pointer to a log file handle that will		*/
/*						 be initialized.								*/
/*		szFilePath : The full path of the log fileto generate.			*/
/*		szPrefix   : The prefix used to create the file (e.g. : "god_"	*/
/*																		*/
/*					 The length of these strings must not exceed the	*/
/*					 _MAX_PATH definition minus the length of the time	*/
/*					 stamp inserted in the name.						*/
/*																		*/
/*																		*/
/*	Return value :														*/
/*					GLG_ERR_OKAY if successful.							*/
/*					GLG_ERR_MEMORY if the memory is full.				*/
/*					GLG_ERR_NOTINITIALIZED need to call glg_Init()		*/
/*======================================================================*/
#ifdef ANSI
int glg_CreateLogFileHandle(glg_handle* pLogFileHandle,
							const char* szFilePath,
						    const char* szPrefix,
							const char* szAppName)
#else
int glg_CreateLogFileHandle(pLogFileHandle,szFilePath,szPrefix,szAppName)
glg_handle* pLogFileHandle;
char* szFilePath;
char* szPrefix;
char* szAppName;
#endif
{
#if (defined __unix__ && defined __STDC__ && defined __linux__)
	pthread_mutexattr_t	mattr;
#endif
	int				iStatus;
	PT_GLG_LOGFILE	pNewLogFile;
	/* Make sure the module has been initialized */
	if(glg_iInitialized == 0)
		return GLG_ERR_NOTINITIALIZED;

	pNewLogFile = (PT_GLG_LOGFILE)malloc(sizeof(GLG_LOGFILE));
	if(pNewLogFile == NULL)
	{
		glg_FormatErrorMessage(GLG_ERR_MEMORY,NULL);
		return GLG_ERR_MEMORY;
	}
	/* Initialize the new structure */
	strcpy(pNewLogFile->szFilePath,szFilePath);
	strcpy(pNewLogFile->szPrefixName,szPrefix);
	strcpy(pNewLogFile->szApplicationName,szAppName);
	pNewLogFile->nFlag = 0;
#if (defined __unix__ && defined __STDC__ && defined __sun__)
	mutex_init(&pNewLogFile->mtMutex,USYNC_THREAD,NULL);
#elif (defined __unix__ && defined __STDC__ && defined __linux__)
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_PRIVATE);
	pthread_mutex_init(&pNewLogFile->mtMutex,&mattr);
#elif (defined _WIN32)
	InitializeCriticalSection(&pNewLogFile->csCriticalSection); 
#endif
	pNewLogFile->pNextLogFile = NULL;
	pNewLogFile->pPrevLogFile = NULL;
	/* Lock the list before to access it */
	iStatus = glg_LockList();
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;
	/* If the list of log file is currently empty */
	if(g_pFirstLogFile == NULL)
	{
		g_pFirstLogFile = g_pLastLogFile = pNewLogFile;
	}
	else
	/* Insert the new structure at the end of the list */
	{
		PT_GLG_LOGFILE	pNewPrev = g_pLastLogFile;
		g_pLastLogFile->pNextLogFile = pNewLogFile;
		g_pLastLogFile = pNewLogFile;
		g_pLastLogFile->pPrevLogFile = pNewPrev;
	}
	/* We have finish to access the list, unlock it */
	glg_UnlockList();
	/* Store the new log file handle */
	*pLogFileHandle = (glg_handle)pNewLogFile;

	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Delete the specified log file handle or all log file handles created.*/
/* *** WARNING: Make sure that a thread do not access the log file		*/
/*              handle before to call this function!					*/
/*																		*/
/* Parameter :															*/
/*		hLogFile : The handle to delete, or NULL to delete all handles.	*/
/*																		*/
/* Return value :														*/
/*		GLG_ERR_OKAY													*/
/*======================================================================*/
#if defined(ANSI)
int glg_DeleteLogFileHandle(glg_handle hLogFile)
#else
int glg_DeleteLogFileHandle(hLogFile)
glg_handle hLogFile;
#endif
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	int				iStatus;

	/* Lock the list before to access it */
	iStatus = glg_LockList();
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;

	if(pLogFile == NULL)
	{
		/* Delete the entire log file structure list */
		PT_GLG_LOGFILE	pCurrentLogFile = g_pFirstLogFile;
		while(pCurrentLogFile)
		{
			PT_GLG_LOGFILE	pNextLogFile = pCurrentLogFile->pNextLogFile;
			
#if (defined __unix__ && defined __STDC__)
			/* Nothing to do for the moment */
#elif defined(_WIN32)
			DeleteCriticalSection(&pCurrentLogFile->csCriticalSection);
#endif
			free(pCurrentLogFile);
			pCurrentLogFile = pNextLogFile;
		}
		g_pFirstLogFile = g_pLastLogFile = NULL;
	}
	else
	{
		/* Delete the specified log file structure */
		PT_GLG_LOGFILE	pPrevLogFile,pNextLogFile;
		
		/* Retrieve previous and next log file structure from the list */
		pPrevLogFile = pLogFile->pPrevLogFile;
		pNextLogFile = pLogFile->pNextLogFile;
		/* Attach the two log file structure */
		if(pPrevLogFile)
		{
			if(pPrevLogFile == g_pFirstLogFile)
				g_pFirstLogFile->pNextLogFile = pNextLogFile;
			pPrevLogFile->pNextLogFile = pNextLogFile;
		}
		if(pNextLogFile)
		{
			if(pNextLogFile == g_pLastLogFile)
				g_pLastLogFile->pPrevLogFile = pPrevLogFile;
			pNextLogFile->pPrevLogFile = pPrevLogFile;
		}
		if(pLogFile == g_pFirstLogFile)
			g_pFirstLogFile = pNextLogFile;
		else
		if(pLogFile == g_pLastLogFile)
			g_pLastLogFile = pPrevLogFile;

#if (defined __unix__ && defined __STDC__)
		/* Nothing to do for the moment */
#elif defined(_WIN32)
		DeleteCriticalSection(&pLogFile->csCriticalSection);
#endif
		free(pLogFile);
	}
	glg_UnlockList();

	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Test if the specified log file is valid or not.						*/
/*																		*/
/* Parameter :															*/
/*		pLogFile : A pointer to a GLG_LOGFILE structure to test.		*/
/*																		*/
/* Return value :														*/
/*		GLG_ERR_OKAY if the log file is valid.							*/
/*		GLG_ERR_NOTFOUND if the specified log file does not exist.		*/
/*		GLG_ERR_NULLHANDLE if the pointer specified is NULL.			*/
/*		GLG_ERR_MEMCORRUPTED (only under Windows) if the memory is		*/
/*		corrupted.														*/
/*======================================================================*/
#if defined(ANSI)
int glg_IsValidLogFile(glg_handle hLogFile)
#else
int glg_IsValidLogFile(hLogFile)
glg_handle hLogFile;
#endif
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	PT_GLG_LOGFILE	pCurrentLogFile = g_pFirstLogFile;
	int				iFound = 0;
	
	if(pLogFile == NULL)
	{
		glg_FormatErrorMessage(GLG_ERR_NULLHANDLE,NULL);
		return GLG_ERR_NULLHANDLE;
	}

	/* Under Windows on PC, add some other validations to help debugging */
#if defined(_WIN32) && defined(_M_IX86)
	if(IsBadReadPtr(pLogFile,sizeof(GLG_LOGFILE)) == TRUE)
	{
		glg_FormatErrorMessage(GLG_ERR_MEMCORRUPTED,NULL);
		return GLG_ERR_MEMCORRUPTED;
	}

	if(IsBadWritePtr(pLogFile,sizeof(GLG_LOGFILE)) == TRUE)
	{
		glg_FormatErrorMessage(GLG_ERR_MEMCORRUPTED,NULL);
		return GLG_ERR_MEMCORRUPTED;
	}

#endif
	/* Look if the specified structure is in the list or not */
	while(pCurrentLogFile)
	{
		if(pCurrentLogFile == pLogFile)
		{
			iFound = 1;
			break;
		}
		pCurrentLogFile = pCurrentLogFile->pNextLogFile;
	}
	/* If the specified log file cannot be found */
	if(iFound == 0)
	{
		glg_FormatErrorMessage(GLG_ERR_NOTFOUND,NULL);
		return GLG_ERR_NOTFOUND;
	}
	/* The log file has been found */
	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Set preferences for the specified log file.							*/
/*																		*/
/* Parameter :															*/
/*		hLogFile   : Handle to the log file.							*/
/*		nFlag : Contains flag that specify the behavior of this module.	*/
/*																		*/
/* Return Value :														*/
/*			GLG_ERR_OKAY is successful;									*/
/*======================================================================*/
#if defined(ANSI)
int glg_SetLogPreferences(glg_handle hLogFile,unsigned int nFlag)
#else
int glg_SetLogPreferences(hLogFile,nFlag)
glg_handle hLogFile;
unsigned int nFlag;
#endif
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	int				iStatus;

	iStatus = glg_IsValidLogFile((glg_handle)pLogFile);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;

	iStatus = glg_LockLogFileHandle(pLogFile);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;

	if( (nFlag & GLG_FLAG_FILE) ||
		(nFlag & GLG_FLAG_SCREEN) ||
		(nFlag & GLG_FLAG_NOHEADER) )
	{
		pLogFile->nFlag = nFlag;
		glg_UnlockLogFileHandle(pLogFile);
		return GLG_ERR_OKAY;
	}
	glg_UnlockLogFileHandle(pLogFile);
	glg_FormatErrorMessage(GLG_ERR_INVALID_PREF,NULL);
	return GLG_ERR_INVALID_PREF;
}

/*======================================================================*/
/* Write a log information message.										*/
/*																		*/
/* Parameter :															*/
/*		hLogFile   : Handle to the log file.							*/
/*		szMessage : The message to write in the log (file or screen)	*/
/*																		*/
/* Return value :														*/
/*		GLG_ERR_OKAY is successfull; otherwise return a GLG_ error code */
/*======================================================================*/
#ifdef ANSI
int glg_WriteInfoMessage(glg_handle hLogFile,
						 const char* szMessage,
						 ...)
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	char			szHeader[GLG_MAX_HEADER_LENGTH];
	char			szFinalMsg[GLG_MAX_MSG_LENGTH];
	int				iStatus = GLG_ERR_OKAY;
	va_list			marker;
	
	iStatus = glg_LockLogFileHandle(pLogFile);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;

	if(pLogFile->nFlag & GLG_FLAG_NOHEADER)
		*szHeader = '\0';
	else
	{
		iStatus = glg_CreateHeader(hLogFile,szHeader,"INFO");
		if(iStatus != GLG_ERR_OKAY)
			goto cleanup;
	}
	
	va_start(marker,szMessage);
	vsprintf(szFinalMsg,szMessage,marker);
	va_end(marker);

	iStatus = glg_WriteMessage(hLogFile,szHeader,szFinalMsg);
	
cleanup:
	glg_UnlockLogFileHandle(pLogFile);
	return iStatus;
}
#else
int glg_WriteInfoMessage(hLogFile,szMessage,va_alist)
glg_handle hLogFile;
char* szMessage;
va_dcl
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	char			szHeader[GLG_MAX_HEADER_LENGTH];
	char			szFinalMsg[GLG_MAX_MSG_LENGTH];
	int				iStatus = GLG_ERR_OKAY;
	va_list			marker;

	va_start(marker);
	vsprintf(szFinalMsg,szMessage,marker);
	va_end(marker);

	iStatus = glg_LockLogFileHandle(pLogFile);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;
	
	if(pLogFile->nFlag & GLG_FLAG_NOHEADER)
		*szHeader = '\0';
	else
	{
		iStatus = glg_CreateHeader(hLogFile,szHeader,"INFO");
		if(iStatus != GLG_ERR_OKAY)
			goto cleanup;
	}
	
	iStatus = glg_WriteMessage(hLogFile,szHeader,szFinalMsg);

cleanup:
	glg_UnlockLogFileHandle(pLogFile);
	return iStatus;
}
#endif

/*======================================================================*/
/* Write a log warning message.											*/
/*																		*/
/* Parameter :															*/
/*		hLogFile   : Handle to the log file.							*/
/*		szMessage : The message to write in the log (file or screen)	*/
/*																		*/
/* Return value :														*/
/*		GLG_ERR_OKAY is successfull; otherwise return a GLG_ error code */
/*======================================================================*/
#ifdef ANSI
int glg_WriteWarningMessage(glg_handle hLogFile,
							const char* szMessage,
							...)
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	char			szHeader[GLG_MAX_HEADER_LENGTH];
	char			szFinalMsg[GLG_MAX_MSG_LENGTH];
	int				iStatus = GLG_ERR_OKAY;
	va_list			marker;
	
	iStatus = glg_LockLogFileHandle(pLogFile);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;

	if(pLogFile->nFlag & GLG_FLAG_NOHEADER)
		*szHeader = '\0';
	else
	{
		iStatus = glg_CreateHeader(hLogFile,szHeader,"*WARNING");
		if(iStatus != GLG_ERR_OKAY)
			goto cleanup;
	}
	
	va_start(marker,szMessage);
	vsprintf(szFinalMsg,szMessage,marker);
	va_end(marker);

	iStatus = glg_WriteMessage(hLogFile,szHeader,szFinalMsg);
	
cleanup:
	glg_UnlockLogFileHandle(pLogFile);
	return iStatus;
}
#else
int glg_WriteWarningMessage(hLogFile,szMessage,va_alist)
glg_handle hLogFile;
char* szMessage;
va_dcl
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	char			szHeader[GLG_MAX_HEADER_LENGTH];
	char			szFinalMsg[GLG_MAX_MSG_LENGTH];
	int				iStatus = GLG_ERR_OKAY;
	va_list			marker;

	va_start(marker);
	vsprintf(szFinalMsg,szMessage,marker);
	va_end(marker);

	iStatus = glg_LockLogFileHandle(pLogFile);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;
	
	if(pLogFile->nFlag & GLG_FLAG_NOHEADER)
		*szHeader = '\0';
	else
	{
		iStatus = glg_CreateHeader(hLogFile,szHeader,"*WARNING");
		if(iStatus != GLG_ERR_OKAY)
			goto cleanup;
	}
	
	iStatus = glg_WriteMessage(hLogFile,szHeader,szFinalMsg);

cleanup:
	glg_UnlockLogFileHandle(pLogFile);
	return iStatus;
}
#endif

/*======================================================================*/
/* Write a log error message.											*/
/*																		*/
/* Parameter :															*/
/*		hLogFile   : Handle to the log file.							*/
/*		szMessage : The message to write in the log (file or screen)	*/
/*																		*/
/* Return value :														*/
/*		GLG_ERR_OKAY is successfull; otherwise return a GLG_ error code */
/*======================================================================*/
#ifdef ANSI
int glg_WriteErrorMessage(glg_handle hLogFile,
						  const char* szMessage,
						  ...)
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	char			szHeader[GLG_MAX_HEADER_LENGTH];
	char			szFinalMsg[GLG_MAX_MSG_LENGTH];
	int				iStatus = GLG_ERR_OKAY;
	va_list			marker;
	
	iStatus = glg_LockLogFileHandle(pLogFile);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;

	if(pLogFile->nFlag & GLG_FLAG_NOHEADER)
		*szHeader = '\0';
	else
	{
		iStatus = glg_CreateHeader(hLogFile,szHeader,"***ERROR");
		if(iStatus != GLG_ERR_OKAY)
			goto cleanup;
	}
	
	va_start(marker,szMessage);
	vsprintf(szFinalMsg,szMessage,marker);
	va_end(marker);

	iStatus = glg_WriteMessage(hLogFile,szHeader,szFinalMsg);
cleanup:
	glg_UnlockLogFileHandle(pLogFile);
	return iStatus;
}
#else
int glg_WriteErrorMessage(hLogFile,szMessage,va_alist)
glg_handle hLogFile;
char* szMessage;
va_dcl
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	char			szHeader[GLG_MAX_HEADER_LENGTH];
	char			szFinalMsg[GLG_MAX_MSG_LENGTH];
	int				iStatus = GLG_ERR_OKAY;
	va_list			marker;

	va_start(marker);
	vsprintf(szFinalMsg,szMessage,marker);
	va_end(marker);

	iStatus = glg_LockLogFileHandle(pLogFile);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;
	
	if(pLogFile->nFlag & GLG_FLAG_NOHEADER)
		*szHeader = '\0';
	else
	{
		iStatus = glg_CreateHeader(hLogFile,szHeader,"***ERROR");
		if(iStatus != GLG_ERR_OKAY)
			goto cleanup;
	}
	
	iStatus = glg_WriteMessage(hLogFile,szHeader,szFinalMsg);
cleanup:
	glg_UnlockLogFileHandle(pLogFile);
	return iStatus;
}
#endif

/*======================================================================*/
/* Retrieve the last error message.										*/
/*======================================================================*/
char* glg_GetLastErrorMessage()
{
	return g_szErrorMessage; 
}

/*======================================================================*/
/*================== Private functions implementation ==================*/
/*======================================================================*/

/*======================================================================*/
/* Set the full name of the specified log file.							*/
/*																		*/
/* Parameter :															*/
/*		hLogFile   : Handle to the log file.							*/
/*		szFilePath : The full path of the log fileto generate.			*/
/*		szPrefix   : The prefix used to create the file (e.g. : "god_"	*/
/*																		*/
/*					 The length of these strings must not exceed the	*/
/*					 _MAX_PATH definition minus the length of the time	*/
/*					 stamp inserted in the name.						*/
/*																		*/
/* Return Value :														*/
/*			GLG_ERR_OKAY is successful; GLG_ERR_STRINGTOOLONG if the	*/
/*			specified string length exceed _MAX_PATH.					*/
/*======================================================================*/
#ifdef ANSI
int glg_SetFullLogFileName(glg_handle hLogFile, char* szFullFileName)
#else
int glg_SetFullLogFileName(hLogFile,szFullFileName)
glg_handle hLogFile;
char* szFullFileName;
#endif
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	int				iStatus,iPathLength;
	char			szTimeStamp[GLG_MAX_TIMESTAMP_LENGTH];
	char			szPath[_MAX_PATH];

	iStatus = glg_IsValidLogFile((glg_handle)pLogFile);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;

	/* Retrieve the associated path */
	strcpy(szPath,pLogFile->szFilePath); 
	/* Make sure the last character of the path name is a '/' (\ for Windows) */
	iPathLength = strlen(szPath);
	if( (szPath[iPathLength-1] != '\\') && 
		(szPath[iPathLength-1] != '/') )
	{
		/* Add the character at the end of the string */
#ifdef __unix__
		szPath[iPathLength] = '/';
#else
		szPath[iPathLength] = '\\';
#endif
		if(iPathLength >= _MAX_PATH)
		{
			glg_FormatErrorMessage(GLG_ERR_STRINGTOOLONG,NULL);
			return GLG_ERR_STRINGTOOLONG;
		}
		/* Close the string, and update its length */
		szPath[++iPathLength] = '\0';
	}
	iStatus = glg_GetShortTimeStamp(szTimeStamp);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;
	
	/* Check file name length validity */
	if(iPathLength + strlen(pLogFile->szPrefixName) + strlen(szTimeStamp) >= _MAX_PATH)
	{
		glg_FormatErrorMessage(GLG_ERR_STRINGTOOLONG,NULL);
		return GLG_ERR_STRINGTOOLONG;
	}

	/* Store the log file name */
	(void)sprintf(szFullFileName,"%s%s%s.log",szPath,pLogFile->szPrefixName,szTimeStamp);

	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Format a log file message header.									*/
/*																		*/
/* Parameter :															*/
/*		hLogFile   : Handle to the log file.							*/
/*		szHeader : The string that receive the header.					*/
/*																		*/
/* Return value : GLG_ERR_OKAY if no error; otherwise return a GLG_		*/
/*				  error code.											*/
/*======================================================================*/
#if defined(ANSI)
static int glg_CreateHeader(glg_handle hLogFile, char *szHeader, char *szType)
#else
static int glg_CreateHeader(hLogFile,szHeader,szType)
glg_handle hLogFile;
char* szHeader;
char* szType;
#endif
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	char			szTimeStamp[GLG_MAX_TIMESTAMP_LENGTH];
	int				iStatus;

	iStatus = glg_GetFullTimeStamp(szTimeStamp);
	if(iStatus != GLG_ERR_OKAY)
		return iStatus;

	sprintf(szHeader,"[%s-%s] : %s\n",pLogFile->szApplicationName,szType,szTimeStamp);

	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Format a full time stamp in the specified string.					*/
/*																		*/
/* Parameter :															*/
/*		szTimeStamp : The string that receive the time stamp.			*/
/*																		*/
/* Return value : GLG_ERR_OKAY if no error, or GLG_ERR_TIMESTAMP if	the */
/*				  timestamp cannot be formatted.						*/
/*======================================================================*/
#if defined(ANSI)
static int glg_GetFullTimeStamp(char* szTimeStamp)
#else
static int glg_GetFullTimeStamp(szTimeStamp)
char* szTimeStamp;
#endif
{
	time_t		ltime;
	struct tm*	pltdata;
	size_t		tStringSize;
	/* Retrieve current time */
	time( &ltime );
	/* Fill tm structure */
	pltdata = localtime(&ltime);
	/* Format the timestamp string */
	tStringSize = strftime(szTimeStamp,GLG_MAX_TIMESTAMP_LENGTH,"%H:%M:%S - %b %d %Y",pltdata);
	if(tStringSize == 0)
	{
		/* The timestamp cannot be formatted ! */
		glg_FormatErrorMessage(GLG_ERR_TIMESTAMP,NULL);
		return GLG_ERR_TIMESTAMP;
	}
	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Format a short time stamp in the specified string.					*/
/*																		*/
/* Parameter :															*/
/*		szTimeStamp : The string that receive the time stamp.			*/
/*																		*/
/* Return value : GLG_ERR_OKAY if no error, or GLG_ERR_TIMESTAMP if	the */
/*				  timestamp cannot be formatted.						*/
/*======================================================================*/
#if defined(ANSI)
static int glg_GetShortTimeStamp(char *szTimeStamp)
#else
static int glg_GetShortTimeStamp(szTimeStamp)
char* szTimeStamp;
#endif
{
	time_t		ltime;
	struct tm*	pltdata;
	size_t		tStringSize;
	/* Retrieve current time */
	time( &ltime );
	/* Fill tm structure */
	pltdata = localtime(&ltime);
	/* Format the timestamp string */
	tStringSize = strftime(szTimeStamp,GLG_MAX_TIMESTAMP_LENGTH,"%m%d%Y",pltdata);
	if(tStringSize == 0)
	{
		/* The timestamp cannot be formatted ! */
		glg_FormatErrorMessage(GLG_ERR_TIMESTAMP,NULL);
		return GLG_ERR_TIMESTAMP;
	}
	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Write a message switch current preferences.							*/
/*																		*/
/* Parameter :															*/
/*		hLogFile   : Handle to the log file.							*/
/*		szHeader : The header that could be written in the log.			*/
/*		szMessage : The user message written in the log.				*/
/*																		*/
/* Return value :														*/
/*		GLG_ERR_OKAY is successful;										*/
/*======================================================================*/
#ifdef ANSI
static int glg_WriteMessage(glg_handle hLogFile,
							const char* szHeader,
							const char* szMessage)
#else
static int glg_WriteMessage(hLogFile,szHeader,szMessage)
glg_handle hLogFile;
char* szHeader;
char* szMessage;
#endif
{
	PT_GLG_LOGFILE	pLogFile = (PT_GLG_LOGFILE)hLogFile;
	int				iStatus;

	if(pLogFile->nFlag & GLG_FLAG_FILE)
	{
		FILE*	hFile;
		char	szFullLogFileName[_MAX_PATH];

		iStatus = glg_SetFullLogFileName((glg_handle)pLogFile,szFullLogFileName);
		if(iStatus != GLG_ERR_OKAY)
			return iStatus;

		/* Opens for writing at the end of the file. */
		hFile = fopen(szFullLogFileName,"a");
		if(hFile == NULL)
		{
			glg_FormatErrorMessage(GLG_ERR_OPENFILE,szFullLogFileName);
			return GLG_ERR_OPENFILE;
		}
		if(pLogFile->nFlag & GLG_FLAG_EXTRALINE)
			iStatus = fprintf(hFile,"%s%s\n\n",szHeader,szMessage);
		else
			iStatus = fprintf(hFile,"%s%s\n",szHeader,szMessage);
		if(iStatus < 0)
		{
			glg_FormatErrorMessage(GLG_ERR_WRITEFILE,szFullLogFileName);	
			return GLG_ERR_WRITEFILE;
		}
		if(fclose(hFile) == EOF)
		{
			glg_FormatErrorMessage(GLG_ERR_CLOSEFILE,szFullLogFileName);
			return GLG_ERR_CLOSEFILE;
		}
	}
	if(pLogFile->nFlag & GLG_FLAG_SCREEN)
	{
		/* Write all messages (info, warning and error) in the standard output */
		if(pLogFile->nFlag & GLG_FLAG_EXTRALINE)
			printf("%s%s\n\n",szHeader,szMessage);
		else
			printf("%s%s\n",szHeader,szMessage);
	}

	return GLG_ERR_OKAY;	
}

/*======================================================================*/
/* Lock the list for multi-thread safe access.							*/
/*																		*/
/* Return value :														*/
/*	GLG_ERR_OKAY if successful;											*/
/*	GLG_ERR_LOCK if an error occured.									*/
/*======================================================================*/
static int glg_LockList()
{
#if (defined __unix__ && defined __STDC__ && defined __sun__)
	int		iStatus;
	
	iStatus = mutex_lock(&g_mtMutex);
	if(iStatus != 0)
		return GLG_ERR_LOCK;

#elif (defined __unix__ && defined __STDC__ && defined __linux__)
	int		iStatus;
	
	iStatus = pthread_mutex_lock(&g_mtMutex);
	if(iStatus != 0)
		return GLG_ERR_LOCK;

#elif (defined _WIN32)

	EnterCriticalSection(&g_csCriticalSection);

#endif

	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Unlock the list previously locked.									*/
/*======================================================================*/
static void glg_UnlockList()
{
#if (defined __unix__ && defined __STDC__ && defined __sun__)

	(void)mutex_unlock(&g_mtMutex);

#elif (defined __unix__ && defined __STDC__ && defined __linux__)

	(void)pthread_mutex_unlock(&g_mtMutex);

#elif (defined _WIN32)

	LeaveCriticalSection(&g_csCriticalSection);

#endif
}

/*======================================================================*/
/* Lock the specified log file handle for multi-thread safe access.		*/
/*																		*/
/* Return value :														*/
/*	GLG_ERR_OKAY if successful;											*/
/*	GLG_ERR_LOCK if an error occured.									*/
/*======================================================================*/
#if defined(ANSI)
static int glg_LockLogFileHandle(PT_GLG_LOGFILE pLogFile)
#else
static int glg_LockLogFileHandle(pLogFile)
PT_GLG_LOGFILE pLogFile;
#endif
{
#if (defined __unix__ && defined __STDC__ && defined __sun__)
	int		iStatus;

	iStatus = mutex_lock(&pLogFile->mtMutex);
	if(iStatus != 0)
		return GLG_ERR_LOCK;

#elif (defined __unix__ && defined __STDC__ && defined __linux__)
	int		iStatus;

	iStatus = pthread_mutex_lock(&pLogFile->mtMutex);
	if(iStatus != 0)
		return GLG_ERR_LOCK;

#elif (defined _WIN32)

	EnterCriticalSection(&pLogFile->csCriticalSection);

#endif

	return GLG_ERR_OKAY;
}

/*======================================================================*/
/* Unlock the specified log file structure.								*/
/*======================================================================*/
#if defined(ANSI)
static void glg_UnlockLogFileHandle(PT_GLG_LOGFILE pLogFile)
#else
static void glg_UnlockLogFileHandle(pLogFile)
PT_GLG_LOGFILE pLogFile;
#endif
{
#if (defined __unix__ && defined __STDC__ && defined __sun__)

	(void)mutex_unlock(&pLogFile->mtMutex);

#elif (defined __unix__ && defined __STDC__ && defined __linux__)

	(void)pthread_mutex_unlock(&pLogFile->mtMutex);

#elif (defined _WIN32)

	LeaveCriticalSection(&pLogFile->csCriticalSection);

#endif
}

/*======================================================================*/
/* Format an error message related to this module.						*/
/*																		*/
/* Parameters :															*/
/*		iErrorCode : The module error code used to format the message.	*/
/*		szMsg1	   : An additional string to add to the message.		*/
/*======================================================================*/
#ifdef ANSI
static void glg_FormatErrorMessage(int iErrorCode,const char* szMsg1)
#else
static void glg_FormatErrorMessage(iErrorCode,szMsg1)
int iErrorCode;
char* szMsg1;
#endif
{
	switch(iErrorCode)
	{
	case GLG_ERR_OKAY:
		strcpy(g_szErrorMessage,"No error");
		break;
	case GLG_ERR_TIMESTAMP:
		strcpy(g_szErrorMessage,"The time stamp cannot be formatted");
		break;
	case GLG_ERR_STRINGTOOLONG:
		strcpy(g_szErrorMessage,"The string specified is too long");
		break;
	case GLG_ERR_INVALID_PREF:
		strcpy(g_szErrorMessage,"The specified preferences are invalid");
		break;
	case GLG_ERR_OPENFILE:
		sprintf(g_szErrorMessage,"Cannot open file : %s",szMsg1);
		break;
	case GLG_ERR_WRITEFILE:
		sprintf(g_szErrorMessage,"Cannot write to file : %s",szMsg1);
		break;
	case GLG_ERR_CLOSEFILE:
		sprintf(g_szErrorMessage,"Cannot close file : %s",szMsg1);
		break;
	case GLG_ERR_MEMORY:
		strcpy(g_szErrorMessage,"Out of memory");
		break;
	case GLG_ERR_NOTFOUND:
		strcpy(g_szErrorMessage,"The specified handle cannot be found");
		break;
	case GLG_ERR_MEMCORRUPTED:
		strcpy(g_szErrorMessage,"The memory is corrupted");
		break;
	case GLG_ERR_NULLHANDLE:
		strcpy(g_szErrorMessage,"The specified handle is NULL");
		break;
	case GLG_ERR_LOCK:
		strcpy(g_szErrorMessage,"Error occured during lock operation");
		break;
	default:
		break;

	}
}

