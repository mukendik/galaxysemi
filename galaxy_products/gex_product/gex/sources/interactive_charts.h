///////////////////////////////////////////////////////////
// interactive_charts.h class HEADER : all classes to process files
///////////////////////////////////////////////////////////

#ifndef INTERACTIVE_CHARTS_H__INCLUDED_
#define INTERACTIVE_CHARTS_H__INCLUDED_
#include <qwidget.h>
#include <qpoint.h>
#include <qstring.h>
#include <qcolor.h>
#include <QMap>
#include <QJsonObject>

#define	GEX_WHISKER_RANGE           0
#define	GEX_WHISKER_Q1Q3            1
#define	GEX_WHISKER_IQR             2
#define	GEX_WHISKER_HALFPERCEN		3
#define GEX_WHISKER_TWOPERCEN       4
#define GEX_WHISKER_TENPERCEN       5

#include "test_defines.h"

class CReportOptions;

void    ColorRGBToJsonArray(QJsonArray& colorJson, const QColor &color);
QColor  JsonArrayToColorRGB(const QJsonArray& colors, const QColor &defaultColor);

class CGexCustomMarker
{
public:
    enum MarkerType
    {
        eMarkerText,					// Write a vertical line marker
        eMarkerSurround,				// Image overlay: Surround specified area, without drawing an arrow
        eMarkerSurroundWithArrow,		// Image overlay:Surround specified area, and add an arrow pointing to the surrounded area
        eMarkerArrow					// Image overlay:Draw an arrow pointing to the specified area
    };

    enum MarkerSize
    {
        eMarkerSmall,					// Draw a small marker (or 1 pixel if text marker)
        eMarkerMedium,					// Draw a medium marker (or 2 pixel if text marker)
        eMarkerLarge					// Draw a large marker (or 3 pixel if text marker)
    };

    MarkerType		m_eMarkerType;		// Type of marker (see enum)
    double			m_lfPosX;			// X position of the area to highlight
    double			m_lfPosY;			// Y position of the area to highlight (ignored if marker type is 'eMarkerText')
    MarkerSize		m_eMarkerSize;		// Size of marker to draw (see enum)
    QColor			m_cMarkerColor;		// RGB color (only applies to 'eMarketTest' type).
    QString			m_strMarkerLabel;	// Label to display to the side of the vertical marker line (only applies to 'eMarketTest' type).
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexProbabilityPlotData
//
// Description	:	Class used to store the result of data computed for
//					Probability Plot chart
//
///////////////////////////////////////////////////////////////////////////////////
class CGexProbabilityPlotData
{

public:

    CGexProbabilityPlotData();
    CGexProbabilityPlotData(const CGexProbabilityPlotData& data);
    ~CGexProbabilityPlotData();

///////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////

    double			minY() const					{ return m_dMinY; }
    double			maxY() const					{ return m_dMaxY; }

    long			size() const					{ return m_lSize; }

    const double *	dataX() const					{ return m_pXData; }
    const double *	dataY() const					{ return m_pYData; }

    const double&	dataXAt(int nIndex) const		{ return m_pXData[nIndex]; }
    const double&	dataYAt(int nIndex) const		{ return m_pYData[nIndex]; }

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
    bool			isReady(long lTestNumber) const;							// Test if data have already been computer for this test

    void			init(long lSize, long lTestNumber, int localScaleFactor);							// initialize class and create internal array
    void			clean();													// Clean data and delete internal array

    void            rescale(int newScaleFator);

    void			addData(long lIndex, double dXData, double dYData);			// Add a data at the index

