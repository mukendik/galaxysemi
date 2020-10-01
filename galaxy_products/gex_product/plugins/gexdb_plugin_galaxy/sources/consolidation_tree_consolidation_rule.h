#ifndef CONSOLIDATION_TREE_CONSOLIDATION_RULE_H
#define CONSOLIDATION_TREE_CONSOLIDATION_RULE_H

#include <QString>

class QTextStream;

class CTConsolidationRule
{
public:

    CTConsolidationRule();
    CTConsolidationRule(const QString& mode, const QString& baseConsolidation = QString());

    bool                isProductionFlow() const;
    const QString&      mode() const;
    const QString&      baseConsolidation() const;
    const QString&      name() const;
    const QString&      dataType() const;
    const QString&      algorithm() const;
    const QString&      storedResult() const;
    const QString&      groupedBy() const;

    void                setProductionFlow(bool productionFlow);
    void                setName(const QString& name);
    void                setDataType(const QString& dataType);
    void                setAlgorithm(const QString& algorithm);
    void                setStoredResult(const QString& storedResult);
    void                setGroupedBy(const QString& groupedBy);

    void                dump(QTextStream &stream) const;

protected:

    bool        m_productionFlow;
    QString     m_mode;
    QString     m_baseConsolidation;
    QString     m_name;
    QString     m_dataType;
    QString     m_algorithm;
    QString     m_storedResult;
    QString     m_groupedBy;
};

#endif // CONSOLIDATION_TREE_CONSOLIDATION_RULE_H
