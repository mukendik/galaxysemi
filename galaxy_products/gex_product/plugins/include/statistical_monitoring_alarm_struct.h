#ifndef SM_ALARM_STRUCT_H
#define SM_ALARM_STRUCT_H

#include <QString>
#include <QDateTime>

///
/// \brief The StatMonAlarm struct stores the data related to a statistical monitoring task alarm for later printing
///
struct StatMonAlarm
{
public:
    QDateTime execDate;
    QString productName;
    QString lotId;
    QString sublotId;
    QString waferId;
    int splitlotId;
    int criticityLevel;
    int MonitoredItemId;
    QString monitoredItemType;
    QString monitoredItemNum;
    QString monitoredItemName;
    QString monitoredItemCat;
    QString monitoredItemUnit;
    int limitId;
    QString statName;
    int siteNum;
    int execCount;
    int failCount;
    double lowLimit;
    double outlierValue;
    double highLimit;
    bool alreadyWritten;

    StatMonAlarm(QDateTime execDate,
             QString productName,
             QString lotId,
             QString sublotId,
             QString waferId,
             int splitlotId,
             int criticityLevel,
             int monitoredItemId,
             QString monitoredItemType,
             QString monitoredItemNum,
             QString monitoredItemName,
             QString monitoredItemCat,
             QString monitoredItemUnit,
             int limitId,
             QString statName,
             int siteNum,
             int execCount,
             int failCount,
             double lowLimit,
             double outlierValue,
             double highLimit)
        : execDate(execDate),
          productName(productName),
          lotId(lotId),
          sublotId(sublotId),
          waferId(waferId),
          splitlotId(splitlotId),
          criticityLevel(criticityLevel),
          MonitoredItemId(monitoredItemId),
          monitoredItemType(monitoredItemType),
          monitoredItemNum(monitoredItemNum),
          monitoredItemName(monitoredItemName),
          monitoredItemCat(monitoredItemCat),
          monitoredItemUnit(monitoredItemUnit),
          limitId(limitId),
          statName(statName),
          siteNum(siteNum),
          execCount(execCount),
          failCount(failCount),
          lowLimit(lowLimit),
          outlierValue(outlierValue),
          highLimit(highLimit),
          alreadyWritten(false)
    {
    }

};

#endif // SM_ALARM_STRUCT_H