    /*!
     * \fn SetLineToTheoreticalNormalQQPlotInter
     */
    void SetLineToTheoreticalNormalQQPlotInter(double a) {
        this->mLineToTheoreticalNormalQQPlotInter = a; }
    /*!
     * \fn SetLineToTheoreticalNormalQQPlotSlope
     */
    void SetLineToTheoreticalNormalQQPlotSlope(double a) {
        this->mLineToTheoreticalNormalQQPlotSlope = a; }
    /*!
     * \fn GetLineToTheoreticalNormalQQPlotInter
     */
    double GetLineToTheoreticalNormalQQPlotInter() const {
        return this->mLineToTheoreticalNormalQQPlotInter; }
    /*!
     * \fn GetLineToTheoreticalNormalQQPlotSlope
     */
    double GetLineToTheoreticalNormalQQPlotSlope() const {
        return this->mLineToTheoreticalNormalQQPlotSlope; }

    CGexProbabilityPlotData& operator=(const CGexProbabilityPlotData& data);	// Override the copy operator

private:

    long		m_lSize;				// Internal array size
    long		m_lTestNumber;			// Test number used to compute this data
    int         mLocalScaleFactor;      // Scale factor for current test
    int         mReferenceScaleFactor;  // Scale factor for reference test

    double *	m_pXData;				// Array of data on X axis
    double *	m_pYData;				// Array of data on Y axis

    double		m_dMaxY;				// Max value in Y array
    double		m_dMinY;				// Min value in X array

    /*!
     * \var mLineToTheoreticalNormalQQPlotInter
     * \brief
     */
    double mLineToTheoreticalNormalQQPlotInter;
    /*!
     * \var mLineToTheoreticalNormalQQPlotSlope
     * \brief
     */
    double mLineToTheoreticalNormalQQPlotSlope;
};


class CGexSingleChart
{

public:

    CGexSingleChart(int iEntry=0);					// constructor.
    CGexSingleChart(const CGexSingleChart& other);	// copy constructor
    ~CGexSingleChart();

    CGexSingleChart&			operator=(const CGexSingleChart& other);
    void						resetVariables(int iEntry=0);

    CGexProbabilityPlotData&	dataProbabilityPlot()		{ return m_dataProbabilityPlot; }


    void            CopySettings (const CGexSingleChart* other);

    QString strChartName;	// Label for this chart

    // Parameter in X to plot on the chart
    unsigned int	iTestNumberX;
    int				iPinMapX;
    QString			strTestNameX;	// Parameter name (as found in data file)
    QString			strTestLabelX;	// Parameter label (user may change it. Default = Parameter name)
    int				iGroupX;

    // Parameter in Y to plot on the chart
    unsigned int	iTestNumberY;
    int				iPinMapY;
    QString			strTestNameY;	// Parameter name (as found in data file)
    QString			strTestLabelY;	// Parameter label (user may change it. Default = Parameter name)
    int				iGroupY;

    // Parameter in Z to plot on the chart
    unsigned int	iTestNumberZ;
    int				iPinMapZ;
    QString			strTestNameZ;	// Parameter name (as found in data file)
    QString			strTestLabelZ;	// Parameter label (user may change it. Default = Parameter name)
    int				iGroupZ;

    // Style
    bool			bVisible;		// true if visible (default)
    bool			bBoxBars;		// true if draw with BARS (histogram charting only)
    bool			bBox3DBars;		// true if draw with 3D-BARS (histogram charting only)


    bool            mIsStacked;
    bool            mLayerName;
    bool            mQQLine;
   // bool            mSpecLimit;
    bool			bFittingCurve;	// true if draw fitting curve / spin.
    bool			bBellCurve;		// true if draw the Guaussian Bell-curve shape.
    bool			bLines;			// true if connect points with a line
    bool			bSpots;			// true if draw a spot at each data point.
    int				iWhiskerMode;	// Box-Plot Whisker mode: GEX_WHISKER_XXX (range, IQR, etc)
    int				iLineWidth;		// Plotting line width
    QColor			cColor;			// Line color
    int				iLineStyle;		// Line style: solid, dashed, etc...
    int				iSpotStyle;		// Spot style: circle, diamond, rectangle, etc...
    double			lfDataOffset;	// Offset to apply to data in layer (trend chart only)
    double			lfDataScale;	// Scale factor to apply to data in layer (trend chart only)

