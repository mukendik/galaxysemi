/****************************************************************************
** Deriven from gexftp_passwordconfirmation_dialog_base.h
****************************************************************************/

#ifndef CGexFtpPasswordConfirmation_dialog_H
#define CGexFtpPasswordConfirmation_dialog_H

#include "ui_gexftp_passwordconfirmation_dialog_base.h"

class CGexFtpPasswordConfirmation_dialog : public QDialog, public Ui::CGexFtpPasswordConfirmation_dialog_base
{
public:
    CGexFtpPasswordConfirmation_dialog( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~CGexFtpPasswordConfirmation_dialog();

	QString GetPassword(void);
};

#endif // CGexFtpPasswordConfirmation_dialog_H
