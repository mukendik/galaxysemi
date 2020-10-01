/****************************************************************************
** Form interface generated from reading ui file '.\gts_station_infowidget_base.ui'
**
** Created: Fri Sep 9 18:07:11 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GTSSTATIONINFOWIDGET_BASE_H
#define GTSSTATIONINFOWIDGET_BASE_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QLineEdit;
class QFrame;

class GtsStationInfowidget_base : public QWidget
{
    Q_OBJECT

public:
    GtsStationInfowidget_base( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~GtsStationInfowidget_base();

    QLabel* labelLoadedStdfFile;
    QLineEdit* editLoadedStdfFile;
    QFrame* line1;
    QLabel* labelProgramName;
    QLineEdit* editProgramName;
    QLabel* labelLotID;
    QLineEdit* editLotID;
    QLabel* labelSublotID;
    QLineEdit* editSublotID;
    QLabel* labelWaferID;
    QLineEdit* editWaferID;

protected:
    QVBoxLayout* GtsStationInfowidget_baseLayout;
    QSpacerItem* spacer5;
    QHBoxLayout* layout12;
    QHBoxLayout* layout26;
    QHBoxLayout* layout23;
    QHBoxLayout* layout24;
    QHBoxLayout* layout25;

protected slots:
    virtual void languageChange();

};

#endif // GTSSTATIONINFOWIDGET_BASE_H
