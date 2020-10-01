#include <stdlib.h>
#include <stdio.h>
#include <QApplication>
#include <QUdpSocket>
#include <QObject>
#include <QMessageBox>
#include "logwidget.h"
#include "syslog_server.h"


int main(int argc, char** argv)
{
    QApplication a(argc, argv);
    a.setWindowIcon(QPixmap(":/res/galaxyb.png"));

    QUdpSocket lUDPSock(0); // when this is local, it overwrites the GexLogWidget memory...

    quint16 udpport=514;
    if (QApplication::arguments().size()>1)
    {
        bool ok=false;
        quint16 lUdpPort=0;
        lUdpPort=QApplication::arguments().at(1).toInt(&ok);
        if (ok)
            udpport=lUdpPort;
    }

    qDebug("Trying to bind udp port %d...", udpport);
    if (!lUDPSock.bind(QHostAddress::Any, udpport))
    {
        //qDebug("Failed to bind udp 514");
        QMessageBox::critical(0, "GalaxySemi LogsCenter",
          QString("Failed to bind UDP port %1. Please :\n" \
          "- turn off any syslog server/daemon already running (syslogd, syslog-ng, rsyslog,...)\n " \
          "- close any application already listening to this port\n" \
          "- use a different port : example : gs-logcenter 5140 and configure your client to log to this port\n" \
          "The application will now exit.").arg((int)udpport) );
        return EXIT_FAILURE;
    }

    //qDebug("UDP bind ok.");

    SyslogServer ss(5140, 0);
    if (!QObject::connect(&lUDPSock, SIGNAL(readyRead()), &ss, SLOT(readClient())))
        return EXIT_FAILURE;

    CGexLogWidget w(0);
    QObject::connect(&ss, SIGNAL(sNewMessage(SMessage&)), &w, SLOT(AppendMessage(SMessage&)));
    //QObject::connect(&w, SIGNAL(sLogLevelChanged(int)), &ss, SLOT(SetMaxLogLevel(int)));
    w.showMaximized();
    //a.setActiveWindow((QWidget*)&w);

    int r=a.exec();

    //return EXIT_SUCCESS;
    return r;
}
