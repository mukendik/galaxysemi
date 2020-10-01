#ifdef GCORE15334


#include <time.h>
#include <gtl_core.h>
#include <gqtl_log.h>

#include "license_provider_manager.h"
#include "clientsocket.h"
#include "clientnode.h"
#include "engine.h"
#include "gex_options_handler.h"
#include "license_provider.h"
#include <QApplication>
//#include "DebugMemory.h"


namespace GS
{
namespace Gex
{
    QMap<qintptr, ClientSocket*> ClientSocket::sInstances;

    ClientSocket::ClientSocket( qintptr sock, QObject *parent, const char *lName ) :
        QTcpSocket(parent), mBusy(false), mDeleteSocket(false), mClientNode(0)
    {
        sInstances.insert(sock, this);

        GSLOG(SYSLOG_SEV_NOTICE, "new ClientSocket");
        setObjectName(lName);

        if (!connect( this, SIGNAL(readyRead()), SLOT(readClient()) ))
            GSLOG(4, "Cannot connect readyRead")

        // Qt3 : use disconnected() instead
        //if (!connect( this, SIGNAL(connectionClosed()), SLOT(OnConnectionClosed()) ))
          //  GSLOG(4, "Cannot connect connectionClosed");
        if (!QObject::connect(this, SIGNAL(disconnected()), this, SLOT(OnConnectionClosed())))
          GSLOG(4, "Cannot connect signal 'disconnected'");

        if (!QObject::connect(this, SIGNAL(connected()), this, SLOT(OnConnected())))
          GSLOG(4, "Cannot connect signal 'connected'");
        if (!QObject::connect(this, SIGNAL(hostFound()), this, SLOT(OnHostFound()) ))
          GSLOG(4, "Cannot connect signal 'hostFound'");
        if (!QObject::connect(this, SIGNAL(error(QAbstractSocket::SocketError)),
                              this, SLOT(OnError(QAbstractSocket::SocketError)) ) )
          GSLOG(4, "Cannot connect signal 'error'");


        if (!setSocketDescriptor(sock)) // Qt4
           GSLOG(3, "Failed to set socket decriptor");

        m_iSocketInstance = sock;

        if (waitForConnected()) // Qt4
            GSLOG(5, "Client socket connected")
        else
        {
            GSLOG(3, "Client socket not connected");
        }

        // Set hooks to read/write message data
        gnm_SetReadBufferHook(gtcnm_ReadBufferData);
        gnm_SetAddDataToBufferHook(gtcnm_CreateBufferData);

        uiPacketNb = 0;

        //QTimer::singleShot(100, this, SLOT(readClient()));
    }

    ClientSocket::~ClientSocket()
    {
        // socketDescriptor() is already -1 because disconnected
        GSLOG(5, QString("ClientSocket %1 destructor...").arg(m_iSocketInstance).toLatin1().constData() );
        //  clearPendingData(); // from Q3Socket ?
        flush(); // why not ?
        close();

        qintptr lLibId = sInstances.key(this);
        GS::Gex::Engine::GetInstance().ReleaseGTLLicense(lLibId);

        sInstances.remove(m_iSocketInstance);
        if (isOpen())
            //if (!QTcpSocket::disconnect()) // or disconnect(this,0,0,0) ?
                GSLOG(4, "Socket still opened despite close() call...");
    }

    void ClientSocket::OnHostFound()
    {
        GSLOG(5, "OnHostFound");
    }

    void ClientSocket::OnConnected()
    {
        GSLOG(6, "ClientSocket Connected");
    }

    void ClientSocket::WriteLineToSocket(QString strMessage)
    {
        QTextStream os(this);
        // Send command line server
        os << strMessage << "\n";
    }

    QString ClientSocket::ReadLineFromSocket(void)
    {
        QString		strMessage;
        strMessage = readLine();
        strMessage = strMessage.trimmed();	// Remove leading '\n'
        return strMessage;
    }

