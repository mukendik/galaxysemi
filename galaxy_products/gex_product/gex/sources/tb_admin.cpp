///////////////////////////////////////////////////////////
// Examinator ToolBox: Admin functions
///////////////////////////////////////////////////////////

#include <QFileDialog>
#include <QtGlobal>
#include <QInputDialog>
#include <QToolTip>
#include <QProgressDialog>
#include <QApplication>
#include <QPainter>
#include <QSettings>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QCloseEvent>

#include <math.h>

#include <gqtl_log.h>
#include <gqtl_sysutils.h>

#include "tb_toolbox.h"                             // Examinator ToolBox
#include "browser.h"
#include "browser_dialog.h"
#include "engine.h"
#include "gex_shared.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_constants.h"
#include "gex_pat_constants_extern.h"
#include "db_engine.h"
#include "export_csv.h"                             // Export STDF to CSV file.
#include "export_ascii_dialog.h"
#include "tb_merge_retest_dialog.h"
#include "tb_pat_recipe_wizard_dialog.h"            // Recipe Tests selection wizard
#include "pat_info.h"
#include "patman_lib.h"
#include "pat_global.h"
#include "scripting_io.h"
#include "script_wizard.h"
#include "import_csv.h"
#include "filter_dialog.h"
#include "picktest_dialog.h"
#include "tb_csv_operations_dialog.h"
#include "report_options.h"
#include "temporary_files_manager.h"
#include "product_info.h"
#include "db_keys_editor.h"
#include "regexp_validator.h"
#include "gqtl_datakeys.h"
#include "csl/csl_engine.h"
#include "message.h"
#include "gex_algorithms.h"
#include "pat_recipe_io.h"
#include "pat_widget_ft.h"
#include "pat_limits_dialog.h"
#include "pat_recipe_editor.h"
#include "pat_definition.h"
#include <QInputDialog>

#include "stats_engine.h"
#include "mv_outliers_finder.h"
#include "mv_groups_builder.h"
#include "r_data.h"
#include "r_matrix.h"
#include "pat_mv_rule.h"
#include "pat_recipe_io.h"
#include "pat_recipe.h"
#include "db_onequery_wizard.h"
#include "pat_recipe_historical_data_gui.h"

#ifdef __WIN32
    #undef CopyFile
#endif

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
//#include "DebugMemory.h"

#define TB_IMPORT_CELL_COUNT_LIMIT 10000

// in main.cpp
extern GexMainwindow *	pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern CGexReport *		gexReport;				// Handle to report class

// script_wizard.h
extern void ConvertToScriptString(QString &strFile);

// In patman_lib.cpp
static int		sToolBoxTask, sPrevTask;
static	bool	sPivoted;		//  'true' if table pivoted (cells lines & cols swapped)

///////////////////////////////////////////////////////////
// WIZARD: Convert files to STDF
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_STDF(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Show wizard page : Convert files file.
    sToolBoxTask = GEXTB_CONVERT_STDF_WIZARD_P1;
    ShowWizardDialog(sToolBoxTask);

    // Update the URL edit box
    QString strString = GEX_BROWSER_ACTIONBOOKMARK;
    strString += GEXTB_BROWSER_MAKE_STDF;
    AddNewUrl(strString);

    // If previous call was a different 'Convert' wizard....erase list box.
    if(sPrevTask != GEXTB_CONVERT_STDF_WIZARD_P1)
        pGexTbTools->treeWidget->clear();	// Clear 'Status' window

    sPrevTask = sToolBoxTask;
}

///////////////////////////////////////////////////////////
// WIZARD: DUMP STDF file records into an ASCII file
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_DumpSTDF(void)
{
    bool	bEvaluationMode;

    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
          QString m=ReportOptions.GetOption("messages", "upgrade").toString();
          GS::Gex::Message::information("", m);
            return;
    }

    // Under Evaluation mode: only dump all records until MIR found.
    bEvaluationMode = (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
            ? true : false;

    // DUMP Stdf Dialog box.
    GS::Gex::ExportAsciiDialog cDumpStdf(bEvaluationMode);
    cDumpStdf.exec();
}

///////////////////////////////////////////////////////////
// WIZARD: Database keys editor + RegExp validator
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_DbKeys()
{
    if(!GS::LPPlugin::ProductInfo::getInstance()->isTDRAllowed())
    {
      QString m=ReportOptions.GetOption("messages", "upgrade").toString();
      GS::Gex::Message::information("", m);
        return;
    }

    if (!mDbKeysEditor)
        mDbKeysEditor = new GS::Gex::DbKeysEditor(GS::QtLib::DatakeysContent(),
                                     QStringList(),
                                     GS::Gex::DbKeysEditor::TOOLBOX,
                                     this);

    mDbKeysEditor->show();
}

void GexMainwindow::Wizard_GexTb_RegExp()
{
    if (!mRegExpValidator)
        mRegExpValidator = new GS::Gex::RegExpValidator(QString(),
                                     this);

    mRegExpValidator->show();
}

///////////////////////////////////////////////////////////
// WIZARD: Merge Retest + Test data into one file!
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_MergeRetest(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    if(!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
    {
      QString m=ReportOptions.GetOption("messages", "upgrade").toString();
      GS::Gex::Message::information("", m);
        return;
    }

    // Merge Test + Retest Dialog box.
    GexTbMergeRetestDialog cMergeRetest(this);
    cMergeRetest.exec();
}

///////////////////////////////////////////////////////////
// WIZARD: Process file (PAT filtering on outiers.)
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_PAT(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Check if this optional module is enabled
    if(!(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT()))
    {
      QString m=ReportOptions.GetOption("messages", "upgrade").toString();
      GS::Gex::Message::information("", m);
        //"Your Examinator release doesn't allow this function.\nYou need the PAT-Man module (Outlier Removal).\n\nPlease contact "+QString(GEX_EMAIL_SALES));
        return;
    }

    // Show wizard page : PAT, Process file (Outlier removal).
    ShowWizardDialog(GEX_WS_PAT_PROCESS_WIZARD_P1);

    // Update the URL edit box
    QString strString = GEX_BROWSER_ACTIONBOOKMARK;
    strString += GEXTB_BROWSER_PAT;
    AddNewUrl(strString);
}

void GexMainwindow::Wizard_FT_PAT()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Check if this optional module is enabled
    if(!(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT()))
    {
      QString m=ReportOptions.GetOption("messages", "upgrade").toString();
      GS::Gex::Message::information("", m);
        //"Your Examinator release doesn't allow this function.\nYou need the PAT-Man module (Outlier Removal).\n\nPlease contact "+QString(GEX_EMAIL_SALES));
        return;
    }

    // Show wizard page : PAT, Process file (Outlier removal).
    ShowWizardDialog(GEX_FT_PAT_PROCESS_WIZARD_P1);

    // Update the URL edit box
    QString strString = GEX_BROWSER_ACTIONBOOKMARK;
    strString += GEXTB_BROWSER_FTPAT;
    AddNewUrl(strString);
}

#ifdef GCORE15334

///////////////////////////////////////////////////////////
// WIZARD: Create PAT recipe File
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_FT_PAT_Limits(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION
       || GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in Evaluation mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Check if this optional module is enabled
    if(!(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT()))
    {
        GS::Gex::Message::information(
            "", "Your current license does not allow this function.\n"
            "You need the PAT-Man module (Outlier Removal).\n\n"
            "Please contact " + QString(GEX_EMAIL_SALES));
        return;
    }

    QStringList lFilesSelected;
    GS::Gex::PATRecipeHistoricalDataGui lDataSourceDialog(this);

    lDataSourceDialog.setWindowTitle("Select Historical Data for FT PAT recipe");
    lDataSourceDialog.SetTestingStage("Final Test");
    lDataSourceDialog.SetDataSource(GS::Gex::PATRecipeHistoricalDataGui::DataFromFiles);

    if (lDataSourceDialog.exec() == QDialog::Rejected)
        return;

    // Get historical data files selected
    lFilesSelected = lDataSourceDialog.GetHistoricalData();

    if(lFilesSelected.isEmpty() == TRUE)
    {
        GS::Gex::Message::warning("", QString("No historical data selected"));
        return;	// Empty list...ignore task!
    }

    // Now, see to analyze ALL input files to compute SPAT statistics...
    // Filter data?
    QMessageBox mb(
      GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
      "Read all the data files or filter samples?\nYou can ignore/filter some parts or bins if needed...\n\nE.g.: Use GOOD parts only.",
    QMessageBox::Question,
    QMessageBox::Yes | QMessageBox::Escape | QMessageBox::Default,
    QMessageBox::No,
    QMessageBox::NoButton,this);
    mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    mb.setButtonText( QMessageBox::Yes, "Read ALL" );
    mb.setButtonText( QMessageBox::No, "Filter data..." );

    // User want to select a filter...
    QString strFilterType="all";
    QString	strFilterList="";
    if (mb.exec() == QMessageBox::No)
    {
        // Show selection made...
        FilterDialog *dFilter = new FilterDialog();
        dFilter->SetParameters(0,false,0,NULL,GEX_FILE_FILTER_NOSITE | GEX_FILE_FILTER_NOCONTROL |
                               GEX_FILE_FILTER_NOGROUPNAME | GEX_FILE_FILTER_NOADVANCEDTAB);
        dFilter->adjustSize();

        // Prompt Filter dialog box
        if(dFilter->exec() == 1)
        {
            // Extract filter info
            strFilterType = gexFileProcessPartsItems[dFilter->comboProcess->currentIndex()];
            strFilterList = dFilter->editProcess->text();
        }
    }

    // Switch to the Excel-type sheet page (create it if needed)
    Wizard_GexTb_EditPAT_Limits();

    // Create Recipe window
    ShowWizardDialog(GEX_FT_PAT_PROCESS_WIZARD_P1);	// Ensures PatWizard window is created
    ShowWizardDialog(GEX_BROWSER);

    // instantiate global CPatInfo if not already done
    GS::Gex::PATRecipe lPatRecipe;

    // Reset recipe to default value for this testing stage
    lPatRecipe.Reset(GS::Gex::PAT::RecipeFinalTest);

    QString strErrorMessage;
    QString lRecipeName;

    lRecipeName += GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + QDir::separator();
    lRecipeName += GS::Gex::PAT::GetDefaultRecipeName(GS::Gex::PAT::RecipeFinalTest);

    if (QFile::exists(lRecipeName))
    {
        QSharedPointer<GS::Gex::PATRecipeIO> lRecipeIO(GS::Gex::PATRecipeIO::CreateRecipeIo(lRecipeName));

        if (lRecipeIO.isNull())
        {
            strErrorMessage = "Unable to instanciate Recipe reader";
            GSLOG(SYSLOG_SEV_ERROR, strErrorMessage.toLatin1().constData());
            return;
        }

        lRecipeIO->SetRecipeName(lRecipeName);

        if (lRecipeIO->Read(lPatRecipe) == false)
        {
            bool lOk;
            GS::Gex::Message::request("", QString("Unable to read your default recipe %1\n"
                                                  "Do you want to continue with the default recipe settings ?")
                                          .arg(lRecipeName), lOk);
            if (! lOk)
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("Unable to read recipe %1: %2").arg(lRecipeName)
                      .arg(lRecipeIO->GetErrorMessage()).toLatin1().constData());
                return;
            }
        }
        else
        {
            if (lPatRecipe.GetOptions().GetRecipeType() != GS::Gex::PAT::RecipeFinalTest)
            {
                GS::Gex::Message::warning("",
                                          QString("Default recipe file %1 is not a Final Test recipe")
                                          .arg(lRecipeName));
                return;
            }
        }
    }

    // Set Test key based on user choice
    switch(lDataSourceDialog.GetTestKey())
    {
        case GS::Gex::PATRecipeHistoricalDataGui::TestNumber:
            lPatRecipe.GetOptions().mOptionsTestKey = GEX_TBPAT_KEY_TESTNUMBER;
            lPatRecipe.GetOptions().mTestKey        = GEX_TBPAT_KEY_TESTNUMBER;
            break;

        case GS::Gex::PATRecipeHistoricalDataGui::TestName:
            lPatRecipe.GetOptions().mOptionsTestKey = GEX_TBPAT_KEY_TESTNAME;
            lPatRecipe.GetOptions().mTestKey        = GEX_TBPAT_KEY_TESTNAME;
            break;

        case GS::Gex::PATRecipeHistoricalDataGui::TestMix:
            lPatRecipe.GetOptions().mOptionsTestKey = GEX_TBPAT_KEY_TESTMIX;
            lPatRecipe.GetOptions().mTestKey        = GEX_TBPAT_KEY_TESTMIX;
            break;

        default:
            GS::Gex::Message::warning("", QString("Unsuppored test key selected"));
            return;	// Empty list...ignore task!
    }

    /////////////////////////////////////////////////////////////////
    // Recipe Wizard for excluding some tests (Pre-PAT checker)
    // Reads first file selected to run pre-PAT checker over it.
    /////////////////////////////////////////////////////////////////

    PatRecipeWizardDialog cRecipeWizard(this);
    QString strFileName = *lFilesSelected.begin();

    // Use first of Historical data files selected.
    if (cRecipeWizard.setFile(strFileName, lPatRecipe.GetOptions(), strFilterType, strFilterList) == false)
        return;

    // Create script that will read data file + compute all statistics (but NO report created)
    FILE *hFile = fopen(GS::Gex::Engine::GetInstance().GetAssistantScript().toLatin1().constData(),"w");

    if(hFile == NULL)
    {
        strErrorMessage = "  > Failed to create script file: " + GS::Gex::Engine::GetInstance().GetAssistantScript();
        GS::Gex::Message::information("", strErrorMessage);
        return;
    }

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        GSLOG(3, "Failed to write option section to file");
        strErrorMessage = QString("error : can't write option section");
        return;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  var group_id;\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");
    fprintf(hFile,"  gexQuery('db_report','DataSet_1');\n");
    fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");

    // Write all files to merge in analysis
    QStringList::Iterator it;
    for (it = lFilesSelected.begin(); it != lFilesSelected.end(); ++it )
    {
        // Get file name
        strFileName = *it;
        ConvertToScriptString(strFileName);	// Make sure any '\' in string is doubled.

        // Write script line
        fprintf(hFile,"  gexFile(group_id,'insert','%s','All','%s',' %s','','','','');\n",
                strFileName.toLatin1().constData(),
                strFilterType.toLatin1().constData(),
                strFilterList.toLatin1().constData()
                );
    }
    fprintf(hFile,"}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n");

    // ToDo : write csl version 2
    // fprintf(hFile,"  gexCslVersion('2.0');\n");
    // Ensure continue on fail
    fprintf(hFile,"  gexOptions('dataprocessing','fail_count','all');\n");

    // Default: keep test values as is (no scaling)
    fprintf(hFile,"  gexOptions('dataprocessing', 'scaling', 'normalized');\n");
    fprintf(hFile,"  gexOptions('output','format','interactive');\n");              // Avoids editor to be launched if output options is set to Word or Excel!
    fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");                 // Always compute Quartiles & Skew & Kurtosis.
    fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");	// Force to compute statistics from samples, ignore summary.
    fprintf(hFile,"  gexOptions('dataprocessing','data_cleaning_mode','none');\n");   // Disable outlier removal so to make sure all values are taken into account.
    fprintf(hFile,"  gexOptions('dataprocessing','multi_parametric_merge_mode','no_merge');\n"); // MPR split always forced at Final Test level

    // Choose test merge option based on recipe test key option
    switch (lPatRecipe.GetOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            // Merge tests with same test number (even if test name is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            // Merge tests with same test name (even if test number is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge_name');\n");
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            // Merge tests with same test number and test name
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'no_merge');\n");
            break;

        default:
            // Use default option from options tab
            break;
    }

    fprintf(hFile,"  SetProcessData();\n");
    fprintf(hFile,"  gexOptions('report','build','false');\n");	// Only data analysis, no report created!
    fprintf(hFile,"  gexBuildReport('none','0');\n");		// Build report.
    fprintf(hFile,"}\n\n");
    fclose(hFile);

    // Execute script.
    if (GS::Gex::CSLEngine::GetInstance().RunScript(GS::Gex::Engine::GetInstance().GetAssistantScript()).IsFailed())
        return;

    // Switch to the Excel-type sheet page (create it if needed)
    Wizard_GexTb_EditPAT_Limits();

    // 'save mode' to QUIET: NO save config file to disk when 'Ok' button will be pressed (done at the end of this function!)
    // Clear Version#
    lPatRecipe.GetOptions().iRecipeVersion = DEFAULT_RECIPE_VERSION;
    lPatRecipe.GetOptions().iRecipeBuild = DEFAULT_RECIPE_BUILD;
    GS::Gex::PatLimitsDialog *patConfigGlobals  = new GS::Gex::PatLimitsDialog(&lPatRecipe.GetOptions(), this);
    patConfigGlobals->setEditMode(false,true,true);

    // Display dialog box only if valid data available, ortherwise a error message is shown
    if(patConfigGlobals->isValidData() == false)
        return;

    wizard_page1:
    // Wizard page1: Display Recipe global options.
    if(patConfigGlobals->exec() != 1)
        return;	// User abort

    // Check if we have parametric tests
    int iExitCode;
    if(cRecipeWizard.hasParametricTests() == false)
    {
        GS::Gex::Message::information(
            "", "Your file doesn't hold any parametric test,\n"
            "therefore ONLY geographic PAT rules will have effect.");
        iExitCode = 0;	// Signals to ignore tests selection!
    }
    else
    {
        // Launch Wizard GUI to see which tests are suggested to REJECT
        iExitCode = cRecipeWizard.exec();
    }

    // If 'Back' button clicked, return too previous page!
    if(iExitCode == -2)
        goto wizard_page1;	// 'back' button pressed

    // Set univariates in the recipe
    if (FillUniVariateRules(cRecipeWizard.getEnabledTests(), lPatRecipe) == false)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "FT PAT Recipe creation was aborted by the user");
        ShowWizardDialog(GEX_BROWSER);
        return;
    }

    // Build the recipe name
    QString fileName = patConfigGlobals->BuildRecipeFileName();

    // case 7887: Zied: you had to check that the user has not cancelled
    if (fileName.isEmpty())
    {
        ShowWizardDialog(GEX_BROWSER);
        return;
    }

    GS::Gex::PATRecipeIO* patRecipeIO = GS::Gex::PATRecipeIO::CreateRecipeIo(GS::Gex::PATRecipeIO::JSON);

    if (patRecipeIO)
    {
        patRecipeIO->Write(lPatRecipe, fileName);
        delete patRecipeIO;
        patRecipeIO = NULL;
    }
    else
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Failed to write the recipe %1")
              .arg(fileName).toLatin1().constData());

    // Re-Load Config file in the Excel-type table & internal structures
    GS::Gex::PATRecipeEditor::GetInstance().RefreshEditor(fileName);
}


