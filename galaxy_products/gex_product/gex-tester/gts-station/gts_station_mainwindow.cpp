/****************************************************************************
** GTS MainWindow implementation
****************************************************************************/
#include <QApplication>
#include <QFileInfo>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QCloseEvent>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QMimeData>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <gstdl_utils.h>
#include <gstdl_utils_c.h>
#include <gstdl_systeminfo.h>
#include <gqtl_datakeys.h>

#include "gtl_core.h"
#include "gts_station_mainwindow.h"
#include "gts_station_infowidget.h"
#include "gts_station_statswidget.h"
#include "gts_station_outputwidget.h"
#include "gts_station_messagedialog.h"
#include "gts_station_gtlcommandsdialog.h"
#include "gts_station_newlotdialog.h"
#include "gts_station_setupwidget.h"
#include "gts_station_gtlwidget.h"


extern void NormalizePath(QString& strPath);	// Normalize path (unix/windows)

// Some constants
#define GTSTSATION_BUTTON_LOAD_TEXT			"&Load..."
#define GTSTSATION_BUTTON_DELETE_TEXT		"&Delete..."

// Benchmark mode
#define BENCHMARK_PASSES 20

/////////////////////////////////////////////////////////////////////////////////////////
// Test Definition OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
GtsStation_TestDef::GtsStation_TestDef(GQTL_STDF::Stdf_PTR_V4 *PtrRecord) : mPTR(PtrRecord), mMPR(NULL)
{
    GSLOG(5, "new GtsStation_TestDef");
    mForceNewLimits = false;
    mNewLimitsValid = false;
    mExecCount = 0;
    mFailCount = 0;
}

GtsStation_TestDef::GtsStation_TestDef(GQTL_STDF::Stdf_MPR_V4 *MprRecord) : mPTR(NULL), mMPR(MprRecord)
{
    GSLOG(5, "new GtsStation_TestDef");
    mForceNewLimits = false;
    mNewLimitsValid = false;
    mExecCount = 0;
    mFailCount = 0;
}

GtsStation_TestDef::~GtsStation_TestDef()
{
    if(mPTR)
    {
        delete mPTR;
        mPTR=NULL;
    }
    if(mMPR)
    {
        delete mMPR;
        mMPR=NULL;
    }
}

