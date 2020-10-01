///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "drill_chart_xbar_r_tabwidget.h"
#include "drill_chart.h"
#include "ui_drill_chart_xbar_r_tabwidget.h"
#include "gex_xbar_r_chart.h"
#include <gqtl_log.h>

///////////////////////////////////////////////////////////////////////////////////
// Defines
///////////////////////////////////////////////////////////////////////////////////

#define CONTROL_CHART_XBAR		0
#define CONTROL_CHART_R			1
#define CONTROL_CHART_XBAR_R	2

GexXBarRTabWidget::GexXBarRTabWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::drill_chart_xbar_r_tabwidget),
    mWizardParent(0)
{
    ui->setupUi(this);

	// Fill combo box for control chart
	ui->comboBoxTypeControlChart->clear();
	ui->comboBoxTypeControlChart->insertItem(CONTROL_CHART_XBAR,	"Xbar");
	ui->comboBoxTypeControlChart->insertItem(CONTROL_CHART_R ,		"R");
	ui->comboBoxTypeControlChart->insertItem(CONTROL_CHART_XBAR_R,	"Xbar-R");
    ui->comboBoxTypeControlChart->setCurrentIndex(CONTROL_CHART_XBAR_R);

	// Fill combo box for viewport
	ui->comboBoxViewport->clear();
	ui->comboBoxViewport->addItem("Chart over Test limits",				QVariant(GexAbstractControlChart::viewportOverLimits));
	ui->comboBoxViewport->addItem("Chart over Control limits",			QVariant(GexAbstractControlChart::viewportOverControlLimits));
	ui->comboBoxViewport->addItem("Chart over results",					QVariant(GexAbstractControlChart::viewportOverData));
	ui->comboBoxViewport->addItem("Adaptive: data & Test limits",		QVariant(GexAbstractControlChart::viewportAdaptive));
	ui->comboBoxViewport->addItem("Adaptive: data & Control limits",	QVariant(GexAbstractControlChart::viewportAdaptiveControl));
    ui->comboBoxViewport->setCurrentIndex(ui->comboBoxViewport->findData(QVariant(GexAbstractControlChart::viewportAdaptiveControl)));

	// Disable Auto repaint
	ui->XbarChartWidget->enableAutoRepaint(false);
	ui->RChartWidget->enableAutoRepaint(false);

	connect(ui->XbarChartWidget,			SIGNAL(viewportChanged(const GexSingleChartWidget * )),	this,	SLOT(replotChart(const GexSingleChartWidget * )));
	connect(ui->RChartWidget,				SIGNAL(viewportChanged(const GexSingleChartWidget * )),	this,	SLOT(replotChart(const GexSingleChartWidget * )));

	connect(ui->comboBoxTypeControlChart,	SIGNAL(currentIndexChanged(int)),						this,	SLOT(onControlChartChanged(int)));
	connect(ui->comboBoxViewport,			SIGNAL(currentIndexChanged(int)),						this,	SLOT(onViewportChanged(int)));
	connect(ui->buttonSelectXbarR,			SIGNAL(clicked()),										this,	SLOT(onSelect()));
	connect(ui->buttonMoveXbarR,			SIGNAL(clicked()),										this,	SLOT(onMove()));
	connect(ui->buttonZoomXbarR,			SIGNAL(clicked()),										this,	SLOT(onZoom()));
	connect(ui->buttonHomeXbarR,			SIGNAL(clicked()),										this,	SLOT(onHome()));
}



GexXBarRTabWidget::~GexXBarRTabWidget()
{
    delete ui;
}



