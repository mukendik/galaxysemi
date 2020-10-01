#include "tree_model_completer.h"
#include <QStringList>
#include <QCompleter>

namespace GS
{
namespace Gex
{
TreeModelCompleter::TreeModelCompleter(QObject *parent) :
    QCompleter(parent)
{
}

TreeModelCompleter::TreeModelCompleter(QAbstractItemModel *model, QObject *parent) :
    QCompleter(model, parent)
{
}

TreeModelCompleter::~TreeModelCompleter()
{
}

void TreeModelCompleter::setSeparator(const QString &separator)
{
    mSeparator = separator;
}

QString TreeModelCompleter::separator() const
{
    return mSeparator;
}

QStringList TreeModelCompleter::splitPath(const QString &path) const
{
    if (mSeparator.isNull())
        return QCompleter::splitPath(path);

    return path.split(mSeparator);
}

QString TreeModelCompleter::pathFromIndex(const QModelIndex &index) const
{
    if (mSeparator.isNull())
        return QCompleter::pathFromIndex(index);

    // navigate up and accumulate data
    QStringList dataList;
    for (QModelIndex i = index; i.isValid(); i = i.parent())
        dataList.prepend(model()->data(i, completionRole()).toString());

    return dataList.join(mSeparator);
}

} // END Gex
} // END GS
