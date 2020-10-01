/////////////////////////////////////////////////////////////////////////////
// Codes to handle PAT outlier filtering
/////////////////////////////////////////////////////////////////////////////
#include <gqtl_log.h>

#define SOURCEFILE_PAT_LIB_CPP
#include <math.h>

#include "sitetestresults.h"
#include "patman_lib.h"				// List of '#define' to return type of distribution (gaussian, lognormal, etc...)
#include "browser.h"
#include "engine.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_pat_constants_extern.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_shared.h"
#include "cstats.h"
#include "filter_dialog.h"
#include "scheduler_engine.h"
#include "tb_merge_retest_dialog.h"
#include "tb_merge_retest.h"
#include "pat_potatoclusterfinder.h"
#include "../../../pat_prod_admin/prod_recipe_file.h"	// PAT-Man Production recipe (eg: ST users)
#include "temporary_files_manager.h"
#include "import_semi_e142_xml.h"
#include "import_prober_tel.h"
#include "import_kla_inf_layers.h"
#include "pat_gdbn_engine.h"
#include "pat_gdbn_weighting_algo.h"
#include "pat_gdbn_generic_baddies.h"
#include "stdfrecords_v4.h"
#include "gex_version.h"
#include "cbinning.h"
#include "pat_rules.h"
#include "product_info.h"
#include "csl/csl_engine.h"
#include "script_wizard.h"
#include "message.h"
#include "pat_global.h"
#include "pat_definition.h"
#include "pat_engine.h"
#include "pat_process_ws.h"
#include "export_atdf.h"
#include <QProgressDialog>
#include <QApplication>
#include <QShortcut>
#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include "wafermap_parametric.h"

// Galaxy QT libraries
#include "gqtl_sysutils.h"
#include "stdfparse.h"

#include "browser_dialog.h"
#include "tb_pat_outlier_removal.h"
#include "tb_toolbox.h"	// Examinator ToolBox
#include "report_build.h"
#include "scripting_io.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "pat_options.h"
#include "pat_recipe_editor.h"
#include "parserFactory.h"
#include "parserAbstract.h"
#include "importSkyNPCsv.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;	// main.cpp
extern CGexReport *		gexReport;				// report_build.cpp: Handle to report class
extern void             ConvertToScriptString(QString &strFile);
extern QString          ConvertToScriptString(const QString &strFile);
extern CReportOptions	ReportOptions;			// Holds options (report_build.h)
extern bool ComparePatResultScore(CPatDefinition* test1, CPatDefinition* test2); // tb_pat_outlier_removal.cpp
extern bool ComparePatResultNum(CPatDefinition* test1, CPatDefinition* test2); // tb_pat_outlier_removal.cpp

#include <gtm_testerwindow.h>
//#include "gex_pat_constants_extern.h"

#define REGEXP_REF_LOCATION_FROM_COORD  "Coord\\((\\-?\\d{1,5}),(\\-?\\d{1,5})\\)"
#define REGEXP_REF_LOCATION_FROM_BIN    "Bin\\((\\d{1,5})\\)"

extern double	ScalingPower(int iPower);				// cstats.cpp

//extern CPatInfo	*lContext;

extern bool sortStaticPatDefinition(const CPatDefinition * pLeft, const CPatDefinition * pRight);

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
#include "DebugMemory.h"

#ifdef GCORE15334

GexTbPatDialog::GexTbPatDialog(QWidget* parent, bool modal, Qt::WindowFlags fl/*, CPatInfo *lPatInfo*/)
    : QDialog(parent, fl), m_cFields(this)
{
    GSLOG(6, "new GexTbPatDialog...");
    setupUi(this);
    setModal(modal);

    initComboBoxOutputWafermapFormat();

    setWindowFlags(Qt::FramelessWindowHint);

    // Support for Drag&Drop
    setAcceptDrops(true);

    // Default report output: HTML
    comboBoxReportFormat->setCurrentIndex(1);

    // Default file output: STDF
    comboBoxOutputTestDataFormat->setCurrentIndex(1);

    // Default wafermap file output: None
    comboBoxOutputWafermapFormat->setCurrentIndex(0);

    // Reticle Step Information
    comboBoxReticleStepInfo->clear();
    comboBoxReticleStepInfo->addItem("None", QVariant("None"));
    comboBoxReticleStepInfo->addItem("Input File", QVariant("input_file"));
    comboBoxReticleStepInfo->addItem("Map File", QVariant("map_file"));

    // Hide / show Widgets
    buttonRemoveFile->hide();
    textLabelFilesToProcess->hide();
    buttonCreateZPAT_File->hide();

    // Selection mode: select all columns at once
    treeWidgetDataFiles->setAllColumnsShowFocus(true);

    // signals and slots connections
    QObject::connect(buttonDetachWindow,		SIGNAL(clicked()),					this, SLOT(OnDetachWindow()));
    connect( buttonSelectFile, SIGNAL( clicked() ), this, SLOT( OnSelectFile() ) );
    connect( m_poAddNewGroup, SIGNAL( clicked() ), this, SLOT( OnAddGroup() ) );
    connect( buttonUp, SIGNAL( clicked() ), this, SLOT( OnMoveFileUp() ) );
    connect( buttonDown, SIGNAL( clicked() ), this, SLOT( OnMoveFileDown() ) );
    connect( buttonCreateTrigger, SIGNAL( clicked() ), this, SLOT( OnCreateTriggerFile() ) );
    connect( buttonCreateZPAT_File, SIGNAL( clicked() ), this, SLOT( OnCreateZPAT_File() ) );
    connect( buttonProperties, SIGNAL( clicked() ), this, SLOT( OnFileProperties() ) );
    connect( buttonSelectConfigFile, SIGNAL( clicked() ), this, SLOT( OnSelectConfigFile() ) );
    connect( buttonSelectCompositeFile, SIGNAL( clicked() ), this, SLOT( OnSelectCompositeExclusionFile() ) );
    connect( buttonEditExecutedScript, SIGNAL( clicked() ), this, SLOT( OnEditConfigFile() ) );
    connect( pushButtonProcessFile, SIGNAL( clicked() ), this, SLOT( OnProcessFile() ) );
    connect( buttonRemoveFile, SIGNAL( clicked() ), this, SLOT( OnRemoveFiles() ) );
    connect( PushButtonClearAll, SIGNAL( clicked() ), this, SLOT( OnRemoveAll() ) );
    connect( comboBoxOutputWafermapFormat, SIGNAL(activated(QString)), this, SLOT( OnWafermapFormat() ) );
    //    connect( treeWidgetDataFiles, SIGNAL( itemDoubleClicked(QTreeWidgetItem*, int) ),	this, SLOT( OnFileProperties(QTreeWidgetItem*, int)));
    connect( treeWidgetDataFiles, SIGNAL( itemChanged(QTreeWidgetItem*, int) ),			this, SLOT( onItemChanged(QTreeWidgetItem*, int)));
    connect( treeWidgetDataFiles, SIGNAL( currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT( showHideRemoveFile(QTreeWidgetItem*,QTreeWidgetItem*)));

    // At startip, this window is a child of Examinator Framr
    bChildWindow = true;
    buttonDetachWindow->setChecked(bChildWindow);

    // Reset all variables.
    clear(/*lPatInfo*/);

    // GUI setup
    OnWafermapFormat();

    // Adjust columns width
    treeWidgetDataFiles->resizeColumnToContents(0);
    treeWidgetDataFiles->resizeColumnToContents(1);
    treeWidgetDataFiles->resizeColumnToContents(2);
    treeWidgetDataFiles->resizeColumnToContents(3);
    treeWidgetDataFiles->resizeColumnToContents(4);

    // Disable file properties button
    buttonProperties->setEnabled(false);
}

///////////////////////////////////////////////////////////
// Close signal: If window is detached, this reparent it instead
///////////////////////////////////////////////////////////
void GexTbPatDialog::closeEvent(QCloseEvent *e)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    if(bChildWindow)
        e->accept();
    else
        if(pGexMainWindow!=NULL)
        {
            // Re-attac window to Examinator!
            buttonDetachWindow->setChecked(true);
            ForceAttachWindow(buttonDetachWindow->isChecked(), false);

            e->ignore();
        }
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void GexTbPatDialog::ForceAttachWindow(bool bAttach /*= true*/,
                                       bool bToFront /*= true*/)
{
    Q_UNUSED(bToFront);
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    bChildWindow = bAttach;
    buttonDetachWindow->setChecked(bChildWindow);

    if(bChildWindow && pGexMainWindow != NULL)
    {
        // Re-attacch dialog box to Examinator's scroll view Widget
        pGexMainWindow->pScrollArea->layout()->addWidget(this);

        // Minimum width is 720 pixels
        setMinimumWidth(720);
    }
    else
    {
        // Make sure the HTML page in Examinator's window remains visible
        if(pGexMainWindow != NULL && isVisible())
            pGexMainWindow->ShowHtmlBrowser();

        pGexMainWindow->pScrollArea->layout()->removeWidget(this);
        setParent(NULL, Qt::Dialog);

        // Setup the application buttons
        setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

        move(QPoint(100,100));
        show();

        // No Minimum width
        setMinimumWidth(10);
    }
}

void GexTbPatDialog::clear(/*CPatInfo *lPatInfo=NULL*/)
{
    //    // If caller is GTM (for real-time PAT processing), then get the handle to the PAT structure of the given station
    //    if(lPatInfo != NULL)
    //        lContext = lPatInfo;

    //    // Clear structures from previous call
    //    if(lContext != NULL)
    //        lContext->clear();

    // Reset variables
    m_iReloadDataFileID = -1;	// Holds FileID which dataset is in memory (only used when multi-batch PAT processing done)
}

///////////////////////////////////////////////////////////
// GexTbPatDialog: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void GexTbPatDialog::reject(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // Post-processing PAT: we had created our own object to hold receipe info, we need to delete it now!
    GS::Gex::PATEngine::GetInstance().DeleteContext();
    //  if(lContext != NULL)
    //      delete lContext;
    //  lContext = NULL;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexTbPatDialog::~GexTbPatDialog()
{
    // Ensure we delete all objects from memory...IN COMMENT for now, creates Q3Dict exception on exit!
    clear();
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexTbPatDialog::dragEnterEvent(QDragEnterEvent *e)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

QString GexTbPatDialog::SetRecipeFile(const QString &strFile)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return "ok";

    // Recipe file (.CSV)
    lineEditConfigFile->setText(strFile);

    // Always update recipe editor
    if (GS::Gex::PATRecipeEditor::IsInstantiated())
        GS::Gex::PATRecipeEditor::GetInstance().RefreshEditor(strFile);

    return "ok";
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexTbPatDialog::dropEvent(QDropEvent *e)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

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
    // Empty list if only drag & drop one data file.
    bool bEraseFileList=false;

    // Insert first file selected into the listbox
    QString strFile;
    QStringList oSTDFFileList;
    for (QStringList::Iterator it = strFileList.begin(); it != strFileList.end(); ++it )
    {
        strFile = *it;
        if(strFile.endsWith("_composite_pat.txt", Qt::CaseInsensitive))
            lineEditCompositeFile->setText(strFile);	// Composite/Exclusion Z-PAT map file (.TXT)
        else
            if(strFile.endsWith(".csv", Qt::CaseInsensitive) ||
                    strFile.endsWith(".json", Qt::CaseInsensitive))
            {
                SetRecipeFile(strFile);
            }
            else
            {
                oSTDFFileList.append(strFile);
            }
    }

    bool bMerge = true;
    bool bUserChoice = false;
    for(int iStdfIdx=0; iStdfIdx<oSTDFFileList.count(); ++iStdfIdx )
    {
        strFile = oSTDFFileList[iStdfIdx];
        if(!bUserChoice && (oSTDFFileList.count() >1) && !treeWidgetDataFiles->topLevelItemCount()){
            bMerge = userMergeOrCreateGroups();
            bUserChoice = true;
        }
        // Data file
        AddFile(strFile,bEraseFileList, bMerge);
    }

    e->acceptProposedAction();
}

void	GexTbPatDialog::OnSelectFile()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    GS::Gex::SelectDataFiles	cSelectFile;

    // User wants to analyze a single file
    QStringList strFiles;
    QString		strFile;

    // Get working folder
    QSettings settings;
    strFile = settings.value("toolbox/workingFolder").toString();

    strFiles = cSelectFile.GetFiles(this, strFile, "Select Test Data Files to process");
    if(strFiles.count() == 0)
        return;

    bool bMerge = true;
    if(strFiles.count() >1 ){
        bMerge = userMergeOrCreateGroups();
    }
    // Empty previous list.
    OnRemoveAll();

    // Add data files
    for (QStringList::Iterator it = strFiles.begin(); it != strFiles.end(); ++it ){
        AddFile(*it,false, bMerge);
    }

    // Update working folder
    QFileInfo cFileInfo(strFiles.first());

    settings.setValue("toolbox/workingFolder",cFileInfo.absolutePath());
}

///////////////////////////////////////////////////////////
// Move selection UP
///////////////////////////////////////////////////////////
void	GexTbPatDialog::OnMoveFileUp()
{
    // Get handle to relevant tree (Data files, or Map files)
    QTreeWidget *treeWidget;
    treeWidget = treeWidgetDataFiles;//(tabWidget->currentIndex() == 0) ? treeWidgetDataFiles : treeWidgetMapFiles;

    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 1)
        return;

    // Scan the list to move UP the selected item.
    QTreeWidgetItem *	pTreeWidgetItem		= static_cast<QTreeWidgetItem *>(treeWidget->topLevelItem(0));
    //FIXME: not used ?
    //QTreeWidgetItem *	pTreeWidgetPrevItem	= NULL;
    int					nPosPreviousItem	= -1;

    while(pTreeWidgetItem != NULL)
    {
        // Find first object selected (only consider first one)
        if(pTreeWidgetItem->isSelected() == true)
        {
            //FIXME: not used ?
            //pTreeWidgetPrevItem =
            //  static_cast<QTreeWidgetItem*>(
            //  treeWidget->itemAbove(pTreeWidgetItem));

            // if 1st file selected...can't move up..ignore!
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
            pTreeWidgetItem = static_cast<QTreeWidgetItem *>(treeWidget->itemBelow(pTreeWidgetItem));
        }
    };
}

///////////////////////////////////////////////////////////
// Move selection DOWN
///////////////////////////////////////////////////////////
void	GexTbPatDialog::OnMoveFileDown()
{
    // Get handle to relevant tree (Data files, or Map files)
    QTreeWidget *treeWidget;
    treeWidget = treeWidgetDataFiles;//(tabWidget->currentIndex() == 0) ? treeWidgetDataFiles : treeWidgetMapFiles;

    // If no item in list, just return!
    if(treeWidget->topLevelItemCount() <= 1)
        return;

    // Scan the list to move DOWN the selected item.
    QTreeWidgetItem *	pTreeWidgetItem		= NULL;
    QTreeWidgetItem *	pTreeWidgetNextItem	= NULL;
    int				nCurrentItem		= 0;

    pTreeWidgetItem = static_cast<QTreeWidgetItem *>(treeWidget->topLevelItem(treeWidget->topLevelItemCount()-1));

    while(pTreeWidgetItem != NULL)
    {
        // Find first object selected
        if(pTreeWidgetItem->isSelected() == true)
        {
            // Keep track of next item before removing it !
            pTreeWidgetNextItem = static_cast<QTreeWidgetItem *>(treeWidget->itemBelow(pTreeWidgetItem));

            if (pTreeWidgetNextItem  && pTreeWidgetNextItem->isSelected() == false)
            {
                // Move selected item after following file in the list
                // Get position of the current item
                nCurrentItem = treeWidget->indexOfTopLevelItem(pTreeWidgetItem);

                // Remove current group from the treewidget and insert it at its new position
                treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetNextItem));
                treeWidget->insertTopLevelItem(nCurrentItem, pTreeWidgetNextItem);
                return;
            }
        }

        // Move to next item
        pTreeWidgetItem = static_cast<QTreeWidgetItem *>(treeWidget->itemAbove(pTreeWidgetItem));
    };
}

