#ifndef CONSOLIDATION_TREE_QUERY_FILTER_H
#define CONSOLIDATION_TREE_QUERY_FILTER_H

#include <QMap>

class QTextStream;

class CTQueryFilter
{
public:

    enum Filter
    {
        FilterTestingStage,
        FilterProductID,
        FilterDate,
        FilterOption
    };

    CTQueryFilter();

    // Set a filter on the value of a given field.
    void                        add(Filter filter, const QString& value);
    // Remove a filter on a given field
    void                        remove(Filter filter);
    // Remove all filters
    void                        removeAll();

    // Retrieve the value of the filter for a given field
    QString                     value(Filter filter) const;

    void                        dump(QTextStream& txtStream) const;

protected:

    // Holds the filters for the query
    QMap<Filter, QString>        m_filters;
};

#endif // CONSOLIDATION_TREE_QUERY_FILTER_H
