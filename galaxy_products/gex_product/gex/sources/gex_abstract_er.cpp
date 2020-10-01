///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_abstract_er.h"
#include "browser.h"
#include "report_options.h"
#include "gex_report.h"
#include "report_template.h"
#include "xychart_data.h"

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern	CGexReport*		gexReport;
extern	CReportOptions		ReportOptions;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAbstractEnterpriseReport
//
// Description	:	Base class for enteprise report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAbstractEnterpriseReport::GexAbstractEnterpriseReport(
    FILE* hReportFile,
    GexDatabaseEntry* pDatabaseEntry,
    GexDbPlugin_Filter& cFilters,
    GS::Gex::ReportTemplateSection* pSection)
    : m_cFilters(cFilters)
{
	m_hReportFile		= hReportFile;
	m_pDatabaseEntry	= pDatabaseEntry;
	m_pSection			= pSection;

	// Build base names for image files to store the charts
	m_strChartBaseFileName.sprintf("sql_image_s%d",	m_pSection->iSection_ID);
	m_strChartDirectory.sprintf("%s/images/", ReportOptions.strReportDirectory.toLatin1().constData());
	m_strRawDataFullFileName.sprintf("%s/pages/sql_rawdata_s%d.csv", ReportOptions.strReportDirectory.toLatin1().constData(), m_pSection->iSection_ID);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAbstractEnterpriseReport::~GexAbstractEnterpriseReport()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractEnterpriseReport::writeHtmlHeader()
//
// Description	:	write a generic html header
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractEnterpriseReport::writeHtmlHeader()
{
	QStringList::ConstIterator	it;
	QString						strString;

	fprintf(m_hReportFile, "<table border=\"1\" width=\"800\" cellspacing=\"1\" cellpadding=\"1\">\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Dataset</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">\n");
	fprintf(m_hReportFile, "<table border=\"0\" width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Database</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", m_pSection->pEnterpriseReport->m_strDatabaseLogicalName.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">DataType</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", m_pSection->pEnterpriseReport->m_strDatabaseType.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">TimePeriod</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", m_pSection->pEnterpriseReport->m_strTimePeriod.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");

	if(m_cFilters.bUseTimePeriod)
	{
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">From</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", m_cFilters.calendarFrom.toString("dd MMM yyyy").toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">To</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", m_cFilters.calendarTo.toString("dd MMM yyyy").toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
	}

	for(it = m_pSection->pEnterpriseReport->m_strFiltersList.begin(); it != m_pSection->pEnterpriseReport->m_strFiltersList.end(); it++)
	{
		strString = (*it).section('=', 1, 1);
		if(!strString.isNull())
		{
			fprintf(m_hReportFile, "<tr>\n");
			fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">%s</td>\n", (*it).section('=', 0, 0).toLatin1().constData());
			fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", (*it).section('=', 1, 1).toLatin1().constData());
			fprintf(m_hReportFile, "</tr>\n");
		}
	}

	fprintf(m_hReportFile, "</table>\n");
	fprintf(m_hReportFile, "</td>\n");
	fprintf(m_hReportFile, "</tr>\n");

	if(!m_pSection->pEnterpriseReport->m_strAggregateField.isEmpty())
	{
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">X-Axis</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">%s</td>\n", m_pSection->pEnterpriseReport->m_strAggregateField.toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
	}

	if(!m_pSection->pEnterpriseReport->m_strSplitField.isEmpty())
	{
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Split by</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">%s</td>\n", m_pSection->pEnterpriseReport->m_strSplitField.toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
	}

	if(!m_pSection->pEnterpriseReport->m_strBinList.isEmpty())
	{
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Binnings</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">%s</td>\n", m_pSection->pEnterpriseReport->m_strBinList.toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
	}

	if(!m_strRawDataFullFileName.isEmpty())
	{
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Raw data</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\"><a href=\"file:///%s\">%s</a></td>\n", m_strRawDataFullFileName.toLatin1().constData(), m_strRawDataFullFileName.toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
	}

	fprintf(m_hReportFile, "</table><br>\n");
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractEnterpriseReport::makeXYChart(GexDbPlugin_XYChartList& clXYChartList,const QString& strChartFullName, const QString& strYAxis, int nMarkerPrecision, bool bBarGraph /*= true*/)
//
// Description	:	Create a generic XYChart for production report
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractEnterpriseReport::makeXYChart(GexDbPlugin_XYChartList& clXYChartList,const QString& strChartFullName, const QString& strYAxis, int nMarkerPrecision, bool bBarGraph /*= true*/)
{
	double *					plfData;
    char **                     pcLabels;
	int							nIndex, nNbMergedLabels, nMergedIndex;
	GexDbPlugin_XYChart_Data *	pXYChartData;

	// Create a XYChart object of size 900 x 500 pixels.
	XYChart * pChart = new XYChart(900,500);

	// Add a title
	QString strTitle = m_pSection->pEnterpriseReport->m_strSectionTitle;
	strTitle += " (Legend: ";
	strTitle += m_pSection->pEnterpriseReport->m_strSplitField;
	strTitle += ")";

	pChart->addTitle(strTitle.toLatin1().constData(), "timesbi.ttf", 14);

	// Compute plot area
	QRect	clPlotRect;
	int		nStartY = 30+(int)ceil((double)(clXYChartList.count())/5.0)*30;
	if(bBarGraph && m_pSection->pEnterpriseReport->m_strBarStyle.toLower().startsWith("3d"))
		nStartY += 10;

	clPlotRect.setRect(70, nStartY, 810, 410-nStartY);

	// Add a legend box
	LegendBox *pLegendBox = pChart->addLegend(70, 20, false, "arialbi.ttf", 10);
	pLegendBox->setBackground(Chart::Transparent);

	// Compute background colors
	int nBgColor = Chart::Transparent, nAltBgColor = -1;
	if(m_pSection->pEnterpriseReport->m_strBackgroundStyle.toLower() == "white")
		nBgColor = 0xffffff;
	else if(m_pSection->pEnterpriseReport->m_strBackgroundStyle.toLower() == "white & grey")
	{
		nBgColor = 0xf8f8f8;
		nAltBgColor = 0xffffff;
	}
	else if(m_pSection->pEnterpriseReport->m_strBackgroundStyle.toLower() == "custom color...")
		nBgColor = m_pSection->pEnterpriseReport->m_cBackgroundColor.rgb() & 0xffffff;
	else if(m_pSection->pEnterpriseReport->m_strBackgroundStyle.toLower() == "gradient color...")
		nBgColor = pChart->linearGradientColor(clPlotRect.left(), clPlotRect.top(), clPlotRect.width() , clPlotRect.height(), m_pSection->pEnterpriseReport->m_cBackgroundColor.rgb() & 0xffffff, 0xffffff);

	// Set plot area
	pChart->setPlotArea(clPlotRect.left(), clPlotRect.top(), clPlotRect.width() , clPlotRect.height(), nBgColor, nAltBgColor);

	// Compute X-Axis label merge
	QStringList strlLabelMerge;
//	for(pXYChartData = clXYChartList.first(); pXYChartData; pXYChartData = clXYChartList.next())
    foreach(pXYChartData, clXYChartList)
	{
		for(nIndex = 0; nIndex < pXYChartData->m_nNbDataPoints; nIndex++)
		{
            if(strlLabelMerge.indexOf(pXYChartData->m_pcXAxisLabels[nIndex]) == -1)
				strlLabelMerge.append(pXYChartData->m_pcXAxisLabels[nIndex]);
		}
	}
	nNbMergedLabels = strlLabelMerge.count();
	// Sort X-Axis label merge
	strlLabelMerge.sort();

	// Allocate mem space for merged label and data pointers
	plfData		= new double[nNbMergedLabels];
    pcLabels	= new char*[nNbMergedLabels];

	// Init merged label pointer
	for(nIndex = 0; nIndex < nNbMergedLabels; nIndex++)
    {
        pcLabels[nIndex] = new char[strlLabelMerge[nIndex].size() + 1];
        strcpy(pcLabels[nIndex], strlLabelMerge[nIndex].toLatin1().constData());
    }

	// Set the labels on the x axis.
	pChart->xAxis()->setLabelStyle(0, 7.5, 0, 30);
	pChart->xAxis()->setLabels(StringArray(pcLabels, nNbMergedLabels));

	// Add a title to the y axis
	pChart->yAxis()->setTitle(strYAxis.toLatin1().constData());

	// Add a title to the x axis
	pChart->xAxis()->setTitle(m_pSection->pEnterpriseReport->m_strAggregateField.toLatin1().constData());

	// Add a multi-color data layer
	LineLayer *	pLineLayer;
	BarLayer *	pBarLayer;
	Layer *		pDataLayer;
	DataSet *	pDataSet;
	QString		strMarkerFormat;

	if(bBarGraph)
	{
		// Add bar layer
        if(m_pSection->pEnterpriseReport->m_strBarStyle.toLower().indexOf("stacked") != -1)
		{
			pBarLayer = pChart->addBarLayer(Chart::Stack);
			if(m_pSection->pEnterpriseReport->m_bShowValueMarkers)
				pBarLayer->setAggregateLabelStyle();
		}
        else if(m_pSection->pEnterpriseReport->m_strBarStyle.toLower().indexOf("overlapping") != -1)
		{
			pBarLayer = pChart->addBarLayer(Chart::Side);
			// Set 50% overlap between bars
			pBarLayer->setOverlapRatio(0.5, false);
			// Configure the bars within a group to touch each others (no gap)
			pBarLayer->setBarGap(0.2);
			// Draw the ticks between label positions (instead of at label positions)
			pChart->xAxis()->setTickOffset(0.5);
		}
		else
		{
			pBarLayer = pChart->addBarLayer(Chart::Side);
			// Configure the bars within a group to touch each others (no gap)
			pBarLayer->setBarGap(0.2, Chart::TouchBar);
			// Draw the ticks between label positions (instead of at label positions)
			pChart->xAxis()->setTickOffset(0.5);
		}

		// Use gradient colors in bars?
		if(m_pSection->pEnterpriseReport->m_bGradientBarColor)
			pBarLayer->setBorderColor(0, Chart::softLighting(Chart::Right));

		// Set layer to 3D depth
		if(m_pSection->pEnterpriseReport->m_strBarStyle.toLower().startsWith("3d"))
			pBarLayer->set3D(10);

		// Set bar shape
        if(m_pSection->pEnterpriseReport->m_strBarStyle.toLower().indexOf("cylindrical") != -1)
			pBarLayer->setBarShape(Chart::CircleShape);

		// Set data layer pointer
		pDataLayer = pBarLayer;
	}
	else
	{
		// Add line layer
		if(m_pSection->pEnterpriseReport->m_strLineStyle.toLower().startsWith("spline line"))
			pLineLayer = pChart->addSplineLayer();
		else if(m_pSection->pEnterpriseReport->m_strLineStyle.toLower().startsWith("step line"))
			pLineLayer = pChart->addStepLineLayer();
		else
			pLineLayer = pChart->addLineLayer();

		// Set data layer pointer
		pDataLayer = pLineLayer;
	}

	// Enable data labels on data
	if(m_pSection->pEnterpriseReport->m_bShowValueMarkers)
	{
		strMarkerFormat.sprintf("{value|%d}", nMarkerPrecision);
		pDataLayer->setDataLabelStyle();
		pDataLayer->setDataLabelFormat(strMarkerFormat.toLatin1().constData());
	}

	// Compute merged dataset for each split chart, and add it to the layer
	int nDataColor,	nColorIndex = 0xffff0008;
//	for(pXYChartData = clXYChartList.first(); pXYChartData; pXYChartData = clXYChartList.next())
    foreach(pXYChartData, clXYChartList)
	{
		for(nIndex = 0; nIndex < nNbMergedLabels; nIndex++)
			plfData[nIndex] = 0.0;

		for(nIndex = 0; nIndex < pXYChartData->m_nNbDataPoints; nIndex++)
		{
            nMergedIndex = strlLabelMerge.indexOf(pXYChartData->m_pcXAxisLabels[nIndex]);
			plfData[nMergedIndex] = pXYChartData->m_plfData[nIndex];
		}

		if(bBarGraph)
		{
			// Use transparency?
			nDataColor = pChart->getColor(nColorIndex) & 0xffffff;
			if(m_pSection->pEnterpriseReport->m_bSemiTransparentBar)
				nDataColor = nDataColor | 0x80000000;
			pDataSet = pDataLayer->addDataSet(DoubleArray(plfData, nNbMergedLabels), nDataColor, pXYChartData->m_strSplitValue.toLatin1().constData());
		}
		else
		{
			// Dashed line?
			if(m_pSection->pEnterpriseReport->m_bDashedLine)
				pDataSet = pDataLayer->addDataSet(DoubleArray(plfData, nNbMergedLabels), pChart->dashLineColor(nColorIndex), pXYChartData->m_strSplitValue.toLatin1().constData());
			else
				pDataSet = pDataLayer->addDataSet(DoubleArray(plfData, nNbMergedLabels), nColorIndex, pXYChartData->m_strSplitValue.toLatin1().constData());

			// Use spots?
			if(m_pSection->pEnterpriseReport->m_strLineSpots.toLower() == "square")
				pDataSet->setDataSymbol(Chart::SquareSymbol, 7);
			else if(m_pSection->pEnterpriseReport->m_strLineSpots.toLower() == "circle")
				pDataSet->setDataSymbol(Chart::CircleSymbol, 7);
			else if(m_pSection->pEnterpriseReport->m_strLineSpots.toLower() == "diamonds")
				pDataSet->setDataSymbol(Chart::DiamondSymbol, 7);
			else if(m_pSection->pEnterpriseReport->m_strLineSpots.toLower() == "triangle")
				pDataSet->setDataSymbol(Chart::TriangleSymbol, 7);
			else if(m_pSection->pEnterpriseReport->m_strLineSpots.toLower() == "cross")
				pDataSet->setDataSymbol(Chart::CrossSymbol, 7);
		}
		nColorIndex++;
	}

	// output the chart
	pChart->makeChart(strChartFullName.toLatin1().constData());

	//free up resources
	delete [] plfData;
    for (int lLabelIndx = 0; lLabelIndx < nNbMergedLabels; ++lLabelIndx)
        if (pcLabels[lLabelIndx])
            delete pcLabels[lLabelIndx];
	delete [] pcLabels;
	delete pChart;
	pChart=0;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractEnterpriseReport::makeXYChart(const GexDbPlugin_XYChart_Data * pXYChartData, const QString& strChartFullName, bool bBarGraph /*= true*/)
//
// Description	:	Create a generic XYChart for production report
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractEnterpriseReport::makeXYChart(const GexDbPlugin_XYChart_Data * pXYChartData, const QString& strChartFullName, bool bBarGraph /*= true*/)
{
	// Create a XYChart object of size 900 x 500 pixels.
	XYChart *pChart = new XYChart(900,500);

	// Add a title
	if(m_pSection->pEnterpriseReport->m_strSplitField.isEmpty())
		pChart->addTitle(m_pSection->pEnterpriseReport->m_strSectionTitle.toLatin1().constData(), "timesbi.ttf", 14);
	else
	{
		QString strTitle = m_pSection->pEnterpriseReport->m_strSectionTitle + " (";
		strTitle += m_pSection->pEnterpriseReport->m_strSplitField + ": ";
		strTitle += pXYChartData->m_strSplitValue;
		strTitle += ")";
		pChart->addTitle(strTitle.toLatin1().constData(), "timesbi.ttf", 14);
	}

	// Compute plot area
	QRect clPlotRect;
	if(pXYChartData->m_bDoubleYAxis)
	{
		if(bBarGraph && m_pSection->pEnterpriseReport->m_strBarStyle.toLower().startsWith("3d"))
			clPlotRect.setRect(70, 70, 740, 340);
		else
			clPlotRect.setRect(70, 60, 740, 350);

		// Add a legend box
		pChart->addLegend(70, 20, false, "arialbi.ttf", 10)->setBackground(Chart::Transparent);
	}
	else
	{
		if(bBarGraph && m_pSection->pEnterpriseReport->m_strBarStyle.toLower().startsWith("3d"))
			clPlotRect.setRect(70, 40, 810, 370);
		else
			clPlotRect.setRect(70, 30, 810, 380);
	}

	// Compute background colors
	int nBgColor = Chart::Transparent, nAltBgColor = -1;
	if(m_pSection->pEnterpriseReport->m_strBackgroundStyle.toLower() == "white")
		nBgColor = 0xffffff;
	else if(m_pSection->pEnterpriseReport->m_strBackgroundStyle.toLower() == "white & grey")
	{
		nBgColor = 0xf8f8f8;
		nAltBgColor = 0xffffff;
	}
	else if(m_pSection->pEnterpriseReport->m_strBackgroundStyle.toLower() == "custom color...")
		nBgColor = m_pSection->pEnterpriseReport->m_cBackgroundColor.rgb() & 0xffffff;
	else if(m_pSection->pEnterpriseReport->m_strBackgroundStyle.toLower() == "gradient color...")
		nBgColor = pChart->linearGradientColor(clPlotRect.left(), clPlotRect.top(), clPlotRect.width() , clPlotRect.height(), m_pSection->pEnterpriseReport->m_cBackgroundColor.rgb() & 0xffffff, 0xffffff);

	// Set plot area
	pChart->setPlotArea(clPlotRect.left(), clPlotRect.top(), clPlotRect.width() , clPlotRect.height(), nBgColor, nAltBgColor);

	// Set the labels on the x axis.
	pChart->xAxis()->setLabelStyle(0, 7.5, 0, 30);
	pChart->xAxis()->setLabels(StringArray(pXYChartData->m_pcXAxisLabels, pXYChartData->m_nNbDataPoints));

	// Add a title to the left y axis
	pChart->yAxis()->setTitle(pXYChartData->m_strYAxisLegend.toLatin1().constData());

	// Add a title to the x axis
	pChart->xAxis()->setTitle(m_pSection->pEnterpriseReport->m_strAggregateField.toLatin1().constData());

	// Add line layer?
	LineLayer	*pLineLayer;
	BarLayer	*pBarLayer;
	Layer		*pDataLayer;
	DataSet		*pDataSet;
	QString		strYLegend="", strMarkerFormat;

	if(pXYChartData->m_bDoubleYAxis)
	{
		// Add a line chart layer
		int	nLineColor = pChart->dashLineColor(0, Chart::DotLine);
		if(m_pSection->pEnterpriseReport->m_strLineStyle.toLower().startsWith("spline line"))
			pLineLayer = pChart->addSplineLayer();
		else if(m_pSection->pEnterpriseReport->m_strLineStyle.toLower().startsWith("step line"))
			pLineLayer = pChart->addStepLineLayer();
		else
			pLineLayer = pChart->addLineLayer();
		pDataSet = pLineLayer->addDataSet(DoubleArray(pXYChartData->m_plfYAxisData_2, pXYChartData->m_nNbDataPoints), nLineColor, pXYChartData->m_strYAxisLegend_2.toLatin1().constData());
		pDataSet->setDataSymbol(Chart::StarShape(4), 7);
		pLineLayer->setUseYAxis2();
		pChart->yAxis2()->setTitle(pXYChartData->m_strYAxisLegend_2.toLatin1().constData());

		// Enable data labels on line
		if(m_pSection->pEnterpriseReport->m_bShowValueMarkers)
		{
			strMarkerFormat.sprintf("{value|%d}", pXYChartData->m_nMarkerPrecision_2);
			pLineLayer->setDataLabelStyle();
			pLineLayer->setDataLabelFormat(strMarkerFormat.toLatin1().constData());
		}

		// Use a lgend for data layer
		strYLegend = pXYChartData->m_strYAxisLegend;
	}

	// Add data layer
	int	nDataColor = m_pSection->pEnterpriseReport->m_cDefaultColor.rgb() & 0xffffff;
	if(bBarGraph)
	{
		// Use transparency?
		if(m_pSection->pEnterpriseReport->m_bSemiTransparentBar)
			nDataColor = nDataColor | 0x80000000;

		// Add bar layer
		pBarLayer = pChart->addBarLayer();

		// Add dataset
		pDataSet = pBarLayer->addDataSet(DoubleArray(pXYChartData->m_plfYAxisData, pXYChartData->m_nNbDataPoints), nDataColor, strYLegend.toLatin1().constData());

		// Use gradient colors in bars?
		if(m_pSection->pEnterpriseReport->m_bGradientBarColor)
			pBarLayer->setBorderColor(0, Chart::softLighting(Chart::Right));

		// Set layer to 3D depth
		if(m_pSection->pEnterpriseReport->m_strBarStyle.toLower().startsWith("3d"))
			pBarLayer->set3D(10);

		// Set bar shape
        if(m_pSection->pEnterpriseReport->m_strBarStyle.toLower().indexOf("cylindrical") != -1)
			pBarLayer->setBarShape(Chart::CircleShape);

		// Set data layer pointer
		pDataLayer = pBarLayer;
	}
	else
	{
		// Dashed line?
		if(m_pSection->pEnterpriseReport->m_bDashedLine)
			nDataColor = pChart->dashLineColor(nDataColor, Chart::DashLine);

		// Add line layer
		if(m_pSection->pEnterpriseReport->m_strLineStyle.toLower().startsWith("spline line"))
			pLineLayer = pChart->addSplineLayer();
		else if(m_pSection->pEnterpriseReport->m_strLineStyle.toLower().startsWith("step line"))
			pLineLayer = pChart->addStepLineLayer();
		else
			pLineLayer = pChart->addLineLayer();

		// Add dataset
		pDataSet = pLineLayer->addDataSet(DoubleArray(pXYChartData->m_plfYAxisData, pXYChartData->m_nNbDataPoints), nDataColor, strYLegend.toLatin1().constData());

		// Use spots?
		if(m_pSection->pEnterpriseReport->m_strLineSpots.toLower() == "square")
			pDataSet->setDataSymbol(Chart::SquareSymbol, 7);
		else if(m_pSection->pEnterpriseReport->m_strLineSpots.toLower() == "circle")
			pDataSet->setDataSymbol(Chart::CircleSymbol, 7);
		else if(m_pSection->pEnterpriseReport->m_strLineSpots.toLower() == "diamonds")
			pDataSet->setDataSymbol(Chart::DiamondSymbol, 7);
		else if(m_pSection->pEnterpriseReport->m_strLineSpots.toLower() == "triangle")
			pDataSet->setDataSymbol(Chart::TriangleSymbol, 7);
		else if(m_pSection->pEnterpriseReport->m_strLineSpots.toLower() == "cross")
			pDataSet->setDataSymbol(Chart::CrossSymbol, 7);

		// Set data layer pointer
		pDataLayer = pLineLayer;
	}

	// Enable data labels on data
	if(m_pSection->pEnterpriseReport->m_bShowValueMarkers)
	{
		strMarkerFormat.sprintf("{value|%d}", pXYChartData->m_nMarkerPrecision);
		pDataLayer->setDataLabelStyle();
		pDataLayer->setDataLabelFormat(strMarkerFormat.toLatin1().constData());
	}

	// output the chart
	pChart->makeChart(strChartFullName.toLatin1().constData());

	//free up resources
	delete pChart;
	pChart=0;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractEnterpriseReport::create()
//
// Description	:	Create the enterprise report
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractEnterpriseReport::create()
{
	// Compute data for the report. If an error occurs, do nothing
	if (computeData())
	{
		// Create the raw data file
		writeRawData();

		// Write the report html header
		writeHtmlHeader();

		// Write the report (graphs, tables)
		writeReport();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexERTableHtml
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexERTableHtml::GexERTableHtml(FILE * hReportFile, int nMaxLine /*= 35*/)
{
	m_nMaxLine		= nMaxLine;
	m_hReportFile	= hReportFile;
	m_nLineCount	= 0;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexERTableHtml::~GexERTableHtml()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERTableHtml::open(const QString& strOption)
//
// Description	:	Open the html table
//
///////////////////////////////////////////////////////////////////////////////////
void GexERTableHtml::open(const QString& strOption)
{
        fprintf(m_hReportFile, "%s", strOption.toLatin1().constData());
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERTableHtml::close()
//
// Description	:	Close the html table
//
///////////////////////////////////////////////////////////////////////////////////
void GexERTableHtml::close()
{
	fprintf(m_hReportFile, "</table><br>\n");
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERTableHtml::setHeader(const QString& strHeader)
//
// Description	:	Sets the header of the html table
//
///////////////////////////////////////////////////////////////////////////////////
void GexERTableHtml::setHeader(const QString& strHeader)
{
	m_strHeader = strHeader;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERTableHtml::addLine(const QString& strLine)
//
// Description	:	Add a line to the html table
//
///////////////////////////////////////////////////////////////////////////////////
void GexERTableHtml::addLine(const QString& strLine)
{
	QString of=ReportOptions.GetOption("output", "format").toString();
	if ( m_nLineCount >= m_nMaxLine
         &&  ( of=="DOC"|| of=="PDF" || of=="PPT" || of=="ODT"
			 ) //(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML))
		)
	{
		m_nLineCount = 0;
		gexReport->WritePageBreak();
	}

	if (m_nLineCount == 0)
                fprintf(m_hReportFile, "%s", m_strHeader.toLatin1().constData());

        fprintf(m_hReportFile, "%s", strLine.toLatin1().constData());

	m_nLineCount++;
}
