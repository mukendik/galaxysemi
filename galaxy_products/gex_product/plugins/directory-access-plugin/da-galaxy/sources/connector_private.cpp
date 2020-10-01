#include "connector_private.h"

namespace GS
{
namespace DAPlugin
{
ConnectorPrivate::ConnectorPrivate()
{
    mConnected = 0;
    mLastError = "";
}

ConnectorPrivate::~ConnectorPrivate()
{
}

} // END DAPlugin
} // END GS

