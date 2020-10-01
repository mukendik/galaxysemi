#ifndef TREEMODELCOMPLETER_H
#define TREEMODELCOMPLETER_H

#include <QCompleter>

namespace GS
{
namespace Gex
{
/*! \class TreeModelCompleter
 *
 */
class TreeModelCompleter : public QCompleter
{
    Q_OBJECT

public:
    /// \brief Constructor
    TreeModelCompleter(QObject *parent = 0);
    /// \brief Constructor
    /// \param model ptr to tree model used by the completer
    TreeModelCompleter(QAbstractItemModel *model, QObject *parent = 0);
    /// \brief Destructor
    virtual ~TreeModelCompleter();
    /// \return completer separator
    QString separator() const;

public slots:
    void setSeparator(const QString &separator);

protected:
    /// \return a list of all words splitted by the separator if exists
    QStringList splitPath(const QString &path) const;
    /// \brief starting from the index rebuild the word concatenation
    /// that has lead to this index
    QString pathFromIndex(const QModelIndex &index) const;

private:
    Q_DISABLE_COPY(TreeModelCompleter);
    QString mSeparator; ///<  Holds the completer separator
};

} // END Gex
} // END GS
#endif // TREEMODELCOMPLETER_H
