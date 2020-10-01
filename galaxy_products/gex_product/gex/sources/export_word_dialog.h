/****************************************************************************
** Derived from export_word_dialog_base.h
****************************************************************************/

#ifndef EXPORT_WORD_DIALOG_H
#define EXPORT_WORD_DIALOG_H

#include "ui_export_word_dialog.h"

class ExportWordDialog : public QDialog, public Ui::ExportWordDialog_base
{
	Q_OBJECT

public:
    ExportWordDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~ExportWordDialog();


private:

private slots:
    //! \brief Button to select a HTML files to convert has been clicked
    void OnButtonSelect();
    void OnButtonProperties();
    void OnButtonExport();
    void OnHtmlFileSelected();
	
private:
	QValidator*	m_pMarginValidator;
	QString		m_strHtmlFileName;

signals:

	void sHtmlFileSelected(const QString&);
};

#endif // EXPORT_WORD_DIALOG_H
