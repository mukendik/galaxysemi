#include <stdio.h>
#include <QtCore/QCoreApplication>
#include <QScriptEngine>
#include <QScriptValue>
#include <QThread>
#include <QMetaType>
#include <QMetaClassInfo>
#include <QMetaTypeId>

#include "myclass.h"

unsigned MyClass::sNumOfInstances=0;

template <typename T>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, T const &qobject)
{
    return engine->newQObject(qobject);
}

template <typename T>
void qScriptValueToQObject(const QScriptValue &value, T &qobject)
{
    qobject = qobject_cast<T>(value.toQObject());
}

template <typename T>
int qScriptRegisterQObjectMetaType(
        QScriptEngine *engine,
        const QScriptValue &prototype = QScriptValue(),
        T* /* dummy */ = 0
        )
{
    return qScriptRegisterMetaType<T>(
                engine, qScriptValueFromQObject, qScriptValueToQObject, prototype);
}

Q_DECLARE_METATYPE(MyClass*)
Q_DECLARE_METATYPE(MyClass)
Q_SCRIPT_DECLARE_QMETAOBJECT(MyClass, QObject*)

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QScriptEngine lEngine;

    QScriptValue lSV = lEngine.scriptValueFromQMetaObject<MyClass>();
    if (!lSV.isNull())
    {
        //qScriptRegisterMetaType(pGexScriptEngine, );
        int id=qScriptRegisterQObjectMetaType<MyClass*>( &lEngine );
        qDebug("MyClass registered with id %d", id);
        lEngine.globalObject().setProperty("MyClass", lSV );
    }
    else
    {
        return EXIT_FAILURE;
    }

    qDebug("Size of MyClass: %d o", sizeof(MyClass));

    // 250Mo without collectGarbage
    // 10Mo with collectGarbage
    // 4Mo if only 1 evaluate done
    for (int i=0; i<10; i++)
    {
        QScriptValue lSV=lEngine.evaluate("var i=new MyClass()");
        if (lEngine.hasUncaughtException())
        {
            qDebug("Exception: %s", lEngine.uncaughtException().toString().toLatin1().data());
            return EXIT_FAILURE;
        }
        printf(".");
        //system("pause");
        lEngine.collectGarbage();
    }

    system("pause");

    return 0; //a.exec();
}
