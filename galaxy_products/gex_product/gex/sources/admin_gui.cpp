///////////////////////////////////////////////////////////
// Database admin: Create/Delete database, insert files
///////////////////////////////////////////////////////////
#include <QShortcut>
#include <QPixmap>
#include <QMenu>
#include <QInputDialog>
#include <QSqlError>
#include <qevent.h>

//#include "dbc_admin.h" // CHARACTERIZATION DB
#include "admin_engine.h"
#include "admin_gui.h"
#include "mo_admin.h"
#include "mo_task.h"
#include "gexmo_constants.h"
#include "db_onequery_wizard.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "db_new_database_dialog.h"
#include "db_external_database.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "gqtl_datakeys.h"
#include "db_sql_housekeeping_dialog.h"
#include "gex_constants.h"
#include "db_engine.h"
#include "product_info.h"
#include "engine.h"
#include "scheduler_engine.h"
#include "mo_scheduler_gui.h"
#include <gqtl_log.h>
#include "options_center_widget.h"
#include "command_line_options.h"
#include "message.h"
#include "dir_access_base.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// in main.cpp
extern GexMainwindow *      pGexMainWindow;
extern CGexSkin *           pGexSkin;      // holds the skin settings
// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// in classes.cpp
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

///////////////////////////////////////////////////////////
// Database Administration page
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_AdminGui(void)
{
    // Check if data access type if FILES or DATABASE
    if(!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
    {
        //    // DATABASE type
        //    case GEX_DATATYPE_GEX_MONITORING:     // ExaminatorMonitoring
        //    case GEX_DATATYPE_GEX_PATMAN:         // Galaxy PAT-Man
        //    case GEX_DATATYPE_ALLOWED_DATABASE:   // ExaminatorDB: Database access, any file type.
        //    case GEX_DATATYPE_ALLOWED_DATABASEWEB:// ExaminatorWeb
        //    case GEX_DATATYPE_GEX_YIELD123:       // Yield123
        //        break;
        //    default:
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        QMessageBox::warning(this,"", m);
        return;
    }

    // Show wizard page : Database Admin
    ShowWizardDialog(GEX_DATABASE_ADMIN);

    // Make sure ExaminatorDB has the latest list of databases
    ReloadDatabasesList(true);
}

///////////////////////////////////////////////////////////
// Refresh Database list & GUI
///////////////////////////////////////////////////////////
int GexMainwindow::ReloadDatabasesList(bool bRepaint)
{
    int iTotalEntries = 0;
    //	Check that the DataBase center has been created. When it is NULL, it means that database is not allowed
    // Make sure ExaminatorDB has the latest list of databases
    iTotalEntries = GS::Gex::Engine::GetInstance().GetDatabaseEngine().LoadDatabasesListIfEmpty();

    if(pWizardAdminGui == NULL)
        return iTotalEntries;

    // If no need to repaint GUI, exit.
    // Only if not display
    // Admin Server: don't repain if widget not visible
    if((bRepaint == false) || (!pWizardAdminGui->isVisible()))
        return iTotalEntries;	// Return total number of entries available.

    pWizardAdminGui->ShowDatabaseList(NULL);


#if defined unix || __MACH__
    // FIX QT BUG: crash when opening new connection after having removal all existing connection
    // using QSqlDatabase::removeDatabase(...)
    // With this fix we always keep at least one connection
    if (!QSqlDatabase::contains("ghost_conn") && QSqlDatabase::isDriverAvailable("QMYSQL"))
        QSqlDatabase::addDatabase("QMYSQL", "ghost_conn");
#endif

    return iTotalEntries;
}

///////////////////////////////////////////////////////////
// Fill a combo with the list of Databases entries
///////////////////////////////////////////////////////////
void	AdminGui::ListDatabasesInCombo(QComboBox *pcomboBox, int nDbOptions, int dbTDRType, bool monitoringOnly)
{
    if (!pcomboBox)
        return;

    // Empty the combo
    pcomboBox->clear();

    // Fill combo with list of Databases...
    QString	strDatabase;
    QString lFirstCompatible;

    // If list is empty...check again over the network
    if(GS::Gex::Engine::GetInstance().GetDatabaseEngine().LoadDatabasesListIfEmpty() == 0) return;

    QList<GexDatabaseEntry*>::iterator itBegin	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.begin();
    QList<GexDatabaseEntry*>::iterator itEnd	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.end();

    bool bAddEntry;
    QString strIcon;
    while(itBegin != itEnd)
    {
        bAddEntry = false;
        // Select the type of database needed
        if((nDbOptions & DB_TYPE_FILEBASED) && ((*itBegin)->IsFileBased()))
            bAddEntry = true;
        else if((nDbOptions & DB_TYPE_BLACKHOLE) && ((*itBegin)->IsBlackHole()))
            bAddEntry = true;
        else if((nDbOptions & DB_TYPE_SQL) && ((*itBegin)->IsExternal()))
            bAddEntry = true;

        if(bAddEntry)
        {
            if ((*itBegin)->IsAdr())
                bAddEntry = false;
            // remove some databases
            if((nDbOptions & DB_STATUS_CONNECTED) && !((*itBegin)->StatusFlags() & STATUS_CONNECTED))
                bAddEntry = false;
            if((nDbOptions & DB_STATUS_UPLOADED) && !((*itBegin)->IsStoredInDb()))
                bAddEntry = false;
            if((nDbOptions & DB_STATUS_UPTODATE) && !((*itBegin)->IsCompatible()))
                bAddEntry = false;
            if((nDbOptions & DB_SUPPORT_INSERTION) && !((*itBegin)->StatusFlags() & STATUS_INSERTION))
                bAddEntry = false;
            if((nDbOptions & DB_SUPPORT_UPDATE) && !((*itBegin)->StatusFlags() & STATUS_HOUSEKEEPING))
                bAddEntry = false;
            if((nDbOptions & DB_SUPPORT_ER) && ((*itBegin)->IsExternal()) && !((*itBegin)->m_pExternalDatabase->IsEnterpriseReportsSupported()))
                bAddEntry = false;
            if((nDbOptions & DB_SUPPORT_RC) && ((*itBegin)->IsExternal()) && !((*itBegin)->m_pExternalDatabase->IsReportsCenterSupported()))
                bAddEntry = false;
            if((nDbOptions & DB_STATUS_ADR_LINK) && !(*itBegin)->HasAdrLink())
                bAddEntry = false;
        }


        // TDR filter activated
        if (bAddEntry && dbTDRType && ((*itBegin)->IsExternal()))
        {
            bool macthfilter = false;

            if ((dbTDRType & DB_TDR_MANUAL_PROD) && (*itBegin)->IsManualProdTdr())
                macthfilter = true;
            else if ((dbTDRType & DB_TDR_MANUAL_CHARAC) && (*itBegin)->IsCharacTdr())
                macthfilter = true;
            else if ((dbTDRType & DB_TDR_YM_PROD) && (*itBegin)->IsYmProdTdr())
                macthfilter = true;
            //else if ((dbTDRType & DB_ADR) && (*itBegin)->IsAdr())
             //   macthfilter = true;

            if (!macthfilter)
                bAddEntry = false;
        }

        // if monitoring only, the database has to be compatible for insertion
        if(bAddEntry && monitoringOnly)
        {
            GexDatabaseEntry *pDatabaseEntry = *itBegin;
            bAddEntry = pDatabaseEntry->IsCompatible();
        }

        // Should we add entry to combo?
        if(bAddEntry)
        {
            // Add entry to combo.
            if((*itBegin)->IsStoredInDb())
                strDatabase = "";
            else if((*itBegin)->IsStoredInFolderLocal() == true)
                strDatabase = "[Local] ";
            else
                strDatabase = "[Server] ";

            strDatabase += (*itBegin)->LogicalName();
            strIcon = (*itBegin)->IconName();

            if(lFirstCompatible.isEmpty())
            {
                if((*itBegin)->IsCompatible())
                    lFirstCompatible = strDatabase;
            }

            pcomboBox->insertItem(pcomboBox->count(),QPixmap(QString::fromUtf8(strIcon.toLatin1().constData())),strDatabase);
        }

        // Move to next enry.
        itBegin++;
    };
    if(!lFirstCompatible.isEmpty())
    {
        pcomboBox->setCurrentText(lFirstCompatible);
    }
}

///////////////////////////////////////////////////////////
// Manage the Database Admin page
///////////////////////////////////////////////////////////
AdminGui::AdminGui( QWidget* parent, bool /*modal*/, Qt::WFlags fl) : QWidget(parent, fl)
{
    mUpdatePrivilegesInProgress = false;
    mDisplayWindowOnError = true;
    m_pLoginDialog = NULL;
    mButtonConnect = NULL;
    m_bCheckAdminActivity = false;
    m_nNbMinBeforeDisconnect = -1;
    m_nFromServerBuild = 0;
    mWindowDetached = false;

    mUsersUpdateChecksum = mUsersIdChecksum = 0;

    setupUi(this);
    setObjectName("GSAdminGui");

    pushButtonCharac->setVisible(false);

    // Show Gex Database list
    stackedWidgetDbList->setCurrentIndex(TAB_ENTERPRISE_INDEX);

    setWindowFlags(Qt::FramelessWindowHint);
    move(0,0);

    buttonNewDatabase->setToolTip("Create a new Database link");

    buttonEditDatabase->setToolTip("Edit Database settings");
    buttonRemoveDatabase->setToolTip("Delete a Database link");

    m_bDatabaseEdited = false;
    m_bYieldManItemEdited = false;
    buttonRemoveUser->hide();

    // remove all tab
    while(tabWidgetAdminGui->count() > 0)
        tabWidgetAdminGui->removeTab(tabWidgetAdminGui->count()-1);

    // Add Database Admin with icon
    tabWidgetAdminGui->addTab(tabDatabases,QIcon(QPixmap(QString::fromUtf8(":/gex/icons/database-display.png"))),"Databases");

    tableWidgetUser->setContextMenuPolicy(Qt::CustomContextMenu);

    QObject::connect(&GS::Gex::Engine::GetInstance().GetDatabaseEngine(),
                     SIGNAL(sOnSelectDatabase(GexDatabaseEntry *)), this, SLOT(OnSelectDatabase(GexDatabaseEntry *)));
    QObject::connect(&GS::Gex::Engine::GetInstance().GetDatabaseEngine(),
                     SIGNAL(sShowDatabaseList(GexDatabaseEntry *, bool)), this, SLOT(ShowDatabaseList(GexDatabaseEntry *, bool)));
    QObject::connect(&GS::Gex::Engine::GetInstance().GetDatabaseEngine(),
                     SIGNAL(sButtonConnectEnabled(bool)), this, SLOT(ButtonConnectEnabled(bool)));
    QObject::connect(&GS::Gex::Engine::GetInstance().GetDatabaseEngine(),
                     SIGNAL(sButtonConnectIsEnabled(bool&)), this, SLOT(ButtonConnectIsEnabled(bool&)));
    QObject::connect(&GS::Gex::Engine::GetInstance().GetDatabaseEngine(),
                     SIGNAL(sButtonReloadListEnabled(bool)), this, SLOT(ButtonReloadListEnabled(bool)));
    QObject::connect(&GS::Gex::Engine::GetInstance().GetDatabaseEngine(),
                     SIGNAL(sButtonReloadListIsEnabled(bool&)), this, SLOT(ButtonReloadListIsEnabled(bool&)));

    QObject::connect(&GS::Gex::Engine::GetInstance().GetAdminEngine(),
                     SIGNAL(sShowUserList()), this, SLOT(ShowUserList()));
    QObject::connect(&GS::Gex::Engine::GetInstance().GetAdminEngine(),
                     SIGNAL(sEnableGexAccess()),  this, SLOT(EnableGexAccess()));
    QObject::connect(&GS::Gex::Engine::GetInstance().GetAdminEngine(),
                     SIGNAL(sWelcomeDialog(QString,bool&)), this, SLOT(WelcomeDialog(QString,bool&)));
    QObject::connect(&GS::Gex::Engine::GetInstance().GetAdminEngine(),
                     SIGNAL(sSummaryDialog()), this, SLOT(SummaryDialog()));
    QObject::connect(&GS::Gex::Engine::GetInstance().GetAdminEngine(),
                     SIGNAL(sConnectionErrorDialog(QString &, bool)), this, SLOT(ConnectionErrorDialog(QString &, bool)));

    QObject::connect(&GS::Gex::Engine::GetInstance().GetSchedulerEngine(),
                     SIGNAL(sEnableGexAccess()),  this, SLOT(EnableGexAccess()));

    QObject::connect(tabWidgetAdminGui,SIGNAL(currentChanged (int)), this, SLOT(OnSelectTab(int)));

    QObject::connect(buttonNewDatabase,     SIGNAL(clicked()), this, SLOT(OnCreateDatabase()));
    QObject::connect(buttonAddFiles,        SIGNAL(clicked()), this, SLOT(OnImportFiles()));
    QObject::connect(buttonRemoveDatabase,  SIGNAL(clicked()), this, SLOT(OnDeleteDatabase()));
    QObject::connect(buttonHousekeeping,    SIGNAL(clicked()), this, SLOT(OnHousekeeping()));
    QObject::connect(buttonEditDatabase,    SIGNAL(clicked()), this, SLOT(OnEditDatabase()));
    QObject::connect(buttonReloadList,      SIGNAL(clicked()), this, SLOT(OnReloadDatabaseList()));
    QObject::connect(treeWidgetDatabase,    SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequestedAdmin(QPoint)));
    QObject::connect(treeWidgetDatabase,    SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(OnDoubleClickDatabase()));
    QObject::connect(treeWidgetDatabase,    SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectDatabase()));
    QObject::connect(pushButtonCharac,      SIGNAL(clicked()), this, SLOT(OnSwitchToCharac()));
    QObject::connect(buttonNewUser,         SIGNAL(clicked()), this, SLOT(OnManageUsersGroups()));
    QObject::connect(buttonEditUser,        SIGNAL(clicked()), this, SLOT(OnEditUser()));
    QObject::connect(buttonRemoveUser,      SIGNAL(clicked()), this, SLOT(OnDeleteUser()));
    QObject::connect(tableWidgetUser,       SIGNAL(cellDoubleClicked(int,int)), this, SLOT(OnEditUser()));
    QObject::connect(tableWidgetUser,       SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequestedUser(const QPoint &)));

    buttonNewUser->setToolTip("Create a new User");
    buttonEditUser->setToolTip("Edit User options");

    buttonReloadList->setEnabled(false);
    buttonRemoveDatabase->setEnabled(false);
    buttonAddFiles->setEnabled(false);
    buttonHousekeeping->setEnabled(false);
    checkBoxEditHeader->setEnabled(false);

    buttonNewUser->setEnabled(true);
    buttonRemoveUser->setEnabled(false);
    buttonRemoveUser->hide();

    // At startup, list box is empty...so can only create entries
    CheckButtonsStatus();


    // Support for Drag&Drop
    setAcceptDrops(true);

    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    checkBoxEditHeader->hide();

    // Status message
    TextLabelStatus->hide();
    TextLabelTip->show();

}

void AdminGui::ImportDatabase(QString filePath)
{
    // Load selected settings file
    GexDatabaseEntry lDatabaseEntry(this);
    if ((lDatabaseEntry.LoadFromXmlFile(filePath)) &&
            lDatabaseEntry.CopyToDatabasesFolder(filePath,
                                                 ReportOptions.GetLocalDatabaseFolder()))
    {
        OnReloadDatabaseList();
        GS::Gex::Message::information("", "New database link recorded!");
    }
    else
        QMessageBox::warning(this,"", "Unable to record new database link");
}

///////////////////////////////////////////////////////////
// GexAdminGui: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void AdminGui::reject(void)
{
}

///////////////////////////////////////////////////////////
// signal select tab
///////////////////////////////////////////////////////////
void AdminGui::OnSelectTab(int nIndex)
{
    if(mWindowDetached)
        return;

    QCoreApplication::processEvents();

    QWidget * pCurrentTab = tabWidgetAdminGui->currentWidget();

    // If no item in list, just return!
    if(pCurrentTab == NULL)
        return;

    // Reload users list if updated
    if(pCurrentTab == tabUsers)
    {
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().LoadUsersList();
            ShowUserList();
        }
    }

    // Have to enable/disable New/Edit/... button if Examinator-PRO
    if(pGexMainWindow->pWizardSchedulerGui)
        pGexMainWindow->pWizardSchedulerGui->OnSelectTab(0);

    if(pGexMainWindow->mMonitoringWidget
            && (tabWidgetAdminGui->indexOf(pGexMainWindow->mMonitoringWidget) == nIndex))
        TextLabelTip->hide();
    else
        TextLabelTip->show();

}

///////////////////////////////////////////////////////////
// Give the list of databases selected
///////////////////////////////////////////////////////////
GexDatabaseEntry* AdminGui::GetSelectedDatabase(bool bOnlyConnected)
{

    // Check if DB list has some elements
    if(treeWidgetDatabase->topLevelItemCount() == 0)
        return NULL;

    // Check if only one was selected
    if(treeWidgetDatabase->currentItem() == NULL)
        return NULL;
    //if(treeWidgetDatabase->selectedItems().isEmpty())
    //    return NULL;
     if(treeWidgetDatabase->selectedItems().count() > 1)
        return NULL;

    QTreeWidgetItem		*pQtwiItem;
    GexDatabaseEntry	*ptDatabaseEntry = NULL;
    // Get selected item
    pQtwiItem = treeWidgetDatabase->currentItem();
    if((pQtwiItem == NULL)
            && (!treeWidgetDatabase->selectedItems().isEmpty()))
    pQtwiItem = (treeWidgetDatabase->selectedItems()).first();

    if(pQtwiItem == NULL) return NULL;

    // Extract database name
    QString strDatabaseName = pQtwiItem->text(0);

    // Find database ptr
    ptDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName,false);
    if(ptDatabaseEntry == NULL) return NULL;

    if(bOnlyConnected)
    {
        if(ptDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
            return ptDatabaseEntry;
    }
    else
        return ptDatabaseEntry;

    return NULL;
}



///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void AdminGui::ShowPage(void)
{
    OnDetachWindow(false);

    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        checkBoxEditHeader->show();
    else
        checkBoxEditHeader->hide();

    // Remove tabLogs
    tabWidgetAdminGui->removeTab(tabWidgetAdminGui->indexOf(tabLogFile));

    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        // Remove tabUsers
        tabWidgetAdminGui->removeTab(tabWidgetAdminGui->indexOf(tabUsers));

    // Always hide Characterization module
    pushButtonCharac->setVisible(false);

    // Make Widget visible.
    show();
}

///////////////////////////////////////////////////////////
// Refresh User list & GUI
///////////////////////////////////////////////////////////
// for optimization
// store the line where the user is stored in the view
QMap<int,int> gUsersRowInView;

