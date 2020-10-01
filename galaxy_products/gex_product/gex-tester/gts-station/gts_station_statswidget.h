/****************************************************************************
** Deriven from gts_station_statswidget_base.h
****************************************************************************/

#ifndef gts_station_statswidget_H
#define gts_station_statswidget_H

#include "ui_gts_station_statswidget_base.h"
#include "gts_station_objects.h"

class GtsStationStatswidget : public QWidget, public Ui::GtsStationStatswidget_base
{
    Q_OBJECT

public:
    GtsStationStatswidget(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~GtsStationStatswidget();

	void Reset();
	void Init(GtsStation_SiteList & listSites);
	void UpdateGUI(GtsStation_SiteList & listSites);
};

#endif // gts_station_statswidget_H
