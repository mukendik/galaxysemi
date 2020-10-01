///////////////////////////////////////////////////////////
// ALL Browser wizard pages for 'Merge files': TYPE 3 wizard
///////////////////////////////////////////////////////////
#include <QShortcut>
#include <QDragEnterEvent>
#include <QMenu>

#include "mergefiles_wizard.h"
#include "browser_dialog.h"
#include "gex_constants.h"
#include "filter_dialog.h"
#include "import_all.h"
#include "report_build.h"
#include "settings_dialog.h"
#include "report_options.h"
#include <gqtl_log.h>
#include "product_info.h"
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
// WIZARD PAGE 1: SELECT STDF FILES
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_MergeFile_Page1(void)
{
    // GCore-17482
    // Update the Wizard type before the OnDoInstantReport
    SetWizardType(GEX_ADDFILES_WIZARD);

    // Force Report mode = Standard reports (not Enterprise ones)
    pGexMainWindow->pWizardSettings->OnDoInstantReport();

    // Show empty wizard page
    ShowWizardDialog(GEX_ADDFILES_WIZARD_P1);

    // Enable/disable some features...
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        case GS::LPPlugin::LicenseProvider::eLtxcOEM:			// OEM-Examinator for LTXC
            pWizardAddFiles->buttonTimePeriod->setEnabled(FALSE);	// Merge Files
            break;
        default:
            break;
    }
}

///////////////////////////////////////////////////////////
// SETTINGS for ''MERGE files'
///////////////////////////////////////////////////////////
GexAddFileWizardPage1::GexAddFileWizardPage1( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    GSLOG(SYSLOG_SEV_DEBUG, "");

    setupUi(this);
    setModal(modal);

    setWindowFlags(Qt::FramelessWindowHint);
    move(0,0);

    QObject::connect(buttonRemoveFile,		SIGNAL(clicked()),			this, SLOT(OnRemoveFiles()));
    QObject::connect(PushButtonClearAll,	SIGNAL(clicked()),			this, SLOT(OnRemoveAll()));
    QObject::connect(buttonProperties,		SIGNAL(clicked()),			this, SLOT(OnProperties()));
    QObject::connect(buttonAddFile,			SIGNAL(clicked()),			this, SLOT(OnAddFiles()));
    QObject::connect(buttonTimePeriod,		SIGNAL(clicked()),			this, SLOT(OnTimePeriod()));
    QObject::connect(buttonUp,				SIGNAL(clicked()),			this, SLOT(OnMoveFileUp()));
    QObject::connect(buttonDown,			SIGNAL(clicked()),			this, SLOT(OnMoveFileDown()));
    QObject::connect(buttonLoadDatasetList, SIGNAL(clicked()),			this, SLOT(OnLoadDatasetList()));
    QObject::connect(buttonSaveDatasetList, SIGNAL(clicked()),			this, SLOT(OnSaveDatasetList()));
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
    treeWidget->setColumnWidth (6,70);		// Set minimum size for the icon 'Map Test#' column

    // Until a file is selected, disable the 'Next',Properties,Clear,ClearAll buttons
    buttonSettings->setEnabled(false);
    buttonProperties->hide();
    Line1->hide();
    buttonRemoveFile->hide();
    PushButtonClearAll->hide();
    Line2->hide();
    buttonUp->hide();
    buttonDown->hide();
    buttonSaveDatasetList->hide();

    // Empty path to File List.
    strDatasetListFile = "";

    // Support for Drag&Drop
    setAcceptDrops(true);

    // Accelerators
    // 'Delete' key
    new QShortcut(Qt::Key_Delete, this, SLOT(OnRemoveFiles()));

    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());
}

///////////////////////////////////////////////////////////
// GexAddFileWizardPage1: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::reject(void)
{
}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::ShowPage(void)
{
    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    // Make Widget visible.
    show();
}

