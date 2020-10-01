///////////////////////////////////////////////////////////
// GEX-LM server implementation file
///////////////////////////////////////////////////////////
#include <time.h>
#include <stdlib.h>

#include <QStringList>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "gexlm_server.h"
#include "gexlm.h"
#include "cryptofile.h"



///////////////////////////////////////////////////////////
// Extern functions
///////////////////////////////////////////////////////////
// main.cpp
extern void LogMessage(QString strMessage, bool bQWarning);
extern void g_event(QString strEvent,QString strMessage, QDateTime dateTimeStamp = QDateTime::currentDateTime());
extern QString g_normalize_text(QString &strParameter);
extern void	WriteDebugMessageFile(const QString & strMessage);

///////////////////////////////////////////////////////////
// Extern variables
///////////////////////////////////////////////////////////
// from gexlm.cpp
extern char	*szAppFullName;
extern QString strLocalFolder;

///////////////////////////////////////////////////////////
// Global variables
///////////////////////////////////////////////////////////
QList<ClientNode *>		pClientNodes;							// List of client nodes.
int						iMaximumLicenses=0;						// Maximum concurrent licenses allowed
int						iLicensesUsed=0;						// Total licenses in use
int						iEditionType=0;							// Standard Edition
int						iMonitorProducts=0;						// Total Products allowed in Monitoring
int						iOptionalModules=0;						// Bit0=Plugin support, Bit1=PAT support,Bit2=Y123Web mode,Bit3=AllTimezones accepted
int						iProductID=GEX_DATATYPE_ALLOWED_ANY;	// 0=Examinator, 1=Examinator for credence, ...,4=ExaminatorDB
QString					strExpiration;							// License expiration date
QString					m_strMaintenanceExpiration="";			// Maintenance expiration date
QString					strLicenseID;							// License Key ID
int						iAllowedTimeShift=90;					// Allowed tile shift in minutes
int						iMaxVersion_Maj=-1;						// Max allowed GEX version (major)
int						iMaxVersion_Min=-1;						// Max allowed GEX version (minor)
ClientNode *			clientNodeIdentifiedBy(QString strClientId);// Return Client identified by strClientId, NULL if not exist
QString					strYieldManDbConnection;				// YieldManDbConnection


///////////////////////////////////////////////////////////
//  The SimpleServer class handles new connections to the server. For every
//  client that connects, it creates a new ClientSocket -- that instance is now
//  responsible for the communication with that client.
///////////////////////////////////////////////////////////
SimpleServer::SimpleServer(QObject* parent, int nPortNumber) : QTcpServer(parent)
{
  QString strMessage;

  // Try to bind to given port
  if(listen(QHostAddress::Any, nPortNumber) == false)
  {
    strMessage = "Failed to bind to port " + QString::number(nPortNumber);
    strMessage += "...: Port may be used by another application, or 'gexlm' already running";
    LogMessage(strMessage, true);
    exit(1);
  }

  strMessage = "Gex-lm server socket bound to port " + QString::number(nPortNumber);
  LogMessage(strMessage, false);

  // Reset licenses counter
  iLicensesUsed = 0;

  // Signal/Slot connections
  connect(this, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));
}

///////////////////////////////////////////////////////////
// A client just connected!
///////////////////////////////////////////////////////////
void SimpleServer::OnNewConnection()
{
  while(hasPendingConnections() == true)
  {
    QTcpSocket *clientConnection = nextPendingConnection();
    new ClientSocket(clientConnection);
  }
}

///////////////////////////////////////////////////////////
//  The ClientNode class provides a list of sockets that are connected with a client.
///////////////////////////////////////////////////////////
ClientNode::ClientNode()
{

}

ClientNode::~ClientNode()
{
  WriteDebugMessageFile(QString("Deleting Client Node %1...").arg(m_strClientId).toLatin1());
}

////////////////////////////////////////////////////////////////////
// Build the message to log when client start or stop a connection
////////////////////////////////////////////////////////////////////
QString	ClientNode::BuildLogMessage(ClientSocket * pClientSocket)
{
  QString strLogMessage;

  // Log event: <date_time> <computer> <user> <license# given> <total licenses available> <peer port> <Product Id>
  strLogMessage = strComputer + " " + strUser + " " + QString::number(iLicensesUsed) + " " + QString::number(iMaximumLicenses) + " ";
  strLogMessage += QString::number(pClientSocket->tcpSocketPeerPort()) + " " + QString::number(iProductID);

  return strLogMessage;
}

