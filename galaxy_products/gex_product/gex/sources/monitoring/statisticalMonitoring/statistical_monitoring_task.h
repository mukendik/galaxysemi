#ifndef CGEXMOTASKSTATISTICALMONITORING_H
#define CGEXMOTASKSTATISTICALMONITORING_H

#include <gstdl_errormgr.h>
#include "mo_task.h"
#include "statistical_monitoring_monitored_item_unique_key_rule.h"

class GexMoStatisticalMonitoringTaskData;
struct MonitoredItemRule;
struct MonitoredItemDesc;
struct StatMonDataPoint;
struct StatMonAlarm;
struct StatMonLimit;
class QSqlQuery;
class SMSQLTableModel;
namespace GS{namespace QtLib{ class DatakeysContent;}}

enum RenewalResult
{
    RenewalOK,
    RenewalFailed,
    RenewalDuplicated
};

enum RenewalStatus
{
    OK,
    Warning,
    Warning24h,
    Expired,
    LicenseExpired
};

enum DuplicateStatus
{
    NOTDUPLICATED,
    TITLEDUPLICATED,
    MAININFODUPLICATEDONTRIGGER,
    MAININFODUPLICATEDONINSERTION,
    FILTERSDULPLICATEDONINSERTION
};

class CGexMoTaskStatisticalMonitoring : public CGexMoTaskItem
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskStatisticalMonitoring)

public:
    explicit CGexMoTaskStatisticalMonitoring(QObject* parent=NULL);
protected:
    CGexMoTaskStatisticalMonitoring(CGexMoTaskStatisticalMonitoring* orig);
public:
    virtual ~CGexMoTaskStatisticalMonitoring();
    virtual CGexMoTaskStatisticalMonitoring* Clone() = 0;

    GDECLARE_ERROR_MAP(CGexMoTaskStatisticalMonitoring)
    {
        eErrNullPointer,
        eErrInvalidTask,
        eErrUnexpectedTaskState,
        eErrMailSendFailed,
        eErrRenewalCheckFailed,
        eErrCloneLimitsForRenewalFailed,
        eErrComputeLimitRulesError,
        eErrTaskExpired,
        eErrPluginError,
        eErrDataBaseError,
        eErrNoDataPointsError,
        eErrLicenseError
    }
    GDECLARE_END_ERROR_MAP(CGexMoTaskStatisticalMonitoring)

    virtual DuplicateStatus GetDuplicatedParam(CGexMoTaskStatisticalMonitoring& other) = 0;

    ///
    /// \brief Loads a lightWeight definition of the task details from the ym_admin_db (new fragrance)
    ///  into the current object
    /// \param the version to load, if not specified, the latest one will be used
    /// \return true if succeeded, false otherwise
    ///
    bool LoadTaskLightWeightDetails();

    ///
    /// \brief Loads the task details from the ym_admin_db (new fragrance) into the current object
    /// \param the version to load, if not specified, the latest one will be used
    /// \return true if succeeded, false otherwise
    ///
    bool LoadTaskDetails(int version = 0);

    ///
    /// \brief Gets the latest production version (ie. not draft) number
    /// \return -1 if no prod version exists, the prod version number otherwise
    ///
    int GetActiveProdVersionId();

    ///
    /// \brief Duplicates the provided version limits to the draft
    /// \param the version to duplicate
    /// \return true if succeeded, false otherwise
    ///
    bool DuplicateVersionToDraft(int version);

    ///
    /// \brief Returns whether this task has limits defined
    /// \param the version of this task to check, if not provided, the latest version is used
    /// \return true if limits are defined, false otherwise
    ///
    bool HasComputedLimits(int version = 0);

    ///
    /// \brief Checks the limits against the provided data set
    /// using the currently loaded version
    /// \param Name of the product to check
    /// \param Lot identifier to check (* or empty to check all lots)
    /// \param SubLot identifier to check (* or empty to check all sublots)
    /// \param wafer identifier to check (* or empty to check all wafers)
    /// \param ExecutionType: M (manual), T (trigger), I (insertion), S (Simulate)
    /// \param The log summary of the execution
    /// \param The alarms raised by the execution
    /// \param Set of datapoint used to compute alarms
    /// \param Set of limit items for this rule
    /// \param The date from which the datapoints should be checked
    /// \param The date to which the datapoints should be checked
    /// \return true if succeeded, false otherwise
    ///
    bool CheckAgainstLimits(
            const QString& productName,
            const QString& lot,
            const QString& sublot,
            const QString& wafer,
            const int& slpitlot,
            const QChar& ExecutionType,
            QMap<QString, QVariant>& logSummary,
            QList<StatMonAlarm>& alarmsOutput,
            QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > > &monitoredItemToSiteToStatToDataPoint,
            QHash<MonitoredItemDesc, QMap<int, StatMonLimit> > &aItemsLimits,
            const QDateTime* dateFrom = NULL,
            const QDateTime* dateTo = NULL);

    ///
    /// \brief Renew the active production version of the provided task
    /// \return one of the different renewal status enum
    ///
    RenewalResult RenewActiveVersion();

    ///
    /// \brief Gets the task's version model for displaying in a tableView
    /// \return if succeeded or not
    ///
    SMSQLTableModel* CreateVersionsModel();

    ///
    /// \brief Build extraction query of limits associated with the task and to a given version
    /// \return a SQL query string
    ///
    QString GetLimitsExtractionQuery(QStringList visibleFields,
                                     QStringList idFields,
                                     bool lastProdVersion = false);

    ///
    /// \brief Update stats table with param values using filters
    ///
    bool UpdateStatTable(QString tableName,
                         QList<QPair<QString, QString > > updateFieldValuePairs,
                         QList<QPair<QString, QString > > filterFieldValuePairs);

    // CGexMoTaskItem virtual methods overrides:

    bool LoadTaskDataFromDb();
    bool SaveTaskDataToDb();
    void SaveTaskToXML(QTextStream& XMLStream);
    void UpdatePrivateAttributes();

    virtual QString GetMonitoredItemDesignation() = 0;

    virtual QString ExecuteTask(GS::QtLib::DatakeysContent &dbKeysContent, const QString& aMailFilePath) = 0;