void AdminGui::ShowUserList()
{
    bool lConnected = GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected();
    // No Users GUI when not connected
    if(!lConnected) return;

    bool lNeedToRefresh = false;
    bool lUserGroupReaderConnected = GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges(true);
    bool lUserGroupAdminConnected = GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges();
    int  lNbUsersInGui = tableWidgetUser->rowCount();

    if(lNbUsersInGui == 0)
        lNeedToRefresh = true;

    // Check if the mode changed
    // not Reader && not Admin => 1 row (current user)
    // Reader => N rows;
    if((lNbUsersInGui > 1)
            && !lUserGroupReaderConnected)
    {
        // Reset the list
        tableWidgetUser->setRowCount(0);
        lNbUsersInGui = 0;
        lNeedToRefresh = true;
    }

    if((mUsersUpdateChecksum != GS::Gex::Engine::GetInstance().GetAdminEngine().m_LastDbUsersUpdateChecksum)
            || (mUsersIdChecksum != GS::Gex::Engine::GetInstance().GetAdminEngine().m_LastDbUsersIdChecksum))
        lNeedToRefresh = true;

    if(!lNeedToRefresh) return;

    // GUI update
    int     lIndex = 0;
    int     lUserId = 0;
    int     lNbUsers = 0;
    AdminUser*  pUser=0;
    // Load all users
    bool    lResizeColumn = false;
    bool    lSeachGoodLine = true;
    QString lMsg = "Updating view... %1 users";
    QTime   timer;
    timer.start();
    DisplayStatusMessage();

    // Update the Users List GUI
    // Check if all columns are created
    if(tableWidgetUser->columnCount() < 9)
    {
        // Have to create all needed columns
        tableWidgetUser->setColumnCount(9);
        // Reset rows
        tableWidgetUser->setRowCount(0);
        tableWidgetUser->setColumnHidden(0,true);
        tableWidgetUser->setColumnHidden(1,true);

        QStringList lstLabels;
        lstLabels << "UserId" << "GroupId" << "Name" << "Login" << "Emails" << "Type" << "Creation Date" << "Last Update" << "Last Connection";
        tableWidgetUser->setHorizontalHeaderLabels(lstLabels);
        tableWidgetUser->verticalHeader()->setVisible(false);
        tableWidgetUser->setSelectionBehavior(QAbstractItemView::SelectRows);

        // Set minimum size for the columns
        tableWidgetUser->setColumnWidth (lIndex++,50);
        tableWidgetUser->setColumnWidth (lIndex++,50);
        tableWidgetUser->setColumnWidth (lIndex++,150);   // Name
        tableWidgetUser->setColumnWidth (lIndex++,80);    // Login
        tableWidgetUser->setColumnWidth (lIndex++,200);   // Emails
        tableWidgetUser->setColumnHidden(lIndex,true);    // Hide Type info
        tableWidgetUser->setColumnWidth (lIndex++,200);   // Type
        tableWidgetUser->setColumnWidth (lIndex++,200);   // Creation
        tableWidgetUser->setColumnWidth (lIndex++,200);   // Update
        tableWidgetUser->setColumnWidth (lIndex++,200);   // Connection

        lResizeColumn = true;
    }
    // Resetted after each disconnection
    if(tableWidgetUser->rowCount() == 0)
    {
        gUsersRowInView.clear();
        lResizeColumn = true;
        lSeachGoodLine = false;
    }

    tableWidgetUser->setEnabled(false);

    foreach(pUser, GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers)
    {
        if(!lUserGroupReaderConnected
                && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser
                && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nUserId != pUser->m_nUserId)
            continue;

        ++lNbUsers;
        lUserId = pUser->m_nUserId;

        // Find the good row
        // check if the task already referenced
        int nCurrentRow = -1;
        if(lSeachGoodLine
                && gUsersRowInView.contains(lUserId))
        {
            nCurrentRow = gUsersRowInView[lUserId];
            QTableWidgetItem *ptItem = tableWidgetUser->item(nCurrentRow,0);
            if(ptItem && (ptItem->text().toInt() == lUserId))
            {
                // found
            }
            else
            {
                nCurrentRow = -1;
            }
        }

        // Insert item into ListView
        if(nCurrentRow < 0)
        {
            nCurrentRow = tableWidgetUser->rowCount();
            tableWidgetUser->setRowCount(1+nCurrentRow);
            tableWidgetUser->setRowHeight(nCurrentRow, 20);
        }

        gUsersRowInView[lUserId] = nCurrentRow;

        // Add entry: title, task type, Frequency, Alarm condition, email addresses
        lIndex = 0;
        fillCellData(tableWidgetUser,nCurrentRow,lIndex++,QString::number(pUser->m_nUserId));
        fillCellData(tableWidgetUser,nCurrentRow,lIndex++,QString::number(pUser->m_nGroupId));
        fillCellData(tableWidgetUser,nCurrentRow,lIndex++,pUser->m_strName);
        fillCellData(tableWidgetUser,nCurrentRow,lIndex++,pUser->m_strLogin);
        fillCellData(tableWidgetUser,nCurrentRow,lIndex++,pUser->m_strEmail);
        fillCellData(tableWidgetUser,nCurrentRow,lIndex++,GS::Gex::Engine::GetInstance().GetAdminEngine().UserTypeToString(pUser->m_nType));
        fillCellData(tableWidgetUser,nCurrentRow,lIndex++,pUser->m_clCreationDate.toString());
        fillCellData(tableWidgetUser,nCurrentRow,lIndex++,pUser->m_clUpdateDate.toString());
        fillCellData(tableWidgetUser,nCurrentRow,lIndex++,pUser->m_clAccessDate.toString());


        if(!lUserGroupAdminConnected
                && (GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser
                    && (GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nUserId != pUser->m_nUserId)))
        {
            // Set font color
            QColor rgbColor = QColor(180,180,180);
            for(int ii=0; ii<tableWidgetUser->columnCount(); ii++)
                tableWidgetUser->item(nCurrentRow,ii)->setTextColor(rgbColor);
        }
        else
            tableWidgetUser->setRowHidden(nCurrentRow,false);

        if(timer.elapsed() > 200)
        {
            if(lResizeColumn)
            {
                lResizeColumn = false;
                // Adjust the size of the column
                for(int iCol=0; iCol<tableWidgetUser->columnCount(); iCol++)
                {
                    tableWidgetUser->resizeColumnToContents(iCol);
                    if(tableWidgetUser->columnWidth(iCol) > 350)
                        tableWidgetUser->setColumnWidth(iCol,350);
                }
                if(lNbUsers < (GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.count()/2))
                    DisplayStatusMessage(lMsg.arg(lNbUsers));
            }
            else
                DisplayStatusMessage(lMsg.arg(lNbUsers));
            timer.start();
        }

        if(nCurrentRow == 10)
            QCoreApplication::processEvents();
    }

    if(lResizeColumn)
    {
        // Adjust the size of the column
        for(int iCol=0; iCol<tableWidgetUser->columnCount(); iCol++)
        {
            tableWidgetUser->resizeColumnToContents(iCol);
            if(tableWidgetUser->columnWidth(iCol) > 350)
                tableWidgetUser->setColumnWidth(iCol,350);
        }
    }

    mUsersUpdateChecksum = GS::Gex::Engine::GetInstance().GetAdminEngine().m_LastDbUsersUpdateChecksum;
    mUsersIdChecksum = GS::Gex::Engine::GetInstance().GetAdminEngine().m_LastDbUsersIdChecksum;

    tableWidgetUser->setEnabled(true);
    DisplayStatusMessage();
}
///////////////////////////////////////////////////////////
// Refresh Database list & GUI
// If NULL => Refresh the databases list
///////////////////////////////////////////////////////////
void AdminGui::ShowDatabaseList(GexDatabaseEntry *pSelectDatabaseEntry, bool bDetach)
{
    if(bDetach)
        OnDetachWindow(true);

    // Rebuild new list with latest entries refreshed...
    QTreeWidgetItem	*   pQtwiItem;
    QString     strCompressed            = "Zip files";
    QString     strCopy                  = "Copy all files";
    QString     strLink                  = "Links only";
    QString     strSummary               = "Summary";
    QString     strBlackHole             = "Black Hole";
    QString     strCorporateReadOnly     = "Corporate (Read only)";
    QString     strCorporateReadWrite    = "Corporate";
    QString     strDatabaseStorage;
    QString     strSize;
    QString     strConnected, strType, strDbVersion, strPluginVersion, strSupportedVersion, strUpToDate, strLocation, strTdrType;
    QString     strPluginName, strDbInfo, strStartupType, strToolTip;

    GexDatabaseEntry* pDatabaseEntry;
    GexDatabaseEntry* pDatabaseEntrySelected = pSelectDatabaseEntry;
    bool        ResizeColumn = false;

    if(treeWidgetDatabase->columnCount() < 6)
    {
        QTreeWidgetItem* pQtwiHeaderItem = treeWidgetDatabase->headerItem();
        pQtwiHeaderItem->setText(0, QString("Database Name"));
        pQtwiHeaderItem->setText(1, QString(" "));
        pQtwiHeaderItem->setText(2, QString("Size"));
        pQtwiHeaderItem->setText(3, QString("Data Storage"));
        pQtwiHeaderItem->setText(4, QString("Type"));
        pQtwiHeaderItem->setText(5, QString(" "));
        pQtwiHeaderItem->setText(6, QString("Version"));
        pQtwiHeaderItem->setText(7, QString(" "));
        pQtwiHeaderItem->setText(8, QString("Info"));
        pQtwiHeaderItem->setText(9, QString("Startup Type"));
        pQtwiHeaderItem->setText(10, QString("Plugin Name"));
        pQtwiHeaderItem->setText(11, QString("Description"));

        pQtwiHeaderItem->setTextAlignment(1,Qt::AlignHCenter);
        pQtwiHeaderItem->setTextAlignment(6,Qt::AlignHCenter);
        pQtwiHeaderItem->setTextAlignment(9,Qt::AlignHCenter);

        ResizeColumn = true;
    }

    // Empty list box on the Admin page!
    if(treeWidgetDatabase->topLevelItemCount() != GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.count())
    {
        treeWidgetDatabase->clear();
        pDatabaseEntrySelected = NULL;
    }

    bool NewItem = false;
    int nCurrentRow = -1;
    QList<GexDatabaseEntry*>::iterator itBegin;
    QList<GexDatabaseEntry*>::iterator itEnd    = GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.end();
    for(itBegin=GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.begin(); itBegin != itEnd; itBegin++)
    {
        pDatabaseEntry = (*itBegin);

        // Ignore duplicated Database
        // Present in the list for controle update
        if(pDatabaseEntry->LogicalName() != pDatabaseEntry->m_strDatabaseRef)
            continue;

        nCurrentRow++;
        if(pDatabaseEntrySelected && (pDatabaseEntrySelected->m_nDatabaseId != pDatabaseEntry->m_nDatabaseId))
            continue;

        NewItem = false;
        pQtwiItem = NULL;
        if(treeWidgetDatabase->topLevelItemCount() > nCurrentRow)
            pQtwiItem = treeWidgetDatabase->topLevelItem(nCurrentRow);

        if(pQtwiItem == NULL)
        {
            pQtwiItem = new QTreeWidgetItem(treeWidgetDatabase);
            pQtwiItem->setTextAlignment(1,Qt::AlignHCenter);
            pQtwiItem->setTextAlignment(2,Qt::AlignRight);
            pQtwiItem->setTextAlignment(5,Qt::AlignHCenter);
            pQtwiItem->setTextAlignment(6,Qt::AlignHCenter);
            pQtwiItem->setTextAlignment(7,Qt::AlignHCenter);
            pQtwiItem->setTextAlignment(9,Qt::AlignHCenter);
            NewItem = true;
        }


        if(pDatabaseEntry->IsSummaryOnly())
            strDatabaseStorage = strSummary;
        else if(pDatabaseEntry->IsBlackHole())
            strDatabaseStorage = strBlackHole;
        else if(pDatabaseEntry->HoldsFileCopy())
        {
            if(pDatabaseEntry->IsCompressed())
                strDatabaseStorage = strCompressed;
            else
                strDatabaseStorage = strCopy;
        }
        else
            strDatabaseStorage = strLink;

        // Corporate database
        if(pDatabaseEntry->IsExternal())
        {
            if(pDatabaseEntry->IsReadOnly())
                strDatabaseStorage = strCorporateReadOnly;	// Read-only database
            else
                strDatabaseStorage = strCorporateReadWrite;	// Read+Write database
        }

        // Check if can connect the database - force disconnection
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsInRestrictedMode(pDatabaseEntry))
        {
            strDatabaseStorage = "Restricted Mode";
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToModify(pDatabaseEntry))
                strDatabaseStorage += "";
            else if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToRead(pDatabaseEntry))
                strDatabaseStorage += " (Read only)";
            else
                strDatabaseStorage += " (No Access)";
        }

        strConnected = strDbVersion = strPluginVersion = strSupportedVersion = strUpToDate = strLocation = "";
        strPluginName = strDbInfo = strStartupType = strToolTip = strTdrType = "";
        strLocation = pDatabaseEntry->IconName();

        if(pDatabaseEntry->IsExternal())
        {
            strConnected = "disable.png";
            strUpToDate = "disable-flag.png";
            strSize = "";
            strType = "Database";
            strDbInfo = "";
            strDbVersion = "- not loaded -";
            strTdrType = "- not loaded -";

            // database referenced in YieldManDb
            if(pDatabaseEntry->IsStoredInDb())
                strType = "Shared Database";
        }
        else if(pDatabaseEntry->IsBlackHole())
        {
            strConnected = "";
            strUpToDate = "";
            strType = strDbVersion = "* BLACK-HOLE *";
            strDbInfo = "";
            strSize = "";
        }
        else
        {
            strConnected = "";
            strUpToDate = "";
            strType = strDbVersion = "* FILE-BASED *";
            strDbInfo = "";
            strSize = GS::Gex::Engine::GetInstance().GetDatabaseEngine().BuildDatabaseSize(
                        pDatabaseEntry->CacheSize(),
                        pDatabaseEntry->IsExternal());
        }

        strToolTip = "Database:           \t"+pDatabaseEntry->LogicalName();
        strToolTip+= "\nStorage:        \t\t"+QString(strDatabaseStorage);


        if(pDatabaseEntry->m_pExternalDatabase)
        {
            // Get TDR type
            strTdrType = pDatabaseEntry->GetTdrTypeName();

            if(strTdrType.isEmpty() || strTdrType == "Not recognized")
                strTdrType = pDatabaseEntry->TdrTypeRecorded();

            if(strTdrType.isEmpty())
                strTdrType = "Not recognized";

            if (strTdrType == "Not recognized")
                GSLOG(SYSLOG_SEV_ERROR, QString("Database type of %1 not recognized!").arg(pDatabaseEntry->LogicalName()).toLatin1().constData());

            // Get the plugin version name
            if(!pDatabaseEntry->DatabaseVersion().isEmpty())
                strDbVersion = pDatabaseEntry->DatabaseVersion();

            if(strDbVersion != "- not loaded -")
                strPluginVersion = "V" + strDbVersion.section("(",0,0).section("V",1);
            strSupportedVersion = pDatabaseEntry->m_pExternalDatabase->GetPluginName().section("[Plugin ",1).section("]",0,0);

            if(strDbVersion == "<unknown>")
                strDbVersion = strTdrType;

            // For startup info
            if(pDatabaseEntry->m_pExternalDatabase->IsAutomaticStartup())
                strStartupType = "Automatic";
            else
                strStartupType = "Manual";
            strToolTip+= "\nStartup Type:       \t"+strStartupType;

            if(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
                strToolTip+= "\nConnection Status:   \tConnected";
            else
                strToolTip+= "\nConnection Status:   \tDisconnected";

            // For compression protocol
            if(pDatabaseEntry->StatusFlags() & STATUS_COMPRESS_PROTOCOL)
            {
                strDbInfo = "Network Protocol Compression";
                strToolTip+= "\nConnection Options: \tNetwork Protocol Compression";
            }

            strToolTip+= "\n\nPlugin Version:     \t" + strPluginVersion;
            strToolTip+= "\nSupported Version:    \t" + strSupportedVersion;

            // Check if the database is up to date
            // Can be not uptodate but compatible (not same build version for YM, not same minor version for examinator)
            if(pDatabaseEntry->IsUpToDate())
            {
                strUpToDate = "green-flag.png";
                strToolTip+= "\nPlugin Status:      \tUp to date";

                // FLEXIBLE CONSOLIDATION
                if(!(pDatabaseEntry->IsCompatible()))
                {
                    strDbInfo = pDatabaseEntry->m_strLastInfoMsg.section(":",1);
                    strUpToDate = "red-flag.png";
                    strToolTip+= "\nPlugin Status:      \t"+strDbInfo;
                }
            }
            else if(pDatabaseEntry->IsCompatible())
            {
                strUpToDate = "blue-flag.png";
                strDbInfo = "Compatible - New build available";

                // Check if it is a new version
                if(strSupportedVersion.section("B",1).simplified().toInt() < strPluginVersion.section("B",1).simplified().toInt())
                    strDbInfo = "Compatible version";


                strToolTip+= "\nPlugin Status:      \t"+strDbInfo;
            }
            else if(!(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED))
            {
                strUpToDate = "disable-flag.png";
            }
            else
            {
                strDbInfo = "Not up to date";
                if(!pDatabaseEntry->m_strLastInfoMsg.isEmpty())
                {
                    strDbInfo = pDatabaseEntry->m_strLastInfoMsg;
                }

                // Check if it is a new version
                if(strSupportedVersion.section("B",1).simplified().toInt() < strPluginVersion.section("B",1).simplified().toInt())
                    strDbInfo = "Incompatible version";

                strUpToDate = "red-flag.png";
                strToolTip+= "\nPlugin Status:      \t"+strDbInfo;

                if(strDbInfo.contains(":"))
                    strDbInfo = strDbInfo.section(":",1).simplified();
                if(strDbInfo.contains("."))
                    strDbInfo = strDbInfo.section(".",0,0).simplified();
            }

            if(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
            {
                strConnected = "enable.png";
                strSize = "Empty";
            }
            else
            {
                // Could be manually disconnected
                if(pDatabaseEntry->StatusFlags() & STATUS_MANUALLY_DISCONNECTED)
                {
                    strConnected = "disable.png";
                    if(pDatabaseEntry->m_strLastInfoMsg.isEmpty())
                        strDbInfo = "Disconnected";
                    else
                    {
                        strToolTip+= "\n\n"+QString(pDatabaseEntry->m_strLastInfoMsg).replace(":",":\n");
                        // Take the last message
                        strDbInfo = pDatabaseEntry->m_strLastInfoMsg.section(":",pDatabaseEntry->m_strLastInfoMsg.count(":"));
                    }

                }
                else
                {
                    strToolTip+= "\n\n"+QString(pDatabaseEntry->m_strLastInfoMsg).replace(":",":\n");
                    strDbInfo = pDatabaseEntry->m_strLastInfoMsg.section(":",pDatabaseEntry->m_strLastInfoMsg.count(":"));
                }
            }


            // Get the Database Size
            if(pDatabaseEntry->DbSize() > 0.0)
                strSize = GS::Gex::Engine::GetInstance().GetDatabaseEngine().BuildDatabaseSize(
                            pDatabaseEntry->DbSize(),
                            pDatabaseEntry->IsExternal()).section(" (",0,0);


            strPluginName = pDatabaseEntry->m_pExternalDatabase->GetPluginName();
            if(strPluginName.contains("["))
                strPluginName = strPluginName.section("[",0,0).simplified();
        }

        // Enabled or disabled flag for button access
        if(pDatabaseEntry->IsExternal() && pDatabaseEntry->m_pExternalDatabase)
        {
            pDatabaseEntry->UnsetGuiFlags(BUTTON_EDIT);
            pDatabaseEntry->UnsetGuiFlags(BUTTON_REMOVE);
            pDatabaseEntry->UnsetGuiFlags(BUTTON_INSERTION);
            pDatabaseEntry->UnsetGuiFlags(BUTTON_HOUSEKEEPING);
            pDatabaseEntry->UnsetGuiFlags(BUTTON_EDIT_HEADER);
        }

        if(strDbInfo.endsWith("."))
            strDbInfo = strDbInfo.left(strDbInfo.length()-1).simplified();

        // Set font color
        bool bGrey = false;
        if((strConnected == "disable.png")
                || (pDatabaseEntry->StatusFlags() & STATUS_INTERNAL_REMOVE))
            bGrey = true;

        fillCellData(pQtwiItem,0, pDatabaseEntry->LogicalName(),strToolTip,true,bGrey);     // 'Database name' column
        fillCellData(pQtwiItem,2, strSize,strToolTip,true,bGrey);                           // 'Size' column
        fillCellData(pQtwiItem,3, strDatabaseStorage,strToolTip,true,bGrey);                // 'Data storage' column
        fillCellData(pQtwiItem,4, strTdrType,strToolTip,true,bGrey);                        // 'TDR Type' column
        fillCellData(pQtwiItem,6, strDbVersion,"",true,bGrey);                              // 'Db version' column
        fillCellData(pQtwiItem,8, strDbInfo,strToolTip,true,bGrey);                         // 'Db Info' column
        fillCellData(pQtwiItem,9, strStartupType,strToolTip,true,bGrey);                    // 'Startup Type' column
        fillCellData(pQtwiItem,10, strPluginName,strToolTip,true,bGrey);                    // 'Plugin Name' column
        fillCellData(pQtwiItem,11, pDatabaseEntry->Description(),strToolTip,true,bGrey);    // 'Description' column

        // Update icons
        pQtwiItem->setIcon(1, QIcon(QString::fromUtf8(strLocation.toLatin1().constData())));	// 'Type' column
        pQtwiItem->setToolTip(1,strType);

        if(!strConnected.isEmpty())
        {
            pQtwiItem->setIcon(5, QIcon(QString::fromUtf8(QString(":/gex/icons/"+strConnected).toLatin1().constData())));	// 'Connected' column
            if(strConnected == "enable.png")
                pQtwiItem->setToolTip(5,"Database is connected");
            else
                pQtwiItem->setToolTip(5,"Database is not connected");
        }

        if(!strUpToDate.isEmpty())
        {
            pQtwiItem->setIcon(7, QIcon(QString::fromUtf8(QString(":/gex/icons/"+strUpToDate).toLatin1().constData())));	// 'Up to date' column
            if(strUpToDate == "red-flag.png")
                pQtwiItem->setToolTip(7,"Database is not up to date\nSupported Plugin Version: " + pDatabaseEntry->m_pExternalDatabase->GetPluginName().section("[Plugin ",1).section("]",0,0));
            else if(strUpToDate == "green-flag.png")
                pQtwiItem->setToolTip(7,"Database is up to date");
            else if(strUpToDate == "blue-flag.png")
                pQtwiItem->setToolTip(7,"Database is not up to date but still compatible\nNew Plugin Version: " + pDatabaseEntry->m_pExternalDatabase->GetPluginName().section("[Plugin ",1).section("]",0,0));
        }

        // Do not display INTERNAL_REMOVE
        if(pDatabaseEntry->StatusFlags() & STATUS_INTERNAL_REMOVE)
        {
            pQtwiItem->setHidden(true);
        }

        // Do not display internal BlackHole
        if(pDatabaseEntry->IsBlackHole() && pDatabaseEntry->IsStoredInDb())
        {
            if(NewItem)
                GSLOG(SYSLOG_SEV_DEBUG, QString("Hide Database Entry [%1]: Internal Black-Hole").arg(pDatabaseEntry->LogicalName()).toLatin1().constData());
            pQtwiItem->setHidden(true);
        }

        // Don't show ADR databases
        if (pDatabaseEntry && pDatabaseEntry->IsAdr())
        {
            pQtwiItem->setHidden(true);
        }

        // Ignore non YM db in monitoring
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                && (pDatabaseEntry->IsCharacTdr() || pDatabaseEntry->IsManualProdTdr()))
        {
            if(!NewItem && !pQtwiItem->isHidden())
                GSLOG(SYSLOG_SEV_NOTICE, QString("Hide Database Entry [%1]: TDR type not supported")
                      .arg(pDatabaseEntry->LogicalName()).toLatin1().constData());
            pQtwiItem->setHidden(true);
        }

        if(pDatabaseEntrySelected)
            break;
    };

    // Enable/Disable GUI icons if list is empty/not empty
    OnSelectDatabase(pSelectDatabaseEntry);

    // resize columns
    if(ResizeColumn)
    {
        for(int ii=0; ii<treeWidgetDatabase->columnCount(); ii++)
            treeWidgetDatabase->resizeColumnToContents(ii);
    }
}

///////////////////////////////////////////////////////////
// Update GUI
///////////////////////////////////////////////////////////
void AdminGui::DisplayStatusMessage(QString strText/*=""*/)
{
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(strText);
}

///////////////////////////////////////////////////////////
// Update GUI fileds based on Product type running
///////////////////////////////////////////////////////////
void AdminGui::UpdateSkin(int /*lProductID*/)
{
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void AdminGui::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Receive drag move events
///////////////////////////////////////////////////////////
void AdminGui::dragMoveEvent(QDragMoveEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->hasUrls())
    {
        if (e->mimeData()->urls().first().toLocalFile().endsWith(QString(DB_INI_FILE)))
            return ;



        treeWidgetDatabase->activateWindow();
        QTreeWidgetItem *pSelectedItem = treeWidgetDatabase->
                itemAt(treeWidgetDatabase->mapFrom(this, e->pos()));
        if (pSelectedItem)
        {
            // Extract database name
            QString strDatabaseName = pSelectedItem->text(0);
            GexDatabaseEntry *ptDatabaseEntry = NULL;
            // Find database ptr
            ptDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseName,false);
            if(ptDatabaseEntry == NULL)
                return;

            if ((ptDatabaseEntry->StatusFlags() & STATUS_CONNECTED) && !(ptDatabaseEntry->GuiFlags() & GUI_READONLY))
                treeWidgetDatabase->setCurrentItem(pSelectedItem);
            e->acceptProposedAction();
        }
    }
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void AdminGui::dropEvent(QDropEvent *e)
{
    if (!e->mimeData()->formats().contains("text/uri-list"))
    {
        e->ignore();
        return;
    }

    QString		strFileName;
    QStringList strFileList;
    QList<QUrl> lstUrls = e->mimeData()->urls();

    for (int nUrl = 0; nUrl < lstUrls.count(); nUrl++)
    {
        strFileName = lstUrls.at(nUrl).toLocalFile();

        if (!strFileName.isEmpty())
            strFileList << strFileName;
    }

    if(strFileList.count() <= 0)
    {
        // Items dropped are not regular files...ignore.
        e->ignore();
        return;
    }

    // Check if is a gexdb_ini.xml file
    if ((strFileList.count() == 1) && (QFileInfo(strFileList.first()).fileName().endsWith(QString(DB_INI_FILE))))
    {
        ImportDatabase(strFileList.first());
        return;
    }

    // Check if a database exists or is selected.
    if(treeWidgetDatabase->topLevelItemCount() <= 0)
    {
        QMessageBox::warning(this,"", "You must create a database entry first!");
        return;
    }

    // Check if a database name has been selected
    GexDatabaseEntry *pDatabaseEntry = GetSelectedDatabase(true);
    if(pDatabaseEntry == NULL)
    {
        QMessageBox::warning(this,"", "You must first select a connected database\n"
                             "(simply click its entry name)");
        return;
    }

    // Check if the insertion is allowed
    OnSelectDatabase(pDatabaseEntry);
    if(!buttonAddFiles->isVisible() || !buttonAddFiles->isEnabled())
    {
        e->ignore();
        return;
    }

    // Ask user to confirm data insertion...
    QString strCurrentItemDatabaseName = pDatabaseEntry->LogicalName();
    QString strMessage = "Do you want to import data file(s)\ninto database: " + strCurrentItemDatabaseName + " ?";
    bool lOk;
    GS::Gex::Message::request("Data Insertion", strMessage, lOk);
    if (! lOk)
    {
        return;
    }

    // Insert files selected into the active database
    OnImportFiles(strCurrentItemDatabaseName,strFileList);

    e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Contextual menu: Create/Remove database, import files
///////////////////////////////////////////////////////////
void AdminGui::contextMenuRequestedAdmin(const QPoint& /*pos = QPoint()*/)
{
    QMenu menu(this);
    QIcon icon;
    GexDatabaseEntry *pDatabaseEntry=NULL;

    // Build menu.
    menu.addAction(*pixCreateFolder,"Create a new Database link", this, SLOT(OnCreateDatabase()))->setEnabled(buttonNewDatabase->isEnabled());
    menu.addAction("Import new Database link", this, SLOT(OnImportDatabase()));
    menu.addAction("Export this Database link", this, SLOT(OnExportDatabase()))
            ->setEnabled(buttonEditDatabase->isEnabled());

    // Check if DB list has some elements
    if(treeWidgetDatabase->topLevelItemCount() == 0)
        goto labelExit;

    // Make sure an item is selected
    pDatabaseEntry = GetSelectedDatabase();
    if(!pDatabaseEntry)
        goto labelExit;

    // Update flags (after connection)
    OnSelectDatabase(pDatabaseEntry);

    menu.addSeparator();
    if(pDatabaseEntry->IsExternal() && pDatabaseEntry->m_pExternalDatabase)
    {
        bool bConnectButton = true;
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            bConnectButton = buttonRemoveDatabase->isEnabled();

        if(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
            menu.addAction(QPixmap(QString::fromUtf8(":/gex/icons/disable.png")),"Disconnect this database" , this, SLOT(OnDisconnectDatabase()))->setEnabled(bConnectButton);
        else
            if(pDatabaseEntry->StatusFlags() & STATUS_MANUALLY_DISCONNECTED)
                menu.addAction(QPixmap(QString::fromUtf8(":/gex/icons/enable.png")),"Connect this database", this, SLOT(OnConnectDatabase()))->setEnabled(bConnectButton);
            else
                menu.addAction(QPixmap(QString::fromUtf8(":/gex/icons/disable.png")),"Disconnect this database" , this, SLOT(OnConnectDatabase()))->setEnabled(bConnectButton);
    }


    icon.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/housekeeping.png")), QIcon::Normal, QIcon::Off);
    menu.addAction(icon,"Manage this Database", this, SLOT(OnHousekeeping()))->setEnabled(buttonHousekeeping->isEnabled());
    if(buttonAddFiles->isEnabled())
        menu.addAction(*pixOpen,"Add files to this database", this, SLOT(OnImportFiles()))->setEnabled(buttonAddFiles->isEnabled());

    menu.addSeparator();
    menu.addAction(*pixProperties,"Edit this Database link", this, SLOT(OnEditDatabase()))->setEnabled(buttonEditDatabase->isEnabled());
    menu.addAction(*pixRemove,"Delete this Database link", this, SLOT(OnDeleteDatabase()))->setEnabled(buttonRemoveDatabase->isEnabled());

labelExit:
    menu.setMouseTracking(true);
    menu.exec(QCursor::pos());
    menu.setMouseTracking(false);
}


///////////////////////////////////////////////////////////
// Contextual menu: Create/Remove database, import files
///////////////////////////////////////////////////////////
void AdminGui::contextMenuRequestedUser(const QPoint& /*pPoint*/)
{
    // Check if yieldman is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    // Check if a user is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
        return;

    QMenu *pMenu = new QMenu(this);
    QAction *pAction_NewEntry = NULL;
    QAction *pAction_EditEntry = NULL;
    QAction *pAction_EditEntryForAll = NULL;
    QAction *pAction_DeleteEntry = NULL;

    int nNbUsers = 0;
    QStringList lUsers;
    for(int iRow=0 ; iRow<tableWidgetUser->rowCount(); ++iRow)
    {
        if(tableWidgetUser->isRowHidden(true))
            continue;
        if(tableWidgetUser->item(iRow,0)->isSelected())
        {
            ++nNbUsers;
            lUsers += tableWidgetUser->item(iRow,0)->text();
        }
    }

    bool bUserGroupAdminConnected = GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges();

    if(buttonNewUser->isEnabled())
    {
        if(bUserGroupAdminConnected)
            pAction_NewEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/yieldmandb_groups_mng.png")),"Manage Users/Groups properties..." );
        else
            pAction_NewEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/yieldmandb_user_mng.png")),"Manage your properties..." );
    }

    // Check if found a selection (the 1st one)
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.count() == 1)
    {
        // Only the master user
        if(nNbUsers == 1)
            pAction_EditEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/properties.png")),"Edit your options..." );
    }
    else
    {
        pMenu->addSeparator();
        pAction_EditEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/properties.png")),"Edit your options..." );
        // Check if it is not me
        if((nNbUsers > 0)
                && (GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser
                    && !lUsers.contains(QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nUserId))))
            pAction_EditEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/properties.png")),"Edit options for users selected" );

        if(buttonEditUser->isEnabled() && bUserGroupAdminConnected)
            pAction_EditEntryForAll = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/properties.png")),"Edit options for all users" );
        if(!buttonRemoveUser->isHidden() && (nNbUsers == 1))
            pAction_DeleteEntry = pMenu->addAction(QPixmap(QString::fromUtf8(":/gex/icons/file_remove.png")),"Delete this user" );
    }

    // Display menu...
    pMenu->setMouseTracking(true);
    QAction *pActionResult = pMenu->exec(QCursor::pos());
    pMenu->setMouseTracking(false);

    // Check menu selection activated
    if(pActionResult == NULL)
        return;

    if(pActionResult == pAction_NewEntry)
        OnManageUsersGroups();
    else if(pActionResult == pAction_EditEntry)
        OnEditUser();
    else if(pActionResult == pAction_EditEntryForAll)
        OnEditUser(true);
    else if(pActionResult == pAction_DeleteEntry)
        OnDeleteUser();
}

