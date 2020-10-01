#include <QCoreApplication>
#include <gqtl_sysutils.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug("Calling gqtl get processes...\n");
    QMap<unsigned, QString> lProcesses=CGexSystemUtils::GetProcesses();
    if (lProcesses.size()<=1)
    {
        if (lProcesses.size()==1)
            qDebug("Failure: %s\n", lProcesses.first().toLatin1().data());
        else
            qDebug("Failure: no processes at all found\n");
        return EXIT_FAILURE;
    }
    qDebug("Success: %d processes found\n", lProcesses.count());
    return EXIT_SUCCESS; //a.exec();
}