public slots:
    ///
    /// \brief Computes the task's limits using the
    /// currently loaded version
    /// \param whether the limits are updated or newly computed
    /// \return true if succeeded, false otherwise
    ///
    bool ComputeLimits(bool update=false,
                       const QList<QPair<int, QString> >* const manualItemsNumNames = NULL,
                       int version = -1);

    ///
    /// \brief Validates the current draft (provided a draft is currently loaded)
    /// as a prod version, and create a new draft for further processing which is
    /// immediately loaded into the current task
    /// \return true if succeeded, false otherwise
    ///
    bool ValidateCurrentDraft();

    void SetProperties(GexMoStatisticalMonitoringTaskData* properties);
    GexMoStatisticalMonitoringTaskData* GetProperties();

    // CGexMoTaskItem virtual methods overrides:

    bool IsUsable(bool bCheckExecutionWindows=false, bool bCheckFrequency=false);
    QString GetDatabaseName();
    bool SetDatabaseName(QString DatabaseName);

protected:
    ///
    /// \brief Gets the table prefix associated with the task's testing stage
    /// \return a TDR table prefix
    ///
    QString GetTDRTablePrefix();

    MonitoredItemUniqueKeyRule mUniqueKeyRule;

private:
    GexMoStatisticalMonitoringTaskData* mProperties;
    bool mLoadedFromDb;
    bool mLightWeightOnly;

    void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption);

    ///
    /// \brief Gets the expiration and warning dates of the task's active prod (no draft) version
    /// \param If succeeded, this parameter contains the id of the version
    /// \param If succeeded, this parameter contains the expiration date
    /// \param If succeeded, this parameter contains the warning date
    /// \param If failed, this parameters contains the error message
    /// \return true if succeeded, false otherwise
    ///
    bool GetActiveProdVersionExpirationDate(int& id, QDateTime& expirationDate, QDateTime& warningDate, QString& errorMsg);

    ///
    /// \brief Gets the expiration and warning dates of the task's latest prod (no draft) version
    /// \param If succeeded, this parameter contains the id of the version
    /// \param If succeeded, this parameter contains the start date
    /// \param If failed, this parameters contains the error message
    /// \return true if succeeded, false otherwise
    ///
    bool GetLatestProdVersionExpirationDate(int& id, QDateTime& startTime, QString& errorMsg);

    virtual QString GetStatisticalMonitoringTablesPrefix() = 0;

    virtual bool ParseTaskOptions(const QMap<QString, QString>& taskOptions) = 0;
    virtual bool ParseOldTaskOptions(const QMap<QString, QString>& taskOptions) = 0;
    virtual const QMap<QString, QString>& GetTaskLightWeightOptions() = 0;


    ///
    /// \brief Migrates the task details from the TDR (old school) to the ym_admin-db (new fragrance)
    /// \return true if succeeded, false otherwise
    ///
    virtual bool MigrateTaskDetailsFromTdrToYmAdminDb() = 0;

    virtual QString BuildTaskOptionsQuery() = 0;

    ///
    /// \brief Saves the currently loaded version into the TDR
    /// \return true if succeeded, false otherwise
    ///
    bool SaveTaskDetails();

    bool DeleteTaskDetails();

    ///
    /// \brief Deletes the specified version from the TDR
    /// \param the version to delete
    /// \return true if succeeded, false otherwise
    ///
    bool DeleteVersionFromDB(int version);

    ///
    /// \brief Duplicates (in the database) the stats of the source version of this task into the destination version
    /// \param the source version number
    /// \param the destination version number
    /// \return true if succeeded, false otherwise
    ///
    bool DbDuplicateLimits(int srcVersionId, int destVersionId);

    virtual bool CheckComputeSpecificRequirements() = 0;

    virtual bool FetchDataPointsForComputing(QString testingStage,
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
                                             QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& monitoredItemToSiteToStatToValues) = 0;

    bool CreateNewMonitoredItems(const QString& monitoredItemType,
                                 const QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& monitoredItemToSiteToStatToValues,
                                 QHash<MonitoredItemDesc, int>& registeredItemsIds);

    bool CreateNewLimits(const int version,
                         const QList<MonitoredItemRule> &monitoredItemRules,
                         const QHash<MonitoredItemDesc, int> &registeredItemsIds,
                         const QSet<int> &sites,
                         const QStringList &stats,
                         const QString &defaultAlgorithm);

    virtual void ProcessSpecificCleanLimits(const QString& itemCat,
                                            double& ll,
                                            bool& llEnabled,
                                            double& hl,
                                            bool& hlEnabled) = 0;

    QString HasUnit(QString stat);

    ///
    /// \brief Cleans the task's limits using the
    /// currently loaded version
    /// \return
    ///
    bool CleanLimits();

    ///
    /// \brief Checks whether the task needs to be renewed and, if so
    /// performs the user configured actions.
    /// \return true if the task can be run, false otherwise
    ///
    bool CheckForRenewal();

    bool CheckRenewalStatus(RenewalStatus& status, bool& sendMail, bool& renew);

    bool SetWarningDone(int value);

    ///
    /// \brief Clones the last validated prod version of the task as a new
    /// prod version in the database. Does not affect the current task.
    /// \return true if succeeded, false otherwise
    ///
    bool DbCloneActiveProdVersionForRenewal(int& newProdVersion);

    bool WriteLog(const QMap<QString, QVariant>& logSummary);

    bool UpdateLog(int logId, const QMap<QString, QVariant>& logSummary);

    virtual bool FetchDataPointsForCheck(QChar ExecutionType,
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
                                         QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& monitoredItemToSiteToStatToDataPoint) = 0;

    ///
    /// \brief Check that the value does not exceed the threshold
    /// categorie could be use to perfom the comparaison or not
    /// return true if yes (no if not)
    virtual bool ExceedLimits(double value, double lowLimit, bool lowLimitEnabled, double highLimit, bool highLimitEnabled, double threshold, const QString & itemCat);

    bool WriteAlarms(int logId, int taskId, int versionId, QList<StatMonAlarm>& alarms);

    MonitoredItemDesc CreateItemDesc(QString itemType, QString itemNum, QString itemName, QString itemCat="", QString itemUnit="", int itemScale=0);
};

#endif // CGEXMOTASKSTATISTICALMONITORING_H