    int				meanLineWidth() const						{ return m_iMeanLineWidth; }
    int				medianLineWidth() const						{ return m_iMedianLineWidth; }
    int				minLineWidth() const						{ return m_iMinLineWidth; }
    int				maxLineWidth() const						{ return m_iMaxLineWidth; }
    int				limitsLineWidth() const						{ return m_iLimitsLineWidth; }
    int				specLimitsLineWidth() const                 { return m_iSpecLimitsLineWidth; }
    int				sigma2LineWidth() const						{ return m_i2SigmaLineWidth; }
    int				sigma3LineWidth() const						{ return m_i3SigmaLineWidth; }
    int				sigma6LineWidth() const						{ return m_i6SigmaLineWidth; }
    int				sigma12LineWidth() const					{ return m_i12SigmaLineWidth; }
    int				quartileQ3LineWidth() const					{ return m_iQuartileQ3LineWidth; }
    int				quartileQ1LineWidth() const					{ return m_iQuartileQ1LineWidth; }
    int				GetRollingLimitsLineWidth() const			{ return m_iRollingLimitsLineWidth; }
    /*!
     * \fn getTextRotation
     * \brief in degrees
     */
    int getTextRotation() const { return mTextRotation; }

    const QColor&	meanColor(bool bMultiLayer = false) const	{ return (bMultiLayer) ? cColor : m_cMeanColor; }
    const QColor&	medianColor(bool bMultiLayer = false) const	{ return (bMultiLayer) ? cColor : m_cMedianColor; }
    const QColor&	minColor(bool bMultiLayer = false) const	{ return (bMultiLayer) ? cColor : m_cMinColor; }
    const QColor&	maxColor(bool bMultiLayer = false) const	{ return (bMultiLayer) ? cColor : m_cMaxColor; }
    const QColor&	limitsColor(bool bMultiLayer = false) const	{ return (bMultiLayer) ? cColor : m_cLimitsColor; }
    const QColor&	specLimitsColor(bool bMultiLayer = false) const	{ return (bMultiLayer) ? cColor : m_cSpecLimitsColor; }
    const QColor&	sigma2Color(bool bMultiLayer = false) const	{ return (bMultiLayer) ? cColor : m_c2SigmaColor; }
    const QColor&	sigma3Color(bool bMultiLayer = false) const	{ return (bMultiLayer) ? cColor : m_c3SigmaColor; }
    const QColor&	sigma6Color(bool bMultiLayer = false) const	{ return (bMultiLayer) ? cColor : m_c6SigmaColor; }
    const QColor&	sigma12Color(bool bMultiLayer = false) const{ return (bMultiLayer) ? cColor : m_c12SigmaColor; }
    const QColor&	quartileQ3Color(bool bMultiLayer = false) const{ return (bMultiLayer) ? cColor : m_cQuartileQ3Color; }
    const QColor&	quartileQ1Color(bool bMultiLayer = false) const{ return (bMultiLayer) ? cColor : m_cQuartileQ1Color; }

