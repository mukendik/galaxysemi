/****************************************************************************
** Form interface generated from reading ui file 'gts_mainwindow_base.ui'
**
** Created: Mon Feb 20 17:52:14 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GTS_MAINWINDOW_BASE_H
#define GTS_MAINWINDOW_BASE_H

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
class QPushButton;
class QListView;
class QListViewItem;

class Gts_MainWindow_base : public QMainWindow
{
    Q_OBJECT

public:
    Gts_MainWindow_base( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~Gts_MainWindow_base();

    QPushButton* buttonNewStation;
    QPushButton* buttonExit;
    QListView* listViewStations;
    QMenuBar *MenuBar;
    QPopupMenu *File;
    QPopupMenu *Help;
    QAction* NewStationAction;
    QAction* ExitAction;
    QAction* AboutAction;

protected:
    QVBoxLayout* Gts_MainWindow_baseLayout;
    QHBoxLayout* layout41;
    QSpacerItem* spacer1;

protected slots:
    virtual void languageChange();

    virtual void OnExit();
    virtual void OnNewStation();
    virtual void OnAbout();
    virtual void OnCloseStation();


};

#endif // GTS_MAINWINDOW_BASE_H