void	GexTbPatDialog::OnCreateTriggerFile()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    if(!treeWidgetDataFiles->topLevelItemCount())
    {
        // Display error message!
        GS::Gex::Message::critical("", "Can't create trigger file: No Test Data file selected yet!");
        return;
    }

    // Get list of data files
    QStringList strDataFiles;

    // Build Zmap mask from active Data file list
    strGetFiles(treeWidgetDataFiles->topLevelItem(0), strDataFiles);    // Get list of data files (or Map files)

    if(strDataFiles.count() <= 0)
    {
        // Display error message!
        GS::Gex::Message::critical(
                    "", "Can't create trigger file: No Test Data file selected yet!");
        return;
    }

    // Get recipe name
    QString strRecipeFile = lineEditConfigFile->text();

    // Clean input file name strings in case they come from drag & drop (start with string 'file:///'
    if(strRecipeFile.startsWith("file:///"))
        strRecipeFile = QUrl(strRecipeFile).toLocalFile();

    // Remove leading \r & \n if any
    strRecipeFile.replace("\r","");
    strRecipeFile.replace("\n","");

    // If file doesn't exists, simply ignore request!
    QString	strString;
    if(QFile::exists(strRecipeFile) == false)
    {
        // Display error message!
        strString = "Can't create trigger file: ";
        if(strRecipeFile.isEmpty())
            strString += "No PAT recipe selected yet!";
        else
            strString += "PAT Recipe file doesn't exist!\nFile: " + strRecipeFile;
        GS::Gex::Message::critical("", strString);
        return;
    }

    // Select folder where to create trigger files. Point to same folder as first file selected
    QFileInfo	cFileInfo(strDataFiles[0]);

    // If merging files, ensure we add the 'merge' string to the trigger file name.
    bool bMergedTrigger=false;
    if(strDataFiles.count() > 1)
        bMergedTrigger = true;

    QString strTriggerFile;
    QFile file;
    QTextStream hTrigger;
    int iFileID=0;
    do
    {
        // Get folder where trigger file will be created
        cFileInfo.setFile(strDataFiles[iFileID]);

        // Trigger name to create: use file name, but remove extension.
        strTriggerFile = cFileInfo.absolutePath() + "/" + cFileInfo.completeBaseName();

        // If merging files, ensure we add the 'merge' string tothe trigger file name.
        if(bMergedTrigger)
            strTriggerFile += "_merged";

        // Trigger file extension
        strTriggerFile +=  ".gtf";

        // Create trigger file.
        file.setFileName(strTriggerFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        {
            QString strError = "Write error. Disk full/privilege issue?\nFile: " + strTriggerFile;
            GS::Gex::Message::critical("", strError);
            return;	// Failed writing to trigger file.
        }

        // Write Trigger File
        hTrigger.setDevice(&file);	// Assign file handle to data stream
        QDateTime cTime = QDateTime::currentDateTime();

        // Write header
        hTrigger << "######################################################################" << endl;
        hTrigger << "# PAT-Man Trigger file" << endl;
        hTrigger << "# Created with : "
                 << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl;
        hTrigger << "# Creation Date: " << cTime.toString("d MMMM yyyy h:mm:ss") << endl;
        hTrigger << "######################################################################" << endl;
        hTrigger << "<GalaxyTrigger>" << endl;
        hTrigger << "Action = PAT" << endl;

        // Write file (or list of files)
        if(bMergedTrigger)
        {
            // Merging files
            hTrigger << "# Define list of Data Files to merge" << endl;

            for(iFileID=0;iFileID<(int)strDataFiles.count();iFileID++)
            {
                hTrigger << "DataSource=" << strDataFiles[iFileID] << endl;
            }
        }
        else
        {
            // Create one trigger per file...
            hTrigger << "# Test Data File to process" << endl;
            hTrigger << "DataSource=" << strDataFiles[iFileID] << endl;
        }

        // Recipe file
        hTrigger << endl;
        hTrigger << "# Path to Recipe file..." << endl;
        hTrigger << "Recipe=" << strRecipeFile << endl;

        // Composite/Exclusion map?
        strString = lineEditCompositeFile->text();
        // Clean input file name strings in case they come from drag & drop (start with string 'file:///')
        if(strString.startsWith("file:///"))
            strString = QUrl(strString).toLocalFile();

        strString.replace("\r","");// Remove leading \r & \n if any
        strString.replace("\n","");
        if(QFile::exists(strString))
        {
            hTrigger << endl;
            hTrigger << "# Composite/Exclusion file:" << endl;
            hTrigger << "CompositeFile=" << strString << endl;
        }

        if (comboBoxReticleStepInfo->currentIndex() >= 0)
        {
            hTrigger << endl;
            hTrigger << "# Reticle Step Information:" << endl;
            hTrigger << "ReticleStepInfo=" << comboBoxReticleStepInfo->currentData().toString() << endl;
        }

        // Output STDF file?
        switch(getGenerateStdfFile())
        {
        case GEX_TPAT_DLG_OUTPUT_NONE:
            break;
        case GEX_TPAT_DLG_OUTPUT_STDF:
            // Output=STDF <folder_path>
            hTrigger << endl;
            hTrigger << "# Generate a output STDF datalog file with updated records (PAT binning, etc)"<< endl;
            hTrigger << "Output=STDF " << cFileInfo.absolutePath() << endl;
            break;
        case GEX_TPAT_DLG_OUTPUT_ATDF:
            // Output=ATDF <folder_path>
            hTrigger << endl;
            hTrigger << "# Generate a output ADTF datalog file with updated records (PAT binning, etc)"<< endl;
            hTrigger << "Output=ATDF " << cFileInfo.absolutePath() << endl;
            break;
        }

        // Output wafermap file?
        strString = getWafermapFormat().toUpper();
        if(strString.isEmpty() == false)
        {
            // Output=<wafer_format> <folder_path>
            hTrigger << endl;
            hTrigger << "# Create a wafermap file (specify wafer format and folder)"<< endl;
            hTrigger << "Output=" << strString << " " << cFileInfo.absolutePath() << endl;
        }

        hTrigger << endl;
        hTrigger << "######################################################################" << endl;
        hTrigger << "# Below are optional features..." << endl;
        hTrigger << "######################################################################" << endl;

        hTrigger << "# If you want a .log file created after PAT processing, uncomment next line and set output folder" << endl;
        hTrigger << "# LogFile = /server/pat/log_files" << endl;
        hTrigger << "#" << endl;
        hTrigger << "# Shell command: allows launching an application after PAT processing" << endl;
        hTrigger << "# Shell = /server/shell/my_shell.csh" << endl << endl;

        hTrigger << "######################################################################" << endl;
        hTrigger << "# For PAT-Man trigger file syntax, refer to PAT-Man manual at:" << endl;
        hTrigger << "# support.galaxysemi.com" << endl;
        hTrigger << "# Customer login required: check with " << GEX_EMAIL_SALES << " if needed." << endl;
        hTrigger << "######################################################################" << endl;

        // End of trigger file marker
        hTrigger << "</GalaxyTrigger>" << endl;

        // close file
        file.close();

        // Update File# to process
        iFileID++;
    }
    while(iFileID < (int)strDataFiles.count());

    // Success creating triggers.
    if(bMergedTrigger || (strDataFiles.count() == 1))
    {
        // Single trigger file created
        strString = "Success, trigger file created!\n\nFile: " + strTriggerFile;
    }
    else
    {
        // All Trigger files created.
        strString = "Success: All trigger files created!\nFiles are located in the same folder as your\ntest data files (look for extension *.gtf)";
    }
    GS::Gex::Message::information("", strString);
}

void	GexTbPatDialog::OnCreateZPAT_File()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    QStringList strFilesList;
    QString     lErrorMessage       = "";
    QString     strCompositeFile    = "";                           // Will create composite file in same folder as STDF files
    QString     strRecipeFile       = lineEditConfigFile->text();	// Recipe name

    // If file doesn't exists, simply ignore request!
    if(QFile::exists(strRecipeFile) == false)
    {
        // Display error message!
        lErrorMessage = "Can't create trigger file: ";
        if(strRecipeFile.isEmpty())
            lErrorMessage += "No PAT recipe selected yet!";
        else
            lErrorMessage += "PAT Recipe file doesn't exist!\nFile: " + strRecipeFile;

        GS::Gex::Message::critical("", lErrorMessage);
        return;
    }

    int         lGroupIdx = 0;
    QStringList lGroupFiles;
    while (lGroupIdx < treeWidgetDataFiles->topLevelItemCount())
    {
        // Get list of files in GUI
        strGetFiles(treeWidgetDataFiles->topLevelItem(lGroupIdx), lGroupFiles);

        strFilesList.append(lGroupFiles);

        ++lGroupIdx;
    }

    // Build Z-PAT map
    if (GS::Gex::PATEngine::GetInstance().CreateZPATCompositeFile(
                strRecipeFile, strFilesList, strCompositeFile) == false)
        GS::Gex::Message::information("", GS::Gex::PATEngine::GetInstance().GetErrorMessage());
    else
    {
        lErrorMessage = "Success: ZPAT / Composite map file created:\n" + strCompositeFile;
        lErrorMessage += "\n\nDo you want to select this ZPAT file now?";
        bool lOk;
        GS::Gex::Message::request("", lErrorMessage, lOk);
        if (lOk)
        {
            lineEditCompositeFile->setText(strCompositeFile);
        }
    }
}

void	GexTbPatDialog::OnFileProperties(QTreeWidgetItem * poItem, int iColumn)
{
    // Note: Properties disabled for now: ensures user not overloading PAT internal
    // file filtering which is 'last_restest'

    // Get file name selected
    if(!poItem && sender()) {
        QWidget *poSender = qobject_cast<QWidget*> (sender()->parent());
        GS::Gex::SelectDataFiles	cSelectFile;
        QString		strFile;

        // Get working folder
        QSettings settings;
        strFile = settings.value("toolbox/workingFolder").toString();

        strFile = cSelectFile.GetSingleFile(this, strFile, "Select Test Data Files to process");
        if(!strFile.isEmpty())
        {
            if (strFile.contains(" "))
            {
                QMessageBox::information(this, tr("Wrong path name"), tr("Spaces are not allowed as part of the path."));
                return;
            }
            QFileInfo cFileInfo(strFile);
            settings.setValue("toolbox/workingFolder",cFileInfo.absolutePath());
            QLabel *poLabel = poSender->findChild<QLabel*>();
            poLabel->setText(strFile);
        }
    }
    return;

    // If file properties need to be enabled, do not return, but execute remaining code.
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // If no selection, just ignore action.
    QTreeWidgetItem	* pTreeWidgetItemSelection = treeWidgetDataFiles->currentItem();

    if(pTreeWidgetItemSelection == NULL)
        return;

    // Get file name selected
    if(pTreeWidgetItemSelection->parent()){
        if(poItem && iColumn == 6) {
            GS::Gex::SelectDataFiles	cSelectFile;
            QString                     strFile;

            // Get working folder
            QSettings settings;
            strFile = settings.value("toolbox/workingFolder").toString();

            strFile = cSelectFile.GetSingleFile(this, strFile, "Select Test Data Files to process");
            if(!strFile.isEmpty()){
                poItem->setText(6, strFile);
                QFileInfo cFileInfo(strFile);
                settings.setValue("toolbox/workingFolder",cFileInfo.absolutePath());
            }
        }
        return;
    }
    else
    {
        QString strFileName = pTreeWidgetItemSelection->text(5);

        // Show selection made...
        FilterDialog *dFilter = new FilterDialog();

        // Give to the filter dialog the file info: file name, site#,etc...
        int iSelectedItems=1;
        int	iFlags = GEX_FILE_FILTER_NOSITE	| GEX_FILE_FILTER_NOGROUPNAME | GEX_FILE_FILTER_NOADVANCEDTAB;
        dFilter->SetParameters(iSelectedItems,false,0,pTreeWidgetItemSelection,iFlags);

        // Prompt Filter dialog box
        if(dFilter->exec() != 1)
            return;	// User 'Abort'
    }
}

void	GexTbPatDialog::OnSelectConfigFile()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // User wants to analyze a single file
    QSettings settings;
    QString strFile = settings.value("editor/workingFolder").toString();	// Recipe working folder

    strFile = QFileDialog::getOpenFileName(this, "Select recipe file", strFile, "Recipe file (*.csv *.json)");
    if(strFile.isEmpty() == false)
        SetRecipeFile(strFile);
}

void	GexTbPatDialog::OnSelectCompositeExclusionFile()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    QString strFile = "";
    strFile = QFileDialog::getOpenFileName(this, "Select Composite/Exclusion file", strFile, "Composite data file (*.txt)");
    if(strFile.isEmpty() == false)
        lineEditCompositeFile->setText(strFile);
}

void	GexTbPatDialog::OnEditConfigFile()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    QString strFile = lineEditConfigFile->text();

    // Clean input file name strings in case they come from drag & drop (start with string 'file:///'
    if(strFile.startsWith("file:///"))
        strFile = QUrl(strFile).toLocalFile();

    // Remove leading \r & \n if any
    strFile.replace("\r","");
    strFile.replace("\n","");

    // If file doesn't exists, simply ignore request!
    if(QFile::exists(strFile) == false)
        return;

    // Switch to the Spreadsheet-type sheet page (create it if needed)
    pGexMainWindow->Wizard_GexTb_EditPAT_Limits();

    // Detach the Editor window so we can still see the PAT: Process file wizard page
    GS::Gex::PATRecipeEditor::GetInstance().DetachRecipeEditor();

    // Re-Load Config file in the Spreadsheet-type table & internal structures
    GS::Gex::PATRecipeEditor::GetInstance().RefreshEditor(strFile);

    // Force to reload the wizard PAT page
    pGexMainWindow->Wizard_GexTb_PAT();
}

///////////////////////////////////////////////////////////
// Enable/Disable GUI buttons
///////////////////////////////////////////////////////////
void	GexTbPatDialog::enableGuiField(bool bEnable)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    groupBoxProcessPAT->setEnabled(bEnable);
    groupBoxProcessPAT_Output->setEnabled(bEnable);
    pushButtonProcessFile->setEnabled(bEnable);

    // Enable/Disable the 'PAT Process' button to avoid double-clicking on it!
    if (GS::Gex::PATRecipeEditor::IsInstantiated())
        GS::Gex::PATRecipeEditor::GetInstance().mApplyPAT->setEnabled(bEnable);
}

///////////////////////////////////////////////////////////
// Removing file(s) form the list
///////////////////////////////////////////////////////////
void GexTbPatDialog::OnRemoveFiles(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // Get handle to relevant tree (Data files, or Map files)
    QTreeWidget *treeWidget = treeWidgetDataFiles;

    // If no item in list, just return!
    if(treeWidget && treeWidget->topLevelItemCount() <= 0)
        return;

    QWidget *poWidget = qobject_cast<QWidget*>(sender());
    if(poWidget == buttonRemoveFile){
        QList<QTreeWidgetItem *> oSelectedItems = treeWidget->selectedItems();
        if(oSelectedItems.count()){
            qDeleteAll(oSelectedItems);
        }
        return;
    }

    // Scan the list to remove selected items.
    QTreeWidgetItem  * pTreeWidgetItem		= treeWidget->topLevelItem(0);
    QTreeWidgetItem  * pTreeWidgetNextItem	= NULL;

    while(pTreeWidgetItem != NULL)
    {
        // Keep track of next item before removing it !
        pTreeWidgetNextItem = treeWidget->topLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItem)+1);

        // Remove selected item from list
        treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(pTreeWidgetItem));

        // Destroy it from memory.
        delete pTreeWidgetItem;

        // Update pointer to process next item
        pTreeWidgetItem = pTreeWidgetNextItem;
    };
}

