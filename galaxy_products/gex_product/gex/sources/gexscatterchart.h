#ifndef _GEX_SCATTER_CHART_H_
#define _GEX_SCATTER_CHART_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexabstractchart.h"

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexScatterChart
//
// Description	:	chart class to draw the scatter chart into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////
class GexScatterChart : public GexAbstractChart
{
    // Map containing linear regression results per couple of test.
    // Must be cleared each time the report is regenerated !
    // The possible keys are : Correlation, Determination, StdErrorEstimation, MinX, MinY, MaxX, MaxY, A,B, NumOfValidResults
    static QMap< QPair< CTest*,CTest* >, QMap<QString, QVariant> > sLinearRegressionResults;

public:
    GexScatterChart (int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays = NULL);
    ~GexScatterChart ();

    // Clear the cache (LinearRegression data,...)
    static void clearCache() { sLinearRegressionResults.clear(); }

    ///////////////////////////////////////////////////////////
    // Methods
    ///////////////////////////////////////////////////////////

    QString		makeTooltip();										// generate the label tooltip's
    void		setViewportModeFromChartMode(int nChartMode);		// convert chart mode from Gex into viewport mode
    void		resetViewportManager(bool bEraseCustomViewport);			// reset viewport values

    // Warning : for the linear regresssion library : y = bx + a !!!!
    static bool getLinearRegressionResults(CTest* lTX, CTest* lTY, float &a, float &b);
    static bool getLinearRegressionResults(CTest* lTX, CTest* lTY, QMap<QString, QVariant> &m);

protected:
    // custom drawing : regression line, diagonal, ...
    void customizeChart();

    // Build ChartDirector image
    void		buildOwnerChart();									// build the owner chart

    // compute the dataset : could be used to precompute and will be called each time X or Y is changed
    void		computeDataset();
    void		keepXTestViewport();
    void		keepYTestViewport();

    void		chartingLineStyle(CGexSingleChart * pLayerStyle);	// Update some characteristics
    bool		hasSpotsToChart(CGexSingleChart* /*pLayerStyle*/) { return true; }	// Always return true

    // test if we can use the custom viewport
    bool		useTestCustomViewport();

    QList <CTest *> m_listTestX,m_listTestY;
    QList <CGexFileInGroup *> m_listFileX,m_listFileY;
    bool        findRunId(const int &iIdx, QString &strRunX, QString &strRunY);

private:
    // only if not already computed and cached in QMap
    QString     computeLinearRegression(CTest *aTestX, CTest *aTestY);
    // draw trend line of the regression
    void        DrawTrendLine(const QMap<QString, QVariant> &aLinearReg, int nLineColor);
    bool        mIsLinearRegressionEnabled;  ///< True if linear regression is enabled

};

#endif // _GEX_SCATTER_CHART_H_