///////////////////////////////////////////////////////////
// WIZARD: Create PAT recipe File
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_WS_PAT_Limits(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION
       || GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode or OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in Evaluation mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Check if this optional module is enabled
    if(!(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT()))
    {
        GS::Gex::Message::information(
            "", "Your current license does not allow this function.\n"
            "You need the PAT-Man module (Outlier Removal).\n\n"
            "Please contact " + QString(GEX_EMAIL_SALES));
        return;
    }

    QStringList lFilesSelected;
    GS::Gex::PATRecipeHistoricalDataGui lDataSourceDialog(this);

    lDataSourceDialog.setWindowTitle("Select Historical Data for WS PAT recipe");
    lDataSourceDialog.SetTestingStage("Wafer Sort");
    lDataSourceDialog.SetDataSource(GS::Gex::PATRecipeHistoricalDataGui::DataFromFiles);

    if (lDataSourceDialog.exec() == QDialog::Rejected)
        return;

    // Get historical data files selected
    lFilesSelected = lDataSourceDialog.GetHistoricalData();

    if(lFilesSelected.isEmpty() == TRUE)
    {
        GS::Gex::Message::warning("", QString("No historical data selected"));
        return;	// Empty list...ignore task!
    }

    // Now, see to analyze ALL input files to compute SPAT statistics...
    // Filter data?
    QMessageBox mb(
      GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
      "Read all the data files or filter samples?\nYou can ignore/filter some parts or bins if needed...\n\nE.g.: Use GOOD parts only.",
    QMessageBox::Question,
    QMessageBox::Yes | QMessageBox::Escape | QMessageBox::Default,
    QMessageBox::No,
    QMessageBox::NoButton,this);
    mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    mb.setButtonText( QMessageBox::Yes, "Read ALL" );
    mb.setButtonText( QMessageBox::No, "Filter data..." );

    // User want to select a filter...
    QString strFilterType="all";
    QString	strFilterList="";
    if (mb.exec() == QMessageBox::No)
    {
        // Show selection made...
        FilterDialog *dFilter = new FilterDialog();
        dFilter->SetParameters(0,false,0,NULL,GEX_FILE_FILTER_NOSITE | GEX_FILE_FILTER_NOCONTROL |
                               GEX_FILE_FILTER_NOGROUPNAME | GEX_FILE_FILTER_NOADVANCEDTAB);
        dFilter->adjustSize();

        // Prompt Filter dialog box
        if(dFilter->exec() == 1)
        {
            // Extract filer info
            strFilterType = gexFileProcessPartsItems[dFilter->comboProcess->currentIndex()];
            strFilterList = dFilter->editProcess->text();
        }
    }

    // Switch to the Excel-type sheet page (create it if needed)
    Wizard_GexTb_EditPAT_Limits();

    // Create Recipe window
    ShowWizardDialog(GEX_WS_PAT_PROCESS_WIZARD_P1);	// Ensures PatWizard window is created
    ShowWizardDialog(GEX_BROWSER);

    GS::Gex::PATRecipe patRecipe;
    // Force recipe type to final test
    patRecipe.Reset(GS::Gex::PAT::RecipeWaferSort);

    QString strErrorMessage;
    QString lRecipeName;

    lRecipeName += GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + QDir::separator();
    lRecipeName += GS::Gex::PAT::GetDefaultRecipeName(GS::Gex::PAT::RecipeWaferSort);

    if (QFile::exists(lRecipeName))
    {
        QSharedPointer<GS::Gex::PATRecipeIO> lRecipeIO(GS::Gex::PATRecipeIO::CreateRecipeIo(lRecipeName));

        if (lRecipeIO.isNull())
        {
            strErrorMessage = "Unable to instanciate Recipe reader";
            GSLOG(SYSLOG_SEV_ERROR, strErrorMessage.toLatin1().constData());
            return;
        }

        lRecipeIO->SetRecipeName(lRecipeName);

        if (lRecipeIO->Read(patRecipe) == false)
        {
            bool lOk;
            GS::Gex::Message::request("", QString("Unable to read your default recipe %1\n"
                                                  "Do you want to continue with the default recipe settings ?")
                                          .arg(lRecipeName), lOk);
            if (! lOk)
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("Unable to read recipe %1: %2").arg(lRecipeName)
                      .arg(lRecipeIO->GetErrorMessage()).toLatin1().constData());

                return;
            }
        }
        else
        {
            if (patRecipe.GetOptions().GetRecipeType() != GS::Gex::PAT::RecipeWaferSort)
            {
                GS::Gex::Message::warning("",
                                          QString("Default recipe file %1 is not a Final Test recipe")
                                          .arg(lRecipeName));
                return;
            }
        }
    }

    // Set Test key based on user choice
    switch(lDataSourceDialog.GetTestKey())
    {
        case GS::Gex::PATRecipeHistoricalDataGui::TestNumber:
            patRecipe.GetOptions().mOptionsTestKey = GEX_TBPAT_KEY_TESTNUMBER;
            patRecipe.GetOptions().mTestKey        = GEX_TBPAT_KEY_TESTNUMBER;
            break;

        case GS::Gex::PATRecipeHistoricalDataGui::TestName:
            patRecipe.GetOptions().mOptionsTestKey = GEX_TBPAT_KEY_TESTNAME;
            patRecipe.GetOptions().mTestKey        = GEX_TBPAT_KEY_TESTNAME;
            break;

        case GS::Gex::PATRecipeHistoricalDataGui::TestMix:
            patRecipe.GetOptions().mOptionsTestKey = GEX_TBPAT_KEY_TESTMIX;
            patRecipe.GetOptions().mTestKey        = GEX_TBPAT_KEY_TESTMIX;
            break;

        default:
            GS::Gex::Message::warning("", QString("Unsuppored test key selected"));
            return;	// Empty list...ignore task!
    }

    /////////////////////////////////////////////////////////////////
    // Recipe Wizard for excluding some tests (Pre-PAT checker)
    // Reads first file selected to run pre-PAT checker over it.
    /////////////////////////////////////////////////////////////////

    PatRecipeWizardDialog cRecipeWizard(this);
    QString strFileName = *lFilesSelected.begin();

    // Use first of Historical data files selected.
    if (cRecipeWizard.setFile(strFileName, patRecipe.GetOptions(), strFilterType, strFilterList) == false)
        return;

    QStringList::Iterator it;
    //pWizardProcessPAT->checkBoxMergeFiles->setChecked(false);
    for (it = lFilesSelected.begin(); it != lFilesSelected.end(); ++it )
    {
        // Add Data file name to list of files to process
        mWizardWSPAT->AddFile(*it,(it == lFilesSelected.begin()));
    }

    // Create script that will read data file + compute all statistics (but NO report created)
    FILE *hFile = fopen(GS::Gex::Engine::GetInstance().GetAssistantScript().toLatin1().constData(),"w");

    if(hFile == NULL)
    {
        strErrorMessage = "  > Failed to create script file: " + GS::Gex::Engine::GetInstance().GetAssistantScript();
        GS::Gex::Message::information("", strErrorMessage);
        return;
    }

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        GSLOG(3, "Failed to write option section to file");
        strErrorMessage = QString("error : can't write option section");
        return;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  var group_id;\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");
    fprintf(hFile,"  gexQuery('db_report','DataSet_1');\n");
    fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");

    // Write all files to merge in analysis
    for (it = lFilesSelected.begin(); it != lFilesSelected.end(); ++it )
    {
        // Get file name
        strFileName = *it;
        ConvertToScriptString(strFileName);	// Make sure any '\' in string is doubled.

        // Write script line
        fprintf(hFile,"  gexFile(group_id,'insert','%s','All','%s',' %s','','','','');\n",
                strFileName.toLatin1().constData(),
                strFilterType.toLatin1().constData(),
                strFilterList.toLatin1().constData()
                );
    }
    fprintf(hFile,"}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n");

    // ToDo : write csl version 2
    // fprintf(hFile,"  gexCslVersion('2.0');\n");
    // Ensure continue on fail
    fprintf(hFile,"  gexOptions('dataprocessing','fail_count','all');\n");

    fprintf(hFile,"  gexOptions('output','format','interactive');\n");              // Avoids editor to be launched if output options is set to Word or Excel!
    fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");                 // Always compute Quartiles & Skew & Kurtosis.
    fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");	// Force to compute statistics from samples, ignore summary.
    fprintf(hFile,"  gexOptions('dataprocessing','data_cleaning_mode','none');\n");   // Disable outlier removal so to make sure all values are taken into account.

    // Choose test merge option based on recipe test key option
    switch (patRecipe.GetOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            // Merge tests with same test number (even if test name is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            // Merge tests with same test name (even if test number is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge_name');\n");
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            // Merge tests with same test number and test name
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'no_merge');\n");
            break;

        default:
            // Use default option from options tab
            break;
    }

    fprintf(hFile,"  SetProcessData();\n");
    fprintf(hFile,"  gexOptions('report','build','false');\n");	// Only data analysis, no report created!
    fprintf(hFile,"  gexBuildReport('none','0');\n");		// Build report.
    fprintf(hFile,"}\n\n");
    fclose(hFile);

    // Execute script.
    if (GS::Gex::CSLEngine::GetInstance().RunScript(GS::Gex::Engine::GetInstance().GetAssistantScript()).IsFailed())
        return;

    // Switch to the Excel-type sheet page (create it if needed)
    Wizard_GexTb_EditPAT_Limits();

    // 'save mode' to QUIET: NO save config file to disk when 'Ok' button will be pressed (done at the end of this function!)
    // Clear Version#
    patRecipe.GetOptions().iRecipeVersion = DEFAULT_RECIPE_VERSION;
    patRecipe.GetOptions().iRecipeBuild = DEFAULT_RECIPE_BUILD;

    GS::Gex::PatLimitsDialog *patConfigGlobals  = new GS::Gex::PatLimitsDialog(&patRecipe.GetOptions(), this);
    patConfigGlobals->setEditMode(false,true,true);

    // Display dialog box only if valid data available, ortherwise a error message is shown
    if(patConfigGlobals->isValidData() == false)
        return;

wizard_page1:
    // Wizard page1: Display Recipe global options.
    if(patConfigGlobals->exec() != 1)
        return;	// User abort

    // Check if we have parametric tests
    int iExitCode;
    if(cRecipeWizard.hasParametricTests() == false)
    {
        GS::Gex::Message::information(
            "", "Your file doesn't hold any parametric test,\n"
            "therefore ONLY geographic PAT rules will have effect.");
        iExitCode = 0;	// Signals to ignore tests selection!
    }
    else
    {
        // Launch Wizard GUI to see which tests are suggested to REJECT
        iExitCode = cRecipeWizard.exec();
    }

    // If 'Back' button clicked, return to previous page!
    if(iExitCode == -2)
        goto wizard_page1;	// 'back' button pressed

    // Set univariates in the recipe
    if (FillUniVariateRules(cRecipeWizard.getEnabledTests(), patRecipe) == false)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "WS PAT Recipe creation was aborted by the user");
        ShowWizardDialog(GEX_BROWSER);
        return;
    }

    // Calculate the multivariate' groups
    if(patRecipe.GetOptions().GetMVPATEnabled() && patRecipe.GetOptions().GetMVPATAutomaticGroupCreation())
    {
        QString lAppDir = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        QString lError;
        GS::SE::StatsEngine * lStatsEngine = GS::SE::StatsEngine::GetInstance(lAppDir, lError);
        if (lStatsEngine)
        {
            ComputeMVPATGroups(cRecipeWizard.getEnabledTests(), patRecipe, lStatsEngine);
            GS::SE::StatsEngine::ReleaseInstance();
        }
        else
        {
            GSLOG(SYSLOG_SEV_CRITICAL, QString("Unable to instantiate StatsEngine: %1. Application will now exit.")
                  .arg(lError).toLatin1().data());
            exit(EXIT_FAILURE);
        }
    }

    QString fileName = patConfigGlobals->BuildRecipeFileName();

    if (fileName.isEmpty())
    {
        ShowWizardDialog(GEX_BROWSER);
        return;
    }

    GS::Gex::PATRecipeIO* patRecipeIO = GS::Gex::PATRecipeIO::CreateRecipeIo(GS::Gex::PATRecipeIO::JSON);

    if (patRecipeIO)
    {
        patRecipeIO->Write(patRecipe, fileName);
        delete patRecipeIO;
        patRecipeIO = NULL;
    }
    else
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Failed to write the recipe %1")
              .arg(fileName).toLatin1().constData());

    // Re-Load Config file in the Excel-type table & internal structures
    GS::Gex::PATRecipeEditor::GetInstance().RefreshEditor(fileName);
}
#endif

bool GexMainwindow::FillUniVariateRules(const QList<TestInfo>& lTests, GS::Gex::PATRecipe& patRecipe)
{
    CGexGroupOfFiles *              lGroup  = NULL;
    QHash<QString, CPatDefinition*> lUnivariate;
    QList<CTest *>                  lZeroExecutionTest;

    if (gexReport->getGroupsList().count() > 0)
    {
        lGroup  = gexReport->getGroupsList().first();

        CTest *             lTest   = NULL;
        CPatDefinition *    lPatdef = NULL;
        QString             lTestKey;

        QString lMPRMergeMode = ReportOptions.GetOption("dataprocessing","multi_parametric_merge_mode").toString();
        for (int lIdx = 0; lIdx < lTests.count(); ++lIdx)
        {

            int lPinMergeIndex = lTests[lIdx].mPinIndex;
            //-- in merge mode we need to force the pinIndex to -1 because all the CTest have -1 as a pinIndex
            //-- This is to manage the TestInfo with pinIndex -2 that won't find the related CTest if the search is based
            //-- on there pinIndex.
            if (lMPRMergeMode == "merge")
            {
                lPinMergeIndex = -1;
            }

            lTest = lGroup->FindTestCell(lTests[lIdx].mNumber, lTests[lIdx].mName, lPinMergeIndex);

            if (!lTest)
                continue;

            if (lTest->lPinmapIndex == GEX_MPTEST)
            {
                CTest * lMPTest = NULL;

                // Create one Univariate rule per pin
                for (int lPin = 0; lPin < lTest->getPinCount(); ++lPin)
                {
                    lMPTest = lGroup->FindTestCell(lTest->lTestNumber, lTest->strTestName, lPin);

                    // For each Parametric test, create entry in PAT file...
                    // If not a parametric / multiparametric (eg: functional) test,
                    // Or no data for this test: ignore!
                    if (lMPTest == NULL || lMPTest->bTestType == '-' || lMPTest->bTestType == 'F')
                        continue;

                    // If no data for this test: ignore it and inform the customer!
                    if (lMPTest->m_testResult.count() == 0 || lMPTest->ldSamplesValidExecs <= 0)
                        lZeroExecutionTest.append(lMPTest);
                    else
                    {
                        switch(patRecipe.GetOptions().mTestKey)
                        {
                            case GEX_TBPAT_KEY_TESTNUMBER:
                                lTestKey = QString::number(lMPTest->lTestNumber);
                                if(lMPTest->lPinmapIndex >= 0)
                                    lTestKey += "." + QString::number(lMPTest->lPinmapIndex);
                                break;
                            case GEX_TBPAT_KEY_TESTNAME:
                                lTestKey = lMPTest->strTestName;
                                if(lMPTest->lPinmapIndex >= 0)
                                    lTestKey += "." + QString::number(lMPTest->lPinmapIndex);
                                break;
                            case GEX_TBPAT_KEY_TESTMIX:
                                lTestKey = lMPTest->strTestName.trimmed();
                                lTestKey += "." + QString::number(lMPTest->lTestNumber);
                                if(lMPTest->lPinmapIndex >= 0)
                                    lTestKey += "." + QString::number(lMPTest->lPinmapIndex);
                                break;
                            default:
                                lTestKey = QString::number(lMPTest->lTestNumber);
                        }

                        if (lUnivariate.contains(lTestKey) == false)
                        {
                            lPatdef = CreatePATDefinition(patRecipe.GetOptions(), *lMPTest);

                            if (lPatdef)
                                lUnivariate.insert(lTestKey, lPatdef);
                        }
                    }
                }
            }
            else
            {
                // For each Parametric test, create entry in PAT file...
                // If not a parametric / multiparametric (eg: functional) test, ignore it
                if (lTest->bTestType == '-' || lTest->bTestType == 'F')
                    continue;

                // If no data for this test: ignore it and inform the customer!
                if (lTest->m_testResult.count() == 0 || lTest->ldSamplesValidExecs <= 0)
                    lZeroExecutionTest.append(lTest);
                else
                {
                    switch(patRecipe.GetOptions().mTestKey)
                    {
                        case GEX_TBPAT_KEY_TESTNUMBER:
                            lTestKey = QString::number(lTest->lTestNumber);
                            if(lTest->lPinmapIndex >= 0)
                                lTestKey += "." + QString::number(lTest->lPinmapIndex);
                            break;
                        case GEX_TBPAT_KEY_TESTNAME:
                            lTestKey = lTest->strTestName;
                            if(lTest->lPinmapIndex >= 0)
                                lTestKey += "." + QString::number(lTest->lPinmapIndex);
                            break;
                        case GEX_TBPAT_KEY_TESTMIX:
                            lTestKey = lTest->strTestName.trimmed();
                            lTestKey += "." + QString::number(lTest->lTestNumber);
                            if(lTest->lPinmapIndex >= 0)
                                lTestKey += "." + QString::number(lTest->lPinmapIndex);
                            break;
                        default:
                            lTestKey = QString::number(lTest->lTestNumber);
                    }

                    if (lUnivariate.contains(lTestKey) == false)
                    {
                        lPatdef = CreatePATDefinition(patRecipe.GetOptions(), *lTest);

                        if (lPatdef)
                            lUnivariate.insert(lTestKey, lPatdef);
                    }
                }
            }
        }
    }

    // Warn the customer that some tests have 0 execution and won't be added to the recipe
    if (lZeroExecutionTest.isEmpty() == false)
    {
        QMessageBox lWarningBox(this);

        lWarningBox.setWindowTitle(GS::Gex::Engine::GetInstance().Get("AppFullName").toString());
        lWarningBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
        lWarningBox.setText("Some tests have 0 execution and won't be added to the recipe.");

        QString lTestList;

        // Prepare the detailed list of test with 0 execution
        for (int lIdx = 0; lIdx < lZeroExecutionTest.count(); ++lIdx)
        {
            CTest * lTest = lZeroExecutionTest.at(lIdx);

            if (lTestList.isEmpty() == false)
                lTestList += "\n";

            lTestList += QString::number(lTest->lTestNumber);

            if (lTest->lPinmapIndex == GEX_MPTEST)
            {
                lTestList += "." + QString::number(lTest->lPinmapIndex);
            }

            lTestList += " - " + lTest->strTestName;
        }

        lWarningBox.setInformativeText("Do you want to continue?");
        lWarningBox.setDetailedText(lTestList);
        lWarningBox.setIcon(QMessageBox::Warning);
        lWarningBox.setStandardButtons(QMessageBox::Ok|QMessageBox::Abort);
        lWarningBox.setDefaultButton(QMessageBox::Ok);

        if (lWarningBox.exec() == QMessageBox::Abort)
            return false;
    }

    patRecipe.SetUniVariateRules(lUnivariate);

    return true;
}