void GexTbPatDialog::OnRemoveAll(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // Get handle to relevant tree (Data files, or Map files)
    QTreeWidget *treeWidget;
    treeWidget = treeWidgetDataFiles;//(tabWidget->currentIndex() == 0) ? treeWidgetDataFiles : treeWidgetMapFiles;

    // Select first item in list
    QTreeWidgetItem *ptItem = treeWidget->topLevelItem(0);
    if(ptItem)
        treeWidget->setCurrentItem(ptItem);

    // Select all entries
    treeWidget->selectAll();

    // Remove selected entries!
    OnRemoveFiles();
}


void	GexTbPatDialog::OnDropTestDataFiles(QDropEvent * e)
{
    GSLOG(6, "OnDropTestDataFiles...");

    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // Same function as if dropping files over the dialog box!
    dropEvent(e);
}


///////////////////////////////////////////////////////////
// Add file to the list.
///////////////////////////////////////////////////////////
QWidget *GexTbPatDialog::createMapEntry(){

    QWidget *poWidget = new QWidget;

    QHBoxLayout* poHorizontalLayout = new QHBoxLayout(poWidget);
    poHorizontalLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *poMapPath = new QLabel(poWidget);
    poMapPath->setText("N/A");
    poHorizontalLayout->addWidget(poMapPath);

    QToolButton *poMapFile = new QToolButton(poWidget);
    poMapFile->setText("...");
    poHorizontalLayout->addWidget(poMapFile);

    QObject::connect(poMapFile, SIGNAL(clicked()), this, SLOT(OnFileProperties()));

    return poWidget;
}

QTreeWidgetItem *GexTbPatDialog::AddGroup(int iGroup){

    QTreeWidgetItem * pTreeWidgetItemGroup = 0;

    QString strDataSetName = "Group_" + QString::number(iGroup);
    pTreeWidgetItemGroup = new QTreeWidgetItem(treeWidgetDataFiles);
    pTreeWidgetItemGroup->setText(0, strDataSetName);
    treeWidgetDataFiles->setCurrentItem(pTreeWidgetItemGroup, 0);
    treeWidgetDataFiles->setItemWidget(pTreeWidgetItemGroup,6,createMapEntry());

    return pTreeWidgetItemGroup;
}

void	GexTbPatDialog::AddFile(QString strFile, bool bClearList/*=true*/, bool bMerge/*=true*/)
{
    GSLOG(6, QString("Add File %1...").arg(strFile).toLatin1().data());

    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // Empty list of existing files
    if(bClearList)
        OnRemoveAll();

    // Get handle to relevant tree (Data files, or Map files)
    QTreeWidgetItem * pTreeWidgetItem;
    switch(tabWidget->currentIndex())
    {
    case 0: //Data files
    {
        // Add file to the list.
        QTreeWidgetItem * pTreeWidgetItemGroup = 0;
        if(treeWidgetDataFiles->topLevelItemCount() <= 0)
            pTreeWidgetItemGroup = AddGroup(0);
        else if(!bMerge)
        {
            pTreeWidgetItemGroup = AddGroup(treeWidgetDataFiles->topLevelItemCount());
        }
        else
        {
            pTreeWidgetItemGroup = treeWidgetDataFiles->currentItem();
            if(!pTreeWidgetItemGroup || pTreeWidgetItemGroup->parent()){
                pTreeWidgetItemGroup = AddGroup(treeWidgetDataFiles->topLevelItemCount());
            }
        }

        pTreeWidgetItem = new QTreeWidgetItem;
        pTreeWidgetItemGroup->addChild(pTreeWidgetItem);
        pTreeWidgetItem->setText(1, "All");
        pTreeWidgetItem->setText(2, ProcessPartsItems[GEX_PROCESSPART_LASTINSTANCE]);
        pTreeWidgetItem->setText(3, " ");
        pTreeWidgetItem->setText(4, " ");
        pTreeWidgetItem->setText(5, strFile);
        pTreeWidgetItemGroup->setExpanded(true);
    }
        break;

        //        case 1:// Map files
        //        {
        //            if(strFile.count(" ")){
        //                //Check if map files name contains spaces.
        //                QString strMessage = "Map files must include no spaces, neither in the path, nor in the file name.\n"
        //                        "Please remove spaces, then insert your map file again.";
        //                Message::information(this, szAppFullName, strMessage);
        //                return ;
        //            }
        //            pTreeWidgetItem = new QTreeWidgetItem(treeWidgetMapFiles);
        //            pTreeWidgetItem->setText(0, strFile);
        //        }
        //        break;
    }

    // If more than one file in list, display the 'merge' checkbox, otherwise hide it!
    if(treeWidgetDataFiles->topLevelItemCount() >= 2)
    {
        //checkBoxMergeFiles->show();

        // Displpay total number of files selected
        QString strMessage = QString::number(treeWidgetDataFiles->topLevelItemCount());
        strMessage += " Files to process.";
        textLabelFilesToProcess->setText(strMessage);
        textLabelFilesToProcess->show();
        buttonCreateZPAT_File->show();
    }
    else
    {
        //        checkBoxMergeFiles->hide();
        //        checkBoxMergeFiles->setChecked(false);
        textLabelFilesToProcess->hide();
        buttonCreateZPAT_File->hide();
    }

}

///////////////////////////////////////////////////////////
// Get files in listview
///////////////////////////////////////////////////////////
void	GexTbPatDialog::strGetFiles(QTreeWidgetItem *poGroupItem,QStringList & lFiles)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // Get handle to relevant tree (Data files, or Map files)
    int lFileNameID = 5;    // File name is in Column#5

    if(poGroupItem == NULL || poGroupItem->childCount() == 0)
        return;

    QTreeWidgetItem *   lChildItem  = NULL;
    int                 lChildIdx   = 0;
    QString             lFile;

    lFiles.clear();
    while(lChildIdx < poGroupItem->childCount())
    {
        lChildItem = poGroupItem->child(lChildIdx);

        // Get file in list
        lFile = lChildItem->text(lFileNameID);
        if(lFile.startsWith("file:///"))
            lFile = QUrl(lFile).toLocalFile();

        // Remove leading \r & \n if any
        lFile.replace("\r","");
        lFile.replace("\n","");

        lFiles += lFile;

        ++lChildIdx;
    };
}

///////////////////////////////////////////////////////////
// Get Nth file in list view.
///////////////////////////////////////////////////////////
void GexTbPatDialog::strGetFile(QString & lFile, int lGroupIdx /*= 0*/,
                                int lFileIdx/*=0*/)
{
    if (lFileIdx < 0)
        return;

    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // Get handle to relevant tree (Data files, or Map files)
    int                 lFileNameID = 4;
    QTreeWidgetItem *   pTWIGroup   = treeWidgetDataFiles->topLevelItem(lGroupIdx);
    QTreeWidgetItem *   pTWIFile    = NULL;

    if (pTWIGroup != NULL)
    {
        for (int lIdxChild = 0; lIdxChild < pTWIGroup->childCount(); ++lIdxChild)
        {
            // Check if relevant file reached
            if(lIdxChild == lFileIdx)
            {
                pTWIFile    = pTWIGroup->child(lIdxChild);
                lFile       = pTWIFile->text(lFileNameID);
                return;
            }
        }
    };
}

///////////////////////////////////////////////////////////
// Tells if STDF output requested
///////////////////////////////////////////////////////////
int	GexTbPatDialog::getGenerateStdfFile(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return GEX_TPAT_DLG_OUTPUT_NONE;

    switch(comboBoxOutputTestDataFormat->currentIndex())
    {
    default:
    case 0:	// Disabled: Delete temporary STDF after PAT step
        return GEX_TPAT_DLG_OUTPUT_NONE;

    case 1:	// Create STDF output file (keep temporary STDF file created!)
        return GEX_TPAT_DLG_OUTPUT_STDF;

    case 2:	// Create ATDF output file
        return GEX_TPAT_DLG_OUTPUT_ATDF;
    }
}

///////////////////////////////////////////////////////////
// User selecting wafermap output format
///////////////////////////////////////////////////////////
void GexTbPatDialog::OnWafermapFormat(void)
{
    QString waferType = comboBoxOutputWafermapFormat->itemData(comboBoxOutputWafermapFormat->currentIndex()).toString();

    if((waferType == "") || (waferType == "stif"))
        comboBoxOutputWafermapType->hide();
    else
        comboBoxOutputWafermapType->show();
}

///////////////////////////////////////////////////////////
// Tells wafermap output format to create
QString	GexTbPatDialog::getWafermapFormat(void)
{
    QString strFormat = "";

    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return strFormat;

    strFormat = comboBoxOutputWafermapFormat->itemData(comboBoxOutputWafermapFormat->currentIndex()).toString();

    // Return wafermap format to generate.
    return strFormat;
}

///////////////////////////////////////////////////////////
// Tells wafermap output type (SBIN or HBIN)
///////////////////////////////////////////////////////////
bool	GexTbPatDialog::getWafermapBinType(void)
{
    QString strFormat;

    switch(comboBoxOutputWafermapType->currentIndex())
    {
    default:
    case 0:	// Soft-Bin
        return true;
        break;
    case 1:	// Hard-Bin
        return false;
        break;
    }
    GSLOG(5, "unknown comboBoxOutputWafermapType ! returning default bin type (soft).");
    return true;
}

///////////////////////////////////////////////////////////
// HTML summary line. Caller is 'OnProcessFile' function below
///////////////////////////////////////////////////////////
static void WriteHtmlPatSummaryLine(FILE *hFile,
                                    float fYield, float fDPAT_PatYieldLoss, float fMVPAT_PatYieldLoss, float fNNR_PatYieldLoss,
                                    float fIDDQ_PatYieldLoss,float fGDBN_PatYieldLoss,float fClustering_PatYieldLoss,
                                    float fReticle_PatYieldLoss,float fTotalZPAT_YieldLoss,int lTotalBins,
                                    int iYieldCount,int iDPAT_PatYieldLossCount, int iMVPAT_PatYieldLossCount, int iNNR_PatYieldLossCount,
                                    int iIDDQ_PatYieldLossCount, int iGDBN_PatYieldLossCount,int iClustering_PatYieldLossCount,
                                    int iReticle_PatYieldLossCount, int iTotalZPAT_YieldLossCount)
{
    // Check if HTML report page to be created...
    if(hFile == NULL)
        return;

    fprintf(hFile,"<tr>\n");
    fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"left\"><b>Summary</b></td>\n");
    fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"left\">All</td>\n");
    fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"left\">All</td>\n");
    fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"center\">All</td>\n");
    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">All</td>\n");
    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d</td>\n",lTotalBins);

    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d<br><b>(%.1f%%)</b></td>\n",iYieldCount,fYield);

    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d<br><b>(%.1f%%)</b></td>\n",iDPAT_PatYieldLossCount, fDPAT_PatYieldLoss);
    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d<br><b>(%.1f%%)</b></td>\n",iMVPAT_PatYieldLossCount, fMVPAT_PatYieldLoss);
    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d<br><b>(%.1f%%)</b></td>\n",iNNR_PatYieldLossCount, fNNR_PatYieldLoss);
    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d<br><b>(%.1f%%)</b></td>\n",iIDDQ_PatYieldLossCount, fIDDQ_PatYieldLoss);
    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d<br><b>(%.1f%%)</b></td>\n",iGDBN_PatYieldLossCount, fGDBN_PatYieldLoss);
    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d<br><b>(%.1f%%)</b></td>\n",iClustering_PatYieldLossCount, fClustering_PatYieldLoss);
    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d<br><b>(%.1f%%)</b></td>\n",iReticle_PatYieldLossCount, fReticle_PatYieldLoss);
    fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d<br><b>(%.1f%%)</b></td>\n",iTotalZPAT_YieldLossCount, fTotalZPAT_YieldLoss); // Z-PAT

    fprintf(hFile,"<td width=\"18%%\" bgcolor=\"#F8F8F8\" align=\"center\">-</td>\n");
    fprintf(hFile,"<td width=\"8%%\" bgcolor=\"#F8F8F8\" align=\"center\">-</td>\n");
    // case 5909
    //fprintf(hFile,"<td width=\"9%%\" bgcolor=\"#F8F8F8\" align=\"center\">-</td>\n");
    fprintf(hFile,"</tr>\n");
}

QString GexTbPatDialog::retrieveMapFileName(QTreeWidgetItem *poItem){
    QWidget *poWidget = treeWidgetDataFiles->itemWidget(poItem, 6);
    QLabel *poLabel = poWidget->findChild<QLabel*>();
    if(poLabel->text() != "N/A")
        return poLabel->text();
    else
        return QString();

}

