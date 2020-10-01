#include "consolidation_tree_validator.h"
#include "consolidation_tree_replies.h"
#include "consolidation_tree_defines.h"
#include "gexdb_plugin_galaxy.h"
#include <gqtl_log.h>

#include <QSqlRecord>
#include <QFile>
#include <QtXmlPatterns>
#include <QDomDocument>

class CTMessageHandler : public QAbstractMessageHandler
{
    public:
        CTMessageHandler()
            : QAbstractMessageHandler(0)
        {
        }

        QString statusMessage() const
        {
            return m_description;
        }

        int line() const
        {
            return m_sourceLocation.line();
        }

        int column() const
        {
            return m_sourceLocation.column();
        }

    protected:
        virtual void handleMessage(QtMsgType type, const QString &description,
                                   const QUrl &identifier, const QSourceLocation &sourceLocation)
        {
            Q_UNUSED(type);
            Q_UNUSED(identifier);

            m_messageType = type;
            m_description = description;
            m_sourceLocation = sourceLocation;
        }

    private:
        QtMsgType m_messageType;
        QString m_description;
        QSourceLocation m_sourceLocation;
};

ConsolidationTreeValidator::ConsolidationTreeValidator(GexDbPlugin_Galaxy * pPlugin, const QString &schema)
    : m_pPlugin(pPlugin), m_xsdSchema(schema), m_currentRuleLevel(0)
{
    m_currentPhysicalRulesCount = 0;
}

bool ConsolidationTreeValidator::validate(const QString & xmlData, CTReplies &replies)
{
    bool    success = false;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Consolidation Tree Validator [validate]: starting");

    if (xmlData.isEmpty() == false)
    {
        if (validateSchema(xmlData))
        {
            if (validateTree(xmlData))
                success = true;
        }

        replies = m_replies;
    }
    else
    {
        QString message = "Consolidation Tree Validator [validate]: xml content is empty";

        GSLOG(SYSLOG_SEV_WARNING, message.toLatin1().data());
        replies.appendReply(CTReply(CTReply::ReplyError, message));
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Consolidation Tree Validator [validate]: %1")
          .arg( (success) ? "success" : "failed").toLatin1().constData());

    return success;
}

bool ConsolidationTreeValidator::validateSchema(const QString &xmlData)
{
    bool    success = false;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Consolidation Tree Validator [schema validation]: starting");

    CTMessageHandler    messageHandler;
    QXmlSchema          xmlSchema;

    xmlSchema.setMessageHandler(&messageHandler);

    xmlSchema.load(QUrl::fromLocalFile(m_xsdSchema));

    // Check whether the schema is valid
    if (xmlSchema.isValid())
    {
        QXmlSchemaValidator validator(xmlSchema);

        if (validator.validate(xmlData.toLatin1()))
            success = true;
        else
        {
            QString message = messageHandler.statusMessage();

            GSLOG(SYSLOG_SEV_ERROR, QString("Consolidation Tree Validator [schema validation]: %1")
                  .arg( message).toLatin1().constData());

            CTReply replyError;

            replyError.setType(CTReply::ReplyError);
            replyError.setMessage(message);
            replyError.setLine(messageHandler.line());
            replyError.setColumn(messageHandler.column());

            m_replies.appendReply(replyError);
        }
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "Consolidation Tree Validator [schema validation]: XSD schema is not valid");

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Consolidation Tree Validator [schema validation]: %1").arg(
             (success) ? "success" : "failed").toLatin1().constData());

    return success;
}

bool ConsolidationTreeValidator::validateTree(const QString& xmlData)
{
    QString         message;
    QString         path;
    bool            valid = true;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Consolidation Tree Validator [tree validation]: starting");

    if (m_xmlDocument.setContent(xmlData, &message))
    {
        if (m_xmlDocument.documentElement().tagName() == CT_XML_NODE_ROOT)
        {
            QDomElement child = m_xmlDocument.documentElement().firstChildElement();

            while (child.isNull() == false)
            {
                m_currentTestingStage = child.tagName();

                path = buildPath("", child);

                valid &= validateDefault(child, path);
                valid &= validateProduct(child, path);

                child = child.nextSiblingElement();
            }
        }
        else
        {
            QString message = QString("Consolidation Tree Validator [tree validation]: Root node %1 not found")
                                        .arg(CT_XML_NODE_ROOT);

            GSLOG(SYSLOG_SEV_WARNING, message.toLatin1().data());
            m_replies.appendReply(CTReply(CTReply::ReplyError, message));
        }
    }
    else
    {
        valid = false;
        GSLOG(SYSLOG_SEV_WARNING, "Consolidation Tree Validator [tree validation]: content is not XML compliant");
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Consolidation Tree Validator [tree validation]: %1").arg(
             (valid) ? "success" : "failed").toLatin1().constData());

    return valid;
}