///////////////////////////////////////////////////////////
// Edit an existing Database entry
///////////////////////////////////////////////////////////
void	AdminGui::OnEditDatabase()
{
    switch (stackedWidgetDbList->currentIndex())
    {
    case TAB_ENTERPRISE_INDEX:
        OnEditEnterpriseDatabase();
        break;
    case TAB_LOCAL_INDEX:
        break;
    }
}

///////////////////////////////////////////////////////////
// Switch between
// - Edit an existing Database entry
// - Housekeeping
// when double click on a database
///////////////////////////////////////////////////////////
void AdminGui::OnDoubleClickDatabase()
{
    switch (stackedWidgetDbList->currentIndex())
    {
    case TAB_ENTERPRISE_INDEX:
        // Check if DB list has some elements
        if(treeWidgetDatabase->topLevelItemCount() == 0)
            return;

        GexDatabaseEntry	*pDatabaseEntry;
        // Get selected database
        pDatabaseEntry = GetSelectedDatabase();
        if(!pDatabaseEntry)
            return;

        // Update flags (after connection)
        OnSelectDatabase(pDatabaseEntry);

        // If the database is connected and the HouseKeeping is activated
        // Goto the House Keeping GUI
        // Else goto the Edit Connection GUI
        if(pDatabaseEntry->GuiFlags() & BUTTON_HOUSEKEEPING)
            OnHousekeeping();
        else
            OnEditEnterpriseDatabase();
        break;
    case TAB_LOCAL_INDEX:
        break;
    }
}

///////////////////////////////////////////////////////////
// Edit an existing Database entry
///////////////////////////////////////////////////////////
void	AdminGui::OnEditEnterpriseDatabase()
{
    // Check if a user is connected
    if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer())
        return;

    if(!buttonEditDatabase->isEnabled())
        return;

    GexDatabaseEntry	*pDatabaseEntry;

    // Check if DB list has some elements
    if(treeWidgetDatabase->topLevelItemCount() == 0)
        return;

    // Get selected database
    pDatabaseEntry = GetSelectedDatabase();
    if(!pDatabaseEntry)
        return;

    // Update flags (after connection)
    OnSelectDatabase(pDatabaseEntry);

    // If external database, display config wizard
    if(pDatabaseEntry->IsExternal() && pDatabaseEntry->m_pExternalDatabase)
    {
        // Check if user can read this DB
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsInRestrictedMode(pDatabaseEntry))
        {
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToRead(pDatabaseEntry))
            {
                QString strMsg;
                strMsg += "Database "+pDatabaseEntry->LogicalName() + " is in Restricted Access Mode.\n\n";
                strMsg += "You are not allowed to edit this database.";
                QMessageBox::warning(this,"Restricted Mode",
                                     strMsg);
                return;
            }
        }

        GexDbPlugin_ID *pPluginID = pDatabaseEntry->m_pExternalDatabase->GetPluginID();

        if((pPluginID == NULL) || (pPluginID->m_pPlugin == NULL))
        {
            const std::vector<std::string> lstErrorMsg = GGET_ERRORSTRINGLIST(GexRemoteDatabase,pDatabaseEntry->m_pExternalDatabase);
            QString strMsg;
            for(int i = lstErrorMsg.size()-1; i >= 0 ; i--)
            {
                if(!strMsg.isEmpty())
                    strMsg += "\n";

                strMsg += QString(lstErrorMsg[i].c_str());
            }

            if(strMsg.isEmpty())
                strMsg = "No Database plugin found";
            QMessageBox::warning(this,"Database " + pDatabaseEntry->LogicalName(),
                                 "Connection fail :\n" + strMsg);
            return;
        }

        bool bReadOnly = false;

        // Database property is ReadOnly if no user connected or no enought permission
        bReadOnly = pDatabaseEntry->GuiFlags() & GUI_READONLY;

        if(pDatabaseEntry->IsStoredInDb())
        {
            if(!bReadOnly
                    && GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            {
                bReadOnly = !GS::Gex::Engine::GetInstance().GetAdminEngine().Lock(pDatabaseEntry);
                if(bReadOnly)
                {
                    QString msg;
                    GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(msg);
                    QMessageBox::information(pGexMainWindow,
                                             GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                             msg);
                }

            }
        }

        QMap<QString, QString>      lProductInfoMap;
        QMap<QString, QString>      lGuiOptionsMap;

        // Set  product info
        lProductInfoMap.insert("product_id",
                               QString::number(GS::LPPlugin::ProductInfo::getInstance()->getProductID()));
        lProductInfoMap.insert("optional_modules",
                               QString::number(GS::LPPlugin::ProductInfo::getInstance()->getOptionalModules()));
        QString lDbTdrType;
        if(pPluginID->m_pPlugin->m_pclDatabaseConnector &&
                !pPluginID->m_pPlugin->m_pclDatabaseConnector->m_strSchemaName.isEmpty())
            lDbTdrType = pPluginID->m_pPlugin->m_pclDatabaseConnector->m_strSchemaName;
        else
            lDbTdrType = pDatabaseEntry->LogicalName();
        lDbTdrType += pDatabaseEntry->TdrTypeRecorded();
        lProductInfoMap.insert("db_type_recorded",
                               QCryptographicHash::hash(lDbTdrType.toAscii(),QCryptographicHash::Md5).toHex());

        // Set GUI options
        lGuiOptionsMap.insert("open_mode","edition");
        lGuiOptionsMap.insert("read_only",(bReadOnly ? "yes" : "no"));
        lGuiOptionsMap.insert("allowed_database_type",pDatabaseEntry->TdrTypeRecorded());

        m_bDatabaseEdited = true;
        if(!bReadOnly)
            pDatabaseEntry->TraceUpdate("EDIT","START","Database edition");

        pPluginID->setConfigured(pPluginID->m_pPlugin->ConfigWizard(lProductInfoMap,
                                                                    lGuiOptionsMap));

        // Change the ADR IP if the IP has been changed for a yield man prod data base
        if (pPluginID->isConfigured() && pDatabaseEntry->m_pExternalDatabase->MustHaveAdrLink())
        {
            // Try first of all to connect the ADR
            GexDatabaseEntry	*lADRDatabaseEntry;
            lADRDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(pDatabaseEntry->m_pExternalDatabase->GetAdrLinkName(),false);
            if(lADRDatabaseEntry)
            {
                lADRDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name =
                        pPluginID->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name;
                lADRDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_pclDatabaseConnector->m_strHost_IP =
                        pPluginID->m_pPlugin->m_pclDatabaseConnector->m_strHost_IP;
                lADRDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_pclDatabaseConnector->m_strHost_LastUsed =
                        pPluginID->m_pPlugin->m_pclDatabaseConnector->m_strHost_LastUsed;
                lADRDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_pclDatabaseConnector->m_strHost_Unresolved =
                        pPluginID->m_pPlugin->m_pclDatabaseConnector->m_strHost_Unresolved;
                lADRDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin->m_pclDatabaseConnector->m_strDriver =
                        pPluginID->m_pPlugin->m_pclDatabaseConnector->m_strDriver;
                if((lADRDatabaseEntry->IsStoredInDb())
                        && (!GS::Gex::Engine::GetInstance().GetAdminEngine().SaveDatabase(lADRDatabaseEntry)))
                {
                    QString strError;
                    GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
                    QMessageBox::warning(this,"YieldManDb error",
                                         strError + "\nContact Quantix support at " +
                                         QString(GEX_EMAIL_SUPPORT) +
                                         " for more details.\n\nThe software will now exit.");
                    lADRDatabaseEntry->TraceUpdate("EDIT","FAIL",strError);
                    GS::Gex::Engine::GetInstance().GetAdminEngine().Unlock(pDatabaseEntry);
                    return;
                }
            }
        }

        m_bDatabaseEdited = false;
        if(!bReadOnly && pPluginID->isConfigured())
        {
            ShowDatabaseList(pDatabaseEntry);

            if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            {
                if((pDatabaseEntry->IsStoredInDb())
                        && (!GS::Gex::Engine::GetInstance().GetAdminEngine().SaveDatabase(pDatabaseEntry)))
                {
                    QString strError;
                    GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
                    QMessageBox::warning(this,"YieldManDb error",
                                         strError + "\nContact Quantix support at " +
                                         QString(GEX_EMAIL_SUPPORT) +
                                         " for more details.\n\nThe software will now exit.");
                    pDatabaseEntry->TraceUpdate("EDIT","FAIL",strError);
                    GS::Gex::Engine::GetInstance().GetAdminEngine().Unlock(pDatabaseEntry);
                    return;
                }
            }

            pDatabaseEntry->SaveToXmlFile(QString());

            GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckDatabaseEntryStatus(pDatabaseEntry);
            ShowDatabaseList(pDatabaseEntry);

            // Tasks are disabled when database not connected or with error
            // Reload Tasks list to recheck task status
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTasksList();

        }
        if(!bReadOnly)
            pDatabaseEntry->TraceUpdate("EDIT",(pPluginID->isConfigured()?"PASS":"FAIL"),
                                        (pPluginID->isConfigured()?"Saved":"Edition canceled"));

    }

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        GS::Gex::Engine::GetInstance().GetAdminEngine().Unlock(pDatabaseEntry);

    ShowDatabaseList(pDatabaseEntry);
}

///////////////////////////////////////////////////////////
// Connect or disconnect current database
///////////////////////////////////////////////////////////
void	AdminGui::OnConnectDatabase(void)
{
    GexDatabaseEntry	*pDatabaseEntry;

    // Check if DB list has some elements
    if(treeWidgetDatabase->topLevelItemCount() == 0)
        return;

    // Get selected item
    pDatabaseEntry = GetSelectedDatabase();
    if(!pDatabaseEntry)
        return;

    // if the DB is a production DB and contains an ADR, connect both TDR and ADR
    if(pDatabaseEntry->IsExternal() && pDatabaseEntry->m_pExternalDatabase)
    {

        if (pDatabaseEntry->m_pExternalDatabase->MustHaveAdrLink())
        {
            // Try first of all to connect the ADR
            GexDatabaseEntry	*lADRDatabaseEntry;
            lADRDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(pDatabaseEntry->m_pExternalDatabase->GetAdrLinkName(),false);
            if(lADRDatabaseEntry)
            {
                bool lIsTdrConnected = (pDatabaseEntry->StatusFlags() & STATUS_CONNECTED);
                bool lIsAdrConnected = (lADRDatabaseEntry->StatusFlags() & STATUS_CONNECTED);
                // Check if there are aligned
                if(lIsTdrConnected == lIsAdrConnected)
                    CheckDBConnection(lADRDatabaseEntry);
            }
        }
        CheckDBConnection(pDatabaseEntry);
    }
    ShowDatabaseList(pDatabaseEntry);

    // Tasks are disabled when database not connected
    // Reload Tasks list to recheck task status
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTasksList();
}

void AdminGui::OnDisconnectDatabase()
{
    GexDatabaseEntry	*pDatabaseEntry = NULL;

    // Check if DB list has some elements
    if(treeWidgetDatabase->topLevelItemCount() == 0)
        return;

    // Get selected item
    pDatabaseEntry = GetSelectedDatabase();
    if(!pDatabaseEntry)
        return;

    GS::Gex::Engine::GetInstance().GetDatabaseEngine().DisconnectDatabase(pDatabaseEntry->LogicalName());
    ShowDatabaseList(pDatabaseEntry);

    // Tasks are disabled when database not connected
    // Reload Tasks list to recheck task status
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTasksList();
}

bool AdminGui::CheckDBConnection(GexDatabaseEntry *pDatabaseEntry)
{
    // Check if user can read this DB
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsInRestrictedMode(pDatabaseEntry))
    {
        if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToRead(pDatabaseEntry))
        {
            QString strMsg;
            strMsg += "Database <b>'"+pDatabaseEntry->LogicalName() + "'</b> is in Restricted Access Mode.<BR><BR>";
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
            {
                strMsg += "You do not have the correct privileges to access this database.<BR>";
                strMsg += "Please log in as a different user - OR - <BR>";
                strMsg += "Contact your company'�s Yield-Man Admin to grant you access if necessary.";
            }
            else
            {
                strMsg += "You need to log in to access this database.";
            }
            QMessageBox::warning(this,"Database Restricted Access Mode",
                                 strMsg);
            return false;
        }
    }

    if(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
    {
        pDatabaseEntry->UnsetStatusFlags(STATUS_CONNECTED);
    }
    else if(pDatabaseEntry->StatusFlags() & STATUS_MANUALLY_DISCONNECTED)
    {
        pDatabaseEntry->UnsetStatusFlags(STATUS_MANUALLY_DISCONNECTED);
    }
    else
    {
        // Database not connected due to some error
        // Manually Disconnect
        pDatabaseEntry->SetStatusFlags(STATUS_MANUALLY_DISCONNECTED);
    }

    GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckDatabaseEntryStatus(pDatabaseEntry,
                                                                                !(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED));
    return true;
}

///////////////////////////////////////////////////////////
// Edit an existing Database entry
///////////////////////////////////////////////////////////
void AdminGui::OnSelectDatabase(GexDatabaseEntry *ptDatabaseEntry)
{
    // Make sure have some items
    if(treeWidgetDatabase->topLevelItemCount() == 0)
    {
        CheckButtonsStatus();
        return;
    }

    GexDatabaseEntry	*pDatabaseEntry = ptDatabaseEntry;

    // Check if DB list has some elements
    if((pDatabaseEntry == NULL)
            || (treeWidgetDatabase->currentItem() == NULL))
    {
        pDatabaseEntry = GetSelectedDatabase();
        if(pDatabaseEntry == NULL)
        {
            // Select 1st database (not hidden) in the list.
            QTreeWidgetItem *pQtwiItem = NULL;
            for(int i=0; i<treeWidgetDatabase->topLevelItemCount();i++)
            {
                pQtwiItem = treeWidgetDatabase->topLevelItem(i);
                if(pQtwiItem && !pQtwiItem->isHidden())
                    break;
                pQtwiItem = NULL;
            }
            if(pQtwiItem != NULL)
            {
                // emit signal on select
                treeWidgetDatabase->setCurrentItem(pQtwiItem);
                treeWidgetDatabase->setItemSelected(pQtwiItem,true);
                return;
            }
        }
    }

    // Make sure an item is selected
    if((treeWidgetDatabase->topLevelItemCount() == 0)
            || (pDatabaseEntry == NULL))
    {
        CheckButtonsStatus();
        return;
    }

    // Enabled or Disabled some access
    bool bUserConnected = false;
    bool bEnabled = false;

    // Check if yieldman is connected
    // Check if connected as admin
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        bUserConnected = GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected();
    else
        bEnabled = bUserConnected = true;

    if (!pDatabaseEntry->IsExternal())
        bEnabled = true;
    else if (!pDatabaseEntry->IsStoredInDb())
        bEnabled = true;
    else if(pDatabaseEntry->IsYmProdTdr())
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            // User must be connected to edit Uploaded database
            bEnabled = bUserConnected;
            // Monitoring must be paused
            if(bEnabled)
                bEnabled = GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped();
        }
    }
    else if (pDatabaseEntry->IsCharacTdr() || pDatabaseEntry->IsManualProdTdr())
    {
        if (GS::LPPlugin::ProductInfo::getInstance()->isTDRAllowed() &&
                !GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            bEnabled = true;
    }
    // DB not recognize only editable on monitoring
    else if (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        bEnabled = true;


    if(!pDatabaseEntry)
    {
        // Database is corrupted
        buttonEditDatabase->setEnabled(bEnabled);
        buttonRemoveDatabase->setEnabled(bEnabled);
        return;
    }

    pDatabaseEntry->UnsetGuiFlags(GUI_READONLY);
    pDatabaseEntry->UnsetGuiFlags(BUTTON_EDIT);
    pDatabaseEntry->UnsetGuiFlags(BUTTON_REMOVE);
    pDatabaseEntry->UnsetGuiFlags(BUTTON_INSERTION);
    pDatabaseEntry->UnsetGuiFlags(BUTTON_HOUSEKEEPING);
    pDatabaseEntry->UnsetGuiFlags(BUTTON_EDIT_HEADER);

    // Edit
    if(pDatabaseEntry->StatusFlags() & STATUS_EDIT)
    {
        if ((pDatabaseEntry->IsYmProdTdr() && GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() && bUserConnected) ||
            (!pDatabaseEntry->IsYmProdTdr()))
        {
            pDatabaseEntry->SetGuiFlags(BUTTON_EDIT);
        }
        pDatabaseEntry->SetGuiFlags(GUI_READONLY);
        if(bEnabled
                || (!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() // or not YM and Local
                    && !pDatabaseEntry->IsStoredInDb()))
            pDatabaseEntry->UnsetGuiFlags(GUI_READONLY);
    }

    // Remove
    if((pDatabaseEntry->StatusFlags() & STATUS_REMOVE)
            && (bUserConnected                                 // User connected
                || (!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() // or not YM and Local
                    && !pDatabaseEntry->IsStoredInDb())))
        pDatabaseEntry->SetGuiFlags(BUTTON_REMOVE);

    // Manual insertion
    if((pDatabaseEntry->StatusFlags() & STATUS_INSERTION)
            && bEnabled
            && !pDatabaseEntry->IsYmProdTdr()                   // not YM Prod
            && (pDatabaseEntry->StatusFlags() & STATUS_CONNECTED)
            && (pDatabaseEntry->IsCompatible()))
        pDatabaseEntry->SetGuiFlags(BUTTON_INSERTION);

    // HouseKeeping
    if((pDatabaseEntry->StatusFlags() & STATUS_HOUSEKEEPING)
            && bEnabled
            && (pDatabaseEntry->StatusFlags() & STATUS_CONNECTED))
        pDatabaseEntry->SetGuiFlags(BUTTON_HOUSEKEEPING);

    // Edit header for File-Based
    if(!pDatabaseEntry->IsExternal())
        pDatabaseEntry->SetGuiFlags(BUTTON_EDIT_HEADER);

    CheckButtonsStatus(pDatabaseEntry);
}

///////////////////////////////////////////////////////////
// Reload Database entry
///////////////////////////////////////////////////////////
void	AdminGui::OnReloadDatabaseList(void)
{
    if(!mWindowDetached)
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                && !GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
        {
            buttonReloadList->setEnabled(false);

            QString strApp = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
            // Get the short name
            if(strApp.count("-") > 1)
                strApp = strApp.section("-",0,1);
            else
                strApp = strApp.section("-",0,0);

            QString strMsg = strApp+" is running!<br><br>\n";
            strMsg += "Please pause the Monitoring Scheduler <img src=\""+QString::fromUtf8(":/gex/icons/mo_scheduler.png")+"\" WIDTH=\"20\" HEIGHT=\"20\"> <br>before reloading the databases list.";
            QString strTitle = strApp + " request";
            QMessageBox::warning(this,strTitle, strMsg);
            return;
        }

        if(m_bDatabaseEdited)
        {
            QString strApp = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
            // Get the short name
            if(strApp.count("-") > 1)
                strApp = strApp.section("-",0,1);
            else
                strApp = strApp.section("-",0,0);
            QString strMsg = "A database is being edited!<br><br>\n";
            strMsg += "Please finalize your modification <br>before reloading the databases list.";
            QString strTitle = strApp + " request";
            QMessageBox::information(this,strTitle, strMsg);
            return;
        }

        m_bDatabaseEdited = true;

        // Before to delete DatabaseEntry pointer
        // Reset m_pRdbOptionsWidget
        if(pGexMainWindow->pWizardOneQuery)
            pGexMainWindow->pWizardOneQuery->ResetRdbPluginHandles();

        QList<GexDatabaseEntry*>::iterator itBegin	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.begin();
        QList<GexDatabaseEntry*>::iterator itEnd	= GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.end();
        while(itBegin != itEnd)
        {
            (*itBegin)->SetStatusFlags(STATUS_INTERNAL_REMOVE);
            // Move to next enry.
            itBegin++;
        }

        GS::Gex::Engine::GetInstance().GetDatabaseEngine().LoadDatabasesListIfEmpty(true);

        m_bDatabaseEdited = false;
    }
    else
        OnDetachWindow(false);
}


///////////////////////////////////////////////////////////
// Create an empty Database folder
///////////////////////////////////////////////////////////
void AdminGui::OnCreateDatabase()
{
    switch (stackedWidgetDbList->currentIndex())
    {
    case TAB_ENTERPRISE_INDEX:
        OnCreateEnterpriseDatabase();
        break;
    case TAB_LOCAL_INDEX:
        break;
    }
}

///////////////////////////////////////////////////////////
// Create an empty Database folder
///////////////////////////////////////////////////////////
void	AdminGui::OnCreateEnterpriseDatabase(void)
{
    CreateNewDatabaseDialog cCreateDatabase;

    m_bDatabaseEdited = true;
    // Display dialog box.
    int nStatus = cCreateDatabase.exec();
    m_bDatabaseEdited = false;

    if(nStatus != 1)
        return;	// User 'Abort'
}