void	GexTbPatDialog::OnProcessFile()
{
    GSLOG(6, "On Process File...");
    m_cFields.clear();

    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return;

    // Disable GUI buttons to avoid user clicking multiple times the 'OK' button!
    enableGuiField(false);

    // Used to measure the batch processing time.
    QTime	cTime;
    cTime.start();

    // check if the line edit is not empty because the user can paste the lineEdit
    if(GS::Gex::PATRecipeEditor::IsInstantiated() && !lineEditConfigFile->text().isEmpty())
    {
        QString lRecipeFile = lineEditConfigFile->text();
        GS::Gex::PATRecipeEditor::GetInstance().RefreshEditor(lRecipeFile);
    }

    // Check if recipe editor currently opened with recipe edits to be saved...reload GUI with returned recipe name in case it has changed (version# increased)
    if(GS::Gex::PATRecipeEditor::IsInstantiated())
        lineEditConfigFile->setText(GS::Gex::PATRecipeEditor::GetInstance().CheckSaveForModifiedRecipe());

    // Readback file names from the edit box.
    //QTreeWidgetItem *	pTreeWidgetItem = treeWidgetDataFiles->topLevelItem(0);
    QStringList		strInputFilesList;
    QStringList		strInputMapFiles;
    //QString			strInputFile; // unused
    QList<QPair<QString, QStringList> > lGroupInputFiles;
    QMap<QString, QString>              lGroupMapFiles;

    for(int iGroup=0; iGroup < treeWidgetDataFiles->topLevelItemCount(); iGroup++)
    {
        QTreeWidgetItem *poItem = treeWidgetDataFiles->topLevelItem(iGroup);
        if(!poItem)
            continue;
        QString strGroup    = poItem->text(0);
        QString strMap      = retrieveMapFileName(poItem);

        if (!strMap.isEmpty() && checkBoxUpdateExternalMap->isChecked() == false)
            strMap += " no_update";

        // Get list of Data files
        strGetFiles(poItem, strInputFilesList);    // Get list Data file

        if(strInputFilesList.isEmpty())
        {
            GS::Gex::Message::information("", "Can not process an empty group");
            // Re-enable GUI buttons
            enableGuiField(true);
            return;
        }

        lGroupInputFiles.append(QPair<QString, QStringList>(strGroup, strInputFilesList));

        if(!strMap.isEmpty())
            lGroupMapFiles.insert(strGroup, strMap);
    }

    // for all groups
    strInputFilesList.clear();
    strInputMapFiles.clear();

    // Check all optional maps can be updated
    if (checkBoxUpdateExternalMap->isChecked())
    {
        bool    bUpdatableMap = true;
        QString strOptionalMap;

        GS::Parser::ParserFactory *lParserFactory = GS::Parser::ParserFactory::GetInstance();
        GS::Parser::ParserAbstract *lParser = NULL;
        bool lIsMapUpdaterCompatible = false;

        foreach (const QString &strGroup, lGroupMapFiles.keys())
        {
            lIsMapUpdaterCompatible = false;
            QString strInputMapFile = lGroupMapFiles[strGroup];
            strOptionalMap = strInputMapFile.section(" ", 0, 0);

            // Check if the file exists and if it is readable
            if(!QFile::exists(strOptionalMap))
            {
                QString strMessage = "Some of the selected map files don't exist.\n\n";
                strMessage += "Please uncheck the 'Update map' option.\n";
                strMessage += "If you need an output map with the same format as your map file, please select it in the 'Wafer map file' option.";
                GS::Gex::Message::information("", strMessage);

                // Re-enable GUI buttons
                enableGuiField(true);
                return;
            }
            QFile lOFile(strOptionalMap);
            if(!lOFile.open(QIODevice::ReadOnly))
            {
                QString strMessage = "Some of the selected map files cannot be opened.\n\n";
                strMessage += "Please uncheck the 'Update map' option.\n";
                strMessage += "If you need an output map with the same format as your map file, please select it in the 'Wafer map file' option.";
                GS::Gex::Message::information("", strMessage);

                // Re-enable GUI buttons
                enableGuiField(true);
                return;
            }
            lOFile.close();

            if(lParserFactory != NULL)
            {
                lParser = lParserFactory->CreateParser(strOptionalMap.toLatin1().constData());
            }

            if(lParser != NULL && lParser->IsCompatibleMapUpdater())
            {
                lIsMapUpdaterCompatible = true;
            }
            bUpdatableMap &= CGKlaInfLayerstoSTDF::IsCompatible(strOptionalMap.toLatin1().constData()) ||
                    CGSemiE142XmltoSTDF::IsCompatible(strOptionalMap.toLatin1().constData()) ||
                    CGProberTelThousandToSTDF::IsCompatible(strOptionalMap.toLatin1().constData()) ||
                    CGProberTelMillionToSTDF::IsCompatible(strOptionalMap.toLatin1().constData()) ||
                    lIsMapUpdaterCompatible;

            if (bUpdatableMap == false)
            {
                QString strMessage = "Some of the selected map files do not support update.\nSupported formats are: TELP8, SEMI E142, KLA INF, SkyNP, Woburn SCESCII.\n\n";
                strMessage += "Please uncheck the 'Update map' option.\n";
                strMessage += "If you need an output map with the same format as your map file, please select it in the 'Wafer map file' option.";
                GS::Gex::Message::information("", strMessage);

                // Re-enable GUI buttons
                enableGuiField(true);
                return;
            }
        }
    }

    /*
        specify source
    */

    // Readback Composite/Exclusion file
    QString strInputCompositeFile = lineEditCompositeFile->text();
    // Clean input file name strings in case they come from drag & drop (start with string 'file:///')
    if(strInputCompositeFile.startsWith("file:///"))
        strInputCompositeFile = QUrl(strInputCompositeFile).toLocalFile();

    // Remove leading \r & \n if any
    strInputCompositeFile.replace("\r","");
    strInputCompositeFile.replace("\n","");
    m_cFields.strCompositeFile = strInputCompositeFile;	// Composite/Exclusion file.

    // Readback Reticle Step Information
    m_cFields.mReticleStepInfo = comboBoxReticleStepInfo->currentData().toString();

    // Readback Recipe name from the edit box.
    QString strConfigFile = lineEditConfigFile->text();
    // Clean input file name strings in case they come from drag & drop (start with string 'file:///')
    if(strConfigFile.startsWith("file:///"))
        strConfigFile = QUrl(strConfigFile).toLocalFile();

    // Remove leading \r & \n if any
    strConfigFile.replace("\r","");
    strConfigFile.replace("\n","");
    m_cFields.strRecipeFile = strConfigFile;	// Recipe file to use.

    // Report format: CSV, HTML, Word, PDF, etc...
    switch(comboBoxReportFormat->currentIndex())
    {
    case 0:	// Disabled
        m_cFields.strOutputReportFormat = "interactive";
        break;
    case 1:	// HTML
        m_cFields.strOutputReportFormat = "html";
        break;
    case 2:	// WORD report to create & append
        m_cFields.strOutputReportFormat = "word";
        break;
    case 3:	// CSV report to create & append
        m_cFields.strOutputReportFormat = "excel";
        break;
    case 4:	// PowerPoint report to create & append
        m_cFields.strOutputReportFormat = "ppt";
        break;
    case 5:	// PDF report to create & append
        m_cFields.strOutputReportFormat = "pdf";
        break;
    }

    // STDF report to be created?
    m_cFields.mOutputDatalogFormat = getGenerateStdfFile();

    // Output wafermap file format: Disabled, TSMC, Amkor, ...
    QPair<GS::Gex::PATProcessing::MapBinType, QString> lMapInfo(GS::Gex::PATProcessing::MapDefaultBin, "");
    m_cFields.mOutputFormat.insert(getWafermapFormat(), lMapInfo);

    // Output wafermap format: SoftBin or HardBin
    m_cFields.bOutputMapIsSoftBin = getWafermapBinType();

    // Security check: ensure STIF output is always for HBIN map only
    if( m_cFields.mOutputFormat.contains("stif"))
        m_cFields.bOutputMapIsSoftBin = false;

    // Check if original limits must be reported
    if (checkBoxReportOriginalLimits->isChecked())
        m_cFields.bReportStdfOriginalLimits = true;
    else
        m_cFields.bReportStdfOriginalLimits = false;

    // Process file: Identify Outliers & remove them
    QList<CPatDefinition*>			cDPatResults;// Holds summary of DPAT test failures (all files merged).
    tdPatResultList			cNnrResults;// Holds summary of NNR test failures (all files merged).
    QList<CPatDefinition*>			cIddqResults;// Holds summary of IDDQ test failures (all files merged).

    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
    //    CPatDefinition *        ptPatDefList                = 0;
    CPatDefinition *        ptPatDef                    = 0;
    CPatDefinition *        ptFailTest                  = 0;
    CPatOutlierNNR *		ptNRR_OutlierPart           = 0;
    CPatOutlierIDDQ_Delta *	ptIDDQ_Delta_OutlierPart    = 0;

    QFileInfo cFileInfo;
    FILE	*hFile=NULL;
    QString strTocPage="";
    QString strString;
    QString	strSubLot;
    QString strError;
    int iStatus  = NoError;

    {
        // Process one file at a time.
        bool	bLaunchReportViewer = true;	// Set to 'false' in case of Batch report processing.
        bool	bIgnoreTest;
        QString	strLot="";
        QString	strProduct="";
        QString strString;
        int	iGroupID;
        int	lGroupCount=0;
        int     i,iBin,lTotalBins,lTotalGoodBins;
        //FIXME: not used ?
        //int     iDpatFailures;
        int     iGeographicFailures;
        int	iTotalWafersInLot=0;
        float	fOriginalYield,fTotalYield=0.0;
        float	fPatYieldLoss;
        // Strings to build hyperlinks
        QString	strHyperlinkBegionSection,strHyperlinkEndSection;
        // Total yield losses for each PAT algorithms
        float	fTotalDPat_YieldLoss=0.0;
        float	fTotalNNR_YieldLoss=0.0;
        float	fTotalIDDQ_Delta_YieldLoss=0.0;
        float	fTotalGDBN_YieldLoss=0.0;
        float	fTotalZPAT_YieldLoss=0.0;
        float	fTotalReticle_YieldLoss=0.0;
        float	fTotalCluster_YieldLoss=0.0;
        float	fTotalMVPAT_YieldLoss=0.0;
        int     iTotalParts = 0;
        int     iOriginalYieldCount=0;
        int     iTotalDPat_YieldLossCount=0;
        int     iTotalNNR_YieldLossCount=0;
        int     iTotalIDDQ_Delta_YieldLossCount=0;
        int     iTotalGDBN_YieldLossCount=0;
        int     iTotalCluster_YieldLossCount=0;
        int     iTotalReticle_YieldLossCount=0;
        int     iTotalZPAT_YieldLossCount=0;
        int     iTotalMVPAT_YieldLossCount=0;
        CPatInfo* lContext = NULL;
        // Get total files to process (either STDF or Map files!)
        lGroupCount = lGroupInputFiles.count();

        QProgressDialog cProgress( "Processing files...", "Abort", 0, lGroupCount+1, this);
        cProgress.setModal(true);

        if(lGroupCount == 1)
            cProgress.hide();

        for(iGroupID = 0; iGroupID < lGroupCount; iGroupID++)
        {
            strInputFilesList = lGroupInputFiles.at(iGroupID).second;

            retriveFieldsOptions(lGroupInputFiles.at(iGroupID).first, strInputFilesList);

            // Display progress bar showing how many of the files are processed so far...
            if(lGroupCount > 1)
            {
                cProgress.setValue(iGroupID+1);

                // Show which file# is being processed...
                strString.sprintf("Processing Groups: %d of %d",iGroupID+1, lGroupCount);
                cProgress.setLabelText (strString);

                // Ensure NOT to launch the editor as this is a batch processing!
                bLaunchReportViewer = false;
            }
            else
                bLaunchReportViewer = true;

            GSLOG(7, QString("On process file : LaunchReportViewer= %1").arg(bLaunchReportViewer).toLatin1().data() );

            // check for Break condition
            if(cProgress.wasCanceled())
                break;

            // Clear previous PAT session if exists.
            clear();
            // Get SINGLE file to process
            m_cFields.strSources.clear();
            m_cFields.strSources << strInputFilesList;

            // Get Map file if any exist
            if(lGroupMapFiles.count() >0 && lGroupMapFiles.contains(lGroupInputFiles.at(iGroupID).first) > 0)
            {
                m_cFields.strOptionalSource = lGroupMapFiles[lGroupInputFiles.at(iGroupID).first];	// Optional input: KLA/INF, etc...

                // If no parametric file, select map one as fall-back
                if(m_cFields.strSources.count() == 0)
                    m_cFields.strSources << m_cFields.strOptionalSource;
            }
            else
                m_cFields.strOptionalSource=""; // No external map!

            // Process file (one at a time)
            if (GS::Gex::PATEngine::GetInstance().GetContext() != NULL)
                GS::Gex::PATEngine::GetInstance().DeleteContext();
            GS::Gex::PATEngine::GetInstance().CreateContext();
            lContext = GS::Gex::PATEngine::GetInstance().GetContext();
            GS::Gex::PATProcessWS lPATProcess(GS::Gex::PATEngine::GetInstance().GetContext());
            if (lPATProcess.Execute(m_cFields) == false)
            {
                strError = lPATProcess.GetErrorMessage();
                iStatus = WriteError;
                break;
            }

            // Generate a PAT report if output report format is defined
            if (m_cFields.strOutputReportFormat.isEmpty() == false)
            {
                if (GS::Gex::PATEngine::GetInstance().BuildPATReport(lContext->GetOutputDataFilename(),
                                                                     m_cFields,
                                                                     lContext->GetSiteList(),
                                                                     bLaunchReportViewer) == false)
                {
                    strError    = GS::Gex::PATEngine::GetInstance().GetErrorMessage();
                    iStatus     = ReportError;
                    break;
                }
            }

            // If output report file is not STDF, (convert to other format if needed),
            // then erase the STDF temporary file created while computing PAT results
            switch(m_cFields.mOutputDatalogFormat)
            {
            case GEX_TPAT_DLG_OUTPUT_NONE:
                // Delete intermadiate STDF file.
                QFile::remove(lContext->GetOutputDataFilename());
                break;

            case GEX_TPAT_DLG_OUTPUT_STDF:
                // Keep STDF file created
                break;

            case GEX_TPAT_DLG_OUTPUT_ATDF:
                // Convert STDF to ATDF
                CSTDFtoATDF lATDFConverter(false);
                QFileInfo   lSTDFFileInfo(lContext->GetOutputDataFilename());
                QString     lATDFName   = lSTDFFileInfo.path() + "/" +
                        lSTDFFileInfo.completeBaseName() + ".atd";

                lATDFConverter.SetProcessRecord(true);          // Convert ALL STDF records to ATDF
                lATDFConverter.SetWriteHeaderFooter(false);     // Disable DUMP comments in ATDF file.

                if (lATDFConverter.Convert(lContext->GetOutputDataFilename(), lATDFName) == false)
                {
                    lATDFConverter.GetLastError(strError);
                    GSLOG(SYSLOG_SEV_WARNING, "Failed to convert STDF output to ATDF. ");
                    iStatus = WriteError;
                }
                else
                    // Delete intermadiate STDF file.
                    QFile::remove(lContext->GetOutputDataFilename());

                break;
            }

            if (iStatus != NoError)
                break;

            // Update HTML TOC page (table of countents) if relevant
            if(lGroupCount > 1)
            {
                // Get handle to list of PAT recipe list
                //                ptPatDefList = *lContext->mPATDefinitions.begin();

                /////////////////////////////////////////////////////////////////////
                // Keep track of top-level DPAT summary stats
                /////////////////////////////////////////////////////////////////////
                for(itPATDefinifion = lContext->GetUnivariateRules().begin();
                    itPATDefinifion != lContext->GetUnivariateRules().end(); ++itPATDefinifion)
                {
                    ptPatDef = *itPATDefinifion;

                    // Only include tests that have failures
                    bIgnoreTest = false;
                    if(ptPatDef == NULL)
                        bIgnoreTest = true;
                    if(!ptPatDef->m_TotalFailures)
                        bIgnoreTest = true;
                    if(bIgnoreTest == false)
                    {
                        // Check if this test is already in list: if so cummulate the fail count.
                        bool bCreateEntry = true;
                        foreach(ptFailTest, cDPatResults)
                        {
                            if((ptPatDef->m_strTestName == ptFailTest->m_strTestName) &&
                                    (ptPatDef->m_lTestNumber == ptFailTest->m_lTestNumber) &&
                                    (ptPatDef->mPinIndex == ptFailTest->mPinIndex))
                            {
                                ptFailTest->m_TotalFailures += ptPatDef->m_TotalFailures;
                                //ptFailTest->m_lSeverityScore += ptPatDef->m_lSeverityScore;	// Will sort by severity score
                                ptFailTest->m_lSeverityScore += ptPatDef->m_TotalFailures;		// Will sort by fail count
                                bCreateEntry = false;
                                break;
                            }
                        }

                        if(bCreateEntry)
                        {
                            // Entry not in list, add it!
                            ptFailTest = new CPatDefinition;
                            ptFailTest->m_strTestName = lContext->GetTestName(ptPatDef->m_lTestNumber,
                                                                              ptPatDef->mPinIndex);
                            ptFailTest->m_lTestNumber = ptPatDef->m_lTestNumber;
                            ptFailTest->mPinIndex = ptPatDef->mPinIndex;
                            ptFailTest->m_TotalFailures = ptPatDef->m_TotalFailures;
                            //ptFailTest->m_lSeverityScore = ptPatDef->m_lSeverityScore;	// Will sort by severity score
                            ptFailTest->m_lSeverityScore = ptPatDef->m_TotalFailures;		// Will sort by fail count
                            cDPatResults.append(ptFailTest);
                        }
                    }
                }

                /////////////////////////////////////////////////////////////////////
                // Keep track of top-level NNR summary stats
                /////////////////////////////////////////////////////////////////////
                foreach(ptNRR_OutlierPart, lContext->pNNR_OutlierTests)
                {
                    bool bCreateEntry = true;

                    tdPatResultListIterator itNnrResultsList(cNnrResults);
                    while(itNnrResultsList.hasNext())
                    {
                        ptFailTest = itNnrResultsList.next();

                        if((ptNRR_OutlierPart->mTestNumber == ptFailTest->m_lTestNumber) &&
                                (ptNRR_OutlierPart->mPinmap == ptFailTest->mPinIndex))
                        {
                            ptFailTest->m_TotalFailures++;
                            ptFailTest->m_lSeverityScore++;
                            bCreateEntry = false;
                            break;
                        }
                    }

                    if(bCreateEntry)
                    {
                        // Entry not in list, add it!
                        ptFailTest = new CPatDefinition;
                        ptFailTest->m_strTestName   = ptNRR_OutlierPart->mTestName;
                        ptFailTest->m_lTestNumber   = ptNRR_OutlierPart->mTestNumber;
                        ptFailTest->mPinIndex       = ptNRR_OutlierPart->mPinmap;
                        ptFailTest->m_TotalFailures = 1;
                        ptFailTest->m_lSeverityScore = 1;
                        cNnrResults.append(ptFailTest);
                    }
                }

                /////////////////////////////////////////////////////////////////////
                // Keep track of top-level IDDQ-Delta summary stats
                ///////////////////////////////////////////////////////////////////
                foreach(ptIDDQ_Delta_OutlierPart, lContext->pIDDQ_Delta_OutlierTests)
                {
                    bool bCreateEntry = true;
                    foreach(ptFailTest, cIddqResults)
                    {
                        if((ptIDDQ_Delta_OutlierPart->lTestNumber1 == ptFailTest->m_lTestNumber) &&
                                (ptIDDQ_Delta_OutlierPart->lPinmapIndex1 == ptFailTest->mPinIndex))
                        {
                            ptFailTest->m_TotalFailures++;
                            ptFailTest->m_lSeverityScore++;
                            bCreateEntry = false;
                            break;
                        }
                    }

                    if(bCreateEntry)
                    {
                        // Entry not in list, add it!
                        ptFailTest = new CPatDefinition;
                        ptFailTest->m_strTestName = lContext->GetTestName(ptIDDQ_Delta_OutlierPart->lTestNumber1,
                                                                          ptIDDQ_Delta_OutlierPart->lPinmapIndex1);
                        ptFailTest->m_lTestNumber = ptIDDQ_Delta_OutlierPart->lTestNumber1;
                        ptFailTest->mPinIndex = ptIDDQ_Delta_OutlierPart->lPinmapIndex1;
                        ptFailTest->m_TotalFailures = 1;
                        ptFailTest->m_lSeverityScore = 1;
                        cIddqResults.append(ptFailTest);
                    }
                }

                // If first
                if(hFile == NULL)
                {
                    // Get home folder to first STDF file to process...or output folder (if custom folder path defined)
                    QString strOutputLocation=CGexReport::outputLocation(&ReportOptions);
                    if((strOutputLocation.isEmpty() == false) && (strOutputLocation != "default"))
                    {
                        // Report created in custom folder
                        strTocPage = strOutputLocation;	//ReportOptions.strOutputPath;
                    }
                    else
                    {
                        // Report created in same folder as first data file
                        if(strInputFilesList.count() > 0)
                            cFileInfo.setFile(strInputFilesList.first());
                        else
                            cFileInfo.setFile(strInputMapFiles.first());
                        strTocPage = cFileInfo.absolutePath();
                    }

                    if(strTocPage.endsWith("/") == false)
                        strTocPage += "/";
                    strTocPage += "index.htm";
                    hFile = fopen(strTocPage.toLatin1().constData(),"w");
                    if(hFile == NULL)
                    {
                        GS::Gex::Message::critical(
                                    "", "Failed creating Lot Summary HTML page.");
                    }
                    else
                    {
                        fprintf(hFile,"<html>\n");
                        fprintf(hFile,"<head>\n</head>\n");
                        fprintf(hFile,"<body>\n");
                        fprintf(hFile,"<h1 align=\"left\"><font color=\"#006699\">Batch processing summary:</font></h1><br>\n");
                        fprintf(hFile,"<table width=\"90%%\">\n");
                        fprintf(hFile,"<tr>\n");
                        fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#CCECFF\" align=\"left\"><b>Testing date</b></td>\n");
                        fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#CCECFF\" align=\"left\"><b>Product</b></td>\n");
                        fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#CCECFF\" align=\"left\"><b>Lot ID</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"left\"><b>SubLot</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Wafer ID</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Total parts</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Good/<br>Yield</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>DPAT Loss</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>MVPAT Loss</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>NNR Loss</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>IDDQ-D Loss</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>GDBN Loss</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Clustering Loss</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Reticle Loss</b></td>\n");
                        fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Z-PAT Loss</b></td>\n");
                        fprintf(hFile,"<td width=\"8%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Wafer</b></td>\n");
                        fprintf(hFile,"<td width=\"8%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>PAT Report</b></td>\n");
                        // case 5909
                        //fprintf(hFile,"<td width=\"9%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Reload Dataset</b></td>\n");
                        fprintf(hFile,"</tr>\n");
                    }
                }
                // Write file entry details in page
                CGexGroupOfFiles *pGroup=gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
                CGexFileInGroup *pFile=(pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

                // Get product name
                strProduct = pFile->getMirDatas().szPartType;

                // Check total DPAT and geographic failures (GDBN + Reticle + Clustering + NRR...)
                //FIXME: not used ?
                //iDpatFailures = lContext->m_lstOutlierParts.count();
                iGeographicFailures = lContext->mGDBNOutliers.count() + lContext->mReticleOutliers.count();
                iGeographicFailures += lContext->mClusteringOutliers.count() + lContext->mNNROutliers.count();
                iGeographicFailures += lContext->mIDDQOutliers.count();

                // Compute wafer yield level...
                lTotalBins = 0;
                lTotalGoodBins = 0;


                // Ensure we have the full Hard-Bin wafermap (all sites merged)...without PAT dies.
                pFile->getWaferMapData() = lContext->m_AllSitesMap_Hbin;

                for(i=0;i<pFile->getWaferMapData().SizeX*pFile->getWaferMapData().SizeY;i++)
                {
                    // Get die.
                    iBin = pFile->getWaferMapData().getWafMap()[i].getBin();
                    if(iBin >= 0)
                    {
                        lTotalBins++;
                        // Check if this bin belongs to our list of 'good bins'
                        if(lContext->GetRecipeOptions().pGoodHardBinsList->Contains(iBin))
                            lTotalGoodBins++;	// Counts total Good Bins.
                    }
                }

                // Compute Percentage for all groups merged (all testing sites)
                if(lTotalBins)
                {
                    // fYield = 100.0 * ((double) lTotalGoodBins-iGeographicFailures-iDpatFailures) / (double) lTotalBins;
                    fOriginalYield = 100.0 * ((double) lTotalGoodBins) / (double) lTotalBins;
                    iOriginalYieldCount += lTotalGoodBins;
                }
                else
                    fOriginalYield = 0;

                // Get Lot#
                strLot = pFile->getMirDatas().szLot;

                // Get WaferID info
                strSubLot = pFile->getMirDatas().szSubLot;
                if(*pFile->getWaferMapData().szWaferID)
                    strSubLot = pFile->getWaferMapData().szWaferID;

                // Get folder name where report was created for this file...
                cFileInfo.setFile(gexReport->reportAbsFilePath());
                strString = cFileInfo.absolutePath();
                GSLOG(7, QString("File dir path = %s").arg(strString).toLatin1().data() );
                strString = strString.mid(1 + gex_max(strString.lastIndexOf('/'),strString.lastIndexOf('\\')));

                fprintf(hFile,"<tr>\n");
                fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"left\">%s</td>\n",TimeStringUTC(pFile->getMirDatas().lStartT));
                fprintf(hFile,
                        "<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"left\">%s</td>\n",pFile->getMirDatas().szPartType); // Product
                fprintf(hFile,
                        "<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"left\">%s</td>\n",pFile->getMirDatas().szLot);
                fprintf(hFile,
                        "<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%s</td>\n",pFile->getMirDatas().szSubLot);
                // case 5909 : enhanced link
                fprintf(hFile,
                        "<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\"><a href=\"%s/index.htm%s%s#%d\">%s</a></td>\n",
                        strString.toLatin1().constData(),
                        GEX_BROWSER_ACTIONBOOKMARK, GEXTB_BROWSER_RELOADDATA_PAT, iGroupID+1,
                        pFile->getWaferMapData().szWaferID);

                // Total parts
                fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d</td>\n",lTotalBins);
                iTotalParts += lTotalBins;
                // Yield: Total GOOD parts
                fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%d<br>(%.1f%%)</td>\n",lTotalGoodBins,fOriginalYield);
                fTotalYield += fOriginalYield;

                // DPAT yield loss
                fPatYieldLoss = 100.0*(lContext->m_lstOutlierParts.count()) / lTotalBins;
                if(fPatYieldLoss > 0)
                {
                    // 5909 : let s force PAT reload on every links
                    strHyperlinkBegionSection.sprintf(
                                "<a href=\"%s/pages/advanced_summary.htm#all_tests_dyn_fails#_gex_tb_patreload.htm#%d\">",
                                strString.toLatin1().constData(), iGroupID+1);
                    strHyperlinkEndSection ="</a>";
                }
                else
                    strHyperlinkBegionSection = strHyperlinkEndSection = "";	// Yield = 0: do not build hyperlinks!
                fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%s %d<br>(%.1f%%)%s</td>\n",
                        strHyperlinkBegionSection.toLatin1().constData(),
                        lContext->m_lstOutlierParts.count(),
                        fPatYieldLoss,
                        strHyperlinkEndSection.toLatin1().constData());
                fTotalDPat_YieldLoss += fPatYieldLoss;
                iTotalDPat_YieldLossCount += lContext->m_lstOutlierParts.count();

                // MVPAT yield loss
                fPatYieldLoss = 100.0*(lContext->GetMVOutliers().count()) / lTotalBins;
                if(fPatYieldLoss > 0)
                {
                    strHyperlinkBegionSection.sprintf(
                                "<a href=\"%s/pages/advanced_mvpat_summary.htm#mvpat_summary\">",strString.toLatin1().constData());
                    strHyperlinkEndSection ="</a>";
                }
                else
                    strHyperlinkBegionSection = strHyperlinkEndSection = "";	// Yield = 0: do not build hyperlinks!

                fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%s %d<br>(%.1f%%)%s</td>\n",
                        strHyperlinkBegionSection.toLatin1().constData(),
                        lContext->GetMVOutliers().count(),
                        fPatYieldLoss,
                        strHyperlinkEndSection.toLatin1().constData());
                fTotalMVPAT_YieldLoss += fPatYieldLoss;
                iTotalMVPAT_YieldLossCount += lContext->GetMVOutliers().count();

                // NNR yield loss
                fPatYieldLoss = 100.0*(lContext->mNNROutliers.count()) / lTotalBins;
                if(fPatYieldLoss > 0)
                {
                    strHyperlinkBegionSection.sprintf(
                                "<a href=\"%s/pages/advanced_nnr_summary.htm#nnr_summary\">",strString.toLatin1().constData());
                    strHyperlinkEndSection ="</a>";
                }
                else
                    strHyperlinkBegionSection = strHyperlinkEndSection = "";	// Yield = 0: do not build hyperlinks!
                fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%s %d<br>(%.1f%%)%s</td>\n",
                        strHyperlinkBegionSection.toLatin1().constData(),
                        lContext->mNNROutliers.count(),
                        fPatYieldLoss,
                        strHyperlinkEndSection.toLatin1().constData());
                fTotalNNR_YieldLoss += fPatYieldLoss;
                iTotalNNR_YieldLossCount += lContext->mNNROutliers.count();

                // IDDQ-Delta yield loss
                fPatYieldLoss = 100.0*(lContext->mIDDQOutliers.count()) / lTotalBins;
                if(fPatYieldLoss > 0)
                {
                    strHyperlinkBegionSection.sprintf(
                                "<a href=\"%s/pages/advanced_iddq_delta_summary.htm#iddq_delta_summary\">",
                                strString.toLatin1().constData());
                    strHyperlinkEndSection ="</a>";
                }
                else
                    strHyperlinkBegionSection = strHyperlinkEndSection = "";	// Yield = 0: do not build hyperlinks!
                fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%s %d<br>(%.1f%%)%s</td>\n",
                        strHyperlinkBegionSection.toLatin1().constData(),
                        lContext->mIDDQOutliers.count(),
                        fPatYieldLoss,strHyperlinkEndSection.toLatin1().constData());
                fTotalIDDQ_Delta_YieldLoss += fPatYieldLoss;
                iTotalIDDQ_Delta_YieldLossCount += lContext->mIDDQOutliers.count();

                // GDBN yield loss
                fPatYieldLoss = 100.0*(lContext->mGDBNOutliers.count()) / lTotalBins;
                if(fPatYieldLoss > 0)
                {
                    strHyperlinkBegionSection.sprintf("<a href=\"%s/pages/wafermap.htm#_gex_tb_patreload.htm#%d\">",
                                                      strString.toLatin1().constData(), iGroupID+1);
                    strHyperlinkEndSection ="</a>";
                }
                else
                    strHyperlinkBegionSection = strHyperlinkEndSection = "";	// Yield = 0: do not build hyperlinks!
                fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%s %d<br>(%.1f%%)%s</td>\n",
                        strHyperlinkBegionSection.toLatin1().constData(),
                        lContext->mGDBNOutliers.count(),
                        fPatYieldLoss,strHyperlinkEndSection.toLatin1().constData());
                fTotalGDBN_YieldLoss += fPatYieldLoss;
                iTotalGDBN_YieldLossCount += lContext->mGDBNOutliers.count();

                // Clustering yield loss
                fPatYieldLoss = 100.0*(lContext->mClusteringOutliers.count()) / lTotalBins;
                if(fPatYieldLoss > 0)
                {
                    strHyperlinkBegionSection.sprintf("<a href=\"%s/pages/wafermap.htm#_gex_tb_patreload.htm#%d\">",
                                                      strString.toLatin1().constData(), iGroupID+1);
                    strHyperlinkEndSection ="</a>";
                }
                else
                    strHyperlinkBegionSection = strHyperlinkEndSection = "";	// Yield = 0: do not build hyperlinks!
                fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%s %d<br>(%.1f%%)%s</td>\n",
                        strHyperlinkBegionSection.toLatin1().constData(),
                        lContext->mClusteringOutliers.count(),
                        fPatYieldLoss,strHyperlinkEndSection.toLatin1().constData());
                fTotalCluster_YieldLoss += fPatYieldLoss;
                iTotalCluster_YieldLossCount += lContext->mClusteringOutliers.count();

                // Reticle yield loss
                fPatYieldLoss = 100.0*(lContext->mReticleOutliers.count()) / lTotalBins;
                if(fPatYieldLoss > 0)
                {
                    strHyperlinkBegionSection.sprintf("<a href=\"%s/pages/advanced_reticle.htm#reticle_summary\">",
                                                      strString.toLatin1().constData());
                    strHyperlinkEndSection ="</a>";
                }
                else
                    strHyperlinkBegionSection = strHyperlinkEndSection = "";	// Yield = 0: do not build hyperlinks!
                fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%s %d<br>(%.1f%%)%s</td>\n",
                        strHyperlinkBegionSection.toLatin1().constData(),
                        lContext->mReticleOutliers.count(),
                        fPatYieldLoss,strHyperlinkEndSection.toLatin1().constData());
                fTotalReticle_YieldLoss += fPatYieldLoss;
                iTotalReticle_YieldLossCount += lContext->mReticleOutliers.count();

                // Z-PAT Loss (if Z-PAT map specified)
                fPatYieldLoss = 100.0*(lContext->mZPATOutliers.count()) / lTotalBins;
                if(fPatYieldLoss > 0)
                {
                    strHyperlinkBegionSection.sprintf("<a href=\"%s/pages/wafermap.htm\">",
                                                      strString.toLatin1().constData());
                    strHyperlinkEndSection ="</a>";
                }
                else
                    strHyperlinkBegionSection = strHyperlinkEndSection = "";	// Yield = 0: do not build hyperlinks!
                fprintf(hFile,"<td width=\"5%%\" bgcolor=\"#F8F8F8\" align=\"center\">%s %d<br>(%.1f%%)%s</td>\n",
                        strHyperlinkBegionSection.toLatin1().constData(),
                        lContext->mZPATOutliers.count(),
                        fPatYieldLoss,strHyperlinkEndSection.toLatin1().constData());
                fTotalZPAT_YieldLoss += fPatYieldLoss;
                iTotalZPAT_YieldLossCount += lContext->mZPATOutliers.count();

                if (m_iReloadDataFileID!=iGroupID+1)
                    fprintf(hFile,
                            "<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"center\"><a href=\"%s/pages/wafermap.htm%s%s#%d\"><b>Map</b></a></td>\n",
                            strString.toLatin1().constData(),
                            GEX_BROWSER_ACTIONBOOKMARK, GEXTB_BROWSER_RELOADDATA_PAT, iGroupID+1
                            );
                else
                    fprintf(hFile,
                            "<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"center\"><a href=\"%s/pages/wafermap.htm\"><b>Map</b></a></td>\n",
                            strString.toLatin1().constData()  );

                fprintf(hFile,
                        "<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"center\"><a href=\"%s/pages/advanced.htm%s%s#%d\"><b>PAT</b></a></td>\n",
                        strString.toLatin1().constData(),
                        GEX_BROWSER_ACTIONBOOKMARK, GEXTB_BROWSER_RELOADDATA_PAT, iGroupID+1
                        );
                /* case 5909
                fprintf(hFile,
                        "<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"center\"><a href=\"%s%s#%d\"><b>Sync.</b></a></td>\n",
                        GEX_BROWSER_ACTIONBOOKMARK,
                        GEXTB_BROWSER_RELOADDATA_PAT,
                        iGroupID+1);
                */
                fprintf(hFile,"</tr>\n");

                // Update yield/Wafer count
                iTotalWafersInLot++;
            }
        }
        // Total Yield
        fOriginalYield = fTotalYield / iTotalWafersInLot;

        // Average yield losses
        fTotalDPat_YieldLoss /= iTotalWafersInLot;
        fTotalNNR_YieldLoss /= iTotalWafersInLot;
        fTotalIDDQ_Delta_YieldLoss /= iTotalWafersInLot;
        fTotalGDBN_YieldLoss /= iTotalWafersInLot;
        fTotalReticle_YieldLoss /= iTotalWafersInLot;
        fTotalZPAT_YieldLoss /= iTotalWafersInLot;
        fTotalCluster_YieldLoss /= iTotalWafersInLot;
        fTotalMVPAT_YieldLoss /= iTotalWafersInLot;

        // Write HTML line
        WriteHtmlPatSummaryLine(hFile,fOriginalYield,fTotalDPat_YieldLoss,fTotalMVPAT_YieldLoss,fTotalNNR_YieldLoss,
                                fTotalIDDQ_Delta_YieldLoss,fTotalGDBN_YieldLoss,fTotalCluster_YieldLoss,fTotalReticle_YieldLoss,
                                fTotalZPAT_YieldLoss,iTotalParts, iOriginalYieldCount,iTotalDPat_YieldLossCount,iTotalMVPAT_YieldLossCount,
                                iTotalNNR_YieldLossCount,iTotalIDDQ_Delta_YieldLossCount,iTotalGDBN_YieldLossCount,
                                iTotalCluster_YieldLossCount,iTotalReticle_YieldLossCount,iTotalZPAT_YieldLossCount);
    }

    // Close HTML TOC page if created
    if(hFile != NULL)
    {
        fprintf(hFile,"</table>\n");
        /* case 5909
        fprintf(hFile,
          "<br><br><b>Note</b>: The <b>'Sync'</b> link allows reloading the relevant dataset and PAT markers in memory "\
                "(ensures Interactive Charts display the correct data file).<br>\n");
        fprintf(hFile,"You don't need to click the 'Sync' link as long as you only navigate into the HTML report pages.\n");
        */

        // Write Pareto of test failures
        fprintf(hFile,"<h1 align=\"left\"><font color=\"#006699\">DPAT pareto (summary):</font></h1>\n");
        if(cDPatResults.count() > 0)
        {
            fprintf(hFile,"<table width=\"70%%\">\n");
            fprintf(hFile,"<tr>\n");
            fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#CCECFF\" align=\"left\"><b>Test#</b></td>\n");
            fprintf(hFile,"<td width=\"40%%\" bgcolor=\"#CCECFF\" align=\"left\"><b>Name</b></td>\n");
            fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Severity</b></td>\n");
            fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Fail count</b></td>\n");
            fprintf(hFile,"</tr>\n");

            // Sort by Severity score
            qSort(cDPatResults.begin(), cDPatResults.end(), ComparePatResultNum);
            foreach(ptFailTest, cDPatResults)
            {
                fprintf(hFile,"<tr>\n");
                fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"left\">%ld</td>\n",ptFailTest->m_lTestNumber);
                fprintf(hFile,"<td width=\"40%%\" bgcolor=\"#F8F8F8\" align=\"left\">%s</td>\n",ptFailTest->m_strTestName.toLatin1().constData());
                fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"center\">%ld</td>\n",ptFailTest->m_lSeverityScore);
                fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"center\">%ld</td>\n",ptFailTest->m_TotalFailures);
                fprintf(hFile,"</tr>\n");

            }
            fprintf(hFile,"</table>\n<br>\n");
        }
        else
            fprintf(hFile,"<p>No DPAT outlier!</p>\n");

        // Write Pareto of test failures
        fprintf(hFile,"<h1 align=\"left\"><font color=\"#006699\">NNR pareto (summary):</font></h1>\n");
        if(cNnrResults.count() > 0)
        {
            fprintf(hFile,"<table width=\"70%%\">\n");
            fprintf(hFile,"<tr>\n");
            fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#CCECFF\" align=\"left\"><b>Test#</b></td>\n");
            fprintf(hFile,"<td width=\"40%%\" bgcolor=\"#CCECFF\" align=\"left\"><b>Name</b></td>\n");
            fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#CCECFF\" align=\"center\"><b>Fail count</b></td>\n");
            fprintf(hFile,"</tr>\n");

            // Sort by Severity score
            qSort(cNnrResults.begin(), cNnrResults.end(), sortStaticPatDefinition);

            tdPatResultListIterator itNnrResultsList(cNnrResults);
            while(itNnrResultsList.hasNext())
            {
                ptFailTest = itNnrResultsList.next();

                fprintf(hFile,"<tr>\n");
                fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"left\">%ld</td>\n",ptFailTest->m_lTestNumber);
                fprintf(hFile,"<td width=\"40%%\" bgcolor=\"#F8F8F8\" align=\"left\">%s</td>\n",ptFailTest->m_strTestName.toLatin1().constData());
                fprintf(hFile,"<td width=\"10%%\" bgcolor=\"#F8F8F8\" align=\"center\">%ld</td>\n",ptFailTest->m_TotalFailures);
                fprintf(hFile,"</tr>\n");
            }
            fprintf(hFile,"</table>\n");
        }
        else
            fprintf(hFile,"<p>No NNR outlier!</p>\n");

        // Write Batch processing time
        int	iElapsed = 	cTime.elapsed() / 1000;	// Elapsed time in secs.
        int	iHours,iMinutes,iSec;
        iHours = iElapsed / 3600;
        iMinutes = (iElapsed % 3600) / 60;
        iSec = iElapsed % 60;
        strString = "";
        // Check if X Hours
        if(iHours)
            strString += QString::number(iHours) + " hour";
        if(iHours > 1)
            strString += "s";	// 'Hours'!
        strString += " ";

        // Check if X minutes
        if(iMinutes)
            strString += QString::number(iMinutes) + " minute";
        if(iMinutes > 1)
            strString += "s";	// 'Minutes'!


        // Check if X minutes
        if(iSec)
            strString += " " + QString::number(iSec) + " sec.";

        fprintf(hFile,"<h1 align=\"left\"><font color=\"#006699\">Batch processing time: %s</font></h1><br><br>\n",strString.toLatin1().constData());


        fprintf(hFile,"</body>\n");
        fprintf(hFile,"</html>\n");
        fclose(hFile);

        // Force tab to be on 'Report'
        pGexMainWindow->ReportLink();

        // Force to load TOC page instead of standard report index page.
        gexReport->setLegacyReportName(strTocPage);
        pGexMainWindow->LoadUrl(strTocPage);
    }

    // Check Status and display message accordingly.
    if(iStatus == NoError)
    {
        if(strInputFilesList.count() == 1)
        {
            strError = "Outlier processing successful!\nFile(s) created in folder" + m_cFields.mOutputFolderSTDF;
            // Only display successful message if the report was not created. When report is created, it is displayed, so no need for another popup!
            if(m_cFields.strOutputReportFormat == "interactive")
                GS::Gex::Message::information("", strError);
        }
    }
    else
    {
        if(iStatus == ScriptError && strError.isEmpty())
        {
            GS::Gex::Message::critical("", "Failed to start file analysis");
        }
        else
        {
            GS::Gex::Message::critical("", strError);
        }
    }

    // Re-enable GUI buttons
    enableGuiField(true);

    // Clean up objects
    qDeleteAll(cNnrResults);
    cNnrResults.clear();
    qDeleteAll(cDPatResults);
    cDPatResults.clear();
    qDeleteAll(cIddqResults);
    cIddqResults.clear();
}

