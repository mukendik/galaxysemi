#include "consolidation_tree_data.h"
#include "consolidation_tree_validator.h"
#include "consolidation_tree_replies.h"
#include "gexdb_plugin_galaxy.h"
#include <gqtl_log.h>

ConsolidationTreeData::ConsolidationTreeData(GexDbPlugin_Galaxy *pPlugin)
    : QObject(), m_valid(false), m_pPlugin(pPlugin)
{
}

ConsolidationTreeData::ConsolidationTreeData(const ConsolidationTreeData &other)
    : QObject()
{
    *this = other;
}

ConsolidationTreeData::~ConsolidationTreeData()
{
}

void ConsolidationTreeData::clear()
{
    m_valid = false;
    m_xmlContent.clear();
    m_domDocument.clear();
}

const QString &ConsolidationTreeData::content() const
{
    return m_xmlContent;
}

const QDomDocument &ConsolidationTreeData::domDocument() const
{
    return m_domDocument;
}

bool ConsolidationTreeData::isValid() const
{
    return m_valid;
}

bool ConsolidationTreeData::setContent(const QString &content, CTReplies &replies)
{
    QString errorMessage;
    int     line    = -1;
    int     column  = -1;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Consolidation Tree Data[set Content]: starting");

    m_valid         = false;
    m_xmlContent    = content;

    if (m_domDocument.setContent(m_xmlContent, &errorMessage, &line, &column))
    {
        ConsolidationTreeValidator  CTValidator(m_pPlugin, ":/resources/consolidation_tree.xsd");

        m_valid = CTValidator.validate(m_xmlContent, replies);
    }
    else
    {
        QString message = "Consolidation Tree Data [setContent]: not XML compliant - " + errorMessage;

        GSLOG(SYSLOG_SEV_ERROR, message.toLatin1().constData());

        CTReply replyError;

        replyError.setType(CTReply::ReplyError);
        replyError.setMessage(message);
        replyError.setLine(line);
        replyError.setColumn(column);

        replies.appendReply(replyError);
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Consolidation Tree Data[set Content]: %1").arg(
             (m_valid) ? "success" : "failed").toLatin1().constData());

    // Emit signal when Consolidation Tree Data changed
    emit dataChanged();

    return m_valid;
}

ConsolidationTreeData &ConsolidationTreeData::operator =(const ConsolidationTreeData &other)
{
    if (this != &other)
    {
        m_valid         = other.m_valid;
        m_xmlContent    = other.m_xmlContent;
        m_domDocument   = other.m_domDocument;
        m_pPlugin       = other.m_pPlugin;
    }

    return *this;
}
