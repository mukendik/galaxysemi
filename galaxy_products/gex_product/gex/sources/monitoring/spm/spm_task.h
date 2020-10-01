#ifndef SPM_TASK_H
#define SPM_TASK_H

#include "statistical_monitoring_task.h"

///
/// \brief The CGexMoTaskSPM class specializes the CGexMoTaskStatisticalMonitoring for SPM processing
///
class CGexMoTaskSPM : public CGexMoTaskStatisticalMonitoring
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskSPM)

public:
    explicit CGexMoTaskSPM(QString email="", QObject* parent=NULL);
protected:
    CGexMoTaskSPM(CGexMoTaskSPM* orig);
public:
    virtual ~CGexMoTaskSPM();
    CGexMoTaskStatisticalMonitoring* Clone();

    DuplicateStatus GetDuplicatedParam(CGexMoTaskStatisticalMonitoring& other);

    // CGexMoTaskItem virtual methods overrides:

    CGexMoTaskItem* Duplicate(QString copyName);
    CGexMoTaskItem* CreateTask(SchedulerGui* gui, bool readOnly);
    void UpdateListView(SchedulerGui* gui, int nCurrentRow, bool allowEdit);

    QString ExecuteTask(GS::QtLib::DatakeysContent &aDbKeysContent, const QString &aMailFilePath);

public slots:

    // CGexMoTaskItem virtual methods overrides:

    int GetTaskType();
    QString GetTypeName();

private:
    bool ParseTaskOptions(const QMap<QString, QString>& taskOptions);
    bool ParseOldTaskOptions(const QMap<QString, QString>& taskOptions);
    virtual const QMap<QString, QString>& GetTaskLightWeightOptions();

    QString BuildTaskOptionsQuery();
    bool MigrateTaskDetailsFromTdrToYmAdminDb();

    QString GetStatisticalMonitoringTablesPrefix();

    bool CheckComputeSpecificRequirements();

    bool FetchDataPointsForComputing(QString testingStage,
                                     QString productRegexp,
                                     QMap<QString, QString> filtersMetaData,
                                     QString monitoredItemType,
                                     QList<MonitoredItemRule> monitoredItemRules,
                                     QStringList excludedItems,
                                     MonitoredItemUniqueKeyRule uniqueKeyRule,
                                     QString testFlow,
                                     QString consolidationType,
                                     QString consolidationLevel,
                                     QString testInsertion,
                                     QStringList statsToMonitor,
                                     QString siteMergeMode,
                                     bool useGrossDie,
                                     QDateTime computeFrom,
                                     QDateTime computeTo,
                                     QStringList& productList,
                                     int& numLots,
                                     int& numDataPoints,
                                     QSet<int>& siteList,
                                     QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& monitoredItemToSiteToStatToValues);

    QString GetMonitoredItemDesignation() {return "Test";}

    void ProcessSpecificCleanLimits(const QString &itemCat,
                                    double &ll,
                                    bool &llEnabled,
                                    double &hl,
                                    bool &hlEnabled);

    bool FetchDataPointsForCheck(QChar ExecutionType,
                                 QString testingStage,
                                 QString productName,
                                 QString lot,
                                 QString sublot,
                                 QString wafer,
                                 int splitlot,
                                 QMap<QString, QString> filtersMetaData,
                                 QList<MonitoredItemDesc> monitoredItemList,
                                 QStringList excludedItems,
                                 MonitoredItemUniqueKeyRule uniqueKeyRule,
                                 QString testFlow,
                                 QString consolidationType,
                                 QString consolidationLevel,
                                 QString testInsertion,
                                 QList<int> siteList,
                                 QStringList statList,
                                 bool useGrossDie,
                                 const QDateTime* dateFrom,
                                 const QDateTime* dateTo,
                                 QString& resolvedProductList,
                                 QString& resolvedLotList,
                                 QString& resolvedSublotList,
                                 QString& resolvedWaferList,
                                 int& resolvedNumParts,
                                 QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& monitoredItemToSiteToStatToDataPoint);

    GexDatabaseEntry* GetADREntry(const QString &lDBName);
};
#endif // SPM_TASK_H
