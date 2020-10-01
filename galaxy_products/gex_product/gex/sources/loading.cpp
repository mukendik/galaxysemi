
#include "loading.h"

Loading::Loading()
{
    mLabel = new QLabel();
    mMovie = new QMovie();
    mTimer = new QElapsedTimer();
}

Loading::~Loading()
{
    delete mLabel;
    delete mMovie;
    delete mTimer;
}


void Loading::update(QString lLabel)
{
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(lLabel);
    QCoreApplication::processEvents();
}

void Loading::update(/*int lStep*/)
{
    if(mTimer == 0)
        return;

    if(mTimeOut - mTimer->elapsed() <= 0 )
    {
        mTimer->restart();
        //if(lStep)
        //    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true, 100, (lStep*100)/lMax);
        QCoreApplication::processEvents();
    }
}

void Loading::start(int lTimeoutmsecond)
{
    if(mTimer == 0)
        mTimer =  new QElapsedTimer();

    mLabel->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    mMovie->setFileName(":/gex/icons/loading.gif");
    mMovie->setCacheMode(QMovie::CacheAll);
    mLabel->setMovie(mMovie);
    mMovie->start();
    mLabel->show();

    mTimer->start();
    mTimeOut = lTimeoutmsecond;
}

void Loading::stop()
{
    mMovie->stop();
    mLabel->hide();
    delete mTimer;
    mTimer =0;
    GS::Gex::Engine::GetInstance().HideProgress();
    GS::Gex::Engine::GetInstance().UpdateLabelStatus();
}
