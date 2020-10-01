/****************************************************************************
** Deriven from gts_station_messagedialog_base.cpp
****************************************************************************/
#include <qtimer.h>
#include <qlabel.h>

#include "gts_station_messagedialog.h"


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a GtsStationMessagedialog as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
GtsStationMessagedialog::GtsStationMessagedialog(int nTimeout, QWidget* parent, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
  // Setup UI
  setupUi(this);

  QString strString;

	if(nTimeout < 0)
	{
    labelTimer->hide();
	}
	else if(nTimeout == 0)
	{
		accept();
	}
	else
	{
		m_nTimeout = nTimeout;
		strString.sprintf("This message will be automatically discarded in %d seconds...", nTimeout);
		labelTimer->setText(strString);
		QTimer::singleShot(1000, this, SLOT(OnTimer()));
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
GtsStationMessagedialog::~GtsStationMessagedialog()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// Timer function
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMessagedialog::OnTimer()
{
	QString strString;
	
	m_nTimeout--;
	if(m_nTimeout <= 0)
	{
		accept();
		return;
	}

	strString.sprintf("This message will be automatically discarded in %d seconds...", m_nTimeout);
	labelTimer->setText(strString);
	QTimer::singleShot(1000, this, SLOT(OnTimer()));
}
