/****************************************************************************
** Deriven from gts_station_gtlcommandsdialog_base.h
****************************************************************************/

#ifndef gts_station_gtlcommandsdialog_H
#define gts_station_gtlcommandsdialog_H

#include "ui_gts_station_gtlcommandsdialog_base.h"

#define GTS_STATION_GTLCOMMANDS_NONE			0
#define GTS_STATION_GTLCOMMANDS_DEBUGON			3
#define GTS_STATION_GTLCOMMANDS_DEBUGOFF		4
#define GTS_STATION_GTLCOMMANDS_QUIETON			5
#define GTS_STATION_GTLCOMMANDS_QUIETOFF		6
#define GTS_STATION_GTLCOMMANDS_INFO			11
#define GTS_STATION_GTLCOMMANDS_STATUS			12
#define GTS_STATION_GTLCOMMANDS_TEST			13
#define GTS_STATION_GTLCOMMANDS_TESTLIST		14
#define GTS_STATION_GTLCOMMANDS_HELP			15

//! \class Deriven from gts_station_gtlcommandsdialog_base.cpp
class GtsStation_GtlCommandsdialog : public QDialog, public Ui::GtsStation_GtlCommandsdialog_base
{
    Q_OBJECT

public:
    // Constructs a GtsStation_GtlCommandsdialog as a child of 'parent', with widget flags set to 'f'.
    GtsStation_GtlCommandsdialog(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~GtsStation_GtlCommandsdialog();

    //! \brief Virtual function overwritten to avoid closing the dialog box without sending close signal to main window
    //! \brief Override Close event, make sure signal is sent to main window
    void closeEvent(QCloseEvent *event);

signals:
    void sButtonSend(unsigned int uiCommand, long lTestNumber);
    void sButtonClose();

private:

protected slots:
    //! \brief 'Run' button clicked
    void OnButtonSend();
    //! \brief 'Close' button clicked
    void OnButtonClose();
};

#endif // gts_station_gtlcommandsdialog_H
