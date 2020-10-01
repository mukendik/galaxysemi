#include "gex_line_edit.h"

#include <QtGui>
#include <QModelIndex>

#include "tree_model_completer.h"
#define QT_DEBUG

GexLineEdit::GexLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    completer = new TreeModelCompleter(this);
    completer->setWidget(this);
//    setCompleter(completer);
    completer->setModel(treeModelFromFile("treemodel.txt"));
    completer->setSeparator(QLatin1String("."));
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::UnsortedModel);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setWrapAround(false);

    QObject::connect(completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

void GexLineEdit::insertCompletion(const QString& completionText) {
    if (completer->widget() != this)
        return;

    QString beforePrefix, afterPrefix, initialText;

    // Get current position
    int currentPos = cursorPosition();
    int nextPosition = currentPos + completionText.length() - completer->completionPrefix().length();
    initialText = text();
    beforePrefix = initialText.left(currentPos - completer->completionPrefix().length());
    afterPrefix = initialText.right(initialText.length() - currentPos);

    // clean prefix
    setText(beforePrefix + completionText + afterPrefix);

    // Set right cursor position
    setCursorPosition(nextPosition);
}

QString GexLineEdit::textUnderCursor() const {
//    QTextCursor tc = textCursor();
//    tc.select(QTextCursor::WordUnderCursor);
//    QString t = tc.selectedText();
//    return tc.selectedText();

    int currentPos = cursorPosition();
    QString fullText = text();
    QString currentText;
    currentText = fullText.left(currentPos);

    return currentText;
}

void GexLineEdit::focusInEvent(QFocusEvent *e) {
    if (completer)
        completer->setWidget(this);
    QLineEdit::focusInEvent(e);
}


void GexLineEdit::mouseMoveEvent(QMouseEvent *e)
{
    int cursorPos = cursorPositionAt( e->pos());

    QString wordUnderCursor = "";
    QTextLayout textLayout(text());
    int start = textLayout.previousCursorPosition(cursorPos, QTextLayout::SkipWords);

    // ## text layout should support end of words.
    int end = textLayout.nextCursorPosition(cursorPos, QTextLayout::SkipWords);
    while (start < end)
    {
        wordUnderCursor.append(text().at(start));
        ++start;
    }

    setToolTip(QString().append(wordUnderCursor)
               .append(": syntax"));
    e->accept();
}


void GexLineEdit::keyPressEvent(QKeyEvent *e) {
//    if (completer && completer->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            qDebug() << "nothing to complete";
            QLineEdit::keyPressEvent(e);
            return; // let the completer do default behavior
        default:
            break;
        }
//    }

    const bool ctrlSpace = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+SPACE
    if (!completer)
    {
        QLineEdit::keyPressEvent(e);
        qDebug() << "No completer !";
        return;
    }

    if (!ctrlSpace)
        QLineEdit::keyPressEvent(e);

    bool isModifierOnly = ((e->key() == Qt::Key_Shift) ||
            (e->key() == Qt::Key_Control) ||
            (e->key() == Qt::Key_Alt)) &&
            !ctrlSpace;
    if (isModifierOnly)
    {
        qDebug() << "Modifier : hide popup";
        completer->popup()->hide();
        return;
    }


    static QString eow("~!@#$%^&*_+{}|:\"<>?,./;'[]\\-="); // end of word

    // Remove separator from end of word
    eow.remove(completer->separator());

    QString completionPrefix = textUnderCursor();
    qDebug() << "completionPrefix:" << completionPrefix;

    if (completionPrefix.isEmpty() || !eow.contains(completionPrefix.right(1)))
    {
        if (completionPrefix != completer->completionPrefix())
        {
            qDebug() << "completer->completionPrefix():" << completer->completionPrefix();
            completer->setCompletionPrefix(completionPrefix);
            completer->popup()->setCurrentIndex(completer->completionModel()->index(0, 0));
            qDebug() << completer->completionModel()->index(0, 0).data().toString();
        }

        QRect cr = cursorRect();
        cr.setWidth(completer->popup()->sizeHintForColumn(0) + completer->popup()->verticalScrollBar()->sizeHint().width());
        completer->complete(cr); // popup it up!
    }
    else
    {
        completer->popup()->hide();
        qDebug() << "endofword detected, hide popup";
    }

}

QAbstractItemModel *GexLineEdit::treeModelFromFile(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    QStandardItemModel *model = new QStandardItemModel(completer);

    QStandardItem *lastItem = model->invisibleRootItem();
    int lastLevel = 0;

    while (!file.atEnd())
    {
        QString line = file.readLine();
        QString trimmmedLine = line.trimmed();
        if (trimmmedLine.isEmpty()) // Ignore empty line
            continue;

        QRegExp re("^\\s+");
        int nonws = re.indexIn(line);
        int level = 0;

        if (nonws == -1) // New root item found!
            level = 0;
        else
        {
            if (line.startsWith("\t"))
                level = re.cap(0).length();
            else
                level = re.cap(0).length()/4;
        }

        QStandardItem *item = new QStandardItem;
        item->setText(trimmmedLine);

        if (level == 0)
            model->invisibleRootItem()->appendRow(item);
        else
        {
            int lvlGap = lastLevel - level;
            if (lvlGap == 0) // same level
            {
                QStandardItem *parentItem = lastItem->parent();
                if (!parentItem)
                    parentItem = model->invisibleRootItem();
                parentItem->appendRow(item);
            }
            else if (lvlGap < 0) // add child to last item
                lastItem->appendRow(item);
            else
            {
                QStandardItem *parentItem = lastItem->parent();
                for (int i = 0 ; i < lvlGap; ++i)
                {
                    parentItem = parentItem->parent();
                    if (!parentItem)
                        parentItem = model->invisibleRootItem();
                }
                parentItem->appendRow(item);
            }
        }
        lastItem = item;
        lastLevel = level;
    }

    file.close();

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    QString indent = "";
    dumpItems(model->invisibleRootItem(), indent);

    return model;
}

void GexLineEdit::dumpItems(QStandardItem *item, QString &indent)
{
    QString localIndent = indent + "  ";
    qDebug() << localIndent << item->text();
    for (int i = 0; i < item->rowCount(); ++i)
        dumpItems(item->child(i), localIndent);
}

