#ifndef SYA_TASK_H
#define SYA_TASK_H

#include "gexdb_plugin_base.h"
#include "statisticalMonitoring/statistical_monitoring_task.h"

class CGexOldSYAParameters
{
public:
    CGexOldSYAParameters();	// Constructor

    int     mBiningType;                // Binning type: 0=Good bins, 1=Failing bins
    int     mAlarmLevel;                // Ranges in 0-100
    int     mAlarmIfOverLimit;          // 0= alarm if Yield Under Limit, 1= alarm if Yield Over limit
    long    mMinimumyieldParts;         // Minimum parts required to check the yield.
    QString mSblFile;                   // Full path to SBL/YBL data file (for Statisti Limits monitoring)
    int     mEmailReportType;           // GEXMO_YIELD_EMAILREPORT_xxx  :ASCII text in BODY, Excel, Word,  PPT, PDF
    int     mNotificationType;          // 0= Report attached to email, 1= keep on server, only email its URL
    int     mExceptionLevel;            // 0=Standard, 1=Critical...
    int     mRenewalDateRangeUnit;      // 0=Months, 1=Weeks, 2=days
    int     mRenewalDateRange;
    OutlierRule mSYA_Rule;              // Rule: 0=N*Sigma, 1=N*IQR,...
    bool    mSYA_IgnoreDataPointsWithNullSigma;	// Set to true if datapoints with null sigma should be ignored
    QDate   mExpirationDate;
    int     mExpirationWarningReprieve;
    int     mExpirationAlgo;
    // key is bin number (-1 = All Pass)
    // values are : <field , value> : (field = "RuleType", "N1", "N2", ...
    // example : mapBins_rules[1]=QMap<>{("RuleType", 1), ("N1", 6.0), ("N2", 4.0)}
    QMap<int, QMap<QString, QVariant> > mMapBins_rules;
    QString mExcludedBins;

    void    clear(void);                    // Reset variables
;
};



///
/// \brief The CGexMoTaskSYA class specializes the CGexMoTaskStatisticalMonitoring for SYA processing
///
class CGexMoTaskSYA : public CGexMoTaskStatisticalMonitoring
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskSYA)

public:
    explicit CGexMoTaskSYA(QString email="", QObject* parent=NULL);
protected:
    CGexMoTaskSYA(CGexMoTaskSYA* orig);
public:
    virtual ~CGexMoTaskSYA();
    CGexMoTaskStatisticalMonitoring *Clone();

    DuplicateStatus GetDuplicatedParam(CGexMoTaskStatisticalMonitoring& other);

    // CGexMoTaskItem virtual methods overrides:

    CGexMoTaskItem* Duplicate(QString copyName);
    CGexMoTaskItem* CreateTask(SchedulerGui* gui, bool readOnly);
    void UpdateListView(SchedulerGui* gui, int nCurrentRow, bool allowEdit);
    QString ExecuteTask(GS::QtLib::DatakeysContent &aDbKeysContent, const QString& aMailFilePath);

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
                                     QMap<QString, QString> filters,
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

    QString GetMonitoredItemDesignation() {return "Binning";}

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
                                 QMap<QString, QString> filters,
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

    QString ConvertRuleType(QString oldRuleType);
    bool ExceedLimits(double value, double lowLimit, bool lowLimitEnabled, double highLimit, bool highLimitEnabled, double threshold , const QString &categorie);

    CGexOldSYAParameters* mOldSYATaskParam;      // Contains the old SYA parameters (read from the old task)
};

#endif // SYA_TASK_H