CPatDefinition *GexMainwindow::CreatePATDefinition(const COptionsPat &lOptions, CTest &lTest)
{
    CPatDefinition * lPatDefinition = new CPatDefinition();

    if (lPatDefinition)
    {
        // Fail Static Bin.
        if(lOptions.bStaticPAT)
            lPatDefinition->m_lFailStaticBin = lOptions.iFailStatic_SBin;
        else
            lPatDefinition->m_lFailStaticBin = -1;

        // Fail Dynamic Bin.
        if(lOptions.bDynamicPAT)
            lPatDefinition->m_lFailDynamicBin = lOptions.iFailDynamic_SBin;
        else
            lPatDefinition->m_lFailDynamicBin = -1;

        // Test number
        lPatDefinition->m_lTestNumber = lTest.lTestNumber;
        // Pin index
        lPatDefinition->mPinIndex = lTest.lPinmapIndex;
        // Test name
        lPatDefinition->m_strTestName = lTest.strTestName;
        // Test Type
        lPatDefinition->SetTestTypeLegacy(lTest.bTestType);

        // Low limit
        if((lTest.GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
        {
            lPatDefinition->m_lfLowLimit = lTest.GetCurrentLimitItem()->lfLowLimit;
            lPatDefinition->m_strUnits = lTest.szTestUnits;
        }
        else
            lPatDefinition->m_strUnits = "";

        // High limit
        if((lTest.GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
        {
            lPatDefinition->m_lfHighLimit = lTest.GetCurrentLimitItem()->lfHighLimit;
            lPatDefinition->m_strUnits = lTest.szTestUnits;
        }
        else
            lPatDefinition->m_strUnits = "";

        // Distribution Shape
        lPatDefinition->m_iDistributionShape = patlib_GetDistributionType(&lTest,
                                                                          lOptions.iCategoryValueCount,
                                                                          lOptions.bAssumeIntegerCategory,
                                                                          lOptions.mMinConfThreshold);

        // Historical Cpk
        lPatDefinition->m_lHistoricalCPK = lTest.GetCurrentLimitItem()->lfCpk;

        // Median
        lPatDefinition->m_lfMedian =  lTest.lfSamplesQuartile2;

        // Robust Sigma: IRQ/1.35
        lPatDefinition->m_lfRobustSigma = (lTest.lfSamplesQuartile3 - lTest.lfSamplesQuartile1)/1.35;

        // Mean
        lPatDefinition->m_lfMean = lTest.lfMean;

        // Sigma
        lPatDefinition->m_lfSigma = lTest.lfSigma;

        // Range
        lPatDefinition->m_lfRange = lTest.lfRange;

        // Samples to  Ignore: "none", "Negative values" or "Positive values". Default is "none" when creating the data file.
        lPatDefinition->m_SamplesToIgnore   = GEX_TPAT_IGNOREDATA_NONEID;

        // Outliers to  Keep: "none", "low values" or "high values". Default is "none" when creating the data file.
        lPatDefinition->m_OutliersToKeep    = GEX_TPAT_KEEPTYPE_NONEID;

        // 'Outlier Limits set': tells if outliers to remove are 'near and higher', 'medium and higher' or 'far'
        lPatDefinition->m_iOutlierLimitsSet = GEX_TPAT_LIMITSSET_NEAR;

        // Outlier Ruleset: %limit == Percentage of limit space, PAT == AEC-Q001 & JEDEC JESD62 PAT algorithm (default), Smart = Smart outlier detection
        COptionsPat::GexDefaultRuleSet lRuleID = lOptions.GetDefaultDynamicRule();
        switch(lRuleID)
        {
            case COptionsPat::n_sigma:			// Rule set: N*Sigma
            case COptionsPat::n_robust_sigma:	// Rule set: N*RobustSigma
            case COptionsPat::q1_q3_idq:			// Rule set: Q1-N*IQR, Q3+N*Sigma
            case COptionsPat::percent_of_limits:		// Rule set: % of Limits
                lPatDefinition->mOutlierRule   = (int)lRuleID;
                lPatDefinition->m_lfOutlierNFactor = lOptions.GetDefaultFactor();
                break;

            case COptionsPat::new_limits:		// Rule set: Force custom Limits
            case COptionsPat::range:			// Rule set: Range
            case COptionsPat::smart_adaptive:		// Rule set: Smart & adaptive
            case COptionsPat::disabled:	// Rule set: Disabled
                lPatDefinition->mOutlierRule = (int)lRuleID;	// Leave N & T factors fields empty.
                break;

            default:
                lPatDefinition->mOutlierRule = (int)COptionsPat::smart_adaptive;	// Leave N & T factors fields empty.
        }

        // Low limit scale factor
        lPatDefinition->m_llm_scal = lTest.llm_scal;

        // High limit scale factor
        lPatDefinition->m_hlm_scal = lTest.hlm_scal;

        // SPAT Rule: AEC, other...
        lPatDefinition->m_SPATRuleType = GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA;

        lPatDefinition->m_lfSpatOutlierNFactor = 6;	// Default Head RobustSigma: 6
        lPatDefinition->m_lfSpatOutlierTFactor = 6;	// Default Tail RobustSigma: 6

        // NNR Rule
        lPatDefinition->m_iNrrRule = (lOptions.IsNNREnabled()) ? GEX_TPAT_NNR_ENABLED : GEX_TPAT_NNR_DISABLED;
    }

    return lPatDefinition;
}

bool GexMainwindow::ComputeMVPATGroups(const QList<TestInfo>& lTests,
                                       GS::Gex::PATRecipe& lPATRecipe,
                                       GS::SE::StatsEngine * statsEngine)
{
    if (!statsEngine)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Stats Engine NULL");
        return false;
    }

    QMap<GS::SE::StatsAlgo::Parameter,QVariant> lParam;
    lParam.insert(GS::SE::StatsAlgo::CORR_THRESHOLD, QVariant(lPATRecipe.GetOptions().GetMVPATGroupCorrelation()));
    GS::SE::MVGroupsBuilder * lAlgo = static_cast<GS::SE::MVGroupsBuilder*>(
                statsEngine->GetAlgorithm(GS::SE::StatsAlgo::MV_GROUPS_BUILDER));
    if (lAlgo == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Groups builder algorithm not loaded");
        return false;
    }

    // Get pointer to first group & first file (we always have them exist)
    QListIterator<CGexGroupOfFiles*>    itGroupsList(gexReport->getGroupsList());
    CGexGroupOfFiles *                  lGroup      = NULL;
    CTest *                             lTestCell   = NULL;
    QList<CTest*>                       lTestList;
    int                                 lCol        = 0;
    int                                 lRow        = 0;
    int                                 lRowCount   = 0;

    // Count the number of runs
    itGroupsList.toFront();
    while(itGroupsList.hasNext())
    {
        lGroup = itGroupsList.next();

        if(lGroup)
            lTestCell = lGroup->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN, "", GEX_PTEST);
        else
            return false;

        if (lTestCell == NULL)
            return false;

        lRowCount += lTestCell->m_testResult.count();
    }

    // Full Test list
    for (int lIdx = 0; lIdx < lTests.count(); ++lIdx)
    {
        lTestCell = lGroup->FindTestCell(lTests[lIdx].mNumber, lTests[lIdx].mName, lTests[lIdx].mPinIndex);

        if (!lTestCell)
            continue;

        if (lTestCell->lPinmapIndex == GEX_MPTEST)
        {
            CTest * lMPTest = NULL;

            // Create one Univariate rule per pin
            for (int lPin = 0; lPin < lTestCell->getPinCount(); ++lPin)
            {
                lMPTest = lGroup->FindTestCell(lTestCell->lTestNumber, lTestCell->strTestName, lPin);

                if (lMPTest == NULL)
                    continue;

                if ((lPATRecipe.GetOptions().GetMVPATNormalShapeOnly() == false) ||
                        (patlib_GetDistributionType(lMPTest, lPATRecipe.GetOptions().iCategoryValueCount,
                                                    lPATRecipe.GetOptions().bAssumeIntegerCategory,
                                                    lPATRecipe.GetOptions().mMinConfThreshold) == PATMAN_LIB_SHAPE_GAUSSIAN))
                {
                    lTestList.append(lMPTest);
                }
            }
        }
        else if ((lPATRecipe.GetOptions().GetMVPATNormalShapeOnly() == false) ||
                 (patlib_GetDistributionType(lTestCell, lPATRecipe.GetOptions().iCategoryValueCount,
                                            lPATRecipe.GetOptions().bAssumeIntegerCategory,
                                             lPATRecipe.GetOptions().mMinConfThreshold) == PATMAN_LIB_SHAPE_GAUSSIAN))
        {
            lTestList.append(lTestCell);
        }
    }

    QSharedPointer<GS::SE::RData> lStatsData = QSharedPointer<GS::SE::RData>(new GS::SE::RData());
    GS::SE::RMatrix* lRMatrix = lStatsData.data()->AllocateMatrix(GS::SE::StatsData::MVGROUP_IN_1,
                                                          lRowCount,
                                                          lTestList.count());
    if (lRMatrix == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Unable to allocate data matrix");
        return false;
    }

    // Iterate over all parameters
    for (int lIdx = 0; lIdx < lTestList.count(); ++lIdx)
    {
        lTestCell = lTestList.at(lIdx);

        if (lTestCell == NULL)
            return false;

        lRow = 0;

        // Fill the R Matrix with results
        for(int lIdx = 0; lIdx < lTestCell->m_testResult.count(); ++lIdx)
        {
            if (lTestCell->m_testResult.isValidResultAt(lIdx))
                lRMatrix->Fill(lRow, lCol, lTestCell->m_testResult.resultAt(lIdx));
            else
                lRMatrix->Fill(lRow, lCol, nan(""));

            ++lRow;
        }

        // Increment the col number (correspond to a parameter)
        ++lCol;
    }

    if (lAlgo->Execute(lParam, lStatsData.data()) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error while executing Groups Builder algorithm: %1")
              .arg(lAlgo->GetLastError()).toLatin1().data());
        return false;
    }

    bool lOk;
    QList<GS::Gex::PATMultiVariateRule> lMvRules;

    int lGgroupsSize = lAlgo->GetGroupsSize();
    for (int i = 0; i < lGgroupsSize; ++i)
    {
        GS::SE::RVector lVector = lAlgo->GetGroup(i);
        if (lVector.IsNull())
            continue;

        GS::Gex::PATMultiVariateRule lMvGroup;
        for (int j = 0; j < lVector.GetSize(); ++j)
        {
            int index = lVector.GetIntegerItem(j, lOk);
            if (lOk && index < lTestList.count())
            {
                CTest * ltest = lTestList.at(index);

                GS::Gex::PATMultiVariateRule::MVTestData lmvData(ltest->strTestName,
                                                                 ltest->lTestNumber,
                                                                 ltest->lPinmapIndex);
                lMvGroup.AddTestData(lmvData);
            }
        }

        lMvGroup.SetEnabled(true);
        lMvGroup.SetRule(GS::Gex::PATMultiVariateRule::Generated);
        lMvGroup.SetBin(lPATRecipe.GetOptions().GetMVPATSoftBin());
        lMvGroup.SetName(QString("Rule# %1").arg(i));

        lMvRules.append(lMvGroup);
    }

    // Add MV rules to the recipe
    lPATRecipe.SetMultiVariateRules(lMvRules);

    return true;
}




///////////////////////////////////////////////////////////
// WIZARD: PAT Recipe editor...
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_EditPAT_Limits(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION
       || GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in Evaluation mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Check if this optional module is enabled
    if(!GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
    {
        GS::Gex::Message::information(
            "", "Your current license does not allow this function.\n"
            "You need the PAT-Man module (Outlier Removal).\n\n"
            "Please contact " + QString(GEX_EMAIL_SALES));
        return;
    }

    // Show wizard page : Edit PAT file
    ShowWizardDialog(GEXTB_EDIT_PAT_WIZARD_P1);

    // Update the URL edit box
    QString strString = GEX_BROWSER_ACTIONBOOKMARK;
    strString += GEXTB_BROWSER_PAT;
    AddNewUrl(strString);
}


///////////////////////////////////////////////////////////
// WIZARD: Modify test program to be PAT-Ready
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_ProgramPAT(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION
       || GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in Evaluation mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Check if this optional module is enabled
    if(!GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
    {
        GS::Gex::Message::information(
            "", "Your current license does not allow this function.\n"
            "You need the PAT-Man module (Outlier Removal).\n\n"
            "Please contact " + QString(GEX_EMAIL_SALES));
        return;
    }

    // Show wizard page : Convert files to be real-time PAT ready.
    sToolBoxTask = GEXTB_CONVERT_PAT_WIZARD_P1;
    ShowWizardDialog(sToolBoxTask);

    // Update the URL edit box
    QString strString = GEX_BROWSER_ACTIONBOOKMARK;
    strString += GEXTB_BROWSER_MAKE_CSV;
    AddNewUrl(strString);

    // If previous call was a different 'Convert' wizard....erase list box.
    if(sPrevTask != GEXTB_CONVERT_PAT_WIZARD_P1)
        pGexTbTools->treeWidget->clear();	// Clear 'Status' window

    // Keep track of the last toolbox wizard used.
    sPrevTask = sToolBoxTask;
}

///////////////////////////////////////////////////////////
// WIZARD: Force reprocessing a data file (used when batch PAT
// processing done from GUI and user want to navigate into
// interactive mode into one of the files
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_ReloadDatasetPAT(void)
{
    GSLOG(6, QString("Wizard_GexTb_ReloadDatasetPAT %1").arg(m_selectedURL).toLatin1().data());
    // Parse HTML Hyperlink clicked: see argument after the '#'
    QString strParameter = m_selectedURL.section('#',2);

#ifndef GCORE15334
    GSLOG(3 , "Feature not available in this version, please contact support for more informations");
#else
    // Force reprocessing given file.
    if(mWizardWSPAT)
    {
        QString r=mWizardWSPAT->OnReProcessFile(strParameter.toInt());
        GSLOG(6, QString("Wizard_GexTb_ReloadDatasetPAT : OnReProcessFile returned %1").arg(r).toLatin1().data());
        // LoadUrl( ? ); // We dont have here the real desired url
    }
    else
        GSLOG(4, "Cant reprocess Pat because WizardProcessPAT NULL" );

    GSLOG(5, "Wizard_GexTb_ReloadDatasetPAT end.");
#endif
}

///////////////////////////////////////////////////////////
// WIZARD: Release PAT config file to PAT-Man production folder
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_ReleasePAT(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION
       || GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode or OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in Evaluation mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        GS::Gex::Message::information(
            "", "Your Examinator release doesn't allow this function.\n"
            "You need Quantix PAT-Man.\n\nPlease contact " +
            QString(GEX_EMAIL_SALES));
        return;
    }

    // Check if this optional module is enabled
    if(!GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
    {
        GS::Gex::Message::information(
            "", "Your Examinator release doesn't allow this function.\n"
            "You need the PAT-Man module (Outlier Removal).\n\n"
            "Please contact " + QString(GEX_EMAIL_SALES));
        return;
    }

    QFileDialog cFileDialog(this);
    QString strPATfile = cFileDialog.getOpenFileName(this, "Select Config file", "", "Outlier Config file *.csv;;All Files (*.*)");
    if(strPATfile.isEmpty() == TRUE)
        return;	// No mailing list file selected...return!

    // Copy file into PAT-Man folder : <databases>/.patman
    QString		strReleaseFolder;
    QDir	cDir;

    // Build path to the Config list.
    strReleaseFolder = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    strReleaseFolder += GEX_DATABASE_FOLDER;
    cDir.mkdir(strReleaseFolder);
    strReleaseFolder += GEXMO_PATMAN_FOLDER;
    cDir.mkdir(strReleaseFolder);

    // Output file path
    QFileInfo cFileInfo(strPATfile);
    strReleaseFolder += "/" + cFileInfo.completeBaseName() + "." + cFileInfo.suffix();

    // IF file already exists in 'Release' folder, askk confirmation to overwrite.
    if(QFile::exists(strReleaseFolder) == true)
    {
        // File exists...overwrite it?
        bool lOk;
        GS::Gex::Message::request("", "File already exists in production folder. Overwrite it ?", lOk);
        if (! lOk)
        {
            return;
        }
    }

    if(CGexSystemUtils::CopyFile(strPATfile,strReleaseFolder))
        GS::Gex::Message::information("",
                                      "Outlier Configuration file released!");
    else
    {
        QString strMessage;
        strMessage = "FAILED releasing file. Disk access privilege issue?\n\nFile:\n";
        strMessage += strReleaseFolder;
        GS::Gex::Message::warning("", strMessage);
    }

    OnHome();
}

///////////////////////////////////////////////////////////
// WIZARD: Convert files to CSV
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_CSV(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION
       || GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // Evaluation mode or OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in Evaluation mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Show wizard page : Convert files file.
    sToolBoxTask = GEXTB_CONVERT_CSV_WIZARD_P1;
    ShowWizardDialog(sToolBoxTask);

    // Update the URL edit box
    QString strString = GEX_BROWSER_ACTIONBOOKMARK;
    strString += GEXTB_BROWSER_MAKE_CSV;
    AddNewUrl(strString);

    // If previous call was a different 'Convert' wizard....erase list box.
    if(sPrevTask != GEXTB_CONVERT_CSV_WIZARD_P1)
        pGexTbTools->treeWidget->clear();	// Clear 'Status' window

    // Keep track of the last toolbox wizard used.
    sPrevTask = sToolBoxTask;
}


///////////////////////////////////////////////////////////
// WIZARD: Edit data file as Excel/Spreadsheet
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexTb_Edit(void)
{
    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Show wizard page : Convert files file.
    sToolBoxTask = GEXTB_EDIT_CSV_WIZARD_P1;
    ShowWizardDialog(sToolBoxTask);

    // Update the URL edit box
    QString strString = GEX_BROWSER_ACTIONBOOKMARK;
    strString += GEXTB_BROWSER_EDIT_CSV;
    AddNewUrl(strString);

    // Keep track of the last toolbox wizard used.
    sPrevTask = sToolBoxTask;
}

///////////////////////////////////////////////////////////
// Constructor.
///////////////////////////////////////////////////////////
GexToolboxTable::GexToolboxTable( QWidget * parent)
    : QTableWidget( parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
}


///////////////////////////////////////////////////////////
// Call-back function to paint cell background based on its content, show custom color to highligh some rules selections.
void GexToolboxTable::paintCell (int row, int col)
{
    blockSignals(true);
    QString strCell;
    strCell = item(row,0)->text();
    QTableWidgetItem *cellItem = item(row,col);

    // Check if line selected is within the parameter settings line, if not quietly exit.
    // check if we ignore those line (if starts with a '#')
    if( (row > pGexMainWindow->pGexTbTools->mLastTestNumberParameterLine)
        || (row <= pGexMainWindow->pGexTbTools->mLineTestNumberParameter)
        || (item(row,0)->text().startsWith("#")))
        return;

    cellItem->setTextAlignment(Qt::AlignRight);

    // If Data file editor, check if cell to redraw holds a value out of spec lmits (and if so, highlight in red!)
    if(sToolBoxTask == GEXTB_EDIT_CSV_WIZARD_P1)
    {
        long lOffsetLowL	= 3;	// Offset of the low limit cell from the parameter line
        long lOffsetHighL	= 4;	// Offset of the high limit cell from the parameter line

        if (pGexMainWindow->pGexTbTools->m_bLinePatternsExists)
        {
            lOffsetLowL++;
            lOffsetHighL++;
        }

        if(sPivoted)
        {
            // only check columns with data values, not the header lines holding Test#, limits, etc.
            if(col <= lOffsetHighL)
                goto end_paint;
        }
        else
        {
            // only check lines with data values, not the header lines holding Test#, limits, etc.
            if(row <= pGexMainWindow->pGexTbTools->mLineTestNumberParameter + lOffsetHighL)
                goto end_paint;
        }
        // Data start after columns holding fields: SBIN, HBIN, DIE_X, DIE_Y, ..., TIME,
        if(col < 7)
            goto end_paint;

        double	lfLowLimit(0),lfHighLimit(0),lfData(0);
        bool	bValidLimit(false),bValidData(true);
        lfData = cellItem->text().toDouble(&bValidData);
        // If current cell holds valid data, check if it passes its limits
        if(bValidData)
        {
            if(sPivoted)
            {
                if (item(row,lOffsetHighL))
                    lfLowLimit = item(row,lOffsetHighL)->text().toDouble(&bValidLimit);
                if(bValidLimit && lfData < lfLowLimit)
                    bValidData = false;	// Value is UNDER the Low Spec Limit: FAIL VALUE
                if (item(row,lOffsetLowL))
                    lfHighLimit = item(row,lOffsetLowL)->text().toDouble(&bValidLimit);
                if(bValidLimit && lfData > lfHighLimit)
                    bValidData = false;	// Value is UNDER the Low Spec Limit: FAIL VALUE
            }
            else
            {
                if (item(pGexMainWindow->pGexTbTools->mLineTestNumberParameter+lOffsetHighL,col))
                    lfLowLimit = item(pGexMainWindow->pGexTbTools->mLineTestNumberParameter+lOffsetHighL,col)->text().toDouble(&bValidLimit);
                if(bValidLimit && lfData < lfLowLimit)
                    bValidData = false;	// Value is UNDER the Low Spec Limit: FAIL VALUE
                if (item(pGexMainWindow->pGexTbTools->mLineTestNumberParameter+lOffsetLowL,col))
                    lfHighLimit = item(pGexMainWindow->pGexTbTools->mLineTestNumberParameter+lOffsetLowL,col)->text().toDouble(&bValidLimit);
                if(bValidLimit && lfData > lfHighLimit)
                    bValidData = false;	// Value is UNDER the Low Spec Limit: FAIL VALUE
            }

            if(bValidData == false)
            {
                cellItem->setBackground(QColor(Qt::red));	// Selection to ignore outlier removal on this test!...so highlight in Red!
            }
        }
        goto end_paint;
    }

    // Below code only applies to PAT-Man Editing mode.
    switch(sToolBoxTask)
    {
        case GEXTB_CONVERT_PAT_WIZARD_P1:
            break;

        default:
            goto end_paint;
    }

    if(col == GEX_TPAT_CONFIG_FIELD_IGNOREDATA)
    {
        // 'Samples to ignore' (either none, negative values or positive values)
        if(GS::Gex::PAT::GetSamplesToIgnoreIndex(item(row,col)->text(),NULL) != GEX_TPAT_IGNOREDATA_NONEID)
            item(row,col)->setBackground(QColor(Qt::yellow));	// Selection to Ignore some data!...so highlight in Yellow!
    }
    else
    if(col == GEX_TPAT_CONFIG_FIELD_KEEP)
    {
        // 'Outliers to keep' (either none, low values or high values)
        if(GS::Gex::PAT::GetOutlierToKeepIndex(item(row,col)->text(),NULL) != GEX_TPAT_KEEPTYPE_NONEID)
            item(row,col)->setBackground(QColor(Qt::yellow)); // Selection to keep some outliers!...so highlight in Yellow!
    }
    else
    if(col == GEX_TPAT_CONFIG_FIELD_NNR_RULE)
    {
        // 'NNR Rule'
        if(GS::Gex::PAT::GetNNRRuleIndex(item(row,col)->text(),NULL) == GEX_TPAT_NNR_DISABLED)
            item(row,col)->setBackground(QColor(255, 128, 128));	// Selection to ignore outlier removal on this test!...so highlight in Red!
    }
    else
    if(col == GEX_TPAT_CONFIG_FIELD_OUTLIER_LIMIT_SET)
    {
        // 'Outliers Limits set' (either all, medium+far or far)
        if(GS::Gex::PAT::GetOutlierLimitsSetIndex(item(row,col)->text(),NULL) != GEX_TPAT_LIMITSSET_NEAR)
            item(row,col)->setBackground(QColor(Qt::yellow));	// Selection to keep some outliers!...so highlight in Yellow!
    }
    else
    if(col == GEX_TPAT_CONFIG_FIELD_RULE)// || col == GEX_TPAT_CONFIG_FIELD_TNUM)
    {
        if(GS::Gex::PAT::GetRuleIndex(item(row,GEX_TPAT_CONFIG_FIELD_RULE)->text(),NULL) == GEX_TPAT_RULETYPE_IGNOREID)
            item(row,col)->setBackground(QColor(255, 128, 128));	// Selection to ignore outlier removal on this test!...so highlight in Red!
    }
    else
    if(col == GEX_TPAT_CONFIG_FIELD_SPC_TEST_CPK)
    {
        // If a custom Cpk alarm has been set, show it with a yellow background
        strCell = item(row,col)->text();
        strCell = strCell.trimmed();
        if(strCell.isEmpty() == false)
            item(row,col)->setBackground(QColor(Qt::yellow));	// Cpk alarm set (non empty cell)!...so highlight in Yellow!
    }
    else
    if(col == GEX_TPAT_CONFIG_FIELD_SBINRULE)
    {
        // 'SBIN PAT tule type'
        switch(GS::Gex::PAT::GetSPATRuleIndex(item(row,col)->text(),NULL))
        {
            case GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA: // Default => keep standard white color
                break;

            case GEX_TPAT_SPAT_ALGO_IGNORE: // Disabled
                item(row,col)->setBackground(QColor(255, 128, 128));	// Selection to ignore outlier removal on this test!...so highlight in Red!
                break;

            default:
                item(row,col)->setBackground(QColor(Qt::yellow));	// SPAT rule is NOT the AEC!, Highlight in yellow!
                break;
        }
    }
    else
    if(col == GEX_TPAT_CONFIG_FIELD_DPAT_COMMENT || col == GEX_TPAT_CONFIG_FIELD_SPAT_COMMENT)
    {
        // If a comment is defined, higlight it in watery-green
        strCell = item(row,col)->text();
        strCell = strCell.trimmed();
        if(strCell.isEmpty() == false)
            item(row,col)->setBackground(QColor(192, 255, 192));	// comment defined (non empty cell)!...so highlight in Yellow!
    }

    // Paint cell backgound.
end_paint:;
    blockSignals(false);
}

///////////////////////////////////////////////////////////
// GexToolboxTable: return first selected column
///////////////////////////////////////////////////////////
int GexToolboxTable::firstSelectedColumn()
{
    int lCol(-1);

    QList<QTableWidgetSelectionRange> selectedRegion = selectedRanges();
    if (!selectedRegion.empty())
        lCol = (*(selectedRegion).begin()).leftColumn();

    return lCol;
}

///////////////////////////////////////////////////////////
// GexToolboxTable: return first selected row
///////////////////////////////////////////////////////////
int GexToolboxTable::firstSelectedRow()
{
    int lRow(-1);

    QList<QTableWidgetSelectionRange> selectedRegion = selectedRanges();
    if (!selectedRegion.empty())
        lRow = (*(selectedRegion).begin()).topRow();

    return lRow;
}


///////////////////////////////////////////////////////////
// Toolbox: constructor
///////////////////////////////////////////////////////////
GexTbToolBox::GexTbToolBox( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setupUi(this);
    setModal(modal);

    // apply gex palette
    GexMainwindow::applyPalette(this);

    // Do not display SPC tab
    QObject::connect(treeWidget,				SIGNAL(doubleClicked(QModelIndex)),     this, SLOT(OnProperties()));
    QObject::connect(treeWidget,				SIGNAL(customContextMenuRequested(QPoint)),
                                                            this, SLOT(onCustomContextMenuRequested(QPoint)));
    QObject::connect(buttonProperties,			SIGNAL(clicked()),						this, SLOT(OnProperties()));
    QObject::connect(buttonConvertFile,			SIGNAL(clicked()),						this, SLOT(OnConvertButton()));
    QObject::connect(buttonSave,				SIGNAL(clicked()),						this, SLOT(OnSaveButton()));
    QObject::connect(lineEdit,					SIGNAL(textChanged(QString)),		this, SLOT(OnEditChanged(QString)));
    QObject::connect(buttonDetachWindow,		SIGNAL(clicked()),						this, SLOT(OnDetachWindow()));

    // title
    setWindowTitle("ToolBox");

    // At startip, this window is a child of Examinator Frame
    mChildWindow = true;
    buttonDetachWindow->setChecked(mChildWindow);

    connect( buttonPivot, SIGNAL( clicked() ), this, SLOT( OnPivoteTable() ) );

    Init();
}

///////////////////////////////////////////////////////////
// Toolbox: init function
///////////////////////////////////////////////////////////
void GexTbToolBox::Init()
{
    // Create & Move Table to the right location
    mExcelTable = new GexToolboxTable(this);
    mExcelTable->move(0,83);
    lEditorRows				= 50;	// Total lines in the editor table
    lEditorCols				= 15;	// Total Columns in the editor table
    m_bLinePatternsExists = false;	// Holds that the line patterns exists in the table

    lineEdit->hide();
    // Disable 'Pivote' button
    buttonPivot->hide();

    // Deactivate the 'Properties' button when the editor is empty
    buttonProperties->setEnabled(false);

    mLineTestNumberParameter=-1;// Holds the Table Line# that has the Parameters list (in case we have a Examinator-CSV compliant file)
    mLastTestNumberParameterLine=-1; // Holds the line# of the LAST parameter found in the list.

    // Connects table signal (right click, cell click, etc...)
    connect( mExcelTable, SIGNAL( cellChanged(int,int) ), this, SLOT( OnTableValueChanged(int,int) ) );
    connect( mExcelTable, SIGNAL( cellChanged(int,int) ), this, SLOT( OnTableModified() ) );
    connect( mExcelTable, SIGNAL( customContextMenuRequested(const QPoint&) ),
                                                this, SLOT( OnContextualMenu(const QPoint&) ) );

    mExcelTable->setColumnCount(lEditorCols);
    mExcelTable->setRowCount(lEditorRows);

    // Support for Drag&Drop
    setAcceptDrops(true);

    QSettings settings;
    // Clear variables
    m_bTableModified = false;
    m_strToolboxFolder      = settings.value("toolbox/workingFolder").toString();
    m_bNewMajorRelease      = false;
    setContextMenuPolicy(Qt::CustomContextMenu);
}


///////////////////////////////////////////////////////////
// Cell content edited...update the EditBox accordingly.
///////////////////////////////////////////////////////////
void GexTbToolBox::OnTableValueChanged(int row, int col)
{
    GSLOG(7, QString("ToolBox On Table Value Changed row %1 col %2").arg(row).arg(col).toLatin1().constData() );
    QString strText = GetTextFromItem(row, col);
    lineEdit->setText(strText);

    // Make sure the Edit box is visible
    lineEdit->show();

    QString strTitle;
    switch(sToolBoxTask)
    {
        case GEXTB_CONVERT_STDF_WIZARD_P1:
        case GEXTB_CONVERT_CSV_WIZARD_P1:
        case GEXTB_CONVERT_PAT_WIZARD_P1:
        case GEXTB_EDIT_CSV_WIZARD_P1:
            // If we have a Examinator-CSV compliant file, also show the Parameter # and name + info
            if(sPivoted)
            {
                if(mLineTestNumberParameter < 0 || row < mLineTestNumberParameter)
                {
                    ParameterTitle->setText("");
                    break;
                }
            }
            else
            {
                if(mLineTestNumberParameter < 0 || col < 7)
                {
                    ParameterTitle->setText("");
                    break;
                }
            }
            if(sPivoted)
            {
                strTitle  = "Parameter#:\t" + GetTextFromItem(row,1);
                strTitle += "\nName:\t" + GetTextFromItem(row,0);
                strTitle += "\nPattern Name:\t" + GetTextFromItem(row,2);
                strTitle += "\nLow Spec Limit:\t" + GetTextFromItem(row,5) + "  " + GetTextFromItem(row,3);
                strTitle += "\nHigh Spec Limit:\t" + GetTextFromItem(row,4) + "  " + GetTextFromItem(row,3);
            }
            else
            {
                strTitle  = "Parameter#:\t" + GetTextFromItem(mLineTestNumberParameter+1,col);
                strTitle += "\nName:\t" + GetTextFromItem(mLineTestNumberParameter,col);
                strTitle += "\nPattern Name:\t" + GetTextFromItem(mLineTestNumberParameter+2,col);
                strTitle += "\nLow Spec Limit:\t" + GetTextFromItem(mLineTestNumberParameter+5,col) + "  " + GetTextFromItem(mLineTestNumberParameter+3,col);
                strTitle += "\nHigh Spec Limit:\t" + GetTextFromItem(mLineTestNumberParameter+4,col) + "  " + GetTextFromItem(mLineTestNumberParameter+3,col);
            }
            ParameterTitle->setText(strTitle);
            break;
    }
}

///////////////////////////////////////////////////////////
// Toolbox: destructor
///////////////////////////////////////////////////////////
GexTbToolBox::~GexTbToolBox()
{
    // Save GUI setti   ngs
    QSettings settings;
    settings.setValue("editor/workingFolder", m_strEditFile);
    settings.setValue("toolbox/workingFolder",m_strToolboxFolder);
}

///////////////////////////////////////////////////////////
// EditBox content modified, copy to selected Cell.
///////////////////////////////////////////////////////////
void GexTbToolBox::OnEditChanged(const QString& strText)
{
//    GSLOG(7, QString("ToolBox edit changed to %1...").arg(strText) );
    int lRow = mExcelTable->currentRow();
    int lCol = mExcelTable->currentColumn();

    if (lRow == -1 || lCol == -1)
        return;

    mExcelTable->blockSignals(true);

    if (!mExcelTable->item(lRow, lCol))
        mExcelTable->setItem(lRow, lCol, new QTableWidgetItem());
    mExcelTable->item(lRow,lCol)->setText(strText);

    mExcelTable->blockSignals(false);
}


///////////////////////////////////////////////////////////
// Contextual menu Table clicked
//////////////////////////////////////////////////////////////////////
void GexTbToolBox::OnContextualMenu(const QPoint& /*pos*/)
{
    QAction *	pActionSelect						= NULL;
    QAction *	pActionProperties					= NULL;
    QAction *	pActionSave							= NULL;
    QAction *   pActionEditTests                    = NULL;
    QAction *	pActionDeleteLines					= NULL;
    QAction *	pActionDeleteCols					= NULL;
    QAction *	pSortByPartID						= NULL;
    QMenu		menu(this);

    // Build menu.
    pActionSelect					= menu.addAction(*pixOpen,"Select file to edit...");
    pActionProperties				= menu.addAction(*pixProperties,"Properties...");
    pActionSave						= menu.addAction(*pixSave,"Save edits to spreadsheet CSV file");

    // Advanced proerties...
    menu.addSeparator();
    pActionEditTests = menu.addAction("Edit Test(s)...", this, SLOT(onEditTests()));
    pSortByPartID = menu.addAction("Sort By Part-ID");

    menu.addSeparator();
    pActionDeleteLines = menu.addAction(*pixRemove,"Delete selected rows");

    // Toolbox (except whan in PAT Editor mode) accepts to add test and delete columns
    pActionDeleteCols = menu.addAction(*pixRemove,"Delete selected columns");
    menu.addAction("Add Test before selection...", this, SLOT(onAddTestBeforeSelected()));
    menu.addAction("Add Test after selection...", this, SLOT(onAddTestAfterSelected()));

    menu.setMouseTracking(true);
    QAction * pActionResult = menu.exec(QCursor::pos());
    menu.setMouseTracking(true);

    // Menu cancelled?
    if(pActionResult == NULL)
        return;

    // Process Scale/Offset parameters
    if (pActionResult == pActionEditTests)
        return;

    // Process menu selection
    if(pActionResult == pActionSelect)
    {
        OnConvertButton();
        return;
    }

    if(pActionResult == pActionProperties)
    {
        OnProperties();
        return;
    }

    if(pActionResult == pActionSave)
    {
        OnSaveButton();
        return;
    }

    if(pActionResult == pActionDeleteLines)
    {
        OnDelete(true);
        return;
    }

    if(pActionResult == pActionDeleteCols)
    {
        OnDelete(false);
        return;
    }

    if(pActionResult == pSortByPartID)
    {
        // Sort by Part ID (ascending)
        OnSortByPartID(true);
        return;
    }
}

///////////////////////////////////////////////////////////
// Toolbox: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void GexTbToolBox::reject(void)
{
}

///////////////////////////////////////////////////////////
// Close signal: If window is detached, this reparent it instead
///////////////////////////////////////////////////////////
void GexTbToolBox::closeEvent(QCloseEvent *e)
{
    if(mChildWindow)
        e->accept();
    else if(pGexMainWindow!=NULL)
    {
        // Re-attac window to Examinator!
        buttonDetachWindow->setChecked(true);
        ForceAttachWindow(buttonDetachWindow->isChecked(), false);

        e->ignore();
    }
}

///////////////////////////////////////////////////////////
// Close signal: If window is detached, this reparent it instead
///////////////////////////////////////////////////////////
void GexTbToolBox::resizeEvent(QResizeEvent* /*e*/)
{
    QSize space = size();
    if(space.width() < 2 || space.height() < 2)
        return;
    resize(space.width(),space.height());
    mExcelTable->resize(space.width()-2,space.height()-85);
}

///////////////////////////////////////////////////////////
// NON-GUI Function to decide to attached or detach the window
///////////////////////////////////////////////////////////
void GexTbToolBox::ForceAttachWindow(bool bAttach/*=true*/, bool bToFront /*= true*/)
{
    mChildWindow = bAttach;
    buttonDetachWindow->setChecked(mChildWindow);

    if(mChildWindow && pGexMainWindow != NULL)
    {
        // Re-attacch dialog box to Examinator's scroll view Widget
        pGexMainWindow->pScrollArea->layout()->addWidget(this);

        // Minimum width is 720 pixels
        setMinimumWidth(720);

        if (bToFront)
            pGexMainWindow->ShowWizardDialog(sToolBoxTask);
        else
            hide();
    }
    else if (parent() != NULL)
    {
        pGexMainWindow->pScrollArea->layout()->removeWidget(this);
        setParent(NULL, Qt::Dialog);

        // Setup the application buttons
        setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

        move(QPoint(100, 100));
        show();

        // No Minimum width
        setMinimumWidth(10);

        // Make sure the HTML page is visible now that this window is detached from Examinator.
        if(pGexMainWindow != NULL)
            pGexMainWindow->ShowHtmlBrowser();
    }
}

///////////////////////////////////////////////////////////
// Attach / Detach window, toggle button.
///////////////////////////////////////////////////////////
void GexTbToolBox::OnDetachWindow(void)
{
    ForceAttachWindow(buttonDetachWindow->isChecked());
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexTbToolBox::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexTbToolBox::dropEvent(QDropEvent *e)
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

    // Convert (or edit) file(s) dropped.
    switch(sToolBoxTask)
    {
        case GEXTB_CONVERT_STDF_WIZARD_P1:
        case GEXTB_CONVERT_CSV_WIZARD_P1:
        case GEXTB_CONVERT_PAT_WIZARD_P1:
            // Convert files selected to given format (STDF or CSV)
            // or, Convert test program to include real-time PAT function calls.
            ConvertFiles(strFileList);
            break;

        case GEXTB_EDIT_CSV_WIZARD_P1:
            m_strEditFile = strFileList[0];
            // Load data file into the table.
            LoadExcelTable();
            break;
    }

    e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Contextual menu on TreeWidget
///////////////////////////////////////////////////////////
void GexTbToolBox::onCustomContextMenuRequested(const QPoint& /*pos*/)
{
    QMenu menu(this);

    // Build menu.
    menu.addAction(*pixOpen,		"Select files...",	this, SLOT(OnConvertButton()));
    menu.addAction(*pixProperties,	"Properties...",	this, SLOT(OnProperties()));

    menu.setMouseTracking(true);
    menu.exec(QCursor::pos());
    menu.setMouseTracking(false);
}

///////////////////////////////////////////////////////////
// Rebuild combo-box items for a given cell
///////////////////////////////////////////////////////////
QComboBox * GexTbToolBox::builtCellComboList(const char *szComboItems[])
{
    QStringList	strItemsList;
    QComboBox *pCombo = new QComboBox();

    strItemsList.clear();
    int iIndex = 0;
    while(szComboItems[iIndex])
    {
        strItemsList.append(szComboItems[iIndex++]);
    };
    return pCombo;
}

///////////////////////////////////////////////////////////
// Update tooltip / show/hide relevant Forms
///////////////////////////////////////////////////////////
void GexTbToolBox::showConvertFields(bool bConvertPage)
{
    if(bConvertPage == true)
    {
        // Showing 'Convert data file' page
        mExcelTable->hide();
        buttonSave->hide();
        LineBreak->hide();
        lineEdit->hide();
        ParameterTitle->hide();
        groupBox->show();
        TextLabelNote->show();

        // Tooltips
        buttonConvertFile->setToolTip(tr("Select (or Drag) files to convert"));
        buttonProperties->setToolTip(tr("Properties: File conversion status"));
    }
    else
    {
        // Showing 'Edit data file' page
        mExcelTable->show();
        buttonSave->show();
        LineBreak->show();
        ParameterTitle->setText("");
        ParameterTitle->show();
        groupBox->hide();
        TextLabelNote->hide();
        lineEdit->hide();	// Only visible when editing a cell!

        // Tooltips
        buttonConvertFile->setToolTip(tr("Load (or Drag) data file to edit"));
        buttonProperties->setToolTip(tr("Properties: File information"));
    }
}

///////////////////////////////////////////////////////////
// Show Wizard + update title
///////////////////////////////////////////////////////////
void GexTbToolBox::showWindows(void)
{
    GSLOG(7, QString("ToolBox (task %1) show window...").arg(sToolBoxTask).toLatin1().constData());

    // title
    setWindowTitle("ToolBox");
    mExcelTable->blockSignals(true);

    // Disable 'Pivote' button
    buttonPivot->hide();

    switch(sToolBoxTask)
    {
        case GEXTB_CONVERT_STDF_WIZARD_P1:
            // Update tooltip + show relevant Fields
            showConvertFields(true);

            TextLabel->setText("Convert to STDF");
            break;

        case GEXTB_CONVERT_CSV_WIZARD_P1:
            // Update tooltip + show relevant Fields
            showConvertFields(true);

            TextLabel->setText("Convert to CSV");
            break;

        case GEXTB_CONVERT_PAT_WIZARD_P1:
            // Update tooltip + show relevant Fields
            showConvertFields(true);

            TextLabel->setText("Convert Test programs for PAT");
            break;

        case GEXTB_EDIT_CSV_WIZARD_P1:
            // Update tooltip + show relevant Fields
            showConvertFields(false);

            // Enable 'Pivote' button
            buttonPivot->show();

            TextLabel->setText("Edit Semiconductor Data");
            break;
    }

    mExcelTable->blockSignals(false);
    ShowPage();
}

///////////////////////////////////////////////////////////
// Make page visible.
///////////////////////////////////////////////////////////
void GexTbToolBox::ShowPage(void)
{
    // Show page
    // restore window if it is minimized
    if (isMinimized())
        setWindowState(windowState() & ~Qt::WindowMinimized);
    else
        show();

    // If window detached, make sure it comes back to the front, and have Examinator show the Home page
    if(mChildWindow == false)
        // Move to front.
        raise();
}

///////////////////////////////////////////////////////////
// Select + Convert files
// or
// Select + Edit CSV file.
///////////////////////////////////////////////////////////
void GexTbToolBox::OnConvertButton(void)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in Evaluation mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    QString strTitle,strComment,strSelection;
    bool	bConvertPage=FALSE;

    // Computer date not okay...license out of date!
    if(GS::Gex::Engine::GetInstance().CheckForLicense() != GS::StdLib::Stdf::NoError)
        return;

    switch(sToolBoxTask)
    {
        case GEXTB_CONVERT_STDF_WIZARD_P1:
            strComment   = "Convert Semiconductor Data to STDF";
            strTitle = "Convert to STDF";
            bConvertPage = true;
            break;

        case GEXTB_CONVERT_CSV_WIZARD_P1:
            strComment   = "Convert Semiconductor Data to spreadsheet.CSV";
            strTitle = "Convert to spreadsheet CSV";
            bConvertPage = true;
            break;

        case GEXTB_CONVERT_PAT_WIZARD_P1:
            strComment   = "Convert test programs for Real-Time PAT";
            strTitle = "Convert programs for PAT";
            bConvertPage = true;
            break;

        case GEXTB_EDIT_CSV_WIZARD_P1:
            strTitle = strComment   = "Edit Semiconductor Data";
            bConvertPage = false;
            break;
    }

    // Select file(s) from disk...
    GS::Gex::SelectDataFiles	cSelectFiles;
    QStringList			sFilesSelected;

    if(bConvertPage == true)
    {
        // Convert files to XXXX format
        switch(sToolBoxTask)
        {
            default:
                sFilesSelected = cSelectFiles.GetFiles(this,m_strToolboxFolder, strComment);
                break;

            case GEXTB_CONVERT_PAT_WIZARD_P1:
                sFilesSelected = QFileDialog::getOpenFileNames(this, "Modify programs for real-time PAT", "", "Test programs (*.tl *.load);;All (*.*)");
                break;

        }
        if(sFilesSelected.isEmpty() == TRUE)
            return;	// Empty list...ignore task!

        // Convert files selected to given format (STDF or CSV)
        // ...or convert test program to be PAR ready
        ConvertFiles(sFilesSelected);

        // Extract working folder
        QFileInfo cFileInfo(sFilesSelected.first());
        m_strToolboxFolder = cFileInfo.absolutePath();
    }
    else
    {
        // Edit file.
        strSelection = cSelectFiles.GetSingleFile(this,m_strEditFile, strComment);
        if(strSelection.isEmpty() == TRUE)
            return;	// No file selected...ignore task!

        // Update file selected
        m_strEditFile = strSelection;

        // Load data file into the table.
        LoadExcelTable();
    }
}

///////////////////////////////////////////////////////////
// Convert a test program to be PAT ready:
// File type: Image A5xx .tl test program
///////////////////////////////////////////////////////////
bool GexTbToolBox::ConvertProgramForPat_Image_TL(QString strTestProgram,QString &strErrorMessage)
{
    // Make a copy of the original file (<file name>.bak)
    QString strBackupCopy = strTestProgram + ".bak";
    GS::Gex::Engine::RemoveFileFromDisk(strBackupCopy);
    if(CGexSystemUtils::CopyFile(strTestProgram,strBackupCopy) == false)
    {
        strErrorMessage = "Failed to create a backup copy";
        return false;
    }

    // Open file and insert all PAT keywords at the right place.
    QString strNewFile = strTestProgram + ".pattmp";
    QFile	cInput(strTestProgram);
    QFile	cOutput(strNewFile);

    // Open source in read mode
    if(!cInput.open(QIODevice::ReadOnly))
    {
        strErrorMessage = "Failed to open program file (file locked?)";
        return false;
    }

    // Create new program file
    if(!cOutput.open(QIODevice::WriteOnly))
    {
        strErrorMessage = "Failed to create new file (disk protected?)";
        return false;
    }

    // Assign file I/O stream
    QTextStream hOutFile;
    QTextStream hInFile;
    hInFile.setDevice(&cInput);	// Original test program
    hOutFile.setDevice(&cOutput);	// Modified version to create

    // Variables telling which Image functions have been found
    bool	bEndLotFunction=false;		// Set to true if program has function 'tl_user_endlot'
    bool	bBeginJobFunction=false;	// Set to true if program has function 'tl_user_startup'
    bool	bEndJobFunction=false;		// Set to true if program has function 'tl_user_shutdown'
    bool	bSortBinNbr=false;			// Set to true if find 'sort bin=xxx;'

    QString	strString;
    QString	strTest;
    int	iIndex,i1,i2;
    do
    {
        // Read line.
        strString = hInFile.readLine();

        // Check for line '#include <image.h>
        i1 = strString.indexOf("#include");
        i2 = strString.indexOf("<image.h>");
        if(i1 >= 0 && i2 > i1)
        {
            strTest = strString.trimmed();
            if(strTest.startsWith("#include"))
            {
                // This is the '#include <image.h>' line!
                hOutFile << strString << endl;
                hOutFile << "#include \"gtl_core_image.h\"\t\t/* Quantix Pat-Lib header file */" << endl;
                continue;
            }
        }

        // If line starts with 'test ' and includes a ';' after it, then replace string...
        i1 = strString.indexOf("test ");
        i2 = strString.indexOf("test\t");
        if(i1 >= 0 || i2 >= 0)
        {
            if(i1 >= 0 && i2 >= 0)
                iIndex = gex_min(i1,i2);	// locates the first occurance of 'test + white space'
            else
            if(i1 > 0)
                iIndex = i1;			// Only 'test' + <tab> detected
            else
                iIndex = i2;			// Only 'test' + <space> detected
            // string 'test' exists in text...then see if a leading ';' exists
            i1 = strString.indexOf(';',iIndex);
            if(i1 >= 0)
            {
                // Yes...then let's make sure it is not a comment or other text.
                strTest = strString.trimmed();
                if(strTest.startsWith("test"))
                {
                    // Replace ';' with ')' as the macro holds the ';'
                    strString[i1]=')';

                    // Replace the 'test' keyword with the PAT-Lib keyword
                    strTest = strString.left(iIndex);	// All string before the 'test' text
                    strTest += "GTL_TEST(";
                    strTest += strString.mid(iIndex+4);	// All string after the 'test' text
                    hOutFile << strTest << endl;
                    continue;
                }
            }
            goto copy_line;
        }

        // If line starts with 'sort bin ' and includes a ';' after it, then replace string...with GTL_SORT or GTL_SORT_NB
        i1 = strString.indexOf("sort ");
        i2 = strString.indexOf("sort\t");
        if(i1 >= 0 || i2 >= 0)
        {
            if(i1 >= 0 && i2 >= 0)
                iIndex = gex_min(i1,i2);	// locates the first occurance of 'sort + white space'
            else
            if(i1 > 0)
                iIndex = i1;			// Only 'sort' + <tab> detected
            else
                iIndex = i2;			// Only 'sort' + <space> detected

            i1 = strString.indexOf("bin",iIndex);
            if(i1 <= 0)
                goto copy_line;	// Not a 'sort bin' instruction

            // Get all text before the 'sort bin'
            strTest = strString.left(iIndex);	// All string before the 'sort' text

            // Check if 'sort bin;' or 'sort bin=xxx;'
            iIndex = strString.indexOf('=');
            if(iIndex > 0)
                bSortBinNbr = true;
            else
                bSortBinNbr = false;

            if(bSortBinNbr)
            {
                strTest += "GTL_SORT_NB(";		// replaces the 'sort bin=xxx;'
                strTest += strString.mid(iIndex+1);	// All string after the 'sort bin=' text
            }
            else
            {
                strTest += "GTL_SORT(";			// replaces the 'sort bin;'
                strTest += strString.mid(i1+3);	// All string after the 'sort bin' text
            }

            // Sort bin macro must not end with a ';'
            i1 = strTest.indexOf(';');
            if(i1 >= 0)
            {
                // Replace ';' with ')' as the macro holds the ';'
                strTest[i1]=')';
            }

            hOutFile << strTest << endl;
            continue;
        }

        // Detect if array defining hardbins (line: 'device_bins =')
        i1 = strString.indexOf("device_bins ");
        i2 = strString.indexOf("device_bins\t");
        if(i1 >= 0 || i2 >= 0)
        {
            if(i1 >= 0 && i2 >= 0)
                iIndex = gex_min(i1,i2);	// locates the first occurance of 'device_bins + white space'
            else
            if(i1 > 0)
                iIndex = i1;			// Only 'device_bins' + <tab> detected
            else
                iIndex = i2;			// Only 'device_bins' + <space> detected
            // string 'device_bins' exists in text...then see if a leading '=' exists
            i1 = strString.indexOf('=',iIndex);
            if(i1 >= 0)
            {
                // Yes...then let's make sure it is not a comment or other text.
                strTest = strString.trimmed();
                if(strTest.startsWith("device_bins"))
                {
                    // Copy lines until end of array (ie: finding '}' character)
                    hOutFile << strString << endl;
                    do
                    {
                        // Get next line
                        strString = hInFile.readLine();
                        if(hInFile.atEnd())
                            break;	// unexpected end of file!

                        // Check if end of array
                        if(strString.indexOf('}') >= 0)
                        {
                            // Insert line that includes the PAT hardbin
                            hOutFile << "141 \"DPAT_REJECT\"		   	hbin =  4      	fail," << endl;
                            // Closes hadbin structure
                            hOutFile << strString << endl;
                            continue;
                        }
                        else
                            hOutFile << strString << endl;
                    }
                    while(1);

                    // Replace the 'test' keyword with the PAT-Lib keyword
                    strTest = strString.left(iIndex);	// All string before the 'test' text
                    strTest += "GTL_TEST(";
                    strTest += strString.mid(iIndex+4);	// All string after the 'test' text
                    hOutFile << strTest << endl;
                    continue;
                }
            }
            goto copy_line;
        }

        // Detect if finding function 'tl_user_endlot'
        i1 = strString.indexOf("void");
        i2 = strString.indexOf("tl_user_endlot",i1);
        if(i1 >= 0 && i2 >= 0 && i2 > i1)
        {
            // Need to find the '( )' after the function...
            i1 = strString.indexOf("(",i2);
            i2 = strString.indexOf(")",i1);
            if(i1 >= 0 && i2 >= 0 && i1 < i2)
            {
                // Yes this is the fuction we're looking for!
                bEndLotFunction = true;

                // Copy original line into new file
                hOutFile << strString << endl;

                i1 = strString.indexOf("{",i2);
                if(i1 < 0)
                {
                    // Function bracket is on next line, need to read it!
                    do
                    {
                        strString = hInFile.readLine();
                        hOutFile << strString << endl;
                        if(hInFile.atEnd())
                            break;	// Unexpected end of file...
                    }
                    while(strString.indexOf("{") < 0);
                }

                // Write our Quantix code!
                hOutFile << endl;
                hOutFile << "  /* Calling Quantix Pat-Lib */" << endl;
                hOutFile << "  GTL_ENDLOT();" << endl << endl;
                continue;
            }
        }

        // Detect if finding function 'tl_user_startup'
        i1 = strString.indexOf("void");
        i2 = strString.indexOf("tl_user_startup",i1);
        if(i1 >= 0 && i2 >= 0 && i2 > i1)
        {
            // Need to find the '( xxxxx )' after the function...
            i1 = strString.indexOf("(",i2);
            i2 = strString.indexOf(")",i1);
            if(i1 >= 0 && i2 >= 0 && i1 < i2)
            {
                // Yes this is the fuction we're looking for!
                bBeginJobFunction = true;

                // Copy original line into new file
                hOutFile << strString << endl;

                i1 = strString.indexOf("{",i2);
                if(i1 < 0)
                {
                    // Function bracket is on next line, need to read it!
                    do
                    {
                        strString = hInFile.readLine();
                        hOutFile << strString << endl;
                        if(hInFile.atEnd())
                            break;	// Unexpected end of file...
                    }
                    while(strString.indexOf("{") < 0);
                }

                // Write our Quantix code!
                hOutFile << endl;
                hOutFile << "  /* Calling Quantix Pat-Lib */" << endl;
                hOutFile << "  GTL_BEGINJOB(&argc,argv);" << endl << endl;
                continue;
            }
        }

        // Detect if finding function 'tl_user_startup'
        i1 = strString.indexOf("void");
        i2 = strString.indexOf("tl_user_shutdown",i1);
        if(i1 >= 0 && i2 >= 0 && i2 > i1)
        {
            // Need to find the '( xxxxx )' after the function...
            i1 = strString.indexOf("(",i2);
            i2 = strString.indexOf(")",i1);
            if(i1 >= 0 && i2 >= 0 && i1 < i2)
            {
                // Yes this is the fuction we're looking for!
                bEndJobFunction = true;

                // Copy original line into new file
                hOutFile << strString << endl;

                i1 = strString.indexOf("{",i2);
                if(i1 < 0)
                {
                    // Function bracket is on next line, need to read it!
                    do
                    {
                        strString = hInFile.readLine();
                        hOutFile << strString << endl;
                        if(hInFile.atEnd())
                            break;	// Unexpected end of file...
                    }
                    while(strString.indexOf("{") < 0);
                }

                // Write our Quantix code!
                hOutFile << endl;
                hOutFile << "  /* Calling Quantix Pat-Lib */" << endl;
                hOutFile << "  GTL_ENDJOB();"  << endl << endl;
                continue;
            }
        }

copy_line:
        // Copy modified line into new file
        hOutFile << strString << endl;
    }
    while(hInFile.atEnd() == false);

    // Add Image functions that need to be present and that are missing:
    if(!bEndLotFunction)
    {
        hOutFile << endl;
        hOutFile << "/*******************************************************/" << endl;
        hOutFile << "void tl_user_endlot()" << endl;
        hOutFile << "{" << endl;
        hOutFile << "  /* Calling Quantix Pat-Lib */" << endl;
        hOutFile << "  GTL_ENDLOT();" << endl;
        hOutFile << "}" << endl << endl;
    }

    // Add Image functions that need to be present and that are missing:
    if(!bBeginJobFunction)
    {
        hOutFile << endl;
        hOutFile << "/*******************************************************/" << endl;
        hOutFile << "void tl_user_startup(int argc , char *argv[])" << endl;
        hOutFile << "{" << endl;
        hOutFile << "  /* Calling Quantix Pat-Lib */" << endl;
        hOutFile << "  GTL_BEGINJOB(&argc,argv);" << endl;
        hOutFile << "}" << endl << endl;
    }

    // Add Image functions that need to be present and that are missing:
    if(!bEndJobFunction)
    {
        hOutFile << endl;
        hOutFile << "/*******************************************************/" << endl;
        hOutFile << "void tl_user_shutdown()" << endl;
        hOutFile << "{" << endl;
        hOutFile << "  /* Calling Quantix Pat-Lib */" << endl;
        hOutFile << "  GTL_ENDJOB();"  << endl;
        hOutFile << "}" << endl << endl;
    }

    // Add gtl_baseline_restart() function
    hOutFile << endl;
    hOutFile << "/*******************************************************/" << endl;
    hOutFile << "void gtl_baseline_restart()" << endl;
    hOutFile << "{" << endl;
    hOutFile << "}" << endl << endl;

    // Close files
    cInput.close();
    cOutput.close();

    // Erase original test program
    GS::Gex::Engine::RemoveFileFromDisk(strTestProgram);

    // Rename modified program to default name
    QDir cDir;
    if(cDir.rename(strNewFile,strTestProgram) == false)
    {
        strErrorMessage = "Failed to rename new program file (disk protected?)";
        return false;
    }

    // Copy libraries from GEX folder into destination folder
    QString		strFrom,strTo;
    QString		strFromPath,strToPath;
    strFromPath = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    strFromPath += "/patman/lib";

    QFileInfo cFileInfo(strTestProgram);
    strToPath = cFileInfo.absolutePath();

    // Copy .H file
    strFrom = strFromPath + "/gtl_core_image.h";
    strTo   = strToPath  + "/gtl_core_image.h";
    if(CGexSystemUtils::CopyFile(strFrom,strTo) == false)
    {
        strErrorMessage = "Failed to copy PATMAN libraries";
        return false;
    }

    // Copy .conf file
    strFrom = strFromPath + "/gtl_tester.conf";
    strTo   = strToPath  + "/gtl_tester.conf";
    if(CGexSystemUtils::CopyFile(strFrom,strTo) == false)
    {
        strErrorMessage = "Failed to copy PATMAN .conf file";
        return false;
    }

    //.a files to copy in <source file>/sun4V/ if folder exists, otherwise copy into <source file>/
    QString strSun4V;
    strSun4V = strToPath + "/sun4V";
    if(QFile::exists(strSun4V))
        strToPath = strSun4V;

    // Copy .a library files
    strFrom = strFromPath + "/gtl_core_image.a";
    strTo   = strToPath  + "/gtl_core_image.a";
    if(CGexSystemUtils::CopyFile(strFrom,strTo) == false)
    {
        strErrorMessage = "Failed to copy PATMAN libraries";
        return false;
    }

    strFrom = strFromPath + "/gtl_tester_image.a";
    strTo   = strToPath  + "/gtl_tester_image.a";
    if(CGexSystemUtils::CopyFile(strFrom,strTo) == false)
    {
        strErrorMessage = "Failed to copy PATMAN libraries";
        return false;
    }

    // Success
    return true;
}

///////////////////////////////////////////////////////////
// Convert a test program to be PAT ready:
// File type: Image A5xx .load project file
///////////////////////////////////////////////////////////
bool GexTbToolBox::ConvertProgramForPat_Image_Load(QString strProjectFile,QString &strErrorMessage)
{
    // Make a copy of the original file (<file name>.bak)
    QString strBackupCopy = strProjectFile + ".bak";
    GS::Gex::Engine::RemoveFileFromDisk(strBackupCopy);
    if(CGexSystemUtils::CopyFile(strProjectFile,strBackupCopy) == false)
    {
        strErrorMessage = "Failed to create a backup copy";
        return false;
    }

    // Open file and insert all PAT keywords at the right place.
    QString strNewFile = strProjectFile + ".pattmp";
    QFile	cInput(strProjectFile);
    QFile	cOutput(strNewFile);

    // Open source in read mode
    if(!cInput.open(QIODevice::ReadOnly))
    {
        strErrorMessage = "Failed to open program file (file locked?)";
        return false;
    }

    // Create new program file
    if(!cOutput.open(QIODevice::WriteOnly))
    {
        strErrorMessage = "Failed to create new file (disk protected?)";
        return false;
    }

    // Assign file I/O stream
    QTextStream hOutFile;
    QTextStream hInFile;
    hInFile.setDevice(&cInput);	// Original test program
    hOutFile.setDevice(&cOutput);	// Modified version to create

    QString	strString;
    QString	strTest;
    do
    {
        // Read line.
        strString = hInFile.readLine();

        // Copy modified line into new file
        hOutFile << strString << endl;
    }
    while(hInFile.atEnd() == false);

    // Add Quantix library functions
    hOutFile << endl;
    hOutFile << "#*******************************************************" << endl;
    hOutFile << "# Quantix Library" << endl;
    hOutFile << "gtl_core_image.a" << endl;
    hOutFile << "gtl_core_image.h" << endl;
    hOutFile << "gtl_tester_image.a" << endl;
    hOutFile << "gtl_tester.conf" << endl;

    // Close files
    cInput.close();
    cOutput.close();

    // Erase original .load project
    GS::Gex::Engine::RemoveFileFromDisk(strProjectFile);

    // Rename modified project to default name
    QDir cDir;
    if(cDir.rename(strNewFile,strProjectFile) == false)
    {
        strErrorMessage = "Failed to rename .load project file (disk protected?)";
        return false;
    }

    // Success
    return true;
}

///////////////////////////////////////////////////////////
// Convert a test program to be PAT ready (include macros,...)
///////////////////////////////////////////////////////////
bool GexTbToolBox::ConvertProgramForPat(QString strFile,QString &strErrorMessage)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in Evaluation mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

    // Check if Teradyne A5xx .tl test program
    if(strFile.endsWith(".tl") == true)
        return ConvertProgramForPat_Image_TL(strFile,strErrorMessage);
    else
    // Check if .load of Teradyne Image test program
    if(strFile.endsWith(".load") == true)
        return ConvertProgramForPat_Image_Load(strFile,strErrorMessage);

    // Unknown file type
    strErrorMessage = "File type not supported";
    return false;
}

void GexTbToolBox::ConvertFiles(QStringList &sFilesSelected)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in Evaluation mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Empty Status TreeWidget, and resize columns to a small size that will be overwritten as we fill the TreeWidget!
    treeWidget->clear();
    treeWidget->setColumnWidth(0, 60);
    treeWidget->setColumnWidth(1, 60);
    treeWidget->setColumnWidth(2, 60);
    treeWidget->setColumnWidth(3, 60);

    // Convert Each file selected...
    GS::Gex::ConvertToSTDF StdfConvert;	// To Convert to STDF
    QString         strStdf,strDestination;
    QStringList     lstFileNameStdf; // for Multi files generation with Convert
    QString         strErrorMessage;
    QString         strStatus;
    QStringList::Iterator it;
    bool	bFileCreated,bResult;
    int		nConvertStatus;
    QTreeWidgetItem *pTreeWidgetItem;
    QStringList lstRowData;
    QDir cDir;
    int	iStatus;

    // Progress bar...
    QProgressDialog progress( "Processing files...", "Abort", 0,
                              sFilesSelected.count(), this);
    if(sFilesSelected.count() > 1)
    {
        progress.setModal(true);
        progress.show();
    }

    // Read the list of files selected, and add them to the database
    int	iIndex=0;
    for (it = sFilesSelected.begin(); it != sFilesSelected.end(); ++it )
    {
        // Update progress bar
        progress.setValue( iIndex );
        QCoreApplication::processEvents();

        // Index update
        iIndex++;

        if(progress.wasCanceled())
            break;

        switch(sToolBoxTask)
        {
        case GEXTB_CONVERT_STDF_WIZARD_P1:
            // Convert files to STDF...
            strDestination = (*it) + ".gextb.std";
            lstFileNameStdf.clear();
            lstFileNameStdf.append(strDestination);
            strErrorMessage = "";
            nConvertStatus = StdfConvert.Convert(false,true,false, true,
                                                 *it, lstFileNameStdf,"",bFileCreated,strErrorMessage);
            if(nConvertStatus == GS::Gex::ConvertToSTDF::eConvertError)
            {
                // If conversion failed...
                strStatus = "*FAILED*";
                cDir.remove(strDestination);	// Erase destination in case partial output create.
                strDestination = " - ";
                for (int i = 1; i < lstFileNameStdf.count(); i++)
                    cDir.remove(lstFileNameStdf[i]);	// Erase destination in case partial output create.
            }
            else
            {
                // Success converting to STDF
                strStatus = "Ok";
                if(bFileCreated == false)
                    strDestination = *it;	// Input file was already in STDF format!
                else
                    strDestination = lstFileNameStdf.first();

                strErrorMessage.replace( "\n", ";" );
                for (int i = 0; i < lstFileNameStdf.count(); i++)
                {
                    // Check we succeed reading STDF FAR record.
                    GS::StdLib::Stdf cStdf;
                    GS::StdLib::StdfRecordReadInfo ReadInfo;

                    iStatus = cStdf.Open(lstFileNameStdf[i].toLatin1().constData(),STDF_READ,1000000L);
                    if(iStatus == GS::StdLib::Stdf::NoError)
                        iStatus = cStdf.LoadRecord(&ReadInfo);
                    if(iStatus != GS::StdLib::Stdf::NoError)
                    {
                        // Error. Can't open STDF file in read mode!
                        strStatus = "*FAILED* File corrupted or unknown format (check extension)";
                        cStdf.Close();	// Clean close.
                    }
                    // If have more than one file generated
                    if(i>0)
                    {
                        lstRowData.clear();
                        lstRowData << strStatus << *it << lstFileNameStdf[i] << strErrorMessage;
                        pTreeWidgetItem = new QTreeWidgetItem(treeWidget, lstRowData);
                        for (int j = 0; j < pTreeWidgetItem->columnCount(); j++)
                            treeWidget->resizeColumnToContents(j);
                    }
                }
            }
            break;

        case GEXTB_CONVERT_CSV_WIZARD_P1:

            // If 'split per 256 rows' option enabled, then split file if it needs to be split...
            if (ReportOptions.GetOption("toolbox", "csv_split_export").toString() == "true")
            {
                // Load data file into CSV Editor page...
                LoadExcelTable(*it);

                // Save to disk (split files so maximum row# is 256 per file)
                strDestination = (*it) + ".gextb.csv";
                bResult = SaveTableToFile(strDestination,true,false,true);
                if(bResult == false)
                {
                    // If conversion failed...
                    strStatus = "*FAILED*";
                    strDestination = " - ";
                }
                else
                    strStatus = "Ok";
            }
            else
            {
                // First: Convert to STDF
                strStdf = (*it) + ".gextb.std";
                strErrorMessage = "";
                nConvertStatus = StdfConvert.Convert(false,true,false, true,
                                                     *it, strStdf,"",bFileCreated,strErrorMessage);
                if(nConvertStatus == GS::Gex::ConvertToSTDF::eConvertError)
                {
                    // If conversion failed...
                    strStatus = "*FAILED*";
                    strDestination = " - ";
                }
                else
                {
                    if(bFileCreated == false)
                        strStdf = *it;	// Input file was already in STDF format!

                    // Success creating intermediate STDF file...then now convert to CSV!
                    strDestination = (strStdf) + ".gextb.csv";
                    cDir.remove(strDestination);	// Make sure destination doesn't exist.

                    CSTDFtoCSV      CsvExport;			// To Convert to CSV
                    CsvExport.setProgressDialog(&progress, iIndex, sFilesSelected.count());
                    bResult = CsvExport.Convert(strStdf, strDestination);
                    if(bResult == false)
                    {
                        if(progress.wasCanceled())
                            strStatus = "*FAILED* (Aborted)";
                        else
                            // If conversion failed...
                            strStatus = "*FAILED*";

                        strDestination = " - ";
                        // Get error message.
                        strErrorMessage = CsvExport.GetLastError();
                        cDir.remove(strDestination);	// Erase destination in case partial output create.
                    }
                    else
                        strStatus = "Ok";
                }

                // Last: Erase intermediate STDF files created.
                if(bFileCreated == true)
                    cDir.remove(strStdf);
            }
            break;

        case GEXTB_CONVERT_PAT_WIZARD_P1:
            // Convert files for real-time PAT...
            strDestination = *it;
            strErrorMessage = "";
            bResult = ConvertProgramForPat(strDestination,strErrorMessage);
            if(bResult == false)
            {
                // If conversion failed...
                strStatus = "*FAILED*";
                strDestination = " - ";
            }
            else
            {
                // Success converting to STDF
                strStatus = "Ok";
                if(bFileCreated == false)
                    strDestination = *it;	// Input file was already in STDF format!
            }
            break;
        }
        // In case of Error message, replace the '\n' with ';' characters.
        strErrorMessage.replace( "\n", ";" );

        lstRowData.clear();
        lstRowData << strStatus << *it << strDestination << strErrorMessage;
        pTreeWidgetItem = new QTreeWidgetItem(treeWidget, lstRowData);
        for (int i = 0; i < pTreeWidgetItem->columnCount(); i++)
            treeWidget->resizeColumnToContents(i);
    }
}

///////////////////////////////////////////////////////////
// Load data file into the table.
///////////////////////////////////////////////////////////
void GexTbToolBox::LoadExcelTable(QString strCsvFile/*=""*/)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Toolbox Load '%1'").arg(strCsvFile).toLatin1().data() );

    if (GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
        QMessageBox::information(this, tr("No feature"),
                                 tr("error : Your licence doesn't allow this function" ));
        return;
    }

    bool	bFileCreated=false, bResult;
    QString strStdf;
    QString strDestination;
    QString	strErrorMessage;
    GS::Gex::ConvertToSTDF StdfConvert;	// To Convert to STDF
    CSTDFtoCSV CsvExport;			// To Convert to CSV
    QDir	cDir;
    QLabel	*pLabel;

    // Inhibit the capture of any signal until the end of loading of spreadsheet table
    mExcelTable->blockSignals(true);

    // If file name specified in argument, it overwrites current default file name.
    if(strCsvFile.isEmpty() == false)
        m_strEditFile = strCsvFile;

    // Make sure the Edit box is hidden + cleared
    lineEdit->hide();
    lineEdit->clear();
    mExcelTable->clearSelection();	// Make sure no cell is selected.
    ParameterTitle->setText("");

    QProgressDialog progress("Title" , "Abort", 0, 100, this);
    progress.setModal(true);
    pLabel = new QLabel("Checking file compliance...", this);
    progress.setLabel(pLabel);
    progress.setValue(0);
    QCoreApplication::processEvents();
    progress.show();

    // First: Convert to STDF then CSV unless it is already a CSV file!
    // If FileName to process is not a STDF file, convert it first!
    // If CSV file, keep it...other wise: Convert a NON-CSV file to STDF then CSV-Examinator compliant!
    if(m_strEditFile.endsWith(".csv", Qt::CaseInsensitive) == true)
    {
        // CSV file to import: no pre-processing required
        strDestination = m_strEditFile;
    }
    else
    {
        // Non-CSV file: convert it to STDF then Examinator-CSV format.
        pLabel->setText("Preprocessing data file...");
        QCoreApplication::processEvents();

        strStdf = m_strEditFile + "_galaxy.std";
        int nConvertStatus = StdfConvert.Convert(false, true, false, true, m_strEditFile, strStdf
                                                 ,"",bFileCreated,strErrorMessage);
        GSLOG(SYSLOG_SEV_NOTICE, QString("Convertion returned %1").arg(nConvertStatus).toLatin1().data());

        if(nConvertStatus == GS::Gex::ConvertToSTDF::eConvertError)
        {
            progress.hide();
            GS::Gex::Message::information( "",
                "Failed importing data file: "+strErrorMessage
                + "\nContact Quantix support at " + QString(GEX_EMAIL_SUPPORT) );
            // Reactivate the capture of signal before aborting the function
            mExcelTable->blockSignals(false);
            return;
        }

        if(nConvertStatus == GS::Gex::ConvertToSTDF::eConvertWarning)
        {
            GS::Gex::Message::information("", strErrorMessage);
        }

        if(progress.wasCanceled())
        {
            // Reactivate the capture of signal before aborting the function
            mExcelTable->blockSignals(false);
            return;	// User abort
        }
        progress.setValue(33);	// 33% done
        pLabel->setText("Reading data file...");
        QCoreApplication::processEvents();

        if(bFileCreated == false)
            strStdf = m_strEditFile;	// Input file was already in STDF format!

        // Success creating intermediate STDF file...then now convert to CSV!
        strDestination = strStdf + ".gextb.csv";
        if (QFile::exists(strDestination))
        {
            strDestination = strStdf + ".temp.gextb.csv";
            cDir.remove(strDestination);	// Make sure destination doesn't exist.
        }
        // Some time, the file has been already created in a previous flow, so don't delete it automatically
        //cDir.remove(strDestination);	// Make sure destination doesn't exist.
        bResult = CsvExport.Convert(strStdf, strDestination);
        if(bResult == false)
        {
            // If conversion failed...
            progress.hide();
            GS::Gex::Message::information(
                "", "Failed importing data file:\n "+CsvExport.GetLastError()
                    + ".\nContact Quantix support at " + QString(GEX_EMAIL_SUPPORT));
            cDir.remove(strDestination);	// Erase destination in case partial output create.
            return;
        }
    }

    if(progress.wasCanceled())
    {
        // Reactivate the capture of signal before aborting the function
        mExcelTable->blockSignals(false);
        return;	// User abort
    }
    progress.setValue(66);	// 66% done
    QString strMessage = "Loading data into table...";
    pLabel->setText(strMessage);
    QCoreApplication::processEvents();

    // Last: Erase intermediate STDF files created.
    if(bFileCreated == true)
        cDir.remove(strStdf);

    // Load CSV file created into the Table.
    QStringList strCells;
    QString strString;
    long	lLine=0;
    lEditorRows=1;	// At startup, allocates 1 to force reset of all other cells (was 50 lines.)
    lEditorCols=1;	// At startup, allocates 1to force reset of all other cells (was 15 cols.)

    mExcelTable->setRowCount(lEditorRows);
    mExcelTable->setColumnCount(lEditorCols);

    // Open CSV file
    QFile f( strDestination );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        progress.hide();
        GS::Gex::Message::information(
            "", "Failed reading data file\nDisk full, or access violation?");
        // Reactivate the capture of signal before aborting the function
        mExcelTable->blockSignals(false);
        return;
    }

    // Assign file I/O stream
    QTextStream hCsvFile(&f);
    long	lFileSize = f.size();
    float	fPercentage=-1,fNewPercentage;
    mLineTestNumberParameter=-1;		// Holds the line# that starts with 'parameter'...in case we have a Examinator-CSV compliant file.
    long	lCount;
    int		iIndex;
    bool	bMultilineString = false;
    bool	bShowLimitMessage = true;
    do
    {
        // Update Process bar
        fNewPercentage = (100.0 * f.pos())/lFileSize;
        if(fNewPercentage != fPercentage)
        {
            fPercentage = fNewPercentage;
            progress.setValue(66 + (((int)fNewPercentage)/3));	// 66% done
            QString strNewTitle = strMessage + QString::number(lLine) + " lines";
            pLabel->setText(strNewTitle);
            if(progress.wasCanceled())
            {
                // Reactivate the capture of signal before aborting the function
                mExcelTable->blockSignals(false);
                return;	// User abort
            }
        }

        // Extract all lines, and load them into the Table.
        strString = hCsvFile.readLine().trimmed();

        strCells.clear();

        // Extract the N column names
        if(!bMultilineString && strString.startsWith("\""))
        {
            if(strString.endsWith("\""))
                strCells << strString.mid(1, strString.length()-2);
            else
            {
                strCells << strString.right(strString.length()-1);
                bMultilineString = true;
            }
        }
        else if(bMultilineString)
        {
            if(strString.endsWith("\""))
            {
                strCells << strString.left(strString.length()-1);
                bMultilineString = false;
            }
            else
                strCells << strString;
        }
        else
            strCells << strString.split(",",QString::KeepEmptyParts);

        lCount = strCells.count();
        // Ignore last columns that are empty.
        while(lCount > 0 && strCells[lCount-1].isEmpty() == true)
            lCount--;

        if(lCount > TB_IMPORT_CELL_COUNT_LIMIT )
        {
            if(bShowLimitMessage)
            {
                GS::Gex::Message::information(
                    "", QString("The csv import will be limited to the first %1 "
                                "columns since the file contains more than %1 "
                                "columns").arg(TB_IMPORT_CELL_COUNT_LIMIT));
                bShowLimitMessage = false;
            }
            lCount = TB_IMPORT_CELL_COUNT_LIMIT;
            strCells.erase(strCells.begin()+TB_IMPORT_CELL_COUNT_LIMIT +1, strCells.end());
        }

        // If need be resize the columns count
        if(lEditorCols < lCount )
        {
            lEditorCols = lCount;
            mExcelTable->setColumnCount(lEditorCols);
        }

        iIndex=0;
        for(QStringList::iterator it = strCells.begin(); iIndex < lCount; it++)
        {
            // Keep track of line that holds the parameters (in cas ewe have a Examinator-CSV compliant file)
            if(mLineTestNumberParameter < 0 && iIndex == 0 && (*it).toLower() == "parameter")
                mLineTestNumberParameter = lLine;

            // Keep track of
            if(mLineTestNumberParameter >= 0 && iIndex == 0 && (*it).toLower() == "patterns")
                m_bLinePatternsExists = true;

//            bool isBloc = mExcelTable->signalsBlocked();
            if (NULL == mExcelTable->item(lLine,iIndex))
                mExcelTable->setItem(lLine,iIndex, new QTableWidgetItem());
            mExcelTable->item(lLine,iIndex)->setText(*it);
            iIndex++;
//            isBloc = true;
        }
        // Fill leading empty cells with empty strings: makes sure we overwrite any previous data!
        while(iIndex < lEditorCols)
        {
            if (NULL == mExcelTable->item(lLine,iIndex))
                mExcelTable->setItem(lLine,iIndex, new QTableWidgetItem());
            mExcelTable->item(lLine,iIndex++)->setText("");
        }

        // Make line visible (in case it was hidden/deleted during a previous session)
        mExcelTable->showRow(lLine);

        // Update line#
        lLine++;
        if(lLine > TB_IMPORT_CELL_COUNT_LIMIT){
            GS::Gex::Message::information(
                "", QString("The csv import will be limited to the first %1 lines "
                            "since the file contains more than %1 lines").
                arg(TB_IMPORT_CELL_COUNT_LIMIT));
            break;
        }
        if(lLine >= lEditorRows)
        {
            // Need to resize table...add 100 more lines!
            lEditorRows += 100;
            mExcelTable->setRowCount(lEditorRows);
        }
    }
    while(hCsvFile.atEnd() == false);

    // Close file
    f.close();

    // At load time, table not pivoted (yet).
    sPivoted=false;

    // Set the total exact number of lines to display
    mLastTestNumberParameterLine = lEditorRows = lLine;	// Total lines (Rows) in the editor table
    mExcelTable->setRowCount(lEditorRows);

    if (m_bLinePatternsExists == true)
        mNbParamHeader = NEW_PARAMETER_NUMBER;
    else
        mNbParamHeader = OLD_PARAMETER_NUMBER;

    mFirstTestNumberParameterLine = mLineTestNumberParameter + mNbParamHeader;

    // Check if the file imported is a Examinator CSV compliant file. And if so, update table Header titles
    UpdateHeaderTitles();

    // Remove CSV file (if was created internally by the application)
    if(m_strEditFile != strDestination)
        cDir.remove(strDestination);

    // Set the backgroud of cells
    for (int row=0; row<=mExcelTable->rowCount(); ++row)
    {
        for (int col=0; col<=mExcelTable->columnCount(); ++col)
        {
            // don't process for empty cell
            if (mExcelTable->item(row, col))
                mExcelTable->paintCell(row, col);
        }
    }

    // Clear variables
    m_bTableModified = false;

    buttonProperties->setEnabled(true);

    // Enable the capture of signals
    mExcelTable->blockSignals(false);
}

