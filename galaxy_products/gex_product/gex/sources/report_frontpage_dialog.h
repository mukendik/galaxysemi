#ifndef REPORT_FRONTPAGE_DIALOG_H
#define REPORT_FRONTPAGE_DIALOG_H

#include "ui_report_frontpage_dialog.h"

/////////////////////////////////////////////////////////////////////////////
class FrontPageDialog : public QDialog, public Ui::FrontPageDialogBase
{
	Q_OBJECT

public:
	FrontPageDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	void SetSelection(QString strFrontPageText,QString strFrontPageImage);
	void GetSelection(QString &strFrontPageText,QString &strFrontPageImage);

private:
	QString	strImagePath;

public slots:
    void OnEditText(int,int);
	void OnSelectLogo();
	void OnRemoveLogo();
};

#endif // REPORT_FRONTPAGE_DIALOG_H
