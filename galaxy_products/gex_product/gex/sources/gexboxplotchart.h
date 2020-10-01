#ifndef _GEX_BOXPLOT_CHART_H_
#define _GEX_BOXPLOT_CHART_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexabstractchart.h"
#include "gex_box_plot_data.h"

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexBoxPlotChart
//
// Description	:	chart class to draw the box plot chart into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////
class GexBoxPlotChart : public GexAbstractChart
{

public:

    GexBoxPlotChart(int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays);
    ~GexBoxPlotChart();

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

    QString		makeTooltip();									// generate the label tooltip's
    void		setViewportModeFromChartMode(int nChartMode);	// convert chart mode from Gex into viewport mode

protected:

    void		buildOwnerChart();								// build the owner chart
    void		computeDataset()	{}							// compute the owner dataset
    void		keepXTestViewport();

    int			visibleLayerCount() const;						// Get the number of layer visible

    bool		fillChart(CGexFileInGroup * pFile, CTest * ptTestCell, GexInternalChartInfo& internalChartInfo, TextBox * pTextBox);

    void		plotSpecificScriptingCustomMarker(TestMarker * pTestMarker, int nColor, CTest *ptTestCell, double dCustomScaleFactor);	// Specific plot for scripting custom markers

private:

    CGexBoxPlotData		m_boxPlotDataSet;
    int					m_nMaxTextHeight;
    int					m_nMaxTextWidth;

    Chart::SymbolType   convertToChartDirectorSpot(int nSpotIndex);
};


#endif // _GEX_BOXPLOT_CHART_H_
