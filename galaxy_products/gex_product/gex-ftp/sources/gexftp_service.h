#ifndef	GEX_FTP_SERVICE_H
#define GEX_FTP_SERVICE_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-FTP Includes
///////////////////////////////////////////////////////////////////////////////////
#include <gqtl_service.h>

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QEvent>

///////////////////////////////////////////////////////////////////////////////////
// External variables
///////////////////////////////////////////////////////////////////////////////////
extern const char* szAppFullName;
extern bool		bStartFtpSpoolerAtLaunch;

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern bool CheckGexFtpEnvironment(QString& strUserHome, QString& strApplicationDir, QString& strLocalFolder, QString& strError);

//////////////////////////////////////////	/////////////////////////////////////////
// 
// Name			:	GEXFtpService<typename Application>

// Description	:	Base class which manages the service for GEX-FTP
//
///////////////////////////////////////////////////////////////////////////////////
template <typename Application, typename FtpMain> class GEXFtpService : public QtService<Application>
{
public:

    GEXFtpService(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription);
	virtual ~GEXFtpService();

protected:

	void						start();						// Function which is called when the service starts
	void						stop();							// Function which is called when the service stops
	void						pause();						// Function which is called when the service goes to pause
    void						resume();						// Function which is called when the service resumes
	virtual FtpMain *			CreateGexFtpMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder) = 0; 
	virtual void				OnError(const QString& strError) = 0;
	virtual void				OnInitFailed()	{};
	
	FtpMain *					m_pFtpMain;

};

template <typename Application, typename FtpMain> GEXFtpService<Application, FtpMain>::GEXFtpService(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription)
			: QtService<Application>(argc, argv, strServiceName)
{
	QtService<Application>::setServiceDescription(strServiceDescription);
	QtService<Application>::setServiceFlags(QtServiceBase::CanBeSuspended);
	QtService<Application>::setStartupType(QtServiceController::AutoStartup);
}

template <typename Application, typename FtpMain> GEXFtpService<Application, FtpMain>::~GEXFtpService()
{
	
}

template <typename Application, typename FtpMain> void GEXFtpService<Application, FtpMain>::start()
{	
	QString		strUserHome, strApplicationDir, strLocalFolder, strError;

	if (CheckGexFtpEnvironment(strUserHome, strApplicationDir, strLocalFolder, strError))
	{
		m_pFtpMain = CreateGexFtpMainBase(strUserHome, strApplicationDir, strLocalFolder);
	
		// Lock successful: continue...
		bool bExitApplication;
		if((m_pFtpMain->init(bStartFtpSpoolerAtLaunch, true, &bExitApplication) == false))
		{
			if(bExitApplication)
				OnInitFailed();
		}
	}
	else
		OnError(strError);
}

template <typename Application, typename FtpMain> void GEXFtpService<Application, FtpMain>::pause()
{
	if(m_pFtpMain != NULL)
		QApplication::postEvent(m_pFtpMain, new QEvent(QEvent::Type(QEvent::User+10)));
}

template <typename Application, typename FtpMain> void GEXFtpService<Application, FtpMain>::resume()
{
	if(m_pFtpMain != NULL)
		QApplication::postEvent(m_pFtpMain, new QEvent(QEvent::Type(QEvent::User+20)));
}

template <typename Application, typename FtpMain> void GEXFtpService<Application, FtpMain>::stop()
{
	if(m_pFtpMain != NULL)
	{
		m_pFtpMain->stop();

		// Delete the pointer here to ensure that the QPaintDevice is valid.
		delete m_pFtpMain;
		m_pFtpMain = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GEXFtpServiceGui

// Description	:	Class which manages the service for GEX-Ftp with a GUI
//
///////////////////////////////////////////////////////////////////////////////////
class GEXFtpServiceGui : public GEXFtpService<QApplication, CGexFtp_mainwindow>
{
public:
	GEXFtpServiceGui(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription);
	virtual ~GEXFtpServiceGui() {};

protected:

	CGexFtp_mainwindow *		CreateGexFtpMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder);
	void						OnError(const QString& strError);
	void						OnInitFailed();
};

GEXFtpServiceGui::GEXFtpServiceGui(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription) 
					: GEXFtpService<QApplication, CGexFtp_mainwindow>(argc, argv, strServiceName, strServiceDescription)
{
	// Set GUI style
    //CGexSystemUtils::SetGuiStyle();
}

CGexFtp_mainwindow * GEXFtpServiceGui::CreateGexFtpMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder)
{	
	return new CGexFtp_mainwindow(application(), strUserHome, strApplicationDir, strLocalFolder, szAppFullName, 0, Qt::WindowTitleHint);
}

void GEXFtpServiceGui::OnError(const QString& strError)
{
	QMessageBox::critical(NULL, szAppFullName, strError);
}

void GEXFtpServiceGui::OnInitFailed()
{
	if (m_pFtpMain)
		m_pFtpMain->close();
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GEXEFtpServiceCore

// Description	:	Class which manages the service for GEX-Email without GUI
//
///////////////////////////////////////////////////////////////////////////////////
class GEXFtpServiceCore : public GEXFtpService<QCoreApplication, CGexFtpMainConsole>
{
public:
	GEXFtpServiceCore(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription);
	virtual ~GEXFtpServiceCore() {};

protected:

	CGexFtpMainConsole *		CreateGexFtpMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder);
	void						OnError(const QString& strError);
	void						OnInitFailed();
};

GEXFtpServiceCore::GEXFtpServiceCore(int argc, char **argv, const QString& strServiceName, const QString& strServiceDescription) 
						: GEXFtpService<QCoreApplication, CGexFtpMainConsole>(argc, argv, strServiceName, strServiceDescription)
{

}

CGexFtpMainConsole * GEXFtpServiceCore::CreateGexFtpMainBase(const QString& strUserHome, const QString& strApplicationDir, const QString& strLocalFolder)
{
	return new CGexFtpMainConsole(strUserHome, strApplicationDir, strLocalFolder, szAppFullName);
}

void GEXFtpServiceCore::OnError(const QString& strError)
{
	logMessage(strError, QtServiceBase::Error);
}

void GEXFtpServiceCore::OnInitFailed()
{
	OnError(m_pFtpMain->criticalMessage());
}

#endif // GEX_FTP_SERVICE_H
