///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_charac_histo_chart.h"
#include "gex_constants.h"
#include "interactive_charts.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include <gqtl_log.h>
#include "gex_box_plot_data.h"
#include <gqtl_global.h>
#include "charac_box_whisker_template.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QString>
#include <QMap>

///////////////////////////////////////////////////////////////////////////////////
// Std Includes
///////////////////////////////////////////////////////////////////////////////////
#include "limits"

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
// Name			:	GexCharacHistoChart
//
// Description	:	chart class to draw the histo charts into report or interactive mode
//                  for characterization
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexCharacHistoChart::GexCharacHistoChart(int nSizeMode, GexWizardChart * lWizardParent, CGexChartOverlays *pChartOverlays)
    : GexAbstractChart(GexAbstractChart::chartTypeCharacHisto, nSizeMode, lWizardParent, pChartOverlays)
{
    mClassesCount       = 30;
    mForcedBottomMargin = -1;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexCharacHistoChart::~GexCharacHistoChart()
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
QString GexCharacHistoChart::makeTooltip()
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
void GexCharacHistoChart::keepXTestViewport()
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
// Name			:	void setViewportModeFromChartMode()
//
// Description	:	convert chart mode from Gex into viewport mode
//
///////////////////////////////////////////////////////////////////////////////////
void GexCharacHistoChart::setViewportModeFromChartMode(int nChartMode)
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

void GexCharacHistoChart::ForceBottomMargin(int bottom)
{
    mForcedBottomMargin = bottom;
}

void GexCharacHistoChart::computeDataset()
{
    // Clear classes dataset
    mDatasets.clear();

    // Compute min/max samples value
    if (computeMinMax() && mClassesCount > 0)
    {
        // Compute the data range and data step
        double                  lDataRange   = mInternalChartInfo.m_dHighX - mInternalChartInfo.m_dLowX;
        QString                 lKey;
        CGexGroupOfFiles *      pGroup;
        CGexFileInGroup *       pFile;
        CTest *                 ptTestCell;				// Pointer to test cell of groups2 or higher
        long                    lTestNumber;
        long                    lPinmapIndex;

        double *                arrayData;

        QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());

        while (itGroupsList.hasNext())
        {
            pGroup = itGroupsList.next();

            int nFillColor, nLineColor;
            Chart::SymbolType layerSymbol;

            if (gexReport->GetTestConditionsCount() > 0)
            {
                QString mainCondition   = gexReport->GetTestConditions(0);
                lKey  = pGroup->GetTestConditionsValue(mainCondition).toString();
            }
            else
                lKey = pGroup->strGroupName;

            int idxDataset = mDatasets.indexOf(privateDataset(lKey));

            if (idxDataset == -1)
            {
                privateDataset newDataset(lKey, mClassesCount);

                // Get the color
                if (m_pReportOptions &&
                    m_pReportOptions->mBoxWhiskerTemplate &&
                    m_pReportOptions->mBoxWhiskerTemplate->GetTopLevelAggregates().contains(lKey))
                    nFillColor = m_pReportOptions->mBoxWhiskerTemplate->GetTopLevelColor(lKey).rgb() & 0xffffff;
                else
                    getLayerStyles(mDatasets.count()+1, NULL, nFillColor, nLineColor, layerSymbol);

                newDataset.setColor(nFillColor);

                mDatasets.append(newDataset);

                idxDataset = mDatasets.count() - 1;
            }

            arrayData = mDatasets.at(idxDataset).GetDataArray();

            GEX_ASSERT(arrayData != NULL);

            if (arrayData == NULL)
            {
                GSLOG(SYSLOG_SEV_CRITICAL, QString("No data array allocated for dataset : %1").arg( lKey.toLatin1().constData()).toLatin1().constData());
                continue;
            }

            pFile			= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            lTestNumber		= referenceTestX()->lTestNumber;
            lPinmapIndex    = referenceTestX()->lPinmapIndex;

            if(pFile && pFile->FindTestCell(lTestNumber,lPinmapIndex,&ptTestCell,false,false,referenceTestX()->strTestName)==1)
            {
                double lCustomScaleFactor = 1.0 / GS_POW(10.0, (ptTestCell->res_scal - mInternalChartInfo.m_nTestReferenceScaleFactor));

                for (int indexResult = 0; indexResult < ptTestCell->m_testResult.count(); ++indexResult)
                {
                    if (ptTestCell->m_testResult.isValidResultAt(indexResult))
                    {
                        double  value       = ptTestCell->m_testResult.resultAt(indexResult) * lCustomScaleFactor;
                        int     indexCell   = (int) (mClassesCount * (value - mInternalChartInfo.m_dLowX) / (lDataRange));

                        if (indexCell < 0)
                            indexCell = 0;
                        else if (indexCell >= mClassesCount-1)
                            indexCell = mClassesCount - 1;

                        arrayData[indexCell]++;
                    }
                }
            }
        }
    }
}

