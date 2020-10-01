/******************************************************************************!
 * \file gexhistogramchart.cpp
 ******************************************************************************/
#include "gexhistogramchart.h"
#include "report_options.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include <gqtl_log.h>
#include <gqtl_global.h>

extern CGexReport* gexReport;
extern double ScalingPower(int iPower);

// from X = -4.Sigma to X = +4.Sigma; Step = 0.25
static double lfNormalValues[] =
{
    0.000133830, 0.000352596, 0.000872683, 0.002029048, 0.004431848,
    0.009093563, 0.017528300, 0.031739652, 0.053990967, 0.086277319,
    0.129517596, 0.182649085, 0.241970725, 0.301137432, 0.352065327,
    0.386668117, 0.398942280, 0.386668117, 0.352065327, 0.301137432,
    0.241970725, 0.182649085, 0.129517596, 0.086277319, 0.053990967,
    0.031739652, 0.017528300, 0.009093563, 0.004431848, 0.002029048,
    0.000872683, 0.000352596, 0.000133830
};

/******************************************************************************!
 * \fn GexHistogramChart
 * \brief Constructor
 ******************************************************************************/
GexHistogramChart::
GexHistogramChart(bool bStandard,
                  int nSizeMode,
                  GexWizardChart *lWizardParent,
                  CGexChartOverlays* pChartOverlays  /*= NULL*/) :
    GexAbstractChart(chartTypeHistogram, nSizeMode, lWizardParent, pChartOverlays),
    m_bStandard(bStandard),
    m_eYAxis(YAxisUndefined)
{
    this->computeDataset();
}

/******************************************************************************!
 * \fn ~GexHistogramChart
 * \brief Destructor
 ******************************************************************************/
GexHistogramChart::~GexHistogramChart()
{
}

/******************************************************************************!
 * \fn computeDataset
 ******************************************************************************/
void GexHistogramChart::computeDataset()
{
    mIsStacked = false;
    if (! m_pReportOptions)
    {
        return;
    }

    if (m_pReportOptions->GetOption("adv_histogram", "chart_type").toString() == "stack")
    {
        if (m_eYAxis == YAxisUndefined)
        {
            m_eYAxis = YAxisHits;
            m_pReportOptions->SetOption("adv_histogram", "y_axis", "hits");
        }
        else if (m_pReportOptions->GetOption("adv_histogram", "y_axis").toString() == "hits")
        {
            m_eYAxis = YAxisHits;
        }
        else
        {
            m_eYAxis = YAxisPercentage;
        }
        mIsStacked = true;
    }
    else if (m_pReportOptions->GetOption("adv_histogram", "y_axis").toString() == "hits")
    {
        m_eYAxis = YAxisHits;
    }
    else
    {
        m_eYAxis = YAxisPercentage;
    }
}

/**
 * @brief GexHistogramChart::resetReportOptionToInteractiveOverlay
 * In interactive mode, the settings are not automaticaly set in the option
 * This method enable to
 */
void GexHistogramChart::resetReportOptionToInteractiveOverlay()
{
    if(m_pChartOverlays && m_pChartOverlays->chartsList()[0])
    {

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("mean"),
                                                (m_pChartOverlays->chartsList()[0]->meanLineWidth() > 0) );

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("limits"),
                                                (m_pChartOverlays->chartsList()[0]->limitsLineWidth()>0));

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("speclimits"),
                                                (m_pChartOverlays->chartsList()[0]->specLimitsLineWidth()>0));

        m_pReportOptions->bPlotLegend = m_pChartOverlays->mLayerName;

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("median"),
                                                (m_pChartOverlays->chartsList()[0]->medianLineWidth()>0));

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("minimum"),
                                                (m_pChartOverlays->chartsList()[0]->minLineWidth()>0));

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("maximum"),
                                                (m_pChartOverlays->chartsList()[0]->maxLineWidth()>0));

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("quartile_q1"),
                                                (m_pChartOverlays->chartsList()[0]->quartileQ1LineWidth()>0));

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("quartile_q3"),
                                                (m_pChartOverlays->chartsList()[0]->quartileQ3LineWidth()>0));

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("2sigma"),
                                                (m_pChartOverlays->chartsList()[0]->sigma2LineWidth()>0));

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("3sigma"),
                                                (m_pChartOverlays->chartsList()[0]->sigma3LineWidth()>0));

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("6sigma"),
                                                (m_pChartOverlays->chartsList()[0]->sigma6LineWidth()>0));

        m_pReportOptions->SetSpecificFlagOption(QString("adv_histogram"),
                                                QString("marker"),
                                                QString("12sigma"),
                                                (m_pChartOverlays->chartsList()[0]->sigma12LineWidth()>0));
    }
}

/******************************************************************************!
 * \fn buildOwnerChart
 ******************************************************************************/
