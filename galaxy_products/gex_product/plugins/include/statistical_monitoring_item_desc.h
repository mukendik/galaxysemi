#ifndef STATISTICAL_MONITORING_ITEM_DESC_H
#define STATISTICAL_MONITORING_ITEM_DESC_H

#include <QString>
#include <QStringList>
#include "statistical_monitoring_monitored_item_unique_key_rule.h"

struct MonitoredItemDesc
{
private:
    QString uniqueKey;
public:
    QString type;
    QString num;
    QString name;
    QString cat;
    QString unit;
    int scale;

private:
    MonitoredItemDesc(QString uniqueKey, QString type, QString num, QString name, QString cat, QString unit, int scale)
        : uniqueKey(uniqueKey), type(type), num(num), name(name), cat(cat), unit(unit), scale(scale)
    {
    }

public:
    static MonitoredItemDesc CreateItemDesc(MonitoredItemUniqueKeyRule keyRule, QString type, QString num, QString name, QString cat="", QString unit="", int scale=0)
    {
        switch(keyRule)
        {
            case useNum:
                return MonitoredItemDesc(type + "|" + num, type, num, name, cat, unit, scale);
            case useName:
                return MonitoredItemDesc(type + "|" + name, type, num, name, cat, unit, scale);
            case useNumAndName:
            default:
                return MonitoredItemDesc(type + "|" + num + "|" + name, type, num, name, cat, unit, scale);
        }
    }

    bool equals(const MonitoredItemDesc& other) const { return this->uniqueKey == other.uniqueKey; }

    bool listLessThan(const QStringList& aRef, const QStringList& aOther, const int& aStartingIndex) const
    {
        if (aRef.at(aStartingIndex) == aOther.at(aStartingIndex))
        {
            int lNextIndex = aStartingIndex + 1;
            if ((aRef.size() <= lNextIndex) || (aOther.size() <= lNextIndex)) return aRef.size() < aOther.size();
            else return listLessThan(aRef, aOther, lNextIndex);
        }
        else return aRef.at(aStartingIndex).toInt() < aOther.at(aStartingIndex).toInt();
    }

    bool lessThan(const MonitoredItemDesc& other) const
    {
        return listLessThan(this->num.split('+'), other.num.split('+'), 0);
    }

    uint qHash(uint seed) const { return ::qHash(uniqueKey, seed); }
};

inline bool operator==(const MonitoredItemDesc& t1, const MonitoredItemDesc& t2)
{
    return t1.equals(t2);
}

static inline bool lessThan(const MonitoredItemDesc &t1, const MonitoredItemDesc &t2)
{
    return t1.lessThan(t2) ;
}

inline uint qHash(const MonitoredItemDesc& key, uint seed)
{
    return key.qHash(seed);
}

#endif // STATISTICAL_MONITORING_ITEM_DESC_H
