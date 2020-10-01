///////////////////////////////////////////////////////////
// ALL Browser wizard pages for 'Compare file'
///////////////////////////////////////////////////////////

#include <QStringList>
#include <QMenu>
#include <QShortcut>
#include <QDragEnterEvent>

#include "message.h"
#include "csl/cslscriptparser.h"
#include "comparefiles_wizard.h"
#include "report_build.h"
#include "report_options.h"
#include "browser_dialog.h"
#include "gex_constants.h"
#include "import_all.h"
#include "settings_dialog.h"
#include "filter_dialog.h"
#include "treewidgetitemcmp.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

// In report_build.h
extern CReportOptions	ReportOptions;

// in script_wizard.h
extern void ConvertToScriptString(QString &strFile);
extern void ConvertFromScriptString(QString &strFile);

///////////////////////////////////////////////////////////
// WIZARD PAGE 1: SELECT STDF FILES
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_CompareFile_Page1(void)
{
    // Once wizard launches, triggers a report type.
    // GCore-17482
    // Update the Wizard type before the OnDoInstantReport
    SetWizardType(GEX_CMPFILES_WIZARD);

    // Force Report mode = Standard reports (not Enterprise ones)
    pGexMainWindow->pWizardSettings->OnDoInstantReport();

    // Show wizard page Settings
    ShowWizardDialog(GEX_CMPFILES_WIZARD_P1);

}

///////////////////////////////////////////////////////////
// WIZARD PAGE 1: SELECT STDF FILES
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_ShiftAnalysis()
{
    // Once wizard launches, triggers a report type.
    // GCore-17482
    // Update the Wizard type before the OnDoInstantReport
    SetWizardType(GEX_SHIFT_WIZARD);

    // Force Report mode = Standard reports (not Enterprise ones)
    pGexMainWindow->pWizardSettings->OnDoInstantReport();

    // Show wizard page Settings
    ShowWizardDialog(GEX_SHIFT_WIZARD_P1);

}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::ShowPage(void)
{
    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    // Make Widget visible.
    show();
}

void GexCmpFileWizardPage1::SetMode(const GexCmpFileWizardPage1::ViewMode mode)
{
    mMode = mode;
}

///////////////////////////////////////////////////////////
// Update GUI fileds based on Product type running
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::UpdateSkin(int lProductID)
{
    // Set flags
    mIsDataSetList = true;
    mSortingCapability = true;

    switch(lProductID)
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:       // OEM-Examinator for LTXC
        case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
            buttonLoadDatasetList->hide();
            buttonSaveDatasetList->hide();
            mIsDataSetList = false;
            break;

        default:
            break;
    }

    if (treeWidget)
    {
        treeWidget->headerItem()->setData(8,
                                Qt::ToolTipRole,
                                QString("Control groups are used for Shift Analysis."
                                          "\nTo set as a Control group go to:"
                                          "\nFile Properties"));
        if (mMode == SHIFT)
        {
            treeWidget->showColumn(8);
        }
        else
        {
            if (!labelStatus->isHidden())
            {
                labelStatus->hide();
                buttonSettings->setEnabled(true);
            }
            treeWidget->hideColumn(8);
        }
    }

}

///////////////////////////////////////////////////////////
// GexCmpFileWizardPage1: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::reject(void)
{
}

