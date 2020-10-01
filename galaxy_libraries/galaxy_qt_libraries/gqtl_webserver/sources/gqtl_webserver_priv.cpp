#include "gqtl_webserver.h"
#include <gqtl_log.h>
#include <gqtl_utils.h>

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
    WebServerPrivate::WebServerPrivate(QObject* p):
        Tufao::AbstractHttpServerRequestHandler(p), mServer(this), mScriptEngine(0)
    {
        setObjectName("WebServerPrivate");
        GSLOG(5, "new QtLib WebServer...");
        mRouter.map(QRegExp("^/([^/]+|[^/]+\\(.*\\))$"), this).map(QRegExp(".*"), this);

        GSLOG(5, "segfault tracker n°1...");
        bool lR=QObject::connect(&mServer, SIGNAL(requestReady(Tufao::HttpServerRequest*,Tufao::HttpServerResponse*)),
                         &mRouter, SLOT(handleRequest(Tufao::HttpServerRequest*,Tufao::HttpServerResponse*)));

        GSLOG(5, "segfault tracker n°2...");
        if (!lR)
            GSLOG(3, "Failed to connect");
    }

    bool WebServerPrivate::Data(QByteArray lBA)
    {
        GSLOG(5, QString("Data coming: '%1'").arg(lBA.data()).toLatin1().data() );
        Tufao::HttpServerRequest *request = qobject_cast<Tufao::HttpServerRequest *>(sender());
        if (!request)
        {
            GSLOG(3, QString("Cannot find request in cache: '%1'").arg(lBA.data()).toLatin1().data() );
            return false;
        }
        request->setProperty("data", request->property("data").toByteArray()+lBA);
        return true;
    }

    bool WebServerPrivate::handleRequest(Tufao::HttpServerRequest *request,
                                    Tufao::HttpServerResponse *response,
                                    const QStringList &args)
    {
        if (!request)
        {
            GSLOG(3, "Received a null request");
            return false;
        }

        GSLOG(5, QString("Handle Request http version %1: url:%2 : '%3' request name: '%4' : %5 headers")
              .arg(request->httpVersion()).arg(request->url().data()).arg(request->method().data())
              .arg(request->objectName()).arg(request->headers().size())
              .toLatin1().data()
              );
        GSLOG(5, QString("Args: %1: %2").arg(args.size()).arg(args.join(", ")).toLatin1().data() );

        request->setObjectName("request");

        foreach(QByteArray lKey, request->headers().keys() )
        {
            GSLOG(7, QString("%1 %2").arg(lKey.data()).arg(request->headers().value(lKey).data())
                  .toLatin1().data() );
        }

        if (!mClients.contains(request))
        {
            GSLOG(5, "Registering new request and connecting signals/slots...");
            bool lR=QObject::connect(request, SIGNAL(data(QByteArray)), this, SLOT(Data(QByteArray)) );
            if (!lR)
                GSLOG(3, "Cannot connect request data to WebServer Data slot");
            lR=QObject::connect(request, SIGNAL(end()), this, SLOT(EndOfRequest()) );
            if (!lR)
                GSLOG(3, "Cannot connect request end to WebServer EndOfRequest slot");
        }
        else
        {
            GSLOG(5, "Request already registered...");
        }

        mClients.insert(request, response);

        if (request->method()=="GET")
        {
            // no body to expect
            if ( (args.size()<1) || (args[0].isEmpty()) )
            {
                if (!response)
                {
                    GSLOG(3, "null response. Cannot reply.");
                    return false;
                }
                response->setObjectName("response");
                response->writeHead(Tufao::HttpServerResponse::OK);
                response->headers().replace("Content-Type", "text/html; charset=utf-8");
                response->end("{ \"status\":\"error\" \n"
                    " \"comment\":\"GET command must have an argument: example: http://192.168.1.1:8080/GSengine \"\n }");
                return false;
            }
            //  the request should be in the url
            response->writeHead(Tufao::HttpServerResponse::OK);
            response->headers().replace("Content-Type", "text/html; charset=utf-8");
            QString lScript=args.at(0);
            QString lR=Evaluate(lScript, request, response);
            if (lR.startsWith("err"))
                 return false;
            return true;
        }
        else
        {
            GSLOG(5, "Request method POST. Waiting for full data...");
        }
        // clear in case of any previous request
        request->setProperty("data", QVariant());

        //QByteArray lBody=request->socket()->readAll();
        //qDebug("Body= %s", lBody.data());

        return true;
    }


    QString WebServerPrivate::Evaluate(QString lScript,
                                       Tufao::HttpServerRequest *request, Tufao::HttpServerResponse *response)
    {
        if (!request)
            return "error: null request";
        if (!response)
            return "error: null response";
        if (!mScriptEngine)
        {
            response->end("{ \"status\" :\"error\" \n \"comment\" : \"ScriptEngine null\" }");
            return "error: ScriptEngine null";
        }
        if (lScript.isEmpty())
        {
            response->end("{ \"status\" :\"error\" \n \"comment\" : \"empty script\" }");
            return "error: empty script";
        }

        GSLOG(5, QString("Evaluating : '%1'").arg(lScript).toLatin1().data() );

        QScriptValue lSV=mScriptEngine->evaluate(lScript);

        if (lSV.isError() || mScriptEngine->hasUncaughtException())
        {
            GSLOG(4, mScriptEngine->uncaughtException().toString().toLatin1().data());
            response->end(mScriptEngine->uncaughtException().toString().toLatin1());
            return "error: "+mScriptEngine->uncaughtException().toString();
        }

        if (!lSV.isQObject())
        {
            response->end(lSV.toString().toLatin1());
            return "ok";
        }

        QString lOutput("{\n");

        QString lIndent("  ");
        bool lHierarchical=false;
        if (request->headers().contains(QByteArray("hierarchical")))
            if (request->headers().value(QByteArray("hierarchical"))==QByteArray("true"))
                lHierarchical=true;
        QString lRes=GS::QtLib::QObjectToJSON(lSV.toQObject(), lOutput, lHierarchical, lIndent);
        if (lRes.startsWith("err"))
            GSLOG(4, QString("QObjectToJSON failed: %1").arg(lRes).toLatin1().data() );

        lOutput.append("}\n");
        response->end(lOutput.toLatin1());

        //QJsonDocument lJD=QJsonDocument::fromVariant(lSV.toVariant());
        //response->end(lJD.toJson());

        /*
            (*response) << "<html><head><title>Request dumper</title></head><body>"
                           "<p>Method: " << request->method() << "</p><p>Path: "
                        << request->url() << "</p><p>Version: HTTP/1."
                        << QByteArray(1, request->httpVersion() + '0')
                        << "</p><p>Headers:</p><ul>";

            {
                Tufao::Headers headers = request->headers();
                for (Tufao::Headers::const_iterator i = headers.begin()
                     ;i != headers.end();++i) {
                    (*response) << "<li>" << i.key() << ": " << i.value();
                }
            }
            (*response) << "</ul><p>Args:</p><ul>";

            for (int i = 0;i != args.size();++i) {
                (*response) << "<li>" << args[i].toUtf8();
            }

            response->end("</ul></body></html>");
        */

        return "ok";
    }

    QString WebServerPrivate::EndOfRequest()
    {        
        // fix me: this message sometimes appear in console:
        // content-type missing in HTTP POST, defaulting to application/x-www-form-urlencoded. Use QNetworkRequest::setHeader() to fix this problem

        Tufao::HttpServerRequest *request = qobject_cast<Tufao::HttpServerRequest*>(sender());
        GSLOG(7, QString("WebServer: EndOfRequest: request: %1").arg(request?"ok":"null").toLatin1().data() );

        if (!request)
        {
            //response->end("{ \"status\" :\"error\" \n \"comment\":\"cannot retrieve original request\" }");
            GSLOG(3, "cannot retrieve original request");
            return QString("error: cannot retrieve original request");
        }

        Tufao::HttpServerResponse *response=mClients.value(request);
        if (!response)
        {
            //request->socket()->close();
            GSLOG(3, "Cannot retrieve corresponding response");
            return "error: cannot retrieve corresponding response";
        }

        // The request must be removed from the list.
        mClients.remove(request);

        response->writeHead(Tufao::HttpServerResponse::OK);
        response->headers().replace("Content-Type", "text/html; charset=utf-8");

        if (!mScriptEngine)
        {
            GSLOG(3, "Cannot evaluate: script engine null");
            response->end("{ \"status\" : \"error\" \n \"comment\" : \"cannot evaluate: script engine null\" }");
            return QString("error: cannot evaluate: script engine null");
        }

        if (mScriptEngine->isEvaluating())
        {
            GSLOG(4, "cannot evaluate: script engine busy");
            response->end("{ \"status\" :\"error\" \n \"comment\" : \"cannot evaluate: script engine busy\" }");
            return QString("error: cannot evaluate: script engine already evaluating");
        }

        QVariant lJSScript=request->property("data");
        if (lJSScript.toString().isEmpty())
        {
            response->end("{ \"status\" : \"error\" \n \"comment\" : \"request body empty. Nothing to evaluate\" }");
            return QString("error: request body empty. Nothing to evaluate");
            // the request should be in the GS url : example: http://192.168.0.1:8080/GSEngine
            //lJSScript=request->
        }
        else
        {
            GSLOG(5, QString("End of request: data='%1'").arg(lJSScript.toByteArray().data())
              .toLatin1().data() );
            return Evaluate(lJSScript.toString(), request, response);
        }



        // check me
        //request->disconnect();

        //return "ok";
    }

}
}