///////////////////////////////////////////////////////////
// Create intermediate input STDF file if needed
// Note: Not available if running at Tester
///////////////////////////////////////////////////////////
//int	GexTbPatDialog::ConvertToSTDF(QString &strInputFile, QString &strErrorMessage)
//{
//    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
//        return NoError;

//    // Build output name and extension
//    CGConvertToSTDF StdfConvert;
//    QString         lSTDFFilename = strInputFile + "_galaxy.std";
//    bool            bFileCreated  = false;

//    int nConvertStatus = StdfConvert.Convert(false, true, false, true, strInputFile, lSTDFFilename, "", bFileCreated,strErrorMessage);
//    if(nConvertStatus == CGConvertToSTDF::eConvertError)
//    {
//        // Ensure to delete intermediate STDF file created.
//        GS::Gex::Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);

//        strErrorMessage = "*Error* Failed reading or parsing data file (Invalid path, or file corrupted). File :\n" + strInputFile;
//        return ReadError;
//    }

//    // Check if Input file was already in STDF format!
//    if(bFileCreated == false)
//        lContext->SetSTDFFilename(strInputFile);
//    else
//        lContext->SetSTDFFilename(lSTDFFilename);

//    return NoError;
//}

//bool GexTbPatDialog::AnalyzeFile_AllBins(QString &strFileToProcess, QString &strErrorMessage)
//{
//    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
//        return true;

