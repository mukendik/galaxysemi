
#ifdef GCORE15334
#include <gqtl_log.h>
#include <QSharedPointer>

#ifdef GSDAEMON
    #include "daemon.h"
#endif
#include "testerserver.h"
#include "clientthread.h"
#include "clientnode.h"
#include "station.h"
#include "socketmessage.h"
#include "engine.h"
#include <product_info.h>

//#include "DebugMemory.h" // must be the last include


namespace GS
{
namespace Gex
{
    const char* TesterServer::sPropertyAcceptNewClient="mAcceptNewClient";

    TesterServer::TesterServer(QObject* parent): QTcpServer(parent), mPort(-1) //, mAcceptNewClient(true)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "new TesterServer...");
        setObjectName("GSTesterServer");
        setProperty(sPropertyAcceptNewClient, true);
        // in order for some Qt connections to work it needs to register the params
        // needs : default constr, copy constr, destructor
        int lR=qRegisterMetaType<CStation*>("GSStation");
        GSLOG(SYSLOG_SEV_NOTICE, QString("qRegisterMetaType on Station returned %1").arg(lR).toLatin1().data() );
        // Registreing ClientNode needs a copy constructor
        //lR=qRegisterMetaType<ClientNode>("GSClientNode");
        lR=qRegisterMetaType< QWeakPointer<ClientNode> >("GSClientNodeWP");
        GSLOG(SYSLOG_SEV_NOTICE, QString("qRegisterMetaType on ClientNode returned %1").arg(lR).toLatin1().data() );
        //connect(this, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));
    }

    TesterServer::~TesterServer()
    {
        GSLOG(SYSLOG_SEV_NOTICE, "TesterServer destruction...");
        GSLOG(6, QString("Still %1 instance(s) of CTestData").arg(CTestData::GetNumOfInstances()).toLatin1().data() );
        GSLOG(6, QString("Still %1 instance(s) of SocketMessage")
              .arg(GS::Gex::SocketMessage::GetNumOfInstances()).toLatin1().data() );
        GSLOG(6, QString("Still %1 instance(s) of COutlierSummary").arg(COutlierSummary::GetNumOfInstances())
              .toLatin1().data() );
    }

    QString TesterServer::Start(quint16 port)
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("TesterServer::Start on port %1...").arg(port).toLatin1().data() );

        if (isListening())
            return QString("ok: already listening port %1").arg(serverPort());
        if (!listen(QHostAddress::Any, port))
        {
            return QString("error: listening on port %1 failed ").arg(port);
        }

        mPort=port;

        #ifdef GSDAEMON
            if (!QObject::connect(GS::Gex::Daemon::GetInstance(), SIGNAL(sStopping()), this, SLOT(OnExit())))
                GSLOG(3, "Cannot connect daemon stopping signal")
            else
                GSLOG(6, "Daemon stoping successfully connected to TesterServer OnExit");
        #endif

        return "ok: listen ok";
    }

    void TesterServer::OnExit()
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("On exit: %1 client(s) to kill...").arg(mClients.size()).toLatin1().data() );
        foreach(ClientThread* lCT, mClients)
        {
            if (lCT)
            {
                lCT->mExitRequested=true;
                lCT->exit(-1); // Should kill the run loop ?
                //mClients.remove(lCT); // mClients will be refreshed by OnClientEnd()
            }
        }
    }

    bool TesterServer::Pause()
    {
        // Tester Server can be started/paused/resumed with GTM product
        if (GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Pause reuqested");
            setProperty(sPropertyAcceptNewClient, false); //mAcceptNewClient=false;

            // Status changed, UI component can be updated
            emit sStatusChanged(false);

            return true;
        }
        else
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Pause is not allowed without any GTM license");

            return false;
        }

    }

    bool TesterServer::Resume()
    {
        // Tester Server can be started/paused/resumed with GTM product
        if (GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Resume requested");
            setProperty(sPropertyAcceptNewClient, true); //mAcceptNewClient=true;

            // Status changed, UI component can be updated
            emit sStatusChanged(true);

            return true;
        }
        else
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Resume is not allowed without any GTM license");

            return false;
        }
    }

    void TesterServer::discardClient()
    {
        GSLOG(SYSLOG_SEV_NOTICE, "discardClient");
    }

    void TesterServer::clientConnected()
    {
        GSLOG(SYSLOG_SEV_NOTICE, "clientConnected");
    }

    void TesterServer::OnConnectionClosed()
    {
        GSLOG(SYSLOG_SEV_NOTICE, "connectionClosed");
    }

    void TesterServer::aboutToClose()
    {
        GSLOG(SYSLOG_SEV_NOTICE, "aboutToClose");
    }

    void TesterServer::bytesWritten(qint64 lN)
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("bytesWritten %1").arg((long)lN).toLatin1().constData() );
    }

    void TesterServer::error(QAbstractSocket::SocketError lSE)
    {
        GSLOG(4, QString("Server error: %1").arg(lSE).toLatin1().data() );
    }

    void TesterServer::OnClientEnd()
    {
        GSLOG(SYSLOG_SEV_NOTICE, "OnClientEnd...");
        QObject *lS=QObject::sender();
        if (!lS)
            return;
        ClientThread* lCT=qobject_cast<ClientThread*>(lS);
        if (!lCT)
            return;
        //mClients.remove(lCT); // Qt3
        mClients.removeAll(lCT); // Check me : for the Eagle muti sector case: do we really have to remove all ?
    }

    void TesterServer::incomingConnection(qintptr socket)
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("Incoming connection on socket %1...").arg(socket).toLatin1().data() );

        if (!property(sPropertyAcceptNewClient).toBool()) //if (!mAcceptNewClient)
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Refusing any new client.");
            // Add code here to refuse this connection for the client not to wait indefinitely...
            QTcpSocket lS(this);
            lS.setSocketDescriptor(socket);
            lS.close();
            //lS.reset();
            return;
        }

        ClientThread* lCT=new ClientThread(socket, this);
        //QSharedPointer<ClientThread> lThreadSP(lCT);
        if (!lCT)
        {
            GSLOG(3, "Incoming connection: failed to create a new thread. Out of memory ?");
            //GS::Gex::Engine::GetInstance().Exit(-1); // Exit or not ?
            return;
        }
        lCT->setObjectName(QString("ClientThread on socket %1").arg(socket));

        mClients.append(lCT);

        if (!QObject::connect(lCT, SIGNAL(finished()), lCT, SLOT(deleteLater())))
            GSLOG(4, "Cannot connect client thread finished and deleteLater");

        if (!QObject::connect(lCT, SIGNAL(finished()), this, SLOT(OnClientEnd())))
            GSLOG(4, "Cannot connect client thread finished and OnClientEnd");

        GSLOG(SYSLOG_SEV_NOTICE, "incomingConnection: starting thread for this client...");
        lCT->start();
        QSharedPointer<ClientThread>* lCTSP=new QSharedPointer<ClientThread>(lCT);
        emit sNewClient(QWeakPointer<ClientThread>(*lCTSP));
        if (hasPendingConnections())
            GSLOG(6, "Server has pending connections");
    }

} // Gex

} // GS
#endif