    void ClientSocket::readClient()
    {
        if (!isOpen())
        {
            GSLOG(4, "ClientSocket::readClient but socket not opened")
            return;
        }

        mBusy=true;

        GS::Gex::SocketMessage cSocketMessage(GS::Gex::SocketMessage::eReceive);

        //QString	strClientMessage;
        //QString	strReply;
        // Fix compilation issue on Lugburz: bTimeout set but not used
        //bool bTimedOut=false;

        /*
        // old Qt3
        if(waitForMore(1, &bTimedOut) <= 0)
        {
            //QTimer::singleShot(100, this, SLOT(readClient()));
            mBusy=false;
            return;
        }
        */

        // Read Message sent by the client
        //while(waitForMore(1, &bTimedOut) > 0) // Qt3
        //while(waitForReadyRead()) // does not work...
        while(bytesAvailable()>0)
        {
            // Fix compilation issue on Lugburz: bTimeout set but not used
            //bTimedOut = (error() == QAbstractSocket::SocketTimeoutError);

            if (!isOpen())
                break;
            if (!ReadMessageFromSocket(cSocketMessage))
                GSLOG(4, "ReadMessageFromSocket failed");
            if (!isOpen())
                break;
            if (!ProcessMessage(cSocketMessage))
            {
                GSLOG(3, "ProcessMessage failed");
            }

            QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers); // Just to refresh gui if any
        }
        //while(bytesAvailable() > 0);

        //QTimer::singleShot(100, this, SLOT(readClient()));

        #if 0
        bool bMessageRead = ReadMessageFromSocket(cSocketMessage);
        while(bMessageRead)
        {
            ProcessMessage(cSocketMessage);
            bMessageRead = ReadMessageFromSocket(cSocketMessage);
        }
        #endif

        mBusy=false;

    }

    bool ClientSocket::ReadMessageFromSocket(GS::Gex::SocketMessage & cSocketMessage)
    {
        char        pHeaderBuffer[GNM_HEADER_SIZE];
        char        *pMessageBuffer=NULL;
        void        *pMessage=NULL;
        int         nStatus=0;
        GNM_HEADER  stHeader;

        // Read Header data
        if(ReadDataFromSocket(pHeaderBuffer, GNM_HEADER_SIZE, true, 0) == false)
        {
            GSLOG(4, "ClientSocket::ReadMessageFromSocket: ReadDataFromSocket failed");
            return false;
        }
        // Convert header data to header structure
        nStatus = gnm_ReadBufferHeader(&stHeader, pHeaderBuffer);
        if(nStatus != GNET_ERR_OKAY)
            return false;

        // Check if header is valid
        if(!IsValidHeader(&stHeader))
            return false;

        // Allocate memory to receive data from the remote side
        pMessageBuffer = (char *)malloc(stHeader.mMessageLength*sizeof(char));
        if(!pMessageBuffer)
            return false;

        // Read Message data
        if(ReadDataFromSocket(pMessageBuffer, stHeader.mMessageLength, false, stHeader.mMessageType) == false)
        {
            free(pMessageBuffer);
            return false;
        }

        // Read data from received buffer into corresponding structure
        nStatus = gnm_ReadBufferData(stHeader.mMessageType, &pMessage, pMessageBuffer, stHeader.mMessageLength);
        if(nStatus != GNET_ERR_OKAY)
        {
            free(pMessageBuffer);
            return false;
        }

        // Free allocated ressources
        free(pMessageBuffer);

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = stHeader.mMessageType;
        cSocketMessage.mAckRequested = (stHeader.mAckRequested == 0) ? false : true;
        cSocketMessage.mMessage = pMessage;

        return true;
    }

