#include <QMap>
#include <QDir>
#include <QObject>
#include <QLibrary>
#include <QMetaType>

#include "license_provider.h"

#ifndef LICENSE_PROVIDER_LOADER_H
#define LICENSE_PROVIDER_LOADER_H

namespace GS
{
  namespace LPPlugin
  {
    class LicenseProviderManagerPrivate;
    ////
    /// \brief The LicenseProviderManager class is used as link between gex and lp to process the license requests
    ///
    class LICENSE_PROVIDERSHARED_EXPORT LicenseProviderManager : public QObject
    {
        Q_OBJECT
        LicenseProviderManagerPrivate *mPrivate;
    protected:
        static LicenseProviderManager *mInstance;
        LicenseProviderManager(LicenseProvider::GexProducts product,
                               ProductInfo *productInfo,
                               const QVariantMap &appConfig,
                               QObject *parent = 0);
        virtual ~LicenseProviderManager();
        int initialize();
        int cleanup();
        virtual void setLastError(int code, const QString &message);
    public :
        static LicenseProviderManager *instanciate(LicenseProvider::GexProducts product,
                                                   ProductInfo *productInfo, const QVariantMap &appConfig,
                                                   int &errorCode, QString &error,
                                                   QObject *parent = 0);
        static LicenseProviderManager *getInstance();
        static void Destroy();
        static LicenseProvider *getCurrentProvider();
        static QString getLastLPUsed(const QString &path);
        void setCurrentProvider(const QString &);
        int getLastErrorCode();
        QString getLastError();
        QMap <QString , LicenseProvider *> &availableLP();
        QMap <QString , QLibrary *> &availableLibrary();
        class LicenseProviderThread *getLPThread ();
        QVariant GetAppConfig(const QString &key);
        void SetAppConfig(const QString &key, const QVariant &val);
        QVariant getLPData (const QString &key);
        void stopLicensingThread();
        void waitUntilThreadFinshed();

    signals:
        // Forward messages
        void forwardLPMessage(const GS::LPPlugin::LPMessage &);
        void forwardGexMessage(const GS::LPPlugin::GEXMessage &);

    public slots:
        // receive messages
        void receiveGexMessage(const GS::LPPlugin::GEXMessage &);
        void requestingLicenseEnd ();

    public slots: //USER interaction slots
        void userInteraction(GS::LPPlugin::LicenseProvider *provider, QString *, QString *EditField2,
                             int *selectionMode, bool &b, bool &result, bool legacy);
        void userChoice(const QStringList *choices, int &choice);
        // slot used to show a splash durring a background action that will conatins the message "message"
        //mode parameter will be used to hide or show the splash
        void ShowWaitingMessage(const QString &message, int mode);
        void userAvailableChoices(const QString *message, const QStringList *products,bool, QString *returnedVal);
        void userEvalLicense(const QString *message, QString *orderId);
        void notifyUser(const QString *);
        void notifyDaemon();
        void openURL(const QString *url);

    public:
        enum LoadingError
        {
            eNoError            = 0x01,
            eMissingInitData    = 0x02,
            ePluginDirNotFound  = 0x04,
            ePluginDirEmpty     = 0x08,
            eNoLPFound          = 0x10
        };

    };
  }
}

Q_DECLARE_METATYPE(GS::LPPlugin::LicenseProviderManager::LoadingError)


#endif // LICENSE_PROVIDER_LOADER_H
