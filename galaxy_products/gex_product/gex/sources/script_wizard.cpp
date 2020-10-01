///////////////////////////////////////////////////////////
// GEX Scripting Wizard Dialog box
///////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#if defined unix || __MACH__
#include <stdlib.h>
#endif
#include <time.h>

#include <QDesktopServices>
#include <QFileDialog>
#include <qmessagebox.h>
#include <qsplitter.h>
#include "message.h"
#include "csl/csl_engine.h"
#include "qfile.h"
#include "browser_dialog.h"
#include "browser.h"
#include "engine.h"
#include "command_line_options.h"
#include "report_build.h"
#include "report_options.h"
#include "db_transactions.h"
#include "pickfilter_dialog.h"
#include "getstring_dialog.h"
#include "import_all.h"
#include "script_wizard.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include "db_engine.h"
#include <gqtl_log.h>
#include "product_info.h"
#include <QDesktopServices>
#include <QProgressBar>
#include "gex_report.h"
#include "onefile_wizard.h"
#include "comparefiles_wizard.h"
#include "mergefiles_wizard.h"
#include "mixfiles_wizard.h"
#include "db_onequery_wizard.h"
#include "compare_query_wizard.h"

// in main.cpp
extern CGexReport *         gexReport;				// Handle to report class
extern GexMainwindow *	pGexMainWindow;
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// in csl/Zgexlib.cpp
//extern GexDatabaseQuery *getScriptQuery(void);	// Returns handle to cQuery structure

// In ZcGexLib.cpp
extern const GexDatabaseQuery& getScriptQuery(void);

///////////////////////////////////////////////////////////
// Converts string to be script compatible...
///////////////////////////////////////////////////////////
void ConvertToScriptString(QString &strFile)
{
    // Replace '\' by '\\' in the path
    int i = strFile.indexOf( '\\' );
    while(i>=0)
    {
        strFile.insert(i,'\\');
        i = strFile.indexOf( '\\',i+2 );
    };
    // Replace ' by \'
    i = strFile.indexOf( '\'' );
    while(i>=0)
    {
        strFile.insert(i,'\\');
        i = strFile.indexOf( '\'',i+2 );
    };
    // Replace , by '\x2c'
    strFile = strFile.replace(",","\\x2c");

    strFile = strFile.replace("\n", "\\x0d");
    strFile = strFile.replace("\r", "\\x0a");
}

QString ConvertToScriptString(const QString &strFile)
{
    QString lFileName = strFile;

    // Replace '\' by '\\' in the path
    int i = lFileName.indexOf( '\\' );
    while(i>=0)
    {
        lFileName.insert(i,'\\');
        i = lFileName.indexOf( '\\',i+2 );
    };
    // Replace ' by \'
    i = lFileName.indexOf( '\'' );
    while(i>=0)
    {
        lFileName.insert(i,'\\');
        i = lFileName.indexOf( '\'',i+2 );
    };
    // Replace , by '\x2c'
    lFileName = lFileName.replace(",","\\x2c");

    lFileName = lFileName.replace("\n", "\\x0d");
    lFileName = lFileName.replace("\r", "\\x0a");

    return lFileName;
}

///////////////////////////////////////////////////////////
// Decodes script string...
///////////////////////////////////////////////////////////
void ConvertFromScriptString(QString &strFile)
{
    strFile.replace("\\'","'");
    strFile.replace("\\\\","\\");
    strFile = strFile.replace("\\x2c",",");

    strFile = strFile.replace("\\x0d", "\n");
    strFile = strFile.replace("\\x0a", "\r");
}

///////////////////////////////////////////////////////////
// Scripting invoked from Toolbar
///////////////////////////////////////////////////////////
void GexMainwindow::ScriptingLink(void)
{
    // Reresh HTML page link.
    QString strHtmlPage = GEX_BROWSER_ACTIONLINK;
    strHtmlPage += GEX_BROWSER_SCRIPTING_LINK;

    LoadUrl(strHtmlPage);
}


///////////////////////////////////////////////////////////
// Scripting invoked from HTML hyperlink
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_Scripting(void)
{
    // Enable/disable some features...
    if (GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
            return;
    }

    // Show Option page
    ShowWizardDialog(GEX_SCRIPTING);
}

