#include "consolidation_tree_consolidation_rule.h"
#include "consolidation_tree_defines.h"

#include <QTextStream>

CTConsolidationRule::CTConsolidationRule()
{
    m_productionFlow    = false;
}

CTConsolidationRule::CTConsolidationRule(const QString &mode, const QString &baseConsolidation)
{
    m_productionFlow        = false;
    m_mode                  = mode;
    m_baseConsolidation     = baseConsolidation;
}

bool CTConsolidationRule::isProductionFlow() const
{
    return (m_productionFlow || m_dataType.toLower() == CT_XML_VALUE_DATA_TYPE_PHYSICAL);
}

const QString &CTConsolidationRule::mode() const
{
    return m_mode;
}

const QString &CTConsolidationRule::baseConsolidation() const
{
    return m_baseConsolidation;
}

const QString &CTConsolidationRule::name() const
{
    return m_name;
}

const QString &CTConsolidationRule::dataType() const
{
    return m_dataType;
}

const QString &CTConsolidationRule::algorithm() const
{
    return m_algorithm;
}

const QString &CTConsolidationRule::storedResult() const
{
    return m_storedResult;
}

const QString &CTConsolidationRule::groupedBy() const
{
    return m_groupedBy;
}

void CTConsolidationRule::setProductionFlow(bool productionFlow)
{
    m_productionFlow = productionFlow;
}

void CTConsolidationRule::setName(const QString &name)
{
    m_name = name;
}

void CTConsolidationRule::setDataType(const QString &dataType)
{
    m_dataType = dataType;
}

void CTConsolidationRule::setAlgorithm(const QString &algorithm)
{
    m_algorithm = algorithm;
}

void CTConsolidationRule::setStoredResult(const QString &storedResult)
{
    m_storedResult = storedResult;
}

void CTConsolidationRule::setGroupedBy(const QString &groupedBy)
{
    m_groupedBy = groupedBy;
}

void CTConsolidationRule::dump(QTextStream &stream) const
{
    stream << "-- CTConsolidationRule --" << endl;
    stream << " mode              : " << m_mode << endl;
    stream << " name              : " << m_name << endl;
    stream << " base              : " << m_baseConsolidation << endl;
    stream << " algo              : " << m_algorithm << endl;
    stream << " data_type         : " << m_dataType << endl;
    stream << " grouped_by        : " << m_groupedBy << endl;
    stream << " stored_results    : " << m_storedResult << endl;
    stream << " production_flow   : " << (isProductionFlow() ? "Yes" : "No") << endl;

    stream << endl;
}


