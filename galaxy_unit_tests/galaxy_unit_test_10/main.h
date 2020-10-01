#ifndef MAIN_H
#define MAIN_H

#include <QThread>

class MyThread : public QThread
{
    Q_OBJECT
public:
    void run();
    //{
      //  TestPi();
    //}
};


#endif // MAIN_H
