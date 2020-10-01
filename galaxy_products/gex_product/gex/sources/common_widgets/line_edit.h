#ifndef GEXLINEEDIT_H
#define GEXLINEEDIT_H

#include <QLineEdit>
#include <QMap>

class QAbstractItemModel;
class QLineEdit;
class QStandardItem;
class QStandardItemModel;

namespace GS
{
namespace Gex
{
class TreeModelCompleter;
/*! \class GexLineEdit
 * \brief Provide a line edit with word to word tool tips and hierarchical completers
 *
 */
class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    /// \brief Constructor
    LineEdit(QWidget *parent = 0);
    /// \brief Destructor
    virtual ~LineEdit();
    void SetCompleterSeparator(const QString &separator);
    void SetCompleterModel(QStandardItemModel *model);
    void SetToolTipsMap(QMap<QString, QString> toolTips);

protected:
     void keyPressEvent(QKeyEvent *e);
     void focusInEvent(QFocusEvent *e);
     void mouseMoveEvent(QMouseEvent *e);

private slots:
     /// \brief Insert completionText at the cursor position and set the cursor position after completionText
     void insertCompletion(const QString &completionText);

private:
     Q_DISABLE_COPY(LineEdit);
     /// \return text between the beginning and the cursor
     QString textUnderCursor() const;
     TreeModelCompleter     *mCompleter;    ///< Holds the tree model completers
     QMap<QString, QString> mTooltips;      ///< Holds the tool tips associated with the key
};
} // END Gex
} // END GS

#endif // GEXLINEEDIT_H