////////////////////////////////////////////////////////////////////
// Client closed connection...
////////////////////////////////////////////////////////////////////
void ClientNode::OnDisconnectClient()
{
  while (!m_lstClientSocket.isEmpty())
    m_lstClientSocket.first()->OnDisconnectSocket();
}

///////////////////////////////////////////////////////////
//  The ClientSocket class provides a socket that is connected with a client.
//  For every client that connects to the server, the server creates a new
//  instance of this class.
///////////////////////////////////////////////////////////
ClientSocket::ClientSocket(QTcpSocket *pTcpSocket) : QObject(pTcpSocket), m_pTcpSocket(pTcpSocket)
{
  // Trace message
  QString strMessage;
  strMessage =  "Serving new socket connection from host "  + pTcpSocket->peerAddress().toString();
  strMessage += " using port " + QString::number(pTcpSocket->peerPort());
  strMessage += ".";
  WriteDebugMessageFile(strMessage);

  // Signal/Slot connections
  connect(m_pTcpSocket, SIGNAL(disconnected()), this, SLOT(OnDisconnectSocket()));
  connect(m_pTcpSocket, SIGNAL(readyRead()), this, SLOT(OnReadyRead()));
}

ClientSocket::~ClientSocket()
{
  WriteDebugMessageFile("Deleting Client Socket...");
}

///////////////////////////////////////////////////////////
// Send string to client (encrypt string)
///////////////////////////////////////////////////////////
void ClientSocket::WriteLineToSocket(QString strMessage)
{
  CCryptoFile ascii;	// Buffer used for crypting data!
  QTextStream os(m_pTcpSocket);
  char*		szCryptedString;
  QByteArray	pString = strMessage.toLatin1();

  ascii.SetBufferFromBuffer(pString.data(), (strMessage.length()+1)*sizeof(char));
  ascii.Encrypt_Buffer((unsigned char *)GEX_CRYPTO_KEY, GEX_KEY_BYTES);
  ascii.GetBufferToHexa(&szCryptedString);

  // Send command line server
  os << szCryptedString << "\n";
  free(szCryptedString);
}

///////////////////////////////////////////////////////////
// Read + decrypt string received from client
///////////////////////////////////////////////////////////
QString ClientSocket::ReadLineFromSocket(void)
{
  CCryptoFile ascii;	// Buffer used for crypting data!
  char*		szAsciiKey;
  QString		strMessage;
  QByteArray	pString;

  strMessage = m_pTcpSocket->readLine();
  strMessage = strMessage.trimmed();	// Remove leading '\n'
  pString = strMessage.toLatin1();
  ascii.SetBufferFromHexa(pString.data());
  ascii.Decrypt_Buffer((unsigned char *)GEX_CRYPTO_KEY,GEX_KEY_BYTES);
  ascii.GetBuffer(&szAsciiKey);

  QString strAsciiKey = szAsciiKey;
  free(szAsciiKey);
  return strAsciiKey;
}