///////////////////////////////////////////////////////////
// Update table lines/columns titles.
///////////////////////////////////////////////////////////
void	GexTbToolBox::UpdateHeaderTitles(void)
{
    if(mExcelTable->isVisible() == false)
        return;

    mExcelTable->blockSignals(true);
    if(sPivoted)
    {
        if((mExcelTable->item(mLineTestNumberParameter,1)->text()).toLower() == "tests#" &&
            mExcelTable->item(mLineTestNumberParameter,0)->text().toLower() == "parameter" &&
            mExcelTable->item(mLineTestNumberParameter+1,0)->text().trimmed() == "SBIN" &&
            mExcelTable->item(mLineTestNumberParameter+2,0)->text().trimmed() == "HBIN" &&
            mExcelTable->item(mLineTestNumberParameter+3,0)->text().trimmed() == "DIE_X" &&
            mExcelTable->item(mLineTestNumberParameter+4,0)->text().trimmed() == "DIE_Y" &&
            mExcelTable->item(mLineTestNumberParameter+5,0)->text().trimmed() == "SITE" &&
            mExcelTable->item(mLineTestNumberParameter+6,0)->text().trimmed() == "TIME")
        {
            // Format compliant with CSV file created by Examinator (Edited data file)
            mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter,new QTableWidgetItem(tr("Part ID")));
            mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+1,new QTableWidgetItem(tr("Soft Bin")));
            mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+2,new QTableWidgetItem(tr("Hard Bin")));
            mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+3,new QTableWidgetItem(tr("Die X")));
            mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+4,new QTableWidgetItem(tr("Die Y")));
            mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+5,new QTableWidgetItem(tr("Site #")));
            mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+6,new QTableWidgetItem(tr("Time")));

            mExcelTable->setHorizontalHeaderItem(0,new QTableWidgetItem(tr("Param.")));
            mExcelTable->setHorizontalHeaderItem(1,new QTableWidgetItem(tr("Test#")));

            // New csv galaxy format
            if (mExcelTable->item(mLineTestNumberParameter,2)->text().toLower() == "patterns")
            {
                mExcelTable->setHorizontalHeaderItem(2,new QTableWidgetItem(tr("Patterns")));
                mExcelTable->setHorizontalHeaderItem(3,new QTableWidgetItem(tr("Unit")));
                mExcelTable->setHorizontalHeaderItem(4,new QTableWidgetItem(tr("HighL")));
                mExcelTable->setHorizontalHeaderItem(5,new QTableWidgetItem(tr("LowL")));
                mExcelTable->setHorizontalHeaderItem(6,new QTableWidgetItem("7"));
            }
            else	// Old csv galaxy format
            {
                mExcelTable->setHorizontalHeaderItem(2,new QTableWidgetItem(tr("Unit")));
                mExcelTable->setHorizontalHeaderItem(3,new QTableWidgetItem(tr("HighL")));
                mExcelTable->setHorizontalHeaderItem(4,new QTableWidgetItem(tr("LowL")));
                mExcelTable->setHorizontalHeaderItem(5,new QTableWidgetItem("6"));
                mExcelTable->setHorizontalHeaderItem(6,new QTableWidgetItem("7"));
            }

            // Enable 'Pivote' button
            buttonPivot->show();
        }
        else
        {
            // Disable 'Pivote' button
            buttonPivot->hide();
        }
    }
    else
    {
        if((mExcelTable->item(mLineTestNumberParameter+1,0)->text()).toLower() == "tests#" &&
            mExcelTable->item(mLineTestNumberParameter,0)->text().toLower() == "parameter" &&
            mExcelTable->item(mLineTestNumberParameter,1)->text().trimmed() == "SBIN" &&
            mExcelTable->item(mLineTestNumberParameter,2)->text().trimmed() == "HBIN" &&
            mExcelTable->item(mLineTestNumberParameter,3)->text().trimmed() == "DIE_X" &&
            mExcelTable->item(mLineTestNumberParameter,4)->text().trimmed() == "DIE_Y" &&
            mExcelTable->item(mLineTestNumberParameter,5)->text().trimmed() == "SITE" &&
            mExcelTable->item(mLineTestNumberParameter,6)->text().trimmed() == "TIME")
        {
            // Format compliant with CSV file created by Examinator (Edited data file)
            mExcelTable->setHorizontalHeaderItem(0,new QTableWidgetItem(tr("Part ID")));
            mExcelTable->setHorizontalHeaderItem(1,new QTableWidgetItem(tr("Soft Bin")));
            mExcelTable->setHorizontalHeaderItem(2,new QTableWidgetItem(tr("Hard Bin")));
            mExcelTable->setHorizontalHeaderItem(3,new QTableWidgetItem(tr("Die X")));
            mExcelTable->setHorizontalHeaderItem(4,new QTableWidgetItem(tr("Die Y")));
            mExcelTable->setHorizontalHeaderItem(5,new QTableWidgetItem(tr("Site #")));
            mExcelTable->setHorizontalHeaderItem(6,new QTableWidgetItem(tr("Time")));

            mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter,new QTableWidgetItem(tr("Param.")));
            mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+1,new QTableWidgetItem(tr("Test#")));

            // New csv galaxy format
            if (mExcelTable->item(mLineTestNumberParameter+2,0)->text().toLower() == "patterns")
            {
                mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+2,new QTableWidgetItem(tr("Patterns")));
                mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+3,new QTableWidgetItem(tr("Unit")));
                mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+4,new QTableWidgetItem(tr("HighL")));
                mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+5,new QTableWidgetItem(tr("LowL")));
                mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+6,new QTableWidgetItem(QString::number(mLineTestNumberParameter+7)));
            }
            else // old csv galaxy format
            {
                mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+2,new QTableWidgetItem(tr("Unit")));
                mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+3,new QTableWidgetItem(tr("HighL")));
                mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+4,new QTableWidgetItem(tr("LowL")));
                mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+5,new QTableWidgetItem(QString::number(mLineTestNumberParameter+6)));
                mExcelTable->setVerticalHeaderItem(mLineTestNumberParameter+6,new QTableWidgetItem(QString::number(mLineTestNumberParameter+7)));
            }

            // Enable 'Pivote' button
            buttonPivot->show();
        }
        else
        {
            // Disable 'Pivote' button
            buttonPivot->hide();
        }
    }
    mExcelTable->blockSignals(false);
}

