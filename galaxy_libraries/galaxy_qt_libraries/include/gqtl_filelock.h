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
/*============================== HEADER ================================*/
/*======================================================================*/
#ifndef _GFileLock_h_

#define _GFileLock_h_

// Project headers
#include <gstdl_constant.h>
#include <gstdl_type.h>
#include <gstdl_macro.h>

/////////////////////////////////////////////////////////////////////////////
// CGFileLock defines
typedef	int GFILELOCK_HANDLE;

/////////////////////////////////////////////////////////////////////////////
// CGFileLock class
class CGFileLock
{
// Construction
public:
	CGFileLock();
	CGFileLock(char *szLockFile);

// Attributes
public:
	enum ErrorCodes
	{
		eNone,					// No error
		eOpenFile,				// Cannot open file
		eAlreadyLocked,			// File already locked
		eLockDenied,			// Lock not successful
		eUnlockDenied,			// Unlock not successful
		eLockStatusError		// Error when checking lock status
	};

protected:
	char				m_szLockFile[GMAX_PATH];
	GFILELOCK_HANDLE	m_hLockFile;
	char				m_szLastErrorMsg[512];
	char				m_szErrorStack[512];
	int					m_iLastErrorCode;

// Operations
public:

// Implementation
public:
	virtual ~CGFileLock();
	int 	Lock();
	int  	Lock(unsigned long ulTimeout);
	int 	Unlock();
	int 	CheckLock(BOOL *pbIsLocked);
	int 	WaitForLock(unsigned long ulTimeout, BOOL *pbIsLocked);
	void	SetLockFileName(char *szLockFile);
	void	GetLockFileName(char *szLockFile);
	int		GetLastErrorCode()	{ return m_iLastErrorCode; };
	void	GetLastErrorMsg(char *szErrorMsg);

protected:
  void SetError(int iErrorCode, char* szErrorStack, const char* szInfo);
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(_GFileLock_h_)
