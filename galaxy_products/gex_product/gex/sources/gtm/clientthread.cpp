#ifdef GCORE15334


#include <QHostAddress>
#include <QMutex>
#include <QSharedPointer>
#include <stdint.h> // for intptr_t
#include <gqtl_log.h>
#include <gqtl_sysutils.h>

#include "clientthread.h"
#include "clientnode.h"
#include "clientsocket.h"

//#include "DebugMemory.h" // must be the last include


namespace GS
{
namespace Gex
{
    ClientThread::ClientThread(qintptr socketDescriptor, QObject *parent)
        : QThread(parent), mSocketDescriptor(socketDescriptor), mExitRequested(false)
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("New client thread on socket %1").arg(socketDescriptor).toLatin1().data() );
    }

    ClientThread::~ClientThread()
    {
        // (intptr_t)currentThreadId() returns the main thread bacause the thread has been created by the main thread
        GSLOG(5, QString("ClientThread destructor for socket %1...").arg(mSocketDescriptor).toLatin1().data() );
        /*
            QMap<QString, QVariant> lCounts;
            CGexSystemUtils::GetQObjectsCount(*this, lCounts, true);
            foreach(QString k, lCounts.keys())
                GSLOG(7, k+":"+QString::number(lCounts[k].toInt()) );
        */
    }

    void ClientThread::displayError(QAbstractSocket::SocketError socketError)
    {
        GSLOG(4, QString("ClientThread socketError %1").arg(socketError).toLatin1().data() );
        //quit(); //terminate(); ?
    }

    void ClientThread::read()
    {
        GSLOG(6, QString("ClientThread read: in thread %1").arg((intptr_t)QThread::currentThreadId())
              .toLatin1().constData());
    }

    void ClientThread::connected()
    {
        GSLOG(6, "ClientThread connected");
    }

    void ClientThread::hostFound()
    {
        GSLOG(6, "ClientThread host found");
    }

    void ClientThread::run()
    {
        GSLOG(5, QString("Run : current thread id: %1").arg((intptr_t)QThread::currentThreadId())
              .toLatin1().data() );

        // the Client has been created in main thread so his members belongs to the main thread.
        //mTcpSocket.moveToThread(QThread::currentThread()); // does not work
        // the socket cannot be parented to the Client because they do not belong to the same thread
        /*
          From http://qt-project.org/doc/qt-4.8/threads-qobject.html
          The child of a QObject must always be created in the thread where the parent was created.
            This implies, among other things, that you should never pass the QThread object (this)
            as the parent of an object created in the thread
            (since the QThread object itself was created in another thread).
        */
        ClientSocket mTcpSocket(mSocketDescriptor, (QObject*)0,
           QString("ClientSocket %1").arg(mSocketDescriptor).toLatin1().constData() );

        if (mTcpSocket.socketDescriptor()==-1)
        {
            GSLOG(4, QString("Failed to set socket descriptor: error %1").arg(mTcpSocket.error()).toLatin1().data() );
            emit error(mTcpSocket.error());
            return;
        }

        // connecting read to Client read ?
        // The Client belongs to the main thread. So the read function will be runned by the main thread.
        //if (!QObject::connect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(read())))
          //GSLOG("Client : connect readyRead failed");
        //if (!QObject::connect(&tcpSocket, SIGNAL(readyRead()), ((Server*)parent()), SLOT(readClient())))
          //GSLOG("Client : connect readyRead failed");
        QObject::connect(&mTcpSocket, SIGNAL(disconnected()), this, SLOT(discardClient()), Qt::QueuedConnection);
        QObject::connect(&mTcpSocket, SIGNAL(connected()), this, SLOT(connected()), Qt::QueuedConnection);
        QObject::connect(&mTcpSocket, SIGNAL(hostFound()), this, SLOT(hostFound()), Qt::QueuedConnection );
        qRegisterMetaType<QAbstractSocket::SocketState>("SocketState"); // needed in order to be used in a signal/slot
        QObject::connect(&mTcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                       this, SLOT(stateChanged(QAbstractSocket::SocketState)), Qt::QueuedConnection );

        // mandatory : if not called the socket wont work
        if (mTcpSocket.waitForConnected()) // Qt4 Socket only
        {
            GSLOG(6, QString("Client socket connected to %1").arg(mTcpSocket.peerAddress().toString()).toLatin1().data() );
        }
        else
        {
            GSLOG(3, "Client socket not connected");
            return;
        }

        // The socket must be the parent of the client for the client to speak with his parent
        //ClientNode lClientNode(&mTcpSocket);
        QSharedPointer<ClientNode> lCNSP(new ClientNode(&mTcpSocket));

        // Connections between tester station and socket communicating with tester station
        if (!connect(lCNSP.data(), SIGNAL(sNotifyTester(const QString &, int)),
                     &mTcpSocket, SLOT(OnNotifyTester(const QString &, int)), Qt::QueuedConnection) )
        {
            GSLOG(3, "Cannot connect signal NotifyTester");
            return;
        }
        if (!connect(lCNSP.data(), SIGNAL(sSendDynamicPatLimits(void *)),
                     &mTcpSocket, SLOT(OnSendDynamicPatLimits(void *)), Qt::QueuedConnection) )
        {
            GSLOG(3, "Cannot connect signal SendDynamicPatLimits");
            return;
        }
        if (!connect(lCNSP.data(), SIGNAL(sSendCommand(void *)),
                     &mTcpSocket, SLOT(OnSendCommand(void *)), Qt::QueuedConnection) )
        {
            GSLOG(3, "Cannot connect signal SendCommand");
            return;
        }
        if (!connect(lCNSP.data(), SIGNAL(sSendWriteToStdf(void *)),
                     &mTcpSocket, SLOT(OnSendWriteToStdf(void *)), Qt::QueuedConnection) )
        {
            GSLOG(3, "Cannot connect signal SendWriteToStdf");
            return;
        }

        QWeakPointer<ClientNode> lCNWP(lCNSP);
        emit sNewClient(lCNWP);

        // So do we have to start the loop of that thread or doing a while/sleep ?
        // from http://qt-project.org/doc/qt-4.8/threads-qobject.html
        exec(); // adviced by Digia

        /*
        while(!mExitRequested && !mTcpSocket.GetDeleteSocket()) // the thread should exit in discardClient()
        {
            // 1ms max to prevent the gui to freeze
            QCoreApplication::processEvents(QEventLoop::AllEvents, 1); // process events just for that thread...
            //QThread::currentThread()->wait(500); // ?
            //mTcpSocket->waitForReadyRead(); // or do we simply have to wait ?
            msleep(100); // how long ?

            //this->exec(); // Or dont we have to exec() the htread ?
            // Impossible : we need to deal with some local variables before exiting the thread

            //if (mTcpSocket.bytesAvailable()==0)
            //{
              //  continue;
            //}

            //GSLOG(5, QString("Client %1: tcp socket has %2 byte available and still %3 bytes to write...")
              //     .arg(mSocketDescriptor).arg((int)mTcpSocket.bytesAvailable()).arg((int)mTcpSocket.bytesToWrite()) );

            //mTcpSocket.waitForMore(); // deprecated
        }
        */

        //QMutex lMutex;
        //lMutex.lock();
        lCNSP.data()->setProperty("ClientAboutToClose", true);
        //lMutex.unlock();

        emit lCNSP.data()->sAboutToClose();

        GSLOG(5, QString("Finishing thread for socket %1 (ExitRequested=%2 DeleteSocket=%3)...")
              .arg(mTcpSocket.m_iSocketInstance).arg(mExitRequested?"true":"false")
              .arg(mTcpSocket.GetDeleteSocket()?"true":"false")
              .toLatin1().data() );

        // usually connected to OnCloseClientNode(...). Warning: Not called if computer too fast.
        emit sCloseClient(lCNWP);

        QCoreApplication::processEvents();
        //emit lClientNode.destroyed();

        QCoreApplication::processEvents(); // process for the calling thread !
        mTcpSocket.disconnectFromHost();
        //sleep(2);
        QCoreApplication::processEvents(); // process for the calling thread !
        // These lines are generating a warning output from Qt:
        // QAbstractSocket::waitForDisconnected() is not allowed in UnconnectedState
        if (mTcpSocket.isOpen() && mTcpSocket.state()!=QAbstractSocket::UnconnectedState)
            mTcpSocket.waitForDisconnected(); // useful ? probably not.
        QCoreApplication::processEvents(); // process for the calling thread !
        if (QCoreApplication::hasPendingEvents())
            GSLOG(5, "Exiting thread but pending events");
        GSLOG(6, "Exiting thread run method");
        //sleep(2);
    }

    void ClientThread::connectionClosed()
    {
        GSLOG(5, "Client connection closed");
        quit(); //terminate(); is not recommended by Qt doc
    }

    void ClientThread::stateChanged(QAbstractSocket::SocketState lSS)
    {
        GSLOG(5, QString("Client stateChanged %1").arg((int)lSS).toLatin1().constData() );
        // Why not...?
        //if (lSS==QAbstractSocket::ClosingState)
            //terminate();
    }

    void ClientThread::discardClient()
    {
        ClientSocket* lTcpSocket=qobject_cast<ClientSocket*>(sender());
        if (lTcpSocket)
        {
            if (lTcpSocket->mClientNode)
                emit lTcpSocket->mClientNode->sAboutToClose();
        }
        GSLOG(5, QString("Client %1 discard").arg(mSocketDescriptor).toLatin1().constData() );
        //QThread::exit(0); // does not break the while loop in run()...
        mExitRequested=true;
        quit(); // to finish exec() loop ?
        //terminate(); // to finish exec() ?

        /* Impossible to do that as the ClientNode and ClientSocket are local to the run method
            in order to be really multithreaded!
        emit sCloseClient(&lClientNode);
        QCoreApplication::processEvents();
        mTcpSocket.disconnectFromHost();
        if (mTcpSocket.isOpen())
            mTcpSocket.waitForDisconnected();
        QThread::exit(0);
        */
    }

}
}
#endif
