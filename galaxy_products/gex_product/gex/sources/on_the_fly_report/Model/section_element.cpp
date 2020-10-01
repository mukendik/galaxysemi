#include <set>
#include <QJsonArray>
#include <QJsonObject>
#include "chart_element.h"
#include "section_element.h"


SectionElement::SectionElement(const QString &name, Component* parent , const QJsonObject& jsonDescription):
    Composite(name, jsonDescription, parent, T_SECTION),
    mTestFilterType(T_TESTS_NOFILTER), mGroupFilterType(T_GROUPS_NOFILTER), mTopNValue(0), mSplitByGroup(false)
{

}

SectionElement::~SectionElement()
{
    ClearTestsFilter();
    ClearGroupsFilter();
}

void  SectionElement::AddTestFilter(const QString& testNumber,
                                    const QString& testName,
                                    const QString& pinName,
                                    int groupId,
                                    int fileIndex)
{

    Test* lTest = new Test();
    lTest->mNumber  = testNumber;
    lTest->mName    = testName;
    lTest->mPinIndex = pinName;
    lTest->mGroupId = groupId;
    lTest->mFileIndex= fileIndex;

    mTestsFilter.push_back(lTest);
}

void  SectionElement::AddGroupFilter(const QString& groupNumber,
                                    const QString& groupName)
{
    Group* lGroup = new Group();
    lGroup->mNumber  = groupNumber;
    lGroup->mName    = groupName;

    mGroupsFilter.append(lGroup);
}

void SectionElement::SetTestListFilterType(T_TestListFilter type)
{
    mTestFilterType = type;
}

void SectionElement::SetGroupListFilterType(T_GroupListFilter type)
{
    mGroupFilterType = type;
}

T_TestListFilter SectionElement::GetTestListFilterType() const
{
    return mTestFilterType;
}

T_GroupListFilter SectionElement::GetGroupListFilterType() const
{
    return mGroupFilterType;
}

QList<Test *> SectionElement::GetTestsListFilter() const
{
    return mTestsFilter;
}

QList<Group *> SectionElement::GetGroupsListFilter() const
{
    return mGroupsFilter;
}

void SectionElement::SetTopNValue(int topNValue)
{
    mTopNValue = topNValue;
}

int SectionElement::GetTopNValue() const
{
    return mTopNValue;
}

void SectionElement::SetSplitByGroup(bool value)
{
    mSplitByGroup = value;
}

bool SectionElement::IsSplitByGroup() const
{
    return mSplitByGroup;
}

void SectionElement::ClearTestsFilter()
{
    QList<Test*>::iterator lBegin(mTestsFilter.begin()), lEnd(mTestsFilter.end());
    for(; lBegin != lEnd; ++lBegin)
    {
        delete *lBegin;
    }
    mTestsFilter.clear();
}

void SectionElement::ClearGroupsFilter()
{
    while (!mGroupsFilter.isEmpty())
        delete mGroupsFilter.takeFirst();
}

bool SectionElement::LoadJson(const QJsonObject &description)
{
    mJsonDescription = description;
    if(mJsonDescription.contains("Name"))
        mName = mJsonDescription["Name"].toString();

    mComment = mJsonDescription["Comment"].toString();
    mIndexPosition = mJsonDescription["IndexPosition"].toInt();
    mSplitByGroup = mJsonDescription["SplitByGroup"].toBool();

    if(mJsonDescription.contains("Tests_Filter"))
    {
         QJsonObject lJsonTestsFilter = mJsonDescription["Tests_Filter"].toObject();
         mTestFilterType = T_TESTS_NOFILTER;
         if(lJsonTestsFilter.contains("Type"))
         {
            QString lType =  lJsonTestsFilter["Type"].toString();
            if(lType == "TopN")
            {
                mTopNValue      = lJsonTestsFilter["Value"].toInt();
                mTestFilterType = T_TESTS_TOPN;
            }
            else if(lType == "TestsList" )
            {
                mTestFilterType = T_TESTS_LIST;
                ClearTestsFilter();
                QJsonArray lJsonTests = lJsonTestsFilter["Tests"].toArray();
                for(int i = 0; i < lJsonTests.count(); ++i)
                {
                     QJsonObject lJsonTest = lJsonTests[i].toObject();
                     Test* lTest = new Test();
                     lTest->fromJson(lJsonTest);
                     mTestsFilter.push_back(lTest);
                }
            }
         }
    }

    if(mJsonDescription.contains("Groups_Filter"))
    {
         QJsonObject lJsonGroupsFilter = mJsonDescription["Groups_Filter"].toObject();
         mGroupFilterType = T_GROUPS_NOFILTER;
         if(lJsonGroupsFilter.contains("Type"))
         {
            QString lType =  lJsonGroupsFilter["Type"].toString();
            if(lType == "GroupsList" )
            {
                mGroupFilterType = T_GROUPS_LIST;
                ClearGroupsFilter();
                QJsonArray lJsonGroups = lJsonGroupsFilter["Groups"].toArray();
                for(int i = 0; i < lJsonGroups.count(); ++i)
                {
                     QJsonObject lJsonGroup = lJsonGroups[i].toObject();
                     Group* lGroup = new Group();
                     lGroup->fromJson(lJsonGroup);
                     mGroupsFilter.append(lGroup);
                }
            }
         }
    }

    if(mJsonDescription.contains("Elements"))
    {
        //-- as the json format does not garanti the order
        //-- we need to sort the Component based on their IndexPostion
        // -- (used in the SortComponent struct
        std::set<Component*, SortComponent> lComponents;

        QJsonArray lJsonElements = mJsonDescription["Elements"].toArray();
        for(int i = 0; i < lJsonElements.count(); ++i)
        {
            Component* lComponent = BuildReportElement(lJsonElements[i].toObject());
            if(lComponent == 0)
                continue;

            //static_cast<ReportElement*>(lComponent)->LoadJson();
            lComponents.insert(lComponent);
        }

        //-- then we can add them in the right order
        std::set<Component*, SortComponent>::iterator lIterBegin(lComponents.begin()), lIterEnd(lComponents.end());
        for(; lIterBegin != lIterEnd; ++lIterBegin)
        {
            AddElement(*lIterBegin);
        }
    }
    return true;
}

