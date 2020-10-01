#ifndef HTML_TABLE_RENDERER_H
#define HTML_TABLE_RENDERER_H

#include <QString>
#include "component.h"

class CTest;
class CGexFileInGroup;
class CGexGroupOfFiles;

class HTMLTableRenderer
{
public:
    HTMLTableRenderer();


    /**
     * Create a HTML header
     */
    QString CreateHeader(const QList<QString> &fields) const;

    /**
     * @brief CreateHTMLTable Create a html table with a list of tests for the row and
     * a list a fileds for the column
     * @return the html string build
     */
    QString CreateTable  (const CTestContainer &tests, const QList<QString>& fields, const QList<Group *> groups, bool splitByGroup) const;


    /**
     * @brief create as many html rows as test
     * @param fields the list of the columns
     * @param test the list of row
     * @param group associated with the Ctest
     * @param file associated with the group
     * @return
     */
    QString CreateRow    (const QList<QString>& fields, CTest *test, CGexGroupOfFiles *group, CGexFileInGroup *file) const;


    void    InitShiftColumnLabels() const;


    QString CreateItem(const QString& field, CTest* test, CGexGroupOfFiles *group, CGexFileInGroup* file ) const;



    /**
     * @brief CreateHTMLTableCell
     * @param result indicate if there is a result to display. If not, n/a will be put
     * @param value the value to display
     */
    template<typename T_DATA>
    QString CreateCell(const T_DATA &value, char result, const QString &color = DATA_COLOR) const;

    /**
     * create an html cell displaying the T_DATA value;
     */
    template<typename T_DATA>
    QString CreateCell(const T_DATA& value, const QString &color = DATA_COLOR) const;
};

#endif // TABLE_RENDERER_H
