#include "license_provider.h"
#include "product_info.h"
#include <QMetaType>


#ifndef FNP_LICENSE_PROVIDER_H
#define FNP_LICENSE_PROVIDER_H

class ProductInfo;

namespace GS
{
    namespace LPPlugin
    {
      class FNPLicenseProvider : public GS::LPPlugin::LicenseProvider
      {
        Q_OBJECT
        Q_DISABLE_COPY(FNPLicenseProvider)

      protected:
        FNPLicenseProvider(QObject* ,
                           GexProducts ,
                           const QVariantMap &);

        static FNPLicenseProvider* mInstance;
        class FNPLicenseProviderPrivate *mPrivate;
      public:
        virtual ~FNPLicenseProvider();
		
	  protected :
        virtual int initialize();
        virtual int cleanup();
        void processGexMessage(const GEXMessage &gexMessage);
        bool checkStartupArguments(ProductInfo *productInfo);

      public slots:
        // Slot used to reserve appropriate  increment from flexera license and update the productInfo attribute
        bool checkLicense(ProductInfo *productInfo);
        bool createEvaluationLicense(ProductInfo *productInfo);
        void checkActiveStatusUpdate();
        void launchActivation(const QString &AdditionalMessage, bool showDialog);
        //Slot used to luanch the evaluation activation flow
        void LaunchEvaluationActivation(bool &exitGex);
        void DirectRequest(const GEXMessage &gexMessage,LPMessage &lpMessage);
      public :
        static FNPLicenseProvider *initializeLP(QObject* parent, GexProducts product,
                                               const QVariantMap &appConfig,
                                               int &libErrorCode, QString &libError);
        static FNPLicenseProvider *getInstance();
        static void destroy();

      public:
        //Customization functions
        QVariantList getRunningModeAvailable();
        QString getWelcomePage();

        int getExtendedError(QString &m);
        void disconnectFromLM(const QString &m, const QString &disconnectedFeature);
        void reconnectToLM(const QString &m);

        QString buildUserNotificationMessage();

      protected:
        void setInternalError(int, const QString &);
        int getInternalError(QString &);

      private:
        bool RequestGTLToken(int library, QString &info);
        void ReleaseGTLToken(int library);

        bool reserveIncrement(const QString &feature, const QString &version, int nlic, char *internalInfo[]);
        void releaseIncrement(const QString &feature);

        bool reserve(ProductInfo *productInfo, const QStringList &increments);
        void release(const QStringList &increments);
        bool getMaintenanceExpirationDate(const QString &licenseType, QDate &lMaintenanceExprationDate);
        bool setYieldManDbSettings(const QString &setYMAdminDBConfig, QString &daemonError);
        bool getYieldManDbSettings(QString &getYMAdminDBConfig, QString &daemonError);
        QString clientDaemonQA(const QString &);

        QString getLicenseFiles(const QString &licensesDirPath);
        void notifyUserOnExpiration(const QDate &expirationDate);
        // Check if the Application and the FNP proxy are compatible and returned true in this case
        bool CheckAppAndFNPLibVersion();
        //Method used to download the flexera file based on the provided entitelemntId hostName hostIds and return if an error occured in the error parameter with an error message in errorMessage parameter
        void DownloadLicenseFile(const QString &entitelemntId,const QString & hostName, const QString &hostIds, QString &lLicContent, int &error, QString &errorMessage);
        //Return the license path used to retrieve the license
        QString GetFloatingLicensePath();
      };
    }
}


#endif // FNP_LICENSE_PROVIDER_H
