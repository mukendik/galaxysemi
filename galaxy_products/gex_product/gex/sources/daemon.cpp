#include "engine.h"
#include "daemon.h"
#include <gqtl_service.h>
#include <gqtl_log.h>

#include <QGuiApplication>

extern int	MainInit();

namespace GS
{
namespace Gex
{

Daemon* Daemon::sInstance=0;

Daemon::Daemon(int argc, char **argv, const QString& name, const QString& description)
    : QObject(0), QtServiceBase(argc, argv, name)
{
    sInstance=this;
    GSLOG(5, "Creating daemon...");
    QtServiceBase::setServiceDescription(description);
    GSLOG(6, "Setting CanBeSuspended flag...");
    QtServiceBase::setServiceFlags(QtServiceBase::CanBeSuspended);
    GSLOG(6, "Setting AutoStartup");
    QtServiceBase::setStartupType(QtServiceController::AutoStartup);
}

Daemon::~Daemon()
{
    GSLOG(5, "Destroying daemon...")
}

void Daemon::start()
{
    GSLOG(5, "Starting daemon...");

    if (MainInit() == 0)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Posting event START...");
        // Issue : on Linux the daemon does not enter into Engine::OnStart()
        // Perhaps the event is never posted or never processed or what else ?
        // TODO1 : Don't we have to use sendEvent() instead of postEvent() ?
        // TODO2 : add a priority to postEvent : Qt::HighEventPriority
        QCoreApplication::postEvent(&Engine::GetInstance(),
          new QEvent(QEvent::Type(Engine::EVENT_START)));

//        QCoreApplication::processEvents(); // To be sure the event is processed ?
    }
    else
    {
        logMessage("GS Daemon failed to start", Error);
        GSLOG(3, "Daemon MainInit failed");
        QCoreApplication::exit(EXIT_FAILURE);
    }
}

void Daemon::pause()
{
    GSLOG(5, "Pausing daemon...");
}

void Daemon::resume()
{
    GSLOG(5, "Resuming daemon...");
}

void Daemon::stop()
{
    GSLOG(5, QString("Stopping daemon (args %1)...").arg(QGuiApplication::arguments().join(" ") ).toLatin1().data() );
    emit sStopping();
//    QCoreApplication::processEvents();
}

//QApplication* Daemon::application() const
//{
//    return mApp;
//}

int Daemon::executeApplication()
{
    GSLOG(5, "Daemon executeApplication...")
    return QGuiApplication::exec();
}

void Daemon::createApplication(int &argc, char **argv)
{
    GSLOG(5, "Creating app...");
    new QGuiApplication(argc, argv);
}

}   // namespace Gex
}   // namespace GS