bool ConsolidationTreeValidator::validateProduct(const QDomElement& parent, const QString &path)
{
    bool            valid = true;
    QDomElement     child = parent.firstChildElement(CT_XML_NODE_GROUP);
    QString         message;
    QString         nodePath;
    CTReplies       replies;
    QSet<QString>   mapProducts;

    while (child.isNull() == false)
    {
        replies.clear();
        nodePath = buildPath(path, child);

        m_currentProducts = child.attribute(CT_XML_ATTR_PRODUCTS_ID).toLower().split("|");
        m_currentTestConditions.clear();

        foreach(const QString& product, m_currentProducts)
        {
            if (mapProducts.contains(product) == false)
                mapProducts.insert(product);
            else
            {
                valid   = false;
                message = QString("Duplicated product %1").arg(product);

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }
        }

        // add local replies to the global replies list
        addReplies(replies, nodePath);

        valid &= validatePeriod(child, nodePath);

        child = child.nextSiblingElement(CT_XML_NODE_GROUP);
    }

    return valid;
}

bool ConsolidationTreeValidator::validateDefault(const QDomElement &parent, const QString& path)
{
    bool            valid = true;
    QDomElement     child = parent.firstChildElement(CT_XML_NODE_DEFAULT);
    QString         nodePath;

    m_currentPeriod = CTPeriod();
    m_currentProducts.clear();
    m_currentTestConditions.clear();

    if (child.isNull() == false)
    {
        nodePath = buildPath(path, child);

        valid &= validateSets(child, true, nodePath);
    }

    return valid;
}

bool ConsolidationTreeValidator::validatePeriod(const QDomElement &parent, const QString& path)
{
    bool            valid = true;
    QDomElement     child = parent.firstChildElement(CT_XML_NODE_CONSOLIDATION);
    QString         nodePath;
    CTReplies       replies;
    QList<CTPeriod> listPeriod;

    while (child.isNull() == false)
    {
        replies.clear();

        nodePath = buildPath(path, child);

        QDate           begin = QDate::fromString(child.attribute(CT_XML_ATTR_BEGIN_DATE, ""), Qt::ISODate);
        QDate           end   = QDate::fromString(child.attribute(CT_XML_ATTR_END_DATE, ""), Qt::ISODate);

        m_currentPeriod.setBeginDate(begin);
        m_currentPeriod.setEndDate(end);

        if (m_currentPeriod.isValid() == false)
        {
            valid   = false;
            replies.appendReply(CTReply(CTReply::ReplyError, "Begin date is higher than end date in consolidation node"));
        }

        // If consolidation dates are not valid, don't check child elements
        if (replies.count() == 0)
        {
            listPeriod.append(m_currentPeriod);

            valid &= validateTestConditions(child, nodePath);

            valid &= validateSets(child, false, nodePath);
        }

        // add local replies to the global replies list
        addReplies(replies, nodePath);

        child = child.nextSiblingElement(CT_XML_NODE_CONSOLIDATION);
    }

    if (listPeriod.count() > 1)
    {
        // clear replies list
        replies.clear();

        // Sort period by chronological order
        qSort(listPeriod);

        CTPeriod  refPeriod = listPeriod.at(0);
        CTPeriod  otherPeriod;

        // Check there is no overlap between periods
        for (int idx = 1; idx < listPeriod.count(); idx++)
        {
            otherPeriod = listPeriod.at(idx);

            if (refPeriod.contains(otherPeriod.beginDate()) ||
                    refPeriod.contains(otherPeriod.endDate()))
            {
                valid   = false;
                replies.appendReply(CTReply(CTReply::ReplyError, "Overlapping period in consolidation nodes"));
            }

            refPeriod = otherPeriod;
        }

        addReplies(replies, path);
    }

    return valid;
}

