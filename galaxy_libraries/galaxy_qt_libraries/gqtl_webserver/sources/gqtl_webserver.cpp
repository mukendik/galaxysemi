#include "gqtl_webserver.h"
#include <gqtl_log.h>
#include <QObject>
#include <QVariant>
#include <QMetaProperty>
// tufao
#include <HttpServer>
#include <HttpServerRequest>
#include <HttpServerRequestRouter>
#include <HttpServerResponse>
#include <Headers>
//
#include "gqtl_webserver_priv.h"

namespace GS
{
namespace QtLib
{
    WebServer::WebServer(QObject *parent) :  QObject(parent) //, mScriptEngine(lSE)
    {
        //setProperty("Port", lPort);
        //if (lSE.isEvaluating())
          //  GSLOG(4, "The ScriptEngine is already evaluating");

        mPrivate=new WebServerPrivate(this);
    }

    void WebServer::SetScriptEngine(QScriptEngine* lSE)
    {
        if (mPrivate)
            mPrivate->mScriptEngine=lSE;
        else
            GSLOG(3, "Private null");
    }

    QString WebServer::Listen(const QHostAddress &lAddress, quint16 lPort)
    {
        if (!mPrivate)
            return "error: private not ready";
        bool lR=mPrivate->mServer.listen(lAddress, lPort);
        //Tufao::HttpServer* mServer=(Tufao::HttpServer*)userData(0);
        //qDebug("Server %s", mServer->objectName().toLatin1().data());
        return lR?"ok":"error: cannot listen";
    }

    bool WebServer::IsListening()
    {
        if (!mPrivate)
            return false;
        else
            return (mPrivate->mServer.isListening());

    }

    void WebServer::Close()
    {
        mPrivate->mServer.close();
        return;
    }

}
}
