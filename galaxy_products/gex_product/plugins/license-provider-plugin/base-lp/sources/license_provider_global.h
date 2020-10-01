#include <QtCore/qglobal.h>

#ifndef LICENSE_PROVIDER_GLOBAL_H
#define LICENSE_PROVIDER_GLOBAL_H

#if defined(LICENSE_PROVIDER_LIBRARY)
#  define LICENSE_PROVIDERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LICENSE_PROVIDERSHARED_EXPORT Q_DECL_IMPORT
#endif

namespace GS
{
  namespace LPPlugin
  {


// The following strings define custom QObject property
#define LP_TYPE "lp_type" // The license provider type porperty used internnaly when specifying -uselp option
#define LP_FRIENDLY_NAME "lp_friendly_name" // The license provider friendly name
#define LP_USAGE_ORDER "lp_usage_order" // A numric value used to define which lp to be used if -uselp not specified
#define LP_VERSION "lp_version" // The lp library version

// The function name to be called when loading an lp provider plugin
#define LP_INSTANTIATE_FUNCTION "instantiateLP" // The instatntiation function
#define LP_DESTROY_FUNCTION "destroyLP" // The clean function

// A suffix used to find the lp plugin under ..../LP_GEX_PLUGIN_DIR/LP_SUB_PLUGIN_DIR dierctory
#define LP_PLUGIN_SUFFIX "_lp"

// subdirectory to find the LP plugins
#define LP_GEX_PLUGIN_DIR "plugins"
#define LP_SUB_PLUGIN_DIR "lp"

// An xml saving the configuration of the licensing (ip, port, last lp used, license type)
#define XML_LP_SETTING_FILE "gs_lp.xml"
#define JSON_LP_SETTING_FILE "gs_lp.json"
  }
}

#endif // LICENSE_PROVIDER_GLOBAL_H
