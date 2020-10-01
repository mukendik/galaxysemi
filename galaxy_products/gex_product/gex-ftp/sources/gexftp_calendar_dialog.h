#ifndef GEX_FTP_CALENDAR_DIALOG_H
#define GEX_FTP_CALENDAR_DIALOG_H

#include <QDateTime> 
#include <QToolButton> 

#include "ui_gexftp_calendar_dialogbase.h"

/////////////////////////////////////////////////////////////////////////////
class CGexFtpCalendarDialog : public QDialog, public Ui::CalendarDialogBase
{

	Q_OBJECT

public:
	CGexFtpCalendarDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
	
	void	setDate(const QDate &);
	QDate	getDate(void);

protected slots:

	void	OnDayClicked(const QDate& date);
};

#endif // GEX_FTP_CALENDAR_DIALOG_H
