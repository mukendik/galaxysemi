/****************************************************************************
** Form implementation generated from reading ui file '.\g-trigger_dialogbase.ui'
**
** Created: Fri Jan 23 14:16:31 2009
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "g-trigger_dialogbase.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlistbox.h>
#include <qtextbrowser.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>


/*
 *  Constructs a G_TriggerDialogBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
G_TriggerDialogBase::G_TriggerDialogBase( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "G_TriggerDialogBase" );
    G_TriggerDialogBaseLayout = new QVBoxLayout( this, 11, 6, "G_TriggerDialogBaseLayout"); 

    tabWidget = new QTabWidget( this, "tabWidget" );

    tabStatus = new QWidget( tabWidget, "tabStatus" );
    tabStatusLayout = new QVBoxLayout( tabStatus, 11, 6, "tabStatusLayout"); 

    listBox_Status = new QListBox( tabStatus, "listBox_Status" );
    tabStatusLayout->addWidget( listBox_Status );
    tabWidget->insertTab( tabStatus, QString::fromLatin1("") );

    tabSettings = new QWidget( tabWidget, "tabSettings" );
    tabSettingsLayout = new QVBoxLayout( tabSettings, 11, 6, "tabSettingsLayout"); 

    listBox_Settings = new QListBox( tabSettings, "listBox_Settings" );
    tabSettingsLayout->addWidget( listBox_Settings );
    tabWidget->insertTab( tabSettings, QString::fromLatin1("") );

    TabPage = new QWidget( tabWidget, "TabPage" );
    TabPageLayout = new QVBoxLayout( TabPage, 11, 6, "TabPageLayout"); 

    textBrowser = new QTextBrowser( TabPage, "textBrowser" );
    TabPageLayout->addWidget( textBrowser );
    tabWidget->insertTab( TabPage, QString::fromLatin1("") );
    G_TriggerDialogBaseLayout->addWidget( tabWidget );
    languageChange();
    resize( QSize(613, 350).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
G_TriggerDialogBase::~G_TriggerDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void G_TriggerDialogBase::languageChange()
{
    setCaption( tr( "G-Trigger" ) );
    tabWidget->changeTab( tabStatus, tr( "Status" ) );
    tabWidget->changeTab( tabSettings, tr( "Settings" ) );
    tabWidget->changeTab( TabPage, tr( "Release History" ) );
}

void G_TriggerDialogBase::OnCloseConnection()
{
    qWarning( "G_TriggerDialogBase::OnCloseConnection(): Not implemented yet" );
}

void G_TriggerDialogBase::OnConnect()
{
    qWarning( "G_TriggerDialogBase::OnConnect(): Not implemented yet" );
}

void G_TriggerDialogBase::OnTimerEvent()
{
    qWarning( "G_TriggerDialogBase::OnTimerEvent(): Not implemented yet" );
}

void G_TriggerDialogBase::socketConnected()
{
    qWarning( "G_TriggerDialogBase::socketConnected(): Not implemented yet" );
}

void G_TriggerDialogBase::socketConnectionClosed()
{
    qWarning( "G_TriggerDialogBase::socketConnectionClosed(): Not implemented yet" );
}

void G_TriggerDialogBase::socketError(int)
{
    qWarning( "G_TriggerDialogBase::socketError(int): Not implemented yet" );
}

void G_TriggerDialogBase::socketReadyRead()
{
    qWarning( "G_TriggerDialogBase::socketReadyRead(): Not implemented yet" );
}

