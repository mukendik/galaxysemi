#include "gex_database_filter.h"
#include "gex_constants.h"
#include "gex_shared.h"

GexDatabaseFilter::GexDatabaseFilter()
{
    reset();
}

const QList<QPair<QString, QString> > &GexDatabaseFilter::narrowFilters() const
{
    return mNarrowFilters;
}

const QString& GexDatabaseFilter::queryFilter() const
{
    return mQueryFilter;
}

void GexDatabaseFilter::reset()
{
    // Clear the query filter
    mQueryFilter.clear();

    // Clear narrow filter
    mNarrowFilters.clear();

    // clear other fields
    strDatabaseLogicalName.clear();
    strDataTypeQuery.clear();
    bOfflineQuery       = false;
    iTimePeriod         = -1;
    iTimeNFactor        = 0;
    m_eTimeStep         = DAYS;
    calendarFrom        = QDate();
    calendarTo          = QDate();
    calendarFrom_Time   = QTime();
    calendarTo_Time     = QTime();

    bConsolidatedData   = false;
}

void
GexDatabaseFilter::addNarrowFilter(const QString &name, const QString &value)
{
    if (!name.isEmpty() && name != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE])
        mNarrowFilters.append(QPair<QString,QString>(name, value));
}

void GexDatabaseFilter::setQueryFilter(const QString &name)
{
    if (!name.isEmpty() && name != gexLabelFilterChoices[GEX_QUERY_FILTER_NONE])
        mQueryFilter = name;
}
