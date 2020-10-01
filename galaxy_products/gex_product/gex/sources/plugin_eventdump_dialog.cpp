// Plugin Event content viewer
#include "plugin_eventdump_dialog.h"
#include "browser_dialog.h"

PluginEventDumpDialog::PluginEventDumpDialog( const QString& strEventName, const QString& strEventDump, QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);
	
	QObject::connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
	
    QStringList listEventDump = strEventDump.split(';');

	// Set event name field
	editEventName->setText(strEventName);
	
	// Fill Event dump view
	// Set column width
	treeWidgetEventDump->setColumnWidth(0, 120);
	treeWidgetEventDump->setColumnWidth(1, 400);

	// Disable sorting on field listview
	treeWidgetEventDump->setSortingEnabled(false);

	// Fill tree widget
	for ( QStringList::Iterator it = listEventDump.begin(); it != listEventDump.end(); ++it ) 
        new QTreeWidgetItem(treeWidgetEventDump,(*it).split('='));
}

/*
 *  Destroys the object and frees any allocated resources
 */
PluginEventDumpDialog::~PluginEventDumpDialog()
{
}
