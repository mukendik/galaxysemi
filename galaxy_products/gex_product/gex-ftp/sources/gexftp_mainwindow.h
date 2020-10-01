#ifndef CGexFtp_mainwindow_H
#define CGexFtp_mainwindow_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-FTP Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexftp_mainbase.h"
#include "gexftp_constants.h"
#include "gexftp_settings.h"
#include "gexftp_servertransfer_widget.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <qlist.h>
#include <qpixmap.h>
#include <qdatetime.h>

///////////////////////////////////////////////////////////////////////////////////
// GUI Includes
///////////////////////////////////////////////////////////////////////////////////
#include "ui_gexftp_mainwindow_base.h"

class QTime;
class CGexFtpClient;
class QApplication;
class QtServiceController;
class CGexFtpService_widget;
class CGexFtpPage_widget;

typedef struct t_GexFtp_Page
{
	QWidget*	pPage;
	bool		bDisableWhenRunning;
} GexFtp_Page;

class CGexFtp_mainwindow : public QMainWindow, public Ui::CGexFtpMainWindowBase, public CGexFtpMainBase
{
	Q_OBJECT
		
public:

    CGexFtp_mainwindow(QApplication* qApplication, const QString & strUserHome, const QString & strApplicationPath, const QString& strLocalFolder, const QString & strApplicationName, QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~CGexFtp_mainwindow();

	bool	init(bool bStartFtpSpoolerAtLaunch, bool bRunAsService, bool *pbExitApplication, QtServiceController * pServiceController = NULL);
	void	pause(void);
	void	resume(void);
	void	stop(void);

protected:
	
	void	initGui(QtServiceController * pServiceController);						// Initialize gui
	void	ResizeApplication(void);
    void	SetStatus(int nStatus, const QString& strArg1 = "", const QString& strArg2 = "");
	void	AddPage(const QString& strSelectionText, const QString& strSelectionPixmap, QWidget* pPage, bool bDisableWhenRunning);

// Overrides
protected:

	void	closeEvent(QCloseEvent * e);
	void	customEvent(QEvent * e);

protected:

	QList<GexFtp_Page>				m_listPages;					// List of pages displayed 
	QString							m_strImagesDirectory;			// Image folder
    bool							m_bStartEnabled;				// Set to true when the start button is enabled
	QPixmap							m_pxInfo;						// Pixmap for information messages
	QPixmap							m_pxInfoTransfer;				// Pixmap for 'file transferred' information messages
	QPixmap							m_pxError;						// Pixmap for error messages
	QPixmap							m_pxWarning;					// Pixmap for warning messages
    bool							m_bServicePaused;				// Set to true when running as a service and the service is paused
	int								m_nPageNb;						// Number of pages
	int								m_nPage_Help;					// ID of the Help page
	int								m_nPage_General;				// ID of the General page
	int								m_nPage_Ftp;					// ID of the FTP page
	int								m_nPage_Log;					// ID of the Log page
	int								m_nPage_Service;				// ID of the service page
	
	QApplication *					m_qApplication;					// QT Application
	CGexFtpService_widget *			m_pServicePage;					// Service widget page
	CGexFtpPage_widget *			m_pFtpPage;						// FTP page (settings + status)
	
protected slots:
   
	void OnMainSelectionChanged();
    void OnButtonStartStop();
    void OnStartSpooling();	
    void OnStopSpooling();
    void OnButtonHelp();
    void OnButtonClearLog();
	void OnSettingsChanged();
	void OnCloseWindow(void);
    void OnStateChanged_FtpSpoolingSettings();
    void DisplayFtpSettingsHelp();
	void OnStatusChanged(CGexFtpCore::Status eStatus, const QString& strArg1, const QString& strArg2);
	void OnLog(int nMsgType, const QString& strFtpServer, const QString& strMsg, const QString& strDateTime);
	void OnSpoolingTimer(bool bActive);
	void onStartDownload(const QString& strFtpServer);
	void onStopDownload();
	void onProfileChanging(bool bIsChangeApplied);

	void onSettingsLoaded(bool bResult, const QString& strSettingsFile);
	void onSettingsSaved(bool bResult, const QString& strSettingsFile);


signals:
   
	void sMainSelectionChanged();
};

#endif // CGexFtp_mainwindow_H
