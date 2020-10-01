#ifndef MY_EVENT_H
#define MY_EVENT_H

#include <QEvent>

class MyEvent
        : public QEvent
{
public:

    enum Type
    {
        UserSyncTask = User +1,
        UserAsyncTask = User +2
    };

    MyEvent(Type type, int lDuration);
    ~MyEvent();

    int     GetDuration() const;

private:

    int     mDuration;
};

#endif // MY_EVENT_H
