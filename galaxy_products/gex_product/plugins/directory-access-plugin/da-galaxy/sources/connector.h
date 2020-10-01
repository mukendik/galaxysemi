#ifndef CONNECTOR_H
#define CONNECTOR_H

/*! \class Connector
 * \brief
 *
 */

#include <dir_access_base.h>

#include <QStringList>
#include <QDomNode>
#include <QMap>

#define PARAM_DIR_USER      "dir_user"
#define PARAM_DIR_USER_PASS "dir_pass"

#define PARAM_SQL_HOST      "sql_host"
#define PARAM_SQL_PORT      "sql_port"
#define PARAM_SQL_USER      "sql_user"
#define PARAM_SQL_PASS      "sql_pass"
#define PARAM_SQL_DBSID     "sql_database_sid"
#define PARAM_SQL_SCHEMA    "sql_shema"
#define PARAM_SQL_DRIVER    "sql_driver"
#define PARAM_SQL_CONNECTID "sql_connection_id"

class ConnectorPrivate;

namespace GS
{
namespace DAPlugin
{

class Connector : public ConnectorBase
{
    Q_OBJECT
public:
    /// \brief Constructor
    Connector();
    /// \brief Desctructor
    virtual ~Connector();
    /// \brief open connection to da
    bool Connect(const QMap<QString, QString> &parameters);
    /// \brief reconnect without specifying parameter
    bool Reconnect();
    /// \brief disconnect
    bool Disconnect();
    /// \brief change connected user without deconnection
    bool ChangeUser(const QMap<QString, QString> &parameters);
    /// \brief return last error
    QString GetLastError();
    /// \brief true if user connected
    bool IsConnected();
    /// \brief return server datatime
    QDateTime GetServerDateTime();
    /// \brief return SQL Session Id
    QString GetSessionId();
    /// \brief check if evrything is OK and try repair
    bool ValidityCheck();

public slots:
    /// \brief save changes to db
    bool OnApplyChangesRequested();
    /// \brief Try to open connection
    void OnConnectionRequested();
    /// \brief revert changes
    void OnCancelChangesRequested();
    /// \brief open /close edit session
    void OnChangeSessionStateRequested(bool openSession);
    /// \brief return connected user
    QString GetCurrentUser();

private slots:
    /// \brief connect
    bool Connect();

signals:
    /// \brief emitted when new users are loaded
    void sUsersLoaded(const QDomNode& usersNode);
    /// \brief emitted when new groups are loaded
    void sGroupsLoaded(const QDomNode& groupsNode);
    /// \brief emitted when new app entries are load
    void sAppEntriesLoaded(const QDomNode& appEntriesNode);
    /// \brief emitted when data have been updated outside
    void sDataUpdated();
    /// \brief emitted when connection state change
    void sConnectionStateChanged(bool connectionIsOk);
    /// \brief emitted when session edit is opened or closed
    void sSessionStateChanged(bool sessionIsOpened);
    /// \brief emitted when someone else opened/closed an edit session
    void sOtherSessionStateChanged(QString sessionId);

private:
    Q_DISABLE_COPY(Connector);

    /// \brief load connection parameters
    bool LoadConParameters(QMap<QString, QString> parameters);
    /// \brief check user validity
    bool CheckUserValidity(const QString& login, const QString& password);
    /// \brief load input data
    bool LoadInputData();
    /// \brief build superuser dynamic password
    QString SuperUserPass();

    ConnectorPrivate* mPrivate; ///< Holds private data
};

} // END DAPlugin
} // END GS

#endif // CONNECTOR_H
