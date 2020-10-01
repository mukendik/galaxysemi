#include "consolidation_tree_replies.h"

CTReply::CTReply()
    : m_line(-1), m_column(-1)
{
    m_type      = CTReply::ReplyError;
}

CTReply::CTReply(ReplyType type, const QString& message)
    : m_line(-1), m_column(-1)
{
    m_message   = message;
    m_type      = type;
}

int CTReply::line() const
{
    return m_line;
}

int CTReply::column() const
{
    return m_column;
}

const QString &CTReply::message() const
{
    return m_message;
}

CTReply::ReplyType CTReply::type() const
{
    return m_type;
}

void CTReply::setMessage(const QString &message)
{
    m_message = message;
}

void CTReply::setType(CTReply::ReplyType type)
{
    m_type = type;
}

void CTReply::setLine(int line)
{
    m_line = line;
}

void CTReply::setColumn(int column)
{
    m_column = column;
}

CTReplies::CTReplies()
{
}

CTReplies::CTReplies(const CTReplies &other)
{
    *this = other;
}

void CTReplies::appendReply(const CTReply &reply)
{
    m_replyList.append(reply);
}

void CTReplies::appendReplies(const CTReplies& replies)
{
    m_replyList.append(replies.m_replyList);
}

void CTReplies::clear()
{
    m_replyList.clear();
}

unsigned int CTReplies::count() const
{
    return m_replyList.count();
}

unsigned int CTReplies::warningCount() const
{
    return 0;
}

unsigned int CTReplies::errorCount() const
{
    return 0;
}

CTReply CTReplies::reply(int idx) const
{
    if (idx >= 0 && idx < m_replyList.count())
        return m_replyList.at(idx);

    return CTReply(CTReply::ReplyError, "Internal error: invalid index");
}

CTReplies &CTReplies::operator =(const CTReplies &other)
{
    if (this != &other)
    {
        m_replyList = other.m_replyList;
    }

    return *this;
}