///////////////////////////////////////////////////////////
// Pivote data in CSV file.
///////////////////////////////////////////////////////////
void	GexTbToolBox::OnPivoteTable(void)
{
    if (mLineTestNumberParameter < 0)
        return;

    // If large table, ask user to confirm first!
    QString strTmpText;
    float	fValue = (double)lEditorCols*lEditorRows;
    if(fValue > 20000)
    {
        strTmpText.sprintf("This is a very large table: %.0lf cells\nDo you want to proceed?",fValue);
        bool lOk;
        GS::Gex::Message::request("", strTmpText, lOk);
        if (! lOk)
        {
            return;
        }
    }

    // Toggle pivote status
    sPivoted = (sPivoted)? false:true;

    // Compute total effective cells to swap.
    int	iMaxCols;
    int iMaxRows;
    iMaxCols = lEditorCols;
    iMaxRows = lEditorRows;
    fValue = (double)iMaxCols*iMaxRows;

    // Progress bar...
    strTmpText.sprintf("Pivote table: %.0lf cells",fValue);
    QProgressDialog progress(strTmpText, "Abort", 0, 100, pGexMainWindow);
    progress.setModal(true);
    progress.show();

    QCoreApplication::processEvents();

    // Pivote table (swap lines & columns & headers)
    // 1: avoid paint event
    mExcelTable->setUpdatesEnabled(false);

    // 2: keep data cells in a map(keep header section holding program name, date, etc...)
    int						iLine,lCol;
    QMap<QString, QString>	mapTableItem;

    mExcelTable->blockSignals(true);
    for(lCol=0;lCol < iMaxCols; lCol++)
    {
        // Update progress bar
        fValue = (50.0*lCol/iMaxCols);
        progress.setValue( (int)fValue );
        QCoreApplication::processEvents();

        // Check for abort
        if(progress.wasCanceled())
            break;

        for(iLine = mLineTestNumberParameter; iLine < iMaxRows; iLine++)
        {
            QString strCoord = QString("%1;%2").arg(lCol+mLineTestNumberParameter).arg(iLine-mLineTestNumberParameter);

            if (NULL == mExcelTable->item(iLine, lCol))
                mExcelTable->setItem(iLine, lCol, new QTableWidgetItem());
            mapTableItem.insert(strCoord,  mExcelTable->item(iLine, lCol)->text());
        }
    }

    mExcelTable->setRowCount(mLineTestNumberParameter);
    mExcelTable->setColumnCount(mLineTestNumberParameter);

    // 3: adjust to correct lines & columns + update variables holding table size
    mExcelTable->setRowCount(iMaxCols+mLineTestNumberParameter);
    mExcelTable->setColumnCount(iMaxRows-mLineTestNumberParameter);

    for(lCol=0;lCol < iMaxRows-mLineTestNumberParameter; lCol++)
    {
        for(iLine=0; iLine < mLineTestNumberParameter; iLine++)
        {
            if (lCol > mLineTestNumberParameter)
            {
                if (mExcelTable->item(iLine, lCol) == NULL)
                    mExcelTable->setItem(iLine, lCol, new QTableWidgetItem());
                mExcelTable->item(iLine, lCol)->setText("");
            }
        }

        for(iLine=mLineTestNumberParameter;iLine < iMaxCols+mLineTestNumberParameter; iLine++)
        {
            if (mExcelTable->item(iLine, lCol) == NULL)
                mExcelTable->setItem(iLine, lCol, new QTableWidgetItem());
            mExcelTable->item(iLine, lCol)->setText("");
        }
    }

    // 4: restore data cells
    QMapIterator<QString, QString> itMapTableItem(mapTableItem);
    int									nTableItem = 0;


    while (itMapTableItem.hasNext())
    {
        itMapTableItem.next();

        // Update progress bar
        fValue = 50.0 + ((50.0*nTableItem)/mapTableItem.count());
        progress.setValue( (int)fValue );
        QCoreApplication::processEvents();

        // Check for abort
        if(progress.wasCanceled())
            break;

        QString strCoord	= itMapTableItem.key();
        int		nLine		= strCoord.section(";", 0, 0).toInt();
        int		nCol		= strCoord.section(";", 1, 1).toInt();

        if (mExcelTable->item(nLine, lCol) == NULL)
            mExcelTable->setItem(nLine, lCol, new QTableWidgetItem());

        mExcelTable->item(nLine, nCol)->setText(itMapTableItem.value());
        nTableItem++;
    }

    // The paint in not in the previous loop because it desable the block of signal and
    // the traitement will be so long
    for(lCol=0;lCol < mExcelTable->columnCount(); lCol++)
    {
        for(iLine=0; iLine < mExcelTable->rowCount(); iLine++)
        {
             mExcelTable->paintCell(iLine, lCol);
        }
    }

    lEditorRows = mExcelTable->rowCount();
    lEditorCols = mExcelTable->columnCount();

    // Refresh header texts
    UpdateHeaderTitles();

    mExcelTable->setUpdatesEnabled(true);

    mExcelTable->blockSignals(false);
}