    ///////////////////////////////////////////////////////////
    // Reads data from the socket.
    // Returns true if amount of expected data read, false else
    ///////////////////////////////////////////////////////////
    bool ClientSocket::ReadDataFromSocket(
            char *pBuffer, unsigned int uiExpectedBytes, bool bHeader, unsigned int uiMessageType)
    {
        //GSLOG(6, "Client socket ReadDataFromSocket...");
        unsigned int	uiTotalBytesRead=0, uiRemainingBytes;
        int				nBytesRead=0;
        char			*pBufferPos=0;
        bool			bTimedOut=false;
        unsigned int    uiNbReadCalls=0;

        // Avoid compilation warnings.
        #ifndef GTM_DUMP_SOCKETDATA
            Q_UNUSED(bHeader)
            Q_UNUSED(uiMessageType)
        #endif

        // Read avilable data
        //nBytesRead = readBlock(pBuffer, uiExpectedBytes); // Qt3
        nBytesRead = read(pBuffer, uiExpectedBytes);
        uiNbReadCalls++;
        if(nBytesRead == -1)
            return false;
        if(nBytesRead == (int)uiExpectedBytes)
        {
            #ifdef GTM_DUMP_SOCKETDATA
                if(!bHeader && (uiMessageType == GNM_TYPE_RESULTS))
                  DumpBuffer(pBuffer, uiExpectedBytes, nBytesRead, nBytesRead, uiNbReadCalls);
            #endif
            return true;
        }

        // Read more data
        uiTotalBytesRead += nBytesRead;
        uiRemainingBytes = uiExpectedBytes - uiTotalBytesRead;
        pBufferPos = pBuffer + uiTotalBytesRead;
        while((uiTotalBytesRead != uiExpectedBytes) && (nBytesRead > 0) && (!bTimedOut))
        //	while(uiTotalBytesRead != uiExpectedBytes)
        {
            nBytesRead = 0;

            //if(waitForMore(GTM_SOCK_RCV_TIMEOUT*1000, &bTimedOut) > 0) // Qt3 : waitForMore( msecs, bool timeout)
            //    Use waitForReadyRead() instead.
            //    For example, if you have code like:
            //     bool timeout;
            //     Q_ULONG numBytes = socket->waitForMore(30000, &timeout);
            //    you can rewrite it as:
            //     qint64 numBytes = 0;
            //     if (socket->waitForReadyRead(msecs))
            //         numBytes = socket->bytesAvailable();
            //     bool timeout = (error() == QAbstractSocket::SocketTimeoutError);

            if (waitForReadyRead( GTM_SOCK_RCV_TIMEOUT*1000 )) // This function blocks until new data is available for reading
            {
                bTimedOut=(error() == QAbstractSocket::SocketTimeoutError);

                //if((nBytesRead = readBlock(pBufferPos, uiRemainingBytes)) != -1) // Qt3
                if((nBytesRead = read(pBufferPos, uiRemainingBytes)) != -1)
                {
                    uiNbReadCalls++;
                    uiTotalBytesRead += nBytesRead;
                    uiRemainingBytes = uiExpectedBytes - uiTotalBytesRead;
                    pBufferPos = pBuffer + uiTotalBytesRead;
                }
            }
        }

        #ifdef GTM_DUMP_SOCKETDATA
            if(!bHeader && (uiMessageType == GNM_TYPE_RESULTS))
                DumpBuffer(pBuffer, uiExpectedBytes, uiTotalBytesRead, nBytesRead, uiNbReadCalls);
        #endif

        // Check if expected amount of data read
        if(uiTotalBytesRead == uiExpectedBytes)
            return true;

        return false;
    }

    void ClientSocket::DumpBuffer(
            char *pBuffer, unsigned int uiExpectedBytes,
            unsigned int uiTotalBytesRead, unsigned int nBytesRead, unsigned int uiNbReadCalls)
    {
        if((uiExpectedBytes == 0)  || (pBuffer == NULL))
            return;

        GSLOG(5, "Dump buffer...");
        char szMessage[256];
        char szTemp[256];
        int	i=0;
        char cChar='0';

        FILE *hFile = fopen("c:\\trace_socket_gtm.txt", "a+");
        if(hFile)
        {
            sprintf(szTemp, "Buffer [total=%d][last=%d][calls=%d][packet nb=%2X] :",
                    uiTotalBytesRead, nBytesRead, uiNbReadCalls, uiPacketNb++);
            strcpy(szMessage, szTemp);
            for(i=0 ; i<10 && i<(int)uiTotalBytesRead ; i++)
            {
                cChar = *(pBuffer+i);
                sprintf(szTemp, " %2X", cChar & 0xff);
                strcat(szMessage, szTemp);
            }
            strcat(szMessage, " ...");
            fprintf(hFile, "%s\n", szMessage);
            fclose(hFile);
        }
    }

    bool ClientSocket::SendMessageToSocket(const GS::Gex::SocketMessage & cSocketMessage)
    {
        int	nStatus=0;
        unsigned int uiBufferSize=0, uiSentBytes=0;
        char *pBuffer=NULL;

        if((cSocketMessage.mMessageType != GNM_TYPE_RESULTS) && (cSocketMessage.mMessageType != GNM_TYPE_ACK))
            GSLOG(SYSLOG_SEV_DEBUG, QString("Send message (type %1) to socket...")
                  .arg(cSocketMessage.mMessageType).toLatin1().data() );

        // Prepare buffer with header and data
        nStatus = gnm_CreateBufferHeaderData(cSocketMessage.mMessageType, 0,
                                             cSocketMessage.mMessage, &pBuffer, &uiBufferSize);
        if(nStatus != GNET_ERR_OKAY)
        {
            return nStatus;
        }

        // Send Data
        //uiSentBytes = writeBlock(pBuffer, uiBufferSize); // Qt3
        uiSentBytes = write(pBuffer, uiBufferSize);
        if(uiSentBytes != uiBufferSize)
        {
            free(pBuffer);
            return false;
        }
        // Free buffer
        free(pBuffer);
        // Flush socket
        flush();
        return true;
    }

//    bool ClientSocket::IsValidHeader(PT_GNM_HEADER pHeader)
//    {
//        if(pHeader == NULL)
//            return false;

//        // Control message length
//        if(pHeader->mMessageLength == 0)
//            return false;

//        // Control message type
//        if((pHeader->mMessageType < GNM_TYPE_BEGIN_NUMBER) ||
//           (pHeader->mMessageType > GNM_TYPE_END_NUMBER))
//           return false;

//        return true;
//    }
    ClientSocket::HEADER_CHECK ClientSocket::IsValidHeader(PT_GNM_HEADER pHeader)
       {
           if(pHeader == NULL)
               return NULL_HEADER;

           // Control message length
           if(pHeader->mMessageLength == 0)
               return BAD_LENGTH_HEADER;

           // Control message type
           if((pHeader->mMessageType < GNM_TYPE_BEGIN_NUMBER) || (pHeader->mMessageType > GNM_TYPE_END_NUMBER))
              return ILLEGAL_MESSAGETYPE_HEADER;

           return VALID_HEADER;
       }

