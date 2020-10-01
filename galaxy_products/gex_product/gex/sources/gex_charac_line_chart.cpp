///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gex_charac_line_chart.h"
#include "gex_report.h"
#include "gex_group_of_files.h"
#include <gqtl_global.h>
#include <gqtl_log.h>
#include "charac_line_chart_template.h"

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern double			ScalingPower(int iPower);

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport *     gexReport;

namespace GS
{
namespace Gex
{

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CharacLineChart
//
// Description	:	chart class to draw the line charts into characterization
//                  report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CharacLineChart::CharacLineChart(int sizeMode, GexWizardChart * lWizardParent, CGexChartOverlays * pChartOverlays)
    : GexAbstractChart(GexAbstractChart::chartTypeCharacLine, sizeMode, lWizardParent, pChartOverlays),
      mLabels(NULL)
{

}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CharacLineChart::~CharacLineChart()
{
    CleanLabels();
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
QString CharacLineChart::makeTooltip()
{
    QString	strTooltip;

    return strTooltip;
}

void CharacLineChart::setViewportModeFromChartMode(int nChartMode)
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

void CharacLineChart::SetSerieVariables(const QStringList &variables)
{
    mSerieVariable = variables;
}

void CharacLineChart::drawXAxisLabels()
{
    if (mAggregateVariable.count() > 0)
    {
        CDMLTable * cmdlTable = m_pXYChart->xAxis()->makeLabelTable();

        // Set the default top/bottom margins of the cells to 3 pixels
        cmdlTable->getStyle()->setMargin(0, 0, 3, 3);
        cmdlTable->getStyle()->setFontStyle("arialbd.ttf");

        while (cmdlTable->getRowCount() < mAggregateVariable.count())
            cmdlTable->appendRow();

        int                 colIndex        = 0;
        int                 colWidth        = 1;
        QString             conditionName;
        QString             conditionValue;
        QString             lValue;
        QListIterator<QMap<QString, QString> > itAggregate(mAggregate);

        // Insert new column to the left to display condition name
        cmdlTable->insertCol(0)->setMargin(2, 2, 0, 0);

        for (int rowIndex = 0; rowIndex < cmdlTable->getRowCount(); ++rowIndex)
        {
            colIndex = 0;
            itAggregate.toFront();

            conditionName = mAggregateVariable.at(rowIndex);
            conditionValue.clear();

            // Set name of the condition
            cmdlTable->setText(colIndex, rowIndex, conditionName.toLatin1().constData());

            ++colIndex;

            while (itAggregate.hasNext())
            {
                lValue = itAggregate.next().value(conditionName);

                if (conditionValue.isEmpty())
                {
                    conditionValue = lValue;
                    colWidth = 1;
                }
                else if (conditionValue != lValue)
                {
                    cmdlTable->setCell(colIndex, rowIndex, colWidth, 1, conditionValue.toLatin1().constData());

                    conditionValue = lValue;
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

void CharacLineChart::computeDataset()
{
    // Count the total number of VISIBLE layers
    CGexGroupOfFiles *	pGroup;
    CGexFileInGroup *	pFile;
    CTest *				ptTestCell;				// Pointer to test cell of groups2 or higher
    long				lTestNumber;
    long				lPinmapIndex;
    int                 indexGroup = 0;
    int                 lIdxAggregate = 0;

    QString             optionsScaling  = m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    QString             optionsVariable = m_pReportOptions->GetOption("adv_charac2","variable").toString();

    // Clear previous dataset
    CleanLabels();

    // Create Dataset from template
    CreateDatasetsFromTemplate();

    QListIterator<CGexGroupOfFiles*>    itGroupsList(gexReport->getGroupsList());

    itGroupsList.toFront();
    while (itGroupsList.hasNext())
    {
        // Find test cell: RESET list to ensure we scan list of the right group!
        // Check if pinmap index...
        pGroup			= itGroupsList.next();
        pFile			= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        lTestNumber		= referenceTestX()->lTestNumber;
        lPinmapIndex    = referenceTestX()->lPinmapIndex;

        QMap<QString, QString>  lAggregateKey;
        QString                 lSerieKey;

        // Build aggregates
        foreach(const QString &lVariable, mAggregateVariable)
        {
            lAggregateKey.insert(lVariable,
                                pGroup->GetTestConditionsValue(lVariable).toString());
        }

        // Build serie key
        foreach(const QString &serie, mSerieVariable)
        {
            if (lSerieKey.isEmpty() == false)
                lSerieKey += ".";

            lSerieKey += serie + "[" + pGroup->GetTestConditionsValue(serie).toString() + "]";
        }

        // Find index of the aggregate
        lIdxAggregate = mAggregate.indexOf(lAggregateKey);

        // Find index of the serie
        int idxDataset = mDatasets.indexOf(privateDataset(lSerieKey));

        if (idxDataset == -1)
        {
            privateDataset newDataset(lSerieKey, mAggregate.count());

            newDataset.SetName(lSerieKey);
            newDataset.SetVariable(optionsVariable);

            mDatasets.append(newDataset);

            idxDataset = mDatasets.count() - 1;
        }

        if(pFile && pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,referenceTestX()->strTestName) == 1 &&
           ptTestCell->m_testResult.count() > 0)
        {
            // If multiple charts (groups or overlays), scale based on 1st parameter in 1st group.
            if (mInternalChartInfo.m_bTestReferenceScaleFactor == false)
            {
                mInternalChartInfo.m_pTestReferenceScaleX        = ptTestCell;
                mInternalChartInfo.m_nTestReferenceScaleFactor   = ptTestCell->res_scal;
                mInternalChartInfo.m_bTestReferenceScaleFactor   = true;
            }

            // Compute scale factor comparing to the ref group
            double lCustomScaleFactor = 1.0 / GS_POW(10.0, (ptTestCell->res_scal - mInternalChartInfo.m_nTestReferenceScaleFactor));

            // Apply the scale factor use for the test result
            lCustomScaleFactor /= ScalingPower(ptTestCell->res_scal);

            if (mDatasets[idxDataset].GetVariable().compare("median", Qt::CaseInsensitive) == 0)
            {
                if (ptTestCell->lfSamplesQuartile2 != -C_INFINITE)
                    mDatasets[idxDataset].SetData(lIdxAggregate, ptTestCell->lfSamplesQuartile2 * lCustomScaleFactor);
            }
            else
                mDatasets[idxDataset].SetData(lIdxAggregate, ptTestCell->lfMean * lCustomScaleFactor);

            // Compute low limit markers in Y
            if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0))
            {
                // Low limit exists
                double dTmpLimit = ptTestCell->GetCurrentLimitItem()->lfLowLimit;

                // If we have to keep values in normalized format, do not rescale!
                if (optionsScaling != "normalized")
                {
                    // convert LowLimit to same scale as results:
                    pFile->FormatTestResultNoUnits(&dTmpLimit, ptTestCell->llm_scal);

                    dTmpLimit *=  ScalingPower(ptTestCell->llm_scal);	// normalized
                    dTmpLimit /=  ScalingPower(ptTestCell->res_scal);	// normalized
                }

                mInternalChartInfo.m_dLowLimitY = qMin(mInternalChartInfo.m_dLowLimitY, dTmpLimit);
            }

            // Compute high limit markers in X
            if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
            {
                // Low limit exists
                double dTmpLimit = ptTestCell->GetCurrentLimitItem()->lfHighLimit;

                // If we have to keep values in normalized format, do not rescale!
                if (optionsScaling != "normalized")
                {
                    // convert LowLimit to same scale as results:
                    pFile->FormatTestResultNoUnits(&dTmpLimit, ptTestCell->hlm_scal);

                    dTmpLimit *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
                    dTmpLimit /=  ScalingPower(ptTestCell->res_scal);	// normalized
                }

                mInternalChartInfo.m_dHighLimitY = qMax(mInternalChartInfo.m_dHighLimitY, dTmpLimit);
            }
        }

        QString testName;
        gexReport->BuildTestNameString(pFile, ptTestCell, testName);

        // Format test name to display
        testName = gexReport->buildDisplayName(testName, false);
        mInternalChartInfo.m_strTitle = QString("Test %1: %2").arg(ptTestCell->szTestLabel).arg(testName);

        indexGroup++;
    }

    // Create label array
    mLabels = new char*[mAggregate.count()];

    memset(mLabels, 0, mAggregate.count() * sizeof(char*));

    for (int lIdx = 0; lIdx < mAggregate.count(); ++lIdx)
    {
        QString lLabel;

        foreach(const QString &lKey, mAggregate.at(lIdx).keys())
        {
            if (lLabel.isEmpty() == false)
                lLabel += ",";

            lLabel += mAggregate.at(lIdx).value(lKey);
        }

        mLabels[lIdx] = new char[lLabel.size()+1];

        strcpy(mLabels[lIdx], lLabel.toLatin1().constData());
    }
}

void CharacLineChart::keepXTestViewport()
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
// Name			:	void drawTitles()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void CharacLineChart::drawTitles()
{
    // this should never happened
    if (mInternalChartInfo.m_pTestReferenceScaleX)
    {
        double      lCustomScaleXFactor = 1.0;
        QString     lLabel;
        QString     lUnits;

        // We have a chart title unless the chart size in only 200 pixel wide (SMALL chart mode)!
        if ((m_nSizeMode != GEX_CHARTSIZE_SMALL))
        {
            lLabel = mInternalChartInfo.m_strTitle;

            if (mInternalChartInfo.m_bTestReferenceScaleFactor == false)
                lLabel += "<*color=FF0000*> - No data samples<*/color*>";

            // Multilayrs, check if we have assigned it a title...
            if(m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bChartTitle)
                lLabel = m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.strChartTitle;	// custom title.

            m_pXYChart->addTitle(lLabel.toLatin1().constData());
        }

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

        // Add a legend box and anchored the top center at the horizontal center of the
        // chart, just under the title. Use 10pts Arial Bold as font, with transparent
        // background and border.
        LegendBox * lLegendBox = m_pXYChart->addLegend(m_pXYChart->getWidth() / 2,
                                                       topMargin(), false,
                                                       "arialbd.ttf", 10);
        lLegendBox->setAlignment(Chart::TopCenter);
        lLegendBox->setBackground(Chart::Transparent, Chart::Transparent);

        // Layout the legend to get its size
        m_pXYChart->layoutLegend();

        setTopMargin(topMargin() + lLegendBox->getHeight());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void buildOwnerChart()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void CharacLineChart::buildOwnerChart()
{
    // Draw layers
    drawLayers();

    // Draw chart and axis title
    drawTitles();

    // Draw x-axis labels
    drawXAxisLabels();

    // Draw limits markers
    drawMarkersLimits();

    // Scale chart
    scaleChart();
}

bool CharacLineChart::drawLayers()
{
    LineLayer * pLayer      = NULL;
    DataSet *   pDataset    = NULL;

    for (int idx = 0; idx < mDatasets.count(); ++idx)
    {
        if (mDatasets[idx].GetSize() > 0 && mDatasets[idx].GetDataArray() != NULL)
        {
            pLayer = m_pXYChart->addLineLayer();

            pDataset = pLayer->addDataSet(DoubleArray(mDatasets[idx].GetDataArray(), mDatasets[idx].GetSize()),
                                          mDatasets[idx].GetColor(),
                                          mDatasets[idx].GetName().toLatin1().constData());

            pDataset->setDataSymbol(Chart::CircleSymbol, 6);

            // Compute min low and high X/Y values
            mInternalChartInfo.m_dLowY  = qMin(mInternalChartInfo.m_dLowY, mDatasets[idx].GetMinData());
            mInternalChartInfo.m_dHighY = qMax(mInternalChartInfo.m_dHighY, mDatasets[idx].GetMaxData());
        }
    }

    if (mLabels)
        m_pXYChart->xAxis()->setLabels(StringArray(mLabels, mAggregate.count()));




    return true;
}

void CharacLineChart::scaleChart()
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

void CharacLineChart::CleanLabels()
{
    if (mLabels)
    {
        for (int nLabelIndex = 0; nLabelIndex < mAggregate.count(); nLabelIndex++)
        {
            if (mLabels[nLabelIndex])
            {
                delete mLabels[nLabelIndex];
                mLabels[nLabelIndex] = NULL;
            }
        }

        delete [] mLabels;
        mLabels = NULL;
    }
}

void CharacLineChart::fitPlotArea()
{
    m_pXYChart->packPlotArea(0, top(), 0, bottom());

    setBottomMargin(bottomMargin() + m_pXYChart->xAxis()->getThickness());

    int nLeftMargin = leftMargin();
    int nTopMargin  =  topMargin();

    int nAreaWidth  = width() - horizontalMargin();
    int nAreaHeight = height() - verticalMargin();

    m_pXYChart->setPlotArea(nLeftMargin, nTopMargin, nAreaWidth, nAreaHeight);
}

void CharacLineChart::CreateDatasetsFromTemplate()
{
    mDatasets.clear();
    mAggregate.clear();
    mAggregateVariable.clear();

    if (m_pReportOptions->mLineChartTemplate)
        mSerieVariable = m_pReportOptions->mLineChartTemplate->GetSerieConditions();

    // Build variables list to use as aggregate
    for (int lIdx = 0; lIdx < gexReport->GetTestConditionsCount(); ++lIdx)
    {
        QString lAggregateName = gexReport->GetTestConditions(lIdx);

        // If no serie variable defined, use the first variable of the aggregate instead
        if (mSerieVariable.isEmpty())
            mSerieVariable.append(lAggregateName);

        // If test condition doesn't belong to the serie variable, it as to
        // be used for aggregate
        if (mSerieVariable.indexOf(lAggregateName) == -1)
            mAggregateVariable.append(lAggregateName);
    }

    QListIterator<CGexGroupOfFiles*>    itGroupsList(gexReport->getGroupsList());
    CGexGroupOfFiles *                  pGroup = NULL;

    // Find aggregates values
    while (itGroupsList.hasNext())
    {
        QMap<QString, QString>  lAggregate;
        pGroup			= itGroupsList.next();

        // Build aggregates
        foreach(const QString &lVariable, mAggregateVariable)
        {
            lAggregate.insert(lVariable,
                                pGroup->GetTestConditionsValue(lVariable).toString());
        }

        if (mAggregate.indexOf(lAggregate) == -1)
            mAggregate.append(lAggregate);
    }

    if (m_pReportOptions->mLineChartTemplate)
    {
        for (int lIdx = 0; lIdx < m_pReportOptions->mLineChartTemplate->GetSerieDefinitionCount(); ++lIdx)
        {
            GS::Gex::CharacLineChartSerie   serie = m_pReportOptions->mLineChartTemplate->GetSerieDefinitionAt(lIdx);
            QString                         lSerieKey;
            QString                         lSerieName;

            for (int lIdx = 0; lIdx < serie.GetConditions().count(); ++lIdx)
            {
                // Build serie key
                if (lSerieKey.isEmpty() == false)
                    lSerieKey += ".";

                lSerieKey += m_pReportOptions->mLineChartTemplate->GetSerieConditions().at(lIdx);
                lSerieKey += "[" + serie.GetConditions().at(lIdx) + "]";
            }

            if (serie.GetName().isEmpty())
                lSerieName = lSerieKey;
            else
                lSerieName = serie.GetName();

            // Find index of the serie
            int idxDataset = mDatasets.indexOf(privateDataset(lSerieKey));

            if (idxDataset == -1)
            {
                privateDataset newDataset(lSerieKey, mAggregate.count());

                newDataset.SetName(lSerieName);
                newDataset.SetColor(serie.GetColor().rgb() & 0xffffff);
                newDataset.SetVariable(serie.GetVariable());

                mDatasets.append(newDataset);
            }
        }
    }
}

void CharacLineChart::drawMarkersLimits()
{
    QString         optionsMarkers  = m_pReportOptions->GetOption("adv_charac2","marker").toString();
    QString         strLabel;
    QColor			cPenColor;
    int				cPenWidth;

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
            if (mInternalChartInfo.m_dLowLimitY != C_INFINITE)
            {
                strLabel = QString("%1").arg(m_strLowLimit);

                addMarker(m_pXYChart->yAxis(), mInternalChartInfo.m_dLowLimitY,
                          cPenColor.rgb() & 0xffffff, strLabel, cPenWidth,
                          Chart::TopLeft);
            }

            if (mInternalChartInfo.m_dHighLimitY != -C_INFINITE)
            {
                strLabel = QString("%1").arg(m_strHighLimit);

                addMarker(m_pXYChart->yAxis(), mInternalChartInfo.m_dHighLimitY,
                          cPenColor.rgb() & 0xffffff, strLabel, cPenWidth,
                          Chart::BottomLeft);
            }
        }
    }
}

CharacLineChart::privateDataset::privateDataset(const QString &key)
    : mKey(key), mDataArray(NULL), mSize(0), mColor(-1), mMinData(C_INFINITE),
        mMaxData(-C_INFINITE)
{
}

CharacLineChart::privateDataset::privateDataset(const QString &key, int size)
    : mKey(key), mDataArray(NULL),mSize(size), mColor(-1), mMinData(C_INFINITE),
      mMaxData(-C_INFINITE)
{
    if (mSize > 0 )
    {
        mDataArray  = new double[size];

        for(int lIdx = 0; lIdx < size; ++lIdx)
            mDataArray[lIdx] = GEX_C_DOUBLE_NAN;
    }
}

CharacLineChart::privateDataset::privateDataset(const CharacLineChart::privateDataset &other)
    : mDataArray(NULL), mSize(-1), mColor(0)
{
    *this = other;
}

CharacLineChart::privateDataset::~privateDataset()
{
    if (mDataArray)
    {
        delete [] mDataArray;
        mDataArray = NULL;
    }
}

bool CharacLineChart::privateDataset::Resize(int size)
{
    if (mSize == 0)
    {
        mDataArray  = new double[size];
        mSize       = size;

        for(int lIdx = 0; lIdx < size; ++lIdx)
            mDataArray[lIdx] = GEX_C_DOUBLE_NAN;
    }
    else
    {
        if (size > mSize)
        {
            double * oldArray = mDataArray;

            mDataArray  = new double[size];

            memcpy(mDataArray, oldArray, mSize * sizeof(double));

            for(int lIdx = mSize; lIdx < size; ++lIdx)
                mDataArray[lIdx] = GEX_C_DOUBLE_NAN;

            delete [] oldArray;
        }

        mSize = size;
    }

    return (mDataArray != NULL);
}

CharacLineChart::privateDataset &CharacLineChart::privateDataset::operator =(const CharacLineChart::privateDataset &other)
{
    if (this != &other)
    {
        if (mDataArray)
        {
            delete mDataArray;
            mDataArray = NULL;
        }

        mKey        = other.mKey;
        mName       = other.mName;
        mVariable   = other.mVariable;
        mSize       = other.mSize;
        mColor      = other.mColor;
        mMinData    = other.mMinData;
        mMaxData    = other.mMaxData;

        if (mSize > 0)
        {
            mDataArray  = new double[mSize];

            if (other.mDataArray)
                memcpy(mDataArray, other.mDataArray, mSize * sizeof(double));
            else
            {
                for(int lIdx = 0; lIdx < mSize; ++lIdx)
                    mDataArray[lIdx] = GEX_C_DOUBLE_NAN;
            }
        }
        else
            mDataArray = NULL;
    }

    return *this;
}

bool CharacLineChart::privateDataset::operator ==(const CharacLineChart::privateDataset &other) const
{
    return (mKey == other.mKey);
}

const QString &CharacLineChart::privateDataset::GetVariable() const
{
    return mVariable;
}

const QString &CharacLineChart::privateDataset::GetName() const
{
    return mName;
}

const QString &CharacLineChart::privateDataset::GetKey() const
{
    return mKey;
}

int CharacLineChart::privateDataset::GetColor() const
{
    return mColor;
}

int CharacLineChart::privateDataset::GetSize() const
{
    return mSize;
}

double CharacLineChart::privateDataset::GetData(int index) const
{
    if (mDataArray && index >= 0 && index < mSize)
        return mDataArray[index];
    else
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Invalid index used");
        return 0.0;
    }
}

double CharacLineChart::privateDataset::GetMinData() const
{
    return mMinData;
}

double CharacLineChart::privateDataset::GetMaxData() const
{
    return mMaxData;
}

double *CharacLineChart::privateDataset::GetDataArray() const
{
    return mDataArray;
}

void CharacLineChart::privateDataset::SetColor(int color)
{
    mColor = color;
}

void CharacLineChart::privateDataset::SetData(int index, double value)
{
    if (mDataArray && index >= 0 && index < mSize)
    {
        mDataArray[index] = value;

        if (value != GEX_C_DOUBLE_NAN)
        {
            mMinData = qMin(mMinData, value);
            mMaxData = qMax(mMaxData, value);
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Invalid index used");
    }
}

void CharacLineChart::privateDataset::SetName(const QString &lName)
{
    mName = lName;
}

void CharacLineChart::privateDataset::SetVariable(const QString &lVariable)
{
    mVariable = lVariable;
}

} // namespace Gex
} // namespace GS
