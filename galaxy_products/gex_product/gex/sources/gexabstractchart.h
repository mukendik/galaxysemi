#ifndef _GEX_ABSTRACT_CHART_H_
#define _GEX_ABSTRACT_CHART_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "chart_director.h"
#include "report_build.h"

#define OLD_CHART_LIMIT_INFINITE (double)3.402820018375656e+038

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexViewportManager
//
// Description	:	Class to manage the viewport
//
///////////////////////////////////////////////////////////////////////////////////
class GexViewportManager : public ViewPortManager
{

protected:

    GexViewportManager();
    virtual ~GexViewportManager();

    bool			isViewportInitialized() const;									// return true if viewport is fully initialized
    bool			isHorizontalViewportInitialized() const;						// return true if horizontal values have been initialized
    bool			isVerticalViewportInitialized() const;							// return true if vertical values have been initialized

    void			initHorizontalViewport(double dXMinBound, double dXMaxBound, double dXMinView, double dXMaxView, double dExtra = 0.0);	// Initialize horizontal values
    void			initVerticalViewport(double dXMinBound, double dXMaxBound, double dXMinView, double dXMaxView, double dExtra = 0.0);	// Initialize vertical values

    void			computeXBounds(double& dXLowValue, double& dXHighValue);		// Compute the lower and the upper bounds of the chart for X axis
    void			computeYBounds(double& dYLowValue, double& dYHighValue);		// Compute the lower and the upper bounds of the chart for Y axis

    void			resetVerticalViewport();										// reset vertical viewport values
    void			resetHorizontalViewport();										// reset horizontal viewport values

    virtual void	resetViewport();												// reset viewport values

private:

    double			m_dXLowerBound;
    double			m_dXHigherBound;
    double			m_dYLowerBound;
    double			m_dYHigherBound;
};


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAbstractChart
//
// Description	:	Base class to draw a chart into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////
/// \brief The GexAbstractChart class
class GexWizardChart;
class GexAbstractChart : public GexViewportManager, public CChartDirector
{
public:

    enum viewportMode
    {
        viewportOverLimits,
        viewportOverData,
        viewportAdaptive,
        viewportUser
    };

    enum chartType
    {
        chartTypeHistogram			= 0,
        chartTypeTrend				= 1,
        chartTypeProbabilityPlot	= 2,
        chartTypeScatter			= 3,
        chartTypeBoxPlot			= 4,
        chartTypeXBar				= 5,
        chartTypeRChart				= 6,
        chartTypeBoxWhisker         = 7,
        chartTypeCharacBoxWhisker   = 8,
        chartTypeCharacHisto        = 9,
        chartTypeCharacLine         = 10
    };

    enum chartOption
    {
        chartOptionHistoYScale,
        chartOptionHistoBar,
        chartOptionBoxWhiskerHorizontal,
        chartOptionBoxWhiskerVertical
    };

    GexAbstractChart(chartType eChartType, int nSizeMode, GexWizardChart*   lWizardParent, CGexChartOverlays * pChartOverlays = NULL);
    virtual ~GexAbstractChart();

    ///////////////////////////////////////////////////////////
    // Properties
    ///////////////////////////////////////////////////////////

    chartType			type() const									{ return m_eChartType; }
    CTest *				referenceTestX() const 							{ return m_pReferenceTestX; }
    CTest *				referenceTestY() const 							{ return m_pReferenceTestY; }
    CGexChartOverlays * chartOverlays() const							{ return m_pChartOverlays; }
    bool                isInteractiveMode() const                       { return (m_pChartOverlays != NULL); }

    double				xCursorValue() const							{ return m_dXCursorValue; }
    double				yCursorValue() const							{ return m_dYCursorValue; }
    double				xHotSpotValue() const							{ return m_dXHotSpotValue; }
    double				yHotSpotValue() const							{ return m_dYHotSpotValue; }

    int					viewportMode() const							{ return m_nViewportMode; }

    int					top() const										{ return topMargin(); }
    int					bottom() const									{ return height() - bottomMargin(); }
    int					left() const									{ return leftMargin(); }
    int					right() const									{ return width() - rightMargin(); }

    void				setViewportMode(int nViewport)					{ m_nViewportMode = nViewport; }
    void				setKeepViewport(bool bKeepViewport);

    ///////////////////////////////////////////////////////////
    // Methods
    ///////////////////////////////////////////////////////////

    bool				computeData(CReportOptions * pReportOptions, CTest * pReferenceTestX, CTest * pReferenceTestY = NULL);	// Compute the data before building the chart
    void				buildChart(int nWidth, int nHeight);								// Build the chart with a specific size
    void				buildChart();														// build the chart
    virtual void        buildGeometry();                                                    // Build the chart geometry

