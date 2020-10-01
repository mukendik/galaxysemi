#ifndef CONSOLIDATION_TREE_H
#define CONSOLIDATION_TREE_H

#include "consolidation_tree_data.h"
#include "consolidation_tree_replies.h"

#include <QDateTime>

class CTQueryFilter;
class GexDbPlugin_Galaxy;
class QTextEdit;

class ConsolidationTree
{
public:

    ConsolidationTree(GexDbPlugin_Galaxy * pPlugin);

    const QDateTime&                lastModification() const;
    const QString &                 xmlContent() const;
    const ConsolidationTreeData&    data() const;
    const CTReplies&                replies() const;

    // Load the Consolidation Tree data from the DB
    // Return true when succeed to load a valid CT, otherwise false
    bool                            loadFromDB();
    bool                            importFromFile(const QString& fileLocation);

    // Return true if the Consolidation Tree is uptodate comparing the one stored in the DB, otherwise false
    bool                            isUpToDate() const;

    // Try to update the Consolidation Tree data.
    // Return true when succeed, otherwise false.
    bool                            update(const QString &xmlContent);

    // Return true if the Consolidation Tree data is valid, otherwise false.
    bool                            isValid() const;

protected:

    // Load Consolidation Tree Data and validate it
//    bool                          load(const QByteArray& data, const QDateTime& lastModification, CTReplies& replies);
    // Validate the Consolidation Tree data (Use ConsolidationTreeValidator)
//    bool                          validate(const QByteArray& data, CTReplies& replies) const;

    bool                            saveIntoDB(const QString& xmlContent, const QDateTime& lastUpdate);

private:

    GexDbPlugin_Galaxy *            m_pPlugin;
    ConsolidationTreeData           m_data;
    CTReplies                       m_replies;
    QDateTime                       m_lastModification;
    bool                            m_valid;
};

#endif // CONSOLIDATION_TREE_H
