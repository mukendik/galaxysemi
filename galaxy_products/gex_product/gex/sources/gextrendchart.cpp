///////////////////////////////////////////////////////////////////////////////////
// GEX includes
///////////////////////////////////////////////////////////////////////////////////
#include "browser_dialog.h"
#include "drill_chart.h"
#include "gextrendchart.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "cpart_info.h"
#include "report_options.h"
#include <gqtl_global.h>

#include <gqtl_log.h>

///////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport *     gexReport;
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
extern GexMainwindow *	pGexMainWindow;

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern double	ScalingPower(int iPower);

///////////////////////////////////////////////////////////////////////////////////
// Class GexTrendChart - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexTrendChart::GexTrendChart(bool bPluginCall, int nSizeMode, GexWizardChart* lWizardParent, CGexChartOverlays * pChartOverlays /*= NULL*/) : GexAbstractChart(chartTypeTrend, nSizeMode, lWizardParent, pChartOverlays)
{
    m_bPluginCall		= bPluginCall;
    m_bValidDie			= false;
    m_bNumericalPartID	= true;

    m_nXDie				= 0;
    m_nYDie				= 0;

    m_pMarkerGroup		= NULL;
    m_pMarkerFile		= NULL;

    m_nMaxTextHeight	= 0;
    m_nMaxTextWidth		= 0;

    m_bShowPartID		= false;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexTrendChart::~GexTrendChart()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexTrendChart::computeDataset()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexTrendChart::computeDataset()
{
    if (m_pReportOptions)
        m_bShowPartID	= (m_pReportOptions->GetOption("dataprocessing", "part_id").toString() == "show") ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void buildOwnerChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexTrendChart::buildOwnerChart()
{
    CTest *	ptTestCell					= NULL;
    CTest *	ptScaleTestCellReference	= referenceTestX();	// Pointer to Test used a reference for scaling if multiple layers involved
    char	szTestName[2*GEX_MAX_STRING];
    char	szXscaleLabel[50];
    char	szString[300];
    int		i,iIndex;
    int		iCurvePenSize;					// curve pen size 1 or 2 pixels.
    double	*xArray=NULL;								// buffer to holds X data.
    double	*yArray=NULL;								// Buffer to hold trend data in case we have multiple layers and need to rescale datalog results.
    double	lfMean,lfMin,lfMax,lfMedian,lfSigma,lfExtra,lfChartBottom=C_INFINITE,lfChartTop=-C_INFINITE,lfHighL,lfLowL;
    double	lfLowSpecL;
    double	lfHighSpecL;
    double	lfDataStart = C_INFINITE, lfDataEnd = -C_INFINITE;
    double	lStartID = 0.0, lEndID = 0.0, lStep;
    double	lfTotalSamplesInX = 0.0;
    double	lfViewportLowY		= C_INFINITE;
    double	lfViewportHighY		= -C_INFINITE;
    double	lfSwap;
    QString	strString;
    double dDiff =0;
    double	lfQuartileQ1=-C_INFINITE;
    double	lfQuartileQ3=-C_INFINITE;

    GexInternalChartInfo	internalChartInfo;
    QList<GexTrendPartInfo> lstPartInfo;

    // Pointer to label strings...may vary depending of the chart size!
    int		lTestNumber;
    int		lPinmapIndex;
    int		iGroup=1;						// Counter used to know on which group we are (#1 or other)
    bool	bTestReferenceScaleFactor = false;
    int		iTestReferenceScaleFactor = 0;	// Scale factor for 1st test (Chart scales based on it)
    int		iCustomScaleFactor=0;				// Correction custom Scale factor to apply on all but 1st chart so all scales match!
    double	fCustomScaleFactor = 0.0;		// Scale ratio to apply multiplyer)
    double	lfDataOffset=0.0;				// Layer may have a custom offset (user defined)
    double	lfDataScale=1.0;				// Layer may have a custom scale (user defined)
    QString	strLabel;
    QString	strTitle;
    long	ldTotalValidSamples=0;			// Max nb of valid samples (over all layers)

    Chart::SymbolType	symbolLayer;
    int					nLineColor;
    int					nFillColor;

    bool				bIsMultiLayer = isMultiLayers();

    m_pXYChart->setAntiAlias(false);

    // Index used to keep track of Chart layers being charted.
    QString         lParameterName;
    CGexSingleChart	*pChart=NULL;	// Handle to Parameter Layer info.
    int				iLineWidth=1;
    int				iLayerIndex=0;
    QColor			cPenColor;
    int				cPenWidth;
    bool			bVisible;
    bool            lShowTitle = false;
    QColor			cLayerColor;	// Charting color for a given layer
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    QString scaling=m_pReportOptions->GetOption("dataprocessing","scaling").toString();

    QString strAdvTrendMarkerOptions = (m_pReportOptions->GetOption(QString("adv_trend"), QString("marker"))).toString();
    QStringList qslAdvTrendMarkerOptionList = strAdvTrendMarkerOptions.split(QString("|"));

    if ((m_pChartOverlays && m_pChartOverlays->mTitle == true) ||
        qslAdvTrendMarkerOptionList.contains(QString("test_name")))
    {
        lShowTitle = true;
    }

    // Clear PartID
    m_lstPartID.clear();

    // Get pointer to first group & first file (we always have them exist)
    QList<CGexGroupOfFiles*>::iterator	itGroupList = gexReport->getGroupsList().begin();
    CGexGroupOfFiles *					pGroup		= (itGroupList != gexReport->getGroupsList().end()) ? (*itGroupList) : NULL;
    CGexFileInGroup *					pFile		= NULL;

    // If not a parametric / multiparametric (eg: functional) test, ignore!
    if(referenceTestX()->bTestType == 'F')
        goto end_trend_chart;

    // Stack all charts of all groups for each same test
    while(pGroup != NULL)
    {
        if(m_pChartOverlays)
        {
            // Get handle to the Layer to plot
            if (iLayerIndex >= 0 && iLayerIndex < m_pChartOverlays->chartsList().count())
                pChart = m_pChartOverlays->chartsList().at(iLayerIndex);
            else
                break;

            // Seek to the relevant Group to plot.
            if ((pChart->iGroupX < 0) || (pChart->iGroupX >= gexReport->getGroupsList().size()))
                pGroup = NULL;
            else
                pGroup = gexReport->getGroupsList().at(pChart->iGroupX);

            // We have multiple charts (layers) to plot!
            lTestNumber		= pChart->iTestNumberX;
            lPinmapIndex	= pChart->iPinMapX;
            lParameterName  = pChart->strTestNameX;

            iLayerIndex++;
        }
        else
        {
            // Find test cell: RESET list to ensure we scan list of the right group!
            // Check if pinmap index...
            lTestNumber		= referenceTestX()->lTestNumber;
            lParameterName  = referenceTestX()->strTestName;

            if(referenceTestX()->lPinmapIndex >= 0)
                lPinmapIndex = referenceTestX()->lPinmapIndex;
            else
                lPinmapIndex = GEX_PTEST;
        }

        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        // No test cell found or no sample results, then go to the next group
        if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,FALSE,FALSE,lParameterName) !=1)
            goto NextGroup_FillPartInfo;

        // If multiple charts (groups or overlays), scale based on 1st parameter in 1st group.
        if(ptTestCell->m_testResult.count() > 0 && bTestReferenceScaleFactor == false)
        {
            iTestReferenceScaleFactor	= ptTestCell->res_scal;
            ptScaleTestCellReference	= ptTestCell;

            fillPartInfoReferenceMap();
            bTestReferenceScaleFactor = true;
        }

    NextGroup_FillPartInfo:
        if(m_pChartOverlays == NULL)
        {
            ++itGroupList;

            pGroup = (itGroupList != gexReport->getGroupsList().end()) ? (*itGroupList) : NULL;
        }
    };	// Read trend of a given test in all groups so to stack all charts.

    // Plot all groups...unless in interactive mode (then only plot the layers defined in the layer list)
    iLayerIndex				= 0;
    iGroup					= 1;
    itGroupList				= gexReport->getGroupsList().begin();
    pGroup					= (itGroupList != gexReport->getGroupsList().end()) ? (*itGroupList) : NULL;

    // Stack all charts of all groups for each same test
    while(pGroup != NULL)
    {
        if(m_pChartOverlays)
        {
            // Get handle to the Layer to plot
            if (iLayerIndex >= 0 && iLayerIndex < m_pChartOverlays->chartsList().count())
                pChart = m_pChartOverlays->chartsList().at(iLayerIndex);
            else
                break;

            // Seek to the relevant Group to plot.
            if ((pChart->iGroupX < 0) || (pChart->iGroupX >= gexReport->getGroupsList().size()))
                pGroup = NULL;
            else
                pGroup = gexReport->getGroupsList().at(pChart->iGroupX);

            // We have multiple charts (layers) to plot!
            lTestNumber     = pChart->iTestNumberX;
            lPinmapIndex    = pChart->iPinMapX;
            lParameterName  = pChart->strTestNameX;

            // Get ploting details
            if(pChart->bLines)
                iLineWidth = pChart->iLineWidth;
            else
                iLineWidth = 0;

            // Flag saying if layer is visible...
            bVisible = pChart->bVisible;

            // Next layerID to chart
            iLayerIndex++;
        }
        else
        {
            // Find test cell: RESET list to ensure we scan list of the right group!
            // Check if pinmap index...
            lTestNumber		= referenceTestX()->lTestNumber;
            lParameterName  = referenceTestX()->strTestName;

            if(referenceTestX()->lPinmapIndex >= 0)
                lPinmapIndex = referenceTestX()->lPinmapIndex;
            else
                lPinmapIndex = GEX_PTEST;

            // Chart always visible when creating an image.
            bVisible = true;
            if ( m_pReportOptions->GetOption("adv_trend","chart_type").toString()=="spots"  )
                iLineWidth = 0;
        }

        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        // No test cell found or no sample results, then go to the next group
        if(pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,FALSE,FALSE,lParameterName) !=1)
            goto NextGroup;

        if (m_pChartOverlays
                || (m_pReportOptions->isReportOutputHtmlBased()) )  //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE") )
        {
            // We have a chart title unless the chart size in only 200 pixel wide (SMALL chart mode)!
            if((m_nSizeMode != GEX_CHARTSIZE_SMALL) && (lShowTitle)
                    && (iGroup == 1) && ( (m_pChartOverlays == NULL) || (iLayerIndex == 1) ) )
            {
                if(bIsMultiLayer && (isSingleTestAllLayers() == false))
                {
                    // In multi-layer mode and with multiple layers shown, title is generic:
                    strTitle = "Overlay Tests/Parameters";
                }
                else
                {
                    gexReport->BuildTestNameString(pFile, ptTestCell,szTestName);
                    QString strNormalizedName = gexReport->buildDisplayName(szTestName, false);
                    strTitle = QString("Test %1: %2").arg(ptTestCell->szTestLabel).arg(strNormalizedName);

                    if (bTestReferenceScaleFactor == false)
                        strTitle += "<*color=FF0000*> - No data samples<*/color*>";
                }
            }
            else
                strTitle.clear();	// No title!

            // Check if we have a custom title to overwrite default one
            if(m_pChartOverlays)
            {
                // Multilayrs, check if we have assigned it a title...
                if(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bChartTitle)
                    strTitle = gexReport->buildDisplayName(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strChartTitle, false);	// custom title.
            }

            m_pXYChart->addTitle(strTitle.toLatin1().constData());			// Non-custom title
        }

        // No data available!
        if(ptTestCell->m_testResult.count() == 0)
            goto NextGroup;

        // Fill the part info list for this dataset
        fillPartInfoList(pGroup, &lstPartInfo);

        // For tests such as binning type = '-'), do not rescale, align to first layer scale.
        if(ptTestCell->bTestType == '-')
            fCustomScaleFactor = 1;
        else
        {
            iCustomScaleFactor = ptTestCell->res_scal - iTestReferenceScaleFactor;
            fCustomScaleFactor = 1/GS_POW(10.0,iCustomScaleFactor);
        }
        if(bVisible && pChart)
        {
            lfDataOffset = pChart->lfDataOffset;	// Layer may have a custom offset (user defined)
            lfDataScale  = pChart->lfDataScale;		// Layer may have a custom scale (user defined)
        }
        else
        {
            lfDataOffset=0.0;				// Layer may have a custom offset (user defined)
            lfDataScale=1.0;				// Layer may have a custom scale (user defined)
        }

        xArray = new double[ptTestCell->ldSamplesValidExecs];
        if(xArray == NULL)
            return;

        if	(m_pChartOverlays == NULL && of=="CSV") //m_pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_CSV)
        {
            // CSV dump...
            if(gexReport->reportFileHandle())
            {
                gexReport->BuildTestNameString(pFile,ptTestCell,szTestName);
                fprintf(gexReport->reportFileHandle(),"\nTest %s: %s, Group name: %s\n", ptTestCell->szTestLabel, szTestName, pGroup->strGroupName.toLatin1().constData());
            }
        }

        if(ptTestCell->ldSamplesValidExecs > 0)
        {
            //  X axis will simply show Part count starting from ID=1....to N
            lStep = 1;
            strcpy(szXscaleLabel,"Data count");
        }
        else
        {
            lStep = 0;
            strcpy(szXscaleLabel,"No data");
        }

        // Init. X values to sample#.
        lStartID = lEndID = 1;

        if (m_pReportOptions->GetOption("adv_trend","x_axis").toString()=="part_id")	//(m_pReportOptions->iTrendXAxis == GEX_TREND_XAXIS_PARTID)
        {
            iIndex		= 0;
            lStartID	= C_INFINITE;
            lEndID		= -C_INFINITE;

            for (int nPartInfo = 0; nPartInfo < lstPartInfo.count(); nPartInfo++)
            {
                if(ptTestCell->m_testResult.isValidResultAt(lstPartInfo.at(nPartInfo).runID()))
                {
                    if (ptTestCell->m_testResult.isMultiResultAt(lstPartInfo.at(nPartInfo).runID()))
                    {
                        for (int nCount = 0; nCount < ptTestCell->m_testResult.at(lstPartInfo.at(nPartInfo).runID())->count(); ++nCount)
                        {
                            if(!ptTestCell->m_testResult.at(lstPartInfo.at(nPartInfo).runID())->isValidResultAt(nCount))
                                continue;
                            xArray[iIndex++] = lstPartInfo.at(nPartInfo).posID();
                        }
                    }
                    else
                        xArray[iIndex++] = lstPartInfo.at(nPartInfo).posID();

                    lStartID	= gex_min(lStartID, lstPartInfo.at(nPartInfo).posID());
                    lEndID		= gex_max(lEndID, lstPartInfo.at(nPartInfo).posID());
                }
            }

            // No parts
            if (lStartID == C_INFINITE && lEndID == -C_INFINITE)
                lStartID = lEndID = 1;
        }
        else
        {
            for(i = 0, iIndex = 0; i < ptTestCell->ldSamplesExecs; i++)
            {

                if (ptTestCell->m_testResult.isValidResultAt(i))
                {
                    if (ptTestCell->m_testResult.isMultiResultAt(i))
                    {
                        for (int nCount = 0; nCount < ptTestCell->m_testResult.at(i)->count(); ++nCount)
                        {
                            if(!ptTestCell->m_testResult.at(i)->isValidResultAt(nCount))
                                continue;
                            xArray[iIndex++] = lEndID;
                        }
                    }
                    else
                        xArray[iIndex++] = lEndID;
                }

                lEndID += lStep;
            }
        }

        // Keep track of total samples in X (if multi-layers, make sure we adjust to largest data set)
        lfTotalSamplesInX = gex_max(lfTotalSamplesInX,lEndID);

        // Keep track of max valid samples n X
        ldTotalValidSamples = gex_max(ldTotalValidSamples, ptTestCell->ldSamplesValidExecs);

        // Insert new curves: only give an internal name if Interactive chart and not hidden
        if(pChart != NULL  && pChart->bVisible)
            strString = pChart->strChartName;
        else
            strString = "";

        // Compute start-end of test results (scale data to correct scale)
        lfExtra = lfDataOffset+(ptTestCell->lfSamplesMax*lfDataScale);
        if(iCustomScaleFactor)
            lfExtra *= fCustomScaleFactor;
        pFile->FormatTestResultNoUnits(&lfExtra,ptTestCell->res_scal);
        lfDataEnd = gex_max(lfDataEnd,lfExtra);

        lfExtra = lfDataOffset+(ptTestCell->lfSamplesMin*lfDataScale);
        if(iCustomScaleFactor)
            lfExtra *= fCustomScaleFactor;
        pFile->FormatTestResultNoUnits(&lfExtra,ptTestCell->res_scal);
        lfDataStart= gex_min(lfDataStart,lfExtra);

        if (ldTotalValidSamples == 0)
            goto NextGroup;

        // Insert new curves
        if (m_pChartOverlays || m_pReportOptions->isReportOutputHtmlBased() )  //of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        {
            // Set curve styles: if curve is a straight line, use 2pixels size so Mean line doesn't hide it!
            if(ptTestCell->lfSamplesMax == ptTestCell->lfSamplesMin)
                iCurvePenSize = 2;	// 2pixel wide curve
            else
                iCurvePenSize = iLineWidth;	// Xpixel wide curve (1 is default)

            // Set Line style + symbol type + color (if any)
            getLayerStyles(iGroup, pChart, nFillColor, nLineColor, symbolLayer);

            // tmp variables
            int nTestCellRsltCnt = ptTestCell->ldSamplesValidExecs;
            bool bIsAdvTrendXAxisPartId = m_pReportOptions->GetOption("adv_trend","x_axis").toString()=="part_id";
            int nCopyIndex=0, nDataIndex=0, nDataCount = 0;
            iIndex = 0;
            if(bIsAdvTrendXAxisPartId)
                nDataCount = lstPartInfo.count();
            else
                nDataCount = ptTestCell->m_testResult.count();

            double lfScaleFactor = 1;
            if(iCustomScaleFactor)
                lfScaleFactor=fCustomScaleFactor;

            if(yArray)
                delete yArray;
            yArray = new double[nTestCellRsltCnt];
            if(!yArray)
                return;

            for(nCopyIndex=0; nCopyIndex<nDataCount; nCopyIndex++)
            {
                nDataIndex = bIsAdvTrendXAxisPartId ? lstPartInfo.at(nCopyIndex).runID() : nCopyIndex;
                GEX_ASSERT(ptTestCell->m_testResult.isValidIndex(nDataIndex));

                if(ptTestCell->m_testResult.isValidResultAt(nDataIndex))
                {
                    if(ptTestCell->m_testResult.isMultiResultAt(nDataIndex))
                    {
                        for(int nCount = 0; nCount<ptTestCell->m_testResult.at(nDataIndex)->count(); ++nCount)
                        {
                            if(ptTestCell->m_testResult.at(nDataIndex)->isValidResultAt(nCount))
                            {
                                yArray[iIndex++] = lfDataOffset + (ptTestCell->m_testResult.at(nDataIndex)->multiResultAt(nCount) * lfScaleFactor * lfDataScale);
                            }
                        }
                    }
                    else{
                        yArray[iIndex++] = lfDataOffset + (ptTestCell->m_testResult.resultAt(nDataIndex) * lfScaleFactor * lfDataScale);
                    }
                }
            }

            // Insert Curve (if visible)
            if(bVisible)
            {
                LineLayer * pLineLayer = m_pXYChart->addLineLayer();

                pLineLayer->addDataGroup(pGroup->strGroupName.toLatin1().constData());
                pLineLayer->addDataSet(DoubleArray(yArray, iIndex), nLineColor, strString.toLatin1().constData());
                pLineLayer->setLineWidth(iCurvePenSize);
                pLineLayer->setXData(DoubleArray(xArray, iIndex));

                if (symbolLayer != Chart::NoSymbol)
                    pLineLayer->getDataSet(0)->setDataSymbol(symbolLayer, 6, nFillColor, nFillColor);
            }

            delete xArray;
            xArray=0;	// delete array as now a copy is in the 'plot' object.

            delete yArray;
            yArray=0;
        }
        else
        {
            // CSV file: dump data to CSV file.
            if(gexReport->reportFileHandle())
            {
                fprintf(gexReport->reportFileHandle(),"Step/EventID,");

                for(i = 0; i < ptTestCell->ldSamplesExecs; i++)
                {
                    if(ptTestCell->m_testResult.isValidResultAt(i))
                    {
                        if(ptTestCell->m_testResult.isMultiResultAt(i)){
                            for (int nCount = 0; nCount < ptTestCell->m_testResult.at(i)->count(); ++nCount){
                                if(!ptTestCell->m_testResult.at(i)->isValidResultAt(nCount)) continue;
                                fprintf(gexReport->reportFileHandle(),"%f,",xArray[i]);
                            }
                        }
                        else
                            fprintf(gexReport->reportFileHandle(),"%f,",xArray[i]);
                    }
                }

                fprintf(gexReport->reportFileHandle(),"\nValue,");

                //for(i = 0; i < ptTestCell->ldSamplesExecs; i++)
                for(i = 0; i < ptTestCell->m_testResult.count(); i++)
                {
                    if(ptTestCell->m_testResult.isValidResultAt(i))
                    {
                        if(ptTestCell->m_testResult.isMultiResultAt(i))
                        {
                            for (int nCount = 0; nCount < ptTestCell->m_testResult.at(i)->count(); ++nCount){
                                if(!ptTestCell->m_testResult.at(i)->isValidResultAt(nCount)) continue;
                                fprintf(gexReport->reportFileHandle(), "%f,", lfDataOffset + (lfDataScale * ptTestCell->m_testResult.at(i)->multiResultAt(nCount)));
                            }
                        }
                        else
                            fprintf(gexReport->reportFileHandle(), "%f,", lfDataOffset + (lfDataScale * ptTestCell->m_testResult.resultAt(i)));
                    }
                }

                fprintf(gexReport->reportFileHandle(),"\n");
            }

            delete xArray; xArray=0;	// delete array as now a copy is in the 'plot' object.
            goto NextGroup;
        }

        // Draw limit markers (done only once: when charting Plot for group#1)
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
        {
            lfLowL = ptTestCell->GetCurrentLimitItem()->lfLowLimit;		// Low limit exists

            // If we have to keep values in normalized format, do not rescale!
            if (scaling!="normalized")
            {
                // convert LowLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfLowL,ptTestCell->llm_scal);
                lfLowL *=  ScalingPower(ptTestCell->llm_scal);	// normalized
                lfLowL /=  ScalingPower(ptTestCell->res_scal);	// normalized
            }
            if(iCustomScaleFactor)
                lfLowL *= fCustomScaleFactor;

            // Update global lowest limit for chart viewport
            internalChartInfo.m_dLowLimit = gex_min(internalChartInfo.m_dLowLimit, lfLowL);

            // LowLimit Marker
            if(qslAdvTrendMarkerOptionList.contains(QString("limits")))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = (pChart->bVisible) ? pChart->limitsLineWidth() : 0;
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart	*pLayerStyle = NULL;
                    if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
                        cPenColor	= pLayerStyle->limitsColor(bIsMultiLayer);
                        cPenWidth	= pLayerStyle->limitsLineWidth();

                        if(!cPenWidth)
                            cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                    }
                    else
                    {
                        cPenColor = Qt::red;
                        cPenWidth = 1;
                    }
                }
                // Request to show limits
                if(cPenWidth)
                    addMarker(m_pXYChart->yAxis(), lfLowL, cPenColor.rgb() & 0xffffff, m_strLowLimit, cPenWidth, Chart::TopLeft, false);
            }
        }

        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
        {
            lfHighL = ptTestCell->GetCurrentLimitItem()->lfHighLimit;		// High limit exists

            // If we have to keep values in normalized format, do not rescale!
            if (scaling!="normalized")
            {
                // convert HighLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfHighL,ptTestCell->hlm_scal);
                lfHighL *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
                lfHighL /=  ScalingPower(ptTestCell->res_scal);	// normalized
            }
            if(iCustomScaleFactor)
                lfHighL *= fCustomScaleFactor;

            // Update global highest limit for chart viewport
            internalChartInfo.m_dHighLimit = gex_max(internalChartInfo.m_dHighLimit, lfHighL);

            // High limit Marker
            if(qslAdvTrendMarkerOptionList.contains(QString("limits")))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = (pChart->bVisible) ? pChart->limitsLineWidth() : 0;
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart	*pLayerStyle = NULL;
                    if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
                        cPenColor	= pLayerStyle->limitsColor(bIsMultiLayer);
                        cPenWidth	= pLayerStyle->limitsLineWidth();

                        if(!cPenWidth)
                            cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                    }
                    else
                    {
                        cPenColor = Qt::red;
                        cPenWidth = 1;
                    }
                }
                // Request to show limits
                if(cPenWidth)
                    addMarker(m_pXYChart->yAxis(), lfHighL, cPenColor.rgb() & 0xffffff, m_strHighLimit, cPenWidth, Chart::BottomLeft, false);
            }
        }

        // Draw multi-limit markers.
        addMultiLimitMarkers(pChart, ptTestCell, internalChartInfo);

        // Draw spec limit markers (done only once: when charting Plot for group#1)
        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
        {
            lfLowSpecL = ptTestCell->lfLowSpecLimit;		// Low Spec limit exists

            // If we have to keep values in normalized format, do not rescale!
            //if(m_pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
            if (scaling!="normalized")
            {
                // convert LowLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfLowSpecL,ptTestCell->llm_scal);
                lfLowSpecL *=  ScalingPower(ptTestCell->llm_scal);	// normalized
                lfLowSpecL /=  ScalingPower(ptTestCell->res_scal);	// normalized
            }

            if(iCustomScaleFactor)
                lfLowSpecL *= fCustomScaleFactor;

            internalChartInfo.m_dLowSpecLimit = gex_min(internalChartInfo.m_dLowSpecLimit, lfLowSpecL);

            // LowLimit Marker
            if(qslAdvTrendMarkerOptionList.contains(QString("speclimits")))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = (pChart->bVisible) ? pChart->limitsLineWidth() : 0;
                    if(!cPenWidth)
                        cPenWidth = 1;
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart	*pLayerStyle = NULL;
                    if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
                        cPenColor	= pLayerStyle->limitsColor(bIsMultiLayer);
                        cPenWidth	= pLayerStyle->limitsLineWidth();

                        if(!cPenWidth)
                            cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                    }
                    else
                    {
                        cPenColor = Qt::red;
                        cPenWidth = 1;
                    }
                }
                // Request to show limits
                if(cPenWidth)
                    addMarker(m_pXYChart->yAxis(), lfLowSpecL, cPenColor.rgb() & 0xffffff, m_strLowSpecLimit, cPenWidth, Chart::TopLeft,false);

            }
        }

        if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
        {
            lfHighSpecL = ptTestCell->lfHighSpecLimit;		// High Spec limit exists

            // If we have to keep values in normalized format, do not rescale!
            //if(m_pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
            if (scaling!="normalized")
            {
                // convert HighLimit to same scale as results:
                pFile->FormatTestResultNoUnits(&lfHighSpecL,ptTestCell->hlm_scal);
                lfHighSpecL *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
                lfHighSpecL /=  ScalingPower(ptTestCell->res_scal);	// normalized
            }

            if(iCustomScaleFactor)
                lfHighSpecL *= fCustomScaleFactor;

            internalChartInfo.m_dHighSpecLimit = gex_max(internalChartInfo.m_dHighSpecLimit, lfHighSpecL);

            // High Spec limit Marker
            if(qslAdvTrendMarkerOptionList.contains(QString("speclimits")))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = (pChart->bVisible) ? pChart->limitsLineWidth() : 0;
                    if(!cPenWidth)
                        cPenWidth = 1;
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart	*pLayerStyle = NULL;
                    if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
                        cPenColor	= pLayerStyle->limitsColor(bIsMultiLayer);
                        cPenWidth	= pLayerStyle->limitsLineWidth();

                        if(!cPenWidth)
                            cPenWidth = 1;	// Just in case line width not defined under interactive chart, then ensure we still show line in image
                    }
                    else
                    {
                        cPenColor = Qt::red;
                        cPenWidth = 1;
                    }
                }
                // Request to show limits
                if(cPenWidth)
                    addMarker(m_pXYChart->yAxis(), lfHighSpecL, cPenColor.rgb() & 0xffffff, m_strHighSpecLimit, cPenWidth, Chart::BottomLeft, false);
            }
        }

        // Insert markers (horizontal lines): Mean, LowL, HighL
        // Scale Mean & Sigmato correct scale
        lfMean = fCustomScaleFactor*ptTestCell->lfMean;
        pFile->FormatTestResultNoUnits(&lfMean,ptTestCell->res_scal);

        lfSigma = fCustomScaleFactor*ptTestCell->lfSigma;
        pFile->FormatTestResultNoUnits(&lfSigma,ptTestCell->res_scal);

        if (ptTestCell->lfSamplesQuartile2 != -C_INFINITE)
        {
            lfMedian = fCustomScaleFactor * ptTestCell->lfSamplesQuartile2;
            pFile->FormatTestResultNoUnits(&lfMedian,ptTestCell->res_scal);
        }

        // If request to show the Mean
        if(qslAdvTrendMarkerOptionList.contains(QString("mean")))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->meanColor(bIsMultiLayer);
                cPenWidth = (pChart->bVisible) ? pChart->meanLineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
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
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Change label position for group 1...just in another group as exctly the same mean!
                if(iGroup == 1)
                    addMarker(m_pXYChart->yAxis(), lfMean, nPenColor, "Mean", cPenWidth, Chart::TopRight, false);
                else
                    addMarker(m_pXYChart->yAxis(), lfMean, nPenColor, "Mean", cPenWidth, Chart::BottomRight, false);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean);
            }
        }

        // If request to show the Median
        if((qslAdvTrendMarkerOptionList.contains(QString("median"))) && (lfMedian != -C_INFINITE))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->medianColor(bIsMultiLayer);
                cPenWidth = (pChart->bVisible) ? pChart->medianLineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
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
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Change label position for group 1...just in another group as exctly the same mean!
                if(iGroup == 1)
                    addMarker(m_pXYChart->yAxis(), lfMedian, nPenColor, "Median", cPenWidth, Chart::TopRight, false);
                else
                    addMarker(m_pXYChart->yAxis(), lfMedian, nPenColor, "Median", cPenWidth, Chart::BottomRight, false);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMedian);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMedian);
            }
        }

        // If request to show the Min
        if(m_pReportOptions->bTrendMarkerMin || qslAdvTrendMarkerOptionList.contains(QString("minimum")))
        {
            lfMin = fCustomScaleFactor*ptTestCell->lfSamplesMin;
            pFile->FormatTestResultNoUnits(&lfMin,ptTestCell->res_scal);
            if(pChart != NULL)
            {
                cPenColor = pChart->minColor(bIsMultiLayer);
                cPenWidth = (pChart->bVisible) ? pChart->minLineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
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
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Change label position for group 1...just in another group as exctly the same mean!
                if(iGroup == 1)
                    addMarker(m_pXYChart->yAxis(), lfMin, nPenColor, "Min.", cPenWidth, Chart::TopLeft, false);
                else
                    addMarker(m_pXYChart->yAxis(), lfMin, nPenColor, "Min.", cPenWidth, Chart::TopRight, false);
            }
        }

        // If request to show the Max
        if(m_pReportOptions->bTrendMarkerMax || qslAdvTrendMarkerOptionList.contains(QString("maximum")))
        {
            lfMax = fCustomScaleFactor*ptTestCell->lfSamplesMax;
            pFile->FormatTestResultNoUnits(&lfMax,ptTestCell->res_scal);
            if(pChart != NULL)
            {
                cPenColor = pChart->maxColor(bIsMultiLayer);
                cPenWidth = (pChart->bVisible) ? pChart->maxLineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
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
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Change label position for group 1...just in another group as exctly the same mean!
                if(iGroup == 1)
                    addMarker(m_pXYChart->yAxis(), lfMax, nPenColor, "Max.", cPenWidth, Chart::TopLeft, false);
                else
                    addMarker(m_pXYChart->yAxis(), lfMax, nPenColor, "Max.", cPenWidth, Chart::TopRight, false);
            }
        }
        if(qslAdvTrendMarkerOptionList.contains(QString("quartile_q1")))
        {
            lfQuartileQ1 = fCustomScaleFactor*ptTestCell->lfSamplesQuartile1;
            pFile->FormatTestResultNoUnits(&lfQuartileQ1,ptTestCell->res_scal);
            if(pChart != NULL)
            {
                cPenColor = pChart->quartileQ1Color(bIsMultiLayer);
                cPenWidth = (pChart->bVisible) ? pChart->quartileQ1LineWidth() : 0;
                if(!cPenWidth)
                    cPenWidth = 1;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
                    cPenColor	= pLayerStyle->quartileQ1Color(bIsMultiLayer);
                    cPenWidth	= pLayerStyle->quartileQ1LineWidth();

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
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Change label position for group 1...just in another group as exctly the same mean!
                if(iGroup == 1)
                    addMarker(m_pXYChart->yAxis(), lfQuartileQ1, nPenColor, "Q1", cPenWidth, Chart::TopLeft, false);
                else
                    addMarker(m_pXYChart->yAxis(), lfQuartileQ1, nPenColor, "Q1", cPenWidth, Chart::TopRight, false);
            }
        }

        if(qslAdvTrendMarkerOptionList.contains(QString("quartile_q3")))
        {
            lfQuartileQ3 = fCustomScaleFactor*ptTestCell->lfSamplesQuartile3;
            pFile->FormatTestResultNoUnits(&lfQuartileQ3,ptTestCell->res_scal);
            if(pChart != NULL)
            {
                cPenColor = pChart->quartileQ3Color(bIsMultiLayer);
                cPenWidth = (pChart->bVisible) ? pChart->quartileQ3LineWidth() : 0;
                if(!cPenWidth)
                    cPenWidth = 1;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
                    cPenColor	= pLayerStyle->quartileQ3Color(bIsMultiLayer);
                    cPenWidth	= pLayerStyle->quartileQ3LineWidth();

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
                int nPenColor = cPenColor.rgb() & 0xffffff;

                // Change label position for group 1...just in another group as exctly the same mean!
                if(iGroup == 1)
                    addMarker(m_pXYChart->yAxis(), lfQuartileQ3, nPenColor, "Q3", cPenWidth, Chart::TopLeft, false);
                else
                    addMarker(m_pXYChart->yAxis(), lfQuartileQ3, nPenColor, "Q3", cPenWidth, Chart::TopRight, false);
            }
        }
        // If request to show the 2sigma space
        if(qslAdvTrendMarkerOptionList.contains(QString("2sigma")))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->sigma2Color(bIsMultiLayer);
                cPenWidth = (pChart->bVisible) ? pChart->sigma2LineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
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

                addMarker(m_pXYChart->yAxis(), lfMean-lfSigma, nLineColor, "-1s", cPenWidth, Chart::BottomLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->yAxis(), lfMean+lfSigma, nLineColor, "+1s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean - lfSigma);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean + lfSigma);
            }
        }

        // If request to show the 3sigma space
        if(qslAdvTrendMarkerOptionList.contains(QString("3sigma")))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->sigma3Color(bIsMultiLayer);
                cPenWidth = (pChart->bVisible) ? pChart->sigma3LineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
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

                addMarker(m_pXYChart->yAxis(), lfMean-1.5*lfSigma, nLineColor, "-1.5s", cPenWidth, Chart::BottomLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->yAxis(), lfMean+1.5*lfSigma, nLineColor, "+1.5s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean - 1.5 * lfSigma);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean + 1.5 * lfSigma);
            }
        }

        // If request to show the 6sigma space
        if(qslAdvTrendMarkerOptionList.contains(QString("6sigma")))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->sigma6Color(bIsMultiLayer);
                cPenWidth = (pChart->bVisible) ? pChart->sigma6LineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
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

                addMarker(m_pXYChart->yAxis(), lfMean-3*lfSigma, nLineColor, "-3s", cPenWidth, Chart::BottomLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->yAxis(), lfMean+3*lfSigma, nLineColor, "+3s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean - 3 * lfSigma);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean + 3 * lfSigma);
            }
        }

        // If request to show the 12sigma space
        if(qslAdvTrendMarkerOptionList.contains(QString("12sigma")))
        {
            if(pChart != NULL)
            {
                cPenColor = pChart->sigma12Color(bIsMultiLayer);
                cPenWidth = (pChart->bVisible) ? pChart->sigma12LineWidth() : 0;
            }
            else
            {
                // Check if index is valid
                CGexSingleChart	*pLayerStyle = NULL;
                if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                {
                    pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
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

                addMarker(m_pXYChart->yAxis(), lfMean-6*lfSigma, nLineColor, "-6s", cPenWidth, Chart::BottomLeft, false)->setFontColor(nTextColor);
                addMarker(m_pXYChart->yAxis(), lfMean+6*lfSigma, nLineColor, "+6s", cPenWidth, Chart::TopLeft, false)->setFontColor(nTextColor);

                internalChartInfo.m_dMarkerMin = gex_min(internalChartInfo.m_dMarkerMin, lfMean - 6 * lfSigma);
                internalChartInfo.m_dMarkerMax = gex_max(internalChartInfo.m_dMarkerMax, lfMean + 6 * lfSigma);
            }
        }

        // Keep updatig title as total samples may increase as each layer doesn't necesseraly hold same number of samples.
        if((m_nSizeMode != GEX_CHARTSIZE_SMALL))
        {
            if(m_nSizeMode == GEX_CHARTSIZE_BANNER)
            {
                // Ue the X axis line to include statistivs instead of X legend!
                // Include: samples, mean, sigma, Cp, Cpk
                gexReport->BuildTestNameString(pFile, referenceTestX(), szTestName);
                strLabel.sprintf("Test %s: %s - Samples=%d Mean=%g Sigma=%g Cp=%s Cpk=", referenceTestX()->szTestLabel, szTestName, referenceTestX()->ldSamplesValidExecs, referenceTestX()->lfMean, referenceTestX()->lfSigma, gexReport->CreateResultStringCpCrCpk(referenceTestX()->GetCurrentLimitItem()->lfCp));
                strLabel += gexReport->CreateResultStringCpCrCpk(referenceTestX()->GetCurrentLimitItem()->lfCpk);
                strcpy(szString,strLabel.toLatin1().constData());
            }
            else
                sprintf(szString,"%s (%ld samples found)",szXscaleLabel, ldTotalValidSamples);

            // Check if we have a custom legend to overwrite default one
            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLegendX)
                m_pXYChart->xAxis()->setTitle(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strAxisLegendX.toLatin1().constData());
            else
                m_pXYChart->xAxis()->setTitle(szString);
        }

        // We have a chart legend unless the chart size in only 200 pixel wide (SMALL chart mode)!...done only once at 1st group processing
        if((m_nSizeMode != GEX_CHARTSIZE_SMALL) && (iGroup==1))
        {
            // compute a string <value> units...just to extract the units!
            if(m_nSizeMode == GEX_CHARTSIZE_BANNER)
            {
                QString strUnits = ptScaleTestCellReference->GetScaledUnits(&fCustomScaleFactor, scaling);

                if(strUnits.length() > 0)
                    strLabel += "(" + strUnits + ")";
                else
                    strLabel = "Results";

                strcpy(szString,strLabel.toLatin1().constData());
            }
            else
            {
                strLabel = "Test results ";
                QString strUnits = ptScaleTestCellReference->GetScaledUnits(&fCustomScaleFactor, scaling);

                if(strUnits.length() > 0)
                    strLabel += "(" + strUnits + ")";

                strcpy(szString,strLabel.toLatin1().constData());
            }

            if(m_pChartOverlays && (m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX || m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY))
                strcat(szString," - Log scale");

            // Check if we have a custom legend to overwrite default one
            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLegendY)
                m_pXYChart->yAxis()->setTitle(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strAxisLegendY.toLatin1().constData());
            else
                m_pXYChart->yAxis()->setTitle(szString);
        }

        // If layer visible: Write Custom markers defined thru the scripting interface (if a marker is sticky to a layer, it
        if(bVisible)
        {
            if(m_pChartOverlays)
            {
                CGexSingleChart *pChartLayer = m_pChartOverlays->chartsList().at(iLayerIndex-1);

                if(pChartLayer && pChartLayer->bVisible)
                    plotScriptingCustomMarkers(ptTestCell, pChartLayer->cColor.rgb() & 0xffffff, iLayerIndex, fCustomScaleFactor, false, internalChartInfo.m_dMarkerMin, internalChartInfo.m_dMarkerMax);
            }
            else
                plotScriptingCustomMarkers(ptTestCell, cLayerColor.rgb() & 0xffffff, iGroup, fCustomScaleFactor, false, internalChartInfo.m_dMarkerMin, internalChartInfo.m_dMarkerMax);
        }

        // Check if charting window is test samples or test limits.
        switch(viewportMode())
        {
            case viewportOverLimits : // Chart over test limits
            default:
                if(internalChartInfo.m_dLowLimit < C_INFINITE)
                    lfChartBottom = gex_min(internalChartInfo.m_dLowLimit,lfChartBottom);	// Some low limit exists
                else
                {
                    if(internalChartInfo.m_dHighLimit > -C_INFINITE)
                        lfChartBottom = gex_min(internalChartInfo.m_dHighLimit, lfChartBottom);

                    lfChartBottom = gex_min(lfChartBottom, lfDataStart);
                }

                if(internalChartInfo.m_dHighLimit > -C_INFINITE)
                    lfChartTop = gex_max(internalChartInfo.m_dHighLimit,lfChartTop);		// Some high limit exists
                else
                {
                    if(internalChartInfo.m_dLowLimit < C_INFINITE)
                        lfChartTop = gex_max(internalChartInfo.m_dLowLimit, lfChartTop);

                    lfChartTop = gex_max(lfChartTop, lfDataEnd);
                }

                break;

            case viewportOverData : // Chart over data samples!
                lfChartBottom = gex_min(lfChartBottom,lfDataStart);
                lfChartTop = gex_max(lfChartTop,lfDataEnd);
                break;

            case viewportAdaptive : // Adaptive: Chart must show limits & data samples!
                // Update Min/Max of samples
                lfChartBottom	= gex_min(lfChartBottom,	lfDataStart);
                lfChartTop		= gex_max(lfChartTop,		lfDataEnd);

                // update Min/Max over limits
                if(internalChartInfo.m_dLowLimit < C_INFINITE)
                    lfChartBottom = gex_min(internalChartInfo.m_dLowLimit,lfChartBottom);		// Some low limit exists
                else if(internalChartInfo.m_dHighLimit > -C_INFINITE)
                    lfChartBottom = gex_min(internalChartInfo.m_dHighLimit, lfChartBottom);

                if(internalChartInfo.m_dHighLimit > -C_INFINITE)
                    lfChartTop = gex_max(internalChartInfo.m_dHighLimit, lfChartTop);			// Some high limit exists
                else if(internalChartInfo.m_dLowLimit < C_INFINITE)
                    lfChartTop = gex_max(internalChartInfo.m_dLowLimit, lfChartTop);

                if (internalChartInfo.m_dMarkerMin != C_INFINITE)
                    lfChartBottom = gex_min(lfChartBottom, internalChartInfo.m_dMarkerMin);

                if (internalChartInfo.m_dMarkerMax != -C_INFINITE)
                    lfChartTop = gex_max(lfChartTop, internalChartInfo.m_dMarkerMax);

                break;
        }

        // Determine min and max values for the viewport
        if(internalChartInfo.m_dLowLimit < C_INFINITE)
            lfViewportLowY = gex_min(lfViewportLowY, internalChartInfo.m_dLowLimit);
        else if(internalChartInfo.m_dHighLimit > -C_INFINITE)
            lfViewportLowY = gex_min(internalChartInfo.m_dHighLimit, lfViewportLowY);

        lfViewportLowY = gex_min(lfViewportLowY, lfDataStart);

        if(internalChartInfo.m_dHighLimit > -C_INFINITE)
            lfViewportHighY = gex_max(lfViewportHighY, internalChartInfo.m_dHighLimit);
        else if(internalChartInfo.m_dLowLimit < C_INFINITE)
            lfViewportHighY = gex_max(internalChartInfo.m_dLowLimit, lfViewportHighY);

        lfViewportHighY = gex_max(lfViewportHighY, lfDataEnd);

        if (internalChartInfo.m_dMarkerMin != C_INFINITE)
            lfViewportLowY = gex_min(lfViewportLowY, internalChartInfo.m_dMarkerMin);

        if (internalChartInfo.m_dMarkerMax != -C_INFINITE)
            lfViewportHighY = gex_max(lfViewportHighY, internalChartInfo.m_dMarkerMax);

        if (internalChartInfo.m_dLowSpecLimit != C_INFINITE
                && internalChartInfo.m_dLowSpecLimit > -F_INFINITE
                && internalChartInfo.m_dLowSpecLimit != -C_INFINITE
                && internalChartInfo.m_dLowSpecLimit < F_INFINITE)
            lfViewportLowY = gex_min(lfViewportLowY, internalChartInfo.m_dLowSpecLimit);

        if (internalChartInfo.m_dHighSpecLimit != C_INFINITE
                && internalChartInfo.m_dHighSpecLimit > -F_INFINITE
                &&internalChartInfo.m_dHighSpecLimit != -C_INFINITE
                && internalChartInfo.m_dHighSpecLimit < F_INFINITE)
            lfViewportHighY = gex_max(lfViewportHighY, internalChartInfo.m_dHighSpecLimit);

        lfViewportLowY = gex_min(lfViewportLowY, lfMedian);
        lfViewportLowY = gex_min(lfViewportLowY, lfMean);
        lfViewportLowY = gex_min(lfViewportLowY, lfMean - 6 * lfSigma);

        lfViewportHighY = gex_max(lfViewportHighY, lfMedian);
        lfViewportHighY = gex_max(lfViewportHighY, lfMean);
        lfViewportHighY = gex_max(lfViewportHighY, lfMean + 6 * lfSigma);

        NextGroup:
        if(m_pChartOverlays == NULL)
        {
            ++itGroupList;
            pGroup = (itGroupList != gexReport->getGroupsList().end()) ? (*itGroupList) : NULL;

            iGroup++;	// Keep track of group index processed.
        }

    };	// Read trend of a given test in all groups so to stack all charts.

    dDiff = lfChartBottom + OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        lfChartBottom = lfDataStart;
    }
    dDiff = lfChartTop - OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        lfChartTop = lfDataEnd;
    }

    // Building Trend chart image for report (not interactive). Make sure we display all samples on the X axis (all layers)
    lEndID = lfTotalSamplesInX;

    // If custom viewport (in scripting file, not in interactive mode), overwrite defaults window viewing
    if(useTestCustomViewport())
    {
        // reset the current viewport
        resetViewportManager(false);

        // If custom viewport in X, overwrite defaults window viewing
        if(referenceTestX()->ptChartOptions->customViewportX())
        {
            lfExtra			= 0;
            lfChartBottom	= referenceTestX()->ptChartOptions->lowX();
            lfChartTop		= referenceTestX()->ptChartOptions->highX();
        }

        // If custom viewport in Y, overwrite defaults window viewing
        if(referenceTestX()->ptChartOptions->customViewportY())
        {
            lStartID	= referenceTestX()->ptChartOptions->lowY();
            lEndID		= referenceTestX()->ptChartOptions->highY();
        }
    }
    else
    {
        // Customer force viewport limit
        if (m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].bForceViewport)
        {
            resetViewportManager(false);

            if (m_pChartOverlays->getViewportRectangle()[type()].lfLowX != -C_INFINITE)
                lStartID		= m_pChartOverlays->getViewportRectangle()[type()].lfLowX;

            if (m_pChartOverlays->getViewportRectangle()[type()].lfHighX != C_INFINITE)
                lEndID			= m_pChartOverlays->getViewportRectangle()[type()].lfHighX;

            if (m_pChartOverlays->getViewportRectangle()[type()].lfLowY != -C_INFINITE)
                lfChartBottom	= m_pChartOverlays->getViewportRectangle()[type()].lfLowY;

            if (m_pChartOverlays->getViewportRectangle()[type()].lfHighY != C_INFINITE)
                lfChartTop		= m_pChartOverlays->getViewportRectangle()[type()].lfHighY;

            lfExtra			= 0;

            if( m_pChartOverlays->getViewportRectangle()[type()].bForceViewport)
                m_pChartOverlays->getViewportRectangle()[type()].mChangeViewPort = true;

            m_pChartOverlays->getViewportRectangle()[type()].bForceViewport = false;
        }
        else
        {
            // Y axis scale. Add 10% of scale over and under chart window...
            // to ensure markers will be seen
            lfExtra		= (lfChartTop - lfChartBottom) * 0.10;
            lStartID	-= 1;

            // X axis scale.
            if (m_pReportOptions->GetOption("adv_trend","x_axis").toString()=="part_id")	//iTrendXAxis == GEX_TREND_XAXIS_PARTID)
                // X axis: PartID. Start at offset N
                lEndID += 0.5;	// Ensure last PartID is visible!
            else
                // X axis: Sample#. Start at offset N-1
                lEndID -= 0.5;
        }
    }

    lfSwap			= lfChartBottom;
    lfChartBottom	= gex_min(lfChartBottom,lfChartTop);
    lfChartTop		= gex_max(lfSwap,lfChartTop);

    lfViewportLowY	= gex_min(lfViewportLowY, lfChartBottom);
    lfViewportHighY	= gex_max(lfViewportHighY, lfChartTop);

    dDiff = lfViewportLowY + OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        lfViewportLowY = lfDataStart;
    }
    dDiff = lfViewportHighY - OLD_CHART_LIMIT_INFINITE;
    if (dDiff == 0.0) {
        lfViewportHighY = lfDataEnd;
    }

    // Add extra space
    if(m_pChartOverlays == NULL || (m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX == false && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY == false))
    {
        if (lfChartBottom != -C_INFINITE && lfChartTop != C_INFINITE)
        {
            lfChartTop		+= lfExtra;
            lfChartBottom	-= lfExtra;
        }
    }

    // Define viewport: if in Interactive charting, consider zoomin factor
    if (m_pChartOverlays && lfViewportLowY != C_INFINITE && lfViewportHighY != -C_INFINITE)
    {
        // Initialize viewport
        if (isViewportInitialized() == false)
        {
            initHorizontalViewport(lStartID, lEndID, lStartID, lEndID);
            initVerticalViewport(lfViewportLowY, lfViewportHighY, lfChartBottom, lfChartTop, 0.1);
        }

        // Using ViewPortLeft and ViewPortWidth, get the start and end dates of the view port.
        computeXBounds(lStartID, lEndID);
        computeYBounds(lfChartBottom, lfChartTop);
    }

    if (lfChartBottom != -C_INFINITE && lfChartTop != C_INFINITE)
    {
        // Check if scale type (in X) is: Linear or Logarithmic
        // Log scale: then force it
        if((m_pChartOverlays && (m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX || m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY)
            &&(m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeTrend || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 1){
            m_pXYChart->yAxis()->setLogScale(lfChartBottom,lfChartTop);
            m_iYScaleType = 1;
        } else if((m_pChartOverlays && (!m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX && !m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY)
               &&(m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeTrend || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 0) {
            m_pXYChart->yAxis()->setLinearScale(lfChartBottom, lfChartTop);
            m_iYScaleType = 0;
        }
    }

    // Set the labels on the x axis. Rotate the font by 90 degrees.
    if ( (m_pReportOptions->GetOption("adv_trend","x_axis").toString()=="part_id")	//iTrendXAxis == GEX_TREND_XAXIS_PARTID
        && (m_bNumericalPartID == false)
        )
    {
        // One tick visible every 40 pixels
        long lTick		= (width() - horizontalMargin()) / 40;
        long lPartCount = (long) (lEndID - lStartID);
        long lPartStep	= (lPartCount / lTick) + 1;

        // Set the linear scale for x axis without displaying values
        m_pXYChart->xAxis()->setLinearScale(lStartID, lEndID, Chart::NoValue);

        TextBox *	pTextBox = m_pXYChart->xAxis()->setLabelStyle("", 8, TextColor, 45.0);

        // reset value
        m_nMaxTextHeight	= 40;
        m_nMaxTextWidth		= 65;

        for (int nLabel = 0; nLabel < m_lstPartID.count(); nLabel += lPartStep)
        {
            if (pTextBox)
            {
                int nTextWidth;
                int nTextHeight;

                // With version 5.X
                if (Chart::getVersion() == 5)
                {
                    pTextBox->setText(m_lstPartID.at(nLabel).toLatin1().constData());

                    nTextWidth	= pTextBox->getWidth() + 10;
                    nTextHeight	= pTextBox->getHeight() + 10;
                }
                // with older version (4.X)
                else
                {
                    nTextWidth	= (int) (m_lstPartID.at(nLabel).size() * 4.4 + 10);
                    nTextHeight = (int) (m_lstPartID.at(nLabel).size() * 4.4 + 10);
                }

            //	nTextWidth			-= (internalChartInfo.m_nVisibleLayerIndex + 1) * ((1.0 / (m_boxPlotDataSet.size() * 2)) * (width() - horizontalMargin()));
                m_nMaxTextHeight	= gex_max(m_nMaxTextHeight, nTextHeight);
                m_nMaxTextWidth		= gex_max(m_nMaxTextWidth, nTextWidth);
            }

            m_pXYChart->xAxis()->addLabel(nLabel+1, m_lstPartID.at(nLabel).toLatin1().constData());
        }

        // Add 20 pixels for the axis title
        setBottomMargin(m_nMaxTextHeight + 20);
        setLeftMargin(m_nMaxTextWidth);
    }
    else
    {
        m_pXYChart->xAxis()->setLinearScale(lStartID, lEndID);

        // Make sure X axis only reports integer divisions (Run#)
        if(lEndID-lStartID < 10)
            m_pXYChart->xAxis()->setTickOffset(1.0);
    }

end_trend_chart:
    //if (m_pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if (m_pReportOptions->isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // Plot vertical Lots# markers if any.
        plotLotMarkers(ptTestCell);
    }

    if (m_pReportOptions->bPlotLegend)
        m_pXYChart->addLegend(75, 30)->setBackground(0x80000000 | (QColor(Qt::white).rgb() & 0xffffff));

    if (m_pChartOverlays)
    {
        m_pChartOverlays->getViewportRectangle()[type()].lfLowX	= lStartID;
        m_pChartOverlays->getViewportRectangle()[type()].lfLowY	= lfChartBottom;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighX	= lEndID;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighY	= lfChartTop;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Add plot markers for each lot (if multi files merge)
/////////////////////////////////////////////////////////////////////////////
void GexTrendChart::plotLotMarkers(CTest * ptTestCell, bool bAggregateData /*= false*/)
{
    if (ptTestCell == NULL)
        return;

    if((ptTestCell->ptChartOptions != NULL) && (ptTestCell->ptChartOptions->lotMarker() == false))
        return;	// Do not show Lots markers.

    CGexGroupOfFiles *	pGroup;
    CGexFileInGroup *	pFile;
    QString				strCurrentLot;
    double				lfPosition,lfPreviousPosition=0;
    char				szString[200];
    long				lDataOffset		= 1;
    int					iMarkerIndex	= 0;
    int					nMarkerColor	= QColor(Qt::blue).rgb() & 0xffffff;
    Mark *				pPreviousMarker = NULL;

    // OPTIONS
    QString strAdvTrendMarkerOptions = (m_pReportOptions->GetOption(QString("adv_trend"), QString("marker"))).toString();
    QStringList qslAdvTrendMarkerOptionList = strAdvTrendMarkerOptions.split(QString("|"));

    // If a single group with multiple files, show vertical delimiters for each file (lot)
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();

    //if(m_pReportOptions->iGroups == 1)
    if( (m_pReportOptions->iGroups == 1) &&
            (m_pReportOptions->GetOption(QString("adv_trend"),
                                         QString("x_axis")).toString()==
             QString("run_id")) )
    {
        QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

        while(itFilesList.hasNext())
        {
            pFile = itFilesList.next();

            if(bAggregateData)
                lfPosition = iMarkerIndex+0.5;
            else
            {
                // If marker defines a samples count#, then show marker between last data of previous lot and next data of new lot
                lfPosition = lDataOffset;
                lfPosition -= 0.5;
            }

            // Show verticale marker at position X (only one per Lot)
            if( (qslAdvTrendMarkerOptionList.contains(QString("lot"))) && (strCurrentLot != pFile->getMirDatas().szLot))
            {
                // Write lotID with a 90° angle
                strCurrentLot = pFile->getMirDatas().szLot;
                sprintf(szString,"Lot: %s",pFile->getMirDatas().szLot);
                // Aggregate data: only one point per sub-lot
                addMarker(m_pXYChart->xAxis(), lfPosition, nMarkerColor, szString, 1, Chart::TopRight, false);

                // If previous marker inserted is at same position as new one, then remove previous maker!
                if(lfPreviousPosition == lfPosition && pPreviousMarker)
                    pPreviousMarker->setMarkColor(0xFFFFFFFF); // Transparent, marker won't appear

                // Update marker status
                lfPreviousPosition = lfPosition;
            }

            // If display marker at each sub-lot (file)
            if(qslAdvTrendMarkerOptionList.contains(QString("sublot")))
            {
                // If valid sublot & wafer ID, or Only WaferID: show WaferID
                if((*pFile->getMirDatas().szSubLot && *pFile->getWaferMapData().szWaferID) || *pFile->getWaferMapData().szWaferID)
                {
                    if(pGroup->pFilesList.count() <= 10)
                        sprintf(szString,"Waf:%s",pFile->getWaferMapData().szWaferID);
                    else
                    if(pGroup->pFilesList.count() <= 26)
                        sprintf(szString,"W:%s",pFile->getWaferMapData().szWaferID);
                    else
                        sprintf(szString,"%s",pFile->getWaferMapData().szWaferID);	// Shaortest strinng possible!
                }
                else
                if(*pFile->getMirDatas().szSubLot)
                    sprintf(szString,"SubLot: %s",pFile->getMirDatas().szSubLot);

                // Write sub-lots with a -20° angle...
                Mark * pMarker = addMarker(m_pXYChart->xAxis(), lfPosition, nMarkerColor, szString, 1, Chart::BottomRight, false);
                pMarker->setFontAngle(20);
            }

            // If display marker for each file (dataset name)
            // if(m_pReportOptions->bTrendMarkerGroupName && (pFile->strDatasetName.isEmpty() == false))
            if( (qslAdvTrendMarkerOptionList.contains(QString("group_name"))) && (pFile->strDatasetName.isEmpty() == false))
                addMarker(m_pXYChart->xAxis(), lfPosition, nMarkerColor, pFile->strDatasetName, 1, Chart::BottomRight, false);

            // Compute offset position to next data array (next file in group)...
            if(ptTestCell!= NULL)
            {
                // Find test cell: RESET list to ensure we scan list of the right group!
                pFile->FindTestCell(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex,&ptTestCell,FALSE,FALSE, ptTestCell->strTestName);
                if(ptTestCell->ldSamplesValidExecs > 0)
                    lDataOffset += ptTestCell->pSamplesInSublots[iMarkerIndex];
            }
            else
                lDataOffset = pFile->getMirDatas().lFirstPartResult;	// Using PartID as the offset.

            // Keep track of marker# plotted
            iMarkerIndex++;
        };
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void computeCursorValue(const QPoint& pointCursor)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexTrendChart::computeCursorValue(const QPoint& pointCursor)
{
    GexAbstractChart::computeCursorValue(pointCursor);

    // Compute the x and y values under the cursor
    if (inPlotArea(pointCursor.x(), pointCursor.y()))
    {
        adjustXCursorValue();

        m_bValidDie = computePointerDie();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void adjustXCursorValue()
//
// Description	:	Round xCursor value to the right run number
//
///////////////////////////////////////////////////////////////////////////////////
void GexTrendChart::adjustXCursorValue()
{
    if (m_dXCursorValue != GEX_C_DOUBLE_NAN)
        m_dXCursorValue  = qRound(m_dXCursorValue);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool computePointerDie()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexTrendChart::computePointerDie()
{
    // Find LotId & SubLotID
    CTest *				ptDieCell;
    CGexSingleChart	*	pChart = NULL;	// Handle to Parameter Layer info.
    CTest			*	ptTest;
    int					iMarkerIndex;
    int					iMarkerOffset = 0;
    double				lfValue;
    long				lRunNumber;

    // use hotspot value if valid
    if (m_dXHotSpotValue != GEX_C_DOUBLE_NAN)
        lRunNumber = (long) m_dXHotSpotValue;
    else
        lRunNumber = (long) m_dXCursorValue;

    m_lMarkerGroupID	= 0;
    m_pMarkerGroup		= NULL;
    m_pMarkerFile		= NULL;

    for (int nGroup = 0; nGroup < gexReport->getGroupsList().count(); nGroup++)
    {
        m_pMarkerGroup	= gexReport->getGroupsList().at(nGroup);

        if (m_pMarkerGroup->strGroupName == m_strGroupName)
            break;

        m_lMarkerGroupID++;
    }

    if (m_pMarkerGroup && m_pChartOverlays->chartsList().isEmpty() == false)
    {
        m_nXDie			= 0;
        m_nYDie			= 0;

        for(iMarkerIndex = 0; iMarkerIndex < referenceTestX()->pSamplesInSublots.count();iMarkerIndex++)
        {
            // Compute sample 'ending' offset
            iMarkerOffset += referenceTestX()->pSamplesInSublots[iMarkerIndex];

            // Check if this file has its PartID below the pixel pointed...
            if((int) m_dXCursorValue < iMarkerOffset)
                break;	// Found which lotID we hoover
        }

        // Check for over-run
        if(iMarkerIndex >= referenceTestX()->pSamplesInSublots.count())
            iMarkerIndex--;

        if ((iMarkerIndex >= m_pMarkerGroup->pFilesList.size()) || (iMarkerIndex < 0))
            return false;

        // Point relevant File (as we've got 1 file per sublot)
        m_pMarkerFile = m_pMarkerGroup->pFilesList.at(iMarkerIndex);

        // Get which result occurance is pointed
        pChart = m_pChartOverlays->chartsList().first();

        if(!m_pMarkerFile || m_pMarkerFile->FindTestCell(pChart->iTestNumberX, pChart->iPinMapX, &ptTest, true, false) != 1)
            return false;

        // Report Die coordinates
        // DieX parameter...
        if(m_pMarkerFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &ptDieCell, true, false) != 1)
            return false;

        if(ptDieCell->FindValidResult((long) lRunNumber - 1, lfValue) < 0)
            return false;

        m_nXDie = (int) lfValue;

        // DieY parameter...
        if(m_pMarkerFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &ptDieCell, true, false) != 1)
            return false;

        if(ptDieCell->FindValidResult((long) lRunNumber - 1, lfValue) < 0)
            return false;

        m_nYDie = (int) lfValue;

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
QString GexTrendChart::makeTooltip()
{
    QString	strTooltip;
    CGexFileInGroup cFile(NULL,0,"",0,0,"","","");	// in order to use FormatTestResultNoUnits(...)
    cFile.UpdateOptions(&ReportOptions);

    if (referenceTestX() && isViewportInitialized())
    {
        // Compute X and Y positions;
        double				lfCustomScaleFactor;
        QString				strUnitsX;
        QString				strValue;

        if (m_pMarkerGroup && m_pMarkerFile)
        {
            // Get X and Y units (if any)
            strUnitsX = referenceTestX()->GetScaledUnits(&lfCustomScaleFactor,
                        ReportOptions.GetOption("dataprocessing","scaling").toString());
            strUnitsX.truncate(10);

            if (m_dXHotSpotValue != GEX_C_DOUBLE_NAN || m_dYHotSpotValue != GEX_C_DOUBLE_NAN)
                strValue.sprintf("X: %d\nY: %g %s",(int)m_dXHotSpotValue, m_dYHotSpotValue, strUnitsX.toLatin1().constData());

            // If group name exists, add it in first position
            if (m_strGroupName.isEmpty() == false)
            {
                strTooltip = "Group: " + m_strGroupName;

                if (strValue.isEmpty() == false)
                    strTooltip +=	"\n" + strValue;
            }
            else
                strTooltip = strValue;

            if (m_bShowPartID && m_dXHotSpotValue != GEX_C_DOUBLE_NAN)
            {
                long lXCursor = (long)m_dXHotSpotValue-1;

                if (m_pReportOptions->GetOption("adv_trend","x_axis").toString()=="part_id")	//iTrendXAxis == GEX_TREND_XAXIS_PARTID)
                {
                    if (m_bNumericalPartID == true)
                    {
                        strTooltip += "\nPartID : ";
                        strTooltip += QString::number((long) m_dXHotSpotValue);
                    }
                    else if (m_lstPartID.isEmpty() == false && lXCursor >= 0 && lXCursor < m_lstPartID.count())
                    {
                        strTooltip += "\nPartID : ";
                        strTooltip += m_lstPartID.at(lXCursor);
                    }
                }
                else if (m_pMarkerFile && lXCursor >= 0 && lXCursor < m_pMarkerFile->pPartInfoList.count())
                {
                    strTooltip += "\nPartID : ";
                    strTooltip += m_pMarkerFile->pPartInfoList.at(lXCursor)->getPartID();
                }
            }


            // If log scale, do not show the data pointed...don't know its location!
            if(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX || m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY)
                strTooltip = "Plot Area\n";

            if(*m_pMarkerFile->getMirDatas().szLot)
                strTooltip += QString("\nLot: ") + QString(m_pMarkerFile->getMirDatas().szLot);

            if(*m_pMarkerFile->getMirDatas().szSubLot)
                strTooltip += QString("\nSublot: ") + QString(m_pMarkerFile->getMirDatas().szSubLot);

            if(m_pMarkerFile->getWaferMapData().bWaferMapExists)
            {
                // Wafer ID
                if(m_pMarkerFile->getWaferMapData().szWaferID[0])
                    strTooltip += QString("\nWaferID: ") + QString(m_pMarkerFile->getWaferMapData().szWaferID);

                // Display DieX,Y info
                if(m_dXHotSpotValue != GEX_C_DOUBLE_NAN && isValidDie())
                    strTooltip += QString("\nDieXY: (") + QString::number(m_nXDie) + "," + QString::number(m_nYDie) + ")";
            }
        }
    }

    return strTooltip;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setViewportModeFromChartMode(int nChartMode)
//
// Description	:	convert trend chart mode from Gex into viewport mode
//
///////////////////////////////////////////////////////////////////////////////////
void	GexTrendChart::setViewportModeFromChartMode(int nChartMode)
{
    int nViewportMode;

    switch (nChartMode)
    {
        case GEX_ADV_TREND_OVERLIMITS	:	nViewportMode = viewportOverLimits;
                                            break;

        case GEX_ADV_TREND_OVERDATA		:	nViewportMode = viewportOverData;
                                            break;

        case GEX_ADV_TREND_DATALIMITS	:	nViewportMode = viewportAdaptive;
                                            break;

        default							:	nViewportMode = viewportOverLimits;
    }

    setViewportMode(nViewportMode);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void chartingLineStyle(CGexSingleChart * pLayerStyle)
//
// Description	:	Update some characteristics
//
///////////////////////////////////////////////////////////////////////////////////
void	GexTrendChart::chartingLineStyle(CGexSingleChart * pLayerStyle)
{
    if (pLayerStyle)
    {
        QString ct=m_pReportOptions->GetOption("adv_trend","chart_type").toString();
        // Lines?
        /*
        if((m_pReportOptions->iTrendChartType == GEX_CHARTTYPE_LINES) ||
            (m_pReportOptions->iTrendChartType == GEX_CHARTTYPE_LINESSPOTS))
            pLayerStyle->iLineStyle = 0;	// Solid Line
        else
            pLayerStyle->iLineStyle = -1;	// No line.
        */
        if (	( ct == "lines") || (ct == "lines_spots") )
            pLayerStyle->iLineStyle = 0;	// Solid Line
        else
            pLayerStyle->iLineStyle = -1;	// No line.

        // Spots?
        if ( ct=="spots" || ct=="lines_spots" ) //if((m_pReportOptions->iTrendChartType == GEX_CHARTTYPE_SPOTS) || (m_pReportOptions->iTrendChartType == GEX_CHARTTYPE_LINESSPOTS))
            pLayerStyle->bSpots = true;
        else
            pLayerStyle->bSpots = false;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void plotSpecificScriptingCustomMarker(TestMarker * pTestMarker, int nColor)
//
// Description	:	Plot specific data for scripting custom markers
//
///////////////////////////////////////////////////////////////////////////////////
void GexTrendChart::plotSpecificScriptingCustomMarker(TestMarker * pTestMarker, int nColor,
                                                      CTest *ptTestCell, double dCustomScaleFactor)
{
    if (pTestMarker->lRunID != -1)
    {
        long dRun = pTestMarker->lRunID;
        if(m_pReportOptions->GetOption("adv_trend","x_axis").toString() == "part_id")
        {
            CGexGroupOfFiles *pGroup = (gexReport->getGroupsList().isEmpty()) ? NULL : gexReport->getGroupsList().at(pTestMarker->iLayer);
            QList<GexTrendPartInfo> lstPartInfo;
            fillPartInfoList(pGroup, &lstPartInfo);

            for (int nPartInfo = 0; nPartInfo < lstPartInfo.count(); nPartInfo++)
            {
                if(ptTestCell->m_testResult.isValidResultAt(lstPartInfo.at(nPartInfo).runID()))
                {
                    if(dRun == lstPartInfo.at(nPartInfo).runID()+1){
                        dRun =   lstPartInfo.at(nPartInfo).posID();
                        break;
                    }
                }
            }
        }

        pTestMarker->lRunID = dRun;

        // Value is always normalized
        double dValue	= pTestMarker->lfPos * dCustomScaleFactor;

        if (ReportOptions.GetOption("dataprocessing","scaling").toString() != "normalized")
        {
            double* ptlfValue=&dValue;
            int nResScale=ptTestCell->res_scal;
            switch(nResScale)
            {
                default: *ptlfValue *= GS_POW(10.0,nResScale); return;
                case 0: break;
                case 254:
                case -2:	// '%'
                    *ptlfValue *=1e-2; break;
                case 253:	// for unsigned -3
                case -3: *ptlfValue *=1e-3; break;
                case 250:	// for unsigned -6
                case -6: *ptlfValue *=1e-6; break;
                case 247:	// for unsigned -9
                case -9: *ptlfValue *=1e-9; break;
                case 244:	// for unsigned -13
                case -12: *ptlfValue *=1e-12; break;
                case 2:	// '%'
                    *ptlfValue *=1e2; break;
                case 3: *ptlfValue *=1e3; break;
                case 6: *ptlfValue *=1e6; break;
                case 9: *ptlfValue *=1e9; break;
                case 12: *ptlfValue *=1e12; break;
                case 15: *ptlfValue *=1e15; break;
            }
        }

        double double_run = dRun;
        m_pXYChart->addScatterLayer(DoubleArray(&double_run, 1),
                                    DoubleArray(&dValue, 1), "",
                                    Chart::CircleSymbol, 6, nColor);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void fillPartInfoReferenceMap()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
struct GexTrendPartInfoComparePointers
{
    bool operator()(GexTrendPartInfo* a, GexTrendPartInfo* b)
    {
        return *a < *b;
    }
} GexTrendPartInfoComparePointers;

void GexTrendChart::fillPartInfoReferenceMap()
{
    bool bNumericalOk;

    // Clear the oldest list
    m_mapPosID.clear();
    m_lstPartID.clear();

    if (m_pReportOptions->GetOption("adv_trend","x_axis").toString()=="part_id")	//iTrendXAxis == GEX_TREND_XAXIS_PARTID)
    {
        CGexGroupOfFiles *		pGroup = NULL;
        QList<GexTrendPartInfo*> lstTrendPartInfo;
        QHash<QString, GexTrendPartInfo*> lTrendPartInfoHash;

        for(int nGroup = 0; nGroup < gexReport->getGroupsList().count(); nGroup++)
        {
            pGroup = gexReport->getGroupsList().at(nGroup);

            CGexFileInGroup *		pFile		= NULL;
            CPartInfo *				pPartInfo	= NULL;
            int						nPartInfo   = 0;

            QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

            while (itFilesList.hasNext())
            {
                pFile	= itFilesList.next();

                for (int nIndex = 0; nIndex < pFile->pPartInfoList.count(); nIndex++)
                {
                    pPartInfo = pFile->pPartInfoList.at(nIndex);

                   if (lTrendPartInfoHash.contains(pPartInfo->getPartID()) == false)
                   {
                       GexTrendPartInfo* trendPartInfo = new GexTrendPartInfo(pPartInfo->getPartID(), nPartInfo++);
                       lTrendPartInfoHash.insert(trendPartInfo->partID(), trendPartInfo);
                   }

                    // Check if part id is a numerical value
                    pPartInfo->getPartID().toLong(&bNumericalOk);

                    m_bNumericalPartID &= bNumericalOk;
                }
            }
        }

        lstTrendPartInfo = lTrendPartInfoHash.values();
        // Sort the part info list
        std::sort(lstTrendPartInfo.begin(),
                  lstTrendPartInfo.end(), GexTrendPartInfoComparePointers);

        // Set the pos ID for each part ID
        for (int nIndex = 0; nIndex < lstTrendPartInfo.count(); nIndex++)
        {
            if (m_bNumericalPartID)
                m_mapPosID.insert(lstTrendPartInfo.at(nIndex)->partID(), lstTrendPartInfo.at(nIndex)->partID().toLong());
            else
                m_mapPosID.insert(lstTrendPartInfo.at(nIndex)->partID(), nIndex+1);

            m_lstPartID.append(lstTrendPartInfo.at(nIndex)->partID());
        }
        // free
        while (!lstTrendPartInfo.isEmpty())
            delete lstTrendPartInfo.takeFirst();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void fillPartInfoList(CGexGroupOfFiles * pGroup, QList<GexTrendPartInfo> * pLstPartInfo)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexTrendChart::fillPartInfoList(CGexGroupOfFiles * pGroup, QList<GexTrendPartInfo> * pLstPartInfo)
{
    if (pGroup && pLstPartInfo)
    {
        if (m_pReportOptions->GetOption("adv_trend","x_axis").toString()=="part_id")	//iTrendXAxis == GEX_TREND_XAXIS_PARTID)
        {
            QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);
            CGexFileInGroup *	pFile		= NULL;
            CPartInfo *			pPartInfo	= NULL;
            int					nPartInfo   = 0;

            // Clear the list
            pLstPartInfo->clear();

            while (itFilesList.hasNext())
            {
                pFile = itFilesList.next();

                for (int nIndex = 0; nIndex < pFile->pPartInfoList.count(); nIndex++)
                {
                    pPartInfo = pFile->pPartInfoList.at(nIndex);

                    GexTrendPartInfo trendPartInfo(pPartInfo->getPartID(), nPartInfo++);

                    if (m_mapPosID.contains(pPartInfo->getPartID()))
                    {
                        trendPartInfo.setPosID(m_mapPosID.value(pPartInfo->getPartID()));

                        pLstPartInfo->append(trendPartInfo);
                    }
                }
            }

            // Sort the part info list
            qSort(*pLstPartInfo);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void keepXTestViewport()
//
// Description	:	Keep wiewport for test X
//
///////////////////////////////////////////////////////////////////////////////////
void GexTrendChart::keepXTestViewport()
{
    if (m_pChartOverlays && referenceTestX())
    {
        referenceTestX()->ptChartOptions->setCustomViewportX(true);
        referenceTestX()->ptChartOptions->setLowX(m_pChartOverlays->getViewportRectangle()[type()].lfLowY);
        referenceTestX()->ptChartOptions->setHighX(m_pChartOverlays->getViewportRectangle()[type()].lfHighY);

        referenceTestX()->ptChartOptions->setCustomViewportY(true);
        referenceTestX()->ptChartOptions->setLowY(m_pChartOverlays->getViewportRectangle()[type()].lfLowX);
        referenceTestX()->ptChartOptions->setHighY(m_pChartOverlays->getViewportRectangle()[type()].lfHighX);

    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool formatXAxisLabel()
//
// Description	:	Indicates if the X axis label should be formatted. *
//					X axis label shouldn't be formatted for the Trend Chart as
//					we display only Run ID or Part ID
//
///////////////////////////////////////////////////////////////////////////////////
bool GexTrendChart::formatXAxisLabel()
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool hasSpotsToChart(CGexSingleChart * pLayerStyle)
//
// Description	:	return true if the layer contains spots on this chart
//
///////////////////////////////////////////////////////////////////////////////////
bool GexTrendChart::hasSpotsToChart(CGexSingleChart * pLayerStyle)
{
    // Use spots when spots is checked or line is unchecked.
    if (pLayerStyle)
        return pLayerStyle->bSpots || !pLayerStyle->bLines;
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Class GexTrendPartInfo - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexTrendPartInfo::GexTrendPartInfo(const QString& strPartID, int nRunID, int nPosID)
{
    m_strPartID = strPartID;
    m_nRunID	= nRunID;
    m_nPosID	= nPosID;
}

GexTrendPartInfo::GexTrendPartInfo(const GexTrendPartInfo& trendPartID)
{
    *this = trendPartID;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexTrendPartInfo::~GexTrendPartInfo()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	 GexTrendPartInfo& operator=(const GexTrendPartInfo& trendPartInfo)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
GexTrendPartInfo& GexTrendPartInfo::operator=(const GexTrendPartInfo& trendPartInfo)
{
    if (this != &trendPartInfo)
    {
        m_strPartID	= trendPartInfo.m_strPartID;
        m_nRunID	= trendPartInfo.m_nRunID;
        m_nPosID	= trendPartInfo.m_nPosID;
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	 bool operator<(const GexTrendPartInfo& trendPartInfo) const
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexTrendPartInfo::operator<(const GexTrendPartInfo& trendPartInfo) const
{
    bool bOkThis	= false;
    bool bOkArg		= false;

    long lPartIDThis	= m_strPartID.toLong(&bOkThis);
    long lPartIDArg		= trendPartInfo.m_strPartID.toLong(&bOkArg);

    if (bOkThis && bOkArg)
        return lPartIDThis < lPartIDArg;

    return m_strPartID < trendPartInfo.m_strPartID;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	 bool operator==(const GexTrendPartInfo& trendPartInfo) const
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexTrendPartInfo::operator==(const GexTrendPartInfo& trendPartInfo) const
{
    return m_strPartID == trendPartInfo.m_strPartID;
}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void customizeChart()
//
// Description	:   This function implements the virtual function of GexAbstractChart.
//                  In this function, we draw all test limits
//
///////////////////////////////////////////////////////////////////////////////////
void GexTrendChart::customizeChart()
{
    // Chart rolling limits only when trend chart is plotted over run_id
    if(m_pReportOptions->GetOption("adv_trend","x_axis").toString() == "run_id")
    {
        CTest * test = referenceTestX();
        QString strAdvTrendMarkerOptions =(m_pReportOptions->GetOption(QString("adv_trend"), QString("marker"))).toString();
        QStringList qslAdvTrendMarkerOptionList = strAdvTrendMarkerOptions.split(QString("|"));

        // rolling limits are drawn only if the toggle is at true and the test has subset limits
        if (!qslAdvTrendMarkerOptionList.contains("rolling_limits") ||
            test->GetRollingLimits().HasSubsetLimit() == false)
            return;

        QString lScaling = m_pReportOptions->GetOption("dataprocessing","scaling").toString();

        if (m_pChartOverlays)
        {
            CGexSingleChart *   lChartLayer = NULL;
            CGexGroupOfFiles *  lGroup      = NULL;
            CTest *             lTest       = NULL;
            Chart::SymbolType	symbolLayer;
            int					nLineColor;
            int					nFillColor;
            bool				bIsMultiLayer = isMultiLayers();
            double              lLowLimit;
            double              lHighLimit;

            for (int lLayer = 0; lLayer < m_pChartOverlays->chartsList().count(); ++lLayer)
            {
                lChartLayer = m_pChartOverlays->chartsList().at(lLayer);

                // Seek to the relevant Group to plot.
                if ((lChartLayer->iGroupX < 0) ||
                    (lChartLayer->iGroupX >= gexReport->getGroupsList().size()) ||
                    lChartLayer->bVisible == false)
                    lGroup = NULL;
                else
                    lGroup = gexReport->getGroupsList().at(lChartLayer->iGroupX);

                if (lGroup)
                    lTest = lGroup->FindTestCell(test->lTestNumber, test->strTestName, test->lPinmapIndex);
                else
                    lTest = NULL;

                if (lTest && lTest->GetRollingLimits().HasSubsetLimit())
                {
                    GS::Gex::TestSubsetLimits& subsetLimits = lTest->GetRollingLimits().GetTestLimits();

                    GS::Gex::RunIdLimits    lPreviousLimit;
                    int                     lPreviousRun    = -1;

                    // Set Line style + symbol type + color (if any)
                    getLayerStyles(0, lChartLayer, nFillColor, nLineColor, symbolLayer);

                    for(GS::Gex::TestSubsetLimits::const_iterator it = subsetLimits.begin();
                        it != subsetLimits.end();
                        ++it)
                    {
                        if (lPreviousRun != -1)
                        {
                            for (int lIdx = 0; lIdx < lPreviousLimit.count(); ++lIdx)
                            {
                                lLowLimit   = lPreviousLimit.at(lIdx).lowLimit();
                                lHighLimit  = lPreviousLimit.at(lIdx).highLimit();

                                if (lScaling != "normalized")
                                {
                                    lLowLimit   /= ScalingPower(test->res_scal);
                                    lHighLimit  /= ScalingPower(test->res_scal);
                                }

                                drawCustomLine( m_pXYChart->getXCoor( lPreviousRun ),
                                                m_pXYChart->getYCoor( lLowLimit),
                                                m_pXYChart->getXCoor( it.key() ),
                                                m_pXYChart->getYCoor( lLowLimit),
                                                lChartLayer->limitsColor(bIsMultiLayer).rgb() & 0xffffff,
                                                //m_pXYChart->
    //                                            0x000000, //0xffffff:white
                                                //m_pXYChart->dashLineColor( QColor(Qt::blue).rgb() & 0xffffff, Chart::DotDashLine),
                                                1
                                                );

                                drawCustomLine( m_pXYChart->getXCoor( lPreviousRun ),
                                                m_pXYChart->getYCoor( lHighLimit),
                                                m_pXYChart->getXCoor( it.key() ),
                                                m_pXYChart->getYCoor( lHighLimit),
                                                lChartLayer->limitsColor(bIsMultiLayer).rgb() & 0xffffff,
                                                //m_pXYChart->
    //                                            0x000000, //0xffffff:white
                                                //m_pXYChart->dashLineColor( QColor(Qt::blue).rgb() & 0xffffff, Chart::DotDashLine),
                                                1
                                                );
                            }
                        }

                        lPreviousRun    = it.key();
                        lPreviousLimit  = it.value();
                    }

                    if (lPreviousRun != -1)
                    {
                        for (int lIdx = 0; lIdx < lPreviousLimit.count(); ++lIdx)
                        {
                            lLowLimit   = lPreviousLimit.at(lIdx).lowLimit();
                            lHighLimit  = lPreviousLimit.at(lIdx).highLimit();

                            if (lScaling != "normalized")
                            {
                                lLowLimit   /= ScalingPower(test->res_scal);
                                lHighLimit  /= ScalingPower(test->res_scal);
                            }

                            drawCustomLine( m_pXYChart->getXCoor( lPreviousRun ),
                                            m_pXYChart->getYCoor( lLowLimit),
                                            m_pXYChart->getXCoor( lTest->m_testResult.count() ),
                                            m_pXYChart->getYCoor( lLowLimit),
                                            lChartLayer->limitsColor(bIsMultiLayer).rgb() & 0xffffff,
                                            //m_pXYChart->
    //                                        0x000000, //0xffffff:white
                                            //m_pXYChart->dashLineColor( QColor(Qt::blue).rgb() & 0xffffff, Chart::DotDashLine),
                                            1
                                            );

                            drawCustomLine( m_pXYChart->getXCoor( lPreviousRun ),
                                            m_pXYChart->getYCoor( lHighLimit),
                                            m_pXYChart->getXCoor( lTest->m_testResult.count() ),
                                            m_pXYChart->getYCoor( lHighLimit),
                                            lChartLayer->limitsColor(bIsMultiLayer).rgb() & 0xffffff,
                                            //m_pXYChart->
    //                                        0x000000, //0xffffff:white
                                            //m_pXYChart->dashLineColor( QColor(Qt::blue).rgb() & 0xffffff, Chart::DotDashLine),
                                            1
                                            );
                        }
                    }
                }
            }
        }
    }
}
