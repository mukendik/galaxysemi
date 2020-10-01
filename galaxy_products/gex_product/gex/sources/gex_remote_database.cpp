#include "db_external_database.h"
#include "gexdb_plugin_sya.h"
#include "gex_database_filter.h"
#include "engine.h"
#include "db_engine.h"
#include <gqtl_log.h>

bool GexRemoteDatabase::SPM_GetProductList(QString testingStage,
                                           QString productRegexp,
                                           QStringList & cMatchingValues)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SPM_GetProductList(testingStage, productRegexp, cMatchingValues);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}

bool GexRemoteDatabase::SPM_GetFlowList(QString testingStage,
                                        QString productRegexp,
                                        QStringList & cMatchingValues)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SPM_GetFlowList(testingStage,productRegexp,cMatchingValues);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}
bool GexRemoteDatabase::SPM_GetInsertionList(QString testingStage,
                                             QString productRegexp,
                                             QString flowRegexp,
                                             QStringList & cMatchingValues)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SPM_GetInsertionList(testingStage,productRegexp,flowRegexp,cMatchingValues);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}
bool GexRemoteDatabase::SPM_GetItemsList(QString testingStage,
                                         QString productRegexp,
                                         QString flowRegexp,
                                         QString insertionRegexp,
                                         QString testType,
                                         QStringList & cMatchingValues)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SPM_GetItemsList(testingStage,productRegexp,flowRegexp,insertionRegexp,testType,cMatchingValues);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}

bool GexRemoteDatabase::SPM_FetchWaferKeysFromFilters(QString testingStage,
                                                      QString productRegexp,
                                                      QString lotId,
                                                      QString sublotId,
                                                      QString waferId,
                                                      const QMap<QString, QString>& filtersMetaData,
                                                      QStringList &waferKeyList)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SPM_FetchWaferKeysFromFilters(testingStage, productRegexp, lotId, sublotId, waferId, filtersMetaData, waferKeyList);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}

bool GexRemoteDatabase::SPM_GetConditionsFromFilters(QString testingStage,
                                                     const QMap<QString, QString>& filtersMetaData,
                                                     QMap<QString, QStringList>& filtersConditions)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SPM_GetConditionsFromFilters(testingStage, filtersMetaData, filtersConditions);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}

bool GexRemoteDatabase::SPM_FetchDataPointsForComputing(QString testingStage,
                                                        QString productRegexp,
                                                        QString monitoredItemType,
                                                        const QList<MonitoredItemRule>& monitoredItemRules,
                                                        MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                        QString testFlow,
                                                        QString consolidationType,
                                                        QString consolidationLevel,
                                                        QString testInsertion,
                                                        const QStringList& statsToMonitor,
                                                        QString siteMergeMode,
                                                        bool useGrossDie,
                                                        const QMap<QString, QStringList>& filtersConditions,
                                                        QDateTime dateFrom,
                                                        QDateTime dateTo,
                                                        QStringList& productsMatched,
                                                        int& numLotsMatched,
                                                        int& numDataPointsMatched,
                                                        QSet<int>& siteList,
                                                        QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > > &testToSiteToStatToValues)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SPM_FetchDataPointsForComputing(testingStage, productRegexp,
                                                                           monitoredItemType, monitoredItemRules,
                                                                           uniqueKeyRule, testFlow,
                                                                           consolidationType, consolidationLevel,
                                                                           testInsertion, statsToMonitor,
                                                                           siteMergeMode, useGrossDie,
                                                                           filtersConditions, dateFrom, dateTo,
                                                                           productsMatched, numLotsMatched,
                                                                           numDataPointsMatched,
                                                                           siteList, testToSiteToStatToValues);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}

bool GexRemoteDatabase::SPM_FetchDataPointsForCheckOnTrigger(QString testingStage,
                                                             QString productRegexp,
                                                             QString lotId,
                                                             QString sublotId,
                                                             QString waferId,
                                                             const QList<MonitoredItemDesc>& testList,
                                                             MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                             QString testFlow, //in
                                                             QString consolidationType, //in
                                                             QString consolidationLevel, //in
                                                             QString testInsertion, //in
                                                             const QList<int>& siteList,
                                                             const QList<QString>& statsList,
                                                             bool useGrossDie,
                                                             const QDateTime* dateFrom,
                                                             const QDateTime* dateTo,
                                                             const QMap<QString, QStringList>& filtersConditions,
                                                             QString& productList,
                                                             QString& lotList,
                                                             QString& sublotList,
                                                             QString& waferList,
                                                             int& numParts,
                                                             QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& testToSiteToStatToDataPoint)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SPM_FetchDataPointsForCheckOnTrigger(
                testingStage, productRegexp, lotId, sublotId, waferId, testList,
                uniqueKeyRule, testFlow, consolidationType, consolidationLevel,
                testInsertion, siteList, statsList, useGrossDie, dateFrom, dateTo,
                filtersConditions, productList, lotList, sublotList, waferList, numParts,
                testToSiteToStatToDataPoint);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}

