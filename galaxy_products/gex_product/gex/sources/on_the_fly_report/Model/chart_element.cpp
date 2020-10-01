
#include "gexhistogramchart.h"
#include "gextrendchart.h"
#include "gexboxplotchart.h"
#include "gexprobabilityplotchart.h"
#include "gex_report.h"
#include "chart_element.h"

extern CGexReport* gexReport;


unsigned int ChartElement::mCurrentId = 0;

ChartElement::ChartElement(Component* parent):ReportElement("", QJsonObject(), parent, T_NONE)
{

}

ChartElement::ChartElement(const QString& name, const QJsonObject& description, Component* parent, T_Component type):
    ReportElement(name, description, parent, type)
{
    mName = mJsonRefName + "_" + QString::number(++ChartElement::mCurrentId);
}

QPixmap ChartElement::CreatePixmap()
{
    QString lImage;
    if(DrawSection(lImage, GEX_CHARTSIZE_MEDIUM))
        return QPixmap(lImage);
    else
        return QPixmap();
}

bool ChartElement::DrawSection(QString &imagePath, int imageSize, CTest* sectionTest/* = NULL*/)
{

    int lChartSize(imageSize/*GEX_CHARTSIZE_LARGE*/);
    CTest* lTestReferenceCell = NULL;
    QJsonObject  lChartOverlaysJson;
    if (sectionTest != NULL)
    {
        lTestReferenceCell = sectionTest;
        Test lTest;
        lTest.mGroupId = 0;
        lTest.mFileIndex =0;
        lTest.mNumber = sectionTest->GetTestNumber();
        lTest.mPinIndex = sectionTest->GetPinIndex();
        lTest.mName = sectionTest->GetTestName();
        UpdateTestInChartOverlays(lTest, lChartOverlaysJson);

    }
    else
    {
        lTestReferenceCell = FindTestCell(mTest.mNumber.toInt(),
                                          mTest.mPinIndex.toInt(),
                                          mTest.mName,
                                          mTest.mGroupId,
                                          mTest.mFileIndex);
        lChartOverlaysJson = mJsonDescription[mJsonRefName].toObject()["ChartOverlays"].toObject();

//        Debug(lChartOverlaysJson);
    }

    if(lTestReferenceCell == 0)
        return false;

    CGexChartOverlays lChartOverlays;
    QString lPrefix;
    GexAbstractChart* lGexAbstractChart = 0;
    switch(mType)
    {
        case T_HISTO :
        case T_HISTO_CONNECTED :
        {
            if (! lChartOverlays.InitFromJSon(lChartOverlaysJson, GexAbstractChart::chartTypeHistogram))
            {
                return false;
            }
            lChartOverlays.getViewportRectangle()[GexAbstractChart::chartTypeHistogram].bForceViewport = true;
            lGexAbstractChart = new GexHistogramChart(false, lChartSize, 0, &lChartOverlays);
            lPrefix = "adv_h_";
            break;
        }
        case T_TREND :
        case T_TREND_CONNECTED :
        {
            if (! lChartOverlays.InitFromJSon(lChartOverlaysJson, GexAbstractChart::chartTypeTrend))
            {
                return false;
            }
            lChartOverlays.getViewportRectangle()[GexAbstractChart::chartTypeTrend].bForceViewport = true;
            lGexAbstractChart = new GexTrendChart(false, lChartSize, 0, &lChartOverlays);
            lPrefix = "adv_t_";
            break;
        }
        case T_PROBA:
        case T_PROBA_CONNECTED:
        {
            if (! lChartOverlays.InitFromJSon(lChartOverlaysJson, GexAbstractChart::chartTypeProbabilityPlot))
            {
                return false;
            }
            lChartOverlays.getViewportRectangle()[GexAbstractChart::chartTypeProbabilityPlot].bForceViewport = true;
            lGexAbstractChart = new GexProbabilityPlotChart(lChartSize, 0, &lChartOverlays);
            lPrefix = "adv_p_";
            break;
        }
        case T_BOXPLOT :
        case T_BOXPLOT_CONNECTED :
        {
            if (! lChartOverlays. InitFromJSon(lChartOverlaysJson, GexAbstractChart::chartTypeBoxPlot))
            {
                return false;
            }
            lChartOverlays.getViewportRectangle()[GexAbstractChart::chartTypeBoxPlot].bForceViewport = true;
            lGexAbstractChart = new GexBoxPlotChart(lChartSize, 0, &lChartOverlays);
            lPrefix = "adv_b_";
            break;
        }
        default:
            return false;
    }

    if (lTestReferenceCell->ptChartOptions)
    {
        lTestReferenceCell->ptChartOptions->setCustomViewportX(false);
        lTestReferenceCell->ptChartOptions->setCustomViewportY(false);
    }

    QJsonObject lChart = mJsonDescription[mJsonRefName].toObject();
    lGexAbstractChart->setIsAdvHistoOnFile(true);
    if (lChart.contains("ViewportMode"))
    {
        lGexAbstractChart->setViewportMode(lChart["ViewportMode"].toInt());
    }
    else
    {
        lGexAbstractChart->setViewportModeFromChartMode(gexReport->getReportOptions()->getAdvancedReportSettings());
    }
    lGexAbstractChart->computeData(gexReport->getReportOptions(), lTestReferenceCell);
    lGexAbstractChart->buildChart();
    // create the image name
    imagePath = CreateImageName(lTestReferenceCell, "/images/"+lPrefix, mTest.mGroupId);

    // if the file already exist, in the case of the same test, add an increment
    QFileInfo lInfo(imagePath);
    if(lInfo.exists(imagePath))
    {
        imagePath += QString("_%1").arg(mImageIncrement++);
    }


    if(lGexAbstractChart->drawChart(imagePath, "www.mentor.com"))
    {
        delete lGexAbstractChart;
        return true;
    }

    delete lGexAbstractChart;
    return false;
}


void ChartElement::UpdateTestInChartOverlays(const Test& test, QJsonObject& chartOverlays)
{

    /*1*/QJsonObject lRefChart          =  mJsonDescription[mJsonRefName].toObject();
    /*2*/chartOverlays                  =  lRefChart["ChartOverlays"].toObject();
    /*3*/QJsonArray  lSingleChartsArray =  chartOverlays["SingleCharts"].toArray();

    for(int i = 0; i < lSingleChartsArray.count(); ++i)
    {
        /*4*/QJsonObject lSingleCharts      =  lSingleChartsArray[i].toObject();
        lSingleCharts.insert("ChartName",       "Test " + test.mNumber + ": " + test.mName);
        lSingleCharts.insert("TestNumberX",     test.mNumber.toInt());
        lSingleCharts.insert("PinMapX",         test.mPinIndex.toInt());
        lSingleCharts.insert("TestNameX",       test.mName);
        lSingleCharts.insert("TestLabelX",      "Test " + test.mNumber + ": " + test.mName);
        lSingleCharts.insert("GroupX",          test.mGroupId );
        /*4*/lSingleChartsArray.replace(i, lSingleCharts);
    }
    /*3*/
    chartOverlays.remove("SingleCharts");
//    Debug(chartOverlays);
    chartOverlays.insert("SingleCharts", lSingleChartsArray);
}


/*void ChartElement::UpdateJson()
{
   QJsonObject  lRefChart = mJsonDescription[mJsonRefName].toObject();
   lRefChart.insert("Name", mName);
   lRefChart.insert("Comment", mComment);
   UpdateTestJson(lRefChart);
   mJsonDescription.remove(mJsonRefName);
   mJsonDescription.insert(mJsonRefName, lRefChart);
}*/

