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
#include <QTimer>
#include <HttpServer>
#include <HttpServerRequestRouter>

#include "mainhandler.h"

int main(int argc, char *argv[])
{
    printf("main : Build with Qt '%s'...\n", QT_VERSION_STR);
    QCoreApplication a(argc, argv);
    printf("Creating server...\n");
    Tufao::HttpServer server;
    printf("Creating router...\n");
    Tufao::HttpServerRequestRouter router;
    printf("Creating main handler...\n");
    MainHandler h;
    printf("Setting router...\n");
    router.map(QRegExp("^/([^/]+)$"), &h).map(QRegExp(".*"), &h);

    printf("Connecting server and router...\n");
    bool lR=QObject::connect(
                &server, SIGNAL(requestReady(Tufao::HttpServerRequest*,Tufao::HttpServerResponse*)),
                &router, SLOT(handleRequest(Tufao::HttpServerRequest*,Tufao::HttpServerResponse*)));
    if (!lR)
    {
        qDebug("Connection of requestReady with handleRequest failed");
        return EXIT_FAILURE;
    }
    printf("Launching listening...\n");
    lR=server.listen(QHostAddress::Any, 8008);
    if (!lR)
    {
        printf("Server listen failed...\n");
        return EXIT_FAILURE;
    }

    QTimer lTimer;
    QObject::connect(&lTimer, SIGNAL(timeout()), &h, SLOT(OnClose()) );
    lTimer.start(1000);

    int lRet=a.exec();

    printf("App exec returned %d\n", lRet);

    return lRet;
}