    bool				drawChart(const QString& strFile, const QString& strCopyright);		// draw the chart into a file
    bool				drawChart(MemBlock& memoryBlock);									// draw the chart into the memory

    virtual void		computeCursorValue(const QPoint& pointCursor);						// compute the X/Y values under the cursor

    virtual QString		makeTooltip() = 0;													// generate the label tooltip's, must be implemented for each derived class
    virtual void		setViewportModeFromChartMode(int nChartMode) = 0;					// convert chart mode from Gex into viewport mode, must be implemented for each derived class

    virtual void		onOptionChanged(GexAbstractChart::chartOption /*eChartOption*/)	{}		// Called when options changed

    virtual void		resetViewportManager(bool bEraseCustomViewport);							// reset viewport values
    void                setIsAdvHistoOnFile(bool b){ m_bIsAdvHistoOnFile = b; }
    inline void         setReportOptions(CReportOptions *pReportOptions){
        m_pReportOptions = pReportOptions;
    }
    inline const CReportOptions *getReportOptions(){
        return m_pReportOptions;
    }

    void                SetChartOverlays(CGexChartOverlays * chartOverlays);

protected:
    // Forward declaration
    class GexInternalChartInfo;

    bool                m_bIsAdvHistoOnFile ;
    CReportOptions *	m_pReportOptions;							// General GEX options
    CGexChartOverlays * m_pChartOverlays;							// overlays in Interactive mode
    ImageMapHandler *	m_hotSpotTester;							// handle the imagemap of the chart

    int					m_nSizeMode;								// holds the sizing mode of the chart
    int 				m_nViewportMode;							// holds the viewport mode

    QString				m_strLowLimit;								// Label for low limit marker
    QString				m_strHighLimit;								// label for high limit marker
    QString				m_strLowSpecLimit;							// Label for low Spec limit marker
    QString				m_strHighSpecLimit;							// label for high Spec limit marker

    double				m_dYHotSpotValue;							// Y value of the hotspot under the cursor, GEX_NAN if no hot spot
    double				m_dXHotSpotValue;							// X value of the hotspot under the cursor, GEX_NAN if no hot spot
    QString				m_strGroupName;
    int					m_nDataItem;								// The data point number within the data set. The first data point is 0. The nth data point is (n-1).
    int					m_nDataset;									// The data set number to which the data point belongs. The first data set is 0. The nth data set is (n-1)

    double				m_dYCursorValue;							// Y value of the point under the cursor, GEX_NAN if outside of the plot area
    double				m_dXCursorValue;							// X value of the point under the cursor, GEX_NAN if outside of the plot area
    bool				canUseCustomViewport() const						{ return m_bCanUseCustomViewport; }

    void				setLeftMargin(int nMargin)							{ m_nLeftMargin		= nMargin; }
    void				setRightMargin(int nMargin)							{ m_nRightMargin	= nMargin; }
    void				setTopMargin(int nMargin)							{ m_nTopMargin		= nMargin; }
    void				setBottomMargin(int nMargin)						{ m_nBottomMargin	= nMargin; }

    virtual bool		formatXAxisLabel()									{ return true; }		// Should the X axis label be formatted? Default is true.
    virtual void		customizeChart()									{}						// Customize chart by drawing directly on the chart
    virtual void		buildOwnerChart() = 0;														// build the owner chart, must be implemented in each derived class
    virtual void		computeDataset() = 0;														// computer the owner dataset, must be implemented in each derived class
    virtual void		chartingLineStyle(CGexSingleChart* /*pLayerStyle*/)	{}			// Default behaviour, do nothing
    virtual bool		hasSpotsToChart(CGexSingleChart * pLayerStyle);					// return true if the layer contains spots on this chart

    void				plotScriptingCustomMarkers(CTest *ptTestCell, int nLayerColor, int iLayer, double fCustomScaleFactor, bool bVerticalMarker, double& dLowValue, double& dHighValue);
    virtual void		plotSpecificScriptingCustomMarker(TestMarker* /*pTestMarker*/, int /*nColor*/, CTest* /*ptTestCell*/, double /*dCustomScaleFactor*/)	{}	// Specific plot for scripting custom markers

    bool				isSingleTestAllLayers();										// Tells if all layers relate to same test...
    bool				isMultiLayers();												// Check if charting multi-layers chart

