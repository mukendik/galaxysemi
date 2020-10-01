#ifndef VFEI_SERVER_H
#define VFEI_SERVER_H

#include <QTcpServer>

namespace GS
{
namespace Gex
{

class VFEIServerPrivate;

/*! \class  VFEIServer
    \brief  The TcpServer class handles new connections to the server. When the
            client connects, it creates a new VFEIClient -- that instance is now
            responsible for the communication with that client.
*/

class VFEIServer : protected QTcpServer
{
    Q_OBJECT

public:

    /*!
      @brief    Constructs a VFEI server object

      @param    parent Pointer to the parent object
      */
    VFEIServer(QObject * parent = NULL);

    /*!
      @brief    Destroys the VFEIServer object. If the server is running, the socket
                is automatically closed.

                Any client \l{VFEIClient}s that are still connected are automatically disconnected.

      @sa       Stop
    */
    virtual ~VFEIServer();

    /*!
      @brief    Initialize the VFEI server by loading the configuration file \a lConfigFile.

      @param    lConfigFile     File name of the VFEI configuration file
      */
    bool    Init(const QString& lConfigFile);

    /*!
      @brief    Starts the server in order to listen for incoming connections.

      @returns  True on success; otherwise returns false.

      @sa       Stop, IsRunning
      */
    bool    Start();

    /*!
      @brief    Stops the server. The server will no longer listen for incoming
                connections.

      @sa       Start
      */
    void    Stop();

    /*!
      @brief    Returns true if the server is currently listening for incoming
                connections; otherwise returns false.

      @sa       Start
      */
    bool    IsRunning() const;

private:

    Q_DISABLE_COPY(VFEIServer)

    VFEIServerPrivate * mPrivate;

private slots:

    /*!
      @brief    Called when a client connects to the VFEI Server.
      */
    void	OnNewConnection();
};

}   // namespace Gex
}   // namespace GS

#endif // VFEI_SERVER_H
