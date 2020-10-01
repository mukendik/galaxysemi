///////////////////////////////////////////////////////////
// Database Admin class
///////////////////////////////////////////////////////////

#ifndef GEX_ADMIN_GUI_H
#define GEX_ADMIN_GUI_H
#include "ui_yieldmandb_login_dialog.h"
#include "ui_db_admin_dialog.h"
#include "admin_engine.h"
#include "gex_shared.h"
#include "db_new_database_dialog.h"

#define TAB_ENTERPRISE_INDEX    0

#define TAB_LOCAL_INDEX         1


#include <QStringList>
#include <QTreeWidget>
#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QStack>
#include <QDomElement>

// Galaxy modules includes
#include <gstdl_errormgr.h>
class GexDbPlugin_Connector;
class GexDatabaseEntry;
class CGexMoTaskItem;
class QTableWidget;

class AdminUserProfile;
class AdminUser;
class AdminUserGroup;
class AdminUserLogin : public QDialog, public Ui::yieldmandb_login_dialog
{
    Q_OBJECT

public:
    AdminUserLogin( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    void    ShowPageUserConnection();
    void    ShowPageUserOptions(QStringList lstUsersId=QStringList());
    void    ShowPageGroupProperties(QStringList lstGroupsId=QStringList());

    static void    EnabledFieldItem(QObjectList lstObject, bool bEnabled = true);
    static void    EnabledFieldTable(QTableWidget *pTable, bool bEnabled = true);
    static bool    SetCurrentComboItem(QComboBox *pCombo, QString strItem, bool bInsertIfMissing=false, QString strIcon=QString());


    bool  m_bForceConnection;
    bool  m_bAsAdmin;



private:
    QStringList m_lstUsersId;
    QStringList m_lstGroupsId;
    QStringList m_lstProfileId;

    void CheckAdminConnection();
public slots:

    void    onAccept(void);
    void    onReject(void);
    void    onEditLogin(void);
    void    onSelectLogin(void);
    void    onSelectType(void);
    void    onTableOptionContextualMenu(const QPoint& ptMousePoint);

};

class AdminGui : public QWidget, public Ui::db_admin_basedialog
{
    Q_OBJECT

public:
    AdminGui( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    void        ShowPage(void);
    void        refreshEnterpriseDatabaseList();

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void        dragEnterEvent(QDragEnterEvent *);
    void        dragMoveEvent(QDragMoveEvent *e);
    void        dropEvent(QDropEvent *);

    void        fillCellData(QTableWidget *ptTable, int iRow, int iCol, QString strText,
                             QString strTooltip="", bool bReadOnly=true, bool bAlarm=false);
    void        fillCellData(QTreeWidgetItem *pQtwiItem, int iCol, QString strText,
                             QString strTooltip="", bool bReadOnly=true, bool bGrey=false);

    bool        mDisplayWindowOnError;
    bool        m_bDatabaseEdited;				// True if a database is actualy edited then ignore the reload
    bool        m_bYieldManItemEdited;			// True if an item (user/group/profile) is actualy edited then ignore the reload

    void        CheckButtonsStatus(GexDatabaseEntry *pDatabaseEntry = NULL);


    QAction*        mButtonConnect;
    QLabel*         mLoginUser;
    AdminUserLogin* m_pLoginDialog;
    bool    m_bCheckAdminActivity;
    int     m_nNbMinBeforeDisconnect;


    // EnableGexAccess could be called with signal or other process
    bool        mUpdatePrivilegesInProgress;

private:

    GexDatabaseEntry* GetSelectedDatabase(bool bOnlyConnected = false);

    void    OnImportFiles(QString strDatabase,QStringList sFilesSelected);
    void    UpdateSkin(int lProductID);
    void    UpdateIndexFile(const QString & strDir, const QStringList & strlFilesRemoved);
    /// \brief try to import database link froma from a file
    void    ImportDatabase(QString filePath);
    void    DisplayStatusMessage(QString strText="");

    int     m_nFromServerBuild;

    QStringList  OpenDirAccessGui();

    // To check if UsersList need update
    qlonglong mUsersUpdateChecksum;
    qlonglong mUsersIdChecksum;
    bool    mWindowDetached;

    bool CheckDBConnection(GexDatabaseEntry *pDatabaseEntry);

public slots:
    // Do NOT move these functions to a non public slots part as it is necessary for JavaScript access

    bool    InfoDialog(QString TopMessage, QString HtmlMessage, QString BottomMessage, QString Buttons);
    void    WelcomeDialog(QString Options, bool &Result);
    void    SummaryDialog();
    QString HtmlSection(QString Type);
    void    ConnectionErrorDialog(QString &strRootCause, bool bCriticalError);

    //List registered databases (i.e. found in the databases folder) and fill the given combobox according options
    void    ListDatabasesInCombo(QComboBox *pcomboBox, int nDbOptions, int dbTDRType = DB_TDR_ALL, bool monitoringOnly = false);

    void    ShowDatabaseList(GexDatabaseEntry *pSelectDatabaseEntry, bool bDetach=false);
    void    ShowUserList();

    // Called if user hits the ESCAPE key.
    void    reject(void);
    void    OnCreateDatabase();
    void    OnCreateEnterpriseDatabase();
    void    OnImportFiles(void);
    void    OnDeleteDatabase();
    /// \brief open a dialog and allow to save db link
    void    OnExportDatabase();
    void    OnImportDatabase();
    void    OnDeleteEnterpriseDatabase();
    void    OnUploadDatabase(GexDatabaseEntry *pDatabaseEntry=NULL);
    void    OnHousekeeping(void);
    void    OnHousekeeping_SQL(const QString & strDatabaseName);
    void    contextMenuRequestedAdmin(const QPoint & pos = QPoint());
    void    contextMenuRequestedUser(const QPoint &pPoint);
    void    OnEditDatabase(void);
    void    OnDoubleClickDatabase(void);
    void    OnEditEnterpriseDatabase();
    void    OnConnectDatabase();
    void    OnDisconnectDatabase();
    void    OnSelectDatabase(GexDatabaseEntry *ptDatabaseEntry = NULL);
    void    OnReloadDatabaseList(void);

    void    OnManageUsersGroups(void);
    void    OnEditUser(bool ForAll=false);
    void    OnDeleteUser(void);
    void    OnResetPasswordUser(void);
    void    OnSelectUser(QTableWidgetItem* item=NULL);

    void    OnDetachWindow(bool bDetachWindow);
    void    OnSelectTab(int nIndex);
    void    OnSwitchToCharac();

    //
    void    EnableGexAccess();
    void    UpdateUIPrivileges(bool loginStateChanged);
    void    UpdateLoginButton(bool loginStateChanged);
    void    UpdateTasksTabs(bool loginStateChanged, bool userConnected, bool userConnectedAsAdmin);
    void    UpdateMonitoringTabs(bool userConnected, bool usersHasGroupsAdminPrivileges);
    void    UpdateDBConnection(bool loginStateChanged);
    void    ConnectUserDialog(bool bAsAdmin = false, bool bForceConnection = true);
    void    UserConnectionNeededToDialog(QString &strAction);
    void    OnCheckAdminActivity();

    // Connect and disconnect user
    void    OnActionConnect();
    void    ButtonConnectEnabled(bool Enable);
    void    ButtonConnectIsEnabled(bool& Enable);
    void    ButtonReloadListEnabled(bool Enable);
    void    ButtonReloadListIsEnabled(bool& Enable);

};

#endif
