#ifndef PICKCOMBODIALOG_H
#define PICKCOMBODIALOG_H
#include "ui_pickcombo_dialog.h"

class PickComboDialog : public QDialog, public Ui::PickComboDialogBase
{
	Q_OBJECT 

public:

	PickComboDialog(QWidget * pParent = 0, Qt::WindowFlags fl = 0);

	void	setText(QString strTitle,QString strText1,QString strText2,QString strCheckBox);
	void	InsertItem(int iComboID,QString strEntry,bool bReset=false);
	void	CheckBox(bool bCheckBox);
	int		currentItem(int iComboID);
	bool	isChecked(void);
};

#endif // PICKCOMBODIALOG_H
