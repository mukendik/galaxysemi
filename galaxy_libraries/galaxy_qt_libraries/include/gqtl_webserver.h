#ifndef GQTL_WEBSERVER_H
#define GQTL_WEBSERVER_H

#include <QObject>
#include <QScriptEngine>
#include <QHostAddress>

#ifndef GQTL_WS_SHARED_EXPORT
    #if defined(GQTL_WS_LIBRARY)
    #  define GQTL_WS_SHARED_EXPORT Q_DECL_EXPORT
    #else
    #  define GQTL_WS_SHARED_EXPORT Q_DECL_IMPORT
    #endif
#endif

namespace GS
{
    namespace QtLib
    {
        class GQTL_WS_SHARED_EXPORT WebServer : public QObject
        {
            Q_OBJECT

            class WebServerPrivate* mPrivate;

            public:
                //! \brief Constructor: parent could be null,
                //! ScriptEngine is mandatory and will be used to evaluate http script request
                //! Wont listen untill Listen is called.
                explicit WebServer(QObject *parent);

            signals:

            public slots:
                void SetScriptEngine(QScriptEngine*);
                //! \brief Start to network listen: will fail if already listening or if ScriptEngine has not been set.
                //! returns "ok" or "error:..."
                //! It is said that is port=0, Qt/OS will choose a port automatically. To be tested....
                QString Listen(const QHostAddress &address, quint16 port);
                //! \brief Closes the server. The server will no longer listen for incoming connections.
                void Close();
                //! \brief test if the webserver is already listening
                bool IsListening();
        };
    }
}
#endif // GQTL_WEBSERVER_H
