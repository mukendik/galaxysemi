///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_xbar_r_chart.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "report_options.h"

#include "limits"

///////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport *		gexReport;
extern CReportOptions	ReportOptions;

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern double	ScalingPower(int iPower);

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAbstractControlChart
//
// Description	:	chart class to draw the control charts into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAbstractControlChart::GexAbstractControlChart(GexAbstractChart::chartType eChartType, int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays /* = NULL */)
    : GexAbstractChart(eChartType, nSizeMode, lWizardParent, pChartOverlays),
      m_pXbarRDataset(NULL)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAbstractControlChart::~GexAbstractControlChart()
{
//	if (m_pXbarRDataset)
//	{
//		delete m_pXbarRDataset;
//		m_pXbarRDataset = NULL;
//	}
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
QString GexAbstractControlChart::makeTooltip()
{
    QString		strTooltip;

    if (referenceTestX() && m_pXbarRDataset && isViewportInitialized())
    {
        double		lfCustomScaleFactor;

        // Get units (if any)
        QString strUnits = referenceTestX()->GetScaledUnits(&lfCustomScaleFactor, ReportOptions.GetOption("dataprocessing","scaling").toString());
        strUnits.truncate(10);

        if (m_dXHotSpotValue != GEX_C_DOUBLE_NAN && m_dXHotSpotValue != GEX_C_DOUBLE_NAN)
        {
            int					nGroup = m_nDataItem / m_pXbarRDataset->deviceCount();
            CGexGroupOfFiles *	pGroup = (nGroup < gexReport->getGroupsList().count()) ? gexReport->getGroupsList().at(nGroup) : NULL;

            if (pGroup)
            {
                QTextStream txtStream(&strTooltip);

                txtStream << "Appraiser: " << pGroup->strGroupName << endl;
                txtStream << "Device: " << m_pXbarRDataset->XBarRLabelArray()[m_nDataItem] << endl;
                txtStream << "Value: " << m_dYHotSpotValue;

                if (strUnits.isEmpty() == false)
                    txtStream << " " << strUnits;
            }
        }
    }

    return strTooltip;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setViewportModeFromChartMode()
//
// Description	:	convert chart mode from Gex into viewport mode
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractControlChart::setViewportModeFromChartMode(int nChartMode)
{
    Q_UNUSED(nChartMode);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void computeDataset()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractControlChart::computeDataset()
{
    CGexGroupOfFiles *	pGroup	= (gexReport->getGroupsList().isEmpty() == false) ? gexReport->getGroupsList().first() : NULL;
    CGexFileInGroup *	pFile	= (pGroup && pGroup->pFilesList.isEmpty() == false) ? pGroup->pFilesList.first() : NULL;

    if (pFile && referenceTestX())
    {
        if (referenceTestX()->m_pXbarRDataset)
            m_pXbarRDataset = referenceTestX()->m_pXbarRDataset;
        else
            m_pXbarRDataset = NULL;

        // Build the Chart Title
        QString strTestName;
        gexReport->BuildTestNameString(pFile, referenceTestX(), strTestName);
        m_strTitle = QString("Test %1: %2").arg(referenceTestX()->szTestLabel).arg(strTestName);
    }

//	if (m_pXbarRDataset)
//	{
//		delete m_pXbarRDataset;
//		m_pXbarRDataset = NULL;
//	}

//	CGexGroupOfFiles *	pGroup				= (gexReport->getGroupsList().isEmpty() == false) ? gexReport->getGroupsList().first() : NULL;
//	CGexFileInGroup *	pFile				= (pGroup && pGroup->pFilesList.isEmpty() == false) ? pGroup->pFilesList.first() : NULL;
//	QString				strScalingOption	= m_pReportOptions->GetOption("dataprocessing","scaling").toString();

//	if (pFile && referenceTestX())
//	{
//		CTest *			pTest				= NULL;
//		int				nPinmapIndex		= 0;
//		int				nRunIndex			= 0;
//		uint			uiAppraisersCount	= gexReport->getGroupsList().count();
//		uint			uiTrialsCount		= pGroup->pFilesList.count();
//		uint			uiDevicesCount		= referenceTestX()->m_testResult.count() / uiTrialsCount;
//		int				nRefScaleFactor		= 0;
//		int				nCustomScaleFactor	= 0;
//		double			dCustomScaleFactor	= 0.0;

//		QString			strUnits			= pFile->GetTestScaledUnits(referenceTestX(), &dCustomScaleFactor);
//		QVector<double>	vecDataPoint(uiTrialsCount);

//		// Build the Chart Title
//		QString strTestName;
//		gexReport->BuildTestNameString(pFile, referenceTestX(), strTestName);
//		m_strTitle = QString("Test %1: %2").arg(referenceTestX()->szTestLabel).arg(strTestName);

//		// Allocate memory for XBar R dataset
//		m_pXbarRDataset = new GexXBarRDataset(uiTrialsCount, uiDevicesCount, uiAppraisersCount, strUnits);

//		// Calculate XBar per appraiser
//		for(uint uiAppraiser = 0; uiAppraiser < uiAppraisersCount; ++uiAppraiser)
//		{
//			pGroup	= gexReport->getGroupsList().at(uiAppraiser);
//			pFile	= pGroup->pFilesList.first();

//			if(referenceTestX()->lPinmapIndex >= 0)
//				nPinmapIndex = referenceTestX()->lPinmapIndex;
//			else
//				nPinmapIndex = GEX_PTEST;

//			// Find the test cell for the current appraiser
//			if (pFile && pFile->FindTestCell(referenceTestX()->lTestNumber, nPinmapIndex, &pTest, true, false, false, referenceTestX()->strTestName.toLatin1().data()) == 1)
//			{
//				if (uiAppraiser == 0)
//					nRefScaleFactor = pTest->res_scal;

//				nCustomScaleFactor = pTest->res_scal - nRefScaleFactor;
//				dCustomScaleFactor = 1/pow(10.0, nCustomScaleFactor);

//				// Calculate XBar R valuefor each device
//				for(uint uiDevice = 0; uiDevice < uiDevicesCount; ++uiDevice)
//				{
//					vecDataPoint.clear();

//					// Retrieve all trial values for each device
//					for(uint uiTrial = 0; uiTrial < uiTrialsCount; ++uiTrial)
//					{
//						// Compute run index in the test result
//						nRunIndex = uiDevice + (uiTrial * uiDevicesCount);

//						if (pTest->m_testResult.isValidResultAt(nRunIndex))
//							vecDataPoint.append(pTest->m_testResult.resultAt(nRunIndex) * dCustomScaleFactor);
//					}

//					// Add the datapoint for this device to the dataset
//					m_pXbarRDataset->addDataPoint(uiAppraiser, uiDevice, vecDataPoint);
//				}

//				// Set low limit
//				if((pTest->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
//				{
//					double dLimit = pTest->lfLowLimit;		// Low limit exists

//					// If we have to keep values in normalized format, do not rescale!
//					if (strScalingOption != "normalized")
//					{
//						// convert LowLimit to same scale as results:
//						pFile->FormatTestResultNoUnits(&dLimit, pTest->llm_scal);
//						dLimit *=  ScalingPower(pTest->llm_scal);	// normalized
//						dLimit /=  ScalingPower(pTest->res_scal);	// normalized
//					}

//					if(nCustomScaleFactor)
//						dLimit *= dCustomScaleFactor;

//					m_pXbarRDataset->setLowTestLimit(uiAppraiser, dLimit);
//				}

//				// Set High limit
//				if((pTest->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
//				{
//					double dLimit = pTest->lfHighLimit;		// Low limit exists

//					// If we have to keep values in normalized format, do not rescale!
//					if (strScalingOption != "normalized")
//					{
//						// convert LowLimit to same scale as results:
//						pFile->FormatTestResultNoUnits(&dLimit, pTest->hlm_scal);
//						dLimit *=  ScalingPower(pTest->hlm_scal);	// normalized
//						dLimit /=  ScalingPower(pTest->res_scal);	// normalized
//					}

//					if(nCustomScaleFactor)
//						dLimit *= dCustomScaleFactor;

//					m_pXbarRDataset->setHighTestLimit(uiAppraiser, dLimit);
//				}
//			}
//		}

//		// When all data have been added to the dataset, compute the control limits
//		m_pXbarRDataset->computeControlLimits();
//	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void keepXTestViewport()
//
// Description	:	Keep the viewport values for the reference test X
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractControlChart::keepXTestViewport()
{
    // Do nothing, not available on control chart
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXBarChart
//
// Description	:	chart class to draw the XBar charts into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexXBarChart::GexXBarChart(int nSizeMode, GexWizardChart * lWizardParent, CGexChartOverlays * pChartOverlays /* = NULL */)
    : GexAbstractControlChart(chartTypeXBar, nSizeMode, lWizardParent, pChartOverlays)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexXBarChart::~GexXBarChart()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void buildOwnerChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarChart::buildOwnerChart()
{
    if (m_pXbarRDataset)
    {
        GexInternalChartInfo	internalChartInfo;

        LineLayer * pLineLayer	= m_pXYChart->addLineLayer();
        DataSet *	pDataset	= pLineLayer->addDataSet(DoubleArray(m_pXbarRDataset->XBarDataArray(), m_pXbarRDataset->dataCount()));

        pDataset->setDataSymbol(Chart::CircleSymbol, 5, QColor(Qt::red).rgb() & 0xffffff, QColor(Qt::black).rgb() & 0xffffff);

        m_pXYChart->xAxis()->setLabelStyle("", 7, TextColor, 90.0);
        m_pXYChart->xAxis()->setLabels(DoubleArray(m_pXbarRDataset->XBarRLabelArray(), m_pXbarRDataset->dataCount()));

        // Add horizontal marker for Lower Control Limit
        addMarker(m_pXYChart->yAxis(), m_pXbarRDataset->XBarLCL(), m_pXYChart->dashLineColor(QColor(Qt::red).rgb() & 0xffffff, Chart::AltDashLine),
                    "", 1, Chart::TopLeft);

        internalChartInfo.m_dMarkerMin = qMin(internalChartInfo.m_dMarkerMin, m_pXbarRDataset->XBarLCL());

        // Add Marker for Upper Control Limit
        addMarker(m_pXYChart->yAxis(), m_pXbarRDataset->XBarUCL(), m_pXYChart->dashLineColor(QColor(Qt::red).rgb() & 0xffffff, Chart::AltDashLine),
                    "", 1, Chart::BottomLeft);

        internalChartInfo.m_dMarkerMax = qMax(internalChartInfo.m_dMarkerMax, m_pXbarRDataset->XBarUCL());

        // Add Marker for Mean
        addMarker(m_pXYChart->yAxis(), m_pXbarRDataset->XBarMean(), QColor(Qt::green).rgb() & 0xffffff, "", 1, Chart::BottomLeft);

        // Add vertical marker for each appraiser
        for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
            addMarker(m_pXYChart->xAxis(), (uiAppraiser * m_pXbarRDataset->deviceCount()) - 0.5, QColor(Qt::black).rgb() & 0xffffff, "");

        // Add Title to the Y axis
        QString strYAxisTitle = "XBar CHART";

        if (m_pXbarRDataset->units().isEmpty() == false)
            strYAxisTitle += " (" + m_pXbarRDataset->units() + ")";

        m_pXYChart->yAxis()->setTitle(strYAxisTitle.toLatin1().constData());

        // Update viewport space if required
        switch(viewportMode())
        {
            case viewportOverLimits :
            default:
                // Chart has to be done over limits...unless they do not exist!
                if(isnan(m_pXbarRDataset->lowTestLimit()) == 0)
                    internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, m_pXbarRDataset->lowTestLimit());		// If chart over limits, update charting
                else
                    internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, m_pXbarRDataset->XBarMin());

                if(isnan(m_pXbarRDataset->highTestLimit()) == 0)
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->highTestLimit());	// If chart over limits, update charting
                else
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->XBarMax());

                break;

            case viewportOverData :

                // Chart has to be done over data (min & max)
                internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,  m_pXbarRDataset->XBarMin());
                internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->XBarMax());
                break;

            case viewportAdaptive :

                // Chart has to be done over limits...unless they do not exist!
                if(isnan(m_pXbarRDataset->lowTestLimit()) == 0)
                    internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, m_pXbarRDataset->lowTestLimit());		// If chart over limits, update charting

                if(isnan(m_pXbarRDataset->highTestLimit()) == 0)
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->highTestLimit());	// If chart over limits, update charting

                internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,  m_pXbarRDataset->XBarMin());
                internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->XBarMax());

                break;

            case viewportAdaptiveControl:

                // Chart has to be done over maxi of both datapoints & limits
                if(isnan(m_pXbarRDataset->XBarLCL()) == 0)
                    internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, m_pXbarRDataset->XBarLCL());

                if(isnan(m_pXbarRDataset->XBarUCL()) == 0)
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->XBarUCL());

                for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
                {
                        internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,	m_pXbarRDataset->XBarLCLAppraiser(uiAppraiser));
                        internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY,	m_pXbarRDataset->XBarUCLAppraiser(uiAppraiser));
                }

                internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,  m_pXbarRDataset->XBarMin());
                internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->XBarMax());

                break;

            case viewportOverControlLimits:

                // Chart has to be done over limits...unless they do not exist!
                if(isnan(m_pXbarRDataset->XBarLCL()) == 0)
                    internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, m_pXbarRDataset->XBarLCL());		// If chart over limits, update charting
                else
                    internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, m_pXbarRDataset->XBarMin());

                if(isnan(m_pXbarRDataset->XBarUCL()) == 0)
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->XBarUCL());	// If chart over limits, update charting
                else
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->XBarMax());

                for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
                {
                        internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,	m_pXbarRDataset->XBarLCLAppraiser(uiAppraiser));
                        internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY,	m_pXbarRDataset->XBarUCLAppraiser(uiAppraiser));
                }
                break;
        }

        internalChartInfo.m_dLowX	= -0.5;
        internalChartInfo.m_dHighX	= m_pXbarRDataset->dataCount() - 0.5;

        // Viewport: add 1% extra space at each end
        // Fit the scales to min/ max value + 1%
        double dYExtra	= (internalChartInfo.m_dHighY - internalChartInfo.m_dLowY) * 0.01;

        internalChartInfo.m_dLowY	-= dYExtra;
        internalChartInfo.m_dHighY	+= dYExtra;

        // Determine min and max values for the viewport
        if(isnan(m_pXbarRDataset->XBarLCL()) == 0)
            internalChartInfo.m_dViewportLowY = qMax(internalChartInfo.m_dViewportLowY, m_pXbarRDataset->XBarLCL());

        internalChartInfo.m_dViewportLowY = qMin(internalChartInfo.m_dViewportLowY, m_pXbarRDataset->XBarMin());

        if(isnan(m_pXbarRDataset->XBarLCL()) == 0)
            internalChartInfo.m_dViewportHighY = qMax(internalChartInfo.m_dViewportHighY, m_pXbarRDataset->XBarLCL());

        internalChartInfo.m_dViewportHighY = qMax(internalChartInfo.m_dViewportHighY, m_pXbarRDataset->XBarMax());

        if(isnan(m_pXbarRDataset->lowTestLimit()) == 0)
            internalChartInfo.m_dViewportLowY = qMin(internalChartInfo.m_dViewportLowY, m_pXbarRDataset->lowTestLimit());

        if(isnan(m_pXbarRDataset->highTestLimit()) == 0)
            internalChartInfo.m_dViewportHighY = qMax(internalChartInfo.m_dViewportLowY, m_pXbarRDataset->highTestLimit());

        if (internalChartInfo.m_dMarkerMin != C_INFINITE)
            internalChartInfo.m_dViewportLowY = qMin(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dMarkerMin);

        if (internalChartInfo.m_dMarkerMax != -C_INFINITE)
            internalChartInfo.m_dViewportHighY = qMax(internalChartInfo.m_dViewportHighY, internalChartInfo.m_dMarkerMax);

        internalChartInfo.m_dViewportLowY	= qMin(internalChartInfo.m_dViewportLowY,	m_pXbarRDataset->XBarMean());
        internalChartInfo.m_dViewportHighY	= qMax(internalChartInfo.m_dViewportHighY,	m_pXbarRDataset->XBarMean());

        for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
        {
            internalChartInfo.m_dViewportLowY	= qMin(internalChartInfo.m_dViewportLowY,	m_pXbarRDataset->XBarLCLAppraiser(uiAppraiser));
            internalChartInfo.m_dViewportHighY	= qMax(internalChartInfo.m_dViewportHighY,	m_pXbarRDataset->XBarUCLAppraiser(uiAppraiser));

            internalChartInfo.m_dViewportLowY	= qMin(internalChartInfo.m_dViewportLowY,	m_pXbarRDataset->XBarMeanAppraiser(uiAppraiser));
            internalChartInfo.m_dViewportHighY	= qMax(internalChartInfo.m_dViewportHighY,	m_pXbarRDataset->XBarMeanAppraiser(uiAppraiser));
        }

        internalChartInfo.m_dViewportLowY	= qMin(internalChartInfo.m_dViewportLowY,	internalChartInfo.m_dLowY);
        internalChartInfo.m_dViewportHighY	= qMax(internalChartInfo.m_dViewportHighY,	internalChartInfo.m_dHighY);

        internalChartInfo.m_dViewportLowX	= qMin(internalChartInfo.m_dViewportLowX,	internalChartInfo.m_dLowX);
        internalChartInfo.m_dViewportHighX	= qMax(internalChartInfo.m_dViewportHighX,	internalChartInfo.m_dHighX);

        // Define scales: if in Interactive charting, consider zoomin factor
        if(m_pChartOverlays && internalChartInfo.m_dViewportLowY != C_INFINITE && internalChartInfo.m_dViewportHighY != -C_INFINITE)
        {
            // Initialize viewport
            if (isViewportInitialized() == false)
            {
                initHorizontalViewport(internalChartInfo.m_dViewportLowX, internalChartInfo.m_dViewportHighX, internalChartInfo.m_dLowX, internalChartInfo.m_dHighX);
                initVerticalViewport(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dViewportHighY, internalChartInfo.m_dLowY, internalChartInfo.m_dHighY, 0.05);
            }

            // Using ViewPortLeft and ViewPortWidth, get the start and end dates of the view port.
            computeXBounds(internalChartInfo.m_dLowX, internalChartInfo.m_dHighX);
            computeYBounds(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY);
        }

        // Set the scale on the chart
        m_pXYChart->yAxis()->setLinearScale(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY);
        m_pXYChart->xAxis()->setLinearScale(internalChartInfo.m_dLowX, internalChartInfo.m_dHighX, Chart::NoValue);

        // Set the chart title
        m_pXYChart->addTitle(m_strTitle.toLatin1().constData());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void customizeChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexXBarChart::customizeChart()
{
    if (m_pXbarRDataset)
    {
        uint uiXValue = 0;

        for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
        {
            uiXValue = uiAppraiser * m_pXbarRDataset->deviceCount();

            drawCustomLine(m_pXYChart->getXCoor(uiXValue),
                            m_pXYChart->getYCoor(m_pXbarRDataset->XBarLCLAppraiser(uiAppraiser)),
                            m_pXYChart->getXCoor(uiXValue + m_pXbarRDataset->deviceCount() - 1),
                            m_pXYChart->getYCoor(m_pXbarRDataset->XBarLCLAppraiser(uiAppraiser)),
                            m_pXYChart->dashLineColor(QColor(Qt::blue).rgb() & 0xffffff, Chart::DotDashLine), 1);

            drawCustomLine(m_pXYChart->getXCoor(uiXValue),
                            m_pXYChart->getYCoor(m_pXbarRDataset->XBarUCLAppraiser(uiAppraiser)),
                            m_pXYChart->getXCoor(uiXValue + m_pXbarRDataset->deviceCount() - 1),
                            m_pXYChart->getYCoor(m_pXbarRDataset->XBarUCLAppraiser(uiAppraiser)),
                            m_pXYChart->dashLineColor(QColor(Qt::blue).rgb() & 0xffffff, Chart::DotDashLine), 1);

            drawCustomLine(m_pXYChart->getXCoor(uiXValue),
                            m_pXYChart->getYCoor(m_pXbarRDataset->XBarMeanAppraiser(uiAppraiser)),
                            m_pXYChart->getXCoor(uiXValue + m_pXbarRDataset->deviceCount() - 1),
                            m_pXYChart->getYCoor(m_pXbarRDataset->XBarMeanAppraiser(uiAppraiser)),
                            QColor(Qt::blue).rgb() & 0xffffff, 1);

            drawCustomLine(m_pXYChart->getXCoor(uiXValue - 0.5),
                            m_pXYChart->getYCoor(m_pXbarRDataset->lowTestLimitAppraiser(uiAppraiser)),
                            m_pXYChart->getXCoor(uiXValue + m_pXbarRDataset->deviceCount() - 0.5),
                            m_pXYChart->getYCoor(m_pXbarRDataset->lowTestLimitAppraiser(uiAppraiser)),
                            QColor(Qt::red).rgb() & 0xffffff, 1);

            drawCustomLine(m_pXYChart->getXCoor(uiXValue - 0.5),
                            m_pXYChart->getYCoor(m_pXbarRDataset->highTestLimitAppraiser(uiAppraiser)),
                            m_pXYChart->getXCoor(uiXValue + m_pXbarRDataset->deviceCount() - 0.5),
                            m_pXYChart->getYCoor(m_pXbarRDataset->highTestLimitAppraiser(uiAppraiser)),
                            QColor(Qt::red).rgb() & 0xffffff, 1);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexRChart
//
// Description	:	chart class to draw the R charts into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexRChart::GexRChart(int nSizeMode, GexWizardChart * lWizardParent, CGexChartOverlays * pChartOverlays /* = NULL */)
    : GexAbstractControlChart(chartTypeRChart, nSizeMode, lWizardParent, pChartOverlays)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexRChart::~GexRChart()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void buildOwnerChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexRChart::buildOwnerChart()
{
    if (m_pXbarRDataset)
    {
        GexInternalChartInfo	internalChartInfo;
        double					dLimitRange = std::numeric_limits<double>::quiet_NaN();

        LineLayer * pLineLayer	= m_pXYChart->addLineLayer();
        DataSet *	pDataset	= pLineLayer->addDataSet(DoubleArray(m_pXbarRDataset->RDataArray(), m_pXbarRDataset->dataCount()));

        pDataset->setDataSymbol(Chart::CircleSymbol, 5, QColor(Qt::red).rgb() & 0xffffff, QColor(Qt::black).rgb() & 0xffffff);

        m_pXYChart->xAxis()->setLabelStyle("", 7, TextColor, 90.0);
        m_pXYChart->xAxis()->setLabels(DoubleArray(m_pXbarRDataset->XBarRLabelArray(), m_pXbarRDataset->dataCount()));

        // Add horizontal marker for Lower Control Limit
        addMarker(m_pXYChart->yAxis(), m_pXbarRDataset->RLCL(), m_pXYChart->dashLineColor(QColor(Qt::red).rgb() & 0xffffff, Chart::AltDashLine),
                    "", 1, Chart::TopLeft);

        internalChartInfo.m_dMarkerMin = qMin(internalChartInfo.m_dMarkerMin, m_pXbarRDataset->RLCL());

        // Add horizontal marker for Upper Control Limit
        addMarker(m_pXYChart->yAxis(), m_pXbarRDataset->RUCL(), m_pXYChart->dashLineColor(QColor(Qt::red).rgb() & 0xffffff, Chart::AltDashLine),
                    "", 1, Chart::BottomLeft);

        internalChartInfo.m_dMarkerMax = qMax(internalChartInfo.m_dMarkerMin, m_pXbarRDataset->RUCL());

        // Add Marker for Mean
        addMarker(m_pXYChart->yAxis(), m_pXbarRDataset->RMean(), QColor(Qt::green).rgb() & 0xffffff, "", 1, Chart::BottomLeft);

        // Add vertical marker for each appraiser
        for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
            addMarker(m_pXYChart->xAxis(), (uiAppraiser * m_pXbarRDataset->deviceCount()) - 0.5, QColor(Qt::black).rgb() & 0xffffff, "");

        // Compute Test Limits Range
        if (isnan(m_pXbarRDataset->lowTestLimit()) == 0 &&
            isnan(m_pXbarRDataset->highTestLimit()) == 0)
            dLimitRange = m_pXbarRDataset->highTestLimit() - m_pXbarRDataset->lowTestLimit();

        // Add Title to the Y axis
        QString strYAxisTitle = "R CHART";

        if (m_pXbarRDataset->units().isEmpty() == false)
            strYAxisTitle += " (" + m_pXbarRDataset->units() + ")";

        m_pXYChart->yAxis()->setTitle(strYAxisTitle.toLatin1().constData());

        // Update viewport space if required
        switch(viewportMode())
        {
            case viewportOverLimits :
            default:
                // Chart has to be done over limits...unless they do not exist!
                internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, 0.0);		// If chart over limits, update charting

                if(isnan(dLimitRange) == 0)
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, dLimitRange);	// If chart over limits, update charting
                else
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->RMax());

                break;

            case viewportOverData :

                // Chart has to be done over data (min & max)
                internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,  m_pXbarRDataset->RMin());
                internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->RMax());
                break;

            case viewportAdaptive :
                // Chart has to be done over maxi of both datapoints & limits
                internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, 0.0);		// If chart over limits, update charting

                if(isnan(dLimitRange) == 0)
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, dLimitRange);	// If chart over limits, update charting

                for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
                {
                        internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,	m_pXbarRDataset->RLCLAppraiser(uiAppraiser));
                        internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY,	m_pXbarRDataset->RUCLAppraiser(uiAppraiser));

                        // To Do : Add R mean
                }

                internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,  m_pXbarRDataset->RMin());
                internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->RMax());

                break;

            case viewportAdaptiveControl:

                // Chart has to be done over maxi of both datapoints & limits
                if(m_pXbarRDataset->RLCL() != std::numeric_limits<double>::quiet_NaN())
                    internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, m_pXbarRDataset->RLCL());

                if(m_pXbarRDataset->RUCL() != std::numeric_limits<double>::quiet_NaN())
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->RUCL());

                for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
                {
                        internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,	m_pXbarRDataset->RLCLAppraiser(uiAppraiser));
                        internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY,	m_pXbarRDataset->RUCLAppraiser(uiAppraiser));
                }

                internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,  m_pXbarRDataset->RMin());
                internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->RMax());

                break;

            case viewportOverControlLimits:

                // Chart has to be done over limits...unless they do not exist!
                if(m_pXbarRDataset->RLCL() != std::numeric_limits<double>::quiet_NaN())
                    internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, m_pXbarRDataset->RLCL());		// If chart over limits, update charting
                else
                    internalChartInfo.m_dLowY = qMin(internalChartInfo.m_dLowY, m_pXbarRDataset->RMin());

                if(m_pXbarRDataset->RUCL() != std::numeric_limits<double>::quiet_NaN())
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->RUCL());	// If chart over limits, update charting
                else
                    internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY, m_pXbarRDataset->RMax());

                for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
                {
                        internalChartInfo.m_dLowY	= qMin(internalChartInfo.m_dLowY,	m_pXbarRDataset->RLCLAppraiser(uiAppraiser));
                        internalChartInfo.m_dHighY	= qMax(internalChartInfo.m_dHighY,	m_pXbarRDataset->RUCLAppraiser(uiAppraiser));
                }
                break;

        }

        internalChartInfo.m_dLowX	= -0.5;
        internalChartInfo.m_dHighX	= m_pXbarRDataset->dataCount() - 0.5;

        // Viewport: add 1% extra space at each end
        // Fit the scales to min/ max value + 1%
        double dYExtra	= (internalChartInfo.m_dHighY - internalChartInfo.m_dLowY) * 0.01;

        internalChartInfo.m_dLowY	-= dYExtra;
        internalChartInfo.m_dHighY	+= dYExtra;

        // Determine min and max values for the viewport
        if(isnan(m_pXbarRDataset->RLCL()) == 0)
            internalChartInfo.m_dViewportLowY = qMax(internalChartInfo.m_dViewportLowY, m_pXbarRDataset->RLCL());

        internalChartInfo.m_dViewportLowY = qMin(internalChartInfo.m_dViewportLowY, m_pXbarRDataset->RMin());

        if(isnan(m_pXbarRDataset->RLCL()) == 0)
            internalChartInfo.m_dViewportHighY = qMax(internalChartInfo.m_dViewportHighY, m_pXbarRDataset->RLCL());

        internalChartInfo.m_dViewportHighY = qMax(internalChartInfo.m_dViewportHighY, m_pXbarRDataset->RMax());

        if (internalChartInfo.m_dMarkerMin != C_INFINITE)
            internalChartInfo.m_dViewportLowY = qMin(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dMarkerMin);

        if (internalChartInfo.m_dMarkerMax != -C_INFINITE)
            internalChartInfo.m_dViewportHighY = qMax(internalChartInfo.m_dViewportHighY, internalChartInfo.m_dMarkerMax);

        internalChartInfo.m_dViewportLowY	= qMin(internalChartInfo.m_dViewportLowY,	m_pXbarRDataset->RMean());
        internalChartInfo.m_dViewportHighY	= qMax(internalChartInfo.m_dViewportHighY,	m_pXbarRDataset->RMean());

        for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
        {
            internalChartInfo.m_dViewportLowY	= qMin(internalChartInfo.m_dViewportLowY,	m_pXbarRDataset->RLCLAppraiser(uiAppraiser));
            internalChartInfo.m_dViewportHighY	= qMax(internalChartInfo.m_dViewportHighY,	m_pXbarRDataset->RUCLAppraiser(uiAppraiser));

            internalChartInfo.m_dViewportLowY	= qMin(internalChartInfo.m_dViewportLowY,	m_pXbarRDataset->RMeanAppraiser(uiAppraiser));
            internalChartInfo.m_dViewportHighY	= qMax(internalChartInfo.m_dViewportHighY,	m_pXbarRDataset->RMeanAppraiser(uiAppraiser));
        }

        internalChartInfo.m_dViewportLowY	= qMin(internalChartInfo.m_dViewportLowY,	internalChartInfo.m_dLowY);
        internalChartInfo.m_dViewportHighY	= qMax(internalChartInfo.m_dViewportHighY,	internalChartInfo.m_dHighY);

        internalChartInfo.m_dViewportLowX	= qMin(internalChartInfo.m_dViewportLowX,	internalChartInfo.m_dLowX);
        internalChartInfo.m_dViewportHighX	= qMax(internalChartInfo.m_dViewportHighX,	internalChartInfo.m_dHighX);

        // Define scales: if in Interactive charting, consider zoomin factor
        if(m_pChartOverlays && internalChartInfo.m_dViewportLowY != C_INFINITE && internalChartInfo.m_dViewportHighY != -C_INFINITE)
        {
            // Initialize viewport
            if (isViewportInitialized() == false)
            {
                initHorizontalViewport(internalChartInfo.m_dViewportLowX, internalChartInfo.m_dViewportHighX, internalChartInfo.m_dLowX, internalChartInfo.m_dHighX);
                initVerticalViewport(internalChartInfo.m_dViewportLowY, internalChartInfo.m_dViewportHighY, internalChartInfo.m_dLowY, internalChartInfo.m_dHighY, 0.05);
            }

            // Using ViewPortLeft and ViewPortWidth, get the start and end dates of the view port.
            computeXBounds(internalChartInfo.m_dLowX, internalChartInfo.m_dHighX);
            computeYBounds(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY);
        }

        // Set the scale on the chart
        m_pXYChart->yAxis()->setLinearScale(internalChartInfo.m_dLowY, internalChartInfo.m_dHighY);
        m_pXYChart->xAxis()->setLinearScale(internalChartInfo.m_dLowX, internalChartInfo.m_dHighX, Chart::NoValue);

        // Set the chart title
        m_pXYChart->addTitle(m_strTitle.toLatin1().constData());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void customizeChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexRChart::customizeChart()
{
    if (m_pXbarRDataset)
    {
        uint uiXValue = 0;

        for (uint uiAppraiser = 0; uiAppraiser < m_pXbarRDataset->appraiserCount(); ++uiAppraiser)
        {
            uiXValue = uiAppraiser * m_pXbarRDataset->deviceCount();

            drawCustomLine(m_pXYChart->getXCoor(uiXValue),
                            m_pXYChart->getYCoor(m_pXbarRDataset->RLCLAppraiser(uiAppraiser)),
                            m_pXYChart->getXCoor(uiXValue + m_pXbarRDataset->deviceCount() - 1),
                            m_pXYChart->getYCoor(m_pXbarRDataset->RLCLAppraiser(uiAppraiser)),
                            m_pXYChart->dashLineColor(QColor(Qt::blue).rgb() & 0xffffff, Chart::DotDashLine), 1);

            drawCustomLine(m_pXYChart->getXCoor(uiXValue),
                            m_pXYChart->getYCoor(m_pXbarRDataset->RUCLAppraiser(uiAppraiser)),
                            m_pXYChart->getXCoor(uiXValue + m_pXbarRDataset->deviceCount() - 1),
                            m_pXYChart->getYCoor(m_pXbarRDataset->RUCLAppraiser(uiAppraiser)),
                            m_pXYChart->dashLineColor(QColor(Qt::blue).rgb() & 0xffffff, Chart::DotDashLine), 1);

            drawCustomLine(m_pXYChart->getXCoor(uiXValue),
                            m_pXYChart->getYCoor(m_pXbarRDataset->RMeanAppraiser(uiAppraiser)),
                            m_pXYChart->getXCoor(uiXValue + m_pXbarRDataset->deviceCount() - 1),
                            m_pXYChart->getYCoor(m_pXbarRDataset->RMeanAppraiser(uiAppraiser)),
                            QColor(Qt::blue).rgb() & 0xffffff, 1);

            if (isnan(m_pXbarRDataset->lowTestLimitAppraiser(uiAppraiser)) == 0 &&
                isnan(m_pXbarRDataset->highTestLimitAppraiser(uiAppraiser)) == 0)
            {
                double dLimitRange = m_pXbarRDataset->highTestLimitAppraiser(uiAppraiser) - m_pXbarRDataset->lowTestLimitAppraiser(uiAppraiser);

                drawCustomLine(m_pXYChart->getXCoor(uiXValue - 0.5),
                                m_pXYChart->getYCoor(dLimitRange),
                                m_pXYChart->getXCoor(uiXValue + m_pXbarRDataset->deviceCount() - 0.5),
                                m_pXYChart->getYCoor(dLimitRange),
                                QColor(Qt::red).rgb() & 0xffffff, 1);
            }
        }
    }
}

