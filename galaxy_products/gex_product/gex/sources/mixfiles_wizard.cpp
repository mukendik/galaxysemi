///////////////////////////////////////////////////////////
// ALL Browser wizard pages for 'Merge+Compare files': TYPE 4 wizard
///////////////////////////////////////////////////////////

#include <QDragEnterEvent>
#include "mixfiles_wizard.h"
#include "browser_dialog.h"
#include "gex_constants.h"
#include "filter_dialog.h"
#include "getstring_dialog.h"
#include "import_all.h"
#include "report_build.h"
#include "settings_dialog.h"
#include "treewidgetitemcmp.h"
#include "report_options.h"
#include "product_info.h"
#include "common_widgets/conditions_widget.h"
#include "engine.h"
#include "message.h"
#include "csl/cslscriptparser.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

// in report_build.h
extern CReportOptions	ReportOptions;

// in script_wizard.h
extern void ConvertToScriptString(QString &strFile);
extern void ConvertFromScriptString(QString &strFile);

///////////////////////////////////////////////////////////
// WIZARD PAGE 1: CREATE GROUPS, SELECT STDF FILES
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_MixFile_Page1(void)
{
    // GCore-17482
    // Update the Wizard type before the OnDoInstantReport
    SetWizardType(GEX_MIXFILES_WIZARD);

    // Force Report mode = Standard reports (not Enterprise ones)
    pGexMainWindow->pWizardSettings->OnDoInstantReport();

    // Examinator-OEM doesn't allow this function
    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eSzOEM)
        return;

    // Show empty wizard page
    ShowWizardDialog(GEX_MIXFILES_WIZARD_P1);
}

///////////////////////////////////////////////////////////
// SETTINGS for ''MERGE+Compare files'
///////////////////////////////////////////////////////////
GexMixFileWizardPage1::GexMixFileWizardPage1( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setupUi(this);
    setModal(modal);

    move(0,0);

    QObject::connect(buttonAddFile,			SIGNAL(clicked()), this, SLOT(OnAddFiles()));
    QObject::connect(buttonRemoveFile,		SIGNAL(clicked()), this, SLOT(OnRemoveFiles()));
    QObject::connect(PushButtonClearAll,	SIGNAL(clicked()), this, SLOT(OnRemoveAll()));
    QObject::connect(buttonProperties,		SIGNAL(clicked()), this, SLOT(OnProperties()));
    QObject::connect(buttonAddFile_2,		SIGNAL(clicked()), this, SLOT(OnNewGroup()));
    QObject::connect(buttonTimePeriod,		SIGNAL(clicked()), this, SLOT(OnTimePeriod()));
    QObject::connect(buttonUp,				SIGNAL(clicked()), this, SLOT(OnMoveFileUp()));
    QObject::connect(buttonDown,			SIGNAL(clicked()), this, SLOT(OnMoveFileDown()));
    QObject::connect(buttonLoadDatasetList, SIGNAL(clicked()), this, SLOT(OnLoadDatasetList()));
    QObject::connect(buttonSaveDatasetList, SIGNAL(clicked()), this, SLOT(OnSaveDatasetList()));

    QObject::connect(treeWidget,			SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),	this,	SLOT(OnDoubleClick()));
    QObject::connect(treeWidget,			SIGNAL(customContextMenuRequested(const QPoint&)),	this,	SLOT(onContextualMenu(const QPoint&)));
    QObject::connect(treeWidget,			SIGNAL(itemChanged(QTreeWidgetItem*,int)),			this,	SLOT(onItemChanged(QTreeWidgetItem*,int)));

    // Prevent from sorting items.
    treeWidget->setSortingEnabled(false);

    // Adjust columns width
    treeWidget->resizeColumnToContents(0);
    treeWidget->resizeColumnToContents(1);
    treeWidget->resizeColumnToContents(2);
    treeWidget->resizeColumnToContents(3);
    treeWidget->resizeColumnToContents(4);
    treeWidget->resizeColumnToContents(5);
    treeWidget->setColumnWidth(6, 70);		// Set minimum size for the icon 'Map Test#' column

    // Until a file is selected, disable the 'Next',FileOpen,Properties,ClearClearAll button
    buttonSettings->setEnabled(false);
    buttonTimePeriod->hide();
    buttonAddFile->hide();
    buttonProperties->hide();
    Line1->hide();
    buttonRemoveFile->hide();
    PushButtonClearAll->hide();
    Line2->hide();
    buttonUp->hide();
    buttonDown->hide();
    buttonSaveDatasetList->hide();
    iDataSet=1;	// Suggests a group ID.

    // Support for Drag&Drop
    setAcceptDrops(true);

    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());
}

///////////////////////////////////////////////////////////
// GexMixFileWizardPage1: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::reject(void)
{
}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::ShowPage(void)
{
    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    // Make Widget visible.
    show();
}