///////////////////////////////////////////////////////////
// Sort table by Part-ID#
///////////////////////////////////////////////////////////
void GexTbToolBox::OnSortByPartID(bool /*bAscending*/)
{
    // Nothing to sort!
    if(mLineTestNumberParameter <= 0)
        return;

    // Perform buble sort
    QString strString;
    long	lPartMajor1,lPartMinor1;
    long	lPartMajor2,lPartMinor2;
    bool	bSwap;
    int		iLine,lCol,iSortLine;
    lPartMajor1 = lPartMinor1 = lPartMajor2 = lPartMinor2 = -1;
    for(iLine = mLineTestNumberParameter; iLine <= mLastTestNumberParameterLine; iLine++)
    {
        // Get PID#1 string
        strString = mExcelTable->item(iLine,0)->text();
        strString = strString.section("PID-",1);	// Extract PID number (eg: XXXX or XX.YYY)
        lPartMinor1 = 0;
        if(sscanf(strString.toLatin1().constData(),"%ld%*c%ld",&lPartMajor1,&lPartMinor1) < 1)
            goto next_csv1_line;

        for(iSortLine = iLine+1 ; iSortLine <= mLastTestNumberParameterLine; iSortLine++)
        {
            // Get PID#2 string
            strString = mExcelTable->item(iSortLine,0)->text();
            strString = strString.section("PID-",1);	// Extract PID number (eg: XXXX or XX.YYY)
            lPartMinor2=0;
            if(sscanf(strString.toLatin1().constData(),"%ld%*c%ld",&lPartMajor2,&lPartMinor2) < 1)
                goto next_csv2_line;

            // Compare...
            if(lPartMajor1 < lPartMajor2)
            {
                // These two part# already sorted!
                bSwap = false;
            }
            else
            if(lPartMajor1 == lPartMajor2)
            {
                // Same major value, check minor values, eg: XXXX.YYYY and XXXX.ZZZZ
                if(lPartMinor1 <= lPartMinor2)
                    bSwap = false;
                else
                    bSwap = true;
            }
            else
                bSwap = true;


            // Swap needed?
            if(bSwap)
            {
                // Swap lines.
                for(lCol = 0;lCol < mExcelTable->columnCount(); lCol++)
                {
                    strString = mExcelTable->item(iSortLine-1,lCol)->text();
                    mExcelTable->item(iSortLine-1,lCol)->setText(mExcelTable->item(iSortLine,lCol)->text());
                    mExcelTable->item(iSortLine,lCol)->setText(strString);
                }
            }

            // Move to netx cell!
            lPartMajor1 = lPartMajor2;
            lPartMinor1 = lPartMinor2;
next_csv2_line:;
        }
next_csv1_line:;
    }
}

