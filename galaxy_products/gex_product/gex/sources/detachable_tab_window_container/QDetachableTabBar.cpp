#include "QDetachableTabBar.h"
#include "QDetachableTabWidget.h"
#include "QDetachableTabTextEdit.h"
#include "QDetachableTabWindow.h"

#include <QMouseEvent>
#include <QtMath>
#include "QSnappedVBoxLayout.h"
#include <QLabel>

QDetachableTabBar::QDetachableTabBar( QWidget *parent ) :
    QTabBar( parent ),
    mTargetedTabIndexWhenClick( -1 ),
    // 30 px seems sufficient
    mMinRipOffDistance( 30. ),
    mPreviewWindow( NULL ),
    mIsLocked( false )
{
}

QDetachableTabBar::~QDetachableTabBar()
{
    // this window needs to be managed
    delete mPreviewWindow;
}

bool QDetachableTabBar::tabCanBeDetachedTo( const QPoint &position ) const
{
    // get all axis distances
    float xDistance = qAbs( mClickPosition.x() - position.x() );
    float yDistance = qAbs( mClickPosition.y() - position.y() );

    // distance traveled has to be sufficient and it must remain one tab at
    // least
    return
        // tab bar must not be locked
        ( ! mIsLocked ) &&
        (
            // y-move is sufficient or...
            ( yDistance >= mMinRipOffDistance ) ||
            // x-move AND y-move is sufficient. Necessary because of the horizontal
            // tab move feature in the tab bar
            (
                ( xDistance >= mMinRipOffDistance ) &&
                ( yDistance >= mMinRipOffDistance )
            )
        );
}

void QDetachableTabBar::mouseReleaseEvent( QMouseEvent *event )
{
    processMouseReleaseEvent( event );
    QTabBar::mouseReleaseEvent( event );
}

void QDetachableTabBar::processMouseReleaseEvent( QMouseEvent *event )
{
    // only left button is interesting here
    if( event->button() == Qt::LeftButton )
        detachOrMoveTab( event->globalPos() );

    // restore movable feature after release. This feature was disabled in mouse
    // move event processing
    setMovable( true );

    // hide the preview window, and remove content refreshed by mouse move
    if( mPreviewWindow != NULL )
    {
        // delete all layout element of the preview window
        QLayoutItem *child;
        while( ( child = mPreviewWindow->layout()->takeAt( 0 ) ) != NULL )
        {
            // close label widget and delete layout element
            child->widget()->close();
            delete child;
        }

        // hide this window but don't delete it. It will be deleted in the
        // destructor
        mPreviewWindow->hide();
    }
}

void QDetachableTabBar::detachOrMoveTab( const QPoint &position )
{
    // get the parent of this tab bar
    QDetachableTabWidget *parent =
        static_cast< QDetachableTabWidget * >( parentWidget() );

    // GCORE-7617 - Use currentIndex instead of
    //              mTabBar->mTargetedTabIndexWhenClick
    // first, try to move the tab from a window to another one if necessary.
    // there is not any check about the distance traveled by the mouse's pointer
    if ( ! parent->tryMoveTab( currentIndex(), position ) )
    {
        // call the parent detachTab method if conditions are met
        if( tabCanBeDetachedTo( position ) )
            parent->detachTab( currentIndex(), position );
    }
}

void QDetachableTabBar::mousePressEvent( QMouseEvent *event )
{
    processMousePressEvent( event );
    QTabBar::mousePressEvent( event );
}

void QDetachableTabBar::processMousePressEvent( QMouseEvent *event )
{
    if( event->button() == Qt::LeftButton )
    {
        // record position of the click
        mClickPosition = event->globalPos();

        // GCORE-7377 - provide a workaround about a Qt bug related to tab bar
        //              having as much tab as needed to create a side scrolling
        //              control. In case of double click, a bad tab could be
        //              selected if the double clicked tab is near the scroll
        //              control
        // if locked, means a rename is in progress
        if( tabAt( event->pos() ) != mTargetedTabIndexWhenClick )
            stopEditing();

        // get the index in the tab bar of the targeted tab
        mTargetedTabIndexWhenClick = tabAt( event->pos() );

        // focus the tab targeted using the parent wiget feature
        QDetachableTabWidget *parent =
            static_cast< QDetachableTabWidget * >( parentWidget() );

        parent->focusTab( mTargetedTabIndexWhenClick );

        // modify the title of the parent window
        changeParentWindowTitle( tabText( mTargetedTabIndexWhenClick ) );
    }
}