///////////////////////////////////////////////////////////
// Manage the List of files to compare
///////////////////////////////////////////////////////////
GexCmpFileWizardPage1::GexCmpFileWizardPage1( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    GSLOG(SYSLOG_SEV_DEBUG, "");

    setupUi(this);
    setModal(modal);

    move(0,0);

    QObject::connect(buttonRemoveFile,		SIGNAL(clicked()),	this, SLOT(OnRemoveFiles()));
    QObject::connect(PushButtonClearAll,	SIGNAL(clicked()),	this, SLOT(OnRemoveAll()));
    QObject::connect(buttonProperties,		SIGNAL(clicked()),	this, SLOT(OnProperties()));
    QObject::connect(buttonUp,				SIGNAL(clicked()),	this, SLOT(OnMoveFileUp()));
    QObject::connect(buttonDown,			SIGNAL(clicked()),	this, SLOT(OnMoveFileDown()));
    QObject::connect(buttonAddFile,			SIGNAL(clicked()),	this, SLOT(OnAddFiles()));
    QObject::connect(buttonLoadDatasetList, SIGNAL(clicked()),	this, SLOT(OnLoadDatasetList()));
    QObject::connect(buttonSaveDatasetList, SIGNAL(clicked()),	this, SLOT(OnSaveDatasetList()));

    QObject::connect(treeWidget,			SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),	this,	SLOT(OnProperties()));
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
    treeWidget->setColumnWidth (6,70);			// Set minimum size for the icon 'Map Test#' column

    // Until a file is selected, disable the 'Next',Properties,Clear,ClearAll buttons
    buttonSettings->setEnabled(false);
    labelStatus->hide();
    buttonProperties->hide();
    Line1->hide();
    buttonRemoveFile->hide();
    PushButtonClearAll->hide();
    Line2->hide();
    buttonUp->hide();
    buttonDown->hide();
    buttonSaveDatasetList->hide();

    // Empty path to File List.
    mDatasetListFile = "";

    // Support for Drag&Drop
    setAcceptDrops(true);

    // Accelerators
    // 'Delete' key
    new QShortcut(Qt::Key_Delete, this, SLOT(OnRemoveFiles()));

    // Init data set number
    mDataSetNumber = 0;
    // Init mode
    mMode = COMPARE;

    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::dropEvent(QDropEvent *e)
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

    // If only one file dragged over and it is a .CSL...then execute it to load the File List!
    if((strFileList.count() == 1) && (strFileList[0].endsWith(".csl",Qt::CaseInsensitive)))
        ReadProcessFilesSection(strFileList[0]);
    else
        // Insert files selected into the listbox
        OnAddFiles(strFileList);

    e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Contextual menu: Select file
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::onContextualMenu(const QPoint& /*pos*/)
{
    int		iTotalSelection	= totalSelections();
    QMenu	menu(this);

    // Build menu.
    menu.addAction(*pixOpen,"Select/Add files to compare...", this, SLOT(OnAddFiles()));
    if(iTotalSelection >= 1)
    {
        menu.addAction(*pixProperties,"Properties: File details and parts/sites to process", this, SLOT(OnProperties()));
        menu.addAction(*pixRemove,"Remove selected files from list", this, SLOT(OnRemoveFiles()));

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
// Adding file(s) to the list
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::OnAddFiles(
        QStringList filesSelected,
        bool clearList /* =false */,
        QString newGroup,
        QString newSite,
        QString newParts,
        QString newRange,
        QString newMapTests,
        QString temperature,
        QString newWaferId,
        QString newType)
{
    // Check if at list on file to insert
    if(filesSelected.count() <= 0)
        return;

    // Check if clear list first
    if(clearList)
        OnRemoveAll();

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    QString lParts,lType,lSites,lRange,lMapTests, lWaferId;

    if(newParts.isEmpty())
    {
        // GCORE-4437 HTH
        // When Shift Analysis required, Processed parts is forced to last test instance
        if (mMode == SHIFT)
            lParts = ProcessPartsItems[GEX_PROCESSPART_LASTINSTANCE];
        else
            lParts = ProcessPartsItems[GEX_PROCESSPART_ALL];
    }
    else
        lParts = newParts;
    lParts += " ";

    if(newSite.isEmpty())
        lSites = "All ";
    else
        lSites = newSite;

    if(newRange.isEmpty())
        lRange = " ";
    else
        lRange = newRange;

    if(newMapTests.isEmpty())
        lMapTests = " ";
    else
        lMapTests = newMapTests;

    if (newWaferId.isEmpty())
        lWaferId = "";
    else
        lWaferId = newWaferId;

    if (newType.isEmpty())
        lType = "No";
    else
        lType = newType;

    QTreeWidgetItem * pTreeWidgetItemAfter = treeWidget->topLevelItem(treeWidget->topLevelItemCount()-1);

    insertFiles(pTreeWidgetItemAfter, filesSelected, newGroup, lSites, lParts, lRange,
                lMapTests, temperature, lWaferId, lType);
}

///////////////////////////////////////////////////////////
// Adding file(s) to the list
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::OnAddFiles(void)
{
    // Select files from disk...
    GS::Gex::SelectDataFiles    cSelectFiles;

    QStringList         sFilesSelected = cSelectFiles.GetFiles(this,mWorkingPath,"Select Files to compare");

    if(sFilesSelected.isEmpty() == true)
        return;	// Empty list...ignore task!

    // Add files selected into active group.
    OnAddFiles(sFilesSelected);
}

///////////////////////////////////////////////////////////
// Compare ALL sites from within a same file
// Called from external wizard page
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::SplitFileSitesEx(QList <int> *sitesList, QString parts, QString range,
                                             QString temperature, QString file, QString mapTests,
                                             QString waferId,QString isControlsGroup)
{
    // Remove all data existing in the list
    OnRemoveAll();

    // Split file
    SplitFileSites(sitesList, parts, range, temperature, file, mapTests, waferId, isControlsGroup);
}

///////////////////////////////////////////////////////////
// Compare ALL sites from within a same file.
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::SplitFileSites(QList <int> *pSitesList,
                                           QString parts,
                                           QString range,
                                           QString temperature,
                                           QString file,
                                           QString mapTests,
                                           QString waferId,
                                           QString group,
                                           QString isControlGroup)
{
    QStringList			lFiles;
    QString				lGroup, lSite;
    QTreeWidgetItem *	pTreeWidgetItemAfter = treeWidget->topLevelItem(treeWidget->topLevelItemCount()-1);
    QList<int>::iterator lIter;

    // Assign file
    lFiles.clear();
    lFiles << file;

    // Find the item after which we must insert the split files
    QTreeWidgetItem * lTreeWidgetItem = treeWidget->topLevelItem(0);

    while(lTreeWidgetItem != NULL)
    {
        // check if item selected...
        if(lTreeWidgetItem->isSelected() == true)
        {
            pTreeWidgetItemAfter = treeWidget->itemAbove(lTreeWidgetItem);
            break;
        }

        // Move to next item
        lTreeWidgetItem = treeWidget->itemBelow(lTreeWidgetItem);
    };

    // Remove only the selected file
    OnRemoveFiles();

    // Group name is empty, set a new dataset name
    if (group.isEmpty())
        group = nextDataSetName();

    for ( lIter = pSitesList->begin(); lIter != pSitesList->end(); ++lIter )
    {
        // Build group name
        lGroup.sprintf("%s - Site %d", group.toLatin1().constData(), *lIter);

        // Build Site#
        lSite.sprintf("%d ",*lIter);

        // insert file to list
        pTreeWidgetItemAfter = insertFiles(pTreeWidgetItemAfter, lFiles, lGroup, lSite,
                                           parts, range, mapTests, temperature, waferId, isControlGroup);
    }
}

QString GexCmpFileWizardPage1::IsValidInputParameters()
{
    QString lOutMessage;
    // check is at least 1 control defined
    // If more than one selection....copy properties changes (part filter) to all other selections
    QTreeWidgetItem* lTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->topLevelItem(0));
    bool lExistsControl = false;
    int lItemCount = 0;
    QMap<QString,QString> lControlGroup;
    while (lTreeWidgetItem != NULL)
    {
        QString lGroupName = lTreeWidgetItem->text(0);
        QString lControlSample = lTreeWidgetItem->text(8);
        if (lControlSample.startsWith("yes", Qt::CaseInsensitive))
        {
            lExistsControl = true;
            if (lControlSample.trimmed().toLower() == "yes") // no sample defined
            {
                lOutMessage = "No sample defined for group " + lGroupName + "!";
                return lOutMessage;
            }
            if (lControlGroup.contains(lGroupName))
            {
                lOutMessage = "Group " + lGroupName + " shouldn't be reference several times!";
                return lOutMessage;
            }
            QString lSample = lControlSample.toLower().section("-", 1, 1);
            if (!lControlGroup.key(lSample, QString()).isEmpty())
            {
                lOutMessage = "Sample " + lSample + " already linked to control group " + lControlGroup.key(lSample) + "!";
                return lOutMessage;
            }
            lControlGroup.insert(lGroupName, lSample);
        }

        // Move to next item
        lTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(lTreeWidgetItem));
        ++lItemCount;
    };
    // if not return true
    if (!lExistsControl || (lItemCount == 0))
        return "ok";

    // if yes, make sure each control is associated with one sample
    // we must have as much sample as control
    if (lItemCount == (lControlGroup.size() * 2))
        return "ok";
    else if (lItemCount > (lControlGroup.size() * 2))
    {
        lOutMessage = "Some samples are not linked to control!";
        return lOutMessage;
    }
    else
    {
        lOutMessage = "Some controls are not linked to sample!";
        return lOutMessage;
    }
}

