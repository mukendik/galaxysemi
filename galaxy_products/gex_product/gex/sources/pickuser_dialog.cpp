#include <qdir.h>
#include <qinputdialog.h>

#include "browser_dialog.h"
#include "engine.h"
#include "pickuser_dialog.h"
#include "report_build.h"
#include "product_info.h"
#include "admin_engine.h"
#include "license_provider_profile.h"
#include "message.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

///////////////////////////////////////////////////////////
// Constructor
PickUserDialog::PickUserDialog( QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,		SIGNAL(clicked()),						this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()),						this, SLOT(reject()));

    // Empty Filter list
    treeWidgetProfiles->clear();

    // Multiple selection mode allowed, Fill the list with sorting enabled
    treeWidgetProfiles->setSelectionMode(QTreeWidget::SingleSelection);
    treeWidgetProfiles->sortByColumn(0, Qt::AscendingOrder);

    // Set minimum size for the Value column
    treeWidgetProfiles->setColumnWidth(0, 200);
    treeWidgetProfiles->setColumnWidth(1, 60);

    // Read the default user name
    m_strDefaultUserName = GS::Gex::Engine::GetInstance().GetDefaultUserProfile();

    // Fill list with current list of users.
    FillList();

    // Slots connections
    connect(buttonRemoveUser,	SIGNAL(clicked()),	this,	SLOT(OnRemoveUser(void)));
    connect(buttonProperties,	SIGNAL(clicked()),	this,	SLOT(OnProperties(void)));
    connect(treeWidgetProfiles,	SIGNAL(itemClicked(QTreeWidgetItem*, int)),			this, SLOT(onItemClicked(void)));
    connect(treeWidgetProfiles,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),	this, SLOT(onItemDoubleClicked(void)));
    connect(treeWidgetProfiles, SIGNAL(itemChanged(QTreeWidgetItem*, int)),			this, SLOT(onItemChanged(QTreeWidgetItem*, int)));
}

///////////////////////////////////////////////////////////
// Sets GUI based on action to execute ('Load' or 'Save as')
///////////////////////////////////////////////////////////
void PickUserDialog::setLoadMode(bool bLoad)
{
    if(bLoad)
    {
        // Hide the 'New user' fields: can only load existing profiles.
        TextLabelNewUser->hide();
        lineEditNewUser->hide();

        // Set text
        TextLabel->setText("Pick one User name/Profile from the list...");
    }
    else
    {
        // Show the 'New user' fields
        TextLabelNewUser->show();
        lineEditNewUser->show();
        lineEditNewUser->setFocus();

        // Set text
        TextLabel->setText("Pick one name from the list or enter a new User name/Profile!");
    }
}

///////////////////////////////////////////////////////////
// Fill list with user profiles detected.
///////////////////////////////////////////////////////////
void PickUserDialog::FillList(void)
{
    // If we have no valid license yet, don't load any user profile
    if(!GS::Gex::Engine::GetInstance().HasNodeValidLicense())
        return;

    // Synchronize with YieldManDB profiles
    if(GS::Gex::Engine::GetInstance().IsAdminServerMode(true))
        GS::Gex::Engine::GetInstance().GetAdminEngine().SynchronizeProfiles(m_strDefaultUserName);

    // For YieldMan Monitoring, ignore user profile
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return;

    // Try to load a user profile
    QString strPath = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();

    // Find all files with name <name>_user_profile.csl
    QDir cDir(strPath);
    cDir.setFilter(QDir::Files);	// Non-recursive: ONLY import this folder
    QStringList strFiles = cDir.entryList(QStringList("*_user_profile.csl"));	// Files extensions to look for...

    // Empty list
    treeWidgetProfiles->clear();

    // Extract users names from file name
    QTreeWidgetItem * pItem = NULL;
    QStringList::Iterator it;
    QString strUserName;
    QString strDefaultName = m_strDefaultUserName;

    // Clear default user name, will be set if found in the list
    m_strDefaultUserName.clear();

    for(it = strFiles.begin(); it != strFiles.end(); ++it )
    {
        // Get file name
        strUserName = *it;

        // Extract user name string from it (keep what's at the left of the string '_user_profile.csl')
        strUserName = strUserName.section("_user_profile.csl", 0, 0);

        pItem = new QTreeWidgetItem(treeWidgetProfiles);
        pItem->setText(0, strUserName);

        if (strDefaultName == strUserName)
        {
            pItem->setCheckState(1, Qt::Checked);
            m_strDefaultUserName = strDefaultName;
            treeWidgetProfiles->setCurrentItem(pItem);
        }
        else
            pItem->setCheckState(1, Qt::Unchecked);

    }
}

void PickUserDialog::writeDefaultProfile(const QString &strDefaultProfile)
{
    write_private_profile_string("Startup", "profile", strDefaultProfile.toLatin1().constData(),
                                 GS::Gex::Engine::GetInstance().GetLocalConfigFileName().toLatin1().constData());
}

