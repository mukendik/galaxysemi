/****************************************************************************
** Form interface generated from reading ui file 'import_spektra_dialog_base.ui'
**
** Created: Wed Mar 3 12:44:40 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.3   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef IMPORT_SPEKTRA_DIALOG_H
#define IMPORT_SPEKTRA_DIALOG_H

#include "ui_import_spektra_dialog.h"
#include "cspektraparse_v3.h"

class CSPEKTRAtoASCII;

class ImportSpektraDialog : public QDialog, public Ui::ImportSpektraDialog_base
{
	Q_OBJECT

public:
    ImportSpektraDialog( bool bEvaluationMode, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~ImportSpektraDialog();

	// Overwrite virtual fcts from main class for Drag&Drop support.
	void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
	CSpektra_Label_V3	m_clLabel;

private:
	void OnSelectFile(QString strFileName);
	void SetRecordsToProcess(CSPEKTRAtoASCII & clSpektraToAscii);
	void ExportToStdf();
	void ExportToAscii();

private slots:
    void OnButtonSelect();
    void OnButtonProperties();
    void OnButtonExport();
    void OnSpektraFileSelected();
    void OnButtonSetAllRecords();
    void OnButtonClearAllRecords();
    void OnOutputFormatChanged(int nItem);
	
private:
	QString		m_strSpektraFileName;
	bool		m_bEvaluationMode;

signals:
	
	void		sSpektraFileSelected(const QString&);
};

#endif // IMPORT_SPEKTRA_DIALOG_H