///////////////////////////////////////////////////////////
// Update GUI fileds based on Product type running
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::UpdateSkin(int lProductID)
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

}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::dropEvent(QDropEvent *e)
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
    if((strFileList.count() == 1) && (strFileList[0].endsWith(".csl", Qt::CaseInsensitive)))
    {
        ReadProcessFilesSection(strFileList[0]);
    }
    else
    {
        // Insert files selected into the listbox
        OnAddFiles(strFileList);
    }

    e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Contextual menu: Select file
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::onContextualMenu(const QPoint& /*pos*/)
{
    int		iTotalSelection = totalSelections();
    QMenu	menu(this);

    // Build menu.
    menu.addAction(*pixOpen,"Select/Add files to merge...", this, SLOT(OnAddFiles()));
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
// Returns total number of lines (files) selected
///////////////////////////////////////////////////////////
int GexAddFileWizardPage1::totalSelections(void)
{
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

    int	iTotal=0;
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
// Adding file(s) matching a time period + Filter criteria to the list
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::OnTimePeriod(void)
{
    // Dispplay Calendar based Import dialog box
    if(TimePeriodFiles.exec() != 1)
        return;	// User canceled dialog box.

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    QString strParts = ProcessPartsItems[0];
    strParts += " ";

    // Read the list of files selected, and add them to the list box.
    QTreeWidgetItem * pTreeWidgetItem = NULL;

    for (QStringList::Iterator it = TimePeriodFiles.ImportFile.begin(); it != TimePeriodFiles.ImportFile.end(); ++it )
    {
        pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
        pTreeWidgetItem->setText(0, "");
        pTreeWidgetItem->setText(1, "All ");
        pTreeWidgetItem->setText(2, strParts);
        pTreeWidgetItem->setText(3, " ");
        pTreeWidgetItem->setText(4, " ");
        pTreeWidgetItem->setText(5, (*it).toLatin1().constData());
        pTreeWidgetItem->setText(6, " ");

        // Saves latest file selected...so next call start from there.
        strWorkingPath = (*it);
    }

    // Free some memory: empty intermediate Import list
    TimePeriodFiles.ImportFile.clear();

    // File(s) selected, enable the 'Next',Properties,Clear,ClearAll buttons
    buttonSettings->setEnabled(true);
    buttonProperties->show();
    Line1->show();
    buttonRemoveFile->show();
    PushButtonClearAll->show();

    // If at least 2 files in list...allow sorting
    if(treeWidget->topLevelItemCount() >= 2)
    {
        Line2->show();
        buttonUp->show();
        buttonDown->show();
    }

    if(bDataSetList)
        buttonSaveDatasetList->show();
}

///////////////////////////////////////////////////////////
// Move file UP in the list
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::OnMoveFileUp(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 1)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Scan the list to move UP the selected item.
    QTreeWidgetItem *	pTreeWidgetItem		= treeWidget->topLevelItem(0);
    int					nPosPreviousItem	= -1;

    while(pTreeWidgetItem != NULL)
    {
        // Find first object selected
        if (pTreeWidgetItem->isSelected() == true)
        {
            // if 1st file selected...can't move up..ignore!
            if(nPosPreviousItem != -1)
            {
                // Remove the current item from the tree
                treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItem));

                // Move file above selection to now be just after...
                treeWidget->insertTopLevelItem(nPosPreviousItem, pTreeWidgetItem);

                // Re-select the current item
                pTreeWidgetItem->setSelected(true);
            }

            return;
        }
        else
        {
            // Keep track of 'previous' file in list
            nPosPreviousItem = treeWidget->indexOfTopLevelItem(pTreeWidgetItem);

            // Move to next item
            pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
        }
    };
}

///////////////////////////////////////////////////////////
// Move file DOWN in the list
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::OnMoveFileDown(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 1)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Scan the list to move DOWN the selected item.
    QTreeWidgetItem *	pTreeWidgetItem		= treeWidget->topLevelItem(0);
    QTreeWidgetItem *	pTreeWidgetNextItem	= NULL;

    while(pTreeWidgetItem != NULL)
    {
        // Find first object selected
        if(pTreeWidgetItem->isSelected() == true)
        {
            // Keep track of next item before removing it !
            pTreeWidgetNextItem = treeWidget->itemBelow(pTreeWidgetItem);

            // Get index of the current selection
            int nCurrentSelection = treeWidget->indexOfTopLevelItem(pTreeWidgetItem);

            // Remove the next item from the tree
            treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetNextItem));

            // Move selected item after following file in the list
            treeWidget->insertTopLevelItem(nCurrentSelection, pTreeWidgetNextItem);

            // only move one selection at a time
            return;
        }
        else
        {
            // Move to next item
            pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
        }
    };
}

