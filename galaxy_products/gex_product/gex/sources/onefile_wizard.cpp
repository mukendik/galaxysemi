///////////////////////////////////////////////////////////
// ALL Browser wizard pages for 'Analyze One file'
///////////////////////////////////////////////////////////
#include <qmenu.h>
#include <qapplication.h>
#include "onefile_wizard.h"
#include "comparefiles_wizard.h"
#include "browser_dialog.h"
#include "gex_shared.h"
#include "gex_constants.h"
#include "filter_dialog.h"
#include "import_all.h"
#include "report_build.h"
#include "report_options.h"
#include "settings_dialog.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "message.h"
#include "csl/cslscriptparser.h"
#include "script_wizard.h"

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
// WIZARD PAGE 1: SELECT STDF FILE
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_OneFile_Page1(void)
{
    // Updates Wizard type
    // GCore-17482
    // Update the Wizard type before the OnDoInstantReport
    SetWizardType(GEX_ONEFILE_WIZARD);

    // Force Report mode = Standard reports (not Enterprise ones)
    pGexMainWindow->pWizardSettings->OnDoInstantReport();

    // Show wizard page : select file.
    ShowWizardDialog(GEX_ONEFILE_WIZARD_P1);
}

///////////////////////////////////////////////////////////
// WIZARD: Check STDF file contents to identify any discrepency that could affect PAT (eg: test number duplication, etc)
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_OneFile_FileAudit(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    if(!GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() &&
            !GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() &&
            !GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Force 'Settings' page to select the 'MyReports' on tempate '<user_profile>/data_audit.grt
    QString strScriptFile = GS::Gex::Engine::GetInstance().Get("UserFolder").toString()
            + "/data_audit.grt";

    // If this template file doesn't exist, then create a default one.
    if(QFile::exists(strScriptFile) == false)
    {
        QFile file(strScriptFile); // Write the template file
        if (file.open(QIODevice::WriteOnly) == false)
        {
            QString strMessage =
                "Failed creating file...folder is write protected?\n" +
                strScriptFile;
            GS::Gex::Message::critical("", strMessage);
            return ;	// Failed.
        }

        // Read Template File
        QTextStream hTemplate(&file);	// Assign file handle to data stream

        // Write file content
        hTemplate << "<galaxy_template>" << endl;	// Start template marker
        hTemplate << "<section_home_page>" << endl;
        hTemplate << "HomeText=" << endl;
        hTemplate << "HomeLogo=" << endl;
        hTemplate << "</section_home_page>" << endl;
        hTemplate << "<section_file_audit>" << endl;
        hTemplate << "Title=Section: Data Audit" << endl;
        // No need to set the options as they are in the file_audit constructor!
        hTemplate << "</section_file_audit>" << endl;
        hTemplate << "</galaxy_template>" << endl;
        file.close();

    }
    pWizardSettings->OnSelectReportTemplate(strScriptFile);

    // Flag to Split data file per testing site
    pWizardOneFile->m_bSplitPerSiteFileSelection = true;

    // Perform a 'select one file'
    Wizard_OneFile_Page1();

    // Clear any file already selected in the 'single file' dialog box
    pGexMainWindow->pWizardOneFile->OnRemoveFile();
}

///////////////////////////////////////////////////////////
// Manage the List of files to compare
///////////////////////////////////////////////////////////
GexOneFileWizardPage1::GexOneFileWizardPage1( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "");

    setupUi(this);
    setModal(modal);
    setObjectName("GSOneFileWizard");

    setWindowFlags(Qt::FramelessWindowHint);
    move(0,0);

    QObject::connect(buttonAddFile,		SIGNAL(clicked()), this, SLOT(OnAddFiles()));
    QObject::connect(buttonProperties,	SIGNAL(clicked()), this, SLOT(OnProperties()));

    QObject::connect(treeWidget,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),	this,	SLOT(OnProperties()));
    QObject::connect(treeWidget,	SIGNAL(customContextMenuRequested(const QPoint&)),	this,	SLOT(onContextualMenu(const QPoint&)));
    QObject::connect(treeWidget,	SIGNAL(itemChanged(QTreeWidgetItem*,int)),			this,	SLOT(onItemChanged(QTreeWidgetItem*,int)));

    // Flag allows to force splitting file per sites (so to compare sites). Usually set by GUI hyperlink (eg: PAT Complpiancy Checker)
    m_bSplitPerSiteFileSelection = false;

    // Until a file is selected, disable the 'Next' & properties buttons
    buttonSettings->setEnabled(false);
    buttonProperties->hide();

    // Support for Drag&Drop
    setAcceptDrops(true);

    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    // Adjust columns width
    treeWidget->resizeColumnToContents(0);
    treeWidget->resizeColumnToContents(1);
    treeWidget->resizeColumnToContents(2);
    treeWidget->resizeColumnToContents(3);
    treeWidget->resizeColumnToContents(4);
    treeWidget->setColumnWidth(5, 70);			// Set minimum size for the icon 'Map Test#' column
}