///////////////////////////////////////////////////////////
// GEX Scripting Wizard dialog box.
///////////////////////////////////////////////////////////
GexScripting::GexScripting( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "");

    setupUi(this);
    setModal(modal);

    setWindowFlags(Qt::FramelessWindowHint);
    move(0,0);

    QObject::connect(buttonAddFile,				SIGNAL(clicked()), this, SLOT(OnAddFile()));
    QObject::connect(buttonProperties,			SIGNAL(clicked()), this, SLOT(OnProperties()));
    QObject::connect(buttonRemoveFile,			SIGNAL(clicked()), this, SLOT(OnRemoveFile()));
    QObject::connect(buttonRunFile,				SIGNAL(clicked()), this, SLOT(OnRunScript()));
    QObject::connect(buttonNewScript,			SIGNAL(clicked()), this, SLOT(OnNewScript()));
    QObject::connect(buttonEditScript,			SIGNAL(clicked()), this, SLOT(OnEditScript()));
    QObject::connect(buttonLoadRunScript,		SIGNAL(clicked()), this, SLOT(OnLoadRunScript()));
    QObject::connect(buttonEditExecutedScript,	SIGNAL(clicked()), this, SLOT(OnEditExecutedScript()));
    QObject::connect(buttonRunScriptAgain,		SIGNAL(clicked()), this, SLOT(OnRunScriptAgain()));
    QObject::connect(PushButtonInputOk,			SIGNAL(clicked()), this, SLOT(OnConsoleInputReady()));
    QObject::connect(PushButtonScriptPrompt,	SIGNAL(clicked()), this, SLOT(OnConsoleInputFileName()));
    QObject::connect(mUi_treeWidget,			SIGNAL(doubleClicked(QModelIndex)),this,SLOT(OnRunScript()));


    // Console input
    TextLabelInput->hide();
    PushButtonScriptPrompt->hide();
    LineEditScriptPrompt->hide();
    PushButtonInputOk->hide();

    mInputReady         = false;

    // Initialized by startup script: holds date of last Galaxy message reminder (Ad)
    lGexConfLastReminder = (long) time(NULL);

    // Sets slot so we know when user presses RETURN / OK after entering input string
    connect( LineEditScriptPrompt,	SIGNAL(returnPressed()),			this, SLOT(OnConsoleInputReady()));

    // Signal that a Hyperlink clicked in the Script HTML window.
    connect( HtmlWindow,			SIGNAL(anchorClicked(const QUrl&)),  this, SLOT(OnHtmlLink(const QUrl&)));

    // Hide the HTML console window
    HtmlWindow->hide();

    // Set some of the Environment variable to default initial value
    strGexConfLastFilePathSelected = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();

    // Connect CSL engine
    connect(&GS::Gex::CSLEngine::GetInstance(), SIGNAL(scriptStarted(const QString&, bool)),
            this, SLOT(OnScriptStarted(const QString&, bool)));
    connect(&GS::Gex::CSLEngine::GetInstance(), SIGNAL(scriptFinished(GS::Gex::CSLStatus)),
            this, SLOT(OnScriptFinished()));
    connect(&GS::Gex::CSLEngine::GetInstance(), SIGNAL(scriptLog(QString)),
            this, SLOT(OnScriptLog(QString)));
}

///////////////////////////////////////////////////////////
// GexScripting: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void GexScripting::reject(void)
{
}

void GexScripting::RunStartupScript(const QString &strStartupScript)
{
    QFile cFile(strStartupScript);

    if(cFile.exists() == false)
    {
        // Debug message (written only if gex started with -D option)
        GSLOG(SYSLOG_SEV_WARNING, "Default script doesn't exist. Let's create it !");

        // Startup script doesn't exist...create the default list !
        QString	strScriptName;
        QStringList strListTreeItem;
        //GexScriptingTreeItem *pTreeItem;

        // Script: Hello Quantix world
        strScriptName = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strScriptName += "/samples/hello.csl";
        strListTreeItem.clear();
        strListTreeItem << QString("Prints \'Hello Quantix World\'")<<strScriptName;
        /*pTreeItem = */new GexScriptingTreeItem(mUi_treeWidget,strListTreeItem);

        // Script: Menu_datalog
        strScriptName = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strScriptName += "/samples/menu_datalog.csl";
        strListTreeItem.clear();
        strListTreeItem << QString("Datalog report")<<strScriptName;
        /*pTreeItem = */new GexScriptingTreeItem(mUi_treeWidget,strListTreeItem);

        // Script: Menu_datalog
        strScriptName = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strScriptName += "/samples/menu_wafermap.csl";
        strListTreeItem.clear();
        strListTreeItem << QString("Wafermap report")<<strScriptName;
        /*pTreeItem = */new GexScriptingTreeItem(mUi_treeWidget,strListTreeItem);

        // Script: HANGMAN game!!
        strScriptName = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strScriptName += "/samples/hangman-html/hangman-html.csl";
        strListTreeItem.clear();
        strListTreeItem << QString("Games: HANGMAN (HTML)")<<strScriptName;
        /*pTreeItem = */new GexScriptingTreeItem(mUi_treeWidget,strListTreeItem);
    }

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;
}

