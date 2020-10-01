/****************************************************************************
** Deriven from gts_station_infowidget_base.h
****************************************************************************/

#ifndef gts_station_infowidget_H
#define gts_station_infowidget_H

#include <stdfparse.h>

#include "gts_station_objects.h"
#include "ui_gts_station_infowidget_base.h"

//! \class Deriven from gts_station_infowidget_base.cpp
class GtsStationInfowidget : public QWidget, public Ui::GtsStationInfowidget_base
{
    Q_OBJECT

public:
    GtsStationInfowidget(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~GtsStationInfowidget();

    //! \brief Reset widget data
    void            Reset();
    void            Update(const QString & StdfFileName, const GS::QtLib::DatakeysContent & KeysContent);
    void            Update(const GtsStdfFiles & StdfFiles);
};

#endif // gts_station_infowidget_H