///////////////////////////////////////////////////////////
// Import given files to the selected Database
///////////////////////////////////////////////////////////
void	AdminGui::OnImportFiles(QString strDatabase,
                                QStringList sFilesSelected)
{
    QString strGexLogMsg;
    strGexLogMsg = QString("GexDataBaseAdmin::OnImportFiles() \n") + QString("\t data base : ") + strDatabase;
    strGexLogMsg += QString("\n\t selected file list : \n\t\t") + sFilesSelected.join(QString("\n\t\t"));
    GSLOG(SYSLOG_SEV_DEBUG, strGexLogMsg.toLatin1().constData());

    // Check if password protected...
    QString	strPassword = GS::Gex::Engine::GetInstance().GetDatabaseEngine().GetImportPassword(strDatabase);
    if(strPassword.isEmpty() == false)
    {
        bool ok;
        QString text = QInputDialog::getText(this,
                    GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                    "Enter Import password:", QLineEdit::Password,
                    QString::null, &ok );

        if (!ok)
            return;	// Escape
        if(text != strPassword)
        {
            // Incorrect password
            QMessageBox::warning(this,"", "Incorrect password!");
            return;
        }
    }

    // Get flag for editing data headers...
    bool bEditHeaderInfo = checkBoxEditHeader->isChecked();

    // Clear task status message
    DisplayStatusMessage("");

    // Import list of files to the given database...
    QTreeWidgetItem                     *pQtwiItem=0;
    QString                             strErrorMessage;
    QStringList                         strCorruptedFiles;
    QList<GexDatabaseInsertedFiles>     listInsertedFiles;
    //QString                             strText="Database name: "+ strDatabase;

    pGexMainWindow->ShowWizardDialog(GEXMO_HISTORY);
    pGexMainWindow->mMonitoringWidget->mUi_qlwSelectionList->setCurrentItem(
                pGexMainWindow->mMonitoringWidget->mUi_qlwSelectionList->topLevelItem(0));
    if(GS::Gex::Engine::GetInstance().GetDatabaseEngine().ImportFiles(
                strDatabase,sFilesSelected,&strCorruptedFiles,listInsertedFiles,
                strErrorMessage,bEditHeaderInfo,false ) == true)
    {
        // Success importing all files.

        // Update Database size field.
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateDatabaseEntry(strDatabase);

        // Update database GUI size field
        GexDatabaseEntry *pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabase);
        if(pDatabaseEntry && !pDatabaseEntry->IsExternal())
        {
            // Update listview to show new database size.
            pQtwiItem = treeWidgetDatabase->currentItem();
            // Case 5560: make sure current item is the good one, so that if current item was changed elsewhere
            // (through code, or by the user), we still make sure to update the right line
            if(pQtwiItem->text(0) != pDatabaseEntry->LogicalName())
            {
                for(int i=0; i<treeWidgetDatabase->topLevelItemCount();i++)
                {
                    pQtwiItem = treeWidgetDatabase->topLevelItem(i);
                    if(pQtwiItem && (pQtwiItem->text(0) == pDatabaseEntry->LogicalName()))
                        break;
                    pQtwiItem = NULL;
                }
            }

            if(pQtwiItem)
                pQtwiItem->setText(2,GS::Gex::Engine::GetInstance().GetDatabaseEngine().BuildDatabaseSize(
                                       pDatabaseEntry->CacheSize(),
                                       pDatabaseEntry->IsExternal()));
        }

        pGexMainWindow->ShowWizardDialog(GEXMO_HISTORY);
        pGexMainWindow->mMonitoringWidget->mUi_qlwSelectionList->setCurrentItem(
                    pGexMainWindow->mMonitoringWidget->mUi_qlwSelectionList->topLevelItem(0));

    }
}

///////////////////////////////////////////////////////////
// Import files to the selected Database
///////////////////////////////////////////////////////////
void	AdminGui::OnImportFiles(void)
{
    static	bool bEvalMessageInfoMessage=true;

    // If no item in list, just return!
    if(treeWidgetDatabase->topLevelItemCount() <= 0)
        return;

    // Get Handle to database selected...
    GexDatabaseEntry *pDatabaseEntry = GetSelectedDatabase(true);
    if(!pDatabaseEntry)
        return;

    // If it's a FLAT db identified by not being a TDR here
    if (pDatabaseEntry->TdrTypeRecorded().isEmpty())
    {
        QMessageBox::warning(this,
                             "", "Flat file databases are no longer supported.\n"
                             "For more information, contact " +
                             QString(GEX_EMAIL_SUPPORT));

        return;
    }


    // Database name
    QString strDatabase = pDatabaseEntry->LogicalName();
    QString strTitle = "Import files to: " + strDatabase;

    // If running in "Demo mode", display a message that using the real solution, Import would be automated!
    if(bEvalMessageInfoMessage
            && (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION))
    {
        QMessageBox::warning(this,
                             "", "Data insertion is manual in your 'Demo' release...\n"
                             "But when you'll order the solution, the 'Yield-Man'\n"
                             "will do this work for you automatically!");
        // Show it only once per session!
        bEvalMessageInfoMessage = false;
    }

    // Select files from disk...
    GS::Gex::SelectDataFiles	cSelectFiles;
    QString lWorkingFolder = GS::Gex::Engine::GetInstance().GetLastAccessedFodler();
    QStringList sFilesSelected = cSelectFiles.GetFiles(this,
                                                       lWorkingFolder,
                                                       strTitle);

    if(sFilesSelected.isEmpty())
        return;	// Empty list...ignore task!

    GS::Gex::Engine::GetInstance().UpdateLastAccessedFolder(QFileInfo(sFilesSelected.first()).absolutePath());

    // Insert list of files into the given database
    OnImportFiles(strDatabase,sFilesSelected);
}

///////////////////////////////////////////////////////////
// Delete a Database folder and content...
///////////////////////////////////////////////////////////
void AdminGui::OnDeleteDatabase()
{
    switch (stackedWidgetDbList->currentIndex())
    {
    case TAB_ENTERPRISE_INDEX:
        OnDeleteEnterpriseDatabase();
        break;
    case TAB_LOCAL_INDEX:
        break;
    }
}

void AdminGui::OnExportDatabase()
{
    // Get Handle to database selected...
    GexDatabaseEntry *lDatabaseEntry = GetSelectedDatabase(true);
    if(!lDatabaseEntry)
        return;

    // Ask user to chose a destination file
    QString lFilePath = QFileDialog::getSaveFileName(this,
                                                     "Export database link to...",
                                                     QDir::homePath(),
                                                     "Database link (*" + QString(DB_INI_FILE) + ")");
    if(lFilePath.isEmpty())
        return;

    lDatabaseEntry->SaveToXmlFile(lFilePath);
}

void AdminGui::OnImportDatabase()
{
    QString lFilePath = QFileDialog::getOpenFileName(this,
                                                     "Import database link...",
                                                     QDir::homePath(),
                                                     "Database link (*" + QString(DB_INI_FILE) + ")");
    ImportDatabase(lFilePath);
}

///////////////////////////////////////////////////////////
// Delete a Database folder and content...
///////////////////////////////////////////////////////////
void AdminGui::OnDeleteEnterpriseDatabase()
{
    QString strMessage;
    bool	bStatus;
    QTreeWidgetItem *pQtwiItem;

    // If no item in list, just return!
    if(treeWidgetDatabase->topLevelItemCount() <=0 )
        return;

    // Remove selected item (no multi-selection allowed!)
    pQtwiItem = treeWidgetDatabase->currentItem ();
    if(pQtwiItem == NULL)
        return;	// No selection!

    QString	strDatabaseToDelete = pQtwiItem->text(0);

    // Check if password protected...
    QString	strPassword = GS::Gex::Engine::GetInstance().GetDatabaseEngine().GetDeletePassword(strDatabaseToDelete);
    if(strPassword.isEmpty() == false)
    {
        bool ok;
        QString text = QInputDialog::getText(this,
                    GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                    "Enter Delete Password:", QLineEdit::Password,
                    QString::null, &ok);
        if (!ok)
            return;	// Escape
        if(text != strPassword)
        {
            // Incorrect password
            QMessageBox::warning(this,"", "Incorrect password!");
            return;
        }
    }


    // Ask for confirmation...
    strMessage = "Confirm to permanently remove database:\n" + strDatabaseToDelete;
    strMessage += "\n\n(All data files in its folder will be erased)";
    bool lOk;
    m_bDatabaseEdited = true;
    GS::Gex::Message::request("Delete Database", strMessage, lOk);
    m_bDatabaseEdited = false;
    if (! lOk)
    {
        return;
    }

    // Remove database folder and content from disk
    // Request Database Transaction module to remove Database Entry
    bStatus = GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteDatabaseEntry(strDatabaseToDelete);
    if(bStatus == false)
    {
        // Failed creating the database entry...
        QString strErrorMessage = "*Error* Failed deleting database entry\nYou probably don't have write access to the database folder.";
        QMessageBox::warning(this,"", strErrorMessage);
        return;
    }

    // Remove selected item from list
    treeWidgetDatabase->takeTopLevelItem(treeWidgetDatabase->indexOfTopLevelItem(pQtwiItem));

    if(!pQtwiItem)
    {
        delete pQtwiItem; pQtwiItem=0;
    }

    // If list box is empty...grey ImportFiles + Delete database icons
    if(treeWidgetDatabase->topLevelItemCount() <= 0)
    {
        buttonAddFiles->setEnabled(FALSE);
        buttonRemoveDatabase->setEnabled(FALSE);
    }

    // if scheduler is active, update Tasks view
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTasksList();
}



///////////////////////////////////////////////////////////
// Upload a Database folder
// Move Database folder to GalaxySemi/databases/yieldman
///////////////////////////////////////////////////////////
void	AdminGui::OnUploadDatabase(GexDatabaseEntry *pDatabaseEntry)
{
    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        return;

    GexDatabaseEntry	*pLocalDatabaseEntry = pDatabaseEntry;

    if(pLocalDatabaseEntry == NULL)
    {
        // Check if DB list has some elements
        // Find database ptr
        pLocalDatabaseEntry = GetSelectedDatabase();
    }

    if(!pLocalDatabaseEntry)
        return;

    if(!GS::Gex::Engine::GetInstance().GetDatabaseEngine().UploadDatabaseEntry(pLocalDatabaseEntry))
    {
        QString strError;
        GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
        QMessageBox::warning(this,"YieldManDb error",
                             strError + "\nContact Quantix support at " +
                             QString(GEX_EMAIL_SUPPORT) +
                             " for more details.\n\nThe software will now exit.");
        return;
    }

}

///////////////////////////////////////////////////////////
// SQL Housekeeping: update DB, purge DB...
///////////////////////////////////////////////////////////
void	AdminGui::OnHousekeeping_SQL(const QString & strDatabaseName)
{
    GexSqlHousekeepingDialog clSqlHousekeepingDlg(strDatabaseName, this, true);

    m_bDatabaseEdited = true;
    clSqlHousekeepingDlg.exec();
    m_bDatabaseEdited = false;
}

///////////////////////////////////////////////////////////
// Housekeeping: clean database (remove given products, lots, dates, etc.)
///////////////////////////////////////////////////////////
void	AdminGui::OnHousekeeping(void)
{
    GexDatabaseEntry    *pDatabaseEntry;
    QString             strDatabaseName;

    // Check if DB list has some elements
    if(treeWidgetDatabase->topLevelItemCount() == 0)
        return;

    // Get selected item
    pDatabaseEntry = GetSelectedDatabase(true);
    if(!pDatabaseEntry)
        return;

    strDatabaseName = pDatabaseEntry->LogicalName();

    // Check if SQL DB with update supported
    if(pDatabaseEntry->IsExternal())
    {
        // HouseKeeping is not allowed IF:
        // * Not with YM and YM DB
        if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() &&
                pDatabaseEntry->IsYmProdTdr())
            return;
        // * With YM and monitoring not paused
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() &&
                !GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
            return;
        // * With YM and not YM DB
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() &&
                !pDatabaseEntry->IsYmProdTdr())
            return;
        // * TDR DB not allowed and TDR DB Char or Prod manual
        if(!GS::LPPlugin::ProductInfo::getInstance()->isTDRAllowed() &&
                (pDatabaseEntry->IsCharacTdr() ||
                 pDatabaseEntry->IsManualProdTdr()))
            return;

        if(!pDatabaseEntry->m_pExternalDatabase)
            return;
        if(!pDatabaseEntry->m_pExternalDatabase->IsUpdateSupported())
            return;

        // Allow to edit if database not in readonly mode
        bool bReadOnly = false;
        // Database property is ReadOnly if no user connected or no enought permission
        bReadOnly = pDatabaseEntry->GuiFlags() & GUI_READONLY;
        // Try to lock DB
        if(pDatabaseEntry->IsStoredInDb())
        {
            if(!bReadOnly
                    && GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            {
                bReadOnly = !GS::Gex::Engine::GetInstance().GetAdminEngine().Lock(pDatabaseEntry);
                if(bReadOnly)
                {
                    QString msg;
                    GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(msg);
                    QMessageBox::information(pGexMainWindow,
                                             GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                             msg);
                }
            }
        }
        if(bReadOnly)
            return;

        // The HouseKeeping is allowed only if all nodes are paused
        // First check if we have nodes connected and active
        // Update all nodes status to have the STOP_REQUESTED
        // Wait until all nodes connected STOPPED
        // Display the housekeeping
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        {
            // Force to pause other YM
            // * With YM and YM DB
            if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() &&
                    pDatabaseEntry->IsYmProdTdr())
            {
                if(GS::Gex::Engine::GetInstance().GetAdminEngine().OtherNodesRunning())
                {
                    QString strNodesList = HtmlSection("NodesList|Running|NotMe");
                    QString strTop = "You must STOP all nodes running to be allowed to open the HouseKeeping dialog";
                    QString strBottom = "Do you want to STOP all nodes ?";
                    QString Buttons = "Yes|Cancel";
                    if(!InfoDialog(strTop, strNodesList, strBottom, Buttons)){
                        GS::Gex::Engine::GetInstance().GetAdminEngine().Unlock(pDatabaseEntry);
                        return;
                    }

                    GS::Gex::Engine::GetInstance().GetAdminEngine().StopAllOtherNodes();
                }
            }
        }

        pDatabaseEntry->TraceUpdate("HOUSEKEEPING","START","Open HouseKeeping");
        // Call SQL specific Housekeeping function
        OnHousekeeping_SQL(strDatabaseName);
        pDatabaseEntry->TraceUpdate("HOUSEKEEPING","PASS","Close HouseKeeping");

        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            GS::Gex::Engine::GetInstance().GetAdminEngine().Unlock(pDatabaseEntry);

        ShowDatabaseList(pDatabaseEntry);

        GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckDatabaseEntryStatus(pDatabaseEntry,true);
        ShowDatabaseList(pDatabaseEntry);

        // Tasks are disabled when database not uptodate
        // Reload Tasks list to recheck task status
        GS::Gex::Engine::GetInstance().GetSchedulerEngine().CheckTasksList(false);

        return;
    }

    // Not a SQL database: open standard HouseKeeping dialog
    GexOneQueryWizardPage1 cQueryDialog;
    ListDatabasesInCombo(cQueryDialog.comboBoxDatabaseName, DB_TYPE_FILEBASED);

    // Call member to resize & hide/show relevant fields when displayed as a PopUp dialog box.
    cQueryDialog.PopupSkin(GexOneQueryWizardPage1::eHouseKeeping,true);

    // Force the selected database for HouseKeeping tasks.
    if(pDatabaseEntry->IsStoredInFolderLocal() == true)
        strDatabaseName = "[Local] ";
    else
        strDatabaseName = "[Server] ";
    strDatabaseName += pDatabaseEntry->LogicalName();
    SetCurrentComboItem(cQueryDialog.comboBoxDatabaseName, strDatabaseName);

    // Display dialog box.
    m_bDatabaseEdited = true;
    int nStatus = cQueryDialog.exec();
    if(nStatus != 1)
    {
        m_bDatabaseEdited = false;
        return;	// User 'Abort'
    }
    // Load Query structure according to the GUI selections
    GexDatabaseQuery cQuery;
    cQueryDialog.GetQueryFields(cQuery);

    // Ask user to confirm data insertion...
    QString strMessage = "Do you confirm to purge the database: ";
    strMessage += cQuery.strDatabaseLogicalName + " ?";
    bool lOk;
    GS::Gex::Message::request("HouseKeeping", strMessage, lOk);
    m_bDatabaseEdited = false;
    if (! lOk)
    {
        return;
    }

    // Compute calendar period to focus on...
    QDate Today = QDate::currentDate();

    switch(cQuery.iTimePeriod)
    {
    case GEX_QUERY_HOUSEKPERIOD_TODAY:
        cQuery.calendarFrom = Today;
        cQuery.calendarTo = Today;
        break;
    case GEX_QUERY_HOUSEKPERIOD_1DAY:
        cQuery.calendarFrom = QDate(1900,1,1);
        cQuery.calendarTo = Today.addDays(-1);
        break;
    case GEX_QUERY_HOUSEKPERIOD_2DAYS:
        cQuery.calendarFrom = QDate(1900,1,1);
        cQuery.calendarTo = Today.addDays(-2);
        break;
    case GEX_QUERY_HOUSEKPERIOD_3DAYS:
        cQuery.calendarFrom = QDate(1900,1,1);
        cQuery.calendarTo = Today.addDays(-3);
        break;
    case GEX_QUERY_HOUSEKPERIOD_4DAYS:
        cQuery.calendarFrom = QDate(1900,1,1);
        cQuery.calendarTo = Today.addDays(-4);
        break;
    case GEX_QUERY_HOUSEKPERIOD_1WEEK:
        cQuery.calendarFrom = QDate(1900,1,1);
        cQuery.calendarTo = Today.addDays(-7);
        break;
    case GEX_QUERY_HOUSEKPERIOD_2WEEKS:
        cQuery.calendarFrom = QDate(1900,1,1);
        cQuery.calendarTo = Today.addDays(-14);
        break;
    case GEX_QUERY_HOUSEKPERIOD_3WEEKS:
        cQuery.calendarFrom = QDate(1900,1,1);
        cQuery.calendarTo = Today.addDays(-21);
        break;
    case GEX_QUERY_HOUSEKPERIOD_1MONTH:
        cQuery.calendarFrom = QDate(1900,1,1);
        cQuery.calendarTo = Today.addDays(-31);
        break;
    case GEX_QUERY_HOUSEKPERIOD_CALENDAR:
        // Keep dates defined!
        break;
    }
    // Force Time period mode to calendar type.
    cQuery.iTimePeriod = GEX_QUERY_TIMEPERIOD_CALENDAR;

    // Find all files matching the HouseKeeping/Purge criteria (local files only: do NOT perform any FTP!)
    cQuery.bOfflineQuery = true;
    QString lErrorMessage;
    QStringList sFilesMatchingQuery = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QuerySelectFiles(&cQuery,lErrorMessage,0);
    if(sFilesMatchingQuery.count() <= 0)
    {
        QMessageBox::warning(this,"", "No Data matching your purge criteria!");
        return;
    }

    // Clear Admin Log window...
    QString strText="############ Database name: "+ cQuery.strDatabaseLogicalName;
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().AppendMoHistoryLog("",
                                                                           "*Starting purge...*  ",
                                                                           strText);
    // Focus on 'Log file' tab
    tabWidgetAdminGui->setCurrentIndex(1);

    QString					strFile, strDir, strPrevDir;
    int						iTotalErased=0;
    double					lfFileSize=0;
    QFileInfo				cFileInfo;
    QStringList				strlFilesRemoved;
    for( QStringList::Iterator it = sFilesMatchingQuery.begin(); it != sFilesMatchingQuery.end(); ++it )
    {
        // Get file name to remove.
        strFile = *it;
        cFileInfo.setFile(strFile);

        // If new directory, update index file of previous directory
        strDir = cFileInfo.absolutePath();
        if(strPrevDir.isEmpty())
            strPrevDir = strDir;
        if(strDir != strPrevDir)
        {
            if(strlFilesRemoved.count() > 0)
                UpdateIndexFile(strPrevDir, strlFilesRemoved);
            strlFilesRemoved.clear();
            strPrevDir = strDir;
        }

        //////////// ERASE STDF FILE /////////////
        // Keep track of disk space freed
        lfFileSize += cFileInfo.size()/(1024.0*1024.0);
        if(!(pDatabaseEntry->HoldsFileCopy() == false &&
             pDatabaseEntry->IsSummaryOnly() == false) &&
                GS::Gex::Engine::RemoveFileFromDisk(strFile))
        {
            // Get pointer to last item, so we happend after it!
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().AppendMoHistoryLog("",
                                                                                   "*Erased*  ",
                                                                                   strFile);

            // Keep track of total files erased successfuly
            strlFilesRemoved.append(cFileInfo.fileName());
            iTotalErased++;
        }else
            lfFileSize -= cFileInfo.size()/(1024.0*1024.0);

        //////////// ERASE .SUM (Summary) FILE /////////////

        if(pDatabaseEntry->HoldsFileCopy() == false && pDatabaseEntry->IsSummaryOnly() == false)
        {
            GS::QtLib::DatakeysContent  cKeyContent;
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().ExtractFileKeys(strFile, cKeyContent);
            QString strDestinationPath = pDatabaseEntry->PhysicalPath();
            QDateTime	cFileDate;
            cFileDate.setTime_t(cKeyContent.Get("StartTime").toUInt());
            strDestinationPath += cFileDate.toString("/yyyy");
            strDestinationPath += cFileDate.toString("/MM");
            strDestinationPath += cFileDate.toString("/dd");
            QFileInfo cFileInfoTemp(strFile);
            QString strFileShortSTDF = cFileInfoTemp.fileName();
            strFile = strDestinationPath + "/" + strFileShortSTDF +".sum";
        }
        else
            strFile += ".sum";

        cFileInfo.setFile(strFile);
        lfFileSize += cFileInfo.size()/(1024.0*1024.0);
        if(GS::Gex::Engine::RemoveFileFromDisk(strFile))
        {
            // Get pointer to last item, so we happend after it!
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().AppendMoHistoryLog("",
                                                                                   "*Erased*  ",
                                                                                   strFile);

            // Keep track of total files erased successfuly
            iTotalErased++;
            if(pDatabaseEntry->HoldsFileCopy() == false && pDatabaseEntry->IsSummaryOnly() == false){
                QStringList strlLinkFilesRemoved;
                QString strLinkDir;
                strlLinkFilesRemoved.append(*it);
                cFileInfo.setFile(strFile);
                strLinkDir = cFileInfo.absolutePath();
                UpdateIndexFile(strLinkDir, strlLinkFilesRemoved);
            }
        }
    }

    // Update index file for last directory
    if(strlFilesRemoved.count() > 0)
        UpdateIndexFile(strDir, strlFilesRemoved);

    // Check if no file at all removed...
    if(iTotalErased == 0)
        return;

    // Update database GUI size field
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(cQuery.strDatabaseLogicalName);
    if(!pDatabaseEntry)
        return;
    strMessage = "Purge successful!\n\nTotal files erased: ";
    strMessage += QString::number(iTotalErased);
    strMessage += "\nDisk space saved: ";
    strMessage += GS::Gex::Engine::GetInstance().GetDatabaseEngine().BuildDatabaseSize(
                lfFileSize,
                pDatabaseEntry->IsExternal());

    QMessageBox::warning(this,"", strMessage);

    // Get handle to database purged
    pDatabaseEntry->SetCacheSize(lfFileSize);	// Update new database size!
    pDatabaseEntry->SaveToXmlFile(QString());

    // If Remote database, also erase the FTP history file so all next queries will retrieve files by FTP
    if(pDatabaseEntry->IsExternal())
    {
        // Update file holding list of files already FTPed over.
        QString strFtpCopies = pDatabaseEntry->PhysicalPath() + GEX_DATABASE_EXTERNAL_FTP_DEF;
        GS::Gex::Engine::RemoveFileFromDisk(strFtpCopies);
    }

    // Reload Database info & size!
    OnReloadDatabaseList();

    // Focus on 'Admin' tab
    tabWidgetAdminGui->setCurrentIndex(0);
}

///////////////////////////////////////////////////////////
// Housekeeping: update index file
///////////////////////////////////////////////////////////
void AdminGui::UpdateIndexFile(const QString & strDir, const QStringList & strlFilesRemoved)
{
    QString						strFile;
    QStringList::const_iterator	itRemoved;
    char						szLine[2048];
    char						*pLine;
    FILE						*pFile;
    long						lOffset1, lOffset2;

    // Compute nam of index file, open it
    strFile = strDir;
    strFile += GEX_DATABASE_INDEX_DEFINITION;
    pFile = fopen(strFile.toLatin1().constData(), "r+b");
    if(pFile)
    {
        // Go through list of removed files
        for(itRemoved = strlFilesRemoved.begin(); itRemoved != strlFilesRemoved.end(); itRemoved++)
        {
            // Rewind index file
            fseek(pFile, 0, SEEK_SET);
            lOffset1 = ftell(pFile);
            pLine = fgets(szLine, 2048, pFile);
            lOffset2 = ftell(pFile);
            while(pLine)
            {
                // Does the line read contain the name of the removed file?
                if((strcmp(szLine, "")) && (strstr(szLine, (*itRemoved).toLatin1().constData())))
                {
                    fseek(pFile, lOffset1, SEEK_SET);
                    fputc('#', pFile);
                    fseek(pFile, lOffset2, SEEK_SET);
                }
                // Read next line
                lOffset1 = lOffset2;
                pLine = fgets(szLine, 2048, pFile);
                lOffset2 = ftell(pFile);
            }
        }
        fclose(pFile);
    }
}