///////////////////////////////////////////////////////////
// Returns total number of lines (files) selected
///////////////////////////////////////////////////////////
int GexCmpFileWizardPage1::totalSelections(void)
{
    QTreeWidgetItem *	pTreeWidgetItem = treeWidget->topLevelItem(0);;
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
// Move file UP in the list
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::OnMoveFileUp(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 1)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Scan the list to move UP the selected item.
    QTreeWidgetItemCmp *	pTreeWidgetItem		= static_cast<QTreeWidgetItemCmp *>(treeWidget->topLevelItem(0));
    QTreeWidgetItemCmp *	pTreeWidgetPrevItem	= NULL;
    int						nCurrentItem		= 0;

    while(pTreeWidgetItem != NULL)
    {
        // Find first object selected
        if(pTreeWidgetItem->isSelected() == true)
        {
            pTreeWidgetPrevItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemAbove(pTreeWidgetItem));

            // if 1st file selected...can't move up..ignore!
            if(pTreeWidgetPrevItem != NULL && pTreeWidgetPrevItem->isSelected() == false)
            {
                // Move file above selection to now be just after...
                // Get position of the current item
                nCurrentItem = treeWidget->indexOfTopLevelItem(pTreeWidgetPrevItem);

                // Remove current group from the treewidget and insert it at its new position
                treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItem));
                treeWidget->insertTopLevelItem(nCurrentItem, pTreeWidgetItem);

                // Set the item as selected
                pTreeWidgetItem->setSelected(true);

                pTreeWidgetPrevItem->updateGroupName();
                pTreeWidgetItem->updateGroupName();
            }
        }

        // Move to next item
        pTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(pTreeWidgetItem));
    };
}

///////////////////////////////////////////////////////////
// Move file DOWN in the list
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::OnMoveFileDown(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 1)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Scan the list to move DOWN the selected item.
    QTreeWidgetItemCmp *	pTreeWidgetItem		= NULL;
    QTreeWidgetItemCmp *	pTreeWidgetNextItem	= NULL;
    int						nCurrentItem		= 0;

    pTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->topLevelItem(treeWidget->topLevelItemCount()-1));

    while(pTreeWidgetItem != NULL)
    {
        // Find first object selected
        if(pTreeWidgetItem->isSelected() == true)
        {
            // Keep track of next item before removing it !
            pTreeWidgetNextItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(pTreeWidgetItem));

            if (pTreeWidgetNextItem  && pTreeWidgetNextItem->isSelected() == false)
            {
                // Move selected item after following file in the list
                // Get position of the current item
                nCurrentItem = treeWidget->indexOfTopLevelItem(pTreeWidgetItem);

                // Remove current group from the treewidget and insert it at its new position
                treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetNextItem));
                treeWidget->insertTopLevelItem(nCurrentItem, pTreeWidgetNextItem);

                // update group name
                pTreeWidgetNextItem->updateGroupName();
                pTreeWidgetItem->updateGroupName();
            }
        }

        // Move to next item
        pTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemAbove(pTreeWidgetItem));
    };
}

