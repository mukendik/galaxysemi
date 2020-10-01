/****************************************************************************!
 * \file drillDataMining3D.cpp
 * \brief Interactive Drill: 3D Data Mining
 ****************************************************************************/
#ifdef _WIN32
# include <windows.h>
#endif

#include "component.h"
#include <QApplication>
#include <QInputDialog>
#include <QListIterator>
#include "message.h"
#include "gex_shared.h"
#include "drillDataMining3D.h"
#include "report_build.h"
#include "report_options.h"
#include "browser_dialog.h"
#include "snapshot_dialog.h"
#include "settings_dialog.h"
#include "assistant_flying.h"
#include "drill_table.h"
#include "picktest_dialog.h"
#include "pickpart_dialog.h"
#include "cstats.h"
#include "cpart_info.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "scale_color_widget.h"
#include "gexpanelwidget.h"
#include "drill_3d_viewer.h"
#include "pat_engine.h"
#include "patman_lib.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "wafermap_parametric.h"
#include "ofr_controller.h"

#if defined unix || __MACH__
# include <stdlib.h>
# include <unistd.h>
#endif
#include <math.h>

// Galaxy QT libraries
#include "gqtl_sysutils.h"

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"

const QString prefix = "(W)";

// main.cpp
extern GexMainwindow* pGexMainWindow;

// cstats.cpp
extern double ScalingPower(int iPower);

// in report_build.cpp
extern CGexReport*    gexReport; // Handle to report class
extern CReportOptions ReportOptions;  // Holds options (report_build.h)

namespace Gex {

#define GEX3D_EMPTY_CELL   -1  // Cell Heigth marked negative for empty cells.

#define GEX3D_SELECT_COLOR -1  // Cell color for selected cells.
#define GEX3D_NNR_COLOR    -2  // Cell color for NNR cells.
#define GEX3D_PASS_COLOR   -3  // Cell color for NNR cells.
#define GEX3D_FAIL_COLOR   -4  // Cell color for NNR cells.

// If over 7000 Dies objects to paint, disable spinning
#define GEX3D_MAX_WAF_OBJECTS_SPIN  1
// If over 100 Parts objects to paint, disable spinning
#define GEX3D_MAX_PACK_OBJECTS_SPIN 100
// If up to 25 Parts, render a chip image on each part!
#define GEX3D_MAX_OBJECTS_CHIP      25
#define SELECT_BUF_SIZE 256

#define GEX3D_SPECTRUM_IGNORE     -1
#define GEX3D_SPECTRUM_DATARANGE  0
#define GEX3D_SPECTRUM_TESTLIMITS 1
#define GEX3D_SPECTRUM_PASSFAIL   2

static double lfDrillHighL;
static double lfDrillLowL;
static double fDataStart = C_INFINITE;
static double fDataEnd   = -C_INFINITE;
static double lfHistoClassesWidth;  // MaxSample - MinSample
double lfBarScaling;
static CTest* ptDrillTestCell;
long iRenderIndex;

#ifndef DRILL_NO_DATA_MINING
static Vec previousPoint;
#endif

/******************************************************************************!
 * \fn DrillDataMining3D
 * \brief Constructor
 ******************************************************************************/
DrillDataMining3D::DrillDataMining3D(QWidget* parent, Qt::WindowFlags fl)
    : GS::Gex::QTitleFor< QDialog >(parent, fl), mWizardHandler(0)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "new DrillDataMining3D...");

    mIndex = -1;
    setupUi(this);
    setModal(false);

    setFocusPolicy(Qt::StrongFocus);

    // apply gex palette
    GexMainwindow::applyPalette(this);

