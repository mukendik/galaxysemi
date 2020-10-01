#include "consolidation_tree_query_engine_p.h"
#include "consolidation_tree_query_filter.h"
#include "consolidation_tree_defines.h"
#include "consolidation_tree_period.h"
#include "consolidation_tree_data.h"

#include <QDate>
#include <QStringList>

ConsolidationTreeQueryEnginePrivate::ConsolidationTreeQueryEnginePrivate(const ConsolidationTreeData &data)
    : m_data(data), m_source(FromBoth)
{
}

ConsolidationTreeQueryEnginePrivate::~ConsolidationTreeQueryEnginePrivate()
{

}

void ConsolidationTreeQueryEnginePrivate::setRuleSources(RuleSources source)
{
    m_source = source;
}

QList<QDomElement> ConsolidationTreeQueryEnginePrivate::findOption(const CTQueryFilter &filter)
{
    QList<QDomElement>  parentElements = findTestingStageElement(filter);
    QList<QDomElement>  listElement;

    for (int item = 0; item < parentElements.count(); ++item)
    {
        QDomElement     parent   = parentElements.at(item);
        QDomElement     options  = parent.firstChildElement(CT_XML_NODE_OPTIONS);

        if (options.isNull() == false)
            listElement = childElements(options, filter.value(CTQueryFilter::FilterOption));
    }

    return listElement;
}

QList<QDomElement> ConsolidationTreeQueryEnginePrivate::findTestingStageElement(const CTQueryFilter &filter)
{
    QList<QDomElement>  listElement;

    if (m_data.isValid())
    {
        QStringList testingStages;

        if (filter.value(CTQueryFilter::FilterTestingStage).isEmpty())
            testingStages << CT_XML_NODE_WAFER_SORT << CT_XML_NODE_FINAL_TEST;
        else
        {
            if (filter.value(CTQueryFilter::FilterTestingStage) == "Wafer Sort")
                testingStages << CT_XML_NODE_WAFER_SORT;
            else if (filter.value(CTQueryFilter::FilterTestingStage) == "Final Test")
                testingStages << CT_XML_NODE_FINAL_TEST;
        }

        foreach(const QString& testingStage, testingStages)
        {
            QList<QDomElement> stageElements   = childElements(m_data.domDocument().documentElement(), testingStage);

            for (int item = 0; item < stageElements.count(); ++item)
            {
                QDomElement stage = stageElements.at(item);

                listElement.append(stage);
            }
        }
    }

    return listElement;
}

QList<QDomElement> ConsolidationTreeQueryEnginePrivate::findGroupElement(const CTQueryFilter &filter)
{
    QList<QDomElement>  listElement;
    QList<QDomElement>  parentElements  = findTestingStageElement(filter);
    QString             filterValue     = filter.value(CTQueryFilter::FilterProductID);
    QStringList         products;
    QDomElement         parent;
    QDomElement         groupElt;
    QList<QDomElement>  groupElements;

    for (int item = 0; item < parentElements.count(); ++item)
    {
        parent          = parentElements.at(item);
        groupElements   = childElements(parent, CT_XML_NODE_GROUP);

        for (int group = 0; group < groupElements.count(); ++group)
        {
            groupElt    = groupElements.at(group);
            products    = groupElt.attribute(CT_XML_ATTR_PRODUCTS_ID).split("|");

            if (filterValue.isEmpty() == true || products.contains(filterValue))
                listElement.append(groupElt);
        }
    }

    return listElement;
}

QList<QDomElement> ConsolidationTreeQueryEnginePrivate::findDefaultElement(const CTQueryFilter &filter)
{
    QList<QDomElement>  listElement;
    QList<QDomElement>  parentElements  = findTestingStageElement(filter);
    QDomElement         parent;
    QList<QDomElement>  defaultElements;

    for (int item = 0; item < parentElements.count(); ++item)
    {
        parent          = parentElements.at(item);
        defaultElements = childElements(parent, CT_XML_NODE_DEFAULT);

        for (int elt = 0; elt < defaultElements.count(); ++elt)
            listElement.append(defaultElements.at(elt));
    }

    return listElement;
}

