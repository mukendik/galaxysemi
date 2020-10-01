/****************************************************************************
** Deriven from gexftp_passwordconfirmation_dialog_base.cpp
****************************************************************************/
#include "gexftp_passwordconfirmation_dialog.h"

#include <gqtl_skin.h>


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexFtpPasswordConfirmation_dialog as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
//
// The dialog will by default be modeless, unless you set 'modal' to
// true to construct a modal dialog.
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpPasswordConfirmation_dialog::CGexFtpPasswordConfirmation_dialog(QWidget* parent, Qt::WindowFlags fl ) : QDialog(parent, fl)
{
	// Setup UI
	setupUi(this);

	// Apply default palette 
	CGexSkin gexSkin;
	gexSkin.applyPalette(this);
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpPasswordConfirmation_dialog::~CGexFtpPasswordConfirmation_dialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/////////////////////////////////////////////////////////////////////////////////////
// Return the password
/////////////////////////////////////////////////////////////////////////////////////
QString CGexFtpPasswordConfirmation_dialog::GetPassword(void)
{
	return editPassword->text();
}
