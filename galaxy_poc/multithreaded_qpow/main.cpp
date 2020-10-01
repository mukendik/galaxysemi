#include <stdlib.h>
#include <stdio.h>
//#include <sys/time.h> // needed for the time() function
#include <unistd.h> // unistd also available on mingw (even if partial)

#include <QtCore/QCoreApplication>
#include <QtCore/qmath.h>
#include <QCoreApplication>
#include <QThread>

#define V1 0.12237762237762238
#define V2 3.8409090909090908
#define KS 26.235882510447745
// 26.235879973876745
// 26.2358799738767
// 26.235882510447745

class Thread : public QThread
{
public:
    Thread()
    {
        //double ks=V1*KS-V2;
        //printf("%1.15lf\n", ks);
    }

    void run()
    {
        double lST=7.0093640685081482;
        double lPow=pow(lST, 2.0);
        double lqPow=qPow(lST, 2.0);

        printf("SubThread: mt pow should return 49.131184644893104 in debug build:\npow=%2.20lf qPow=%2.20lf\n",
               lPow, lqPow );

        /*
        double v1=V1;
        double v2=V2;
        double ks=KS;
        double kurt=v1*ks-v2;
        printf("SubThread: %1.17lf\n", kurt);
        */
        //kurt=V1*KS-V2;
        //printf("SubThread: %1.17lf\n", kurt);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Thread t;
    t.start();

    double lST=7.0093640685081482;
    double lPow=pow(lST, 2.0);
    double lqPow=qPow(lST, 2.0);

    printf("Main thread: pow should return 49.131184644893096 in debug build:\npow=%2.20lf qPow=%2.20lf\n",
           lPow, lqPow );

    /*
    double v1=V1;
    double v2=V2;
    double ks=KS;
    double kurt=v1*ks-v2;
    //printf("Expected result: -0.63022416830184902\n");
    printf("Main thread: %1.17lf\n", kurt);
    */

    while (t.isRunning())
        ;

    //return a.exec();
    return 0;
}
