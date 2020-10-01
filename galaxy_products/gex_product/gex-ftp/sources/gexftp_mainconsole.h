#ifndef GEX_FTP_MAINCONSOLE_H
#define GEX_FTP_MAINCONSOLE_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-FTP Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexftp_mainbase.h"

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CGexFtpMainConsole
//
// Description	:	Class used for console mode
//
///////////////////////////////////////////////////////////////////////////////////
class CGexFtpMainConsole : public QObject, public CGexFtpMainBase
{
	Q_OBJECT
		
public:

	CGexFtpMainConsole(const QString & strUserHome, const QString & strApplicationDir, const QString & strLocalFolder, const QString & strApplicationName);
	virtual ~CGexFtpMainConsole();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////

	const QString&		criticalMessage() const				{ return m_strCriticalMessage; }

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	bool				init(bool bStartFtpSpoolerAtLaunch, bool bRunAsService, bool *pbExitApplication, QtServiceController * pServiceController = NULL);	// Initialization
	void				pause(void);			// Suspends the execution
	void				resume(void);			// Resumes the execution
	void				stop(void);				// Stops the execution

protected slots:

	void				onCriticalMessage(const QString& strMessage);

private:

	QString				m_strCriticalMessage;	// Error message if initialization fails
};

#endif // GEX_FTP_MAINCONSOLE_H
