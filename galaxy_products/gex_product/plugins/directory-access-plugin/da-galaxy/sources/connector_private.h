#ifndef CONNECTOR_PRIVATE_H
#define CONNECTOR_PRIVATE_H


/*! \class ConnectorPrivate
 * \brief
 *
 */

#include <QTimer>

#include "sql_connector.h"
#include "dir_file.h"

class QString;

namespace GS
{
namespace DAPlugin
{
class ConnectorPrivate
{
public:
    /// \brief Constructor
    ConnectorPrivate();
    /// \brief Destructor
    virtual ~ConnectorPrivate();

    bool            mConnected;             ///< Holds true if connection opened
    QString         mLastError;             ///< Holds last error
    SqlConnector    mSqlConnector;          ///< Holds sql connector
    DirFile         mDirFile;               ///< Holds dir file data
    QString         mUserId;                ///< Holds user id of connected user
    QString         mUserPass;              ///< Holds user pass of connected user
};

} // END DAPlugin
} // END GS

#endif // CONNECTOR_PRIVATE_H
