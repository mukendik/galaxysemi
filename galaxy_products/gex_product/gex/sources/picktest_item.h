#ifndef PICKTEST_ITEM_H
#define PICKTEST_ITEM_H

#include <QString>
#include <QList>

class PickTestItem
{
public:
    PickTestItem();

    void        reset();

    // Setters
    void setTestNumber(const QString& value);
    void setTestName(const QString& value);
    void setTestType(const QString& value);
    void setPinName(const QString& value);
    void addGroupId(int value);
    void setGroupIndex(int index);
    void setFileIndex(int index);
    // Getters
    QString     testNumber();
    QString     testName();
    QString     testType();
    QString     pinName();
    QList<int>  groupList();
    bool        isInGroup(int value);
    int         GetGroupIndex();
    int         GetFileIndex();


private:
    QString     m_testNumber;
    QString     m_testName;
    QString     m_testType;
    QString     m_pinName;
    QList<int>  m_groupList;  // Holds list of all groups containing this test
    int         mGroupIndex;
    int         mFileIndex;
};

#endif // PICKTEST_ITEM_H
