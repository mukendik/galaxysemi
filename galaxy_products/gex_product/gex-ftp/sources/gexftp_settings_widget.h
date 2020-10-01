/****************************************************************************
** Deriven from gexftp_settings_widget_base.h
****************************************************************************/

#ifndef CGexFtpSettings_widget_H
#define CGexFtpSettings_widget_H

#include "gexftp_server.h"

#include <QList>

#include "ui_gexftp_settings_widget_base.h"

class CGexFtpSettings;

class CGexFtpSettings_widget : public QWidget, public Ui::CGexFtpSettings_widget_base
{

	Q_OBJECT

public:
    CGexFtpSettings_widget(CGexFtpSettings * pDataSettings, QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~CGexFtpSettings_widget();

// Overrides
protected:
	void		keyPressEvent(QKeyEvent * e);

// Member functions
protected:
	void		SetLocalDirectory(const QString& strLocalDirectory);
	void		ClearSettingsForm();
	void		AddToComboURL(const QString& strURL);
	void		AddServerToList(const CGexFtpServer& clFtpServer, bool bFillSettingsForm);
	bool		ValidateServer(const CGexFtpServer& clFtpServer, bool bDisplayMsg);
	bool		ConfirmPassword(const QString & strPassword);
	void		updateProfileButton();

protected:

	CGexFtpSettings *		m_pDataSettings;

protected slots:
    void	OnButtonBrowseLocalFolder();
	void	onButtonBrowseRemoteFolder();
	void	onButtonTestRemoteFolder();
    bool	OnButtonApply();
    void	OnNewProfileSelected(const QString & strSettingsName);
    void	OnButtonDelete();
    void	OnButtonHelp();
    void	OnButtonPrevProfile();
    void	OnButtonNextProfile();
    void	OnProfileModified();
    void	OnButtonFromCalendar();
    void	OnButtonToCalendar();
    void	OnFromDateChanged();
    void	OnToDateChanged();

signals:
    void sLocalDirectoryChanged(const QString &);
    void sSettingsChanged();
    void sDisplayHelp();
};

#endif // CGexFtpSettings_widget_H
