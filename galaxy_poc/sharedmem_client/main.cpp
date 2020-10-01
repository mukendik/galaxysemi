#include <QtCore/QCoreApplication>
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QSharedMemory lSM("SM1");
    bool lB=lSM.attach();
    if (!lB)
        return -1;

    qDebug("SharedMem 'SM1' successfully attached !");

    if (!lSM.lock())
    {
        qDebug("Cannot lock 'SM1'");
        return -1;
    }
    double* lData=(double*)lSM.data();
    qDebug("data[0]=%g", lData[0]);
    lSM.unlock();

    return a.exec();
}
