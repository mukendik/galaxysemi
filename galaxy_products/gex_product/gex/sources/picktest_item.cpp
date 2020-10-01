#include "picktest_item.h"

PickTestItem::PickTestItem()
{
    this->reset();
}

void PickTestItem::reset()
{
    m_testNumber = "";
    m_testName = "";
    m_testType = "";
    m_pinName = "";
    m_groupList.clear();
    mGroupIndex = -1;
    mFileIndex = -1;
}

void PickTestItem::setTestNumber(const QString &value)
{
    m_testNumber = value;
}

void PickTestItem::setTestName(const QString &value)
{
    m_testName = value;
}

void PickTestItem::setTestType(const QString &value)
{
    m_testType = value;
}

void PickTestItem::setPinName(const QString &value)
{
    m_pinName = value;
}

void PickTestItem::addGroupId(int value)
{
    if (!isInGroup(value))
        m_groupList.append(value);
}

void PickTestItem::setGroupIndex(int index)
{
    mGroupIndex = index;
}

void PickTestItem::setFileIndex(int index)
{
    mFileIndex = index;
}

QString PickTestItem::testNumber()
{
    return m_testNumber;
}

QString PickTestItem::testName()
{
    return m_testName;
}

QString PickTestItem::testType()
{
    return m_testType;
}

QString PickTestItem::pinName()
{
    return m_pinName;
}

QList<int> PickTestItem::groupList()
{
    return m_groupList;
}

bool PickTestItem::isInGroup(int value)
{
    return m_groupList.contains(value);
}

int PickTestItem::GetGroupIndex()
{
    return mGroupIndex;
}

int PickTestItem::GetFileIndex()
{
    return mFileIndex;
}
