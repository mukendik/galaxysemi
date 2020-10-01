#include <QJsonArray>
#include <QJsonObject>

#include "gex_report.h"
#include "wafermap_element.h"

extern CGexReport* gexReport;

unsigned int WaferMapElement::sCurrentId = 0;

WaferMapElement::WaferMapElement(const QString& name, const QJsonObject &jsonDescription, Component* parent):ReportElement(name, jsonDescription, parent, T_WAFER)
{
    mName = "Wafermap_" + QString::number(++WaferMapElement::sCurrentId);
}

bool WaferMapElement::DrawSection(QString &imagePath, int /*imageSize*/, CTest *sectionTest)
{
    CTest* lTestReferenceCell;
    if (sectionTest != NULL)
    {
        lTestReferenceCell = sectionTest;
    }
    else
    {
        lTestReferenceCell = FindTestCell(mTest.mNumber.toInt(),
                                          mTest.mPinIndex.toInt(),
                                          mTest.mName,
                                          mTest.mGroupId,
                                          mTest.mFileIndex);
    }

    if (lTestReferenceCell == 0)
        return false;


    imagePath = CreateImageName(lTestReferenceCell, "WaferMap", 1);
    QJsonObject lWaferMap           = mJsonDescription[mJsonRefName].toObject();
    QJsonObject lParam              = lWaferMap["Parameters"].toObject();
    int lWaferMapType               = lParam["WaferMapType"].toInt();
    double lSpectrumColorLowValue (GEX_C_DOUBLE_NAN), lSpectrumColorHighValue(GEX_C_DOUBLE_NAN);
    bool   lHighValueChanged        = lParam["HighValueChanged"].toBool();
    bool   lLowValueChanged         = lParam["LowValueChanged"].toBool();
    if (lHighValueChanged)
        lSpectrumColorHighValue  = lParam["SpectrumColorHighValue"].toDouble();
    if (lLowValueChanged)
        lSpectrumColorLowValue   = lParam["SpectrumColorLowValue"].toDouble();

    gexReport->FillWaferMap(mTest.mGroupId,
                            mTest.mFileIndex,
                            lTestReferenceCell,
                            lWaferMapType,
                            false,
                            lSpectrumColorLowValue,
                            lSpectrumColorHighValue);


    QStringList lDeactivatedBins;
    if (lWaferMapType == GEX_WAFMAP_HARDBIN || lWaferMapType == GEX_WAFMAP_SOFTBIN)
    {
        QString LBinsString  = lParam["DeactivatedBins"].toString();
        lDeactivatedBins = LBinsString.split(",");
    }

    bool pbBreakPage = false;
    if (gexReport->getGroupsList().size() < mTest.mGroupId+1)
        return false;

    CGexGroupOfFiles* lGroup=gexReport->getGroupsList().at(mTest.mGroupId);

    if (!lGroup)
        return false;

    //if (lGroup->GetTotalValidFiles()<lFileIndex+1)
    if (lGroup->pFilesList.size() < mTest.mFileIndex+1)
        return false;

    CGexFileInGroup* lFile=lGroup->pFilesList.at(mTest.mFileIndex);
    if (!lFile)
        return false;
    gexReport->CreateWaferMapImage(CGexReport::individualWafermap,
                                   lGroup,
                                   lFile,
                                   true,
                                   imagePath.toLatin1().constData(),
                                   "wafermap",
                                   true,
                                   false,
                                   &pbBreakPage,
                                   lWaferMapType,
                                   true,
                                   lTestReferenceCell,
                                   lDeactivatedBins);
    return true;
}

void WaferMapElement::UpdateJson()
{
    QJsonObject  lRefChart = mJsonDescription[mJsonRefName].toObject();
    lRefChart.insert("Name", mName);
    lRefChart.insert("Comment", mComment);

    UpdateTestJson(lRefChart);
    mJsonDescription.remove(mJsonRefName);
    mJsonDescription.insert(mJsonRefName, lRefChart);
}