void GexHistogramChart::buildOwnerChart()
{
    LegendBox* poLegendBox2 = 0;
    QString strString;
    CTest* ptTestCell = NULL;
    double lfStep;
    char szTestName[2 * GEX_MAX_STRING];
    char szString[300];
    int i;
    int iCurvePenSize;  // curve pen size 1 or 2 pixels.
    //FIXME: not used ?
    // long lLowOutliers=0;  // Keeps track of total of outliers < LowLimit
    // long lHighOutliers=0; // Keeps track of total of outliers > HighLimit
    // Total samples in each histogram (used to compute % of classes)
    long lTotalSamples;
    double lfMean = -C_INFINITE;
    double lfMin = -C_INFINITE;
    double lfMax = -C_INFINITE;
    double lfSigma = -C_INFINITE;
    double lfExtraLow = 0;
    double lfExtraHigh = 0;
    double lfMedian = -C_INFINITE;
    // Will include lowest Ploted data result over all groups
    double lfChartBottom = C_INFINITE;
    double lfMergedChartBottom = C_INFINITE;
    double lfMergedChartTop = -C_INFINITE;
    double lfChartTop = -C_INFINITE;
    double lfHistoClassesWidth;  // MaxSample-MinSample
    int iCell;  // Used to compute histogram classes
    double lfQuartileQ1 = -C_INFINITE;
    double lfQuartileQ3 = -C_INFINITE;
    // Either 'TEST_HISTOSIZE' or 'TEST_ADVHISTOSIZE' or custom bar count
    int iHistoClasses =
        m_pReportOptions->GetOption("adv_histogram", "total_bars").toInt();
    QString scaling =
        m_pReportOptions->GetOption("dataprocessing", "scaling").toString();
    QString strOptionStorageDevice =
        (m_pReportOptions->GetOption("adv_histogram", "chart_type")).toString();

    GexInternalChartInfo internalChartInfo;

    // Pointer to label strings, may vary depending of the chart size
    QString strLabelMean("Mean");
    QString strLabelMedian("Median");
    double lfValue;
    double lfSamplesMin, lfSamplesMax;
    double dDiff = 0;
    double fStart, fDataStart = C_INFINITE, fDataEnd = -C_INFINITE;
    double fPercent = 0;
    double fPercentTopBar = 0;
    double fCumul;
    double fStep;
    int lTestNumber;
    int lPinmapIndex;
    int iGroup = 1;  // Counter used to know on which group we are (#1 or other)
    bool bTestReferenceScaleFactor = false;
    // Scale factor for 1st test (Chart scales based on it)
    int iTestReferenceScaleFactor = 0;
    // Correction custom Scale factor to apply
    // on all but 1st chart so all scales match
    int iCustomScaleFactor = 0;
    double fCustomScaleFactor = 0.0;  // Scale ratio to apply multiplyer)
    QString strLabel;
    QString strTitle;
    QColor cPenColor;  // Color used to paint the layer
    int cPenWidth;
    BarLayer* pBarLayer = NULL;

    // Buffers to hold the data to chart
    double* x;
    double* y;
    double* lSumPerBar;
    x = new double[iHistoClasses + 2];
    y = new double[iHistoClasses + 2];
    lSumPerBar = new double[iHistoClasses + 2];
    if (x == NULL || y == NULL || lSumPerBar == NULL)
    {
        return;  // Memory allocation problem
    }
    for (i = 1; i <= iHistoClasses; ++i)
    {
        lSumPerBar[i] = 0.0;
    }
    // Get pointer to first group & first file (we always have them exist)
    QList<CGexGroupOfFiles*>::iterator itGroupList;
    CGexGroupOfFiles* pGroup = NULL;
    CGexFileInGroup* pFile = NULL;
    int iChartCounter;

    // Index used to keep track of Chart layers being charted
    double lfChartLowX = C_INFINITE;
    //FIXME: not used ?
    // double lfChartLowXBeforeCompute = C_INFINITE;
    // double lfChartHighXBeforeCompute = C_INFINITE;

    double lfChartLowY = C_INFINITE;
    double lfChartHighX = -C_INFINITE;
    double lfChartHighY = -C_INFINITE;
    double lfViewportLowX = C_INFINITE;
    double lfViewportHighX = -C_INFINITE;

    QString lParameterName;
    CGexSingleChart* pChart = NULL;  // Handle to Parameter Layer info
    int iLineWidth = 2;
    int iLayerIndex;
    bool bShowSpline;
    bool bShowBellCurve;
    bool bShowHisto;
    bool bCumulativeHisto;
    //FIXME: not used ?
    // bool bSpots;
    bool bVisible = true;
    Chart::SymbolType symbolLayer;
    int nLineColor;
    int nFillColor;
    int nOpacity = 0x00000000;
    bool bIsMultiLayer = isMultiLayers();
    bool lShowTitle = false;

    m_pXYChart->xAxis()->addMark(0, -1, "0")->
    setMarkColor(Chart::Transparent, 0x000000, 0x000000);


    // -- if interactive settings, use those one rather than those in the ReportOption
    if(m_pChartOverlays)
        resetReportOptionToInteractiveOverlay();


    // OPTIONS
    QString strAdvHistogramMarkerOptions =
        (m_pReportOptions->GetOption(QString("adv_histogram"),
                                     QString("marker"))).toString();
    QStringList qslAdvHistogramMarkerOptionsList =
        strAdvHistogramMarkerOptions.split(QString("|"));

    QString strAdvHistogramFieldOptions =
        (m_pReportOptions->GetOption(QString("adv_histogram"),
                                     QString("field"))).toString();
    QStringList qslAdvHistogramFieldOptionList =
        strAdvHistogramFieldOptions.split(QString("|"));

    if ((m_pChartOverlays && m_pChartOverlays->mTitle == true) ||
        qslAdvHistogramMarkerOptionsList.contains(QString("test_name")))
    {
        lShowTitle = true;
    }

    // If not a parametric / multiparametric (eg: functional) test, ignore
    if (referenceTestX()->bTestType == 'F')
    {
        goto end_of_histo;
    }

    // Cumulative Histogram or Not ?
    if ((viewportMode() == viewportCumulLimits) ||
        (viewportMode() == viewportCumulData))
    {
        bCumulativeHisto = true;
    }
    else
    {
        bCumulativeHisto = false;
    }

    // First step: detect the Lowest sample
    // and Highest sample that will be plotted
    iLayerIndex = 0;
    lfSamplesMin = C_INFINITE;
    lfSamplesMax = -C_INFINITE;

    itGroupList = gexReport->getGroupsList().begin();
    pGroup =
        (itGroupList != gexReport->getGroupsList().end()) ?
        (*itGroupList) : NULL;
    iChartCounter = 0;
    if (m_pChartOverlays == NULL)
    {
        ++itGroupList;
        pGroup =
            (itGroupList != gexReport->getGroupsList().end()) ?
            (*itGroupList) : NULL;
        iChartCounter++;
    }

    itGroupList = gexReport->getGroupsList().begin();
    pGroup =
        (itGroupList != gexReport->getGroupsList().end()) ?
        (*itGroupList) : NULL;

    // Find the Min-Max samples windo for all groups/layer to plot
    while (pGroup != NULL)
    {
        if (m_pChartOverlays)
        {
            // Get handle to the Layer to plot
            if (iLayerIndex >= 0 && iLayerIndex <
                m_pChartOverlays->chartsList().count())
            {
                pChart = m_pChartOverlays->chartsList().at(iLayerIndex);
            }
            else
            {
                break;
            }

            // Seek to the relevant Group to plot
            if ((pChart->iGroupX < 0) ||
                (pChart->iGroupX >= gexReport->getGroupsList().size()))
            {
                pGroup = NULL;
            }
            else
            {
                pGroup = gexReport->getGroupsList().at(pChart->iGroupX);
            }

            // We have multiple charts (layers) to plot
            lTestNumber     = pChart->iTestNumberX;
            lPinmapIndex    = pChart->iPinMapX;
            lParameterName  = pChart->strTestNameX;

            // Next layerID to chart
            iLayerIndex++;
        }
        else
        {
            // Find test cell: RESET list to ensure
            // we scan list of the right group
            // Check if pinmap index
            lTestNumber = referenceTestX()->lTestNumber;

            if (referenceTestX()->lPinmapIndex >= 0)
            {
                lPinmapIndex = referenceTestX()->lPinmapIndex;
            }
            else
            {
                lPinmapIndex = GEX_PTEST;
            }

            lParameterName  = referenceTestX()->strTestName;
        }

        // Hanle to first file in group
        pFile =
            (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false,
                                false, lParameterName) != 1)
        {
            goto NextGroup_FindMinMax;
        }

        if (ptTestCell->m_bUseCustomBarNumber)
        {
            iHistoClasses = ptTestCell->m_iHistoClasses;
            delete[] x;
            x = new double[iHistoClasses + 2];
            delete[] y;
            y = new double[iHistoClasses + 2];
            delete[] lSumPerBar;
            lSumPerBar = new double[iHistoClasses + 2];
            if (x == NULL || y == NULL || lSumPerBar == NULL)
            {
                return;  // Memory allocation problem
            }
            for (i = 1; i <= iHistoClasses; ++i)
            {
                lSumPerBar[i] = 0.0;
            }
        }

        // If multiple charts (groups or overlays),
        // scale based on 1st parameter in 1st group
        if (ptTestCell->m_testResult.count() > 0 && bTestReferenceScaleFactor ==
            false)
        {
            // Multi-layer or Multi group :
            // Chart scales will be of 1st Parameter in 1st layer
            iTestReferenceScaleFactor = ptTestCell->res_scal;
            bTestReferenceScaleFactor = true;
        }

        // For tests such as binning type = '-'),
        // do not rescale, align to first layer scale
        if (ptTestCell->bTestType == '-')
        {
            fCustomScaleFactor = 1;
        }
        else
        {
            iCustomScaleFactor =
                ptTestCell->res_scal - iTestReferenceScaleFactor;
            fCustomScaleFactor = 1 / GS_POW(10.0, iCustomScaleFactor);
        }

        // Compute some variables
        if (m_bStandard == true)
        {
            // Normalize histogram Min & Max values
            double lfHistoMin = ptTestCell->GetCurrentLimitItem()->lfHistogramMin;
            double lfHistoMax = ptTestCell->GetCurrentLimitItem()->lfHistogramMax;
            pFile->FormatTestResultNoUnits(&lfHistoMin, ptTestCell->res_scal);
            pFile->FormatTestResultNoUnits(&lfHistoMax, ptTestCell->res_scal);

            // Using standard Histogam array
            lfSamplesMin = fCustomScaleFactor * lfHistoMin;
            lfSamplesMax = fCustomScaleFactor * lfHistoMax;
            fDataStart = gex_min(fDataStart, lfSamplesMin);
            fDataEnd = gex_max(fDataEnd, lfSamplesMax);
        }
        else
        {
            // Interactive mode:
            // if no samples (or layer hidden) for this test, ignore it
            if ((pChart &&
                 pChart->bVisible == false) ||
                (ptTestCell->m_testResult.count() == 0))
            {
                goto NextGroup_FindMinMax;
            }

            // Find data samples min and max values
            double lfTmpSamplesMin = C_INFINITE;
            double lfTmpSamplesMax = -C_INFINITE;

            for (i = 0; i < ptTestCell->m_testResult.count(); i++)
            {
                if (ptTestCell->m_testResult.isValidResultAt(i))
                {
                    if (ptTestCell->m_testResult.isMultiResultAt(i))
                    {
                        for (int nCount = 0;
                             nCount < ptTestCell->m_testResult.at(i)->count();
                             ++nCount)
                        {
                            if (ptTestCell->m_testResult.at(i)->
                                isValidResultAt(nCount))
                            {
                                lfTmpSamplesMin =
                                    gex_min(lfTmpSamplesMin,
                                            ptTestCell->m_testResult.
                                            at(i)->multiResultAt(nCount));
                                lfTmpSamplesMax =
                                    gex_max(lfTmpSamplesMax,
                                            ptTestCell->m_testResult.
                                            at(i)->multiResultAt(nCount));
                            }
                        }
                    }
                    else
                    {
                        lfTmpSamplesMin =
                            gex_min(lfTmpSamplesMin,
                                    ptTestCell->m_testResult.resultAt(i));
                        lfTmpSamplesMax =
                            gex_max(lfTmpSamplesMax,
                                    ptTestCell->m_testResult.resultAt(i));
                    }
                }
            }

            lfTmpSamplesMin *= fCustomScaleFactor;
            lfTmpSamplesMax *= fCustomScaleFactor;
            lfSamplesMin = gex_min(lfSamplesMin, lfTmpSamplesMin);
            lfSamplesMax = gex_max(lfSamplesMax, lfTmpSamplesMax);
            fDataStart = gex_min(fDataStart, lfSamplesMin);
            fDataEnd = gex_max(fDataEnd, lfSamplesMax);
        }

        // Scale limits so we may need to compare test data to them,
        // only needed on 1st group (reference)
        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
        {
            // Draw LL limit markers
            // (done only once: when charting Plot for group#1)
            double lfLowL = ptTestCell->GetCurrentLimitItem()->lfLowLimit;  // Low limit exists
            // If we have to keep values in normalized format, do not rescale
            if (scaling != "normalized")
            {
                // convert LowLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfLowL, ptTestCell->llm_scal);
                lfLowL *= ScalingPower(ptTestCell->llm_scal);  // normalized
                lfLowL /= ScalingPower(ptTestCell->res_scal);  // normalized
            }
            if (iCustomScaleFactor)
            {
                lfLowL *= fCustomScaleFactor;
            }

            internalChartInfo.m_dLowLimit =
                gex_min(internalChartInfo.m_dLowLimit, lfLowL);


            // LowLimit Marker
            if (qslAdvHistogramMarkerOptionsList.contains(QString("limits")))
            {
                if (pChart != NULL)
                {
                    cPenColor = pChart->limitsColor();
                    cPenWidth =
                        (pChart->bVisible) ? pChart->limitsLineWidth() : 0;
                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart* pLayerStyle = NULL;
                    if ((iGroup - 1) >= 0 && (iGroup - 1) <
                        m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle =
                            m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                        cPenColor = pLayerStyle->limitsColor();
                        cPenWidth = pLayerStyle->limitsLineWidth();

                        if (! cPenWidth)
                        {
                            // Just in case line width not defined under
                            // interactive chart, then ensure we still
                            // show line in image
                            cPenWidth = 1;
                        }
                    }
                    else
                    {
                        cPenColor = CGexReport::GetChartingColor(iGroup);
                        cPenWidth = 1;
                    }
                }
                // Requested to show limits (and layer visible)
                if (cPenWidth)
                {
                    int nPenColor = cPenColor.rgb() & 0xffffff;

                    //if (pChart != NULL && pChart->getTextRotation())
                    {
                        Mark* lMark =
                            addMarker(m_pXYChart->xAxis(), lfLowL, nPenColor,
                                      "LL", cPenWidth,
                                      Chart::TopRight);
                        //lMark->setFontAngle(pChart->getTextRotation());
                        //lMark->setPos(0, -lMark->getWidth());
                    }
                    /*else
                    {
                        addMarker(m_pXYChart->xAxis(), lfLowL, nPenColor,
                                  m_strLowLimit, cPenWidth, Chart::TopRight);
                    }*/
                }
            }
        }

        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
        {
            // Draw HL limit markers
            // (done only once: when charting Plot for group#1)
            double lfHighL = ptTestCell->GetCurrentLimitItem()->lfHighLimit;  // High limit exists
            // If we have to keep values in normalized format, do not rescale
            if (scaling != "normalized")
            {
                // convert HighLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfHighL, ptTestCell->hlm_scal);
                lfHighL *= ScalingPower(ptTestCell->hlm_scal);  // normalized
                lfHighL /= ScalingPower(ptTestCell->res_scal);  // normalized
            }

            if (iCustomScaleFactor)
            {
                lfHighL *= fCustomScaleFactor;
            }

            internalChartInfo.m_dHighLimit =
                gex_max(internalChartInfo.m_dHighLimit, lfHighL);

            // High limit Marker
            if (qslAdvHistogramMarkerOptionsList.contains(QString("limits")))
            {
                if (pChart != NULL)
                {
                    cPenColor = pChart->limitsColor();
                    cPenWidth =
                        (pChart->bVisible) ? pChart->limitsLineWidth() : 0;
                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart* pLayerStyle = NULL;
                    if ((iGroup - 1) >= 0 && (iGroup - 1) <
                        m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle =
                            m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                        cPenColor = pLayerStyle->limitsColor();
                        cPenWidth = pLayerStyle->limitsLineWidth();

                        if (! cPenWidth)
                        {
                            // Just in case line width not defined under
                            // interactive chart, then ensure we still
                            // show line in image
                            cPenWidth = 1;
                        }
                    }
                    else
                    {
                        cPenColor = CGexReport::GetChartingColor(iGroup);
                        cPenWidth = 1;
                    }
                }

                // Requested to show limits (and layer visible)
                if (cPenWidth)
                {
                    int nPenColor = cPenColor.rgb() & 0xffffff;

                   // if (pChart != NULL && pChart->getTextRotation())
                    {
                        Mark* lMark =
                            addMarker(m_pXYChart->xAxis(), lfHighL, nPenColor,
                                      "HL", cPenWidth,
                                      Chart::TopRight);
                        //lMark->setFontAngle(pChart->getTextRotation());
                        //lMark->setPos(0, -lMark->getWidth());
                    }
                   /* else
                    {
                        addMarker(m_pXYChart->xAxis(), lfHighL, nPenColor,
                                  m_strHighLimit, cPenWidth, Chart::TopLeft);
                    }*/
                }
            }
        }

        // Draw multi-limit markers.
        addMultiLimitMarkers(pChart, ptTestCell, internalChartInfo);

        // Scale limits so we may need to compare test data to them...
        // only needed on 1st group (reference)
        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
        {
            // Draw LL limit markers
            // (done only once: when charting Plot for group#1)
            double lfLowSpecL = ptTestCell->lfLowSpecLimit;  // Low Spec limit exists
            // If we have to keep values in normalized format, do not rescale
            if (scaling != "normalized")
            {
                // convert LowLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfLowSpecL,
                                               ptTestCell->llm_scal);
                lfLowSpecL *= ScalingPower(ptTestCell->llm_scal);  // normalized
                lfLowSpecL /= ScalingPower(ptTestCell->res_scal);  // normalized
            }

            if (iCustomScaleFactor)
            {
                lfLowSpecL *= fCustomScaleFactor;
            }

            internalChartInfo.m_dLowSpecLimit =
                gex_min(internalChartInfo.m_dLowSpecLimit, lfLowSpecL);

            // LowLimit Marker
            if (qslAdvHistogramMarkerOptionsList.
                contains(QString("speclimits")))
            {
                if (pChart != NULL)
                {
                    cPenColor = pChart->limitsColor();
                    cPenWidth =
                        (pChart->bVisible) ? pChart->limitsLineWidth() : 0;
                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart* pLayerStyle = NULL;
                    if ((iGroup - 1) >= 0 && (iGroup - 1) <
                        m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle =
                            m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                        cPenColor = pLayerStyle->limitsColor();
                        cPenWidth = pLayerStyle->limitsLineWidth();

                        if (! cPenWidth)
                        {
                            // Just in case line width not defined under
                            // interactive chart, then ensure we still
                            // show line in image
                            cPenWidth = 1;
                        }
                    }
                    else
                    {
                        cPenColor = CGexReport::GetChartingColor(iGroup);
                        cPenWidth = 1;
                    }
                }
                // Requested to show limits (and layer visible)
                if (cPenWidth)
                {
                    int nPenColor = cPenColor.rgb() & 0xffffff;

                    addMarker(m_pXYChart->xAxis(), lfLowSpecL, nPenColor,
                              m_strLowSpecLimit, cPenWidth, Chart::TopRight);
                }
            }
        }

        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
        {
            // Draw HL limit markers
            // (done only once: when charting Plot for group#1)
            double lfHighSpecL = ptTestCell->lfHighSpecLimit;  // High limit exists

            // If we have to keep values in normalized format, do not rescale
            if (scaling != "normalized")
            {
                // convert HighLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfHighSpecL,
                                               ptTestCell->hlm_scal);
                lfHighSpecL *=
                    ScalingPower(ptTestCell->hlm_scal);  // normalized
                lfHighSpecL /=
                    ScalingPower(ptTestCell->res_scal);  // normalized
            }

            if (iCustomScaleFactor)
            {
                lfHighSpecL *= fCustomScaleFactor;
            }

            internalChartInfo.m_dHighSpecLimit = gex_max(
                    internalChartInfo.m_dHighSpecLimit,
                    lfHighSpecL);

            // High limit Marker
            if (qslAdvHistogramMarkerOptionsList.
                contains(QString("speclimits")))
            {
                if (pChart != NULL)
                {
                    cPenColor = pChart->limitsColor();
                    cPenWidth =
                        (pChart->bVisible) ? pChart->limitsLineWidth() : 0;
                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart* pLayerStyle = NULL;
                    if ((iGroup - 1) >= 0 && (iGroup - 1) <
                        m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle =
                            m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                        cPenColor = pLayerStyle->limitsColor();
                        cPenWidth = pLayerStyle->limitsLineWidth();

                        if (! cPenWidth)
                        {
                            // Just in case line width not defined under
                            // interactive chart, then ensure we still
                            // show line in image
                            cPenWidth = 1;
                        }
                    }
                    else
                    {
                        cPenColor = CGexReport::GetChartingColor(iGroup);
                        cPenWidth = 1;
                    }
                }

                // Requested to show limits (and layer visible)
                if (cPenWidth)
                {
                    int nPenColor = cPenColor.rgb() & 0xffffff;

                    addMarker(m_pXYChart->xAxis(), lfHighSpecL, nPenColor,
                              m_strHighSpecLimit, cPenWidth, Chart::TopLeft);
                }
            }
        }

