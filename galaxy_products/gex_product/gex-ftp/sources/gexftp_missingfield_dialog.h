/****************************************************************************
** Deriven from gexftp_missingfield_dialog_base.h
****************************************************************************/

#ifndef CGexFtpMissingField_dialog_H
#define CGexFtpMissingField_dialog_H

#include "ui_gexftp_missingfield_dialog_base.h"

class CGexFtpMissingField_dialog : public QDialog, public Ui::CGexFtpMissingFieldDialogBase
{
public:
    CGexFtpMissingField_dialog( QString strFieldName = "", QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~CGexFtpMissingField_dialog();

	void SetFieldName(const QString& strFieldName) { labelField->setText(strFieldName); }
};

#endif // CGexFtpMissingField_dialog_H
