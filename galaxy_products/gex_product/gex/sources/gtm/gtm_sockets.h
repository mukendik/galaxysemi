/*

  REMOVE ME : old Qt3 mess

*/

#ifndef GTM_SOCKETS_H
#define GTM_SOCKETS_H

#include <QDateTime>
#include <Q3ServerSocket>
#include <Q3Socket>
#include <QTcpSocket>
#include <QObject>

// From Galaxy_std_libraries (gstdutils_c)
#include <gstdl_netmessage_c.h>

// From gtc
#include <gtc_netmessage.h>

#include "socketmessage.h"

#define	GTM_SERVER_PORT	4747		// Default Socket port#

///////////////////////////////////////////////////////////
//  The SimpleServer class handles new tester connections to the GTM (server). For every
//  client (tester station) that connects, it creates a new ClientSocket 
// -- that instance is now responsible for the communication with that client.
///////////////////////////////////////////////////////////
class SimpleServer : public Q3ServerSocket
{
public:
    SimpleServer( QObject* parent=0, int nPortNumber=GTM_SERVER_PORT );
    void newConnection( int socket );	// A client just connected!
};

#endif // GTM_SOCKETS_H
