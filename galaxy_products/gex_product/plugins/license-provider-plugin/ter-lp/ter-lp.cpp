#ifdef WIN32

#include "ter_license_provider.h"

#ifdef __cplusplus
extern "C"
{
#endif

LICENSE_PROVIDERSHARED_EXPORT GS::LPPlugin::LicenseProvider *instantiateLP(QObject* parent,
                                                                           GS::LPPlugin::LicenseProvider::GexProducts product,
                                                                           const QVariantMap &appConfigData,
                                                                           int &libErrorCode,
                                                                           QString &libError)
{
    return GS::LPPlugin::TerLicenseProvider::initializeLP(parent, product, appConfigData, libErrorCode, libError);
}

LICENSE_PROVIDERSHARED_EXPORT void destroyLP()
{
  GS::LPPlugin::TerLicenseProvider::destroy();
}


#ifdef __cplusplus
}
#endif

#endif
