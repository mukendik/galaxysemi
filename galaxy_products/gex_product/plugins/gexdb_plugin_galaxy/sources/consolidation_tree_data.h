#ifndef CONSOLIDATION_TREE_DATA_H
#define CONSOLIDATION_TREE_DATA_H

#include <QObject>
#include <QDomDocument>

class CTReplies;
class GexDbPlugin_Galaxy;

class ConsolidationTreeData : public QObject
{
    Q_OBJECT

public:
    ConsolidationTreeData(GexDbPlugin_Galaxy * pPlugin);
    ConsolidationTreeData(const ConsolidationTreeData& other);
    virtual ~ConsolidationTreeData();

    void                    clear();

    const QString&          content() const;
    const QDomDocument&     domDocument() const;

    bool                    isValid() const;
    bool                    setContent(const QString& content, CTReplies& replies);

    ConsolidationTreeData&  operator=(const ConsolidationTreeData& other);

private:

    bool                    m_valid;
    QString                 m_xmlContent;
    QDomDocument            m_domDocument;

    GexDbPlugin_Galaxy *    m_pPlugin;

signals:

    void                    dataChanged();
};

#endif // CONSOLIDATION_TREE_DATA_H
