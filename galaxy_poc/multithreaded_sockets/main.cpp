#include <QtCore/QCoreApplication>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "main.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug("main: current thread id: %d", (int)QThread::currentThreadId() );

    Server s(4747);
    if (!s.isListening())
        return EXIT_FAILURE;

    /*
    QTimer t(0);
    t.connect(&t, SIGNAL(timeout()), &s, SLOT(createNewClient()));
    t.start(1000);
    */

    return a.exec();
}
