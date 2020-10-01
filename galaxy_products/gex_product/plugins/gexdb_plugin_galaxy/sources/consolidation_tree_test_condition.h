#ifndef CONSOLIDATION_TREE_TEST_CONDITION_H
#define CONSOLIDATION_TREE_TEST_CONDITION_H

#include <QStringList>

class QTextStream;

class CTTestCondition
{
public:

    CTTestCondition();

    const QString&      conditionName() const;
    const QString&      splitlotField() const;
    const QStringList&  allowedValues() const;

    // Set the name of the current Test Condition
    void                setConditionName(const QString& conditionName);
    // Set the field name used as Test Condition in the splitlot table
    void                setSplitlotField(const QString& splitlotField);
    // Set the list allowed values for the current test condition.
    // Different values have to be separate with "|"
    void                setAllowedValues(const QString& allowedValues);
    // Set the list of allowed values for the current test condition
    void                setAllowedValues(const QStringList& allowedValues);

    // Check whether a Test Condition value is allowed
    // Return true when the given value is allowed, otherwise return false.
    bool                isAllowedValue(const QString& value) const;

    void                dump(QTextStream& stream) const;

protected:

    // Name of the Test Condition.
    QString             m_conditionName;
    // Field name in the splitlot table which is used to store test condition value
    QString             m_splitlotField;
    // List of values allowed for this test condtition
    QStringList         m_allowedValues;
};

#endif // CONSOLIDATION_TREE_TEST_CONDITION_H