///////////////////////////////////////////////////////////
// Writes 'Preferences' section in script file
///////////////////////////////////////////////////////////
void GexScripting::WritePreferencesSection(FILE *hFile)
{
    // Writes 'Favorite Scripts' section
    fprintf(hFile,"// Function to setup the GEX 'Preferences'\n");
    fprintf(hFile,"SetPreferences()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n");
    fprintf(hFile,"  // Erase the list in case it is not empty\n");
    fprintf(hFile,"  gexFavoriteScripts('clear');\n");
    fprintf(hFile,"\n");
    fprintf(hFile,"  // Insert scripts path + title in the list\n");


    QTreeWidgetItem *pTreeItem;
    pTreeItem = mUi_treeWidget->topLevelItem(0);

    while(pTreeItem != NULL)
    {
        // Replace '\' by '\\' in the path
        QString string = pTreeItem->text(1);
        // Convert file path to be script compatible '\' become '\\' and ' by \'
        ConvertToScriptString(string);
        fprintf(hFile,"  gexFavoriteScripts('insert','%s'",string.toLatin1().constData());

        // Replace '\' by '\\' in the title
        string = pTreeItem->text(0);
        // Convert file path to be script compatible '\' become '\\'
        ConvertToScriptString(string);
        fprintf(hFile,",'%s');\n",string.toLatin1().constData());

        // Move to next Script entry
        pTreeItem = mUi_treeWidget->itemBelow(pTreeItem);
    }

    // Message to console saying 'success loading list'
    fprintf(hFile,"\n  sysLog('List of Quantix Favorites Scripts loaded!');\n");
    fprintf(hFile,"\n");


    // Holds last file name selected...convert '\' to '\\' so scripting doesn't get corrupted!
    ConvertToScriptString(strGexConfLastFilePathSelected);
    fprintf(hFile,"  sysConfig('Global','LastFilePath','%s');\n",strGexConfLastFilePathSelected.toLatin1().constData());

    // Holds last time a reminder message box was written to the screen
    fprintf(hFile, "  sysConfig('Global','LastReminder','%ld');\n",
            lGexConfLastReminder);

    // Close function
    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");
}

///////////////////////////////////////////////////////////
// Function name	: EraseFavoriteList
// Description	    : Clears all items in the list.
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::EraseFavoriteList(void)
{
    // Erase tree + delete all objects from memory
    mUi_treeWidget->clear();
}

///////////////////////////////////////////////////////////
// Function name	: InsertFavoriteList
// Description	    : Insert script to the favorite list
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::InsertFavoriteList(const char *szPath,const char *szTitle)
{
    //GexScriptingTreeItem *pTreeItem;
    QStringList	strListTreeItemID;
    strListTreeItemID << QString(szTitle) << QString(szPath) ;
    /*pTreeItem = */new GexScriptingTreeItem(mUi_treeWidget,strListTreeItemID);
}

///////////////////////////////////////////////////////////
// Function name	: GexScripting::OnAddFile
// Description	    : Add Script file to the 'Favorites' list
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnAddFile(void)
{
    // Prompt user to select the script file of choice.
    QString fn = QFileDialog::getOpenFileName(this, "", "", "Script File (*.csl);;All Files (*.*)");

    // If no file selected, ignore command and return to home page.
    if(fn.isEmpty())
        return;

    // Ask for the Description string
    char	szString[1024];
    GetStringDialog cScriptDescription(0, true, Qt::MSWindowsFixedSizeDialogHint);

    cScriptDescription.setDialogText("Add Script to Favorites",
        "Description:",
        "",
        "Enter your Script description");

    // Display Edit window
    if(cScriptDescription.exec() == 0)
        return;	// User canceled task.

    // Get string entered if any.
    cScriptDescription.getString(szString);
    // Add leading space for nicer display!
    strcat(szString," ");


    // Add this entry to the list.
    InsertFavoriteList(fn.toLatin1().constData(), szString);

    emit OptionChanged();
}

///////////////////////////////////////////////////////////
// Function name	: OnNewScript
// Description	    : Creates a new script (launch text editor)
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnNewScript(void)
{
#ifdef _WIN32
    ShellExecuteA(NULL,
           "open",
        ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(),   //ReportOptions.strTextEditor.toLatin1().constData(),
           NULL,
           NULL,
           SW_SHOWNORMAL);
#endif
#if defined unix || __MACH__
    char	szString[2048];
    sprintf(szString,"%s&",
            ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data() //ReportOptions.strTextEditor.toLatin1().constData()
            );
  if (system(szString) == -1) {
    //FIXME: send error
  }
#endif
}

