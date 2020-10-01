/****************************************************************************
** Form interface generated from reading ui file '.\gts_station_gtlcommandsdialog_base.ui'
**
** Created: Fri Sep 9 18:07:13 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GTSSTATION_GTLCOMMANDSDIALOG_BASE_H
#define GTSSTATION_GTLCOMMANDSDIALOG_BASE_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QButtonGroup;
class QRadioButton;
class QSpinBox;
class QPushButton;

class GtsStation_GtlCommandsdialog_base : public QDialog
{
    Q_OBJECT

public:
    GtsStation_GtlCommandsdialog_base( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GtsStation_GtlCommandsdialog_base();

    QButtonGroup* groupCommands;
    QRadioButton* buttonTraceON;
    QRadioButton* buttonTraceOFF;
    QRadioButton* buttonDebugON;
    QRadioButton* buttonDebugOFF;
    QRadioButton* buttonDisabled;
    QRadioButton* buttonEnabled;
    QRadioButton* buttonQuietON;
    QRadioButton* buttonQuietOFF;
    QRadioButton* buttonErrON;
    QRadioButton* buttonErrOFF;
    QRadioButton* buttonScreenON;
    QRadioButton* buttonScreenOFF;
    QRadioButton* buttonInfo;
    QRadioButton* buttonStatus;
    QRadioButton* buttonReset;
    QRadioButton* buttonTest;
    QSpinBox* spinTestNumber;
    QRadioButton* buttonTestlist;
    QRadioButton* buttonHelp;
    QRadioButton* buttonNone;
    QPushButton* buttonRun;
    QPushButton* buttonCancel;

signals:
    void sButtonRun(unsigned int uiCommand, long lTestNumber, const QString & strTestName);
    void sButtonClose();

protected:
    QVBoxLayout* GtsStation_GtlCommandsdialog_baseLayout;
    QVBoxLayout* groupCommandsLayout;
    QHBoxLayout* layout2;
    QHBoxLayout* layout6;

protected slots:
    virtual void languageChange();

    virtual void OnButtonRun();
    virtual void OnButtonClose();


};

#endif // GTSSTATION_GTLCOMMANDSDIALOG_BASE_H