//    GSLOG(6, "AnalyzeFile_AllBins...");
//    // convert to STDF if needed
//    if(ConvertToSTDF(strFileToProcess, strErrorMessage) != NoError)
//        return false;


//    // Have the STDF file analysed so we can create a summary version!
//    QString strScriptFile;

//    // Create script that will read data file + compute all statistics (but NO report created)
//    strScriptFile = GS::Gex::Engine::GetInstance().GetAssistantScript();
//    FILE *hFile = fopen(strScriptFile.toLatin1().constData(),"w");

//    if(hFile == NULL)
//    {
//        strErrorMessage = "  > Failed to create script file: " + strScriptFile;
//        return false;
//    }

//    // Creates 'SetOptions' section
//    if(!ReportOptions.WriteOptionSectionToFile(hFile))
//    {
//        //GEX_ASSERT(false);
//        strErrorMessage = QString("Error : can't write option section");
//        return false;
//    }

//    // Creates 'SetProcessData' section
//    fprintf(hFile,"SetProcessData()\n");
//    fprintf(hFile,"{\n");
//    fprintf(hFile,"  var group_id;\n");
//    fprintf(hFile,"  gexGroup('reset','all');\n");

//    // WCreate group
//    // Make sure any '\' in string is doubled.
//    QString lSTDFScriptFile = ConvertToScriptString(lContext->GetSTDFFilename());

//    fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");
//    fprintf(hFile,
//            "  gexFile(group_id,'insert','%s','All','last_instance',' ','','');\n\n",
//            lSTDFScriptFile.toLatin1().constData());

//    fprintf(hFile,"}\n\n");

//    // Creates 'main' section
//    fprintf(hFile,"main()\n");
//    fprintf(hFile,"{\n");
//    fprintf(hFile,"  SetOptions();\n");
//    // Avoids editor to be launched if output options is set to Word or Excel!
//    fprintf(hFile,"  gexOptions('output','format','html');\n");
//    // Merge tests with same test number (even if test number is different)
//    fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
//    // Always compute Quartiles & Skew & Kurtosis.
//    fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");
//    // Force to compute statistics from samples, ignore summary.
//    fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");
//    // Disable outlier removal so to make sure all values are taken into account.
//    fprintf(hFile,"  gexOptions('dataprocessing', 'data_cleaning_mode','none');\n");
//    // Ensure FULL wafermap is created in any case, non matter the filtering of parts (ensures good processing over Bad Neigbhoors)
//    fprintf(hFile,"  gexOptions('wafer','visual_options','all_parts');\n");
//    /* BG 1 June 2011: do not force pos_x and pos_y
//      // Use 'auto' mode for positive X direction
//    fprintf(hFile,"  gexOptions('wafer','positive_x','auto');\n");
//    // Use 'auto' mode for positive Y direction
//    fprintf(hFile,"  gexOptions('wafer','positive_y','auto');\n");
//    */
//    fprintf(hFile,"  gexOptions('binning','computation','wafer_map');\n");
//    fprintf(hFile,"  gexReportType('wafer','soft_bin');\n");

//    fprintf(hFile,"  SetProcessData();\n");
//    // Only data analysis, no report created!
//    fprintf(hFile,"  gexOptions('report','build','false');\n");
//    // Show 'Database Admin' page after processing file.
//    fprintf(hFile,"  gexBuildReport('admin','0');\n");
//    fprintf(hFile,"}\n\n");
//    fclose(hFile);

//    // Execute script.
//    if (GS::Gex::CSLEngine::GetInstance().RunScript(strScriptFile).IsFailed())
//    {
//        strErrorMessage = "  > Failed to analyze bins from stdf file: " +
//                          lContext->GetSTDFFilename();
//        return false;
//    }

//    // Success
//    return true;
//}

#endif

// function to guess severity combobox (from 0 to 3) from old index (from 0 to 8)
int GetSeverityIndexFromOldIndex(int old_index)
{
    if (old_index==0 ) // stop prod
        return 0; // critical
    if (old_index==1) // Continue production
        return 1; // warning
    return 2; // notice
}