#ifndef DRILL_NO_DATA_MINING

    m_nSpectrumColors = GEX3D_SPECTRUM_DATARANGE;
    m_eMode = DrillDataMining3D::standard;

    // Build path to HOME or PC-Windows directory...where Neptus
    // Properties HTML pages will be created...
    strPropertiesPath = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();

    // Build full page to HTML page used for Neptus Selection's Info.
    // Config file for info to access PRIOR to process scripts.
    strPropertiesPath = strPropertiesPath + GEX3D_NEPTUS_INFO;

    // init viewer
    m_pSplitter    = new QSplitter(this);
    m_pSplitter->insertWidget(0, m_pViewer      = new GexGLViewer(NULL));
    m_pSplitter->insertWidget(1, m_pPanelWidget = new GexPanelWidget(NULL));

    frameWaferMap->layout()->addWidget(m_pSplitter);

    m_pSplitter->setCollapsible(0, false);
    m_pSplitter->setCollapsible(1, false);
    m_pSplitter->setOpaqueResize(false);

    // Init panel widget
    m_pPropertiesBrowser = new QTextBrowser();
    m_pPropertiesBrowser->setSource(QUrl::fromLocalFile(strPropertiesPath));

    m_pScaleColorWidget = new GexScaleColorWidget();

    m_pPanelWidget->addPanelWidget("Properties", m_pPropertiesBrowser);
    m_pPanelWidget->addPanelWidget("Scale colors", m_pScaleColorWidget);

    // Reset Flags
    m_FullScreen     = false;
    m_RenderMode     = FlatBars;  // Drawing objects with Flat blocs (default)
    landscape        = NULL;
    vertexNormals    = NULL;
    normals          = NULL;
    hProperties      = NULL;
    landscapeSize    = 0;
    landscapeObjects = 0;

    // Check Parameter filter
    buttonFilterParameters->setChecked(gexReport->hasInteractiveTestFilter());

    // At startup: none selected.
    m_lstSelections.clear();

    // Initialize contents of ui control
    initGui();

    // Init OpenGL Font library
    InitOpenGLFont();

    m_pclFont3D = new CFont_3D;

    ShowGlobalProperties();

    //////////////////////////////////////////////////////////////////////////////
    // Connect signals :
    // MUST BE DONE AT THE END OF THE CONSTRUCTOR TO AVOID TO CATCH SIGNALS
    //////////////////////////////////////////////////////////////////////////////
    InitInteractiveView();

    if (! connect(pushButtonShowParam, SIGNAL(clicked(bool)),
                  m_pPanelWidget, SLOT(onOpenClose(bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect hideshow button fail");
    }
    if (! connect(buttonFilterParameters, SIGNAL(clicked(bool)),
                  SLOT(onFilterParameters(bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onFilterParameters fail");
    }
    if (! connect(m_pViewer, SIGNAL(viewerRightClickSelection(const QPoint &)),
                  SLOT(onContextualMenu(const QPoint &))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onContextualMenu fail");
    }
    if (! connect(m_pViewer, SIGNAL(viewerLeftClickSelection(const QPoint &)),
                  SLOT(onSelectionDone(const QPoint &))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onSelectionDone fail");
    }
    if (! connect(m_pViewer, SIGNAL(viewerDrawing()), SLOT(onPaintGL())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onPaintGL fail");
    }
    if (! connect(buttonFind, SIGNAL(clicked()), SLOT(OnFindDie())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect OnFindDie fail");
    }
    if (! connect(buttonHome, SIGNAL(clicked()), m_pViewer, SLOT(goHome())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect goHome fail");
    }
    if (! connect(comboBoxChartType, SIGNAL(activated(int)),
                  SLOT(onChangeChartType(int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onChangeChartType fail");
    }
    if (! connect(comboBoxSpectrumColors, SIGNAL(activated(int)),
                  SLOT(onChangeSpectrumColors(int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onChangeSpectrumColors fail");
    }
    if (! connect(pushButtonPickTest, SIGNAL(clicked()), SLOT(onPickTest())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onPickTest fail");
    }
    if (! connect(m_pScaleColorWidget, SIGNAL(spectrumValueChanged()),
                  SLOT(onSpectrumValueChanged())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onSpectrumValueChanged fail");
    }
    if (! connect(&m_drillDataInfo, SIGNAL(dataLoaded(bool, bool)),
                  SLOT(onDataLoaded(bool, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onDataLoaded fail");
    }
    if (! connect(&m_drillDataInfo, SIGNAL(testNavigate()),
                  SLOT(onTestNavigate())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onTestNavigate fail");
    }
    if (! connect(pushButtonNextTest, SIGNAL(clicked()),
                  &m_drillDataInfo, SLOT(nextTestCell())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect nextTestCell fail");
    }
    if (! connect(pushButtonPreviousTest, SIGNAL(clicked()),
                  &m_drillDataInfo, SLOT(previousTestCell())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect previousTestCell fail");
    }
    if (! connect(comboBoxWafermap, SIGNAL(activated(int)),
                  &m_drillDataInfo, SLOT(changeFileID(int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect changeFileID fail");
    }
    if (! connect(comboBoxGroup, SIGNAL(activated(int)),
                  SLOT(onChangeGroup(int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onChangeGroup fail");
    }
    if (! connect(pushButtonSave, SIGNAL(clicked()), SLOT(onExportWaferMap())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onExportWaferMap fail");
    }
    if (! connect(gexReport, SIGNAL(interactiveFilterChanged(bool)),
                  SLOT(onFilterChanged(bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect onFilterChanged fail");
    }
    if (! connect(m_pScaleColorWidget->binScaleColor->treeWidget,
                  SIGNAL(itemChanged(QTreeWidgetItem *, int)),
                  SLOT(OnChangeBinFilter(QTreeWidgetItem *, int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect OnChangeBinFilter fail");
    }

    //
    //m_pPanelWidget->onOpenClose(false);

#endif  // DRILL_NO_DATA_MINING
}


/******************************************************************************!
 * \fn ~DrillDataMining3D
 * \brief Destructor
 ******************************************************************************/
DrillDataMining3D::~DrillDataMining3D()
{
   // GSLOG(SYSLOG_SEV_DEBUG, "Deleting DrillDataMining3D...");
#ifndef DRILL_NO_DATA_MINING
    // Delete OpenGL objects
    glDeleteLists(m_glDispList, 1);

    // Delete lamdscape array
    destroyLandscape();

    if (m_pclFont3D)
    {
        delete m_pclFont3D;
    }
    m_pclFont3D = 0;
    mWizardHandler->RemoveWizardWaferMap3D(this);
#endif

}


void DrillDataMining3D::InitInteractiveView()
{
    QPixmap  lTable(50, 50);
    lTable.load(QString::fromUtf8(":/gex/icons/options_statistics.png"));

    QPixmap lWafermap(50, 50);
    lWafermap.load(QString::fromUtf8(":/gex/icons/options_wafermap.png"));

    QPixmap lHisto(50, 50);
    lHisto.load(QString::fromUtf8(":/gex/icons/options_advhisto.png"));


    QString lOPenWafer("Open a Wafermap");
    lOPenWafer.append("\t(Ctrl+W)");

    QString lOPenTable("Open a Table");
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        lOPenTable.append("\t(Ctrl+T)");

    QString lOPenDiagram("Open a Diagram");
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        lOPenDiagram.append("\t(Ctrl+D)");

    QAction* lDisplayWafermap   = new QAction(lWafermap,     lOPenWafer,     this );
    QAction* lDisplayTable      = new QAction(lTable,        lOPenTable,     this );
    QAction* lDisplayHisto      = new QAction(lHisto,        lOPenDiagram,   this );

    QAction* lSwitchChart      = new QAction(lHisto, "Switch to Diagram", this );

    connect(lSwitchChart,       SIGNAL(triggered()), this, SLOT(OnSwitchInteractiveCharts()));

    connect(lDisplayTable,      SIGNAL(triggered()), this, SLOT(OnInteractiveTables()));
    connect(lDisplayWafermap,   SIGNAL(triggered()), this, SLOT(OnInteractiveWafermap()));
    connect(lDisplayHisto,      SIGNAL(triggered()), this, SLOT(OnInteractiveCharts()));

    QMenu* lMenu= new QMenu(toolButtonInteractiveViews);

    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
    {
        lMenu->addAction(lSwitchChart);
        lMenu->addSeparator();
    }

    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        lMenu->addAction(lDisplayWafermap);

    lMenu->addAction(lDisplayTable);
    lMenu->addAction(lDisplayHisto);

    toolButtonInteractiveViews->setMenu(lMenu);
}


void DrillDataMining3D::OnSwitchInteractiveCharts()
{
    pGexMainWindow->mWizardsHandler.SetWidgetToSwitch(this);
    pGexMainWindow->mWizardsHandler.RemoveWizardWaferMap3D(this);
    OnInteractiveCharts();
}

/******************************************************************************!
 * \fn OnInteractiveTables
 * \brief Switch to the Interactive Tables display
 ******************************************************************************/
void DrillDataMining3D::OnInteractiveTables()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: on interactive tables...");
    pGexMainWindow->Wizard_DrillTable("", false);
}

QString DrillDataMining3D::BuildInteractiveLinkWafermap()
{
    CTest*  ptTestCell = m_drillDataInfo.currentTestCell();
    // Build URL
    QString strLink = "";
    if (ptTestCell)
    {
        strLink  = "#_gex_drill--drill_3d=wafer--g=";
        strLink += QString::number(0);
        strLink += "--f=0--Test=";
        strLink += QString::number(ptTestCell->lTestNumber);
        strLink += "--Pinmap=";
        strLink += QString::number(ptTestCell->lPinmapIndex);
    }

    return   strLink;
}

void DrillDataMining3D::OnInteractiveWafermap()
{
    pGexMainWindow->Wizard_Drill3D(BuildInteractiveLinkWafermap(), false);
}


QString DrillDataMining3D::BuildInteractiveLinkChart()
{
    // Build hyperlink argument string: drill_chart=adv_histo--data=<test#>
    QString strLink    = "";
    CTest*  ptTestCell = m_drillDataInfo.currentTestCell();

    if (ptTestCell)
    {
        strLink  = "drill--drill_chart=adv_histo--data=";
        strLink += QString::number(ptTestCell->lTestNumber);
       // strLink += QString("--g=%1").arg(m_drillDataInfo.groupID());

        // If pinmap exists, add it to URL
       /* if (ptTestCell->lPinmapIndex >= 0)
        {
            strLink += "." + QString::number(ptTestCell->lPinmapIndex);
        }*/
    }
    return strLink;
}

/******************************************************************************!
 * \fn OnInteractiveCharts
 * \brief Switch to the Interactive Charts display
 ******************************************************************************/
void DrillDataMining3D::OnInteractiveCharts()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: on interactive charts...");

    pGexMainWindow->Wizard_DrillChart(BuildInteractiveLinkChart(), false);
}

/******************************************************************************!
 * \fn closeEvent
 * \brief Close signal: If window is detached, this reparent it instead
 ******************************************************************************/
void DrillDataMining3D::closeEvent(QCloseEvent* e)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: close event...");
#ifndef __unix__
    if (pGexMainWindow != NULL)
    {
        e->accept();

        mWizardHandler->RemoveWizardWaferMap3D(this);
    }
#else
    e->accept();
#endif
}

void DrillDataMining3D::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
        case Qt::Key_Escape:
            e->accept();
            break;
        case Qt::Key_Left:
        case Qt::Key_PageUp:
            m_drillDataInfo.previousTestCell();
            break;
        case Qt::Key_Right:
        case Qt::Key_PageDown:
            m_drillDataInfo.nextTestCell();
            break;
        case Qt::Key_T:
        case Qt::Key_W:
        case Qt::Key_D:
        {
            if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
            {
                if(QApplication::keyboardModifiers() && Qt::ControlModifier)
                {
                    if(e->key() == Qt::Key_T)
                    {
                        pGexMainWindow->OnNewWizard(GEX_TABLE_WIZARD_P1);
                    }
                    else if(e->key() == Qt::Key_W)
                    {
                        pGexMainWindow->OnNewWizard(GEX_DRILL_3D_WIZARD_P1,BuildInteractiveLinkWafermap());
                    }
                    else if(e->key() == Qt::Key_D)
                    {
                        pGexMainWindow->OnNewWizard(GEX_CHART_WIZARD_P1, BuildInteractiveLinkChart());
                    }
                }
            }
        }
        default:
        {
            e->ignore();
            QDialog::keyPressEvent(e);
            break;
        }
    }
}

/******************************************************************************!
 * \fn ShowChart
 * \brief Draw 3D chart into Frame (caller is Web link)
 ******************************************************************************/
void DrillDataMining3D::ShowChart(QString strLink)
{

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: show chart...");

    // Enable/disable some features...
    if (GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m = ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    if (strLink.isNull() || strLink.isEmpty())
    {
        ShowChart(m_drillDataInfo.groupID(),m_drillDataInfo.fileID(),
                  m_drillDataInfo.testNumber(),m_drillDataInfo.pinmap(),
                  GEX_DRILL_WAFERPROBE);
        return;
    }

    // Parse the link
    parseLink(strLink);

    //-- no datas to draw the chart (all fail parts for exempl), just return
    if(m_drillDataInfo.currentTestCell() == NULL)
        return;

    QString local_prefix = prefix;
    QString suffix = m_drillDataInfo.currentTestCell()->GetTestNumber();

    pGexMainWindow->ChangeWizardTabName(this, local_prefix, suffix );
}

/******************************************************************************!
 * \fn ShowChart
 * \brief Draw 3D chart into Frame
 ******************************************************************************/
void
DrillDataMining3D::ShowChart(int nGroupID,
                             int nFileID,
                             int nTestNumber,
                             int nPinmap,
                             int nDrillType)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Show chart for group %1 file %2 test %3...")
          .arg(nGroupID).arg( nFileID).arg( nTestNumber).toLatin1().constData());

    // Enable/disable some features...
    if (GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m = ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }
    if (nFileID == -1)
    {
        return;
    }

    pGexMainWindow->pWizardSettings->iDrillType = nDrillType;

    // This create a temp copy not good nor efficient.
    // todo : use Reset, changeFileID & changeGroupID instead ?
    m_drillDataInfo = DrillData3DInfo(nGroupID, nFileID);

    m_drillDataInfo.loadTestCell(nTestNumber, nPinmap);

    QString local_prefix = prefix;
    if( m_drillDataInfo.currentTestCell())
    {
        QString suffix = m_drillDataInfo.currentTestCell()->GetTestNumber();

        pGexMainWindow->ChangeWizardTabName(this, local_prefix, suffix);
    }
}

/******************************************************************************!
 * \fn parseLink
 * \brief Parse an external link. Return true if the wafer map must be filled
 ******************************************************************************/
void DrillDataMining3D::parseLink(QString strLink)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("DrillDataMining3D : parse Link '%1'").arg(strLink).toLatin1().constData());

    int nGroupID = 0;
    int nFileID  = 0;
    int nPinmap  = 0;
    int iDieX    = -32768;
    int iDieY    = -32768;
    unsigned int nTestNumber = 0;


    // Reset mode
    m_eMode = DrillDataMining3D::standard;

    // Extract what to drill into.
    // line is like:
    QString strString    = strLink.section("--", 1, 1);  // = "drill_3d=<type>"
    QString strField     = strString.section('=', 0, 0);  // = "drill_3d"
    QString strChartType = strString.section('=', 1, 1);  // "wafer", etc...

    // Extract Group# and file#.
    strString = strLink.section("--g=", 1);  // = "g=<group#>
    strField  = strString.section("--", 0, 0);  // = group# (0= 1st group, etc.)
    nGroupID  = strField.toLong();

    strString = strLink.section("--f=", 1);  // = "f=<file#>
    strField  = strString.section("--", 0, 0);  // = file# (0=1st file, etc...)
    nFileID   = strField.toLong();

    strString   = strLink.section("--Test=", 1); // = "Test=<test#>
    strField    = strString.section("--", 0, 0); //
    nTestNumber = strField.toLong();

    strString = strLink.section("--Pinmap=", 1);  // = "Pinmap=<pinmap#>
    strField  = strString.section("--", 0, 0);  //
    nPinmap   = strField.toLong();

    strString = strLink.section("--DieX=", 1);  // = "DieX=<DieX position>
    strField  = strString.section("--", 0, 0);  //
    if (! strField.isEmpty())
    {
        iDieX = strField.toLong();
    }

    strString = strLink.section("--DieY=", 1);  // = "DieY=<DieY position>
    strField  = strString.section("--", 0, 0);  //
    if (! strField.isEmpty())
    {
        iDieY = strField.toLong();
    }

#ifdef GCORE15334
    CParametricWaferMapNNR cNNR;

    // Optional arguments, used in NRR
    strString = strLink.section("--AlgoType=", 1);  // = "AlgoType=<AlgoNumber>
    strField  = strString.section("--", 0, 0);  //
    if (! strField.isEmpty())
    {
        cNNR.iAlgorithm = strField.toLong();
    }

    //
    strString = strLink.section("--NFactor=", 1);  // = "NFactor=<float>
    strField  = strString.section("--", 0, 0);  //
    if (! strField.isEmpty())
    {
        cNNR.lfN_Factor = strField.toDouble();
    }
    //
    strString = strLink.section("--Size=", 1);  // = "Size=<MatrixSize>
    strField  = strString.section("--", 0, 0);  //
    if (! strField.isEmpty())
    {
        cNNR.iMatrixSize = strField.toLong();
    }
    //
    strString = strLink.section("--LA=", 1);  // = "LA=<LocationAveraging>
    strField  = strString.section("--", 0, 0);  //
    if (! strField.isEmpty())
    {
        cNNR.lf_LA = strField.toLong();
    }
#endif
    // Special short-cuts for SoftBin, HardBin wafermaps!
    if (strChartType == "wafer_sbin")
    {
        // Force to Soft Bin analysis
        pGexMainWindow->pWizardSettings->iDrillType = GEX_DRILL_WAFERPROBE;

        nTestNumber = GEX_TESTNBR_OFFSET_EXT_SBIN;
        nPinmap     = GEX_PTEST;
    }
    else if (strChartType == "wafer_hbin")
    {
        // Force to Soft Bin analysis
        pGexMainWindow->pWizardSettings->iDrillType = GEX_DRILL_WAFERPROBE;

        nTestNumber = GEX_TESTNBR_OFFSET_EXT_HBIN;
        nPinmap     = GEX_PTEST;
    }
    else if (strChartType == "wafer_param_limits")
    {
        // Force to Parametric Wafermap (over limits)
        pGexMainWindow->pWizardSettings->iDrillType = GEX_DRILL_WAFERPROBE;
        m_nSpectrumColors = GEX3D_SPECTRUM_TESTLIMITS;
    }
    else if (strChartType == "wafer_param_range")
    {
        // Force to Parametric Wafermap (over range)
        pGexMainWindow->pWizardSettings->iDrillType = GEX_DRILL_WAFERPROBE;
        m_nSpectrumColors = GEX3D_SPECTRUM_DATARANGE;
    }
#ifdef GCORE15334

    else if (strChartType == "wafer_param_range_nnr")
    {
        // Force to Parametric Wafermap (over range)
        pGexMainWindow->pWizardSettings->iDrillType = GEX_DRILL_WAFERPROBE;
        m_nSpectrumColors = GEX3D_SPECTRUM_DATARANGE;

        // We create the NNR test from the original test
        if (pGexMainWindow->mWizardWSPAT)
        {
            pGexMainWindow->mWizardWSPAT->
                    CreateTestNRR(nGroupID,
                                  nFileID,
                                  nTestNumber,
                                  nPinmap,
                                  &cNNR);
        }
    }
#endif
    else if (strChartType == "wafer_nnr")
    {
        // Force to Parametric Wafermap (over range)
        pGexMainWindow->pWizardSettings->iDrillType = GEX_DRILL_WAFERPROBE;
        m_nSpectrumColors = GEX3D_SPECTRUM_DATARANGE;
        m_eMode = DrillDataMining3D::nnr;
    }
    else if (strChartType == "wafer_test_passfail")
    {
        // Force to Parametric/Functional Wafermap (pass/fail)
        pGexMainWindow->pWizardSettings->iDrillType = GEX_DRILL_WAFERPROBE;
        m_nSpectrumColors = GEX3D_SPECTRUM_PASSFAIL;
    }
    else if (strChartType != "wafer")
    {
        nGroupID    = nFileID = 0;
        nTestNumber = nPinmap = -1;
    }

    // Source of bug : update m_drillDataInfo before loading any tests data...
    // Reupdate group & file
    GSLOG(SYSLOG_SEV_DEBUG, "Reseting drillDataInfo...");
    m_drillDataInfo.Reset();
    // todo : avoid dangerous copy operator
    //m_drillDataInfo = DrillData3DInfo(nGroupID, nFileID);
    // always set the group first then the file
    m_drillDataInfo.setTestNumber(nTestNumber);
    m_drillDataInfo.changeGroupID(nGroupID);
    m_drillDataInfo.changeFileID(nFileID);

     //-- no datas to draw the chart (all fail parts for exempl), just return
    if( m_drillDataInfo.currentTestCell() ==  NULL)
        return;

    QString local_prefix = prefix;
    QString suffix = m_drillDataInfo.currentTestCell()->GetTestNumber();

    pGexMainWindow->ChangeWizardTabName(this, local_prefix, suffix);

    // Do not start with Generic Galaxy Parameters
    {
        QString strOptionStorageDevice =
                ReportOptions.GetOption("statistics",
                                        "generic_galaxy_tests").toString();
        // GEX_ASSERT((strOptionStorageDevice=="hide") ||
        //   (strOptionStorageDevice=="show") );
        if ((strOptionStorageDevice == "hide") &&
                (nTestNumber >= GEX_TESTNBR_OFFSET_EXT_MIN) &&
                (nTestNumber <= GEX_TESTNBR_OFFSET_EXT_MAX))
        {
            nTestNumber = 0;
            // Bug : loadDefaultTestCell will call ShowGlobalProp
            // which needs correct group anf files.
            // Solution : update m_drillDataInfo before calling that code
            m_drillDataInfo.loadDefaultTestCell();
            CTest* pFirstValid = m_drillDataInfo.currentTestCell();
            if (pFirstValid)
            {
                nTestNumber = pFirstValid->lTestNumber;
            }
        }
    }

    // Do it upper ?
    m_drillDataInfo.setDieCoord(iDieX, iDieY);
    m_drillDataInfo.loadTestCell(nTestNumber, nPinmap);
}

/******************************************************************************!
 * \fn fillComboWafermap
 * \brief Load the list of wafer to display
 ******************************************************************************/
void DrillDataMining3D::fillComboWafermap(int nGroupID)
{
    comboBoxWafermap->clear();

    CGexGroupOfFiles* pGroup = NULL;
    CGexFileInGroup*  pFile  = NULL;

    if (nGroupID >= 0 && nGroupID < gexReport->getGroupsList().count())
    {
        pGroup = gexReport->getGroupsList().at(nGroupID);
    }

    if (pGroup)
    {
        QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

        while (itFilesList.hasNext())
        {
            pFile = itFilesList.next();

            // A wafermap is available for this file, add it to the list
            if (pFile->getWaferMapData().getWafMap() != NULL)
            {
                QString strLabel;

                // Lot ID not empty
                if (*pFile->getMirDatas().szLot)
                {
                    strLabel = pFile->getMirDatas().szLot;
                }
                else
                {
                    strLabel = "??";
                }

                strLabel += " / ";
                strLabel += pFile->getWaferMapData().szWaferID;

                // add the wafer
                comboBoxWafermap->addItem(strLabel);
            }
        }
    }

    if (comboBoxWafermap->count() > 1)
    {
        comboBoxWafermap->show();
    }
    else
    {
        comboBoxWafermap->hide();
    }
}

/******************************************************************************!
 * \fn fillViewer
 * \brief Fill the viewer 3D. Fill the wafermap if need be
 ******************************************************************************/
void DrillDataMining3D::fillViewer(bool bReset  /* = false */)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: fill viewer...");

    // Fill wafermap
    if (m_drillDataInfo.currentTestCell())
    {
        gexReport->
                FillWaferMap(m_drillDataInfo.groupID(),
                             m_drillDataInfo.fileID(),
                             m_drillDataInfo.currentTestCell(),
                             pGexMainWindow->pWizardSettings->iDrillSubType,
                             false,
                             m_pScaleColorWidget->spectrumColorLowValue(),
                             m_pScaleColorWidget->spectrumColorHighValue());
    }

    updateGui();

    // Load GEX data read from STDF files into Landscape Array...so to fit
    // OpenGL format needs
#ifndef DRILL_NO_DATA_MINING
    // Reset variables
    landscapeObjects = 0;

    // Reload data points array
    LoadLandscapeArray();

    // Build the OpenGL list of vectors to be painted
    refreshViewer(bReset);
#endif
}

/******************************************************************************!
 * \fn updateData
 * \brief Update drill subtype
 ******************************************************************************/
void DrillDataMining3D::updateData()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Update Data for DrillDataMining3D...");
    if (m_drillDataInfo.testNumber() == GEX_TESTNBR_OFFSET_EXT_SBIN)
    {
        pGexMainWindow->pWizardSettings->iDrillSubType = GEX_WAFMAP_SOFTBIN;
        m_pScaleColorWidget->binScaleColor->setBinType(BinScaleColorWidget::typeSoftBin);
        m_pScaleColorWidget->binScaleColor->setGroup(m_drillDataInfo.group());
        m_pScaleColorWidget->binScaleColor->setFile(m_drillDataInfo.file());
        m_pScaleColorWidget->setCurrentPage(GexScaleColorWidget::scalePageBinning);
    }
    else if (m_drillDataInfo.testNumber() == GEX_TESTNBR_OFFSET_EXT_HBIN)
    {
        pGexMainWindow->pWizardSettings->iDrillSubType = GEX_WAFMAP_HARDBIN;
        m_pScaleColorWidget->binScaleColor->setBinType(BinScaleColorWidget::typeHardBin);
        m_pScaleColorWidget->binScaleColor->setGroup(m_drillDataInfo.group());
        m_pScaleColorWidget->binScaleColor->setFile(m_drillDataInfo.file());
        m_pScaleColorWidget->setCurrentPage(GexScaleColorWidget::scalePageBinning);
    }
    else if (m_drillDataInfo.isFakeTest())
    {
        double dLowerBound;
        double dUpperBound;

        m_pScaleColorWidget->
                setCurrentPage(GexScaleColorWidget::scalePageParametric);
        m_pScaleColorWidget->parametricScaleColor->
                setTestCell(m_drillDataInfo.currentTestCell());

        pGexMainWindow->pWizardSettings->iDrillSubType = GEX_WAFMAP_TESTOVERDATA;

        dUpperBound = m_drillDataInfo.currentTestCell()->lfSamplesMax;
        dLowerBound = m_drillDataInfo.currentTestCell()->lfSamplesMin;

        m_pScaleColorWidget->setColorSpectrumBounds(dLowerBound, dUpperBound);
    }
    else
    {
        if (m_nSpectrumColors == GEX3D_SPECTRUM_PASSFAIL)
        {
            pGexMainWindow->pWizardSettings->iDrillSubType =
                    GEX_WAFMAP_TEST_PASSFAIL;

            m_pScaleColorWidget->parametricPassFailScaleColor->
                    setFile(m_drillDataInfo.file());
            m_pScaleColorWidget->
                    setCurrentPage(GexScaleColorWidget::scalePageParametricPassFail);
        }
        else
        {
            double dLowerBound;
            double dUpperBound;

            m_pScaleColorWidget->
                    setCurrentPage(GexScaleColorWidget::scalePageParametric);
            m_pScaleColorWidget->parametricScaleColor->
                    setTestCell(m_drillDataInfo.currentTestCell());

            switch (m_nSpectrumColors)
            {
            case GEX3D_SPECTRUM_DATARANGE:
                pGexMainWindow->pWizardSettings->iDrillSubType =
                        GEX_WAFMAP_TESTOVERDATA;
                dUpperBound = m_drillDataInfo.currentTestCell()->lfSamplesMax;
                dLowerBound = m_drillDataInfo.currentTestCell()->lfSamplesMin;
                break;

            case GEX3D_SPECTRUM_TESTLIMITS:
                pGexMainWindow->pWizardSettings->iDrillSubType =
                        GEX_WAFMAP_TESTOVERLIMITS;

                if ((m_drillDataInfo.currentTestCell()->GetCurrentLimitItem()->bLimitFlag &
                     CTEST_LIMITFLG_NOHTL) == 0)
                {
                    dUpperBound = m_drillDataInfo.currentTestCell()->GetCurrentLimitItem()->lfHighLimit;
                }
                else
                {
                    dUpperBound = m_drillDataInfo.currentTestCell()->lfMax;
                }

                if ((m_drillDataInfo.currentTestCell()->GetCurrentLimitItem()->bLimitFlag &
                     CTEST_LIMITFLG_NOLTL) == 0)
                {
                    dLowerBound = m_drillDataInfo.currentTestCell()->GetCurrentLimitItem()->lfLowLimit;
                }
                else
                {
                    dLowerBound = m_drillDataInfo.currentTestCell()->lfMin;
                }
                break;

            default:
                GSLOG(SYSLOG_SEV_NOTICE, "Invalid Spectrum color");
                GEX_ASSERT(false);
                pGexMainWindow->pWizardSettings->iDrillSubType =
                        GEX_WAFMAP_TESTOVERDATA;
                dUpperBound = m_drillDataInfo.currentTestCell()->lfSamplesMax;
                dLowerBound = m_drillDataInfo.currentTestCell()->lfSamplesMin;
                break;

            }

            m_pScaleColorWidget->setColorSpectrumBounds(dLowerBound, dUpperBound);
        }
    }
}

/******************************************************************************!
 * \fn onDataLoaded
 * \brief New data has been loaded
 ******************************************************************************/
void DrillDataMining3D::onDataLoaded(bool bSuccess, bool bFirst)
{
    GSLOG(SYSLOG_SEV_DEBUG, "onDataLoaded...");
    // Reset interactive test filter if test loaded
    // doesn't with the filter parameter
    if (gexReport->
            isInteractiveTestFiltered(m_drillDataInfo.currentTestCell()) == false)
    {
        gexReport->clearInteractiveTestFilter();
    }

    // reset selection when test has changed
    resetSelection();

    if (bSuccess)
    {
        // updates data
        updateData();
    }

    // fill the viewer with new values
    fillViewer(bFirst);

    // Select die location if valid coord
    if (m_drillDataInfo.hasValidDieCoord())
    {
        SelectDieLocation(m_drillDataInfo.file(),
                          m_drillDataInfo.dieX(), m_drillDataInfo.dieY(), true);
    }
}

/******************************************************************************!
 * \fn onTestNavigate
 * \brief Navigates among the test list
 ******************************************************************************/
void DrillDataMining3D::onTestNavigate()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: on test navigate...");

    // Go back to standard mode since we move to another test
    m_eMode = DrillDataMining3D::standard;

    // reset selection when test has changed
    resetSelection();

    // updates data
    updateData();

    // fill the viewer with new values
    fillViewer();

    QString local_prefix = prefix;
    QString suffix = m_drillDataInfo.currentTestCell()->GetTestNumber();

    pGexMainWindow->ChangeWizardTabName(this, local_prefix, suffix);
}

/******************************************************************************!
 * \fn isDataAvailable
 * \brief Tells if data available for charting
 ******************************************************************************/
bool DrillDataMining3D::isDataAvailable()
{
    CGexGroupOfFiles* pGroup = 0;
    CGexFileInGroup*  pFile  = 0;

    if (gexReport == NULL)
    {
        return false;
    }

    // Get pointer to first group & first file
    pGroup = gexReport->getGroupsList().size() > 0 ?
                gexReport->getGroupsList().first() : NULL;
    if (pGroup == NULL)
    {
        return false;
    }

    if (pGroup->pFilesList.count() <= 0)
    {
        return false;
    }

    pFile = (pGroup->pFilesList.isEmpty()) ? NULL :
                                             (pGroup->pFilesList.first());
    if (pFile == NULL)
    {
        return false;
    }

    if (pFile->ptTestList == NULL)
    {
        return false;
    }

    // Some data available.
    return true;
}

/******************************************************************************!
 * \fn showWindows
 * \brief Show dialog box and child windows (OpenGL)
 ******************************************************************************/
void DrillDataMining3D::showWindows()
{
    // restore window if it is minimized
    if (isMinimized())
    {
        setWindowState(windowState() & ~Qt::WindowMinimized);
    }
    else
    {
        show();
    }

}

/******************************************************************************!
 * \fn hideWindows
 * \brief Hide dialog box and child windows (OpenGL)
 ******************************************************************************/
void DrillDataMining3D::hideWindows()
{
    hide();
}

#ifndef DRILL_NO_DATA_MINING
/******************************************************************************!
 * \fn SetPlotColor
 * \brief Map color to value
 *
 * Color mapping is as following (RGB normalized))
 * Color = 0     25    50    75    100
 *  R    = 0     0     0   ..255   255
 *  G    = 0   ..255   255   255 ..0
 *  B    = 255   255 ..0     0     0
 ******************************************************************************/
void DrillDataMining3D::SetPlotColor(float         fColor,
                                     const QColor& rgbColor  /*=QColor()*/)
{
    float fRed, fGreen, fBlue;

    // If RGB color specified, simply use it!
    if (rgbColor.isValid())
    {
        // Get colors, normalize to a number in [0..1]
        fRed   = rgbColor.red()   / 255.0;
        fBlue  = rgbColor.blue()  / 255.0;
        fGreen = rgbColor.green() / 255.0;
    }
    else
    {
        if (fColor == GEX3D_SELECT_COLOR)
        {
            // White
            glColor3f(1.0, 1.0, 1.0);
            return;
        }
        else if (fColor == GEX3D_NNR_COLOR)
        {
            // White
            fRed   = 100.0 / 255.0;
            fBlue  = 100.0 / 255.0;
            fGreen = 100.0 / 255.0;
        }
        else if (fColor == GEX3D_PASS_COLOR)
        {
            // Green
            fRed   = 134.0 / 255.0;
            fGreen = 242.0 / 255.0;
            fBlue  = 134.0 / 255.0;
        }
        else if (fColor == GEX3D_FAIL_COLOR)
        {
            // Red
            fRed   = 252.0 / 255.0;
            fGreen = 90.0 / 255.0;
            fBlue  = 90.0 / 255.0;
        }
        else if (fColor <= 25.0)
        {
            fRed   = 0;
            fBlue  = 1.0;
            fGreen = fColor / 25.0;
        }
        else if (fColor <= 50.0)
        {
            fRed   = 0.0;
            fGreen = 1.0;
            fBlue  = (50.0 - fColor) / 25.0;
        }
        else if (fColor <= 75.0)
        {
            fGreen = 1.0;
            fBlue  = 0.0;
            fRed   = (fColor - 50.0) / 25.0;
        }
        else
        {
            fRed   = 1.0;
            fBlue  = 0;
            fGreen = (100.0 - fColor) / 25.0;
        }
    }

    glColor3f(0.4 * fRed, 0.4 * fGreen, 0.4 * fBlue);
}

/******************************************************************************!
 * \fn SetPlotColorOutline
 * \brief Map color to value (outline...so make it a little darker)
 ******************************************************************************/
void
DrillDataMining3D::SetPlotColorOutline(float         fColor,
                                       const QColor& rgbColor  /*= QColor()*/,
                                       bool          bRetested  /*= false*/)
{
    float fRed, fGreen, fBlue;

    if (bRetested)
    {
        fRed   = 0;
        fBlue  = 0;
        fGreen = 0;
    }
    // If RGB color specified, simply use it!
    else if (rgbColor.isValid())
    {
        // Get colors, normalize to a number in [0..1]
        fRed   = rgbColor.red()   / 255.0;
        fBlue  = rgbColor.blue()  / 255.0;
        fGreen = rgbColor.green() / 255.0;
    }
    else
    {
        if (fColor == GEX3D_SELECT_COLOR)
        {
            // White
            glColor3f(0.7, 0.7, 0.7);
            return;
        }
        else if (fColor == GEX3D_NNR_COLOR)
        {
            fRed   = 100.0 / 255.0;
            fBlue  = 100.0 / 255.0;
            fGreen = 100.0 / 255.0;
        }
        else if (fColor == GEX3D_PASS_COLOR)
        {
            // Green
            fRed   = 134.0 / 255.0;
            fGreen = 242.0 / 255.0;
            fBlue  = 134.0 / 255.0;
        }
        else if (fColor == GEX3D_FAIL_COLOR)
        {
            // Red
            fRed   = 252.0 / 255.0;
            fBlue  = 90.0 / 255.0;
            fGreen = 90.0 / 255.0;
        }
        else if (fColor <= 25.0)
        {
            fRed   = 0;
            fBlue  = 1.0;
            fGreen = fColor / 25.0;
        }
        else if (fColor <= 50.0)
        {
            fRed   = 0.0;
            fGreen = 1.0;
            fBlue  = (50.0 - fColor) / 25.0;
        }
        else if (fColor <= 75.0)
        {
            fGreen = 1.0;
            fBlue  = 0.0;
            fRed   = (fColor - 50.0) / 25.0;
        }
        else
        {
            fRed   = 1.0;
            fBlue  = 0;
            fGreen = (100.0 - fColor) / 25.0;
        }
    }

    glColor3f(0.3 * fRed, 0.3 * fGreen, 0.3 * fBlue);
}

int hFont[1];

void DrillDataMining3D::InitOpenGLFont()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: init openGL font...");

    QString strString = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();

    // Reset 3D Font library
    m_pclFont3D->glfInit();

    // Load default font
    // strString += "/fonts_3d/times_new1.glf";
    strString += "/fonts_3d/arial1.glf";
    hFont[0]   = m_pclFont3D->glfLoadFont(strString.toLatin1().constData());
}

/******************************************************************************!
 * \fn renderText
 * \brief Display text at given location
 ******************************************************************************/
void
DrillDataMining3D::renderText(const Vec&  cOrigin,
                              int         iDirection,
                              bool        bFill,
                              float       fHeight,
                              const char* szString)
{
    glPushMatrix();
    glTranslatef(cOrigin.x, cOrigin.y, cOrigin.z);

    float fMinX, fMinY, fMaxX, fMaxY, fRatio;
    m_pclFont3D->
            glfGetStringBoundsF(hFont[0],
            szString,
            &fMinX,
            &fMinY,
            &fMaxX,
            &fMaxY);
    // Compute scaling factor so string has the required Heigth
    fRatio = fHeight / fabs(fMaxY - fMinY);
    glScalef(fRatio, fRatio, fRatio);
    // Default Text writing is along X axis, Vertical is Y...
    // Do necessary rotates if showing a WaferMap.
    if (pGexMainWindow->pWizardSettings->iDrillType == GEX_DRILL_WAFERPROBE)
    {
        glRotatef(180, 0, 0, 1);
    }
    switch (iDirection)
    {
    case XRight:
        glRotatef(90, 1, 0, 0);
        break;
    case XLeft:
        break;
    case YRight:
        break;
    case YLeft:
        break;
    case ZRight:
        glRotatef(90, 1, 0, 0);
        glRotatef(90, 0, 0, 1);
        break;
    case ZLeft:
        break;
    }

    if (bFill == true)
    {
        m_pclFont3D->glfDraw3DSolidStringF(hFont[0], szString);
    }
    else
    {
        m_pclFont3D->glfDraw3DWiredStringF(hFont[0], szString);
    }
    glPopMatrix();
}

/******************************************************************************!
 * \fn CreatePropertiesFile
 * \brief Create HTML Properties page header
 ******************************************************************************/
void DrillDataMining3D::CreatePropertiesFile(QString strTitle)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: create properties file...");

    if (gexReport == NULL)
    {
        return;
    }

    // Create HTML page
    if (hProperties != NULL)
    {
        fclose(hProperties);
    }
    hProperties = fopen(strPropertiesPath.toLatin1().constData(), "w");

    // Write header in propserties HTML page
    // (not a standard Examinator full HTML report)
    // Default: Text is Black
    gexReport->WriteHeaderHTML(hProperties, "#000000", "#FFFFFF", "", false);
    fprintf(hProperties,
            "<font color=\"#006699\"><b>%s</b><br>\n",
            strTitle.toLatin1().constData());
    fprintf(hProperties,
            "<table border=\"0\" width=\"98%%\" cellspacing=\"1\">\n");
}

/******************************************************************************!
 * \fn WriteInfoLine
 * \brief Write one line in HTML table
 ******************************************************************************/
void
DrillDataMining3D::WriteInfoLine(const char* szLabel,
                                 const char* szData,
                                 bool        bAlarmLabelColor,
                                 bool        bAlarmTextColor)
{

    if (szLabel == NULL)
    {
        // Display a separator line
        fprintf(hProperties, "<tr>\n");
        fprintf(hProperties,
                "<td width=\"80\"  bgcolor=%s><b><HR></b></td>\n",
                szFieldColor);
        fprintf(hProperties, "<td bgcolor=%s><HR></td>\n", szDataColor);
        fprintf(hProperties, "</tr>\n");
    }
    else
    {
        // Skip spaces if any
        while ((*szData == ' ') || (*szData == '\t'))
        {
            szData++;
        }
        if (*szData == 0)
        {
            return;  // Empty string !

        }
        fprintf(hProperties, "<tr>\n");

        // Display given parameters
        if (bAlarmLabelColor)
        {
            fprintf(hProperties,
                    "<td width=\"80\" bgcolor=%s>"
                    "<font color=\"#FF0000\"><b>%s</b></font></td>\n",
                    szFieldColor,
                    szLabel);
        }
        else
        {
            fprintf(hProperties,
                    "<td width=\"80\" bgcolor=%s><b>%s</b></td>\n",
                    szFieldColor,
                    szLabel);
        }

        if (bAlarmTextColor)
        {
            fprintf(hProperties,
                    "<td bgcolor=%s><font color=\"#FF0000\"><b>%s</b></font></td>\n",
                    szDataColor,
                    szData);
        }
        else
        {
            fprintf(hProperties, "<td bgcolor=%s>%s</td>\n", szDataColor, szData);
        }

        fprintf(hProperties, "</tr>\n");
    }

}

/******************************************************************************!
 * \fn WriteInfoLine
 * \brief Write one line in HTML table
 ******************************************************************************/
void
DrillDataMining3D::WriteInfoLine(const char* szLabel,
                                 QString     strData,
                                 bool        bAlarmLabelColor,
                                 bool        bAlarmTextColor)
{
    WriteInfoLine(szLabel,
                  strData.toLatin1().constData(), bAlarmLabelColor,
                  bAlarmTextColor);
}

/******************************************************************************!
 * \fn ClosePropertiesFile
 * \brief Close HTML Properties page
 ******************************************************************************/
void DrillDataMining3D::ClosePropertiesFile()
{
    fprintf(hProperties, "</table>\n");
    fprintf(hProperties, "</body>\n");
    fprintf(hProperties, "</html>\n");
    fclose(hProperties);
    hProperties = NULL;
}

/******************************************************************************!
 * \fn ShowGlobalProperties
 * \brief Show Global properties for the whole view
 ******************************************************************************/
void DrillDataMining3D::ShowGlobalProperties()
{
    if ((! m_drillDataInfo.group()) || (! m_drillDataInfo.file()))
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot show global properties : no group or no file");
        return;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("DrillDataMining3D Show Global Properties : group=%1 file=%2")
          .arg(m_drillDataInfo.group()->strGroupName.toLatin1().data())
          .arg(m_drillDataInfo.file() ? m_drillDataInfo.file()->lFileID : -1).toLatin1().constData());

    float fData;
    int   lTestDuration;
    char  szString[256];
    //QString strMessage;
    int nInvalidData = (int) 4294967295UL;

    if (m_drillDataInfo.isValid())
    {
        // Create/Open HTML page to include Neptus Selection's Info
        CreatePropertiesFile("Global Info");

        if (ReportOptions.iGroups > 1)
        {
            // crash sometimes here :
            WriteInfoLine("Group name",
                          m_drillDataInfo.group() ?
                              m_drillDataInfo.group()->strGroupName.toLatin1().constData()
                            : "N/A");
        }

        WriteInfoLine("File name",
                      m_drillDataInfo.file()->strFileName.toLatin1().constData());
        WriteInfoLine("Tests mapping file",
                      m_drillDataInfo.file()->strMapTests.toLatin1().constData());
        WriteInfoLine("Setup time",
                      TimeStringUTC(m_drillDataInfo.file()->getMirDatas().lSetupT));
        WriteInfoLine("Start time",
                      TimeStringUTC(m_drillDataInfo.file()->getMirDatas().lStartT));
        WriteInfoLine("End time",
                      TimeStringUTC(m_drillDataInfo.file()->getMirDatas().lEndT));

        // Checks if can compute test duration.
        if (m_drillDataInfo.file()->getMirDatas().lEndT <= 0 ||
                m_drillDataInfo.file()->getMirDatas().lStartT <= 0)
        {
            lTestDuration = -1;
        }
        else
        {
            lTestDuration = m_drillDataInfo.file()->getMirDatas().lEndT
                    - m_drillDataInfo.file()->getMirDatas().lStartT;
        }

        WriteInfoLine("Test duration",
                      gexReport->HMSString(lTestDuration, 0, szString));
        WriteInfoLine("Program", m_drillDataInfo.file()->getMirDatas().szJobName);
        WriteInfoLine("Revision", m_drillDataInfo.file()->getMirDatas().szJobRev);
        WriteInfoLine("Lot", m_drillDataInfo.file()->getMirDatas().szLot);
        WriteInfoLine("Sub-Lot", m_drillDataInfo.file()->getMirDatas().szSubLot);

        // Parts to process
        WriteInfoLine("Parts processed",
                      (m_drillDataInfo.file()->BuildPartsToProcess()).toLatin1().
                      data());

        // Testing on sites
        WriteInfoLine("Data from Sites", m_drillDataInfo.file()->
                      BuildSitesToProcess().toLatin1().data());

        // Device test time info (on GOOD PARTS).
        if (m_drillDataInfo.group()->cMergedData.lMergedTestTimeParts_Good != 0)
        {
            // Compute test time in seconds.
            double fValue =
                    (m_drillDataInfo.group()->cMergedData.lMergedAverageTestTime_Good /
                     m_drillDataInfo.group()->cMergedData.lMergedTestTimeParts_Good) /
                    1000.0;
            sprintf(szString, "%.3lf sec. (excludes tester idle time)", fValue);
        }
        else
        {
            sprintf(szString, GEX_NA);
        }

        WriteInfoLine("Test time (GOOD parts)", szString);

        // Device test time info (on ALL PARTS).
        if (m_drillDataInfo.group()->cMergedData.lMergedTestTimeParts_All != 0)
        {
            // Compute test time in seconds.
            double fValue =
                    (m_drillDataInfo.group()->cMergedData.lMergedAverageTestTime_All /
                     m_drillDataInfo.group()->cMergedData.lMergedTestTimeParts_All) /
                    1000.0;
            sprintf(szString, "%.3lf sec. (excludes tester idle time)", fValue);
        }
        else
        {
            sprintf(szString, GEX_NA);
        }

        WriteInfoLine("Test time (ALL parts)", szString);

        // Average testing time / device
        if ((m_drillDataInfo.file()->getPcrDatas().lPartCount) &&
                (m_drillDataInfo.file()->getPcrDatas().lPartCount != nInvalidData) &&
                (lTestDuration > 0))
        {
            fData =
                    (float) lTestDuration / m_drillDataInfo.file()->getPcrDatas().lPartCount;
            sprintf(szString,
                    "%.3f sec. / device (includes tester idle time between parts)",
                    fData);
        }
        else
        {
            sprintf(szString, GEX_NA);
        }

        WriteInfoLine("Average test time", szString);

        // Total parts tested
        if ((m_drillDataInfo.file()->getPcrDatas().lPartCount) &&
                (m_drillDataInfo.file()->getPcrDatas().lPartCount != nInvalidData))
        {
            sprintf(szString, "%ld", m_drillDataInfo.file()->getPcrDatas().lPartCount);
        }
        else
        {
            sprintf(szString, GEX_NA);
        }

        WriteInfoLine("Total parts tested", szString);

        // Good parts
        if ((m_drillDataInfo.file()->getPcrDatas().lPartCount) &&
                (m_drillDataInfo.file()->getPcrDatas().lPartCount != nInvalidData) &&
                (m_drillDataInfo.file()->getPcrDatas().lGoodCount > 0))
        {

            fData =
                    (float) (100.0 * m_drillDataInfo.file()->getPcrDatas().lGoodCount) /
                    m_drillDataInfo.file()->getPcrDatas().lPartCount;
            sprintf(szString, "%d (%.2f%%)",
                    m_drillDataInfo.file()->getPcrDatas().lGoodCount, fData);
        }
        else
        {
            sprintf(szString, GEX_NA);
        }

        WriteInfoLine("Good parts (Yield)", szString);

        // Bad parts
        if ((m_drillDataInfo.file()->getPcrDatas().lPartCount) &&
                (m_drillDataInfo.file()->getPcrDatas().lPartCount != nInvalidData)  &&
                (m_drillDataInfo.file()->getPcrDatas().lGoodCount > 0))
        {

            fData =
                    (float) (100.0 * (m_drillDataInfo.file()->getPcrDatas().lPartCount -
                                      m_drillDataInfo.file()->getPcrDatas().lGoodCount)) /
                    m_drillDataInfo.file()->getPcrDatas().lPartCount;
            sprintf(szString, "%ld (%.2f%%)",
                    m_drillDataInfo.file()->getPcrDatas().lPartCount -
                    m_drillDataInfo.file()->getPcrDatas().lGoodCount, fData);
        }
        else
        {
            sprintf(szString, GEX_NA);
        }

        WriteInfoLine("Bad parts (Yield loss)", szString);

        // Parts retested.
        if ((m_drillDataInfo.file()->getPcrDatas().lPartCount) &&
                (m_drillDataInfo.file()->getPcrDatas().lPartCount != nInvalidData) &&
                (m_drillDataInfo.file()->getPcrDatas().lRetestCount >= 0))
        {
            fData =
                    (float) (100.0 * m_drillDataInfo.file()->getPcrDatas().lRetestCount) /
                    m_drillDataInfo.file()->getPcrDatas().lPartCount;
            sprintf(szString, "%d (%.2f%%)",
                    m_drillDataInfo.file()->getPcrDatas().lRetestCount, fData);
        }
        else
        {
            sprintf(szString, GEX_NA);
        }

        WriteInfoLine("Parts retested", szString);

        // Parts aborted.
        if ((m_drillDataInfo.file()->getPcrDatas().lPartCount) &&
                (m_drillDataInfo.file()->getPcrDatas().lPartCount != nInvalidData) &&
                (m_drillDataInfo.file()->getPcrDatas().lAbortCount >= 0))
        {
            fData =
                    (float) (100.0 *  m_drillDataInfo.file()->getPcrDatas().lAbortCount) /
                    m_drillDataInfo.file()->getPcrDatas().lPartCount;
            sprintf(szString, "%d (%.2f%%)",
                    m_drillDataInfo.file()->getPcrDatas().lAbortCount, fData);
        }
        else
        {
            sprintf(szString, GEX_NA);
        }

        WriteInfoLine("Parts aborted", szString);

        // Functional parts tested
        if (m_drillDataInfo.file()->getPcrDatas().lFuncCount >= 0)
        {
            sprintf(szString, "%d", m_drillDataInfo.file()->getPcrDatas().lFuncCount);
        }
        else
        {
            strcpy(szString, GEX_NA);
        }

        if (m_drillDataInfo.file()->getPcrDatas().lFuncCount != nInvalidData)
        {
            WriteInfoLine("Functional parts tested", szString);
        }

        // Separator line using '-'
        WriteInfoLine(NULL, "-");

        if (m_drillDataInfo.file()->StdfRecordHeader.iStdfVersion == 3)
        {
            WriteInfoLine("STDF Version", "3.0");
        }
        else if (m_drillDataInfo.file()->StdfRecordHeader.iStdfVersion == 4)
        {
            WriteInfoLine("STDF Version", "4.0");
        }

        WriteInfoLine("Tester name", m_drillDataInfo.file()->getMirDatas().szNodeName);
        WriteInfoLine("Tester type", m_drillDataInfo.file()->getMirDatas().szTesterType);
        sprintf(szString, "%d", m_drillDataInfo.file()->getMirDatas().bStation & 0xff);
        WriteInfoLine("Station", szString);
        WriteInfoLine("Part type", m_drillDataInfo.file()->getMirDatas().szPartType);
        WriteInfoLine("Operator", m_drillDataInfo.file()->getMirDatas().szOperator);
        WriteInfoLine("Exec_type", m_drillDataInfo.file()->getMirDatas().szExecType);
        WriteInfoLine("Exec_version", m_drillDataInfo.file()->getMirDatas().szExecVer);
        WriteInfoLine("TestCode", m_drillDataInfo.file()->getMirDatas().szTestCode);
        WriteInfoLine("Test Temperature",
                      m_drillDataInfo.file()->getMirDatas().szTestTemperature);
        WriteInfoLine("User Text", m_drillDataInfo.file()->getMirDatas().szUserText);
        WriteInfoLine("Aux_file", m_drillDataInfo.file()->getMirDatas().szAuxFile);
        WriteInfoLine("Package type", m_drillDataInfo.file()->getMirDatas().szPkgType);
        WriteInfoLine("Per_freq", m_drillDataInfo.file()->getMirDatas().szperFrq);
        WriteInfoLine("Spec_name", m_drillDataInfo.file()->getMirDatas().szSpecName);
        WriteInfoLine("Spec_version",
                      m_drillDataInfo.file()->getMirDatas().szSpecVersion);
        WriteInfoLine("Family ID", m_drillDataInfo.file()->getMirDatas().szFamilyID);
        WriteInfoLine("Date code", m_drillDataInfo.file()->getMirDatas().szDateCode);
        WriteInfoLine("Design Rev", m_drillDataInfo.file()->getMirDatas().szDesignRev);
        WriteInfoLine("Facility ID", m_drillDataInfo.file()->getMirDatas().szFacilityID);
        WriteInfoLine("Floor ID", m_drillDataInfo.file()->getMirDatas().szFloorID);
        WriteInfoLine("Proc ID", m_drillDataInfo.file()->getMirDatas().szProcID);
        WriteInfoLine("Flow ID", m_drillDataInfo.file()->getMirDatas().szFlowID);
        WriteInfoLine("Setup ID", m_drillDataInfo.file()->getMirDatas().szSetupID);
        WriteInfoLine("Eng ID", m_drillDataInfo.file()->getMirDatas().szEngID);
        WriteInfoLine("ROM code", m_drillDataInfo.file()->getMirDatas().szROM_Code);
        WriteInfoLine("Serial #", m_drillDataInfo.file()->getMirDatas().szSerialNumber);
        WriteInfoLine("Super user name",
                      m_drillDataInfo.file()->getMirDatas().szSuprName);

        // Close HTML Neptus page
        ClosePropertiesFile();

        // Show properties
        m_pPropertiesBrowser->reload();
    }
}

/******************************************************************************!
 * \fn ShowDieProperties
 * \brief Show die properties (about the data view)
 ******************************************************************************/
void DrillDataMining3D::ShowDieProperties()
{
    QString strMapType;
    QString strTitle;

    // Get Map type
    if (m_drillDataInfo.file()->getWaferMapData().bStripMap)
    {
        strMapType = "Packaged";
    }
    else
    {
        strMapType = "Wafer";
    }

    switch (pGexMainWindow->pWizardSettings->iDrillType)
    {
    // Any Drill not listed hereafter
    default:
        strTitle = "Unknown";
        break;

    case GEX_DRILL_WAFERPROBE:
        switch (pGexMainWindow->pWizardSettings->iDrillSubType)
        {
        case GEX_WAFMAP_SOFTBIN:
        case GEX_WAFMAP_STACK_SOFTBIN:
            strTitle = "Software Bin - " + strMapType;
            break;

        case GEX_WAFMAP_HARDBIN:
        case GEX_WAFMAP_STACK_HARDBIN:
            strTitle = "Hardware Bin - " + strMapType;
            break;

        case GEX_WAFMAP_TESTOVERLIMITS:
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            strTitle = "Test over limits - " + strMapType;
            break;

        case GEX_WAFMAP_TESTOVERDATA:
        case GEX_WAFMAP_STACK_TESTOVERDATA:
            strTitle = "Test over values - " + strMapType;
            break;

        case GEX_WAFMAP_TEST_PASSFAIL:
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            strTitle = "Test pass/fail - " + strMapType;
            break;

        }
        break;

    case GEX_DRILL_PACKAGED:
        switch (pGexMainWindow->pWizardSettings->iDrillSubType)
        {
        case GEX_WAFMAP_SOFTBIN:
        case GEX_WAFMAP_STACK_SOFTBIN:
            strTitle = "Software Bin - Packaged";
            break;

        case GEX_WAFMAP_HARDBIN:
        case GEX_WAFMAP_STACK_HARDBIN:
            strTitle = "Hardware Bin - Packaged";
            break;

        case GEX_WAFMAP_TESTOVERLIMITS:
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            strTitle = "Test over limits - Packaged";
            break;

        case GEX_WAFMAP_TESTOVERDATA:
        case GEX_WAFMAP_STACK_TESTOVERDATA:
            strTitle = "Test over values - Packaged";
            break;

        case GEX_WAFMAP_TEST_PASSFAIL:
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            strTitle = "Test pass/fail - Packaged";
            break;
        }
        break;
    }

    // Create/Open HTML page to include Neptus Selection's Info
    CreatePropertiesFile(strTitle);

    int iDrillReport = pGexMainWindow->pWizardSettings->iDrillType;
    switch (iDrillReport)
    {
    // Any Drill not listed hereafter
    default:
        break;

    case GEX_DRILL_WAFERPROBE:
    case GEX_DRILL_PACKAGED:
        switch (pGexMainWindow->pWizardSettings->iDrillSubType)
        {
        case GEX_WAFMAP_SOFTBIN:
        case GEX_WAFMAP_STACK_SOFTBIN:
        case GEX_WAFMAP_HARDBIN:
        case GEX_WAFMAP_STACK_HARDBIN:
            ShowBinProperties(iDrillReport);
            break;

        case GEX_WAFMAP_TESTOVERLIMITS:
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
        case GEX_WAFMAP_TESTOVERDATA:
        case GEX_WAFMAP_STACK_TESTOVERDATA:
        case GEX_WAFMAP_TEST_PASSFAIL:
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            ShowTestProperties(iDrillReport);
            break;
        }
        break;
    }

    // Close HTML Neptus page
    ClosePropertiesFile();

    // Show Properties dialog box (Neptus)
    m_pPropertiesBrowser->reload();
}

/******************************************************************************!
 * \fn pGetPartInfo
 * \brief Finds PartInfo structure for a given DieX,DieY part
 ******************************************************************************/
CPartInfo* DrillDataMining3D::pGetPartInfo(int lObjectIndex)
{
    // Convert to landscape X,Y coordinates
    int iChipY = lObjectIndex / landscapeSize;
    int iChipX = lObjectIndex % landscapeSize;

    // Turn wafermap up side down + rotate to match the same view
    // as the PNG image created under Examinator HTML report.
    iChipX = landscapeSize - iChipX - 1;

    // Convert to WaferMap physical X,Y coordinates.
    iChipX += m_drillDataInfo.file()->getWaferMapData().iLowDieX;
    iChipY += m_drillDataInfo.file()->getWaferMapData().iLowDieY;

    QListIterator<CPartInfo*>
            lstIteratorPartInfo(m_drillDataInfo.file()->pPartInfoList);
    CPartInfo* ptrPartInfo = NULL;

    lstIteratorPartInfo.toFront();

    while (lstIteratorPartInfo.hasNext())
    {
        ptrPartInfo = lstIteratorPartInfo.next();

        if ((ptrPartInfo->iDieX == iChipX) && (ptrPartInfo->iDieY == iChipY))
        {
            return ptrPartInfo;
        }
    }

    // Didn't find this part!
    return NULL;
}

/******************************************************************************!
 * \fn SelectDieLocation
 * \brief Activate selection over object at location X,Y
 ******************************************************************************/
void
DrillDataMining3D::SelectDieLocation(CGexFileInGroup* pFile,
                                     int              iDieX,
                                     int              iDieY,
                                     bool             bClearSelections)
{
    // Clear all other selections if requested.
    if (bClearSelections)
    {
        m_lstSelections.clear();
    }

    // Convert to landscape X,Y coordinates
    iDieX = iDieX - pFile->getWaferMapData().iLowDieX;
    iDieY = iDieY - pFile->getWaferMapData().iLowDieY;

    // Turn wafermap according X and Y pos direction
    if (m_drillDataInfo.file()->getWaferMapData().GetPosXDirection() == false)
    {
        iDieX = landscapeSize - iDieX - 1 -
                (landscapeSize - m_drillDataInfo.file()->getWaferMapData().SizeX);
    }

    if (m_drillDataInfo.file()->getWaferMapData().GetPosYDirection() == true)
    {
        iDieY = landscapeSize - iDieY - 1 -
                (landscapeSize - m_drillDataInfo.file()->getWaferMapData().SizeY);
    }

    // Calculate the die index in OpenGL object list
    int nIndex = iDieX + landscapeSize * iDieY;
    DrillObjectCell dieObject;

    // Search the run associated with the die
    if (findSelectedDie(nIndex, dieObject))
    {
        m_lstSelections.append(dieObject);

        // Redraw the wafer map
        m_pViewer->updateOverlayGL();
        m_pViewer->updateGL();

        // Show die properties
        OnProperties();
    }
}

/******************************************************************************!
 * \fn WriteTestStatisticsDetails
 * \brief Write Test statistics info into HTML page
 ******************************************************************************/
void DrillDataMining3D::WriteTestStatisticsDetails(CTest*           ptTestCell,
                                                   CGexFileInGroup* pFile)
{
    if (ptTestCell == NULL)
    {
        return;
    }

    WriteInfoLine("Statistics", "........");
    WriteInfoLine("Test#", ptTestCell->szTestLabel);
    WriteInfoLine("Name", ptTestCell->strTestName.toLatin1().constData());
    WriteInfoLine("Low L.", ptTestCell->GetCurrentLimitItem()->szLowL);
    WriteInfoLine("High L.", ptTestCell->GetCurrentLimitItem()->szHighL);
    WriteInfoLine("Execs.", gexReport->CreateResultString(ptTestCell->ldExecs));
    WriteInfoLine("Fails",
                  gexReport->CreateResultString(ptTestCell->GetCurrentLimitItem()->ldFailCount));
    WriteInfoLine("Removed",
                  gexReport->CreateResultString(ptTestCell->GetCurrentLimitItem()->ldOutliers));
    WriteInfoLine("Mean",
                  pFile->FormatTestResult(ptTestCell, ptTestCell->lfMean,
                                          ptTestCell->res_scal));
    WriteInfoLine("Sigma",
                  pFile->FormatTestResult(ptTestCell, ptTestCell->lfSigma,
                                          ptTestCell->res_scal));
    WriteInfoLine("Min",
                  pFile->FormatTestResult(ptTestCell, ptTestCell->lfMin,
                                          ptTestCell->res_scal));
    WriteInfoLine("Max",
                  pFile->FormatTestResult(ptTestCell, ptTestCell->lfMax,
                                          ptTestCell->res_scal));
    WriteInfoLine("Range",
                  pFile->FormatTestResult(ptTestCell, ptTestCell->lfRange,
                                          ptTestCell->res_scal));
    WriteInfoLine("Cp", gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));
    WriteInfoLine("Cpk", gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));
}

void DrillDataMining3D::OnTabRenamed( QWidget *widget, const QString &text )
{
  // base class valid conversion, allow pointer comparison as each slot is
  // triggered while tab is renamed
  QWidget *that = static_cast< QWidget * >( this );

  if( that == widget )
  {
    setTitle( text );
    setCustomTitleFlag();
  }
}

/******************************************************************************!
 * \fn OnProperties
 * \brief ToolBar button pressed
 ******************************************************************************/
void DrillDataMining3D::OnProperties()
{
    GSLOG(SYSLOG_SEV_DEBUG, "DrillDataMining3D On Properties...");

    // If No data to show, but only the title...show global properties!
    if (landscapeSize == 0)
    {
        ShowGlobalProperties();
        return;
    }

    // If no selection done, then show global properties.
    if (m_lstSelections.isEmpty() || m_lstSelections.count() > 1)
    {
        ShowGlobalProperties();
        m_pScaleColorWidget->parametricScaleColor->setSelection(GEX_C_DOUBLE_NAN);
        m_pScaleColorWidget->parametricScaleColor->updateScale();
        return;
    }

    double lfValue = GEX_C_DOUBLE_NAN;
    int    nSelectionFirstRun = m_lstSelections.first().run();
    if (m_drillDataInfo.currentTestCell()->
            m_testResult.isValidIndex(nSelectionFirstRun))
    {
        lfValue = m_drillDataInfo.currentTestCell()->
                m_testResult.resultAt(nSelectionFirstRun) *
                ScalingPower(m_drillDataInfo.currentTestCell()->res_scal);
    }
    /*double lfValue = m_drillDataInfo.currentTestCell()->
     m_testResult.resultAt(m_lstSelections.first().run()) *
     ScalingPower(m_drillDataInfo.currentTestCell()->res_scal);*/

    m_pScaleColorWidget->parametricScaleColor->setSelection(lfValue);
    m_pScaleColorWidget->parametricScaleColor->updateScale();

    ShowDieProperties();
}

/******************************************************************************!
 * \fn onChangeChartType
 * \brief ToolBar buttons pressed
 ******************************************************************************/
void DrillDataMining3D::onChangeChartType(int iIndex)
{
    switch (iIndex)
    {
    case 0: m_RenderMode = FlatBars;     break;
    case 1: m_RenderMode = SolidBars;    break;
    case 2: m_RenderMode = SolidPoints;  break;
    case 3: m_RenderMode = SimpleLines;  break;
    case 4: m_RenderMode = Wireframe;    break;
    case 5: m_RenderMode = SmoothShaded; break;
    }

    // refresh the viewer
    refreshViewer();
}

/******************************************************************************!
 * \fn refreshViewer
 ******************************************************************************/
void DrillDataMining3D::refreshViewer(bool bReset  /* = false */)
{
    // First loading, sets the flat bar mode
    if (bReset)
    {
        // Default drawing mode = Flat bars
        m_RenderMode = FlatBars;
        comboBoxChartType->setCurrentIndex(0);
    }

    // Build the vector list to paint
    makeOpenGLList();

    if (bReset)
    {
        resetViewerPosition();
    }
    else
    {
        m_pViewer->updateGL();
    }

    m_pViewer->setFocus();
}

/******************************************************************************!
 * \fn GrabWidget
 ******************************************************************************/
QPixmap DrillDataMining3D::GrabWidget()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: Grab Widget...");

    // QPixmap p=QPixmap::grabWidget(this);
    // grabWidget cant catch correctly the 3d opengl viewer wafer on my windows7
    // Let s use the grabFrameBuffer which is working.

    // renderScene(); // As it do glCallList(..), do we have to re render ?

    QImage wm = m_pViewer->grabFrameBuffer(); wm.save(
                QDir::homePath() + "/GalaxySemi/temp/wafermap.png");
    QImage label = QPixmap::grabWidget(labelTest).toImage();
    // wm.save(QDir::homePath()+"/GalaxySemi/temp/panel.png");
    QImage panel = QPixmap::grabWidget(m_pPanelWidget->currentTabWidget()).toImage();
    panel.save(QDir::homePath() + "/GalaxySemi/temp/panel.png");
    QImage all(wm.width() + panel.width(),
               wm.height() + label.height(), wm.format());

    QPainter painter(&all);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    // painter.fillRect(wm.rect(), Qt::transparent);
    // painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.drawImage(0, label.height(), wm);
    painter.drawImage(wm.width(), label.height(), panel);
    painter.drawImage(0, 0, label);
    painter.end();

    // If the wafermap sometimes disappear after grabing,
    // launch this function to reset camera pos
    // m_pViewer->goHome();

    return QPixmap::fromImage(all);
}

/******************************************************************************!
 * \fn GrabGLViewer
 ******************************************************************************/
QPixmap DrillDataMining3D::GrabGLViewer()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: Grab GL Viewer...");

    qglviewer::Camera c;
    if (! m_pViewer->camera())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cant retrieve camera pos");
    }
    else
    {
        c = *m_pViewer->camera();
    }

    if (! m_pViewer->isValid())
    {
        GSLOG(SYSLOG_SEV_WARNING, "GrabGLViewer : viewer invalid");
    }
    m_pViewer->setAxisIsDrawn(true);
    m_pViewer->setGridIsDrawn(true);
    m_pViewer->setFPSIsDisplayed(true);
    // m_pViewer->showEntireScene(); // ?
    // m_pViewer->goHome(); //
    // use grabWindow or grabWidget ?
    // grabWidget seems to change the camera pos and/or dir :
    // the image is full dark blue without any wafermap.
    QPixmap p = QPixmap::grabWidget(m_pViewer);
    // grabWindow cannot catch the openGL rendering on most computers/OS
    // Let s use the grabWidget
    // QPixmap p=QPixmap::grabWindow(m_pViewer->winId());
    // if (!(c.orientation()==(*m_pViewer->camera()).orientation() ))
    //   GSLOG(SYSLOG_SEV_WARNING, "Camera changed orientation when grabing !");
    if (c.position() != (*m_pViewer->camera()).position())
    {
        GSLOG(SYSLOG_SEV_WARNING, "Camera changed position when grabing !");
    }
    return p;
}


void DrillDataMining3D::focusInEvent( QFocusEvent* ) {
    mWizardHandler->SetLastWafer3DViewed(this);
}

/******************************************************************************!
 * \fn onPickTest
 * \brief Pick a test
 ******************************************************************************/
void DrillDataMining3D::onPickTest()
{
    // Show TestList
    PickTestDialog dialogPickTest;

    // Allow/Disable Multiple selections.
    dialogPickTest.setMultipleSelection(false);
    dialogPickTest.setMultipleGroups(false, false);
    dialogPickTest.setUseInteractiveFilters(true);
    dialogPickTest.setAllowedTestType(PickTestDialog::TestAll);

    // Check if List was successfuly loaded
    if (dialogPickTest.fillParameterList(m_drillDataInfo.groupID()) &&
            dialogPickTest.exec() == QDialog::Accepted)
    {
        // Get test# selected
        // string format: <Test#>.<Pinmap#> , <Test#>.<Pinmap#> , etc.
        QString strTestsSelected = dialogPickTest.testItemizedList();

        // Empty selection!
        if (strTestsSelected.isEmpty() == false)
        {
            int nTestNumber;
            int nPinmap;

            if (sscanf(strTestsSelected.toLatin1().constData(), "%d%*c%d",
                       &nTestNumber, &nPinmap) < 2)
            {
                nPinmap = GEX_UNKNOWNTEST;  // No Pinmap index specified.

            }
            m_drillDataInfo.navigateTo(nTestNumber,
                                       nPinmap,
                                       dialogPickTest.testItemizedListNames()[0]);
        }
    }
}

/******************************************************************************!
 * \fn onExportWaferMap
 * \brief Export wafermap into a file
 ******************************************************************************/
void DrillDataMining3D::onExportWaferMap()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: on export wafer map...");

    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM() ||
            GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
    {
        QString m = ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    if (m_drillDataInfo.currentTestCell()->lTestNumber <
            GEX_TESTNBR_OFFSET_EXT_MIN ||
            m_drillDataInfo.currentTestCell()->lTestNumber >
            GEX_TESTNBR_OFFSET_EXT_MAX)
    {
        exportParametricWaferMap();
    }
    else if (pGexMainWindow)
    {
        pGexMainWindow->Wizard_ExportWafermap(m_drillDataInfo.makeExportLink());
    }
}

/******************************************************************************!
 * \fn exportParametricWaferMap
 * \brief Export parametric wafermap into a text file
 ******************************************************************************/
void DrillDataMining3D::exportParametricWaferMap()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: export parametric wafer map...");

    QString strOutputFileName;

    // Build default file name
    strOutputFileName =
            QString(m_drillDataInfo.file()->getMirDatas().szPartType) + QString("-");
    strOutputFileName +=
            QString(m_drillDataInfo.file()->getMirDatas().szLot) + QString("-");
    strOutputFileName +=
            QString(m_drillDataInfo.file()->getWaferMapData().szWaferID) + QString("-");
    strOutputFileName +=
            QString("T") + QString::number(m_drillDataInfo.testNumber());
    strOutputFileName += ".txt";

    // allows the user to change the output file name
    strOutputFileName =
            QFileDialog::getSaveFileName(this,
                                         "Save Wafer data as...",
                                         strOutputFileName,
                                         "text file (*.txt)");

    // If no file name selected, ignore command
    if (strOutputFileName.isEmpty())
    {
        return;
    }

    // Create file (IO_Translate to have CR or CR-LF for end-of-line
    // depending of the OS platform)
    QFile fileWafermap(strOutputFileName);
    QTextStream streamWafermapFile(&fileWafermap);

    if (fileWafermap.open(QIODevice::WriteOnly | QIODevice::Text) == false)
    {
        return;  // Failed writing to wafermap file.

    }
    // Write the header file
    streamWafermapFile << "File name     : "
                       << m_drillDataInfo.file()->strFileName << endl;
    streamWafermapFile << "Product       : "
                       << m_drillDataInfo.file()->getMirDatas().szPartType << endl;
    streamWafermapFile << "Lot ID        : "
                       << m_drillDataInfo.file()->getMirDatas().szLot << endl;
    streamWafermapFile << "SubLot        : "
                       << m_drillDataInfo.file()->getMirDatas().szSubLot << endl;
    streamWafermapFile << "Wafer ID      : "
                       << m_drillDataInfo.file()->getWaferMapData().szWaferID
                       << endl;
    streamWafermapFile << "Wafer Started : "
                       << TimeStringUTC(m_drillDataInfo.file()->
                                        getWaferMapData().lWaferStartTime);
    streamWafermapFile << "Wafer Ended   : "
                       << TimeStringUTC(m_drillDataInfo.file()->
                                        getWaferMapData().lWaferEndTime);
    streamWafermapFile << "Total dies    : "
                       << m_drillDataInfo.file()->getWaferMapData().
                          iTotalPhysicalDies << endl;
    streamWafermapFile << "Test          : "
                       << "T" + m_drillDataInfo.currentTestLabel() << endl;

    double  lfCustomScaleFactor;
    QString strUnits = m_drillDataInfo.currentTestCell()->GetScaledUnits(&lfCustomScaleFactor,
                                                                         ReportOptions.GetOption("dataprocessing","scaling").toString());

    streamWafermapFile << "Units         :" << strUnits << endl;
    streamWafermapFile << endl;

    for (int nDash = 0;
         nDash < m_drillDataInfo.file()->getWaferMapData().SizeX;
         nDash++)
    {
        streamWafermapFile << "---------";
    }

    streamWafermapFile << endl << endl;

    // Check the wafer map orientation
    int nDieX, nDieY;
    int nDieXStart, nDieXEnd, nDieXStep;
    int nDieYStart, nDieYEnd, nDieYStep;

    // Check for X direction
    if (m_drillDataInfo.file()->getWaferMapData().GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        nDieXStart = m_drillDataInfo.file()->getWaferMapData().iLowDieX;
        nDieXEnd   = nDieXStart + m_drillDataInfo.file()->getWaferMapData().SizeX;
        nDieXStep  = 1;
    }
    else
    {
        // X direction = 'L' (left)
        nDieXStart = m_drillDataInfo.file()->getWaferMapData().iHighDieX;
        nDieXEnd   = nDieXStart - m_drillDataInfo.file()->getWaferMapData().SizeX;
        nDieXStep  = -1;
    }

    // Check for Y direction
    if (m_drillDataInfo.file()->getWaferMapData().GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        nDieYStart = m_drillDataInfo.file()->getWaferMapData().iLowDieY;
        nDieYEnd   = nDieYStart + m_drillDataInfo.file()->getWaferMapData().SizeY;
        nDieYStep  = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        nDieYStart = m_drillDataInfo.file()->getWaferMapData().iHighDieY;
        nDieYEnd   = nDieYStart - m_drillDataInfo.file()->getWaferMapData().SizeY;
        nDieYStep  = -1;
    }

    // write wafermap data
    // double dValue;
    long lRun;

    int nCurrentTestCellResultsCount =
            m_drillDataInfo.currentTestCell()->m_testResult.count();
    QString strMsgToWrite;

    for (nDieY = nDieYStart; nDieY != nDieYEnd; nDieY += nDieYStep)
    {
        // Processing a wafer line.
        for (nDieX = nDieXStart; nDieX != nDieXEnd; nDieX += nDieXStep)
        {
            strMsgToWrite = "         ";
            lRun = m_drillDataInfo.file()->
                    findRunNumber(nDieX,
                                  nDieY,
                                  m_drillDataInfo.
                                  currentTestCell());

            if (lRun >= 0 && lRun < nCurrentTestCellResultsCount)
            {
                // condition could be changed with
                // m_drillDataInfo.currentTestCell()->m_testResult.isValidIndex(lRun)
                if (m_drillDataInfo.currentTestCell()->
                        m_testResult.isValidResultAt(lRun))
                {
                    strMsgToWrite =
                            QString::number(m_drillDataInfo.currentTestCell()->
                                            m_testResult.resultAt(lRun)).rightJustified(9,
                                                                                        ' ',
                                                                                        true);
                }
            }

            streamWafermapFile << strMsgToWrite;
        }

        streamWafermapFile << endl;
    }

    // close file
    fileWafermap.close();
}

/******************************************************************************!
 * \fn initGui
 * \brief Initialize the GUI
 ******************************************************************************/
void DrillDataMining3D::initGui()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: init GUI...");

    // Initialize contents of chart type combo box
    comboBoxChartType->clear();
    comboBoxChartType->insertItem(0, "Flat");
    comboBoxChartType->insertItem(1, "Bars");
    comboBoxChartType->insertItem(2, "Points");
    comboBoxChartType->insertItem(3, "Lines");
    comboBoxChartType->insertItem(4, "Wires");
    comboBoxChartType->insertItem(5, "Surface");

    // Initialize contents of chart color
    comboBoxSpectrumColors->clear();
    comboBoxSpectrumColors->insertItem(GEX3D_SPECTRUM_DATARANGE,
                                       "Over Data Range");
    comboBoxSpectrumColors->insertItem(GEX3D_SPECTRUM_TESTLIMITS,
                                       "Over Test Limits");
    comboBoxSpectrumColors->insertItem(GEX3D_SPECTRUM_PASSFAIL, "Pass/Fail");

    comboBoxSpectrumColors->hide();

    // Fill combo group
    if (gexReport->getGroupsList().count() > 1)
    {
        for (int nGroupIndex = 0;
             nGroupIndex < gexReport->getGroupsList().count();
             nGroupIndex++)
        {
            comboBoxGroup->
                    addItem(gexReport->getGroupsList().at(nGroupIndex)->strGroupName);
        }
    }
    else
    {
        comboBoxGroup->hide();
    }

    // Fill combo wafer
    fillComboWafermap(0);
}

/******************************************************************************!
 * \fn updateGui
 * \brief Update the GUI
 ******************************************************************************/
void DrillDataMining3D::updateGui()
{
    GSLOG(SYSLOG_SEV_DEBUG, "DrillDataMining3D update Gui...");
    pushButtonPreviousTest->setEnabled(m_drillDataInfo.hasPreviousTestCell());
    pushButtonNextTest->setEnabled(m_drillDataInfo.hasNextTestCell());

    if (comboBoxSpectrumColors->currentIndex() != m_nSpectrumColors)
    {
        comboBoxSpectrumColors->setCurrentIndex(m_nSpectrumColors);
    }

    comboBoxGroup->setCurrentIndex(m_drillDataInfo.groupID());
    comboBoxWafermap->setCurrentIndex(m_drillDataInfo.fileID());

    if ((m_drillDataInfo.testNumber() <= GEX_TESTNBR_OFFSET_EXT_HBIN) ||
            (m_drillDataInfo.testNumber() > GEX_TESTNBR_OFFSET_EXT_MAX))
    {
        pushButtonSave->show();
    }
    else
    {
        pushButtonSave->hide();
    }

    if (pGexMainWindow->pWizardSettings->iDrillSubType == GEX_WAFMAP_HARDBIN ||
            pGexMainWindow->pWizardSettings->iDrillSubType == GEX_WAFMAP_SOFTBIN)
    {
        m_pScaleColorWidget->binScaleColor->updateScale();
    }
    else if (pGexMainWindow->pWizardSettings->iDrillSubType ==
             GEX_WAFMAP_TEST_PASSFAIL ||
             pGexMainWindow->pWizardSettings->iDrillSubType ==
             GEX_WAFMAP_STACK_TEST_PASSFAIL)
    {
        m_pScaleColorWidget->parametricPassFailScaleColor->updateScale();
    }
    else
    {
        m_pScaleColorWidget->parametricScaleColor->updateScale();
    }

    if (! m_drillDataInfo.isFakeTest())
    {
        bool bEnableSpectrumColor = true;

        switch (m_eMode)
        {
        case DrillDataMining3D::standard:
        default:
            lineSpectrumColors->show();
            comboBoxSpectrumColors->show();
            break;

        case DrillDataMining3D::nnr:
            lineSpectrumColors->hide();
            comboBoxSpectrumColors->hide();
            break;
        }

        m_pScaleColorWidget->enableSpectrumColor(bEnableSpectrumColor);
    }
    else
    {
        lineSpectrumColors->hide();
        comboBoxSpectrumColors->hide();

        m_pScaleColorWidget->enableSpectrumColor(false);
    }

    QString strTestTitle("Test " + m_drillDataInfo.currentTestLabel());

    // If test doesn't hold any data sample,
    // then add an information in the test name
    if (m_drillDataInfo.currentTestCell() &&
            m_drillDataInfo.currentTestCell()->m_testResult.count() == 0)
    {
        strTestTitle +=
                QString("<font color=\"#FF0000\"><b> - No data samples</b></font>");
    }

    labelTest->setText(strTestTitle);
}

/******************************************************************************!
 * \fn onChangeSpectrumColors
 * \brief Spectrum colors selection changed
 ******************************************************************************/
void DrillDataMining3D::onChangeSpectrumColors(int iIndex)
{
    m_nSpectrumColors = iIndex;

    m_pScaleColorWidget->unLockBrowsing();

    updateData();

    fillViewer();
}

/******************************************************************************!
 * \fn onChangeGroup
 * \brief Group changed
 ******************************************************************************/
void DrillDataMining3D::onChangeGroup(int nGroupID)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("onChangeGroup %1").arg( nGroupID).toLatin1().constData());
    // Refill combo wafermap
    fillComboWafermap(nGroupID);

    m_drillDataInfo.changeGroupID(nGroupID);
}

/******************************************************************************!
 * \fn onSpectrumValueChanged
 * \brief Spectrum color has changed
 ******************************************************************************/
void DrillDataMining3D::onSpectrumValueChanged()
{
    fillViewer();
}

/******************************************************************************!
 * \fn OnChangeBinFilter
 ******************************************************************************/
void DrillDataMining3D::OnChangeBinFilter(QTreeWidgetItem*, int)
{
    GSLOG(SYSLOG_SEV_DEBUG, "OnChangeBinFilter");
    fillViewer();
}

/******************************************************************************!
 * \fn resetViewerPosition
 * \brief Reset viewer position
 ******************************************************************************/
void DrillDataMining3D::resetViewerPosition()
{
    Vec   vecEyePoint;
    Vec   vecCenterPoint;
    float fX, fY;

    switch (pGexMainWindow->pWizardSettings->iDrillType)
    {
    case GEX_DRILL_WAFERPROBE:

        // Set focus point
        if (landscapeSize)
        {
            fX = (50.0f * (float) landscapeUsageX) / (float) landscapeSize;
            fY = (50.0f * (float) landscapeUsageY) / (float) landscapeSize;

            // Set scene center
            vecCenterPoint.setValue(fX, fY, 0.0);

            // Set camera position
            vecEyePoint.setValue(fX, fY, 130.0);
        }
        else
        {
            // Set camera position
            vecEyePoint.setValue(50.0, 50.0, 60.0);

            // Set scene center
            vecCenterPoint.setValue(50.0, 50.0, 50.0);
        }
        break;

    case GEX_DRILL_PACKAGED:

        // Default drawing mode = Flat bars
        m_RenderMode = FlatBars;
        comboBoxChartType->setCurrentIndex(0);

        // Set focus point
        if (landscapeSize)
        {
            fX = (50.0f * (float) landscapeUsageX) / (float) landscapeSize;
            fY = (50.0f * (float) landscapeUsageY) / (float) landscapeSize;

            // Set scene center
            vecCenterPoint.setValue(fX, fY, 0.0);

            // Set camera position
            vecEyePoint.setValue(60, -90.0, 50.0);

        }
        else
        {
            // Set camera position
            vecEyePoint.setValue(60.0, -350.0, 200.0);

            // Set scene center
            vecCenterPoint.setValue(50.0, -50.0, 50.0);
        }
        break;

    case GEX_DRILL_HISTOGRAM:

        // Default drawing mode = Solid bars
        m_RenderMode = SolidBars;
        comboBoxChartType->setCurrentIndex(1);

        // Set focus point
        if (landscapeSize)
        {
            fX = (50.0f * (float) landscapeUsageX) / (float) landscapeSize;
            fY = (50.0f * (float) landscapeUsageY) / (float) landscapeSize;

            // Set scene center
            vecCenterPoint.setValue(fX, fY, 60.0);

            // Set camera position
            vecEyePoint.setValue(60, -280.0, 120.0);
        }
        else
        {
            // Set camera position
            vecEyePoint.setValue(60.0, -350.0, 200.0);

            // Set scene center
            vecCenterPoint.setValue(50.0, -50.0, 50.0);
        }
        break;

    case GEX_DRILL_TREND:

        // Default drawing mode = Solid bars
        m_RenderMode = SolidPoints;
        comboBoxChartType->setCurrentIndex(2);

        // Set focus point
        if (landscapeSize)
        {
            fX = (50.0f * (float) landscapeUsageX) / (float) landscapeSize;
            fY = (50.0f * (float) landscapeUsageY) / (float) landscapeSize;

            // Set scene center
            vecCenterPoint.setValue(fX, fY, 60.0);

            // Set camera position
            vecEyePoint.setValue(60, -280.0, 120.0);
        }
        else
        {
            // Set camera position
            vecEyePoint.setValue(60.0, -350.0, 200.0);

            // Set scene center
            vecCenterPoint.setValue(50.0, -50.0, 50.0);
        }
        break;

    default:
        break;
    }

    m_pViewer->setHome(vecEyePoint, vecCenterPoint);
}

/******************************************************************************!
 * \fn LoadLandscapeArray
 * \brief Load GEX data read from STDF files into Landscape Array...
 *        so to fit OpenGL format needs
 ******************************************************************************/
void DrillDataMining3D::LoadLandscapeArray()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: Load data mining 3d...");

    // Ensure we have a valide handle.
    if (gexReport == NULL)
    {
        return;  // Just in case
    }
    // Seek to correct groupID
    if (m_drillDataInfo.isValid())
    {
        // Destroy previous grid if any.
        destroyLandscape();

        switch (pGexMainWindow->pWizardSettings->iDrillType)
        {
        default:
            break;
        case GEX_DRILL_WAFERPROBE:
            // Load Landscape array from wafermap data
            LoadLandscapeArrayWafermap();
            break;
        case GEX_DRILL_PACKAGED:
            // Load Landscape array from Final test / Packaged data
            LoadLandscapeArrayPackaged();
            break;
        case GEX_DRILL_HISTOGRAM:
            // Load Landscape array from Histogram datadata
            LoadLandscapeArrayHistogram();
            break;
        case GEX_DRILL_TREND:
            // Load Landscape array from Trend datadata
            LoadLandscapeArrayTrend();
            break;
        }
    }
}

/******************************************************************************!
 * \fn LoadLandscapeArrayWafermap
 * \brief Copies ALL Wafermap data into the 3D Landscape array
 ******************************************************************************/
void DrillDataMining3D::LoadLandscapeArrayWafermap()
{
    CPartInfo* ptrPartInfo=0;
    int iChipX, iChipY, iCode, iIndex, iWaferIndex;

    // Ensure we have a valide handle.
    if (gexReport == NULL)
    {
        return;  // Just in case
    }
    // Check if die specific data collected
    if (m_drillDataInfo.file()->pPartInfoList.count() <= 0)
    {
        return;  // No data !

    }
    if (m_drillDataInfo.file()->getWaferMapData().bWaferMapExists == false)
    {
        return;  // No data!

    }
    // Clear list of NNR Dies
    m_lstNNRDie.clear();

    fLowColorHeight  = 0.0;  // 0% for Lowest  color
    fHighColorHeight = 100.00;  // 100% for Highest color
    iHighBin         = 0;  // Will hold the highest Bin#...if we draw Bin info.

    // Get highest Bin value in wafermap.
    for (iChipX = 0; iChipX < m_drillDataInfo.file()->getWaferMapData().SizeX;iChipX++)
    {
        for (iChipY = 0; iChipY < m_drillDataInfo.file()->getWaferMapData().SizeY; iChipY++)
        {
            iIndex = iChipX + (iChipY * m_drillDataInfo.file()->getWaferMapData().SizeX);
            iCode  = m_drillDataInfo.file()->getWaferMapData().getWafMap()[iIndex].getBin();

            switch (pGexMainWindow->pWizardSettings->iDrillSubType)
            {
            case GEX_WAFMAP_SOFTBIN:  // Standard Wafermap: SOFT BIN
            case GEX_WAFMAP_STACK_SOFTBIN:
            case GEX_WAFMAP_HARDBIN:  // Standard Wafermap: HARD BIN
            case GEX_WAFMAP_STACK_HARDBIN:
                // Collapse some binnings in same classes...typically 16 classes.
                if (iCode != GEX_WAFMAP_EMPTY_CELL)
                {
                    iHighBin = gex_max(iHighBin, iCode);
                }
                break;
            }
        }
    }

    // Create a square grid array that inclues the whole data
    iCode = 2 + qMax(m_drillDataInfo.file()->getWaferMapData().SizeX,
                     m_drillDataInfo.file()->getWaferMapData().SizeY);
    createLandscape(iCode);

    // Find PartInfo for this specific part
    QListIterator<CPartInfo*> lstIteratorPartInfo(m_drillDataInfo.file()->pPartInfoList);

    lstIteratorPartInfo.toFront();
    while (lstIteratorPartInfo.hasNext())
    {
        ptrPartInfo = lstIteratorPartInfo.next();

        // Convert to landscape X,Y coordinates
        iIndex =
                (ptrPartInfo->iDieX - m_drillDataInfo.file()->getWaferMapData().iLowDieX)
                + landscapeSize *
                (ptrPartInfo->iDieY - m_drillDataInfo.file()->getWaferMapData().iLowDieY);
        if (iIndex < 0)
        {
            goto next_chip;
        }

        iChipY = iIndex / landscapeSize;
        iChipX = iIndex % landscapeSize;

        // Turn wafermap according X and Y pos direction
        if (m_drillDataInfo.file()->getWaferMapData().GetPosXDirection() == false)
        {
            iChipX = landscapeSize - iChipX - 1 -
                    (landscapeSize - m_drillDataInfo.file()->getWaferMapData().SizeX);
        }

        if (m_drillDataInfo.file()->getWaferMapData().GetPosYDirection() == true)
        {
            iChipY = landscapeSize - iChipY - 1 -
                    (landscapeSize - m_drillDataInfo.file()->getWaferMapData().SizeY);
        }

        switch (pGexMainWindow->pWizardSettings->iDrillSubType)
        {
#if 0
        case GEX_WAFMAP_SOFTBIN:  // Packaged: SOFT BIN
        case GEX_WAFMAP_STACK_SOFTBIN:
            iCode = ptrPartInfo->iSoftBin;
            break;
        case GEX_WAFMAP_HARDBIN:  // Packaged: HARD BIN
        case GEX_WAFMAP_STACK_HARDBIN:
            iCode = ptrPartInfo->iHardBin;
            break;
        case GEX_WAFMAP_TESTOVERLIMITS:  // Zonnig on limits
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
        case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
        case GEX_WAFMAP_STACK_TESTOVERDATA:
            // Zoning test wafermap.
            iCode = ptrPartInfo->iValue;
            if (iCode > 100)
            {
                iCode = GEX3D_EMPTY_CELL;  // Invalid value...-1 means: ignore it!
            }
            break;
#endif
        case GEX_WAFMAP_SOFTBIN:  // Packaged: SOFT BIN
        case GEX_WAFMAP_STACK_SOFTBIN:
        case GEX_WAFMAP_HARDBIN:  // Packaged: HARD BIN
        case GEX_WAFMAP_STACK_HARDBIN:
        {
            iWaferIndex =
                    (ptrPartInfo->iDieX -
                     m_drillDataInfo.file()->getWaferMapData().iLowDieX) +
                    (ptrPartInfo->iDieY -
                     m_drillDataInfo.file()->getWaferMapData().iLowDieY) *
                    m_drillDataInfo.file()->getWaferMapData().SizeX;

            CGexFileInGroup* cgfigPtrDrillDataFile = m_drillDataInfo.file();
            if (! cgfigPtrDrillDataFile)
            {
                GEX_ASSERT(false);
                return;
            }

            const CWaferMap& cwmWaferMap =
                const_cast<CWaferMap&>(cgfigPtrDrillDataFile->
                                       getWaferMapData());
            if ((! cwmWaferMap.getWafMap()) || (iWaferIndex < 0))
            {
                // max index to test ?
                GEX_ASSERT(false);
                return;
            }

            iCode = cwmWaferMap.getWafMap()[iWaferIndex].getBin();

            break;
        }

        case GEX_WAFMAP_TESTOVERLIMITS:  // Zonnig on limits
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
        case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
        case GEX_WAFMAP_STACK_TESTOVERDATA:
        case GEX_WAFMAP_TEST_PASSFAIL:  // Zoning on test values range
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            iWaferIndex = (ptrPartInfo->iDieX - m_drillDataInfo.file()->getWaferMapData().iLowDieX) +
                          (ptrPartInfo->iDieY - m_drillDataInfo.file()->getWaferMapData().iLowDieY) *
                          m_drillDataInfo.file()->getWaferMapData().SizeX;
            iCode = m_drillDataInfo.file()->getWaferMapData().
                    getWafMap()[iWaferIndex].getBin();
            if (iCode < 0 || iCode > 100)
            {
                iCode = GEX3D_EMPTY_CELL;  // Invalid value... -1 means: ignore it!
            }
            CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
            if (m_eMode == nnr &&
                    lPatInfo &&
                    lPatInfo->isNNR_Die(ptrPartInfo->iDieX, ptrPartInfo->iDieY,
                                        m_drillDataInfo.testNumber(),
                                        m_drillDataInfo.pinmap()))
            {
                m_lstNNRDie.append(iChipX + (iChipY * landscapeSize));
            }

            break;

        }

        if (iCode != GEX3D_EMPTY_CELL &&
           m_pScaleColorWidget->binScaleColor->BinFilterState(iCode) == Qt::Unchecked)
        {
            iCode = GEX3D_EMPTY_CELL;  // Invalid value... -1 means: ignore it!
        }

        // Save value in Landscape array!
        landscape[iChipX][iChipY] = iCode;

        // Update number of objects to draw
        landscapeObjects++;

next_chip:;
    }  // while

    // Saves exact landscape array effectively loaded with data
    landscapeUsageX = m_drillDataInfo.file()->getWaferMapData().SizeX;
    landscapeUsageY = m_drillDataInfo.file()->getWaferMapData().SizeY;
}

/******************************************************************************!
 * \fn LoadLandscapeArrayPackaged
 * \brief Copies ALL Binning Packaged data into the 3D Landscape array
 ******************************************************************************/
void DrillDataMining3D::LoadLandscapeArrayPackaged()
{
    CPartInfo* ptrPartInfo;

    // Ensure we have a valide handle.
    if (gexReport == NULL)
    {
        return;  // Just in case

    }
    if (m_drillDataInfo.isValid())
    {
        if (m_drillDataInfo.file()->pPartInfoList.count() <= 0)
        {
            return;  // No data !

        }
        fLowColorHeight  = 0.0;  // 0% for Lowest  color
        fHighColorHeight = 100.00;  // 100% for Highest color
        iHighBin = 0;  // Will hold the highest Bin#...if we draw binning info.

        int iChipX, iChipY, iCode, iIndex;

        // Create a square grid array that inclues the whole data
        iCode = (int)
                (2.0 + sqrt((double) (m_drillDataInfo.file()->pPartInfoList.count())));
        createLandscape(iCode);

        // Find PartInfo for this specific part
        QListIterator<CPartInfo*>
                lstIteratorPartInfo(m_drillDataInfo.file()->pPartInfoList);

        iIndex = 0;

        lstIteratorPartInfo.toFront();
        while (lstIteratorPartInfo.hasNext())
        {
            ptrPartInfo = lstIteratorPartInfo.next();

            // Convert to landscape X,Y coordinates
            iChipY = iIndex / landscapeSize;
            iChipX = iIndex % landscapeSize;

            // Turn wafermap up side down + rotate to match the same view
            // as the PNG image created under Examinator HTML report.
            iChipX = landscapeSize - iChipX - 1;

            switch (pGexMainWindow->pWizardSettings->iDrillSubType)
            {
            case GEX_WAFMAP_SOFTBIN:  // Packaged: SOFT BIN
            case GEX_WAFMAP_STACK_SOFTBIN:
                iCode = ptrPartInfo->iSoftBin;
                // Keep track of binning spectrum covered: used later to compute color.
                iHighBin = gex_max(iHighBin, iCode);
                break;

            case GEX_WAFMAP_HARDBIN:  // Packaged: HARD BIN
            case GEX_WAFMAP_STACK_HARDBIN:
                iCode = ptrPartInfo->iHardBin;
                // Keep track of binning spectrum covered: used later to compute color.
                iHighBin = gex_max(iHighBin, iCode);
                break;

            case GEX_WAFMAP_TESTOVERLIMITS:  // Zonnig on limits
            case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
            case GEX_WAFMAP_STACK_TESTOVERDATA:
                // Zoning test wafermap.
                iCode = ptrPartInfo->iValue;
                if (iCode > 100)
                {
                    iCode = GEX3D_EMPTY_CELL;  // Invalid value...-1 means: ignore it!
                }
                break;
            }

            // Save value in Landscape array!
            landscape[iChipX][iChipY] = iCode;

            // Update number of objects to draw
            landscapeObjects++;

            // Increase the part info number
            iIndex++;
        }  // while

        // Saves exact landscape array effectively loaded with data
        landscapeUsageX = (iIndex < landscapeSize) ? iIndex : landscapeSize;
        landscapeUsageY = 1 + (iIndex / landscapeSize);
    }
}

/******************************************************************************!
 * \fn LoadLandscapeArrayHistogram
 * \brief Copies ALL Histogram data bars into the 3D Landscape array
 ******************************************************************************/
void DrillDataMining3D::LoadLandscapeArrayHistogram()
{
    unsigned long lTestNumber;
    long lPinmapIndex;

    // Ensure we have a valide handle.
    if (gexReport == NULL)
    {
        return;  // Just in case

    }
    if (m_drillDataInfo.isValid())
    {
        // List of tests in this group.
        ptDrillTestCell = m_drillDataInfo.group()->cMergedData.ptMergedTestList;

        // Find test cell: RESET list to ensure we scan list of the right group !
        // Check if pinmap index...
        QString strTest = pGexMainWindow->pWizardSettings->strDrillParameter;
        sscanf(strTest.toLatin1().constData(), "%lu%*c%ld",
               &lTestNumber, &lPinmapIndex);
        if (lPinmapIndex < 0)
        {
            lPinmapIndex = GEX_PTEST;
        }

        if (m_drillDataInfo.file()->
                FindTestCell(lTestNumber, lPinmapIndex,
                             &ptDrillTestCell, false, false) != 1)
        {
            return;  // Failed finding test!

        }
        fLowColorHeight  = 0.0;  // 0% for Lowest  color
        fHighColorHeight = 100.00;  // 100% for Highest color
        iHighBin = 0;  // Will hold the highest Bin#...if we draw binning info.

        int iCode, iIndex;
        // Will include lowest Ploted data result over all groups
        double lfChartBottom = C_INFINITE;
        // Will include highest Ploted data result over all groups
        double lfChartTop = -C_INFINITE;
        double lfValue;
        long   lLowOutliers = 0, lHighOutliers = 0;
        int    iChartType   = pGexMainWindow->pWizardSettings->iDrillSubType;
        int    iCell;

        // Scale limits so we may need to compare test data to them...
        // only needed on 1st group (reference)
        if ((ptDrillTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
        {
            // Draw LL limit markers (done only once: when charting Plot for group#1)
            lfDrillLowL = ptDrillTestCell->GetCurrentLimitItem()->lfLowLimit;  // Low limit exists

            // convert LowLimit to same scale as results:
            m_drillDataInfo.file()->
                    FormatTestResultNoUnits(&lfDrillLowL,
                                            ptDrillTestCell->llm_scal);

            lfDrillLowL *= ScalingPower(ptDrillTestCell->llm_scal);  // normalized
            lfDrillLowL /= ScalingPower(ptDrillTestCell->res_scal);  // normalized
        }
        else
        {
            lfDrillLowL = -C_INFINITE;  // Say: no Low limit!
        }
        if ((ptDrillTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
        {
            // Draw HL limit markers (done only once: when charting Plot for group#1)
            lfDrillHighL = ptDrillTestCell->GetCurrentLimitItem()->lfHighLimit;  // High limit exists

            // convert HighLimit to same scale as results:
            m_drillDataInfo.file()->
                    FormatTestResultNoUnits(&lfDrillHighL,
                                            ptDrillTestCell->hlm_scal);

            lfDrillHighL *= ScalingPower(ptDrillTestCell->hlm_scal);  // normalized
            lfDrillHighL /= ScalingPower(ptDrillTestCell->res_scal);  // normalized
        }
        else
        {
            lfDrillHighL = C_INFINITE;  // Say: no High limit!
        }
        // Create a square grid array that inclues the whole data
        iCode = 2 + TEST_ADVHISTOSIZE;
        createLandscape(iCode, 2);

        // Find data samples mix and max values...
        fDataStart = ptDrillTestCell->lfSamplesMin;
        fDataEnd   = ptDrillTestCell->lfSamplesMax;

        fDataStart /= ScalingPower(ptDrillTestCell->res_scal);  // normalized
        fDataEnd   /= ScalingPower(ptDrillTestCell->res_scal);  // normalized

        // Chart viewport is data...unless specified otherwise
        lfChartBottom = fDataStart;
        lfChartTop    = fDataEnd;

        // Now: check what viewport will be used...
        if ((iChartType == GEX_ADV_HISTOGRAM_OVERLIMITS) ||
                (iChartType == GEX_ADV_HISTOGRAM_CUMULLIMITS))
        {
            // Chart has to be done over limits...unless they do not exist!
            if (lfDrillLowL != -C_INFINITE)
            {
                lfChartBottom = lfDrillLowL;  // If chart over limits, update charting
            }
            if (lfDrillHighL != C_INFINITE)
            {
                lfChartTop = lfDrillHighL;  // If chart over limits, update charting
            }
            // Compute width of the 'TEST_ADVHISTOSIZE' classes to create
            lfHistoClassesWidth = lfChartTop - lfChartBottom;
        }
        else
        {
            // Compute width of the 'TEST_ADVHISTOSIZE' classes to create
            lfHistoClassesWidth = fDataEnd - fDataStart;
        }
        // Reset histogram array
        for (iIndex = 0; iIndex < TEST_ADVHISTOSIZE + 2; iIndex++)
        {
            landscape[iIndex][0] = 0;
        }

        // Load array
        for (iIndex = 0; iIndex < ptDrillTestCell->m_testResult.count(); ++iIndex)
        {
            if (ptDrillTestCell->m_testResult.isValidResultAt(iIndex))
            {
                lfValue = ptDrillTestCell->m_testResult.resultAt(iIndex);
                switch (iChartType)
                {
                case GEX_ADV_HISTOGRAM_OVERLIMITS:
                default:
                case GEX_ADV_HISTOGRAM_CUMULLIMITS:
                    // Only include data within the limits...compute outliers too
                    if (lfValue < lfDrillLowL)
                    {
                        lLowOutliers++;
                    }
                    else if (lfValue > lfDrillHighL)
                    {
                        lHighOutliers++;
                    }
                    else
                    {
                        // Valid test number, to record in the appropriate histogram class
                        iCell = (int)
                                (1 + ((TEST_ADVHISTOSIZE) *
                                      (lfValue - lfDrillLowL)) / (lfHistoClassesWidth));
                        // In case data is just on the edge...
                        // ensure we do not have an overflow !
                        if (iCell >= TEST_ADVHISTOSIZE)
                        {
                            iCell = TEST_ADVHISTOSIZE;
                        }
                        landscape[iCell][0]++;
                        landscapeObjects++;
                    }
                    break;
                case GEX_ADV_HISTOGRAM_OVERDATA:
                case GEX_ADV_HISTOGRAM_CUMULDATA:
                    // Include all samples...!
                    iCell = (int)
                            (1 + ((TEST_ADVHISTOSIZE) *
                                  (lfValue - fDataStart)) / (lfHistoClassesWidth));
                    // In case data is just on the edge...
                    // ensure we do not have an overflow!
                    if (iCell >= TEST_ADVHISTOSIZE)
                    {
                        iCell = TEST_ADVHISTOSIZE;
                    }
                    landscape[iCell][0]++;
                    landscapeObjects++;
                    break;
                }
            }
        }

        // Normalize the Histogram bars to range in 0-100
        lfBarScaling = 0;
        double lfCumul = 0;
        for (iIndex = 0; iIndex < TEST_ADVHISTOSIZE + 2; iIndex++)
        {
            // Compute Column size
            if ((iChartType == GEX_ADV_HISTOGRAM_CUMULLIMITS) ||
                    (iChartType == GEX_ADV_HISTOGRAM_CUMULDATA))
            {
                lfValue = landscape[iIndex][0];
                if (lfValue > 0)
                {
                    lfCumul += lfValue;
                    landscape[iIndex][0] = lfCumul;
                }
            }
            lfBarScaling = gex_max(lfBarScaling, landscape[iIndex][0]);
        }
        for (iIndex = 0; iIndex < TEST_ADVHISTOSIZE + 2; iIndex++)
        {
            if (landscape[iIndex][0] < 1)
            {
                landscape[iIndex][0] = -1;  // Reset to -1 cells NOT to draw!
            }
            else
            {
                landscape[iIndex][0] = 100.0 * (landscape[iIndex][0] / lfBarScaling);
            }
        }

        // Saves exact landscape array effectively loaded with data
        landscapeUsageX = (iIndex < landscapeSize) ? iIndex : landscapeSize;
        landscapeUsageY = 1 + (iIndex / landscapeSize);
    }
}

/******************************************************************************!
 * \fn LoadLandscapeArrayTrend
 * \brief Copies ALL Trend data plots into the 3D Landscape array
 ******************************************************************************/
void DrillDataMining3D::LoadLandscapeArrayTrend()
{
    long lTestNumber, lPinmapIndex;

    // Ensure we have a valide handle.
    if (gexReport == NULL)
    {
        return;  // Just in case

    }
    if (m_drillDataInfo.isValid())
    {
        // List of tests in this group.
        ptDrillTestCell = m_drillDataInfo.group()->cMergedData.ptMergedTestList;

        // Find test cell: RESET list to ensure we scan list of the right group!
        // Check if pinmap index...
        QString strTest = pGexMainWindow->pWizardSettings->strDrillParameter;
        sscanf(strTest.toLatin1().constData(),
               "%lu%*c%ld", &lTestNumber, &lPinmapIndex);
        if (lPinmapIndex < 0)
        {
            lPinmapIndex = GEX_PTEST;
        }

        if (m_drillDataInfo.file()->
                FindTestCell(lTestNumber, lPinmapIndex,
                             &ptDrillTestCell, false, false) != 1)
        {
            return;  // Failed finding test!

        }
        fLowColorHeight  = 0.0;  // 0% for Lowest  color
        fHighColorHeight = 100.00;  // 100% for Highest color
        iHighBin = 0;  // Will hold the highest Bin#...if we draw binning info.

        int iCode, iIndex;

        // Scale limits so we may need to compare test data to them...
        // only needed on 1st group (reference)
        if ((ptDrillTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
        {
            // Draw LL limit markers (done only once: when charting Plot for group#1)
            lfDrillLowL = ptDrillTestCell->GetCurrentLimitItem()->lfLowLimit;  // Low limit exists

            // convert LowLimit to same scale as results:
            m_drillDataInfo.file()->
                    FormatTestResultNoUnits(&lfDrillLowL,
                                            ptDrillTestCell->llm_scal);

            lfDrillLowL *= ScalingPower(ptDrillTestCell->llm_scal);  // normalized
            lfDrillLowL /= ScalingPower(ptDrillTestCell->res_scal);  // normalized
        }
        else
        {
            lfDrillLowL = -C_INFINITE;  // Say: no Low limit!
        }
        if ((ptDrillTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
        {
            // Draw HL limit markers (done only once: when charting Plot for group#1)
            lfDrillHighL = ptDrillTestCell->GetCurrentLimitItem()->lfHighLimit;  // High limit exists

            // convert HighLimit to same scale as results:
            m_drillDataInfo.file()->
                    FormatTestResultNoUnits(&lfDrillHighL,
                                            ptDrillTestCell->hlm_scal);

            lfDrillHighL *= ScalingPower(ptDrillTestCell->hlm_scal);  // normalized
            lfDrillHighL /= ScalingPower(ptDrillTestCell->res_scal);  // normalized
        }
        else
        {
            lfDrillHighL = C_INFINITE;  // Say: no High limit!
        }
        // Create a square grid array that inclues the whole data
        iCode = 2 + ptDrillTestCell->ldSamplesValidExecs;
        createLandscape(iCode, 2);

        // Find data samples mix and max values...
        fDataStart  = ptDrillTestCell->lfSamplesMin;
        fDataEnd    = ptDrillTestCell->lfSamplesMax;
        fDataStart /= ScalingPower(ptDrillTestCell->res_scal);  // normalized
        fDataEnd   /= ScalingPower(ptDrillTestCell->res_scal);  // normalized

        // Reset Trend array
        for (iIndex = 0;
             iIndex < ptDrillTestCell->ldSamplesValidExecs + 2;
             iIndex++)
        {
            landscape[iIndex][0] = -1;
        }

        // Normalize the Trend plots to range in 0-100
        lfBarScaling = (fDataEnd - fDataStart) / 100.0;

        // Load array so all data range in 0-100
        double lfValue;

        if (lfBarScaling != 0)
        {
            for (iIndex = 0;
                 iIndex < ptDrillTestCell->m_testResult.count(); ++iIndex)
            {
                if (ptDrillTestCell->m_testResult.isValidResultAt(iIndex))
                {
                    // Include all samples...!
                    lfValue = ptDrillTestCell->m_testResult.resultAt(iIndex);
                    landscape[iIndex][0] = (lfValue - fDataStart) / lfBarScaling;
                    landscapeObjects++;
                }
            }
        }

        // Saves exact landscape array effectively loaded with data
        landscapeUsageX = (iIndex < landscapeSize) ? iIndex : landscapeSize;
        landscapeUsageY = 1 + (iIndex / landscapeSize);
    }
}

/******************************************************************************!
 * \fn renderLandscapeArrayWafermap
 * \brief Renders the 3D Landscape array to paint the Wafermap
 ******************************************************************************/
void DrillDataMining3D::renderLandscapeArrayWafermap()
{
    // build OpenGL drawing sequence from Landscape array
    int    iCellX, iCellY;
    double fHeight;
    Vec    DiePosition3D;     // Die space position (Center)
    // Used to hold the object ID (user for select mode)
    long       lObjectIndex = 0;
    float      fColor = 0.0F; // Object color
    bool       lRetested = false;
    QString    strPartID;
    CPartInfo* ptrPartInfo;
    QColor     rgbColor;
    bool       bWafermapMarkerRetest =
            ReportOptions.GetOption("wafer", "marker").toString().split("|").
            contains("retest", Qt::CaseSensitive);

    // If Smooth Surface rendering mode: compute Normals vectors
    if (m_RenderMode == SmoothShaded)
    {
        calculateVertexNormals();
    }
    else
    {
        // Destroy Normal vectors if any Surface rendering previously done
        destroyNormals();
    }
    // Radius of plot will be 90% of die width
    float fRadius = m_fObjectCellSize / landscapeSize;

    for (iCellX = landscapeSize - 1; iCellX >= 0; --iCellX)
    {
        for (iCellY = 0; iCellY < landscapeSize; ++iCellY)
        {
            lRetested     = false;
            fHeight       = landscape[iCellX][iCellY];

            // Ignore negative values as they mean: ignore data!
            if (fHeight < 0)
            {
                goto next_cell;
            }

            DiePosition3D =
                    Vec((100.0 * iCellX) / (float) landscapeSize,
                        (100.0 * iCellY) / (float) landscapeSize, 0.0);

            // assign Index ID which will reply in selectionmode
            lObjectIndex = (iCellX + (iCellY * landscapeSize));
            glLoadName(lObjectIndex);

            // Check more Color mapping (eg: Bin1 = RED).
            switch (pGexMainWindow->pWizardSettings->iDrillSubType)
            {
            case GEX_WAFMAP_SOFTBIN:  // Standard Wafermap: SOFT BIN
            case GEX_WAFMAP_STACK_SOFTBIN:
            case GEX_WAFMAP_HARDBIN:  // Standard Wafermap: HARD BIN
            case GEX_WAFMAP_STACK_HARDBIN:

                // Die retested?
                if (bWafermapMarkerRetest &&
                        pGexMainWindow->pWizardSettings->iDrillSubType ==
                        GEX_WAFMAP_SOFTBIN)
                {
                    // Convert to landscape X,Y coordinates
                    int iChipY = lObjectIndex / landscapeSize;
                    int iChipX = lObjectIndex % landscapeSize;

                    // Turn wafermap according X and Y pos direction
                    if (m_drillDataInfo.file()->getWaferMapData().GetPosXDirection() == false)
                    {
                        iChipX = landscapeSize - iChipX - 1  -
                                (landscapeSize -
                                 m_drillDataInfo.file()->getWaferMapData().SizeX);
                    }

                    if (m_drillDataInfo.file()->getWaferMapData().GetPosYDirection() == true)
                    {
                        iChipY = landscapeSize - iChipY - 1  -
                                (landscapeSize -
                                 m_drillDataInfo.file()->getWaferMapData().SizeY);
                    }

                    long lIndex = iChipX + (iChipY * m_drillDataInfo.file()->getWaferMapData().SizeX);

                    // Draw a thin border if die retested...
                    if (m_drillDataInfo.file()->getWaferMapData().getCellTestCounter()[lIndex] > 1)
                        lRetested = true;
                }

                // Compute color...map Bin1 to GREEN
                if (fHeight == 1)
                {
                    fColor =  // Green
                            fLowColorHeight + 0.5 * (fHighColorHeight - fLowColorHeight);
                    // Rescale to that highest bin = 100% of height space
                    // fHeight *= 100.0/(float)iHighBin;
                }
                else
                {
                    // Rescale to that highest bin = 100% of height space
                    // fHeight *= 100.0/(float)iHighBin;
                    if (iHighBin)
                    {
                        fColor = fHeight * (100.0 / (float) iHighBin);
                    }
                    else
                    {
                        fColor = fHeight;
                    }
                }

                if (pGexMainWindow->pWizardSettings->iDrillSubType ==
                        GEX_WAFMAP_SOFTBIN ||
                        pGexMainWindow->pWizardSettings->iDrillSubType ==
                        GEX_WAFMAP_STACK_SOFTBIN)
                {
                    rgbColor = gexReport->cDieColor.GetWafmapDieColor((int) fHeight);
                }
                else
                {
                    rgbColor = gexReport->cDieColor.GetWafmapDieColor((int) fHeight,
                                                                      false);
                }
                break;

            case GEX_WAFMAP_TESTOVERLIMITS:  // Zonnig on limits
            case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
            case GEX_WAFMAP_STACK_TESTOVERDATA:
                if (m_eMode == DrillDataMining3D::nnr &&
                        m_lstNNRDie.contains(lObjectIndex) == false)
                {
                    fColor = GEX3D_NNR_COLOR;
                }
                else
                {
                    fColor = fHeight;
                }

                rgbColor = QColor();  // No RGB color
                break;
            case GEX_WAFMAP_TEST_PASSFAIL:  // Zoning on test values range
            case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                if (fHeight == GEX_WAFMAP_FAIL_CELL)
                {
                    fColor = GEX3D_FAIL_COLOR;
                }
                else
                {
                    fColor = GEX3D_PASS_COLOR;
                }

                rgbColor = QColor();  // No RGB color
                break;
            }

            // Plot object
            renderObject(&DiePosition3D, iCellX, iCellY, fColor, fRadius, fHeight,
                         rgbColor, lRetested);

next_cell:;
            // For the first line of Strip-Parts, show part #
            if (m_drillDataInfo.file()->getWaferMapData().bStripMap &&
                    (iCellY == m_drillDataInfo.file()->getWaferMapData().SizeY - 1) &&
                    (fHeight >= 0))
            {
                // Display DeviceId...in bright orange!
                glColor3f(1.0f, 0.764f, 0.0f);
                m_pclFont3D->glfStringCentering(true);
                DiePosition3D.x = (DiePosition3D.x + (fRadius / 2.0));
                DiePosition3D.y = (DiePosition3D.y + fRadius);
                DiePosition3D.z = (-fRadius / 10.0);

                // Get PartID into to write its name.
                ptrPartInfo = pGetPartInfo(lObjectIndex);
                if (ptrPartInfo != NULL)
                {
                    strPartID = "Part-" + QString::number(ptrPartInfo->lPartNumber);
                    renderText(DiePosition3D, XRight, true, fRadius / 5.0,
                               strPartID.toLatin1().constData());
                }
            }
        }
    }
}

/******************************************************************************!
 * \fn renderLandscapeArrayPackaged
 * \brief Renders the 3D Landscape array to paint the Packaged data
 ******************************************************************************/
void DrillDataMining3D::renderLandscapeArrayPackaged()
{
    // build OpenGL drawing sequence from Landscape array
    int     iChipX, iChipY, iIndex;
    double  fHeight;
    Vec     DiePosition3D; // Die space position (Center)
    QString strPartID;
    float   fColor = 0.0F; // Object color

    // If Smooth Surface rendering mode: compute Normals vectors.
    if (m_RenderMode == SmoothShaded)
    {
        calculateVertexNormals();
    }
    else
    {
        // Destroy Normal vectors if any Surface rendering previously done.
        destroyNormals();
    }
    // Radius of plot will be 70% of die width.
    float fRadius = m_fObjectCellSize / landscapeSize;

    CPartInfo* ptrPartInfo;
    QString    strMessage;

    if (m_drillDataInfo.isValid())
    {
        // Find PartInfo for this specific part
        QListIterator<CPartInfo*> lstIteratorPartInfo(
                    m_drillDataInfo.file()->pPartInfoList);
        iIndex = 0;

        lstIteratorPartInfo.toFront();
        while (lstIteratorPartInfo.hasNext())
        {
            ptrPartInfo = lstIteratorPartInfo.next();

            // Convert to landscape X,Y coordinates
            ptrPartInfo->iDieY = iChipY = iIndex / landscapeSize;
            iChipX = iIndex % landscapeSize;

            // Turn wafermap up side down + rotate to match the same view
            // as the PNG image created under Examinator HTML report.
            ptrPartInfo->iDieX = iChipX = landscapeSize - iChipX - 1;

            fHeight = landscape[iChipX][iChipY];
            if (fHeight < 0)
            {
                goto next_cell;  // Ignore negative values as they mean: ignore data!

            }
            DiePosition3D =
                    Vec((100.0 * iChipX) / (float) landscapeSize,
                        (100.0 * iChipY) / (float) landscapeSize, 0.0);

            // assign Index ID which will reply in selectionmode
            glLoadName(iIndex);

            // Check more Color mapping (eg: Bin1 = RED).
            switch (pGexMainWindow->pWizardSettings->iDrillSubType)
            {
            case GEX_WAFMAP_SOFTBIN:  // Standard Wafermap: SOFT BIN
            case GEX_WAFMAP_STACK_SOFTBIN:
            case GEX_WAFMAP_HARDBIN:  // Standard Wafermap: HARD BIN
            case GEX_WAFMAP_STACK_HARDBIN:
                // Compute color...map Bin1 to GREEN
                if (fHeight == 1)
                {
                    fColor =  // Green
                            fLowColorHeight + 0.5 * (fHighColorHeight - fLowColorHeight);
                    // Rescale to that highest bin = 100% of height space.
                    // fHeight *= 100.0/(float)iHighBin;
                }
                else
                {
                    // Rescale to that highest bin = 100% of height space.
                    // fHeight *= 100.0/(float)iHighBin;
                    fColor = fHeight * (100.0 / (float) iHighBin);
                }
                break;

            case GEX_WAFMAP_TESTOVERLIMITS:  // Zonnig on limits
            case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            case GEX_WAFMAP_TESTOVERDATA:  // Zoning on test values range
            case GEX_WAFMAP_STACK_TESTOVERDATA:
                fColor = fHeight;
                break;
            }

            // Plot object
            renderObject(&DiePosition3D, iChipX, iChipY, fColor, fRadius, fHeight);

            // For the first line of Parts, show part #
            if (iChipY == 0)
            {
                // Display DeviceId...in bright orange!
                glColor3f(1.0f, 0.764f, 0.0f);
                m_pclFont3D->glfStringCentering(true);
                DiePosition3D.z = (-fRadius / 10.0);
                DiePosition3D.x = (DiePosition3D.x + (fRadius / 2.0));
                strPartID = "Part-" + QString::number(ptrPartInfo->lPartNumber);
                renderText(DiePosition3D, XRight, true, fRadius / 5.0,
                           strPartID.toLatin1().constData());
            }

next_cell:;
            // Increase array index.
            iIndex++;
        }  // while
    }
}

/******************************************************************************!
 * \fn renderLandscapeArrayHistogram
 * \brief Renders the 3D Landscape array to paint the Histogram data
 ******************************************************************************/
void DrillDataMining3D::renderLandscapeArrayHistogram()
{
    // build OpenGL drawing sequence from Landscape array
    int     iHistoX, iHistoY, iIndex;
    double  fHeight;
    Vec     BarPosition3D; // Histogram Bar space position (Center)
    float   fColor = 0.0F; // Object color
    QString strPartID;
    double  lfBarPosition;

    // If Smooth Surface rendering mode: compute Normals vectors.
    if (m_RenderMode == SmoothShaded)
    {
        calculateVertexNormals();
    }
    else
    {
        // Destroy Normal vectors if any Surface rendering previously done.
        destroyNormals();
    }
    // Radius of plot will be 70% of die width.
    float   fRadius = m_fObjectCellSize / landscapeSize;
    QString strMessage;

    if (m_drillDataInfo.isValid())
    {
        // Review all bars.
        for (iIndex = 0; iIndex < landscapeUsageX; iIndex++)
        {
            // Convert to landscape X,Y coordinates
            iHistoY = iIndex / landscapeSize;
            iHistoX = iIndex % landscapeSize;

            // Turn wafermap up side down + rotate to match the same view
            // as the PNG image created under Examinator HTML report.
            iHistoX = landscapeSize - iHistoX - 1;

            fHeight = landscape[iHistoX][iHistoY];
            if (fHeight < 0)
            {
                goto next_cell;  // Ignore negative values as they mean: ignore data!

            }
            BarPosition3D = Vec((100.0 * iHistoX) / (float) landscapeSize,
                                (100.0 * iHistoY) / (float) landscapeSize, 0.0);

            // assign Index ID which will reply in selectionmode
            glLoadName(iIndex);

            // Check the color to apply :
            // Green (within the limits) or Red (outside the limits)
            lfBarPosition =
                    fDataStart + ((lfHistoClassesWidth * iIndex) / TEST_ADVHISTOSIZE);

            if ((lfBarPosition < lfDrillLowL) || (lfBarPosition > lfDrillHighL))
            {
                fColor = fHighColorHeight;  // RED
            }
            else
            {
                fColor =  // Green
                        fLowColorHeight + 0.5 * (fHighColorHeight - fLowColorHeight);

            }
            // Plot object
            renderObject(&BarPosition3D, iHistoX, iHistoY, fColor, fRadius, fHeight);

            // For the first line of Parts, show part #
            if (iHistoY == 0)
            {
                // Display DeviceId...in bright orange!
                glColor3f(1.0f, 0.764f, 0.0f);
                m_pclFont3D->glfStringCentering(true);
                BarPosition3D.z = (-fRadius / 10.0);
                BarPosition3D.x = (BarPosition3D.x + (fRadius / 2.0));
                strPartID = "Part";

                renderText(BarPosition3D, XRight, true, fRadius / 5.0,
                           strPartID.toLatin1().constData());
            }

next_cell:;
            // Next cell & array index.
        }
    }
}

/******************************************************************************!
 * \fn renderLandscapeArrayTrend
 * \brief Renders the 3D Landscape array to paint the Trend data
 ******************************************************************************/
void DrillDataMining3D::renderLandscapeArrayTrend()
{
    // build OpenGL drawing sequence from Landscape array
    int     iTrendX, iTrendY, iIndex;
    double  fHeight;
    Vec     BarPosition3D; // Histogram Bar space position (Center)
    float   fColor = 0.0F; // Object color
    QString strPartID;
    double  lfPlotPosition;

    // If Smooth Surface rendering mode: compute Normals vectors.
    if (m_RenderMode == SmoothShaded)
    {
        calculateVertexNormals();
    }
    else
    {
        // Destroy Normal vectors if any Surface rendering previously done.
        destroyNormals();
    }
    // Radius of plot will be 70% of die width.
    float   fRadius = m_fObjectCellSize / landscapeSize;
    QString strMessage;

    if (m_drillDataInfo.isValid())
    {
        // Review all plots.
        for (iIndex = 0; iIndex < landscapeUsageX; iIndex++)
        {
            // Convert to landscape X,Y coordinates
            iTrendY = iIndex / landscapeSize;
            iTrendX = iIndex % landscapeSize;

            // Turn wafermap up side down + rotate to match the same view
            // as the PNG image created under Examinator HTML report.
            iTrendX = landscapeSize - iTrendX - 1;

            fHeight = landscape[iTrendX][iTrendY];
            if (fHeight < 0)
            {
                goto next_cell;  // Ignore negative values as they mean: ignore data!

            }
            BarPosition3D = Vec((100.0 * iTrendX) / (float) landscapeSize,
                                (100.0 * iTrendY) / (float) landscapeSize, 0.0);

            // assign Index ID which will reply in selectionmode
            glLoadName(iIndex);

            // Check the color to apply :
            // Green (within the limits) or Red (outside the limits)
            lfPlotPosition = fDataStart + (lfBarScaling * fHeight);

            if ((lfPlotPosition < lfDrillLowL) || (lfPlotPosition > lfDrillHighL))
            {
                fColor = fHighColorHeight;  // RED
            }
            else
            {
                fColor =  // Green
                        fLowColorHeight + 0.5 * (fHighColorHeight - fLowColorHeight);

            }
            // Plot object
            renderObject(&BarPosition3D, iTrendX, iTrendY, fColor, fRadius, fHeight);

            // For the first line of Parts, show part #
            if ((iTrendY == 0) && ((landscapeUsageX % 10) == 0))
            {
                // Display DeviceId...in bright orange!
                glColor3f(1.0f, 0.764f, 0.0f);
                m_pclFont3D->glfStringCentering(true);
                BarPosition3D.z = (-fRadius / 10.0);
                BarPosition3D.x = (BarPosition3D.x + (fRadius / 2.0));
                strPartID = "Part";

                renderText(BarPosition3D, XRight, true, fRadius / 5.0,
                           strPartID.toLatin1().constData());
            }

next_cell:;
            // Next cell & array index.
        }
    }
}

/******************************************************************************!
 * \fn ValueToPosition
 * \brief Converts absolute data value into scaled number to fit
 *        into our OpenGL scaling (0=data Min, 100 = data Max)
 ******************************************************************************/
double ValueToPosition(double lfValue)
{
    if (fDataEnd == fDataStart)
    {
        return 0;
    }
    return 100 * (lfValue - fDataStart) / (fDataEnd - fDataStart);
}

/******************************************************************************!
 * \fn renderMarker
 * \brief Draw a marker (Mean, sigma, etc.)
 ******************************************************************************/
double
DrillDataMining3D::renderMarker(int         iPosition,
                                int         m_iGroupID,
                                double      lfValue,
                                double      fRadius,
                                bool  /*bDashedLine*/,
                                const char* szTitle)
{
    double lfMarkerPosition;
    Vec    cTextPoint;

    // Convert marker location to scaling
    lfMarkerPosition = ValueToPosition(lfValue);

    // Draw marker
    glBegin(GL_LINES);
    switch (iPosition)
    {
    case TopLoc:
    case BottomLoc:
        glVertex3d(lfMarkerPosition, 10 * m_iGroupID, -fRadius);
        glVertex3d(lfMarkerPosition, 10 * m_iGroupID, 110);
        break;
    case LeftLoc:
    case RightLoc:
        glVertex3d(-fRadius, 10 * m_iGroupID, lfMarkerPosition);
        glVertex3d(110, 10 * m_iGroupID, lfMarkerPosition);
        break;
    }
    glEnd();

    // Display Legend
    m_pclFont3D->glfStringCentering(true);

    switch (iPosition)
    {
    case TopLoc:
        cTextPoint.x = lfMarkerPosition;
        cTextPoint.y = 10 * m_iGroupID;
        cTextPoint.z = 110 + fRadius;
        break;
    case BottomLoc:
        cTextPoint.x = lfMarkerPosition;
        cTextPoint.y = 10 * m_iGroupID;
        cTextPoint.z = (-fRadius);
        break;
    case LeftLoc:
        cTextPoint.x = (-fRadius);
        cTextPoint.y = 10 * m_iGroupID;
        cTextPoint.z = lfMarkerPosition;
        break;
    case RightLoc:
        cTextPoint.x = 110 + fRadius;
        cTextPoint.y = (10 * m_iGroupID);
        cTextPoint.z = lfMarkerPosition;
        break;
    }

    // 1 char. = 4% of the landscape space.
    renderText(cTextPoint, XRight, true, 4.0, szTitle);
    return lfMarkerPosition;
}

/******************************************************************************!
 * \fn renderLandscapeArrayGrid
 * \brief Renders the 3D Landscape array to paint the Chart Grid & scales
 ******************************************************************************/
void
DrillDataMining3D::renderLandscapeArrayGrid(int iDirection1,
                                            int iDirection2,
                                            int iLimitDir,
                                            int iMarkersDir)
{
    Vec    cTextPoint;
    float  fRadius;
    double lfMean, lfSigma, lfMarker, lfTextSize, lfValue;
    char   szString[2 * GEX_MAX_STRING], szTestName[2 * GEX_MAX_STRING];

    // OPTIONS
    QString strAdvHistogramMarkerOptions =
            ((gexReport->getReportOptions())->GetOption(QString("adv_histogram"),
                                                        QString("marker"))).toString();
    QStringList qslAdvHistogramMarkerOptionsList =
            strAdvHistogramMarkerOptions.split(QString("|"));

    // disable selection mode on following drawings
    glLoadName(0);

    lfTextSize = 5.0;  // 1 char. = 5% of the landscape space

    // Radius of plot will be 100% of die width
    fRadius = m_fObjectCellSize / landscapeSize;

    // build OpenGL drawing sequence from Landscape array
    glPushMatrix();

    // Set Drawing color: light yellow
    glColor3f(0.326, 0.324, 0.137);

    // Set drawing origin
    glTranslatef(0, 0, 0);

    for (int iZ = 0; iZ <= 110; iZ += 10)
    {
        // Outline the Horizontal grid line
        glBegin(GL_LINE_STRIP);
        glVertex3d(0, -fRadius, iZ);
        glVertex3d(0, fRadius, iZ);
        glVertex3d(100, fRadius, iZ);
        glEnd();
    }

    // Base
    glBegin(GL_LINE_STRIP);
    glVertex3d(0, -fRadius, 0);
    glVertex3d(100, -fRadius, 0);
    glVertex3d(100, fRadius, 0);
    glEnd();

    // Verticale lines (Plain)
    glBegin(GL_LINES);
    glVertex3d(0, -fRadius, 0);
    glVertex3d(0, -fRadius, 110);
    glVertex3d(0, fRadius, 0);
    glVertex3d(0, fRadius, 110);
    glEnd();

    for (int iX = 10; iX <= 100; iX += 10)
    {
        // Outline the Vertical grid line (Dashed)
        glBegin(GL_LINES);
        for (int iZ = 0; iZ <= 110; iZ += 2)
        {
            glVertex3d(iX, fRadius, iZ);
            glVertex3d(iX, fRadius, iZ + 1);
        }
        glEnd();
    }

    if (m_drillDataInfo.isValid())
    {
        glColor3f(0, 1.0, 0);
        m_pclFont3D->glfStringCentering(true);
        cTextPoint.z = 120;
        cTextPoint.x = 50;
        cTextPoint.y = 0;
        gexReport->BuildTestNameString(
                    m_drillDataInfo.file(), ptDrillTestCell, szTestName);

        sprintf(szString, "Test %s: %s", ptDrillTestCell->szTestLabel, szTestName);
        renderText(cTextPoint, iDirection1, true, lfTextSize, szString);

        // Insert markers (vertical lines): Mean, LowL, HighL
        // Display Low Limit in RED
        if (((ptDrillTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) &&
                (qslAdvHistogramMarkerOptionsList.contains(QString("limits"))))
        {
            // Low limit exists...show it!
            glColor3f(1.0, 0, 0);
            lfMarker = ptDrillTestCell->GetCurrentLimitItem()->lfLowLimit;  // Low limit exists

            // convert LowLimit to same scale as results:
            m_drillDataInfo.file()->
                    FormatTestResultNoUnits(&lfMarker, ptDrillTestCell->llm_scal);

            lfMarker *= ScalingPower(ptDrillTestCell->llm_scal);  // normalized
            lfMarker /= ScalingPower(ptDrillTestCell->res_scal);  // normalized
            lfMarker  =
                    renderMarker(iLimitDir, 0, lfMarker, fRadius, false, "Low Limit");
        }

        // Display High Limit in RED
        if (((ptDrillTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) &&
                (qslAdvHistogramMarkerOptionsList.contains(QString("limits"))))
        {
            glColor3f(1.0, 0, 0);
            lfMarker = ptDrillTestCell->GetCurrentLimitItem()->lfHighLimit;  // High limit exists

            // convert HighLimit to same scale as results:
            m_drillDataInfo.file()->
                    FormatTestResultNoUnits(&lfMarker, ptDrillTestCell->hlm_scal);

            lfMarker *= ScalingPower(ptDrillTestCell->hlm_scal);  // normalized
            lfMarker /= ScalingPower(ptDrillTestCell->res_scal);  // normalized
            lfMarker  =
                    renderMarker(iLimitDir, 0, lfMarker, fRadius, false, "High Limit");
        }

        // Scale Mean & Sigma to correct scale
        lfMean = ptDrillTestCell->lfMean;
        m_drillDataInfo.file()->FormatTestResultNoUnits(&lfMean,
                                                        ptDrillTestCell->res_scal);

        lfSigma = ptDrillTestCell->lfSigma;
        m_drillDataInfo.file()->FormatTestResultNoUnits(&lfSigma,
                                                        ptDrillTestCell->res_scal);

        // If request to show the Mean marker
        if (qslAdvHistogramMarkerOptionsList.contains(QString("mean")))
        {
            glColor3f(0, 0, 1.0);  // BLUE color
            renderMarker(iMarkersDir, 0, lfMean, fRadius, false, "Mean");
        }

        // If request to show the 2sigma space (only written for 1st group)
        if (qslAdvHistogramMarkerOptionsList.contains(QString("2sigma")))
        {
            glColor3f(1.0, 0, 0);  // RED color
            renderMarker(iMarkersDir, 0, lfMean - lfSigma, fRadius, true, "-1s");
            renderMarker(iMarkersDir, 0, lfMean + lfSigma, fRadius, true, "+1s");
        }

        // If request to show the 3sigma space  (only written for 1st group)
        if (qslAdvHistogramMarkerOptionsList.contains(QString("3sigma")))
        {
            glColor3f(1.0, 0, 0);  // RED color
            renderMarker(iMarkersDir, 0,
                         lfMean - 1.5 * lfSigma, fRadius, true, "-1.5s");
            renderMarker(iMarkersDir, 0,
                         lfMean + 1.5 * lfSigma, fRadius, true, "+1.5s");
        }

        // If request to show the 6sigma space
        if (qslAdvHistogramMarkerOptionsList.contains(QString("6sigma")))
        {
            glColor3f(1.0, 0, 0);  // RED color
            renderMarker(iMarkersDir,
                         0, lfMean - 3.0 * lfSigma, fRadius, true, "-3s");
            renderMarker(iMarkersDir,
                         0, lfMean + 3.0 * lfSigma, fRadius, true, "+3s");
        }

        // Set Drawing color: light yellow
        glColor3f(0.5, 0.382f, 0.0);

        // Extract units to build values strings.
        char* ptChar = m_drillDataInfo.file()->
                FormatTestResult(ptDrillTestCell,
                                 ptDrillTestCell->lfMin,
                                 ptDrillTestCell->res_scal);
        if (sscanf(ptChar, "%*f %s", szTestName) != 1)
        {
            *szTestName = 0;
        }

        switch (pGexMainWindow->pWizardSettings->iDrillType)
        {
        default:
        case GEX_DRILL_WAFERPROBE:
        case GEX_DRILL_PACKAGED:
            break;

        case GEX_DRILL_HISTOGRAM:
            // chart X,Y axis legends...done only once at 1st group processing
            m_pclFont3D->glfStringCentering(true);
            cTextPoint.z = 50;
            cTextPoint.x = (-3);
            cTextPoint.y = 0;
            renderText(cTextPoint, iDirection2, true, lfTextSize, "Distribution (%)");

            // Write low bar percentage: 0%
            cTextPoint.x = (-5.0);
            cTextPoint.z = 3.0;
            renderText(cTextPoint, iDirection1, true, 2.0, "0 %");

            // Write highest bar percentage
            cTextPoint.x = (-5.0);
            cTextPoint.z = 100.0;
            lfValue      = (100.0 * lfBarScaling) / landscapeObjects;
            sprintf(szString, "%.0lf %%", lfValue);
            renderText(cTextPoint, iDirection1, true, 2.0, szString);

            // compute a string <value> units...just to extract the units!
            if (*szTestName)
            {
                sprintf(szString, "Test results (%s)", szTestName);
            }
            else
            {
                sprintf(szString, "Test results");  // No units!
            }
            cTextPoint.z = -3;
            cTextPoint.x = 50;
            cTextPoint.y = 0;

            renderText(cTextPoint, iDirection1, true, lfTextSize, szString);

            // Write low value
            sprintf(szString, "%s",
                    m_drillDataInfo.file()->
                    FormatTestResult(ptDrillTestCell, fDataStart,
                                     ptDrillTestCell->res_scal));
            cTextPoint.x = 0;
            renderText(cTextPoint, iDirection1, true, 2.0, szString);

            // Write high value
            sprintf(szString, "%s",
                    m_drillDataInfo.file()->
                    FormatTestResult(ptDrillTestCell, fDataEnd,
                                     ptDrillTestCell->res_scal));
            cTextPoint.x = 100;
            renderText(cTextPoint, iDirection1, true, 2.0, szString);
            break;

        case GEX_DRILL_TREND:
            // chart X,Y axis legends...done only once at 1st group processing
            // Write highest bar percentage
            cTextPoint.x = (-5.0);
            cTextPoint.y = 0;
            cTextPoint.z = 3.0;
            // Write low value
            sprintf(szString, "%s",
                    m_drillDataInfo.file()->
                    FormatTestResult(ptDrillTestCell, fDataStart,
                                     ptDrillTestCell->res_scal));
            renderText(cTextPoint, iDirection1, true, 2.0, szString);

            // Write high value
            sprintf(szString, "%s",
                    m_drillDataInfo.file()->
                    FormatTestResult(ptDrillTestCell, fDataEnd,
                                     ptDrillTestCell->res_scal));
            cTextPoint.z = 100;
            renderText(cTextPoint, iDirection1, true, 2.0, szString);

            // Tell number of samples found.
            cTextPoint.z = -3;
            cTextPoint.x = 50;
            cTextPoint.y = 0;
            sprintf(szString, "Data count/ID (%d samples)",
                    ptDrillTestCell->ldSamplesValidExecs);
            renderText(cTextPoint, iDirection1, true, lfTextSize, szString);
            break;
        }

        glPopMatrix();
    }
}

/******************************************************************************!
 * \fn makeOpenGLList
 * \brief Called in Constructor: allocate/set required buffers
 ******************************************************************************/
void DrillDataMining3D::makeOpenGLList()
{
    //m_pViewer->makeCurrent();

    // generate display list for all vectors to draw!
    if (glIsList(m_glDispList))
    {
        glDeleteLists(m_glDispList, 1);
    }
    m_glDispList = glGenLists(1);
    glNewList(m_glDispList, GL_COMPILE);

    // Define light
    GLfloat mat_ambient[]     = {0.8f, 0.8f, 0.8f, 1.0f}; //{0.2, 0.2, 0.2, 1.0};
    GLfloat mat_diffuse[]     = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat mat_specular[]    = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat matObj_specular[] = { .72f, .8f, .93f, 1.0f };
    GLfloat mat_shininess[]   = { 100.0f };
    GLfloat arfLightPos[4];

    glFlush();
    // individual vertices of the faces, etc...
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

    // Set the GL_AMBIENT_AND_DIFFUSE color state variable to be the
    // one referred to by all following calls to glColor
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glLightfv(GL_LIGHT0, GL_AMBIENT, mat_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, mat_specular);
    arfLightPos[0] = 150.0;
    arfLightPos[1] = 0.0;
    arfLightPos[2] = 150.0;
    arfLightPos[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_POSITION, arfLightPos);

    // Set Light for drawing scene.
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.5);

    glMaterialfv(GL_FRONT, GL_SPECULAR, matObj_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128.0);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate correct Rendering Mode
    SetRenderingMode();

    Vec cObjectPoint;
    if (landscapeSize == 0)
    {
        // No Data to render!
        QString strMessage;
        switch (pGexMainWindow->pWizardSettings->iDrillType)
        {
        default:
            strMessage = "No data to render!";
            cObjectPoint.setValue(50.0, 100.0, 50.0);
            break;
        case GEX_DRILL_WAFERPROBE:
            strMessage = "Wafer view";
            cObjectPoint.setValue(0.0, 30.0, 50.0);
            break;
        case GEX_DRILL_PACKAGED:
            strMessage = "Packaged view";
            cObjectPoint.setValue(50.0, 0.0, 50.0);
            break;
        case GEX_DRILL_HISTOGRAM:
            strMessage = "Histogram view";
            cObjectPoint.setValue(50.0, 0.0, 50.0);
            break;
        }
        // Show message! (Text color is bright orange)
        glColor3f(1.0f, 0.764f, 0.0f);
        m_pclFont3D->glfStringCentering(true);
        renderText(cObjectPoint, XRight, true, 10.0,
                   strMessage.toLatin1().constData());
        cObjectPoint.z = cObjectPoint.z - 15.0;
        renderText(cObjectPoint, XRight, true, 10.0,
                   "No data matching your filter");
    }
    else
    {
        switch (pGexMainWindow->pWizardSettings->iDrillType)
        {
        default:
            break;
        case GEX_DRILL_WAFERPROBE:
            // Load Landscape array from wafermap
            renderLandscapeArrayWafermap();
            break;
        case GEX_DRILL_PACKAGED:
            // Load Landscape array from Final test binning data
            renderLandscapeArrayPackaged();
            break;
        case GEX_DRILL_HISTOGRAM:
            // Load Landscape array from Histogram data
            renderLandscapeArrayHistogram();
            // Load Grid array
            renderLandscapeArrayGrid(XRight, ZRight, BottomLoc, TopLoc);
            break;
        case GEX_DRILL_TREND:
            // Load Landscape array from Histogram data
            renderLandscapeArrayTrend();
            // Load Grid array
            renderLandscapeArrayGrid(XRight, ZRight, LeftLoc, RightLoc);
            break;
        }
    }
    glEndList();
}

/******************************************************************************!
 * \fn renderScene
 * \brief Build ALL OpenGL objects & display them
 ******************************************************************************/
void DrillDataMining3D::renderScene()
{
    static unsigned iCheckAppEvents = 0;

    // Ensure we continue to process App. events...
    // even if doing heavy graphics (every 8 scene rendering)
    iCheckAppEvents++;
    if (iCheckAppEvents & 8)
    {
        QCoreApplication::processEvents();
    }

    // Do not bother rendering if use has moved to another Page!
    if (isVisible() == false)
    {
        return;
    }

    // Have our OpenGL compiled list ploted again!

    glCallList(m_glDispList);
}

/******************************************************************************!
 * \fn renderObject
 * \brief Render object (Line, Bar, Surface, etc.)
 ******************************************************************************/
void
DrillDataMining3D::renderObject(Vec*          pDiePosition3D,
                                int           x,
                                int           y,
                                float         fColor,
                                float         fRadius,
                                float         fHeight,
                                const QColor& rgbColor  /*= QColor()*/,
                                bool          bRetested  /*= false*/)
{
    // Update Object index rendered
    iRenderIndex++;

    switch (m_RenderMode)
    {
    case FlatBars:
        renderBar(*pDiePosition3D,
                  fColor, true, fRadius, fRadius, rgbColor, bRetested);
        break;
    case SolidBars:
        renderBar(*pDiePosition3D,
                  fColor, true, fRadius, fHeight, rgbColor, bRetested);
        break;
    case OutlinedBars:
        renderBar(*pDiePosition3D, fColor, false, fRadius, fHeight, rgbColor);
        break;
    case SolidPoints:
        renderPoint(*pDiePosition3D,
                    fColor, true, fRadius, fHeight, rgbColor, bRetested);
        break;
    case OutlinedPoints:
        renderPoint(*pDiePosition3D, fColor, false, fRadius, fHeight, rgbColor);
        break;
    case SimpleLines:
        // Draw a Line
        renderLine(*pDiePosition3D, fColor, fHeight, rgbColor);
        break;
    case Wireframe:
        // Draw a WIRE
        renderWire(*pDiePosition3D, fColor, x, y, rgbColor);
        break;
    case SmoothShaded:
        drawSmoothShaded(*pDiePosition3D, fColor, x, y, rgbColor);
        break;
    }
}

/******************************************************************************!
 * \fn renderLine
 ******************************************************************************/
void
DrillDataMining3D::renderLine(const Vec&    cOrigin,
                              float         fColor,
                              float         fHeight,
                              const QColor& rgbColor  /*= QColor()*/,
                              float  /*iLineWidth = 2.5*/)
{
    glPushMatrix();

    // Set drawing origin
    glTranslatef(cOrigin.x, cOrigin.y, 0);

    // Set Drawing color
    SetPlotColor(fColor, rgbColor);

    // Outline the line
    glBegin(GL_LINES);
    glVertex3d(0, 0, 0);
    glVertex3d(0, 0, fHeight);
    glEnd();

    glPopMatrix();
}

/******************************************************************************!
 * \fn renderPoint
 ******************************************************************************/
void
DrillDataMining3D::renderPoint(const Vec&    cOrigin,
                               float         fColor,
                               bool          bSolid,
                               float         iWidth,
                               float         fHeight,
                               const QColor& rgbColor  /*= QColor()*/,
                               bool          bRetested  /*= false*/,
                               float  /*iLineWidth = 2.5*/)
{
#ifndef DRILL_NO_DATA_MINING
    glPushMatrix();

    // Half box width...so we have the point centered on it!
    iWidth /= 2;

    // Set drawing origin
    glTranslatef(cOrigin.x, cOrigin.y, fHeight - iWidth);

    // Set Drawing color
    SetPlotColor(fColor, rgbColor);

    if (bSolid == true)
    {
        // Draw the sides of the Point
        glBegin(GL_QUAD_STRIP);
        glVertex3d(0, 0, 0);
        glVertex3d(0, iWidth, 0);
        glVertex3d(iWidth, 0, 0);
        glVertex3d(iWidth, iWidth, 0);

        glVertex3d(iWidth, 0, iWidth);
        glVertex3d(iWidth, iWidth, iWidth);
        glVertex3d(0, 0, iWidth);
        glVertex3d(0, iWidth, iWidth);
        glVertex3d(0, 0, 0);
        glVertex3d(0, iWidth, 0);
        glEnd();

        // Draw the top and bottom of the Point
        glBegin(GL_QUADS);
        glVertex3d(0, iWidth, 0);
        glVertex3d(iWidth, iWidth, 0);
        glVertex3d(iWidth, iWidth, iWidth);
        glVertex3d(0, iWidth, iWidth);

        glVertex3d(0, 0, 0);
        glVertex3d(iWidth, 0, 0);
        glVertex3d(iWidth, 0, iWidth);
        glVertex3d(0, 0, iWidth);
        glEnd();
        // Outline color is made a little darked so to be seen
        SetPlotColorOutline(fColor, rgbColor, bRetested);
    }

    // Make sure we disable Alias while drawing lines.
    int iLineSmooth = glIsEnabled(GL_LINE_SMOOTH);
    glDisable(GL_LINE_SMOOTH);

    if (iRenderIndex > 1)
    {
        // If Object#2 or higher, we may connect each point to the previous one!
        switch (pGexMainWindow->pWizardSettings->iDrillType)
        {
        case GEX_DRILL_WAFERPROBE:
        case GEX_DRILL_PACKAGED:
        case GEX_DRILL_HISTOGRAM:
            // Outline the solid bar...with a little darker color.
            glBegin(GL_LINES);
            glVertex3d(0, 0, 0);
            glVertex3d(0, iWidth, 0);
            glVertex3d(iWidth, 0, 0);
            glVertex3d(iWidth, iWidth, 0);
            glVertex3d(iWidth, 0, iWidth);
            glVertex3d(iWidth, iWidth, iWidth);
            glVertex3d(0, 0, iWidth);
            glVertex3d(0, iWidth, iWidth);
            glVertex3d(0, iWidth, 0);
            glVertex3d(0, iWidth, iWidth);
            glVertex3d(iWidth, iWidth, 0);
            glVertex3d(iWidth, iWidth, iWidth);
            glVertex3d(0, 0, 0);
            glVertex3d(0, 0, iWidth);
            glVertex3d(iWidth, 0, 0);
            glVertex3d(iWidth, 0, iWidth);

            glVertex3d(0, iWidth, 0);
            glVertex3d(iWidth, iWidth, 0);
            glVertex3d(0, 0, 0);
            glVertex3d(iWidth, 0, 0);
            glVertex3d(0, iWidth, iWidth);
            glVertex3d(iWidth, iWidth, iWidth);
            glVertex3d(0, 0, iWidth);
            glVertex3d(iWidth, 0, iWidth);
            glEnd();
            break;
        case GEX_DRILL_TREND:  // Plot point, then conect it to previous one.
            // Outline the line
            glBegin(GL_LINES);
            glVertex3d(0, 0, iWidth);  // Current point (current origin)
            glVertex3d(previousPoint.x - cOrigin.x,
                       previousPoint.y - cOrigin.y,
                       previousPoint.z - (fHeight - iWidth));
            glEnd();
            break;
        }
    }
    glPopMatrix();

    // Reload default line style
    if (iLineSmooth)
    {
        glEnable(GL_LINE_SMOOTH);
    }
    else
    {
        glDisable(GL_LINE_SMOOTH);
    }


    // Save previous point location.
    previousPoint   = cOrigin;
    previousPoint.z = fHeight - iWidth;
#endif
}

/******************************************************************************!
 * \fn renderBar
 ******************************************************************************/
void
DrillDataMining3D::renderBar(const Vec&    cOrigin,
                             float         fColor,
                             bool          bSolid,
                             float         iWidth,
                             float         fHeight,
                             const QColor& rgbColor  /*= QColor()*/,
                             bool          bRetested  /*= false*/,
                             float  /*iLineWidth = 2.5*/)
{
    glPushMatrix();

    // Set drawing origin
    glTranslatef(cOrigin.x, cOrigin.y, cOrigin.z);

    // Set Drawing color
    SetPlotColor(fColor, rgbColor);

    if (bSolid == true)
    {
        // Draw the sides of the Bar
        glBegin(GL_QUAD_STRIP);
        glVertex3d(0, 0, 0);
        glVertex3d(0, iWidth, 0);
        glVertex3d(iWidth, 0, 0);
        glVertex3d(iWidth, iWidth, 0);
        glVertex3d(iWidth, 0, fHeight);
        glVertex3d(iWidth, iWidth, fHeight);
        glVertex3d(0, 0, fHeight);
        glVertex3d(0, iWidth, fHeight);
        glVertex3d(0, 0, 0);
        glVertex3d(0, iWidth, 0);
        glEnd();

        // Draw the top and bottom of the bar
        glBegin(GL_QUADS);
        glVertex3d(0, iWidth, 0);
        glVertex3d(iWidth, iWidth, 0);
        glVertex3d(iWidth, iWidth, fHeight);
        glVertex3d(0, iWidth, fHeight);

        glVertex3d(0, 0, 0);
        glVertex3d(iWidth, 0, 0);
        glVertex3d(iWidth, 0, fHeight);
        glVertex3d(0, 0, fHeight);
        glEnd();
        // Outline color is made a little darked so to be seen
        SetPlotColorOutline(fColor, rgbColor, bRetested);
    }

    // Make sure we disable Alias while drawing lines.
    int iLineSmooth = glIsEnabled(GL_LINE_SMOOTH);
    glDisable(GL_LINE_SMOOTH);

    // Outline the solid bar...with a little darker color.
    glBegin(GL_LINES);
    glVertex3d(0, 0, 0);
    glVertex3d(0, iWidth, 0);
    glVertex3d(iWidth, 0, 0);
    glVertex3d(iWidth, iWidth, 0);
    glVertex3d(iWidth, 0, fHeight);
    glVertex3d(iWidth, iWidth, fHeight);
    glVertex3d(0, 0, fHeight);
    glVertex3d(0, iWidth, fHeight);
    glVertex3d(0, iWidth, 0);
    glVertex3d(0, iWidth, fHeight);
    glVertex3d(iWidth, iWidth, 0);
    glVertex3d(iWidth, iWidth, fHeight);
    glVertex3d(0, 0, 0);
    glVertex3d(0, 0, fHeight);
    glVertex3d(iWidth, 0, 0);
    glVertex3d(iWidth, 0, fHeight);

    glVertex3d(0, iWidth, 0);
    glVertex3d(iWidth, iWidth, 0);
    glVertex3d(0, 0, 0);
    glVertex3d(iWidth, 0, 0);
    glVertex3d(0, iWidth, fHeight);
    glVertex3d(iWidth, iWidth, fHeight);
    glVertex3d(0, 0, fHeight);
    glVertex3d(iWidth, 0, fHeight);
    glEnd();

    // Reload default line style.
    if (iLineSmooth)
    {
        glEnable(GL_LINE_SMOOTH);
    }
    else
    {
        glDisable(GL_LINE_SMOOTH);
    }

    glPopMatrix();
}

/******************************************************************************!
 * \fn renderWire
 ******************************************************************************/
void
DrillDataMining3D::renderWire(const Vec&    cOrigin,
                              float         fColor,
                              int           x,
                              int           y,
                              const QColor& rgbColor  /*= QColor()*/)
{
    float fHeight1 = landscape[x][y];
    float fHeight2 = landscape[x + 1][y];
    float fHeight3 = landscape[x][y + 1];
    float fHeight4 = landscape[x + 1][y + 1];
    float fRadius  = 100.0 / landscapeSize; // Full Die size.

    // Set Drawing color
    SetPlotColor(fColor, rgbColor);

    glPushMatrix();
    glTranslatef(cOrigin.x, cOrigin.y, cOrigin.z);

    // Draw the Wire lines.
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, fHeight1);
    glVertex3f(fRadius, 0.0, fHeight2);
    glVertex3f(0.0, 0.0, fHeight1);
    glVertex3f(fRadius, fRadius, fHeight4);
    glVertex3f(0.0, 0.0, fHeight1);
    glVertex3f(0.0, fRadius, fHeight3);
    glEnd();

    glPopMatrix();
}

/******************************************************************************!
 * \fn renderSolidCylinder
 ******************************************************************************/
void
DrillDataMining3D::
renderSolidCylinder(const Vec& cOrigin,
                    float      radius,
                    float      fHeight,
                    int        iSlices,
                    int        iStacks,
                    const      QColor&  /*rgbColor = QColor()*/,
                    float  /*iLineWidth = 2.5*/)
{
    static GLUquadricObj* quadObj;
    static int entry = 0;

    glPushMatrix();
    glTranslatef(cOrigin.x, cOrigin.y, cOrigin.z);

    quadObj = gluNewQuadric();
    if (! entry)
    {
        gluQuadricDrawStyle(quadObj, (GLenum) GLU_FILL);
        gluQuadricOrientation(quadObj, (GLenum) GLU_OUTSIDE);
        gluQuadricNormals(quadObj, (GLenum) GLU_SMOOTH);
    }

    // Draw Cylinder
    gluCylinder(quadObj, radius, radius, fHeight, iSlices, iStacks);
    gluDeleteQuadric(quadObj);

    // Close Bottom with disk
    quadObj = gluNewQuadric();
    gluDisk(quadObj, 0, radius, iSlices, iStacks);
    gluDeleteQuadric(quadObj);

    // Close Top with disk
    glTranslatef(0, 0, fHeight);
    quadObj = gluNewQuadric();
    gluDisk(quadObj, 0, radius, iSlices, iStacks);
    gluDeleteQuadric(quadObj);

    glPopMatrix();
}

/******************************************************************************!
 * \fn renderSolidSphere
 ******************************************************************************/
void
DrillDataMining3D::
renderSolidSphere(const Vec& cCenter,
                  float      radius,
                  int        iSlices,
                  int        iStacks,
                  const      QColor&  /*rgbColor = QColor()*/,
                  float  /*iLineWidth = 2.5*/)
{
    static GLUquadricObj* quadObj;
    static int entry = 0;

    glPushMatrix();
    glTranslatef(cCenter.x, cCenter.y, cCenter.z);
    quadObj = gluNewQuadric();
    if (! entry)
    {
        gluQuadricDrawStyle(quadObj, (GLenum) GLU_FILL);
        gluQuadricOrientation(quadObj, (GLenum) GLU_OUTSIDE);
        gluQuadricNormals(quadObj, (GLenum) GLU_SMOOTH);
    }
    gluSphere(quadObj, radius, iSlices, iStacks);
    gluDeleteQuadric(quadObj);

    glPopMatrix();
}


/******************************************************************************!
 * \fn renderWireSphere
 ******************************************************************************/
void
DrillDataMining3D::renderWireSphere(const Vec& cCenter,
                                    float      radius,
                                    int        iSlices,
                                    int        iStacks,
                                    const      QColor&  /*rgbColor = QColor()*/,
                                    float      iLineWidth  /*= 2.5*/)
{
    static GLUquadricObj* quadObj;
    static int entry = 0;

    glLineWidth(iLineWidth);
    glPushMatrix();
    glTranslatef(cCenter.x, cCenter.y, cCenter.z);
    quadObj = gluNewQuadric();
    if (! entry)
    {
        gluQuadricDrawStyle(quadObj, (GLenum) GLU_LINE);
        gluQuadricOrientation(quadObj, (GLenum) GLU_OUTSIDE);
        gluQuadricNormals(quadObj, (GLenum) GLU_SMOOTH);
    }
    gluSphere(quadObj, radius, iSlices, iStacks);
    gluDeleteQuadric(quadObj);

    glPopMatrix();
}


/******************************************************************************!
 * \fn resetSelection
 * \brief Slot to reset selection
 ******************************************************************************/
void DrillDataMining3D::resetSelection(bool resetAll/*=true*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, "DrillDataMining3D reset selection...");
    // Empty the list of selections. Auto delete pointers + buffers
    m_lstSelections.clear();

    // Show global properties
    ShowGlobalProperties();

    // Remove selection on the scale color
    m_pScaleColorWidget->parametricScaleColor->setSelection(GEX_C_DOUBLE_NAN);
    m_pScaleColorWidget->parametricScaleColor->updateScale();

    if (resetAll)
    {
        // Set all bin checked
        m_pScaleColorWidget->binScaleColor->SetAllBinFilterState();
    }
}


/******************************************************************************!
 * \fn onPaintGL
 * \brief Slot for paint "events"
 *
 * Actual openGL commands for drawing box are performed
 ******************************************************************************/
void DrillDataMining3D::onPaintGL()
{
    renderScene();

    // If selections made, repaint them in custom color...
    if (m_lstSelections.isEmpty() == false)
    {
        // Repaint selected objects...
        int   x, y;
        float fHeight;
        Vec   DiePosition3D;
        float fRadius = m_fObjectCellSize / landscapeSize;  // x% of cell size.

        QList<DrillObjectCell>::iterator itBegin = m_lstSelections.begin();
        QList<DrillObjectCell>::iterator itEnd   = m_lstSelections.end();

        while (itBegin != itEnd)
        {
            x = (*itBegin).index() % landscapeSize;
            y = (*itBegin).index() / landscapeSize;
            fHeight = landscape[x][y];

            // If we are processing real test, check if die matches with bounds
            if (m_drillDataInfo.isFakeTest() || fHeight != GEX3D_EMPTY_CELL)
            {
                DiePosition3D = Vec((100.0 * x) / (float) landscapeSize,
                                    (100.0 * y) / (float) landscapeSize, 0.0);

                // Plot object...in custom color
                // Clear index to make sure we only paint the object...
                // not any connection between points!
                iRenderIndex = 0;
                renderObject(&DiePosition3D,
                             x, y, GEX3D_SELECT_COLOR, fRadius, fHeight);
            }

            itBegin++;
        }
    }
}

/******************************************************************************!
 * \fn onSelectionDone
 ******************************************************************************/
void DrillDataMining3D::onSelectionDone(const QPoint&  /*ptPoint*/)
{
    long nObjectChosenID = m_pViewer->selectedName();

//    if (nObjectChosenID < 0)
//    {
//        resetSelection();

//        return;
//    }

    // Erase all previous selection, only keep this one.
    m_lstSelections.clear();

    // Search the run associated with the die
    DrillObjectCell dieObject;

    if (findSelectedDie(nObjectChosenID, dieObject))
    {
        // Single object to keep in the Selection list.
        m_lstSelections.append(dieObject);
    }

    // Display selected die properties
    OnProperties();

    // Show interactive table
    OnShowTestsExecutedInDie();
}

/******************************************************************************!
 * \fn onContextualMenu
 ******************************************************************************/
void DrillDataMining3D::onContextualMenu(const QPoint&  /*ptPoint*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: on contextual menu...");

    if (m_pViewer && m_pViewer->selectedName() > 0 && m_drillDataInfo.currentTestCell())
    {
        // Search the run associated with the die
        DrillObjectCell dieObject;

        if (findSelectedDie(m_pViewer->selectedName(), dieObject))
        {
            // If die already selected,
            // Do not erase the current selection
            if (! isSelectedDie(m_pViewer->selectedName(), dieObject))
            {
                // Erase all previous selection, only keep this one.
                resetSelection(false);

                // Single object to keep in the Selection list.
                m_lstSelections.append(dieObject);
            }

            // Display contextual menu
            QMenu menuContextual;
            QIcon iconRemove(*pixRemove);

            if (!GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
            {
                if(GS::Gex::OFR_Controller::GetInstance()->IsReportEmpty())
                {
                    menuContextual.addAction(QIcon(":/gex/icons/ofr_icon.png"),
                                             "Start a report builder session",
                                             this,
                                             SLOT(OnAddNewOFRSection()));
                }
                else
                {

                    GS::Gex::OFR_Controller* lOFRInstance = GS::Gex::OFR_Controller::GetInstance();

                    QAction* lLastUpdateAction = 0;
                    QAction* lNewSection = 0;
                    QList<QAction*> actionSections;
                    QMenu* lMenuSection =lOFRInstance->BuildContextualMenu(&menuContextual, &lLastUpdateAction, &lNewSection, actionSections);
                    if(lMenuSection)
                    {
                        lMenuSection->setIcon(QIcon(":/gex/icons/ofr_icon.png"));
                        if(lLastUpdateAction)
                        {
                            connect(lLastUpdateAction, SIGNAL(triggered(bool)), this, SLOT(OnAddToLastUpdatedOFRSection()));
                            lMenuSection->addAction(lLastUpdateAction);
                        }

                        if(lNewSection)
                        {
                            connect(lNewSection, SIGNAL(triggered(bool)),this,  SLOT(OnAddNewOFRSection()));
                            lMenuSection->addAction(lNewSection);
                        }

                        lMenuSection->addSeparator();
                        QList<QAction*>::iterator lIterBegin(actionSections.begin()), lIterEnd(actionSections.end());
                        for(;lIterBegin != lIterEnd; ++lIterBegin)
                        {
                            lMenuSection->addAction(*lIterBegin);
                        }
                        connect(lMenuSection, SIGNAL(triggered(QAction*)), this, SLOT(OnAddToAnExistingOFRSection(QAction*)));

                        menuContextual.addMenu(lMenuSection);
                    }
                }
            }
            // Changing binning allowed on bin wafermap.
            if (pGexMainWindow->pWizardSettings->iDrillSubType == GEX_WAFMAP_HARDBIN ||
                    pGexMainWindow->pWizardSettings->iDrillSubType == GEX_WAFMAP_SOFTBIN)
            {
                QIcon iconWafermap(*pixWafermap);
                menuContextual.addAction(iconWafermap, "Change binning", this, SLOT(onChangeBinning()));
            }

            // Add item to remove all seclected parts
            // (only if multiselection has been done)
            if (m_lstSelections.count() > 1)
            {
                menuContextual.addAction(iconRemove, "Remove selected parts", this, SLOT(onRemovePart()));
            }
            else
            {
                // Add item to remove the selected part
                menuContextual.addAction(iconRemove, "Remove part", this, SLOT(onRemovePart()));
            }

            menuContextual.setMouseTracking(true);
            menuContextual.exec(QCursor::pos());
            menuContextual.setMouseTracking(false);
        }
    }
}


void DrillDataMining3D::OnAddToLastUpdatedOFRSection()
{
     AddToLastUpdatedOFRSection();
}

void DrillDataMining3D::OnAddToAnExistingOFRSection(QAction* action)
{
    QVariant lData = action->data();
    if(lData.isValid())
    {
        int lIndexSection = lData.toInt();
        AddToAnExistingOFRSection(lIndexSection);
    }
}

void DrillDataMining3D::OnAddNewOFRSection()
{
    AddNewOFRSection();
}


void DrillDataMining3D::AddToLastUpdatedOFRSection()
{
    QJsonObject lElt;
    toJson(lElt);
    GS::Gex::OFR_Controller::GetInstance()->AddElementToSection
            (GS::Gex::OFR_Controller::GetInstance()->GetLastIndexSectionUpdated(), lElt);
}

void DrillDataMining3D::AddToAnExistingOFRSection(int indexSection)
{
    QJsonObject lElt;
    toJson(lElt);
    GS::Gex::OFR_Controller::GetInstance()->AddElementToSection(indexSection, lElt);
}

void DrillDataMining3D::AddNewOFRSection()
{
    QJsonObject lElt;
    toJson(lElt);
    GS::Gex::OFR_Controller::GetInstance()->AddElementSettings(lElt);
}

void DrillDataMining3D::toJson(QJsonObject& lElt)
{
    QJsonObject lSettings;
    // Export all settings in the settings structure
    //-- Create test infos
    CTest* lTestCell = m_drillDataInfo.currentTestCell();
    if (!lTestCell)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Empty test for the current wafermap");
        return;
    }


    Test lTest;
    lTest.mNumber   = lTestCell->GetTestNumber();
    lTest.mName     = lTestCell->GetTestName();
    lTest.mPinIndex = lTestCell->GetPinIndex();
    lTest.mGroupId  = m_drillDataInfo.groupID();
    lTest.mFileIndex= m_drillDataInfo.fileID();

    lSettings.insert("Test", lTest.toJson());

    // Fill parameters
    QJsonObject lParameters;
    if (m_pScaleColorWidget)
    {
        lParameters.insert("HighValueChanged",  m_pScaleColorWidget->highValueChanged());
        lParameters.insert("LowValueChanged",   m_pScaleColorWidget->lowValueChanged());
        lParameters.insert("SpectrumColorLowValue", m_pScaleColorWidget->spectrumColorLowValue());
        lParameters.insert("SpectrumColorHighValue", m_pScaleColorWidget->spectrumColorHighValue());
    }
    lParameters.insert("WaferMapType", pGexMainWindow->pWizardSettings->iDrillSubType);

    if (pGexMainWindow->pWizardSettings->iDrillSubType == GEX_WAFMAP_HARDBIN ||
            pGexMainWindow->pWizardSettings->iDrillSubType == GEX_WAFMAP_SOFTBIN)
    {
        QString lDeactivatedBins;
        QListIterator<CPartInfo*> lstIteratorPartInfo(m_drillDataInfo.file()->pPartInfoList);
        CPartInfo* lPartInfo = 0;
        QList<int> lKnownBins;
        while (lstIteratorPartInfo.hasNext())
        {
            lPartInfo = lstIteratorPartInfo.next();
            if (!lPartInfo)
            {
                GSLOG(SYSLOG_SEV_ERROR, "Empty part info");
            }
            int lBin;
            if (pGexMainWindow->pWizardSettings->iDrillSubType == GEX_WAFMAP_SOFTBIN)
                lBin = lPartInfo->iSoftBin;
            else
                lBin = lPartInfo->iHardBin;
            if (!lKnownBins.contains(lBin))
            {
                lKnownBins.append(lBin);
                if (m_pScaleColorWidget->binScaleColor->BinFilterState(lBin) == Qt::Unchecked)
                {
                    // first element
                    if (lDeactivatedBins == "")
                        lDeactivatedBins = QString::number(lBin);
                    else
                        lDeactivatedBins += "," + QString::number(lBin);
                }
            }
        }
        lParameters.insert("DeactivatedBins", lDeactivatedBins);
    }
    lSettings.insert("Parameters", lParameters);

    //-- export the setting into the builder element
    lElt.insert("Wafermap", lSettings);
}

/******************************************************************************!
 * \fn onChangeBinning
 * \brief Open a dialog box to change die binning
 ******************************************************************************/
void DrillDataMining3D::onChangeBinning()
{
    if (pGexMainWindow->pWizardSettings->iDrillSubType == GEX_WAFMAP_HARDBIN ||
            pGexMainWindow->pWizardSettings->iDrillSubType == GEX_WAFMAP_SOFTBIN)
    {
        long lIndexObject = m_pViewer->selectedName();
        DrillObjectCell dieObject;

        if (lIndexObject > 0 && findSelectedDie(lIndexObject, dieObject))
        {
            bool bOk = false;
            int  nForcedValue  = (int) GEX_C_DOUBLE_NAN;
            int  nDieObjectRun = dieObject.run();
            if (m_drillDataInfo.currentTestCell()->m_testResult.isValidIndex(nDieObjectRun))
            {
                nForcedValue = (int) m_drillDataInfo.currentTestCell()->m_testResult.resultAt(nDieObjectRun);
            }

            // Get user value
            nForcedValue =
                    QInputDialog::getInteger(this, "Edit Binning",
                                             "Enter Bin value (e.g: 14):", nForcedValue,
                                             0, 65535, 1, &bOk);

            m_drillDataInfo.currentTestCell()->m_testResult.forceResultAt(dieObject.run(), nForcedValue);

            fillViewer();
        }
    }
}

/******************************************************************************!
 * \fn onRemovePart
 * \brief Remove all data samples in selected part#
 ******************************************************************************/
void DrillDataMining3D::onRemovePart()
{
    if (m_drillDataInfo.isValid())
    {
        // Find the X,Y die coordinates for this part...
        CTest* ptCellDieX;
        CTest* ptCellDieY;

        if (m_drillDataInfo.file()->
                FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,
                             GEX_PTEST, &ptCellDieX, true, false) != 1)
        {
            return;
        }

        if (m_drillDataInfo.file()->
                FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,
                             GEX_PTEST, &ptCellDieY, true, false) != 1)
        {
            return;
        }

        // Create list of parts located at this die location
        QList <long> cPartList;
        int iDieX = GEX_WAFMAP_INVALID_COORD;
        int iDieY = GEX_WAFMAP_INVALID_COORD;
        int nBegin;
        int nEnd;

        // Find begin and end offset for this sublot
        m_drillDataInfo.currentTestCell()->
                findSublotOffset(nBegin, nEnd, m_drillDataInfo.fileID());

        int nMinCellTestCount =
                gex_min(ptCellDieX->m_testResult.count(),
                        ptCellDieY->m_testResult.count());
        int nCurrentSelectionRun = 0;
        GEX_ASSERT(nEnd > nMinCellTestCount);

        for (int nIndex = 0; nIndex < m_lstSelections.count(); nIndex++)
        {
            nCurrentSelectionRun = m_lstSelections.at(nIndex).run();
            if (! ((nCurrentSelectionRun >= 0) &&
                   (nCurrentSelectionRun < nMinCellTestCount)))
            {
                continue;
            }

            iDieX = (int) ptCellDieX->m_testResult.resultAt(nCurrentSelectionRun);
            iDieY = (int) ptCellDieY->m_testResult.resultAt(nCurrentSelectionRun);

            for (long lSample = nBegin; lSample < nEnd; lSample++)
            {
                if ((iDieX == ptCellDieX->m_testResult.resultAt(lSample)) &&
                        (iDieY == ptCellDieY->m_testResult.resultAt(lSample)))
                {
                    cPartList.append(lSample);
                }
            }
        }

        // Ask confirmation
        QString strText;

        if (m_lstSelections.count() > 1)
        {
            strText = "Confirm to remove tests results for selected dies ?";
        }
        else
        {
            strText = "Confirm to remove test results for die (" +
                    QString::number(iDieX) + "," + QString::number(iDieY) + ")?";

            if (cPartList.count() > 1)
            {
                strText += "\nNote: Die tested " +
                        QString::number(cPartList.count()) + " times.";
            }
        }

        bool lOk;
        GS::Gex::Message::request("", strText, lOk);
        if (! lOk)
        {
            return;
        }

        // Remove all results
        m_drillDataInfo.group()->removeParts(cPartList);

        // Force to reload Statistics table with updated counts
        if (pGexMainWindow->LastCreatedWizardTable() != NULL)
        {
            pGexMainWindow->LastCreatedWizardTable()->clear();
            pGexMainWindow->LastCreatedWizardTable()->ShowTable("");
        }

        // Erase all selections
        resetSelection();

        // Redraw wafermap
        fillViewer();
    }
}

/******************************************************************************!
 * \fn crossProduct
 * \brief Calculate the vector cross product of v and w, store result in n
 ******************************************************************************/
static void crossProduct(double v[3], double w[3], double n[3])
{
    n[0] = v[1] * w[2] - w[1] * v[2];
    n[1] = w[0] * v[2] - v[0] * w[2];
    n[2] = v[0] * w[1] - w[0] * v[1];
}


/******************************************************************************!
 * \fn createLandscape
 * \brief Allocates Grid buffer to hold all points to draw
 ******************************************************************************/
void DrillDataMining3D::createLandscape(int size, int iPlans)
{
    // Destroy previous grid if any.
    destroyLandscape();

    // Reset object rendering index
    iRenderIndex = 0;

    // Make sure size is a power of 2
    if ((size % 2) != 0)
    {
        size++;
    }
    landscapeSize = size;

    // Number of buffer plans to allocate
    if (iPlans <= 0)
    {
        landscapePlans = size;
    }
    else
    {
        landscapePlans = iPlans;
    }

    // Create the array size*size cells
    landscape = new double*[landscapeSize + 1];
    for (int i = 0; i <= landscapeSize; i++)
    {
        landscape[i] = new double[landscapePlans + 1];
        for (int j = 0; j < landscapePlans; j++)
        {
            landscape[i][j] = GEX3D_EMPTY_CELL;
        }
    }
}

/******************************************************************************!
 * \fn destroyLandscape
 * \brief Cleanup: Erase any existing grid
 ******************************************************************************/
void DrillDataMining3D::destroyLandscape()
{
    // Erase arrays allocated
    if (landscape != NULL)
    {
        for (int i = 0; i <= landscapeSize; i++)
        {
            delete[] landscape[i];
        }
        delete[] landscape;
    }
    landscape = NULL;

    // Destroy normals if Surface rendering was done...
    destroyNormals();
}

/******************************************************************************!
 * \fn destroyNormals
 * \brief Cleanup: Erase Normals array used in Surface rendering
 ******************************************************************************/
void DrillDataMining3D::destroyNormals()
{
    // If rendering was Surface, also erase Normals arrays
    if (normals != NULL)
    {
        for (int i = 0; i <= landscapeSize; i++)
        {
            delete[] normals[i];
            delete[] vertexNormals[i];
        }
        delete[] normals;
        delete[] vertexNormals;
    }
    normals = NULL;
    vertexNormals = NULL;
}

/******************************************************************************!
 * \fn calculateVertexNormals
 * \brief Computes Normals vectors
 ******************************************************************************/
void DrillDataMining3D::calculateVertexNormals()
{
    double len, v[3], v2[3], w[3], w2[3], n[3], n2[3];

    // Allocate buffer if it doesn't already exist...
    if (normals == NULL)
    {
        normals = new gridNormals*[landscapeSize + 1];
        vertexNormals = new avgNormals*[landscapeSize + 1];
        for (int i = 0; i <= landscapeSize; i++)
        {
            normals[i] = new gridNormals[landscapePlans + 1];
            vertexNormals[i] = new avgNormals[landscapePlans + 1];

            memset(normals[i], 0, landscapePlans * sizeof(gridNormals));
            memset(vertexNormals[i], 0, landscapePlans * sizeof(avgNormals));
        }
    }

    // Calculate the surface normals for all polygons in the
    // height field
    for (int i = 0; i < (landscapeSize - 1); i++)
    {
        for (int k = 0; k < (landscapePlans - 1); k++)
        {
            /* Lower poly normal */
            v[0] = 1;  // (i+1)-i
            v[1] = 0;  // k-k
            v[2] = landscape[i + 1][k] - landscape[i][k];
            w[0] = 1;  // (i+1)-i
            w[1] = 1;  // (k+1)-k
            w[2] = landscape[i + 1][k + 1] - landscape[i][k];
            crossProduct(v, w, n);
            len = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
            normals[i][k].l[0] = n[0] / len;
            normals[i][k].l[1] = n[1] / len;
            normals[i][k].l[2] = n[2] / len;

            /* Upper poly normal */
            v2[0] = -1.0;  // i-(i+1);
            v2[1] = 0.0;  // (k+1)-(k+1);
            v2[2] = landscape[i][k + 1] - landscape[i + 1][k + 1];
            w2[0] = -1.0;  // i-(i+1);
            w2[1] = -1.0;  // k-(k+1);
            w2[2] = landscape[i][k] - landscape[i + 1][k + 1];
            crossProduct(v2, w2, n2);
            len = sqrt(n2[0] * n2[0] + n2[1] * n2[1] + n2[2] * n2[2]);
            normals[i][k].u[0] = n2[0] / len;
            normals[i][k].u[1] = n2[1] / len;
            normals[i][k].u[2] = n2[2] / len;
        }
    }
    // Calculate proper vertex normals
    averageNormals();
}

/******************************************************************************!
 * \fn averageNormals
 * \brief Computes the Average Normal vectors
 ******************************************************************************/
void DrillDataMining3D::averageNormals()
{
    // Calculate the average surface normal for a vertex based on
    // the normals of the surrounding polygons
    for (int i = 0; i < landscapeSize; i++)
    {
        for (int k = 0; k < landscapePlans; k++)
        {
            if (i > 0 && k > 0 &&
                    i < (landscapeSize - 1) &&
                    k < (landscapePlans - 1))
            {
                // For vertices that are *not* on the edge of the height field
                for (int t = 0; t < 3; t++)  // X, Y and Z components
                {
                    vertexNormals[i][k].n[t] =
                            (normals[i][k].u[t] +
                             normals[i][k].l[t] +
                             normals[i][k - 1].u[t] +
                            normals[i - 1][k - 1].u[t] +
                            normals[i - 1][k - 1].l[t] +
                            normals[i - 1][k].l[t]) / 6.0;
                }
            }
            else
            {
                // Vertices that are on the edge of the height field require
                // special attention..
                if (i == 0 && k == 0)
                {
                    for (int t = 0; t < 3; t++)
                    {
                        vertexNormals[i][k].n[t] = (normals[i][k].u[t] +
                                                    normals[i][k].l[t]) / 2.0;
                    }
                }
                else if (i == landscapeSize - 1 && k == landscapeSize - 1)
                {
                    for (int t = 0; t < 3; t++)
                    {
                        vertexNormals[i][k].n[t] = (normals[i][k].u[t] +
                                                    normals[i][k].l[t]) / 2.0;
                    }
                }
                else if (i == landscapeSize - 1)
                {
                    for (int t = 0; t < 3; t++)
                    {
                        vertexNormals[i][k].n[t] = vertexNormals[i - 1][k].n[t];
                    }
                }
                else if (k == landscapeSize - 1)
                {
                    for (int t = 0; t < 3; t++)
                    {
                        vertexNormals[i][k].n[t] = vertexNormals[i][k - 1].n[t];
                    }
                }
                else if (k > 0)
                {
                    for (int t = 0; t < 3; t++)
                    {
                        vertexNormals[i][k].n[t] = (normals[i][k].u[t] +
                                                    normals[i][k].l[t] +
                                                    normals[i][k - 1].u[t]) / 3.0;
                    }
                }
                else if (i > 0)
                {
                    for (int t = 0; t < 3; t++)
                    {
                        vertexNormals[i][k].n[t] = (normals[i][k].u[t] +
                                                    normals[i][k].l[t] +
                                                    normals[i - 1][k].l[t]) / 3.0;
                    }
                }
            }
        }
    }
}

/******************************************************************************!
 * \fn SetRenderingMode
 * \brief Set Rendering Mode
 ******************************************************************************/
void DrillDataMining3D::SetRenderingMode()
{
    switch (m_RenderMode)
    {
    case SolidBars:
    case OutlinedBars:
    case SolidPoints:
    case OutlinedPoints:
    case SimpleLines:
    case FlatBars:
        glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
        break;
    case Wireframe:
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_NORMALIZE);
        glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
        break;
    case SmoothShaded:
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_BLEND);
        glShadeModel(GL_SMOOTH);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, true);
        break;
    }

    if (m_drillDataInfo.file() != NULL &&
        m_drillDataInfo.file()->getWaferMapData().bStripMap)
    {
        m_fObjectCellSize = 70.0;  // Strip Map: show each die as 70% of full size
    }
    else
    {
        m_fObjectCellSize = 90.0;  // Wafermap
    }
}


/******************************************************************************!
 * \fn drawSmoothShaded
 * \brief Draw Object: Rendering mode is SMOOTH SURFACE
 ******************************************************************************/
void
DrillDataMining3D::drawSmoothShaded(const Vec&    cOrigin,
                                    float         fColor,
                                    int           x,
                                    int           y,
                                    const QColor& rgbColor  /*= QColor()*/)
{
    float fRadius = 100.0 / landscapeSize;  // Draw as 100% of cube space size

    // Set Drawing color
    SetPlotColor(fColor, rgbColor);

    glPushMatrix();
    glTranslatef(cOrigin.x, cOrigin.y, cOrigin.z);

    // Draw the Surface
    glBegin(GL_POLYGON);
    glNormal3dv(vertexNormals[x][y].n);
    glVertex3f(0, 0, landscape[x][y]);

    glNormal3dv(vertexNormals[x + 1][y].n);
    glVertex3f(fRadius, 0, landscape[x + 1][y]);

    glNormal3dv(vertexNormals[x + 1][y + 1].n);
    glVertex3f(fRadius, fRadius, landscape[x + 1][y + 1]);
    glEnd();

    glBegin(GL_POLYGON);
    glNormal3dv(vertexNormals[x][y].n);
    glVertex3f(0, 0, landscape[x][y]);

    glNormal3dv(vertexNormals[x + 1][y + 1].n);
    glVertex3f(fRadius, fRadius, landscape[x + 1][y + 1]);

    glNormal3dv(vertexNormals[x][y + 1].n);
    glVertex3f(0, fRadius, landscape[x][y + 1]);
    glEnd();

    glPopMatrix();
}

/******************************************************************************!
 * \fn OnShowTestsExecutedInDie
 * \brief Allow user to display tests executed for given die
 ******************************************************************************/
void DrillDataMining3D::OnShowTestsExecutedInDie()
{
    // If no object, nothing to do!
    if (m_lstSelections.isEmpty())
    {
        return;
    }

    // If multiple groups, do not display the PartsResult interactive table
    if (gexReport && gexReport->getGroupsList().count() > 1)
    {
        return;
    }

    // Find LAST-Run# associated with this die location...
    if (m_lstSelections.first().run() >= 0)
    {
        // Force back to have Interactive Table as window with focus.
        pGexMainWindow->ShowWizardDialog(GEX_TABLE_WIZARD_P1, false);

        // Ensure Table tab is fully loaded
        pGexMainWindow->LastCreatedWizardTable()->ShowTable("drill_table=parts_results");

        // Show Interactive table with selected run#.
        pGexMainWindow->LastCreatedWizardTable()->OnShowDeviceResults(m_lstSelections.first().run(),
                                                          m_lstSelections.first().dieX(),
                                                          m_lstSelections.first().dieY());
    }
}

/******************************************************************************!
 * \fn findSelectedDie
 * \brief Find the die from the selected object
 ******************************************************************************/
bool DrillDataMining3D::findSelectedDie(int nIndexObject, DrillObjectCell& dieObjectCell)
{
    // Check if invalid object
    if (nIndexObject < 0)
    {
        return false;
    }

    // Convert to landscape X,Y coordinates
    int iChipY = nIndexObject / landscapeSize;
    int iChipX = nIndexObject % landscapeSize;

    if (landscape[iChipX][iChipY] != GEX3D_EMPTY_CELL &&
            (m_eMode == DrillDataMining3D::standard ||
             m_lstNNRDie.contains(nIndexObject) == true))
    {
        // Turn wafermap according X and Y pos direction
        if (m_drillDataInfo.file()->getWaferMapData().GetPosXDirection() == false)
        {
            iChipX = landscapeSize - iChipX - 1  -
                    (landscapeSize - m_drillDataInfo.file()->getWaferMapData().SizeX);
        }

        if (m_drillDataInfo.file()->getWaferMapData().GetPosYDirection() == true)
        {
            iChipY = landscapeSize - iChipY - 1  -
                    (landscapeSize - m_drillDataInfo.file()->getWaferMapData().SizeY);
        }

        // Convert to WaferMap physical X,Y coordinates.
        iChipX += m_drillDataInfo.file()->getWaferMapData().iLowDieX;
        iChipY += m_drillDataInfo.file()->getWaferMapData().iLowDieY;

        // Find LAST-Run# associated with this die location...
        if (m_drillDataInfo.isValid())
        {
            long lSelectedRun = m_drillDataInfo.file()->
                    findRunNumber(iChipX,
                                  iChipY,
                                  m_drillDataInfo.currentTestCell());

            if (lSelectedRun != -1)
            {
                dieObjectCell =
                        DrillObjectCell(nIndexObject, lSelectedRun, iChipX, iChipY);
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

/******************************************************************************!
 * \fn isSelectedDie
 * \brief Find if a die is already selected
 ******************************************************************************/
bool DrillDataMining3D::isSelectedDie(int  /*nIndex*/,
                                      const DrillObjectCell& dieCell)
{
    for (int iIndex = 0; iIndex < m_lstSelections.count(); iIndex++)
    {
        if (dieCell == m_lstSelections.at(iIndex))
        {
            return true;
        }
    }

    return false;
}

/******************************************************************************!
 * \fn OnFindDie
 * \brief Allow user to define which die X,Y to highlight
 ******************************************************************************/
void DrillDataMining3D::OnFindDie()
{
    PickPartDialog cPickPart;

    // Load list of runs & dies in group.
    if (cPickPart.setGroupAndFileID(m_drillDataInfo.groupID(),
                                    m_drillDataInfo.fileID()) == false)
    {
        return;
    }

    if (cPickPart.exec() != 1)
    {
        return;
    }

    // Get list of dies (GUI raw#3) selected
    QString strDieList = cPickPart.getPartsList(4, ';');

    // Split die list by dieXY
    QStringList strlistDies;
    QString     strField;
    int nIndex;
    int iDieX, iDieY;

    // Extract each test from list
    strlistDies = strDieList.split(";",QString::SkipEmptyParts);;
    for (nIndex = 0; nIndex < strlistDies.count(); nIndex++)
    {
        // Get die location to select
        strField = strlistDies[nIndex];

        iDieX = strField.section(",", 0, 0).toInt();
        iDieY = strField.section(",", 1, 1).toInt();

        // Add selection to active list
        SelectDieLocation(m_drillDataInfo.file(), iDieX, iDieY, (nIndex == 0));
    }

    // Display selected die properties
    OnProperties();

    // Show interactive table
    OnShowTestsExecutedInDie();
}

/******************************************************************************!
 * \fn ShowBinProperties
 * \brief Show BINNING properties of the Die/Part clicked (HBIN,SBIN)
 ******************************************************************************/
void DrillDataMining3D::ShowBinProperties(int iDrillReport)
{
    CTest*     ptTestCell  = 0;  // Pointer to test cell
    CPartInfo* ptrPartInfo = 0;
    int     iRetestCount   = 0; // Tracks retest info.
    int     nDieX = 0, nDieY = 0;
    QString strBinName;
    QString strMessage;

    // Scan list of selections.
    for (int nIndex = 0; nIndex < m_lstSelections.count(); nIndex++)
    {
        // Convert to landscape X,Y coordinates
        if (landscapeSize != 0)
        {
            nDieY = m_lstSelections.at(nIndex).index() / landscapeSize;
        }
        else
        {
            GSLOG(SYSLOG_SEV_WARNING, "Show Bin Properties : Avoiding division by zero");
        }
        nDieX = m_lstSelections.at(nIndex).index() % landscapeSize;

        if (iDrillReport == GEX_DRILL_WAFERPROBE)
        {
            // Turn wafermap according X and Y pos direction
            if (m_drillDataInfo.file()->getWaferMapData().GetPosXDirection() == false)
            {
                nDieX = landscapeSize - nDieX - 1  -
                        (landscapeSize -
                         m_drillDataInfo.file()->getWaferMapData().SizeX);
            }

            if (m_drillDataInfo.file()->getWaferMapData().GetPosYDirection() == true)
            {
                nDieY = landscapeSize - nDieY - 1  -
                        (landscapeSize -
                         m_drillDataInfo.file()->getWaferMapData().SizeY);
            }

            // Convert to WaferMap physical X,Y coordinates.
            nDieX += m_drillDataInfo.file()->getWaferMapData().iLowDieX;
            nDieY += m_drillDataInfo.file()->getWaferMapData().iLowDieY;
        }

        // Find PartInfo for this specific part
        QListIterator<CPartInfo*>
                lstIteratorPartInfo(m_drillDataInfo.file()->pPartInfoList);

        while (lstIteratorPartInfo.hasNext())
        {
            ptrPartInfo = lstIteratorPartInfo.next();

            if ((ptrPartInfo->iDieX == nDieX) && (ptrPartInfo->iDieY == nDieY))
            {
                if (iRetestCount == 0)
                {
                    // If first test, show Die location.
                    strMessage = "( " +
                            QString::number(nDieX) + " , " +
                            QString::number(nDieY) + " )";
                    if (iDrillReport == GEX_DRILL_WAFERPROBE)
                    {
                        WriteInfoLine("Part (X,Y)", strMessage);
                    }
                }
                else
                {
                    strMessage = "Retest#" + QString::number(iRetestCount);
                    WriteInfoLine(strMessage.toLatin1().constData(),
                                  "........", true, true);
                }

                // Do NOT display the PartID if we have multiple groups because
                // this information is innacurate (it it the Run# in the group,
                // not in the stacked groups & wafermap) !
                if (gexReport && gexReport->getGroupsList().count() == 1)
                {
                    WriteInfoLine("PartID", ptrPartInfo->getPartID());
                }

                strMessage = QString::number(ptrPartInfo->m_site) + " (Head: " +
                        QString::number(ptrPartInfo->bHead) + ")";
                WriteInfoLine("Site", strMessage);

                // Soft Bin#
                strMessage = QString::number(ptrPartInfo->iSoftBin);
                strBinName =
                        m_drillDataInfo.group()->GetBinName(true, ptrPartInfo->iSoftBin);

                if (strBinName.isEmpty() == false)
                {
                    strMessage += "<br>" + strBinName;
                }
                WriteInfoLine("SoftBin", strMessage);

                // Hard Bin#
                strMessage = QString::number(ptrPartInfo->iHardBin);
                strBinName =
                        m_drillDataInfo.group()->GetBinName(false, ptrPartInfo->iHardBin);

                if (strBinName.isEmpty() == false)
                {
                    strMessage += "<br>" + strBinName;
                }
                WriteInfoLine("HardBin", strMessage);

                WriteInfoLine("Tests done",
                              QString::number(ptrPartInfo->iTestsExecuted));
                if (ptrPartInfo->lExecutionTime > 0)
                {
                    float fTime = ((float) ptrPartInfo->lExecutionTime) / 1000.0f;
                    strMessage = QString::number(fTime) + " sec.";
                    WriteInfoLine("Exec. time", strMessage);
                }

                // Part info if exist...
                if (ptrPartInfo->getPartText().isEmpty() == false)
                {
                    WriteInfoLine("Part text", ptrPartInfo->getPartText());
                }
                if (ptrPartInfo->strPartRepairInfo.isEmpty() == false)
                {
                    WriteInfoLine("Repair text", ptrPartInfo->strPartRepairInfo);
                }

                // Check if Failing test info to list
                if (ptrPartInfo->iFailTestNumber != (unsigned) -1)
                {
                    // Show all details about the failing test!
                    if (m_drillDataInfo.file()->
                            FindTestCell(ptrPartInfo->iFailTestNumber,
                                         ptrPartInfo->iFailTestPinmap,
                                         &ptTestCell, false, false) != 1)
                    {
                        break;
                    }
                    WriteInfoLine("Failed test",
                                  ptTestCell->strTestName.toLatin1().constData(), true,
                                  true);
                    WriteInfoLine("Test#", ptTestCell->szTestLabel);
                    WriteInfoLine("Low L.", ptTestCell->GetCurrentLimitItem()->szLowL);
                    WriteInfoLine("High L.", ptTestCell->GetCurrentLimitItem()->szHighL);
                    WriteInfoLine("Value", m_drillDataInfo.file()->
                                  FormatTestResult(ptTestCell,
                                                   ptrPartInfo->fFailingValue,
                                                   ptTestCell->res_scal), true, true);
                    WriteInfoLine("Statistics", "........");
                    WriteInfoLine("Execs.",
                                  gexReport->CreateResultString(ptTestCell->ldExecs));
                    WriteInfoLine("Fails",
                                  gexReport->CreateResultString(ptTestCell->GetCurrentLimitItem()->ldFailCount));
                    WriteInfoLine("Removed",
                                  gexReport->CreateResultString(ptTestCell->GetCurrentLimitItem()->ldOutliers));
                    WriteInfoLine("Mean",
                                  m_drillDataInfo.file()->
                                  FormatTestResult(ptTestCell,
                                                   ptTestCell->lfMean,
                                                   ptTestCell->res_scal));
                    WriteInfoLine("Sigma",
                                  m_drillDataInfo.file()->
                                  FormatTestResult(ptTestCell,
                                                   ptTestCell->lfSigma,
                                                   ptTestCell->res_scal));
                    WriteInfoLine("Min",
                                  m_drillDataInfo.file()->
                                  FormatTestResult(ptTestCell,
                                                   ptTestCell->lfMin,
                                                   ptTestCell->res_scal));
                    WriteInfoLine("Max",
                                  m_drillDataInfo.file()->
                                  FormatTestResult(ptTestCell,
                                                   ptTestCell->lfMax,
                                                   ptTestCell->res_scal));
                    WriteInfoLine("Range",
                                  m_drillDataInfo.file()->
                                  FormatTestResult(ptTestCell,
                                                   ptTestCell->lfRange,
                                                   ptTestCell->res_scal));
                    WriteInfoLine("Cp",
                                  gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));
                    WriteInfoLine("Cpk",
                                  gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));
                }
                else if (ptrPartInfo->iSoftBin != 1)
                {
                    WriteInfoLine("Fail info", GEX_NA);
                }

                // Keep track of the total number of retest!
                iRetestCount++;
            }
        }  // while

        WriteInfoLine(NULL, "-");
    }
}

/******************************************************************************!
 * \fn ShowTestProperties
 * \brief Show properties of the Die clicked (Test value)
 ******************************************************************************/
void DrillDataMining3D::ShowTestProperties(int iDrillReport)
{
    CPartInfo* ptrPartInfo;
    int     iRetestCount = 0; // Tracks retest info.
    int     nDieX, nDieY;
    QString strMessage;

    // Scan list of selections.
    for (int nIndex = 0; nIndex < m_lstSelections.count(); nIndex++)
    {
        // Convert to landscape X,Y coordinates
        nDieY = m_lstSelections.at(nIndex).index() / landscapeSize;
        nDieX = m_lstSelections.at(nIndex).index() % landscapeSize;

        if (iDrillReport == GEX_DRILL_WAFERPROBE)
        {
            // Turn wafermap according X and Y pos direction
            if (m_drillDataInfo.file()->getWaferMapData().GetPosXDirection() == false)
            {
                nDieX = landscapeSize - nDieX - 1  -
                        (landscapeSize -
                         m_drillDataInfo.file()->getWaferMapData().SizeX);
            }

            if (m_drillDataInfo.file()->getWaferMapData().GetPosYDirection() == true)
            {
                nDieY = landscapeSize - nDieY - 1  -
                        (landscapeSize -
                         m_drillDataInfo.file()->getWaferMapData().SizeY);
            }

            // Convert to WaferMap physical X,Y coordinates.
            nDieX += m_drillDataInfo.file()->getWaferMapData().iLowDieX;
            nDieY += m_drillDataInfo.file()->getWaferMapData().iLowDieY;
        }

        // Find PartInfo for this specific part
        QListIterator<CPartInfo*>
                lstIteratorPartInfo(m_drillDataInfo.file()->pPartInfoList);

        while (lstIteratorPartInfo.hasNext())
        {
            ptrPartInfo = lstIteratorPartInfo.next();

            if (((ptrPartInfo->iDieX == nDieX) && (ptrPartInfo->iDieY == nDieY)))
            {
                if (iRetestCount == 0)
                {
                    // If first test, show Die location.
                    strMessage = "( " +
                            QString::number(nDieX) + " , " +
                            QString::number(nDieY) + " )";
                    if (iDrillReport == GEX_DRILL_WAFERPROBE)
                    {
                        WriteInfoLine("Part (X,Y)", strMessage);
                    }
                }
                else
                {
                    strMessage = "Retest#" + QString::number(iRetestCount);
                    WriteInfoLine(strMessage.toLatin1().constData(),
                                  "........", true, true);
                }

                // Do NOT display the PartID if we have multiple groups because
                // this information is innacurate (it it the Run# in the group,
                // not in the stacked groups & wafermap) !
                if (gexReport && gexReport->getGroupsList().count() == 1)
                {
                    WriteInfoLine("PartID", ptrPartInfo->getPartID());
                }

                strMessage =
                        QString::number(ptrPartInfo->m_site) + " (Head: " +
                        QString::number(ptrPartInfo->bHead) + ")";
                WriteInfoLine("Site", strMessage);
                WriteInfoLine("SoftBin", QString::number(ptrPartInfo->iSoftBin));
                WriteInfoLine("HardBin", QString::number(ptrPartInfo->iHardBin));
                if (ptrPartInfo->getPartText().isEmpty() == false)
                {
                    WriteInfoLine("Part text", ptrPartInfo->getPartText());
                }
                if (ptrPartInfo->strPartRepairInfo.isEmpty() == false)
                {
                    WriteInfoLine("Repair text", ptrPartInfo->strPartRepairInfo);
                }

                double lfValue = GEX_C_DOUBLE_NAN;
                int    nCurrentSelectionRun = m_lstSelections.at(nIndex).run();
                if (m_drillDataInfo.currentTestCell()->m_testResult.isValidIndex(
                            nCurrentSelectionRun))
                {
                    lfValue = m_drillDataInfo.currentTestCell()->m_testResult.resultAt(
                                nCurrentSelectionRun) * ScalingPower(
                                m_drillDataInfo.currentTestCell()->res_scal);
                }

                // Show all details about the test analysed...
                WriteInfoLine("Test value",
                              m_drillDataInfo.file()->
                              FormatTestResult(m_drillDataInfo.currentTestCell(),
                                               lfValue,
                                               m_drillDataInfo.currentTestCell()->
                                               res_scal));

                // Keep track of the total number of retest!
                iRetestCount++;
            }
        }

        // After all entries listed, show Test statistics
        WriteTestStatisticsDetails(
                    m_drillDataInfo.currentTestCell(), m_drillDataInfo.file());

        WriteInfoLine(NULL, "-");
    }
}

/******************************************************************************!
 * \fn onFilterParameters
 * \brief Filter the parameters listed in the Test Statistics table
 ******************************************************************************/
void DrillDataMining3D::onFilterParameters(bool bState)
{
    if (bState)
    {
        // Show TestList
        PickTestDialog dialogPickTest;

        // Allow/Disable Multiple selections.
        dialogPickTest.setMultipleSelection(true);
        dialogPickTest.setMultipleGroups(false, false);
        // dialogPickTest.setAllowedTestType(PickTestDialog::TestAll);

        // Prompt dialog box, let user pick tests from the list
        if (dialogPickTest.fillParameterList() && dialogPickTest.exec() ==
                QDialog::Accepted)
        {
            // Get test# selected
            // string format: <Test#>.<Pinmap#> , <Test#>.<Pinmap#> , etc.
            gexReport->setInteractiveTestFilter(dialogPickTest.testItemizedList());
        }
        else
        {
            gexReport->clearInteractiveTestFilter();
        }
    }
    else
    {
        // Remove test filter
        gexReport->clearInteractiveTestFilter();
    }
}

/******************************************************************************!
 * \fn onFilterChanged
 * \brief Update view with parameter list
 ******************************************************************************/
void DrillDataMining3D::onFilterChanged(bool bState)
{
    if (bState)
    {
        m_drillDataInfo.applyTestFilter();
    }

    buttonFilterParameters->setChecked(gexReport->hasInteractiveTestFilter());
}

// if drill allowed
#endif

}  // namespace Gex