///////////////////////////////////////////////////////////
// GexOneFileWizardPage1: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::reject(void)
{
}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::ShowPage(void)
{
    // Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    // Make Widget visible.
    show();

    // Focus on Report title edit box (in the event it is disabled/hidden, then there is simply no startup focus!)
    lineEditReportTitle->setFocus();
}

///////////////////////////////////////////////////////////
// Update GUI fileds based on Product type running
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::UpdateSkin(int /*lProductID*/)
{
//    if (GS::LPPlugin::ProductInfo::getInstance()->isY123())
//    {
//        TextLabelReportTitle->show();
//        lineEditReportTitle->show();
//    }
//    else
//    {
            TextLabelReportTitle->hide();
            lineEditReportTitle->hide();
//    }
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::dropEvent(QDropEvent *e)
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

    // Get file name dropped & accept drop.
    QString strFile = strFileList[0];
    e->acceptProposedAction();

    // If file is a Template file, make it selected into the 'Settings page
    if(strFile.endsWith(".grt", Qt::CaseInsensitive) && pGexMainWindow->pWizardSettings != NULL)
    {
        // Select Template file
        pGexMainWindow->pWizardSettings->OnSelectReportTemplate(strFile);

        // Go to the Settings page
        pGexMainWindow->Wizard_Settings();
    }
    else
    {
        // Insert first file selected into the listbox
        OnSelectFile(strFile);
    }
}

///////////////////////////////////////////////////////////
// Contextual menu: Select file
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::onContextualMenu(const QPoint& /*pos*/)
{
    QMenu menu(this);

    // Build menu.
    menu.addAction(*pixOpen,"Select file to analyze ...", this, SLOT(OnAddFiles()));
    if(treeWidget->topLevelItemCount() != 0)
        menu.addAction(*pixProperties,"Properties: File details and parts/sites to process", this, SLOT(OnProperties()));

    menu.setMouseTracking(true);
    menu.exec(QCursor::pos());
    menu.setMouseTracking(false);
}

///////////////////////////////////////////////////////////
// Called if a STDF file was specified at startup time...
// Or when using the Wizard.
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::OnSelectFile(QString & strFileName)
{
    QTreeWidgetItem *	pTreeWidgetItem = NULL;

    if(pGexMainWindow == NULL)
        return;

    if(strFileName.isNull() || strFileName.isEmpty())
        return;

    // Reset HTML sections to create flag: ALL pages to create (unless startup call: window not created yet!)
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Saves latest file selected...so nect call start from there.
    strWorkingPath = strFileName;

    // Make this file the new and only one in the list.
    treeWidget->clear();

    QString strParts = ProcessPartsItems[0];
    strParts += " ";

    pTreeWidgetItem = new QTreeWidgetItem(treeWidget);
    pTreeWidgetItem->setText(0, "All ");
    pTreeWidgetItem->setText(1, strParts);
    pTreeWidgetItem->setText(2, " ");
    pTreeWidgetItem->setText(3, " ");
    pTreeWidgetItem->setText(4, strFileName);
    pTreeWidgetItem->setText(5, " ");
    pTreeWidgetItem->setText(6, "");		// case 3935
    m_strWaferIdFilter = QString();

    // A file is selected, enable the 'Next' + properties buttons
    buttonSettings->setEnabled(true);
    buttonProperties->show();

    // If flag to split per site, do it!
    if(m_bSplitPerSiteFileSelection)
    {
        // Change cursor to Hour glass (Wait cursor)...
        QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

        // Split per site
        SplitTestingSites();

        // Clear flag: this flag is only set when specific GUI hyperlink is clicked (eg: PAT Compliancy Check)
        m_bSplitPerSiteFileSelection = false;

        // Change cursor back to normal
        QGuiApplication::restoreOverrideCursor();
    }
}


void GexOneFileWizardPage1::SetWaferIDFilter(QString strWaferIdFilter)
{
    m_strWaferIdFilter = strWaferIdFilter;
    QTreeWidgetItem* qtwiPtrWidgetItem = treeWidget->topLevelItem(0);

    if(qtwiPtrWidgetItem)
    {
        qtwiPtrWidgetItem->setText(6, m_strWaferIdFilter);
    }
}