///////////////////////////////////////////////////////////
// Check button status when scheduler is actived/desactived
// Check button status when a task is selected
///////////////////////////////////////////////////////////
void AdminGui::CheckButtonsStatus(GexDatabaseEntry *pDatabaseEntry)
{
    // Enabled or Disabled some access
    // during loading databases (buttonReloadList->text() == "Close")
    // or when scheduler is running (!buttonReloadList->isEnabled())
    if(!buttonReloadList->isEnabled() || mWindowDetached)
    {
        buttonNewDatabase->setEnabled(false);
        buttonReloadList->setEnabled(mWindowDetached);
        buttonRemoveDatabase->setEnabled(false);
        buttonAddFiles->setEnabled(false);
        buttonHousekeeping->setEnabled(false);
        checkBoxEditHeader->setEnabled(false);

        if(pDatabaseEntry)
            buttonEditDatabase->setEnabled(pDatabaseEntry->GuiFlags() & BUTTON_EDIT);
        else
            buttonEditDatabase->setEnabled(false);
        return;
    }

    // Enabled or Disabled some access
    bool bUserConnected = false;
    bool bEnabled = false;
    bool bRunningScheduler = false;

    // Check if yieldman is connected
    // Check if connected as admin
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        bEnabled = bUserConnected = GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected();
    else
        bEnabled = bUserConnected = true;

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        // User must be connected to edit Uploaded database
        // If User not connected
        // Only manage local databases
        if(pDatabaseEntry && !pDatabaseEntry->IsStoredInDb())
            bEnabled = true;
        // Monitoring must be paused
        if(bEnabled)
            bEnabled = GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped();

        bRunningScheduler = !GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped();

    }
    else
    {
        // If User not connected
        // Only manage local databases
        if(pDatabaseEntry && !pDatabaseEntry->IsStoredInDb())
            bEnabled = true;
    }

    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        buttonNewDatabase->setEnabled(bUserConnected && !bRunningScheduler);
    else
        buttonNewDatabase->setEnabled(!bRunningScheduler);
    buttonReloadList->setEnabled(!bRunningScheduler);

    if(pDatabaseEntry)
    {
        buttonEditDatabase->setEnabled(pDatabaseEntry->GuiFlags() & BUTTON_EDIT);
        buttonRemoveDatabase->setEnabled(bEnabled &&
                                         (bUserConnected || !pDatabaseEntry->IsStoredInDb()) &&
                                         (pDatabaseEntry->GuiFlags() & BUTTON_REMOVE));
        buttonAddFiles->setEnabled(bEnabled &&
                                   (pDatabaseEntry->GuiFlags() & BUTTON_INSERTION));
        buttonHousekeeping->setEnabled(bEnabled &&
                                       (pDatabaseEntry->GuiFlags() & BUTTON_HOUSEKEEPING));
        checkBoxEditHeader->setEnabled(bEnabled &&
                                       (pDatabaseEntry->GuiFlags() & BUTTON_EDIT_HEADER));

    }
    else
    {
        buttonEditDatabase->setEnabled(false);
        buttonAddFiles->setEnabled(false);
        buttonRemoveDatabase->setEnabled(false);
        buttonHousekeeping->setEnabled(false);
        checkBoxEditHeader->setEnabled(false);
    }
}

///////////////////////////////////////////////////////////
// set Table cell contents + tootip
///////////////////////////////////////////////////////////
void AdminGui::fillCellData(QTableWidget *ptTable, int iRow, int iCol, QString strText, QString strTooltip/*=""*/, bool bReadOnly/*=false*/, bool bAlarm/*=false*/)
{
    QTableWidgetItem *ptItem = ptTable->item(iRow,iCol);

    if(ptItem == NULL)
    {
        ptItem = new QTableWidgetItem(strText);
        // Add item to table cell
        ptTable->setItem(iRow,iCol,ptItem);
    }
    else
    {
        // Check if need update
        if(ptItem->text() == strText)
        {
            if((strTooltip.isEmpty() && ptItem->toolTip() == strText)
                    || (!strTooltip.isEmpty() && ptItem->toolTip() == strTooltip))
                return;
        }
    }

    ptItem->setText(strText);

    // Add tooltip
    ptItem->setToolTip(strTooltip);

    // Check if Read-Only mode
    if(bReadOnly)
        ptItem->setFlags(ptItem->flags() & ~Qt::ItemIsEditable);
    else
        ptItem->setFlags(ptItem->flags() | Qt::ItemIsEditable);

    // Check if Alarm color (orange) to use as background color
    if(bAlarm)
        ptItem->setBackground(QBrush(QColor(255,146,100)));	// Orange background
}

///////////////////////////////////////////////////////////
// set Table cell contents + tootip
///////////////////////////////////////////////////////////
void AdminGui::fillCellData(QTreeWidgetItem *pQtwiItem, int iCol, QString strText,QString strTooltip, bool bReadOnly, bool bGrey)
{
    if(pQtwiItem == NULL)
        return;

    QColor rgbColor = QColor(0,0,0);
    if(bGrey)
        rgbColor = QColor(180,180,180);

    // Check if need update
    if((pQtwiItem->text(iCol) == strText)
            && (pQtwiItem->textColor(iCol) == rgbColor))
    {
        if((strTooltip.isEmpty() && pQtwiItem->toolTip(iCol) == strText)
                || (!strTooltip.isEmpty() && pQtwiItem->toolTip(iCol) == strTooltip))
            return;
    }


    pQtwiItem->setText(iCol,strText);

    // Add tooltip
    pQtwiItem->setToolTip(iCol,strTooltip);

    // Check if Read-Only mode
    if(bReadOnly)
        pQtwiItem->setFlags(pQtwiItem->flags() & ~Qt::ItemIsEditable);
    else
        pQtwiItem->setFlags(pQtwiItem->flags() | Qt::ItemIsEditable);

    // Check if Alarm color (orange) to use as background color
    pQtwiItem->setTextColor(iCol,rgbColor);
}

///////////////////////////////////////////////////////////
// Admin Server
///////////////////////////////////////////////////////////

QStringList AdminGui::OpenDirAccessGui()
{
    QStringList lModif;
    GS::DAPlugin::DirAccessBase* lDirAccess = GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin();
    if(!lDirAccess)
        return lModif;

    lDirAccess->YmAdbV2SupportClearLastUsersChanges();
    if(!lDirAccess->OpenAdministrationUi())
    {
        // Error during GUI
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Users/Groups Manager returns "+lDirAccess->GetLastError())
              .toLatin1().constData());
    }
    lModif = lDirAccess->YmAdbV2SupportGetLastUsersChanges();
    lDirAccess->YmAdbV2SupportClearLastUsersChanges();

    return lModif;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void AdminGui::OnManageUsersGroups()
{
    GS::DAPlugin::DirAccessBase* lDirAccess = GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin();
    if (lDirAccess)
    {
        // TODO check if user is admin
        // button Add User must be disabled is not connected as ADMIN
        // Open the Users/Groups manager
        QStringList lModif = OpenDirAccessGui();

        if(lModif.isEmpty())
            return;

        GS::DAPlugin::UsersBase * users =  lDirAccess->GetUsers();
        GS::DAPlugin::AppEntriesBase * applications = lDirAccess->GetAppEntries();
        if(!users || !applications)
            return;
        // Need to update ym_users table for V7.0 compatibility
        // Parse all the users list to have the login=id association from m_mapUsers
        QMap<QString,int> lMapLoginId; // for update from ym_users
        AdminUser* lUser = NULL;
        foreach(lUser, GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.values())
            lMapLoginId[lUser->m_strLogin.toLower()] = lUser->m_nUserId;

        // to parse all update for one user
        QStringList lLogins;  // to update from DA
        foreach(const QString& modif, lModif)
        {
            if(lLogins.contains(modif.section("|",1,1)))
                continue;
            lLogins << modif.section("|",1,1);
        }

        lUser = NULL;
        foreach(const QString& login, lLogins)
        {
            // Check if login exists
            if(lMapLoginId.contains(login))
                lUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[lMapLoginId[login]];
            else
            {
                // create user
                lUser = new AdminUser;
                lUser->m_nUserId = 0;
                lUser->m_strLogin = login;
                lUser->m_strPwd.clear();
                lUser->m_nProfileId = 0;
                lUser->m_nType = 1;
                lUser->m_nGroupId = 0;
            }
            foreach(const QString& modif, lModif)
            {
                if(login != modif.section("|",1,1))
                    continue;
                if(modif.section("|",3,3) == "password") lUser->m_strPwd = modif.section("|",5,5);
                if(modif.section("|",3,3) == "email") lUser->m_strEmail = modif.section("|",5,5);
                if(modif.section("|",3,3) == "name") lUser->m_strName = modif.section("|",5,5);
                if(modif.section("|",3,3) == "creation_date") lUser->m_clCreationDate = QDateTime::fromString(modif.section("|",5,5),"yyyy-MM-dd HH:mm:ss");
            }
            // Save modification
            GS::Gex::Engine::GetInstance().GetAdminEngine().SaveUser(lUser);
        }

        GS::Gex::Engine::GetInstance().GetAdminEngine().LoadUsersList();
        ShowUserList();

        return;
    }

    QMessageBox::warning(this,
                         QCoreApplication::applicationName() + " user properties",
                         "Directory Access Plugin not loaded");
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void AdminGui::OnEditUser(bool ForAll)
{
    bool bEnabled = true;
    QStringList lstUsersSelected;
    if(ForAll)
    {
        foreach(const int& UserId, GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.keys())
        {
            lstUsersSelected += QString::number(UserId);
            bEnabled &= GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToModify(GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[UserId]);
        }
    }
    else
    {
        for(int iRow=0 ; iRow<tableWidgetUser->rowCount(); ++iRow)
        {
            if(tableWidgetUser->isRowHidden(iRow))
                continue;
            if(tableWidgetUser->item(iRow,0)->isSelected()
                    && (tableWidgetUser->item(iRow,0)->text().toInt() > 0)
                    && GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.contains(tableWidgetUser->item(iRow,0)->text().toInt()))
            {
                lstUsersSelected += tableWidgetUser->item(iRow,0)->text();
                bEnabled &= GS::Gex::Engine::GetInstance().GetAdminEngine().IsAllowedToModify(GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[tableWidgetUser->item(iRow,0)->text().toInt()]);
            }
        }
        if(lstUsersSelected.isEmpty()
                && tableWidgetUser->rowCount() == 1)
            lstUsersSelected += tableWidgetUser->item(0,0)->text();
    }

    // Check if found a selection (the 1st one)
    if(lstUsersSelected.count() == 0)
    {
        // Get the current user connected
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
        {
            lstUsersSelected += QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nUserId);
            bEnabled = true;
        }
        else
            return;
    }

    AdminUserLogin loginBox(pGexMainWindow->centralWidget());

    // If ReadOnly mode
    AdminUserLogin::EnabledFieldItem(loginBox.children(), bEnabled);

    loginBox.ShowPageUserOptions(lstUsersSelected);

    m_bYieldManItemEdited = true;
    loginBox.exec();
    m_bYieldManItemEdited = false;

}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void AdminGui::OnDeleteUser()
{
    QTableWidgetItem *ptItem = NULL;
    ptItem = tableWidgetUser->item(tableWidgetUser->currentRow(),0);

    // Check if found a selection (the 1st one)
    if(ptItem == NULL)
        return;

    // Admin root can not be deleted
    if(ptItem->text().toInt() == 1)
        return;

    int nUserId = ptItem->text().toInt();

    if(nUserId < 1)
        return;

    AdminUser *pUser;

    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.contains(nUserId))
        return;

    pUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[nUserId];

    if(pUser == NULL)
        return;

    // Check if user is the owner of some monitoring tasks
    // Display message to confirm the delete and specify if some tasks while be deleted
    if(QMessageBox::information(this,"Delete user " + pUser->m_strName,
                                "Do you want to delete this user?",
                                "Yes","Not now")
            != 0)
        return;


}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
void AdminGui::OnResetPasswordUser()
{
    QTableWidgetItem *ptItem = NULL;
    ptItem = tableWidgetUser->item(tableWidgetUser->currentRow(),0);

    // Check if found a selection (the 1st one)
    if(ptItem == NULL)
        return;

    int nUserId = ptItem->text().toInt();

    if(nUserId < 1)
        return;

    AdminUser *pUser;

    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.contains(nUserId))
        return;

    pUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[nUserId];

    if(pUser == NULL)
        return;

    pUser->m_strPwd = "";

    GS::Gex::Engine::GetInstance().GetAdminEngine().SaveUser(pUser);
}
///////////////////////////////////////////////////////////
// Edit an existing User entry
///////////////////////////////////////////////////////////
void AdminGui::OnSelectUser(QTableWidgetItem* /*item*/)
{
}

///////////////////////////////////////////////////////////
// NON-GUI Function to decide to attached or detach the window
///////////////////////////////////////////////////////////
void AdminGui::OnDetachWindow(bool bDetachWindow)
{
    bool lDetachWindow = bDetachWindow;

    if(GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden())
        lDetachWindow = false;

    if(lDetachWindow)
    {
        // Check if already display
        if (mWindowDetached)
            return;
        if (!mDisplayWindowOnError)
            return;
    }
    else
    {
        mDisplayWindowOnError = false;
        // Check if already display
        if(!mWindowDetached)
            return;
    }


    if(lDetachWindow)
    {
        mDisplayWindowOnError = false;

        QIcon icon5;
        buttonReloadList->setIcon(icon5);
#ifndef QT_NO_TOOLTIP
        buttonReloadList->setToolTip(QCoreApplication::translate("db_admin_basedialog", "Close window", 0, QCoreApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonReloadList->setText(QString("Close"));

        setWindowTitle("Database Administration");

        // Hide other tabs
        // remove all tab except the first
        while(tabWidgetAdminGui->count() > 0)
            tabWidgetAdminGui->removeTab(tabWidgetAdminGui->count()-1);

        // Add Database Admin
        tabWidgetAdminGui->addTab(tabDatabases,QIcon(QPixmap(QString::fromUtf8(":/gex/icons/database-display.png"))),"Databases");

        pGexMainWindow->pScrollArea->layout()->removeWidget(this);
        setParent(NULL, Qt::Dialog | Qt::CustomizeWindowHint);
        move(QPoint(100,100));
        show();

        // No Minimum width
        setMinimumWidth(10);

        // Make sure the HTML page in Examinator's window remains visible
        pGexMainWindow->ShowHtmlBrowser();
        buttonReloadList->setEnabled(true);
        mWindowDetached = true;
    }
    else
    {
        // remove all tab
        while(tabWidgetAdminGui->count() > 0)
            tabWidgetAdminGui->removeTab(tabWidgetAdminGui->count()-1);

        // Add Database Admin
        tabWidgetAdminGui->addTab(tabDatabases,QIcon(QPixmap(QString::fromUtf8(":/gex/icons/database-display.png"))),"Databases");

        // Add MoScheduler
        if(pGexMainWindow->pWizardSchedulerGui)
            tabWidgetAdminGui->addTab(pGexMainWindow->pWizardSchedulerGui,QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_scheduler.png"))),"Tasks Scheduler");

        // Add Insertion Log files
        if(pGexMainWindow->mMonitoringWidget)
            tabWidgetAdminGui->addTab(pGexMainWindow->mMonitoringWidget,
                                      QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_logs.png"))),QString("Admin Logs"));

        QIcon icon5;
        icon5.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/refresh.png")), QIcon::Normal, QIcon::Off);
        buttonReloadList->setIcon(icon5);
        buttonReloadList->setIconSize(QSize(32, 32));
#ifndef QT_NO_TOOLTIP
        buttonReloadList->setToolTip(
                        QCoreApplication::translate("db_admin_basedialog", "Reload the Databases list", 0, QCoreApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonReloadList->setText(QString());
        // Re-attacch dialog box to Examinator's scroll view Widget
        pGexMainWindow->pScrollArea->layout()->addWidget(this);

        // Minimum width is 720 pixels
        setMinimumWidth(720);

        mDisplayWindowOnError = false;

        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            buttonReloadList->setEnabled(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped());
        mWindowDetached = false;
    }
}

///////////////////////////////////////////////////////////
// Switch between the Gex DB list and the charac Session list
///////////////////////////////////////////////////////////
void AdminGui::OnSwitchToCharac()
{
    stackedWidgetDbList->currentIndex() == TAB_ENTERPRISE_INDEX ? stackedWidgetDbList->setCurrentIndex(TAB_LOCAL_INDEX):stackedWidgetDbList->setCurrentIndex(TAB_ENTERPRISE_INDEX);
}

//////////////////////////////////////////////////////////////////////
void AdminGui::ConnectionErrorDialog(QString &strRootCause, bool bCriticalError)
{
    QString strError;
    GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);

    QString strTopMessage;
    QString strBottomMessage;
    QString strHtmlText;
    QString strApplicationName = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
    // Get the short name
    if(strApplicationName.count("-") > 1)
        strApplicationName = strApplicationName.section("-",0,1);
    else
        strApplicationName = strApplicationName.section("-",0,0);

    if(!strError.isEmpty())
        strError+="\n";
    strError+=strRootCause.section("\n",1);
    if(strError.endsWith("\n"))
        strError = strError.left(strError.length()-2);

    strTopMessage = "<center><h2>Yield-Man Administration Server</h2>\n";
    strTopMessage+= "--------------------------------------------------------------------------------------------------</center><br>\n";
    strTopMessage+= "<center><b><img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\">";
    strTopMessage+= "The Yield-Man Administration Server"+strRootCause.section("\n",0,0)+"";
    strTopMessage+= "<img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\"></b></center><br>\n";

    if(strRootCause.contains("was aborted"))
    {
        strHtmlText+= "<h2>Yield-Man Administration Server"+strRootCause.section("\n",0,0)+"</h2>\n";
        strHtmlText+= "<ul>";
        strHtmlText+= "  <li> "+strError.replace("\n","<li>")+"\n";
        strHtmlText+= "</ul>\n";

        strHtmlText+= "<b>\n";
        strHtmlText+= "Contact Quantix support at "+QString(GEX_EMAIL_SUPPORT)+" for more details.<br>\n";
        strHtmlText+= "</b>\n";
    }
    else
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            strHtmlText+= "<h2>"+strApplicationName+" information</h2>\n";
            strHtmlText+= "<ul>";
            strHtmlText+= " <li> Version: "+GS::Gex::Engine::GetInstance().Get("AppFullName").toString()+"\n";
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_nNodeId>0)
                strHtmlText+= " <li> NodeId:"+QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().m_nNodeId)+"\n";
            strHtmlText+= " <li> ServerProfile: "
                    +GS::Gex::Engine::GetInstance().Get("UserFolder").toString()+"\n";
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().m_strAllCpu.isEmpty())
                strHtmlText+= " <li> CPU: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_strAllCpu+"\n";
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().m_strHostId.isEmpty())
                strHtmlText+= " <li> HostId: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_strHostId+"\n";
            strHtmlText+= "</ul>\n";
        }
        strHtmlText+= " <h2>Yield-Man Administration Server information</h2>\n";
        strHtmlText+= "<ul>";
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsConnected())
            strHtmlText+= " <li> Version:"+GS::Gex::Engine::GetInstance().GetAdminEngine().GetServerVersionName(true)+"\n";
        strHtmlText+= " <li> HostName: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strHost_Name+"\n";
        strHtmlText+= " <li> IP: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strHost_IP+"\n";
        strHtmlText+= " <li> Port: "+QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_uiPort)+"\n";
        strHtmlText+= " <li> Driver: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strDriver+"\n";
        strHtmlText+= " <li> Database: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strSchemaName+"\n";
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsOracleDB())
            strHtmlText+= " <li> SID: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strDatabaseName+"\n";
        strHtmlText+= " <li> Password: "+QString().rightJustified(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strPassword_Admin.length(),'*')+"\n";
        strHtmlText+= "</ul>\n";

        strHtmlText+= "<h2>Yield-Man Administration Server"+strRootCause.section("\n",0,0)+"</h2>\n";
        strHtmlText+= "<ul>";
        strHtmlText+= "  <li> "+strError.replace("\n","<li>")+"\n";
        strHtmlText+= "</ul>\n";

        strHtmlText+= "<b>\n";
        strHtmlText+= "Ask to your administrator to log on Yield-Man to enable the necessary changes.<br>\n";
        strHtmlText+= "Contact Quantix support at "+QString(GEX_EMAIL_SUPPORT)+" for more details.<br>\n";
        strHtmlText+= "</b>\n";

    }


    if(bCriticalError)
        strBottomMessage+= "<h4>"+strApplicationName+" will now exit.</h4>";
    else
        strBottomMessage+= "<h4>"+strApplicationName+" will start without Yield-Man Administration Services.</h4>";


    QDialog clDialogBox;
    QVBoxLayout clVBoxLayout;
    QHBoxLayout clHBoxLayout;

    QLabel clTopLabel;
    QTextEdit clHtmlText;
    QLabel clBottomLabel;
    QPushButton clPwdButton;
    QPushButton clOkButton;

    clHtmlText.setReadOnly(true);
    clHtmlText.setAcceptDrops(false);
    clDialogBox.setLayout(&clVBoxLayout);
    clDialogBox.setWindowTitle("Yield-Man Administration Server");
    clTopLabel.setText(strTopMessage);
    clVBoxLayout.addWidget(&clTopLabel);
    clHtmlText.setText(strHtmlText);
    clVBoxLayout.addWidget(&clHtmlText);
    clBottomLabel.setText(strBottomMessage);
    clVBoxLayout.addWidget(&clBottomLabel);

    if(bCriticalError)
        clOkButton.setText("Exit");
    else
        clOkButton.setText("Start");

    clHBoxLayout.addWidget(&clOkButton);
    clVBoxLayout.addLayout(&clHBoxLayout);
    QObject::connect(&clPwdButton, SIGNAL(clicked()), &clDialogBox, SLOT(accept()));
    QObject::connect(&clOkButton, SIGNAL(clicked()), &clDialogBox, SLOT(close()));

    clDialogBox.setModal(true);

    int nHight = 600;
    int nLine = strHtmlText.count("<br>",Qt::CaseInsensitive);
    nLine += strHtmlText.count("<h",Qt::CaseInsensitive)*3;
    nLine += strHtmlText.count("<li",Qt::CaseInsensitive);
    nLine += strHtmlText.count("<dd",Qt::CaseInsensitive);
    nHight = nLine*25;
    if(nHight > 600)
        nHight = 600;

    clDialogBox.setMinimumSize(500,nHight);

    clDialogBox.adjustSize();
    clDialogBox.exec();
}

