#ifndef DIR_ACCESS_GALAXY_H
#define DIR_ACCESS_GALAXY_H

/*! \class DirAccessGalaxy
 * \brief
 *
 */

#define DA_GALAXY_VERSION 1.0
#define DA_GALAXY_NAME    "Plugin Directory Access Galaxy"

#include <QStringList>
#include <QObject>

#include "dir_access_base.h"

namespace GS
{
namespace DAPlugin
{

class AppEntries;
class Connector;
class Groups;
class Users;
class AdministrationDialog;

class DirAccessGalaxy : public DirAccessBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "dagalaxy")
    Q_INTERFACES(GS::DAPlugin::DirAccessBase)
public:
    /// \brief Constructor
    DirAccessGalaxy();
    /// \brief Desctructor
    virtual ~DirAccessGalaxy();
    /// \brief return pluggin version
    QString             Version();
    /// \brief return plugin name
    QString             Name();
    /// \brief return pointer to app entries
    AppEntriesBase*     GetAppEntries();
    /// \brief return pointer to connector
    ConnectorBase*      GetConnector();
    /// \brief return pointer to groups
    GroupsBase*         GetGroups();
    /// \brief return pointer to users
    UsersBase*          GetUsers();
    /// \brief open admin ui
    bool                OpenAdministrationUi();
    /// \brief save changes to DB
    bool                SaveChanges();
    /// \brief return connected user
    QString             GetCurrentUser();
    /// \brief return last error
    QString             GetLastError();
    /// \brief return last changes
    /// to stay compatible with previous version of user manager management
    /// has to be removed when not compatibler anynmore
    QStringList         YmAdbV2SupportGetLastUsersChanges();
    /// \brief clear last changes list
    /// to stay compatible with previous version of user manager management
    /// has to be removed when not compatibler anynmore
    void                YmAdbV2SupportClearLastUsersChanges();

private slots:
    /// \brief save change done
    bool OnSaveChangesRequested();

private:
    Q_DISABLE_COPY(DirAccessGalaxy)

    AdministrationDialog*   mAdminDialog;   ///< Holds admin dialog
    AppEntries*             mAppEntries;    ///< Holds app entries data
    Connector*              mConnector;     ///< Holds connector
    Users*                  mUsers;         ///< Holds users data
    Groups*                 mGroups;        ///< Holds groups data
    QString                 mLastError;     ///< Holds last error
    QStringList             mYmAdminDbV2SupportUsersChanges; ///< Holds last changes
                                                             ///has to be removed when no compatibility needed
};

} // END DAPlugin
} // END GS

#endif // DIR_ACCESS_GALAXY_H
