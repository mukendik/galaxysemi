#ifndef CONSOLIDATION_TREE_VALIDATOR_H
#define CONSOLIDATION_TREE_VALIDATOR_H

#include "consolidation_tree_period.h"
#include "consolidation_tree_replies.h"

#include <QStringList>
#include <QDomDocument>

class CTReplies;
class GexDbPlugin_Galaxy;

class ConsolidationTreeValidator
{
public:

    ConsolidationTreeValidator(GexDbPlugin_Galaxy * pPlugin, const QString &schema);

    // Validate the Consolidation Tree data.
    // Returns true when the Consolidation Tree is correct, other return false.
    // CTReplies object contains warning or error messages.
    bool        validate(const QString &data, CTReplies& replies);

private:

    // Validate the xml schema.
    // Return true when the schema is valid, otherwise false.
    bool        validateSchema(const QString &xmlData);

    bool        validateTree(const QString& xmlData);
    bool        validateProduct(const QDomElement &parent, const QString& path);
    bool        validateDefault(const QDomElement &parent, const QString& path);
    bool        validatePeriod(const QDomElement &parent, const QString &path);
    bool        validateTestConditions(const QDomElement &parent, const QString &path);
    bool        validateSets(const QDomElement &parent, bool defaultRules, const QString &path);
    bool        validateRules(const QDomElement &parent, const QStringList& testConditions, const QString& path);
    bool        validateDefaultRules(const QDomElement &parent, const QString &path);

    QString     buildPath(const QString &parentPath, const QDomElement &element, const QString &attribute = QString());
    void        addReplies(const CTReplies& replies, const QString& nodePath);

    // Holds the xsd schema to validate the consolidation tree
    GexDbPlugin_Galaxy *    m_pPlugin;
    QString                 m_xsdSchema;
    QDomDocument            m_xmlDocument;
    CTReplies               m_replies;

    QString                 m_currentTestingStage;
    QStringList             m_currentProducts;
    QStringList             m_currentTestConditions;
    CTPeriod                m_currentPeriod;
    QStringList             m_currentRules;
    int                     m_currentPhysicalRulesCount;
    int                     m_currentRuleLevel;
};

#endif // CONSOLIDATION_TREE_VALIDATOR_H