    bool ClientSocket::ProcessMessage(GS::Gex::SocketMessage &cSocketMessage)
    {
        if((cSocketMessage.mMessageType != GNM_TYPE_RESULTS) && (cSocketMessage.mMessageType != GNM_TYPE_ACK))
            GSLOG(SYSLOG_SEV_DEBUG, QString("Received message (type %1) from socket...")
                  .arg(cSocketMessage.mMessageType).toLatin1().data() );

        if (!isOpen())
        {
            GSLOG(4, "ProcessMessage but socket not open");
            return false;
        }

        // Message dispatcher
        bool lStatus=false;
        switch(cSocketMessage.mMessageType)
        {
            case GNM_TYPE_Q_CFC: // Socket 'CheckForConnection'  startup handshake
                lStatus = ProcessMessage_Q_CFC((PT_GNM_Q_CFC)cSocketMessage.mMessage);
                break;

            case GNM_TYPE_Q_INIT: // Station Q_INIT message received.
                lStatus = ProcessMessage_Q_INIT((PT_GNM_Q_INIT)cSocketMessage.mMessage);
                break;

            case GNM_TYPE_PRODINFO:	// Station PRODINFO message received.
                lStatus = ProcessMessage_PRODINFO((PT_GNM_PRODINFO)cSocketMessage.mMessage);
                break;

            case GNM_TYPE_Q_TESTLIST: // Query for TestList received.
                lStatus = ProcessMessage_Q_TESTLIST((PT_GNM_Q_TESTLIST)cSocketMessage.mMessage);
                break;

            case GNM_TYPE_Q_PATCONFIG_STATIC: // Query for static PAT config received.
                lStatus = ProcessMessage_Q_PATCONFIG_STATIC((PT_GNM_Q_PATCONFIG_STATIC)cSocketMessage.mMessage);
                break;

            case GNM_TYPE_Q_PATCONFIG_DYNAMIC: // Query for dynamic PAT config received.
                lStatus = ProcessMessage_Q_PATCONFIG_DYNAMIC((PT_GNM_Q_PATCONFIG_DYNAMIC)cSocketMessage.mMessage);
                break;

            case GNM_TYPE_RESULTS: // Station sending one/few runs (test results+bin,etc)
                lStatus = ProcessMessage_RESULTS((PT_GNM_RESULTS)cSocketMessage.mMessage);
                break;

            case GNM_TYPE_Q_ENDLOT:	// Station sending a reset (new lot...)
                lStatus = ProcessMessage_Q_ENDLOT((PT_GNM_Q_ENDLOT)cSocketMessage.mMessage);
                break;

            case GNM_TYPE_Q_END_OF_SPLITLOT: // Could be a end of splitlot
                lStatus = ProcessMessage_Q_END_OF_SPLITLOT((PT_GNM_Q_END_OF_SPLITLOT)cSocketMessage.mMessage);
                break;


            default:
                GSLOG(4, QString("ProcessMessage: unknown message %1.")
                      .arg(cSocketMessage.mMessageType).toLatin1().data() );
                lStatus = false;
                break;
        }

        //qApp->processEvents(QEventLoop::ExcludeSocketNotifiers); // could delete the pClient though a Close message7

        // Check if we need to send an aknowledge
        if(cSocketMessage.mAckRequested)
        {
            GNM_ACK                 lAcknowledge;
            GS::Gex::SocketMessage	cSocketMessage(GS::Gex::SocketMessage::eSend);

            // Fill reply structure
            lAcknowledge.mTimeStamp = time(NULL);
            if(lStatus)
                lAcknowledge.mStatus = GTC_STATUS_OK;
            else
                lAcknowledge.mStatus = GTC_STATUS_EPM;

            // Fill SocketMessage structure
            cSocketMessage.mMessageType = GNM_TYPE_ACK;
            cSocketMessage.mMessage = (void *)&lAcknowledge;

            // Send acknowledge
            SendMessageToSocket(cSocketMessage);
            // Free stuff
            gtcnm_FreeStruct(GNM_TYPE_ACK, &lAcknowledge);
        }

        return lStatus;
    }