bool ConsolidationTreeValidator::validateTestConditions(const QDomElement &parent, const QString& path)
{
    bool            valid = true;
    QString         message;
    QDomElement     testCondition = parent.firstChildElement(CT_XML_NODE_TESTCONDITIONS);
    QDomElement     child;
    QString         testConditionNodePath;
    QString         nodePath;
    CTReplies       replies;
    QString         conditionName;
    QString         splitlot_field;
    QStringList     allowedValues;

    if (testCondition.isNull() || m_currentProducts.count() == 0)
        return true;

    // Build path for 'test_conditions' node
    testConditionNodePath = buildPath(path ,testCondition);

    // Initialize SQL Query
    QString             query;
    GexDbPlugin_Query	clQuery(m_pPlugin,
                                QSqlDatabase::database(m_pPlugin->m_pclDatabaseConnector->m_strConnectionName));

    child = testCondition.firstChildElement(CT_XML_NODE_CONDITION);

    m_currentTestConditions.clear();

    while (child.isNull() == false)
    {
        replies.clear();

        nodePath        = buildPath(testConditionNodePath, testCondition);
        conditionName   = child.attribute(CT_XML_ATTR_CONDITION_NAME, "").toLower();
        splitlot_field  = child.attribute(CT_XML_ATTR_SPLITLOT_FIELD, "").toLower();
        allowedValues   = child.attribute(CT_XML_ATTR_ALLOWED_VALUES, "").toLower().split("|");

        if (m_currentTestConditions.indexOf(conditionName) != -1)
        {
            valid   = false;
            message = QString("Duplicated condition %1").arg(conditionName);

            replies.appendReply(CTReply(CTReply::ReplyError, message));
        }
        else
            m_currentTestConditions.append(conditionName);

        // Check splitlot field
        query = QString("select %1 from wt_splitlot").arg(splitlot_field);

        if (clQuery.Execute(query) == false)
        {
            message = QString("Invalid splitlot field '%1' in test condition definition").arg(splitlot_field);

            replies.appendReply(CTReply(CTReply::ReplyError, message));

            addReplies(replies, nodePath);

            return false;
        }

        // TD-78: check how to se the query system to avoid hard-coding LotID and ProductID

        // Check allowed values
        query = "select distinct(TSL." + splitlot_field + ")\n";
        query += "from wt_splitlot TSL \n";
        query += "left outer join wt_lot TL on TSL.lot_id = TL.lot_id \n";
        query += "where \n";
        query += "( \n";
        query += "TSL.prod_data = 'Y' \n";

        for(int idx = 0; idx < m_currentProducts.count(); idx++)
        {
            // GCORE-1200: Checked [SC]
            if (idx == 0)
                query += "and TL.product_name = \'" + m_currentProducts.at(idx) + "\' \n";
            else
                query += "or TL.product_name = \'" + m_currentProducts.at(idx) + "\' \n";
        }

        query += ") \n";

        if (!m_currentPeriod.beginDate().isNull())
        {
            QDateTime dt = QDateTime(m_currentPeriod.beginDate());
            dt.setTime(QTime());

            query += "and TSL.start_t >= " + QString::number(dt.toTime_t()) + " \n";
        }

        if (!m_currentPeriod.endDate().isNull())
        {
            QDateTime dt = QDateTime(m_currentPeriod.endDate());
            dt.setTime(QTime(23, 59, 59, 999));

            query += "and TSL.start_t <= " + QString::number(dt.toTime_t()) + " \n";
        }

        if (clQuery.Execute(query) == false)
        {
            message = QString("SQL error when validating conditions");
            replies.appendReply(CTReply(CTReply::ReplyError, message));

            addReplies(replies, nodePath);

            return false;
        }

        QSqlRecord  sqlRecord;
        int         index;
        while (clQuery.Next())
        {
            sqlRecord = clQuery.record();

            index = sqlRecord.indexOf(splitlot_field);

            if (index >= 0)
            {
                if (allowedValues.contains(sqlRecord.value(index).toString().toLower()) == false)
                {
                    valid   = false;

                    message = "Missing allowed value " + sqlRecord.value(index).toString() + " in test condition definition ";

                    replies.appendReply(CTReply(CTReply::ReplyError, message));
                }
            }
        }

        addReplies(replies, nodePath);

        child = child.nextSiblingElement(CT_XML_NODE_CONDITION);
    }

    return valid;
}

