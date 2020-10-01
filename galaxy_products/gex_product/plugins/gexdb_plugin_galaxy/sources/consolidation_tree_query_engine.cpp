#include "consolidation_tree_query_engine.h"
#include "consolidation_tree_query_engine_p.h"
#include "consolidation_tree.h"
#include "consolidation_tree_defines.h"
#include "consolidation_tree_query_filter.h"
#include <gqtl_log.h>

ConsolidationTreeQueryEngine::ConsolidationTreeQueryEngine(const ConsolidationTree& consolidationTree)
    : m_qEnginePrivate(NULL)
{
    if (consolidationTree.isValid())
        m_qEnginePrivate = new ConsolidationTreeQueryEnginePrivate(consolidationTree.data());

    m_mapDefaultOptionValue.insert(CT_XML_OPTION_DATE_CONSOLIDATION, "min_start_time");
}

ConsolidationTreeQueryEngine::~ConsolidationTreeQueryEngine()
{
    if (m_qEnginePrivate)
    {
        delete m_qEnginePrivate;
        m_qEnginePrivate = NULL;
    }
}

bool ConsolidationTreeQueryEngine::findOption(const CTQueryFilter &filter, QString& optionValue)
{
    if (filter.value(CTQueryFilter::FilterTestingStage).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Testing Stage is missing or empty");
        return false;
    }

    if (filter.value(CTQueryFilter::FilterOption).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Option name is missing or empty");
        return false;
    }

    if (m_qEnginePrivate)
    {
        QList<QDomElement>  listElement = m_qEnginePrivate->findOption(filter);

        if (listElement.count() > 1)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: More than one Option found.");
            return false;
        }
        else if (listElement.count() == 0)
        {
            optionValue = m_mapDefaultOptionValue.value((filter.value(CTQueryFilter::FilterOption)));

            return (optionValue.isEmpty() == false);
        }
        else
        {
            optionValue = listElement.first().text();
            return true;
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: no query engine instantiated");
        return false;
    }
}

bool ConsolidationTreeQueryEngine::hasConsolidation(const CTQueryFilter &filter)
{
    if (filter.value(CTQueryFilter::FilterTestingStage).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Testing Stage is missing or empty");
        return false;
    }

    if (filter.value(CTQueryFilter::FilterProductID).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Product ID is missing or empty");
        return false;
    }

    if (filter.value(CTQueryFilter::FilterDate).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Date is missing or empty");
        return false;
    }

    if (m_qEnginePrivate)
        return (m_qEnginePrivate->findConsolidationElement(filter).count() > 0);
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: no query engine instantiated");
        return false;
    }
}

QList<CTTestCondition> ConsolidationTreeQueryEngine::findTestConditions(const CTQueryFilter &filter)
{
    QList<CTTestCondition>  listConditions;

    if (filter.value(CTQueryFilter::FilterTestingStage).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Testing Stage is missing or empty");
        return listConditions;
    }

    if (filter.value(CTQueryFilter::FilterProductID).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Product ID is missing or empty");
        return listConditions;
    }

    if (filter.value(CTQueryFilter::FilterDate).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Date is missing or empty");
        return listConditions;
    }

    if (m_qEnginePrivate)
    {
        QList<QDomElement>      listElement = m_qEnginePrivate->findConditionElement(filter);
        QDomElement             xmlCondition;

        for (int item = 0; item < listElement.count(); ++item)
        {
            xmlCondition = listElement.at(item);

            CTTestCondition testCondition;

            testCondition.setConditionName(xmlCondition.attribute(CT_XML_ATTR_CONDITION_NAME));
            testCondition.setSplitlotField(xmlCondition.attribute(CT_XML_ATTR_SPLITLOT_FIELD));
            testCondition.setAllowedValues(xmlCondition.attribute(CT_XML_ATTR_ALLOWED_VALUES));

            listConditions.append(testCondition);
        }
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: no query engine instantiated");

    return listConditions;
}

QList<CTConsolidationRule> ConsolidationTreeQueryEngine::findConsolidationRules(const CTQueryFilter &filter)
{
    QList<CTConsolidationRule>  listConsolidation;

    if (filter.value(CTQueryFilter::FilterTestingStage).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Testing Stage is missing or empty");
        return listConsolidation;
    }

    if (filter.value(CTQueryFilter::FilterProductID).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Product ID is missing or empty");
        return listConsolidation;
    }

    if (filter.value(CTQueryFilter::FilterDate).isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: Filter on Date is missing or empty");
        return listConsolidation;
    }

    if (m_qEnginePrivate)
    {
        bool                validRule           = true;
        bool                withTestConditions  = (m_qEnginePrivate->findConditionElement(filter).count() > 0);
        QList<QDomElement>  listElement         = m_qEnginePrivate->findSetElement(filter);

        for (int item = 0; item < listElement.count(); ++item)
        {
            QDomElement         xmlSet      = listElement.at(item);
            QList<QDomElement>  xmlRules    = m_qEnginePrivate->childElements(xmlSet, CT_XML_NODE_RULE);

            for (int rule = 0; rule < xmlRules.count(); ++rule)
            {
                if (withTestConditions)
                    validRule = (xmlRules.at(rule).attribute(CT_XML_ATTR_GROUP_BY).compare("none", Qt::CaseInsensitive));
                else
                    validRule = (xmlRules.at(rule).attribute(CT_XML_ATTR_GROUP_BY).compare("none", Qt::CaseInsensitive) == false);

                if (validRule)
                    m_qEnginePrivate->createConsolidationRules(listConsolidation, xmlSet, xmlRules.at(rule));
            }
        }
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: no query engine instantiated");

    return listConsolidation;
}

QList<CTPeriod> ConsolidationTreeQueryEngine::findConsolidationPeriod(const CTQueryFilter &filter)
{
    QList<CTPeriod>  listPeriods;

    if (m_qEnginePrivate)
    {
        QList<QDomElement>      listElement = m_qEnginePrivate->findConsolidationElement(filter);

        foreach(const QDomElement& xmlConsolidation, listElement)
        {
            CTPeriod    period;

            period.setBeginDate(QDate::fromString(xmlConsolidation.attribute(CT_XML_ATTR_BEGIN_DATE), Qt::ISODate));
            period.setEndDate(QDate::fromString(xmlConsolidation.attribute(CT_XML_ATTR_END_DATE), Qt::ISODate));

            listPeriods.append(period);
        }
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, "Consolidation Tree Query Engine: no query engine instantiated");

    return listPeriods;
}