///////////////////////////////////////////////////////////
// Function name	: GexCmpFileWizardPage1::OnProperties
// Description	    : Edit file properties: site+parts to process
// Return type		: void
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::OnProperties(void)
{
    // If no selection, just ignore action.
    QTreeWidgetItemCmp*	lTreeWidgetItemSelection = static_cast<QTreeWidgetItemCmp *>(treeWidget->currentItem());
    if(lTreeWidgetItemSelection == NULL)
        return;

    // Check if more than one file selected
    QTreeWidgetItemCmp*	lTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->topLevelItem(0));
    int	lSelectedItems	= 0;

    QStringList lSampleGroups;
    while(lTreeWidgetItem != NULL)
    {
        // check if item selected...
        if(lTreeWidgetItem->isSelected() == true)
            lSelectedItems++;

        // Retrieve sample groups
        if ((mMode == SHIFT) && lTreeWidgetItem->text(8).startsWith("no", Qt::CaseInsensitive) &&
                !lTreeWidgetItem->isSelected())
            lSampleGroups << lTreeWidgetItem->text(0);

        // Move to next item
        lTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(lTreeWidgetItem));
    };

    // If no selection found...return!
    if(lSelectedItems <= 0)
        return;

    // Show selection made...
    int nCount = treeWidget->topLevelItemCount();
    FilterDialog *lFilter = new FilterDialog(treeWidget);

    // Give to the filter dialog the file info: file name, site#,etc...Note: Under Yield123, No Split-site selection allowed.
    bool	lAllowSplitSites;
    int		lFlags=0;

    // Allows split site when only one file is being selected and has not been split
    if(lSelectedItems == 1 && (lTreeWidgetItemSelection->text(1) == "All " ||
        lTreeWidgetItemSelection->text(1) == "All")/* && (GS::LPPlugin::ProductInfo::getInstance()->isY123() == false)*/)
    {
        lAllowSplitSites = true;
        lFlags |= GEX_FILE_FILTER_ALLOWSPLIT;
    }
    else
        lAllowSplitSites = false;

    lFlags |= GEX_FILE_FILTER_ALLOWGROUPTYPE;

    lFilter->SetParameters(lSelectedItems,true,0,lTreeWidgetItemSelection,lFlags, lSampleGroups);
    lFilter->SetWaferIdFilter(lTreeWidgetItemSelection->text(7));		// case 3935

    // Check if the conversion call by Filterdialog SPLIT the file...
    if(nCount != treeWidget->topLevelItemCount())
        return;

    // Prompt Filter dialog box
    if(lFilter->exec() != 1)
        return;	// User 'Abort'

    //-- retrieve the group name from the filter window and update
    //-- GCORE-4776
    lTreeWidgetItemSelection->setText(0, lTreeWidgetItemSelection->text(0));
    lTreeWidgetItemSelection->updateGroupName();

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // If more than one selection....copy properties changes (part filter) to all other selections
    lTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->topLevelItem(0));

    while(lTreeWidgetItem != NULL)
    {
        // check if item selected...if so update filter changes
        if(lTreeWidgetItem->isSelected() == true)
        {
            // Update site/part/range filter
            //lTreeWidgetItem->setText(0, lTreeWidgetItemSelection->text(0));	// Do not update Group Name
            lTreeWidgetItem->setText(1, lTreeWidgetItemSelection->text(1));	// Site#
            lTreeWidgetItem->setText(2, lTreeWidgetItemSelection->text(2));	// Parts
            lTreeWidgetItem->setText(3, lTreeWidgetItemSelection->text(3));	// Range
            lTreeWidgetItem->setText(4, lTreeWidgetItemSelection->text(4));	// Temperature
            //lTreeWidgetItem->setText(5, lTreeWidgetItemSelection->text(5));	// Do not update File name
            lTreeWidgetItem->setText(6, lTreeWidgetItemSelection->text(6));	// Test# mapping file
            lTreeWidgetItem->setText(7, lFilter->GetWaferIdFilter());		// Selected wafer ID
            //lTreeWidgetItem->setText(8, lTreeWidgetItemSelection->text(8));	// Group Type
        }

        // Move to next item
        lTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(lTreeWidgetItem));
    };

    if (mMode == SHIFT)
    {
        QString lOut = IsValidInputParameters();
        if (lOut == "ok")
        {
            labelStatus->hide();
            buttonSettings->setEnabled(true);
        }
        else
        {
            labelStatus->setText(lOut);
            labelStatus->show();
            buttonSettings->setEnabled(false);
        }
    }
    else
    {
        labelStatus->hide();
        buttonSettings->setEnabled(true);
    }


    if(lAllowSplitSites == false)
        return;

    // Check if must SPLIT file over each site in it...
    if(lFilter->SpitValidSites() == false)
        return;	// Do not try to spli file overe each individual site (keep all sites merged)

    // Split file over each site
    lFilter->GetValidSites();

    // Check if we have at least 2 sites in the file!
    if(lFilter->mSitesList.count() <= 1)
        return;

    // We have at least 2 sites...then split file to as many sites found.
    QString lSite, lParts, lIsControlsGroup, lRange, lFile, lMapTests, lTemperature, lGroup;

    lSite			= lTreeWidgetItemSelection->text(1);
    lSite			= lSite.trimmed();	// remove leading spaces.
    lParts          = lTreeWidgetItemSelection->text(2);
    lParts          = lParts.trimmed();	// remove leading spaces.
    lRange          = lTreeWidgetItemSelection->text(3);
    lTemperature    = lTreeWidgetItemSelection->text(4);
    lFile			= lTreeWidgetItemSelection->text(5);
    lMapTests		= lTreeWidgetItemSelection->text(6);
    lIsControlsGroup= lTreeWidgetItemSelection->text(8);
    lIsControlsGroup= lIsControlsGroup.trimmed();	// remove leading spaces.
    lMapTests		= lMapTests.trimmed();	// remove leading spaces.
    lGroup          = lTreeWidgetItemSelection->groupBase();

    pGexMainWindow->pWizardCmpFiles->SplitFileSites(&lFilter->mSitesList, lParts, lRange, lTemperature,
                                                    lFile, lMapTests, "", lGroup, lIsControlsGroup);
}

