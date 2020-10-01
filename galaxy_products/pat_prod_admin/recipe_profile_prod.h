#ifndef RECIPE_PROFILE_PROD_H
#define RECIPE_PROFILE_PROD_H

#include <QtGui/QDialog>
#include "patprodrecipe_admin.h"

namespace Ui {
	class CRecipeProfile_PROD;
}

class CRecipeProfile_PROD : public QDialog
{
	Q_OBJECT
public:
	CRecipeProfile_PROD(QWidget *parent = 0);
	~CRecipeProfile_PROD();

	void writeProfile(CPROD_Recipe cEntry,bool bAllowEnable);	// Write profile to GUI dialog box
	void readProfile(CPROD_Recipe &cEntry);	// get profile from GUI dialog box

protected:
	void changeEvent(QEvent *e);

private:
	Ui::CRecipeProfile_PROD *m_ui;
};

#endif // RECIPE_PROFILE_PROD_H
