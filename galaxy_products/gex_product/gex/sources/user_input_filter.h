#ifndef USER_INPUT_FILTER_H
#define USER_INPUT_FILTER_H

#include <QObject>
#include <QTimer>

namespace GS
{
namespace Gex
{

class UserInputFilter : public QObject
{
    Q_OBJECT

public:

    static UserInputFilter *geLastInstance();
    UserInputFilter(QObject * parent = NULL);
    virtual ~UserInputFilter();


    void resetValueTimerBeforeAutoClose(QString &newValue);

protected:

    bool eventFilter(QObject * obj, QEvent * event);
public slots:
    void checkAutoClose();
    void initAutoCloseTimer();

private:
    QTimer mTimerAutoClose;
    static UserInputFilter *mLastInstance;

    int  mCurrentTimerBeforeAutoClose;


};
}
}
#endif // USER_INPUT_FILTER_H
