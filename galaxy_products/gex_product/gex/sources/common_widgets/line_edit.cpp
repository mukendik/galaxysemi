#include "line_edit.h"

#include <QtWidgets>
#include <QModelIndex>
#include <QLatin1String>

#include "tree_model_completer.h"

namespace GS
{
namespace Gex
{
LineEdit::LineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    mCompleter = new TreeModelCompleter(this);
    mCompleter->setWidget(this);
    mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    mCompleter->setModelSorting(QCompleter::UnsortedModel);
    mCompleter->setCompletionMode(QCompleter::PopupCompletion);
    mCompleter->setWrapAround(false);

    QObject::connect(mCompleter, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}

LineEdit::~LineEdit()
{
    if (mCompleter)
        delete mCompleter;
}

void LineEdit::SetCompleterSeparator(const QString &separator)
{
    if (mCompleter)
        mCompleter->setSeparator(QLatin1String(separator.toLatin1().constData()));
}

void LineEdit::SetCompleterModel(QStandardItemModel *model)
{
    if (mCompleter)
        mCompleter->setModel(model);
}

void LineEdit::SetToolTipsMap(QMap<QString, QString> toolTips)
{
    mTooltips = toolTips;
}

void LineEdit::insertCompletion(const QString& completionText)
{
    if (!mCompleter || (mCompleter->widget() != this))
        return;

    QString beforePrefix, afterPrefix, initialText;

    // Get current position
    int currentPos = cursorPosition();
    // Compute new pos: current  pos  + diff between full completion and prefix
    int nextPosition = currentPos +
                        completionText.length() -
                        mCompleter->completionPrefix().length();
    initialText = text();
    beforePrefix = initialText.left(
                currentPos - mCompleter->completionPrefix().length());
    afterPrefix = initialText.right(initialText.length() - currentPos);

    // Concatenate new text
    setText(beforePrefix + completionText + afterPrefix);
    // Set right cursor position
    setCursorPosition(nextPosition);
}

QString LineEdit::textUnderCursor() const
{
    int currentPos = cursorPosition();
    QString fullText = text();
    QString currentText;
    currentText = fullText.left(currentPos);

    return currentText;
}

void LineEdit::focusInEvent(QFocusEvent *e)
{
    if (mCompleter)
        mCompleter->setWidget(this);
    QLineEdit::focusInEvent(e);
}

void LineEdit::mouseMoveEvent(QMouseEvent *e)
{
    int cursorPos = cursorPositionAt( e->pos());

    QString wordUnderCursor = "";
    QTextLayout textLayout(text());
    int start = textLayout.previousCursorPosition(
                cursorPos, QTextLayout::SkipWords);

    // ## text layout should support end of words.
    int end = textLayout.nextCursorPosition(cursorPos, QTextLayout::SkipWords);
    while (start < end)
    {
        wordUnderCursor.append(text().at(start));
        ++start;
    }

    QString toolTip = mTooltips.value(wordUnderCursor);

//    if (toolTip.isEmpty())
//        toolTip = wordUnderCursor;

    setToolTip(toolTip);

    e->accept();
}


void LineEdit::keyPressEvent(QKeyEvent *e)
{
//    if (completer && completer->popup()->isVisible())
//    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            QLineEdit::keyPressEvent(e);
            return; // let the completer do default behavior
        default:
            break;
        }
//    }

    const bool ctrlSpace = ((e->modifiers() & Qt::ControlModifier) &&
                            e->key() == Qt::Key_Space); // CTRL+SPACE
    if (!mCompleter)
    {
        QLineEdit::keyPressEvent(e);
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
        mCompleter->popup()->hide();
        return;
    }

    static QString eow("~!@#$%^&*_+{}|:\"<>?,./;'[]\\-="); // end of word

    // Remove separator from end of word
    eow.remove(mCompleter->separator());

    QString completionPrefix = textUnderCursor();

    if (completionPrefix.isEmpty() || !eow.contains(completionPrefix.right(1)))
    {
        if (completionPrefix != mCompleter->completionPrefix())
        {
            mCompleter->setCompletionPrefix(completionPrefix);
            mCompleter->popup()->setCurrentIndex(
                        mCompleter->completionModel()->index(0, 0));
        }

        QRect cr = cursorRect();
        cr.setWidth(mCompleter->popup()->sizeHintForColumn(0) +
                    mCompleter->popup()->verticalScrollBar()->sizeHint().width());
        mCompleter->complete(cr); // popup it up!
    }
    else
        mCompleter->popup()->hide();
}

} // END Gex
} // END GS

