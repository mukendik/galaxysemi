#ifndef GQTL_LOG_H
#define GQTL_LOG_H

#include <QCoreApplication>
#include <QLibrary>
#include <QString>
// GQT LOG is no more GUI enable. Todo : move/rename GEXASSERT to GSASSERT
/*
    #ifdef QT_DEBUG
     #include <QApplication>
     #include <QMessageBox>
    #endif
*/
//#include <QtCore/qglobal.h> // needed to use Q_DECL_EXPORT and Q_DECL_IMPORT

//#include <stdarg.h>	// for variable args

#if defined(GQTLLOG_LIBRARY)
#  define GQTLLOGSHARED_EXPORT Q_DECL_EXPORT
#else
#  define GQTLLOGSHARED_EXPORT Q_DECL_IMPORT
#endif

//#define __STR(X) X
//#define _STR(X) __STR(X)
#define STR(X) (#X)
#define STR2(X) STR(X)

//extern "C"
//LIBGEXLOGSHARED_EXPORT	const char* module,

#ifdef QT_DEBUG
 #define LIBNAME "gqtl_logd"
#else
 #define LIBNAME "gqtl_log"
#endif

typedef int (*gqtl_log_function)(int, const char*, const char*, const char*, const char*);

extern "C" GQTLLOGSHARED_EXPORT
 int gqtl_log(int sev, const char* module, const char* file, const char* fctn, const char* m);


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
// log with only one argument.
// To use several, use QString("%1 %2").arg(...).arg(...)
// __PRETTY_FUNCTION__ is a defined from GNU which include all infos : "int Class::Method(...)"
#define GSLOG(sev, m) { gqtl_log_function lFP=(gqtl_log_function)QLibrary::resolve(LIBNAME,"gqtl_log"); \
    if (lFP) lFP( (int)sev, STR2(MODULE), (__FILE__), TOSTRING(__LINE__), (m) ); }

// When loging a message, choose 1 between those level :
enum SYSLOG_SEV {
	SYSLOG_SEV_EMERGENCY = 0,
	SYSLOG_SEV_ALERT = 1,
	SYSLOG_SEV_CRITICAL = 2,
	SYSLOG_SEV_ERROR = 3,
	SYSLOG_SEV_WARNING = 4,
	SYSLOG_SEV_NOTICE = 5,
	SYSLOG_SEV_INFORMATIONAL = 6,
    SYSLOG_SEV_DEBUG = 7
};

// in order to use the GUI version of GEX ASSERT, move it to another lib : gqtl_log is not GUI enabled
#if 0    //#ifdef  QT_DEBUG
 #define GEX_ASSERT(X) if (!(X)) \
    {	\
        GSLOG(SYSLOG_SEV_ERROR, QString("assert failed : '%1'").arg(#X).toLatin1().constData()); \
        if (qApp) \
            { \
                QMessageBox msgBox;	\
                QString m=QString("Assert failed in file %1\n").arg(__FILE__) \
                 + QString("in function %1").arg(__PRETTY_FUNCTION__) \
                 + QString("\nat line %1 :\n").arg(__LINE__)+QString(#X);  \
                msgBox.setWindowTitle("GEX_ASSERT failed !"); \
                msgBox.setText(m); \
                msgBox.setStandardButtons(QMessageBox::Ok|QMessageBox::Abort|QMessageBox::Ignore); \
                msgBox.setButtonText( QMessageBox::Ok, "Breakpoint"); \
                msgBox.setButtonText( QMessageBox::Abort, "Exit" ); \
                int iExecStatus = msgBox.exec(); \
                switch(iExecStatus) \
                {	\
                case QMessageBox::Ok :	asm("int3"); break; \
                case QMessageBox::Abort	: exit(0); break;	\
                default				: \
                case QMessageBox::Ignore : break; \
                } \
            } \
    }
#else
 #define GEX_ASSERT(X) if (!(X)) \
    GSLOG(SYSLOG_SEV_ERROR, QString("assert failed : '%1'").arg(#X).toLatin1().constData());
#endif

#endif // header
