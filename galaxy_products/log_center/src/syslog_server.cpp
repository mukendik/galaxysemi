#include <QApplication>
#include "syslog_server.h"


SyslogServer::SyslogServer(quint16 tcp_port, QObject* parent): QTcpServer(parent), disabled(false), mMaxLogLevel(7) //, mUdpSocket(0)
{
    qDebug("SyslogServer::SyslogServer");
    if (!listen(QHostAddress::Any, tcp_port))
    {
        qDebug("SyslogServer::SyslogServer: TCP listen failed !");
    }
    else
        qDebug("SyslogServer::SyslogServer: TCP listen ok.");

    //connect(this, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));

    /*
    if (!mUdpSocket.bind(QHostAddress::Any, 514))
    {
        qDebug("Failed to bind udp 514");
    }
    */
    //connect(&mUdpSocket, SIGNAL(readyRead()), this, SLOT(readClient()) );
}

SyslogServer::~SyslogServer()
{
    qDebug("~SyslogServer");
}

void SyslogServer::SetMaxLogLevel(int MLL)
{
    mMaxLogLevel=MLL;
}

void SyslogServer::readClient()
{
    //qDebug("SyslogServer::readClient");
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
    {
        //qDebug("SyslogServer::readClient: cant retrieve Tcp client socket");
        QUdpSocket* lS=qobject_cast<QUdpSocket*>(QObject::sender());
        if (lS)
        {
            if (lS->hasPendingDatagrams())
            {
                //qDebug("SyslogServer::readClient: Udp pending data : %d", (int)lS->pendingDatagramSize());
                QByteArray lD; lD.resize(lS->pendingDatagramSize());
                QHostAddress senderAddress;
                quint16 senderPort=0;
                if (lS->readDatagram(lD.data(), lD.size(), &senderAddress, &senderPort) == -1)
                {
                    qDebug("Cant read udp datagram");
                    lS->close();
                    return;
                }
                //qDebug(lD.data());
                SMessage lM;
                QString lMsg=QString(lD.data()).remove(0, 21); // 21 to remove all (pri + date)
                QRegExp lRE("[<](\\d*)[>].*");
                lRE.exactMatch(QString(lD.data()));
                // todo : extract facility:
                //#define LOG_FACMASK	0x03f8	/* mask to extract facility part dec=1016 bin=001111111000 */
                bool ok=false;
                QString lPri=lRE.cap(1); // could be 123, 124, 135 because of the facility :
                //lM.m_sev=lPri.toInt(&ok); // could be <x> or <xxx>
                ushort pri=lPri.toUShort(&ok);
                if (ok)
                {
                        //pri=pri<<5;
                        //pri=pri>>5; // 135 = 0x87 = 10000111
                        pri=pri&0x0007; // 00F8=11111000

                        if (pri>mMaxLogLevel)
                            return; // ignore this message

                        lM.m_sev=(int)pri;
                }
                else
                {
                    lM.m_sev=0;
                }
                //
                lM.m_atts.insert("msg", lMsg);
                // date
                // sometimes there is no space between priority and date : <54>Jul... vs <534> Jul
                //QString lDate=QString(lD.data()).section(' ', 0, 2).remove(0,3);
                // Let s use a regexp

                lRE.setPattern("[<]\\d*[>][ ]*([a-z|A-Z]{3,3}[ ]\\d{1,2}[ ]\\d{1,2}[:]\\d{1,2}[:]\\d{1,2}).*");
                lRE.exactMatch(QString(lD.data()));
                QString lDate=lRE.cap(1);

                QDateTime lDT=QDateTime::fromString(lDate, "MMM d hh:mm:ss"); // "Jun 29 21:12:31" or "Jul 6 ...."
                if (!lDT.isValid() || lDT.isNull())
                    lM.m_date=QDateTime::currentDateTime();
                else
                    lM.m_date=lDT;
                emit sNewMessage(lM);
                QApplication::processEvents();
            }
        }

        return;
    }
    if (!socket->canReadLine())
    {
        qDebug("SyslogServer::readClient: cant read line on socket !");
        return;
    }

    char lData[1024];
    //qint64 lNum=
    socket->readLine(lData, 1024);
    /*
    if (lNum>0)
        qDebug(lData);
    */
}

void SyslogServer::discardClient()
{
    qDebug("SyslogServer::discardClient");
}

void SyslogServer::clientConnected()
{
    qDebug("SyslogServer::clientConnected");
}

void SyslogServer::OnConnectionClosed()
{
    qDebug("SyslogServer::connectionClosed");
}

void SyslogServer::aboutToClose()
{
    qDebug("aboutToClose");
}

void SyslogServer::bytesWritten(qint64 lN)
{
    qDebug("SyslogServer::bytesWritten %ld", (long)lN);
}

void SyslogServer::error(QAbstractSocket::SocketError lSE)
{
    qDebug("SyslogServer error: %d", lSE);
}

void SyslogServer::OnNewConnection()
{
    qDebug("new connection");
    QTcpSocket *clientConnection = this->nextPendingConnection();
    if (!clientConnection)
    {
        qDebug("Failed to get tcp socket");
        return;
    }

    clientConnection->setTextModeEnabled(true);

    QObject::connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readClient()));
    QObject::connect(clientConnection, SIGNAL(disconnected()), this, SLOT(discardClient()));
    QObject::connect(clientConnection, SIGNAL(connected()), this,    SLOT(clientConnected()));
    QObject::connect(clientConnection, SIGNAL(connectionClosed()), this, SLOT(OnConnectionClosed()));
    QObject::connect(clientConnection, SIGNAL(aboutToClose()), this,   SLOT(aboutToClose()) );
    QObject::connect(clientConnection, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)) );
    QObject::connect(clientConnection, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(error(QAbstractSocket::SocketError)) );

    qDebug("bytes available: %d", (int)clientConnection->bytesAvailable());
    if (clientConnection->canReadLine())
    {
        char lData[1024];
        qint64 lNum=clientConnection->readLine(lData, 1024);
        lData[lNum]='\0';
        qDebug("received %s", lData);
    }
}

/*
void SyslogServer::incomingConnection(int socket)
{
    qDebug("SyslogServer::incomingConnection on socket %d", socket);
    QTcpSocket* s = new QTcpSocket(this);

    if (!s->setSocketDescriptor(socket))
        qDebug("SyslogServer::incomingConnection: failed to set socket desc");

}
*/
