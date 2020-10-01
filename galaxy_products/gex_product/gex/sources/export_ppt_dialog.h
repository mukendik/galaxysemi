/****************************************************************************
** Derived from export_ppt_dialog_base.h
****************************************************************************/

#ifndef EXPORT_PPT_DIALOG_H
#define EXPORT_PPT_DIALOG_H

#include "ui_export_ppt_dialog.h"

class ExportPptDialog : public QDialog, public Ui::ExportPptDialog_base
{
	Q_OBJECT

public:
    ExportPptDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~ExportPptDialog();


private:

private slots:
    //! \brief Button to select a HTML files to convert has been clicked
    void OnButtonSelect();
    void OnButtonProperties();
    void OnButtonExport();
    void OnHtmlFileSelected();
	
private:
	QString		m_strHtmlFileName;

signals:

	void sHtmlFileSelected(const QString&);
};

#endif // EXPORT_PPT_DIALOG_H
