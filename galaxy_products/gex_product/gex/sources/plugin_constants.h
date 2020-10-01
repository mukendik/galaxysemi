///////////////////////////////////////////////////////////
// Constants for Plugin module
///////////////////////////////////////////////////////////

#ifndef GEX_PLUGIN_CONSTANTS_H
#define GEX_PLUGIN_CONSTANTS_H

///////////////////////////////////////////////////////////
// List of the only valid Plugin IDs
///////////////////////////////////////////////////////////
#define GEX_PLUGINID_MASAMDE	1000		// Masa MDE Plugin
#define GEX_PLUGINID_YIELD123	2000		// Yield-123 Plugin

// Min,Max macros
#ifndef gex_max
#define gex_max(a,b)	((a) < (b) ? (b) :(a))
#endif
#ifndef gex_min
#define gex_min(a,b)	((a) > (b) ? (b) :(a))
#endif
#ifndef gex_maxAbs
#define gex_maxAbs(a,b)	((fabs(a)) < (fabs(b)) ? (b) :(a))
#endif
#ifndef gex_minAbs
#define gex_minAbs(a,b)	((fabs(a)) > (fabs(b)) ? (b) :(a))
#endif

///////////////////////////////////////////////////////////
// Plugin specific home pages
///////////////////////////////////////////////////////////
// Masa MDE plugin
#define GEX_PLUGINHOMEPAGE_MASAMDE	"_gex_db_pro_masamde_home.htm"

///////////////////////////////////////////////////////////
// Plugin specific action links
///////////////////////////////////////////////////////////
// Masa MDE plugin
#define GEX_PLUGINACTION_MASAMDE_CONVERT			"plugin_masa_convert.htm"
#define GEX_PLUGINACTION_MASAMDE_ADMIN				"plugin_masa_admin.htm"
#define GEX_PLUGINACTION_MASAMDE_TRAIN				"plugin_masa_train.htm"
#define GEX_PLUGINACTION_MASAMDE_APPLY				"plugin_masa_apply.htm"
#define GEX_PLUGINACTION_MASAMDE_TOEXCEL			"plugin_masa_toexcel"

// Gex Yield-123 plugin
#define GEX_PLUGINACTION_YIELD123_YIELD				"plugin_y123_yield"
#define GEX_PLUGINACTION_YIELD123_QUALITY			"plugin_y123_quality"
#define GEX_PLUGINACTION_YIELD123_REPEATABILITY		"plugin_y123_repeatability"
// Following action links are followed by some custom parameters, so
// the actual action link name begins with the following texts, but contains
// some additional text.
#define GEX_PLUGINACTION_YIELD123_PRIVATELINK		"plugin_y123_privatelink"

///////////////////////////////////////////////////////////
// Common constants
///////////////////////////////////////////////////////////
#define CSV_SEPARATOR ','

///////////////////////////////////////////////////////////
// Special numeric values
///////////////////////////////////////////////////////////
// Value considered invalid (floating point max). Used to mark elements as not available or not executed.
#define GEX_PLUGIN_INVALID_VALUE_FLOAT		3.4e+38F
#define GEX_PLUGIN_INVALID_VALUE_DOUBLE		1.7e+308
#define GEX_PLUGIN_INVALID_VALUE_INT		0x7FFFFFFF
#define GEX_PLUGIN_INVALID_VALUE_UINT		0xFFFFFFFF
// Value considered infinite.
#define GEX_PLUGIN_INFINITE_VALUE_FLOAT		3.3e+38F	
#define GEX_PLUGIN_INFINITE_VALUE_DOUBLE	1.6e+308

#endif // ifdef GEX_PLUGIN_CONSTANTS_H

