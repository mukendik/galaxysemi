///////////////////////////////////////////////////////////
// GEX Debug functions
///////////////////////////////////////////////////////////
#include <math.h>

#include "browser_dialog.h"

#define GEX_DEBUG 1

#if GEX_DEBUG
#include "import_spektra_dialog.h"
#include "chartdirector_dialog.h"
#include "import_file_dialog.h"
#include "db_insert_file_dialog.h"
#include "gex_debug_dialog.h"

//#include <qlibrary.h>
#include "gexdb_plugin_base.h"
#endif

// gex_constants.cpp
extern const char *		gexLabelFilterChoices[];

///////////////////////////////////////////////////////////
// GEX Debug: Function 1
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexDebug_Function1(void)
{
#if GEX_DEBUG
    // DUMP Spektra Dialog box.
    ImportSpektraDialog cDumpSpektra(false);
    cDumpSpektra.exec();
#endif
}

///////////////////////////////////////////////////////////
// GEX Debug: Function 2
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexDebug_Function2(void)
{
#if GEX_DEBUG
    ChartDirectorDialog	clDialog;

    clDialog.exec();
#endif
}

///////////////////////////////////////////////////////////
// GEX Debug: Function 3
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexDebug_Function3(void)
{
#if GEX_DEBUG
    // Import File Dialog box.
    ImportFileDialog cImportFile;
    cImportFile.exec();
#endif
}

///////////////////////////////////////////////////////////
// GEX Debug: Function 4
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexDebug_Function4(void)
{
#if GEX_DEBUG
    // Import File Dialog box.
    InsertFileDialog cInsertFile;
    cInsertFile.exec();
#endif
}

///////////////////////////////////////////////////////////
// GEX Debug: Function 5
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_GexDebug_Function5(void)
{
#if GEX_DEBUG
    GexDebugDialog	clDialog;

    clDialog.exec();
#endif
}

///////////////////////////////////////////////////////////
// GEX Debug: Function 5Vishay DB cleanup//////////////////////////////////////////////
void GexMainwindow::Wizard_GexDebug_Vishay_Db_Cleanup(void)
{
#if GEX_DEBUG
    GexDebugDialog	clDialog;

    clDialog.exec();
#endif
}

