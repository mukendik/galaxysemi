#include "consolidation_tree_test_condition.h"

#include "QTextStream"

CTTestCondition::CTTestCondition()
{

}

const QString &CTTestCondition::conditionName() const
{
    return m_conditionName;
}

const QString &CTTestCondition::splitlotField() const
{
    return m_splitlotField;
}

const QStringList &CTTestCondition::allowedValues() const
{
    return m_allowedValues;
}

void CTTestCondition::setConditionName(const QString &conditionName)
{
    m_conditionName = conditionName;
}

void CTTestCondition::setSplitlotField(const QString &splitlotField)
{
    m_splitlotField = splitlotField;
}

void CTTestCondition::setAllowedValues(const QString &allowedValues)
{
    m_allowedValues = allowedValues.split("|");
}

void CTTestCondition::setAllowedValues(const QStringList &allowedValues)
{
    m_allowedValues = allowedValues;
}

bool CTTestCondition::isAllowedValue(const QString &value) const
{
    return m_allowedValues.contains(value, Qt::CaseInsensitive);
}

void CTTestCondition::dump(QTextStream &stream) const
{
    stream << "-- CTTestCondition --" << endl;
    stream << " conditionName     : " << m_conditionName << endl;
    stream << " splitlot_field    : " << m_splitlotField << endl;
    stream << " allowed_values    : " << m_allowedValues.join(" | ") << endl;
    stream << endl;
}
