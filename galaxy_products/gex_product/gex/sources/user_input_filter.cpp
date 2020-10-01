#include "user_input_filter.h"
#include "engine.h"
#include "gqtl_log.h"
#include "report_options.h"
#include <QApplication>

extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

namespace GS
{
namespace Gex
{

UserInputFilter *UserInputFilter::mLastInstance = 0;

UserInputFilter *UserInputFilter::geLastInstance()
{
    return mLastInstance;
}

UserInputFilter::UserInputFilter(QObject *parent)
    : QObject(parent)
{

    mLastInstance = this;
}

void UserInputFilter::initAutoCloseTimer()
{    

    QObject::connect(&mTimerAutoClose, SIGNAL(timeout()), this, SLOT(checkAutoClose()), Qt::UniqueConnection);

    int lTimer = ReportOptions.GetTimeBeforeAutoClose();
    mCurrentTimerBeforeAutoClose = lTimer;
    GSLOG(SYSLOG_SEV_DEBUG, QString("Setting mTimerAutoClose(%1)").arg(lTimer).toLatin1().constData() );

    if(lTimer>0)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Setting Timer Interval(%1)").arg(lTimer* 1000).toLatin1().constData() );
        mTimerAutoClose.setInterval(lTimer * 1000);
        GSLOG(SYSLOG_SEV_DEBUG, QString("mTimerAutoClose.start").toLatin1().constData() );
        mTimerAutoClose.start();
    }
}

void UserInputFilter::resetValueTimerBeforeAutoClose(QString &newValue)
{
    int lTimer = ReportOptions.GetTimeBeforeAutoClose(newValue);
    if( mCurrentTimerBeforeAutoClose != lTimer)
    {
        mCurrentTimerBeforeAutoClose = lTimer;
        GSLOG(SYSLOG_SEV_DEBUG, QString("Setting new timer interval(%1)").arg(lTimer* 1000).toLatin1().constData() );

        if(mCurrentTimerBeforeAutoClose > 0)
        {
            mTimerAutoClose.setInterval(lTimer * 1000);
            GSLOG(SYSLOG_SEV_DEBUG, QString("Timer auto close restarts").toLatin1().constData() );
            mTimerAutoClose.start();
        }
        else
        {
             GSLOG(SYSLOG_SEV_DEBUG, QString("Timer auto close stops").toLatin1().constData() );
             mTimerAutoClose.stop();
        }
    }
}

UserInputFilter::~UserInputFilter()
{
    mLastInstance = 0;

}

bool UserInputFilter::eventFilter(QObject */*obj*/, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::MouseButtonPress)
    {
        QCoreApplication::postEvent(&Engine::GetInstance(), new QEvent(QEvent::Type(Engine::EVENT_USER_ACTIVITY)));
        if(mTimerAutoClose.isActive())
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("mTimerAutoClose.start").toLatin1().constData() );
            mTimerAutoClose.start();
        }
    }

    // Don't do anything
    return false;
}

void UserInputFilter::checkAutoClose()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("autoCloseReached ").toLatin1().constData() );
    QString message = "Software was idle for too long: license has been released to other users.\nNote: You can change the timeout value from the 'Options' tab,\nin the section 'Environment preferences'";
    GS::Gex::Engine::GetInstance().autoCloseReached(message);
}

}
}
