#ifndef _DB_One_Query_Dialog_h_
#define _DB_One_Query_Dialog_h_

#include "browser_dialog.h"
#include "db_single_query_filterform_dialog.h"

///////////////////////////////////////////////////////////
// Constructor
GexOneQueryFilterFormDialog::GexOneQueryFilterFormDialog( QWidget* parent, bool modal, Qt::WindowFlags f ) 
	: QDialog( parent, f )
{
	setupUi(this);
	setModal(modal);

	QObject::connect(pushButtonClose, SIGNAL(clicked()), this, SLOT(accept()));
}

#endif // _DB_One_Query_Dialog_h_
