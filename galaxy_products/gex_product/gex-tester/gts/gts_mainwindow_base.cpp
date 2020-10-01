/****************************************************************************
** Form implementation generated from reading ui file 'gts_mainwindow_base.ui'
**
** Created: Mon Feb 20 17:52:15 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "gts_mainwindow_base.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>

#include "gts_mainwindow_base.ui.h"

/*
 *  Constructs a Gts_MainWindow_base as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
Gts_MainWindow_base::Gts_MainWindow_base( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    (void)statusBar();
    if ( !name )
	setName( "Gts_MainWindow_base" );
    setPaletteBackgroundColor( QColor( 255, 255, 204 ) );
    setIcon( QPixmap::fromMimeSource( "galaxy_logo_32.png" ) );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );
    Gts_MainWindow_baseLayout = new QVBoxLayout( centralWidget(), 11, 6, "Gts_MainWindow_baseLayout"); 

    layout41 = new QHBoxLayout( 0, 0, 6, "layout41"); 

    buttonNewStation = new QPushButton( centralWidget(), "buttonNewStation" );
    buttonNewStation->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, buttonNewStation->sizePolicy().hasHeightForWidth() ) );
    buttonNewStation->setMinimumSize( QSize( 100, 50 ) );
    buttonNewStation->setMaximumSize( QSize( 100, 50 ) );
    buttonNewStation->setIconSet( QIconSet( QPixmap::fromMimeSource( "new_station.png" ) ) );
    layout41->addWidget( buttonNewStation );

    buttonExit = new QPushButton( centralWidget(), "buttonExit" );
    buttonExit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, buttonExit->sizePolicy().hasHeightForWidth() ) );
    buttonExit->setMinimumSize( QSize( 100, 50 ) );
    buttonExit->setMaximumSize( QSize( 100, 50 ) );
    buttonExit->setIconSet( QIconSet( QPixmap::fromMimeSource( "exit.png" ) ) );
    layout41->addWidget( buttonExit );
    spacer1 = new QSpacerItem( 265, 16, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout41->addItem( spacer1 );
    Gts_MainWindow_baseLayout->addLayout( layout41 );

    listViewStations = new QListView( centralWidget(), "listViewStations" );
    listViewStations->addColumn( tr( "Station" ) );
    listViewStations->addColumn( tr( "Test Program" ) );
    listViewStations->addColumn( tr( "Parts tested" ) );
    listViewStations->addColumn( tr( "Yield" ) );
    listViewStations->setMinimumSize( QSize( 500, 80 ) );
    Gts_MainWindow_baseLayout->addWidget( listViewStations );

    // actions
    NewStationAction = new QAction( this, "NewStationAction" );
    NewStationAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "new_station.png" ) ) );
    ExitAction = new QAction( this, "ExitAction" );
    ExitAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "exit.png" ) ) );
    AboutAction = new QAction( this, "AboutAction" );
    AboutAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "galaxy_logo_22.png" ) ) );


    // toolbars


    // menubar
    MenuBar = new QMenuBar( this, "MenuBar" );


    File = new QPopupMenu( this );
    NewStationAction->addTo( File );
    File->insertSeparator();
    ExitAction->addTo( File );
    MenuBar->insertItem( QString(""), File, 1 );

    Help = new QPopupMenu( this );
    AboutAction->addTo( Help );
    MenuBar->insertItem( QString(""), Help, 2 );

    languageChange();
    resize( QSize(522, 239).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonNewStation, SIGNAL( clicked() ), this, SLOT( OnNewStation() ) );
    connect( buttonExit, SIGNAL( clicked() ), this, SLOT( OnExit() ) );
    connect( AboutAction, SIGNAL( activated() ), this, SLOT( OnAbout() ) );
    connect( ExitAction, SIGNAL( activated() ), this, SLOT( OnExit() ) );
    connect( NewStationAction, SIGNAL( activated() ), this, SLOT( OnNewStation() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
Gts_MainWindow_base::~Gts_MainWindow_base()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void Gts_MainWindow_base::languageChange()
{
    setCaption( tr( "GTS - Galaxy Tester Simulator" ) );
    buttonNewStation->setText( tr( "New Station" ) );
    QToolTip::add( buttonNewStation, tr( "Open a new station window" ) );
    buttonExit->setText( tr( "Exit" ) );
    listViewStations->header()->setLabel( 0, tr( "Station" ) );
    listViewStations->header()->setLabel( 1, tr( "Test Program" ) );
    listViewStations->header()->setLabel( 2, tr( "Parts tested" ) );
    listViewStations->header()->setLabel( 3, tr( "Yield" ) );
    NewStationAction->setText( tr( "New Station" ) );
    NewStationAction->setMenuText( tr( "&New Station" ) );
    NewStationAction->setAccel( tr( "Ctrl+N" ) );
    ExitAction->setText( tr( "Exit" ) );
    ExitAction->setMenuText( tr( "E&xit" ) );
    ExitAction->setAccel( tr( "Alt+X" ) );
    AboutAction->setText( tr( "About" ) );
    AboutAction->setMenuText( tr( "&About" ) );
    AboutAction->setAccel( tr( "Ctrl+A" ) );
    if (MenuBar->findItem(1))
        MenuBar->findItem(1)->setText( tr( "&File" ) );
    if (MenuBar->findItem(2))
        MenuBar->findItem(2)->setText( tr( "&Help" ) );
}

