#include "gex_email_sendwidget.h"
#include "gex_connectiontest_dialog.h"
#include "gex_email_smtp.h"


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexEmailSendWidget::CGexEmailSendWidget(QWidget* parent, Qt::WindowFlags fl) : QWidget(parent, fl)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexEmailSendWidget::~CGexEmailSendWidget()
{
}

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexEmailSendWidgetSmtp::CGexEmailSendWidgetSmtp(CGexEmailSmtp * pSmtp, QWidget* parent, Qt::WindowFlags fl) : CGexEmailSendWidget(parent, fl)
{
	m_strName	= "Smtp";
	m_strIcon	= "gexemail_smtp.png";
	m_pSmtp		= pSmtp;

	// Setup UI
	setupUi(this);

	// Set some validators
    m_pSmtpPortValidator = new QIntValidator( 0, 9999, this );
	editSmtpPort->setValidator(m_pSmtpPortValidator);

   	editSmtpPort->setText(QString::number(m_pSmtp->port()));
	editSmtpServer->setText(m_pSmtp->smtpServer());

	// Signal/Slot connections
	connect(editSmtpPort,	SIGNAL(textChanged(const QString&)), m_pSmtp, SLOT(setPort(const QString&)));
    connect(editSmtpServer, SIGNAL(textChanged(const QString&)), m_pSmtp, SLOT(setSmtpServer(const QString&)));
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexEmailSendWidgetSmtp::~CGexEmailSendWidgetSmtp()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
// Test button pressed
/////////////////////////////////////////////////////////////////////////////////////
void CGexEmailSendWidgetSmtp::on_buttonTest_clicked()
{
	QString	strSmtpServer	= editSmtpServer->text();
	uint	uiSmtpPort		= editSmtpPort->text().toUInt();
	
	CGexConnectionTestDialog clConnectionTest(this);

	clConnectionTest.Init(strSmtpServer, uiSmtpPort);
	clConnectionTest.exec();
}
