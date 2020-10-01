#ifndef STATISTICALTABLE_H
#define STATISTICALTABLE_H

#include "report_element.h"


class StatisticalTable : public ReportElement
{
public:
    StatisticalTable(const QString &name, const QJsonObject &jsonDescription, Component *parent);
    ~StatisticalTable();

    /**
     * \fn bool DrawSection(QString &imagePath, CTest* sectionTest = NULL);
     * \brief this function draws the the StatisticalTable in an image.
     * \param imagePath the path of the created image
     * \param sectionTest the test comming from the section if exists
     * \return true if the draw has been done with success. Otherwise return false.
     */
    virtual bool                DrawSection(QString &imagePath, int imageSize, CTest* sectionTest = NULL);

    const QList<Group *> &      GetGroupList() const;
    const QList<Test *> &       GetTestList() const;
    const QList<QString>&       GetFieldsList() const { return mFieldList; }

    void                        LoadJson();
private:
    static unsigned int          sCurrentId;     /// \param The id of the current TableElement
    QList<Test*>                 mTestList;
    QList<Group*>                mGroupList;
    QList<QString>               mFieldList;

};


#endif // STATISTICALTABLE_H
