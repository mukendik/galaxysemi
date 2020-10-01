///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "browser_dialog.h"
#include "gexabstractchart.h"
#include "gex_constants.h"
#include "report_options.h"
#include "gex_file_in_group.h"
#include "gex_report.h"
#include "drill_chart.h"

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern Chart::SymbolType	convertToChartDirectorSpot(int nSpotIndex);
extern CReportOptions ReportOptions;
extern CGexReport * gexReport;
extern double	ScalingPower(int iPower);

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexViewportManager::GexViewportManager() : ViewPortManager()
{
    setZoomInWidthLimit(0.001);
    setZoomInHeightLimit(0.001);

    resetViewport();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexViewportManager::~GexViewportManager()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool isViewportInitialized() const
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexViewportManager::isViewportInitialized() const
{
    return (isHorizontalViewportInitialized() && isVerticalViewportInitialized());
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool isHorizontalViewportInitialized() const
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexViewportManager::isHorizontalViewportInitialized() const
{
    bool bInitialized = true;

    bInitialized &= (m_dXLowerBound		!= GEX_C_DOUBLE_NAN);
    bInitialized &= (m_dXHigherBound	!= GEX_C_DOUBLE_NAN);

    return bInitialized;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool isVerticalViewportInitialized() const
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexViewportManager::isVerticalViewportInitialized() const
{
    bool bInitialized = true;

    bInitialized &= (m_dYLowerBound		!= GEX_C_DOUBLE_NAN);
    bInitialized &= (m_dYHigherBound	!= GEX_C_DOUBLE_NAN);

    return bInitialized;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void computeXBounds(double& dXLowValue, double& dXHighValue)
//
// Description	:	Compute the lower and the upper bounds of the chart for X axis
//
///////////////////////////////////////////////////////////////////////////////////
void GexViewportManager::computeXBounds(double& dXLowValue, double& dXHighValue)
{
    if (isHorizontalViewportInitialized())
    {
        double	dRange = m_dXHigherBound - m_dXLowerBound;

        dXLowValue	= m_dXLowerBound + getViewPortLeft() * dRange;
        dXHighValue	= dXLowValue + getViewPortWidth() * dRange;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void computeYBounds(double& dYLowValue, double& dYHighValue)
//
// Description	:	Compute the lower and the upper bounds of the chart for Y axis
//
///////////////////////////////////////////////////////////////////////////////////
void GexViewportManager::computeYBounds(double& dYLowValue, double& dYHighValue)
{
    if (isVerticalViewportInitialized())
    {
        double	dRange = m_dYHigherBound - m_dYLowerBound;

        dYHighValue = m_dYHigherBound - getViewPortTop() * dRange;
        dYLowValue	= dYHighValue - getViewPortHeight() * dRange;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void resetViewport()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexViewportManager::resetViewport()
{
    resetHorizontalViewport();
    resetVerticalViewport();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void resetVerticalViewport()
//
// Description	:	reset vertical viewport values
//
///////////////////////////////////////////////////////////////////////////////////
void GexViewportManager::resetVerticalViewport()
{
    m_dYLowerBound	= GEX_C_DOUBLE_NAN;
    m_dYHigherBound	= GEX_C_DOUBLE_NAN;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void resetVerticalViewport()
//
// Description	:	reset horizontal viewport values
//
///////////////////////////////////////////////////////////////////////////////////
void GexViewportManager::resetHorizontalViewport()
{
    m_dXLowerBound	= GEX_C_DOUBLE_NAN;
    m_dXHigherBound = GEX_C_DOUBLE_NAN;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void initHorizontalViewport(double dXMinBound, double dXMaxBound, double dXMinView, double dXMaxView, double dExtra /*=0.0*/)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexViewportManager::initHorizontalViewport(double dXMinBound, double dXMaxBound, double dXMinView, double dXMaxView, double dExtra /*=0.0*/)
{
    m_dXLowerBound	= dXMinBound;
    m_dXHigherBound	= dXMaxBound;

    // Add extra range if asked
    if (dExtra)
    {
        double dExtraRange = (m_dXHigherBound - m_dXLowerBound) * dExtra;

        m_dXLowerBound	-= dExtraRange;
        m_dXHigherBound += dExtraRange;
    }

    // Lower and higher are equal, add an extra range to ensure viewport is visible
    if (m_dXLowerBound == m_dXHigherBound)
    {
        if (m_dXLowerBound != 0.0)
        {
            m_dXLowerBound	*= (1 - dExtra);
            m_dXHigherBound	*= (1 + dExtra);
        }
        else
        {
            m_dXLowerBound	= -1;
            m_dXHigherBound	= 1;
        }
    }

    // Min and max are equal, add an extra range
    if (dXMinView == dXMaxView)
    {
        if (dXMinView != 0.0)
        {
            dXMinView	*= (1 - dExtra);
            dXMaxView	*= (1 + dExtra);
        }
        else
        {
            dXMinView	= -1;
            dXMaxView	= 1;
        }
    }

    if (dXMinView < m_dXLowerBound)
        dXMinView = m_dXLowerBound;

    if (dXMaxView > m_dXHigherBound)
        dXMaxView = m_dXHigherBound;

    // Calculate the range
    double dBoundRange = m_dXHigherBound - m_dXLowerBound;

    // Calculate the default viewport left
    double dViewportLeft = (dXMinView - m_dXLowerBound) / dBoundRange;

    // Calculate the default viewport width
    double dViewportWidth = ((dXMaxView - m_dXLowerBound) / dBoundRange) - dViewportLeft;

    // set viewport values
    setViewPortLeft(dViewportLeft);
    setViewPortWidth(dViewportWidth);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void initVerticalViewport(double dYMinBound, double dYMaxBound, double dYMinView, double dYMaxView, double dExtra /*=0.0*/)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexViewportManager::initVerticalViewport(double dYMinBound, double dYMaxBound, double dYMinView, double dYMaxView, double dExtra /*=0.0*/)
{
    m_dYLowerBound	= dYMinBound;
    m_dYHigherBound	= dYMaxBound;

    // Add extra range if asked
    if (dExtra)
    {
        double dExtraRange = (m_dYHigherBound - m_dYLowerBound) * dExtra;

        m_dYLowerBound	-= dExtraRange;
        m_dYHigherBound += dExtraRange;
    }

    // Lower and higher are equal, add an extra range to ensure viewport is visible
    if (m_dYLowerBound == m_dYHigherBound)
    {
        if (m_dXLowerBound != 0.0)
        {
            m_dYLowerBound	*= (1 - dExtra);
            m_dYHigherBound	*= (1 + dExtra);
        }
        else
        {
            m_dYLowerBound	= -1;
            m_dYHigherBound	= 1;
        }
    }

    // Min and max are equal, add an extra range
    if (dYMinView == dYMaxView)
    {
        if (dYMinView != 0.0)
        {
            dYMinView	*= (1 - dExtra);
            dYMaxView	*= (1 + dExtra);
        }
        else
        {
            dYMinView	= -1;
            dYMaxView	= 1;
        }
    }

    if (dYMinView < m_dYLowerBound)
        dYMinView = m_dYLowerBound;

    if (dYMaxView > m_dYHigherBound)
        dYMaxView = m_dYHigherBound;

    // Calculate the range
    double dBoundRange = m_dYHigherBound - m_dYLowerBound;

    // Calculate the default viewport left
    double dViewportTop = (m_dYHigherBound - dYMaxView) / dBoundRange;

    // Calculate the default viewport width
    double dViewportHeight = ((m_dYHigherBound - dYMinView) / dBoundRange) - dViewportTop;

    // set viewport values
    setViewPortTop(dViewportTop);
    setViewPortHeight(dViewportHeight);
}

///////////////////////////////////////////////////////////////////////////////////
// Class GexAbstractChart - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAbstractChart::GexAbstractChart(chartType eChartType, int nSizeMode, GexWizardChart*   lWizardParent, CGexChartOverlays * pChartOverlays /*= NULL*/) : GexViewportManager(), m_pReportOptions(0)
{
    m_bIsAdvHistoOnFile = false;
    m_eChartType			= eChartType;

    m_pMultiChart           = NULL;

    m_pReferenceTestX		= NULL;
    m_pReferenceTestY		= NULL;
    m_hotSpotTester			= NULL;

    m_strLowLimit			= "Low Limit";
    m_strHighLimit			= "High Limit";
    m_strLowSpecLimit		= "Low Spec Limit";
    m_strHighSpecLimit		= "High Spec Limit";
    m_pChartOverlays		= pChartOverlays;
    mWizardParent           = lWizardParent;

    m_nSizeMode				= nSizeMode;
    m_nWidth				= 0;
    m_nHeight				= 0;
    m_nLeftMargin			= 65;
    m_nRightMargin			= 20;
    m_nTopMargin			= 25;
    m_nBottomMargin			= 40;
    m_nDataItem				= -1;
    m_nDataset				= -1;

    m_dXHotSpotValue		= GEX_C_DOUBLE_NAN;
    m_dYHotSpotValue		= GEX_C_DOUBLE_NAN;

    m_dXCursorValue			= GEX_C_DOUBLE_NAN;
    m_dYCursorValue			= GEX_C_DOUBLE_NAN;

    m_nViewportMode			= viewportOverLimits;
    m_bKeepWieport			= false;
    m_bCanUseCustomViewport	= true;
    m_iXScaleType = 0;
    m_iYScaleType = 0;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAbstractChart::~GexAbstractChart()
{
    if (m_pMultiChart)
    {
        delete m_pMultiChart;
        m_pMultiChart = NULL;
    }

    clearXYChart();

    if (m_hotSpotTester)
    {
        delete m_hotSpotTester;
        m_hotSpotTester = NULL;
    }
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool isSingleTestAllLayers()
//
// Description	:	Tells if all layers relate to same test...
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAbstractChart::isSingleTestAllLayers()
{
    if(m_pChartOverlays)
    {
        CGexSingleChart	*	pChart	= NULL;	// Handle to Parameter Layer info.
        unsigned int		iTest	= 0;
        int					iPinmap = 0;

        for(int nLayerIndex = 0 ; nLayerIndex < m_pChartOverlays->chartsList().count(); nLayerIndex++)
        {
            pChart = m_pChartOverlays->chartsList().at(nLayerIndex);

            if(nLayerIndex == 0)
            {
                iTest	= pChart->iTestNumberX;
                iPinmap = pChart->iPinMapX;
            }

            if((pChart->iTestNumberX != iTest) || (pChart->iPinMapX != iPinmap))
                return false;	// Mismatch: not all layers showing same test#
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool isMultiLayers()
//
// Description	:	Check if charting multi-layers chart...
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAbstractChart::isMultiLayers()
{
    if(m_pChartOverlays && m_pChartOverlays->chartsList().count() > 1)
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool initXYChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAbstractChart::initXYChart()
{
    // Clear previous chart
    clearXYChart();

    if (m_pReportOptions)
    {
        // Allocate new one
        allocateXYChart(width(), height(), m_pReportOptions->cBkgColor.rgb() & 0xffffff);

        if(m_pChartOverlays)
            allocateXYChart(width(), height(), m_pChartOverlays->mBackgroundColor.rgb() & 0xffffff);

        if (m_pXYChart)
        {
            m_pXYChart->setClipping();

            //Set the font for the y axis labels to Arial
            m_pXYChart->yAxis()->setLabelStyle("arial.ttf");
            m_pXYChart->xAxis()->setLabelStyle("arial.ttf");

            m_pXYChart->xAxis()->setRounding(false, false);
            m_pXYChart->yAxis()->setRounding(false, false);

            if (formatXAxisLabel())
            {
                m_pXYChart->xAxis()->setFormatCondition("<", 0.001);
                m_pXYChart->xAxis()->setLabelFormat("{value|g4}");

                m_pXYChart->xAxis()->setFormatCondition(">", 9999);
                m_pXYChart->xAxis()->setLabelFormat("{value|g4}");

                m_pXYChart->xAxis()->setFormatCondition(">", 99999);
                m_pXYChart->xAxis()->setLabelFormat("{value|g5}");

                m_pXYChart->xAxis()->setFormatCondition("else");
                m_pXYChart->xAxis()->setLabelFormat("{value}");
            }

            m_pXYChart->yAxis()->setFormatCondition("<", 0.001);
            m_pXYChart->yAxis()->setLabelFormat("{value|g4}");

            m_pXYChart->yAxis()->setFormatCondition(">", 9999);
            m_pXYChart->yAxis()->setLabelFormat("{value|g4}");

            m_pXYChart->yAxis()->setFormatCondition(">", 99999);
            m_pXYChart->yAxis()->setLabelFormat("{value|g5}");

            m_pXYChart->yAxis()->setFormatCondition("else");
            m_pXYChart->yAxis()->setLabelFormat("{value}");

            // Force 0 label to O; issue known in ChartDirector.
            if (m_eChartType != GexAbstractChart::chartTypeBoxPlot &&
                m_eChartType != GexAbstractChart::chartTypeTrend &&
                m_eChartType != GexAbstractChart::chartTypeXBar &&
                m_eChartType != GexAbstractChart::chartTypeRChart &&
                m_eChartType != GexAbstractChart::chartTypeCharacBoxWhisker &&
                m_eChartType != GexAbstractChart::chartTypeCharacHisto &&
                m_eChartType != GexAbstractChart::chartTypeHistogram &&
                m_eChartType != GexAbstractChart::chartTypeCharacLine)
                m_pXYChart->xAxis()->addMark(0, -1, "0")->setMarkColor(Chart::Transparent, 0x000000, 0x000000);

            if (m_eChartType != GexAbstractChart::chartTypeProbabilityPlot &&
                m_eChartType != GexAbstractChart::chartTypeCharacHisto)
                m_pXYChart->yAxis()->addMark(0, -1, "0")->setMarkColor(Chart::Transparent, 0x000000, 0x000000);
        }
    }

    return (bool) (m_pXYChart != NULL);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void sizePlotArea()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::sizePlotArea()
{
    int nGridColor	= m_pXYChart->dashLineColor(0xc0c0c0, DashLine);
    int iHeight     =  height();
    int nLeftMargin = leftMargin();
    int nTopMargin  =  topMargin();

    int nAreaWidth  = width() - horizontalMargin();
    int nAreaHeight = iHeight - verticalMargin();

    PlotArea* paPtrPlotArea = m_pXYChart->setPlotArea(nLeftMargin, nTopMargin, nAreaWidth, nAreaHeight);
    paPtrPlotArea->setGridColor(nGridColor, nGridColor);

    // fit the plot area depending on the axix thickness
    fitPlotArea();

    // Layout the chart
    m_pXYChart->layout();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool computeData(CReportOptions * pReportOptions, CTest * pReferenceTestX, CTest * pReferenceTestY = NULL)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAbstractChart::computeData(CReportOptions * pReportOptions, CTest * pReferenceTestX, CTest * pReferenceTestY /*= NULL*/)
{
    if (isEnabled())
    {
        // Set first build flag
        if (m_bKeepWieport)
            m_bCanUseCustomViewport	= true;

        // Set options
        m_pReportOptions		= pReportOptions;

        // Set reference tests
        m_pReferenceTestX		= pReferenceTestX;
        m_pReferenceTestY		= pReferenceTestY;

        // Compute dataset
        computeDataset();

        // GCORE-8734: HTH
        // Don't reset vieport manager when recomputing data as it will makes the chart loosing zoom in/out
        // each time a display option is changed
//        resetViewportManager(false);

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void buildChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::buildChart()
{
// Build Image size
    buildGeometry();

    // Initialize some labels
    switch(m_nSizeMode)
    {
        case GEX_CHARTSIZE_SMALL:
        case GEX_CHARTSIZE_MEDIUM:
            // Shortest label possible
            m_strLowLimit		= "LL";
            m_strHighLimit		= "HL";
            m_strLowSpecLimit	= "LSL";
            m_strHighSpecLimit	= "HSL";
            break;

        default:
            break;
    }

    // Build XY Chart
    buildXYChart();
}

void GexAbstractChart::buildGeometry()
{
    // Image size to create for the trend chart
    switch(m_nSizeMode)
    {
        case GEX_CHARTSIZE_SMALL:
            m_nWidth	= GEX_CHARTSIZE_SMALL_X;
            m_nHeight	= GEX_CHARTSIZE_SMALL_Y;
            break;

        case GEX_CHARTSIZE_MEDIUM:
            m_nWidth	= GEX_CHARTSIZE_MEDIUM_X;
            m_nHeight	= GEX_CHARTSIZE_MEDIUM_Y;
            break;

        case GEX_CHARTSIZE_LARGE:
            m_nWidth	= GEX_CHARTSIZE_LARGE_X;
            m_nHeight	= GEX_CHARTSIZE_LARGE_Y;
            break;

        case GEX_CHARTSIZE_BANNER:
            m_nWidth	= GEX_CHARTSIZE_BANNER_X;
            m_nHeight	= GEX_CHARTSIZE_BANNER_Y;
            break;

        case GEX_CHARTSIZE_AUTO:
        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void buildChart(int nWidth, int nHeight)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::buildChart(int nWidth, int nHeight)
{
    m_nWidth	= nWidth;
    m_nHeight	= nHeight;

    // Build XY Chart
    buildXYChart();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void buildXYChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::buildXYChart()
{
    // Allocate a new XYChart
    if (initXYChart())
    {
        // Build the chart
        buildOwnerChart();

        // Adjust the plot area
        sizePlotArea();

        // Add custom draw on chart
        customizeChart();

        // keep viewport
        keepViewport();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool drawChart(MemBlock& memoryBlock)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAbstractChart::drawChart(MemBlock& memoryBlock)
{
    if (m_pXYChart)
    {
        // Draw the chart into the memory
        memoryBlock = m_pXYChart->makeChart(BMP);
        m_pXYChart->setTransparentColor(-1);
        // Sets the viewport
        setChartMetrics(m_pXYChart->getChartMetrics());

        // Create hotspot handler
        if (m_hotSpotTester)
        {
            delete m_hotSpotTester;
            m_hotSpotTester = NULL;
        }

        m_hotSpotTester = new ImageMapHandler(m_pXYChart->getHTMLImageMap("", "", "title='{x|10};{value|10};{dataGroupName};{dataItem};{dataSet}'"));

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool drawChart(const QString& strFile, const QString& strCopyright)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAbstractChart::drawChart(const QString& strFile, const QString& strCopyright)
{
    if (m_pMultiChart)
    {
        return m_pMultiChart->makeChart(strFile.toLatin1().data());
    }
    else
    {
        if (m_nSizeMode == GEX_CHARTSIZE_SMALL)
            return CChartDirector::drawChart(strFile, QString(""));
        else
            return CChartDirector::drawChart(strFile, strCopyright);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void getLayerStyles(int iGroup, CGexSingleChart *pChart, int& nFillColor, int& nLineColor, Chart::SymbolType& layerSymbol)
//
// Description	:	Get layer style & symbol style (if any)
//
///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::getLayerStyles(int iGroup, CGexSingleChart *pChart, int& nFillColor, int& nLineColor, Chart::SymbolType& layerSymbol)
{
    int nPatternCode = 0;

    // Set curve color: one different color per plot to allow easy stacking.
    if(pChart)
    {
        if(iGroup == 1)
            nFillColor = pChart->cColor.rgb() & 0xffffff;				// Interactive Histogram: user defined color
        else
            nFillColor = CGexReport::GetChartingColor(iGroup).rgb() & 0xffffff;	// Group#2 and higher can't be customized by users.

        switch(pChart->iLineStyle)
        {
            case 0:	// Solid line
                nPatternCode = 0;
                break;
            case 1:	// Dash line
                nPatternCode = DashLine;
                break;
            case 2:	// Dot line
                nPatternCode = DotLine;
                break;
            case 3:	// DashDot line
                nPatternCode = DotDashLine;
                break;
            case 4:	// DashDotDot line
                nPatternCode = AltDashLine;
                break;
        }

        if (nPatternCode)
            nLineColor  = m_pXYChart->dashLineColor(nFillColor, nPatternCode);
        else
            nLineColor  = nFillColor;

        // Symbol?
        if(hasSpotsToChart(pChart) == false)
            layerSymbol = Chart::NoSymbol;
        else
            layerSymbol = convertToChartDirectorSpot(pChart->iSpotStyle);
    }
    else
    {
        CGexSingleChart	*pLayerStyle = NULL;

        // Check if index is valid
        if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
            pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);

        // Non-interactive chart: either use default color, or custom color if defined under intercative mode
        if(pLayerStyle == NULL)
        {
            // No color defined under interactive mode for this group...then use default color
            nFillColor	= CGexReport::GetChartingColor(iGroup).rgb() & 0xffffff;
            nLineColor  = nFillColor;

            // Symbol?
            bool bSpot = false;
            if(!pLayerStyle)
            {
                QString ct=m_pReportOptions->GetOption("adv_trend","chart_type").toString();
                if ( ct=="spots" || ct=="lines_spots" )
                    bSpot = true;
            }

            if(hasSpotsToChart(pLayerStyle) == false && !bSpot)
                layerSymbol = Chart::NoSymbol;
            else
                layerSymbol = Chart::SquareSymbol;
        }
        else
        {
            // Get charting line style for PNG image to create.
            chartingLineStyle(pLayerStyle);

            // Use interactive color
            nFillColor = pLayerStyle->cColor.rgb() & 0xffffff;

            switch(pLayerStyle->iLineStyle)
            {
                case -1:	// No line
                    nPatternCode = -1;
                    break;
                case 0:	// Solid line
                    nPatternCode = 0;
                    break;
                case 1:	// Dash line
                    nPatternCode = DashLine;
                    break;
                case 2:	// Dot line
                    nPatternCode = DotLine;
                    break;
                case 3:	// DashDot line
                    nPatternCode = DotDashLine;
                    break;
                case 4:	// DashDotDot line
                    nPatternCode = AltDashLine;
                    break;
            }

            if (nPatternCode > 0)
                // Apply pattern to color
                nLineColor = m_pXYChart->dashLineColor(nFillColor, nPatternCode);
//            else if (nPatternCode == -1)
//                // Color set to transparent
//                nLineColor = nFillColor | 0xFF000000;
            else
                nLineColor = nFillColor;

            // Symbol?
            if(hasSpotsToChart(pLayerStyle) == false)
                layerSymbol = Chart::NoSymbol;
            else
                layerSymbol = convertToChartDirectorSpot(pLayerStyle->iSpotStyle);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void plotScriptingCustomMarkers(CTest *ptTestCell, int nLayerColor, int iLayer, double fCustomScaleFactor, bool bVerticalMarker, double& dLowValue, double& dHighValue)
//
// Description	:	Write Custom markers defined thru the scripting interface
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::plotScriptingCustomMarkers(CTest *ptTestCell, int nLayerColor, int iLayer, double fCustomScaleFactor, bool bVerticalMarker, double& dLowValue, double& dHighValue)
{
    // Return if we are in daemon mode (no gex main window)
    if(!ptTestCell )
        return;

    if(mWizardParent)
        mWizardParent->showPATLimitGroupBox(ptTestCell, ptTestCell->ptCustomMarkers);

    // Plot all markers defined for this test (for PAT markers, check options)
    int 	nLineMarkerColor;
    int 	nTextMarkerColor;
    double	lfMarkerPos;
    Axis *	pAxis = NULL;
    //int lTextRotation = 0;

    QList<TestMarker*>::iterator itBegin	= ptTestCell->ptCustomMarkers.begin();
    QList<TestMarker*>::iterator itEnd		= ptTestCell->ptCustomMarkers.end();
    CGexFileInGroup cFile(NULL,0,"",0,0,"","","");	// in order to use FormatTestResultNoUnits(...)
    cFile.UpdateOptions(m_pReportOptions);
    long dTempRunId = -1;

    // Get option about displaying PAT markers
    QStringList lMarkerOptions;
    if(m_pReportOptions)
        lMarkerOptions = m_pReportOptions->GetOption(QString("adv_histogram"), QString("marker")).toString()
                .split(QChar('|'));
    TestMarker* lMarker = NULL;
    while(itBegin != itEnd)
    {
        // Make sure we get a non-null pointer
        lMarker = *itBegin;
        if(lMarker == NULL)
            goto next_marker;

        // If static report and we have a PAT marker, verify option to plot PAT markers is set
        if(!isInteractiveMode() && lMarker->IsMarkerOfType(TestMarker::PatLimits) && !lMarkerOptions.contains(QString("patlimits")))
            goto next_marker;

        // Check if marker is visible: If marker is sticky to a layer, ignore color, and use same color as layer colr!
        if((lMarker->iLayer != iLayer-1) && (lMarker->iLayer >= 0))
            goto next_marker;	// This marker must only be charted for a specific layer!

        // If interactive mode, make sure the marker is selected
        if(isInteractiveMode() && mWizardParent &&
                !mWizardParent->patMarkerIsEnabled(lMarker->strLabel))
            goto next_marker;

        // Select marker color: either defined thru script, or left to invalid so to use same color as layer charting color
        // If marker is sticky to a layer, ignore color, and use same color as layer colr!
        if((lMarker->iLayer == iLayer-1) && (lMarker->bForceColor == false))
            nTextMarkerColor = nLayerColor;				// Use color of layer for a leyer-specific marker
        else
            nTextMarkerColor = lMarker->cColor.rgb() & 0xffffff;		// If marker visible on all layers....

        // Line color
        nLineMarkerColor = m_pXYChart->dashLineColor(nTextMarkerColor, DotDashLine);

        // Specific case in trend chart mode, outlier marker must always be vertical
        if (type() == chartTypeTrend && lMarker->lRunID != -1)
        {
            lfMarkerPos	= lMarker->lRunID;
            pAxis		= m_pXYChart->xAxis();
        }
        else
        {
            // Scale position from normalized to viewing scale
            lfMarkerPos = fCustomScaleFactor * lMarker->lfPos;
            cFile.FormatTestResultNoUnits(&lfMarkerPos,ptTestCell->res_scal);

            // check if Vertical or Horizontal
            if (!bVerticalMarker)
                pAxis = m_pXYChart->yAxis();
            else
                pAxis = m_pXYChart->xAxis();

            // Update viewport start-ending points, based on marker position
            dLowValue	= gex_min(dLowValue,	lfMarkerPos);
            dHighValue	= gex_max(dHighValue,	lfMarkerPos);
        }

        if (type() == chartTypeTrend && lMarker->lRunID != -1)
            dTempRunId = lMarker->lRunID;

        // Add specific data
        plotSpecificScriptingCustomMarker(lMarker, nTextMarkerColor, ptTestCell, fCustomScaleFactor);

        if (type() == chartTypeTrend && lMarker->lRunID != -1)
            lfMarkerPos = lMarker->lRunID;

        // Insert  marker
        /*lTextRotation = m_pReportOptions->
            GetOption(QString("adv_histogram"),
                      QString("marker_rotation")).toInt();*/
        //if (TextRotation != 0 && lMarker->strLabel.size() < 3)
        {
        Mark* lMark = addMarker(pAxis,lfMarkerPos, nLineMarkerColor,lMarker->strLabel, lMarker->iLine, Chart::TopRight);
        lMark->setFontColor(nTextMarkerColor);
        }
            //lMark->setFontAngle(lTextRotation);
           //lMark->setPos(0, -lMark->getWidth());
       // }
       /* else
        {
            addMarker(pAxis, lfMarkerPos, nLineMarkerColor,lMarker->strLabel, lMarker->iLine, Chart::TopLeft)->
                setFontColor(nTextMarkerColor);
        }*/

        if (type() == chartTypeTrend && lMarker->lRunID != -1)
            lMarker->lRunID = dTempRunId;

        // Move to next marker
next_marker:
        itBegin++;
    };
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void computeCursorValue(const QPoint& pointCursor)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::computeCursorValue(const QPoint& pointCursor)
{
    // Retrieve the hot spot under the mouse cursor
    if (m_hotSpotTester && m_hotSpotTester->getHotSpot(pointCursor.x(), pointCursor.y()))
    {
        // Get the tool tip on the hot spot (returns NULL if not on any hot spot)
        QString strTooltip = m_hotSpotTester->getValue("title");

        m_dXHotSpotValue	= strTooltip.section(";", 0, 0).toDouble();
        m_dYHotSpotValue	= strTooltip.section(";", 1, 1).toDouble();
        m_strGroupName		= strTooltip.section(";", 2, 2);
        m_nDataItem			= strTooltip.section(";", 3, 3).toInt();
        m_nDataset			= strTooltip.section(";", 4, 4).toInt();
    }
    else
    {
        m_dXHotSpotValue	= GEX_C_DOUBLE_NAN;
        m_dYHotSpotValue	= GEX_C_DOUBLE_NAN;
        m_strGroupName.clear();
        m_nDataItem			= -1;
        m_nDataset			= -1;
    }

    // Compute the x and y values under the cursor
    if (inPlotArea(pointCursor.x(), pointCursor.y()))
    {
        // Find the relative position in the plot area
        double dXOffset = ((double)pointCursor.x() - (double)getPlotAreaLeft()) / (double) getPlotAreaWidth();

        // Compute the x value
        m_dXCursorValue = m_pXYChart->xAxis()->getMinValue() + (dXOffset * ( m_pXYChart->xAxis()->getMaxValue() - m_pXYChart->xAxis()->getMinValue()));

        // Find the relative position in the plot area
        double dYOffset = ((double)pointCursor.y() - (double)getPlotAreaTop()) / (double) getPlotAreaHeight();

        // Compute the y value
        m_dYCursorValue = m_pXYChart->yAxis()->getMaxValue() + (dYOffset * ( m_pXYChart->yAxis()->getMinValue() - m_pXYChart->yAxis()->getMaxValue()));
    }
    else
    {
        m_dXCursorValue		= GEX_C_DOUBLE_NAN;
        m_dYCursorValue		= GEX_C_DOUBLE_NAN;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void computeCursorValue(const QPoint& pointCursor)
//
// Description	:	Save the viewport for the active tests
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::keepViewport()
{
    if (m_bKeepWieport)
    {
        // Check if test X is valid and if we are in interactive mode
        if (referenceTestX() && m_pChartOverlays)
        {
            // If no chart options exists, simply create it
            if (referenceTestX()->ptChartOptions == NULL)
                referenceTestX()->ptChartOptions = new CGexTestChartingOptions();

            // Store the viewport for X test
            keepXTestViewport();
        }

        // Check if test Y is valid and if we are in interactive mode
        if (referenceTestY() && m_pChartOverlays)
        {
            if (referenceTestY()->ptChartOptions == NULL)
                referenceTestY()->ptChartOptions = new CGexTestChartingOptions();

            // Store the viewport for Y test
            keepYTestViewport();
        }
    }

    // Indicate that the chart has been drawn at least one time. Do not take into account the custom viewport when redrawing chart
    m_bCanUseCustomViewport = false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setKeepViewport(bool bKeepViewport)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::setKeepViewport(bool bKeepViewport)
{
    m_bKeepWieport = bKeepViewport;

    if (m_bKeepWieport)
        keepViewport();
    else
        resetViewportManager(false);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void useTestCustomViewport()
//
// Description	:	test if we can use the custom viewport
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAbstractChart::useTestCustomViewport()
{
    if (m_bCanUseCustomViewport && referenceTestX() && referenceTestX()->ptChartOptions &&
        (referenceTestX()->ptChartOptions->customViewportX() || referenceTestX()->ptChartOptions->customViewportY()))
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void resetViewport(bool bEraseCustomViewport)
//
// Description	:	reset viewport values
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::resetViewportManager(bool bEraseCustomViewport)
{
    if (bEraseCustomViewport && referenceTestX() && referenceTestX()->ptChartOptions)
    {
        referenceTestX()->ptChartOptions->setCustomViewportX(false);
        referenceTestX()->ptChartOptions->setCustomViewportY(false);
    }
    m_iXScaleType = 0;
    m_iYScaleType = 0;
    GexViewportManager::resetViewport();
}

void GexAbstractChart::SetChartOverlays(CGexChartOverlays *chartOverlays)
{
    m_pChartOverlays = chartOverlays;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool hasSpotsToChart(CGexSingleChart * pLayerStyle)
//
// Description	:	return true if the layer contains spots on this chart
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAbstractChart::hasSpotsToChart(CGexSingleChart * pLayerStyle)
{
    if (pLayerStyle)
        return pLayerStyle->bSpots;
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void drawCustomLine(int x1, int y1, int x2, int y2, int nLineColor, int nWidth)
//
// Description	:	Draw a line directly on the chart
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractChart::drawCustomLine(int x1, int y1, int x2, int y2, int nLineColor, int nWidth)
{
    double m = (double)(y1-y2)/(x1-x2);         // Coefficient directeur
    double p = (double) (y1 - m * x1);          // Ordonnée à l'origine

    // ensure we are going to draw the line only in the plot area.
    if (x1 < left())
    {
        x1 = left();
        y1 = static_cast<int>(m * x1 + p);
    }
    else if (x1 > right())
    {
        x1 = right();
        y1 = static_cast<int>(m * x1 + p);
    }

    if (x2 < left())
    {
        x2 = left();
        y2 = static_cast<int>(m * x2 + p);
    }
    else if (x2 > right())
    {
        x2 = right();
        y2 = static_cast<int>(m * x2 + p);
    }

    if (y1 > bottom())
    {
        y1 = bottom();
        x1 = static_cast<int>((y1 - p) / m);
    }
    else if (y1 < top())
    {
        y1 = top();
        x1 = static_cast<int>((y1 - p) / m);
    }

    if (y2 > bottom())
    {
        y2 = bottom();
        x2 = static_cast<int>((y2 - p) / m);
    }
    else if (y2 < top())
    {
        y2 = top();
        x2 = static_cast<int>((y2 - p) / m);
    }

    // Define the rectangle of the chart area.
    QRect rectArea(left(), top(), width(), height());

    // Draw the line only the 2 points are in the plot area.
    if (rectArea.contains(x1, y1) && rectArea.contains(x2, y2))
        m_pXYChart->addLine(x1, y1, x2, y2, nLineColor, nWidth);
}


///////////////////////////////////////////////////////////
// Class GexAbstractChart::GexInternalChartInfo
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAbstractChart::GexInternalChartInfo::GexInternalChartInfo()
{
    reset();
}

void GexAbstractChart::GexInternalChartInfo::reset()
{
    m_bTestReferenceScaleFactor     = false;
    m_bTestReferenceScaleFactorY	= false;
    m_nGroup                        = 1;
    m_nVisibleLayerIndex            = 0;
    m_nLayerIndex                   = 0;
    m_nTestReferenceScaleFactor     = 1;
    m_nTestReferenceScaleFactorY    = 1;
    m_dLowX                         = C_INFINITE;
    m_dLowY                         = C_INFINITE;
    m_dViewportLowX                 = C_INFINITE;
    m_dViewportLowY                 = C_INFINITE;
    m_dMarkerMin                    = C_INFINITE;
    m_dLowLimit                     = C_INFINITE;
    m_dLowSpecLimit                 = C_INFINITE;
    m_dMarkerMinY                   = C_INFINITE;
    m_dLowLimitY                    = C_INFINITE;
    m_dLowSpecLimitY                = C_INFINITE;
    m_dHighX                        = -C_INFINITE;
    m_dHighY                        = -C_INFINITE;
    m_dViewportHighX                = -C_INFINITE;
    m_dViewportHighY                = -C_INFINITE;
    m_dMarkerMax                    = -C_INFINITE;
    m_dHighLimit                    = -C_INFINITE;
    m_dHighSpecLimit                = -C_INFINITE;
    m_dMarkerMaxY                   = -C_INFINITE;
    m_dHighLimitY                   = -C_INFINITE;
    m_dHighSpecLimitY               = -C_INFINITE;

    m_pTestReferenceScaleX          = NULL;
    m_pTestReferenceScaleY          = NULL;
    m_pChart                        = NULL;
}

void GexAbstractChart::addMultiLimitMarkers(CGexSingleChart* Chart, CTest* TestCell,
                                            GexInternalChartInfo& InternalChartInfo)
{
    // Sanity check
    if(!Chart || !TestCell)
        return;

    // Draw multi-limit markers. Draw only if:
    // - Interactive mode (pChart != NULL)
    // - Single group, single layer (isMultiLayers() = false)
    if((Chart==NULL) || (!Chart->bVisible) || isMultiLayers())
        return;

    QString lScaling=m_pReportOptions->GetOption("dataprocessing","scaling").toString();

    // First create a LL & HL map so that differnt labels for a same limit value do not overlap
    QMap<double, QString> lLLMarkers, lHLMarkers;
    for (int lLimitItemIdx = 0; lLimitItemIdx < TestCell->MultiLimitItemCount(); ++lLimitItemIdx)
    {
        // LL
        GS::Core::MultiLimitItem* lMultiLimitItem = TestCell->GetMultiLimitItem(lLimitItemIdx);
        if (!lMultiLimitItem)
        {
            continue;
        }
        if (lMultiLimitItem->IsValidLowLimit())
        {
            double lfLowL = lMultiLimitItem->GetLowLimit();
            if (lScaling!="normalized")
            {
                // convert LowLimit to same scale as results:
                lfLowL /=  ScalingPower(TestCell->res_scal);	// normalized
            }

            // No custom scaling depending on ref test for now, as multi limits markers are displayed only in case
            // of single layer

            // Update global lowest limit for chart viewport
            InternalChartInfo.m_dLowLimit = gex_min(InternalChartInfo.m_dLowLimit, lfLowL);
        }

        // HL
        if (lMultiLimitItem->IsValidHighLimit())
        {
            double lfHighL = lMultiLimitItem->GetHighLimit();
            if (lScaling!="normalized")
            {
                // convert HighLimit to same scale as results:
                lfHighL /=  ScalingPower(TestCell->res_scal);	// normalized
            }

            // No custom scaling depending on ref test for now, as multi limits markers are displayed only in case
            // of single layer

            // Update global highest limit for chart viewport
            InternalChartInfo.m_dHighLimit = gex_max(InternalChartInfo.m_dHighLimit, lfHighL);
        }
    }

    // Get pen color and width. Use the one defined for standard limits. If width is 0 (used to indicate
    // that standard limits checkbox is unchecked, set pen width to 1)
    QColor cPenColor = Chart->limitsColor(isMultiLayers());
    int cPenWidth = Chart->limitsLineWidth() > 0 ? Chart->limitsLineWidth() : 1;

    // Add markers depending on chart type
    QMap<double, QString>::const_iterator i;
    // LL
    for (i=lLLMarkers.constBegin(); i!=lLLMarkers.constEnd(); ++i)
    {
        if (m_pXYChart == NULL)
        {
            return;
        }
        switch(m_eChartType)
        {
            case chartTypeTrend:
                addMarker(m_pXYChart->yAxis(), i.key(), cPenColor.rgb() & 0xffffff, i.value(), cPenWidth, Chart::BottomLeft);
                break;

            case chartTypeHistogram:
                addMarker(m_pXYChart->xAxis(), i.key(), cPenColor.rgb() & 0xffffff, i.value(), cPenWidth, Chart::TopLeft);
                break;

            case chartTypeBoxPlot:
                addMarker(m_pXYChart->yAxis(), i.key(), cPenColor.rgb() & 0xffffff, i.value(), cPenWidth, Chart::BottomLeft);
                break;

            case chartTypeProbabilityPlot:
                addMarker(m_pXYChart->xAxis(), i.key(), cPenColor.rgb() & 0xffffff, i.value(), cPenWidth, Chart::TopLeft);
                break;

            default:
                break;
        }
    }
    // HL
    for (i=lHLMarkers.constBegin(); i!=lHLMarkers.constEnd(); ++i)
    {
        if (m_pXYChart == NULL)
        {
            return;
        }
        switch(m_eChartType)
        {
            case chartTypeTrend:
                addMarker(m_pXYChart->yAxis(), i.key(), cPenColor.rgb() & 0xffffff, i.value(), cPenWidth);
                break;

            case chartTypeHistogram:
                addMarker(m_pXYChart->xAxis(), i.key(), cPenColor.rgb() & 0xffffff, i.value(), cPenWidth, Chart::TopRight);
                break;

            case chartTypeBoxPlot:
                addMarker(m_pXYChart->yAxis(), i.key(), cPenColor.rgb() & 0xffffff, i.value(), cPenWidth);
                break;

            case chartTypeProbabilityPlot:
                addMarker(m_pXYChart->xAxis(), i.key(), cPenColor.rgb() & 0xffffff, i.value(), cPenWidth, Chart::TopRight);
                break;

            default:
                break;
        }
    }
}
