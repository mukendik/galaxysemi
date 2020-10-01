#ifndef GEX_DATABASE_FILTER_H
#define GEX_DATABASE_FILTER_H

#include <QComboBox>
#include <QDateTime>

// Used to fill a List box with valid Filters values.
class GexDatabaseFilter
{
public:

    enum eTimeStep
    {
        DAYS,
        WEEKS,
        MONTHS,
        QUARTERS,
        YEARS
    };

    GexDatabaseFilter();

    QString     strDatabaseLogicalName;
    QString     strDataTypeQuery;	// Type of data to query on in SQL-DB (wafer sort, final test,...)
    bool        bOfflineQuery;		// true if query to be executed over local cache database, not remote database!
    int         iTimePeriod;		// Time period to consider.
    int         iTimeNFactor;
    eTimeStep   m_eTimeStep;       // Step used in Last N * X
    QDate       calendarFrom;		// Filter: From date
    QDate       calendarTo;			// Filter: To date
    QTime       calendarFrom_Time;	// Filter: From time
    QTime       calendarTo_Time;	// Filter: To time
    bool        bConsolidatedData;        // Extract only consolidated data

    const QList<QPair<QString, QString> >&  narrowFilters() const;
    const QString&                          queryFilter() const;

    void        addNarrowFilter(const QString& name, const QString& value);
    void        setQueryFilter(const QString& name);

    void        reset();

private:

    QString                             mQueryFilter;
    QList<QPair<QString, QString> >     mNarrowFilters;
};


#endif // GEX_DATABASE_FILTER_H
