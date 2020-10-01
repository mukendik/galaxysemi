#include <QtCore/QCoreApplication>
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QSharedMemory lSM1("SM1");
    unsigned lSize=800000000;
    bool lB=lSM1.create(lSize);
    if (!lB)
        return -1;

    qDebug("SharedMem successfully created!");

    if (!lSM1.lock())
    {
        qDebug("Cannot lock SM");
        return -1;
    }

    double* lData=(double*)lSM1.data();
    for (unsigned int i=0; i<lSize/sizeof(double); i++)
        lData[i]=69.;
    lSM1.unlock();

    return a.exec();
}