///////////////////////////////////////////////////////////
// Extracts Test#, and Pin# from a given string.
///////////////////////////////////////////////////////////
int	ExtractTestRange(const char *szRange,unsigned long *lFromTest,long *lFromPinmapIndex,unsigned long *lToTest,long *lToPinmapIndex)
{
    char	szValue[100];
    char	szFrom[50];
    char	szTo[50];
    char	*ptChar;
    int		iCount;
    int		iParameters=0;
    unsigned long	iSwapTest;
    long	iSwapPinmap;

    // Check validity of the string to parse!
    if(szRange == NULL)
        return 0;

    // Ensure string is in Lower case
    strcpy(szValue,szRange);
    ptChar = szValue;
    while (*ptChar)
    {
        *ptChar = tolower(*ptChar);
        ptChar++;
    };

    // Format the filter string so we can extract the low/high bin values.
    ptChar=strchr(szValue,'-');
    if(ptChar != NULL)
        *ptChar = ' ';
    ptChar=strstr(szValue,"to");
    if(ptChar != NULL)
    {
        *ptChar++ = ' ';
        *ptChar++ = ' ';
    }

    iCount = sscanf(szValue,"%s %s",szFrom,szTo);
    switch(iCount)
    {
    case 0:	// empty string, or incorrect...then force default values.
        *lFromTest=0;
        *lFromPinmapIndex=GEX_PTEST; // GEX_MINPINMAP;
        return iParameters;	// 0 parameter...
    case 1:	// Only one value given...then force LowTest.PinmapIndex = HighTest.PinmapIndex
        strcpy(szTo,szFrom);
        break;
    }

    // Parse 'From' test.PinmapIndex entry
    iCount = sscanf(szFrom,"%lu%*c%ld",lFromTest,lFromPinmapIndex);
    switch(iCount)
    {
    case 0:	// empty string, or incorrect...
        *lFromTest=0;
        *lFromPinmapIndex=GEX_PTEST; // GEX_MINPINMAP;
        iParameters=0;
        break;
    case 1:	// Only test#, no PinmapIndex
        *lFromPinmapIndex=GEX_PTEST; // GEX_MINPINMAP;
        iParameters=1;
        break;
    case 2: // Text#.pinmap given
        iParameters=1;
        break;
    }

    if(lToTest==NULL || lToPinmapIndex==NULL)
        return iParameters;	// Only 'From' value is of interest.

    iCount = sscanf(szTo,"%lu%*c%ld",lToTest,lToPinmapIndex);
    switch(iCount)
    {
    case 0:	// empty string, or incorrect...then force '0 to 10000'
        *lToTest=0;
        *lToPinmapIndex=GEX_PTEST; // GEX_MAXPINMAP;
        break;;
    case 1:	// Only test#, no PinmapIndex
        *lToPinmapIndex=GEX_PTEST; // GEX_MAXPINMAP;
        iParameters++;
        break;
    case 2: // Text#.pinmap given
        iParameters++;
        break;
    }

    // Swap values In case To > From !
    if(*lToTest < *lFromTest)
    {
        iSwapTest = *lFromTest;
        *lFromTest = *lToTest;
        *lToTest = iSwapTest;
    }

    // Swap PinmapIndex # if same test # but PinmapIndex# not in correct order
    if((*lToTest == *lFromTest) && (*lToPinmapIndex < *lFromPinmapIndex))
    {
        iSwapPinmap = *lFromPinmapIndex;
        *lFromPinmapIndex = *lToPinmapIndex;
        *lToPinmapIndex = iSwapPinmap;
    }

    // Returns the number of tests read: 0,1, or 2
    return iParameters;
}

///////////////////////////////////////////////////////////
// Extracts "Test#1.Pin#1 Test#2.Pin#2"  from a given string.
///////////////////////////////////////////////////////////
void	ExtractTestCouple(char *szRange,unsigned long *lFromTest,long *lFromPinmapIndex,unsigned long *lToTest,long *lToPinmapIndex)
{
    char	szValue[100];
    char	szFrom[50];
    char	szTo[50];
    char	*ptChar;
    int		iCount;

    // Ensure string is in Lower case
    strcpy(szValue,szRange);
    ptChar = szValue;
    while (*ptChar)
    {
        *ptChar = tolower(*ptChar);
        ptChar++;
    };

    // Format the filter string so we can extract the low/high bin values.
    ptChar=strchr(szValue,'-');
    if(ptChar != NULL)
        *ptChar = ' ';
    ptChar=strstr(szValue,"to");
    if(ptChar != NULL)
    {
        *ptChar++ = ' ';
        *ptChar++ = ' ';
    }

    iCount = sscanf(szValue,"%s %s",szFrom,szTo);
    switch(iCount)
    {
    case 0:	// empty string, or incorrect...then force default values.
        *lFromTest=0;
        *lFromPinmapIndex=GEX_MINPINMAP;
        return;
    case 1:	// Only one value given...then force LowTest.PinmapIndex = HighTest.PinmapIndex
        strcpy(szTo,szFrom);
        break;
    }

    // Parse 'From' test.PinmapIndex entry
    iCount = sscanf(szFrom,"%lu%*c%ld",lFromTest,lFromPinmapIndex);
    switch(iCount)
    {
    case 0:	// empty string, or incorrect...
        *lFromTest=0;
        *lFromPinmapIndex=GEX_PTEST;
        break;;
    case 1:	// Only test#, no PinmapIndex
        *lFromPinmapIndex=GEX_PTEST;
        break;
    }

    if(lToTest==NULL || lToPinmapIndex==NULL)
        return;	// Only 'From' value is of interest.

    iCount = sscanf(szTo,"%lu%*c%ld",lToTest,lToPinmapIndex);
    switch(iCount)
    {
    case 0:	// empty string, or incorrect...then force '0 to 10000'
        *lToTest=0;
        *lToPinmapIndex=GEX_PTEST;
        break;;
    case 1:	// Only test#, no PinmapIndex
        *lToPinmapIndex=GEX_PTEST;
        break;
    }
}

#ifdef GCORE15334


///////////////////////////////////////////////////////////
// Note: Following code DOES NOT apply at Tester

///////////////////////////////////////////////////////////
// Check if this good die should fail because of exessive bad neighbours
///////////////////////////////////////////////////////////
#ifdef OLD_GDBN_ALGORITHM
bool	GexTbPatDialog::CheckDieBadNeighbours(CGDBN_Rule *ptRule,CWaferMap	*ptWafermap, int iCell,CWaferMap::Ring eMaskRegion, int iMaskRadius)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return true;

    if (ptRule->iGDBN_Algorithm == GEX_TPAT_GDBN_ALGO_WEIGHT)
    {
        int nCoordX = GEX_WAFMAP_INVALID_COORD;
        int nCoordY	= GEX_WAFMAP_INVALID_COORD;
        int iBin;

        // Get die coordinates from index
        ptWafermap->coordFromIndex(iCell, nCoordX, nCoordY);

        // This good bin passes the conditions to be failed
        // Check if not already an outlier
        if(isDieOutlier(nCoordX, nCoordY, iBin) == false)
        {
            PatGdbnWeightingAlgo gdbnAlgo;

            gdbnAlgo.setRuleName(ptRule->strRuleName);
            gdbnAlgo.setRingArea(eMaskRegion);
            gdbnAlgo.setRingRadius(iMaskRadius);
            gdbnAlgo.setEdgeDieType(ptRule->iGDBN_EdgeDieType);
            gdbnAlgo.setEdgeDieWeighting(ptRule->iGDBN_EdgeDieWeighting);
            gdbnAlgo.setEdgeDieWeightingScale(ptRule->lfGDBN_EdgeDieWeightingScale);
            gdbnAlgo.setMinimumWeighting(ptRule->iGDBN_MinimumWeighting);
            gdbnAlgo.setAdjacentWeighting(ptRule->lstAdjWeight);
            gdbnAlgo.setDiagonalWeighting(ptRule->lstDiagWeight);
            gdbnAlgo.setBadDies(new PatGdbnGenericBadDies(ptRule->pGDBN_BadBinList));

            if (gdbnAlgo.excludeDie(ptWafermap, iCell))
            {
                QString strKey = QString::number(nCoordX) + "." + QString::number(nCoordY);
                CPatDieCoordinates *ptGoodBin = new CPatDieCoordinates;
                ptGoodBin->iDieX = nCoordX;
                ptGoodBin->iDieY = nCoordY;
                ptGoodBin->iSite = gexReport->getDieTestingSite(-1, 0, nCoordX, nCoordY);
                ptGoodBin->uFailType = GEX_TPAT_BINTYPE_BADNEIGHBORS;
                ptGoodBin->strRuleName = ptRule->strRuleName;

                // Save PAT definition in our list
                lContext->ptPatBadDieNeighbours.insert(strKey, ptGoodBin);	// Holds the Good Die X,Y position to fail

                return true;
            }
        }

        return false;
    }

    // Compute die location
    int nCoordX = GEX_WAFMAP_INVALID_COORD;
    int nCoordY	= GEX_WAFMAP_INVALID_COORD;
    int iX		= iCell % ptWafermap->SizeX;
    int iY		= iCell / ptWafermap->SizeX;

    // Get die coordinates from index
    ptWafermap->coordFromIndex(iCell, nCoordX, nCoordY);

    // If Mask exists, checks if die
    if(iMaskRadius >= 0)
    {
        if(ptWafermap->isDieInsideRing(eMaskRegion,iMaskRadius, nCoordX, nCoordY) == false)
            return false;	// Outside of mask: do not fail die (ignore it)
    }

    int	iClusterSize;
    switch(ptRule->iGDBN_Algorithm)
    {
    default:
    case GEX_TPAT_GDBN_ALGO_SQUEEZE:	// Squeeze algorithm
        iClusterSize = ptRule->iGDBN_ClusterSize;
        break;

    case GEX_TPAT_GDBN_ALGO_WEIGHT:		// WEIGHTING algorithm
        iClusterSize = 3;	// For this algorithm, only work over 3x3 matrix
        break;
    }

    // Compute valid Low & High X offsets in wafermap
    int iLowX = iX - iClusterSize/2;
    int iHighX = iX + iClusterSize/2;

    // Compute valid Low & High Y offsets in wafermap
    int iLowY = iY - iClusterSize/2;
    int iHighY = iY + iClusterSize/2;

    // We only remove good dies if they are sqeezed between bad dies, as such we need to keep track of the bad die location
    // compared to the good die, we then devide the cluster zone in 8 areas, which are relative to the good bien (center):
    // Left, left-top,  letf-bottom, Right, right-top,  right-bottom, center-top,center-bottom
    int	icClusterSubZone[8];
    int	iSubZoneOffset;
    memset(&icClusterSubZone[0],0,8*sizeof(int));

    int		iIndex;
    int		iFailSqueezeCount	= 0;									// Used for Squeeze algorithm
    double	lfFailWeitght		= 0;									// Used for Weighting algorithm
    int		iBin;
    bool	bValidBin;
    bool	bEdgeModeDetected	= false;
    bool	bEdgeDie			= ptWafermap->isDieOnEdge(iCell, ptRule->iGDBN_EdgeDieType);

    // Check all dies in the cluster zone
    for(int iDieX = iLowX; iDieX <= iHighX; iDieX++)
    {
        for(int iDieY = iLowY; iDieY <= iHighY; iDieY++)
        {
            bValidBin	= false;
            iBin		= GEX_WAFMAP_EMPTY_CELL;

            // If die is within the wafermap array
            if (iDieX >= 0 && iDieX < ptWafermap->SizeX && iDieY >=0 && iDieY < ptWafermap->SizeY)
            {
                iIndex = iDieX + (ptWafermap->SizeX*iDieY);

                iBin = ptWafermap->getWafMap()[iIndex].iBin;

                // Check if this bin belongs to our list of 'bad neighbors Bins'
                if((iBin >= 0) && (ptRule->pGDBN_BadBinList->IsInList(iBin)))
                    bValidBin = true;
            }

            // If no die (edge), check what to do
            if (bEdgeDie && (iBin == GEX_WAFMAP_EMPTY_CELL))
            {
                switch(ptRule->iGDBN_Algorithm)
                {
                case GEX_TPAT_GDBN_ALGO_SQUEEZE:	// Squeeze algorithm
                    // Check if die not tested (Bin value = -1)
                    if(ptRule->bGDBN_FailWaferEdges)
                        bValidBin = true;
                    break;

                case GEX_TPAT_GDBN_ALGO_WEIGHT:		// WEIGHTING algorithm
                    switch(ptRule->iGDBN_EdgeDieWeighting)
                    {
                    case GEX_TPAT_GPAT_EDGE_BAD:
                        bValidBin = true;
                        break;

                    case GEX_TPAT_GPAT_EDGE_GOOD:
                    case GEX_TPAT_GPAT_EDGE_IGNORE:
                        break;

                    case GEX_TPAT_GPAT_EDGE_SCALE:
                        bEdgeModeDetected = true;
                        break;
                    }
                    break;
                }
            }

            // If valid bin, proceed...
            if(bValidBin)
            {
                // Update fail count: Used for Squeeze algorithm
                iFailSqueezeCount++;

                // Update the die count in the cluster sub-zone: Left, Right, Top, bottom, Letf-top, Top-right, etc...
                if(iDieX < iX)
                {
                    // LEFT sub-zones
                    if(iDieY < iY)
                    {
                        iSubZoneOffset = GEX_TPAT_NEIGHBOR_LEFT_TOP;

                        // Update cluster Fail Weigth
                        lfFailWeitght += ptRule->iGDBN_DiagonalWeighting;
                    }
                    else
                        if(iDieY == iY)
                        {
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_LEFT;

                            // Update cluster Fail Weigth
                            lfFailWeitght += ptRule->iGDBN_AdjacentWeighting;
                        }
                        else
                        {
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_LEFT_BOTTOM;

                            // Update cluster Fail Weigth
                            lfFailWeitght += ptRule->iGDBN_DiagonalWeighting;
                        }
                }
                else
                    if(iDieX > iX)
                    {
                        // RIGHT sub-zones
                        if(iDieY < iY)
                        {
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_RIGHT_TOP;

                            // Update cluster Fail Weigth
                            lfFailWeitght += ptRule->iGDBN_DiagonalWeighting;
                        }
                        else
                            if(iDieY == iY)
                            {
                                iSubZoneOffset = GEX_TPAT_NEIGHBOR_RIGHT;

                                // Update cluster Fail Weigth
                                lfFailWeitght += ptRule->iGDBN_AdjacentWeighting;
                            }
                            else
                            {
                                iSubZoneOffset = GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM;

                                // Update cluster Fail Weigth
                                lfFailWeitght += ptRule->iGDBN_DiagonalWeighting;
                            }
                    }
                    else
                    {
                        // CENTER sub-zones
                        if(iDieY < iY)
                        {
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_CENTER_TOP;

                            // Update cluster Fail Weigth
                            lfFailWeitght += ptRule->iGDBN_AdjacentWeighting;
                        }
                        else
                        {
                            iSubZoneOffset = GEX_TPAT_NEIGHBOR_CENTER_BOTTOM;

                            // Update cluster Fail Weigth
                            lfFailWeitght += ptRule->iGDBN_AdjacentWeighting;
                        }
                    }

                // Update fail count per sub-zone
                icClusterSubZone[iSubZoneOffset]++;
            }
        }
    }

    int iBinSqueezed;
    switch(ptRule->iGDBN_Algorithm)
    {
    case GEX_TPAT_GDBN_ALGO_SQUEEZE:	// Squeeze algorithm
        // If not enough failures, return now
        if(iFailSqueezeCount < ptRule->iGDBN_FailCount)
            return false;

        // We have enough failures...but check if they are located in such zones that the centered (good bin)
        // appears to be squeezed in!
        if(ptRule->iGDBN_FailCount == 1)
        {
            // Special situation: if asked to remove any bin touching a bad bin, then fail even if no sandwich situation!
            iBinSqueezed  = icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT] + icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT];
            iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_TOP] + icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM];
            iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP] + icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_BOTTOM];
            iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM] + icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_TOP];
        }
        else
        {
            // Not stringent mode (requires real diagonal or sandwich situation)
            if(1)
            {
                iBinSqueezed  = icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_BOTTOM];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_TOP];
            }
            else
            {
                // Very stringent detection (diagonal, or close diagonal)
                iBinSqueezed  = icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_TOP];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_BOTTOM];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_BOTTOM];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT_TOP];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM]*icClusterSubZone[GEX_TPAT_NEIGHBOR_CENTER_TOP];
                iBinSqueezed += icClusterSubZone[GEX_TPAT_NEIGHBOR_LEFT_BOTTOM]*icClusterSubZone[GEX_TPAT_NEIGHBOR_RIGHT];
            }
        }

        // If result is 0, this means we never have two subzones appart from the good bin (in the center) that have failures
        if(iBinSqueezed == 0)
            return false;
        break;

    case GEX_TPAT_GDBN_ALGO_WEIGHT:		// WEIGHTING algorithm
        if(bEdgeModeDetected)
            lfFailWeitght *= ptRule->lfGDBN_EdgeDieWeightingScale;

        if(lfFailWeitght < ptRule->iGDBN_MinimumWeighting)
            return false;	// Total weight of surrounding dies not high enough to reject center good die!
        break;
    }

    // This good bin passes the conditions to be failed
    // Check if not already an outlier
    if(isDieOutlier(nCoordX, nCoordY, iBin))
        return false;

    QString strKey = QString::number(nCoordX) + "." + QString::number(nCoordY);
    CPatDieCoordinates *ptGoodBin = new CPatDieCoordinates;
    ptGoodBin->iDieX = nCoordX;
    ptGoodBin->iDieY = nCoordY;
    ptGoodBin->iSite = gexReport->getDieTestingSite(-1, 0, nCoordX, nCoordY);
    ptGoodBin->uFailType = GEX_TPAT_BINTYPE_BADNEIGHBORS;
    ptGoodBin->strRuleName = ptRule->strRuleName;

    // Save PAT definition in our list
    lContext->ptPatBadDieNeighbours.insert(strKey,ptGoodBin);	// Holds the Good Die X,Y position to fail

    return true;	// Must fail this good bin!
}
#endif // OLD_GDBN_ALGORITHM

