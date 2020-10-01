///////////////////////////////////////////////////////////
// All about Settings page
///////////////////////////////////////////////////////////
#include <QToolTip>
#include <QTreeWidget>
#include <QMessageBox>

#include <gqtl_log.h>

#include "gex_shared.h"
#include "settings_dialog.h"
#include "settings_sql.h"		// Holds SQL related defines
#include "browser.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "onefile_wizard.h"
#include "comparefiles_wizard.h"
#include "mergefiles_wizard.h"
#include "mixfiles_wizard.h"
#include "db_onequery_wizard.h"
#include "compare_query_wizard.h"
#include "drill_what_if.h"
#include "picktest_dialog.h"
#include "gex_constants.h"
#include "product_info.h"
#include "gex_report.h"
#include "ftr_correlation_report.h"
#include "engine.h"
#include "message.h"
#include "pat_report_ft_gui.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;
extern CGexReport* gexReport;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// script_wizard.h
extern void ConvertToScriptString(QString &strFile);
extern void ConvertFromScriptString(QString &strFile);

extern bool FillComboBox(QComboBox * pCombo, const char * szTextTab[]);

///////////////////////////////////////////////////////////
// WIZARD Settings PAGE
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_Settings(void)
{
    QString	strString;

    // Ensures all fields show their up-to-date status.
    pWizardSettings->OnComboStatisticsChange();
    pWizardSettings->OnComboWaferMapChange();
    pWizardSettings->OnComboHistogramTypeChange();
    pWizardSettings->OnComboHistogramChange();
    pWizardSettings->OnComboAdvancedReportChange();
    pWizardSettings->OnComboAdvancedReportSettingsChange();

    // Decide which tester images to show!
    pWizardSettings->UpdateBannerImages(GS::LPPlugin::ProductInfo::getInstance()->getProductID());

    // Hide/Show edit fields, depending of the Report output format.
    // small heck to ensure old behaviour
    QString strOutputFormatOption = (GS::Gex::Engine::GetInstance().GetOptionsHandler().GetOptionsMap().GetOption("output", "format")).toString();
    if (strOutputFormatOption==QString("HTML"))
        pWizardSettings->RefreshOutputFormat(GEX_SETTINGS_OUTPUT_HTML);	// HTML output format
    else if (strOutputFormatOption==QString("DOC"))
        pWizardSettings->RefreshOutputFormat(GEX_SETTINGS_OUTPUT_WORD);
    else if (strOutputFormatOption==QString("CSV"))
        pWizardSettings->RefreshOutputFormat(GEX_SETTINGS_OUTPUT_CSV);
    else if (strOutputFormatOption==QString("PPT"))
        pWizardSettings->RefreshOutputFormat(GEX_SETTINGS_OUTPUT_PPT);		// PPT output format
    else if (strOutputFormatOption==QString("PDF"))
        pWizardSettings->RefreshOutputFormat(GEX_SETTINGS_OUTPUT_PDF);		// PDF output format
    else if (strOutputFormatOption==QString("INTERACTIVE"))
        pWizardSettings->RefreshOutputFormat(GEX_SETTINGS_OUTPUT_INTERACTIVE);		// Interactive mode only (no report created)
    else if (strOutputFormatOption==QString("ODT"))
        pWizardSettings->RefreshOutputFormat(GEX_SETTINGS_OUTPUT_ODT);
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, "Undefined 'output' 'format' option");
        GEX_ASSERT(false);
    }

    pWizardSettings->OnChangeReportFormat();

    // Hide/Show edit fields, depending of the Use Limits options.
    QString lUsedLimits = (ReportOptions.GetOption("dataprocessing", "used_limits")).toString();
    int     lComboIdx   = pWizardSettings->comboBoxUsedLimits->findData(QVariant(lUsedLimits));

    if (lComboIdx >= 0)
        pWizardSettings->comboBoxUsedLimits->setCurrentIndex(lComboIdx);
    else
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Undefined 'dataprocessing' 'used_limits' options: %1").arg(lUsedLimits).toLatin1().constData());
    }

    // Checks the number of file to process...
    int iFiles=0;

    switch(mWizardType)
    {
    case GEX_ONEFILE_WIZARD:
        iFiles = pWizardOneFile->treeWidget->topLevelItemCount();
        break;
    case GEX_SHIFT_WIZARD:
    case GEX_CMPFILES_WIZARD:
        iFiles = pWizardCmpFiles->treeWidget->topLevelItemCount();
        break;
    case GEX_FT_PAT_FILES_WIZARD:
        iFiles = 0;
        if (mPATReportFTGui)
        {
            if (mPATReportFTGui->IsDataReady())
                iFiles = 1;
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, "FT PAT report gui not allocated");
        break;
    case GEX_ADDFILES_WIZARD:
        iFiles = pWizardAddFiles->treeWidget->topLevelItemCount();
        break;
    case GEX_MIXFILES_WIZARD:
        iFiles = pWizardMixFiles->treeWidget->topLevelItemCount();
        break;
    case GEX_ONEQUERY_WIZARD:
    case GEX_SQL_QUERY_WIZARD:
    case GEX_CHAR_QUERY_WIZARD:
        // Make sure ExaminatorDB has the latest list of databases (do not repaint Admin GUI page)
        iFiles = ReloadDatabasesList(false);
        // If database available...check if valid query available
        if(iFiles)
        {
            strString = pWizardOneQuery->lineEditReportTitle->text();
            iFiles = strString.length();
        }
        break;
    case GEX_CMPQUERY_WIZARD:
    case GEX_MIXQUERY_WIZARD:
        // Make sure ExaminatorDB has the latest list of databases (do not repaint Admin GUI page)
        iFiles = ReloadDatabasesList(false);
        // If database available...check if valid query list
        if(iFiles)
            iFiles = pWizardCmpQueries->treeWidget->topLevelItemCount();
        break;
    default:
        GEX_ASSERT(false);
        GSLOG(SYSLOG_SEV_WARNING, "Invalid wizard type");
        break;
    }

    // If no files, do not show 'Settings' page, rather display links to the assistants
    if(iFiles == 0)
    {
        // No file: show HTML page with links to
        strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
        strString += GEX_HELP_FOLDER;

        // Display relevant HTML page (if running Examinator or ExaminatorDB, or ExaminatorWeb)
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() ||
                GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() ||
                GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
                GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
        {
            strString += GEX_HTMLPAGE_DB_NOSETTINGS;
        }
        else
        {
            strString += GEX_HTMLPAGE_NOSETTINGS;
        }

        LoadUrl( strString );
        return;
    }

    // We have files to process: Show wizard page Settings
    ShowWizardDialog(GEX_WIZARD_SETTINGS);
}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void GexSettings::ShowPage(void)
{
    // First: Make Widget visible.
    show();

    // Then (and only then): Enable/disable some features...
    UpdateSkin(GS::LPPlugin::ProductInfo::getInstance()->getProductID());
}

///////////////////////////////////////////////////////////
// Update GUI fileds based on Product type running
///////////////////////////////////////////////////////////
void GexSettings::UpdateSkin(int /*lProductID*/)
{
    QString lPageTitle;

    // Page title
    //if(pGexMainWindow->iWizardType != GEX_SQL_QUERY_WIZARD)
    if(pGexMainWindow && pGexMainWindow->GetWizardType() == GEX_SQL_QUERY_WIZARD)
    {
        lPageTitle = "<b>- Create Enterprise Reports -</b>";

        // Check if a Enterprise report type is selected or not (so to alow 'Build repoprt' or not...
        // buttonBuildReport->setEnabled(m_SqlReportSelected != SQL_REPORT_NONE);		// PYC, 30/05/2011, case 4819
    }
    else
    {
        lPageTitle = "<b>- Create an Instant Report -</b>";
    }

    labelSettings->setText(lPageTitle);

    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        buttonLoadSettings->hide();
        buttonSaveSettings->hide();
        return;
    }
}

