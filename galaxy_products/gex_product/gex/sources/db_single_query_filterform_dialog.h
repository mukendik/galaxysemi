#ifndef GEX_DB_SINGLE_QUERY_FILTERFORM_H
#define GEX_DB_SINGLE_QUERY_FILTERFORM_H

#include "ui_db_single_query_filterform_dialog.h"

/////////////////////////////////////////////////////////////////////////////
class GexOneQueryFilterFormDialog : public QDialog, public Ui::onequery_filterform_basedialog
{
	Q_OBJECT

public:
	
	GexOneQueryFilterFormDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
};

#endif