///////////////////////////////////////////////////////////
// Update GUI fileds based on Product type running
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::UpdateSkin(int lProductID)
{
    // Enable/disable some features...
    switch(lProductID)
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:		// OEM-Examinator for LTXC
        case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
            buttonLoadDatasetList->hide();
            buttonSaveDatasetList->hide();
            bDataSetList = false;
            break;
        default:
            bDataSetList = true;
            break;
    }

    if (!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
    {
        m_poCompareGroupLabel->setEnabled(false);
        m_poCompareGroupCondition->setEnabled(false);
    }
    else
    {
        m_poCompareGroupLabel->setEnabled(true);
        m_poCompareGroupCondition->setEnabled(true);
    }
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::dropEvent(QDropEvent *e)
{
    if(!e->mimeData()->formats().contains("text/uri-list"))
    {
        // No files dropped...ignore drag&drop.
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

    // If only one file dragged over and it is a .CSL...then execute it to load the File List!
    if((strFileList.count() == 1) && (strFileList[0].endsWith(".csl", Qt::CaseInsensitive)))
    {
        ReadProcessFilesSection(strFileList[0]);
    }
    else
    {
        // Do not Allow Drag&Drop if no destination group exist
        if(treeWidget->topLevelItemCount() <= 0)
        {
            GS::Gex::Message::
                information("", "You must create a destination group first!");
            return;
        }

        // Do not Allow Drag&Drop if no destination group is selected
        if(treeWidget->topLevelItemCount() == 0)
        {
            GS::Gex::Message::information(
                "", "You must select the destination group first:\n"
                "Simply click on it to have it highlighted.");
            return;
        }

        // Insert files in selected group.
        OnAddFiles(strFileList);
    }

    e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Contextual menu: Select file
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::onContextualMenu(const QPoint& /*pos*/)
{
    int		iTotalSelection		= totalSelections();
    QMenu	menu(this);

    // Build menu.
    menu.addAction(*pixCreateFolder,"Create a new Group of files", this, SLOT(OnNewGroup()));
    menu.addSeparator();

    if(treeWidget->topLevelItemCount() >= 1)
        menu.addAction(*pixOpen,"Add files to a group...", this, SLOT(OnAddFiles()));

    if(iTotalSelection >= 1)
    {
        menu.addAction(*pixProperties,"Properties: File details and parts/sites to process", this, SLOT(OnProperties()));
        menu.addAction(*pixRemove,"Remove selected files/groups from list", this, SLOT(OnRemoveFiles()));
    }
    // If at least 2 files in the list...
    if(treeWidget->topLevelItemCount() >= 1)
    {
        menu.addSeparator();
        menu.addAction(*pixRemove,"Remove ALL files from list...", this, SLOT(OnRemoveAll()));
    }

    menu.setMouseTracking(true);
    menu.exec(QCursor::pos());
    menu.setMouseTracking(false);
}

///////////////////////////////////////////////////////////
// Returns total number of lines (files) selected
///////////////////////////////////////////////////////////
int GexMixFileWizardPage1::totalSelections(void)
{
    QTreeWidgetItem *	pTreeWidgetItem = treeWidget->topLevelItem(0);
    int					iTotal			= 0;

    while(pTreeWidgetItem != NULL)
    {
        // Find first object selected
        if(pTreeWidgetItem->isSelected() == true)
            iTotal++;

        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    // return toal of selections found
    return iTotal;
}

///////////////////////////////////////////////////////////
// Adding file(s) to the list
///////////////////////////////////////////////////////////
void
GexMixFileWizardPage1::OnAddFiles(QStringList sFilesSelected,
                                  bool bClearList /* = false */,
                                  QString /*strNewGroup*/,
                                  QString strNewSite,
                                  QString strNewParts,
                                  QString strNewRange,
                                  QString strNewMapTests,
                                  QString strTemperature,
                                  QString strNewWaferId)
{
    // Check if at list on file to insert
    if(sFilesSelected.count() <= 0)
        return;

    // Check if clear list first
    if(bClearList)
        OnRemoveAll();

    // If files selected, add them to the current group.
    QTreeWidgetItem * pTreeWidgetItemSelection	= treeWidget->currentItem();

    if(pTreeWidgetItemSelection == NULL)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // If selection is on a file, we make sure
    // we do not create a node under the other file names, but stay under the group name!
    if(pTreeWidgetItemSelection->parent() != NULL)
        pTreeWidgetItemSelection = pTreeWidgetItemSelection->parent();	// Behave 'as-if' selection was on the group name rather than on a file 'leaf'

    QString strFile, strParts,strSites,strRange,strMapTests,strWaferId;

    if(strNewParts.isEmpty())
        strParts = ProcessPartsItems[0];
    else
        strParts = strNewParts;
    strParts += " ";

    if(strNewSite.isEmpty())
        strSites = "All ";
    else
        strSites = strNewSite;

    if(strNewRange.isEmpty())
        strRange = " ";
    else
        strRange = strNewRange;

    if(strNewMapTests.isEmpty())
        strMapTests = " ";
    else
        strMapTests = strNewMapTests;

    if (strNewWaferId.isEmpty())
        strWaferId = "";
    else
        strWaferId = strNewWaferId;

    // Read the list of files selected, and add them to the list box.
    QTreeWidgetItem *	pTreeWidgetItem		= NULL;
    bool				bTemplateSelected	= false;

    for (QStringList::Iterator it = sFilesSelected.begin(); it != sFilesSelected.end(); ++it )
    {
        // Check if file name forced
        strFile = *it;

        // If one of the file is a Template file, make it selected into the 'Settings page
        if(strFile.endsWith(".grt",Qt::CaseInsensitive) && pGexMainWindow->pWizardSettings != NULL)
        {
            pGexMainWindow->pWizardSettings->OnSelectReportTemplate(strFile);
            bTemplateSelected = true;
            goto next_file;
        }

        pTreeWidgetItem = new QTreeWidgetItem();

        pTreeWidgetItemSelection->addChild(pTreeWidgetItem);

        pTreeWidgetItem->setText(0, " ");
        pTreeWidgetItem->setText(1, strSites);
        pTreeWidgetItem->setText(2, strParts);
        pTreeWidgetItem->setText(3, strRange);
        pTreeWidgetItem->setText(4, strTemperature);
        pTreeWidgetItem->setText(5, strFile);
        pTreeWidgetItem->setText(6, strMapTests);
        pTreeWidgetItem->setText(7, strWaferId);

        // Saves latest file selected...so next call start from there.
        strWorkingPath = strFile;
next_file:;
    }

    // Selects last file insterted as new default selection.
    if(pTreeWidgetItem != NULL)
        treeWidget->setCurrentItem(pTreeWidgetItem);

    // Adjust columns width to the contents
    treeWidget->resizeColumnToContents(0);
    treeWidget->resizeColumnToContents(1);
    treeWidget->resizeColumnToContents(2);
    treeWidget->resizeColumnToContents(3);
    treeWidget->resizeColumnToContents(4);
    treeWidget->resizeColumnToContents(5);
    treeWidget->resizeColumnToContents(6);
    treeWidget->resizeColumnToContents(7);

    // Make sure the Group node is opened (so we can see the selection made !
    pTreeWidgetItemSelection->setExpanded(true);

    // File(s) selected, enable the 'Next',Properties button
    buttonSettings->setEnabled(true);
    buttonProperties->show();
    Line1->show();
    buttonUp->show();
    buttonDown->show();

    if(bDataSetList)
        buttonSaveDatasetList->show();

    // If one of the files seledcted is a template file, then switch to the next page 'Settings' tab
    if(bTemplateSelected)
    {
        // Go to the Settings page
        pGexMainWindow->Wizard_Settings();
    }
}

///////////////////////////////////////////////////////////
// Function name	: GexMixFileWizardPage1::OnAddFiles
// Description	    : Add files to the current group selection
// Return type		: void
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnAddFiles(void)
{
    GS::Gex::SelectDataFiles	cSelectFiles;
    QStringList sFilesSelected = cSelectFiles.GetFiles(this,strWorkingPath,"Select STDF Files to merge in selected group");

    if(sFilesSelected.isEmpty() == true)
        return;	// Empty list...ignore task!

    // Add selected files to list box
    OnAddFiles(sFilesSelected);
}

///////////////////////////////////////////////////////////
// Adding file(s) matching a time period + Filter criteria to the list
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnTimePeriod(void)
{
    // Dispplay Calendar based Import dialog box
    if(TimePeriodFiles.exec() != 1)
        return;	// User canceled dialog box.

    // If files selected, add them to the current group.
    QTreeWidgetItem * pTreeWidgetItemSelection = treeWidget->currentItem();

    if(pTreeWidgetItemSelection == NULL)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // If selection is on a file, we make sure
    // we do not create a node under the other file names, but stay under the group name!
    if(pTreeWidgetItemSelection->parent() != NULL)
        pTreeWidgetItemSelection = pTreeWidgetItemSelection->parent();	// Behave 'as-if' selection was on the group name rather than on a file 'leaf'

    QString strParts = ProcessPartsItems[0];
    strParts += " ";

    // Read the list of files selected, and add them to the list box.
    QTreeWidgetItem * pTreeWidgetItem = NULL;
    for (QStringList::Iterator it = TimePeriodFiles.ImportFile.begin(); it != TimePeriodFiles.ImportFile.end(); ++it )
    {
        pTreeWidgetItem = new QTreeWidgetItem(pTreeWidgetItemSelection);

        pTreeWidgetItemSelection->addChild(pTreeWidgetItem);

        pTreeWidgetItem->setText(0, " ");
        pTreeWidgetItem->setText(1, "All ");
        pTreeWidgetItem->setText(2, strParts);
        pTreeWidgetItem->setText(3, " ");
        pTreeWidgetItem->setText(3, " ");
        pTreeWidgetItem->setText(5, (*it).toLatin1().constData());
        pTreeWidgetItem->setText(6, " ");

        // Saves latest file selected...so next call start from there.
        strWorkingPath = (*it);
    }

    // Free some memory: empty intermediate Import list
    TimePeriodFiles.ImportFile.clear();

    // Selects last file insterted as new default selection.
    if(pTreeWidgetItem != NULL)
        treeWidget->setCurrentItem(pTreeWidgetItem);

    // Adjust columns width to the contents
    treeWidget->resizeColumnToContents(0);
    treeWidget->resizeColumnToContents(1);
    treeWidget->resizeColumnToContents(2);
    treeWidget->resizeColumnToContents(3);
    treeWidget->resizeColumnToContents(4);
    treeWidget->resizeColumnToContents(5);
    treeWidget->resizeColumnToContents(6);

    // Make sure the Group node is opened (so we can see the selection made !
    pTreeWidgetItemSelection->setExpanded(true);

    // File(s) selected, enable the 'Next',Properties button
    buttonSettings->setEnabled(true);
    buttonProperties->show();
    Line1->show();
    buttonUp->show();
    buttonDown->show();

    if(bDataSetList)
        buttonSaveDatasetList->show();
}

///////////////////////////////////////////////////////////
// Move file UP in the list
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnMoveFileUp(void)
{
    QTreeWidgetItem * pTreeWidgetItemSelection = treeWidget->currentItem();

    // If no selection, just ignore action.
    if(pTreeWidgetItemSelection == NULL)
        return;

    // Check if more than one file selected
    QTreeWidgetItem * pTreeWidgetItem			= NULL;
    QTreeWidgetItem * pTreeWidgetPreviousItem	=NULL;

    pTreeWidgetItem = treeWidget->topLevelItem(0);

    int	iSelectedItems	= 0;
    int iMoveGroup		= 0;

    while(pTreeWidgetItem != NULL)
    {
        // check if item selected...
        if(pTreeWidgetItem->isSelected() == true)
        {
            // If selection is a group, ignore it ; otherwise update counter!
            if(pTreeWidgetItem->text(5).isEmpty() == false)
                iSelectedItems++;
            else
                iMoveGroup++;	// Flag we've got to move group(s) up
        }
        // Keep track of 'previous' file in list
        pTreeWidgetPreviousItem = pTreeWidgetItem;

        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    // If no valid selection or more than one selection...ignore!
    if(iMoveGroup > 1)
    {
        GS::Gex::Message::
            information("", "You can only move one group at a time...");
        return;
    }

    // If more than one file selected in group...ignore!
    if(iSelectedItems > 1 && (iMoveGroup == 0))
    {
        GS::Gex::Message::
            information("", "You can only move one file at a time...");
        return;
    }

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Moving group up
    QTreeWidgetItemCmp *	pTreeWidgetItemGroup		= NULL;
    QTreeWidgetItemCmp *	pTreeWidgetItemPrevGroup	= NULL;
    int						nPreviousItem				= 0;

    if(iMoveGroup)
    {
        QList<QTreeWidgetItemCmp*> listGroups;
        int	iSelection=0;

        // Move group up in group list
        pTreeWidgetItemGroup = static_cast<QTreeWidgetItemCmp *>(treeWidget->topLevelItem(0));
        while(pTreeWidgetItemGroup != NULL)
        {
            if (pTreeWidgetItemGroup->parent() == NULL)
            {
                listGroups.append(pTreeWidgetItemGroup);

                if(pTreeWidgetItemGroup->isSelected() == true)
                    iSelection = listGroups.count()-1;
            }

            // Move to next item
            pTreeWidgetItemGroup = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(pTreeWidgetItemGroup));
        };

        // If group already first in list, nothing to do!
        if(iSelection == 0)
            return;

        // Move group up!
        pTreeWidgetItemGroup		= listGroups.at(iSelection);
        pTreeWidgetItemPrevGroup	= listGroups.at(iSelection-1);

        // Get position of the previous group
        nPreviousItem = treeWidget->indexOfTopLevelItem(pTreeWidgetItemPrevGroup);

        // Keep the status of the item we are going to move
        bool bIsExpanded = pTreeWidgetItemGroup->isExpanded();

        // Remove current group from the treewidget and insert it at its new position
        treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItemGroup));
        treeWidget->insertTopLevelItem(nPreviousItem, pTreeWidgetItemGroup);

        // Set the old status
        pTreeWidgetItemGroup->setExpanded(bIsExpanded);

        // Set the item as selected
        pTreeWidgetItemGroup->setSelected(true);

        // Update the group name for both groups
        pTreeWidgetItemGroup->updateGroupName();
        pTreeWidgetItemPrevGroup->updateGroupName();

        return;
    }

    // Moving file up in group.
    pTreeWidgetItem			= treeWidget->topLevelItem(0);
    pTreeWidgetPreviousItem	= NULL;

    while(pTreeWidgetItem != NULL)
    {
        // Keep track of position in group...
        // and reset file offset in group each time we reach a group
        if(pTreeWidgetItem->text(5).isEmpty() == true)
        {
            pTreeWidgetItemGroup	= static_cast<QTreeWidgetItemCmp *>(pTreeWidgetItem);
            pTreeWidgetPreviousItem = NULL;
        }
        else if(pTreeWidgetItem->isSelected() == true)					// check if item selected...if selected, can only be a file...not a group.
        {
            // Move this selection UP!...unless it's the 1st in group...
            // if 1st file selected...can't move up..ignore!
            if(pTreeWidgetPreviousItem)
            {
                // Get position of the previous group
                nPreviousItem = pTreeWidgetItemGroup->indexOfChild(pTreeWidgetPreviousItem);

                // Remove current group from the treewidget and insert it at its new position
                pTreeWidgetItemGroup->takeChild(pTreeWidgetItemGroup->indexOfChild(pTreeWidgetItem));
                pTreeWidgetItemGroup->insertChild(nPreviousItem, pTreeWidgetItem);

                pTreeWidgetItem->setSelected(true);
            }

            return;
        }
        else		// Keep track of previous item seen.
            pTreeWidgetPreviousItem = pTreeWidgetItem;

        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };
}

///////////////////////////////////////////////////////////
// Move file DOWN in the list
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnMoveFileDown(void)
{
    QTreeWidgetItem	* pTreeWidgetItemSelection = treeWidget->currentItem();

    // If no selection, just ignore action.
    if (pTreeWidgetItemSelection == NULL)
        return;

    // Check if more than one file selected
    QTreeWidgetItem * pTreeWidgetItem		= treeWidget->topLevelItem(0);
    QTreeWidgetItem * pTreeWidgetNextItem	= NULL;

    int	iSelectedItems	= 0;
    int iMoveGroup		= 0;

    while(pTreeWidgetItem != NULL)
    {
        // check if item selected...
        if(pTreeWidgetItem->isSelected() == true)
        {
            // If selection is a group, ignore it ; otherwise update counter!
            if(pTreeWidgetItem->text(5).isEmpty() == false)
                iSelectedItems++;
            else
                iMoveGroup++;	// Flag we've got to move group(s) up
        }
        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    // If no valid selection or more than one selection...ignore!
    if(iMoveGroup > 1)
    {
        GS::Gex::Message::
            information("", "You can only move one group at a time...");
        return;
    }

    // If more than one file selected in group...ignore!
    if(iSelectedItems > 1 && (iMoveGroup == 0))
    {
        GS::Gex::Message::
            information("", "You can only move one file at a time...");
        return;
    }

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Moving group down
    QTreeWidgetItemCmp * pTreeWidgetItemGroup		= NULL;
    QTreeWidgetItemCmp * pTreeWidgetItemNextGroup	= NULL;
    int					nPreviousItem				= 0;

    if(iMoveGroup)
    {
        QList <QTreeWidgetItemCmp*> listGroups;
        int	iSelection=0;

        // Move group up in group list
        pTreeWidgetItemGroup = static_cast<QTreeWidgetItemCmp *>(treeWidget->topLevelItem(0));
        while(pTreeWidgetItemGroup != NULL)
        {
            if (pTreeWidgetItemGroup->parent() == NULL)
            {
                listGroups.append(pTreeWidgetItemGroup);

                if(pTreeWidgetItemGroup->isSelected() == true)
                    iSelection = listGroups.count()-1;
            }

            // Move to next item
            pTreeWidgetItemGroup = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(pTreeWidgetItemGroup));
        };

        // If group already last in list, nothing to do!
        if(iSelection == (int)listGroups.count()-1)
            return;

        // Move group down!
        pTreeWidgetItemGroup		= listGroups.at(iSelection);
        pTreeWidgetItemNextGroup	= listGroups.at(iSelection+1);

        // Get position of the previous group
        nPreviousItem = treeWidget->indexOfTopLevelItem(pTreeWidgetItemGroup);

        // Keep the status of the item we are going to move
        bool bIsExpanded = pTreeWidgetItemNextGroup->isExpanded();

        // Remove current group from the treewidget and insert it at its new position
        treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItemNextGroup));
        treeWidget->insertTopLevelItem(nPreviousItem, pTreeWidgetItemNextGroup);

        // Set the old status
        pTreeWidgetItemNextGroup->setExpanded(bIsExpanded);

        // Update the group name for both groups
        pTreeWidgetItemGroup->updateGroupName();
        pTreeWidgetItemNextGroup->updateGroupName();

        return;
    }

    // Moving file down in group.
    pTreeWidgetItem		= treeWidget->topLevelItem(0);
    pTreeWidgetNextItem	= NULL;

    while(pTreeWidgetItem != NULL)
    {
        // Move to next item
        pTreeWidgetNextItem = treeWidget->itemBelow(pTreeWidgetItem);

        // Keep track of position in group...
        // and reset file offset in group each time we reach a group
        if(pTreeWidgetItem->text(5).isEmpty() == true)
            pTreeWidgetItemGroup = static_cast<QTreeWidgetItemCmp *>(pTreeWidgetItem);
        else if(pTreeWidgetItem->isSelected() == true)					// check if item selected...if selected, can only be a file...not a group.
        {
            // Move this selection DOWN!...unless it's the last in group...
            // if last file selected...can't move down..ignore!
            if(pTreeWidgetNextItem && pTreeWidgetNextItem->text(5).isEmpty() == false)
            {
                // Get position of the current item
                nPreviousItem = pTreeWidgetItemGroup->indexOfChild(pTreeWidgetItem);

                // Remove current item from the treewidget and insert it at its new position
                pTreeWidgetItemGroup->takeChild(pTreeWidgetItemGroup->indexOfChild(pTreeWidgetNextItem));
                pTreeWidgetItemGroup->insertChild(nPreviousItem, pTreeWidgetNextItem);
            }

            return;
        }

        // Move to next item
        pTreeWidgetItem = pTreeWidgetNextItem;
    };
}

