#ifndef PICK_PARAMETERFILTER_DIALOG_H
#define PICK_PARAMETERFILTER_DIALOG_H

#include "ui_pick_parameterfilter_dialog.h"

/////////////////////////////////////////////////////////////////////////////
class PickParameterFilterDialog : public QDialog, public Ui::PickParameterFilterDialogBase
{
	Q_OBJECT
		
public:

	PickParameterFilterDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	
	void	getFilterSelection(QString &strParameterList);

public slots:

    void	OnNameMask(const QString&);
    void	OnFilterSelection(const QString&);
    void	OnLimit(const QString&);
};

#endif // PICK_PARAMETERFILTER_DIALOG_H
