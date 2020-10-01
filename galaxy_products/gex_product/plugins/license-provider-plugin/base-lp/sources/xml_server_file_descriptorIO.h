#ifndef XMLSERVERFILEDESCRIPTORIO_H
#define XMLSERVERFILEDESCRIPTORIO_H

#include <QMap>
#include <QJsonDocument>
#include "server_file_descriptorIO.h"

namespace GS
{
    namespace LPPlugin
    {


        struct XMLServerDescriptor {

                QString                     mProvider;
                QString                     mServerName;
               // QString                     mLabel;
                // -- floating
                QVector<QString>            mServerIP;
                QVector<int>                mSocketPort;

                // -- standalone
                QString                     mUserName;
                QString                     mProductKey;

                bool                        mConnectionStatus;
                T_LicenseType               mLicenseType;
        };

        class XMLServerFileDescriptorIO: public ServerFileDescriptorIO {
    		
    		public:

                XMLServerFileDescriptorIO   () ;
                ~XMLServerFileDescriptorIO  () ;

                bool                        LoadFile                (const QString &fileName, QString forceProvider="");
                bool                        GetConnectionStatus     (QString providerType = "", T_LicenseType licenseType =  undef) const;

                const QVector<int>&         GetListSocketPorts      (QString providerType = "", T_LicenseType licenseType =  standalone) const ;
                const QVector<QString>&     GetListServerIP         (QString providerType = "", T_LicenseType licenseType =  standalone) const ;
                const QString &             GetUserName             (QString providerType= "", T_LicenseType =  standalone) const;
                const QString &             GetProductKey           (QString providerType= "", T_LicenseType =  standalone) const;

                bool                        ConvertToJson           ();
        private:
                QMap<QString, XMLServerDescriptor*>     mServerLicenseDescripors;
                bool                                    mConvertionDone;

                void                                    BuildServerLicenseDescriptorString  (const QString provider, const QStringList &lastChoiceItems, XMLServerDescriptor *lServerDescriptor, T_LicenseType licenseType);
                bool                                    XmlToJsonDocument                   (QJsonDocument &jsonDocument) const ;

                bool                                    SaveFile                            (const QString& providerType, T_LicenseType licenseType, const QVector<QString> &datas1, const QVector<int> &datas2);
                void                                    UpdateConnectionStatus              (QJsonDocument& lJsonDocument, bool connectionStatus);

        };
    
    }
}


#endif