///////////////////////////////////////////////////////////
// Removing file(s) form the list
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::OnRemoveFiles(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 0)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Scan the list to remove selected items.
    QTreeWidgetItemCmp * pTreeWidgetItem		= static_cast<QTreeWidgetItemCmp *> (treeWidget->topLevelItem(0));
    QTreeWidgetItemCmp * pTreeWidgetNextItem	= NULL;

    while(pTreeWidgetItem != NULL)
    {
        // Remove object if it is selected
        if(pTreeWidgetItem->isSelected() == true)
        {
            // Keep track of next item before removing it !
            pTreeWidgetNextItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(pTreeWidgetItem));

            // Remove selected item from list
            treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItem));

            // Destroy it from memory.
            delete pTreeWidgetItem;

            // Update pointer to process next item
            pTreeWidgetItem = pTreeWidgetNextItem;

            // Update the group name, if new position is 0
            if (pTreeWidgetItem && treeWidget->indexOfTopLevelItem(pTreeWidgetItem) == 0)
                pTreeWidgetItem->updateGroupName();
        }
        else
        {
            // Move to next item
            pTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(pTreeWidgetItem));
        }
    };

    // If the list is empty, disable the 'Next',Properties,Clear,ClearAll button.s
    if(treeWidget->topLevelItemCount() <= 0)
    {
        buttonSettings->setEnabled(false);
        buttonProperties->hide();
        Line1->hide();
        buttonRemoveFile->hide();
        PushButtonClearAll->hide();
        buttonSaveDatasetList->hide();
    }

    // If the list has less than 2 files...disable sorting buttons
    if(treeWidget->topLevelItemCount() <= 1)
    {
        Line2->hide();
        buttonUp->hide();
        buttonDown->hide();
    }
}

///////////////////////////////////////////////////////////
// Removing ALL files form the list
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::OnRemoveAll(void)
{
    // Select all entries
    treeWidget->selectAll();

    // Remove selected entries!
    OnRemoveFiles();

    // Reset Dataset number
    mDataSetNumber = 0;
}

///////////////////////////////////////////////////////////
// Reload ListView with list of files from disk
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::OnLoadDatasetList(void)
{
    // Let's user pick the File List to load
    mDatasetListFile = QFileDialog::getOpenFileName(this, "Load a File List...", mDatasetListFile, "File List / Script (*.csl)");

    // If no file selected, ignore command.
    if(mDatasetListFile.isEmpty())
        return;

    // Read section & load it in the list...
    ReadProcessFilesSection(mDatasetListFile);
}

///////////////////////////////////////////////////////////
// Save ListView into a ListFile on disk
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::OnSaveDatasetList(void)
{
    // Let's user tell where to save the File List.
    mDatasetListFile = QFileDialog::getSaveFileName(this, "Save File List to...",
        mDatasetListFile,
        "File List / Script (*.csl)",
        NULL  ,QFileDialog::DontConfirmOverwrite);

    // If no file selected, ignore command.
    if(mDatasetListFile.isEmpty())
        return;

    // Make sure file name ends with ".csl" extension
    if(mDatasetListFile.endsWith(".csl", Qt::CaseInsensitive) == false)
        mDatasetListFile += ".csl";

    FILE *hFile;
    hFile = fopen(mDatasetListFile.toLatin1().constData(),"w");
    if(hFile == NULL)
    {
        GS::Gex::Message::
            critical("", "Failed creating file...folder is write protected?");
        return;
    }

    fprintf(hFile,"<gex_template>\n");
    fprintf(hFile,"<BlockType = compare_files>\n\n");
    WriteProcessFilesSection(hFile);
    fprintf(hFile,"\n</gex_template>\n");
    fclose(hFile);
}