///////////////////////////////////////////////////////////
// Function name	: OnEditScript
// Description	    : Edit selected new script (launch text editor)
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnEditScript(void)
{
    QList<QTreeWidgetItem*> cListSelectedTreeItem;

    // If no selection, just ignore action.
    cListSelectedTreeItem = mUi_treeWidget->selectedItems();
    if (cListSelectedTreeItem.count()==0)
        return;

    // Edit selected file
    QString strTreeScriptName = cListSelectedTreeItem[0]->text(1);
    GexMainwindow::OpenFileInEditor(strTreeScriptName, GexMainwindow::eDocText);
//    if (ReportOptions.GetOption("preferences","use_os_default").toBool() && QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(strTreeScriptName)))) //Case 4619 - Use OS's default program to open file
//      return;
//#ifdef _WIN32
//	// If file includes a space, we need to bath it between " "
//	if(strTreeScriptName.indexOf(' ') != -1)
//	{
//		strTreeScriptName = "\"" + strTreeScriptName;
//		strTreeScriptName = strTreeScriptName + "\"";
//	}
//	ShellExecuteA(NULL,
//		"open",
//		ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(),   //ReportOptions.strTextEditor.toLatin1().constData(),
//		   strTreeScriptName.toLatin1().constData(),
//		   NULL,
//		   SW_SHOWNORMAL);
//#endif
//#if defined unix || __MACH__
//	char	szString[2048];
//	sprintf(szString,"%s %s&",
//		ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(), //ReportOptions.strTextEditor.toLatin1().constData(),
//		strTreeScriptName.toLatin1().constData());
//  if (system(szString) == -1) {
//    //FIXME: send error
//  }
//#endif
}

///////////////////////////////////////////////////////////
// Function name	: GexScripting::OnRemoveFile
// Description	    : Remove selected Script from list
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnRemoveFile(void)
{
    // If no item in list, just return!
    if(mUi_treeWidget->topLevelItemCount() <= 0)
        return;

    // Ask for confirmation...
    bool lOk;
    GS::Gex::Message::request("Remove Script from Favorites", "Confirm to remove Script from list ?", lOk);
    if (! lOk)
    {
        return;
    }

    // Get pointer of selected Script entry, and remove it!
    QList<QTreeWidgetItem*> cListTreeItemSelection;

    // Remove group from list
    cListTreeItemSelection=mUi_treeWidget->selectedItems();
    if (cListTreeItemSelection.count()>0)
        mUi_treeWidget->takeTopLevelItem(mUi_treeWidget->indexOfTopLevelItem(cListTreeItemSelection[0]));

    emit OptionChanged();
}

///////////////////////////////////////////////////////////
// Function name	: GexScripting::OnProperties
// Description	    : Edit file properties: site+parts to process
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnProperties(void)
{
    QList<QTreeWidgetItem*> cListTreeSelection;

    // If no selection, just ignore action.
    cListTreeSelection = mUi_treeWidget->selectedItems();
    if (cListTreeSelection.count() == 0)
        return;


    // Shows current Description, lets user edit it.
    char	szTreeString[1024];
    strcpy(szTreeString,cListTreeSelection[0]->text(0).toLatin1().constData());
    GetStringDialog cTreeScriptDescription(0, true);

    cTreeScriptDescription.setDialogText("Script Properties",
                                         "Description:",
                                         szTreeString,
                                         "Enter your Script description");

    // Display Edit window
    if (cTreeScriptDescription.exec()==0)
        return; // User canceled task.

    // Get string entered if any.
    cTreeScriptDescription.getString(szTreeString);
    // Add leading space for nicer display!
    strcat(szTreeString," ");
    // Get new path, and update list view content.
    cTreeScriptDescription.getString(szTreeString);
    cListTreeSelection[0]->setText(0,szTreeString);

    emit OptionChanged();
}

///////////////////////////////////////////////////////////
// Function name	: GexScripting::OnRunScript
// Description	    : Runs selected script!
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnRunScript(void)
{
    QList<QTreeWidgetItem*> cListTreeItemSelection;

    // If no selection, just ignore action.
    cListTreeItemSelection=mUi_treeWidget->selectedItems();
    if(cListTreeItemSelection.count()==0)
        return;

    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        GS::Gex::Message::information(
            "", "This is a 'Standalone' GEX release: it doesn't allow "
            "scripting execution.\nTo run user scripts, you need to "
            "upgrade your Quantix Examinator.\n\nPlease contact " +
            QString(GEX_EMAIL_SALES));
        return;
    }

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Keep track of the last script executed...
    TextLabelScriptExecuted->setText(cListTreeItemSelection[0]->text(1));

    // Execute selected script.
    RunScript(TextLabelScriptExecuted->text());
}

