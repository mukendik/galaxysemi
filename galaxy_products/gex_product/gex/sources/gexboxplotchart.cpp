///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexboxplotchart.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "report_options.h"
#include <gqtl_global.h>
#include "drill_chart.h"


///////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport * gexReport;
extern CReportOptions ReportOptions;

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern double	ScalingPower(int iPower);

///////////////////////////////////////////////////////////////////////////////////
// Class GexBoxPlotChart - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexBoxPlotChart::GexBoxPlotChart(int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays /* = NULL */) : GexAbstractChart(chartTypeBoxPlot, nSizeMode, lWizardParent,pChartOverlays)
{
    m_nMaxTextHeight	= 0;
    m_nMaxTextWidth		= 0;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexBoxPlotChart::~GexBoxPlotChart()
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
int GexBoxPlotChart::visibleLayerCount() const
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
                // Seek to the relevant Group to plot
                if ((pChart->iGroupX < 0) || (pChart->iGroupX >= gexReport->getGroupsList().size()))
                    pGroup = NULL;
                else
                    pGroup = gexReport->getGroupsList().at(pChart->iGroupX);
                pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

                // Get test# & pinmap#
                lTestNumber		= pChart->iTestNumberX;
                lPinmapIndex	= pChart->iPinMapX;

                if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,FALSE,FALSE,pChart->strTestNameX)==1)
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
        QListIterator<CGexGroupOfFiles*>	itGroupsList(gexReport->getGroupsList());

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

            if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,FALSE,FALSE,referenceTestX()->strTestName)==1)
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
// Name			:	void buildOwnerChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexBoxPlotChart::buildOwnerChart()
{
    // Count the total number of VISIBLE layers
    CGexGroupOfFiles *		pGroup;
    CGexFileInGroup	*		pFile;
    CTest *					ptTestCell = NULL;	// Pointer to test cell of groups2 or higher
    TextBox *				pTextBox = NULL;
    GexInternalChartInfo	internalChartInfo;
    long					lTestNumber;
    long					lPinmapIndex;
    int						iTotalVisibleLayers = visibleLayerCount();

    // If all layers hiden, return now!
/*	if(!iTotalVisibleLayers)
        return;*/

    //Set the plot area Enable both horizontal and vertical grids by setting their colors to grey (0xc0c0c0)
    setTopMargin(25);
    setRightMargin(15);
    setBottomMargin(25);

    // If more than One box plot, write the label with an angle of 30°
    if(iTotalVisibleLayers > 1)
    {
        pTextBox = m_pXYChart->xAxis()->setLabelStyle("", 8, TextColor, 45.0);

        // reset value
        m_nMaxTextHeight	= 40;
        m_nMaxTextWidth		= 65;
    }

    m_boxPlotDataSet.init(iTotalVisibleLayers);

    if (m_pChartOverlays)
    {
        // Plot all the layers
        int iLayerBegin, iLayerEnd, iLayerStep;

        // If axis 'Swap' option set, we need to create layers in the opposite order...
        if(m_pReportOptions->GetOption("adv_boxplot_ex","orientation").toString()=="vertical")		//(m_pReportOptions->bBoxPlotExVertical)
        {
            iLayerBegin = 0;
            iLayerEnd	= m_pChartOverlays->chartsList().count();
            iLayerStep	= 1;
        }
        else
        {
            iLayerBegin = m_pChartOverlays->chartsList().count()-1;
            iLayerEnd	= -1;
            iLayerStep	= -1;
        }



        for (internalChartInfo.m_nLayerIndex = iLayerBegin; internalChartInfo.m_nLayerIndex != iLayerEnd; internalChartInfo.m_nLayerIndex += iLayerStep)
        {
            // Interactive mode: plot layers defined in the layer list
            internalChartInfo.m_pChart = m_pChartOverlays->chartsList().at(internalChartInfo.m_nLayerIndex);

            // Skip hidden layers!
            if(internalChartInfo.m_pChart->bVisible)
            {
                // Seek to the relevant Group to plot.
                if ((internalChartInfo.m_pChart->iGroupX < 0) || (internalChartInfo.m_pChart->iGroupX >= gexReport->getGroupsList().size()))
                    pGroup = NULL;
                else
                    pGroup	= gexReport->getGroupsList().at(internalChartInfo.m_pChart->iGroupX);
                pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

                // Get test# & pinmap#
                lTestNumber		= internalChartInfo.m_pChart->iTestNumberX;
                lPinmapIndex	= internalChartInfo.m_pChart->iPinMapX;

                if(pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, FALSE, FALSE,
                                       internalChartInfo.m_pChart->strTestNameX) == 1)
                {
                    // Keep track of visible layer index count
                    if (fillChart(pFile, ptTestCell, internalChartInfo, pTextBox))
                        internalChartInfo.m_nVisibleLayerIndex++;
                }
            }
        }
    }
    else
    {
        QListIterator<CGexGroupOfFiles*>	itGroupsList(gexReport->getGroupsList());

        if(m_pReportOptions->GetOption("adv_boxplot_ex","orientation").toString()=="vertical")		//(m_pReportOptions->bBoxPlotExVertical)
        {
            internalChartInfo.m_nGroup		= 1;
            internalChartInfo.m_nLayerIndex	= 0;

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

                if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,FALSE,FALSE,referenceTestX()->strTestName) == 1)
                {
                    // Keep track of visible layer index count
                    if (fillChart(pFile, ptTestCell, internalChartInfo, pTextBox))
                        internalChartInfo.m_nVisibleLayerIndex++;
                }

                internalChartInfo.m_nGroup++;
                internalChartInfo.m_nLayerIndex++;
            }
        }
        else
        {
            internalChartInfo.m_nGroup		= gexReport->getGroupsList().count();
            internalChartInfo.m_nLayerIndex	= gexReport->getGroupsList().count()-1;

            // Starts from the end of the Group list
            itGroupsList.toBack();

            while (itGroupsList.hasPrevious())
            {
                // Find test cell: RESET list to ensure we scan list of the right group!
                // Check if pinmap index...
                pGroup			= itGroupsList.previous();
                pFile			= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
                lTestNumber		= referenceTestX()->lTestNumber;

                if(referenceTestX()->lPinmapIndex >= 0)
                    lPinmapIndex = referenceTestX()->lPinmapIndex;
                else
                    lPinmapIndex = GEX_PTEST;

                if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,FALSE,FALSE,referenceTestX()->strTestName) == 1)
                {
                    // Keep track of visible layer index count
                    if (fillChart(pFile, ptTestCell, internalChartInfo, pTextBox))
                        internalChartInfo.m_nVisibleLayerIndex++;
                }

                internalChartInfo.m_nGroup--;
                internalChartInfo.m_nLayerIndex--;
            }

        }


    }

    if (internalChartInfo.m_bTestReferenceScaleFactor == false)
        internalChartInfo.m_strTitle += "<*color=FF0000*> - No data samples<*/color*>";

    m_pXYChart->addTitle(internalChartInfo.m_strTitle.toLatin1().constData());

    double dDiff = internalChartInfo.m_dHighY - OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        internalChartInfo.m_dHighY = ptTestCell->lfSamplesMax;
    }
    dDiff = internalChartInfo.m_dLowY + OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        internalChartInfo.m_dLowY = ptTestCell->lfSamplesMin;
    }

    // If custom viewport (in scripting file, not in interactive mode), overwrite defaults window viewing
    if(useTestCustomViewport())
    {
        // reset the current viewport
        resetViewportManager(false);

        // If custom viewport in X, overwrite defaults window viewing
        if(referenceTestX()->ptChartOptions->customViewportX())
        {
            internalChartInfo.m_dLowY	= referenceTestX()->ptChartOptions->lowX();
            internalChartInfo.m_dHighY	= referenceTestX()->ptChartOptions->highX();
        }
    }
    else if (m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].bForceViewport)
    {
        // Customer force viewport limit
        resetViewportManager(false);

        if (m_pChartOverlays->getViewportRectangle()[type()].lfLowY != -C_INFINITE)
            internalChartInfo.m_dLowY	= m_pChartOverlays->getViewportRectangle()[type()].lfLowY;

        if (m_pChartOverlays->getViewportRectangle()[type()].lfHighY != C_INFINITE)
            internalChartInfo.m_dHighY	= m_pChartOverlays->getViewportRectangle()[type()].lfHighY;

        if( m_pChartOverlays->getViewportRectangle()[type()].bForceViewport)
            m_pChartOverlays->getViewportRectangle()[type()].mChangeViewPort = true;

        m_pChartOverlays->getViewportRectangle()[type()].bForceViewport = false;
    }
    else
    {
        // Viewport: add 5% extra space at each end
        double	lfExtra = (internalChartInfo.m_dHighY - internalChartInfo.m_dLowY) * 0.05;	// Extra space +/- 5%

        internalChartInfo.m_dHighY	+= lfExtra;
        internalChartInfo.m_dLowY	-= lfExtra;
    }

    internalChartInfo.m_dViewportLowY	= gex_min(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dLowY);
    internalChartInfo.m_dViewportHighY	= gex_max(internalChartInfo.m_dViewportHighY, internalChartInfo.m_dHighY);

    dDiff = internalChartInfo.m_dViewportHighY - OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        internalChartInfo.m_dViewportHighY = internalChartInfo.m_dHighY;
    }
    dDiff = internalChartInfo.m_dViewportLowY + OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        internalChartInfo.m_dViewportLowY = internalChartInfo.m_dLowY;
    }

    // Define viewport: if in Interactive charting, consider zoomin factor
    if (m_pChartOverlays)
    {
        // Initialize viewport
        if (isVerticalViewportInitialized() == false)
        {
            initVerticalViewport(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dViewportHighY, internalChartInfo.m_dLowY, internalChartInfo.m_dHighY, 0.05);
            initHorizontalViewport(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dViewportHighY, internalChartInfo.m_dLowY, internalChartInfo.m_dHighY, 0.05);
        }


        // Using ViewPortLeft and ViewPortWidth, get the start and end dates of the view port.
        // Make Boxplot horizontal if option set
        if(m_pReportOptions->GetOption("adv_boxplot_ex","orientation").toString()=="vertical")		//(m_pReportOptions->bBoxPlotExVertical == true)
            computeYBounds(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY);
        else
            computeXBounds(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY);
    }

    m_pXYChart->yAxis()->setLinearScale(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY);

    // If many visible layers, change legend format
    if(internalChartInfo.m_nVisibleLayerIndex > 1)
    {
        if(m_pReportOptions->GetOption("adv_boxplot_ex","orientation").toString()=="horizontal") //(m_pReportOptions->bBoxPlotExVertical == false)
        {
            setBottomMargin(m_nMaxTextWidth);
            setLeftMargin(m_nMaxTextHeight);
        }
        else
        {
            setBottomMargin(m_nMaxTextHeight);
            setLeftMargin(m_nMaxTextWidth);
        }
    }
    else if(internalChartInfo.m_nVisibleLayerIndex == 1)
        strcpy(m_boxPlotDataSet.labels()[0], "");

    // Set the labels on the x axis and the font to Arial
    m_pXYChart->xAxis()->setLabels(StringArray(m_boxPlotDataSet.labels(), iTotalVisibleLayers));

    // Next layer: Box Whisker layer. Set the line width to 2 pixels
    m_pXYChart->addBoxWhiskerLayer2(DoubleArray(m_boxPlotDataSet.dataQ3(), iTotalVisibleLayers),
                                    DoubleArray(m_boxPlotDataSet.dataQ1(), iTotalVisibleLayers),
                                    DoubleArray(m_boxPlotDataSet.dataQ4(), iTotalVisibleLayers),
                                    DoubleArray(m_boxPlotDataSet.dataQ0(), iTotalVisibleLayers),
                                    DoubleArray(m_boxPlotDataSet.dataQ2(), iTotalVisibleLayers),
                                    IntArray(m_boxPlotDataSet.colors(), iTotalVisibleLayers),
                                    0.5)->setLineWidth(2);

    if (m_pChartOverlays)
    {
        m_pChartOverlays->getViewportRectangle()[type()].lfLowX	= -C_INFINITE;
        m_pChartOverlays->getViewportRectangle()[type()].lfLowY	= internalChartInfo.m_dLowY;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighX	= C_INFINITE;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighY	= internalChartInfo.m_dHighY;
    }

    // Make Boxplot horizontal if option set
    if(m_pReportOptions->GetOption("adv_boxplot_ex","orientation").toString()=="horizontal")	//(m_pReportOptions->bBoxPlotExVertical == false)
        m_pXYChart->swapXY();

    // Delete temporary buffers
    m_boxPlotDataSet.clean();
}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void fillChart(CGexFileInGroup * pFile, CTest * ptTestCell, GexInternalChartInfo& internalChartInfo, TextBox * pTextBox)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexBoxPlotChart::fillChart(CGexFileInGroup * pFile, CTest * ptTestCell, GexInternalChartInfo& internalChartInfo, TextBox * pTextBox)
{
    double	dCustomScaleFactor;
    double	lfMin;
    double	lfMax;
    double	lfMean;
    double	lfSigma;
    double	lfMedian = -C_INFINITE;
    double	lfIQR;
    double	lfLowWhisker;
    double	lfHighWhisker;
    double	lfLayerIndex;
    double	lfValue;
    double	lfQuartileQ1=-C_INFINITE;
    double	lfQuartileQ3=-C_INFINITE;
    int		iColor;
    int		iPenWidth;
    int		nFillColor;
    int		nLineColor;
    QString	strString;
    bool	bIsMultiLayers = isMultiLayers();
    bool    lShowTitle = false;

    // OPTIONS
    QString strAdvBoxplotExMarkerOptions = (m_pReportOptions->GetOption(QString("adv_boxplot_ex"), QString("marker"))).toString();
    QStringList qslAdvBoxplotExMarkerOptionList = strAdvBoxplotExMarkerOptions.split(QString("|"));
    QString scaling = m_pReportOptions->GetOption("dataprocessing","scaling").toString();

    if ((m_pChartOverlays && m_pChartOverlays->mTitle == true) ||
        qslAdvBoxplotExMarkerOptionList.contains(QString("test_name")))
    {
        lShowTitle = true;
    }

    // On first visible layer...extra limits, etc...
    if(internalChartInfo.m_nVisibleLayerIndex == 0)
    {
        // Check if we have a custom title to overwrite default one
        if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bChartTitle)
        {
            internalChartInfo.m_strTitle =  gexReport->buildDisplayName(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strChartTitle, false);	// custom title.
        }
        else if (lShowTitle)
        {
            if(bIsMultiLayers && (isSingleTestAllLayers() == false))
            {
                // In multi-layer mode and with multiple layers shown, title is generic...unless all layers refer to same test#!
                switch(internalChartInfo.m_pChart->iWhiskerMode)
                {
                    case GEX_WHISKER_RANGE:
                        internalChartInfo.m_strTitle = "Box-Plot: Range type";
                        break;

                    case GEX_WHISKER_Q1Q3:
                        internalChartInfo.m_strTitle = "Box-Whisker: Q1-1.5*IQR / Q3+1.5*IQR";
                        break;

                    case GEX_WHISKER_IQR:
                        internalChartInfo.m_strTitle = "Box-Whisker: Median +/- 1.5*IQR";
                        break;
                }
            }
            else
            {
                char	szTestName[2*GEX_MAX_STRING];

                gexReport->BuildTestNameString(pFile, ptTestCell, szTestName);
                internalChartInfo.m_strTitle = "Test ";
                internalChartInfo.m_strTitle += ptTestCell->szTestLabel;
                internalChartInfo.m_strTitle += " : ";
                QString strNormalizedName = gexReport->buildDisplayName(szTestName, false);
                internalChartInfo.m_strTitle += strNormalizedName;
            }
        }

        // No data samples.
        if (ptTestCell->m_testResult.count() == 0)
            return false;

        QString strLabel;

        // Check if we have a custom legend to overwrite default one
        if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLegendY)
            strLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strAxisLegendY;
        else
        {
            // Set Y scale label (units)
            strLabel = "Test results ";
            QString strUnits = ptTestCell->GetScaledUnits(&dCustomScaleFactor, scaling);

            if(strUnits.length() > 0)
                strLabel += "(" + strUnits + ")";
        }

        // Set Y label
        m_pXYChart->yAxis()->setTitle(strLabel.toLatin1().constData());

        // Check if we have a custom legend to overwrite default one
        // and set the X label
        if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLegendX)
            m_pXYChart->xAxis()->setTitle(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strAxisLegendX.toLatin1().constData());
    }
    else if (ptTestCell->m_testResult.count() == 0)
        // No data samples
        return false;

    if (internalChartInfo.m_bTestReferenceScaleFactor == false)
    {
        internalChartInfo.m_nTestReferenceScaleFactor = ptTestCell->res_scal;
        internalChartInfo.m_bTestReferenceScaleFactor = true;
    }

    Chart::SymbolType	spotSymbol;

    // Get layer style
    getLayerStyles(internalChartInfo.m_nGroup, internalChartInfo.m_pChart, nFillColor, nLineColor, spotSymbol);

    int iCustomScaleFactor = ptTestCell->res_scal - internalChartInfo.m_nTestReferenceScaleFactor;
    dCustomScaleFactor = 1/GS_POW(10.0, iCustomScaleFactor);

    // Compute Min (and scale to result scale)
    lfMin = ptTestCell->lfSamplesMin * dCustomScaleFactor;
    pFile->FormatTestResultNoUnits(&lfMin, ptTestCell->res_scal);

    // Compute Max (and scale to result scale)
    lfMax = ptTestCell->lfSamplesMax * dCustomScaleFactor;
    pFile->FormatTestResultNoUnits(&lfMax, ptTestCell->res_scal);

    // Compute Mean (and scale to result scale)
    lfMean = ptTestCell->lfMean * dCustomScaleFactor;
    pFile->FormatTestResultNoUnits(&lfMean, ptTestCell->res_scal);

    // Compute Sigma (and scale to result scale)
    lfSigma = ptTestCell->lfSigma * dCustomScaleFactor;
    pFile->FormatTestResultNoUnits(&lfSigma, ptTestCell->res_scal);

    // Compute Median (and scale to result scale)
    if (ptTestCell->lfSamplesQuartile2 != -C_INFINITE)
    {
        lfMedian = ptTestCell->lfSamplesQuartile2 * dCustomScaleFactor;
        pFile->FormatTestResultNoUnits(&lfMedian,ptTestCell->res_scal);
    }

    // Save Quartiles info
    lfIQR = ptTestCell->lfSamplesQuartile3 - ptTestCell->lfSamplesQuartile1;

    // Add sigma marker on 1 group
    if (ptTestCell->ldSamplesValidExecs/* && internalChartInfo.m_nVisibleLayerIndex == 0*/)
    {
        // If request to show the 2sigma space
        // if (m_pReportOptions->bBoxPlotExMarker2Sigma || internalChartInfo.m_pChart)
        if ( (qslAdvBoxplotExMarkerOptionList.contains(QString("2sigma"))))
        {
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->sigma2Color(bIsMultiLayers).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->sigma2LineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->sigma2Color(bIsMultiLayers).rgb() & 0xffffff;
                    iPenWidth	= pLayerStyle->sigma2LineWidth();

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
            {
                int nLineColor = m_pXYChart->dashLineColor(iColor, DotDashLine);

                //Add marker -1 sigma
                addMarker(m_pXYChart->yAxis(), lfMean-lfSigma, nLineColor, "-1s", iPenWidth, Chart::BottomLeft, false)->setFontColor(iColor);

                //Add marker +1 sigma
                addMarker(m_pXYChart->yAxis(), lfMean+lfSigma, nLineColor, "+1s", iPenWidth, Chart::TopLeft, false)->setFontColor(iColor);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean - lfSigma);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean + lfSigma);
            }
        }

        // If request to show the 3sigma space
        if ( (qslAdvBoxplotExMarkerOptionList.contains(QString("3sigma"))))
        {
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->sigma3Color(bIsMultiLayers).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->sigma3LineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->sigma3Color(bIsMultiLayers).rgb() & 0xffffff;
                    iPenWidth	= pLayerStyle->sigma3LineWidth();

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
            {
                int nLineColor = m_pXYChart->dashLineColor(iColor, DotDashLine);

                //Add marker -1.5 sigma
                addMarker(m_pXYChart->yAxis(), lfMean - 1.5 * lfSigma, nLineColor, "-1.5s", iPenWidth, Chart::BottomLeft, false)->setFontColor(iColor);

                //Add marker +1.5 sigma
                addMarker(m_pXYChart->yAxis(), lfMean + 1.5 * lfSigma, nLineColor, "+1.5s", iPenWidth, Chart::TopLeft, false)->setFontColor(iColor);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean - 1.5 * lfSigma);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean + 1.5 * lfSigma);
            }
        }

        // If request to show the 6sigma space
        if ( (qslAdvBoxplotExMarkerOptionList.contains(QString("6sigma"))))
        {
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->sigma6Color(bIsMultiLayers).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->sigma6LineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->sigma6Color(bIsMultiLayers).rgb()  & 0xffffff;
                    iPenWidth	= pLayerStyle->sigma6LineWidth();

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
            {
                int nLineColor = m_pXYChart->dashLineColor(iColor, DotDashLine);

                //Add marker -3 sigma
                addMarker(m_pXYChart->yAxis(), lfMean - 3 * lfSigma, nLineColor, "-3s", iPenWidth, Chart::BottomLeft, false)->setFontColor(iColor);

                //Add marker +3 sigma
                addMarker(m_pXYChart->yAxis(), lfMean + 3 * lfSigma, nLineColor, "+3s", iPenWidth, Chart::TopLeft, false)->setFontColor(iColor);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean - 3 * lfSigma);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean + 3 * lfSigma);
            }
        }

        // If request to show the 12sigma space
        if ( (qslAdvBoxplotExMarkerOptionList.contains(QString("12sigma"))))
        {
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->sigma12Color(bIsMultiLayers).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->sigma12LineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->sigma12Color(bIsMultiLayers).rgb() & 0xffffff;
                    iPenWidth	= pLayerStyle->sigma12LineWidth();

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
            {
                int nLineColor = m_pXYChart->dashLineColor(iColor, DotDashLine);

                //Add marker -6 sigma
                addMarker(m_pXYChart->yAxis(), lfMean - 6 * lfSigma, nLineColor, "-6s", iPenWidth, Chart::BottomLeft, false)->setFontColor(iColor);

                //Add marker +6 sigma
                addMarker(m_pXYChart->yAxis(), lfMean + 6 * lfSigma, nLineColor, "+6s", iPenWidth, Chart::TopLeft, false)->setFontColor(iColor);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean - 6 * lfSigma);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean + 6 * lfSigma);
            }
        }
    }

    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
    {
        // Draw LL limit markers
        internalChartInfo.m_dLowLimit = ptTestCell->GetCurrentLimitItem()->lfLowLimit;		// Low limit exists

        // If we have to keep values in normalized format, do not rescale!
        //if(m_pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
        if (scaling!="normalized")
        {
            // convert LowLimit to same scale as results:
            pFile->FormatTestResultNoUnits(&internalChartInfo.m_dLowLimit, ptTestCell->llm_scal);

            internalChartInfo.m_dLowLimit *=  ScalingPower(ptTestCell->llm_scal);	// normalized
            internalChartInfo.m_dLowLimit /=  ScalingPower(ptTestCell->res_scal);	// normalized
        }

        // LowLimit Marker
        if ( (qslAdvBoxplotExMarkerOptionList.contains(QString("limits"))) )
        {
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->limitsColor(bIsMultiLayers).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->limitsLineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor = pLayerStyle->limitsColor(bIsMultiLayers).rgb()  & 0xffffff;
                    iPenWidth = pLayerStyle->limitsLineWidth();

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
                //Add marker
                addMarker(m_pXYChart->yAxis(), internalChartInfo.m_dLowLimit, iColor, "Low L.", iPenWidth, Chart::TopLeft, false);
        }
    }

    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
    {
        // Draw HL limit markers (done only once: when charting Plot for group#1)
        internalChartInfo.m_dHighLimit = ptTestCell->GetCurrentLimitItem()->lfHighLimit;		// High limit exists

        // If we have to keep values in normalized format, do not rescale!
        //if(m_pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
        if (scaling!="normalized")
        {
            // convert HighLimit to same scale as results:
            pFile->FormatTestResultNoUnits(&internalChartInfo.m_dHighLimit, ptTestCell->hlm_scal);

            internalChartInfo.m_dHighLimit *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
            internalChartInfo.m_dHighLimit /=  ScalingPower(ptTestCell->res_scal);	// normalized
        }

        if ( (qslAdvBoxplotExMarkerOptionList.contains(QString("limits"))))
        {
            if (internalChartInfo.m_pChart)
            {
                // High limit Marker
                iColor		= internalChartInfo.m_pChart->limitsColor(bIsMultiLayers).rgb() & 0xffffff;
                iPenWidth	= internalChartInfo.m_pChart->limitsLineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                    iColor		= pLayerStyle->limitsColor(bIsMultiLayers).rgb()  & 0xffffff;
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
                //Add marker
                addMarker(m_pXYChart->yAxis(), internalChartInfo.m_dHighLimit, iColor, "High L.", iPenWidth, Chart::BottomLeft, false);
        }
    }

    // Draw multi-limit markers.
    addMultiLimitMarkers(internalChartInfo.m_pChart, ptTestCell, internalChartInfo);

    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
    {
        // Draw LL limit markers
        internalChartInfo.m_dLowSpecLimit = ptTestCell->lfLowSpecLimit;		// Low Spec limit exists

        // If we have to keep values in normalized format, do not rescale!
        //if(m_pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
        if (scaling!="normalized")
        {
            // convert LowLimit to same scale as results:
            pFile->FormatTestResultNoUnits(&internalChartInfo.m_dLowSpecLimit, ptTestCell->llm_scal);

            internalChartInfo.m_dLowSpecLimit *=  ScalingPower(ptTestCell->llm_scal);	// normalized
            internalChartInfo.m_dLowSpecLimit /=  ScalingPower(ptTestCell->res_scal);	// normalized
        }

        // LowLimit Marker
        if ( (qslAdvBoxplotExMarkerOptionList.contains(QString("speclimits"))) )
        {
            if (internalChartInfo.m_pChart)
            {
                iColor		= internalChartInfo.m_pChart->limitsColor(bIsMultiLayers).rgb() & 0xffffff;
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
                    iColor = pLayerStyle->limitsColor(bIsMultiLayers).rgb()  & 0xffffff;
                    iPenWidth = pLayerStyle->limitsLineWidth();

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
                //Add marker
                addMarker(m_pXYChart->yAxis(), internalChartInfo.m_dLowSpecLimit, iColor, m_strLowSpecLimit, iPenWidth, Chart::TopLeft,false);
        }
    }

    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
    {
        // Draw HL limit markers (done only once: when charting Plot for group#1)
        internalChartInfo.m_dHighSpecLimit = ptTestCell->lfHighSpecLimit;		// High limit exists

        // If we have to keep values in normalized format, do not rescale!
        if (scaling!="normalized")
        {
            // convert HighLimit to same scale as results:
            pFile->FormatTestResultNoUnits(&internalChartInfo.m_dHighSpecLimit , ptTestCell->hlm_scal);

            internalChartInfo.m_dHighSpecLimit  *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
            internalChartInfo.m_dHighSpecLimit  /=  ScalingPower(ptTestCell->res_scal);	// normalized
        }

        if ( (qslAdvBoxplotExMarkerOptionList.contains(QString("speclimits"))) )
        {
            if (internalChartInfo.m_pChart)
            {
                // High limit Marker
                iColor		= internalChartInfo.m_pChart->limitsColor(bIsMultiLayers).rgb() & 0xffffff;
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
                    iColor		= pLayerStyle->limitsColor(bIsMultiLayers).rgb()  & 0xffffff;
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
                //Add marker
                addMarker(m_pXYChart->yAxis(), internalChartInfo.m_dHighSpecLimit , iColor, m_strHighSpecLimit, iPenWidth, Chart::BottomLeft, false);
        }
    }

    // If request to show the Mean
    if((qslAdvBoxplotExMarkerOptionList.contains(QString("mean"))))
    {
        if (internalChartInfo.m_pChart)
        {
            iColor		= internalChartInfo.m_pChart->meanColor(bIsMultiLayers).rgb() & 0xffffff;
            iPenWidth	= internalChartInfo.m_pChart->meanLineWidth();
        }
        else
        {
            // Check if index is valid
            CGexSingleChart	*pLayerStyle = NULL;
            if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
            {
                pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                iColor		= pLayerStyle->meanColor(bIsMultiLayers).rgb()  & 0xffffff;
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

            // Change label position for group 1...just in another group as exctly the same mean!
            if (internalChartInfo.m_nVisibleLayerIndex == 0)
                addMarker(m_pXYChart->yAxis(), lfMean, iColor, "Mean", iPenWidth, Chart::TopRight, false);
            else
                addMarker(m_pXYChart->yAxis(), lfMean, iColor, "Mean", iPenWidth, Chart::BottomRight, false);

            internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean);
            internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean);
        }
    }

    // If request to show the Median
    if ( (qslAdvBoxplotExMarkerOptionList.contains(QString("median"))) && lfMedian != -C_INFINITE)
    {
        if (internalChartInfo.m_pChart)
        {
            iColor		= internalChartInfo.m_pChart->medianColor(bIsMultiLayers).rgb() & 0xffffff;
            iPenWidth	= internalChartInfo.m_pChart->medianLineWidth();
        }
        else
        {
            // Check if index is valid
            CGexSingleChart	*pLayerStyle = NULL;
            if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
            {
                pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                iColor		= pLayerStyle->medianColor(bIsMultiLayers).rgb()  & 0xffffff;
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

            // Change label position for group 1...just in another group as exctly the same mean!
            if (internalChartInfo.m_nVisibleLayerIndex == 0)
                addMarker(m_pXYChart->yAxis(), lfMedian, iColor, "Median", iPenWidth, Chart::TopRight, false);
            else
                addMarker(m_pXYChart->yAxis(), lfMedian, iColor, "Median", iPenWidth, Chart::BottomRight, false);

            internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMedian);
            internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMedian);
        }
    }

    // If request to show the Min
    if((m_pReportOptions->bBoxPlotExMarkerMin || qslAdvBoxplotExMarkerOptionList.contains(QString("minimum"))) || internalChartInfo.m_pChart)
    {
        if (internalChartInfo.m_pChart)
        {
            iColor		= internalChartInfo.m_pChart->minColor(bIsMultiLayers).rgb() & 0xffffff;
            iPenWidth	= internalChartInfo.m_pChart->minLineWidth();
        }
        else
        {
            // Check if index is valid
            CGexSingleChart	*pLayerStyle = NULL;
            if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
            {
                pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                iColor		= pLayerStyle->minColor(bIsMultiLayers).rgb() & 0xffffff;
                iPenWidth	= pLayerStyle->minLineWidth();

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

            // Change label position for group 1...just in another group as exctly the same mean!
            if (internalChartInfo.m_nVisibleLayerIndex == 0)
                addMarker(m_pXYChart->yAxis(), lfMin, iColor, "Min.", iPenWidth, Chart::TopLeft, false);
            else
                addMarker(m_pXYChart->yAxis(), lfMin, iColor, "Min.", iPenWidth, Chart::TopRight, false);
        }
    }

    // If request to show the Max
    if((m_pReportOptions->bBoxPlotExMarkerMax || qslAdvBoxplotExMarkerOptionList.contains(QString("maximum")) ) || internalChartInfo.m_pChart)
    {
        if (internalChartInfo.m_pChart)
        {
            iColor		= internalChartInfo.m_pChart->maxColor(bIsMultiLayers).rgb() & 0xffffff;
            iPenWidth	= internalChartInfo.m_pChart->maxLineWidth();
        }
        else
        {
            // Check if index is valid
            CGexSingleChart	*pLayerStyle = NULL;
            if ((internalChartInfo.m_nGroup-1) >= 0 && (internalChartInfo.m_nGroup-1) < m_pReportOptions->pLayersStyleList.count())
            {
                pLayerStyle = m_pReportOptions->pLayersStyleList.at(internalChartInfo.m_nGroup-1);
                iColor		= pLayerStyle->maxColor(bIsMultiLayers).rgb()  & 0xffffff;
                iPenWidth	= pLayerStyle->maxLineWidth();

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

            // Change label position for group 1...just in another group as exctly the same mean!
            if (internalChartInfo.m_nVisibleLayerIndex == 0)
                addMarker(m_pXYChart->yAxis(), lfMax, iColor, "Max.", iPenWidth, Chart::TopLeft, false);
            else
                addMarker(m_pXYChart->yAxis(), lfMax, iColor, "Max.", iPenWidth, Chart::TopRight, false);
            }
    }

     if ( qslAdvBoxplotExMarkerOptionList.contains(QString("quartile_q1")))
     {
         lfQuartileQ1 = dCustomScaleFactor*ptTestCell->lfSamplesQuartile1;
         pFile->FormatTestResultNoUnits(&lfQuartileQ1,ptTestCell->res_scal);
         if (internalChartInfo.m_pChart)
         {
             iColor		= internalChartInfo.m_pChart->quartileQ1Color(bIsMultiLayers).rgb() & 0xffffff;
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
                 iColor		= pLayerStyle->quartileQ1Color(bIsMultiLayers).rgb()  & 0xffffff;
                 iPenWidth	= pLayerStyle->quartileQ1LineWidth();

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

             // Change label position for group 1...just in another group as exctly the same mean!
             if (internalChartInfo.m_nVisibleLayerIndex == 0)
                 addMarker(m_pXYChart->yAxis(), lfQuartileQ1, iColor, "Q1", iPenWidth, Chart::TopRight, false);
             else
                 addMarker(m_pXYChart->yAxis(), lfQuartileQ1, iColor, "Q1", iPenWidth, Chart::BottomRight, false);
         }
     }

     if ( qslAdvBoxplotExMarkerOptionList.contains(QString("quartile_q3")))
     {
         lfQuartileQ3 = dCustomScaleFactor*ptTestCell->lfSamplesQuartile3;
         pFile->FormatTestResultNoUnits(&lfQuartileQ3,ptTestCell->res_scal);
         if (internalChartInfo.m_pChart)
         {
             iColor		= internalChartInfo.m_pChart->quartileQ3Color(bIsMultiLayers).rgb() & 0xffffff;
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
                 iColor		= pLayerStyle->quartileQ3Color(bIsMultiLayers).rgb()  & 0xffffff;
                 iPenWidth	= pLayerStyle->quartileQ3LineWidth();

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

             // Change label position for group 1...just in another group as exctly the same mean!
             if (internalChartInfo.m_nVisibleLayerIndex == 0)
                 addMarker(m_pXYChart->yAxis(), lfQuartileQ3, iColor, "Q3", iPenWidth, Chart::TopRight, false);
             else
                 addMarker(m_pXYChart->yAxis(), lfQuartileQ3, iColor, "Q3", iPenWidth, Chart::BottomRight, false);
         }
     }


    if (m_pChartOverlays)
    {
        switch(internalChartInfo.m_pChart->iWhiskerMode)
        {
            case GEX_WHISKER_RANGE:
                // Whisker = Range
                lfLowWhisker	= lfMin;
                lfHighWhisker	= lfMax;
                break;

            case GEX_WHISKER_Q1Q3:
                // Whisker = Q1-1.5*IQR, Q3+1.5*IQR
                lfLowWhisker	= dCustomScaleFactor * (ptTestCell->lfSamplesQuartile1 - 1.5*lfIQR);
                lfHighWhisker	= dCustomScaleFactor * (ptTestCell->lfSamplesQuartile3 + 1.5*lfIQR);

                pFile->FormatTestResultNoUnits(&lfLowWhisker,  ptTestCell->res_scal);
                pFile->FormatTestResultNoUnits(&lfHighWhisker, ptTestCell->res_scal);
                break;

            case GEX_WHISKER_IQR:
                // Whisker = Q2 +/-1.5*IQR
                lfLowWhisker	= dCustomScaleFactor * (ptTestCell->lfSamplesQuartile2 - 1.5*lfIQR);
                lfHighWhisker	= dCustomScaleFactor * (ptTestCell->lfSamplesQuartile2 + 1.5*lfIQR);

                pFile->FormatTestResultNoUnits(&lfLowWhisker,	ptTestCell->res_scal);
                pFile->FormatTestResultNoUnits(&lfHighWhisker,	ptTestCell->res_scal);
                break;

            case GEX_WHISKER_HALFPERCEN:
                lfLowWhisker	= dCustomScaleFactor * (ptTestCell->lfSamplesP0_5);
                lfHighWhisker	= dCustomScaleFactor * (ptTestCell->lfSamplesP99_5);

                pFile->FormatTestResultNoUnits(&lfLowWhisker,	ptTestCell->res_scal);
                pFile->FormatTestResultNoUnits(&lfHighWhisker,	ptTestCell->res_scal);
                break;

            case GEX_WHISKER_TWOPERCEN:
                lfLowWhisker	= dCustomScaleFactor * (ptTestCell->lfSamplesP2_5);
                lfHighWhisker	= dCustomScaleFactor * (ptTestCell->lfSamplesP97_5);

                pFile->FormatTestResultNoUnits(&lfLowWhisker,	ptTestCell->res_scal);
                pFile->FormatTestResultNoUnits(&lfHighWhisker,	ptTestCell->res_scal);
                break;

            case GEX_WHISKER_TENPERCEN:
                lfLowWhisker	= dCustomScaleFactor * (ptTestCell->lfSamplesP10);
                lfHighWhisker	= dCustomScaleFactor * (ptTestCell->lfSamplesP90);

                pFile->FormatTestResultNoUnits(&lfLowWhisker,	ptTestCell->res_scal);
                pFile->FormatTestResultNoUnits(&lfHighWhisker,	ptTestCell->res_scal);
                break;
        }
    }
    else
    {
        lfLowWhisker	= lfMin;
        lfHighWhisker	= lfMax;
    }

    lfLayerIndex = (double) internalChartInfo.m_nVisibleLayerIndex;

    if (m_pChartOverlays)
    {
        // Save test name
        if(m_boxPlotDataSet.size() == 1 || gexReport->getGroupsList().count() == 1)
            strString	= internalChartInfo.m_pChart->strTestLabelX;	// Only one layer, or only one group/dataset : show test name
        else
        {
            if ((internalChartInfo.m_pChart->iGroupX >= 0) && (internalChartInfo.m_pChart->iGroupX < gexReport->getGroupsList().size()))
                strString	= gexReport->getGroupsList().at(internalChartInfo.m_pChart->iGroupX)->strGroupName;
        }
    }
    else
    {
        if (((internalChartInfo.m_nGroup-1) >= 0) && ((internalChartInfo.m_nGroup-1) < gexReport->getGroupsList().size()))
            strString	= gexReport->getGroupsList().at(internalChartInfo.m_nGroup-1)->strGroupName;
    }


    if(mWizardParent)
    {
        // format the name in order to remove duplicate info like the spliter used
        mWizardParent->formatRequestDBName(strString);
    }

    // Apply options to truncate test name if need be
    //if ((m_pReportOptions->iTruncateLabels > 0) && (strString.size() >= m_pReportOptions->iTruncateLabels))
    QString TN=m_pReportOptions->GetOption("output","truncate_names").toString();
    if (TN!="no")
    {
        bool ok=false; int s=TN.toInt(&ok);
        if (!ok || s<2)
            s=32;
        if (strString.size() >= s)
        {	// Do not truncate in the middle of a '<>' string!
            strString.truncate(s);
            strString += "...";
        }
    }

    if (pTextBox)
    {
        int nTextWidth;
        int nTextHeight;

        // With version 5.X
        if (Chart::getVersion() == 5)
        {
            pTextBox->setText(strString.toLatin1().constData());

            nTextWidth	= pTextBox->getWidth() + 10;
            nTextHeight	= pTextBox->getHeight() + 10;
        }
        // with older version (4.X)
        else
        {
            nTextWidth	= (int) (strString.size() * 4.4 + 10);
            nTextHeight = (int) (strString.size() * 4.4 + 10);
        }

  nTextWidth -= (internalChartInfo.m_nVisibleLayerIndex + 1) *
                ((int) (1.0 / (m_boxPlotDataSet.size() * 2)) *
                 (width() - horizontalMargin()));
        m_nMaxTextHeight	= gex_max(m_nMaxTextHeight, nTextHeight);
        m_nMaxTextWidth		= gex_max(m_nMaxTextWidth, nTextWidth);
    }

    m_boxPlotDataSet.addData(internalChartInfo.m_nVisibleLayerIndex, gex_max(lfLowWhisker,lfMin), dCustomScaleFactor * ptTestCell->lfSamplesQuartile1,
                                dCustomScaleFactor * ptTestCell->lfSamplesQuartile2, dCustomScaleFactor * ptTestCell->lfSamplesQuartile3, gex_min(lfHighWhisker,lfMax),
                                nFillColor, strString);

    pFile->FormatTestResultNoUnits(&m_boxPlotDataSet.dataQ1()[internalChartInfo.m_nVisibleLayerIndex], ptTestCell->res_scal);
    pFile->FormatTestResultNoUnits(&m_boxPlotDataSet.dataQ2()[internalChartInfo.m_nVisibleLayerIndex], ptTestCell->res_scal);
    pFile->FormatTestResultNoUnits(&m_boxPlotDataSet.dataQ3()[internalChartInfo.m_nVisibleLayerIndex], ptTestCell->res_scal);

    // Top layer: Mean marker ('+' character in light green color)
    m_pXYChart->addScatterLayer(DoubleArray(&lfLayerIndex, 1), DoubleArray(&lfMean, 1), "", Chart::CrossShape(0.2), 20, 0x33FF33);

    // If layer visible: Write Custom markers defined thru the scripting interface (if a marker is sticky to a layer, it
    plotScriptingCustomMarkers(ptTestCell, nFillColor, internalChartInfo.m_nLayerIndex+1, dCustomScaleFactor, false, internalChartInfo.m_dMarkerMin, internalChartInfo.m_dMarkerMax);

    // Update viewport space if required
    switch(viewportMode())
    {
        case viewportOverLimits :
        default :
            // Chart has to be done over limits...unless they do not exist!
            if(internalChartInfo.m_dLowLimit != C_INFINITE)
                internalChartInfo.m_dLowY = gex_min(internalChartInfo.m_dLowY, internalChartInfo.m_dLowLimit);	// If chart over limits, update charting
            else
            {
                if(internalChartInfo.m_dHighLimit != -C_INFINITE)
                    internalChartInfo.m_dLowY = gex_min(internalChartInfo.m_dLowY, internalChartInfo.m_dHighLimit);

                internalChartInfo.m_dLowY = gex_min(internalChartInfo.m_dLowY, lfMin);
            }

            if(internalChartInfo.m_dHighLimit != -C_INFINITE)
                internalChartInfo.m_dHighY = gex_max(internalChartInfo.m_dHighY, internalChartInfo.m_dHighLimit);	// If chart over limits, update charting
            else
            {
                if(internalChartInfo.m_dLowLimit != C_INFINITE)
                    internalChartInfo.m_dHighY = gex_max(internalChartInfo.m_dHighY, internalChartInfo.m_dLowLimit);

                internalChartInfo.m_dHighY = gex_max(internalChartInfo.m_dHighY,lfMax);
            }

            break;

        case viewportOverData :
            // Chart has to be done over data (min & max)
            internalChartInfo.m_dLowY	= gex_min(internalChartInfo.m_dLowY,lfMin);
            internalChartInfo.m_dHighY	= gex_max(internalChartInfo.m_dHighY,lfMax);
            break;

        case viewportAdaptive :
            // Chart has to be done over maxi of both datapoints & limits
            if(internalChartInfo.m_dLowLimit != C_INFINITE)
                internalChartInfo.m_dLowY = gex_min(internalChartInfo.m_dLowY, internalChartInfo.m_dLowLimit);
            else if(internalChartInfo.m_dHighLimit != -C_INFINITE)
                internalChartInfo.m_dLowY = gex_min(internalChartInfo.m_dLowY, internalChartInfo.m_dHighLimit);

            if(internalChartInfo.m_dHighLimit != -C_INFINITE)
                internalChartInfo.m_dHighY = gex_max(internalChartInfo.m_dHighY, internalChartInfo.m_dHighLimit);
            else if(internalChartInfo.m_dLowLimit != C_INFINITE)
                internalChartInfo.m_dHighY = gex_max(internalChartInfo.m_dHighY, internalChartInfo.m_dLowLimit);

            internalChartInfo.m_dLowY	= gex_min(internalChartInfo.m_dLowY, lfMin);
            internalChartInfo.m_dHighY	= gex_max(internalChartInfo.m_dHighY, lfMax);

            if (internalChartInfo.m_dMarkerMin != C_INFINITE)
                internalChartInfo.m_dLowY = gex_min(internalChartInfo.m_dLowY, internalChartInfo.m_dMarkerMin);

            if (internalChartInfo.m_dMarkerMax != -C_INFINITE)
                internalChartInfo.m_dHighY = gex_max(internalChartInfo.m_dHighY, internalChartInfo.m_dMarkerMax);

            break;
    }

    // Determine min and max values for the viewport
    if(internalChartInfo.m_dLowLimit != C_INFINITE)
        internalChartInfo.m_dViewportLowY = gex_min(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dLowLimit);
    else if(internalChartInfo.m_dHighLimit != -C_INFINITE)
        internalChartInfo.m_dViewportLowY = gex_min(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dHighLimit);

    if(internalChartInfo.m_dHighLimit != -C_INFINITE)
        internalChartInfo.m_dViewportHighY = gex_max(internalChartInfo.m_dViewportHighY, internalChartInfo.m_dHighLimit);
    else if(internalChartInfo.m_dLowLimit != C_INFINITE)
        internalChartInfo.m_dViewportHighY = gex_max(internalChartInfo.m_dViewportHighY, internalChartInfo.m_dLowLimit);

    internalChartInfo.m_dViewportLowY = gex_min(internalChartInfo.m_dViewportLowY, lfMin);
    internalChartInfo.m_dViewportHighY = gex_max(internalChartInfo.m_dViewportHighY, lfMax);

    if (internalChartInfo.m_dMarkerMin != C_INFINITE)
        internalChartInfo.m_dViewportLowY = gex_min(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dMarkerMin);

    if (internalChartInfo.m_dMarkerMax != -C_INFINITE)
        internalChartInfo.m_dViewportHighY = gex_max(internalChartInfo.m_dViewportHighY, internalChartInfo.m_dMarkerMax);

    if (internalChartInfo.m_dLowSpecLimit != C_INFINITE
            && internalChartInfo.m_dLowSpecLimit > -F_INFINITE
            && internalChartInfo.m_dLowSpecLimit != -C_INFINITE
            && internalChartInfo.m_dLowSpecLimit < F_INFINITE)
        internalChartInfo.m_dViewportLowY  = gex_min(internalChartInfo.m_dViewportLowY , internalChartInfo.m_dLowSpecLimit);

    if (internalChartInfo.m_dHighSpecLimit != C_INFINITE
            && internalChartInfo.m_dHighSpecLimit > -F_INFINITE
            &&internalChartInfo.m_dHighSpecLimit != -C_INFINITE
            && internalChartInfo.m_dHighSpecLimit < F_INFINITE)
        internalChartInfo.m_dViewportHighY = gex_max(internalChartInfo.m_dViewportHighY, internalChartInfo.m_dHighSpecLimit);

    internalChartInfo.m_dViewportLowY = gex_min(internalChartInfo.m_dViewportLowY, lfMedian);
    internalChartInfo.m_dViewportLowY = gex_min(internalChartInfo.m_dViewportLowY, lfMean);
    internalChartInfo.m_dViewportLowY = gex_min(internalChartInfo.m_dViewportLowY, lfMean - 6 * lfSigma);

    internalChartInfo.m_dViewportHighY = gex_max(internalChartInfo.m_dViewportHighY, lfMedian);
    internalChartInfo.m_dViewportHighY = gex_max(internalChartInfo.m_dViewportHighY, lfMean);
    internalChartInfo.m_dViewportHighY = gex_max(internalChartInfo.m_dViewportHighY, lfMean + 6 * lfSigma);

    if (m_pChartOverlays)
    {
        // If Whisker is based on the percetiles..., add outliers!
        switch(internalChartInfo.m_pChart->iWhiskerMode)
        {
        case GEX_WHISKER_RANGE:
                break;
        default:
            //for(int iIndex = 0; iIndex < ptTestCell->ldSamplesExecs;iIndex++)
            for(int iIndex = 0; iIndex < ptTestCell->m_testResult.count();iIndex++)
            {
                if (ptTestCell->m_testResult.isValidResultAt(iIndex))
                {
                    if (ptTestCell->m_testResult.isMultiResultAt(iIndex))
                    {
                        for (int nCount = 0; nCount < ptTestCell->m_testResult.at(iIndex)->count(); ++nCount)
                        {
                            if(ptTestCell->m_testResult.at(iIndex)->isValidResultAt(nCount)){
                                lfValue = dCustomScaleFactor * ptTestCell->m_testResult.at(iIndex)->multiResultAt(nCount);

                                if(lfValue < lfLowWhisker || lfValue > lfHighWhisker)
                                    m_pXYChart->addScatterLayer(DoubleArray(&lfLayerIndex, 1), DoubleArray(&lfValue, 1), "", convertToChartDirectorSpot(internalChartInfo.m_pChart->iSpotStyle), 10, 0xFFC62A);
                            }
                        }
                    }
                    else
                    {
                        lfValue = dCustomScaleFactor * ptTestCell->m_testResult.resultAt(iIndex);

                       if(lfValue < lfLowWhisker || lfValue > lfHighWhisker)
                           m_pXYChart->addScatterLayer(DoubleArray(&lfLayerIndex, 1), DoubleArray(&lfValue, 1), "", convertToChartDirectorSpot(internalChartInfo.m_pChart->iSpotStyle), 10, 0xFFC62A);
                     }

                }
            }
            break;
        }
    }

    return true;
}





/////////////////////////////////////////////////////////////////////////////
// Retrieve the chart director symbol for plot accorind to the Gex combo
/////////////////////////////////////////////////////////////////////////////
Chart::SymbolType GexBoxPlotChart::convertToChartDirectorSpot(int nSpotIndex)
{
    Chart::SymbolType spotType = Chart::SquareSymbol;

    switch(nSpotIndex)
    {
        case 0	:	spotType = Chart::CircleSymbol;
                    break;

        case 1	:	spotType = Chart::SquareSymbol;
                    break;

        case 2	:	spotType = Chart::DiamondSymbol;
                    break;

        case 3	:	spotType = Chart::CrossSymbol;
                    break;

        case 4	:	spotType = Chart::Cross2Symbol;
                    break;

        case 5	:	spotType = Chart::TriangleSymbol;
                    break;

        case 6	:	spotType = Chart::InvertedTriangleSymbol;
                    break;

        case 7	:	spotType = Chart::LeftTriangleSymbol;
                    break;

        case 8	:	spotType = Chart::RightTriangleSymbol;
                    break;

        default :	break;
    }

    return spotType;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString makeTooltip()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
QString	GexBoxPlotChart::makeTooltip()
{
    QString		strTooltip;

    if (referenceTestX() && isVerticalViewportInitialized())
    {
        // If log scale, do not show the data pointed...don't know its location!
        if(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX || m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY)
            strTooltip = "BoxPlot Area";
    }

    return strTooltip;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setViewportModeFromChartMode(int nChartMode)
//
// Description	:	convert boxplot chart mode from Gex into viewport mode
//
///////////////////////////////////////////////////////////////////////////////////
void GexBoxPlotChart::setViewportModeFromChartMode(int nChartMode)
{
    int nViewportMode;

    switch (nChartMode)
    {
        case GEX_BOXPLOTTYPE_LIMITS		:	nViewportMode = viewportOverLimits;
                                            break;

        case GEX_BOXPLOTTYPE_RANGE		:	nViewportMode = viewportOverData;
                                            break;

        case GEX_BOXPLOTTYPE_ADAPTIVE	:	nViewportMode = viewportAdaptive;
                                            break;

        default							:	nViewportMode = viewportOverLimits;
    }

    setViewportMode(nViewportMode);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void plotSpecificScriptingCustomMarker(TestMarker * pTestMarker, int nColor)
//
// Description	:	Plot specific data for scripting custom markers
//
///////////////////////////////////////////////////////////////////////////////////
void GexBoxPlotChart::plotSpecificScriptingCustomMarker(TestMarker * pTestMarker, int nColor, CTest *ptTestCell, double dCustomScaleFactor)
{
    if(pTestMarker->iLayer == -1)
        return;// No need to add the scatter
    double dLayer	= pTestMarker->iLayer;
    double dValue	= pTestMarker->lfPos * dCustomScaleFactor;

    CGexFileInGroup cFile(NULL,0,"",0,0,"","","");	// in order to use FormatTestResultNoUnits(...)
    cFile.UpdateOptions(&ReportOptions);

    cFile.FormatTestResultNoUnits(&dValue, ptTestCell->res_scal);

    m_pXYChart->addScatterLayer(DoubleArray(&dLayer, 1),DoubleArray(&dValue, 1),"",Chart::Cross2Shape(0.3), 10, nColor);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void keepXTestViewport()
//
// Description	:	Keep wiewport for test X
//
///////////////////////////////////////////////////////////////////////////////////
void GexBoxPlotChart::keepXTestViewport()
{
    if (m_pChartOverlays && referenceTestX())
    {
        referenceTestX()->ptChartOptions->setCustomViewportX(true);
        referenceTestX()->ptChartOptions->setLowX(m_pChartOverlays->getViewportRectangle()[type()].lfLowY);
        referenceTestX()->ptChartOptions->setHighX(m_pChartOverlays->getViewportRectangle()[type()].lfHighY);
    }
}
