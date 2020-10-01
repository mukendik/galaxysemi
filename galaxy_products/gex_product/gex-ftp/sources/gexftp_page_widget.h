/****************************************************************************
** Deriven from gexftp_page_widget_base.h
****************************************************************************/

#ifndef CGexFtpPage_widget_H
#define CGexFtpPage_widget_H

#include "gexftp_settings_widget.h"

#include <QList>

#include "ui_gexftp_page_widget_base.h"

class CGexFtpServerTransfer_widget;
class CGexFtpSettings;
class CGexFtpDownload;

class CGexFtpPage_widget : public QWidget, public Ui::CGexFtpPage_widget_base
{

	Q_OBJECT

public:

    CGexFtpPage_widget(CGexFtpSettings * pDataSettings, QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~CGexFtpPage_widget();

	void								EnableSettingsWidget(bool bEnable);
	QWidget*							GetSettingsWidget(void) { return m_pFtpSettingsWidget; }
	void								StartTransfer(QApplication* qApplication, const QString & strApplicationPath, const CGexFtpDownload * pDownload, const QString& strSettingsName);
	void								CloseTransfers();

public slots:
    void								setPageEnabled(bool bEnable);

protected:
	CGexFtpSettings_widget *			m_pFtpSettingsWidget;		// FTP settings widget
	CGexFtpServerTransfer_widget *		m_pDownloadWidget;			// FTP transfer status widget
};

#endif // CGexFtpPage_widget_H
