#include "QDetachableTabWindowContainer.h"
#include "ui_QDetachableTabWindowContainer.h"

#include "QDetachableTabWindow.h"
#include "QSnappedVBoxLayout.h"
#include "QDetachableTabWidget.h"
#include "WindowsZLevelCompare.h"

QDetachableTabWindowContainer::QDetachableTabWindowContainer
    ( QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::QDetachableTabWindowContainer)
{
    ui->setupUi(this);

    // vertical layout here
    setLayout( new QSnappedVBoxLayout () );

    createInitialWindow();
    firstWindowIsEmpty_ = false;
}

void QDetachableTabWindowContainer::createInitialWindow()
{
    // this windows gains the highest z-level
    QDetachableTabWindow *firstWindow =
        new QDetachableTabWindow( this );

    // store it in the container. MUST be in index 0
    windows_.insert(0, firstWindow);
    mainWindow_ = firstWindow;

    setMaxZLevelFor( firstWindow );

    // adding it to layout
    layout()->addWidget( activeWindow() );
}

QDetachableTabWindowContainer::~QDetachableTabWindowContainer()
{
    QList< QDetachableTabWindow * >::iterator lIterBegin(windows_.begin()), lIterEnd(windows_.end());
    for(; lIterBegin != lIterEnd; ++lIterBegin)
        (*lIterBegin)->close();

    // Qt resource management
    delete ui;
}

QDetachableTabWindow * QDetachableTabWindowContainer::getDetachableTabWindowAt
    ( const QPoint &position )
{
    // container of found windows at specfied position
    std::vector< QDetachableTabWindow * > windowsFound;
    windowsFound.reserve( windows_.count() );

    // prepare iteration in the collection of untabbed widgets
    QList< QDetachableTabWindow * >::iterator it = windows_.begin();

    for( ; it != windows_.end(); ++it )
    {
        // currently explore window from the container
        QDetachableTabWindow *window = *it;

        // check position regarding position and size of currently explored
        // window
        QPoint windowTopLeft = window->mapToGlobal( QPoint( 0, 0 ) );
        QPoint windowBottomRight =
            window->mapToGlobal
            (
                QPoint
                    (
                      window->geometry().width(),
                      window->geometry().height()
                    )
            );

        // this is the rectangular area occupied by the currently explore window
        QRect windowRect( windowTopLeft, windowBottomRight );

        // if mouse's cursor is in this area, a window is found. It could
        // trigger a bad behavior if several windows are overlapping
        if( windowRect.contains( position ) )
            windowsFound.push_back( *it );
    }

    // no window found end here
    if( windowsFound.empty() )
        return NULL;

    // take the window having the highest z-index
    std::vector< QDetachableTabWindow * >::iterator activeWindowIt =
        std::max_element
        (
            windowsFound.begin(), windowsFound.end(),
            WindowsZLevelCompare()
        );

    // if nothing is found, choose the first window in the vector
    return *activeWindowIt;
}

void QDetachableTabWindowContainer::removeTabWindowContaining
    ( QWidget *widget )
{
     QList< QDetachableTabWindow * >::iterator lIterBegin(windows_.begin()),
         lIterEnd(windows_.end());

     for( ; lIterBegin != lIterEnd; ++lIterBegin )
     {
           QDetachableTabWidget *lTabsWidget =
               static_cast< QDetachableTabWidget * >( ( *lIterBegin )->tabs() );

           // retrive the index if exist
           int lIndex = lTabsWidget->indexOf(widget) ;

           // if exist, remove it
           if(lIndex != -1)
               lTabsWidget->closeTab(lIndex);
     }

}

void QDetachableTabWindowContainer::removeDetachableTabWindow
    ( QDetachableTabWindow *widget )
{
    // try to find the specified window in the collection
    int index = windows_.indexOf( widget );

    // a window is found
    if( index != -1 )
    {
        // allow Qt to manage this resource
        widget->forceClose();
        widget->close();

       // int beforeRemove = windows_.count();
        // remove the sole instance of this window. Poor Qt's interface
        windows_.removeOne( widget );

        refreshZLevel();

        // the first window has been removed
        //if(index == 0 && windows_.count() != beforeRemove)
        if(mainWindow_ == widget)
        {
            firstWindowIsEmpty_ = true;
        }

        // re create initial window if it doesn't remain any one
        if( windows_.size() == 0 )
        {
            createInitialWindow();
        }
    }
}


void QDetachableTabWindowContainer::addDetachableTabWindow
    ( QDetachableTabWindow *widget )
{
    // adding a true window. NULL is not allowed
    if( widget != NULL )
    {
        windows_.push_back( widget );
        setMaxZLevelFor(widget);
    }
}

void QDetachableTabWindowContainer::replaceWidget
  ( QWidget *previousWidget, QWidget *newWidget, const QString& label )
{
    QList< QDetachableTabWindow * >::iterator itBegin(windows_.begin()),
        itEnd( windows_.end());

    for( ; itBegin != itEnd; ++itBegin )
    {
        if( ( *itBegin )->containsWidget( previousWidget ) )
        {
            ( *itBegin )->replaceWidget( previousWidget, newWidget, label );
        }
    }
}

