#ifndef MAIN_H
#define MAIN_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QNetworkSession>
#include "gstdl_netmessage_c.h"
#include "gstdl_utils_c.h"
#include "gtc_netmessage.h"

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server(quint16 port, QObject* parent = 0);
        //: QTcpServer(parent),
       //COutput(),
       //disabled(false)

   void incomingConnection(int socket);

    void pause()
    {
        qDebug("pause");
        disabled = true;
    }

    void resume()
    {
        qDebug("resume");
        disabled = false;
    }

public slots:
    //void createNewClient();

public slots:
    //void OnNewConnection();
    void readClient(); // called when a client asks something ('GET' or not)
    void discardClient();
    void clientConnected();
    void OnConnectionClosed();
    void aboutToClose();
    void bytesWritten(qint64);
    void error(QAbstractSocket::SocketError);

private:
    bool disabled;
};

class Client : public QThread   //, QTcpSocket
{
    Q_OBJECT

public:
    Client(int socketDescriptor, QObject* p);
    ~Client();
    //QTcpSocket *tcpSocket;
    quint16 blockSize;
    QNetworkSession *networkSession;
    int socketDescriptor;
    QTcpSocket *mTcpSocket;

    void run();

signals:
    void error(QTcpSocket::SocketError socketError);

public slots:
    void discardClient();
    void stateChanged(QAbstractSocket::SocketState);
    //void requestNew();
    void read();
    void hostFound();
    void displayError(QAbstractSocket::SocketError socketError);
    //void sessionOpened();
    void connected();
    void connectionClosed();

};

#endif // MAIN_H
