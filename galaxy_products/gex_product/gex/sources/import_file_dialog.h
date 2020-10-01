/****************************************************************************
** Form interface generated from reading ui file 'import_file_dialog_base.ui'
**
** Created: Wed Mar 3 12:44:40 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.3   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef IMPORT_FILE_DIALOG_H
#define IMPORT_FILE_DIALOG_H

#include "ui_import_file_dialog.h"

class ImportFileDialog : public QDialog, public Ui::ImportFileDialog_base
{
	Q_OBJECT
		
public:
    ImportFileDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~ImportFileDialog();

	// Overwrite virtual fcts from main class for Drag&Drop support.
	void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
	void OnSelectFile(QString strFileName);

private slots:
    void OnButtonSelect();
    void OnButtonImport();
    void OnFileSelected();
	
private:
	QString		m_strFileName;

signals:

	void sFileSelected(const QString&);
};

#endif // IMPORT_FILE_DIALOG_H
