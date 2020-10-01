#undef QT3_SUPPORT

#include "spektra_record_dialog.h"
#include "cspektrarecords_v3.h"
#include "browser_dialog.h"

SpektraRecordDialog::SpektraRecordDialog( CSpektra_Record_V3* pclSpektraRecord, QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));

	QStringList		listFields;

	// Set column width
	treeWidgetFields->setColumnWidth(0, 200);

	// Get fields
	pclSpektraRecord->GetAsciiFieldList(listFields);

	// Set dialog caption
	QString strCaption = tr( "SPEKTRA record viewer: " );
	strCaption += pclSpektraRecord->GetRecordLongName();
    setWindowTitle(strCaption);

	// Disable sorting on field listview
	treeWidgetFields->setSortingEnabled(false);

	// Fill tree widget
	for ( QStringList::Iterator it = listFields.begin(); it != listFields.end(); ++it )
        new QTreeWidgetItem(treeWidgetFields, (*it).split(';', QString::SkipEmptyParts));
}

/*
 *  Destroys the object and frees any allocated resources
 */
SpektraRecordDialog::~SpektraRecordDialog()
{
}