///////////////////////////////////////////////////////////
// Saves the Report settings into a Script file!
///////////////////////////////////////////////////////////
void GexMainwindow::SaveReportScript(void)
{
    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM(GS::LPPlugin::LicenseProvider::eLtxcOEM))
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Let's user tell where to save the configuration.
    QString fn = QFileDialog::getSaveFileName(this, "Save Examinator Script as...",
                                              "report_builder_script.csl", "Examinator Script file (*.csl)");

    // Reloads Report HOME page.
    ReportLink();

    // If no file selected, ignore command.
    if(fn.isEmpty())
        return;

    // Copy script xxx/.gex_assistant.csl into 'fn' file!
    FILE	*hSource;
    FILE	*hDest;
    char	szBuffer[256];

    // Before saving script, check if destination exists...
    hDest = fopen(fn.toLatin1().constData(),"r");
    if(hDest != NULL)
    {
        // File already exist...ask if overwrite!
        fclose(hDest);

        int i=QMessageBox::warning( this,
                                    GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                    "File already exists. Overwrite it?",
                                    "No",
                                    "Yes", 0, 0, 1 );
        if(i != 1)
            return;
    }

    hSource = fopen(GS::Gex::Engine::GetInstance().GetAssistantScript().toLatin1().constData(),"r");
    if(hSource == NULL)
    {
        GS::Gex::Message::
            critical("", GEX_T("ERROR: Failed reading script..."));
        return;	// Failed reading source
    }

    hDest = fopen(fn.toLatin1().constData(),"w");
    if(hDest == NULL)
    {
        fclose(hSource);
        GS::Gex::Message::critical(
            "", GEX_T("ERROR: Failed creating a copy of the script..."));
        return;	// Failed writing to destination
    }

    int iChoice = QMessageBox::question( this,
                                         GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                         "Is this going to be an automated report in Yield-Man?",
                                         QMessageBox::Yes|QMessageBox::No,
                                         QMessageBox::Yes);
    bool bUpdatePaths = false;
    if(iChoice == QMessageBox::Yes)
        bUpdatePaths = true;

    // Copy file...
    while(!feof(hSource))
    {
        if(fgets(szBuffer,255,hSource) != NULL)
        {
            //	gexReportType('adv_my_report','template','C:\\Documents and Sett//ings\\HerveT/.galaxy_sql.grt');
            QString strBuffer(szBuffer);
            QString strGrtFileName;

            if(bUpdatePaths && strBuffer.indexOf("gexOptions('") != -1){
                QString strOptionLine = strBuffer.trimmed();
                //"gexOptions('adv_boxplot' , 'r&r_file' , 'path/.galaxy_rr.txt' ); "
                //"-----0-----'-----1-----'-2-'----3---'-4-'----------------------5------------------'-5--"
                QString strOptionSection = strOptionLine.section("'",1,1);
                QString strOptionName = strOptionLine.section("'",3,3);
                if(ReportOptions.GetOptionType(strOptionSection, strOptionName) == "Path"){
                    continue;
                }
            }

            if (strBuffer.indexOf("gexReportType('adv_my_report','template','") != -1)
            {
                strGrtFileName = strBuffer.trimmed();
                strGrtFileName.remove("gexReportType('adv_my_report','template','", Qt::CaseSensitive);
                strGrtFileName = strGrtFileName.section("'", 0, 0);
            }
            else if (strBuffer.indexOf("gexReportType('adv_report_center','template','") != -1)
            {
                strGrtFileName = strBuffer.trimmed();
                strGrtFileName.remove("gexReportType('adv_report_center','template','", Qt::CaseSensitive);
                strGrtFileName = strGrtFileName.section("'", 0, 0);
            }

            if (strGrtFileName.isEmpty() == false)
            {
                QFileInfo	srcCslFile(fn);
                QString		strFinalGrtFileName	= srcCslFile.absolutePath() + QString("/") + srcCslFile.baseName() + QString(".grt");

                // If destination .grt file already exists, delete it
                if (QFile::exists(strFinalGrtFileName) && QFile::remove(strFinalGrtFileName) == false)
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("unable to remove file %1").arg(strFinalGrtFileName).toLatin1().data() );

                    GS::Gex::Message::critical(
                        "", GEX_T("ERROR: Failed removing an existing copy "
                                  "of the Quantix Report Template file..."));

                    // Close files
                    fclose(hSource);
                    fclose(hDest);

                    // Remove the destination file
                    QFile::remove(fn);

                    return;
                }

                if (QFile::copy(strGrtFileName, strFinalGrtFileName) == false)
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("unable to copy file %1 to %2").arg(strGrtFileName).arg(strFinalGrtFileName).toLatin1().data() );

                    GS::Gex::Message::critical(
                        "", GEX_T("ERROR: Failed creating a copy "
                                  "of the Quantix Report Template file..."));

                    // Close files
                    fclose(hSource);
                    fclose(hDest);

                    // Remove the destination file
                    QFile::remove(fn);

                    return;
                }

                ConvertToScriptString(strFinalGrtFileName);
                strBuffer = strBuffer.replace(strGrtFileName, strFinalGrtFileName);

                qstrncpy(szBuffer, strBuffer.toLatin1().constData(), 255);
            }

            fputs(szBuffer,hDest);
        }
    };

    // Close files
    fclose(hSource);
    fclose(hDest);

    // Reloads Report HOME page.
    ReportLink();
}

///////////////////////////////////////////////////////////
// Decide which tester image to display, depending of Examinator type (Credence, LTX, etc...)
///////////////////////////////////////////////////////////
void GexSettings::UpdateBannerImages(int lProductID)
{
    switch(lProductID)
    {
        default:
            // Show Asl3000 + Fusion: 'pixmapASL3000' + 'pixmapFusionHfi'
            widgetStackTesters->setCurrentIndex(0);
            break;

        case GS::LPPlugin::LicenseProvider::eSzOEM:             // OEM-Examinator for Credence SZ
            // Only show SZ images:  'pixmapSzFalcon'
            widgetStackTesters->setCurrentIndex(3);
            break;

        case GS::LPPlugin::LicenseProvider::eLtxcOEM:           // OEM-Examinator for LTXC
            // Only show FUSION images: 'pixmapFusionCx' + 'pixmapFusionHfi_2'
            widgetStackTesters->setCurrentIndex(0);
            break;

        case GS::LPPlugin::LicenseProvider::eTerOEM:                // Teradyne-Examinator
        case GS::LPPlugin::LicenseProvider::eTerProPlus:            // Teradyne-Examinator=Pro+
            // Only show Teradyne testers
            widgetStackTesters->setCurrentIndex(7);
            break;
    }
}

///////////////////////////////////////////////////////////
// Diaplay list of available tests, and let the user select
// a test, or multiple tests
///////////////////////////////////////////////////////////
QString GexSettings::PickTestFromList(bool bAllowMultipleSelections,
                                      int nTestType,
                                      bool enableBlocksCompression)
{
    // Check if SQL databse
    //switch(pGexMainWindow->iWizardType)
    switch(pGexMainWindow->GetWizardType())
    {
    case GEX_ONEQUERY_WIZARD:
    case GEX_CHAR_QUERY_WIZARD:
    case GEX_SQL_QUERY_WIZARD:
        if(pGexMainWindow->pWizardOneQuery->IsSqlDatabase())
            return pGexMainWindow->pWizardOneQuery->SqlPickParameterList(true);
        break;
    case GEX_CMPQUERY_WIZARD:
        QTreeWidgetItem * pTreeWidgetItem = NULL;
        if(pGexMainWindow->pWizardCmpQueries->FindFirstSqlDataset(&pTreeWidgetItem))
            return pGexMainWindow->pWizardCmpQueries->SqlPickParameterList(true);
        break;
    }

    // File database: use standard picktest dialog box
    PickTestDialog dPickTest;

    // Allow Multiple selections.
    dPickTest.setMultipleSelection(bAllowMultipleSelections);

    // Define test type allowed
    dPickTest.setAllowedTestType((PickTestDialog::TestType) nTestType);

    // Enable/Disable block compression
    dPickTest.setBlockCompressionEnabled(enableBlocksCompression);

    // Check if List was successfuly loaded
    if(dPickTest.fillParameterList())
    {
        // Prompt dialog box, let user pick tests from the list
        if(dPickTest.exec() == QDialog::Accepted)
            // Return selection
            return dPickTest.testList();
    }

    return "";
}

///////////////////////////////////////////////////////////
// Fill ComboBox with: Test Statistics option types available
///////////////////////////////////////////////////////////
void GexSettings::FillListBox_TestStatistics(QComboBox *comboStats,bool bAllowDisabledItem)
{
    comboStats->clear();
    if(bAllowDisabledItem)
        comboStats->addItem(QIcon(*pixDisabled),"Disabled");
    comboStats->addItem(QIcon(*pixTestStatistics),"All tests");
    comboStats->addItem(QIcon(*pixTestStatistics),"Failing tests only");
    comboStats->addItem(QIcon(*pixTestStatistics),"Tests with Data Cleaning");
    comboStats->addItem(QIcon(*pixTestStatistics),"Test list...");
    comboStats->addItem(QIcon(*pixTestStatistics),"Tests with Cp lower than...");
    comboStats->addItem(QIcon(*pixTestStatistics),"Tests with Cpk lower than...");
    comboStats->addItem(QIcon(*pixTestStatistics),"Top N failing tests...");
}

///////////////////////////////////////////////////////////
// Fill ComboBox with: Test Statistics option types available
///////////////////////////////////////////////////////////
void GexSettings::FillListBox_Histograms(QComboBox *comboHistogram,bool bAllowDisabledItem)
{
    comboHistogram->clear();
    if(bAllowDisabledItem)
        comboHistogram->addItem(QIcon(*pixDisabled),"Disabled");
    comboHistogram->addItem(QIcon(*pixHisto),"Chart over test limits");
    comboHistogram->addItem(QIcon(*pixHisto),"Cumulate over test limits");
    comboHistogram->addItem(QIcon(*pixHisto),"Chart over test results");
    comboHistogram->addItem(QIcon(*pixHisto),"Cumulate over test results");
    comboHistogram->addItem(QIcon(*pixHisto),"Adaptive: data & limits");
}

///////////////////////////////////////////////////////////
// Fill ComboBox with: Wafermap option types available
///////////////////////////////////////////////////////////
void GexSettings::FillListBox_Wafermap(QComboBox *comboWafer,bool bAllowDisabledItem)
{
    comboWafer->clear();
    if(bAllowDisabledItem)
        comboWafer->addItem(QIcon(*pixDisabled),"Disabled");
    comboWafer->addItem(QIcon(*pixWafermap),"Software Binning");
    comboWafer->addItem(QIcon(*pixWafermap),"Hardware Binning");
    comboWafer->addItem(QIcon(*pixWafermap),"Parametric test (over test limits)...");
    comboWafer->addItem(QIcon(*pixWafermap),"Parametric test (over test results)...");
    comboWafer->addItem(QIcon(*pixWafermap),"Parametric/Functional test (pass/fail)...");
    comboWafer->addItem(QIcon(*pixSeparator),"------ Stack wafers ------");
    comboWafer->addItem(QIcon(*pixWafermap),"Software Binning (stack wafers)...");
    comboWafer->addItem(QIcon(*pixWafermap),"Hardware Binning (stack wafers)...");
    comboWafer->addItem(QIcon(*pixWafermap),"Parametric test (stack wafers over test limits)...");
    comboWafer->addItem(QIcon(*pixWafermap),"Parametric test (stack wafers over test results)...");
    comboWafer->addItem(QIcon(*pixWafermap),"Parametric/Functional test (stack wafers pass/fail)...");
    comboWafer->addItem(QIcon(*pixSeparator),"------ Zonal Maps ------");
    comboWafer->addItem(QIcon(*pixWafermap),"Zonal map: Software bin");
    comboWafer->addItem(QIcon(*pixWafermap),"Zonal map: Hardware bin");
}