void QDetachableTabWindowContainer::focusOnTab( QWidget *widget)
{
   QList< QDetachableTabWindow * >::iterator itBegin(windows_.begin()),
       itEnd( windows_.end());

   for( ; itBegin != itEnd; ++itBegin )
   {
       if( ( *itBegin )->containsWidget( widget ) )
       {
           ( *itBegin )->focusOnTab( widget );
           ( *itBegin )->activateWindow();
           ( *itBegin )->raise();
           break;
       }
   }
}


void QDetachableTabWindowContainer::addDetachableTabInActiveWindow
    ( QWidget *widget, const QString &title )
{
    // only left unpinned window.
    // the new one must be in the main window that have to be created
    if(empty() == false && firstWindowIsEmpty_ == true)
    {
        createInitialWindow();
    }

    firstWindowIsEmpty_ = false;
    // working with the first window of the container
    activeWindow()->addTab( widget, title );
}

QPoint QDetachableTabWindowContainer::fixPosition
    ( const QPoint *position ) const
{
    // if a position is provided, it is considered as correct, otherwise,
    // return this widget position
    return
        ( position == NULL ) ?
        mapToGlobal( pos() ) :
        *position;
}

void QDetachableTabWindowContainer::addDetachedWindowAt
    ( QWidget *widget, const QString &title, const QPoint *position )
{
    // process the provided position to fix it if needed
    QPoint fixedPosition = fixPosition( position );

    // create an external window
    QDetachableTabWindow *externalWindow =
        new QDetachableTabWindow( this );

    externalWindow->setParent( this, Qt::Window );

    // set title and position of the new window
    externalWindow->setWindowTitle( title );
    externalWindow->setGeometry
        ( fixedPosition.x(), fixedPosition.y(), 0, 0 );

    // adding the content of the tab inside this window
    externalWindow->addTab( widget, title );

    // show as external window
    externalWindow->adjustSize();
    externalWindow->show();

    // add this external window to parent's collection
    addDetachableTabWindow( externalWindow );

    // this external window has the highest z-level
    setMaxZLevelFor( externalWindow );
    externalWindow->raise();
}

void QDetachableTabWindowContainer::changeWindowTitleByContent
    ( QWidget *containedWidget, const QString &title )
{
    // nothing todo if specified widget is null
    if( containedWidget == NULL )
        return;

    // iterator on the beginning of the collection
    QList< QDetachableTabWindow * >::iterator it =
        windows_.begin();

    for( ;it != windows_.end(); ++it )
    {
        // caught a window, look in its tab now
        QDetachableTabWindow *window = *it;
        const QDetachableTabWidget *tabs = window->tabs();

        for( int i = 0; i < tabs->count(); ++i )
        {
            // widget found in the tab, cool
            if( tabs->widget( i ) == containedWidget )
            {
                window->setWindowTitle( title );
                return;
            }
        }
    }
}

void QDetachableTabWindowContainer::changeTabTitleByContent
    ( QWidget *containedWidget, const QString &title )
{
    // nothing todo if specified widget is null
    if( containedWidget == NULL )
        return;

    // iterator on the beginning of the collection
    QList< QDetachableTabWindow * >::iterator it =
        windows_.begin();

    for( ;it != windows_.end(); ++it )
    {
        // caught a window, look in its tab now
        QDetachableTabWindow *window = *it;
        QDetachableTabWidget *tabs = window->tabs();

        for( int i = 0; i < tabs->count(); ++i )
        {
            // widget found in the tab, cool
            if( tabs->widget( i ) == containedWidget )
            {
                tabs->setTabText( i, title );
                return;
            }
        }
    }
}

bool QDetachableTabWindowContainer::empty()
{
    QList< QDetachableTabWindow * >::iterator it =
        windows_.begin();

    for( ;it != windows_.end(); ++it )
    {
        if((*it)->mTabs->count())
            return false;
    }
    return true;
}

QDetachableTabWindow * QDetachableTabWindowContainer::activeWindow()
{
    // look for the window having the highest zlevel according to the compare
    // type behavior
    QList< QDetachableTabWindow * >::iterator it =
        std::max_element
        ( windows_.begin(), windows_.end(), WindowsZLevelCompare() );

    // return window found
    return *it;
}

void QDetachableTabWindowContainer::refreshZLevel()
{
    // sort windows by z-level
    std::sort
        ( windows_.begin(), windows_.end(), WindowsZLevelCompare() );

    // shrink z-level values to fit the window count in the container
    int i = 0;
    QList< QDetachableTabWindow * >::iterator it = windows_.begin();

    for( ; it != windows_.end(); ++it, ++i )
    {
        QDetachableTabWindow *window = *it;
        window->mZLevel = i;
    }
}

void QDetachableTabWindowContainer::setMaxZLevelFor
    ( QDetachableTabWindow *window )
{
    // get the current z-level of specified window
    std::size_t currentZLevel = window->mZLevel;

    // decrease z-level of all window whose z-level is greater than current
    // z-level
    QList< QDetachableTabWindow * >::iterator it = windows_.begin();

    for( ; it != windows_.end(); ++it )
    {
        if( ( *it )->mZLevel > currentZLevel )
            ( *it )->mZLevel--;
    }

    // set the max zlevel for the specified window : the number of existing
    // windows minus 1
    window->mZLevel = windows_.size() - 1;

    // give the specified window the focus
    window->setWindowState( Qt::WindowActive );
}
