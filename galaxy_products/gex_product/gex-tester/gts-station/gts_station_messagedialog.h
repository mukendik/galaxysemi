/****************************************************************************
** Deriven from gts_station_messagedialog_base.h
****************************************************************************/

#ifndef gts_station_messagedialog_H
#define gts_station_messagedialog_H

#include "ui_gts_station_messagedialog_base.h"

class GtsStationMessagedialog : public QDialog, public Ui::GtsStationMessagedialog_base
{
    Q_OBJECT

public:
    GtsStationMessagedialog(int nTimeout = -1, QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~GtsStationMessagedialog();

private:
	int m_nTimeout;

protected slots:
	void OnTimer();
};

#endif // gts_station_messagedialog_H
