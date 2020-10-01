///////////////////////////////////////////////////////////
// Examinator Monitoring: History log page
///////////////////////////////////////////////////////////
#include <QFile>
#include <QDir>

#include <gqtl_sysutils.h>

#include "browser.h"
#include "browser_dialog.h"
#include "engine.h"
#include "gex_constants.h"
#include "admin_gui.h"

///////////////////////////////////////////////////////////
// WIZARD: History log file.
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexMo_History(void)
{
    // Show wizard page : select file.
    ShowWizardDialog(GEXMO_HISTORY);

    // Update the URL edit box
    QString strString = GEX_BROWSER_ACTIONBOOKMARK;
    strString += GEXMO_BROWSER_HISTORY_LINK;
    AddNewUrl(strString);
}