bool ConsolidationTreeValidator::validateSets(const QDomElement &parent, bool defaultRules, const QString& path)
{
    bool            valid = true;
    QString         message;
    QString         mode;
    QDate           begin;
    QDate           end;
    QDomElement     child = parent.firstChildElement(CT_XML_NODE_SET);
    QString         nodePath;
    CTReplies       replies;
    QList<CTPeriod> listPeriod;

    while (child.isNull() == false)
    {
        replies.clear();

        nodePath = buildPath(path, child);

        mode    = child.attribute(CT_XML_ATTR_MODE).toLower();
        begin   = QDate::fromString(child.attribute(CT_XML_ATTR_BEGIN_DATE, ""), Qt::ISODate);
        end     = QDate::fromString(child.attribute(CT_XML_ATTR_END_DATE, ""), Qt::ISODate);

        CTPeriod qPeriod(begin, end);

        if (qPeriod.isValid() == false)
        {
            valid   = false;
            message = QString("Begin date is higher than end date in consolidation set node");

            replies.appendReply(CTReply(CTReply::ReplyError, message));
        }

        if (qPeriod.beginDate().isNull() == false && m_currentPeriod.beginDate().isNull() == false &&
                qPeriod.beginDate() < m_currentPeriod.beginDate())
        {
            valid   = false;
            message = QString("Begin date for the set is outside the consolidation period");

            replies.appendReply(CTReply(CTReply::ReplyError, message));
        }

        if (qPeriod.endDate().isNull() == false && m_currentPeriod.endDate().isNull() == false &&
                qPeriod.endDate() > m_currentPeriod.endDate())
        {
            valid   = false;
            message = QString("End date for the set is outside the consolidation period");

            replies.appendReply(CTReply(CTReply::ReplyError, message));
        }

        if (replies.count() == 0)
        {
            listPeriod.append(qPeriod);

            // Only stacked mode is allowed at Wafer Sort
            if (m_currentTestingStage == CT_XML_NODE_WAFER_SORT && mode != CT_XML_VALUE_MODE_STACKED)
            {
                valid   = false;
                message = QString("Consolidation mode 'stacked' is not allowed at Wafer Sort level");

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }

            // Start validation rules for the current set
            m_currentRules.clear();
            m_currentPhysicalRulesCount = 0;

            if (defaultRules)
                validateDefaultRules(child, nodePath);
            else
                validateRules(child, m_currentTestConditions, nodePath);

            // Check that set has one rule that will create a physical consolidated results
            if (defaultRules == false)
            {
                if (m_currentPhysicalRulesCount == 0)
                {
                    valid = false;
                    message = "Missing rule for physical consolidation";

                    replies.appendReply(CTReply(CTReply::ReplyError, message));
                }
            }

            addReplies(replies, nodePath);
        }
        else
            addReplies(replies, nodePath);


        child = child.nextSiblingElement(CT_XML_NODE_SET);
    }

    if (listPeriod.count() > 1)
    {
        // clear replies list
        replies.clear();

        // Sort period by chronological order
        qSort(listPeriod);

        CTPeriod  refPeriod = listPeriod.at(0);
        CTPeriod  otherPeriod;

        // Check there is no overlap between periods
        for (int idx = 1; idx < listPeriod.count(); idx++)
        {
            otherPeriod = listPeriod.at(idx);

            if (refPeriod.contains(otherPeriod.beginDate()) ||
                    refPeriod.contains(otherPeriod.endDate()))
            {
                valid   = false;
                message = QString("Overlapping period in consolidation set nodes");

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }

            refPeriod = otherPeriod;
        }

        addReplies(replies, path);
    }

    return valid;
}

