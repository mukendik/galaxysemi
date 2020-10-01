/////////////////////////////////////////////////////////////////////////////////////
// Email sending object wrapper
/////////////////////////////////////////////////////////////////////////////////////

#ifndef CGEXEMAILSEND_H
#define CGEXEMAILSEND_H

#include <QObject>

#include "gexemail_constants.h"

class CGexEmailSend : public QObject
{
    Q_OBJECT

public:
	
	enum sendType
	{
		sendType_smtp = 0,
		sendType_outlook
	};

    CGexEmailSend(sendType eType, QObject * pParent);
    ~CGexEmailSend();

	virtual bool		SaveSettings(FILE* hSettingsFile) = 0;
	virtual bool		LoadSettings(FILE* hSettingsFile) = 0;
	virtual void		DefaultSettings(void) = 0;

	sendType			type() const				{ return m_eType; }

private:

	sendType			m_eType;

signals:
    void				sEmailSent(bool bStatus);

public slots:

    virtual bool		Send(const QString& strFrom, const QStringList& strlistTo, const QString& strSubject, const QString& strBody, const QStringList& strlistAttachement, QString& strResponse, int nBodyFormat = GEXEMAIL_BODYFORMAT_TEXT) = 0;

};

#endif // CGEXEMAILSEND_H
