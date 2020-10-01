#ifndef GEX_XBAR_R_CHART_H
#define GEX_XBAR_R_CHART_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexabstractchart.h"
#include "gex_xbar_r_data.h"

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAbstractControlChart
//
// Description	:	chart class to draw the control charts into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////
class GexAbstractControlChart : public GexAbstractChart
{
public:

	enum ControlChartViewportMode
	{
		viewportOverControlLimits	= viewportUser + 1,
		viewportAdaptiveControl		= viewportUser + 2,
	};

    GexAbstractControlChart(GexAbstractChart::chartType eChartType, int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays);
	virtual ~GexAbstractControlChart();

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

	QString				makeTooltip();									// generate the label tooltip's
	void				setViewportModeFromChartMode(int nChartMode);	// convert chart mode from Gex into viewport mode

protected:

	virtual void		buildOwnerChart() = 0;							// build the owner chart, must be redefined by sub classes
	void				computeDataset();								// compute the owner dataset
	void				keepXTestViewport();							// Keep the viewport values for the reference test X

	GexXBarRDataset *	m_pXbarRDataset;
	QString				m_strTitle;										// Keep the chart title

};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXBarChart
//
// Description	:	chart class to draw the XBar chart into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////
class GexXBarChart : public GexAbstractControlChart
{

public:

    GexXBarChart(int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays);
	~GexXBarChart();

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

protected:

	void				buildOwnerChart();								// build the owner chart
	void				customizeChart();								// Customize chart by drawing directly on the chart
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexRChart
//
// Description	:	chart class to draw the R chart into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////
class GexRChart : public GexAbstractControlChart
{

public:

    GexRChart(int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays);
	~GexRChart();

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

protected:

	void				buildOwnerChart();								// build the owner chart
	void				customizeChart();								// Customize chart by drawing directly on the chart
};

#endif // GEX_XBAR_R_CHART_H
