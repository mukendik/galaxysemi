#ifndef DIR_FILE_H
#define DIR_FILE_H

/*! \class DirFile
 * \brief
 *
 */

#include <QDomDocument>

#include "sql_connector.h"

#define ENCRYPTION_KEY "95F5A1F52A554"


namespace GS
{
namespace DAPlugin
{
class DirFile : public QObject
{
    Q_OBJECT
public:
    /// \brief Constructor
    DirFile();
    /// \brief Destructor
    virtual ~DirFile();
    /// \brief
    // sync with SQL server
    /// \brief load data
    bool Load(SqlConnector &conn);
    /// \brief save data
    bool Save(SqlConnector &conn);
    /// \brief return da file retrieved from server
    QString GetFileFromSqlServer(SqlConnector &conn, bool &ok);
    /// \brief check if data in memory are up to date
    bool IsFileUpToDate(SqlConnector &conn);
    /// \brief update data to server
    bool UpdateFileToSqlServer(SqlConnector &conn);
    /// \brief
    // File management
    /// \brief decrypt data
    QString Decrypt(const QString& cryptedDirFile);
    /// \brief encrypt data
    QString Encrypt(const QString& dirFile);
    /// \brief check if file is valid
    bool IsValidFile(const QDomDocument &doc);
    // File management
    /// \brief return users node
    QDomNode GetUsersNode();
    /// \brief return groups node
    QDomNode GetGroupsNode();
    /// \brief return app entries node
    QDomNode GetAppEntriesNode();
    /// \brief nullify dir file
    void Nullify();
    // Misc
    /// \brief return last error
    QString GetLastError();
    /// \brief return default file
    QString GetDefaultFile();
    /// \brief try to get an edit session
    bool GetEditSession(SqlConnector &conn);
    /// \brief release an edit session
    bool ReleaseEditSession(SqlConnector &conn);
    /// \brief return id of opened session (empty if no session opened)
    QString GetCurrentSessionIdFromServer(SqlConnector &conn, bool &ok);
private:
    Q_DISABLE_COPY(DirFile);
    /// \brief open edit session
    bool OpenEditSession(SqlConnector &conn);
    /// \brief check if dir file exits on server
    bool ExistsOnSqlServer(SqlConnector &conn);

    /// \brief return last update of dir file in the server
    QDateTime GetServerFileLastUpdate(SqlConnector &conn, bool& ok);

    QString         mLastError;     ///< Holds last error
    QDomDocument    mGalaxyDirDoc;  ///< Holds dom documnts
    QDateTime       mLastUpdate;    ///< Holds date time of data in memory
};
} // END DAPlugin
} // END GS

#endif // DIR_FILE_H
