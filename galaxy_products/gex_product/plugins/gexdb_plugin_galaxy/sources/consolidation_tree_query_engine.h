#ifndef CONSOLIDATION_TREE_QUERY_ENGINE_H
#define CONSOLIDATION_TREE_QUERY_ENGINE_H

#include "consolidation_tree_consolidation_rule.h"
#include "consolidation_tree_test_condition.h"
#include "consolidation_tree_period.h"
#include <QMap>

class CTQueryFilter;
class ConsolidationTree;
class ConsolidationTreeQueryEnginePrivate;

class ConsolidationTreeQueryEngine
{
public:

    ConsolidationTreeQueryEngine(const ConsolidationTree &consolidationTree);
    ~ConsolidationTreeQueryEngine();

    // Retrieve an option value
    bool                        findOption(const CTQueryFilter& filter, QString &optionValue);
    // Indicates whether a consolidation is defined for a given filter
    bool                        hasConsolidation(const CTQueryFilter& filter);
    // Retrieve the Test Conditions list using the given filter
    QList<CTTestCondition>      findTestConditions(const CTQueryFilter& filter);
    // Retrieve the Consolidation Rules list using the given filter
    // The Consolidation rules are ordered, so when exploring the list, every consolidation rule would have
    // its data ready to use.
    QList<CTConsolidationRule>  findConsolidationRules(const CTQueryFilter& filter);

    QList<CTPeriod>             findConsolidationPeriod(const CTQueryFilter& filter);

private:

    ConsolidationTreeQueryEnginePrivate *   m_qEnginePrivate;
    QMap<QString, QString>                  m_mapDefaultOptionValue;
};

#endif // CONSOLIDATION_TREE_QUERY_ENGINE_H
