#ifndef GEX_EMAIL_MAINBASE_H
#define GEX_EMAIL_MAINBASE_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-EMAIL Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_email_core.h"

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CGexEmailMainBase
//
// Description	:	Base class for main object of gex-email
//
///////////////////////////////////////////////////////////////////////////////////
class CGexEmailMainBase
{

public:

	CGexEmailMainBase(const QString & strUserHome, const QString & strApplicationDir, const QString & strLocalFolder, const QString & strApplicationName);
	virtual ~CGexEmailMainBase();

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	virtual bool	init(bool bStartEmailSpoolerAtLaunch, bool bAllowUserToSelectEmailSpoolingDir, bool bRunAsService, bool *pbExitApplication) = 0;	// Initialization
	virtual void	pause(void) = 0;		// Suspends the execution
	virtual void	resume(void) = 0;		// Resumes the execution
	virtual void	stop(void) = 0;			// Stops the execution
	
protected:

	CGexEmailCore *		m_pCore;
};



#endif // GEX_EMAIl_MAINBASE_H
