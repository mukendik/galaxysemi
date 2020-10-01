#include "mainwindow.h"
#include "mycom.h"
#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Main window
    MainWindow w;

    // Worker
    MyCOM* lCom = new MyCOM();

    // Move COM worker into a separate thread
#ifdef USE_THREAD
    QThread lComThread;
    lCom->moveToThread(&lComThread);

    QObject::connect(&lComThread,  SIGNAL(started()),  &w, SLOT(onStart()));
    QObject::connect(&lComThread,  SIGNAL(finished()),  lCom, SLOT(deleteLater()));
#endif

    // Signal/slots connections
    QObject::connect(&w, SIGNAL(sInitCom()), lCom, SLOT(onInit()));
    QObject::connect(&w, SIGNAL(sQuery()), lCom, SLOT(onQuery()));
    QObject::connect(lCom, SIGNAL(sComInitialized(const QString &)), &w, SLOT(onComInitialized(const QString &)));
    QObject::connect(lCom, SIGNAL(sSubObjectInitialized(const QString &)), &w, SLOT(onSubObjectInitialized(const QString &)));
    QObject::connect(lCom, SIGNAL(sStatus(const QString &)), &w, SLOT(onStatus(const QString &)));

#ifdef USE_THREAD
    // Start Engine thread
    lComThread.start();
#else
    // Direct start
    w.onStart();
#endif

    // Execute application loop
    int lStatus = a.exec();

#ifdef USE_THREAD
    lComThread.quit();
    lComThread.wait();
#endif

    return lStatus;
}