///////////////////////////////////////////////////////////
// Function name	: GexMixFileWizardPage1::OnNewGroup
// Description	    : GUI to Creates a new group in the STDF list
// Return type		: void
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnNewGroup(void)
{
    char	szString[1024];
    QString	strGroupName;

    GetStringDialog cNewGroup(0, true, Qt::MSWindowsFixedSizeDialogHint);

    // Suggest a new dataset name
    strcpy(szString, nextDataSetName().toLatin1().constData());

    cNewGroup.setDialogText("Create New group of files",
        "Group name:",
        szString,
        "Enter the group name to create");
    cNewGroup.setProhibatedList(getGroupNameList(true));
    ConditionsWidget *poConditions = 0;
    if(!m_poCompareGroupCondition->text().isEmpty()){
        //Add conditions
        QStringList strListCondition = m_poCompareGroupCondition->text().split(QRegExp("[;,]"),QString::SkipEmptyParts);
        poConditions = new ConditionsWidget(strListCondition, &cNewGroup);
        cNewGroup.addConditions(poConditions);
    }

    // Display Edit window
    if(cNewGroup.exec() == 0)
        return;	// User canceled task.

    // Get string entered if any.
    cNewGroup.getString(szString);
    strGroupName = szString;

    if(strGroupName.isEmpty())
        return;	// Empty string...ignore task

    // Make sure string doesn't include characters that would corrupt script execution
    strGroupName.replace('\\',' ');	// Replace '\\' char with a space
    strGroupName.replace('\"',' ');	// Replace '"' char with a space

    CreateNewGroup(strGroupName);

    if(poConditions){
        QMap<QString, QVariant > oConditionMap = poConditions->getConditionsValues();
        m_oGroupConditions.insert(treeWidget->currentItem()->text(0), oConditionMap);
    }
}