bool ConsolidationTreeValidator::validateRules(const QDomElement &parent, const QStringList &testConditions, const QString &path)
{
    bool            valid = true;
    QString         message;
    QString         groupedBy;
    QString         dataType;
    QString         name;
    QStringList     remainingTestConditions;
    QDomElement     child = parent.firstChildElement(CT_XML_NODE_RULE);
    QString         nodePath;
    CTReplies       replies;

    m_currentRuleLevel++;

    while (child.isNull() == false)
    {
        replies.clear();

        nodePath    = buildPath(path, child, CT_XML_ATTR_NAME);
        groupedBy   = child.attribute(CT_XML_ATTR_GROUP_BY, "").toLower();
        dataType    = child.attribute(CT_XML_ATTR_DATA_TYPE, "").toLower();
        name        = child.attribute(CT_XML_ATTR_NAME, "").toLower();

        if (m_currentRules.indexOf(name) != -1)
        {
            valid = false;
            message = "Duplicated rule " + name;

            replies.appendReply(CTReply(CTReply::ReplyError, message));
        }
        else
            m_currentRules.append(name);

        if (groupedBy == CT_XML_VALUE_GROUP_BY_NONE)
        {
            if (m_currentTestConditions.count() > 0)
            {
                valid = false;
                message = "'none' not allowed for group_by attribute if test conditions defined";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }

            if (child.hasChildNodes())
            {
                valid = false;
                message = "Children not allowed for rules having group_by attribute set to 'none'";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }
        }
        else if (m_currentTestConditions.count() == 0)
        {
            valid = false;
            message = "group_by attribute must be set to 'none' if no test conditions defined";

            replies.appendReply(CTReply(CTReply::ReplyError, message));
        }

        if (groupedBy == CT_XML_VALUE_GROUP_BY_NULL)
        {
            if (child.hasChildNodes())
            {
                valid = false;
                message = "Children not allowed for rules having group_by attribute set to 'null'";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }
        }

        if (groupedBy == CT_XML_VALUE_GROUP_BY_ALL)
        {
            if (m_currentRuleLevel > 1)
            {
                valid = false;
                message = "group_by attribute can be set to 'all' only for top level rules";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }

            remainingTestConditions = m_currentTestConditions;
        }

        if (dataType == CT_XML_VALUE_DATA_TYPE_PHYSICAL)
        {
            m_currentPhysicalRulesCount++;

            if (groupedBy != CT_XML_VALUE_GROUP_BY_NONE && groupedBy != CT_XML_VALUE_GROUP_BY_NULL)
            {
                valid = false;
                message = "Rules for physical consolidation must have the group_by attribute set to 'none' or 'null'";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }

            if (child.hasChildNodes())
            {
                valid = false;
                message = "Rules for physical consolidation must be leaf nodes";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }

            if (m_currentPhysicalRulesCount > 1)
            {
                valid = false;
                message = "Only one rule for physical consolidation allowed per set";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }
        }

        if (groupedBy != CT_XML_VALUE_GROUP_BY_NONE &&
                groupedBy != CT_XML_VALUE_GROUP_BY_NULL &&
                groupedBy != CT_XML_VALUE_GROUP_BY_ALL)
        {
            QStringList tcList = groupedBy.split("|");

            foreach(const QString& tc, tcList)
            {
                if (m_currentTestConditions.contains(tc) == false)
                {
                    valid = false;
                    message = "Test condition '" + tc + "' used in the group_by attribute is not defined";

                    replies.appendReply(CTReply(CTReply::ReplyError, message));
                }

                if (testConditions.contains(tc) == false)
                {
                    valid = false;
                    message = "Test condition used in the group_by attribute must be included in it's parent group_by attribute";

                    replies.appendReply(CTReply(CTReply::ReplyError, message));
                }

                remainingTestConditions.append(tc);
            }

            if (remainingTestConditions.count() == testConditions.count() && m_currentRuleLevel > 1)
            {
                valid = false;
                message = "Test condition used in the group_by attribute must be a subset of it's parent group_by attribute";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }
        }

        addReplies(replies, nodePath);

        valid &= validateRules(child, remainingTestConditions, nodePath);

        child = child.nextSiblingElement(CT_XML_NODE_RULE);
    }

    m_currentRuleLevel--;

    return valid;
}

