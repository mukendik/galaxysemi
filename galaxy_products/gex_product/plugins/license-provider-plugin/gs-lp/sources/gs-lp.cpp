#include "gex_license_provider.h"

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
    GS::LPPlugin::LicenseProvider *instance = GS::LPPlugin::GexLicenseProvider::initializeLP(parent,
                                                                                      product,
                                                                                      appConfigData,
                                                                                      libErrorCode,
                                                                                      libError);
    return static_cast <GS::LPPlugin::LicenseProvider *> (instance);

}

LICENSE_PROVIDERSHARED_EXPORT void destroyLP()
{
  GS::LPPlugin::GexLicenseProvider::destroy();
}


#ifdef __cplusplus
}
#endif

