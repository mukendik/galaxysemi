#include "license_provider_thread.h"
#include "license_provider_manager.h"
#include "license_provider.h"
#include "gqtl_log.h"
#include "product_info.h"
#include <QObject>

namespace GS
{
namespace LPPlugin
{


LicenseProviderThread::LicenseProviderThread(LicenseProviderManager *lpManager, ProductInfo *productInfo, QObject* poObj)
  :QThread(poObj),mLPManager(lpManager),mProductInfo(productInfo)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Create ...");
    mStatus = eWaitForLicenseRequest;
    setTerminationEnabled(true);
}

LicenseProviderThread::~LicenseProviderThread()
{
/*
    QLibrary* lLibrary;
    QList<QLibrary*>::const_iterator lLibraryIter;
    for(lLibraryIter = mAvailableLibrary.begin();
        lLibraryIter != mAvailableLibrary.end(); ++lLibraryIter) {
        lLibrary = *lLibraryIter;
        lp_destroy_function destroyFunction =
             (lp_destroy_function) lLibrary->resolve(LP_DESTROY_FUNCTION);
        destroyFunction();
        lLibrary->unload();
        delete lLibrary;
    }
*/
}

LicenseProviderThread::LPThreadStatus LicenseProviderThread::getStatus()
{
    GSLOG(SYSLOG_SEV_DEBUG, "mStatus ...");
    return mStatus;
}

QString LicenseProviderThread::getProviderMessage()
{
    GSLOG(SYSLOG_SEV_DEBUG, "mProviderMessage ...");
    return mProviderMessage;
}

bool lpOrdering(GS::LPPlugin::LicenseProvider *lp1, GS::LPPlugin::LicenseProvider *lp2)
{
    GSLOG(SYSLOG_SEV_DEBUG, "LP Ordering ...");
    return (lp1->property(LP_USAGE_ORDER).toInt() < lp2->property(LP_USAGE_ORDER).toInt());
}

void LicenseProviderThread::run()
{
    GSLOG(SYSLOG_SEV_DEBUG, "running ...");
    if(!mLPManager || !mProductInfo)
    {
        mStatus = eNoProviderFound;
        return;
    }

    QList<LicenseProvider*> availableLP;
    mAvailableLP = mLPManager->availableLP().values();
    mAvailableLibrary =  mLPManager->availableLibrary().values();

    if(mLPManager->GetAppConfig("UseLP").isNull()
            || !mLPManager->availableLP().contains(mLPManager->GetAppConfig("UseLP").toString()))
    {
        availableLP = mLPManager->availableLP().values();
    }
    else
    {
        availableLP.append(mLPManager->availableLP()[mLPManager->GetAppConfig("UseLP").toString()]);
    }

    //qSort(availableLP.begin(), availableLP.end(), lpOrdering);

    bool lEnableLegacyButton = false;

    if (getenv("GS_ENABLE_LEGACY_LP"))
        lEnableLegacyButton=true;

    QStringList lUsedProvider;
    mStatus = eNoProviderFound;
    while (!availableLP.isEmpty())
    {
        LicenseProvider *provider = availableLP.takeFirst();
        lUsedProvider.append(provider->property(LP_TYPE).toString());

        GSLOG(SYSLOG_SEV_DEBUG, QString("Trying to use LP (%1) ...")
              .arg(provider->property(LP_TYPE).toString()).toLatin1().constData());
        provider->moveToThread(this);
        provider->setLPData("EnableLegacyButton", lEnableLegacyButton);
        provider->setLPData("Exit", QVariant(false));

        mProductInfo->setProductID(provider->getProduct());
        mStatus = eNoLicenseFound;
        if(!provider)
            continue;
        mLPManager->setCurrentProvider(provider->property(LP_TYPE).toString());
        QString logMessage =  QString("Check License with %1").arg(provider->property(LP_TYPE).toString());

        GSLOG(7, QString("Try provider->checkLicense Prod (%1)").arg(mProductInfo->getProductID()).toLatin1().constData());
        if(provider->checkLicense(mProductInfo))
        {
            mStatus = eLicenseFound;
            logMessage+= QString(" License found from %1").arg(provider->property(LP_TYPE).toString());
            GSLOG(SYSLOG_SEV_DEBUG, logMessage.toLatin1().constData());
            break ;
        }
        else
        {
            mLPManager->getCurrentProvider()->getExtendedError(mProviderMessage);
            logMessage+= QString(" License not found %1").arg(mProviderMessage);
            GSLOG(SYSLOG_SEV_DEBUG, logMessage.toLatin1().constData());
            mLPManager->setCurrentProvider("");
            break;
        }
        GSLOG(7, QString("Trying End provider->checkLicense Prod (%1)").arg(mProductInfo->getProductID()).toLatin1().constData());
    }

    GSLOG(7, QString("End With Prod (%1)").arg(mProductInfo->getProductID()).toLatin1().constData());

    if(mStatus == eLicenseFound)
    {
        int ret = exec();
        if (ret == 0)
        {
            //GSLOG(SYSLOG_SEV_DEBUG, "Running Thread Exit Called ...");
            for(int lProviderIdx=0; lProviderIdx<mAvailableLP.count(); ++lProviderIdx)
            {
                LicenseProvider *provider = mAvailableLP[lProviderIdx];
                //GSLOG(SYSLOG_SEV_DEBUG, "Running Thread Exit Called=> provider->moveToThread ...");
                if(provider)
                    provider->moveToThread(QCoreApplication::instance()->thread());

            }
        }

    }
}
}

}

