#ifndef	G_TRIGGER_SERVICE_H
#define G_TRIGGER_SERVICE_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-EMAIL Includes
///////////////////////////////////////////////////////////////////////////////////
#include <gqtl_service.h>

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QEvent>

///////////////////////////////////////////////////////////////////////////////////
// External variables
///////////////////////////////////////////////////////////////////////////////////
extern char szAppFullName[100];

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern bool CheckEnvironment(QString& strUserHome, QString& strApplicationDir, QString& strLocalFolder, QString& strError);

//////////////////////////////////////////	/////////////////////////////////////////
// 
// Name			:	GTriggerService<typename Application>

// Description	:	Base class which manages the service for G-Trigger
//
///////////////////////////////////////////////////////////////////////////////////
template <typename Application, typename GTriggerMain> class GTriggerService : public QtService<Application>
{
public:

    GTriggerService(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription);
	virtual ~GTriggerService();

protected:

	void						start();						// Function which is called when the service starts
	void						stop();							// Function which is called when the service stops
	void						pause();						// Function which is called when the service goes to pause
    void						resume();						// Function which is called when the service resumes
	virtual GTriggerMain *		CreateGTriggerMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder) = 0; 
	virtual void				OnError(const QString& strError) = 0;
	virtual void				OnInitFailed()	{};
	
	GTriggerMain *				m_pGTriggerMain;

};

template <typename Application, typename GTriggerMain> GTriggerService<Application, GTriggerMain>::GTriggerService(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription)
			: QtService<Application>(argc, argv, strServiceName)
{
	QtService<Application>::setServiceDescription(strServiceDescription);
	QtService<Application>::setServiceFlags(QtServiceBase::CanBeSuspended);
	QtService<Application>::setStartupType(QtServiceController::AutoStartup);
}

template <typename Application, typename GTriggerMain> GTriggerService<Application, GTriggerMain>::~GTriggerService()
{
	
}

template <typename Application, typename GTriggerMain> void GTriggerService<Application, GTriggerMain>::start()
{	
	QString		strUserHome, strApplicationDir, strLocalFolder, strError;

	if (CheckEnvironment(strUserHome, strApplicationDir, strLocalFolder, strError))
	{
		m_pGTriggerMain = CreateGTriggerMainBase(strUserHome, strApplicationDir, strLocalFolder);
	
		// Lock successful: continue...
		bool bExitApplication;
		if((m_pGTriggerMain->init(true, &bExitApplication) == false))
		{
			if(bExitApplication)
				OnInitFailed();
		}
	}
	else
		OnError(strError);
}

template <typename Application, typename GTriggerMain> void GTriggerService<Application, GTriggerMain>::pause()
{
	if(m_pGTriggerMain != NULL)
		QApplication::postEvent(m_pGTriggerMain, new QEvent(QEvent::Type(QEvent::User+10)));
}

template <typename Application, typename GTriggerMain> void GTriggerService<Application, GTriggerMain>::resume()
{
	if(m_pGTriggerMain != NULL)
		QApplication::postEvent(m_pGTriggerMain, new QEvent(QEvent::Type(QEvent::User+20)));
}

template <typename Application, typename GTriggerMain> void GTriggerService<Application, GTriggerMain>::stop()
{
	if(m_pGTriggerMain != NULL)
	{
		m_pGTriggerMain->stop();

		// Delete the pointer here to ensure that the QPaintDevice is valid.
		delete m_pGTriggerMain;
		m_pGTriggerMain = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GTriggerServiceGui

// Description	:	Class which manages the service for G-Trigger with a GUI
//
///////////////////////////////////////////////////////////////////////////////////
class GTriggerServiceGui : public GTriggerService<QApplication, CGTriggerMainwindow>
{
public:
	GTriggerServiceGui(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription);
	virtual ~GTriggerServiceGui() {};

protected:

	CGTriggerMainwindow *		CreateGTriggerMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder);
	void						OnError(const QString& strError);
	void						OnInitFailed();
};

GTriggerServiceGui::GTriggerServiceGui(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription) 
					: GTriggerService<QApplication, CGTriggerMainwindow>(argc, argv, strServiceName, strServiceDescription)
{
	// Set GUI style
    CGexSystemUtils::SetGuiStyle();
}

CGTriggerMainwindow * GTriggerServiceGui::CreateGTriggerMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder)
{	
	return new CGTriggerMainwindow(application(), strUserHome, strApplicationDir, strLocalFolder, szAppFullName, 0, Qt::WindowTitleHint);
}

void GTriggerServiceGui::OnError(const QString& strError)
{
	QMessageBox::critical(NULL, szAppFullName, strError);
}

void GTriggerServiceGui::OnInitFailed()
{
	if (m_pGTriggerMain)
		m_pGTriggerMain->close();
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GTriggerServiceCore

// Description	:	Class which manages the service for G-Trigger without GUI
//
///////////////////////////////////////////////////////////////////////////////////
class GTriggerServiceCore : public GTriggerService<QCoreApplication, CGTriggerMainConsole>
{
public:
	GTriggerServiceCore(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription);
	virtual ~GTriggerServiceCore() {};

protected:

	CGTriggerMainConsole *		CreateGTriggerMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder);
	void						OnError(const QString& strError);
	void						OnInitFailed();
};

GTriggerServiceCore::GTriggerServiceCore(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription) 
						: GTriggerService<QCoreApplication, CGTriggerMainConsole>(argc, argv, strServiceName, strServiceDescription)
{

}

CGTriggerMainConsole * GTriggerServiceCore::CreateGTriggerMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder)
{
	return new CGTriggerMainConsole(strUserHome, strApplicationDir, strLocalFolder, szAppFullName);
}

void GTriggerServiceCore::OnError(const QString& strError)
{
	logMessage(strError, QtServiceBase::Error);
}

void GTriggerServiceCore::OnInitFailed()
{
	OnError(m_pGTriggerMain->criticalMessage());
}
#endif // G_TRIGGER_SERVICE_H
