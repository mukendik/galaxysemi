#ifndef GEX_FTP_MAINBASE_H
#define GEX_FTP_MAINBASE_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-EMAIL Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexftp_core.h"

class QtServiceController;

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CGexFtpMainBase
//
// Description	:	Base class for main object of gex-ftp
//
///////////////////////////////////////////////////////////////////////////////////
class CGexFtpMainBase
{

public:

	CGexFtpMainBase(const QString & strUserHome, const QString & strApplicationDir, const QString & strLocalFolder, const QString & strApplicationName);
	virtual ~CGexFtpMainBase();

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	virtual bool	init(bool bStartFtpSpoolerAtLaunch, bool bRunAsService, bool *pbExitApplication, QtServiceController * pServiceController = NULL) = 0;	// Initialization
	virtual void	pause(void) = 0;		// Suspends the execution
	virtual void	resume(void) = 0;		// Resumes the execution
	virtual void	stop(void) = 0;			// Stops the execution
	
protected:

	CGexFtpCore *		m_pCore;			// Pointer on the core system
};



#endif // GEX_FTP_MAINBASE_H