void ClientSocket::OnReadyRead()
{
  QString strClientMessage, strReply;
  int		i,j;
  QString	strLogEventMessage;
  QString strPlatform;
  QString	strComputer;
  QString	strClientUser;
  QString	strClientGexFullAppName;
  QString strClientId;
  QString strMessage;
  int		iVersionMaj=-1;
  int		iVersionMin=-1;

  // Trace message
  WriteDebugMessageFile("Received data on socket connection...");

  // Read all lines sent by client node
  while(m_pTcpSocket->canReadLine())
  {

    // Read one line (decrypt it) received from client
    strClientMessage = ReadLineFromSocket();

    // Check command type
    if(strClientMessage.startsWith("GEX_RequestLicense"))	// LEGACY (< V6.0)
    {
      // Trace message
      strMessage = "New client message: GEX_RequestLicense...";
      WriteDebugMessageFile(strMessage);
      // Command: "GEX_RequestLicense;<GMT_Time>;<ComputerPlatform>;<ComputerName>;<UserName>"

      // Extract GMt time of request, string is: YYYY MM DD HH MM
      tm t_GmtRequestTime;
      tm *pt_GmtServerTime;
      time_t t_ServerTime;
      bool bBadTimeZone=false;
      strReply = strClientMessage.section(';',1,1);
      t_GmtRequestTime.tm_year = strReply.section(' ',0,0).toInt();	// Years (Year-1900 in fact!)
      t_GmtRequestTime.tm_mon  = strReply.section(' ',1,1).toInt();	// Months (Month-1 in fact!)
      t_GmtRequestTime.tm_mday = strReply.section(' ',2,2).toInt();	// Days
      t_GmtRequestTime.tm_hour = strReply.section(' ',3,3).toInt();	// Hours
      t_GmtRequestTime.tm_min  = strReply.section(' ',4,4).toInt();	// Minutes

      // Extract client node info...normalize 'remove spaces)
      strPlatform = strClientMessage.section(';',2,2);
      strPlatform = g_normalize_text(strComputer);
      strComputer = strClientMessage.section(';',3,3);
      strComputer = g_normalize_text(strComputer);
      strClientUser = strClientMessage.section(';',4,4);
      strClientUser = g_normalize_text(strClientUser);

      // Server time
      time ( &t_ServerTime );
      pt_GmtServerTime = gmtime ( &t_ServerTime );
      if(t_GmtRequestTime.tm_year != pt_GmtServerTime->tm_year)
        bBadTimeZone = true;
      if(t_GmtRequestTime.tm_mon != pt_GmtServerTime->tm_mon)
        bBadTimeZone = true;
      if(t_GmtRequestTime.tm_mday != pt_GmtServerTime->tm_mday)
        bBadTimeZone = true;
      long lServerMinutes,lClientMinutes;
      lServerMinutes = ((long)pt_GmtServerTime->tm_hour*60) + (long)pt_GmtServerTime->tm_min;
      lClientMinutes = ((long)t_GmtRequestTime.tm_hour*60) + (long)t_GmtRequestTime.tm_min;
      if(abs(lServerMinutes - lClientMinutes) > 90)
        bBadTimeZone = true;	// Allow up to 90 minutes error!

      // If license file doesn't have the flag to support all timezones,
      // Check if time difference between server and client is too big!...if so refuse license!
      if(!(iOptionalModules & 0x08) && (bBadTimeZone == true))
      {
        strReply = "GEX_REJECT_BADTIMEZONE";		// Server and Client have a time difference too big!

        // Log event: g_client_rejected <date_time> <computer> <user> "time_zone"
        strLogEventMessage = strComputer + " " + strClientUser + " time_zone";
        g_event("g_client_rejected",strLogEventMessage);
        goto send_reply;
      }

#if 0
      // Check platform type...Server and client must match!
      strReply = strClientMessage.section(';',2,2);
      if(strReply != strOSplatform)
      {
        strReply = "GEX_REJECT_BADOS";		// Server and Client not running under same type of OS....
        // Log event: g_client_rejected <date_time> <computer> <user> bad_os
        strLogEventMessage = strComputer + " " + strClientUser + " bad_os";
        g_event("g_client_rejected",strLogEventMessage);
        goto send_reply;
      }
#endif

      if(iLicensesUsed >= iMaximumLicenses)
      {
        strReply = "GEX_REJECT_ALLUSED";	// All licenses already in use.
        // Log event: g_client_rejected <date_time> <computer> <user> all_lic_used <max_licenses>
        strLogEventMessage = strComputer + " " + strClientUser + " all_lic_used " + QString::number(iMaximumLicenses);
        g_event("g_client_rejected",strLogEventMessage);
        goto send_reply;
      }

      iLicensesUsed++;

      // Build OK string: "GEX_ACCEPT ; YYYY MM DD (License Expiration date) ; ProductID ; Edition ; MaxMonitorProducts ; PluginSupport"
      strReply = "GEX_ACCEPT ;";			// Accept license request.
      strReply += strExpiration;			// YYYY MM DD expiration date.
      strReply += " ; " + QString::number(iProductID);	// ProductID. 0=Examinator, 4=ExaminatorDB,...
      strReply += " ; ";
      strReply += QString::number(iEditionType);		// Edition: 0=Standard, 1=Professional
      strReply += " ; ";
      strReply += QString::number(iMonitorProducts);	// Maximum ProductsIDs allowed in Monitoring (-1 = unlimited)
      strReply += " ; ";
      strReply += QString::number(iOptionalModules);	// Bit0=Plugin support, Bit1=PAT support,Bit2=Y123Web mode,Bit3=AllTimezones accepted

      // YIELDMANDB
      // Read YieldManDb Settings if exist
      strYieldManDbConnection = LoadYieldManDbSettings();

      if(!strYieldManDbConnection.isEmpty())
        strReply += " ; " + strYieldManDbConnection;
      // YIELDMANDB

      // Create + Add client entry to the list.
      ClientNode *pNewClient;
      pNewClient = new ClientNode();
      pNewClient->LastHandshake = QDateTime::currentDateTime();
      pNewClient->strComputer = strClientMessage.section(';',3,3);
      pNewClient->strUser = strClientMessage.section(';',4,4);
      this->m_dtSessionStart = QDateTime::currentDateTime();
      pNewClient->m_lstClientSocket.append(this);

      // Save structure into list
      pClientNodes.append(pNewClient);

      // Log event : g_client_start
      g_event("g_client_start", pNewClient->BuildLogMessage(this));
      goto send_reply;
    }

    if(strClientMessage.startsWith("GEX_RequestAdvLicense"))	// NEW (>= V6.0)
    {
      // Trace message
      strMessage = "New client message: GEX_RequestAdvLicense...";
      WriteDebugMessageFile(strMessage);
      // Command: "GEX_RequestAdvLicense;<Local_Time>;<VersionMaj>;<VersionMin>;<ComputerPlatform>;<ComputerName>;<UserName>;<GexFullAppName>;<ClientSignature>"

      // Extract Client local time of request, string is: YYYY MM DD HH MM
      int nYear, nMonth, nDay, nHour, nMin;
      strReply = strClientMessage.section(';',1,1);
      nYear = strReply.section(' ',0,0).toInt();		// Year (yyyy)
      nMonth  = strReply.section(' ',1,1).toInt();	// Months
      nDay = strReply.section(' ',2,2).toInt();		// Days
      nHour = strReply.section(' ',3,3).toInt();		// Hours
      nMin  = strReply.section(' ',4,4).toInt();		// Minutes

      // Extract client node info...normalize 'remove spaces)
      iVersionMaj = strClientMessage.section(';',2,2).toInt();
      iVersionMin = strClientMessage.section(';',3,3).toInt();
      strPlatform = strClientMessage.section(';',4,4);
      strPlatform = g_normalize_text(strPlatform);
      strComputer = strClientMessage.section(';',5,5);
      strComputer = g_normalize_text(strComputer);
      strClientUser = strClientMessage.section(';',6,6);
      strClientUser = g_normalize_text(strClientUser);
      strClientGexFullAppName = strClientMessage.section(';',7,7);
      strClientGexFullAppName = g_normalize_text(strClientGexFullAppName);
      strClientId = strClientMessage.section(';',8,8);
      strClientId = g_normalize_text(strClientId);

      // Check Examinator version
      // Check if user has a valid maintenance contract...
      bool bRejectUser=false;
      if((iMaxVersion_Maj >= 0) && (iMaxVersion_Min >= 0))
      {
        // Check validity X.Y
        if(iVersionMaj < iMaxVersion_Maj)
          bRejectUser=false;
        else if(iVersionMaj > iMaxVersion_Maj)
          bRejectUser=true;
        else
          // Major version 'X' matching... check if minor version
          bRejectUser = (iVersionMin > iMaxVersion_Min);
      }
      if(bRejectUser)
      {
        strReply = "GEX_REJECT_MAINTENANCE_CONTRACT";

        // Log event: g_client_rejected <date_time> <computer> <user> "maintenance_contract"
        strLogEventMessage = strComputer + " " + strClientUser + " maintenance_contract";
        g_event("g_client_rejected",strLogEventMessage);
        goto send_reply;
      }

      // Check time shift (GEX <-> GEX-LM)
      bool		bBadTimeZone=false;
      QDateTime	clServerTime=QDateTime::currentDateTime();
      QDateTime	clRequestTime(QDate(nYear, nMonth, nDay),QTime(nHour, nMin));
      int			nTimeShift = abs(clServerTime.secsTo(clRequestTime));
      // For debug /////////////////////////////////////////////////
      QString		strDateTime;
      strDateTime = clServerTime.toString("dd MM yyyy hh:mm:ss");
      strDateTime = clRequestTime.toString("dd MM yyyy hh:mm:ss");
      //////////////////////////////////////////////////////////////
      nTimeShift = nTimeShift/60;
      if(nTimeShift > iAllowedTimeShift)
        bBadTimeZone = true;

      // If license file doesn't have the flag to support all timezones,
      // Check if time difference between server and client is too big!...if so refuse license!
      if(!(iOptionalModules & 0x08) && (bBadTimeZone == true))
      {
        strReply = "GEX_REJECT_BADTIMEZONE";		// Server and Client have a time difference too big!

        // Log event: g_client_rejected <date_time> <computer> <user> "time_zone"
        strLogEventMessage = strComputer + " " + strClientUser + " time_zone";
        g_event("g_client_rejected",strLogEventMessage);
        goto send_reply;
      }

#if 0
      // Check platform type...Server and client must match!
      strReply = strClientMessage.section(';',2,2);
      if(strReply != strOSplatform)
      {
        strReply = "GEX_REJECT_BADOS";		// Server and Client not running under same type of OS....
        // Log event: g_client_rejected <date_time> <computer> <user> bad_os
        strLogEventMessage = strComputer + " " + strClientUser + " bad_os";
        g_event("g_client_rejected",strLogEventMessage);
        goto send_reply;
      }
#endif
      // Retrieve other running instance of the user
      ClientNode *pClientNode = clientNodeIdentifiedBy(strClientId);

      // If the user has no other running instance
      if(!pClientNode && (iLicensesUsed >= iMaximumLicenses))
      {
        strReply = "GEX_REJECT_ALLUSED";	// All licenses already in use.
        // Log event: g_client_rejected <date_time> <computer> <user> all_lic_used <max_licenses>
        strLogEventMessage = strComputer + " " + strClientUser + " all_lic_used " + QString::number(iMaximumLicenses);
        g_event("g_client_rejected",strLogEventMessage);
        goto send_reply;
      }

      // If the client does not use any node create one
      if (!pClientNode)
      {
        pClientNode = new ClientNode();
        iLicensesUsed++;
        // Save structure into list
        pClientNodes.append(pClientNode);
      }

      // Build OK string: "GEX_ACCEPT ; YYYY MM DD (License Expiration date) ; ProductID ; Edition ; MaxMonitorProducts ; PluginSupport"
      strReply = "GEX_ACCEPT ;";			// Accept license request.
      strReply += strExpiration;			// YYYY MM DD expiration date.
      strReply += " ; " + QString::number(iProductID);	// ProductID. 0=Examinator, 4=ExaminatorDB,...
      strReply += " ; ";
      strReply += QString::number(iEditionType);		// Edition: 0=Standard, 1=Professional
      strReply += " ; ";
      strReply += QString::number(iMonitorProducts);	// Maximum ProductsIDs allowed in Monitoring (-1 = unlimited)
      strReply += " ; ";
      strReply += QString::number(iOptionalModules);	// Bit0=Plugin support, Bit1=PAT support,Bit2=Y123Web mode,Bit3=AllTimezones accepted

      // YIELDMANDB
      // Read YieldManDb Settings if exist
      strYieldManDbConnection = LoadYieldManDbSettings();

      if(!strYieldManDbConnection.isEmpty())
        strReply += " ; " + strYieldManDbConnection;
      // YIELDMANDB

      // Create + Add client entry to the list.
      pClientNode->LastHandshake = QDateTime::currentDateTime();
      pClientNode->strComputer = strComputer;
      pClientNode->strUser = strClientUser;
      pClientNode->m_strClientId = strClientId;
      this->m_dtSessionStart = QDateTime::currentDateTime();
      this->m_strClientAppName = strClientGexFullAppName;
      pClientNode->m_lstClientSocket.append(this);

      // Log event : g_client_start
      g_event("g_client_start", pClientNode->BuildLogMessage(this));
      goto send_reply;
    }

    if(strClientMessage.startsWith("GEX_GetUsers") == true)
    {
      // Trace message
      strMessage = "New client message: " + strClientMessage;
      WriteDebugMessageFile(strMessage);

      // Returns the network status + license info + list of nodes currently using GEX...
      strReply = "GEX_NETWORKSTATUS;";
      strReply += QString::number(iLicensesUsed);		// Licenses used
      strReply += ";";
      strReply += QString::number(iMaximumLicenses);	// Max licenses allowed
      strReply += ";";
      strReply += QString::number(iProductID);		// Product: 0=Examinator, 4=ExaminatorDB,...
      // Send back string to client (crypted string).
      WriteLineToSocket(strReply);

      strReply = "GEX_LICENSEINFO;";
      strReply += szAppFullName;		// License Manager Name/Version
      strReply += ";";
      strReply += strExpiration;		// License expiration date
      strReply += ";";
      strReply += strLicenseID;		// License Key ID
      strReply += ";";

      // Send back string to client (crypted string).
      WriteLineToSocket(strReply);

      ClientNode *pClient;
      for(i=0; i<pClientNodes.size(); i++)
      {
        pClient = pClientNodes.at(i);
        for(j = 0; j < pClient->m_lstClientSocket.size(); j++)
        {
          strReply = "GEX_NODESTATUS;";
          strReply += pClient->strUser;
          strReply += ";";
          strReply += pClient->strComputer;
          strReply += ";";
          // Compute session time
          QDateTime dtNow = QDateTime::currentDateTime();
          int	iTime,iSec,iMin,iHours;
          iTime = pClient->m_lstClientSocket.at(j)->sessionStartDate().secsTo(dtNow);
          iHours = iTime / 3600;
          iTime = iTime % 3600;
          iMin = iTime / 60;
          iTime = iTime % 60;
          iSec = iTime;
          strReply += QString::number(iHours).rightJustified(2, '0');
          strReply += ":";
          strReply += QString::number(iMin).rightJustified(2, '0');
          strReply += ":";
          strReply += QString::number(iSec).rightJustified(2, '0');
          strReply += ";";
          strReply += pClient->m_lstClientSocket.at(j)->clientAppName();
          strReply += ";";
          strReply += QString::number(i + 1); // Node Id

          // Send back string to client (crypted string).
          WriteLineToSocket(strReply);
        }
      };
      strReply = "GEX_OK";
      goto send_reply;
    }

    if(strClientMessage.startsWith("GEX_NodeAlive") == true)
    {
      // A GEX node is telling server that it is still alive...
      ClientNode *pClient;
      strReply = "GEX_OK";
      for(i=0; i<pClientNodes.size(); i++)
      {
        pClient = pClientNodes.at(i);
        if(pClient->m_lstClientSocket.indexOf(this) != -1)
        {
          // Update time stamp of last handshake...
          pClient->LastHandshake = QDateTime::currentDateTime();

          // Exit the for loop
          break;
        }
      };
      goto send_reply;
    }

    if(strClientMessage.startsWith("GEX_GetHistory") == true)
    {
      QStringList strHistoryList;
      QString		strFilter;
      QDate		dateBegin, dateEnd;

      // Extract the begin date field
      strFilter	= strClientMessage.section(';',1,1);
      dateBegin	= QDate::fromString(strFilter, Qt::ISODate);

      // Extract the end date field
      strFilter	= strClientMessage.section(';',2,2);
      dateEnd		= QDate::fromString(strFilter, Qt::ISODate);

      // Extract the list of history data to send to client
      ReadHistoryDataFromLogFile(strHistoryList, dateBegin, dateEnd);

      // Send a start message to the client
      strReply = "GEX_HISTORYSTART;";
      strReply += QString::number(strHistoryList.count());

      for (int nData = 0; nData < strHistoryList.count(); nData++)
      {
        strReply = "GEX_HISTORYDATA;";
        strReply += strHistoryList.at(nData);

        WriteLineToSocket(strReply);
      }

      // Send a stop message to the client
      strReply = "GEX_HISTORYSTOP;";
      goto send_reply;
    }

    // YIELDMANDB
    if(strClientMessage.startsWith("GEX_YieldManDbSettings") == true)
    {
      // First YM admin client sent the YieldManDb settings
      // Gex-lm has to save it and send it for all other client connections
      WriteYieldManDbSettings(strClientMessage.section(";",1,1));
      return;
    }
    // YIELDMANDB

    // Unknown command.
    strReply = "GEX_BAD_COMMAND";

send_reply:
    // Trace message
    WriteDebugMessageFile("Sending reply to client...");

    // Send back string to client (crypted string).
    WriteLineToSocket(strReply);
  };
}