bool GexOneFileWizardPage1::FillDataTab(const QStringList &lFileStrings)
{
    int		i;
    QStringList strFiles;
    QString lString;
    GEX::CSL::CslScriptParser cslScriptParser;
    QString strSite,strParts,strRange,strFile,strMapTests,strTemperature, strWaferId;
    for (int j=0; j<lFileStrings.size(); ++j)
    {
        lString = lFileStrings[j];

        // -- analyse csl string
        if(cslScriptParser.init(lString) == false)
            continue;

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

            QTreeWidgetItem * lTreeWidgetItem = treeWidget->takeTopLevelItem(0);
            lTreeWidgetItem = new QTreeWidgetItem(treeWidget);
            lTreeWidgetItem->setText(0, strSite);
            lTreeWidgetItem->setText(1, strParts);
            lTreeWidgetItem->setText(2, strRange);
            lTreeWidgetItem->setText(3, strTemperature);
            lTreeWidgetItem->setText(4, strFile);
            lTreeWidgetItem->setText(5, strMapTests);
            lTreeWidgetItem->setText(6, strWaferId);

            // A file is selected, enable the 'Next' + properties buttons
            buttonSettings->setEnabled(true);
            buttonProperties->show();
        }
    }
    return true;
}

///////////////////////////////////////////////////////////
// Select file to process
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::OnAddFiles(void)
{
    QString fn;
    GS::Gex::SelectDataFiles	cSelectFile;

    // User wants to analyze a single file
    fn = cSelectFile.GetSingleFile(this,strWorkingPath,"Select File to analyze");

    if(fn.isEmpty())
        return;

    // Add file to list...
    OnSelectFile(fn);
}

///////////////////////////////////////////////////////////
// Removing file from the list
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::OnRemoveFile(void)
{
    // If no item in list, just return!
    if (treeWidget->topLevelItemCount() == 0)
        return;

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Scan the list to remove selected items.
    QTreeWidgetItem * pTreeWidgetItem = treeWidget->takeTopLevelItem(0);

    if (pTreeWidgetItem)
        delete pTreeWidgetItem;
    pTreeWidgetItem=0;
}

///////////////////////////////////////////////////////////
// Function name	: GexOneFileWizardPage1::OnProperties
// Description	    : Edit file properties: site+parts to process
// Return type		: void
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::OnProperties(void)
{
    QTreeWidgetItem * pTreeWidgetSelection = treeWidget->currentItem();

    // If no selection, just ignore action.
    if(pTreeWidgetSelection == NULL)
        return;

    // Show selection made...
    int nCount = treeWidget->topLevelItemCount();
    FilterDialog *dFilter = new FilterDialog(treeWidget);

    // Give to the filter dialog the file info: file name, site#,etc...
    int iSelectedItems=1;
    dFilter->SetParameters(iSelectedItems,false,0,pTreeWidgetSelection,GEX_FILE_FILTER_ALLOWSPLIT);
    dFilter->SetWaferIdFilter(pTreeWidgetSelection->text(6));		// case 3935

    // Check if the conversion call by Filterdialog SPLIT the file...
    if(nCount != treeWidget->topLevelItemCount())
    {
        // Have to go to appropriate page
        QTreeWidgetItem * pTreeWidgetItem = treeWidget->takeTopLevelItem(0);

        while(pTreeWidgetItem != NULL)
        {
            pGexMainWindow->pWizardCmpFiles->OnAddFiles(QStringList(pTreeWidgetItem->text(4)));

            // Destroy it from memory.
            delete pTreeWidgetItem;

            // Move to next item
            pTreeWidgetItem = treeWidget->takeTopLevelItem(0);
        };

        // Call 'Compare files'  GUI page.
        pGexMainWindow->Wizard_CompareFile_Page1();
        return;
    }

    // Prompt Filter dialog box
    if(dFilter->exec() != 1)
        return;	// User 'Abort'

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Get the wafer id filter selected if any
    SetWaferIDFilter(dFilter->GetWaferIdFilter());
    //m_strWaferIdFilter = dFilter->GetWaferIdFilter();

    // Check if must SPLIT file over each site in it...
    if(dFilter->SpitValidSites() == false)
        return;	// Do not try to split file over each individual site (keep all sites merged)

    // Split file over each site
    dFilter->GetValidSites(QString(), false);

    // Check if we have at least 2 sites in the file!
    if(dFilter->mSitesList.count() >= 1)
        SplitTestingSites();	// We do: so spllit filer/site!
}

