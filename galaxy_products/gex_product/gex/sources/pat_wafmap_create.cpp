/////////////////////////////////////////////////////////////////////////////
// Codes to handle PAT outlier filtering
/////////////////////////////////////////////////////////////////////////////

#define SOURCEFILE_PAT_WAFMAP_CREATE_CPP
#include <math.h>

//#include "pat_info.h"
#include "pat_engine.h"
#include "browser.h"
#include "cbinning.h"
#include "engine.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "report_build.h"
#include "message.h"
#include "report_options.h"
#include <gqtl_log.h>

#include <QFileDialog>
#include <QProgressDialog>
#include <QSettings>

// Galaxy QT libraries
#include <gqtl_sysutils.h>
#include <gqtl_utils.h>

#include "browser_dialog.h"
#include "tb_pat_outlier_removal.h"
#include "tb_toolbox.h"	// Examinator ToolBox
#include "report_build.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "gex_scriptengine.h"
#include "plugin_base.h"


// main.cpp
extern GexMainwindow *	pGexMainWindow;	// main.cpp
extern CGexReport *		gexReport;				// report_build.cpp: Handle to report class

extern CReportOptions	ReportOptions;			// Holds options (report_build.h)
extern GexScriptEngine*	pGexScriptEngine;

#include <gtm_testerwindow.h>
//#include "gex_pat_constants_extern.h"

// Holds all global pat info required to create & build PAT reports for a given
// station (real-time final test), or given file (post-processing/wafer-sort)
extern CPatInfo	*lPatInfo;

///////////////////////////////////////////////////////////
// Constructor: used if updating KLA/INF AND building SINF output!
///////////////////////////////////////////////////////////
GexTbPatSinf::GexTbPatSinf()
{
    clear();
}

void GexTbPatSinf::clear(void)
{
    strDevice.clear();
    strLot.clear();
    strWafer.clear();
    strNewWafermap.clear();

    iRefPX = -32768;
    iRefPY = -32768;

    iColRdc = 0;
    iRowRdc = 0;
    iTotalDies = 0;

    iWaferAndPaddingCols=0;	// Total dies & padding dies over the X axis
    iWaferAndPaddingRows=0;	// Total dies & padding dies over the Y axis

    lfDieSiezX = -1;
    lfDieSiezY = -1;

    iFlatDirection = -1;
    iTotalMatchingMapDiesLoc = 0;	// Total dies that exist in both STDF and MAP file.
    iTotalMismatchingMapPassBin=0;
    iTotalMismatchingMapFailBin=0;
    iGoodPartsMap_DPAT_Failures=0;	// number of good die from the input wafer map that become bad die (dpat failures) in the output wafer map.
    iGoodPartsMap_STDF_Missing=0;	// number of good die from the input wafer map that do not have matching STDF data.

    // Clear strings
    strBCEQ = "";
    strNOTOUCHBC = "";
    strSKIPBC = "";
    strINKONLYBCBC = "";

    // Reset input-map bin count info
    cMapBinCount_BeforePAT.clear();	// Holds list of bin count of the input map Before PAT
    cMapBinCount_AfterPAT.clear();	// Holds list of bin count of the input map After PAT
}