///////////////////////////////////////////////////////////
// Return Client identified by strClientId, NULL if not exist
///////////////////////////////////////////////////////////
ClientNode *clientNodeIdentifiedBy(QString strClientId)
{
  if (strClientId.isEmpty())
    return NULL;

  ClientNode *pMatchingNode = NULL;
  ClientNode *pCurrentNode = NULL;
  bool bClientFound = false;

  QListIterator<ClientNode *> iter(pClientNodes);
  while (iter.hasNext() && !bClientFound)
  {
    pCurrentNode = iter.next();
    if (pCurrentNode->m_strClientId == strClientId)
    {
      bClientFound = true;
      pMatchingNode = pCurrentNode;
    }
  }

  return pMatchingNode;
}

///////////////////////////////////////////////////////////
// Client closed connection...
///////////////////////////////////////////////////////////
void ClientSocket::OnDisconnectSocket()
{
  // Trace message
  QString strMessage;
  strMessage =  "Closing socket connection from host "  + m_pTcpSocket->peerAddress().toString();
  strMessage += " using port " + QString::number(m_pTcpSocket->peerPort());
  strMessage += ".";
  WriteDebugMessageFile(strMessage);

  bool bSocketFound = false;
  QListIterator<ClientNode *> iterClientNode(pClientNodes);
  ClientNode* pCurrentNode = NULL;

  // Check all Client nodes
  while (!bSocketFound && iterClientNode.hasNext())
  {
    pCurrentNode = iterClientNode.next();
    int iSocketIndex = pCurrentNode->m_lstClientSocket.indexOf(this);

    // If socket found in this client node
    if (iSocketIndex != -1)
    {
      bSocketFound = true;

      // Remove client socket for user list
      pCurrentNode->m_lstClientSocket.removeAt(iSocketIndex);

      // If it's the last connection socket of the user
      if (pCurrentNode->m_lstClientSocket.size() == 0)
      {
        // One GEX client closed...update license counter.
        iLicensesUsed--;

        // Log event: g_client_stop
        g_event("g_client_stop",pCurrentNode->BuildLogMessage(this));

        // Remove & Free entry
        WriteDebugMessageFile("Remove ClientNode: " + pCurrentNode->strUser );

        pClientNodes.removeAt(pClientNodes.indexOf(pCurrentNode));
        pCurrentNode->deleteLater();
      }
    }
  }

  // Delete ressources
  m_pTcpSocket->deleteLater();
}