NextGroup_FindMinMax:

        if (m_pChartOverlays == NULL)
        {
            ++itGroupList;
            pGroup =
                (itGroupList != gexReport->getGroupsList().end()) ?
                (*itGroupList) : NULL;

            // Keep track of group index processed
            iGroup++;
        }
    }  // Read histogram of a given test in all groups so to stack all charts

    // Plot all groups, unless in interactive mode
    // (then only plot the layers defined in the layer list)
    iLayerIndex = 0;
    iGroup = 1;
    itGroupList = gexReport->getGroupsList().begin();
    pGroup =
        (itGroupList != gexReport->getGroupsList().end()) ?
        (*itGroupList) : NULL;

    m_pXYChart->xAxis()->setIndent(false);

    // Increase the top margin when 3D mode
    if (strOptionStorageDevice == "3d_bars")
    {
       // if (pChart != NULL && pChart->getTextRotation())
        {
            setTopMargin(70);
        }
       /* else
        {
            setTopMargin(35);
        }*/
    }
    else
    {
        //if (pChart != NULL && pChart->getTextRotation())
        {
            setTopMargin(50);
        }
        /*else
        {
            setTopMargin(25);
        }*/
    }

    dDiff = internalChartInfo.m_dHighLimit - OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0)
    {
        internalChartInfo.m_dHighLimit = lfSamplesMax;
    }
    dDiff = internalChartInfo.m_dLowLimit + OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0)
    {
        internalChartInfo.m_dLowLimit = lfSamplesMin;
    }

    if (lfSamplesMax == lfSamplesMin)
    {
        switch (viewportMode())
        {
        case viewportOverLimits:
        case viewportCumulLimits:
            if (internalChartInfo.m_dLowLimit != C_INFINITE)
            {
                lfSamplesMin = internalChartInfo.m_dLowLimit;
            }
            if (internalChartInfo.m_dHighLimit != -C_INFINITE)
            {
                lfSamplesMax = internalChartInfo.m_dHighLimit;
            }
            break;

        case viewportOverData:
        case viewportCumulData:
            if (lfSamplesMin > 0)
            {
                lfSamplesMin *= 0.95;
            }
            else if (lfSamplesMin < 0)
            {
                lfSamplesMin *= 1.05;
            }
            else
            {
                lfSamplesMin = -1;
            }

            if (lfSamplesMax > 0)
            {
                lfSamplesMax *= 1.05;
            }
            else if (lfSamplesMax < 0)
            {
                lfSamplesMax *= 0.95;
            }
            else
            {
                lfSamplesMax = 1;
            }
            break;

        case viewportAdaptive:
            if (internalChartInfo.m_dLowLimit != C_INFINITE)
            {
                lfSamplesMin =
                    qMin(internalChartInfo.m_dLowLimit, lfSamplesMin);
            }
            if (internalChartInfo.m_dHighLimit != -C_INFINITE)
            {
                lfSamplesMax =
                    qMax(internalChartInfo.m_dHighLimit, lfSamplesMax);
            }

            break;
        }
    }

    fDataStart = lfSamplesMin;
    fDataEnd = lfSamplesMax;

    // Compute width of the 'iHistoClasses' classes to create
    lfHistoClassesWidth = lfSamplesMax - lfSamplesMin;

    while (pGroup != NULL)
    {
        // Copy the data with 2 extra plot:
        // head and tail cells added to force plot strat and end at 0
        if (m_pChartOverlays)
        {
            // Get handle to the Layer to plot
            if (iLayerIndex >= 0 &&
                iLayerIndex < m_pChartOverlays->chartsList().count())
            {
                pChart = m_pChartOverlays->chartsList().at(iLayerIndex);
            }
            else
            {
                break;
            }

            // Seek to the relevant Group to plot
            if ((pChart->iGroupX < 0) ||
                (pChart->iGroupX >= gexReport->getGroupsList().size()))
            {
                pGroup = NULL;
            }
            else
            {
                pGroup = gexReport->getGroupsList().at(pChart->iGroupX);
            }
            pFile =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                (pGroup->pFilesList.first());

            // We have multiple charts (layers) to plot
            lTestNumber     = pChart->iTestNumberX;
            lPinmapIndex    = pChart->iPinMapX;
            lParameterName  = pChart->strTestNameX;

            // Get ploting details
            iLineWidth = pChart->iLineWidth;

            // Next layerID to chart
            iLayerIndex++;
        }
        else
        {
            // Find test cell: RESET list to ensure
            // we scan list of the right group
            // Check if pinmap index
            pFile =
                (pGroup->pFilesList.isEmpty()) ? NULL :
                (pGroup->pFilesList.first());
            lTestNumber     = referenceTestX()->lTestNumber;
            lParameterName  = referenceTestX()->strTestName;

            if (referenceTestX()->lPinmapIndex >= 0)
            {
                lPinmapIndex = referenceTestX()->lPinmapIndex;
            }
            else
            {
                lPinmapIndex = GEX_PTEST;
            }
        }

        // No test cell found or no sample results, then go to the next group
        if (pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false,
                                false, lParameterName) != 1)
        {
            goto NextGroup;
        }

        // We have a chart title unless the chart size in
        // only 200 pixel wide (SMALL chart mode)
        if (m_nSizeMode != GEX_CHARTSIZE_SMALL &&
            (lShowTitle) && (iGroup == 1) && (m_pChartOverlays == NULL || iLayerIndex == 1))
        {
            // Test name required + chart big enough
            if (bIsMultiLayer && (isSingleTestAllLayers() == false))
            {
                // In multi-layer mode and with multiple layers shown,
                // title is generic, unless all layers refer to same test#
                if (mIsStacked)
                {
                    strTitle = "Stack Tests/Parameters";
                }
                else
                {
                    strTitle = "Overlay Tests/Parameters";
                }
            }
            else
            {
                gexReport->BuildTestNameString(pFile, ptTestCell, szTestName);
                QString strNormalizedName =
                    gexReport->buildDisplayName(szTestName, false);
                strTitle = QString("Test %1: %2").
                    arg(ptTestCell->szTestLabel).arg(strNormalizedName);

                if (bTestReferenceScaleFactor == false)
                {
                    strTitle += "<*color=FF0000*> - No data samples<*/color*>";
                }
            }
        }
        else
        {
            strTitle.clear();  // No title!
        }
        // Check if we have a custom title to overwrite default one
        if (m_pChartOverlays)
        {
            // Multilayers, check if we have assigned it a title
            if (m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bChartTitle)
            {
                // Custom title
                strTitle = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strChartTitle;
                strTitle = gexReport->buildDisplayName(strTitle, false);
            }
        }

        // Non-custom title
        m_pXYChart->addTitle(strTitle.toLatin1().constData());

        if (ptTestCell->m_testResult.count() == 0)
        {
            goto NextGroup;
        }

        // For tests such as binning type = '-'),
        // do not rescale, align to first layer scale
        if (ptTestCell->bTestType == '-')
        {
            fCustomScaleFactor = 1;
        }
        else
        {
            iCustomScaleFactor =
                ptTestCell->res_scal - iTestReferenceScaleFactor;
            fCustomScaleFactor = 1 / GS_POW(10.0, iCustomScaleFactor);
        }

        // Interactive mode:
        // if no samples for this test or hidden layeer, ignore it
        if (m_bStandard == false)
        {
            if ((pChart && pChart->bVisible == false))
            {
                goto NextGroup;
            }
        }

        // Reset histogram array
        for (i = 0; i < iHistoClasses + 2; i++)
        {
            x[i] = 0;
        }

        // List all test results so to build the histogram
        if (m_bStandard == true)
        {
            // Standard histogram: data already in histogram buffer
            lTotalSamples = ptTestCell->ldSamplesValidExecs;
            //FIXME: not used ?
            // lLowOutliers = -1;
            // lHighOutliers = -1;
            // Copy std. histogram buffer into Plot buffer
            for (i = 1; i <= iHistoClasses; i++)
            {
                x[i] = ptTestCell->lHistogram[i - 1];
            }
        }
        else
        {
            // Advanced histogram: samples collected in buffer
            lTotalSamples = 0;
            for (i = 0; i < ptTestCell->m_testResult.count(); i++)
            {
                // Include all valid samples
                if (ptTestCell->m_testResult.isValidResultAt(i))
                {
                    if (ptTestCell->m_testResult.isMultiResultAt(i))
                    {
                        for (int nCount = 0;
                             nCount < ptTestCell->m_testResult.at(i)->count();
                             ++nCount)
                        {
                            if (ptTestCell->m_testResult.at(i)->
                                isValidResultAt(nCount))
                            {
                                lfValue = ptTestCell->
                                    m_testResult.at(i)->multiResultAt(nCount);

                                if (iCustomScaleFactor)
                                {
                                    lfValue *= fCustomScaleFactor;
                                }

                                if (lfValue >= lfSamplesMin &&
                                    lfValue <= lfSamplesMax)
                                {
                                    iCell = (int) (1.0 +
                                                   ((double) iHistoClasses *
                                                    (lfValue - lfSamplesMin) /
                                                    (lfHistoClassesWidth)));
                                    // In case data is just on the edge,
                                    // ensure we do not have an overflow
                                    if (iCell >= iHistoClasses)
                                    {
                                        iCell = iHistoClasses;
                                    }
                                    if (iCell < 0)
                                    {
                                        iCell = 0;
                                    }

                                    x[iCell]++;
                                    lTotalSamples++;
                                }
                            }
                        }
                    }
                    else
                    {
                        lfValue = ptTestCell->m_testResult.resultAt(i);

                        if (iCustomScaleFactor)
                        {
                            lfValue *= fCustomScaleFactor;
                        }

                        if (lfValue >= lfSamplesMin && lfValue <= lfSamplesMax)
                        {
                            iCell = (int) (1.0 +
                                           ((double) iHistoClasses *
                                            (lfValue - lfSamplesMin) /
                                            (lfHistoClassesWidth)));
                            // In case data is just on the edge,
                            // ensure we do not have an overflow
                            if (iCell >= iHistoClasses)
                            {
                                iCell = iHistoClasses;
                            }
                            if (iCell < 0)
                            {
                                iCell = 0;
                            }

                            x[iCell]++;
                            lTotalSamples++;
                        }
                    }
                }
            }
        }

        // Load Chart array with histogram values,
        // scale so it matches the units-scaling info
        fStart = lfSamplesMin;
        fCumul = 0;
        fStep = (lfHistoClassesWidth) / (iHistoClasses);
        // Starting point must be at Y = 0
        y[0] = GEX_C_DOUBLE_NAN;

        // Added first step
        x[0] = fStart - (fStep / 2);
        // Adjust starting value of the 1st bar
        // to be correctly charted by ChartDirector
        fStart += (fStep / 2);

        fPercentTopBar = 0;
        for (i = 1; i <= iHistoClasses; i++)
        {
            // Compute Column size
            if (bCumulativeHisto)
            {
                if (m_eYAxis == YAxisHits)
                // m_pReportOptions->bHistoFrequencyCountScale)
                {
                    // Y scale: Frequency count
                    fCumul += x[i];
                }
                else
                {
                    // Y scale: Percentage
                    fCumul += (100.0 * x[i]) / lTotalSamples;
                }
                if (fCumul == 0.0)
                {
                    y[i] = GEX_C_DOUBLE_NAN;
                }
                else
                {
                    y[i] = fCumul;
                }
            }
            else
            {
                double dYValue;

                if (m_eYAxis == YAxisHits)
                // m_pReportOptions->bHistoFrequencyCountScale)
                {
                    // Y scale: Frequency count
                    dYValue = x[i];
                }
                else
                {
                    // Y scale: Percentage
                    dYValue = (100.0 * x[i]) / (double) lTotalSamples;
                }
                if (dYValue == 0.0)
                {
                    y[i] = GEX_C_DOUBLE_NAN;
                }
                else
                {
                    y[i] = dYValue;
                }
            }

            // Holds percentage/value of highest bar
            if (m_eYAxis == YAxisHits)
            // m_pReportOptions->bHistoFrequencyCountScale)
            {
                // Y scale: Frequency count
                fPercentTopBar = gex_max(fPercentTopBar, x[i]);
            }
            else
            {
                // Y scale: Percentage
                fPercentTopBar =
                    gex_max(fPercentTopBar, (100.0 * x[i]) / lTotalSamples);

            }
            // Change histogram column count value by its step position
            x[i] = fStart;
            fStart += fStep;

            // Computes the highest column found in the histogram
            // (over all groups)
            if (y[i] != GEX_C_DOUBLE_NAN)
            {
                if (mIsStacked)
                {
                    lSumPerBar[i] += y[i];
                    fPercent = gex_max(fPercent, lSumPerBar[i]);
                }
                else
                {
                    fPercent = gex_max(fPercent, y[i]);
                }
            }
        }

        // Ending point must be at Y = 0
        x[i] = fStart;
        if (bCumulativeHisto)
        {
            y[i] = 100;  // Cumulative chart, last cell is always 100%
        }
        else
        {
            // If ALL data have the same value, this is a special case
            if (fPercent == 0.0)
            {
                if (m_eYAxis == YAxisHits)
                // m_pReportOptions->bHistoFrequencyCountScale)
                {
                    fPercent = lTotalSamples;
                }
                else
                {
                    fPercent = 100.0;
                }
                y[i] = fPercent;
            }
            else
            {
                y[i] = GEX_C_DOUBLE_NAN;  // Last cell is 0 (non cumulative)
            }
        }

        // If ALL Samples equal, ensure we draw a bar
        // (only one class for all samples)
        if (lfHistoClassesWidth == 0)
        {
            y[1] = 100;
        }

        if (lTotalSamples == 0 || ptTestCell->ldSamplesValidExecs == 0)
        {
            goto NextGroup;
        }

        // Insert new curves:
        // only give an internal name if Interactive chart not hidden
        if (pChart != NULL && pChart->bVisible)
        {
            strString = pChart->strChartName;
        }
        else
        {
            strString = "";
        }


        // If single column makes Pen 20 pixels wide
        if (fDataStart == fDataEnd)
        {
            iCurvePenSize = 20;  // 20 pixel wide curve
        }
        else
        // If large chart over test results: make Pen 2pixels wide
        if ((m_pChartOverlays == NULL) && (m_nSizeMode == GEX_CHARTSIZE_LARGE))
        {
            iCurvePenSize = 2;  // 2 pixel wide curve
        }
        else
        {
            iCurvePenSize = iLineWidth;  // 1pixel wide curve is the default

        }
        // If Chart type has to plot SPLINE
        if (pChart != NULL)
        {
            bShowSpline = pChart->bFittingCurve;
            bShowBellCurve = pChart->bBellCurve;
        }
        else
        {
            GEX_ASSERT((strOptionStorageDevice == "bars") ||
                       (strOptionStorageDevice == "3d_bars") ||
                       (strOptionStorageDevice == "stack") ||
                       (strOptionStorageDevice == "bars_outline") ||
                       (strOptionStorageDevice == "curve") ||
                       (strOptionStorageDevice == "bars_curve") ||
                       (strOptionStorageDevice == "gaussian") ||
                       (strOptionStorageDevice == "bars_gaussian") ||
                       (strOptionStorageDevice == "bars_outline_gaussian"));

            if ((strOptionStorageDevice == "curve") ||
                (strOptionStorageDevice == "bars_curve"))
            {
                bShowSpline = true;
            }
            else
            {
                bShowSpline = false;
            }

            if ((strOptionStorageDevice == "gaussian") ||
                (strOptionStorageDevice == "bars_gaussian") ||
                (strOptionStorageDevice == "bars_outline_gaussian"))
            {
                bShowBellCurve = true;
            }
            else
            {
                bShowBellCurve = false;
            }
        }

        // If Chart type has to plot BARS
        if (pChart != NULL)
        {
            bVisible = pChart->bVisible;
            pChart->mIsStacked = mIsStacked;
            bShowHisto =
                (pChart->bBoxBars ||
                 pChart->bBox3DBars ||
                 pChart->mIsStacked ||
                 pChart->bLines);
            // Histogram shows lines
            if ((pChart->bBoxBars == false) &&
                (pChart->bBox3DBars == false) &&
                pChart->bLines)
            {
                nOpacity = 0xFF000000;
            }

            if ((bShowHisto == false) && (bShowSpline == false) &&
                (bShowBellCurve == false))
            {
                // Force to show bars when no options are checked
                bShowHisto = true;
            }
        }
        else
        {
            bVisible = true;
            if ((strOptionStorageDevice == "bars") ||
                (strOptionStorageDevice == "3d_bars") ||
                (strOptionStorageDevice == "stack") ||
                (strOptionStorageDevice == "bars_outline") ||
                (strOptionStorageDevice == "bars_curve") ||
                (strOptionStorageDevice == "bars_gaussian") ||
                (strOptionStorageDevice == "bars_outline_gaussian"))
            {
                bShowHisto = true;
            }
            else
            {
                bShowHisto = false;
            }

            // Histogram shows lines (not filled)
            if ((strOptionStorageDevice == "bars_outline_gaussian") ||
                (strOptionStorageDevice == "bars_outline"))
            {
                nOpacity = 0xFF000000;
            }
        }

        // Get the style for the layer
        getLayerStyles(iGroup, pChart, nFillColor, nLineColor, symbolLayer);

        if (bVisible && bShowHisto)
        {
            if (mIsStacked)
            {
                if (pBarLayer == NULL)
                {
                    pBarLayer = m_pXYChart->addBarLayer(Chart::Stack);
                    pBarLayer->addDataGroup("Stack");
                }
            }
            else
            {
                pBarLayer = m_pXYChart->addBarLayer();
                pBarLayer->addDataGroup(pGroup->strGroupName.
                                        toLatin1().constData());
            }

            if (nOpacity)
            {
                nFillColor = nOpacity;
            }

            // case 6891: Don't force multi-layer to use 3D mode
            //            Always rely on the option value
            DataSet* lDataSet = NULL;
            if (strOptionStorageDevice == "3d_bars")
            {
                lDataSet =
                    pBarLayer->addDataSet(DoubleArray(y, iHistoClasses + 2),
                                          0x80000000 | nFillColor,
                                          strString.toLatin1().constData());
                pBarLayer->set3D(5);
            }
            else
            {
                lDataSet =
                    pBarLayer->addDataSet(DoubleArray(y, iHistoClasses + 2),
                                          nFillColor,
                                          strString.toLatin1().constData());
            }

            pBarLayer->setXData(DoubleArray(x, iHistoClasses + 2));
            pBarLayer->setLineWidth(iCurvePenSize);
            pBarLayer->setBorderColor(nLineColor, 1);
            if (mIsStacked && lDataSet != NULL)
            {
                lDataSet->setDataColor(nLineColor, nLineColor);
            }
            pBarLayer->setBarGap(Chart::TouchBar);

            // Curves style, if all data on same line,
            // let's ensure we draw a vertical line
            if (fStep == 0)
            {
                pBarLayer->setBarWidth(1);
            }
        }

        // Spline plot
        if (bVisible && bShowSpline)
        {
            // first y point and last y point must be equal to 0
            y[0] = 0.0;
            y[iHistoClasses + 1] = 0.0;

            // All invalid point must be equal to 0
            for (i = 1; i <= iHistoClasses; i++)
            {
                if (y[i] == GEX_C_DOUBLE_NAN)
                {
                    y[i] = 0.0;
                }
            }

            LineLayer* pLineLayer = m_pXYChart->addLineLayer();

            pLineLayer->addDataGroup(pGroup->strGroupName.
                                     toLatin1().constData());
            pLineLayer->addDataSet(DoubleArray(y, iHistoClasses + 2),
                                   nLineColor);
            pLineLayer->setXData(DoubleArray(x, iHistoClasses + 2));
            pLineLayer->setLineWidth(iCurvePenSize);

            if (symbolLayer != Chart::NoSymbol)
            {
                pLineLayer->getDataSet(0)->setDataSymbol(symbolLayer);
            }

            // Bar chart not shown, add the curve name tp the fitting curve
            if (! bShowHisto)
            {
                pLineLayer->getDataSet(0)->
                setDataName(strString.toLatin1().constData());
            }
        }

        // Compute & Scale Mean & Sigma to correct scale
        lfMean = fCustomScaleFactor * ptTestCell->lfMean;
        pFile->FormatTestResultNoUnits(&lfMean, ptTestCell->res_scal);
        lfSigma = fCustomScaleFactor * ptTestCell->lfSigma;
        pFile->FormatTestResultNoUnits(&lfSigma, ptTestCell->res_scal);

        if (ptTestCell->lfSamplesQuartile2 != -C_INFINITE)
        {
            lfMedian = fCustomScaleFactor * ptTestCell->lfSamplesQuartile2;
            pFile->FormatTestResultNoUnits(&lfMedian, ptTestCell->res_scal);
        }

        // Gaussian / Bell curve shape plot (do not plot if sigma is 0)
        if (bVisible && bShowBellCurve)
        {
            LineLayer* pLineLayerGaussian = m_pXYChart->addLineLayer();

            // Build curve with 33 values to plot the Bell-shape curve
            double lfxNormal[33], lfyNormal[33];
            lfStep = -4;  // Starts from -4 sigma, step = 0.25 sigma
            double dMaxlfyNormal = 0.0;
            for (i = 0; i < 33; i++)
            {
                lfxNormal[i] = lfMean + lfStep * lfSigma;
                lfStep += 0.25;  // Move by 0.25 sigma
                lfyNormal[i] = lfNormalValues[i] * fPercentTopBar / 0.39894228;
                // If cumulative Histogram rendering,
                // then show cumulative Bell/Gaussian curve
                if (bCumulativeHisto && i)
                {
                    lfyNormal[i] += lfyNormal[i - 1];
                }

                if (! i)
                {
                    dMaxlfyNormal = lfyNormal[i];
                }
                else
                {
                    dMaxlfyNormal = qMax(dMaxlfyNormal, lfyNormal[i]);
                }
            }
            if (bCumulativeHisto)
            {
                for (i = 0; i < 33; i++)
                {
                    if (m_eYAxis == YAxisHits)
                    {
                        lfyNormal[i] =
                            (lfyNormal[i] / dMaxlfyNormal) * y[iHistoClasses];
                    }
                    else
                    {
                        lfyNormal[i] = 100 * lfyNormal[i] / dMaxlfyNormal;
                    }
                }
            }

            // Set Line style + symbol type + color (if any)
            // Minimum curve line width is 2 pixels
            iCurvePenSize = gex_max(iCurvePenSize, 2);

            // Compute Y to display the Bell-curve shape
            pLineLayerGaussian->
            addDataGroup(pGroup->strGroupName.toLatin1().constData());
            pLineLayerGaussian->addDataSet(DoubleArray(lfyNormal, 33),
                                           nLineColor);
            pLineLayerGaussian->setXData(DoubleArray(lfxNormal, 33));
            pLineLayerGaussian->setLineWidth(iCurvePenSize);

            if (symbolLayer != Chart::NoSymbol)
            {
                pLineLayerGaussian->getDataSet(0)->setDataSymbol(symbolLayer);
            }

            // Bar chart and fitting curve not shown,
            // add the curve name to the gaussian
            if (! bShowHisto && ! bShowSpline)
            {
                pLineLayerGaussian->getDataSet(0)->
                setDataName(strString.toLatin1().constData());
            }
        }

        // Insert markers (vertical lines): Mean, LowL, HighL
        // If request to show the Mean
        if (qslAdvHistogramMarkerOptionsList.contains(QString("mean")))
        {
            if (pChart != NULL)
            {
                cPenColor = pChart->meanColor();
                cPenWidth = (pChart->bVisible) ? pChart->meanLineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart* pLayerStyle = NULL;
                if ((iGroup - 1) >= 0 && (iGroup - 1) <
                    m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle =
                        m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                    cPenColor = pLayerStyle->meanColor();
                    cPenWidth = pLayerStyle->meanLineWidth();

                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    cPenColor = CGexReport::GetChartingColor(iGroup);
                    cPenWidth = 1;
                }
            }

            if (cPenWidth)
            {
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Max: Vertical (X axis)
                //if (pChart != NULL && pChart->getTextRotation())
                {
                    Mark* lMark =
                        addMarker(m_pXYChart->xAxis(), lfMean, nPenColor,
                                  strLabelMean.toLatin1().constData(),
                                  cPenWidth, Chart::TopRight);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                }
               /* else if (iGroup == 1)
                {
                    addMarker(m_pXYChart->xAxis(), lfMean, nPenColor,
                              strLabelMean.toLatin1().constData(), cPenWidth,
                              Chart::TopLeft);
                }
                else
                {
                    addMarker(m_pXYChart->xAxis(), lfMean, nPenColor,
                              strLabelMean.toLatin1().constData(), cPenWidth,
                              Chart::TopRight);
                }*/

                internalChartInfo.m_dMarkerMin =
                    gex_min(internalChartInfo.m_dMarkerMin, lfMean);
                internalChartInfo.m_dMarkerMax =
                    gex_max(internalChartInfo.m_dMarkerMax, lfMean);
            }
        }

        if ((qslAdvHistogramMarkerOptionsList.contains(QString("median"))) &&
            lfMedian != -C_INFINITE)
        {
            if (pChart != NULL)
            {
                cPenColor = pChart->medianColor();
                cPenWidth = (pChart->bVisible) ? pChart->medianLineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart* pLayerStyle = NULL;
                if ((iGroup - 1) >= 0 && (iGroup - 1) <
                    m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle =
                        m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                    cPenColor = pLayerStyle->medianColor();
                    cPenWidth = pLayerStyle->medianLineWidth();

                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    cPenColor = CGexReport::GetChartingColor(iGroup);
                    cPenWidth = 1;
                }
            }

            if (cPenWidth)
            {
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Max: Vertical (X axis)
               // if (pChart != NULL && pChart->getTextRotation())
                {
                    Mark* lMark =
                        addMarker(m_pXYChart->xAxis(), lfMedian, nPenColor,
                                  strLabelMedian.toLatin1().constData(),
                                  cPenWidth, Chart::TopRight);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                }
                /*else if (iGroup == 1)
                {
                    addMarker(m_pXYChart->xAxis(), lfMedian, nPenColor,
                              strLabelMedian.toLatin1().constData(), cPenWidth,
                              Chart::TopLeft);
                }
                else
                {
                    addMarker(m_pXYChart->xAxis(), lfMedian, nPenColor,
                              strLabelMedian.toLatin1().constData(), cPenWidth,
                              Chart::TopRight);
                }*/

                internalChartInfo.m_dMarkerMin =
                    gex_min(internalChartInfo.m_dMarkerMin, lfMedian);
                internalChartInfo.m_dMarkerMax =
                    gex_max(internalChartInfo.m_dMarkerMax, lfMedian);
            }
        }

        if (m_pReportOptions->bHistoMarkerMin ||
            qslAdvHistogramMarkerOptionsList.contains(QString("minimum")))
        {
            lfMin = fCustomScaleFactor * ptTestCell->lfSamplesMin;
            pFile->FormatTestResultNoUnits(&lfMin, ptTestCell->res_scal);
            if (pChart != NULL)
            {
                cPenColor = pChart->minColor();
                cPenWidth = (pChart->bVisible) ? pChart->minLineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart* pLayerStyle = NULL;
                if ((iGroup - 1) >= 0 && (iGroup - 1) <
                    m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle =
                        m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                    cPenColor = pLayerStyle->minColor();
                    cPenWidth = pLayerStyle->minLineWidth();

                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    cPenColor = CGexReport::GetChartingColor(iGroup);
                    cPenWidth = 1;
                }
            }

            if (cPenWidth)
            {
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Max: Vertical (X axis)
               // if (pChart != NULL && pChart->getTextRotation())
                {
                    Mark* lMark =
                        addMarker(m_pXYChart->xAxis(), lfMin, nPenColor,
                                  "Min", cPenWidth, Chart::TopRight);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                }
               /* else if (iGroup == 1)
                {
                    addMarker(m_pXYChart->xAxis(), lfMin, nPenColor,
                              "Min", cPenWidth, Chart::TopLeft);
                }
                else
                {
                    addMarker(m_pXYChart->xAxis(), lfMin, nPenColor,
                              "Min", cPenWidth, Chart::TopRight);
                }*/
            }
        }

        if (m_pReportOptions->bHistoMarkerMax ||
            qslAdvHistogramMarkerOptionsList.contains(QString("maximum")))
        {
            lfMax = fCustomScaleFactor * ptTestCell->lfSamplesMax;
            pFile->FormatTestResultNoUnits(&lfMax, ptTestCell->res_scal);
            if (pChart != NULL)
            {
                cPenColor = pChart->maxColor();
                cPenWidth = (pChart->bVisible) ? pChart->maxLineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart* pLayerStyle = NULL;
                if ((iGroup - 1) >= 0 && (iGroup - 1) <
                    m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle =
                        m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                    cPenColor = pLayerStyle->maxColor();
                    cPenWidth = pLayerStyle->maxLineWidth();

                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    cPenColor = CGexReport::GetChartingColor(iGroup);
                    cPenWidth = 1;
                }
            }

            if (cPenWidth)
            {
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Max: Vertical (X axis)
                //if (pChart != NULL && pChart->getTextRotation())
                {
                    Mark* lMark =
                        addMarker(m_pXYChart->xAxis(), lfMax, nPenColor,
                                  "Max", cPenWidth, Chart::TopRight);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                }
//                else if (iGroup == 1)
//                {
//                    addMarker(m_pXYChart->xAxis(), lfMax, nPenColor,
//                              "Max", cPenWidth, Chart::TopLeft);
//                }
//                else
//                {
//                    addMarker(m_pXYChart->xAxis(), lfMax, nPenColor,
//                              "Max", cPenWidth, Chart::TopRight);
//                }
            }
        }
        if (qslAdvHistogramMarkerOptionsList.contains(QString("quartile_q1")))
        {
            lfQuartileQ1 = fCustomScaleFactor * ptTestCell->lfSamplesQuartile1;
            pFile->FormatTestResultNoUnits(&lfQuartileQ1, ptTestCell->res_scal);
            if (pChart != NULL)
            {
                cPenColor = pChart->maxColor();
                cPenWidth =
                    (pChart->bVisible) ? pChart->quartileQ1LineWidth() : 0;
                if (! cPenWidth)
                {
                    cPenWidth = 1;
                }
            }
            else
            {
                // Check if index is valid
                CGexSingleChart* pLayerStyle = NULL;
                if ((iGroup - 1) >= 0 && (iGroup - 1) <
                    m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle =
                        m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                    cPenColor = pLayerStyle->maxColor();
                    cPenWidth = pLayerStyle->maxLineWidth();

                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    cPenColor = CGexReport::GetChartingColor(iGroup);
                    cPenWidth = 1;
                }
            }

            if (cPenWidth)
            {
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Max: Vertical (X axis)
//                if (pChart != NULL && pChart->getTextRotation())
                {
                    Mark* lMark =
                        addMarker(m_pXYChart->xAxis(), lfQuartileQ1,
                                  nPenColor, "Q1", cPenWidth, Chart::TopRight);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                }
//                else if (iGroup == 1)
//                {
//                    addMarker(m_pXYChart->xAxis(), lfQuartileQ1, nPenColor,
//                              "Q1", cPenWidth, Chart::TopLeft);
//                }
//                else
//                {
//                    addMarker(m_pXYChart->xAxis(), lfQuartileQ1, nPenColor,
//                              "Q1", cPenWidth, Chart::TopRight);
//                }
            }
        }

        if (qslAdvHistogramMarkerOptionsList.contains(QString("quartile_q3")))
        {
            lfQuartileQ3 = fCustomScaleFactor * ptTestCell->lfSamplesQuartile3;
            pFile->FormatTestResultNoUnits(&lfQuartileQ3, ptTestCell->res_scal);
            if (pChart != NULL)
            {
                cPenColor = pChart->maxColor();
                cPenWidth =
                    (pChart->bVisible) ? pChart->quartileQ3LineWidth() : 0;
                if (! cPenWidth)
                {
                    cPenWidth = 1;
                }
            }
            else
            {
                // Check if index is valid
                CGexSingleChart* pLayerStyle = NULL;
                if ((iGroup - 1) >= 0 && (iGroup - 1) <
                    m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle =
                        m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                    cPenColor = pLayerStyle->maxColor();
                    cPenWidth = pLayerStyle->quartileQ3LineWidth();

                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    cPenColor = CGexReport::GetChartingColor(iGroup);
                    cPenWidth = 1;
                }
            }

            if (cPenWidth)
            {
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Max: Vertical (X axis)
                //if (pChart != NULL && pChart->getTextRotation())
                {
                    Mark* lMark =
                        addMarker(m_pXYChart->xAxis(), lfQuartileQ3,
                                  nPenColor, "Q3", cPenWidth, Chart::TopRight);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                }
//                else if (iGroup == 1)
//                {
//                    addMarker(m_pXYChart->xAxis(), lfQuartileQ3, nPenColor,
//                              "Q3", cPenWidth, Chart::TopLeft);
//                }
//                else
//                {
//                    addMarker(m_pXYChart->xAxis(), lfQuartileQ3, nPenColor,
//                              "Q3", cPenWidth, Chart::TopRight);
//                }
            }
        }

        /* */
        // If request to show the 2sigma space
        if (qslAdvHistogramMarkerOptionsList.contains(QString("2sigma")))
        {
            if (pChart != NULL)
            {
                cPenColor = pChart->sigma2Color();
                cPenWidth = (pChart->bVisible) ? pChart->sigma2LineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart* pLayerStyle = NULL;
                if ((iGroup - 1) >= 0 && (iGroup - 1) <
                    m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle =
                        m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                    cPenColor = pLayerStyle->sigma2Color();
                    cPenWidth = pLayerStyle->sigma2LineWidth();

                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    cPenColor = CGexReport::GetChartingColor(iGroup);
                    cPenWidth = 1;
                }
            }

            if (cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pXYChart->dashLineColor(nTextColor,
                                                           DotDashLine);

               // if (pChart != NULL && pChart->getTextRotation())
                {
                    Mark* lMark =
                        addMarker(m_pXYChart->xAxis(), lfMean - lfSigma,
                                  nLineColor, "-1s", cPenWidth,
                                  Chart::TopRight);
                    lMark->setFontColor(nTextColor);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                    lMark =
                        addMarker(m_pXYChart->xAxis(), lfMean + lfSigma,
                                  nLineColor, "+1s", cPenWidth,
                                  Chart::TopRight);
                    lMark->setFontColor(nTextColor);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                }
//                else
//                {
//                    // 2Sigma: Vertical (X axis)
//                    addMarker(m_pXYChart->xAxis(), lfMean - lfSigma,
//                              nLineColor, "-1s", cPenWidth,
//                              Chart::TopLeft)->setFontColor(nTextColor);

//                    // 2Sigma: Vertical (X axis)
//                    addMarker(m_pXYChart->xAxis(), lfMean + lfSigma,
//                              nLineColor,"+1s", cPenWidth,
//                              Chart::TopRight)->setFontColor(nTextColor);

//                    internalChartInfo.m_dMarkerMin =
//                        gex_min(internalChartInfo.m_dMarkerMin,
//                                lfMean - lfSigma);
//                    internalChartInfo.m_dMarkerMax =
//                        gex_max(internalChartInfo.m_dMarkerMax,
//                                lfMean + lfSigma);
//                }
            }
        }

        // If request to show the 3sigma space
        if (qslAdvHistogramMarkerOptionsList.contains(QString("3sigma")))
        {
            if (pChart != NULL)
            {
                cPenColor = pChart->sigma3Color();
                cPenWidth = (pChart->bVisible) ? pChart->sigma3LineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart* pLayerStyle = NULL;
                if ((iGroup - 1) >= 0 && (iGroup - 1) <
                    m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle =
                        m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                    cPenColor = pLayerStyle->sigma3Color();
                    cPenWidth = pLayerStyle->sigma3LineWidth();

                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    cPenColor = CGexReport::GetChartingColor(iGroup);
                    cPenWidth = 1;
                }
            }

            if (cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pXYChart->dashLineColor(nTextColor,
                                                           DotDashLine);

                //if (pChart != NULL && pChart->getTextRotation())
                {
                    Mark* lMark =
                        addMarker(m_pXYChart->xAxis(), lfMean - 1.5 * lfSigma,
                                  nLineColor, "-1.5s", cPenWidth,
                                  Chart::TopRight);
                    lMark->setFontColor(nTextColor);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                    lMark =
                        addMarker(m_pXYChart->xAxis(), lfMean + 1.5 * lfSigma,
                                  nLineColor, "+1.5s", cPenWidth,
                                  Chart::TopRight);
                    lMark->setFontColor(nTextColor);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                }
//                else
//                {
//                    // 3Sigma: Vertical (X axis)
//                    addMarker(m_pXYChart->xAxis(), lfMean - 1.5 * lfSigma,
//                              nLineColor, "-1.5s", cPenWidth,
//                              Chart::TopLeft)->setFontColor(nTextColor);

//                    // 3Sigma: Vertical (X axis)
//                    addMarker(m_pXYChart->xAxis(), lfMean + 1.5 * lfSigma,
//                              nLineColor, "+1.5s", cPenWidth,
//                              Chart::TopRight)->setFontColor(nTextColor);

//                    internalChartInfo.m_dMarkerMin =
//                        gex_min(internalChartInfo.m_dMarkerMin,
//                                lfMean - 1.5 * lfSigma);
//                    internalChartInfo.m_dMarkerMax =
//                        gex_max(internalChartInfo.m_dMarkerMax,
//                                lfMean + 1.5 * lfSigma);
//                }
            }
        }

        // If request to show the 6sigma space
        if (qslAdvHistogramMarkerOptionsList.contains(QString("6sigma")))
        {
            if (pChart != NULL)
            {
                cPenColor = pChart->sigma6Color();
                cPenWidth = (pChart->bVisible) ? pChart->sigma6LineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart* pLayerStyle = NULL;
                if ((iGroup - 1) >= 0 && (iGroup - 1) <
                    m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle =
                        m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                    cPenColor = pLayerStyle->sigma6Color();
                    cPenWidth = pLayerStyle->sigma6LineWidth();

                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    cPenColor = CGexReport::GetChartingColor(iGroup);
                    cPenWidth = 1;
                }
            }

            if (cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pXYChart->dashLineColor(nTextColor,
                                                           DotDashLine);

                //if (pChart != NULL && pChart->getTextRotation())
                {
                    Mark* lMark =
                        addMarker(m_pXYChart->xAxis(), lfMean - 3 * lfSigma,
                                  nLineColor, "-3s", cPenWidth,
                                  Chart::TopRight);
                    lMark->setFontColor(nTextColor);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                    lMark =
                        addMarker(m_pXYChart->xAxis(), lfMean + 3 * lfSigma,
                                  nLineColor, "+3s", cPenWidth,
                                  Chart::TopRight);
                    lMark->setFontColor(nTextColor);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                }
//                else
//                {
//                    // 6Sigma: Vertical (X axis)
//                    addMarker(m_pXYChart->xAxis(), lfMean - 3 * lfSigma,
//                              nLineColor, "-3s", cPenWidth,
//                              Chart::TopLeft)->setFontColor(nTextColor);

//                    // 6Sigma: Vertical (X axis)
//                    addMarker(m_pXYChart->xAxis(), lfMean + 3 * lfSigma,
//                              nLineColor, "+3s", cPenWidth,
//                              Chart::TopRight)->setFontColor(nTextColor);

//                    internalChartInfo.m_dMarkerMin =
//                        gex_min(internalChartInfo.m_dMarkerMin,
//                                lfMean - 3 * lfSigma);
//                    internalChartInfo.m_dMarkerMax =
//                        gex_max(internalChartInfo.m_dMarkerMax,
//                                lfMean + 3 * lfSigma);
//                }
            }
        }

        // If request to show the 12sigma space
        if (qslAdvHistogramMarkerOptionsList.contains(QString("12sigma")))
        {
            if (pChart != NULL)
            {
                cPenColor = pChart->sigma12Color();
                cPenWidth = (pChart->bVisible) ? pChart->sigma12LineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart* pLayerStyle = NULL;
                if ((iGroup - 1) >= 0 && (iGroup - 1) <
                    m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle =
                        m_pReportOptions->pLayersStyleList.at(iGroup - 1);
                    cPenColor = pLayerStyle->sigma12Color();
                    cPenWidth = pLayerStyle->sigma12LineWidth();

                    if (! cPenWidth)
                    {
                        // Just in case line width not defined under
                        // interactive chart, then ensure we still
                        // show line in image
                        cPenWidth = 1;
                    }
                }
                else
                {
                    cPenColor = CGexReport::GetChartingColor(iGroup);
                    cPenWidth = 1;
                }
            }

            if (cPenWidth)
            {
                int nTextColor = cPenColor.rgb() & 0xffffff;
                int nLineColor = m_pXYChart->dashLineColor(nTextColor,
                                                           DotDashLine);

               // if (pChart != NULL && pChart->getTextRotation())
                {
                    Mark* lMark =
                        addMarker(m_pXYChart->xAxis(), lfMean - 6 * lfSigma,
                                  nLineColor, "-6s", cPenWidth,
                                  Chart::TopRight);
                    lMark->setFontColor(nTextColor);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                    lMark =
                        addMarker(m_pXYChart->xAxis(), lfMean + 6 * lfSigma,
                                  nLineColor, "+6s", cPenWidth,
                                  Chart::TopRight);
                    lMark->setFontColor(nTextColor);
                    //lMark->setFontAngle(pChart->getTextRotation());
                    //lMark->setPos(0, -lMark->getWidth());
                }
//                else
//                {
//                    // 12Sigma: Vertical (X axis)
//                    addMarker(m_pXYChart->xAxis(), lfMean - 6 * lfSigma,
//                              nLineColor, "-6s", cPenWidth,
//                              Chart::TopLeft)->setFontColor(nTextColor);

//                    // 12Sigma: Vertical (X axis)
//                    addMarker(m_pXYChart->xAxis(), lfMean + 6 * lfSigma,
//                              nLineColor, "+6s", cPenWidth,
//                              Chart::TopRight)->setFontColor(nTextColor);

//                    internalChartInfo.m_dMarkerMin =
//                        gex_min(internalChartInfo.m_dMarkerMin,
//                                lfMean - 6 * lfSigma);
//                    internalChartInfo.m_dMarkerMax =
//                        gex_max(internalChartInfo.m_dMarkerMax,
//                                lfMean + 6 * lfSigma);
//                }
            }
        }

        // We have a chart legend unless the chart
        // size in only 200 pixel wide (SMALL chart mode),
        // done only once at 1st group processing
        if (m_nSizeMode != GEX_CHARTSIZE_SMALL)
        {
            // Set axis titles, unless SMALL chart
            if (m_nSizeMode == GEX_CHARTSIZE_BANNER)
            {
                if (m_eYAxis == YAxisHits)
                // m_pReportOptions->bHistoFrequencyCountScale)
                {
                    strLabel = "Freq.";
                }
                else
                {
                    strLabel = "%";
                }
            }
            else
            {
                if (m_eYAxis == YAxisHits)
                // m_pReportOptions->bHistoFrequencyCountScale)
                {
                    strLabel = "Frequency (Hits count)";
                }
                else
                {
                    strLabel = "Distribution (%)";
                }
            }
            if (m_pChartOverlays != NULL &&
                m_pChartOverlays->
                getViewportRectangle()[type()].cChartOptions.bLogScaleY)
            {
                strLabel += " - Log scale";
            }

            m_pXYChart->yAxis()->setTitle(strLabel.toLatin1().constData());

            // Check if we have a custom legend to overwrite default one
            if (m_pChartOverlays &&
                m_pChartOverlays->
                getViewportRectangle()[type()].cChartOptions.bLegendY)
            {
                m_pXYChart->yAxis()->setTitle(
                    m_pChartOverlays->
                    getViewportRectangle()[type()].cChartOptions.
                    strAxisLegendY.toLatin1().constData());
            }

            // compute a string <value> units...just to extract the units
            bool bUseCurrTest =
                m_bIsAdvHistoOnFile && (iGroup != 1 || iLayerIndex != 0);
            if (m_nSizeMode == GEX_CHARTSIZE_BANNER)
            {
                strLabel.clear();

                // Ue the X axis line to include statistivs instead of X legend
                // Include: samples, mean, sigma, Cp, Cpk
                gexReport->BuildTestNameString(pFile,
                                               (! bUseCurrTest) ?
                                               referenceTestX() : ptTestCell,
                                               szTestName);

                double dValue;

                if (qslAdvHistogramFieldOptionList.contains(QString("limits")))
                {
                    double dLowLimit = (! bUseCurrTest) ?
                        referenceTestX()->GetCurrentLimitItem()->lfLowLimit : ptTestCell->GetCurrentLimitItem()->lfLowLimit;
                    double dHighLimit = (! bUseCurrTest) ?
                        referenceTestX()->GetCurrentLimitItem()->lfHighLimit : ptTestCell->GetCurrentLimitItem()->lfHighLimit;

                    // If we have to keep values in normalized format,
                    // do not rescale
                    if (scaling != "normalized")
                    {
                        // Convert LowLimit to same scale as results:
                        pFile->
                        FormatTestResultNoUnits(&dLowLimit,
                                                (! bUseCurrTest) ?
                                                referenceTestX()->llm_scal :
                                                ptTestCell->llm_scal);
                        // Normalized
                        dLowLimit *= ScalingPower(
                                (! bUseCurrTest) ?
                                referenceTestX()->llm_scal :
                                ptTestCell->llm_scal);
                        // Normalized
                        dLowLimit /= ScalingPower(
                                (! bUseCurrTest) ?
                                referenceTestX()->res_scal :
                                ptTestCell->res_scal);
                    }

                    // If we have to keep values in normalized format,
                    // do not rescale
                    if (scaling != "normalized")
                    {
                        // convert LowLimit to same scale as results:
                        pFile->
                        FormatTestResultNoUnits(&dHighLimit,
                                                (! bUseCurrTest) ?
                                                referenceTestX()->hlm_scal :
                                                ptTestCell->hlm_scal);
                        // Normalized
                        dHighLimit *= ScalingPower(
                                (! bUseCurrTest) ?
                                referenceTestX()->hlm_scal :
                                ptTestCell->hlm_scal);
                        // Normalized
                        dHighLimit /= ScalingPower(
                                (! bUseCurrTest) ?
                                referenceTestX()->res_scal :
                                ptTestCell->res_scal);
                    }

                    QString strLL = QString("%1").arg(dLowLimit);
                    if (dLowLimit <= -C_INFINITE)
                    {
                        strLL = "n/a";
                    }

                    QString strHL = QString("%1").arg(dHighLimit);
                    if (dHighLimit >= C_INFINITE)
                    {
                        strHL = "n/a";
                    }

                    strLabel += QString("Low L.=%1    High L.=%2    ").
                        arg(strLL).arg(strHL);
                }

                if (qslAdvHistogramFieldOptionList.
                    contains(QString("exec_count")))
                {
                    strLabel += QString("Samples=%1    ").
                        arg((! bUseCurrTest) ?
                            referenceTestX()->ldSamplesValidExecs :
                            ptTestCell->ldSamplesValidExecs);
                }

                if (qslAdvHistogramFieldOptionList.contains(QString("mean")))
                {
                    dValue = (! bUseCurrTest) ?
                        referenceTestX()->lfMean : ptTestCell->lfMean;
                    pFile->
                    FormatTestResultNoUnits(&dValue,
                                            (! bUseCurrTest) ?
                                            referenceTestX()->res_scal :
                                            ptTestCell->res_scal);
                    strLabel += QString("Mean=%1    ").arg(dValue);
                }

                if (qslAdvHistogramFieldOptionList.
                    contains(QString("quartile2")))
                {
                    dValue = (! bUseCurrTest) ?
                        referenceTestX()->lfSamplesQuartile2 :
                        ptTestCell->lfSamplesQuartile2;
                    pFile->
                    FormatTestResultNoUnits(&dValue,
                                            (! bUseCurrTest) ?
                                            referenceTestX()->res_scal :
                                            ptTestCell->res_scal);
                    strLabel += QString("Median=%1    ").arg(dValue);
                }

                if (qslAdvHistogramFieldOptionList.contains(QString("sigma")))
                {
                    dValue = (! bUseCurrTest) ?
                        referenceTestX()->lfSigma : ptTestCell->lfSigma;
                    pFile->
                    FormatTestResultNoUnits(&dValue,
                                            (! bUseCurrTest) ?
                                            referenceTestX()->res_scal :
                                            ptTestCell->res_scal);
                    strLabel += QString("Sigma=%1    ").arg(dValue);
                }

                if (qslAdvHistogramFieldOptionList.
                    contains(QString("fail_count")))
                {
                    strLabel += QString("Failures=%1    ").
                        arg((! bUseCurrTest) ?
                            referenceTestX()->GetCurrentLimitItem()->ldFailCount :
                            ptTestCell->GetCurrentLimitItem()->ldFailCount);
                }

                if (qslAdvHistogramFieldOptionList.
                    contains(QString("removed_count")))
                {
                    int nOutliers = ((! bUseCurrTest ?
                                      referenceTestX()->GetCurrentLimitItem()->ldOutliers :
                                      ptTestCell->GetCurrentLimitItem()->ldOutliers) < 0) ? 0 :
                        (! bUseCurrTest) ?
                        referenceTestX()->GetCurrentLimitItem()->ldOutliers : ptTestCell->GetCurrentLimitItem()->ldOutliers;
                    strLabel += QString("Removed=%1    ").arg(nOutliers);
                }

                if (qslAdvHistogramFieldOptionList.contains(QString("min")))
                {
                    dValue = (! bUseCurrTest) ?
                        referenceTestX()->lfMin : ptTestCell->lfMin;
                    pFile->
                    FormatTestResultNoUnits(&dValue,
                                            (! bUseCurrTest) ?
                                            referenceTestX()->res_scal :
                                            ptTestCell->res_scal);
                    strLabel += QString("Min=%1    ").arg(dValue);
                }

                if (qslAdvHistogramFieldOptionList.contains(QString("max")))
                {
                    dValue = (! bUseCurrTest) ?
                        referenceTestX()->lfMax : ptTestCell->lfMax;
                    pFile->
                    FormatTestResultNoUnits(&dValue,
                                            (! bUseCurrTest) ?
                                            referenceTestX()->res_scal :
                                            ptTestCell->res_scal);
                    strLabel += QString("Max=%1    ").arg(dValue);
                }

                if (qslAdvHistogramFieldOptionList.contains(QString("cp")))
                {
                    strLabel += QString("Cp=%1    ").
                        arg(gexReport->
                            CreateResultStringCpCrCpk(
                                (! bUseCurrTest) ?
                                referenceTestX()->GetCurrentLimitItem()->lfCp : ptTestCell->GetCurrentLimitItem()->lfCp));
                }

                if (qslAdvHistogramFieldOptionList.contains(QString("cpk")))
                {
                    strLabel += QString("Cpk=%1    ").
                        arg(gexReport->
                            CreateResultStringCpCrCpk(
                                (! bUseCurrTest) ?
                                referenceTestX()->GetCurrentLimitItem()->lfCpk : ptTestCell->GetCurrentLimitItem()->lfCpk));
                }

                strcpy(szString, strLabel.toLatin1().constData());
            }
            else
            {
                strLabel = "Test results ";
                QString strUnits;

                if (bUseCurrTest)
                {
                    strUnits = ptTestCell->
                        GetScaledUnits(&fCustomScaleFactor, scaling);
                }
                else
                {
                    strUnits = referenceTestX()->
                        GetScaledUnits(&fCustomScaleFactor, scaling);
                }

                if (strUnits.length() > 0)
                {
                    strLabel += "(" + strUnits + ")";
                }

                strcpy(szString, strLabel.toLatin1().constData());
                if (m_pChartOverlays != NULL &&
                    m_pChartOverlays->
                    getViewportRectangle()[type()].cChartOptions.bLogScaleX)
                {
                    strcat(szString, " - Log scale");
                }
            }

            if (iGroup == 1 && m_nSizeMode != GEX_CHARTSIZE_BANNER)
            {
                TextBox* pTextBoxTitle = NULL;

                // Check if we have a custom legend to overwrite default one
                if (m_pChartOverlays &&
                    m_pChartOverlays->
                    getViewportRectangle()[type()].cChartOptions.bLegendX)
                {
                    pTextBoxTitle =
                        m_pXYChart->xAxis()->setTitle(
                            m_pChartOverlays->
                            getViewportRectangle()[type()].cChartOptions.
                            strAxisLegendX.toLatin1().constData());
                }
                else
                {
                    pTextBoxTitle = m_pXYChart->xAxis()->setTitle(szString);
                }

                if (pTextBoxTitle)
                {
                    pTextBoxTitle->setMaxWidth(700);

                    // With version 5.X
                    if (Chart::getVersion() == 5)
                    {
                        if (pTextBoxTitle->getHeight() > bottomMargin())
                        {
                            setBottomMargin(pTextBoxTitle->getHeight());
                        }
                    }
                    // with older version (4.X)
                    else
                    {
                        if (strLabel.size() > 125)
                        {
                            setBottomMargin(50);
                        }
                    }
                }
            }
            else if (m_bIsAdvHistoOnFile && m_nSizeMode == GEX_CHARTSIZE_BANNER)
            {
                if (!strLabel.isEmpty() && !poLegendBox2)
                {
                    int iLegend2Y = m_pXYChart->getHeight() - bottomMargin() + 2;
                    poLegendBox2 =  m_pXYChart->addLegend2(m_pXYChart->getWidth() / 2,
                                                           iLegend2Y,0,0,7.5);
                    poLegendBox2->setAlignment(Chart::TopCenter);
                    poLegendBox2->setMargin(0, 0, 0, 0);
                    poLegendBox2->setLineStyleKey();
                    poLegendBox2->setText("TESTS");
                    poLegendBox2->setBackground(Chart::Transparent,
                                                Chart::Transparent);
                }

                if (poLegendBox2 && !strLabel.isEmpty())
                {
                    poLegendBox2->addKey(strLabel.toLatin1().constData(),
                                         nFillColor);
                    if (iGroup == 1)
                    {
                        poLegendBox2->setMaxWidth(poLegendBox2->getWidth());
                    }
                }
            }

        }