QList<QDomElement> ConsolidationTreeQueryEnginePrivate::findConditionElement(const CTQueryFilter &filter)
{
    QList<QDomElement>  parentElements = findConsolidationElement(filter);
    QList<QDomElement>  listElement;

    for (int item = 0; item < parentElements.count(); ++item)
    {
        QDomElement     parent          = parentElements.at(item);
        QDomElement     testConditions  = parent.firstChildElement(CT_XML_NODE_TESTCONDITIONS);

        if (testConditions.isNull() == false)
        {
            QList<QDomElement>  tcElements  = childElements(testConditions, CT_XML_NODE_CONDITION);

            for (int tc = 0; tc < tcElements.count(); ++tc)
                listElement.append(tcElements.at(tc));
        }
    }

    return listElement;
}

QList<QDomElement> ConsolidationTreeQueryEnginePrivate::findSetElement(const CTQueryFilter &filter)
{
    QList<QDomElement>  listElement;
    QList<QDomElement>  parentElements;
    QString             beginConsolidation;
    QString             endConsolidation;
    CTPeriod            setPeriod;

    QDate               filterDate;
    CTPeriod            filterPeriod;
    QString             filterValue = filter.value(CTQueryFilter::FilterDate);

    if (filterValue.contains("|"))
    {
        filterPeriod.setBeginDate(QDate::fromString(filterValue.section("|", 0, 0), Qt::ISODate));
        filterPeriod.setEndDate(QDate::fromString(filterValue.section("|", 1, 1), Qt::ISODate));
    }
    else
        filterDate = QDate::fromString(filterValue, Qt::ISODate);


    if (m_source & FromGroup)
    {
        parentElements = findConsolidationElement(filter);

        for (int item = 0; item < parentElements.count(); ++item)
        {
            QDomElement         set;
            QDomElement         parent      = parentElements.at(item);
            QList<QDomElement>  setElements = childElements(parent, CT_XML_NODE_SET);

            beginConsolidation  = parent.attribute(CT_XML_ATTR_BEGIN_DATE);
            endConsolidation    = parent.attribute(CT_XML_ATTR_END_DATE);

            for (int elt = 0; elt < setElements.count(); ++elt)
            {
                set = setElements.at(elt);

                setPeriod.setBeginDate(QDate::fromString(set.attribute(CT_XML_ATTR_BEGIN_DATE, beginConsolidation), Qt::ISODate));
                setPeriod.setEndDate(QDate::fromString(set.attribute(CT_XML_ATTR_END_DATE, endConsolidation), Qt::ISODate));

                // Filter date is valid, so check it matches with the period
                if (filterDate.isNull() == false)
                {
                    // Filter date matches with the period defined for this set.
                    if (setPeriod.contains(filterDate))
                        listElement.append(set);
                }
                else if (setPeriod.intersects(filterPeriod))
                {
                    // No date define in the filter
                    // if set period matches with the filter period
                    listElement.append(set);
                }
            }
        }
    }

    if (m_source & FromDefault)
    {
        if (listElement.count() == 0)
        {
            parentElements = findDefaultElement(filter);

            for (int item = 0; item < parentElements.count(); ++item)
            {
                QDomElement         set;
                QDomElement         parent      = parentElements.at(item);
                QList<QDomElement>  setElements = childElements(parent, CT_XML_NODE_SET);

                for (int elt = 0; elt < setElements.count(); ++elt)
                {
                    set = setElements.at(elt);

                    setPeriod.setBeginDate(QDate::fromString(set.attribute(CT_XML_ATTR_BEGIN_DATE, beginConsolidation), Qt::ISODate));
                    setPeriod.setEndDate(QDate::fromString(set.attribute(CT_XML_ATTR_END_DATE, endConsolidation), Qt::ISODate));

                    // Filter date is valid, so check it matches with the period
                    if (filterDate.isNull() == false)
                    {
                        // Filter date matches with the period defined for this set.
                        if (setPeriod.contains(filterDate))
                            listElement.append(set);
                    }
                    else if (filterPeriod.intersects(setPeriod))
                    {
                        // No date define in the filter, so every set are added to the list
                        listElement.append(set);
                    }
                }
            }
        }
    }

    return listElement;
}

