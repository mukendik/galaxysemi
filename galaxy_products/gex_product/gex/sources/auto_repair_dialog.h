#ifndef AUTO_REPAIR_DIALOG_H
#define AUTO_REPAIR_DIALOG_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "ui_auto_repair_dialog.h"

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexAutoRepairDialog
//
// Description	:	Class to prompt user how to manage corrupted stdf files
//
///////////////////////////////////////////////////////////////////////////////////
class CGexAutoRepairDialog : public QDialog, protected Ui::AutoRepairDialog
{

	Q_OBJECT

public:

	// Define the user choice to repair the corrupted files
	enum autoRepairStatus
	{
		repairUndefined = -1,
		repairNo,
		repairYes,
		repairYesAll
	};

// CONSTRUCTOR / DESTRUCTOR
	CGexAutoRepairDialog(QWidget * pParent, const QString& strFileName);
	virtual ~CGexAutoRepairDialog();

// PROPERTIES ACCESSORS
	int			repairOption() const	{ return m_nAutoRepairOption; }

private:

// MEMBERS

	int			m_nAutoRepairOption;

private slots:

	void		onOptionChanged();
	void		onClickYesAll();
};

#endif // AUTO_REPAIR_DIALOG_H
