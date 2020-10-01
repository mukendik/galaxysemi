#ifndef GQTL_WEBSERVER_PRIV_H
#define GQTL_WEBSERVER_PRIV_H

#include <QObject>
#include <QScriptEngine>
// tufao
#include <HttpServer>
#include <HttpServerRequestRouter>

namespace GS
{
    namespace QtLib
    {
        class WebServerPrivate : public Tufao::AbstractHttpServerRequestHandler
        {
            Q_OBJECT

            public:
                Tufao::HttpServer mServer;
                Tufao::HttpServerRequestRouter mRouter;
                QMap<Tufao::HttpServerRequest*, Tufao::HttpServerResponse*> mClients;
                QScriptEngine *mScriptEngine;
                explicit WebServerPrivate(QObject* p);

            public slots:
                bool handleRequest(Tufao::HttpServerRequest *request,
                                   Tufao::HttpServerResponse *response,
                                   const QStringList &args = QStringList());

                bool Data(QByteArray);
                //! \brief End of request: triggered when all data/body downloaded from the socket
                QString EndOfRequest();
                //! \brief Evaluate the script and return the result via response
                QString Evaluate(QString lScript, Tufao::HttpServerRequest *request, Tufao::HttpServerResponse *response);

        };
    }
}

#endif // GQTL_WEBSERVER_PRIV_H
