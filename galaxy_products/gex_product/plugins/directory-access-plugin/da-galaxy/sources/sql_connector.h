#ifndef SQL_CONNECTOR_H
#define SQL_CONNECTOR_H

/*! \class SqlConnector
 * \brief
 *
 */

#include <QObject>
#include <QSqlQuery>
#include <QDateTime>
#include <QString>

#define SQL_DIR_TABLE       "da_galaxy"

#define SQL_DIR_FIELD_ID            "idda_galaxy"
#define SQL_DIR_FIELD_SESSION       "session_id"
#define SQL_DIR_FIELD_DIRECTORY     "directory"
#define SQL_DIR_FIELD_CHECKSUM      "checksum"
#define SQL_DIR_FIELD_LAST_UPDATE   "last_update"

namespace GS
{
namespace DAPlugin
{
class SqlConnector : public QObject
{
    Q_OBJECT
public:
    /// \brief Constructor
    SqlConnector();
    /// \brief Desctructor
    virtual ~SqlConnector();
    /// \brief Exec query
    QSqlQuery ExecQuery(const QString &query, bool &ok);
    /// \brief set host
    void SetHost(const QString& value);
    /// \brief set port
    void SetPort(const int value);
    /// \brief set sql user
    void SetUser(const QString& value);
    /// \brief set sql pass
    void SetPass(const QString& value);
    /// \brief set driver
    void SetDriver(const QString& value);
    /// \brief set db sid
    void SetDatabaseSID(const QString& value);
    /// \brief set schema name
    void SetSchemaName(const QString& value);
    /// \brief set ConnectionID shared by the application
    /// \brief da_galaxy will use the same QSqlDatabase connection
    void SetConnectionID(const QString& value);
    /// \brief return connection ID
    QString ConnectionID() const;
    /// \brief return schema name
    QString Schema() const;
    /// \brief connect
    bool Connect(const QString &dirUserId);
    /// \brief return session id
    QString GetSessionId();
    /// \brief true if sql session is active
    bool IsSessionActive(const QString &sessionId);
    /// \brief disconnect
    bool Disconnect();
    /// \brief return last error
    QString GetLastError() const;
    /// \brief return server date time
    QDateTime GetServerDateTime();
    /// \brief true if connection is valid
    bool IsDatabaseConnected();
    /// \brief translate string
    QString TranslateStringToSqlVarChar(const QString& value);
    /// \brief translate string
    QString TranslateStringToSqlLob(const QString& value);
    /// \brief true if sql server is oracle
    bool IsOracleServer();
    /// \brief true if sql server is mysql
    bool IsMySqlServer();

private:
    Q_DISABLE_COPY(SqlConnector);
    /// \brief return sql session id
    QString RetrieveSqlSessionId();
    /// \brief open connection to mysql server
    bool Connect();

    QString mHost;          ///< Holds sql host
    int     mPort;          ///< Holds sql port
    QString mUser;          ///< Holds sql user
    QString mPass;          ///< Holds sql pass
    QString mConnectionID;  ///< Holds Connection ID used in QSqlDatabase to identify connections
    QString mDatabaseSID;   ///< Holds Only for connection -->    Oracle: SID             MySQL: database name
    QString mSchema;        ///< Holds Only for query -->         Oracle: database name   MySQL: database name
    QString mDriver;        ///< Holds sql driver
    QString mLastError;     ///< Holds last error
    QString mSessionId;     ///< Holds session id

    bool mPrivateConnectionID;  ///< Holds When the application doesn't shared the Connection ID
                                /// the plugin will generate is own and used it in QSqlDatabase to identify connections
};
} // END DAPlugin
} // END GS

#endif // SQL_CONNECTOR_H
