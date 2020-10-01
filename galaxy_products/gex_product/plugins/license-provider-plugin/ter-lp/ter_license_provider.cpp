#ifdef WIN32

#include <QTimer>
#include <QThread>
#include <QAxObject>

#include <time.h>
#include <stdio.h>

#include "license_provider_profile.h"
#include "gqtl_log.h"
#include "gex_shared.h"
#include "gex_version.h"
#include "product_info.h"
#include "gex_constants.h"
#include "ter_license_provider.h"
#include "gex_errors.h"
#include "license_provider_common.h"
#include "license_provider_dialog.h"
#include "read_system_info.h"
#include "gstdl_type.h"
#include <windows.h>
#include <objbase.h>
#undef CopyFile

namespace GS
{
namespace LPPlugin
{

#define TER_LP_VERSION "0.0"
#define INCREMENT_TERGEX    "TER-GEX-PRO"
#define INCREMENT_TERGEXPRO "TER-GEX"
// For debug
// If set to 1, this will avoid calling the COM checkout that crashes in debug mode
#define TER_DEBUG           0
// If running in TER_DEBUG mode, the checkout will return this instead of calling the COM checkout
#define TER_DEBUG_LICSTATUS false

const QString sTerGexInc = QString("TER-GEX");
const QString sTerGexProInc = QString("TER-GEX-PRO");

class TerLicenseProviderPrivate
{
public:
    //Internal error handling
    QString mInternalErrorMessage;
    int     mInternalError;

    QVariantMap mAppConfigData;
    ProductInfo *mProductInfo;

    // For communication with TER license COM objects
    QAxObject*  mTAGLM_Proxy;
    QAxObject*  mTAGLM_LicInfo;

public:
    TerLicenseProviderPrivate(const QVariantMap &appConfigData);
    virtual ~TerLicenseProviderPrivate();