bool GexCharacHistoChart::computeMinMax()
{
    CGexGroupOfFiles *      pGroup;
    CGexFileInGroup *       pFile;
    CTest *                 ptTestCell;				// Pointer to test cell of groups2 or higher
    long                    lTestNumber;
    long                    lPinmapIndex;
    QString                 optionsScaling  = m_pReportOptions->GetOption("dataprocessing","scaling").toString();

    QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());

    while (itGroupsList.hasNext())
    {
        pGroup = itGroupsList.next();

        pFile			= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        lTestNumber		= referenceTestX()->lTestNumber;
        lPinmapIndex    = referenceTestX()->lPinmapIndex;

        if(pFile && pFile->FindTestCell(lTestNumber, lPinmapIndex, &ptTestCell, false, false, referenceTestX()->strTestName)==1)
        {
            // If multiple charts (groups or overlays), scale based on 1st parameter in 1st group.
            if (ptTestCell->m_testResult.count() > 0 && mInternalChartInfo.m_bTestReferenceScaleFactor == false)
            {
                mInternalChartInfo.m_pTestReferenceScaleX        = ptTestCell;
                mInternalChartInfo.m_nTestReferenceScaleFactor   = ptTestCell->res_scal;
                mInternalChartInfo.m_bTestReferenceScaleFactor   = true;
            }

            double lCustomScaleFactor = 1.0 / GS_POW(10.0, (ptTestCell->res_scal - mInternalChartInfo.m_nTestReferenceScaleFactor));

            // Get Min data
            double samplesMin = ptTestCell->lfSamplesMin / ScalingPower(ptTestCell->res_scal);
            mInternalChartInfo.m_dLowX = qMin(mInternalChartInfo.m_dLowX, samplesMin * lCustomScaleFactor);

            // Get max data
            double samplesMax = ptTestCell->lfSamplesMax / ScalingPower(ptTestCell->res_scal);
            mInternalChartInfo.m_dHighX = qMax(mInternalChartInfo.m_dHighX, samplesMax * lCustomScaleFactor);

            double dTmpLimit;

            // Compute low limit markers in X
            if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0))
            {
                // Low limit exists
                dTmpLimit = ptTestCell->GetCurrentLimitItem()->lfLowLimit;

                // If we have to keep values in normalized format, do not rescale!
                if (optionsScaling != "normalized")
                {
                    // convert LowLimit to same scale as results:
                    pFile->FormatTestResultNoUnits(&dTmpLimit, ptTestCell->llm_scal);

                    dTmpLimit *=  ScalingPower(ptTestCell->llm_scal);	// normalized
                    dTmpLimit /=  ScalingPower(ptTestCell->res_scal);	// normalized
                }

                dTmpLimit *= lCustomScaleFactor;

                mInternalChartInfo.m_dLowLimit = qMin(mInternalChartInfo.m_dLowLimit, dTmpLimit);
            }

            // Compute high limit markers in X
            if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
            {
                // Low limit exists
                dTmpLimit = ptTestCell->GetCurrentLimitItem()->lfHighLimit;

                // If we have to keep values in normalized format, do not rescale!
                if (optionsScaling != "normalized")
                {
                    // convert LowLimit to same scale as results:
                    pFile->FormatTestResultNoUnits(&dTmpLimit, ptTestCell->hlm_scal);

                    dTmpLimit *=  ScalingPower(ptTestCell->hlm_scal);	// normalized
                    dTmpLimit /=  ScalingPower(ptTestCell->res_scal);	// normalized
                }

                dTmpLimit *= lCustomScaleFactor;

                mInternalChartInfo.m_dHighLimit = qMax(mInternalChartInfo.m_dHighLimit, dTmpLimit);
            }

        }
    }

    return (mInternalChartInfo.m_dLowX != C_INFINITE && mInternalChartInfo.m_dHighX != -C_INFINITE);
}