//////////////////////////////////////////////////////////////////////
void AdminGui::SummaryDialog()
{
    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));

    QString strAdminName = "ym_admin_db";
    QString strAdminId = "1";
    QString strPwd = "ymadmin";
    bool    bNewInstall = false;

    strQuery = "SELECT login, pwd, creation_date, user_id FROM ym_users WHERE lower(login)='root' OR lower(login)='admin'";
    if(!clQuery.exec(strQuery))
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().SetLastError(GS::Gex::AdminEngine::eDB_Query,
                                                                     strQuery.left(1024),clQuery.lastError().text());
        return;
    }
    if(clQuery.first())
    {
        strAdminName = clQuery.value(0).toString();
        strAdminId = clQuery.value(3).toString();
        bNewInstall = clQuery.isNull(2) || clQuery.value(2).toString().isEmpty();
        if(clQuery.value(2).toDateTime().addDays(1) > GS::Gex::Engine::GetInstance().GetServerDateTime())
            bNewInstall = true;

#ifdef QT_DEBUG
        // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
        // DO NOT ADD THIS GSLOG in RELEASE VERSION
        //GSLOG(SYSLOG_SEV_NOTICE, QString("Decrypting password for ym_users.login=%1, ym_users.user_id=1 from string: %2").arg(strAdminName).arg(clQuery.value(1).toString()));
#endif
        GexDbPlugin_Base::DecryptPassword(clQuery.value(1).toString(),strPwd);
        if(!bNewInstall)
            strPwd = QString().rightJustified(strPwd.length(),'*');
    }

    QString strTopMessage;
    QString strBottomMessage;
    QString strHtmlText;
    QString strApplicationName = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
    // Get the short name
    if(strApplicationName.count("-") > 1)
        strApplicationName = strApplicationName.section("-",0,1);
    else
        strApplicationName = strApplicationName.section("-",0,0);

    strTopMessage = "<center><h2>Yield-Man Administration Server has been successfully updated</h2>\n";
    strTopMessage+= "--------------------------------------------------------------------------------------------------</center><br>\n";
    if(bNewInstall)
        strTopMessage+= "<b>The Yield-Man Administration Server installation is now complete.</b><br>\n";
    else
        strTopMessage+= "<b>The Yield-Man Administration Server update is now complete.</b><br>\n";
    strTopMessage+= "For help getting started, read the summary below.\n";

    if(bNewInstall)
    {
        strHtmlText+= "<h2> <img src=\""+QString::fromUtf8(":/gex/icons/yieldmandb_users.png")
                +"\"> Your Yield-Man Administrator Account </h2>\n";
        strHtmlText+= "<ul>\n";
        if(strPwd == "ymadmin")
        {
            strHtmlText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                    +"\"> <b>Please note your Administrator login credentials </b> <img src=\""
                    +QString::fromUtf8(":/gex/icons/warning.png")+"\">\n";
            strHtmlText+= "<ul>\n";
        }
        strHtmlText+= " <li> Yield-Man Administrator Login: <b>"+strAdminName+"</b>\n";
        if(strPwd == "ymadmin")
            strHtmlText+= " <li> Yield-Man Administrator Password: <b>"+strPwd+"</b>\n";
        else
            if(strPwd.isEmpty())
                strHtmlText+= " <li> Yield-Man Administrator Password: <img src=\""
                        +QString::fromUtf8(":/gex/icons/warning.png")
                        +"\"> <b>no password configured !</b>\n";
            else
                strHtmlText+= " <li> Yield-Man Administrator Password: "+strPwd+"\n";
        if(strPwd == "ymadmin")
            strHtmlText+= "</ul>\n";
        if((strPwd == "ymadmin") || strPwd.isEmpty())
        {
            strHtmlText+= " <br><li> On the next page, you will enter the Yield-Man Administrator password.\n";
            strHtmlText+= " <br> This is the password that the Yield-Man Administrator will use to log in to Examinator or Yield-Man application.\n";
        }
        strHtmlText+= "</ul>\n";
    }
    else
        strHtmlText+= HtmlSection("NewFeatures");

    // Databases uploaded
    if(bNewInstall)
        strHtmlText+= HtmlSection("DatabasesList|Uploaded");
    else
        strHtmlText+= HtmlSection("DatabasesList|OnlyError");

    // Tasks uploaded

    if(bNewInstall)
        strHtmlText+= HtmlSection("TasksDetail");
    else
        strHtmlText+= HtmlSection("TasksDetail|OnlyError");


    strHtmlText+= HtmlSection("YieldManDbConfig");

    if(bNewInstall && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsMySqlDB())
    {
        strHtmlText+= HtmlSection("MySqlServerConfig");
    }

    if(bNewInstall && ((strPwd == "ymadmin") || strPwd.isEmpty()))
        strBottomMessage+= "<img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                +"\"> Update your Yield-Man Administrator password.\n";
    strBottomMessage+= "<h4>Start Yield-Man Administration Server now.</h4>";

    // for debug
    // save html
    QString SaveFile = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
            +QDir::separator()+ QString("logs")+ QDir::separator()
            +QDate::currentDate().toString(Qt::ISODate)
            + QDir::separator() + "YieldManDbSummaryDialog.html";
    QFile File(SaveFile);
    File.open(QFile::ReadWrite);
    File.write("<HTML>\n");
    File.write(strHtmlText.toLatin1().constData());
    File.write("</HTML>\n");
    File.close();
    // for debug



    QDialog clDialogBox;
    QVBoxLayout clVBoxLayout;
    QHBoxLayout clHBoxLayout;

    QLabel clTopLabel;
    QTextEdit clHtmlText;
    QLabel clBottomLabel;
    QPushButton clAcceptButton;
    QPushButton clCloseButton;

    clHtmlText.setReadOnly(true);
    clHtmlText.setAcceptDrops(false);
    clDialogBox.setLayout(&clVBoxLayout);
    clDialogBox.setWindowTitle("Yield-Man Administration Server");
    clTopLabel.setText(strTopMessage);
    clVBoxLayout.addWidget(&clTopLabel);
    clHtmlText.setText(strHtmlText);
    clVBoxLayout.addWidget(&clHtmlText);
    clBottomLabel.setText(strBottomMessage);
    clVBoxLayout.addWidget(&clBottomLabel);

    bool bUpdatePwd = false;
    bool bOpenUserMng = false;
    if((strPwd == "ymadmin") || strPwd.isEmpty())
    {
        bUpdatePwd = true;
        clAcceptButton.setText("Update root password");
        clHBoxLayout.addWidget(&clAcceptButton);
        QObject::connect(&clAcceptButton, SIGNAL(clicked()), &clDialogBox, SLOT(accept()));
    }
    else if(GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin()
            && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
    {
        bOpenUserMng = true;
        clAcceptButton.setText("Open the Users/Groups Manager now");
        clHBoxLayout.addWidget(&clAcceptButton);
        QObject::connect(&clAcceptButton, SIGNAL(clicked()), &clDialogBox, SLOT(accept()));

        clCloseButton.setText("Close");
        clHBoxLayout.addWidget(&clCloseButton);
        QObject::connect(&clCloseButton, SIGNAL(clicked()), &clDialogBox, SLOT(close()));
    }
    else
    {

        clCloseButton.setText("Close");
        clHBoxLayout.addWidget(&clCloseButton);
        QObject::connect(&clCloseButton, SIGNAL(clicked()), &clDialogBox, SLOT(close()));
    }
    clVBoxLayout.addLayout(&clHBoxLayout);

    GS::Gex::Engine::GetInstance().GetAdminEngine().m_bFirstActivation = false;

    clDialogBox.setModal(true);

    int nHight = 600;
    int nLine = strHtmlText.count("<br>",Qt::CaseInsensitive);
    nLine += strHtmlText.count("<h",Qt::CaseInsensitive)*3;
    nLine += strHtmlText.count("<li",Qt::CaseInsensitive);
    nLine += strHtmlText.count("<dd",Qt::CaseInsensitive);
    nHight = nLine*25;
    if(nHight > 600)
        nHight = 600;

    clDialogBox.setMinimumSize(500,nHight);

    clDialogBox.adjustSize();
    if(clDialogBox.exec()==QDialog::Accepted)
    {
        if(bUpdatePwd || bOpenUserMng)
        {
            OnManageUsersGroups();
        }
    }
}

//////////////////////////////////////////////////////////////////////
bool AdminGui::InfoDialog(QString TopMessage, QString HtmlMessage,
                          QString BottomMessage, QString Buttons)
{

    QString strTopMessage = TopMessage;
    QString strBottomMessage = BottomMessage;
    QString strHtmlText = HtmlMessage;

    QString OkButton = Buttons.section("|",0,0);
    QString RemindButton = Buttons.section("|",1,1);

    QDialog clDialogBox;
    QVBoxLayout clVBoxLayout;
    QHBoxLayout clHBoxLayout;

    QLabel clTopLabel;
    QTextEdit clHtmlText;
    QLabel clBottomLabel;
    QPushButton clRemindButton;
    QPushButton clOkButton;

    clHtmlText.setReadOnly(true);
    clHtmlText.setAcceptDrops(false);
    clDialogBox.setLayout(&clVBoxLayout);
    clDialogBox.setWindowTitle("Yield-Man Administration Server");
    clTopLabel.setText(strTopMessage);
    clVBoxLayout.addWidget(&clTopLabel);
    clHtmlText.setText(strHtmlText);
    clVBoxLayout.addWidget(&clHtmlText);
    clBottomLabel.setText(strBottomMessage);
    clVBoxLayout.addWidget(&clBottomLabel);

    if(!OkButton.isEmpty())
    {
        clOkButton.setText(OkButton);
        clHBoxLayout.addWidget(&clOkButton);
    }
    if(!RemindButton.isEmpty())
    {
        clRemindButton.setText(RemindButton);
        clHBoxLayout.addWidget(&clRemindButton);
    }
    clVBoxLayout.addLayout(&clHBoxLayout);
    if(!RemindButton.isEmpty())
        QObject::connect(&clRemindButton, SIGNAL(clicked()), &clDialogBox, SLOT(reject()));
    if(!OkButton.isEmpty())
        QObject::connect(&clOkButton, SIGNAL(clicked()), &clDialogBox, SLOT(accept()));

    clDialogBox.setModal(true);

    int nHight = 600;
    int nLine = strHtmlText.count("<br>",Qt::CaseInsensitive);
    nLine += strHtmlText.count("<h",Qt::CaseInsensitive)*3;
    nLine += strHtmlText.count("<li",Qt::CaseInsensitive);
    nLine += strHtmlText.count("<dd",Qt::CaseInsensitive);
    nHight = nLine*25;
    if(nHight > 600)
        nHight = 600;

    clDialogBox.setMinimumSize(500,nHight);
    clDialogBox.adjustSize();
    if(clDialogBox.exec() != QDialog::Accepted)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////
void AdminGui::WelcomeDialog(QString Options, bool &Result)
{

    QString strTopMessage;
    QString strBottomMessage;
    QString strHtmlPresentationText;
    QString strHtmlServerCheckText;
    QString strHtmlConnectionCheckText;
    QString strHtmlOptionText;
    QString RemindButton;
    QString OkButton;

    bool bAskForAdminUserPwd = false;
    Result = true;

    // To know if have an update to do
    // check also the build
    QString strValue;
    GS::Gex::Engine::GetInstance().GetAdminEngine().GetSettingsValue("DB_BUILD_NB",strValue);
    m_nFromServerBuild = strValue.toInt();

    // when connector is already configured (for update to load-balancing)
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector
            && GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsConnected())
    {
        // This is a least a minor ym_admin_db update
        // This means that all YM/PAT must be upgraded
        // First
        // Reject the update if some other YM/PAT are running or connected
        // Ask explicitely to shutdown all YM/PAT
        // Second
        // Then warning on the fact that all Examinator-PRO users will be automatically disconnected if continue

        // Check Nodes connection
        strHtmlConnectionCheckText+= HtmlSection("NodesList|Connected|NotMe");
        // strHtmlOptionText+= HtmlSection("ApplicationsConnected");
        strHtmlServerCheckText += HtmlSection("MySqlServerConfig");
    }


    // general presentation
    strHtmlPresentationText+= HtmlSection("Presentation");

    if(Options == "install")
    {
        strTopMessage = "<center><h2>Welcome to Yield-Man Administration Server</h2>\n";
        strTopMessage+= "--------------------------------------------------------------------------------------------------</center><br>\n";
        strTopMessage+= "You are about to create a database holding all Yield-Man tasks, database links, ";
        strTopMessage+= "and profiles (Yield-Man Administration database).<br>\n";
        strTopMessage+= "<center><b> <img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\"> ";
        strTopMessage+= "You need to have the MySQL or Oracle root access and credentials. ";
        strTopMessage+= "<img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\"> </b></center><br>\n";
        // Case 5787 - Make YmAdminDb server mandatory for RDB
        strHtmlOptionText+= "<h2>Do you want to skip the Yield-Man Administration Server installation?</h2>\n";
        strHtmlOptionText+= "If you do not need any RDB tools or SQL database management, \n";
        strHtmlOptionText+= "you may skip the Yield-Man Administration Server installation.\n";
        strHtmlOptionText+= "<dl>\n";
        strHtmlOptionText+= " <dt> <b>Thus the Yield-Man version will not support:</b>\n";
        strHtmlOptionText+="   <dd> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                +"\"> RDB tools\n";
        strHtmlOptionText+="   <dd> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                +"\"> Production TDR Databases\n";
        strHtmlOptionText+="   <dd> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                +"\"> Centralization tools\n";
        strHtmlOptionText+="   <dd> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                +"\"> Users and Profiles managment\n";
        strHtmlOptionText+= " <br>\n";
        strHtmlOptionText+= " <dt> <b>You can display this Welcome dialog at any time:</b>\n";
        strHtmlOptionText+="   <dd>- with the arg -W\n";
        strHtmlOptionText+="   <dd>- to install Yield-Man Administration Server\n";
        strHtmlOptionText+= "</dl>\n\n";
        strBottomMessage+= "<h4>Are you ready for Yield-Man Administration Server ?</h4>";
        OkButton = "Install now";
        RemindButton = "Skip";
    }
    else if(Options == "update")
    {
        strTopMessage = "<center><h2>Yield-Man Administration Server Update</h2>\n";
        strTopMessage+= "--------------------------------------------------------------------------------------------------</center><br>\n";
        strTopMessage+= "You are about to update your Yield-Man Administration database ";
        strTopMessage+= "to activate the new features for Yield-Man Scalability.<br>\n";
        strHtmlOptionText+= HtmlSection("Version");
        strHtmlOptionText+= "<h2>Caution</h2>\n";
        strHtmlOptionText+= "During the update, all Quantix Applications will be disconnected.<br>\n";
        strHtmlOptionText+= "Make sure that all users have been warned.<br>\n";
        strHtmlOptionText+= "If you need more information about Yield-Man Administration tools, ";
        strHtmlOptionText+= " you may abort the Yield-Man Administration Server update and ";
        strHtmlOptionText+= "this application will exit.\n";

        QString lCompatibleInfo;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsServerCompatible(false,lCompatibleInfo))
            RemindButton = "Ignore";
        else
            RemindButton = "Abort";
        strBottomMessage+= "<h4>Are you ready to activate the new features for Yield-Man Scalability?</h4>\n";
        OkButton = "Update now";

        bAskForAdminUserPwd = true;
    }
    else
    {
        Result = false;
        return;
    }

    if(!strHtmlConnectionCheckText.isEmpty())
    {
        // MySql server is not configured as we want
        // force to fail

        QString strApplicationName = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
        // Get the short name
        if(strApplicationName.count("-") > 1)
            strApplicationName = strApplicationName.section("-",0,1);
        else
            strApplicationName = strApplicationName.section("-",0,0);

        strHtmlConnectionCheckText+= "<center><b> <img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\"> ";
        strHtmlConnectionCheckText+= " Close all Yield-Man or Pat-Man applications ";
        strHtmlConnectionCheckText+= "<img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\"> </b></center><br>\n";

        strBottomMessage = "<h4>You must close all Yield-Man and Pat-Man applications before to update the Quantix Server.</h4>";
        strHtmlPresentationText = strHtmlPresentationText.section("<BR>",0,0);
        OkButton = "";
        RemindButton = "Abort";
    }
    if(!strHtmlServerCheckText.isEmpty())
    {
        // MySql server is not configured as we want
        // force to fail

        QString strApplicationName = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
        // Get the short name
        if(strApplicationName.count("-") > 1)
            strApplicationName = strApplicationName.section("-",0,1);
        else
            strApplicationName = strApplicationName.section("-",0,0);

        strHtmlServerCheckText+= "<center><b> <img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\"> ";
        strHtmlServerCheckText+= " Your MySql Server need to be updated. ";
        strHtmlServerCheckText+= "<img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\"> </b></center><br>\n";

        strBottomMessage = "<h4>You must update your MySql Server.</h4>";
        strHtmlPresentationText = strHtmlPresentationText.section("<BR>",0,0);
        OkButton = "";
        RemindButton = "Abort";
    }

    QDialog clDialogBox;
    QVBoxLayout clVBoxLayout;
    QHBoxLayout clHBoxLayout;

    QLabel clTopLabel;
    QTextEdit clHtmlText;
    QLabel clBottomLabel;
    QPushButton clRemindButton;
    QPushButton clOkButton;

    QString strHtmlText;
    strHtmlText = strHtmlPresentationText;
    strHtmlText+= strHtmlServerCheckText;
    strHtmlText+= strHtmlConnectionCheckText;
    strHtmlText+= strHtmlOptionText;
    strHtmlText+= "<DL><DT><b>For more details:</b> \n";
    strHtmlText+= "<DD>- Go to Quantix Support at http://support.galaxysemi.com\n";
    strHtmlText+= "<DD>- Contact Quantix support at "+QString(GEX_EMAIL_SUPPORT)+".\n";
    strHtmlText+= "</DL>\n";

    // for debug
    // save html
    QString SaveFile = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
            +QDir::separator()+ QString("logs")+ QDir::separator()
            +QDate::currentDate().toString(Qt::ISODate)
            + QDir::separator() + "YieldManDbWelcomeDialog.html";
    QFile File(SaveFile);
    File.open(QFile::ReadWrite);
    File.write("<HTML>\n");
    File.write(strHtmlText.toLatin1().constData());
    File.write("</HTML>\n");
    File.close();
    // for debug


    clHtmlText.setReadOnly(true);
    clHtmlText.setAcceptDrops(false);
    clDialogBox.setLayout(&clVBoxLayout);
    clDialogBox.setWindowTitle("Yield-Man Administration Server");
    clTopLabel.setText(strTopMessage);
    clVBoxLayout.addWidget(&clTopLabel);
    clHtmlText.setText(strHtmlText);
    clVBoxLayout.addWidget(&clHtmlText);
    clBottomLabel.setText(strBottomMessage);
    clVBoxLayout.addWidget(&clBottomLabel);

    if(!OkButton.isEmpty())
    {
        clOkButton.setText(OkButton);
        clHBoxLayout.addWidget(&clOkButton);
    }
    if(!RemindButton.isEmpty())
    {
        clRemindButton.setText(RemindButton);
        clHBoxLayout.addWidget(&clRemindButton);
    }
    clVBoxLayout.addLayout(&clHBoxLayout);
    if(!RemindButton.isEmpty())
        QObject::connect(&clRemindButton, SIGNAL(clicked()), &clDialogBox, SLOT(reject()));
    if(!OkButton.isEmpty())
        QObject::connect(&clOkButton, SIGNAL(clicked()), &clDialogBox, SLOT(accept()));

    clDialogBox.setModal(true);

    int nHight = 600;
    int nLine = strHtmlText.count("<br>",Qt::CaseInsensitive);
    nLine += strHtmlText.count("<h",Qt::CaseInsensitive)*3;
    nLine += strHtmlText.count("<li",Qt::CaseInsensitive);
    nLine += strHtmlText.count("<dt",Qt::CaseInsensitive);
    nLine += strHtmlText.count("<dd",Qt::CaseInsensitive);
    nHight = nLine*25;
    if(nHight > 600)
        nHight = 600;

    clDialogBox.setMinimumSize(500,nHight);
    clDialogBox.adjustSize();
    if(clDialogBox.exec() != QDialog::Accepted)
    {
        Result = false;
        return;
    }

    if(bAskForAdminUserPwd)
    {
        // Check admin user pwd before to update
        if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
        {
            ConnectUserDialog(true);

            // wait until user connect
            QTime cTime,cWait;
            cTime = QTime::currentTime();
            while(m_pLoginDialog && m_pLoginDialog->isVisible())
            {
                QCoreApplication::processEvents();
                //then wait 150msec & retry.
                cWait = cTime.addMSecs(150);
                do
                {
                    cTime = QTime::currentTime();
                }
                while(cTime < cWait);
            }

            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
            {
                Result = true;
                return;
            }
        }

    }
    else
        return;

    Result = false;
}

