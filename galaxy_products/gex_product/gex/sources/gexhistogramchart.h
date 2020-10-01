#ifndef _GEX_HISTOGRAM_CHART_H_
#define _GEX_HISTOGRAM_CHART_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexabstractchart.h"

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexHistogramChart
//
// Description	:	chart class to draw the histogram chart into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////
class GexHistogramChart : public GexAbstractChart
{

public:

    enum histoViewportMode
    {
        viewportCumulLimits = viewportUser + 1,
        viewportCumulData = viewportUser + 2
    };

    GexHistogramChart(bool bStandard, int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays = NULL);
    ~GexHistogramChart();

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

    QString		makeTooltip();													// generate the label tooltip's
    void        buildGeometry();                                                // Build the chart geometry
    void		setViewportModeFromChartMode(int nChartMode);					// convert chart mode from Gex into viewport mode

    void		onOptionChanged(GexAbstractChart::chartOption eChartOption);	// Called when options changed

protected:

    void		buildOwnerChart();												// build the owner chart
    void		computeDataset();												// compute the owner dataset
    void		keepXTestViewport();

    /*!
           @brief fit the plot area depending on the axix thickness
       */
    void        fitPlotArea();

private:

    enum YAxisMode
    {
        YAxisUndefined,
        YAxisHits,
        YAxisPercentage
    };

    bool		m_bStandard;													// true if histogram must be drawn in standard mode, otherwise false
    YAxisMode	m_eYAxis;														// Hits or frequency
    /*!
     * \var mIsStacked
     */
    bool mIsStacked;
    void resetReportOptionToInteractiveOverlay();
public:
    void        buildChartFromData(char **pstzLegend, double *dY, int iCount, const QString &strTitle, const QString &strXTitle, const QString &strYTitle);
};

#endif // _GEX_HISTOGRAM_CHART_H
