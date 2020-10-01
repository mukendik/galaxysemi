#include <gqtl_log.h>
#include <QRadioButton>
#include <QSpinBox>
#include <QCloseEvent>

#include "gts_station_gtlcommandsdialog.h"


GtsStation_GtlCommandsdialog::GtsStation_GtlCommandsdialog(QWidget* parent, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
  // Setup UI
  setupUi(this);

  // Signal/Slot connections
  if (!connect( buttonSend, SIGNAL(clicked()),this, SLOT(OnButtonSend())))
      GSLOG(3, "Failed to connect send button signal to slot");
  if (!connect( buttonCancel, SIGNAL(clicked()),this, SLOT(OnButtonClose())))
      GSLOG(3, "Failed to connect cancel button signal and slot");
  if (!connect( buttonTest, SIGNAL(toggled(bool)),spinTestNumber, SLOT(setEnabled(bool))))
      GSLOG(3, "Failed to connect test button signal to slot");
}

GtsStation_GtlCommandsdialog::~GtsStation_GtlCommandsdialog()
{
}

void GtsStation_GtlCommandsdialog::OnButtonSend()
{
    if(buttonNone->isChecked())
        emit sButtonSend(GTS_STATION_GTLCOMMANDS_NONE, 0);
	if(buttonDebugON->isChecked())
        emit sButtonSend(GTS_STATION_GTLCOMMANDS_DEBUGON, 0);
	if(buttonDebugOFF->isChecked())
        emit sButtonSend(GTS_STATION_GTLCOMMANDS_DEBUGOFF, 0);
	if(buttonQuietON->isChecked())
        emit sButtonSend(GTS_STATION_GTLCOMMANDS_QUIETON, 0);
	if(buttonQuietOFF->isChecked())
        emit sButtonSend(GTS_STATION_GTLCOMMANDS_QUIETOFF, 0);
	if(buttonStatus->isChecked())
        emit sButtonSend(GTS_STATION_GTLCOMMANDS_STATUS, 0);
	if(buttonInfo->isChecked())
        emit sButtonSend(GTS_STATION_GTLCOMMANDS_INFO, 0);
	if(buttonTest->isChecked())
        emit sButtonSend(GTS_STATION_GTLCOMMANDS_TEST, spinTestNumber->value());
	if(buttonTestlist->isChecked())
        emit sButtonSend(GTS_STATION_GTLCOMMANDS_TESTLIST, 0);
	if(buttonHelp->isChecked())
        emit sButtonSend(GTS_STATION_GTLCOMMANDS_HELP, 0);
}

void GtsStation_GtlCommandsdialog::OnButtonClose()
{
	emit sButtonClose();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void GtsStation_GtlCommandsdialog::closeEvent(QCloseEvent *event)
{
    // Close properly, as if 'Close' button clicked
    OnButtonClose();
    event->accept();
}