    bool initialize();
    bool checkout(QString Increment, bool *CheckoutStatus);
    bool checkin();
};

bool TerLicenseProviderPrivate::initialize()
{
    mInternalError = TerLicenseProvider::eLPLibNoError;
    mInternalErrorMessage = "";

    // Check if COM initialization already done. If not, let's do it!
    if(mTAGLM_Proxy == NULL)
    {
        // Initialize COM system
        HRESULT lResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if(lResult != S_OK)
        {
            mInternalError = TerLicenseProvider::eLPInternalError;
            mInternalErrorMessage = QString("Failed to initialize COM library");
            return false;
        }

        // Create object tom communicate with TAGLM COM object
        try
        {
            mTAGLM_Proxy = new  QAxObject();
        }
        catch(std::bad_alloc&)
        {
            mInternalError = TerLicenseProvider::eLPInternalError;
            mInternalErrorMessage = QString("Failed to allocate the COM container object (QAxObject)");
            CoUninitialize();
            return false;
        };

        // Set control to TAGLMProxy COM
        mTAGLM_Proxy->setControl("TAGLMProxyLib.TAGLMProxy");
        if(mTAGLM_Proxy->isNull())
        {
            mInternalError = TerLicenseProvider::eLPInternalError;
            mInternalErrorMessage = QString("Failed to initialize the COM object (TAGLMProxyLib.TAGLMProxy)");
            CoUninitialize();
            delete mTAGLM_Proxy;
            mTAGLM_Proxy = NULL;
            return false;
        }
    }

    return true;
}

bool TerLicenseProviderPrivate::checkout(QString Increment, bool * CheckoutStatus)
{
    mInternalError = TerLicenseProvider::eLPLibNoError;
    mInternalErrorMessage = "";
    *CheckoutStatus = false;

    // Check if COM initialized.
    if(mTAGLM_Proxy == NULL)
    {
        mInternalError = TerLicenseProvider::eLPInternalError;
        mInternalErrorMessage = QString("COM object not initialized: TAGLMProxyLib.TAGLMProxy");
        return false;
    }

#if TER_DEBUG
    // In debug mode, the querySubObject() function will crash, so just pretend it's OK.
    *CheckoutStatus = TER_DEBUG_LICSTATUS;
    return true;
#endif

    // Call Checkout COM function
    QString lErr;
    mTAGLM_LicInfo = mTAGLM_Proxy->querySubObject("CheckOut(QString, QString&, QString, QString, bool)", Increment, lErr, QString("0.0"), QString(""), QString("true"));
    if(!mTAGLM_LicInfo)
    {
        // Checkout NOK but function succeeded
       return true;
    }

    // Checkout OK
    *CheckoutStatus = true;
    return true;
}

bool TerLicenseProviderPrivate::checkin()
{
    mInternalError = TerLicenseProvider::eLPLibNoError;
    mInternalErrorMessage = "";

    // Make sure COM object is loaded
    if(mTAGLM_Proxy == NULL)
    {
        mInternalError = TerLicenseProvider::eLPInternalError;
        mInternalErrorMessage = QString("COM object not initialized: TAGLMProxyLib.TAGLMProxy");
        return false;
    }

#if TER_DEBUG
    // In debug mode, the querySubObject() function will crash, so just pretend it's OK.
    return true;
#endif

    // Make sure we have a license checked out
    if(!mTAGLM_LicInfo)
    {
        mInternalError = TerLicenseProvider::eLPInternalError;
        mInternalErrorMessage = QString("No Teradyne license checked out");
        return false;
    }

    // Checkin
    QVariant lParam = mTAGLM_LicInfo->asVariant();
    mTAGLM_Proxy->dynamicCall("CheckIn(IDispatch*", lParam);
    mTAGLM_LicInfo = NULL; // CheckMe

    return true;
}

TerLicenseProviderPrivate::TerLicenseProviderPrivate(const QVariantMap &appConfigData)
    : mAppConfigData(appConfigData)
{
    mInternalError = TerLicenseProvider::eLPLibNoError;
    mInternalErrorMessage = "";
    mProductInfo = NULL;
    mTAGLM_Proxy = NULL;
    mTAGLM_LicInfo = NULL;
}

TerLicenseProviderPrivate::~TerLicenseProviderPrivate()
{
    // In case...
    checkin();

    // Cleanup
    if(mTAGLM_Proxy)
    {
        CoUninitialize();
        delete mTAGLM_Proxy;
    }
}

TerLicenseProvider* TerLicenseProvider::mInstance = 0;
TerLicenseProvider *TerLicenseProvider::initializeLP(QObject* parent,
                                                    GexProducts product,
                                                    const QMap<QString, QVariant> &appConfig,
                                                    int &libErrorCode,
                                                    QString &libError)
{
  libErrorCode = eLPLibNoError;
  libError = "";

  if(!mInstance)
  {
      try
      {
          mInstance = new TerLicenseProvider(parent, product, appConfig);
      }
      catch(std::bad_alloc&)
      {
          libErrorCode = eLPAlloc;
          libError = QString("Failed to allocate TerLicenseProvider object.");
          mInstance = 0;
          return 0;
      };

      if(mInstance->getLastErrorCode() != eLPLibNoError)
      {
          libErrorCode = mInstance->getLastErrorCode();
          libError = mInstance->getLastError();
          delete mInstance;
          mInstance = 0;
          return 0;
      }
  }
  return mInstance;
}

void TerLicenseProvider::destroy()
{
    delete mInstance;
    mInstance = 0;
}

TerLicenseProvider::TerLicenseProvider(QObject* parent,
                                       GexProducts product,
                                       const QVariantMap &appConfig)
  : LicenseProvider(parent, product, appConfig)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");
    setProperty(LP_TYPE , QString("ter_lp"));
    setProperty(LP_FRIENDLY_NAME , QString("Teradyne License Provider"));
    setProperty(LP_VERSION , TER_LP_VERSION);
    mPrivate = 0;

