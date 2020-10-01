#ifndef PICKBINDIALOG_H
#define PICKBINDIALOG_H

#include "ui_pickbin_dialog.h"

class PickBinDialog : public QDialog, public Ui::PickBinDialogBase
{
	Q_OBJECT

public:
	PickBinDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	bool	setFile(QString strFile,bool bSoftBin);
	QString getBinsList(void);
private:
	bool	bGetSofBin;	// to hold type of bin to list (true=soft-bin, false=hard-bin)
};

#endif //PICKBINDIALOG_H
