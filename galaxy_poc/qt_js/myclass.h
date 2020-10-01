#ifndef MYCLASS_H
#define MYCLASS_H

#include <QObject>
#include <QArrayData>
#include <QScriptValue>

class MyClass: public QObject
{
    Q_OBJECT

public:
    MyClass():QObject(0) { qDebug("MyClass");  }
    MyClass(QObject* parent):QObject(parent) { qDebug("MyClass"); }
    MyClass(const MyClass& lMC):QObject(lMC.parent()) {  } // copy constructor mandatory
    ~MyClass() { qDebug("~MyClass for %s", objectName().toLatin1().data()); }

    QString MyPublicFunction1() { qDebug("MyFunction1"); return QString("ok"); }
    Q_INVOKABLE QString MyQInvokableFunction()
    {
        qDebug("MyQInvokableFunction");
        return QString("ok");
    }

    //Q_INVOKABLE QString MyQInvokableFunction2(QMap<QVariant, QVariant>&); // not knwn by JS engine
    //Q_INVOKABLE QString MyQInvokableFunction2(QScriptValue&); // ?
    Q_INVOKABLE QString MyQInvokableFunction2(QScriptValue);

public slots:
    QString MyPublicSlotFunction1() { qDebug("MyPublicSlotFunction1"); return QString("ok"); }
    QString MyPublicSlotFunction2(MyClass* lMC)
    {
        qDebug("MyPublicSlotFunction2 %s", lMC?lMC->objectName().toLatin1().data():"null");
        return QString("ok");
    }
    QString MyPublicSlotFunction3(MyClass &lMC)
    {
        qDebug("MyPublicSlotFunction3 %s", lMC.objectName().toLatin1().data());
        return QString("ok");
    }

    QString MyPublicSlotFunction6(const QString &lS)
    {   qDebug("MyPublicSlotFunction6 %s", lS.toLatin1().data());
        return QString("ok");
    }

    QString MyPublicSlotFunction4(QString &lS)
    {   qDebug("MyPublicSlotFunction4 %s", lS.toLatin1().data());
        return QString("ok");
    }

    QString MyPublicSlotFunction5(QString *lS)
    {   qDebug("MyPublicSlotFunction5 %s", lS->toLatin1().data());
        return QString("ok");
    }

};

#endif // MYCLASS_H