///////////////////////////////////////////////////////////
// Called each time the table is modified (requires to be saved to disk)
///////////////////////////////////////////////////////////
void GexTbToolBox::OnTableModified()
{
    m_bTableModified = true;
}



///////////////////////////////////////////////////////////
// Return the text of item if it is not null, else return ""
///////////////////////////////////////////////////////////
QString GexTbToolBox::GetTextFromItem(int row, int col)
{
    QString retString("");
    if (NULL != mExcelTable->item(row, col))
        retString = mExcelTable->item(row, col)->text();
    return retString;
}

///////////////////////////////////////////////////////////
// Show Conversion status for files selected.
///////////////////////////////////////////////////////////
void GexTbToolBox::OnProperties(void)
{
    QString strMessage;
    QString strComment;
    QFile f;
    QTreeWidgetItem *pTreeWidgetItem;

    switch(sToolBoxTask)
    {
        case GEXTB_CONVERT_STDF_WIZARD_P1:
        case GEXTB_CONVERT_CSV_WIZARD_P1:
        case GEXTB_CONVERT_PAT_WIZARD_P1:
            if(sToolBoxTask == GEXTB_CONVERT_STDF_WIZARD_P1)
                strMessage = "* Conversion to STDF: Information *\n";
            else
            if(sToolBoxTask == GEXTB_CONVERT_CSV_WIZARD_P1)
                strMessage = "* Conversion to Excel, MS-Access : Information *\n";
            else
            if(sToolBoxTask == GEXTB_CONVERT_PAT_WIZARD_P1)
                strMessage = "* Modify for real-time PAT : Information *\n";

            // If no item in list, just return!
            if (treeWidget->selectedItems().size() <= 0)
                return;

            pTreeWidgetItem = treeWidget->selectedItems().first();

            // Input file + size
            strMessage += "\nInput: " + pTreeWidgetItem->text(1);
            f.setFileName(pTreeWidgetItem->text(1));
            if(f.exists())
                strMessage += "\nSize: " + QString::number(f.size()) + " Bytes\n";

            // Output file + size
            strMessage += "\nOutput: " + pTreeWidgetItem->text(2);
            f.setFileName(pTreeWidgetItem->text(2));
            if(f.exists())
                strMessage += "\nSize: " + QString::number(f.size()) + " Bytes\n";

            // Conversion status
            strMessage += "\nConversion Status: " + pTreeWidgetItem->text(0);
            strComment = pTreeWidgetItem->text(3);
            strComment.replace( ";","\n");

            // Conversion completion message.
            if(strComment.isEmpty() == false)
            {
                strMessage += "\nComment:\n";
                strMessage += strComment;
            }
            break;

        case GEXTB_EDIT_CSV_WIZARD_P1:
            // File
            // Size
            // Lines
            strMessage = "* Editing Data: Information*\n";

            // Input file + size
            strMessage += "\nFile: " + m_strEditFile;
            f.setFileName(m_strEditFile);
            if(f.exists() == false)
                return;
            strMessage += "\nTable size: " + QString::number(f.size()) + " Bytes";

            // Total# of Rows & lines
            strMessage += "\nTotal Rows : " + QString::number(lEditorRows);
            strMessage += "\nTotal Columns: " + QString::number(lEditorCols);
            break;
    }
    GS::Gex::Message::information("", strMessage);
}

