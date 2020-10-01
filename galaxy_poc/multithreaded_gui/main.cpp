#include <stdlib.h>
#include <stdio.h>
//#include <sys/time.h> // needed for the time() function
#include <unistd.h> // unistd also available on mingw (even if partial)

#include <QtCore/QCoreApplication>
#include <QtCore/qmath.h>
#include <QCoreApplication>
#include <QThread>
#include <QApplication>
#include <QLabel>

#include <thread.h>
#include <widget.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Thread* lT1=new Thread();
    lT1->setObjectName("Thread1");
    QSharedPointer<Thread> lTSP1(lT1);
    QWeakPointer<Thread> lTWP1(lTSP1);

    Widget lW1(lTWP1);
    lW1.show();

    qRegisterMetaType< QWeakPointer<Thread> >("ThreadWP"); // in order to use such in signals/slots
    QObject::connect(lT1, SIGNAL(sNewValue()), &lW1, SLOT(UpdateGUI()) );

    lT1->start();

    return a.exec();
}
