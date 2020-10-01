/****************************************************************************
** Form interface generated from reading ui file '.\gts_station_newlotdialog_base.ui'
**
** Created: Fri Sep 9 18:07:04 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GTSSTATION_NEWLOTDIALOG_BASE_H
#define GTSSTATION_NEWLOTDIALOG_BASE_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QLineEdit;
class QPushButton;

class GtsStation_Newlotdialog_base : public QDialog
{
    Q_OBJECT

public:
    GtsStation_Newlotdialog_base( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GtsStation_Newlotdialog_base();

    QLabel* labelLotID;
    QLineEdit* editSublotID;
    QLineEdit* editLotID;
    QLabel* labelSublotID;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;

protected:
    QVBoxLayout* GtsStation_Newlotdialog_baseLayout;
    QSpacerItem* spacer3;
    QGridLayout* layout11;
    QHBoxLayout* layout10;
    QSpacerItem* Horizontal_Spacing2;

protected slots:
    virtual void languageChange();

};

#endif // GTSSTATION_NEWLOTDIALOG_BASE_H
