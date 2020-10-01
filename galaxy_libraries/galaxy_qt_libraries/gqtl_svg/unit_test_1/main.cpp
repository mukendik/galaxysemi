#include <QApplication>
#include <QTimer>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    qDebug("QT_VERSION_STR = %s", QT_VERSION_STR);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    #ifdef QT_DEBUG
        //QApplication::beep();
        //QApplication::aboutQt();
    #else

    #endif

    QTimer lTimer;
    QObject::connect(&lTimer, SIGNAL(timeout()), &w, SLOT(close()) );

    lTimer.start(2000);

    int r=0;
    #ifdef QT_DEBUG
        r=a.exec();
    #endif
    return r;
}
