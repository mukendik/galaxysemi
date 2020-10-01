#include <QThread>
#include "license_provider_global.h"

#ifndef LICENSE_PROVIDER_THREAD_H
#define LICENSE_PROVIDER_THREAD_H

class QLibrary;

namespace GS
{
  namespace LPPlugin
  {
    class LicenseProviderManager;
    class ProductInfo;
    class LicenseProvider;
    ////
    /// \brief The LicenseProviderThread class is used to exec all the licensing stuff
    ///
    class LICENSE_PROVIDERSHARED_EXPORT LicenseProviderThread : public QThread
    {
      Q_OBJECT
    public:
      LicenseProviderThread(LicenseProviderManager *lpManager, ProductInfo *productInfo, QObject* poObj=0);
      virtual ~LicenseProviderThread();
    private:
       void run();

    public:
       enum LPThreadStatus
       {
           eWaitForLicenseRequest,
           eNoProviderFound,
           eNoLicenseFound,
           eLicenseFound
       };

    private:
       LicenseProviderManager *mLPManager;
       ProductInfo *mProductInfo;
       LPThreadStatus mStatus;
       QString mProviderMessage;
       QList<LicenseProvider *> mAvailableLP;
       QList<QLibrary *> mAvailableLibrary;
    public:
       LPThreadStatus getStatus();
       QString getProviderMessage();
    };
  }
}
#endif // LICENSE_PROVIDER_THREAD_H
