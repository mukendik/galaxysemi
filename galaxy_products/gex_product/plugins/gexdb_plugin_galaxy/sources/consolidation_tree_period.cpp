#include "consolidation_tree_period.h"

CTPeriod::CTPeriod()
{
}

CTPeriod::CTPeriod(const QDate &begin, const QDate &end)
    : m_beginDate(begin), m_endDate(end)
{
}

CTPeriod::CTPeriod(const CTPeriod &other)
{
    m_beginDate = other.m_beginDate;
    m_endDate   = other.m_endDate;
}

const QDate &CTPeriod::beginDate() const
{
    return m_beginDate;
}

const QDate &CTPeriod::endDate() const
{
    return m_endDate;
}

void CTPeriod::setBeginDate(const QDate &begin)
{
    m_beginDate = begin;
}

void CTPeriod::setEndDate(const QDate &end)
{
    m_endDate = end;
}

bool CTPeriod::isValid() const
{
    if (m_beginDate.isNull() == false && m_endDate.isNull() == false)
        return m_beginDate <= m_endDate;

    return true;
}

bool CTPeriod::contains(const QDate &date) const
{
    if ((m_beginDate.isNull() || date >= m_beginDate) &&
            (m_endDate.isNull() || (date.isNull() == false && date <= m_endDate)))
        return true;

    return false;
}

bool CTPeriod::contains(const CTPeriod &period) const
{
    if ((m_beginDate.isNull() || period.m_beginDate >= m_beginDate) &&
            (m_endDate.isNull() || (period.m_endDate.isNull() == false && period.m_endDate <= m_endDate)))
        return true;

    return false;
}

bool CTPeriod::intersects(const CTPeriod &period) const
{
    if (contains(period.beginDate()) || contains(period.endDate()))
        return true;

    if (period.contains(beginDate()) || period.contains(endDate()))
        return true;

    return false;
}

bool CTPeriod::operator <(const CTPeriod &period) const
{
    if (m_beginDate < period.m_beginDate)
        return true;

    if (m_beginDate == period.m_beginDate)
        return (m_endDate < period.m_endDate);

    return false;
}

bool CTPeriod::operator ==(const CTPeriod &period) const
{
    return ((m_beginDate == period.m_beginDate) && (m_endDate == period.m_endDate));
}

CTPeriod &CTPeriod::operator =(const CTPeriod &other)
{
    if (this != &other)
    {
        m_beginDate = other.m_beginDate;
        m_endDate   = other.m_endDate;
    }

    return *this;
}
