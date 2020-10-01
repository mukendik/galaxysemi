#ifndef SOCKETMESSAGE_H
#define SOCKETMESSAGE_H

#include <QObject>

namespace GS
{
namespace Gex
{
    // The SocketMessage class holds a message received through
    // the socket, or to be sent through the socket.
    /*! \class  ClientSocket
        \brief  The Client socket class inherit from TcpSocket and handle network communication with the tester server.
            The process is not to be done in this class but in the friend class ClientNode.
    */
    class SocketMessage : public QObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(SocketMessage)
        static unsigned sNumOfInstances;

        public:
        enum Direction
        {
            eSend,
            eReceive
        };

        SocketMessage(Direction eDirection);
        virtual ~SocketMessage();

        Direction		mDirection;
        unsigned int	mMessageType;
        bool            mAckRequested;
        void			*mMessage;
        public slots:
        static unsigned GetNumOfInstances() { return sNumOfInstances; }
    };

} // Gex
} // GS

#endif // SOCKETMESSAGE_H
