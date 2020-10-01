#ifndef QX_STD_UTILS_API_H
#define QX_STD_UTILS_API_H

#include <QtCore/qglobal.h>

/* This is a really common block of pre-processing directives, taking into account the configuration in the
 * common project, checking if we want a shared object or no. This block can be used as template for future
 * library implementations
 */
#ifdef QX_SHARED_API
#    ifndef QX_STD_UTILS_API_DECL
#        ifdef QX_STD_UTILS_API
#            define QX_STD_UTILS_API_DECL Q_DECL_EXPORT
#        else
#            define QX_STD_UTILS_API_DECL Q_DECL_IMPORT
#        endif
#    endif
#else
#    define QX_STD_UTILS_API_DECL
#endif

#endif // QX_STD_UTILS_API_H