bool ConsolidationTreeValidator::validateDefaultRules(const QDomElement &parent, const QString& path)
{
    bool            valid = true;
    QString         message;
    QString         groupedBy;
    QString         dataType;
    QString         name;
    bool            withoutTCRule   = false;
    bool            withTCRule      = false;
    QDomElement     child = parent.firstChildElement(CT_XML_NODE_RULE);
    QString         nodePath;
    CTReplies       replies;

    m_currentRuleLevel++;

    while (child.isNull() == false)
    {
        replies.clear();

        nodePath    = buildPath(path, child, CT_XML_ATTR_NAME);
        groupedBy   = child.attribute(CT_XML_ATTR_GROUP_BY, "").toLower();
        dataType    = child.attribute(CT_XML_ATTR_DATA_TYPE, "").toLower();
        name        = child.attribute(CT_XML_ATTR_NAME, "").toLower();

        if (m_currentRuleLevel == 1)
        {
            if (groupedBy == CT_XML_VALUE_GROUP_BY_NONE)
            {
                if (withoutTCRule)
                {
                    valid = false;
                    message = "Duplicated rule with group_by attribute set to 'none'";

                    replies.appendReply(CTReply(CTReply::ReplyError, message));
                }
                else
                    withoutTCRule = true;

                if (child.hasChildNodes())
                {
                    valid = false;
                    message = "Children not allowed for rules having group_by attribute set to 'None'";

                    replies.appendReply(CTReply(CTReply::ReplyError, message));
                }

                if (dataType != CT_XML_VALUE_DATA_TYPE_PHYSICAL)
                {
                    valid = false;
                    message = "data_type attribute must be 'physical' for rules where group_by attribute is set to 'none'";

                    replies.appendReply(CTReply(CTReply::ReplyError, message));
                }
                else
                    m_currentPhysicalRulesCount++;
            }
            else if (groupedBy == CT_XML_VALUE_GROUP_BY_ALL)
            {
                if (withTCRule)
                {
                    valid = false;
                    message = "Duplicated rule with group_by attribute set to 'all'";

                    replies.appendReply(CTReply(CTReply::ReplyError, message));
                }
                else
                    withTCRule = true;

                if (child.childNodes().count() != 1)
                {
                    valid = false;
                    message = "Rules with group_by attribute set to 'all' must have one child";

                    replies.appendReply(CTReply(CTReply::ReplyError, message));
                }

                if (dataType != CT_XML_VALUE_DATA_TYPE_INTERMEDIATE)
                {
                    valid = false;
                    message = "data_type attribute must be 'physical' for rules where group_by attribute is set to 'all'";

                    replies.appendReply(CTReply(CTReply::ReplyError, message));
                }

                m_currentRules.append(name);
            }
            else
            {
                valid = false;
                message = "Default rules must have group_by attribute set to 'none' or 'all'";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }
        }

        if (m_currentRuleLevel == 2)
        {
            if (m_currentRules.indexOf(name) != -1)
            {
                valid = false;
                message = "Duplicated rule " + name;

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }
            else
                m_currentRules.append(name);

            if (groupedBy != CT_XML_VALUE_GROUP_BY_NULL)
            {
                valid = false;
                message = "For default rules, group_by attribute must be set to 'null' if the parent group_by attribute is 'all'";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }

            if (dataType != CT_XML_VALUE_DATA_TYPE_PHYSICAL)
            {
                valid = false;
                message = "For default rules, data_type attribute must be set to 'physical' if the parent group_by attribute is 'all'";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }
            else
                m_currentPhysicalRulesCount++;

            if (child.hasChildNodes())
            {
                valid = false;
                message = "Children not allowed for rules having group_by attribute set to 'null'";

                replies.appendReply(CTReply(CTReply::ReplyError, message));
            }

        }

        addReplies(replies, nodePath);

        valid &= validateDefaultRules(child, nodePath);

        child = child.nextSiblingElement(CT_XML_NODE_RULE);
    }

    if (m_currentRuleLevel == 1)
    {
        replies.clear();

        if (withoutTCRule == false)
        {
            valid   = false;
            message = "There must be one default rule with group_by attribute set to 'none'";

            replies.appendReply(CTReply(CTReply::ReplyError, message));
        }

        if (withTCRule == false)
        {
            valid   = false;
            message = "There must be one default rule with group_by attribute set to 'all'";

            replies.appendReply(CTReply(CTReply::ReplyError, message));
        }

        if (replies.count() > 0 )
            addReplies(replies, path);
    }

    m_currentRuleLevel--;

    return valid;
}

QString ConsolidationTreeValidator::buildPath(const QString& parentPath, const QDomElement &element, const QString& attribute)
{
    QString path = parentPath;

    if (element.isNull() == false)
    {
        if (path.isEmpty() == false)
            path += "/";

        path += element.tagName();

        if (attribute.isEmpty() == false)
        {
            if (element.hasAttribute(attribute))
                path += "[" + attribute + "=" + element.attribute(attribute) +"]";
        }
        else if (element.hasAttributes())
        {
            path += "[";

            QDomNamedNodeMap attributes = element.attributes();

            for (int idx = 0; idx < attributes.count(); idx++)
            {
                path += attributes.item(idx).nodeName() + "=" + attributes.item(idx).nodeValue();
                path += " ";
            }

            path += "]";
        }
    }

    return path;
}

void ConsolidationTreeValidator::addReplies(const CTReplies &replies, const QString &nodePath)
{
    if (replies.count() > 0)
    {
        if (nodePath.isEmpty() == false)
        {
            QString message = "In " + nodePath;
            m_replies.appendReply(CTReply(CTReply::ReplyUnknown, message));
        }

        m_replies.appendReplies(replies);
    }
}


