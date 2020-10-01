/****************************************************************************
** Form interface generated from reading ui file '.\gts_station_statswidget_base.ui'
**
** Created: Fri Sep 9 18:07:00 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GTSSTATIONSTATSWIDGET_BASE_H
#define GTSSTATIONSTATSWIDGET_BASE_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QListView;
class QListViewItem;

class GtsStationStatswidget_base : public QWidget
{
    Q_OBJECT

public:
    GtsStationStatswidget_base( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~GtsStationStatswidget_base();

    QLabel* labelYield;
    QListView* listViewYield;
    QLabel* labelBinning;
    QListView* listViewBinning;

protected:
    QVBoxLayout* GtsStationStatswidget_baseLayout;
    QVBoxLayout* layout13;
    QVBoxLayout* layout12;

protected slots:
    virtual void languageChange();

};

#endif // GTSSTATIONSTATSWIDGET_BASE_H