    int lStatus = initialize();
    GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with initialize return code (%1)").arg(lStatus).toLatin1().constData());
}

TerLicenseProvider::~TerLicenseProvider()
{
    cleanup();
}

int TerLicenseProvider::initialize()
{
    try
    {
        mPrivate = new TerLicenseProviderPrivate(getFullAppConfigData());
    }
    catch(std::bad_alloc&)
    {
        setLastError(eLPAlloc, "Failed to allocate private object.");
        mPrivate = 0;
        return eLPAlloc;
    };

    setLastError(mPrivate->mInternalError,  mPrivate->mInternalErrorMessage);

    if(mPrivate->mInternalError != eLPLibNoError)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with code (%1) message(%2)")
              .arg(mPrivate->mInternalError).arg(mPrivate->mInternalErrorMessage).toLatin1().constData());
        return mPrivate->mInternalError;
    }

    return eLPLibNoError;
}

int TerLicenseProvider::cleanup()
{
    delete mPrivate;
    mPrivate = 0;
    return 0;
}

void TerLicenseProvider::setInternalError(int ErrorCode, const QString &ErrorMsg)
{
    mPrivate->mInternalError = ErrorCode;
    mPrivate->mInternalErrorMessage = ErrorMsg;
}

int TerLicenseProvider::getInternalError(QString &ErrorMsg)
{
    ErrorMsg = mPrivate->mInternalErrorMessage;
    return mPrivate->mInternalError;
}

int TerLicenseProvider::getExtendedError(QString &m)
{
    m = QString("%1").arg(mPrivate->mInternalErrorMessage);
    return mPrivate->mInternalError;
}

// Only useful if the provider also supports userInteraction signal to ask the user to choose between
// multiple license types (Evaluation, Standalone, Floating)
QString TerLicenseProvider::getWelcomePage()
{
    return "";
}

// Only useful if the provider also supports userInteraction signal to ask the user to choose between
// multiple license types (Evaluation, Standalone, Floating).
// Let's just fill teh available modes with the only one supported for ter_lp (floating)
QVariantList TerLicenseProvider::getRunningModeAvailable()
{
    QVariantList availableMode ;
    availableMode.append("Floating");

    return availableMode;
}

// Process message received from GEX
void TerLicenseProvider::processGexMessage(const GEXMessage &gexMessage)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("processGexMessage GEXMessage(%1 , %2)").arg(gexMessage.getType())
          .arg(gexMessage.getData()).toLatin1().constData())

    if(gexMessage.getType() == GEXMessage::eDisconnect)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "eDisconnect clientIdle(1)");
        mPrivate->checkin();
        return;
    }

    if(gexMessage.getType() == GEXMessage::eGetYMAdminDBConfig)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "eGetYMAdminDBConfig (Not supported)");
        QString yieldManDbSettingsConf = QString("GEX_YieldManDbSettings|NotSupported");
        emit sendLPMessage(LPMessage(LPMessage::eSetYMAdminDBConfig, yieldManDbSettingsConf));
        return;
    }

    if(gexMessage.getType() == GEXMessage::eReconnect)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "eReconnect clientIdle(0)");
        bool    lStatus=false, lCheckout=false;
        QString lMessage, lIncrement;
        if(mPrivate->mProductInfo && (mPrivate->mProductInfo->getProductID()==eTerOEM))
            lIncrement = sTerGexInc;
        else if(mPrivate->mProductInfo && (mPrivate->mProductInfo->getProductID()==eTerProPlus))
            lIncrement = sTerGexProInc;
        else
        {
            lMessage = QString("Failed to re-connect: unsupported product %1").arg(mPrivate->mProductInfo->getProductID());
            GSLOG(SYSLOG_SEV_CRITICAL, lMessage.toLatin1().constData());
            setInternalError(LPMessage::eReject,lMessage);
            emit sendLPMessage(LPMessage(LPMessage::eReject, lMessage));
            return;
        }

        // Let's try to checkout the appropriate increment
        lStatus = mPrivate->checkout(lIncrement, &lCheckout);
        if(!lStatus)
        {
            lMessage = QString("Error during Teradyne license checkout for increment %1 (%2)")
                    .arg(lIncrement).arg(mPrivate->mInternalErrorMessage);
            GSLOG(SYSLOG_SEV_CRITICAL, lMessage.toLatin1().constData());
            setInternalError(LPMessage::eReject, lMessage);
            emit sendLPMessage(LPMessage(LPMessage::eReject, lMessage));
            return;
        }

        // Check checkout status ;-)
        if(!lCheckout)
        {
            // No increment could be checked out
            lMessage = QString("Failed to checkout a Teradyne license for increment %1")
                    .arg(lIncrement);
            GSLOG(SYSLOG_SEV_CRITICAL, lMessage.toLatin1().constData());
            setInternalError(LPMessage::eReject, lMessage);
            emit sendLPMessage(LPMessage(LPMessage::eReject, lMessage));
            return;
        }

        // SUCCESS
        emit sendLPMessage(LPMessage(LPMessage::eAccept,"Reconnect succeed"));
        return;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("End processGexMessage : %1 : %2").arg(gexMessage.getType())
          .arg(gexMessage.getData()).toLatin1().constData());
}