    bool ClientSocket::ProcessMessage_Q_CFC(PT_GNM_Q_CFC pMsg_Q_CFC)
    {
        int				nStatus=0;
        GNM_R_CFC		stMessage_R_CFC;
        GS::Gex::SocketMessage	cSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill reply structure
        stMessage_R_CFC.mTimeStamp = pMsg_Q_CFC->mTimeStamp;
        stMessage_R_CFC.mStatus = GTC_STATUS_OK;

        bool ok=false;
        // Check if we max nb of allowed connections reached
        int lMaxConnections = GS::Gex::Engine::GetInstance().GetOptionsHandler().GetOptionsMap()
                .GetOption("gtm", "max_gtl_connections").toInt(&ok);
        if (!ok)
            GSLOG(4, QString("Cant get max_gtl_connections option as an integer: %1").arg(lMaxConnections)
                  .toLatin1().data() );

        //Check if running GTL is allowed
        bool lGTLLicenseStatus = false;
        QString lGTLLicenseInfo;
        qintptr lGTLLibId = m_iSocketInstance;

        if(sInstances.size() < lMaxConnections)
        {
            GS::Gex::Engine::GetInstance().RequestGTLLicense(lGTLLibId,lGTLLicenseStatus,lGTLLicenseInfo);
        }


        GSLOG(5, QString("Client version %1.%2 is calling...")
              .arg(pMsg_Q_CFC->mGtlVersionMajor).arg(pMsg_Q_CFC->mGtlVersionMinor).toLatin1().data() );

        if (pMsg_Q_CFC->mGtlVersionMajor!=GTL_VERSION_MAJOR || pMsg_Q_CFC->mGtlVersionMinor!=GTL_VERSION_MINOR)
        {
            GSLOG(4, QString("Version mismatch : %1.%2 (GTL) vs %3.%4 (GTM)")
                  .arg(pMsg_Q_CFC->mGtlVersionMajor).arg(pMsg_Q_CFC->mGtlVersionMinor)
                  .arg(GTL_VERSION_MAJOR).arg(GTL_VERSION_MINOR).toLatin1().data() );
            stMessage_R_CFC.mStatus = GTC_STATUS_VERSIONMISMATCH;
        }
        else
        //if(pGexMainWindow->mGtmWidget->TotalStations() > lMaxConnections)
        if (sInstances.size() > lMaxConnections)
        {
            // Reject this connection
            stMessage_R_CFC.mStatus = GTC_STATUS_MAXCONNECTIONS;
            GSLOG(4, "Max connection reached. Client will be rejected.");
        }
        else if (GS::Gex::Engine::GetInstance().GetClientState() != Engine::eState_NodeReady)
        {
            stMessage_R_CFC.mStatus = GTC_STATUS_NO_GTM_LIC;
            GSLOG(4, "No (more) license. Client will be rejected.");
        }
        else if(!lGTLLicenseStatus)
        {
            // Reject this connection license not allowed
            stMessage_R_CFC.mStatus = GTC_STATUS_NO_GTL_LIC; //GTM License
            GSLOG(SYSLOG_SEV_WARNING, QString("License not accorded to %1 and the reaseon is  : %2").arg(lGTLLibId).arg(lGTLLicenseInfo).toLatin1().constData());
        }
        else
        {
            // This connection will be accepted, show tester GUI
            //pGexMainWindow->mGtmWidget->clientShowTesterGUI(m_iSocketInstance);
            emit sClientAccepted();
            //emit mClientNode->sClientAccepted();
            GSLOG(6, "Connection accepted");
        }

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = GNM_TYPE_R_CFC;
        cSocketMessage.mMessage = (void *)&stMessage_R_CFC;

        // Send reply message
        nStatus = SendMessageToSocket(cSocketMessage);

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_Q_CFC, pMsg_Q_CFC);
        gtcnm_FreeStruct(GNM_TYPE_R_CFC, &stMessage_R_CFC);