NextGroup:
        // If layer visible: Write Custom markers defined thru the
        // scripting interface (if a marker is sticky to a layer, it
        if (bVisible && ptTestCell)
        {
            if (m_pChartOverlays)
            {
                CGexSingleChart* pChartLayer =
                    m_pChartOverlays->chartsList().at(iLayerIndex - 1);

                if (pChartLayer && pChartLayer->bVisible)
                {
                    plotScriptingCustomMarkers(ptTestCell,
                                               nFillColor,
                                               iLayerIndex,
                                               fCustomScaleFactor,
                                               true,
                                               internalChartInfo.m_dMarkerMin,
                                               internalChartInfo.m_dMarkerMax);
                }
            }
            else
            {
                plotScriptingCustomMarkers(ptTestCell,
                                           nFillColor,
                                           iGroup,
                                           fCustomScaleFactor,
                                           true,
                                           internalChartInfo.m_dMarkerMin,
                                           internalChartInfo.m_dMarkerMax);
            }
        }

        // Now: check what viewport will be used
        switch (viewportMode())
        {
        case viewportOverLimits:
        case viewportCumulLimits:
            // Chart has to be done over limits, unless they do not exist
            if (internalChartInfo.m_dLowLimit != C_INFINITE)
            {
                // If chart over limits, update charting
                lfChartBottom = gex_min(lfChartBottom,
                                        internalChartInfo.m_dLowLimit);
            }
            else
            {
                if (internalChartInfo.m_dHighLimit != -C_INFINITE)
                {
                    lfChartBottom = gex_min(lfChartBottom,
                                            internalChartInfo.m_dHighLimit);
                }

                lfChartBottom = gex_min(lfChartBottom, fDataStart);
            }

            if (internalChartInfo.m_dHighLimit != -C_INFINITE)
            {
                // If chart over limits, update charting
                lfChartTop =
                    gex_max(lfChartTop, internalChartInfo.m_dHighLimit);
            }
            else
            {
                if (internalChartInfo.m_dLowLimit != C_INFINITE)
                {
                    lfChartTop = gex_max(lfChartTop,
                                         internalChartInfo.m_dLowLimit);
                }

                lfChartTop = gex_max(lfChartTop, fDataEnd);
            }

            lfStep = lfChartBottom;
            lfChartBottom = gex_min(lfChartBottom, lfChartTop);
            lfChartTop = gex_max(lfStep, lfChartTop);
            break;

        case viewportOverData:
        case viewportCumulData:

            // Chart viewport is data, unless specified otherwise
            lfChartBottom = gex_min(lfChartBottom, fDataStart);
            lfChartTop = gex_max(lfChartTop, fDataEnd);
            break;

        case viewportAdaptive:

            // Chart has to be done over maxi of both datapoints & limits
            if (internalChartInfo.m_dLowLimit != C_INFINITE)
            {
                lfChartBottom = gex_min(lfChartBottom,
                                        internalChartInfo.m_dLowLimit);
            }
            else if (internalChartInfo.m_dHighLimit != -C_INFINITE)
            {
                lfChartBottom = gex_min(lfChartBottom,
                                        internalChartInfo.m_dHighLimit);
            }

            if (internalChartInfo.m_dHighLimit != -C_INFINITE)
            {
                lfChartTop =
                    gex_max(lfChartTop, internalChartInfo.m_dHighLimit);
            }
            else if (internalChartInfo.m_dLowLimit != C_INFINITE)
            {
                lfChartTop = gex_max(lfChartTop, internalChartInfo.m_dLowLimit);
            }

            // Chart viewport is data, unless specified otherwise
            lfChartBottom = gex_min(lfChartBottom, fDataStart);
            lfChartTop = gex_max(lfChartTop, fDataEnd);

            if (internalChartInfo.m_dMarkerMin != C_INFINITE)
            {
                lfChartBottom = gex_min(lfChartBottom,
                                        internalChartInfo.m_dMarkerMin);
            }

            if (internalChartInfo.m_dMarkerMax != -C_INFINITE)
            {
                lfChartTop =
                    gex_max(lfChartTop, internalChartInfo.m_dMarkerMax);
            }

            break;
        }


        // Determine min and max values for the viewport
        if (internalChartInfo.m_dLowLimit != C_INFINITE
                && internalChartInfo.m_dLowLimit > -F_INFINITE
                && internalChartInfo.m_dLowLimit != -C_INFINITE
                && internalChartInfo.m_dLowLimit < F_INFINITE)
        {
            lfViewportLowX = gex_min(lfViewportLowX,
                                     internalChartInfo.m_dLowLimit);
        }
        else if (internalChartInfo.m_dHighLimit != -C_INFINITE
                 && internalChartInfo.m_dHighLimit != -F_INFINITE)
        {
            lfViewportLowX = gex_min(lfViewportLowX,
                                     internalChartInfo.m_dHighLimit);
        }

        lfViewportLowX = gex_min(lfViewportLowX, fDataStart);

        if (internalChartInfo.m_dHighLimit != -C_INFINITE
                && internalChartInfo.m_dHighLimit != -F_INFINITE)
        {
            lfViewportHighX = gex_max(lfViewportHighX,
                                      internalChartInfo.m_dHighLimit);
        }
        else if (internalChartInfo.m_dLowLimit != C_INFINITE
                 && internalChartInfo.m_dLowLimit != F_INFINITE)
        {
            lfViewportHighX = gex_max(lfViewportHighX,
                                      internalChartInfo.m_dLowLimit);
        }

        lfViewportHighX = gex_max(lfViewportHighX, fDataEnd);

        if (internalChartInfo.m_dMarkerMin != C_INFINITE
                && internalChartInfo.m_dMarkerMin != F_INFINITE)
        {
            lfViewportLowX = gex_min(lfViewportLowX,
                                     internalChartInfo.m_dMarkerMin);
        }

        if (internalChartInfo.m_dMarkerMax != -C_INFINITE
                && internalChartInfo.m_dMarkerMax != -F_INFINITE)
        {
            lfViewportHighX = gex_max(lfViewportHighX,
                                      internalChartInfo.m_dMarkerMax);
        }

        if (internalChartInfo.m_dLowSpecLimit != C_INFINITE
                && internalChartInfo.m_dLowSpecLimit > -F_INFINITE
                && internalChartInfo.m_dLowSpecLimit != -C_INFINITE
                && internalChartInfo.m_dLowSpecLimit < F_INFINITE)
        {
            lfViewportLowX = gex_min(lfViewportLowX,
                                     internalChartInfo.m_dLowSpecLimit);
        }


        if (internalChartInfo.m_dHighSpecLimit != C_INFINITE
                && internalChartInfo.m_dHighSpecLimit > -F_INFINITE
                &&internalChartInfo.m_dHighSpecLimit != -C_INFINITE
                && internalChartInfo.m_dHighSpecLimit < F_INFINITE)
        {
            lfViewportHighX = gex_max(lfViewportHighX,
                                      internalChartInfo.m_dHighSpecLimit);
        }

        if (lfMedian != -C_INFINITE)
        {
            lfViewportLowX = gex_min(lfViewportLowX, lfMedian);
        }

        if (lfMean != -C_INFINITE)
        {
            lfViewportLowX = gex_min(lfViewportLowX, lfMean);
        }
        if (lfSigma != -C_INFINITE)
        {
            lfViewportLowX = gex_min(lfViewportLowX, lfMean - 6 * lfSigma);
        }

        if (lfMedian != -C_INFINITE)
        {
            lfViewportHighX = gex_max(lfViewportHighX, lfMedian);
        }

        if (lfMean != -C_INFINITE)
        {
            lfViewportHighX = gex_max(lfViewportHighX, lfMean);
        }
        if (lfSigma != -C_INFINITE)
        {
            lfViewportHighX = gex_max(lfViewportHighX, lfMean + 6 * lfSigma);
        }

        // In case we have multiple charts, we need to merge the viewports
        lfMergedChartBottom = gex_min(lfMergedChartBottom, lfChartBottom);
        lfMergedChartTop = gex_max(lfMergedChartTop, lfChartTop);

        if (m_pChartOverlays == NULL)
        {
            ++itGroupList;
            pGroup =
                (itGroupList != gexReport->getGroupsList().end()) ?
                (*itGroupList) : NULL;

            // Keep track of group index processed
            iGroup++;
        }
    }  // Read histogram of a given test in all groups so to stack all charts

    // Y axis scale. Add 1% of scale over and under chart window,
    // to ensure markers will be seen
    fPercent *= 1.1;

    lfChartLowX = gex_min(lfChartBottom, lfChartTop);
    lfChartHighX = gex_max(lfChartBottom, lfChartTop);
    lfChartLowY = 0;  // Default: 0%
    lfChartHighY = fPercent;  // Highest histogram bar percentage

    // If custom viewport (in scripting file, not in interactive mode),
    // overwrite defaults window viewing
    if (useTestCustomViewport())
    {
        // reset the current viewport
        resetViewportManager(false);

        lfExtraLow = lfExtraHigh = 0;

        // If custom viewport in X, overwrite defaults window viewing
        if (referenceTestX()->ptChartOptions->customViewportX())
        {
            lfChartLowX = referenceTestX()->ptChartOptions->lowX();
            lfChartHighX = referenceTestX()->ptChartOptions->highX();
        }

        // If custom viewport in Y, overwrite defaults window viewing
        // case 4573, PYC, 12/04/2011
        //      if(referenceTestX()->ptChartOptions->customViewportY())
        //      {
        //          lfChartLowY  = referenceTestX()->ptChartOptions->lowY();
        //          lfChartHighY = referenceTestX()->ptChartOptions->highY();
        //      }
    }
    else
    {
        // Customer force viewport limit
        if (m_pChartOverlays &&
            m_pChartOverlays->getViewportRectangle()[type()].bForceViewport)
        {
            resetViewportManager(false);

            if (m_pChartOverlays->getViewportRectangle()[type()].lfLowX !=
                -C_INFINITE)
            {
                if (m_pChartOverlays &&
                    (m_pChartOverlays->getAppliedToChart() ==
                     GexAbstractChart::chartTypeHistogram ||
                     m_pChartOverlays->getAppliedToChart() == -1) &&
                    m_iXScaleType != 1)
                {
                    lfChartLowX =
                        m_pChartOverlays->getViewportRectangle()[type()].lfLowX;
                }
            }

            if (m_pChartOverlays->getViewportRectangle()[type()].lfHighX !=
                C_INFINITE)
            {
                if (m_pChartOverlays &&
                    (m_pChartOverlays->getAppliedToChart() ==
                     GexAbstractChart::chartTypeHistogram ||
                     m_pChartOverlays->getAppliedToChart() == -1) &&
                    m_iXScaleType != 1)
                {
                    lfChartHighX = m_pChartOverlays->
                        getViewportRectangle()[type()].lfHighX;
                }
            }

            if (m_pChartOverlays->getViewportRectangle()[type()].lfLowY !=
                -C_INFINITE)
            {
                if (m_pChartOverlays &&
                    (m_pChartOverlays->getAppliedToChart() ==
                     GexAbstractChart::chartTypeHistogram ||
                     m_pChartOverlays->getAppliedToChart() == -1) &&
                    m_iYScaleType != 1)
                {
                    lfChartLowY = gex_max(
                            m_pChartOverlays->
                            getViewportRectangle()[type()].lfLowY, lfChartLowY);
                }
            }

            if (m_pChartOverlays->getViewportRectangle()[type()].lfHighY !=
                C_INFINITE)
            {
                if (m_pChartOverlays &&
                    (m_pChartOverlays->getAppliedToChart() ==
                     GexAbstractChart::chartTypeHistogram ||
                     m_pChartOverlays->getAppliedToChart() == -1) &&
                    m_iYScaleType != 1)
                {
                    lfChartHighY = gex_max(
                            m_pChartOverlays->
                            getViewportRectangle()[type()].lfHighY,
                            lfChartHighY);
                }
            }

            if( m_pChartOverlays->getViewportRectangle()[type()].bForceViewport)
                m_pChartOverlays->getViewportRectangle()[type()].mChangeViewPort = true;

            m_pChartOverlays->getViewportRectangle()[type()].bForceViewport = false;

            lfExtraLow = lfExtraHigh = 0;
        }
        else
        {
            // Make viewport in X a bit larger than data
            lfExtraLow = lfExtraHigh =
                    (lfMergedChartTop - lfMergedChartBottom) * 0.05;
        }

        // Set Y scale
        if (m_eYAxis == YAxisHits)
        // m_pReportOptions->bHistoFrequencyCountScale)
        {
            // Make sure we have integer values
            if (lfChartHighY < 2)
            {
                lfChartHighY = 2;
            }
        }
    }

    // Add extra space
    if (m_pChartOverlays == NULL ||
        m_pChartOverlays->
        getViewportRectangle()[type()].cChartOptions.bLogScaleX == false)
    {
        if (lfChartLowX != -C_INFINITE && lfChartHighX != C_INFINITE)
        {
            lfChartLowX -= lfExtraLow;
            lfChartHighX += lfExtraHigh;
        }
    }

    lfViewportLowX = gex_min(lfViewportLowX, lfChartLowX);
    lfViewportHighX = gex_max(lfViewportHighX, lfChartHighX);

    // Define viewport: if in Interactive charting, consider zoomin factor
    if (m_pChartOverlays &&
        lfViewportHighX != -C_INFINITE &&
        lfViewportLowX != C_INFINITE)
    {
        // Initialize viewport
        if (isHorizontalViewportInitialized() == false)
        {
            initHorizontalViewport(lfViewportLowX,
                                   lfViewportHighX,
                                   lfChartLowX,
                                   lfChartHighX,
                                   0.05);
        }

        if (isVerticalViewportInitialized() == false)
        {
            if (m_pChartOverlays &&
                (m_pChartOverlays->getAppliedToChart() ==
                 GexAbstractChart::chartTypeHistogram ||
                 m_pChartOverlays->getAppliedToChart() == -1))
            {
                initVerticalViewport(lfChartLowY,
                                     lfChartHighY,
                                     lfChartLowY,
                                     lfChartHighY);
            }
        }

        // Using ViewPortLeft and ViewPortWidth,
        // get the start and end dates of the view port
        //FIXME: not used ?
        // lfChartLowXBeforeCompute = lfChartLowX;
        // lfChartHighXBeforeCompute = lfChartHighX;
        computeXBounds(lfChartLowX, lfChartHighX);
        if (m_pChartOverlays &&
            (m_pChartOverlays->getAppliedToChart() ==
             GexAbstractChart::chartTypeHistogram ||
             m_pChartOverlays->getAppliedToChart() == -1))
        {
            computeYBounds(lfChartLowY, lfChartHighY);
        }
    }

    if (lfChartLowY != -C_INFINITE && lfChartHighY != C_INFINITE)
    {
        // Check if Y scale type is: Linear or Logarithmic
        if ((m_pChartOverlays &&
             m_pChartOverlays->
             getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
             (m_pChartOverlays->getAppliedToChart() ==
              GexAbstractChart::chartTypeHistogram ||
              m_pChartOverlays->getAppliedToChart() == -1)) ||
            m_iYScaleType == 1)
        {
            m_pXYChart->yAxis()->setLogScale(0.01, lfChartHighY);
            m_iYScaleType = 1;
        }
        else if ((m_pChartOverlays &&
                  ! m_pChartOverlays->
                  getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
                  (m_pChartOverlays->getAppliedToChart() ==
                   GexAbstractChart::chartTypeHistogram ||
                   m_pChartOverlays->getAppliedToChart() == -1)) ||
                 m_iYScaleType == 0)
        {
            m_pXYChart->yAxis()->setLinearScale(lfChartLowY, lfChartHighY);
            m_iYScaleType = 0;
        }
    }

    if (lfChartLowX != -C_INFINITE && lfChartHighX != C_INFINITE)
    {
        // Check if X scale type is: Linear or Logarithmic
        if ((m_pChartOverlays &&
             m_pChartOverlays->
             getViewportRectangle()[type()].cChartOptions.bLogScaleX &&
             (m_pChartOverlays->getAppliedToChart() ==
              GexAbstractChart::chartTypeHistogram ||
              m_pChartOverlays->getAppliedToChart() == -1)) ||
            m_iXScaleType == 1)
        {
            m_pXYChart->xAxis()->setLogScale(lfChartLowX, lfChartHighX);
            m_iXScaleType = 1;
        }
        else if ((m_pChartOverlays &&
                  ! m_pChartOverlays->
                  getViewportRectangle()[type()].cChartOptions.bLogScaleX &&
                  (m_pChartOverlays->getAppliedToChart() ==
                   GexAbstractChart::chartTypeHistogram ||
                   m_pChartOverlays->getAppliedToChart() == -1)) ||
                 m_iXScaleType == 0)
        {
            m_pXYChart->xAxis()->setLinearScale(lfChartLowX, lfChartHighX);
            m_iXScaleType = 0;
        }
    }

    if (m_pReportOptions->bPlotLegend)
    {
        m_pXYChart->addLegend(75, 30)->
        setBackground(0x80000000 | (QColor(Qt::white).rgb() & 0xffffff));
    }

    if (m_pChartOverlays)
    {
        m_pChartOverlays->getViewportRectangle()[type()].lfLowX = lfChartLowX;
        m_pChartOverlays->getViewportRectangle()[type()].lfLowY = lfChartLowY;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighX = lfChartHighX;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighY = lfChartHighY;
    }

