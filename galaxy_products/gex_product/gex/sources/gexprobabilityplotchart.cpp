///////////////////////////////////////////////////////////////////////////////////
// GEX includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexprobabilityplotchart.h"
#include "interactive_charts.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "report_options.h"
#include <gqtl_global.h>

#include <vector>

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport * gexReport;
extern CReportOptions ReportOptions;

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern double				ScalingPower(int iPower);
extern double				approximationOrderStatistic(int nIndex, int nSize);
extern long double			normsinv(long double p);
extern	Chart::SymbolType	convertToChartDirectorSpot(int nSpotIndex);

///////////////////////////////////////////////////////////////////////////////////
// Class GexProbabilityPlotChart - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexProbabilityPlotChart::GexProbabilityPlotChart(int nSizeMode, GexWizardChart* lWizardParent, CGexChartOverlays * pChartOverlays /* = NULL */) : GexAbstractChart(chartTypeProbabilityPlot, nSizeMode, lWizardParent, pChartOverlays)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexProbabilityPlotChart::~GexProbabilityPlotChart()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	int visibleLayerCount() const
//
// Description	:	Get the number of layer visible
//
///////////////////////////////////////////////////////////////////////////////////
int GexProbabilityPlotChart::visibleLayerCount() const
{
    CGexSingleChart	*	pChart;					// Handle to Parameter Layer info.
    CGexGroupOfFiles *	pGroup;
    CGexFileInGroup *	pFile;
    CTest *				ptTestCell;				// Pointer to test cell of groups2 or higher
    long				lTestNumber;
    long				lPinmapIndex;
    int					nVisibleLayer = 0;

    if (m_pChartOverlays)
    {
        // Interactive mode: plot layers defined in the layer list
        for(int iLayerIndex = 0; iLayerIndex < m_pChartOverlays->chartsList().count(); iLayerIndex++)
        {
            pChart = m_pChartOverlays->chartsList().at(iLayerIndex);
            if(pChart->bVisible)
            {
                // Seek to the relevant Group to plot.
                if ((pChart->iGroupX < 0) || (pChart->iGroupX >= gexReport->getGroupsList().size()))
                    pGroup = NULL;
                else
                    pGroup = gexReport->getGroupsList().at(pChart->iGroupX);
                pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

                // Get test# & pinmap#
                lTestNumber		= pChart->iTestNumberX;
                lPinmapIndex	= pChart->iPinMapX;

                if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,pChart->strTestNameX)==1)
                {
                    // Interactive mode: if no samples for this test, ignore it!
                    if(ptTestCell->m_testResult.count() > 0)
                        nVisibleLayer++;
                }
            }
        }
    }
    else
    {
        QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());

        while (itGroupsList.hasNext())
        {
            // Find test cell: RESET list to ensure we scan list of the right group!
            // Check if pinmap index...
            pGroup			= itGroupsList.next();
            pFile			= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            lTestNumber		= referenceTestX()->lTestNumber;

            if(referenceTestX()->lPinmapIndex >= 0)
                lPinmapIndex = referenceTestX()->lPinmapIndex;
            else
                lPinmapIndex = GEX_PTEST;

            if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,referenceTestX()->strTestName)==1)
            {
                // Interactive mode: if no samples for this test, ignore it!
                if(ptTestCell->m_testResult.count() > 0)
                    nVisibleLayer++;
            }
        }
    }

    return nVisibleLayer;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void fillDataset(CGexProbabilityPlotData& dataSet, CTest * ptTestCell)
