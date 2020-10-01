#include "license_provider.h"
#include "product_info.h"


#include <QDate>
#include <QtGlobal>
#include <QMetaType>
#include <QAbstractSocket>

#ifndef GEX_LICENSE_PROVIDER_H
#define GEX_LICENSE_PROVIDER_H

class QTcpSocket;

namespace GS
{
    namespace LPPlugin
    {
      class GEXMessage;

      class GexLicenseProvider : public GS::LPPlugin::LicenseProvider
      {
          Q_OBJECT
          Q_DISABLE_COPY(GexLicenseProvider)
      private:
          class GexLicenseProviderPrivate *mPrivate;

      public :
          static GexLicenseProvider *initializeLP(QObject* parent,
                                                  GexProducts product,
                                                  const QVariantMap &appConfig,
                                                  int &libErrorCode, QString &libError);
          static void destroy();


      protected:
          static GexLicenseProvider* mInstance;
          GexLicenseProvider(QObject* ,
                             GexProducts ,
                             const QVariantMap &);
          int initialize();
          int cleanup();
          void processGexMessage(const GEXMessage &gexMessage);
      public:
          virtual ~GexLicenseProvider();
      public slots:
          bool checkLicense(ProductInfo *productInfo);
          bool createEvaluationLicense(ProductInfo *);
      public:
          //Customization functions
          QVariantList getRunningModeAvailable();
          QString getWelcomePage();

      protected:
          void setInternalError(int, const QString &);
          int getInternalError(QString &);
      public:
          int getExtendedError(QString &m);

      private:
          enum MissingLicenseChoice
          {
              eUndefinedChoice	= 0x01, // Need the intervention of user if an issue is detected when trying to find the license
              eMissingLicense	= 0x02, // Missing license detected
              eValidLicenseFound  = 0x04, // different valid license found
              eUseFloatingAlways  = 0x08, // use always the floating license
              eUseFloatingOnce  	= 0x10, // use this time the floating license
              eUseLocal  		= 0x20 //  use this time the Evaluation/Standalone license
          };
          enum RunningMode
          {
              eStandalone,
              eClientServer,
              eEvaluation
          };
          QString getFields(RunningMode mode);

          void InitServerDescription();
          void SaveToJson(const QString &provider, const QString &licenseType, const QString &host);
      private slots:
          bool IsCorrectLicenseID(MissingLicenseChoice &eStatus, ProductInfo *productInfo, QString &additionalError);
          bool checkStartupArguments(ProductInfo *productInfo);
          bool ReadLicenseFile(ProductInfo *productInfo, bool bValidityCheckOnly, bool *pbCriticalError);
          void CreateLicenseRequestFile(QString strLicenseType);
          bool StartRunning(ProductInfo *productInfo);
          bool StartRunningStandalone(ProductInfo *productInfo);
          bool StartRunningEvaluation(ProductInfo *productInfo);
          bool StartRunningServer(ProductInfo *productInfo);

      private slots:
          FILE	*getCryptedFile();
          QString	getConfigFilePath();
          QString getIniClientSection();
          QString getLicenseFile();
          QString getLicenseeName();
          QString getProductKey();
          QString getRunningMode();
          unsigned long getChecksum();
          QString getEditField2();
          QString getEditField1();
          int getSelectionMode();

          void setCryptedFile(FILE *file);
          void setConfigFilePath(const QString &strString);
          void setLicenseFile(const QString &strString);
          void setLicenseeName(const QString &strString);
          void setProductKey(const QString &strString);
          void setRunningMode(const QString &strString);
          void setChecksum(unsigned long ulVal);
          void setEditField2(const QString &strString);
          void setEditField1(const QString &strString);
          void setSelectionMode(int iVal);


      protected:
          QStringList getUserChoice();
          QString buildUserNotificationMessage();

      private slots:
          //GEX_LM PART
          void OnConnectToServer();
          void CloseGexLmSocket(void);
          QString WriteLineToSocket(QString strMessage);
          QString ReadLineFromSocket(void);
          void socketConnected();
          void socketConnectionClosed();
          void OnCheckSocketStatus();
          void socketReadyRead();
          void socketError(QAbstractSocket::SocketError /*socketError*/);
          //Getter/setter
          QTcpSocket* getGexLmSocket();
          void setGexLmSocket(QTcpSocket*	pGexLmSocket);
          quint16 getGexLmServerPort();
          void setGexLmServerPort(quint16 uiPort);
          QString	getGexLmServerName();
          void setGexLmServerName(const QString&strName );
          QString getGexLmServerIP();
          void setGexLmServerIP(const QString &strIp);
          QDateTime getSocketEvent();
          void setSocketEvent(const QDateTime &rDateTime);
          int getSocketStatus();
          void setSocketStatus(int iVal);
          void disconnectionSender(const QString &message);
      };
    }
}

#endif // GEX_LICENSE_PROVIDER_H
