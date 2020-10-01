#ifndef CGEXEMAILMAINWINDOW_H
#define CGEXEMAILMAINWINDOW_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-EMAIL Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_email_mainbase.h"
#include "gexemail_constants.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QDateTime>
#include <QMainWindow>

///////////////////////////////////////////////////////////////////////////////////
// GUI Includes
///////////////////////////////////////////////////////////////////////////////////
#include "ui_gex_email_mainwindow_base.h"

class CGexEmailSend;
class CGexEmailSmtp;
class CGexEmailOutlook;
class CGexEmailSend;
class QWidget;

typedef struct t_GexEmail_Page
{
	QWidget*	pPage;
	bool		bDisableWhenRunning;
} GexEmail_Page;


///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CGexEmailMainwindow
//
// Description	:	Class used for gui mode
//
///////////////////////////////////////////////////////////////////////////////////
class CGexEmailMainwindow : public QMainWindow, public Ui::CGexEmailMainwindow_base, public CGexEmailMainBase
{
    Q_OBJECT

public:

    CGexEmailMainwindow(QApplication* qApplication, const QString & strUserHome, const QString & strApplicationDir,
                        const QString & strLocalFolder, const QString & strApplicationName, QWidget* parent = 0,
                        Qt::WindowFlags fl = 0);
    virtual ~CGexEmailMainwindow();

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////
	
	bool	init(bool bStartEmailSpoolerAtLaunch, bool bAllowUserToSelectEmailSpoolingDir, bool bRunAsService, bool *pbExitApplication); // Initialization
	void	pause(void);					// Suspends the execution
	void	resume(void);					// Resumes the execution
	void	stop();							// Stops the execution

protected:

	void	initGui();						// Initialize gui
	void	ResizeApplication(void);		// Resizes the application
    void	AddPage(const QString& strSelectionText, const QString& strSelectionPixmap, QWidget* pPage, bool bDisableWhenRunning);	// Add a page to the stacked widget
	
// Overrides
protected:

	void	closeEvent(QCloseEvent * e);	// Called when close event is raised
	void	customEvent(QEvent * e);		// Called when custom events are raised

protected:

	QList<GexEmail_Page>		m_listPages;					// List of pages used for mail system definition
	QString						m_strImagesDirectory;			// Folder for images
	QString						m_strHtmlSource;				// File path for html page
	QPixmap						m_pxInfo;						// Pixmap for information messages
	QPixmap						m_pxInfoSent;					// Pixmap for 'email sent' messages
	QPixmap						m_pxError;						// Pixmap for error messages
	QPixmap						m_pxWarning;					// Pixmap for warning messages
	unsigned int				m_uiPage_Help;					// Index of the help page
	unsigned int				m_uiPage_General;				// Index of the general page
	unsigned int				m_uiPage_Smtp;					// Index of the smtp page
	unsigned int				m_uiPage_Log;					// Index of the log page

	bool						m_bServicePaused;				// Indicates if the service is in pause
	bool						m_bStartEnabled;				// Indicates if the user can start the process

	QApplication *				m_qApplication;					// QT Application

signals:
    void sEmailDirectoryChanged(const QString &);
    void sMainSelectionChanged(int);
    void sSettingsChanged();

protected slots:
	void OnStatusChanged(CGexEmailCore::Status eStatus, const QString& strArg1, const QString& strArg2);
    void OnMainSelectionChanged(int nCurrentRow);
    void OnButtonStartStop();
    void OnStartSpooling();
    void OnStopSpooling();
	void OnSettingsChanged();
    void OnButtonSave();
    void OnButtonHelp();
    void OnButtonClearLog();
    void OnCloseWindow();
    void OnButtonBrowseEmailDirectory();
	void OnLog(int nMsgType, const QString& strMailFile, const QString& strMsg, const QString& strDateTime);
	void OnSpoolingTimer(bool bActive);
	void onCriticalMessage(const QString& strMessage);
};

#endif // CGEXEMAILMAINWINDOW_H
