#include "tree_model_completer.h"
#include <QStringList>
#include <QStandardItem>
#include <QDebug>

TreeModelCompleter::TreeModelCompleter(QObject *parent) :
    QCompleter(parent) {
}

TreeModelCompleter::TreeModelCompleter(QAbstractItemModel *model, QObject *parent) :
    QCompleter(model, parent) {
}

void TreeModelCompleter::setSeparator(const QString &separator) {
    sep = separator;
}

QString TreeModelCompleter::separator() const {
    return sep;
}

QStringList TreeModelCompleter::splitPath(const QString &path) const {
    if (sep.isNull()) {
        return QCompleter::splitPath(path);
    }

    return path.split(sep);
}

void TreeModelCompleter::dumpIndexes(const QModelIndex &index, QString &indent) const
{
    QString localIndent = indent + "  ";
    qDebug() << "dump" << indent << index.data(completionRole()).toString();
    for (int i = 0; i < model()->rowCount(index) ; ++i)
        dumpIndexes(index.child(i,0), localIndent);
}

QString TreeModelCompleter::pathFromIndex(const QModelIndex &index) const {
    if (sep.isNull()) {
        return QCompleter::pathFromIndex(index);
    }

    QString indent = "";
    dumpIndexes(index, indent);

    // navigate up and accumulate data
    QStringList dataList;
    for (QModelIndex i = index; i.isValid(); i = i.parent()) {
        dataList.prepend(model()->data(i, completionRole()).toString());
    }

    qDebug() << "Path from index: " << dataList.join(sep) << endl;
    return dataList.join(sep);
}



