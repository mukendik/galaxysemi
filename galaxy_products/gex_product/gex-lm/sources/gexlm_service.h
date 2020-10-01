#ifndef	GEXLM_SERVICE_H
#define GEXLM_SERVICE_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-LM Includes
///////////////////////////////////////////////////////////////////////////////////
#include <qmessagebox.h>
#include <gqtl_service.h>
#include <gex_shared.h>

///////////////////////////////////////////////////////////////////////////////////
// External variables
///////////////////////////////////////////////////////////////////////////////////
extern int		portnumber;
extern bool		bLicenseOK;
extern bool		bDisabledSupportPerpetualLicense;
extern QString	strApplicationDir;
extern QString	strUserHome;
extern char		szServiceDescription[100];
extern char		szServiceName[100];
extern const char* szAppFullName;

///////////////////////////////////////////////////////////////////////////////////
// External Function
///////////////////////////////////////////////////////////////////////////////////
extern void	LogMessage(QString strMessage, bool bQWarning);

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GEXLMService<typename Application>

// Description	:	Base class which manages the service for GEX-LM
//
///////////////////////////////////////////////////////////////////////////////////
template <typename Application> class GEXLMService : public QtService<Application>
{
public:

	GEXLMService(int argc, char **argv);
	virtual ~GEXLMService() {};

protected:

	virtual void	start();									// Function which is called when the service starts
	void			stop();										// Function which is called when the service stops
	virtual	void	licenseInvalid() {};						// Do specific treatments when the license is not found or invalid
    GexLicenseManager *			m_pGexServer;					// Pointer on License Manager object
	ActivationServerKeyDialog * m_pActivationServerKeyDlg;		// Pointer on the dailog that allows to activate a server key
};

template <typename Application> GEXLMService<Application>::GEXLMService(int argc, char **argv)
			: QtService<Application>(argc, argv, szServiceName), m_pGexServer(0), m_pActivationServerKeyDlg(0)
{
        QtService<Application>::setServiceDescription(szServiceDescription);
        QtService<Application>::setServiceFlags(QtServiceBase::CanBeSuspended);
        QtService<Application>::setStartupType(QtServiceController::AutoStartup);
}

template <typename Application> void GEXLMService<Application>::start()
{	
	if (bLicenseOK)
	{
		// Port used message
		QString strMessage= "Socket Port used: ";
		strMessage += QString::number(portnumber);
		LogMessage(strMessage, true);

		// Instanciate server object
		m_pGexServer = new GexLicenseManager(portnumber);
	}
	else
		licenseInvalid();
}

template <typename Application> void GEXLMService<Application>::stop()
{
	// Clean data before quit
	if (m_pGexServer)
		delete m_pGexServer;

	if (m_pActivationServerKeyDlg)
		delete m_pActivationServerKeyDlg;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GEXLMServiceGui

// Description	:	Class which manages the service for GEX-LM with a GUI
//
///////////////////////////////////////////////////////////////////////////////////
class GEXLMServiceGui : public GEXLMService<QApplication>
{
public:
	GEXLMServiceGui(int argc, char **argv);
	virtual ~GEXLMServiceGui() {};

protected:

	void start();				// Function which is called when the service starts
	void pause();				// Function which is called when the service goes to pause
    void resume();				// Function which is called when the service resumes
	void licenseInvalid();		// Display the dialog which allow to activate a server key
};

GEXLMServiceGui::GEXLMServiceGui(int argc, char **argv) : GEXLMService<QApplication>(argc, argv)
{

}

void GEXLMServiceGui::start()
{	
    CGexSystemUtils::SetGuiStyle();
	
	GEXLMService<QApplication>::start();
}

void GEXLMServiceGui::licenseInvalid()
{
	QString strMessage;
	if(bDisabledSupportPerpetualLicense)
		strMessage = "Your maintenance contract has expired. You can't run this recent release.\nContact the Galaxy sales team to upgrade your contract.";
	else
		strMessage = "The license file was not found or is invalid. Please contact "+QString(GEX_EMAIL_SALES)+"\n";

	QMessageBox::information(NULL,szAppFullName,strMessage);
	logMessage(strMessage, QtServiceBase::Error);

	m_pActivationServerKeyDlg = new ActivationServerKeyDialog(strUserHome, strApplicationDir, 0);

	if (m_pActivationServerKeyDlg)
		m_pActivationServerKeyDlg->show();
}

void GEXLMServiceGui::pause()
{
}

void GEXLMServiceGui::resume()
{
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GEXLMServiceCore

// Description	:	Class which manages the service for GEX-LM without GUI
//
///////////////////////////////////////////////////////////////////////////////////
class GEXLMServiceCore : public GEXLMService<QCoreApplication>
{
public:
	GEXLMServiceCore(int argc, char **argv);
	virtual ~GEXLMServiceCore() {};

protected:

	void pause();				// Function which is called when the service goes to pause
    void resume();				// Function which is called when the service resumes
	void licenseInvalid();		// Display the dialog which allow to activate a server key
};

GEXLMServiceCore::GEXLMServiceCore(int argc, char **argv) : GEXLMService<QCoreApplication>(argc, argv)
{

}

void GEXLMServiceCore::licenseInvalid()
{
	QString strMessage;
	if(bDisabledSupportPerpetualLicense)
		strMessage = "Your maintenance contract has expired. You can't run this recent release.\nContact the Galaxy sales team to upgrade your contract.";
	else
		strMessage = "The license file was not found or is invalid. Please contact "+QString(GEX_EMAIL_SALES)+"\n";

	logMessage(strMessage, QtServiceBase::Error);

    exit(0);
}

void GEXLMServiceCore::pause()
{
}

void GEXLMServiceCore::resume()
{
}

#endif // GEXLM_SERVICE_H
