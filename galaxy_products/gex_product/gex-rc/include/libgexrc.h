#ifndef LIBGEXRC_H
#define LIBGEXRC_H

#include <QLibrary>

//#include "options_center_widget.h"

#if defined(LIBGEXRC_LIBRARY)
#  define LIBGEXRCSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBGEXRCSHARED_EXPORT Q_DECL_IMPORT
#endif

#ifdef QT_DEBUG
 #define LIBGEXRC "libgexrcd"
#else
 #define LIBGEXRC "libgexrc"
#endif

class GexMainwindow;
class CReportsCenterWidget;

extern "C"
LIBGEXRCSHARED_EXPORT
CReportsCenterWidget* GetInstance(QWidget* parent, GexMainwindow* gmw);

//typedef QWidget* (*gexoc_get_instance_function)(QWidget* parent, GexMainwindow* gmw);
typedef CReportsCenterWidget* (*gexrc_get_instance_function)(QWidget* parent, GexMainwindow* gmw);

// The function will try to retrieve the Widget and put the pointer in the first arg.
#define REPORTSCENTER_GET_INSTANCE(p, parent, gmw) gexrc_get_instance_function gif=(gexrc_get_instance_function)QLibrary::resolve(LIBGEXRC,"GetInstance"); \
	if (gif) p=gif(parent, gmw);

typedef bool (*gexrc_set_option_function)(QString section, QString csl_name, QString new_value);
// This function try to set the given option to the given value updating the GUI if necessary
#define REPORTSCENTER_SET_OPTION(result, section, csl_name, new_value) gexrc_set_option_function sof=(gexrc_set_option_function)QLibrary::resolve("libgexrc_d","SetOption"); \
	if (sof) result=sof(section, csl_name, new_value); else result=false;
#endif
