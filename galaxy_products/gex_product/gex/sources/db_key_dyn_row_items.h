#ifndef DB_KEY_DYN_ROW_ITEMS_H
#define DB_KEY_DYN_ROW_ITEMS_H

#include <QObject>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

#include "common_widgets/line_edit.h"

class QStandardItemModel;

namespace GS
{
namespace Gex
{
class DbKeyData;
/*! \class DbKeyDynRowItems
 * \brief Used to store a row of dynamic key for the db keys editor
 */
class DbKeyDynRowItems: public QHBoxLayout
{
    Q_OBJECT

public:
    /// \brief Constructor
    /// \param keyData ptr to data used in the gui
    /// \param allowedNames list of allowed key names
    DbKeyDynRowItems(DbKeyData &keyData, const QStringList &allowedNames);
    /// \brief Destructor
    virtual ~DbKeyDynRowItems();
    /// Setters
    void SetNameToolTips(QMap<QString,QString> toolTips);
    void SetExpressionToolTips(QMap<QString,QString> toolTips);
    void SetExpressionIsValid();
    void CheckValidity();

signals:
    /// \brief Add row requested
    void AddRow(int rowId);
    /// \brief Remove row requested
    void RemoveRow(int rowId);

private slots:
    /// \brief update keydata
    void OnNameChanged(QString newName);
    /// \brief update keydata
    void OnExpressionChanged(QString newExpression);
    /// \brief emit a signal with the right row id
    void OnAddRowRequested();
    /// \brief emit a signal with the right row id
    void OnRemoveRowRequested();

private:
    Q_DISABLE_COPY(DbKeyDynRowItems)
    /// \brief init widgets
    void                initWidgets();
    /// \brief build the tree model completer used for mLineName
    QStandardItemModel  *BuildNameTreeModelCompleter();
    /// \brief build the tree model completer used for mLineExpression
    QStandardItemModel  *BuildExpressionTreeModelCompleter();

    /// Members
    DbKeyData       &mKeyData;          ///< ref to data object
    LineEdit        mLineName;          ///< holds the key name
    LineEdit        mLineExpression;    ///< holds the key expression
    QPushButton     mAddRow;            ///< button to request new row
    QPushButton     mRemoveRow;         ///< button to remove one row
    QStringList     mAllowedNames;      ///< list of all allowed names
};

} // END Gex
} // END GS

#endif // DB_KEY_DYN_ROW_ITEMS_H
