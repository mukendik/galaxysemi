#include <QCoreApplication>
#include "main.h"

Client::Client(int socketDescriptor, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor)
{
    qDebug("New client");
}

Client::~Client()
{
    qDebug("~Client");
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    qDebug("socketError %d", socketError);
}

void Client::read()
{
    qDebug("Client::read: %d", (int)QThread::currentThreadId());

}

void Client::connected()
{
    qDebug("client connected");
}

void Client::hostFound()
{
    qDebug("host found");
}

void Client::run()
{
    qDebug("current thread id: %d", (int)QThread::currentThreadId());

    // the Client has been created in main thread so his members belongs to the main thread.
    //mTcpSocket.moveToThread(QThread::currentThread()); // does not work
    // the socket cannot be parented to the CLient because they do not belong to the same thread
    mTcpSocket=new QTcpSocket();

    if (!mTcpSocket->setSocketDescriptor(socketDescriptor))
    {
        qDebug("Failed to set Socket desc");
        emit error(mTcpSocket->error());
        return;
    }

    // connecting read to Client read ?
    // The Client belongs to the main thread. So the read function will be runned by the main thread.
    //if (!QObject::connect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(read())))
      //qDebug("Client : connect readyRead failed");
    //if (!QObject::connect(&tcpSocket, SIGNAL(readyRead()), ((Server*)parent()), SLOT(readClient())))
      //qDebug("Client : connect readyRead failed");

    QObject::connect(mTcpSocket, SIGNAL(disconnected()), this, SLOT(discardClient()));
    QObject::connect(mTcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    QObject::connect(mTcpSocket, SIGNAL(hostFound()), this, SLOT(hostFound()) );
    //QObject::connect(&tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
      //               this, SLOT(stateChanged(QAbstractSocket::SocketState)) );
    //QObject::connect(&tcpSocket, SIGNAL(connectionClosed()), this, SLOT(connectionClosed()) );

    // mandatory : if not called the socket wont work
    if (mTcpSocket->waitForConnected())
        qDebug("Client socket Connected");
    else
    {
        qDebug("Client socket not connected");
        return;
    }

    // Set hooks to read/write message data
    gnm_SetReadBufferHook(gtcnm_ReadBufferData);
    gnm_SetAddDataToBufferHook(gtcnm_CreateBufferData);

    while(true)
    {
        QCoreApplication::processEvents();

        //mTcpSocket->waitForReadyRead(); // or do we simply have to wait ?

        msleep(1000); //sleep(1);

        if (mTcpSocket->bytesAvailable()==0)
        {
            continue;
        }

        qDebug("Client %d: tcp socket has %d byte available and still %d bytes to write...",
               socketDescriptor, (int)mTcpSocket->bytesAvailable(), (int)mTcpSocket->bytesToWrite());

        //mTcpSocket.waitForMore(); // deprecated

        qint64 s=mTcpSocket->bytesAvailable();
        QByteArray ba=mTcpSocket->readAll();
        qDebug("Client: received %d bytes...", (int)s);
        QString st=QString(ba.data()).replace('\n', ' ');
        st.truncate(30);
        qDebug("Client : received: %s", st.toLatin1().data() );
        //qDebug("Client : received: Hex = %s", ba.toHex().data());

        char *pBuffer=NULL;
        unsigned int uiBufferSize=0;

        if (s==20) // probably the CFC
        {
            // Sending CFC ok
            GNM_R_CFC	stMessage_R_CFC;
            char		szTimestamp[UT_MAX_TIMESTAMP_LEN];
            stMessage_R_CFC.mTimeStamp = ut_GetFullTextTimeStamp(szTimestamp); //pMsg_Q_CFC->mTimeStamp;
            stMessage_R_CFC.mStatus = GTC_STATUS_OK;
            int nStatus = gnm_CreateBufferHeaderData(GNM_TYPE_R_CFC, (void*)&stMessage_R_CFC, &pBuffer, &uiBufferSize);
            if(nStatus != GNET_ERR_OKAY)
            {
                qDebug("gnm_CreateBufferHeaderData failed: %d", nStatus);
                return;
            }
            qint64 n=mTcpSocket->write(pBuffer, uiBufferSize); // or writeData ?
            if (mTcpSocket->flush())
                qDebug("flush works");
            qDebug("Client: %ld bytes written to socket (R_CFC)...", (long)n);
            free(pBuffer);
        }

        if (s>20)    //s==5331) // probably the INIT message (with recipe)
        {
            // sending INIT OK
            GNM_R_INIT m;
            gtcnm_InitStruct(GNM_TYPE_R_INIT, &m);
            m.nStatus = GTC_STATUS_OK;
            m.nGtlModules = GTC_GTLMODULE_PAT;
            m.uiResultFrequency = 1;	// Number of test program runs per socket packet
            gnm_CreateBufferHeaderData(GNM_TYPE_R_INIT, (void*)&m, &pBuffer, &uiBufferSize);
            qint64 n=mTcpSocket->write(pBuffer, uiBufferSize); // or writeData ?
            if (mTcpSocket->flush())
                qDebug("Client: flush works");
            qDebug("Client: %ld bytes written to socket...", (long)n);
            gtcnm_FreeStruct(GNM_TYPE_R_INIT, &m);
            free(pBuffer);
        }
    }


    /*
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (quint16)0;
    out << text;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    tcpSocket.write(block);
    tcpSocket.disconnectFromHost();
    tcpSocket.waitForDisconnected();
    */

    mTcpSocket->disconnectFromHost();
}

void Client::connectionClosed()
{
    qDebug("Client connection closed");
}

void Client::stateChanged(QAbstractSocket::SocketState ss)
{
    qDebug("Client stateChanged %d", (int)ss);
}

void Client::discardClient()
{
    qDebug("Client::discardClient");
}
