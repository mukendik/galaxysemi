#ifndef PICKPRODUCT_ID_DIALOG_H
#define PICKPRODUCT_ID_DIALOG_H

#include "ui_pick_product_id_dialog.h"

class GexDatabaseEntry;

/////////////////////////////////////////////////////////////////////////////
class PickProductIdDialog : public QDialog, public Ui::PickProductIdDialogBase
{
	Q_OBJECT
		
public:
	PickProductIdDialog( QWidget* parent = 0, GexDatabaseEntry* pDatabaseEntry = NULL,bool modal = false, Qt::WindowFlags f = 0 );
	QString getSelection(void);
};

#endif // PICKPRODUCT_ID_DIALOG_H
