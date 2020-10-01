///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexmultichartwidget.h"
#include "drill_chart.h"
#include "gex_constants.h"

///////////////////////////////////////////////////////////////////////////////////
// Class GexMultiChartWidget - class which display many single chart
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexMultiChartWidget::GexMultiChartWidget(QWidget * pParent /* = NULL */) : QWidget(pParent), mWizardParent(0)
{
    m_pHistoChart			= new GexSingleChartWidget(this, false);
    m_pTrendChart			= new GexSingleChartWidget(this, false);
    m_pProbabilityPlotChart	= new GexSingleChartWidget(this, false);
    m_pBoxPlotChart			= new GexSingleChartWidget(this, false);

    m_pGridLayout			= new QGridLayout(this);

    m_pGridLayout->addWidget(m_pHistoChart,				0, 0);
    m_pGridLayout->addWidget(m_pTrendChart,				0, 1);
    m_pGridLayout->addWidget(m_pProbabilityPlotChart,	1, 1);
    m_pGridLayout->addWidget(m_pBoxPlotChart,			1, 0);

    // reset chart
    //resetChart(false);

    // Connect single chart widget
    connect(m_pHistoChart,	SIGNAL(currentDieSelected()),							this,	SIGNAL(currentDieSelected()));
    connect(m_pHistoChart,	SIGNAL(deviceResults(long)),							this,	SIGNAL(deviceResults(long)));
    connect(m_pHistoChart,	SIGNAL(editStyles()),									this,	SIGNAL(editStyles()));
    connect(m_pHistoChart,	SIGNAL(editMarkers()),									this,	SIGNAL(editMarkers()));
    connect(m_pHistoChart,	SIGNAL(selectWafermapRange(double, double)),			this,	SIGNAL(selectWafermapRange(double, double)));
    connect(m_pHistoChart,	SIGNAL(removeDatapointRange(double, double, bool)),		this,	SIGNAL(removeDatapointRange(double, double, bool)));
    connect(m_pHistoChart,	SIGNAL(removeDatapointHigher(double, bool)),			this,	SIGNAL(removeDatapointHigher(double, bool)));
    connect(m_pHistoChart,	SIGNAL(removeDatapointLower(double, bool)),				this,	SIGNAL(removeDatapointLower(double, bool)));
    connect(m_pHistoChart,	SIGNAL(removeIQR(double, bool)),						this,	SIGNAL(removeIQR(double, bool)));
    connect(m_pHistoChart,	SIGNAL(removeNSigma(double, bool)),						this,	SIGNAL(removeNSigma(double, bool)));
    connect(m_pHistoChart,	SIGNAL(rebuildReport(const QString&)),					this,	SIGNAL(rebuildReport(const QString&)));
    //connect(m_pHistoChart,	SIGNAL(viewportChanged(const GexSingleChartWidget * )),	this,	SLOT(replotChart(const GexSingleChartWidget * )));

    connect(m_pTrendChart,	SIGNAL(currentDieSelected()),							this,	SIGNAL(currentDieSelected()));
    connect(m_pTrendChart,	SIGNAL(deviceResults(long)),							this,	SIGNAL(deviceResults(long)));
    connect(m_pTrendChart,	SIGNAL(editStyles()),									this,	SIGNAL(editStyles()));
    connect(m_pTrendChart,	SIGNAL(editMarkers()),									this,	SIGNAL(editMarkers()));
    connect(m_pTrendChart,	SIGNAL(selectWafermapRange(double, double)),			this,	SIGNAL(selectWafermapRange(double, double)));
    connect(m_pTrendChart,	SIGNAL(removeDatapointRange(double, double, bool)),		this,	SIGNAL(removeDatapointRange(double, double, bool)));
    connect(m_pTrendChart,	SIGNAL(removeDatapointHigher(double, bool)),			this,	SIGNAL(removeDatapointHigher(double, bool)));
    connect(m_pTrendChart,	SIGNAL(removeDatapointLower(double, bool)),				this,	SIGNAL(removeDatapointLower(double, bool)));
    connect(m_pTrendChart,	SIGNAL(removeIQR(double, bool)),						this,	SIGNAL(removeIQR(double, bool)));
    connect(m_pTrendChart,	SIGNAL(removeNSigma(double, bool)),						this,	SIGNAL(removeNSigma(double, bool)));
    connect(m_pTrendChart,	SIGNAL(rebuildReport(const QString&)),					this,	SIGNAL(rebuildReport(const QString&)));
    //connect(m_pTrendChart,	SIGNAL(viewportChanged(const GexSingleChartWidget * )),	this,	SLOT(replotChart(const GexSingleChartWidget * )));

    connect(m_pBoxPlotChart,	SIGNAL(currentDieSelected()),							this,	SIGNAL(currentDieSelected()));
    connect(m_pBoxPlotChart,	SIGNAL(deviceResults(long)),							this,	SIGNAL(deviceResults(long)));
    connect(m_pBoxPlotChart,	SIGNAL(editStyles()),									this,	SIGNAL(editStyles()));
    connect(m_pBoxPlotChart,	SIGNAL(editMarkers()),									this,	SIGNAL(editMarkers()));
    connect(m_pBoxPlotChart,	SIGNAL(selectWafermapRange(double, double)),			this,	SIGNAL(selectWafermapRange(double, double)));
    connect(m_pBoxPlotChart,	SIGNAL(removeDatapointRange(double, double, bool)),		this,	SIGNAL(removeDatapointRange(double, double, bool)));
    connect(m_pBoxPlotChart,	SIGNAL(removeDatapointHigher(double, bool)),			this,	SIGNAL(removeDatapointHigher(double, bool)));
    connect(m_pBoxPlotChart,	SIGNAL(removeDatapointLower(double, bool)),				this,	SIGNAL(removeDatapointLower(double, bool)));
    connect(m_pBoxPlotChart,	SIGNAL(removeIQR(double, bool)),						this,	SIGNAL(removeIQR(double, bool)));
    connect(m_pBoxPlotChart,	SIGNAL(removeNSigma(double, bool)),						this,	SIGNAL(removeNSigma(double, bool)));
    connect(m_pBoxPlotChart,	SIGNAL(rebuildReport(const QString&)),					this,	SIGNAL(rebuildReport(const QString&)));
    //connect(m_pBoxPlotChart,	SIGNAL(viewportChanged(const GexSingleChartWidget * )),	this,	SLOT(replotChart(const GexSingleChartWidget * )));

    connect(m_pProbabilityPlotChart,	SIGNAL(currentDieSelected()),							this,	SIGNAL(currentDieSelected()));
    connect(m_pProbabilityPlotChart,	SIGNAL(deviceResults(long)),							this,	SIGNAL(deviceResults(long)));
    connect(m_pProbabilityPlotChart,	SIGNAL(editStyles()),									this,	SIGNAL(editStyles()));
    connect(m_pProbabilityPlotChart,	SIGNAL(editMarkers()),									this,	SIGNAL(editMarkers()));
    connect(m_pProbabilityPlotChart,	SIGNAL(selectWafermapRange(double, double)),			this,	SIGNAL(selectWafermapRange(double, double)));
    connect(m_pProbabilityPlotChart,	SIGNAL(removeDatapointRange(double, double, bool)),		this,	SIGNAL(removeDatapointRange(double, double, bool)));
    connect(m_pProbabilityPlotChart,	SIGNAL(removeDatapointHigher(double, bool)),			this,	SIGNAL(removeDatapointHigher(double, bool)));
    connect(m_pProbabilityPlotChart,	SIGNAL(removeDatapointLower(double, bool)),				this,	SIGNAL(removeDatapointLower(double, bool)));
    connect(m_pProbabilityPlotChart,	SIGNAL(removeIQR(double, bool)),						this,	SIGNAL(removeIQR(double, bool)));
    connect(m_pProbabilityPlotChart,	SIGNAL(removeNSigma(double, bool)),						this,	SIGNAL(removeNSigma(double, bool)));
    connect(m_pProbabilityPlotChart,	SIGNAL(rebuildReport(const QString&)),					this,	SIGNAL(rebuildReport(const QString&)));
    //connect(m_pProbabilityPlotChart,	SIGNAL(viewportChanged(const GexSingleChartWidget * )),	this,	SLOT(replotChart(const GexSingleChartWidget * )));

    connect(this,	SIGNAL(keepViewportChanged(bool)),	m_pHistoChart,				SLOT(onKeepViewportChanged(bool)));
    connect(this,	SIGNAL(keepViewportChanged(bool)),	m_pTrendChart,				SLOT(onKeepViewportChanged(bool)));
    connect(this,	SIGNAL(keepViewportChanged(bool)),	m_pBoxPlotChart,			SLOT(onKeepViewportChanged(bool)));
    connect(this,	SIGNAL(keepViewportChanged(bool)),	m_pProbabilityPlotChart,	SLOT(onKeepViewportChanged(bool)));

    connect(m_pHistoChart,              SIGNAL(deselectHighlightedDie()),	this,	SIGNAL(deselectHighlightedDie()));
    connect(m_pTrendChart,              SIGNAL(deselectHighlightedDie()),	this,   SIGNAL(deselectHighlightedDie()));
    connect(m_pBoxPlotChart,            SIGNAL(deselectHighlightedDie()),	this,	SIGNAL(deselectHighlightedDie()));
    connect(m_pProbabilityPlotChart,	SIGNAL(deselectHighlightedDie()),	this,	SIGNAL(deselectHighlightedDie()));
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexMultiChartWidget::~GexMultiChartWidget()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void replotChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
// in comment because never called with an argument. Replace directly by the call of the update
void GexMultiChartWidget::replotChart( CReportOptions* lOption,  CTest* lTestCellX, CTest* lTestCellY, const GexSingleChartWidget * pSingleChartWidget)
{
    if (pSingleChartWidget != m_pHistoChart)
        m_pHistoChart->replotChart(lOption, lTestCellX, lTestCellY);

    if (pSingleChartWidget != m_pTrendChart)
        m_pTrendChart->replotChart(lOption, lTestCellX, lTestCellY);

    if (pSingleChartWidget != m_pBoxPlotChart)
        m_pBoxPlotChart->replotChart(lOption, lTestCellX, lTestCellY);

    if (pSingleChartWidget != m_pProbabilityPlotChart)
        m_pProbabilityPlotChart->replotChart(lOption, lTestCellX, lTestCellY);

    update();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setMouseMode(GexSingleChartWidget::MouseMode mouseMode)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexMultiChartWidget::setMouseMode(GexSingleChartWidget::MouseMode mouseMode)
{
    m_pHistoChart->setMouseMode(mouseMode);
    m_pTrendChart->setMouseMode(mouseMode);
    m_pBoxPlotChart->setMouseMode(mouseMode);
    m_pProbabilityPlotChart->setMouseMode(mouseMode);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setViewportMode(int nViewportMode)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexMultiChartWidget::setViewportMode(int nViewportMode)
{
    if (m_pHistoChart)
        m_pHistoChart->setViewportMode(nViewportMode);

    if (m_pTrendChart)
        m_pTrendChart->setViewportMode(nViewportMode);

    if (m_pProbabilityPlotChart)
        m_pProbabilityPlotChart->setViewportMode(nViewportMode);

    if (m_pBoxPlotChart)
        m_pBoxPlotChart->setViewportMode(nViewportMode);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void resetViewPortManager(bool bEraseCustomViewport)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexMultiChartWidget::resetViewPortManager(bool bEraseCustomViewport)
{
    m_pHistoChart->resetViewPortManager(bEraseCustomViewport);
    m_pTrendChart->resetViewPortManager(bEraseCustomViewport);
    m_pBoxPlotChart->resetViewPortManager(bEraseCustomViewport);
    m_pProbabilityPlotChart->resetViewPortManager(bEraseCustomViewport);
}

void    GexMultiChartWidget::init(GexWizardChart*   lWizardParent)
{
    if(lWizardParent)
    {
        mWizardParent = lWizardParent;
        resetChart(false);
        if (m_pHistoChart)
            m_pHistoChart->init(mWizardParent);
        if (m_pTrendChart)
            m_pTrendChart->init(mWizardParent);
        if (m_pProbabilityPlotChart)
            m_pProbabilityPlotChart->init(mWizardParent);
        if (m_pBoxPlotChart)
            m_pBoxPlotChart->init(mWizardParent);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void resetChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexMultiChartWidget::resetChart(bool bKeepViewport)
{
    // Set up histogram widget
    m_pHistoChart->setChartType(GexAbstractChart::chartTypeHistogram, mWizardParent->getChartInfo(), GexAbstractChart::viewportOverLimits, bKeepViewport);

    // Set up trend widget
    m_pTrendChart->setChartType(GexAbstractChart::chartTypeTrend, mWizardParent->getChartInfo(), GexAbstractChart::viewportOverLimits, bKeepViewport);

    // Set up Probability plot widget
    m_pProbabilityPlotChart->setChartType(GexAbstractChart::chartTypeProbabilityPlot,mWizardParent->getChartInfo(),  GexAbstractChart::viewportOverLimits, bKeepViewport);

    // Set up Box plot widget
    m_pBoxPlotChart->setChartType(GexAbstractChart::chartTypeBoxPlot, mWizardParent->getChartInfo(), GexAbstractChart::viewportOverLimits, bKeepViewport);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onChartOptionChanged(GexAbstractChart::chartOption eChartOption)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexMultiChartWidget::onChartOptionChanged(GexAbstractChart::chartOption eChartOption)
{
    m_pHistoChart->onChartOptionChanged(eChartOption);
    m_pTrendChart->onChartOptionChanged(eChartOption);
    m_pProbabilityPlotChart->onChartOptionChanged(eChartOption);
    m_pBoxPlotChart->onChartOptionChanged(eChartOption);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onKeepViewportChanged(bool isChecked)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexMultiChartWidget::onKeepViewportChanged(bool isChecked)
{
    emit keepViewportChanged(isChecked);
}
