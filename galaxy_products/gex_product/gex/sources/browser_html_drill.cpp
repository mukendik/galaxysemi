///////////////////////////////////////////////////////////
// User has clicked an action Bookmark that includes the '#_gex_'
// string in it...this is not a real bookmark, but a
// tag for requesting GEX to perform some task on an image!
// E.g: zoom in/out, etc...
///////////////////////////////////////////////////////////

#include <qapplication.h>

#include "browser.h"
#include "browser_dialog.h"
#include "assistant_flying.h"
#include "gex_constants.h"
#include "onefile_wizard.h"
#include "mergefiles_wizard.h"
#include "comparefiles_wizard.h"
#include <gqtl_log.h>
#include "report_build.h"

extern GexMainwindow	*pGexMainWindow;

//////////////////////////////////////////////////////////
// GexBrowser constructor.
///////////////////////////////////////////////////////////
GexBrowser::GexBrowser(QWidget * parent):QTextBrowser (parent)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "");
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void GexBrowser::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(!e->mimeData()->formats().contains("text/uri-list"))
        e->ignore();
}

///////////////////////////////////////////////////////////
// On-Going DRAG sequence
///////////////////////////////////////////////////////////
void GexBrowser::dragMoveEvent( QDragMoveEvent *e )
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
    else
        e->ignore();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void GexBrowser::dropEvent(QDropEvent *e)
{
    // Have the drag&drop request processe by the main window.
    if(pGexMainWindow != NULL)
        pGexMainWindow->dropEvent(e);
}