///////////////////////////////////////////////////////////
// Function name	: GexMixFileWizardPage1::CreateNewGroup
// Description	    : Code to create a new group in the STDF list
// Return type		: void
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::CreateNewGroup(QString strGroupName)
{
    QTreeWidgetItemCmp * pTreeWidgetItemGroup = NULL;

    pTreeWidgetItemGroup = new QTreeWidgetItemCmp(treeWidget);
    pTreeWidgetItemGroup->setText(0, strGroupName);

    // Sets it as the default selection
    treeWidget->setCurrentItem(pTreeWidgetItemGroup);

    // Increment counter so next suggested ID is different!
    iDataSet++;

    // At least 1 group, enable 'file open',Remove,RemoveAll
    buttonTimePeriod->show();
    buttonAddFile->show();
    buttonProperties->show();
    Line1->show();
    buttonRemoveFile->show();
    PushButtonClearAll->show();
}

///////////////////////////////////////////////////////////
// Function name	: GexMixFileWizardPage1::OnRemoveFiles
// Description	    : Remove selected file to the Cluster1 list.
// Return type		: void
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnRemoveFiles(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 0)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Scan the list to remove selected items.
    QTreeWidgetItem * pTreeWidgetItemGroup		= NULL;
    QTreeWidgetItem * pTreeWidgetItemNextGroup	= NULL;
    QTreeWidgetItem * pTreeWidgetItemFile		= NULL;
    QTreeWidgetItem * pTreeWidgetItemNextFile	= NULL;

    bool	bEraseGroup;	// true if files in group have to be erase
    int		nCurrentFileIndex	= -1;
    int		nNextFileIndex		= -1;

    pTreeWidgetItemGroup = treeWidget->topLevelItem(0);	// first group in list

    while(pTreeWidgetItemGroup != NULL)
    {
        // If group is selected, will have to erase all files (event if they are not selected)
        if(pTreeWidgetItemGroup->isSelected() == true)
            bEraseGroup = true;	// Forces to erase all files ni group, even if not selected.
        else
            bEraseGroup = false;	// Only erase files in this group that are selected.

        // Always Erase files in group before group

        // Remove all files listed in this group.
        pTreeWidgetItemFile = pTreeWidgetItemGroup->child(0);
        while(pTreeWidgetItemFile != NULL)
        {
            // Saves pointer to next file before erasing this one.
            nCurrentFileIndex	= pTreeWidgetItemGroup->indexOfChild(pTreeWidgetItemFile);
            nNextFileIndex		= nCurrentFileIndex + 1;

            if (nNextFileIndex >= 0 && nNextFileIndex < pTreeWidgetItemGroup->childCount())
                pTreeWidgetItemNextFile = pTreeWidgetItemGroup->child(nNextFileIndex);
            else
                pTreeWidgetItemNextFile = NULL;

            // Remove file from list if selected, or its group name was selected
            if((pTreeWidgetItemFile->isSelected() == true) || (bEraseGroup==true))
            {
                pTreeWidgetItemGroup->takeChild(nCurrentFileIndex);
                delete pTreeWidgetItemFile;
            }

            // Move to next file
            pTreeWidgetItemFile = pTreeWidgetItemNextFile;
        };

        // Saves pointer to next group before erasing this one.
        pTreeWidgetItemNextGroup = treeWidget->itemBelow(pTreeWidgetItemGroup);

        if(bEraseGroup == true)
        {
            if(m_oGroupConditions.contains(pTreeWidgetItemGroup->text(0)))
                m_oGroupConditions.remove(pTreeWidgetItemGroup->text(0));
            // Remove group from list
            treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItemGroup));

            // Destroy it from memory.
            delete pTreeWidgetItemGroup;

            if (pTreeWidgetItemNextGroup){
                QString strGroupNameBeforeUpdate = pTreeWidgetItemNextGroup->text(0);
                ((QTreeWidgetItemCmp*)pTreeWidgetItemNextGroup)->updateGroupName();
                if(m_oGroupConditions.contains(strGroupNameBeforeUpdate)&& strGroupNameBeforeUpdate != pTreeWidgetItemNextGroup->text(0)){
                    QMap<QString, QVariant> oNewConditionKey = m_oGroupConditions[strGroupNameBeforeUpdate];
                    m_oGroupConditions.remove(strGroupNameBeforeUpdate);
                    m_oGroupConditions.insert(pTreeWidgetItemNextGroup->text(0), oNewConditionKey);
                }
            }
        }

        // Move to next group
        pTreeWidgetItemGroup = pTreeWidgetItemNextGroup;
    };

    // If the list is empty, disable the 'Next' FileOpen,Properties,Clear,ClearAll buttons
    if(treeWidget->topLevelItemCount() <= 0)
    {
        buttonSettings->setEnabled(false);
        buttonTimePeriod->hide();
        buttonAddFile->hide();
        buttonProperties->hide();
        Line1->hide();
        buttonRemoveFile->hide();
        PushButtonClearAll->hide();
        Line2->hide();
        buttonUp->hide();
        buttonDown->hide();
        buttonSaveDatasetList->hide();
        m_poCompareGroupCondition->clear();
        m_oGroupConditions.clear();
    }

}