    void			setMeanLineWidth(int nWidth)				{ m_iMeanLineWidth = nWidth; }
    void			setMedianLineWidth(int nWidth)				{ m_iMedianLineWidth = nWidth; }
    void			setMinLineWidth(int nWidth)					{ m_iMinLineWidth = nWidth; }
    void			setMaxLineWidth(int nWidth)					{ m_iMaxLineWidth = nWidth; }
    void			setLimitsLineWidth(int nWidth)				{ m_iLimitsLineWidth = nWidth; }
    void			setSpecLimitsLineWidth(int nWidth)          { m_iSpecLimitsLineWidth = nWidth; }
    void			setRollingLimitsLineWidth(int nWidth)		{ m_iRollingLimitsLineWidth = nWidth; }
    void			set2SigmaLineWidth(int nWidth)				{ m_i2SigmaLineWidth = nWidth; }
    void			set3SigmaLineWidth(int nWidth)				{ m_i3SigmaLineWidth = nWidth; }
    void			set6SigmaLineWidth(int nWidth)				{ m_i6SigmaLineWidth = nWidth; }
    void			set12SigmaLineWidth(int nWidth)				{ m_i12SigmaLineWidth = nWidth; }
    void			setQuartileQ1(int nWidth)				{ m_iQuartileQ1LineWidth = nWidth; }
    void			setQuartileQ3(int nWidth)				{ m_iQuartileQ3LineWidth = nWidth; }
    /*!
     * \fn setTextRotation
     * \brief in degrees
     */
    void setTextRotation(int degrees) { mTextRotation = degrees; }

    void			setMeanColor(const QColor& cColor)			{ m_cMeanColor = cColor; }
    void			setMedianColor(const QColor& cColor)		{ m_cMedianColor = cColor; }
    void			setMinColor(const QColor& cColor)			{ m_cMinColor = cColor; }
    void			setMaxColor(const QColor& cColor)			{ m_cMaxColor = cColor; }
    void			setLimitsColor(const QColor& cColor)		{ m_cLimitsColor = cColor; }
    void			setSpecLimitsColor(const QColor& cColor)	{ m_cSpecLimitsColor = cColor; }
    void			set2SigmaColor(const QColor& cColor)		{ m_c2SigmaColor = cColor; }
    void			set3SigmaColor(const QColor& cColor)		{ m_c3SigmaColor = cColor; }
    void			set6SigmaColor(const QColor& cColor)		{ m_c6SigmaColor = cColor; }
    void			set12SigmaColor(const QColor& cColor)		{ m_c12SigmaColor = cColor; }
    void			setQuartileQ1Color(const QColor& cColor)	{ m_cQuartileQ1Color = cColor; }
    void			setQuartileQ3Color(const QColor& cColor)	{ m_cQuartileQ3Color = cColor; }

    bool            InitFromJSon                                (const QJsonObject &description);
    bool            ToJson                                      (QJsonObject& descriptionOut) const;
private:

    // Probability plot data
    CGexProbabilityPlotData		m_dataProbabilityPlot;

    // Markers: Line width (0=hide), color
    int				m_iMeanLineWidth;		// Plotting line width
    QColor			m_cMeanColor;			// Line color

    int				m_iMedianLineWidth;		// Plotting line width
    QColor			m_cMedianColor;			// Line color

    int				m_iMinLineWidth;		// Plotting line width
    QColor			m_cMinColor;			// Line color

    int				m_iMaxLineWidth;		// Plotting line width
    QColor			m_cMaxColor;			// Line color

    int				m_iLimitsLineWidth;		// Plotting line width
    QColor			m_cLimitsColor;			// Line color

    int				m_iSpecLimitsLineWidth;		// Plotting specline width
    QColor			m_cSpecLimitsColor;			// Line speccolor

    int				m_i2SigmaLineWidth;		// Plotting line width
    QColor			m_c2SigmaColor;			// Line color

    int				m_i3SigmaLineWidth;		// Plotting line width
    QColor			m_c3SigmaColor;			// Line color

    int				m_i6SigmaLineWidth;		// Plotting line width
    QColor			m_c6SigmaColor;			// Line color

    int				m_i12SigmaLineWidth;	// Plotting line width
    QColor			m_c12SigmaColor;		// Line color

    int             m_iRollingLimitsLineWidth;  // Plotting rolling line width

    int				m_iQuartileQ3LineWidth;
    QColor			m_cQuartileQ3Color;

    int				m_iQuartileQ1LineWidth;
    QColor			m_cQuartileQ1Color;

    /*!
     * \var mTextRotation
     * \brief in degrees
     */
    int mTextRotation;
};

class CGexChartOptions
{
public:
    CGexChartOptions();		// constructor.

