/****************************************************************************
** Deriven from gexftp_service_widget_base.h
****************************************************************************/

#ifndef CGexFtpService_widget_H
#define CGexFtpService_widget_H

#include <gqtl_service.h>

#include "ui_gexftp_service_widget_base.h"

class QPixmap;
class CGexFtpSettings;

class CGexFtpService_widget : public QWidget, public Ui::CGexFtpService_widget_base
{

	Q_OBJECT

public:
    CGexFtpService_widget( const QPixmap *ppxInfo, const QPixmap *ppxInfoTransfer, const QPixmap *ppxError, const QPixmap *ppxWarning, QtServiceController * pServiceController, CGexFtpSettings * pDataSettings, QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~CGexFtpService_widget();

protected:
	const QPixmap *			m_ppxInfo;						// Pixmap for information messages
	const QPixmap *			m_ppxInfoTransfer;				// Pixmap for 'file transferred' information messages
	const QPixmap *			m_ppxError;						// Pixmap for error messages
	const QPixmap *			m_ppxWarning;					// Pixmap for warning messages
	QString					m_strServiceHomeDir;
	QString					m_strSettingsFile;
	QString					m_strServiceLogFile;
	unsigned int			m_uiServiceLogFileLinesRead;
	unsigned int			m_uiServiceLogFileSize;
	bool					m_bSettingsOK;
	QtServiceController *	m_pServiceController;
	CGexFtpSettings *		m_pDataSettings;

protected slots:
    void		UpdateGUI();
    void		OnBrowseServiceHomeDir(void);
    void		OnButtonStartService(void);
    void		OnButtonStopService(void);
	void		OnSettingsLoaded(bool bOK);
};

#endif // CGexFtpService_widget_H
