/****************************************************************************
** Form interface generated from reading ui file 'export_xml_dialog_base.ui'
**
** Created: Wed Mar 3 12:44:40 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.3   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef EXPORT_XML_DIALOG_H
#define EXPORT_XML_DIALOG_H

#include "ui_export_xml_dialog.h"
#include "stdfparse.h"
#include "stdfrecords_v4.h"

class ExportXmlDialog : public QDialog, public Ui::ExportXmlDialog_base
{
	Q_OBJECT
		
public:
    ExportXmlDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~ExportXmlDialog();


private:
    GQTL_STDF::Stdf_MIR_V4	m_clMIR;

private slots:
    //! \brief Button to select a STDF files to convert has been clicked
    void OnButtonSelect();
    void OnButtonProperties();
    void OnButtonExport();
    void OnStdfFileSelected();
	
private:
	QString		m_strStdfFileName;

signals:
	
	void sStdfFileSelected(const QString&);
};

#endif // EXPORT_XML_DIALOG_H
