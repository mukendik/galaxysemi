#ifndef DB_KEY_STATIC_ROW_ITEMS_H
#define DB_KEY_STATIC_ROW_ITEMS_H

#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMap>
#include "common_widgets/line_edit.h"
#include "db_key_data.h"

class QStandardItemModel;

namespace GS
{
namespace Gex
{
//class DbKeyData;
/*! \class DbKeyStaticRowItems
 * \brief Used to store a row of static key for the db keys editor
 */
class DbKeyStaticRowItems: public QHBoxLayout
{
    Q_OBJECT

public:
    /// \brief Constructor
    /// \param keyData ptr to data used in the gui
    /// \param allowedNames list of allowed key names
    DbKeyStaticRowItems(DbKeyData &keyData,
                        const QStringList &allowedNames,
                        QMap<QString,QString> toolTips);
    /// \brief Destructor
    virtual ~DbKeyStaticRowItems();
    /// \brief Setters
    void SetName(const QString &name);
    void SetValue(const QString &value);
    void SetExpression(const QString &expression);
    void SetEvaluatedValue(const QString &evaluatedValue);

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
    Q_DISABLE_COPY(DbKeyStaticRowItems)
    /// \brief init widgets
    void                initWidgets();
    /// \brief build the tree model completer used for mLineExpression
    QStandardItemModel *BuildExpressionTreeModelCompleter();

    /// Members
    DbKeyData               &mKeyData;              ///< ref to data object
    QStringList             mAllowedNames;          ///< list of all allowed names
    QComboBox               mComboName;             ///< Combo to display list of allowed names
    QLineEdit               mLineValue;             ///< holds the key name
    LineEdit                mLineExpression;        ///< holds the key expression
    QLineEdit               mLineEvaluatedValue;    ///< holds the key value after evaluation
    QPushButton             mAddRow;                ///< button to request new row
    QPushButton             mRemoveRow;             ///< button to remove one row
    QMap<QString,QString>   mExpressionsToolTips;   ///< tool tips for all expression and functions
};

} // END Gex
} // END GS

#endif // DB_KEY_STATIC_ROW_ITEMS_H
