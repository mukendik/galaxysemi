/****************************************************************************
** Derived from processoutput_dialog_base.h
****************************************************************************/

#ifndef PROCESSOUTPUT_DIALOG_H
#define PROCESSOUTPUT_DIALOG_H

#include "ui_process_output_dialog.h"

class ProcessOutputDialog : public QDialog, public Ui::ProcessOutputDialog_base
{
	Q_OBJECT

public:
    ProcessOutputDialog( const QString& strCaption, const QString& strCommandLine, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~ProcessOutputDialog();

private:

private slots:
    void OnStdoutChanged(const QString & strStdout);
    void OnStderrChanged(const QString & strStderr);
	
private:
};

#endif // PROCESSOUTPUT_DIALOG_H
