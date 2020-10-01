#ifndef CONSOLIDATION_TREE_QUERY_ENGINE_P_H
#define CONSOLIDATION_TREE_QUERY_ENGINE_P_H

#include "consolidation_tree_consolidation_rule.h"
#include "consolidation_tree_data.h"

#include <QDomElement>

class CTQueryFilter;

class ConsolidationTreeQueryEnginePrivate
{
public:

    enum RuleSource
    {
        FromGroup   = 0x01,
        FromDefault = 0x02,
        FromBoth    = 0x03
    };

    Q_DECLARE_FLAGS(RuleSources, RuleSource);

    ConsolidationTreeQueryEnginePrivate(const ConsolidationTreeData& data);
    ~ConsolidationTreeQueryEnginePrivate();

    void                        setRuleSources(RuleSources source);

    QList<QDomElement>          findOption(const CTQueryFilter &filter);
    QList<QDomElement>          findTestingStageElement(const CTQueryFilter& filter);
    QList<QDomElement>          findGroupElement(const CTQueryFilter& filter);
    QList<QDomElement>          findDefaultElement(const CTQueryFilter& filter);
    QList<QDomElement>          findConditionElement(const CTQueryFilter& filter);
    QList<QDomElement>          findSetElement(const CTQueryFilter& filter);
    QList<QDomElement>          findRuleElement(const CTQueryFilter& filter);
    QList<QDomElement>          findConsolidationElement(const CTQueryFilter& filter);

    bool                        createConsolidationRules(QList<CTConsolidationRule> &listConsolidation,
                                                         const QDomElement& setElement,
                                                         const QDomElement &ruleElement,
                                                         const QString &base = QString());

    QList<QDomElement>          childElements(const QDomElement& parent, const QString& tagName);

private:

    ConsolidationTreeData       m_data;
    RuleSources                 m_source;
};

#endif // CONSOLIDATION_TREE_QUERY_ENGINE_P_H
