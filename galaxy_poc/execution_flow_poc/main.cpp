#include <QtCore/QCoreApplication>
#include <QList>
#include <stdio.h>

class GReturn
{
    static QList<GReturn*> sStack;
    static int sNumInstances;
public:
    GReturn(const GReturn &c)
    {
        mOk=c.mOk;
        mFunction=c.mFunction;
        mMessage=c.mMessage;
        mChild=c.mChild;
        GReturn::sNumInstances++;
        printf("GReturn::GReturn(copy) (%d instances)\n", GReturn::sNumInstances);
    }

    GReturn(bool ok, const char* msg, GReturn* child, const char* f)
    {
        GReturn::sNumInstances++;
        mOk=ok;
        mFunction=QString(f);
        mMessage=QString(msg);
        if (child)
            mChild=child; //new GReturn(*child);
        else
            mChild=0;

        /*
        if (!ok)
            foreach(GReturn* r, sStack)
            {   GReturn sr=*r;
                mStack.push_back(sr);
            }
        */
        sStack.append(this);
    };

    ~GReturn()
    {
        if (mChild)
            delete mChild;
        sStack.removeAll(this);
        GReturn::sNumInstances--;
        printf("GReturn::~GReturn: still %d instances\n", GReturn::sNumInstances);
    };

    bool mOk;
    //QList<GReturn> mStack;
    GReturn *mChild;
    QString mFunction;
    QString mMessage;

    void LogStack()
    {
        printf("%s:%s\n%c", mFunction.toLatin1().data(), mMessage.toLatin1().data(), mChild?'\t':' ');
        if (mChild)
            mChild->LogStack();
        /*
        foreach(GReturn r, mStack)
            printf("\t%s\n", r.mFunction.toLatin1().data());
        */
    };

    static void PrintStack()
    {
        int i=0;
        foreach(GReturn* r, sStack)
        {
            printf("\t%d:%s\n", i++, r?r->mFunction.toLatin1().data():"?");
        }
    }
};

int GReturn::sNumInstances=0;
QList<GReturn*> GReturn::sStack;

#define _STR(X) (X)_list
#define STR(x) _STR(x)
//#define GADDRETURN(r) static QList<GReturn> STR(__func__); STR(__func__).insert(r);

#define GRETURN(BOOL, MSG, GR) return new GReturn(BOOL, MSG, (GR), __func__);
//#define GRETURN(BOOL, MSG, GR) return new GReturn(BOOL, MSG, (GR), __PRETTY_FUNCTION__);

//#define GRETURN(BOOL, MSG)  return new GReturn(BOOL, MSG, 0, __PRETTY_FUNCTION__);








GReturn* DoSomething3()
{
    //GReturn::PrintStack();
    GRETURN(false, "failed to do something 3", 0);
}

GReturn* DoSomething2()
{
    GReturn* r=DoSomething3();
    //GReturn::PrintStack();
    GRETURN(r->mOk, r->mOk?"ok":"failed to ...", r);
}

GReturn* DoSomething1()
{
    GReturn* r=DoSomething2();
    //GADDRETURN(r);
    if (r && r->mOk)
        GRETURN(true, "DoSomething1 ok", 0)
    else
        GRETURN(false, "DoSomething1 failed", r);
}


int main(int argc, char *argv[])
{
    qDebug("main");

    GReturn* r=DoSomething1();
    //if (r.mOk)
    r->LogStack();
    delete r;

    //r.LogStack();

    //QCoreApplication a(argc, argv);

    //return a.exec();
    return 0;
}
