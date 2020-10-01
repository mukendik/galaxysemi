#ifndef GEX_LICENSE_STATUS_H
#define GEX_LICENSE_STATUS_H

#include <QtGui>
#include <QAbstractSocket>

#include "gqtl_skin.h"

#include "ui_licensestatus_dialogbase.h"
#include "historyeventmanager.h"

class QTcpSocket;

class LicenseStatusDialog : public QDialog, public Ui::LicenseStatusDialogBase
{
    Q_OBJECT

public:
        LicenseStatusDialog(const QString & strUserHome, const QString & strAppName, const QString & strOutputStatusFile, QWidget* parent = 0);

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
        CStatisticManager                       m_statisticManager;			// Manages the data computed for statistic
	CHistoryEventManager	m_EventManager;				// Manages the event history received from GEX-LM
	CGexSkin				m_gexSkin;					// Skin style

	void		UpdateGuiButtons(bool bConnected);
	void		UpdateProcessBar(QString strMessage,int iTotalSteps,int iStep);
	void		RequestLicenseManagerStatus();
	QString		ReadLineFromSocket(void);

	void		createReportDir();

protected slots:
	void		on_buttonConnect_clicked();		// Implicit signal/slot connection
	void		on_buttonDisconnect_clicked();	// Implicit signal/slot connection
	void		OnCloseConnection();
	void		OnTimerEvent();
	void		onSourceChanged(const QUrl& urlName);

	// Slots used for socket communication
	void		socketConnected();
	void		socketDisconnected();
	void		socketReadyRead();
	void		socketError(QAbstractSocket::SocketError);

	// Slots used for history request
	void		RequestLicenseManagerHistory();
};

#endif