///////////////////////////////////////////////////////////
// Function name	: GexMixFileWizardPage1::OnRemoveAll
// Description	    : Remove all list.
// Return type		: void
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnRemoveAll(void)
{
    // Select all entries
    treeWidget->selectAll();

    // Remove selected entries!
    OnRemoveFiles();
}

///////////////////////////////////////////////////////////
// Function name	: GexMixFileWizardPage1::OnDoubleClick
// Description	    : User double cicked an entry in the list
// Return type		: void
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnDoubleClick(void)
{
    // Check if double clicked a file or group?
    QTreeWidgetItem * pTreeWidgetItemSelection = treeWidget->currentItem();

    if(pTreeWidgetItemSelection == NULL)
        return;

    // Case 5543: DOuble click opens properties for both files and groups.
    //	if(pTreeWidgetItemSelection->text(5).isEmpty())
//		return;

    // We've double-clicked on a file, then show its proerties
    OnProperties();
}

///////////////////////////////////////////////////////////
// Function name	: GexMixFileWizardPage1::OnProperties
// Description	    : Edit file properties: site+parts to process
// Return type		: void
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnProperties(void)
{
    QTreeWidgetItem * pTreeWidgetItemSelection = treeWidget->currentItem();

    // If no selection, just ignore action.
    if(pTreeWidgetItemSelection == NULL)
        return;

    // Check if more than one file selected
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

    int		iSelectedItems	= 0;
    bool	bFileSelected	= false;
    bool	bGroupSelected	= false;

    while(pTreeWidgetItem != NULL)
    {
        // check if item selected...
        if(pTreeWidgetItem->isSelected() == true)
        {
            iSelectedItems++;

            // If selection is a group, properties allows to rename the group name.
            if(pTreeWidgetItem->text(5).isEmpty() == false)
                bFileSelected = true;
            else
                bGroupSelected = true;
        }
        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    // If no valid selection, ignore!
    if(iSelectedItems <= 0)
        return;

    // If at least one file belongs to the selection, only show Files properties (even if a group name is selected)
    if(bFileSelected)
    {
        // Show selection made...
        FilterDialog *dFilter = new FilterDialog();

        // Give to the filter dialog the file info: file name, site#,etc...
        dFilter->SetParameters(iSelectedItems,
                               false,
                               1,
                               pTreeWidgetItemSelection);
        dFilter->SetWaferIdFilter(pTreeWidgetItemSelection->text(7));		// case 3935

        // Prompt Filter dialog box
        if(dFilter->exec() != 1)
            return;	// User 'Abort'

        // Reset HTML sections to create flag: ALL pages to create.
        pGexMainWindow->iHtmlSectionsToSkip = 0;
        pGexMainWindow->m_bDatasetChanged	= true;

        // If more than one selection....copy properties changes (part filter) to all other selections
        pTreeWidgetItem = treeWidget->topLevelItem(0);
        while(pTreeWidgetItem != NULL)
        {
            // check if item selected...if so update filter changes
            if(pTreeWidgetItem->isSelected() == true)
            {
                // Update site/part/range filter...except for selections that are group names!
                if(pTreeWidgetItem->text(5).isEmpty() == false)
                {
                    pTreeWidgetItem->setText(1, pTreeWidgetItemSelection->text(1));	// Site#
                    pTreeWidgetItem->setText(2, pTreeWidgetItemSelection->text(2));	// Parts
                    pTreeWidgetItem->setText(3, pTreeWidgetItemSelection->text(3));	// Range
                    pTreeWidgetItem->setText(4, pTreeWidgetItemSelection->text(4));	// Temperature
                    pTreeWidgetItem->setText(6, pTreeWidgetItemSelection->text(6));	// Test # mapping
                    pTreeWidgetItem->setText(7, dFilter->GetWaferIdFilter());		// WaferId Filter
                }
            }

            // Move to next item
            pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
        };
    }
    else if(bGroupSelected)
    {
        // A group was selected...edit its name!
        QString	strGroupName;
        char	szString[1024];

        // Get pointer to group selected
        QTreeWidgetItemCmp * pTreeWidgetItemCmp = static_cast<QTreeWidgetItemCmp *> (treeWidget->topLevelItem(0));

        while(pTreeWidgetItemCmp != NULL)
        {
            // check if item selected...
            if((pTreeWidgetItemCmp->isSelected() == true) && (pTreeWidgetItemCmp->text(5).isEmpty() == true))
                break;

            // Move to next item
            pTreeWidgetItemCmp = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(pTreeWidgetItemCmp));
        };

        GetStringDialog cEditGroup(0, true, Qt::MSWindowsFixedSizeDialogHint);

        // Get current group name
        strcpy(szString, pTreeWidgetItemCmp->groupBase().toLatin1().constData());

        cEditGroup.setDialogText("Rename group name",
            "Group name:",
            szString,
            "Edit the group name");
        cEditGroup.setProhibatedList(getGroupNameList(false));

        ConditionsWidget *poConditions = 0;
        QMap<QString, QVariant> oConditionMap ;
        QString strGuiGroupName = pTreeWidgetItemCmp->groupBase();
        if(!m_poCompareGroupCondition->text().isEmpty())
        {
            QStringList strListCondition = m_poCompareGroupCondition->text().split(QRegExp("[;,]"),QString::SkipEmptyParts);

            if(m_oGroupConditions.contains(strGuiGroupName))
            {
                oConditionMap = m_oGroupConditions[strGuiGroupName];
                if (oConditionMap.count())
                    poConditions = new ConditionsWidget(oConditionMap, &cEditGroup, strListCondition);
                else
                    poConditions = new ConditionsWidget(strListCondition, &cEditGroup);
                m_oGroupConditions.remove(strGuiGroupName);
            }
            else
                poConditions = new ConditionsWidget(strListCondition, &cEditGroup);

            cEditGroup.addConditions(poConditions);
        }

        // Display Edit window
        if(cEditGroup.exec() == 0)
        {
            m_oGroupConditions.insert(strGuiGroupName, oConditionMap);
            return;	// User canceled task.
        }

        // Get string entered if any.
        cEditGroup.getString(szString);
        strGroupName = szString;

        if(strGroupName.isEmpty())
            return;	// Empty string...ignore rename

        // Make sure string doesn't include characters that would corrupt script execution
        strGroupName.replace('\\',' ');	// Replace '\\' char with a space
        strGroupName.replace('\"',' ');	// Replace '"' char with a space

        // Update group name in the list
        pTreeWidgetItemCmp->setText(0,strGroupName);
        if(poConditions){
            oConditionMap.clear();
            oConditionMap = poConditions->getConditionsValues();
            m_oGroupConditions.insert(pTreeWidgetItemCmp->groupBase(), oConditionMap);
        }
    }
}

