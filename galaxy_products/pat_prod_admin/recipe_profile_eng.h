#ifndef RECIPE_PROFILE_ENG_H
#define RECIPE_PROFILE_ENG_H

#include <QtGui/QDialog>
#include "patprodrecipe_admin.h"

namespace Ui {
	class CRecipeProfile_ENG;
}

class CRecipeProfile_ENG : public QDialog
{
    Q_OBJECT
public:
	CRecipeProfile_ENG(QWidget *parent = 0);
	~CRecipeProfile_ENG();

	void writeProfile(CENG_Recipe cEntry,bool bAllowEnable);	// Write profile to GUI dialog box
	void readProfile(CENG_Recipe &cEntry);	// get profile from GUI dialog box

protected:
    void changeEvent(QEvent *e);

private:
	Ui::CRecipeProfile_ENG *m_ui;
};

#endif // RECIPE_PROFILE_ENG_H
