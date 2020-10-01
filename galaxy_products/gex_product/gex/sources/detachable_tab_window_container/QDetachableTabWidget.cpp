#include "QDetachableTabWidget.h"
#include "QDetachableTabBar.h"
#include "QDetachableTabWindow.h"
#include "QDetachableTabWindowContainer.h"
#include "QDetachableTabTextEdit.h"

QDetachableTabWidget::QDetachableTabWidget
    (QDetachableTabWindowContainer *windowContainer, QWidget *parent) :
    QTabWidget( parent ),
    mWindowContainer( windowContainer ),
    mTabBar( NULL )
{
    // replace standard QTabBar by an extended one
    mTabBar = new QDetachableTabBar( this );

    QObject::connect
      (
        mTabBar, SIGNAL( tabRenamed( QWidget*, const QString & ) ),
        mWindowContainer, SLOT( slotTabRenamed( QWidget*, const QString & ) )
      );

    setTabBar( mTabBar );

    // scroll button used if there is a lot of tabs
    setUsesScrollButtons( true );

    // tabs can be moved for sorting purposes
    setMovable( true );

    // tabs are closable
    setTabsClosable( true );

    // enable mouse move event to be fired even if no button is pressed
    setMouseTracking( true );

    // connect a close tab request signal to the custom close tab slot
    connect
        (
            this, SIGNAL( tabCloseRequested( int ) ),
            this, SLOT( closeTab( int ) )
        );

    // GCORE-7617
    // connect a signal on current changed
    connect
        (
            this, SIGNAL( currentChanged( int ) ),
            this, SLOT( currentTabChanged( int ) )
        );
}

QDetachableTabWidget::~QDetachableTabWidget()
{

}

bool QDetachableTabWidget::tryMoveTab( int index, const QPoint &position )
{
    // look in main window if an untabbed window exists in the position
    // specified
    QDetachableTabWindow *externalWindow =
        mWindowContainer->getDetachableTabWindowAt( position );

    // the source window
    QDetachableTabWindow *sourceWindow =
        static_cast< QDetachableTabWindow * >( parentWidget() );

    // a window is found, proceed to move if source window differs from the
    // found window
    if
        (
            ( externalWindow != NULL ) &&
            ( sourceWindow != externalWindow ) &&
            ( ! externalWindow->mTabs->isTabBarLocked() ) &&
            ( ! sourceWindow->mTabs->isTabBarLocked() )
        )
    {
        // get the element in the targeted tab
        QWidget *tabContent = widget( index );
        const QString &tabTitle = this->tabText( index );

        // change the owner of the widget
        externalWindow->addTab( tabContent, tabTitle );

        // if the source window is empty, remove it
        if( sourceWindow->count() == 0 )
            mWindowContainer->removeDetachableTabWindow( sourceWindow );

        return true;
    }

    // no external window found, tab has not been moved. A detach window will be
    // done
    return false;
}

void QDetachableTabWidget::detachTab( int index, const QPoint &position )
{
    // get the element in the targeted tab
    QWidget *tabContent = widget( index );
    const QString &tabTitle = this->tabText( index );

    // current tab index that is active
    int activeTabIndex = currentIndex();

    // at least one tab must remain to detach feature be effective
    if( count() > 1 )
    {
        // create a separate window with this tab content
        createExternalWindow( tabTitle, tabContent, position );

        // activate the correct tab after the removal if one remains
        // this code is needed if it remains less tab after the detach window
        // operation than the index of currently active one
        if( count() <= activeTabIndex )
            --activeTabIndex;

        setTabEnabled( activeTabIndex, true );
    }
}

void QDetachableTabWidget::createExternalWindow
    ( const QString &tabTitle, QWidget *tabContent, const QPoint &position )
{
    // create an external window
    QDetachableTabWindow *externalWindow =
        new QDetachableTabWindow( mWindowContainer );

    externalWindow->setAttribute(Qt::WA_DeleteOnClose);

    // set title and position of the new window
    externalWindow->setWindowTitle( tabTitle );
    externalWindow->setGeometry( position.x(), position.y(), 0, 0 );

    // adding the content of the tab inside this window
    externalWindow->addTab( tabContent, tabTitle );

    // GCORE-7615 - specify branding icon
    externalWindow->setWindowIcon
        ( QIcon( QString::fromUtf8( ":/gex/icons/gex_application.png" ) ) );

    // show as external window
    externalWindow->adjustSize();
    externalWindow->show();

    // add this external window to parent's collection
    mWindowContainer->addDetachableTabWindow( externalWindow );
}

void QDetachableTabWidget::changeParentWindowTitle( const QString &text )
{
    // the parent window
    QDetachableTabWindow *parent =
        static_cast< QDetachableTabWindow * >( parentWidget() );

    // change the tile of this parent window
    parent->setWindowTitle( text );
}

void QDetachableTabWidget::closeWidgetTabs()
{
    // GCORE-7644: As we delete a widget, there is a mechanism that
    // automatically removes it from the list. Everything works fine as long as
    // there is only one widget.
    // For instance, with 2 widgets tabs to close:
    // first iteration:
    //   - indexTab is 0 and count() is 2 => we delete the 1st widget
    // Second iteration:
    //   - indexTab is 1 and count() is & => WE EXIT THE LOOP WITHOUY DELETING THE 2ND WIDGET
    //
    // Old code was
    // for(int indexTab = 0; indexTab < count(); ++indexTab )

    // Use a while loop instead. As long as there are some widgets tabs remaining,
    // delete the first one in the list
    while(count() > 0)
    {
        // retrieve the first widget
        QWidget* widgetToDelete = widget(0);

        // As removeTab doesn't call the delete of the widget, call it now
        if(widgetToDelete)
        {
            delete widgetToDelete;
            widgetToDelete = 0;
        }
    }
}

void QDetachableTabWidget::currentTabChanged( int index )
{
    // acts just like if the user clicked on the tab this one replace in term of
    // position; only if not is locked state
    if( ! mTabBar->mIsLocked )
        mTabBar->mTargetedTabIndexWhenClick = index;
}

void QDetachableTabWidget::closeTab( int index )
{
    // stop edition if applicable
    mTabBar->stopEditing();

    // the source window
    QDetachableTabWindow *sourceWindow =
        static_cast< QDetachableTabWindow * >( parentWidget() );

    // retrieve the current widget
    QWidget* widgetToDelete = widget(index);

    // remove the targetd tab
    removeTab( index );

    // if the source window is empty, remove it
    if( sourceWindow->count() == 0 )
    {
        mWindowContainer->removeDetachableTabWindow( sourceWindow );
    }

    // As removeTab doesn't call the delete of the widget, call it now
    if(widgetToDelete)
    {
        //widgetToDelete->close();
        delete widgetToDelete;
        widgetToDelete = 0;
    }
}


bool QDetachableTabWidget::isTabBarLocked() const
{
    return mTabBar->mIsLocked;
}

void QDetachableTabWidget::focusTab( int index )
{
    QDetachableTabWindow *parent =
        static_cast< QDetachableTabWindow * >( parentWidget() );

    // delegate to parent's feature
    parent->focusTab( index );
}
