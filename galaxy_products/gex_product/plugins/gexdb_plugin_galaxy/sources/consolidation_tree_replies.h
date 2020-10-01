#ifndef CONSOLIDATION_TREE_REPLIES_H
#define CONSOLIDATION_TREE_REPLIES_H

#include <QList>
#include <QString>

class CTReply
{
public:

    enum ReplyType
    {
        ReplyUnknown    = -1,
        ReplyWarning    = 0,
        ReplyError      = 1
    };

    CTReply();
    CTReply(ReplyType type, const QString& message);

    int             line() const;
    int             column() const;
    const QString&  message() const;
    ReplyType       type() const;

    void            setMessage(const QString& message);
    void            setType(ReplyType type);
    void            setLine(int line);
    void            setColumn(int column);

private:

    QString         m_message;
    ReplyType       m_type;
    int             m_line;
    int             m_column;
//    CTLocation  m_location;
};

class CTReplies
{
public:

    CTReplies();
    CTReplies(const CTReplies& other);

    void            appendReply(const CTReply& reply);
    void            appendReplies(const CTReplies &replies);
    void            clear();

    unsigned int    count() const;
    unsigned int    warningCount() const;
    unsigned int    errorCount() const;

    CTReply         reply(int idx) const;

    CTReplies&      operator=(const CTReplies& other);

protected:

    QList<CTReply>  m_replyList;
};

#endif // CONSOLIDATION_TREE_REPLIES_H
