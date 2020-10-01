#ifdef GCORE15334

#ifndef TESTERSERVER_H
#define TESTERSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QNetworkSession>

// Deprecated. Use instead GTL_DEFAULT_SERVER_PORT in gtl_core.h
//#define	TESTER_SERVER_PORT	4747		// Default Socket port#

namespace GS
{
    namespace Gex
    {
        class ClientThread;

        /*! \class TesterServer
            \brief The TesterServer class implements a server able to communicate with a GTL (tester, station,...) client.
        */
        class TesterServer : public QTcpServer
        {
            Q_OBJECT
            Q_DISABLE_COPY(TesterServer)
            Q_PROPERTY(int mCurrentNumOfClients READ GetNumOfClients)
            Q_PROPERTY(bool mIsListening READ isListening)

            /*! \brief server port : set when Start succeed */
            int mPort;
            /*! \brief Prevent any new connection/client (usefull in order to close smartly)*/
            //bool mAcceptNewClient; // moved to a dyn property

            QList<ClientThread*> mClients;

            public:
                TesterServer(QObject* parent = 0);
                    //: QTcpServer(parent),
                   //COutput(),
                   //disabled(false)
                virtual ~TesterServer();

                /*! \brief */
                Q_INVOKABLE QString Start(quint16 port);

                /*! \brief // inheritated form TcpServer : the main entry for a new client tester
                */
                void incomingConnection(qintptr socket);

                static const char* sPropertyAcceptNewClient;

            public slots:
                /*! \brief Request to pause the tester server, will refuse any new clients */
                bool Pause();
                /*! \brief Request to resume the tester server, will accept new clients */
                bool Resume();

                /*! \brief */
                void discardClient();
                /*! \brief */
                void clientConnected();
                /*! \brief */
                void OnConnectionClosed();
                /*! \brief Triggered when a client is going to finish/exit/close */
                void OnClientEnd();
                /*! \brief */
                void aboutToClose();
                /*! \brief */
                void bytesWritten(qint64);
                /*! \brief */
                void error(QAbstractSocket::SocketError);
                /*! \brief Force TesterServer to exit : will kill ALL current clients without waiting to finish. */
                void OnExit();
                /*! \brief Returns the number of clients connected to the Tester Sevrer */
                int GetNumOfClients() { return mClients.size(); }

            signals:
                /*! \brief */
                void sNewClient(QWeakPointer<GS::Gex::ClientThread>);
                void sStatusChanged(bool isRunning);

            private:
                /*! \brief Unused.*/
                //bool disabled;
        };
    } // Gex
} // GS

#endif // TESTERSERVER_H
#endif
