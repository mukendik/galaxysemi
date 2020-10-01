#include <gqtl_log.h>
#include <QThread>

#include "gex_constants.h"
#include "product_info.h"
#ifdef linux
    #include <stdint.h>
#endif

namespace GS
{
namespace LPPlugin
{

ProductInfo *ProductInfo::mProductInfo = NULL;

ProductInfo::ProductInfo(): QObject()
{
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Creating ProductInfo singleton from thread ID: %1")
          .arg((intptr_t)QThread::currentThreadId()).toLatin1().constData());

    mProductID  = GS::LPPlugin::LicenseProvider::eNoProduct;
    mEditionID          = GEX_EDITION_STD;
    mLicenseRunningMode = GEX_RUNNINGMODE_EVALUATION;
    mMonitorProducts    = 0;
    mOptionalModules    = GS::LPPlugin::LicenseProvider::eNoOptions;
    CreateCapabilityRestrictionByProduct();
}

void ProductInfo::CreateCapabilityRestrictionByProduct()
{
    //Create capability for ltxc
    QList<Capabilities> lLtxcRestrictions;
    lLtxcRestrictions << ftrCorrolation << advHisto << advMultiChart << advPearson << advProbabilityPlot << waferStack
                      << toolbox;
    mCapabilityRestrictions.insert(GS::LPPlugin::LicenseProvider::eLtxcOEM, lLtxcRestrictions);

    //Create capability for GEX-TER
    QList<Capabilities> lGexTercRestrictions;
    lGexTercRestrictions << ftrCorrolation << advHisto << advMultiChart << advPearson << advProbabilityPlot<<waferStack
                         << toolbox << waferMap;
    mCapabilityRestrictions.insert(GS::LPPlugin::LicenseProvider::eTerOEM, lGexTercRestrictions);

    //Create capability for PRO+
    QList<Capabilities> lProPluscRestrictions;
    // for the moment, this list is empty
    mCapabilityRestrictions.insert(GS::LPPlugin::LicenseProvider::eTerProPlus, lProPluscRestrictions);
}


ProductInfo::ProductInfo(const ProductInfo &other) : QObject(other.parent())
{
    mProductID          = other.mProductID;
    mEditionID          = other.mEditionID;
    mLicenseRunningMode = other.mLicenseRunningMode;
    mMonitorProducts    = other.mMonitorProducts;
    mOptionalModules    = other.mOptionalModules;
    mCapabilityRestrictions = other.mCapabilityRestrictions;
}

ProductInfo::~ProductInfo()
{
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("Deleting ProductInfo singleton from thread ID: %1")
          .arg((intptr_t)QThread::currentThreadId()).toLatin1().constData());
}

void ProductInfo::Destroy()
{
    if(mProductInfo)
    {
        delete mProductInfo;
        mProductInfo = NULL;
    }

}

QString ProductInfo::GetProfileVariable() const
{
    QString lProfileVariable;

    if (isPATMan())
        lProfileVariable = "PM_SERVER_PROFILE";
    else if (isYieldMan())
        lProfileVariable = "YM_SERVER_PROFILE";
    else if (isGTM())
        lProfileVariable = "GTM_SERVER_PROFILE";
    else
        lProfileVariable = "GEX_CLIENT_PROFILE";

    // Backward compatibility, use GEX_SERVER_PROFILE when new env variable is not defined
    if (getenv(lProfileVariable.toLatin1().constData()) == NULL && isMonitoring())
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Server profile variable " + lProfileVariable +
                      " is not defined. The software will use the old one GEX_SERVER_PROFILE instead.")
              .toLatin1().constData());

        lProfileVariable = "GEX_SERVER_PROFILE";
    }

    return lProfileVariable;
}

QString ProductInfo::GetProfileFolder() const
{
    return qgetenv(GetProfileVariable().toLatin1().constData());
}

ProductInfo *ProductInfo::getInstance()
{
    if(!mProductInfo)
        mProductInfo = new ProductInfo;

    return mProductInfo;
}

bool ProductInfo::isOEM(qlonglong ignore) const
{
    qlonglong oem = GS::LPPlugin::LicenseProvider::eLtxcOEM | GS::LPPlugin::LicenseProvider::eSzOEM |
                    GS::LPPlugin::LicenseProvider::eAslOEM | GS::LPPlugin::LicenseProvider::eSapphireOEM |
                    GS::LPPlugin::LicenseProvider::eTerOEM;
    oem = oem & ~(ignore);
    return ((mProductID & oem ) != 0);
}

bool  ProductInfo::isDBPluginAllowed() const
{
    return isMonitoring() || isExaminatorPAT() || isExaminatorPRO() || isExaminatorTerProPlus();
}

bool  ProductInfo::isExaminatorEval() const
{
    return (mProductID == GS::LPPlugin::LicenseProvider::eExaminatorEval);

}

bool  ProductInfo::isExaminator () const
{
    return (mProductID == GS::LPPlugin::LicenseProvider::eExaminator);
}