///////////////////////////////////////////////////////////
// Function name	: GexScripting::OnLoadRunScript
// Description	    : Loads+Runs picked script with browser.
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnLoadRunScript(void)
{
    // Prompt user to select the script file of choice.
    QFileDialog qfdOpenFileName(this);
    qfdOpenFileName.setNameFilter("Script File (*.csl);;All Files (*.*)");
    qfdOpenFileName.setAcceptDrops(false);      // solve problem under linux if direct drop in lineEdit
    //qfdOpenFileName.setOption(QFileDialog::DontUseNativeDialog, false);

    QString strScript;
    if(qfdOpenFileName.exec())
    {
        if(!(qfdOpenFileName.selectedFiles()).isEmpty())
            strScript = qfdOpenFileName.selectedFiles().first();
    }
    //QString strScript = QFileDialog::getOpenFileName(this, "", "", "Script File (*.csl);;All Files (*.*)");

    // If no file selected, ignore command and return to home page.
    if(strScript.isEmpty())
        return;

    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        GS::Gex::Message::information(
            "", "This is a 'Standalone' GEX release: it doesn't allow "
            "scripting execution.\nTo run user scripts, you need to upgrade "
            "your Quantix Examinator.\n\nPlease contact " +
            QString(GEX_EMAIL_SALES));
        return;
    }

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Keep track of the last script executed...
    TextLabelScriptExecuted->setText(strScript);

    // Execute selected script.
    RunScript(TextLabelScriptExecuted->text());
}

void GexScripting::RunScript(const QString& fileName)
{
    // If the option file is modified, then, ask the user if he wants to save the current profile and load the
    // profile existing in the executed csl file
    pGexMainWindow->SaveCurrentProfile();

    // save options from the csl file into the profile file
    FILE *lProfile=NULL;
    // Creates header of Gex Assistant script file.
    QString lFileName = GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + GEX_SCRIPTING_CENTER_NAME;
    lProfile = fopen(lFileName.toLatin1().constData(), "w");
    if(lProfile == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR ,QString("error : can't create script file '%1' !")
                                       .arg(lFileName).toLatin1().constData());
    }
    if(!ReportOptions.WriteOptionSectionToFile(lProfile))
    {
        GEX_ASSERT(false);
    }
    fclose(lProfile);
    pGexMainWindow->LoadProfileFromScriptingCenter();
    pGexMainWindow->ClearDetachableWindows();
    GS::Gex::CSLEngine::GetInstance().RunScript(fileName);

    // Populate the Data Tab if needed
    if ((  GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eExaminatorPro
        || GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eExaminatorPAT
        || GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eTerProPlus
        || GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eTerOEM)
        && pGexMainWindow->GetAnalyseMode() != GexMainwindow::InvalidAnalyse)
        PopulateDataTab(fileName);
}

