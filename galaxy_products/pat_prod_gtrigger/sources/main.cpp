///////////////////////////////////////////////////////////
// G-Trigger application
///////////////////////////////////////////////////////////
#include <QApplication>
#include <QThread>

#include "g-trigger_dialog.h"
#include "g-trigger_engine.h"

#include <gqtl_sysutils.h>

#define G_TRIGGER_APP	"G-Trigger V3.12"

///////////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
    // Create application object
    QApplication	lApp(argc, argv);
    lApp.setOrganizationName(QString("GalaxySemi"));
    lApp.setWindowIcon(QIcon(QString(":/g-trigger/gtrigger-icon")));

    // Get application path
    QString   lAppPath;
    CGexSystemUtils::GetApplicationDirectory(lAppPath);

    // Create GUI & Engine
    GS::GTrigger::GTriggerDialog lGTDialog(QString(G_TRIGGER_APP), lAppPath);
    GS::GTrigger::GTriggerEngine* lGTEngine = new GS::GTrigger::GTriggerEngine(QString(G_TRIGGER_APP), lAppPath);

    // Move Engine into a separate thread
    QThread* lEngineThread = new QThread();
    lGTEngine->moveToThread(lEngineThread);

    // Connect signals/slots
    QObject::connect(lGTEngine, SIGNAL(sStatusLog(const QString &)), &lGTDialog, SLOT(OnInsertStatusLog(const QString &)));
    QObject::connect(lGTEngine, SIGNAL(sSettingsLog(const QString &)), &lGTDialog, SLOT(OnInsertSettingsLog(const QString &)));
    QObject::connect(lEngineThread,  SIGNAL(started()),  lGTEngine, SLOT(OnStart()));
    QObject::connect(&lGTDialog, SIGNAL(sAboutToClose()), lEngineThread, SLOT(quit()));
    QObject::connect(lEngineThread,  SIGNAL(finished()),  lGTEngine, SLOT(deleteLater()));
    QObject::connect(lEngineThread,  SIGNAL(finished()),  &lGTDialog, SLOT(Close()));

    // QA
    if (argc > 1 && ::strcmp(argv[1], "--qa") == 0)
    {
        lGTEngine->setQA(true);
    }

    // Start Engine thread
    lEngineThread->start();

    // Execute application loop
    int lStatus = lApp.exec();

    // Delete Engine thread
    delete lEngineThread;

    // Done!
    return lStatus;
}

