/****************************************************************************
** Form interface generated from reading ui file '.\gts_station_outputwidget_base.ui'
**
** Created: Mon Feb 20 17:50:47 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GTSSTATIONOUTPUTWIDGET_BASE_H
#define GTSSTATIONOUTPUTWIDGET_BASE_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QTextEdit;
class QPushButton;

class GtsStationOutputwidget_base : public QWidget
{
    Q_OBJECT

public:
    GtsStationOutputwidget_base( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~GtsStationOutputwidget_base();

    QTextEdit* editOutput;
    QPushButton* buttonClear;

protected:
    QVBoxLayout* GtsStationOutputwidget_baseLayout;
    QHBoxLayout* layout5;
    QSpacerItem* spacer4;
    QSpacerItem* spacer5;

protected slots:
    virtual void languageChange();

};

#endif // GTSSTATIONOUTPUTWIDGET_BASE_H
