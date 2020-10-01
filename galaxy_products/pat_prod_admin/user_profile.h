#ifndef USER_PROFILE_H
#define USER_PROFILE_H

#include <QtGui/QDialog>
#include "patprodrecipe_admin.h"

namespace Ui {
    class CUserProfile;
}

class CUserProfile : public QDialog
{
    Q_OBJECT
public:
    CUserProfile(QWidget *parent = 0);
    ~CUserProfile();

    void writeUser(CUserInfo cEntry);			    // Write profile to GUI dialog box
    void readUser(CUserInfo &cEntry,bool bEncrypt=false);   // get profile from GUI dialog box

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CUserProfile *m_ui;
};

#endif // RECIPE_PROFILE_H
