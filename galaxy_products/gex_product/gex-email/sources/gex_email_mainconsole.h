#ifndef GEX_EMAIL_MAINCONSOLE_H
#define GEX_EMAIL_MAINCONSOLE_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-EMAIL Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_email_mainbase.h"

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CGexEmailMainConsole
//
// Description	:	Class used for console mode
//
///////////////////////////////////////////////////////////////////////////////////
class CGexEmailMainConsole : public QObject, public CGexEmailMainBase
{
	Q_OBJECT
		
public:

	CGexEmailMainConsole(const QString & strUserHome, const QString & strApplicationDir, const QString & strLocalFolder, const QString & strApplicationName);
	virtual ~CGexEmailMainConsole();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////

	const QString&		criticalMessage() const				{ return m_strCriticalMessage; }

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	bool		init(bool bStartEmailSpoolerAtLaunch, bool bAllowUserToSelectEmailSpoolingDir, bool bRunAsService, bool *pbExitApplication);	// Initialization
	void		pause(void);			// Suspends the execution
	void		resume(void);			// Resumes the execution
	void		stop(void);				// Stops the execution

protected slots:

	void		onCriticalMessage(const QString& strMessage);

private:

	QString		m_strCriticalMessage;	// Error message if initialization fails
};

#endif // GEX_EMAIL_MAINCONSOLE_H
