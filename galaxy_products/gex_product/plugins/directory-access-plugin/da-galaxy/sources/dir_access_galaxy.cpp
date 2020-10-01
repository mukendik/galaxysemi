
#include "administration_dialog.h"
#include "dir_access_galaxy.h"
#include "app_entries.h"
#include "connector.h"
#include "groups.h"
#include "users.h"

namespace GS
{
namespace DAPlugin
{
DirAccessGalaxy::DirAccessGalaxy()
{
    mConnector = new Connector();
    mUsers = new Users();
    mGroups = new Groups();
    mAppEntries = new AppEntries();
    mAdminDialog = 0;

    connect(mConnector, SIGNAL(sUsersLoaded(QDomNode)), mUsers, SLOT(Load(QDomNode)));
    connect(mConnector, SIGNAL(sGroupsLoaded(QDomNode)), mGroups, SLOT(Load(QDomNode)));
    connect(mConnector, SIGNAL(sAppEntriesLoaded(QDomNode)), mAppEntries, SLOT(Load(QDomNode)));
    connect(mConnector, SIGNAL(sConnectionStateChanged(bool)), this, SIGNAL(sConnectionStateChanged(bool)));
}

DirAccessGalaxy::~DirAccessGalaxy()
{
    if (mUsers)
    {
        delete mUsers;
        mUsers = 0;
    }

    if (mGroups)
    {
        delete mGroups;
        mGroups = 0;
    }

    if (mAppEntries)
    {
        delete mAppEntries;
        mAppEntries = 0;
    }
    if (mConnector)
    {
        delete mConnector;
        mConnector = 0;
    }
    if (mAdminDialog)
    {
        delete mAdminDialog;
        mAdminDialog = 0;
    }
}

QString DirAccessGalaxy::Version()
{
    return QString::number(DA_GALAXY_VERSION);
}

QString DirAccessGalaxy::Name()
{
    return QString(DA_GALAXY_NAME);
}

UsersBase *DirAccessGalaxy::GetUsers()
{
    if (!mConnector->IsConnected())
    {
        mLastError = "Unable to get Users object, connection not opened";
        return 0;
    }

    return mUsers;
}

bool DirAccessGalaxy::OpenAdministrationUi()
{
    if (!mConnector->IsConnected())
    {
        mLastError = "Unable to open admin dialog, connection not opened";
        return false;
    }

    if (!mAdminDialog)
        mAdminDialog = new AdministrationDialog(mConnector);

    mAdminDialog->SetData(mUsers, mGroups, mAppEntries);

    connect(mConnector, SIGNAL(sDataUpdated()),
                                                mAdminDialog, SLOT(OnInputUpdated()));
    connect(mConnector, SIGNAL(sConnectionStateChanged(bool)),
                                                mAdminDialog, SLOT(OnConnectionStateChanged(bool)));
    connect(mConnector, SIGNAL(sSessionStateChanged(bool)),
                                                mAdminDialog, SLOT(OnSessionStateChanged(bool)));
    connect(mConnector, SIGNAL(sOtherSessionStateChanged(QString)),
                                                mAdminDialog, SLOT(OnOtherSessionStateChanged(QString)));
    connect(mAdminDialog, SIGNAL(sConnectionRequested()),
                                                mConnector, SLOT(OnConnectionRequested()));
    connect(mAdminDialog, SIGNAL(sSaveRequested()),
                                                this, SLOT(OnSaveChangesRequested()));
    connect(mAdminDialog, SIGNAL(sCancelRequested()),
                                                mConnector, SLOT(OnCancelChangesRequested()));
    connect(mAdminDialog, SIGNAL(sChangeSessionStateRequested(bool)),
                                                mConnector, SLOT(OnChangeSessionStateRequested(bool)));

    mAdminDialog->SelectUser(mConnector->GetCurrentUser());
    mAdminDialog->exec();

    disconnect(mConnector, SIGNAL(sDataUpdated()),
                                                mAdminDialog, SLOT(OnInputUpdated()));
    disconnect(mConnector, SIGNAL(sConnectionStateChanged(bool)),
                                                mAdminDialog, SLOT(OnConnectionStateChanged(bool)));
    disconnect(mConnector, SIGNAL(sSessionStateChanged(bool)),
                                                mAdminDialog, SLOT(OnSessionStateChanged(bool)));
    disconnect(mConnector, SIGNAL(sOtherSessionStateChanged(QString)),
                                                mAdminDialog, SLOT(OnOtherSessionStateChanged(QString)));
    disconnect(mAdminDialog, SIGNAL(sConnectionRequested()),
                                                mConnector, SLOT(Connect()));
    disconnect(mAdminDialog, SIGNAL(sSaveRequested()),
                                                this, SLOT(OnSaveChangesRequested()));
    disconnect(mAdminDialog, SIGNAL(sCancelRequested()),
                                                mConnector, SLOT(OnCancelChangesRequested()));
    disconnect(mAdminDialog, SIGNAL(sChangeSessionStateRequested(bool)),
                                                mConnector, SLOT(OnChangeSessionStateRequested(bool)));

    return true;
}

bool DirAccessGalaxy::SaveChanges()
{
    return OnSaveChangesRequested();
}

QString DirAccessGalaxy::GetCurrentUser()
{
    if (!mConnector->IsConnected())
    {
        return QString();
    }

    return mConnector->GetCurrentUser();
}

QString DirAccessGalaxy::GetLastError()
{
    if(mLastError.isEmpty())
        mLastError = this->mConnector->GetLastError();
    if(mLastError.isEmpty())
        mLastError = this->mUsers->GetLastError();
    if(mLastError.isEmpty())
        mLastError = this->mGroups->GetLastError();
    if(mLastError.isEmpty())
        mLastError = this->mAppEntries->GetLastError();
    return mLastError;
}

QStringList DirAccessGalaxy::YmAdbV2SupportGetLastUsersChanges()
{
    return mYmAdminDbV2SupportUsersChanges;
}

void DirAccessGalaxy::YmAdbV2SupportClearLastUsersChanges()
{
    mYmAdminDbV2SupportUsersChanges.clear();
}

GroupsBase *DirAccessGalaxy::GetGroups()
{
    if (!mConnector->IsConnected())
    {
        mLastError = "Unable to get Groups object, connection not opened";
        return 0;
    }

    return mGroups;
}

AppEntriesBase *DirAccessGalaxy::GetAppEntries()
{
    if (!mConnector->IsConnected())
    {
        mLastError = "Unable to get AppEntries object, connection not opened";
        return 0;
    }

    return mAppEntries;
}

ConnectorBase *DirAccessGalaxy::GetConnector()
{
    return mConnector;
}

bool DirAccessGalaxy::OnSaveChangesRequested()
{
    if (!mConnector->IsConnected())
    {
        mLastError = "Unable to save, connection not opened";
        return false;
    }

    bool lOk = mConnector->OnApplyChangesRequested();

    if (lOk && mUsers)
    {
        mYmAdminDbV2SupportUsersChanges.append(mUsers->YmAdbV2SupportGetLastChanges());
        mUsers->YmAdbV2SupportClearLastChanges();
    }

    return lOk;
}

} // END DAPlugin
} // END GS