bool GexScripting::PopulateDataTab(const QString& fileName)
{
    if (  GS::LPPlugin::ProductInfo::getInstance()->getProductID() != GS::LPPlugin::LicenseProvider::eExaminatorPro
        && GS::LPPlugin::ProductInfo::getInstance()->getProductID() != GS::LPPlugin::LicenseProvider::eExaminatorPAT
        && GS::LPPlugin::ProductInfo::getInstance()->getProductID() != GS::LPPlugin::LicenseProvider::eTerProPlus
        && GS::LPPlugin::ProductInfo::getInstance()->getProductID() != GS::LPPlugin::LicenseProvider::eTerOEM)
        return false;

    // Open the script file
    QFile lFile(fileName);
    if (!lFile.open(QIODevice::ReadOnly))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Can't open input file %1 ").arg(fileName).toLatin1().constData());
        lFile.close();
        return false;
    }

    QStringList lDataProcessingSection;
    // Assign file I/O stream
    QTextStream lStreamFile(&lFile);

    // Check the compatibility
    while (!lStreamFile.atEnd())
    {
        QString lString = lStreamFile.readLine();
        if (lString.contains("// BEGIN PROCESS_DATA"))
        {
            lString = lStreamFile.readLine();
            while (!lString.contains("// END PROCESS_DATA") && !lStreamFile.atEnd())
            {
                lDataProcessingSection.append(lString);
                lString = lStreamFile.readLine();
            }
        }
    }
    lFile.close();

    switch (pGexMainWindow->GetAnalyseMode())
    {
        case GexMainwindow::SingleFile:
            if (pGexMainWindow->pWizardOneFile)
            {
                pGexMainWindow->SetWizardType(GEX_ONEFILE_WIZARD);
                return pGexMainWindow->pWizardOneFile->FillDataTab(lDataProcessingSection);
            }
            break;
        case GexMainwindow::CompareFiles:
            if (pGexMainWindow->pWizardCmpFiles)
            {
                pGexMainWindow->SetWizardType(GEX_CMPFILES_WIZARD);
                return pGexMainWindow->pWizardCmpFiles->FillDataTab(lDataProcessingSection);
            }
            break;
        case GexMainwindow::MergeFiles:
            if (pGexMainWindow->pWizardAddFiles)
            {
                pGexMainWindow->SetWizardType(GEX_ADDFILES_WIZARD);
                return pGexMainWindow->pWizardAddFiles->FillDataTab(lDataProcessingSection);
            }
            break;
        case GexMainwindow::CompareGroupOfFiles:
            if (pGexMainWindow->pWizardMixFiles)
            {
                pGexMainWindow->SetWizardType(GEX_MIXFILES_WIZARD);
                return pGexMainWindow->pWizardMixFiles->FillDataTab(lDataProcessingSection);
            }
            break;
        case GexMainwindow::SingleDataset:
            if(pGexMainWindow->pWizardOneQuery)
            {
                pGexMainWindow->SetWizardType(GEX_ONEQUERY_WIZARD);
                return pGexMainWindow->pWizardOneQuery->FillDataTab(lDataProcessingSection[0]);
            }
            break;
        case GexMainwindow::CompareDatasets:
            if (pGexMainWindow->pWizardCmpQueries)
            {
                pGexMainWindow->SetWizardType(GEX_CMPQUERY_WIZARD);
                return pGexMainWindow->pWizardCmpQueries->FillDataTab(lDataProcessingSection[0]);
            }
            break;
        default:
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Function name	: GexScripting::OnEditExecutedScript
// Description	    : Edit the last executed script.
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnEditExecutedScript(void)
{
    // If no script executed yet...ignore!
    QString strScript = TextLabelScriptExecuted->text();
    if(strScript.isEmpty()==true)
        return;
    GexMainwindow::OpenFileInEditor(strScript, GexMainwindow::eDocText);
//    if (ReportOptions.GetOption("preferences","use_os_default").toBool() && QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(strScript)))) //Case 4619 - Use OS's default program to open file
//      return;
//	// Trial to replace horrible ifdef code with nice cross platform openUrl
//	if (QDesktopServices::openUrl( QUrl( QString("file:///%1").arg(strScript) ) ))
//		return;
//	GSLOG(SYSLOG_SEV_NOTICE, QString(" error in QDesktopServices::openUrl %1 ! using old school codes...").arg( strScript).toLatin1().constData() );

//#ifdef _WIN32
//	// If file includes a space, we need to bath it between " "
//	if(strScript.indexOf(' ') != -1)
//	{
//		strScript = "\"" + strScript;
//		strScript = strScript + "\"";
//	}
//	ShellExecuteA(NULL,
//		   "open",
//		ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(),   //ReportOptions.strTextEditor.toLatin1().constData(),
//		   strScript.toLatin1().constData(),
//		   NULL,
//		   SW_SHOWNORMAL);
//#endif
//#if defined unix || __MACH__
//	char	szString[2048];
//	sprintf(szString,"%s %s&",
//	 ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(),	//ReportOptions.strTextEditor.toLatin1().constData(),
//		strScript.toLatin1().constData());
//  if (system(szString) == -1) {
//    //FIXME: send error
//  }
//#endif
}

///////////////////////////////////////////////////////////
// Function name	: OnRunScriptAgain::OnEditExecutedScript
// Description	    : Restart the last executed script.
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnRunScriptAgain(void)
{
    // If no script executed yet...ignore!
    QString strScript = TextLabelScriptExecuted->text();
    if(strScript.isEmpty()==true)
        return;

    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        GS::Gex::Message::information(
            "", "This is a 'Standalone' GEX release: it doesn't allow "
            "scripting execution.\nTo run user scripts, you need to upgrade "
            "your Quantix Examinator.\n\nPlease contact " +
            QString(GEX_EMAIL_SALES));
        return;
    }

    // Reset HTML sections to create flag: ALL pages to create.
    pGexMainWindow->iHtmlSectionsToSkip = 0;
    pGexMainWindow->m_bDatasetChanged	= true;

    // Execute selected script.
    GS::Gex::CSLEngine::GetInstance().RunScript(TextLabelScriptExecuted->text());
}

