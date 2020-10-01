///////////////////////////////////////////////////////////
// All classes used for ToolBox 'Merge Test & Retest'
///////////////////////////////////////////////////////////

#ifndef TB_MERGE_RETEST_DIALOG_H
#define TB_MERGE_RETEST_DIALOG_H
#include "ui_tb_merge_retest_dialog.h"
#include "stdf.h"
#include "stdfrecords_v4.h"

class GexTbMergeRetest;

class GexTbMergeRetestDialog : public QDialog, public Ui::MergeRetestDialogBase
{
	Q_OBJECT
		
public:
	GexTbMergeRetestDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
	~GexTbMergeRetestDialog();

    int     MergeFiles(QStringList strSources,QString strOutput,QString &strErrorMessage);
    int     MergeSamplesFiles(QStringList strSources,QString strOutput,bool bRebuildHbrSbr,QString &strErrorMessage);

    // Overwrite virtual fcts from main class for Drag&Drop support.
	void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
	void	OnAddTestFileList(QStringList &strFiles);
	void	OnAddRetestFileList(QStringList &strFiles);
	QStringList	getTestFileList(bool bTestList/*=true*/);

public slots:
	void	OnAddTestFile();
	void	OnMoveUpTestFile();
	void	OnMoveDownTestFile();
	void	OnAddRetestFile();
	void	OnMoveUpRetestFile();
	void	OnMoveDownRetestFile();
	void	OnDeleteTestFile();
	void	OnDeleteRetestFile();
	void	OnDeleteAllTestFiles();
	void	OnDeleteAllRetestFiles();
	void	OnMergeFiles();
protected:
    QString strOutputFile;
    GexTbMergeRetest *m_poMerge;
};

#endif
