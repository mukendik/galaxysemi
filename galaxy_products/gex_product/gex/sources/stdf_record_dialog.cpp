#undef QT3_SUPPORT

#include "stdf_record_dialog.h"
#include "stdfparse.h"
#include "browser_dialog.h"

StdfRecordDialog::StdfRecordDialog( GQTL_STDF::Stdf_Record* pclStdfRecord, QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));

	QStringList		listFields;
  //QTreeWidgetItem* pTreeWidgetItem;

	// Set column width
	treeWidgetFields->setColumnWidth(0, 200);

	// Get fields
	pclStdfRecord->GetAsciiFieldList(listFields);

	// Set dialog caption
	QString strCaption = tr( "STDF record viewer: " );
	strCaption += pclStdfRecord->GetRecordLongName();
    setWindowTitle(strCaption);

	// Disable sorting on field listview
	treeWidgetFields->setSortingEnabled(false);

	// Fill list view
	for ( QStringList::Iterator it = listFields.begin(); it != listFields.end(); ++it ) 
        /*pTreeWidgetItem = */new QTreeWidgetItem(treeWidgetFields, (*it).split(';', QString::SkipEmptyParts));
}

/*
 *  Destroys the object and frees any allocated resources
 */
StdfRecordDialog::~StdfRecordDialog()
{
}

