#ifndef CONSOLIDATION_TREE_PERIOD_H
#define CONSOLIDATION_TREE_PERIOD_H

#include <QDate>

class CTPeriod
{
public:

    CTPeriod();
    CTPeriod(const QDate& begin, const QDate& end);
    CTPeriod(const CTPeriod& other);

    const QDate&    beginDate() const;
    const QDate&    endDate() const;

    void            setBeginDate(const QDate& begin);
    void            setEndDate(const QDate& end);

    bool            isValid() const;
    bool            contains(const QDate& date) const;
    bool            contains(const CTPeriod& period) const;
    bool            intersects(const CTPeriod& period) const;

    bool            operator<(const CTPeriod& period) const;
    bool            operator==(const CTPeriod& period) const;
    CTPeriod&       operator=(const CTPeriod& other);

private:

    QDate           m_beginDate;
    QDate           m_endDate;
};

#endif // CONSOLIDATION_TREE_PERIOD_H