QString GtsStation_TestDef::GetTestName()
{
    if(mPTR)
        return QString(mPTR->m_cnTEST_TXT);
    if(mMPR)
        return QString(mMPR->m_cnTEST_TXT);
    return QString("");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Function to check if test has a LL
/////////////////////////////////////////////////////////////////////////////////////////
bool GtsStation_TestDef::HasLowLimit()
{
    if(mPTR)
        return mPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposLO_LIMIT);
    if(mMPR)
        return mMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposLO_LIMIT);
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Function to check if test has a HL
/////////////////////////////////////////////////////////////////////////////////////////
bool GtsStation_TestDef::HasHighLimit()
{
    if(mPTR)
        return mPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposHI_LIMIT);
    if(mMPR)
        return mMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposHI_LIMIT);
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Update Test definition with current PTR, and eventually
// update current PTR if new limits forced by test program (or PAT library)
/////////////////////////////////////////////////////////////////////////////////////////
bool GtsStation_TestDef::Update(GQTL_STDF::Stdf_PTR_V4* CurrentPTR)
{
    if(!mPTR)
        return false;

    bool ForceNewLimitsInCurrentPTR = false;

	// Update test limits in Reference PTR if need be (if current PTR has valid limits)
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposOPT_FLAG))
        mPTR->SetOPT_FLAG(CurrentPTR->m_b1OPT_FLAG);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposRES_SCAL))
        mPTR->SetRES_SCAL(CurrentPTR->m_i1RES_SCAL);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposLLM_SCAL))
        mPTR->SetLLM_SCAL(CurrentPTR->m_i1LLM_SCAL);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposHLM_SCAL))
        mPTR->SetHLM_SCAL(CurrentPTR->m_i1HLM_SCAL);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposLO_LIMIT))
        mPTR->SetLO_LIMIT(CurrentPTR->m_r4LO_LIMIT);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposHI_LIMIT))
        mPTR->SetHI_LIMIT(CurrentPTR->m_r4HI_LIMIT);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposUNITS))
        mPTR->SetUNITS(CurrentPTR->m_cnUNITS);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposC_RESFMT))
        mPTR->SetC_RESFMT(CurrentPTR->m_cnC_RESFMT);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposC_LLMFMT))
        mPTR->SetC_LLMFMT(CurrentPTR->m_cnC_LLMFMT);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposC_HLMFMT))
        mPTR->SetC_HLMFMT(CurrentPTR->m_cnC_HLMFMT);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposLO_SPEC))
        mPTR->SetLO_SPEC(CurrentPTR->m_r4LO_SPEC);
    if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposHI_SPEC))
        mPTR->SetHI_SPEC(CurrentPTR->m_r4HI_SPEC);

	// Check if limits updated by test program (or PAT library)
    if(HasLowLimit() && mForceNewLimits)
	{
        mPTR->SetLO_LIMIT(mNewLowLimit);
        mForceNewLimits = false;
        mNewLimitsValid = true;
        ForceNewLimitsInCurrentPTR = true;
	}
    if(HasHighLimit() && mForceNewLimits)
	{
        mPTR->SetHI_LIMIT(mNewHighLimit);
        mForceNewLimits = false;
        mNewLimitsValid = true;
        ForceNewLimitsInCurrentPTR = true;
	}

	// Force new limits current PTR?
    if(ForceNewLimitsInCurrentPTR)
	{
		// Force new limits, even if current PTR has no limits
        GQTL_STDF::Stdf_PTR_V4 lTmpPtr = *CurrentPTR;
        *CurrentPTR = *mPTR;
        if(lTmpPtr.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposSITE_NUM))
            CurrentPTR->SetSITE_NUM(lTmpPtr.m_u1SITE_NUM);
        if(lTmpPtr.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposTEST_FLG))
            CurrentPTR->SetTEST_FLG(lTmpPtr.m_b1TEST_FLG);
        if(lTmpPtr.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposPARM_FLG))
            CurrentPTR->SetPARM_FLG(lTmpPtr.m_b1PARM_FLG);
        if(lTmpPtr.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposRESULT))
            CurrentPTR->SetRESULT(lTmpPtr.m_r4RESULT);
        if(lTmpPtr.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposALARM_ID))
            CurrentPTR->SetALARM_ID(lTmpPtr.m_cnALARM_ID);
	}
    else if(mNewLimitsValid)
	{
		// Overwrite limits if current PTR has limits
        if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposLO_LIMIT))
            CurrentPTR->SetLO_LIMIT(mNewLowLimit);
        if(CurrentPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposHI_LIMIT))
            CurrentPTR->SetHI_LIMIT(mNewHighLimit);
	}

	// If new limits set by test program (or PAT library), update Pass/Fail status for current PTR
    if(mNewLimitsValid)
        CurrentPTR->UpdatePassFailInfo(*mPTR);

	// Update counters
    mExecCount++;
    if(CurrentPTR->IsTestFail(*mPTR))
	{
        mFailCount++;
        return false;
	}

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Update Test definition with current MPR, and eventually
// update current MPR if new limits forced by test program (or PAT library)
/////////////////////////////////////////////////////////////////////////////////////////
bool GtsStation_TestDef::Update(GQTL_STDF::Stdf_MPR_V4* CurrentMPR)
{
    if(!mMPR)
        return false;

    bool ForceNewLimitsInCurrentMPR = false;

    // Update test limits in Reference MPR if need be (if current MPR has valid limits)
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposOPT_FLAG))
        mMPR->SetOPT_FLAG(CurrentMPR->m_b1OPT_FLAG);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposRES_SCAL))
        mMPR->SetRES_SCAL(CurrentMPR->m_i1RES_SCAL);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposLLM_SCAL))
        mMPR->SetLLM_SCAL(CurrentMPR->m_i1LLM_SCAL);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposHLM_SCAL))
        mMPR->SetHLM_SCAL(CurrentMPR->m_i1HLM_SCAL);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposLO_LIMIT))
        mMPR->SetLO_LIMIT(CurrentMPR->m_r4LO_LIMIT);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposHI_LIMIT))
        mMPR->SetHI_LIMIT(CurrentMPR->m_r4HI_LIMIT);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposUNITS))
        mMPR->SetUNITS(CurrentMPR->m_cnUNITS);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposC_RESFMT))
        mMPR->SetC_RESFMT(CurrentMPR->m_cnC_RESFMT);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposC_LLMFMT))
        mMPR->SetC_LLMFMT(CurrentMPR->m_cnC_LLMFMT);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposC_HLMFMT))
        mMPR->SetC_HLMFMT(CurrentMPR->m_cnC_HLMFMT);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposLO_SPEC))
        mMPR->SetLO_SPEC(CurrentMPR->m_r4LO_SPEC);
    if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposHI_SPEC))
        mMPR->SetHI_SPEC(CurrentMPR->m_r4HI_SPEC);

    // Check if limits updated by test program (or PAT library)
    if(HasLowLimit() && mForceNewLimits)
    {
        mMPR->SetLO_LIMIT(mNewLowLimit);
        mForceNewLimits = false;
        mNewLimitsValid = true;
        ForceNewLimitsInCurrentMPR = true;
    }
    if(HasHighLimit() && mForceNewLimits)
    {
        mMPR->SetHI_LIMIT(mNewHighLimit);
        mForceNewLimits = false;
        mNewLimitsValid = true;
        ForceNewLimitsInCurrentMPR = true;
    }

    // Force new limits current MPR?
    if(ForceNewLimitsInCurrentMPR)
    {
        // Force new limits, even if current MPR has no limits
        GQTL_STDF::Stdf_MPR_V4 lTmpMpr = *CurrentMPR;
        *CurrentMPR = *mMPR;
        if(lTmpMpr.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposSITE_NUM))
            CurrentMPR->SetSITE_NUM(lTmpMpr.m_u1SITE_NUM);
        if(lTmpMpr.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposTEST_FLG))
            CurrentMPR->SetTEST_FLG(lTmpMpr.m_b1TEST_FLG);
        if(lTmpMpr.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposPARM_FLG))
            CurrentMPR->SetPARM_FLG(lTmpMpr.m_b1PARM_FLG);
        if(lTmpMpr.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposRTN_RSLT))
            CurrentMPR->SetRTN_RSLT(lTmpMpr);
        if(lTmpMpr.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposALARM_ID))
            CurrentMPR->SetALARM_ID(lTmpMpr.m_cnALARM_ID);
    }
    else if(mNewLimitsValid)
    {
        // Overwrite limits if current MPR has limits
        if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposLO_LIMIT))
            CurrentMPR->SetLO_LIMIT(mNewLowLimit);
        if(CurrentMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposHI_LIMIT))
            CurrentMPR->SetHI_LIMIT(mNewHighLimit);
    }

    // If new limits set by test program (or PAT library), update Pass/Fail status for current MPR
    if(mNewLimitsValid)
        CurrentMPR->UpdatePassFailInfo(*mMPR);

    // Update counters
    mExecCount++;
    if(CurrentMPR->IsTestFail(*mMPR))
    {
        mFailCount++;
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Station MainWindow OBJECT
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Constructs a GtsStationMainwindow as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
/////////////////////////////////////////////////////////////////////////////////////
GtsStationMainwindow::GtsStationMainwindow(QApplication* qApplication, const QString & ApplicationPath,
                                           const QString & ApplicationName, QWidget *parent)
    : QMainWindow(parent)//, mHelpPage(widgetStack)
{
    GSLOG(5, "Seting up GUI...");

    // Setup UI
    setupUi(this);

    GSLOG(SYSLOG_SEV_DEBUG, "GtsStationMainwindow constructor...");

    // Initialize properties
    mProperties.insert(GTS_PROPERTY_STATION_NB, 1);
    mProperties.insert(GTS_PROPERTY_HIDDEN, false);
    mProperties.insert(GTS_PROPERTY_AUTO_CLOSE, false);
    mProperties.insert(GTS_PROPERTY_AUTO_TESTNUMBERS, false);
    mProperties.insert(GTS_PROPERTY_AUTO_RUN, false);
    mProperties.insert(GTS_PROPERTY_RUNNING_MODE, GtsStationMainwindow::Normal);
    mProperties.insert(GTS_PROPERTY_RUN_DELAY, 100);    // default is 100ms between each run
    mProperties.insert(GTS_PROPERTY_STOP_DELAY, 2000);  // default is 2sec before stop

    // Last test number used when test number are auto-incremented
    mLastAutoTestNumber = 0;

    // No mode gtl_endlot() supported in GTL V3.5, so hide NewLot button for now
    // (button hidden in GUI)
    buttonNewlot->hide();

    // Signal/Slot connections
    connect( buttonLoadDelete, SIGNAL(clicked()),this, SLOT(OnButtonLoadDelete()));
    connect( buttonDatalog, SIGNAL(clicked()),this, SLOT(OnButtonDatalog()));
    connect( buttonExit, SIGNAL(clicked()),this, SLOT(OnButtonExit()));
    connect( buttonHelp, SIGNAL(clicked()),this, SLOT(OnButtonHelp()));
    connect( buttonReset, SIGNAL(clicked()),this, SLOT(OnButtonReset()));
    connect( buttonRun, SIGNAL(clicked()),this, SLOT(OnButtonRun()));
    connect( listMainSelection, SIGNAL(itemSelectionChanged()),this, SLOT(OnMainSelectionChanged()));
    connect( buttonRunN, SIGNAL(clicked()),this, SLOT(OnButtonRunN()));
    connect( buttonGtlCommads, SIGNAL(clicked()),this, SLOT(OnButtonGtlCommands()));
    connect( buttonNewlot, SIGNAL(clicked()),this, SLOT(OnButtonNewlot()));
    connect( buttonRunAll, SIGNAL(clicked()),this, SLOT(OnButtonRunAll()));
    connect( buttonRecipe, SIGNAL(clicked()),this, SLOT(OnButtonRecipe()));
    connect( buttonTesterConf, SIGNAL(clicked()),this, SLOT(OnButtonTesterConf()));

    // Initialize data
    m_qApplication = qApplication;
    m_strApplicationPath = ApplicationPath;
    m_strApplicationName = ApplicationName;
    m_qMessageBox.setIconPixmap(QPixmap(":/images/GalaxyLogo22"));

    m_ui_gtl_test_lib = 0;
    m_ui_gtl_mptest_lib = 0;
    m_ui_gtl_close_lib = 0;
    m_ui_gtl_beginjob_lib = 0;
    m_ui_gtl_endjob_lib = 0;
    m_ui_gtl_binning_lib = 0;

	// Create widgets to add to widget stack
    m_pageInfo = new GtsStationInfowidget(widgetStack);
    m_pageStats = new GtsStationStatswidget(widgetStack);
    m_pageSetup = new GtsStationSetupwidget(widgetStack);
    m_pageOutput = new GtsStationOutputwidget(m_pageSetup, widgetStack);
    m_pageGtlWidget = new GtsStationGtlWidget(widgetStack);

    // Add connections
    connect( m_pageInfo->editProgramName, SIGNAL(textChanged(const QString &)),this, SLOT(OnJobNameChanged(const QString &)));

	// Add page selections
	unsigned int uiPageNb = 0;
    buttonHelp->hide();
#if 0
    AddPage(tr( "Help" ), QPixmap(":/images/HelpIcon"), pageHelp, false);
	m_uiPage_Help = uiPageNb++;
#endif
    AddPage(tr( "Main" ), QPixmap(":/images/HomeIcon"), m_pageInfo, true);
	m_uiPage_Main = uiPageNb++;
    AddPage(tr( "Stats" ), QPixmap(":/images/StatsIcon"), m_pageStats, false);
	m_uiPage_Stats = uiPageNb++;
    AddPage(tr( "Output" ), QPixmap(":/images/OutputIcon"), m_pageOutput, false);
	m_uiPage_Output = uiPageNb++;
    AddPage(tr( "Setup" ), QPixmap(":/images/SetupIcon"), m_pageSetup, true);
	m_uiPage_Setup = uiPageNb++;
    AddPage(tr( "GTL" ), QPixmap(":/images/GtlIcon"), m_pageGtlWidget, true);
    m_uiPage_GtlWidget = uiPageNb++;

    // Help page
    mHelpPage.setParent(widgetStack);
    AddPage(tr( "Help" ), QPixmap(":/images/HelpIcon"), &mHelpPage, false);
    mHelpPage.setReadOnly(true);
    mHelpPage.setText(
                "Command line arguments:\n"
                " --gtm_communication=synchronous : the communication between GTL and GTM will be sync: default in assynchronous.\n"
                " --hidden\n"
                " --multifile_mode=...\n"
                " --desired_limits=...\n"
                " --rundelay=(delay in ms)\n"
                "\nEnvironment variables:\n"
                "GTL_LOGLEVEL : log level of the GTL used by the GTS\n"
                "GTS_LOGLEVEL : log level of the GTS (usually useless)\n"
                "GS_GTS_DATAFOLDER : path to a folder containing any datafiles\n"
                "GS_GTS_STDF : ?\n"
                "GS_GTS_RECIPE : path to a recipe\n"
                "GS_GTS_TESTERCONF : path to .conf GTL file\n"
                "GS_QA : GalaxySemi QA mode. On if variable defined. If on, needs:\n"
                "- GS_QA_OUTPUT_FOLDER : ?\n"
                );

#if 0
  // Set the source of html help viewer
	QString strHtmlSource = m_strApplicationPath + "/help/pages/gts-station_help.htm";
	NormalizePath(strHtmlSource);
	textBrowser_Help->setSource(strHtmlSource);
#endif

	// Display 'Main' page on startup
    listMainSelection->setCurrentItem(listMainSelection->item(m_uiPage_Main));

	// Set some dialog options
    setAcceptDrops(true);		// Support for Drag&Drop

	// Clear output widget
	m_pageOutput->Reset();

    GSLOG(SYSLOG_SEV_DEBUG, "Exiting GtsStationMainwindow constructor...");
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
GtsStationMainwindow::~GtsStationMainwindow()
{
    // no need to delete child widgets, Qt does it all for us

	// Reset station to free ressources
	ClearStation();

	// Close STDF files
	mStdfReader.Close();
	mStdfWriter.Close();

    // Clear map of stdf files
    mStdfFiles.clear();
}

bool GtsStationMainwindow::InitGts(int argc, char **argv)
{
    GSLOG(5, QString("Init GTS (%1 args)...").arg(argc).toLatin1().data() );

    // Parse arguments and check env. variables
    bool lStatus = CheckArguments(argc, argv);
    if(!lStatus)
    {
        Error(QString("Error parsing arguments, the application will now exit!"), SYSLOG_SEV_CRITICAL);
        return false;
    }

    // Clear Station
    ClearStation();

    // Update GUI depending on some properties
    if(isAutoRun())
    {
        m_pageSetup->checkBoxGenerateSTDFFile->setChecked(false);
        m_pageSetup->checkBoxGenerateSTDFFile->setEnabled(false);
        m_pageSetup->checkBoxEnableOutput->setChecked(true);
        m_pageSetup->checkBoxEnableOutput->setEnabled(false);
        m_pageSetup->checkBoxStopOnFailMode->setChecked(true);
        m_pageSetup->checkBoxStopOnFailMode->setEnabled(false);
    }
    else if(isRunningModeBench())
    {
        listMainSelection->setEnabled(false);
        m_pageSetup->checkBoxGenerateSTDFFile->setChecked(false);
        m_pageSetup->checkBoxGenerateSTDFFile->setEnabled(false);
        m_pageSetup->checkBoxEnableOutput->setChecked(false);
        m_pageSetup->checkBoxEnableOutput->setEnabled(false);
        m_pageSetup->checkBoxStopOnFailMode->setChecked(true);
        m_pageSetup->checkBoxStopOnFailMode->setEnabled(false);
    }
    else
    {
        m_pageSetup->checkBoxGenerateSTDFFile->setChecked(false);
        m_pageSetup->checkBoxGenerateSTDFFile->setEnabled(true);
        m_pageSetup->checkBoxEnableOutput->setChecked(true);
        m_pageSetup->checkBoxEnableOutput->setEnabled(true);
        m_pageSetup->checkBoxStopOnFailMode->setChecked(true);
        m_pageSetup->checkBoxStopOnFailMode->setEnabled(true);
        m_pageInfo->labelBenchmark->hide();
        m_pageInfo->editBenchPatEnabled->hide();
        m_pageInfo->editBenchPatDisabled->hide();
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Clear station
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::ClearStation()
{
    GtsTestlistMap::Iterator    it;
    GtsStation_TestDef          *pclTestDef;

	// Set Caption and icon: app name + station nb
    setWindowTitle(QString("%1 - Station %2").arg(m_strApplicationName).arg(StationNb()));
    setWindowIcon(QIcon(QPixmap(":/images/StationOffIcon")));

	// Free allocated ressources
	m_listSites.clear();

	for(it = m_mapTestlist.begin(); it != m_mapTestlist.end(); ++it) 
	{
        pclTestDef = it.value();
		delete pclTestDef;
	}	
	m_mapTestlist.clear();

	// Reset variables
    m_bProgramLoaded = false;
	m_strStdfFullFileName = "";
	m_strStdfFullFileName_Out = "";
	m_strStdfFileName = "";
	m_strStdfFilePath = "";
    m_bUseLastPIR = false;
	m_uiTotalPartsInSTDF = 0;
	m_nStdf_CPU_Type = 1;
    m_bPartIsFail = false;
    m_ui_gtl_test_lib = 0;
    m_ui_gtl_mptest_lib = 0;
    m_ui_gtl_close_lib = 0;
    m_ui_gtl_beginjob_lib = 0;
    m_ui_gtl_endjob_lib = 0;
    m_ui_gtl_binning_lib = 0;

    if(!mProperties.value(GTS_PROPERTY_RECIPE_FILE).toString().isEmpty())
        editRecipe->setText(mProperties.value(GTS_PROPERTY_RECIPE_FILE).toString());
    if(!mProperties.value(GTS_PROPERTY_TC_FILE).toString().isEmpty())
        editTesterConf->setText(mProperties.value(GTS_PROPERTY_TC_FILE).toString());

    // Enable/disable buttons
    if(isAutoRun())
    {
        buttonLoadDelete->setText("&Start");
        buttonLoadDelete->setEnabled(false);
        buttonRun->setEnabled(false);
        buttonRunN->setEnabled(false);
        spinRunN->setEnabled(false);
        buttonRunAll->setEnabled(false);
        buttonGtlCommads->setEnabled(false);
        buttonNewlot->setEnabled(false);
        buttonReset->setEnabled(false);
        buttonDatalog->setEnabled(false);
        buttonHelp->setEnabled(false);
        buttonExit->setEnabled(true);
    }
    else if(isRunningModeBench())
    {
        buttonLoadDelete->setText("&Start");
        buttonLoadDelete->setEnabled(true);
        buttonRun->setEnabled(false);
        buttonRunN->setEnabled(false);
        spinRunN->setEnabled(false);
        buttonRunAll->setEnabled(false);
        buttonGtlCommads->setEnabled(false);
        buttonNewlot->setEnabled(false);
        buttonReset->setEnabled(false);
        buttonDatalog->setEnabled(false);
        buttonHelp->setEnabled(false);
        buttonExit->setEnabled(true);
    }
    else
    {
        buttonLoadDelete->setText(GTSTSATION_BUTTON_LOAD_TEXT);
        buttonLoadDelete->setEnabled(true);
        buttonRun->setEnabled(false);
        buttonRunN->setEnabled(false);
        spinRunN->setEnabled(false);
        buttonRunAll->setEnabled(false);
        buttonGtlCommads->setEnabled(true);
        buttonNewlot->setEnabled(false);
        buttonReset->setEnabled(false);
        buttonDatalog->setEnabled(false);
        buttonHelp->setEnabled(true);
        buttonExit->setEnabled(true);
    }

	// Reset part count and binning
	textPartsTested->setText("<p align=\"center\">n/a</p>");
    textGtlState->setText("<p align=\"center\">n/a</p>");
    lCDSite0Bin->setEnabled(false);
	lCDSite0Bin->setProperty("value", 0);
    lCDSite1Bin->setEnabled(false);
	lCDSite1Bin->setProperty("value", 0);
    lCDSite2Bin->setEnabled(false);
	lCDSite2Bin->setProperty("value", 0);
    lCDSite3Bin->setEnabled(false);
	lCDSite3Bin->setProperty("value", 0);
    lCDSite4Bin->setEnabled(false);
	lCDSite4Bin->setProperty("value", 0);
    lCDSite5Bin->setEnabled(false);
	lCDSite5Bin->setProperty("value", 0);
    lCDSite6Bin->setEnabled(false);
	lCDSite6Bin->setProperty("value", 0);
    lCDSite7Bin->setEnabled(false);
	lCDSite7Bin->setProperty("value", 0);
    lCDSite8Bin->setEnabled(false);
    lCDSite8Bin->setProperty("value", 0);
    lCDSite9Bin->setEnabled(false);
    lCDSite9Bin->setProperty("value", 0);
    lCDSite10Bin->setEnabled(false);
    lCDSite10Bin->setProperty("value", 0);
    lCDSite11Bin->setEnabled(false);
    lCDSite11Bin->setProperty("value", 0);
    lCDSite12Bin->setEnabled(false);
    lCDSite12Bin->setProperty("value", 0);
    lCDSite13Bin->setEnabled(false);
    lCDSite13Bin->setProperty("value", 0);
    lCDSite14Bin->setEnabled(false);
    lCDSite14Bin->setProperty("value", 0);
    lCDSite15Bin->setEnabled(false);
    lCDSite15Bin->setProperty("value", 0);
    lCDSite16Bin->setEnabled(false);
    lCDSite16Bin->setProperty("value", 0);
    lCDSite17Bin->setEnabled(false);
    lCDSite17Bin->setProperty("value", 0);
    lCDSite18Bin->setEnabled(false);
    lCDSite18Bin->setProperty("value", 0);
    lCDSite19Bin->setEnabled(false);
    lCDSite19Bin->setProperty("value", 0);
    lCDSite20Bin->setEnabled(false);
    lCDSite20Bin->setProperty("value", 0);
    lCDSite21Bin->setEnabled(false);
    lCDSite21Bin->setProperty("value", 0);
    lCDSite22Bin->setEnabled(false);
    lCDSite22Bin->setProperty("value", 0);
    lCDSite23Bin->setEnabled(false);
    lCDSite23Bin->setProperty("value", 0);
    lCDSite24Bin->setEnabled(false);
    lCDSite24Bin->setProperty("value", 0);
    lCDSite25Bin->setEnabled(false);
    lCDSite25Bin->setProperty("value", 0);
    lCDSite26Bin->setEnabled(false);
    lCDSite26Bin->setProperty("value", 0);
    lCDSite27Bin->setEnabled(false);
    lCDSite27Bin->setProperty("value", 0);
    lCDSite28Bin->setEnabled(false);
    lCDSite28Bin->setProperty("value", 0);
    lCDSite29Bin->setEnabled(false);
    lCDSite29Bin->setProperty("value", 0);
    lCDSite30Bin->setEnabled(false);
    lCDSite30Bin->setProperty("value", 0);
    lCDSite31Bin->setEnabled(false);
    lCDSite31Bin->setProperty("value", 0);

	// Reset widgets
	m_pageInfo->Reset();
	m_pageStats->Reset();

	// Close STDF files
	mStdfReader.Close();
	mStdfWriter.Close();

    // Clear map of stdf files
    mStdfFiles.clear();

    UpdateSummary();

	// Set status to 'Ready'
	SetStatus(StatusReady);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Reset station
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::ResetStation()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Reset station.");

    // Reset sites
	m_listSites.ResetAll();

	// Reset variables
    m_bUseLastPIR = false;
    m_bPartIsFail = false;
    m_ui_gtl_test_lib = 0;
    m_ui_gtl_mptest_lib = 0;
    m_ui_gtl_close_lib = 0;
    m_ui_gtl_beginjob_lib = 0;
    m_ui_gtl_endjob_lib = 0;
    m_ui_gtl_binning_lib = 0;

	// Reset part count and binning
	textPartsTested->setText("<p align=\"center\">n/a</p>");
    textGtlState->setText("<p align=\"center\">n/a</p>");
    lCDSite0Bin->setEnabled(false);
    lCDSite0Bin->setProperty("value", 0);
    lCDSite1Bin->setEnabled(false);
    lCDSite1Bin->setProperty("value", 0);
    lCDSite2Bin->setEnabled(false);
    lCDSite2Bin->setProperty("value", 0);
    lCDSite3Bin->setEnabled(false);
    lCDSite3Bin->setProperty("value", 0);
    lCDSite4Bin->setEnabled(false);
    lCDSite4Bin->setProperty("value", 0);
    lCDSite5Bin->setEnabled(false);
    lCDSite5Bin->setProperty("value", 0);
    lCDSite6Bin->setEnabled(false);
    lCDSite6Bin->setProperty("value", 0);
    lCDSite7Bin->setEnabled(false);
    lCDSite7Bin->setProperty("value", 0);
    lCDSite8Bin->setEnabled(false);
    lCDSite8Bin->setProperty("value", 0);
    lCDSite9Bin->setEnabled(false);
    lCDSite9Bin->setProperty("value", 0);
    lCDSite10Bin->setEnabled(false);
    lCDSite10Bin->setProperty("value", 0);
    lCDSite11Bin->setEnabled(false);
    lCDSite11Bin->setProperty("value", 0);
    lCDSite12Bin->setEnabled(false);
    lCDSite12Bin->setProperty("value", 0);
    lCDSite13Bin->setEnabled(false);
    lCDSite13Bin->setProperty("value", 0);
    lCDSite14Bin->setEnabled(false);
    lCDSite14Bin->setProperty("value", 0);
    lCDSite15Bin->setEnabled(false);
    lCDSite15Bin->setProperty("value", 0);
    lCDSite16Bin->setEnabled(false);
    lCDSite16Bin->setProperty("value", 0);
    lCDSite17Bin->setEnabled(false);
    lCDSite17Bin->setProperty("value", 0);
    lCDSite18Bin->setEnabled(false);
    lCDSite18Bin->setProperty("value", 0);
    lCDSite19Bin->setEnabled(false);
    lCDSite19Bin->setProperty("value", 0);
    lCDSite20Bin->setEnabled(false);
    lCDSite20Bin->setProperty("value", 0);
    lCDSite21Bin->setEnabled(false);
    lCDSite21Bin->setProperty("value", 0);
    lCDSite22Bin->setEnabled(false);
    lCDSite22Bin->setProperty("value", 0);
    lCDSite23Bin->setEnabled(false);
    lCDSite23Bin->setProperty("value", 0);
    lCDSite24Bin->setEnabled(false);
    lCDSite24Bin->setProperty("value", 0);
    lCDSite25Bin->setEnabled(false);
    lCDSite25Bin->setProperty("value", 0);
    lCDSite26Bin->setEnabled(false);
    lCDSite26Bin->setProperty("value", 0);
    lCDSite27Bin->setEnabled(false);
    lCDSite27Bin->setProperty("value", 0);
    lCDSite28Bin->setEnabled(false);
    lCDSite28Bin->setProperty("value", 0);
    lCDSite29Bin->setEnabled(false);
    lCDSite29Bin->setProperty("value", 0);
    lCDSite30Bin->setEnabled(false);
    lCDSite30Bin->setProperty("value", 0);
    lCDSite31Bin->setEnabled(false);
    lCDSite31Bin->setProperty("value", 0);

	// Update widgets
    m_pageInfo->Update(m_strStdfFileName, mKeysContent);
	m_pageStats->UpdateGUI(m_listSites);

    // Clear map of stdf files
    //mStdfFiles.clear();

    // Close and re-open STDF file (rewind doesn't work once too much data read)
	mStdfReader.Close();
    mStdfReader.Open(m_strStdfFullFileName.toLatin1().constData());
	
	// Open STDF output file
	SetStdfOutputFileName();
	mStdfReader.GetCpuType(&m_nStdf_CPU_Type);
	mStdfWriter.Close();
    if(m_pageSetup->IsStdfON())
	{
		m_bGenerateStdfFile = true;
        mStdfWriter.Open(m_strStdfFullFileName_Out.toLatin1().constData(), STDF_WRITE, m_nStdf_CPU_Type);
	}
	else
		m_bGenerateStdfFile = false;

	// Set status to 'Ready'
	SetStatus(StatusReady);
}

int GetGTLLibState()
{
    char lLibStateString[256]="?";
    if (gtl_get(GTL_KEY_LIB_STATE, lLibStateString)!=0)
        return -1;
    int lLibState=0;
    bool lOk=false;
    lLibState=QString(lLibStateString).toInt(&lOk);
    if (!lOk)
        return -1;

    return lLibState;
}

void GtsStationMainwindow::ModeCode(QChar & cModeCode)
{
    // Assign a space by default
    cModeCode = QChar(' ');

    // Make sure data is loaded
    if(!m_bProgramLoaded)
        return;

    // Get key value and make sure it is OK
    QVariant lValue = mKeysContent.Get("DataType");
    if(lValue.isNull() || !lValue.isValid())
        return;

    // If Qvariant is a char, get it
    QMetaType::Type t = QMetaType::Type(lValue.type());
    if(t == QMetaType::QChar)
    {
        QChar lMC = lValue.toChar();
        if(!lMC.isNull())
            cModeCode = lMC;
        return;
    }

    // If QVariant is a 1-character string, get that character
    if(t == QMetaType::QString)
    {
        QString lMC = lValue.toString();
        if(lMC.length() == 1)
            cModeCode = lMC.at(0);
        return;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update GTL state in GUI
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::UpdateGtlState()
{
    if(isRunningModeBench())
        return;
    int lLibState=GetGTLLibState();

    // Update GTL state
    switch(lLibState)
    {
        case GTL_STATE_NOT_INITIALIZED:
            textGtlState->setText(QString("<p align=\"center\">GTL_STATE_NOT_INITIALIZED</p>"));
            break;
        case GTL_STATE_ENABLED:
            textGtlState->setText(QString("<p align=\"center\">GTL_STATE_ENABLED</p>"));
            break;
        case GTL_STATE_OFFLINE:
            textGtlState->setText(QString("<p align=\"center\">GTL_STATE_OFFLINE</p>"));
            break;
        case GTL_STATE_DISABLED:
            textGtlState->setText(QString("<p align=\"center\">GTL_STATE_DISABLED</p>"));
            break;
        default:
            textGtlState->setText(QString("<p align=\"center\">UNKNOWN</p>"));
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Update summary (parts tested and binning)
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::UpdateSummary()
{
    if(isRunningModeBench())
        return;

    GtsStation_Site*    lSite=NULL;
    int                 nBinning;

    // Update GTL state
    UpdateGtlState();

    // Set part tested text
    textPartsTested->setText(QString("<p align=\"center\">%1 out of %2</p>").arg(PartCount()).arg(m_uiTotalPartsInSTDF));

	// Set binnings
    for(int lIndex=0; lIndex<m_listSites.size(); ++lIndex)
	{
        lSite = m_listSites.at(lIndex);
        nBinning = lSite->m_nBinning_Soft;
        if(lSite->m_nBinning_Hard != -1)
        {
            switch(lSite->m_uiSiteNb)
            {
                case 0:
                    lCDSite0Bin->setEnabled(true);
                    lCDSite0Bin->setProperty("value", nBinning);
                    break;
                case 1:
                    lCDSite1Bin->setEnabled(true);
                    lCDSite1Bin->setProperty("value", nBinning);
                    break;
                case 2:
                    lCDSite2Bin->setEnabled(true);
                    lCDSite2Bin->setProperty("value", nBinning);
                    break;
                case 3:
                    lCDSite3Bin->setEnabled(true);
                    lCDSite3Bin->setProperty("value", nBinning);
                    break;
                case 4:
                    lCDSite4Bin->setEnabled(true);
                    lCDSite4Bin->setProperty("value", nBinning);
                    break;
                case 5:
                    lCDSite5Bin->setEnabled(true);
                    lCDSite5Bin->setProperty("value", nBinning);
                    break;
                case 6:
                    lCDSite6Bin->setEnabled(true);
                    lCDSite6Bin->setProperty("value", nBinning);
                    break;
                case 7:
                    lCDSite7Bin->setEnabled(true);
                    lCDSite7Bin->setProperty("value", nBinning);
                    break;
                case 8:
                    lCDSite8Bin->setEnabled(true);
                    lCDSite8Bin->setProperty("value", nBinning);
                    break;
                case 9:
                    lCDSite9Bin->setEnabled(true);
                    lCDSite9Bin->setProperty("value", nBinning);
                    break;
                case 10:
                    lCDSite10Bin->setEnabled(true);
                    lCDSite10Bin->setProperty("value", nBinning);
                    break;
                case 11:
                    lCDSite11Bin->setEnabled(true);
                    lCDSite11Bin->setProperty("value", nBinning);
                    break;
                case 12:
                    lCDSite12Bin->setEnabled(true);
                    lCDSite12Bin->setProperty("value", nBinning);
                    break;
                case 13:
                    lCDSite13Bin->setEnabled(true);
                    lCDSite13Bin->setProperty("value", nBinning);
                    break;
                case 14:
                    lCDSite14Bin->setEnabled(true);
                    lCDSite14Bin->setProperty("value", nBinning);
                    break;
                case 15:
                    lCDSite15Bin->setEnabled(true);
                    lCDSite15Bin->setProperty("value", nBinning);
                    break;
                case 16:
                    lCDSite16Bin->setEnabled(true);
                    lCDSite16Bin->setProperty("value", nBinning);
                    break;
                case 17:
                    lCDSite17Bin->setEnabled(true);
                    lCDSite17Bin->setProperty("value", nBinning);
                    break;
                case 18:
                    lCDSite18Bin->setEnabled(true);
                    lCDSite18Bin->setProperty("value", nBinning);
                    break;
                case 19:
                    lCDSite19Bin->setEnabled(true);
                    lCDSite19Bin->setProperty("value", nBinning);
                    break;
                case 20:
                    lCDSite20Bin->setEnabled(true);
                    lCDSite20Bin->setProperty("value", nBinning);
                    break;
                case 21:
                    lCDSite21Bin->setEnabled(true);
                    lCDSite21Bin->setProperty("value", nBinning);
                    break;
                case 22:
                    lCDSite22Bin->setEnabled(true);
                    lCDSite22Bin->setProperty("value", nBinning);
                    break;
                case 23:
                    lCDSite23Bin->setEnabled(true);
                    lCDSite23Bin->setProperty("value", nBinning);
                    break;
                case 24:
                    lCDSite24Bin->setEnabled(true);
                    lCDSite24Bin->setProperty("value", nBinning);
                    break;
                case 25:
                    lCDSite25Bin->setEnabled(true);
                    lCDSite25Bin->setProperty("value", nBinning);
                    break;
                case 26:
                    lCDSite26Bin->setEnabled(true);
                    lCDSite26Bin->setProperty("value", nBinning);
                    break;
                case 27:
                    lCDSite27Bin->setEnabled(true);
                    lCDSite27Bin->setProperty("value", nBinning);
                    break;
                case 28:
                    lCDSite28Bin->setEnabled(true);
                    lCDSite28Bin->setProperty("value", nBinning);
                    break;
                case 29:
                    lCDSite29Bin->setEnabled(true);
                    lCDSite29Bin->setProperty("value", nBinning);
                    break;
                case 30:
                    lCDSite30Bin->setEnabled(true);
                    lCDSite30Bin->setProperty("value", nBinning);
                    break;
                case 31:
                    lCDSite31Bin->setEnabled(true);
                    lCDSite31Bin->setProperty("value", nBinning);
                    break;
                default:
                    break;
            }
        }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Init station from STDF file
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::InitStation()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Init station.");

    GQTL_STDF::Stdf_PIR_V4		clPIR;
    GQTL_STDF::Stdf_SBR_V4		clSBR;
    GQTL_STDF::Stdf_HBR_V4		clHBR;
    int                         nStatus, nRecordType;
    GtsStation_BinningList      listSoftBinnings, listHardBinnings;

	// Open STDF file
    mStdfReader.Open(m_strStdfFullFileName.toLatin1().constData());

	// Read all records to get nb of sites, list of binnings...
	m_uiTotalPartsInSTDF = 0;
	nStatus = mStdfReader.LoadNextRecord(&nRecordType);
    while((nStatus == GQTL_STDF::StdfParse::NoError) || (nStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
	{
		switch(nRecordType)
		{
            case GQTL_STDF::Stdf_Record::Rec_PIR:
				// PIR record
                mStdfReader.ReadRecord((GQTL_STDF::Stdf_Record *)&clPIR);
                m_listSites.AddSite(clPIR.m_u1SITE_NUM);
				m_uiTotalPartsInSTDF++;
				break;

            case GQTL_STDF::Stdf_Record::Rec_PRR:
                // PRR record: keep track of part_ids for retest simulation ?
                // Not needed for now because test & retest is expected to be in different files.
                // If we have to support test & retest in the same STDF file, possible solution is to:
                // - during this quick read, save list of parts where the next part has part_id < part_id(current part)
                // - during simulation, at the end of each run processed, if part_id in the list, simulate a retest
                break;

            case GQTL_STDF::Stdf_Record::Rec_SBR:
				// SBR record
                mStdfReader.ReadRecord((GQTL_STDF::Stdf_Record *)&clSBR);
				if(clSBR.m_c1SBIN_PF == 'P')
					listSoftBinnings.AddBinning(clSBR.m_u2SBIN_NUM, GtsStation_Binning::eBinningPass);
				else if(clSBR.m_c1SBIN_PF == 'F')
					listSoftBinnings.AddBinning(clSBR.m_u2SBIN_NUM, GtsStation_Binning::eBinningFail);
				else
					listSoftBinnings.AddBinning(clSBR.m_u2SBIN_NUM);
				break;

            case GQTL_STDF::Stdf_Record::Rec_HBR:
				// HBR record
                mStdfReader.ReadRecord((GQTL_STDF::Stdf_Record *)&clHBR);
				if(clHBR.m_c1HBIN_PF == 'P')
					listHardBinnings.AddBinning(clHBR.m_u2HBIN_NUM, GtsStation_Binning::eBinningPass);
				else if(clHBR.m_c1HBIN_PF == 'F')
					listHardBinnings.AddBinning(clHBR.m_u2HBIN_NUM, GtsStation_Binning::eBinningFail);
				else
					listHardBinnings.AddBinning(clHBR.m_u2HBIN_NUM);
				break;

			default:
				break;
        }

		// Load next record
		nStatus = mStdfReader.LoadNextRecord(&nRecordType);
	}

	// Set binning list for each site
	m_listSites.SetBinningLists(listSoftBinnings, listHardBinnings);

	// Update part count and binning
	UpdateSummary();

	// Init widgets
	m_pageStats->Init(m_listSites);

	// Close and re-open STDF file (rewind doesn't work once too much data read)
	mStdfReader.Close();
    mStdfReader.Open(m_strStdfFullFileName.toLatin1().constData());

	// Open STDF file for write
	SetStdfOutputFileName();
	mStdfReader.GetCpuType(&m_nStdf_CPU_Type);
    if(m_pageSetup->IsStdfON())
	{
		m_bGenerateStdfFile = true;
        mStdfWriter.Open(m_strStdfFullFileName_Out.toLatin1().constData(), STDF_WRITE, m_nStdf_CPU_Type);
    }
	else
		m_bGenerateStdfFile = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Fill array with site numbers
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::FillSiteNumbersArray(int *pnSiteNumbers)
{
    for(int lIndex=0; lIndex<m_listSites.size(); ++lIndex)
        pnSiteNumbers[lIndex] = m_listSites.at(lIndex)->m_uiSiteNb;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Fill string with site numbers
//
// Argument(s) :
//
// Return type : none
/////////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::FillSiteNumbersString(QString & SiteNumbers)
{
    for(int lIndex=0; lIndex<m_listSites.size(); ++lIndex)
    {
        if(lIndex > 0)
            SiteNumbers += " ";
        SiteNumbers += QString("%1").arg(m_listSites.at(lIndex)->m_uiSiteNb);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Return total Part count (all sites)
//
// Argument(s) :
//
// Return type : part count
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int GtsStationMainwindow::PartCount()
{
	return m_listSites.PartCount();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Return total PASS count (all sites)
//
// Argument(s) :
//
// Return type : PASS count
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int GtsStationMainwindow::PassCount()
{
	return m_listSites.PassCount();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Return total FAIL count (all sites)
//
// Argument(s) :
//
// Return type : FAIL count
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int GtsStationMainwindow::FailCount()
{
	return m_listSites.FailCount();
}

///////////////////////////////////////////////////////////
// Validate selected (or dragged) STDF file.
///////////////////////////////////////////////////////////
bool GtsStationMainwindow::ValidateStdfFile(QString strFileName)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Checking validity of file %1").arg(strFileName).toLatin1().data() );

    // Check if new file selected
    if(strFileName.isEmpty() == true)
        return false;

	// Make sure the file has a valid STDF extension
	QFileInfo	fileInfo(strFileName);
    QString		strExtension = fileInfo.suffix();

	if((strExtension != "std") && (strExtension != "stdf"))
	{
        Warning(QString("Invalid file extension %1.\nPlease select a file with a valid STDF extension (*.std *.stdf)")
                .arg(strExtension));
        return false;
	}

    // Make sure the file is a valid STDF file
    GQTL_STDF::StdfParse lStdfReader;
    if(lStdfReader.Open(strFileName.toLatin1().constData()) == false)
	{
        Warning(QString("Invalid STDF file (couldn't open file):\n\n%1\n\nPlease select a valid STDF file")
                .arg(GGET_LASTERRORMSG(StdfParse, &lStdfReader)));
        return false;
	}

	// Read MIR record
    GQTL_STDF::Stdf_MIR_V4 lMIR;
    if(lStdfReader.LoadNextRecord(GQTL_STDF::Stdf_Record::Rec_MIR, true) != GQTL_STDF::StdfParse::NoError)
	{
        Warning(QString("Invalid STDF file (no MIR record):\n\n%1\n\nPlease select a valid STDF file")
                .arg(GGET_LASTERRORMSG(StdfParse, &lStdfReader)));
        return false;
	}
    lStdfReader.ReadRecord((GQTL_STDF::Stdf_Record *)&lMIR);
	
	// Make sure there is at leat 1 PIR record
    if(lStdfReader.LoadNextRecord(GQTL_STDF::Stdf_Record::Rec_PIR) != GQTL_STDF::StdfParse::NoError)
	{
        Warning(QString("Invalid STDF file (no PIR record):\n\n%1\n\nPlease select a valid STDF file")
                .arg(GGET_LASTERRORMSG(StdfParse, &lStdfReader)));
        return false;
	}

	// Close STDF file (rewind doesn't work once too much data read)
    lStdfReader.Close();

    GSLOG(SYSLOG_SEV_NOTICE, QString("File %1 is a valid STDF file and has a MIR and a PRR").
          arg(strFileName).toLatin1().data() );

    return true;
}

///////////////////////////////////////////////////////////
// A new STDF file has been selected: load it.
///////////////////////////////////////////////////////////
bool GtsStationMainwindow::LoadStdfFile(const QString & StdfFile)
{
    // Update member variables
    QFileInfo clFileInfo(StdfFile);
    m_strStdfFullFileName = StdfFile;
    m_strStdfFileName = clFileInfo.fileName();
    m_strStdfFilePath = clFileInfo.absolutePath();
    m_bProgramLoaded = true;	    

	// Initialize station (must be called only once STDF filename has been set
    ResetStation();
    InitStation();
	
	// Update information in station window
	buttonLoadDelete->setText(GTSTSATION_BUTTON_DELETE_TEXT);
    if(isAutoRun() || isRunningModeBench())
    {
        buttonRun->setEnabled(false);
        buttonRunN->setEnabled(false);
        spinRunN->setEnabled(false);
        buttonRunAll->setEnabled(false);
        buttonGtlCommads->setEnabled(false);
        buttonNewlot->setEnabled(false);
    }
    else
    {
        buttonRun->setEnabled(true);
        buttonRunN->setEnabled(true);
        spinRunN->setEnabled(true);
        buttonRunAll->setEnabled(true);
        buttonGtlCommads->setEnabled(true);
        buttonNewlot->setEnabled(true);
    }

    m_pageInfo->Update(StdfFile, mKeysContent);

	// Set Caption and icon: station + station nb
    setWindowTitle(QString("%1 - Station %2: %3").arg(m_strApplicationName).arg(StationNb())
                   .arg(mKeysContent.Get("ProgramName").toString()));
    setWindowIcon(QIcon(QPixmap(":/images/StationOnIcon")));

    return true;
}

void GtsStationMainwindow::Station(QString & strStation)
{
    strStation = QString("%1").arg(StationNb());
    if(!m_bProgramLoaded)
        return;

    bool lOk=false;
    int lStation = mKeysContent.Get("Station").toInt(&lOk);
    if(lOk)
        strStation = QString("%1").arg(lStation);
}

///////////////////////////////////////////////////////////
// Init GTL.
///////////////////////////////////////////////////////////
bool GtsStationMainwindow::GTL_Open(bool RemoveTempGtlSqlite)
{
    GSLOG(SYSLOG_SEV_DEBUG, "GTL_Open.");

    // Bernard and co: we dont have the choice:
    // - we have to avoid potential conflicts between several GTL/GTS/GTM QA scenarios
    // - we have to handle the fact that under winXP/cygwin32, there seems to be no retrievable OS temp folder

    QString lRecipe = editRecipe->text();
    QString lTesterConf = editTesterConf->text();
    QString lAN; ApplicationName(lAN);
    QString lTestJobName; TestJobName(lTestJobName);
    QString lTestJobFile; TestJobFile(lTestJobFile);
    QString lTestJobPath; TestJobPath(lTestJobPath);
    QString lTestSourceFilesPath; TestSourceFilesPath(lTestSourceFilesPath);
    QString lTestDataFile; TestDataFile(lTestDataFile);

    // First GTL call should be to set output folder and temp folder
    int r=GTL_CORE_ERR_OKAY;

    // In QA mode, do some custom stuff...
    if(isRunningModeQA())
    {
        char    lGTLTF[1024]="./temp";
        QString lTempFolder;

        // In QA mode, force temp folder so it is different for each GTS
#if 0   // Looks get temp folder sometimes crashes in release
        // Get Temp folder as detected by GTL
        GSLOG(SYSLOG_SEV_DEBUG, "Getting GTL temp folder.");
        r=gtl_get(GTL_KEY_TEMP_FOLDER, lGTLTF);
        if (r!=GTL_CORE_ERR_OKAY)
        {
            Error(QString("gtl_get(\"temp_folder\") failed (return code %1), program will be unloaded.").arg(r));
            return false;
        }
#endif
        // Append site info
        GSLOG(SYSLOG_SEV_DEBUG, "Appending site nb. to GTL temp folder.");
        if(m_listSites.size() == 1)
        {
            GtsStation_Site* lSite=NULL;
            lSite = m_listSites.at(0);
            if(lSite)
                lTempFolder = QString("%1/site_%2").arg(lGTLTF).arg(lSite->m_uiSiteNb);
            else
                lTempFolder = QString("%1/site_?").arg(lGTLTF);
        }
        else
            lTempFolder = QString("%1/all_sites").arg(lGTLTF);
        // Create new temp folder in case it doesn't exist
        GSLOG(SYSLOG_SEV_DEBUG, QString("Creating folder %1.").arg(lTempFolder).toLatin1().constData());
        QDir lDir;
        if(!lDir.mkpath(lTempFolder))
        {
            Error(QString("Unable to create temp folder %1, program will be unloaded.").arg(lTempFolder));
            return false;
        }
        // Set new GTL temp folder
        GSLOG(SYSLOG_SEV_DEBUG, QString("Setting GTL temp folder to %1.").arg(lTempFolder).toLatin1().constData());
        r=gtl_set((char*)GTL_KEY_TEMP_FOLDER, lTempFolder.toLatin1().data());
        PopGtlMessageStack();
        if (r!=GTL_CORE_ERR_OKAY)
        {
            Error(QString("gtl_set(\"temp_folder\") failed (return code %1), program will be unloaded.").arg(r));
            return false;
        }

        // very useful in QA mode: eventually delete existing gtl.sqlite
        if (RemoveTempGtlSqlite)
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Trying to delete any previous temp GTL sqlite file...");
            // Remove GTL temp sqlite file
            r=gtl_get(GTL_KEY_TEMP_FOLDER, lGTLTF);
            if (r==0)
            {
                QDir lDir;
                QString lTempSqlite = QString(lGTLTF) + QString("/gtl.sqlite");
                GSLOG(SYSLOG_SEV_NOTICE, QString("Attempt to remove temp SQLite file %1...").arg(lTempSqlite)
                      .toLatin1().constData());
                if(!lDir.remove(lTempSqlite))
                    Warning(QString("Could not remove temporary Sqlite file %1").arg(lTempSqlite));
            }
            else
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Cannot retrieve GTL temp folder: %1").arg(r).toLatin1().data() );
                Warning(QString("Cannot retrieve GTL temp folder"));
            }
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Setting output folder  to %1.").arg(lTestSourceFilesPath).toLatin1().constData());
    r=gtl_set((char*)GTL_KEY_OUTPUT_FOLDER, lTestSourceFilesPath.toLatin1().data());
    PopGtlMessageStack();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_set(\"output_folder\") failed (return code %1), program will be unloaded.").arg(r));
        return false;
    }

    r=gtl_set((char*)GTL_KEY_DATA_FILENAME, lTestDataFile.toLatin1().data());
    PopGtlMessageStack();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_set(\"datafile_name\") failed (return code %1), program will be unloaded.").arg(r));
        return false;
    }

    r=gtl_set((char*)GTL_KEY_OUTPUT_FILENAME, QString("%1.sqlite").arg(lTestDataFile).toLatin1().data());
    PopGtlMessageStack();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_set(\"outputfile_name\") failed (return code %1), program will be unloaded.").arg(r));
        return false;
    }

#if 0
    // Not supported since GTL V4.0 (Traceability file is mandatory)
    r=gtl_set((char*)GTL_KEY_TRACEABILITY_MODE, (char*)"on");
    PopGtlMessageStack();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_set(\"traceability_mode\") failed (return code %1), program will be unloaded.").arg(r));
        return false;
    }
#endif

    r=gtl_set((char*)"socket_trace", (char *)"on");
    PopGtlMessageStack();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_set(\"socket_trace\") failed (return code %1), program will be unloaded.").arg(r));
        return false;
    }

    if(mProperties.value(GTS_PROPERTY_GTM_COMM_MODE).toString() == "synchronous")
    {
        r=gtl_set((char*)"gtm_communication_mode", (char *)"synchronous");
        PopGtlMessageStack();
        if (r!=GTL_CORE_ERR_OKAY)
        {
            Error(QString("gtl_set(\"gtm_communication_mode\") failed (return code %1), program will be unloaded.").arg(r));
            return false;
        }
    }

    // 6995
    QString lON; OperatorName(lON);
    QString lJR; JobRevision(lJR);
    QString lLotID; LotID(lLotID);
    QString lSublotID; SublotID(lSublotID);
    QString lProductID; ProductID(lProductID);
#if 1
    // gcore-1180
    /*
    r=gtl_set_prod_info(lON.toLatin1().data(), lJR.toLatin1().data(), lLotID.toLatin1().data(),
      lSublotID.toLatin1().data(), lProductID.toLatin1().data() );
    */
    r=gtl_set((char*)GTL_KEY_OPERATOR_NAME, lON.toLatin1().data() );
    if (r!=GTL_CORE_ERR_OKAY)
        goto gtl_set_prodinfo_error;
    r=gtl_set((char*)GTL_KEY_JOB_REVISION, lJR.toLatin1().data() );
    if (r!=GTL_CORE_ERR_OKAY)
        goto gtl_set_prodinfo_error;
    r=gtl_set((char*)GTL_KEY_LOT_ID, lLotID.toLatin1().data() );
    if (r!=GTL_CORE_ERR_OKAY)
        goto gtl_set_prodinfo_error;
    r=gtl_set((char*)GTL_KEY_SUBLOT_ID, lSublotID.toLatin1().data() );
    if (r!=GTL_CORE_ERR_OKAY)
        goto gtl_set_prodinfo_error;
    r=gtl_set((char*)GTL_KEY_PRODUCT_ID, lProductID.toLatin1().data() );
    if (r!=GTL_CORE_ERR_OKAY)
        goto gtl_set_prodinfo_error;

    PopGtlMessageStack();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        gtl_set_prodinfo_error:
        Error(QString("gtl_set prod info failed (return code %1), program will be unloaded.").arg(r));
        return false;
    }

    // 6995
    CGSystemInfo clSysInfo;
    // Dont we have to do a ReadSystemInfo() ?
    //char* lHostID=(char*)clSysInfo.m_strHostID.c_str();
    char* lHostName=(char*)clSysInfo.m_strHostName.c_str();
    char* lUN=(char*)clSysInfo.m_strAccountName.c_str();
    // GCORE-1180
    /*
    r=gtl_set_node_info(
        StationNb(), lHostID,lHostName,lUN, (char*)"GTS",
        lAN.toLatin1().data(),
        lTestJobName.toLatin1().data(),
        lTestJobFile.toLatin1().data(),
        lTestJobPath.toLatin1().data(),
        lTestSourceFilesPath.toLatin1().data() );
        */
    r=gtl_set((char*)GTL_KEY_STATION_NUMBER, QString::number(StationNb()).toLatin1().data() );
    if (r!=GTL_CORE_ERR_OKAY)
        goto gtl_set_nodeinfo_error;
    r=gtl_set((char*)GTL_KEY_TESTER_TYPE, (char*)"GTS");
    if (r!=GTL_CORE_ERR_OKAY)
        goto gtl_set_nodeinfo_error;
    r=gtl_set((char*)GTL_KEY_TESTER_NAME, lHostName); // was NodeName
    if (r!=GTL_CORE_ERR_OKAY)
        goto gtl_set_nodeinfo_error;
    r=gtl_set((char*)GTL_KEY_USER_NAME, lUN); // diff than operator name ?
    if (r!=GTL_CORE_ERR_OKAY)
        goto gtl_set_nodeinfo_error;
    r=gtl_set((char*)GTL_KEY_JOB_NAME, (char*)lTestJobName.toLatin1().data());
    if (r!=GTL_CORE_ERR_OKAY)
        goto gtl_set_nodeinfo_error;

    PopGtlMessageStack();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        gtl_set_nodeinfo_error:
        Error(QString("gtl_set_node_info failed (return code %1), program will be unloaded.").arg(r));
        return false;
    }
#endif

    // Test all other GTL Keys
    QString lTN; TesterName(lTN);
    QString lTT; TesterType(lTT);
    QString lFacility; Facility(lFacility);
    QString lFamily; Family(lFamily);
    QString lSpecName; SpecName(lSpecName);
    QString lUserText; UserText(lUserText);
    QString lTemperature; Temperature(lTemperature);
    QString lTestingCode; TestingCode(lTestingCode);
    QString lStation; Station(lStation);
    QString lRetestIndex; RetestIndex(lRetestIndex);
    QChar   lModeCode; ModeCode(lModeCode);
    gtl_set((char*)GTL_KEY_STATION_NUMBER, lStation.toLatin1().data());
    gtl_set((char*)GTL_KEY_TESTER_NAME, lTN.toLatin1().data());
    gtl_set((char*)GTL_KEY_TESTER_TYPE, lTT.toLatin1().data());
    gtl_set((char*)GTL_KEY_TESTER_EXEC_TYPE, (char*)"GTS: GTL_KEY_TESTER_EXEC_TYPE to be retrieved from STDF");
    gtl_set((char*)GTL_KEY_JOB_NAME, lTestJobName.toLatin1().data());
    gtl_set((char*)GTL_KEY_OPERATOR_NAME, lON.toLatin1().data());
    gtl_set((char*)GTL_KEY_JOB_REVISION, lJR.toLatin1().data());
    gtl_set((char*)GTL_KEY_LOT_ID, lLotID.toLatin1().data());
    gtl_set((char*)GTL_KEY_SUBLOT_ID, lSublotID.toLatin1().data());
    gtl_set((char*)GTL_KEY_PRODUCT_ID, lProductID.toLatin1().data());
    gtl_set((char*)GTL_KEY_AUX_FILE, (char*)"GTS: GTL_KEY_AUX_FILE to be retrieved from STDF");
    gtl_set((char*)GTL_KEY_FACILITY_ID, lFacility.toLatin1().data());
    gtl_set((char*)GTL_KEY_FAMILY_ID, lFamily.toLatin1().data());
    gtl_set((char*)GTL_KEY_FLOW_ID, (char*)"GTS: GTL_KEY_FLOW_ID to be retrieved from STDF");
    gtl_set((char*)GTL_KEY_FLOOR_ID, (char*)"GTS: GTL_KEY_FLOOR_ID to be retrieved from STDF");
    gtl_set((char*)GTL_KEY_ROM_CODE, (char*)"GTS: GTL_KEY_ROM_CODE to be retrieved from STDF");
    gtl_set((char*)GTL_KEY_SPEC_NAME, lSpecName.toLatin1().data());
    gtl_set((char*)GTL_KEY_SPEC_VERSION, (char*)"GTS: GTL_KEY_SPEC_VERSION to be retrieved from STDF");
    gtl_set((char*)GTL_KEY_SETUP_ID, (char*)"GTS: GTL_KEY_SETUP_ID to be retrieved from STDF");
    gtl_set((char*)GTL_KEY_TEST_CODE, lTestingCode.toLatin1().data());
    gtl_set((char*)GTL_KEY_TEST_TEMPERATURE, lTemperature.toLatin1().data());
    gtl_set((char*)GTL_KEY_USER_TXT, lUserText.toLatin1().data());
    gtl_set((char*)GTL_KEY_MODE_CODE, QString(lModeCode).toLatin1().data());
    gtl_set((char*)GTL_KEY_RETEST_CODE, lRetestIndex.toLatin1().data());
    gtl_set((char*)GTL_KEY_TESTER_EXEC_VERSION, (char*)"GTS: GTL_KEY_TESTER_EXEC_VERSION to be retrieved from STDF");

    // Replace gtl_init(...) with gtl_set(...) + gtl_open() :
#if 0
    int lSiteNumbers[256];
    FillSiteNumbersArray(lSiteNumbers);
    r=gtl_init(lTesterConf.toLatin1().data(), lRecipe.toLatin1().data(), NbSites(), lSiteNumbers, 1024*1024);
    GSLOG(SYSLOG_SEV_DEBUG, QString("gtl_init() completed with status %1").arg(r).toLatin1().constData());
#else
    QString lSiteNumbers;
    FillSiteNumbersString(lSiteNumbers);
    gtl_set(GTL_KEY_CONFIG_FILE, lTesterConf.toLatin1().data());
    gtl_set(GTL_KEY_RECIPE_FILE, lRecipe.toLatin1().data());
    gtl_set(GTL_KEY_MAX_NUMBER_OF_ACTIVE_SITES, QString("%1").arg(NbSites()).toLatin1().data());
    gtl_set(GTL_KEY_SITES_NUMBERS, lSiteNumbers.toLatin1().data());
    gtl_set(GTL_KEY_MAX_MESSAGES_STACK_SIZE, (char*)"1048576");
    GSLOG(SYSLOG_SEV_DEBUG, "Calling gtl_open()");
    r=gtl_command((char*)GTL_COMMAND_OPEN);
    GSLOG(SYSLOG_SEV_DEBUG, QString("gtl_command(%1) completed with status %2").arg(GTL_COMMAND_OPEN)
          .arg(r).toLatin1().constData());
#endif
    PopGtlMessageStack();

    UpdateSummary();

    // Check status
    if(r != GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_open failed (return code %1), program will be unloaded.").arg(r));
        return false;
    }

    // Check if some new PAT limits are available.
    long lTestNumbers[1000];
    unsigned int lFlags[1000];
    unsigned int lHardBins[1000];
    unsigned int lSoftBins[1000];
    double lLowLimits[1000];
    double lHighLimits[1000];
    unsigned int lArrayFilledSize;
    QString lMessage;
    for(int lIndex=0; lIndex<m_listSites.size(); ++lIndex)
    {
        r = gtl_get_spat_limits(m_listSites.at(lIndex)->m_uiSiteNb, lTestNumbers, lFlags, lLowLimits, lHighLimits, lHardBins, lSoftBins, 1000, &lArrayFilledSize);
        if(r != GTL_CORE_ERR_OKAY)
        {
            Error(QString("gtl_get_spat_limits failed (return code %1).").arg(r));
            return false;
        }
        if(lArrayFilledSize > 0)
        {
            lMessage = QString("Received SPAT limits for %1 tests on site %2.").arg(lArrayFilledSize).arg(m_listSites.at(lIndex)->m_uiSiteNb);
            GSLOG(SYSLOG_SEV_NOTICE, lMessage.toLatin1().constData());
            m_pageOutput->Printf(QString("INFO: %1\n").arg(lMessage));
            for(unsigned int lIndex=0; lIndex<lArrayFilledSize; ++lIndex)
            {
                lMessage = QString("**** Test %1: Flags=%2, LL=%3, HL=%4, Bin=%5.").arg(lTestNumbers[lIndex])
                        .arg(lFlags[lIndex]).arg(lLowLimits[lIndex]).arg(lHighLimits[lIndex]).arg(lHardBins[lIndex]);
                GSLOG(SYSLOG_SEV_NOTICE, lMessage.toLatin1().constData());
                m_pageOutput->Printf(QString("%1\n").arg(lMessage));
            }
        }
    }

    return true;
}

bool GtsStationMainwindow::GTL_Close()
{
    GSLOG(5, "GTL close...");
    if(GetGTLLibState() != GTL_STATE_NOT_INITIALIZED)
    {
        int r=GTL_CORE_ERR_OKAY;
        if(isRunningModeQA())
        {
            // Rename output file name before close: traceability_sl<splitlot_id>_ri<retest_index>.sqlite
            char lSplitlotID[1024]="?";
            gtl_get(GTL_KEY_SPLITLOT_ID, lSplitlotID);
            r=gtl_set((char*)GTL_KEY_OUTPUT_FILENAME, QString("traceability_sl%1_ri%2.sqlite")
                      .arg(QString(lSplitlotID)).arg(QString::number(mKeysContent.Get("RetestIndex").toInt())).toLatin1().data());
            PopGtlMessageStack();
            if (r!=GTL_CORE_ERR_OKAY)
            {
                Error(QString("gtl_set(\"outputfile_name\") failed (return code %1), program will be unloaded.")
                      .arg(r));
                return false;
            }

        }

        r=gtl_command((char*)GTL_COMMAND_CLOSE);
        m_ui_gtl_close_lib++;
        PopGtlMessageStack();
        if (r!=GTL_CORE_ERR_OKAY)
        {
            Error(QString("gtl_close failed (return code %1).").arg(r));
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Set the STDF output filename, depending on input filename + timestamp.
///////////////////////////////////////////////////////////
void GtsStationMainwindow::SetStdfOutputFileName()
{
	char szBuffer[100];

	// Get timestamp string
	ut_GetCompactTimeStamp(szBuffer);

	// Construct output filename
    m_strStdfFullFileName_Out = m_strStdfFullFileName + QString("_out_") + QString(szBuffer) + QString(".stdf");
}

/////////////////////////////////////////////////////////////////////////////////////
// Adds a page
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::AddPage(const QString& strSelectionText, const QPixmap& pixmap, QWidget* pPage, bool bDisableWhenRunning)
{
	GtsStation_Page		Page;

	// Add to list of pages
	Page.pPage = pPage;
	Page.bDisableWhenRunning = bDisableWhenRunning;
	m_listPages.append(Page);
	
	// Insert item into selection listbox
    new QListWidgetItem(QIcon(pixmap), strSelectionText, listMainSelection);

    // Add to stacked widget
    widgetStack->addWidget(pPage);
}

/////////////////////////////////////////////////////////////////////////////////////
// Main selection changed, switch to correct widget in the stack
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnMainSelectionChanged()
{
	// Get selected item and activate corresponding page
    widgetStack->setCurrentWidget(m_listPages[listMainSelection->currentRow()].pPage);
}

void GtsStationMainwindow::OnJobNameChanged(const QString & text)
{
    mKeysContent.Set("ProgramName",text);
}

/////////////////////////////////////////////////////////////////////////////////////
// Display Help page
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonHelp()
{
	// Display 'Help' page
    listMainSelection->setCurrentItem(listMainSelection->item(m_uiPage_Help));
}

/////////////////////////////////////////////////////////////////////////////////////
// Set text in status bar
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::SetStatus(int nStatus)
{
    if(isRunningModeBench())
        return;

    QString strString = "Unknown";

	// Set status text
	switch(nStatus)
	{
		case StatusReady:
			strString = tr("Ready");
			break;
	}

	// Set text in the status bar widget

	// Force application to process events
	m_qApplication->processEvents();
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GtsStationMainwindow::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GtsStationMainwindow::dropEvent(QDropEvent *e)
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

    // Check if a test program is not already loaded
    if(m_bProgramLoaded == true)
    {
        if(m_qMessageBox.warning(this, m_strApplicationName, "A test program is already loded.\nDo you want to discard it?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
        {
            // Ignore event
            e->ignore();
            return;
        }
    }

    // Validate selected STDF file: if OK, it will be loaded
    LoadCommand(strFileList);

    e->acceptProposedAction();
}

/////////////////////////////////////////////////////////////////////////////////////
// Close event received, any cleanup code should be performed before accepting the event
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::closeEvent(QCloseEvent * e)
{
	// Cleanup code
    GSLOG(7, "closeEvent received");

	// Call GTL close function
    GTL_Close();

	// Accept close event
	e->accept();
}

/////////////////////////////////////////////////////////////////////////////////////
// The application is being closed: send the close signal
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonExit()
{
    GSLOG(5, "On button Exit...");
    // in order to cleanly close the connection if any. If not, GTS window disapear but GTS still running at the back
    GTL_Close();
	// Close Window
	close();
}

/////////////////////////////////////////////////////////////////////////////////////
// 'Datalog' button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonDatalog()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// 'Reset' button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonReset()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// 'Newlot' button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonNewlot()
{
    // No more gtl_endlot() supported in GTL V3.5, so do not support NewLot for now
    // (button hidden in GUI)
    // If we need to support 'NewLot' function, we have to:
    // 1) gtl_close() and gtl_open()
    // 2) Select new files for the new lot (clear and reload mStdfFiles)
    return;

    GtsStation_Newlotdialog	*pDialog = new GtsStation_Newlotdialog(mKeysContent);

    if(pDialog && pDialog->exec() == QDialog::Accepted)
	{
		// Send endlot command to output window 
        m_pageOutput->Command(QString("endlot for lot=%1, sublot=%2\n").arg(mKeysContent.Get("Lot").toString())
                              .arg(mKeysContent.Get("SubLot").toString()));

		// Get new IDs
        mKeysContent.Set("Lot", pDialog->GetLotID());
        mKeysContent.Set("SubLot", pDialog->GetSublotID());
		
		// Reset station
        ResetStation();

        // Call GTL endlot function if need be
        // No more gtl_endlot() in GTL V3.5
        //EndLot();
	}

    if (pDialog)
        delete pDialog;
}

/////////////////////////////////////////////////////////////////////////////////////
// 'GTL commands' button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonGtlCommands()
{
    buttonGtlCommads->setEnabled(false);

	m_pGtlCommandsDialog = new GtsStation_GtlCommandsdialog;

    connect(m_pGtlCommandsDialog, SIGNAL(sButtonSend(unsigned int, long)), this, SLOT(OnGtlCommandsDialog_Send(unsigned int, long)));
	connect(m_pGtlCommandsDialog, SIGNAL(sButtonClose()), this, SLOT(OnGtlCommandsDialog_Close()));

	m_pGtlCommandsDialog->show();
}

/////////////////////////////////////////////////////////////////////////////////////
// 'GTL commands' dialog box: Send button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnGtlCommandsDialog_Send(unsigned int uiCommand, long lTestNumber)
{
    QString lCommand;

	switch(uiCommand)	
	{
		case GTS_STATION_GTLCOMMANDS_DEBUGON:
            lCommand=QString("-gtl_debugon");
			break;
		case GTS_STATION_GTLCOMMANDS_DEBUGOFF:
            lCommand=QString("-gtl_debugoff");
			break;
        case GTS_STATION_GTLCOMMANDS_QUIETON:
            lCommand=QString("-gtl_quieton");
			break;
		case GTS_STATION_GTLCOMMANDS_QUIETOFF:
            lCommand=QString("-gtl_quietoff");
			break;
		case GTS_STATION_GTLCOMMANDS_INFO:
            lCommand=QString("-gtl_info");
			break;
		case GTS_STATION_GTLCOMMANDS_STATUS:
            lCommand=QString("-gtl_status");
			break;
		case GTS_STATION_GTLCOMMANDS_TEST:
            lCommand=QString("-gtl_test %1").arg(lTestNumber);
			break;
		case GTS_STATION_GTLCOMMANDS_TESTLIST:
            lCommand=QString("-gtl_testlist");
			break;
		case GTS_STATION_GTLCOMMANDS_HELP:
            lCommand=QString("-gtl_help");
			break;
        case GTS_STATION_GTLCOMMANDS_NONE:
		default:
			break;
	}

    // Execute command
    if(!lCommand.isEmpty())
    {
        // Display command to output window
        m_pageOutput->Command(QString("send command: %1").arg(lCommand));

        // Execute
        int r=gtl_command(lCommand.toLatin1().data());
        PopGtlMessageStack();
        if (r!=GTL_CORE_ERR_OKAY)
        {
            Warning(QString("gtl_command failed (return code %1).").arg(r));
            return;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// Pop GTL message stack, and display in output window
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::PopGtlMessageStack(bool lDisplay)
{
    // Display messages if any
    // gcore-1180
    //int n=gtl_get_number_messages_in_stack();
    char lNumMsgString[256]="?";
    if (gtl_get(GTL_KEY_NUM_OF_MESSAGES_IN_STACK, lNumMsgString)!=0)
        return;
    int n=QString(lNumMsgString).toInt();

    for (int i=0; i<n; i++)
    {
        // gcore-1180
        //int r=gtl_pop_first_message(&severity, message, &messageID);
        int severity=0, messageID=0;
        char message_sev_string[1024]="?";
        char message_id_string[1024]="?";
        char message[GTL_MESSAGE_STRING_SIZE]="?"; // currently 1 message is 1024 chars

        int r=gtl_command((char*)GTL_COMAND_POP_FIRST_MESSAGE);

        if(lDisplay)
        {
            if (r==0)
            {
                gtl_get(GTL_KEY_CURRENT_MESSAGE, message);
                gtl_get(GTL_KEY_CURRENT_MESSAGE_SEV, message_sev_string);
                sscanf(message_sev_string, "%d", &severity);
                gtl_get(GTL_KEY_CURRENT_MESSAGE_ID, message_id_string);
                sscanf(message_id_string, "%d", &messageID);
                m_pageOutput->Command(QString(message));
            }
            else
                m_pageOutput->Command(QString("Failed to pop message: return code %1").arg(r).toLatin1().constData());
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// 'GTL commands' dialog box: Close button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnGtlCommandsDialog_Close()
{
	delete m_pGtlCommandsDialog;
    buttonGtlCommads->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////
// 'RunN' button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonRunN()
{
    unsigned int uiRunN = (unsigned int)spinRunN->value();

    buttonLoadDelete->setEnabled(false);
    buttonRun->setEnabled(false);
    buttonRunN->setEnabled(false);
    spinRunN->setEnabled(false);
    buttonRunAll->setEnabled(false);
    buttonGtlCommads->setEnabled(false);
    buttonNewlot->setEnabled(false);

	while(uiRunN > 0)
	{
        if(!RunCommand())
			uiRunN = 0;
		else
			uiRunN--;
	}

    buttonLoadDelete->setEnabled(true);
    buttonRun->setEnabled(true);
    buttonRunN->setEnabled(true);
    spinRunN->setEnabled(true);
    buttonRunAll->setEnabled(true);
    buttonGtlCommads->setEnabled(true);
    buttonNewlot->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////
// 'Run' button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonRun()
{
	// Call Run cammand 
    RunCommand();
}

/////////////////////////////////////////////////////////////////////////////////////
// 'RunAll' button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonRunAll()
{
    buttonLoadDelete->setEnabled(false);
    buttonRun->setEnabled(false);
    buttonRunN->setEnabled(false);
    spinRunN->setEnabled(false);
    buttonRunAll->setEnabled(false);
    buttonGtlCommads->setEnabled(false);
    buttonNewlot->setEnabled(false);

    RunAll();

    buttonLoadDelete->setEnabled(true);
    buttonRun->setEnabled(true);
    buttonRunN->setEnabled(true);
    spinRunN->setEnabled(true);
    buttonRunAll->setEnabled(true);
    buttonGtlCommads->setEnabled(true);
    buttonNewlot->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////
// Run all parts in STDF
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::RunAll()
{
    bool	bStatus=true;

    while(bStatus && (PartCount() < m_uiTotalPartsInSTDF))
        bStatus = RunCommand();

    return bStatus;
}

/////////////////////////////////////////////////////////////////////////////////////
// Run command
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::RunCommand()
{
	// Send command to output window
    m_pageOutput->Command(QString("run\n"));

	// Run test program
    return RunTestProgram();
}

bool GtsStationMainwindow::LoadCommand(const QStringList &FileNames)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Loading %1 STDF files...").arg(FileNames.count()).toLatin1().constData());

    // Clear station
    ClearStation();

    // Loop through all files: validate, get keys, and add to list
    for(int lIndex=0; lIndex<FileNames.size(); ++lIndex)
    {
        QString lStdfFile = FileNames.at(lIndex);

        // Validate selected STDF file: if OK, it will be loaded
        if(ValidateStdfFile(lStdfFile))
        {
            // Load keys
            GSLOG(SYSLOG_SEV_DEBUG, "Create keyscontent object");

            GS::QtLib::DatakeysContent lKeysContent;
            lKeysContent.SetInternal("FileName",lStdfFile);
            lKeysContent.SetInternal("FileSize",QFileInfo(lStdfFile).size());
            lKeysContent.SetInternal("StdfFileName",lStdfFile);
            lKeysContent.SetInternal("StdfFileSize",QFileInfo(lStdfFile).size());
            lKeysContent.SetInternal("SourceArchive",lStdfFile);

            // Stdf quick read and config keys processing
            GSLOG(SYSLOG_SEV_DEBUG, "Stdf quick read");
            QString lError;
            if(!GS::QtLib::DatakeysLoader::Load(lStdfFile, lKeysContent, lError))
            {
                Error(QString("Failed loading keys from file %1 (error: %2).").arg(lStdfFile).arg(lError));
                ClearStation();
                return false;
            }

            GSLOG(SYSLOG_SEV_DEBUG, "Find config keys file");
            GS::QtLib::DatakeysEngine   lKeysEngine(lKeysContent);
            QString                     lConfigFileName;
            int                         lLineNb=0;
            bool                        lFailedValidation=false;
            if(lKeysEngine.findConfigDbKeysFile(lConfigFileName, lKeysContent, ""))
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("Found config keys file %1, load it.")
                      .arg(lConfigFileName).toLatin1().constData());
                if(!lKeysEngine.loadConfigDbKeysFile(lConfigFileName, lLineNb, lError))
                {
                    Error(QString("Failed loading config keys file %1 (line: %2, error: %3).")
                          .arg(lConfigFileName).arg(lLineNb).arg(lError));
                    ClearStation();
                    return false;
                }

                GSLOG(SYSLOG_SEV_DEBUG, "Keys loaded, evaluate static keys.");
                if(!lKeysEngine.evaluateStaticDbKeys(lFailedValidation, lLineNb, lError))
                {
                    if(lFailedValidation)
                    {
                        Warning(QString("Validation rules specified in config keys file %1 failed (line: %2, error: %3). This file will be ignored.")
                              .arg(lConfigFileName).arg(lLineNb).arg(lError));
                    }
                    else
                    {
                        Error(QString("Failed evaluating rules specified in config keys file %1 failed (line: %2, error: %3).")
                              .arg(lConfigFileName).arg(lLineNb).arg(lError));
                        ClearStation();
                        return false;
                    }
                }
            }

            // Add to list of STDF files
            if(!lFailedValidation)
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("Add STDF file %1 to list of files to process.")
                      .arg(lStdfFile).toLatin1().constData());
                lKeysContent = lKeysEngine.dbKeysContent();
                mStdfFiles.insert(QPair<int, time_t>(lKeysContent.Get("RetestIndex").toInt(), lKeysContent.Get("StartTime").toLongLong()),
                                  QPair<QString, GS::QtLib::DatakeysContent>(lStdfFile, lKeysContent));
            }
        }
        else
            Warning(QString("Invalid file '%1'. This file will be ignored.").arg(lStdfFile));
    }

    // Check if some files to process
    if(mStdfFiles.size()==0)
    {
        Error(QString("No valid Stdf file found."));
        ClearStation();
        return false;
    }

    // Check if retest indexes are consistent: no holes
    GSLOG(SYSLOG_SEV_DEBUG, "Check if retest indexes are consistent: no holes.");
    QList< QPair< int, time_t> >  lKeys = mStdfFiles.keys();
    int lRI, lPrevRI;
    lRI = lPrevRI = lKeys.at(0).first;
    if((lRI > 0) && !GTL_Retest())
    {
        Error(QString("GTL_Retest failed."));
        ClearStation();
        return false;
    }
    for(int lIndex=1; lIndex<lKeys.size(); ++lIndex)
    {
        lRI = lKeys.at(lIndex).first;
        if((lRI != lPrevRI) && (lRI != (lPrevRI+1)))
        {
            Error(QString("Retest indexes are not contigous (%1 -> %2).").arg(lPrevRI).arg(lRI));
            ClearStation();
            return false;
        }
        lPrevRI = lRI;
    }

    // Update GUI
    m_pageInfo->Update(mStdfFiles);

    // Load first file
    QPair<QString, GS::QtLib::DatakeysContent> lFirstStdf = mStdfFiles.take(lKeys.at(0));
    mKeysContent = lFirstStdf.second;

    // Send command to output window
    m_pageOutput->Command(QString("load %1\n").arg(lFirstStdf.first));

    // The selected file is a valid STDF file, load it
    GSLOG(SYSLOG_SEV_DEBUG, "Load first STDF file.");
    if(!LoadStdfFile(lFirstStdf.first))
        return false;

    // Check if temp SQLite files should be removed
    bool lRemoveTempSqliteFile=false;
    if(getenv("GS_GTS_DELETE_TEMP_SQLITE_BEFORE_LOAD_FILES"))
        lRemoveTempSqliteFile=true;

    // Init GTL
    if(!GTL_Open(lRemoveTempSqliteFile))
    {
        DeleteCommand();
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Delete command
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::DeleteCommand()
{
    GSLOG(7, "DeleteCommand()");

    // Send command to output window
    m_pageOutput->Command(QString("delete\n"));

    // If required, call GTL close function
    if(!GTL_Close())
        return false;

    // Reset station
	ClearStation();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Simulate Retest
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::GTL_Retest()
{
    gtl_set((char*)GTL_KEY_DESIRED_LIMITS, (char*)mProperties.value(GTS_PROPERTY_DESIRED_LIMITS).toString()
            .toLatin1().data());

    gtl_set((char*)GTL_KEY_RETEST_HBINS, (char*)mKeysContent.Get("RetestBinList").toString().toLatin1().data());
    int lRes=gtl_command((char*)GTL_COMMAND_RETEST);
    if (lRes!=0)
    {
        Error(QString("gtl_retest failed (return code %1).").arg(lRes));
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Simulate Begin Job
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::GTL_BeginJob()
{
    //int lRes=gtl_beginjob();
    int lRes=gtl_command((char*)GTL_COMMAND_BEGINJOB);
    PopGtlMessageStack();
    // Update GTL state in case it changed (ie OFFLINE)
    UpdateGtlState();
    if (lRes!=GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_command(%1) failed (return code %2).").arg(GTL_COMMAND_BEGINJOB).arg(lRes));
        return false;
    }
    m_ui_gtl_beginjob_lib++;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Simulate End Job
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::GTL_EndJob()
{
    //int lRes=gtl_endjob();
    int lRes=gtl_command((char*)GTL_COMMAND_ENDJOB);
    PopGtlMessageStack();
    // Update part count and binning
    UpdateSummary();
    if (lRes!=GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_command(%1) failed (return code %2).").arg(GTL_COMMAND_ENDJOB).arg(lRes));
        return false;
    }
    m_ui_gtl_endjob_lib++;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Simulate test program run
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::RunTestProgram()
{
    int		nStatus=0, nRecordType=0;
    bool	bEndOfRun = false, bPartDetected = false, bDuplicateRecord=false;

    // Reset some variables
    m_bPartIsFail = false;
    mLastAutoTestNumber = 0;

	// Check if we should use last PIR record read
	if(m_bUseLastPIR)
	{
        m_bUseLastPIR = false;
        m_listSites.SetSiteStatus(m_clLastPIR.m_u1SITE_NUM, GtsStation_Site::eSiteStarted);

		// Call GTL beginjob function
        if(!GTL_BeginJob())
            return false;
        bPartDetected = true;
    }

	// Record loop
	nStatus = mStdfReader.LoadNextRecord(&nRecordType);
    while(((nStatus == GQTL_STDF::StdfParse::NoError)
           || (nStatus == GQTL_STDF::StdfParse::UnexpectedRecord)) && (!bEndOfRun))
	{
        bDuplicateRecord = true;
        switch(nRecordType)
		{
            case GQTL_STDF::Stdf_Record::Rec_PIR:
				// PIR record
                mStdfReader.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clLastPIR);
                if( m_listSites.CheckSiteStatus(m_clLastPIR.m_u1SITE_NUM, GtsStation_Site::eSiteStarted) ||
                    m_listSites.CheckSiteStatus(m_clLastPIR.m_u1SITE_NUM, GtsStation_Site::eSiteResultsAvailable))
				{
                    m_bUseLastPIR = true;
                    bEndOfRun = true;
				}
                else
                    m_listSites.SetSiteStatus(m_clLastPIR.m_u1SITE_NUM, GtsStation_Site::eSiteStarted);
                if(bPartDetected == false)
				{
                    bPartDetected = true;
                    // Call GTL beginjob function
                    if(!GTL_BeginJob())
                        return false;
                }
                break;

            case GQTL_STDF::Stdf_Record::Rec_PRR:
                // Process PRR (check PAT binning, ...)
                if(!ProcessPRR())
                    return false;
                bDuplicateRecord = false;

				// Check if all sites finished
                if(m_listSites.CheckAllSitesStatus(GtsStation_Site::eSiteResultsAvailable))
                    bEndOfRun = true;
				break;

            case GQTL_STDF::Stdf_Record::Rec_PTR:
				// Process PTR (normalize test name, check limits...)
                if(!ProcessPTR())
                    return false;
                bDuplicateRecord = false;
				break;

            case GQTL_STDF::Stdf_Record::Rec_MPR:
                // Process MPR (normalize test name, check limits...)
                if(!ProcessMPR())
                    return false;
                bDuplicateRecord = false;
                break;

            default:
                break;
		}

		// Write record to output file
		if(bDuplicateRecord && m_bGenerateStdfFile)
			mStdfReader.DumpRecord(&mStdfWriter);

		// Load next record, unless end of part reached
		if(!bEndOfRun)
            nStatus = mStdfReader.LoadNextRecord(&nRecordType);
    }

    // GS_QA: add a little delay (ms) to make sure GTM digests the packets
    if(isRunningModeQA())
    {
        QElapsedTimer lTimer;
        lTimer.start();
        while(lTimer.elapsed()<RunDelay())
            m_qApplication->processEvents();
    }

    // Check if we had a run
    if(!bEndOfRun)
	{
#if 0
        // no more gtl_endlot() in GTL V3.5
        // Call GTL EndOfLot function if need be
        if(!EndLot())
            return false;
#endif

        // End of STDF file reached, check if there are some more files to process...
        if(mStdfFiles.size() == 0)
        {
            Warning(QString("End of STDF file reached.\nYou must load a new STDF file."));
            return true;
        }

        // Get next file (first file in list of remaining files)
        QList< QPair< int, time_t> >  lKeys = mStdfFiles.keys();
        QPair<QString, GS::QtLib::DatakeysContent> lFirstStdf = mStdfFiles.take(lKeys.at(0));
        GS::QtLib::DatakeysContent  lNewKeysContent = lFirstStdf.second;

        // Next file could be:

        // 1) same retest index AND multi-session mode => multi-session resume
        if((lNewKeysContent.Get("RetestIndex").toInt() == mKeysContent.Get("RetestIndex").toInt()) &&
                (mProperties.value(GTS_PROPERTY_MULTIFILE_MODE).toString() == "ms"))
        {
            // Close GTL
            if(!GTL_Close())
            {
                Error(QString("GTL_Close failed."));
                DeleteCommand();
                return false;
            }

            // Assign new keyscontent
            mKeysContent = lNewKeysContent;

            // Load new file
            if(!LoadStdfFile(lFirstStdf.first))
            {
                Error(QString("LoadStdfFile failed."));
                DeleteCommand();
                return false;
            }

            // Open GTL
            if(!GTL_Open(false))
            {
                Error(QString("GTL_Open failed."));
                DeleteCommand();
                return false;
            }
        }

        // 2) retest index +1 AND multi-session mode => multi-session retest
        else if((lNewKeysContent.Get("RetestIndex").toInt() == (mKeysContent.Get("RetestIndex").toInt()+1)) &&
                (mProperties.value(GTS_PROPERTY_MULTIFILE_MODE).toString() == "ms"))
        {
            // Close GTL
            if(!GTL_Close())
            {
                Error(QString("GTL_Close failed."));
                DeleteCommand();
                return false;
            }

            // Assign new keyscontent
            mKeysContent = lNewKeysContent;

            // Set retest (set limit set to use, retest_hbins...)
            if(!GTL_Retest())
            {
                Error(QString("GTL_Retest failed."));
                DeleteCommand();
                return false;
            }

            // Load new file
            if(!LoadStdfFile(lFirstStdf.first))
            {
                Error(QString("LoadStdfFile failed."));
                DeleteCommand();
                return false;
            }

            // Open GTL
            if(!GTL_Open(false))
            {
                Error(QString("GTL_Open failed."));
                DeleteCommand();
                return false;
            }
        }

        // 3) retest index +1 AND on-the-fly option => on-the-fly retest
        else if((lNewKeysContent.Get("RetestIndex").toInt() == (mKeysContent.Get("RetestIndex").toInt()+1)) &&
                (mProperties.value(GTS_PROPERTY_MULTIFILE_MODE).toString() == "otf"))
        {
            // Assign new keyscontent
            mKeysContent = lNewKeysContent;

            // Set retest (set limit set to use, retest_hbins...)
            GTL_Retest();

            // Load new file
            if(!LoadStdfFile(lFirstStdf.first))
            {
                Error(QString("LoadStdfFile failed."));
                DeleteCommand();
                return false;
            }
        }

        // 4) anything else => continue as if same STDF
        else
        {
            // Assign new keyscontent
            mKeysContent = lNewKeysContent;

            // Load new file
            if(!LoadStdfFile(lFirstStdf.first))
            {
                Error(QString("LoadStdfFile failed."));
                DeleteCommand();
                return false;
            }
        }

        return true;
	}

    // Call GTL endjob function
    if(!GTL_EndJob())
        return false;

	// Update Output window for each site
	m_pageOutput->UpdateGUI(m_listSites);
	
	// Update Stats window for each site
	m_pageStats->UpdateGUI(m_listSites);
	
	// Reset sites
	m_listSites.ResetRun();

    if(!isRunningModeBench())
        // Force GUI refresh
        m_qApplication->processEvents();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// PRR has been read, process it
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::ProcessPRR()
{
    GQTL_STDF::Stdf_PRR_V4	clPRR;
    int				lNewSoftBin, lNewHardBin;

	// Read record from STDF parser
    mStdfReader.ReadRecord((GQTL_STDF::Stdf_Record *)&clPRR);

	// If this is a PAT Binning, replace it with a Bin1
	if(clPRR.m_u2SOFT_BIN == 141)
	{
		clPRR.m_u2SOFT_BIN = 1;
        clPRR.m_u2HARD_BIN = 1;
	}

    // Call GTL Binning function
    // 6995 : GTL 2.3 to 3.0
    //nBinning_Soft=gtl_binning_lib(clPRR.m_u1SITE_NUM, clPRR.m_u2SOFT_BIN);
    int r=gtl_binning(clPRR.m_u1SITE_NUM, clPRR.m_u2HARD_BIN, clPRR.m_u2SOFT_BIN, &lNewHardBin, &lNewSoftBin,
                      clPRR.m_cnPART_ID.toLatin1().constData());

    PopGtlMessageStack();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_binning failed (return code %1).").arg(r));
        return false;
    }
    m_ui_gtl_binning_lib++;

    // Update binnings in case they have been modified by GTL (PAT binning)
    clPRR.m_u2SOFT_BIN = lNewSoftBin;
    clPRR.m_u2HARD_BIN = lNewHardBin;

    // If valid PRR, update site results (ie Eagle testers use first PIR/PRR for
    // static test definition (Test Plan Delimiter). First PRR has HBin=65535.
    if(clPRR.m_u2HARD_BIN != 65535)
        m_listSites.SetSiteResults(clPRR.m_u1SITE_NUM, lNewSoftBin, lNewHardBin);
    else
        // Invalid run, consider site is not running
        m_listSites.ResetRun(clPRR.m_u1SITE_NUM);

	// Write PRR??
	if(m_bGenerateStdfFile)
        mStdfWriter.WriteRecord((GQTL_STDF::Stdf_Record *)(&clPRR));

    // Check if some new PAT limits are available.
    long lTestNumbers[1000];
    unsigned int lFlags[1000];
    unsigned int lHardBins[1000];
    unsigned int lSoftBins[1000];
    double lLowLimits[1000];
    double lHighLimits[1000];
    unsigned int lArrayFilledSize;
    QString lMessage;
    r = gtl_get_dpat_limits(clPRR.m_u1SITE_NUM, lTestNumbers, lFlags, lLowLimits, lHighLimits, lHardBins, lSoftBins, 1000, &lArrayFilledSize);
    if(r != GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_get_dpat_limits failed (return code %1).").arg(r));
        return false;
    }
    if(lArrayFilledSize > 0)
    {
        lMessage = QString("Received DPAT limits for %1 tests on site %2.").arg(lArrayFilledSize).arg(clPRR.m_u1SITE_NUM);
        GSLOG(SYSLOG_SEV_NOTICE, lMessage.toLatin1().constData());
        m_pageOutput->Printf(QString("INFO: %1\n").arg(lMessage));
        for(unsigned int lIndex=0; lIndex<lArrayFilledSize; ++lIndex)
        {
            lMessage = QString("**** Test %1: Flags=%2, LL=%3, HL=%4, Bin=%5.").arg(lTestNumbers[lIndex])
                    .arg(lFlags[lIndex]).arg(lLowLimits[lIndex]).arg(lHighLimits[lIndex]).arg(lHardBins[lIndex]);
            GSLOG(SYSLOG_SEV_NOTICE, lMessage.toLatin1().constData());
            m_pageOutput->Printf(QString("%1\n").arg(lMessage));
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// PTR has been read, process it
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::ProcessPTR()
{
    GtsTestlistMap::Iterator	lIterator;
    GQTL_STDF::Stdf_PTR_V4*	lPTR=NULL;
    bool                        lDeletePTR=true;
    GtsStation_TestDef*			lTestDef=NULL;
    QString						lTestName;
    int							lStatus;

    // Create PTR object
    lPTR = new GQTL_STDF::Stdf_PTR_V4;
    if(!lPTR)
    {
        Error(QString("Error processing PTR. Could not allocate Stdf_PTR_V4."));
        return false;
    }

	// Read record from STDF parser
    mStdfReader.ReadRecord((GQTL_STDF::Stdf_Record *)lPTR);

	// If test name valid, normalize it
    if(lPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposTEST_TXT))
	{
        lTestName = lPTR->m_cnTEST_TXT.trimmed();
        lStatus = lTestName.indexOf(" <>");
        if(lStatus >= 0)
            lTestName.truncate(lStatus);
        lPTR->SetTEST_TXT(lTestName);
	}

    if (isAutoTestNumbers())
    {
        ++mLastAutoTestNumber;
        lPTR->SetTEST_NUM(mLastAutoTestNumber);
    }

	// Update map
    lIterator = m_mapTestlist.find((long)lPTR->m_u4TEST_NUM);
    if(lIterator == m_mapTestlist.end())
	{
        // Test not found in map. Create one. The PTR object will be deleted by it.
        lDeletePTR = false;

		// Insert test into map
        lTestDef = new GtsStation_TestDef(lPTR);
        m_mapTestlist.insert((long)lPTR->m_u4TEST_NUM, lTestDef);
	}
	else
	{
		// Test found in map, get it
        lTestDef = lIterator.value();
	}

	// Call GTL test result function
    if(lPTR->IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposRESULT))
  {
      int r=gtl_test(lPTR->m_u1SITE_NUM, lPTR->m_u4TEST_NUM,
                         (lTestDef->GetTestName().toLatin1().data()), lPTR->m_r4RESULT);
      PopGtlMessageStack();
      if (r!=GTL_CORE_ERR_OKAY && r!=GTL_CORE_ERR_INVALID_TEST)
      {
          Error(QString("gtl_test failed for site %1, test %2/%3 (return code %4).").arg(lPTR->m_u1SITE_NUM)
                .arg(lPTR->m_u4TEST_NUM).arg(lTestDef->GetTestName()).arg(r));
          // Need to delete PTR?
          if(lDeletePTR && lPTR)
          {
              delete lPTR;
              lPTR = NULL;
          }
          return false;
      }
      //if (r==GTL_CORE_ERR_INVALID_TEST)
      //    Warning(QString("gtl_test failed for site %1, test %2/%3 (return code %4).").arg(lPTR->m_u1SITE_NUM)
      //          .arg(lPTR->m_u4TEST_NUM).arg(pclTestDef->mPTR->m_cnTEST_TXT).arg(r));
      m_ui_gtl_test_lib++;
  }

	// Update Reference PTR and current PTR if need be (new limits defined...)
    bool bTestPassed = lTestDef->Update(lPTR);

	// Write PTR??
	if(m_bGenerateStdfFile && (!(m_pageSetup->IsStopOnFailON()) || !m_bPartIsFail))
        mStdfWriter.WriteRecord((GQTL_STDF::Stdf_Record *)(lPTR));

	// Check if test is fail
	if(!bTestPassed)
        m_bPartIsFail = true;

    // Need to delete PTR?
    if(lDeletePTR && lPTR)
    {
        delete lPTR;
        lPTR = NULL;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// MPR has been read, process it
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::ProcessMPR()
{
    GtsTestlistMap::Iterator	lIterator;
    GQTL_STDF::Stdf_MPR_V4*	lMPR=NULL;
    bool                        lDeleteMPR=true;
    GtsStation_TestDef*			lTestDef=NULL;
    QString						lTestName;
    int							lStatus;

    // Create MPR object
    lMPR = new GQTL_STDF::Stdf_MPR_V4;
    if(!lMPR)
    {
        Error(QString("Error processing MPR. Could not allocate Stdf_MPR_V4."));
        return false;
    }

    // Read record from STDF parser
    mStdfReader.ReadRecord((GQTL_STDF::Stdf_Record *)lMPR);

    // If test name valid, normalize it
    if(lMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposTEST_TXT))
    {
        lTestName = lMPR->m_cnTEST_TXT.trimmed();
        lStatus = lTestName.indexOf(" <>");
        if(lStatus >= 0)
            lTestName.truncate(lStatus);
        lMPR->SetTEST_TXT(lTestName);
    }

    if (isAutoTestNumbers())
    {
        ++mLastAutoTestNumber;
        lMPR->SetTEST_NUM(mLastAutoTestNumber);
    }

    // Update map
    lIterator = m_mapTestlist.find((long)lMPR->m_u4TEST_NUM);
    if(lIterator == m_mapTestlist.end())
    {
        // Test not found in map. Create one. The MPR object will be deleted by it.
        lDeleteMPR = false;

        // Insert test into map
        lTestDef = new GtsStation_TestDef(lMPR);
        m_mapTestlist.insert((long)lMPR->m_u4TEST_NUM, lTestDef);
    }
    else
    {
        // Test found in map, get it
        lTestDef = lIterator.value();
    }

    // Call GTL test result function
    if(lMPR->IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposRTN_RSLT))
    {
        // Prepare pin indexes array
        int* lPinIndexes = (int*)malloc(lMPR->m_u2RSLT_CNT*sizeof(int));
        if(!lPinIndexes)
        {
            Error(QString("Could not allocate Pin Indexes array for %1 integers.").arg(lMPR->m_u2RSLT_CNT));
            // Need to delete MPR?
            if(lDeleteMPR && lMPR)
            {
                delete lMPR;
                lMPR = NULL;
            }
            return false;
        }
        double* lPinResults = (double*)malloc(lMPR->m_u2RSLT_CNT*sizeof(double));
        if(!lPinResults)
        {
            Error(QString("Could not allocate Pin Results array for %1 doubles.").arg(lMPR->m_u2RSLT_CNT));
            delete lPinIndexes;
            // Need to delete MPR?
            if(lDeleteMPR && lMPR)
            {
                delete lMPR;
                lMPR = NULL;
            }
            return false;
        }
        for(int lIndex=0; lIndex<lMPR->m_u2RSLT_CNT; ++lIndex)
            lPinIndexes[lIndex] = lIndex;
        for(int lIndex=0; lIndex<lMPR->m_u2RSLT_CNT; ++lIndex)
            lPinResults[lIndex] = lMPR->m_kr4RTN_RSLT[lIndex];
        int r=gtl_mptest(lMPR->m_u1SITE_NUM, lMPR->m_u4TEST_NUM,
                         (lTestDef->GetTestName().toLatin1().data()), lPinResults, lPinIndexes, lMPR->m_u2RSLT_CNT);
        delete lPinIndexes; lPinIndexes=0;
        delete lPinResults; lPinResults=0;
        PopGtlMessageStack();
        if (r!=GTL_CORE_ERR_OKAY && r!=GTL_CORE_ERR_INVALID_TEST
                              && r!=GTL_CORE_ERR_INVALID_PIN ) // WT : perhaps this MP tests is not in the recipe...
        {
            Error(QString("gtl_mptest failed for site %1, test %2 '%3' (return code %4).").arg(lMPR->m_u1SITE_NUM)
                  .arg(lMPR->m_u4TEST_NUM).arg(lTestDef->GetTestName()).arg(r));
            // Need to delete MPR?
            if(lDeleteMPR && lMPR)
            {
                delete lMPR;
                lMPR = NULL;
            }
            return false;
        }
        //if (r==GTL_CORE_ERR_INVALID_TEST)
        //    Warning(QString("gtl_test failed for site %1, test %2/%3 (return code %4).").arg(lMPR->m_u1SITE_NUM)
        //          .arg(lMPR->m_u4TEST_NUM).arg(pclTestDef->GetTestName()).arg(r));
        m_ui_gtl_mptest_lib++;
  }

    // Update Reference MPR and current MPR if need be (new limits defined...)
    bool bTestPassed = lTestDef->Update(lMPR);

    // Write MPR??
    if(m_bGenerateStdfFile && (!(m_pageSetup->IsStopOnFailON()) || !m_bPartIsFail))
        mStdfWriter.WriteRecord((GQTL_STDF::Stdf_Record *)(lMPR));

    // Check if test is fail
    if(!bTestPassed)
        m_bPartIsFail = true;

    // Need to delete MPR?
    if(lDeleteMPR && lMPR)
    {
        delete lMPR;
        lMPR = NULL;
    }

    return true;
}

void GtsStationMainwindow::RunBenchmark()
{
  //qint64 elapsed=0;
  //QElapsedTimer timer;

  QString strStdfFile = m_strApplicationPath + "/data_samples.std";
  strStdfFile = "S:\\galaxy_docs\\roadmaps\\specifications\\Product\\FT_PAT\\6935\\Validation_Data\\TNA0KA031E_R_FT_ROOM_ETS88-016A_20120614_012427.stdf";
  GtsStation_Binning clBinPat(141, GtsStation_Binning::eBinningFail), *pBinPat;
  int nIndex;
  unsigned int uiIndex;

  m_pageInfo->editBenchPatDisabled->clear();
  m_pageInfo->editBenchPatEnabled->clear();
  buttonExit->setEnabled(false);

  // Run N times in ENABLED mode
  for(uiIndex=0; uiIndex<BENCHMARK_PASSES; uiIndex++)
  {
    m_pageInfo->editBenchPatEnabled->setText(QString("PAT ENABLED: pass %1 out of %2").arg(uiIndex+1).arg(BENCHMARK_PASSES));
    m_qApplication->processEvents();

    LoadCommand(QStringList(strStdfFile));

    //timer.start();
    OnButtonRunAll();
    //elapsed = timer.elapsed();

    clBinPat.m_uiBinCount = 0;
    nIndex = m_listSites.m_listSoftBinnings.Find(&clBinPat);
    if(nIndex != -1)
    {
      pBinPat = m_listSites.m_listSoftBinnings.at(nIndex);
      if(pBinPat)
        clBinPat.m_uiBinCount = pBinPat->m_uiBinCount;
    }

    DeleteCommand();
    m_qApplication->processEvents();
  }

  // Run N times in DISABLED mode
  for(uiIndex=0; uiIndex<BENCHMARK_PASSES; uiIndex++)
  {
    m_pageInfo->editBenchPatDisabled->setText(QString("PAT DISABLED: pass %1 out of %2").arg(uiIndex+1).arg(BENCHMARK_PASSES));
    m_qApplication->processEvents();

    LoadCommand(QStringList(strStdfFile));

    //timer.start();
#if 0 // Deprecated since GTL V3.6. Need to implement gtl_set("pause", "on/off") or something similar
    OnGtlCommandsDialog_Send(GTS_STATION_GTLCOMMANDS_DISABLED,0);
#endif
    OnButtonRunAll();
    //elapsed = timer.elapsed();

    clBinPat.m_uiBinCount = 0;
    nIndex = m_listSites.m_listSoftBinnings.Find(&clBinPat);
    if(nIndex != -1)
    {
      pBinPat = m_listSites.m_listSoftBinnings.at(nIndex);
      if(pBinPat)
        clBinPat.m_uiBinCount = pBinPat->m_uiBinCount;
    }

    DeleteCommand();
    m_qApplication->processEvents();
  }

  buttonExit->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////
// Run all STDF for QA
/////////////////////////////////////////////////////////////////////////////////////
bool GtsStationMainwindow::RunQA()
{
    GtsStation_Binning    clBinPat(141, GtsStation_Binning::eBinningFail), *pBinPat;
    int                   nIndex;
    unsigned int          uiIndex;

    GSLOG(SYSLOG_SEV_DEBUG, "Running QA loop...");

    m_pageInfo->editBenchPatDisabled->clear();
    m_pageInfo->editBenchPatEnabled->clear();
    buttonExit->setEnabled(false);

    // Run 1 times in ENABLED mode
    for(uiIndex=0; uiIndex<1; uiIndex++)
    {
        m_pageInfo->editBenchPatEnabled->setText(QString("PAT ENABLED: pass %1 out of %2").arg(uiIndex+1).arg(BENCHMARK_PASSES));
        m_qApplication->processEvents();

        QStringList lDataFiles, lFiles;
        if(!mProperties.value(GTS_PROPERTY_DATAFOLDER).toString().isEmpty())
        {
            GSLOG(SYSLOG_SEV_NOTICE, QString("Scanning folder '%1'").arg(mProperties.value(GTS_PROPERTY_DATAFOLDER).toString())
                  .toLatin1().data() );
            mCurrentDir = mProperties.value(GTS_PROPERTY_DATAFOLDER).toString();
            QDir lDir(mCurrentDir);
            lFiles = lDir.entryList(QDir::nameFiltersFromString("*.std;*.stdf"),QDir::Files);
            GSLOG(SYSLOG_SEV_NOTICE, QString("%1 datafiles found").arg(lFiles.size()).toLatin1().data() );
            // Make a list with full file name
            while(!lFiles.isEmpty())
                lDataFiles.append(mCurrentDir+QDir::separator()+lFiles.takeFirst());
        }
        else
        {
            lDataFiles = QStringList(mProperties.value(GTS_PROPERTY_STDF_FILE).toString());
            mCurrentDir = QFileInfo(lDataFiles.first()).path();
        }

        if(!LoadCommand(lDataFiles))
        {
            GSLOG(SYSLOG_SEV_ERROR, "LoadCommand() failed");
            return false;
        }

        if(!RunAll())
        {
            GSLOG(SYSLOG_SEV_ERROR, "RunAll() failed");
            if(!DeleteCommand())
                GSLOG(SYSLOG_SEV_ERROR, "DeleteCommand() failed");
            return false;
        }

        clBinPat.m_uiBinCount = 0;
        nIndex = m_listSites.m_listSoftBinnings.Find(&clBinPat);
        if(nIndex != -1)
        {
            pBinPat = m_listSites.m_listSoftBinnings.at(nIndex);
            if(pBinPat)
                clBinPat.m_uiBinCount = pBinPat->m_uiBinCount;
        }

        // Add a little delay (2sec) to make sure GTM digests the packets
        QElapsedTimer lTimer;
        lTimer.start();
        while(lTimer.elapsed()<StopDelay())
            m_qApplication->processEvents();

        if(!DeleteCommand())
        {
            GSLOG(3, "DeleteCommand() failed");
            return false;
        }

        m_qApplication->processEvents();
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// 'loadDelete' button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonLoadDelete()
{
    if(isRunningModeBench())
    {
        if(m_bProgramLoaded == false)
        {
            RunBenchmark();
        }
        else
        {
            // Delete command
            DeleteCommand();
        }
    }
    else
    {
        if(m_bProgramLoaded == false)
        {
            // Display Open file dialog so the user can select a STDF file
            QStringList strFileNames = QFileDialog::getOpenFileNames(this, tr("Select the STDF files to load"),
                                                             mCurrentDir, tr("STDF files (*.std *.stdf)"));

            // Load command
            if(!strFileNames.isEmpty())
            {
                mCurrentDir = QFileInfo(strFileNames.first()).path();
                LoadCommand(strFileNames);
            }
        }
        else
        {
            // Delete command
            DeleteCommand();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// Display help section
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::DisplayHelp()
{
	// Display 'Help' page
    listMainSelection->setCurrentItem(listMainSelection->item(m_uiPage_Help));
}

/////////////////////////////////////////////////////////////////////////////////////
// 'Recipe' button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonRecipe()
{
    // Display Open file dialog so the user can select a recipe file
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a recipe file"),
                                                     mCurrentDir,
                                                     tr("Recipes (*.csv *.json)"));
    if(!fileName.isEmpty())
    {
        mCurrentDir = QFileInfo(fileName).path();
        editRecipe->setText(fileName);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// 'TesterConf' button clicked
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::OnButtonTesterConf()
{
    // Display Open file dialog so the user can select a tester conf file
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a tester conf file"),
                                                     mCurrentDir,
                                                     tr("Conf files (*.conf)"));
    if(!fileName.isEmpty())
    {
        mCurrentDir = QFileInfo(fileName).path();
        editTesterConf->setText(fileName);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// Issue message (LOG and eventually GUI)
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::Message(const QString &lMessage, const int Severity/*=SYSLOG_SEV_NOTICE*/)
{
    GSLOG(Severity, lMessage.toLatin1().constData());
    m_pageOutput->Printf(QString("MESSAGE: %1\n").arg(lMessage));
}

/////////////////////////////////////////////////////////////////////////////////////
// Issue warning (LOG and eventually GUI)
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::Warning(const QString &lMessage, const int Severity/*=SYSLOG_SEV_WARNING*/)
{
    GSLOG(Severity, lMessage.toLatin1().constData());
    m_pageOutput->Printf(QString("WARNING: %1\n").arg(lMessage));
}

/////////////////////////////////////////////////////////////////////////////////////
// Issue error (LOG and eventually GUI)
/////////////////////////////////////////////////////////////////////////////////////
void GtsStationMainwindow::Error(const QString &lMessage, const int Severity /*=SYSLOG_SEV_ERROR*/)
{
    GSLOG(Severity, lMessage.toLatin1().constData());
    m_pageOutput->Printf(QString("ERROR: %1\n").arg(lMessage));
}

bool GtsStationMainwindow::EndLot()
{
    // No more gtl_endlot() in GTL V3.5
    return true;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Perform end of lot if need be...");

    // If required, call GTL end lot function
    if(GetGTLLibState() == GTL_STATE_NOT_INITIALIZED)
        return true;

#if 0 // No more gtl_endlot() in GTL V3.5
    // Call GTL EndOfLot function
    int r=gtl_endlot();
    PopGtlMessageStack();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        Error(QString("gtl_endlot failed (return code %1).").arg(r));
        return false;
    }
#endif

    return true;
}

bool GtsStationMainwindow::CheckArguments(int argc, char **argv)
{
    // Get GS_QA env variables
    char *lEnvTC = getenv("GS_GTS_TESTERCONF");
    if(lEnvTC)
        mProperties.insert(GTS_PROPERTY_TC_FILE, QString(lEnvTC));
    char *lEnvRecipe = getenv("GS_GTS_RECIPE");
    if(lEnvRecipe)
        mProperties.insert(GTS_PROPERTY_RECIPE_FILE, QString(lEnvRecipe));
    char *lEnvStdf = getenv("GS_GTS_STDF");
    if(lEnvStdf)
        mProperties.insert(GTS_PROPERTY_STDF_FILE, QString(lEnvStdf));
    char *lEnvQAOutputFolder = getenv("GS_QA_OUTPUT_FOLDER");
    if(lEnvQAOutputFolder)
        mProperties.insert(GTS_PROPERTY_QA_OUTPUT_FOLDER, QString(lEnvQAOutputFolder));
    char* lEnvDataFolder = getenv("GS_GTS_DATAFOLDER");
    if (lEnvDataFolder)
    {
        mProperties.insert(GTS_PROPERTY_DATAFOLDER, QString(lEnvDataFolder)); // todo : scan all datafiles and fill GUI
        GSLOG(5, QString("DataFolder: %1").arg(lEnvDataFolder).toLatin1().data() );
    }

    // Check if in GS_QA mode
    if(getenv("GS_QA"))
    {
        Message(QString("GS_QA defined to %1: lets check all other env vars...").arg(getenv("GS_QA")));
        // In QA mode: force station nb to 1, GS_QA_OUTPUT_FOLDER must be specified in env variables
        if(!lEnvQAOutputFolder)
        {
            Error(QString("GS_QA_OUTPUT_FOLDER environment variable must be defined along with GS_QA."));
            return false;
        }
        mProperties.insert(GTS_PROPERTY_RUNNING_MODE, GtsStationMainwindow::QA);
    }

    // If all entries are defined, force automatic mode
    if(lEnvTC && lEnvRecipe && (lEnvStdf || lEnvDataFolder))
        mProperties.insert(GTS_PROPERTY_AUTO_RUN, true);

    if(getenv("GS_GTM_BENCH"))
        // In BENCH mode: force station nb to 1
        mProperties.insert(GTS_PROPERTY_RUNNING_MODE, GtsStationMainwindow::Bench);

    // Loop through arguments
    QString         lArgument;
    bool            lOk;
    unsigned int    lValue;
    for(int lIdx = 1; lIdx < argc; ++lIdx)
    {
        lArgument = QString(argv[lIdx]).toLower();
        GSLOG(5, QString("Interpreting argument '%1' ...").arg(lArgument).toLatin1().data() );
        if((lArgument=="-h") || (lArgument=="--hidden"))
            mProperties.insert(GTS_PROPERTY_HIDDEN, true);
        else if((lArgument=="-ac") || (lArgument=="--autoclose"))
            mProperties.insert(GTS_PROPERTY_AUTO_CLOSE, true);
        else if(lArgument == "--auto_testnumbers")
        {
            mProperties.insert(GTS_PROPERTY_AUTO_TESTNUMBERS, true);
        }
        else if(lArgument.startsWith("--station="))
        {
            lValue = lArgument.section("=", 1).toUInt(&lOk);
            if(!lOk)
            {
                Error(QString("Invalid argument %1.").arg(lArgument));
                return false;
            }
            mProperties.insert(GTS_PROPERTY_STATION_NB, lValue);
        }
        else if(lArgument.startsWith("--rundelay="))
        {
            lValue = lArgument.section("=", 1).toUInt(&lOk);
            if(!lOk)
            {
                Error(QString("Invalid argument %1.").arg(lArgument));
                return false;
            }
            mProperties.insert(GTS_PROPERTY_RUN_DELAY, lValue);
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Run delay set to %1 ms").arg(lValue).toLatin1().data() );
        }
        else if(lArgument.startsWith("--stopdelay="))
        {
            lValue = lArgument.section("=", 1).toUInt(&lOk);
            if(!lOk)
            {
                Error(QString("Invalid argument %1.").arg(lArgument));
                return false;
            }
            mProperties.insert(GTS_PROPERTY_STOP_DELAY, lValue);
        }
        else if(lArgument.startsWith("--gtm_communication="))
        {
            mProperties.insert(GTS_PROPERTY_GTM_COMM_MODE, lArgument.section("=", 1));
        }
        else if(lArgument.startsWith("--multifile_mode="))
        {
            mProperties.insert(GTS_PROPERTY_MULTIFILE_MODE, lArgument.section("=", 1));
        }
        else if(lArgument.startsWith("--desired_limits="))
        {
            mProperties.insert(GTS_PROPERTY_DESIRED_LIMITS, lArgument.section("=", 1));
        }
        else
        {
            Error(QString("Invalid argument %1.").arg(lArgument));
            return false;
        }
    }

    return true;
}

bool GtsStationMainwindow::isRunningModeQA()
{
    bool lOk=true;
    unsigned int lValue = mProperties.value(GTS_PROPERTY_RUNNING_MODE, QVariant(Normal)).toUInt(&lOk);
    return (lOk && (lValue == QA));
}

bool GtsStationMainwindow::isRunningModeBench()
{
    bool lOk=true;
    unsigned int lValue = mProperties.value(GTS_PROPERTY_RUNNING_MODE, QVariant(Normal)).toUInt(&lOk);
    return (lOk && (lValue == Bench));
}
