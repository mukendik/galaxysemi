/****************************************************************************
** Deriven from gts_station_setupwidget_base.h
****************************************************************************/

#ifndef gts_station_setupwidget_H
#define gts_station_setupwidget_H

#include <qcheckbox.h>

#include "ui_gts_station_setupwidget_base.h"

class GtsStationSetupwidget : public QWidget, public Ui::GtsStationSetupwidget_base
{
    Q_OBJECT

public:
    GtsStationSetupwidget(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~GtsStationSetupwidget();

	bool	IsStdfON()			{ return checkBoxGenerateSTDFFile->isChecked(); }
	bool	IsStopOnFailON()	{ return checkBoxStopOnFailMode->isChecked(); }
    bool	IsOutputEnabled()	{ return checkBoxEnableOutput->isChecked(); }
	int		GexMaxLines();
};

#endif // gts_station_setupwidget_H
