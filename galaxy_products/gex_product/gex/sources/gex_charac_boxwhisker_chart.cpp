///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_charac_boxwhisker_chart.h"
#include "gex_constants.h"
#include "interactive_charts.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include <gqtl_log.h>
#include "gex_box_plot_data.h"
#include "charac_box_whisker_template.h"
#include <gqtl_global.h>

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QString>
#include <QMap>

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport * gexReport;

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern double	ScalingPower(int iPower);

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexBoxWHiskerChart
//
// Description	:	chart class to draw the Box Whisker charts into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexCharacBoxWhiskerChart::GexCharacBoxWhiskerChart(int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays *pChartOverlays)
    : GexAbstractChart(GexAbstractChart::chartTypeCharacBoxWhisker, nSizeMode, lWizardParent, pChartOverlays)
{

}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexCharacBoxWhiskerChart::~GexCharacBoxWhiskerChart()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void makeTooltip()
//
// Description	:	generate the label tooltip's
//
///////////////////////////////////////////////////////////////////////////////////
QString GexCharacBoxWhiskerChart::makeTooltip()
{
    QString	strTooltip;

    return strTooltip;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void keepXTestViewport()
//
// Description	:	Keep wiewport for test X
//
///////////////////////////////////////////////////////////////////////////////////
void GexCharacBoxWhiskerChart::keepXTestViewport()
{
    if (m_pChartOverlays && referenceTestX())
    {
        referenceTestX()->ptChartOptions->setCustomViewportX(true);
        referenceTestX()->ptChartOptions->setLowX(m_pChartOverlays->getViewportRectangle()[type()].lfLowY);
        referenceTestX()->ptChartOptions->setHighX(m_pChartOverlays->getViewportRectangle()[type()].lfHighY);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void fitPlotArea()
//
// Description	:	fit the plot area depending on the axix thickness
//
///////////////////////////////////////////////////////////////////////////////////
void GexCharacBoxWhiskerChart::fitPlotArea()
{
    m_pXYChart->packPlotArea(0, top(), 0, bottom());

    setBottomMargin(bottomMargin() + m_pXYChart->xAxis()->getThickness());

    int nLeftMargin = leftMargin();
    int nTopMargin  =  topMargin();

    int nAreaWidth  = width() - horizontalMargin();
    int nAreaHeight = height() - verticalMargin();

    m_pXYChart->setPlotArea(nLeftMargin, nTopMargin, nAreaWidth, nAreaHeight);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setViewportModeFromChartMode()
//
// Description	:	convert chart mode from Gex into viewport mode
//
///////////////////////////////////////////////////////////////////////////////////
void GexCharacBoxWhiskerChart::setViewportModeFromChartMode(int nChartMode)
{
    int nViewportMode;

    switch (nChartMode)
    {
        case GEX_ADV_CHARAC_CHART_OVERLIMITS	:	nViewportMode = viewportOverLimits;
                                                    break;

        case GEX_ADV_CHARAC_CHART_OVERDATA		:	nViewportMode = viewportOverData;
                                                    break;

        case GEX_ADV_CHARAC_CHART_DATALIMITS	:	nViewportMode = viewportAdaptive;
                                                    break;

        default                                 :	nViewportMode = viewportOverLimits;
    }

    setViewportMode(nViewportMode);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void computeDataset()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexCharacBoxWhiskerChart::computeDataset()
{
    // Count the total number of VISIBLE layers
    CGexGroupOfFiles *	pGroup;
    CGexFileInGroup *	pFile;
    CTest *				ptTestCell;				// Pointer to test cell of groups2 or higher
    long				lTestNumber;
    long				lPinmapIndex;
    QString             lKey;
    int                 indexGroup = 0;

    {
        // Clear previous dataset
        mLayerDatasets.clean();
        mLayerDatasets.init(gexReport->getGroupsList().count());

        mMapConditionsColor.clear();

        QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());

        while (itGroupsList.hasNext())
        {
            // Find test cell: RESET list to ensure we scan list of the right group!
            // Check if pinmap index...
            pGroup			= itGroupsList.next();
            pFile			= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            lTestNumber		= referenceTestX()->lTestNumber;
            lPinmapIndex = referenceTestX()->lPinmapIndex;

            int nFillColor, nLineColor;
            Chart::SymbolType layerSymbol;

            if (gexReport->GetTestConditionsCount() > 0)
            {
                QString mainCondition   = gexReport->GetTestConditions(0);
                lKey  = pGroup->GetTestConditionsValue(mainCondition).toString();
            }
            else
                lKey = pGroup->strGroupName;

            if (m_pReportOptions->mBoxWhiskerTemplate &&
                m_pReportOptions->mBoxWhiskerTemplate->GetTopLevelAggregates().contains(lKey))
                nFillColor = m_pReportOptions->mBoxWhiskerTemplate->GetTopLevelColor(lKey).rgb() & 0xffffff;
            else if (mMapConditionsColor.contains(lKey))
                nFillColor = mMapConditionsColor.value(lKey);
            else
            {
                getLayerStyles(mMapConditionsColor.count()+1, NULL, nFillColor, nLineColor, layerSymbol);

                mMapConditionsColor.insert(lKey, nFillColor);
            }

            if(pFile && pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,referenceTestX()->strTestName) == 1 &&
               ptTestCell->m_testResult.count() > 0)
            {
                // If multiple charts (groups or overlays), scale based on 1st parameter in 1st group.
                if (ptTestCell->m_testResult.count() > 0 && mInternalChartInfo.m_bTestReferenceScaleFactor == false)
                {
                    mInternalChartInfo.m_pTestReferenceScaleX        = ptTestCell;
                    mInternalChartInfo.m_nTestReferenceScaleFactor   = ptTestCell->res_scal;
                    mInternalChartInfo.m_bTestReferenceScaleFactor   = true;
                }

                // Compute scale factor comparing to the ref group
                double lCustomScaleFactor = 1.0 / GS_POW(10.0, (ptTestCell->res_scal - mInternalChartInfo.m_nTestReferenceScaleFactor));
                // Apply the scale factor use for the test result
                lCustomScaleFactor /= ScalingPower(ptTestCell->res_scal);
                // Fill the dataset
                mLayerDatasets.addData(indexGroup, ptTestCell->lfSamplesMin * lCustomScaleFactor,
                                       ptTestCell->lfSamplesQuartile1 * lCustomScaleFactor,
                                       ptTestCell->lfSamplesQuartile2 * lCustomScaleFactor,
                                       ptTestCell->lfSamplesQuartile3 * lCustomScaleFactor,
                                       ptTestCell->lfSamplesMax * lCustomScaleFactor,
                                       nFillColor, pGroup->strGroupName);
            }
            else
                mLayerDatasets.addData(indexGroup, GEX_C_DOUBLE_NAN, GEX_C_DOUBLE_NAN,
                                       GEX_C_DOUBLE_NAN, GEX_C_DOUBLE_NAN,
                                       GEX_C_DOUBLE_NAN, nFillColor, pGroup->strGroupName);

            indexGroup++;
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
void GexCharacBoxWhiskerChart::buildOwnerChart()
{
    CGexGroupOfFiles *		pGroupX             = NULL;
    CGexFileInGroup *		pFileX              = NULL;
    CTest *					ptTestCellX         = NULL;
    long					lTestNumberX;
    long					lPinmapIndexX;

//    m_pXYChart->xAxis()->setIndent(false);

    for (mInternalChartInfo.m_nLayerIndex = 0; mInternalChartInfo.m_nLayerIndex < gexReport->getGroupsList().count(); mInternalChartInfo.m_nLayerIndex++)
    {
        mInternalChartInfo.m_nGroup = mInternalChartInfo.m_nLayerIndex+1;

        // Seek to the relevant Group to plot.
        pGroupX	= gexReport->getGroupsList().at(mInternalChartInfo.m_nLayerIndex);
        pFileX	= (pGroupX->pFilesList.isEmpty()) ? NULL : pGroupX->pFilesList.first();

        // Get test# & pinmap#
        lTestNumberX	= referenceTestX()->lTestNumber;
        lPinmapIndexX	= referenceTestX()->lPinmapIndex;

        if ((pFileX->FindTestCell(lTestNumberX, lPinmapIndexX, &ptTestCellX, false, false, referenceTestX()->strTestName) == 1))
        {
            // Draw title, labels
            drawLabels(pFileX, ptTestCellX);

            // Skip layers without data or hidden
            if (drawLayers(pFileX, ptTestCellX))
            {
                drawMarkers(pFileX, ptTestCellX);
                mInternalChartInfo.m_nVisibleLayerIndex++;
            }
        }
    }

    // Customize labels on x-axis
    drawXAxisLabels();

    // No right margin
    setRightMargin(1);

    // Set bottom margin to 20 pixels to show the 'www.mentor.com'
    setBottomMargin(20);

    // Scale the chart according to plotted data
    scaleChart();
}

void GexCharacBoxWhiskerChart::drawLabels(CGexFileInGroup * pFileX, CTest * pTestX)
{
    // this should never happened
    if (pFileX && pTestX)
    {
        double lCustomScaleXFactor = 1.0 / GS_POW(10.0, (pTestX->res_scal - mInternalChartInfo.m_nTestReferenceScaleFactor));

        if (mInternalChartInfo.m_nVisibleLayerIndex == 0)
        {
            QString     lLabel;
            QString     lUnits;

            // We have a chart title unless the chart size in only 200 pixel wide (SMALL chart mode)!
            if ((m_nSizeMode != GEX_CHARTSIZE_SMALL))
            {
                QString testName;
                gexReport->BuildTestNameString(pFileX, pTestX, testName);

                // Format test name to display
                testName = gexReport->buildDisplayName(testName, false);
                lLabel = QString("Test %1: %2").arg(pTestX->szTestLabel).arg(testName);

                if (mInternalChartInfo.m_bTestReferenceScaleFactor == false)
                    lLabel += "<*color=FF0000*> - No data samples<*/color*>";
            }

            // Multilayrs, check if we have assigned it a title...
            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bChartTitle)
                lLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strChartTitle;	// custom title.

            m_pXYChart->addTitle(lLabel.toLatin1().constData());

            // Set Y axis title
            lLabel  = "Units";

            if (mInternalChartInfo.m_pTestReferenceScaleX)
            {
                lUnits = mInternalChartInfo.m_pTestReferenceScaleX->GetScaledUnits(&lCustomScaleXFactor, ReportOptions.GetOption("dataprocessing","scaling").toString());

                if(lUnits.trimmed().length() > 0)
                    lLabel += " (" + lUnits + ")";
            }

            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY)
                lLabel += " - Log scale";

            if (mInternalChartInfo.m_bTestReferenceScaleFactor == false)
                lLabel += "<*color=FF0000*> - No data samples<*/color*>";

            // Check if we have a custom legend to overwrite default one
            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLegendY)
                lLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strAxisLegendY;

            m_pXYChart->yAxis()->setTitle(lLabel.toLatin1().constData());
        }
    }
}

bool GexCharacBoxWhiskerChart::drawLayers(CGexFileInGroup *pFileX, CTest *pTestX)
{
    // this should never happened
    if (pFileX == NULL  || pTestX == NULL)
        return false;

    // Set the labels on the x axis and the font to Arial
    m_pXYChart->xAxis()->setLabels(StringArray(mLayerDatasets.labels(), mLayerDatasets.size()));

    // Next layer: Box Whisker layer. Set the line width to 2 pixels
//    for (int index = 0; index < mLayerDatasets.size(); ++index)
//    {
//       /* BoxWhiskerLayer * pLayer = */m_pXYChart->addBoxWhiskerLayer(DoubleArray(&mLayerDatasets.dataQ3()[index], 1),
//                                       DoubleArray(&mLayerDatasets.dataQ1()[index], 1),
//                                       DoubleArray(&mLayerDatasets.dataQ4()[index], 1),
//                                       DoubleArray(&mLayerDatasets.dataQ0()[index], 1),
//                                       DoubleArray(&mLayerDatasets.dataQ2()[index], 1),
//                                       Chart::Transparent,
//                                       mLayerDatasets.colors()[index],
//                                       mLayerDatasets.colors()[index]);
////        pLayer->setLineWidth(2);
////        pLayer->setXData(index, index);
//    }
    BoxWhiskerLayer * pLayer = m_pXYChart->addBoxWhiskerLayer2(DoubleArray(mLayerDatasets.dataQ3(), mLayerDatasets.size()),
                                    DoubleArray(mLayerDatasets.dataQ1(), mLayerDatasets.size()),
                                    DoubleArray(mLayerDatasets.dataQ4(), mLayerDatasets.size()),
                                    DoubleArray(mLayerDatasets.dataQ0(), mLayerDatasets.size()),
                                    DoubleArray(mLayerDatasets.dataQ2(), mLayerDatasets.size()),
                                    IntArray(mLayerDatasets.colors(), mLayerDatasets.size()),
                                    0.5);

////     Set the line width
    pLayer->setLineWidth(2);
//    pLayer->setBorderColor();
//    pLayer->setXData(DoubleArray());

    // Compute min low and high X/Y values
    mInternalChartInfo.m_dLowY  = qMin(mInternalChartInfo.m_dLowY, mLayerDatasets.minData());
    mInternalChartInfo.m_dHighY = qMax(mInternalChartInfo.m_dHighY, mLayerDatasets.maxData());

    return true;
}

bool GexCharacBoxWhiskerChart::drawMarkers(CGexFileInGroup *pFileX, CTest *pTestX)
{
    if (drawMarkersLimits(pFileX, pTestX) == false)
        return false;

    return true;
}

bool GexCharacBoxWhiskerChart::drawMarkersLimits(CGexFileInGroup *pFileX, CTest *pTestX)
{
    QString         optionsScaling  = m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    QString         optionsMarkers  = m_pReportOptions->GetOption("adv_charac1","marker").toString();;
    QString         strLabel;
    QColor			cPenColor;
    int				cPenWidth;

    // this should never happened
    if (pFileX == NULL  || pTestX == NULL)
        return false;

    double dTmpLimit;

    // Compute low limit markers in X
    if(((pTestX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0))
    {
        // Low limit exists
        dTmpLimit = pTestX->GetCurrentLimitItem()->lfLowLimit;

        // If we have to keep values in normalized format, do not rescale!
        if (optionsScaling != "normalized")
        {
            // convert LowLimit to same scale as results:
            pFileX->FormatTestResultNoUnits(&dTmpLimit, pTestX->llm_scal);

            dTmpLimit *=  ScalingPower(pTestX->llm_scal);	// normalized
            dTmpLimit /=  ScalingPower(pTestX->res_scal);	// normalized
        }

        mInternalChartInfo.m_dLowLimitY = qMin(mInternalChartInfo.m_dLowLimitY, dTmpLimit);

        if((optionsMarkers.contains("limits")) || (mInternalChartInfo.m_pChart != NULL))
        {
            if(mInternalChartInfo.m_pChart != NULL)
            {
                cPenColor = mInternalChartInfo.m_pChart->limitsColor(false);
                cPenWidth = mInternalChartInfo.m_pChart->limitsLineWidth();
            }
            else
            {
                cPenColor = Qt::red;
                cPenWidth = 1;
            }
            // Request to show limits
            if(cPenWidth)
            {
                strLabel = QString("%1").arg(m_strLowLimit);

                addMarker(m_pXYChart->yAxis(), dTmpLimit, cPenColor.rgb() & 0xffffff, strLabel, cPenWidth, Chart::TopLeft);
            }
        }
    }

    // Compute high limit markers in X
    if(((pTestX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
    {
        // Low limit exists
        dTmpLimit = pTestX->GetCurrentLimitItem()->lfHighLimit;

        // If we have to keep values in normalized format, do not rescale!
        if (optionsScaling != "normalized")
        {
            // convert LowLimit to same scale as results:
            pFileX->FormatTestResultNoUnits(&dTmpLimit, pTestX->hlm_scal);

            dTmpLimit *=  ScalingPower(pTestX->hlm_scal);	// normalized
            dTmpLimit /=  ScalingPower(pTestX->res_scal);	// normalized
        }

        mInternalChartInfo.m_dHighLimitY = qMax(mInternalChartInfo.m_dHighLimitY, dTmpLimit);

        if((optionsMarkers.contains("limits")) || (mInternalChartInfo.m_pChart != NULL))
        {
            if(mInternalChartInfo.m_pChart != NULL)
            {
                cPenColor = mInternalChartInfo.m_pChart->limitsColor(false);
                cPenWidth = mInternalChartInfo.m_pChart->limitsLineWidth();
            }
            else
            {
                cPenColor = Qt::red;
                cPenWidth = 1;
            }
            // Request to show limits
            if(cPenWidth)
            {
                strLabel = QString("%1").arg(m_strHighLimit);

                addMarker(m_pXYChart->yAxis(), dTmpLimit, cPenColor.rgb() & 0xffffff, strLabel, cPenWidth, Chart::BottomLeft);
            }
        }
    }

    return true;
}

void GexCharacBoxWhiskerChart::drawXAxisLabels()
{
    if (gexReport->GetTestConditionsCount() > 0)
    {
        CDMLTable * cmdlTable = m_pXYChart->xAxis()->makeLabelTable();

        // Set the default top/bottom margins of the cells to 3 pixels
        cmdlTable->getStyle()->setMargin(0, 0, 3, 3);
        cmdlTable->getStyle()->setFontStyle("arialbd.ttf");

        while (cmdlTable->getRowCount() < gexReport->GetTestConditionsCount())
            cmdlTable->appendRow();

        int                 colIndex        = 0;
        int                 colWidth        = 1;
        QString             conditionName;
        QString             conditionValue;
        CGexGroupOfFiles *	pGroup;
        QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());

        // Insert new column to the left to display condition name
        cmdlTable->insertCol(0)->setMargin(2, 2, 0, 0);

        for (int rowIndex = 0; rowIndex < cmdlTable->getRowCount(); ++rowIndex)
        {
            colIndex = 0;
            itGroupsList.toFront();

            conditionName = gexReport->GetTestConditions(rowIndex);
            conditionValue.clear();

            // Set name of the condition
            cmdlTable->setText(colIndex, rowIndex, conditionName.toLatin1().constData());

            ++colIndex;

            while (itGroupsList.hasNext())
            {
                pGroup	= itGroupsList.next();

                if (conditionValue.isEmpty())
                {
                    conditionValue = pGroup->GetTestConditionsValue(conditionName).toString();
                    colWidth = 1;
                }
                else if (conditionValue != pGroup->GetTestConditionsValue(conditionName).toString())
                {
                    cmdlTable->setCell(colIndex, rowIndex, colWidth, 1, conditionValue.toLatin1().constData());

                    conditionValue = pGroup->GetTestConditionsValue(conditionName).toString();
                    colIndex += colWidth;
                    colWidth = 1;
                }
                else
                {
                    ++colWidth;
                }
            }

            // Write last column labels
            cmdlTable->setCell(colIndex, rowIndex, colWidth, 1, conditionValue.toLatin1().constData());
        }
    }
}

void GexCharacBoxWhiskerChart::scaleChart()
{
    double dLowX;
    double dLowY;
    double dHighX;
    double dHighY;
    double dExtraX;
    double dExtraY;

    // Check if charting window is test samples or test limits.
    switch(viewportMode())
    {
        case viewportOverLimits : // Chart over limits samples!
        default:
            if(mInternalChartInfo.m_dLowLimit != C_INFINITE)
                dLowX = mInternalChartInfo.m_dLowLimit;             // X Low limit exists
            else
                dLowX = mInternalChartInfo.m_dLowX;

            if(mInternalChartInfo.m_dLowLimitY != C_INFINITE)
                dLowY = mInternalChartInfo.m_dLowLimitY;            // Y Low limit exists
            else
                dLowY = mInternalChartInfo.m_dLowY;

            if(mInternalChartInfo.m_dHighLimit != -C_INFINITE)
                dHighX = mInternalChartInfo.m_dHighLimit ;          // X High limit exists
            else
                dHighX = mInternalChartInfo.m_dHighX;

            if(mInternalChartInfo.m_dHighLimitY != -C_INFINITE)
                dHighY = mInternalChartInfo.m_dHighLimitY;          // Y High limit exists
            else
                dHighY = mInternalChartInfo.m_dHighY;

            break;

        case viewportOverData : // Chart over data samples!
            dLowX   = mInternalChartInfo.m_dLowX;
            dHighX  = mInternalChartInfo.m_dHighX;
            dLowY   = mInternalChartInfo.m_dLowY;
            dHighY  = mInternalChartInfo.m_dHighY;
            break;

        case viewportAdaptive :
            dLowX = mInternalChartInfo.m_dLowLimit;             // X Low limit exists
            dLowX = qMin(dLowX,mInternalChartInfo.m_dLowX);

            dLowY = mInternalChartInfo.m_dLowLimitY;            // Y Low limit exists
            dLowY = qMin(dLowY, mInternalChartInfo.m_dLowY);

            dHighX = mInternalChartInfo.m_dHighLimit ;          // X High limit exists
            dHighX = qMax(dHighX, mInternalChartInfo.m_dHighX);

            dHighY = mInternalChartInfo.m_dHighLimitY;          // Y High limit exists
            dHighY = qMax(dHighY, mInternalChartInfo.m_dHighY);

            break;
    }

    // X axis scale. Add 1% of scale over and under chart window...
    // to ensure markers will be seen
    dExtraX = (dHighX - dLowX) * 0.05;

    // Y axis scale. Add 1% of scale over and under chart window...
    // to ensure markers will be seen
    dExtraY = (dHighY - dLowY) * 0.05;

    if (useTestCustomViewport())
    {
        // reset the current viewport
        resetViewportManager(false);

        // If custom X viewport (in scripting file, not in interactive mode), overwrite defaults window viewing
        if(referenceTestX()->ptChartOptions != NULL)
        {
            // If custom viewport in X (translated to Y in trend axis), overwrite defaults window viewing
            if(referenceTestX()->ptChartOptions->customViewportX())
            {
                dExtraX		= 0;
                dLowX       = referenceTestX()->ptChartOptions->lowX();
                dLowY		= referenceTestX()->ptChartOptions->highX();
            }
        }

        // If custom Y viewport (in scripting file, not in interactive mode), overwrite defaults window viewing
        if(referenceTestY()->ptChartOptions != NULL)
        {
            // If custom viewport in Y, overwrite defaults window viewing
            if(referenceTestY()->ptChartOptions->customViewportX())
            {
                dExtraY = 0;
                dLowY	= referenceTestY()->ptChartOptions->lowX();
                dHighY  = referenceTestY()->ptChartOptions->highX();
            }
        }
    }
    else if (m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].bForceViewport)
    {
        // Customer force viewport limit
        resetViewportManager(false);

        if (m_pChartOverlays->getViewportRectangle()[type()].lfLowX != -C_INFINITE)
            dLowX	= m_pChartOverlays->getViewportRectangle()[type()].lfLowX;

        if (m_pChartOverlays->getViewportRectangle()[type()].lfHighX != C_INFINITE)
            dHighX		= m_pChartOverlays->getViewportRectangle()[type()].lfHighX;

        if (m_pChartOverlays->getViewportRectangle()[type()].lfLowY != -C_INFINITE)
            dLowY	= m_pChartOverlays->getViewportRectangle()[type()].lfLowY;

        if (m_pChartOverlays->getViewportRectangle()[type()].lfHighY != C_INFINITE)
            dHighY		= m_pChartOverlays->getViewportRectangle()[type()].lfHighY;

        dExtraX	= 0;
        dExtraY	= 0;

        m_pChartOverlays->getViewportRectangle()[type()].bForceViewport = false;
    }

    // Define scales: if in Interactive charting, consider zoomin factor
    if(m_pChartOverlays == NULL || m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX == false)
    {
        if (dLowX != C_INFINITE && dHighX != -C_INFINITE)
        {
            dLowX	-= dExtraX;
            dHighX	+= dExtraX;
        }
    }

    if (m_pChartOverlays == NULL || m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY == false)
    {
        if (dLowY != C_INFINITE && dHighY != -C_INFINITE)
        {
            dLowY	-= dExtraY;
            dHighY	+= dExtraY;
        }
    }

    mInternalChartInfo.m_dViewportLowX  = qMin(mInternalChartInfo.m_dLowLimit, mInternalChartInfo.m_dLowX);
    mInternalChartInfo.m_dViewportHighX = qMax(mInternalChartInfo.m_dHighLimit, mInternalChartInfo.m_dHighX);
    mInternalChartInfo.m_dViewportLowY  = qMin(mInternalChartInfo.m_dLowLimitY, mInternalChartInfo.m_dLowY);
    mInternalChartInfo.m_dViewportHighY = qMax(mInternalChartInfo.m_dHighLimitY, mInternalChartInfo.m_dHighY);

    // Define viewport: if in Interactive charting, consider zoomin factor
    if (m_pChartOverlays)
    {
        // Initialize viewport
        if (isViewportInitialized() == false)
        {
            if (dLowX != C_INFINITE && dHighX != -C_INFINITE)
                initHorizontalViewport(mInternalChartInfo.m_dViewportLowX, mInternalChartInfo.m_dViewportHighX,dLowX, dHighX, 0.05);

            if (dLowY != C_INFINITE && dHighY != -C_INFINITE)
                initVerticalViewport(mInternalChartInfo.m_dViewportLowY, mInternalChartInfo.m_dViewportHighY, dLowY, dHighY, 0.05);
        }

        // Using ViewPortLeft and ViewPortWidth, get the start and end dates of the view port.
        computeXBounds(dLowX, dHighX);
        computeYBounds(dLowY, dHighY);
    }

    if (dLowX != C_INFINITE && dHighX != -C_INFINITE)
    {
        // Check if scale type is: Linear or Logarithmic
        if((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX &&
            ( m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeCharacBoxWhisker || m_pChartOverlays->getAppliedToChart() == -1)) || m_iXScaleType == 1)
        {
            // Log scale: then force it
            m_pXYChart->xAxis()->setLogScale(dLowX, dHighX);
            m_iXScaleType = 1;
        }
        else if((m_pChartOverlays && !m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX &&
               ( m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeCharacBoxWhisker || m_pChartOverlays->getAppliedToChart() == -1)) || m_iXScaleType == 0)
        {
            m_pXYChart->xAxis()->setLinearScale(dLowX, dHighX);
            m_iXScaleType = 0;
        }
    }

    if (dLowY != C_INFINITE && dHighY != -C_INFINITE)
    {
        // Check if scale type is: Linear or Logarithmic
        if((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
            (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeCharacBoxWhisker || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 1)
        {
            // Log scale: then force it
            m_pXYChart->yAxis()->setLogScale(dLowY, dHighY);
            m_iYScaleType = 1;
        }
        else if((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
               (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeCharacBoxWhisker || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 0)
        {
            m_pXYChart->yAxis()->setLinearScale(dLowY, dHighY);
            m_iYScaleType = 0;
        }
    }

    if (m_pChartOverlays)
    {
        m_pChartOverlays->getViewportRectangle()[type()].lfLowX	= dLowX;
        m_pChartOverlays->getViewportRectangle()[type()].lfLowY	= dLowY;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighX	= dHighX;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighY	= dHighY;
    }
}
