#include "interactive_charts.h"
#include "drill_chart.h"
#include "test_defines.h" // for C_INFINITE
#include "browser_dialog.h"
#include "gqtl_log.h"

extern GexMainwindow *	pGexMainWindow;

///////////////////////////////////////////////////////////
// Constructor: Holds layers to chart
///////////////////////////////////////////////////////////
CGexChartOverlays::CGexChartOverlays(QWidget *lParent)
{
    // List of layers are deleted when we empty the list.
    ptParent=lParent;

    // Clear member variables
    clear();
    for(int iChartType =  GexAbstractChart::chartTypeHistogram ; iChartType <=  GexAbstractChart::chartTypeCharacHisto ; iChartType++){
        m_oViewportRectangle.insert(iChartType, CGexChartOverlays::ViewportRectangle());
        m_oViewportRectangle[iChartType].bForceViewport = false;
    }

    m_iAppliedToChart = -1;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexChartOverlays::~CGexChartOverlays()
{
    removeCharts();
}

void CGexChartOverlays::CopySettings(const CGexChartOverlays* other)
{
    mBackgroundColor    = other->mBackgroundColor;
    mTotalBars          = other->mTotalBars;
    mYScale             = other->mYScale;
    mCustom             = other->mCustom;
    mCustomTotalBars    = other->mCustomTotalBars;
    lfZoomFactorX       = other->lfZoomFactorX;
    lfZoomFactorY       = other->lfZoomFactorY;
    bFileOutput         = other->bFileOutput;
    pStart              = other->pStart;
    pEnd                = other->pEnd;
    mLayerName          = other->mLayerName;
    mTextRotation       = other->mTextRotation;
    mQQLine             = other->mQQLine;
    mTitle              = other->mTitle;

//    mColor                  = other->mColor;
//    mBoxBars                = other->mBoxBars;
//    mBox3DBars              = other->mBox3DBars;
//    mIsStacked              = other->mIsStacked;
//    mFittingCurve           = other->mFittingCurve;
//    mBellCurve              = other->mBellCurve;
//    mLines                  = other->mLines;
//    mSpots                  = other->mSpots;
//    mSpecLimit              = other->mSpecLimit;
//    mLineStyle              = other->mLineStyle;
//    mSpotStyle              = other->mSpotStyle;

//    mWhiskerMode            = other->mWhiskerMode;
//    mMeanLineWidth          = other->mMeanLineWidth;
//    mMinLineWidth           = other->mMinLineWidth;
//    mMaxLineWidth           = other->mMaxLineWidth;
//    mMedianLineWidth        = other->mMedianLineWidth;
//    mLimitsLineWidth        = other->mLimitsLineWidth;
//    mSpecLimitsLineWidth    = other->mSpecLimitsLineWidth;
//    mSigma2LineWidth        = other->mSigma2LineWidth;
//    mSigma3LineWidth        = other->mSigma3LineWidth;
//    mSigma6LineWidth        = other->mSigma6LineWidth;
//    mSigma12LineWidth       = other->mSigma12LineWidth;
//    mQuartileQ3LineWidth    = other->mQuartileQ3LineWidth;
//    mQuartileQ1LineWidth    = other->mQuartileQ1LineWidth;
//    mRollingLimitsLineWidth = other->mRollingLimitsLineWidth;
//    mLineWidth              = other->mLineWidth;

    int lNbIterations = qMin(m_lstChartsList.count(), other->chartsList().count());
    for(int i = 0; i < lNbIterations; ++i)
    {
        m_lstChartsList[i]->CopySettings( other->chartsList()[i]);
    }
 }

//-- arno
void  CGexChartOverlays::InitGlobal(CReportOptions*	reportOptions)
{
    mTotalBars                  = reportOptions->mTotalBars;
    mBackgroundColor            = reportOptions->cBkgColor;
    mYScale                     = reportOptions->mYScale;
    mCustom                     = reportOptions->mCustom;
    mCustomTotalBars            = reportOptions->mTotalBarsCustom;

    mLayerName                  = reportOptions->bPlotLegend;
    mTextRotation               = reportOptions->mTextRotation;
    mQQLine                     = reportOptions->mPlotQQLine;

    QStringList lMarkers        = reportOptions->GetOption("adv_histogram","marker").toString().split("|");

    mTitle                      = lMarkers.contains("test_name");

    // mScaling                    = reportOptions->GetOption("dataprocessing", "scaling").toString();
   // mOptionStorageDevice        = reportOptions->GetOption("adv_histogram", "chart_type").toString();

   // mAdvHistogramMarkerOptions  = reportOptions->GetOption(QString("adv_histogram"), QString("marker")).toString();
    //mAdvHistogramFieldOptions   = reportOptions->GetOption(QString("adv_histogram"),QString("field")).toString();

    //mHistoMarkerMin             = reportOptions->bHistoMarkerMin;
    //mHistoMarkerMax             = reportOptions->bHistoMarkerMax;
}




///////////////////////////////////////////////////////////
// Add a chart to the list
///////////////////////////////////////////////////////////
void CGexChartOverlays::addChart(CGexSingleChart * pChart)
{
    m_lstChartsList.append(pChart);
}

///////////////////////////////////////////////////////////
// Move a chart to the end the list
///////////////////////////////////////////////////////////
void CGexChartOverlays::moveBack(int nIndex)
{
    GEX_ASSERT(nIndex >= 0 && nIndex < m_lstChartsList.count());

    CGexSingleChart * pLayer = m_lstChartsList.takeAt(nIndex);

    m_lstChartsList.append(pLayer);
}

///////////////////////////////////////////////////////////
// Move a chart to the begin of the list
///////////////////////////////////////////////////////////
void CGexChartOverlays::moveFront(int nIndex)
{
    GEX_ASSERT(nIndex >= 0 && nIndex < m_lstChartsList.count());

    CGexSingleChart * pLayer = m_lstChartsList.takeAt(nIndex);

    m_lstChartsList.prepend(pLayer);
}

///////////////////////////////////////////////////////////
// Remove chart at the given index
///////////////////////////////////////////////////////////
void CGexChartOverlays::removeChart(int nIndex)
{
    if (nIndex > 0 && nIndex < m_lstChartsList.count())
        delete m_lstChartsList.takeAt(nIndex);
}

///////////////////////////////////////////////////////////
// Remove all charts from charts list
///////////////////////////////////////////////////////////
void CGexChartOverlays::removeCharts()
{
    while (m_lstChartsList.isEmpty() == false)
        delete m_lstChartsList.takeFirst();
}



void ColorRGBToJsonArray(QJsonArray& colorJson, const QColor &color)
{
    if(colorJson.isEmpty())
    {
        colorJson.insert(0, QJsonValue(color.red()));
        colorJson.insert(1, QJsonValue(color.green()));
        colorJson.insert(2, QJsonValue(color.blue()));
    }
    else
    {
        colorJson.replace(0, QJsonValue(color.red()));
        colorJson.replace(1, QJsonValue(color.green()));
        colorJson.replace(2, QJsonValue(color.blue()));
    }
}

QColor JsonArrayToColorRGB(const QJsonArray& colors, const QColor &defaultColor)
{
    QColor lColor(defaultColor);

    if(colors.size() == 3)
    {
        if (colors[0].isDouble())
            lColor.setRed(static_cast<int>(colors[0].toDouble()));

        if (colors[1].isDouble())
            lColor.setGreen(static_cast<int>(colors[1].toDouble()));

        if (colors[2].isDouble())
            lColor.setBlue(static_cast<int>(colors[2].toDouble()));
    }

    return lColor;
}


bool
CGexChartOverlays::ToJson(QJsonObject& descriptionOut, int chartType) const
{
    //QJsonObject lGlobalSettingJson;

    // -- Insert CGexChartOverlays attributs
    descriptionOut.insert("TotalBars", mTotalBars);
    QJsonArray lBackgroundColor;
    ColorRGBToJsonArray(lBackgroundColor,           mBackgroundColor);
    descriptionOut.insert("BackgroundColor",    lBackgroundColor);
    descriptionOut.insert("YScale",             mYScale);
    descriptionOut.insert("Custom",             mCustom);
    descriptionOut.insert("CustomTotalBars",    mCustomTotalBars);
    descriptionOut.insert("LayerName",          mLayerName);
    descriptionOut.insert("TextRotation",       mTextRotation);
    descriptionOut.insert("PlotQQLine",         mQQLine);
    descriptionOut.insert("HistoMarkerMin",     mHistoMarkerMin);
    descriptionOut.insert("HistoMarkerMax",     mHistoMarkerMax);
    descriptionOut.insert("YScale",             mYScale);
    descriptionOut.insert("Custom",             mCustom);
    descriptionOut.insert("FileOutput",         bFileOutput);
    descriptionOut.insert("XStartSelection",    pStart.x());
    descriptionOut.insert("YStartSelection",    pStart.y());
    descriptionOut.insert("XEndSelection",      pEnd.x());
    descriptionOut.insert("YEndSelection",      pEnd.y());
    descriptionOut.insert("Title",              mTitle);

    // -- Loop over viewports
    QJsonArray lViewports ;
    QJsonObject lViewport;
    m_oViewportRectangle[chartType].ToJson(lViewport);
    lViewports.insert(0, lViewport);
    descriptionOut.insert("ViewPorts", lViewports);


    // -- Loop over SingleChartWidget
    QJsonArray lSingleChartsWidgetJson;
    QList <CGexSingleChart*>::const_iterator lIterBeginChart(m_lstChartsList.begin()), lIterEndChart(m_lstChartsList.end());
    for(int lPositionChart = 0; lIterBeginChart != lIterEndChart; ++lIterBeginChart, ++lPositionChart)
    {
        QJsonObject lSingleChartJson;
        (*lIterBeginChart)->ToJson(lSingleChartJson);
        lSingleChartsWidgetJson.insert(lPositionChart, lSingleChartJson);
    }
    descriptionOut.insert("SingleCharts", lSingleChartsWidgetJson);


    return true;
}

bool
CGexChartOverlays::InitFromJSon(const QJsonObject &descriptionIn,
                                int chartType)
{

    // -- init all global settings from JSON
    mTotalBars                  = descriptionIn["TotalBars"].toInt();
    mBackgroundColor            = JsonArrayToColorRGB(descriptionIn["BackgroundColor"].toArray(), QColor(Qt::green));
    mYScale                     = descriptionIn["YScale"].toInt();
    mCustom                     = descriptionIn["Custom"].toBool();
    mCustomTotalBars            = descriptionIn["TotalBarsCustom"].toInt();
    mLayerName                  = descriptionIn["LayerName"].toBool();
    mTextRotation               = descriptionIn["TextRotation"].toInt();
    mQQLine                     = descriptionIn["PlotQQLine"].toBool();
    mHistoMarkerMin             = descriptionIn["HistoMarkerMin"].toBool();
    mHistoMarkerMax             = descriptionIn["HistoMarkerMax"].toBool();
    mYScale                     = descriptionIn["YScale"].toInt();
    mCustom                     = descriptionIn["Custom"].toBool();
    mTitle                      = descriptionIn["Title"].toBool();

    bFileOutput                 = descriptionIn["FileOutput"].toBool();
    int lXStart                 =  descriptionIn["XStartSelection"].toInt();
    int lYStart                 =  descriptionIn["YStartSelection"].toInt();
    pStart.setX(lXStart);
    pStart.setY(lYStart);

    int lXEnd                   =  descriptionIn["XEndSelection"].toInt();
    int lYEnd                   =  descriptionIn["YEndSelection"].toInt();
    pEnd.setX(lXEnd);
    pEnd.setY(lYEnd);

    QJsonArray lViewports = descriptionIn["ViewPorts"].toArray();
    QJsonArray::iterator lIterViewportBegin(lViewports.begin());

    CGexChartOverlays::ViewportRectangle lViewPort;
    if (lViewPort.InitFromJSon((*lIterViewportBegin).toObject()) == false)
    {
        return false;
    }
    m_oViewportRectangle.insert(chartType, lViewPort);


//    if(m_oViewportRectangle[chartType].cChartOptions.strChartTitle.isEmpty() && mTitle.isEmpty() == false)
//    {
//        m_oViewportRectangle[chartType].cChartOptions.strChartTitle = mTitle;
//        m_oViewportRectangle[chartType].cChartOptions.bChartTitle = true;
//    }

    QJsonArray lSingleCharts = descriptionIn["SingleCharts"].toArray();
    QJsonArray::iterator lIterSingleChartBegin(lSingleCharts.begin()), lIterSingleChartEnd(lSingleCharts.end());
    for(;lIterSingleChartBegin != lIterSingleChartEnd; ++lIterSingleChartBegin)
    {
        CGexSingleChart* lSingleChartWidget = new CGexSingleChart();

        if(lSingleChartWidget->InitFromJSon((*lIterSingleChartBegin).toObject()) == false)
        {
            delete lSingleChartWidget;
            return false;
        }

        m_lstChartsList.append(lSingleChartWidget);
    }

    return true;
}

///////////////////////////////////////////////////////////
// Reset variables
///////////////////////////////////////////////////////////
void CGexChartOverlays::clear(void)
{
    m_iAppliedToChart = -1;
    /*if(pGexMainWindow  && pGexMainWindow->pWizardChart)
        ptParent = pGexMainWindow->pWizardChart;	// Keeps pointer to frame that holds the drawing.
    else
    if(ptParent == NULL)
        ptParent = pGexMainWindow;*/	// At startup: tie parent to main winndow, until 'pWizardChart' exists.

    // View 100% of the original chart! (no zoom initially)
    pStart.setX(0);
    pStart.setY(100);
    pEnd.setX(0);
    pEnd.setY(100);
    lfZoomFactorX = lfZoomFactorY =  -1.0;

    // Default mode: output to screen
    bFileOutput = false;

    // Empty charts list
    removeCharts();
}

CGexChartOverlays::ViewportRectangle::ViewportRectangle(){
    lfLowX	= -C_INFINITE;
    lfLowY	= -C_INFINITE;
    lfHighX	= C_INFINITE;
    lfHighY	= C_INFINITE;
    mChangeViewPort = false;
    cChartOptions.bChartTitle = false;
    cChartOptions.bLegendX = false;
    cChartOptions.bLegendY = false;
}
CGexChartOverlays::ViewportRectangle::~ViewportRectangle(){

}

bool CGexChartOverlays::ViewportRectangle::ToJson(QJsonObject& descriptionOut) const
{
    if(mChangeViewPort)
    {
        descriptionOut.insert("ForceViewport", true);
        descriptionOut.insert("LowX", lfLowX);
        descriptionOut.insert("LowY", lfLowY);
        descriptionOut.insert("HighX", lfHighX);
        descriptionOut.insert("HighY", lfHighY);
    }
    else
    {
        descriptionOut.insert("ForceViewport", false);

        //-- will be recalculated whenn needed
        descriptionOut.insert("LowX", -C_INFINITE);
        descriptionOut.insert("LowY", -C_INFINITE);
        descriptionOut.insert("HighX", C_INFINITE);
        descriptionOut.insert("HighY", C_INFINITE);
    }

    QJsonObject lChartOptionsJson;
    cChartOptions.ToJson(lChartOptionsJson);

    descriptionOut.insert("ChartOption", lChartOptionsJson );
    return true;
}

bool  CGexChartOverlays::ViewportRectangle::InitFromJSon(const QJsonObject& descriptionIn)
{
    lfLowX          = descriptionIn["LowX"].toDouble();
    lfLowY          = descriptionIn["LowY"].toDouble();
    lfHighX         = descriptionIn["HighX"].toDouble();
    lfHighY         = descriptionIn["HighY"].toDouble();
    bForceViewport  = descriptionIn["ForceViewport"].toBool();

    if(cChartOptions.InitFromJSon(descriptionIn["ChartOption"].toObject()) ==false)
        return false;

    return true;

}
