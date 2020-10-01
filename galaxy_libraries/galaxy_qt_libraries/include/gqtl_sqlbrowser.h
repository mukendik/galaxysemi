#ifndef GQTL_SQLBROWSER_H
#define GQTL_SQLBROWSER_H

#include <QLibrary>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QWidget>
#include <QFile>

#if defined(LIB_LIBRARY)
#  define LIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#ifdef QT_DEBUG
 #define LIBSQLBROWSER "gqtl_sqlbrowserd"
#else
 #define LIBSQLBROWSER "gqtl_sqlbrowser"
#endif

namespace GS
{
    namespace QtLib
    {
        class SqlBrowser : public QWidget
        {
            Q_OBJECT

            class SqlBrowserPrivate* mPrivate; // Private, do not touch
            public:
                SqlBrowser(QWidget* p);
                ~SqlBrowser();
            public slots:
                // Remove me
                //void NewDBConnection(QMap< QString, QVariant> params);

                // Refresh the DB list with the current content of QSqlDatabase cache list
                void Refresh();
            signals:
                void statusMessage(const QString &message);
        };
    }
}

typedef GS::QtLib::SqlBrowser* (*get_instance_function)(QWidget* parent, QString& strOutputMsg);

extern "C"
LIBSHARED_EXPORT
GS::QtLib::SqlBrowser* GetInstance(QWidget* parent, QString& strOutputMsg);

// The function will try to retrieve/create the SqlBrowser Widget and put the pointer in the first argument.
// pointer is the pointer to the SqlBrowser QWidget. Will be NULL on error.
// parent is the desired parent widget. Can be NULL.
#define SQLBROWSER_GET_INSTANCE(pointer, parent, strOutputMsg) get_instance_function gif=(get_instance_function)QLibrary::resolve(LIBSQLBROWSER,"GetInstance"); \
	if (gif) pointer=gif(parent, strOutputMsg); \
    else { pointer=NULL; QLibrary l(LIBSQLBROWSER); \
            if(!l.load()) \
            { \
                strOutputMsg=QString("cant load library %1:%2").arg(LIBSQLBROWSER).arg(l.errorString()); \
                GS::Gex::Engine::GetInstance().Download( \
                  "http://galaxyec7.com/helpconsole2010/GalaxyUserAssistance/docs/extensions/" \
                  +CGexSystemUtils::GetPlatform()+QString("/gqtl_sqlbrowser."\
                  +CGexSystemUtils::GetPlatformDynLibExtension()+".pdf"), \
                  "gqtl_sqlbrowser."+CGexSystemUtils::GetPlatformDynLibExtension()); \
            } \
        }

#endif  // GQTL_SQLBROWSER_H