///////////////////////////////////////////////////////////
// Reload ListView with list of files from disk
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnLoadDatasetList(void)
{
    // Let's user pick the File List to load
    strDatasetListFile = QFileDialog::getOpenFileName(this, "Load a File List...", strDatasetListFile, "File List / Script (*.csl)");

    // If no file selected, ignore command.
    if(strDatasetListFile.isEmpty())
        return;

    // Read section & load it in the list...
    ReadProcessFilesSection(strDatasetListFile);
}

///////////////////////////////////////////////////////////
// Save ListView into a ListFile on disk
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::OnSaveDatasetList(void)
{
    // Let's user tell where to save the File List.
//    strDatasetListFile = QFileDialog::getSaveFileName(
//                strDatasetListFile,
//                "File List / Script (*.csl)",
//                this,
//                "save file dialog",
//                "Save File List to..." );
    strDatasetListFile = QFileDialog::getSaveFileName(this,
                                                      "Save File List to...",
                                                      strDatasetListFile,
                                                      "File List / Script (*.csl)");

    // If no file selected, ignore command.
    if(strDatasetListFile.isEmpty())
        return;

    // Make sure file name ends with ".csl" extension
    if(strDatasetListFile.endsWith(".csl", Qt::CaseInsensitive) == false)
        strDatasetListFile += ".csl";

    FILE *hFile;
    hFile = fopen(strDatasetListFile.toLatin1().constData(),"w");
    if(hFile == NULL)
    {
        GS::Gex::Message::
            critical("", "Failed creating file...folder is write protected?");
        return;
    }

    fprintf(hFile,"<gex_template>\n");
    fprintf(hFile,"<BlockType = compare_groups>\n\n");
    WriteProcessFilesSection(hFile);
    fprintf(hFile,"\n</gex_template>\n");
    fclose(hFile);
}