///////////////////////////////////////////////////////////
// SETTINGS dialog box
///////////////////////////////////////////////////////////
GexSettings::GexSettings( QWidget* parent, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl), mCharacLineChartTemplate(NULL), mCharacBoxWhiskerTemplate(NULL)
{
    GSLOG(SYSLOG_SEV_DEBUG, " ");
    setupUi(this);
    setModal(modal);
    setObjectName("GSSettings");

    setWindowFlags(Qt::FramelessWindowHint);
    move(100,100);

    QObject::connect(buttonImportScatterPairs,		SIGNAL(clicked()),				this, SLOT(OnImportScatterPairs()));
    QObject::connect(comboAdvancedReport,			SIGNAL(activated(int)),			this, SLOT(OnComboAdvancedReportChange()));
    QObject::connect(comboAdvancedReportSettings,	SIGNAL(activated(QString)),		this, SLOT(OnComboAdvancedReportSettingsChange()));
    QObject::connect(comboBoxOutputFormat,			SIGNAL(activated(QString)),		this, SLOT(OnChangeReportFormat()));
    QObject::connect(comboHistogram,				SIGNAL(activated(QString)),		this, SLOT(OnComboHistogramTypeChange()));
    QObject::connect(comboHistogramTests,			SIGNAL(activated(QString)),		this, SLOT(OnComboHistogramChange()));
    QObject::connect(comboStats,					SIGNAL(activated(QString)),		this, SLOT(OnComboStatisticsChange()));
    QObject::connect(comboWafer,					SIGNAL(activated(QString)),		this, SLOT(OnComboWaferMapChange()));
    QObject::connect(comboWafermapTests,			SIGNAL(activated(QString)),		this, SLOT(OnComboWaferMapTestChange()));
    QObject::connect(editAdvanced,					SIGNAL(textChanged(QString)),	this, SLOT(OnAdvancedValueChanged()));
    //QObject::connect(comboAdvanced,					SIGNAL()
    QObject::connect(editHistogram,					SIGNAL(textChanged(QString)),	this, SLOT(OnHistogramValueChanged()));
    QObject::connect(editStats,						SIGNAL(textChanged(QString)),	this, SLOT(OnStatisticsValueChanged()));
    QObject::connect(editTestX,						SIGNAL(textChanged(QString)),	this, SLOT(OnAdvancedValueChanged()));
    QObject::connect(editTestY,						SIGNAL(textChanged(QString)),	this, SLOT(OnAdvancedValueChanged()));
    QObject::connect(editWafer,						SIGNAL(textChanged(QString)),	this, SLOT(OnWaferMapValueChanged()));
    QObject::connect(PickTestAdvanced,				SIGNAL(clicked()),				this, SLOT(OnPickTestFromListAdvanced()));
    QObject::connect(PickTestHistogam,				SIGNAL(clicked()),				this, SLOT(OnPickTestFromListHistogram()));
    QObject::connect(PickTestScatterX,				SIGNAL(clicked()),				this, SLOT(OnPickTestScatterX()));
    QObject::connect(PickTestScatterY,				SIGNAL(clicked()),				this, SLOT(OnPickTestScatterY()));
    QObject::connect(PickTestStats,					SIGNAL(clicked()),				this, SLOT(OnPickTestFromListStats()));
    QObject::connect(PickTestWaferMap,				SIGNAL(clicked()),				this, SLOT(OnPickTestFromListWaferMap()));
    QObject::connect(pushButtonProductionOptions,	SIGNAL(clicked()),				this, SLOT(OnProductionReportOptions()));
    QObject::connect(comboBoxVolumeAxis,			SIGNAL(activated(QString)),		this, SLOT(OnProductionReportOptionsChanged()));
    QObject::connect(checkBoxYieldMarker,			SIGNAL(toggled(bool)),			this, SLOT(OnProductionReportOptionsChanged()));
    QObject::connect(buttonLoadSettings,			SIGNAL(clicked()),				this, SLOT(onLoadSettings()));
    QObject::connect(buttonSaveSettings,			SIGNAL(clicked()),				this, SLOT(onSaveSettings()));

    // Define Edit box maximum length allowed
    // HTH: case 6542 - Assign same max length for all text edit used for a test list
    //    editStats->setMaxLength(3000);
    //    editWafer->setMaxLength(3000);
    //    editHistogram->setMaxLength(3000);
    //    editAdvanced->setMaxLength(3000);
    //    editTestX->setMaxLength(32767);

    // Build Test Statistics combo-box
    FillListBox_TestStatistics(comboStats,true);
    editStats->setToolTip(
        tr("For list of tests: enter list. E.g: 1 to 50, 60 to 80,83,85\nFor Cp/Cpk limit: enter limit. E.g: 1.33"));

    // Build wafermap combo-box
    FillListBox_Wafermap(comboWafer,true);
    editWafer->setToolTip(tr("For binning: enter Bin#. E.g: 1\nFor test#: enter test#. E.g: 1024"));

    // Build histogram combo-box
    FillListBox_Histograms(comboHistogram,true);

    // Wafermap - 2nd combo
    comboWafermapTests->clear();
    comboWafermapTests->addItem("All tests");
    comboWafermapTests->addItem("Test list...");
    comboWafermapTests->addItem("Top N failing tests...");

    //QToolTip::add( editWafer, tr("Enter list. E.g: 1 to 50, 60 to 80,83,85") );

    // Histogram- 2nd combo
    comboHistogramTests->clear();
    comboHistogramTests->addItem("All tests");
    comboHistogramTests->addItem("Test list...");
    comboHistogramTests->addItem("Top N failing tests...");
    editStats->setToolTip(tr("Enter list. E.g: 1 to 50, 60 to 80,83,85"));

    comboAdvanced->addItem(ADV_ALL_TESTS);
    comboAdvanced->addItem(ADV_TEST_LIST);
    comboAdvanced->addItem(ADV_TOP_N_FAILTEST);
    comboAdvanced->setCurrentIndex(1);

    QObject::connect(comboAdvanced, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboAdvancedTestsChanged(int)));

    comboAdvanced->setCurrentIndex(1);

    // Build Advanced reports combo-box
    comboAdvancedReport->clear();
    comboAdvancedReport->setMaxVisibleItems( 50 );
    comboAdvancedReport->addItem(QIcon(*pixDisabled),"Disabled",GEX_ADV_DISABLED);
    comboAdvancedReport->addItem(QIcon(*pixSeparator),"------ Advanced charts ------", GEX_ADV_SEPARATOR);
    comboAdvancedReport->addItem(QIcon(*pixAdvHisto),"Histogram (Bar chart)", GEX_ADV_HISTOGRAM);
    comboAdvancedReport->addItem(QIcon(*pixTrend),"Trend (Control chart)", GEX_ADV_TREND);
    comboAdvancedReport->addItem(QIcon(*pixScatter),"Correlation (Scatter/BiVariate chart)", GEX_ADV_CORRELATION);
    comboAdvancedReport->addItem(QIcon(*pixProbabilityPlot),"Probability Plot", GEX_ADV_PROBABILITY_PLOT);
    comboAdvancedReport->addItem(QIcon(*pixBoxPlot),"Box-Plot", GEX_ADV_BOXPLOT);
    comboAdvancedReport->addItem(QIcon(*pixMultiCharts),"Multi-charts", GEX_ADV_MULTICHART);

    comboAdvancedReport->addItem(QIcon(*pixSeparator),"------ Miscellaneous ------", GEX_ADV_SEPARATOR);
    comboAdvancedReport->addItem(QIcon(*pixBoxMeanRange),"Gage R&R (BoxPlot/Candle chart)", GEX_ADV_CANDLE_MEANRANGE);
    comboAdvancedReport->addItem(QIcon(*pixDatalog),"Datalog", GEX_ADV_DATALOG);
    comboAdvancedReport->addItem(QIcon(*pixGuardBanding),"Guard banding, What-If / Virtual retest...", GEX_ADV_GUARDBANDING);
    comboAdvancedReport->addItem(QIcon(*pixStopwatch),"Pearson's correlation: Test flow & Test time reduction", GEX_ADV_PEARSON);
#ifdef GCORE15334
    comboAdvancedReport->addItem(QIcon(*pixPatHistory),"PAT-Man History (extract PAT report from data file)", GEX_ADV_PAT_TRACEABILITY);
