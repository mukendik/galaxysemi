#ifndef JSONSERVERFILEDESCRIPTORIO_H
#define JSONSERVERFILEDESCRIPTORIO_H

#include <QJsonDocument>
#include "server_file_descriptorIO.h"

namespace GS
{
    namespace LPPlugin
    {

        /**
         * @brief The ServerDescriptor struct (serverIP, name, port...)
         */
        struct ServerDescriptor {
                int                         mSocketPort;
                int                         mOrder;
                QString                     mServerIP;
                QString                     mServerName;
                QString                     mUserName;
                QString                     mProductKey;
        };

        class LicenseTypeDescriptor;
        class ProviderDescriptor ;
        class JSONServerFileDescriptorIO : public ServerFileDescriptorIO {

            public:

                JSONServerFileDescriptorIO          ();
                ~JSONServerFileDescriptorIO         ();

                bool                        LoadFile                (const QString &fileName, QString forceProvider="");
              //  void                        updateConnectionStatus  (bool connectionStatus);

                bool                        GetConnectionStatus     (QString providerType = "", T_LicenseType licenseType =  undef) const;
               // const QString&              getLabel                (QString providerType = "", T_LicenseType licenseType =  undef) const ;
                const QVector<int>&         GetListSocketPorts      (QString providerType = "", T_LicenseType licenseType =  undef) const ;
                const QVector<QString>&     GetListServerIP         (QString providerType = "", T_LicenseType licenseType =  undef) const ;
                T_LicenseType               GetLastUsedLicenseType  (QString providerType) const;
                const QString &             GetUserName             (QString providerType= "", T_LicenseType licenseType =  undef) const;
                const QString &             GetProductKey           (QString providerType= "", T_LicenseType licenseType =  undef) const;


            private:

                bool                        LoadProviderDescriptor      (QJsonObject &jsonObject);
                bool                        LoadServerDescriptor        (QJsonObject &jsonObject, LicenseTypeDescriptor *lLicenseTypeDescriptor);
                bool                        CheckValidityOfRequestedData(QString &providerType, T_LicenseType &licenseType) const ;
                void                        BuildServerDescriptorString ();
                void                        UpdateJsondocument          (QJsonDocument& lJsonDocument, const QString &providerType, T_LicenseType licenseType, const QVector<QString> &serverIPs, const QVector<int> &datas2);
                bool                        SaveFile                    (const QString& providerType, T_LicenseType licenseType, const QVector<QString> &serverIPs, const QVector<int> &datas2);
                void                        UpdateConnectionStatus      (QJsonDocument& lJsonDocument, bool connectionStatus);

                /**
                 * @brief mDescribedServers. Key : the license provider type (fnl_lp or gs_lp);
                 */
                QMap<QString, const ProviderDescriptor* >               mProviders;

                friend class XMLServerFileDescriptorIO;


        };

        class ProviderDescriptor {

            public :
                ProviderDescriptor() ;
                ~ProviderDescriptor() ;

                void                                                        AddLicenseTypeDescriptor (T_LicenseType licenseType, const LicenseTypeDescriptor* serverDescriptor) { mLicenseTypeDescriptors.insert(licenseType, serverDescriptor);}
                const QMap<T_LicenseType, const LicenseTypeDescriptor*>&    GetLicenseTypeDescriptors() const { return mLicenseTypeDescriptors; }

                void                                                        RetLastLp               (T_LicenseType licenseType) { mLastLpUsed = licenseType;}
                T_LicenseType                                               GetLastLp               () const { return mLastLpUsed; }

            private :
                QMap<T_LicenseType, const LicenseTypeDescriptor*>           mLicenseTypeDescriptors;
                T_LicenseType                                               mLastLpUsed;
        };


        /**
         * @brief The ServerTypeDescriptor class. Either standlone, evalution or floating. May contais several ServerDescriptor
         */
        class LicenseTypeDescriptor {

            public :
                LicenseTypeDescriptor(T_LicenseType licenseType);
                ~LicenseTypeDescriptor();

                T_LicenseType                       GetLicenseType      () const                                        { return mLicenseType; }
                int                                 GetConnectionStatus () const                                        { return mConnectionStatus; }
                void                                SetConnectionStatus (int connectionStatus)                          { mConnectionStatus = connectionStatus;}
                void                                AddServerDescriptor (const ServerDescriptor& serverDescriptor)     ;

                const QVector<int>&                 GetListSocketPorts  () const { return mSocketPorts; }
                const QVector<QString>&             GetListServerIP     () const { return mServerIPs; }
                const QString&                      GetLabel            () const { return mLabel; }
                void                                SetLabel            (const QString& label) { mLabel = label;}
                const QString&                      GetLabelGUI         () const { return mLabelGUI ;}

            private:

                 QVector<QString>                   mServerIPs;
                 QVector<int>                       mSocketPorts;
                 QList<ServerDescriptor>            mServerDescriptors;
                 QString                            mLabel;
                 QString                            mLabelGUI;
                 int                                mConnectionStatus;
                 T_LicenseType                      mLicenseType;


                 void                               BuidLabelGUI        (const ServerDescriptor &serverDescriptor);

                 friend class JSONServerFileDescriptorIO;


        };

    }
}


#endif