///////////////////////////////////////////////////////////
// Script function to define Data file groups
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::WriteProcessFilesSection(FILE *hFile)
{
    QTreeWidgetItem *	pTreeWidgetItemGroup	= NULL;
    QTreeWidgetItem *	pTreeWidgetItemFile	= NULL;
    QString				strSite,strParts,strRange,strFile,strGroup,strMapTests,strTemperature, strWaferIdFilter;
    int					nNextFileIndex = 0;

    fprintf(hFile,"  // Compare groups of files...\n");
    pTreeWidgetItemGroup = treeWidget->topLevelItem(0);
    while(pTreeWidgetItemGroup != NULL)
    {
        bool bForceComputeFromSamples = false;
        if (pTreeWidgetItemGroup->text(5).isEmpty() == true)
        {
            pTreeWidgetItemFile = pTreeWidgetItemGroup->child(0);
            int nNextFileIndex = 0;
            while(pTreeWidgetItemFile != NULL)
            {
                strParts		= pTreeWidgetItemFile->text(2);
                strParts		= strParts.trimmed();
                int i=0;
                while(ProcessPartsItems[i] != 0)
                {
                    if( strParts == ProcessPartsItems[i])
                        break;	// found matching string
                    i++;
                };
                bForceComputeFromSamples = pGexMainWindow->forceToComputeFromDataSample(hFile, i);
                if(bForceComputeFromSamples)
                    break;
                nNextFileIndex = pTreeWidgetItemGroup->indexOfChild(pTreeWidgetItemFile) + 1;

                if (nNextFileIndex >= 0 && nNextFileIndex< pTreeWidgetItemGroup->childCount())
                    pTreeWidgetItemFile = pTreeWidgetItemGroup->child(nNextFileIndex);
                else
                    pTreeWidgetItemFile = NULL;
            }
        }

        if(bForceComputeFromSamples)
            break;
        pTreeWidgetItemGroup = treeWidget->itemBelow(pTreeWidgetItemGroup);
    }

    fprintf(hFile,"  gexGroup('reset','all');\n");

    if(!m_poCompareGroupCondition->text().isEmpty()){
         QStringList oOrdredCondition = m_poCompareGroupCondition->text().split(QRegExp("[;,]"),QString::SkipEmptyParts);
         foreach(const QString &strCondition, oOrdredCondition){
             fprintf(hFile,"  gexGroup('declare_condition','%s');\n",strCondition.toLatin1().constData());
         }

     }
    // Report title (used in report by applications like yield123)
    fprintf(hFile,"  gexQuery('db_report','DataSet_4');\n");

    // Loop on ALL groups
    pTreeWidgetItemGroup = treeWidget->topLevelItem(0);

    while(pTreeWidgetItemGroup != NULL)
    {
        if (pTreeWidgetItemGroup->text(5).isEmpty() == true)
        {
            strGroup = pTreeWidgetItemGroup->text(0);	// Group name

            fprintf(hFile,"\n  // Create group...\n");
            fprintf(hFile,"  group_id = gexGroup('insert','%s');\n",strGroup.toLatin1().constData());

            // Loop on all files in group
            pTreeWidgetItemFile = pTreeWidgetItemGroup->child(0);
            while(pTreeWidgetItemFile != NULL)
            {
                strSite			= pTreeWidgetItemFile->text(1);
                strSite			= strSite.trimmed();	// remove leading spaces.
                strParts		= pTreeWidgetItemFile->text(2);
                strParts		= strParts.trimmed();	// remove leading spaces.
                strRange		= pTreeWidgetItemFile->text(3);
                strTemperature	= pTreeWidgetItemFile->text(4);
                strFile			= pTreeWidgetItemFile->text(5);
                strMapTests		= pTreeWidgetItemFile->text(6);
                strMapTests		= strMapTests.trimmed();	// remove leading spaces.
                strWaferIdFilter = pTreeWidgetItemFile->text(7);

                // Convert file path to be script compatible '\' become '\\'
                ConvertToScriptString(strFile);
                ConvertToScriptString(strMapTests);

                int i=0;
                while(ProcessPartsItems[i] != 0)
                {
                    if( strParts == ProcessPartsItems[i])
                        break;	// found matching string
                    i++;
                };	// loop until we have found the string entry.

                                // Write script sequence
                fprintf(hFile,"  gexFile(group_id,'insert','%s','%s','%s','%s','%s','%s','%s');\n",
                        strFile.toLatin1().constData(),
                        strSite.toLatin1().constData(),
                        gexFileProcessPartsItems[i],
                        strRange.toLatin1().constData(),
                        strMapTests.toLatin1().constData(),
                        strWaferIdFilter.toLatin1().constData(),
                        strTemperature.toLatin1().constData());

                // Move to next file
                nNextFileIndex = pTreeWidgetItemGroup->indexOfChild(pTreeWidgetItemFile) + 1;

                if (nNextFileIndex >= 0 && nNextFileIndex< pTreeWidgetItemGroup->childCount())
                    pTreeWidgetItemFile = pTreeWidgetItemGroup->child(nNextFileIndex);
                else
                    pTreeWidgetItemFile = NULL;
            };

            //insert the condition part if exist
            if(!m_oGroupConditions.isEmpty() && m_oGroupConditions.contains(strGroup)){
                QStringList oOrdredCondition = m_poCompareGroupCondition->text().split(QRegExp("[;,]"),QString::SkipEmptyParts);
                foreach(const QString &strKey, oOrdredCondition){
                    QString strValue = ((m_oGroupConditions[strGroup])[strKey]).toString();
                    fprintf(hFile,"  gexCondition(group_id,'%s','%s');\n", strKey.toLatin1().constData(), strValue.toLatin1().constData());
                }
            }
        }

        // Move to next group
        pTreeWidgetItemGroup = treeWidget->itemBelow(pTreeWidgetItemGroup);
    };

    fprintf(hFile,"  gexAnalyseMode('CompareGroupOfFiles');\n");

    fprintf(hFile,"\n  sysLog('* Quantix Examinator Files groups set ! *');\n\n");
}