    QString strChartTitle;	// Label for this chart
    bool	bChartTitle;	// true if ChartTitle is valid

    QString	strAxisLegendX;	// Legend for X axis.
    bool	bLegendX;		// 'true' if Legend for X axis is valid
    bool	bLogScaleX;		// 'true' if X scale to use Logarithmic resolution ('false' for linear scale)

    QString	strAxisLegendY;	// Legend for Y axis.
    bool	bLegendY;		// true if Legend for Y axis is valid
    bool	bLogScaleY;		// 'true' if X scale to use Logarithmic resolution ('false' for linear scale)

    double	lfLowX;			// Custom Viewport rectangle to focus on...
    double	lfLowY;
    double	lfHighX;
    double	lfHighY;


    bool  InitFromJSon(const QJsonObject& description);
    bool  ToJson(QJsonObject& descriptionOut) const;

};

class CGexChartOverlays
{
public:
    CGexChartOverlays(QWidget * lParent = 0);	// Constructor
    ~CGexChartOverlays();	// Destructir


    void CopySettings(const CGexChartOverlays* other);

    const QList<CGexSingleChart*> chartsList() const		{ return m_lstChartsList; }

    void            InitGlobal  (CReportOptions*	reportOptions);
    void			addChart    (CGexSingleChart * pChart);
    void			moveBack    (int nIndex);
    void			moveFront   (int nIndex);
    void			removeChart (int nIndex);
    void			removeCharts();

    /**
     * \fn bool Init(const QJsonObject& description);
     * \brief this function initialize the chart overlays with the description from the json file.
     * \param description: the description of the overlays in the json format.
     * \return true if the init has been done with success. Otherwise return false.
     */
    bool InitFromJSon(const QJsonObject& descriptionIn, int chartType);

    bool ToJson(QJsonObject& descriptionOut, int chartType) const;

    void			clear(void);		// Reset members

    QWidget *ptParent;

    QColor      mBackgroundColor;
    double      lfZoomFactorX;	// 1.0 (none) , 2.0 (x2), etc
    double      lfZoomFactorY;	// 1.0 (none) , 2.0 (x2), etc
    int         mTotalBars;
    int         mCustomTotalBars;

    bool        mLayerName;
    int         mTextRotation;
    bool        mQQLine;
    bool        mTitle;

    bool    mHistoMarkerMin ;
    bool    mHistoMarkerMax ;
    int     mYScale;
    bool    mCustom;

    class ViewportRectangle{
    public:
        // Viewport rectangle.
        double	lfLowX;
        double	lfLowY;
        double	lfHighX;
        double	lfHighY;
        bool	bForceViewport;
        bool    mChangeViewPort;
        CGexChartOptions			cChartOptions;			// Custom chart options (Title, legends, etc...)
        ViewportRectangle();
        ~ViewportRectangle();

        bool    InitFromJSon(const QJsonObject& descriptionIn);
        bool    ToJson      (QJsonObject& descriptionOut) const;

    };

    bool	bFileOutput;	// 'true' if Chart tobe plotted to data file.
    QPoint	pStart;			// Selection Starting point, value ranges in: 0-100% (offset from lower lef corner)
    QPoint	pEnd;			// Selection Ending point, value ranges in: 0-100% (offset from lower lef corner)

private:

    QList <CGexSingleChart*>	m_lstChartsList;		// List of charts to overlay.

protected:
    QMap<int , CGexChartOverlays::ViewportRectangle> m_oViewportRectangle;
    int m_iAppliedToChart;

public:
    QMap<int , CGexChartOverlays::ViewportRectangle> &getViewportRectangle() {
        return m_oViewportRectangle;
    }

    int getAppliedToChart() {
        return m_iAppliedToChart;
    }
    void setAppliedToChart(int iChart) {
        m_iAppliedToChart = iChart;
    }

};
#endif