void GexScripting::OnScriptStarted(const QString & scriptName, bool startup)
{
    if (startup)
        RunStartupScript(scriptName);

    // Updates WizardScripting 'last script executed' name
    TextLabelScriptExecuted->setText(scriptName);

    // Disables some scripting command until current script is done!
    EnableScriptToolBar(false);

    // Gives Scripting a copy of the GUI handle.
    Scripting_IO_Init(this);

    // Display BANNER in script console, then launch script
    ResetScriptConsole();	// Erase screen + reset variables

    QString strBanner;
    strBanner = "------------------------------------------------------";
    strBanner += "\n> Date: ";

    time_t cTime;
    time(&cTime);
    strBanner += ctime(&cTime);
    strBanner += "> ";
    strBanner += GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
    strBanner += "\n";
    strBanner += "------------------------------------------------------";
    strBanner += "\n\n";

    WriteMessage(strBanner);
}

void GexScripting::OnScriptFinished()
{
    CloseScriptConsole();
}

void GexScripting::OnScriptLog(const QString & logMessage)
{
    WriteScriptMessage(logMessage, true);
}


///////////////////////////////////////////////////////////
// Function name	: WriteMessage
// Description	    : Printes given text to Script console window
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::WriteMessage(QString strMessage)
{
    TextEditScript->append(strMessage);
#if defined unix || __MACH__
    // Under Unix, print message to 'stdout'...in case GEX is running in hidden mode.
    if(GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden() == true)
    {
                printf("%s", strMessage.toLatin1().constData());
        fflush(stdout);
    }
#endif
}

void GexScripting::ResetScriptConsole(void)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Reset script console");
    // Erase screen
    TextEditScript->clear();

    // Disables Input buffer related buttons
    TextLabelInput->hide();
    PushButtonScriptPrompt->hide();
    LineEditScriptPrompt->hide();
    PushButtonInputOk->hide();
    LineEditScriptPrompt->setFocus();

    // Update status bar
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Script: running...");

}

///////////////////////////////////////////////////////////
// Function name	: GetScriptConsoleMessage
// Description	    : Returns the text entered in console
//						window (if any), or NULL if nothing
//						available...
// Return type		: void
///////////////////////////////////////////////////////////
QString GexScripting::GetScriptConsoleMessage(QString strField)
{
    // Save arguments (may be empty if call is from 'sysPrompt()'
    mDatabaseField = strField;

    // If the INPUT (LineEdit object) is disabled, enable + reset it!
    if(LineEditScriptPrompt->isVisible() == false)
    {
        // This is the first call to read the console buffer: enable input!
        mInputReady = false;
        TextLabelInput->show();
        PushButtonScriptPrompt->show();
        LineEditScriptPrompt->show();
        PushButtonInputOk->show();
        LineEditScriptPrompt->setFocus();

        // Erase input buffer content
        LineEditScriptPrompt->clear();
        return NULL;	// Tells script thread that no text is available yet!
    }

    // User has NOT entered a string+pressed 'return' or clicked OK yet!
    if(	mInputReady == false)
        return NULL;

    // Get the message, disable Edit box, return text to the child Script thread!
    TextLabelInput->hide();
    PushButtonScriptPrompt->hide();
    LineEditScriptPrompt->hide();
    PushButtonInputOk->hide();
    return LineEditScriptPrompt->text();
}

///////////////////////////////////////////////////////////
// Function name	: OnConsoleInputReady
// Description	    : When <CR-LF> pressed or 'Ok' button
//						and user has entered a string in
//						the console input Edit box.
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnConsoleInputReady(void)
{
    mInputReady = true;
    WriteMessage(LineEditScriptPrompt->text());
}


///////////////////////////////////////////////////////////
// Function name	: OnConsoleInputFileName
// Description	    : User wants to enter a file path in the
//						console input buffer using the 'browse'
//						button....or Select data fields for database query
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnConsoleInputFileName(void)
{
    QString	strSelection="";

    // Check if caller is sysPrompt() or gexPromptQuery()...
    if(mDatabaseField.isEmpty() == false)
    {
        // Dataset source results from running a Query.
        ReportOptions.bQueryDataset = true;

        // Find all files matching query into the group.
        GexDatabaseEntry * pDatabaseEntry = NULL;
        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(getScriptQuery().strDatabaseLogicalName);
        if(pDatabaseEntry == NULL)
            return;

        // Fill Filter list with relevant strings
        PickFilterDialog dPickFilter;
        dPickFilter.fillList(getScriptQuery(), mDatabaseField);

        // Prompt dialog box, let user pick Filter string from the list
        if(dPickFilter.exec() != 1)
            return;	// User 'Abort'

        // Save the list selected into the edit field...unless it is already in!
        strSelection = dPickFilter.filterList();
    }
    else
    {
        // User wants to enter a file name
        GS::Gex::SelectDataFiles cSelectFile;
        QString szPath="";
        strSelection = cSelectFile.GetSingleFile(this,szPath,"Select your file");
    }

    // If no file selected, ignore.
    if(strSelection.isEmpty())
        return;

    // Copies the selected text (file or Query fields) into the console input buffer
    LineEditScriptPrompt->setText(strSelection);
    LineEditScriptPrompt->setFocus();

    // Once file is selected, automatically validate the entry (simulates 'OK').
    OnConsoleInputReady();
}

