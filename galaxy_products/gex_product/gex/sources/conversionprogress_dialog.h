/****************************************************************************
** Derived from conversionprogress_dialog_base.h
****************************************************************************/

#ifndef CONVERSIONPROGRESS_DIALOG_H
#define CONVERSIONPROGRESS_DIALOG_H

#include "ui_conversionprogress_dialog.h"

class ConversionProgressDialog : public QDialog, public Ui::ConversionProgressDialog_base
{
	Q_OBJECT

public:
    ConversionProgressDialog( const QString& strCaption, const QString& strTitle, const QString& strAnimationFile, const QString& strInputFile, const QString& strOutputFile, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~ConversionProgressDialog();

private:
	int		m_nSeconds;

private slots:

    void	OnSecTimer(void);
};

#endif // CONVERSIONPROGRESS_DIALOG_H