end_of_histo:
    // Free buffers allocated
    delete[] x;
    delete[] y;
    delete[] lSumPerBar;
}

/******************************************************************************!
 * \fn makeTooltip
 ******************************************************************************/
QString GexHistogramChart::makeTooltip()
{
    QString strTooltip;

    if (referenceTestX() && isViewportInitialized())
    {
        QString strUnitsX;
        QString strUnitsY;
        QString strValue;
        double lfCustomScaleFactor;

        QString scalingOption =
            ReportOptions.GetOption("dataprocessing", "scaling").toString();

        // Get X and Y units (if any)
        strUnitsX = referenceTestX()->GetScaledUnits(&lfCustomScaleFactor,
                                                     scalingOption);
        strUnitsX.truncate(10);

        if (referenceTestY() != NULL)
        {
            strUnitsY = referenceTestY()->GetScaledUnits(&lfCustomScaleFactor,
                                                         scalingOption);
            strUnitsY.truncate(10);
        }

        if (m_dXHotSpotValue != GEX_C_DOUBLE_NAN ||
            m_dYHotSpotValue != GEX_C_DOUBLE_NAN)
        {
            // Check if Histogram Y scale is frequency count, or %
            if (m_eYAxis == YAxisHits)
            // m_pReportOptions->bHistoFrequencyCountScale)
            {
                strValue.sprintf("X: %12f %s\nY: %.0f Hits",
                                 m_dXHotSpotValue,
                                 strUnitsX.toLatin1().constData(),
                                 m_dYHotSpotValue);
            }
            else
            {
                strValue.sprintf("X: %12f %s\nY: %.2f %%",
                                 m_dXHotSpotValue,
                                 strUnitsX.toLatin1().constData(),
                                 m_dYHotSpotValue);
            }

            if (! m_strGroupName.isEmpty())
            {
                strTooltip = "Group: " + m_strGroupName;
                strTooltip += "\n";
            }

            strTooltip += strValue;
        }
        else if (m_dXCursorValue != GEX_C_DOUBLE_NAN ||
                 m_dYCursorValue != GEX_C_DOUBLE_NAN)
        {
            strTooltip.sprintf("X: %12f %s",
                               m_dXCursorValue,
                               strUnitsX.toLatin1().constData());
        }

        // If log scale, do not show the data pointed, don't know its location
        if (m_pChartOverlays &&
            m_pChartOverlays->getViewportRectangle().contains(type()))
        {
            if (m_pChartOverlays->
                getViewportRectangle()[type()].cChartOptions.bLogScaleX ||
                m_pChartOverlays->
                getViewportRectangle()[type()].cChartOptions.bLogScaleY)
            {
                strTooltip = "Plot Area";
            }
        }
    }

    return strTooltip;
}