/////////////////////////////////////////////////////////////////////////////
// Create a NNR test from its test
/////////////////////////////////////////////////////////////////////////////
bool	GexTbPatDialog::CreateTestNRR(int nGroupID,int nFileID,unsigned int &lTestNumber,int lPinmap,CParametricWaferMapNNR *ptNNR)
{
    CGexGroupOfFiles	*pGroup	= NULL;
    CGexFileInGroup		*pFile	= NULL;
    CTest *ptTestCell;
    CTest *ptTestCell_DieX;
    CTest *ptTestCell_DieY;
    CTest *ptTestCell_NRR;
    unsigned int ldOriginalTestNumber;
    QString strNrrTestName;
    CParametricWaferMap cParamWafMap;		// To hold Parametric wafermap
    CParametricWaferMap cParamWafMap_NNR;	// To hold NNR wafermap (delta values)

    // Get handle to group & file
    if ((nGroupID < 0) || (nGroupID >= gexReport->getGroupsList().count()))
        return false;
    pGroup = gexReport->getGroupsList().at(nGroupID);
    pFile = pGroup->pFilesList.at(nFileID);

    // Returns pointer to correct cell.
    if(pFile->FindTestCell(lTestNumber,lPinmap,&ptTestCell) !=1)
        return false;	// Error

    // Get pointers to DieX and DieY coordinates.
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST,&ptTestCell_DieX,true,false) != 1)
        return false;
    if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST,&ptTestCell_DieY,true,false) != 1)
        return false;

    // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
    strNrrTestName = "NRR for T" + QString::number(lTestNumber) + " - " + ptTestCell->strTestName;
    ldOriginalTestNumber = lTestNumber ;
    lTestNumber += 1700000000L;
    if(lTestNumber >= GEX_MAXTEST)
        lTestNumber = GEX_MAXTEST-1;
    if(pFile->FindTestCell(lTestNumber,GEX_PTEST,&ptTestCell_NRR,true,true, strNrrTestName.toLatin1().data()) !=1)
        return false;	// Error

    CPatInfo* lContext = GS::Gex::PATEngine::GetInstance().GetContext();
    if (lContext == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Empty Context");
        return false;
    }
    // Load Parametric wafermap: all sites merged
    cParamWafMap.FillWaferMapFromMultipleDatasets(ptTestCell, lContext->GetRecipeOptions().pGoodSoftBinsList);
    cParamWafMap_NNR.FillWaferMapFromMultipleDatasets(ptTestCell, lContext->GetRecipeOptions().pGoodSoftBinsList);

    // Build NNR map
    if(cParamWafMap_NNR.NNR_BuildMap(cParamWafMap,*ptNNR) == false)
        return false;

    // Copy values of NNR wafermap into TestCell_NRR created!
    int iIndex,iNRR_Index,iDieX,iDieY;
    double	lfValue;
    pFile->initDynamicTest(ptTestCell_NRR,cParamWafMap_NNR.mLowLimit,ptTestCell->llm_scal,cParamWafMap_NNR.mHighLimit,ptTestCell->hlm_scal,ptTestCell->res_scal);

    for(iIndex = 0; iIndex < ptTestCell_NRR->ldSamplesExecs; iIndex++)
    {
        iDieX = ptTestCell_DieX->m_testResult.isValidIndex(iIndex) ? (int) ptTestCell_DieX->m_testResult.resultAt(iIndex) : (int) GEX_C_DOUBLE_NAN;
        iDieY = ptTestCell_DieY->m_testResult.isValidIndex(iIndex) ? (int) ptTestCell_DieY->m_testResult.resultAt(iIndex) : (int) GEX_C_DOUBLE_NAN;

        // Get NRR value at location (X,Y)
        iNRR_Index = (iDieX-cParamWafMap_NNR.GetLowDieX()) + (cParamWafMap_NNR.GetSizeX() * (iDieY-cParamWafMap_NNR.GetLowDieY()));
        if((iNRR_Index >= 0) && (iNRR_Index < cParamWafMap_NNR.GetSizeX() * cParamWafMap_NNR.GetSizeY()))
            lfValue = cParamWafMap_NNR.cParametricWafMap[iNRR_Index].lfValue;
        else
            lfValue = GEX_C_DOUBLE_NAN;

        // Save NRR value into DataSamples array
        ptTestCell_NRR->m_testResult.pushResultAt(iIndex, lfValue);
    }

    // Re-insert NNR value of failing test (as they've been removed from original test dataset!)
    foreach(CPatOutlierNNR *ptNRR_OutlierPart, lContext->pNNR_OutlierTests)
    {
        if(ptNRR_OutlierPart->mTestNumber == ldOriginalTestNumber && ptNRR_OutlierPart->mPinmap == lPinmap)
        {
            // Clear data sample offset to start from
            iNRR_Index = pFile->findRunNumber(ptNRR_OutlierPart->mDieX,ptNRR_OutlierPart->mDieY,ptTestCell_NRR,0);
            while(iNRR_Index >= 0)
            {
                // Save NRR value into DataSamples array: 'forceResultAt' because we overload an existing cell!
                ptTestCell_NRR->m_testResult.forceResultAt(iNRR_Index, ptNRR_OutlierPart->mValue);

                // Find next datasample matching this same DieX,DieY location
                iNRR_Index = pFile->findRunNumber(ptNRR_OutlierPart->mDieX,ptNRR_OutlierPart->mDieY,ptTestCell_NRR,iNRR_Index+1);
            }
        }
    };

    // Build all statistics for this NNR test!
    bool bPercentile;
    QString pf=ReportOptions.GetOption("statistics", "cp_cpk_computation").toString();
    bPercentile = (pf=="percentile") ? true:false;
    pFile->computeAllStatistics(ptTestCell_NRR,bPercentile,GEX_HISTOGRAM_OVERLIMITS);	// Compute/Update all test statistics

    // success
    return true;
}

void GexTbPatDialog::OnAddGroup(){

    QString strDataSetName = "Group_" + QString::number(treeWidgetDataFiles->topLevelItemCount());

    bool bOk=true;
    QString strText = QInputDialog::getText(this, "Create New group",
                                            "Group name:", QLineEdit::Normal,
                                            strDataSetName, &bOk);
    if(!bOk)
        return;

    if(!strText.isEmpty())
        strDataSetName = strText;

    QTreeWidgetItem *pTreeWidgetItemGroup = AddGroup(-1);
    pTreeWidgetItemGroup->setText(0, strDataSetName);
    treeWidgetDataFiles->setCurrentItem(pTreeWidgetItemGroup, 0);

}

void GexTbPatDialog::retriveFieldsOptions(const QString &strGoup, const QStringList& lGroupStdfFiles)
{
    int iTopLevelItemIdx = 0;
    QTreeWidgetItem * pTreeWidgetItem = treeWidgetDataFiles->topLevelItem(0);

    while(pTreeWidgetItem != NULL)
    {
        if(pTreeWidgetItem->text(0) == strGoup)
        {
            // Get filtering option if any (note: only last filter in list is used!)
            QTreeWidgetItem *poItem = pTreeWidgetItem->child(0);
            m_cFields.strProcessPart = poItem->text(2).trimmed();

            // Find the Type of parts to process
            int iIndex=0;
            while(ProcessPartsItems[iIndex] != 0)
            {
                if( m_cFields.strProcessPart == ProcessPartsItems[iIndex])
                    break;	// found matching string
                iIndex++;
            };	// loop until we have found the string entry.

            m_cFields.strProcessPart = gexFileProcessPartsItems[iIndex];
            m_cFields.strProcessList = poItem->text(3);

            for(int iIdx = 0; iIdx < lGroupStdfFiles.count(); iIdx++)
            {
                QString strInputFile = lGroupStdfFiles.at(iIdx);

                // Clean input file name strings in case they come from drag & drop (start with string 'file:///')
                if(strInputFile.startsWith("file:///"))
                    strInputFile = QUrl(strInputFile).toLocalFile();

                // Remove leading \r & \n if any
                strInputFile.replace("\r","");
                strInputFile.replace("\n","");
                QFileInfo cFileInfo(strInputFile);
                m_cFields.mOutputFolderSTDF = cFileInfo.absolutePath();	// Output folder for new STDF
            }
            break;

        } else            // Get next files in list.
            pTreeWidgetItem = treeWidgetDataFiles->topLevelItem(++iTopLevelItemIdx);
    };
}

void GexTbPatDialog::showHideRemoveFile(QTreeWidgetItem *poCurrent, QTreeWidgetItem *)
{
    if(poCurrent)
        buttonRemoveFile->show();
    else
        buttonRemoveFile->hide();
}

bool GexTbPatDialog::userMergeOrCreateGroups()
{
    if(QMessageBox::question(this,
                             GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                             "Do you want to merge selected files?", QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes)
        return true;
    else
        return false;
}

int GexTbPatDialog::ProcessFile(bool lLaunchReportViewer, GS::Gex::PATProcessing &lFields,
                                QString& lErrorMessage)
{
    // Process file (one at a time)
    if (GS::Gex::PATEngine::GetInstance().GetContext() != NULL)
        GS::Gex::PATEngine::GetInstance().DeleteContext();
    GS::Gex::PATEngine::GetInstance().CreateContext();
    CPatInfo* lContext = GS::Gex::PATEngine::GetInstance().GetContext();
    GS::Gex::PATProcessWS   lPATProcess(GS::Gex::PATEngine::GetInstance().GetContext());
    int                     lStatus = NoError;

    if (lPATProcess.Execute(lFields) == false)
    {
        lErrorMessage   = lPATProcess.GetErrorMessage();
        lStatus         = WriteError;

        return lStatus;
    }

    // Generate a PAT report if output report format is defined
    if (m_cFields.strOutputReportFormat.isEmpty() == false)
    {
        if (GS::Gex::PATEngine::GetInstance().BuildPATReport(lContext->GetOutputDataFilename(),
                                                             m_cFields,
                                                             lContext->GetSiteList(),
                                                             lLaunchReportViewer) == false)
        {
            lErrorMessage   = GS::Gex::PATEngine::GetInstance().GetErrorMessage();
            lStatus         = ReportError;
            return lStatus;
        }
    }

    // If output report file is not STDF, (convert to other format if needed),
    // then erase the STDF temporary file created while computing PAT results
    switch(lFields.mOutputDatalogFormat)
    {
    case GEX_TPAT_DLG_OUTPUT_NONE:
        // Delete intermadiate STDF file.
        QFile::remove(lContext->GetOutputDataFilename());
        break;

    case GEX_TPAT_DLG_OUTPUT_STDF:
        // Keep STDF file created
        break;

    case GEX_TPAT_DLG_OUTPUT_ATDF:
        // Convert STDF to ATDF
        CSTDFtoATDF lATDFConverter(false);
        QFileInfo   lSTDFFileInfo(lContext->GetOutputDataFilename());
        QString     lATDFName   = lSTDFFileInfo.path() + "/" +
                lSTDFFileInfo.completeBaseName() + ".atd";

        lATDFConverter.SetProcessRecord(true);          // Convert ALL STDF records to ATDF
        lATDFConverter.SetWriteHeaderFooter(false);     // Disable DUMP comments in ATDF file.

        if (lATDFConverter.Convert(lContext->GetOutputDataFilename(), lATDFName) == false)
        {
            lATDFConverter.GetLastError(lErrorMessage);
            GSLOG(SYSLOG_SEV_WARNING, "Failed to convert STDF output to ATDF. ");
            lStatus = WriteError;
        }
        else
            // Delete intermadiate STDF file.
            QFile::remove(lContext->GetOutputDataFilename());

        break;
    }

    return lStatus;
}

void GexTbPatDialog::initComboBoxOutputWafermapFormat()
{
    comboBoxOutputWafermapFormat->clear();

    QIcon iconWafer;
    iconWafer.addFile(QString::fromUtf8(":/gex/icons/options_wafmap.png"), QSize(), QIcon::Normal, QIcon::On);
    QIcon iconDisable;
    iconDisable.addFile(QString::fromUtf8(":/gex/icons/stop.png"), QSize(), QIcon::Normal, QIcon::On);

    comboBoxOutputWafermapFormat->addItem(iconDisable, QString("Disabled"), QString(""));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString("TSMC (inkless ASCII wafermap)"),QString("tsmc"));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString("G85 Ascii (Semi85 inkless assembly)"),QString("g85"));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString("G85 XML (Semi85 XML format)"),QString("xml_g85"));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString("SEMI E142 (XML inkless wafermap)"),QString("semi_e142"));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString("SEMI E142 - Integer 2 (XML inkless wafermap)"),
                                          QString("semi_e142_integer2"));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString("Simple INF (KLA/INF simplified)"),QString("sinf"));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString("KLA INF (full INF wafermap)"),QString("kla_sinf"));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString("TEL P8 wafer prober format"),QString("telp8"));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString("STIF (Hard-Bin map)"), QString("stif"));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString("Olympus AL2000 (soft bin wafermap)"),
                                          QString("olympus_al2000"));
    comboBoxOutputWafermapFormat->addItem(iconWafer, QString(".SUM (summary ASCII map)"),
                                          QString("sum_micron"));

}

#endif