void GexXBarRTabWidget::init(GexWizardChart*     lWizardParent)
{

    if(lWizardParent)
    {
        mWizardParent = lWizardParent;
        // Initialize chart type
        ui->XbarChartWidget->setChartType(GexAbstractChart::chartTypeXBar,	mWizardParent->getChartInfo(), GexAbstractControlChart::viewportAdaptiveControl, false);
        ui->RChartWidget->setChartType(GexAbstractChart::chartTypeRChart,	mWizardParent->getChartInfo(), GexAbstractControlChart::viewportAdaptiveControl, false);
    }
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onMouseModeChanged(GexSingleChartWidget::MouseMode)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::onMouseModeChanged(GexSingleChartWidget::MouseMode eMouseMode)
{
	// Set mouse mode on chartdirector object
	ui->XbarChartWidget->setMouseMode(eMouseMode);
	ui->RChartWidget->setMouseMode(eMouseMode);

	// toggle/untoggle selection mode buttons
	ui->buttonZoomXbarR->setChecked(eMouseMode == GexSingleChartWidget::mouseZoom);
	ui->buttonMoveXbarR->setChecked(eMouseMode == GexSingleChartWidget::mouseDrag);
	ui->buttonSelectXbarR->setChecked(eMouseMode == GexSingleChartWidget::mouseSelect);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onSelect()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::onSelect()
{
	// Set mouse mode on chartdirector object
	onMouseModeChanged(GexSingleChartWidget::mouseSelect);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onMove()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::onMove()
{
	// Set mouse mode on chartdirector object
	onMouseModeChanged(GexSingleChartWidget::mouseDrag);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onZoom()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::onZoom()
{
	// Set mouse mode on chartdirector object
	onMouseModeChanged(GexSingleChartWidget::mouseZoom);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onHome()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::onHome()
{
	home(true);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onControlChartChanged()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::onControlChartChanged(int nNewIUndex)
{
	switch(nNewIUndex)
	{
		case CONTROL_CHART_XBAR:
			ui->XbarChartWidget->show();
			ui->RChartWidget->hide();
			break;

		case CONTROL_CHART_R:
			ui->XbarChartWidget->hide();
			ui->RChartWidget->show();
			break;

		case CONTROL_CHART_XBAR_R:
			ui->XbarChartWidget->show();
			ui->RChartWidget->show();
			break;

		default:
			GEX_ASSERT(false);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onViewportChanged(int nNewIUndex)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::onViewportChanged(int nNewIUndex)
{
	int iViewport = ui->comboBoxViewport->itemData(nNewIUndex).toInt();

	ui->XbarChartWidget->setViewportMode(iViewport);
	ui->RChartWidget->setViewportMode(iViewport);

	// Reset Zoom/Drag flags + replot
	home();
}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void home(bool bEraseCustomViewport /* = false */)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::home(bool bEraseCustomViewport /* = false */)
{
	// Go back to select mode
	onSelect();

	// Reset viewport
	ui->XbarChartWidget->resetViewPortManager(bEraseCustomViewport);
	ui->RChartWidget->resetViewPortManager(bEraseCustomViewport);

	//	Update widget
    ui->XbarChartWidget->replotChart(mWizardParent->chartReportOptions(), mWizardParent->currentTestCellX(), mWizardParent->currentTestCellY());
    ui->RChartWidget->replotChart(mWizardParent->chartReportOptions(), mWizardParent->currentTestCellX(), mWizardParent->currentTestCellY());

	update();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void replotChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::replotChart(const GexSingleChartWidget * pSingleChartWidget)
{
	if (pSingleChartWidget != ui->XbarChartWidget)
        ui->XbarChartWidget->replotChart(mWizardParent->chartReportOptions(), mWizardParent->currentTestCellX(), mWizardParent->currentTestCellY());

	if (pSingleChartWidget != ui->RChartWidget)
        ui->RChartWidget->replotChart(mWizardParent->chartReportOptions(), mWizardParent->currentTestCellX(), mWizardParent->currentTestCellY());

	update();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setMouseMode(GexSingleChartWidget::MouseMode mouseMode)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::setMouseMode(GexSingleChartWidget::MouseMode mouseMode)
{
	ui->XbarChartWidget->setMouseMode(mouseMode);
	ui->RChartWidget->setMouseMode(mouseMode);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setViewportMode(int nViewportMode)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::setViewportMode(int nViewportMode)
{
	if (ui->XbarChartWidget)
		ui->XbarChartWidget->setViewportMode(nViewportMode);

	if (ui->RChartWidget)
		ui->RChartWidget->setViewportMode(nViewportMode);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void resetViewPortManager(bool bEraseCustomViewport)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRTabWidget::resetViewPortManager(bool bEraseCustomViewport)
{
	ui->XbarChartWidget->resetViewPortManager(bEraseCustomViewport);
	ui->RChartWidget->resetViewPortManager(bEraseCustomViewport);
}

