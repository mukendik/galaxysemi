#ifndef LIMITSDELEGATE_H
#define LIMITSDELEGATE_H

#include <QStyledItemDelegate>

namespace GS
{
namespace Gex
{

class LimitsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    /// \brief Constructor
    explicit LimitsDelegate(QObject *parent = 0);
    virtual ~LimitsDelegate(){}

    // implement QAbstractTableModel functions
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void setAlgoValues(QStringList values);
    void setAlgoNames(QStringList names);

private:
    QStringList             mAlgoValues;    ///< Holds possible algo values
    QStringList             mAlgoNames;     ///< Holds possible algo names
};

} // namespace Gex
} // namespace GS

#endif // LIMITSDELEGATE_H
