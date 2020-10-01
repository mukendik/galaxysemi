// -------------------------------------------------------------------------- //
// qscriptengine.cpp
// -------------------------------------------------------------------------- //
#include <stdio.h>
#include <QCoreApplication>
#include <QtScript/QScriptEngine>

void myMsgHandler(QtMsgType , const QMessageLogContext& , const QString& msg)
{
    fprintf(stderr, "myMsgHandler: %s\n", msg.toLocal8Bit().constData());
}

// -------------------------------------------------------------------------- //
// main
// -------------------------------------------------------------------------- //
int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QScriptEngine e;

    if (argc > 1 && strcmp(argv[1], "") != 0)
    {
        for (int i = 1; i < argc; ++i)
        {
            e.evaluate(argv[i]);
        }
        return EXIT_SUCCESS;
    }

    QtMessageHandler oldHandler = qInstallMessageHandler(myMsgHandler);
    e.evaluate("print('hello, world!')");
    qInstallMessageHandler(oldHandler);
    e.evaluate("print('hello, world!')");

    e.evaluate(
    "var d = new Date(1409238304000);"
    "print('hours from GTM = ' + d.getHours())"
    );
    e.evaluate(
    "var d = new Date(1409231104000);"
    "print('hours from local = ' + d.getHours())"
    );
    e.evaluate(
    "var d = new Date(1409238304000);"
    "print('UTC hours from GTM = ' + d.getUTCHours())"
    );
    e.evaluate(
    "var d = new Date(2014, 8 , 28, 15, 5, 4, 0);"
    "print('hours = ' + d.getHours())"
    );
    e.evaluate(
    "var d = new Date(2014, 8 , 28, 15, 5, 4, 0);"
    "print('UTC hours = ' + d.getUTCHours())"
    );
    e.evaluate(
    "var d = new Date();"
    "print('offset = ' + d.getTimezoneOffset())"
    );

    return EXIT_SUCCESS;
}