        return nStatus;
    }

    ///////////////////////////////////////////////////////////
    // Received "Q_INIT message": Station initialization
    bool ClientSocket::ProcessMessage_Q_INIT(PT_GNM_Q_INIT pMsg_Q_INIT)
    {
        GSLOG(5, "ProcessMessage_Q_INIT...");
        int nStatus=0;
        GNM_R_INIT	stMessage_R_INIT;

        // Init reply structure
        gtcnm_InitStruct(GNM_TYPE_R_INIT, &stMessage_R_INIT);

        // Process 'Q_INIT' message: return status and some global settings to Tester station
        //pGexMainWindow->mGtmWidget->clientInit(m_iSocketInstance,pMsg_Q_INIT,&stMessage_R_INIT);
        if (mClientNode)
            mClientNode->ClientInit(pMsg_Q_INIT,&stMessage_R_INIT);
        else
            GSLOG(3, "ClientNode null");

        // Prepare to return the reply packet to the tester
        GS::Gex::SocketMessage	cSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = GNM_TYPE_R_INIT;
        cSocketMessage.mMessage = (void *)&stMessage_R_INIT;

        // Send reply message
        nStatus = SendMessageToSocket(cSocketMessage);

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_Q_INIT, pMsg_Q_INIT);
        gtcnm_FreeStruct(GNM_TYPE_R_INIT, &stMessage_R_INIT);

        return nStatus;
    }

    ///////////////////////////////////////////////////////////
    // Received "PRODINFO message": Production information
    bool ClientSocket::ProcessMessage_PRODINFO(PT_GNM_PRODINFO pMsg_PRODINFO)
    {
        // Process 'PRODINFO' message
        //pGexMainWindow->mGtmWidget->clientProdInfo(m_iSocketInstance, pMsg_PRODINFO);
        if (mClientNode)
            mClientNode->clientProdInfo(pMsg_PRODINFO);
        else
            GSLOG(3, "mClientNode null");

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_PRODINFO, pMsg_PRODINFO);

        return true;
    }

    // Received "Q_TESTLIST message": TestList request
    bool ClientSocket::ProcessMessage_Q_TESTLIST(PT_GNM_Q_TESTLIST pMsg_Q_TESTLIST)
    {
        int	nStatus=0;
        GNM_R_TESTLIST stMessage_R_TESTLIST;

        // Init reply structure
        gtcnm_InitStruct(GNM_TYPE_R_TESTLIST, &stMessage_R_TESTLIST);

        // Process 'Q_TESTLIST' message: return TestList details to tester station
        //pGexMainWindow->mGtmWidget->clientTestList(m_iSocketInstance,pMsg_Q_TESTLIST,&stMessage_R_TESTLIST);
        if (!mClientNode)
            return false;
        QString lRes=mClientNode->clientTestList(pMsg_Q_TESTLIST,&stMessage_R_TESTLIST);
        if (lRes.startsWith("err"))
            return false;

        // Prepare to return the reply packet to the tester
        GS::Gex::SocketMessage	cSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = GNM_TYPE_R_TESTLIST;
        cSocketMessage.mMessage = (void *)&stMessage_R_TESTLIST;

        // Send reply message
        nStatus = SendMessageToSocket(cSocketMessage);

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_Q_TESTLIST, pMsg_Q_TESTLIST);
        gtcnm_FreeStruct(GNM_TYPE_R_TESTLIST, &stMessage_R_TESTLIST);

        return nStatus;
    }

    ///////////////////////////////////////////////////////////
    // Received "Q_PATCONFIG_STATIC message": Static Pat config request
    ///////////////////////////////////////////////////////////
    bool ClientSocket::ProcessMessage_Q_PATCONFIG_STATIC(PT_GNM_Q_PATCONFIG_STATIC pMsg_Q_PATCONFIG_STATIC)
    {
        int						nStatus;
        GNM_R_PATCONFIG_STATIC	stMessage_R_PATCONFIG_STATIC;

        // Init reply structure
        gtcnm_InitStruct(GNM_TYPE_R_PATCONFIG_STATIC, &stMessage_R_PATCONFIG_STATIC);

            // Process 'Q_PATCONFIG' message: return PAT config details to tester station
            //pGexMainWindow->mGtmWidget->clientPatInit_Static(m_iSocketInstance,
                                                             //pMsg_Q_PATCONFIG_STATIC,&stMessage_R_PATCONFIG_STATIC);
        if (mClientNode)
            mClientNode->clientPatInit_Static(pMsg_Q_PATCONFIG_STATIC, &stMessage_R_PATCONFIG_STATIC);
        else
            GSLOG(3, "mClientNode null");

        // Prepare to return the reply packet to the tester
        GS::Gex::SocketMessage	cSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = GNM_TYPE_R_PATCONFIG_STATIC;
        cSocketMessage.mMessage = (void *)&stMessage_R_PATCONFIG_STATIC;

        // Send reply message
        nStatus = SendMessageToSocket(cSocketMessage);

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_Q_PATCONFIG_STATIC, pMsg_Q_PATCONFIG_STATIC);
        gtcnm_FreeStruct(GNM_TYPE_R_PATCONFIG_STATIC, &stMessage_R_PATCONFIG_STATIC);

        return nStatus;
    }

    ///////////////////////////////////////////////////////////
    // Received "Q_PATCONFIG_DYNAMIC message": Dynamic Pat config request
    ///////////////////////////////////////////////////////////
    bool ClientSocket::ProcessMessage_Q_PATCONFIG_DYNAMIC(PT_GNM_Q_PATCONFIG_DYNAMIC pMsg_Q_PATCONFIG_DYNAMIC)
    {
        int						nStatus;
        GNM_PATCONFIG_DYNAMIC	stMessage_PATCONFIG_DYNAMIC;

        // Init reply structure
        gtcnm_InitStruct(GNM_TYPE_PATCONFIG_DYNAMIC, &stMessage_PATCONFIG_DYNAMIC);

        if (mClientNode)
            mClientNode->clientPatInit_Dynamic(pMsg_Q_PATCONFIG_DYNAMIC, &stMessage_PATCONFIG_DYNAMIC);
        else
            GSLOG(3, "mClientNode null");

        // Prepare to return the reply packet to the tester
        GS::Gex::SocketMessage cSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = GNM_TYPE_PATCONFIG_DYNAMIC;
        cSocketMessage.mMessage = (void *)&stMessage_PATCONFIG_DYNAMIC;

        // Send reply message
        nStatus = SendMessageToSocket(cSocketMessage);

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_Q_PATCONFIG_DYNAMIC, pMsg_Q_PATCONFIG_DYNAMIC);
        gtcnm_FreeStruct(GNM_TYPE_PATCONFIG_DYNAMIC, &stMessage_PATCONFIG_DYNAMIC);

        return nStatus;
    }

    bool ClientSocket::ProcessMessage_RESULTS(PT_GNM_RESULTS pMsg_RESULTS)
    {
        if (!mClientNode)
        {
            gtcnm_FreeStruct(GNM_TYPE_RESULTS, pMsg_RESULTS);
            GSLOG(3, "Null mClientNode");
            return false;
        }

        // New flow
        mClientNode->clientTestResults(pMsg_RESULTS);

        //emit sNewTestResults;

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_RESULTS, pMsg_RESULTS);

        return true;
    }

    bool ClientSocket::ProcessMessage_Q_END_OF_SPLITLOT(PT_GNM_Q_END_OF_SPLITLOT pMsg_Q)
    {
        GSLOG(5, QString("Client socket %1 end of splitlot...").arg(m_iSocketInstance).toLatin1().data() );

        // Reply structure
        GNM_R_END_OF_SPLITLOT stMessage_R;
        stMessage_R.mTimeStamp = pMsg_Q->mTimeStamp;
        stMessage_R.mStatus = GTC_STATUS_OK;

        if (mClientNode)
        {
            GSLOG(4, "End Of Splitlot: anything to do ?");
            //mClientNode->clientEndlot(pMsg_Q);
        }
        else
        {
            GSLOG(3, "mClientNode null");
            stMessage_R.mStatus = GTC_STATUS_EPM;
        }

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_Q_END_OF_SPLITLOT, pMsg_Q);

        /*
        // Send reply ? No, because of the bug
        GS::Gex::SocketMessage lSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill SocketMessage structure
        lSocketMessage.mMessageType = GNM_TYPE_R_END_OF_SPLITLOT;
        lSocketMessage.mMessage = (void*)&stMessage_R;

        // Send reply message
        bool lRes=SendMessageToSocket(lSocketMessage);
        if (!lRes)
            GSLOG(4, "Send Message 'end_of_splitlot' to socket failed");
        */

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_R_END_OF_SPLITLOT, &stMessage_R);

        return true;
    }

    bool ClientSocket::ProcessMessage_Q_ENDLOT(PT_GNM_Q_ENDLOT pMsg_Q_ENDLOT)
    {
        GSLOG(5, QString("Client socket %1 endlot received").arg(m_iSocketInstance).toLatin1().data() );

        // Reply structure
        GNM_R_ENDLOT stMessage_R_ENDLOT;
        stMessage_R_ENDLOT.mTimeStamp = pMsg_Q_ENDLOT->uiTimeStamp;
        stMessage_R_ENDLOT.mStatus = GTC_STATUS_OK;

        // Process 'Endlot' message
        //pGexMainWindow->mGtmWidget->clientEndlot(m_iSocketInstance,pMsg_Q_ENDLOT);
        if (mClientNode)
            mClientNode->clientEndlot(pMsg_Q_ENDLOT);
        else
        {
            GSLOG(3, "mClientNode null");
            stMessage_R_ENDLOT.mStatus = GTC_STATUS_EPM;
        }

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_Q_ENDLOT, pMsg_Q_ENDLOT);

        // Send reply
        GS::Gex::SocketMessage	cSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = GNM_TYPE_R_ENDLOT;
        cSocketMessage.mMessage = (void *)&stMessage_R_ENDLOT;

        // Send reply message
        SendMessageToSocket(cSocketMessage);

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_R_ENDLOT, &stMessage_R_ENDLOT);

        return true;
    }

    void ClientSocket::OnConnectionClosed()
    {
        GSLOG(5, QString("Client socket %1 connection closed").arg(m_iSocketInstance).toLatin1().data() );
        // Socket closing: delete socket connection
        if (mBusy)
            mDeleteSocket=true;
        mClientNode->setProperty("ClientAboutToClose", true);
    }

    void ClientSocket::OnError(QAbstractSocket::SocketError lSE)
    {
        // case 7527
        if (lSE!=RemoteHostClosedError) // no problem, GTL has just closed the connection without saying goodbye.
            GSLOG(4, QString("Socket error %1: '%2'").arg((int)lSE).arg(errorString()).toLatin1().data() );
        // todo : force disconnect ? try to continue ? Delete socket ?
        mDeleteSocket=true;
    }

    QString ClientSocket::OnNotifyTester(const QString & strMessage, int lSeverity)
    {
        GSLOG(7, QString("GTM to tester notification. Message=%1, Severity=%2").arg(strMessage).arg(lSeverity)
              .toLatin1().constData());

        GNM_NOTIFY stMessage_NOTIFY;
        GS::Gex::SocketMessage cSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill printf structure
        strcpy(stMessage_NOTIFY.szMessage, strMessage.toLatin1().data());
        stMessage_NOTIFY.nTimeout = lSeverity;

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = GNM_TYPE_NOTIFY;
        cSocketMessage.mMessage = (void*)&stMessage_NOTIFY;

        // Send message
        if (!SendMessageToSocket(cSocketMessage))
            return "error: SendMessageToSocket failed";
        return "ok";
    }

    void ClientSocket::OnSendDynamicPatLimits(void *pMessage)
    {
        PT_GNM_PATCONFIG_DYNAMIC pMsg_PATCONFIG_DYNAMIC = (PT_GNM_PATCONFIG_DYNAMIC)pMessage;
        GS::Gex::SocketMessage cSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = GNM_TYPE_PATCONFIG_DYNAMIC;
        cSocketMessage.mMessage = pMessage;

        // Send message
        SendMessageToSocket(cSocketMessage);

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_PATCONFIG_DYNAMIC, pMsg_PATCONFIG_DYNAMIC);
        free(pMsg_PATCONFIG_DYNAMIC);
    }

    ///////////////////////////////////////////////////////////
    // Send a COMMAND to the tester.
    ///////////////////////////////////////////////////////////
    void ClientSocket::OnSendCommand(void *pMessage)
    {
        PT_GNM_COMMAND	pMsg_COMMAND = (PT_GNM_COMMAND)pMessage;
        GS::Gex::SocketMessage	cSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = GNM_TYPE_COMMAND;
        cSocketMessage.mMessage = pMessage;

        // Send message
        SendMessageToSocket(cSocketMessage);

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_COMMAND, pMsg_COMMAND);
        free(pMsg_COMMAND);
    }

    ///////////////////////////////////////////////////////////
    // Send a WRITETOSTDF request to the tester.
    ///////////////////////////////////////////////////////////
    void ClientSocket::OnSendWriteToStdf(void *pMessage)
    {
        PT_GNM_WRITETOSTDF	pMsg_WRITETOSTDF = (PT_GNM_WRITETOSTDF)pMessage;
        GS::Gex::SocketMessage	cSocketMessage(GS::Gex::SocketMessage::eSend);

        // Fill SocketMessage structure
        cSocketMessage.mMessageType = GNM_TYPE_WRITETOSTDF;
        cSocketMessage.mMessage = pMessage;

        // Send message
        SendMessageToSocket(cSocketMessage);

        // Free stuff
        gtcnm_FreeStruct(GNM_TYPE_WRITETOSTDF, pMsg_WRITETOSTDF);
        free(pMsg_WRITETOSTDF);
    }

} // Gex
} // GS

#endif