///////////////////////////////////////////////////////////
// Function name	: CloseScriptConsole
// Description	    : Hides console input buttons+process bar
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::CloseScriptConsole()
{
    // Hide input fields
    TextLabelInput->hide();
    PushButtonScriptPrompt->hide();
    LineEditScriptPrompt->hide();
    PushButtonInputOk->hide();

    // Hides progress bar...since script is completed
    pGexMainWindow->UpdateProgressStatus(true,0,-1,true);

    // Update status bar: hide the script status label
    GS::Gex::Engine::GetInstance().UpdateLabelStatus("");

    // Refresh buttons in script tool bar to ENABLE state
    EnableScriptToolBar(true);
}

///////////////////////////////////////////////////////////
// Function name	: EnableScriptToolBar
// Description	    : Enable/Disable some scripting command
//					  until current script is done!
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::EnableScriptToolBar(bool bEnable)
{
    // Enable-Disable icons in Scripting page
    buttonRunFile->setEnabled(bEnable);
    buttonLoadRunScript->setEnabled(bEnable);
    buttonRunScriptAgain->setEnabled(bEnable);
}


///////////////////////////////////////////////////////////
// Function name	: ShowHtmlConsole
// Description	    : Show/Hide HTML console window
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::ShowHtmlConsole(bool bShow)
{
    if(bShow == true)
        HtmlWindow->show();		// Show HTML Script window
    else
        HtmlWindow->hide();		// Hide HTML Script window
}

///////////////////////////////////////////////////////////
// Function name	: ShowHtmlPage
// Description	    : Load page into HTML console window
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::ShowHtmlPage(const char *szPage)
{
    QFile hHtmlFile(szPage);
    if(hHtmlFile.exists())
        HtmlWindow->setSource(QUrl::fromLocalFile(szPage));
}

///////////////////////////////////////////////////////////
// Function name	: OnHtmlLink
// Description	    : User hyperlink clicked in script HTML page
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::OnHtmlLink(const QUrl &link)
{
    HtmlWindow->setSource(link);
}

///////////////////////////////////////////////////////////
// Function name	: SetConfigurationField
// Description	    : Sets a GEX environment variable
// Return type		: void
///////////////////////////////////////////////////////////
void GexScripting::SetConfigurationField(char *szSection,char *szField,char *szValue)
{
    // Section		field		value
    // ---------	----------	---------------
    // Global
    //				KeyID		<ActivationKey>
    //				LastPath	<PathOfLastFileAccessed>
    if(!qstricmp(szSection,"Global"))
    {
        // Path to last file selected
        if(!qstricmp(szField,"LastFilePath"))
        {
            strGexConfLastFilePathSelected = szValue;
            return;
        }
        // Date of last Galaxy message box Ad reminder
        if(!qstricmp(szField,"LastReminder"))
        {
            sscanf(szValue,"%ld",&lGexConfLastReminder);
            return;
        }
    }
}

///////////////////////////////////////////////////////////
// GEX Scripting TreeView Items
///////////////////////////////////////////////////////////
GexScriptingTreeItem::GexScriptingTreeItem(QTreeWidget *pTreeWidget,
                                           QStringList strTreeListItem) : QTreeWidgetItem(pTreeWidget,strTreeListItem)
{
}

GexScriptingTreeItem::~GexScriptingTreeItem()
{
}


///////////////////////////////////////////////////////////////////////////////////
// Function name	: operator < (class GexScriptingTreeItem)
// Description		: Override the comparison methods
// Return type		: bool
///////////////////////////////////////////////////////////////////////////////////
bool GexScriptingTreeItem::operator< ( const QTreeWidgetItem & other ) const
{
    int		nCol		= treeWidget() ? treeWidget()->sortColumn() : 0;
    QString strLeft		= text(nCol);
    QString strRight	= other.text(nCol);

    switch(nCol)
    {
    case 0 :
    case 1 :	return ( QString::compare(strLeft,strRight,Qt::CaseInsensitive) < 0);

    default:	return false;
    }
}
