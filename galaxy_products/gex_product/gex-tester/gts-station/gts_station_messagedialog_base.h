/****************************************************************************
** Form interface generated from reading ui file '.\gts_station_messagedialog_base.ui'
**
** Created: Fri Sep 9 18:07:06 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GTSSTATIONMESSAGEDIALOG_BASE_H
#define GTSSTATIONMESSAGEDIALOG_BASE_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;

class GtsStationMessagedialog_base : public QDialog
{
    Q_OBJECT

public:
    GtsStationMessagedialog_base( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GtsStationMessagedialog_base();

    QLabel* labelMessage;
    QLabel* labelTimer;
    QPushButton* buttonOK;

protected:
    QVBoxLayout* GtsStationMessagedialog_baseLayout;
    QHBoxLayout* layout13;
    QSpacerItem* spacer4;
    QSpacerItem* spacer5;

protected slots:
    virtual void languageChange();

    virtual void OnTimer();


};

#endif // GTSSTATIONMESSAGEDIALOG_BASE_H