bool ProductInfo::isMonitoring() const
{
    if ((mProductID == GS::LPPlugin::LicenseProvider::eYieldMan) || (mProductID == GS::LPPlugin::LicenseProvider::eYieldManEnterprise)
        || (mProductID == GS::LPPlugin::LicenseProvider::ePATMan) || (mProductID == GS::LPPlugin::LicenseProvider::ePATManEnterprise))
        return true;

    return false;
}


bool ProductInfo::isGenealogyAllowed() const
{
    if (mOptionalModules & GS::LPPlugin::LicenseProvider::eGenealogy)
        return true;

    return false;
}

bool ProductInfo::isSYLSBLAllowed() const
{
    if (mOptionalModules & GS::LPPlugin::LicenseProvider::eSYA)
        return true;

    return false;
}

bool ProductInfo::isTDRAllowed() const
{
    if (mOptionalModules & GS::LPPlugin::LicenseProvider::eTDR)
        return true;

    return false;
}

bool ProductInfo::isGTM() const
{
    if(mProductID == GS::LPPlugin::LicenseProvider::eGTM)
        return true;
    return false;
}

bool ProductInfo::isYieldMan() const
{
    if((mProductID == GS::LPPlugin::LicenseProvider::eYieldMan) || (mProductID == GS::LPPlugin::LicenseProvider::eYieldManEnterprise))
        return true;
    return false;

}

bool ProductInfo::isPATMan() const
{
    if((mProductID == GS::LPPlugin::LicenseProvider::ePATMan) || (mProductID == GS::LPPlugin::LicenseProvider::ePATManEnterprise))
        return true;
    return false;

}

bool ProductInfo::isEnterprise() const
{
    if((mProductID == GS::LPPlugin::LicenseProvider::eYieldManEnterprise) || (mProductID == GS::LPPlugin::LicenseProvider::ePATManEnterprise))
        return true;
    return false;

}

bool ProductInfo::isExaminatorPAT() const
{
    if(mProductID == GS::LPPlugin::LicenseProvider::eExaminatorPAT)
        return true;
    return false;

}

bool ProductInfo::isExaminatorPRO() const
{
    if(mProductID == GS::LPPlugin::LicenseProvider::eExaminatorPro)
        return true;
    return false;

}

bool ProductInfo::isExaminatorTerProPlus() const
{
    if(mProductID == GS::LPPlugin::LicenseProvider::eTerProPlus)
        return true;
    return false;
}

bool ProductInfo::isExaminatorTer() const
{
    if(mProductID == GS::LPPlugin::LicenseProvider::eTerOEM)
        return true;
    return false;
}

GS::LPPlugin::LicenseProvider::GexProducts ProductInfo::getProductID() const
{
    return mProductID;
}

long ProductInfo::getEditionID() const
{
    return mEditionID;
}

long ProductInfo::getLicenseRunningMode() const
{
    return mLicenseRunningMode;
}

long ProductInfo::getMonitorProducts() const
{
    return mMonitorProducts;
}

unsigned int ProductInfo::getOptionalModules() const
{
    return mOptionalModules;
}

void ProductInfo::setProductID(GS::LPPlugin::LicenseProvider::GexProducts productID)
{
    mProductID = productID;
}

void ProductInfo::setEditionID(long editionID)
{
    mEditionID = editionID;
}

void ProductInfo::setLicenseRunningMode(long licenseRunningMode)
{
    mLicenseRunningMode = licenseRunningMode;
}

void ProductInfo::setMonitorProducts(long monitoredProducts)
{
    mMonitorProducts = monitoredProducts;
}

void ProductInfo::ClearOptionalModules()
{
    mOptionalModules = 0;
}

QString ProductInfo::setOptionalModules(unsigned int optionalModules)
{
    //if (!getenv("GS_ROOT"))
        //return;

    unsigned int temp  = mOptionalModules | optionalModules;
    mOptionalModules = temp;
    return "ok";
}

bool ProductInfo::isNotSupportedCapability(Capabilities capability)
{
    if (mCapabilityRestrictions.keys().contains(mProductID))
    {
        if (mCapabilityRestrictions.value(mProductID).contains(capability))
            return true;
    }
    return false;
}



/*
bool ProductInfo::isAdditionalFeatureAllowed(const QString &feature) const
{
    if(!mAdditionalFeature.contains(feature))
        return false;

    return mAdditionalFeature[feature];
}

void ProductInfo::addAdditionalFeature(const QString &feature)
{
    if(!mAdditionalFeature.contains(feature))
        mAdditionalFeature.insert(feature, false);

}

QMap<QString, bool> &ProductInfo::AdditionalFeature()
{
    return mAdditionalFeature;
}

void ProductInfo::updateAdditionalFeatureStatus(const QString &feature, bool allowed)
{
    if(!mAdditionalFeature.contains(feature))
        mAdditionalFeature.insert(feature, allowed);
    else
        mAdditionalFeature[feature] = allowed;
}*/

}
}