    void				keepViewport();													// Save the viewport for the active tests
    virtual void		keepXTestViewport()	= 0;										// Keep the viewport values for the reference test X
    virtual void		keepYTestViewport()							{}					// Keep the viewport values for the reference test Y
    virtual bool		useTestCustomViewport();										// test if we can use the custom viewport

    void				drawCustomLine(int x1, int y1, int x2, int y2,
                                    int nLineColor, int nWidth);                        // Draw a line directly on the chart

    bool                addToMultiChart(int x, int y, BaseChart * pChart);

    /*!
     * \fn addMultiLimitMarkers(CGexSingleChart* Chart, CTest* TestCell, GexInternalChartInfo& InternalChartInfo)
     * \brief adds muti-limits markers to activ chart depending on user selection
     * \param Chart pointer to the chart object
     * \param TestCell pointer to the current test displayed in the chart
     * \param InternalChartInfo object containing internal chart information
     */
    void    addMultiLimitMarkers(CGexSingleChart* Chart, CTest* TestCell,
                                 GexInternalChartInfo& InternalChartInfo);

    class GexInternalChartInfo
    {
    public:

        GexInternalChartInfo();
        ~GexInternalChartInfo()		{}

        void                reset();

        int					m_nGroup;
        int					m_nVisibleLayerIndex;
        int					m_nLayerIndex;

        bool				m_bTestReferenceScaleFactor;
        bool				m_bTestReferenceScaleFactorY;
        int					m_nTestReferenceScaleFactor;
        int					m_nTestReferenceScaleFactorY;

        CTest *             m_pTestReferenceScaleX;
        CTest *             m_pTestReferenceScaleY;

        double				m_dLowX;
        double				m_dHighX;
        double				m_dLowY;
        double				m_dHighY;
        double				m_dViewportLowX;
        double				m_dViewportHighX;
        double				m_dViewportLowY;
        double				m_dViewportHighY;
        double				m_dMarkerMin;
        double				m_dMarkerMax;
        double				m_dMarkerMinY;
        double				m_dMarkerMaxY;
        double				m_dLowLimit;
        double				m_dHighLimit;
        double				m_dLowLimitY;
        double				m_dHighLimitY;
        double				m_dLowSpecLimit;
        double				m_dHighSpecLimit;
        double				m_dLowSpecLimitY;
        double				m_dHighSpecLimitY;
        QString				m_strTitle;
        QString				m_strXAxisLabel;
        QString				m_strYAxisLabel;
        CGexSingleChart *	m_pChart;
    };

protected:

    void				buildXYChart();								// build the chart, meaning allocate, build and size plot area
    bool				initXYChart();								// init a new chart struct
    void				sizePlotArea();								// size the plot area
    virtual void        fitPlotArea()       {}						// fit the plot area depending on the axix thickness

private:
    chartType			m_eChartType;								// holds the chart type
    int					m_nWidth;									// holds the width of the chart
    int					m_nHeight;									// holds the height of the chart
    int					m_nLeftMargin;								// defines the left margin beetween chart and plot area
    int					m_nRightMargin;								// defines the right margin beetween chart and plot area
    int					m_nTopMargin;								// defines the top margin beetween chart and plot area
    int					m_nBottomMargin;							// defines the bottom margin beetween chart and plot area
    bool				m_bKeepWieport;								// holds the status for keeping viewport through the navigation
    bool				m_bCanUseCustomViewport;					// holds the status if we can use the custom viewport

    CTest *				m_pReferenceTestX;							// holds the test used as reference for X data
    CTest *				m_pReferenceTestY;							// holds the test used as reference for Y data


protected:
    MultiChart *        m_pMultiChart;
    GexWizardChart*     mWizardParent;
    int m_iXScaleType, m_iYScaleType;
public:
    int					height() const										{ return m_nHeight; }
    void                setHeight(int iH)                                   { m_nHeight = iH; }
    int					width() const										{ return m_nWidth; }
    void                setWidth(int iW)                                    { m_nWidth = iW; }

    int					leftMargin() const									{ return m_nLeftMargin; }
    int					rightMargin() const									{ return m_nRightMargin; }
    int					topMargin() const									{ return m_nTopMargin; }
    int					bottomMargin() const								{ return m_nBottomMargin; }
    int					horizontalMargin() const							{ return (m_nLeftMargin + m_nRightMargin); }
    int					verticalMargin() const								{ return (m_nTopMargin + m_nBottomMargin); }
    void				getLayerStyles(int iGroup, CGexSingleChart *pChart, int& nFillColor, int& nLineColor, Chart::SymbolType& layerSymbol);

};

#endif // _GEX_ABSTRACT_CHART_H_
