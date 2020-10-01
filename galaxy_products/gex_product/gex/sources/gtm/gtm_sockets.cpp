#include <gqtl_log.h>
#include <gtl_core.h> // to check GTL version
#include "browser_dialog.h"
#include "engine.h"
#include "gtm_sockets.h"
#include "gtm_mainwidget.h"
#include "clientsocket.h"


// global variables
extern GexMainwindow  *pGexMainWindow;

///////////////////////////////////////////////////////////
//  The SimpleServer class handles new connections to the server. For every
//  client that connects, it creates a new ClientSocket -- that instance is now
//  responsible for the communication with that client.
///////////////////////////////////////////////////////////
SimpleServer::SimpleServer( QObject* parent, int nPortNumber ) : Q3ServerSocket( nPortNumber, 1, parent )
{
    GSLOG(5, "new SimpleServer");
    // Check if port already used/or GEXLM already running!
    if (!ok())
    {
        QString strMessage = "Failed to bind to port " + QString::number(nPortNumber)
                + "...\nPort may be used by another application, or 'gtm' already running";
        GSLOG(4, strMessage.toLatin1().constData());
        //exit(1);
    }
}

///////////////////////////////////////////////////////////
// A client just connected!
///////////////////////////////////////////////////////////
void SimpleServer::newConnection( int socket )
{
    (void)new GS::Gex::ClientSocket(socket, this);
}
