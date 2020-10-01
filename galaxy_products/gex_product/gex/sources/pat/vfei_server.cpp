#include "vfei_server.h"
#include "vfei_config.h"
#include "vfei_client.h"
#include "gqtl_log.h"

namespace GS
{
namespace Gex
{

class VFEIServerPrivate
{
public:
    VFEIServerPrivate();
    ~VFEIServerPrivate();

    VFEIConfig          mConfig;
    QList<VFEIClient*>  mClients;
};

VFEIServerPrivate::VFEIServerPrivate()
{
}

VFEIServerPrivate::~VFEIServerPrivate()
{
}

VFEIServer::VFEIServer(QObject * parent)
    : QTcpServer(parent), mPrivate(new VFEIServerPrivate())
{
    setObjectName("GSVFEIServer");
}

VFEIServer::~VFEIServer()
{
    // Disconnect all clients before stopping the server
    while(mPrivate->mClients.isEmpty() == false)
        mPrivate->mClients.takeFirst()->DisconnectFromHost();

    Stop();
    delete mPrivate;
}

bool VFEIServer::Init(const QString& lConfigFile)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Initialyzing VFEI Server");

    QString lErrorMessage;
    if (mPrivate->mConfig.LoadFromFile(lConfigFile, lErrorMessage) == false)
    {
        GSLOG(SYSLOG_SEV_WARNING, lErrorMessage.toLatin1().constData());
        return false;
    }

    return true;
}

bool VFEIServer::Start()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Starting VFEI Server");

    if (isListening())
    {
        GSLOG(SYSLOG_SEV_ERROR, "VFEI Service already running");
        return false;
    }

    QString lErrorMessage;
    int     lSocketPort = -1;

    if (mPrivate->mConfig.IsValid(lErrorMessage) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
        return false;
    }

    for(int lIdx = 0; lIdx < mPrivate->mConfig.GetMaxAllowedInstances() && lSocketPort == -1; lIdx++)
    {
        // Try to bind to given port
        if(listen(QHostAddress::Any, mPrivate->mConfig.GetSocketPort() + lIdx) == true)
            lSocketPort = mPrivate->mConfig.GetSocketPort() + lIdx;

    }

    // Binding failed: all ports allowed are already in use
    if(lSocketPort == -1)
    {
        if(mPrivate->mConfig.GetMaxAllowedInstances() > 1)
        {
            lErrorMessage   = "Failed to bind to any port between ";
            lErrorMessage   += QString::number(mPrivate->mConfig.GetSocketPort());
            lErrorMessage   += " and ";
            lErrorMessage   += QString::number(mPrivate->mConfig.GetSocketPort() + mPrivate->mConfig.GetMaxAllowedInstances() - 1);
            lErrorMessage   += " .Ports already in use";
        }
        else
        {
            lErrorMessage   = "Failed to bind to port " + QString::number(mPrivate->mConfig.GetSocketPort());
            lErrorMessage   += "...: Port may be used by another application, or server already running";
        }

        GSLOG(SYSLOG_SEV_CRITICAL, lErrorMessage.toLatin1().constData());
        return false;
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("TCP/IP VFEI Server socket bound to port %").arg(lSocketPort).toLatin1().data());

    // Signal/Slot connections
    connect(this, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));

    return true;
}

void VFEIServer::Stop()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Stopping VFEI Server");

    if (isListening())
        close();
}

bool VFEIServer::IsRunning() const
{
    return isListening();
}

void VFEIServer::OnNewConnection()
{
    while(hasPendingConnections() == true)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Receiving new VFEI connection");

        mPrivate->mClients.append(new VFEIClient(nextPendingConnection(), mPrivate->mConfig));
    }
}

}   // namespace Gex
}   // namespace GS
