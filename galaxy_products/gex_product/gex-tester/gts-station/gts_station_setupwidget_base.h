/****************************************************************************
** Form interface generated from reading ui file '.\gts_station_setupwidget_base.ui'
**
** Created: Mon Feb 20 17:50:43 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GTSSTATIONSETUPWIDGET_BASE_H
#define GTSSTATIONSETUPWIDGET_BASE_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QButtonGroup;
class QCheckBox;
class QLabel;
class QSpinBox;

class GtsStationSetupwidget_base : public QWidget
{
    Q_OBJECT

public:
    GtsStationSetupwidget_base( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~GtsStationSetupwidget_base();

    QButtonGroup* groupDatalog;
    QCheckBox* checkBoxGenerateSTDFFile;
    QCheckBox* checkBoxStopOnFailMode;
    QButtonGroup* groupOutput;
    QCheckBox* checkBoxEnableOutput;
    QLabel* labelMaxOutputLines;
    QSpinBox* spinMaxOutputLines;

protected:
    QVBoxLayout* GtsStationSetupwidget_baseLayout;
    QSpacerItem* spacer3;
    QVBoxLayout* groupDatalogLayout;
    QVBoxLayout* groupOutputLayout;
    QHBoxLayout* layout4;
    QSpacerItem* spacer3_2;
    QSpacerItem* spacer2;

protected slots:
    virtual void languageChange();

};

#endif // GTSSTATIONSETUPWIDGET_BASE_H
