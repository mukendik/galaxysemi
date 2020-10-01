/******************************************************************************!
 * \file drill_chart.cpp
 * \brief Interactive Chart navigator
 ******************************************************************************/
#ifdef _WIN32
#include <windows.h>
#endif

#include "gex_shared.h"
#include "drill_chart.h"
#include "drill_table.h"
#include "drillDataMining3D.h"
#include "engine.h"
#include "settings_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "browser_dialog.h"
#include "browser.h"
#include "picktest_dialog.h"
#include "assistant_flying.h"
#include "getstring_dialog.h"
#include "interactive_charts.h"  // Layer classes, etc
#include "drill_chart_editlayer.h"
#include "drill_editchart_options.h"
#include "cstats.h"
#include "component.h"
#include "report_options.h"
#include "gexhistogramchart.h"
#include "gextrendchart.h"
#include "gexscatterchart.h"
#include "gexboxplotchart.h"
#include "gexprobabilityplotchart.h"
#include "gex_box_whisker_chart.h"
#include "gex_xbar_r_chart.h"
#include "gex_file_in_group.h"
#include "gex_report.h"
#include "gex_group_of_files.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "message.h"
#include "ofr_controller.h"
#include <QBitmap>
#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QToolTip>
#include <QColorDialog>
#include <QPainter>
#include <QMessageBox>

#if defined unix || __MACH__
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>
#endif

const QString PrefixTab = "(D)";
// main.cpp
extern GexMainwindow* pGexMainWindow;

// report_build.cpp
extern CReportOptions ReportOptions;  // Holds options (report_build.h)
extern CGexSkin*      pGexSkin;       // Holds options (report_build.h)

// patman_lib.cpp
extern void patlib_GetTailCutoff(const CTest*  ptTestCell,
                                 bool    bRight,
                                 double* lfLimit,
                                 CPatDefinition *patDef);

// cstats.cpp
extern double ScalingPower(int iPower);


// External definitions for pointers to pixmaps
#include "gex_pixmap_extern.h"

// in report_build.cpp
extern CGexReport* gexReport;  // Handle to report class

#include <QUndoStack>
#include "gex_undo_command.h"

/******************************************************************************!
 * \fn Wizard_DrillChart
 * \brief Assistant link to 2D Drill
 ******************************************************************************/
void GexMainwindow::Wizard_DrillChart(QString strLink, bool fromHTMLBrowser)
{
    bool bRefuse = false;

    // Enable/disable some features...
    if (GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eSzOEM)
    {
        bRefuse = true;
    }

    // case 4768 cleanup

    // Reject Drill?
    if (bRefuse)
    {
        QString m = ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Show wizard page : Interactive Charts
    ShowWizardDialog(GEX_CHART_WIZARD_P1, fromHTMLBrowser);

    if(LastCreatedWizardChart())
    {
        if ( LastCreatedWizardChart()->isDataAvailable() == false)
        {
            GS::Gex::Message::information("", "No data available yet\nSelect your data first ('Home' page)!");

            // Switch to the home page
            OnHome();
            return;
        }

        if(LastCreatedWizardChart())
            LastCreatedWizardChart()->ShowChart(strLink);

        mCanCreateNewDetachableWindow = true;
    }

}

/******************************************************************************!
 * \fn Wizard_DrillChart
 *
 * Needed as table of addresses to function is only of type (void)
 * for parameters...
 ******************************************************************************/
void GexMainwindow::Wizard_DrillChart()
{
    Wizard_DrillChart("");
}

/******************************************************************************!
 * \fn GexWizardChart
 * \brief Dialog box Constructor
 ******************************************************************************/
GexWizardChart::GexWizardChart(QWidget* parent, bool modal, Qt::WFlags fl)
    : GS::Gex::QTitleFor< QDialog >(parent, fl),
      m_eChartType(GexAbstractChart::chartTypeHistogram)
{
    mTabFocusChartType= false;
    m_poLastTest = 0;
    mWizardHandler = 0;
    setupUi(this);
    setModal(modal);
    setObjectName("GSChartGui");

    mChartsInfo = new CGexChartOverlays(this);

    tabXBarR->init(this);
    m_pMultiChartWidget->init(this);
    m_pSingleChartWidget->init(this);

    m_poPATLimitGroupBox->hide();
    mMultipleTest = false;
    // apply gex palette
    if (pGexSkin)
    {
        pGexSkin->applyPalette(this);
    }

    // Fill combo chart type
    comboBoxChartType->clear();
    comboBoxChartType->insertItem(0, QIcon(*pixAdvHisto), "Histogram",QVariant(GexAbstractChart::chartTypeHistogram));
    comboBoxChartType->insertItem(1, QIcon(*pixTrend), "Trend",QVariant(GexAbstractChart::chartTypeTrend));
    comboBoxChartType->insertItem(2, QIcon(*pixProbabilityPlot), "Probability Plot",QVariant(GexAbstractChart::chartTypeProbabilityPlot));
    comboBoxChartType->insertItem(3, QIcon(*pixScatter), "Correlation/Bivariate",QVariant(GexAbstractChart::chartTypeScatter));
    comboBoxChartType->insertItem(4, QIcon(*pixBoxMeanRange), "Box-Plot", QVariant(GexAbstractChart::chartTypeBoxPlot));


    InitToolsButtons();


    if (ReportOptions.getAdvancedReport() != GEX_ADV_CANDLE_MEANRANGE)
    {
        tabWidgetChart->removeTab(tabWidgetChart->indexOf(tabXBarR));
    }

    // Disable some functions under Solaris & HP-UX
#if (defined __sun__ || __hpux__)
    buttonExportExcelClipboard->hide();
    buttonExportChartStatsExcelClipboard->hide();
#endif

    // Accept keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // Enable/disable some features...
    switch (GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
    case GS::LPPlugin::LicenseProvider::eLtxcOEM:  // OEM-Examinator for LTXC
    {
        buttonExportExcelClipboard->hide();
        buttonExportData->hide();
        buttonExportChartStatsExcelClipboard->hide();
        buttonRemoveData->hide();
        // Credence requested backround to be white...
        QPalette palette;
        palette.setColor(backgroundRole(), Qt::white);
        setPalette(palette);
    }
        break;
    default:
        break;
    }

    // Decide who does the Admin (GUI, Colros, Layers,...)
    switch (GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
    default:
        m_bExternalAdmin = false;  // Examinator does all the admin itself
        break;
    }

    // Initialize Qtable object
    tableWidget->setColumnCount(3);
    tableWidget->setRowCount(3);
    tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    // Get handle to Histo scales tab window
    // (as we sometimes have to hide/show it)
    pTabLinesStyle  = tabWidgetStyles->widget(0);
    pTabHistoScales = tabWidgetStyles->widget(2);
    tabWidgetStyles->removeTab(tabWidgetStyles->indexOf(pTabHistoScales));
    // do not show it at startup

    // Check Parameter filter
    mFilterTestAction->setChecked(gexReport->hasInteractiveTestFilter());

    tmpReportOptions = new CReportOptions(ReportOptions);
    //  tmpReportOptionsOri = &ReportOptions;
    //tmpReportOptions = new CReportOptions();

    // Set startup style + 'Chart style' checkboxes
    tmpReportOptions->SetOption(QString("adv_histogram"),       QString("marker"),  QString("test_name|mean|limits"));
    tmpReportOptions->SetOption(QString("adv_probabilityplot"), QString("marker"),  QString("test_name|mean|limits"));
    tmpReportOptions->SetOption(QString("adv_correlation"),     QString("marker"),  QString("test_name|mean|limits"));
    tmpReportOptions->SetOption(QString("adv_trend"),           QString("marker"),  QString("test_name|mean|limits|lot"));
    tmpReportOptions->SetOption(QString("adv_boxplot_ex"),      QString("marker"),  QString("test_name|mean|limits"));

    // set variables
    iPrevShiftX      = -5;
    iPrevShiftY      = -5;
    m_pPrevTestX     = NULL;
    m_pPrevTestY     = NULL;
    m_KeyboardStatus = 0;  // Holds keybaords status on SHIFT and CTRL keys

    // Build path to HTML Test info page updated live
    strPropertiesPath = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString();

    // Build full page to HTML page used for Neptus Selection's Info
    // Config file for info to access PRIOR to process scripts
    strPropertiesPath = strPropertiesPath + GEX3D_NEPTUS_INFO;

    // Clear internal variables
    //clear();

    // Histogram Advanced default settings
    // Total histogram bars ranges in [2-10,000]
    spinBoxHistBars->setRange(2, 10000);

    // Startup default: 40 classes in histogram
    // spinBoxHistBars->setValue(tmpReportOptions->iHistoClasses);

    mChartsInfo->InitGlobal(tmpReportOptions);


    spinBoxHistBars->setValue(mChartsInfo->mTotalBars);
    checkBoxMean->setChecked(true);
    checkBoxLimits->setChecked(true);
    checkBoxMedian->setChecked(false);
    checkBox1Sigma->setChecked(false);
    checkBox15Sigma->setChecked(false);
    checkBox3Sigma->setChecked(false);
    checkBox6Sigma->setChecked(false);

    // Plot curve's name on chart
    checkBoxLayerName->setChecked(tmpReportOptions->bPlotLegend);
    // qqline
    checkBoxQQLine->setChecked(tmpReportOptions->mPlotQQLine);

    // Chart background color: paint GUI background + "Pick BkgColor" button
    pushButtonChartBackgroundColor->setActiveColor(ReportOptions.cBkgColor);

    m_iHistoViewport = GexHistogramChart::viewportOverLimits;
    m_iTrendViewport = GexTrendChart::viewportOverLimits;
    m_iScatterViewport     = GexScatterChart::viewportOverLimits;
    m_iBoxPlotViewport     = GexBoxPlotChart::viewportOverData;
    m_iProbabilityViewport = GexProbabilityPlotChart::viewportOverData;

    // Fill combo box for multi charts
    m_nMultiChartViewport = GexAbstractChart::viewportOverLimits;

    comboBoxDatasetMulti->clear();
    comboBoxDatasetMulti->
            insertItem(0, "Chart over test limits",
                       QVariant(GexAbstractChart::viewportOverLimits));
    comboBoxDatasetMulti->
            insertItem(1, "Chart over test results",
                       QVariant(GexAbstractChart::viewportOverData));
    comboBoxDatasetMulti->
            insertItem(2, "Adaptive: data & limits",
                       QVariant(GexAbstractChart::viewportAdaptive));

    comboBoxDatasetMulti->setCurrentIndex(0);

    // Get GUI user preferences
    mAnchorAction->setChecked(false);

    // Box whisker support for correlation mode is deactivated by default
    m_bSupportBoxWhisker = false;

    // Fill combo box for correlation charting mode
    comboBoxWhiskerOrientation->clear();
    comboBoxWhiskerOrientation->insertItem(0, "Horizontal",QVariant(GexBoxWhiskerChart::Horizontal));
    comboBoxWhiskerOrientation->insertItem(1, "Vertical",QVariant(GexBoxWhiskerChart::Vertical));

    if (tmpReportOptions->GetOption("adv_correlation","boxwhisker_orientation").toString() == "horizontal")
    {
        comboBoxWhiskerOrientation->setCurrentIndex(0);
    }
    else
    {
        comboBoxWhiskerOrientation->setCurrentIndex(1);
    }

    // Initializes all internal variables
    clear();

    //-- Call all the Qt connect
    ConnectAllElements();

    mSelectZone->setEnabled(gexReport->getGroupsList().count() == 1 &&
                            (m_eChartType != GexAbstractChart::chartTypeScatter &&
                            m_eChartType != GexAbstractChart::chartTypeBoxWhisker));
    // By default, check box of subset limits is hidden
    checkBoxSubsetLimits->setHidden(true);


    OnPreviousTest();

    // force defaut visible tab
    tabWidgetStyle->setCurrentIndex(2); // style
    tabWidgetStyles->setCurrentIndex(1);// Markers

   installEventFilter(this);

}

void GexWizardChart::ConnectAllElements()
{

    connect(buttonAddChart, SIGNAL(clicked()), SLOT(OnAddChart()));
    connect(buttonExportData, SIGNAL(clicked()), SLOT(OnExportData()));
    //connect(buttonFilterParameters, SIGNAL(clicked(bool)),SLOT(OnFilterParameters(bool)));


    if (! connect(buttonLayerProperties, SIGNAL(clicked()),
                  SLOT(OnChartProperties())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonLayerProperties fail");
    }
    /*if (! connect(buttonMove, SIGNAL(clicked()), SLOT(OnDrag())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonMove fail");
    }*/
    if (! connect(buttonNextTest, SIGNAL(clicked()), SLOT(OnNextTest())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonNextTest fail");
    }
    buttonNextTest->setFocusPolicy(Qt::NoFocus);  // GCORE-8733
    if (! connect(buttonNextTestY, SIGNAL(clicked()), SLOT(OnNextTestY())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonNextTestY fail");
    }
    buttonNextTestY->setFocusPolicy(Qt::NoFocus);  // GCORE-8733
    if (! connect(buttonPreviousTest, SIGNAL(clicked()), SLOT(OnPreviousTest())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonPreviousTest fail");
    }
    buttonPreviousTest->setFocusPolicy(Qt::NoFocus);  // GCORE-8733
    if (! connect(buttonPreviousTestY, SIGNAL(clicked()),
                  SLOT(OnPreviousTestY())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonPreviousTestY fail");
    }
    buttonPreviousTestY->setFocusPolicy(Qt::NoFocus);  // GCORE-8733
    if (! connect(buttonRemoveChart, SIGNAL(clicked()), SLOT(OnRemoveChart())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonRemoveChart fail");
    }
    /*if (! connect(buttonSelect, SIGNAL(clicked()), SLOT(OnSelect())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonSelect fail");
    }
    if (! connect(buttonZoom, SIGNAL(clicked()), SLOT(OnZoom())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonZoom fail");
    }
    if (! connect(m_poSelectZone, SIGNAL(clicked()), SLOT(onSelectZone())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_poSelectZone fail");
    }*/
    if (! connect(checkBox15Sigma, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBox15Sigma fail");
    }
    if (! connect(checkBox1Sigma, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBox1Sigma fail");
    }
    if (! connect(checkBox3Sigma, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBox3Sigma fail");
    }
    if (! connect(checkBoxBars, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxBars fail");
    }
    if (! connect(checkBox3DBars, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBox3DBars fail");
    }
    if (! connect(checkBoxStack, SIGNAL(clicked()), SLOT(OnCheckBoxStack())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxStack fail");
    }
    if (! connect(checkBoxfittingCurve, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxfittingCurve fail");
    }
    if (! connect(checkBoxLimits, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxLimits fail");
    }
    if (! connect(checkBoxSpecLimits, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxSpecLimits fail");
    }
    if (! connect(checkBoxLines, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxLines fail");
    }
    if (! connect(checkBoxMaximum, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxMaximum fail");
    }
    if (! connect(checkBoxMean, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxMean fail");
    }
    if (! connect(checkBoxMinimum, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxMinimum fail");
    }
    if (! connect(checkBoxspots, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxspots fail");
    }
    if (! connect(checkBoxMedian, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxMedian fail");
    }
    if (! connect(checkBoxQuartileQ1, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxQuartileQ1 fail");
    }
    if (! connect(checkBoxQuartileQ3, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxQuartileQ3 fail");
    }
    if (! connect(checkBoxSubsetLimits, SIGNAL(clicked()), SLOT(OnChangeSubsetLimit())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect SubsetLimitsToggle fail");
    }
    if (! connect(checkBox6Sigma, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBox6Sigma fail");
    }
    if (! connect(checkBoxQQLine, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxQQLine fail");
    }
    if (! connect(buttonRemoveData, SIGNAL(clicked()),
                  SLOT(OnRemoveTableDataPoints())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonRemoveData fail");
    }
    if (! connect(checkBoxBellCurve, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxBellCurve fail");
    }
    if (! connect(buttonNewParameter, SIGNAL(clicked()),
                  SLOT(OnCustomParameter())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonNewParameter fail");
    }
    if (! connect(checkBoxLotID, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxLotID fail");
    }
    if (! connect(checkBoxSubLotID, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxSubLotID fail");
    }
    if (! connect(checkBoxGroupName, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxGroupName fail");
    }
    if (! connect(comboBoxWhiskerType, SIGNAL(activated(int)),
                  SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect comboBoxWhiskerType fail");
    }
    if (! connect(checkBoxLayerName, SIGNAL(clicked()), SLOT(OnChangeStyle())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxLayerName fail");
    }

    connect(comboBoxScaleType, SIGNAL(currentIndexChanged(int)), SLOT(OnScaleTypeIndexChanged(int)));

    /* if (! connect(comboBoxScaleType, SIGNAL(activated(QString)), SLOT(OnScaleType(QString))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect comboBoxScaleType fail");
    }*/
    if (! connect(comboBoxChartLayer, SIGNAL(activated(QString)),
                  SLOT(OnChangeChartLayer(QString))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect comboBoxChartLayer fail");
    }
    if (! connect(comboBoxChartType, SIGNAL(activated(int)),
                  SLOT(OnChartType(int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect comboBoxChartType fail");
    }
    if (! connect(comboBoxDataWindow, SIGNAL(activated(QString)),
                  SLOT(OnChangeDatasetWindow(QString))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect comboBoxDataWindow fail");
    }
    if (! connect(comboBoxLineSpotStyle, SIGNAL(activated(QString)),
                  SLOT(OnChangeLineStyle(QString))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect comboBoxLineSpotStyle fail");
    }
    if (! connect(comboBoxLineStyle, SIGNAL(activated(QString)), SLOT(OnChangeLineStyle(QString))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect comboBoxLineStyle fail");
    }
    if (! connect(spinBoxHistBars, SIGNAL(valueChanged(int)),
                  SLOT(OnHistoBarSize(int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect spinBoxHistBars fail");
    }
    if (! connect(customSpinBoxHistBars, SIGNAL(valueChanged(int)),
                  SLOT(OnHistoBarSize(int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect customSpinBoxHistBars fail");
    }
    if (! connect(radioButtonScatter, SIGNAL(toggled(bool)),
                  SLOT(OnCorrelationMode(bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect radioButtonScatter fail");
    }
    if (! connect(comboBoxWhiskerOrientation, SIGNAL(activated(int)),
                  SLOT(OnWhiskerOrientation(int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect comboBoxWhiskerOrientation fail");
    }
    if (! connect(spinBoxLineWidth, SIGNAL(valueChanged(QString)),
                  SLOT(OnChangeLineStyle(QString))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect spinBoxLineWidth fail");
    }
    if (! connect(tabWidgetChart, SIGNAL(currentChanged(int)),
                  SLOT(OnTabChange(int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect tabWidgetChart fail");
    }
    if (! connect(treeWidgetLayers,
                  SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
                  SLOT(OnEditLayer(QTreeWidgetItem *, int))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect treeWidgetLayers fail");
    }
    if (! connect(treeWidgetLayers,
                  SIGNAL(customContextMenuRequested(const QPoint &)),
                  SLOT(OnContextualMenu(const QPoint &))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect treeWidgetLayers fail");
    }
    if (! connect(buttonExportExcelClipboard, SIGNAL(clicked()),
                  SLOT(OnExportExcelClipboard())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonExportExcelClipboard fail");
    }
    if (! connect(buttonExportChartStatsExcelClipboard, SIGNAL(clicked()),
                  SLOT(OnExportChartStats())))
    {
        GSLOG(SYSLOG_SEV_WARNING,
              "connect buttonExportChartStatsExcelClipboard fail");
    }
    if (! connect(pushButtonLineColor, SIGNAL(colorChanged(const QColor &)),
                  SLOT(onChangedLineColor(const QColor &))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect pushButtonLineColor fail");
    }
    if (! connect(pushButtonChartBackgroundColor,
                  SIGNAL(colorChanged(const QColor &)),
                  SLOT(onChangedChartBackgroundColor(const QColor &))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect pushButtonChartBackgroundColor fail");
    }
    /*if (! connect(pushButtonKeepViewport, SIGNAL(clicked(bool)),
                  m_pSingleChartWidget, SLOT(onKeepViewportChanged(bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxKeepViewport fail");
    }
    if (! connect(pushButtonKeepViewport, SIGNAL(clicked(bool)),
                  m_pMultiChartWidget, SLOT(onKeepViewportChanged(bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect checkBoxKeepViewport fail");
    }*/
    if (! connect(this, SIGNAL(testChanged()), SLOT(onTestChanged())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect this fail");
    }
    if (! connect(m_pSingleChartWidget, SIGNAL(currentDieSelected()),
                  SLOT(onCurrentDieSelected())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pSingleChartWidget, SIGNAL(deviceResults(long)),
                  SLOT(onDeviceResults(long))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pSingleChartWidget, SIGNAL(editStyles()),
                  SLOT(onEditChartStyles())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pSingleChartWidget, SIGNAL(editMarkers()),
                  SLOT(onEditChartMarkers())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pSingleChartWidget,
                  SIGNAL(selectWafermapRange(double, double)),
                  SLOT(onSelectWafermapRange(double, double))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pSingleChartWidget,
                  SIGNAL(removeDatapointRange(double, double, bool)),
                  SLOT(onRemoveDatapointRange(double, double, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pSingleChartWidget,
                  SIGNAL(removeDatapointHigher(double, bool)),
                  SLOT(onRemoveDatapointHigher(double, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pSingleChartWidget,
                  SIGNAL(removeDatapointLower(double, bool)),
                  SLOT(onRemoveDatapointLower(double, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pSingleChartWidget, SIGNAL(removeIQR(double, bool)),
                  SLOT(onRemoveIQR(double, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pSingleChartWidget, SIGNAL(removeNSigma(double, bool)),
                  SLOT(onRemoveNSigma(double, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pSingleChartWidget, SIGNAL(rebuildReport(const QString &)),
                  SLOT(onRebuildReport(const QString &))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (!connect(m_pSingleChartWidget, SIGNAL(deselectHighlightedDie()),   SLOT(OnDeselectHighlightedDie())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pSingleChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget, SIGNAL(currentDieSelected()),
                  SLOT(onCurrentDieSelected())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget, SIGNAL(deviceResults(long)),
                  SLOT(onDeviceResults(long))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget, SIGNAL(editStyles()),
                  SLOT(onEditChartStyles())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget, SIGNAL(editMarkers()),
                  SLOT(onEditChartMarkers())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget,
                  SIGNAL(selectWafermapRange(double, double)),
                  SLOT(onSelectWafermapRange(double, double))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget,
                  SIGNAL(removeDatapointRange(double, double, bool)),
                  SLOT(onRemoveDatapointRange(double, double, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget,
                  SIGNAL(removeDatapointHigher(double, bool)),
                  SLOT(onRemoveDatapointHigher(double, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget,
                  SIGNAL(removeDatapointLower(double, bool)),
                  SLOT(onRemoveDatapointLower(double, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget, SIGNAL(removeIQR(double, bool)),
                  SLOT(onRemoveIQR(double, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget, SIGNAL(removeNSigma(double, bool)),
                  SLOT(onRemoveNSigma(double, bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (! connect(m_pMultiChartWidget, SIGNAL(rebuildReport(const QString &)),
                  SLOT(onRebuildReport(const QString &))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    if (!connect(m_pMultiChartWidget, SIGNAL(deselectHighlightedDie()), SLOT(OnDeselectHighlightedDie())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect m_pMultiChartWidget fail");
    }
    //    if (! connect(buttonSelectMulti, SIGNAL(clicked()), SLOT(onSelectMulti())))
    //    {
    //        GSLOG(SYSLOG_SEV_WARNING, "connect buttonSelectMulti fail");
    //    }
    //    if (! connect(buttonMoveMulti, SIGNAL(clicked()), SLOT(onMoveMulti())))
    //    {
    //        GSLOG(SYSLOG_SEV_WARNING, "connect buttonMoveMulti fail");
    //    }
    //    if (! connect(buttonZoomMulti, SIGNAL(clicked()), SLOT(onZoomMulti())))
    //    {
    //        GSLOG(SYSLOG_SEV_WARNING, "connect buttonZoomMulti fail");
    //    }
    //    if (! connect(buttonHomeMulti, SIGNAL(clicked()), SLOT(onHomeMulti())))
    //    {
    //        GSLOG(SYSLOG_SEV_WARNING, "connect buttonHomeMulti fail");
    //    }
    if (! connect(comboBoxDatasetMulti, SIGNAL(activated(QString)),
                  SLOT(OnChangeDatasetMulti(QString))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect comboBoxDatasetMulti fail");
    }
    if (! connect(gexReport, SIGNAL(interactiveFilterChanged(bool)),
                  SLOT(onFilterChanged(bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect gexReport fail");
    }
    if (! connect(tableWidget, SIGNAL(customContextMenuRequested(const QPoint &)),
                  SLOT(OnTableContextualMenu(const QPoint &))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect tableWidget fail");
    }
    if (! connect(customPropGroupBox, SIGNAL(toggled(bool)),
                  SLOT(useCustomHistoBars(bool))))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect customPropGroupBox fail");
    }
}

void GexWizardChart::ShowParam()
{
    frameParameters->show();
    // pushButtonReduce->setText(">");
    this->resize(size().width() + mFrameNavigationSize.width(), size().height() + mFrameNavigationSize.height());
}

void GexWizardChart::OnShowHideParam()
{
    if (frameParameters->isHidden())
    {
        frameParameters->show();
        // pushButtonReduce->setText(">");
        this->resize(size().width() + mFrameNavigationSize.width(), size().height() + mFrameNavigationSize.height());
    }
    else
    {
        mFrameNavigationSize = frameParameters->size();
        frameParameters->hide();
        // pushButtonReduce->setText("<");
        this->resize(size().width() - mFrameNavigationSize.width(), size().height() - mFrameNavigationSize.height());
    }
}

/******************************************************************************!
 * \fn closeEvent
 * \brief Close signal: If window is detached, this reparent it instead
 ******************************************************************************/
void GexWizardChart::closeEvent(QCloseEvent* e)
{
    if (pGexMainWindow != NULL)
    {
        e->accept();
        mWizardHandler->RemoveWizardChart(this);
    }
}

/******************************************************************************!
 * \fn OnChangeTestSortOrder
 ******************************************************************************/
void GexWizardChart::OnChangeTestSortOrder()
{
    if (pFile == NULL)
    {
        GSLOG(SYSLOG_SEV_WARNING, "pFile == NULL");
        return;
    }

    pFile->ChangeTestSortOrder();
    buttonPreviousTest->setEnabled(true);
    buttonNextTest->setEnabled(true);
}

/******************************************************************************!
 * \fn ShowPage
 * \brief Make page visible
 ******************************************************************************/
void GexWizardChart::ShowPage()
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
 * \fn CGexSingleChart
 * \brief Constructor: Parameter chart class
 ******************************************************************************/
CGexSingleChart::CGexSingleChart(int iEntry)
    :mTextRotation(0)
{
    iTestNumberX = iTestNumberY = iTestNumberZ = (unsigned ) -1;
    iPinMapX     = iPinMapY = iPinMapZ = -1;
    iGroupX      = iGroupY = iGroupZ = 0;

    resetVariables(iEntry);
}

/******************************************************************************!
 * \fn CGexSingleChart
 * \brief Copy constructor
 ******************************************************************************/
CGexSingleChart::CGexSingleChart(const CGexSingleChart& other)
{
    *this = other;
}

/******************************************************************************!
 * \fn ~CGexSingleChart
 * \brief Destructor
 ******************************************************************************/
CGexSingleChart::~CGexSingleChart()
{

}

/******************************************************************************!
 * \fn operator=
 ******************************************************************************/
CGexSingleChart& CGexSingleChart::operator=(const CGexSingleChart& other)
{
    if (this != &other)
    {
        // Label for this chart
        strChartName = other.strChartName;

        // Parameter in X
        iTestNumberX  = other.iTestNumberX;
        iPinMapX      = other.iPinMapX;
        strTestNameX  = other.strTestNameX;
        strTestLabelX = other.strTestLabelX;
        iGroupX       = other.iGroupX;

        // Parameter in Y
        iTestNumberY  = other.iTestNumberY;
        iPinMapY      = other.iPinMapY;
        strTestNameY  = other.strTestNameY;
        strTestLabelY = other.strTestLabelY;
        iGroupY       = other.iGroupY;

        // Parameter in Z
        iTestNumberZ  = other.iTestNumberZ;
        iPinMapZ      = other.iPinMapZ;
        strTestNameZ  = other.strTestNameZ;
        strTestLabelZ = other.strTestLabelZ;
        iGroupZ       = other.iGroupZ;

        CopySettings(&other);

        // Probability plot data
        m_dataProbabilityPlot = other.m_dataProbabilityPlot;

    }

    return *this;
}


void CGexSingleChart::CopySettings(const CGexSingleChart* other)
{
    // Style
    bVisible      = other->bVisible;
    bBoxBars      = other->bBoxBars;
    bBox3DBars    = other->bBox3DBars;
    mIsStacked    = other->mIsStacked;
    bFittingCurve = other->bFittingCurve;
    bBellCurve    = other->bBellCurve;
    bLines        = other->bLines;
    bSpots        = other->bSpots;
    iWhiskerMode  = other->iWhiskerMode;
    iLineWidth    = other->iLineWidth;
    cColor        = other->cColor;
    iLineStyle    = other->iLineStyle;
    iSpotStyle    = other->iSpotStyle;
    lfDataOffset  = other->lfDataOffset;
    lfDataScale   = other->lfDataScale;
    //mSpecLimit    = other->mSpecLimit;
    mLayerName    = other->mLayerName;
    mQQLine      = other->mQQLine;

    // Markers: Line width (0=hide), color
    m_iMeanLineWidth    = other->m_iMeanLineWidth;
    m_cMeanColor        = other->m_cMeanColor;
    m_iMedianLineWidth  = other->m_iMedianLineWidth;
    m_cMedianColor      = other->m_cMedianColor;
    m_iMinLineWidth     = other->m_iMinLineWidth;
    m_cMinColor         = other->m_cMinColor;
    m_iMaxLineWidth     = other->m_iMaxLineWidth;
    m_cMaxColor         = other->m_cMaxColor;
    m_iLimitsLineWidth  = other->m_iLimitsLineWidth;
    m_cLimitsColor      = other->m_cLimitsColor;
    m_iSpecLimitsLineWidth  = other->m_iSpecLimitsLineWidth;
    m_cSpecLimitsColor = other->m_cSpecLimitsColor;

    m_i2SigmaLineWidth  = other->m_i2SigmaLineWidth;
    m_c2SigmaColor      = other->m_c2SigmaColor;
    m_i3SigmaLineWidth  = other->m_i3SigmaLineWidth;
    m_c3SigmaColor      = other->m_c3SigmaColor;
    m_i6SigmaLineWidth  = other->m_i6SigmaLineWidth;
    m_c6SigmaColor      = other->m_c6SigmaColor;
    m_i12SigmaLineWidth = other->m_i12SigmaLineWidth;
    m_c12SigmaColor     = other->m_c12SigmaColor;
    m_iRollingLimitsLineWidth   = other->m_iRollingLimitsLineWidth;

    m_iQuartileQ3LineWidth = other->m_iQuartileQ3LineWidth;
    m_cQuartileQ3Color     = other->m_cQuartileQ3Color;

    m_iQuartileQ1LineWidth = other->m_iQuartileQ1LineWidth;
    m_cQuartileQ1Color     = other->m_cQuartileQ1Color;

    mTextRotation          = other->mTextRotation;
}

bool  CGexSingleChart::ToJson(QJsonObject& descriptionOut) const
{

    descriptionOut.insert("ChartName", strChartName);

    // Parameter in X to plot on the chart
    descriptionOut.insert("TestNumberX",   static_cast<int>(iTestNumberX));
    descriptionOut.insert("PinMapX",       iPinMapX);
    descriptionOut.insert("TestNameX",     strTestNameX);
    descriptionOut.insert("TestLabelX",    strTestLabelX);
    descriptionOut.insert("GroupX",        iGroupX );

    descriptionOut.insert("TestNumber",    static_cast<int>(iTestNumberY));
    descriptionOut.insert("PinMapY",       iPinMapY);
    descriptionOut.insert("TestNameY",     strTestNameY);
    descriptionOut.insert("TestLabelY",    strTestLabelY);
    descriptionOut.insert("GroupY",        iGroupY);

    // Parameter in Z to plot on the chart
    descriptionOut.insert("TestNumberZ",   static_cast<int>(iTestNumberZ));
    descriptionOut.insert("PinMapZ",       iPinMapZ);
    descriptionOut.insert("TestNameZ",     strTestNameZ);
    descriptionOut.insert("TestLabelZ",    strTestLabelZ);
    descriptionOut.insert("GroupZ",        iGroupZ);

    descriptionOut.insert("Visible",       bVisible);
    descriptionOut.insert("BoxBars",       bBoxBars);
    descriptionOut.insert("Box3DBars",     bBox3DBars);

    descriptionOut.insert("IsStacked",     mIsStacked);
    descriptionOut.insert("LayerName",     mLayerName);
    descriptionOut.insert("QQLine",        mQQLine);

    descriptionOut.insert("FittingCurve",  bFittingCurve);
    descriptionOut.insert("BellCurve",     bBellCurve);
    descriptionOut.insert("Lines",         bLines);
    descriptionOut.insert("Spots",         bSpots);
    descriptionOut.insert("WhiskerMode",   iWhiskerMode);
    descriptionOut.insert("LineWidth",     iLineWidth);

    QJsonArray lColor;
    ColorRGBToJsonArray(lColor, cColor);
    descriptionOut.insert("Color", lColor);

    descriptionOut.insert("LineStyle",     iLineStyle);
    descriptionOut.insert("SpotStyle",     iSpotStyle);
    descriptionOut.insert("DataOffset",    lfDataOffset);
    descriptionOut.insert("DataScale",     lfDataScale);

    descriptionOut.insert("MeanLineWidth",             m_iMeanLineWidth);
    descriptionOut.insert("MedianLineWidth",           m_iMedianLineWidth);
    descriptionOut.insert("MinLineWidth",              m_iMinLineWidth);
    descriptionOut.insert("MaxLineWidth",              m_iMaxLineWidth);
    descriptionOut.insert("LimitsLineWidth",           m_iLimitsLineWidth);
    descriptionOut.insert("SpecLimitsLineWidth",       m_iSpecLimitsLineWidth);
    descriptionOut.insert("RollingLimitsLineWidth",    m_iRollingLimitsLineWidth);
    descriptionOut.insert("2SigmaLineWidth",           m_i2SigmaLineWidth);
    descriptionOut.insert("3SigmaLineWidth",           m_i3SigmaLineWidth);
    descriptionOut.insert("6SigmaLineWidth",           m_i6SigmaLineWidth);
    descriptionOut.insert("12SigmaLineWidth",          m_i12SigmaLineWidth);
    descriptionOut.insert("QuartileQ1",                m_iQuartileQ1LineWidth);
    descriptionOut.insert("QuartileQ3LineWidth",       m_iQuartileQ3LineWidth);
    descriptionOut.insert("TextRotation",              mTextRotation);


    ColorRGBToJsonArray(lColor, m_cMeanColor);
    descriptionOut.insert("MeanColor", lColor);

    ColorRGBToJsonArray(lColor, m_cMedianColor);
    descriptionOut.insert("MedianColor", lColor);

    ColorRGBToJsonArray(lColor, m_cMinColor);
    descriptionOut.insert("MinColor", lColor);

    ColorRGBToJsonArray(lColor, m_cMaxColor);
    descriptionOut.insert("MaxColor", lColor);

    ColorRGBToJsonArray(lColor, m_cLimitsColor);
    descriptionOut.insert("LimitsColor", lColor);

    ColorRGBToJsonArray(lColor, m_cSpecLimitsColor);
    descriptionOut.insert("SpecLimitsColor", lColor);

    ColorRGBToJsonArray(lColor, m_c2SigmaColor);
    descriptionOut.insert("C2SigmaColor", lColor);

    ColorRGBToJsonArray(lColor, m_c3SigmaColor);
    descriptionOut.insert("C3SigmaColor", lColor);

    ColorRGBToJsonArray(lColor, m_c6SigmaColor);
    descriptionOut.insert("C6SigmaColor", lColor);

    ColorRGBToJsonArray(lColor, m_c12SigmaColor);
    descriptionOut.insert("C12SigmaColor", lColor);

    ColorRGBToJsonArray(lColor, m_cQuartileQ1Color);
    descriptionOut.insert("QuartileQ1Color", lColor);

    ColorRGBToJsonArray(lColor, m_cQuartileQ3Color);
    descriptionOut.insert("QuartileQ3Color", lColor);

    return true;
}

bool CGexSingleChart::InitFromJSon( const QJsonObject &description)
{
    QColor lDefaultColor(Qt::green);

    if(description.contains("ChartName"))
        strChartName    = description["ChartName"].toString();

    // Parameter in X to plot on the chart
    if(description.contains("TestNumberX"))
        iTestNumberX    = description["TestNumberX"].toInt();

    if(description.contains("PinMapX"))
        iPinMapX        = description["PinMapX"].toInt();

    if(description.contains("TestNameX"))
        strTestNameX    = description["TestNameX"].toString();

    if(description.contains("TestLabelX"))
        strTestLabelX   = description["TestLabelX"].toString();

    if(description.contains("GroupX"))
        iGroupX         = description["GroupX"].toInt();

    if(description.contains("TestNumber"))
        iTestNumberY    = description["TestNumber"].toInt();

    if(description.contains("PinMapY"))
        iPinMapY        = description["PinMapY"].toInt();

    if(description.contains("TestNameY"))
        strTestNameY    = description["TestNameY"].toString();

    if(description.contains("TestLabelY"))
        strTestLabelY   = description["TestLabelY"].toString();

    if(description.contains("GroupY"))
        iGroupY         = description["GroupY"].toInt();

    // Parameter in Z to plot on the chart
    if(description.contains("TestNumberZ"))
        iTestNumberZ    = description["TestNumberZ"].toInt();

    if(description.contains("PinMapZ"))
        iPinMapZ        = description["PinMapZ"].toInt();

    if(description.contains("TestNameZ"))
        strTestNameZ    = description["TestNameZ"].toString();

    if(description.contains("GroupX"))
        strTestLabelZ   = description["TestLabelZ"].toString();

    if(description.contains("GroupZ"))
        iGroupZ         = description["GroupZ"].toInt();

    if(description.contains("Visible"))
        bVisible        = description["Visible"].toBool();

    if(description.contains("BoxBars"))
        bBoxBars        = description["BoxBars"].toBool();

    if(description.contains("Box3DBars"))
        bBox3DBars      = description["Box3DBars"].toBool();

    if(description.contains("IsStacked"))
        mIsStacked      = description["IsStacked"].toBool();

    if(description.contains("LayerName"))
        mLayerName      = description["LayerName"].toBool();

    if(description.contains("QQLine"))
        mQQLine         = description["QQLine"].toBool();

    if(description.contains("FittingCurve"))
       bFittingCurve   = description["FittingCurve"].toBool();

    if(description.contains("BellCurve"))
        bBellCurve      = description["BellCurve"].toBool();

    if(description.contains("Lines"))
        bLines          = description["Lines"].toBool();

    if(description.contains("Spots"))
        bSpots          = description["Spots"].toBool();

    if(description.contains("WhiskerMode"))
        iWhiskerMode    = description["WhiskerMode"].toInt();

    if(description.contains("LineWidth"))
        iLineWidth      = description["LineWidth"].toInt();

    if(description.contains("Color"))
        cColor          = JsonArrayToColorRGB( description["Color"].toArray(), lDefaultColor);

    if(description.contains("LineStyle"))
        iLineStyle      = description["LineStyle"].toInt();

    if(description.contains("SpotStyle"))
        iSpotStyle      = description["SpotStyle"].toInt();

    if(description.contains("DataOffset"))
        lfDataOffset    = description["DataOffset"].toDouble();

    if(description.contains("DataScale"))
        lfDataScale     = description["DataScale"].toDouble();


    if(description.contains("MeanLineWidth"))
        m_iMeanLineWidth            = description["MeanLineWidth"].toInt();

    if(description.contains("MedianLineWidth"))
        m_iMedianLineWidth          = description["MedianLineWidth"].toInt();

    if(description.contains("MinLineWidth"))
        m_iMinLineWidth             = description["MinLineWidth"].toInt();

    if(description.contains("MaxLineWidth"))
        m_iMaxLineWidth             = description["MaxLineWidth"].toInt();

    if(description.contains("LimitsLineWidth"))
        m_iLimitsLineWidth          = description["LimitsLineWidth"].toInt();

    if(description.contains("SpecLimitsLineWidth"))
        m_iSpecLimitsLineWidth      = description["SpecLimitsLineWidth"].toInt();

    if(description.contains("RollingLimitsLineWidth"))
        m_iRollingLimitsLineWidth   = description["RollingLimitsLineWidth"].toInt();

    if(description.contains("2SigmaLineWidth"))
        m_i2SigmaLineWidth          = description["2SigmaLineWidth"].toInt();

    if(description.contains("3SigmaLineWidth"))
        m_i3SigmaLineWidth          = description["3SigmaLineWidth"].toInt();

    if(description.contains("6SigmaLineWidth"))
        m_i6SigmaLineWidth          = description["6SigmaLineWidth"].toInt();

    if(description.contains("12SigmaLineWidth"))
        m_i12SigmaLineWidth         = description["12SigmaLineWidth"].toInt();

    if(description.contains("QuartileQ1"))
        m_iQuartileQ1LineWidth      = description["QuartileQ1"].toInt();

    if(description.contains("QuartileQ3LineWidth"))
        m_iQuartileQ3LineWidth      = description["QuartileQ3LineWidth"].toInt();

    if(description.contains("TextRotation"))
        mTextRotation               = description["TextRotation"].toInt();

    if(description.contains("MeanColor") && description["MeanColor"].isArray())
        m_cMeanColor                = JsonArrayToColorRGB( description["MeanColor"].toArray(),       lDefaultColor);

    if(description.contains("MedianColor") && description["MedianColor"].isArray())
        m_cMedianColor              = JsonArrayToColorRGB( description["MedianColor"].toArray(),     lDefaultColor);

    if(description.contains("MinColor") && description["MinColor"].isArray())
        m_cMinColor                 = JsonArrayToColorRGB( description["MinColor"].toArray(),        lDefaultColor);

    if(description.contains("MaxColor") && description["MaxColor"].isArray())
        m_cMaxColor                 = JsonArrayToColorRGB( description["MaxColor"].toArray(),       lDefaultColor);

    if(description.contains("LimitsColor") && description["LimitsColor"].isArray())
        m_cLimitsColor              = JsonArrayToColorRGB( description["LimitsColor"].toArray(),     lDefaultColor);

    if(description.contains("SpecLimitsColor") && description["SpecLimitsColor"].isArray())
        m_cSpecLimitsColor          = JsonArrayToColorRGB( description["SpecLimitsColor"].toArray(), lDefaultColor);

    if(description.contains("C2SigmaColor") && description["C2SigmaColor"].isArray())
        m_c2SigmaColor              = JsonArrayToColorRGB( description["C2SigmaColor"].toArray(),    lDefaultColor);

    if(description.contains("C3SigmaColor") && description["C3SigmaColor"].isArray())
        m_c3SigmaColor              = JsonArrayToColorRGB( description["C3SigmaColor"].toArray(),    lDefaultColor);

    if(description.contains("C6SigmaColor") && description["C6SigmaColor"].isArray())
        m_c6SigmaColor              = JsonArrayToColorRGB( description["C6SigmaColor"].toArray(),    lDefaultColor);

    if(description.contains("C12SigmaColor") && description["C12SigmaColor"].isArray())
        m_c12SigmaColor             = JsonArrayToColorRGB( description["C12SigmaColor"].toArray(),   lDefaultColor);

    if(description.contains("QuartileQ1Color") && description["QuartileQ1Color"].isArray())
        m_cQuartileQ1Color          = JsonArrayToColorRGB( description["QuartileQ1Color"].toArray(), lDefaultColor);

    if(description.contains("QuartileQ3Color") && description["QuartileQ3Color"].isArray())
        m_cQuartileQ3Color          = JsonArrayToColorRGB( description["QuartileQ3Color"].toArray(), lDefaultColor);

    return true;
}

/******************************************************************************!
 * \fn CGexChartOptions
 * \brief Constructor for Chart Options (Title, Legends, Custom Viewport)
 ******************************************************************************/
CGexChartOptions::CGexChartOptions()
{
    bChartTitle = false;  // true if ChartTitle is valid
    bLegendX    = false; // true if Legend for X axis is valid
    bLegendY    = false; // true if Legend for Y axis is valid
    bLogScaleX  = false; // true if X scale is LOG instead of linear
    bLogScaleY  = false; // true if Y scale is LOG instead of linear

    lfLowX = 0.;			// Custom Viewport rectangle to focus on...
    lfLowY= 0.;
    lfHighX= 0.;
    lfHighY= 0.;
}


bool CGexChartOptions::InitFromJSon(const QJsonObject& description)
{
    if(description.contains("ChartStringTitle"))
        strChartTitle   = description["ChartStringTitle"].toString();

    // Always force to have the test name
    if(description.contains("HasChartTitle"))
        bChartTitle     = description["HasChartTitle"].toBool();

    if(description.contains("AxisLegendX"))
        strAxisLegendX  = description["AxisLegendX"].toString();

    if(description.contains("LegendX"))
        bLegendX        = description["LegendX"].toBool();

    if(description.contains("LogScaleX"))
        bLogScaleX      = description["LogScaleX"].toBool();

    if(description.contains("AxisLegendY"))
        strAxisLegendY  = description["AxisLegendY"].toString();

    if(description.contains("LegendY"))
        bLegendY        = description["LegendY"].toBool();

    if(description.contains("LogScaleY"))
        bLogScaleY      = description["LogScaleY"].toBool();

    if(description.contains("LowX"))
        lfLowX          = description["LowX"].toDouble();

    if(description.contains("LowY"))
        lfLowY          = description["LowY"].toDouble();

    if(description.contains("HighX"))
        lfHighX         = description["HighX"].toDouble();

    if(description.contains("HighY"))
        lfHighY         = description["HighY"].toDouble();

    return true;
}

bool  CGexChartOptions::ToJson(QJsonObject& descriptionOut) const
{
    descriptionOut.insert("ChartStringTitle",   strChartTitle);
    descriptionOut.insert("AxisLegendX",        strAxisLegendX);
    descriptionOut.insert("LegendX",            bLegendX);
    descriptionOut.insert("LogScaleX",          bLogScaleX);
    descriptionOut.insert("AxisLegendY",        strAxisLegendY);
    descriptionOut.insert("LegendY",            bLegendY);
    descriptionOut.insert("LogScaleY",          bLogScaleY);
    descriptionOut.insert("LowX",               lfLowX);
    descriptionOut.insert("LowY",               lfLowY);
    descriptionOut.insert("HighX",              lfHighX);
    descriptionOut.insert("HighY",              lfHighY);
    descriptionOut.insert("HasChartTitle",      bChartTitle);
    return true;
}

///////////////////////////////////////////////////////////
// Default Color assigned to a curve index at creation time
///////////////////////////////////////////////////////////
static QColor cColorMap[] =
{
    QColor(121, 182, 91),  // light green
    QColor(255, 0, 255),  // Magenta
    QColor(0, 0, 128),  // Blue-grey
    QColor(255, 255, 0),  // Yellow
    QColor(0, 255, 255),  // Blue lagoon
    QColor(128, 0, 128),  // Violet
    QColor(128, 0, 0),  // Brown
    QColor(255, 153, 0),  // brighter Brown-orange
    QColor(0, 0, 255),  // Full blue
    QColor(0, 204, 255),  // Light blue lagoon
    QColor(0, 0, 0),  // Black
    QColor(0, 128, 0),  // Green
    QColor(128, 128, 0),  // Brown
    QColor(0, 128, 128),  // blue-grey
    QColor(255, 124, 128),  // Pink
    QColor(0, 51, 0),  // Green
    QColor(51, 51, 0),  // Brown
    QColor(153, 51, 0),  // Brown-Red
    QColor(255, 102, 0),  // Brown-orange
    QColor(0, 85, 0),  // Green
    QColor(153, 204, 0),  // Light green
    QColor(255, 204, 0),  // Yellow
    QColor(102, 0, 102),  // dark Violet
    QColor(0, 51, 102),  // dark blue-grey
    QColor(153, 51, 102),  // dark pink
    QColor(51, 153, 102),  // medium Green
    QColor(51, 51, 153),  // Blue
    QColor(255, 153, 204),  // Light pink
    QColor(0, 102, 204),  // Blue
    QColor(51, 204, 204),  // light blue lagoon
    QColor(102, 102, 153),  // grey-Blue
    QColor(204, 153, 255),  // light violet
    QColor(150, 150, 150),  // grey
    QColor(255, 0, 0),  // red
    QColor(0, 255, 0)  // Green
};

/******************************************************************************!
 * \fn resetVariables
 * \brief Set variables to predefined state
 ******************************************************************************/
void CGexSingleChart::resetVariables(int iEntry)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "CGexSingleChart reset variables...");
    CGexSingleChart* pLayerStyle=0;
    // Style
    bVisible     = true; // true if visible (default)
    lfDataOffset = 0.0;  // Offset to apply to data in layer (trend chart only)
    lfDataScale  = 1.0;  // Scale factor to apply to data in layer

    // (trend chart only)

    // Clear data probability plot
    dataProbabilityPlot().clean();

    // Get default Style if exists...
    if ((int) ReportOptions.pLayersStyleList.count() > iEntry && iEntry >= 0)
    {
        // Copy information from structures read in startup script file
        pLayerStyle = ReportOptions.pLayersStyleList.at(iEntry);

        if (pLayerStyle)
        {
            // true if draw with BARS (histogram charting only)
            bBoxBars            = pLayerStyle->bBoxBars;
            // true if draw with 3D-BARS (histogram charting only)
            bBox3DBars          = pLayerStyle->bBoxBars;
            mIsStacked          = pLayerStyle->mIsStacked;
            // true if draw fitting curve / spin
            bFittingCurve       = pLayerStyle->bFittingCurve;
            // true if draw Guaussian Bell-curve shape
            bBellCurve          = pLayerStyle->bBellCurve;
            // Type of whisker : range, Q2 +/- 1.5*IQR etc
            iWhiskerMode        = pLayerStyle->iWhiskerMode;
            // true if connect points with a line
            bLines              = pLayerStyle->bLines;
            // true if draw a spot at each data point
            bSpots              = pLayerStyle->bSpots;

            // mSpecLimit          = pLayerStyle->mSpecLimit;
            mLayerName          = pLayerStyle->mLayerName;
            mQQLine            = pLayerStyle->mQQLine;

            // Spot style: circle, rectangle, diamond, etc...
            iSpotStyle          = pLayerStyle->iSpotStyle;
            iLineWidth          = pLayerStyle->iLineWidth;  // Plotting line width
            iLineStyle          = pLayerStyle->iLineStyle;  // Line style: solid
            cColor              = pLayerStyle->cColor;

            // Markers
            m_iMeanLineWidth    = pLayerStyle->meanLineWidth();;
            m_cMeanColor        = pLayerStyle->meanColor();

            m_iMedianLineWidth  = pLayerStyle->medianLineWidth();
            m_cMedianColor      = pLayerStyle->medianColor();

            m_iMinLineWidth     = pLayerStyle->minLineWidth();
            m_cMinColor         = pLayerStyle->minColor();

            m_iMaxLineWidth     = pLayerStyle->maxLineWidth();
            m_cMaxColor         = pLayerStyle->maxColor();

            m_iLimitsLineWidth  = pLayerStyle->limitsLineWidth();
            m_cLimitsColor      = pLayerStyle->limitsColor();

            m_iSpecLimitsLineWidth  = pLayerStyle->specLimitsLineWidth();
            m_cSpecLimitsColor      = pLayerStyle->specLimitsColor();

            m_i2SigmaLineWidth  = pLayerStyle->sigma2LineWidth();
            m_c2SigmaColor      = pLayerStyle->sigma2Color();

            m_i3SigmaLineWidth  = pLayerStyle->sigma3LineWidth();
            m_c3SigmaColor      = pLayerStyle->sigma3Color();

            m_i6SigmaLineWidth  = pLayerStyle->sigma6LineWidth();
            m_c6SigmaColor      = pLayerStyle->sigma6Color();

            m_i12SigmaLineWidth = pLayerStyle->sigma12LineWidth();
            m_c12SigmaColor     = pLayerStyle->sigma12Color();

            m_iRollingLimitsLineWidth = pLayerStyle->GetRollingLimitsLineWidth();

            m_iQuartileQ3LineWidth = pLayerStyle->quartileQ3LineWidth();
            m_cQuartileQ3Color     = pLayerStyle->quartileQ3Color();

            m_iQuartileQ1LineWidth = pLayerStyle->quartileQ1LineWidth();
            m_cQuartileQ1Color     = pLayerStyle->quartileQ1Color();
        }

        return;
    }

    bBoxBars      = true; // true if draw with BARS (histogram charting only)
    bBox3DBars    = true; // true if draw with 3D-BARS (histogram charting only)
    mIsStacked = false;
    bFittingCurve = false;  // true if draw fitting curve / spin
    bBellCurve    = false; // true if draw Guaussian Bell-curve shape
    bLines        = true; // true if connect points with a line
    bSpots        = false; // true if draw a spot at each data point
    // Whisker charting mode: Range, Q2 +/- 1.5*IQR, etc.
    iWhiskerMode = GEX_WHISKER_RANGE;

    // assign different colors & line style & spots for upto 9 entries
    // then users will need to manually change the settings if more than 9
    // parameters are overlayed. eg: play with the colors
    iSpotStyle = iEntry % 9;  // Spot style: circle, rectangle, diamond, etc.
    iLineWidth = 1;  // Plotting line width
    iLineStyle = 0;  // Line style: solid

    // Get color
    iEntry = iEntry % (sizeof(cColorMap) / sizeof(QColor));
    cColor = cColorMap[iEntry];
    QColor cMarkerColor = cColor;

    m_iMeanLineWidth = 1;
    m_cMeanColor     = cMarkerColor;

    m_iMedianLineWidth = 0;
    m_cMedianColor     = cMarkerColor;

    m_iMinLineWidth = 0;
    m_cMinColor     = cMarkerColor;

    m_iMaxLineWidth = 0;
    m_cMaxColor     = cMarkerColor;

    m_iLimitsLineWidth = 1;
    m_cLimitsColor     = cMarkerColor;

    m_iSpecLimitsLineWidth = 1;
    m_cSpecLimitsColor     = cMarkerColor;

    m_i2SigmaLineWidth = 0;
    m_c2SigmaColor     = cMarkerColor;

    m_i3SigmaLineWidth = 0;
    m_c3SigmaColor     = cMarkerColor;

    m_i6SigmaLineWidth = 0;
    m_c6SigmaColor     = cMarkerColor;

    m_i12SigmaLineWidth = 0;
    m_c12SigmaColor     = cMarkerColor;

    m_iRollingLimitsLineWidth = 1;

    m_iQuartileQ3LineWidth = 0;
    m_cQuartileQ3Color     = cMarkerColor;

    m_iQuartileQ1LineWidth = 0;
    m_cQuartileQ1Color     = cMarkerColor;

}

/******************************************************************************!
 * \fn clear
 * \brief Called each time the new Query & script is executed: resets flags
 ******************************************************************************/
void GexWizardChart::clear()
{
    // Clear variables
    ptTestCell  = NULL;
    ptTestCellY = NULL;
    lTestNumber = (unsigned) -1;
    pGroup      = NULL;
    pFile       = NULL;
    pFileY      = NULL;

    // Default tab is 'Chart'
    tabWidgetChart->setCurrentIndex(tabWidgetChart->indexOf(tabChart));

    // Keeps pointer to frame that holds the drawing
    mChartsInfo->ptParent = this;

    // If Examinator does the admin: erase layers (if any)
    if (m_bExternalAdmin == false)
    {
        mChartsInfo->clear();
    }

    // Default plotting color
    pushButtonLineColor->setActiveColor(QColor(0, 85, 0));

    checkBoxBars->setChecked(true);
    checkBox3DBars->setChecked(true);
    checkBoxStack->setChecked(false);
    checkBoxfittingCurve->setChecked(false);
    checkBoxLines->setChecked(true);
    checkBoxspots->setChecked(true);

    // Refresh the list of charts
    UpdateChartsList();

    // Refersh style structures
    OnChangeStyle();

    // Reset multi-chart widget
    m_pMultiChartWidget->resetChart(mAnchorAction->isChecked());

    // Default mode at startup: Histogram, Select mode
    // Bars in histogram
    // spinBoxHistBars->setValue(ReportOptions.iHistoClasses);
    OnHistogram();
    OnSelect();

    OnChangeDatasetWindow("");
}

/******************************************************************************!
 * \fn OnInteractiveTables
 * \brief Switch to the Interactive Tables display
 ******************************************************************************/

void GexWizardChart::OnSwitchInteractiveWafermap()
{
    pGexMainWindow->mWizardsHandler.SetWidgetToSwitch(this);

    pGexMainWindow->mWizardsHandler.RemoveWizardChart(this);
    OnInteractiveWafermap();
}

void GexWizardChart::OnInteractiveTables()
{
    pGexMainWindow->Wizard_DrillTable("", false);
}

QString GexWizardChart::BuildInteractiveLinkChart()
{
    // Build hyperlink argument string: drill_chart=adv_histo--data=<test#>
    QString strLink    = "";

    if (ptTestCell)
    {
        strLink  = "drill--drill_chart=adv_histo--data=";
        strLink += QString::number(ptTestCell->lTestNumber);

        // If pinmap exists, add it to URL
        if (ptTestCell->lPinmapIndex >= 0)
        {
            strLink += "." + QString::number(ptTestCell->lPinmapIndex);
        }
    }

    return strLink;
}

void GexWizardChart::OnInteractiveCharts(void)
{

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "DrillDataMining3D: on interactive charts...");

    pGexMainWindow->Wizard_DrillChart(BuildInteractiveLinkChart(), false);
}
/******************************************************************************!
 * \fn OnInteractiveWafermap
 * \brief Switch to the Interactive Wafermap display
 ******************************************************************************/

QString GexWizardChart::BuildInteractiveLinkWafermap()
{
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

    return strLink;
}

void GexWizardChart::OnInteractiveWafermap()
{
    pGexMainWindow->Wizard_Drill3D(BuildInteractiveLinkWafermap(), false);

   // if( pGexMainWindow->LastCreatedWizardWafer3D())
   //      pGexMainWindow->LastCreatedWizardWafer3D()->ShowChart(BuildInteractiveLinkWafermap());
}

void GexWizardChart::SetEnabledCharts(bool enabled)
{
    setEnabled(enabled);
    m_pMultiChartWidget->setEnabled(enabled);

    if(enabled == false)
    {
        resetPatMarker();
        m_pSingleChartWidget->deleteAbstractChart();
    }

    m_pSingleChartWidget->setEnabled(enabled);
}

/******************************************************************************!
 * \fn ~GexWizardChart
 * \brief Dialog box destructor
 ******************************************************************************/
GexWizardChart::~GexWizardChart()
{
    mWizardHandler->RemoveWizardChart(this);

    delete tmpReportOptions;
    tmpReportOptions = 0;

    delete mChartsInfo;
    delete verticalLayout;
}

/******************************************************************************!
 * \fn SelectWaferMapDiesForParameterRange
 ******************************************************************************/
void
GexWizardChart::SelectWaferMapDiesForParameterRange(double lfLow,
                                                    double lfHigh)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Select WaferMap Dies For ParameterRange %1 %2")
          .arg(lfLow)
          .arg(lfHigh).toLatin1().constData());
    if (mChartsInfo->chartsList().isEmpty())
    {
        return;
    }

    CGexSingleChart* pChart = mChartsInfo->chartsList().first();
    if (pChart == NULL)
    {
        return;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Select WaferMap Dies For ParameterRange for test %1").arg(
              pChart->iTestNumberX).toLatin1().constData());

    if ((pChart->iGroupX < 0) ||
            (pChart->iGroupX >= gexReport->getGroupsList().size()))
    {
        return;
    }
    // Get pointer to relevant group
    CGexGroupOfFiles* ptGroup = gexReport->getGroupsList().at(pChart->iGroupX);
    CGexFileInGroup* ptFile   = (ptGroup->pFilesList.isEmpty()) ? NULL : (ptGroup->pFilesList.first());
    CTest* ptLayerTestCell    = NULL;

    if (ptFile->FindTestCell(pChart->iTestNumberX, pChart->iPinMapX,
                             &ptLayerTestCell, false, false,
                             pChart->strTestNameX.toLatin1().data()) != 1)
    {
        return;
    }

    /*   Gex::IInteractiveWizard* lInteractiveWafer3D = pGexMainWindow->mWizardsHandler.FindWizard(Gex::T_WAFER3D, pChart->iTestNumberX, pChart->strTestNameX, pChart->iPinMapX);
    if(lInteractiveWafer3D == 0)
    {
        pGexMainWindow->mWizardsHandler.CreateWizardWaferMap3D(0, pChart->iTestNumberX, pChart->strTestNameX, pChart->iPinMapX);
    }*/


    pGexMainWindow->ShowWizardDialog(GEX_DRILL_3D_WIZARD_P1);
    Gex::DrillDataMining3D*  lWafer3DWizard = pGexMainWindow->LastCreatedWizardWafer3D();

    // case 2376
    lWafer3DWizard->ShowChart(pChart->iGroupX, 0, pChart->iTestNumberX, GEX_PTEST, GEX_DRILL_WAFERPROBE);

    // Select all dies where the parameter value is high enough
    if (gexReport->SelectWaferMapDies(ptGroup, ptFile, ptLayerTestCell, lfLow, lfHigh) == 0)
    {
        GS::Gex::Message::information("", "No Die matching your criteria!");
        return;
    }

    // redraw the 3D viewer
    lWafer3DWizard->refreshViewer();

    // Detach this Window so we can see side-by-side
    //lWafer3DWizard->ForceAttachWindow(false);
}



void  GexWizardChart::OnKeyUpDown()
{
   if(tabWidgetChart->currentIndex() == tabWidgetChart->indexOf(tabChart))
   {
       if(mTabFocusChartType)
       {
           comboBoxDataWindow->showPopup();
       }
       else
       {
           comboBoxChartType->showPopup();
       }
   }
   else if(tabWidgetChart->currentIndex() == tabWidgetChart->indexOf(tabMultiChart))
   {
       comboBoxDatasetMulti->showPopup();
   }
}

bool GexWizardChart::eventFilter(QObject *obj, QEvent *event)
{
    if(tabWidgetChart->currentIndex() == tabWidgetChart->indexOf(tabChart))
    {
        if( obj == this  && (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Tab  ||
                             static_cast<QKeyEvent*>(event)->key() == Qt::Key_Backtab) &&
                static_cast<QKeyEvent*>(event)->type() ==QEvent::KeyPress)
        {

             if(mTabFocusChartType == false)
            {
                comboBoxDataWindow->setStyleSheet( "border:  0.5px solid rgba(31, 132, 202, 255);"
                                                    "border-radius:4px;" );
                comboBoxChartType->setStyleSheet( "border:  none;" );
                comboBoxDataWindow->showPopup();
                mTabFocusChartType = true;

            }
            else
            {
                comboBoxChartType->setStyleSheet( "border:   0.5px solid rgba(31, 132, 202, 255);"
                                                  "border-radius:4px;" );
                comboBoxDataWindow->setStyleSheet( "border:  none;" );
                comboBoxChartType->showPopup();
                mTabFocusChartType = false;
            }
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

/******************************************************************************!
 * \fn keyPressEvent
 * \brief Handle: Key pressed...LEFT RIGHT
 ******************************************************************************/
void GexWizardChart::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
    case Qt::Key_Escape:
        e->accept();
        break;
    case Qt::Key_Left:
    case Qt::Key_PageUp:
        OnPreviousTest();
        e->accept();
        break;

    case Qt::Key_Down:
    case Qt::Key_Up:
        OnKeyUpDown();
        e->accept();
        break;
    case Qt::Key_Right:
    case Qt::Key_PageDown:
        OnNextTest();
        e->accept();
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
                    pGexMainWindow->OnNewWizard(GEX_DRILL_3D_WIZARD_P1, BuildInteractiveLinkWafermap());
                }
                else if(e->key() == Qt::Key_D)
                {
                    pGexMainWindow->OnNewWizard(GEX_CHART_WIZARD_P1, BuildInteractiveLinkChart());
                }
            }
        }
    }
    default:
        QDialog::keyPressEvent(e);
        e->ignore();
        break;
    }
}

///******************************************************************************!
// * \fn keyReleaseEvent
// * \brief Handle: Key depressed...
// ******************************************************************************/
//void GexWizardChart::keyReleaseEvent(QKeyEvent*  /*e*/)
//{
//    m_KeyboardStatus = 0;
//}

/******************************************************************************!
 * \fn onCurrentDieSelected
 * \brief Handle: Control click on trend chart
 ******************************************************************************/
void GexWizardChart::onCurrentDieSelected()
{
    // Ensure focus is on the Trend chart window
    activateWindow();
    comboBoxChartType->setFocus();
}

/******************************************************************************!
 * \fn isDataAvailable
 * \brief Tells if data available for charting
 ******************************************************************************/
bool GexWizardChart::isDataAvailable()
{
    if (gexReport == NULL)
    {
        return false;
    }

    // Get pointer to first group & first file
    pGroup =
            (gexReport->getGroupsList().size() > 0) ?
                (gexReport->getGroupsList().first()) : NULL;
    if (pGroup == NULL)
    {
        return false;
    }

    if (pGroup->pFilesList.count() <= 0)
    {
        return false;
    }

    pFile = pFileY =
            (pGroup->pFilesList.isEmpty()) ? NULL :
                                             (pGroup->pFilesList.first());
    if (pFile == NULL)
    {
        return false;
    }

    if (pFile->ptTestList == NULL)
    {
        return false;
    }

    if (ptTestCell == NULL)
    {
        ptTestCell = ptTestCellY = findFirstValidTestCell();


    }

    // In case we have multiple group, get pointer to second group
    if (gexReport->getGroupsList().count() >= 2)
    {
        pGroup = gexReport->getGroupsList().at(1);

        // Get pointer to second group
        pFileY =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                                                 (pGroup->pFilesList.first());

        // If pointers to tests not initialized, set them
        if (ptTestCellY == NULL)
        {
            ptTestCellY = findFirstValidTestCellY();
        }
    }

    // Some data available
    return true;
}

void GexWizardChart::OnTabRenamed( QWidget *widget, const QString &text )
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
 * \fn PlotRelevantChart
 * \brief Draw chart into Frame
 ******************************************************************************/
void GexWizardChart::PlotRelevantChart()
{
    // Check if samples available: data collection MAY have been
    // disabled for speed optimization reasons! + Display message
    CheckForDataAvailability();

    // If no test pointed yet...quietly return
    if (ptTestCell == NULL || gexReport == NULL)
    {
        return;
    }

    // Update navigation buttons + other fields
    UpdateSkin();

    // Copy fields we have to inherit from RepotsOptions
    tmpReportOptions->iGroups = ReportOptions.iGroups;
    tmpReportOptions->iFiles  = ReportOptions.iFiles;

    // If this is the first call, and user hasn't selected manually a test,
    // we have to force a refresh of some GUI + add default parameter to our list
    if (mChartsInfo->chartsList().count() == 0)
    {
        // Fill Parameter list with default Parameter to plot
        addChart(true, true, ptTestCell);

        // Select 1st parameter in combo-box list
        comboBoxChartLayer->setCurrentIndex(0);

        // Update the 'Style' tab GUI content to reflect parameters options
        OnChangeChartLayer("");
        OnChangeStyle();

    }

    // Force screen to refresh
    m_pSingleChartWidget->replotChart(chartReportOptions(), currentTestCellX(), currentTestCellY());
}

/******************************************************************************!
 * \fn ListRelevantRawData
 * \brief List Raw data into Table
 ******************************************************************************/
void GexWizardChart::ListRelevantRawData()
{
    // Show/Hide "Remove data points" icon
    switch (GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
    case GS::LPPlugin::LicenseProvider::eLtxcOEM:   // OEM-Examinator for LTXC
    case GS::LPPlugin::LicenseProvider::eSzOEM:     // OEM-Examinator for Credence SZ
        break;

    default:
        switch (m_eChartType)
        {
        case GexAbstractChart::chartTypeHistogram:
        case GexAbstractChart::chartTypeTrend:
        case GexAbstractChart::chartTypeBoxPlot:
        case GexAbstractChart::chartTypeProbabilityPlot:
            buttonRemoveData->show();
            break;

        case GexAbstractChart::chartTypeScatter:
        case GexAbstractChart::chartTypeBoxWhisker:
            buttonRemoveData->hide();
            break;

        default:
            GEX_ASSERT(false);
            break;
        }
    }

    // If no test pointed yet...quietly return
    if (ptTestCell == NULL)
    {
        return;
    }

    // If lots of samples, first ask confirmation to load the table!
    if (ptTestCell->ldSamplesValidExecs > 5000)
    {
        QString strMessage;
        strMessage  = "This is a very large data collection: ";
        strMessage += QString::number(ptTestCell->ldSamplesExecs);
        strMessage += " samples\nDo you confirm to show them all?";
        bool lOk;
        GS::Gex::Message::request("", strMessage, lOk);
        if (! lOk)
        {
            tabWidgetChart->setCurrentIndex(0);  // Switch back to the "Chart" tab
            return;  // No!...quiet return
        }
    }

    // Update navigation buttons + other fields
    UpdateSkin();

    // Save 'options'
    CReportOptions cOptionsBackup = ReportOptions;
    // Copy fields we have to inherit from RepotsOptions
    tmpReportOptions->iGroups = ReportOptions.iGroups;
    tmpReportOptions->iFiles  = ReportOptions.iFiles;

    // Switch to 'Interactive charting' options
    ReportOptions = *tmpReportOptions;

    // Hide table when loading data to avoid flickering
    tableWidget->hide();

    switch (m_eChartType)
    {
    case GexAbstractChart::chartTypeHistogram:
    case GexAbstractChart::chartTypeTrend:
    case GexAbstractChart::chartTypeBoxPlot:
        // Draw BoxPlot in Frame window
    case GexAbstractChart::chartTypeProbabilityPlot:
        // Dump Raw data in the table
        gexReport->ListAdvTrendData(mChartsInfo,tableWidget, pFile, ptTestCell);
        break;

    case GexAbstractChart::chartTypeScatter:
    case GexAbstractChart::chartTypeBoxWhisker:
        // Dump Raw data in the table
        gexReport->ListAdvScatterData(mChartsInfo, tableWidget, pFile, ptTestCell, ptTestCellY);
        break;

    default:
        GEX_ASSERT(false);
        break;
    }

    tableWidget->show();

    // Restore default 'Options'
    ReportOptions = cOptionsBackup;
}

/******************************************************************************!
 * \fn UpdateSkin
 * \brief Draw chart into Frame
 ******************************************************************************/
void GexWizardChart::UpdateSkin()
{
    switch (m_eChartType)
    {
    case GexAbstractChart::chartTypeTrend:
        // Draw Trend in Frame window
    case GexAbstractChart::chartTypeHistogram:
        // Draw Histogram in Frame window
    case GexAbstractChart::chartTypeBoxPlot:
        // Draw BoxPlot in Frame window
        spinBoxLineWidth->show();
        textLabelLineWidth->show();
        textLabelLineStyle->show();
        comboBoxLineStyle->show();
        widgetStackDrawingMode->show();
        textLabelParamX->setText("");
        textLabelParamY->setText("");
        buttonPreviousTestY->hide();
        buttonNextTestY->hide();
        mPickTestYAction->setEnabled(false);
        break;
    case GexAbstractChart::chartTypeScatter:
        // Draw Scatter plot in Frame window
    case GexAbstractChart::chartTypeBoxWhisker:
        spinBoxLineWidth->show();
        textLabelLineWidth->show();
        textLabelLineStyle->show();
        comboBoxLineStyle->show();
        if (m_bSupportBoxWhisker)
        {
            widgetStackDrawingMode->show();
        }
        else
        {
            widgetStackDrawingMode->hide();
        }
        textLabelParamX->setText("X Parameter");
        textLabelParamY->setText("Y Parameter");
        buttonPreviousTestY->show();
        buttonNextTestY->show();
        mPickTestYAction->setEnabled(true);
        break;
    case GexAbstractChart::chartTypeProbabilityPlot:
        // Draw probability plot in Frame window
        widgetStackDrawingMode->hide();
        spinBoxLineWidth->hide();
        textLabelLineWidth->hide();
        textLabelLineStyle->hide();
        comboBoxLineStyle->hide();
        textLabelParamX->setText("");
        textLabelParamY->setText("");
        buttonPreviousTestY->hide();
        buttonNextTestY->hide();
        mPickTestYAction->setEnabled(false);
        break;
    default:
        GEX_ASSERT(false);
        break;
    }
    if (m_eChartType == GexAbstractChart::chartTypeProbabilityPlot)
    {
        checkBoxQQLine->show();
        checkBox1Sigma->show();
        checkBox15Sigma->show();
        checkBox3Sigma->show();
        checkBox6Sigma->show();
    } else {
        checkBoxQQLine->hide();
        checkBox1Sigma->show();
        checkBox15Sigma->show();
        checkBox3Sigma->show();
        checkBox6Sigma->show();
    }

    ShowHideRollingLimitsMarker();
}

/******************************************************************************!
 * \fn pluginShowChart
 * \brief Draw chart from layers built within plugin
 ******************************************************************************/
void GexWizardChart::pluginShowChart(int iRendering)
{
    // Save Rendering mode. eg: GEX_ADV_SCATTER
    switch (iRendering)
    {
    case GEX_ADV_HISTOGRAM:        OnHistogram();       break;
    case GEX_ADV_TREND:            OnTrend();           break;
    case GEX_ADV_CORRELATION:      OnScatter();         break;
    case GEX_ADV_CANDLE_MEANRANGE: OnBoxPlot();         break;
    case GEX_ADV_PROBABILITY_PLOT: OnProbabilityPlot(); break;
    default: /*GEX_ASSERT(false)*/; break;
    }

    // Set Viewport (over limits, over data, etc...)
    comboBoxDataWindow->setCurrentIndex(
                ReportOptions.getAdvancedReportSettings() - 1);
    OnChangeDatasetWindow("");

    // Display chart
    ShowChart();
}

/******************************************************************************!
 * \fn ShowChart
 * \brief Draw chart into Frame (caller is Web link)
 ******************************************************************************/
void GexWizardChart::ShowChart(QString strLink)
{
    int iGroupX = -1;

    // Enable/disable some features...
    switch (GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
    case GS::LPPlugin::LicenseProvider::eLtxcOEM:   // OEM-Examinator for LTXC
        // Disable 'Multichart' tab
        tabWidgetChart->setTabEnabled(tabWidgetChart->indexOf(tabWidgetChart->widget(1)), false);
        // Disable 'Table' tab: so no Raw data visible
        tabWidgetChart->setTabEnabled(tabWidgetChart->indexOf(tabWidgetChart->widget(2)), false);
        // Disable 'Layers' tab
        tabWidgetStyle->setTabEnabled(tabWidgetStyle->indexOf(tabWidgetStyle->widget(0)), false);
        break;
    case GS::LPPlugin::LicenseProvider::eSzOEM:
        // OEM-Examinator for Credence SZ
        break;
    default:
        break;
    }

    if (pFile == NULL)
    {
        return;
    }

    // We have data...but we do not have any test selected yet...
    // then force it to the 1st test available
    if (ptTestCell == NULL)
    {
        // 1st test# in the list
        ptTestCell = findFirstValidTestCell();
    }

    if (ptTestCellY == NULL)
    {
        // 1st test# (on Y axis) in the list
        ptTestCellY = findFirstValidTestCellY();
    }

    //-- No Data to draw a chart
    if(ptTestCell == NULL)
        return;

    // Active boxwhisker capabilities according to the option
    m_bSupportBoxWhisker =
            ReportOptions.GetOption("adv_correlation", "chart_type").isValid();

    // update the ofr view
    updateOFRManagerView();

    if (strLink.isNull() || strLink.isEmpty())
    {
        tabWidgetChart->setCurrentIndex(tabWidgetChart->indexOf(tabChart));

        // Refresh current chart (if any)
        PlotRelevantChart();

        // Select 1st parameter in combo-box list
        comboBoxChartLayer->setCurrentIndex(0);

        // Update the 'Style' tab GUI content to reflect parameters options
        OnChangeChartLayer("");

        QString prefix = PrefixTab;
        QString suffix = ptTestCell->GetTestNumber();

        pGexMainWindow->ChangeWizardTabName(this, prefix, suffix);

        return;
    }

    // Clear zoom
    mChartsInfo->lfZoomFactorX   =
            mChartsInfo->lfZoomFactorY = -1.0;
    m_pSingleChartWidget->resetViewPortManager();
    m_pMultiChartWidget->resetViewPortManager();

    // Extract chart type to Drill into
    bool bMultiChart = false;
    bool bXBarR      = false;
    int  iArgSection = 1;

    // line is like:
    QString strString = strLink.section("--", iArgSection, iArgSection);
    //= "drill_chart=<type>"
    QString strField = strString.section('=', 0, 0);
    // = "drill_chart"
    QString strChartType = strString.section('=', 1, 1);
    // one of: "histo", "adv_histo", "adv_trend", "adv_scatter"...

    // Check which Charting type selected...
    if (strChartType == "histo" || strChartType == "adv_histo")
    {
        OnHistogram();
    }
    else if (strChartType == "adv_trend")
    {
        OnTrend();
    }
    else if (strChartType == "adv_scatter")
    {
        radioButtonScatter->setChecked(true);
        OnScatter();
    }
    else if (strChartType == "adv_boxwhisker")
    {
        radioButtonBoxWhisker->setChecked(true);
        OnScatter();
    }
    else if (strChartType == "adv_boxplot")
    {
        OnBoxPlot();
    }
    else if (strChartType == "adv_probabilityplot")
    {
        OnProbabilityPlot();
    }
    else if (strChartType == "adv_xbarr")
    {
        bXBarR = true;
    }
    else if (strChartType == "adv_multichart")
    {
        bMultiChart = true;
    }

    // Extract Test# to see
    iArgSection++;
    strString = strLink.section("--", iArgSection, iArgSection);
    // = "data=<Test#> or "data=Test_List <test#>;<test#>;<test#>
    strField = strString.section('=', 1, 1);  // = test#
    if (strField.isEmpty())
    {
        // Refresh current chart (if any)
        if (bMultiChart)
        {
            tabWidgetChart->setCurrentIndex(tabWidgetChart->indexOf(tabMultiChart));

            activateMuiltiChartTab();
        }
        else if (bXBarR)
        {
            tabWidgetChart->setCurrentIndex(tabWidgetChart->indexOf(tabXBarR));

            activateXBarRChartTab();
        }
        else
        {
            tabWidgetChart->setCurrentIndex(tabWidgetChart->indexOf(tabChart));

            activateChartTab();
        }
        return;
    }

    QString strAction;
    QString strTestNameX, strTestNameY;
    char    szTestName[256];
    char*   ptTestName;
    ptTestName = szTestName;

    // Check if multiple tests (test list) is given as argument...
    // if so, then we have to display in multi-layer mode
    if (strField.startsWith("Test_List", Qt::CaseInsensitive))
    {
        QStringList strTestList;
        strString   = strField.section(' ', 1); // Extract test list
        strTestList = strString.split(";");  // Each test from list
        int nIndex;
        for (nIndex = 0; nIndex < strTestList.count(); nIndex++)
        {
            strField     = strTestList[nIndex];
            *szTestName  = 0; // Reset test name
            lPinmapIndex = GEX_PTEST;  // No Pinmap index specified
            ptTestName   = NULL;
            if (strField.isEmpty())
            {
                break;
            }
            if (sscanf(strField.toLatin1().constData(), "%lu%*c%ld%*c%s",
                       &lTestNumber, &lPinmapIndex, szTestName) < 2)
            {
                ptTestName = NULL;  // No name available...
            }
            else
            {
                strTestNameX = gexReport->BuildParameterNameFromURL(szTestName);
            }

            if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false,
                                    false, ptTestName) != 1)
            {
                ptTestCell = NULL;
                return;  // Test not found...should never occur!
            }
            if (strTestNameX.isEmpty())
            {
                strTestNameX = ptTestCell->strTestName;
            }

            // Save pointer to previous test
            m_pPrevTestX = pFile->PrevTest(ptTestCell);

            // Add layer ot list
            addChart(true, (nIndex == 0), lTestNumber, lPinmapIndex, szTestName, 0);
        }

        // Mode only used if interactive table selects to
        // trend/histogram multiple tests
        goto plot_chart;
    }

    lPinmapIndex = GEX_PTEST;  // No Pinmap index specified
    lPinmapIndex = -1;
    switch (sscanf(strField.toLatin1().constData(), "%lu%*c%ld%*c%s",
                   &lTestNumber, &lPinmapIndex, szTestName))
    {
    case 0:
    case 1:  // Test#, no pin, no name
        ptTestName = NULL;
        break;
    case 2:  // Test#, Pin#, no name
        ptTestName = NULL;  // No name available
        break;
    case 3:  // Test#, Pin#, no name
        strTestNameX = gexReport->BuildParameterNameFromURL(szTestName);
        break;
    }

    // Group
    ++iArgSection;
    strString = strLink.section("--", iArgSection, iArgSection);
    if (sscanf(strString.toLatin1().constData(), "g=%d", &iGroupX) == 1)
    {
        if (gexReport->getGroupsList().size() < iGroupX)
        {
            GSLOG(SYSLOG_SEV_WARNING, "bad group");
            ptTestCell = NULL;
            return;
        }
        pGroup = gexReport->getGroupsList().at(iGroupX);
        if (pGroup == NULL)
        {
            GSLOG(SYSLOG_SEV_WARNING, "group not found");
            ptTestCell = NULL;
            return;
        }
        if (pGroup->pFilesList.isEmpty())
        {
            GSLOG(SYSLOG_SEV_WARNING, "no file in group");
            ptTestCell = NULL;
            return;
        }
        pFile = pGroup->pFilesList.first();
    }
    else
    {
        --iArgSection;
    }

    if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false, false
                            , ptTestName) != 1)
    {
        ptTestCell = NULL;
        return;  // Test not found...should never occur!
    }
    if (strTestNameX.isEmpty())
    {
        strTestNameX = ptTestCell->strTestName;
    }
    if (iGroupX != -1)
    {
        this->getChartInfo()->getViewportRectangle()[m_eChartType].
            cChartOptions.bChartTitle = true;
        this->getChartInfo()->getViewportRectangle()[m_eChartType].
            cChartOptions.strChartTitle = strTestNameX;
    }

    // Save pointer to previous test
    m_pPrevTestX = pFile->PrevTest(ptTestCell);

    // Next section: Either Y Test# or Marker for X axis
    iArgSection++;
    strString = strLink.section("--", iArgSection, iArgSection);
    // = "data=<Test#>  or "marker=<layer> <Run#> <value> <MarkerLabel>
    strAction = strString.section('=', 0, 0);  // = 'data' or 'marker'
    strField  = strString.section('=', 1, 1);  // = test# to plot in Y

    // If Marker...
    if (strAction == "marker")
    {
# if 0
        // Remove all outlier markers that already exist (must scan all groups)
        pGroup = gexReport->getGroupsList().size() >
                0 ? gexReport->getGroupsList().first() : NULL;
        TestMarker* ptMark;
        while (pGroup != NULL)
        {
            pFile =
                    (pGroup->pFilesList.isEmpty()) ? NULL :
                                                     (pGroup->pFilesList.first());
            if (pFile == NULL)
            {
                return;
            }

            if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false,
                                    false, true, ptTestName) != 1)
            {
                ptTestCell = NULL;
                return;  // Test not found...should never occur!
            }

            // Remove all outlier markers found in this group and for this test
            ptMark = ptTestCell->ptCustomMarkers.first();
            while (ptMark != NULL)
            {
                if (ptMark->strLabel.startsWith("outlier", Qt::CaseInsensitive))
                {
                    // Remove existing custom markers of type "outlier"
                    ptTestCell->ptCustomMarkers.remove(ptMark);
                }
                ptMark = ptTestCell->ptCustomMarkers.next();
            }

            // Check next group
            pGroup = gexReport->getGroupsList().next();
        }
#endif
        // Remove all outlier markers that already exist (must scan all groups)
        gexReport->ClearCustomMarkers("outlier");

        QString strMarkerName;
        int     iSite, iGroup = 0;
        int     iSiteMarker, iMarkerDieX, iMarkerDieY;
        long    lRunID;
        double  lfMarkerPos;
        // Format: <site#> <RunID> <DieX> <DieY> <value> <MakerLabel>
        strMarkerName = strField.section(' ', 4);

        // Extract: <Site#> <Run#> <MarkerPos>
        if (sscanf(strField.toLatin1().constData(), "%d %ld %d %d %lf",
                   &iSiteMarker, &lRunID,
                   &iMarkerDieX, &iMarkerDieY, &lfMarkerPos) != 5)
        {
            return;  // parsing error...should never occur!

        }
        // Point to relevant group & file list

        if(ptTestCell == NULL)
        {
            pGroup = (gexReport->getGroupsList().isEmpty()) ? NULL :
                                                              gexReport->getGroupsList().at(iGroup);
            if (gexReport->getGroupsList().count() > 1)
            {
                // Multi-sites
                while (pGroup != NULL)
                {
                    // Extract site# from group name since group name string is 'Site %d'
                    if (sscanf(pGroup->strGroupName.toLatin1().constData(), "%*s %d",
                               &iSite) != 1)
                    {
                        return;  // parsing error...should never occur!
                    }
                    if (iSite == iSiteMarker)
                    {
                        break;
                    }

                    // Not correct group yet, move to next
                    iGroup++;
                    pGroup = (iGroup >= gexReport->getGroupsList().size()) ? NULL :
                                                                             gexReport->getGroupsList().at(iGroup);
                }
            }
            if (pGroup == NULL)
            {
                return;
            }

            pFile =
                    (pGroup->pFilesList.isEmpty()) ? NULL :
                                                     (pGroup->pFilesList.first());
            if (pFile == NULL)
            {
                return;
            }
            if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false,
                                    false, ptTestName) != 1)
            {
                ptTestCell = NULL;
                return;  // Test not found...should never occur!
            }
        }

        // In multigroup, the RunID is not usable as it is the Run#
        // when all sites are merged...we need to recompute it from relevant group!
        if (gexReport->getGroupsList().count() > 1)
        {
            CTest* ptDieX, * ptDieY;
            int    iDieX, iDieY;
            long   lPartId;
            double lfValue;
            // Get handle to Die X,Y info
            if (pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &ptDieX,
                                    true, false) != 1)
            {
                return;
            }
            if (pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &ptDieY,
                                    true, false) != 1)
            {
                return;
            }

            // Scale Marker vamue to be in same unit as raw data
            lfValue = lfMarkerPos / ScalingPower(ptTestCell->res_scal);

            // Check testResults size
            // (ensure that lPartId is correct in next for loop)
            int nSamplesExecsCount = gex_min(ptDieX->m_testResult.count(),
                                             ptDieY->m_testResult.count());
            nSamplesExecsCount = gex_min(nSamplesExecsCount,
                                         ptTestCell->m_testResult.count());


            // Scan all parts tested
            for (lPartId = 0; lPartId < nSamplesExecsCount; lPartId++)
            {
                iDieX = int (ptDieX->m_testResult.resultAt(lPartId));
                iDieY = int (ptDieY->m_testResult.resultAt(lPartId));

                if ((iDieX == iMarkerDieX) && (iDieY == iMarkerDieY))
                {
                    // We've found the data point!
                    if (fabs(lfValue - ptTestCell->m_testResult.resultAt(lPartId)) <
                            1e-3)
                    {
                        // We allow 1/1000 error over marker value and real value
                        // (due to string encoding!)
                        // RunID starts at one while array starts at 0!
                        lRunID = lPartId + 1;
                        break;
                    }
                }
            }
        }


        TestMarker* ptMark = new TestMarker();
        ptMark->strLabel = strMarkerName;
        ptMark->iLayer   = iGroup;    // Marker sticky to a specific layer
        ptMark->lfPos    = lfMarkerPos;
        ptMark->lRunID   = lRunID;
        ptMark->iLine    = 2;
        ptMark->cColor   = QColor(0, 0, 255);
        // Force to ALWAYS use this color, even if multi-layer mode
        // (where normally markers share same color as layer chart)
        ptMark->bForceColor = true;

        ptTestCell->ptCustomMarkers.append(ptMark);

        if (bMultiChart)
        {
            comboBoxDatasetMulti->setCurrentIndex(2);

            OnChangeDatasetMulti("");
        }
        else
        {
            // Ensure charting is over test results or adaptive so marker is visible!
            if (comboBoxDataWindow->currentIndex() == 0)
            {
                // Select last option in drop-down: 'adaptive mode')
                comboBoxDataWindow->setCurrentIndex(comboBoxDataWindow->count() - 1);
                OnChangeDatasetWindow("");
            }
        }

        // Parse next fields (if any)
        iArgSection++;
        strString = strLink.section("--", iArgSection, iArgSection);
        // = "data=<Test#>  or "marker=<name> <value>
        strAction = strString.section('=', 0, 0);  // = 'data' or 'marker'
        strField  = strString.section('=', 1, 1);  // = test# to plot in Y
    }

    int iGroupY;
    unsigned int iTestX, iTestY;
    int iPinmapX, iPinmapY;
    iGroupY  = -1;
    iTestX   = lTestNumber;
    iPinmapX = lPinmapIndex;
    iTestY   = 0;
    iPinmapY = -1;

    // In case it is a scatter plot, we have 2 parameters
    // Extract Test# in Y (group#2) to see
    if (strField.isEmpty() == false)
    {
        ptTestName   = szTestName;
        lPinmapIndex = GEX_PTEST;  // No Pinmap index specified

        switch (sscanf(strField.toLatin1().constData(), "%lu%*c%ld%*c%s",
                       &lTestNumber, &lPinmapIndex, szTestName))
        {
        case 0:
        case 1:  // Test#, no pin, no name
            ptTestName = NULL;
            break;
        case 2:  // Test#, Pin#, no name
            ptTestName = NULL;  // No name available
            break;
        case 3:  // Test#, Pin#, no name
            strTestNameY = gexReport->BuildParameterNameFromURL(szTestName);
            break;
        }

        if (pFileY->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCellY, false,false, ptTestName) != 1)
        {
            ptTestCellY = NULL;
            return;  // Test not found...should never occur!
        }
        if (strTestNameY.isEmpty())
        {
            strTestNameY = ptTestCellY->strTestName;
        }

        iTestY   = lTestNumber;
        iPinmapY = lPinmapIndex;
        iGroupX  = 0;
        // If two datasets exists, then Y axis is on second dataset
        iGroupY = (gexReport->getGroupsList().count() == 2) ? 1 : 0;

        // Save pointer to previous test
        m_pPrevTestY = pFileY->PrevTest(ptTestCellY);
    }

    // Make this chart replace the current overlays
    addChart(true,
             true,
             iTestX,
             iPinmapX,
             strTestNameX,
             iGroupX,
             iTestY,
             iPinmapY,
             strTestNameY,
             iGroupY);

    // Plot relevant Chart (if any)
plot_chart:

    if (bMultiChart)
    {
        tabWidgetChart->setCurrentIndex(tabWidgetChart->indexOf(tabMultiChart));

        activateMuiltiChartTab();
    }
    else if (bXBarR)
    {
        tabWidgetChart->setCurrentIndex(tabWidgetChart->indexOf(tabXBarR));

        activateXBarRChartTab();
    }
    else
    {
        tabWidgetChart->setCurrentIndex(tabWidgetChart->indexOf(tabChart));

        activateChartTab();

        // Replot as first plot is using incorrect X scale
        goHome();
    }

    QString prefix = PrefixTab;
    QString suffix = ptTestCell->GetTestNumber();

    pGexMainWindow->ChangeWizardTabName(this, prefix, suffix);

    if(ptTestCell)
    {
        m_pPrevTestX = pFile->PrevTest(ptTestCell);
        if(m_pPrevTestX)
        {
             buttonPreviousTest->setEnabled(true);
        }
    }
}

/******************************************************************************!
 * \fn OnSelect
 * \brief Selection mode reset: Mouse cursor
 ******************************************************************************/
void GexWizardChart::OnSelect()
{
    // Set selection mode on chartdirector object
    m_pSingleChartWidget->setMouseMode(GexSingleChartWidget::mouseSelect);


    toolButtonMouseMode->setIcon(mIconsMouse[E_ICON_SELECT]);
    // Untoggle other selection mode buttons
    // buttonSelect->setChecked(true);
    // buttonZoom->setChecked(false);
    // buttonMove->setChecked(false);

    mSelectZone->
            setEnabled(gexReport->getGroupsList().count() == 1 &&
                       (m_eChartType != GexAbstractChart::chartTypeScatter &&
            m_eChartType != GexAbstractChart::chartTypeBoxWhisker));
}

/******************************************************************************!
 * \fn OnDrag
 * \brief Selection mode: Drag draw area
 ******************************************************************************/
void GexWizardChart::OnDrag()
{
    // Set selection mode on chartdirector object
    m_pSingleChartWidget->setMouseMode(GexSingleChartWidget::mouseDrag);

    toolButtonMouseMode->setIcon(mIconsMouse[E_ICON_MOVE]);
    // Untoggle other selection mode buttons
    //buttonMove->setChecked(true);
    //buttonSelect->setChecked(false);
    //buttonZoom->setChecked(false);

    mSelectZone->
            setEnabled(gexReport->getGroupsList().count() == 1 &&
                       (m_eChartType != GexAbstractChart::chartTypeScatter &&
            m_eChartType != GexAbstractChart::chartTypeBoxWhisker));
}

/******************************************************************************!
 * \fn OnZoom
 * \brief Enter in zooming mode
 ******************************************************************************/
void GexWizardChart::OnZoom()
{
    // Set selection mode on chartdirector object
    m_pSingleChartWidget->setMouseMode(GexSingleChartWidget::mouseZoom);

    toolButtonMouseMode->setIcon(mIconsMouse[E_ICON_ZOOM]);
    // Untoggle other selection mode buttons
    //buttonZoom->setChecked(true);
    //buttonSelect->setChecked(false);
    //buttonMove->setChecked(false);

    mSelectZone->
            setEnabled(gexReport->getGroupsList().count() == 1 &&
                       (m_eChartType != GexAbstractChart::chartTypeScatter &&
            m_eChartType != GexAbstractChart::chartTypeBoxWhisker));
}

/******************************************************************************!
 * \fn onSelectZone
 * \brief Enter in zone selection mode
 ******************************************************************************/
void GexWizardChart::onSelectZone()
{
    // OEM-Examinator for LTXC
    if (GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Set selection mode on chartdirector object
    m_pSingleChartWidget->setMouseMode(GexSingleChartWidget::mouseZoneSelect);

    // Untoggle other selection mode buttons
    mSelectZone->
            setEnabled(gexReport->getGroupsList().count() == 1 &&
                       (m_eChartType != GexAbstractChart::chartTypeScatter &&
            m_eChartType != GexAbstractChart::chartTypeBoxWhisker));


    toolButtonMouseMode->setIcon(mIconsMouse[E_ICON_SPLIT]);
}

/******************************************************************************!
 * \fn OnChangeDatasetWindow
 * \brief Data set window to consider: Test limits or All samples, etc.
 ******************************************************************************/
void GexWizardChart::OnChangeDatasetWindow(const QString&  /*strSelection*/)
{
    switch (m_eChartType)
    {
    case GexAbstractChart::chartTypeHistogram:
        // Histogram chart
        m_iHistoViewport =
                comboBoxDataWindow->itemData(comboBoxDataWindow->currentIndex()).toInt();

        m_pSingleChartWidget->setViewportMode(m_iHistoViewport);
        break;

    case GexAbstractChart::chartTypeTrend:
        // Draw Trend in Frame window
        m_iTrendViewport =
                comboBoxDataWindow->itemData(comboBoxDataWindow->currentIndex()).toInt();

        m_pSingleChartWidget->setViewportMode(m_iTrendViewport);
        break;

    case GexAbstractChart::chartTypeScatter:
        // Draw Scatter plot in Frame window
    case GexAbstractChart::chartTypeBoxWhisker:
        m_iScatterViewport =
                comboBoxDataWindow->itemData(comboBoxDataWindow->currentIndex()).toInt();

        m_pSingleChartWidget->setViewportMode(m_iScatterViewport);
        break;

    case GexAbstractChart::chartTypeBoxPlot:
        // Draw BoxPlot in Frame window
        m_iBoxPlotViewport =
                comboBoxDataWindow->itemData(comboBoxDataWindow->currentIndex()).toInt();

        m_pSingleChartWidget->setViewportMode(m_iBoxPlotViewport);
        break;

    case GexAbstractChart::chartTypeProbabilityPlot:
        m_iProbabilityViewport =
                comboBoxDataWindow->itemData(comboBoxDataWindow->currentIndex()).toInt();

        m_pSingleChartWidget->setViewportMode(m_iProbabilityViewport);
        break;

    default:
        GEX_ASSERT(false);
        break;
    }

    // Reset Zoom/Drag flags + replot
    goHome();
}

/******************************************************************************!
 * \fn OnChangeDatasetMulti
 ******************************************************************************/
void GexWizardChart::OnChangeDatasetMulti(const QString&)
{
    m_nMultiChartViewport = comboBoxDatasetMulti->itemData(
                comboBoxDatasetMulti->currentIndex()).toInt();

    m_pMultiChartWidget->setViewportMode(m_nMultiChartViewport);

    goHomeMulti();
}

/******************************************************************************!
 * \fn clearChart
 * \brief Remove one or ALL charts
 ******************************************************************************/
void GexWizardChart::clearChart(int iChartID)
{
    if (iChartID < 0)
    {
        // Clear ALL charts
        mChartsInfo->removeCharts();
    }
    else
    {
        // Erase a specific ChartID only, keep the other charts overlays
        mChartsInfo->removeChart(iChartID);
    }
}

/******************************************************************************!
 * \fn addChart
 * \brief Add one chart to the list of charts overlays
 ******************************************************************************/
void
GexWizardChart::addChart(bool   bUpdateGUI,
                         bool   bClear,
                         CTest* ptTestCellX,
                         CTest* ptTestCellY  /*=NULL*/,
                         int    iGroupX  /*=0*/,
                         int    iGroupY  /*=0*/)
{
    if (ptTestCellY == NULL)
    {
        addChart(bUpdateGUI, bClear,
                 ptTestCellX->lTestNumber,
                 ptTestCellX->lPinmapIndex,
                 ptTestCellX->strTestName,
                 iGroupX, (unsigned) -1, -1, "", 0);  // Only one test
    }
    else
    {
        addChart(bUpdateGUI, bClear,
                 ptTestCellX->lTestNumber,
                 ptTestCellX->lPinmapIndex,
                 ptTestCellX->strTestName, iGroupX,
                 ptTestCellY->lTestNumber,
                 ptTestCellY->lPinmapIndex,
                 ptTestCellY->strTestName, iGroupY);  // Two tests
    }
}

bool GexWizardChart::buildTreeLayerHeader(const QString& aChartName)
{
    // -- Extract value of filter and recreate the displayed name
    QList<QString> lFilterNames;
    QList<QString> lFilterValues;
    bool lIsResquestName = mQxChartNameSpliteredExtractor.SeparateSplitsNameValue(aChartName,
                                                            lFilterNames,
                                                            lFilterValues);
    if(lIsResquestName == false)
    {
        return false;
    }

    QString lTitle = mQxChartNameSpliteredExtractor.ExtractRequestTitle(aChartName);

    QString lHeaderName = QStringLiteral("Layers");
    if(lTitle.isEmpty() == false)
    {
        lHeaderName += QStringLiteral(" - ") + lTitle ;
    }

    QList<QString>::iterator lIter(lFilterNames.begin());
    QList<QString>::iterator lIterEnd(lFilterNames.end());
    for(; lIter != lIterEnd; ++lIter)
    {
        lHeaderName += QStringLiteral("[") + *lIter + QStringLiteral("]");
    }

    treeWidgetLayers->headerItem()->setText(0, lHeaderName);

    return true;
}

bool  GexWizardChart::formatRequestDBName(QString& aChartName)
{
    // -- Extract value of filter and recreate the displayed name
    QList<QString> lFilterNames;
    QList<QString> lFilterValues;
    bool lIsResquestName = mQxChartNameSpliteredExtractor.SeparateSplitsNameValue(aChartName,
                                                            lFilterNames,
                                                            lFilterValues);

    if(lIsResquestName == false)
    {
        return false;
    }

    //-- rebuild the name
    QString lDisplayedName;
    QList<QString>::iterator lIter(lFilterValues.begin());
    QList<QString>::iterator lIterEnd(lFilterValues.end());
    for(; lIter != lIterEnd; ++lIter)
    {
        lDisplayedName += QStringLiteral("[") + *lIter + QStringLiteral("]");
    }

   /* QString lName = mQxChartNameSpliteredExtractor.ExtractTestName(aChartName);
    if(lName.isEmpty() == false)
    {
        lDisplayedName +=  QStringLiteral(" ") + lName;
    }*/

    if(lDisplayedName.isEmpty() == false)
    {
        aChartName = lDisplayedName;
    }

    return true;
}


/******************************************************************************!
 * \fn addChart
 * \brief Add one chart to the list of charts overlays
 ******************************************************************************/
void
GexWizardChart::addChart(bool         bUpdateGUI,
                         bool         bClear,
                         unsigned int iTestX,
                         int          iPinmapX,
                         QString      strNameX,
                         int          iGroupX,
                         unsigned int iTestY,
                         int          iPinmapY,
                         QString      strNameY,
                         int          iGroupY,
                         bool         /*usedExistingChart*/)
{
    // bool bDoClear = bClear;

    // Save visible attribute
    bool* visible      = NULL;
    int   visibleCount = 0;
    if (mAnchorAction->isChecked())
    {
        visibleCount = mChartsInfo->chartsList().count();
        if (visibleCount > 0)
        {
            visible = new bool[visibleCount];
            for (int i = 0; i < visibleCount; ++i)
            {
                visible[i] = mChartsInfo->chartsList().at(i)->bVisible;
            }
        }
    }

#if 0
    // Don't know the reason for this code. But it creates a problem for
    // files having a test with TestNumber=0 (won't display in interactive charts)
    if (iTestX == 0)
    {
        if (bDoClear)
        {
            clearChart();
        }
        return;  // no valid test to add...simply reset
    }
#endif

    // May create ONE layer (if specific group specified) or
    // ALL layers (as many as group, if groupID = -1)
    int     iGroupIDX, iGroupIDY;
    QString strString;
    CGexGroupOfFiles* pTestGroup;
    if (iGroupX >= 0 && iGroupX < gexReport->getGroupsList().count())
    {
        iGroupIDX = iGroupX;
    }
    else
    {
        iGroupIDX = 0;
    }

    if (iGroupY >= 0 && iGroupY < gexReport->getGroupsList().count())
    {
        iGroupIDY = iGroupY;
    }
    else
    {
        iGroupIDY = 0;
    }

    CGexSingleChart* pChart, * pLayerStyle;
    int iLayerID = 0;

    bool lFromDefaultChartInfo = true;

    if (bClear && mChartsInfo->chartsList().count() > 0)
    {
        int lNbSites = mChartsInfo->chartsList().size();
        QList<CGexSingleChart*> lTempoListChart;
        for (int i = 0; i < lNbSites; ++i)
        {
            lTempoListChart.
                append(new CGexSingleChart(*mChartsInfo->chartsList()[i]));
            if (iGroupX >= 0)
            {
                break;
            }
        }

        clearChart();

        for (int i = 0; i < lNbSites; ++i)
        {
            mChartsInfo->addChart(lTempoListChart[i]);
            if (iGroupX >= 0)
            {
                break;
            }
        }
        lFromDefaultChartInfo = false;
    }

    bool lNewChart = false;
    while (iGroupIDX < gexReport->getGroupsList().size())
    {
        // Get pointer to relevant group
        pTestGroup = gexReport->getGroupsList().at(iGroupIDX);

        lNewChart =false;
        if (iGroupX >= 0)
        {
            if( mChartsInfo->chartsList().count() > 0)
                pChart = mChartsInfo->chartsList()[0];
            else
            {
                pChart   = new CGexSingleChart(0);
                lNewChart = true;
            }
            iLayerID = 0;
        }
        else if (lFromDefaultChartInfo == false &&
                 iGroupIDX < mChartsInfo->chartsList().count())
        {
            pChart = mChartsInfo->chartsList()[iGroupIDX];
            iLayerID = iGroupIDX;
        }
        else
        {
              iLayerID = mChartsInfo->chartsList().count();
              pChart   = new CGexSingleChart(iLayerID);
              lNewChart = true;
        }

        // If new layer, add its style info into Options repository so to
        // save it into script file on exit
        if(lNewChart)
        {
            if (ReportOptions.pLayersStyleList.count() > iLayerID)
            {
                // LayerID already exists in list, so simpy update its content
                // with the current new style info (color, style, ...)
                pLayerStyle = ReportOptions.pLayersStyleList.at(iLayerID);
                //*pLayerStyle =  *pChart;
                *pChart     = *pLayerStyle;
            }
            else
            {
                // Layer style archive doesn't include such LayerID...need to append it!
                pLayerStyle = new CGexSingleChart(*pChart);

                ReportOptions.pLayersStyleList.append(pLayerStyle);
            }
        }

        if (visible != NULL && iLayerID < visibleCount)
        {
            pChart->bVisible = visible[iLayerID];
        }

        if (ReportOptions.GetOption("adv_histogram", "chart_type").toString() == "stack")
        {
            pChart->mIsStacked = true;
        }

        // Update Layer info to hold correct Parameter#
        // Erase name, 'addChart' function will recreate the updated one!
        pChart->strChartName  = "";
        pChart->iTestNumberX  = iTestX;
        pChart->iPinMapX      = iPinmapX;
        pChart->strTestNameX  = strNameX;
        pChart->strTestLabelX = "";
        // If multi layers, show layer name in test label...
        if (gexReport->getGroupsList().count() > 1)
        {
            pChart->strTestLabelX += pTestGroup->strGroupName + " - ";
        }
        strString.sprintf("T%u : ", iTestX);
        pChart->strTestLabelX += strString + strNameX;
        pChart->iGroupX        = iGroupIDX;

        pChart->iTestNumberY = iTestY;
        pChart->iPinMapY     = iPinmapY;
        pChart->strTestNameY = strNameY;
        pChart->iGroupY      = iGroupIDY;
        // If TestY is valid (GroupY >= 0)
        if ((pChart->iTestNumberY != (unsigned) -1) &&
                (iGroupIDY < gexReport->getGroupsList().size()))
        {
            // Get pointer to relevant group
            //FIXME: not used ?
            //pTestGroupY = gexReport->getGroupsList().at(iGroupIDY);
            pChart->strTestLabelY = "";
            // If multi layers, show layer name in test label...
            if (gexReport->getGroupsList().count() > 1)
            {
                pChart->strTestLabelY += pTestGroup->strGroupName + " - ";
            }
            strString.sprintf("T%u : ", iTestY);
            pChart->strTestLabelY += strString + strNameY;
        }
        else
        {
            pChart->strTestNameY = pChart->strTestLabelY = "- none -";
        }

        // No Z parameter for now
        pChart->iTestNumberZ = (unsigned) -1;
        pChart->iPinMapZ     = -1;
        pChart->strTestNameZ = pChart->strTestLabelZ = "- none -";
        pChart->iGroupZ      = 0;

        if (bClear)
        {
            pChart->lfDataOffset = 0.0;
            pChart->lfDataScale  = 1.0;
        }

        // Add layer to list
        addChart(bUpdateGUI, false, pChart, lNewChart);

        // If we must chart only one layer, exit now!
        if (iGroupX >= 0)
        {
            break;
        }

        // We must plot ALL layers, then move to next group!
        iGroupIDX++;
        iGroupIDY++;
    }

    if (visible != NULL)
    {
        delete visible;
    }
}

/******************************************************************************!
 * \fn addChart
 * \brief Add one chart to the list of charts overlays
 ******************************************************************************/
void
GexWizardChart::addChart(bool             bUpdateGUI,
                         bool             bClear,
                         CGexSingleChart* pLayer,
                         bool             newChart)
{
    if (pLayer->iTestNumberX == (unsigned) -1)
    {
        if (bClear)
        {
            clearChart();
        }
        return;  // no valid test to add...simply reset
    }

    // If we have to clear all existing charts overlays...
    // do it, but first inherit settings
    if (bClear)
    {
        // Now we can erase the previous list!
        clearChart();
    }

    // create chart name
    CGexGroupOfFiles* pTestGroup;
    CGexFileInGroup*  pTestFile;
    CTest*            ptParameterCell;
    char szTestName[1024];

    int iGroupID;
    if (pLayer->iGroupX >= 0 && pLayer->iGroupX <
            gexReport->getGroupsList().count())
    {
        iGroupID = pLayer->iGroupX;
    }
    else
    {
        iGroupID = 0;
    }

    // Add one layer or "all" layers, depending of user selection
    bool lConstructHeader = false;
    while (iGroupID < gexReport->getGroupsList().size())
    {
        pTestGroup = gexReport->getGroupsList().at(iGroupID);
        pTestFile  = (pTestGroup->pFilesList.isEmpty()) ?
                    NULL : (pTestGroup->pFilesList.first());

        if (pTestFile->FindTestCell(pLayer->iTestNumberX, pLayer->iPinMapX,
                                    &ptParameterCell, false, false,
                                    pLayer->strTestNameX) != 1)
        {
            return;  // Failed finding parameter entry...
        }
        // Build string name if not yet built
        if (pLayer->strChartName.isEmpty())
        {
            // Test XXXX: <Name>
            // or
            // <Groupname> - Test XXXX: <Name>    if we have multiple groups!
            if (gexReport->getGroupsList().count() > 1)
            {
                pLayer->strChartName = pTestGroup->strGroupName + " - ";
            }
            else
            {
                pLayer->strChartName = "";
            }

            pLayer->strChartName += "Test ";
            gexReport->BuildTestNumberString(ptParameterCell);
            pLayer->strChartName += ptParameterCell->szTestLabel;
            pLayer->strChartName += " : ";
            gexReport->BuildTestNameString(pTestFile, ptParameterCell, szTestName);
            pLayer->strChartName += szTestName;

        }


        // if option format request name
        //if()
        QString lRawName = pLayer->strChartName;
        formatRequestDBName(pLayer->strTestLabelX);
        formatRequestDBName(pLayer->strChartName);

        if(lConstructHeader == false)
        {
            buildTreeLayerHeader(lRawName);
            lConstructHeader = true;
        }


        // Add new chart to the list
        if(newChart)
            mChartsInfo->addChart(pLayer);

        // If we must chart only one layer, exit now!
        if (pLayer->iGroupX >= 0)
        {
            break;
        }

        // We must plot ALL layers, then move to next group!
        iGroupID++;
    }

    // Update chart list...unless multi-layers insterted at once,
    // in which case, GUI only updated once at the end
    if (bUpdateGUI)
    {
        UpdateChartsList();
    }
}

/******************************************************************************!
 * \fn UpdateChartsList
 * \brief Refresh Combo Boxes +
 *        Layer list that include the list of charts plotted
 ******************************************************************************/
void GexWizardChart::UpdateChartsList()
{
    if (mChartsInfo == NULL)
    {
        return;
    }

    if (mChartsInfo->chartsList().isEmpty())
    {
        return;
    }

    // Scan all charts in our stack,
    // to add their name to the combo list + Layer list
    QTreeWidgetItem* pTreeItem = NULL;

    comboBoxChartLayer->clear();

    // Clear Layer list GUI
    treeWidgetLayers->clear();
    pPixmapLayers.clear();

    CGexSingleChart* pList = NULL;

    for (int nChartIndex = 0;
         nChartIndex < mChartsInfo->chartsList().count();
         ++nChartIndex)
    {
        pList = mChartsInfo->chartsList().at(nChartIndex);

        // Add chart name to the list
        // Fill Combo with Parameters names
        comboBoxChartLayer->addItem(pList->strChartName);

        // Add chart name to the layer list
        pTreeItem = new QTreeWidgetItem(treeWidgetLayers);

        // Create pixmap to show Chart color...unless Layer is Hidden
        QPixmap pixmapColor(17, 17);

        if (pList->bVisible)
        {
            // Layer is visible: show its filling color
            pixmapColor.fill(pList->cColor);
        }
        else
        {
            // Layer is Hidden, show Red cross
            // Begin painting task
            QPainter p;
            pixmapColor.fill(QColor(Qt::white));

            p.begin(&pixmapColor);
            p.setPen(QPen(Qt::red, 2));
            p.drawLine(5, 5, 13, 13);
            p.drawLine(13, 5, 5, 13);
            p.end();
        }
        pPixmapLayers.append(pixmapColor);

        pTreeItem->setText(0, pList->strChartName);
        pTreeItem->setIcon(0, pixmapColor);
    }

    // If multiple Parameters overlayed,
    // then add the "All chart layers" (unless it already exists)
    if (mChartsInfo->chartsList().count() > 1)
    {
        comboBoxChartLayer->setDuplicatesEnabled(false);
        comboBoxChartLayer->insertItem(0, "All chart layers");
    }

    // Select 1st parameter in combo-box list
    comboBoxChartLayer->setCurrentIndex(0);
    if (mChartsInfo &&
            mChartsInfo->chartsList().count() > 1)
    {
        mMultipleTest = true;
        customPropGroupBox->hide();
        //resetAllButton->hide();
    }
    else
    {
        mMultipleTest = false;
        customPropGroupBox->show();
        // resetAllButton->show();
    }
    // Update the 'Style' tab GUI content to reflect parameters options
    OnChangeChartLayer("");
}

/******************************************************************************!
 * \fn OnChangeStyle
 * \brief Chart style change (Markers, etc.)
 ******************************************************************************/
void GexWizardChart::OnChangeStyle()
{
    checkBoxspots->setEnabled(true);

    if(comboBoxChartType->currentIndex() == GexAbstractChart::chartTypeHistogram)
    {
        if(checkBoxfittingCurve->isChecked() == false && checkBoxBellCurve->isChecked() == false)
        {
            checkBoxspots->setChecked(false);
            checkBoxspots->setEnabled(false);
        }
    }

    // If Examinator does the admin: erase layers (if any)
    if (m_bExternalAdmin == false)
    {
        // Histogram type: Bar + fitting curve
        if (checkBoxBars->isChecked() && checkBoxfittingCurve->isChecked())
        {
            tmpReportOptions->SetOption("adv_histogram", "chart_type", "bars_curve");
        }
        else if (checkBoxStack->isChecked())
        {
            tmpReportOptions->SetOption("adv_histogram", "chart_type", "stack");
        }
        else if (checkBox3DBars->isChecked())  // 3D-Bars only
        {
            tmpReportOptions->SetOption("adv_histogram", "chart_type", "3d_bars");
        }
        else if (checkBoxBars->isChecked())  // Bars only
        {
            tmpReportOptions->SetOption("adv_histogram", "chart_type", "bars");
        }
        else if (checkBoxfittingCurve->isChecked())  // Fitting curve only
        {
            tmpReportOptions->SetOption("adv_histogram", "chart_type", "curve");
        }
        else  // If nothing selected, force bars!
        {
            tmpReportOptions->SetOption("adv_histogram", "chart_type", "bars");
        }

        // Trend type: Lines + spots
        if (checkBoxLines->isChecked() && checkBoxspots->isChecked())
        {
            tmpReportOptions->SetOption("adv_trend", "chart_type", "lines_spots");
        }
        else if (checkBoxLines->isChecked())  // Lines only
        {
            tmpReportOptions->SetOption("adv_trend", "chart_type", "lines");
        }
        else if (checkBoxspots->isChecked())  // Spots only
        {
            tmpReportOptions->SetOption("adv_trend", "chart_type", "spots");
        }
        else  // If nothing selected, force Lines!
        {
            tmpReportOptions->SetOption("adv_trend", "chart_type", "lines");
        }

        // Scatter type : always spots
        tmpReportOptions->iScatterChartType = GEX_CHARTTYPE_SPOTS;

        // Mean check-box
        bool bStatus = checkBoxMean->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_histogram"),QString("marker"), QString("mean"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("mean"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_correlation"), QString("marker"), QString("mean"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_probabilityplot"),QString("marker"), QString("mean"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_boxplot_ex"), QString("marker"), QString("mean"), bStatus);

        // Min check-box
        bStatus = checkBoxMinimum->isChecked();
        tmpReportOptions->bHistoMarkerMin     = bStatus;
        tmpReportOptions->bTrendMarkerMin     = bStatus;
        tmpReportOptions->bScatterMarkerMin   = bStatus;
        tmpReportOptions->bProbPlotMarkerMin  = bStatus;
        tmpReportOptions->bBoxPlotExMarkerMin = bStatus;

        // Max check-box
        bStatus = checkBoxMaximum->isChecked();
        tmpReportOptions->bHistoMarkerMax     = bStatus;
        tmpReportOptions->bTrendMarkerMax     = bStatus;
        tmpReportOptions->bScatterMarkerMax   = bStatus;
        tmpReportOptions->bProbPlotMarkerMax  = bStatus;
        tmpReportOptions->bBoxPlotExMarkerMax = bStatus;

        // Limits check-box
        bStatus = checkBoxLimits->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_histogram"),QString("marker"), QString("limits"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("limits"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_correlation"),QString("marker"), QString("limits"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_probabilityplot"),QString("marker"), QString("limits"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_boxplot_ex"),QString("marker"), QString("limits"), bStatus);

        // Spec Limits check-box
        bStatus = checkBoxSpecLimits->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_histogram"), QString("marker"), QString("speclimits"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("speclimits"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_correlation"),QString("marker"), QString("speclimits"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_probabilityplot"),QString("marker"), QString("speclimits"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_boxplot_ex"),QString("marker"), QString("speclimits"), bStatus);

        // Rolling limits check-box
        bStatus = checkBoxSubsetLimits->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("rolling_limits"), bStatus);

        // Lot ID, Sub-LotID
        bStatus = checkBoxLotID->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("lot"), bStatus);
        bStatus = checkBoxSubLotID->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("sublot"), bStatus);

        // Group name
        bStatus = checkBoxGroupName->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("group_name"), bStatus);

        // 1-sigma check-box
        bStatus = checkBox1Sigma->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_histogram"),QString("marker"), QString("2sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("2sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_correlation"),QString("marker"), QString("2sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_probabilityplot"),QString("marker"), QString("2sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_boxplot_ex"),QString("marker"), QString("2sigma"), bStatus);

        // 3-sigma check-box
        bStatus = checkBox15Sigma->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_histogram"),QString("marker"), QString("3sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("3sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_correlation"),QString("marker"), QString("3sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_probabilityplot"),QString("marker"), QString("3sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_boxplot_ex"),QString("marker"), QString("3sigma"), bStatus);

        // 6-sigma check-box
        bStatus = checkBox3Sigma->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_histogram"),QString("marker"), QString("6sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("6sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_correlation"),QString("marker"), QString("6sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_probabilityplot"),QString("marker"), QString("6sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_boxplot_ex"), QString("marker"), QString("6sigma"), bStatus);

        // 12-sigma check-box
        bStatus = checkBox6Sigma->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_histogram"),QString("marker"),QString("12sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("12sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_correlation"),QString("marker"),QString("12sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_probabilityplot"),QString("marker"), QString("12sigma"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_boxplot_ex"), QString("marker"),QString("12sigma"), bStatus);

        // Median check-box
        bStatus = checkBoxMedian->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_histogram"),QString("marker"), QString("median"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("median"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_correlation"),QString("marker"), QString("median"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_probabilityplot"),QString("marker"), QString("median"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_boxplot_ex"),QString("marker"), QString("median"), bStatus);

        tmpReportOptions->bPlotLegend = checkBoxLayerName->isChecked();  // Write layer's name on chart

        // qqline
        tmpReportOptions->mPlotQQLine = checkBoxQQLine->isChecked();

        //tmpReportOptions->mTextRotation = (checkBoxTextRotation->isChecked())?45:0;

        bStatus = checkBoxQuartileQ1->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_histogram"),QString("marker"), QString("quartile_q1"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"), QString("marker"), QString("quartile_q1"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_correlation"), QString("marker"), QString("quartile_q1"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_probabilityplot"), QString("marker"), QString("quartile_q1"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_boxplot_ex"),QString("marker"), QString("quartile_q1"), bStatus);

        bStatus = checkBoxQuartileQ3->isChecked();
        tmpReportOptions->SetSpecificFlagOption(QString("adv_histogram"),QString("marker"), QString("quartile_q3"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"),QString("marker"), QString("quartile_q3"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_correlation"),QString("marker"), QString("quartile_q3"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_probabilityplot"),QString("marker"), QString("quartile_q3"), bStatus);
        tmpReportOptions->SetSpecificFlagOption(QString("adv_boxplot_ex"),QString("marker"), QString("quartile_q3"), bStatus);

       /* bStatus = checkBoxTextRotation->isChecked();
        tmpReportOptions->SetOption("adv_histogram","marker_rotation", (bStatus) ? "45" : "0");
        tmpReportOptions->SetOption("adv_trend","marker_rotation", (bStatus) ? "45" : "0");
        tmpReportOptions->SetOption("adv_correlation","marker_rotation", (bStatus) ? "45" : "0");
        tmpReportOptions->SetOption("adv_probabilityplot","marker_rotation", (bStatus) ? "45" : "0");
        tmpReportOptions->SetOption("adv_boxplot_ex","marker_rotation", (bStatus) ? "45" : "0");*/
    }

    // Have relevant Parameter structure reflect this change + redraw charts
    OnChangeLineStyle("");

    // Refresh HTML description + GUI
    OnChangeChartLayer("");
}

/******************************************************************************!
 * \fn OnCheckBoxStack
 ******************************************************************************/
void GexWizardChart::OnCheckBoxStack()
{
    if (checkBoxStack->isChecked())
    {
        tmpReportOptions->SetOption("adv_histogram", "y_axis", "hits");
        comboBoxScaleType->setCurrentIndex(1);
    }
    else
    {
        tmpReportOptions->SetOption("adv_histogram", "y_axis", "percentage");
        comboBoxScaleType->setCurrentIndex(0);
    }
    this->OnChangeStyle();

}

/******************************************************************************!
 * \fn OnChangeSubsetLimit
 * \brief Change the drawing type of the subset limits
 ******************************************************************************/
void GexWizardChart::OnChangeSubsetLimit()
{
    tmpReportOptions->SetSpecificFlagOption(QString("adv_trend"), QString("marker"),
                                            QString("rolling_limits"), checkBoxSubsetLimits->isChecked());

    // Have relevant Parameter structure reflect this change + redraw charts
    OnChangeLineStyle("");

    // Refresh HTML description + GUI
    OnChangeChartLayer("");
}

/******************************************************************************!
 * \fn onChangedLineColor
 * \brief Change the drawing color for a given chart
 ******************************************************************************/
void GexWizardChart::onChangedLineColor(const QColor&  /*clrLine*/)
{
    // Have relevant Parameter structure reflect this change + redraw charts
    OnChangeLineStyle("");

    // Update chart list
    UpdateChartsList();

    // Refresh HTML description + GUI
    OnChangeChartLayer("");
}

/******************************************************************************!
 * \fn onChangedChartBackgroundColor
 * \brief Change the drawing Chart Background color
 ******************************************************************************/
void GexWizardChart::onChangedChartBackgroundColor(const QColor& clrBackground)
{
    // Save color into options so it will be saved to the disk in the script file
    // Has to be done on:
    //    � Local report options object to be take into account in the interactive chart
    //    � Global report options object to be saved in the user profile

    ReportOptions.cBkgColor = clrBackground;
    tmpReportOptions->cBkgColor = clrBackground;

    CopyDefaultStyle();
    // Force Redraw : Chart or Table, (depending of current tab selected)
    OnTabChange();
}

/******************************************************************************!
 * \fn OnChangeChartLayer
 * \brief Show the Parameter Info in the tabs:
 *        o Show statistics info in the HTML window
 *        o Show Lines/Markers tabs options
 ******************************************************************************/
void GexWizardChart::OnChangeChartLayer(const QString&  /*strString*/)
{
    // Check if data loaded
    if (gexReport->getGroupsList().count() <= 0)
    {
        return;
    }

    // Get Parameter entry selected
    int iEntry = comboBoxChartLayer->currentIndex();

    // If multiple layers, 1st parameter in list is
    // "all layers" which allows to change all settings of all layers at once
    bool bEnable;
    if (mChartsInfo->chartsList().count() > 1)
    {
        if (iEntry)
        {
            iEntry--;  // Real Parameter index offset in our list!

            // Enable GUI buttons for single parameter changes
            // (eg: color, line style, etc.
            bEnable = true;
        }
        else
        {
            // ALL Parameters: do not allow to modify settings like color,
            // line type, etc.
            bEnable = false;
        }
    }
    else
    {
        bEnable = true;  // only one parameter in list!

    }
    // Enable or Disable GUI buttons for single parameter changes
    // (eg: color, line style, etc.
    if (bEnable)
    {
        textLabelLineColor->show();
        pushButtonLineColor->show();
        // textLabelLineStyle->show();
        // comboBoxLineStyle->show();
        textLabelSpotStyle->show();
        comboBoxLineSpotStyle->show();
    }
    else
    {
        textLabelLineColor->hide();
        pushButtonLineColor->hide();
        // textLabelLineStyle->hide();
        // comboBoxLineStyle->hide();
        textLabelSpotStyle->hide();
        comboBoxLineSpotStyle->hide();
    }

    // If multiple groups (comparing datasets),
    // DO NOT allow to edit the line color
    if (gexReport->getReportOptions()->iGroups > 1)
    {
        pushButtonLineColor->setEnabled(false);
    }

    CGexSingleChart* pChart = NULL;

    if (iEntry >= 0 && iEntry < mChartsInfo->chartsList().count())
    {
        pChart = mChartsInfo->chartsList().at(iEntry);
    }

    if (pChart == NULL)
    {
        return;  // Entry not found

    }
    // Find Parameter cell to list in HTML page
    CGexGroupOfFiles* pTestGroup      = NULL, * pTestGroupY = NULL;
    CGexFileInGroup*  pTestFile       = NULL, * pTestFileY = NULL;
    CTest*            ptParameterCell = NULL, * ptParameterCellY = NULL;

    if ((pChart->iGroupX < 0) ||
            (pChart->iGroupX >= gexReport->getGroupsList().size()))
    {
        return;
    }
    pTestGroup = gexReport->getGroupsList().at(pChart->iGroupX);
    pTestFile  =
            (pTestGroup->pFilesList.isEmpty()) ? NULL :
                                                 (pTestGroup->pFilesList.first());
    if (pTestFile->FindTestCell(pChart->iTestNumberX, pChart->iPinMapX,
                                &ptParameterCell, false, false,
                                pChart->strTestNameX) != 1)
    {
        return;  // Failed finding parameter entry

    }
    if ((pChart->iGroupY < 0) ||
            (pChart->iGroupY >= gexReport->getGroupsList().size()))
    {
        pTestGroupY = NULL;
    }
    else
    {
        pTestGroupY = gexReport->getGroupsList().at(pChart->iGroupY);
    }
    if (pTestGroupY != NULL)
    {
        pTestFileY =
                (pTestGroupY->pFilesList.isEmpty()) ? NULL :
                                                      (pTestGroupY->pFilesList.first());
        if (pTestFileY->FindTestCell(pChart->iTestNumberY, pChart->iPinMapY,
                                     &ptParameterCellY, false, false,
                                     pChart->strTestNameY) != 1)
        {
            // No Y parameter
            pTestGroupY = NULL;
            pTestFileY  = NULL;
        }
    }
    else
    {
        ptParameterCellY = NULL;
    }

    // Create HTML page / info page header
    gexReport->CheckForNewHtmlPage(mChartsInfo/*, strPropertiesPath*/);

    // Define type of statistics to display
    QString strChartType = "adv_trend";

    switch (m_eChartType)
    {
    case GexAbstractChart::chartTypeTrend:
        // Draw Trend in Frame window
        strChartType = "adv_trend";
        break;
    case GexAbstractChart::chartTypeHistogram:
        // Draw Histogram in Frame window
        strChartType = "adv_histo";
        break;
    case GexAbstractChart::chartTypeScatter:
        // Draw Scatter plot in Frame window
    case GexAbstractChart::chartTypeBoxWhisker:
        strChartType = "adv_scatter";
        break;
    case GexAbstractChart::chartTypeBoxPlot:
        // Data is: BoxPlot
        strChartType = "adv_boxplot";
        break;
    case GexAbstractChart::chartTypeProbabilityPlot:
        // Data is: Probability plot
        strChartType = "adv_boxplot";
        break;
    default:
        GEX_ASSERT(false);
        break;
    }

    // Writes HTML table with global test info + chart
    // (name, limits, Cp, cp shift, ...)
    if (bEnable)
    {
        gexReport->
                WriteHistoHtmlTestInfo(mChartsInfo,
                                       strChartType.toLatin1().constData(),
                                       pTestFile,
                                       ptParameterCell,
                                       pTestFileY,
                                       ptParameterCellY,
                                       true,
                                       QStringList(),
                                       GEX_CHARTSIZE_LARGE);
    }
    else
    {
        // Combo-box shows "All parameters"...
        // so create HTML page with color assigned to each parameter
        gexReport->WriteHtmlPageLayerLegends(mChartsInfo);
    }

    gexReport->CloseReportFile();

    // Get HTML page reloaded to reflect changes
    textBrowserInfo->setSource(QUrl::fromLocalFile(strPropertiesPath));
    textBrowserInfo->reload();

    // If multiple layers, we can't show the exact current combined style
    if (mChartsInfo->chartsList().count() > 1)
    {
        return;
    }

    UpdateGUIGlobal(mChartsInfo);
    UpdateGUIByChart(pChart);

    // Write layer's name on chart
    // checkBoxLayerName->setChecked(tmpReportOptions->bPlotLegend);
    // qqline
    //  checkBoxQQLine->setChecked(tmpReportOptions->mPlotQQLine);
}

//--arno
void GexWizardChart::UpdateGUIGlobal(const CGexChartOverlays *overlay)
{
    pushButtonChartBackgroundColor->setActiveColor  (overlay->mBackgroundColor);
    spinBoxHistBars->setValue                       (overlay->mTotalBars);
    comboBoxScaleType->setCurrentIndex              (overlay->mYScale);
    customPropGroupBox->setChecked                  (overlay->mCustom);
    customSpinBoxHistBars->setValue                 (overlay->mCustomTotalBars);
    checkBoxLayerName->setChecked                   (overlay->mLayerName);
    //checkBoxTextRotation->setChecked                (overlay->mTextRotation);
    checkBoxQQLine->setChecked                      (overlay->mQQLine);


    //    pushButtonLineColor->setActiveColor     (overlay->mColor);          // Line color
    //    checkBoxBars->setChecked                (overlay->mBoxBars);        // Bars check-box
    //    checkBox3DBars->setChecked              (overlay->mBox3DBars);      // 3D-Bars check-box
    //    checkBoxStack->setChecked               (overlay->mIsStacked);      // Fitting curve check-box
    //    checkBoxfittingCurve->setChecked        (overlay->mFittingCurve);   // guaussian bell-curve shape
    //    checkBoxBellCurve->setChecked           (overlay->mBellCurve);
    //    checkBoxLines->setChecked               (overlay->mLines);          // Lines check-box
    //    checkBoxspots->setChecked               (overlay->mSpots);          // Spots check-box
    //    checkBoxSpecLimits->setChecked          (overlay->mSpecLimit);      // Set line style: slid, dashed, etc.
    //    comboBoxLineStyle->setCurrentIndex      (overlay->mLineStyle);      // Set spot style: circle, rectangle, diamond, etc.
    //    comboBoxLineSpotStyle->setCurrentIndex  (overlay->mSpotStyle);

    //    switch (overlay->mWhiskerMode)
    //    {
    //    case GEX_WHISKER_RANGE:
    //        comboBoxWhiskerType->setCurrentIndex(0);  // Range
    //        break;
    //    case GEX_WHISKER_Q1Q3:
    //        comboBoxWhiskerType->setCurrentIndex(1);  // Q1 - 1.5*IQR, Q3 + 1.5*IQR
    //        break;
    //    case GEX_WHISKER_IQR:
    //        comboBoxWhiskerType->setCurrentIndex(2);  // Q2 +/- 1.5*IQR
    //        break;
    //    }

    //    // Update the "Markers" Tab page options
    //    // Mean check-box
    //    checkBoxMean->setChecked(((overlay->mMeanLineWidth > 0) ? true : false));
    //    // Min check-box
    //    checkBoxMinimum->setChecked(((overlay->mMinLineWidth > 0) ? true : false));
    //    // Max check-box
    //    checkBoxMaximum->setChecked(((overlay->mMaxLineWidth > 0) ? true : false));
    //    // Median check-box
    //    checkBoxMedian->setChecked(((overlay->mMedianLineWidth > 0) ? true : false));
    //    // Test limits check-box
    //    checkBoxLimits->setChecked(((overlay->mLimitsLineWidth > 0) ? true : false));
    //    // Test speclimits check-box
    //    checkBoxSpecLimits->setChecked(((overlay->mSpecLimitsLineWidth > 0) ? true : false));
    //    // +/-1sigma check-box
    //    checkBox1Sigma->setChecked(((overlay->mSigma2LineWidth > 0) ? true : false));
    //    // +/-1.5sigma check-box
    //    checkBox15Sigma->setChecked(((overlay->mSigma3LineWidth > 0) ? true : false));
    //    // +/-3sigma check-box
    //    checkBox3Sigma->setChecked(((overlay->mSigma6LineWidth > 0) ? true : false));
    //    // +/-6sigma check-box
    //    checkBox6Sigma->setChecked(((overlay->mSigma12LineWidth > 0) ? true : false));
    //    checkBoxQuartileQ3->setChecked(((overlay->mQuartileQ3LineWidth > 0) ? true : false));
    //    checkBoxQuartileQ1->setChecked(((overlay->mQuartileQ1LineWidth > 0) ? true : false));// Test Rolling limits check-box
    //    checkBoxSubsetLimits->setChecked(((overlay->mRollingLimitsLineWidth > 0) ? true : false));
    //    spinBoxLineWidth->setValue          (overlay->mLineWidth);  // Set line width


}

void GexWizardChart::UpdateGUIByChart(const CGexSingleChart* chart)
{
    if(chart == 0)
        return;

    pushButtonLineColor->setActiveColor (chart->cColor);  // Line color
    checkBoxBars->setChecked            (chart->bBoxBars);  // Bars check-box
    checkBox3DBars->setChecked          (chart->bBox3DBars);  // 3D-Bars check-box
    checkBoxStack->setChecked           (chart->mIsStacked);
    // Fitting curve check-box
    checkBoxfittingCurve->setChecked    (chart->bFittingCurve);
    // guaussian bell-curve shape
    checkBoxBellCurve->setChecked       (chart->bBellCurve);
    checkBoxLines->setChecked           (chart->bLines);  // Lines check-box
    checkBoxspots->setChecked           (chart->bSpots);  // Spots check-box
    //checkBoxSpecLimits->setChecked      (chart->mSpecLimit);
    // Set line style: slid, dashed, etc.
    comboBoxLineStyle->setCurrentIndex  (chart->iLineStyle);
    // Set spot style: circle, rectangle, diamond, etc.
    comboBoxLineSpotStyle->setCurrentIndex(chart->iSpotStyle);
    switch (chart->iWhiskerMode)
    {
    case GEX_WHISKER_RANGE:
        comboBoxWhiskerType->setCurrentIndex(0);  // Range
        break;
    case GEX_WHISKER_Q1Q3:
        comboBoxWhiskerType->setCurrentIndex(1);  // Q1 - 1.5*IQR, Q3 + 1.5*IQR
        break;
    case GEX_WHISKER_IQR:
        comboBoxWhiskerType->setCurrentIndex(2);  // Q2 +/- 1.5*IQR
        break;
    }

    // Update the "Markers" Tab page options
    // Mean check-box
    checkBoxMean->setChecked(((chart->meanLineWidth() > 0) ? true : false));
    // Min check-box
    checkBoxMinimum->setChecked(((chart->minLineWidth() > 0) ? true : false));
    // Max check-box
    checkBoxMaximum->setChecked(((chart->maxLineWidth() > 0) ? true : false));
    // Median check-box
    checkBoxMedian->setChecked(((chart->medianLineWidth() > 0) ? true : false));
    // Test limits check-box
    checkBoxLimits->setChecked(((chart->limitsLineWidth() > 0) ? true : false));
    // Test speclimits check-box
    checkBoxSpecLimits->setChecked(((chart->specLimitsLineWidth() > 0) ? true : false));
    // +/-1sigma check-box
    checkBox1Sigma->setChecked(((chart->sigma2LineWidth() > 0) ? true : false));
    // +/-1.5sigma check-box
    checkBox15Sigma->setChecked(((chart->sigma3LineWidth() > 0) ? true : false));
    // +/-3sigma check-box
    checkBox3Sigma->setChecked(((chart->sigma6LineWidth() > 0) ? true : false));
    // +/-6sigma check-box
    checkBox6Sigma->setChecked(((chart->sigma12LineWidth() > 0) ? true : false));
    checkBoxQuartileQ3->
            setChecked(((chart->quartileQ3LineWidth() > 0) ? true : false));
    checkBoxQuartileQ1->
            setChecked(((chart->quartileQ1LineWidth() > 0) ? true : false));
    // Test Rolling limits check-box
    checkBoxSubsetLimits->setChecked(((chart->GetRollingLimitsLineWidth() > 0) ? true : false));

    //checkBoxTextRotation->setChecked(((chart->getTextRotation() > 0) ? true : false));
    //checkBoxLayerName->setChecked(chart->mLayerName );
    //checkBoxQQLine->setChecked(chart->mQQLine);
    spinBoxLineWidth->setValue          (chart->iLineWidth);  // Set line width
}

/******************************************************************************!
 * \fn OnChangeLineStyle
 * \brief Read changes in the "Lines" tab page + update charts accordingly
 ******************************************************************************/
void GexWizardChart::OnChangeLineStyle(const QString&  /*strString*/)
{
    // Check if we have any Parameters in our list yet...
    if (mChartsInfo->chartsList().count() <= 0)
    {
        return;
    }

    // If Examinator does the admin: get default style from the 'Options'
    if (m_bExternalAdmin == false)
    {
        CopyDefaultStyle();
    }

    // Switch back to 'Select' mode...avoids unexpected 'drag' or 'zoom' action!
    OnSelect();

    // Force Redraw : Chart or Table, (depending of current tab selected)
    OnTabChange();
}

/******************************************************************************!
 * \fn CopyDefaultStyle
 * \brief Get default style from the 'Options', copy into layers
 ******************************************************************************/
void GexWizardChart::CopyDefaultStyle()
{
    int iEntry;
    // Indexes if multiple parameters settings to be changed at once!
    // Get pointer to relevant Parameter structure to update
    int iFrom = iEntry = comboBoxChartLayer->currentIndex();
    int iTo   = iFrom + 1;

    // If multiple layers, 1st parameter in list is
    // "all layers" which allows to change all settings of all layers at once
    bool bSingleParameterEdit = true;
    if (mChartsInfo->chartsList().count() > 1)
    {
        if (iEntry)
        {
            iEntry--;  // Real Parameter index offset in our list!
            bSingleParameterEdit = true;  // Editing ONE parameter settings structure
            iFrom = iEntry;
            iTo   = iFrom + 1;
        }
        else
        {
            // Changing ALL parameters at once!
            bSingleParameterEdit = false;
            iFrom = 0;
            iTo   = mChartsInfo->chartsList().count();
        }
    }

    mChartsInfo->mTotalBars       = spinBoxHistBars->value();
    mChartsInfo->mBackgroundColor = pushButtonChartBackgroundColor->activeColor();
    mChartsInfo->mYScale          = comboBoxScaleType->currentIndex();
    mChartsInfo->mCustom          = customPropGroupBox->isChecked();
    mChartsInfo->mCustomTotalBars = customSpinBoxHistBars->value();

    mChartsInfo->mLayerName       = checkBoxLayerName->isChecked();
    //mChartsInfo->mTextRotation    = checkBoxTextRotation->isChecked();
    mChartsInfo->mQQLine          = checkBoxQQLine->isChecked();

    CGexSingleChart* pChart;
    CGexSingleChart* pLayerStyle;

    for (iEntry = iFrom; iEntry < iTo; iEntry++)
    {
        // Get structure to Parameter#X
        pChart = mChartsInfo->chartsList().at(iEntry);

        // Read the "Lines" Tab page options
        pChart->bBoxBars   = checkBoxBars->isChecked();    // Bars check-box
        pChart->bBox3DBars = checkBox3DBars->isChecked();    // 3D-Bars check-box
        pChart->mIsStacked = checkBoxStack->isChecked();
        // Fitting curve check-box
        pChart->bFittingCurve = checkBoxfittingCurve->isChecked();
        // Guaussian bell-curve shape
        pChart->bBellCurve = checkBoxBellCurve->isChecked();
        pChart->bLines     = checkBoxLines->isChecked();    // Lines check-box
        pChart->bSpots     = checkBoxspots->isChecked();    // Spots check-box
        pChart->iLineWidth = spinBoxLineWidth->value();    // Get line width
        // Get line style: slid, dashed, etc.
        pChart->iLineStyle   = comboBoxLineStyle->currentIndex();
        pChart->iWhiskerMode = comboBoxWhiskerType->currentIndex();

        if (bSingleParameterEdit)
        {
            // These settings are only allowed to be changed
            //  one Chart layer at a time!
            pChart->cColor = pushButtonLineColor->activeColor();     // Line color
            // Get spot style: circle, rectangle, diamond, etc.
            pChart->iSpotStyle = comboBoxLineSpotStyle->currentIndex();
        }

        // Update the "Markers" Tab page options
        // Mean check-box
        pChart->setMeanLineWidth((checkBoxMean->isChecked()) ? 1 : 0);

        // Min check-box
        pChart->setMinLineWidth((checkBoxMinimum->isChecked()) ? 1 : 0);

        // Max check-box
        pChart->setMaxLineWidth((checkBoxMaximum->isChecked()) ? 1 : 0);

        // Median check-box
        pChart->setMedianLineWidth((checkBoxMedian->isChecked()) ? 1 : 0);

        // Test limits check-box
        pChart->setLimitsLineWidth((checkBoxLimits->isChecked()) ? 1 : 0);
        pChart->setSpecLimitsLineWidth((checkBoxSpecLimits->isChecked()) ? 1 : 0);
        //pChart->mSpecLimit = ((checkBoxSpecLimits->isChecked()) ? 1 :0);
        //pChart->mLayerName = ((checkBoxLayerName->isChecked()) ? 1 :0);
        //pChart->mQQLine  = ((checkBoxQQLine->isChecked()) ? 1 :0);

        // +/-1sigma check-box
        pChart->set2SigmaLineWidth((checkBox1Sigma->isChecked()) ? 1 : 0);

        // +/-1.5sigma check-box
        pChart->set3SigmaLineWidth((checkBox15Sigma->isChecked()) ? 1 : 0);

        // +/-3sigma check-box
        pChart->set6SigmaLineWidth((checkBox3Sigma->isChecked()) ? 1 : 0);

        // +/-6sigma check-box
        pChart->set12SigmaLineWidth((checkBox6Sigma->isChecked()) ? 1 : 0);

        // Test Rolmling limits check-box
        pChart->setRollingLimitsLineWidth((checkBoxSubsetLimits->isChecked()) ? 1 : 0);

        pChart->setQuartileQ1((checkBoxQuartileQ1->isChecked()) ? 1 : 0);
        pChart->setQuartileQ3((checkBoxQuartileQ3->isChecked()) ? 1 : 0);
        //pChart->setTextRotation((checkBoxTextRotation->isChecked()) ? 45 : 0);

        pLayerStyle = 0;
        if (iEntry >= 0)
        {
            // Save new settings into the 'ReportOptions' list
            // so to be saved in script file on exit (unless list not ye built!)
            /*  if(iEntry < ReportOptions.pLayersStyleList.count())
            {
                pLayerStyle = ReportOptions.pLayersStyleList.at(iEntry);
                if (pLayerStyle)
                {
                    *pLayerStyle = *pChart;
                }
            }*/

            // save in the settings
            if(iEntry < mChartsInfo->chartsList().count()/*mSettings.count()*/)
            {
                pLayerStyle = mChartsInfo->chartsList().at(iEntry); // mSettings.at(iEntry);
                if (pLayerStyle)
                {
                    *pLayerStyle = *pChart;
                }
            }
        }
    }
}

/******************************************************************************!
 * \fn OnAddChart
 * \brief Overlay charts assistant: ADD chart to plot
 ******************************************************************************/
void GexWizardChart::OnAddChart()
{
    EditLayerDialog cEditLayer;

    // Preset Layer styles  according to layer index
    cEditLayer.resetVariables(treeWidgetLayers->topLevelItemCount());

    // Display new layer Diaog box
    if (cEditLayer.exec() != 1)
    {
        return;  // User 'Abort'

    }
    // Get layer info to add to our list
    CGexSingleChart* pLayer = new CGexSingleChart();
    cEditLayer.getLayerInfo(pLayer);

    // Add layer into internal list
    addChart(true, false, pLayer, true);

    // Reset Zoom/Drag flags + replot
    goHome();
}

/******************************************************************************!
 * \fn CreateCustomParameter
 * \brief Constructor
 ******************************************************************************/
CreateCustomParameter::CreateCustomParameter(QWidget*   parent,
                                             bool       modal,
                                             Qt::WFlags f)
    : QDialog(parent, f)
{
    setupUi(this);
    setModal(modal);

    if (! connect(buttonPickTest, SIGNAL(clicked()), SLOT(OnPickTest())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonPickTest fail");
    }
    if (! connect(buttonLess, SIGNAL(clicked()), SLOT(OnLess())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonLess fail");
    }
    if (! connect(buttonEqual, SIGNAL(clicked()), SLOT(OnEqual())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonEqual fail");
    }
    if (! connect(buttonGreater, SIGNAL(clicked()), SLOT(OnGreater())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonGreater fail");
    }
    if (! connect(buttonLessEqual, SIGNAL(clicked()), SLOT(OnLessEqual())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonLessEqual fail");
    }
    if (! connect(buttonNotEqual, SIGNAL(clicked()), SLOT(OnNotEqual())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonNotEqual fail");
    }
    if (! connect(buttonGreaterEqual, SIGNAL(clicked()), SLOT(OnGreaterEqual())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonGreaterEqual fail");
    }
    if (! connect(pushButtonOk, SIGNAL(clicked()), SLOT(OnOk())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect pushButtonOk fail");
    }
    if (! connect(buttonPickOriginalTest, SIGNAL(clicked()),
                  SLOT(OnPickOriginalTest())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonPickOriginalTest fail");
    }
    if (! connect(buttonClear, SIGNAL(clicked()), SLOT(OnClearExpression())))
    {
        GSLOG(SYSLOG_SEV_WARNING, "connect buttonClear fail");
    }

    // Clear original test# & name at startup
    labelOriginalParamNumber->setText("");
    labelOriginalParamName->setText("");

    // Get the highest available test#
    // Default Test# in case we can't find the current highest test#
    // in the next lines
    lineEditNewTestNumber->setText("80000");
    lineEditNewTestName->setText("Custom Parameter");
    CGexGroupOfFiles* pGroup =
            gexReport->getGroupsList().size() > 0 ?
                gexReport->getGroupsList().first() : NULL;
    if (pGroup == NULL)
    {
        return;
    }

    // Get highest Test# currently in list,
    // and build new test# by incrementing it by one
    CTest* ptTestCell = pGroup->cMergedData.ptMergedTestList;
    while (ptTestCell->GetNextTest() != NULL)
    {
        ptTestCell = ptTestCell->GetNextTest();
    }
    lineEditNewTestNumber->setText(QString::number(ptTestCell->lTestNumber + 1));

    // Focus on new Test name to create
    lineEditNewTestName->setFocus();
}

/******************************************************************************!
 * \fn setExpression
 * \brief Load default expression into Expression-Editor
 ******************************************************************************/
void
CreateCustomParameter::
setExpression(QString strExpression, bool bClear  /*=true*/)
{
    if (bClear)
    {
        textEditFilterExpression->setText(strExpression);

        // Set cursor editor to the end of the text!
        textEditFilterExpression->moveCursor(QTextCursor::End);
    }
    else
    {
        textEditFilterExpression->insertPlainText(strExpression);
        textEditFilterExpression->setFocus();
    }
}

/******************************************************************************!
 * \fn getExpression
 * \brief Return expression created under the Expression-Editor
 ******************************************************************************/
QString CreateCustomParameter::getExpression()
{
    return m_strExpression;
}

/******************************************************************************!
 * \fn OnLess
 * \brief Insert comparision string into Expression-Editor string
 ******************************************************************************/
void CreateCustomParameter::OnLess()
{
    setExpression(" < ", false);
}

/******************************************************************************!
 * \fn OnEqual
 * \brief Insert comparision string into Expression-Editor string
 ******************************************************************************/
void CreateCustomParameter::OnEqual()
{
    setExpression(" = ", false);
}

/******************************************************************************!
 * \fn OnGreater
 * \brief Insert comparision string into Expression-Editor string
 ******************************************************************************/
void CreateCustomParameter::OnGreater()
{
    setExpression(" > ", false);
}

/******************************************************************************!
 * \fn OnLessEqual
 * \brief Insert comparision string into Expression-Editor string
 ******************************************************************************/
void CreateCustomParameter::OnLessEqual()
{
    setExpression(" <= ", false);
}

/******************************************************************************!
 * \fn OnNotEqual
 * \brief Insert comparision string into Expression-Editor string
 ******************************************************************************/
void CreateCustomParameter::OnNotEqual()
{
    setExpression(" != ", false);
}

/******************************************************************************!
 * \fn OnGreaterEqual
 * \brief Insert comparision string into Expression-Editor string
 ******************************************************************************/
void CreateCustomParameter::OnGreaterEqual()
{
    setExpression(" >= ", false);
}

/******************************************************************************!
 * \fn OnPickOriginalTest
 * \brief Pick the original test from which to create the custom one
 ******************************************************************************/
void CreateCustomParameter::OnPickOriginalTest()
{
    // Show TestList: Single selection mode
    PickTestDialog dPickTest;

    // Disable selection of multiple layers :
    // user can only pick a test from a given layer
    dPickTest.setMultipleGroups(true, false);

    if (dPickTest.fillParameterList())
    {
        // Prompt dialog box, let user pick tests from the list
        if (dPickTest.exec() == QDialog::Accepted)
        {
            // Get test# selected. string format: <Test#>.<Pinmap#>
            QString strTest = dPickTest.testItemizedList();

            if (strTest.isEmpty() == false)
            {
                if (sscanf(strTest.toLatin1().constData(), "%lu%*c%ld",
                           &m_lOriginalTestNumber, &m_lOriginalTestPinMap) < 2)
                {
                    m_lOriginalTestPinMap = GEX_PTEST;  // No Pinmap index specified

                }
                strTest = "Test: " + strTest;
                labelOriginalParamNumber->setText(strTest);
                QStringList strTests = dPickTest.testItemizedListNames();
                labelOriginalParamName->setText(strTests[0]);

                // Get GroupID selected (in case multi-groups available)
                m_GroupID = dPickTest.getGroupID();
            }
        }
    }
}

/******************************************************************************!
 * \fn OnPickTest
 * \brief Pick test to insert into Expression-Editor string
 ******************************************************************************/
void CreateCustomParameter::OnPickTest()
{
    // Show TestList: Single selection mode
    PickTestDialog dPickTest;

    // Disable selection of multiple layers :
    // user can only pick a test from a given layer
    dPickTest.setMultipleGroups(true, false);

    // Check if List was successfuly loaded
    if (dPickTest.fillParameterList())
    {
        // Prompt dialog box, let user pick tests from the list
        if (dPickTest.exec() == QDialog::Accepted)
        {
            // Get test# selected. string format: <Test#>.<Pinmap#>
            QString strTestsSelected = dPickTest.testItemizedList();

            if (strTestsSelected.isEmpty() == false)
            {
                strTestsSelected = QString(" T") +  strTestsSelected + QString(" ");

                setExpression(strTestsSelected, false);
            }
        }
    }
}

/******************************************************************************!
 * \fn OnClearExpression
 * \brief Empty the expression field
 ******************************************************************************/
void CreateCustomParameter::OnClearExpression()
{
    textEditFilterExpression->clear();
}

/******************************************************************************!
 * \fn OnOk
 * \brief Empty the expression field
 ******************************************************************************/
void CreateCustomParameter::OnOk()
{
    m_strExpression = textEditFilterExpression->toPlainText();
    m_strExpression = m_strExpression.trimmed();
    m_strExpression = m_strExpression.toUpper();

    done(1);
}

/******************************************************************************!
 * \fn isValidExpression
 * \brief Evaluate expression for a given sample ID
 ******************************************************************************/
bool
CreateCustomParameter::isValidExpression(long lSampleIndex, int iGroup  /*=0*/)
{
    // Expression format is of:
    // <Txxx> <Compare-Sign> <Value>
    // e.g:  T1500.1 >= 3.55

    // Extract Test number
    char* ptChar = (char*) m_strExpression.toLatin1().constData();
    if (*ptChar != 'T')
    {
        // Invalid first character, must always be (T' to define a Test#)
        return false;
    }
    // Move to first character in test#
    char* ptStart = ++ptChar;
    do
    {
        if (! isdigit(*ptChar) && *ptChar != '.')
        {
            break;  // End of Test# string

        }
        // Move to next char
        ptChar++;
    }
    while (*ptChar);
    char szTestName[50];
    strncpy(szTestName, ptStart, ptChar - ptStart);
    szTestName[ptChar - ptStart] = 0;

    long lTestNumber, lPinmap;
    if (sscanf(szTestName, "%lu%*c%ld", &lTestNumber, &lPinmap) != 2)
    {
        lPinmap = GEX_PTEST;
    }

    // Extract compare-sign
    while (*ptChar == ' ')
    {
        ptChar++;
    }
    ptStart = ptChar;
    do
    {
        if (isdigit(*ptChar) || isspace(*ptChar))
        {
            break;  // End of compare-sign string

        }
        // Move to next char
        ptChar++;
    }
    while (*ptChar);
    char szCompareSign[5];
    strncpy(szCompareSign, ptStart, ptChar - ptStart);
    szCompareSign[ptChar - ptStart] = 0;


    // Extract value
    float lfValue;
    if (sscanf(ptChar, "%f", &lfValue) != 1)
    {
        return false;  // Invalid compare value
    }
    // Evaluate expression
    CGexGroupOfFiles* pGroup = NULL;
    CGexFileInGroup*  pFile  = NULL;

    if (iGroup >= 0 && iGroup < gexReport->getGroupsList().count())
    {
        pGroup = gexReport->getGroupsList().at(iGroup);
    }

    if (pGroup == NULL)
    {
        return false;
    }

    pFile =
            (pGroup->pFilesList.isEmpty()) ? NULL :
                                             (pGroup->pFilesList.first());
    if (pFile == NULL)
    {
        return false;
    }

    // Get test entry (original test from which the new one must be derived)
    CTest* ptTestCell;
    if (pFile->FindTestCell(lTestNumber, lPinmap, &ptTestCell, true, true) != 1)
    {
        return false;  // Error: Failed finding existing entry
    }
    if (ptTestCell->m_testResult.count() <= lSampleIndex)
    {
        // Test doesn't have a sample ID as high as the current run count analyzed
        return false;
    }
    if (ReportOptions.GetOption("dataprocessing",
                                "scaling").toString() == "normalized")
    {
        // Scaling to convert from normalized value to result-scaled value
        lfValue /= ScalingPower(ptTestCell->res_scal);
    }
    // Convert sample data to 'float' as STDF file store data in float,
    // not double (and this can lead to error when comparing)
    float lfSampleResult = GEX_C_DOUBLE_NAN;
    if (ptTestCell->m_testResult.isValidIndex(lSampleIndex))
    {
        lfSampleResult = ptTestCell->m_testResult.resultAt(lSampleIndex);
    }

    QString strCompareSign = szCompareSign;

    if (strCompareSign == "<")
    {
        // Evaluate <test> < <value>
        return (lfSampleResult < lfValue) ? true : false;
    }
    if (strCompareSign == "<=")
    {
        // Evaluate <test> <= <value>
        return (lfSampleResult <= lfValue) ? true : false;
    }
    if (strCompareSign == ">")
    {
        // Evaluate <test> > <value>
        return (lfSampleResult > lfValue) ? true : false;
    }
    if (strCompareSign == ">=")
    {
        // Evaluate <test> >= <value>
        return (lfSampleResult >= lfValue) ? true : false;
    }
    if (strCompareSign == "=")
    {
        // Evaluate <test> = <value> (well as we may have a 1LSB error,
        // we don't test a clear 'equal' but a lmost equal!
        return (fabs(lfSampleResult - lfValue) <
                fabs(gex_max(lfSampleResult, lfValue) / 1e4)) ? true : false;
    }
    if (strCompareSign == "!=")
    {
        // Evaluate <test> != <value>
        return (lfSampleResult != lfValue) ? true : false;
    }
    return false;
}

/******************************************************************************!
 * \fn OnCustomParameter
 * \brief Overlay charts assistant:
 *        ADD a new overlay by creating a custom parameter
 ******************************************************************************/
void GexWizardChart::OnCustomParameter()
{
    CreateCustomParameter cNewParameter;
    long lIndex;

loop_enter_expression:
    if (cNewParameter.exec() != 1)
    {
        return;  // User abort

    }
    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles* pGroup =
            gexReport->getGroupsList().size() > 0 ?
                gexReport->getGroupsList().first() : NULL;
    CGexFileInGroup* pFile =
            (pGroup->pFilesList.isEmpty()) ? NULL :
                                             (pGroup->pFilesList.first());
    if (pFile == NULL)
    {
        return;
    }

    // Get test entry (original test from which the new one must be derived)
    CTest* ptTestCell;
    if (pFile->FindTestCell(cNewParameter.m_lOriginalTestNumber,
                            cNewParameter.m_lOriginalTestPinMap, &ptTestCell,
                            true, true) != 1)
    {
        return;  // Error: Failed finding existing entry
    }
    // Check if the expression entered is valid and
    // that some samples will match it!
    long lTotalSamples = 0;
    for (lIndex = 0; lIndex < ptTestCell->ldSamplesExecs; lIndex++)
    {
        // For each sample that passes the filter, save it in new array
        if (cNewParameter.isValidExpression(lIndex))
        {
            lTotalSamples++;
        }
    }
    if (lTotalSamples == 0)
    {
        QString strErrorMessage = "Filter expression is never verified:\n";
        strErrorMessage += cNewParameter.getExpression();
        strErrorMessage +=
                "\n\nNote: Values must be normalized (eg: 1e6 for 1Mhz)\nTry again!";
        GS::Gex::Message::information("", strErrorMessage);
        goto loop_enter_expression;
    }

    // Create new entry
    CTest* ptNewTestCell;
    unsigned long lTestNumber = 80000;
    sscanf(cNewParameter.lineEditNewTestNumber->text().toLatin1().constData(),
           "%lu", &lTestNumber);
    QString strTestName = cNewParameter.lineEditNewTestName->text();
    if (pFile->FindTestCell(lTestNumber, GEX_PTEST, &ptNewTestCell, true, true,
                            strTestName) != 1)
    {
        return;  // Error: Failed finding existing entry
    }
    // Inherit test limits, units, etc.
    ptNewTestCell->GetCurrentLimitItem()->bLimitFlag  = ptTestCell->GetCurrentLimitItem()->bLimitFlag;
    ptNewTestCell->GetCurrentLimitItem()->lfHighLimit = ptTestCell->GetCurrentLimitItem()->lfHighLimit;
    ptNewTestCell->GetCurrentLimitItem()->lfLowLimit  = ptTestCell->GetCurrentLimitItem()->lfLowLimit;
    ptNewTestCell->bTestType   = ptTestCell->bTestType;
    ptNewTestCell->hlm_scal    = ptTestCell->hlm_scal;
    ptNewTestCell->llm_scal    = ptTestCell->llm_scal;
    ptNewTestCell->res_scal    = ptTestCell->res_scal;
    strcpy(ptNewTestCell->szTestUnits, ptTestCell->szTestUnits);
    // Build limits string if applicable
    pFile->FormatTestLimit(ptNewTestCell);

    // Build test number string
    gexReport->BuildTestNumberString(ptNewTestCell);

    // Get data samples matching the Expression filter
    QString lRes=ptNewTestCell->m_testResult.createResultTable(ptTestCell->ldSamplesExecs);
    if ( lRes.startsWith("error") )
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("on custom parameter : Failed to create result table : %1").arg(lRes.toLatin1().data())
              .toLatin1().constData());
        return;  // If failed allocating the memory buffer, report error and exit
    }

    for (lIndex = 0; lIndex < ptTestCell->m_testResult.count(); lIndex++)
    {
        // For each sample that passes the filter, save it in new array
        if (cNewParameter.isValidExpression(lIndex))
        {
            ptNewTestCell->m_testResult.
                    pushResultAt(lIndex, ptTestCell->m_testResult.resultAt(lIndex));
        }
    }

    ptNewTestCell->ldSamplesExecs = ptTestCell->ldSamplesExecs;

    // Get sublot information
    for (int nSubLot = 0;
         nSubLot < ptTestCell->pSamplesInSublots.count();
         nSubLot++)
    {
        ptNewTestCell->
                pSamplesInSublots.append(ptTestCell->pSamplesInSublots.at(nSubLot));
    }

    // Compute & update statistics
    // (min, max, SumX, SumX2, sigma, mean, quartiles, etc.)
    float lfExponent = ScalingPower(ptNewTestCell->res_scal);

    if (ReportOptions.GetOption("dataprocessing",
                                "scaling").toString() == "normalized")
    {
        lfExponent = 1.0;
    }
    CGexStats cStats;
    cStats.ComputeLowLevelTestStatistics(ptNewTestCell, lfExponent);
    cStats.ComputeBasicTestStatistics(ptNewTestCell, true);
    cStats.RebuildHistogramArray(ptNewTestCell, GEX_HISTOGRAM_OVERDATA);
    QString pf =
            ReportOptions.GetOption("statistics", "cp_cpk_computation").toString();

    // Ensure we use latest options set
    cStats.UpdateOptions(&ReportOptions);

    // Computes advances stats (quartiles, etc)
    cStats.ComputeAdvancedDataStatistics(ptNewTestCell,
                                         true,
                                         pf == "standard" ? false : true);
}

/******************************************************************************!
 * \fn OnEditLayer
 * \brief Edit selected layer
 ******************************************************************************/
void GexWizardChart::OnEditLayer(QTreeWidgetItem* pSelection, int  /*nColumn*/)
{
    // If no item in list, just return!
    if (treeWidgetLayers->topLevelItemCount() <= 0)
    {
        return;
    }

    int iLayerID = 0;

    // Find LayerID doubled-clicked
    if (pSelection)
    {
        iLayerID = treeWidgetLayers->indexOfTopLevelItem(pSelection);
    }

    // Get current Layer selected
    CGexSingleChart* pLayer = NULL;

    if (iLayerID >= 0 && iLayerID <
            mChartsInfo->chartsList().count())
    {
        pLayer = mChartsInfo->chartsList().at(iLayerID);
    }

    // If no layer found (maybe no selection made), then return
    if (pLayer == NULL)
    {
        return;
    }

    // Preset Layer styles  according to layer index
    EditLayerDialog cEditLayer;
    cEditLayer.setLayerInfo(pLayer, m_eChartType);

    // Display new layer Diaog box
    if (cEditLayer.exec() != 1)
    {
        return;  // User 'Abort'

    }
    // Get layer info back
    cEditLayer.getLayerInfo(pLayer);

    /* if (iLayerID >= 0)
    {
        // Saves this new layer style into the Options repository
        // (so to be saved to disk in script file in due time)
        if(iLayerID < ReportOptions.pLayersStyleList.count() )
        {
            CGexSingleChart* pLayerStyle = ReportOptions.pLayersStyleList.at(iLayerID);
            if (pLayerStyle != NULL)
            {
                *pLayerStyle = *pLayer;
            }
        }

        // save in the current settings
        if (iLayerID < mChartsInfo->chartsList().count())// mSettings.count())
        {
            //(mSettings.at[iLayerID]) = *pLayer;
            CGexSingleChart* pLayerStyle = mChartsInfo->chartsList().at(iLayerID); //mSettings.at(iLayerID);
            if (pLayerStyle != NULL)
            {
                *pLayerStyle = *pLayer;
            }
        }
    }*/

    // Update list (as layer name may have been edited, etc)
    UpdateChartsList();

    // Switch back to 'Select' mode...avoids unexpected 'drag' or 'zoom' action!
    OnSelect();

    // Reset Zoom/Drag flags + replot if one or more Axis include a new
    if (cEditLayer.needResetViewport())
    {
        goHome();
    }
    else
    {
        // Force Redraw : Chart or Table, (depending of current tab selected)
        OnTabChange();
    }
}

/******************************************************************************!
 * \fn OnRemoveChart
 * \brief REMOVE Selected chart signal
 ******************************************************************************/
void GexWizardChart::OnRemoveChart()
{
    OnRemoveChart(false);

    // Reset Zoom/Drag flags + replot
    goHome();
}

/******************************************************************************!
 * \fn OnRemoveChart
 * \brief Overlay charts assistant: REMOVE ONE chart or ALL charts
 ******************************************************************************/
void GexWizardChart::OnRemoveChart(bool bRemoveAll)
{
    // If no item in list, just return!
    if (treeWidgetLayers->topLevelItemCount() <= 0)
    {
        return;
    }

    // Remove all content
    if (bRemoveAll)
    {
        mChartsInfo->removeCharts();
    }
    else
    {
        QList<QTreeWidgetItem*> lstSelectedItems =
                treeWidgetLayers->selectedItems();
        QTreeWidgetItem* pTreeItem = NULL;
        int iLayerID = -1;

        for (int nCount = 0; nCount < lstSelectedItems.count(); nCount++)
        {
            pTreeItem = lstSelectedItems.at(nCount);
            iLayerID  = treeWidgetLayers->indexOfTopLevelItem(pTreeItem) - nCount;

            mChartsInfo->removeChart(iLayerID);
        }
    }

    // update list of layers remaining
    UpdateChartsList();

    // Force Redraw..as edits probably affect the chart rendering!
    OnTabChange();
}

/******************************************************************************!
 * \fn OnChartProperties
 * \brief Overlay charts assistant: EDIT chart properties
 ******************************************************************************/
void GexWizardChart::OnChartProperties()
{
    // Display Edit dialog box
    OnEditLayer(treeWidgetLayers->currentItem(), 0);
}

/******************************************************************************!
 * \fn OnHistoBarSize
 * \brief Advanced: User modifies the Histogram bar width
 *        Default is 40 bars in a histogram (see TEST_ADVHISTOSIZE)
 ******************************************************************************/
void GexWizardChart::OnHistoBarSize(int iTotalBars)
{
    // Switch back to 'Select' mode...avoids unexpected 'drag' or 'zoom' action!
    OnSelect();
    QWidget* poSender = (QWidget*) sender();
    // Read number of bars (classes) to use when drawing histograms
    if (! poSender)
    {
        tmpReportOptions->SetOption("adv_histogram", "total_bars", QString::number(spinBoxHistBars->value()));

        if (customPropGroupBox->isChecked())
        {
            ptTestCell->m_iHistoClasses       = customSpinBoxHistBars->value();
            mChartsInfo->mCustomTotalBars     = customSpinBoxHistBars->value();
        }
        else
            mChartsInfo->mTotalBars = spinBoxHistBars->value();
    }
    else
    {
        if (poSender == spinBoxHistBars)
        {
            tmpReportOptions->SetOption("adv_histogram", "total_bars", QString::number(spinBoxHistBars->value()));

            if (customPropGroupBox->isChecked())
            {
                ptTestCell->m_iHistoClasses       = customSpinBoxHistBars->value();
                mChartsInfo->mCustomTotalBars     = customSpinBoxHistBars->value();
            }
            else
                mChartsInfo->mTotalBars = spinBoxHistBars->value();

            if (customPropGroupBox->isChecked())
            {
                customPropGroupBox->setChecked(false);
                ptTestCell->m_bUseCustomBarNumber = false;
                // ptTestCellY->m_bUseCustomBarNumber = false;
            }
            if (mMultipleTest)
            {
                customPropGroupBox->hide();
                //resetAllButton->hide();
            }
            else
            {
                customPropGroupBox->show();
                //resetAllButton->show();
            }
        }
        else // customPropGroupBox
        {
            ptTestCell->m_iHistoClasses       = customSpinBoxHistBars->value();
            mChartsInfo->mCustomTotalBars     = customSpinBoxHistBars->value();
            // ptTestCellY->m_iHistoClasses = customSpinBoxHistBars->value();
        }
    }

    // Force Redraw : Chart or Table, (depending of current tab selected)
    if (iTotalBars > 0)
    {
        // Force recompute Y scale as the number of
        // bars in histogram affect highest bar value!
        if (mChartsInfo->getAppliedToChart() != -1)
        {
            mChartsInfo->getViewportRectangle()[mChartsInfo->getAppliedToChart()].lfHighY = C_INFINITE;
        }
        else
        {
            foreach(int iChart, mChartsInfo->getViewportRectangle().keys())
            {
                mChartsInfo->getViewportRectangle()[iChart].lfHighY =C_INFINITE;
            }

        }

        m_pSingleChartWidget->onChartOptionChanged(GexAbstractChart::chartOptionHistoBar);
        m_pMultiChartWidget->onChartOptionChanged(GexAbstractChart::chartOptionHistoBar);
        // Repaint histogram
        OnTabChange();
    }
}

/******************************************************************************!
 * \fn OnPreviousTest
 * \brief Plot previous test (if any)
 ******************************************************************************/
void GexWizardChart::OnPreviousTest()
{
    buttonPreviousTest->setEnabled(true);
    // Check if valid call
    if (ptTestCell == NULL)
    {
        buttonPreviousTest->setEnabled(false);
        return;
    }

prev_test:

    if (m_pPrevTestX == NULL)
    {
        m_pPrevTestX = pFile->PrevTest(ptTestCell);
    }
    if (m_pPrevTestX == NULL)
    {
        buttonPreviousTest->setEnabled(false);
        return;
    }

    // Get point to Previous test
    if (pFile->
            FindTestCell(m_pPrevTestX->lTestNumber,
                         m_pPrevTestX->lPinmapIndex, &ptTestCell, false, false,
                         m_pPrevTestX->strTestName) != 1)
    {
        // No previous test, then remain on current one
        m_pPrevTestX = NULL;
        buttonPreviousTest->setEnabled(false);
        return;
    }

    // Success finding cell, then save pointer to its previous cell
    m_pPrevTestX = pFile->PrevTest(ptTestCell);

    if (gexReport->isInteractiveTestFiltered(ptTestCell) == false)
    {
        goto prev_test;
    }

    // Try to find at least data sample in one group, otherwise skip this test
    bool bValidTest = false;
    CGexGroupOfFiles* pGroupTemp     = NULL;
    CGexFileInGroup*  pFileTemp      = NULL;
    CTest*            ptTestCellTemp = NULL;

    for (int nGroup = 0;
         nGroup < gexReport->getGroupsList().count() && bValidTest == false;
         nGroup++)
    {
        pGroupTemp = gexReport->getGroupsList().at(nGroup);
        pFileTemp  =
                (pGroupTemp->pFilesList.isEmpty()) ? NULL :
                                                     (pGroupTemp->pFilesList.first());

        if (pFileTemp->
                FindTestCell(ptTestCell->lTestNumber,
                             ptTestCell->lPinmapIndex, &ptTestCellTemp, true, false,
                             ptTestCell->strTestName) == 1)
        {
            // Only list
            if ((ptTestCellTemp->ldExecs == 0 &&
                 ptTestCellTemp->GetCurrentLimitItem()->ldOutliers == 0))
            {
                continue;
            }

            // IF Muti-result parametric test, do not show master test record
            if (ptTestCellTemp->lResultArraySize > 0)
            {
                continue;
            }

            // Do not display functional test in list, only Parametric tests
            if (ptTestCellTemp->bTestType == 'F')
            {
                continue;
            }

            // Ignore Generic Galaxy Parameters
            {
                QString strOptionStorageDevice =
                        (ReportOptions.GetOption("statistics",
                                                 "generic_galaxy_tests")).toString();
                if ((ptTestCellTemp->bTestType == '-')
                        && (strOptionStorageDevice == "hide"))
                {
                    continue;
                }
            }

            // Data samples found
            bValidTest = true;
        }
    }

    // No data samples found
    if (bValidTest == false)
    {
        goto prev_test;
    }

    // emit a signal when the test is going to change to do all operations needed
    emit testChanged();

    // check if this test is the first one in the list
    // check if this test is the first one in the list
    if (IsFirstTestCell(ptTestCell))
        buttonPreviousTest->setEnabled(false);

    buttonNextTest->setEnabled(true);

    QString prefix = PrefixTab;
    QString suffix = ptTestCell->GetTestNumber();

    pGexMainWindow->ChangeWizardTabName(this, prefix, suffix);

    // Draw rolling Limits check box if needed
    ShowHideRollingLimitsMarker();
}

/******************************************************************************!
 * \fn OnPreviousTestY
 * \brief Scatter plot only: Plot previous test on Y axis (if any)
 ******************************************************************************/
void GexWizardChart::OnPreviousTestY()
{
    // Check if valid call
    if (ptTestCellY == NULL)
    {
        buttonPreviousTestY->setEnabled(false);
        return;
    }

prev_testY:

    if (m_pPrevTestY == NULL)
    {
        m_pPrevTestY = pFileY->PrevTest(ptTestCellY);
    }
    if (m_pPrevTestY == NULL)
    {
        buttonPreviousTestY->setEnabled(false);
        return;
    }

    // Get point to Previous test
    if (pFileY->
            FindTestCell(m_pPrevTestY->lTestNumber,
                         m_pPrevTestY->lPinmapIndex, &ptTestCellY, false, false,
                         m_pPrevTestY->strTestName) != 1)
    {
        // No previous test, then remain on current one
        m_pPrevTestY = NULL;
        buttonPreviousTestY->setEnabled(false);
        return;
    }

    // Success finding cell, then save pointer to its previous cell
    m_pPrevTestY = pFileY->PrevTest(ptTestCellY);

    if (gexReport->isInteractiveTestFiltered(ptTestCellY) == false)
    {
        goto prev_testY;
    }

    // Try to find at least data sample in one group, otherwise skip this test
    bool bValidTest = false;
    CGexGroupOfFiles* pGroupTemp     = NULL;
    CGexFileInGroup*  pFileTemp      = NULL;
    CTest*            ptTestCellTemp = NULL;

    for (int nGroup = 0;
         nGroup < gexReport->getGroupsList().count() && bValidTest == false;
         nGroup++)
    {
        pGroupTemp = gexReport->getGroupsList().at(nGroup);
        pFileTemp  =
                (pGroupTemp->pFilesList.isEmpty()) ? NULL :
                                                     (pGroupTemp->pFilesList.first());

        if (pFileTemp->
                FindTestCell(ptTestCellY->lTestNumber,
                             ptTestCellY->lPinmapIndex, &ptTestCellTemp, true, false,
                             ptTestCellY->strTestName) == 1)
        {
            // Only list
            if (ptTestCellTemp->ldExecs == 0 &&
                    ptTestCellTemp->GetCurrentLimitItem()->ldOutliers == 0)
            {
                continue;
            }

            // IF Muti-result parametric test, do not show master test record
            if (ptTestCellTemp->lResultArraySize > 0)
            {
                continue;
            }

            // Do not display functional test in list, only Parametric tests
            if (ptTestCellTemp->bTestType == 'F')
            {
                continue;
            }

            // Ignore Generic Galaxy Parameters
            {
                QString strOptionStorageDevice =
                        (ReportOptions.GetOption("statistics",
                                                 "generic_galaxy_tests")).toString();
                if ((ptTestCellTemp->bTestType == '-')
                        && (strOptionStorageDevice == "hide"))
                {
                    continue;
                }
            }

            // Data samples found
            bValidTest = true;
        }
    }

    // No data samples found
    if (bValidTest == false)
    {
        goto prev_testY;
    }

    // emit a signal when the test is going to change to do all operations needed
    emit testChanged();

    // check if this test is the first one in the list
    if (IsFirstTestCellY(ptTestCellY))
        buttonPreviousTestY->setEnabled(false);

    buttonNextTestY->setEnabled(true);

    // Draw rolling Limits check box if needed
    ShowHideRollingLimitsMarker();
}

/******************************************************************************!
 * \fn OnNextTest
 * \brief Plot next test (if any)
 ******************************************************************************/
void GexWizardChart::OnNextTest()
{
    CTest* pCurrentTest;

    // Check if valid call
    if (ptTestCell == NULL)
    {
        buttonNextTest->setEnabled(false);
        return;
    }

    // Backup current valid pointer (in case need to be restored)
    pCurrentTest = ptTestCell;

    int iIloop = 0;

next_test:

    iIloop++;
    if (iIloop > 10000)
    {
        // Security escape
        ptTestCell = NULL;
        return;

    }

    // Move to next test
    m_pPrevTestX = ptTestCell;
    ptTestCell = pFile->NextTest(ptTestCell);

    // Get point to Next test
    if ((ptTestCell == NULL) ||
            (pFile->FindTestCell(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex,
                                 &ptTestCell, false, false,
                                 ptTestCell->strTestName) != 1))
    {
        // No next test, then remain on current one
        ptTestCell = pCurrentTest;
        pFile->ptPrevCell = m_pPrevTestX;
        buttonNextTest->setEnabled(false);
        return;
    }

    //    // Success finding cell, then save pointer to its previous cell
    //    m_pPrevTestX = pFile->PrevTest(ptTestCell);

    if (gexReport->isInteractiveTestFiltered(ptTestCell) == false)
    {
        goto next_test;
    }

    // Try to find at least data sample in one group, otherwise skip this test
    bool bValidTest = false;
    CGexGroupOfFiles* pGroupTemp     = NULL;
    CGexFileInGroup*  pFileTemp      = NULL;
    CTest*            ptTestCellTemp = NULL;

    for (int nGroup = 0;
         nGroup < gexReport->getGroupsList().count() && bValidTest == false;
         ++nGroup)
    {
        pGroupTemp = gexReport->getGroupsList().at(nGroup);
        pFileTemp  =
                (pGroupTemp->pFilesList.isEmpty()) ? NULL :
                                                     (pGroupTemp->pFilesList.first());

        if (pFileTemp->
                FindTestCell(ptTestCell->lTestNumber,
                             ptTestCell->lPinmapIndex, &ptTestCellTemp, true, false,
                             ptTestCell->strTestName) == 1)
        {
            // Only list
            if (ptTestCellTemp->ldExecs == 0 &&
                    ptTestCellTemp->GetCurrentLimitItem()->ldOutliers == 0)
            {
                continue;
            }

            // If Muti-result parametric test, do not show master test record
            if (ptTestCellTemp->lResultArraySize > 0)
            {
                continue;
            }

            // Do not display functional test in list, only Parametric tests
            if (ptTestCellTemp->bTestType == 'F')
            {
                continue;
            }

            // Ignore Generic Galaxy Parameters
            {
                QString strOptionStorageDevice =
                        (ReportOptions.GetOption("statistics",
                                                 "generic_galaxy_tests")).toString();

                if (ptTestCellTemp->bTestType == '-' &&
                        strOptionStorageDevice == "hide")
                {
                    continue;
                }
            }

            // Data samples found
            bValidTest = true;
        }
    }

    // No data samples found
    if (bValidTest == false)
    {
        goto next_test;
    }

    // Emit a signal when the test is going to change to do all operations needed
    emit testChanged();

    // Check if valid call
    if (IsLastTestCell(ptTestCell) == true)
        buttonNextTest->setEnabled(false);

    buttonPreviousTest->setEnabled(true);

    QString prefix = PrefixTab;
    QString suffix = ptTestCell->GetTestNumber();

    pGexMainWindow->ChangeWizardTabName(this, prefix, suffix);

    // Draw rolling Limits check box if needed
    ShowHideRollingLimitsMarker();
}

/******************************************************************************!
 * \fn OnNextTestY
 * \brief Scatter plot only: Plot next test on Y axis (if any)
 ******************************************************************************/
void GexWizardChart::OnNextTestY()
{
    CTest* pCurrentTestY;

    // Check if valid call
    if (ptTestCellY == NULL)
    {
        buttonNextTestY->setEnabled(false);
        return;
    }

    // Backup current valid pointer (in case need to be restored)
    pCurrentTestY = ptTestCellY;

next_testY:

    // Move to next test
    ptTestCellY = pFileY->NextTest(ptTestCellY);

    // Get point to Next test
    if ((ptTestCellY == NULL) ||
            (pFileY->
             FindTestCell(ptTestCellY->lTestNumber, ptTestCellY->lPinmapIndex,
                          &ptTestCellY, false, false,
                          ptTestCellY->strTestName) != 1))
    {
        // No next test, then remain on current one
        ptTestCellY = pCurrentTestY;
        pFileY->ptPrevCell = m_pPrevTestY;
        buttonNextTestY->setEnabled(false);
        return;
    }

    // Success finding cell, then save pointer to its previous cell
    m_pPrevTestY = pFileY->PrevTest(ptTestCellY);

    if (gexReport->isInteractiveTestFiltered(ptTestCellY) == false)
    {
        goto next_testY;
    }

    // Try to find at least data sample in one group, otherwise skip this test
    bool bValidTest = false;
    CGexGroupOfFiles* pGroupTemp     = NULL;
    CGexFileInGroup*  pFileTemp      = NULL;
    CTest*            ptTestCellTemp = NULL;

    for (int nGroup = 0;
         nGroup < gexReport->getGroupsList().count() && bValidTest == false;
         nGroup++)
    {
        pGroupTemp = gexReport->getGroupsList().at(nGroup);
        pFileTemp  =
                (pGroupTemp->pFilesList.isEmpty()) ? NULL :
                                                     (pGroupTemp->pFilesList.first());

        if (pFileTemp->
                FindTestCell(ptTestCellY->lTestNumber,
                             ptTestCellY->lPinmapIndex, &ptTestCellTemp, true, false,
                             ptTestCellY->strTestName) == 1)
        {
            // Only list
            if ((ptTestCellTemp->ldExecs == 0 &&
                 ptTestCellTemp->GetCurrentLimitItem()->ldOutliers == 0))
            {
                continue;
            }

            // IF Muti-result parametric test, do not show master test record
            if (ptTestCellTemp->lResultArraySize > 0)
            {
                continue;
            }

            // Do not display functional test in list, only Parametric tests
            if (ptTestCellTemp->bTestType == 'F')
            {
                continue;
            }

            // Ignore Generic Galaxy Parameters
            {
                QString strOptionStorageDevice =
                        (ReportOptions.GetOption("statistics",
                                                 "generic_galaxy_tests")).toString();
                if ((ptTestCellTemp->bTestType == '-')
                        && (strOptionStorageDevice == "hide"))
                {
                    continue;
                }
            }

            // Data samples found
            bValidTest = true;
        }
    }

    // No data samples found
    if (bValidTest == false)
    {
        goto next_testY;
    }

    // emit a signal when the test is going to change to do all operations needed
    emit testChanged();

    // Check if valid call
    if (IsLastTestCellY(ptTestCellY) == true)
        buttonNextTestY->setEnabled(false);

    buttonPreviousTestY->setEnabled(true);

    // Draw rolling Limits check box if needed
    ShowHideRollingLimitsMarker();
}

/******************************************************************************!
 * \fn exitInsertMultiLayers
 * \brief Make necessary refresh after mutliple layers inserted
 ******************************************************************************/
void GexWizardChart::exitInsertMultiLayers(unsigned int lTestNumber,
                                           int          lPinmapIndex,
                                           QString      strParameterName)
{
    // This calls make sure all our pointers are reset properly
    // (in case the data files has been re-read,
    // thus having all previous pointers out of date)
    if (pFile == NULL)
    {
        isDataAvailable();
    }

    // Update GUI: Layers list + HTML info page
    UpdateChartsList();

    // Get pointer to last test# selected
    if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false, false,
                            strParameterName) != 1)
    {
        ptTestCell = NULL;
        return;  // Test not found...should never occur!
    }
    if (mMultipleTest)
    {
        ptTestCell->m_bUseCustomBarNumber = false;

    }
    if (ptTestCell->m_bUseCustomBarNumber)
    {
        customSpinBoxHistBars->setValue(ptTestCell->m_iHistoClasses);
        customPropGroupBox->setChecked(true);
    }
    else
    {
        customSpinBoxHistBars->setValue(ptTestCell->m_iHistoClasses);
        customPropGroupBox->setChecked(false);
        if (mMultipleTest)
        {
            customPropGroupBox->hide();
            //resetAllButton->hide();
        }
        else
        {
            customPropGroupBox->show();
            // resetAllButton->show();
        }
    }

    // Success finding cell, then save pointer to its previous cell
    m_pPrevTestX = pFile->PrevTest(ptTestCell);

    // Reset Zoom/Drag flags
    buttonPreviousTest->setEnabled(true);
    buttonPreviousTestY->setEnabled(true);
    buttonNextTest->setEnabled(true);
    buttonNextTestY->setEnabled(true);
    mChartsInfo->lfZoomFactorX   =
            mChartsInfo->lfZoomFactorY = -1.0;
    m_pSingleChartWidget->resetViewPortManager();
    m_pMultiChartWidget->resetViewPortManager();

    // Force Redraw : Chart or Table, (depending of current tab selected)
    OnTabChange();

    // Select 1st parameter in combo-box list
    comboBoxChartLayer->setCurrentIndex(0);

    // Update the 'Style' tab GUI content to reflect parameters options
    OnChangeChartLayer("");
}

/******************************************************************************!
 * \fn OnPickTest
 * \brief Pick tests + plot them
 ******************************************************************************/
void GexWizardChart::OnPickTest()
{
    bool bMultiselection  = true;
    bool bSelectAllGroups = true;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("On Pick Test (ProductID=%1)...")
          .arg(GS::LPPlugin::ProductInfo::getInstance()->getProductID()).toLatin1().constData());

    // Enable/disable some features
    switch (GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
    case GS::LPPlugin::LicenseProvider::eLtxcOEM:   // OEM-Examinator for LTXC
        bMultiselection = false;  // Do not allow multiple layers
        break;
    case GS::LPPlugin::LicenseProvider::eSzOEM:     // OEM-Examinator for Credence SZ
        break;
    default:
        break;
    }

    // Show TestList
    PickTestDialog dPickTest;

    // If Current plotting mode is scatter-plot: disable the multi-selection
    if (m_eChartType == GexAbstractChart::chartTypeScatter ||
            m_eChartType == GexAbstractChart::chartTypeBoxWhisker)
    {
        // Can only pick one test at a time
        bMultiselection = false;
        // Can't select a test from "All groups", need to specify which group
        bSelectAllGroups = false;
    }

    // Allow/Disable Multiple selections
    dPickTest.setMultipleSelection(bMultiselection);
    dPickTest.setMultipleGroups(true, bSelectAllGroups);
    dPickTest.setUseInteractiveFilters(true);

    // Check if List was successfuly loaded
    if (dPickTest.fillParameterList())
    {
        // Prompt dialog box, let user pick tests from the list
        if (dPickTest.exec() == QDialog::Accepted)
        {
            // This calls make sure all our pointers are reset properly
            // (in case the data files has been re-read,
            // thus having all previous pointers out of date)
            isDataAvailable();

            // Get test# selected
            // String format: <Test#>.<Pinmap#> , <Test#>.<Pinmap#> , etc.
            QString strTestsSelected =
                    dPickTest.testItemizedList();
            QStringList strTestsNamesSelected = dPickTest.testItemizedListNames();
            QStringList strTestListNumber     = strTestsSelected.split(",", QString::KeepEmptyParts);

            GEX_ASSERT(strTestListNumber.count() == strTestsNamesSelected.count());

            if (strTestsSelected.isEmpty() == false)
            {
                int         iGroupID = dPickTest.getGroupID();
                QString     strParameterName;
                QStringList strParameters;
                strParameters     = strTestsSelected.split(",", QString::KeepEmptyParts);
                mMultipleTest = strParameters.count() > 1;


                // Clear Previous layers
                mChartsInfo->removeCharts();

                // Insert all parameters selected to create relevant charts
                //bool bDelete = true;
                int lastIndex = 0;
                for (int i = 0; i < strParameters.count(); ++i, ++lastIndex)
                {
                    // For each parameter listed, add it to the list!
                    if (sscanf(strParameters[i].toLatin1().constData(), "%lu%*c%ld",
                               &lTestNumber, &lPinmapIndex) < 2)
                    {
                        lPinmapIndex = GEX_PTEST;  // No Pinmap index specified

                    }
                    if (i < strTestsNamesSelected.size())
                        strParameterName = strTestsNamesSelected[i];

                    addChart(false,
                             false,
                             lTestNumber,
                             lPinmapIndex,
                             dPickTest.GetOriginalTestName(strParameterName),
                             iGroupID,0,-1,"",-1, true);
                }

                // to apply the settings style to the added chart
                OnChangeStyle();

                // Do necessary GUI refresh & cleanup as multiple layers inserted
                exitInsertMultiLayers(lTestNumber,
                                      lPinmapIndex,
                                      dPickTest.GetOriginalTestName(strParameterName));

                QString prefix = PrefixTab;
                QString suffix = QString::number(lTestNumber);

                pGexMainWindow->ChangeWizardTabName(this, prefix, suffix);
            }
        }
    }

    if (mMultipleTest)
    {
        if (customPropGroupBox)
        {
            customPropGroupBox->hide();
        }
        //resetAllButton->hide();
    }
    else
    {
        customPropGroupBox->show();
        //resetAllButton->show();
    }

    // Show/Hide rolling limits check box if needed
    ShowHideRollingLimitsMarker();

    ManageNextAndPreviousTestButton();
}

/******************************************************************************!
 * \fn OnPickTestY
 * \brief Scatter plot only: Pick test + plot it
 ******************************************************************************/
void GexWizardChart::OnPickTestY()
{
    // Show TestList
    PickTestDialog dPickTest;

    // Only allow ONE selection
    dPickTest.setMultipleSelection(false);
    // Do not allow to select "All groups"
    dPickTest.setMultipleGroups(true, false);
    dPickTest.setUseInteractiveFilters(true);

    // Check if List was successfuly loaded
    if (dPickTest.fillParameterList() == true)
    {
        // Prompt dialog box, let user pick tests from the list
        if (dPickTest.exec() == QDialog::Accepted)
        {
            // This calls make sure all our pointers are reset properly
            // (in case the data files has been re-read,
            // thus having all previous pointers out of date)
            isDataAvailable();

            // Get test# selected. string format: <Test#>.<Pinmap#>
            QString strTestSelected = dPickTest.testList();

            if (strTestSelected.isEmpty())
            {
                return;
            }

            QStringList strTestsNamesSelected = dPickTest.testItemizedListNames();
            QString     strParameterName      = strTestsNamesSelected[0];

            if (sscanf(strTestSelected.toLatin1().constData(), "%lu%*c%ld",
                       &lTestNumber, &lPinmapIndex) < 2)
            {
                lPinmapIndex = GEX_PTEST;  // No Pinmap index specified

            }
            // Get pointer to test#
            if (pFileY->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCellY,
                                     false, false,
                                     dPickTest.GetOriginalTestName(strParameterName)) != 1)
            {
                ptTestCellY = NULL;
                return;  // Test not found...should never occur!
            }

            // Make this chart replace the current overlays
            addChart(true, true, ptTestCell, ptTestCellY);

            // Reset Zoom/Drag flags
            buttonPreviousTest->setEnabled(true);
            buttonPreviousTestY->setEnabled(true);
            buttonNextTest->setEnabled(true);
            buttonNextTestY->setEnabled(true);
            mChartsInfo->lfZoomFactorX   =
                    mChartsInfo->lfZoomFactorY = -1.0;
            m_pSingleChartWidget->resetViewPortManager();
            m_pMultiChartWidget->resetViewPortManager();

            // Force Redraw : Chart or Table, (depending of current tab selected)
            OnTabChange();
        }
    }

    // Show/Hide rolling limits check box if needed
    ShowHideRollingLimitsMarker();
}

/******************************************************************************!
 * \fn CheckForDataAvailability
 * \brief Check if samples available: data collection MAY have been
 *        disabled for speed optimization reasons!
 ******************************************************************************/
bool GexWizardChart::CheckForDataAvailability()
{
    if (pGexMainWindow == NULL)
    {
        return false;
    }

    if (ReportOptions.bSpeedCollectSamples == false)
    {
        // Hide Charting window
        stackedWidgetChart->setCurrentIndex(1);
        // No data available. No chart!
        return false;
    }
    else
    {
        switch (m_eChartType)
        {
        case GexAbstractChart::chartTypeHistogram:
        case GexAbstractChart::chartTypeScatter:
        case GexAbstractChart::chartTypeBoxWhisker:
        case GexAbstractChart::chartTypeTrend:
        case GexAbstractChart::chartTypeBoxPlot:
        case GexAbstractChart::chartTypeProbabilityPlot:
            stackedWidgetChart->setCurrentIndex(1);
            break;

        default:
            GEX_ASSERT(false);
            break;
        }

        // Data available: can chart them!
        return true;
    }
}

/******************************************************************************!
 * \fn OnChartType
 * \brief  Charting mode: Histogram, Trend
 ******************************************************************************/
void GexWizardChart::OnChartType(int iSelection)
{
    int nChartType     = GEX_ADV_HISTOGRAM;  // Histogram chart
    int nViewportType  = m_iHistoViewport;
    int nChartSelected = comboBoxChartType->itemData(iSelection).toInt();

    // hide subset limit's check box and draw it onlu in trend type
    checkBoxSubsetLimits->setHidden(true);

    // Set variables for chart type and type of viewport, depending on selection
    switch (nChartSelected)
    {
    case GexAbstractChart::chartTypeHistogram:
        // Histogram
        nChartType    = GEX_ADV_HISTOGRAM;  // Histogram chart
        nViewportType = m_iHistoViewport;
        break;
    case GexAbstractChart::chartTypeTrend:
        // Trend
        nChartType    = GEX_ADV_TREND;  // Trend chart
        nViewportType = m_iTrendViewport;
        break;
    case GexAbstractChart::chartTypeScatter:
        nChartType    = GEX_ADV_CORRELATION;  // Correlation chart
        nViewportType = m_iScatterViewport;
        break;
    case GexAbstractChart::chartTypeBoxPlot:
        // Scatter
        nChartType    = GEX_ADV_CANDLE_MEANRANGE;
        nViewportType = m_iBoxPlotViewport;
        break;
    case GexAbstractChart::chartTypeProbabilityPlot:
        // Probability plot
        nChartType    = GEX_ADV_PROBABILITY_PLOT;
        nViewportType = m_iProbabilityViewport;
        break;
    default:
        GEX_ASSERT(false);
        break;
    }

    if(ReportOptions.mComputeAdvancedTest == false &&
       (nChartType == GEX_ADV_CANDLE_MEANRANGE || nChartType == GEX_ADV_PROBABILITY_PLOT) )
    {
        QMessageBox::warning(this, "No data computed", "[No data computed] This chart requires that you enable advanced statistics computation through "
                             "the option [Speed optimization] -> [Computing advanced statistics]");
    }

    checkBoxfittingCurve->hide();
    checkBoxBellCurve->hide();
    checkBoxStack->hide();
    checkBox3DBars->hide();
    checkBoxBars->hide();

    GSLOG(SYSLOG_SEV_DEBUG, QString("On chart type: ChartType=%1 ViewportType=%2").arg( nChartType).arg( nViewportType).toLatin1().constData());
    // Activate relevant chart
    switch (nChartSelected)
    {
    case GexAbstractChart::chartTypeHistogram:
        // Histogram
        OnHistogram();
        break;
    case GexAbstractChart::chartTypeTrend:
        // Trend
        OnTrend();
        break;
    case GexAbstractChart::chartTypeScatter:
    case GexAbstractChart::chartTypeBoxWhisker:
        // Scatter
        OnScatter();
        break;
    case GexAbstractChart::chartTypeBoxPlot:
        // BoxPlot - Gage R&R
        OnBoxPlot();
        break;
    case GexAbstractChart::chartTypeProbabilityPlot:
        // Probability Plot
        OnProbabilityPlot();
        break;
    default:
        GEX_ASSERT(false);
        break;
    }

    // Force Redraw : Chart or Table, (depending of current tab selected)
    OnTabChange();

    switch (nChartSelected)
    {
    case GexAbstractChart::chartTypeHistogram:
        // Histogram
        textLabelHistBars->show();
        spinBoxHistBars->show();
        customPropGroupBox->show();
        //resetAllButton->show();
        break;
    case GexAbstractChart::chartTypeTrend:
        // Trend
        textLabelHistBars->hide();
        spinBoxHistBars->hide();
        customPropGroupBox->hide();
        //resetAllButton->hide();
        break;
    case GexAbstractChart::chartTypeScatter:
    case GexAbstractChart::chartTypeBoxWhisker:
        // Scatter
        textLabelHistBars->hide();
        spinBoxHistBars->hide();
        customPropGroupBox->hide();
        //resetAllButton->hide();
        break;
    case GexAbstractChart::chartTypeBoxPlot:
        // BoxPlot - Gage R&R
        textLabelHistBars->hide();
        spinBoxHistBars->hide();
        customPropGroupBox->hide();
        //resetAllButton->hide();
        break;
    case GexAbstractChart::chartTypeProbabilityPlot:
        // Probability Plot
        textLabelHistBars->hide();
        spinBoxHistBars->hide();
        customPropGroupBox->hide();
        //resetAllButton->hide();
        break;
    default:
        //        GEX_ASSERT(false);
        break;
    }
}

/******************************************************************************!
 * \fn OnHistogram
 * \brief Charting mode: Histogram
 ******************************************************************************/
void GexWizardChart::OnHistogram()
{
    checkBoxStack->show();
    checkBoxfittingCurve->show();
    checkBoxBellCurve->show();
    checkBox3DBars->show();
    checkBoxBars->show();

    if(checkBoxBellCurve->isChecked()  == false && checkBoxfittingCurve->isChecked() == false)
    {
        checkBoxspots->setEnabled(false);
        checkBoxspots->setChecked(false);
    }

    // Make Combo selection to: Histogram
    comboBoxChartType->
            setCurrentIndex(comboBoxChartType->
                            findData(QVariant(GexAbstractChart::chartTypeHistogram)));

    // Reset zoom & drag offsets
    mChartsInfo->lfZoomFactorX   =  mChartsInfo->lfZoomFactorY = -1.0;
    m_pSingleChartWidget->resetViewPortManager();

    // Force drawing mode to Histogram
    m_eChartType = GexAbstractChart::chartTypeHistogram;  // Histogram chart
    // Saves ploting mode: over data, limits, both
    m_pSingleChartWidget->setChartType(m_eChartType, mChartsInfo, m_iHistoViewport, mAnchorAction->isChecked());

    // Fill combo-box
    comboBoxDataWindow->clear();
    comboBoxDataWindow->insertItem(0, "Chart over test limits",                     QVariant(GexHistogramChart::viewportOverLimits));
    comboBoxDataWindow->insertItem(1, "Cumulate over test limits",                  QVariant(GexHistogramChart::viewportCumulLimits));
    comboBoxDataWindow->insertItem(2, "Chart over test results",                    QVariant(GexHistogramChart::viewportOverData));
    comboBoxDataWindow->insertItem(3, "Cumulate over test results",                 QVariant(GexHistogramChart::viewportCumulData));
    comboBoxDataWindow->insertItem(SYSLOG_SEV_WARNING, "Adaptive: data & limits",   QVariant(GexHistogramChart::viewportAdaptive));

    comboBoxDataWindow->setCurrentIndex(comboBoxDataWindow->findData(QVariant(m_iHistoViewport)));

    // Refresh Advanced custom settings + info shown
    if (tabWidgetStyles->widget(2) == NULL)
    {
        tabWidgetStyles->addTab(pTabHistoScales, "Scales");
    }

    // Show useless control on Scale page
    textLabelHistBars->show();
    spinBoxHistBars->show();
    if (! mMultipleTest)
    {
        customPropGroupBox->show();
        //resetAllButton->show();
    }
    else
    {
        customPropGroupBox->hide();
        //resetAllButton->hide();
    }
    // Fill scale combo box
    comboBoxScaleType->clear();
    comboBoxScaleType->
            insertItem(comboBoxScaleType->count(), tr("Percentage  (%)"));
    comboBoxScaleType->
            insertItem(comboBoxScaleType->count(), tr("Frequency count / Hits"));

    // Set the text label
    textLabelScaleType->setText("Y scale:");

    // Histogram Y-axis type (percentage or Hits)

    if (tmpReportOptions->GetOption("adv_histogram",
                                    "y_axis").toString() == "hits")
    {
        comboBoxScaleType->setCurrentIndex(1);  // Hits
    }
    else
    {
        comboBoxScaleType->setCurrentIndex(0);  // Percentage
    }

    checkBoxLotID->setEnabled(false);
    checkBoxSubLotID->setEnabled(false);
    checkBoxGroupName->setEnabled(false);

    OnHistoBarSize(-1);

    // Histogram, trend, scatter drawing options GUI
    widgetStackDrawingMode->setCurrentIndex(0);

    // Check if samples available: data collection MAY have been
    // disabled for speed optimization reasons! + Display message
    CheckForDataAvailability();
}


/******************************************************************************!
 * \fn OnTrend
 * \brief Charting mode: Trend
 ******************************************************************************/
void GexWizardChart::OnTrend()
{
    checkBoxspots->setEnabled(true);


    // Make Combo selection to: Trend
    comboBoxChartType->
            setCurrentIndex(comboBoxChartType->
                            findData(QVariant(GexAbstractChart::chartTypeTrend)));

    // Reset zoom & drag offsets
    mChartsInfo->lfZoomFactorX   =
            mChartsInfo->lfZoomFactorY = -1.0;

    // Force drawing mode to Trend
    m_eChartType = GexAbstractChart::chartTypeTrend;  // Trend chart
    // Saves ploting mode: over data, limits, both
    m_pSingleChartWidget->setChartType(m_eChartType, mChartsInfo,
                                       m_iTrendViewport,
                                       mAnchorAction->isChecked());

    // Fill combo-box
    comboBoxDataWindow->clear();
    comboBoxDataWindow->insertItem(0, "Chart over test limits",
                                   QVariant(GexTrendChart::viewportOverLimits));
    comboBoxDataWindow->insertItem(1, "Chart over test results",
                                   QVariant(GexTrendChart::viewportOverData));
    comboBoxDataWindow->insertItem(2, "Adaptive: data & limits",
                                   QVariant(GexTrendChart::viewportAdaptive));

    comboBoxDataWindow->
            setCurrentIndex(comboBoxDataWindow->findData(QVariant(m_iTrendViewport)));

    // Show advanced settings
    // Refresh Advanced custom settings + info shown
    if (tabWidgetStyles->widget(2) == NULL)
    {
        tabWidgetStyles->addTab(pTabHistoScales, "Scales");
    }

    // Fill scale combo box
    comboBoxScaleType->clear();
    comboBoxScaleType->addItem(tr("Run ID"));
    comboBoxScaleType->addItem(tr("Part ID"));

    // Set the text label
    textLabelScaleType->setText("X scale:");

    // Probability plot X-axis type (run_id or part_id)
    bool bIsGuiUpdateValid = false;
    if (tmpReportOptions->GetOption("adv_trend",
                                    "x_axis").toString() == "run_id")
    {
        comboBoxScaleType->setCurrentIndex(0);  // run id
        bIsGuiUpdateValid = SetTrendChartXAxis(QString("run_id"));
        GEX_ASSERT(bIsGuiUpdateValid);
    }
    else
    {
        comboBoxScaleType->setCurrentIndex(1);  // should be 'part_id'
        bIsGuiUpdateValid = SetTrendChartXAxis(QString("part_id"));
        GEX_ASSERT(bIsGuiUpdateValid);
    }

    // Hide useless control on Scale page
    textLabelHistBars->hide();
    spinBoxHistBars->hide();
    customPropGroupBox->hide();
    //resetAllButton->hide();
    // Histogram, trend, scatter drawing options GUI
    widgetStackDrawingMode->setCurrentIndex(0);

    // Check if samples available: data collection MAY have been
    // disabled for speed optimization reasons! + Display message
    CheckForDataAvailability();
}

/******************************************************************************!
 * \fn OnScatter
 * \brief Charting mode: Scatter plot
 ******************************************************************************/
void GexWizardChart::OnScatter()
{
    // Make Combo selection to: Scatter
    comboBoxChartType->
            setCurrentIndex(comboBoxChartType->
                            findData(QVariant(GexAbstractChart::chartTypeScatter)));

    // Reset zoom & drag offsets
    mChartsInfo->lfZoomFactorX   =
            mChartsInfo->lfZoomFactorY = -1.0;
    m_pSingleChartWidget->resetViewPortManager();

    if (radioButtonScatter->isChecked())
    {
        m_eChartType = GexAbstractChart::chartTypeScatter;  // Scatter chart
    }
    else
    {
        m_eChartType = GexAbstractChart::chartTypeBoxWhisker;  // Box whisker
    }
    // Saves ploting mode: over data, limits, both
    m_pSingleChartWidget->setChartType(m_eChartType, mChartsInfo,
                                       m_iScatterViewport,
                                       mAnchorAction->isChecked());

    // Fill combo-box
    comboBoxDataWindow->clear();
    comboBoxDataWindow->insertItem(0, "Chart over test limits",
                                   QVariant(GexScatterChart::viewportOverLimits));
    comboBoxDataWindow->insertItem(1, "Chart over test results",
                                   QVariant(GexScatterChart::viewportOverData));
    comboBoxDataWindow->insertItem(2, "Adaptive: data & limits",
                                   QVariant(GexScatterChart::viewportAdaptive));

    comboBoxDataWindow->
            setCurrentIndex(comboBoxDataWindow->findData(QVariant(m_iScatterViewport)));

    // Hide Histogram advanced settings
    if (tabWidgetStyles->widget(2) != NULL)
    {
        tabWidgetStyles->removeTab(tabWidgetStyles->indexOf(pTabHistoScales));
    }

    checkBoxLotID->setEnabled(false);
    checkBoxSubLotID->setEnabled(false);
    checkBoxGroupName->setEnabled(false);

    // Histogram, trend, scatter drawing options GUI
    widgetStackDrawingMode->setCurrentIndex(2);

    customPropGroupBox->hide();
    //resetAllButton->hide();

    // Check if samples available: data collection MAY have been
    // disabled for speed optimization reasons! + Display message
    CheckForDataAvailability();
}

/******************************************************************************!
 * \fn OnBoxPlot
 * \brief Charting mode: BoxPlot
 ******************************************************************************/
void GexWizardChart::OnBoxPlot()
{
    // Make Combo selection to: Scatter
    comboBoxChartType->
            setCurrentIndex(comboBoxChartType->
                            findData(QVariant(GexAbstractChart::chartTypeBoxPlot)));

    // Reset zoom & drag offsets
    mChartsInfo->lfZoomFactorX   =
            mChartsInfo->lfZoomFactorY = -1.0;

    // Force drawing mode to BoxPlot
    m_eChartType = GexAbstractChart::chartTypeBoxPlot;

    // Saves ploting mode: over data, limits, both
    m_pSingleChartWidget->setChartType(m_eChartType, mChartsInfo,
                                       m_iBoxPlotViewport,
                                       mAnchorAction->isChecked());

    // Fill combo-box
    comboBoxDataWindow->clear();
    comboBoxDataWindow->insertItem(0, "Chart over test limits",
                                   QVariant(GexBoxPlotChart::viewportOverLimits));
    comboBoxDataWindow->insertItem(1, "Chart over test results",
                                   QVariant(GexBoxPlotChart::viewportOverData));
    comboBoxDataWindow->insertItem(2, "Adaptive: data & limits",
                                   QVariant(GexBoxPlotChart::viewportAdaptive));

    comboBoxDataWindow->
            setCurrentIndex(comboBoxDataWindow->findData(QVariant(m_iBoxPlotViewport)));

    checkBoxLotID->setEnabled(false);
    checkBoxSubLotID->setEnabled(false);
    checkBoxGroupName->setEnabled(false);

    // Hide Histogram advanced settings
    if (tabWidgetStyles->widget(2) != NULL)
    {
        tabWidgetStyles->removeTab(tabWidgetStyles->indexOf(pTabHistoScales));
    }

    // Boxplot drawing options GUI
    widgetStackDrawingMode->setCurrentIndex(1);

    customPropGroupBox->hide();
    //resetAllButton->hide();

    // Check if samples available: data collection MAY have been
    // disabled for speed optimization reasons! + Display message
    CheckForDataAvailability();
}

/******************************************************************************!
 * \fn OnProbabilityPlot
 * \brief Charting mode: ProbabilityPlot
 ******************************************************************************/
void GexWizardChart::OnProbabilityPlot()
{
    // OEM-Examinator for LTXC
    if (GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        comboBoxChartType->setCurrentIndex(comboBoxChartType->findData(QVariant(m_eChartType)));

        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        GS::Gex::Message::information("", m);
        return;
    }

    // Make Combo selection to: Probability plot
    comboBoxChartType->
            setCurrentIndex(comboBoxChartType->
                            findData(QVariant(GexAbstractChart::
                                              chartTypeProbabilityPlot)));

    // Reset zoom & drag offsets
    mChartsInfo->lfZoomFactorX   =
            mChartsInfo->lfZoomFactorY = -1.0;
    m_pSingleChartWidget->resetViewPortManager();

    // Force drawing mode to BoxPlot
    m_eChartType = GexAbstractChart::chartTypeProbabilityPlot;

    // Saves ploting mode: over data, limits, both
    m_pSingleChartWidget->setChartType(m_eChartType, mChartsInfo,
                                       m_iProbabilityViewport,
                                       mAnchorAction->isChecked());

    // Fill combo-box
    comboBoxDataWindow->clear();
    comboBoxDataWindow->insertItem(0, "Chart over test limits",
                                   QVariant(GexProbabilityPlotChart::
                                            viewportOverLimits));
    comboBoxDataWindow->insertItem(1, "Chart over test results",
                                   QVariant(GexProbabilityPlotChart::
                                            viewportOverData));
    comboBoxDataWindow->insertItem(2, "Adaptive: data & limits",
                                   QVariant(GexProbabilityPlotChart::
                                            viewportAdaptive));

    comboBoxDataWindow->
            setCurrentIndex(comboBoxDataWindow->
                            findData(QVariant(m_iProbabilityViewport)));

    // Show advanced settings
    // Refresh Advanced custom settings + info shown
    if (tabWidgetStyles->widget(2) == NULL)
    {
        tabWidgetStyles->addTab(pTabHistoScales, "Scales");
    }

    // Fill scale combo box
    comboBoxScaleType->clear();
    comboBoxScaleType->addItem(tr("Z (Sigma)"));
    comboBoxScaleType->addItem(tr("Cumulative Probability"));

    // Set the text label
    textLabelScaleType->setText("Y scale:");

    // Probability plot Y-axis type (Cumulative Probability or sigma)
    QString pbya =
            tmpReportOptions->GetOption("adv_probabilityplot", "y_axis").toString();
    if (pbya == "sigma")
    {
        comboBoxScaleType->setCurrentIndex(0);  // Sigma
    }
    else if (pbya ==
             "percentage")
    {
        comboBoxScaleType->setCurrentIndex(1);  // Cumulative Probability
    }
    // Hide useless control on Scale page
    textLabelHistBars->hide();
    spinBoxHistBars->hide();
    customPropGroupBox->hide();
    //resetAllButton->hide();
    // Boxplot drawing options GUI
    widgetStackDrawingMode->setCurrentIndex(1);

    checkBoxLotID->setEnabled(false);
    checkBoxSubLotID->setEnabled(false);
    checkBoxGroupName->setEnabled(false);

    // Check if samples available: data collection MAY have been
    // disabled for speed optimization reasons! + Display message
    CheckForDataAvailability();
}

/******************************************************************************!
 * \fn OnTabChange
 * \brief Tab ('Chart' or 'Table') clicked
 ******************************************************************************/
void GexWizardChart::OnTabChange(int nCurrentTab  /* = -1 */)
{
    // Reset cursor selection mode to default
    OnSelect();
    onSelectMulti();

    // Check if samples available: data collection MAY have been
    // disabled for speed optimization reasons! + Display message
    CheckForDataAvailability();

    // If no test pointed yet...quietly return
    if (ptTestCell == NULL || gexReport == NULL)
    {
        return;
    }

    // If parametric test with no samples available while SampleCount > 0,
    // means the 'Options' is set to disable data collection because of
    // dataset too big
    if ((ptTestCell->bTestType != 'F') &&
            (ptTestCell->m_testResult.count() == 0) &&
            (ptTestCell->ldSamplesExecs > 5000))
    {
        // case 5867 :
        GSLOG(SYSLOG_SEV_WARNING,
              QString("A lot of samples (%1) will be loaded."
                      " Low memory state could happen soon.").arg(ptTestCell->ldSamplesExecs).toLatin1().constData());
        // Evaluation mode, refuse to run this function!
        /* Message::
       information(this, szAppFullName,
                  "No samples available because dataset is bigger than the"
                  " limit allowed!\nTo change the limit, click the 'Options'"
                  " tab\nand go into section 'Speed Optimization/Data points"
                  " available...'");
       return;*/
    }

    QWidget* pWidgetTab = (nCurrentTab >= 0) ?
                tabWidgetChart->widget(nCurrentTab) :
                tabWidgetChart->currentWidget();

    if (pWidgetTab == tabChart)
    {
        activateChartTab();
        if (! mMultipleTest)
        {
            customPropGroupBox->show();
            //resetAllButton->show();
        }
        else
        {
            customPropGroupBox->hide();
            //resetAllButton->hide();
        }
    }
    else if (pWidgetTab == tabXBarR)
    {
        activateXBarRChartTab();
        frameParameters->show();
    }
    else if (pWidgetTab == tabMultiChart)
    {
        customPropGroupBox->hide();
        //resetAllButton->hide();
        activateMuiltiChartTab();
    }
    else if (pWidgetTab == tabTable)
    {
        activateTableTab();
    }
}

/******************************************************************************!
 * \fn activateChartTab
 ******************************************************************************/
void GexWizardChart::activateChartTab()
{
    // Show Layer and style page
    if (tabWidgetStyle->indexOf(tabLayers) == -1)
    {
        tabWidgetStyle->insertTab(0, tabLayers, "Layers");
    }

    if (tabWidgetStyle->indexOf(tabStyle) == -1)
    {
        tabWidgetStyle->insertTab(-1, tabStyle, "Style");
    }

    // Redraw single chart if need be
    PlotRelevantChart();
}

/******************************************************************************!
 * \fn activateMuiltiChartTab
 ******************************************************************************/
void GexWizardChart::activateMuiltiChartTab()
{
    if(ReportOptions.mComputeAdvancedTest == false )
    {
        QMessageBox::warning(this, "No data computed", "Box Plot and Probability plot charts require that you enable advanced statistics computation through "
                             "the option [Speed optimization] -> [Computing advanced statistics].");

    }

    // Show Layer and style page
    if (tabWidgetStyle->indexOf(tabLayers) == -1)
    {
        tabWidgetStyle->insertTab(0, tabLayers, "Layers");
    }

    if (tabWidgetStyle->indexOf(tabStyle))
    {
        tabWidgetStyle->insertTab(-1, tabStyle, "Style");
    }

    textLabelParamX->setText("");
    textLabelParamY->setText("");
    buttonPreviousTestY->hide();
    buttonNextTestY->hide();
    mPickTestYAction->setEnabled(false);

    // Redraw multi chart if need be
    //m_pMultiChartWidget->update();
    m_pMultiChartWidget->replotChart(chartReportOptions(), currentTestCellX(), currentTestCellY());
}

/******************************************************************************!
 * \fn activateXBarRChartTab
 ******************************************************************************/
void GexWizardChart::activateXBarRChartTab()
{
    // Hide layers tab and style tab
    tabWidgetStyle->removeTab(tabWidgetStyle->indexOf(tabLayers));
    tabWidgetStyle->removeTab(tabWidgetStyle->indexOf(tabStyle));

    // Update skin
    textLabelParamX->setText("");
    textLabelParamY->setText("");
    buttonPreviousTestY->hide();
    buttonNextTestY->hide();
    mPickTestYAction->setEnabled(false);

    // Redraw XBar R chart if need be
    tabXBarR->replotChart();
}

/******************************************************************************!
 * \fn activateTableTab
 ******************************************************************************/
void GexWizardChart::activateTableTab()
{
    // Show Layer and style page
    if (tabWidgetStyle->indexOf(tabLayers) == -1)
    {
        tabWidgetStyle->insertTab(0, tabLayers, "Layers");
    }

    if (tabWidgetStyle->indexOf(tabStyle))
    {
        tabWidgetStyle->insertTab(-1, tabStyle, "Style");
    }

    // Load relevant data in the table
    ListRelevantRawData();
}

/******************************************************************************!
 * \fn OnCorrelationMode
 * \brief Advanced: User modifies the correlation charting mode
 ******************************************************************************/
void GexWizardChart::OnCorrelationMode(bool bChecked)
{
    comboBoxWhiskerOrientation->setEnabled(! bChecked);

    OnScatter();

    OnTabChange();
}

/******************************************************************************!
 * \fn OnWhiskerOrientation
 * \brief Advanced: User modifies the whisker orientation
 ******************************************************************************/
void GexWizardChart::OnWhiskerOrientation(int nIndex)
{
    int nItemData = comboBoxWhiskerOrientation->itemData(nIndex).toInt();

    if (nItemData == GexBoxWhiskerChart::Horizontal)
    {
        tmpReportOptions->SetOption("adv_correlation",
                                    "boxwhisker_orientation",
                                    "horizontal");
    }
    else
    {
        tmpReportOptions->SetOption("adv_correlation",
                                    "boxwhisker_orientation",
                                    "vertical");
    }

    OnTabChange();
}

/******************************************************************************!
 * \fn OnScaleType
 * \brief Advanced: User modifies the Y scale type
 ******************************************************************************/
void GexWizardChart::OnScaleTypeIndexChanged(int)
{
    mChartsInfo->mYScale = comboBoxScaleType->currentIndex();
    OnScaleType("");
}

void GexWizardChart::OnScaleType(const QString&  /*strSelection*/)
{
    // Switch back to 'Select' mode...avoids unexpected 'drag' or 'zoom' action!
    OnSelect();

    switch (m_eChartType)
    {
    case GexAbstractChart::chartTypeHistogram:
        // Refresh Y-axis ploting mode
        if (comboBoxScaleType->currentIndex() == 0)
        {
            tmpReportOptions->SetOption("adv_histogram", "y_axis", "percentage");
        }
        else
        {
            tmpReportOptions->SetOption("adv_histogram", "y_axis", "hits");
        }

        m_pSingleChartWidget->
                onChartOptionChanged(GexAbstractChart::chartOptionHistoBar);
        m_pMultiChartWidget->
                onChartOptionChanged(GexAbstractChart::chartOptionHistoBar);
        break;
    case GexAbstractChart::chartTypeProbabilityPlot:
        // Refresh Y-axis ploting mode
        if (comboBoxScaleType->currentIndex() == 0)
        {
            tmpReportOptions->SetOption("adv_probabilityplot", "y_axis", "sigma");
        }
        else
        {
            tmpReportOptions->
                    SetOption("adv_probabilityplot", "y_axis", "percentage");
        }
        break;
    case GexAbstractChart::chartTypeTrend:
    {
        // Refresh X-axis ploting mode
        bool bIsValidGuiUpdate = false;
        if (comboBoxScaleType->currentIndex() == 0)
        {
            bIsValidGuiUpdate = SetTrendChartXAxis(QString("run_id"));
            GEX_ASSERT(bIsValidGuiUpdate);
            // tmpReportOptions->SetOption("adv_trend","x_axis","run_id");
        }
        else
        {
            bIsValidGuiUpdate = SetTrendChartXAxis(QString("part_id"));
            GEX_ASSERT(bIsValidGuiUpdate);
            // tmpReportOptions->SetOption("adv_trend","x_axis","part_id");
        }
        break;
    }
    default:
        break;
    }

    // Force recompute Y scale (hits or %)
    if (m_eChartType >= GexAbstractChart::chartTypeHistogram &&
            m_eChartType <= GexAbstractChart::chartTypeCharacHisto)
    {
        mChartsInfo->getViewportRectangle()[m_eChartType].lfHighY =
                C_INFINITE;
    }
    else
    {
        foreach(int iChart,
                mChartsInfo->getViewportRectangle().keys()) {
            mChartsInfo->getViewportRectangle()[iChart].lfHighY =
                    C_INFINITE;
        }
    }
    // Force Redraw : Chart or Table, (depending of current tab selected)
    OnTabChange();
}

/******************************************************************************!
 * \fn SetTrendChartXAxis
 * \brief Managed gui from X_Axis trend chart config
 ******************************************************************************/
bool GexWizardChart::SetTrendChartXAxis(const QString strXAxisOpt)
{
    bool lAllowedTrendMarker = false;

    if (gexReport->getGroupsList().count() == 1 && strXAxisOpt == QString("run_id"))
        lAllowedTrendMarker = true;

    if (lAllowedTrendMarker)
    {
        checkBoxLotID->setEnabled(true);
        checkBoxSubLotID->setEnabled(true);
        checkBoxGroupName->setEnabled(true);
    }
    else
    {
        checkBoxLotID->setChecked(false);
        checkBoxSubLotID->setChecked(false);
        checkBoxGroupName->setChecked(false);

        bool bIsValidSetOption = true;
        bIsValidSetOption &= tmpReportOptions->SetSpecificFlagOption(
                    QString("adv_trend"), QString("marker"), QString("lot"),
                    false);
        bIsValidSetOption &= tmpReportOptions->SetSpecificFlagOption(
                    QString("adv_trend"), QString("marker"), QString("sublot"),
                    false);
        bIsValidSetOption &= tmpReportOptions->SetSpecificFlagOption(
                    QString("adv_trend"), QString("marker"),
                    QString("group_name"), false);
        GEX_ASSERT(bIsValidSetOption);

        checkBoxLotID->setEnabled(false);
        checkBoxSubLotID->setEnabled(false);
        checkBoxGroupName->setEnabled(false);
    }

    return tmpReportOptions->SetOption(QString("adv_trend"), QString("x_axis"),
                                       strXAxisOpt);
}

void GexWizardChart::OnDeselectHighlightedDie()
{
    if (gexReport)
    {
        gexReport->ClearCustomMarkers("Selection");

        OnTabChange();
    }
}

/******************************************************************************!
 * \fn OnTableEdited
 * \brief Table value changed...update the Statistics in the HTML page!
 ******************************************************************************/
void GexWizardChart::OnTableEdited(int  /*row*/, int  /*col*/)
{
    if (pFile == NULL)
    {
        return;
    }

    // If no test pointed yet...quietly return
    if (ptTestCell == NULL || gexReport == NULL)
    {
        return;
    }

    // Update the samples data in Gex memory + updates Test statistics HTML page
    gexReport->UpdateSamplesRawData(mChartsInfo,
                                    tableWidget, pFile, ptTestCell);
}

/******************************************************************************!
 * \fn OnExportChartStats
 * \brief Save Chart statistics to the clipboard
 ******************************************************************************/
void GexWizardChart::OnExportChartStats()
{
#if (defined _WIN32 || __linux__)
    if (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() ==
            GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
                    "", "This function is disabled in Evaluation mode\n\nContact " +
                    QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    // Get path to HTML Layer Stats info and open file to read it and copy into
    QString strBigLine;
    QFile   file(strPropertiesPath); // Read the text from the HTML file
    if (file.open(QIODevice::ReadOnly) == false)
    {
        return;  // Failed opening file
    }
    // Read all file
    QTextStream hFile;
    hFile.setDevice(&file);  // Assign file handle to data stream
    do
    {
        // Read one line from file
        strBigLine += hFile.readLine();
    }
    while (hFile.atEnd() == false);
    file.close();

    // Copy string to clipboard
    QClipboard* lClipBoard = QGuiApplication::clipboard();
    if (!lClipBoard)
    {
        GSLOG(SYSLOG_SEV_WARNING, "failed to retrieve application clipboard");
    }
    else
    {
        lClipBoard->setText(strBigLine, QClipboard::Clipboard);
    }

    // Prompt user if launch Excel?
    QString strMessage =
            "Statistics saved to clipboard in Spreadsheet format,\n"
            "ready to paste!\nLaunch Spreadsheet application now?";
    bool lOk;
    GS::Gex::Message::request("", strMessage, lOk);
    if (! lOk)
    {
        return;
    }
#ifdef _WIN32
    // Launch excel
    ShellExecuteA(NULL, "open",
                  ReportOptions.GetOption("preferences",
                                          "ssheet_editor").toString().toLatin1().constData(),
                  NULL, NULL, SW_SHOWNORMAL);
#else
    // system(ReportOptions.m_PrefMap["ssheet_editor"].toLatin1().constData());
    if (system(ReportOptions.GetOption("preferences", "ssheet_editor").
               toString().toLatin1().constData()) == -1)
    {
        //FIXME: send error
    }
#endif

#endif
}

/******************************************************************************!
 * \fn OnExportExcelClipboard
 * \brief Save data samples into clipboard, ready to paste to spreadsheet file
 *        (Windows only)
 ******************************************************************************/
void GexWizardChart::OnExportExcelClipboard()
{
#if (defined _WIN32 || __linux__)
    if (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() ==
            GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
                    "", "This function is disabled in Evaluation mode\n\nContact " +
                    QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    if (pFile == NULL)
    {
        return;
    }

    // If no test pointed yet...quietly return
    if (ptTestCell == NULL || gexReport == NULL)
    {
        return;
    }

    long    lLine, lCol, lMaxCol;
    int     iIndex;
    QString strCell;
    QString strMessage;
    QString strBigLine;
    QTableWidgetItem* pTableWidgetItem = NULL;

    // Write as first data line the Query names
    for (lCol = 0; lCol < tableWidget->columnCount(); lCol++)
    {
        pTableWidgetItem = tableWidget->horizontalHeaderItem(lCol);
        // Write cell string
        strBigLine += pTableWidgetItem->text();
        // Insert 2 tabs:
        // because data that follow are in 2 columns: <Value> , <units>
        strBigLine += "\t\t";
    }
    strBigLine += "\n";

    for (lLine = 0; lLine < tableWidget->rowCount(); lLine++)
    {
        // Ignore all leading empty cells on each line
        lMaxCol = tableWidget->columnCount();
        while (lMaxCol > 0)
        {
            pTableWidgetItem = tableWidget->item(lLine, lMaxCol - 1);

            if (pTableWidgetItem)
            {
                strCell = pTableWidgetItem->text().trimmed();
                if (strCell.isEmpty() == false)
                {
                    break;  // Reached the last cell that is not blank
                }
            }

            lMaxCol--;
        }

        // Write cell string
        for (lCol = 0; lCol < lMaxCol; lCol++)
        {
            // Write Data <tab> Units <tab>
            pTableWidgetItem = tableWidget->item(lLine, lCol);

            if (pTableWidgetItem)
            {
                strCell = pTableWidgetItem->text();
            }
            else
            {
                strCell.clear();
            }

            iIndex = strCell.indexOf(' ');
            if (iIndex >= 0)
            {
                strCell.insert(iIndex, "\t");
            }
            else
            {
                strCell += "\t";
            }

            // Export to clipboard in XL format:
            strBigLine += strCell;
            strBigLine += "\t";
        }
        strBigLine += "\n";
    }

    switch (m_eChartType)
    {
    case GexAbstractChart::chartTypeHistogram:  // Data is: Histogram
    case GexAbstractChart::chartTypeTrend:  // Data is: Trend
    case GexAbstractChart::chartTypeScatter:  // Data is: Scatter
    case GexAbstractChart::chartTypeBoxPlot:  // Data is: BoxPlot
    case GexAbstractChart::chartTypeProbabilityPlot:  // Data is: Probability plot
    {
        QClipboard* lClipBoard = QGuiApplication::clipboard();
        if (!lClipBoard)
        {
            GSLOG(SYSLOG_SEV_WARNING, "failed to retrieve application clipboard");
        }
        else
        {
            lClipBoard->setText(strBigLine, QClipboard::Clipboard);
        }
    }
        break;

    default:
        GEX_ASSERT(false);
        break;
    }

    strMessage =
            "Data saved to clipboard in Spreadsheet format,\n"
            "ready to paste!\nLaunch Spreadsheet application now?";
    bool lOk;
    GS::Gex::Message::request("", strMessage, lOk);
    if (! lOk)
    {
        return;
    }
#ifdef _WIN32
    // Launch excel
    ShellExecuteA(NULL, "open",
                  ReportOptions.GetOption("preferences",
                                          "ssheet_editor").toString().toLatin1().
                  data(),  //ReportOptions.m_PrefMap["ssheet_editor"],
                  NULL, NULL, SW_SHOWNORMAL);
#else
    // system(ReportOptions.m_PrefMap["ssheet_editor"].toLatin1().constData());
    if (system(ReportOptions.GetOption("preferences", "ssheet_editor").
               toString().toLatin1().constData()) == -1)
    {
        //FIXME: send error
    }
#endif

#endif
}

/******************************************************************************!
 * \fn OnExportData
 * \brief Save data samples into spreadsheet csv file
 ******************************************************************************/
void GexWizardChart::OnExportData()
{
    if (GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() ==
            GEX_RUNNINGMODE_EVALUATION)
    {
        // Evaluation mode, refuse to run this function!
        GS::Gex::Message::information(
                    "", "This function is disabled in Evaluation mode\n\nContact " +
                    QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

    if (pFile == NULL)
    {
        return;
    }

    // If no test pointed yet...quietly return
    if (ptTestCell == NULL || gexReport == NULL)
    {
        return;
    }

    // Let's user tell where to save the table
    QString strName;
    QString strFile;
    switch (m_eChartType)
    {
    case GexAbstractChart::chartTypeHistogram:  // Data is: Histogram
    case GexAbstractChart::chartTypeTrend:  // Data is: Trend
    case GexAbstractChart::chartTypeBoxPlot:  // Data is: BoxPlot
    case GexAbstractChart::chartTypeProbabilityPlot:  // Data is: Probability plot

        // Only one test listed, use it to built file name
        // File name: <Test#>.csv
        strName = ptTestCell->szTestLabel;
        break;

    case GexAbstractChart::chartTypeScatter:  // Data is: Scatter
    case GexAbstractChart::chartTypeBoxWhisker:  // Data is: Scatter
        // Only one test listed, use it to built file name
        // File name: <Test#_inX>-<Test#_inY>.csv
        strName  = ptTestCell->szTestLabel;
        strName += "-";
        strName += ptTestCellY->szTestLabel;
        break;

    default:
        GEX_ASSERT(false);
        break;
    }
    strName += ".csv";

    strFile =
            QFileDialog::getSaveFileName(this, "Save Table data as...", strName,
                                         "Spreadsheet CSV(*.csv)", NULL,
                                         QFileDialog::DontConfirmOverwrite);

    // If no file selected, ignore command
    if (strFile.isEmpty())
    {
        return;
    }

    // Check if file exists
    QFile f(strFile);
    if (f.exists() == true)
    {
        // File exists...overwrite it?
        bool lOk;
        GS::Gex::Message::request("", "File already exists. Overwrite it ?", lOk);
        if (! lOk)
        {
            return;
        }
    }

    // Assign file I/O stream
    QTextStream hCsvTableFile;
    QString     strCell;
    QString     strMessage;
    int  iIndex;
    long lLine, lCol, lMaxCol;

    // Create file
    if (! f.open(QIODevice::WriteOnly))
    {
        // Error. Can't create CSV file!
        GS::Gex::Message::information("", "Failed to create CSV data file\n"
                                          "Disk full or write protection issue.");
        return;
    }
    hCsvTableFile.setDevice(&f);

    // Header (1st few columns to list tests involved.)
    QString strHeader[11];
    switch (m_eChartType)
    {
    case GexAbstractChart::chartTypeHistogram:  // Data is: Histogram
    case GexAbstractChart::chartTypeTrend:  // Data is: Trend
    case GexAbstractChart::chartTypeBoxPlot:  // Data is: BoxPlot
    case GexAbstractChart::chartTypeProbabilityPlot:  // Data is: Probability plot
        strHeader[9] = GS::Gex::Engine::GetInstance().Get("AppFullName").toString()
                + QString(",,,,");  // Examinator release name
        strHeader[10] = "www.mentor.com,,,,";  // Galaxy Web site
        if (mChartsInfo->chartsList().count() != 1)
        {
            for (int i = 0; i < 9; i++)
            {
                strHeader[i] = ",,,,";  // Blank line
            }
            break;  // If multi-layers, do not write test name+limits
        }
        strHeader[0]  = "Test number, ";
        strHeader[0] += ptTestCell->szTestLabel;
        strHeader[0] += ",,,";
        strHeader[1]  = "Test name, ";
        strHeader[1] += ptTestCell->strTestName;
        strHeader[1] += ",,,";
        strHeader[2]  = "Low Limit, ";
        strHeader[2] += ptTestCell->GetCurrentLimitItem()->szLowL;
        strHeader[2] += ",,,";
        strHeader[3]  = "High Limit, ";
        strHeader[3] += ptTestCell->GetCurrentLimitItem()->szHighL;
        strHeader[3] += ",,,";
        strHeader[4]  = ",,,,"; // Blank line
        strHeader[5]  = ",,,,"; // Blank line
        strHeader[6]  = ",,,,"; // Blank line
        strHeader[7]  = ",,,,"; // Blank line
        strHeader[8]  = ",,,,"; // Blank line
        break;
    case GexAbstractChart::chartTypeScatter:  // Data is: Scatter plot (2 tests)
    case GexAbstractChart::chartTypeBoxWhisker:  // Data is: Scatter
        strHeader[0]  = "X: Test number, ";
        strHeader[0] += ptTestCell->szTestLabel;
        strHeader[0] += ",,,";
        strHeader[1]  = "X: Test name, ";
        strHeader[1] += ptTestCell->strTestName;
        strHeader[1] += ",,,";
        strHeader[2]  = "X: Low Limit, ";
        strHeader[2] += ptTestCell->GetCurrentLimitItem()->szLowL;
        strHeader[2] += ",,,";
        strHeader[3]  = "X: High Limit, ";
        strHeader[3] += ptTestCell->GetCurrentLimitItem()->szHighL;
        strHeader[3] += ",,,";
        strHeader[4]  = ",,,,"; // Blank line between the 2 tests description
        strHeader[5]  = "Y: Test number, ";
        strHeader[5] += ptTestCell->szTestLabel;
        strHeader[5] += ",,,";
        strHeader[6]  = "Y: Test name, ";
        strHeader[6] += ptTestCell->strTestName;
        strHeader[6] += ",,,";
        strHeader[7]  = "Y: Low Limit, ";
        strHeader[7] += ptTestCell->GetCurrentLimitItem()->szLowL;
        strHeader[7] += ",,,";
        strHeader[8]  = "Y: High Limit, ";
        strHeader[8] += ptTestCell->GetCurrentLimitItem()->szHighL;
        strHeader[8] += ",,,";
        strHeader[9]  = GS::Gex::Engine::GetInstance().Get("AppFullName").toString()
                + QString(",,,,");  // Examinator release name
        strHeader[10] = "www.mentor.com,,,,";  // Galaxy Web site
        break;
    default:
        GEX_ASSERT(false);
        break;
    }

    QTableWidgetItem* pTableWidgetItem = NULL;

    // Write as first data line the Query names
    hCsvTableFile << GS::Gex::Engine::GetInstance().Get("AppFullName").toString()
                  << ",,,,";
    for (lCol = 0; lCol < tableWidget->columnCount(); lCol++)
    {
        pTableWidgetItem = tableWidget->horizontalHeaderItem(lCol);

        // Write cell string
        hCsvTableFile << "," << pTableWidgetItem->text() << ",";
    }
    hCsvTableFile << endl;

    for (lLine = 0; lLine < tableWidget->rowCount(); lLine++)
    {
        // Ignore all leading empty cells on each line
        lMaxCol = tableWidget->columnCount();

        while (lMaxCol > 0)
        {
            pTableWidgetItem = tableWidget->item(lLine, lMaxCol - 1);

            if (pTableWidgetItem)
            {
                strCell = pTableWidgetItem->text().trimmed();

                if (strCell.isEmpty() == false)
                {
                    break;  // Reached the last cell that is not blank
                }
            }

            lMaxCol--;
        }

        // Header lines to write?
        if (lLine <= 10)
        {
            hCsvTableFile << strHeader[lLine];
        }
        else
        {
            hCsvTableFile << ",,,,";  // Leave first 4 columns empty

        }
        // Write cell string
        for (lCol = 0; lCol < lMaxCol; lCol++)
        {
            // Write Data , Units
            pTableWidgetItem = tableWidget->item(lLine, lCol);

            if (pTableWidgetItem)
            {
                strCell = pTableWidgetItem->text();
            }
            else
            {
                strCell.clear();
            }

            iIndex = strCell.indexOf(' ');
            if (iIndex >= 0)
            {
                strCell.insert(iIndex, ",");
            }
            else
            {
                strCell += ",";
            }
            hCsvTableFile << "," << strCell;
        }

        // Add new line character
        hCsvTableFile << endl;
    }

    // Close file
    f.close();

    strMessage = "Table saved!\nFile created: " + strFile;
    GS::Gex::Message::information("", strMessage);
}

/******************************************************************************!
 * \fn OnFilterParameters
 * \brief Filter the parameters listed in the Test Statistics table
 ******************************************************************************/
void GexWizardChart::OnFilterParameters()
{
    bool bState = mFilterTestAction->isChecked();
    if (GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        // OEM-Examinator for LTX
        QString m = ReportOptions.GetOption("messages", "upgrade").toString();

        GS::Gex::Message::information("", m);
        mFilterTestAction->setChecked(false);
        return;
    }

    if (bState)
    {
        // Show TestList
        PickTestDialog dialogPickTest;

        // Allow/Disable Multiple selections
        dialogPickTest.setMultipleSelection(true);
        dialogPickTest.setMultipleGroups(false, false);
        // dialogPickTest.setAllowedTestType(PickTestDialog::TestAll);

        // Check if List was successfuly loaded
        if (dialogPickTest.fillParameterList() && dialogPickTest.exec() ==
                QDialog::Accepted)
        {
            // Get test# selected
            // String format: <Test#>.<Pinmap#> , <Test#>.<Pinmap#> , etc.
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

    // check if this test is the first one in the list
    ManageNextAndPreviousTestButton();
}

/******************************************************************************!
  This function enable and desable the buttonPreviousTest and buttonPreviousTest
 ******************************************************************************/
void GexWizardChart::ManageNextAndPreviousTestButton()
{
    if (IsFirstTestCell(ptTestCell) == true)
        buttonPreviousTest->setEnabled(false);
    else
        buttonPreviousTest->setEnabled(true);

    if (IsFirstTestCellY(ptTestCellY) == true)
        buttonPreviousTestY->setEnabled(false);
    else
        buttonPreviousTestY->setEnabled(true);

    // check if this test is the last one in the list
    if (IsLastTestCell(ptTestCell) == true)
        buttonNextTest->setEnabled(false);
    else
        buttonNextTest->setEnabled(true);

    if (IsLastTestCellY(ptTestCellY) == true)
        buttonNextTestY->setEnabled(false);
    else
        buttonNextTestY->setEnabled(true);
}


/******************************************************************************!
 * \fn onFilterChanged
 * \brief Update view with parameter list
 ******************************************************************************/
void GexWizardChart::onFilterChanged(bool bState)
{
    if (bState)
    {
        bool bReload = false;

        if (gexReport->isInteractiveTestFiltered(ptTestCell) == false)
        {
            ptTestCell = findFirstValidTestCell();
            bReload    = true;
        }

        if (ptTestCellY &&
                gexReport->isInteractiveTestFiltered(ptTestCellY) == false)
        {
            ptTestCellY = findFirstValidTestCellY();
            bReload     = true;
        }

        if (bReload)
        {
            // Make this chart replace the current overlays
            addChart(true, true, ptTestCell, ptTestCellY);

            // Reset Zoom/Drag flags
            mChartsInfo->lfZoomFactorX   =
                    mChartsInfo->lfZoomFactorY = -1.0;
            m_pSingleChartWidget->resetViewPortManager();
            m_pMultiChartWidget->resetViewPortManager();

            // Force Redraw : Chart or Table, (depending of current tab selected)
            OnTabChange();

            // Select 1st parameter in combo-box list
            comboBoxChartLayer->setCurrentIndex(0);

            // Update the 'Style' tab GUI content to reflect parameters options
            OnChangeChartLayer("");
        }
    }

    mFilterTestAction->setChecked(gexReport->hasInteractiveTestFilter());
}

/******************************************************************************!
 * \fn OnRemoveTableDataPoints
 * \brief Remove data points selected in the table
 *        (not available under Scatter plot mode)
 ******************************************************************************/
void GexWizardChart::OnRemoveTableDataPoints()
{
    if (tableWidget->selectedItems().size() <= 0)
    {
        GS::Gex::Message::information("", "No selection made!");
        return;
    }

    // Request to confirm data removal
    bool lOk;
    GS::Gex::Message::request("Confirm removal", "Confirm to remove data points selected ?", lOk);
    if (! lOk)
    {
        return;
    }

    CTest* ptTestCell;
    long   lTestNumber, lPinmapIndex;
    CGexSingleChart* pChart = NULL;  // Handle to Parameter Layer info
    unsigned iLayerIndex    = 0;
    unsigned iTotalLayers   = mChartsInfo->chartsList().count();

    // If no layers...exit!
    if (iTotalLayers <= 0)
    {
        return;
    }

    // X column per group (as many as layers)
    QTableWidgetItem* pTableWidgetItem = NULL;
    int  iRow, iCol;
    bool bShowPartID =
            (ReportOptions.GetOption("dataprocessing",
                                     "part_id").toString() == "show") ? true : false;

    // Stack all charts of all groups for each same test
    for (iLayerIndex = 0; iLayerIndex < iTotalLayers; iLayerIndex++)
    {
        pChart = mChartsInfo->chartsList().at(iLayerIndex);
        lTestNumber  = pChart->iTestNumberX;
        lPinmapIndex = pChart->iPinMapX;

        // Check if we have the PartID visible
        if (bShowPartID)
        {
            iCol = 1 + (2 * iLayerIndex);
        }
        else
        {
            iCol = iLayerIndex;
        }

        // Get pointer to relevant group
        if ((pChart->iGroupX < 0) ||
                (pChart->iGroupX >= gexReport->getGroupsList().size()))
        {
            pGroup = NULL;
        }
        else
        {
            pGroup = gexReport->getGroupsList().at(pChart->iGroupX);
        }
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false,
                                false, pChart->strTestNameX) == 1)
        {
            // Get all datapoints selected in the column
            // (scan from last line then backward so to avoid having
            // removal offsets be adjusted to compensate data offsets removed!)
            for (iRow = tableWidget->rowCount() - 1; iRow >= 0; iRow--)
            {
                pTableWidgetItem = tableWidget->item(iRow, iCol);
                if (pTableWidgetItem && pTableWidgetItem->isSelected())
                {
                    // Remove data point at position 'iRow'
                    gexReport->RemoveTableDataPoint(ptTestCell, iRow);
                }
            }
            // Force to compute the new statistics as one or
            // more data points have been removed
            // Index of '-1' means no data remove, only compute stats
            gexReport->RemoveTableDataPoint(ptTestCell, -1, true);
        }
    }

    // Reload Table of raw data
    ListRelevantRawData();

    // Flag to rebuild 'Interactive Table' in due time
    clear();

    // Force to refresh 'Statistics' HTML page
    OnChangeChartLayer("");

    // Force Redraw (clear zoom level)
    goHome();
}

/******************************************************************************!
 * \fn OnContextualMenu
 * \brief Contextual menu on Layer list clicked
 ******************************************************************************/
void GexWizardChart::OnContextualMenu(const QPoint&  /*ptPoint*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "On chart contextual menu...");
    QAction* pActionEdit = NULL;
    QAction* pActionShow = NULL;
    QAction* pActionHide = NULL;
    QAction* pActionShowAllLayers      = NULL;
    QAction* pActionHideAllLayers      = NULL;
    QAction* pActionAddLayer           = NULL;
//    QAction* pActionRemoveLayer        = NULL;
    QAction* pActionShowTheseLayerOnly = NULL;
    QAction* pActionBringLayerToFront  = NULL;
    QAction* pActionSendLayerToBack    = NULL;
    QIcon    icEdit;
//    QIcon    icShowHide;
    QIcon    icAddLayer;
//    QIcon    icRemoveLayer;

    // Enable/disable some features...
    switch (GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
    case GS::LPPlugin::LicenseProvider::eLtxcOEM:   // OEM-Examinator for LTXC
    case GS::LPPlugin::LicenseProvider::eSzOEM:     // OEM-Examinator for Credence SZ
        return;
    default:
        break;
    }

    // Find LayerID right-clicked
    QList<QTreeWidgetItem*> lstSelectedItems = treeWidgetLayers->selectedItems();
    QTreeWidgetItem* pTreeItem = NULL;
    int iLayerID = -1;
    CGexSingleChart* pLayer = NULL;

    // Build menu
    QMenu menu(this);

    if (lstSelectedItems.count() == 1)
    {
        pTreeItem = lstSelectedItems.at(0);
        iLayerID  = treeWidgetLayers->indexOfTopLevelItem(pTreeItem);

        if (iLayerID > 0)
        {
            pActionBringLayerToFront = menu.addAction("Bring to front");
        }

        if (iLayerID < mChartsInfo->chartsList().count() - 1)
        {
            pActionSendLayerToBack = menu.addAction("Send to back");
        }

        menu.addSeparator();
    }

    // If A layer is selected...allow to Show / Hide it)
    if (lstSelectedItems.count() > 0)
    {
        pActionHide = menu.addAction(icEdit, "Hide Selected Layers");
        pActionShow = menu.addAction(icEdit, "Show Selected Layers");

        if (mChartsInfo->chartsList().count() > 1)
        {
            pActionShowTheseLayerOnly = menu.addAction(icEdit,
                                                       "Show Selected Layers Only");
        }

        menu.addSeparator();
    }

    pActionHideAllLayers = menu.addAction(icEdit, "Hide ALL Layers");
    pActionShowAllLayers = menu.addAction(icEdit, "Show ALL Layers");

    menu.addSeparator();

    icAddLayer.addPixmap(*pixCreateFolder);
    pActionAddLayer = menu.addAction(icAddLayer, "Add Layer");

    // If A layer is selected...allow to Remove it and o show its properties
    if (lstSelectedItems.count() > 0)
    {
//        icRemoveLayer.addPixmap(*pixRemove);
//        pActionRemoveLayer =
//                menu.addAction(icRemoveLayer, "Remove Selected Layers");

//        menu.addSeparator();

        if (lstSelectedItems.count() == 1)
        {
            icEdit.addPixmap(*pixProperties);
            pActionEdit = menu.addAction(icEdit, "Edit Layer Properties");
        }
    }

    menu.setMouseTracking(true);
    QAction* pActionResult = menu.exec(QCursor::pos());
    menu.setMouseTracking(false);

    if (pActionResult == NULL)
    {
        return;
    }

    // If Show/Hide toggle
    if (pActionResult == pActionShow || pActionResult == pActionHide)
    {
        bool bVisible = (pActionResult == pActionShow) ? true : false;

        for (int nCount = 0; nCount < lstSelectedItems.count(); nCount++)
        {
            pTreeItem = lstSelectedItems.at(nCount);
            iLayerID  = treeWidgetLayers->indexOfTopLevelItem(pTreeItem);

            if (iLayerID >= 0 && iLayerID <
                    mChartsInfo->chartsList().count())
            {
                pLayer = mChartsInfo->chartsList().at(iLayerID);
            }

            if (pLayer)
            {
                pLayer->bVisible = bVisible;
            }
        }

        // Update list: so to show the new pixmap image
        // (Either the layer color, or a red cross)
        UpdateChartsList();

        // Force Redraw : Chart or Table, (depending of current tab selected)
        OnTabChange();
    }

    // Show or Hide ALL layers
    if (pActionResult == pActionShowAllLayers ||
            pActionResult == pActionHideAllLayers)
    {
        // Set all layers bVisible attibute
        bool bVisible;
        bVisible = (pActionResult == pActionShowAllLayers) ? true : false;

        // Edit al layers
        for (int iLayerIndex = 0;
             iLayerIndex < mChartsInfo->chartsList().count();
             iLayerIndex++)
        {
            pLayer = mChartsInfo->chartsList().at(iLayerIndex);

            // Set Show/Hide attribute
            pLayer->bVisible = bVisible;
        }

        // Update list: so to show the new pixmap image
        // (Either the layer color, or a red cross)
        UpdateChartsList();

        // Force Redraw : Chart or Table, (depending of current tab selected)
        OnTabChange();
    }

    // Show only ONE layer (the one selected), hide all others
    if (pActionResult == pActionShowTheseLayerOnly)
    {
        for (int iLayerIndex = 0;
             iLayerIndex < mChartsInfo->chartsList().count();
             iLayerIndex++)
        {
            pTreeItem = treeWidgetLayers->topLevelItem(iLayerIndex);

            if (pTreeItem)
            {
                pLayer = mChartsInfo->chartsList().at(iLayerIndex);

                // Set Show/Hide attribute
                pLayer->bVisible = (pTreeItem->isSelected()) ? true : false;
            }
        }

        // Update list: so to show the new pixmap image
        // (Either the layer color, or a red cross)
        UpdateChartsList();

        // Force Redraw : Chart or Table, (depending of current tab selected)
        OnTabChange();
    }

    // If Add Layer
    if (pActionResult == pActionAddLayer)
    {
        OnAddChart();
    }

//    // If Remove Layer
//    if (pActionResult == pActionRemoveLayer)
//    {
//        OnRemoveChart(false);
//    }

    // 'Edit Layer Properties'
    if (pActionResult == pActionEdit)
    {
        OnChartProperties();
    }

    // Bring layer to front or send it to back
    if (pActionResult == pActionBringLayerToFront ||
            pActionResult == pActionSendLayerToBack)
    {
        pTreeItem = lstSelectedItems.at(0);
        iLayerID  = treeWidgetLayers->indexOfTopLevelItem(pTreeItem);

        if (iLayerID >= 0 && iLayerID <
                mChartsInfo->chartsList().count())
        {
            if (pActionResult == pActionBringLayerToFront)
            {
                mChartsInfo->moveFront(iLayerID);
            }
            else
            {
                mChartsInfo->moveBack(iLayerID);
            }
        }

        // Update list: so to show the new pixmap image
        // (Either the layer color, or a red cross)
        UpdateChartsList();

        // Force Redraw : Chart or Table, (depending of current tab selected)
        OnTabChange();
    }
}

/******************************************************************************!
 * \fn OnTableContextualMenu
 * \brief Show contextual menu from Table
 ******************************************************************************/
void GexWizardChart::OnTableContextualMenu(const QPoint& ptMousePoint)
{
    QAction* pActionRemoveDatapoints  = NULL;
    QAction* pActionDeviceResults     = NULL;
    QAction* pActionSaveToDisk        = NULL;
    QAction* pActionSaveToClipboard   = NULL;
    QAction* pActionShowPartMarker    = NULL;
    QAction* pActionShowDie           = NULL;
    QAction* pActionShowParametricDie = NULL;

    QIcon   icSave(*pixSave);
    QIcon   icExcel(*pixCSVSpreadSheet);
    QIcon   icRemoveDatapoints(*pixAdvHisto);
    QIcon   icDeviceResults(*pixTestStatistics);
    QIcon   icWafer(*pixWafermap);
    QString strSelection;

    CGexSingleChart*  pChart = NULL;
    CGexGroupOfFiles* pGroup = NULL;
    CGexFileInGroup*  pFile  = NULL;
    CTest*            ptCellDieX;
    CTest*            ptCellDieY;
    int lDieX(-32768), lDieY(-32768);

    // Enable/disable some features...
    switch (GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
    case GS::LPPlugin::LicenseProvider::eLtxcOEM:   // OEM-Examinator for LTXC
    case GS::LPPlugin::LicenseProvider::eSzOEM:     // OEM-Examinator for Credence SZ
        return;
    default:
        break;
    }

    QMenu* menu = new QMenu(this);

    int nCurrentRow = tableWidget->rowAt(ptMousePoint.y());
    int nCurrentCol = tableWidget->columnAt(ptMousePoint.x());

    // "Remove data point" is NOT available under scatter plot mode
    if ((tableWidget->selectedItems().count() == 1) &&
            ((m_eChartType == GexAbstractChart::chartTypeHistogram) ||
             (m_eChartType == GexAbstractChart::chartTypeTrend)))
    {
        pActionShowPartMarker = menu->addAction(*pixAdvHisto,
                                                "Show Value/Part on histogram");

        // Show results in device only available if one dataset group only!
        if (gexReport->getGroupsList().count() == 1)
        {
            QString strString = "Show all Tests results for Part#: " +
                    QString::number(nCurrentRow + 1);
            pActionDeviceResults = menu->addAction(*pixTestStatistics, strString);

            pGroup =
                    gexReport->getGroupsList().isEmpty() ? NULL :
                                                           gexReport->getGroupsList().first();
            if (pGroup->cStackedWaferMapData.bWaferMapExists)
            {
                pActionShowDie = menu->addAction(
                            *pixWafermap,
                            "Show Die on Soft-Bin wafermap");
                pActionShowParametricDie = menu->addAction(
                            *pixWafermap,
                            "Show Value on parametric wafermap");
            }
        }
        menu->addSeparator();
    }

    // "Remove data point" is NOT available under scatter plot mode
    // Samle for 'show all results in part'
    if ((tableWidget->selectedItems().count() > 0) &&
            ((m_eChartType == GexAbstractChart::chartTypeHistogram) ||
             (m_eChartType == GexAbstractChart::chartTypeTrend)))
    {
        pActionRemoveDatapoints =
                menu->addAction(*pixRemove,
                                "Remove data points (table items selected)");
        menu->addSeparator();
    }

#ifdef _WIN32  // Allow to save to clipboard in spreadsheet format
    pActionSaveToClipboard =
            menu->addAction(icExcel, "Save Table to clipboard (Spreadsheet format)");
#endif
    // Allow to export to disk
    pActionSaveToDisk = menu->addAction(icSave, "Save Table to disk...");

    menu->setMouseTracking(true);
    QAction* pActionResult = menu->exec(QCursor::pos());
    menu->setMouseTracking(false);

    // Delete allocated ressources
    delete menu; menu = 0;

    if (pActionResult == NULL)
    {
        return;
    }

    // 'Save to Clipboard (spreadsheet format)'
    if (pActionResult == pActionSaveToClipboard)
    {
        OnExportExcelClipboard();
    }

    // 'Save to disk'
    if (pActionResult == pActionSaveToDisk)
    {
        OnExportData();
    }

    // Remove selected datapoints
    if (pActionResult == pActionRemoveDatapoints)
    {
        OnRemoveTableDataPoints();
    }

    if (pActionResult == pActionDeviceResults)
    {
        onDeviceResults(nCurrentRow, lDieX, lDieY);

        // Scroll back to row
        tableWidget->scrollToItem(tableWidget->itemAt(ptMousePoint.x(),
                                                      ptMousePoint.y()));
    }

    // Show selected part/value on histogram chart
    if (pActionResult == pActionShowPartMarker)
    {
        OnShowPartOnHistogram(nCurrentRow, nCurrentCol);
    }

    // Show die on wafermap
    if (pActionResult == pActionShowDie)
    {
        // Get die location
        pGroup =
                gexReport->getGroupsList().isEmpty() ? NULL :
                                                       gexReport->getGroupsList().first();
        pFile =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                                                 (pGroup->pFilesList.first());
        if (pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &ptCellDieX,
                                true, false) != 1)
        {
            return;
        }
        if (pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &ptCellDieY,
                                true, false) != 1)
        {
            return;
        }

        GEX_ASSERT(ptCellDieX->m_testResult.isValidIndex(nCurrentRow));
        GEX_ASSERT(ptCellDieY->m_testResult.isValidIndex(nCurrentRow));
        lDieX =
                ptCellDieX->m_testResult.isValidIndex(nCurrentRow) ?
                    (int) ptCellDieX->m_testResult.resultAt(nCurrentRow) :
                    (int) GEX_C_DOUBLE_NAN;
        lDieY =
                ptCellDieY->m_testResult.isValidIndex(nCurrentRow) ?
                    (int) ptCellDieY->m_testResult.resultAt(nCurrentRow) :
                    (int) GEX_C_DOUBLE_NAN;

        // Launch 3D window...unless already detached (not a child window hidden)
        pGexMainWindow->ShowWizardDialog(GEX_DRILL_3D_WIZARD_P1);

        Gex::DrillDataMining3D*  lWafer3DWizard = pGexMainWindow->LastCreatedWizardWafer3D();

        // Make it display the wafermap
        lWafer3DWizard->ShowChart(0, 0,GEX_TESTNBR_OFFSET_EXT_SBIN, GEX_PTEST, GEX_DRILL_WAFERPROBE);

        // Select relevant die
        lWafer3DWizard->SelectDieLocation(pFile, lDieX, lDieY, true);

        // redraw the 3D viewer
        lWafer3DWizard->refreshViewer();

        //lWafer3DWizard->ForceAttachWindow(false);
    }

    // Show value/die on parametric wafermap
    if (pActionResult == pActionShowParametricDie)
    {
        if (mChartsInfo->chartsList().isEmpty() == false)
        {
            // Get Parameter
            CTest* ptTestCell = NULL;
            pChart = mChartsInfo->chartsList().first();
            pGroup =
                    gexReport->getGroupsList().isEmpty() ? NULL :
                                                           gexReport->getGroupsList().first();
            pFile =
                    (pGroup->pFilesList.isEmpty()) ? NULL :
                                                     (pGroup->pFilesList.first());

            if (pFile->FindTestCell(pChart->iTestNumberX, pChart->iPinMapX,
                                    &ptTestCell, false, false,
                                    pChart->strTestNameX) != 1)
            {
                return;
            }

            // Launch 3D window...unless already detached (not a child window hidden)
            pGexMainWindow->ShowWizardDialog(GEX_DRILL_3D_WIZARD_P1);

            Gex::DrillDataMining3D*  lWafer3DWizard = pGexMainWindow->LastCreatedWizardWafer3D();
            // Get die location
            if (pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST,
                                    &ptCellDieX, true, false) != 1)
            {
                return;
            }
            if (pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST,
                                    &ptCellDieY, true, false) != 1)
            {
                return;
            }

            // isValidIndex already tested in this function
            lDieX =
                    ptCellDieX->m_testResult.isValidIndex(nCurrentRow) ?
                        (int) ptCellDieX->m_testResult.resultAt(nCurrentRow) :
                        (int) GEX_C_DOUBLE_NAN;
            lDieY =
                    ptCellDieY->m_testResult.isValidIndex(nCurrentRow) ?
                        (int) ptCellDieY->m_testResult.resultAt(nCurrentRow) :
                        (int) GEX_C_DOUBLE_NAN;

            // Make it display the parametric wafermap
            lWafer3DWizard->ShowChart(0, 0,pChart->iTestNumberX,pChart->iPinMapX, GEX_DRILL_WAFERPROBE);

            // Select relevant die
            lWafer3DWizard->SelectDieLocation(pFile, lDieX, lDieY, true);

            // redraw the 3D viewer
            lWafer3DWizard->refreshViewer();

            //lWafer3DWizard->ForceAttachWindow(false);
        }
    }
}

/******************************************************************************!
 * \fn OnShowPartOnHistogram
 * \brief Show selected Part/Value on histogram
 ******************************************************************************/
void GexWizardChart::OnShowPartOnHistogram(int row, int  /*col*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("On Show Part On Histogram %1").arg( row).toLatin1().constData());
    // Get Test handle for cell selected
    int  iCol;
    bool bShowPartID =
            (ReportOptions.GetOption("dataprocessing",
                                     "part_id").toString() == "show") ? true : false;
    CGexSingleChart* pChart = NULL;  // Handle to Parameter Layer info
    unsigned iLayerIndex    = 0;
    unsigned iTotalLayers   =
            mChartsInfo->chartsList().count();
    unsigned     uIndex;
    double       lfValue;
    TestMarker* ptMark;
    QString      ds =
            ReportOptions.GetOption("dataprocessing", "scaling").toString();


    // Remove all Part selection markers already assigned to these active layers
    for (uIndex = 0; uIndex < iTotalLayers; uIndex++)
    {
        // Get pointer to relevant group
        pChart = mChartsInfo->chartsList().at(uIndex);
        lTestNumber  = pChart->iTestNumberX;
        lPinmapIndex = pChart->iPinMapX;
        if ((pChart->iGroupX < 0) ||
                (pChart->iGroupX >= gexReport->getGroupsList().size()))
        {
            pGroup = NULL;
        }
        else
        {
            pGroup = gexReport->getGroupsList().at(pChart->iGroupX);
        }
        pFile =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                                                 (pGroup->pFilesList.first());

        if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false,
                                false) != 1)
        {
            return;
        }

        // Clear all markers of type 'Selection' for all tests in all groups
        gexReport->ClearCustomMarkers("Selection");
    }

    // create one marker per layer!
    for (iLayerIndex = 0; iLayerIndex < iTotalLayers; iLayerIndex++)
    {
        // Reset handles
        pChart = mChartsInfo->chartsList().at(
                    iLayerIndex);
        lTestNumber  = pChart->iTestNumberX;
        lPinmapIndex = pChart->iPinMapX;
        if ((pChart->iGroupX < 0) ||
                (pChart->iGroupX >= gexReport->getGroupsList().size()))
        {
            pGroup = NULL;
        }
        else
        {
            pGroup = gexReport->getGroupsList().at(pChart->iGroupX);
        }
        pFile =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                                                 (pGroup->pFilesList.first());

        if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false,
                                false) != 1)
        {
            return;
        }

        // Check if we have the PartID visible
        if (bShowPartID)
        {
            iCol = 1 + (2 * iLayerIndex);
        }
        else
        {
            iCol = iLayerIndex;
        }

        // Extract value from cell pointed
        QTableWidgetItem* poTempItem = tableWidget->item(row, iCol);
        if (! poTempItem)
        {
            continue;
        }
        QString strString = " " + poTempItem->text().trimmed();

        sscanf(strString.toLatin1().constData(), "%lf", &lfValue);
        // Normalize value
        if (ds != "normalized")
        {
            lfValue *= ScalingPower(ptTestCell->res_scal);  // normalized
        }
        // Create marker or this test
        ptMark = new TestMarker();
        ptMark->strLabel = "Selection";
        ptMark->iLayer   = iLayerIndex; // Marker sticky to a specific layer
        ptMark->lfPos    = lfValue;
        ptMark->iLine    = 2;
        //ptMark->cColor = QColor(0,0,255);
        ptMark->bForceColor = false;  // Keep same color as layer color

        ptTestCell->ptCustomMarkers.append(ptMark);
    }

    // Ensure charting is over test results or adaptive so marker is visible!
    if (comboBoxDataWindow->currentIndex() == 0)
    {
        // Select last option in drop-down: 'adaptive mode')
        comboBoxDataWindow->setCurrentIndex(comboBoxDataWindow->count() - 1);
        OnChangeDatasetWindow("");
    }

    // Force to switch back to the Charting display
    tabWidgetChart->setCurrentIndex(0);
}

/******************************************************************************!
 * \fn onDeviceResults
 ******************************************************************************/
void GexWizardChart::onDeviceResults(long lDeviceID, int x, int y)
{
    // Force back to have Interactive Table as window with focus
    pGexMainWindow->ShowWizardDialog(GEX_TABLE_WIZARD_P1);

    // Ensure Table tab is fully loaded
    pGexMainWindow->LastCreatedWizardTable()->ShowTable("drill_table=parts_results");

    pGexMainWindow->LastCreatedWizardTable()->OnShowDeviceResults(lDeviceID, x, y);

    // Force back to have Interactive Chart as window with focus
    // pGexMainWindow->ShowWizardDialog(GEX_CHART_WIZARD_P1);
}

/******************************************************************************!
 * \fn onEditChartStyles
 ******************************************************************************/
void GexWizardChart::onEditChartStyles()
{
    if (frameParameters->isHidden())
        OnShowHideParam();

    // Make 'Chart style' widget visible
    tabWidgetStyle->setCurrentIndex(2);  // Tab: 'Style'
    tabWidgetStyles->setCurrentIndex(0);  // sub-tab: 'Drawing mode'
}

/******************************************************************************!
 * \fn onEditChartMarkers
 ******************************************************************************/
void GexWizardChart::onEditChartMarkers()
{
    if (frameParameters->isHidden())
        OnShowHideParam();

    // Make 'Chart markers' widget visible
    tabWidgetStyle->setCurrentIndex(2);  // Tab: 'Style'
    tabWidgetStyles->setCurrentIndex(1);  // sub-tab: 'Markers'
}

/******************************************************************************!
 * \fn onSelectWafermapRange
 ******************************************************************************/
void GexWizardChart::onSelectWafermapRange(double dLowLimit, double dHighLimit)
{
    SelectWaferMapDiesForParameterRange(dLowLimit, dHighLimit);
}

/******************************************************************************!
 * \fn ReloadWizardHandler
 ******************************************************************************/
void GexWizardChart::ReloadWizardHandler(bool removeAssociatedParts)
{
    if(removeAssociatedParts)
        mWizardHandler->ReloadWizards();
    else
        mWizardHandler->ReloadWizardsDisplayingTest(ptTestCell->GetTestNumber(), ptTestCell->GetTestName(), ptTestCell->GetPinIndex());

}

/******************************************************************************!
 * \fn CopyDefaultStyle
 * \brief Get default style from the 'Options', copy into layers
 ******************************************************************************/
void GexWizardChart::RetrieveGUISettings()
{
    tmpReportOptions->bPlotLegend   = checkBoxLayerName->isChecked();  // Write layer's name on chart
    // qqline
    tmpReportOptions->mPlotQQLine   = checkBoxQQLine->isChecked();
    // tmpReportOptions->mTextRotation = (checkBoxTextRotation->isChecked())?45:0;
    //tmpReportOptions->mTotalBars    = spinBoxHistBars->value();

    mChartsInfo->mTotalBars       = spinBoxHistBars->value();
    mChartsInfo->mBackgroundColor = pushButtonChartBackgroundColor->activeColor();
    mChartsInfo->mYScale          = comboBoxScaleType->currentIndex();
    mChartsInfo->mCustom          = customPropGroupBox->isChecked();
    mChartsInfo->mCustomTotalBars = customSpinBoxHistBars->value();

    // Read the "Lines" Tab page options
    //    mChartsInfo->mBoxBars           = checkBoxBars->isChecked();    // Bars check-box
    //    mChartsInfo->mBox3DBars         = checkBox3DBars->isChecked();    // 3D-Bars check-box
    //    mChartsInfo->mIsStacked         = checkBoxStack->isChecked();
    //    // Fitting curve check-box
    //    mChartsInfo->mFittingCurve      = checkBoxfittingCurve->isChecked();
    //    // Guaussian bell-curve shape
    //    mChartsInfo->mBellCurve         = checkBoxBellCurve->isChecked();
    //    mChartsInfo->mLines             = checkBoxLines->isChecked();    // Lines check-box
    //    mChartsInfo->mSpots             = checkBoxspots->isChecked();    // Spots check-box
    //    mChartsInfo->mLineWidth         = spinBoxLineWidth->value();    // Get line width
    //    // Get line style: slid, dashed, etc.
    //    mChartsInfo->mLineStyle         = comboBoxLineStyle->currentIndex();
    //    mChartsInfo->mWhiskerMode       = comboBoxWhiskerType->currentIndex();
    //    mChartsInfo->mTextRotation      = (checkBoxTextRotation->isChecked()?45:0);
    //    mChartsInfo->mQQLine            = checkBoxQQLine->isChecked();



    //    // These settings are only allowed to be changed
    //    //  one Chart layer at a time!
    //    mChartsInfo->mColor = pushButtonLineColor->activeColor();     // Line color
    //        //pChart->mBackgroundColor = pushButtonChartBackgroundColor->activeColor();

    //    // Get spot style: circle, rectangle, diamond, etc.
    //    mChartsInfo->mSpotStyle = comboBoxLineSpotStyle->currentIndex();


    //    // Update the "Markers" Tab page options
    //    // Mean check-box
    //    mChartsInfo->mMeanLineWidth         =   ((checkBoxMean->isChecked()) ? 1 : 0);

    //    // Min check-box
    //    mChartsInfo->mMinLineWidth          =   ((checkBoxMinimum->isChecked()) ? 1 : 0);

    //    // Max check-box
    //    mChartsInfo->mMaxLineWidth          =   ((checkBoxMaximum->isChecked()) ? 1 : 0);

    //    // Median check-box
    //    mChartsInfo->mMedianLineWidth       =   ((checkBoxMedian->isChecked()) ? 1 : 0);

    //    // Test limits check-box
    //    mChartsInfo->mLimitsLineWidth       =   ((checkBoxLimits->isChecked()) ? 1 : 0);

    //    mChartsInfo->mSpecLimitsLineWidth   =  ((checkBoxSpecLimits->isChecked()) ? 1 :0);
    //   // pChart->mLayerName = ((checkBoxLayerName->isChecked()) ? 1 :0);

    //    // +/-1sigma check-box
    //    mChartsInfo->mSigma2LineWidth   =    ((checkBox1Sigma->isChecked()) ? 1 : 0);

    // +/-1.5sigma check-box
    //    mChartsInfo->mSigma3LineWidth = ((checkBox15Sigma->isChecked()) ? 1 : 0);

    //    // +/-3sigma check-box
    //    mChartsInfo->mSigma3LineWidth =  ((checkBox3Sigma->isChecked()) ? 1 : 0);

    //    // +/-6sigma check-box
    //    mChartsInfo->mSigma6LineWidth = ((checkBox6Sigma->isChecked()) ? 1 : 0);

    //    // Test Rolmling limits check-box
    //    mChartsInfo->mRollingLimitsLineWidth = ((checkBoxSubsetLimits->isChecked()) ? 1 : 0);

    //    mChartsInfo->mQuartileQ1LineWidth = ((checkBoxQuartileQ1->isChecked()) ? 1 : 0);
    //    mChartsInfo->mQuartileQ3LineWidth = ((checkBoxQuartileQ3->isChecked()) ? 1 : 0);
    //mChartsInfo->mTextRotation = ((checkBoxTextRotation->isChecked()) ? 45 : 0);




    /*mScaling                    = reportOptions->GetOption("dataprocessing", "scaling").toString();
    mOptionStorageDevice        = reportOptions->GetOption("adv_histogram", "chart_type").toString();

    mAdvHistogramMarkerOptions  = reportOptions->GetOption(QString("adv_histogram"), QString("marker")).toString();
    mAdvHistogramFieldOptions   = reportOptions->GetOption(QString("adv_histogram"),QString("field")).toString();

    mHistoMarkerMin             = reportOptions->bHistoMarkerMin;
    mHistoMarkerMax             = reportOptions->bHistoMarkerMax;*/


    /*CGexSingleChart* pChart;
    for (int i = 0; i < mChartsInfo->chartsList().count(); ++i)
    {
        // Get structure to Parameter#X
        pChart = mChartsInfo->chartsList().at(i);

        // Read the "Lines" Tab page options
        pChart->bBoxBars   = checkBoxBars->isChecked();    // Bars check-box
        pChart->bBox3DBars = checkBox3DBars->isChecked();    // 3D-Bars check-box
        pChart->mIsStacked = checkBoxStack->isChecked();
        // Fitting curve check-box
        pChart->bFittingCurve = checkBoxfittingCurve->isChecked();
        // Guaussian bell-curve shape
        pChart->bBellCurve = checkBoxBellCurve->isChecked();
        pChart->bLines     = checkBoxLines->isChecked();    // Lines check-box
        pChart->bSpots     = checkBoxspots->isChecked();    // Spots check-box
        pChart->iLineWidth = spinBoxLineWidth->value();    // Get line width
        // Get line style: slid, dashed, etc.
        pChart->iLineStyle      = comboBoxLineStyle->currentIndex();
        pChart->iWhiskerMode    = comboBoxWhiskerType->currentIndex();
        pChart->setTextRotation( checkBoxTextRotation->isChecked()?45:0);
        pChart->mQQLine         = checkBoxQQLine->isChecked();

        if (mChartsInfo->chartsList().count() == 1)
        {
            // These settings are only allowed to be changed
            //  one Chart layer at a time!
            pChart->cColor = pushButtonLineColor->activeColor();     // Line color
            //pChart->mBackgroundColor = pushButtonChartBackgroundColor->activeColor();

            // Get spot style: circle, rectangle, diamond, etc.
            pChart->iSpotStyle = comboBoxLineSpotStyle->currentIndex();
        }

        // Update the "Markers" Tab page options
        // Mean check-box
        pChart->setMeanLineWidth((checkBoxMean->isChecked()) ? 1 : 0);

        // Min check-box
        pChart->setMinLineWidth((checkBoxMinimum->isChecked()) ? 1 : 0);

        // Max check-box
        pChart->setMaxLineWidth((checkBoxMaximum->isChecked()) ? 1 : 0);

        // Median check-box
        pChart->setMedianLineWidth((checkBoxMedian->isChecked()) ? 1 : 0);

        // Test limits check-box
        pChart->setLimitsLineWidth((checkBoxLimits->isChecked()) ? 1 : 0);

        pChart->setSpecLimitsLineWidth((checkBoxSpecLimits->isChecked()) ? 1 :0);
       // pChart->mLayerName = ((checkBoxLayerName->isChecked()) ? 1 :0);

        // +/-1sigma check-box
        pChart->set2SigmaLineWidth((checkBox1Sigma->isChecked()) ? 1 : 0);

        // +/-1.5sigma check-box
        pChart->set3SigmaLineWidth((checkBox15Sigma->isChecked()) ? 1 : 0);

        // +/-3sigma check-box
        pChart->set6SigmaLineWidth((checkBox3Sigma->isChecked()) ? 1 : 0);

        // +/-6sigma check-box
        pChart->set12SigmaLineWidth((checkBox6Sigma->isChecked()) ? 1 : 0);

        // Test Rolmling limits check-box
        pChart->setRollingLimitsLineWidth((checkBoxSubsetLimits->isChecked()) ? 1 : 0);

        pChart->setQuartileQ1((checkBoxQuartileQ1->isChecked()) ? 1 : 0);
        pChart->setQuartileQ3((checkBoxQuartileQ3->isChecked()) ? 1 : 0);
        pChart->setTextRotation((checkBoxTextRotation->isChecked()) ? 45 : 0);

    }*/
}

void GexWizardChart::SaveSettingsAsDefault()
{
    bool lOk;
    GS::Gex::Message::request("", "Save those settings as the default ?", lOk);
    if (! lOk)
    {
        return;
    }

        RetrieveGUISettings();

        tmpReportOptions->mTotalBars         = mChartsInfo->mTotalBars;  // Write layer's name on chart
        tmpReportOptions->mYScale            = mChartsInfo->mYScale;
        tmpReportOptions->mCustom            = mChartsInfo->mCustom;
        tmpReportOptions->mTotalBarsCustom   = mChartsInfo->mCustomTotalBars;


        ReportOptions = *(tmpReportOptions);

        QList<CGexSingleChart*>::const_iterator lIterBegin(mChartsInfo->chartsList().begin()), lIterEnd(mChartsInfo->chartsList().end());
        int lIndex = 0;
        for(;lIterBegin != lIterEnd; ++lIterBegin, ++lIndex)
        {
            if(ReportOptions.pLayersStyleList.count() <= lIndex)
                ReportOptions.pLayersStyleList.push_back(new CGexSingleChart(*(mChartsInfo->chartsList()[lIndex] )) );
            else
                *(ReportOptions.pLayersStyleList[lIndex]) = *(mChartsInfo->chartsList()[lIndex]);
        }

        pGexMainWindow->OnSaveSettingsCharts();
}


void GexWizardChart::ApplySettingsToAllOtherCharts()
{
    bool lOk;
    GS::Gex::Message::request("", "Apply settings to the other charts ?\nAll the others settings will be lost", lOk);
    if (! lOk)
    {
        return;
    }

        RetrieveGUISettings();

        QList<GexWizardChart* > lListOfCharts = mWizardHandler->ChartWizards();
        QList<GexWizardChart* >::iterator lIterBegin(lListOfCharts.begin()), lIterEnd(lListOfCharts.end());

        for(;lIterBegin != lIterEnd; ++lIterBegin)
        {
            if( (*lIterBegin) != this)
                (*lIterBegin)->ApplyNewSettings(mChartsInfo );
        }
}

void GexWizardChart::ApplyNewSettings( const CGexChartOverlays *chartsInfo)
{
    /*  bool lWasFalse = false;
    if(m_bExternalAdmin == false)
    {
        m_bExternalAdmin = true;
        lWasFalse = true;
    }*/
    mChartsInfo->CopySettings(chartsInfo);

    // -- must be done before
    // -- in OnChangeStyle, call to CopyDefaultStyle that init chartInfo from GUI
    // -- so the GUI must be set first
    UpdateGUIGlobal ( mChartsInfo);

    UpdateGUIByChart(mChartsInfo->chartsList()[0]);

    /* QList<CGexSingleChart*> lListChart = mChartsInfo->chartsList();
    QList<CGexSingleChart*>::const_iterator lIterBegin(lListChart.begin()), lIterEnd(lListChart.end());
    for(; lIterBegin != lIterEnd; ++lIterBegin)
    {
        UpdateGUIByChart(*lIterBegin);
    }*/

    OnChangeStyle();
    OnTabChange();
    UpdateChartsList();
    //  if(lWasFalse)
    //    m_bExternalAdmin = false;
}


/******************************************************************************!
 * \fn onRemoveDatapointRange
 ******************************************************************************/
void
GexWizardChart::onRemoveDatapointRange(double dLowLimit,
                                       double dHighLimit,
                                       bool   bRemoveAssociatedParts)
{
    QVariant varOptionsCleanSamples  = ReportOptions.GetOption("dataprocessing", "clean_samples");
    bool bOptionsCleanSamples    = (varOptionsCleanSamples.isValid() ? varOptionsCleanSamples.toBool() : false);
    bool bEnableUndoRedo = (bOptionsCleanSamples == false);

    // Ensure we use latest options set
    gexReport->m_cStats.UpdateOptions(gexReport->getReportOptions());
    QList<TestRemoveResultCommand::TestRemovedResult *> oRunIdxRemoved;
    // Remove range of data
    gexReport->
            RemoveDataPoints(mChartsInfo, NULL, NULL,
                             dLowLimit,
                             dHighLimit,
                             GEX_REMOVE_DATA_RANGE, true, bRemoveAssociatedParts, &oRunIdxRemoved);
    if(bEnableUndoRedo && pGexMainWindow && pGexMainWindow->getUndoStack() && !oRunIdxRemoved.isEmpty()){
        pGexMainWindow->getUndoStack()->push(new TestRemoveResultCommand(QString("Remove values in range [%1, %2]").arg(dLowLimit).arg(dHighLimit), oRunIdxRemoved, 0));
    }
    if(!bEnableUndoRedo){
        qDeleteAll(oRunIdxRemoved);
        oRunIdxRemoved.clear();
    }

    // Force to reload Statistics table with updated counts
    ReloadWizardHandler(bRemoveAssociatedParts);

    // Force to refresh 'Statistics' HTML page
    OnChangeChartLayer("");

    // Force Redraw (clear zoom level)
    goHome();
}

/******************************************************************************!
 * \fn onRemoveDatapointHigher
 ******************************************************************************/
void
GexWizardChart::onRemoveDatapointHigher(double dLimit,
                                        bool   bRemoveAssociatedParts)
{

    QVariant varOptionsCleanSamples  = ReportOptions.GetOption("dataprocessing", "clean_samples");
    bool bOptionsCleanSamples    = (varOptionsCleanSamples.isValid() ? varOptionsCleanSamples.toBool() : false);
    bool bEnableUndoRedo = (bOptionsCleanSamples == false);

    // Ensure we use latest options set
    gexReport->m_cStats.UpdateOptions(gexReport->getReportOptions());
    QList<TestRemoveResultCommand::TestRemovedResult *> oRunIdxRemoved;

    // Remove range of data
    gexReport->
            RemoveDataPoints(mChartsInfo, NULL, NULL,
                             0.0,
                             dLimit,
                             GEX_REMOVE_HIGHER_THAN, true, bRemoveAssociatedParts, &oRunIdxRemoved);
    if(bEnableUndoRedo && pGexMainWindow && pGexMainWindow->getUndoStack() && !oRunIdxRemoved.isEmpty()){
        pGexMainWindow->getUndoStack()->push(new TestRemoveResultCommand(QString("Remove values higher then \"%1\" ").arg(dLimit),oRunIdxRemoved, 0));
    }

    if(!bEnableUndoRedo){
        qDeleteAll(oRunIdxRemoved);
        oRunIdxRemoved.clear();
    }
    // Force to reload Statistics table with updated counts
    ReloadWizardHandler(bRemoveAssociatedParts);

    // Force to refresh 'Statistics' HTML page
    OnChangeChartLayer("");

    // Force Redraw (clear zoom level)
    goHome();
}

void
GexWizardChart::updateOFRManagerView()
{
    GS::Gex::OFR_Controller* lInstanceOFR = GS::Gex::OFR_Controller::GetInstance();
    if(lInstanceOFR && lInstanceOFR->IsReportEmpty() == false)
    {
        lInstanceOFR->RefreshView();
    }
}

/******************************************************************************!
 * \fn onRemoveDatapointLower
 ******************************************************************************/
void
GexWizardChart::onRemoveDatapointLower(double dLimit,
                                       bool   bRemoveAssociatedParts)
{
    QVariant varOptionsCleanSamples  = ReportOptions.GetOption("dataprocessing", "clean_samples");
    bool bOptionsCleanSamples    = (varOptionsCleanSamples.isValid() ? varOptionsCleanSamples.toBool() : false);
    bool bEnableUndoRedo = (bOptionsCleanSamples == false);
    // Ensure we use latest options set
    gexReport->m_cStats.UpdateOptions(gexReport->getReportOptions());
    QList<TestRemoveResultCommand::TestRemovedResult *> oRunIdxRemoved;

    // Remove range of data
    gexReport->
            RemoveDataPoints(mChartsInfo, NULL, NULL,
                             dLimit,
                             0.0,
                             GEX_REMOVE_LOWER_THAN, true, bRemoveAssociatedParts, &oRunIdxRemoved);
    if(bEnableUndoRedo && pGexMainWindow && pGexMainWindow->getUndoStack() && !oRunIdxRemoved.isEmpty()){
        pGexMainWindow->getUndoStack()->push(new TestRemoveResultCommand( QString("Remove values lower then \"%1\" ").arg(dLimit),oRunIdxRemoved, 0));
    }
    if(!bEnableUndoRedo){
        qDeleteAll(oRunIdxRemoved);
        oRunIdxRemoved.clear();
    }
    // Force to reload Statistics table with updated counts
    ReloadWizardHandler(bRemoveAssociatedParts);

    // Force to refresh 'Statistics' HTML page
    OnChangeChartLayer("");

    // Force Redraw (clear zoom level)
    goHome();
}

/******************************************************************************!
 * \fn onRemoveIQR
 ******************************************************************************/
void GexWizardChart::onRemoveIQR(double dValue, bool bRemoveAssociatedParts)
{
    QVariant varOptionsCleanSamples  = ReportOptions.GetOption("dataprocessing", "clean_samples");
    bool bOptionsCleanSamples    = (varOptionsCleanSamples.isValid() ? varOptionsCleanSamples.toBool() : false);
    bool bEnableUndoRedo = (bOptionsCleanSamples == false);
    CTest* ptLayerTestCell    = NULL;
    CGexSingleChart*  pChart  = NULL;
    CGexGroupOfFiles* ptGroup = NULL;
    CGexFileInGroup*  ptFile  = NULL;
    int     lTestNumber  = 0;
    int     lPinmapIndex = 0;
    double  dLowOutlierLimit;
    double  dHighOutlierLimit;
    //QString ds = ReportOptions.GetOption("dataprocessing", "scaling").toString();
    QList<TestRemoveResultCommand::TestRemovedResult *> oRunIdxRemoved;

    // Ensure we use latest options set
    gexReport->m_cStats.UpdateOptions(gexReport->getReportOptions());

    // Compute IQR removal for each layer
    for (int iLayerIndex = 0;
         iLayerIndex < mChartsInfo->chartsList().count();
         iLayerIndex++)
    {
        // Get handle to layer
        pChart = mChartsInfo->chartsList().at(
                    iLayerIndex);
        lTestNumber     = pChart->iTestNumberX;
        lPinmapIndex    = pChart->iPinMapX;

        // Get pointer to relevant group
        if ((pChart->iGroupX < 0) ||
                (pChart->iGroupX >= gexReport->getGroupsList().size()))
        {
            ptGroup = NULL;
        }
        else
        {
            ptGroup = gexReport->getGroupsList().at(pChart->iGroupX);
        }
        ptFile =
                (ptGroup->pFilesList.isEmpty()) ? NULL :
                                                  (ptGroup->pFilesList.first());

        if (ptFile->FindTestCell(lTestNumber, lPinmapIndex, &ptLayerTestCell, false,
                                 false, pChart->strTestNameX) == 1)
        {
            dLowOutlierLimit =
                    (ptLayerTestCell->lfSamplesQuartile1 - dValue *
                     (ptLayerTestCell->lfSamplesQuartile3 -
                      ptLayerTestCell->lfSamplesQuartile1));  // /lfExponent;
            dHighOutlierLimit =
                    (ptLayerTestCell->lfSamplesQuartile3 + dValue *
                     (ptLayerTestCell->lfSamplesQuartile3 -
                      ptLayerTestCell->lfSamplesQuartile1));  // /lfExponent;

            // Remove outside IQR data space
            gexReport->
                    RemoveDataPoints(NULL, ptGroup, ptLayerTestCell,
                                     dLowOutlierLimit,
                                     0.0,
                                     GEX_REMOVE_LOWER_THAN, true, bRemoveAssociatedParts, &oRunIdxRemoved);
            gexReport->
                    RemoveDataPoints(NULL, ptGroup, ptLayerTestCell,
                                     0.0,
                                     dHighOutlierLimit,
                                     GEX_REMOVE_HIGHER_THAN, true, bRemoveAssociatedParts, &oRunIdxRemoved);
            if(bEnableUndoRedo && pGexMainWindow && pGexMainWindow->getUndoStack() && !oRunIdxRemoved.isEmpty()){
                pGexMainWindow->getUndoStack()->push(new TestRemoveResultCommand(QString ("Applying IQR outlier removal with N=%1" ).arg(dValue), oRunIdxRemoved, 0));
            }
            if(!bEnableUndoRedo){
                qDeleteAll(oRunIdxRemoved);
                oRunIdxRemoved.clear();
            }

        }
    }

    // Force to reload Statistics table with updated counts
    ReloadWizardHandler(bRemoveAssociatedParts);

    // Force to refresh 'Statistics' HTML page
    OnChangeChartLayer("");

    // Force Redraw (clear zoom level)
    goHome();
}

/******************************************************************************!
 * \fn onRemoveNSigma
 ******************************************************************************/
void GexWizardChart::onRemoveNSigma(double dValue, bool bRemoveAssociatedParts)
{
    QVariant varOptionsCleanSamples  = ReportOptions.GetOption("dataprocessing", "clean_samples");
    bool bOptionsCleanSamples    = (varOptionsCleanSamples.isValid() ? varOptionsCleanSamples.toBool() : false);
    bool bEnableUndoRedo = (bOptionsCleanSamples == false);

    CTest* ptLayerTestCell    = NULL;
    CGexSingleChart*  pChart  = NULL;
    CGexGroupOfFiles* ptGroup = NULL;
    CGexFileInGroup*  ptFile  = NULL;
    int     lTestNumber   = 0;
    int     lPinmapIndex  = 0;
    int     iOutlierCount = 0;
    double  dLowOutlierLimit;
    double  dHighOutlierLimit;
    // QString ds = ReportOptions.GetOption("dataprocessing", "scaling").toString();
    QList<TestRemoveResultCommand::TestRemovedResult *> oRunIdxRemoved;

    // Remove N*Sigma on each layer
    for (int iLayerIndex = 0;
         iLayerIndex < mChartsInfo->chartsList().count();
         iLayerIndex++)
    {
        // Get handle to layer
        pChart = mChartsInfo->chartsList().at(iLayerIndex);
        lTestNumber     = pChart->iTestNumberX;
        lPinmapIndex    = pChart->iPinMapX;

        // Get pointer to relevant group
        if ((pChart->iGroupX < 0) ||
                (pChart->iGroupX >= gexReport->getGroupsList().size()))
        {
            ptGroup = NULL;
        }
        else
        {
            ptGroup = gexReport->getGroupsList().at(pChart->iGroupX);
        }
        ptFile =
                (ptGroup->pFilesList.isEmpty()) ? NULL :
                                                  (ptGroup->pFilesList.first());

        if (ptFile->FindTestCell(lTestNumber, lPinmapIndex, &ptLayerTestCell, false,
                                 false, pChart->strTestNameX) == 1)
        {
            // Recursive process
            do
            {
                // Clear outlier counter
                iOutlierCount = 0;

                // Update Outlier limits
                ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier = ptLayerTestCell->lfMean +
                        (dValue * ptLayerTestCell->lfSigma);  // Mean + N*Sigma/2;
                ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier = ptLayerTestCell->lfMean -
                        (dValue * ptLayerTestCell->lfSigma);  // Mean - N*Sigma/2;

                // Outlier limits scaled to result format
                dHighOutlierLimit = ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier;  // Mean+N*Sigma/2;
                dLowOutlierLimit  = ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier;  // Mean-N*Sigma/2;

                iOutlierCount += gexReport->
                        RemoveDataPoints(NULL, ptGroup, ptLayerTestCell,
                                         dLowOutlierLimit,
                                         0.0,
                                         GEX_REMOVE_LOWER_THAN, true,
                                         bRemoveAssociatedParts, &oRunIdxRemoved);
                iOutlierCount += gexReport->
                        RemoveDataPoints(NULL, ptGroup, ptLayerTestCell,
                                         0.0,
                                         dHighOutlierLimit,
                                         GEX_REMOVE_HIGHER_THAN, true,
                                         bRemoveAssociatedParts, &oRunIdxRemoved);
            }
            while (iOutlierCount);
            if(bEnableUndoRedo && pGexMainWindow && pGexMainWindow->getUndoStack() && !oRunIdxRemoved.isEmpty()){
                pGexMainWindow->getUndoStack()->push(new TestRemoveResultCommand(QString ("Applying N*Sigma outlier removal with N=%1" ).arg(dValue), oRunIdxRemoved, 0));
            }
            if(!bEnableUndoRedo){
                qDeleteAll(oRunIdxRemoved);
                oRunIdxRemoved.clear();
            }
        }
    }

    // Force to reload Statistics table with updated counts
    ReloadWizardHandler(bRemoveAssociatedParts);

    // Force to refresh 'Statistics' HTML page
    OnChangeChartLayer("");

    // Force Redraw (clear zoom level)
    goHome();
}

/******************************************************************************!
 * \fn onSelectMulti
 ******************************************************************************/
void GexWizardChart::onSelectMulti()
{
    // Set selection mode on chartdirector object
    m_pMultiChartWidget->setMouseMode(GexSingleChartWidget::mouseSelect);

    toolButtonMouseMode_2->setIcon(mIconsMouse[E_ICON_SELECT]);
    // Untoggle other selection mode buttons
    //buttonSelect->setChecked(true);
    //buttonZoomMulti->setChecked(false);
    //buttonMoveMulti->setChecked(false);
}

/******************************************************************************!
 * \fn onMoveMulti
 ******************************************************************************/
void GexWizardChart::onMoveMulti()
{
    // Set selection mode on chartdirector object
    m_pMultiChartWidget->setMouseMode(GexSingleChartWidget::mouseDrag);

    toolButtonMouseMode_2->setIcon(mIconsMouse[E_ICON_MOVE]);
    // Untoggle other selection mode buttons
    //    buttonMoveMulti->setChecked(true);
    //    buttonSelectMulti->setChecked(false);
    //    buttonZoomMulti->setChecked(false);
}

/******************************************************************************!
 * \fn onZoomMulti
 ******************************************************************************/
void GexWizardChart::onZoomMulti()
{
    // Set selection mode on chartdirector object
    m_pMultiChartWidget->setMouseMode(GexSingleChartWidget::mouseZoom);

    toolButtonMouseMode_2->setIcon(mIconsMouse[E_ICON_ZOOM]);
    // Untoggle other selection mode buttons
    //    buttonZoomMulti->setChecked(true);
    //    buttonSelectMulti->setChecked(false);
    //    buttonMoveMulti->setChecked(false);
}

/******************************************************************************!
 * \fn onHomeMulti
 ******************************************************************************/
void GexWizardChart::onHomeMulti()
{
    goHomeMulti(true);
}

/******************************************************************************!
 * \fn onHome
 ******************************************************************************/
void GexWizardChart::onHome()
{
    goHome(true);
}

void GexWizardChart::initTestCell(unsigned int lTestNumber, QString lTestName, int lPinmapIndex)
{
    CGexGroupOfFiles * lGrpOfFile = *gexReport->getGroupsList().begin();



    ptTestCell = ptTestCellY =  lGrpOfFile->FindTestCell(lTestNumber, lTestName, lPinmapIndex);
}


void GexWizardChart::toJson(QJsonObject& descriptionOut, const QString& chartTypeName)
{
    QJsonObject lSetting;
    //-- Create test infos
    CTest* lTestCell = currentTestCellX();

    Test lTest;
    lTest.mNumber       = lTestCell->GetTestNumber();
    lTest.mName         = lTestCell->GetTestName();
    lTest.mPinIndex      = lTestCell->GetPinIndex();
    lTest.mGroupId      = pGroup->GetGroupId();
    lTest.mFileIndex    = 0;

    // find the index of the file using mWizardParent->pFile
    lSetting.insert("Test", lTest.toJson());

    RetrieveGUISettings();
    QJsonObject lChartOverlayJson;
    mChartsInfo->ToJson(lChartOverlayJson, m_eChartType);

//    QString lName = mChartsInfo->mTitle;
//    if( lName.isEmpty())
//        lName = lTestCell->GetTestNumber();

//    lSetting.insert("Title", lName);
    lSetting.insert("ChartOverlays", lChartOverlayJson);

    switch(m_eChartType)
    {
        case GexAbstractChart::chartTypeHistogram       : lSetting.insert("ViewportMode", m_iHistoViewport);	break;
        case GexAbstractChart::chartTypeTrend           : lSetting.insert("ViewportMode", m_iTrendViewport);	break;
        case GexAbstractChart::chartTypeProbabilityPlot : lSetting.insert("ViewportMode", m_iProbabilityViewport);	break;
        case GexAbstractChart::chartTypeScatter         : lSetting.insert("ViewportMode", m_iScatterViewport);	break;
        case GexAbstractChart::chartTypeBoxPlot         : lSetting.insert("ViewportMode", m_iBoxPlotViewport);	break;
        default: lSetting.insert("ViewportMode", 0);
    }

    //-- export the setting into the builder element
    descriptionOut.insert(chartTypeName, lSetting);
}

void GexWizardChart::AddToLastUpdatedOFRSection( const QString& chartTypeName)
{
    QJsonObject lElt;
    toJson(lElt,  chartTypeName);
    GS::Gex::OFR_Controller::GetInstance()->AddElementToSection(
                GS::Gex::OFR_Controller::GetInstance()->GetLastIndexSectionUpdated(), lElt);
}

void GexWizardChart::AddToAnExistingOFRSection(int indexSection, const QString& chartTypeName)
{
    QJsonObject lElt;
    toJson(lElt,  chartTypeName);
    GS::Gex::OFR_Controller::GetInstance()->AddElementToSection(indexSection, lElt);
}

void GexWizardChart::AddNewOFRSection(const QString& chartTypeName)
{
    QJsonObject lElt;
    toJson(lElt,  chartTypeName);
    GS::Gex::OFR_Controller::GetInstance()->AddElementSettings(lElt);
}

/******************************************************************************!
 * \fn findFirstValidTestCell
 ******************************************************************************/
CTest* GexWizardChart::findFirstValidTestCell()
{
    bool bValidNextTest   = false;
    bool showGalaxyTest   = false;

    if ((ReportOptions.GetOption("statistics", "generic_galaxy_tests")).toString() != "hide")
        showGalaxyTest = true;

    CTest * lTestCell   = pFile->ptTestList;
    CTest * pTmpTest    = NULL;

    // Iterate on next test until new test has valid data...
    while (lTestCell && bValidNextTest == false)
    {
        foreach(CGexGroupOfFiles * gof, gexReport->getGroupsList())
        {
            pTmpTest = gof->FindTestCell(lTestCell->lTestNumber,
                                         lTestCell->strTestName,
                                         lTestCell->lPinmapIndex);

            if (pTmpTest)
            {
                if ((pTmpTest->ldExecs == 0 && pTmpTest->GetCurrentLimitItem()->ldOutliers == 0) ||
                        pTmpTest->bTestType == 'F' || pTmpTest->lResultArraySize > 0 ||
                        gexReport->isInteractiveTestFiltered(pTmpTest) == false)
                {
                    continue;
                }
                else
                    // Ignore Generic Galaxy Parameters
                    if ((pTmpTest->bTestType == '-') && (!showGalaxyTest))
                    {
                        continue;
                    }
                    else
                    {
                        bValidNextTest = true;
                        break;
                    }
            }
        }

        if (bValidNextTest == false)
            lTestCell = lTestCell->GetNextTest();
    }

    return lTestCell;
}

/******************************************************************************!
 * \fn findFirstValidTestCellY
 ******************************************************************************/
CTest* GexWizardChart::findFirstValidTestCellY()
{
    bool bValidNextTest   = false;
    bool showGalaxyTest   = false;

    if ((ReportOptions.GetOption("statistics", "generic_galaxy_tests")).toString() != "hide")
        showGalaxyTest = true;

    CTest * lTestCell   = pFileY->ptTestList;
    CTest * pTmpTest    = NULL;

    // Iterate on next test until new test has valid data...
    while (lTestCell && bValidNextTest == false)
    {
        foreach(CGexGroupOfFiles * gof, gexReport->getGroupsList())
        {
            pTmpTest = gof->FindTestCell(lTestCell->lTestNumber,
                                         lTestCell->strTestName,
                                         lTestCell->lPinmapIndex);

            if (pTmpTest)
            {
                if ((pTmpTest->ldExecs == 0 && pTmpTest->GetCurrentLimitItem()->ldOutliers == 0) ||
                        pTmpTest->bTestType == 'F' || pTmpTest->lResultArraySize > 0 ||
                        gexReport->isInteractiveTestFiltered(pTmpTest) == false)
                {
                    continue;
                }
                else
                    // Ignore Generic Galaxy Parameters
                    if ((pTmpTest->bTestType == '-') && (!showGalaxyTest))
                    {
                        continue;
                    }
                    else
                    {
                        bValidNextTest = true;
                        break;
                    }
            }
        }

        if (bValidNextTest == false)
            lTestCell = lTestCell->GetNextTest();
    }

    return lTestCell;
}

CTest *GexWizardChart::findLastValidTestCell()
{
    bool showGalaxyTest   = false;

    if ((ReportOptions.GetOption("statistics", "generic_galaxy_tests")).toString() != "hide")
        showGalaxyTest = true;

    CTest * lTestCell   = pFile->ptTestList;
    CTest * pTmpTest    = NULL;
    CTest * lLastTest   = NULL;

    // Iterate on test to find last valid test cell...
    while (lTestCell)
    {
        foreach(CGexGroupOfFiles * gof, gexReport->getGroupsList())
        {
            pTmpTest = gof->FindTestCell(lTestCell->lTestNumber, lTestCell->strTestName,
                                         lTestCell->lPinmapIndex);

            if (pTmpTest)
            {
                if ((pTmpTest->ldExecs == 0 && pTmpTest->GetCurrentLimitItem()->ldOutliers == 0) ||
                        pTmpTest->bTestType == 'F' || pTmpTest->lResultArraySize > 0 ||
                        gexReport->isInteractiveTestFiltered(pTmpTest) == false)
                {
                    continue;
                }
                // Ignore Generic Galaxy Parameters
                else if ((pTmpTest->bTestType == '-') && (!showGalaxyTest))
                {
                    continue;
                }
                else
                {
                    lLastTest = lTestCell;
                }
            }
        }

        lTestCell = lTestCell->GetNextTest();
    }

    return lLastTest;
}

CTest *GexWizardChart::findLastValidTestCellY()
{
    bool showGalaxyTest   = false;

    if ((ReportOptions.GetOption("statistics", "generic_galaxy_tests")).toString() != "hide")
        showGalaxyTest = true;

    CTest * lTestCell   = pFileY->ptTestList;
    CTest * pTmpTest    = NULL;
    CTest * lLastTest   = NULL;

    // Iterate on test to find last valid test cell...
    while (lTestCell)
    {
        foreach(CGexGroupOfFiles * gof, gexReport->getGroupsList())
        {
            pTmpTest = gof->FindTestCell(lTestCell->lTestNumber, lTestCell->strTestName,
                                         lTestCell->lPinmapIndex);

            if (pTmpTest)
            {
                if ((pTmpTest->ldExecs == 0 && pTmpTest->GetCurrentLimitItem()->ldOutliers == 0) ||
                        pTmpTest->bTestType == 'F' || pTmpTest->lResultArraySize > 0 ||
                        gexReport->isInteractiveTestFiltered(pTmpTest) == false)
                {
                    continue;
                }
                // Ignore Generic Galaxy Parameters
                else if ((pTmpTest->bTestType == '-') && (!showGalaxyTest))
                {
                    continue;
                }
                else
                {
                    lLastTest = lTestCell;
                }
            }
        }

        lTestCell = lTestCell->GetNextTest();
    }

    return lLastTest;
}

bool GexWizardChart::IsFirstTestCell(const CTest *lTest)
{
    return (lTest == findFirstValidTestCell());
}

bool GexWizardChart::IsFirstTestCellY(const CTest *lTest)
{
    return (lTest == findFirstValidTestCellY());
}

bool GexWizardChart::IsLastTestCell(const CTest *lTest)
{
    return (lTest == findLastValidTestCell());
}

bool GexWizardChart::IsLastTestCellY(const CTest *lTest)
{
    return (lTest == findLastValidTestCellY());
}

/******************************************************************************!
 * \fn onTestChanged
 ******************************************************************************/
void GexWizardChart::onTestChanged()
{
    // Make this chart replace the current overlays
    addChart(true, true, ptTestCell, ptTestCellY);
    if (ptTestCell->m_bUseCustomBarNumber
            /*|| ptTestCellY->m_bUseCustomBarNumber*/)
    {
        customSpinBoxHistBars->setValue(ptTestCell->m_iHistoClasses);
        customPropGroupBox->setChecked(true);
    }
    else if (! mMultipleTest)
    {
        customSpinBoxHistBars->setValue(ptTestCell->m_iHistoClasses);
        customPropGroupBox->setChecked(false);
    }
    if (mMultipleTest)
    {
        customPropGroupBox->hide();
        //resetAllButton->hide();
    }
    // Reset Zoom/Drag flags
    mChartsInfo->lfZoomFactorX   =
            mChartsInfo->lfZoomFactorY = -1.0;
    m_pSingleChartWidget->resetViewPortManager();
    m_pMultiChartWidget->resetViewPortManager();
    // Force Redraw : Chart or Table, (depending of current tab selected)
    OnTabChange();

    // Select 1st parameter in combo-box list
    comboBoxChartLayer->setCurrentIndex(0);

    // Update the 'Style' tab GUI content to reflect parameters options
    OnChangeChartLayer("");
}

/******************************************************************************!
 * \fn onRebuildReport
 ******************************************************************************/
void GexWizardChart::onRebuildReport(const QString& format)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString(" '%1' ").arg( format).toLatin1().constData());
    // Select report format to create...
    ReportOptions.SetOption("output", "format", format);

    ReportOptions.SetOption(QString("report"), QString("build"), QString("true"));

    // When initial report was a CSL for Report Builder replay, remove the template file name for
    // Recreating the report from Interactive views
    if (ReportOptions.getAdvancedReport() == GEX_ADV_REPORT_BUILDER)
        ReportOptions.strTemplateFile.clear();

    // Build report now!
    gexReport->RebuildReport();

    // Display report page if format is HTML
    if (format.toUpper() == "HTML")
    {
        pGexMainWindow->ReportLink();
    }
}

/******************************************************************************!
 * \fn goHome
 ******************************************************************************/
void GexWizardChart::goHome(bool bEraseCustomViewport/* = false*/)
{
    // Reset zoom & drag offsets
    mChartsInfo->lfZoomFactorX   =
            mChartsInfo->lfZoomFactorY = -1.0;
    m_pSingleChartWidget->resetViewPortManager(bEraseCustomViewport);

    // Force Redraw
    OnTabChange();
}


/******************************************************************************!
 * \fn goHomeMulti
 ******************************************************************************/
void GexWizardChart::goHomeMulti(bool bEraseCustomViewport)
{
    // Reset zoom & drag offsets
    m_pMultiChartWidget->resetViewPortManager(bEraseCustomViewport);

    // Force Redraw
    OnTabChange();
}

/******************************************************************************!
 * \fn resetBarsToDefault
 ******************************************************************************/
void GexWizardChart::resetBarsToDefault(bool)
{
    CTest* poTempTest = findFirstValidTestCell();
    while (poTempTest)
    {
        poTempTest->m_bUseCustomBarNumber = false;
        poTempTest->m_iHistoClasses       = 40;
        poTempTest                        = poTempTest->GetNextTest();
    }

    poTempTest = findFirstValidTestCellY();
    while (poTempTest)
    {
        poTempTest->m_bUseCustomBarNumber = false;
        poTempTest->m_iHistoClasses       = 40;
        poTempTest                        = poTempTest->GetNextTest();
    }
    customPropGroupBox->setChecked(false);
    customSpinBoxHistBars->setValue(40);
}

/******************************************************************************!
 * \fn useCustomHistoBars
 ******************************************************************************/
void GexWizardChart::useCustomHistoBars(bool bVal)
{
    ptTestCell->m_bUseCustomBarNumber = bVal;
    ptTestCell->m_iHistoClasses       = customSpinBoxHistBars->value();

    mChartsInfo->mCustom = bVal;
    OnHistoBarSize(bVal ?
                       customSpinBoxHistBars->value() : spinBoxHistBars->value());




    /*ptTestCellY->m_bUseCustomBarNumber = bVal;
     ptTestCellY->m_iHistoClasses = customSpinBoxHistBars->value();
     if (bVal)
     {
     tmpReportOptions->
      SetOption("adv_histogram","num_bars_modifier",  "custom" );
     }
     else
     {
     tmpReportOptions->
      SetOption("adv_histogram","num_bars_modifier",  "global" );
     }*/
}

/******************************************************************************!
 * \fn resetButtonStatus
 ******************************************************************************/
void GexWizardChart::resetButtonStatus() {
    // buttonSelect->setChecked(true);
    //buttonZoom->setChecked(false);
    //buttonMove->setChecked(false);

    toolButtonMouseMode->setIcon(mIconsMouse[E_ICON_SELECT]);

    mSelectZone->
            setEnabled(gexReport->getGroupsList().count() == 1 &&
                       (m_eChartType != GexAbstractChart::chartTypeScatter &&
            m_eChartType != GexAbstractChart::chartTypeBoxWhisker));
}

/******************************************************************************!
 * \fn onRemoveDatapointScatter
 ******************************************************************************/
void
GexWizardChart::
onRemoveDatapointScatter(const QList<QRectF>& rAreaToRemove,
                         bool                 bRemoveAssociatedParts)
{
    // Ensure we use latest options set
    gexReport->m_cStats.UpdateOptions(gexReport->getReportOptions());

    gexReport->
            removeDataPointsScatter(rAreaToRemove, true, bRemoveAssociatedParts);

    // Force to reload Statistics table with updated counts
    if (pGexMainWindow->LastCreatedWizardTable() != NULL)
    {
        pGexMainWindow->LastCreatedWizardTable()->clear();
        pGexMainWindow->LastCreatedWizardTable()->ShowTable("");
    }

    // Force to refresh 'Statistics' HTML page
    OnChangeChartLayer("");

    // Force Redraw (clear zoom level)
    goHome();
    if (bRemoveAssociatedParts)
    {
        ReportOptions.
                SetOption(QString("report"), QString("build"), QString("true"));
        // Build report now!
        gexReport->RebuildReport();
        if (ReportOptions.
                GetOption("output", "format").toString().toUpper() == "HTML")
        {
            pGexMainWindow->ReportLink();
        }
    }
}

/******************************************************************************!
 * \fn showPATLimitGroupBox
 ******************************************************************************/
void
GexWizardChart::showPATLimitGroupBox(CTest* poTest,
                                     const QList<TestMarker*>&
                                     ptCustomMarkers)
{
    if (! m_poLastTest)
    {
        m_poLastTest = poTest;
    }
    else if (poTest->lTestNumber != m_poLastTest->lTestNumber)
    {
        //m_poLastTest->m_oPatMarkerEnable.clear();
        //m_poLastTest->m_oPatMarkerEnable = m_oPatMarkerEnable;
        m_poLastTest                     = poTest;
    }
    else
    {
        return;
    }

    m_poPATLimitGroupBox->hide();

    TestMarker* poMarker = 0;
    bool    bNewEntry     = false;
    QString strLabel;
    int     iLine = 0;
    int     iCol  = 0;
    if (! m_oPatMarkerMap.isEmpty())
    {
        qDeleteAll(m_oPatMarkerMap.values());
    }
    m_oPatMarkerMap.clear();
    m_oPatMarkerEnable.clear();

    if (! m_poLastTest->m_oPatMarkerEnable.isEmpty())
    {
        m_oPatMarkerEnable = m_poLastTest->m_oPatMarkerEnable;
    }

    if (! m_poLastTest->ptCustomMarkers.count())
    {
        return;
    }

    for (int iIdx = 0; iIdx < ptCustomMarkers.count(); iIdx++)
    {
        poMarker  = ptCustomMarkers.at(iIdx);
        bNewEntry = false;
        strLabel  = "";
        // Org. Limits
        if ((poMarker->strLabel == "Org. LL." ||
             poMarker->strLabel == "Org. HL.") &&
                ! m_oPatMarkerMap.contains("Org. Limits"))
        {
            bNewEntry = true;
            strLabel  = "Org. Limits";
        }
        else if ((poMarker->strLabel.contains("CpkL=") ||
                  poMarker->strLabel.contains("CpkH=")) &&
                 ! m_oPatMarkerMap.contains("Cpk Limits"))
        {
            bNewEntry = true;
            strLabel  = "Cpk Limits";
        }
        else if ((poMarker->strLabel == "-SPAT" ||
                  poMarker->strLabel == "+SPAT") &&
                 ! m_oPatMarkerMap.contains("SPAT Limits"))
        {
            bNewEntry = true;
            strLabel  = "SPAT Limits";
        }
        else if ((poMarker->strLabel == "+N" ||
                  poMarker->strLabel == "-N") &&
                 ! m_oPatMarkerMap.contains("PAT Limits N"))
        {
            bNewEntry = true;
            strLabel  = "PAT Limits N";
        }
        else if ((poMarker->strLabel == "+M" ||
                  poMarker->strLabel == "-M") &&
                 ! m_oPatMarkerMap.contains("PAT Limits M"))
        {
            bNewEntry = true;
            strLabel  = "PAT Limits M";
        }
        else if ((poMarker->strLabel == "+F" ||
                  poMarker->strLabel == "-F") &&
                 ! m_oPatMarkerMap.contains("PAT Limits F"))
        {
            bNewEntry = true;
            strLabel  = "PAT Limits F";
        }
        else if (poMarker->strLabel.endsWith(" relaxed") &&
                 ! m_oPatMarkerMap.contains("PAT Limits relaxed"))
        {
            bNewEntry = true;
            strLabel  = "PAT Limits relaxed";
        }

        if (bNewEntry)
        {
            QCheckBox* poCheckBox = new QCheckBox(strLabel, m_poPATLimitGroupBox);
            m_poPATLimitGridLayout->addWidget(poCheckBox, iLine, iCol, 1, 1);
            iCol   = (iCol + 1) % 2;
            iLine += ! iCol ? 1 : 0;
            m_oPatMarkerMap.insert(strLabel, poCheckBox);
            if (! m_oPatMarkerEnable.contains(strLabel))
            {
                m_oPatMarkerEnable.insert(strLabel, true);
                poCheckBox->setChecked(true);
            }
            else
            {
                poCheckBox->setChecked(m_oPatMarkerEnable[strLabel]);
            }
            if (! connect(poCheckBox, SIGNAL(clicked()), SLOT(showHidePatMarker())))
            {
                GSLOG(SYSLOG_SEV_WARNING, "connect poCheckBox fail");
            }
        }
    }

    m_poPATLimitGroupBox->show();

}

void GexWizardChart::onMlCheckboxChaged()
{
    // Update single and multi chart widgets
    m_pSingleChartWidget->update();
    m_pMultiChartWidget->update();
}

/******************************************************************************!
 * \fn showHidePatMarker
 ******************************************************************************/
void GexWizardChart::showHidePatMarker()
{
    QCheckBox* poCheckBox = qobject_cast<QCheckBox*>(sender());
    if (! poCheckBox)
    {
        return;
    }
    m_oPatMarkerEnable[poCheckBox->text()] = poCheckBox->isChecked();
    m_pSingleChartWidget->update();

}

/******************************************************************************!
 * \fn patMarkerIsEnabled
 ******************************************************************************/
bool GexWizardChart::patMarkerIsEnabled(const QString& strMarker)
{
    if ((strMarker == "Org. LL." ||
         strMarker == "Org. HL.") &&
            m_oPatMarkerMap.contains("Org. Limits"))
    {
        return m_oPatMarkerEnable["Org. Limits"];
    }
    else if ((strMarker.contains("CpkL=") ||
              strMarker.contains("CpkH=")) &&
             m_oPatMarkerMap.contains("Cpk Limits"))
    {
        return m_oPatMarkerEnable["Cpk Limits"];
    }
    else if ((strMarker == "-SPAT" ||
              strMarker == "+SPAT") &&
             m_oPatMarkerMap.contains("SPAT Limits"))
    {
        return m_oPatMarkerEnable["SPAT Limits"];
    }
    else if ((strMarker == "+N" ||
              strMarker == "-N") &&
             m_oPatMarkerMap.contains("PAT Limits N"))
    {
        return m_oPatMarkerEnable["PAT Limits N"];
    }
    else if ((strMarker == "+M" ||
              strMarker == "-M") &&
             m_oPatMarkerMap.contains("PAT Limits M"))
    {
        return m_oPatMarkerEnable["PAT Limits M"];
    }
    else if ((strMarker == "+F" ||
              strMarker == "-F") &&
             m_oPatMarkerMap.contains("PAT Limits F"))
    {
        return m_oPatMarkerEnable["PAT Limits F"];
    }
    else if (strMarker.endsWith(" relaxed") &&
             m_oPatMarkerMap.contains("PAT Limits relaxed"))
    {
        return m_oPatMarkerEnable["PAT Limits relaxed"];

    }
    return true;
}

void GexWizardChart::ShowHideRollingLimitsMarker()
{
    bool                lDisplay    = false;

    // Draw subset Limits check box only trend type
    if (GexAbstractChart::chartTypeTrend == m_eChartType
            && tmpReportOptions->GetOption("adv_trend", "x_axis").toString() == "run_id")
    {
        CGexGroupOfFiles *  lGroup      = NULL;
        CTest *             lTest       = NULL;

        for (int lIdx = 0; lIdx < gexReport->getGroupsList().count(); ++lIdx)
        {
            lGroup = gexReport->getGroupsList().at(lIdx);

            if (ptTestCell)
            {
                lTest = lGroup->FindTestCell(ptTestCell->lTestNumber, ptTestCell->strTestName, ptTestCell->lPinmapIndex);

                if (lTest && lTest->GetRollingLimits().HasSubsetLimit())
                {
                    lDisplay = true;
                    break;
                }
            }

            if (ptTestCellY)
            {
                lTest = lGroup->FindTestCell(ptTestCellY->lTestNumber, ptTestCellY->strTestName, ptTestCellY->lPinmapIndex);

                if (lTest && lTest->GetRollingLimits().HasSubsetLimit())
                {
                    lDisplay = true;
                    break;
                }
            }
        }
    }

    checkBoxSubsetLimits->setVisible(lDisplay);
}

/******************************************************************************!
 * \fn resetPatMarker
 ******************************************************************************/
void GexWizardChart::resetPatMarker()
{
    m_oPatMarkerEnable.clear();
    m_poLastTest = 0;
}

/******************************************************************************!
 * \fn updateChart
 ******************************************************************************/
void GexWizardChart::updateChart(){

    // Force to reload Statistics table with updated counts
    if (pGexMainWindow->LastCreatedWizardTable() != NULL)
    {
        pGexMainWindow->LastCreatedWizardTable()->clear();
        pGexMainWindow->LastCreatedWizardTable()->ShowTable("");
    }

    // Force to refresh 'Statistics' HTML page
    OnChangeChartLayer("");

    // Force Redraw (clear zoom level)
    goHome();
}

void GexWizardChart::InitToolsButtons()
{
    // -- Tests filter, sort...
    mPickTestAction = new QAction(QPixmap(QString::fromUtf8(":/gex/icons/combo.png")), "Pick parameters to plot", this );
    connect(mPickTestAction, SIGNAL(triggered()), this, SLOT(OnPickTest()));

    mPickTestYAction = new QAction(QPixmap(QString::fromUtf8(":/gex/icons/combo.png")), "Pick parameters to plot on Y axis", this );
    connect(mPickTestYAction, SIGNAL(triggered()), this, SLOT(OnPickTestY()));

    mFilterTestAction = new QAction(QPixmap(QString::fromUtf8(":/gex/icons/filter.png")), "Define which parameters to focus on!", this );
    mFilterTestAction->setCheckable(true);
    connect(mFilterTestAction, SIGNAL(triggered()), this, SLOT(OnFilterParameters()));

    mFilterOderIDAction = new QAction("Sorted by FlowID", this );
    mFilterOderIDAction->setCheckable(true);
    connect(mFilterOderIDAction, SIGNAL(triggered()), this, SLOT(OnChangeTestSortOrder()));

    toolButtonTests->addAction(mPickTestAction);
    toolButtonTests->addAction(mPickTestYAction);
    toolButtonTests->addAction(mFilterTestAction);
    toolButtonTests->addAction(mFilterOderIDAction);

    toolButtonTests_2->addAction(mPickTestAction);
    toolButtonTests_2->addAction(mPickTestYAction);
    toolButtonTests_2->addAction(mFilterTestAction);
    toolButtonTests_2->addAction(mFilterOderIDAction);

    toolButtonTests_3->addAction(mPickTestAction);
    toolButtonTests_3->addAction(mPickTestYAction);
    toolButtonTests_3->addAction(mFilterTestAction);
    toolButtonTests_3->addAction(mFilterOderIDAction);

    // -- viewport singlechart
    mResetViewport.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/home.png")));
    mKeepViewport.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/anchor.png")));

    QAction* lHomeAction = new QAction(mResetViewport, "Reset viewport", this );
    mAnchorAction = new QAction(mKeepViewport, "Keep viewport", this );
    mAnchorAction->setCheckable(true);


    connect(lHomeAction, SIGNAL(triggered()), this, SLOT(onHome()));

    connect(mAnchorAction, SIGNAL(triggered(bool)), m_pSingleChartWidget, SLOT(onKeepViewportChanged(bool)));
    connect(mAnchorAction, SIGNAL(triggered(bool)), m_pMultiChartWidget, SLOT(onKeepViewportChanged(bool)));

    toolButtonViewPort->addAction(lHomeAction);
    toolButtonViewPort->addAction(mAnchorAction);

    // -- viewport multichart
    lHomeAction = new QAction(mResetViewport, "Reset viewport", this );
    connect(lHomeAction, SIGNAL(triggered()), this, SLOT(onHomeMulti()));
    toolButtonViewPort_2->addAction(lHomeAction);

    //-- interactive views
    InitInteractiveView(toolButtonInteractiveViews);
    InitInteractiveView(toolButtonInteractiveViews_2);
    InitInteractiveView(toolButtonInteractiveViews_3);

    // -- Mouse
    mSelectIcon.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/selectmode.png")));
    mZoomIcon.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/drill3d_zoom.png")));
    mMoveIcon.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/move.png")));
    mSplitDataIcon.addPixmap(QPixmap(QString::fromUtf8(":/gex/icons/select_area.png")));

    mIconsMouse.append(mSelectIcon);
    mIconsMouse.append(mMoveIcon);
    mIconsMouse.append(mZoomIcon);
    mIconsMouse.append(mSplitDataIcon);

    //--single chart
    QAction* lSelectAction =  new QAction(mSelectIcon ,"Select", this);
    QAction* lZoomAction  =  new QAction( mZoomIcon , "Zoom", this);
    QAction* lMoveAction  =  new QAction( mMoveIcon , "Drag", this);
    mSelectZone  =  new QAction( mSplitDataIcon , "Split data by selection", this);

    connect(lSelectAction, SIGNAL(triggered()), this, SLOT(OnSelect()));
    connect(lZoomAction, SIGNAL(triggered()), this, SLOT(OnZoom()));
    connect(mSelectZone, SIGNAL(triggered()), this, SLOT(onSelectZone()));
    connect(lMoveAction, SIGNAL(triggered()), this, SLOT(OnDrag()));

    toolButtonMouseMode->addAction(lSelectAction);
    toolButtonMouseMode->addAction(lMoveAction);
    toolButtonMouseMode->addAction(lZoomAction);
    toolButtonMouseMode->addAction(mSelectZone);


    //--Multi-chart
    lSelectAction =  new QAction(mSelectIcon ,"Select", this);
    lZoomAction  =  new QAction( mZoomIcon , "Zoom", this);
    lMoveAction  =  new QAction( mMoveIcon , "Drag", this);

    connect(lSelectAction, SIGNAL(triggered()), this, SLOT(onSelectMulti()));
    connect(lZoomAction, SIGNAL(triggered()), this, SLOT(onZoomMulti()));
    connect(lMoveAction, SIGNAL(triggered()), this, SLOT(onMoveMulti()));

    toolButtonMouseMode_2->addAction(lSelectAction);
    toolButtonMouseMode_2->addAction(lMoveAction);
    toolButtonMouseMode_2->addAction(lZoomAction);

    //--Settings
    InitSettingsButton(toolButtonSettingsMenu);
    InitSettingsButton(toolButtonSettingsMenu_2);
    InitSettingsButton(toolButtonSettingsMenu_3);

}


void GexWizardChart::InitInteractiveView(QToolButton* button)
{
    QPixmap  lTable(50, 50);
    lTable.load(QString::fromUtf8(":/gex/icons/options_statistics.png"));

    QPixmap lWafermap(50, 50);
    lWafermap.load(QString::fromUtf8(":/gex/icons/options_wafermap.png"));


    QPixmap lHisto(50, 50);
    lHisto.load(QString::fromUtf8(":/gex/icons/options_advhisto.png"));

    QAction* lSwitchWafermap   = new QAction(lWafermap, "Switch to Wafermap", this );

    QString lOPenWafer("Open a Wafermap");
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        lOPenWafer.append("\t(Ctrl+W)");

    QString lOPenTable("Open a Table");
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        lOPenTable.append("\t(Ctrl+T)");

    QString lOPenDiagram("Open a Diagram");
    lOPenDiagram.append("\t(Ctrl+D)");

    QAction* lDisplayWafermap   = new QAction(lWafermap,    lOPenWafer, this );
    QAction* lDisplayTable      = new QAction(lTable,       lOPenTable, this );
    QAction* lDisplayHisto      = new QAction(lHisto,       lOPenDiagram, this );

    connect(lSwitchWafermap,    SIGNAL(triggered()), this, SLOT(OnSwitchInteractiveWafermap()));

    connect(lDisplayTable,      SIGNAL(triggered()), this, SLOT(OnInteractiveTables()));
    connect(lDisplayWafermap,   SIGNAL(triggered()), this, SLOT(OnInteractiveWafermap()));
    connect(lDisplayHisto,      SIGNAL(triggered()), this, SLOT(OnInteractiveCharts()));

    QMenu* lMenu= new QMenu(button);

    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
    {
        lMenu->addAction(lSwitchWafermap);
        lMenu->addSeparator();
    }

    lMenu->addAction(lDisplayWafermap);
    lMenu->addAction(lDisplayTable);
    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminator() == false)
        lMenu->addAction(lDisplayHisto);

    button->setMenu(lMenu);
}

void GexWizardChart::InitSettingsButton(QToolButton* button)
{
    //--Settings
    QPixmap  lSave(50, 50);
    lSave.load(QString::fromUtf8(":/gex/icons/save_settings.png"));

    QPixmap lApply(50, 50);
    lApply.load(QString::fromUtf8(":/gex/icons/arrow-scatter.png"));

    QPixmap lShowHide(60,60);
    lShowHide.load(QString::fromUtf8(":/gex/icons/new_parameters.png"));


    QAction* lShowHideSettings = new QAction(lShowHide, "Show/hide settings", this );
    QAction* lApplySettings = new QAction(lApply, "Apply settings to other windows", this );
    QAction* lSaveSettings = new QAction(lSave, "Save settings as the default", this );

    connect(lShowHideSettings, SIGNAL(triggered()), this, SLOT(OnShowHideParam()));
    connect(lApplySettings, SIGNAL(triggered()), this, SLOT(ApplySettingsToAllOtherCharts()));
    connect(lSaveSettings, SIGNAL(triggered()), this, SLOT(SaveSettingsAsDefault()));

    button->addAction(lShowHideSettings);
    button->addAction(lApplySettings);
    button->addAction(lSaveSettings);
    button->setArrowType(Qt::NoArrow);
}

void GexWizardChart::focusInEvent( QFocusEvent* )
{
     mWizardHandler->SetLastChartViewed(this);
}
