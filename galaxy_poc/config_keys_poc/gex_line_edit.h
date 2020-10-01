#ifndef GEXLINEEDIT_H
#define GEXLINEEDIT_H

#include <QLineEdit>

class TreeModelCompleter;
class QAbstractItemModel;
class QLineEdit;
class QStandardItem;

class GexLineEdit : public QLineEdit {
    Q_OBJECT

public:
    GexLineEdit(QWidget *parent = 0);

protected:
     void keyPressEvent(QKeyEvent *e);
     void focusInEvent(QFocusEvent *e);
     void mouseMoveEvent(QMouseEvent *e);

 private slots:
     // Insert completionText at the cursor position and set the cursor position after completionText
     void insertCompletion(const QString &completionText);

 private:
     QString textUnderCursor() const;

     QAbstractItemModel *treeModelFromFile(const QString& fileName);
     void dumpItems(QStandardItem *item, QString &indent);

     TreeModelCompleter *completer;
};


#endif // GEXLINEEDIT_H