// Check if a license is available
bool TerLicenseProvider::checkLicense(ProductInfo *productInfo)
{
    QString lErrorMsg;

    GSLOG(SYSLOG_SEV_DEBUG, QString("::checkLicense Start %1").arg(getProduct()).toLatin1().constData());
    if(!productInfo)
    {
        lErrorMsg = QString("Internal error: NULL productInfo pointer");
        GSLOG(SYSLOG_SEV_CRITICAL, lErrorMsg.toLatin1().constData());
        setInternalError(LPMessage::eReject, lErrorMsg);
        return false;
    }

    // TER license provider is necessarily client/server mode
    productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_CLIENT);
    mPrivate->mProductInfo = productInfo;

    // In any case, the TER license provider does not really manage an expiration date, so let's set a dynamic
    // date at +10 years.
    QDate ed = QDate::currentDate().addYears(10);
    setLPData("ExpirationDate", ed);
    setLPData("MaintenanceExpirationDate", ed);

    // Intitalize private object. This will fail if we do it to early, ie in TerLicenseProvider::initialize().
    // The thread has to be up and running
    if(!mPrivate->mTAGLM_Proxy)
    {
        if(!mPrivate->initialize())
        {
            lErrorMsg = QString("Internal error (%1): %2")
                    .arg(mPrivate->mInternalError).arg(mPrivate->mInternalErrorMessage);
            GSLOG(SYSLOG_SEV_CRITICAL, lErrorMsg.toLatin1().constData());
            setInternalError(LPMessage::eReject, lErrorMsg);
            return false;
        }

        // Connect exception signal
        QObject::connect(mPrivate->mTAGLM_Proxy, SIGNAL(exception(int, const QString&, const QString&, const QString&)),
                         this, SLOT(onException(int, const QString&, const QString&, const QString&)));
    }

    // Let's make sure we are running in TER mode
    LicenseProvider::GexProducts lProduct = getProduct();
    QString lPrimaryIncrement, lAltIncrement;
    bool lStatus=false, lCheckout=false;
    if(lProduct == eTerOEM)
    {
        lPrimaryIncrement = sTerGexInc;
        lAltIncrement = sTerGexProInc;
    }
    else if(lProduct == eTerProPlus)
    {
        lPrimaryIncrement = sTerGexProInc;
        lAltIncrement = sTerGexInc;
        productInfo->setOptionalModules(eSYA);
        productInfo->setOptionalModules(eTDR);
        productInfo->setOptionalModules(eGenealogy);
    }
    else
    {
        lErrorMsg = QString("You must run a Teradyne edition to use the ter_lp license provider\n(\"-uselp=ter_lp\" argument)");
        GSLOG(SYSLOG_SEV_CRITICAL, lErrorMsg.toLatin1().constData());
        setInternalError(LPMessage::eReject, lErrorMsg);
        return false;
    }

    // Let's try to checkout the appropriate increment
    lStatus = mPrivate->checkout(lPrimaryIncrement, &lCheckout);
    if(!lStatus)
    {
        lErrorMsg = QString("Error during Teradyne license checkout for increment %1 (%2)")
                .arg(lPrimaryIncrement).arg(mPrivate->mInternalErrorMessage);
        GSLOG(SYSLOG_SEV_CRITICAL, lErrorMsg.toLatin1().constData());
        setInternalError(LPMessage::eReject, lErrorMsg);
        return false;
    }

    // Check checkout status ;-)
    if(lCheckout)
    {
        emit sendLPMessage(LPMessage(LPMessage::eAccept,"The license is granted..."));
        return true;
    }

    // Let's see if the other increment would work
    lStatus = mPrivate->checkout(lAltIncrement, &lCheckout);
    if(!lStatus)
    {
        lErrorMsg = QString("Error during Teradyne license checkout for increment %1 (%2)")
            .arg(lAltIncrement).arg(mPrivate->mInternalErrorMessage);
        GSLOG(SYSLOG_SEV_CRITICAL, lErrorMsg.toLatin1().constData());
        setInternalError(LPMessage::eReject, lErrorMsg);
        return false;
    }

    // Check checkout status ;-)
    if(lCheckout)
    {
        // An alternative increment is available, ask the user if we should use it
        QString lRetValue;
        QStringList lIncrements(lAltIncrement);

        QString lMessage = QString("You have attempted to launch %1."
                                   "\n\nNo %2 increment could be checked out for this product edition."
                                   "\nHowever we have found an increment for another edition."
                                   "\n\nYou can click \"Use\" below to use that license."
                                   "\n\nIn the future you may want to use the correct shortcut for the"
                                   "\nversion of Examinator that you are trying to launch.\n")
                .arg(getAppConfigData("AppFullName").toString().section(" - ",0,0).simplified())
                .arg(lPrimaryIncrement);
        emit userAvailableChoices(&lMessage, &lIncrements,false,&lRetValue);

        // Check user choice
        QStringList lValues = lRetValue.split("|",QString::SkipEmptyParts);
        if((lValues.count()>0) && (lValues[0].toInt() == 0))
        {
            LicenseProvider::GexProducts lProductToBeUsed = (lProduct == eTerOEM) ? eTerProPlus : eTerOEM;
            setProduct(lProductToBeUsed);
            productInfo->setProductID(lProductToBeUsed);
            if (lProduct == eTerProPlus)
            {
                productInfo->setOptionalModules(eSYA);
                productInfo->setOptionalModules(eTDR);
                productInfo->setOptionalModules(eGenealogy);
            }
            emit sendLPMessage(LPMessage(LPMessage::eAccept,"The license is granted..."));
            return true;
        }

        // The user chose to not use the alternative increment, so check it back in
        mPrivate->checkin();
    }

    // No increment could be checked out
    lErrorMsg = QString("Failed to checkout a Teradyne license for increment %1")
            .arg(lPrimaryIncrement);
    GSLOG(SYSLOG_SEV_CRITICAL, lErrorMsg.toLatin1().constData());
    setInternalError(LPMessage::eReject, lErrorMsg);

    return false;
}

void TerLicenseProvider::onException(int code, const QString& source, const QString& desc, const QString& help)
{
    Q_UNUSED(help)
    Q_UNUSED(source)
    mPrivate->mInternalError = TerLicenseProvider::eLPInternalError;
    mPrivate->mInternalErrorMessage = QString("TAGLMProxyLib exception (%1: %2).").arg(code).arg(desc);
}

}
}

#endif