///////////////////////////////////////////////////////////
// Return username selected
///////////////////////////////////////////////////////////
void PickUserDialog::getSelectedUserName(QString &strUserName,QString &strProfileFile)
{
    QString strName = lineEditNewUser->text();
    if(strName.isEmpty() == false)
    {
        // Normalize string: change any non-alphanumerical value to underscore
        strName.replace(QRegExp("[^A-Za-z0-9-.]"), "_" );
    }
    else
    {
        // User selected? Then get its name!
        QTreeWidgetItem * pTreeSelectedItem = treeWidgetProfiles->currentItem();

        if (pTreeSelectedItem)
            strName = pTreeSelectedItem->text(0);
        else
            return;
    }

    // Get name selected (normalized string)
    strUserName = strName;

    // Build full file name from User name
    strProfileFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + "/";
    strProfileFile += strName + "_user_profile.csl";
}

///////////////////////////////////////////////////////////
// User entry selected (single click) in the list
///////////////////////////////////////////////////////////
void PickUserDialog::onItemClicked(void)
{
    // If empty list, nothing to edit!
    if(treeWidgetProfiles->topLevelItemCount() <= 0)
        return;

    // Save string to Edit box
    lineEditNewUser->setText(treeWidgetProfiles->currentItem()->text(0));
}

///////////////////////////////////////////////////////////
// User entry double-clicked.
///////////////////////////////////////////////////////////
void PickUserDialog::onItemDoubleClicked(void)
{
    // Copy selection into Editbox
    onItemClicked();

    // Close dialog box.
    done(1);
}

///////////////////////////////////////////////////////////
// User entry double-clicked.
///////////////////////////////////////////////////////////
void PickUserDialog::onItemChanged(QTreeWidgetItem * pItem, int nColumn)
{
    // Default user name has changed
    if (nColumn == 1)
    {
        // Get the state of the item
        int nState = pItem->checkState(nColumn);

        // state change into true, clear all other checkbox in the list
        if (nState)
        {
            for (int nItem = 0; nItem < treeWidgetProfiles->topLevelItemCount(); nItem++)
            {
                if (pItem != treeWidgetProfiles->topLevelItem(nItem))
                    treeWidgetProfiles->topLevelItem(nItem)->setCheckState(nColumn, Qt::Unchecked);
            }

            // Write new default user name
            writeDefaultProfile(pItem->text(0));
        }
        else
            // state change into false, delete the current user name
            writeDefaultProfile("");
    }
    treeWidgetProfiles->setCurrentItem(pItem);
}

///////////////////////////////////////////////////////////
// Rename selected entry
///////////////////////////////////////////////////////////
void PickUserDialog::OnProperties(void)
{
    // If empty list, ignore call!
    if(treeWidgetProfiles->topLevelItemCount() <= 0)
        return;

    bool ok;
    QString strNewName = QInputDialog::getText(this, "Rename Profile to...",
             "Rename to:", QLineEdit::Normal,
            QString::null, &ok);

    if(!ok || strNewName.isEmpty())
        return;	// Invalid selection

    // Get User name selected
    QString strUser;
    QString strOldProfileFile;
    getSelectedUserName(strUser,strOldProfileFile);

    // New name
    lineEditNewUser->setText(strNewName);
    QString strNewProfileFile;
    getSelectedUserName(strUser,strNewProfileFile);

    // Perform rename
    QDir cDir;
    cDir.rename(strOldProfileFile,strNewProfileFile);

    // delete default profile
    if (treeWidgetProfiles->currentItem()->checkState(1))
    {
        m_strDefaultUserName = strNewName;
        writeDefaultProfile(strNewName);
    }

    // Refresh list
    FillList();
}

///////////////////////////////////////////////////////////
// Delete selected entry
///////////////////////////////////////////////////////////
void PickUserDialog::OnRemoveUser(void)
{
    // If empty list, ignore call!
    if(treeWidgetProfiles->topLevelItemCount() <= 0)
        return;

    // Get item selected
    if (treeWidgetProfiles->currentItem() == NULL)
        return;

    // Get User name selected
    QString strUser;
    QString strProfileFile;
    getSelectedUserName(strUser,strProfileFile);

    // Request to confirm the 'delete' action
    QString strMessage = "Confirm to delete profile: '"+strUser+"' ?";
    bool lOk;
    GS::Gex::Message::request("Delete Profile", strMessage, lOk);
    if (! lOk)
    {
        return;
    }

    // Delete File
    QDir cDir;
    cDir.remove(strProfileFile);

    // delete default profile
    if (treeWidgetProfiles->currentItem()->checkState(1))
    {
        m_strDefaultUserName = "";
        writeDefaultProfile("");
    }

    // YIEMDMANDB
    // Have to delete profile from ym_user_profiles
    // Synchronize with YieldManDB profiles
    if(GS::Gex::Engine::GetInstance().IsAdminServerMode(true))
        GS::Gex::Engine::GetInstance().GetAdminEngine().DeleteProfiles(strUser);

    // Refresh list of users
    FillList();

}