#endif
    comboAdvancedReport->addItem(QIcon(*pixTrend),"Production reports (yield)", GEX_ADV_PROD_YIELD);

    comboAdvancedReport->addItem(QIcon(*pixSeparator),"------ Your Own Quantix ------", GEX_ADV_SEPARATOR);
    comboAdvancedReport->addItem(QIcon(*pixTasks),"My Reports: For Personalized & tailored reports!", GEX_ADV_TEMPLATE);

    comboAdvancedReport->addItem(QIcon(*pixSeparator),"------ Others ------", GEX_ADV_SEPARATOR);
    comboAdvancedReport->addItem(QIcon(*pixShift),"Shift report", GEX_ADV_SHIFT);

    comboAdvancedReport->addItem(QIcon(*pixSeparator),"------ Digital Reports ------", GEX_ADV_SEPARATOR);
    comboAdvancedReport->addItem(QIcon(*pixAdvFunctional),"Functional Reports", GEX_ADV_FUNCTIONAL);
    comboAdvancedReport->addItem(QIcon(*pixAdvFTRCorr),"FTR correlation", GEX_ADV_FTR_CORRELATION);


    // Tolltip texts
    editAdvanced->setToolTip(
        tr("For list of tests: enter list. E.g: 1 to 50, 60 to 80,83,85\nFor binning: enter Bin# or list. E.g: 1"));

    // Used limits Combo box
    comboBoxUsedLimits->clear();
    comboBoxUsedLimits->addItem("Use spec limits if any", QVariant("spec_limits_if_any"));
    comboBoxUsedLimits->addItem("Use standard limits only", QVariant("standard_limits_only"));
    comboBoxUsedLimits->addItem("Use spec limits only", QVariant("spec_limits_only"));

    // Support for Drag&Drop
    setAcceptDrops(true);

    // At startup, default is:
    comboStats->setCurrentIndex(GEX_STATISTICS_ALL);	// Test statistics: ALL tests
    comboWafer->setCurrentIndex(GEX_WAFMAP_SOFTBIN);	// Wafermap: Soft bin
    comboHistogram->setCurrentIndex(GEX_HISTOGRAM_OVERDATA);	// Histgram over data
    comboHistogramTests->setCurrentIndex(GEX_HISTOGRAM_ALL);	// Histogram: all tests
    comboWafermapTests->setCurrentIndex(GEX_WAFMAP_ALL);	// Histogram: all tests

    // Show standard 'test#' edit box, hide 'Scatter edit box'
    widgetStackAdvanced->setCurrentIndex(0);

    // Default binning for production reports (yield) is Bin1
    editAdvancedProduction->setText("1");
    OnProductionReportOptions();	// Make sure advanced production options are hidden by default.

    // Reset SQL reports menu to default: close all preselections, hide settings
    OnSqlReport_Startup();

    // Init characterization wizard page
    InitCharWizardPage();

    // Default Settings is: Create Instant Report (other selection available is 'Data Mining')
    OnDoInstantReport();

    // Clear the 'Report Template' GUI field
    textLabelTemplateFile->setText("");

    // Connect signals: MyReport Template Wizard
    connect((QObject *)buttonLoadTemplate,			SIGNAL(clicked()),	this,SLOT(OnSelectReportTemplate(void)));
    connect((QObject *)pushButtonTemplateWizard,	SIGNAL(clicked()),	this,SLOT(OnReportTemplateWizard(void)));

    /////////////////////////////////////////////////////////
    // ENTERPRISE REPORTS GUI (signals)
    /////////////////////////////////////////////////////////

    // GUI settings for Enterprise Advanced Yield reports
    listviewSQL_YieldWizard_AllChartSplitFields->setSortingEnabled(false);
    listviewSQL_YieldWizard_AllLayerSplitFields->setSortingEnabled(false);
    labelSQL_YieldWizard_Page1->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    labelSQL_YieldWizard_Page2->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    labelSQL_YieldWizard_Page3->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    labelSQL_YieldWizard_Page4->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // Mission toolbox
    connect((QObject *)toolbox_Mission,							SIGNAL(currentChanged(int)),	this,SLOT(OnSqlReport_MissionChanged(void)));
    connect((QObject *)toolButtonSql_UPH,						SIGNAL(clicked()),				this,SLOT(OnSqlReport_Prod_UPH(void)));
    connect((QObject *)toolButtonSql_Yield,						SIGNAL(clicked()),				this,SLOT(OnSqlReport_Prod_Yield(void)));
    connect((QObject *)toolButtonSql_ConsolidatedYield,			SIGNAL(clicked()),				this,SLOT(OnSqlReport_Prod_ConsolidatedYield(void)));
    connect((QObject *)toolButtonSql_WYR_Standard,				SIGNAL(clicked()),				this,SLOT(OnSqlReport_WYR_Standard(void)));
    connect((QObject *)toolButtonSql_YieldWizard,				SIGNAL(clicked()),				this,SLOT(OnSqlReport_Prod_YieldWizard(void)));
    connect((QObject *)toolButtonSql_Genealogy_YieldVsYield,	SIGNAL(clicked()),				this,SLOT(OnSqlReport_Genealogy_YieldVsYield(void)));
    connect((QObject *)toolButtonSql_Genealogy_YieldVsParameter,	SIGNAL(clicked()),			this,SLOT(OnSqlReport_Genealogy_YieldVsParameter(void)));

    // 'More Settings' button
    connect((QObject *)buttonSQL_UPH_MoreSettings,					SIGNAL(clicked()),			this,SLOT(OnSqlReport_MoreSettings(void)));
    connect((QObject *)buttonSQL_Yield_MoreSettings,				SIGNAL(clicked()),			this,SLOT(OnSqlReport_MoreSettings(void)));
    connect((QObject *)buttonSQL_ConsolidatedYield_MoreSettings,	SIGNAL(clicked()),			this,SLOT(OnSqlReport_MoreSettings(void)));

    // Charting style modified
    connect((QObject *)comboSQL_BackgroundStyle,SIGNAL(activated(int)),this,SLOT(OnSqlReport_UpdateGui(void)));

    // Production - UPH
    connect((QObject *)checkSQL_UPH_SplitBy,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_UPH_UpdateGui(void)));

    // Production - Yield/Volume
    connect((QObject *)checkSQL_Yield_SplitBy,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_Yield_UpdateGui(void)));
    connect((QObject *)comboSQL_Yield_Binning,SIGNAL(activated(int)),this,SLOT(OnSqlReport_Prod_Yield_UpdateGui(void)));
    connect((QObject *)buttonSQL_Yield_PickFilter_Binning,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_Yield_PickFilter_Binning(void)));

    // Production - Consolidated Yield/Volume
    connect((QObject *)checkSQL_ConsolidatedYield_SplitBy,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_ConsolidatedYield_UpdateGui(void)));

    // WYR - Standard
    connect((QObject *)buttonSQL_StdWYR_PickFilter_Site,SIGNAL(clicked()),this,SLOT(OnSqlReport_WYR_Standard_PickFilter_Site(void)));
    connect((QObject *)buttonSQL_StdWYR_PickFilter_Week,SIGNAL(clicked()),this,SLOT(OnSqlReport_WYR_Standard_PickFilter_Week(void)));
    connect((QObject *)buttonSQL_StdWYR_PickFilter_Year,SIGNAL(clicked()),this,SLOT(OnSqlReport_WYR_Standard_PickFilter_Year(void)));

    // Production - Yield Wizard
    connect((QObject *)comboSQL_YieldWizard_BackgroundStyle,SIGNAL(activated(int)),this,SLOT(OnSqlReport_Prod_YieldWizard_UpdateGui(void)));
    connect((QObject *)mUi_qtwSQLYieldWizardSeriesView,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(OnSqlReport_Prod_YieldWizard_EditSerie(void)));
    connect((QObject *)buttonSQL_YieldWizard_AddSerie,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_AddSerie(void)));
    connect((QObject *)buttonSQL_YieldWizard_RemoveSerie,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_RemoveSerie(void)));
    connect((QObject *)buttonSQL_YieldWizardClearSeries,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_ClearAllSeries(void)));
    connect((QObject *)buttonSQL_YieldWizard_NextPage1,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_NextPage(void)));
    connect((QObject *)buttonSQL_YieldWizard_NextPage2,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_NextPage(void)));
    connect((QObject *)buttonSQL_YieldWizard_NextPage3,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_NextPage(void)));
    connect((QObject *)buttonSQL_YieldWizard_NextPage4,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_NextPage(void)));
    connect((QObject *)buttonSQL_YieldWizard_PreviousPage5,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_PreviousPage(void)));
    connect((QObject *)buttonSQL_YieldWizard_PreviousPage4,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_PreviousPage(void)));
    connect((QObject *)buttonSQL_YieldWizard_PreviousPage3,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_PreviousPage(void)));
    connect((QObject *)buttonSQL_YieldWizard_PreviousPage2,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_PreviousPage(void)));
    connect((QObject *)buttonYieldWizard_AddChartSplitField,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_AddChartSplit(void)));
    connect((QObject *)buttonYieldWizard_RemoveChartSplitField,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_RemoveChartSplit(void)));
    connect((QObject *)buttonYieldWizard_ResetChartSplitField,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_RemoveAllChartSplit(void)));
    connect((QObject *)buttonYieldWizard_AddLayerSplitField,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_AddLayerSplit(void)));
    connect((QObject *)buttonYieldWizard_RemoveLayerSplitField,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_RemoveLayerSplit(void)));
    connect((QObject *)buttonYieldWizard_ResetLayerSplitField,SIGNAL(clicked()),this,SLOT(OnSqlReport_Prod_YieldWizard_RemoveAllLayerSplit(void)));

    // Genealogy - Yield vs Yield
    connect( comboSQL_Genealogy_YieldVsYield_TestingStage_Xaxis, SIGNAL( activated(int) ), this, SLOT( OnSqlReport_Genealogy_YieldVsYield_UpdateGui() ) );
    connect( comboSQL_Genealogy_YieldVsYield_TestingStage_Yaxis, SIGNAL( activated(int) ), this, SLOT( OnSqlReport_Genealogy_YieldVsYield_UpdateGui() ) );
    connect( comboSQL_Genealogy_YieldVsYield_Binning_Xaxis, SIGNAL( activated(int) ), this, SLOT( OnSqlReport_Genealogy_YieldVsYield_UpdateGui() ) );
    connect( comboSQL_Genealogy_YieldVsYield_Binning_Yaxis, SIGNAL( activated(int) ), this, SLOT( OnSqlReport_Genealogy_YieldVsYield_UpdateGui() ) );
    connect( buttonSQL_Genealogy_YieldVsYield_PickBinlist_Xaxis, SIGNAL( clicked() ), this, SLOT( OnSqlReport_Genealogy_YieldVsYield_OnPickBinlist_Xaxis() ) );
    connect( buttonSQL_Genealogy_YieldVsYield_PickBinlist_Yaxis, SIGNAL( clicked() ), this, SLOT( OnSqlReport_Genealogy_YieldVsYield_OnPickBinlist_Yaxis() ) );
    connect( buttonSQL_Genealogy_YieldVsYield_PickProduct, SIGNAL( clicked() ), this, SLOT( OnSqlReport_Genealogy_YieldVsYield_OnPickProduct() ) );

    // Genealogy - Yield vs Parameter
    connect( comboSQL_Genealogy_YieldVsParameter_TestingStage_Xaxis, SIGNAL( activated(int) ), this, SLOT( OnSqlReport_Genealogy_YieldVsParameter_UpdateGui() ) );
    connect( comboSQL_Genealogy_YieldVsParameter_TestingStage_Yaxis, SIGNAL( activated(int) ), this, SLOT( OnSqlReport_Genealogy_YieldVsParameter_UpdateGui() ) );
    connect( comboSQL_Genealogy_YieldVsParameter_Binning_Yaxis, SIGNAL( activated(int) ), this, SLOT( OnSqlReport_Genealogy_YieldVsParameter_UpdateGui() ) );
    connect( buttonSQL_Genealogy_YieldVsParameter_PickParameter_Xaxis, SIGNAL( clicked() ), this, SLOT( OnSqlReport_Genealogy_YieldVsParameter_OnPickParameter_Xaxis() ) );
    connect( buttonSQL_Genealogy_YieldVsParameter_PickBinlist_Yaxis, SIGNAL( clicked() ), this, SLOT( OnSqlReport_Genealogy_YieldVsParameter_OnPickBinlist_Yaxis() ) );
    connect( buttonSQL_Genealogy_YieldVsParameter_PickProduct, SIGNAL( clicked() ), this, SLOT( OnSqlReport_Genealogy_YieldVsParameter_OnPickProduct() ) );

    //    Genealogy - Yield vs Parameter/Yield  enable build report
    connect( toolButtonSql_Genealogy_YieldVsParameter, SIGNAL( clicked() ), this, SLOT( disableBuildReportButton() ) );
    connect( toolButtonSql_Genealogy_YieldVsYield,SIGNAL( clicked() ), this, SLOT( disableBuildReportButton() ) );

    connect( editSQL_Genealogy_YieldVsParameter_Product,SIGNAL( textChanged ( const QString &  ) ), this, SLOT( enableBuildReportButton(const QString &) ) );
    connect( editSQL_Genealogy_YieldVsYield_Product, SIGNAL( textChanged ( const QString & ) ), this, SLOT( enableBuildReportButton(const QString &) ) );
}