void GexCharacHistoChart::buildOwnerChart()
{
    if (mDatasets.count())
    {
        double  lDataRange   = mInternalChartInfo.m_dHighX - mInternalChartInfo.m_dLowX;
        double  lDataStep    = lDataRange / mClassesCount;
        int     indexGroup   = 1;

        m_pXYChart->xAxis()->setIndent(false);

        // Create the Bar layer object, make the dataset stacked
        BarLayer * pLayer = m_pXYChart->addBarLayer(Chart::Stack);
        pLayer->setBarGap(Chart::TouchBar);

        QVector<double> stackedData(mClassesCount, 0.0);

        for(int index = 0; index < mDatasets.count(); ++index)
        {
            double *    arrayData = mDatasets.at(index).GetDataArray();
            int         fillColor = mDatasets.at(index).GetColor();

            GEX_ASSERT(arrayData != NULL);

            if (arrayData == NULL)
            {
                GSLOG(SYSLOG_SEV_CRITICAL, QString("No data array allocated for dataset : %1")
                      .arg( mDatasets.at(index).GetKey().toLatin1().constData()).toLatin1().constData());
                continue;
            }

            // Add one empty bar at the beginning and at the end to avoid
            // to have larger bar at the extremity
            double *    pXData = new double[mClassesCount+2];

            pXData[0] = Chart::NoValue;
            for (int indexData = 0; indexData < mClassesCount; ++indexData)
            {
                stackedData[indexData]  += arrayData[indexData];
                pXData[indexData+1]      = arrayData[indexData];
            }

            pXData[31] = Chart::NoValue;

            pLayer->addDataSet(DoubleArray(pXData, mClassesCount+2), fillColor);

            delete [] pXData;

            ++indexGroup;
        }

        // Set min/max x data
        pLayer->setXData(mInternalChartInfo.m_dLowX - lDataStep/2, mInternalChartInfo.m_dHighX + lDataStep/2);

        // Sort stacked data vector to get the max value
        qSort(stackedData);

        mInternalChartInfo.m_dHighY = stackedData.last();

        // Set margins
        if (mForcedBottomMargin >= 0)
            setBottomMargin(mForcedBottomMargin);
        else if (gexReport->GetTestConditionsCount() > 0)
            setBottomMargin(gexReport->GetTestConditionsCount()*20 + 20);

        setLeftMargin(0);
        setRightMargin(0);

        // Scale the chart
        scaleChart();

        // Swap x and y axis
        m_pXYChart->swapXY();
    }
}

