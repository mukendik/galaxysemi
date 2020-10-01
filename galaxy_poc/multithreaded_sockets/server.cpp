#include "main.h"

Server::Server(quint16 port, QObject* parent): QTcpServer(parent)
{
    qDebug("Server::Server");
    if (!listen(QHostAddress::Any, port))
    {
        qDebug("Server::Server: listen failed !");
    }
    else
        qDebug("Server::Server: listen ok.");

    //connect(this, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));
}

void Server::readClient()
{
    qDebug("Server::readClient");
    QTcpSocket* socket = (QTcpSocket*)sender();
    if (!socket)
    {
        qDebug("Server::readClient: cant retrieve client socket");
        return;
    }

    //socket->connectToHostImplementation();
    //socket->connectToHost();
    //socket->moveToThread();
    //socket->open()
    //socket->thread()


    if (!socket->canReadLine())
    {
        qDebug("Server::readClient: cant read line on socket !");
        return;
    }
    char lData[1024];
    qint64 lNum=socket->readLine(lData, 1024);
    if (lNum>0)
        qDebug(lData);
}

void Server::discardClient()
{
    qDebug("Server::discardClient");
}

void Server::clientConnected()
{
    qDebug("Server::clientConnected");
}

void Server::OnConnectionClosed()
{
    qDebug("Server::connectionClosed");
}

void Server::aboutToClose()
{
    qDebug("Server::aboutToClose");
}

void Server::bytesWritten(qint64 lN)
{
    qDebug("Server::bytesWritten %ld", (long)lN);
}

void Server::error(QAbstractSocket::SocketError lSE)
{
    qDebug("Server error: %d", lSE);
}

/*
void Server::OnNewConnection()
{
    qDebug("Server new connection");
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
*/

void Server::incomingConnection(int socket)
{
    qDebug("Server::incomingConnection on socket %d...", socket);
    Client *thread = new Client(socket, this);
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    qDebug("Server::incomingConnection: starting thread...");
    thread->start();
    if (this->hasPendingConnections())
        qDebug("Server has pending connections");
}