///////////////////////////////////////////////////////////
// Delete the selected lines
///////////////////////////////////////////////////////////
void GexTbToolBox::OnDelete(bool bLines)
{
    // Get the list of al selected element in the table
    QList<QTableWidgetSelectionRange> selectedRegion = mExcelTable->selectedRanges();

    // Check if selection affects ALL the table...
    QString strConfirm;
    if(!selectedRegion.empty()
       && ( (!bLines && selectedRegion.first().columnCount() == mExcelTable->columnCount())
            || (bLines && selectedRegion.first().rowCount() == mExcelTable->rowCount()))
      )
    {
        // Confirm to delete FULL table
        strConfirm = "Do you confirm to delete the FULL table?";
        bool lOk;
        GS::Gex::Message::request("", "strConfirm", lOk);
        if (! lOk)
        {
            return;
        }
    }
    else
    {
        // Confirm to delete some lines/columns?
        if(bLines)
            strConfirm = "Do you confirm to delete the selected rows?";
        else
            strConfirm = "Do you confirm to delete the selected columns?";
        bool lOk;
        GS::Gex::Message::request("", strConfirm, lOk);
        if (! lOk)
        {
            return;
        }
    }

    if(bLines)
    {
        // Delete lines: Hide selected rows
        QList<QTableWidgetSelectionRange>::iterator itTableRange;
        for(itTableRange = selectedRegion.begin(); itTableRange != selectedRegion.end(); ++itTableRange)
        {
            int selectedRow = (*itTableRange).topRow();
            do
            {
                // hide row only if it not begins with "<" because they separte section in some PAT' files
                QString strCell = mExcelTable->item(selectedRow,0)->text();
                if(strCell.startsWith("<") == false)
                    mExcelTable->hideRow(selectedRow);
                ++selectedRow;
            }
            while(selectedRow <= (*itTableRange).bottomRow ());
        }
    }
    else
    {
        // Delete lines: Hide selected columns
        QList<QTableWidgetSelectionRange>::iterator itTableRange;
        for(itTableRange = selectedRegion.begin(); itTableRange != selectedRegion.end(); ++itTableRange)
        {
            int selectedCol = (*itTableRange).leftColumn();
            do
            {
                // hide row only if it not begins with "<" because they separte section in some PAT' files

                QTableWidgetItem* lItem =  mExcelTable->item(selectedCol,0);
                if(lItem)
                {
                    QString strCell = lItem->text();
                    if(strCell.startsWith("<") == false)
                        mExcelTable->hideColumn(selectedCol);
                }
                else
                {
                    mExcelTable->hideColumn(selectedCol);
                }
                ++selectedCol;
            }
            while(selectedCol <= (*itTableRange).rightColumn());
        }
    }

    // Flag table is modified
    OnTableModified();
}


///////////////////////////////////////////////////////////
// Open dialog to select operation and apply it on
// the selected columns
///////////////////////////////////////////////////////////
void GexTbToolBox::onEditTests(int iOpenMode/* = 0*/)
{
    bool isColumnUsed;
    long lStartingProcess;
    if (mLineTestNumberParameter == -1)
        return;
    if (sPivoted) // Work on Row
    {
        lStartingProcess = mNbParamHeader;
        isColumnUsed = false;
    }
    else // Work on Columns
    {
        lStartingProcess = mFirstTestNumberParameterLine;
        isColumnUsed = true;
    }
    ToolBoxCSVOperationsDialog dialOpCVS(*mExcelTable, isColumnUsed, lStartingProcess, mLineTestNumberParameter, iOpenMode);
    dialOpCVS.exec();
}


///////////////////////////////////////////////////////////
// Add a Test before the selected one
///////////////////////////////////////////////////////////
void GexTbToolBox::onAddTestBeforeSelected()
{
    int lSelectedTest;
    if (sPivoted) // Work on Row
    {
        QList<QTableWidgetSelectionRange> selectedRegion = mExcelTable->selectedRanges();
        QList<QTableWidgetSelectionRange>::iterator itTableRange;
        itTableRange = selectedRegion.begin();
        lSelectedTest = (*(mExcelTable->selectedRanges()).begin()).topRow()/*firstSelectedRow()*/;
        mExcelTable->insertRow(lSelectedTest); // Add a row
        ++lEditorRows;
        mExcelTable->setCurrentCell(lSelectedTest, mExcelTable->currentColumn());
        mExcelTable->selectRow(lSelectedTest);
    }
    else
    {
        lSelectedTest = mExcelTable->firstSelectedColumn();
        mExcelTable->insertColumn(lSelectedTest);// Add a column
        ++lEditorCols;
        mExcelTable->setCurrentCell(mExcelTable->currentRow(), lSelectedTest);
        mExcelTable->selectColumn(lSelectedTest);
    }
    onEditTests(1);
}

///////////////////////////////////////////////////////////
// Add a Test after the selected one
///////////////////////////////////////////////////////////
void GexTbToolBox::onAddTestAfterSelected()
{
    int iSelectedTest;
    if (sPivoted) // Work on Row
    {
        iSelectedTest = mExcelTable->firstSelectedRow();
        mExcelTable->insertRow(iSelectedTest + 1); // Add a row
        ++lEditorRows;
        mExcelTable->setCurrentCell(iSelectedTest + 1, mExcelTable->currentColumn());
        mExcelTable->selectRow(iSelectedTest + 1);
    }
    else
    {
        iSelectedTest = mExcelTable->firstSelectedColumn();
        mExcelTable->insertColumn(iSelectedTest + 1);// Add a column
        lEditorCols++;
        mExcelTable->setCurrentCell(mExcelTable->currentRow(), iSelectedTest + 1);
        mExcelTable->selectColumn(iSelectedTest + 1);
    }
    onEditTests(1);
}

///////////////////////////////////////////////////////////
// Save data file (Save SpreadSheet content to disk)
///////////////////////////////////////////////////////////
void GexTbToolBox::OnSaveButton(void)
{

    if (GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
        GS::Gex::Message::information(tr("No feature"), tr("error : Your licence doesn't allow this function" ));
        return;
    }

    if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
            "", "This function is disabled in Evaluation mode\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Let's user tell where to save the table.
    QString strNewName = m_strEditFile;
    QString strFile = QFileDialog::getSaveFileName(this,	// parent
                    "Save Table data as...",				// caption
                    strNewName,								// directory
                    "Spreadsheet CSV(*.csv)",                  // filter
                    NULL,									// selected filter
                    QFileDialog::DontConfirmOverwrite);		// options; comment : internal overwrite management (cf. SaveTableToFile method )

    // If no file selected, ignore command.
    if(strFile.isEmpty())
        return;

    // Check if Excel compliant (less than 250 visible columns)
    bool bSplitFile=false;
    int	lTotalVisibleCols=0;
    for(int lCol=0;lCol<lEditorCols;lCol++)
    {
        if(mExcelTable->isColumnHidden(lCol) == false)
            lTotalVisibleCols++;
    }

    if(lTotalVisibleCols > 256)
    {
        int iSplit = QMessageBox::warning(
            this, GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
            "This table has over 256 columns!\nIf you need to load it under MS-Excel, have Examinator split it into multiple files.\nWhat do you want to do?",
                "Split the table","Keep it as one wide CSV file", "Abort", 2, 2 );
        if(iSplit == 0)
            bSplitFile = true;
        else
        if(iSplit == 1)
            bSplitFile = false;
        else
            return;	// Abort!
    }

    // Save table into file
    SaveTableToFile(strFile,bSplitFile);
}

///////////////////////////////////////////////////////////
// Save data file (Save SpreadSheet content to disk)
///////////////////////////////////////////////////////////
bool GexTbToolBox::SaveTableToFile(QString strFile,bool bSplitFile/*=false*/,bool bAskOverwrite/*=true*/,bool bQuietSuccess/*=false*/)
{
    if (GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
        GS::Gex::Message::information(tr("No feature"), tr("error : Your licence doesn't allow this function" ));
        return false;
    }
    QFile f(strFile);
    // Update default file name we work with.
    m_strEditFile = strFile;		// m_strEditFile is GexTbToolBox attribute

    // Assign file I/O stream
    QTextStream hCsvTableFile;
    QString		strCell;
    QString		strMessage;
    long		lLine,lCol,lMaxCol;

    bool bSaveAllColumns=false;	// Default: only save VISIBLE columns

    if(bSplitFile == false)
    {
        // No split: save all the table into a single file.
        if(bAskOverwrite && (f.exists() == true))
        {
            // File exists...overwrite it?
            bool lOk;
            GS::Gex::Message::request("", "File already exists. Overwrite it ?", lOk);
            if (! lOk)
            {
                return true;
            }
        }

        // Update default file name we work with.
        // Create file
        if(!f.open(QIODevice::WriteOnly))
        {
            // Error. Can't create CSV file!
            GS::Gex::Message::information(
                "", "Failed to create CSV data file\n"
                "Disk full or write protection issue.");
            return false;
        }
        hCsvTableFile.setDevice(&f);

        for(lLine=0;lLine<lEditorRows;lLine++)
        {
            // Ignore all leading empty cells on each line
            lMaxCol = lEditorCols;
            while(lMaxCol > 0)
            {
                if(mExcelTable->item(lLine,lMaxCol-1))
                    strCell = (mExcelTable->item(lLine,lMaxCol-1)->text()).trimmed();
                else
                    strCell = "";
                if(strCell.isEmpty() == false)
                    break;	// Reached the last cell that is not blank.
                lMaxCol--;
            }

            // Write each line...unless hidden!
            if(mExcelTable->isRowHidden(lLine) == false)
            {
                for(lCol=0;lCol<lMaxCol;lCol++)
                {
                    if(bSaveAllColumns || (mExcelTable->isColumnHidden(lCol) == false))
                    {
                        // Insert ',' separator prior to 2nd and higher columns
                        if(lCol)
                            hCsvTableFile << ",";
                        // If cell not hidden, save it to disk.
                        if(mExcelTable->item(lLine,lCol))
                            hCsvTableFile << mExcelTable->item(lLine,lCol)->text();
                        else
                            hCsvTableFile << "";
                    }
                }
                // Add new line character
                hCsvTableFile << endl;
            }
        }
        // Close file.
        f.close();
    }
    else
    {
        // Split table into N files (230 columns per file)
        QString strFileName;
        QString strFilesCreated;
        int iFilesToCreate;
        long lStartSplitCol=0;
        long lMaxSplitCol=0;

        // Compute total visible columns
        int	lTotalVisibleCols=0;
        for(lCol=0;lCol<lEditorCols;lCol++)
        {
            if(mExcelTable->isColumnHidden(lCol) == false)
                lTotalVisibleCols++;
        }
        iFilesToCreate = lTotalVisibleCols/230;	// Number of files to create.

        // Create X files. Each file will hold upto 230 columns.
        for(int iFileIndex=0;iFileIndex <= iFilesToCreate; iFileIndex++)
        {
            // Create file: <file.csv>-split-XXX.csv
            // strFileName=strFile + "-split-" + QString::number(iFileIndex+1) + ".csv";
            strFileName = m_strEditFile + "-split-" + QString::number(iFileIndex+1) + ".csv";
            strFilesCreated += strFileName + "\n";
            f.setFileName(strFileName);

            if(bAskOverwrite && (f.exists() == true))
            {
                // File exists...overwrite it?
                QMessageBox mb;
                mb.setWindowTitle(
                  GS::Gex::Engine::GetInstance().Get("AppFullName").toString());
                mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
                mb.setText("File " + strFileName + " already exists. Overwrite it?");
                mb.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel | QMessageBox::SaveAll);
                mb.setDefaultButton(QMessageBox::Save);

                switch (mb.exec()) {
                case QMessageBox::Save:
                    break;
                case QMessageBox::Cancel:
                    return true;
                case QMessageBox::SaveAll:
                    bAskOverwrite = false;
                    break;
                }
            }

            if(!f.open(QIODevice::WriteOnly))
            {
                // Error. Can't create CSV file!
                GS::Gex::Message::information(
                    "", "Failed to create CSV data file\n"
                    "Disk full or write protection issue.");
                return false;
            }
            hCsvTableFile.setDevice(&f);

            // Last column to save into file
            lMaxSplitCol = lStartSplitCol;
            for(lCol = lStartSplitCol;lCol < lEditorCols && (lMaxSplitCol-lStartSplitCol < 230);lCol++)
            {
                if(bSaveAllColumns || (mExcelTable->isColumnHidden(lCol) == false))
                    lMaxSplitCol++;
            }
            // Last column to save in file.
            lMaxSplitCol = lCol;

            // header section writting, pivoted mode or not pivoted mode
            long lCountHeader ;
            if (sPivoted)
                lCountHeader = mNbParamHeader;
            else
                lCountHeader = CSV_RAW_WAFER_ID +2 ;	// CSV_RAW_HEADER count starts at 0 with SBIN (Parameter to add in count)

            for(lLine=0;lLine<lEditorRows;lLine++)
            {
                // For split#2 and higher, ensure we write the header section again...
                if(iFileIndex >= 1)
                {
                    for(lCol = 0;lCol < lCountHeader;lCol++)
                    {
                        // Insert ',' separator prior to 2nd and higher columns
                        if(lCol)
                            hCsvTableFile << ",";
                        if(mExcelTable->item(lLine,lCol))
                            hCsvTableFile << mExcelTable->item(lLine,lCol)->text();
                        else
                            hCsvTableFile << "";
                    }
                }

                // Write line...unless hidden!
                if(mExcelTable->isRowHidden(lLine) == false)
                {
                    for(lCol = lStartSplitCol;lCol<lMaxSplitCol;lCol++)
                    {
                        if(bSaveAllColumns || (mExcelTable->isColumnHidden(lCol) == false))
                        {
                            // Insert ',' separator prior to 2nd and higher columns
                            if(lCol)
                                hCsvTableFile << ",";
                            if(mExcelTable->item(lLine,lCol))
                                hCsvTableFile << mExcelTable->item(lLine,lCol)->text();
                            else
                                hCsvTableFile << "";
                        }
                    }
                    // Add new line character
                    hCsvTableFile << endl;
                }
            }
            // Close file.
            f.close();

            // Move to next block
            lStartSplitCol = lMaxSplitCol;
        }
        // Set default name back to original .csv file name
        strFileName = m_strEditFile;

        strMessage = "Table saved!\nSplit files created:\n" + strFilesCreated;
        if(bQuietSuccess == false)
          GS::Gex::Message::information("", strMessage);
    }

    // Reset variable
    m_bTableModified = false;

    return true;
}


void GexTbToolBox::moveTestDetailsColumn()
{
    mExcelTable->horizontalHeader()->moveSection(GEX_TPAT_CONFIG_FIELD_TTYPE, GEX_TPAT_CONFIG_FIELD_TNUM+2);
    mExcelTable->horizontalHeader()->moveSection(GEX_TPAT_CONFIG_FIELD_PINMAP_IDX, GEX_TPAT_CONFIG_FIELD_TNUM+1);
}

void GexTbToolBox::initExcelTableItemFlags(int row, int col, QTableWidgetItem *item)
{
    //Settings Flags
    if (row < 2 || row == mExcelTable->rowCount()-1)
    {
        item->setFlags(Qt::NoItemFlags);
    }
    else
    {
        if (col == GEX_TPAT_CONFIG_FIELD_TNUM   ||
            col == GEX_TPAT_CONFIG_FIELD_TNUM	||
            col == GEX_TPAT_CONFIG_FIELD_TNAME	||
            col == GEX_TPAT_CONFIG_FIELD_LSL	||
            col == GEX_TPAT_CONFIG_FIELD_USL	||
            col == GEX_TPAT_CONFIG_FIELD_RMEAN	||
            col == GEX_TPAT_CONFIG_FIELD_RSIGMA	||
            col == GEX_TPAT_CONFIG_FIELD_SHAPE	||
            col == GEX_TPAT_CONFIG_FIELD_TTYPE	||
            col == GEX_TPAT_CONFIG_FIELD_PINMAP_IDX	)
        {
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        }

    }

}


