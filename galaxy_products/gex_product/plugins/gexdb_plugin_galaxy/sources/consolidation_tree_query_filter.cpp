#include "consolidation_tree_query_filter.h"

#include <QTextStream>

CTQueryFilter::CTQueryFilter()
{
}

void CTQueryFilter::add(CTQueryFilter::Filter filter, const QString &value)
{
    m_filters.insert(filter, value);
}

void CTQueryFilter::remove(CTQueryFilter::Filter filter)
{
    m_filters.remove(filter);
}

void CTQueryFilter::removeAll()
{
    m_filters.clear();
}

QString CTQueryFilter::value(CTQueryFilter::Filter filter) const
{
    if (m_filters.contains(filter))
        return m_filters.value(filter);

    return QString("");
}

void CTQueryFilter::dump(QTextStream &txtStream) const
{
    txtStream << "-- CTQueryFilters -- " << endl;
    txtStream << " TestingStage : " << ((m_filters.contains(CTQueryFilter::FilterTestingStage)) ?
                                        value(CTQueryFilter::FilterTestingStage) : "<none>") << endl;
    txtStream << " ProductId    : " << ((m_filters.contains(CTQueryFilter::FilterProductID)) ? \
                                        value(CTQueryFilter::FilterProductID) : "<none>") << endl;
    txtStream << " Date         : " << ((m_filters.contains(CTQueryFilter::FilterDate)) ? \
                                        value(CTQueryFilter::FilterDate) : "<none>") << endl;
    txtStream << endl;
}
