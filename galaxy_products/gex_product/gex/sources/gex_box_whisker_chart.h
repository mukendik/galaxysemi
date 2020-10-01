#ifndef GEX_BOX_WHISKER_CHART_H
#define GEX_BOX_WHISKER_CHART_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexabstractchart.h"

class CGexBoxPlotData;

class GexBoxWhiskerChart : public GexAbstractChart
{
public:

    enum Orientation
    {
        Horizontal,
        Vertical
    };

    GexBoxWhiskerChart(int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays = NULL);
    ~GexBoxWhiskerChart();

    ///////////////////////////////////////////////////////////
    // Methods
    ///////////////////////////////////////////////////////////

    QString		makeTooltip();                                              // generate the label tooltip's
    void		setViewportModeFromChartMode(int nChartMode);               // convert chart mode from Gex into viewport mode
    void		resetViewportManager(bool bEraseCustomViewport);                   // reset viewport values

protected:

    void		buildOwnerChart();                                          // build the owner chart, must be redefined by sub classes
    void		computeDataset();                                           // compute the owner dataset
    bool		fillDataset(CGexBoxPlotData*, CTest *, CTest *);            // fill the dataset
    void		keepXTestViewport();                                        // Keep the viewport values for the reference test X
    void		keepYTestViewport();                                        // Keep the viewport values for the reference test Y
    bool		useTestCustomViewport();                                    // test if we can use the custom viewport

    bool        drawLayers(CGexFileInGroup * pFileX, CTest * pTestX, CGexFileInGroup * pFileY, CTest * pTestY);
    bool        drawMarkers(CGexFileInGroup * pFileX, CTest * pTestX, CGexFileInGroup * pFileY, CTest * pTestY);
    bool        drawMarkersLimits(CGexFileInGroup * pFileX, CTest * pTestX, CGexFileInGroup * pFileY, CTest * pTestY);
    bool        drawMarkersStats(CGexFileInGroup * pFileX, CTest * pTestX, CGexFileInGroup * pFileY, CTest * pTestY);
    void        drawLabels(CGexFileInGroup * pFileX, CTest * pTestX, CGexFileInGroup * pFileY, CTest * pTestY);
    void        scaleChart();

    QMap<int, CGexBoxPlotData*> mLayerDatasets;
    GexInternalChartInfo        mInternalChartInfo;
};

#endif // GEX_BOX_WHISKER_CHART_H
