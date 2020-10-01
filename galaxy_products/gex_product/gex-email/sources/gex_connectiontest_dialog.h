/****************************************************************************
** Deriven from gex_connectiontest_dialog_base.h
****************************************************************************/

#ifndef CGEXCONNECTIONTESTDIALOG_H
#define CGEXCONNECTIONTESTDIALOG_H

#include <QDialog>
#include <QAbstractSocket>

#include "ui_gex_connectiontest_dialog_base.h"

class QTcpSocket;
class QTextStream;

class CGexConnectionTestDialog : public QDialog, public Ui::CGexConnectionTestDialog_base
{
    Q_OBJECT

public:
    CGexConnectionTestDialog(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~CGexConnectionTestDialog();

	void	Init(const QString & strSmtpServer, uint uiSmtpPort);

protected:
	void	CloseSocket(void);

protected:
	enum eStatus
	{
		eStatusSearchingHost,
		eStatusConnectingToHost,
		eStatusConnectionSuccessful,
		eStatusConnectionFailed
	};

	QString			m_strSmtpServer;
	uint			m_uiSmtpPort;
	QTcpSocket*		m_pSocket;
    QTextStream*	m_textStream;
	bool			m_bUsingServerName;

protected slots:
    void socketError(QAbstractSocket::SocketError Error);
    void socketHostFound();
    void socketConnected();
    void socketReadyRead();
    void StartTest();
    void StopTest();
};

#endif // CGEXCONNECTIONTESTDIALOG_H
