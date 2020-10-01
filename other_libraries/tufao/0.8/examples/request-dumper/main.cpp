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

#include <QtCore/QCoreApplication>
#include <QtCore/QRegExp>

#include <HttpServer>
#include <HttpServerRequestRouter>

#include "mainhandler.h"

int main(int argc, char *argv[])
{
    printf("main : %s...\n", QT_VERSION_STR);

    QCoreApplication a(argc, argv);
    Tufao::HttpServer server;
    Tufao::HttpServerRequestRouter router;
    MainHandler h;

    router.map(QRegExp("^/([^/]+)$"), &h).map(QRegExp(".*"), &h);

    bool lR=QObject::connect(&server, SIGNAL(requestReady(Tufao::HttpServerRequest*,Tufao::HttpServerResponse*)),
                     &router, SLOT(handleRequest(Tufao::HttpServerRequest*,Tufao::HttpServerResponse*)));

    if (!lR)
    {
        qDebug("Connection of requestReady with handleRequest failed");
        return EXIT_FAILURE;
    }

    lR=server.listen(QHostAddress::Any, 8080);
    if (!lR)
    {
        qDebug("Server listen failed...");
        return EXIT_FAILURE;
    }

    return a.exec();
}
