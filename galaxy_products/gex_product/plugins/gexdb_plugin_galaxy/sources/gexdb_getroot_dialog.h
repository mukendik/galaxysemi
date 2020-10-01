#ifndef GEXDB_GETROOT_DIALOG_H
#define GEXDB_GETROOT_DIALOG_H

// Galaxy modules includes
#include <gqtl_skin.h>

#include "ui_gexdb_getroot_dialog.h"

#ifdef unix
#define F_WINDOW_Flags (QWidget::WStyle_Customize | QWidget::WStyle_NormalBorder | QWidget::WStyle_Title | QWidget::WStyle_SysMenu)
#else
	// Under PC: do not show the system menu...so no 'About Qt' can be seen !
#define F_WINDOW_Flags (QWidget::WStyle_Customize | QWidget::WStyle_NormalBorder)
#endif

/////////////////////////////////////////////////////////////////////////////
class GexdbGetrootDialog : public QDialog, public Ui::GexdbGetrootDialogBase
{
	Q_OBJECT

public:
    GexdbGetrootDialog( CGexSkin * pGexSkin, QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	QString		GetRootUsername() { return editRootName->text(); };
	QString		GetRootPassword() { return editRootPassword->text(); };
};

#endif // GEXDB_GETROOT_DIALOG_H
