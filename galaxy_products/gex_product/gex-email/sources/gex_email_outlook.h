/////////////////////////////////////////////////////////////////////////////////////
// Email sending object using Outlook
/////////////////////////////////////////////////////////////////////////////////////

#ifndef CGEXEMAILOUTLOOK_H
#define CGEXEMAILOUTLOOK_H

#include "gex_email_send.h"

class CGexEmailOutlook : public CGexEmailSend
{
public:
    CGexEmailOutlook(QObject * pParent);
    ~CGexEmailOutlook();

	bool			SaveSettings(FILE* hSettingsFile);
	bool			LoadSettings(FILE* hSettingsFile);
	void			DefaultSettings(void);
	
signals:
    void			sEmailSent(bool bStatus);

public slots:
    bool			Send(const QString& strFrom, const QStringList& strlistTo, const QString& strSubject, const QString& strBody, const QStringList& strlistAttachement, QString& strResponse, int nBodyFormat = GEXEMAIL_BODYFORMAT_TEXT);

};

#endif // CGEXEMAILOUTLOOK_H