void GexHistogramChart::buildGeometry()
{
    GexAbstractChart::buildGeometry();

    switch(m_nSizeMode)
    {
        case GEX_CHARTSIZE_BANNER:
        {
            setBottomMargin(25);

            QStringList lFieldOptions = m_pReportOptions->GetOption(QString("adv_histogram"),
                                                                    QString("field")).toString().split("|", QString::SkipEmptyParts);

            if (lFieldOptions.isEmpty() == false)
            {
                int lGroupCount = gexReport->getGroupsList().count();
                int lYExtent    = 2 + (11 * lGroupCount);

                setHeight(height() + lYExtent);
                setBottomMargin(bottomMargin() + lYExtent);
            }
            break;
        }

        default:
            break;
    }
}

void GexHistogramChart::fitPlotArea()
{
    if (m_nSizeMode == GEX_CHARTSIZE_BANNER)
    {
        m_pXYChart->packPlotArea(0, top(), 0, bottom());

        setBottomMargin(bottomMargin() + m_pXYChart->xAxis()->getThickness());

        int nLeftMargin = leftMargin();
        int nTopMargin  =  topMargin();

        int nAreaWidth  = width() - horizontalMargin();
        int nAreaHeight = height() - verticalMargin();

        m_pXYChart->setPlotArea(nLeftMargin, nTopMargin, nAreaWidth, nAreaHeight);
    }
}

