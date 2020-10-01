#include <QDebug>
#include <QtCore/QCoreApplication>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include "myclass.h"

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
    return qScriptRegisterMetaType<T>(engine, qScriptValueFromQObject, qScriptValueToQObject, prototype);
}

QScriptValue qScriptValueFromQString(QScriptEngine *engine, QString const &qstring)
{
    Q_UNUSED(engine)
    return QScriptValue(qstring);
}
void qScriptValueToQString(const QScriptValue &value, QString &lQString)
{
    lQString = value.toString();
}

QScriptValue qScriptValueFromQStringPtr(QScriptEngine *engine, QString *lQString)
{
    Q_UNUSED(engine)
    return QScriptValue(*lQString);
}
void qScriptValueToQStringPtr(const QScriptValue &value, QString* lQString)
{
    *lQString = value.toString();
}


Q_DECLARE_METATYPE(QString)
//Q_DECLARE_METATYPE(QString&)   // impossible
Q_DECLARE_METATYPE(QString*)
Q_DECLARE_METATYPE(MyClass*)
Q_DECLARE_METATYPE(MyClass)
Q_SCRIPT_DECLARE_QMETAOBJECT(MyClass, QObject*)

QString MyClass::MyQInvokableFunction2(QScriptValue lParam)
{
    if (lParam.isNull())
        return "error: param null";
    if (!lParam.isArray())
        return "error: param not an array";
    if (lParam.isObject())
    {
        qDebug("Param is an 'Object'. Lets iterate on it...");
        QScriptValueIterator it(lParam);
        while (it.hasNext())
        {
             it.next();
             qDebug() << " " << it.name() << ": " << it.value().toString();
        }
    }

    //qScriptValueToSequence(lParam, );

    /*
    QVariantList lVarList=lParam.toVariant().toList();
    qDebug("Param as QVariantList has %d elems: '%s'", lVarList.size(), lVarList.at(0).toString().toLatin1().data() );
    QVariantMap lVarMap=lParam.toVariant().toMap();
    qDebug("Param as QVariantMap has %d elems", lVarMap.size());
    QStringList lStringList=lParam.toVariant().toStringList();
    qDebug("Param as QStringList has %d elems: '%s'", lStringList.size(), lStringList.at(0).toLatin1().data() );

    QScriptValue lSV=lParam.toObject();
    qDebug("Param as Object= %s", lSV.toString().toLatin1().data());

    QVariant lVar=lParam.toVariant();
    if (lVar.isNull())
        return "error: Variant null";
    if (!lVar.isValid())
        qDebug("Variant not valid");
    QMap<QString, QVariant> lMap=lParam.toVariant().toMap();
    foreach(QString k, lMap.keys())
    {
        qDebug(QString(" %1 ").arg(k).toLatin1().data() );
    }
    */
    return "ok";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug("QCoreApp created");

    QScriptEngine lSE;

    QScriptValue lMCSV = lSE.scriptValueFromQMetaObject<MyClass>();
    if (!lMCSV.isNull())
    {
        int id=qScriptRegisterQObjectMetaType<MyClass*>( &lSE );
        qDebug(QString("Registring MyClass* as a QObjectMetaClass : %1").arg(id).toLatin1().data() );
        lSE.globalObject().setProperty("MyClass", lMCSV );

        //id=qScriptRegisterQObjectMetaType<MyClass>( &lSE ); // impossible
        //qDebug(QString("Registring MyClass& as a QObjectMetaClass : %1").arg(id).toLatin1().data() );

        //qScriptRegisterMetaType(&lSE, qScriptValueFromQObject, qScriptValueToQObject);
        //id=qScriptRegisterQObjectMetaType<MyClass&>( &lSE ); // impossible
        //qDebug(QString("Registring MyClass as a QObjectMetaClass : %1").arg(id).toLatin1().data() );
    }
    else
        qDebug("Cannot register MyClass meta class");

    //QScriptValue lStringSV = lSE.scriptValueFromQMetaObject<QString>(); // impossible: QString does not inherit from QObject
    int lID=qScriptRegisterMetaType(&lSE, qScriptValueFromQString, qScriptValueToQString);
    qDebug("Register QString metatype: %d", lID);

    // Does not compile...
    //lID=qScriptRegisterMetaType(&lSE, qScriptValueFromQStringPtr, qScriptValueToQStringPtr);
    //qDebug("Register QString pointer metatype: %d", lID);

    MyClass lMC(0);
    lMC.setObjectName("MyClass1");

    QScriptValue lMCobject = lSE.newQObject((QObject*) &lMC );
    if (!lMCobject.isNull())
        lSE.globalObject().setProperty("GSMyClass", lMCobject);

    QScriptValue lSV=lSE.evaluate("GSMyClass");
    qDebug(lSV.toString().toLatin1().data()); // Should display : MyClass(name = "MyClass1")

    lSV=lSE.evaluate("GSMyClass.MyPublicSlotFunction1()");
    qDebug(lSV.toString().toLatin1().data());

    lSV=lSE.evaluate("GSMyClass.MyQInvokableFunction()");
    qDebug(lSV.toString().toLatin1().data());

    qDebug("Let s instantiate a MyClass in JS:");
    lSV=lSE.evaluate("var MyClass2=new MyClass(); MyClass2.objectName='MyClass2'; MyClass2");
    qDebug(lSV.toString().toLatin1().data());

    qDebug("Let s call a function with a null arg:");
    lSV=lSE.evaluate("GSMyClass.MyPublicSlotFunction2(0)");
    qDebug(lSV.toString().toLatin1().data());

    qDebug("Let s instantiate a MyClass in JS and use it in a function as a pointer:");
    lSV=lSE.evaluate("var MyClass2=new MyClass(); MyClass2.objectName='MyClass2'; GSMyClass.MyPublicSlotFunction2(MyClass2)");
    qDebug(lSV.toString().toLatin1().data());

    qDebug("Let s instantiate a MyClass in JS and use it in a function as a ref:");
    lSV=lSE.evaluate("var MyClass2=new MyClass(); MyClass2.objectName='MyClass2'; GSMyClass.MyPublicSlotFunction3(MyClass2)");
    qDebug(lSV.toString().toLatin1().data());

    qDebug("Let s call a function with a ref to a string :");
    lSV=lSE.evaluate("GSMyClass.MyPublicSlotFunction4('My string')");
    qDebug(lSV.toString().toLatin1().data());

    qDebug("Let s call a function with a pointer to a string :");
    lSV=lSE.evaluate("var lMyString=new String('sfdvs'); GSMyClass.MyPublicSlotFunction5( lMyString )");
    qDebug(lSV.toString().toLatin1().data());

    qDebug("Let s call a function with a const string ref:");
    lSV=lSE.evaluate("var lMyString=new String('sfdvs'); GSMyClass.MyPublicSlotFunction6( lMyString )");
    qDebug(lSV.toString().toLatin1().data());

    qDebug("Let s try to use an Array as parameter...");
    lSV=lSE.evaluate("var lArray=new Array(10); lArray[\"toto\"]=69; GSMyClass.MyQInvokableFunction2(lArray)");
    qDebug(lSV.toString().toLatin1().data());

    qDebug("Let s try a RegExp: should return 1");
    lSV=lSE.evaluate("var myRE=new RegExp('\\\\d'); myRE.exec('1')");
    qDebug(lSV.toString().toLatin1().data()); // Should return 1

    return 0; //a.exec();
}
