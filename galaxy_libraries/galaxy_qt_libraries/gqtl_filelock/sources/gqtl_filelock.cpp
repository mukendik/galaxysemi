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

/*======================================================================*/
/* GLOCKFILE Module : source file										*/
/*																		*/
/* This module implements a file locking mechanism.						*/
/* On Windows OS, it uses functions open() and locking().				*/
/* On unix OS, it uses functions open() and fcntl().					*/
/*																		*/
/*======================================================================*/
/* NOTE :																*/
/*																		*/
/*======================================================================*/
 
#define _GLockFileModule_ 1

// Project headers
#include "gqtl_filelock.h"

// Standard headers
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(__unix__) || __MACH__
#include <unistd.h>
#include <errno.h>
#elif defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <sys/locking.h>
#endif

#if defined(_WIN32)
#	define GFILELOCK_SLEEP(x)		Sleep(x*1000)
#	define GFILELOCK_PERMISSIONS	S_IWRITE | S_IREAD
#elif defined(__unix__) || __MACH__
#	define GFILELOCK_SLEEP(x)		sleep(x)
#	define GFILELOCK_PERMISSIONS	S_IWUSR | S_IRUSR
#endif
#define GFILELOCK_INVALID_HANDLE	-1
#define GFILELOCK_LOCKERROR			-1