//
// Description	:	fill the probability plot dataset
//
///////////////////////////////////////////////////////////////////////////////////
void GexProbabilityPlotChart::fillDataset(CGexProbabilityPlotData& dataSet,
                                          CTest * ptTestCell)
{
    QVector<double>	vecResults;

    vecResults.reserve(ptTestCell->ldSamplesValidExecs);

    //	if (ptTestCell->m_testResult.count() > 0)
    //	{
    //for(int i = 0; i <ptTestCell->ldSamplesExecs; i++)

    // Extract valid data samples...
    for(int i = 0; i <ptTestCell->m_testResult.count(); i++)
    {
        if (ptTestCell->m_testResult.isValidResultAt(i))
        {
            if (ptTestCell->m_testResult.isMultiResultAt(i))
                for (int nCount = 0; nCount < ptTestCell->m_testResult.at(i)->count(); ++nCount){
                    if(ptTestCell->m_testResult.at(i)->isValidResultAt(nCount))
                        vecResults.append(ptTestCell->m_testResult.at(i)->multiResultAt(nCount));
                }
            else
                vecResults.append(ptTestCell->m_testResult.resultAt(i));

        }
    }

    //}

    //-- no result for this test, no need to go any futher
    if(vecResults.isEmpty())
        return;

    // Order data samples
    vecResults.squeeze();
    qSort(vecResults);

    // Initialize the arrays which store probability plot data
    dataSet.init(vecResults.count(), ptTestCell->lTestNumber, ptTestCell->res_scal);

    // Compute data
    double dPercentRank = 0;

    for (int nValue = 0; nValue < vecResults.size(); nValue++)
    {
        dPercentRank = approximationOrderStatistic(nValue+1, vecResults.count());
        dataSet.addData(nValue, vecResults.at(nValue), normsinv(dPercentRank));
    }

    double lQ1 = -0.6744898;  // qnorm(0.25), -0,674490 with normsinv(0.25)
    double lQ3 = 0.6744898;  //  qnorm(0.75),  0,674490 with normsinv(0.75)
    double lA = (vecResults.count() - 1) * 0.25;
    double lB = (vecResults.count() - 1) * 0.75;
    int lA1 = floor(lA);
    int lA2 = ceil(lA);
    int lB1 = floor(lB);
    int lB2 = ceil(lB);
    double lX1 =
        (1 - (lA - lA1)) * vecResults.at(lA1) + (lA - lA1) * vecResults.at(lA2);
    double lX2 =
        (1 - (lB - lB1)) * vecResults.at(lB1) + (lB - lB1) * vecResults.at(lB2);
    double lSlope =  (lQ3 - lQ1) / (lX2 - lX1);
    dataSet.SetLineToTheoreticalNormalQQPlotInter(lQ3 - lSlope * lX2);
    dataSet.SetLineToTheoreticalNormalQQPlotSlope(lSlope);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void computeDataset()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexProbabilityPlotChart::computeDataset()
{
    // Count the total number of VISIBLE layers
    CGexGroupOfFiles *	pGroup;
    CGexFileInGroup *	pFile;
    CTest *				ptTestCell;				// Pointer to test cell of groups2 or higher
    long				lTestNumber;
    long				lPinmapIndex;

    if (visibleLayerCount() == 0)
        return;

    if (m_pChartOverlays)
    {
        CGexSingleChart	*	pChart;					// Handle to Parameter Layer info.

        for(int iLayerIndex=0; iLayerIndex < m_pChartOverlays->chartsList().count(); iLayerIndex++)
        {
            pChart = m_pChartOverlays->chartsList().at(iLayerIndex);

            // Skip hidden layers and layer already computed!
            if(pChart->bVisible && !pChart->dataProbabilityPlot().isReady(pChart->iTestNumberX))
            {
                // Seek to the relevant Group to plot.
                if ((pChart->iGroupX < 0) || (pChart->iGroupX >= gexReport->getGroupsList().size()))
                    pGroup = NULL;
                else
                    pGroup = gexReport->getGroupsList().at(pChart->iGroupX);

                if(pGroup)
                {

                    pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

                    if(pFile)
                    {
                        // Get test# & pinmap#
                        lTestNumber		= pChart->iTestNumberX;
                        lPinmapIndex	= pChart->iPinMapX;

                        if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,pChart->strTestNameX) ==1)
                        {
                            fillDataset(pChart->dataProbabilityPlot(), ptTestCell);
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Clear previous dataset
        m_lstProbabilityPlotDataset.clear();

        QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());

        while (itGroupsList.hasNext())
        {
            // Find test cell: RESET list to ensure we scan list of the right group!
            // Check if pinmap index...
            pGroup			= itGroupsList.next();
            pFile			= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            lTestNumber		= referenceTestX()->lTestNumber;

            if(referenceTestX()->lPinmapIndex >= 0)
                lPinmapIndex = referenceTestX()->lPinmapIndex;
            else
                lPinmapIndex = GEX_PTEST;

            if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,referenceTestX()->strTestName)==1)
            {
                CGexProbabilityPlotData dataSet;

                // Fill the dataset
                fillDataset(dataSet, ptTestCell);

                // add the dataset to the list
                m_lstProbabilityPlotDataset.append(dataSet);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void buildOwnerChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexProbabilityPlotChart::buildOwnerChart()
{
    // Count the total number of VISIBLE layers
    CGexGroupOfFiles *		pGroup;
    CGexFileInGroup *		pFile;
    CTest *					ptTestCell	= NULL;				// Pointer to test cell of groups2 or higher
    long					lTestNumber;
    long					lPinmapIndex;
    GexInternalChartInfo	internalChartInfo;

    if (m_pChartOverlays)
    {
        for (internalChartInfo.m_nLayerIndex = 0; internalChartInfo.m_nLayerIndex < m_pChartOverlays->chartsList().count(); internalChartInfo.m_nLayerIndex++)
        {
            // Interactive mode: plot layers defined in the layer list
            internalChartInfo.m_pChart = m_pChartOverlays->chartsList().at(internalChartInfo.m_nLayerIndex);

            // Skip layers without data or hidden
            if (internalChartInfo.m_pChart->bVisible)
            {
                // Seek to the relevant Group to plot.
                if ((internalChartInfo.m_pChart->iGroupX < 0) || (internalChartInfo.m_pChart->iGroupX >= gexReport->getGroupsList().size()))
                    pGroup = NULL;
                else
                    pGroup	= gexReport->getGroupsList().at(internalChartInfo.m_pChart->iGroupX);

                if(pGroup)
                {
                    pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

                    if(pFile)
                    {

                        // Get test# & pinmap#
                        lTestNumber		= internalChartInfo.m_pChart->iTestNumberX;
                        lPinmapIndex	= internalChartInfo.m_pChart->iPinMapX;

                        if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,internalChartInfo.m_pChart->strTestNameX) ==1)
                        {
                            if (fillChart(pFile, ptTestCell, internalChartInfo, internalChartInfo.m_pChart->dataProbabilityPlot()))
                                internalChartInfo.m_nVisibleLayerIndex++;
                        }
                    }
                }
            }
        }
    }
    else
    {
        QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());

        internalChartInfo.m_nLayerIndex = 0;

        while (itGroupsList.hasNext())
        {
            // Find test cell: RESET list to ensure we scan list of the right group!
            // Check if pinmap index...
            pGroup			= itGroupsList.next();
            pFile			= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            lTestNumber		= referenceTestX()->lTestNumber;

            if(referenceTestX()->lPinmapIndex >= 0)
                lPinmapIndex = referenceTestX()->lPinmapIndex;
            else
                lPinmapIndex = GEX_PTEST;

            if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,referenceTestX()->strTestName)==1)
            {
                if (fillChart(pFile, ptTestCell, internalChartInfo, m_lstProbabilityPlotDataset[internalChartInfo.m_nLayerIndex]))
                    internalChartInfo.m_nVisibleLayerIndex++;
            }

            internalChartInfo.m_nGroup++;
            internalChartInfo.m_nLayerIndex++;
        }
    }

    if (internalChartInfo.m_bTestReferenceScaleFactor == false)
        internalChartInfo.m_strTitle += "<*color=FF0000*> - No data samples<*/color*>";

    m_pXYChart->addTitle(internalChartInfo.m_strTitle.toLatin1().constData());

    if (internalChartInfo.m_nVisibleLayerIndex == 0)
    {
        internalChartInfo.m_dLowX	= 0;
        internalChartInfo.m_dHighX	= 0;
        internalChartInfo.m_dLowY	= 0;
        internalChartInfo.m_dHighY	= 0;
    }

    //internalChartInfo.m_dLowX, internalChartInfo.m_dHighX
    double dDiff = internalChartInfo.m_dHighX - OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        internalChartInfo.m_dHighX = ptTestCell->lfSamplesMax;
    }
    dDiff = internalChartInfo.m_dLowX + OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        internalChartInfo.m_dLowX = ptTestCell->lfSamplesMin;
    }

    // Viewport: add 1% extra space at each end
    // Fit the scales to min/ max value + 1%
    double dYExtra	= (internalChartInfo.m_dHighY - internalChartInfo.m_dLowY) * 0.01;
    double dXExtra	= (internalChartInfo.m_dHighX - internalChartInfo.m_dLowX) * 0.01;

    // If custom viewport (in scripting file, not in interactive mode), overwrite defaults window viewing
    if(useTestCustomViewport())
    {
        // reset the current viewport
        resetViewportManager(false);

        // If custom viewport in X, overwrite defaults window viewing
        if(referenceTestX()->ptChartOptions->customViewportX())
        {
            internalChartInfo.m_dLowX	= referenceTestX()->ptChartOptions->lowX();
            internalChartInfo.m_dHighX	= referenceTestX()->ptChartOptions->highX();
            dXExtra						= 0;
        }
    }
    else if (m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].bForceViewport)
    {
        // Customer force viewport limit
        resetViewportManager(false);

        if (m_pChartOverlays->getViewportRectangle()[type()].lfLowX != -C_INFINITE)
            internalChartInfo.m_dLowX	= m_pChartOverlays->getViewportRectangle()[type()].lfLowX;

        if (m_pChartOverlays->getViewportRectangle()[type()].lfHighX != C_INFINITE)
            internalChartInfo.m_dHighX	= m_pChartOverlays->getViewportRectangle()[type()].lfHighX;

        if (m_pChartOverlays->getViewportRectangle()[type()].lfLowY != -C_INFINITE)
            internalChartInfo.m_dLowY	= m_pChartOverlays->getViewportRectangle()[type()].lfLowY;

        if (m_pChartOverlays->getViewportRectangle()[type()].lfHighY != C_INFINITE)
            internalChartInfo.m_dHighY	= m_pChartOverlays->getViewportRectangle()[type()].lfHighY;

        dXExtra						= 0;
        dYExtra						= 0;

        if( m_pChartOverlays->getViewportRectangle()[type()].bForceViewport)
            m_pChartOverlays->getViewportRectangle()[type()].mChangeViewPort = true;

        m_pChartOverlays->getViewportRectangle()[type()].bForceViewport = false;
    }


    if (internalChartInfo.m_dLowY != C_INFINITE && internalChartInfo.m_dHighY != -C_INFINITE)
    {
        internalChartInfo.m_dHighY	+= dYExtra;
        internalChartInfo.m_dLowY	-= dYExtra;
    }

    if (internalChartInfo.m_dLowX != C_INFINITE && internalChartInfo.m_dHighX != -C_INFINITE)
    {
        internalChartInfo.m_dHighX	+= dXExtra;
        internalChartInfo.m_dLowX	-= dXExtra;
    }

    internalChartInfo.m_dViewportLowX	= gex_min(internalChartInfo.m_dViewportLowX, internalChartInfo.m_dLowX);
    internalChartInfo.m_dViewportHighX	= gex_max(internalChartInfo.m_dViewportHighX, internalChartInfo.m_dHighX);

    dDiff = internalChartInfo.m_dViewportHighX - OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        internalChartInfo.m_dViewportHighX = ptTestCell->lfSamplesMax;
    }
    dDiff = internalChartInfo.m_dViewportLowX + OLD_CHART_LIMIT_INFINITE ;
    if (dDiff == 0.0) {
        internalChartInfo.m_dViewportLowX = ptTestCell->lfSamplesMin;
    }

    // Define scales: if in Interactive charting, consider zoomin factor
    if(m_pChartOverlays && internalChartInfo.m_dViewportLowX != C_INFINITE && internalChartInfo.m_dViewportHighX != -C_INFINITE)
    {
        // Initialize viewport
        if (isViewportInitialized() == false)
        {
            initHorizontalViewport(internalChartInfo.m_dViewportLowX, internalChartInfo.m_dViewportHighX, internalChartInfo.m_dLowX, internalChartInfo.m_dHighX, 0.05);
            initVerticalViewport(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dViewportHighY, internalChartInfo.m_dLowY, internalChartInfo.m_dHighY, 0.05);
        }

        // Using ViewPortLeft and ViewPortWidth, get the start and end dates of the view port.
        computeXBounds(internalChartInfo.m_dLowX, internalChartInfo.m_dHighX);
        computeYBounds(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY);
    }

    if (internalChartInfo.m_dLowY != C_INFINITE && internalChartInfo.m_dHighY != -C_INFINITE
        && internalChartInfo.m_dLowX != C_INFINITE && internalChartInfo.m_dHighX != -C_INFINITE)
    {
        // If percentage Y scale, disable auto scale and build our own custom scale
        QString pbya=m_pReportOptions->GetOption("adv_probabilityplot","y_axis").toString();
        if (pbya=="percentage") //if(m_pReportOptions->bProbPlotYAxisSigma == false)
        {
            if ((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
                 (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeProbabilityPlot || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 1 ){
                m_pXYChart->yAxis()->setLogScale(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY, Chart::NoValue);
                m_iYScaleType = 1;
            } else if((m_pChartOverlays && !m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
                   (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeProbabilityPlot || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 0) {
                m_pXYChart->yAxis()->setLinearScale(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY, Chart::NoValue);
                m_iYScaleType = 0;
            }
            m_pXYChart->yAxis()->addLabel(normsinv(0.001),	"0.001");
            m_pXYChart->yAxis()->addLabel(normsinv(0.01),	"0.01");
            m_pXYChart->yAxis()->addLabel(normsinv(0.05),	"0.05");
            m_pXYChart->yAxis()->addLabel(normsinv(0.1),	"0.10");
            m_pXYChart->yAxis()->addLabel(normsinv(0.25),	"0.25");
            m_pXYChart->yAxis()->addLabel(normsinv(0.5),	"0.5");
            m_pXYChart->yAxis()->addLabel(normsinv(0.75),	"0.75");
            m_pXYChart->yAxis()->addLabel(normsinv(0.9),	"0.90");
            m_pXYChart->yAxis()->addLabel(normsinv(0.95),	"0.95");
            m_pXYChart->yAxis()->addLabel(normsinv(0.99),	"0.99");
            m_pXYChart->yAxis()->addLabel(normsinv(0.999),	"0.999");
        }
        else
        {

            if ((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY
                 && ( m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeProbabilityPlot || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 1){
                m_pXYChart->yAxis()->setLogScale(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY);
                m_iYScaleType = 1;
            } else if ((m_pChartOverlays && !m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY
                    && ( m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeProbabilityPlot || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 0) {
                m_pXYChart->yAxis()->setLinearScale(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY);
                m_iYScaleType = 0;
            }
        }

        if ((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX &&
                (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeProbabilityPlot || m_pChartOverlays->getAppliedToChart() == -1)) || m_iXScaleType == 1){
                m_pXYChart->xAxis()->setLogScale(internalChartInfo.m_dLowX, internalChartInfo.m_dHighX);
                m_iXScaleType = 1;
            } else if ((m_pChartOverlays && !m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX &&
                (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeProbabilityPlot || m_pChartOverlays->getAppliedToChart() == -1)) || m_iXScaleType == 0){
                m_pXYChart->xAxis()->setLinearScale(internalChartInfo.m_dLowX, internalChartInfo.m_dHighX);
                m_iXScaleType = 0;
        }
    }

    if (m_pReportOptions->bPlotLegend)
        m_pXYChart->addLegend(75, 30)->setBackground(0x80000000 | (QColor(Qt::white).rgb() & 0xffffff));

    if (m_pChartOverlays)
    {
        m_pChartOverlays->getViewportRectangle()[type()].lfLowX	= internalChartInfo.m_dLowX;
        m_pChartOverlays->getViewportRectangle()[type()].lfLowY	= internalChartInfo.m_dLowY;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighX	= internalChartInfo.m_dHighX;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighY	= internalChartInfo.m_dHighY;
    }

}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void fillChart(CGexFileInGroup * pFile, CTest * ptTestCell, GexInternalChartInfo& internalChartInfo, CGexProbabilityPlotData& dataSet)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexProbabilityPlotChart::fillChart(CGexFileInGroup * pFile, CTest * ptTestCell, GexInternalChartInfo& internalChartInfo, CGexProbabilityPlotData &dataSet)
{
    double	dCustomScaleFactor;
    double	lfLowSpecL			= C_INFINITE;
    double	lfHighSpecL			= -C_INFINITE;
    int		iColor;
    int		iPenWidth;
    bool	bIsMultiLayer		= isMultiLayers();
    double	lfQuartileQ1=-C_INFINITE;
    double	lfQuartileQ3=-C_INFINITE;
    bool    lShowTitle  = false;


    setTopMargin(50);

    //	OPTIONS
    QString scaling=m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    QString strAdvProbabilityplotMarkerOptions = (m_pReportOptions->GetOption(QString("adv_probabilityplot"), QString("marker"))).toString();
    QStringList qslAdvProbabilityPlotMarkerOptionsList = strAdvProbabilityplotMarkerOptions.split(QString("|"));

    if ((m_pChartOverlays && m_pChartOverlays->mTitle == true) ||
        qslAdvProbabilityPlotMarkerOptionsList.contains(QString("test_name")))
    {
        lShowTitle = true;
    }

    // First layer only,
    // Build the title, define the axis labels and add some markers like Limits
    if (internalChartInfo.m_nVisibleLayerIndex == 0)
    {
        char	szTestName[2*GEX_MAX_STRING];
        QString strLabel;

        // Check if we have a custom title to overwrite default one
        if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bChartTitle){
            internalChartInfo.m_strTitle =  gexReport->buildDisplayName(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strChartTitle, false);	// custom title.
        }
        else if (lShowTitle)
        {
            if(bIsMultiLayer && (isSingleTestAllLayers() == false))
                // In multi-layer mode and with multiple layers shown, title is generic...unless all layers refer to same test#!
                internalChartInfo.m_strTitle = "Overlay Tests/Parameters";
            else
            {
                gexReport->BuildTestNameString(pFile, ptTestCell, szTestName);
                QString strNormalizedName = gexReport->buildDisplayName(szTestName, false);
                internalChartInfo.m_strTitle = "Test ";
                internalChartInfo.m_strTitle += ptTestCell->szTestLabel;
                internalChartInfo.m_strTitle += " : ";
                internalChartInfo.m_strTitle += strNormalizedName;
            }
        }

        if (ptTestCell->m_testResult.count() == 0)
            return false;

        // Check if we have a custom legend to overwrite default one
        if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLegendY)
            strLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strAxisLegendY;
        else
        {
            // Set Y label
            QString pbya=m_pReportOptions->GetOption("adv_probabilityplot","y_axis").toString();
            if (pbya=="sigma")	//if(m_pReportOptions->bProbPlotYAxisSigma)
                strLabel = "Z (Sigma)";
            else
                strLabel = "Cumulative Probability";

            if (m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY)
                strLabel += " - LogScale";
        }

        m_pXYChart->yAxis()->setTitle(strLabel.toLatin1().constData());

        // Check if we have a custom legend to overwrite default one
        if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLegendX)
            strLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strAxisLegendX;
        else
        {
            // Set X scale label (units)
            strLabel = "Test results ";
            QString strUnits = ptTestCell->GetScaledUnits(&dCustomScaleFactor, scaling);

            if(strUnits.length() > 0)
                strLabel += "(" + strUnits + ")";

            if (m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX)
                strLabel += " - LogScale";
        }

        m_pXYChart->xAxis()->setTitle(strLabel.toLatin1().constData());
    }

    if (internalChartInfo.m_bTestReferenceScaleFactor == false)
    {
        internalChartInfo.m_nTestReferenceScaleFactor = ptTestCell->res_scal;
        internalChartInfo.m_bTestReferenceScaleFactor = true;
    }

    // No data available for this test
    if (dataSet.isReady(ptTestCell->lTestNumber))
    {
        int iCustomScaleFactor = ptTestCell->res_scal - internalChartInfo.m_nTestReferenceScaleFactor;
        dCustomScaleFactor = 1/GS_POW(10.0,iCustomScaleFactor);

        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
        {
            // Draw LL limit markers (done only once: when charting Plot for group#1)
            double lfLowL = ptTestCell->GetCurrentLimitItem()->lfLowLimit;		// Low limit exists
            // If we have to keep values in normalized format, do not rescale!
            if (scaling!="normalized")
            {
                // convert LowLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfLowL,ptTestCell->llm_scal);
                lfLowL *=  ScalingPower(ptTestCell->llm_scal);	// normalized
                lfLowL /=  ScalingPower(ptTestCell->res_scal);	// normalized
            }
            if(iCustomScaleFactor)
                lfLowL *= dCustomScaleFactor;

            // Update global lowest limit for chart viewport
            internalChartInfo.m_dLowLimit = gex_min(internalChartInfo.m_dLowLimit, lfLowL);

            if ( (qslAdvProbabilityPlotMarkerOptionsList.contains(QString("limits"))))
            {
                if (internalChartInfo.m_pChart)
                {
                    iColor		= internalChartInfo.m_pChart->limitsColor(bIsMultiLayer).rgb() & 0xffffff;
                    iPenWidth	= internalChartInfo.m_pChart->limitsLineWidth();
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart	*pLayerStyle = NULL;
                    if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                        iColor		= pLayerStyle->limitsColor(bIsMultiLayer).rgb()  & 0xffffff;
                        iPenWidth	= pLayerStyle->limitsLineWidth();

                        if(!iPenWidth)
                            iPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                    }
                    else
                    {
                        iColor		= QColor(Qt::red).rgb() & 0xffffff;
                        iPenWidth	= 1;
                    }
                }

                // Requested to show limits.
                if(iPenWidth)
                    addMarker(m_pXYChart->xAxis(), lfLowL, iColor, "Low L.", iPenWidth, Chart::TopRight);
            }
        }

        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
        {
            // Draw HL limit markers (done only once: when charting Plot for group#1)
            double lfHighL = ptTestCell->GetCurrentLimitItem()->lfHighLimit;		// High limit exists
            // If we have to keep values in normalized format, do not rescale!
            if (scaling!="normalized")
            {
                // convert HighLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfHighL,ptTestCell->hlm_scal);

                lfHighL *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
                lfHighL /=  ScalingPower(ptTestCell->res_scal);	// normalized
            }
            if(iCustomScaleFactor)
                lfHighL *= dCustomScaleFactor;

            // Update global highest limit for chart viewport
            internalChartInfo.m_dHighLimit = gex_max(internalChartInfo.m_dHighLimit, lfHighL);

            if ( (qslAdvProbabilityPlotMarkerOptionsList.contains(QString("limits"))))
            {
                if (internalChartInfo.m_pChart)
                {
                    iColor		= internalChartInfo.m_pChart->limitsColor(bIsMultiLayer).rgb() & 0xffffff;
                    iPenWidth	= internalChartInfo.m_pChart->limitsLineWidth();
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart	*pLayerStyle = NULL;
                    if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                        iColor		= pLayerStyle->limitsColor(bIsMultiLayer).rgb()  & 0xffffff;
                        iPenWidth	= pLayerStyle->limitsLineWidth();

                        if(!iPenWidth)
                            iPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                    }
                    else
                    {
                        iColor		= QColor(Qt::red).rgb() & 0xffffff;
                        iPenWidth	= 1;
                    }
                }

                // Request to show limits
                if(iPenWidth)
                    addMarker(m_pXYChart->xAxis(), lfHighL, iColor, "High L.", iPenWidth, Chart::TopLeft);
            }
        }

        // Draw multi-limit markers.
        addMultiLimitMarkers(internalChartInfo.m_pChart, ptTestCell, internalChartInfo);

        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
        {
            // Draw LL limit markers (done only once: when charting Plot for group#1)
            lfLowSpecL = ptTestCell->lfLowSpecLimit;		// Low Spec limit exists

            // If we have to keep values in normalized format, do not rescale!
            if (scaling!="normalized")
            {
                // convert LowLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfLowSpecL,ptTestCell->llm_scal);
                lfLowSpecL *=  ScalingPower(ptTestCell->llm_scal);	// normalized
                lfLowSpecL /=  ScalingPower(ptTestCell->res_scal);	// normalized
            }
            if(iCustomScaleFactor)
                lfLowSpecL *= dCustomScaleFactor;

            internalChartInfo.m_dLowSpecLimit = gex_min(internalChartInfo.m_dLowSpecLimit, lfLowSpecL);

            if ( (qslAdvProbabilityPlotMarkerOptionsList.contains(QString("speclimits"))) )
            {
                if (internalChartInfo.m_pChart)
                {
                    iColor		= internalChartInfo.m_pChart->limitsColor(bIsMultiLayer).rgb() & 0xffffff;
                    iPenWidth	= internalChartInfo.m_pChart->limitsLineWidth();
                    if(!iPenWidth)
                        iPenWidth = 1;
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart	*pLayerStyle = NULL;
                    if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                        iColor		= pLayerStyle->limitsColor(bIsMultiLayer).rgb()  & 0xffffff;
                        iPenWidth	= pLayerStyle->limitsLineWidth();

                        if(!iPenWidth)
                            iPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                    }
                    else
                    {
                        iColor		= QColor(Qt::red).rgb() & 0xffffff;
                        iPenWidth	= 1;
                    }
                }

                // Requested to show limits.
                if(iPenWidth)
                    addMarker(m_pXYChart->xAxis(), lfLowSpecL, iColor, m_strLowSpecLimit, iPenWidth, Chart::TopRight);
            }
        }

        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
        {
            // Draw HL limit markers (done only once: when charting Plot for group#1)
            lfHighSpecL = ptTestCell->lfHighSpecLimit;		// High Speclimit exists

            // If we have to keep values in normalized format, do not rescale!
            if (scaling!="normalized")
            {
                // convert HighLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfHighSpecL,ptTestCell->hlm_scal);

                lfHighSpecL *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
                lfHighSpecL /=  ScalingPower(ptTestCell->res_scal);	// normalized
            }
            if(iCustomScaleFactor)
                lfHighSpecL *= dCustomScaleFactor;

            internalChartInfo.m_dHighSpecLimit = gex_max(internalChartInfo.m_dHighSpecLimit, lfHighSpecL);

            if ( (qslAdvProbabilityPlotMarkerOptionsList.contains(QString("speclimits"))) )
            {
                if (internalChartInfo.m_pChart)
                {
                    iColor		= internalChartInfo.m_pChart->limitsColor(bIsMultiLayer).rgb() & 0xffffff;
                    iPenWidth	= internalChartInfo.m_pChart->limitsLineWidth();
                    if(!iPenWidth)
                        iPenWidth = 1;
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart	*pLayerStyle = NULL;
                    if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                        iColor		= pLayerStyle->limitsColor(bIsMultiLayer).rgb()  & 0xffffff;
                        iPenWidth	= pLayerStyle->limitsLineWidth();

                        if(!iPenWidth)
                            iPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                    }
                    else
                    {
                        iColor		= QColor(Qt::red).rgb() & 0xffffff;
                        iPenWidth	= 1;
                    }
                }

                // Request to show limits
                if(iPenWidth)
                    addMarker(m_pXYChart->xAxis(), lfHighSpecL, iColor, m_strHighSpecLimit, iPenWidth, Chart::TopLeft);
            }
        }

        // Compute Min (and scale to result scale)
        double lfMin = ptTestCell->lfSamplesMin * dCustomScaleFactor;
        pFile->FormatTestResultNoUnits(&lfMin, ptTestCell->res_scal);

        // Compute Max (and scale to result scale)
        double lfMax = ptTestCell->lfSamplesMax * dCustomScaleFactor;
        pFile->FormatTestResultNoUnits(&lfMax, ptTestCell->res_scal);

        // Compute Mean
        double lfMean = dCustomScaleFactor * ptTestCell->lfMean;
        pFile->FormatTestResultNoUnits(&lfMean, ptTestCell->res_scal);

        // Compute Median
        double lfMedian = -C_INFINITE;
        if (ptTestCell->lfSamplesQuartile2 != -C_INFINITE)
        {
            lfMedian = dCustomScaleFactor * ptTestCell->lfSamplesQuartile2;
            pFile->FormatTestResultNoUnits(&lfMedian, ptTestCell->res_scal);
        }

        if (internalChartInfo.m_nVisibleLayerIndex == 0 &&
            ptTestCell->ldSamplesValidExecs)
        {
            // Compute Sigma
            double lfSigma = dCustomScaleFactor * ptTestCell->lfSigma;
            pFile->FormatTestResultNoUnits(&lfSigma, ptTestCell->res_scal);

            // If request to show the 2sigma space
            if ((qslAdvProbabilityPlotMarkerOptionsList.
                 contains(QString("2sigma"))))
            {
                if (internalChartInfo.m_pChart)
                {
                    iColor = internalChartInfo.m_pChart->
                        sigma2Color(bIsMultiLayer).rgb() & 0xffffff;
                    iPenWidth = internalChartInfo.m_pChart->sigma2LineWidth();
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart* pLayerStyle = NULL;
                    if ((internalChartInfo.m_nGroup - 1) >= 0 &&
                        (internalChartInfo.m_nGroup - 1) <
                        m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.
                            at(internalChartInfo.m_nGroup - 1);
                        iColor = pLayerStyle->sigma2Color(bIsMultiLayer).rgb() &
                            0xffffff;
                        iPenWidth = pLayerStyle->sigma2LineWidth();

                        if (! iPenWidth)
                        {
                            // Just in case line width not defined under
                            // interactive chart, then ensure we still show
                            // line in image
                            iPenWidth = 1;
                        }
                    }
                    else
                    {
                        iColor = QColor(Qt::red).rgb() & 0xffffff;
                        iPenWidth = 1;
                    }
                }

                // Request to show limits
                if (iPenWidth)
                {
                    // Add marker -1 sigma
                    addMarker(m_pXYChart->xAxis(), lfMean - lfSigma,
                              iColor, "-1s", iPenWidth, Chart::TopRight);

                    // Add marker +1 sigma
                    addMarker(m_pXYChart->xAxis(), lfMean + lfSigma,
                              iColor, "+1s", iPenWidth, Chart::TopLeft);

                    internalChartInfo.m_dMarkerMin =
                        gex_min(internalChartInfo.m_dMarkerMin,
                                lfMean - lfSigma);
                    internalChartInfo.m_dMarkerMax =
                        gex_max(internalChartInfo.m_dMarkerMax,
                                lfMean + lfSigma);
                }
            }

            // If request to show the 3sigma space
            if ((qslAdvProbabilityPlotMarkerOptionsList.
                 contains(QString("3sigma"))))
            {
                if (internalChartInfo.m_pChart)
                {
                    iColor = internalChartInfo.m_pChart->
                        sigma3Color(bIsMultiLayer).rgb() & 0xffffff;
                    iPenWidth = internalChartInfo.m_pChart->sigma3LineWidth();
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart* pLayerStyle = NULL;
                    if ((internalChartInfo.m_nGroup - 1) >= 0 &&
                        (internalChartInfo.m_nGroup - 1) <
                        m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.
                            at(internalChartInfo.m_nGroup - 1);
                        iColor = pLayerStyle->sigma3Color(bIsMultiLayer).rgb() &
                            0xffffff;
                        iPenWidth = pLayerStyle->sigma3LineWidth();

                        if (! iPenWidth)
                        {
                            // Just in case line width not defined under
                            // interactive chart, then ensure we still show
                            // line in image
                            iPenWidth = 1;
                        }
                    }
                    else
                    {
                        iColor = QColor(Qt::red).rgb() & 0xffffff;
                        iPenWidth = 1;
                    }
                }

                // Request to show limits
                if (iPenWidth)
                {
                    // Add marker -1.5 sigma
                    addMarker(m_pXYChart->xAxis(), lfMean - 1.5 * lfSigma,
                              iColor, "-1.5s", iPenWidth, Chart::TopRight);

                    // Add marker +1.5 sigma
                    addMarker(m_pXYChart->xAxis(), lfMean + 1.5 * lfSigma,
                              iColor, "+1.5s", iPenWidth, Chart::TopLeft);

                    internalChartInfo.m_dMarkerMin =
                        gex_min(internalChartInfo.m_dMarkerMin,
                                lfMean - 1.5 * lfSigma);
                    internalChartInfo.m_dMarkerMax =
                        gex_max(internalChartInfo.m_dMarkerMax,
                                lfMean + 1.5 * lfSigma);
                }
            }

            // If request to show the 6sigma space
            if ((qslAdvProbabilityPlotMarkerOptionsList.
                 contains(QString("6sigma"))))
            {
                if (internalChartInfo.m_pChart)
                {
                    iColor = internalChartInfo.m_pChart->
                        sigma6Color(bIsMultiLayer).rgb() & 0xffffff;
                    iPenWidth = internalChartInfo.m_pChart->sigma6LineWidth();
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart* pLayerStyle = NULL;
                    if ((internalChartInfo.m_nGroup - 1) >= 0 &&
                        (internalChartInfo.m_nGroup - 1) <
                        m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.
                            at(internalChartInfo.m_nGroup - 1);
                        iColor = pLayerStyle->sigma6Color(bIsMultiLayer).rgb() &
                            0xffffff;
                        iPenWidth = pLayerStyle->sigma6LineWidth();

                        if (! iPenWidth)
                        {
                            // Just in case line width not defined under
                            // interactive chart, then ensure we still show
                            // line in image
                            iPenWidth = 1;
                        }
                    }
                    else
                    {
                        iColor = QColor(Qt::red).rgb() & 0xffffff;
                        iPenWidth = 1;
                    }
                }

                // Request to show limits
                if (iPenWidth)
                {
                    // Add marker -3 sigma
                    addMarker(m_pXYChart->xAxis(), lfMean - 3 * lfSigma,
                              iColor, "-3s", iPenWidth, Chart::TopRight);

                    // Add marker +3 sigma
                    addMarker(m_pXYChart->xAxis(), lfMean + 3 * lfSigma,
                              iColor, "+3s", iPenWidth, Chart::TopLeft);

                    internalChartInfo.m_dMarkerMin =
                        gex_min(internalChartInfo.m_dMarkerMin,
                                lfMean - 3 * lfSigma);
                    internalChartInfo.m_dMarkerMax =
                        gex_max(internalChartInfo.m_dMarkerMax,
                                lfMean + 3 * lfSigma);
                }
            }

            // If request to show the 12sigma space
            if ((qslAdvProbabilityPlotMarkerOptionsList.
                 contains(QString("12sigma"))))
            {
                if (internalChartInfo.m_pChart)
                {
                    iColor = internalChartInfo.m_pChart->
                        sigma12Color(bIsMultiLayer).rgb() & 0xffffff;
                    iPenWidth = internalChartInfo.m_pChart->sigma12LineWidth();
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart* pLayerStyle = NULL;
                    if ((internalChartInfo.m_nGroup - 1) >= 0 &&
                        (internalChartInfo.m_nGroup - 1) <
                        m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.
                            at(internalChartInfo.m_nGroup - 1);
                        iColor = pLayerStyle->
                            sigma12Color(bIsMultiLayer).rgb() & 0xffffff;
                        iPenWidth = pLayerStyle->sigma12LineWidth();

                        if (! iPenWidth)
                        {
                            // Just in case line width not defined under
                            // interactive chart, then ensure we still show
                            // line in image
                            iPenWidth = 1;
                        }
                    }
                    else
                    {
                        iColor = QColor(Qt::red).rgb() & 0xffffff;
                        iPenWidth = 1;
                    }
                }

                // Request to show limits
                if (iPenWidth)
                {
                    // Add marker -6 sigma
                    addMarker(m_pXYChart->xAxis(), lfMean - 6 * lfSigma,
                              iColor, "-6s", iPenWidth, Chart::TopRight);

                    // Add marker +6 sigma
                    addMarker(m_pXYChart->xAxis(), lfMean + 6 * lfSigma,
                              iColor, "+6s", iPenWidth, Chart::TopLeft);

                    internalChartInfo.m_dMarkerMin =
                        gex_min(internalChartInfo.m_dMarkerMin,
                                lfMean - 6 * lfSigma);
                    internalChartInfo.m_dMarkerMax =
                        gex_max(internalChartInfo.m_dMarkerMax,
                                lfMean + 6 * lfSigma);
                }
            }
        }

        // If request to show the mean
        if ( (qslAdvProbabilityPlotMarkerOptionsList.contains(QString("mean"))))
        {
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->meanColor(bIsMultiLayer).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->meanLineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->meanColor(bIsMultiLayer).rgb()  & 0xffffff;
                    iPenWidth	= pLayerStyle->meanLineWidth();

                    if(!iPenWidth)
                        iPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                }
                else
                {
                    iColor		= QColor(Qt::blue).rgb() & 0xffffff;
                    iPenWidth	= 1;
                }
            }
            // Request to show limits
            if(iPenWidth)
            {
                // If multiple groups (non-interactive), draw Mean line with same color as the chart.
                if((m_pReportOptions->iGroups > 1) && internalChartInfo.m_pChart == NULL)
                    iColor = CGexReport::GetChartingColor(internalChartInfo.m_nGroup).rgb() & 0xffffff;

                //Add mean marker
                addMarker(m_pXYChart->xAxis(), lfMean, iColor, "Mean", iPenWidth, Chart::TopLeft);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean);
            }
        }

        // If request to show the max
        if ((m_pReportOptions->bProbPlotMarkerMax || qslAdvProbabilityPlotMarkerOptionsList.contains(QString("maximum"))) || internalChartInfo.m_pChart)
        {
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->maxColor(bIsMultiLayer).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->maxLineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->maxColor(bIsMultiLayer).rgb()  & 0xffffff;
                    iPenWidth	= pLayerStyle->maxLineWidth();
                    if(!iPenWidth)
                        iPenWidth = 1;
                }
                else
                {
                    iColor		= QColor(Qt::blue).rgb() & 0xffffff;
                    iPenWidth	= 1;
                }
            }

            // Request to show limits
            if(iPenWidth)
            {
                // If multiple groups (non-interactive), draw Mean line with same color as the chart.
                if((m_pReportOptions->iGroups > 1) && internalChartInfo.m_pChart == NULL)
                    iColor = CGexReport::GetChartingColor(internalChartInfo.m_nGroup).rgb() & 0xffffff;

                //Add max marker
                addMarker(m_pXYChart->xAxis(), lfMax, iColor, "Max.", iPenWidth, Chart::TopLeft);
            }
        }

        if(qslAdvProbabilityPlotMarkerOptionsList.contains(QString("quartile_q1")))
        {
            lfQuartileQ1 = dCustomScaleFactor*ptTestCell->lfSamplesQuartile1;
            pFile->FormatTestResultNoUnits(&lfQuartileQ1,ptTestCell->res_scal);
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->quartileQ1Color(bIsMultiLayer).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->quartileQ1LineWidth();
                if(!iPenWidth)
                iPenWidth = 1;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->maxColor(bIsMultiLayer).rgb()  & 0xffffff;
                    iPenWidth	= pLayerStyle->maxLineWidth();
                    if(!iPenWidth)
                        iPenWidth = 1;
                }
                else
                {
                    iColor		= QColor(Qt::blue).rgb() & 0xffffff;
                    iPenWidth	= 1;
                }
            }

            // Request to show limits
            if(iPenWidth)
            {
                // If multiple groups (non-interactive), draw Mean line with same color as the chart.
                if((m_pReportOptions->iGroups > 1) && internalChartInfo.m_pChart == NULL)
                    iColor = CGexReport::GetChartingColor(internalChartInfo.m_nGroup).rgb() & 0xffffff;

                //Add max marker
                addMarker(m_pXYChart->xAxis(), lfQuartileQ1, iColor, "Q1", iPenWidth, Chart::TopLeft);
            }

        }

        if(qslAdvProbabilityPlotMarkerOptionsList.contains(QString("quartile_q3")))
        {
            lfQuartileQ3 = dCustomScaleFactor*ptTestCell->lfSamplesQuartile3;
            pFile->FormatTestResultNoUnits(&lfQuartileQ3,ptTestCell->res_scal);
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->quartileQ1Color(bIsMultiLayer).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->quartileQ3LineWidth();
                if(!iPenWidth)
                iPenWidth = 1;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->maxColor(bIsMultiLayer).rgb()  & 0xffffff;
                    iPenWidth	= pLayerStyle->maxLineWidth();
                    if(!iPenWidth)
                        iPenWidth = 1;
                }
                else
                {
                    iColor		= QColor(Qt::blue).rgb() & 0xffffff;
                    iPenWidth	= 1;
                }
            }

            // Request to show limits
            if(iPenWidth)
            {
                // If multiple groups (non-interactive), draw Mean line with same color as the chart.
                if((m_pReportOptions->iGroups > 1) && internalChartInfo.m_pChart == NULL)
                    iColor = CGexReport::GetChartingColor(internalChartInfo.m_nGroup).rgb() & 0xffffff;

                //Add max marker
                addMarker(m_pXYChart->xAxis(), lfQuartileQ3, iColor, "Q3", iPenWidth, Chart::TopLeft);
            }

        }


        // If request to show the min
        if ((m_pReportOptions->bProbPlotMarkerMin  || qslAdvProbabilityPlotMarkerOptionsList.contains(QString("minimum"))) || internalChartInfo.m_pChart)
        {
            if (internalChartInfo.m_pChart)
            {
            iColor		= internalChartInfo.m_pChart->minColor(bIsMultiLayer).rgb() & 0xffffff;
            iPenWidth	= internalChartInfo.m_pChart->minLineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->minColor(bIsMultiLayer).rgb()  & 0xffffff;
                    iPenWidth	= pLayerStyle->minLineWidth();
                    if(!iPenWidth)
                        iPenWidth = 1;
                }
                else
                {
                    iColor		= QColor(Qt::blue).rgb() & 0xffffff;
                    iPenWidth	= 1;
                }
            }

            // Request to show limits
            if(iPenWidth)
            {
                // If multiple groups (non-interactive), draw Mean line with same color as the chart.
                if((m_pReportOptions->iGroups > 1) && internalChartInfo.m_pChart == NULL)
                    iColor = CGexReport::GetChartingColor(internalChartInfo.m_nGroup).rgb() & 0xffffff;

                //Add min marker
                addMarker(m_pXYChart->xAxis(), lfMin, iColor, "Min.", iPenWidth, Chart::TopLeft);
            }
        }

        // If request to show the median
        if (( (qslAdvProbabilityPlotMarkerOptionsList.contains(QString("median"))) ) && (lfMedian != -C_INFINITE))
        {
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->medianColor(bIsMultiLayer).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->medianLineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->medianColor(bIsMultiLayer).rgb()  & 0xffffff;
                    iPenWidth	= pLayerStyle->medianLineWidth();

                    if(!iPenWidth)
                        iPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                }
                else
                {
                    iColor		= QColor(Qt::blue).rgb() & 0xffffff;
                    iPenWidth	= 1;
                }
            }

            // Request to show limits
            if(iPenWidth)
            {
                // If multiple groups (non-interactive), draw Mean line with same color as the chart.
                if((m_pReportOptions->iGroups > 1) && internalChartInfo.m_pChart == NULL)
                    iColor = CGexReport::GetChartingColor(internalChartInfo.m_nGroup).rgb() & 0xffffff;

                //Add median marker
                addMarker(m_pXYChart->xAxis(), lfMedian, iColor, "Median", iPenWidth, Chart::TopLeft);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMedian);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMedian);
            }
        }

        Chart::SymbolType	spotSymbol;
        int					nFillColor;
        int					nLineColor;

        getLayerStyles(internalChartInfo.m_nGroup, internalChartInfo.m_pChart, nFillColor, nLineColor, spotSymbol);

        QString strName = "";

        // Insert new curves: only give an internal name if Interactive chart not hidden
        if(internalChartInfo.m_pChart != NULL)
            strName = internalChartInfo.m_pChart->strChartName;

        // Add data to the chart
        const double* lDataX = dataSet.dataX();
        const double* lDataY = dataSet.dataY();
        long lDataSize = dataSet.size();
        dataSet.rescale(internalChartInfo.m_nTestReferenceScaleFactor);
        if (m_pReportOptions->mPlotQQLine)
        {
            std::vector< double >lLine( lDataSize );
            double lInter = dataSet.GetLineToTheoreticalNormalQQPlotInter();
            double lSlope = dataSet.GetLineToTheoreticalNormalQQPlotSlope();
            for (long i = 0; i < lDataSize; ++i)
            {
                lLine[i] = lInter + lSlope * lDataX[i];
            }
            m_pXYChart->
                addLineLayer(DoubleArray(lLine.data(), lDataSize), 0)->
                setXData(DoubleArray(lDataX, lDataSize));
        }
        m_pXYChart->addScatterLayer(DoubleArray(lDataX, lDataSize),
                                    DoubleArray(lDataY, lDataSize),
                                    strName.toLatin1().constData(),
                                    spotSymbol, 5, nFillColor, nFillColor);

        // Write Custom markers defined thru the scripting interface (if a marker is sticky to a layer, it
        plotScriptingCustomMarkers(ptTestCell, nFillColor, internalChartInfo.m_nLayerIndex+1, dCustomScaleFactor, true, internalChartInfo.m_dMarkerMin, internalChartInfo.m_dMarkerMax);

        // Update viewport space if required
        switch(viewportMode())
        {
            case viewportOverLimits :
            default:
                // Chart has to be done over limits...unless they do not exist!
                if(internalChartInfo.m_dLowLimit < C_INFINITE)
                    internalChartInfo.m_dLowX = gex_min(internalChartInfo.m_dLowX, internalChartInfo.m_dLowLimit);
                else
                {
                    if(internalChartInfo.m_dHighLimit > -C_INFINITE)
                        internalChartInfo.m_dLowX = gex_min(internalChartInfo.m_dLowX, internalChartInfo.m_dHighLimit);

                    internalChartInfo.m_dLowX = gex_min(internalChartInfo.m_dLowX, lfMin);
                }

                if(internalChartInfo.m_dHighLimit > -C_INFINITE)
                    internalChartInfo.m_dHighX	= gex_max(internalChartInfo.m_dHighX, internalChartInfo.m_dHighLimit);
                else
                {
                    if(internalChartInfo.m_dLowLimit < C_INFINITE)
                        internalChartInfo.m_dHighX = gex_max(internalChartInfo.m_dHighX, internalChartInfo.m_dLowLimit);

                    internalChartInfo.m_dHighX	= gex_max(internalChartInfo.m_dHighX, lfMax);
                }

                break;

            case viewportOverData :

                // Chart has to be done over data (min & max)
                internalChartInfo.m_dLowX	= gex_min(internalChartInfo.m_dLowX, lfMin);
                internalChartInfo.m_dHighX	= gex_max(internalChartInfo.m_dHighX, lfMax);
                break;

            case viewportAdaptive :
                // Chart has to be done over maxi of both datapoints & limits
                if(internalChartInfo.m_dLowLimit < C_INFINITE)
                    internalChartInfo.m_dLowX	= gex_min(internalChartInfo.m_dLowX, internalChartInfo.m_dLowLimit);
                else if(internalChartInfo.m_dHighLimit > -C_INFINITE)
                    internalChartInfo.m_dLowX	= gex_min(internalChartInfo.m_dLowX, internalChartInfo.m_dHighLimit);

                if(internalChartInfo.m_dHighLimit > -C_INFINITE)
                    internalChartInfo.m_dHighX	= gex_max(internalChartInfo.m_dHighX, internalChartInfo.m_dHighLimit);
                else if(internalChartInfo.m_dLowLimit < C_INFINITE)
                    internalChartInfo.m_dHighX	= gex_max(internalChartInfo.m_dHighX, internalChartInfo.m_dLowLimit);

                internalChartInfo.m_dLowX	= gex_min(internalChartInfo.m_dLowX, lfMin);
                internalChartInfo.m_dHighX	= gex_max(internalChartInfo.m_dHighX, lfMax);

                if (internalChartInfo.m_dMarkerMin != C_INFINITE)
                    internalChartInfo.m_dLowX = gex_min(internalChartInfo.m_dLowX, internalChartInfo.m_dMarkerMin);

                if (internalChartInfo.m_dMarkerMax != -C_INFINITE)
                    internalChartInfo.m_dHighX = gex_max(internalChartInfo.m_dHighX, internalChartInfo.m_dMarkerMax);

                break;
        }

        internalChartInfo.m_dLowY	= gex_min(internalChartInfo.m_dLowY, dataSet.minY());
        internalChartInfo.m_dHighY	= gex_max(internalChartInfo.m_dHighY, dataSet.maxY());

        // Determine min and max values for the viewport
        if (internalChartInfo.m_dLowLimit < C_INFINITE)
            internalChartInfo.m_dViewportLowX = gex_min(internalChartInfo.m_dViewportLowX, internalChartInfo.m_dLowLimit);
        else if(internalChartInfo.m_dHighLimit > -C_INFINITE)
            internalChartInfo.m_dViewportLowX = gex_min(internalChartInfo.m_dViewportLowX, internalChartInfo.m_dHighLimit);

        internalChartInfo.m_dViewportLowX = gex_min(internalChartInfo.m_dViewportLowX, lfMin);

        if(internalChartInfo.m_dHighLimit > -C_INFINITE)
            internalChartInfo.m_dViewportHighX = gex_max(internalChartInfo.m_dViewportHighX, internalChartInfo.m_dHighLimit);
        else if(internalChartInfo.m_dLowLimit < C_INFINITE)
            internalChartInfo.m_dViewportHighX = gex_max(internalChartInfo.m_dViewportHighX, internalChartInfo.m_dLowLimit);

        internalChartInfo.m_dViewportHighX = gex_max(internalChartInfo.m_dViewportHighX, lfMax);

        if (internalChartInfo.m_dMarkerMin != C_INFINITE)
            internalChartInfo.m_dViewportLowX = gex_min(internalChartInfo.m_dViewportLowX, internalChartInfo.m_dMarkerMin);

        if (internalChartInfo.m_dMarkerMax != -C_INFINITE)
            internalChartInfo.m_dViewportHighX = gex_max(internalChartInfo.m_dViewportHighX, internalChartInfo.m_dMarkerMax);

        if (internalChartInfo.m_dLowSpecLimit != C_INFINITE
                && internalChartInfo.m_dLowSpecLimit < F_INFINITE
                && internalChartInfo.m_dLowSpecLimit != -C_INFINITE
                && internalChartInfo.m_dLowSpecLimit > -F_INFINITE)
            internalChartInfo.m_dViewportLowX = gex_min(internalChartInfo.m_dViewportLowX, internalChartInfo.m_dLowSpecLimit);

        if (internalChartInfo.m_dHighSpecLimit != C_INFINITE
                && internalChartInfo.m_dHighSpecLimit < F_INFINITE
                && internalChartInfo.m_dHighSpecLimit != -C_INFINITE
                && internalChartInfo.m_dHighSpecLimit > -F_INFINITE)
            internalChartInfo.m_dViewportHighX = gex_max(internalChartInfo.m_dViewportHighX, internalChartInfo.m_dHighSpecLimit);

        internalChartInfo.m_dViewportLowX = gex_min(internalChartInfo.m_dViewportLowX, lfMedian);
        internalChartInfo.m_dViewportLowX = gex_min(internalChartInfo.m_dViewportLowX, lfMean);

        internalChartInfo.m_dViewportLowY = gex_min(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dLowY);
        internalChartInfo.m_dViewportLowY = gex_min(internalChartInfo.m_dViewportLowY, -6);

        internalChartInfo.m_dViewportHighX = gex_max(internalChartInfo.m_dViewportHighX, lfMedian);
        internalChartInfo.m_dViewportHighX = gex_max(internalChartInfo.m_dViewportHighX, lfMean);

        internalChartInfo.m_dViewportHighY = gex_max(internalChartInfo.m_dViewportHighY, internalChartInfo.m_dHighY);
        internalChartInfo.m_dViewportHighY = gex_max(internalChartInfo.m_dViewportHighY, 6);

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString makeTooltip()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
QString	GexProbabilityPlotChart::makeTooltip()
{
    QString		strTooltip;

    if (referenceTestX() && isViewportInitialized())
    {
        double		lfCustomScaleFactor;
        QString		strUnitsX;

        // Get X and Y units (if any)
        strUnitsX = referenceTestX()->GetScaledUnits(&lfCustomScaleFactor, ReportOptions.GetOption("dataprocessing","scaling").toString());
        strUnitsX.truncate(10);

        // Cumulative probability scale
        QString pbya=m_pReportOptions->GetOption("adv_probabilityplot","y_axis").toString();
        if (pbya!="sigma")	//if(m_pReportOptions->bProbPlotYAxisSigma == false)
        {
            if (m_dXHotSpotValue != GEX_C_DOUBLE_NAN && m_pChartOverlays)
            {
                if (m_nDataset >= 0 && m_nDataset < m_pChartOverlays->chartsList().count())
                {
                    double dPercentRank = approximationOrderStatistic(m_nDataItem+1, m_pChartOverlays->chartsList().at(m_nDataset)->dataProbabilityPlot().size());

                    strTooltip.sprintf("X: %g %s\nY: %g", m_dXHotSpotValue, strUnitsX.toLatin1().constData(), dPercentRank);
                }
            }
        }
        else
        {
            if (m_dXHotSpotValue != GEX_C_DOUBLE_NAN && m_dYHotSpotValue != GEX_C_DOUBLE_NAN)
                strTooltip.sprintf("X: %g %s\nY: %g", m_dXHotSpotValue, strUnitsX.toLatin1().constData(), m_dYHotSpotValue);
        }

        // If log scale, do not show the data pointed...don't know its location!
        if(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX || m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY)
            strTooltip = "ProbPlot Area";
    }

    return strTooltip;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setViewportModeFromChartMode(int nChartMode)
//
// Description	:	convert probability plot chart mode from Gex into viewport mode
//
///////////////////////////////////////////////////////////////////////////////////
void GexProbabilityPlotChart::setViewportModeFromChartMode(int nChartMode)
{
    int nViewportMode;

    switch (nChartMode)
    {
        case GEX_PROBPLOTTYPE_LIMITS	:	nViewportMode = viewportOverLimits;
                                            break;

        case GEX_PROBPLOTTYPE_RANGE		:	nViewportMode = viewportOverData;
                                            break;

        case GEX_PROBPLOTTYPE_ADAPTIVE	:	nViewportMode = viewportAdaptive;
                                            break;

        default							:	nViewportMode = viewportOverLimits;
    }

    setViewportMode(nViewportMode);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void keepXTestViewport()
//
// Description	:	Keep wiewport for test X
//
///////////////////////////////////////////////////////////////////////////////////
void GexProbabilityPlotChart::keepXTestViewport()
{
    if (m_pChartOverlays && referenceTestX())
    {
        referenceTestX()->ptChartOptions->setCustomViewportX(true);
        referenceTestX()->ptChartOptions->setLowX(m_pChartOverlays->getViewportRectangle()[type()].lfLowX);
        referenceTestX()->ptChartOptions->setHighX(m_pChartOverlays->getViewportRectangle()[type()].lfHighX);
    }
}
