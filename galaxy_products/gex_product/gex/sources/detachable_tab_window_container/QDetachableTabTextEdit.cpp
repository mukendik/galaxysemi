#include "QDetachableTabTextEdit.h"
#include "QDetachableTabWidget.h"
#include "QDetachableTabBar.h"

#include <QKeyEvent>
#include <QFocusEvent>

QDetachableTabTextEdit::QDetachableTabTextEdit
    (
        QDetachableTabBar *parent,
        const QString &text,
        QWidget *oldRightButton
     ) :
    QLineEdit( parent ),
    mInitialText( text ),
    mOldRightButton( oldRightButton ),
    mLastCancelUntilClose( true )
{
    // first initialize this control with provided text and select all text and
    // give the focus immediately
    setText( mInitialText );
    selectAll();
    setFocus();
}

void QDetachableTabTextEdit::closeWithTextInParent( const QString &text )
{
    // get true type of parent
    QDetachableTabBar *parent =
        static_cast< QDetachableTabBar * >( parentWidget() );

    // set the new text, hiding the edit box
    parent->setTabText( parent->currentIndex(), text );

    // remove the edit box from parent, restoring an old button if any
    parent->setTabButton
        ( parent->currentIndex(), QTabBar::RightSide, mOldRightButton );

    // change the parent window title
    parent->changeParentWindowTitle( text );

    // edition finished, unlock the tab bar
    parent->mIsLocked = false;

    // close this edit box
    close();

    QDetachableTabWidget *parent_tab_widget =
        static_cast< QDetachableTabWidget * >( parent->parentWidget() );

    emit endEdit( parent_tab_widget->currentWidget(), text );
}

void QDetachableTabTextEdit::cancelInput()
{
    // cancellation is defined in 2 step :
    // -first : if text has been modified, the first cancel operation reset the
    //  edit box content with tab's initial text
    // -second : if the text has not been modified, the input is totally
    //  cancelled

    // first reset, reinitialize initial text in the input control if text has
    // been modified
    if( !mLastCancelUntilClose )
    {
        setText( mInitialText );
        selectAll();
        mLastCancelUntilClose = true;
    }
    else
    {
        // full reset of tab's text
        closeWithTextInParent( mInitialText );
    }
}

void QDetachableTabTextEdit::validateInput( const QString &text )
{
    // nothing special to do but close this edition box and change tab's text
    closeWithTextInParent( text );
}

void QDetachableTabTextEdit::processKeyPressEvent( QKeyEvent *event )
{
    // implies a cancel of the edition
    if( event->key() == Qt::Key_Escape )
        cancelInput();
    // implies a validation of the input
    else if
        (
            ( event->key() == Qt::Key_Enter ) ||
            ( event->key() == Qt::Key_Return )
        )
        validateInput( text() );
    else
        // change the text of the tab
        mLastCancelUntilClose = false;
}

void QDetachableTabTextEdit::processMousePressEvent( QMouseEvent * )
{
    // get the parent tab bar then the parent tab widget
    QDetachableTabBar *tab_bar =
        static_cast< QDetachableTabBar * >( parentWidget() );

    QDetachableTabWidget *tab_widget =
        static_cast< QDetachableTabWidget * >( tab_bar->parentWidget() );

    tab_widget->setCurrentIndex( tab_bar->mTargetedTabIndexWhenClick );
}

void QDetachableTabTextEdit::keyPressEvent( QKeyEvent *event )
{
   processKeyPressEvent( event );
    /*if (event->key() != Qt::Key_Escape)
    {
        event->accept();
        QLineEdit::keyPressEvent(event);
    }*/
    event->accept();
    QLineEdit::keyPressEvent(event);
}

void QDetachableTabTextEdit::mousePressEvent( QMouseEvent *event )
{
    processMousePressEvent( event );
    QLineEdit::mousePressEvent( event );
}
