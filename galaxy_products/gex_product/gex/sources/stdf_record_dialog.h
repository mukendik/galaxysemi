/****************************************************************************
** Form interface generated from reading ui file 'stdf_record_dialog_base.ui'
**
** Created: Wed Mar 3 12:44:40 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.3   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef STDF_RECORD_DIALOG_H
#define STDF_RECORD_DIALOG_H

#include "ui_stdf_record_dialog.h"

namespace GQTL_STDF
{
    class Stdf_Record;
}

class StdfRecordDialog : public QDialog, public Ui::StdfRecordDialog_base
{
	Q_OBJECT
		
public:
    StdfRecordDialog( GQTL_STDF::Stdf_Record* pclStdfRecord, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~StdfRecordDialog();
};

#endif // STDF_RECORD_DIALOG_H
