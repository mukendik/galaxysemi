/////////////////////////////////////////////////////////////////////////////////////
// Email sending object using smtp
/////////////////////////////////////////////////////////////////////////////////////

#ifndef CGEXEMAILSMTP_H
#define CGEXEMAILSMTP_H

#include "gex_email_send.h"

class CGexEmailSmtp : public CGexEmailSend
{
	Q_OBJECT

public:
    CGexEmailSmtp(QObject * pParent);
    virtual ~CGexEmailSmtp();

	uint			port() const						{ return m_uiPort; }
	const QString&	smtpServer() const					{ return m_strSmtpServer; }
	
	bool			SaveSettings(FILE* hSettingsFile);
	bool			LoadSettings(FILE* hSettingsFile);
	void			DefaultSettings(void);
	
	bool			Send(const QString& strFrom, const QStringList& strlistTo, const QString& strSubject, const QString& strBody, const QStringList& strlistAttachement, QString& strResponse, int nBodyFormat = GEXEMAIL_BODYFORMAT_TEXT);

signals:
    void			sEmailSent(bool bStatus);
	void			sSettingsChanged();

protected:
		
	uint			m_uiPort;
	QString			m_strSmtpServer;

public slots:
	void			setPort(const QString& strPort);
	void			setSmtpServer(const QString& strSmptServer);
};

#endif // CGEXEMAILSMTP_H