QList<QDomElement> ConsolidationTreeQueryEnginePrivate::findRuleElement(const CTQueryFilter &filter)
{
    QList<QDomElement>  parentElements = findSetElement(filter);
    QList<QDomElement>  listElement;

    for (int item = 0; item < parentElements.count(); ++item)
    {
        QDomElement     parent      = parentElements.at(item);
        QDomNodeList    ruleNodes   = parent.elementsByTagName(CT_XML_NODE_RULE);

        for (int idx = 0; idx < ruleNodes.count(); ++idx)
            listElement.append(ruleNodes.at(idx).toElement());
    }

    return listElement;
}

QList<QDomElement> ConsolidationTreeQueryEnginePrivate::findConsolidationElement(const CTQueryFilter &filter)
{
    QList<QDomElement>  listElement;
    QList<QDomElement>  parentElements = findGroupElement(filter);
    CTPeriod            consolidationPeriod;

    QDate               filterDate;
    CTPeriod            filterPeriod;
    QString             filterValue = filter.value(CTQueryFilter::FilterDate);

    if (filterValue.contains("|"))
    {
        filterPeriod.setBeginDate(QDate::fromString(filterValue.section("|", 0, 0), Qt::ISODate));
        filterPeriod.setEndDate(QDate::fromString(filterValue.section("|", 1, 1), Qt::ISODate));
    }
    else
        filterDate = QDate::fromString(filterValue, Qt::ISODate);

    for (int item = 0; item < parentElements.count(); ++item)
    {
        QDomElement         consolidation;
        QDomElement         parent                  = parentElements.at(item);
        QList<QDomElement>  consolidationElements   = childElements(parent, CT_XML_NODE_CONSOLIDATION);

        for (int element = 0; element < consolidationElements.count(); ++element)
        {
            consolidation    = consolidationElements.at(element);

            consolidationPeriod.setBeginDate(QDate::fromString(consolidation.attribute(CT_XML_ATTR_BEGIN_DATE), Qt::ISODate));
            consolidationPeriod.setEndDate(QDate::fromString(consolidation.attribute(CT_XML_ATTR_END_DATE), Qt::ISODate));

            // Filter date is valid, so check it matches with the period
            if (filterDate.isNull() == false)
            {
                // Filter date matches with the period defined for this consolidation.
                if (consolidationPeriod.contains(filterDate))
                    listElement.append(consolidation);
            }
            else if (consolidationPeriod.intersects(filterPeriod))
            {
                // No date define in the filter, so every set are added to the list
                listElement.append(consolidation);
            }
        }
    }

    return listElement;
}

bool ConsolidationTreeQueryEnginePrivate::createConsolidationRules(QList<CTConsolidationRule>& listConsolidation,
                                                            const QDomElement &setElement,
                                                            const QDomElement &ruleElement,
                                                            const QString &base)
{
    QList<CTConsolidationRule>  listRules;
    bool                        productionFlow  = false;
    QString                     mode            = setElement.attribute(CT_XML_ATTR_MODE);
    CTConsolidationRule         consolidationRule(mode, base);

    consolidationRule.setName(ruleElement.attribute(CT_XML_ATTR_NAME));
    consolidationRule.setAlgorithm(ruleElement.attribute(CT_XML_ATTR_ALGO));
    consolidationRule.setDataType(ruleElement.attribute(CT_XML_ATTR_DATA_TYPE));
    consolidationRule.setGroupedBy(ruleElement.attribute(CT_XML_ATTR_GROUP_BY));
    consolidationRule.setStoredResult(ruleElement.attribute(CT_XML_ATTR_STORED_RESULTS));

    // Create consolidation rules for child rule if exists.
    QList<QDomElement> xmlRules = childElements(ruleElement, CT_XML_NODE_RULE);

    for (int rule = 0; rule < xmlRules.count(); ++rule)
    {
        productionFlow |= createConsolidationRules(listRules, setElement, xmlRules.at(rule), consolidationRule.name());
    }

    consolidationRule.setProductionFlow(productionFlow);

    listConsolidation.append(consolidationRule);
    listConsolidation.append(listRules);

    return consolidationRule.isProductionFlow();
}

QList<QDomElement> ConsolidationTreeQueryEnginePrivate::childElements(const QDomElement &parent, const QString &tagName)
{
    QDomElement         child = parent.firstChildElement(tagName);
    QList<QDomElement>  childElements;

    while (child.isNull() == false)
    {
        childElements.append(child);

        child = child.nextSiblingElement(tagName);
    }

    return childElements;
}
