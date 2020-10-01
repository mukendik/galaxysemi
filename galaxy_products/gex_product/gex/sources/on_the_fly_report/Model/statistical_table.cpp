#include "statistical_table.h"

#include <QJsonArray>
#include <QJsonObject>

#include "gex_report.h"

extern CGexReport* gexReport;

unsigned int StatisticalTable::sCurrentId = 0;

StatisticalTable::StatisticalTable(const QString& name, const QJsonObject &jsonDescription, Component* parent):
    ReportElement(name, jsonDescription, parent, T_TABLE)
{
    mName = "Table_" + QString::number(++StatisticalTable::sCurrentId);
    mTestActivated = false;

    mTieWithSection = true;
}

StatisticalTable::~StatisticalTable()
{
    qDeleteAll(mTestList);
}

// TO DO: Implement the draw when the PDF interface is ready
bool StatisticalTable::DrawSection(QString &imagePath, int /*imageSize*/, CTest* /*sectionTest*/)
{
    imagePath = "Table";
    return true;
}

const QList<Group*>& StatisticalTable::GetGroupList() const
{
    return mGroupList;
}

const QList<Test*>& StatisticalTable::GetTestList() const
{
    return mTestList;
}

void StatisticalTable::LoadJson()
{
    if(mJsonDescription.isEmpty())
        return;
    mTestList.clear();

    QJsonObject  lJsonRef   = mJsonDescription[mJsonRefName].toObject();

    mComment        = lJsonRef["Comment"].toString();
    mName           = lJsonRef["Name"].toString();
   // mIndexPosition  = lJsonRef["IndexPosition"].toInt();
    //mTieWithSection = lJsonRef["TieWithSection"].toBool();

    if(lJsonRef.contains("Tests"))
    {
        QJsonArray lJsonTests = lJsonRef["Tests"].toArray();
        for (int i=0; i<lJsonTests.size(); ++i)
        {
           QJsonObject lTestObj = lJsonTests[i].toObject();
           Test* lTest = new Test();
           lTest->fromJson(lTestObj);
           mTestList.push_back(lTest);

        }
    }

    if(lJsonRef.contains("Fields"))
    {
        QJsonArray lJsonFieldss = lJsonRef["Fields"].toArray();
        for (int i=0; i<lJsonFieldss.size(); ++i)
        {
           mFieldList.append(lJsonFieldss[i].toString());
        }
    }

    if(lJsonRef.contains("Groups"))
    {
        QJsonArray lJsonGroups = lJsonRef["Groups"].toArray();
        for(int i = 0; i < lJsonGroups.count(); ++i)
        {
             QJsonObject lJsonGroup = lJsonGroups[i].toObject();
             Group* lGroup = new Group();
             lGroup->fromJson(lJsonGroup);
             mGroupList.append(lGroup);
        }
    }
}






