/****************************************************************************
** Form interface generated from reading ui file 'gts_station_mainwindow_base.ui'
**
** Created: Mon Feb 20 17:50:49 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GTSSTATIONMAINWINDOW_BASE_H
#define GTSSTATIONMAINWINDOW_BASE_H

#include <qvariant.h>
#include <qmainwindow.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QListBox;
class QListBoxItem;
class QWidgetStack;
class QWidget;
class QTextBrowser;
class QFrame;
class QPushButton;
class QSpinBox;
class QLabel;
class QLCDNumber;

class GtsStationMainwindow_base : public QMainWindow
{
    Q_OBJECT

public:
    GtsStationMainwindow_base( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~GtsStationMainwindow_base();

    QListBox* listMainSelection;
    QWidgetStack* widgetStack;
    QWidget* pageHelp;
    QTextBrowser* textBrowser_Help;
    QFrame* frameButtons;
    QPushButton* buttonLoadDelete;
    QPushButton* buttonRun;
    QPushButton* buttonRunN;
    QSpinBox* spinRunN;
    QPushButton* buttonGtlCommads;
    QPushButton* buttonReset;
    QPushButton* buttonDatalog;
    QPushButton* buttonNewlot;
    QPushButton* buttonHelp;
    QPushButton* buttonExit;
    QLabel* labelPartsTested;
    QLabel* textPartsTested;
    QLabel* labelBinning;
    QLCDNumber* lCDSite0Bin;
    QLCDNumber* lCDSite1Bin;
    QLCDNumber* lCDSite2Bin;
    QLCDNumber* lCDSite3Bin;
    QLCDNumber* lCDSite4Bin;
    QLCDNumber* lCDSite5Bin;
    QLCDNumber* lCDSite6Bin;
    QLCDNumber* lCDSite7Bin;

signals:
    void sCloseStation(void *);
    void sLoadStdfFile(const QString &);
    void sMainSelectionChanged();

protected:
    QVBoxLayout* GtsStationMainwindow_baseLayout;
    QHBoxLayout* layout5;
    QVBoxLayout* pageHelpLayout;
    QVBoxLayout* frameButtonsLayout;
    QSpacerItem* spacer8;
    QVBoxLayout* layout20;
    QHBoxLayout* layout19;
    QHBoxLayout* layout18;

protected slots:
    virtual void languageChange();

    virtual void OnMainSelectionChanged();
    virtual void OnButtonHelp();
    virtual void OnButtonLoadDelete();
    virtual void OnButtonRun();
    virtual void OnButtonReset();
    virtual void OnButtonDatalog();
    virtual void OnButtonExit();
    virtual void OnLoadStdfFile(const QString &);
    virtual void OnButtonRunN();
    virtual void OnButtonGtlCommands();
    virtual void OnGtlCommandsDialog_Close();
    virtual void OnGtlCommandsDialog_Run(unsigned int uiCommand, long lTestNumber, const QString & strTestName);
    virtual void OnButtonNewlot();


};

#endif // GTSSTATIONMAINWINDOW_BASE_H