void SectionElement::UpdateElementsBeforeAdded(Component* element)
{
    if(element == 0)
        return;

    ReportElement* lReportElement = static_cast<ReportElement*>(element);
    if(mTestFilterType != T_TESTS_NOFILTER)
    {
        lReportElement->SetSectionSettingsControlActivated(true);
    }
}

void SectionElement::UpdateJson()
{
    mJsonDescription.insert("IndexPosition", mIndexPosition);
    mJsonDescription.insert("SplitByGroup", mSplitByGroup);

    QJsonObject lJsonTestsFilter;
    // -- set test filter if any
    if(mTestFilterType == T_TESTS_TOPN)
    {
         lJsonTestsFilter.insert("Type", QString("TopN"));
         lJsonTestsFilter.insert("Value", mTopNValue);
    }
    else if(mTestFilterType == T_TESTS_LIST)
    {
        lJsonTestsFilter.insert("Type", QString("TestsList"));
        QJsonArray lJsonTests;
        QList<Test*>::iterator lBegin(mTestsFilter.begin()), lEnd(mTestsFilter.end());
        for(int index = 0; lBegin != lEnd; ++lBegin)
        {
            if(*lBegin == 0)
                break;

            Test* lTest = *lBegin;
            QJsonObject lJsonTest = lTest->toJson();
            lJsonTests.insert(index++, lJsonTest);
        }
        lJsonTestsFilter.insert("Tests", lJsonTests);
    }
    else
    {
        lJsonTestsFilter.insert("Type", QString("None"));
    }
    mJsonDescription.insert("Tests_Filter", lJsonTestsFilter);

    QJsonObject lJsonGroupsFilter;
    // -- set group filter if any
    if(mGroupFilterType == T_GROUPS_LIST)
    {
        lJsonGroupsFilter.insert("Type", QString("GroupsList"));
        QJsonArray lJsonGroups;
        QList<Group*>::iterator lBegin(mGroupsFilter.begin()), lEnd(mGroupsFilter.end());
        for(int index = 0; lBegin != lEnd; ++lBegin)
        {
            if(*lBegin == 0)
                break;

            Group* lGroup = *lBegin;
            QJsonObject lJsonGroup = lGroup->toJson();
            lJsonGroups.insert(index++, lJsonGroup);
        }
        lJsonGroupsFilter.insert("Groups", lJsonGroups);
    }
    else
    {
        lJsonGroupsFilter.insert("Type", QString("None"));
    }
    mJsonDescription.insert("Groups_Filter", lJsonGroupsFilter);
}


bool SectionElement::HasTiedChildElements() const
{
    QList<Component*>::iterator lIterBegin(mReportElements.begin()), lIterEnd(mReportElements.end());
    for(;lIterBegin != lIterEnd; ++lIterBegin)
    {
        if(static_cast<ReportElement*>(*lIterBegin)->IsTieWithSection())
        {
            return true;
        }
    }
    return false;
}

bool SectionElement::ContainsTable() const
{
    if (mReportElements.size() > 0)
    {
        if (mReportElements[0] &&
            (mReportElements[0]->GetType() == T_TABLE
            || mReportElements[0]->GetType() == T_CAPABILITY_TABLE_CONNECTED))
        {
            return true;
        }
    }
    return false;
}

void SectionElement::TiedAllChild()
{
    QList<Component*>::iterator lIterBegin(mReportElements.begin()), lIterEnd(mReportElements.end());
    for(;lIterBegin != lIterEnd; ++lIterBegin)
    {
        static_cast<ReportElement*>(*lIterBegin)->SetTieWithSection(true);
    }
}

