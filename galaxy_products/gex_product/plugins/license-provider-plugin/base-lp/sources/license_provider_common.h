#include <QString>
#include <QObject>
#include <QMetaType>

#include "license_provider_global.h"


#ifndef LICENSE_PROVIDER_COMMON_H
#define LICENSE_PROVIDER_COMMON_H

namespace GS
{
  namespace LPPlugin
  {
    //A class defining  the message sent by gex to lp to sk/answer a request
    class LICENSE_PROVIDERSHARED_EXPORT GEXMessage : public QObject
    {
        Q_OBJECT
    public:
      enum MessageType
      {
        eUknown             = 0, // default message type
        eLicenseRequest     = 1, // message defining a license request asked by gex
        eActive             = 2, // message sent periodically to indicate that gex is active
        eDisconnect         = 3, // message sent by gex to disconnect from the license server
        eGetYMAdminDBConfig = 4, // message sent by gex to ask for the ym_admin_db config from the vendor deamon
        eSetYMAdminDBConfig = 5, // message sent by gex and containing the ym_admin_db config for the the vendor deamon
        eReconnect          = 6, // message sent to reconnect to the license server
        eExit               = 7, // message sent when gex is exiting
        eExtended           = 8 // message to sent custom request from  gex
      };

      GEXMessage(MessageType messageType, const QString &data);
      GEXMessage();
      GEXMessage(const GEXMessage &other);
      virtual ~GEXMessage();
      MessageType getType() const;
      QString getData() const;
    protected:
      // The message type
      MessageType mType;
      // The message data if any additional info is needed to be specified
      QString mData;
    };

    //This is the message sent by lp to gex to ask/answer a request
    class LICENSE_PROVIDERSHARED_EXPORT LPMessage : public QObject
    {
        Q_OBJECT
    public:
      enum MessageType
      {
        eUknown             = 0, // default message type
        eAccept             = 1, // message sent to gex to accord a license
        eReject             = 2, // message sent to gex to refuse a license
        eDisconnected       = 3, // message sent to gex to confirm the license disconnection
        eGetYMAdminDBConfig = 4, // message sent to gex containing the ym_admin_db config
        eSetYMAdminDBConfig = 5, // message sent to gex to configure the ym_admin_db
        eExtended           = 6 // message to sent custom request from  lp
      };

      LPMessage(MessageType type, const QString &data);
      LPMessage();
      LPMessage(const LPMessage &other);
      LPMessage& operator=(const LPMessage &other);
      virtual ~LPMessage();
      MessageType getType() const;
      QString getData() const;
      void setType(MessageType ) ;
      void setData(const QString &) ;

    protected:
      // The message type
      MessageType mType;
      // The message data if any additional info is needed to be specified
      QString mData;
    };
  }
}

Q_DECLARE_METATYPE(GS::LPPlugin::LPMessage);
Q_DECLARE_METATYPE(GS::LPPlugin::GEXMessage);


#endif // LICENSE_PROVIDER_COMMON_H