///////////////////////////////////////////////////////////
// Adding file(s) to the list
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::OnAddFiles(void)
{
    GS::Gex::SelectDataFiles	cSelectFiles;
    QStringList sFilesSelected = cSelectFiles.GetFiles(this,strWorkingPath,"Select Files to merge");

    if(sFilesSelected.isEmpty() == true)
        return;	// Empty list...ignore task!

    // Insert files to list box
    OnAddFiles(sFilesSelected);
}

///////////////////////////////////////////////////////////
// Adding file(s) to the list
///////////////////////////////////////////////////////////
void
GexAddFileWizardPage1::OnAddFiles(QStringList sFilesSelected,
                                  bool bClearList /* = false */,
                                  QString /*strNewGroup*/,
                                  QString strNewSite,
                                  QString strNewParts,
                                  QString strNewRange,
                                  QString strNewMapTests,
                                  QString strTemperature,
                                  QString strNewWaferId,
                                  QString strDataSetName)
{
    // Make sure list is not empty
    if(sFilesSelected.count() <= 0)
        return;

    // Check if clear list first
    if(bClearList)
        OnRemoveAll();

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

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
    QTreeWidgetItem * pTreeWidgetItem = NULL;
    bool bTemplateSelected = false;

    for (QStringList::Iterator it = sFilesSelected.begin(); it != sFilesSelected.end(); ++it )
    {
        // Check if file name forced
        strFile = *it;

        // If one of the file is a Template file, make it selected into the 'Settings page
        if(strFile.endsWith(".grt", Qt::CaseInsensitive) && pGexMainWindow->pWizardSettings != NULL)
        {
            pGexMainWindow->pWizardSettings->OnSelectReportTemplate(strFile);
            bTemplateSelected = true;
            goto next_file;
        }

        pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
        pTreeWidgetItem->setText(0, strDataSetName);
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

    // File(s) selected, enable the 'Next',Properties,Clear,ClearAll buttons
    buttonSettings->setEnabled(true);
    buttonProperties->show();
    Line1->show();
    buttonRemoveFile->show();
    PushButtonClearAll->show();

    // If at least 2 files in list...allow sorting
    if(treeWidget->topLevelItemCount() >= 2)
    {
        Line2->show();
        buttonUp->show();
        buttonDown->show();
    }

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
// Removing file(s) form the list
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::OnRemoveFiles(void)
{
    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 0)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Scan the list to remove selected items.
    QTreeWidgetItem * pTreeWidgetItem		= treeWidget->topLevelItem(0);
    QTreeWidgetItem * pTreeWidgetNextItem	= NULL;

    while(pTreeWidgetItem != NULL)
    {
        // Remove object if it is selected
        if(pTreeWidgetItem->isSelected() == true)
        {
            // Keep track of next item before removing it !
            pTreeWidgetNextItem = treeWidget->itemBelow(pTreeWidgetItem);

            // Remove selected item from list
            treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItem));

            // Destroy it from memory.
            delete pTreeWidgetItem;

            // Update pointer to process next item
            pTreeWidgetItem = pTreeWidgetNextItem;
        }
        else
        {
            // Move to next item
            pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
        }
    };

    // If the list is empty, disable the 'Next',Properties,Clear,ClearAll buttons
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
void GexAddFileWizardPage1::OnRemoveAll(void)
{
    // Select all entries
    treeWidget->selectAll();

    // Remove selected entries!
    OnRemoveFiles();
}

