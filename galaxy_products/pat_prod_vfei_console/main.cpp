///////////////////////////////////////////////////////////
// Small Client application that talks to GEX-LM to get
// the list of current licenses in use.
///////////////////////////////////////////////////////////

#include <QApplication>

#include <gqtl_sysutils.h>

#include "licensestatus.h"


char *szAppFullName	= "Galaxy VFEI Console debugger V2.0";

///////////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
	QString			strUserHome, strOutputStatusFile;
    QApplication	app(argc, argv);

#if defined(__sun__) || defined(__linux__)
	// Under Solaris-unix or Linux, the default windows font (MS Sans Serif 8pixels) doesn't give good result!
	// HP-UX is fine with defaukt QT selection.
	app.setFont(QFont( "helvetica",9 ));
#endif

	// Get user's home directory
	CGexSystemUtils::GetUserHomeDirectory(strUserHome);

	// Set GUI style
    CGexSystemUtils::SetGuiStyle();

	// Check for -F argument
	for(int iIndex = 1; iIndex < argc; iIndex++)
	{
		// Check for: Customer debug mode
		if((strcmp(argv[iIndex], "-F") == 0) || (strcmp(argv[iIndex], "-f") == 0))
		{
			// Move to next argument: file name where to output license status info.
			iIndex++;

			if(iIndex < argc)
				strOutputStatusFile = argv[iIndex];
		}
	}

	LicenseStatusDialog cStatusDialog(strUserHome, szAppFullName, strOutputStatusFile, 0, Qt::WindowMinimizeButtonHint);

	// Show dialog box....unless output of status info is file.
	if(strOutputStatusFile.isEmpty())
		cStatusDialog.show();

	// Launch GUI.
    return app.exec();
}

