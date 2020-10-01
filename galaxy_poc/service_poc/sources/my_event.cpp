#include "my_event.h"

MyEvent::MyEvent(Type type, int lDuration)
    :   QEvent(QEvent::Type(type)), mDuration(lDuration)
{
}

MyEvent::~MyEvent()
{

}

int MyEvent::GetDuration() const
{
    return mDuration;
}
