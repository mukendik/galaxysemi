
#include "fnp_license_provider.h"

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
    GS::LPPlugin::LicenseProvider *instance = GS::LPPlugin::FNPLicenseProvider::initializeLP(parent,
                                                                                         product,
                                                                                         appConfigData,
                                                                                         libErrorCode,
                                                                                         libError);
    return static_cast <GS::LPPlugin::LicenseProvider *> (instance);

}

LICENSE_PROVIDERSHARED_EXPORT void destroyLP()
{
  GS::LPPlugin::FNPLicenseProvider::destroy();
}

#ifdef __cplusplus
}
#endif

