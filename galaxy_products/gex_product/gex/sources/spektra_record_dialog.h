/****************************************************************************
** Form interface generated from reading ui file 'stdf_record_dialog_base.ui'
**
** Created: Wed Mar 3 12:44:40 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.3   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef SPEKTRA_RECORD_DIALOG_H
#define SPEKTRA_RECORD_DIALOG_H

#include "ui_spektra_record_dialog.h"

class CSpektra_Record_V3;

class SpektraRecordDialog : public QDialog, public Ui::SpektraRecordDialog_base
{
	Q_OBJECT

public:
    SpektraRecordDialog( CSpektra_Record_V3* pclSpektraRecord, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~SpektraRecordDialog();
};

#endif // SPEKTRA_RECORD_DIALOG_H
