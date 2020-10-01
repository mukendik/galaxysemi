#include <QCloseEvent>
#include <QThread>
#include <QString>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Local includes
#include "g-trigger_dialog.h"

namespace GS
{
namespace GTrigger
{
///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GTriggerDialog::GTriggerDialog(const QString & AppName, const QString & AppPath, QWidget *parent, Qt::WindowFlags fl)
    : QDialog(parent, fl), mAppName(AppName), mAppPath(AppPath)
{
    // Setup UI
    setupUi(this);

    // Updates Windows title to current software version string
    setWindowTitle(mAppName);

    // Startup status
    InsertStatusLog(QString("Starting %1 GUI thread (%2)").arg(mAppName).arg((intptr_t)QThread::currentThreadId()));
    InsertStatusLog(QString("Application path is %1").arg(mAppPath));

    // Load release history into HTML browser
    textBrowser->setSource(QUrl(QString("qrc:/g-trigger/gtrigger-rh")));

    // Show dialog
    show();
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
GTriggerDialog::~GTriggerDialog()
{
    // Startup status
    InsertStatusLog(QString("Stopping %1 GUI").arg(mAppName));

    // Clear list views
    while(listBox_Settings->count() > 0)
        delete listBox_Settings->takeItem(0);
    while(listBox_Status->count() > 0)
        delete listBox_Status->takeItem(0);
}

void GTriggerDialog::closeEvent (QCloseEvent * e)
{
    InsertStatusLog(QString("Close requested. Waiting for engine to stop..."));
    emit sAboutToClose();
    e->ignore();
}

///////////////////////////////////////////////////////////
// Insert line into status log.
///////////////////////////////////////////////////////////
void GTriggerDialog::InsertStatusLog(const QString & Line)
{
    // Keep only 100 lines
    if(listBox_Status->count() >= 100)
        delete listBox_Status->takeItem(0);
    listBox_Status->addItem(Line);
#if 0 // QT3->QT4: check if some code required to select new item, and make sure it is visible
    listBox_Status->setCurrentItem(listBox_Status->count()-1);
    listBox_Status->ensureCurrentVisible();
#endif

    //Not needed anymore, Engine is running in a thread
    //qApplication->processEvents(QEventLoop::AllEvents, 400);
}

///////////////////////////////////////////////////////////
// Insert line into settings log.
///////////////////////////////////////////////////////////
void GTriggerDialog::InsertSettingsLog(const QString & Line)
{
    listBox_Settings->addItem(Line);
}

// SLOTS
void GTriggerDialog::OnInsertStatusLog(const QString & Line)
{
    InsertStatusLog(Line);
}

void GTriggerDialog::OnInsertSettingsLog(const QString & Line)
{
    InsertSettingsLog(Line);
}

void GTriggerDialog::Close()
{
    QApplication::exit();
}

} // namespace GS
} // namespace GTrigger