///////////////////////////////////////////////////////////
// Function name	: GexAddFileWizardPage1::OnProperties
// Description	    : Edit file properties: site+parts to process
// Return type		: void
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::OnProperties(void)
{
    QTreeWidgetItem * pTreeWidgetItemSelection = treeWidget->currentItem();

    // If no selection, just ignore action.
    if(pTreeWidgetItemSelection == NULL)
        return;

    // Check if more than one file selected
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->topLevelItem(0);

    int	iSelectedItems = 0;
    while(pTreeWidgetItem != NULL)
    {
        // check if item selected...
        if(pTreeWidgetItem->isSelected() == true)
            iSelectedItems++;

        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };

    // If no selection found...return!
    if(iSelectedItems <= 0)
        return;

    // Show selection made...
    int nCount = treeWidget->topLevelItemCount();

    FilterDialog *dFilter = new FilterDialog(treeWidget);

    // Give to the filter dialog the file info: file name, site#,etc...
    dFilter->SetParameters(iSelectedItems,
                           false,
                           1,
                           pTreeWidgetItemSelection);
    dFilter->SetWaferIdFilter(pTreeWidgetItemSelection->text(7));		// case 3935

    // Check if the conversion call by Filterdialog SPLIT the file...
    if(nCount != treeWidget->topLevelItemCount())
        return;

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
            // Update site/part/range filter
            pTreeWidgetItem->setText(0, pTreeWidgetItemSelection->text(0));	// Dataset name
            pTreeWidgetItem->setText(1, pTreeWidgetItemSelection->text(1));	// Site#
            pTreeWidgetItem->setText(2, pTreeWidgetItemSelection->text(2));	// Parts
            pTreeWidgetItem->setText(3, pTreeWidgetItemSelection->text(3));	// Range
            pTreeWidgetItem->setText(4, pTreeWidgetItemSelection->text(4));	// Temperature
            pTreeWidgetItem->setText(6, pTreeWidgetItemSelection->text(6));	// Test# mapping file
            pTreeWidgetItem->setText(7, dFilter->GetWaferIdFilter());		// Wafer ID filter
        }
        // Move to next item
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
    };
}

///////////////////////////////////////////////////////////
// Reload ListView with list of files from disk
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::OnLoadDatasetList(void)
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
void GexAddFileWizardPage1::OnSaveDatasetList(void)
{
    // Let's user tell where to save the File List.
    strDatasetListFile = QFileDialog::getSaveFileName(this,
                                                      "Save File List to...",
                                                      strDatasetListFile,
                                                      "File List / Script (*.csl)");
//    strDatasetListFile = QFileDialog::getSaveFileName(strDatasetListFile, "File List / Script (*.csl)",
//                                                        this, "save file dialog", "Save File List to...");

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
    fprintf(hFile,"<BlockType = merge_files>\n\n");
    WriteProcessFilesSection(hFile);
    fprintf(hFile,"\n</gex_template>\n");
    fclose(hFile);
}