void GexCharacHistoChart::scaleChart()
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

            if(mInternalChartInfo.m_dHighLimit != -C_INFINITE)
                dHighX = mInternalChartInfo.m_dHighLimit ;          // X High limit exists
            else
                dHighX = mInternalChartInfo.m_dHighX;

            break;

        case viewportOverData : // Chart over data samples!
            dLowX   = mInternalChartInfo.m_dLowX;
            dHighX  = mInternalChartInfo.m_dHighX;
            break;

        case viewportAdaptive :
            dLowX = mInternalChartInfo.m_dLowLimit;             // X Low limit exists
            dLowX = qMin(dLowX,mInternalChartInfo.m_dLowX);

            dHighX = mInternalChartInfo.m_dHighLimit ;          // X High limit exists
            dHighX = qMax(dHighX, mInternalChartInfo.m_dHighX);

            break;

    }

    dLowY   = 0;
    dHighY  = mInternalChartInfo.m_dHighY;

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
            ( m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeCharacHisto || m_pChartOverlays->getAppliedToChart() == -1)) || m_iXScaleType == 1)
        {
            // Log scale: then force it
            m_pXYChart->xAxis()->setLogScale(dLowX, dHighX, Chart::NoValue);
            m_iXScaleType = 1;
        }
        else if((m_pChartOverlays && !m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleX &&
               ( m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeCharacHisto || m_pChartOverlays->getAppliedToChart() == -1)) || m_iXScaleType == 0)
        {
            m_pXYChart->xAxis()->setLinearScale(dLowX, dHighX, Chart::NoValue);
            m_iXScaleType = 0;
        }
    }

    if (dLowY != C_INFINITE && dHighY != -C_INFINITE)
    {
        // Check if scale type is: Linear or Logarithmic
        if((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
            (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeCharacHisto || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 1)
        {
            // Log scale: then force it
            m_pXYChart->yAxis()->setLogScale(dLowY, dHighY, Chart::NoValue);
            m_iYScaleType = 1;
        }
        else if((m_pChartOverlays && m_pChartOverlays->getViewportRectangle()[type()].cChartOptions.bLogScaleY &&
               (m_pChartOverlays->getAppliedToChart() == GexAbstractChart::chartTypeCharacHisto || m_pChartOverlays->getAppliedToChart() == -1)) || m_iYScaleType == 0)
        {
            m_pXYChart->yAxis()->setLinearScale(dLowY, dHighY, Chart::NoValue);
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

GexCharacHistoChart::privateDataset::privateDataset(const QString &key)
    : mKey(key), mDataArray(NULL), mSize(0), mColor(0)
{
}

GexCharacHistoChart::privateDataset::privateDataset(const QString &key, int size)
    : mKey(key), mDataArray(NULL),mSize(size), mColor(0)
{
    if (mSize > 0 )
    {
        mDataArray  = new double[size];
        memset(mDataArray, 0, mSize * sizeof(double));
    }
}

GexCharacHistoChart::privateDataset::privateDataset(const GexCharacHistoChart::privateDataset &other)
    : mDataArray(NULL), mSize(0), mColor(0)
{
    *this = other;
}

GexCharacHistoChart::privateDataset::~privateDataset()
{
    if (mDataArray)
    {
        delete [] mDataArray;
        mDataArray = NULL;
    }
}

bool GexCharacHistoChart::privateDataset::resize(int size)
{
    if (mSize == 0)
    {
        mDataArray  = new double[size];
        mSize       = size;

        memset(mDataArray, 0, mSize * sizeof(double));
    }
    else
    {
        if (size > mSize)
        {
            double * oldArray = mDataArray;

            mDataArray  = new double[size];

            memcpy(mDataArray, oldArray, size * sizeof(double));
            memset(mDataArray+mSize, 0, (size - mSize) * sizeof(double));

            delete [] oldArray;
        }

        mSize = size;
    }

    return (mDataArray != NULL);
}

GexCharacHistoChart::privateDataset &GexCharacHistoChart::privateDataset::operator =(const GexCharacHistoChart::privateDataset &other)
{
    if (this != &other)
    {
        if (mDataArray)
        {
            delete mDataArray;
            mDataArray = NULL;
        }

        mKey        = other.mKey;
        mSize       = other.mSize;
        mColor      = other.mColor;

        if (mSize > 0)
        {
            mDataArray  = new double[mSize];

            if (other.mDataArray)
                memcpy(mDataArray, other.mDataArray, mSize * sizeof(double));
            else
                memset(mDataArray, 0, mSize * sizeof(double));
        }
        else
            mDataArray = NULL;
    }

    return *this;
}

bool GexCharacHistoChart::privateDataset::operator ==(const GexCharacHistoChart::privateDataset &other) const
{
    return (mKey == other.mKey);
}

const QString &GexCharacHistoChart::privateDataset::GetKey() const
{
    return mKey;
}

int GexCharacHistoChart::privateDataset::GetColor() const
{
    return mColor;
}

int GexCharacHistoChart::privateDataset::GetSize() const
{
    return mSize;
}

double GexCharacHistoChart::privateDataset::GetData(int index) const
{
    if (mDataArray && index > 0 && index < mSize)
        return mDataArray[index];
    else
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Invalid index used");
        return 0.0;
    }
}

double *GexCharacHistoChart::privateDataset::GetDataArray() const
{
    return mDataArray;
}

void GexCharacHistoChart::privateDataset::setColor(int color)
{
    mColor = color;
}

void GexCharacHistoChart::privateDataset::setData(int index, double value)
{
    if (mDataArray && index > 0 && index < mSize)
        mDataArray[index] = value;
    else
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Invalid index used");
    }
}



