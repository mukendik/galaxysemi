#ifndef GEX_LICENSE_STATUS_H
#define GEX_LICENSE_STATUS_H

#include <QtGui>
#include <QAbstractSocket>

#include "gqtl_skin.h"

#include "ui_licensestatus_dialogbase.h"

class QTcpSocket;

class LicenseStatusDialog : public QDialog, public Ui::LicenseStatusDialogBase
{
    Q_OBJECT

public:
	LicenseStatusDialog(const QString & strUserHome, const QString & strAppName, const QString & strOutputStatusFile, QWidget* parent = 0, Qt::WFlags f = 0);
	void	dropEvent(QDropEvent *);
	void	dragEnterEvent(QDragEnterEvent *event);

private:
	QString					m_strAppName;
	QString					m_strOutputStatusFile;
	QString					m_strUserHome;
	QString					m_strDefaultReport;
	QString					m_strConfigFile;
	QTcpSocket			*	socket;						// Used if running in Client/Server mode.
	int						iSocketStatus;				// Keeps track of Client/server pending events.
	QTimer					timerSocket;				// Timer for checking if socket actions completed.
	QDateTime				dtSocketEvent;				
	QString					m_strSocketPort;			// Socket Port# used (string)
	QString					m_strServerName;			// Server name used
	CGexSkin				m_gexSkin;					// Skin style
	bool					m_bIdle;					// =true if connection is idle.
	QFile					m_fScriptFile;
	QTextStream				m_hScriptFile;				// Script file handle.

	void		UpdateGuiButtons(bool bConnected);
	QString		ReadLineFromSocket(void);
	void		StartRunScript(QString strScriptFile);

protected slots:
	void		on_buttonConnect_clicked();		// Implicit signal/slot connection
	void		on_buttonDisconnect_clicked();	// Implicit signal/slot connection
	void		on_buttonSendCMD_clicked();		// Implicit signal/slot connection
	void		on_buttonClearServer_clicked();
	void		on_buttonRunScript_clicked();	// Select script + Run it.
	void		OnCloseConnection();
	void		OnTimerEvent();

	// Slots used for socket communication
	void		socketConnected();
	void		socketConnectionClosed();
	void		socketReadyRead();
	void		socketError(QAbstractSocket::SocketError);
};

#endif