///////////////////////////////////////////////////////////
// Function name	: SplitTestingSites
// Description	    : Force spliting file over its testing sites
// Return type		: void
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::SplitTestingSites(void)
{
    QTreeWidgetItem *	pTreeWidgetSelection = treeWidget->currentItem();

    // If no selection, just ignore action.
    if(pTreeWidgetSelection == NULL)
        return;

    // Show selection made...
    FilterDialog *dFilter = new FilterDialog();

    // Give to the filter dialog the file info: file name, site#,etc...
    int iSelectedItems=1;
    dFilter->SetParameters(iSelectedItems,false,0,pTreeWidgetSelection,GEX_FILE_FILTER_ALLOWSPLIT);

    // Split file over each site
    dFilter->GetValidSites();

    // Check if we have at least 2 sites in the file!
    if(dFilter->mSitesList.count() < 1)
        return;

    // We have at least 2 sites...then split file to as many sites found.
    QString lSite,lParts,lRange,lFile,lMapTests,lTemperature;
    lSite			= pTreeWidgetSelection->text(0);
    lSite			= lSite.trimmed();				// remove leading spaces.
    lParts          = pTreeWidgetSelection->text(1);
    lParts          = lParts.trimmed();				// remove leading spaces.
    lRange          = pTreeWidgetSelection->text(2);
    lTemperature	= pTreeWidgetSelection->text(3);
    lFile			= pTreeWidgetSelection->text(4);
    lMapTests		= pTreeWidgetSelection->text(5);
    lMapTests		= lMapTests.trimmed();			// remove leading spaces.

    pGexMainWindow->pWizardCmpFiles->SplitFileSitesEx(&dFilter->mSitesList, lParts, lRange,
                                                      lTemperature, lFile, lMapTests, m_strWaferIdFilter, "");

    // Call 'Compare files'  GUI page.
    pGexMainWindow->Wizard_CompareFile_Page1();
}

///////////////////////////////////////////////////////////
// Script function to define Data file groups
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::WriteProcessFilesSection(FILE *hFile)
{
    QTreeWidgetItem *	pTreeWidgetItem = treeWidget->topLevelItem(0);
    QString				strSite,strParts,strRange,strFile,strMapTests,strTemperature;

    if (pTreeWidgetItem == NULL)
        return;	//no file selected yet!

    strSite			= pTreeWidgetItem->text(0);
    strSite			= strSite.trimmed();	// remove leading spaces.
    strParts		= pTreeWidgetItem->text(1);
    strParts		= strParts.trimmed();	// remove leading spaces.
    strRange		= pTreeWidgetItem->text(2);
    strTemperature	= pTreeWidgetItem->text(3);
    strFile			= pTreeWidgetItem->text(4);
    strMapTests		= pTreeWidgetItem->text(5);
    strMapTests		= strMapTests.trimmed();	// remove leading spaces.

    int i=0;
    while(ProcessPartsItems[i] != 0)
    {
        if( strParts == ProcessPartsItems[i])
            break;	// found matching string
        i++;
    };	// loop until we have found the string entry.

        pGexMainWindow->forceToComputeFromDataSample(hFile, i);

    // Write script sequence
    fprintf(hFile,"  // One single file to analyze...\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");

    // get report title (if any defined or allowed)
    QString strTitle = lineEditReportTitle->text();
    if(!strTitle.isEmpty())
        fprintf(hFile,"  gexQuery('db_report','%s');\n",strTitle.toLatin1().constData());	// Report title (used in report by applications like yield123)

    fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");
    // Convert file path to be script compatible '\' become '\\'
    ConvertToScriptString(strFile);
    ConvertToScriptString(strMapTests);
    fprintf(hFile,"  gexFile(group_id,'insert','%s','%s','%s','%s','%s','%s','%s');\n",
            strFile.toLatin1().constData(),
            strSite.toLatin1().constData(),
            gexFileProcessPartsItems[i],
            strRange.toLatin1().constData(),
            strMapTests.toLatin1().constData(),
            m_strWaferIdFilter.toLatin1().constData(),
            strTemperature.toLatin1().constData());


    fprintf(hFile,"  gexAnalyseMode('SingleFile');\n");
    fprintf(hFile,"\n  sysLog('* Quantix Examinator Files groups set ! *');\n\n");
}

///////////////////////////////////////////////////////////
// Auto adjust the column width to the content
///////////////////////////////////////////////////////////
void GexOneFileWizardPage1::onItemChanged(QTreeWidgetItem* /*pTreeWidgetItem*/,
                                          int nColumn)
{
        treeWidget->resizeColumnToContents(nColumn);
}
