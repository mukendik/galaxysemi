#ifndef LOADING_H
#define LOADING_H

#include <QLabel>
#include <QMovie>
#include <QElapsedTimer>
#include <QCoreApplication>
#include "engine.h"

class Loading
{

public:

    Loading();
    ~Loading();
    void start (int lTimeoutmsecond);
    void stop();

    void update(QString lLable);
    void update(/*int lStep*/);

private:
    QLabel* mLabel;
    QMovie* mMovie;
    QElapsedTimer* mTimer;
    int     mTimeOut;

};


#endif // LOADING_H

