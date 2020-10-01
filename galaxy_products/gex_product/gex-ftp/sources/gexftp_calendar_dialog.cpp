#include <QLabel>

#include "gexftp_calendar_dialog.h"


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexFtpCalendarDialog::CGexFtpCalendarDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	// Setup UI
	setupUi(this);

	connect(calendarWidget, SIGNAL(clicked(const QDate&)), this, SLOT(OnDayClicked(const QDate&)));
}

///////////////////////////////////////////////////////////
// Set Calendar to a given date
///////////////////////////////////////////////////////////
void CGexFtpCalendarDialog::setDate(const QDate &cDate)
{
	// Sets calendar to given date.
	calendarWidget->setSelectedDate(cDate);
}

///////////////////////////////////////////////////////////
// Get Calendar date currently selected
///////////////////////////////////////////////////////////
QDate CGexFtpCalendarDialog::getDate(void)
{
	return calendarWidget->selectedDate();
}

///////////////////////////////////////////////////////////
// Get Calendar date currently selected
///////////////////////////////////////////////////////////
void CGexFtpCalendarDialog::OnDayClicked(const QDate& /*date*/)
{
	// A day has been selected, we close the dialog
	done(1);
}

