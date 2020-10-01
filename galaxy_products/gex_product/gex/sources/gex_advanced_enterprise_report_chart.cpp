///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gex_advanced_enterprise_report_chart.h"
#include "colors_generator.h"
#include "report_options.h"
#include "report_build.h"
#include <gqtl_log.h>

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QClipboard>

///////////////////////////////////////////////////////////
// Other Includes
///////////////////////////////////////////////////////////
#include <chartdir.h>
#include "gex_report.h"

///////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////
extern CReportOptions	ReportOptions;
extern CGexReport*		gexReport;

///////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////
extern QString			formatHtmlImageFilename(const QString& strImageFileName);

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportChartSerie
//
// Description	:	Class which represents a serie in a chart
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportChartSerie::GexAdvancedEnterpriseReportChartSerie()
{

}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportChartSerie::~GexAdvancedEnterpriseReportChartSerie()
{

}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportChartXAxis
//
// Description	:	Class which represents the X-Axis on a chart
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportChartXAxis::GexAdvancedEnterpriseReportChartXAxis()
    : m_ePos(Bottom), mMaxLabelSize(18)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportChartXAxis::~GexAdvancedEnterpriseReportChartXAxis()
{

}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportChartYAxis
//
// Description	:	Class which represents the YAxis on a chart
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportChartYAxis::GexAdvancedEnterpriseReportChartYAxis() : m_dMax(C_INFINITE), m_dMin(-C_INFINITE), m_eZOrder(GexAdvancedEnterpriseReportChartYAxis::NoOrder), m_ePos(Left),  m_eType(TypeLine)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportChartYAxis::~GexAdvancedEnterpriseReportChartYAxis()
{
    while (m_lstSerie.isEmpty() == false)
        delete m_lstSerie.takeFirst();
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportChartYAxis::addSerie(GexAdvancedEnterpriseReportChartSerie * pSerie)
//
// Description	:	add a serie to the Axis
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportChartYAxis::addSerie(GexAdvancedEnterpriseReportChartSerie * pSerie)
{
    m_lstSerie.append(pSerie);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportChart
//
// Description	:	Class which represents a chart in a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportChart::GexAdvancedEnterpriseReportChart() : GexAdvancedEnterpriseReportSection() , m_eLegendMode(LegendOn), m_pXAxis(NULL)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportChart::~GexAdvancedEnterpriseReportChart()
{
    if (m_pXAxis)
    {
        delete m_pXAxis;
        m_pXAxis = NULL;
    }

    while (m_mapYAxis.isEmpty() == false)
        delete m_mapYAxis.take(m_mapYAxis.begin().key());
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportChart::addYAxis(GexAdvancedEnterpriseReportChartYAxis * pYAxis)
//
// Description	:	add an Axis to the chart
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportChart::addYAxis(GexAdvancedEnterpriseReportChartYAxis * pYAxis)
{
    if (pYAxis)
    {
        if (m_mapYAxis.contains(pYAxis->pos()) == false)
            m_mapYAxis.insert(pYAxis->pos(), pYAxis);
        else
            GSLOG(SYSLOG_SEV_NOTICE, "Axis already defined...");
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportChart::setLegendMode(const QString& strLegendMode)
//
// Description	:	Set the legend display mode
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportChart::setLegendMode(const QString& strLegendMode)
{
    if (strLegendMode.toLower() ==  "true")
        m_eLegendMode = LegendOn;
    else if (strLegendMode.toLower() ==  "false")
        m_eLegendMode = LegendOff;
    else if (strLegendMode.toLower() ==  "auto")
        m_eLegendMode = LegendAuto;
    else
        GSLOG(SYSLOG_SEV_WARNING, QString("Unknown legend visibility (%1)...").arg(strLegendMode).toLatin1().data() );
}

//////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportChart::setXAxis(GexAdvancedEnterpriseReportChartXAxis * pXAxis)
//
// Description	:	add an Axis to the chart
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportChart::setXAxis(GexAdvancedEnterpriseReportChartXAxis * pXAxis)
{
    if (pXAxis)
    {
        if (m_pXAxis == NULL)
            m_pXAxis = pXAxis;
        else
            GSLOG(SYSLOG_SEV_NOTICE, "X Axis already defined...");
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportChart::fillSerieColor(const GexDbPluginERDatasetGroup& datasetGroup, bool bDrab)
//
// Description	:	Fill array of color for series
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportChart::fillSerieColor(QVector<QColor>& vecColor, const GexDbPluginERDatasetGroup& datasetGroup, bool bDrab) const
{
    QColor rgbColor;

    // Clear color vector
    vecColor.clear();
    vecColor.resize(datasetGroup.series().count());

    if (datasetGroup.binGroupBy() == GexDbPluginERDatasetRow::BinGroupBySerie)
    {
        for (int nSerie = 0; nSerie < datasetGroup.series().count(); ++nSerie)
        {
            gexReport->cDieColor.assignBinColor(datasetGroup.serieAt(nSerie).field("bin_no").value().toInt(),
                                                                               (datasetGroup.serieAt(nSerie).field("bin_cat").value().toString() == "P"));

            vecColor[nSerie] = gexReport->cDieColor.GetWafmapDieColor(datasetGroup.serieAt(nSerie).field("bin_no").value().toInt(),
                                                                        datasetGroup.serieAt(nSerie).field("bin_type").value().toString() == "S");
        }
    }
    else
    {
        // Dim the colors generators engine
        ColorsGenerator::setDim(datasetGroup.series().count(), 1, Qt::black, 0.10f);

        int nSerieEven	= 0;
        int nSerieOdd	= (datasetGroup.series().count() + 1) / 2;

        for (int nSerie = 0; nSerie < datasetGroup.series().count(); ++nSerie)
        {
            if (nSerie % 2 == 0)
            {
                ColorsGenerator::getColor(nSerieEven, 0, rgbColor, bDrab);
                nSerieEven++;
            }
            else
            {
                ColorsGenerator::getColor(nSerieOdd, 0, rgbColor, bDrab);
                nSerieOdd++;
            }

            vecColor[nSerie] = rgbColor;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportChart::exportToHtml(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const
//
// Description	:	Export the chart into a html file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportChart::exportToHtml(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const
{
    int											uiIndex, uiNbMergedLabels, uiMergedIndex;
    int											nTotalSerieDisplayed = 0;
    GexAdvancedEnterpriseReportChartYAxis *		pYAxis = NULL;
    QString of=ReportOptions.GetOption("output", "format").toString();

    QStringList displayedAggregatesValues;
    BYTE        displayedAggregates[datasetGroup.aggregateValues().count()];

    for(int nDA = 0; nDA < datasetGroup.aggregateValues().count(); ++nDA)
        displayedAggregates[nDA] = 0;

    QMapIterator<int, GexAdvancedEnterpriseReportChartYAxis *> itMapYAxis(m_mapYAxis);
    while (itMapYAxis.hasNext())
    {
        itMapYAxis.next();
        pYAxis = itMapYAxis.value();

        QList<GexDbPluginERDatasetSerie>::const_iterator		itSerie		= datasetGroup.series().begin();

        m_aerScriptEngine.fillScriptGroupVariables(datasetGroup);

        while (itSerie != datasetGroup.series().end())
        {
            m_aerScriptEngine.fillScriptSerieVariables(*itSerie);

            for (int nItem = 0; nItem < pYAxis->series().count(); ++nItem)
            {
                if ((pYAxis->series().at(nItem)->printWhen().isEmpty() || m_aerScriptEngine.scriptEngine()->evaluate(pYAxis->series().at(nItem)->printWhen()).toBoolean()))
                {
                    QList<GexDbPluginERDatasetRow>::const_iterator	itAggregate = (*itSerie).aggregates().begin();

                    while (itAggregate != (*itSerie).aggregates().end())
                    {
                        uiMergedIndex	= datasetGroup.aggregateValues().indexOf((*itAggregate).aggregate());

                        if (uiMergedIndex != -1)
                        {
                            m_aerScriptEngine.fillScriptSerieVariables(*itSerie, (*itAggregate).aggregate());

                            if ((pYAxis->series().at(nItem)->printDataWhen().isEmpty() || m_aerScriptEngine.scriptEngine()->evaluate(pYAxis->series().at(nItem)->printDataWhen()).toBoolean()))
                                displayedAggregates[uiMergedIndex] = 1;
                        }

                        ++itAggregate;
                    }
                }
            }
            ++itSerie;
        }
    }

    for (int nAggreg = 0; nAggreg < datasetGroup.aggregateValues().count(); ++nAggreg)
    {
        if (displayedAggregates[nAggreg])
            displayedAggregatesValues.append(datasetGroup.aggregateValues().at(nAggreg));
    }

    // Fill group script variables
    m_aerScriptEngine.fillScriptGroupVariables(datasetGroup);

    if (m_pXAxis->limit() > 0 && m_pXAxis->limit() < displayedAggregatesValues.count())
    {
        txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"1\">" << endl;
        txtStream << "<tr><td align=\"center\"><font size=\"4\">Chart limited to " << m_pXAxis->limit() << " out of " << displayedAggregatesValues.count() << "</font></td></tr>" << endl;
        txtStream << "</table>" << endl;
    }

    // Create a XYChart object of size 800 x 500 pixels.
    XYChart * pChart = new XYChart(800,500);

    // Compute background colors
    int nBgColor = Chart::Transparent, nAltBgColor = -1;

    // Compute X-Axis label merge
    uiNbMergedLabels = (m_pXAxis->limit() > 0) ? qMin(m_pXAxis->limit(), displayedAggregatesValues.count()) : displayedAggregatesValues.count();

    // Create list of short labels
    QStringList strlLabelMerge_Short;
    QString		strLabel, strName;

    if (m_pXAxis == NULL || m_pXAxis->expressionLabel().isEmpty())
    {
        for(uiIndex = 0; uiIndex < displayedAggregatesValues.count(); uiIndex++)
        {
            strLabel = displayedAggregatesValues.at(uiIndex);
            if(m_pXAxis->maxLabelSize() > 3 && strLabel.length() > m_pXAxis->maxLabelSize())
            {
                strLabel = strLabel.left(m_pXAxis->maxLabelSize()-3);
                strLabel += "...";
            }
            strlLabelMerge_Short.append(strLabel);
        }
    }
    else
    {
        for(uiIndex = 0; uiIndex < uiNbMergedLabels; uiIndex++)
        {
            m_aerScriptEngine.fillScriptGroupVariables(datasetGroup, displayedAggregatesValues.at(uiIndex));

            strLabel = m_aerScriptEngine.scriptEngine()->evaluate(m_pXAxis->expressionLabel()).toString();

            if(m_pXAxis->maxLabelSize() > 3 && strLabel.length() > m_pXAxis->maxLabelSize())
            {
                strLabel = strLabel.left(m_pXAxis->maxLabelSize()-3);
                strLabel += "...";
            }
            strlLabelMerge_Short.append(strLabel);
        }
    }

    // Allocate mem space for merged label and data pointers
    char **		pcLabels	= new char*[uiNbMergedLabels];
    double *	pYAxisData	= new double[uiNbMergedLabels];

    // Draw the ticks between label positions (instead of at label positions)
    pChart->xAxis()->setTickOffset(0.5);

    int		nYAxis = 0;

    Layer *	pDataLayer	= NULL;
    Axis *	pChartYAxis	= NULL;

    itMapYAxis.toFront();
    while (itMapYAxis.hasNext())
    {
        double			dMaxValue = 0.0;
        QVector<QColor> vecColor;

        // Fill color array
        fillSerieColor(vecColor, datasetGroup, (nYAxis == 0) ? false: true);

        itMapYAxis.next();

        pYAxis = itMapYAxis.value();

        if (pYAxis->type() == GexAdvancedEnterpriseReportChartYAxis::TypeLine)
        {
            // Line Layer initialization
            LineLayer * pLineLayer = pChart->addLineLayer();
            pLineLayer->setGapColor(Chart::SameAsMainColor);
            pDataLayer = pLineLayer;

            pDataLayer->moveFront(NULL);
        }
        else if (pYAxis->type() == GexAdvancedEnterpriseReportChartYAxis::TypeBars)
        {
            // Bar layer initialization
            BarLayer * pBarLayer = pChart->addBarLayer(Chart::Side);
            pBarLayer->setBarGap(0.2, Chart::TouchBar);
            pDataLayer = pBarLayer;
        }
        else if (pYAxis->type() == GexAdvancedEnterpriseReportChartYAxis::TypeStackedBars)
        {
            // Bar layer initialization
            BarLayer * pBarLayer = pChart->addBarLayer(Chart::Stack);
            pBarLayer->setBarGap(0.2, Chart::TouchBar);
            pDataLayer = pBarLayer;
        }

        if (pYAxis->pos() == GexAdvancedEnterpriseReportChartYAxis::Right)
            pChartYAxis = pChart->yAxis2();
        else
            pChartYAxis = pChart->yAxis();

        int														nSerieCount	= 0;
        DataSet *												pDataset	= NULL;
        QList<GexDbPluginERDatasetSerie>::const_iterator		itSerie		= datasetGroup.series().begin();

        m_aerScriptEngine.fillScriptGroupVariables(datasetGroup);

        while (itSerie != datasetGroup.series().end())
        {
            m_aerScriptEngine.fillScriptSerieVariables(*itSerie);

            for (int nItem = 0; nItem < pYAxis->series().count(); ++nItem)
            {
                if ((pYAxis->series().at(nItem)->printWhen().isEmpty() || m_aerScriptEngine.scriptEngine()->evaluate(pYAxis->series().at(nItem)->printWhen()).toBoolean()))
                {
                    // Compute yield
                    for(uiIndex = 0; uiIndex < uiNbMergedLabels; uiIndex++)
                        pYAxisData[uiIndex] = Chart::NoValue;

                    QList<GexDbPluginERDatasetRow>::const_iterator	itAggregate = (*itSerie).aggregates().begin();

                    while (itAggregate != (*itSerie).aggregates().end())
                    {
                        uiMergedIndex	= displayedAggregatesValues.indexOf((*itAggregate).aggregate());

                        if (uiMergedIndex != -1  && uiMergedIndex < uiNbMergedLabels && (*itAggregate).volume() > 0)
                        {
                            m_aerScriptEngine.fillScriptSerieVariables(*itSerie, (*itAggregate).aggregate());

                            pYAxisData[uiMergedIndex]	= m_aerScriptEngine.scriptEngine()->evaluate(pYAxis->series().at(nItem)->value()).toNumber();
                            dMaxValue					= qMax(dMaxValue, pYAxisData[uiMergedIndex]);
                        }

                        ++itAggregate;
                    }

                    strName	= m_aerScriptEngine.scriptEngine()->evaluate(pYAxis->series().at(nItem)->name()).toString();

                    pDataset = pDataLayer->addDataSet(DoubleArray(pYAxisData, uiNbMergedLabels), vecColor.at(nSerieCount).rgb() & 0xffffff, strName.toLatin1().constData());

                    if (pYAxis->type() == GexAdvancedEnterpriseReportChartYAxis::TypeLine)
                        pDataset->setDataSymbol(Chart::SquareSymbol, 7);

                    // Total serie displayed
                    ++nTotalSerieDisplayed;
                }
            }

            ++nSerieCount;
            ++itSerie;
        }

        // Format Y-Axis name
        QString strYAxisName = pYAxis->name();

        if(dMaxValue > 1000000000000.0)
        {
            pChartYAxis->setLabelFormat("{={value}/1000000000000}");
            strYAxisName += " (T)";
        }
        else if(dMaxValue > 1000000000.0)
        {
            pChartYAxis->setLabelFormat("{={value}/1000000000}");
            strYAxisName += " (G)";
        }
        else if(dMaxValue > 1000000.0)
        {
            pChartYAxis->setLabelFormat("{={value}/1000000}");
            strYAxisName += " (M)";
        }
        else if(dMaxValue > 1000.0)
        {
            pChartYAxis->setLabelFormat("{={value}/1000}");
            strYAxisName += " (K)";
        }
        else
            pChartYAxis->setLabelFormat("{={value}}");

        pDataLayer->setUseYAxis(pChartYAxis);
        pChartYAxis->setTitle(strYAxisName.toLatin1().constData());

        if (pYAxis->min() != -C_INFINITE && pYAxis->max() != C_INFINITE)
            pChartYAxis->setLinearScale(pYAxis->min(), pYAxis->max());
        else
            pChartYAxis->setAutoScale(0.1, 0.1, 0.8);

        if (pYAxis->zOrder() == GexAdvancedEnterpriseReportChartYAxis::Back)
            pDataLayer->moveBack(NULL);
        else if (pYAxis->zOrder() == GexAdvancedEnterpriseReportChartYAxis::Front)
            pDataLayer->moveFront(NULL);

        ++nYAxis;
    }

    // Init merged label pointer
    for(uiIndex=0; uiIndex < uiNbMergedLabels; uiIndex++)
    {
        pcLabels[uiIndex] = new char[strlLabelMerge_Short[uiIndex].size()+1];

        strcpy(pcLabels[uiIndex], strlLabelMerge_Short[uiIndex].toLatin1().constData());
    }

    // Set the labels on the x axis.
    pChart->xAxis()->setLabelStyle(0, 7.5, 0, 30);
    pChart->xAxis()->setLabels(StringArray(pcLabels, uiNbMergedLabels));

    // Compute plot area
    QRect	clPlotRect;
    int		nStartY;

    nStartY = 5;

    clPlotRect.setRect(70, nStartY, 640, 500-nStartY);

    // Add a title to the x axis
    if (m_aerScriptEngine.scriptEngine()->canEvaluate(m_pXAxis->name()))
    {
        pChart->xAxis()->setTitle(m_aerScriptEngine.scriptEngine()->evaluate(m_pXAxis->name())
                                  .toString().toLatin1().constData());

        if (m_aerScriptEngine.scriptEngine()->hasUncaughtException())
            pChart->xAxis()->setTitle(m_pXAxis->name().toLatin1().constData());
    }
    else
        pChart->xAxis()->setTitle(m_pXAxis->name().toLatin1().constData());

    pChart->xAxis()->setMargin(10, 10);

    // Add Legend Box to the graph
    if (m_eLegendMode != LegendOff)
    {
        // Add a legend box
        LegendBox *pLegendBox = pChart->addLegend(0, 0, false, "arialbi.ttf", 8);

        if (nTotalSerieDisplayed > 5)
            pLegendBox->setCols(Chart::AutoGrid);

        pLegendBox->setBackground(Chart::Transparent, 0x000000);

        pChart->layoutLegend();

        // If mode is auto, do not display the legend if it takes more than 33% of the total chart space.
        if (m_eLegendMode == LegendAuto && pLegendBox->getHeight() > 165)
        {
            pLegendBox->setPos(-9999999, -9999999);

            TextBox * pTextBox = pChart->addText(0, 455, "Legend has been automatically disabled", "arialbi.ttf", 8, Chart::TextColor, Chart::Center);
            pTextBox->setAlignment(Chart::Center);
            pTextBox->setSize(800, 40);
            pTextBox->setBackground(Chart::Transparent, QColor(Qt::black).rgb() & 0xffffff);
            clPlotRect.setRect(70, nStartY, 640, 445 - nStartY);
        }
        else
        {
            clPlotRect.setRect(70, nStartY, 640, 500 - nStartY - pLegendBox->getHeight());
            pLegendBox->setPos((800-pLegendBox->getWidth()) / 2, clPlotRect.bottom());
        }
    }

    // Set plot area
    pChart->setClipping();
    pChart->setPlotArea(clPlotRect.left(), clPlotRect.top(), clPlotRect.width() , clPlotRect.height(), nBgColor, nAltBgColor);
    pChart->packPlotArea(5, clPlotRect.top(), 795, clPlotRect.height());

    // output the chart

    // Build base names for image files to store the charts
    QString strChartFilename	= QString("sql_image_g%1_s%2.png").arg(datasetGroup.groupId()).arg(sectionID());
    QString strChartDirectory	= QString("%1/images/").arg(ReportOptions.strReportDirectory.toLatin1().constData()) + strChartFilename;
    pChart->makeChart(strChartDirectory .toLatin1().constData());

    // Write HTML code to insert created image
    txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"1\">" << endl;

    // add a link to update bin colors from RC
    if (datasetGroup.binGroupBy() == GexDbPluginERDatasetRow::BinGroupBySerie && of == "HTML")
        txtStream << "<tr><td align=\"left\"> <a href=\"#_gex_bin_colors.htm\"><img src=\"../images/color_palette.png\" border=\"0\"><b>Edit Binnings Colors</b></a></td></tr><br>" << endl;

    txtStream << "<tr><td align=\"center\">" << "<img border=\"0\" src=\"../images/" << formatHtmlImageFilename(strChartFilename) << "\"></img>" << "</td></tr>" << endl;
    txtStream << "</table>" << endl;

    //free up resources
    for (int nLabelIndex = 0; nLabelIndex < uiNbMergedLabels; nLabelIndex++)
    {
        if (pcLabels[nLabelIndex])
        {
            delete pcLabels[nLabelIndex];
            pcLabels[nLabelIndex] = NULL;
        }
    }

    delete [] pcLabels;
    delete pChart;
    pcLabels    = NULL;
    pChart      = 0;

    // Close section
    closeSection("Advanced Chart");
}