bool GexRemoteDatabase::SPM_FetchDataPointsForCheckOnInsertion(QString testingStage,
                                                               int splitlotId,
                                                               const QMap<QString,QString> &filtersMetaData,
                                                               const QList<MonitoredItemDesc>& testList,
                                                               MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                               const QList<int>& siteList,
                                                               const QList<QString>& statsList,
                                                               QString& productList,
                                                               QString& lotList,
                                                               QString& sublotList,
                                                               QString& waferList,
                                                               int& numParts,
                                                               QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& testToSiteToStatToDataPoint)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SPM_FetchDataPointsForCheckOnInsertion(testingStage, splitlotId, filtersMetaData, testList, uniqueKeyRule, siteList, statsList, productList, lotList, sublotList, waferList, numParts, testToSiteToStatToDataPoint);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}

bool GexRemoteDatabase::SYA_FetchDataPointsForComputing(QString testingStage,
                                                        QString productRegexp,
                                                        const QMap<QString,QString>& filters,
                                                        QString monitoredItemType,
                                                        const QList<MonitoredItemRule>& monitoredItemRules,
                                                        const QStringList& binsToExclude,
                                                        MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                        QString testFlow,
                                                        QString consolidationType,
                                                        QString consolidationLevel,
                                                        QString testInsertion,
                                                        const QStringList &statsToMonitor,
                                                        QString siteMergeMode,
                                                        bool useGrossDie,
                                                        QDateTime computeFrom,
                                                        QDateTime computeTo,
                                                        QStringList& productsMatched,
                                                        int& numLotsMatched,
                                                        int& numDataPointsMatched,
                                                        QSet<int>& siteList,
                                                        QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, double> > >& monitoredItemToSiteToStatToValues)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SYA_FetchDataPointsForComputing(testingStage, productRegexp, filters, monitoredItemType, monitoredItemRules, binsToExclude, uniqueKeyRule, testFlow, consolidationType, consolidationLevel, testInsertion, statsToMonitor, siteMergeMode, useGrossDie, computeFrom, computeTo, productsMatched, numLotsMatched, numDataPointsMatched, siteList, monitoredItemToSiteToStatToValues);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}

bool GexRemoteDatabase::SYA_FetchDataPointsForCheckOnTrigger(QString testingStage,
                                                             QString productRegexp,
                                                             QString lotId,
                                                             QString sublotId,
                                                             QString waferId,
                                                             const QMap<QString,QString> &filtersMetaData,
                                                             const QList<MonitoredItemDesc> &binList,
                                                             const QStringList& binsToExclude, //in
                                                             MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                             QString testFlow, //in
                                                             QString consolidationType, //in
                                                             QString consolidationLevel, //in
                                                             QString testInsertion, //in
                                                             const QList<int> &siteList,
                                                             const QList<QString>& statsList,
                                                             bool useGrossDie,
                                                             const QDateTime* dateFrom,
                                                             const QDateTime* dateTo,
                                                             QString& productList,
                                                             QString& lotList,
                                                             QString& sublotList,
                                                             QString& waferList,
                                                             int& numParts,
                                                             QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& binToSiteToStatToDataPoint)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SYA_FetchDataPointsForCheckOnTrigger(testingStage, productRegexp, lotId, sublotId, waferId, filtersMetaData, binList, binsToExclude, uniqueKeyRule, testFlow, consolidationType, consolidationLevel, testInsertion, siteList, statsList, useGrossDie, dateFrom, dateTo, productList, lotList, sublotList, waferList, numParts, binToSiteToStatToDataPoint);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}

bool GexRemoteDatabase::SYA_FetchDataPointsForCheckOnInsertion(QString testingStage,
                                                               int splitlotId,
                                                               const QMap<QString,QString>& filters,
                                                               const QList<MonitoredItemDesc> &binList,
                                                               const QStringList& binsToExclude,
                                                               MonitoredItemUniqueKeyRule uniqueKeyRule,
                                                               const QList<int> &siteList,
                                                               const QList<QString> &statsList,
                                                               bool useGrossDie,
                                                               QString& productList,
                                                               QString& lotList,
                                                               QString& sublotList,
                                                               QString& waferList,
                                                               int& numParts,
                                                               QHash<MonitoredItemDesc, QHash<int, QMultiHash<QString, StatMonDataPoint> > >& binToSiteToStatToDataPoint)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : plugin NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->SYA_FetchDataPointsForCheckOnInsertion(testingStage, splitlotId, filters, binList, binsToExclude, uniqueKeyRule, siteList, statsList, useGrossDie, productList, lotList, sublotList, waferList, numParts, binToSiteToStatToDataPoint);
    if(!bStatus)
    {
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
    }

    return bStatus;
}
