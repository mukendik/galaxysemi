#ifndef MYCLASS_H
#define MYCLASS_H

#include <QObject>

class MyClass:public QObject
{
    Q_OBJECT
public:
    MyClass(QObject* parent=0):QObject(parent)
    {
        sNumOfInstances++;
    }
    MyClass(const MyClass& o):QObject(o.parent()) { *this=o; }
    MyClass& operator=(const MyClass& o)
    {
        this->setParent(o.parent());
        return *this;
    }
    ~MyClass()
    {
        sNumOfInstances--;
        qDebug("~MyClass: now %d instances", sNumOfInstances);
    }
    int mData[8000];
    static unsigned sNumOfInstances;
};


#endif // MYCLASS_H
