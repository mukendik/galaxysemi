#ifndef GROUP_PROFILE_H
#define GROUP_PROFILE_H

#include <QtGui/QDialog>
#include "patprodrecipe_admin.h"

namespace Ui {
	class CGroupProfile;
}

class CGroupProfile : public QDialog
{
    Q_OBJECT
public:
	CGroupProfile(QWidget *parent = 0);
	~CGroupProfile();

	void writeGroup(CGroupInfo cEntry);		// Write profile to GUI dialog box
	void readGroup(CGroupInfo &cEntry);		// get profile from GUI dialog box

protected:
    void changeEvent(QEvent *e);

private:
	Ui::CGroupProfile *m_ui;
};

#endif // RECIPE_PROFILE_H