//////////////////////////////////////////////////////////////
// Read log files and extract datas that match with filters...
//////////////////////////////////////////////////////////////
void ClientSocket::ReadHistoryDataFromLogFile(QStringList& strHistoryList, const QDate& dateBegin, const QDate& dateEnd)
{
  // Create log file name: <user_folder>//gex-lm_<year>_<month>.log
  QDate	dateFile(dateBegin.year(), dateBegin.month(), 1);
  QDate	dateLine;
  QString strLine;
  QString strFile;

  while (dateFile <= dateEnd)
  {
    // Build the path of the log file
    strFile	= strLocalFolder + "/gex-lm_" + dateFile.toString("yyyy_MM") + ".log";

    // Read the log file
    QFile file(strFile);
    if (file.open(QIODevice::ReadOnly) == true)
    {
      QTextStream hFile;
      hFile.setDevice(&file);	// Assign file handle to data stream

      do
      {
        // Read one line from file
        strLine = hFile.readLine();

        // Extract the date from the log line
        dateLine = QDateTime::fromString(strLine.section(' ',1,1), "dd-MM-yyyy_hh:mm:ss").date();

        // History date matches with date filter
        if (dateLine >= dateBegin && dateLine <= dateEnd)
          strHistoryList << strLine;
      }
      while(hFile.atEnd() == false);
      file.close();
    }

    // Go to the next month
    dateFile = dateFile.addMonths(1);
  }
}


