#include "gex_report.h"
#include "report_element.h"

extern CGexReport* gexReport;

ReportElement::ReportElement(const QString &name, const QJsonObject &description, Component* parent, T_Component type)
    :Component(name, description, parent, type),
     mTestActivated(false),
     mTieWithSection(true),
     mSectionSettingsControlActivated(false)
{
    mJsonRefName = "Unknown";
    if(type == T_HISTO)
        mJsonRefName = "Histogram";
    else if(type == T_TREND)
        mJsonRefName = "Trend";
    else if(type == T_PROBA)
        mJsonRefName = "ProbabilityPlot";
    else if(type == T_BOXPLOT)
        mJsonRefName = "BoxPlot";
    else if(type == T_WAFER)
        mJsonRefName = "Wafermap";
    else if(type == T_TABLE)
        mJsonRefName = "Table";

   // LoadJson();
}

ReportElement::~ReportElement()
{
}

QJsonObject ReportElement::ToJson()
{
    return mJsonDescription;
}

QString ReportElement::CreateImageName(CTest* testReferenceCell, const QString& prefix, int dataSet)
{
    // create the image name
    QString strImage    = gexReport->BuildImageUniqueName(gexReport->getReportOptions()->strReportDirectory + prefix, testReferenceCell, dataSet);
    QString lImagePath  = gexReport->getReportOptions()->strReportDirectory;
    // We have this case only if we do a build report for interctive only
    if (lImagePath.isEmpty() || QDir(lImagePath).exists() == false)
    {
        lImagePath = QDir::homePath()+"/GalaxySemi/temp/report_builder/images/";
        QDir lDir(lImagePath);
        if (!lDir.mkpath(lImagePath))
            return "error : cannot make path "+ lImagePath;
    }
    else
    {
        lImagePath			+= "/images/";
    }
    lImagePath			+= testReferenceCell->GetTestNumber() + strImage;
    return lImagePath;
}

CTest* ReportElement::FindTestCell(int testNumber, int pinIndex, const QString &testName, int dataSet, int fileIndex)
{
    CGexGroupOfFiles*   lGroup;
    CGexFileInGroup*    lFile;
    QList<CGexGroupOfFiles*> lGroupList = gexReport->getGroupsList();
    // Find the correct Group, for the first version, we will use the index of the group
    if (dataSet < lGroupList.size())
        lGroup = lGroupList[dataSet];
    else
        return 0;

    if(lGroup == 0)
        return 0;

    const QList<CGexFileInGroup*> lFilesList = lGroup->GetFilesList();
    if (fileIndex < lFilesList.size())
        lFile = lFilesList[fileIndex];
    else
        return 0;

    if(lFile == 0)
        return 0;

    CTest* lTestReferenceCell = 0;
    lFile->FindTestCell(testNumber, pinIndex, &lTestReferenceCell, true, false, testName);
    return lTestReferenceCell;
}

void ReportElement::LoadJson()
{
    if(mJsonDescription.isEmpty())
        return;

    QJsonObject  lJsonRef   = mJsonDescription[mJsonRefName].toObject();

    mComment        = lJsonRef["Comment"].toString();

    QString lName   = lJsonRef["Name"].toString();
    if(lName.isEmpty() == false)
        mName = lName;

    mIndexPosition  = lJsonRef["IndexPosition"].toInt();
    mTieWithSection = lJsonRef["TieWithSection"].toBool();

    mTestActivated  = false;
    if(lJsonRef.contains("Test"))
    {
        QJsonObject lJsonTest = lJsonRef["Test"].toObject();
        mTest.fromJson(lJsonTest);
        mTestActivated = true;
    }
}

bool ReportElement::IsTestActivated()
{
    return mTestActivated;
}

void ReportElement::SetTestActivated(bool activated)
{
    mTestActivated = activated;
}

const Test &ReportElement::GetTestFilter()
{
    return mTest;
}

bool ReportElement::IsTieWithSection() const
{
    return mTieWithSection;
}

void  ReportElement::SetTieWithSection(bool status)
{
    mTieWithSection = status;

    SetSectionSettingsControlActivated(mSectionSettingsControlActivated);
}

bool ReportElement::IsSectionSettingsControlActivated() const
{
    return mSectionSettingsControlActivated;
}

void ReportElement::SetSectionSettingsControlActivated(bool status)
{
    mSectionSettingsControlActivated = status;

    if(mSectionSettingsControlActivated && mTieWithSection)
        mType = GetConnectedType(mType) ;
    else
        mType = GetUnConnectedType(mType) ;
}

void ReportElement::UpdateJson()
{
   QJsonObject  lRefChart = mJsonDescription[mJsonRefName].toObject();
   lRefChart.insert("Name", mName);
   lRefChart.insert("Comment", mComment);
   UpdateTestJson(lRefChart);
   mJsonDescription.remove(mJsonRefName);
   mJsonDescription.insert(mJsonRefName, lRefChart);
}

void   ReportElement::UpdateTestJson(QJsonObject& toUpdate)
{
    toUpdate.insert("IndexPosition", mIndexPosition);
    toUpdate.insert("TieWithSection", mTieWithSection);

    if(mTestActivated)
    {
        toUpdate.insert("Test", mTest.toJson());
    }
}



// To keep for Gui V2
//void ReportElement::AddTestFilter(const QString& testNumber, const QString& testName, const QString& pinIndex, int groupId)
//{

//    if( mTest.mNumber   != testNumber ||
//        mTest.mName     != testName   ||
//        mTest.mPinIndex != pinIndex   ||
//        mTest.mGroupId != groupId    )
//    {
//        mTest.mNumber  = testNumber;
//        mTest.mName    = testName;
//        mTest.mPinIndex = pinIndex;
//        mTest.mGroupId = groupId;
//        if(groupId < 0)
//         mTest.mGroupId = 0;
//            UpdateTestInChartOverlays(const Test& test, QJsonObject& chartOverlays);
//    }
//}