/******************************************************************************!
 * \fn setViewportModeFromChartMode
 * \brief Convert histogram chart mode from Gex into viewport mode
 ******************************************************************************/
void GexHistogramChart::setViewportModeFromChartMode(int nChartMode)
{
    int nViewportMode;

    switch (nChartMode)
    {
    case GEX_ADV_HISTOGRAM_OVERLIMITS: nViewportMode = viewportOverLimits;
        break;
    case GEX_ADV_HISTOGRAM_OVERDATA: nViewportMode = viewportOverData;
        break;
    case GEX_ADV_HISTOGRAM_DATALIMITS: nViewportMode = viewportAdaptive;
        break;
    case GEX_ADV_HISTOGRAM_CUMULLIMITS: nViewportMode = viewportCumulLimits;
        break;
    case GEX_ADV_HISTOGRAM_CUMULDATA: nViewportMode = viewportCumulData;
        break;
    default: nViewportMode = viewportOverLimits;
        break;
    }

    setViewportMode(nViewportMode);
}

/******************************************************************************!
 * \fn onOptionChanged
 * \brief Act according the option which was changed
 ******************************************************************************/
void
GexHistogramChart::onOptionChanged(GexAbstractChart::chartOption eChartOption)
{
    switch (eChartOption)
    {
    case GexAbstractChart::chartOptionHistoBar:
    case GexAbstractChart::chartOptionHistoYScale:
        resetVerticalViewport();
        break;
    default:
        break;
    }
}