//////////////////////////////////////////////////////////////////////
// Read YieldManDb settings
//////////////////////////////////////////////////////////////////////
QString ClientSocket::LoadYieldManDbSettings()
{

  QString strFile;
  strFile	= strLocalFolder + "/.gexlm_config.xml";
  WriteDebugMessageFile("Read the gexlm config file = " + strFile);

  // Open settings file
  QFile clSettingsFile(strFile);
  if(!clSettingsFile.open(QIODevice::ReadOnly))
    return "GEX_YieldManDbSettings|NotConfigured";	// Failed opening file.

  // Assign file I/O stream
  QXmlStreamReader stream(&clSettingsFile);

  QString	strKeyword;
  QString	strParameter;

  QString strHostName;
  QString strDriver;
  QString strDatabaseName;
  QString strUserName;
  QString strPassword;
  QString strSchemaName;
  QString strPort;


  while (!stream.atEnd())
  {
    stream.readNext();
    if(stream.isStartElement() && (stream.name().compare("YieldManDatabase",Qt::CaseInsensitive) == 0))
    {
      while (!stream.atEnd())
      {
        stream.readNext();
        if(stream.isStartElement() && (stream.name().compare("Connection",Qt::CaseInsensitive) == 0))
        {
          foreach(QXmlStreamAttribute attribute, stream.attributes() )
          {
            strKeyword = attribute.name().toString();
            strParameter = attribute.value().toString();
            if(strKeyword.toLower() == "host")
              strHostName = strParameter;
            else
            if(strKeyword.toLower() == "driver")
              strDriver = strParameter;
            else
            if(strKeyword.toLower() == "databasename")
              strDatabaseName = strParameter;
            else
            if(strKeyword.toLower() == "username")
              strUserName = strParameter;
            else
            if(strKeyword.toLower() == "userpassword")
              strPassword = strParameter;
            else
            if(strKeyword.toLower() == "schemaname")
              strSchemaName = strParameter;
            else
            if(strKeyword.toLower() == "port")
              strPort = strParameter;
          }
        }
      }
    }
  }

  clSettingsFile.close();

  if(strHostName.isEmpty())
    return "GEX_YieldManDbSettings|NotConfigured";	// Failed reading file.

  strYieldManDbConnection = strHostName + "|";
  strYieldManDbConnection+= strDriver + "|";
  strYieldManDbConnection+= strDatabaseName + "|";
  strYieldManDbConnection+= strUserName + "|";
  strYieldManDbConnection+= strPassword + "|";
  strYieldManDbConnection+= strSchemaName + "|";
  strYieldManDbConnection+= strPort;

  return strYieldManDbConnection;
}

