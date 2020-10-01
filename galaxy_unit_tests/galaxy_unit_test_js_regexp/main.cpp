#include <stdlib.h>
#include <stdio.h>
#include <QString>
#include <QFile>
#include <QScriptEngine>
#include <QCoreApplication>

int main(int argc, char** argv)
{
    qDebug("main: argc=%d : %s\n", argc, argv[0]);
    qDebug("QT version %s", QT_VERSION_STR);

    #ifdef __GNUC__
        qDebug("Using exec built with GCC version %d.%d.%d",
              __GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__
              );
    #endif

    QCoreApplication lApp(argc, argv);

    QScriptEngine lSE;
    qDebug("ScriptEngine avail ext: '%s'", lSE.availableExtensions().join(',').toLatin1().data() );
    qDebug("Let's try a RegExp that should return 1 ...");
    QScriptValue lSV=lSE.evaluate("var myRE=new RegExp('\\\\d'); myRE.exec('1')");
    qDebug("Evaluation result: '%s'", lSV.toString().toLatin1().data()); // Should be 1

    if (lSV.toInt32()==1)
    {
        qDebug("Success");
        return EXIT_SUCCESS;
    }
    qDebug("Failure: 1 != '%s'", lSV.toString().toLatin1().data());
    qDebug("Error.prototype.backtrace=%s", lSE.evaluate("Error.prototype.backtrace").toString().toLatin1().data() );
    return EXIT_FAILURE;
}
