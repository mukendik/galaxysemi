#include <QtCore/QCoreApplication>
#include <QTime>
#include "cqtest.h"

class CTest
{
    int mNumber;
   public:
    CTest() {};
};

class GObject
{
    public:
        GObject() { };
};

class CGTest : public GObject
{
    int mNumber;
   public:
    CGTest():GObject() {  };
};


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug("Testing without QObject inheritance...");
    qDebug("CTest has no inheritance and has 1 integer member (sizeof(CTest)=%d)", sizeof(CTest) );

    int k=10000;
    QList<CTest*> l;
    QTime t; t.start();
    for (int i=0; i<k; i++)
     l.push_back(new CTest);
    qDebug("Allocating %d instances of CTest needs %d ms", k, t.elapsed() );
    qDebug("%d CTest use: %do of mem", k, k*sizeof(CTest));
    foreach(CTest* t, l)
        if (t) delete t;
    l.clear();


    qDebug("\nCQTest inherites from QObject and has 1 integer member/property: sizeof(CQTest)=%d octet", sizeof(CQTest));
    qDebug("Testing instancing...");
    QList<CQTest*> lq;
    t.start();
    for (int i=0; i<k; i++)
     lq.push_back(new CQTest);
    qDebug("Instancing %d CQTest needs %d ms", k, t.elapsed());
    qDebug("%d CQTest use %do of mem", k, k*sizeof(CQTest));
    foreach(CQTest* qt, lq)
        if (qt)
            delete qt;

    qDebug("\nTesting GObject inheritance...");
    QList<CGTest*> lgt;
    t.start();
    for (int i=0; i<k; i++)
     lgt.push_back(new CGTest);
    qDebug("CGTest %d ms", t.elapsed());
    qDebug("CGTest memuse: %do", k*sizeof(CGTest));
    foreach(CGTest* gt, lgt)
        if (gt)
            delete gt;


    //return a.exec();
    return 0;
}
