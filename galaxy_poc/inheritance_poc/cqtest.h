#ifndef CQTEST_H
#define CQTEST_H

#include <QObject>

class CQTest : public QObject
{
    Q_OBJECT
    // USER true
    Q_PROPERTY(int mNumber READ GetNumber)

    int mNumber;
   public:
    CQTest() : QObject() { };
    const int GetNumber() { return mNumber; };
};


#endif // CQTEST_H

