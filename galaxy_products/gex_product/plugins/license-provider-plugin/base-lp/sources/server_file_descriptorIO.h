#ifndef SERVERFILEDESCRIPTOR_H
#define SERVERFILEDESCRIPTOR_H

#include <QJsonDocument>
#include "license_provider_global.h"
#include <QString>
#include <QVector>
#include <QMap>

namespace GS
{
    namespace LPPlugin
    {

        const QString c_userName            =   "user_name";
        const QString c_port                =   "port";
        const QString c_host                =   "host";
        const QString c_productKey          =   "product_key";
        const QString c_servers             =   "servers";
        const QString c_lpConfig            =   "lp_config";
        const QString c_connectionStatus    =   "connection_status";
        const QString c_version             =   "version";
        const QString c_lastLpUsed          =   "last_lp_used";
        const QString c_lastLtUsed          =   "last_license_type_used";


        enum T_LicenseType {standalone = 0, evaluation, floating, undef};

        /**
         * @brief The ServerFileDescriptorIO class
         * base class for laoding and saving a servers descriptor file
         */
        class LICENSE_PROVIDERSHARED_EXPORT ServerFileDescriptorIO {
    		
    		public:

                ServerFileDescriptorIO           ();
                virtual ~ServerFileDescriptorIO  () ;

                // -- functions
                virtual bool                            LoadFile                        (const QString &fileName, QString forceProvider="") = 0;
                bool                                    SaveFile                        (const QString& fileName, const QString& providerType, const QString& licenseTypeStr, const QString &field1, const QString &field2);
                void                                    UpdateConnectionStatus          (const QString& fileName, bool connectionStatus)    ;

                virtual bool                            GetConnectionStatus             (QString providerType = "", T_LicenseType licenseType =  undef) const = 0;
                //virtual const QString&                  getLabel                        (QString providerType = "", T_LicenseType licenseType =  undef) const = 0;
                virtual const QVector<int>&             GetListSocketPorts              (QString providerType = "", T_LicenseType licenseType =  undef) const = 0;
                virtual const QVector<QString>&         GetListServerIP                 (QString providerType = "", T_LicenseType licenseType =  undef) const = 0;
                virtual T_LicenseType                   GetLastUsedLicenseType          (QString providerType = "") const;
                virtual const QString &                 GetUserName                     (QString providerType= "", T_LicenseType licenseType =  undef) const = 0;
                virtual const QString &                 GetProductKey                   (QString providerType= "", T_LicenseType licenseType =  undef) const = 0;

                virtual bool                            ConvertAndSaveToJsonFormat      (const QString &fileName);

                //const QString&                          GetLastUsedProvider             () const { return mLastProviderConfigured;}
                const QString&                          GetVersion                      () const { return mVersion;}
                const QString&                          GetLastChoice                   () const { return mLastChoice;}
                bool                                    IsLoaded                        () const { return mLoaded;}
                /**
                 * @brief getServerDescriptorString
                 * @return list of string per provider. One provider may have several string description
                 */
                typedef QMultiMap<QString, QString>  T_Multimap;
                typedef T_Multimap::iterator         T_MultimapIter;

                const  T_Multimap &                    GetServerDescriptorsStrings      () const { return mServerDescriptorString;}
                const QString&                         GetServerDescriptorString        (const QString &provider, T_LicenseType licenseType) ;

                QString                                LicenseTypeToStr                 (T_LicenseType licenseType) const;

            protected :
                QJsonDocument                           mJsonDocument ;
                T_Multimap                              mServerDescriptorString;
                QString                                 mLastProviderConfigured;
                QString                                 mVersion;
                QString                                 mLastChoice;
                T_LicenseType                           mLastLicenseTypeUsed;
                bool                                    mLoaded;


                const QVector<QString>&                 GetEmptyServerIPs               () const { return mEmptyServerIPs;}
                const QVector<int>&                     GetEmptyServerPorts             () const { return mEmptyServerPorts;}
                const QString&                          GetEmptyString                  () const { return mEmptyString;}

                T_LicenseType                           StrToLicenseType                (const QString &licenseTypeStr) const;
                QString                                 BuildDefaultLicenseTypeLabel    (T_LicenseType licenseType) const;

                bool                                    ExtractServersPortFromString    (const QString &serversPortString, QVector<QString> &servers, QVector<int> &ports );


            private:
                QVector<QString>                        mEmptyServerIPs;
                QVector<int>                            mEmptyServerPorts;
                QString                                 mEmptyString;

                virtual bool                            SaveFile                        (const QString& providerType, T_LicenseType licenseType, const QVector<QString> &datas1, const QVector<int> &datas2) = 0;
                virtual void                            UpdateConnectionStatus          (QJsonDocument& lJsonDocument, bool connectionStatus) = 0;
                virtual bool                            ConvertToJson                   () { return true;}

        };
    }
}


#endif
