#include "QDetachableTabWindow.h"
#include "QDetachableTabWidget.h"
#include "QDetachableTabWindowContainer.h"

#include <QCloseEvent>

#include "QSnappedVBoxLayout.h"

/******************************************************************************!
 * \fn Constructor
 * \note Qt::WindowMinimizeButtonHint is for GCORE-6886
 ******************************************************************************/
QDetachableTabWindow::QDetachableTabWindow
    (QDetachableTabWindowContainer *windowContainer, QWidget *parent ) :
    QDialog(parent, Qt::WindowMinimizeButtonHint |  Qt::WindowCloseButtonHint),
    mWindowContainer( windowContainer ),
    mTabs( NULL ),
    mZLevel( std::size_t() )
{
    setupContent();
    installEventFilter(this);
    mDoNotClose = false;
}

QDetachableTabWindow::~QDetachableTabWindow()
{

}

bool QDetachableTabWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (  obj == this  && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
        mDoNotClose = true;
        return true;
    } else {
        //return false;
        // standard event processing
        //mDoNotClose = false;
        return QDialog::eventFilter(obj, event);
    }
}

void QDetachableTabWindow::setupContent()
{
    // this new window have the maximum z-level
    mZLevel = mWindowContainer->windows_.size();

    // adding an extended tab widget here
    mTabs = new QDetachableTabWidget( mWindowContainer );

    // setting up a vertical layout
    setLayout( new QSnappedVBoxLayout() );

    // adding the custom tab widget
    layout()->addWidget( mTabs );
}

void QDetachableTabWindow::focusInEvent(QFocusEvent*)
{
    mTabs->currentWidget()->setFocus();
}

bool QDetachableTabWindow::containsWidget(QWidget *widget)
{
    if( widget != NULL )
    {
        return (mTabs->indexOf(widget) != -1);
    }
    return false;
}

void QDetachableTabWindow::focusOnTab( QWidget *widget)
{
    if( widget != NULL )
    {
       int index = mTabs->indexOf(widget);
       if( index != -1)
        focusTab( index );
    }
}

int QDetachableTabWindow::addTab( QWidget *widget, const QString &title )
{
    // adding content if it exists
    if( widget != NULL )
    {        
        int index = mTabs->addTab( widget, title );
        focusTab( index );

        return index;
    }

    // problem here, no tab added, return -1
    return -1;
}

void QDetachableTabWindow::focusTab( int index )
{
    // delegate this to the tab widget
    mTabs->setCurrentIndex( index );

    // focus on the widget displayed
    mTabs->currentWidget()->setFocus();

    // modify z level of windows, giving this the max z-level
    mWindowContainer->setMaxZLevelFor( this );
}

int QDetachableTabWindow::count() const
{
    // simple proxy on method existing in custom tab widget
    return mTabs->count();
}

void QDetachableTabWindow::processCloseEvent( QCloseEvent * )
{
    // close all the widget inside each tab
    mTabs->closeWidgetTabs();

    // remove this window from main windows's container
    mWindowContainer->removeDetachableTabWindow( this );
}

void QDetachableTabWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
         e->ignore();
        break;
    default:
        QDialog::keyPressEvent(e);
        break;
    }
}

void QDetachableTabWindow::closeEvent( QCloseEvent *event )
{
    //event->accept();
    if(mDoNotClose == false)
    {
        processCloseEvent( event );
        QDialog::closeEvent( event );
    }
    else
    {
        //-- let the child decide what to do with it
        event->ignore();
        mDoNotClose =false;
    }
}

void QDetachableTabWindow::processMousePressEvent( QMouseEvent *event )
{
    // this windows gains the highest z-level
    if( event->button() == Qt::LeftButton )
        mWindowContainer->setMaxZLevelFor( this );
}

void QDetachableTabWindow::mousePressEvent( QMouseEvent *event )
{
    processMousePressEvent( event );
    QDialog::mousePressEvent( event );
}

void QDetachableTabWindow::replaceWidget(QWidget*  previousWidget, QWidget* newWidget, const QString &label)
{
    if( newWidget != NULL )
    {
       int index = mTabs->indexOf(previousWidget);
       if( index != -1)
       {
           mTabs->removeTab(index);
           mTabs->insertTab(index, newWidget, label);
           focusTab( index );
       }
    }
}