/******************************************************************************!
 * \fn keepXTestViewport
 * \brief Keep wiewport for test X
 ******************************************************************************/
void GexHistogramChart::keepXTestViewport()
{
    if (m_pChartOverlays && referenceTestX())
    {
        referenceTestX()->ptChartOptions->setCustomViewportX(true);
        referenceTestX()->ptChartOptions->setLowX(
            m_pChartOverlays->getViewportRectangle()[type()].lfLowX);
        referenceTestX()->ptChartOptions->setHighX(
            m_pChartOverlays->getViewportRectangle()[type()].lfHighX);

        referenceTestX()->ptChartOptions->setCustomViewportY(true);
        referenceTestX()->ptChartOptions->setLowY(
            m_pChartOverlays->getViewportRectangle()[type()].lfLowY);
        referenceTestX()->ptChartOptions->setHighY(
            m_pChartOverlays->getViewportRectangle()[type()].lfHighY);
    }
}

/******************************************************************************!
 * \fn buildChartFromData
 ******************************************************************************/
void GexHistogramChart::buildChartFromData(char** pszLegend,
                                           double* dY,
                                           int iCount,
                                           const QString& strTitle,
                                           const QString& strXTitle,
                                           const QString& strYTitle)
{
    // Build chart geometry
    GexAbstractChart::buildGeometry();

    // Size the bottom margin to 25
    setBottomMargin(25);

    // Allocate a new XYChart
    if (initXYChart())
    {
        setBottomMargin(bottomMargin());
        // Non-custom title
        m_pXYChart->addTitle(strTitle.toLatin1().constData());
        // Get the style for the layer
        int nFillColor = 0;
        int nLineColor = 0;
        Chart::SymbolType symbolLayer = Chart::NoSymbol;
        getLayerStyles(0, 0, nFillColor, nLineColor, symbolLayer);
        BarLayer* pBarLayer = m_pXYChart->addBarLayer();
        pBarLayer->addDataGroup(gexReport->getGroupsList().first()->
                                strGroupName.toLatin1().constData());
        pBarLayer->addDataSet(DoubleArray(dY, iCount), nFillColor,
                              strTitle.toLatin1().constData());

        m_pXYChart->xAxis()->setLabels(StringArray(pszLegend, iCount));
        m_pXYChart->xAxis()->setLabelStyle("", 8, TextColor, 45.0);
        pBarLayer->setLineWidth(1);
        pBarLayer->setBorderColor(nLineColor, 1);
        pBarLayer->setBarGap(0.1);
        m_pXYChart->xAxis()->setTitle(strXTitle.toLatin1().constData());
        m_pXYChart->yAxis()->setTitle(strYTitle.toLatin1().constData());

        // Adjust the plot area
        sizePlotArea();
        // Add custom draw on chart
        customizeChart();

        // keep viewport
        keepViewport();
    }
}
