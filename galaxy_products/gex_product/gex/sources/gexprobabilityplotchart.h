#ifndef _GEX_PROBABILITYPLOT_CHART_H_
#define _GEX_PROBABILITYPLOT_CHART_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexabstractchart.h"

class CGexProbabilityPlotData;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexProbabilityPlotChart
//
// Description	:	chart class to draw the probability plot chart into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////
class GexProbabilityPlotChart : public GexAbstractChart
{

public:

    GexProbabilityPlotChart(int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays = NULL);
	~GexProbabilityPlotChart();

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

	QString		makeTooltip();															// generate the label tooltip's
	void		setViewportModeFromChartMode(int nChartMode);							// convert chart mode from Gex into viewport mode

protected:

	void		buildOwnerChart();														// build the owner chart
	void		computeDataset();														// compute the owner dataset
	void		keepXTestViewport();

	bool		hasSpotsToChart(CGexSingleChart* /*pLayerStyle*/) { return true; }	// Always return true

	int			visibleLayerCount() const;												// Get the number of layer visible

	void		fillDataset(CGexProbabilityPlotData& dataSet, CTest * ptTestCell);		// fill the probability plot dataset
	bool		fillChart(CGexFileInGroup * pFile, CTest * ptTestCell, GexInternalChartInfo& internalChartInfo, CGexProbabilityPlotData& dataSet);

private:

	QList<CGexProbabilityPlotData> m_lstProbabilityPlotDataset;
};

#endif // _GEX_PROBABILITYPLOT_CHART_H_
