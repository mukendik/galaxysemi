///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_box_whisker_chart.h"
#include "gex_constants.h"
#include "interactive_charts.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include <gqtl_log.h>
#include "gex_box_plot_data.h"
#include <gqtl_global.h>
#include "gex_algorithms.h"

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
GexBoxWhiskerChart::GexBoxWhiskerChart(int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays *pChartOverlays)
    : GexAbstractChart(GexAbstractChart::chartTypeBoxWhisker, nSizeMode, lWizardParent, pChartOverlays)
{

}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexBoxWhiskerChart::~GexBoxWhiskerChart()
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
QString GexBoxWhiskerChart::makeTooltip()
{
    QString	strTooltip;

    return strTooltip;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setViewportModeFromChartMode()
//
// Description	:	convert chart mode from Gex into viewport mode
//
///////////////////////////////////////////////////////////////////////////////////
void GexBoxWhiskerChart::setViewportModeFromChartMode(int nChartMode)
{
    int nViewportMode;

    switch (nChartMode)
    {
        case GEX_ADV_CORR_OVERLIMITS	:	nViewportMode = viewportOverLimits;
                                            break;

        case GEX_ADV_CORR_OVERDATA		:	nViewportMode = viewportOverData;
                                            break;

        case GEX_ADV_CORR_DATALIMITS	:	nViewportMode = viewportAdaptive;
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
void GexBoxWhiskerChart::keepXTestViewport()
{
    if (m_pChartOverlays && referenceTestX())
    {
        referenceTestX()->ptChartOptions->setCustomViewportX(true);
        referenceTestX()->ptChartOptions->setLowX(m_pChartOverlays->getViewportRectangle()[type()].lfLowX);
        referenceTestX()->ptChartOptions->setHighX(m_pChartOverlays->getViewportRectangle()[type()].lfHighX);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void keepYTestViewport()
//
// Description	:	Keep wiewport for test Y
//
///////////////////////////////////////////////////////////////////////////////////
void GexBoxWhiskerChart::keepYTestViewport()
{
    if (m_pChartOverlays && referenceTestY())
    {
        referenceTestY()->ptChartOptions->setCustomViewportX(true);
        referenceTestY()->ptChartOptions->setLowX(m_pChartOverlays->getViewportRectangle()[type()].lfLowY);
        referenceTestY()->ptChartOptions->setHighX(m_pChartOverlays->getViewportRectangle()[type()].lfHighY);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void useTestCustomViewport()
//
// Description	:	test if we can use the custom viewport
//
///////////////////////////////////////////////////////////////////////////////////
bool GexBoxWhiskerChart::useTestCustomViewport()
{
    if (canUseCustomViewport() && referenceTestY() && referenceTestY()->ptChartOptions &&
        (referenceTestY()->ptChartOptions->customViewportX() || referenceTestY()->ptChartOptions->customViewportY()))
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
void GexBoxWhiskerChart::resetViewportManager(bool bEraseCustomViewport)
{
    if (bEraseCustomViewport && referenceTestY() && referenceTestY()->ptChartOptions)
    {
        referenceTestY()->ptChartOptions->setCustomViewportX(false);
        referenceTestY()->ptChartOptions->setCustomViewportY(false);
    }

    GexAbstractChart::resetViewportManager(bEraseCustomViewport);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void buildOwnerChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexBoxWhiskerChart::buildOwnerChart()
{
    // Count the total number of VISIBLE layers
    CGexGroupOfFiles *		pGroupX             = NULL;
    CGexFileInGroup *		pFileX              = NULL;
    CGexGroupOfFiles *		pGroupY             = NULL;
    CGexFileInGroup *		pFileY              = NULL;
    CTest *					ptTestCellX         = NULL;
    CTest *					ptTestCellY         = NULL;
    long					lTestNumberX;
    long					lPinmapIndexX;
    long					lTestNumberY;
    long					lPinmapIndexY;
    Orientation             whiskerOrientation  = GexBoxWhiskerChart::Vertical;

    if (m_pReportOptions->GetOption("adv_correlation", "boxwhisker_orientation").toString() == "horizontal")
        whiskerOrientation = GexBoxWhiskerChart::Horizontal;

    m_pXYChart->xAxis()->setIndent(false);

    mInternalChartInfo.m_nVisibleLayerIndex = 0;

    if (m_pChartOverlays)
    {
        mInternalChartInfo.m_nGroup = 1;

        for (mInternalChartInfo.m_nLayerIndex = 0; mInternalChartInfo.m_nLayerIndex < m_pChartOverlays->chartsList().count(); mInternalChartInfo.m_nLayerIndex++)
        {
            // Interactive mode: plot layers defined in the layer list
            mInternalChartInfo.m_pChart = m_pChartOverlays->chartsList().at(mInternalChartInfo.m_nLayerIndex);

            // Seek to the relevant Group to plot.
            if ((mInternalChartInfo.m_pChart->iGroupX < 0) || (mInternalChartInfo.m_pChart->iGroupX >= gexReport->getGroupsList().size()))
                pGroupX = NULL;
            else
                pGroupX	= gexReport->getGroupsList().at(mInternalChartInfo.m_pChart->iGroupX);
            pFileX	= (pGroupX->pFilesList.isEmpty()) ? NULL : pGroupX->pFilesList.first();

            // Get test# & pinmap#
            lTestNumberX	= mInternalChartInfo.m_pChart->iTestNumberX;
            lPinmapIndexX	= mInternalChartInfo.m_pChart->iPinMapX;

            // Seek to the relevant Group to plot.
            if ((mInternalChartInfo.m_pChart->iGroupY < 0) || (mInternalChartInfo.m_pChart->iGroupY >= gexReport->getGroupsList().size()))
                pGroupY = NULL;
            else
                pGroupY	= gexReport->getGroupsList().at(mInternalChartInfo.m_pChart->iGroupY);
            pFileY	= (pGroupY->pFilesList.isEmpty()) ? NULL : pGroupY->pFilesList.first();

            // Get test# & pinmap#
            lTestNumberY	= mInternalChartInfo.m_pChart->iTestNumberY;
            lPinmapIndexY	= mInternalChartInfo.m_pChart->iPinMapY;

            if ((pFileX->FindTestCell(lTestNumberX, lPinmapIndexX, &ptTestCellX,
                                      false, false,
                                      mInternalChartInfo.m_pChart->strTestNameX) == 1) &&
                (pFileY->FindTestCell(lTestNumberY, lPinmapIndexY, &ptTestCellY,
                                      false, false,
                                      mInternalChartInfo.m_pChart->strTestNameY) == 1))
            {
                // Draw title, labels
                drawLabels(pFileX, ptTestCellX, pFileY, ptTestCellY);

                // Skip layers without data or hidden
                if (mInternalChartInfo.m_pChart->bVisible && drawLayers(pFileX, ptTestCellX, pFileY, ptTestCellY))
                {
                    drawMarkers(pFileX, ptTestCellX, pFileY, ptTestCellY);
                    mInternalChartInfo.m_nVisibleLayerIndex++;
                }
            }
        }
    }
    else
    {
        for (mInternalChartInfo.m_nLayerIndex = 0; mInternalChartInfo.m_nLayerIndex < gexReport->getGroupsList().count(); mInternalChartInfo.m_nLayerIndex++)
        {
            mInternalChartInfo.m_nGroup = mInternalChartInfo.m_nLayerIndex+1;

            // Seek to the relevant Group to plot.
            pGroupX	= gexReport->getGroupsList().at(mInternalChartInfo.m_nLayerIndex);
            pFileX	= (pGroupX->pFilesList.isEmpty()) ? NULL : pGroupX->pFilesList.first();

            // Get test# & pinmap#
            lTestNumberX	= referenceTestX()->lTestNumber;
            lPinmapIndexX	= referenceTestX()->lPinmapIndex;

            // Seek to the relevant Group to plot.
            pGroupY	= gexReport->getGroupsList().at(mInternalChartInfo.m_nLayerIndex);
            pFileY	= (pGroupY->pFilesList.isEmpty()) ? NULL : pGroupY->pFilesList.first();

            // Get test# & pinmap#
            lTestNumberY	= referenceTestY()->lTestNumber;
            lPinmapIndexY	= referenceTestY()->lPinmapIndex;

            if ((pFileX->FindTestCell(lTestNumberX, lPinmapIndexX, &ptTestCellX, false, false, referenceTestX()->strTestName) == 1) &&
                (pFileY->FindTestCell(lTestNumberY, lPinmapIndexY, &ptTestCellY, false, false, referenceTestY()->strTestName) == 1))
            {
                // Draw title, labels
                drawLabels(pFileX, ptTestCellX, pFileY, ptTestCellY);

                // Skip layers without data or hidden
                if (drawLayers(pFileX, ptTestCellX, pFileY, ptTestCellY))
                {
                    drawMarkers(pFileX, ptTestCellX, pFileY, ptTestCellY);
                    mInternalChartInfo.m_nVisibleLayerIndex++;
                }
            }
        }
    }

    scaleChart();

    // Make boxwhisker horizontal if option set
    if(whiskerOrientation == GexBoxWhiskerChart::Horizontal)
        m_pXYChart->swapXY();
}

void GexBoxWhiskerChart::drawLabels(CGexFileInGroup * pFileX, CTest * pTestX, CGexFileInGroup * pFileY, CTest * pTestY)
{
    // this should never happened
    if (pFileX && pTestX && pFileY && pTestY)
    {
        //	OPTIONS
        QStringList lOptionsMarkers = m_pReportOptions->GetOption("adv_probabilityplot", "marker").toString().split("|");
        QString     scalingOption = ReportOptions.GetOption("dataprocessing","scaling").toString();

        double lCustomScaleXFactor = 1.0 / GS_POW(10.0, (pTestX->res_scal - mInternalChartInfo.m_nTestReferenceScaleFactor));
        double lCustomScaleYFactor = 1.0 / GS_POW(10.0, (pTestY->res_scal - mInternalChartInfo.m_nTestReferenceScaleFactorY));

        if (mInternalChartInfo.m_nVisibleLayerIndex == 0)
        {
            QString     lLabel;
            QString     lUnits;

            // We have a chart title unless the chart size in only 200 pixel wide (SMALL chart mode)!
            if ((m_nSizeMode != GEX_CHARTSIZE_SMALL) && (lOptionsMarkers.contains("test_name")))
                lLabel = "Box whisker / BiVariate plot";

            // Multilayrs, check if we have assigned it a title...
            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bChartTitle)
                lLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strChartTitle;	// custom title.

            m_pXYChart->addTitle(lLabel.toLatin1().constData());

            // Set X axis title
            lLabel  = "T";
            lLabel += pTestX->szTestLabel;
            lLabel += " - " + pTestX->strTestName;

            if (mInternalChartInfo.m_pTestReferenceScaleX)
            {
                lUnits = mInternalChartInfo.m_pTestReferenceScaleX->GetScaledUnits(&lCustomScaleXFactor, scalingOption);

                if(lUnits.length() > 0)
                    lLabel += "(" + lUnits + ")";
            }

            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX)
                lLabel += " - Log scale";

            if (mInternalChartInfo.m_bTestReferenceScaleFactor == false)
                lLabel += "<*color=FF0000*> - No data samples<*/color*>";

            // Check if we have a custom legend to overwrite default one
            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLegendX)
                lLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strAxisLegendX;

            m_pXYChart->xAxis()->setTitle(lLabel.toLatin1().constData());

            // Set Y axis title
            lLabel  = "T";
            lLabel += pTestY->szTestLabel;
            lLabel += " - " + pTestY->strTestName;

            if (mInternalChartInfo.m_pTestReferenceScaleY)
            {
                lUnits = mInternalChartInfo.m_pTestReferenceScaleY->GetScaledUnits(&lCustomScaleYFactor, scalingOption);

                if(lUnits.length() > 0)
                    lLabel += "(" + lUnits + ")";
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

bool GexBoxWhiskerChart::drawLayers(CGexFileInGroup * pFileX, CTest * pTestX, CGexFileInGroup * pFileY, CTest * pTestY)
{
    // this should never happened
    if (pFileX == NULL  || pTestX == NULL || pFileY == NULL || pTestY == NULL)
        return false;

    if (mLayerDatasets.contains(mInternalChartInfo.m_nLayerIndex))
    {
        const CGexBoxPlotData * pDataset = mLayerDatasets.value(mInternalChartInfo.m_nLayerIndex);

        int nFillColor, nLineColor;
        Chart::SymbolType layerSymbol;

        if (mInternalChartInfo.m_pChart)
            getLayerStyles(mInternalChartInfo.m_nLayerIndex+1, mInternalChartInfo.m_pChart, nFillColor, nLineColor, layerSymbol);
        else
            getLayerStyles(mInternalChartInfo.m_nGroup, mInternalChartInfo.m_pChart, nFillColor, nLineColor, layerSymbol);

        // Add Box whisker layer
        BoxWhiskerLayer * pLayer = m_pXYChart->addBoxWhiskerLayer(DoubleArray(pDataset->dataQ3(), pDataset->size()),
                                                                    DoubleArray(pDataset->dataQ1(), pDataset->size()),
                                                                    DoubleArray(pDataset->dataQ4(), pDataset->size()),
                                                                    DoubleArray(pDataset->dataQ0(), pDataset->size()),
                                                                    DoubleArray(pDataset->dataQ2(), pDataset->size()), nFillColor);

        // Add X data to layer
        pLayer->setXData(DoubleArray(pDataset->dataX(), pDataset->size()));
        pLayer->setDataWidth(25);

        // Compute min low and high X/Y values
        mInternalChartInfo.m_dLowY  = qMin(mInternalChartInfo.m_dLowY, pDataset->minData());
        mInternalChartInfo.m_dHighY = qMax(mInternalChartInfo.m_dHighY, pDataset->maxData());

        mInternalChartInfo.m_dLowX  = qMin(mInternalChartInfo.m_dLowX, pDataset->dataX()[0]);
        mInternalChartInfo.m_dHighX = qMax(mInternalChartInfo.m_dHighX, pDataset->dataX()[pDataset->size()-1]);

        return true;
    }

    return false;
}

bool GexBoxWhiskerChart::drawMarkers(CGexFileInGroup * pFileX, CTest * pTestX, CGexFileInGroup * pFileY, CTest * pTestY)
{
    if (drawMarkersLimits(pFileX, pTestX, pFileY, pTestY) == false)
        return false;

    if (drawMarkersStats(pFileX, pTestX, pFileY, pTestY) == false)
        return false;

    return true;
}

bool GexBoxWhiskerChart::drawMarkersLimits(CGexFileInGroup * pFileX, CTest * pTestX, CGexFileInGroup * pFileY, CTest * pTestY)
{
    QString         optionsScaling  = m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    QStringList     optionsMarkers  = m_pReportOptions->GetOption("adv_correlation", "marker").toString().split("|");
    bool            bIsMultiLayer	= isMultiLayers();
    QString         strLabel;
    QColor			cPenColor;
    int				cPenWidth;

    // this should never happened
    if (pFileX == NULL  || pTestX == NULL || pFileY == NULL || pTestY == NULL)
        return false;

    if (mLayerDatasets.contains(mInternalChartInfo.m_nLayerIndex))
    {
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

            mInternalChartInfo.m_dLowLimit = qMin(mInternalChartInfo.m_dLowLimit, dTmpLimit);

            if((optionsMarkers.contains("limits")) || (mInternalChartInfo.m_pChart != NULL))
            {
                if(mInternalChartInfo.m_pChart != NULL)
                {
                    cPenColor = mInternalChartInfo.m_pChart->limitsColor(bIsMultiLayer);
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
                    strLabel = QString("X: %1").arg(m_strLowLimit);

                    addMarker(m_pXYChart->xAxis(), dTmpLimit, cPenColor.rgb() & 0xffffff, strLabel, cPenWidth, Chart::BottomRight, false);
                }
            }
        }

        if(((pTestX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0))
        {
            // Low limit exists
            dTmpLimit = pTestX->lfLowSpecLimit;

            // If we have to keep values in normalized format, do not rescale!
            if (optionsScaling != "normalized")
            {
                // convert LowLimit to same scale as results:
                pFileX->FormatTestResultNoUnits(&dTmpLimit, pTestX->llm_scal);

                dTmpLimit *=  ScalingPower(pTestX->llm_scal);	// normalized
                dTmpLimit /=  ScalingPower(pTestX->res_scal);	// normalized
            }

            if((optionsMarkers.contains("speclimits")) || (mInternalChartInfo.m_pChart != NULL))
            {
                if(mInternalChartInfo.m_pChart != NULL)
                {
                    cPenColor = mInternalChartInfo.m_pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = optionsMarkers.contains("speclimits") ? mInternalChartInfo.m_pChart->limitsLineWidth() : 0;
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    strLabel = QString("X: %1").arg(m_strLowSpecLimit);

                    addMarker(m_pXYChart->xAxis(), dTmpLimit, cPenColor.rgb() & 0xffffff, strLabel, cPenWidth, Chart::BottomRight, false);
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

            mInternalChartInfo.m_dHighLimit = qMax(mInternalChartInfo.m_dHighLimit, dTmpLimit);

            if((optionsMarkers.contains("limits")) || (mInternalChartInfo.m_pChart != NULL))
            {
                if(mInternalChartInfo.m_pChart != NULL)
                {
                    cPenColor = mInternalChartInfo.m_pChart->limitsColor(bIsMultiLayer);
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
                    strLabel = QString("X: %1").arg(m_strHighLimit);

                    addMarker(m_pXYChart->xAxis(), dTmpLimit, cPenColor.rgb() & 0xffffff, strLabel, cPenWidth, Chart::BottomLeft, false);
                }
            }
        }

        // Compute high limit markers in X
        if(((pTestX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0))
        {
            // Low limit exists
            dTmpLimit = pTestX->lfHighSpecLimit;

            // If we have to keep values in normalized format, do not rescale!
            if (optionsScaling != "normalized")
            {
                // convert LowLimit to same scale as results:
                pFileX->FormatTestResultNoUnits(&dTmpLimit, pTestX->hlm_scal);

                dTmpLimit *=  ScalingPower(pTestX->hlm_scal);	// normalized
                dTmpLimit /=  ScalingPower(pTestX->res_scal);	// normalized
            }

            if((optionsMarkers.contains("speclimits")) || (mInternalChartInfo.m_pChart != NULL))
            {
                if(mInternalChartInfo.m_pChart != NULL)
                {
                    cPenColor = mInternalChartInfo.m_pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = optionsMarkers.contains("speclimits") ? mInternalChartInfo.m_pChart->limitsLineWidth() : 0;
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    strLabel = QString("X: %1").arg(m_strHighSpecLimit);

                    addMarker(m_pXYChart->xAxis(), dTmpLimit, cPenColor.rgb() & 0xffffff, strLabel, cPenWidth, Chart::BottomLeft, false);
                }
            }
        }

        // Compute low limit markers in Y
        if(((pTestY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0))
        {
            // Low limit exists
            dTmpLimit = pTestY->GetCurrentLimitItem()->lfLowLimit;

            // If we have to keep values in normalized format, do not rescale!
            if (optionsScaling != "normalized")
            {
                // convert LowLimit to same scale as results:
                pFileY->FormatTestResultNoUnits(&dTmpLimit, pTestY->llm_scal);

                dTmpLimit *=  ScalingPower(pTestY->llm_scal);	// normalized
                dTmpLimit /=  ScalingPower(pTestY->res_scal);	// normalized
            }

            mInternalChartInfo.m_dLowLimitY = qMin(mInternalChartInfo.m_dLowLimitY, dTmpLimit);

            if((optionsMarkers.contains("limits")) || (mInternalChartInfo.m_pChart != NULL))
            {
                if(mInternalChartInfo.m_pChart != NULL)
                {
                    cPenColor = mInternalChartInfo.m_pChart->limitsColor(bIsMultiLayer);
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
                    strLabel = QString("Y: %1").arg(m_strLowLimit);

                    addMarker(m_pXYChart->yAxis(), dTmpLimit, cPenColor.rgb() & 0xffffff, strLabel, cPenWidth, Chart::BottomRight, false);
                }
            }
        }

        if(((pTestY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0))
        {
            // Low limit exists
            dTmpLimit = pTestY->lfLowSpecLimit;

            // If we have to keep values in normalized format, do not rescale!
            if (optionsScaling != "normalized")
            {
                // convert LowLimit to same scale as results:
                pFileY->FormatTestResultNoUnits(&dTmpLimit, pTestY->llm_scal);

                dTmpLimit *=  ScalingPower(pTestY->llm_scal);	// normalized
                dTmpLimit /=  ScalingPower(pTestY->res_scal);	// normalized
            }

            if((optionsMarkers.contains("speclimits")) || (mInternalChartInfo.m_pChart != NULL))
            {
                if(mInternalChartInfo.m_pChart != NULL)
                {
                    cPenColor = mInternalChartInfo.m_pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = optionsMarkers.contains("speclimits") ? mInternalChartInfo.m_pChart->limitsLineWidth() : 0;
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    strLabel = QString("Y: %1").arg(m_strLowSpecLimit);

                    addMarker(m_pXYChart->yAxis(), dTmpLimit, cPenColor.rgb() & 0xffffff, strLabel, cPenWidth, Chart::BottomRight, false);
                }
            }
        }

        // Compute high limit markers in Y
        if(((pTestY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
        {
            // Low limit exists
            dTmpLimit = pTestY->GetCurrentLimitItem()->lfHighLimit;

            // If we have to keep values in normalized format, do not rescale!
            if (optionsScaling != "normalized")
            {
                // convert LowLimit to same scale as results:
                pFileY->FormatTestResultNoUnits(&dTmpLimit, pTestY->hlm_scal);

                dTmpLimit *=  ScalingPower(pTestY->hlm_scal);	// normalized
                dTmpLimit /=  ScalingPower(pTestY->res_scal);	// normalized
            }

            mInternalChartInfo.m_dHighLimitY = qMax(mInternalChartInfo.m_dHighLimitY, dTmpLimit);

            if((optionsMarkers.contains("limits")) || (mInternalChartInfo.m_pChart != NULL))
            {
                if(mInternalChartInfo.m_pChart != NULL)
                {
                    cPenColor = mInternalChartInfo.m_pChart->limitsColor(bIsMultiLayer);
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
                    strLabel = QString("Y: %1").arg(m_strHighLimit);

                    addMarker(m_pXYChart->yAxis(), dTmpLimit, cPenColor.rgb() & 0xffffff, strLabel, cPenWidth, Chart::BottomLeft, false);
                }
            }
        }

        if(((pTestY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0))
        {
            // Low limit exists
            dTmpLimit = pTestY->lfHighSpecLimit;

            // If we have to keep values in normalized format, do not rescale!
            if (optionsScaling != "normalized")
            {
                // convert LowLimit to same scale as results:
                pFileY->FormatTestResultNoUnits(&dTmpLimit, pTestY->hlm_scal);

                dTmpLimit *=  ScalingPower(pTestY->hlm_scal);	// normalized
                dTmpLimit /=  ScalingPower(pTestY->res_scal);	// normalized
            }

            if((optionsMarkers.contains("speclimits")) || (mInternalChartInfo.m_pChart != NULL))
            {
                if(mInternalChartInfo.m_pChart != NULL)
                {
                    cPenColor = mInternalChartInfo.m_pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = optionsMarkers.contains("speclimits") ? mInternalChartInfo.m_pChart->limitsLineWidth() : 0;
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    strLabel = QString("Y: %1").arg(m_strHighSpecLimit);

                    addMarker(m_pXYChart->yAxis(), dTmpLimit, cPenColor.rgb() & 0xffffff, strLabel, cPenWidth, Chart::BottomLeft, false);
                }
            }
        }


        return true;
    }

    return false;
}

bool GexBoxWhiskerChart::drawMarkersStats(CGexFileInGroup * pFileX, CTest * pTestX, CGexFileInGroup * pFileY, CTest * pTestY)
{
    QString         optionsScaling  = m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    QStringList     optionsMarkers  = m_pReportOptions->GetOption("adv_correlation", "marker").toString().split("|");
    bool            bIsMultiLayer	= isMultiLayers();
    QString         strLabel;
    QColor			cPenColor;
    int				cPenWidth;

    // this should never happened
    if (pFileX == NULL  || pTestX == NULL || pFileY == NULL || pTestY == NULL)
        return false;

    if (mLayerDatasets.contains(mInternalChartInfo.m_nLayerIndex))
    {
        double dMeanX;
        double dMeanY;
        double dSigmaX;
        double dSigmaY;
        double dMedianX;
        double dMedianY;

        // Compute Mean
        dMeanX = pTestX->lfMean;
        pFileX->FormatTestResultNoUnits(&dMeanX, pTestX->res_scal);

        dMeanY = pTestY->lfMean;
        pFileY->FormatTestResultNoUnits(&dMeanY, pTestY->res_scal);

        // Compute Sigma
        dSigmaX = pTestX->lfSigma;
        pFileX->FormatTestResultNoUnits(&dSigmaX, pTestX->res_scal);

        dSigmaY = pTestY->lfSigma;
        pFileY->FormatTestResultNoUnits(&dSigmaY, pTestY->res_scal);

        // Compute Median
        if (pTestX->lfSamplesQuartile2 != -C_INFINITE)
        {
            dMedianX = pTestX->lfSamplesQuartile2;
            pFileX->FormatTestResultNoUnits(&dMedianX, pTestX->res_scal);
        }

        if (pTestY->lfSamplesQuartile2 != -C_INFINITE)
        {
            dMedianY = pTestY->lfSamplesQuartile2;
            pFileY->FormatTestResultNoUnits(&dMedianY, pTestY->res_scal);
        }

        // If request to show the Min XY markers
        if(optionsMarkers.contains("min") || mInternalChartInfo.m_pChart != NULL)
        {
            if(mInternalChartInfo.m_pChart != NULL)
            {
                cPenColor = mInternalChartInfo.m_pChart->minColor(bIsMultiLayer);
                cPenWidth = mInternalChartInfo.m_pChart->minLineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	* pLayerStyle = NULL;
                if ((mInternalChartInfo.m_nGroup >= 1 && mInternalChartInfo.m_nGroup <= m_pReportOptions->pLayersStyleList.count()))
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(mInternalChartInfo.m_nGroup-1);
                    cPenColor	= pLayerStyle->minColor(bIsMultiLayer);
                    cPenWidth	= pLayerStyle->minLineWidth();

                    if(!cPenWidth)
                        cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                }
                else
                {
                    cPenColor = Qt::blue;
                    cPenWidth = 1;
                }
            }

            if(cPenWidth)
            {
                double dMinX = pTestX->lfMin;
                pFileX->FormatTestResultNoUnits(&dMinX, pTestX->res_scal);

                double dMinY = pTestY->lfMin;
                pFileY->FormatTestResultNoUnits(&dMinY, pTestY->res_scal);

                addMarker(m_pXYChart->xAxis(), dMinX, cPenColor.rgb() & 0xffffff, "X: Min", cPenWidth, Chart::TopRight, false);
                addMarker(m_pXYChart->yAxis(), dMinY, cPenColor.rgb() & 0xffffff, "Y: Min", cPenWidth, Chart::TopRight, false);

                mInternalChartInfo.m_dMarkerMin = gex_min(mInternalChartInfo.m_dMarkerMin, dMinX);
                mInternalChartInfo.m_dMarkerMax = gex_max(mInternalChartInfo.m_dMarkerMax, dMinY);

                mInternalChartInfo.m_dMarkerMinY = gex_min(mInternalChartInfo.m_dMarkerMinY, dMinX);
                mInternalChartInfo.m_dMarkerMaxY = gex_max(mInternalChartInfo.m_dMarkerMaxY, dMinY);
            }
        }

        // If request to show the Max XY markers
        if(optionsMarkers.contains("max") || mInternalChartInfo.m_pChart != NULL)
        {
            if(mInternalChartInfo.m_pChart != NULL)
            {
                cPenColor = mInternalChartInfo.m_pChart->maxColor(bIsMultiLayer);
                cPenWidth = mInternalChartInfo.m_pChart->maxLineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	* pLayerStyle = NULL;
                if ((mInternalChartInfo.m_nGroup >= 1 && mInternalChartInfo.m_nGroup <= m_pReportOptions->pLayersStyleList.count()))
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(mInternalChartInfo.m_nGroup-1);
                    cPenColor	= pLayerStyle->maxColor(bIsMultiLayer);
                    cPenWidth	= pLayerStyle->maxLineWidth();

                    if(!cPenWidth)
                        cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                }
                else
                {
                    cPenColor = Qt::blue;
                    cPenWidth = 1;
                }
            }

            if(cPenWidth)
            {
                double dMaxX = pTestX->lfMax;
                pFileX->FormatTestResultNoUnits(&dMaxX, pTestX->res_scal);

                double dMaxY = pTestY->lfMax;
                pFileY->FormatTestResultNoUnits(&dMaxY, pTestY->res_scal);

                addMarker(m_pXYChart->xAxis(), dMaxX, cPenColor.rgb() & 0xffffff, "X: Max", cPenWidth, Chart::TopRight, false);
                addMarker(m_pXYChart->yAxis(), dMaxY, cPenColor.rgb() & 0xffffff, "Y: Max", cPenWidth, Chart::TopRight, false);

                mInternalChartInfo.m_dMarkerMin = gex_min(mInternalChartInfo.m_dMarkerMin, dMaxX);
                mInternalChartInfo.m_dMarkerMax = gex_max(mInternalChartInfo.m_dMarkerMax, dMaxX);

                mInternalChartInfo.m_dMarkerMinY = gex_min(mInternalChartInfo.m_dMarkerMinY, dMaxY);
                mInternalChartInfo.m_dMarkerMaxY = gex_max(mInternalChartInfo.m_dMarkerMaxY, dMaxY);
            }
        }

        // If request to show the Mean XY markers
        if(optionsMarkers.contains("mean") || mInternalChartInfo.m_pChart != NULL)
        {
            if(mInternalChartInfo.m_pChart != NULL)
            {
                cPenColor = mInternalChartInfo.m_pChart->meanColor(bIsMultiLayer);
                cPenWidth = mInternalChartInfo.m_pChart->meanLineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	* pLayerStyle = NULL;
                if ((mInternalChartInfo.m_nGroup >= 1 && mInternalChartInfo.m_nGroup <= m_pReportOptions->pLayersStyleList.count()))
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(mInternalChartInfo.m_nGroup-1);
                    cPenColor	= pLayerStyle->meanColor(bIsMultiLayer);
                    cPenWidth	= pLayerStyle->meanLineWidth();

                    if(!cPenWidth)
                        cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                }
                else
                {
                    cPenColor = Qt::blue;
                    cPenWidth = 1;
                }
            }

            if(cPenWidth)
            {
                addMarker(m_pXYChart->xAxis(), dMeanX, cPenColor.rgb() & 0xffffff, "X: Mean", cPenWidth, Chart::TopRight, false);
                addMarker(m_pXYChart->yAxis(), dMeanY, cPenColor.rgb() & 0xffffff, "Y: Mean", cPenWidth, Chart::TopRight, false);

                mInternalChartInfo.m_dMarkerMin = gex_min(mInternalChartInfo.m_dMarkerMin, dMeanX);
                mInternalChartInfo.m_dMarkerMax = gex_max(mInternalChartInfo.m_dMarkerMax, dMeanX);

                mInternalChartInfo.m_dMarkerMinY = gex_min(mInternalChartInfo.m_dMarkerMinY, dMeanY);
                mInternalChartInfo.m_dMarkerMaxY = gex_max(mInternalChartInfo.m_dMarkerMaxY, dMeanY);
            }
        }

        // If request to show the Median XY markers
        if(optionsMarkers.contains("median") || mInternalChartInfo.m_pChart != NULL)
        {
            if(mInternalChartInfo.m_pChart != NULL)
            {
                cPenColor = mInternalChartInfo.m_pChart->medianColor(bIsMultiLayer);
                cPenWidth = mInternalChartInfo.m_pChart->medianLineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	* pLayerStyle = NULL;
                if ((mInternalChartInfo.m_nGroup >= 1 && mInternalChartInfo.m_nGroup <= m_pReportOptions->pLayersStyleList.count()))
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(mInternalChartInfo.m_nGroup-1);
                    cPenColor	= pLayerStyle->medianColor(bIsMultiLayer);
                    cPenWidth	= pLayerStyle->medianLineWidth();

                    if(!cPenWidth)
                        cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                }
                else
                {
                    cPenColor = Qt::blue;
                    cPenWidth = 1;
                }
            }

            if(cPenWidth)
            {
                addMarker(m_pXYChart->xAxis(), dMedianX, cPenColor.rgb() & 0xffffff, "X: Median", cPenWidth, Chart::TopRight, false);
                addMarker(m_pXYChart->yAxis(), dMedianY, cPenColor.rgb() & 0xffffff, "Y: Median", cPenWidth, Chart::TopRight, false);

                mInternalChartInfo.m_dMarkerMin = gex_min(mInternalChartInfo.m_dMarkerMin, dMedianX);
                mInternalChartInfo.m_dMarkerMax = gex_max(mInternalChartInfo.m_dMarkerMax, dMedianX);

                mInternalChartInfo.m_dMarkerMinY = gex_min(mInternalChartInfo.m_dMarkerMinY, dMedianY);
                mInternalChartInfo.m_dMarkerMaxY = gex_max(mInternalChartInfo.m_dMarkerMaxY, dMedianY);
            }
        }

        // If request to show the Sigma XY markers
        if(optionsMarkers.contains("2sigma") || mInternalChartInfo.m_pChart != NULL)
        {
            if(mInternalChartInfo.m_pChart != NULL)
            {
                cPenColor = mInternalChartInfo.m_pChart->sigma2Color(bIsMultiLayer);
                cPenWidth = mInternalChartInfo.m_pChart->sigma2LineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	* pLayerStyle = NULL;
                if ((mInternalChartInfo.m_nGroup >= 1 && mInternalChartInfo.m_nGroup <= m_pReportOptions->pLayersStyleList.count()))
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(mInternalChartInfo.m_nGroup-1);
                    cPenColor	= pLayerStyle->sigma2Color(bIsMultiLayer);
                    cPenWidth	= pLayerStyle->sigma2LineWidth();

                    if(!cPenWidth)
                        cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
            }

            if(cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pXYChart->dashLineColor(nTextColor, DotDashLine);

                // 2Sigma: Vertical (X axis)
                addMarker(m_pXYChart->xAxis(), dMeanX - dSigmaX, nLineColor, "-1s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->xAxis(), dMeanX + dSigmaX, nLineColor, "+1s", cPenWidth, Chart::TopRight, false)->setFontColor(nTextColor);

                // 2Sigma: Vertical (y axis)
                addMarker(m_pXYChart->yAxis(), dMeanY - dSigmaY, nLineColor, "-1s", cPenWidth, Chart::BottomLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->yAxis(), dMeanY + dSigmaY, nLineColor, "+1s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);

                mInternalChartInfo.m_dMarkerMin = gex_min(mInternalChartInfo.m_dMarkerMin, dMeanX - dSigmaX);
                mInternalChartInfo.m_dMarkerMax = gex_max(mInternalChartInfo.m_dMarkerMax, dMeanX + dSigmaX);

                mInternalChartInfo.m_dMarkerMinY = gex_min(mInternalChartInfo.m_dMarkerMinY, dMeanY - dSigmaY);
                mInternalChartInfo.m_dMarkerMaxY = gex_max(mInternalChartInfo.m_dMarkerMaxY, dMeanY + dSigmaY);
            }
        }

        // If request to show the  1.5 sigma XY markers
        if(optionsMarkers.contains("3sigma") || mInternalChartInfo.m_pChart != NULL)
        {
            if(mInternalChartInfo.m_pChart != NULL)
            {
                cPenColor = mInternalChartInfo.m_pChart->sigma3Color(bIsMultiLayer);
                cPenWidth = mInternalChartInfo.m_pChart->sigma3LineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	* pLayerStyle = NULL;
                if ((mInternalChartInfo.m_nGroup >= 1 && mInternalChartInfo.m_nGroup <= m_pReportOptions->pLayersStyleList.count()))
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(mInternalChartInfo.m_nGroup-1);
                    cPenColor	= pLayerStyle->sigma3Color(bIsMultiLayer);
                    cPenWidth	= pLayerStyle->sigma3LineWidth();

                    if(!cPenWidth)
                        cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
            }

            if(cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pXYChart->dashLineColor(nTextColor, DotDashLine);

                // 3Sigma: Vertical (X axis)
                addMarker(m_pXYChart->xAxis(), dMeanX - (1.5 * dSigmaX), nLineColor, "-1.5s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->xAxis(), dMeanX + (1.5 * dSigmaX), nLineColor, "+1.5s", cPenWidth, Chart::TopRight)->setFontColor(nTextColor);

                // 3Sigma: Vertical (y axis)
                addMarker(m_pXYChart->yAxis(), dMeanY - (1.5 * dSigmaY), nLineColor, "-1.5s", cPenWidth, Chart::BottomLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->yAxis(), dMeanY + (1.5 * dSigmaY), nLineColor, "+1.5s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);

                mInternalChartInfo.m_dMarkerMin = gex_min(mInternalChartInfo.m_dMarkerMin, dMeanX - (1.5 * dSigmaX));
                mInternalChartInfo.m_dMarkerMax = gex_max(mInternalChartInfo.m_dMarkerMax, dMeanX + (1.5 * dSigmaX));

                mInternalChartInfo.m_dMarkerMinY = gex_min(mInternalChartInfo.m_dMarkerMinY, dMeanY - (1.5 * dSigmaY));
                mInternalChartInfo.m_dMarkerMaxY = gex_max(mInternalChartInfo.m_dMarkerMaxY, dMeanY + (1.5 * dSigmaY));
            }
        }

        // If request to show the 3sigma XY markers
        if(optionsMarkers.contains("6sigma") || mInternalChartInfo.m_pChart != NULL)
        {
            if(mInternalChartInfo.m_pChart != NULL)
            {
                cPenColor = mInternalChartInfo.m_pChart->sigma6Color(bIsMultiLayer);
                cPenWidth = mInternalChartInfo.m_pChart->sigma6LineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	* pLayerStyle = NULL;
                if ((mInternalChartInfo.m_nGroup >= 1 && mInternalChartInfo.m_nGroup <= m_pReportOptions->pLayersStyleList.count()))
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(mInternalChartInfo.m_nGroup-1);
                    cPenColor	= pLayerStyle->sigma6Color(bIsMultiLayer);
                    cPenWidth	= pLayerStyle->sigma6LineWidth();

                    if(!cPenWidth)
                        cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
            }

            if(cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pXYChart->dashLineColor(nTextColor, DotDashLine);

                // 6Sigma: Vertical (X axis)
                addMarker(m_pXYChart->xAxis(), dMeanX - (3 * dSigmaX), nLineColor, "-3s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->xAxis(), dMeanX + (3 * dSigmaX), nLineColor, "+3s", cPenWidth, Chart::TopRight, false)->setFontColor(nTextColor);

                // 6Sigma: Vertical (y axis)
                addMarker(m_pXYChart->yAxis(), dMeanY - (3 * dSigmaY), nLineColor, "-3s", cPenWidth, Chart::BottomLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->yAxis(), dMeanY + (3 * dSigmaY), nLineColor, "+3s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);

                mInternalChartInfo.m_dMarkerMin = gex_min(mInternalChartInfo.m_dMarkerMin, dMeanX - (3 * dSigmaX));
                mInternalChartInfo.m_dMarkerMax = gex_max(mInternalChartInfo.m_dMarkerMax, dMeanX + (3 * dSigmaX));

                mInternalChartInfo.m_dMarkerMinY = gex_min(mInternalChartInfo.m_dMarkerMinY, dMeanY - (3 * dSigmaY));
                mInternalChartInfo.m_dMarkerMaxY = gex_max(mInternalChartInfo.m_dMarkerMaxY, dMeanY + (3 * dSigmaY));
            }
        }

        // If request to show the 6sigma XY markers
        if(optionsMarkers.contains("12sigma") || mInternalChartInfo.m_pChart != NULL)
        {
            if(mInternalChartInfo.m_pChart != NULL)
            {
                cPenColor = mInternalChartInfo.m_pChart->sigma12Color(bIsMultiLayer);
                cPenWidth = mInternalChartInfo.m_pChart->sigma12LineWidth();
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	* pLayerStyle = NULL;
                if ((mInternalChartInfo.m_nGroup >= 1 && mInternalChartInfo.m_nGroup <= m_pReportOptions->pLayersStyleList.count()))
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(mInternalChartInfo.m_nGroup-1);
                    cPenColor	= pLayerStyle->sigma12Color(bIsMultiLayer);
                    cPenWidth	= pLayerStyle->sigma12LineWidth();

                    if(!cPenWidth)
                        cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
            }

            if(cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pXYChart->dashLineColor(nTextColor, DotDashLine);

                // 12Sigma: Vertical (X axis)
                addMarker(m_pXYChart->xAxis(), dMeanX - (6 * dSigmaX), nLineColor, "-6s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->xAxis(), dMeanX + (6 * dSigmaX), nLineColor, "+6s", cPenWidth, Chart::TopRight, false)->setFontColor(nTextColor);

                // 12Sigma: Vertical (y axis)
                addMarker(m_pXYChart->yAxis(), dMeanY - (6 * dSigmaY), nLineColor, "-6s", cPenWidth, Chart::BottomLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->yAxis(), dMeanY + (6 * dSigmaY), nLineColor, "+6s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);

                mInternalChartInfo.m_dMarkerMin = gex_min(mInternalChartInfo.m_dMarkerMin, dMeanX - (6 * dSigmaX));
                mInternalChartInfo.m_dMarkerMax = gex_max(mInternalChartInfo.m_dMarkerMax, dMeanX + (6 * dSigmaX));

                mInternalChartInfo.m_dMarkerMinY = gex_min(mInternalChartInfo.m_dMarkerMinY, dMeanY - (6 * dSigmaY));
                mInternalChartInfo.m_dMarkerMaxY = gex_max(mInternalChartInfo.m_dMarkerMaxY, dMeanY + (6 * dSigmaY));
            }
        }

        return true;
    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void computeDataset()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexBoxWhiskerChart::computeDataset()
{
    // Get pointer to first group & first file (we always have them exist)
    CGexFileInGroup *	pFileX      = NULL;
    CGexFileInGroup *	pFileY      = NULL;
    CGexGroupOfFiles *	pGroupX     = NULL;
    CGexGroupOfFiles *	pGroupY     = NULL;
    CTest *             ptTestCellX	= NULL;
    CTest *             ptTestCellY	= NULL;

    int                 lTestNumberX;
    int                 lPinmapIndexX;
    int                 lTestNumberY;
    int                 lPinmapIndexY;

    // Delete previous datasets
    qDeleteAll(mLayerDatasets);
    mLayerDatasets.clear();

    // Reset all internal data
    mInternalChartInfo.reset();

    if (m_pChartOverlays)
    {
        CGexSingleChart	*	pChart = NULL;			// Handle to Parameter Layer info.

        for(int iLayerIndex=0; iLayerIndex < m_pChartOverlays->chartsList().count(); iLayerIndex++)
        {
            pChart = m_pChartOverlays->chartsList().at(iLayerIndex);

            // Seek to the relevant Group to plot.
            if ((pChart->iGroupX < 0) || (pChart->iGroupX >= gexReport->getGroupsList().size()))
                pGroupX = NULL;
            else
                pGroupX = gexReport->getGroupsList().at(pChart->iGroupX);

            if ((pChart->iGroupY < 0) || (pChart->iGroupY >= gexReport->getGroupsList().size()))
                pGroupY = NULL;
            else
                pGroupY = gexReport->getGroupsList().at(pChart->iGroupY);

            pFileX	= (pGroupX->pFilesList.isEmpty()) ? NULL : pGroupX->pFilesList.first();
            pFileY	= (pGroupY->pFilesList.isEmpty()) ? NULL : pGroupY->pFilesList.first();

            if (pFileX && pFileY)
            {
                // Get test# & pinmap#
                lTestNumberX	= pChart->iTestNumberX;
                lPinmapIndexX	= pChart->iPinMapX;

                // Get test# & pinmap#
                lTestNumberY	= pChart->iTestNumberY;
                lPinmapIndexY	= pChart->iPinMapY;

                if ((pFileX->FindTestCell(lTestNumberX, lPinmapIndexX, &ptTestCellX, false, false, pChart->strTestNameX) == 1) &&
                        (pFileY->FindTestCell(lTestNumberY, lPinmapIndexY, &ptTestCellY, false, false, pChart->strTestNameY) == 1))
                {
                    // If multiple charts (groups or overlays), scale based on 1st parameter in 1st group.
                    if (ptTestCellX->m_testResult.count() > 0 && mInternalChartInfo.m_bTestReferenceScaleFactor == false)
                    {
                        mInternalChartInfo.m_pTestReferenceScaleX        = ptTestCellX;
                        mInternalChartInfo.m_nTestReferenceScaleFactor   = ptTestCellX->res_scal;
                        mInternalChartInfo.m_bTestReferenceScaleFactor   = true;
                    }

                    if (ptTestCellY->m_testResult.count() > 0 && mInternalChartInfo.m_bTestReferenceScaleFactorY == false)
                    {
                        mInternalChartInfo.m_pTestReferenceScaleY        = ptTestCellY;
                        mInternalChartInfo.m_nTestReferenceScaleFactorY  = ptTestCellY->res_scal;
                        mInternalChartInfo.m_bTestReferenceScaleFactorY  = true;
                    }

                    CGexBoxPlotData * pBoxPlotDataset = new CGexBoxPlotData();

                    if (fillDataset(pBoxPlotDataset, ptTestCellX, ptTestCellY))
                        mLayerDatasets.insert(iLayerIndex, pBoxPlotDataset);
                }
            }
        }
    }
    else
    {
        for(int iLayerIndex=0; iLayerIndex < gexReport->getGroupsList().count(); iLayerIndex++)
        {
            pGroupX = gexReport->getGroupsList().at(iLayerIndex);
            pGroupY = gexReport->getGroupsList().at(iLayerIndex);

            pFileX	= (pGroupX->pFilesList.isEmpty()) ? NULL : pGroupX->pFilesList.first();
            pFileY	= (pGroupY->pFilesList.isEmpty()) ? NULL : pGroupY->pFilesList.first();

            if (pFileX && pFileY)
            {
                // Get test# & pinmap#
                lTestNumberX	= referenceTestX()->lTestNumber;
                lPinmapIndexX	= referenceTestX()->lPinmapIndex;

                // Get test# & pinmap#
                lTestNumberY	= referenceTestY()->lTestNumber;
                lPinmapIndexY	= referenceTestY()->lTestNumber;

                if ((pFileX->FindTestCell(lTestNumberX, lPinmapIndexX, &ptTestCellX, false, false, referenceTestX()->strTestName) == 1) &&
                        (pFileY->FindTestCell(lTestNumberY, lPinmapIndexY, &ptTestCellY, false, false, referenceTestY()->strTestName) == 1))
                {
                    // If multiple charts (groups or overlays), scale based on 1st parameter in 1st group.
                    if (ptTestCellX->m_testResult.count() > 0 && mInternalChartInfo.m_bTestReferenceScaleFactor == false)
                    {
                        mInternalChartInfo.m_pTestReferenceScaleX        = ptTestCellX;
                        mInternalChartInfo.m_nTestReferenceScaleFactor   = ptTestCellX->res_scal;
                        mInternalChartInfo.m_bTestReferenceScaleFactor   = true;
                    }

                    if (ptTestCellY->m_testResult.count() > 0 && mInternalChartInfo.m_bTestReferenceScaleFactorY == false)
                    {
                        mInternalChartInfo.m_pTestReferenceScaleY        = ptTestCellY;
                        mInternalChartInfo.m_nTestReferenceScaleFactorY  = ptTestCellY->res_scal;
                        mInternalChartInfo.m_bTestReferenceScaleFactorY  = true;
                    }

                    CGexBoxPlotData * pBoxPlotDataset = new CGexBoxPlotData();

                    if (fillDataset(pBoxPlotDataset, ptTestCellX, ptTestCellY))
                        mLayerDatasets.insert(iLayerIndex, pBoxPlotDataset);
                }
            }
        }
    }
}

bool GexBoxWhiskerChart::fillDataset(CGexBoxPlotData * pLayerDataset, CTest * ptTestCellX, CTest * ptTestCellY)
{
    QMap<double, QVector<double> >  lMapDataset;

    if (pLayerDataset == NULL)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Dataset for the layer is null");
        return false;
    }

    if (ptTestCellX == NULL || ptTestCellY == NULL)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Test cell is NULL");
        return false;
    }

    if (mInternalChartInfo.m_pTestReferenceScaleX == NULL || mInternalChartInfo.m_pTestReferenceScaleY == NULL)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "No reference test found to scale result");
        return false;
    }

    if (ptTestCellX->m_testResult.count() == 0 || ptTestCellY->m_testResult.count() == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Test cell has no executions");
        return false;
    }

    if (ptTestCellX->ldSamplesExecs == 0 || ptTestCellY->ldSamplesExecs == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Test cell has no valid sample exec.");
        return false;
    }

    double lXValue;
    double lYValue;
    double lCustomScaleXFactor = 1.0 / GS_POW(10.0, (ptTestCellX->res_scal - mInternalChartInfo.m_nTestReferenceScaleFactor));
    double lCustomScaleYFactor = 1.0 / GS_POW(10.0, (ptTestCellY->res_scal - mInternalChartInfo.m_nTestReferenceScaleFactorY));

    for (int index = 0; index < ptTestCellX->m_testResult.count(); ++index)
    {
        if (ptTestCellX->m_testResult.isValidResultAt(index) && ptTestCellY->m_testResult.isValidResultAt(index))
        {
            lXValue = ptTestCellX->m_testResult.resultAt(index);
            lYValue = ptTestCellY->m_testResult.resultAt(index);

            if (ptTestCellX->res_scal != mInternalChartInfo.m_pTestReferenceScaleX->res_scal)
                lXValue *= lCustomScaleXFactor;

            if (ptTestCellY->res_scal != mInternalChartInfo.m_pTestReferenceScaleY->res_scal)
                lYValue *= lCustomScaleYFactor;

            if (lMapDataset.contains(lXValue))
                lMapDataset[lXValue].append(lYValue);
            else
            {
                QVector<double> values;

                values.append(lYValue);
                lMapDataset.insert(lXValue, values);
            }
        }
    }

    if (lMapDataset.count() == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "No common valid samples between the 2 tests");
        return false;
    }

    pLayerDataset->init(lMapDataset.count());

    QMapIterator<double, QVector<double> >  iterator(lMapDataset);
    QVector<double>                         dataset;
    int lIndex = 0;

    while (iterator.hasNext())
    {
        iterator.next();
        dataset = iterator.value();

        qSort(dataset);

        pLayerDataset->addData(lIndex++, dataset.first(), algorithms::gexQ1Value(dataset),
                               algorithms::gexMedianValue(dataset), algorithms::gexQ3Value(dataset), dataset.last(), iterator.key());
     }

    return true;
}

void GexBoxWhiskerChart::scaleChart()
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
                dHighX		= referenceTestX()->ptChartOptions->highX();
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
            ( m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeBoxWhisker || m_pChartOverlays->getAppliedToChart() == -1)) || m_iXScaleType == 1)
        {
            // Log scale: then force it
            m_pXYChart->xAxis()->setLogScale(dLowX, dHighX);
            m_iXScaleType = 1;
        }
        else if((m_pChartOverlays && !m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX &&
               ( m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeBoxWhisker || m_pChartOverlays->getAppliedToChart() == -1)) || m_iXScaleType == 0)
        {
            m_pXYChart->xAxis()->setLinearScale(dLowX, dHighX);
            m_iXScaleType = 0;
        }
    }

    if (dLowY != C_INFINITE && dHighY != -C_INFINITE)
    {
        // Check if scale type is: Linear or Logarithmic
        if((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
            (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeBoxWhisker || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 1)
        {
            // Log scale: then force it
            m_pXYChart->yAxis()->setLogScale(dLowY, dHighY);
            m_iYScaleType = 1;
        }
        else if((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
               (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeBoxWhisker || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 0)
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

