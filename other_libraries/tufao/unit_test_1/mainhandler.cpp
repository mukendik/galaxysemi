/*
  Copyright (c) 2012 Vinícius dos Santos Oliveira

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  */

#include "mainhandler.h"
#include <HttpServerRequest>
#include <Headers>
#include <QtCore/QStringList>
#include <QtScript/QScriptEngine>
#include <QAbstractSocket>
#include <QObject>
#include <QObjectUserData>
#include <QJsonDocument>
#include <QMetaProperty>
#include <QCoreApplication>

MainHandler::MainHandler(QObject *parent) :
    Tufao::AbstractHttpServerRequestHandler(parent)
{
    qDebug("MainHandler...");

    setProperty("Name", "WebServer");

    QScriptValue lSV=mScriptEngine.newQObject((QObject*)this ); // QScriptEngine::AutoCreateDynamicProperties ?
    if (!lSV.isNull())
        mScriptEngine.globalObject().setProperty("WebServer", lSV );

    lSV=mScriptEngine.newQObject((QObject*)QCoreApplication::instance() ); // QScriptEngine::AutoCreateDynamicProperties ?
    if (!lSV.isNull())
        mScriptEngine.globalObject().setProperty("QCoreApplication", lSV );

    lSV=mScriptEngine.newQObject((QObject*)&mScriptEngine ); // QScriptEngine::AutoCreateDynamicProperties ?
    if (!lSV.isNull())
        mScriptEngine.globalObject().setProperty("ScriptEngine", lSV );


}

bool MainHandler::Data(QByteArray lBA)
{
    //qDebug("Data coming: %s", lBA.data());
    Tufao::HttpServerRequest *request = qobject_cast<Tufao::HttpServerRequest *>(sender());
    request->setProperty("data", request->property("data").toByteArray()+lBA);
    return true;
}

bool MainHandler::EndOfRequest()
{
    Tufao::HttpServerRequest *request = qobject_cast<Tufao::HttpServerRequest *>(sender());

    qDebug("EndOfRequest: data=%s", request->property("data").toByteArray().data());

    Tufao::HttpServerResponse *response=mClients.value(request); //(Tufao::HttpServerResponse*)request->userData(0);

    response->writeHead(Tufao::HttpServerResponse::OK);
    response->headers().replace("Content-Type", "text/html; charset=utf-8");

    QScriptValue lSV=mScriptEngine.evaluate(request->property("data").toString());

    if (!lSV.isQObject())
    {
        response->end(lSV.toString().toLatin1());
        QCoreApplication::quit();
        return true;
    }

    QString lOutput("{\n");
    //
    foreach (QString key, lSV.toQObject()->dynamicPropertyNames())
    {
        lOutput.append(QString(" \"%1\":\"%2\" \n").arg(key)
                       .arg(lSV.toQObject()->property(key.toLatin1().data()).toString()) );

    }
    //
    const QMetaObject* lMO=lSV.toQObject()->metaObject();
    for(int i=0; i<lMO->propertyCount(); i++)
    {
        lOutput.append(QString(" \"%1\":\"%2\" \n").arg(lMO->property(i).name())
                       .arg(lSV.toQObject()->property( lMO->property(i).name() ).toString() ) );
    }

    lOutput.append("}");
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

    QCoreApplication::quit();

    return true;
}

bool MainHandler::handleRequest(Tufao::HttpServerRequest *request,
                                Tufao::HttpServerResponse *response,
                                const QStringList &args)
{
    qDebug("handleRequest http version %d: url:%s : %s (%d args)",
           request->httpVersion(), request->url().data(), request->method().data(), args.size() );

    if (!request || !response)
        return false;

    QObject::connect(request, SIGNAL(data(QByteArray)), this, SLOT(Data(QByteArray)) );
    QObject::connect(request, SIGNAL(end()), this, SLOT(EndOfRequest()) );

    mClients.insert(request, response);

    response->writeHead(Tufao::HttpServerResponse::OK);
    response->end("Ok. Quitting.");

    return true;
}
