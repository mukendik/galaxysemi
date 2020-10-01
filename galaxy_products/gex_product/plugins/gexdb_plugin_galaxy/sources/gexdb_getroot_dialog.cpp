#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include "gexdb_getroot_dialog.h"
#include <gqtl_log.h>

#include <stdio.h>
#include <qtooltip.h>

#ifdef unix
    #include <unistd.h>
    #include <stdlib.h>
#endif

GexdbGetrootDialog::GexdbGetrootDialog( CGexSkin * pGexSkin, QWidget* parent, bool modal, Qt::WindowFlags f )
	: QDialog( parent, f )
{
  GSLOG(SYSLOG_SEV_DEBUG, "Creating a new GexdbGetrootDialog...");

	setupUi(this);
	setModal(modal);

	// Set Examinator skin
	if (!pGexSkin)
    {
        GSLOG(SYSLOG_SEV_ERROR, " pGexSkin NULL");
    }
	else
		pGexSkin->applyPalette(this);

	if (!QObject::connect(PushButtonOk,		SIGNAL(clicked()), this, SLOT(accept())))
		GSLOG(SYSLOG_SEV_ERROR, "QObject::connect failed !");
	if (!QObject::connect(PushButtonCancel,	SIGNAL(clicked()), this, SLOT(reject())))
		GSLOG(SYSLOG_SEV_ERROR, "QObject::connect failed !");

}
