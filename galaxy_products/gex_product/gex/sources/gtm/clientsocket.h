#ifdef GCORE15334

#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QMap>
#include <QTcpSocket>

#include <gstdl_netmessage_c.h>
#include <gtc_netmessage.h>
#include "socketmessage.h"

// Timeouts (in seconds)
#define GTM_SOCK_SND_TIMEOUT	2
#define GTM_SOCK_RCV_TIMEOUT	5

namespace GS
{
namespace Gex
{

class ClientNode;

///////////////////////////////////////////////////////////
//  The ClientSocket class provides a socket that is connected with a client (tester station)
//  For every client that connects to the server, the server creates a new
//  instance of this class.
///////////////////////////////////////////////////////////
/*! \class  ClientSocket
    \brief  The Client socket class inherit from TcpSocket and handle network communication with the tester server.
        The process is not to be done in this class but in the friend class ClientNode.
*/
class ClientSocket : public QTcpSocket
{
    Q_OBJECT
    Q_DISABLE_COPY(ClientSocket)

    bool mBusy;
    bool mDeleteSocket;

public:
    enum HEADER_CHECK { NULL_HEADER, BAD_LENGTH_HEADER, ILLEGAL_MESSAGETYPE_HEADER, VALID_HEADER };
    ClientSocket(qintptr sock, QObject *parent=0, const char *name=0 );
    virtual ~ClientSocket();
    qintptr	m_iSocketInstance;													// Instance ID (file descriptor)
    ClientNode* mClientNode;

private:
    static QMap<qintptr, ClientSocket*> sInstances;

    // Sockets I/O
    // Reads data from socket
    bool ReadDataFromSocket(char *pBuffer, unsigned int uiExpectedBytes, bool bHeader, unsigned int uiMessageType);
    // Reads a message from the socket
    bool ReadMessageFromSocket(GS::Gex::SocketMessage & cSocketMessage);
    // Sends a message to the socket
    bool SendMessageToSocket(const GS::Gex::SocketMessage & cSocketMessage);
    // Check if header is valid
    HEADER_CHECK IsValidHeader(PT_GNM_HEADER pHeader);
    // Processes a message received through the socket.
    bool ProcessMessage(GS::Gex::SocketMessage & cSocketMessage);

    // Processes a Q_CFC message received through the socket: Received "Q_CFC message": Socket handshake at startup
    bool ProcessMessage_Q_CFC(PT_GNM_Q_CFC pMsg_Q_CFC);
    // Processes a Q_INIT message received through the socket.
    bool ProcessMessage_Q_INIT(PT_GNM_Q_INIT pMsg_Q_INIT);
    // Processes a PRODINFO message received through the socket.
    bool ProcessMessage_PRODINFO(PT_GNM_PRODINFO pMsg_PRODINFO);
    // Processes a Q_TESTLIST message received through the socket.
    bool ProcessMessage_Q_TESTLIST(PT_GNM_Q_TESTLIST pMsg_Q_TESTLIST);
    // Processes a Q_PATCONFIG_STATIC message received through the socket.
    bool ProcessMessage_Q_PATCONFIG_STATIC(PT_GNM_Q_PATCONFIG_STATIC pMsg_Q_PATCONFIG_STATIC);
    // Processes a Q_PATCONFIG_DYNAMIC message received through the socket.
    bool ProcessMessage_Q_PATCONFIG_DYNAMIC(PT_GNM_Q_PATCONFIG_DYNAMIC pMsg_Q_PATCONFIG_DYNAMIC);
    // Process a RESULTS (one/few runs test results) message received through the socket.
    // Received "RESULTS' message": Station has performed one/few RUNs
    bool ProcessMessage_RESULTS(PT_GNM_RESULTS pMsg_RESULTS);
    // Process a Q_ENDLOT message received through the socket. Station has detected a end lot...
    bool ProcessMessage_Q_ENDLOT(PT_GNM_Q_ENDLOT pMsg_Q_ENDLOT);
    //! \brief Process a end of splitlot message: either the lot is split, either it is the end of initial test, start of retest
    bool ProcessMessage_Q_END_OF_SPLITLOT(PT_GNM_Q_END_OF_SPLITLOT pMsg_Q);
    // Send string to client (encrypt string)
    void WriteLineToSocket(QString strMessage);
    // Read string received from client
    QString ReadLineFromSocket(void);

    void DumpBuffer(char *pBuffer, unsigned int uiExpectedBytes, unsigned int uiTotalBytesRead,
                       unsigned int nBytesRead, unsigned int uiNbReadCalls);
    unsigned int uiPacketNb;

public slots:
    /*! \brief  Return if the socket is to be deleted */
    bool GetDeleteSocket() { return mDeleteSocket; }
    /*! \brief Receiving query from client... Some data received from the client through the socket. */
    void readClient();
    /*! \brief Client closed connection... */
    void OnConnectionClosed();
    /*! \brief Called when error signal triggered... */
    void OnError(QAbstractSocket::SocketError);

    /*! \brief Slot called when received a signal for tester notification. Send a notification message to the tester.
        Severity : 0 = warning
    */
    QString OnNotifyTester(const QString & strMessage, int lSeverity);

    //! \brief Slot called when received a signal for dynamic PAT limits update. Send a dynamic PAT limits to the tester.
    void OnSendDynamicPatLimits(void *pMessage);
    //! \brief Slot called when received a signal to send a command to tester station
    void OnSendCommand(void *pMessage);
    //! \brief Slot called when received a signal to send a writetostdf request to tester station
    void OnSendWriteToStdf(void *pMessage);
    //! \brief On connected...
    void OnConnected();
    //! \brief
    void OnHostFound();
signals:
    /*! \brief */
    void sClientAccepted();
};

}
}

#endif // CLIENTSOCKET_H
#endif