///////////////////////////////////////////////////////////
// Read a script file and extract its File List section
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::ReadProcessFilesSection(QString strScriptFile)
{
    QFile lFile( strScriptFile ); // Read the text from a file
    if (lFile.open(QIODevice::ReadOnly) == false)
    {
        GS::Gex::Message::
            critical("", "Failed reading file...folder is read protected?");
        return;
    }

    // Empty List
    OnRemoveAll();

    // Read Tasks definition File
    QTextStream lFileStream;
    lFileStream.setDevice(&lFile);	// Assign file handle to data stream
    QStringList lFileStrings;
    do
    {
        // Read one line from file
        lFileStrings.append(lFileStream.readLine().trimmed());// Remove starting & leading spaces.
    }
    while(lFileStream.atEnd() == false);
    FillDataTab(lFileStrings);
    lFile.close();
}

bool GexMixFileWizardPage1::FillDataTab(const QStringList &lFileStrings)
{
    int		i;
    QStringList strFiles;
    QString lString;
    QStringList testConditions;
    GEX::CSL::CslScriptParser cslScriptParser;
    QString strGroup, strSite,strParts,strRange,strFile,strMapTests,strTemperature, strWaferId;
    for (int j=0; j<lFileStrings.size(); ++j)
    {
        lString = lFileStrings[j];

        // -- analyse csl string
        if(cslScriptParser.init(lString) == false)
            continue;

        if(cslScriptParser.startWith("gexGroup") )
        {
            // Extract Group name
            if(cslScriptParser.getElementGroup(GEX::CSL::Group_Action) == "reset" &&
                    cslScriptParser.getElementGroup(GEX::CSL::Group_Value) == "all")
            {
                // Empty List as a group starts...
                OnRemoveAll();
            }
            // Extract Group name
            else if(cslScriptParser.getElementGroup(GEX::CSL::Group_Action) == "insert")
            {
                // Line format is:
                // group_id = gexGroup('insert','<group name>');

                // Extract Group name
                strGroup   = cslScriptParser.getElementGroup(GEX::CSL::Group_Value);

                // remove ' (Ref)' at the end of the group name
                if (strGroup.endsWith(" (Ref)"))
                    strGroup.truncate(strGroup.length() - 6);

                // Decode script string.
                ConvertFromScriptString(strGroup);

                // create group, and make it selected
                CreateNewGroup(strGroup);
            }

            // Extract Report condition.
            else if(cslScriptParser.getElementGroup(GEX::CSL::Group_Action) == "declare_condition")
            {
                // Line format is:
                // gexGroup('declare_condition','<Test_condition>');

                // Extract condition
                QString condition = cslScriptParser.getElementGroup(GEX::CSL::Group_Value);

                // Remove starting and leading ' characters
                testConditions.append(condition);
            }
        }
        // Extract File entry
        else if(cslScriptParser.startWith("gexFile") &&
                cslScriptParser.getElementFile(GEX::CSL::File_Filter) =="group_id" &&
                cslScriptParser.getElementFile(GEX::CSL::File_Action) =="insert")
        {
            // Line format is:
            // gexFile(group_id,'insert','<file path<','<site#>','<parts type>','<part range>','<map file>'....);

            // Extract File name
            strFile   = cslScriptParser.getElementFile(GEX::CSL::File_FilePath);

            // Decode script string.
            ConvertFromScriptString(strFile);

            // Extract Site
            strSite   = cslScriptParser.getElementFile(GEX::CSL::File_Site);

            // Extract Parts
            strParts   = cslScriptParser.getElementFile(GEX::CSL::File_Part_Type);

            i=0;
            while(gexFileProcessPartsItems[i] != 0)
            {
                if( strParts == gexFileProcessPartsItems[i])
                    break;	// found matching string
                i++;
            };	// loop until we have found the string entry.
            // Map Parts string to a GUI string (eg: 'all' -> 'All Data / parts (any Bin)'
            strParts = ProcessPartsItems[i];

            // Extract Parts range
            strRange   = cslScriptParser.getElementFile(GEX::CSL::File_PartRange);

            // Extract Mapping file
            strMapTests   = cslScriptParser.getElementFile(GEX::CSL::File_MapFile);

            // Decode script string.
            ConvertFromScriptString(strMapTests);

            // Extract Mapping file
            strWaferId = cslScriptParser.getElementFile(GEX::CSL::File_WaferIdFilter);

            // Extract Temperature
            strTemperature   = cslScriptParser.getElementFile(GEX::CSL::File_Temperature);

            // Add entry to the list
            strFiles.clear();
            strFiles << strFile;
            OnAddFiles(strFiles, false, strGroup, strSite, strParts, strRange, strMapTests,
                       strTemperature, strWaferId);

        }
        // Extract Group conditions.
        if(cslScriptParser.startWith("gexCondition") &&
            cslScriptParser.getElementCondition(GEX::CSL::Condition_Filter) == "group_id")
        {
            // Line format is:
            // gexCondition(group_id,'<Test_condition_name>','<test_condition_value>');

            // Extract condition
            QString conditionName = cslScriptParser.getElementCondition(GEX::CSL::Condition_Name);
            QString conditionValue = cslScriptParser.getElementCondition(GEX::CSL::Condition_Value);

            m_oGroupConditions[strGroup].insert(conditionName, conditionValue);
        }
    }
    if (testConditions.count() > 0)
        m_poCompareGroupCondition->setText(testConditions.join(","));
    return true;
}

///////////////////////////////////////////////////////////
// Build the next dataset name
///////////////////////////////////////////////////////////
QString GexMixFileWizardPage1::nextDataSetName()
{
    QString strDataSetName = "DataSet_" + QString::number(iDataSet);

    return strDataSetName;
}

///////////////////////////////////////////////////////////
// Auto adjust the column width to the content
///////////////////////////////////////////////////////////
void GexMixFileWizardPage1::onItemChanged(QTreeWidgetItem* /*pTreeWidgetItem*/,
                                          int nColumn)
{
    treeWidget->resizeColumnToContents(nColumn);
}

QStringList GexMixFileWizardPage1::getGroupNameList(bool bAll ){ //true All //false all without selected
    QStringList oGroupList;
    for(int iIdx=0; iIdx<treeWidget->topLevelItemCount(); ++iIdx){
        if(bAll)
             oGroupList.append(treeWidget->topLevelItem(iIdx)->text(0));
        else if(!treeWidget->topLevelItem(iIdx)->isSelected())
            oGroupList.append(treeWidget->topLevelItem(iIdx)->text(0));
    }
    return oGroupList;
}
