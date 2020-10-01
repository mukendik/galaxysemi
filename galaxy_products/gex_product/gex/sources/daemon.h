#ifndef	DAEMON_H
#define DAEMON_H

///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "engine.h"
#include <gqtl_service.h>
//#include <gqtl_log.h>

#include <QApplication>

extern int	MainInit();

namespace GS
{
namespace Gex
{

//////////////////////////////////////////	/////////////////////////////////////////
//
// Name			:	Daemon

// Description	:	Base class which manages the service for GalaxySemi Applications
//
///////////////////////////////////////////////////////////////////////////////////
class Daemon : public QObject, public QtServiceBase
{
    Q_OBJECT

    static Daemon* sInstance;
public:

    Daemon(int argc, char **argv, const QString &name, const QString& description);
    ~Daemon();
    // Fix me : cant we apply the singleton pattern ?
    // so moving the constructor to private to prevent to instantiate several instances ?
    static Daemon* GetInstance() { return sInstance; }

protected:

//    QApplication *  application() const;

    void            createApplication(int &argc, char **argv);

    int             executeApplication();

protected:

    void start();   // Function which is called when the service starts
    void stop();	// Function which is called when the service stops
    void pause();	// Function which is called when the service goes to pause
    void resume();	// Function which is called when the service resumes

private:

//    QApplication *  mApp;
signals:
    // Emited when stop() is called
    void sStopping();
};

}   // namespace Gex
}   // namespace GS

#endif // DAEMON_H