QString AdminGui::HtmlSection(QString Type)
{
    QString        strQuery;
    QSqlQuery      clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));

    QString MySqlIniFile;
    QString MySqlLogErrorFile;
    QString HtmlCheckText;
    QString Section;
    QString Value;
    QStringList Errors;

    if(m_nFromServerBuild == 0)
    {
        QString strValue;
        GS::Gex::Engine::GetInstance().GetAdminEngine().GetSettingsValue("DB_BUILD_NB",strValue);
        m_nFromServerBuild = strValue.toInt();
    }

    if(Type.toUpper() == "APPLICATIONSCONNECTED")
    {
        QString        lQuery;
        QSqlQuery      clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));

        QString lDataBaseName = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strSchemaName;
        ///////////////////////////////////////////
        // Check if have some other application connected (except this one)
        QStringList GexConnected;

        // Check if the user isn't already connected
        // WITH MYSQL : ONLY USER CONNECTION CAN CHECK THE TOTAL CONNECTION FOR HIMSELF

        lQuery = "SELECT count(*) FROM information_schema.PROCESSLIST ";
        lQuery+= " WHERE USER='"+lDataBaseName+"'";
        if(clQuery.exec(lQuery) && clQuery.first()
                && (clQuery.value(0).toInt() > 1))
        {
            // User already connected
            lQuery = "SELECT DISTINCT HOST FROM information_schema.PROCESSLIST ";
            lQuery+= " WHERE USER='"+lDataBaseName+"' AND NOT(ID=connection_id()) ";
            clQuery.exec(lQuery);
            while(clQuery.next())
                GexConnected << clQuery.value(0).toString();
        }

        if(!GexConnected.isEmpty())
        {
            Section+= "<h2>Check if some Quantix application running:</h2>";
            Section+= "<DL>";

            // Node already connected
            Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                    +"\">  Some users are connected to the ym_admin_db database\n";
            Section+= "<br>on IP:SID "+GexConnected.join("\n<br>- on IP:SID ")+"\n";
            Section+= " <DD> This can be a Quantix application running\n";
            Section+= " <DD> This can be a SQL user connected\n";
            Section+= "</DL>\n";
        }

    }
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsMySqlDB()
            && (Type.toUpper() == "MYSQLSERVERCONFIG"))
    {
        ///////////////////////////////////////////
        // Check the MySql configuration

        HtmlCheckText+= "<h2>Check the MySql Server configuration:</h2>";
        HtmlCheckText+= "<ul>";

        // Find the MySql my.ini config file
        strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
        strQuery+= "   WHERE VARIABLE_NAME = 'BASEDIR'";
        if(clQuery.exec(strQuery) && clQuery.first())
        {
            if(QFileInfo(clQuery.value(0).toString()+"my.ini").exists())
                MySqlIniFile = clQuery.value(0).toString()+"my.ini";
            else if (QFileInfo(clQuery.value(0).toString()+"my.cnf").exists())
                MySqlIniFile = clQuery.value(0).toString()+"my.cnf";
        }
        if(MySqlIniFile.isEmpty())
        {
            strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
            strQuery+= "   WHERE VARIABLE_NAME = 'DATADIR'";
            if(clQuery.exec(strQuery) && clQuery.first())
            {
                if(QFileInfo(clQuery.value(0).toString()+"my.ini").exists())
                    MySqlIniFile = clQuery.value(0).toString()+"my.ini";
                else if (QFileInfo(clQuery.value(0).toString()+"my.cnf").exists())
                    MySqlIniFile = clQuery.value(0).toString()+"my.cnf";
            }
        }
        if(MySqlIniFile.isEmpty())
            MySqlIniFile = "BASEDIR/my.ini or BASEDIR/my.cnf";

        // Get the MySql LogError
        strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
        strQuery+= "   WHERE VARIABLE_NAME = 'LOG_ERROR'";
        if(clQuery.exec(strQuery) && clQuery.first())
            MySqlLogErrorFile = clQuery.value(0).toString();

        // Check if have INNODB engine
        strQuery = "SELECT support FROM information_schema.engines";
        strQuery+= "   WHERE UPPER(engine) = 'INNODB'";
        if(!clQuery.exec(strQuery))
        {
            Value = "Error executing SQL query.\n";
            Value+= "QUERY=" + strQuery + "\n";
            Value+= "ERROR=" + clQuery.lastError().text();
        }
        else
            Value = "NO VALUE";
        if(clQuery.first())
            Value = clQuery.value(0).toString();
        // YES
        if(!Value.startsWith("YES",Qt::CaseInsensitive)
                && !Value.startsWith("DEFAULT",Qt::CaseInsensitive))
        {
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                    +"\">  - MYSQL GLOBAL_VARIABLES 'HAVE_INNODB' = "+Value;
            Value = "*************************\n";
            Value+= "o UNSUPPORTED MYSQL INNODB ENGINE\n";
            Value+= "o MYSQL GLOBAL_VARIABLES = 'HAVE_INNODB' disabled\n";
            Value+= "*************************\n";
            Value+= " - NEED MYSQL INNODB ENGINE\n";
            Value+= "o Check your MySql config file:\n";
            Value+= " - "+MySqlIniFile+"\n";
            Value+= " - If skip-innodb is uncommented, try commenting it out and restarting mysqld.\n";
            Value+= "o Check your MySql error log file:\n";
            Value+= " - "+MySqlLogErrorFile+"\n";
            Value+= " - InnoDB would be disabled when it fail to start, please check your error log.";

            Errors.append(Value);
        }
        else
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/enable.png")
                    +"\">  - MYSQL GLOBAL_VARIABLES 'HAVE_INNODB' = "+Value;

        // Check if have INNODB_FILE_PER_TABLE
        strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
        strQuery+= "   WHERE VARIABLE_NAME = 'INNODB_FILE_PER_TABLE'";
        if(!clQuery.exec(strQuery))
        {
            Value = "Error executing SQL query.\n";
            Value+= "QUERY=" + strQuery + "\n";
            Value+= "ERROR=" + clQuery.lastError().text();
        }
        else
            Value = "NO VALUE";
        if(clQuery.first())
            Value = clQuery.value(0).toString();
        // ON
        if(!Value.startsWith("ON",Qt::CaseInsensitive))
        {
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                    +"\"> - MYSQL GLOBAL_VARIABLES 'INNODB_FILE_PER_TABLE' = "+Value;
            Value = "*************************\n";
            Value+= "o UNSUPPORTED MYSQL DATA FILE MANAGMENT\n";
            Value+= "o MYSQL GLOBAL_VARIABLES = 'INNODB_FILE_PER_TABLE' disabled\n";
            Value+= "*************************\n";
            Value+= " - This option must be enabled in order to create the Quantix DB\n";
            Value+= "o Check your MySql config file:\n";
            Value+= " - "+MySqlIniFile+"\n";
            Value+= " - add a line to the [mysqld] section.\n";
            Value+= "   [mysqld]\n";
            Value+= "   innodb_file_per_table\n";
            Value+= "o Check your MySql error log file:\n";
            Value+= " - "+MySqlLogErrorFile+"\n";
            Value+= " - InnoDB would be disabled when it fail to start, please check your error log.\n";

            Errors.append(Value);
        }
        else
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/enable.png")
                    +"\"> - MYSQL GLOBAL_VARIABLES 'INNODB_FILE_PER_TABLE' = "+Value;

        // Check if have partitioning
        strQuery = "SELECT plugin_status FROM information_schema.plugins";
        strQuery+= "   WHERE UPPER(plugin_name) = 'PARTITION'";
        if(!clQuery.exec(strQuery))
        {
            Value = "Error executing SQL query.\n";
            Value+= "QUERY=" + strQuery + "\n";
            Value+= "ERROR=" + clQuery.lastError().text();
        }
        else
            Value = "NO VALUE";
        if(clQuery.first())
            Value = clQuery.value(0).toString();
        // YES
        if(!Value.startsWith("YES",Qt::CaseInsensitive)
                && !Value.startsWith("ACTIVE",Qt::CaseInsensitive))
        {
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                    +"\"> - MYSQL GLOBAL_VARIABLES 'HAVE_PARTITIONING' = "+Value;
            Value = "*************************\n";
            Value+= "o UNSUPPORTED MYSQL PARTITION MANAGMENT\n";
            Value+= "o MYSQL GLOBAL_VARIABLES = 'HAVE_PARTITIONING' disabled\n";
            Value+= "*************************\n";
            Value+= " - This option must be enabled in order to create the Quantix DB\n";
            Value+= "o Check your MySql error log file:\n";
            Value+= " - "+MySqlLogErrorFile+"\n";
            Value+= " - your version of MySQL was not built with partitioning support.";

            Errors.append(Value);
        }
        else
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/enable.png")
                    +"\"> - MYSQL GLOBAL_VARIABLES 'HAVE_PARTITIONING' = "+Value;

        // Check if have EVENT_SCHEDULER option
        // min sql version is v5.1.6 for this feature
        strQuery = "SELECT VARIABLE_VALUE FROM information_schema.GLOBAL_VARIABLES ";
        strQuery+= "   WHERE VARIABLE_NAME = 'EVENT_SCHEDULER'";
        if(!clQuery.exec(strQuery))
        {
            Value = "Error executing SQL query.\n";
            Value+= "QUERY=" + strQuery + "\n";
            Value+= "ERROR=" + clQuery.lastError().text();
        }
        else
            Value = "NO VALUE";
        if(clQuery.first())
            Value = clQuery.value(0).toString();
        // ON
        if((!Value.startsWith("ON",Qt::CaseInsensitive))
                && (!Value.startsWith("YES",Qt::CaseInsensitive))
                && (!Value.startsWith("1",Qt::CaseInsensitive))
                && (!Value.startsWith("ENAB",Qt::CaseInsensitive)))
        {
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                    +"\"> - MYSQL GLOBAL_VARIABLES 'EVENT_SCHEDULER' = "+Value;
            Value = "*************************\n";
            Value+= "o UNSUPPORTED MYSQL EVENT SCHEDULER OPTION\n";
            Value+= "o MYSQL GLOBAL_VARIABLES = 'EVENT_SCHEDULER' disabled\n";
            Value+= "*************************\n";
            Value+= " - NEED MYSQL EVENT_SCHEDULER OPTION\n";
            Value+= "o Check your MySql config file:\n";
            Value+= " - "+MySqlIniFile+"\n";
            Value+= " - add a line to the [mysqld] section.\n";
            Value+= "   [mysqld]\n";
            Value+= "   event_scheduler = 1\n";
            Value+= "o Check your MySql error log file:\n";
            Value+= " - "+MySqlLogErrorFile+"\n";
            Value+= " - EVENT_SCHEDULER would be disabled when it fail to start, please check your error log.\n";

            Errors.append(Value);
        }
        else
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/enable.png")
                    +"\"> - MYSQL GLOBAL_VARIABLES 'EVENT_SCHEDULER' = "+Value;

        // Then check if the ym_admin_db user have the EVENT privilege
        strQuery = "SHOW GRANTS FOR ym_admin_db@localhost";
        QStringList lPrivileges;
        Value = "NO VALUE";
        if(!clQuery.exec(strQuery))
        {
            strQuery = "SHOW GRANTS FOR ym_admin_db@'%'";
            if(!clQuery.exec(strQuery))
            {
        strQuery = "SHOW GRANTS FOR ym_admin_db";
        if(!clQuery.exec(strQuery))
        {
            Value = "Error executing SQL query.\n";
            Value+= "QUERY=" + strQuery + "\n";
            Value+= "ERROR=" + clQuery.lastError().text();
        }
            }
        }
        while(clQuery.next())
        {
            QString lPrivilegesString = clQuery.value(0).toString().toUpper().remove("GRANT ").section(" ON ",0,0);
            foreach(const QString privilege, lPrivilegesString.split(','))
            {
                lPrivileges.append(privilege.trimmed());
            }
        }
        // ALL PRIVILEGES or EVENT
        if(Value.contains("ERROR="))
        {
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                    +"\"> - MYSQL USER PRIVILEGE 'GRANTS'\n"+Value;
            Value = "*************************\n";
            Value+= "o CANNOT EXEC USER GRANTS PRIVILEGE\n";
            Value+= "o MYSQL USER PRIVILEGE 'GRANTS' error\n";
            Value+= "*************************\n";
            Value+= " - NEED GRANTS PRIVILEGE FOR USER OPTION\n";
            Value+= "o Apply the GRANTS privilege to 'ym_admin_db' USER:\n";
            Value+= "   GRANT ALL PRIVILEGE ON ym_admin_db.* TO ym_admin_db WITH GRANT OPTION;\n";
            Value+= "   GRANT ALL PRIVILEGE ON *.* TO ym_admin_db WITH GRANT OPTION;\n";

            Errors.append(Value);
        }
        else if(lPrivileges.contains("ALL PRIVILEGES")
                && lPrivileges.contains("EVENT"))
        {
            Value = "disabled";
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/error.png")
                    +"\"> - MYSQL USER PRIVILEGE 'EVENT' = "+Value;
            Value = "*************************\n";
            Value+= "o UNSUPPORTED USER EVENT PRIVILEGE\n";
            Value+= "o MYSQL USER PRIVILEGE 'EVENT' is disabled\n";
            Value+= "*************************\n";
            Value+= " - NEED EVENT PRIVILEGE FOR EVENT_SCHEDULER OPTION\n";
            Value+= "o Apply the EVENT privilege to 'ym_admin_db' USER:\n";
            Value+= "   GRANT EVENT ON ym_admin_db.* TO ym_admin_db;\n";
            Value+= "   GRANT EVENT ON *.* TO ym_admin_db;\n";

            Errors.append(Value);
        }
        else
        {
            Value = "enabled";
            HtmlCheckText+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/enable.png")
                    +"\"> - MYSQL USER PRIVILEGE 'EVENT' = "+Value;
        }
        HtmlCheckText+= "</ul>\n";

        if(!Errors.isEmpty())
        {
            Section = HtmlCheckText;
            Section+= "<br><br>";
            Section+= Errors.join("<br>").replace("\n","<br>\n");
        }
    }
    if(Type.toUpper() == "VERSION")
    {
        Section+= "<ul>\n";
        Section+= " <li> <b>You need your Administrator login credentials</b>\n";
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_nServerMajorVersion !=
                GS::Gex::Engine::GetInstance().GetAdminEngine().GetCurrentVersion()["DB_VERSION_MAJOR"].toInt())
            Section+= " <li> <b>This is a major update: </b> All Quantix applications will need to be updated!\n";
        else
            Section+= " <li> <b>This is a minor update: </b> Only Yield-Man and Pat-Man applications will need to be updated!\n";
        Section+= " <li> <b>Current version: </b> "+GS::Gex::Engine::GetInstance().GetAdminEngine().GetServerVersionName(true)+"\n";
        Section+= " <li> <b>Update to version: </b> "+GS::Gex::Engine::GetInstance().GetAdminEngine().GetCurrentVersionName(true)+"\n";
        Section+= "</ul>\n";
        QString GexUpdate = " Optional ";
        QString YMUpdate = " Optional ";
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_nServerMajorVersion !=
                GS::Gex::Engine::GetInstance().GetAdminEngine().GetCurrentVersion()["DB_VERSION_MAJOR"].toInt())
            YMUpdate = GexUpdate = " Mandatory ";
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_nServerMinorVersion !=
                GS::Gex::Engine::GetInstance().GetAdminEngine().GetCurrentVersion()["DB_VERSION_MINOR"].toInt())
            YMUpdate = " Mandatory ";
        Section+= "<center><table width=80% border=1 cellspacing=1 bordercolor=\"#999999\">\n";
        Section+= "<tr align=center>\n";
        Section+= "<td></td>\n";
        Section+= "<td><b> Examinator-PRO </b></td>\n";
        Section+= "<td><b> Yield-Man </b></td>\n";
        Section+= "<td><b> PAT-Man </b></td>\n";
        Section+= "</tr>\n";
        Section+= "<tr align=center>\n";
        Section+= "<td>Update Quantix application</td>\n";
        Section+= "<td>"+GexUpdate+"</td>\n";
        Section+= "<td>"+YMUpdate+"</td>\n";
        Section+= "<td>"+YMUpdate+"</td>\n";
        Section+= "</tr>\n";
        Section+= "</table></center>\n";

        Section+= " <DL><DD><img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\"><b>";
        Section+= "It is recommended to backup your current ym_admin_db database.";
        Section+= "<img src=\""+QString::fromUtf8(":/gex/icons/warning.png")+"\"></b></DL>";
    }
    if(Type.toUpper() == "PRESENTATION")
    {
        QString lNew;
        Section+= "<h2>Yield-Man Service Administration provides:</h2>";
        Section+= "<DL>";
        if(m_nFromServerBuild>0 && m_nFromServerBuild < 13)
            lNew = " [new] ";
        else
            lNew = "";
        // New SPM
        Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/spm_task.png")
                +"\" height=\"18\" width=\"20\"> "+lNew+"Statistic Parametric Monitoring on Consolidated data\n";
        // New SYA
        Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/sya_task.png")
                +"\" height=\"18\" width=\"20\"> "+lNew+"Statistic Yield Monitoring on Consolidated data\n";

        Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/system_restart.png")
                +"\" height=\"18\" width=\"20\"> "+lNew+"Active/Passive license for Yield-Man or Pat-Man nodes\n";
        Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/yieldmandb_groups_mng.png")
                +"\" height=\"18\" width=\"20\"> "+lNew+"Users/Groups and Privileges Manager\n";
        Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/system_secure.png")
                +"\" height=\"18\" width=\"20\"> "+lNew+"Yield-Man Databases security management tool\n";
        if(m_nFromServerBuild>0 && m_nFromServerBuild < 8)
            lNew = " [new] ";
        else
            lNew = "";
        Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/system_balancing.png")
                +"\" height=\"18\" width=\"20\"> "+lNew+"Tasks load-balancing between active Yield-Man or Pat-Man nodes\n";
        Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/database-share.png")
                +"\" height=\"18\" width=\"20\"> "+lNew+"Production TDR Databases centralization\n";
        Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/task-share.png")
                +"\" height=\"18\" width=\"20\"> "+lNew+"Tasks centralization\n";
        Section+= "</DL>\n";
    }
    if(Type.toUpper() == "NEWFEATURES")
    {

        if(m_nFromServerBuild>0 && m_nFromServerBuild <= 26)
        {
            Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/system_balancing.png")
                    +"\"> Statistics Parametric Monitoring Tasks\n";
            Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/system_balancing.png")
                    +"\"> Statistics Yield Monitoring Tasks\n";
            Section+= "<BR> - For RAWS data statistics";
            Section+= "<BR> - For CONSOLIDATED data statistics";
            Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/database-share.png")
                    +"\"> Yield-Man Production ADR creation\n";
            Section+= "<BR> - For SPM monitoring";
            Section+= "<BR> - For SYA monitoring";
        }
        // V3.1
        if(m_nFromServerBuild>0 && m_nFromServerBuild <= 13)
        {
            Section+= "<DL>";
            Section+= " <DD> ";
            Section+= "<img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                    +"\">The name of the super user has been renamed from 'root' to <b>'admin'</b>. ('root' is now obsolete) </b>";
            Section+= "<BR> <img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                    +"\">This <b>'admin'</b> user is considered a standard user when login from a <b>V7.0</b> client,<BR>"
                    " and a super user when login from a <b>V7.1</b> client.";
            Section+= "<BR>";
            Section+= "</DL>\n";
        }

        Section+= "<h2>Yield-Man Service Administration new features:</h2>";
        Section+= "<DL>";
        if(m_nFromServerBuild>0 && m_nFromServerBuild <= 13)
        {
            Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/system_restart.png")
                    +"\"> <b>Active/Passive license for Yield-Man or Pat-Man nodes</b>\n";
            Section+= "<BR> - the Passive mode is a licensed feature.\n";
            Section+= "<BR> - please contact "+QString(GEX_EMAIL_SUPPORT)+" to get more details.";
            Section+= "<BR>";

            Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/yieldmandb_groups_mng.png")
                    +"\"> <b>Users/Groups and Privileges Manager</b>\n";
            Section+= "<BR> - use the Users/Groups Manager to update Users/Groups properties";
            Section+= "<BR> - use the Users/Groups Manager to specify application access privileges";
            Section+= "<BR>";

            Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/system_secure.png")
                    +"\"> <b>Yield-Man Databases security management tool</b>\n";
            Section+= "<BR> - use the HouseKeeping tool to update a database to SECURED mode";
            Section+= "<BR> - use the Users/Groups Manager to specify database access privileges";
            Section+= "<BR>";
        }
        if(m_nFromServerBuild>0 && m_nFromServerBuild <= 8)
        {
            Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/system_balancing.png")
                    +"\"> Tasks load-balancing between active Yield-Man or Pat-Man nodes\n";
            Section+= "<BR> - load-balancing mechanism";
            Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/database-share.png")
                    +"\"> Production TDR Databases centralization\n";
            Section+= "<BR> - databases management";
        }

        Section+= "</DL>\n";
    }
    if(Type.startsWith("NODESLIST",Qt::CaseInsensitive))
    {
        QStringList ClauseWhere;
        strQuery = "SELECT node_id, host_name, gex_server_profile, type, mutex FROM ym_nodes";
        if(Type.contains("CONNECTED",Qt::CaseInsensitive))
            ClauseWhere << "NOT(status='SHUTDOWN')"
                        << "(mutex IS NOT NULL)";
        if(Type.contains("RUNNING",Qt::CaseInsensitive))
            ClauseWhere << "NOT(status='SHUTDOWN')"
                        << "NOT(status='STOP')"
                        << "NOT(status='STOP_REQUESTED')"
                        << "(mutex IS NOT NULL) ";
        if(Type.contains("NOTME",Qt::CaseInsensitive))
            ClauseWhere << QString(" (node_id!="+QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().m_nNodeId)+") ");
        if(!ClauseWhere.isEmpty())
            strQuery += " WHERE "+ClauseWhere.join(" AND ");

        if(!clQuery.exec(strQuery))
        {
            Section = "Error executing SQL query.\n";
            Section+= "QUERY=" + strQuery + "\n";
            Section+= "ERROR=" + clQuery.lastError().text();
        }
        while(clQuery.next())
        {
            // Then if the mutex is valid
            if(ClauseWhere.contains("(mutex IS NOT NULL)"))
            {
                if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsMutexActive(clQuery.value(4).toString()))
                    continue;
            }

            if(Section.isEmpty())
            {
                Section+= "<h2> <img src=\""+QString::fromUtf8(":/gex/icons/system_monitoring.png")+"\"> ";
                if(Type.contains("CONNECTED",Qt::CaseInsensitive))
                    Section+= "Nodes connected";
                else if(Type.contains("RUNNING",Qt::CaseInsensitive))
                    Section+= "Nodes running";
                else
                    Section+= "Nodes information";
                Section+= " </h2>\n<DL>\n";
            }
            Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/node.png")
                    +"\"> <b>NodeType["+clQuery.value(3).toString()+"]</b>"
                    +" \n<br>- Host["+clQuery.value(1).toString()
                    +"] \n<br>- Server profile["+clQuery.value(2).toString()+"]\n";
        }
        if(!Section.isEmpty())
            Section+= "</DL>\n";
    }
    if(Type.toUpper() == "TASKSLIST")
    {
        bool HaveOldDatapump = false;
        strQuery = "SELECT type, COUNT(*) FROM ym_tasks GROUP BY type ORDER BY type DESC";
        if(!clQuery.exec(strQuery))
        {
            Value = "Error executing SQL query.\n";
            Value+= "QUERY=" + strQuery + "\n";
            Value+= "ERROR=" + clQuery.lastError().text();
        }
        while(clQuery.next())
        {
            Value+= " <DD> ";
            switch(clQuery.value(0).toInt())
            {
            case GEXMO_TASK_DATAPUMP:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/folder_insertion.png")
                        +"\"> <b>DataPump</b>";
                break;
            case GEXMO_TASK_TRIGGERPUMP:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/folder_script.png")
                        +"\"> <b>TriggerPump</b>";
                break;
            case GEXMO_TASK_PATPUMP:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/folder_script.png")
                        +"\"> <b>PATPump</b>";
                break;
            case GEXMO_TASK_YIELDMONITOR:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/mo_yield_monitoring.png")
                        +"\"> <b>Yield Monitoring</b>";
                break;
            case GEXMO_TASK_REPORTING:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/reporting_task.png")
                        +"\"> <b>Reporting</b>";
                break;
            case GEXMO_TASK_STATUS:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/options_datalog.png")
                        +"\"> <b>Status</b>";
                break;
            case GEXMO_TASK_CONVERTER:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/folder_convert.png")
                        +"\"> <b>Converter Pump</b>";
                break;
            case GEXMO_TASK_OUTLIER_REMOVAL:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/options_outlier.png")
                        +"\"> <b>Outlier Removal</b>";
                break;
            case GEXMO_TASK_AUTOADMIN:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/calendar_icon.png")
                        +"\"> <b>Auto Admin</b>";
                break;
            case GEXMO_TASK_SPM:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/spm_task.png")
                        +"\"> <b>SPM</b>";
                break;
            case GEXMO_TASK_SYA:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/sya_task.png")
                        +"\"> <b>SPM</b>";
                break;
            case GEXMO_TASK_OLD_DATAPUMP:
                Value += "<img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                        +"\"> <b> !! Old DataPump !! </b><img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                        +"\"> ";
                HaveOldDatapump = true;
                break;
            }
            Value+= " - "+clQuery.value(1).toString()+(clQuery.value(1).toInt()<=1?" task\n":" tasks\n");
        }
        if(!Value.isEmpty())
        {
            if(HaveOldDatapump)
            {
                Value+= "</DL>\n";
                Value+= "<h3> <img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                        +"\"> Check your tasks </h3>\n";
                Value+= "<DL>\n";
                Value+= " You must specify now which type of pump you create. \n";
                Value+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/folder_insertion.png")
                        +"\"> DataPump: Test data for database insertion\n";
                Value+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/folder_script.png")
                        +"\"> TriggerPump: Trigger file or JS file for script execution for Yield-Man\n";
                Value+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/folder_script.png")
                        +"\"> PATPump: Trigger file or JS file for script execution for Pat-Man\n";
            }

            Section = "<h2> <img src=\""+QString::fromUtf8(":/gex/icons/task.png")
                    +"\"> Tasks information </h2>\n";
            Section+= "<DL>\n";
            Section+= Value;
            Section+= "</DL\n>";
        }
    }
    if(Type.startsWith("TasksDetail",Qt::CaseInsensitive))
    {
        if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().getTasksList().count() > 0)
        {
            QList<CGexMoTaskItem*> lTasksList = GS::Gex::Engine::GetInstance().GetSchedulerEngine().getTasksList();
            CGexMoTaskItem * pTaskEntry;
            QString strTaskHtml;
            bool HaveOldDatapump = false;
            bool HaveLocalTask = false;
            foreach(pTaskEntry,  lTasksList)
            {
                if(pTaskEntry->IsUploaded())
                {
                    if(!Type.contains("OnlyError",Qt::CaseInsensitive))
                        strTaskHtml+=" <DD> <img src=\""
                                +QString::fromUtf8(":/gex/icons/enable.png")+"\"> "
                                +pTaskEntry->m_strName+"\n";
                    else
                    {
                        // Check error
                        QString strErrorStatus = pTaskEntry->GetErrorStatus();
                        if(!strErrorStatus.isEmpty())
                        {
                            strTaskHtml+=" <DD> <img src=\""
                                    +QString::fromUtf8(":/gex/icons/error.png")+"\"> "
                                    +pTaskEntry->m_strName;
                            strTaskHtml+= ": "+strErrorStatus.section(".",0,0)+"\n";
                        }
                    }
                    if(pTaskEntry->GetTaskType() == 99)
                        HaveOldDatapump = true;
                }
                else
                {
                    HaveLocalTask = true;
                    QString strErrorStatus = pTaskEntry->GetErrorStatus();
                    strTaskHtml+=" <DD> <img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                            +"\"> "+pTaskEntry->m_strName;
                    if(!strErrorStatus.isEmpty())
                        strTaskHtml+=" : "+strErrorStatus+"\n";
                    else
                        strTaskHtml+=" : Based on File-Based database";
                }
            };
            if(!strTaskHtml.isEmpty())
            {
                Section = "<h2> <img src=\""+QString::fromUtf8(":/gex/icons/system_monitoring.png")+"\"> ";
                if(Type.contains("OnlyError",Qt::CaseInsensitive))
                    Section+= "Tasks warning </h2>\n";
                else
                    Section+= "Tasks detail </h2>\n";

                if(HaveLocalTask)
                    Section+= "<DL><DT><img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                            +"\"> Local task will not be supported by the load-balancing process. \n";
                if(HaveOldDatapump)
                    Section+= "<DL><DT><img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                            +"\"> Old DataPump will not be supported by the Monitoring process. You must convert all this tasks to specify a new format\n";

                Section+= "<DL>\n";
                Section+= strTaskHtml;
                Section+="</DL>\n";
            }
        }
    }
    if(Type.startsWith("DatabasesList",Qt::CaseInsensitive))
    {
        if(GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.count() > 0)
        {
            QList<GexDatabaseEntry*>::iterator itBegin  = GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.begin();
            QList<GexDatabaseEntry*>::iterator itEnd  = GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.end();
            GexDatabaseEntry * pDatabaseEntry;
            QString strDatabaseHtml;
            bool bHaveFileBased = false;
            bool bHaveLocalDb = false;

            while(itBegin != itEnd)
            {
                pDatabaseEntry = *itBegin;
                if(pDatabaseEntry->IsStoredInDb())
                {
                    if(!pDatabaseEntry->IsBlackHole())
                    {
                        if(!Type.contains("OnlyError",Qt::CaseInsensitive))
                        {
                            strDatabaseHtml+=" <DD> <img src=\""
                                    +QString::fromUtf8(pDatabaseEntry->IconName().toLatin1().constData())+"\"> "
                                    +pDatabaseEntry->LogicalName();
                            strDatabaseHtml+=" : Yield-Man Production TDR";
                        }
                        else
                        {
                            // Check error
                            QString strErrorStatus = pDatabaseEntry->m_strLastInfoMsg;
                            if(!strErrorStatus.isEmpty())
                            {
                                strDatabaseHtml+=" <DD> <img src=\""
                                        +QString::fromUtf8(":/gex/icons/error.png")+"\"> "
                                        +pDatabaseEntry->LogicalName();
                                strDatabaseHtml+= ": "+strErrorStatus.section(".",0,0)+"\n";
                            }
                        }
                    }
                }
                else if(!Type.contains("Uploaded",Qt::CaseInsensitive))
                {
                    bHaveLocalDb = true;
                    if(pDatabaseEntry->IsExternal())
                    {
                        if(pDatabaseEntry->IsCharacTdr())
                        {
                            strDatabaseHtml+=" <DD> <img src=\""
                                    +QString::fromUtf8(pDatabaseEntry->IconName().toLatin1().constData())+"\"> <b>"
                                    +pDatabaseEntry->LogicalName();
                            strDatabaseHtml+="</b> : Characterization TDR";
                        }
                        else if(pDatabaseEntry->IsManualProdTdr())
                        {
                            strDatabaseHtml+=" <DD> <img src=\""
                                    +QString::fromUtf8(pDatabaseEntry->IconName().toLatin1().constData())+"\"> <b>"
                                    +pDatabaseEntry->LogicalName();
                            strDatabaseHtml+="</b> : Manual Production TDR";

                        }
                        else
                        {
                            strDatabaseHtml+=" <DD> <img src=\""
                                    +QString::fromUtf8(pDatabaseEntry->IconName().toLatin1().constData())+"\"> <b>"
                                    +pDatabaseEntry->LogicalName();
                            strDatabaseHtml+="</b> : Coorporate database";
                        }
                    }
                    else if(pDatabaseEntry->IsBlackHole())
                    {
                        strDatabaseHtml+=" <DD> <img src=\""
                                +QString::fromUtf8(pDatabaseEntry->IconName().toLatin1().constData())+"\"> <b>"
                                +pDatabaseEntry->LogicalName();
                        strDatabaseHtml+="</b> : Black Hole";

                    }
                    else
                    {
                        strDatabaseHtml+=" <DD> <img src=\""
                                +QString::fromUtf8(pDatabaseEntry->IconName().toLatin1().constData())+"\"> <b>"
                                +pDatabaseEntry->LogicalName();
                        strDatabaseHtml+="</b> : File-Based database";
                        bHaveFileBased = true;
                    }
                }
                // Move to next enry.
                itBegin++;
            };
            if(!strDatabaseHtml.isEmpty())
            {
                Section+= "<h2> <img src=\""+QString::fromUtf8(":/gex/icons/database.png")+"\">";
                if(bHaveFileBased && !Type.contains("Uploaded",Qt::CaseInsensitive))
                    Section+= "Databases warning </h2>";
                else if(Type.contains("OnlyError",Qt::CaseInsensitive))
                    Section+= "Databases warning </h2>";
                else
                    Section+= "Databases information </h2>";
                if(bHaveFileBased)
                    Section+= "<DL><DT><img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                            +"\"> File-based databases will not be supported by the load-balancing process. \n";
                if(bHaveLocalDb)
                    Section+= "<DL><DT><img src=\""+QString::fromUtf8(":/gex/icons/warning.png")
                            +"\"> Local databases will not be supported by the load-balancing process. \n";

                Section+= "<DL>";
                Section+= strDatabaseHtml;
                Section+= "</DL>\n";
            }
        }
    }
    if(Type.toUpper() == "YIELDMANDBCONFIG")
    {
        Section+= "<h2> <img src=\""+QString::fromUtf8(":/gex/icons/network.png")
                +"\"> Yield-Man Administration Server information </h2>";
        Section+= "<DL>";
        Section+= " <DD> <img src=\""+QString::fromUtf8(":/gex/icons/enable.png")
                +"\"> <b>"+GS::Gex::Engine::GetInstance().GetAdminEngine().GetServerVersionName(true)+"</b>\n";
        Section+= "<ul>";
        Section+= " <li> HostName: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strHost_Name+"\n";
        Section+= " <li> IP: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strHost_IP+"\n";
        Section+= " <li> Port: "+QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_uiPort)+"\n";
        Section+= " <li> Driver: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strDriver+"\n";
        Section+= " <li> Database: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strSchemaName+"\n";
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->IsOracleDB())
            Section+= " <li> SID: "+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strDatabaseName+"\n";
        Section+= " <li> Password: "+QString().rightJustified(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strPassword_Admin.length(),'*')+"\n";
        Section+= "</ul>\n";
        Section+= "</DL>\n";
    }

    return Section;
}

