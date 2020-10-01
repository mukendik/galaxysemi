///////////////////////////////////////////////////////////////////////////////////
// GEX includes
///////////////////////////////////////////////////////////////////////////////////
#include <limits>
#include "gexscatterchart.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "report_options.h"
#include <gqtl_log.h>
#include "gstdl_linreg.h"
#include "cpart_info.h"
#include <gqtl_global.h>

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport * gexReport;
extern CReportOptions ReportOptions;

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern double	ScalingPower(int iPower);

///////////////////////////////////////////////////////////////////////////////////
// Class GexScatterChart - class which
///////////////////////////////////////////////////////////////////////////////////

QMap< QPair< CTest*,CTest* >, QMap<QString, QVariant> > GexScatterChart::sLinearRegressionResults;

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexScatterChart::GexScatterChart(int SizeMode,GexWizardChart* lWizardParent, CGexChartOverlays* pChartOverlays /*= NULL*/)
    : GexAbstractChart(chartTypeScatter, SizeMode, lWizardParent, pChartOverlays)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("new GexScatterChart : SizeMode = %1").arg( SizeMode).toLatin1().constData());
    mIsLinearRegressionEnabled = gexReport->GetOption("adv_correlation", "linear_regression").toBool();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexScatterChart::~GexScatterChart()
{
    //sLinearRegressionResults.clear();
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

void GexScatterChart::buildOwnerChart()
{
    CTest *	ptTestCellX					= NULL;
    CTest *	ptTestCellY					= NULL;
    CTest *	ptScaleTestCellReferenceX	= referenceTestX();	// Pointer to Test used a reference for scaling if multiple layers involved
    CTest *	ptScaleTestCellReferenceY	= referenceTestY();	// Pointer to Test used a reference for scaling if multiple layers involved

  GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Build scatter chart for test %1 vs test %2").arg(
           (ptScaleTestCellReferenceX) ?
            static_cast<int>(ptScaleTestCellReferenceX->lTestNumber) : -1)
        .arg(
           (ptScaleTestCellReferenceY) ?
            static_cast<int>(ptScaleTestCellReferenceY->lTestNumber) : -1).toLatin1().constData());
    char	szString[300];
    double	lfMeanX,lfMinX,lfMaxX,lfMedianX,lfSigmaX,lfExtraX;
    double	lfChartBottomX	= C_INFINITE;
    double	lfChartTopX		= -C_INFINITE;
    double	lfLowLX			= C_INFINITE;
    double	lfHighLX		= -C_INFINITE;
    double	lfLowSpecLX		= C_INFINITE;
    double	lfHighSpecLX	= C_INFINITE;
    double	lfDataStartX,lfDataEndX,lfDataStartY,lfDataEndY;
    double	lfMeanY,lfMinY,lfMaxY,lfMedianY,lfSigmaY,lfExtraY;
    double	lfChartBottomY	= C_INFINITE;
    double	lfChartTopY		= -C_INFINITE;
    double	lfLowLY			= C_INFINITE;
    double	lfLowSpecLY		= C_INFINITE;
    double	lfHighLY		= -C_INFINITE;
    double	lfHighSpecLY	= -C_INFINITE;
    double	lfQuartileQ1X=-C_INFINITE,lfQuartileQ1Y=-C_INFINITE;
    double	lfQuartileQ3X=-C_INFINITE,lfQuartileQ3Y=-C_INFINITE;
    // Pointer to label strings...may vary depending of the chart size!
    int		lTestNumberX;
    int		lPinmapIndexX;
    int		lTestNumberY;
    int		lPinmapIndexY;
    int		iGroup=1;						// Counter used to know on which group we are (#1 or other)
    int		iTestReferenceScaleFactorX=0;	// Scale factor for X test (Chart scales based on it)
    int		iCustomScaleFactorX;			// Correction custom Scale factor to apply on all but 1st chart so all scales match!
    double	fCustomScaleFactorX=0.0;		// Scale ratio to apply multiplyer)
    int		iTestReferenceScaleFactorY=0;	// Scale factor for Y test (Chart scales based on it)
    int		iCustomScaleFactorY;			// Correction custom Scale factor to apply on all but 1st chart so all scales match!
    double	fCustomScaleFactorY=0.0;		// Scale ratio to apply multiplyer)
    QString strLabel;
    double	*x=NULL;				// buffer to hold X data.
    double	*y=NULL;				// Buffer to hold Y data.
    int		iTotalSamples;
    bool	bShowAxisAndGrid	= true;
    double	lfViewportLowX		= C_INFINITE;
    double	lfViewportHighX		= -C_INFINITE;
    double	lfViewportLowY		= C_INFINITE;
    double	lfViewportHighY		= -C_INFINITE;
    bool	bIsMultiLayer		= isMultiLayers();

    GexInternalChartInfo internalChartInfoX;
    GexInternalChartInfo internalChartInfoY;

    // Get pointer to first group & first file (we always have them exist)
    CGexFileInGroup *	pFileX	= NULL;
    CGexFileInGroup *	pFileY	= NULL;
    CGexGroupOfFiles *	pGroupX	= NULL;
    CGexGroupOfFiles *	pGroupY	= NULL;

    QList<CGexGroupOfFiles*>::iterator	itGroupList = gexReport->getGroupsList().begin();
    CGexGroupOfFiles *					pGroup		= (itGroupList != gexReport->getGroupsList().end()) ? (*itGroupList) : NULL;

    QString scaling=m_pReportOptions->GetOption("dataprocessing","scaling").toString();

    // Index used to keep track of Chart layers being charted.
    int				i;
    QString         lParameterNameX;
    QString         lParameterNameY;
    CGexSingleChart	*pChart=NULL;	// Handle to Parameter Layer info.
  //FIXME: not used ?
  //int iLineWidth=1;
    int				iLayerIndex=0;
    QColor			cPenColor;
    QColor			cLayerColor;	// Color used to chart a layer.
    int				cPenWidth;
    bool			bVisible;

    // Stack all charts of all groups for each same test
    m_listTestX.clear();
    m_listTestY.clear();
    m_listFileX.clear();
    m_listFileY.clear();
    while(pGroup)
    {
        if(m_pChartOverlays)
        {
            if (iLayerIndex >= 0 && iLayerIndex < m_pChartOverlays->chartsList().count())
                pChart = m_pChartOverlays->chartsList().at(iLayerIndex);
            else
                break;

            // Seek to the relevant Group to plot.
            if ((pChart->iGroupX < 0) || (pChart->iGroupX >= gexReport->getGroupsList().size()))
                pGroupX = NULL;
            else
                pGroupX = gexReport->getGroupsList().at(pChart->iGroupX);
            pFileX	= pFileY = (pGroupX->pFilesList.isEmpty()) ? NULL : pGroupX->pFilesList.first();

            // If no group defined in Y, then use same group as in X!
            if(pChart->iGroupY >= 0)
            {
                pGroupY = (pChart->iGroupY >= gexReport->getGroupsList().size()) ? NULL : gexReport->getGroupsList().at(pChart->iGroupY);
                pFileY	= (pGroupY->pFilesList.isEmpty()) ? NULL : pGroupY->pFilesList.first();
            }

            // We have multiple charts (layers) to plot!
            lTestNumberX		= pChart->iTestNumberX;
            lPinmapIndexX		= pChart->iPinMapX;
            lParameterNameX     = pChart->strTestNameX;
            lTestNumberY		= pChart->iTestNumberY;
            lPinmapIndexY		= pChart->iPinMapY;
            lParameterNameY     = pChart->strTestNameY;

            // Next layerID to chart
            iLayerIndex++;
        }
        else
        {
            pGroupX	= pGroup;
            pGroupY	= pGroup;

            // Find test cell: RESET list to ensure we scan list of the right group!
            // Check if pinmap index...
            lTestNumberX		= referenceTestX()->lTestNumber;
            lParameterNameX     = referenceTestX()->strTestName;

            if(referenceTestX()->lPinmapIndex >= 0)
                lPinmapIndexX = referenceTestX()->lPinmapIndex;
            else
                lPinmapIndexX = GEX_PTEST;

            lTestNumberY		= referenceTestY()->lTestNumber;
            lParameterNameY     = referenceTestY()->strTestName;

            if(referenceTestY()->lPinmapIndex >= 0)
                lPinmapIndexY = referenceTestY()->lPinmapIndex;
            else
                lPinmapIndexY = GEX_PTEST;
        }

        ptTestCellX = ptTestCellY = NULL;
        pFileX		= (pGroupX->pFilesList.isEmpty()) ? NULL : pGroupX->pFilesList.first();
        pFileY		= (pGroupY->pFilesList.isEmpty()) ? NULL : pGroupY->pFilesList.first();

        if(pFileX->FindTestCell(lTestNumberX,lPinmapIndexX,&ptTestCellX,false,false,lParameterNameX)!=1)
            goto NextGroup_RefScaleFactor;

        if(pFileY->FindTestCell(lTestNumberY,lPinmapIndexY,&ptTestCellY,false,false,lParameterNameY)!=1)
            goto NextGroup_RefScaleFactor;

        m_listFileX.append(pFileX);
        m_listFileY.append(pFileY);
        m_listTestX.append(ptTestCellX);
        m_listTestY.append(ptTestCellY);

        // If multiple charts (groups or overlays), scale based on 1st parameter in 1st group.
        if (ptTestCellX->m_testResult.count() > 0 && internalChartInfoX.m_bTestReferenceScaleFactor == false)
        {
            iTestReferenceScaleFactorX	= ptTestCellX->res_scal;
            ptScaleTestCellReferenceX	= ptTestCellX;
            internalChartInfoX.m_bTestReferenceScaleFactor = true;
        }

        if (ptTestCellY->m_testResult.count() > 0 && internalChartInfoY.m_bTestReferenceScaleFactor == false)
        {
            iTestReferenceScaleFactorY	= ptTestCellY->res_scal;
            ptScaleTestCellReferenceY	= ptTestCellY;
            internalChartInfoY.m_bTestReferenceScaleFactor = true;
        }

    NextGroup_RefScaleFactor:

        if(m_pChartOverlays == NULL)
        {
            ++itGroupList;

            pGroup = (itGroupList != gexReport->getGroupsList().end()) ? (*itGroupList) : NULL;
        }
    };

    // Plot all groups...unless in interactive mode (then only plot the layers defined in the layer list)
    iLayerIndex				= 0;
    iGroup					= 1;
    itGroupList				= gexReport->getGroupsList().begin();
    pGroup					= (itGroupList != gexReport->getGroupsList().end()) ? (*itGroupList) : NULL;

    QString acm         = m_pReportOptions->GetOption("adv_correlation","marker").toString();
    bool    lShowTitle  = false;

    if ((m_pChartOverlays && m_pChartOverlays->mTitle == true) ||
        acm.contains(QString("test_name")))
    {
        lShowTitle = true;
    }

    // Stack all charts of all groups for each same test
    while(pGroup)
    {
        if(m_pChartOverlays)
        {
            if (iLayerIndex >= 0 && iLayerIndex < m_pChartOverlays->chartsList().count())
                pChart = m_pChartOverlays->chartsList().at(iLayerIndex);
            else
                break;

            // Seek to the relevant Group to plot.
            if ((pChart->iGroupX < 0) || (pChart->iGroupX >= gexReport->getGroupsList().size()))
                pGroupX = NULL;
            else
                pGroupX = gexReport->getGroupsList().at(pChart->iGroupX);
            pFileX	= pFileY = (pGroupX->pFilesList.isEmpty()) ? NULL : pGroupX->pFilesList.first();

            // If no group defined in Y, then use same group as in X!
            if(pChart->iGroupY >= 0)
            {
                pGroupY = (pChart->iGroupY >= gexReport->getGroupsList().size()) ? NULL : gexReport->getGroupsList().at(pChart->iGroupY);
                pFileY	= (pGroupY->pFilesList.isEmpty()) ? NULL : pGroupY->pFilesList.first();
            }

            // We have multiple charts (layers) to plot!
            lTestNumberX		= pChart->iTestNumberX;
            lPinmapIndexX		= pChart->iPinMapX;
            lParameterNameX     = pChart->strTestNameX;
            lTestNumberY		= pChart->iTestNumberY;
            lPinmapIndexY		= pChart->iPinMapY;
            lParameterNameY     = pChart->strTestNameY;

            // Get ploting details
            //FIXME: not used ?
            /*
            if(pChart->bLines || pChart->bBoxBars || pChart->bBox3DBars || pChart->bFittingCurve)
                iLineWidth = pChart->iLineWidth;
            else
                iLineWidth = 0;
            */

            // Flag saying if layer is visible...
            bVisible = pChart->bVisible;

            // Next layerID to chart
            iLayerIndex++;
        }
        else
        {
            pGroupX	= pGroup;
            pGroupY	= pGroup;

            // NON Interactive (HTML page created)
            //FIXME: not used ?
            //iLineWidth = 0;	// No line between points.

            // Chart always visible when creating an image.
            bVisible = true;

            // Find test cell: RESET list to ensure we scan list of the right group!
            // Check if pinmap index...
            lTestNumberX		= referenceTestX()->lTestNumber;
            lParameterNameX     = referenceTestX()->strTestName;

            if(referenceTestX()->lPinmapIndex >= 0)
                lPinmapIndexX = referenceTestX()->lPinmapIndex;
            else
                lPinmapIndexX = GEX_PTEST;

            lTestNumberY		= referenceTestY()->lTestNumber;
            lParameterNameY     = referenceTestY()->strTestName;

            if(referenceTestY()->lPinmapIndex >= 0)
                lPinmapIndexY = referenceTestY()->lPinmapIndex;
            else
                lPinmapIndexY = GEX_PTEST;
        }

        ptTestCellX = ptTestCellY = NULL;
        pFileX		= (pGroupX->pFilesList.isEmpty()) ? NULL : pGroupX->pFilesList.first();
        pFileY		= (pGroupY->pFilesList.isEmpty()) ? NULL : pGroupY->pFilesList.first();

        if(pFileX->FindTestCell(lTestNumberX,lPinmapIndexX,&ptTestCellX,false,false,lParameterNameX)!=1)
            goto NextGroup;

        if(pFileY->FindTestCell(lTestNumberY,lPinmapIndexY,&ptTestCellY,false,false,lParameterNameY)!=1)
            goto NextGroup;

        // We have a chart title unless the chart size in only 200 pixel wide (SMALL chart mode)!
        if(		(m_nSizeMode != GEX_CHARTSIZE_SMALL)
                &&
                (acm.split("|").contains("test_name"))
            )
            strLabel = "Scatter / BiVariate plot";

        // Multilayrs, check if we have assigned it a title...
        if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bChartTitle)
            strLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strChartTitle;	// custom title.

        // if LinearRegression on
        {
            QMap<QString, QVariant> lrd;
            QPair< CTest*, CTest* > p; p.first=ptTestCellX; p.second=ptTestCellY;
            if (sLinearRegressionResults.find(p)==sLinearRegressionResults.end() )
            {
                p.first=ptTestCellY; p.second=ptTestCellY;
                if (sLinearRegressionResults.find(p)!=sLinearRegressionResults.end())
                    lrd=sLinearRegressionResults.value(p);
            }
            else
                lrd=sLinearRegressionResults.value(p);

            if (!lrd.isEmpty())
            {
                //strLabel.append(QString(" Y=%1 X+%2").arg(lrd["B"].toDouble()).arg(lrd["A"].toDouble()) );
            }
        }

        m_pXYChart->addTitle(strLabel.toLatin1().constData(), 0, 10);

        iCustomScaleFactorX = ptTestCellX->res_scal - iTestReferenceScaleFactorX;
        fCustomScaleFactorX = 1/GS_POW(10.0,iCustomScaleFactorX);
        iCustomScaleFactorY = ptTestCellY->res_scal - iTestReferenceScaleFactorY;
        fCustomScaleFactorY = 1/GS_POW(10.0,iCustomScaleFactorY);

        // If multiple charts (groups or overlays), axis labels based on 1st parameter in 1st group.
        if (iGroup == 1)
        {
            // Set X axis title
            strLabel  = "T";
            strLabel += ptTestCellX->szTestLabel;
            strLabel += " - " + ptTestCellX->strTestName;
            QString strUnits = ptScaleTestCellReferenceX->GetScaledUnits(&fCustomScaleFactorX, scaling);

            if(strUnits.length() > 0)
                strLabel += "(" + strUnits + ")";

            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX)
                strLabel += " - Log scale";

            if (internalChartInfoX.m_bTestReferenceScaleFactor == false)
                strLabel += "<*color=FF0000*> - No data samples<*/color*>";

            if (bShowAxisAndGrid)
            {
                // Check if we have a custom legend to overwrite default one
                if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLegendX)
                    strLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strAxisLegendX;

                m_pXYChart->xAxis()->setTitle(strLabel.toLatin1().constData());
            }

            // Set Y axis title
            strLabel  = "T";
            strLabel += ptTestCellY->szTestLabel;
            strLabel += " - " + ptTestCellY->strTestName;
            strUnits = ptScaleTestCellReferenceY->GetScaledUnits(&fCustomScaleFactorY, scaling);

            if(strUnits.length() > 0)
                strLabel += "(" + strUnits + ")";

            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY)
                strLabel += " - Log scale";

            if (internalChartInfoY.m_bTestReferenceScaleFactor == false)
                strLabel += "<*color=FF0000*> - No data samples<*/color*>";

            if (bShowAxisAndGrid)
            {
                // Check if we have a custom legend to overwrite default one
                if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLegendY)
                    strLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strAxisLegendY;

                m_pXYChart->yAxis()->setTitle(strLabel.toLatin1().constData());
            }
        }

        // Make sure that we have a valid result table for both axis.
        iTotalSamples = gex_min(ptTestCellX->m_testResult.count(),ptTestCellY->m_testResult.count());

        if (iTotalSamples<=0)
            goto NextGroup;

        // Overlay: We need to rescale data to match 1st Curve scales....

        if(x)
        {
            delete x; x=0;
        }
        if(y)
        {
            delete y; y=0;
        }
        GSLOG(SYSLOG_SEV_DEBUG, QString("Allocing 2 tables of %1 doubles for x & y...").arg( iTotalSamples).toLatin1().constData());
        x = new double[iTotalSamples];
        if (!x)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Cant alloc tables of %1 double for x !").arg( iTotalSamples).toLatin1().constData());
            return;
        }
        y = new double[iTotalSamples];
        if (!y)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Cant alloc tables of %1 double for y !").arg( iTotalSamples).toLatin1().constData());
            return;
        }

        // resultant scale factors
        {
            double lfScaleFactorX = 1, lfScaleFactorY = 1;
            if(iCustomScaleFactorX)
                lfScaleFactorX = fCustomScaleFactorX;
            if(iCustomScaleFactorY)
                lfScaleFactorY = fCustomScaleFactorY;

            // Copy the data
            for(i=0;i<iTotalSamples;i++)
            {
                x[i] = ptTestCellX->m_testResult.resultAt(i) * lfScaleFactorX;
                y[i] = ptTestCellY->m_testResult.resultAt(i) * lfScaleFactorY;
            }
        }   // brace used to avoid 'goto' problems ...

        // Insert Curve (if visible)
        if(bVisible)
        {
            int nFillColor=-1, nLineColor=0;
            Chart::SymbolType layerSymbol;
            getLayerStyles(iGroup, pChart, nFillColor, nLineColor, layerSymbol);
            // Add data to the chart
            m_pXYChart->addScatterLayer(DoubleArray(x, iTotalSamples), DoubleArray(y, iTotalSamples), "", layerSymbol, 6, nLineColor);
        }

        // Delete array
        GSLOG(SYSLOG_SEV_DEBUG, "deleting x & y double tables...");
        if (x)
            delete x;
        x=0; // Case 6094 : Dont forget to nullify x to prevent crash for the next loop  !!!!!!!
        if (y)
            delete y;
        y=0; // Case 6094 : Dont forget to nullify y to prevent crash for the next loop  !!!!!!!

        lfDataEndX		= ptTestCellX->lfSamplesMax;
        lfDataStartX	= ptTestCellX->lfSamplesMin;
        lfDataEndY		= ptTestCellY->lfSamplesMax;
        lfDataStartY	= ptTestCellY->lfSamplesMin;

        if(iCustomScaleFactorX)
        {
            lfDataEndX		*= fCustomScaleFactorX;
            lfDataStartX	*= fCustomScaleFactorX;
        }

        if(iCustomScaleFactorY)
        {
            lfDataEndY		*= fCustomScaleFactorX;
            lfDataStartY	*= fCustomScaleFactorX;
        }

        // Scale data to correct scale
        pFileX->FormatTestResultNoUnits(&lfDataEndX,		ptTestCellX->res_scal);
        pFileX->FormatTestResultNoUnits(&lfDataStartX,	ptTestCellX->res_scal);
        pFileY->FormatTestResultNoUnits(&lfDataEndY,		ptTestCellY->res_scal);
        pFileY->FormatTestResultNoUnits(&lfDataStartY,	ptTestCellY->res_scal);

        // Draw limit markers in X
        if(((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) && (iGroup==1))
        {
            // Low limit exists
            lfLowLX = ptTestCellX->GetCurrentLimitItem()->lfLowLimit;

            // If we have to keep values in normalized format, do not rescale!
            if (scaling!="normalized")
            {
                // convert LowLimit to same scale as results:
                pFileX->FormatTestResultNoUnits(&lfLowLX, ptTestCellX->llm_scal);

                lfLowLX *=  ScalingPower(ptTestCellX->llm_scal);	// normalized
                lfLowLX /=  ScalingPower(ptTestCellX->res_scal);	// normalized
            }

            // LowLimit Marker
            if(	(acm.split("|").contains("limits"))  || (pChart != NULL)	)
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = pChart->limitsLineWidth();
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    sprintf(szString,"X: %s",m_strLowLimit.toLatin1().constData());

                    addMarker(m_pXYChart->xAxis(), lfLowLX, cPenColor.rgb() & 0xffffff, szString, cPenWidth, Chart::BottomRight);
                }
            }
        }

        if(((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) && (iGroup==1))
        {
            lfHighLX = ptTestCellX->GetCurrentLimitItem()->lfHighLimit;		// High limit exists
            // If we have to keep values in normalized format, do not rescale!
            if (scaling!="normalized")
            {
                // convert HighLimit to same scale as results:
                pFileY->FormatTestResultNoUnits(&lfHighLX, ptTestCellX->hlm_scal);

                lfHighLX *=  ScalingPower(ptTestCellX->hlm_scal);	// normalized
                lfHighLX /=  ScalingPower(ptTestCellX->res_scal);	// normalized
            }

            // High limit Marker
            if((acm.split("|").contains("limits"))
                    || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = pChart->limitsLineWidth();
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    sprintf(szString,"X: %s",m_strHighLimit.toLatin1().constData());

                    addMarker(m_pXYChart->xAxis(), lfHighLX, cPenColor.rgb() & 0xffffff, szString, cPenWidth, Chart::TopLeft);
                }
            }
        }

        // Draw Spec limit markers in X
        if(((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0) && (iGroup==1))
        {
            // Low limit exists
            lfLowSpecLX = ptTestCellX->lfLowSpecLimit;

            // If we have to keep values in normalized format, do not rescale!
            if(scaling!="normalized")
            {
                // convert LowLimit to same scale as results:
                pFileX->FormatTestResultNoUnits(&lfLowSpecLX, ptTestCellX->llm_scal);

                lfLowSpecLX *=  ScalingPower(ptTestCellX->llm_scal);	// normalized
                lfLowSpecLX /=  ScalingPower(ptTestCellX->res_scal);	// normalized
            }

            internalChartInfoX.m_dLowSpecLimit = gex_min(internalChartInfoX.m_dLowSpecLimit, lfLowSpecLX);

            // LowLimit Marker
            if((acm.split("|").contains("speclimits"))
                    || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = pChart->limitsLineWidth();
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    sprintf(szString,"X: %s",m_strLowSpecLimit.toLatin1().constData());

                    addMarker(m_pXYChart->xAxis(), lfLowSpecLX, cPenColor.rgb() & 0xffffff, szString, cPenWidth, Chart::BottomRight);
                }
            }
        }

        if(((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0) && (iGroup==1))
        {
            lfHighSpecLX = ptTestCellX->lfHighSpecLimit;		// High Spec limit exists

            // If we have to keep values in normalized format, do not rescale!
            if (scaling!="normalized")
            {
                // convert HighLimit to same scale as results:
                pFileX->FormatTestResultNoUnits(&lfHighSpecLX, ptTestCellX->hlm_scal);

                lfHighSpecLX *=  ScalingPower(ptTestCellX->hlm_scal);	// normalized
                lfHighSpecLX /=  ScalingPower(ptTestCellX->res_scal);	// normalized
            }

            internalChartInfoX.m_dHighSpecLimit = gex_max(internalChartInfoX.m_dHighSpecLimit, lfHighSpecLX);

            // High limit Marker
            if((acm.split("|").contains("speclimits"))
                    || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = pChart->limitsLineWidth();
                }
                else
                {
                    cPenColor = Qt::red;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    sprintf(szString,"X: %s",m_strHighSpecLimit.toLatin1().constData());

                    addMarker(m_pXYChart->xAxis(), lfHighSpecLX, cPenColor.rgb() & 0xffffff, szString, cPenWidth, Chart::TopLeft);
                }
            }
        }

        // Draw limit markers in Y
        if(((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) && (iGroup==1))
        {
            // Low limit exists
            lfLowLY = ptTestCellY->GetCurrentLimitItem()->lfLowLimit;

            // If we have to keep values in normalized format, do not rescale!
            if (scaling!="normalized")
            {
                // convert LowLimit to same scale as results:
                pFileY->FormatTestResultNoUnits(&lfLowLY,ptTestCellY->llm_scal);

                lfLowLY *=  ScalingPower(ptTestCellY->llm_scal);	// normalized
                lfLowLY /=  ScalingPower(ptTestCellY->res_scal);	// normalized
            }

            // LowLimit Marker
            if((acm.split("|").contains("limits"))
                    || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = pChart->limitsLineWidth();
                }
                else
                {
                    cPenColor = Qt::magenta;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    sprintf(szString,"Y: %s", m_strLowLimit.toLatin1().constData());
                    addMarker(m_pXYChart->yAxis(), lfLowLY, cPenColor.rgb() & 0xffffff, szString, cPenWidth, Chart::TopRight);
                }
            }
        }
        if(((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) && (iGroup==1))
        {
            // High limit exists
            lfHighLY = ptTestCellY->GetCurrentLimitItem()->lfHighLimit;

            // If we have to keep values in normalized format, do not rescale!
            //if(m_pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
            if (scaling!="normalized")
            {
                // convert HighLimit to same scale as results:
                pFileY->FormatTestResultNoUnits(&lfHighLY,ptTestCellY->hlm_scal);

                lfHighLY *=  ScalingPower(ptTestCellY->hlm_scal);	// normalized
                lfHighLY /=  ScalingPower(ptTestCellY->res_scal);	// normalized
            }

            // High limit Marker
            if((acm.split("|").contains("limits"))
                    || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = pChart->limitsLineWidth();
                }
                else
                {
                    cPenColor = Qt::magenta;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    sprintf(szString,"Y: %s", m_strHighLimit.toLatin1().constData());
                    addMarker(m_pXYChart->yAxis(), lfHighLY, cPenColor.rgb() & 0xffffff, szString, cPenWidth, Chart::BottomLeft);
                }
            }
        }

        // Draw spec limit markers in Y
        if(((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0) && (iGroup==1))
        {
            // Low limit exists
            lfLowSpecLY = ptTestCellY->lfLowSpecLimit;

            // If we have to keep values in normalized format, do not rescale!
            //if(m_pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
            if (scaling!="normalized")
            {
                // convert LowLimit to same scale as results:
                pFileY->FormatTestResultNoUnits(&lfLowSpecLY,ptTestCellY->llm_scal);

                lfLowSpecLY *=  ScalingPower(ptTestCellY->llm_scal);	// normalized
                lfLowSpecLY /=  ScalingPower(ptTestCellY->res_scal);	// normalized
            }

            internalChartInfoY.m_dLowSpecLimit = gex_min(internalChartInfoY.m_dLowSpecLimit, lfLowSpecLY);

            // LowLimit Marker
            if((acm.split("|").contains("speclimits"))
                    || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = pChart->limitsLineWidth();
                }
                else
                {
                    cPenColor = Qt::magenta;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    sprintf(szString,"Y: %s", m_strLowSpecLimit.toLatin1().constData());
                    addMarker(m_pXYChart->yAxis(), lfLowSpecLY, cPenColor.rgb() & 0xffffff, szString, cPenWidth, Chart::TopRight);
                }
            }
        }
        if(((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0) && (iGroup==1))
        {
            // High limit exists
            lfHighSpecLY = ptTestCellY->lfHighSpecLimit;

            // If we have to keep values in normalized format, do not rescale!
            //if(m_pReportOptions->iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
            if (scaling!="normalized")
            {
                // convert HighLimit to same scale as results:
                pFileY->FormatTestResultNoUnits(&lfHighSpecLY,ptTestCellY->hlm_scal);

                lfHighSpecLY *=  ScalingPower(ptTestCellY->hlm_scal);	// normalized
                lfHighSpecLY /=  ScalingPower(ptTestCellY->res_scal);	// normalized
            }

            internalChartInfoY.m_dHighSpecLimit = gex_max(internalChartInfoY.m_dHighSpecLimit, lfHighSpecLY);

            // High limit Marker
            if((acm.split("|").contains("speclimits"))
                    || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->limitsColor(bIsMultiLayer);
                    cPenWidth = pChart->limitsLineWidth();
                }
                else
                {
                    cPenColor = Qt::magenta;
                    cPenWidth = 1;
                }
                // Request to show limits
                if(cPenWidth)
                {
                    sprintf(szString,"Y: %s", m_strHighSpecLimit.toLatin1().constData());
                    addMarker(m_pXYChart->yAxis(), lfHighSpecLY, cPenColor.rgb() & 0xffffff, szString, cPenWidth, Chart::BottomLeft);
                }
            }
        }

        // X Insert markers (vertical lines): Mean, LowL, HighL
        // Scale XY Mean & Sigma to correct scale
        lfMeanX = ptTestCellX->lfMean;
        pFileX->FormatTestResultNoUnits(&lfMeanX,ptTestCellX->res_scal);
        lfSigmaX = ptTestCellX->lfSigma;
        pFileX->FormatTestResultNoUnits(&lfSigmaX,ptTestCellX->res_scal);
        lfMeanY = ptTestCellY->lfMean;
        pFileY->FormatTestResultNoUnits(&lfMeanY,ptTestCellY->res_scal);
        lfSigmaY = ptTestCellY->lfSigma;
        pFileY->FormatTestResultNoUnits(&lfSigmaY,ptTestCellY->res_scal);

        // COmpute Median
        if (ptTestCellX->lfSamplesQuartile2 != -C_INFINITE)
        {
            lfMedianX = ptTestCellX->lfSamplesQuartile2;
            pFileX->FormatTestResultNoUnits(&lfMedianX, ptTestCellX->res_scal);
        }

        if (ptTestCellY->lfSamplesQuartile2 != -C_INFINITE)
        {
            lfMedianY = ptTestCellY->lfSamplesQuartile2;
            pFileY->FormatTestResultNoUnits(&lfMedianY, ptTestCellY->res_scal);
        }

        if(iGroup == 1)
        {
            // If request to show the Mean XY markers
            //if((m_pReportOptions->bScatterMarkerMean) || (pChart != NULL))
            if((acm.split("|").contains("mean"))
                    || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->meanColor(bIsMultiLayer);
                    cPenWidth = pChart->meanLineWidth();
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
                    addMarker(m_pXYChart->xAxis(), lfMeanX, cPenColor.rgb() & 0xffffff, "X: Mean", cPenWidth, Chart::TopRight);
                    addMarker(m_pXYChart->yAxis(), lfMeanY, cPenColor.rgb() & 0xffffff, "Y: Mean", cPenWidth, Chart::TopRight);

                    internalChartInfoX.m_dMarkerMin = gex_min(internalChartInfoX.m_dMarkerMin, lfMeanX);
                    internalChartInfoX.m_dMarkerMax = gex_max(internalChartInfoX.m_dMarkerMax, lfMeanX);

                    internalChartInfoY.m_dMarkerMin = gex_min(internalChartInfoY.m_dMarkerMin, lfMeanY);
                    internalChartInfoY.m_dMarkerMax = gex_max(internalChartInfoY.m_dMarkerMax, lfMeanY);
                }
            }

            // If request to show the Median XY markers
            if(((acm.split("|").contains("median"))
                || (pChart != NULL)) && (lfMedianX != -C_INFINITE) && (lfMedianY != -C_INFINITE))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->medianColor(bIsMultiLayer);
                    cPenWidth = pChart->medianLineWidth();
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
                    addMarker(m_pXYChart->xAxis(), lfMedianX, cPenColor.rgb() & 0xffffff, "X: Median", cPenWidth, Chart::TopRight);
                    addMarker(m_pXYChart->yAxis(), lfMedianY, cPenColor.rgb() & 0xffffff, "Y: Median", cPenWidth, Chart::TopRight);

                    internalChartInfoX.m_dMarkerMin = gex_min(internalChartInfoX.m_dMarkerMin, lfMedianX);
                    internalChartInfoX.m_dMarkerMax = gex_max(internalChartInfoX.m_dMarkerMax, lfMedianX);

                    internalChartInfoY.m_dMarkerMin = gex_min(internalChartInfoY.m_dMarkerMin, lfMedianY);
                    internalChartInfoY.m_dMarkerMax = gex_max(internalChartInfoY.m_dMarkerMax, lfMedianY);
                }
            }

            // If request to show the Min XY markers
            if((m_pReportOptions->bScatterMarkerMin  || acm.split("|").contains("minimum")) || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->minColor(bIsMultiLayer);
                    cPenWidth = pChart->minLineWidth();
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
                    lfMinX = ptTestCellX->lfMin;
                    pFileX->FormatTestResultNoUnits(&lfMinX,ptTestCellX->res_scal);
                    lfMinY = ptTestCellY->lfMin;
                    pFileY->FormatTestResultNoUnits(&lfMinY,ptTestCellY->res_scal);

                    addMarker(m_pXYChart->xAxis(), lfMinX, cPenColor.rgb() & 0xffffff, "X: Min.", cPenWidth, Chart::TopRight);
                    addMarker(m_pXYChart->yAxis(), lfMinY, cPenColor.rgb() & 0xffffff, "Y: Min.", cPenWidth, Chart::TopRight);
                }
            }

            // If request to show the Max XY markers
            if((m_pReportOptions->bScatterMarkerMax || acm.split("|").contains("maximum")) || (pChart != NULL))
            {
                if(pChart != NULL)
                {
                    cPenColor = pChart->maxColor(bIsMultiLayer);
                    cPenWidth = pChart->maxLineWidth();
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
                    lfMaxX = ptTestCellX->lfMax;
                    pFileX->FormatTestResultNoUnits(&lfMaxX,ptTestCellX->res_scal);
                    lfMaxY = ptTestCellY->lfMax;
                    pFileY->FormatTestResultNoUnits(&lfMaxY,ptTestCellY->res_scal);

                    addMarker(m_pXYChart->xAxis(), lfMaxX, cPenColor.rgb() & 0xffffff, "X: Max.", cPenWidth, Chart::TopRight);
                    addMarker(m_pXYChart->yAxis(), lfMaxY, cPenColor.rgb() & 0xffffff, "X: Max.", cPenWidth, Chart::TopRight);
                }
            }

            // If request to show the 2sigma space
            //if((m_pReportOptions->bScatterMarker2Sigma) || (pChart != NULL))
            if((acm.split("|").contains("2sigma"))
                    || (pChart != NULL))
            {
                // 2Sigma: Horizontal (Y axis)
                if(pChart != NULL)
                {
                    cPenColor = pChart->sigma2Color(bIsMultiLayer);
                    cPenWidth = pChart->sigma2LineWidth();
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

                    addMarker(m_pXYChart->yAxis(), lfMeanY-lfSigmaY, nLineColor, "-1s", cPenWidth, Chart::BottomLeft)->setFontColor(nTextColor);
                    addMarker(m_pXYChart->yAxis(), lfMeanY+lfSigmaY, nLineColor, "+1s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);

                    // 2Sigma: Vertical (X axis)
                    addMarker(m_pXYChart->xAxis(), lfMeanX-lfSigmaX, nLineColor, "-1s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);
                    addMarker(m_pXYChart->xAxis(), lfMeanX+lfSigmaX, nLineColor, "+1s", cPenWidth, Chart::TopRight)->setFontColor(nTextColor);

                    internalChartInfoX.m_dMarkerMin = gex_min(internalChartInfoX.m_dMarkerMin, lfMeanX - lfSigmaX);
                    internalChartInfoX.m_dMarkerMax = gex_max(internalChartInfoX.m_dMarkerMax, lfMeanX + lfSigmaX);

                    internalChartInfoY.m_dMarkerMin = gex_min(internalChartInfoY.m_dMarkerMin, lfMeanY - lfSigmaY);
                    internalChartInfoY.m_dMarkerMax = gex_max(internalChartInfoY.m_dMarkerMax, lfMeanY + lfSigmaY);
                }
            }

            // If request to show the 3sigma space
            if((acm.split("|").contains("3sigma"))
                    || (pChart != NULL))
            {
                // 3Sigma: Horizontal (Y axis)
                if(pChart != NULL)
                {
                    cPenColor = pChart->sigma3Color(bIsMultiLayer);
                    cPenWidth = pChart->sigma3LineWidth();
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

                    addMarker(m_pXYChart->yAxis(), lfMeanY-1.5*lfSigmaY, nLineColor, "-1.5s", cPenWidth, Chart::BottomLeft)->setFontColor(nTextColor);
                    addMarker(m_pXYChart->yAxis(), lfMeanY+1.5*lfSigmaY, nLineColor, "+1.5s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);

                    // 3Sigma: Vertical (X axis)
                    addMarker(m_pXYChart->xAxis(), lfMeanX-1.5*lfSigmaX, nLineColor, "-1.5s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);
                    addMarker(m_pXYChart->xAxis(), lfMeanX+1.5*lfSigmaX, nLineColor, "+1.5s", cPenWidth, Chart::TopRight)->setFontColor(nTextColor);

                    internalChartInfoX.m_dMarkerMin = gex_min(internalChartInfoX.m_dMarkerMin, lfMeanX - 1.5 * lfSigmaX);
                    internalChartInfoX.m_dMarkerMax = gex_max(internalChartInfoX.m_dMarkerMax, lfMeanX + 1.5 * lfSigmaX);

                    internalChartInfoY.m_dMarkerMin = gex_min(internalChartInfoY.m_dMarkerMin, lfMeanY - 1.5 * lfSigmaY);
                    internalChartInfoY.m_dMarkerMax = gex_max(internalChartInfoY.m_dMarkerMax, lfMeanY + 1.5 * lfSigmaY);
                }
            }

            // If request to show the 6sigma space
            if((acm.split("|").contains("6sigma"))
                    || (pChart != NULL))
            {
                // 6Sigma: Horizontal (Y axis)
                if(pChart != NULL)
                {
                    cPenColor = pChart->sigma6Color(bIsMultiLayer);
                    cPenWidth = pChart->sigma6LineWidth();
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

                    addMarker(m_pXYChart->yAxis(), lfMeanY-3*lfSigmaY, nLineColor, "-3s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);
                    addMarker(m_pXYChart->yAxis(), lfMeanY+3*lfSigmaY, nLineColor, "+3s", cPenWidth, Chart::BottomLeft)->setFontColor(nTextColor);

                    // 6Sigma: Vertical (X axis)
                    addMarker(m_pXYChart->xAxis(), lfMeanX-3*lfSigmaX, nLineColor, "-3s", cPenWidth, Chart::TopRight)->setFontColor(nTextColor);
                    addMarker(m_pXYChart->xAxis(), lfMeanX+3*lfSigmaX, nLineColor, "+3s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);

                    internalChartInfoX.m_dMarkerMin = gex_min(internalChartInfoX.m_dMarkerMin, lfMeanX - 3 * lfSigmaX);
                    internalChartInfoX.m_dMarkerMax = gex_max(internalChartInfoX.m_dMarkerMax, lfMeanX + 3 * lfSigmaX);

                    internalChartInfoY.m_dMarkerMin = gex_min(internalChartInfoY.m_dMarkerMin, lfMeanY - 3 * lfSigmaY);
                    internalChartInfoY.m_dMarkerMax = gex_max(internalChartInfoY.m_dMarkerMax, lfMeanY + 3 * lfSigmaY);
                }
            }

            // If request to show the 12sigma space
            //if((m_pReportOptions->bScatterMarker12Sigma) || (pChart != NULL))
            if((acm.split("|").contains("12sigma"))
                    || (pChart != NULL))
            {
                // 6Sigma: Horizontal (Y axis)
                if(pChart != NULL)
                {
                    cPenColor = pChart->sigma12Color(bIsMultiLayer);
                    cPenWidth = pChart->sigma12LineWidth();
                }
                else
                {
                    // Check if index is valid
                    CGexSingleChart	*pLayerStyle = NULL;
                    if ((iGroup-1) >= 0 && (iGroup-1) < m_pReportOptions->pLayersStyleList.count())
                    {
                        pLayerStyle = m_pReportOptions->pLayersStyleList.at(iGroup-1);
                        cPenColor = pLayerStyle->sigma12Color(bIsMultiLayer);
                        cPenWidth = pLayerStyle->sigma12LineWidth();

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

                    addMarker(m_pXYChart->yAxis(), lfMeanY-6*lfSigmaY, nLineColor, "-6s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);
                    addMarker(m_pXYChart->yAxis(), lfMeanY+6*lfSigmaY, nLineColor, "+6s", cPenWidth, Chart::BottomLeft)->setFontColor(nTextColor);

                    // 6Sigma: Vertical (X axis)
                    addMarker(m_pXYChart->xAxis(), lfMeanX-6*lfSigmaX, nLineColor, "-6s", cPenWidth, Chart::TopRight)->setFontColor(nTextColor);
                    addMarker(m_pXYChart->xAxis(), lfMeanX+6*lfSigmaX, nLineColor, "+6s", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);

                    internalChartInfoX.m_dMarkerMin = gex_min(internalChartInfoX.m_dMarkerMin, lfMeanX - 6 * lfSigmaX);
                    internalChartInfoX.m_dMarkerMax = gex_max(internalChartInfoX.m_dMarkerMax, lfMeanX + 6 * lfSigmaX);

                    internalChartInfoY.m_dMarkerMin = gex_min(internalChartInfoY.m_dMarkerMin, lfMeanY - 6 * lfSigmaY);
                    internalChartInfoY.m_dMarkerMax = gex_max(internalChartInfoY.m_dMarkerMax, lfMeanY + 6 * lfSigmaY);
                }
            }
            if(acm.split("|").contains("quartile_q1") && pChart != NULL)
            {
                 lfQuartileQ1X = fCustomScaleFactorX*ptTestCellX->lfSamplesQuartile1;
                 lfQuartileQ1Y = fCustomScaleFactorY*ptTestCellY->lfSamplesQuartile1;
                 pFileX->FormatTestResultNoUnits(&lfQuartileQ1X,ptTestCellX->res_scal);
                 pFileY->FormatTestResultNoUnits(&lfQuartileQ1Y,ptTestCellY->res_scal);

                // 6Sigma: Horizontal (Y axis)
                if(pChart != NULL)
                {
                    cPenColor = pChart->quartileQ1Color(bIsMultiLayer);
                    cPenWidth =  (pChart->bVisible) ? pChart->quartileQ1LineWidth() : 0;
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
                        cPenColor = pLayerStyle->quartileQ1Color(bIsMultiLayer);
                        cPenWidth = pLayerStyle->quartileQ1LineWidth();

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

                    addMarker(m_pXYChart->xAxis(), lfQuartileQ1X, nLineColor, "X: Q1", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);
                    addMarker(m_pXYChart->yAxis(), lfQuartileQ1Y, nLineColor, "Y: Q1", cPenWidth, Chart::BottomLeft)->setFontColor(nTextColor);
                }
            }
            if((acm.split("|").contains("quartile_q3"))
                && (pChart != NULL))
            {
                 lfQuartileQ3X = fCustomScaleFactorX*ptTestCellX->lfSamplesQuartile3;
                 lfQuartileQ3Y = fCustomScaleFactorY*ptTestCellY->lfSamplesQuartile3;
                 pFileX->FormatTestResultNoUnits(&lfQuartileQ3X,ptTestCellX->res_scal);
                 pFileY->FormatTestResultNoUnits(&lfQuartileQ3Y,ptTestCellY->res_scal);

                // 6Sigma: Horizontal (Y axis)
                if(pChart != NULL)
                {
                    cPenColor = pChart->quartileQ3Color(bIsMultiLayer);
                    cPenWidth = (pChart->bVisible) ? pChart->quartileQ3LineWidth() : 0 ;
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
                        cPenColor = pLayerStyle->quartileQ3Color(bIsMultiLayer);
                        cPenWidth = pLayerStyle->quartileQ3LineWidth();

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

                    addMarker(m_pXYChart->xAxis(), lfQuartileQ3X, nLineColor, "X: Q3", cPenWidth, Chart::TopLeft)->setFontColor(nTextColor);
                    addMarker(m_pXYChart->yAxis(), lfQuartileQ3Y, nLineColor, "Y: Q3", cPenWidth, Chart::BottomLeft)->setFontColor(nTextColor);
                }
            }

        }

        // If layer visible: Write Custom markers defined thru the scripting interface (if a marker is sticky to a layer, it
        if(bVisible)
        {
            if(m_pChartOverlays)
            {
                CGexSingleChart *pChartLayer;
                pChartLayer = m_pChartOverlays->chartsList().at(iLayerIndex-1);
                if(pChartLayer && pChartLayer->bVisible)
                {
                    // Write Custom X markers defied thru the scripting interface
                    plotScriptingCustomMarkers(ptTestCellX, pChartLayer->cColor.rgb() & 0xffffff, iLayerIndex, fCustomScaleFactorX, true, internalChartInfoX.m_dMarkerMin, internalChartInfoX.m_dMarkerMax);

                    // Write Custom Y markers defied thru the scripting interface
                    plotScriptingCustomMarkers(ptTestCellY, pChartLayer->cColor.rgb() & 0xffffff, iLayerIndex, fCustomScaleFactorY, false, internalChartInfoY.m_dMarkerMin, internalChartInfoY.m_dMarkerMax);
                }
            }
            else
            {
                // Write Custom X markers defied thru the scripting interface
                plotScriptingCustomMarkers(ptTestCellX, cLayerColor.rgb() & 0xffffff, iGroup, fCustomScaleFactorX, true, internalChartInfoX.m_dMarkerMin, internalChartInfoX.m_dMarkerMin);

                // Write Custom Y markers defied thru the scripting interface
                plotScriptingCustomMarkers(ptTestCellY, cLayerColor.rgb() & 0xffffff, iGroup, fCustomScaleFactorY, false, internalChartInfoY.m_dMarkerMin, internalChartInfoY.m_dMarkerMin);
            }
        }

        // Check if charting window is test samples or test limits.
        switch(viewportMode())
        {
            case viewportOverLimits : // Chart over limits samples!
                if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    lfChartBottomX = gex_min(lfLowLX, lfChartBottomX);		// X Low limit exists
                else
                {
                    if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                        lfChartBottomX = gex_min(lfHighLX, lfChartBottomX);

                    lfChartBottomX = gex_min(lfDataStartX, lfChartBottomX);
                }

                if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    lfChartBottomY = gex_min(lfLowLY, lfChartBottomY);		// Y Low limit exists
                else
                {
                    if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                        lfChartBottomY = gex_min(lfHighLY, lfChartBottomY);

                    lfChartBottomY = gex_min(lfDataStartY, lfChartBottomY);
                }

                if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    lfChartTopX = gex_max(lfHighLX, lfChartTopX);		// X High limit exists
                else
                {
                    if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                        lfChartTopX = gex_max(lfLowLX, lfChartTopX);

                    lfChartTopX = gex_max(lfDataEndX, lfChartTopX);
                }

                if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    lfChartTopY = gex_max(lfHighLY, lfChartTopY);		// Y High limit exists
                else
                {
                    if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                        lfChartTopY = gex_max(lfLowLY, lfChartTopY);

                    lfChartTopY = gex_max(lfDataEndY, lfChartTopY);
                }
                break;

            case viewportOverData : // Chart over data samples!
                lfChartTopX		= gex_max(lfDataEndX,	lfChartTopX);
                lfChartBottomX	= gex_min(lfDataStartX,	lfChartBottomX);
                lfChartTopY		= gex_max(lfDataEndY,	lfChartTopY);
                lfChartBottomY	= gex_min(lfDataStartY,	lfChartBottomY);
                break;

            case viewportAdaptive :
                if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    lfChartBottomX = gex_min(lfLowLX, lfChartBottomX); 	// X Low limit exists
                else if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    lfChartBottomX = gex_min(lfHighLX, lfChartBottomX);

                if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    lfChartBottomY = gex_min(lfLowLY,lfChartBottomY);		// Y Low limit exists
                else if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    lfChartBottomY = gex_min(lfHighLY, lfChartBottomY);

                if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    lfChartTopX = gex_max(lfHighLX, lfChartTopX);		// X High limit exists
                else if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    lfChartTopX = gex_max(lfLowLX, lfChartTopX);

                if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    lfChartTopY = gex_max(lfHighLY, lfChartTopY);		// Y High limit exists
                else if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    lfChartTopY = gex_max(lfLowLY, lfChartTopY);

                lfChartTopX		= gex_max(lfDataEndX,	lfChartTopX);
                lfChartBottomX	= gex_min(lfDataStartX,	lfChartBottomX);
                lfChartTopY		= gex_max(lfDataEndY,	lfChartTopY);
                lfChartBottomY	= gex_min(lfDataStartY,	lfChartBottomY);

                if (internalChartInfoX.m_dMarkerMin != C_INFINITE)
                    lfChartBottomX = gex_min(lfChartBottomX, internalChartInfoX.m_dMarkerMin);

                if (internalChartInfoX.m_dMarkerMax != -C_INFINITE)
                    lfChartTopX = gex_max(lfChartTopX, internalChartInfoX.m_dMarkerMax);

                if (internalChartInfoY.m_dMarkerMin != C_INFINITE)
                    lfChartBottomY = gex_min(lfChartBottomY, internalChartInfoY.m_dMarkerMin);

                if (internalChartInfoY.m_dMarkerMax != -C_INFINITE)
                    lfChartTopY = gex_max(lfChartTopY, internalChartInfoY.m_dMarkerMax);

                break;

        }

        // Determine min and max values for the viewport on X axis
        if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            lfViewportLowX = gex_min(lfViewportLowX, lfLowLX);
        else if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            lfViewportLowX = gex_min(lfViewportLowX, lfHighLX);

        lfViewportLowX = gex_min(lfViewportLowX, lfDataStartX);

        if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            lfViewportHighX = gex_max(lfViewportHighX, lfHighLX);
        else if((ptTestCellX->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            lfViewportHighX = gex_max(lfViewportHighX, lfLowLX);

        lfViewportHighX = gex_max(lfViewportHighX, lfDataEndX);

        // Determine min and max values for the viewport on Y axis
        if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            lfViewportLowY = gex_min(lfViewportLowY, lfLowLY);
        else if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            lfViewportLowY = gex_min(lfViewportLowY, lfHighLY);

        lfViewportLowY = gex_min(lfViewportLowY, lfDataStartY);

        if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            lfViewportHighY = gex_max(lfViewportHighY, lfHighLY);
        else if((ptTestCellY->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            lfViewportHighY = gex_max(lfViewportHighY, lfLowLY);

        lfViewportHighY = gex_max(lfViewportHighY, lfDataEndY);

        if (internalChartInfoX.m_dMarkerMin != C_INFINITE)
            lfViewportLowX = gex_min(lfViewportLowX, internalChartInfoX.m_dMarkerMin);

        if (internalChartInfoX.m_dMarkerMax != -C_INFINITE)
            lfViewportHighX = gex_max(lfViewportHighX, internalChartInfoX.m_dMarkerMax);

        if (internalChartInfoY.m_dMarkerMin != C_INFINITE)
            lfViewportLowY = gex_min(lfViewportLowY, internalChartInfoY.m_dMarkerMin);

        if (internalChartInfoY.m_dMarkerMax != -C_INFINITE)
            lfViewportHighY = gex_max(lfViewportHighY, internalChartInfoY.m_dMarkerMax);

        if (internalChartInfoX.m_dLowSpecLimit != C_INFINITE
            && internalChartInfoX.m_dLowSpecLimit < F_INFINITE
            && internalChartInfoX.m_dLowSpecLimit != -C_INFINITE
            && internalChartInfoX.m_dLowSpecLimit > -F_INFINITE)
            lfViewportLowX  = gex_min(lfViewportLowX , internalChartInfoX.m_dLowSpecLimit);

        if (internalChartInfoX.m_dHighSpecLimit != C_INFINITE
                && internalChartInfoX.m_dHighSpecLimit < F_INFINITE
                && internalChartInfoX.m_dHighSpecLimit != -C_INFINITE
                && internalChartInfoX.m_dHighSpecLimit > -F_INFINITE)
            lfViewportHighX = gex_max(lfViewportHighX, internalChartInfoX.m_dHighSpecLimit);

        if (internalChartInfoY.m_dLowSpecLimit != C_INFINITE
                && internalChartInfoY.m_dLowSpecLimit < F_INFINITE
                && internalChartInfoY.m_dLowSpecLimit != -C_INFINITE
                && internalChartInfoY.m_dLowSpecLimit > -F_INFINITE)
            lfViewportLowY  = gex_min(lfViewportLowY , internalChartInfoY.m_dLowSpecLimit);

        if (internalChartInfoY.m_dHighSpecLimit != C_INFINITE
                && internalChartInfoY.m_dHighSpecLimit < F_INFINITE
                && internalChartInfoY.m_dHighSpecLimit != -C_INFINITE
                && internalChartInfoY.m_dHighSpecLimit > -F_INFINITE)
            lfViewportHighY = gex_max(lfViewportHighY, internalChartInfoY.m_dHighSpecLimit);

        lfViewportLowX = gex_min(lfViewportLowX, lfMedianX);
        lfViewportLowX = gex_min(lfViewportLowX, lfMeanX);
        lfViewportLowX = gex_min(lfViewportLowX, lfMeanX - 6 * lfSigmaX);

        lfViewportHighX = gex_max(lfViewportHighX, lfMedianX);
        lfViewportHighX = gex_max(lfViewportHighX, lfMeanX);
        lfViewportHighX = gex_max(lfViewportHighX, lfMeanX + 6 * lfSigmaX);

        lfViewportLowY = gex_min(lfViewportLowY, lfMedianY);
        lfViewportLowY = gex_min(lfViewportLowY, lfMeanY);
        lfViewportLowY = gex_min(lfViewportLowY, lfMeanY - 6 * lfSigmaY);

        lfViewportHighY = gex_max(lfViewportHighY, lfMedianY);
        lfViewportHighY = gex_max(lfViewportHighY, lfMeanY);
        lfViewportHighY = gex_max(lfViewportHighY, lfMeanY + 6 * lfSigmaY);

NextGroup:

        if(m_pChartOverlays == NULL)
        {
            ++itGroupList;

            pGroup = (itGroupList != gexReport->getGroupsList().end()) ? (*itGroupList) : NULL;

            // Keep track of group index processed.
            iGroup++;
        }

    };	// Read trend of a given test in all groups so to stack all charts.

    // X axis scale. Add 1% of scale over and under chart window...
    // to ensure markers will be seen
    lfExtraX = (lfChartTopX-lfChartBottomX)*0.01;

    // Y axis scale. Add 1% of scale over and under chart window...
    // to ensure markers will be seen
    lfExtraY = (lfChartTopY-lfChartBottomY)*0.01;

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
                lfExtraX		= 0;
                lfChartBottomX	= referenceTestX()->ptChartOptions->lowX();
                lfChartTopX		= referenceTestX()->ptChartOptions->highX();
            }
        }

        // If custom Y viewport (in scripting file, not in interactive mode), overwrite defaults window viewing
        if(referenceTestY()->ptChartOptions != NULL)
        {
            // If custom viewport in Y, overwrite defaults window viewing
            if(referenceTestY()->ptChartOptions->customViewportX())
            {
                lfExtraY		= 0;
                lfChartBottomY	= referenceTestY()->ptChartOptions->lowX();
                lfChartTopY		= referenceTestY()->ptChartOptions->highX();
            }
        }
    }
    else if (m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].bForceViewport)
    {
        // Customer force viewport limit
        resetViewportManager(false);

        if (m_pChartOverlays->getViewportRectangle()[type()].lfLowX != -C_INFINITE)
            lfChartBottomX	= m_pChartOverlays->getViewportRectangle()[type()].lfLowX;

        if (m_pChartOverlays->getViewportRectangle()[type()].lfHighX != C_INFINITE)
            lfChartTopX		= m_pChartOverlays->getViewportRectangle()[type()].lfHighX;

        if (m_pChartOverlays->getViewportRectangle()[type()].lfLowY != -C_INFINITE)
            lfChartBottomY	= m_pChartOverlays->getViewportRectangle()[type()].lfLowY;

        if (m_pChartOverlays->getViewportRectangle()[type()].lfHighY != C_INFINITE)
            lfChartTopY		= m_pChartOverlays->getViewportRectangle()[type()].lfHighY;

        lfExtraX		= 0;
        lfExtraY		= 0;

        m_pChartOverlays->getViewportRectangle()[type()].bForceViewport = false;
    }

    // Define scales: if in Interactive charting, consider zoomin factor
    if(m_pChartOverlays == NULL || m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX == false)
    {
        if (lfChartBottomX != C_INFINITE && lfChartTopX != C_INFINITE)
        {
            lfChartBottomX	-= lfExtraX;
            lfChartTopX		+= lfExtraX;
        }
    }

    if (m_pChartOverlays == NULL || m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY == false)
    {
        if (lfChartBottomY != C_INFINITE && lfChartTopY != C_INFINITE)
        {
            lfChartBottomY	-= lfExtraY;
            lfChartTopY		+= lfExtraY;
        }
    }

    lfViewportLowX	= gex_min(lfViewportLowX, lfChartBottomX);
    lfViewportHighX	= gex_max(lfViewportHighX, lfChartTopX);
    lfViewportLowY	= gex_min(lfViewportLowY, lfChartBottomY);
    lfViewportHighY	= gex_max(lfViewportHighY, lfChartTopY);


    // Define viewport: if in Interactive charting, consider zoomin factor
    if (m_pChartOverlays)
    {
        // Initialize viewport
        if (isViewportInitialized() == false)
        {
            if (lfChartBottomX != C_INFINITE && lfChartTopX != C_INFINITE)
                initHorizontalViewport(lfViewportLowX, lfViewportHighX, lfChartBottomX, lfChartTopX, 0.05);

            if (lfChartBottomY != C_INFINITE && lfChartTopY != C_INFINITE)
                initVerticalViewport(lfViewportLowY, lfViewportHighY, lfChartBottomY, lfChartTopY, 0.05);
        }

        // Using ViewPortLeft and ViewPortWidth, get the start and end dates of the view port.
        computeXBounds(lfChartBottomX, lfChartTopX);
        computeYBounds(lfChartBottomY, lfChartTopY);
    }

    if (lfChartBottomX != C_INFINITE && lfChartTopX != C_INFINITE)
    {
        // Check if scale type is: Linear or Logarithmic
        if((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX &&
            ( m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeScatter || m_pChartOverlays->getAppliedToChart() == -1)) || m_iXScaleType == 1) {
            // Log scale: then force it
            m_pXYChart->xAxis()->setLogScale(lfChartBottomX, lfChartTopX, (bShowAxisAndGrid) ? 0 : Chart::NoValue);
            m_iXScaleType = 1;
        } else if((m_pChartOverlays && !m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX &&
               ( m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeScatter || m_pChartOverlays->getAppliedToChart() == -1)) || m_iXScaleType == 0){
            m_pXYChart->xAxis()->setLinearScale(lfChartBottomX, lfChartTopX, (bShowAxisAndGrid) ? 0 : Chart::NoValue);
            m_iXScaleType = 0;
        }
    }

    if (lfChartBottomY != C_INFINITE && lfChartTopY != C_INFINITE)
    {
        // Check if scale type is: Linear or Logarithmic
        if((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
            (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeScatter || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 1){
            // Log scale: then force it
            m_pXYChart->yAxis()->setLogScale(lfChartBottomY, lfChartTopY, (bShowAxisAndGrid) ? 0 : Chart::NoValue);
            m_iYScaleType = 1;
        } else if((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
               (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeScatter || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 0){
            m_pXYChart->yAxis()->setLinearScale(lfChartBottomY, lfChartTopY, (bShowAxisAndGrid) ? 0 : Chart::NoValue);
            m_iYScaleType = 0;
        }
    }

    if (m_pChartOverlays)
    {
        m_pChartOverlays->getViewportRectangle()[type()].lfLowX	= lfChartBottomX;
        m_pChartOverlays->getViewportRectangle()[type()].lfLowY	= lfChartBottomY;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighX	= lfChartTopX;
        m_pChartOverlays->getViewportRectangle()[type()].lfHighY	= lfChartTopY;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString makeTooltip()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
#include "gex_algorithms.h"
bool GexScatterChart::findRunId(const int &iIdx, QString &strRunX, QString &strRunY){

        CGexFileInGroup *	pFileX  = m_listFileX[iIdx];
        CGexFileInGroup *	pFileY  = m_listFileY[iIdx];
        CTest *             poTestX = m_listTestX[iIdx];
        CTest *             poTestY = m_listTestY[iIdx];

        int     iCustomScaleFactorX = poTestX->res_scal -  m_listTestX[0]->res_scal;
        double  lfScaleFactorX      = 1/GS_POW(10.0,iCustomScaleFactorX);

        int     iCustomScaleFactorY = poTestY->res_scal - m_listTestY[0]->res_scal;
        double  lfScaleFactorY      = 1/GS_POW(10.0,iCustomScaleFactorY);

        // Find begin and end offset of the sublot
        int     lBegin  = 0;
        int     lEnd    = 0;

        // Retrieve upper and lower run index in the test result
        // array for this pFile
        poTestX->findSublotOffset(lBegin, lEnd, iIdx);

        for(int iVal = lBegin; iVal < lEnd; iVal++)
        {
            if (poTestX->m_testResult.isValidIndex(iVal) && poTestX->m_testResult.isValidResultAt(iVal) &&
                    poTestY->m_testResult.isValidIndex(iVal) && poTestY->m_testResult.isValidResultAt(iVal))
            {
                double dValX = algorithms::gexRound(poTestX->m_testResult.resultAt(iVal) * lfScaleFactorX, 10);
                double dValY = algorithms::gexRound(poTestY->m_testResult.resultAt(iVal) * lfScaleFactorY, 10);

                if( dValX == algorithms::gexRound(m_dXHotSpotValue, 10)
                        &&  dValY == algorithms::gexRound(m_dYHotSpotValue, 10))
                {
                    // Compute index of the part inside the current pfile.
                    int lPartInfoIdx = iVal - lBegin;

                    if (lPartInfoIdx >= 0 &&
                            lPartInfoIdx < pFileX->pPartInfoList.count() &&
                            lPartInfoIdx < pFileY->pPartInfoList.count())
                    {
                        strRunX = QString::number(pFileX->findRunNumber(pFileX->pPartInfoList.at(lPartInfoIdx)->iDieX, pFileX->pPartInfoList.at(lPartInfoIdx)->iDieY, poTestX));
                        strRunY = QString::number(pFileY->findRunNumber(pFileY->pPartInfoList.at(lPartInfoIdx)->iDieX, pFileY->pPartInfoList.at(lPartInfoIdx)->iDieY, poTestY));
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Incorrect index computed in correlation chart");
                    }

                    break;
                }
            }
        }

        if(strRunX.isEmpty() || strRunY.isEmpty())
            return false;
        else
            return true;
}

QString	GexScatterChart::makeTooltip()
{
    QString	strTooltip;
    QString strPartId;

    if (referenceTestX() && isViewportInitialized())
    {
        double		lfCustomScaleFactor=0.;
        QString		strUnitsX;
        QString		strUnitsY;
        QString     scalingOption=m_pReportOptions->GetOption("dataprocessing","scaling").toString();

        // Get X and Y units (if any)
        strUnitsX = referenceTestX()->GetScaledUnits(&lfCustomScaleFactor,
                    scalingOption);
        strUnitsX.truncate(10);

        if(referenceTestY() != NULL)
        {
            strUnitsY = referenceTestY()->GetScaledUnits(&lfCustomScaleFactor,
                        scalingOption);
            strUnitsY.truncate(10);
        }

        if (m_dXHotSpotValue != GEX_C_DOUBLE_NAN || m_dXHotSpotValue != GEX_C_DOUBLE_NAN)
        {
            if(m_dXHotSpotValue != GEX_C_DOUBLE_NAN && m_dYHotSpotValue != GEX_C_DOUBLE_NAN)
            {
                for(int iIdx=0; iIdx < m_listFileX.count(); iIdx++)
                {
                    QString  strPartX, strPartY;
                    findRunId(iIdx,strPartX, strPartY);
                    if(!strPartX.isEmpty() && !strPartY.isEmpty())
                    {
                        if(strPartX != strPartY)
                            strPartId += QString("Run Id X:\t%1\nPart Id Y:\t%2\n").arg(strPartX).arg(strPartY);
                        else
                            strPartId += QString("Run Id :\t%1\n").arg(strPartX);
                    }
                }
            }

            if(strPartId.isEmpty())
                strTooltip.sprintf("X: %12f %s\nY: %12f %s", m_dXHotSpotValue, strUnitsX.toLatin1().constData(), m_dYHotSpotValue, strUnitsY.toLatin1().constData());
            else
                strTooltip.sprintf("X: %12f %s\nY: %12f %s\n%s",
                    m_dXHotSpotValue, strUnitsX.toLatin1().constData(), m_dYHotSpotValue, strUnitsY.toLatin1().constData(), strPartId.toLatin1().constData());
        }

        // If log scale, do not show the data pointed...don't know its location!
        if(m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX || m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY)
            strTooltip = "Plot Area";
    }

    return strTooltip;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setViewportModeFromChartMode(int nChartMode)
//
// Description	:	convert scatter chart mode from Gex into viewport mode
//
///////////////////////////////////////////////////////////////////////////////////
void GexScatterChart::setViewportModeFromChartMode(int nChartMode)
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
// Name			:	void chartingLineStyle(CGexSingleChart * pLayerStyle)
//
// Description	:	Update some characteristics
//
///////////////////////////////////////////////////////////////////////////////////
void GexScatterChart::chartingLineStyle(CGexSingleChart * pLayerStyle)
{
    if (pLayerStyle)
        pLayerStyle->bSpots = true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void keepXTestViewport()
//
// Description	:	Keep wiewport for test X
//
///////////////////////////////////////////////////////////////////////////////////
void GexScatterChart::keepXTestViewport()
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
void GexScatterChart::keepYTestViewport()
{
    if (m_pChartOverlays && referenceTestY())
    {
        referenceTestY()->ptChartOptions->setCustomViewportX(true);
        referenceTestY()->ptChartOptions->setLowX(m_pChartOverlays->getViewportRectangle()[type()].lfLowX);
        referenceTestY()->ptChartOptions->setHighX(m_pChartOverlays->getViewportRectangle()[type()].lfHighX);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void useTestCustomViewport()
//
// Description	:	test if we can use the custom viewport
//
///////////////////////////////////////////////////////////////////////////////////
bool GexScatterChart::useTestCustomViewport()
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
void GexScatterChart::resetViewportManager(bool bEraseCustomViewport)
{
    if (bEraseCustomViewport && referenceTestY() && referenceTestY()->ptChartOptions)
    {
        referenceTestY()->ptChartOptions->setCustomViewportX(false);
        referenceTestY()->ptChartOptions->setCustomViewportY(false);
    }

    GexAbstractChart::resetViewportManager(bEraseCustomViewport);
}

void GexScatterChart::computeDataset()
{
    if (!mIsLinearRegressionEnabled)
        return;

    CTest *lRefTestX = referenceTestX();
    CTest *lRefTestY = referenceTestY();
    if (!lRefTestX || !lRefTestY)
        return;

    GSLOG(SYSLOG_SEV_DEBUG, QString("Computing scatter chart dataset between test %1 and %2...")
          .arg( lRefTestX->lTestNumber).arg(lRefTestY->lTestNumber).toLatin1().constData());

    CTest *	lMatchingTestX = NULL;
    CTest *	lMatchingTestY = NULL;
    int		lTestNumberX = 0;
    int		lPinmapIndexX = 0;
    int		lTestNumberY = 0;
    int		lPinmapIndexY = 0;
    CGexFileInGroup *	lFileInGroup = NULL;
    QList<CGexGroupOfFiles*>::iterator	lGroupIter = gexReport->getGroupsList().begin();
    CGexGroupOfFiles *lGroup = (lGroupIter != gexReport->getGroupsList().end()) ? (*lGroupIter) : NULL;
    QString lParameterNameX;
    QString lParameterNameY;

    // Compute linear reagression for each group
    while(lGroup)
    {
        lTestNumberX = lRefTestX->lTestNumber;
        lParameterNameX = lRefTestX->strTestName;

        if(referenceTestX()->lPinmapIndex >= 0)
            lPinmapIndexX = lRefTestX->lPinmapIndex;
        else
            lPinmapIndexX = GEX_PTEST;

        lTestNumberY = lRefTestY->lTestNumber;
        lParameterNameY = lRefTestY->strTestName;

        if(referenceTestY()->lPinmapIndex >= 0)
            lPinmapIndexY = lRefTestY->lPinmapIndex;
        else
            lPinmapIndexY = GEX_PTEST;

        lMatchingTestX = lMatchingTestY = NULL;
        lFileInGroup = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

        if((lFileInGroup->FindTestCell(lTestNumberX,lPinmapIndexX,&lMatchingTestX,false,false,lParameterNameX) ==1) &&
                (lFileInGroup->FindTestCell(lTestNumberY,lPinmapIndexY,&lMatchingTestY,false,false,lParameterNameY) == 1))
        {
            computeLinearRegression(lMatchingTestX, lMatchingTestY);
        }

        ++lGroupIter;
        lGroup = (lGroupIter != gexReport->getGroupsList().end()) ? (*lGroupIter) : NULL;
    };
}

QString GexScatterChart::computeLinearRegression(CTest *aTestX, CTest *aTestY)
{
    if (!aTestX || !aTestY)
        return "error : X and/or Y test null";

    QPair< CTest*, CTest* > lTestPair;
    lTestPair.first = aTestX;
    lTestPair.second = aTestY;

    if (sLinearRegressionResults.find(lTestPair) != sLinearRegressionResults.end())
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Linear regression between test %1 and test %2 already computed.")
              .arg( aTestX->lTestNumber).arg(aTestY->lTestNumber).toLatin1().constData());
        return "ok : already computed";
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Computing linear regression between test %1 and %2...")
          .arg( aTestX->lTestNumber).arg( aTestY->lTestNumber).toLatin1().constData());

    GS::StdLib::LinearRegression lLinearReg;
    double x = 0.;
    double y = 0.;
    // for all test results available in both dataset

    int iTotalSamples = gex_min(aTestX->m_testResult.count(), aTestY->m_testResult.count());

    if (iTotalSamples <= 0)
        return "error : not enough samples";

    // resultant scale factors
    double lfScaleFactorX = 1, lfScaleFactorY = 1;

    int lNumOfValidResults = 0;
    double minx = std::numeric_limits<double>::max();
    double maxx = -1 * std::numeric_limits<double>::max();

    // Copy the data
    for(int i=0; i<iTotalSamples; i++)
    {
        if (!aTestX->m_testResult.isValidResultAt(i))
            continue;
        x = aTestX->m_testResult.resultAt(i) * lfScaleFactorX;
        if (minx > x)
            minx = x;
        if (maxx < x)
            maxx = x;
        if (!aTestY->m_testResult.isValidResultAt(i))
            continue;
        y = aTestY->m_testResult.resultAt(i) * lfScaleFactorY;
        lLinearReg.addXY(x, y);
        ++lNumOfValidResults;
    }

    if (lNumOfValidResults<2)
        return "error : not enough valid result";

    QMap< QString, QVariant > lRegParam;
    lRegParam.insert("Correlation", lLinearReg.getCoefCorrel() );
    lRegParam.insert("Determination", lLinearReg.getCoefDeterm() );
    lRegParam.insert("StdErrorEstimation", lLinearReg.getStdErrorEst() );
    lRegParam.insert("MinX", minx);
    lRegParam.insert("MinY", lLinearReg.estimateY(minx));
    lRegParam.insert("MaxX", maxx);
    lRegParam.insert("MaxY", lLinearReg.estimateY(maxx) );
    lRegParam.insert("A", lLinearReg.getA() );
    lRegParam.insert("B", lLinearReg.getB() );
    lRegParam.insert("NumOfValidResults", lNumOfValidResults);

    GSLOG(SYSLOG_SEV_DEBUG, QString("Linear regression computed on %1 results : cor:%2 det:%3 err:%4 min:%5,%6 max=%7.arg(%8")
          .arg(lNumOfValidResults)
          .arg(lLinearReg.getCoefCorrel())
          .arg(lLinearReg.getCoefDeterm())
          .arg(lLinearReg.getStdErrorEst())
          .arg(minx)
          .arg(lLinearReg.estimateY(minx))
          .arg(maxx)
          .arg(lLinearReg.estimateY(maxx)).toLatin1().constData());

    sLinearRegressionResults.insert(lTestPair, lRegParam);
    return "ok";
}

//static
bool GexScatterChart::getLinearRegressionResults(CTest* lTX, CTest* lTY, QMap<QString, QVariant> &lrd)
{
    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("a=%1 b=%2").arg( a, b));.arg(    //GSLOG(SYSLOG_SEV_INFORMATIONAL, "a=%1 b=%2").arg( a.arg( b);
    if (!lTX || !lTY)
        return false; //"error : X and/or Y test null";

    QPair< CTest*, CTest* > p; p.first=lTX; p.second=lTY;
    if (sLinearRegressionResults.find(p)==sLinearRegressionResults.end() )
    {
        p.first=lTY; p.second=lTX;
        if (sLinearRegressionResults.find(p)!=sLinearRegressionResults.end())
            lrd=sLinearRegressionResults.value(p);
    }
    else
        lrd=sLinearRegressionResults.value(p);

    return true;
}

bool GexScatterChart::getLinearRegressionResults(CTest* lTX, CTest* lTY, float &a, float &b)
{
    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("a=%1 b=%2").arg( a, b));.arg(    //GSLOG(SYSLOG_SEV_INFORMATIONAL, "a=%1 b=%2").arg( a.arg( b);
    if (!lTX || !lTY)
        return false; //"error : X and/or Y test null";

    QMap<QString, QVariant> lrd;
    QPair< CTest*, CTest* > p; p.first=lTX; p.second=lTY;
    if (sLinearRegressionResults.find(p)==sLinearRegressionResults.end() )
    {
        p.first=lTY; p.second=lTX;
        if (sLinearRegressionResults.find(p)!=sLinearRegressionResults.end())
            lrd=sLinearRegressionResults.value(p);
    }
    else
        lrd=sLinearRegressionResults.value(p);

    if (lrd.isEmpty()) // Not found
    {
        return false;
    }

    a=lrd.value("A").toFloat(); b=lrd.value("B").toFloat();

    return true;
}

void GexScatterChart::customizeChart()
{
    if (!mIsLinearRegressionEnabled)
        return;

    double lOrigx = m_pXYChart->xAxis()->getMinValue();
    // Draw diagonal
    double lWidth = m_pXYChart->xAxis()->getMaxValue() - m_pXYChart->xAxis()->getMinValue();
    drawCustomLine(m_pXYChart->getXCoor(lOrigx), m_pXYChart->getYCoor(lOrigx),
                    m_pXYChart->getXCoor(m_pXYChart->xAxis()->getMaxValue()),
                    m_pXYChart->getYCoor(lOrigx + lWidth),
                    m_pXYChart->dashLineColor(0x999999, Chart::DotDashLine),
                    1);

    CTest *lRefTestX = referenceTestX();
    CTest *lRefTestY = referenceTestY();
    if (!lRefTestX || !lRefTestY)
        return;

    CTest *	lMatchingTestX = NULL;
    CTest *	lMatchingTestY = NULL;
    int	lTestNumberX = 0;
    int	lPinmapIndexX = 0;
    int	lTestNumberY = 0;
    int	lPinmapIndexY = 0;
    int lLayerIndex = 1;
    CGexFileInGroup *lFileInGroup = NULL;
    QList<CGexGroupOfFiles*>::iterator lGroupIter = gexReport->getGroupsList().begin();
    CGexGroupOfFiles *lGroup = (lGroupIter != gexReport->getGroupsList().end()) ? (*lGroupIter) : NULL;
    QString lParameterNameX;
    QString lParameterNameY;

    // For each group, get linear regression and draw trend line
    while(lGroup)
    {
        lTestNumberX = lRefTestX->lTestNumber;
        lParameterNameX = lRefTestX->strTestName;

        if(referenceTestX()->lPinmapIndex >= 0)
            lPinmapIndexX = lRefTestX->lPinmapIndex;
        else
            lPinmapIndexX = GEX_PTEST;

        lTestNumberY = lRefTestY->lTestNumber;
        lParameterNameY = lRefTestY->strTestName;

        if(referenceTestY()->lPinmapIndex >= 0)
            lPinmapIndexY = lRefTestY->lPinmapIndex;
        else
            lPinmapIndexY = GEX_PTEST;

        lMatchingTestX = lMatchingTestY = NULL;
        lFileInGroup = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

        if((lFileInGroup->FindTestCell(lTestNumberX,lPinmapIndexX,&lMatchingTestX,false,false,lParameterNameX) ==1) &&
                (lFileInGroup->FindTestCell(lTestNumberY,lPinmapIndexY,&lMatchingTestY,false,false,lParameterNameY) == 1))
        {
            QMap<QString, QVariant> lLinearReg;
            bool lIsLinearRegFound = getLinearRegressionResults(lMatchingTestX, lMatchingTestY, lLinearReg);
            if (lIsLinearRegFound)
            {
                int nFillColor=-1, nLineColor=0;
                Chart::SymbolType layerSymbol;
                getLayerStyles(lLayerIndex, NULL, nFillColor, nLineColor, layerSymbol);
                DrawTrendLine(lLinearReg, nLineColor);
            }
        }

        ++lGroupIter;
        ++lLayerIndex;
        lGroup = (lGroupIter != gexReport->getGroupsList().end()) ? (*lGroupIter) : NULL;
    };
}

void GexScatterChart::DrawTrendLine(const QMap<QString, QVariant> &aLinearReg, int aLineColor)
{
    double lAValue = aLinearReg.value("A").toDouble();
    double lBValue = aLinearReg.value("B").toDouble();
    double lOrigx = m_pXYChart->xAxis()->getMinValue();
    double lTopx = m_pXYChart->xAxis()->getMaxValue();

    drawCustomLine( m_pXYChart->getXCoor(lOrigx),
                    m_pXYChart->getYCoor(lBValue * lOrigx + lAValue),
                    m_pXYChart->getXCoor(lTopx),
                    m_pXYChart->getYCoor(lBValue * lTopx + lAValue),
                    aLineColor,
                    1);
}


