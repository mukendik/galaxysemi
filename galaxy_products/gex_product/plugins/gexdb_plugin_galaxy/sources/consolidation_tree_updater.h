#ifndef CONSOLIDATION_TREE_UPDATER_H
#define CONSOLIDATION_TREE_UPDATER_H

#include "consolidation_tree_data.h"

#include <QStringList>
#include <QDate>

class QDomElement;
class CTPeriod;
class CTAbstractCommand;
class GexDbPlugin_Galaxy;
class QTextEdit;

class ConsolidationTreeUpdater
{
public:

    ConsolidationTreeUpdater();
    ~ConsolidationTreeUpdater();

    bool                        prepare(const ConsolidationTreeData& refData, const ConsolidationTreeData& curData);
    bool                        execute(GexDbPlugin_Galaxy * pPlugin);

    const QString&              errorMessage() const;

private:

    void                        flushCommands();

    bool                        doTestingStage(const QString& testingStage);
    bool                        doDefault();
    bool                        doProduct();
    bool                        doTestCondition(const CTPeriod& period);
    bool                        doSet(const CTPeriod& period, bool defaultOnly = false);
    bool                        doRules(const CTPeriod& period, bool defaultOnly = false);
    bool                        doOptions();

    QStringList                 retrieveProducts();
    QStringList                 retrieveOptions();

    QList<CTPeriod>             createConsolidationPeriods();
    QList<CTPeriod>             createSetPeriods(const CTPeriod& period, bool defaultOnly);
    QList<CTPeriod>             createPeriods(const QList<CTPeriod>& oldPeriods, const QList<CTPeriod>& newPeriods,
                                              const CTPeriod& filterPeriod);

    bool                        m_hasTestCondition;
    QStringList                 m_products;
    QString                     m_currentTestingStage;
    QString                     m_currentProduct;
    QString                     m_xmlNew;
    QString                     m_xmlOld;

    ConsolidationTreeData       m_refData;
    ConsolidationTreeData       m_curData;

    QList<CTAbstractCommand*>   m_commands;

    QString                     m_errorMessage;
};

#endif // CONSOLIDATION_TREE_UPDATER_H
