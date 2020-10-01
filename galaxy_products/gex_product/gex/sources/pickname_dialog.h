#ifndef PICKNAMEDIALOG_H
#define PICKNAMEDIALOG_H

#include "ui_pickname_dialog.h"

/////////////////////////////////////////////////////////////////////////////
class PickNameDialog : public QDialog, public Ui::PickNameDialogBase
{
	Q_OBJECT
		
public:
	PickNameDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );

	void clear(void);
	void insert(QString strLegend,QString strSuggestion);
	bool isEmpty(void);
	bool isAppendMode(void);
	QString nameList(void);
};
#endif