/////////////////////////////////////////////////////////////////////////////
// CGFileLock

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Default constructor
//
// Argument(s) :
//
// Return      : 
/////////////////////////////////////////////////////////////////////////////////////////////
CGFileLock::CGFileLock()
{
	SetError(eNone, NULL, "");
	strcpy(m_szLockFile, "");
	m_hLockFile = GFILELOCK_INVALID_HANDLE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//      char *szLockFile	: file name for locking / unlocking operations
//
// Return      : 
/////////////////////////////////////////////////////////////////////////////////////////////
CGFileLock::CGFileLock(char *szLockFile)
{
	SetError(eNone, NULL, "");
	strcpy(m_szLockFile, szLockFile);
	m_hLockFile = GFILELOCK_INVALID_HANDLE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
//
// Argument(s) :
//
// Return      : 
/////////////////////////////////////////////////////////////////////////////////////////////
CGFileLock::~CGFileLock()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Set lock file name
//
// Argument(s) :
//      char *szLockFile	: file name for locking / unlocking operations
//
// Return      : 
/////////////////////////////////////////////////////////////////////////////////////////////
void CGFileLock::SetLockFileName(char *szLockFile)
{
	strcpy(m_szLockFile, szLockFile);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Get lock file name
//
// Argument(s) :
//      char *szLockFile	: buffer to receive file name for locking / unlocking operations
//
// Return      : 
/////////////////////////////////////////////////////////////////////////////////////////////
void CGFileLock::GetLockFileName(char *szLockFile)
{
	strcpy(szLockFile, m_szLockFile);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Lock file
//
// Argument(s) :
//
// Return      : 1 if file successfully locked, 0 else
/////////////////////////////////////////////////////////////////////////////////////////////
int CGFileLock::Lock()
{
#if defined(__unix__) || __MACH__
	struct flock	stFileLock;
#endif
	int				iStatus;

	// Check FileName
	ASSERT(strcmp(m_szLockFile, "") != 0);

	// Check handle
	if(m_hLockFile != GFILELOCK_INVALID_HANDLE)
	{
		SetError(eAlreadyLocked, NULL, "");
		return 0;
	}
	
	// Get handle
	m_hLockFile = open(m_szLockFile, O_RDWR | O_CREAT, GFILELOCK_PERMISSIONS);
	if(m_hLockFile == GFILELOCK_INVALID_HANDLE)
	{
		SetError(eOpenFile, NULL, "");
		return 0;
	}

#if defined (__unix__) || __MACH__
	// On unix, the file creation mask can affect the file mode. Make sure the file has the expected mode with chmod
	chmod(m_szLockFile, GFILELOCK_PERMISSIONS);

	// Try to lock file
	stFileLock.l_type = F_WRLCK;
	stFileLock.l_whence = SEEK_SET;
	stFileLock.l_start = 0;
	stFileLock.l_len = 0;

	iStatus = fcntl(m_hLockFile, F_SETLK, &stFileLock);
#elif defined(_WIN32)
	// Try to lock file
        iStatus = _locking(m_hLockFile, _LK_NBLCK, 1);
#endif

	if(iStatus == GFILELOCK_LOCKERROR)
	{
		close(m_hLockFile);
		m_hLockFile = GFILELOCK_INVALID_HANDLE;
		SetError(eLockDenied, NULL, "");
		return 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Lock file with timeout. If the lock doesn't succeed at first trial, this
//				 function will try again each 1 sec. until timeout reached
//
// Argument(s) :
//      unsigned long ulTimeout	: timeout value in milliseconds
//
// Return      : 1 if file successfully locked, 0 else
/////////////////////////////////////////////////////////////////////////////////////////////
int CGFileLock::Lock(unsigned long ulTimeout)
{
	time_t	tStart;
	int		iStatus;

	// Get start time for timeout check
	tStart = time(NULL);

	// Loop until Lock succeeds or timeout
	while(((iStatus = Lock()) == 0) && ((unsigned long)(time(NULL) - tStart) < ulTimeout))
		GFILELOCK_SLEEP(1);

	return iStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Unlock file
//
// Argument(s) :
//
// Return      : 1 if file successfully locked, 0 else
/////////////////////////////////////////////////////////////////////////////////////////////
int CGFileLock::Unlock()
{
#if defined(__unix__) || __MACH__
	struct flock	stFileLock;
#endif
	int				iStatus;

	// Check FileName
	ASSERT(strcmp(m_szLockFile, "") != 0);

	// Check handle
	if(m_hLockFile == GFILELOCK_INVALID_HANDLE)
		return 0;

#if defined(__unix__) || __MACH__
	// Try to unlock file
	stFileLock.l_type = F_UNLCK;
	stFileLock.l_whence = SEEK_SET;
	stFileLock.l_start = 0;
	stFileLock.l_len = 0;

	iStatus = fcntl(m_hLockFile, F_SETLK, &stFileLock);
#elif defined(_WIN32)
	// Try to unlock file
        iStatus = _locking(m_hLockFile, _LK_UNLCK, 1);
#endif

	if(iStatus == GFILELOCK_LOCKERROR)
	{
		SetError(eUnlockDenied, NULL, "");
		return 0;
	}

	// Close file handle
	close(m_hLockFile);
	m_hLockFile = GFILELOCK_INVALID_HANDLE;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Check if file is locked
//
// Argument(s) :
//      BOOL *pbIsLocked	: set to TRUE if file is locked, set to FALSE else
//
// Return      : 1 if file successfully locked, 0 else
/////////////////////////////////////////////////////////////////////////////////////////////
int CGFileLock::CheckLock(BOOL *pbIsLocked)
{
#if defined(__unix__) || __MACH__
	struct flock	stFileLock;
	int				iStatus;
#endif

	*pbIsLocked = FALSE;

	// Check FileName
	ASSERT(strcmp(m_szLockFile, "") != 0);

	// Check handle
	if(m_hLockFile != GFILELOCK_INVALID_HANDLE)
	{
		*pbIsLocked = TRUE;
		return 1;
	}
	
	// Get handle
	m_hLockFile = open(m_szLockFile, O_RDWR | O_CREAT, GFILELOCK_PERMISSIONS);
	if(m_hLockFile == GFILELOCK_INVALID_HANDLE)
	{
		SetError(eOpenFile, NULL, "");
		return 0;
	}

#if defined(__unix__) || __MACH__
	// On unix, the file creation mask can affect the file mode. Make sure the file has the expected mode with chmod
	chmod(m_szLockFile, GFILELOCK_PERMISSIONS);

	// Check if lock would succeed
	stFileLock.l_type = F_WRLCK;
	stFileLock.l_whence = SEEK_SET;
	stFileLock.l_start = 0;
	stFileLock.l_len = 0;

	iStatus = fcntl(m_hLockFile, F_GETLK, &stFileLock);
	close(m_hLockFile);
	m_hLockFile = GFILELOCK_INVALID_HANDLE;	
	
	if(iStatus == GFILELOCK_LOCKERROR)
	{
		SetError(eLockStatusError, NULL, "");
		return 0;
	}
	
	if(stFileLock.l_type == F_UNLCK)
	{
		*pbIsLocked = FALSE;
		return 1;
	}

	*pbIsLocked = TRUE;
	return 1;
#elif defined(_WIN32)
	// Check if lock would succeed
        if(_locking(m_hLockFile, _LK_NBLCK, 1) == 0)
	{		
                _locking(m_hLockFile, _LK_UNLCK, 1);
		close(m_hLockFile);
		m_hLockFile = GFILELOCK_INVALID_HANDLE;
		*pbIsLocked = FALSE;
		return 1;
	}
	close(m_hLockFile);
	m_hLockFile = GFILELOCK_INVALID_HANDLE;
	*pbIsLocked = TRUE;
	return 1;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Wait until file is locked with a timeout. If the file is not locked at first 
//				 trial, this function will check again each 1 sec. until timeout reached
//
// Argument(s) :
//      unsigned long ulTimeout	: timeout in microseconds
//      BOOL *pbIsLocked		: set to TRUE if file is locked, set to FALSE else
//
// Return      : 1 if function successfull, 0 else
/////////////////////////////////////////////////////////////////////////////////////////////
int CGFileLock::WaitForLock(unsigned long ulTimeout, BOOL *pbIsLocked)
{
	time_t	tStart;
	int		iStatus;

	// Get start time for timeout check
	tStart = time(NULL);

	*pbIsLocked = FALSE;
	// Loop until Lock succeeds or timeout
	while((((iStatus = CheckLock(pbIsLocked)) == 0) || (*pbIsLocked == FALSE)) && ((unsigned long)(time(NULL) - tStart) < ulTimeout))
		GFILELOCK_SLEEP(1);

	return iStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Set last error code and error message
//
// Argument(s) :
//      int iErrorCode :
//
// Return      : 
/////////////////////////////////////////////////////////////////////////////////////////////
void
CGFileLock::SetError(int iErrorCode,
                     char* szErrorStack,
                     const char* /*szInfo*/)
{
	m_iLastErrorCode = iErrorCode;

	switch(iErrorCode)
	{
		case eNone:
			strcpy(m_szLastErrorMsg, "");
			break;

		case eOpenFile:
			sprintf(m_szLastErrorMsg, "Couldn't open file: %s (%s)", m_szLockFile, strerror(errno));
			break;

		case eAlreadyLocked:
			sprintf(m_szLastErrorMsg, "File already locked: %s\n", m_szLockFile);
			break;

		case eLockDenied:
			sprintf(m_szLastErrorMsg, "Lock denied on file: %s (%s)", m_szLockFile, strerror(errno));
			break;

		case eUnlockDenied:
			sprintf(m_szLastErrorMsg, "Unlock denied on file: %s (%s)", m_szLockFile, strerror(errno));
			break;

		case eLockStatusError:
			sprintf(m_szLastErrorMsg, "Couldn't get lock status on file: %s (%s)", m_szLockFile, strerror(errno));
			break;

		default:
			strcpy(m_szLastErrorMsg, "");
			break;
	}

	if(szErrorStack != NULL)
	{
		strcat(m_szLastErrorMsg, "\n");
		strcat(m_szLastErrorMsg, szErrorStack);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Get last error message
//
// Argument(s) :
//      char *szErrorMsg : 
//
// Return      : 
/////////////////////////////////////////////////////////////////////////////////////////////
void CGFileLock::GetLastErrorMsg(char *szErrorMsg)
{
	strcpy(szErrorMsg, m_szLastErrorMsg);
}

/////////////////////////////////////////////////////////////////////////////
