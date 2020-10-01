#ifndef PICK_AUDIT_FILTER_DIALOG_H
#define PICK_AUDIT_FILTER_DIALOG_H

#include "ui_pick_audit_filter_dialog.h"

#if defined unix || __MACH__
#define F_WINDOW_Flags (Qt::WStyle_Customize | Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu)
#else
	// Under PC: do not show the system menu...so no 'About Qt' can be seen !
#define F_WINDOW_Flags (Qt::WStyle_Customize | Qt::WStyle_NormalBorder)
#endif

/////////////////////////////////////////////////////////////////////////////

class PickAuditFilterDialog : public QDialog, public Ui::PickAuditFilterDialogBase
{
	Q_OBJECT

public:
	PickAuditFilterDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	~PickAuditFilterDialog();

	void	setFields(QString strProduct,QString strFilterType,QString strFilterString);
	QString getProduct(void);
	QString getFilterType(void);
	QString getFilterString(void);

public slots:
	void	OnFilterType(void);
};

#endif // PICK_AUDIT_FILTER_DIALOG_H
