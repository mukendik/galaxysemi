/****************************************************************************
** Deriven from gexftp_missingfield_dialog_base.cpp
****************************************************************************/
#include "gexftp_missingfield_dialog.h"


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexFtpMissingField_dialog as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
//
// The dialog will by default be modeless, unless you set 'modal' to
// true to construct a modal dialog.
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpMissingField_dialog::CGexFtpMissingField_dialog(QString strFieldName, QWidget* parent, Qt::WindowFlags fl) : QDialog(parent, fl)
{
	// Setup UI
	setupUi(this);
	
	if(!strFieldName.isEmpty())
		labelField->setText(strFieldName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpMissingField_dialog::~CGexFtpMissingField_dialog()
{
    // no need to delete child widgets, Qt does it all for us
}
