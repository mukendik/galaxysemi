/****************************************************************************
** Deriven from gts_station_outputwidget_base.h
****************************************************************************/

#ifndef gts_station_outputwidget_H
#define gts_station_outputwidget_H

#include "ui_gts_station_outputwidget_base.h"
#include "gts_station_objects.h"

class GtsStationSetupwidget;

class GtsStationOutputwidget : public QWidget, public Ui::GtsStationOutputwidget_base
{
    Q_OBJECT

public:
    GtsStationOutputwidget(GtsStationSetupwidget *pageSetup, QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~GtsStationOutputwidget();

	void Reset();
    void Command(const QString & lCommand);
    void Printf(const QString & lMessage);
	void UpdateGUI(GtsStation_SiteList & listSites);

protected:
    void Append(const QString & lMessage);
	
	GtsStationSetupwidget	*m_pageSetup;
};

#endif // gts_station_outputwidget_H
