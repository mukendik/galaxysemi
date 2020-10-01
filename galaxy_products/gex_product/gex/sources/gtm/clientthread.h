#ifdef GCORE15334


#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QThread>
#include <QTcpSocket>

namespace GS
{
namespace Gex
{

class ClientNode;

/*! \class  ClientThread
    \brief  The Client thread class inherit from QThread and handle all the network AND stats process inside the run method.
*/
class ClientThread : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(ClientThread)

public:
    ClientThread(qintptr socketDescriptor, QObject* p);
    virtual ~ClientThread();

    quint16 blockSize;
    //QNetworkSession *networkSession; // todo ?
    qintptr mSocketDescriptor;

    // just say if we have to exit/stop the thread or not
    bool mExitRequested;
    // the method to start the thread
    void run();

signals:
    void error(QTcpSocket::SocketError socketError);
    void sNewClient( QWeakPointer<GS::Gex::ClientNode>);
    void sCloseClient( QWeakPointer<GS::Gex::ClientNode>);

public slots:
    void discardClient();
    void stateChanged(QAbstractSocket::SocketState);
    void read();
    void hostFound();
    void displayError(QAbstractSocket::SocketError socketError);
    void connected();
    void connectionClosed();
};

}
}

#endif // CLIENTTHREAD_H
#endif
