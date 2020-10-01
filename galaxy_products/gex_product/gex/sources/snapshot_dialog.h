#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "ui_snapshot_dialog.h"
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////

class SnapshotDialog : public QDialog, public Ui::snapshot_basedialog
{
	Q_OBJECT
		
public:
	SnapshotDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
};

class SnapShotReport
{
public:
	int CreateHomePage(void);
	int AddLink(QString strLinkName,QTextEdit* ptNotes);
	FILE *AddPage(QString strLinkName);
	void ClosePage(QTextEdit* ptNotes);

	int	iSnapShotPageID;

private:
	FILE *hSnapShotReport;
};

#endif
