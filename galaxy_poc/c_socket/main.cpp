#include <QCoreApplication>
#include <io.h>
#include <sys/fcntl.h>
#include <winsock.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //struct sockaddr_in sock
    SOCKET sock= socket(


    return a.exec();
}