///////////////////////////////////////////////////////////
// Script function to define Data file groups
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::WriteProcessFilesSection(FILE *hFile)
{
    QTreeWidgetItem *	lTreeWidgetItem = treeWidget->topLevelItem(0);
    QString				lSite, lParts, lRange, lFile, lGroup, lIsControlsGroup,
                        lMapTests, lTemperature, lWaferIdFilter;
    long				lIndex = 1;

    lTreeWidgetItem = treeWidget->topLevelItem(0);
    while(lTreeWidgetItem != NULL)
    {
        lParts		= lTreeWidgetItem->text(2);
        lParts		= lParts.trimmed();	// remove leading spaces.
        int lPartIdx=0;
        while(ProcessPartsItems[lPartIdx] != 0)
        {
            if( lParts == ProcessPartsItems[lPartIdx])
                break;	// found matching string
            lPartIdx++;
        };
        if(pGexMainWindow->forceToComputeFromDataSample(hFile, lPartIdx))
            break;
        lTreeWidgetItem = treeWidget->itemBelow(lTreeWidgetItem);
    }

    fprintf(hFile,"  // Compare files...\n");
    fprintf(hFile,"  gexGroup('reset','all');\n\n");

    // Report title (used in report by applications like yield123)
    fprintf(hFile,"  gexQuery('db_report','DataSet_2');\n");

    lTreeWidgetItem = treeWidget->topLevelItem(0);
    while(lTreeWidgetItem != NULL)
    {
        lGroup		= lTreeWidgetItem->text(0);
        lSite		= lTreeWidgetItem->text(1);
        lSite		= lSite.trimmed();	// remove leading spaces.
        lParts		= lTreeWidgetItem->text(2);
        lParts		= lParts.trimmed();	// remove leading spaces.
        lRange		= lTreeWidgetItem->text(3);
        lTemperature= lTreeWidgetItem->text(4);
        lFile		= lTreeWidgetItem->text(5);
        lMapTests	= lTreeWidgetItem->text(6);
        lMapTests	= lMapTests.trimmed();	// remove leading spaces.
        lWaferIdFilter = lTreeWidgetItem->text(7);
        lIsControlsGroup= lTreeWidgetItem->text(8);
        lIsControlsGroup= lIsControlsGroup.trimmed();

        QString lSampleGroup;
        if ((mMode == SHIFT) && lIsControlsGroup.startsWith("yes", Qt::CaseInsensitive))
        {
            lSampleGroup = lIsControlsGroup.section(" - ", 1,1).trimmed();
            if (lSampleGroup.isEmpty())
            {
                GS::Gex::Message::
                    critical("", "Please select a sample for control files");
                return;
            }
        }

        // Ref dataset, don't save ref information
        if ((treeWidget->indexOfTopLevelItem(lTreeWidgetItem) == 0) && lGroup.endsWith("(Ref)"))
            lGroup.truncate(lGroup.length()-6);

        // Convert group name, file path to be script compatible '\' become '\\'
        ConvertToScriptString(lGroup);
        ConvertToScriptString(lFile);
        ConvertToScriptString(lMapTests);

        int lPartIdx=0;
        while(ProcessPartsItems[lPartIdx] != 0)
        {
            if( lParts == ProcessPartsItems[lPartIdx])
                break;	// found matching string
            lPartIdx++;
        };	// loop until we have found the string entry.

        fprintf(hFile,"  // one file per group...\n");
        fprintf(hFile,"  group_id = gexGroup('insert','%s');\n", lGroup.toLatin1().constData());

        // if Pearson, then force 'last_instance' (gexFileProcessPartsItems[GEX_PROCESSPART_LASTINSTANCE])
        if (pGexMainWindow->pWizardSettings->comboAdvancedReport->currentData() == GEX_ADV_PEARSON)
        {
            GSLOG(SYSLOG_SEV_WARNING, "Pearsons report requested, last intance processing forced");
            fprintf(hFile,"  // Pearsons report requested, last intance processing and some options forced :\n");
            fprintf(hFile,"  sysLog('Pearsons report requested, last_intance processing and some options will be forced:');\n");
            fprintf(hFile,"  gexOptions('dataprocessing','clean_samples','true');\n");
            fprintf(hFile,"  gexOptions('dataprocessing','sublot_sorting','partid');\n");
            fprintf(hFile,"  gexFile(group_id,'insert','%s','%s','%s','%s','%s','%s','%s');\n\n",
                    lFile.toLatin1().constData(),
                    lSite.toLatin1().constData(),
                    gexFileProcessPartsItems[GEX_PROCESSPART_LASTINSTANCE],
                    lRange.toLatin1().constData(),
                    lMapTests.toLatin1().constData(),
                    lWaferIdFilter.toLatin1().constData(),
                    lTemperature.toLatin1().constData());
        }
        else if (pGexMainWindow->pWizardSettings->comboAdvancedReport->currentData() == GEX_ADV_SHIFT)
        {
            fprintf(
                        hFile,"  gexFile(group_id,'insert','%s','%s','%s','%s','%s','%s','%s',' ','%s');\n\n",
                        lFile.toLatin1().constData(),
                        lSite.toLatin1().constData(),
                        gexFileProcessPartsItems[lPartIdx],
                        lRange.toLatin1().constData(),
                        lMapTests.toLatin1().constData(),
                        lWaferIdFilter.toLatin1().constData(),
                        lTemperature.toLatin1().constData(),
                        lSampleGroup.toLatin1().constData());
        }
        else
        {
            fprintf(
                hFile,"  gexFile(group_id,'insert','%s','%s','%s','%s','%s','%s','%s');\n\n",
                lFile.toLatin1().constData(),
                lSite.toLatin1().constData(),
                gexFileProcessPartsItems[lPartIdx],
                lRange.toLatin1().constData(),
                lMapTests.toLatin1().constData(),
                lWaferIdFilter.toLatin1().constData(),
                lTemperature.toLatin1().constData());
        }

        // Move to next item in list
        lTreeWidgetItem = treeWidget->itemBelow(lTreeWidgetItem);

        // Keep track of the number of groups created
        lIndex++;
    };

    fprintf(hFile,"  gexAnalyseMode('CompareFiles');\n");
    fprintf(hFile,"\n  sysLog('* Quantix Examinator Files groups set ! *');\n\n");
}

