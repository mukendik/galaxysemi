/****************************************************************************
** Form interface generated from reading ui file '.\g-trigger_dialogbase.ui'
**
** Created: Fri Jan 23 14:16:29 2009
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef G_TRIGGERDIALOGBASE_H
#define G_TRIGGERDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QTabWidget;
class QWidget;
class QListBox;
class QListBoxItem;
class QTextBrowser;

class G_TriggerDialogBase : public QDialog
{
    Q_OBJECT

public:
    G_TriggerDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~G_TriggerDialogBase();

    QTabWidget* tabWidget;
    QWidget* tabStatus;
    QListBox* listBox_Status;
    QWidget* tabSettings;
    QListBox* listBox_Settings;
    QWidget* TabPage;
    QTextBrowser* textBrowser;

public slots:
    virtual void OnCloseConnection();
    virtual void OnConnect();
    virtual void OnTimerEvent();
    virtual void socketConnected();
    virtual void socketConnectionClosed();
    virtual void socketError(int);
    virtual void socketReadyRead();

protected:
    QVBoxLayout* G_TriggerDialogBaseLayout;
    QVBoxLayout* tabStatusLayout;
    QVBoxLayout* tabSettingsLayout;
    QVBoxLayout* TabPageLayout;

protected slots:
    virtual void languageChange();

};

#endif // G_TRIGGERDIALOGBASE_H
