#ifndef LIBGEXOC_H
#define LIBGEXOC_H

#include <QLibrary>

#include "options_center_widget.h"

#if defined(LIBGEXOC_LIBRARY)
#  define LIBGEXOCSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBGEXOCSHARED_EXPORT Q_DECL_IMPORT
#endif

#ifdef QT_DEBUG
 #define LIBGEXOC "gexocd"
#else
 #define LIBGEXOC "gexoc"
#endif

class GexMainwindow;
class GexScriptEngine;
class OptionsCenterWidget;

extern "C"
LIBGEXOCSHARED_EXPORT
OptionsCenterWidget* GetOCInstance(
  QWidget* parent, GexScriptEngine* gse, GexMainwindow* gmw, int loglevel, QString& strOutputMsg);

// function prototype
typedef OptionsCenterWidget* (*gexoc_get_instance_function)(QWidget* parent, GexScriptEngine* gse, GexMainwindow* gmw, int loglevel, QString& strOutputMsg);

// The function will try to retrieve/create the OptionsCenter Widget and put the pointer in the first arg.
// parent is the parent widget
// gse is the GexScriptEngine
// gmw is the GexMainWindow
// loglevel is the loglevel specially for OC
#define OPTIONSCENTER_GET_INSTANCE(p, parent, gse, gmw, loglevel, strOutputMsg) gexoc_get_instance_function gif=(gexoc_get_instance_function)QLibrary::resolve(LIBGEXOC,"GetOCInstance"); \
	if (gif) p=gif(parent, gse, gmw, loglevel, strOutputMsg); \
    else { p=NULL; QLibrary l(LIBGEXOC); if(!l.load()) GSLOG(3, "cant load lib gexoc !");  }

// function prototype
typedef bool (*gexoc_set_option_function)(QString section, QString csl_name, QString new_value);
// This function try to set the given option to the given value updating the GUI if necessary
#define OPTIONSCENTER_SET_OPTION(result, section, csl_name, new_value) gexoc_set_option_function sof=(gexoc_set_option_function)QLibrary::resolve(LIBGEXOC,"SetOption"); \
	if (sof) result=sof(section, csl_name, new_value); else result=false;
#endif
