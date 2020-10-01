#ifndef LIBGTLIGXL_GLOBAL_H
#define LIBGTLIGXL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBGTLIGXL_LIBRARY)
#  define LIBGTLIGXLSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBGTLIGXLSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBGTLIGXL_GLOBAL_H