// Transfert to pGexMainWindows
void AdminGui::ConnectUserDialog(bool bAsAdmin, bool bForceConnection)
{
    if(m_pLoginDialog && m_pLoginDialog->isVisible())
    {
        // Already displayed
        return;
    }

    if(m_pLoginDialog == NULL)
        m_pLoginDialog = new AdminUserLogin(pGexMainWindow->centralWidget());

    m_pLoginDialog->m_bForceConnection = bForceConnection;

    m_pLoginDialog->m_bAsAdmin = bAsAdmin;

    m_pLoginDialog->ShowPageUserConnection();

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void AdminGui::OnActionConnect()
{
    if(m_pLoginDialog && m_pLoginDialog->isVisible())
    {
        // Already displayed
        return;
    }

    // Enabled or Disabled some access
    // EnableGexAccess();

    // Check if have one user connected
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                && GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
        {
            QString strMsg = "The Scheduler is stopped.\n\n";
            strMsg += "Do you really want to log out?";
            if(QMessageBox::warning(
                        this,
                        QCoreApplication::applicationName(),
                        strMsg,"Yes, log out","Cancel") != 0)
                return;
        }

        GS::Gex::Engine::GetInstance().GetAdminEngine().DisconnectCurrentUser();
    }
    else
    {
        // Not connected
        if(GS::Gex::Engine::GetInstance().GetLicensePassive())
        {
            QString lMsg;
            lMsg += QCoreApplication::applicationName() + " is startup in Passive License\n\n";
            lMsg += "Please wait for Active License to be ready ";
            QMessageBox::warning(this,QCoreApplication::applicationName(), lMsg);
            return;
        }
        // Check if the connection is allow
        QString lCompatibleInfo;
        if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsServerCompatible(
                    !GS::LPPlugin::ProductInfo::getInstance()->isMonitoring(),
                    lCompatibleInfo))
        {
            QMessageBox::warning(this,QCoreApplication::applicationName(), lCompatibleInfo);
            return;
        }

        ConnectUserDialog(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring(),!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring());
    }
}

void AdminGui::ButtonConnectEnabled(bool Enable)
{
    if(mButtonConnect)
        mButtonConnect->setEnabled(Enable);
}

void  AdminGui::ButtonConnectIsEnabled(bool& Enable)
{
    Enable=false;
    if(mButtonConnect)
        Enable=mButtonConnect->isEnabled();
}

void AdminGui::ButtonReloadListEnabled(bool Enable)
{
    if(mWindowDetached)
        buttonReloadList->setEnabled(true);
    else if(!GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
        buttonReloadList->setEnabled(false);
    else
        buttonReloadList->setEnabled(Enable);
}

void AdminGui::ButtonReloadListIsEnabled(bool& Enable)
{
    Enable = buttonReloadList->isEnabled();
}


//////////////////////////////////////////////////////////////////////
void AdminGui::UserConnectionNeededToDialog(QString &strAction)
{
    QString strMsg = "You must be logged to "+strAction+"!<br><br>\n";
    strMsg += "Please click the loggin button. <img src=\""+QString::fromUtf8(":/gex/icons/da_disconnected.png")+"\">";
    QString strTitle = "Yield-Man Administration Server request";
    QMessageBox::information(pGexMainWindow,strTitle, strMsg);
}

//////////////////////////////////////////////////////////////////////
// Enable some Gex button access
//////////////////////////////////////////////////////////////////////
void AdminGui::EnableGexAccess()
{

    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        CheckButtonsStatus();
        return;
    }

    if(mUpdatePrivilegesInProgress)
    {
        return;
    }

    GSLOG(SYSLOG_SEV_DEBUG, "EnableGexAccess");
    mUpdatePrivilegesInProgress = true;
    // Ignore OnSelectTab update signal
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().setTaskBeingEdited(true);

    bool lLoginStateChanged = true; // track if user login state has changed: logged/not logged/disconnected

    if(mButtonConnect
            && GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin()
            && GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin()->GetConnector()
            && GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin()->GetConnector()->IsConnected())
    {
        QString lCurrentUser = "anonymous";
        QString lCurrentLogin = mButtonConnect->iconText();
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser)
            lCurrentUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strLogin;
        if ((lCurrentUser == lCurrentLogin))
            lLoginStateChanged = false;
    }

    UpdateUIPrivileges(lLoginStateChanged);

    if (GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin()
            && GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin()->GetConnector()
            && GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin()->GetConnector()->IsConnected())
        UpdateDBConnection(lLoginStateChanged);

    // Unlock tasks
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().setTaskBeingEdited(false);
    mUpdatePrivilegesInProgress = false;
}

void AdminGui::UpdateUIPrivileges(bool loginStateChanged)
{
    // Enabled or Disabled some access
    bool lUserConnected = true;
    bool lUserConnectedAsAdmin = true;
    bool lUsersHasGroupsAdminPrivileges = true;

    ///////////////////////////////
    // Admin Server MODE
    // Change when user connected or disconnected
    QString lLoginInfo;

    ///////////////////////////////
    // Check User/admin Connection
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        lLoginInfo += " Yield-Man";
        // YieldMan
        lUsersHasGroupsAdminPrivileges = GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges();
        lUserConnectedAsAdmin = GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected(true);
        // User admin must be connected
        lUserConnected = lUserConnectedAsAdmin;
    }
    else
    {
        lLoginInfo += " Examinator-PRO";
        lUsersHasGroupsAdminPrivileges = GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges();
        lUserConnectedAsAdmin = GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected(true);
        // Examinator
        // User must be connected if OptionLevel >= USER_LOGIN
        lUserConnected = GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected();
    }
    if(lUserConnectedAsAdmin)
        lLoginInfo += " connected as MASTER_ADMIN";
    else if(lUserConnected)
        lLoginInfo += " connected as USER";
    else
        lLoginInfo += " no user connected";

    if (!lLoginInfo.isEmpty())
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Enable Gex Access mode changes : %1").arg(lLoginInfo).toLatin1().data() );


    ///////////////////////////////
    // Option Center - Save/Load Profile
    if(pGexMainWindow->m_pOptionsCenter)
    {
        // WITH gex_mo_user_profile.csl IMPLEMENTATION
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            pGexMainWindow->m_pOptionsCenter->m_SaveProfile->hide();
            pGexMainWindow->m_pOptionsCenter->m_LoadProfile->hide();
        }
        else
        {
            pGexMainWindow->m_pOptionsCenter->m_SaveProfile->show();
            pGexMainWindow->m_pOptionsCenter->m_LoadProfile->show();
        }
    }

    ///////////////////////////////
    // Login Connection Button
    // Add in the Gex tool bar, the login button or update it if needed
    UpdateLoginButton(loginStateChanged);

    ///////////////////////////////
    // Scheduler GUI - Tasks list
    UpdateTasksTabs(loginStateChanged,lUserConnected, lUserConnectedAsAdmin );

    ///////////////////////////////
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        TextLabelStatus->show();
        TextLabelTip->show();
    }
    else
    {
        TextLabelStatus->hide();
        TextLabelTip->show();
    }
    ///////////////////////////////
    // Scheduler GUI - Monitoring tabs
    UpdateMonitoringTabs(lUserConnected, lUsersHasGroupsAdminPrivileges);

    QCoreApplication::processEvents();
}

void AdminGui::UpdateLoginButton(bool /*loginStateChanged*/)
{
    if(mButtonConnect == NULL)
    {
        // Check if already exists
        QList<QAction*>             lstActions = pGexMainWindow->toolBarButton->actions();
        QList<QAction*>::iterator   lIter;
        QAction*                    lAction;
        for ( lIter = lstActions.begin(); lIter != lstActions.end(); ++lIter )
        {
            lAction = *lIter;
            if(lAction->objectName() == "actionConnect")
            {
                mButtonConnect = lAction;
                break;
            }
            else
            {   // Reset text displayed for other Actions
                lAction->setIconText("");
                lAction->setText("");
            }
        }
    }
    if(mButtonConnect == NULL)
    {
        // If no exists, create new button
        mButtonConnect = new QAction(this);
        mButtonConnect->setObjectName(QString::fromUtf8("actionConnect"));
        mButtonConnect->setProperty("name", QVariant(QCoreApplication::translate(
                                                         "GexMainWindow",
                                                         "actionConnect",
                                                         0,
                                                         QCoreApplication::UnicodeUTF8)));

        //pGexMainWindow->toolBarButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        pGexMainWindow->toolBarButton->addSeparator();
        pGexMainWindow->toolBarButton->addAction(mButtonConnect);


        QWidget* spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        // toolBar is a pointer to an existing toolbar
        pGexMainWindow->toolBarButton->addWidget(spacer);

        mLoginUser = new QLabel(pGexMainWindow->toolBarButton);
        mLoginUser->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        mLoginUser->setText("test");
        pGexMainWindow->toolBarButton->addWidget(mLoginUser);

        QObject::connect(mButtonConnect,    SIGNAL(triggered()),        this, SLOT(OnActionConnect()));

        // When gex starts, Connect button must be disabled until the end of the load of the databases list
        mButtonConnect->setEnabled(false);
    }


    pGexMainWindow->setEnabled(true);

    if(mButtonConnect)
    {
        // Check if user connected
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserConnected())
        {
            // Change icon
            QIcon icon;
            QString iconImg = ":/gex/icons/da_logged.png";
            QString loginText = QString("Logged as '%1'").arg(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strLogin);
            QString iconToolTip = loginText+". Change user?";
            if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            {
                iconImg = ":/gex/icons/da_logged.png";
                iconToolTip = loginText+". Log out?";
                if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_nNodeId > 0)
                    loginText += QString(" - Node[%1]").arg(GS::Gex::Engine::GetInstance().GetAdminEngine().m_nNodeId);

            }

            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin())
            {
                iconImg = ":/gex/icons/lock.png";
                loginText = "Connected as '"+GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strLogin+"'";
                iconToolTip = loginText+". Disconnect?";

            }

            //icon.addFile(QString::fromUtf8(":/gex/icons/unlock.png"), QSize(), QIcon::Normal, QIcon::Off);
            icon.addFile(QString::fromUtf8(iconImg.toLatin1().constData()), QSize(), QIcon::Normal, QIcon::Off);
            mButtonConnect->setIcon(icon);
            mButtonConnect->setIconText(QCoreApplication::translate(
                                            "GexMainWindow",
                                            GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_strLogin.toLatin1().constData(),
                                            0,
                                            QCoreApplication::UnicodeUTF8));
            mButtonConnect->setText(QCoreApplication::translate(
                                        "GexMainWindow",
                                        iconToolTip.toLatin1().constData(),
                                        0,
                                        QCoreApplication::UnicodeUTF8));
            mLoginUser->setText(loginText);
        }
        else
        {
            // Change icon
            QIcon icon;
            QString iconImg = ":/gex/icons/da_anonymous.png";
            QString loginText = "Logged as 'anonymous'";
            QString iconToolTip = loginText + ". Change user?";
            if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            {
                iconImg = ":/gex/icons/da_disconnected.png";
                iconToolTip = "User is not logged. Log in?";
                loginText = "Not logged";
                if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_nNodeId > 0)
                    loginText += QString(" - Node[%1]").arg(GS::Gex::Engine::GetInstance().GetAdminEngine().m_nNodeId);
            }

            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin())
            {
                iconImg = ":/gex/icons/unlock.png";
                loginText = "Not connected";
                iconToolTip = "User is not connected. Connect?";
            }
            else if(!GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin()->GetConnector()->IsConnected()
                    // && GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                    )
            {
                iconImg = ":/gex/icons/da_warning.png";
                loginText = "Connection lost...";
                iconToolTip = "Wait or Exit application";
                pGexMainWindow->setEnabled(false);
            }

            icon.addFile(QString::fromUtf8(iconImg.toLatin1().constData()), QSize(), QIcon::Normal, QIcon::Off);
            mButtonConnect->setIcon(icon);
            mButtonConnect->setIconText(QCoreApplication::translate(
                                            "GexMainWindow",
                                            "anonymous",
                                            0,
                                            QCoreApplication::UnicodeUTF8));
            mButtonConnect->setText(QCoreApplication::translate(
                                        "GexMainWindow",
                                        iconToolTip.toLatin1().constData(),
                                        0,
                                        QCoreApplication::UnicodeUTF8));
            mLoginUser->setText(loginText);
        }
    }
}

void AdminGui::UpdateTasksTabs(bool loginStateChanged, bool userConnected, bool userConnectedAsAdmin)
{
    if(pGexMainWindow->pWizardSchedulerGui)
    {
        ///////////////////////////////
        // Monitoring Mode
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            if(!mWindowDetached
                    && (tabWidgetAdminGui->indexOf(pGexMainWindow->pWizardSchedulerGui) == -1))
                tabWidgetAdminGui->insertTab(
                            1,
                            pGexMainWindow->pWizardSchedulerGui,
                            QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_scheduler.png"))),
                            "Tasks Scheduler");
        }
        ///////////////////////////////
        // Examinator-PRO Mode
        else
        {
            // If user not connected
            if(loginStateChanged)
            {
                int TabIndex = 0;
                // Remove all tabs
                while(pGexMainWindow->pWizardSchedulerGui->tabWidget->count() > 0)
                    pGexMainWindow->pWizardSchedulerGui->tabWidget->removeTab(0);
                // Add user Admin tabs
                if(userConnectedAsAdmin)
                {
                    if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListDataPump->isHidden())
                        pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                    TabIndex++,
                                    pGexMainWindow->pWizardSchedulerGui->tabTasksDataPump,
                                    QIcon(QPixmap(QString::fromUtf8(":/gex/icons/folder_insertion.png"))),
                                    "DataPump");
                    if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListTriggerSya->isHidden())
                        pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                    TabIndex++,
                                    pGexMainWindow->pWizardSchedulerGui->tabTasksTriggerSya,
                                    QIcon(QPixmap(QString::fromUtf8(":/gex/icons/folder_script.png"))),
                                    "TriggerPump");
                    if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListTriggerPat->isHidden())
                        pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                    TabIndex++,
                                    pGexMainWindow->pWizardSchedulerGui->tabTasksTriggerPat,
                                    QIcon(QPixmap(QString::fromUtf8(":/gex/icons/folder_script.png"))),
                                    "PatPump");
                    if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListReporting->isHidden())
                        pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                    TabIndex++,
                                    pGexMainWindow->pWizardSchedulerGui->tabTasksConverter,
                                    QIcon(QPixmap(QString::fromUtf8(":/gex/icons/folder_convert.png"))),
                                    "ConverterPump");
                }
                // Standard tabs
                if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListReporting->isHidden())
                    pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                TabIndex++,
                                pGexMainWindow->pWizardSchedulerGui->tabTasksReporting,
                                QIcon(QPixmap(QString::fromUtf8(":/gex/icons/reporting_task.png"))),
                                "Reporting");

                if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListOutlierRemoval->isHidden())
                    pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                TabIndex++,
                                pGexMainWindow->pWizardSchedulerGui->tabTasksOutlierRemoval,
                                QIcon(QPixmap(QString::fromUtf8(":/gex/icons/options_outlier.png"))),
                                "OutlierRemoval");
                if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListYieldLimits->isHidden())
                    pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                TabIndex++,
                                pGexMainWindow->pWizardSchedulerGui->tabTasksYield,
                                QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_yield_monitoring.png"))),
                                "YieldMonitoring");
/*                if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListSPM->isHidden())
                    pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                TabIndex++,
                                pGexMainWindow->pWizardSchedulerGui->tabTasksSpm,
                                QIcon(QPixmap(QString::fromUtf8(":/gex/icons/spm_task.png"))),
                                "SPM");
*/
                if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListSYA_2->isHidden())
                    pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                TabIndex++,
                                pGexMainWindow->pWizardSchedulerGui->tabTasksSya,
                                QIcon(QPixmap(QString::fromUtf8(":/gex/icons/sya_task.png"))),
                                "SYA");
                // Add user Admin tabs
                if(userConnectedAsAdmin)
                {
                    if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListStatus->isHidden())
                        pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                    TabIndex++,
                                    pGexMainWindow->pWizardSchedulerGui->tabTasksStatus,
                                    QIcon(QPixmap(QString::fromUtf8(":/gex/icons/options_datalog.png"))),
                                    "Status");
                    if(!pGexMainWindow->pWizardSchedulerGui->tableWidgetTasksListAutoAdmin->isHidden())
                        pGexMainWindow->pWizardSchedulerGui->tabWidget->insertTab(
                                    TabIndex++,
                                    pGexMainWindow->pWizardSchedulerGui->tabTasksAutoAdmin,
                                    QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_admin_report.png"))),
                                    "AutoAdmin");
                }
            }

            if(userConnected)
            {
                if(!mWindowDetached
                        && (tabWidgetAdminGui->indexOf(pGexMainWindow->pWizardSchedulerGui) == -1))
                    tabWidgetAdminGui->insertTab(
                                1,
                                pGexMainWindow->pWizardSchedulerGui,
                                QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_scheduler.png"))),
                                "Tasks Scheduler");

                GS::Gex::Engine::GetInstance().GetSchedulerEngine().stopScheduler(true);
            }
            else
            {
                tabWidgetAdminGui->removeTab(
                            tabWidgetAdminGui->indexOf(pGexMainWindow->pWizardSchedulerGui));

                ///////////////////////////////
                // Reset all tasks views
                // User can change and tasks list can be different
                pGexMainWindow->pWizardSchedulerGui->UpdateListView(QStringList()<<"Clear");
            }
        }

        pGexMainWindow->pWizardSchedulerGui->CheckButtonsStatus();
        pGexMainWindow->pWizardSchedulerGui->DisplayStatusMessage();
    }
}

void AdminGui::UpdateMonitoringTabs(bool userConnected, bool usersHasGroupsAdminPrivileges)
{
    ///////////////////////////////
    // Add Database Admin if missing
    if(tabWidgetAdminGui->indexOf(tabDatabases) < 0)
        tabWidgetAdminGui->addTab(
                    tabDatabases,
                    QIcon(QPixmap(QString::fromUtf8(":/gex/icons/database-display.png"))),
                    "Databases");



    if(!mWindowDetached)
    {
        // Add Insertion Log files
        if(pGexMainWindow->mMonitoringWidget
                && (tabWidgetAdminGui->indexOf(pGexMainWindow->mMonitoringWidget) < 0))
            tabWidgetAdminGui->addTab(
                        pGexMainWindow->mMonitoringWidget,
                        QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_logs.png"))),
                        "Admin Logs");

        // Remove tabUsers
        if(!userConnected)
            tabWidgetAdminGui->removeTab(tabWidgetAdminGui->indexOf(tabUsers));

        if(userConnected)
        {
            // Add tabTasks
            if(pGexMainWindow->pWizardSchedulerGui
                    && (tabWidgetAdminGui->indexOf(pGexMainWindow->pWizardSchedulerGui) < 0))
                tabWidgetAdminGui->insertTab(
                            1,
                            pGexMainWindow->pWizardSchedulerGui,
                            QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_scheduler.png"))),
                            "Tasks Scheduler");
            // Add tabUsers
            if(usersHasGroupsAdminPrivileges)
            {
                if(tabWidgetAdminGui->indexOf(tabUsers) < 0)
                    tabWidgetAdminGui->addTab(
                                tabUsers,
                                QIcon(QPixmap(QString::fromUtf8(":/gex/icons/yieldmandb_users.png"))),
                                "Users");
                QIcon icon5;
                icon5.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/yieldmandb_groups_mng.png")), QIcon::Normal, QIcon::Off);
                buttonNewUser->setIcon(icon5);
                buttonNewUser->setIconSize(QSize(32, 32));
                buttonNewUser->setToolTip("Manage Users/Groups entries");
            }
            else
            {
                QIcon icon5;
                icon5.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/yieldmandb_user_mng.png")), QIcon::Normal, QIcon::Off);
                buttonNewUser->setIcon(icon5);
                buttonNewUser->setIconSize(QSize(32, 32));
                buttonNewUser->setToolTip("Manage your properties");
                if(tabWidgetAdminGui->indexOf(tabUsers) < 0)
                    tabWidgetAdminGui->addTab(
                                tabUsers,
                                QIcon(QPixmap(QString::fromUtf8(":/gex/icons/yieldmandb_users.png"))),
                                "Users");
            }
        }
        else
        {
            // Add tabTasks
            if(pGexMainWindow->pWizardSchedulerGui
                    && GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                    && (tabWidgetAdminGui->indexOf(pGexMainWindow->pWizardSchedulerGui) < 0))
                tabWidgetAdminGui->insertTab(
                            1,
                            pGexMainWindow->pWizardSchedulerGui,
                            QIcon(QPixmap(QString::fromUtf8(":/gex/icons/mo_scheduler.png"))),
                            "Tasks Scheduler");
            // Empty the Users List GUI
            tableWidgetUser->setRowCount(0);
        }
    }
}

void AdminGui::UpdateDBConnection(bool loginStateChanged)
{
    ///////////////////////////////
    // With ExaminatorPRO
    // Disconnect Restricted DB if User not connected
    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
            && loginStateChanged)
    {
        // User just disconnected
        // ReloadDatabase to disconnect Restricted DB
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckDatabasesList();
    }
    else
        OnSelectDatabase();
}

void AdminGui::OnCheckAdminActivity()
{
    if(!m_bCheckAdminActivity)
        return;

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser == NULL)
        return;

    if(m_nNbMinBeforeDisconnect == -1)
        return;

    QDateTime clLastDateTime = GS::Gex::Engine::GetInstance().GetLastUserActivity();
    if(clLastDateTime.addSecs(m_nNbMinBeforeDisconnect*60) < GS::Gex::Engine::GetInstance().GetClientDateTime())
    {
        // No activity during more than m_nNbMinBeforeDisconnect minutes
        // Check if Scheduler is paused
        // Check if no dialogBox is displayed
        if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
        {
            // Then ignore disconnection
        }
        else if(GS::Gex::Engine::GetInstance().GetSchedulerEngine().isTaskBeingEdited())
        {
            // Then ignore disconnection
        }
        else if(m_bDatabaseEdited)
        {
            // Then ignore disconnection
        }
        else if(m_bYieldManItemEdited)
        {
            // Then ignore disconnection
        }
        else
        {
            // Then disconnect
            GSLOG(SYSLOG_SEV_NOTICE, QString("Inactivity disconnect").toLatin1().constData());
            GS::Gex::Engine::GetInstance().GetAdminEngine().DisconnectCurrentUser();
            return;
        }
    }

    QTimer::singleShot(30000, this, SLOT(OnCheckAdminActivity(void)));
}