///////////////////////////////////////////////////////////
// GexSettings: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void GexSettings::reject(void)
{
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexSettings::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexSettings::dropEvent(QDropEvent *e)
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

    // Check file extension: If template file, do appropriate selections, otherwise pass files to MainWindow (probably test data files)
    QString strFile = strFileList[0];
    if(strFile.endsWith(".grt",Qt::CaseInsensitive))
    {
        // Select file
        OnSelectReportTemplate(strFile);
    }
    // settings file
    else if (strFile.endsWith(".csl",Qt::CaseInsensitive))
        loadSettings(strFileName);
    else
    {
        // Probably test data files...have them selected in the 'Data' page.
        if(pGexMainWindow != NULL)
            pGexMainWindow->dropEvent(e);
    }

    e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Tells if Report type is Enterprise (SQL) or File analysis...
///////////////////////////////////////////////////////////
bool GexSettings::IsEnterpriseSqlReport(void)
{
    if(widgetStackReport->currentIndex() == GEX_REPORT_TYPE_SQL)
        return true;
    else
        return false;
}




///////////////////////////////////////////////////////////
// Synchronises Output format bewteen 'Settings' and 'Options' pages
///////////////////////////////////////////////////////////
void GexSettings::RefreshOutputFormat(int iOptionsOutputFormat)
{
    // If Function called from the 'Options' page (iOptionsOutputFormat >= 0), we must have the 'Settings' page align itself on the 'Options' selection
    int iSettingsOutputFormat = comboBoxOutputFormat->currentIndex();
    if(iSettingsOutputFormat == iOptionsOutputFormat)
        return;

    switch(iOptionsOutputFormat)
    {
    case GEX_SETTINGS_OUTPUT_HTML:	// HTML report
    case GEX_SETTINGS_OUTPUT_WORD:	// Word report
    case GEX_SETTINGS_OUTPUT_PPT:	// PowerPoint slides report
    case GEX_SETTINGS_OUTPUT_PDF:	// PDF report
    case GEX_SETTINGS_OUTPUT_CSV:	// CSV report
    case GEX_SETTINGS_OUTPUT_INTERACTIVE: // No report to create, interactive mode only.
        comboBoxOutputFormat->setCurrentIndex(iOptionsOutputFormat);
        break;
    }
}


///////////////////////////////////////////////////////////
// checks if Examinator running in a Professional mode (with all features enabled)
// This function have to be removed when the new function GS::LPPlugin::ProductInfo::getInstance().isNotSupportedCapability
// will be completed
bool checkProfessionalFeature(bool bProductionReleaseOnly)
{
    if(bProductionReleaseOnly == false)
        return true;

    if(!GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() &&
            !GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus() &&
            !GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() &&
            !GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        return false;

    // Examinator running with enough privileges!
    return true;
}


///////////////////////////////////////////////////////////
// Enables/Disables ALL InstantReport standard Combos
///////////////////////////////////////////////////////////
void GexSettings::enableInstantReportStandardCombos(bool bEnable)
{
    comboStats->setEnabled(bEnable);
    editStats->setEnabled(bEnable);
    PickTestStats->setEnabled(bEnable);
    comboWafer->setEnabled(bEnable);
    comboWafermapTests->setEnabled(bEnable);
    editWafer->setEnabled(bEnable);
    PickTestWaferMap->setEnabled(bEnable);
    comboHistogram->setEnabled(bEnable);
    comboHistogramTests->setEnabled(bEnable);
    editHistogram->setEnabled(bEnable);
    PickTestHistogam->setEnabled(bEnable);
    widgetStackAdvanced->setEnabled(bEnable);
    comboAdvancedReportSettings->setEnabled(bEnable);
}


///////////////////////////////////////////////////////////
// Load settings from .csl file
///////////////////////////////////////////////////////////
void GexSettings::onLoadSettings(void)
{
    // Let's user pick the settings to load
    QString strFileName = QFileDialog::getOpenFileName(this, "Load settings from...", "", "Settings (*.csl)");

    // If no file selected, ignore command.
    if(strFileName.isEmpty())
        return;

    loadSettings(strFileName);
}

void GexSettings::loadSettings(const QString& strFileName)
{
    // Read the text from a file
    QFile file(strFileName);
    if (file.open(QIODevice::ReadOnly) == false)
    {
        GS::Gex::Message::critical(
            "", "Failed reading file...folder is read protected?");
        return;
    }

    // Read Tasks definition File
    QTextStream hFile;
    hFile.setDevice(&file);	// Assign file handle to data stream

    comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));

    QString	strString;
    QString strKey, strParameter, strExtraParameter;
	QString lUsedLimitsChoice;
	QString lLimitsSetChoice;
    do
    {
        // Read one line from file
        strString = hFile.readLine();
        strString = strString.trimmed();	// Remove starting & leading spaces.

        // Extract output format
        if(strString.startsWith("gexOptions('output','format','") == true)
        {
            // Line format is:
            // gexOptions('output','format','<output_format>')

            // Extract Output format
            strKey	= strString.section(',', 2, 2);
            // Remove starting and leading ' characters
            strKey	= strKey.mid(1,strKey.length()-4);

            // Decode script string.
            ConvertFromScriptString(strKey);

            if (strKey == "html")
                comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_HTML);
            else if (strKey == "word")
                comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_WORD);
            else if (strKey == "ppt")
                comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_PPT);
            else if (strKey == "pdf")
                comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_PDF);
            else if (strKey == "csv")
                comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_CSV);
            else if (strKey == "interactive")
                comboBoxOutputFormat->setCurrentIndex(GEX_SETTINGS_OUTPUT_INTERACTIVE);
        }
		//Extract Used Limits
		else if (strString.startsWith("gexOptions('dataprocessing','used_		limits'") == true)
		{
		     // Line format is:
		     // gexOptions('dataprocessing','used_limits'
		
		     lUsedLimitsChoice = strString.section("'", 5, 5);
		     //int debugme = comboBoxUsedLimits->currentIndex();
		     //auto debugme2 = comboBoxUsedLimits->findData( QVariant( "spec_limits_only" ) );
		     if (lUsedLimitsChoice == "spec_limits_only")
		     {
		         comboBoxUsedLimits->setCurrentIndex(GEX_LIMITS_USEDLIMITS_SPECONLY);
		     }
		     else if (lUsedLimitsChoice == "spec_limits_if_any")
		     {
		         comboBoxUsedLimits->setCurrentIndex(GEX_LIMITS_USEDLIMITS_SPECIFANY);
		     }
		     else if (lUsedLimitsChoice == "standard_limits_only")
		     {
		         comboBoxUsedLimits->setCurrentIndex(GEX_LIMITS_USEDLIMITS_STDONLY);
		     }
		     else if (lUsedLimitsChoice == "multi_limits_if_any")
		     {
		         comboBoxUsedLimits->setCurrentIndex(GEX_LIMITS_USEDLIMITS_MULTIIFANY);
		     }
		     else//dataprocessing/limits_set did not come along with dataprocessing/used_limits, error
		     {
		         GS::Gex::Message::critical(
		             "", "Error in limits set options, used_limits set but limits_set is not (they must be present together in csl file)");
		          return;
		      }
		}
