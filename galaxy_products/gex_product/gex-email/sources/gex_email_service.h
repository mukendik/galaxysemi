#ifndef	GEX_EMAIL_SERVICE_H
#define GEX_EMAIL_SERVICE_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-EMAIL Includes
///////////////////////////////////////////////////////////////////////////////////
#include <gqtl_service.h>
#include <QMessageBox>

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QEvent>

///////////////////////////////////////////////////////////////////////////////////
// External variables
///////////////////////////////////////////////////////////////////////////////////
extern char szAppFullName[100];
extern bool bAllowUserToSelectEmailSpoolingDir;
extern bool bStartEmailSpoolerAtLaunch;

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern bool CheckGexEmailEnvironment(QString& strUserHome, QString& strApplicationDir, QString& strLocalFolder, QString& strError);

//////////////////////////////////////////	/////////////////////////////////////////
// 
// Name			:	GEXEMailService<typename Application>

// Description	:	Base class which manages the service for GEX-Email
//
///////////////////////////////////////////////////////////////////////////////////
template <typename Application, typename EmailMain> class GEXEMailService : public QtService<Application>
{
public:

    GEXEMailService(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription);
	virtual ~GEXEMailService();

protected:

	void						start();						// Function which is called when the service starts
	void						stop();							// Function which is called when the service stops
	void						pause();						// Function which is called when the service goes to pause
    void						resume();						// Function which is called when the service resumes
	virtual EmailMain *			CreateGexEmailMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder) = 0; 
	virtual void				OnError(const QString& strError) = 0;
	virtual void				OnInitFailed()	{};
	
	EmailMain *					m_pEmailMain;

};

template <typename Application, typename EmailMain> GEXEMailService<Application, EmailMain>::GEXEMailService(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription)
			: QtService<Application>(argc, argv, strServiceName)
{
	QtService<Application>::setServiceDescription(strServiceDescription);
	QtService<Application>::setServiceFlags(QtServiceBase::CanBeSuspended);
	QtService<Application>::setStartupType(QtServiceController::AutoStartup);
}

template <typename Application, typename EmailMain> GEXEMailService<Application, EmailMain>::~GEXEMailService()
{
	
}

template <typename Application, typename EmailMain> void GEXEMailService<Application, EmailMain>::start()
{	
	QString		strUserHome, strApplicationDir, strLocalFolder, strError;

	if (CheckGexEmailEnvironment(strUserHome, strApplicationDir, strLocalFolder, strError))
	{
		m_pEmailMain = CreateGexEmailMainBase(strUserHome, strApplicationDir, strLocalFolder);
	
		// Lock successful: continue...
		bool bExitApplication;
		if((m_pEmailMain->init(bStartEmailSpoolerAtLaunch, bAllowUserToSelectEmailSpoolingDir, true, &bExitApplication) == false))
		{
			if(bExitApplication)
				OnInitFailed();
		}
	}
	else
		OnError(strError);
}

template <typename Application, typename EmailMain> void GEXEMailService<Application, EmailMain>::pause()
{
	if(m_pEmailMain != NULL)
		QApplication::postEvent(m_pEmailMain, new QEvent(QEvent::Type(QEvent::User+10)));
}

template <typename Application, typename EmailMain> void GEXEMailService<Application, EmailMain>::resume()
{
	if(m_pEmailMain != NULL)
		QApplication::postEvent(m_pEmailMain, new QEvent(QEvent::Type(QEvent::User+20)));
}

template <typename Application, typename EmailMain> void GEXEMailService<Application, EmailMain>::stop()
{
	if(m_pEmailMain != NULL)
	{
		m_pEmailMain->stop();

		// Delete the pointer here to ensure that the QPaintDevice is valid.
		delete m_pEmailMain;
		m_pEmailMain = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GEXEMailServiceGui

// Description	:	Class which manages the service for GEX-Email with a GUI
//
///////////////////////////////////////////////////////////////////////////////////
class GEXEMailServiceGui : public GEXEMailService<QApplication, CGexEmailMainwindow>
{
public:
	GEXEMailServiceGui(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription);
	virtual ~GEXEMailServiceGui() {};

protected:

	CGexEmailMainwindow *		CreateGexEmailMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder);
	void						OnError(const QString& strError);
	void						OnInitFailed();
};

GEXEMailServiceGui::GEXEMailServiceGui(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription) 
					: GEXEMailService<QApplication, CGexEmailMainwindow>(argc, argv, strServiceName, strServiceDescription)
{
	// Set GUI style
    CGexSystemUtils::SetGuiStyle();
}

CGexEmailMainwindow * GEXEMailServiceGui::CreateGexEmailMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder)
{	
	return new CGexEmailMainwindow(application(), strUserHome, strApplicationDir, strLocalFolder, szAppFullName, 0, Qt::WindowTitleHint);
}

void GEXEMailServiceGui::OnError(const QString& strError)
{
	QMessageBox::critical(NULL, szAppFullName, strError);
}

void GEXEMailServiceGui::OnInitFailed()
{
	if (m_pEmailMain)
		m_pEmailMain->close();
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GEXEMailServiceCore

// Description	:	Class which manages the service for GEX-Email without GUI
//
///////////////////////////////////////////////////////////////////////////////////
class GEXEMailServiceCore : public GEXEMailService<QCoreApplication, CGexEmailMainConsole>
{
public:
	GEXEMailServiceCore(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription);
	virtual ~GEXEMailServiceCore() {};

protected:

	CGexEmailMainConsole *		CreateGexEmailMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder);
	void						OnError(const QString& strError);
	void						OnInitFailed();
};

GEXEMailServiceCore::GEXEMailServiceCore(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription) 
						: GEXEMailService<QCoreApplication, CGexEmailMainConsole>(argc, argv, strServiceName, strServiceDescription)
{

}

CGexEmailMainConsole * GEXEMailServiceCore::CreateGexEmailMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder)
{
	return new CGexEmailMainConsole(strUserHome, strApplicationDir, strLocalFolder, szAppFullName);
}

void GEXEMailServiceCore::OnError(const QString& strError)
{
	logMessage(strError, QtServiceBase::Error);
}

void GEXEMailServiceCore::OnInitFailed()
{
	OnError(m_pEmailMain->criticalMessage());
}
#endif // GEX_EMAIL_SERVICE_H
