#ifndef _Calendar_Dialog_h_
#define _Calendar_Dialog_h_

#include "ui_calendar_dialog.h"

#include <qdatetime.h> 

/////////////////////////////////////////////////////////////////////////////
class CalendarDialog : public QDialog, public Ui::CalendarDialogBase
{
	Q_OBJECT

public:
	CalendarDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
	
	void	setDate(const QDate &);
	QDate	getDate(void);
	void	setMaximumDate(const QDate &);
	void	setMinimumDate(const QDate &);

protected slots:

	void	OnDayClicked(const QDate& date);

	/*CalendarDialog( QWidget* parent = 0, bool modal = false, WindowFlags f = 0 );
	void	setDate(QDate &);
	QDate	getDate(void);
public slots:
	void	OnPreviousMonth(void);
	void	OnNextMonth(void);
	void	OnDayClicked(void);

private:
	QToolButton *pDays[42];		// Points to all button-days (6 lines, 7 columns-days)
	QDate	SelectedDate;
	QDate	BrowsedDate;
	void	updateCalendar(void);*/
};

#endif // _Calendar_Dialog_h_