/*       //Extract limits set
       else if (strString.startsWith("gexOptions('dataprocessing','limits_set'") == true)
       {
           //auto debugme = comboBoxLimitsSetOrder->findData( QVariant("chrono_asc" ) );
           lLimitsSetChoice = strString.section("'", 5, 5);
           if (lLimitsSetChoice == "chrono_asc")
           {
               comboBoxLimitsSetOrder->setCurrentIndex(GEX_LIMITS_REF_LIMITS_OLDEST);
           }
           else if (lLimitsSetChoice == "chrono_desc")
           {
               comboBoxLimitsSetOrder->setCurrentIndex(GEX_LIMITS_REF_LIMITS_LATEST);
           }
           else
           {
               GS::Gex::Message::critical("", "Error in limits set order option, choice unexpected");
                return;
            }
        }
        //Extract multi_limit_set
        else if (strString.startsWith("gexOptions('dataprocessing','mult_limit_set'") == true)
        {
            if(lUsedLimitsChoice != "multi_limits_if_any")
            {
                GS::Gex::Message::critical(
                    "", "Error in limits set options, only multi limits should have sub options");
                return;
            }
            QString strKey = strString.section("'", 5, 5);
            // Remove starting and leading ' characters
            strKey = strKey.mid(1, strKey.length() - 4);
            // Extract parameter with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);
            // Decode script string.
            ConvertFromScriptString(strKey);
            spinBoxLimitSet->setValue(strParameter.toInt());
        }
*/


        // Extract stats
        else if(strString.startsWith("gexReportType('stats','") == true)
        {
            // Line format is:
            // gexReportType('stats','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey == "disabled")
                comboStats->setCurrentIndex(GEX_STATISTICS_DISABLED);
            else if (strKey == "all")
                comboStats->setCurrentIndex(GEX_STATISTICS_ALL);
            else if (strKey == "fails")
                comboStats->setCurrentIndex(GEX_STATISTICS_FAIL);
            else if (strKey == "outliers")
                comboStats->setCurrentIndex(GEX_STATISTICS_OUTLIERS);
            else if (strKey == "tests")
                comboStats->setCurrentIndex(GEX_STATISTICS_LIST);
            else if (strKey == "cp")
                comboStats->setCurrentIndex(GEX_STATISTICS_BADCP);
            else if (strKey == "cpk")
                comboStats->setCurrentIndex(GEX_STATISTICS_BADCPK);
            else if (strKey == "top_n_fails")
            {
                comboStats->setCurrentIndex(GEX_STATISTICS_TOP_N_FAILTESTS);
            }



            if (!strParameter.isEmpty())
                editStats->setText(strParameter);
        }
        // Extract wafermap
        else if(strString.startsWith("gexReportType('wafer','") == true)
        {
            // Line format is:
            // gexReportType('wafer','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey == "disabled")
                comboWafer->setCurrentIndex(GEX_WAFMAP_DISABLED);
            else if (strKey == "soft_bin")
                comboWafer->setCurrentIndex(GEX_WAFMAP_SOFTBIN);
            else if (strKey == "stack_soft_bin")
                comboWafer->setCurrentIndex(GEX_WAFMAP_STACK_SOFTBIN);
            else if (strKey == "hard_bin")
                comboWafer->setCurrentIndex(GEX_WAFMAP_HARDBIN);
            else if (strKey == "stack_hard_bin")
                comboWafer->setCurrentIndex(GEX_WAFMAP_STACK_HARDBIN);
            else if (strKey == "param_over_limits")
                comboWafer->setCurrentIndex(GEX_WAFMAP_TESTOVERLIMITS);
            else if (strKey == "stack_param_over_limits")
                comboWafer->setCurrentIndex(GEX_WAFMAP_STACK_TESTOVERLIMITS);
            else if (strKey == "param_over_range")
                comboWafer->setCurrentIndex(GEX_WAFMAP_TESTOVERDATA);
            else if (strKey == "stack_param_over_range")
                comboWafer->setCurrentIndex(GEX_WAFMAP_STACK_TESTOVERDATA);
            else if (strKey == "test_passfail")
                comboWafer->setCurrentIndex(GEX_WAFMAP_TEST_PASSFAIL);
            else if (strKey == "stack_test_passfail")
                comboWafer->setCurrentIndex(GEX_WAFMAP_STACK_TEST_PASSFAIL);
            else if (strKey == "zonal_soft_bin")
                comboWafer->setCurrentIndex(GEX_WAFMAP_ZONAL_SOFTBIN);
            else if (strKey == "zonal_hard_bin")
                comboWafer->setCurrentIndex(GEX_WAFMAP_ZONAL_HARDBIN);

            if (strParameter == "all")
                comboWafermapTests->setCurrentIndex(GEX_WAFMAP_ALL);
            else if (strParameter.startsWith("top"))
            {
                comboWafermapTests->setCurrentIndex(GEX_WAFMAP_TOP_N_FAILTESTS);
                editWafer->setText(strParameter.section(' ', 1,1));
            }
            else if (strParameter.isEmpty() == false)
            {
                comboWafermapTests->setCurrentIndex(GEX_WAFMAP_LIST);
                editWafer->setText(strParameter);
            }
        }
        // Extract histogram
        else if(strString.startsWith("gexReportType('histogram','") == true)
        {
            // Line format is:
            // gexReportType('histogram','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey == "disabled")
			{
                comboHistogram->setCurrentIndex(GEX_HISTOGRAM_DISABLED);
				editHistogram->hide();
			}
			else
			{
				editHistogram->show();
			}

            if (strKey == "test_over_limits")
                comboHistogram->setCurrentIndex(GEX_HISTOGRAM_OVERLIMITS);
            else if (strKey == "cumul_over_limits")
                comboHistogram->setCurrentIndex(GEX_HISTOGRAM_CUMULLIMITS);
            else if (strKey == "test_over_range")
                comboHistogram->setCurrentIndex(GEX_HISTOGRAM_OVERDATA);
            else if (strKey == "cumul_over_range")
                comboHistogram->setCurrentIndex(GEX_HISTOGRAM_CUMULDATA);
            else if (strKey == "adaptive")
                comboHistogram->setCurrentIndex(GEX_HISTOGRAM_DATALIMITS);

            if (strParameter == "all")
			{
				editHistogram->hide();
                comboHistogramTests->setCurrentIndex(GEX_HISTOGRAM_ALL);
			}
            else
			{
                if (strParameter.startsWith("top"))
				{
                    comboHistogramTests->setCurrentIndex(GEX_HISTOGRAM_TOP_N_FAIL_TESTS);
					strParameter = strParameter.section(' ', 1, 1);
					editHistogram->setText(strParameter);
				}
                else
                {
                    comboHistogramTests->setCurrentIndex(GEX_HISTOGRAM_LIST);
                    editHistogram->setText(strParameter);
                }
			}
        }
        // Extract advanced histogram
        else if(strString.startsWith("gexReportType('adv_histogram','") == true)
        {
            // Line format is:
            // gexReportType('adv_histogram','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey != "disabled")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_HISTOGRAM));
                OnComboAdvancedReportChange();
            }

            if (strKey == "test_over_limits")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_HISTOGRAM_OVERLIMITS);
            else if (strKey == "cumul_over_limits")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_HISTOGRAM_CUMULLIMITS);
            else if (strKey == "test_over_range")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_HISTOGRAM_OVERDATA);
            else if (strKey == "cumul_over_range")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_HISTOGRAM_CUMULDATA);
            else if (strKey == "adaptive")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_HISTOGRAM_DATALIMITS);

            if (!strParameter.isEmpty())
            {
                if (strParameter == "all")
                {
                    comboAdvanced->setCurrentIndex(0);
                    editAdvanced->setText("");
                    editAdvanced->hide();
                }
                else
                {
                    editAdvanced->show();
                    comboAdvanced->setCurrentIndex(1);
                    if (strParameter.startsWith("top"))
                    {
                        comboAdvanced->setCurrentIndex(2);
                        strParameter = strParameter.section(' ', 1, 1);
                    }
                    editAdvanced->setText(strParameter);
                }
            }
        }
        // Extract advanced trend
        else if(strString.startsWith("gexReportType('adv_trend','") == true)
        {
            // Line format is:
            // gexReportType('adv_trend','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter one with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);

            if (strKey == "difference")
            {
                // Extra parameter is the second test
                strExtraParameter = strParameter.section(',', 1,1);

                // First parameter is the first test
                strParameter = strParameter.section(',', 0, 0);
            }

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);
            ConvertFromScriptString(strExtraParameter);

            if (strKey != "disabled")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_TREND));
                OnComboAdvancedReportChange();
            }

            if (strKey == "test_over_limits")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_OVERLIMITS);
            else if (strKey == "test_over_range")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_OVERDATA);
            else if (strKey == "adaptive")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_DATALIMITS);
            else if (strKey == "difference")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_DIFFERENCE);
            else if (strKey == "test_mean")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_AGGREGATE_MEAN);
            else if (strKey == "test_sigma")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_AGGREGATE_SIGMA);
            else if (strKey == "test_cp")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_AGGREGATE_CP);
            else if (strKey == "test_cpk")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_AGGREGATE_CPK);
            else if (strKey == "soft_bin_sublots")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_SOFTBIN_SBLOTS);
            else if (strKey == "soft_bin_parts")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_SOFTBIN_PARTS);
            else if (strKey == "hard_bin_sublots")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_HARDBIN_SBLOTS);
            else if (strKey == "hard_bin_parts")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_HARDBIN_PARTS);
            else if (strKey == "soft_bin_rolling")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_SOFTBIN_ROLLING);
            else if (strKey == "hard_bin_rolling")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_TREND_HARDBIN_ROLLING);

            if (! strParameter.isEmpty())
            {
                if (strParameter == "all")
                {
                    comboAdvanced->setCurrentIndex(0);
                    editAdvanced->setText("");
                    editAdvanced->hide();
                }
                else
                {
                    editAdvanced->show();
                    comboAdvanced->setCurrentIndex(1);
                    if (strParameter.startsWith("top"))
                    {
                        comboAdvanced->setCurrentIndex(2);
                        strParameter = strParameter.section(' ', 1, 1);
                    }
                    editAdvanced->setText(strParameter);
                }
            }
            else
            {
                editTestX->setText(strParameter);
                editTestY->setText(strExtraParameter);
            }
        }
        // Extract advanced correlation
        else if(strString.startsWith("gexReportType('adv_correlation','") == true)
        {
            QString strTemp;

            // Line format is:
            // gexReportType('adv_correlation','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extrat correlation parameter
            strTemp = strString.section('\'', 5, 5);

            // Extract parameter with for X axis
            strParameter = strTemp.section(' ', 0, 0);

            // Extract parameter with for Y axis
            strExtraParameter = strTemp.section(' ', 1, 1);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);
            ConvertFromScriptString(strExtraParameter);

            if (strKey != "disabled")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_CORRELATION));
                OnComboAdvancedReportChange();
            }

            if (strKey == "test_over_limits")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_CORR_OVERLIMITS);
            else if (strKey == "test_over_range")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_CORR_OVERDATA);
            else if (strKey == "adaptive")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_CORR_DATALIMITS);

            editTestX->setText(strParameter);
            editTestY->setText(strExtraParameter);
        }
        // Extract advanced box_plot
        else if(strString.startsWith("gexReportType('adv_probabilityplot','") == true)
        {
            // Line format is:
            // gexReportType('adv_probabilityplot','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter  with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey != "disabled")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_PROBABILITY_PLOT));
                OnComboAdvancedReportChange();
            }

            if (strKey == "test_over_limits")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PROBPLOT_OVERLIMITS);
            else if (strKey == "test_over_range")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PROBPLOT_OVERDATA);
            else if (strKey == "adaptive")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PROBPLOT_DATALIMITS);

            if (!strParameter.isEmpty())
			{
			    if (strParameter == "all")
				{
				    comboAdvanced->setCurrentIndex(0);
				    editAdvanced->setText("");
				    editAdvanced->hide();
				}
				else
				{
					editAdvanced->show();
					comboAdvanced->setCurrentIndex(1);
					if (strParameter.startsWith("top"))
					{
					    comboAdvanced->setCurrentIndex(2);
					    strParameter = strParameter.section(' ', 1, 1);
					}
                	editAdvanced->setText(strParameter);
				}
			}
        }
        // Extract advanced box_plot
        else if(strString.startsWith("gexReportType('adv_boxplot_ex','") == true)
        {
            // Line format is:
            // gexReportType('adv_boxplot_ex','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter  with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

			comboAdvanced->setCurrentIndex(GEX_ADV_ALL);
            if (strKey != "disabled")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_BOXPLOT));
                OnComboAdvancedReportChange();
            }

            if (strKey == "test_over_limits")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_BOXPLOT_OVERLIMITS);
            else if (strKey == "test_over_range")
			{
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_BOXPLOT_OVERDATA);
				comboAdvanced->setCurrentIndex(GEX_ADV_LIST);
				editAdvanced->setText(strParameter);
			}
            else if (strKey == "adaptive")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_BOXPLOT_DATALIMITS);

            if (!strParameter.isEmpty())
			{
			     if (strParameter == "all")
			     {
			         comboAdvanced->setCurrentIndex(0);
			         editAdvanced->setText("");
			         editAdvanced->hide();
			     }
			     else
			     {
			     editAdvanced->show();
			     comboAdvanced->setCurrentIndex(1);
			     if (strParameter.startsWith("top"))
			     {
			         comboAdvanced->setCurrentIndex(2);
			         strParameter = strParameter.section(' ', 1, 1);
			     }
                 editAdvanced->setText(strParameter);
				}
			}
        }
        // Extract advanced box plot
        else if(strString.startsWith("gexReportType('adv_boxplot','") == true)
        {
            // Line format is:
            // gexReportType('adv_boxplot','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter  with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey != "disabled")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_CANDLE_MEANRANGE));
                OnComboAdvancedReportChange();
            }

            if (strKey == "tests")
            {
                if (strParameter == "all")
				{
                    //comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_ALL);
					comboAdvanced->setCurrentIndex(0);
					editAdvanced->setText("");
					editAdvanced->hide();
				}
                else
                {
                    //comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_LIST);
				    editAdvanced->show();
					comboAdvanced->setCurrentIndex(1);
					if (strParameter.startsWith("top"))
					{
						comboAdvanced->setCurrentIndex(2);
						strParameter = strParameter.section(' ', 1, 1);
					}
                    editAdvanced->setText(strParameter);
                }
            }
        }
        // Extract advanced box_plot
        else if(strString.startsWith("gexReportType('adv_multichart','") == true)
        {
            // Line format is:
            // gexReportType('adv_multichart','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter  with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey != "disabled")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_MULTICHART));
                OnComboAdvancedReportChange();
            }

            if (strKey == "test_over_limits")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_MULTICHART_OVERLIMITS);
            else if (strKey == "test_over_range")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_MULTICHART_OVERDATA);
            else if (strKey == "adaptive")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_MULTICHART_DATALIMITS);

            if (!strParameter.isEmpty())
			{
			    if (strParameter == "all")
			    {
			        comboAdvanced->setCurrentIndex(0);
			        editAdvanced->setText("");
			        editAdvanced->hide();
			    }
			    else
			    {
			        editAdvanced->show();
			        comboAdvanced->setCurrentIndex(1);
			        if (strParameter.startsWith("top"))
			        {
			            comboAdvanced->setCurrentIndex(2);
			            strParameter = strParameter.section(' ', 1, 1);
			        }
                	editAdvanced->setText(strParameter);
				}
			}
        }
        // Extract datalog
        else if(strString.startsWith("gexReportType('adv_datalog'") == true)
        {
            // Line format is:
            // gexReportType('adv_datalog','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter  with quote cause we may hame comma in the test list
            strParameter = strString.section('\'', 5, 5);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey != "disabled")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DATALOG));
                OnComboAdvancedReportChange();
            }

            if (strKey == "all")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_DATALOG_ALL);
            else if (strKey == "outliers")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_DATALOG_OUTLIER);
            else if (strKey == "fails")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_DATALOG_FAIL);
            else if (strKey == "tests")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_DATALOG_LIST);
            else if (strKey == "tests_only")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_DATALOG_RAWDATA);

            if (!strParameter.isEmpty())
			{
                if (strParameter == "all")
                {
                    comboAdvanced->setCurrentIndex(0);
                    editAdvanced->setText("");
                    editAdvanced->hide();
                }
                else
                {
	                editAdvanced->show();
	                comboAdvanced->setCurrentIndex(1);
	                if (strParameter.startsWith("top"))
	                {
	                    comboAdvanced->setCurrentIndex(2);
	                    strParameter = strParameter.section(' ', 1, 1);
	                }
                	editAdvanced->setText(strParameter);
				}
			}
        }
        // Extract Pearson
        else if(strString.startsWith("gexReportType('adv_pearson'") == true)
        {
            comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_PEARSON));
            OnComboAdvancedReportChange();
        }
        // Extract PAT
        else if(strString.startsWith("gexReportType('adv_pat'") == true)
        {
            comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_PAT_TRACEABILITY));
            OnComboAdvancedReportChange();
        }
        // Extract PAT
        else if(strString.startsWith("gexReportType('drill_what_if'") == true)
        {
            comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_GUARDBANDING));
            OnComboAdvancedReportChange();
        }
        else if(strString.startsWith("gexReportType('adv_my_report'") == true)
        {
            // Line format is:
            // gexReportType('adv_my_report','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter one
            strParameter = strString.section(',', 2, 2);
            // Remove starting and leading ' characters
            strParameter = strParameter.section('\'', 1, 1);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey == "template")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_TEMPLATE));
                OnComboAdvancedReportChange();
                OnSelectReportTemplate(strParameter);
            }
            else
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_DISABLED));
                OnComboAdvancedReportChange();
            }
        }
        else if(strString.startsWith("gexReportType('adv_production_yield'") == true)
        {
            // Line format is:
            // gexReportType('adv_production_yield','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter one
            strParameter = strString.section(',', 2, 2);
            // Remove starting and leading ' characters
            strParameter = strParameter.section('\'', 1, 1);

            // Extract parameter two
            strExtraParameter = strString.section(',', 3, 3);
            // Remove starting and leading ' characters
            strExtraParameter = strExtraParameter.section('\'', 1, 1);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);
            ConvertFromScriptString(strExtraParameter);

            if (strKey != "disabled")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_PROD_YIELD));
                OnComboAdvancedReportChange();
            }

            if (strKey == "title")
                lineEditAdvTitle->setText(strParameter);
            else if (strKey == "sublot")
            {
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PRODYIELD_SBLOT);
                editAdvancedProduction->setText(strParameter);
            }
            else if (strKey == "lot")
            {
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PRODYIELD_LOT);
                editAdvancedProduction->setText(strParameter);
            }
            else if (strKey == "group")
            {
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PRODYIELD_GROUP);
                editAdvancedProduction->setText(strParameter);
            }
            else if (strKey == "daily")
            {
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PRODYIELD_DAY);
                editAdvancedProduction->setText(strParameter);
            }
            else if (strKey == "weekly")
            {
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PRODYIELD_WEEK);
                editAdvancedProduction->setText(strParameter);
            }
            else if (strKey == "monthly")
            {
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_PRODYIELD_MONTH);
                editAdvancedProduction->setText(strParameter);
            }
            else if (strKey == "yield_axis")
            {
                if (strParameter == "0-100")
                    comboBoxYieldAxis->setCurrentIndex(GEX_ADV_PROD_YIELDAXIS_0_100);
                else if (strParameter == "0-max")
                    comboBoxYieldAxis->setCurrentIndex(GEX_ADV_PROD_YIELDAXIS_0_MAX);
                else if (strParameter == "min-100")
                    comboBoxYieldAxis->setCurrentIndex(GEX_ADV_PROD_YIELDAXIS_MIN_100);
                else if (strParameter == "min-max")
                    comboBoxYieldAxis->setCurrentIndex(GEX_ADV_PROD_YIELDAXIS_MIN_MAX);
            }
            else if (strKey == "volume_axis")
            {
                if (strParameter == "0-max")
                    comboBoxVolumeAxis->setCurrentIndex(GEX_ADV_PROD_VOLUMEAXIS_0_MAX);
                else if (strParameter == "0-custom")
                    comboBoxVolumeAxis->setCurrentIndex(GEX_ADV_PROD_VOLUMEAXIS_0_CUSTOM);
                else if (strParameter == "min-max")
                    comboBoxVolumeAxis->setCurrentIndex(GEX_ADV_PROD_VOLUMEAXIS_MIN_MAX);
                else if (strParameter == "min-custom")
                    comboBoxVolumeAxis->setCurrentIndex(GEX_ADV_PROD_VOLUMEAXIS_MIN_CUSTOM);

                if (!strExtraParameter.isEmpty())
                    spinBoxVolumeAxis->setValue(strExtraParameter.toInt());
            }
            else if (strKey == "yield_marker")
            {
                if (strParameter == "0")
                    checkBoxYieldMarker->setChecked(false);
                else
                {
                    checkBoxYieldMarker->setChecked(true);
                    spinBoxYieldMarker->setValue(strParameter.toInt());
                }
            }
        }
        else if(strString.startsWith("gexReportType('adv_global_options'") == true)
        {
            // Line format is:
            // gexReportType('adv_global_options','

            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter one
            strParameter = strString.section(',', 2, 2);
            // Remove starting and leading ' characters
            strParameter = strParameter.section('\'', 1, 1);

            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey == "binning")
            {
                if (strParameter == "soft_bin")
                    comboAdvancedReportSettingsBinningType->setCurrentIndex(0);
                else
                    comboAdvancedReportSettingsBinningType->setCurrentIndex(1);
            }
        }
        else if(strString.startsWith("gexReportType('adv_ftr_correlation'") == true)
        {
            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);
            // Extract parameter one
            strParameter = strString.section(',', 2, 2);
            // Remove starting and leading ' characters
            strParameter = strParameter.section('\'', 1, 1);
            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);

            if (strKey != "disabled")
            {
                comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_FTR_CORRELATION));
                OnComboAdvancedReportChange();
            }

            if(strKey == "all")
			{
                comboAdvancedReportSettings->setCurrentIndex(0);
				comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_FTR_CORRELATION_ALL);
			}
            else
            {
                comboAdvancedReportSettings->setCurrentIndex(1);
				comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_FTR_CORRELATION_LIST);
                editAdvanced->setText(strParameter);
            }
        }
        else if(strString.startsWith("gexReportType('adv_functional_histogram'") == true)
        {
            // Extract Output format
            strKey	= strString.section(',', 1, 1);
            // Remove starting and leading ' characters
            strKey	= strKey.section('\'', 1, 1);

            // Extract parameter one
            strParameter = strString.section(',', 2, 2);
            // Remove starting and leading ' characters
            strParameter = strParameter.section('\'', 1, 1);
            // Decode script string.
            ConvertFromScriptString(strKey);
            ConvertFromScriptString(strParameter);
			if (strKey != "disabled")
			{
			    comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_FUNCTIONAL));
			    OnComboAdvancedReportChange();
			}

            if(strKey == "cycl_cnt")
                comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_FUNCTIONAL_CYCL_CNT);
            else if(strKey == "rel_vadr")
                 comboAdvancedReportSettings->setCurrentIndex(GEX_ADV_FUNCTIONAL_REL_VAD);
            if(strParameter == "all")
			{
                comboAdvanced->setCurrentIndex(0);
				editAdvanced->hide();
			}
            else
			{
                comboAdvanced->setCurrentIndex(1);
				if (! strParameter.isEmpty())
				{
				    editAdvanced->setText(strParameter);
				    editAdvanced->show();
				}
			}

        }
        else if(strString.startsWith("gexReportType('adv_shift'") == true)
        {
            comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_SHIFT));
            OnComboAdvancedReportChange();
        }
		else if (strString.startsWith("gexReportType('adv_functional_histogram'") == true)
		{
		    comboAdvancedReport->setCurrentIndex(comboAdvancedReport->findData(GEX_ADV_FUNCTIONAL));
		    OnComboAdvancedReportChange();
		}

    }
    while(hFile.atEnd() == false);
    file.close();

    OnComboStatisticsChange();
    OnComboWaferMapChange();
    OnComboWaferMapTestChange();
    OnComboHistogramTypeChange();
    OnComboHistogramChange();
    OnComboAdvancedReportSettingsChange();
}