///////////////////////////////////////////////////////////
// Read a script file and extract its File List section
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::ReadProcessFilesSection(QString scriptFile)
{
    QFile lFile( scriptFile ); // Read the text from a file
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
    QStringList lFileStrings;
    lFileStream.setDevice(&lFile);	// Assign file handle to data stream
    do
    {
        // Read one line from file
        lFileStrings.append(lFileStream.readLine().trimmed());// Remove starting & leading spaces.
    }
    while(lFileStream.atEnd() == false);

    FillDataTab(lFileStrings);
    lFile.close();
}

QString GexCmpFileWizardPage1::GetWorkingPath() const
{
    return mWorkingPath;
}

void GexCmpFileWizardPage1::SetWorkingPath(const QString &path)
{
    mWorkingPath = path;
}

bool GexCmpFileWizardPage1::FillDataTab(QStringList &lFileStrings)
{

    GEX::CSL::CslScriptParser cslScriptParser;

    QString		lString;
    int			lIdx;
    QStringList lFiles;
    QString		lGroup, lSite,lParts,lRange,lFileName,lMapTests,lTemperature, lWaferId, lIsControlsGroup;
    for (int i=0; i<lFileStrings.size(); ++i)
    {
        lString = lFileStrings[i];
        if(cslScriptParser.init(lString) == false)
            continue;

        if(cslScriptParser.startWith("gexGroup"))
        {
            // Extract Group name
            if(cslScriptParser.getElementGroup(GEX::CSL::Group_Action) == "reset" &&
                    cslScriptParser.getElementGroup(GEX::CSL::Group_Value) == "all")
            {
                // Empty List as a group starts...
                OnRemoveAll();
            }

            // Extract Group name
            if(cslScriptParser.getElementGroup(GEX::CSL::Group_Action) == "insert")
            {
                 lGroup = cslScriptParser.getElementGroup( GEX::CSL::Group_Value);

                // Decode script string.
                ConvertFromScriptString(lGroup);
            }
        }

        // Extract File entry
        if(cslScriptParser.startWith("gexFile") &&
                cslScriptParser.getElementFile(GEX::CSL::File_Filter) == "group_id" &&
                cslScriptParser.getElementFile(GEX::CSL::File_Action) == "insert")
        {
            // Extract File name
            lFileName = cslScriptParser.getElementFile(GEX::CSL::File_FilePath);

            // Decode script string.
            ConvertFromScriptString(lFileName);

            // Extract Site
            lSite= cslScriptParser.getElementFile(GEX::CSL::File_Site);

            // Extract Parts
            lParts = cslScriptParser.getElementFile(GEX::CSL::File_Part_Type);

            lIdx=0;
            while(gexFileProcessPartsItems[lIdx] != 0)
            {
                if( lParts == gexFileProcessPartsItems[lIdx])
                    break;	// found matching string
                lIdx++;
            };	// loop until we have found the string entry.
            // Map Parts string to a GUI string (eg: 'all' -> 'All Data / parts (any Bin)'
            lParts = ProcessPartsItems[lIdx];

            // Extract Parts range
            lRange = cslScriptParser.getElementFile(GEX::CSL::File_PartRange);

            // Extract Mapping file
            lMapTests = cslScriptParser.getElementFile(GEX::CSL::File_MapFile);

            // Decode script string.
            ConvertFromScriptString(lMapTests);

            // Extract Mapping file
            lWaferId = cslScriptParser.getElementFile(GEX::CSL::File_WaferIdFilter);

            // Extract Temperature
            lTemperature = cslScriptParser.getElementFile(GEX::CSL::File_Temperature);

            // Extract Type
            lIsControlsGroup = cslScriptParser.getElementFile(GEX::CSL::File_ControlGroup);
            if (lIsControlsGroup.trimmed().isEmpty())
                lIsControlsGroup = "No";
            else
                lIsControlsGroup = "Yes - " + lIsControlsGroup;

            // Add entry to the list
            lFiles.clear();
            lFiles << lFileName;
            OnAddFiles(lFiles, false, lGroup, lSite, lParts,
                       lRange, lMapTests, lTemperature, lWaferId, lIsControlsGroup);

        }
    }
    return true;
}

