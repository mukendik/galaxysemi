#ifndef GEX_CHARAC_BOXWHISKER_CHART_H
#define GEX_CHARAC_BOXWHISKER_CHART_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexabstractchart.h"
#include "gex_box_plot_data.h"

class GexCharacBoxWhiskerChart : public GexAbstractChart
{
public:

    GexCharacBoxWhiskerChart(int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays = NULL);
    ~GexCharacBoxWhiskerChart();

    ///////////////////////////////////////////////////////////
    // Methods
    ///////////////////////////////////////////////////////////

    QString		makeTooltip();                                              // generate the label tooltip's
    void		setViewportModeFromChartMode(int nChartMode);               // convert chart mode from Gex into viewport mode

protected:

    void		buildOwnerChart();                                          // build the owner chart, must be redefined by sub classes
    void        drawLabels(CGexFileInGroup *pFileX, CTest *pTestX);
    bool        drawLayers(CGexFileInGroup * pFileX, CTest * pTestX);
    bool        drawMarkers(CGexFileInGroup * pFileX, CTest * pTestX);
    bool        drawMarkersLimits(CGexFileInGroup * pFileX, CTest * pTestX);
    void        drawXAxisLabels();
    void        scaleChart();

    void		computeDataset();                                           // compute the owner dataset
    void		keepXTestViewport();                                        // Keep the viewport values for the reference test X

    void        fitPlotArea();                                              // fit the plot area depending on the axix thickness

    CGexBoxPlotData             mLayerDatasets;
    GexInternalChartInfo        mInternalChartInfo;
    QMap<QString, int>          mMapConditionsColor;
};

#endif // GEX_CHARAC_BOXWHISKER_CHART_H