///////////////////////////////////////////////////////////
// Save settings into a .csl file
///////////////////////////////////////////////////////////
void GexSettings::onSaveSettings(void)
{
    // Let's user tell where to save the settings.
    QString strFileName = QFileDialog::getSaveFileName(this, "Save settings to...", "", "Settings (*.csl)",
                                                       NULL ,QFileDialog::DontConfirmOverwrite);

    // If no file selected, ignore command.
    if(strFileName.isEmpty())
        return;

    // Make sure file name ends with ".csl" extension
    if(strFileName.endsWith(".csl",Qt::CaseInsensitive) == false)
        strFileName += ".csl";

    FILE * hFile = fopen(strFileName.toLatin1().constData(),"w");
    if(hFile == NULL)
    {
        GS::Gex::Message::
            critical("", "Failed creating file...folder is write protected?");
        return;
    }

    fprintf(hFile,"<gex_template>\n");
    fprintf(hFile,"<BlockType = settings>\n\n");
    QString strEmpty;
    WriteReportSettingsSection(hFile, false, strEmpty);
    fprintf(hFile,"\n</gex_template>\n");
    fclose(hFile);
}

void GexSettings::disableBuildReportButton(){
    QObject *poSender = sender();
    if(!poSender) return;
    if((poSender ==  toolButtonSql_Genealogy_YieldVsParameter && editSQL_Genealogy_YieldVsParameter_Product &&!editSQL_Genealogy_YieldVsParameter_Product->text().isEmpty() ) ||
            (poSender == toolButtonSql_Genealogy_YieldVsYield && editSQL_Genealogy_YieldVsYield_Product &&!editSQL_Genealogy_YieldVsYield_Product->text().isEmpty() ))
        buttonBuildReport->setEnabled(true);
    else
        buttonBuildReport->setEnabled(false);
}

void GexSettings::enableBuildReportButton(const QString &strText){

    buttonBuildReport->setEnabled(!strText.isEmpty());

}



void GexMainwindow::exportFTRCorrelationReport()
{
    // Let's user tell where to save the configuration.
    QString strFileName = QFileDialog::getSaveFileName(this, "Export Examinator Report as...", "ftr_correlation.csv",
                                                       "CSV file (*.csv)");

    // If no file selected, ignore command.
    if(strFileName.isEmpty())
        return;
    GS::Gex::FTRCorrelationReport *poObj = (GS::Gex::FTRCorrelationReport *)gexReport->getReportUnit("adv_ftr_correlation");
    poObj->exportToCSV(strFileName);
}