///////////////////////////////////////////////////////////
// Build the next dataset name
///////////////////////////////////////////////////////////
QString GexCmpFileWizardPage1::nextDataSetName()
{
    QString strDataSetName = "DataSet_" + QString::number(++mDataSetNumber);

    return strDataSetName;
}

///////////////////////////////////////////////////////////
// Insert a file list in the list view
///////////////////////////////////////////////////////////
QTreeWidgetItem * GexCmpFileWizardPage1::insertFiles(QTreeWidgetItem * newWidgetItem,
                                                     QStringList filesSelected,
                                                     QString newGroup,
                                                     QString sites,
                                                     QString parts,
                                                     QString range,
                                                     QString mapTests,
                                                     QString temperature,
                                                     QString waferId,
                                                     QString isControlGroup)
{
    QTreeWidgetItem *		lTreeWidgetItemAfter	= newWidgetItem;
    QTreeWidgetItemCmp *	lTreeWidgetItem			= NULL;
    QString					lGroup;
    QString					file;

    // Read the list of files selected, and add them to the list box.
    int		lTotalFiles			= treeWidget->topLevelItemCount();
    bool	lIsTemplateSelected	= false;

    for (QStringList::Iterator it = filesSelected.begin(); it != filesSelected.end(); ++it )
    {
        // Check if file name forced
        file = *it;

        // If one of the file is a Template file, make it selected into the 'Settings page
        if(file.endsWith(".grt", Qt::CaseInsensitive) && pGexMainWindow->pWizardSettings != NULL)
        {
            pGexMainWindow->pWizardSettings->OnSelectReportTemplate(file);
            lIsTemplateSelected = true;
            goto next_file;
        }

        // If Yield 123: Prevent from duplicated entries (can't insert a file multiple times)
//		if(GS::LPPlugin::ProductInfo::getInstance()->isY123())
//		{
//			pTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->topLevelItem(0));
//			while(pTreeWidgetItem != NULL)
//			{
//				// check if file name in list matches the new entry to be added (if so, ignore it)...
//				if(strFile == pTreeWidgetItem->text(5))
//					goto next_file;	// Ignore this file insertion since it is already in the list!

//					// Move to next item
//				pTreeWidgetItem = static_cast<QTreeWidgetItemCmp *>(treeWidget->itemBelow(pTreeWidgetItem));
//			};
//		}

        // Check if Group name forced?
        if(newGroup.isEmpty() == false)
            lGroup = newGroup;
        else // Build a new groupname
            lGroup = nextDataSetName();

        // Keep track of files inserted in list!
        lTotalFiles++;

        // Insert a new item in the list
        lTreeWidgetItem = new QTreeWidgetItemCmp(treeWidget, lTreeWidgetItemAfter);

        lTreeWidgetItem->setText(0, lGroup);
        lTreeWidgetItem->setText(1, sites);
        lTreeWidgetItem->setText(2, parts);
        lTreeWidgetItem->setText(3, range);
        lTreeWidgetItem->setText(4, temperature);
        lTreeWidgetItem->setText(5, file);
        lTreeWidgetItem->setText(6, mapTests);
        lTreeWidgetItem->setText(7, waferId);
        lTreeWidgetItem->setText(8, isControlGroup);

        // If file is inserted in first position, and an item exists below.
        // Update the group name of the second one in order to remove the " (Ref)" text extension.
        if (lTreeWidgetItemAfter == NULL && treeWidget->itemBelow(lTreeWidgetItem))
            ((QTreeWidgetItemCmp*) treeWidget->itemBelow(lTreeWidgetItem))->updateGroupName();

        // Save the new item After
        lTreeWidgetItemAfter = lTreeWidgetItem;

        // Saves latest file selected...so next call start from there.
        mWorkingPath = file;
next_file:;
    }

    // File(s) is selected, enable the 'Next',Properties,Clear,ClearAll buttons
    buttonSettings->setEnabled(true);
    buttonProperties->show();

    Line1->show();
    buttonRemoveFile->show();
    PushButtonClearAll->show();

    // If at least 2 files in list...allow sorting
    if(mSortingCapability && treeWidget->topLevelItemCount() >= 2)
    {
        Line2->show();
        buttonUp->show();
        buttonDown->show();
    }

    if(mIsDataSetList)
        buttonSaveDatasetList->show();

    // If one of the files seledcted is a template file, then switch to the next page 'Settings' tab
    if(lIsTemplateSelected)
    {
        // Go to the Settings page
        pGexMainWindow->Wizard_Settings();
    }

    return lTreeWidgetItemAfter;
}

///////////////////////////////////////////////////////////
// Auto adjust the column width to the content
///////////////////////////////////////////////////////////
void GexCmpFileWizardPage1::onItemChanged(QTreeWidgetItem* /*pTreeWidgetItem*/,
                                          int nColumn)
{
        treeWidget->resizeColumnToContents(nColumn);
}
