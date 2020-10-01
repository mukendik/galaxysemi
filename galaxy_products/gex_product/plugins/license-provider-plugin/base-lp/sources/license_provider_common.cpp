#include "license_provider_common.h"

namespace GS
{
namespace LPPlugin
{

GEXMessage::GEXMessage(GEXMessage::MessageType messageType, const QString &data):QObject(), mType(messageType),mData(data)
{
}

GEXMessage::GEXMessage():mType(eUknown)
{}

GEXMessage::GEXMessage(const GEXMessage &other):QObject()
{
    mType = other.mType;
    mData = other.mData;
}

GEXMessage::~GEXMessage()
{}

GEXMessage::MessageType GEXMessage::getType() const
{
  return mType;
}

QString GEXMessage::getData() const
{
  return mData;
}

LPMessage::LPMessage(MessageType messageType, const QString &data): mType(messageType),mData(data)
{}

LPMessage::LPMessage():mType(eUknown)
{}

LPMessage::LPMessage(const LPMessage &other):QObject()
{
    mType = other.mType;
    mData = other.mData;
}


LPMessage::~LPMessage()
{}

LPMessage::MessageType LPMessage::getType() const
{
  return mType;
}

QString LPMessage::getData() const
{
  return mData;
}

void LPMessage::setType(LPMessage::MessageType type)
{
    mType = type;
}

void LPMessage::setData(const QString &data)
{
    mData = data;
}

LPMessage& LPMessage::operator=(const LPMessage& other)
{
    if (this != &other)
    {
        mType=other.mType;
        mData=other.mData;
    }

    return *this;

}


}
}