///////////////////////////////////////////////////////////
// Script function to define Data file groups
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::WriteProcessFilesSection(FILE *hFile)
{
    QTreeWidgetItem *	pTreeWidgetItem = treeWidget->topLevelItem(0);
    QString				strDatasetName,strSite,strParts,strRange,strFile,strGroup,strMapTests,strTemperature, strWaferIdFilter;
    long				iIndex;

        pTreeWidgetItem = treeWidget->topLevelItem(0);
        while(pTreeWidgetItem != NULL)
        {
            strParts		= pTreeWidgetItem->text(2);
            strParts		= strParts.trimmed();	// remove leading spaces.
            int i=0;
            while(ProcessPartsItems[i] != 0)
            {
                    if( strParts == ProcessPartsItems[i])
                            break;	// found matching string
                    i++;
            };
            if(pGexMainWindow->forceToComputeFromDataSample(hFile, i))
                break;
            pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);
        }

    fprintf(hFile,"  // Merge all files...\n");
    fprintf(hFile,"  gexGroup('reset','all');\n\n");

    // Report title (used in report by applications like yield123)
    fprintf(hFile,"  gexQuery('db_report','DataSet_3');\n");
    fprintf(hFile,"  // All files in same group...\n");
    fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");

    iIndex = 1;
        pTreeWidgetItem = treeWidget->topLevelItem(0);
    while(pTreeWidgetItem != NULL)
    {
        strDatasetName	= pTreeWidgetItem->text(0);
        strDatasetName	 = strDatasetName.trimmed();	// remove leading spaces.
        strSite			= pTreeWidgetItem->text(1);
        strSite			= strSite.trimmed();	// remove leading spaces.
        strParts		= pTreeWidgetItem->text(2);
        strParts		= strParts.trimmed();	// remove leading spaces.
        strRange		= pTreeWidgetItem->text(3);
        strTemperature  = pTreeWidgetItem->text(4);
        strFile			= pTreeWidgetItem->text(5);
        strMapTests		= pTreeWidgetItem->text(6);
        strMapTests		= strMapTests.trimmed();	// remove leading spaces.
        strWaferIdFilter = pTreeWidgetItem->text(7);


        // Convert file path to be script compatible '\' become '\\'
        ConvertToScriptString(strDatasetName);
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
        fprintf(hFile,"  gexFile(group_id,'insert','%s','%s','%s','%s','%s','%s','%s','%s');\n",
                strFile.toLatin1().constData(),
                strSite.toLatin1().constData(),
                gexFileProcessPartsItems[i],
                strRange.toLatin1().constData(),
                strMapTests.toLatin1().constData(),
                strWaferIdFilter.toLatin1().constData(),
                strTemperature.toLatin1().constData(),
                strDatasetName.toLatin1().constData());

        // Move to next item in list
        pTreeWidgetItem = treeWidget->itemBelow(pTreeWidgetItem);

        // Keep track of the number of groups created
        iIndex++;
    };

    fprintf(hFile,"  gexAnalyseMode('MergeFiles');\n");
    fprintf(hFile,"\n  sysLog('* Quantix Examinator Files groups set ! *');\n\n");
}

///////////////////////////////////////////////////////////
// Read a script file and extract its File List section
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::ReadProcessFilesSection(QString strScriptFile)
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

bool GexAddFileWizardPage1::FillDataTab(const QStringList &lFileStrings)
{
    int		i;
    QStringList strFiles;
    QString		lString;
    GEX::CSL::CslScriptParser cslScriptParser;
    QString strGroup, strSite,strParts,strRange,strFile,strMapTests,strTemperature, strWaferId, dataSetName;
    for (int j=0; j<lFileStrings.size(); ++j)
    {
        lString = lFileStrings[j];

        // -- analyse csl string
        if(cslScriptParser.init(lString) == false)
            continue;

        // Extract Group name
        if(cslScriptParser.startWith("gexGroup") )
        {
            if(cslScriptParser.getElementGroup(GEX::CSL::Group_Action) == "reset" &&
                    cslScriptParser.getElementGroup(GEX::CSL::Group_Value) == "all")
            {
                // Empty List as a group starts...
                OnRemoveAll();
            }

            // Extract Group name
            if(cslScriptParser.getElementGroup(GEX::CSL::Group_Action) == "insert")
            {
                // Line format is:
                // group_id = gexGroup('insert','<group name>');

                // Extract Group name
                strGroup   = cslScriptParser.getElementGroup(GEX::CSL::Group_Value);

                // Decode script string.
                ConvertFromScriptString(strGroup);
            }
        }
        else if(cslScriptParser.startWith("gexFile") &&
                cslScriptParser.getElementFile(GEX::CSL::File_Filter) =="group_id" &&
                cslScriptParser.getElementFile(GEX::CSL::File_Action) =="insert")
        {
            // Line format is:
            // gexFile(group_id,'insert','<file path<','<site#>','<parts type>','<part range>','<map file>', '<WaferIDfilter>', '<temperature>','dataset name');

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

            dataSetName   = cslScriptParser.getElementFile(GEX::CSL::File_DataSetName);

            // Add entry to the list
            strFiles.clear();
            strFiles << strFile;
            OnAddFiles(strFiles,false,strGroup,strSite,strParts,strRange,strMapTests,strTemperature,strWaferId, dataSetName);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////
// Auto adjust the column width to the content
///////////////////////////////////////////////////////////
void GexAddFileWizardPage1::onItemChanged(QTreeWidgetItem* /*pTreeWidgetItem*/,
                                          int nColumn)
{
        treeWidget->resizeColumnToContents(nColumn);
}

