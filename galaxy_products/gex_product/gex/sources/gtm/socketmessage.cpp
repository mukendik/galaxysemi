#include <QObject>
#include "socketmessage.h"

namespace GS
{
namespace Gex
{
    unsigned SocketMessage::sNumOfInstances=0;

    SocketMessage::SocketMessage(Direction eDirection): QObject(0)
    {
        sNumOfInstances++;
        mDirection = eDirection;
        mMessageType = 0;
        mAckRequested = false;
        mMessage = NULL;
    }

    SocketMessage::~SocketMessage()
    {
        sNumOfInstances--;
        if((mDirection == eReceive) && (mMessage != NULL))
            free(mMessage);
    }

} // Gex
} // GS