void QDetachableTabBar::mouseMoveEvent( QMouseEvent *event )
{
    processMouseMoveEvent( event );
    QTabBar::mouseMoveEvent( event );
}

void QDetachableTabBar::processMouseMoveEvent( QMouseEvent *event )
{
    // only left button is interesting here,masking is mandatory for mouse move
    // envent...
    if( event->buttons() & Qt::LeftButton )
    {
        // create and display a preview window
        previewWindow( event->globalPos() );

        if( tabCanBeDetachedTo( event->globalPos() ) )
        {
            // stop the movable feature once conditions are met to detach a
            // window from moving tab
            setMovable( false );
        }
    }
}

void QDetachableTabBar::previewWindow( const QPoint &position )
{
    // create a preview window if necessary
    if( mPreviewWindow == NULL )
    {
        // create the tooltip window with its vertical layout
        mPreviewWindow = new QWidget();
        mPreviewWindow->setParent( NULL, Qt::FramelessWindowHint );
        mPreviewWindow->setLayout( new QSnappedVBoxLayout() );
    }

    // if layout is empty, add label
    if( mPreviewWindow->layout()->itemAt( 0 ) == NULL )
    {
        // extract the title of the tab
        const QString &tabTitle = this->tabText( mTargetedTabIndexWhenClick );

        QLabel *label = new QLabel( tabTitle );
        mPreviewWindow->layout()->addWidget( label );

        mPreviewWindow->show();
    }

    // calculation about size of preview window. Refreshed for each movement of
    // mouse's pointer
    mPreviewWindow->setGeometry( position.x(), position.y(), 0, 0 );
    mPreviewWindow->adjustSize();
}

void QDetachableTabBar::processMouseDoubleClickEvent( QMouseEvent *event )
{
    // left double click is interesting here
    if( ( event->button() == Qt::LeftButton ) && ( ! mIsLocked ) )
    {
        if (tabAt(event->pos()) != mTargetedTabIndexWhenClick)
        {
            return;
        }
        // get the tab text at specific index
        const QString &text = tabText( mTargetedTabIndexWhenClick );

        // change current tabText to specify rename is in progress, marking it
        // with '*'
        setTabText( mTargetedTabIndexWhenClick, "*" );

        // override right button, get the old one
        QWidget *old_right_button =
            tabButton( mTargetedTabIndexWhenClick, QTabBar::RightSide );

        // remove parent relation to prevent resource liberation, if any
        if( old_right_button != NULL )
          old_right_button->setParent( NULL );

        // create the custom edition control
        QDetachableTabTextEdit *tabTextEdit =
            new QDetachableTabTextEdit( this, text, old_right_button );

        // connect signal/slot
        QObject::connect
          (
            tabTextEdit, SIGNAL( endEdit( QWidget*, const QString & ) ),
            this, SLOT( slotEndEdit( QWidget*, const QString & ) )
          );

        // adding the edition widget at the right side of the tab
        setTabButton
            (
                mTargetedTabIndexWhenClick,
                QTabBar::RightSide,
                tabTextEdit
            );

        // deactivate rename feature until this tab has been renamed
        mIsLocked = true;
    }
}

void QDetachableTabBar::mouseDoubleClickEvent( QMouseEvent *event )
{
    processMouseDoubleClickEvent( event );
    QTabBar::mouseDoubleClickEvent( event );
}

void QDetachableTabBar::changeParentWindowTitle( const QString &text )
{
    // delegate to parent widget
    QDetachableTabWidget *parent =
        static_cast< QDetachableTabWidget * >( parentWidget() );

    parent->changeParentWindowTitle( text );
}

void QDetachableTabBar::tabInserted( int )
{
    /**
     * \todo Implement this method for create new tab feature
     */
}

void QDetachableTabBar::tabRemoved( int )
{
    /**
     * \todo Implement this method for create new tab feature
     */
}

void QDetachableTabBar::stopEditing()
{
  // only if edition is in progress
  if( mIsLocked )
  {
    // accept current text inside the rename box
    QWidget *right_button =
        tabButton( mTargetedTabIndexWhenClick, QTabBar::RightSide );

    if( right_button != NULL )
    {
      QDetachableTabTextEdit *editBox =
          static_cast< QDetachableTabTextEdit * >( right_button );

      editBox->validateInput( editBox->text() );
    }
  }
}