///////////////////////////////////////////////////////////
// Write YieldManDb settings
///////////////////////////////////////////////////////////
void ClientSocket::WriteYieldManDbSettings(QString strConnection)
{

  QString strHostName;
  QString strDriver;
  QString strDatabaseName;
  QString strUserName;
  QString strPassword;
  QString strSchemaName;
  QString strPort;

  if(strConnection.count("|") < 6)
    return;

  strHostName		= strConnection.section("|",0,0);
  strDriver		= strConnection.section("|",1,1);
  strDatabaseName	= strConnection.section("|",2,2);
  strUserName		= strConnection.section("|",3,3);
  strPassword		= strConnection.section("|",4,4);
  strSchemaName	= strConnection.section("|",5,5);
  strPort			= strConnection.section("|",6,6);

  QString strFile;
  strFile	= strLocalFolder + "/.gexlm_config.xml";
  WriteDebugMessageFile("Write the gexlm config file = " + strFile);

  // Open settings file
  QFile clSettingsFile(strFile);
  if(!clSettingsFile.open(QIODevice::WriteOnly))
    return;	// Failed creating file.

  QXmlStreamWriter stream( &clSettingsFile);
  stream.setAutoFormatting(true);
  stream.writeStartDocument();

  // Start definition marker
  stream.writeStartElement("YieldManDatabase");

  // Fill file with Database details...
  stream.writeEmptyElement("Connection");
  stream.writeAttribute("Host",strHostName);
  stream.writeAttribute("Driver",strDriver);
  stream.writeAttribute("DatabaseName",strDatabaseName);
  stream.writeAttribute("UserName",strUserName);
  stream.writeAttribute("UserPassword",strPassword);
  stream.writeAttribute("SchemaName",strSchemaName);
  stream.writeAttribute("Port",strPort);


  // End definition marker
  stream.writeEndElement();
  stream.writeEndDocument();

  // Clean close
  clSettingsFile.close();

  // Success
  return;
}
