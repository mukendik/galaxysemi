///////////////////////////////////////////////////////////
// GEX Preferences Dialog box
///////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include <qapplication.h>
#include "browser_dialog.h"
#include "browser.h"
#include "options_dialog.h"
#include "settings_dialog.h"
#include "script_wizard.h"
#include "report_build.h"

#include "gex_version.h"
#include "gex_constants.h"

#include "getstring_dialog.h"
#include "report_frontpage_dialog.h"
#include "report_options.h"
#include "pickuser_dialog.h"
#include "gex_skins.h"
#include <gqtl_log.h>
#include "libgexoc.h"

#include <QPixmap>
#include <QListIterator>

// External definitions for pointers to pixmaps.
#include "gex_pixmap_extern.h"


// in main.cpp
extern const char *		szAppFullName;
extern GexMainwindow *	pGexMainWindow;
extern QString			strApplicationDir;
extern QString			strUserFolder;

// in script_wizard.h
extern void ConvertToScriptString(QString &strFile);

extern CReportOptions	ReportOptions;			// Holds options (report_build.h)
extern CGexReport *gexReport;					// Handle to report class

///////////////////////////////////////////////////////////
// Displays the OPTIONS dialog box.
///////////////////////////////////////////////////////////
void GexMainwindow::Wizard_Options(void)
{
	// Show Option page 
	ShowWizardDialog(GEX_OPTIONS);

	// Reset HTML sections to create flag: ALL pages to create.
	pGexMainWindow->iHtmlSectionsToSkip = 0;

	// Display relevant top navigation bar
	QString strPageName, strString;
	switch(ReportOptions.lProductID)
	{
	case GEX_DATATYPE_ALLOWED_ANY:		// Examinator: Any tester type is supported
	case GEX_DATATYPE_ALLOWED_CREDENCE_SAPPHIRE:	// OEM-Examinator for Credence Sapphire
	case GEX_DATATYPE_ALLOWED_CREDENCE_ASL:	// OEM-Examinator for Credence ASL
	case GEX_DATATYPE_ALLOWED_LTX:			// OEM-Examinator for LTX
	case GEX_DATATYPE_ALLOWED_SZ:		// Examinator: Only SZ data files allowed
	case GEX_DATATYPE_ALLOWED_TERADYNE:	// Examinator: Only Teradyne data files allowed
	case GEX_DATATYPE_ALLOWED_DATABASEWEB:
	case GEX_DATATYPE_ALLOWED_DATABASE:	// ExaminatorDB: Database access, any file type.
	case GEX_DATATYPE_GEX_TOOLBOX:		// Examinator ToolBox.
	case GEX_DATATYPE_GEX_YIELD123:		// Yield123
		strPageName = GEX_BROWSER_OPTIONS_TOPLINK;
		break;

	case GEX_DATATYPE_GEX_MONITORING:	// ExaminatorMonitoring
	case GEX_DATATYPE_GEX_PATMAN:		// Galaxy PAT-Man
		strPageName = GEXMO_BROWSER_OPTIONS_TOPLINK;
		break;
	}

	// HTML skin pages Sub-folder .
	strString += navigationSkinDir();
	strString += "pages/";
	strString += GEX_BROWSER_ACTIONLINK;
	strString += strPageName;
	GexHtmlTop->setSource( QUrl::fromLocalFile(strString) );
}

///////////////////////////////////////////////////////////
// GEX Options dialog box.
///////////////////////////////////////////////////////////
GexOptions::GexOptions( QWidget* parent, bool modal, Qt::WFlags fl) : QDialog(parent, fl)
{
	setupUi(this);
	setModal(modal);

	setWindowFlags(Qt::WStyle_Customize | Qt::WStyle_NoBorder);
	move(0,0);

	QObject::connect(ListView, SIGNAL(clicked(Q3ListViewItem*)), this, SLOT(OnSelectionChanged()));
	QObject::connect(ListView, SIGNAL(contextMenuRequested(Q3ListViewItem*,QPoint,int)), this, SLOT(onContextMenuRequested(Q3ListViewItem*)));

	// List of options...: start from last to one as list inverts entries!
	ListView->setRootIsDecorated(true);
	ListView->setSorting(-1);
	ListView->header()->setClickEnabled( false );
	Q3ListViewItem *ptItem,*ptParent;
	Q3CheckListItem *ptCheck;

	//// Environment Preferences
    ptNodePreferences = ptParent = new Q3ListViewItem( ListView, "Environment Preferences ( text editor... )","Application preferences");
	ptParent->setPixmap(0,*pixGexApplication);
	ptCheck  = new Q3CheckListItem(ptParent,"Close application (and release license) if idle too long...",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptNodeAutoClose = ptCheck;
	ptPreferences_AutoCloseNever= new Q3CheckListItem(ptCheck,"Never release license",Q3CheckListItem::RadioButton);
	ptPreferences_AutoClose4hours= new Q3CheckListItem(ptCheck,"Release license if Idle for: 4 hours",Q3CheckListItem::RadioButton);
	ptPreferences_AutoClose2hours= new Q3CheckListItem(ptCheck,"Release license if Idle for: 2 hours",Q3CheckListItem::RadioButton);
	ptPreferences_AutoClose1hour= new Q3CheckListItem(ptCheck,"Release license if Idle for: 1 hour",Q3CheckListItem::RadioButton);
	ptPreferences_AutoClose30minutes= new Q3CheckListItem(ptCheck,"Release license if Idle for: 30 minutes",Q3CheckListItem::RadioButton);
	ptPreferences_AutoClose15minutes= new Q3CheckListItem(ptCheck,"Release license if Idle for: 15 minutes",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// Spreadsheet  editor (CSV/Excel)
	ptCheck = new Q3CheckListItem(ptParent,"Spreadsheet editor (CSV/Excel)",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCustomizeSSheetEditor = new Q3CheckListItem(ptCheck,"Select other Spreadsheet editor...to specify now!",Q3CheckListItem::RadioButton);
#if (defined __sun__ || __hpux__)
	// Under Solaris Unix & HP-UX Unix, default spreadsheet editor is 'textedit'
	ptPreferences_CustomSpreadsheetEditor = new Q3CheckListItem(ptCheck,"Current editor: textedit",Q3CheckListItem::RadioButton);
#elif (defined __linux__)
	// Under linux, default spreadsheet editor is Open Ofice calc: 'oocalc'
	ptPreferences_CustomSpreadsheetEditor = new Q3CheckListItem(ptCheck,"Current editor: oocalc",Q3CheckListItem::RadioButton);
#else
	// Under Windows, path is c:/
	ptPreferences_CustomSpreadsheetEditor = new Q3CheckListItem(ptCheck,"Current editor: (default)",Q3CheckListItem::RadioButton);
#endif
	ListView->insertItem(ptParent);

	// Text Editor
	ptCheck = new Q3CheckListItem(ptParent,"Text editor",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCustomizeTextEditor = new Q3CheckListItem(ptCheck,"Select other text editor...to specify now!",Q3CheckListItem::RadioButton);
#if (defined __sun__ || __hpux__)
	// Under Solaris Unix & HP-UX Unix, default editor is 'textedit'
	ptPreferences_CustomTextEditor = new Q3CheckListItem(ptCheck,"Current editor: textedit",Q3CheckListItem::RadioButton);
#elif (defined __linux__)
	// Under linux, default editor is 'gedit'
	ptPreferences_CustomTextEditor = new Q3CheckListItem(ptCheck,"Current editor: gedit",Q3CheckListItem::RadioButton);
#else
	// Under Windows, path is c:/
	ptPreferences_CustomTextEditor = new Q3CheckListItem(ptCheck,"Current editor: (default)",Q3CheckListItem::RadioButton);
#endif
	ListView->insertItem(ptParent);

	// Pdf Editor
	ptCheck = new Q3CheckListItem(ptParent,"Pdf editor", Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCustomizePdfEditor=new Q3CheckListItem(ptCheck,"Select other pdf editor...to specify now!", Q3CheckListItem::RadioButton);
	ptPreferences_CustomPdfEditor=new Q3CheckListItem(ptCheck,"Current editor: (default)", Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	//// Toolbox
    ptParent = new Q3ListViewItem( ListView, "Toolbox","Define Toolbox options");
	ptParent->setPixmap(0,*pixGexApplication);

	ptItem = new Q3ListViewItem(ptParent,"CSV Generation...");
	ptItem->setPixmap(0,*pixChildTree);
	ptToolbox_CsvSplitExport				= new Q3CheckListItem(ptItem,"Split into multiple CSV files if more than 250 rows (each file = 250 rows)",Q3CheckListItem::CheckBox);
	
	ptCheck									= new Q3CheckListItem(ptItem,"Units mode");
	ptCheck->setPixmap(0,*pixChildTree);
	ptToolbox_csvUnitsModeScalingFactor		= new Q3CheckListItem(ptCheck,"Scaling factor",Q3CheckListItem::RadioButton);
	ptToolbox_csvUnitsModeNormalized		= new Q3CheckListItem(ptCheck,"Normalized",Q3CheckListItem::RadioButton);

	ptCheck									= new Q3CheckListItem(ptItem,"Column ordering", Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptToolbox_CsvSortTestFlowID				= new Q3CheckListItem(ptCheck,"Sort by Test FlowID/execution order",Q3CheckListItem::RadioButton);
	ptToolbox_CsvSortTestID					= new Q3CheckListItem(ptCheck,"Sort by increasing Test# (default)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	//// Pearson's correlation: Test time Optimization
    ptNodePearson = ptParent = new Q3ListViewItem( ListView, "Pearson's correlation report","Test time Optimization missions");
	ptParent->setPixmap(0,*pixStopwatch);

	ptCheck = new Q3CheckListItem(ptParent,"Sorting mode");
	ptCheck->setPixmap(0,*pixChildTree);
	ptPearson_SortRatio= new Q3CheckListItem(ptCheck,"Sort by Pearson's correlation ratio",Q3CheckListItem::RadioButton);
	ptPearson_SortTestName= new Q3CheckListItem(ptCheck,"Sort by Test Name",Q3CheckListItem::RadioButton);

	// Ignore tests with correlartion ratio under a given threshold
	ptCheck = new Q3CheckListItem(ptParent,"Minimum correlation value required (threshold)",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"Ignore tests with a lower correlation value");
	ptPearson_ThresholdRatio = new Q3CheckListItem(ptCheck,"Ignore tests with a Pearson's correlation ratio (r^2) lower than: 0.8",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	//// What-if / Virtual retest
    ptParent = new Q3ListViewItem( ListView, "What-If / Virtual retest","virtual Retest options");
	ptParent->setPixmap(0,*pixGuardBanding);

	// Binning associated with what-if
	ptCheck = new Q3CheckListItem(ptParent,"What-If Binning (when overloading binning)",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"What-if Binning");
	ptWhatif_Fail_BinningIsPass = new Q3CheckListItem(ptCheck,"Force What-if Failing Bin to PASS type",Q3CheckListItem::CheckBox);
	ptWhatif_Fail_Binning = new Q3CheckListItem(ptCheck,"What-if Failing Binning: 0",Q3CheckListItem::RadioButton);
	ptWhatif_Pass_Binning = new Q3CheckListItem(ptCheck,"What-if Passing Binning: 1",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);
	
	//// Datalog
    ptNodeDatalog = ptParent = new Q3ListViewItem( ListView, "Datalog","Datalog options");
	ptParent->setPixmap(0,*pixDatalog);
	ptItem = new Q3ListViewItem(ptParent,"Fields to include");
	ptItem->setPixmap(0,*pixChildTree);
	ptDatalog_DieLoc = new Q3CheckListItem(ptItem,"Die XY location (if it exists)",Q3CheckListItem::CheckBox);
	ptDatalog_Limits = new Q3CheckListItem(ptItem,"Test Limits",Q3CheckListItem::CheckBox);
	ptDatalog_TestName = new Q3CheckListItem(ptItem,"Test name",Q3CheckListItem::CheckBox);
	ptDatalog_TestNumber = new Q3CheckListItem(ptItem,"Test number",Q3CheckListItem::CheckBox);
	ptDatalog_Comment = new Q3CheckListItem(ptItem,"Comments sections",Q3CheckListItem::CheckBox);
	ListView->insertItem(ptParent);
	ptCheck = new Q3CheckListItem(ptParent,"Output format",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptDatalog_2Rows = new Q3CheckListItem(ptCheck,"2 Rows",Q3CheckListItem::RadioButton);
	ptDatalog_1Row= new Q3CheckListItem(ptCheck,"1 Row",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// Global Infos
	ptNodesGlobalInfo = ptParent = new Q3ListViewItem( ListView, "Global Info", "Global Information settings");
	ptNodesGlobalInfo->setPixmap(0,*pixGlobalInfo); //Find the globalinfo pixel
	ptCheck = new Q3CheckListItem(ptParent, "Select a Detail Level when Comparing/Merging", Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptGlobalInfoDetailed = new Q3CheckListItem(ptCheck, "Detailed", Q3CheckListItem::RadioButton);
	ptGlobalInfoSummarized = new Q3CheckListItem(ptCheck, "Summarized", Q3CheckListItem::RadioButton);
	ptGlobalInfoSummarized->setOn(true);
	
	//// Binning
    ptNodeBinning = ptParent = new Q3ListViewItem( ListView, "Binning","Binning options");
	ptParent->setPixmap(0,*pixBinning);
	ptCheck = new Q3CheckListItem(ptParent,"How is Binning computed",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptBinning_UseWafermapOnly= new Q3CheckListItem(ptCheck,"From wafer map / strip map report only (handles retests properly)",Q3CheckListItem::RadioButton);
	ptBinning_UseSamplesOnly= new Q3CheckListItem(ptCheck,"From samples data only",Q3CheckListItem::RadioButton);
	ptBinning_UseSummary = new Q3CheckListItem(ptCheck,"From summary data (if any), otherwise from samples",Q3CheckListItem::RadioButton);
	ptCheck = new Q3CheckListItem(ptParent,"Include section in report?",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptBinning_NotInReport = new Q3CheckListItem(ptCheck,"No",Q3CheckListItem::RadioButton);
	ptBinning_InReport = new Q3CheckListItem(ptCheck,"Yes",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);
	
	//// Pareto
    ptNodePareto = ptParent = new Q3ListViewItem( ListView, "Pareto","Pareto options");
	ptParent->setPixmap(0,*pixPareto);
	ptCheck = new Q3CheckListItem(ptParent,"Define Failure Signature cut-off limit",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptPareto_SignatureFailure_All= new Q3CheckListItem(ptCheck,"Report ALL Signature failures",Q3CheckListItem::RadioButton);
	ptPareto_SignatureFailureCutOff= new Q3CheckListItem(ptCheck,"Total percentage of Signature failures to report: 25.00 %",Q3CheckListItem::RadioButton);
	ptCheck = new Q3CheckListItem(ptParent,"Define Test Failures cut-off limit",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptPareto_Failure_All= new Q3CheckListItem(ptCheck,"Report ALL test failures",Q3CheckListItem::RadioButton);
	ptPareto_FailureCutOff= new Q3CheckListItem(ptCheck,"Total parameters to report (Top N): 10",Q3CheckListItem::RadioButton);
	ptCheck = new Q3CheckListItem(ptParent,"Define Cpk cut-off limit",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptPareto_AllCpk= new Q3CheckListItem(ptCheck,"Report ALL Cpk",Q3CheckListItem::RadioButton);
	ptPareto_CpkCutOff= new Q3CheckListItem(ptCheck,"Only Cpk <= to...: 1.33",Q3CheckListItem::RadioButton);
	ptCheck = new Q3CheckListItem(ptParent,"Define Cp cut-off limit",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptPareto_AllCp= new Q3CheckListItem(ptCheck,"Report ALL Cp",Q3CheckListItem::RadioButton);
	ptPareto_CpCutOff= new Q3CheckListItem(ptCheck,"Only Cp <= to...: 1.33",Q3CheckListItem::RadioButton);

	ptItem = new Q3CheckListItem(ptParent,"Exclude binnings",Q3CheckListItem::RadioButtonController);
	ptItem->setPixmap(0,*pixChildTree);
	ptPareto_ExcludeFailBin= new Q3CheckListItem(ptItem,"FAIL",Q3CheckListItem::CheckBox);
	ptPareto_ExcludePassBin= new Q3CheckListItem(ptItem,"PASS",Q3CheckListItem::CheckBox);
	
	ptItem = new Q3CheckListItem(ptParent,"Pareto tables to include in report?",Q3CheckListItem::RadioButtonController);
	ptItem->setPixmap(0,*pixChildTree);
	ptPareto_Hbin= new Q3CheckListItem(ptItem,"Hard Bin",Q3CheckListItem::CheckBox);
	ptPareto_Sbin= new Q3CheckListItem(ptItem,"Soft Bin",Q3CheckListItem::CheckBox);
	ptPareto_FailSignature= new Q3CheckListItem(ptItem,"Failure signatures",Q3CheckListItem::CheckBox);
	ptPareto_Fail= new Q3CheckListItem(ptItem,"Failures",Q3CheckListItem::CheckBox);
	ptPareto_Cpk= new Q3CheckListItem(ptItem,"Cpk",Q3CheckListItem::CheckBox);
	ptPareto_Cp= new Q3CheckListItem(ptItem,"Cp",Q3CheckListItem::CheckBox);
	ListView->insertItem(ptParent);

	//// Gage R&R - Boxplot
    ptNodeBoxplot = ptParent = new Q3ListViewItem( ListView, "Gage R&R","Gage R&R charting options");
	ptParent->setPixmap(0,*pixBoxMeanRange);

	ptItem = new Q3ListViewItem(ptParent,"R&R WARNINGS & ALARMS criteria");
	ptItem->setPixmap(0,*pixChildTree);
	ptBoxplot_CustomizeAlarm = new Q3CheckListItem(ptItem,"Edit/change 'R&R' Alarm limits criteria...",Q3CheckListItem::RadioButton);

	ptCheck = new Q3CheckListItem(ptParent,"R&R % variation options");
	ptCheck->setPixmap(0,*pixChildTree);
	ptBoxplot_ShiftOverTV= new Q3CheckListItem(ptCheck,"EV%,AV%,R&R%,PV%,TV%: Variation computed over TV",Q3CheckListItem::RadioButton);
	ptBoxplot_ShiftOverLimits= new Q3CheckListItem(ptCheck,"EV%,AV%,R&R%,PV%,TV%: Variation computed over Limit space",Q3CheckListItem::RadioButton);

	ptCheck = new Q3CheckListItem(ptParent,"R&R N*Sigma option");
	ptCheck->setPixmap(0,*pixChildTree);
	ptBoxplot_Nsigma= new Q3CheckListItem(ptCheck,"Customize N*Sigma space (default N value is 5.15). N=5.15",Q3CheckListItem::RadioButton);

	ptCheck = new Q3CheckListItem(ptParent,"Sorting mode");
	ptCheck->setPixmap(0,*pixChildTree);
	ptBoxplot_SortR_and_R= new Q3CheckListItem(ptCheck,"Sort by gage R&R% value (by decreasing R&R% value)",Q3CheckListItem::RadioButton);
	ptBoxplot_SortCpk= new Q3CheckListItem(ptCheck,"Sort by Cpk (of Reference when comparing files/groups)",Q3CheckListItem::RadioButton);
	ptBoxplot_SortCp= new Q3CheckListItem(ptCheck,"Sort by Cp (of Reference when comparing files/groups)",Q3CheckListItem::RadioButton);
	ptBoxplot_SortSigma= new Q3CheckListItem(ptCheck,"Sort by Sigma (of Reference when comparing files/groups)",Q3CheckListItem::RadioButton);
	ptBoxplot_SortMean= new Q3CheckListItem(ptCheck,"Sort by Mean (of Reference when comparing files/groups)",Q3CheckListItem::RadioButton);
	ptBoxplot_SortTestFlowID= new Q3CheckListItem(ptCheck,"Sort by Test Flow ID (test execution order)",Q3CheckListItem::RadioButton);
	ptBoxplot_SortTestName= new Q3CheckListItem(ptCheck,"Sort by Test Name",Q3CheckListItem::RadioButton);
	ptBoxplot_SortTestNumber= new Q3CheckListItem(ptCheck,"Sort by Test Number",Q3CheckListItem::RadioButton);

	ptCheck = new Q3CheckListItem(ptParent,"Chart type");
	ptCheck->setPixmap(0,*pixChildTree);
	ptBoxplot_ChartAdaptive= new Q3CheckListItem(ptCheck,"Adaptive ('over limits' if one data set, 'over range' otherwise)",Q3CheckListItem::RadioButton);
	ptBoxplot_ChartRange= new Q3CheckListItem(ptCheck,"Chart over data range",Q3CheckListItem::RadioButton);
	ptBoxplot_ChartLimits= new Q3CheckListItem(ptCheck,"Chart over test limits",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptItem = new Q3ListViewItem(ptParent,"Fields to include in chart");
	ptItem->setPixmap(0,*pixChildTree);
	ptBoxPlot_P_T			  = new Q3CheckListItem(ptItem,"P/T ratio (Gage Study)",Q3CheckListItem::CheckBox);
	ptBoxPlot_TV              = new Q3CheckListItem(ptItem,"Total Variation: TV (Gage Study)",Q3CheckListItem::CheckBox);
	ptBoxPlot_PV              = new Q3CheckListItem(ptItem,"Part Variation: PV (Gage Study)",Q3CheckListItem::CheckBox);
	ptBoxPlot_GB	          = new Q3CheckListItem(ptItem,"GB (Gage Study)",Q3CheckListItem::CheckBox);
	ptBoxPlot_R_and_R         = new Q3CheckListItem(ptItem,"R&R (Gage Study)",Q3CheckListItem::CheckBox);
	ptBoxPlot_ReproducibilityAV = new Q3CheckListItem(ptItem,"Reproducibility / Appraiser variation: AV (Gage Study)",Q3CheckListItem::CheckBox);
	ptBoxPlot_RepeatabilityEV   = new Q3CheckListItem(ptItem,"Equipment Variation EV (Gage Study)",Q3CheckListItem::CheckBox);
	ptBoxPlot_Repeatability   = new Q3CheckListItem(ptItem,"Repeatability",Q3CheckListItem::CheckBox);
	ptBoxPlot_Cpk             = new Q3CheckListItem(ptItem,"Cpk (of Reference when comparing files/groups)",Q3CheckListItem::CheckBox);
	ptBoxPlot_Cp              = new Q3CheckListItem(ptItem,"Cp (of Reference when comparing files/groups)",Q3CheckListItem::CheckBox);
	ptBoxPlot_Median          = new Q3CheckListItem(ptItem,"Median",Q3CheckListItem::CheckBox);
	ptBoxPlot_Sigma           = new Q3CheckListItem(ptItem,"Sigma (of Reference when comparing files/groups)",Q3CheckListItem::CheckBox);
	ptBoxPlot_Mean            = new Q3CheckListItem(ptItem,"Mean (of Reference when comparing files/groups)",Q3CheckListItem::CheckBox);
	ptBoxPlot_TestLimits      = new Q3CheckListItem(ptItem,"Test limits",Q3CheckListItem::CheckBox);

	//// Production Yield/Volume chart
    ptNodeProductionYield = ptParent = new Q3ListViewItem( ListView, "Production reports (Yield / Volume charts)","Production chart options");
	ptParent->setPixmap(0,*pixTrend);
	ptItem = new Q3ListViewItem(ptParent,"Markers to draw on chart");
	ptItem->setPixmap(0,*pixChildTree);
	ptProdYield_MarkerVolume = new Q3CheckListItem(ptItem,"Volume count for each bar",Q3CheckListItem::CheckBox);
	ptProdYield_MarkerYield = new Q3CheckListItem(ptItem,"Yield value for each data point",Q3CheckListItem::CheckBox);
	ptProdYield_MarkerTitle = new Q3CheckListItem(ptItem,"Title",Q3CheckListItem::CheckBox);
	
	ptCheck = new Q3CheckListItem(ptParent,"Chart type",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptProdYield_YieldVolumeChart = new Q3CheckListItem(ptCheck,"Yield/Volume dual chart",Q3CheckListItem::RadioButton);
	ptProdYield_VolumeChart = new Q3CheckListItem(ptCheck,"Volume trend chart (total parts tested)",Q3CheckListItem::RadioButton);
	ptProdYield_YieldChart = new Q3CheckListItem(ptCheck,"Yield trend chart",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);	ListView->insertItem(ptParent);

	// Box plot
    ptNodeBoxPlotEx = ptParent = new Q3ListViewItem( ListView, "Box plot", "Boxplot options");
	ptParent->setPixmap(0,*pixBoxPlot);
	
	ptCheck = new Q3CheckListItem(ptParent,"Orientation");
	ptCheck->setPixmap(0,*pixChildTree);
	ptBoxplotEx_Intercative_V = new Q3CheckListItem(ptCheck, "Draw Vertical boxes",		Q3CheckListItem::RadioButton);
	ptBoxplotEx_Intercative_H = new Q3CheckListItem(ptCheck, "Draw Horizontal boxes",	Q3CheckListItem::RadioButton);

	ptItem = new Q3ListViewItem(ptParent,"Markers to draw on chart");
	ptItem->setPixmap(0,*pixChildTree);
	ptBoxPlotEx_Marker6Sigma		= new Q3CheckListItem(ptItem,"+/- 3 Sigma range, 6 Sigma space (centered on Mean)",		Q3CheckListItem::CheckBox);
	ptBoxPlotEx_Marker3Sigma		= new Q3CheckListItem(ptItem,"+/- 1.5 Sigma range, 3 Sigma space (centered on Mean)",	Q3CheckListItem::CheckBox);
	ptBoxPlotEx_Marker2Sigma		= new Q3CheckListItem(ptItem,"+/- 1 Sigma range, 2 Sigma space (centered on Mean)",		Q3CheckListItem::CheckBox);
	ptBoxPlotEx_MarkerSpecLimits	= new Q3CheckListItem(ptItem,"Test Spec Limits",	Q3CheckListItem::CheckBox);
	ptBoxPlotEx_MarkerLimits		= new Q3CheckListItem(ptItem,"Test Limits",	Q3CheckListItem::CheckBox);
	ptBoxPlotEx_MarkerMedian		= new Q3CheckListItem(ptItem,"Median",		Q3CheckListItem::CheckBox);
	ptBoxPlotEx_MarkerMean			= new Q3CheckListItem(ptItem,"Mean",		Q3CheckListItem::CheckBox);
	ptBoxPlotEx_MarkerTestName		= new Q3CheckListItem(ptItem,"Title",		Q3CheckListItem::CheckBox);

	ptCheck = new Q3CheckListItem(ptParent,"Chart size");
	ptCheck->setPixmap(0,*pixChildTree);
	ptBoxPlotEx_Adaptive	= new Q3CheckListItem(ptCheck,"Adaptive: Large, then smaller as number of charts increase",Q3CheckListItem::RadioButton);
	ptBoxPlotEx_Large		= new Q3CheckListItem(ptCheck,"Large (800x400)",	Q3CheckListItem::RadioButton);
	ptBoxPlotEx_Medium		= new Q3CheckListItem(ptCheck,"Medium (400x200)",	Q3CheckListItem::RadioButton);
	ptBoxPlotEx_Small		= new Q3CheckListItem(ptCheck,"Small, mosaic display (200x100)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// Probability plot
    ptNodeProbabilityPlot = ptParent = new Q3ListViewItem( ListView, "Probability plot", "Probability plot options");
	ptParent->setPixmap(0,*pixProbabilityPlot);
	
	ptCheck = new Q3CheckListItem(ptParent,"Y-axis scale type");
	ptCheck->setPixmap(0,*pixChildTree);
	ptProbPlot_Yaxis_Sigma		= new Q3CheckListItem(ptCheck, "Z (Sigma)",			Q3CheckListItem::RadioButton);
	ptProbPlot_Yaxis_Percent	= new Q3CheckListItem(ptCheck, "Percentage (%)",	Q3CheckListItem::RadioButton);

	ptCheck = new Q3CheckListItem(ptParent,"Markers to draw on chart");
	ptCheck->setPixmap(0,*pixChildTree);
	ptProbPlot_Marker6Sigma		= new Q3CheckListItem(ptCheck,"+/- 3 Sigma range, 6 Sigma space (centered on Mean)",		Q3CheckListItem::CheckBox);
	ptProbPlot_Marker3Sigma		= new Q3CheckListItem(ptCheck,"+/- 1.5 Sigma range, 3 Sigma space (centered on Mean)",	Q3CheckListItem::CheckBox);
	ptProbPlot_Marker2Sigma		= new Q3CheckListItem(ptCheck,"+/- 1 Sigma range, 2 Sigma space (centered on Mean)",		Q3CheckListItem::CheckBox);
	ptProbPlot_MarkerSpecLimits	= new Q3CheckListItem(ptCheck,"Test Spec Limits",	Q3CheckListItem::CheckBox);
	ptProbPlot_MarkerLimits		= new Q3CheckListItem(ptCheck,"Test Limits",	Q3CheckListItem::CheckBox);
	ptProbPlot_MarkerMedian		= new Q3CheckListItem(ptCheck,"Median",		Q3CheckListItem::CheckBox);
	ptProbPlot_MarkerMean		= new Q3CheckListItem(ptCheck,"Mean",		Q3CheckListItem::CheckBox);
	ptProbPlot_MarkerTestName	= new Q3CheckListItem(ptCheck,"Title",		Q3CheckListItem::CheckBox);

	ptCheck = new Q3CheckListItem(ptParent,"Chart size");
	ptCheck->setPixmap(0,*pixChildTree);
	ptProbPlot_Adaptive	= new Q3CheckListItem(ptCheck,"Adaptive: Large, then smaller as number of charts increase",Q3CheckListItem::RadioButton);
	ptProbPlot_Large	= new Q3CheckListItem(ptCheck,"Large (800x400)",	Q3CheckListItem::RadioButton);
	ptProbPlot_Medium	= new Q3CheckListItem(ptCheck,"Medium (400x200)",	Q3CheckListItem::RadioButton);
	ptProbPlot_Small	= new Q3CheckListItem(ptCheck,"Small, mosaic display (200x100)",Q3CheckListItem::RadioButton);

	ListView->insertItem(ptParent);

	//// Correlation (XY-Scatter) / BiVariate
    ptNodeScatter = ptParent = new Q3ListViewItem( ListView, "Correlation (BiVariate / Scatter chart)","BiVariate chart options");
	ptParent->setPixmap(0,*pixScatter);
	ptItem = new Q3ListViewItem(ptParent,"Markers to draw on chart");
	ptItem->setPixmap(0,*pixChildTree);
	ptScatter_Marker6Sigma		= new Q3CheckListItem(ptItem,"+/- 3 Sigma range, 6 Sigma space (centered on Mean)",Q3CheckListItem::CheckBox);
	ptScatter_Marker3Sigma		= new Q3CheckListItem(ptItem,"+/- 1.5 Sigma range, 3 Sigma space (centered on Mean)",Q3CheckListItem::CheckBox);
	ptScatter_Marker2Sigma		= new Q3CheckListItem(ptItem,"+/- 1 Sigma range, 2 Sigma space (centered on Mean)",Q3CheckListItem::CheckBox);
	ptScatter_MarkerSpecLimits	= new Q3CheckListItem(ptItem,"Test Spec Limits",Q3CheckListItem::CheckBox);
	ptScatter_MarkerLimits		= new Q3CheckListItem(ptItem,"Test Limits",Q3CheckListItem::CheckBox);
	ptScatter_MarkerMedian		= new Q3CheckListItem(ptItem,"Median",Q3CheckListItem::CheckBox);
	ptScatter_MarkerMean		= new Q3CheckListItem(ptItem,"Mean",Q3CheckListItem::CheckBox);
	ptScatter_MarkerTestName	= new Q3CheckListItem(ptItem,"Title",Q3CheckListItem::CheckBox);

	ptCheck = new Q3CheckListItem(ptParent,"Chart size");
	ptCheck->setPixmap(0,*pixChildTree);
	ptScatter_Adaptive= new Q3CheckListItem(ptCheck,"Adaptive: Large, then smaller as number of charts increase",Q3CheckListItem::RadioButton);
	ptScatter_Large= new Q3CheckListItem(ptCheck,"Large (800x400)",Q3CheckListItem::RadioButton);
	ptScatter_Medium= new Q3CheckListItem(ptCheck,"Medium (400x200)",Q3CheckListItem::RadioButton);
	ptScatter_Small= new Q3CheckListItem(ptCheck,"Small, mosaic display (200x100)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	//// Trend
    ptNodeTrend = ptParent = new Q3ListViewItem( ListView, "Trend (Control chart)","Advanced Trend chart options");
	ptParent->setPixmap(0,*pixTrend);

	ptCheck = new Q3CheckListItem(ptParent,"Rolling Yield");
	ptCheck->setPixmap(0,*pixChildTree);
	ptTrend_RollingYield= new Q3CheckListItem(ptCheck,"Edit/Change total parts per Rolling Yield: 200",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptItem = new Q3ListViewItem(ptParent,"Markers to draw on chart");
	ptItem->setPixmap(0,*pixChildTree);
	ptTrend_Marker6Sigma		= new Q3CheckListItem(ptItem,"+/- 3 Sigma range, 6 Sigma space (centered on Mean)",Q3CheckListItem::CheckBox);
	ptTrend_Marker3Sigma		= new Q3CheckListItem(ptItem,"+/- 1.5 Sigma range, 3 Sigma space (centered on Mean)",Q3CheckListItem::CheckBox);
	ptTrend_Marker2Sigma		= new Q3CheckListItem(ptItem,"+/- 1 Sigma range, 2 Sigma space (centered on Mean)",Q3CheckListItem::CheckBox);
	ptTrend_MarkerGroupName		= new Q3CheckListItem(ptItem,"Dataset / Group name",Q3CheckListItem::CheckBox);
	ptTrend_MarkerSubLot		= new Q3CheckListItem(ptItem,"SubLot / Wafer ID",Q3CheckListItem::CheckBox);
	ptTrend_MarkerLot			= new Q3CheckListItem(ptItem,"Lot ID",Q3CheckListItem::CheckBox);
	ptTrend_MarkerSpecLimits	= new Q3CheckListItem(ptItem,"Test Spec Limits",Q3CheckListItem::CheckBox);
	ptTrend_MarkerLimits		= new Q3CheckListItem(ptItem,"Test Limits",Q3CheckListItem::CheckBox);
	ptTrend_MarkerMedian		= new Q3CheckListItem(ptItem,"Median",Q3CheckListItem::CheckBox);
	ptTrend_MarkerMean			= new Q3CheckListItem(ptItem,"Mean",Q3CheckListItem::CheckBox);
	ptTrend_MarkerTestName		= new Q3CheckListItem(ptItem,"Title",Q3CheckListItem::CheckBox);

	ptCheck = new Q3CheckListItem(ptParent,"X-Axis scale type");
	ptCheck->setPixmap(0,*pixChildTree);
	ptTrend_XAxisPartID	= new Q3CheckListItem(ptCheck, "Part ID",	Q3CheckListItem::RadioButton);
	ptTrend_XAxisRunID	= new Q3CheckListItem(ptCheck, "Run ID",	Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptCheck = new Q3CheckListItem(ptParent,"Chart type");
	ptCheck->setPixmap(0,*pixChildTree);
	ptTrend_TypeLinesSpots= new Q3CheckListItem(ptCheck,"Lines + Spots",Q3CheckListItem::RadioButton);
	ptTrend_TypeSpots= new Q3CheckListItem(ptCheck,"Spots",Q3CheckListItem::RadioButton);
	ptTrend_TypeLines= new Q3CheckListItem(ptCheck,"Lines",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptCheck = new Q3CheckListItem(ptParent,"Chart size");
	ptCheck->setPixmap(0,*pixChildTree);
	ptTrend_Adaptive= new Q3CheckListItem(ptCheck,"Adaptive: Large, then smaller as number of charts increase",Q3CheckListItem::RadioButton);
	ptTrend_Large= new Q3CheckListItem(ptCheck,"Large (800x400)",Q3CheckListItem::RadioButton);
	ptTrend_Medium= new Q3CheckListItem(ptCheck,"Medium (400x200)",Q3CheckListItem::RadioButton);
	ptTrend_Small= new Q3CheckListItem(ptCheck,"Small, mosaic display (200x100)",Q3CheckListItem::RadioButton);
	ptTrend_Banner= new Q3CheckListItem(ptCheck,"Banner, maximize number of charts per page (900x120)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	//// Histogram
    ptNodeHistogram = ptParent = new Q3ListViewItem( ListView, "Histogram","Advanced chart options");
	ptParent->setPixmap(0,*pixAdvHisto);

	ptItem = new Q3ListViewItem(ptParent,"Markers to draw on chart  (when using 'More reports' in 'Settings' page)");
	ptItem->setPixmap(0,*pixChildTree);
	ptHistogram_Marker6Sigma		= new Q3CheckListItem(ptItem,"+/- 3 Sigma range, 6 Sigma space (centered on Mean)",Q3CheckListItem::CheckBox);
	ptHistogram_Marker3Sigma		= new Q3CheckListItem(ptItem,"+/- 1.5 Sigma range, 3 Sigma space (centered on Mean)",Q3CheckListItem::CheckBox);
	ptHistogram_Marker2Sigma		= new Q3CheckListItem(ptItem,"+/- 1 Sigma range, 2 Sigma space (centered on Mean)",Q3CheckListItem::CheckBox);
	ptHistogram_MarkerSpecLimits	= new Q3CheckListItem(ptItem,"Test Spec Limits",Q3CheckListItem::CheckBox);
	ptHistogram_MarkerLimits		= new Q3CheckListItem(ptItem,"Test Limits",Q3CheckListItem::CheckBox);
	ptHistogram_MarkerMedian		= new Q3CheckListItem(ptItem,"Median",Q3CheckListItem::CheckBox);
	ptHistogram_MarkerMean			= new Q3CheckListItem(ptItem,"Mean",Q3CheckListItem::CheckBox);
	ptHistogram_MarkerTestName		= new Q3CheckListItem(ptItem,"Title",Q3CheckListItem::CheckBox);
	
	ptCheck = new Q3CheckListItem(ptParent,"Y-axis scale type");
	ptCheck->setPixmap(0,*pixChildTree);
	ptHistogram_Yaxis_Hits = new Q3CheckListItem(ptCheck,"Frequency count / Hits",Q3CheckListItem::RadioButton);
	ptHistogram_Yaxis_Percentage= new Q3CheckListItem(ptCheck,"Percentage (%)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptCheck = new Q3CheckListItem(ptParent,"Total bars/bins to plot histograms");
	ptCheck->setPixmap(0,*pixChildTree);
	ptHistogram_TotalBars = new Q3CheckListItem(ptCheck,"Edit/Change total bars/bins per Histogram: 40",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptCheck = new Q3CheckListItem(ptParent,"Chart type  (when using 'More reports' in 'Settings' page)");
	ptCheck->setPixmap(0,*pixChildTree);
	ptHistogram_TypeBarsOutlineGaussian= new Q3CheckListItem(ptCheck,"Gaussian + bars outline",Q3CheckListItem::RadioButton);
	ptHistogram_TypeBarsGaussian= new Q3CheckListItem(ptCheck,"Gaussian + bars",Q3CheckListItem::RadioButton);
	ptHistogram_TypeGaussian= new Q3CheckListItem(ptCheck,"Gaussian",Q3CheckListItem::RadioButton);
	ptHistogram_TypeBarsSpline= new Q3CheckListItem(ptCheck,"Bars + Fitting curve",Q3CheckListItem::RadioButton);
	ptHistogram_TypeSpline= new Q3CheckListItem(ptCheck,"Fitting curve",Q3CheckListItem::RadioButton);
	ptHistogram_TypeBarsOutline = new Q3CheckListItem(ptCheck,"Bars outline",Q3CheckListItem::RadioButton);
	ptHistogram_Type3D_Bars= new Q3CheckListItem(ptCheck,"3D-Bars",Q3CheckListItem::RadioButton);
	ptHistogram_TypeBars= new Q3CheckListItem(ptCheck,"Bars",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptCheck = new Q3CheckListItem(ptParent,"Chart size  (when using 'More reports' in 'Settings' page)");
	ptCheck->setPixmap(0,*pixChildTree);
	ptHistogram_Adaptive= new Q3CheckListItem(ptCheck,"Adaptive: Large, then smaller as number of charts increase",Q3CheckListItem::RadioButton);
	ptHistogram_Large= new Q3CheckListItem(ptCheck,"Large (800x400)",Q3CheckListItem::RadioButton);
	ptHistogram_Medium= new Q3CheckListItem(ptCheck,"Medium (400x200)",Q3CheckListItem::RadioButton);
	ptHistogram_Small= new Q3CheckListItem(ptCheck,"Small, mosaic display (200x100)",Q3CheckListItem::RadioButton);
	ptHistogram_Banner= new Q3CheckListItem(ptCheck,"Banner, maximize number of charts per page (900x120)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptItem = new Q3ListViewItem(ptParent,"Fields to include in chart");
	ptItem->setPixmap(0,*pixChildTree);
	ptHistogram_Cpk       = new Q3CheckListItem(ptItem,"Cpk",Q3CheckListItem::CheckBox);
	ptHistogram_Cp        = new Q3CheckListItem(ptItem,"Cp",Q3CheckListItem::CheckBox);
	ptHistogram_Max       = new Q3CheckListItem(ptItem,"Max. : Maximum result found (P100.0%)",Q3CheckListItem::CheckBox);
	ptHistogram_Min       = new Q3CheckListItem(ptItem,"Min. : Minimum result found (P0.0%)",Q3CheckListItem::CheckBox);
	ptHistogram_Outlier   = new Q3CheckListItem(ptItem,"Outliers",Q3CheckListItem::CheckBox);
	ptHistogram_Fails     = new Q3CheckListItem(ptItem,"Fails count",Q3CheckListItem::CheckBox);
	ptHistogram_Sigma     = new Q3CheckListItem(ptItem,"Sigma",Q3CheckListItem::CheckBox);
	ptHistogram_Quartile2 =	new Q3CheckListItem(ptItem,"Median - 2nd Quartile",Q3CheckListItem::CheckBox);
	ptHistogram_Mean      = new Q3CheckListItem(ptItem,"Mean",Q3CheckListItem::CheckBox);
	ptHistogram_ExecCount = new Q3CheckListItem(ptItem,"Executions count",Q3CheckListItem::CheckBox);
	ptHistogram_Limits    = new Q3CheckListItem(ptItem,"Test limits",Q3CheckListItem::CheckBox);
	ListView->insertItem(ptParent);

	//// Wafermap
    ptNodeWaferMap = ptParent = new Q3ListViewItem( ListView, "Wafer map, Strip map","Wafer map & Packaged map options");
	ptParent->setPixmap(0,*pixWafermap);

	ptItem = new Q3ListViewItem(ptParent,"Markers to draw on chart");
	ptItem->setPixmap(0,*pixChildTree);
	ptWafermap_MarkerBin = new Q3CheckListItem(ptItem,"Write Value in die (Measurement, Bin# or count)",Q3CheckListItem::CheckBox);
	ptWafermap_MarkerRetest = new Q3CheckListItem(ptItem,"Black Outline border for dies re-tested (soft / hard bin wafermaps only)",Q3CheckListItem::CheckBox);

#if 1 // BG: WaferMap orientation options
	ptItem = new Q3CheckListItem(ptParent,"Wafer Orientation options");
	ptItem->setPixmap(0,*pixChildTree);

	ptCheck = new Q3CheckListItem(ptItem,"Alarms");
	ptCheck->setPixmap(0,*pixChildTree);
	ptWafermap_ExportAlarm_NoFlat		= new Q3CheckListItem(ptCheck, "No flat location specified in data file", Q3CheckListItem::CheckBox);
	ptWafermap_ExportAlarm_FlatDiffers	= new Q3CheckListItem(ptCheck, "Detected flat location differs from the one specified in data file", Q3CheckListItem::CheckBox);

	ptCheck = new Q3CheckListItem(ptItem,"Define 'Positive X' Probing direction",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptWafermap_PositiveX_Detect= new Q3CheckListItem(ptCheck,"Auto : Detect from data file",Q3CheckListItem::RadioButton);
	ptWafermap_PositiveX_Left  = new Q3CheckListItem(ptCheck,"Left : Positive X increases going Left",Q3CheckListItem::RadioButton);
	ptWafermap_PositiveX_Right = new Q3CheckListItem(ptCheck,"Right: Positive X increases going Right (default)",Q3CheckListItem::RadioButton);

	ptCheck = new Q3CheckListItem(ptItem,"Define 'Positive Y' Probing direction");
	ptCheck->setPixmap(0,*pixChildTree);
	ptWafermap_PositiveY_Detect= new Q3CheckListItem(ptCheck,"Auto: Detect from data file",Q3CheckListItem::RadioButton);
	ptWafermap_PositiveY_Up    = new Q3CheckListItem(ptCheck,"Up  : Positive Y increases going Up",Q3CheckListItem::RadioButton);
	ptWafermap_PositiveY_Down  = new Q3CheckListItem(ptCheck,"Down: Positive Y increases going Down (default)",Q3CheckListItem::RadioButton);

#endif

	ptItem = new Q3CheckListItem(ptParent,"Visual options: Mirror axis, Force Round shape, full wafer, etc...");
	ptItem->setPixmap(0,*pixChildTree);
	ptWafermap_AlwaysRound				= new Q3CheckListItem(ptItem, "Always round: Resize dies to always make the wafer look round",Q3CheckListItem::CheckBox);
	ptWafermap_FullWafer				= new Q3CheckListItem(ptItem, "Full wafer: Display full wafermap (device filtering restricted to samples only)",Q3CheckListItem::CheckBox);
	ptWafermap_MirrorY					= new Q3CheckListItem(ptItem, "Y-axis swap: Swap Top dies with Bottom dies",Q3CheckListItem::CheckBox);
	ptWafermap_MirrorX					= new Q3CheckListItem(ptItem, "X-axis swap: Swap Left dies with Right dies",Q3CheckListItem::CheckBox);

	ptCheck = new Q3CheckListItem(ptParent,"Retest policy");
	ptCheck->setPixmap(0,*pixChildTree);
	ptWafermap_RetestPolicy_Last= new Q3CheckListItem(ptCheck,"Last bin: Use Last bin value found for each die (default)",Q3CheckListItem::RadioButton);
	ptWafermap_RetestPolicy_Highest= new Q3CheckListItem(ptCheck,"Highest bin: promote the highest bin value for each die",Q3CheckListItem::RadioButton);

	ptCheck = new Q3CheckListItem(ptParent,"Chart size (HTML reports)");
	ptCheck->setPixmap(0,*pixChildTree);
	ptWafermap_Adaptive= new Q3CheckListItem(ptCheck,"Adaptive: Large, then smaller as number of charts increase",Q3CheckListItem::RadioButton);
	ptWafermap_Large= new Q3CheckListItem(ptCheck,"Large",Q3CheckListItem::RadioButton);
	ptWafermap_Medium= new Q3CheckListItem(ptCheck,"Medium",Q3CheckListItem::RadioButton);
	ptWafermap_Small= new Q3CheckListItem(ptCheck,"Small, mosaic display",Q3CheckListItem::RadioButton);

	ptItem = new Q3ListViewItem(ptParent,"Comparing wafers");
	ptItem->setPixmap(0,*pixChildTree);
	ptWafermap_IncludeDeltaYieldSection = new Q3CheckListItem(ptItem,"Include delta yield section",Q3CheckListItem::CheckBox);
	ptWafermap_IncludeDieMismatchTable = new Q3CheckListItem(ptItem,"Include die mismatch table (may be big if a lot of mismatching dies)",Q3CheckListItem::CheckBox);
	ptWafermap_CompareAnySize = new Q3CheckListItem(ptItem,"Accept comparing wafers with different size",Q3CheckListItem::CheckBox);

	ptCheck = new Q3CheckListItem(ptParent,"Low-Yield Patterns detection (Wafer Stacking)");
	ptCheck->setPixmap(0,*pixChildTree);

	QString strWafermap_StackedLowYieldAlarmContent = "Edit/Change Low-Yield threshold:";
	float fOptionStorageDevice = 33.0f;
	bool bOptionConversionRslt;
	fOptionStorageDevice = ReportOptions.GetOption("wafer","low_yield_pattern").toFloat(&bOptionConversionRslt);

	// ptWafermap_StackedLowYieldAlarm = new Q3CheckListItem(ptCheck,"Edit/Change Low-Yield threshold: 33%",Q3CheckListItem::RadioButton);

	ptWafermap_StackedLowYieldAlarm = new Q3CheckListItem(ptCheck,strWafermap_StackedLowYieldAlarmContent,Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptItem = new Q3ListViewItem(ptParent,"Wafermaps to include in report");
	ptItem->setPixmap(0,*pixChildTree);

	ptCheck = new Q3CheckListItem(ptParent,"Parametric stacked wafermap",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptWafermap_StackedParametricMedian	= new Q3CheckListItem(ptCheck,"Use median",Q3CheckListItem::RadioButton);
	ptWafermap_StackedParametricMean	= new Q3CheckListItem(ptCheck,"Use mean",Q3CheckListItem::RadioButton);

	ptCheck = new Q3CheckListItem(ptParent, ptCheck, "Bin stacked wafermap",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);

#if 0 // Waiting for new options
	ptWafermap_StackedHighestFailingBin	= new Q3CheckListItem(ptCheck,"Highest Failing bin",Q3CheckListItem::RadioButton);
	ptWafermap_StackedHighestBin		= new Q3CheckListItem(ptCheck,"Highest bin",Q3CheckListItem::RadioButton);
	ptWafermap_StackedPassFailAtLeast	= new Q3CheckListItem(ptCheck,"Pass/Fail (display Pass if at least one die is Pass)",Q3CheckListItem::RadioButton);
#endif

	ptWafermap_StackedPassFailAll		= new Q3CheckListItem(ptCheck,"Pass/Fail (display Pass ONLY IF all die are Pass)",Q3CheckListItem::RadioButton);
	ptWafermap_StackedBinCount			= new Q3CheckListItem(ptCheck,"Bin count",Q3CheckListItem::RadioButton);

	ptCheck								= new Q3CheckListItem(ptItem,"Die-To-Die Mismatch (when comparing 2 wafers)",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptWafermap_DieMismatchBinToBin		= new Q3CheckListItem(ptCheck,"Bin to bin color correlation",Q3CheckListItem::CheckBox);
	ptWafermap_DieMismatchBin			= new Q3CheckListItem(ptCheck,"Bin mismatch",Q3CheckListItem::CheckBox);

	ptWafermap_Stacked					= new Q3CheckListItem(ptItem,"One combined wafer per group/query (Wafer Stacking)",Q3CheckListItem::CheckBox);
	ptWafermap_Individual				= new Q3CheckListItem(ptItem,"All individual wafers",Q3CheckListItem::CheckBox);

	//// Test statistics
    ptNodeTestStatistics = ptParent = new Q3ListViewItem( ListView, "Test statistics","Table options");
	ptParent->setPixmap(0,*pixTestStatistics);

	// Auto generate generic Galaxy tests for statistics
	ptCheck = new Q3CheckListItem(ptParent,"Generic Galaxy Parameters",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptStatistics_ExcludeGenericTests = new Q3CheckListItem(ptCheck,"Hide generic Galaxy parameters",Q3CheckListItem::RadioButton);
	ptStatistics_IncludeGenericTests= new Q3CheckListItem(ptCheck,"Show generic Galaxy parameters (Soft_Bin, Hard_Bin, Die_X, Die_Y, Testing_Site, Part_Id)",Q3CheckListItem::RadioButton);

	ptItem = new Q3ListViewItem(ptParent,"Correlation ALARM criteria (when comparing files/groups)");
	ptItem->setPixmap(0,*pixChildTree);
	ptCustomizeCpkDriftAlarm = new Q3CheckListItem(ptItem,"Edit/change Cpk Alarm limit...",Q3CheckListItem::RadioButton);
	ptCustomizeSigmaDriftAlarm = new Q3CheckListItem(ptItem,"Edit/change Sigma Alarm limit...",Q3CheckListItem::RadioButton);
	ptCustomizeMeanDriftAlarm = new Q3CheckListItem(ptItem,"Edit/change Mean Alarm limit...",Q3CheckListItem::RadioButton);
	ptStatistics_CorrCheckCpk = new Q3CheckListItem(ptItem,"Show Alarm if Cpk value drifts: 33%",Q3CheckListItem::CheckBox);
	ptStatistics_CorrCheckSigma = new Q3CheckListItem(ptItem,"Show Alarm if Sigma value drifts: 1%",Q3CheckListItem::CheckBox);
	ptStatistics_CorrCheckMean = new Q3CheckListItem(ptItem,"Show Alarm if Mean drifts: 5%",Q3CheckListItem::CheckBox);

	ptCheck = new Q3CheckListItem(ptParent,"Correlation ALARM formula for the Mean");
	ptCheck->setPixmap(0,*pixChildTree);
	ptStatistics_CorrAbsoluteDrift= new Q3CheckListItem(ptCheck,"Percentage of value drift",Q3CheckListItem::RadioButton);
	ptStatistics_CorrLimitsDrift= new Q3CheckListItem(ptCheck,"Percentage of limits space drift",Q3CheckListItem::RadioButton);

	ptItem = new Q3ListViewItem(ptParent,"Statistics WARNINGS & ALARMS criteria");
	ptItem->setPixmap(0,*pixChildTree);
	ptCustomizeTestYieldRedAlarm = new Q3CheckListItem(ptItem,"Edit/change 'Test Yield' Alarm limit (Red)...",Q3CheckListItem::RadioButton);
	ptCustomizeTestYieldYellowAlarm = new Q3CheckListItem(ptItem,"Edit/change 'Test Yield' Warning limit (Yellow)...",Q3CheckListItem::RadioButton);
	ptCustomizeCpkRedAlarm = new Q3CheckListItem(ptItem,"Edit/change 'Cpk' Alarm limit (Red)...",Q3CheckListItem::RadioButton);
	ptCustomizeCpkYellowAlarm = new Q3CheckListItem(ptItem,"Edit/change 'Cpk' Warning limit (Yellow)...",Q3CheckListItem::RadioButton);
	ptStatistics_RedAlarmTestYield= new Q3CheckListItem(ptItem,"Show 'Test yield' Alarm (Red) if under: 80%",Q3CheckListItem::CheckBox);
	ptStatistics_YellowAlarmTestYield= new Q3CheckListItem(ptItem,"Show 'Test yield' Warning (Yellow) if under: 90%",Q3CheckListItem::CheckBox);
	ptStatistics_RedAlarmCpk = new Q3CheckListItem(ptItem,"Show 'Cpk' Alarm (Red) if under: 1.33",Q3CheckListItem::CheckBox);
	ptStatistics_YellowAlarmCpk = new Q3CheckListItem(ptItem,"Show 'Cpk' Warning (Yellow) if under: 1.67",Q3CheckListItem::CheckBox);

	ptCheck = new Q3CheckListItem(ptParent,"Sorting mode");
	ptCheck->setPixmap(0,*pixChildTree);
	ptStatistics_SortR_and_R= new Q3CheckListItem(ptCheck,"Sort by gage R&R% value (by decreasing R&R% value)",Q3CheckListItem::RadioButton);
	ptStatistics_SortCpk= new Q3CheckListItem(ptCheck,"Sort by Cpk (of Reference when comparing files/groups)",Q3CheckListItem::RadioButton);
	ptStatistics_SortCp= new Q3CheckListItem(ptCheck,"Sort by Cp (of Reference when comparing files/groups)",Q3CheckListItem::RadioButton);
	ptStatistics_SortSigma= new Q3CheckListItem(ptCheck,"Sort by Sigma (of Reference when comparing files/groups)",Q3CheckListItem::RadioButton);
	ptStatistics_SortMeanShift= new Q3CheckListItem(ptCheck,"Sort by Mean-shift (when comparing files/groups)",Q3CheckListItem::RadioButton);
	ptStatistics_SortMean= new Q3CheckListItem(ptCheck,"Sort by Mean (of Reference when comparing files/groups)",Q3CheckListItem::RadioButton);
	ptStatistics_SortTestFlowID= new Q3CheckListItem(ptCheck,"Sort by Test Flow ID (test execution order)",Q3CheckListItem::RadioButton);
	ptStatistics_SortTestName= new Q3CheckListItem(ptCheck,"Sort by Test Name",Q3CheckListItem::RadioButton);
	ptStatistics_SortTestNumber= new Q3CheckListItem(ptCheck,"Sort by Test Number",Q3CheckListItem::RadioButton);

	ptCheck = new Q3CheckListItem(ptParent,"Cp, Cpk formulas (for non-normal distributions)",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptStatistics_CpCpkDefaultFormula= new Q3CheckListItem(ptCheck,"Use standard Sigma formula",Q3CheckListItem::RadioButton);
	ptStatistics_CpCpkPercentileFormula = new Q3CheckListItem(ptCheck,"Use percentile formula (Cnpk)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);
	
	ptItem = new Q3ListViewItem(ptParent,"Fields to include in report table");
	ptItem->setPixmap(0,*pixChildTree);
	ptStatistics_SigmaInterQuartiles =	new Q3CheckListItem(ptItem,"IQR SD Interquartile Standard Deviation    [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_Interquartiles =		new Q3CheckListItem(ptItem,"IQR Interquartile range    [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_P99_5 =				new Q3CheckListItem(ptItem,"P99.5% (Percentile 99.5%)   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_P97_5 =				new Q3CheckListItem(ptItem,"P97.5% (Percentile 97.5%)   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_P90 =					new Q3CheckListItem(ptItem,"P90.0% (Percentile 90.0%)   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_Quartile3 =			new Q3CheckListItem(ptItem,"P75.0% Q3: 3rd Quartile   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_Quartile2 =			new Q3CheckListItem(ptItem,"Median: P50.0% 2nd Quartile   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_Quartile1 =			new Q3CheckListItem(ptItem,"P25.0% Q1: 1st Quartile   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_P10 =					new Q3CheckListItem(ptItem,"P10.0% (Percentile 10.0%)   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_P2_5 =					new Q3CheckListItem(ptItem,"P2.5% (Percentile 2.5%)   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_P0_5 =					new Q3CheckListItem(ptItem,"P0.5% (Percentile 0.5%)   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_Kurtosis =				new Q3CheckListItem(ptItem,"Kurtosis    [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_Skew =					new Q3CheckListItem(ptItem,"Skew    [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_GageP_T = new Q3CheckListItem(ptItem,"P/T Ratio (Gage Study)",Q3CheckListItem::CheckBox); 
	ptStatistics_GageTV = new Q3CheckListItem(ptItem,"TV Total Variation (Gage Study)",Q3CheckListItem::CheckBox);
	ptStatistics_GagePV = new Q3CheckListItem(ptItem,"PV Part Variation (Gage Study)",Q3CheckListItem::CheckBox);
	ptStatistics_GageGB = new Q3CheckListItem(ptItem,"GB GuardBand (Gage Study)",Q3CheckListItem::CheckBox);
	ptStatistics_GageR_and_R = new Q3CheckListItem(ptItem,"R&R (Gage Study)",Q3CheckListItem::CheckBox);
	ptStatistics_GageAV = new Q3CheckListItem(ptItem,"AV: Reproducibility(Gage Study)",Q3CheckListItem::CheckBox);
	ptStatistics_GageEV = new Q3CheckListItem(ptItem,"EV: Repeatability (Gage Study)",Q3CheckListItem::CheckBox);
	ptStatistics_Yield = new Q3CheckListItem(ptItem,"Yield",Q3CheckListItem::CheckBox);
	ptStatistics_CpkShift = new Q3CheckListItem(ptItem,"Cpk shift (when comparing files/groups)",Q3CheckListItem::CheckBox);
	ptStatistics_CpkHigh =				new Q3CheckListItem(ptItem,"CpkH (Cpu)   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_CpkLow =				new Q3CheckListItem(ptItem,"CpkL (Cpl)   [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_Cpk = new Q3CheckListItem(ptItem,"Cpk",Q3CheckListItem::CheckBox);
	ptStatistics_CpShift = new Q3CheckListItem(ptItem,"Cp shift (when comparing files/groups)",Q3CheckListItem::CheckBox);
	ptStatistics_Cp = new Q3CheckListItem(ptItem,"Cp",Q3CheckListItem::CheckBox);
	ptStatistics_MaxRange = new Q3CheckListItem(ptItem,"Max. Range (when comparing files/groups)",Q3CheckListItem::CheckBox);
	ptStatistics_Range = new Q3CheckListItem(ptItem,"Range",Q3CheckListItem::CheckBox);
	ptStatistics_Max = new Q3CheckListItem(ptItem,"Max. : Maximum result found (P100.0%)",Q3CheckListItem::CheckBox);
	ptStatistics_Min = new Q3CheckListItem(ptItem,"Min. : Minimum result found (P0.0%)",Q3CheckListItem::CheckBox);
	ptStatistics_6Sigma = new Q3CheckListItem(ptItem,"6 * Sigma",Q3CheckListItem::CheckBox);
	ptStatistics_3Sigma = new Q3CheckListItem(ptItem,"3 * Sigma",Q3CheckListItem::CheckBox);
	ptStatistics_2Sigma = new Q3CheckListItem(ptItem,"2 * Sigma",Q3CheckListItem::CheckBox);
	ptStatistics_SigmaShift = new Q3CheckListItem(ptItem,"Sigma shift (when comparing files/groups)",Q3CheckListItem::CheckBox);
	ptStatistics_Sigma = new Q3CheckListItem(ptItem,"Sigma",Q3CheckListItem::CheckBox);
	ptStatistics_T_Test = new Q3CheckListItem(ptItem,"Student's T-Test (when comparing two files/groups). Alpha=0.05",Q3CheckListItem::CheckBox);
	ptStatistics_MeanShift = new Q3CheckListItem(ptItem,"Mean shift (when comparing files/groups)",Q3CheckListItem::CheckBox);
	ptStatistics_Mean = new Q3CheckListItem(ptItem,"Mean",Q3CheckListItem::CheckBox);
	ptStatistics_Outlier = new Q3CheckListItem(ptItem,"Outliers count",Q3CheckListItem::CheckBox);
	ptStatistics_TestFlowID = new Q3CheckListItem(ptItem,"Test flow ID (test order in testing sequence) ",Q3CheckListItem::CheckBox);
	ptStatistics_FailBin = new Q3CheckListItem(ptItem,"Failing Bin associated with test/parameter",Q3CheckListItem::CheckBox);
	ptStatistics_Failp = new Q3CheckListItem(ptItem,"Failures % (over production)",Q3CheckListItem::CheckBox);
	ptStatistics_FailCount = new Q3CheckListItem(ptItem,"Failures count",Q3CheckListItem::CheckBox);
	ptStatistics_ExecCount = new Q3CheckListItem(ptItem,"Executions count",Q3CheckListItem::CheckBox);
	ptStatistics_StatsSource = new Q3CheckListItem(ptItem,"Statistics computation mode (eg: Samples, Summary,...)",Q3CheckListItem::CheckBox);
	ptStatistics_Shape = new Q3CheckListItem(ptItem,"Distribution shape [--Examinator Professional only--]",Q3CheckListItem::CheckBox);
	ptStatistics_SpecLimits = new Q3CheckListItem(ptItem,"Spec limits",Q3CheckListItem::CheckBox);
	ptStatistics_TestLimits = new Q3CheckListItem(ptItem,"Test limits",Q3CheckListItem::CheckBox);
	ptStatistics_TestType = new Q3CheckListItem(ptItem,"Test type (Parametric, Functional,...)",Q3CheckListItem::CheckBox);
	ptStatistics_TestName = new Q3CheckListItem(ptItem,"Test name",Q3CheckListItem::CheckBox);
	ListView->insertItem(ptParent);

	ptCheck = new Q3CheckListItem(ptParent,"How are Statistics computed",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptStatistics_UseSamplesOnly= new Q3CheckListItem(ptCheck,"From samples data only",Q3CheckListItem::RadioButton);
	ptStatistics_UseSummaryOnly = new Q3CheckListItem(ptCheck,"From summary data only",Q3CheckListItem::RadioButton);
	ptStatistics_UseSamplesThenSummary= new Q3CheckListItem(ptCheck,"From samples (if any), otherwise from summary ",Q3CheckListItem::RadioButton);
	ptStatistics_UseSummaryThenSamples = new Q3CheckListItem(ptCheck,"From summary data (if any), otherwise from samples",Q3CheckListItem::RadioButton);
	
	ListView->insertItem(ptParent);
	
	// Data processing options
    ptNodeDataProcessing = ptParent = new Q3ListViewItem( ListView, "Data Processing options (Test number handling, Sorting, Fail count, Outliers,...)","Analysis options");
	ptParent->setPixmap(0,*pixTasks);

	//// Outlier
	ptCheck = new Q3CheckListItem(ptParent,"Outlier removal",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixOutlier);
	ptCheck->setText(1,"Outlier filter options");
	ptOutlier_InlinerSigma= new Q3CheckListItem(ptCheck,"Exclude +/- N*Sigma range (centered on Mean). With N=3.0",Q3CheckListItem::RadioButton);
	ptOutlier_IQR = new Q3CheckListItem(ptCheck,"IQR based: LL=Q1-N*IQR, HL=Q3+N*IQR, With N=1.5",Q3CheckListItem::RadioButton);
	ptOutlier_sigma= new Q3CheckListItem(ptCheck,"+/- N*Sigma range (centered on Mean). With N=3.0",Q3CheckListItem::RadioButton);
	ptOutlier_300p= new Q3CheckListItem(ptCheck,"300% of limits space",Q3CheckListItem::RadioButton);
	ptOutlier_250p= new Q3CheckListItem(ptCheck,"250% of limits space",Q3CheckListItem::RadioButton);
	ptOutlier_200p= new Q3CheckListItem(ptCheck,"200% of limits space",Q3CheckListItem::RadioButton);
	ptOutlier_150p= new Q3CheckListItem(ptCheck,"150% of limits space",Q3CheckListItem::RadioButton);
	ptOutlier_100p= new Q3CheckListItem(ptCheck,"100% of limits space (ignore values outside the limits)",Q3CheckListItem::RadioButton);
	ptOutlier_none= new Q3CheckListItem(ptCheck,"none (keep all data)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// STDF Compliancy
	ptCheck = new Q3CheckListItem(ptParent,"Handling STDF Compliancy Issues",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"How to deal with non-compliant STDF files.");
	ptStdfCompliancy_Relaxed= new Q3CheckListItem(ptCheck,"Flexible: Examinator does its best to recover any data (default)",Q3CheckListItem::RadioButton);
	ptStdfCompliancy_Stringent= new Q3CheckListItem(ptCheck,"Stringent: Ignore records not in line with STDF specs.",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// Temporary STDF folder (for intermediate STDF files and uncompressing files)
	ptCheck = new Q3CheckListItem(ptParent,"Intermediate files (STDF & Zip): Define storage folder",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"Folder to use when building intermediate files");
	ptTempStdfFolder_RequestCustom= new Q3CheckListItem(ptCheck,"Other location...to specify now!",Q3CheckListItem::RadioButton);
#ifdef unix
	// Under unix, path is ${HOME}, or "/" if HOME is not defined
	char	*ptChar;
	QString	strLocation;

	ptChar = getenv("HOME");
	if(ptChar != NULL)
	{
		// the HOME variable is valid !
		strLocation.sprintf("Location: %s/", ptChar);
		ptTempStdfFolder_Custom = new Q3CheckListItem(ptCheck,strLocation,Q3CheckListItem::RadioButton);
	}
	else
	{
		// We miss the HOME variable...then use "/"
		ptTempStdfFolder_Custom = new Q3CheckListItem(ptCheck,"Location: /",Q3CheckListItem::RadioButton);
	}
#else
	// Under Windows, path is c:/
	ptTempStdfFolder_Custom = new Q3CheckListItem(ptCheck,"Location: C:\\",Q3CheckListItem::RadioButton);
#endif
	ptTempStdfFolder_Default= new Q3CheckListItem(ptCheck,"Same folder as Data files (default)",Q3CheckListItem::RadioButton);

	// Multiple Parametric tests
	ptItem = new Q3CheckListItem(ptParent,"MRP: Multi-Results Parametric tests",Q3CheckListItem::RadioButtonController);
	ptItem->setPixmap(0,*pixChildTree);
	ptItem->setText(1,"MRP: Multi-Results Parametric");

	// Add Merge options section
	ptCheck = new Q3CheckListItem(ptItem, "Merge option");
	ptCheck->setPixmap(0,*pixChildTree);

	ptMultiParametric_Options_KeepOne	= new Q3CheckListItem(ptCheck,"Merge MRP values based on 'Merge criteria': makes one single result per run.", Q3CheckListItem::CheckBox);

	// Add Merge criteria section
	ptCheck = new Q3CheckListItem(ptItem, "Merge criteria", Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);

	ptMultiParametric_Criteria_Last		= new Q3CheckListItem(ptCheck,"Last MRP value in run", Q3CheckListItem::RadioButton);
	ptMultiParametric_Criteria_First	= new Q3CheckListItem(ptCheck,"First MRP value in run", Q3CheckListItem::RadioButton);
	ptMultiParametric_Criteria_Median	= new Q3CheckListItem(ptCheck,"Median (of MRP values)", Q3CheckListItem::RadioButton);
	ptMultiParametric_Criteria_Mean		= new Q3CheckListItem(ptCheck,"Mean (of MRP values)", Q3CheckListItem::RadioButton);
	ptMultiParametric_Criteria_Max		= new Q3CheckListItem(ptCheck,"Max (of MRP values)", Q3CheckListItem::RadioButton);
	ptMultiParametric_Criteria_Min		= new Q3CheckListItem(ptCheck,"Min (of MRP values)", Q3CheckListItem::RadioButton);

	// Add Merge/Split section
	ptCheck = new Q3CheckListItem(ptItem,"Merge or Split MRP?",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);

	ptMultiParametric_NoMerge	= new Q3CheckListItem(ptCheck,"Do NOT merge: Show each pin/result as a different test",Q3CheckListItem::RadioButton);
	ptMultiParametric_Merge		= new Q3CheckListItem(ptCheck,"Merge: Merge ALL pins/results under same test name",Q3CheckListItem::RadioButton);

	ListView->insertItem(ptParent);

	// Failure count mode
	ptCheck = new Q3CheckListItem(ptParent,"Failures count mode",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"Failures");
	ptFailures_All= new Q3CheckListItem(ptCheck,"Continue on Fail: count ALL failures for all devices tested",Q3CheckListItem::RadioButton);
	ptFailures_First= new Q3CheckListItem(ptCheck,"Stop on First Fail: only count first failure per device tested",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// Sorting mode
	ptCheck = new Q3CheckListItem(ptParent,"Sorting mode",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"Test data sorting mode");
	// ptSort_SortByDate= new Q3CheckListItem(ptCheck,"By date: sort data by testing date",Q3CheckListItem::RadioButton);
	ptSort_SortNone= new Q3CheckListItem(ptCheck,"None: process data files using import order",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// Normalize / scale data
	ptCheck = new Q3CheckListItem(ptParent,"Normalize / Scale",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptScaling_Normalized	= new Q3CheckListItem(ptCheck,"Normalize all units (eg: V, A, etc)",Q3CheckListItem::RadioButton);
	ptScaling_ToLimits		= new Q3CheckListItem(ptCheck,"Re-Scale all units to Limits units",Q3CheckListItem::RadioButton);
	ptScaling_Smart			= new Q3CheckListItem(ptCheck,"Re-Scale to most approriate units (eg: mV, uV, etc)",Q3CheckListItem::RadioButton);
	ptScaling_None			= new Q3CheckListItem(ptCheck,"No smart scaling on data computation (leave results in default units)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);
	
	// PartID display
	ptNodePartID = ptCheck = new Q3CheckListItem(ptParent,"PartID display",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"Show/Hide PartID info");
	ptPartID_Show = new Q3CheckListItem(ptCheck,"Show PartID in reports & charts (Typical to Characterization missions, requires more memory)",Q3CheckListItem::RadioButton);
	ptPartID_Hide = new Q3CheckListItem(ptCheck,"Hide PartID (default mode, faster)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// Test Number / names handling
	ptCheck = new Q3CheckListItem(ptParent,"Handling duplicated Test Numbers & Names",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"Extracting test number / name");
	ptDuplicate_NoMerge= new Q3CheckListItem(ptCheck,"Never merge tests with identical test number if test name not matching",Q3CheckListItem::RadioButton);
	ptDuplicate_MergeName= new Q3CheckListItem(ptCheck,"Ignore test#: Always merge tests with identical name",Q3CheckListItem::RadioButton);
	ptDuplicate_Merge= new Q3CheckListItem(ptCheck,"Ignore test name: Always merge tests with identical test number",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	//// Speed Optimization
    ptNodeSpeedOptimization = ptParent = new Q3ListViewItem( ListView, "Speed Optimization","Optimize analysis Speed Vs. Datasets size");
	ptParent->setPixmap(0,*pixStopwatch);

	//// Ignore data samples, only process SUMMARY records
	ptCheck = new Q3CheckListItem(ptParent,"Database queries: Use SUMMARY records instead of data samples",Q3CheckListItem::RadioButtonController);
	ptSpeedDatabaseSummary = ptCheck;
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"Use Summary records Vs. all samples");
	ptSpeedAdvSummaryDB_Never= new Q3CheckListItem(ptCheck,"Never use summary, read all samples (slowest but accurate)",Q3CheckListItem::RadioButton);
	ptSpeedAdvSummaryDB_500MB = new Q3CheckListItem(ptCheck,"Only if processing at least 500Mb of data samples",Q3CheckListItem::RadioButton);
	ptSpeedAdvSummaryDB_400MB = new Q3CheckListItem(ptCheck,"Only if processing at least 400Mb of data samples",Q3CheckListItem::RadioButton);
	ptSpeedAdvSummaryDB_300MB = new Q3CheckListItem(ptCheck,"Only if processing at least 300Mb of data samples",Q3CheckListItem::RadioButton);
	ptSpeedAdvSummaryDB_200MB = new Q3CheckListItem(ptCheck,"Only if processing at least 200Mb of data samples",Q3CheckListItem::RadioButton);
	ptSpeedAdvSummaryDB_100MB = new Q3CheckListItem(ptCheck,"Only if processing at least 100Mb of data samples",Q3CheckListItem::RadioButton);
	ptSpeedAdvSummaryDB_50MB  = new Q3CheckListItem(ptCheck,"Only if processing at least 50Mb of data samples",Q3CheckListItem::RadioButton);
	ptSpeedAdvSummaryDB_Always= new Q3CheckListItem(ptCheck,"Always use summary, ignore data samples (fastest)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	//// Computing advanced statistics (Heavy data crunching)
	ptCheck = new Q3CheckListItem(ptParent,"Computing advanced statistics, distribution shape (Heavy data crunching)",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"Computing Advanced Statistics");
	ptSpeedAdvStats_Never= new Q3CheckListItem(ptCheck,"Never compute advanced statistics",Q3CheckListItem::RadioButton);
	ptSpeedAdvStats_500MB = new Q3CheckListItem(ptCheck,"Only if processing 500Mb of data or less",Q3CheckListItem::RadioButton);
	ptSpeedAdvStats_400MB = new Q3CheckListItem(ptCheck,"Only if processing 400Mb of data or less",Q3CheckListItem::RadioButton);
	ptSpeedAdvStats_300MB = new Q3CheckListItem(ptCheck,"Only if processing 300Mb of data or less",Q3CheckListItem::RadioButton);
	ptSpeedAdvStats_200MB = new Q3CheckListItem(ptCheck,"Only if processing 200Mb of data or less",Q3CheckListItem::RadioButton);
	ptSpeedAdvStats_100MB = new Q3CheckListItem(ptCheck,"Only if processing 100Mb of data or less",Q3CheckListItem::RadioButton);
	ptSpeedAdvStats_50MB  = new Q3CheckListItem(ptCheck,"Only if processing 50Mb of data or less",Q3CheckListItem::RadioButton);
	ptSpeedAdvStats_Always= new Q3CheckListItem(ptCheck,"Always compute Skew, Kurtosis, Quartiles, Median and distribution Shape",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	//// Output
	ptNodeReportOutput = ptParent = new Q3ListViewItem( ListView, "Report output", "Output options");
	ptParent->setPixmap(0,*pixOutput);
	ptOutputLocation = new Q3CheckListItem(ptParent,"Output location",Q3CheckListItem::RadioButtonController);
	ptOutputLocation->setPixmap(0,*pixChildTree);
	ptCustomizeReportOutputPath = new Q3CheckListItem(ptOutputLocation,"Other location...to specify now!",Q3CheckListItem::RadioButton);
#ifdef unix
	// Under unix, path is ${HOME}, or "/" if HOME is not defined
	ptChar = getenv("HOME");
	if(ptChar != NULL)
	{
		// the HOME variable is valid !
		strLocation.sprintf("Location: %s/", ptChar);
		ptOutput_PathCustom = new Q3CheckListItem(ptOutputLocation,strLocation,Q3CheckListItem::RadioButton);
	}
	else
	{
		// We miss the HOME variable...then use "/"
		ptOutput_PathCustom = new Q3CheckListItem(ptOutputLocation,"Location: /",Q3CheckListItem::RadioButton);
	}
#else
	// Under Windows, path is c:/
	ptOutput_PathCustom = new Q3CheckListItem(ptOutputLocation,"Location: C:\\",Q3CheckListItem::RadioButton);
#endif
	ptOutput_PathFile= new Q3CheckListItem(ptOutputLocation,"Same as the first Data file selected",Q3CheckListItem::RadioButton);

	// Test labels truncation
	ptCheck = new Q3CheckListItem(ptParent,"Truncating test names",Q3CheckListItem::RadioButtonController);
	ptCheck->setText(1,"HTML/Word/PDF/PPT reports only");
	ptCheck->setPixmap(0,*pixChildTree);
	ptOutputTruncateTestNames = new Q3CheckListItem(ptCheck,"Truncate test names to length: 32",Q3CheckListItem::RadioButton);
	new Q3CheckListItem(ptCheck,"No truncation: keep all characters in test labels",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// Paper orientation
	ptCheck = new Q3CheckListItem(ptParent,"Paper orientation (Word & PDF reports only)",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptOutputPaperFormat_Landscape= new Q3CheckListItem(ptCheck,"Landscape",Q3CheckListItem::RadioButton);
	ptOutputPaperFormat_Portrait= new Q3CheckListItem(ptCheck,"Portrait",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// Paper size
	ptCheck = new Q3CheckListItem(ptParent,"Paper size (A4/Letter)",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptOutputPaperSize_A4= new Q3CheckListItem(ptCheck,"A4 (21 x 29.7cm - 8.27 x 11.69 inches)",Q3CheckListItem::RadioButton);
	ptOutputPaperSize_Letter= new Q3CheckListItem(ptCheck,"US Letter (8.5 x 11 inches - 21.6 x 27.9 cm)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	// Customize Report Front page
	ptCheck = new Q3CheckListItem(ptParent,"Home Page Contents (Edit Text & Logo) ",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"Word, PowerPoint & PDF reports only");
	ptCustomizeReportHomePage = new Q3CheckListItem(ptCheck,"Click here...and customize report's home page!",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptCheck = new Q3CheckListItem(ptParent,"Output format",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptOutput_Interactive= new Q3CheckListItem(ptCheck,"Interactive only (no document created)",Q3CheckListItem::RadioButton);
	ptOutput_Pdf= new Q3CheckListItem(ptCheck,"PDF (.PDF Adobe document)",Q3CheckListItem::RadioButton);
	ptOutput_Ppt= new Q3CheckListItem(ptCheck,"PowerPoint Slides (.PPT document)",Q3CheckListItem::RadioButton);
	ptOutput_CSV= new Q3CheckListItem(ptCheck,"Excel (.CSV ASCII file)",Q3CheckListItem::RadioButton);
	ptOutput_Word= new Q3CheckListItem(ptCheck,"MS-Word (.DOC Microsoft Word document)",Q3CheckListItem::RadioButton);
	ptOutput_Html= new Q3CheckListItem(ptCheck,"Web (HTML pages)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);
	// Embed fonts ?
	ptCheck = new Q3CheckListItem(ptParent,"About fonts in PDF", Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptEmbedFontsInPdf=new Q3CheckListItem(ptCheck,"Embed fonts in PDF files (maximize size)", Q3CheckListItem::RadioButton);
	ptNoEmbedFontsInPdf=new Q3CheckListItem(ptCheck,"Do not embed fonts in PDF files (minimize size)", Q3CheckListItem::RadioButton);

	QString efinpdf=ReportOptions.GetOption("output", "embed_fonts_in_pdf").toString();
	if (efinpdf=="true")
		ptEmbedFontsInPdf->setSelected(true);
	else
		ptNoEmbedFontsInPdf->setSelected(true);
	ListView->insertItem(ptParent);


	//// Examinator-Monitoring
    ptNodeMonitoring = ptParent = new Q3ListViewItem( ListView, "Manufacturing Status Web pages","Web pages options");
	ptParent->setPixmap(0,*pixOutput);

	ptCheck = new Q3CheckListItem(ptParent,"History size",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptCheck->setText(1,"HTML history pages to keep");
	ptMonitoring_History_all = new Q3CheckListItem(ptCheck,"Keep ALL history pages (never purge)",Q3CheckListItem::RadioButton);
	ptMonitoring_History_1year= new Q3CheckListItem(ptCheck,"Keep 1 year of history pages",Q3CheckListItem::RadioButton);
	ptMonitoring_History_6month= new Q3CheckListItem(ptCheck,"Keep 6 months of history pages",Q3CheckListItem::RadioButton);
	ptMonitoring_History_5month = new Q3CheckListItem(ptCheck,"Keep 5 months of history pages",Q3CheckListItem::RadioButton);
	ptMonitoring_History_4month = new Q3CheckListItem(ptCheck,"Keep 4 months of history pages",Q3CheckListItem::RadioButton);
	ptMonitoring_History_3month = new Q3CheckListItem(ptCheck,"Keep 3 months of history pages",Q3CheckListItem::RadioButton);
	ptMonitoring_History_2month = new Q3CheckListItem(ptCheck,"Keep 2 months of history pages",Q3CheckListItem::RadioButton);
	ptMonitoring_History_1month = new Q3CheckListItem(ptCheck,"Keep 1 month of history pages",Q3CheckListItem::RadioButton);
	ptMonitoring_History_3week = new Q3CheckListItem(ptCheck,"Keep 3 weeks of history pages",Q3CheckListItem::RadioButton);
	ptMonitoring_History_2week = new Q3CheckListItem(ptCheck,"Keep 2 weeks of history pages",Q3CheckListItem::RadioButton);
	ptMonitoring_History_1week = new Q3CheckListItem(ptCheck,"Keep 1 week of history pages",Q3CheckListItem::RadioButton);
	ptMonitoring_History_none = new Q3CheckListItem(ptCheck,"No history (only keep pages of the day)",Q3CheckListItem::RadioButton);
	ListView->insertItem(ptParent);

	ptItem = new Q3CheckListItem(ptParent,"TESTERS pages fields");
	ptItem->setPixmap(0,*pixChildTree);
	ptMonitoring_Testers_Chart = new Q3CheckListItem(ptItem,"Yield chart",Q3CheckListItem::CheckBox);
	ptMonitoring_Testers_Alarm = new Q3CheckListItem(ptItem,"Alarm level",Q3CheckListItem::CheckBox);
	ptMonitoring_Testers_Yield = new Q3CheckListItem(ptItem,"Yield level",Q3CheckListItem::CheckBox);
	ptMonitoring_Testers_Fail = new Q3CheckListItem(ptItem,"Total Failing parts",Q3CheckListItem::CheckBox);
	ptMonitoring_Testers_Good = new Q3CheckListItem(ptItem,"Total Good parts",Q3CheckListItem::CheckBox);
	ptMonitoring_Testers_Parts = new Q3CheckListItem(ptItem,"Total parts tested",Q3CheckListItem::CheckBox);
	ptMonitoring_Testers_Program = new Q3CheckListItem(ptItem,"Program name",Q3CheckListItem::CheckBox);
	ptMonitoring_Testers_Operator = new Q3CheckListItem(ptItem,"Operator name",Q3CheckListItem::CheckBox);
	ptMonitoring_Testers_Products = new Q3CheckListItem(ptItem,"Product tested",Q3CheckListItem::CheckBox);

	ptItem = new Q3CheckListItem(ptParent,"PRODUCTS pages fields");
	ptItem->setPixmap(0,*pixChildTree);	
	ptMonitoring_Products_Chart = new Q3CheckListItem(ptItem,"Yield chart",Q3CheckListItem::CheckBox);
	ptMonitoring_Products_Alarm = new Q3CheckListItem(ptItem,"Alarm level",Q3CheckListItem::CheckBox);
	ptMonitoring_Products_Yield = new Q3CheckListItem(ptItem,"Yield level",Q3CheckListItem::CheckBox);
	ptMonitoring_Products_Fail = new Q3CheckListItem(ptItem,"Total Failing parts",Q3CheckListItem::CheckBox);
	ptMonitoring_Products_Good = new Q3CheckListItem(ptItem,"Total Good parts",Q3CheckListItem::CheckBox);
	ptMonitoring_Products_Parts = new Q3CheckListItem(ptItem,"Total parts tested",Q3CheckListItem::CheckBox);

	ptItem = new Q3ListViewItem(ptParent,"HOME page fields");
	ptItem->setPixmap(0,*pixChildTree);
	ptMonitoring_Home_Testers = new Q3CheckListItem(ptItem,"Show total testers under test",Q3CheckListItem::CheckBox);
	ptMonitoring_Home_Products = new Q3CheckListItem(ptItem,"Show total products under test",Q3CheckListItem::CheckBox);
	ptItem->setVisible(false);;	// Hide for now!
	////

	//// Database
    ptNodeDatabaseOptions = new Q3ListViewItem( ListView, "Databases options","Databases options");
	ptNodeDatabaseOptions->setPixmap(0,*pixGexApplication);
	ptCheck = new Q3CheckListItem(ptNodeDatabaseOptions,"RDB: default parameters to extract",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptDatabase_RDB_DefaultParameters_All = new Q3CheckListItem(ptCheck,"Extract all parameters",Q3CheckListItem::RadioButton);
	ptDatabase_RDB_DefaultParameters_None = new Q3CheckListItem(ptCheck,"Extract no parameter",Q3CheckListItem::RadioButton);
	ptCheck = new Q3CheckListItem(ptNodeDatabaseOptions,"Locations",Q3CheckListItem::RadioButtonController);
	ptCheck->setPixmap(0,*pixChildTree);
	ptDatabase_LocalPath = new Q3CheckListItem(ptCheck,"Local databases path: (default)",Q3CheckListItem::RadioButton);
	ptDatabase_ServerPath = new Q3CheckListItem(ptCheck,"Server databases path: (default)",Q3CheckListItem::RadioButton);
	////

	// Default options:
	ptOutput_Html->setOn(true);				// HTML report.
	ptOutput_PathFile->setOn(true);			// Path= Path of first Data file.

	// monitoring
	ptMonitoring_History_1week->setOn(true);	// Keep 1 week of History HTML status pages
	ptMonitoring_Products_Yield->setOn(true);
	ptMonitoring_Products_Alarm->setOn(true);
	ptMonitoring_Products_Chart->setOn(true);
	ptMonitoring_Testers_Products->setOn(true);
	ptMonitoring_Testers_Program->setOn(true);
	ptMonitoring_Testers_Yield->setOn(true);
	ptMonitoring_Testers_Chart->setOn(true);

	// Show/Hide PartID: 
	ptPartID_Hide->setOn(true);				// No PartID shown (default)

	// Sorting
	// ptSort_SortNone->setOn(true);			// No sorting on data order CRO_cleanUp

	// Fail count mode
	ptFailures_All->setOn(true);			// Count ALL failures in flow.

	// Multi-parametric merge/split
	ptMultiParametric_Merge->setOn(true);				// Merge Multi-parametric pins under a single test name.

	// Multi-parametric merge criteria
	ptMultiParametric_Criteria_First->setOn(true);		// Use first value as merger result

	// Multi-parametric merge option
	ptMultiParametric_Options_KeepOne->setOn(false);	// Use all results collected

	// outlier
	ptOutlier_none->setOn(true);			// Outlier=none

	// statistics table
	ptStatistics_TestName->setOn(true);		// Test name (test number always part of report)
	ptStatistics_TestLimits->setOn(true);		// Test limits
	ptStatistics_ExecCount->setOn(true);	// Execution count
	ptStatistics_FailCount->setOn(true);	// Failures count
	ptStatistics_Mean->setOn(true);			// Mean
	ptStatistics_MeanShift->setOn(true);	// Mean Shift (if comparing files/groups)
	ptStatistics_T_Test->setOn(true);		// Student's T-Test
	ptStatistics_Sigma->setOn(true);		// Std. Dev.
	ptStatistics_SigmaShift->setOn(true);	// Std. Dev. shift (if comparing files/groups)
	ptStatistics_Range->setOn(true);		// Range
	ptStatistics_Cp->setOn(true);			// Cp
	ptStatistics_CpShift->setOn(true);		// Cp shift (if comparing files/groups)
	ptStatistics_Cpk->setOn(true);			// Cpk
	ptStatistics_CpkShift->setOn(true);		// Cpk shift (if comparing files/groups)
	ptStatistics_SortTestNumber->setOn(true);	// Sorting: test number
	ptStatistics_CorrCheckMean->setOn(true);	// Alarm set on: Mean shift
	ptStatistics_CorrCheckSigma->setOn(true);	// Alarm set on: Sigma shift
	ptStatistics_CorrCheckCpk->setOn(true);		// Alarm set on: Cpk shift
	ptStatistics_CorrAbsoluteDrift->setOn(true);	// Drift formula: percentage of value (not over limits)

	ptStatistics_IncludeGenericTests->setOn(true);// Generic Galaxy Tests
	ptStatistics_ExcludeGenericTests->setOn(false);

	// Wafermap
	ptWafermap_Stacked->setOn(true);					// Show bin Stacked wafermap.
	ptWafermap_StackedParametricMean->setOn(true);		// Build Parametric Stacked wafermap with mean.
	ptWafermap_StackedParametricMedian->setOn(false);	// Build Parametric Stacked wafermap with median.
	ptWafermap_StackedBinCount->setOn(true);			// Build Bin stacked wafermap using bin count
	ptWafermap_Adaptive->setOn(true);					// Chart size= auto
	ptWafermap_MarkerRetest->setOn(true);				// Mark a cross on dies retested.
#if 1 // BG: WaferMap orientation options
	ptWafermap_PositiveX_Detect->setOn(true);	// Probing direction: auto detect
	ptWafermap_PositiveY_Detect->setOn(true);	// Probing direction: auto detect
#endif
	ptWafermap_ExportAlarm_FlatDiffers->setOn(false);
	ptWafermap_ExportAlarm_NoFlat->setOn(false);
	ptWafermap_MirrorX->setOn(false);
	ptWafermap_MirrorY->setOn(false);
	ptWafermap_FullWafer->setOn(false);
	ptWafermap_AlwaysRound->setOn(true);
	ptWafermap_RetestPolicy_Last->setOn(true);		// Last bin: Use Last bin value found for each die (default)

	// (Advanced) Histogram
	ptHistogram_Adaptive->setOn(true);			// Chart size= auto
	ptHistogram_TypeBars->setOn(true);			// Chart type= bars
	ptHistogram_Yaxis_Percentage->setOn(true);	// Default histogram Y-axis: Percentage
	ptHistogram_MarkerTestName->setOn(true);	// Marker: test name
	ptHistogram_MarkerMean->setOn(true);		// Marker: Mean
	ptHistogram_MarkerMedian->setOn(false);		// Marker: median
	ptHistogram_MarkerLimits->setOn(true);		// Marker: limits
	ptHistogram_MarkerSpecLimits->setOn(false);	// Marker: Spec limits
	ptHistogram_Cpk->setOn(true);				// Stats field: Cpk
	ptHistogram_Cp->setOn(false);				// Stats field: Cp
	ptHistogram_Max->setOn(false);				// Stats field: Max
	ptHistogram_Min->setOn(false);				// Stats field: Min
	ptHistogram_Outlier->setOn(false);			// Stats field: total outliers
	ptHistogram_Fails->setOn(false);			// Stats field: total fails
	ptHistogram_Sigma->setOn(true);				// Stats field: Sigma
	ptHistogram_Quartile2->setOn(false);		// Stats field: Median
	ptHistogram_Mean->setOn(true);				// Stats field: Mean
	ptHistogram_ExecCount->setOn(true);			// Stats field: Exec. count
	ptHistogram_Limits->setOn(true);			// Stats field: test limits

	// Trend
	ptTrend_Adaptive->setOn(true);				// Chart size= auto
	ptTrend_TypeLines->setOn(true);				// Chart type= Lines
	ptTrend_XAxisRunID->setOn(true);			// X axis = run ID
	ptTrend_MarkerTestName->setOn(true);		// Marker: test name
	ptTrend_MarkerMean->setOn(true);			// Marker: mean
	ptTrend_MarkerMedian->setOn(false);			// Marker: median
	ptTrend_MarkerLimits->setOn(true);			// Marker: limits
	ptTrend_MarkerSpecLimits->setOn(false);		// Marker: Spec limits

	// Correlation XY-Scatter
	ptScatter_Adaptive->setOn(true);			// Chart size= auto
	ptScatter_MarkerTestName->setOn(true);		// Marker: tests names
	ptScatter_MarkerMean->setOn(true);			// Marker: mean
	ptScatter_MarkerMedian->setOn(false);		// Marker: median
	ptScatter_MarkerLimits->setOn(true);		// Marker: limits
	ptScatter_MarkerSpecLimits->setOn(false);	// Marker: Spec limits
	
	// boxplot
	ptBoxPlot_Cpk->setOn(true);				// Show Cpk
	ptBoxplot_ChartAdaptive->setOn(true);	// Chart type: adaptive (limits if one dataset, range otherwise)
	ptBoxplot_SortTestNumber->setOn(true);	// Sort on TestNumber

	// datalog
	ptDatalog_DieLoc->setOn(true);			// Show XY die location

	// Auto Close of the application if Idle
	ptPreferences_AutoClose2hours->setOn(true);	// Default timeout: 2 hours.

	// Slots connections
    connect((QObject *)buttonProfileLoad,SIGNAL(clicked()),this,SLOT(OnLoadUserOptions(void)));
    connect((QObject *)buttonProfileSave,SIGNAL(clicked()),this,SLOT(OnSaveUserOptions(void)));
    connect((QObject *)buttonDefaultOptions,SIGNAL(clicked()),this,SLOT(OnDefaultOptions(void)));
}

///////////////////////////////////////////////////////////
// GexOptions: Empty function to ignore the ESCAPE key hit!
///////////////////////////////////////////////////////////
void GexOptions::reject(void)
{
}

///////////////////////////////////////////////////////////
// Updates the Tree selection for '<Field>: <value>'
///////////////////////////////////////////////////////////
void GexOptions::UpdateMenuCustomTextItem(Q3CheckListItem *ptItem,QString strField, const char *szString)
{
	QString strString = strField;
	if((szString == NULL) || (*szString == 0))
		strString += "default";
	else
		strString += szString;
	ptItem->setText(0,strString);
}

///////////////////////////////////////////////////////////
// Synchronises Output format bwteen 'Settings' and 'Options' pages
///////////////////////////////////////////////////////////
void GexOptions::RefreshOutputFormat(int iSettingsOutputFormat)
{
	// If Function called from the 'Settings' page (iSettingsOutputFormat >= 0), we must have the 'Options' page align itself on the 'Settings' selection
	switch(iSettingsOutputFormat)
	{
	case GEX_SETTINGS_OUTPUT_HTML:
		if(ptOutput_Html->isOn() == false)
			ptOutput_Html->setOn(true);				// Short HTML pages report.
		break;
	case GEX_SETTINGS_OUTPUT_WORD:
		if(ptOutput_Word->isOn() == false)
			ptOutput_Word->setOn(true);				// WORD report.
		break;
	case GEX_SETTINGS_OUTPUT_PPT:
		if(ptOutput_Ppt->isOn() == false)
			ptOutput_Ppt->setOn(true);				// PowerPoint slides report.
		break;
	case GEX_SETTINGS_OUTPUT_CSV:
		if(ptOutput_CSV->isOn() == false)
			ptOutput_CSV->setOn(true);				// Excel report.
		break;
	case GEX_SETTINGS_OUTPUT_PDF:
		if(ptOutput_Pdf->isOn() == false)
			ptOutput_Pdf->setOn(true);				// PDF report.
		break;
	case GEX_SETTINGS_OUTPUT_INTERACTIVE:
		if(ptOutput_Interactive->isOn() == false)
			ptOutput_Interactive->setOn(true);				// Interactive mode, no report.
		break;

	default:
		// Caller is 'Options'...then make 'Settings' page reflect selection in the 'Options' page.
		int iOptionsOutputFormat=GEX_SETTINGS_OUTPUT_HTML;
		if(ptOutput_Html->isOn())
			iOptionsOutputFormat= GEX_SETTINGS_OUTPUT_HTML;
		else
			if(ptOutput_Word->isOn())
				iOptionsOutputFormat= GEX_SETTINGS_OUTPUT_WORD;
		else
			if(ptOutput_CSV->isOn())
				iOptionsOutputFormat= GEX_SETTINGS_OUTPUT_CSV;
		else
			if(ptOutput_Ppt->isOn())
				iOptionsOutputFormat= GEX_SETTINGS_OUTPUT_PPT;
		else
			if(ptOutput_Pdf->isOn())
				iOptionsOutputFormat= GEX_SETTINGS_OUTPUT_PDF;
		else
			if(ptOutput_Interactive->isOn())
				iOptionsOutputFormat= GEX_SETTINGS_OUTPUT_INTERACTIVE;

		// Call 'Settings' page to show same Output format as just selected in the 'Options' page.
		pGexMainWindow->pWizardSettings->RefreshOutputFormat(iOptionsOutputFormat);
		break;
	}
}

///////////////////////////////////////////////////////////
// Called each time a selection is made in the list!
///////////////////////////////////////////////////////////
void GexOptions::OnSelectionChanged(void)
{
	// Make sure the Output format is in line between the 'Option' page and the 'Settings' page
	RefreshOutputFormat();

	QString strDatabasePath;
	Q3ListViewItem *ptItem = ListView->currentItem();
	if (!ptItem)
	{
		GSLOG(SYSLOG_SEV_ERROR, "ListView->currentItem() NULL !");
		return;
	}

	GSLOG(SYSLOG_SEV_DEBUG, QString("%1").arg( ptItem->text(0)).toLatin1().constData());

	QString strString;

	// Small Edit dialog box to prompt/edit the data.
	GetStringDialog cGetString(0, true, Qt::WStyle_DialogBorder);
	char szString[1024];

	if(ptItem == ptPreferences_AutoClose15minutes)
	{
		ReportOptions.SetOption("preferences","auto_close", "15min"); return;
	}
	if(ptItem == ptPreferences_AutoClose30minutes)
	{
		ReportOptions.SetOption("preferences","auto_close", "30min"); return;
	}
	if(ptItem ==  ptPreferences_AutoClose1hour)
	{
		ReportOptions.SetOption("preferences","auto_close", "1hour"); return;
	}
	if(ptItem == ptPreferences_AutoClose2hours)
	{
		ReportOptions.SetOption("preferences","auto_close", "2hours"); return;
	}
	if(ptItem == ptPreferences_AutoClose4hours)
	{
		ReportOptions.SetOption("preferences","auto_close", "4hours"); return;
	}
	if (ptItem == ptPreferences_AutoCloseNever)
	{
		ReportOptions.SetOption("preferences","auto_close", "never"); return;
	}

	// Edit report front page template?
	if(ptItem == ptCustomizeReportHomePage)
	{
		FrontPageDialog cFrontPage(pGexMainWindow,true, Qt::WStyle_DialogBorder);

		// Tells current Text + Image to display
		cFrontPage.SetSelection( ReportOptions.GetOption("output", "front_page_text").toString(),	//ReportOptions.strFrontPageText,
								 ReportOptions.GetOption("output", "front_page_image").toString());	//cFrontPage.SetSelection(ReportOptions.strFrontPageText,ReportOptions.strFrontPageImage);

		if(cFrontPage.exec() == 0)
			return;	// User canceled task.

		// Retrieve Text + Path to image to display on report's home page (front page)
		QString fpi=ReportOptions.GetOption("output", "front_page_image").toString();
		QString fpt=ReportOptions.GetOption("output", "front_page_text").toString();
		cFrontPage.GetSelection( fpt, fpi );
		//cFrontPage.GetSelection(ReportOptions.strFrontPageText, ReportOptions.strFrontPageImage);
		ReportOptions.SetOption("output", "front_page_text", fpt);
		ReportOptions.SetOption("output", "front_page_image", fpi);
		return;
	}


	// binning options
	{
		// 'binning' 'section'
		if(ptItem == ptBinning_InReport)
		{
			if(ptBinning_InReport->isOn())
				ReportOptions.SetOption("binning","section","enabled");
			return;
		}
		if(ptItem == ptBinning_NotInReport)
		{
			if(ptBinning_NotInReport->isOn())
				ReportOptions.SetOption("binning","section","disabled");
			return;
		}

		// 'binning' 'computation'
		if (ptItem == ptBinning_UseWafermapOnly)
		{
			if(ptBinning_UseWafermapOnly->isOn())
				ReportOptions.SetOption("binning","computation","wafer_map");
			return;
		}
		if (ptItem == ptBinning_UseSamplesOnly)
		{
			if(ptBinning_UseSamplesOnly->isOn())
				ReportOptions.SetOption("binning","computation","samples");
			return;
		}
		if (ptItem == ptBinning_UseSummary)
		{
			if(ptBinning_UseSummary->isOn())
				ReportOptions.SetOption("binning","computation","summary");
			return;
		}
	}

	if(ptItem == ptProbPlot_Small)
	{	ReportOptions.SetOption("adv_probabilityplot","chart_size","small"); return; }
	else if (ptItem==ptProbPlot_Large)
	{	ReportOptions.SetOption("adv_probabilityplot","chart_size","large"); return; }
	else if (ptItem==ptProbPlot_Adaptive)
	{	ReportOptions.SetOption("adv_probabilityplot","chart_size","auto"); return; }
	else if (ptItem==ptProbPlot_Medium)
	{	ReportOptions.SetOption("adv_probabilityplot","chart_size","medium"); return; }

	if(ptItem==ptProbPlot_Yaxis_Sigma)
	{	ReportOptions.SetOption("adv_probabilityplot","y_axis", "sigma"); return; }
	else if (ptItem==ptProbPlot_Yaxis_Percent)
	{	ReportOptions.SetOption("adv_probabilityplot","y_axis", "percentage"); return; }

	// Production Yield/Volume
	{
		if(ptItem == ptProdYield_YieldVolumeChart)
		{			
			if(ptProdYield_YieldVolumeChart->isOn())
				ReportOptions.SetOption("adv_production_yield","chart_type","yield_volume");

			return;
		}

		if(ptItem == ptProdYield_YieldChart)
		{
			if(ptProdYield_YieldChart->isOn())
				ReportOptions.SetOption("adv_production_yield","chart_type","yield");

			return;
		}

		if(ptItem == ptProdYield_VolumeChart)
		{
			if(ptProdYield_VolumeChart->isOn())
				ReportOptions.SetOption("adv_production_yield","chart_type","volume");

			return;
		}
		/*
		if(ReportOptions.bProdYieldChart && ReportOptions.bProdVolumeChart)
			ptProdYield_YieldVolumeChart->setOn(true);	// Yield/Volume production chart
		else
		if(ReportOptions.bProdYieldChart)
			ptProdYield_YieldChart->setOn(true);		// 'Yield only' production chart
		else
			ptProdYield_VolumeChart->setOn(false);		// 'Volume only' production chart
			*/
	}

	// Speed optimization
	{
		// Speed optimization: compute Advanced Statistics
		if(ptItem == ptSpeedAdvStats_Always)	// Always.
		{
			if(ptSpeedAdvStats_Always->isOn())
			{
				ReportOptions.SetOption("speed","adv_stats","always");
				return;
			}
		}
		if(ptItem == ptSpeedAdvStats_50MB)	// 50mb.
		{
			if(ptSpeedAdvStats_50MB->isOn())
			{
				ReportOptions.SetOption("speed","adv_stats","50mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvStats_100MB)	// 100mb.
		{
			if(ptSpeedAdvStats_100MB->isOn())
			{
				ReportOptions.SetOption("speed","adv_stats","100mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvStats_200MB)	// 50mb.
		{
			if(ptSpeedAdvStats_200MB->isOn())
			{
				ReportOptions.SetOption("speed","adv_stats","200mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvStats_300MB)	// 300mb.
		{
			if(ptSpeedAdvStats_300MB->isOn())
			{
				ReportOptions.SetOption("speed","adv_stats","300mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvStats_400MB)	// 400mb.
		{
			if(ptSpeedAdvStats_400MB->isOn())
			{
				ReportOptions.SetOption("speed","adv_stats","400mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvStats_500MB)	// 500mb.
		{
			if(ptSpeedAdvStats_500MB->isOn())
			{
				ReportOptions.SetOption("speed","adv_stats","500mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvStats_Never)	// never.
		{
			if(ptSpeedAdvStats_Never->isOn())
			{
				ReportOptions.SetOption("speed","adv_stats","never");
				return;
			}
		}

		// Speed optimization: using Database SUMMARY records instead of data samples
		if(ptItem == ptSpeedAdvSummaryDB_Always)	// always.
		{
			if(ptSpeedAdvSummaryDB_Always->isOn())
			{
				ReportOptions.SetOption("speed","db_summary","always");
				return;
			}
		}
		if(ptItem == ptSpeedAdvSummaryDB_50MB)	// 50mb.
		{
			if(ptSpeedAdvSummaryDB_50MB->isOn())
			{
				ReportOptions.SetOption("speed","db_summary","50mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvSummaryDB_100MB)	// 100mb.
		{
			if(ptSpeedAdvSummaryDB_100MB->isOn())
			{
				ReportOptions.SetOption("speed","db_summary","100mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvSummaryDB_200MB)	// 200mb.
		{
			if(ptSpeedAdvSummaryDB_200MB->isOn())
			{
				ReportOptions.SetOption("speed","db_summary","200mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvSummaryDB_300MB)	// 300mb.
		{
			if(ptSpeedAdvSummaryDB_300MB->isOn())
			{
				ReportOptions.SetOption("speed","db_summary","300mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvSummaryDB_400MB)	// 400mb.
		{
			if(ptSpeedAdvSummaryDB_400MB->isOn())
			{
				ReportOptions.SetOption("speed","db_summary","400mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvSummaryDB_500MB)	// 500mb.
		{
			if(ptSpeedAdvSummaryDB_500MB->isOn())
			{
				ReportOptions.SetOption("speed","db_summary","500mb");
				return;
			}
		}
		if(ptItem == ptSpeedAdvSummaryDB_Never)	// never.
		{
			if(ptSpeedAdvSummaryDB_Never->isOn())
			{
				ReportOptions.SetOption("speed","db_summary","never");
				return;
			}
		}
	}

	// Histogram chart type
	{
		if (ptItem == ptHistogram_Banner)
		{ ReportOptions.SetOption("adv_histogram","chart_size", "banner"); return; }
		else if (ptItem==ptHistogram_Adaptive)
		{ ReportOptions.SetOption("adv_histogram","chart_size", "auto"); return; }
		else if (ptItem==ptHistogram_Small)
		{ ReportOptions.SetOption("adv_histogram","chart_size", "small"); return; }
		else if (ptItem==ptHistogram_Large)
		{ ReportOptions.SetOption("adv_histogram","chart_size", "large"); return; }
		else if (ptItem==ptHistogram_Medium)
		{ ReportOptions.SetOption("adv_histogram","chart_size", "medium"); return; }


		if (ptItem==ptHistogram_Yaxis_Hits)
			{ ReportOptions.SetOption("adv_histogram","y_axis", "hits"); return; }
		if (ptItem==ptHistogram_Yaxis_Percentage)
			{ ReportOptions.SetOption("adv_histogram","y_axis", "percentage"); return; }

		if(ptItem == ptHistogram_TypeBars)
		{
			if(ptHistogram_TypeBars->isOn())		// Chart type= Bars
				ReportOptions.SetOption("adv_histogram","chart_type","bars");
			return;
		}
		if(ptItem == ptHistogram_Type3D_Bars)
		{
			if(ptHistogram_Type3D_Bars->isOn())		// Chart type= 3DBars
				ReportOptions.SetOption("adv_histogram","chart_type","3d_bars");
			return;
		}
		if(ptItem == ptHistogram_TypeBarsOutline)
		{
			if(ptHistogram_TypeBarsOutline->isOn())		// Chart type= Bars Outline
				ReportOptions.SetOption("adv_histogram","chart_type","bars_outline");
			return;
		}
		if(ptItem == ptHistogram_TypeSpline)
		{
			if(ptHistogram_TypeSpline->isOn())		// Chart type= Fitting curve
				ReportOptions.SetOption("adv_histogram","chart_type","curve");
			return;
		}
		if(ptItem == ptHistogram_TypeBarsSpline)
		{
			if(ptHistogram_TypeBarsSpline->isOn())		// Chart type= Bars+Fitting curve
				ReportOptions.SetOption("adv_histogram","chart_type","bars_curve");
			return;
		}
		if(ptItem == ptHistogram_TypeGaussian)
		{
			if(ptHistogram_TypeGaussian->isOn())		// Chart type= Gaussian
				ReportOptions.SetOption("adv_histogram","chart_type","gaussian");
			return;
		}
		if(ptItem == ptHistogram_TypeBarsGaussian)
		{
			if(ptHistogram_TypeBarsGaussian->isOn())		// Chart type= Gaussian+Bars
				ReportOptions.SetOption("adv_histogram","chart_type","bars_gaussian");
			return;
		}
		if(ptItem == ptHistogram_TypeBarsOutlineGaussian)
		{
			if(ptHistogram_TypeBarsOutlineGaussian->isOn())		// Chart type= Gaussian+Bars outline
				ReportOptions.SetOption("adv_histogram","chart_type","bars_outline_gaussian");
			return;
		}
	}



	// Production reports (yield/volume
	{
		// CRO_cleanUp
		if(ptItem == ptProdYield_MarkerTitle)
		{
			QString qstrVolumeMarkerOptions = (ReportOptions.GetOption("adv_production_yield", "marker")).toString();
			QStringList qstrlVolumeMarkerOptionList = qstrVolumeMarkerOptions.split(QString("|"));

			if(qstrlVolumeMarkerOptionList.contains("title"))
			{
				if(!ptProdYield_MarkerTitle->isOn())	// extract title from the option flag list, else nothing to do
				{
					QString qstrNewOptionValue = QString("");
					for(int ii=0; ii<qstrlVolumeMarkerOptionList.count(); ii++)
					{
						QString qstrStringListElement = qstrlVolumeMarkerOptionList.at(ii);
						if(qstrStringListElement != QString("title"))
							qstrNewOptionValue += qstrStringListElement + QString("|");
					}

					if(qstrNewOptionValue.endsWith("|"))
					{
						int nStrSize = qstrNewOptionValue.size();
						qstrNewOptionValue.remove(nStrSize-1, 1);
					}

					ReportOptions.SetOption(QString("adv_production_yield"), QString("marker"), qstrNewOptionValue);
					return;

				}
			}
			else
			{
				if(ptProdYield_MarkerTitle->isOn())
				{
					QString qstrNewOptionValue = QString("");

					if(qstrVolumeMarkerOptions.isEmpty())
						qstrNewOptionValue = QString("title");
					else
						qstrNewOptionValue += qstrVolumeMarkerOptions + QString("|title");

					ReportOptions.SetOption(QString("adv_production_yield"), QString("marker"), qstrNewOptionValue);
					return;
				}
			}

			return;
		}

		if(ptItem == ptProdYield_MarkerYield)
		{
			QString qstrVolumeMarkerOptions = (ReportOptions.GetOption("adv_production_yield", "marker")).toString();
			QStringList qstrlVolumeMarkerOptionList = qstrVolumeMarkerOptions.split(QString("|"));

			if(qstrlVolumeMarkerOptionList.contains("yield"))
			{
				if(!ptProdYield_MarkerYield->isOn())	// extract yield from the option flag list, else nothing to do
				{
					QString qstrNewOptionValue = QString("");
					for(int ii=0; ii<qstrlVolumeMarkerOptionList.count(); ii++)
					{
						QString qstrStringListElement = qstrlVolumeMarkerOptionList.at(ii);
						if(qstrStringListElement != QString("yield"))
							qstrNewOptionValue += qstrStringListElement + QString("|");
					}

					if(qstrNewOptionValue.endsWith("|"))
					{
						int nStrSize = qstrNewOptionValue.size();
						qstrNewOptionValue.remove(nStrSize-1, 1);
					}

					ReportOptions.SetOption(QString("adv_production_yield"), QString("marker"), qstrNewOptionValue);
					return;

				}
			}
			else
			{
				if(ptProdYield_MarkerYield->isOn())
				{
					QString qstrNewOptionValue = QString("");

					if(qstrVolumeMarkerOptions.isEmpty())
						qstrNewOptionValue = QString("yield");
					else
						qstrNewOptionValue += qstrVolumeMarkerOptions + QString("|yield");

					ReportOptions.SetOption(QString("adv_production_yield"), QString("marker"), qstrNewOptionValue);
					return;

				}
			}

			return;
		}

		if(ptItem == ptProdYield_MarkerVolume)
		{
			QString qstrVolumeMarkerOptions = (ReportOptions.GetOption("adv_production_yield", "marker")).toString();
			QStringList qstrlVolumeMarkerOptionList = qstrVolumeMarkerOptions.split(QString("|"));

			if(qstrlVolumeMarkerOptionList.contains("volume"))
			{
				if(!ptProdYield_MarkerVolume->isOn())	// extract volume from the option flag list, else nothing to do
				{
					QString qstrNewOptionValue = QString("");
					for(int ii=0; ii<qstrlVolumeMarkerOptionList.count(); ii++)
					{
						QString qstrStringListElement = qstrlVolumeMarkerOptionList.at(ii);
						if(qstrStringListElement != QString("volume"))
							qstrNewOptionValue += qstrStringListElement + QString("|");
					}

					if(qstrNewOptionValue.endsWith("|"))
					{
						int nStrSize = qstrNewOptionValue.size();
						qstrNewOptionValue.remove(nStrSize-1, 1);
					}

					ReportOptions.SetOption(QString("adv_production_yield"), QString("marker"), qstrNewOptionValue);
					return;

				}
			}
			else
			{
				if(ptProdYield_MarkerVolume->isOn())
				{
					QString qstrNewOptionValue = QString("");

					if(qstrVolumeMarkerOptions.isEmpty())
						qstrNewOptionValue = QString("volume");
					else
						qstrNewOptionValue += qstrVolumeMarkerOptions + QString("|volume");

					ReportOptions.SetOption(QString("adv_production_yield"), QString("marker"), qstrNewOptionValue);
					return;

				}
			}

			return;
		}
	}



	// Set local database path
	/*
	if(ptItem == ptDatabase_LocalPath)
	{
		// Edit the databases local path...
		strString = ptDatabase_LocalPath->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Specify the path to your Local databases",
								 "Path:",
								 szString,
								 "Enter the path to your local databases",
								 true);

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new path
		cGetString.getString(szString);

		// Format path so it always ends with a "/"
		strDatabasePath = szString;
		strDatabasePath = strDatabasePath.trimmed();
		if(strDatabasePath.isEmpty() == false)
		{
			// Append '/' if needed
			if((strDatabasePath.endsWith("(default)",false) == false) && (strDatabasePath.endsWith(":") == false) && (strDatabasePath.endsWith("/") == false) && (strString.endsWith("\\")== false))
				strDatabasePath += "/";
		}

		// Save it to our Options structure.
		ReportOptions.SetOption("databases","local_path", strDatabasePath); //ReportOptions.strDatabasesLocalPath = strDatabasePath;

		// Update GUI object
		strString = "Local databases path: ";
		strString += strDatabasePath;
		ptDatabase_LocalPath->setText(0,strString);

		// Make it the new selected radio button.
		ptDatabase_LocalPath->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptDatabase_LocalPath->parent());
		return;
	}
	*/

	// Set Server database path
	/*
	if(ptItem == ptDatabase_ServerPath)
	{
		// Edit the databases server path...
		strString = ptDatabase_ServerPath->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Specify the path to your Server databases",
								 "Path:",
								 szString,
								 "Enter the path to your server databases",
								 true);

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new path
		cGetString.getString(szString);

		// Format path so it always ends with a "/"
		strDatabasePath = szString;
		strDatabasePath = strDatabasePath.trimmed();
		if(strDatabasePath.isEmpty() == false)
		{
			// Append '/' if needed
			if((strDatabasePath.endsWith("(default)",false) == false) && (strDatabasePath.endsWith(":") == false) && (strDatabasePath.endsWith("/") == false) && (strDatabasePath.endsWith("\\")== false))
				strDatabasePath += "/";
		}

		// Save it to our Options structure.
		ReportOptions.SetOption("databases","server_path", strDatabasePath);  //ReportOptions.strDatabasesServerPath = strDatabasePath;

		// Update GUI object
		strString = "Server databases path: ";
		strString += strDatabasePath;
		ptDatabase_ServerPath->setText(0,strString);

		// Make it the new selected radio button.
		ptDatabase_ServerPath->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptDatabase_ServerPath->parent());
		return;
	}
	*/

	/*
	if (ptItem == ptOutput_PathFile)
	{
		ReportOptions.SetOption("output","location","");
		return;
	}
	if (ptItem == ptOutputLocation)
	{
		ReportOptions.SetOption("output","location","(default)");
		return;
	}
	// Define a custom report folder path.
	if(ptItem == ptCustomizeReportOutputPath)
	{
		// User wants to specify a specific path...
		// Extract current custom path to propose it
		strString = ptOutput_PathCustom->text(0);
		strString = strString.section(' ',1);	// Extract text to the right of 'Location: '
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Specify the path where report must be created",
								 "Path:",
								 szString,
								 "Enter the path were your report will be created",
								 true);

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new path, and update list view content.
		cGetString.getString(szString);

		// Update Report Custom location
		UpdateMenuCustomTextItem(ptOutput_PathCustom,"Location: ",szString);

		// Highligh the line with the new Path
		ListView->setCurrentItem(ptOutput_PathCustom);

		// Make it the new selected radio button.
		ptOutput_PathCustom->setOn(true);
		ReportOptions.SetOption("output","location", szString);

		return;
	}
	*/

	if (ptItem==ptEmbedFontsInPdf)
	{	ptEmbedFontsInPdf->setOn(true); ptNoEmbedFontsInPdf->setOn(false);
		ReportOptions.SetOption("output","embed_fonts_in_pdf","true");
		return;
	}
	if (ptItem==ptNoEmbedFontsInPdf)
	{
		ptNoEmbedFontsInPdf->setOn(true); ptEmbedFontsInPdf->setOn(false);
		ReportOptions.SetOption("output","embed_fonts_in_pdf","false");
		return;
	}

	if (ptItem==ptDatabase_RDB_DefaultParameters_All)
	{
		ReportOptions.SetOption("databases","rdb_default_parameters", "all");
		return;
	}
	if (ptItem==ptDatabase_RDB_DefaultParameters_None)
	{
		ReportOptions.SetOption("databases","rdb_default_parameters", "none");
		return;
	}

	if (ptItem==ptOutputPaperSize_A4)
	{
		ReportOptions.SetOption("output", "paper_size","A4");
	}
	if (ptItem==ptOutputPaperSize_Letter)
	{
		ReportOptions.SetOption("output", "paper_size","letter");
	}

	if (ptItem==ptOutputPaperFormat_Landscape)
	{
		ptOutputPaperFormat_Landscape->setOn(true); ptOutputPaperFormat_Portrait->setOn(false);
		ReportOptions.SetOption("output", "paper_format","landscape");
	}
	if (ptItem==ptOutputPaperFormat_Portrait)
	{
		ptOutputPaperFormat_Portrait->setOn(true);  ptOutputPaperFormat_Landscape->setOn(false);
		ReportOptions.SetOption("output","paper_format","portrait");
	}

	// Define a custom test name truncation value...
	/*
	if(ptItem == ptOutputTruncateTestNames)
	{
		// User wants to specify a specific path...
		// Extract current custom path to propose it
		strString = ptOutputTruncateTestNames->text(0);
		strString = strString.section(':',1);	// Extract text to the right of 'Truncate test names to length: '
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Specify maximum test name length",
								 "Length:",
								 szString,
								 "Enter maximum length allowed",
								 false);

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new path, and update list view content.
		cGetString.getString(szString);

		// Update Report Custom location
		UpdateMenuCustomTextItem(ptOutputTruncateTestNames,"Truncate test names to length: ",szString);

		// Highligh the line with the new Path
		ListView->setCurrentItem(ptOutputTruncateTestNames);

		// Make it the new selected radio button.
		ptOutputTruncateTestNames->setOn(true);

		ReportOptions.SetOption("output", "truncate_names", szString);
		return;
	}
	*/
	

	// Define a custom report folder path.
	if(ptItem == ptTempStdfFolder_RequestCustom)
	{
		// User wants to specify a specific path for Temporary STDF files...
		// Extract current custom path to propose it
		strString = ptTempStdfFolder_Custom->text(0);
		strString = strString.section(' ',1);	// Extract text to the right of 'Location: '
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Folder to use for Intermediate files",
								 "Path:",
								 szString,
								 "Enter the path were intermediate files are created",
								 true);

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new path, and update list view content.
		cGetString.getString(szString);

		// Update Report Custom location
		UpdateMenuCustomTextItem(ptTempStdfFolder_Custom,"Location: ",szString);

		// Highligh the line with the new Path
		ListView->setCurrentItem(ptTempStdfFolder_Custom);

		// Make it the new selected radio button.
		ptTempStdfFolder_Custom->setOn(true);
		return;
	}


	if (ptItem==ptStatistics_CpCpkDefaultFormula)
	{
		ReportOptions.SetOption("statistics", "cp_cpk_computation", "standard");
		return;
	}
	if (ptItem==ptStatistics_CpCpkPercentileFormula)
	{
		ReportOptions.SetOption("statistics", "cp_cpk_computation", "percentile");
		return;
	}


	// STATISTICS
	{
		// 'statistics' 'sorting'
		if(ptItem == ptStatistics_SortTestNumber)
		{
			if( ptStatistics_SortTestNumber->isOn())
				ReportOptions.SetOption("statistics","sorting","test_number");
			return;
		}
		if(ptItem == ptStatistics_SortTestName)
		{
			if( ptStatistics_SortTestName->isOn())
				ReportOptions.SetOption("statistics","sorting","test_name");
			return;
		}
		if(ptItem == ptStatistics_SortTestFlowID)
		{
			if( ptStatistics_SortTestFlowID->isOn())
				ReportOptions.SetOption("statistics","sorting","test_flow_id");
			return;
		}
		if(ptItem == ptStatistics_SortMean)
		{
			if(ptStatistics_SortMean->isOn())
				ReportOptions.SetOption("statistics","sorting","mean");
			return;
		}
		if(ptItem == ptStatistics_SortMeanShift)
		{
			if( ptStatistics_SortMeanShift->isOn())
				ReportOptions.SetOption("statistics","sorting","mean_shift");
			return;
		}
		if(ptItem == ptStatistics_SortSigma)
		{
			if( ptStatistics_SortSigma->isOn())
				ReportOptions.SetOption("statistics","sorting","sigma");
			return;
		}
		if(ptItem == ptStatistics_SortCp)
		{
			if( ptStatistics_SortCp->isOn())
				ReportOptions.SetOption("statistics","sorting","cp");
			return;
		}
		if(ptItem == ptStatistics_SortCpk)
		{
			if( ptStatistics_SortCpk->isOn())
				ReportOptions.SetOption("statistics","sorting","cpk");
			return;
		}
		if(ptItem == ptStatistics_SortR_and_R)
		{
			if( ptStatistics_SortR_and_R->isOn())
				ReportOptions.SetOption("statistics","sorting","r&r");
			return;
		}


		// 'statistics' 'computation' option:
		if(ptItem == ptStatistics_UseSamplesOnly)
		{
			if(ptStatistics_UseSamplesOnly->isOn())
				ReportOptions.SetOption("statistics","computation","samples_only");
			return;
		}
		if(ptItem == ptStatistics_UseSummaryOnly)
		{
			if(ptStatistics_UseSummaryOnly->isOn())
				ReportOptions.SetOption("statistics","computation","summary_only");
			return;
		}
		if(ptItem == ptStatistics_UseSummaryThenSamples)
		{
			if(ptStatistics_UseSummaryThenSamples->isOn())
				ReportOptions.SetOption("statistics","computation","summary_then_samples");
			return;
		}
		if(ptItem == ptStatistics_UseSamplesThenSummary)
		{
			if(ptStatistics_UseSamplesThenSummary->isOn())
				ReportOptions.SetOption("statistics","computation","samples_then_summary");
			return;
		}


		// 'statistics', 'generic_galaxy_tests', {'show'*,hide'}
		if(ptItem == ptStatistics_IncludeGenericTests)
		{
			if(ptStatistics_IncludeGenericTests->isOn())
				ReportOptions.SetOption("statistics","generic_galaxy_tests","show");
			return;
		}
		if(ptItem == ptStatistics_ExcludeGenericTests)
		{
			if(ptStatistics_ExcludeGenericTests->isOn())
				ReportOptions.SetOption("statistics","generic_galaxy_tests","hide");
			return;
		}

	}

	// Customize the text editor to use
	if(ptItem == ptCustomizeTextEditor)
	{
		// User wants to specify a custom TEXT EDITOR..
		// Extract current custom path to propose it
		strString = ptPreferences_CustomTextEditor->text(0);
		strString = strString.mid(16);
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Specify the new editor",
								 "Editor:",
								 szString,
								 "Enter the text editor to use",
								 true,false);

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new path, and update list view content.
		cGetString.getString(szString);

		// Update Report Custom location
		UpdateMenuCustomTextItem(ptPreferences_CustomTextEditor,"Current editor: ",szString);
		if(!qstricmp(szString,"default"))
		{
#if (defined __sun__ || __hpux__)
			ReportOptions.SetOption("preferences","text_editor","textedit"); //ReportOptions.strTextEditor = "textedit";	// Editor under Solaris & HP-UX
#elif (defined __linux__)
			ReportOptions.SetOption("preferences","text_editor","gedit"); //ReportOptions.strTextEditor = "gedit";	// Editor under Linux
#else
			ReportOptions.SetOption("preferences","text_editor","wordpad"); //ReportOptions.strTextEditor = "wordpad";	// Editor under Windows
#endif
		}
		else
			ReportOptions.SetOption("preferences","text_editor",szString);	//ReportOptions.strTextEditor = szString;

		// Highligh the line with the new Path
		ListView->setCurrentItem(ptPreferences_CustomTextEditor);

		// Make it the new selected radio button.
		ptPreferences_CustomTextEditor->setOn(true);
		return;
	}

	if(ptItem == ptCustomizePdfEditor)
	{
		// User wants to specify a custom SpreadSheet EDITOR..
		// Extract current custom path to propose it
		strString = ptPreferences_CustomPdfEditor->text(0); //ptPreferences_CustomSpreadsheetEditor->text(0);
		strString = strString.mid(16);
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Specify the new editor", "Editor:", szString, "Enter the program to use", true,false);

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new path, and update list view content.
		cGetString.getString(szString);

		// Update Report Custom location
		UpdateMenuCustomTextItem(ptPreferences_CustomPdfEditor, "Current editor: ",szString);
		if(!qstricmp(szString,"default"))
		{
#if (defined __sun__ || __hpux__)
			ReportOptions.SetOption("preferences", "pdf_editor", "gpdf"); //ReportOptions.m_PrefMap["pdf_editor"]= "gpdf";	// Editor under Solaris & HP-UX
#elif (defined __linux__)
			ReportOptions.SetOption("preferences", "pdf_editor", "evince");	//ReportOptions.m_PrefMap["pdf_editor"]= "evince";	// Editor under Linux
#else
			ReportOptions.SetOption("preferences", "pdf_editor", "AcroRd32.exe");	//ReportOptions.m_PrefMap["pdf_editor"]="AcroRd32.exe";	 // Editor under Windows
#endif
		}
		else
			ReportOptions.SetOption("preferences", "pdf_editor", szString);	//ReportOptions.m_PrefMap["pdf_editor"] = szString;

		// Highligh the line with the new Path
		ListView->setCurrentItem(ptPreferences_CustomSpreadsheetEditor);

		// Make it the new selected radio button.
		ptPreferences_CustomSpreadsheetEditor->setOn(true);
		return;
	}


	// Customize the text editor to use
	if(ptItem == ptCustomizeSSheetEditor)
	{
		// User wants to specify a custom SpreadSheet EDITOR..
		// Extract current custom path to propose it
		strString = ptPreferences_CustomSpreadsheetEditor->text(0);
		strString = strString.mid(16);
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Specify the new editor",
								 "Editor:",
								 szString,
								 "Enter the Spreadsheet editor to use",
								 true,false);

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new path, and update list view content.
		cGetString.getString(szString);

		// Update Report Custom location
		UpdateMenuCustomTextItem(ptPreferences_CustomSpreadsheetEditor,"Current editor: ",szString);
		if(!qstricmp(szString,"default"))
		{
#if (defined __sun__ || __hpux__)
			ReportOptions.SetOption("preferences", "ssheet_editor", "textedit");  //ReportOptions.m_PrefMap["ssheet_editor"]="textedit";	// Editor under Solaris & HP-UX
#elif (defined __linux__)
			ReportOptions.SetOption("preferences", "ssheet_editor", "oocalc"); //ReportOptions.m_PrefMap["ssheet_editor"]= "oocalc";	// Editor under Linux
#else
			ReportOptions.SetOption("preferences", "ssheet_editor", "excel"); //ReportOptions.m_PrefMap["ssheet_editor"]="excel";	 // Editor under Windows
#endif
		}
		else
			ReportOptions.SetOption("preferences", "ssheet_editor", szString);	//ReportOptions.strTextEditor = szString;

		// Highligh the line with the new Path
		ListView->setCurrentItem(ptPreferences_CustomSpreadsheetEditor);

		// Make it the new selected radio button.
		ptPreferences_CustomSpreadsheetEditor->setOn(true);
		return;
	}

	// change CPK alarms
	{
		double lfOptionStorageDevice;
		QString strOptionStorageDevice;
		bool bOptionValidity;

		// Change the CPK alarm (Red) limit (only in gui)
		if(ptItem == ptCustomizeCpkRedAlarm)
		{
			// User wants to specify a specific path...
			// Extract current custom path to propose it
			strString = ptStatistics_RedAlarmCpk->text(0);
			int iIndex = strString.find(':')+2;
			strString = strString.mid(iIndex);
			if(strString.isEmpty())
				*szString = 0;
			else
				strcpy(szString,strString);

			cGetString.setDialogText("Specify the new Cpk alarm limit",
									 "Cpk alarm:",
									 szString,
									 "Enter the Cpk alarm limit");

			// Display Edit window
			if(cGetString.exec() == 0)
				return;	// User canceled task.

			// Get new path, and update list view content.
			cGetString.getString(szString);

			strOptionStorageDevice = QString(szString);
			lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
            GEX_ASSERT(bOptionValidity);

			if ( (!bOptionValidity) || (lfOptionStorageDevice<0) )
			{
				ptStatistics_RedAlarmCpk->setOn(false);
				ReportOptions.SetOption("statistics","alarm_test_cpk","-1");
			}
			else
			{
				ptStatistics_RedAlarmCpk->setOn(true);				// Make it the new selected radio button.
				ReportOptions.SetOption("statistics","alarm_test_cpk",strOptionStorageDevice);
			}


			strString = "Show 'Cpk' Alarm (Red) if under: ";
			strString += szString;
			ptStatistics_RedAlarmCpk->setText(0,strString);

			// Highligh the line with the Alarm value
			ListView->setCurrentItem(ptStatistics_RedAlarmCpk);
			return;
		}

		// Change the CPK Warning (Yellow) limit (only in gui)
		if(ptItem == ptCustomizeCpkYellowAlarm)
		{
			// User wants to specify a specific path...
			// Extract current custom path to propose it
			strString = ptStatistics_YellowAlarmCpk->text(0);
			int iIndex = strString.find(':')+2;
			strString = strString.mid(iIndex);
			if(strString.isEmpty())
				*szString = 0;
			else
				strcpy(szString,strString);

			cGetString.setDialogText("Specify the new Cpk warning limit",
									 "Cpk warning:",
									 szString,
									 "Enter the Cpk warning limit");

			// Display Edit window
			if(cGetString.exec() == 0)
				return;	// User canceled task.

			// Get new path, and update list view content.
			cGetString.getString(szString);

			strOptionStorageDevice = QString(szString);
			lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
            GEX_ASSERT(bOptionValidity);

			if ( (!bOptionValidity) || (lfOptionStorageDevice<0) )
			{
				ptStatistics_YellowAlarmCpk->setOn(false);
				ReportOptions.SetOption("statistics","warning_test_cpk","-1");
			}
			else
			{
				ptStatistics_YellowAlarmCpk->setOn(true);			// Make it the new selected radio button.
				ReportOptions.SetOption("statistics","warning_test_cpk",strOptionStorageDevice);
			}

			strString = "Show 'Cpk' Warning (Yellow) if under: ";
			strString += szString;
			ptStatistics_YellowAlarmCpk->setText(0,strString);

			// Highligh the line with the Alarm value
			ListView->setCurrentItem(ptStatistics_YellowAlarmCpk);
			return;
		}

		/////////////////////////////////////////////////
		// on / off buttons

		if(ptItem == ptStatistics_RedAlarmCpk)
		{
			if(ptStatistics_RedAlarmCpk->isOn())
			{
				// Extract current custom path to propose it
				strString = ptStatistics_RedAlarmCpk->text(0);
				int iIndex = strString.find(':')+2;
				strString = strString.mid(iIndex);
				if(strString.isEmpty())
					*szString = 0;
				else
					strcpy(szString,strString);

				bOptionValidity = false;
				strOptionStorageDevice = QString(szString);
				strOptionStorageDevice.remove("%");
				lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
                GEX_ASSERT(bOptionValidity);

				if(lfOptionStorageDevice<0)
				{
					ReportOptions.SetOption("statistics","alarm_test_cpk","1.33");
					ptStatistics_RedAlarmCpk->setText(0,"Show 'Cpk' Alarm (Red) if under: 1.33");
				}
				else
					ReportOptions.SetOption("statistics","alarm_test_cpk",strOptionStorageDevice);
			}
			else
			{
				ReportOptions.SetOption("statistics","alarm_test_cpk","-1");
			}
		}

		if(ptItem == ptStatistics_YellowAlarmCpk)
		{
			if(ptStatistics_YellowAlarmCpk->isOn())
			{
				// Extract current custom path to propose it
				strString = ptStatistics_YellowAlarmCpk->text(0);
				int iIndex = strString.find(':')+2;
				strString = strString.mid(iIndex);
				if(strString.isEmpty())
					*szString = 0;
				else
					strcpy(szString,strString);

				bOptionValidity = false;
				strOptionStorageDevice = QString(szString);
				strOptionStorageDevice.remove("%");
				lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
                GEX_ASSERT(bOptionValidity);

				if(lfOptionStorageDevice<0)
				{
					ReportOptions.SetOption("statistics","warning_test_cpk","1.67");
					ptStatistics_YellowAlarmCpk->setText(0,"Show 'Cpk' Warning (Yellow) if under: 1.67");
				}
				else
					ReportOptions.SetOption("statistics","warning_test_cpk",strOptionStorageDevice);
			}
			else
			{
				ReportOptions.SetOption("statistics","warning_test_cpk","-1");
			}
		}

	}

	// change yield alarms
	{
		double lfOptionStorageDevice;
		bool bOptionValidity;
		QString strOptionStorageDevice;


		// Change the Test Yield alarm (Red) limit
		if(ptItem == ptCustomizeTestYieldRedAlarm)
		{
			// User wants to specify a specific path...
			// Extract current custom path to propose it
			strString = ptStatistics_RedAlarmTestYield->text(0);
			int iIndex = strString.find(':')+2;
			strString = strString.mid(iIndex);
			if(strString.isEmpty())
				*szString = 0;
			else
				strcpy(szString,strString);

			cGetString.setDialogText("Specify the new Yield alarm limit",
									 "Cpk alarm:",
									 szString,
									 "Enter the Yield alarm limit");

			// Display Edit window
			if(cGetString.exec() == 0)
				return;	// User canceled task.

			// Get new path, and update list view content.
			cGetString.getString(szString);
			strString = "Show 'Test yield' Alarm (Red) if under: ";
			strString += szString;
			ptStatistics_RedAlarmTestYield->setText(0,strString);


			bOptionValidity = false;
			strOptionStorageDevice = QString(szString);
			strOptionStorageDevice.remove("%");
			lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
            GEX_ASSERT(bOptionValidity);

			if ( (!bOptionValidity) || (lfOptionStorageDevice<0) )
			{
				ptStatistics_RedAlarmTestYield->setOn(false);
				ReportOptions.SetOption("statistics","alarm_test_yield","-1");
			}
			else
			{
				ptStatistics_RedAlarmTestYield->setOn(true);
				ReportOptions.SetOption("statistics","alarm_test_yield",strOptionStorageDevice);
			}


			// Highligh the line with the Alarm value
			ListView->setCurrentItem(ptStatistics_RedAlarmTestYield);

			// Make it the new selected radio button.
			ptStatistics_RedAlarmTestYield->setOn(true);
			return;
		}

		// Change the Test Yield Warning (Yellow) limit
		if(ptItem == ptCustomizeTestYieldYellowAlarm)
		{
			// User wants to specify a specific path...
			// Extract current custom path to propose it
			strString = ptStatistics_YellowAlarmTestYield->text(0);
			int iIndex = strString.find(':')+2;
			strString = strString.mid(iIndex);
			if(strString.isEmpty())
				*szString = 0;
			else
				strcpy(szString,strString);

			cGetString.setDialogText("Specify the new Yield warning limit",
									 "Cpk warning:",
									 szString,
									 "Enter the Yield warning limit");

			// Display Edit window
			if(cGetString.exec() == 0)
				return;	// User canceled task.

			// Get new path, and update list view content.
			cGetString.getString(szString);
			strString = "Show 'Test yield' Warning (Yellow) if under: ";
			strString += szString;
			ptStatistics_YellowAlarmTestYield->setText(0,strString);


			bOptionValidity = false;
			strOptionStorageDevice = QString(szString);
			strOptionStorageDevice.remove("%");
			lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
            GEX_ASSERT(bOptionValidity);

			if ( (!bOptionValidity) || (lfOptionStorageDevice<0) )
			{
				ptStatistics_YellowAlarmTestYield->setOn(false);
				ReportOptions.SetOption("statistics","warning_test_yield","-1");
			}
			else
			{
				ptStatistics_YellowAlarmTestYield->setOn(true);
				ReportOptions.SetOption("statistics","warning_test_yield",strOptionStorageDevice);
			}


			// Highligh the line with the Alarm value
			ListView->setCurrentItem(ptStatistics_YellowAlarmTestYield);

			// Make it the new selected radio button.
			ptStatistics_YellowAlarmTestYield->setOn(true);
			return;
		}


		////////////////////////////////////
		// on / off buttons

		if(ptItem == ptStatistics_RedAlarmTestYield)
		{
			if(ptStatistics_RedAlarmTestYield->isOn())
			{
				// Extract current custom path to propose it
				strString = ptStatistics_RedAlarmTestYield->text(0);
				int iIndex = strString.find(':')+2;
				strString = strString.mid(iIndex);
				if(strString.isEmpty())
					*szString = 0;
				else
					strcpy(szString,strString);

				bOptionValidity = false;
				strOptionStorageDevice = QString(szString);
				strOptionStorageDevice.remove("%");
				lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
                GEX_ASSERT(bOptionValidity);

				if(lfOptionStorageDevice<0)
				{
					ReportOptions.SetOption("statistics","alarm_test_yield","90");
					ptStatistics_RedAlarmTestYield->setText(0,"Show 'Test yield' Alarm (Red) if under: 90%");
				}
				else
					ReportOptions.SetOption("statistics","alarm_test_yield",strOptionStorageDevice);
			}
			else
			{
				ReportOptions.SetOption("statistics","alarm_test_yield","-1");
			}

			// Highligh the line with the Alarm value
			ListView->setCurrentItem(ptStatistics_RedAlarmTestYield);
			return;
		}

		if(ptItem == ptStatistics_YellowAlarmTestYield)
		{
			if(ptStatistics_YellowAlarmTestYield->isOn())
			{
				// Extract current custom path to propose it
				strString = ptStatistics_YellowAlarmTestYield->text(0);
				int iIndex = strString.find(':')+2;
				strString = strString.mid(iIndex);
				if(strString.isEmpty())
					*szString = 0;
				else
					strcpy(szString,strString);

				bOptionValidity = false;
				strOptionStorageDevice = QString(szString);
				strOptionStorageDevice.remove("%");
				lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
                GEX_ASSERT(bOptionValidity);

				if(lfOptionStorageDevice<0)
				{
					ReportOptions.SetOption("statistics","warning_test_yield","80");
					ptStatistics_YellowAlarmTestYield->setText(0,"Show 'Test yield' Warning (Yellow) if under: 80%");
				}
				else
					ReportOptions.SetOption("statistics","warning_test_yield",strOptionStorageDevice);
			}
			else
			{
				ReportOptions.SetOption("statistics","warning_test_yield","-1");
			}

			// Highligh the line with the Alarm value
			ListView->setCurrentItem(ptStatistics_YellowAlarmTestYield);
			return;
		}

	}


	// drift alarm options
	{
		double lfOptionStorageDevice;
		bool bOptionValidity;
		QString strOptionStorageDevice;

		// Change the MEAN Drift alarm limit
		if(ptItem == ptCustomizeMeanDriftAlarm)
		{
			// User wants to specify a specific path...
			// Extract current custom path to propose it
			strString = ptStatistics_CorrCheckMean->text(0);
			int iIndex = strString.find(':')+2;
			strString = strString.mid(iIndex);
			if(strString.isEmpty())
				*szString = 0;
			else
				strcpy(szString,strString);

			cGetString.setDialogText("Specify the new Mean alarm limit",
									 "Mean alarm:",
									 szString,
									 "Enter the Mean alarm limit (in %)");

			// Display Edit window
			if(cGetString.exec() == 0)
				return;	// User canceled task.

			// Get new path, and update list view content.
			cGetString.getString(szString);

			bOptionValidity = false;
			strOptionStorageDevice = QString(szString);
			strOptionStorageDevice.remove("%");
			lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
            GEX_ASSERT(bOptionValidity);

			if ( (!bOptionValidity) || (lfOptionStorageDevice<0) )
			{
				ptStatistics_CorrCheckMean->setOn(false);
				ReportOptions.SetOption("statistics","alarm_mean","-1");
			}
			else
			{
				ptStatistics_CorrCheckMean->setOn(true);
				ReportOptions.SetOption("statistics","alarm_mean",strOptionStorageDevice);
			}

			strString = "Show Alarm if Mean drifts: ";
			strString += szString;
			ptStatistics_CorrCheckMean->setText(0,strString);

			// Highligh the line with the new Alarm value
			ListView->setCurrentItem(ptStatistics_CorrCheckMean);

			// Make it the new selected radio button.
			ptStatistics_CorrCheckMean->setOn(true);
			return;
		}

		// Change the SIGMA Drift alarm limit
		if(ptItem == ptCustomizeSigmaDriftAlarm)
		{
			// User wants to specify a specific path...
			// Extract current custom path to propose it
			strString = ptStatistics_CorrCheckSigma->text(0);
			int iIndex = strString.find(':')+2;
			strString = strString.mid(iIndex);
			if(strString.isEmpty())
				*szString = 0;
			else
				strcpy(szString,strString);

			cGetString.setDialogText("Specify the new Sigma alarm limit",
									 "Sigma alarm:",
									 szString,
									 "Enter the Sigma alarm limit (in %)");

			// Display Edit window
			if(cGetString.exec() == 0)
				return;	// User canceled task.

			// Get new path, and update list view content.
			cGetString.getString(szString);

			bOptionValidity = false;
			strOptionStorageDevice = QString(szString);
			strOptionStorageDevice.remove("%");
			lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
            GEX_ASSERT(bOptionValidity);

			if ( (!bOptionValidity) || (lfOptionStorageDevice<0) )
			{
				ptStatistics_CorrCheckSigma->setOn(false);
				ReportOptions.SetOption("statistics","alarm_sigma","-1");
			}
			else
			{
				ptStatistics_CorrCheckSigma->setOn(true);
				ReportOptions.SetOption("statistics","alarm_sigma",strOptionStorageDevice);
			}


			strString = "Show Alarm if Sigma drifts: ";
			strString += szString;
			ptStatistics_CorrCheckSigma->setText(0,strString);

			// Highligh the line with the new Alarm value
			ListView->setCurrentItem(ptStatistics_CorrCheckSigma);

			// Make it the new selected radio button.
			ptStatistics_CorrCheckSigma->setOn(true);
			return;
		}

		// Change the CPK Drift alarm limit
		if(ptItem == ptCustomizeCpkDriftAlarm)
		{
			// User wants to specify a specific path...
			// Extract current custom path to propose it
			strString = ptStatistics_CorrCheckCpk->text(0);
			int iIndex = strString.find(':')+2;
			strString = strString.mid(iIndex);
			if(strString.isEmpty())
				*szString = 0;
			else
				strcpy(szString,strString);

			cGetString.setDialogText("Specify the new Cpk alarm limit",
									 "Cpk alarm:",
									 szString,
									 "Enter the Cpk alarm limit (in %)");

			// Display Edit window
			if(cGetString.exec() == 0)
				return;	// User canceled task.

			// Get new path, and update list view content.
			cGetString.getString(szString);
			strString = "Show Alarm if Cpk drifts: ";
			strString += szString;
			ptStatistics_CorrCheckCpk->setText(0,strString);

			// Highligh the line with the Alarm value
			ListView->setCurrentItem(ptStatistics_CorrCheckCpk);


			bOptionValidity = false;
			strOptionStorageDevice = QString(szString);
			strOptionStorageDevice.remove("%");
			lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
            GEX_ASSERT(bOptionValidity);

			if ( (!bOptionValidity) || (lfOptionStorageDevice<0) )
			{
				ReportOptions.SetOption("statistics","alarm_cpk","-1");
				ptStatistics_CorrCheckCpk->setOn(false);
			}
			else
			{

				ReportOptions.SetOption("statistics","alarm_cpk",strOptionStorageDevice);
				ptStatistics_CorrCheckCpk->setOn(true);
			}

			/* // Make it the new selected radio button.
			ptStatistics_CorrCheckCpk->setOn(true); */
			return;
		}

		///////////////////////////////////////
		// on / off buttons

		if(ptItem == ptStatistics_CorrCheckMean)
		{
			if(ptStatistics_CorrCheckMean->isOn())
			{
				// Extract current custom path to propose it
				strString = ptStatistics_CorrCheckMean->text(0);
				int iIndex = strString.find(':')+2;
				strString = strString.mid(iIndex);
				if(strString.isEmpty())
					*szString = 0;
				else
					strcpy(szString,strString);

				bOptionValidity = false;
				strOptionStorageDevice = QString(szString);
				strOptionStorageDevice.remove("%");
				lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
                GEX_ASSERT(bOptionValidity);

				if(lfOptionStorageDevice<0)
				{
					ReportOptions.SetOption("statistics","alarm_mean","5");
					ptStatistics_CorrCheckMean->setText(0,"Show Alarm if Mean drifts: 5%");
				}
				else
					ReportOptions.SetOption("statistics","alarm_mean",strOptionStorageDevice);
			}
			else
			{
				ReportOptions.SetOption("statistics","alarm_mean","-1");
			}

			// Highligh the line with the Alarm value
			ListView->setCurrentItem(ptStatistics_CorrCheckMean);
			return;
		}

		if(ptItem == ptStatistics_CorrCheckSigma)
		{
			if(ptStatistics_CorrCheckSigma->isOn())
			{
				// Extract current custom path to propose it
				strString = ptStatistics_CorrCheckSigma->text(0);
				int iIndex = strString.find(':')+2;
				strString = strString.mid(iIndex);
				if(strString.isEmpty())
					*szString = 0;
				else
					strcpy(szString,strString);

				bOptionValidity = false;
				strOptionStorageDevice = QString(szString);
				strOptionStorageDevice.remove("%");
				lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
                GEX_ASSERT(bOptionValidity);

				if(lfOptionStorageDevice<0)
				{
					ReportOptions.SetOption("statistics","alarm_sigma","1");
					ptStatistics_CorrCheckSigma->setText(0,"Show Alarm if Sigma drifts: 1%");
				}
				else
					ReportOptions.SetOption("statistics","alarm_sigma",strOptionStorageDevice);
			}
			else
			{
				ReportOptions.SetOption("statistics","alarm_sigma","-1");
			}

			// Highligh the line with the Alarm value
			ListView->setCurrentItem(ptStatistics_CorrCheckSigma);
			return;
		}

		if(ptItem == ptStatistics_CorrCheckCpk)
		{
			if(ptStatistics_CorrCheckCpk->isOn())
			{
				// Extract current custom path to propose it
				strString = ptStatistics_CorrCheckCpk->text(0);
				int iIndex = strString.find(':')+2;
				strString = strString.mid(iIndex);
				if(strString.isEmpty())
					*szString = 0;
				else
					strcpy(szString,strString);

				bOptionValidity = false;
				strOptionStorageDevice = QString(szString);
				strOptionStorageDevice.remove("%");
				lfOptionStorageDevice = strOptionStorageDevice.toDouble(&bOptionValidity);
                GEX_ASSERT(bOptionValidity);

				if(lfOptionStorageDevice<0)
				{
					ReportOptions.SetOption("statistics","alarm_cpk","33");
					ptStatistics_CorrCheckCpk->setText(0,"Show Alarm if Cpk drifts: 33%");
				}
				else
					ReportOptions.SetOption("statistics","alarm_cpk",strOptionStorageDevice);
			}
			else
			{
				ReportOptions.SetOption("statistics","alarm_cpk","-1");
			}

			// Highligh the line with the Alarm value
			ListView->setCurrentItem(ptStatistics_CorrCheckCpk);

			return;
		}

	}

	//////////////////////////////////
	// mean drift formula on / off
	{
		if (ptItem == ptStatistics_CorrLimitsDrift)			// Drift formula: % of limits space
		{
			if(ptStatistics_CorrLimitsDrift->isOn())
				ReportOptions.SetOption("statistics","mean_drift_formula","limits");

			return;
		}

		if (ptItem == ptStatistics_CorrAbsoluteDrift)		// Drift formula: % of value
		{
			if (ptStatistics_CorrAbsoluteDrift->isOn())
				ReportOptions.SetOption("statistics","mean_drift_formula","value");

			return;
		}
	}


	// Customize Cp pareto cut-off limit
	if(ptItem == ptPareto_CpCutOff)
	{
		// Define Cp cutoff limit
		strString = ptPareto_CpCutOff->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);
		cGetString.setDialogText("Highest Cp value to report",
								 "Cp limit:",
								 szString,
								 "Enter the higest Cp value to show in the report");
		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.
		// Get new value
		cGetString.getString(szString);
		double lfParetoCpCutoff=-1.f;
		// Save it to our Options structure.
		if(sscanf(szString,"%lf",&lfParetoCpCutoff) != 1)
			lfParetoCpCutoff = -1;	// No cutoff, report ALL cp results

		ReportOptions.SetOption( "pareto", "cutoff_cp", QString::number(lfParetoCpCutoff) );
		// Update GUI object
		strString = "Only Cp <= to...:";
		strString += QString::number(lfParetoCpCutoff);	//szString;
		ptPareto_CpCutOff->setText(0,strString);

		// Make it the new selected radio button.
		ptPareto_CpCutOff->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptPareto_CpCutOff->parent());
		return;
	}
	if (ptItem==ptPareto_AllCp)
	{ ReportOptions.SetOption( "pareto", "cutoff_cp", QString::number(-1) ); return; }

	if (ptItem==ptPareto_AllCpk)
	{ ReportOptions.SetOption( "pareto", "cutoff_cpk", QString::number(-1) ); return; }

	if(ptItem == ptPareto_CpkCutOff)
	{
		// Define Cp cutoff limit
		strString = ptPareto_CpkCutOff->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Highest Cpk value to report",
								 "Cpk limit:",
								 szString,
								 "Enter the higest Cpk value to show in the report");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		double lfParetoCpkCutoff=-1;
		if(sscanf(szString,"%lf",&lfParetoCpkCutoff) != 1)
			lfParetoCpkCutoff = -1;	// No cutoff, report ALL cpk results

		ReportOptions.SetOption("pareto","cutoff_cpk", QString::number(lfParetoCpkCutoff));

		// Update GUI object
		strString = "Only Cpk <= to...:";
		strString += QString::number(lfParetoCpkCutoff); //szString;
		ptPareto_CpkCutOff->setText(0,strString);

		// Make it the new selected radio button.
		ptPareto_CpkCutOff->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptPareto_CpkCutOff->parent());
		return;
	}

	if(ptItem == ptPareto_Failure_All)
	{	ReportOptions.SetOption("pareto", "cutoff_failure", QString::number(-1)); return; }

	if(ptItem == ptPareto_FailureCutOff)
	{
		// Define test failures cutoff limit
		strString = ptPareto_FailureCutOff->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Total failing tests to report",
								 "Total tests:",
								 szString,
								 "Enter the number of tests to report (Top N)");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		int pfc=-1;
		if (sscanf(szString,"%d",&pfc) != 1)
			pfc = -1;	// No cutoff, report ALL failing tests
		ReportOptions.SetOption("pareto", "cutoff_failure", QString::number(pfc));


		// Update GUI object
		strString = "Total parameters to report (Top N): ";
		strString += QString::number(pfc);	//szString;
		ptPareto_FailureCutOff->setText(0,strString);

		// Make it the new selected radio button.
		ptPareto_FailureCutOff->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptPareto_FailureCutOff->parent());
		return;
	}
	
	/* CRO_cleanup
	if(ptItem == ptPareto_SignatureFailureCutOff)
	{
		// Define Sigbature failure cutoff limit
		strString = ptPareto_SignatureFailureCutOff->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Percentage of signature failures to report",
								 "Percentage limit:",
								 szString,
								 "Enter the total percentage of signature failures to report");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		if(sscanf(szString,"%lf",&ReportOptions.lfParetoFailPatternCutoff) != 1)
			ReportOptions.lfParetoFailPatternCutoff = -1;	// No cutoff, report ALL signature failures

		// Update GUI object
		strString = "Total percentage of Signature failures to report: ";
		strString += szString;
		ptPareto_SignatureFailureCutOff->setText(0,strString);

		// Make it the new selected radio button.
		ptPareto_SignatureFailureCutOff->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptPareto_SignatureFailureCutOff->parent());
		return;
	}
	*/

	// Set total bars/bins per histogram
	if(ptItem == ptHistogram_TotalBars)
	{
		strString = ptHistogram_TotalBars->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Bars/Bins used in Histograms",
								 "Total Bars",
								 szString,
								 "Enter the number of bars/bins used to draw histograms");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		int r=2;

		if(sscanf(szString,"%d",&r) != 1)
			r=TEST_ADVHISTOSIZE;	//ReportOptions.SetOption("adv_histogram","total_bars", QString::number(TEST_ADVHISTOSIZE));
		else if (r<2) r=2;
		else if (r>10000) r=10000;

		ReportOptions.SetOption("adv_histogram","total_bars", QString::number(r));

		/*
		if(sscanf(szString,"%d",&ReportOptions.iHistoClasses) != 1)
			ReportOptions.iHistoClasses = TEST_ADVHISTOSIZE;	// No value, force to default (40)
		if(ReportOptions.iHistoClasses < 2)
			ReportOptions.iHistoClasses = 2;
		if(ReportOptions.iHistoClasses > 10000)
			ReportOptions.iHistoClasses = 10000;
		*/

		// Update GUI object
		strString = "Edit/Change total bars/bins per Histogram: ";
		strString += QString::number(r); //strString += szString;
		ptHistogram_TotalBars->setText(0,strString);

		// Make it the new selected radio button.
		ptHistogram_TotalBars->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptHistogram_TotalBars->parent());
		return;
	}

	if (ptItem == ptScatter_Small)
	{	ReportOptions.SetOption("adv_correlation","chart_size","small"); return; }
	else if (ptItem == ptScatter_Adaptive)
	{	ReportOptions.SetOption("adv_correlation","chart_size","auto"); return; }
	else if (ptItem == ptScatter_Medium)
	{	ReportOptions.SetOption("adv_correlation","chart_size","medium"); return; }
	else if (ptItem == ptScatter_Large)
	{	ReportOptions.SetOption("adv_correlation","chart_size","large"); return; }

	/* CRO_cleanUp by HTH case 4291
	if(ptItem==ptWafermap_StackedBinCount)
		{	ReportOptions.SetOption("wafer","bin_stacked","1"); return; }
	if(ptItem==ptWafermap_StackedPassFailAll)
		{	ReportOptions.SetOption("wafer","bin_stacked","2"); return; }
	*/

	if(ptItem==ptWafermap_StackedParametricMean)
	{	ReportOptions.SetOption("wafer","parametric_stacked","1"); return; }
	if(ptItem==ptWafermap_StackedParametricMedian)
	{	ReportOptions.SetOption("wafer","parametric_stacked","2"); return; }

	if(ptItem==ptWafermap_RetestPolicy_Highest)
	{	ReportOptions.SetOption("wafer","retest_policy","highest_bin"); return;	}
	if(ptItem==ptWafermap_RetestPolicy_Last)
	{	ReportOptions.SetOption("wafer","retest_policy","last_bin"); return;	}

	/* CRO_cleanUp by HTH case# 3819
	if(ptItem == ptWafermap_MarkerRetest)
	{
		if (ptWafermap_MarkerRetest->isOn())
			ReportOptions.bWafmapMarkerDieRetested = true;
		else
			ReportOptions.bWafmapMarkerDieRetested = false;
		return;
	}
	*/

	// Set Stack wafer 'low-yield' threshold alarm.
	if(ptItem == ptWafermap_StackedLowYieldAlarm)
	{
		strString = ptWafermap_StackedLowYieldAlarm->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Low-Yield threshold (failing patterns)",
								 "Low-Yield threshold",
								 szString,
								 "Enter the yield level used to detect failing patterns");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		/*
		if(sscanf(szString,"%f",&ReportOptions.fWafmapStackedLowYieldAlarm) != 1)
			ReportOptions.fWafmapStackedLowYieldAlarm = 33.0;	// No value, force to default (33%)
		if(ReportOptions.fWafmapStackedLowYieldAlarm < 0)
			ReportOptions.fWafmapStackedLowYieldAlarm = 0;
		else
		if(ReportOptions.fWafmapStackedLowYieldAlarm > 100)
			ReportOptions.fWafmapStackedLowYieldAlarm = 100.0;
		// */

		float fOptionStorageDevice=33.0f;
		int nScanRslt = sscanf(szString,"%f",&fOptionStorageDevice);

		if(nScanRslt != 1)
			fOptionStorageDevice = 33.0f;	// No value, force to default (33%)

		if(fOptionStorageDevice < 0)
			fOptionStorageDevice = 0.0f;
		else if(fOptionStorageDevice > 100)
			fOptionStorageDevice =100.0f;

		ReportOptions.SetOption("wafer","low_yield_pattern",QString::number(fOptionStorageDevice));

		// Update GUI object
		strString = "Edit/Change Low-Yield threshold: ";
		// strString += szString;
		strString += QString::number(fOptionStorageDevice);
		ptWafermap_StackedLowYieldAlarm->setText(0,strString);

		// Make it the new selected radio button.
		ptWafermap_StackedLowYieldAlarm->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptWafermap_StackedLowYieldAlarm->parent());
		return;
	}

	// WAFER
	// 'wafer','chart_size'
	{
		if(ptItem == ptWafermap_Adaptive)
		{
			if(ptWafermap_Adaptive->isOn())
			{
				ReportOptions.SetOption("wafer","chart_size","auto");
				return;
			}
		}
		if(ptItem == ptWafermap_Small)
		{
			if(ptWafermap_Small->isOn())
			{
				ReportOptions.SetOption("wafer","chart_size","small");
				return;
			}
		}
		if(ptItem == ptWafermap_Medium)
		{
			if(ptWafermap_Medium->isOn())
			{
				ReportOptions.SetOption("wafer","chart_size","medium");
				return;
			}
		}
		if(ptItem == ptWafermap_Large)
		{
			if(ptWafermap_Large->isOn())
			{
				ReportOptions.SetOption("wafer","chart_size","large");
				return;
			}
		}
	}

	if (ptItem==ptTrend_Adaptive) { ReportOptions.SetOption("adv_trend","chart_size", "auto"); return; }
	if (ptItem==ptTrend_Large) { ReportOptions.SetOption("adv_trend","chart_size", "large"); return; }
	if (ptItem==ptTrend_Small) { ReportOptions.SetOption("adv_trend","chart_size", "small"); return; }
	if (ptItem==ptTrend_Medium) { ReportOptions.SetOption("adv_trend","chart_size", "medium"); return; }
	if (ptItem==ptTrend_Banner) { ReportOptions.SetOption("adv_trend","chart_size", "banner"); return; }

	if (ptItem==ptTrend_TypeLines) { ReportOptions.SetOption("adv_trend","chart_type", "lines"); return; }
	if (ptItem==ptTrend_TypeSpots) { ReportOptions.SetOption("adv_trend","chart_type", "spots"); return; }
	if (ptItem==ptTrend_TypeLinesSpots) { ReportOptions.SetOption("adv_trend","chart_type", "lines_spots"); return; }

	if (ptItem==ptTrend_XAxisRunID) { ReportOptions.SetOption("adv_trend","x_axis", "run_id"); return; }
	if (ptItem==ptTrend_XAxisPartID) { ReportOptions.SetOption("adv_trend","x_axis", "part_id"); return; }


	// Set total parts per rolling yield
	if(ptItem == ptTrend_RollingYield)
	{

		strString = ptTrend_RollingYield->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Parts used in a rolling yield",
			"Total Parts",
			szString,
			"Enter the number of parts used to compute the rolling yield");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		/*
		if(sscanf(szString,"%d",&ReportOptions.iTrendRollingYieldParts) != 1)
			ReportOptions.iTrendRollingYieldParts = 200;	// No value, force to default (200)
		if(ReportOptions.iTrendRollingYieldParts < 1)
			ReportOptions.iTrendRollingYieldParts = 1;
		*/
		int ryp=200;
		if(sscanf(szString,"%d",&ryp) != 1) ryp=200;
		if (ryp<1) ryp=1;
		ReportOptions.SetOption("adv_trend","rolling_yield", QString::number(ryp));
		// Update GUI object
		strString = "Edit/Change total parts per Rolling Yield: ";
		strString += QString::number(ryp);	//szString;
		ptTrend_RollingYield->setText(0,strString);
		// Make it the new selected radio button.
		ptTrend_RollingYield->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptTrend_RollingYield->parent());
		return;
	}

	// Set outlier rule: N*Sigma
	if(ptItem == ptOutlier_sigma)
	{
		strString = ptOutlier_sigma->text(0);
		strString = strString.section("With N=",1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Outlier rule: +/-N*Sigma",
			"N=",
			szString,
			"Enter the number of Sigma for the outlier rule (+/- N*Sigma)");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		/*
		if(sscanf(szString,"%f",&ReportOptions.fOutlierRemoveN_Factor) != 1)
			ReportOptions.fOutlierRemoveN_Factor = 6.0;	// No value, force to default (+/-6*Sigma)
		if(ReportOptions.fOutlierRemoveN_Factor < 0)
			ReportOptions.fOutlierRemoveN_Factor = 0;
		// Update GUI object
		strString.sprintf("+/- N*Sigma range, (centered on Mean). With N=%g",ReportOptions.fOutlierRemoveN_Factor);
		ptOutlier_sigma->setText(0,strString);
		// Make it the new selected radio button.
		ptOutlier_sigma->setOn(true);
		*/

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptOutlier_sigma->parent());
		return;
	}


	// Set outlier rule: N*IQR
	if(ptItem == ptOutlier_IQR)
	{
		strString = ptOutlier_IQR->text(0);
		strString = strString.section("With N=",1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Outlier rule: +/-N*IQR",
			"N=",
			szString,
			"Enter the number of IQR for the outlier rule (+/- N*IQR)");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		/*
		if(sscanf(szString,"%f",&ReportOptions.fOutlierRemoveIQR_Factor) != 1)
			ReportOptions.fOutlierRemoveIQR_Factor = 1.5;	// No value, force to default (+/-1.5*IQR)
		if(ReportOptions.fOutlierRemoveIQR_Factor < 0)
			ReportOptions.fOutlierRemoveIQR_Factor = 0;
		// Update GUI object
		strString.sprintf("IQR based: LL=Q1-N*IQR, HL=Q3+N*IQR. With N=%g",ReportOptions.fOutlierRemoveIQR_Factor);
		ptOutlier_IQR->setText(0,strString);
		// Make it the new selected radio button.
		ptOutlier_IQR->setOn(true);
		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptOutlier_IQR->parent());
		*/
		return;
	}

	// Set outlier rule: Exclude N*Sigma
	if(ptItem == ptOutlier_InlinerSigma)
	{
		strString = ptOutlier_InlinerSigma->text(0);
		strString = strString.section("With N=",1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Outlier rule: Exclude +/-N*Sigma",
			"N=",
			szString,
			"Enter the number of Sigma for the 'Inlier' rule (exclude all data within +/- N*Sigma)");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		/* CRO_cleanup
		if(sscanf(szString,"%f",&ReportOptions.fOutlierRemoveN_Factor) != 1)
			ReportOptions.fOutlierRemoveN_Factor = 6.0;	// No value, force to default (+/-6*Sigma)
		if(ReportOptions.fOutlierRemoveN_Factor < 0)
			ReportOptions.fOutlierRemoveN_Factor = 0;
		// Update GUI object
		strString.sprintf("Exclude +/- N*Sigma range, (centered on Mean). With N=%g",ReportOptions.fOutlierRemoveN_Factor);
		ptOutlier_InlinerSigma->setText(0,strString);
		// Make it the new selected radio button.
		ptOutlier_InlinerSigma->setOn(true);
		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptOutlier_InlinerSigma->parent());
		*/
		return;
	}

	/* CRO_cleanUp by HTH case# 4236
	// Define What-if PASS Binning
	if(ptItem == ptWhatif_Pass_Binning)
	{
		strString = ptWhatif_Pass_Binning->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("What-if Passing Binning",
			"What-If Pass Bin#:",
			szString,
			"Enter Bin# wanted for bad parts Passing the what-if limits");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		if(sscanf(szString,"%d",&ReportOptions.iWhatIf_PassBin) != 1)
			ReportOptions.iWhatIf_PassBin =1;
		if(ReportOptions.iWhatIf_PassBin < 0)
			ReportOptions.iWhatIf_PassBin=0;
		if(ReportOptions.iWhatIf_PassBin > 65535)
			ReportOptions.iWhatIf_PassBin=65535;

		// Update GUI object
		strString = "What-if Passing Binning: ";
		strString += szString;
		ptWhatif_Pass_Binning->setText(0,strString);

		// Make it the new selected radio button.
		ptWhatif_Pass_Binning->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptWhatif_Pass_Binning->parent());
		return;
	}
	
	// Define What-if FAIL Binning
	if(ptItem == ptWhatif_Fail_Binning)
	{
		strString = ptWhatif_Fail_Binning->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("What-if Failing Binning",
			"What-If Fail Bin#:",
			szString,
			"Enter Bin# wanted for good parts Failing the what-if limits");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		if(sscanf(szString,"%d",&ReportOptions.iWhatIf_FailBin) != 1)
			ReportOptions.iWhatIf_FailBin =0;
		if(ReportOptions.iWhatIf_FailBin < 0)
			ReportOptions.iWhatIf_FailBin=0;
		if(ReportOptions.iWhatIf_FailBin > 65535)
			ReportOptions.iWhatIf_FailBin=65535;

		// Update GUI object
		strString = "What-if Failing Binning: ";
		strString += szString;
		ptWhatif_Fail_Binning->setText(0,strString);

		// Make it the new selected radio button.
		ptWhatif_Fail_Binning->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptWhatif_Fail_Binning->parent());
		return;
	}
	*/
	
	/* CRO_cleanUp by HTH case# 4237
	// Define Pearson's correlation threshold limit
	if(ptItem == ptPearson_ThresholdRatio)
	{
		strString = ptPearson_ThresholdRatio->text(0);
		strString = strString.section(':',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("Lowest correlation value to report",
			"Threshold limit:",
			szString,
			"Enter the minimum Pearson's correlation value to show in the report (value between 0.0 and 1.0");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		if(sscanf(szString,"%lf",&ReportOptions.lfPearsonCutoff) != 1)
			ReportOptions.lfPearsonCutoff =0.8;
		if(ReportOptions.lfPearsonCutoff < 0)
			ReportOptions.lfPearsonCutoff=0;
		if(ReportOptions.lfPearsonCutoff > 1)
			ReportOptions.lfPearsonCutoff=1;

		// Update GUI object
		strString = "Ignore tests with a Pearson's correlation ratio (r^2) lower than: ";
		strString += szString;
		ptPearson_ThresholdRatio->setText(0,strString);

		// Make it the new selected radio button.
		ptPearson_ThresholdRatio->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptPearson_ThresholdRatio->parent());
		return;
	}
	*/

	////////////////////////////////
	// Customize R&R chart type
	////////////////////////////////
	{
		if(ptItem == ptBoxPlotEx_Small)
		{ ReportOptions.SetOption("adv_boxplot_ex","chart_size","small"); return; }
		if(ptItem == ptBoxPlotEx_Medium)
		{ ReportOptions.SetOption("adv_boxplot_ex","chart_size","medium"); return; }
		if(ptItem == ptBoxPlotEx_Large)
		{ ReportOptions.SetOption("adv_boxplot_ex","chart_size","large"); return; }
		if(ptItem == ptBoxPlotEx_Adaptive)
		{ ReportOptions.SetOption("adv_boxplot_ex","chart_size","auto"); return; }

		if(ptItem == ptBoxplotEx_Intercative_V)
		{ ReportOptions.SetOption("adv_boxplot_ex","orientation","vertical");	return;	}
		if(ptItem == ptBoxplotEx_Intercative_H)
		{ ReportOptions.SetOption("adv_boxplot_ex","orientation","horizontal");	return;	}

		// 'adv_boxplot', 'sorting'
		if(ptItem == ptBoxplot_SortTestNumber)
		{
			if(ptBoxplot_SortTestNumber->isOn())		// Sort by test number
				ReportOptions.SetOption("adv_boxplot","sorting","test_number");
			return;
		}
		if(ptItem == ptBoxplot_SortTestName)
		{
			if(ptBoxplot_SortTestName->isOn())			// Sort by test name
				ReportOptions.SetOption("adv_boxplot","sorting","test_name");
			return;
		}
		if(ptItem == ptBoxplot_SortTestFlowID)
		{
			if(ptBoxplot_SortTestFlowID->isOn())		// Sort by test flow ID
				ReportOptions.SetOption("adv_boxplot","sorting","test_flow_id");
			return;
		}
		if(ptItem == ptBoxplot_SortMean)
		{
			if(ptBoxplot_SortMean->isOn())				// Sort by Mean
				ReportOptions.SetOption("adv_boxplot","sorting","mean");
			return;
		}
		if(ptItem == ptBoxplot_SortSigma)
		{
			if(ptBoxplot_SortSigma->isOn())				// Sort by Sigma
				ReportOptions.SetOption("adv_boxplot","sorting","sigma");
			return;
		}
		if(ptItem == ptBoxplot_SortCp)
		{
			if(ptBoxplot_SortCp->isOn())				// Sort by Cp
				ReportOptions.SetOption("adv_boxplot","sorting","cp");
			return;
		}
		if(ptItem == ptBoxplot_SortCpk)
		{
			if(ptBoxplot_SortCpk->isOn())				// Sort by Cpk
				ReportOptions.SetOption("adv_boxplot","sorting","cpk");
			return;
		}
		if(ptItem == ptBoxplot_SortR_and_R)
		{
			if(ptBoxplot_SortR_and_R->isOn())			// Sort by R&R
				ReportOptions.SetOption("adv_boxplot","sorting","r&r");
			return;
		}


		if (ptItem == ptBoxplot_ChartAdaptive)
		{
			if (ptBoxplot_ChartAdaptive->isOn())
				ReportOptions.SetOption("adv_boxplot","chart_type","adaptive");

			return;
		}


		if (ptItem == ptBoxplot_ChartLimits)
		{
			if (ptBoxplot_ChartLimits->isOn())
				ReportOptions.SetOption("adv_boxplot","chart_type","limits");

			return;
		}


		if (ptItem == ptBoxplot_ChartRange)
		{
			if (ptBoxplot_ChartRange->isOn())
				ReportOptions.SetOption("adv_boxplot","chart_type","range");

			return;
		}
	}


	// Customize R&R N*Sigma computation rule.
	if(ptItem == ptBoxplot_Nsigma)
	{
		// Define Cp cutoff limit
		strString = ptBoxplot_Nsigma->text(0);
		strString = strString.section('=',1);
		strString = strString.trimmed();
		if(strString.isEmpty())
			*szString = 0;
		else
			strcpy(szString,strString);

		cGetString.setDialogText("R&R N*Sigma",
								 "N value:",
								 szString,
								 "Enter (default: N=5.15)");

		// Display Edit window
		if(cGetString.exec() == 0)
			return;	// User canceled task.

		// Get new value
		cGetString.getString(szString);

		// Save it to our Options structure.
		double lfR_R_Nsigma=0.f;
		if (sscanf(szString,"%lf",&lfR_R_Nsigma) != 1)
			lfR_R_Nsigma = 5.15;	// Default
		if (lfR_R_Nsigma<0.1) lfR_R_Nsigma=0.1;
		ReportOptions.SetOption("adv_boxplot","r&r_sigma", QString::number(lfR_R_Nsigma));

		// Update GUI object
		strString = "Customize N*Sigma space (default N value is 5.15). N=";
		strString += QString::number(lfR_R_Nsigma);	//szString;
		ptBoxplot_Nsigma->setText(0,strString);

		// Make it the new selected radio button.
		ptBoxplot_Nsigma->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptBoxplot_Nsigma->parent());
		return;
	}

	/*
	if(ptItem==ptMultiParametric_Criteria_First) { ReportOptions.SetOption("multi_parametric","criteria","first"); return; }
	if(ptItem==ptMultiParametric_Criteria_Last) { ReportOptions.SetOption("multi_parametric","criteria","last"); return; }
	if(ptItem==ptMultiParametric_Criteria_Min) { ReportOptions.SetOption("multi_parametric","criteria","min"); return; }
	if(ptItem==ptMultiParametric_Criteria_Max) { ReportOptions.SetOption("multi_parametric","criteria","max"); return; }
	if(ptItem==ptMultiParametric_Criteria_Mean) { ReportOptions.SetOption("multi_parametric","criteria","mean"); return; }
	if(ptItem==ptMultiParametric_Criteria_Median) { ReportOptions.SetOption("multi_parametric","criteria","median"); return; }
	*/

	if (ptItem==ptMultiParametric_Options_KeepOne)
	{
		if (ptMultiParametric_Options_KeepOne->isOn())
			ReportOptions.SetOption("multi_parametric","option","keep_one");
		else
			ReportOptions.SetOption("multi_parametric","option","keep_all");
		return;
	}

	/*
	// Customize R&R Alarm rules (colors & thresholds)
	if(ptItem == ptBoxplot_CustomizeAlarm)
	{
		// Write Current rules into file, then launch editor on it
		QString strFilePath = strUserFolder + "/.galaxy_rr.txt";		// name = <path>/.galaxy_rr.txt

		if(QFile::exists(strFilePath) == false)
		{
			// File doesn't exist, so create a template one.
			QFile file(strFilePath); // open file
			if (file.open(IO_WriteOnly) == false)
				return;	// Failed writing file.
			QTextStream hFile(&file);

			// Write default file
			hFile << "#################################################################" << endl;
			hFile << "# Gage R&R Warning definitions " << endl;
			hFile << "#" << endl;
			hFile << "# Template file" << endl;
			hFile << "#" << endl;
			hFile << "# o Any line starting with the '#' is a comment line and is ignored." << endl;
			hFile << "# o Each line is evaluated, and if true, defined reports background colors." << endl;
			hFile << "# o All lines are always evaluated." << endl;
			hFile << "#" << endl;
			hFile << "# Format:" << endl;
			hFile << "# If <Expression> Then Color = <BackgroundColorType>" << endl;
			hFile << "#" << endl;
			hFile << "# Example:" << endl;
			hFile << "# IF R_R < 45 AND Cpk > 1.33 THEN Color = White" << endl;
			hFile << "# IF R_R > 45 AND R_R < 65 AND Cpk > 1.33 THEN Color = Yellow" << endl;
			hFile << "#" << endl;
			hFile << "# Note: for full description refer to the support section of www.mentor.com" << endl;
			hFile << "#       and look at Examinator User Manual (PDF document)" << endl;
			hFile << "#################################################################" << endl;
			hFile << "#" << endl;
			hFile << "# 1) Define default background color in case all other expressions are not verified" << endl;
			hFile << "IF 1 THEN Color = Red" << endl;
			hFile << "#" << endl;
			hFile << "# 2) For Cpk greater than 1.33, the RR should be less than 45% to make it pass" << endl;
			hFile << "IF Cpk > 1.33 AND R_R < 45 THEN Color = Default" << endl;
			hFile << "#" << endl;
			hFile << "# 3) For Cpk between 1.0 and 1.33, the RR should be less than 25% to make it pass" << endl;
			hFile << "IF Cpk > 1.0 AND Cpk < 1.33 AND R_R < 25 THEN Color = Default" << endl;
			hFile << "#" << endl;
			hFile << "# 4) For Cpk less than 1.0, the RR should be less than 10% in order to pass" << endl;
			hFile << "IF Cpk < 1.0 AND R_R < 10 THEN Color = Default" << endl;
			hFile << "#" << endl;
			hFile << "# 5) For Cpk greater than 1.33, and the RR is less than 65%, this is a conditional release" << endl;
			hFile << "IF Cpk > 1.33 AND R_R < 65 THEN Color = Yellow" << endl;
			hFile << "#" << endl;
			hFile << "# 6) For Cpk between 1.0 and 1.33, and the RR is less than 45%, this is also a conditional release" << endl;
			hFile << "IF Cpk > 1.0 AND Cpk < 1.33 AND R_R < 45 THEN Color = Yellow" << endl;
			file.close();
		}

#ifdef _WIN32
		// If file includes a space, we need to bath it between " "
		if(strFilePath.find(' ') != -1)
		{
			strFilePath = "\"" + strFilePath;
			strFilePath = strFilePath + "\"";
		}
		ShellExecuteA(NULL,
					  "open",
					  ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(),   //ReportOptions.strTextEditor.toLatin1().constData(),
					  strFilePath.toLatin1().constData(),
					  NULL,
					  SW_SHOWNORMAL);
#endif
#ifdef unix
		char	szString[2048];
		sprintf(szString,"%s %s&",
				ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().constData(), //ReportOptions.strTextEditor.toLatin1().constData(),
				strFilePath.toLatin1().constData());
		system(szString);
#endif

		// Make it the new selected radio button.
		ptBoxplot_CustomizeAlarm->setOn(true);

		// Highligh the parent line (so focus no longer on this item)
		ListView->setCurrentItem(ptBoxplot_CustomizeAlarm->parent());
		return;
	}
	*/

	if (ptItem==ptDatalog_1Row)
	{ ReportOptions.SetOption("adv_datalog","format","1row"); return; }
	if (ptItem==ptDatalog_2Rows)
	{ ReportOptions.SetOption("adv_datalog","format","2rows"); return; }

	////////////////////////////////////////////////////////
	// Application Preferences
	////////////////////////////////////////////////////////
	{
		if (ptItem == ptGlobalInfoDetailed)
		{
			if(ptGlobalInfoDetailed->isOn())
				ReportOptions.SetOption("global_info","detail_level","detailed");
			return;
		}

		if (ptItem == ptGlobalInfoSummarized)
		{
			if(ptGlobalInfoSummarized->isOn())
				ReportOptions.SetOption("global_info","detail_level","summarized");
			return;
		}		
	}


}

bool GexOptions::RefreshOptionsCenter()
{
	GSLOG(SYSLOG_SEV_INFORMATIONAL, " set to wait cursor...");
	// Change cursor to Hour glass (Wait cursor)...
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

	bool r=ReportOptions.RefreshOptionsCenter();

	// Change cursor back to normal
    QApplication::restoreOverrideCursor();
	return r;
}

///////////////////////////////////////////////////////////
// Forces to update items in the option list that may
// have been changed by a script.
///////////////////////////////////////////////////////////
void GexOptions::RefreshOptionsEdits()
{
	char	szString[512];
	QString	strString;

	// Change cursor to Hour glass (Wait cursor)...
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

	// TEMPARARY, for MASA plugin...forces report to be HTML ONLY!!!!
	if(pGexMainWindow->pPluginMgr != NULL)
	{
		ptOutput_Word->setEnabled(false);
		ptOutput_Ppt->setEnabled(false);
		ptOutput_Pdf->setEnabled(false);
		ptOutput_Interactive->setEnabled(false);
		ptOutput_Html->setOn(true);
	}

	// Databases
	strString = "Local databases path: ";
	QString lp=ReportOptions.GetOption("databases","local_path").toString();
	if(lp.isEmpty() == true)
		strString += "default";
	else
		strString += lp; //ReportOptions.strDatabasesLocalPath;

	ptDatabase_LocalPath->setText(0,strString);

	strString = "Server databases path: ";
	QString sp=ReportOptions.GetOption("databases","server_path").toString();
	if(sp.isEmpty() == true)
		strString += "default";
	else
		strString += sp;

	ptDatabase_ServerPath->setText(0,strString);
	
	QString QDP=ReportOptions.GetOption("databases", "rdb_default_parameters").toString();
	if(QDP=="all")	//(ReportOptions.iRdbQueryDefaultParameters == GEX_OPTION_RDB_EXTRACT_PARAMETERS_ALL)
		ptDatabase_RDB_DefaultParameters_All->setOn(true);
	else
		ptDatabase_RDB_DefaultParameters_None->setOn(true);

	// Disabled selections not available under standard edition and/or OEM editions
	bool bAdvancedStats;
	if(ReportOptions.lEditionID != GEX_EDITION_ADV)	
		bAdvancedStats = false;	// Standard Edition
	else
		bAdvancedStats = true;
	ptOutput_Interactive->setEnabled(bAdvancedStats);	// Enabe/Disabled Interactive mode under standard Edition (except for OEM version...then enabled few line below!)

	// Examinator-OEM doesn't allow advanced statistics!...and few other things!
	switch(ReportOptions.lProductID)
	{
	case GEX_DATATYPE_ALLOWED_CREDENCE_ASL:	// OEM-Examinator for Credence ASL
	case GEX_DATATYPE_ALLOWED_CREDENCE_SAPPHIRE:	// OEM-Examinator for Credence Sapphire
	case GEX_DATATYPE_ALLOWED_SZ:			// OEM-Examinator for Credence SZ
	case GEX_DATATYPE_ALLOWED_LTX:			// OEM-Examinator for LTX
		// No advanced statistics
		bAdvancedStats=false;

		// Interactive (limited) allowed.
		ptOutput_Interactive->setEnabled(true);

		// No support for Word, PowerPoint & PDF formats
		ptOutput_Word->setEnabled(false);
		ptOutput_Ppt->setEnabled(false);
		ptOutput_Interactive->setEnabled(false);

		// No capability to set markers in wafermap.
		ptWafermap_MarkerBin->setEnabled(false);
		ptWafermap_MarkerRetest->setEnabled(false);
		break;
	}

	// Hide nodes (Examinator STANDARD EDITION) or Show nodes (Professional Edition)
	ptNodeSpeedOptimization->setVisible(bAdvancedStats);
	ptStatistics_Shape->setEnabled(bAdvancedStats);
	ptStatistics_TestFlowID->setEnabled(bAdvancedStats);
	ptStatistics_CpkHigh->setEnabled(bAdvancedStats);			// Cpk (High)
	ptStatistics_CpkLow->setEnabled(bAdvancedStats);				// Cpk (Low)
	ptStatistics_Skew->setEnabled(bAdvancedStats);				// Skew
	ptStatistics_GageEV->setEnabled(bAdvancedStats);				// Gage r&r
	ptStatistics_GageAV->setEnabled(bAdvancedStats);				// Gage r&r
	ptStatistics_GageR_and_R->setEnabled(bAdvancedStats);				// Gage r&r
	ptStatistics_GagePV->setEnabled(bAdvancedStats);				// Gage r&r
	ptStatistics_GageTV->setEnabled(bAdvancedStats);				// Gage r&r
	ptStatistics_Kurtosis->setEnabled(bAdvancedStats);
	ptStatistics_P0_5->setEnabled(bAdvancedStats);
	ptStatistics_P2_5->setEnabled(bAdvancedStats);
	ptStatistics_P10->setEnabled(bAdvancedStats);
	ptStatistics_Quartile1->setEnabled(bAdvancedStats);
	ptStatistics_Quartile2->setEnabled(bAdvancedStats);
	ptStatistics_Quartile3->setEnabled(bAdvancedStats);
	ptStatistics_P90->setEnabled(bAdvancedStats);
	ptStatistics_P97_5->setEnabled(bAdvancedStats);
	ptStatistics_P99_5->setEnabled(bAdvancedStats);
	ptStatistics_Interquartiles->setEnabled(bAdvancedStats);
	ptStatistics_SigmaInterQuartiles->setEnabled(bAdvancedStats);	// SigmaInterQuartiles	

	ptOutput_Word->setEnabled(bAdvancedStats);	// Word output format
	ptOutput_Ppt->setEnabled(bAdvancedStats);	// PowerPoint output format
	ptOutput_Pdf->setEnabled(bAdvancedStats);	// PDF output format
	ptHistogram_MarkerMedian->setEnabled(bAdvancedStats);	// Median on Advanced  Histogram
	ptTrend_MarkerMedian->setEnabled(bAdvancedStats);		// Median on Advanced  Histogram
	ptScatter_MarkerMedian->setEnabled(bAdvancedStats);		// Median on Advanced  Histogram

	ptStatistics_CpCpkDefaultFormula->setEnabled(bAdvancedStats);
	ptStatistics_CpCpkPercentileFormula->setEnabled(bAdvancedStats);
	ptPareto_Failure_All->setEnabled(bAdvancedStats);
	ptPareto_FailureCutOff->setEnabled(bAdvancedStats);
	ptPareto_SignatureFailure_All->setEnabled(bAdvancedStats);
	ptPareto_SignatureFailureCutOff->setEnabled(bAdvancedStats);

	// PDF support for OEM-Examinator for Credence (ASL, SZ, Sapphire)
	switch(ReportOptions.lProductID)
	{
	case GEX_DATATYPE_ALLOWED_CREDENCE_ASL:
	case GEX_DATATYPE_ALLOWED_SZ:
	case GEX_DATATYPE_ALLOWED_CREDENCE_SAPPHIRE:	// OEM-Examinator for Credence Sapphire
		ptOutput_Pdf->setEnabled(true);	// PDF output format
		break;
	}

	// If Examinator is running, Hide the 'Database' menu
	switch(ReportOptions.lProductID)
	{
	case GEX_DATATYPE_ALLOWED_ANY:		// Examinator: Any tester type is supported
	case GEX_DATATYPE_ALLOWED_CREDENCE_SAPPHIRE:	// OEM-Examinator for Credence Sapphire
	case GEX_DATATYPE_ALLOWED_CREDENCE_ASL:	// OEM-Examinator for Credence ASL
	case GEX_DATATYPE_ALLOWED_LTX:			// OEM-Examinator for LTX
	case GEX_DATATYPE_ALLOWED_SZ:		// Examinator: Only SZ data files allowed
	case GEX_DATATYPE_ALLOWED_TERADYNE:	// Examinator: Only Teradyne data files allowed
	case GEX_DATATYPE_GEX_TOOLBOX:		// Examinator ToolBox
	default:
		ptNodeDatabaseOptions->setVisible(false);
		ptNodeMonitoring->setVisible(false);
		ptSpeedDatabaseSummary->setVisible(false);
		ptNodeProductionYield->setVisible(false);	// Production yield Node.
		break;
	case GEX_DATATYPE_ALLOWED_DATABASE:	// ExaminatorDB: Database access, any file type.
	case GEX_DATATYPE_ALLOWED_DATABASEWEB:
		ptNodeMonitoring->setVisible(false);
		ptNodeDatabaseOptions->setVisible(true);
		ptSpeedDatabaseSummary->setVisible(bAdvancedStats);
		ptNodeProductionYield->setVisible(bAdvancedStats);	// Production yield Node.
		break;

	case GEX_DATATYPE_GEX_YIELD123:		// Yield123
		ptOutput_Interactive->setVisible(false);	// No 'Ineractive mode' report
		ptOutput_CSV->setVisible(false);			// No 'CSV report' (Yield123 only generate HTML, or HTML flat (Word, PDF, PPT)).
		ptNodeMonitoring->setVisible(false);
		ptNodeDatabaseOptions->setVisible(true);
		ptSpeedDatabaseSummary->setVisible(false);
		ptNodeSpeedOptimization->setVisible(false);	// Speed optimization Node
		ptNodePartID->setVisible(false);			// PartID.
		ptNodeWaferMap->setVisible(false);			// Wafermap Node
		ptNodeHistogram->setVisible(false);			// Histogram Node
		ptNodeTrend->setVisible(false);				// Trend Node.
		ptNodeProductionYield->setVisible(false);	// Production yield Node.
		ptNodeScatter->setVisible(false);			// Scatter Node.
		ptNodeProbabilityPlot->setVisible(false);	// Probability plot Node.
		ptNodeBoxplot->setVisible(false);			// BoxPlot Node.
		ptNodePareto->setVisible(false);			// Pareto Node.
		ptNodeBinning->setVisible(false);			// Binning Node.
		ptNodeDatalog->setVisible(false);			// Datalog Node.
		ptNodePearson->setVisible(false);
		break;

	case GEX_DATATYPE_GEX_MONITORING:	// ExaminatorMonitoring
	case GEX_DATATYPE_GEX_PATMAN:		// Galaxy PAT-Man
		buttonBuildReport->hide();
		ptNodeDatabaseOptions->setVisible(true);
		ptNodeMonitoring->setVisible(true);
		ptMonitoring_Home_Products->setVisible(true);
		ptMonitoring_Home_Testers->setVisible(true);

		ptOutputLocation->setVisible(false);		// Output location
		ptNodePreferences->setVisible(false);		// Application preferences: text editor, etc...
		ptNodeDatalog->setVisible(false);			// Datalog Node.
		ptSpeedDatabaseSummary->setVisible(false);
		ptNodePearson->setVisible(false);

		// Disable auto-logout timeout under Monitoring!
		ptPreferences_AutoCloseNever->setOn(true);

#if 0
		ptNodeTrend->setVisible(false);				// Trend Node.
		ptNodeHistogram->setVisible(false);			// Histogram Node
		ptNodeScatter->setVisible(false);			// Scatter Node.
		ptNodeProbabilityPlot->setVisible(false);	// Probability plot Node.
		ptNodeBoxplot->setVisible(false);			// BoxPlot Node.
#endif
		break;
	}

	// Report output format
	QString of=ReportOptions.GetOption("output", "format").toString();
	if(of=="HTML")	//(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_HTML)
		ptOutput_Html->setOn(true);				// Short HTML pages report.
	if(of=="DOC")	//(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_WORD)
		ptOutput_Word->setOn(true);				// MS-Word format (only under Windows, not available under Unix)
	if(of=="PPT")	//(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_PPT)
		ptOutput_Ppt->setOn(true);				// PowerPoint (only under Windows, not available under Unix)
	if(of=="CSV")	//(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_CSV)
		ptOutput_CSV->setOn(true);				// CSV ASCII report.
	if(of=="PDF")	//(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_PDF)
		ptOutput_Pdf->setOn(true);				// PDF report.
	if(of=="INTERACTIVE")	//(ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_INTERACTIVEONLY)
		ptOutput_Interactive->setOn(true);		// Interactive mode only (no report created)
	
	QString efinpdf=ReportOptions.GetOption("output", "embed_fonts_in_pdf").toString();
	if (efinpdf=="true")
	{	ptEmbedFontsInPdf->setOn(true); ptNoEmbedFontsInPdf->setOn(false); }
	else
	{	ptNoEmbedFontsInPdf->setOn(true); ptEmbedFontsInPdf->setOn(false); }

	// Test labels truncation
	/*
	QString tn=ReportOptions.GetOption("output","truncate_names").toString();
	if (tn!="no") //if(ReportOptions.iTruncateLabels > 0)
		ptOutputTruncateTestNames->setOn(true);
		*/

	// Paper size
	QString ps=ReportOptions.GetOption("output", "paper_size").toString();
	if (ps=="A4") //if(ReportOptions.iPaperSize == GEX_OPTION_PAPER_SIZE_A4)
		ptOutputPaperSize_A4->setOn(true);				// Page are A4 format (for WORD and PDF report generation)
	if (ps=="letter") //if(ReportOptions.iPaperSize == GEX_OPTION_PAPER_SIZE_LETTER)
		ptOutputPaperSize_Letter->setOn(true);				// Page are Letter format (for WORD and PDF report generation)

	QString pf=ReportOptions.GetOption("output", "paper_format").toString();
	if (pf=="portrait") //if(ReportOptions.bPortraitFormat)
		ptOutputPaperFormat_Portrait->setOn(true);	// Portrait format (default)
	else
		ptOutputPaperFormat_Landscape->setOn(true);	// Landscape format

	/* CRO_cleanup
	// If Database mode, do not allow to create report in data file location!
	if((ReportOptions.lProductID == GEX_DATATYPE_ALLOWED_DATABASEWEB) ||
	   (ReportOptions.lProductID == GEX_DATATYPE_GEX_MONITORING) ||
	   (ReportOptions.lProductID == GEX_DATATYPE_GEX_PATMAN))
	{
		ptOutput_PathFile->setVisible(false);
		ptOutput_PathCustom->setOn(true);		// Path = custom (default is c:\ on PC, $HOME under unix)

		if(ReportOptions.lProductID == GEX_DATATYPE_ALLOWED_DATABASEWEB)
		{
			// Update Report Custom location to default => local server path/reports
			UpdateMenuCustomTextItem(ptOutput_PathCustom,"Location: ","");
			// Hide some menu sections...
			ptOutputLocation->setVisible(false);
			ptNodeDatabaseOptions->setVisible(false);
		}
		else
			UpdateMenuCustomTextItem(ptOutput_PathCustom,"Location: ",
									 ReportOptions.GetOption("output", "location").toString().toLatin1().data()	//ReportOptions.strOutputPath.toLatin1().constData()
									 );
	}
	else
	{
		// Update Report Custom location to default => local server path/reports
		UpdateMenuCustomTextItem(ptOutput_PathCustom,"Location: ",
								 ReportOptions.GetOption("output", "location").toString().toLatin1().constData()	//ReportOptions.strOutputPath.toLatin1().constData()
								 );
	}
	*/

	// Update default test name truncation in reports...
	/*		CRO_cleanUp
	QString tn=ReportOptions.GetOption("output","truncate_names").toString();
	bool ok; int tl=tn.toInt(&ok);
	if (!ok)
		tl=-1;
	UpdateMenuCustomTextItem(ptOutputTruncateTestNames,"Truncate test names to length: ",
							 QString::number(tl).toLatin1().constData());
							 */

	// Update Report Custom location to default => local server path/reports
	/* CRO_cleanUp by HTH case# 4193
	UpdateMenuCustomTextItem(ptTempStdfFolder_Custom,"Location: ", ReportOptions.strIntermediateStdfFolder.toLatin1().constData());
	*/

	// Speed optimization
	{
		QString strSpeedOptimizationOptionStorageDevice;

		// 'adv_stats'
		strSpeedOptimizationOptionStorageDevice = (ReportOptions.GetOption("speed","adv_stats")).toString();

		if(strSpeedOptimizationOptionStorageDevice == "always")
		{
			ptSpeedAdvStats_Always->setOn(true);	// Always compute Advanced Statistics
		}
		else if(strSpeedOptimizationOptionStorageDevice == "50mb")
		{
			ptSpeedAdvStats_50MB->setOn(true);		// Compute Adv. stats. if 50MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "100mb")
		{
			ptSpeedAdvStats_100MB->setOn(true);		// Compute Adv. stats. if 100MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "200mb")
		{
			ptSpeedAdvStats_200MB->setOn(true);		// Compute Adv. stats. if 200MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "300mb")
		{
			ptSpeedAdvStats_300MB->setOn(true);		// Compute Adv. stats. if 300MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "400mb")
		{
			ptSpeedAdvStats_400MB->setOn(true);		// Compute Adv. stats. if 400MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "500mb")
		{
			ptSpeedAdvStats_500MB->setOn(true);		// Compute Adv. stats. if 500MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "never")
		{
			ptSpeedAdvStats_Never->setOn(true);		// never Compute Adv. stats
		}
		else
		{
            GEX_ASSERT(false);
		}

		// 'db_summary'
		strSpeedOptimizationOptionStorageDevice = (ReportOptions.GetOption("speed","db_summary")).toString();

		if(strSpeedOptimizationOptionStorageDevice == "always")
		{
			ptSpeedAdvSummaryDB_Always->setOn(true);	// Always compute Advanced Statistics
		}
		else if(strSpeedOptimizationOptionStorageDevice == "50mb")
		{
			ptSpeedAdvSummaryDB_50MB->setOn(true);		// Compute Adv. stats. if 50MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "100mb")
		{
			ptSpeedAdvSummaryDB_100MB->setOn(true);		// Compute Adv. stats. if 100MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "200mb")
		{
			ptSpeedAdvSummaryDB_200MB->setOn(true);		// Compute Adv. stats. if 200MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "300mb")
		{
			ptSpeedAdvSummaryDB_300MB->setOn(true);		// Compute Adv. stats. if 300MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "400mb")
		{
			ptSpeedAdvSummaryDB_400MB->setOn(true);		// Compute Adv. stats. if 400MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "500mb")
		{
			ptSpeedAdvSummaryDB_500MB->setOn(true);		// Compute Adv. stats. if 500MB of data or less.
		}
		else if(strSpeedOptimizationOptionStorageDevice == "never")
		{
			ptSpeedAdvSummaryDB_Never->setOn(true);		// never Compute Adv. stats
		}
		else
		{
            GEX_ASSERT(false);
		}


		/* CRO_cleanUp
		switch(ReportOptions.iSpeedComputeAdvancedStatistics)
		{
		case	GEX_OPTION_SPEED_ADVSTATS_ALWAYS:
			ptSpeedAdvStats_Always->setOn(true);	// Always compute Advanced Statistics
			break;
		case	GEX_OPTION_SPEED_ADVSTATS_50MB:
			ptSpeedAdvStats_50MB->setOn(true);		// Compute Adv. stats. if 50MB of data or less.
			break;
		case	GEX_OPTION_SPEED_ADVSTATS_100MB:
			ptSpeedAdvStats_100MB->setOn(true);		// Compute Adv. stats. if 100MB of data or less.
			break;
		case	GEX_OPTION_SPEED_ADVSTATS_200MB:
			ptSpeedAdvStats_200MB->setOn(true);		// Compute Adv. stats. if 200MB of data or less.
			break;
		case	GEX_OPTION_SPEED_ADVSTATS_300MB:
			ptSpeedAdvStats_300MB->setOn(true);		// Compute Adv. stats. if 300MB of data or less.
			break;
		case	GEX_OPTION_SPEED_ADVSTATS_400MB:
			ptSpeedAdvStats_400MB->setOn(true);		// Compute Adv. stats. if 400MB of data or less.
			break;
		case	GEX_OPTION_SPEED_ADVSTATS_500MB:
			ptSpeedAdvStats_500MB->setOn(true);		// Compute Adv. stats. if 500MB of data or less.
			break;
		case	GEX_OPTION_SPEED_ADVSTATS_NEVER:
			ptSpeedAdvStats_Never->setOn(true);		// never Compute Adv. stats
			break;
		}

		switch(ReportOptions.iSpeedUseSummaryDB)
		{
		case	GEX_OPTION_SPEED_SUMMARYDB_ALWAYS:
			ptSpeedAdvSummaryDB_Always->setOn(true);	// Always use database SUMMARY records instead of data samples
			break;
		case	GEX_OPTION_SPEED_SUMMARYDB_50MB:
			ptSpeedAdvSummaryDB_50MB->setOn(true);		// use summary if 50MB or more of data samples
			break;
		case	GEX_OPTION_SPEED_SUMMARYDB_100MB:
			ptSpeedAdvSummaryDB_100MB->setOn(true);		// use summary if 100MB or more of data samples
			break;
		case	GEX_OPTION_SPEED_SUMMARYDB_200MB:
			ptSpeedAdvSummaryDB_200MB->setOn(true);		// use summary if 200MB or more of data samples
			break;
		case	GEX_OPTION_SPEED_SUMMARYDB_300MB:
			ptSpeedAdvSummaryDB_300MB->setOn(true);		// use summary if 300MB or more of data samples
			break;
		case	GEX_OPTION_SPEED_SUMMARYDB_400MB:
			ptSpeedAdvSummaryDB_400MB->setOn(true);		// use summary if 400MB or more of data samples
			break;
		case	GEX_OPTION_SPEED_SUMMARYDB_500MB:
			ptSpeedAdvSummaryDB_500MB->setOn(true);		// use summary if 500MB or more of data samples
			break;
		case	GEX_OPTION_SPEED_SUMMARYDB_NEVER:
			ptSpeedAdvSummaryDB_Never->setOn(true);		// never use database summary records, only data samples
			break;
		}
		*/
	}

	// Monitoring - History size (number of HTML status pages to keep)
	/*		CRO_cleanUp
	switch(ReportOptions.iMonitorHistory)
	{
	case	GEXMO_OPTION_HISTORY_NONE:
		ptMonitoring_History_none->setOn(true);		// NO history: only pages of the day.
		break;
	case	GEXMO_OPTION_HISTORY_1WEEK:
		ptMonitoring_History_1week->setOn(true);	// Keep 1 week of pages
		break;
	case	GEXMO_OPTION_HISTORY_2WEEK:
		ptMonitoring_History_2week->setOn(true);	// Keep 2 weeks of pages
		break;
	case	GEXMO_OPTION_HISTORY_3WEEK:
		ptMonitoring_History_3week->setOn(true);	// Keep 3 weeks of pages
		break;
	case	GEXMO_OPTION_HISTORY_1MONTH:
		ptMonitoring_History_1month->setOn(true);	// Keep 1 month of pages
		break;
	case	GEXMO_OPTION_HISTORY_2MONTH:
		ptMonitoring_History_2month->setOn(true);	// Keep 2 months of pages
		break;
	case	GEXMO_OPTION_HISTORY_3MONTH:
		ptMonitoring_History_3month->setOn(true);	// Keep 3 months of pages
		break;
	case	GEXMO_OPTION_HISTORY_4MONTH:
		ptMonitoring_History_4month->setOn(true);	// Keep 4 months of pages
		break;
	case	GEXMO_OPTION_HISTORY_5MONTH:
		ptMonitoring_History_5month->setOn(true);	// Keep 5 months of pages
		break;
	case	GEXMO_OPTION_HISTORY_6MONTH:
		ptMonitoring_History_6month->setOn(true);	// Keep 6 months of pages
		break;
	case	GEXMO_OPTION_HISTORY_1YEAR:
		ptMonitoring_History_1year->setOn(true);	// Keep 1 year of pages
		break;
	case	GEXMO_OPTION_HISTORY_ALL:
		ptMonitoring_History_all->setOn(true);		// Keep all pages
		break;
	}
	*/

	// Monitoring - HOME page
	// Show: Products
	/*		CRo_cleanUp
	if(ReportOptions.iMonitorHome & GEXMO_OPTION_HOME_PRODUCT)
		ptMonitoring_Home_Products->setOn(true);
	else
		ptMonitoring_Home_Products->setOn(false);
	// Show: Testers
	if(ReportOptions.iMonitorHome & GEXMO_OPTION_HOME_TESTER)
		ptMonitoring_Home_Testers->setOn(true);
	else
		ptMonitoring_Home_Testers->setOn(false);
	*/

	// Monitoring - PRODUCTS page
	// Show: Parts
	/*		CRO_cleanUp
	if(ReportOptions.iMonitorProduct & GEXMO_OPTION_PRODUCTS_PARTS)
		ptMonitoring_Products_Parts->setOn(true);
	else
		ptMonitoring_Products_Parts->setOn(false);
	// Show: Good parts
	if(ReportOptions.iMonitorProduct & GEXMO_OPTION_PRODUCTS_GOOD)
		ptMonitoring_Products_Good->setOn(true);
	else
		ptMonitoring_Products_Good->setOn(false);
	// Show: Fail parts
	if(ReportOptions.iMonitorProduct & GEXMO_OPTION_PRODUCTS_FAIL)
		ptMonitoring_Products_Fail->setOn(true);
	else
		ptMonitoring_Products_Fail->setOn(false);
	// Show: Yield
	if(ReportOptions.iMonitorProduct & GEXMO_OPTION_PRODUCTS_YIELD)
		ptMonitoring_Products_Yield->setOn(true);
	else
		ptMonitoring_Products_Yield->setOn(false);
	// Show: Alarm level
	if(ReportOptions.iMonitorProduct & GEXMO_OPTION_PRODUCTS_ALARM)
		ptMonitoring_Products_Alarm->setOn(true);
	else
		ptMonitoring_Products_Alarm->setOn(false);
	// Show: Chart
	if(ReportOptions.iMonitorProduct & GEXMO_OPTION_PRODUCTS_CHART)
		ptMonitoring_Products_Chart->setOn(true);
	else
		ptMonitoring_Products_Chart->setOn(false);
	*/
	
	// Monitoring - TESTERS page
	/*		CRO_cleanUp
	// Show: Product
	if(ReportOptions.iMonitorTester & GEXMO_OPTION_TESTERS_PRODUCT)
		ptMonitoring_Testers_Products->setOn(true);
	else
		ptMonitoring_Testers_Products->setOn(false);
	// Show: Operator
	if(ReportOptions.iMonitorTester & GEXMO_OPTION_TESTERS_OPERATOR)
		ptMonitoring_Testers_Operator->setOn(true);
	else
		ptMonitoring_Testers_Operator->setOn(false);
	// Show: Program
	if(ReportOptions.iMonitorTester & GEXMO_OPTION_TESTERS_PROGRAM)
		ptMonitoring_Testers_Program->setOn(true);
	else
		ptMonitoring_Testers_Program->setOn(false);
	// Show: Parts
	if(ReportOptions.iMonitorTester & GEXMO_OPTION_TESTERS_PARTS)
		ptMonitoring_Testers_Parts->setOn(true);
	else
		ptMonitoring_Testers_Parts->setOn(false);
	// Show: Good parts
	if(ReportOptions.iMonitorTester & GEXMO_OPTION_TESTERS_GOOD)
		ptMonitoring_Testers_Good->setOn(true);
	else
		ptMonitoring_Testers_Good->setOn(false);
	// Show: Fail parts
	if(ReportOptions.iMonitorTester & GEXMO_OPTION_TESTERS_FAIL)
		ptMonitoring_Testers_Fail->setOn(true);
	else
		ptMonitoring_Testers_Fail->setOn(false);
	// Show: Yield
	if(ReportOptions.iMonitorTester & GEXMO_OPTION_TESTERS_YIELD)
		ptMonitoring_Testers_Yield->setOn(true);
	else
		ptMonitoring_Testers_Yield->setOn(false);
	// Show: Alarm level
	if(ReportOptions.iMonitorTester & GEXMO_OPTION_TESTERS_ALARM)
		ptMonitoring_Testers_Alarm->setOn(true);
	else
		ptMonitoring_Testers_Alarm->setOn(false);
	// Show: Chart
	if(ReportOptions.iMonitorTester & GEXMO_OPTION_TESTERS_CHART)
		ptMonitoring_Testers_Chart->setOn(true);
	else
		ptMonitoring_Testers_Chart->setOn(false);
	*/


	// Report path

	/*
	if(ReportOptions.GetOption("output", "location").toString().isEmpty()) //ReportOptions.strOutputPath.isEmpty())
		ptOutput_PathFile->setOn(true);			// Path= where first Data file is located
	else
		ptOutput_PathCustom->setOn(true);		// Path= custom path.
	*/

	// Duplicated test handling
	/* CRO_cleanUp by HTH case# 4188
	if(ReportOptions.bMergeDuplicateTestNumber)
		ptDuplicate_Merge->setOn(true);			// Merge test numbers identical (ignore test name)
	else
		if(ReportOptions.bMergeDuplicateTestName)
			ptDuplicate_MergeName->setOn(true);		// Merge test with identical name (even if different test number)
	else
		ptDuplicate_NoMerge->setOn(true);	// No merge same test numbers if test name is different (default)
	*/

	// Show/Hide PartID
	/* CRO_cleanUp by HTH case# 4189
	if(ReportOptions.bShowPartID)
		ptPartID_Show->setOn(true);
	else
		ptPartID_Hide->setOn(true);
	*/

	// Sorting mode
	/* CRO_cleanUp
	switch(ReportOptions.iSorting)
	{
	case	GEX_SORTING_NONE:
	default:
		ptSort_SortNone->setOn(true);	// Data Sorting: None
		break;
	case	GEX_SORTING_BYDATE:
		ptSort_SortByDate->setOn(true);// Data Sorting: by date
		break;
	}
	*/

	// Fail count mode
	/* CRO_cleanUp
	switch(ReportOptions.iFailures)
	{
	case	GEX_FAILCOUNT_ALL:
	default:
		ptFailures_All->setOn(true);	// Fail count mode: ALL failures
		break;
	case	GEX_FAILCOUNT_FIRST:
		ptFailures_First->setOn(true);	// Fail count mode: 1st failure in flow
		break;
	}
	*/

	// Multi-Parametric tests: merge/no merge
	/* CRO_cleanup
	if(ReportOptions.bMultiParametricMerge)
		ptMultiParametric_Merge->setOn(true);	// Merge Multi-parametric pins under a single test name.
	else
		ptMultiParametric_NoMerge->setOn(true);
	*/

	// Multi-Parametric tests: merge/no merge
	/*
	QString mpc=ReportOptions.GetOption("multi_parametric","criteria").toString();
	if(mpc=="first") { ptMultiParametric_Criteria_First->setOn(true);  } else
	if(mpc=="last") { ptMultiParametric_Criteria_Last->setOn(true); } else
	if(mpc=="min") { ptMultiParametric_Criteria_Min->setOn(true); } else
	if(mpc=="max") { ptMultiParametric_Criteria_Max->setOn(true); } else
	if(mpc=="mean") { ptMultiParametric_Criteria_Mean->setOn(true); } else
	if(mpc=="median") { ptMultiParametric_Criteria_Median->setOn(true); }
	*/

	/*
	switch(ReportOptions.cMultiResultValue)
	{
		default								:
		case GEX_MULTIRESULT_USE_FIRST		:	ptMultiParametric_Criteria_First->setOn(true); break;
		case GEX_MULTIRESULT_USE_LAST		:	ptMultiParametric_Criteria_Last->setOn(true); break;
		case GEX_MULTIRESULT_USE_MIN		:	ptMultiParametric_Criteria_Min->setOn(true); break;
		case GEX_MULTIRESULT_USE_MAX		:	ptMultiParametric_Criteria_Max->setOn(true); break;
		case GEX_MULTIRESULT_USE_MEAN		:	ptMultiParametric_Criteria_Mean->setOn(true); break;
		case GEX_MULTIRESULT_USE_MEDIAN		:	ptMultiParametric_Criteria_Median->setOn(true); break;
	}
	*/

	// Handling STDF files (compliancy issues)
	if(ReportOptions.GetOption("dataprocessing", "stdf_compliancy").toString() == "stringent")
		ptStdfCompliancy_Stringent->setOn(true);	// Be stringent on STDF records and orders
	else
		ptStdfCompliancy_Relaxed->setOn(true);		// Be adaptive, and do best to recover data no matter what

	// Intermediate STDF data files path
	/* CRO_cleanUp by HTH case# 4193
	if(ReportOptions.strIntermediateStdfFolder.isEmpty())
		ptTempStdfFolder_Default->setOn(true);		// Path= where test Data file are located
	else
		ptTempStdfFolder_Custom->setOn(true);		// Path= custom path.
	*/

	// outlier
	/* CRO_cleanUp
	switch(ReportOptions.iOutlierRemoveMode)
	{
	case	GEX_OPTION_OUTLIER_NONE:
		ptOutlier_none->setOn(true);			// Outlier=none
		break;
	case	GEX_OPTION_OUTLIER_100LIM:
		ptOutlier_100p->setOn(true);			// Outlier=none
		break;
	case	GEX_OPTION_OUTLIER_150LIM:
		ptOutlier_150p->setOn(true);			// Outlier=none
		break;
	case	GEX_OPTION_OUTLIER_200LIM:
		ptOutlier_200p->setOn(true);			// Outlier=none
		break;
	case	GEX_OPTION_OUTLIER_250LIM:
		ptOutlier_250p->setOn(true);			// Outlier=none
		break;
	case	GEX_OPTION_OUTLIER_300LIM:
		ptOutlier_300p->setOn(true);			// Outlier=none
		break;
	case	GEX_OPTION_OUTLIER_SIGMA:
		ptOutlier_sigma->setOn(true);			// Outlier=Nsigma space centered on Mean
		strString.sprintf("+/- N*Sigma range, (centered on Mean). With N=%g",ReportOptions.fOutlierRemoveN_Factor);
		ptOutlier_sigma->setText(0,strString);
		break;
	case	GEX_OPTION_OUTLIER_IQR:
		ptOutlier_IQR->setOn(true);				// Limits are based on Q1,Q3 +/-N*IQR
		strString.sprintf("IQR based: LL=Q1-N*IQR, HL=Q3+N*IQR. With N=%g",ReportOptions.fOutlierRemoveIQR_Factor);
		ptOutlier_IQR->setText(0,strString);
		break;
	case GEX_OPTION_INLINER_SIGMA:
		ptOutlier_InlinerSigma->setOn(true);		// Outlier= INSIDE of Nsigma space centered on Mean
		strString.sprintf("Exclude +/- N*Sigma range, (centered on Mean). With N=%g",ReportOptions.fOutlierRemoveN_Factor);
		ptOutlier_InlinerSigma->setText(0,strString);
		break;
	}
	*/

	// Statistics
	/* CRO_cleanUp
	switch(ReportOptions.iStatsSource)
	{
		case GEX_STATISTICS_FROM_SAMPLES_ONLY:	// Compute statistics from samples only
			ptStatistics_UseSamplesOnly->setOn(true);
			break;
		case GEX_STATISTICS_FROM_SUMMARY_ONLY:	// Compute statistics from summary only
			ptStatistics_UseSummaryOnly->setOn(true);
			break;
		case GEX_STATISTICS_FROM_SAMPLES_THEN_SUMMARY:	// Compute statistics from samples (if exists), then summary
			ptStatistics_UseSamplesThenSummary->setOn(true);
			break;
		case GEX_STATISTICS_FROM_SUMMARY_THEN_SAMPLES:	// Compute statistics from summary (if exists), then samples
			ptStatistics_UseSummaryThenSamples->setOn(true);
			break;
	}*/
	{
		QString strOptionStorageDevice = (ReportOptions.GetOption("statistics","computation")).toString();
		if(strOptionStorageDevice == "samples_only")				// Compute statistics from samples only
		{
			ptStatistics_UseSamplesOnly->setOn(true);
		}
		else if(strOptionStorageDevice == "summary_only")			// Compute statistics from summary only
		{
			ptStatistics_UseSummaryOnly->setOn(true);
		}
		else if(strOptionStorageDevice == "samples_then_summary")	// Compute statistics from samples (if exists), then summary
		{
			ptStatistics_UseSamplesThenSummary->setOn(true);
		}
		else if(strOptionStorageDevice == "summary_then_samples")	// Compute statistics from summary (if exists), then samples
		{
			ptStatistics_UseSummaryThenSamples->setOn(true);
		}
		else
		{
            GEX_ASSERT(false);
		}
	}


	// Cp, Cpk formula to use
	QString formula=ReportOptions.GetOption("statistics", "cp_cpk_computation").toString();
	if (formula=="percentile") //if(ReportOptions.bStatsCpCpkPercentileFormula)
		ptStatistics_CpCpkPercentileFormula->setOn(true);	// Compute Cp,Cpk stats using percentile formulas
	else
		ptStatistics_CpCpkDefaultFormula->setOn(true);	// Compute Cp,Cpk stats using standard formulas

	/*
	switch(ReportOptions.iSmartScaling)
	{
	case GEX_UNITS_RESCALE_NONE:		// Leave units to the default as found in test data files
		ptScaling_None->setOn(true);
		break;
	case GEX_UNITS_RESCALE_SMART:		// Rescale units to the most appropriate units (eg: mVolts or uVolots, etc)
		ptScaling_Smart->setOn(true);
		break;
	case GEX_UNITS_RESCALE_NORMALIZED:	// Normalize all units (results & test limits)
		ptScaling_Normalized->setOn(true);
		break;
	case GEX_UNITS_RESCALE_TO_LIMITS:	// Rescale all units to limits units.
		ptScaling_ToLimits->setOn(true);
		break;
	}
	*/

	// statistics table
	/*	CRO_cleanUp
	if(ReportOptions.bStatsTableTestName)
		ptStatistics_TestName->setOn(true);		// Test name (test number always part of report)
	else
		ptStatistics_TestName->setOn(false);
	if(ReportOptions.bStatsTableTestType)
		ptStatistics_TestType->setOn(true);		// Test type
	else
		ptStatistics_TestType->setOn(false);
	if(ReportOptions.bStatsTableTestLimits)
		ptStatistics_TestLimits->setOn(true);		// Test limits
	else
		ptStatistics_TestLimits->setOn(false);
	if(ReportOptions.bStatsTableSpecLimits)
		ptStatistics_SpecLimits->setOn(true);		// Test Spec. limits
	else
		ptStatistics_SpecLimits->setOn(false);
	if(ReportOptions.bStatsSource)
		ptStatistics_StatsSource->setOn(true);		// Test limits
	else
		ptStatistics_StatsSource->setOn(false);
	if(ReportOptions.bStatsTableExec)
		ptStatistics_ExecCount->setOn(true);	// Execution count
	else
		ptStatistics_ExecCount->setOn(false);
	if(ReportOptions.bStatsTableFail)
		ptStatistics_FailCount->setOn(true);	// Failures count
	else
		ptStatistics_FailCount->setOn(false);
	if(ReportOptions.bStatsTableFailPercent)
		ptStatistics_Failp->setOn(true);		// Failures %
	else
		ptStatistics_Failp->setOn(false);
	if(ReportOptions.bStatsTableFailBin)
		ptStatistics_FailBin->setOn(true);		// Outlier
	else
		ptStatistics_FailBin->setOn(false);
	if(ReportOptions.bStatsTableOutlier)
		ptStatistics_Outlier->setOn(true);		// Outlier
	else
		ptStatistics_Outlier->setOn(false);
	if(ReportOptions.bStatsTableMean)
		ptStatistics_Mean->setOn(true);			// Mean
	else
		ptStatistics_Mean->setOn(false);
	if(ReportOptions.bStatsTableMeanShift)
		ptStatistics_MeanShift->setOn(true);	// Mean Shift (if comparing files/groups)
	else
		ptStatistics_MeanShift->setOn(false);
	if(ReportOptions.bStatsTableT_Test)
		ptStatistics_T_Test->setOn(true);		// Student's T-Test (if comparing two files/groups)
	else
		ptStatistics_T_Test->setOn(false);
	if(ReportOptions.bStatsTableSigma)
		ptStatistics_Sigma->setOn(true);		// Std. Dev.
	else
		ptStatistics_Sigma->setOn(false);
	if(ReportOptions.bStatsTableSigmaShift)
		ptStatistics_SigmaShift->setOn(true);	// Std. Dev. shift (if comparing files/groups)
	else
		ptStatistics_SigmaShift->setOn(false);
	if(ReportOptions.bStatsTable2Sigma)
		ptStatistics_2Sigma->setOn(true);		// 2Sigma
	else
		ptStatistics_2Sigma->setOn(false);
	if(ReportOptions.bStatsTable3Sigma)
		ptStatistics_3Sigma->setOn(true);		// 3Sigma
	else
		ptStatistics_3Sigma->setOn(false);
	if(ReportOptions.bStatsTable6Sigma)
		ptStatistics_6Sigma->setOn(true);		// 6Sigma
	else
		ptStatistics_6Sigma->setOn(false);
	if(ReportOptions.bStatsTableMinVal)
		ptStatistics_Min->setOn(true);			// Min
	else
		ptStatistics_Min->setOn(false);
	if(ReportOptions.bStatsTableMaxVal)
		ptStatistics_Max->setOn(true);			// Max
	else
		ptStatistics_Max->setOn(false);
	if(ReportOptions.bStatsTableRange)
		ptStatistics_Range->setOn(true);		// Range
	else
		ptStatistics_Range->setOn(false);

	ptStatistics_MaxRange->setOn(ReportOptions.bStatsTableMaxRange);		// Max. Range
	

	if(ReportOptions.bStatsTableCp)
		ptStatistics_Cp->setOn(true);			// Cp
	else
		ptStatistics_Cp->setOn(false);
	if(ReportOptions.bStatsTableCpShift)
		ptStatistics_CpShift->setOn(true);		// Cp shift (if comparing files/groups)
	else
		ptStatistics_CpShift->setOn(false);
	if(ReportOptions.bStatsTableCpk)
		ptStatistics_Cpk->setOn(true);			// Cpk
	else
		ptStatistics_Cpk->setOn(false);
	if(ReportOptions.bStatsTableCpkLow)
		ptStatistics_CpkLow->setOn(true);		// CpkL
	else
		ptStatistics_CpkLow->setOn(false);
	if(ReportOptions.bStatsTableCpkHigh)
		ptStatistics_CpkHigh->setOn(true);		// Cpk
	else
		ptStatistics_CpkHigh->setOn(false);
	if(ReportOptions.bStatsTableCpkShift)
		ptStatistics_CpkShift->setOn(true);		// Cpk
	else
		ptStatistics_CpkShift->setOn(false);
	if(ReportOptions.bStatsTableYield)
		ptStatistics_Yield->setOn(true);		// Yield
	else
		ptStatistics_Yield->setOn(false);
	if(ReportOptions.bStatsTableGageRepeatabilityEV)
		ptStatistics_GageEV->setOn(true);		// gage: EV
	else
		ptStatistics_GageEV->setOn(false);
	if(ReportOptions.bStatsTableGageReproducibilityAV)
		ptStatistics_GageAV->setOn(true);		// gage: AV
	else
		ptStatistics_GageAV->setOn(false);
	if(ReportOptions.bStatsTableGageR_and_R)
		ptStatistics_GageR_and_R->setOn(true);		// gage R&R
	else
		ptStatistics_GageR_and_R->setOn(false);
	if(ReportOptions.bStatsTableGageGB)
		ptStatistics_GageGB->setOn(true);		// gage: GB
	else
		ptStatistics_GageGB->setOn(false);
	if(ReportOptions.bStatsTableGagePV)
		ptStatistics_GagePV->setOn(true);		// gage: PV
	else
		ptStatistics_GagePV->setOn(false);
	if(ReportOptions.bStatsTableGageTV)
		ptStatistics_GageTV->setOn(true);		// gage: TV
	else
		ptStatistics_GageTV->setOn(false);
	if(ReportOptions.bStatsTableGageP_T)
		ptStatistics_GageP_T->setOn(true);		// gage: P/T
	else
		ptStatistics_GageP_T->setOn(false);
	
	// Advanced Statistics
	if(ReportOptions.bStatsTableShape)
		ptStatistics_Shape->setOn(true);		// Distribution shape.
	else
		ptStatistics_Shape->setOn(false);
	if(ReportOptions.bStatsTableTestFlowID)
		ptStatistics_TestFlowID->setOn(true);	// Test order in testing sequence
	else
		ptStatistics_TestFlowID->setOn(false);

	if(ReportOptions.bStatsTableSkew)
		ptStatistics_Skew->setOn(true);			// Skew
	else
		ptStatistics_Skew->setOn(false);
	if(ReportOptions.bStatsTableKurtosis)
		ptStatistics_Kurtosis->setOn(true);		// Kurtosis
	else
		ptStatistics_Kurtosis->setOn(false);
	if(ReportOptions.bStatsTableP0_5)
		ptStatistics_P0_5->setOn(true);	// Percentile 0.5%
	else
		ptStatistics_P0_5->setOn(false);
	if(ReportOptions.bStatsTableP2_5)
		ptStatistics_P2_5->setOn(true);	// Percentile 2.5%
	else
		ptStatistics_P2_5->setOn(false);
	if(ReportOptions.bStatsTableP10)
		ptStatistics_P10->setOn(true);	// Percentile 10%
	else
		ptStatistics_P10->setOn(false);
	if(ReportOptions.bStatsTableQuartile1)
		ptStatistics_Quartile1->setOn(true);	// Quartile1 (percentile 25%)
	else
		ptStatistics_Quartile1->setOn(false);
	if(ReportOptions.bStatsTableQuartile2)
		ptStatistics_Quartile2->setOn(true);	// Quartile2 (percentile 50%)
	else
		ptStatistics_Quartile2->setOn(false);
	if(ReportOptions.bStatsTableQuartile3)
		ptStatistics_Quartile3->setOn(true);	// Quartile3 (percentile 75%)
	else
		ptStatistics_Quartile3->setOn(false);
	if(ReportOptions.bStatsTableP90)
		ptStatistics_P90->setOn(true);	// Percentile 90%
	else
		ptStatistics_P90->setOn(false);
	if(ReportOptions.bStatsTableP97_5)
		ptStatistics_P97_5->setOn(true);	// Percentile 97.5%
	else
		ptStatistics_P97_5->setOn(false);
	if(ReportOptions.bStatsTableP99_5)
		ptStatistics_P99_5->setOn(true);	// Percentile 99.5%
	else
		ptStatistics_P99_5->setOn(false);
	if(ReportOptions.bStatsTableInterquartiles)
		ptStatistics_Interquartiles->setOn(true);	// Interquartiles
	else
		ptStatistics_Interquartiles->setOn(false);
	if(ReportOptions.bStatsTableSigmaInterQuartiles)
		ptStatistics_SigmaInterQuartiles->setOn(true);	// SigmaInterQuartiles
	else
		ptStatistics_SigmaInterQuartiles->setOn(false);
	*/


	{
		QString strOptionStorageDevice = (ReportOptions.GetOption("statistics","sorting")).toString();

		if(strOptionStorageDevice == "test_number")
			ptStatistics_SortTestNumber->setOn(true);	// Sorting: test number
		else if (strOptionStorageDevice == "test_name")
			ptStatistics_SortTestName->setOn(true);		// Sorting: test name
		else if (strOptionStorageDevice == "test_flow_id")
			ptStatistics_SortTestFlowID->setOn(true);	// Sorting: test flow ID (test execution order)
		else if (strOptionStorageDevice == "mean")
			ptStatistics_SortMean->setOn(true);			// Sorting: Mean
		else if (strOptionStorageDevice == "mean_shift")
			ptStatistics_SortMeanShift->setOn(true);	// Sorting: MeanShift
		else if (strOptionStorageDevice == "sigma")
			ptStatistics_SortSigma->setOn(true);		// Sorting: Sigma
		else if (strOptionStorageDevice == "cp")
			ptStatistics_SortCp->setOn(true);			// Sorting: Cp
		else if (strOptionStorageDevice == "cpk")
			ptStatistics_SortCpk->setOn(true);			// Sorting: Cpk
		else if (strOptionStorageDevice == "r&r")
			ptStatistics_SortR_and_R->setOn(true);		// Sorting: Gage R&R
		else
            GEX_ASSERT(false);
	}

	/* CRO_cleanUp
	switch(ReportOptions.iStatsTableSorting)
	{
	case	GEX_SORTING_TESTNUMBER:
		ptStatistics_SortTestNumber->setOn(true);	// Sorting: test number
		break;
	case	GEX_SORTING_TESTNAME:
		ptStatistics_SortTestName->setOn(true);		// Sorting: test name
		break;
	case	GEX_SORTING_TESTFLOWID:
		ptStatistics_SortTestFlowID->setOn(true);	// Sorting: test flow ID (test execution order)
		break;
	case	GEX_SORTING_MEAN:
		ptStatistics_SortMean->setOn(true);			// Sorting: Mean
		break;
	case	GEX_SORTING_MEAN_SHIFT:
		ptStatistics_SortMeanShift->setOn(true);	// Sorting: MeanShift
		break;
	case	GEX_SORTING_SIGMA:
		ptStatistics_SortSigma->setOn(true);		// Sorting: Sigma
		break;
	case	GEX_SORTING_CP:
		ptStatistics_SortCp->setOn(true);			// Sorting: Cp
		break;
	case	GEX_SORTING_CPK:
		ptStatistics_SortCpk->setOn(true);			// Sorting: Cpk
		break;
	case	GEX_SORTING_R_R:
		ptStatistics_SortR_and_R->setOn(true);		// Sorting: Gage R&R
		break;
	}
	*/

	// Refresh cpk alarm & warning levels
	{
		bool bGetOptionRslt;
		double lfOptionStorageDevice;

		lfOptionStorageDevice = (ReportOptions.GetOption("statistics","alarm_test_cpk")).toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);
		if( (lfOptionStorageDevice >= 0) && (bGetOptionRslt) )
		{
			ptStatistics_RedAlarmCpk->setOn(true);	// Alarm set ON: CPK - Red alarm
			sprintf(szString,"Show 'Cpk' Alarm (Red) if under: %g",lfOptionStorageDevice);
		}
		else
		{
			ptStatistics_RedAlarmCpk->setOn(false);	// Alarm set OFF
			strcpy(szString,"Show 'Cpk' Alarm (Red) if under: 1.33");
		}
		ptStatistics_RedAlarmCpk->setText(0,szString);


		lfOptionStorageDevice = (ReportOptions.GetOption("statistics","warning_test_cpk")).toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);
		if((lfOptionStorageDevice >= 0) && (bGetOptionRslt))
		{
			ptStatistics_YellowAlarmCpk->setOn(true);	// Alarm set ON: CPK - Yellow alarm (warning)
			sprintf(szString,"Show 'Cpk' Warning (Yellow) if under: %g",lfOptionStorageDevice);
		}
		else
		{
			ptStatistics_YellowAlarmCpk->setOn(false);	// Alarm set OFF
			strcpy(szString,"Show 'Cpk' Warning (Yellow) if under: 1.67");
		}
		ptStatistics_YellowAlarmCpk->setText(0,szString);
	}


	// Refresh yield alarm and warning levels
	{
		double lfOptionStorageDevice;
		bool bGetOptionRslt;

		// Refresh yield red alarm level
		bGetOptionRslt = false;
		lfOptionStorageDevice = (ReportOptions.GetOption("statistics","alarm_test_yield")).toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);

		if(lfOptionStorageDevice >= 0)
		{
			ptStatistics_RedAlarmTestYield->setOn(true);	// Alarm set ON: Test yield - Red alarm
			sprintf(szString,"Show 'Test yield' Alarm (Red) if under: %g %%",lfOptionStorageDevice);
		}
		else
		{
			ptStatistics_RedAlarmTestYield->setOn(false);	// Alarm set OFF
			strcpy(szString,"Show 'Test yield' Alarm (Red) if under: 80%");
		}
		ptStatistics_RedAlarmTestYield->setText(0,szString);


		// Refresh Mean yield yellow warning level
		bGetOptionRslt = false;
		lfOptionStorageDevice = (ReportOptions.GetOption("statistics","warning_test_yield")).toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);

		if(lfOptionStorageDevice >= 0)
		{
			ptStatistics_YellowAlarmTestYield->setOn(true);	// Alarm set ON: Test yield - Yellow alarm (warning)
			sprintf(szString,"Show 'Test yield' Warning (Yellow) if under: %g %%",lfOptionStorageDevice);
		}
		else
		{
			ptStatistics_YellowAlarmTestYield->setOn(false);	// Alarm set OFF
			strcpy(szString,"Show 'Test yield' Warning (Yellow) if under: 90%");
		}
		ptStatistics_YellowAlarmTestYield->setText(0,szString);
	}

	// drift options
	{
		double lfOptionStorageDevice;
		bool bGetOptionRslt;

		// Refresh Mean Drift alarm level
		bGetOptionRslt = false;
		lfOptionStorageDevice = (ReportOptions.GetOption("statistics","alarm_mean")).toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);

		if( lfOptionStorageDevice >= 0)
		{
			ptStatistics_CorrCheckMean->setOn(true);	// Alarm set ON: Mean shift
			sprintf(szString,"Show Alarm if Mean drifts: %g%%",lfOptionStorageDevice);
		}
		else
		{
			ptStatistics_CorrCheckMean->setOn(false);	// Alarm set OFF
			strcpy(szString,"Show Alarm if Mean drifts: 5%");
		}
		ptStatistics_CorrCheckMean->setText(0,szString);



		// Refresh Sigma Drift alarm level
		bGetOptionRslt = false;
		lfOptionStorageDevice = (ReportOptions.GetOption("statistics","alarm_sigma")).toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);


		if(lfOptionStorageDevice >= 0)
		{
			ptStatistics_CorrCheckSigma->setOn(true);	// Alarm set ON: Sigma shift
			sprintf(szString,"Show Alarm if Sigma drifts: %g%%",lfOptionStorageDevice);
		}
		else
		{
			ptStatistics_CorrCheckSigma->setOn(false);	// Alarm set OFF
			strcpy(szString,"Show Alarm if Sigma drifts: 1%");
		}
		ptStatistics_CorrCheckSigma->setText(0,szString);



		// Refresh Cpk Drift alarm level
		bGetOptionRslt = false;
		lfOptionStorageDevice = (ReportOptions.GetOption("statistics","alarm_cpk")).toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);

		if(lfOptionStorageDevice >= 0)
		{
			ptStatistics_CorrCheckCpk->setOn(true);	// Alarm set ON: Cpk shift
			sprintf(szString,"Show Alarm if Cpk drifts: %g%%",lfOptionStorageDevice);
		}
		else
		{
			ptStatistics_CorrCheckCpk->setOn(false);// Alarm set OFF
			strcpy(szString,"Show Alarm if Cpk drifts: 33%");
		}
		ptStatistics_CorrCheckCpk->setText(0,szString);


		/*
		// Refresh Mean Drift alarm level
		if(ReportOptions.fStatsTableAlarmMeanDrift >= 0)
		{
			ptStatistics_CorrCheckMean->setOn(true);	// Alarm set ON: Mean shift
			sprintf(szString,"Show Alarm if Mean drifts: %g%%",ReportOptions.fStatsTableAlarmMeanDrift);
		}
		else
		{
			ptStatistics_CorrCheckMean->setOn(false);	// Alarm set OFF
			strcpy(szString,"Show Alarm if Mean drifts: 5%");
		}
		ptStatistics_CorrCheckMean->setText(0,szString);

		// Refresh Sigma Drift alarm level
		if(ReportOptions.fStatsTableAlarmSigmaDrift >= 0)
		{
			ptStatistics_CorrCheckSigma->setOn(true);	// Alarm set ON: Sigma shift
			sprintf(szString,"Show Alarm if Sigma drifts: %g%%",ReportOptions.fStatsTableAlarmSigmaDrift);
		}
		else
		{
			ptStatistics_CorrCheckSigma->setOn(false);	// Alarm set OFF
			strcpy(szString,"Show Alarm if Sigma drifts: 1%");
		}
		ptStatistics_CorrCheckSigma->setText(0,szString);

		// Refresh Cpk Drift alarm level
		if(ReportOptions.fStatsTableAlarmCpkDrift >= 0)
		{
			ptStatistics_CorrCheckCpk->setOn(true);	// Alarm set ON: Cpk shift
			sprintf(szString,"Show Alarm if Cpk drifts: %g%%",ReportOptions.fStatsTableAlarmCpkDrift);
		}
		else
		{
			ptStatistics_CorrCheckCpk->setOn(false);// Alarm set OFF
			strcpy(szString,"Show Alarm if Cpk drifts: 33%");
		}
		ptStatistics_CorrCheckCpk->setText(0,szString);
		*/
	}


	// Correlation formula type for the Mean: value or limit space.
	{
		QString strOptionStorageDevice;
		strOptionStorageDevice = ReportOptions.GetOption("statistics","mean_drift_formula").toString();

		if (strOptionStorageDevice == QString("limits"))
		{
			ptStatistics_CorrLimitsDrift->setOn(true);	// Drift formula: % of limits space
			// ?? ptStatistics_CorrAbsoluteDrift->setOn(false); // Drift formula: % of value
		}
		else		// strOptionStorageDevice == QString("value")
		{
			ptStatistics_CorrAbsoluteDrift->setOn(true); // Drift formula: % of value
			// ?? ptStatistics_CorrLimitsDrift->setOn(false);	// Drift formula: % of limits space
		}


		/* switch(ReportOptions.iStatsMeanDriftFormula)
		{
			default:
			case	GEX_DRIFT_ALARM_VALUE:
				ptStatistics_CorrAbsoluteDrift->setOn(true); // Drift formula: % of value
				break;
			case	GEX_DRIFT_ALARM_LIMITS:
				ptStatistics_CorrLimitsDrift->setOn(true);	// Drift formula: % of limits space
				break;
		} */
	}
	
	// Auto generate generic Galaxy tests (die, part, bin, site statistics)
	{
		QString strOptionStorageDevice = (ReportOptions.GetOption("statistics","generic_galaxy_tests")).toString();

		if(strOptionStorageDevice=="show")
		{
			ptStatistics_IncludeGenericTests->setOn(true);
			ptStatistics_ExcludeGenericTests->setOn(false);
		}
		else if(strOptionStorageDevice=="hide")
		{
			ptStatistics_IncludeGenericTests->setOn(false);
			ptStatistics_ExcludeGenericTests->setOn(true);
		}
		else
		{
            GEX_ASSERT(false);
			ptStatistics_IncludeGenericTests->setOn(false);
			ptStatistics_ExcludeGenericTests->setOn(true);
		}

		/* CRO_cleanUp
		if(ReportOptions.bIncludeGenericTests)
		{
			ptStatistics_IncludeGenericTests->setOn(true);
			ptStatistics_ExcludeGenericTests->setOn(false);
		}
		else
		{
			ptStatistics_IncludeGenericTests->setOn(false);
			ptStatistics_ExcludeGenericTests->setOn(true);
		}
		*/
	}

	// Wafermap
	/* CRO_cleanUp by HTH case# 3822
	ptWafermap_ExportAlarm_FlatDiffers->setOn(ReportOptions.bWafmapExportAlarmFlatDiffers);
	ptWafermap_ExportAlarm_NoFlat->setOn(ReportOptions.bWafmapExportAlarmNoFlat);
	*/

	/* CRO_cleanUp by HTH case# 3821
	ptWafermap_MirrorX->setOn(ReportOptions.bWafmapMirrorX);
	ptWafermap_MirrorY->setOn(ReportOptions.bWafmapMirrorY);
	ptWafermap_FullWafer->setOn(ReportOptions.bWafmapFullWafer);
	ptWafermap_AlwaysRound->setOn(ReportOptions.bWafmapAlwaysRound);
	*/
	//ptWafermap_MarkerRetest ... voir plus bas

#if 1 // BG: WaferMap orientation options
	/* CRO_cleanUp by HTH case# 3821
	switch(ReportOptions.iWafmapPositiveX)
	{
	case GEX_OPTION_WAFMAP_POSITIVE_LEFT:
		ptWafermap_PositiveX_Left->setOn(true);
		break;
	case GEX_OPTION_WAFMAP_POSITIVE_RIGHT:
		ptWafermap_PositiveX_Right->setOn(true);
		break;
	case GEX_OPTION_WAFMAP_POSITIVE_AUTO:
		ptWafermap_PositiveX_Detect->setOn(true);
		break;
	}
	switch(ReportOptions.iWafmapPositiveY)
	{
	case GEX_OPTION_WAFMAP_POSITIVE_UP:
		ptWafermap_PositiveY_Up->setOn(true);
		break;
	case GEX_OPTION_WAFMAP_POSITIVE_DOWN:
		ptWafermap_PositiveY_Down->setOn(true);
		break;
	case GEX_OPTION_WAFMAP_POSITIVE_AUTO:
		ptWafermap_PositiveY_Detect->setOn(true);
		break;
	}
	*/
#endif

	if (ReportOptions.GetOption("wafer","retest_policy").toString()=="highest_bin") //ReportOptions.bWafMapRetest_HighestBin)
		ptWafermap_RetestPolicy_Highest->setOn(true);	//Highest bin: promote the highest bin value for each die
	else
		ptWafermap_RetestPolicy_Last->setOn(true);		// Last bin: Use Last bin value found for each die (default)

	{
		QString strOptionStorageDevice = (ReportOptions.GetOption("wafer","chart_size")).toString();

		if(strOptionStorageDevice == "auto")
		{
			ptWafermap_Adaptive->setOn(true);		// Chart size= auto
		}
		else if (strOptionStorageDevice == "small")
		{
			ptWafermap_Small->setOn(true);			// Chart size= Small
		}
		else if (strOptionStorageDevice == "medium")
		{
			ptWafermap_Medium->setOn(true);			// Chart size= Medium
		}
		else if (strOptionStorageDevice == "large")
		{
			ptWafermap_Large->setOn(true);			// Chart size= Large
		}
		else
		{
            GEX_ASSERT(false);
		}

		/* CRO_cleanUp
		switch(ReportOptions.iWafmapChartSize)
		{
		case	GEX_CHARTSIZE_AUTO:
			ptWafermap_Adaptive->setOn(true);		// Chart size= auto
			break;
		case	GEX_CHARTSIZE_SMALL:
			ptWafermap_Small->setOn(true);			// Chart size= Small
			break;
		case	GEX_CHARTSIZE_MEDIUM:
			ptWafermap_Medium->setOn(true);			// Chart size= Medium
			break;
		case	GEX_CHARTSIZE_LARGE:
			ptWafermap_Large->setOn(true);			// Chart size= Large
			break;
		}
		*/
	}

	/* CRO_cleanUp by HTH case# 3819
	if(ReportOptions.bWafmapMarkerBinInDie)
		ptWafermap_MarkerBin->setOn(true);		// Write Bin# in die.
	else
		ptWafermap_MarkerBin->setOn(false);

	if(ReportOptions.bWafmapMarkerDieRetested)
		ptWafermap_MarkerRetest->setOn(true);	// Cross dies retested.
	else
		ptWafermap_MarkerRetest->setOn(false);
	*/

	/* CRO_cleanUp by HTH case# 3801
	// Types of wafermaps to plot.
	if(ReportOptions.iWafmapShow & GEX_OPTION_WAFMAP_STACKED) 
		ptWafermap_Stacked->setOn(true);					// Show bin Stacked wafermap.
	else
		ptWafermap_Stacked->setOn(false);
	if(ReportOptions.iWafmapShow & GEX_OPTION_WAFMAP_INDIVIDUAL) 
		ptWafermap_Individual->setOn(true);					// Show Stacked wafermap.
	else
		ptWafermap_Individual->setOn(false);
	if(ReportOptions.iWafmapShow & GEX_OPTION_WAFMAP_DIEMISMATCH_BIN)
		ptWafermap_DieMismatchBin->setOn(true);				// Show wafermap bin mismatches.
	else
		ptWafermap_DieMismatchBin->setOn(false);

	if(ReportOptions.iWafmapShow & GEX_OPTION_WAFMAP_DIEMISMATCH_BIN_TO_BIN)
		ptWafermap_DieMismatchBinToBin->setOn(true);				// Show wafermap bin to bin mismatches.
	else
		ptWafermap_DieMismatchBinToBin->setOn(false);
	*/

	/* CRO_cleanUp by HTH case# 4290
	if (ReportOptions.GetOption("wafer","parametric_stacked").toInt()==GEX_OPTION_WAFMAP_PARAM_STACKED_MEAN)
			//ReportOptions.iWafmapParametricStacked == GEX_OPTION_WAFMAP_PARAM_STACKED_MEAN)
		ptWafermap_StackedParametricMean->setOn(true);
	else
		ptWafermap_StackedParametricMedian->setOn(true);
	*/

	// Type of bin stacked wafermap
	/* CRO_cleanUp by HTH case# 4291
	if (ReportOptions.GetOption("wafer","bin_stacked").toInt()== GEX_OPTION_WAFMAP_BIN_STACKED_BINCOUNT)
		//iWafmapBinStacked == GEX_OPTION_WAFMAP_BIN_STACKED_BINCOUNT)
		ptWafermap_StackedBinCount->setOn(true);
	else if (ReportOptions.GetOption("wafer","bin_stacked").toInt()== GEX_OPTION_WAFMAP_BIN_STACKED_PASSFAILALL)
			 //iWafmapBinStacked == GEX_OPTION_WAFMAP_BIN_STACKED_PASSFAILALL)
		ptWafermap_StackedPassFailAll->setOn(true);
	*/

#if 0	// Waiting for new options
	else if (ReportOptions.iWafmapBinStacked == GEX_OPTION_WAFMAP_BIN_STACKED_PASSFAILATLEAST)
		ptWafermap_StackedPassFailAtLeast->setOn(true);
	else if (ReportOptions.iWafmapBinStacked == GEX_OPTION_WAFMAP_BIN_STACKED_HIGHESTBIN)
		ptWafermap_StackedHighestBin->setOn(true);
	else if (ReportOptions.iWafmapBinStacked == GEX_OPTION_WAFMAP_BIN_STACKED_HIGHESTFAILINGBIN)
		ptWafermap_StackedLowestBin->setOn(true);
#endif

	// Low-Yie		ld alarm for failing pattern identification in stacked wafers.

	bool bOptionConversionRslt;
	float fOptionStorageDevice = ReportOptions.GetOption("wafer","low_yield_pattern").toFloat(&bOptionConversionRslt);
    GEX_ASSERT( (bOptionConversionRslt) && !(fOptionStorageDevice<0) && !(fOptionStorageDevice>100) );
	sprintf(szString,"Edit/Change Low-Yield threshold: %.2f",fOptionStorageDevice);
	ptWafermap_StackedLowYieldAlarm->setText(0,szString);

	// Accept comparing wafers with size not matching?
	/* CRO_cleanUp by HTH case# 3820
	ptWafermap_CompareAnySize->setOn(ReportOptions.bWafmapCompareAnySize);
	ptWafermap_IncludeDieMismatchTable->setOn(ReportOptions.bWafmapIncludeDieMismatchTable);
	ptWafermap_IncludeDeltaYieldSection->setOn(ReportOptions.bWafmapIncludeDeltaYieldSection);
	*/

	// (Advanced) Histogram
	QString hcs=ReportOptions.GetOption("adv_histogram","chart_size").toString();
	if (hcs=="auto") ptHistogram_Adaptive->setOn(true);	// Chart size= auto
	else if (hcs=="small") ptHistogram_Small->setOn(true);
	else if (hcs=="medium") ptHistogram_Medium->setOn(true);
	else if (hcs=="large") ptHistogram_Large->setOn(true);
	else if (hcs=="banner") ptHistogram_Banner->setOn(true);
	/*
	switch(ReportOptions.iHistoChartSize)
	{
	case	GEX_CHARTSIZE_AUTO:
		ptHistogram_Adaptive->setOn(true);		// Chart size= auto
		break;
	case	GEX_CHARTSIZE_SMALL:
		ptHistogram_Small->setOn(true);			// Chart size= Small
		break;
	case	GEX_CHARTSIZE_MEDIUM:
		ptHistogram_Medium->setOn(true);		// Chart size= Medium
		break;
	case	GEX_CHARTSIZE_LARGE:
		ptHistogram_Large->setOn(true);			// Chart size= Large
		break;
	case	GEX_CHARTSIZE_BANNER:
		ptHistogram_Banner->setOn(true);		// Chart size= Banner format
		break;
	}
	*/

	// Histogram Y-axis scaling type.
	if(ReportOptions.GetOption("adv_histogram","y_axis").toString()=="hits")	//bHistoFrequencyCountScale)
		ptHistogram_Yaxis_Hits->setOn(true);
	else
		ptHistogram_Yaxis_Percentage->setOn(true);

	// Charting type
	{
		QString strOptionStorageDevice = (ReportOptions.GetOption("adv_histogram","chart_type")).toString();

		if(strOptionStorageDevice == "bars")
		{
			ptHistogram_TypeBars->setOn(true);		// Chart type= Bars
		}
		else if(strOptionStorageDevice == "3d_bars")
		{
			ptHistogram_Type3D_Bars->setOn(true);	// Chart type= 3DBars
		}
		else if(strOptionStorageDevice == "bars_outline")
		{
			ptHistogram_TypeBarsOutline->setOn(true);	// Chart type= Bars Outline
		}
		else if(strOptionStorageDevice == "curve")
		{
			ptHistogram_TypeSpline->setOn(true);	// Chart type= Fitting curve
		}
		else if(strOptionStorageDevice == "bars_curve")
		{
			ptHistogram_TypeBarsSpline->setOn(true);// Chart type= Bars+Fitting curve
		}
		else if(strOptionStorageDevice == "gaussian")
		{
			ptHistogram_TypeGaussian->setOn(true);// Chart type= Gaussian
		}
		else if(strOptionStorageDevice == "bars_gaussian")
		{
			ptHistogram_TypeBarsGaussian->setOn(true);// Chart type= Gaussian+Bars
		}
		else if(strOptionStorageDevice == "bars_outline_gaussian")
		{
			ptHistogram_TypeBarsOutlineGaussian->setOn(true);// Chart type= Gaussian+Bars outline
		}
		else
		{
            GEX_ASSERT(false);			// wrong field value
		}
	}

	/* CRO_cleanUp
	switch(ReportOptions.iHistoChartType)
	{
	case	GEX_CHARTTYPE_BARS:
		ptHistogram_TypeBars->setOn(true);		// Chart type= Bars
		break;
	case	GEX_CHARTTYPE_3DBARS:
		ptHistogram_Type3D_Bars->setOn(true);	// Chart type= 3DBars
		break;
	case	GEX_CHARTTYPE_BARSOUTLINE:
		ptHistogram_TypeBarsOutline->setOn(true);	// Chart type= Bars Outline
		break;
	case	GEX_CHARTTYPE_SPLINE:
		ptHistogram_TypeSpline->setOn(true);	// Chart type= Fitting curve
		break;
	case	GEX_CHARTTYPE_BARSSPLINE:
		ptHistogram_TypeBarsSpline->setOn(true);// Chart type= Bars+Fitting curve
		break;
	case	GEX_CHARTTYPE_GAUSSIAN:
		ptHistogram_TypeGaussian->setOn(true);// Chart type= Gaussian
		break;
	case	GEX_CHARTTYPE_BARSGAUSSIAN:
		ptHistogram_TypeBarsGaussian->setOn(true);// Chart type= Gaussian+Bars
		break;
	case	GEX_CHARTTYPE_BARSOUTLINEGAUSSIAN:
		ptHistogram_TypeBarsOutlineGaussian->setOn(true);// Chart type= Gaussian+Bars outline
		break;
	}
	*/

	// Bars/Bins used to draw histogram
	sprintf(szString, "Edit/Change total bars/bins per Histogram: %d",
			ReportOptions.GetOption("adv_histogram","total_bars").toInt()); //sprintf(szString, "Edit/Change total bars/bins per Histogram: %d", ReportOptions.iHistoClasses);
	ptHistogram_TotalBars->setText(0,szString);

	/*	CRO_cleanUp
	if(ReportOptions.bHistoMarkerTitle)
		ptHistogram_MarkerTestName->setOn(true);// Title (Test name or else)
	else
		ptHistogram_MarkerTestName->setOn(false);
	if(ReportOptions.bHistoMarkerMean)
		ptHistogram_MarkerMean->setOn(true);	// Mean
	else
		ptHistogram_MarkerMean->setOn(false);
	if(ReportOptions.bHistoMarkerMedian)
		ptHistogram_MarkerMedian->setOn(true);	// Median
	else
		ptHistogram_MarkerMedian->setOn(false);
	if(ReportOptions.bHistoMarkerLimits)
		ptHistogram_MarkerLimits->setOn(true);	// Test Limits
	else
		ptHistogram_MarkerLimits->setOn(false);
	if(ReportOptions.bHistoMarkerSpecLimits)
		ptHistogram_MarkerSpecLimits->setOn(true);	// Test Spec Limits
	else
		ptHistogram_MarkerSpecLimits->setOn(false);
	if(ReportOptions.bHistoMarker2Sigma)
		ptHistogram_Marker2Sigma->setOn(true);	// 2Sigma
	else
		ptHistogram_Marker2Sigma->setOn(false);
	if(ReportOptions.bHistoMarker3Sigma)
		ptHistogram_Marker3Sigma->setOn(true);	// 3Sigma
	else
		ptHistogram_Marker3Sigma->setOn(false);
	if(ReportOptions.bHistoMarker6Sigma)
		ptHistogram_Marker6Sigma->setOn(true);	// 6Sigma
	else
		ptHistogram_Marker6Sigma->setOn(false);
	*/

	/*		CRO_cleanUp
	if(ReportOptions.bHistoStatsLimits)
		ptHistogram_Limits->setOn(true);	// Test limits
	else
		ptHistogram_Limits->setOn(false);
	if(ReportOptions.bHistoStatsExec)
		ptHistogram_ExecCount->setOn(true);	// Exec. count
	else
		ptHistogram_ExecCount->setOn(false);
	if(ReportOptions.bHistoStatsMean)
		ptHistogram_Mean->setOn(true);		// Mean
	else
		ptHistogram_Mean->setOn(false);
	if(ReportOptions.bHistoStatsQuartile2)
		ptHistogram_Quartile2->setOn(true);	// Median
	else
		ptHistogram_Quartile2->setOn(false);
	if(ReportOptions.bHistoStatsSigma)
		ptHistogram_Sigma->setOn(true);		// Sigma
	else
		ptHistogram_Sigma->setOn(false);
	if(ReportOptions.bHistoStatsOutlier)
		ptHistogram_Outlier->setOn(true);	// Outlier
	else
		ptHistogram_Outlier->setOn(false);
	if(ReportOptions.bHistoStatsFails)
		ptHistogram_Fails->setOn(true);	// Failures
	else
		ptHistogram_Fails->setOn(false);
	if(ReportOptions.bHistoStatsCp)
		ptHistogram_Cp->setOn(true);		// Cp
	else
		ptHistogram_Cp->setOn(false);
	if(ReportOptions.bHistoStatsCpk)
		ptHistogram_Cpk->setOn(true);		// Cpk
	else
		ptHistogram_Cpk->setOn(false);
	
	if(ReportOptions.bHistoStatsMin)		// Min
		ptHistogram_Min->setOn(true);
	else
		ptHistogram_Min->setOn(false);

	if(ReportOptions.bHistoStatsMax)		// Max
		ptHistogram_Max->setOn(true);
	else
		ptHistogram_Max->setOn(false);
	*/

	// Trend
	QString atcs=ReportOptions.GetOption("adv_trend","chart_size").toString();
	if (atcs=="auto") ptTrend_Adaptive->setOn(true);
	else if (atcs=="small") ptTrend_Small->setOn(true);
	else if (atcs=="medium") ptTrend_Medium->setOn(true);
	else if (atcs=="large") ptTrend_Large->setOn(true);
	else if (atcs=="banner") ptTrend_Banner->setOn(true);
	/*
	switch(ReportOptions.iTrendChartSize)
	{
	case	GEX_CHARTSIZE_AUTO:
		ptTrend_Adaptive->setOn(true);		// Chart size= auto
		break;
	case	GEX_CHARTSIZE_SMALL:
		ptTrend_Small->setOn(true);			// Chart size= Small
		break;
	case	GEX_CHARTSIZE_MEDIUM:
		ptTrend_Medium->setOn(true);		// Chart size= Medium
		break;
	case	GEX_CHARTSIZE_LARGE:
		ptTrend_Large->setOn(true);			// Chart size= Large
		break;
	case	GEX_CHARTSIZE_BANNER:
		ptTrend_Banner->setOn(true);		// Chart size= Banner
		break;
	}
	*/

	/*
	switch(ReportOptions.iTrendChartType)
	{
	case	GEX_CHARTTYPE_LINES:
		ptTrend_TypeLines->setOn(true);		// Chart type= Lines
		break;
	case	GEX_CHARTTYPE_SPOTS:
		ptTrend_TypeSpots->setOn(true);		// Chart type= Spots
		break;
	case	GEX_CHARTTYPE_LINESSPOTS:
		ptTrend_TypeLinesSpots->setOn(true);// Chart type= Lines+Spots
		break;
	}
	*/
	QString atct=ReportOptions.GetOption("adv_trend","chart_type").toString();
	if (atct=="lines") ptTrend_TypeLines->setOn(true);		// Chart type= Lines
	else if (atct=="spots") ptTrend_TypeSpots->setOn(true);		// Chart type= Spots
	else if (atct=="lines_spots")	ptTrend_TypeLinesSpots->setOn(true);// Chart type= Lines+Spots

	/*
	switch(ReportOptions.iTrendXAxis)
	{
	case GEX_TREND_XAXIS_RUNID	:
	default		:
			ptTrend_XAxisRunID->setOn(true);
	break;

	case GEX_TREND_XAXIS_PARTID:
	ptTrend_XAxisPartID->setOn(true);
	break;
	}
	*/
	if (ReportOptions.GetOption("adv_trend","x_axis").toString()=="part_id")
		ptTrend_XAxisPartID->setOn(true);
	else	ptTrend_XAxisRunID->setOn(true);


	/*		CRO_cleanUp
	if(ReportOptions.bTrendMarkerTitle)
		ptTrend_MarkerTestName->setOn(true);// Title (Test name or else)
	else
		ptTrend_MarkerTestName->setOn(false);
	if(ReportOptions.bTrendMarkerMean)
		ptTrend_MarkerMean->setOn(true);	// Mean
	else
		ptTrend_MarkerMean->setOn(false);
	if(ReportOptions.bTrendMarkerMedian)
		ptTrend_MarkerMedian->setOn(true);	// Median
	else
		ptTrend_MarkerMedian->setOn(false);
	if(ReportOptions.bTrendMarkerLimits)
		ptTrend_MarkerLimits->setOn(true);	// Test Limits
	else
		ptTrend_MarkerLimits->setOn(false);
	if(ReportOptions.bTrendMarkerSpecLimits)
		ptTrend_MarkerSpecLimits->setOn(true);	// Test Spec Limits
	else
		ptTrend_MarkerSpecLimits->setOn(false);
	if(ReportOptions.bTrendMarkerLot)
		ptTrend_MarkerLot->setOn(true);		// Lot ID
	else
		ptTrend_MarkerLot->setOn(false);
	if(ReportOptions.bTrendMarkerSubLot)
		ptTrend_MarkerSubLot->setOn(true);	// Sub-Lot / Wafer ID
	else
		ptTrend_MarkerSubLot->setOn(false);
	if(ReportOptions.bTrendMarkerGroupName)
		ptTrend_MarkerGroupName->setOn(true);	// Dataset name/group name
	else
		ptTrend_MarkerGroupName->setOn(false);
	if(ReportOptions.bTrendMarker2Sigma)
		ptTrend_Marker2Sigma->setOn(true);	// 2Sigma
	else
		ptTrend_Marker2Sigma->setOn(false);
	if(ReportOptions.bTrendMarker3Sigma)
		ptTrend_Marker3Sigma->setOn(true);	// 3Sigma
	else
		ptTrend_Marker3Sigma->setOn(false);
	if(ReportOptions.bTrendMarker6Sigma)
		ptTrend_Marker6Sigma->setOn(true);	// 6Sigma
	else
		ptTrend_Marker6Sigma->setOn(false);
	*/

	// Parts used to compute the rolling yield
	//sprintf(szString,"Edit/Change total parts per Rolling Yield: %d",ReportOptions.iTrendRollingYieldParts);
	sprintf(szString,"Edit/Change total parts per Rolling Yield: %d",ReportOptions.GetOption("adv_trend","rolling_yield").toInt());
	ptTrend_RollingYield->setText(0,szString);

	// Correlation XY-Scatter
	/* case 3842
	switch(ReportOptions.iScatterChartSize)
	{
	case	GEX_CHARTSIZE_AUTO:
		ptScatter_Adaptive->setOn(true);		// Chart size= auto
		break;
	case	GEX_CHARTSIZE_SMALL:
		ptScatter_Small->setOn(true);			// Chart size= Small
		break;
	case	GEX_CHARTSIZE_MEDIUM:
		ptScatter_Medium->setOn(true);			// Chart size= Medium
		break;
	case	GEX_CHARTSIZE_LARGE:
		ptScatter_Large->setOn(true);			// Chart size= Large
		break;
	}
	*/
	QString scs=ReportOptions.GetOption("adv_correlation","chart_size").toString();
	if (scs=="auto") ptScatter_Adaptive->setOn(true);
	else if(scs=="small") ptScatter_Small->setOn(true);
	else if (scs=="medium") ptScatter_Medium->setOn(true);
	else if (scs=="large") ptScatter_Large->setOn(true);

	/*	switch(ReportOptions.iScatterChartType)
	{
	case	GEX_CHARTTYPE_LINES:
		ptScatter_TypeLines->setOn(true);		// Chart type= Lines
		break;
	case	GEX_CHARTTYPE_SPOTS:
		ptScatter_TypeSpots->setOn(true);		// Chart type= Spots
		break;
	case	GEX_CHARTTYPE_LINESSPOTS:
		ptScatter_TypeLinesSpots->setOn(true);// Chart type= Lines+Spots
		break;
	}*/

	/*
	if(ReportOptions.bScatterMarkerTitle)
		ptScatter_MarkerTestName->setOn(true);	// Title (Test name or else)
	else
		ptScatter_MarkerTestName->setOn(false);
	if(ReportOptions.bScatterMarkerMean)
		ptScatter_MarkerMean->setOn(true);		// Mean
	else
		ptScatter_MarkerMean->setOn(false);
	if(ReportOptions.bScatterMarkerMedian)
		ptScatter_MarkerMedian->setOn(true);	// Median
	else
		ptScatter_MarkerMedian->setOn(false);
	if(ReportOptions.bScatterMarkerLimits)
		ptScatter_MarkerLimits->setOn(true);	// Test Limits
	else
		ptScatter_MarkerLimits->setOn(false);
	if(ReportOptions.bScatterMarkerSpecLimits)
		ptScatter_MarkerSpecLimits->setOn(true);	// Test Spec Limits
	else
		ptScatter_MarkerSpecLimits->setOn(false);
	if(ReportOptions.bScatterMarker2Sigma)
		ptScatter_Marker2Sigma->setOn(true);	// 2Sigma
	else
		ptScatter_Marker2Sigma->setOn(false);
	if(ReportOptions.bScatterMarker3Sigma)
		ptScatter_Marker3Sigma->setOn(true);	// 3Sigma
	else
		ptScatter_Marker3Sigma->setOn(false);
	if(ReportOptions.bScatterMarker6Sigma)
		ptScatter_Marker6Sigma->setOn(true);	// 6Sigma
	else
		ptScatter_Marker6Sigma->setOn(false);
	*/
		
	// Probability plot chart 
	QString pbcs=ReportOptions.GetOption("adv_probabilityplot","chart_size").toString();
	if (pbcs=="auto") ptProbPlot_Adaptive->setOn(true);		 // Chart size= auto
	else if (pbcs=="small") ptProbPlot_Small->setOn(true);	 // Chart size= Small
	else if (pbcs=="medium") ptProbPlot_Medium->setOn(true); // Chart size= Medium
	else if (pbcs=="large") ptProbPlot_Large->setOn(true);	 // Chart size= Large
	/*
	switch(ReportOptions.iProbPlotChartSize)
	{
	case	GEX_CHARTSIZE_AUTO:
		ptProbPlot_Adaptive->setOn(true);		// Chart size= auto
		break;
	case	GEX_CHARTSIZE_SMALL:
		ptProbPlot_Small->setOn(true);			// Chart size= Small
		break;
	case	GEX_CHARTSIZE_MEDIUM:
		ptProbPlot_Medium->setOn(true);			// Chart size= Medium
		break;
	case	GEX_CHARTSIZE_LARGE:
		ptProbPlot_Large->setOn(true);			// Chart size= Large
		break;
	}
	*/

	/*		CRO_cleanUp
	if(ReportOptions.bProbPlotMarkerTitle)
		ptProbPlot_MarkerTestName->setOn(true);	// Title (Test name or else)
	else
		ptProbPlot_MarkerTestName->setOn(false);
	if(ReportOptions.bProbPlotMarkerMean)
		ptProbPlot_MarkerMean->setOn(true);		// Mean
	else
		ptProbPlot_MarkerMean->setOn(false);
	if(ReportOptions.bProbPlotMarkerMedian)
		ptProbPlot_MarkerMedian->setOn(true);	// Median
	else
		ptProbPlot_MarkerMedian->setOn(false);
	if(ReportOptions.bProbPlotMarkerLimits)
		ptProbPlot_MarkerLimits->setOn(true);	// Test Limits
	else
		ptProbPlot_MarkerLimits->setOn(false);
	if(ReportOptions.bProbPlotMarkerSpecLimits)
		ptProbPlot_MarkerSpecLimits->setOn(true);	// Test Spec Limits
	else
		ptProbPlot_MarkerSpecLimits->setOn(false);
	if(ReportOptions.bProbPlotMarker2Sigma)
		ptProbPlot_Marker2Sigma->setOn(true);	// 2Sigma
	else
		ptProbPlot_Marker2Sigma->setOn(false);
	if(ReportOptions.bProbPlotMarker3Sigma)
		ptProbPlot_Marker3Sigma->setOn(true);	// 3Sigma
	else
		ptProbPlot_Marker3Sigma->setOn(false);
	if(ReportOptions.bProbPlotMarker6Sigma)
		ptProbPlot_Marker6Sigma->setOn(true);	// 6Sigma
	else
		ptProbPlot_Marker6Sigma->setOn(false);
	*/

	QString pbya=ReportOptions.GetOption("adv_probabilityplot","y_axis").toString();
	if (pbya=="sigma")	//if (ReportOptions.bProbPlotYAxisSigma == true)
		ptProbPlot_Yaxis_Sigma->setOn(true);
	else
		ptProbPlot_Yaxis_Percent->setOn(true);
	
	// Box plot chart 
	QString abcs=ReportOptions.GetOption("adv_boxplot_ex","chart_size").toString();
	if (abcs=="auto") ptBoxPlotEx_Adaptive->setOn(true);		// Chart size= auto
	if (abcs=="small") ptBoxPlotEx_Small->setOn(true);			// Chart size= Small
	if (abcs=="medium") ptBoxPlotEx_Medium->setOn(true);			// Chart size= Small
	if (abcs=="large") ptBoxPlotEx_Large->setOn(true);
	/*
	switch(ReportOptions.iBoxPlotExChartSize)
	{
	case	GEX_CHARTSIZE_AUTO:
		ptBoxPlotEx_Adaptive->setOn(true);		// Chart size= auto
		break;
	case	GEX_CHARTSIZE_SMALL:
		ptBoxPlotEx_Small->setOn(true);			// Chart size= Small
		break;
	case	GEX_CHARTSIZE_MEDIUM:
		ptBoxPlotEx_Medium->setOn(true);			// Chart size= Medium
		break;
	case	GEX_CHARTSIZE_LARGE:
		ptBoxPlotEx_Large->setOn(true);			// Chart size= Large
		break;
	}
	*/

	/* CRO_cleanUp
	// Title (Test name or else)
	if(ReportOptions.bBoxPlotExMarkerTitle)
		ptBoxPlotEx_MarkerTestName->setOn(true);
	else
		ptBoxPlotEx_MarkerTestName->setOn(false);
	// Mean
	if(ReportOptions.bBoxPlotExMarkerMean)
		ptBoxPlotEx_MarkerMean->setOn(true);
	else
		ptBoxPlotEx_MarkerMean->setOn(false);
	// Median
	if(ReportOptions.bBoxPlotExMarkerMedian)
		ptBoxPlotEx_MarkerMedian->setOn(true);
	else
		ptBoxPlotEx_MarkerMedian->setOn(false);
	// Test Limits
	if(ReportOptions.bBoxPlotExMarkerLimits)
		ptBoxPlotEx_MarkerLimits->setOn(true);
	else
		ptBoxPlotEx_MarkerLimits->setOn(false);
	// Test Spec Limits
	if(ReportOptions.bBoxPlotExMarkerSpecLimits)
		ptBoxPlotEx_MarkerSpecLimits->setOn(true);
	else
		ptBoxPlotEx_MarkerSpecLimits->setOn(false);
	// 2Sigma
	if(ReportOptions.bBoxPlotExMarker2Sigma)
		ptBoxPlotEx_Marker2Sigma->setOn(true);
	else
		ptBoxPlotEx_Marker2Sigma->setOn(false);
	// 3Sigma
	if(ReportOptions.bBoxPlotExMarker3Sigma)
		ptBoxPlotEx_Marker3Sigma->setOn(true);
	else
		ptBoxPlotEx_Marker3Sigma->setOn(false);
	// 6Sigma
	if(ReportOptions.bBoxPlotExMarker6Sigma)
		ptBoxPlotEx_Marker6Sigma->setOn(true);
	else
		ptBoxPlotEx_Marker6Sigma->setOn(false);
	*/

	if(ReportOptions.GetOption("adv_boxplot_ex","orientation").toString()=="vertical")	//(ReportOptions.bBoxPlotExVertical == true)
		ptBoxplotEx_Intercative_V->setOn(true);
	else
		ptBoxplotEx_Intercative_H->setOn(true);
	
	// Production Yield/Volume
	{	// chart type
		QString strOptionStorageDevice;
		strOptionStorageDevice = (ReportOptions.GetOption("adv_production_yield","chart_type")).toString();

		if(strOptionStorageDevice == "yield_volume")
			ptProdYield_YieldVolumeChart->setOn(true);	// Yield/Volume production chart
		else if(strOptionStorageDevice == "yield")
			ptProdYield_YieldChart->setOn(true);		// 'Yield only' production chart
		else if (strOptionStorageDevice == "volume")
			ptProdYield_VolumeChart->setOn(true);		// 'Volume only' production chart
		else
            GEX_ASSERT_X(false,"refresh options","wrong 'adv_production_yield','chart_type' option");
	}
	/*
	if(ReportOptions.bProdYieldChart && ReportOptions.bProdVolumeChart)
		ptProdYield_YieldVolumeChart->setOn(true);	// Yield/Volume production chart
	else
	if(ReportOptions.bProdYieldChart)
		ptProdYield_YieldChart->setOn(true);		// 'Yield only' production chart
	else
		ptProdYield_VolumeChart->setOn(false);		// 'Volume only' production chart
		*/

	{	// markers to drawn on chart
		QString qstrVolumeMarkerOptions = (ReportOptions.GetOption("adv_production_yield", "marker")).toString();
		QStringList qstrlVolumeMarkerOptionList = qstrVolumeMarkerOptions.split(QString("|"));

		if(qstrlVolumeMarkerOptionList.contains(QString("title")))
			ptProdYield_MarkerTitle->setOn(true);		// Marker: Title
		else
			ptProdYield_MarkerTitle->setOn(false);

		if(qstrlVolumeMarkerOptionList.contains(QString("yield")))
			ptProdYield_MarkerYield->setOn(true);		// Marker: Yield
		else			
			ptProdYield_MarkerYield->setOn(false);

		if(qstrlVolumeMarkerOptionList.contains(QString("volume")))
			ptProdYield_MarkerVolume->setOn(true);		// Marker: Volume
		else
			ptProdYield_MarkerVolume->setOn(false);
	}
	/* CRO_cleanUp
	if(ReportOptions.bProdMarkerTitle)
		ptProdYield_MarkerTitle->setOn(true);		// Marker: Title
	else
		ptProdYield_MarkerTitle->setOn(false);
	if(ReportOptions.bProdMarkerYield)
		ptProdYield_MarkerYield->setOn(true);		// Marker: Yield
	else
		ptProdYield_MarkerYield->setOn(false);
	if(ReportOptions.bProdMarkerVolume)
		ptProdYield_MarkerVolume->setOn(true);		// Marker: Volume
	else
		ptProdYield_MarkerVolume->setOn(false);
	*/
	
	// boxplot
	/*	CRO_cleanUp
	if(ReportOptions.bBoxplotTestLimits)
		ptBoxPlot_TestLimits->setOn(true);		// Test Limits
	else
		ptBoxPlot_TestLimits->setOn(false);
	if(ReportOptions.bBoxplotMean)
		ptBoxPlot_Mean->setOn(true);			// Mean
	else
		ptBoxPlot_Mean->setOn(false);
	if(ReportOptions.bBoxplotSigma)
		ptBoxPlot_Sigma->setOn(true);			// Sigma
	else
		ptBoxPlot_Sigma->setOn(false);
	if(ReportOptions.bBoxplotMedian)
		ptBoxPlot_Median->setOn(true);			// Median
	else
		ptBoxPlot_Median->setOn(false);
	if(ReportOptions.bBoxplotCp)
		ptBoxPlot_Cp->setOn(true);				// Show Cp
	else
		ptBoxPlot_Cp->setOn(false);
	if(ReportOptions.bBoxplotCpk)
		ptBoxPlot_Cpk->setOn(true);				// Show Cpk
	else
		ptBoxPlot_Cpk->setOn(false);
	if(ReportOptions.bBoxplotRepeatability)
		ptBoxPlot_Repeatability->setOn(true);		// Show Dataset Repetability
	else
		ptBoxPlot_Repeatability->setOn(false);
	if(ReportOptions.bBoxplotRepeatabilityEV)
		ptBoxPlot_RepeatabilityEV->setOn(true);		// Show Equipment Variation EV
	else
		ptBoxPlot_RepeatabilityEV->setOn(false);
	if(ReportOptions.bBoxplotReproducibilityAV)
		ptBoxPlot_ReproducibilityAV->setOn(true);	// Show Appraiser Variation
	else
		ptBoxPlot_ReproducibilityAV->setOn(false);
	if(ReportOptions.bBoxplotR_and_R)
		ptBoxPlot_R_and_R->setOn(true);				// Show R&R
	else
		ptBoxPlot_R_and_R->setOn(false);
	if(ReportOptions.bBoxplotGB)
		ptBoxPlot_GB->setOn(true);				// Show GB (Guard Band)
	else
		ptBoxPlot_GB->setOn(false);
	if(ReportOptions.bBoxplotPV)
		ptBoxPlot_PV->setOn(true);					// Show Part Variation
	else
		ptBoxPlot_PV->setOn(false);
	if(ReportOptions.bBoxplotTV)
		ptBoxPlot_TV->setOn(true);					// Show Total Variation
	else
		ptBoxPlot_TV->setOn(false);
	if(ReportOptions.bBoxplotP_T)
		ptBoxPlot_P_T->setOn(true);					// Show P/T ratio
	else
		ptBoxPlot_P_T->setOn(false);
	*/
	

	// new reportOption
	{
		QString strOptionStorageDevice;
		strOptionStorageDevice = (ReportOptions.GetOption("adv_boxplot","chart_type")).toString();

		if ( strOptionStorageDevice == QString("limits"))
			ptBoxplot_ChartLimits->setOn(true);		// Charting: Over test limits
		else if (strOptionStorageDevice == QString("range"))
			ptBoxplot_ChartRange->setOn(true);		// Charting: Over data range
		else if (strOptionStorageDevice == QString("adaptive"))
			ptBoxplot_ChartAdaptive->setOn(true);		// Charting: Adaptive (over limits if one dataset, over range otherwise)


		/*
		switch(ReportOptions.iBoxplotCharting)
		{
			case GEX_BOXPLOTTYPE_LIMITS:
				ptBoxplot_ChartLimits->setOn(true);		// Charting: Over test limits
				break;
			case GEX_BOXPLOTTYPE_RANGE:
				ptBoxplot_ChartRange->setOn(true);		// Charting: Over data range
				break;
			case GEX_BOXPLOTTYPE_ADAPTIVE:
				ptBoxplot_ChartAdaptive->setOn(true);		// Charting: Adaptive (over limits if one dataset, over range otherwise)
				break;
		}
		*/
	}

	{
		QString strOptionStorageDevice = (ReportOptions.GetOption("adv_boxplot","sorting")).toString();

		if( strOptionStorageDevice == "test_number")
		{
			ptBoxplot_SortTestNumber->setOn(true);	// Sorting: test number
		}
		else if (strOptionStorageDevice == "test_name")
		{
			ptBoxplot_SortTestName->setOn(true);	// Sorting: test name
		}
		else if ( strOptionStorageDevice == "test_flow_id")
		{
			ptBoxplot_SortTestFlowID->setOn(true);	// Sorting: test flow ID (test execution order)
		}
		else if (strOptionStorageDevice == "mean")
		{
			ptBoxplot_SortMean->setOn(true);		// Sorting: Mean
		}
		else if (strOptionStorageDevice == "sigma")
		{
			ptBoxplot_SortSigma->setOn(true);		// Sorting: Sigma
		}
		else if (strOptionStorageDevice == "cp")
		{
			ptBoxplot_SortCp->setOn(true);			// Sorting: Cp
		}
		else if (strOptionStorageDevice == "cpk")
		{
			ptBoxplot_SortCpk->setOn(true);			// Sorting: Cpk
		}
		else if (strOptionStorageDevice == "r&r")
		{
			ptBoxplot_SortR_and_R->setOn(true);		// Sorting: R&R
		}
		else
		{
            GEX_ASSERT(false);
		}
	}
	/* CRO_cleanUp
	switch(ReportOptions.iBoxplotSorting)
	{
	case	GEX_SORTING_TESTNUMBER:
		ptBoxplot_SortTestNumber->setOn(true);	// Sorting: test number
		break;
	case	GEX_SORTING_TESTNAME:
		ptBoxplot_SortTestName->setOn(true);	// Sorting: test name
		break;
	case	GEX_SORTING_TESTFLOWID:
		ptBoxplot_SortTestFlowID->setOn(true);	// Sorting: test flow ID (test execution order)
		break;
	case	GEX_SORTING_MEAN:
		ptBoxplot_SortMean->setOn(true);		// Sorting: Mean
		break;
	case	GEX_SORTING_SIGMA:
		ptBoxplot_SortSigma->setOn(true);		// Sorting: Sigma
		break;
	case	GEX_SORTING_CP:
		ptBoxplot_SortCp->setOn(true);			// Sorting: Cp
		break;
	case	GEX_SORTING_CPK:
		ptBoxplot_SortCpk->setOn(true);			// Sorting: Cpk
		break;
	case	GEX_SORTING_R_R:
		ptBoxplot_SortR_and_R->setOn(true);		// Sorting: R&R
		break;
	}
	*/

	// R&R % computation option
	/*
	if(ReportOptions.bBoxplotShiftOverTV)
		ptBoxplot_ShiftOverTV->setOn(true);		// % shift over TV value
	else
		ptBoxplot_ShiftOverLimits->setOn(true);	// % shift over Limits space value (default)
	*/
		
	// Show R&R N*Sigma computation rule
	ptBoxplot_Nsigma->setOn(true);	// Cp cutoff limit defined
	sprintf(szString, "Customize N*Sigma space (default N value is 5.15). N=%g",
			ReportOptions.GetOption("adv_boxplot","r&r_sigma").toDouble() //ReportOptions.lfR_R_Nsigma
			);
	ptBoxplot_Nsigma->setText(0,szString);

	// Pareto
	/*		CRO_cleanUp
	bool ipsok=false;
	int ips=ReportOptions.GetOption("pareto","section").toInt(&ipsok);
	if ( (!ipsok) && (ReportOptions.GetOption("pareto","section").toString()=="enabled") )
		ips=GEX_OPTION_PARETO_ALL;

	if(ips & GEX_OPTION_PARETO_CP)	//if(ReportOptions.iParetoSection & GEX_OPTION_PARETO_CP)
		ptPareto_Cp->setOn(true);	// Display 'Cp pareto' section
	else
		ptPareto_Cp->setOn(false);
	if(ips & GEX_OPTION_PARETO_CPK)
		ptPareto_Cpk->setOn(true);	// Display 'Cpk pareto' section
	else
		ptPareto_Cpk->setOn(false);
	if(ips & GEX_OPTION_PARETO_FAIL)
		ptPareto_Fail->setOn(true);	// Display 'Failures pareto' section
	else
		ptPareto_Fail->setOn(false);
	if(ips & GEX_OPTION_PARETO_FAILSIG)
		ptPareto_FailSignature->setOn(true);	// Display 'Failures signature' section
	else
		ptPareto_FailSignature->setOn(false);
	if(ips & GEX_OPTION_PARETO_SBIN)
		ptPareto_Sbin->setOn(true);	// Display 'Soft bin pareto' section
	else
		ptPareto_Sbin->setOn(false);
	if(ips & GEX_OPTION_PARETO_HBIN)
		ptPareto_Hbin->setOn(true);	// Display 'Soft bin pareto' section
	else
		ptPareto_Hbin->setOn(false);
	*/


	/* CRO_cleanup
	if(ReportOptions.bParetoExcludePassBins)
		ptPareto_ExcludePassBin->setOn(true);	// Exclude PASS binnings in pareto
	else
		ptPareto_ExcludePassBin->setOn(false);
	if(ReportOptions.bParetoExcludeFailBins)
		ptPareto_ExcludeFailBin->setOn(true);	// Exclude FAIL binnings in pareto
	else
		ptPareto_ExcludeFailBin->setOn(false);
	*/
	
	if(ReportOptions.GetOption("pareto", "cutoff_cp").toDouble() >= 0)
	{
		ptPareto_CpCutOff->setOn(true);	// Cp cutoff limit defined
		sprintf(szString, "Only Cp <= to...: %.2g", ReportOptions.GetOption("pareto", "cutoff_cp").toDouble() );
		ptPareto_CpCutOff->setText(0,szString);
	}
	else
	{
		ptPareto_CpCutOff->setOn(false);// No Cp cutoof
		ptPareto_AllCp->setOn(true);
	}

	if(ReportOptions.GetOption("pareto","cutoff_cpk").toDouble()>=0)	//.lfParetoCpkCutoff >= 0)
	{
		ptPareto_CpkCutOff->setOn(true);	// Cpk cutoff limit defined
		sprintf(szString,"Only Cpk <= to...: %.2g", ReportOptions.GetOption("pareto","cutoff_cpk").toDouble());
		ptPareto_CpkCutOff->setText(0,szString);
	}
	else
	{
		ptPareto_CpkCutOff->setOn(false);// No Cpk cutoof
		ptPareto_AllCpk->setOn(true);
	}

	if (ReportOptions.GetOption("pareto", "cutoff_failure").toInt()>=0)	//iParetoFailCutoff >= 0)
	{
		ptPareto_FailureCutOff->setOn(true);	// Cpk cutoff limit defined
		sprintf(szString,"Total parameters to report (Top N): %d",
				ReportOptions.GetOption("pareto", "cutoff_failure").toInt());
		ptPareto_FailureCutOff->setText(0,szString);
	}
	else
		ptPareto_Failure_All->setOn(true);

	/* CRO_cleanup
	if(ReportOptions.lfParetoFailPatternCutoff >= 0)
	{
		ptPareto_SignatureFailureCutOff->setOn(true);	// Cpk cutoff limit defined
		sprintf(szString,"Total percentage of Signature failures to report: %.2g %%",ReportOptions.lfParetoFailPatternCutoff);
		ptPareto_SignatureFailureCutOff->setText(0,szString);
	}
	else
		ptPareto_SignatureFailure_All->setOn(true);
	*/
	
	// Binning	
	{
		QString strOptionStorageDevice;

		// 'binning' 'section'
		strOptionStorageDevice = (ReportOptions.GetOption("binning","section")).toString();

		if(strOptionStorageDevice=="disabled")
			ptBinning_NotInReport->setOn(true);				// DO NOT include section in report
		else
			ptBinning_InReport->setOn(true);				// Default: Include section in report
		/*
		if(ReportOptions.bBinningSection)
			ptBinning_InReport->setOn(true);				// Default: Include section in report
		else
			ptBinning_NotInReport->setOn(true);				// DO NOT include section in report
		*/

		// 'binning' 'computation'
		/*		CRO_cleanUp
		strOptionStorageDevice = (ReportOptions.GetOption("binning","computation")).toString();

		if(strOptionStorageDevice == "wafer_map")
			ptBinning_UseWafermapOnly->setOn(true);			// Compute Binning from Wafermap report ONY
		else if(strOptionStorageDevice == "samples")
			ptBinning_UseSamplesOnly->setOn(true);			// Compute Binning from Samples report ONY
		else if(strOptionStorageDevice == "summary")
			ptBinning_UseSummary->setOn(true);				// Compute Binning from Summary or wafermap
		else
            GEX_ASSERT(false);
			*/


		/*
		ptBinning_UseWafermapOnly->setOn(false);		// Compute Binning from Wafermap report ONY
		ptBinning_UseSamplesOnly->setOn(false);			// Compute Binning from Samples report ONY
		ptBinning_UseSummary->setOn(false);				// Compute Binning from Summary or wafermap
		if(ReportOptions.bBinningUseWafermapOnly)
			ptBinning_UseWafermapOnly->setOn(true);			// Compute Binning from Wafermap report ONY
		else
		if(ReportOptions.bBinningUseSamplesOnly)
			ptBinning_UseSamplesOnly->setOn(true);			// Compute Binning from Samples report ONY
		else
			ptBinning_UseSummary->setOn(true);				// Compute Binning from Summary or wafermap
		*/
	}
	
	// datalog
	/*		CRO_cleanUp
	if(ReportOptions.bDatalogTableComments)
		ptDatalog_Comment->setOn(true);				// Show comments
	else
		ptDatalog_Comment->setOn(false);
	if(ReportOptions.bDatalogTableTestNumber)
		ptDatalog_TestNumber->setOn(true);			// Show test number
	else
		ptDatalog_TestNumber->setOn(false);
	if(ReportOptions.bDatalogTableTestName)
		ptDatalog_TestName->setOn(true);			// Show test name
	else
		ptDatalog_TestName->setOn(false);
	if(ReportOptions.bDatalogTableLimits)
		ptDatalog_Limits->setOn(true);				// Show test limits
	else
		ptDatalog_Limits->setOn(false);
	if(ReportOptions.bDatalogTableDieXY)
		ptDatalog_DieLoc->setOn(true);				// Show die XY location
	else
		ptDatalog_DieLoc->setOn(false);
	*/

	if(ReportOptions.GetOption("adv_datalog","format").toString()=="1row")	//(ReportOptions.bDatalogSingleRow)
	{
		ptDatalog_1Row->setOn(true);				// Show die XY location
		ptDatalog_2Rows->setOn(false);				// Show die XY location
	}
	else
	{
		ptDatalog_1Row->setOn(false);
		ptDatalog_2Rows->setOn(true);
	}
	
	/* CRO_cleanUp by HTH case# 4236
	// What-if Binning options
	sprintf(szString,"What-if Passing Binning: %d",ReportOptions.iWhatIf_PassBin);
	ptWhatif_Pass_Binning->setText(0,szString);
	sprintf(szString,"What-if Failing Binning: %d",ReportOptions.iWhatIf_FailBin);
	ptWhatif_Fail_Binning->setText(0,szString);
	
	// What-if fail bin is of type 'Pass'.
	ptWhatif_Fail_BinningIsPass->setOn(ReportOptions.bWhatIf_FailBinIsPass);
	*/

	/* CRO_cleanUp by HTH case# 4237
	// Pearson's correlation report
	switch(ReportOptions.iSortingPearson)
	{
	case GEX_PEARSON_SORTING_TESTNAME:
		ptPearson_SortTestName->setOn(true);
		break;
	case GEX_PEARSON_SORTING_RATIO:
		ptPearson_SortRatio->setOn(true);
		break;
	}
	sprintf(szString,"Ignore tests with a Pearson's correlation ratio (r^2) lower than: %.2g",ReportOptions.lfPearsonCutoff);
	ptPearson_ThresholdRatio->setText(0,szString);
	*/

	/* CRO_cleanUp by HTH case# 4238
	// CSV Generation test ordering
	if(ReportOptions.bToolboxCsvSortTestFlow)
		ptToolbox_CsvSortTestFlowID->setOn(true);	// Sorting: By test flowID
	else
		ptToolbox_CsvSortTestID->setOn(true);		// Sorting: By Test#

	// Split CSV file into 250-rows files?
	if(ReportOptions.bToolboxCsvSplitExport)
		ptToolbox_CsvSplitExport->setOn(true);	// Split CSV file into 250-rows CSV files.
	else
		ptToolbox_CsvSplitExport->setOn(false);
	
	// Normalize units in csv export
	switch (ReportOptions.nToolboxCsvUnitsMode)
	{
	case GEX_OPTIONS_TOOLBOX_UNITS_NORMALIZED		:
	default											:	ptToolbox_csvUnitsModeNormalized->setOn(true);
	break;

	case GEX_OPTIONS_TOOLBOX_UNITS_SCALING_FACTOR	:	ptToolbox_csvUnitsModeScalingFactor->setOn(true);
		break;
	}
	*/

	////////////////////////////////////////////////////////
	// Application Preferences
	////////////////////////////////////////////////////////

	{
		QString strOptionStorageDevice;
		strOptionStorageDevice = (ReportOptions.GetOption("global_info","detail_level")).toString();

		if (strOptionStorageDevice == QString("summarized"))
			ptGlobalInfoSummarized->setOn(true);
		else		// strOptionStorageDevice == QString("detailed")
			ptGlobalInfoDetailed->setOn(true);

		/* switch(ReportOptions.iGlobalInfoDetailLevel)
		{
			case GEX_OPTION_GLOBAL_INFO_SUMMARIZED:
				ptGlobalInfoSummarized->setOn(true);
				break;
			case GEX_OPTION_GLOBAL_INFO_DETAILED:
				ptGlobalInfoDetailed->setOn(true);
				break;
		}
		*/
	}


	////////////////////////////////////////////////////////
	// Application Preferences
	////////////////////////////////////////////////////////

	// Custom Text Editor
	UpdateMenuCustomTextItem(ptPreferences_CustomTextEditor,"Current editor: ",
							 ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data() //ReportOptions.strTextEditor.toLatin1().constData()
							 );
	
	// Custom Spreadsheet Editor
	UpdateMenuCustomTextItem(ptPreferences_CustomSpreadsheetEditor,"Current editor: ",
							 ReportOptions.GetOption("preferences", "ssheet_editor").toString().toLatin1().data() //ReportOptions.m_PrefMap["ssheet_editor"].toLatin1().data()
							 );


	// Auto-Close of the application if Idle too long
	switch(ReportOptions.GetTimeBeforeAutoClose()) //switch(ReportOptions.lAutoClose)
	{
	case 15*60:		// 15 minutes
		ptPreferences_AutoClose15minutes->setOn(true);
		break;
	case 30*60:		// 30 minutes
		ptPreferences_AutoClose30minutes->setOn(true);
		break;
	case 60*60:		// 1 hour
		ptPreferences_AutoClose1hour->setOn(true);
		break;
	case 2*60*60:	// 2 hours
		ptPreferences_AutoClose2hours->setOn(true);
		break;
	case 4*60*60:	// 4 hours
		ptPreferences_AutoClose4hours->setOn(true);
		break;
	case 0:			// Never
	default:
		ptPreferences_AutoCloseNever->setOn(true);
		break;
	}
	// This menu item is ONLY for the Floating license mode!
	if(ReportOptions.lLicenseRunningMode == GEX_RUNNINGMODE_CLIENT)
		ptNodeAutoClose->setVisible(true);
	else
		ptNodeAutoClose->setVisible(true);

	// Change cursor back to normal
    QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////
// Load options from user profile file
///////////////////////////////////////////////////////////
void GexOptions::OnLoadUserOptions(void)
{
	GSLOG(SYSLOG_SEV_INFORMATIONAL, "");

	PickUserDialog cPickUser(this);

	// Set GUI in 'Load' mode
	cPickUser.setLoadMode(true);

	if(cPickUser.exec() != 1)
		return;

	// Get username selected
	cPickUser.getSelectedUserName(pGexMainWindow->strUserName,pGexMainWindow->strStartupScript);

	// Load options: run startup script : $HOME/.<profile>.csl
	pGexMainWindow->pWizardScripting->RunStartupScript(pGexMainWindow->strStartupScript);
}

///////////////////////////////////////////////////////////
// Save options to user profile file
///////////////////////////////////////////////////////////
void GexOptions::OnSaveUserOptions(void)
{
	PickUserDialog cPickUser(this);

	// Set GUI in 'Save as...' mode
	cPickUser.setLoadMode(false);

	// Display popup
	if(cPickUser.exec() != 1)
		return;

	// Get username selected
	cPickUser.getSelectedUserName(pGexMainWindow->strUserName,pGexMainWindow->strStartupScript);

	// Save options
	pGexMainWindow->OnSaveEnvironment();

	// Force reload so all changes are effective
	pGexMainWindow->pWizardScripting->RunStartupScript(pGexMainWindow->strStartupScript);

	// Refresh GUI
	//RefreshOptionsEdits();
	RefreshOptionsCenter();
}

///////////////////////////////////////////////////////////
// Save options to user profile file
///////////////////////////////////////////////////////////
void GexOptions::OnDefaultOptions(void)
{
	// Confirm reset of all options to 'default' settings...
    bool lOk;
    GS::Gex::Message::request("", "Confirm to reset all options to default ?", lOk);
    if (! lOk)
    {
        return;
    }

	// Reset options to default
	ReportOptions.Reset(true);

	// Update GUI based on reset options
	//RefreshOptionsEdits();
	RefreshOptionsCenter();
}

///////////////////////////////////////////////////////////
// Writes 'SetOptions' section in script file
// Note: if hFile = NULL, do not write to file, only update
// options variables to reflect GUI selections!
///////////////////////////////////////////////////////////
void GexOptions::WriteOptionsSection(FILE *hFile)
{
//	QString strValue;	// Holds field value to setup
//	int		iIndex;		// Used in String index manipulation

	// Writes 'Favorite Scripts' section
	if(hFile != NULL)
	{
		fprintf(hFile,"//////////////////////////////////////////\n");
		fprintf(hFile,"// Setup the GEX 'Options' section\n");
		fprintf(hFile,"//////////////////////////////////////////\n");
		fprintf(hFile,"SetOptions()\n");
		fprintf(hFile,"{\n");
		fprintf(hFile,"\n");
		fprintf(hFile,"  // Sets all data analysis global options\n");
		//fprintf(hFile,"  gexOptions('reset','all','clear');\n");	// csl v1
		fprintf(hFile,"  gexResetOptions('clear');\n\n");	// csl v2
	}

	// Databases paths
	/*
	strValue = ptDatabase_LocalPath->text(0);
	strValue = strValue.section(':',1);
        strValue = strValue.trimmed();
        strValue = strValue.replace( "\\", "/");
	strValue = QDir::cleanPath(strValue);
	{
		bool bIsDefinePath = !( (strValue.contains(QString("(default)"))) || (strValue.isEmpty()) );
		if(bIsDefinePath)
			strValue += QString("/");		// Paths have to finish by '/' character (galaxy stantard ?)
	}
	// Convert file path to be script compatible '\' becomes '\\'
	ConvertToScriptString(strValue);
	ReportOptions.SetOption("databases","local_path", strValue);
	*/


	/*
	if(hFile != NULL)
		fprintf(hFile,"  gexOptions('databases','local_path','%s');\n",strValue.toLatin1().constData());
	*/

	/*
	strValue = ptDatabase_ServerPath->text(0);
	strValue = strValue.section(':',1);
        strValue = strValue.trimmed();
        strValue = strValue.replace( "\\", "/");
	strValue = QDir::cleanPath(strValue);
	{
		bool bIsDefinePath = !( (strValue.contains(QString("(default)"))) || (strValue.isEmpty()) );
		if(bIsDefinePath)
			strValue += QString("/");		// Paths have to finish by '/' character (galaxy stantard ?)
	}
	// Convert file path to be script compatible '\' becomes '\\'
	ConvertToScriptString(strValue);
	ReportOptions.SetOption("databases","server_path", strValue);
	*/

	/*
	if(hFile != NULL)
		fprintf(hFile,"  gexOptions('databases','server_path','%s');\n",strValue.toLatin1().constData());
	*/

	// Other database options
	/*
	if((hFile != NULL) && (ptDatabase_RDB_DefaultParameters_All->isOn()))		// By default, extract all parameters for RDB dataset
		fprintf(hFile,"  gexOptions('databases','rdb_default_parameters','all');\n");
	if((hFile != NULL) && (ptDatabase_RDB_DefaultParameters_None->isOn()))		// By default, extract no parameter for RDB dataset
		fprintf(hFile,"  gexOptions('databases','rdb_default_parameters','none');\n");
	*/
	/*
	if (ptDatabase_RDB_DefaultParameters_All->isOn())
		ReportOptions.SetOption("databases","rdb_default_parameters","all");
	else
		ReportOptions.SetOption("databases","rdb_default_parameters","none");
	*/

	// Monitoring
	if(ReportOptions.lProductID == GEX_DATATYPE_GEX_MONITORING ||
	   ReportOptions.lProductID == GEX_DATATYPE_GEX_PATMAN)
	{
//		long	lHome=0,lProduct=0,lTester=0;

		/*		CRO_cleanUp
		// History size (number of days to keep the HTML status pages.
		strValue = "1week";
		if(ptMonitoring_History_none->isOn())	// no history
			strValue = "none";
		if(ptMonitoring_History_1week->isOn())	// 1 week
			strValue = "1week";
		if(ptMonitoring_History_2week->isOn())	// 2 weeks
			strValue = "2week";
		if(ptMonitoring_History_3week->isOn())	// 3 weeks
			strValue = "3week";
		if(ptMonitoring_History_1month->isOn())	// 1 month
			strValue = "1month";
		if(ptMonitoring_History_2month->isOn())	// 2 months
			strValue = "2month";
		if(ptMonitoring_History_3month->isOn())	// 3 months
			strValue = "3month";
		if(ptMonitoring_History_4month->isOn())	// 4 months
			strValue = "4month";
		if(ptMonitoring_History_5month->isOn())	// 5 months
			strValue = "5month";
		if(ptMonitoring_History_6month->isOn())	// 6 months
			strValue = "6month";
		if(ptMonitoring_History_1year->isOn())	// 1 year
			strValue = "1year";
		if(ptMonitoring_History_all->isOn())	// Keep all pages
			strValue = "all";
		if(hFile != NULL)
		{
			fprintf(hFile,"  gexOptions('monitoring','history','%s');\n",strValue.toLatin1().constData());
		}
		*/

		// HOME page
		/*		CRO_cleanUp
		if(
->isOn())
			lProduct |= GEXMO_OPTION_HOME_PRODUCT;
		if(ptMonitoring_Home_Testers->isOn())
			lProduct |= GEXMO_OPTION_HOME_TESTER;
		if(hFile != NULL)
			fprintf(hFile,"  gexOptions('monitoring','home_page','%ld');\n",lHome);
		*/
		
		// PRODUCTS page
		/*		CRO_cleanUp
		if(ptMonitoring_Products_Parts->isOn())
			lProduct |= GEXMO_OPTION_PRODUCTS_PARTS;
		if(ptMonitoring_Products_Good->isOn())
			lProduct |= GEXMO_OPTION_PRODUCTS_GOOD;
		if(ptMonitoring_Products_Fail->isOn())
			lProduct |= GEXMO_OPTION_PRODUCTS_FAIL;
		if(ptMonitoring_Products_Yield->isOn())
			lProduct |= GEXMO_OPTION_PRODUCTS_YIELD;
		if(ptMonitoring_Products_Alarm->isOn())
			lProduct |= GEXMO_OPTION_PRODUCTS_ALARM;
		if(ptMonitoring_Products_Chart->isOn())
			lProduct |= GEXMO_OPTION_PRODUCTS_CHART;
		if(hFile != NULL)
			fprintf(hFile,"  gexOptions('monitoring','product_page','%ld');\n",lProduct);
		*/
		
		// TESTERS page
		/*		CRO_cleanUp
		if(ptMonitoring_Testers_Products->isOn())
			lTester |= GEXMO_OPTION_TESTERS_PRODUCT;
		if(ptMonitoring_Testers_Operator->isOn())
			lTester |= GEXMO_OPTION_TESTERS_OPERATOR;
		if(ptMonitoring_Testers_Program->isOn())
			lTester |= GEXMO_OPTION_TESTERS_PROGRAM;
		if(ptMonitoring_Testers_Parts->isOn())
			lTester |= GEXMO_OPTION_TESTERS_PARTS;
		if(ptMonitoring_Testers_Good->isOn())
			lTester |= GEXMO_OPTION_TESTERS_GOOD;
		if(ptMonitoring_Testers_Fail->isOn())
			lTester |= GEXMO_OPTION_TESTERS_FAIL;
		if(ptMonitoring_Testers_Yield->isOn())
			lTester |= GEXMO_OPTION_TESTERS_YIELD;
		if(ptMonitoring_Testers_Alarm->isOn())
			lTester |= GEXMO_OPTION_TESTERS_ALARM;
		if(ptMonitoring_Testers_Chart->isOn())
			lTester |= GEXMO_OPTION_TESTERS_CHART;
		if(hFile != NULL)
			fprintf(hFile,"  gexOptions('monitoring','tester_page','%ld');\n",lTester);
		*/
	} // GEX MONITORING


	// Output sections : output_type,.......
	/*
	if (ptEmbedFontsInPdf->isOn())
		ReportOptions.SetOption("output", "embed_fonts_in_pdf","true");
	else
		ReportOptions.SetOption("output", "embed_fonts_in_pdf","false");
	*/

	// Output Type: HTML report, Word report, etc...
	/*
	if(ptOutput_Html->isOn())
		strValue = "html";
	else
		if(ptOutput_Word->isOn())
			strValue = "word";
	else
		if(ptOutput_CSV->isOn())
			strValue = "csv";
	else
		if(ptOutput_Ppt->isOn())
			strValue = "ppt";
	else
		if(ptOutput_Pdf->isOn())
			strValue = "pdf";
	else
		if(ptOutput_Interactive->isOn())
			strValue = "interactive";
	//ReportOptions.SetOption("output","format", strValue);
	if(hFile != NULL)
		fprintf(hFile,"  gexOptions('output','format','%s');\n",strValue.toLatin1().constData());
	*/

	// Truncate test names
	/*
	if(ptOutputTruncateTestNames->isOn())
	{
		strValue = ptOutputTruncateTestNames->text(0);
		strValue = strValue.section(':',1);	// Extract text to the right of 'Truncate test names to length: '
	}
	else
		strValue = "no";
	ReportOptions.SetOption("output", "truncate_names",strValue);
	//if(hFile != NULL)
	//	fprintf(hFile,"  gexOptions('output','truncate_names','%s');\n",strValue.toLatin1().constData());
	*/

	// Paper size
	/*if(ptOutputPaperSize_Letter->isOn())
		strValue = "letter";
	else
		if(ptOutputPaperSize_A4->isOn())
			strValue = "A4";
	ReportOptions.SetOption("output", "paper_size", strValue);
	*/
	/*
	if(hFile != NULL)
	{
		fprintf(hFile,"  gexOptions('output','paper_size','%s');\n", strValue.toLatin1().constData());
		if(ptOutputPaperFormat_Portrait->isOn())
			strValue = "portrait";
		else
			strValue = "landscape";
		fprintf(hFile,"  gexOptions('output','paper_format','%s');\n",strValue.toLatin1().constData());
	}
	*/
	QString strString;

	// Clean Image path string to be script-compliant
	/*strString =	ReportOptions.GetOption("output", "front_page_image").toString();	//ReportOptions.strFrontPageImage;
	strString.replace("\\","\\\\");
	ReportOptions.SetOption("output", "front_page_image", strString);*/
	/*
	if(strString.isEmpty() == false)
		fprintf(hFile,"  gexOptions('output','front_page_image','%s');\n",strString.toLatin1().constData());
	*/


	// Output: Location
	/*
	if(ptOutput_PathCustom->isOn())
	{
		// Retrieve path
		strValue = ptOutput_PathCustom->text(0);
		// extract path from string 'location: XXXX'
		strValue = strValue.mid(10);
                strValue = strValue.trimmed();
                strValue = strValue.replace( "\\", "/");
		strValue = QDir::cleanPath(strValue);
		{
			bool bIsDefinePath = !( (strValue.contains(QString("(default)"))) || (strValue.isEmpty()) );
			if(bIsDefinePath)
				strValue += QString("/");		// Paths have to finish by '/' character (galaxy stantard)
		}
		// Convert file path to be script compatible '\' becomes '\\'
		ConvertToScriptString(strValue);
	}
	else
		strValue = "";	// Empty path means: we'll use first Data path to create the report!
	ReportOptions.SetOption("output", "location", strValue);
	*/

	// Clean up "front_page_text"
	// Frontpage Text
	strString =	ReportOptions.GetOption("output", "front_page_text").toString();	//ReportOptions.strFrontPageText;
	// Clean text string to be script-compliant
	if(strString.startsWith("<html><head><meta name") == true)
		strString.replace("\n"," ");	// HTML code, ignore LF
	else
		strString.replace("\n","<br>");	// Not a HTML code, then make it HTML!
	strString.replace("\\","\\\\");
	strString.replace("\"","\\\"");
	strString.replace("'","\\'");
	// Remove empty paragraphs
	strString.replace("<p> </p>","");
	ReportOptions.SetOption("output", "front_page_text", strString);
	/*
		if(strString.isEmpty() == false)
			fprintf(hFile,"  gexOptions('output','front_page_text','%s');\n", strString.toLatin1().constData());
		*/

	// Pareto sections to list
	/*
	iIndex = 0;
	if(ptPareto_Cp->isOn())						// Include Cp pareto section in report
		iIndex |= GEX_OPTION_PARETO_CP;
	if(ptPareto_Cpk->isOn())					// Include Cpk pareto section in report
		iIndex |= GEX_OPTION_PARETO_CPK;
	if(ptPareto_Fail->isOn())					// Include Failures pareto section in report
		iIndex |= GEX_OPTION_PARETO_FAIL;
	if(ptPareto_FailSignature->isOn())			// Include Failures singatures pareto section in report
		iIndex |= GEX_OPTION_PARETO_FAILSIG;
	if(ptPareto_Sbin->isOn())					// Include Soft-Bin pareto section in report
		iIndex |= GEX_OPTION_PARETO_SBIN;
	if(ptPareto_Hbin->isOn())					// Include Hard-Bin pareto section in report
		iIndex |= GEX_OPTION_PARETO_HBIN;
	ReportOptions.SetOption("pareto","section", QString("%1").arg(iIndex) );
	//fprintf(hFile,"  gexOptions('pareto','section','%d');\n",iIndex);
	*/

	if(hFile != NULL)
	{
		fprintf(hFile,"  gexCslVersion('%1.2f');\n", GEX_MAX_CSL_VERSION);	// ReportOptions.WriteOptionsToFile options are latest version of the csl format.
		ReportOptions.WriteOptionsToFile(hFile);
		fprintf(hFile,"\n  gexCslVersion('1.0');\n");					// The following options are csl v1 and will be parsed with old function gexOptions_v1
	}


	if(hFile != NULL)
	{

		// Merge/no merge same tests (depending if test name is different)
		/* CRO_cleanUp by HTH case# 4188
		if(ptDuplicate_Merge->isOn())			// Merge identical test numbers with same name.
			fprintf(hFile,"  gexOptions('duplicate_test','merge','');\n");
		if(ptDuplicate_MergeName->isOn())		// Merge test with same name (even if different test number)
			fprintf(hFile,"  gexOptions('duplicate_test','merge_name','');\n");
		if(ptDuplicate_NoMerge->isOn())			// No merge same test numbers if test name is different
			fprintf(hFile,"  gexOptions('duplicate_test','no_merge','');\n");
		*/

		// Smart scaling on test results?
		/*
		if(ptScaling_Smart->isOn())	// Smart scaling enabled.
			fprintf(hFile,"  gexOptions('scaling','smart','');\n");
		if(ptScaling_None->isOn())	// no scaling
			fprintf(hFile,"  gexOptions('scaling','none','');\n");
		if(ptScaling_Normalized->isOn())	// normalize.
			fprintf(hFile,"  gexOptions('scaling','normalized','');\n");
		if(ptScaling_ToLimits->isOn())	// Rescale to limits units.
			fprintf(hFile,"  gexOptions('scaling','to_limits','');\n");
		*/

		// Show/Hide PartID
		/* CRO_cleanUp by HTH case# 4189
		if(ptPartID_Show->isOn())			// Show part ID
			fprintf(hFile,"  gexOptions('part_id','show','');\n");
		else
			fprintf(hFile,"  gexOptions('part_id','hide','');\n");
		*/

		// Sorting mode
		/* CRO_cleanUp
		if(ptSort_SortNone->isOn())			// no Sorting on data input.
			fprintf(hFile,"  gexOptions('sorting','none','');\n");
		if(ptSort_SortByDate->isOn())			// Sorting test data by date.
			fprintf(hFile,"  gexOptions('sorting','date','');\n");

		// Fail count mode
		if(ptFailures_All->isOn())				// Failure count mode: ALL failures
			fprintf(hFile,"  gexOptions('fail_count','all','');\n");
		if(ptFailures_First->isOn())			// Failure count mode: 1st failure in flow
			fprintf(hFile,"  gexOptions('fail_count','first','');\n");
		*/

		// Mutli-parametric tests...
		/*
		if(ptMultiParametric_Merge->isOn())		// Merge all pins of Multi-parametric tests under same test name
			fprintf(hFile,"  gexOptions('multi_parametric','merge','');\n");
		if(ptMultiParametric_NoMerge->isOn())	// No merge
			fprintf(hFile,"  gexOptions('multi_parametric','no_merge','');\n");
		*/

		// Multi-parametric merge criteria
		/*
		if (ptMultiParametric_Criteria_First->isOn())
			fprintf(hFile, "  gexOptions('multi_parametric','criteria','first');\n");
		if (ptMultiParametric_Criteria_Last->isOn())
			fprintf(hFile, "  gexOptions('multi_parametric','criteria','last');\n");
		if (ptMultiParametric_Criteria_Min->isOn())
			fprintf(hFile, "  gexOptions('multi_parametric','criteria','min');\n");
		if (ptMultiParametric_Criteria_Max->isOn())
			fprintf(hFile, "  gexOptions('multi_parametric','criteria','max');\n");
		if (ptMultiParametric_Criteria_Mean->isOn())
			fprintf(hFile, "  gexOptions('multi_parametric','criteria','mean');\n");
		if (ptMultiParametric_Criteria_Median->isOn())
			fprintf(hFile, "  gexOptions('multi_parametric','criteria','median');\n");
		*/

		// Multi-parametric merge option
		/*
		if (ptMultiParametric_Options_KeepOne->isOn())
			fprintf(hFile, "  gexOptions('multi_parametric','option','keepOne');\n");
		else
			fprintf(hFile, "  gexOptions('multi_parametric','option','keep_all');\n");
		*/

		// Handling STDF compliancy issues.
		/*
		if(ptStdfCompliancy_Stringent->isOn())		// Stringent compliancy...
			fprintf(hFile,"  gexOptions('stdf_compliancy','stringent','');\n");
		if(ptStdfCompliancy_Relaxed->isOn())	// No merge
			fprintf(hFile,"  gexOptions('stdf_compliancy','flexible','');\n");
		*/

		/* CRO_cleanUp by HTH case# 4193
		// Intermediate folder for STDF & zip files.
		if(ptTempStdfFolder_Custom->isOn())
		{
			// Retrieve path
			strValue = ptTempStdfFolder_Custom->text(0);
			strValue = strValue.section(' ',1);	// Extract text to the right of 'Location: '
			// Convert file path to be script compatible '\' becomes '\\'
			ConvertToScriptString(strValue);
		}
		else
			strValue = "";	// Empty path means: we'll use first Data path to create the report!
		fprintf(hFile,"  gexOptions('stdf_intermediate','%s','');\n",strValue.toLatin1().constData());
		*/
	}




	// Outlier
	/*
	QString strSetValue="";
	strValue = "none";
	if(ptOutlier_100p->isOn())	// 100%
		strValue = "100%";
	if(ptOutlier_150p->isOn())	// 150%
		strValue = "150%";
	if(ptOutlier_200p->isOn())	// 200%
		strValue = "200%";
	if(ptOutlier_250p->isOn())	// 250%
		strValue = "250%";
	if(ptOutlier_300p->isOn())	// 300%
		strValue = "300%";
	if(ptOutlier_sigma->isOn())	// N*sigma space centered on mean
	{
		strValue = "n_sigma";
		strSetValue = ptOutlier_sigma->text(0).section("With N=",1);	// Take value to the right of 'With N='
	}
	if(ptOutlier_IQR->isOn())	// Limits are based on Q1,Q3 +/-1.5*IQR
	{
		strValue = "n_iqr";
		strSetValue = ptOutlier_IQR->text(0).section("With N=",1);	// Take value to the right of 'With N='
	}
	if(ptOutlier_InlinerSigma->isOn())	// N*sigma space centered on mean
	{
		strValue = "exclude_n_sigma";
		strSetValue = ptOutlier_InlinerSigma->text(0).section("With N=",1);	// Take value to the right of 'With N='
	}
	*/

	// Speed optimization: compute Advanced Statistics
	/* CRO_cleanUp
	QString strSpeedAdvStats;
	if(ptSpeedAdvStats_Always->isOn())	// Always.
		strSpeedAdvStats = "always";
	if(ptSpeedAdvStats_50MB->isOn())	// Compute Adv. stats. if 50MB of data or less.
		strSpeedAdvStats = "50mb";
	if(ptSpeedAdvStats_100MB->isOn())	// Compute Adv. stats. if 100MB of data or less.
		strSpeedAdvStats = "100mb";
	if(ptSpeedAdvStats_200MB->isOn())	// Compute Adv. stats. if 200MB of data or less.
		strSpeedAdvStats = "200mb";
	if(ptSpeedAdvStats_300MB->isOn())	// Compute Adv. stats. if 300MB of data or less.
		strSpeedAdvStats = "300mb";
	if(ptSpeedAdvStats_400MB->isOn())	// Compute Adv. stats. if 400MB of data or less.
		strSpeedAdvStats = "400mb";
	if(ptSpeedAdvStats_500MB->isOn())	// Compute Adv. stats. if 500MB of data or less.
		strSpeedAdvStats = "500mb";
	if(ptSpeedAdvStats_Never->isOn())	// never.
		strSpeedAdvStats = "never";

	// Speed optimization: using Database SUMMARY records instead of data samples
	QString strSpeedAdvSummaryDB;
	if(ptSpeedAdvSummaryDB_Always->isOn())	// Always use summary
		strSpeedAdvSummaryDB = "always";
	if(ptSpeedAdvSummaryDB_50MB->isOn())	// use summary if 50MB of data or more
		strSpeedAdvSummaryDB = "50mb";
	if(ptSpeedAdvSummaryDB_100MB->isOn())	// use summary if 100MB of data or more
		strSpeedAdvSummaryDB = "100mb";
	if(ptSpeedAdvSummaryDB_200MB->isOn())	// use summary if 200MB of data or more
		strSpeedAdvSummaryDB = "200mb";
	if(ptSpeedAdvSummaryDB_300MB->isOn())	// use summary if 300MB of data or more
		strSpeedAdvSummaryDB = "300mb";
	if(ptSpeedAdvSummaryDB_400MB->isOn())	// use summary if 400MB of data or more
		strSpeedAdvSummaryDB = "400mb";
	if(ptSpeedAdvSummaryDB_500MB->isOn())	// use summary if 500MB of data or more
		strSpeedAdvSummaryDB = "500mb";
	if(ptSpeedAdvSummaryDB_Never->isOn())	// never use summary, always data samples
		strSpeedAdvSummaryDB = "never";
		*/


	// Statistics: fields to show + update internal fields
	/* CRO_cleanUp
	ReportOptions.bStatsTableTestName = ptStatistics_TestName->isOn();
	ReportOptions.bStatsTableTestType = ptStatistics_TestType->isOn();
	ReportOptions.bStatsTableTestLimits = ptStatistics_TestLimits->isOn();
	ReportOptions.bStatsTableSpecLimits = ptStatistics_SpecLimits->isOn();
	ReportOptions.bStatsSource = ptStatistics_StatsSource->isOn();
	ReportOptions.bStatsTableExec = ptStatistics_ExecCount->isOn();
	ReportOptions.bStatsTableFail = ptStatistics_FailCount->isOn();
	ReportOptions.bStatsTableFailPercent = ptStatistics_Failp->isOn();
	ReportOptions.bStatsTableFailBin = ptStatistics_FailBin->isOn();
	ReportOptions.bStatsTableOutlier = ptStatistics_Outlier->isOn();
	ReportOptions.bStatsTableMean = ptStatistics_Mean->isOn();
	ReportOptions.bStatsTableMeanShift = ptStatistics_MeanShift->isOn();
	ReportOptions.bStatsTableT_Test = ptStatistics_T_Test->isOn();
	ReportOptions.bStatsTableSigma = ptStatistics_Sigma->isOn();
	ReportOptions.bStatsTableSigmaShift = ptStatistics_SigmaShift->isOn();
	ReportOptions.bStatsTable2Sigma = ptStatistics_2Sigma->isOn();
	ReportOptions.bStatsTable3Sigma = ptStatistics_3Sigma->isOn();
	ReportOptions.bStatsTable6Sigma = ptStatistics_6Sigma->isOn();
	ReportOptions.bStatsTableMinVal = ptStatistics_Min->isOn();
	ReportOptions.bStatsTableMaxVal = ptStatistics_Max->isOn();
	ReportOptions.bStatsTableRange = ptStatistics_Range->isOn();
	ReportOptions.bStatsTableMaxRange = ptStatistics_MaxRange->isOn();
	ReportOptions.bStatsTableCp = ptStatistics_Cp->isOn();
	ReportOptions.bStatsTableCpShift = ptStatistics_CpShift->isOn();
	ReportOptions.bStatsTableCpk = ptStatistics_Cpk->isOn();
	ReportOptions.bStatsTableCpkLow = ptStatistics_CpkLow->isOn();
	ReportOptions.bStatsTableCpkHigh = ptStatistics_CpkHigh->isOn();
	ReportOptions.bStatsTableCpkShift = ptStatistics_CpkShift->isOn();
	ReportOptions.bStatsTableYield = ptStatistics_Yield->isOn();
	// Gage R&R results
	ReportOptions.bStatsTableGageRepeatabilityEV= ptStatistics_GageEV->isOn();
	ReportOptions.bStatsTableGageReproducibilityAV= ptStatistics_GageAV->isOn();
	ReportOptions.bStatsTableGageR_and_R= ptStatistics_GageR_and_R->isOn();
	ReportOptions.bStatsTableGageGB= ptStatistics_GageGB->isOn();
	ReportOptions.bStatsTableGagePV= ptStatistics_GagePV->isOn();
	ReportOptions.bStatsTableGageTV= ptStatistics_GageTV->isOn();
	ReportOptions.bStatsTableGageP_T= ptStatistics_GageP_T->isOn();
	// Advanced Statistics
	ReportOptions.bStatsTableSkew = ptStatistics_Skew->isOn();
	ReportOptions.bStatsTableShape = ptStatistics_Shape->isOn();
	ReportOptions.bStatsTableTestFlowID = ptStatistics_TestFlowID->isOn();
	ReportOptions.bStatsTableKurtosis = ptStatistics_Kurtosis->isOn();
	ReportOptions.bStatsTableP0_5= ptStatistics_P0_5->isOn();
	ReportOptions.bStatsTableP2_5= ptStatistics_P2_5->isOn();
	ReportOptions.bStatsTableP10= ptStatistics_P10->isOn();
	ReportOptions.bStatsTableQuartile1 = ptStatistics_Quartile1->isOn();
	ReportOptions.bStatsTableQuartile2 = ptStatistics_Quartile2->isOn();
	ReportOptions.bStatsTableQuartile3 = ptStatistics_Quartile3->isOn();
	ReportOptions.bStatsTableP90= ptStatistics_P90->isOn();
	ReportOptions.bStatsTableP97_5= ptStatistics_P97_5->isOn();
	ReportOptions.bStatsTableP99_5= ptStatistics_P99_5->isOn();
	ReportOptions.bStatsTableInterquartiles = ptStatistics_Interquartiles->isOn();
	ReportOptions.bStatsTableSigmaInterQuartiles = ptStatistics_SigmaInterQuartiles->isOn();
	*/

	if(hFile != NULL)
	{
		// CRO_cleanUp
		//fprintf(hFile,"  gexOptions('outlier','removal','%s','%s');\n",strValue.toLatin1().constData(), strSetValue.toLatin1().constData());

		/* CRO_cleanUp
		fprintf(hFile,"  gexOptions('speed','adv_stats','%s');\n",strSpeedAdvStats.toLatin1().constData());
		fprintf(hFile,"  gexOptions('speed','db_summary','%s');\n",strSpeedAdvSummaryDB.toLatin1().constData());
		*/

		/*
		// Statistics: How is it computed
		if(ptStatistics_UseSamplesOnly->isOn())	// Compute statistics from samples only
			fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");
		else
		if(ptStatistics_UseSummaryOnly->isOn())
			fprintf(hFile,"  gexOptions('statistics','computation','summary_only');\n");
		else
		if(ptStatistics_UseSummaryThenSamples->isOn())
			fprintf(hFile,"  gexOptions('statistics','computation','summary_then_samples');\n");
		else
		if(ptStatistics_UseSamplesThenSummary->isOn())
			fprintf(hFile,"  gexOptions('statistics','computation','samples_then_summary');\n");
		*/
		
		// Cp,Cpk formulas
		/*
		if(ptStatistics_CpCpkDefaultFormula->isOn())	// Compute Cp,Cpk using standard formulas
			fprintf(hFile,"  gexOptions('statistics','cp_cpk_computation','standard');\n");
		else
			fprintf(hFile,"  gexOptions('statistics','cp_cpk_computation','percentile');\n");
		*/

		/* CRO_cleanUp
		if(ptStatistics_TestName->isOn())		// Show test name
			fprintf(hFile,"  gexOptions('statistics','field','test_name');\n");
		if(ptStatistics_TestType->isOn())		// Show test type
			fprintf(hFile,"  gexOptions('statistics','field','test_type');\n");
		if(ptStatistics_TestLimits->isOn())		// Show test limits
			fprintf(hFile,"  gexOptions('statistics','field','limits');\n");
		if(ptStatistics_SpecLimits->isOn())		// Show test limits
			fprintf(hFile,"  gexOptions('statistics','field','spec_limits');\n");
		if(ptStatistics_StatsSource->isOn())		// Show statistics source (samples, summary,...)
			fprintf(hFile,"  gexOptions('statistics','field','stats_source');\n");
		if(ptStatistics_ExecCount->isOn())		// Show execution count
			fprintf(hFile,"  gexOptions('statistics','field','exec_count');\n");
		if(ptStatistics_FailCount->isOn())		// Show failures count
			fprintf(hFile,"  gexOptions('statistics','field','fail_count');\n");
		if(ptStatistics_FailBin->isOn())		// Show Failing Bin.
			fprintf(hFile,"  gexOptions('statistics','field','fail_bin');\n");
		if(ptStatistics_Failp->isOn())			// Show failure percentage
			fprintf(hFile,"  gexOptions('statistics','field','fail_percent');\n");
		if(ptStatistics_Outlier->isOn())		// Show outlier count
			fprintf(hFile,"  gexOptions('statistics','field','outlier_count');\n");
		if(ptStatistics_Mean->isOn())			// Show Mean
			fprintf(hFile,"  gexOptions('statistics','field','mean');\n");
		if(ptStatistics_MeanShift->isOn())		// Show Mean Shift
			fprintf(hFile,"  gexOptions('statistics','field','mean_shift');\n");
		if(ptStatistics_T_Test->isOn())			// Show Student's T-Test
			fprintf(hFile,"  gexOptions('statistics','field','t_test');\n");
		if(ptStatistics_Sigma->isOn())			// Show Sigma
			fprintf(hFile,"  gexOptions('statistics','field','sigma');\n");
		if(ptStatistics_SigmaShift->isOn())	// Show Sigma Shift
			fprintf(hFile,"  gexOptions('statistics','field','sigma_shift');\n");
		if(ptStatistics_2Sigma->isOn())		// Show 2xSigma
			fprintf(hFile,"  gexOptions('statistics','field','2sigma');\n");
		if(ptStatistics_3Sigma->isOn())		// Show 3xSigma
			fprintf(hFile,"  gexOptions('statistics','field','3sigma');\n");
		if(ptStatistics_6Sigma->isOn())		// Show 6xSigma
			fprintf(hFile,"  gexOptions('statistics','field','6sigma');\n");
		if(ptStatistics_Min->isOn())			// Show Minimum
			fprintf(hFile,"  gexOptions('statistics','field','min');\n");
		if(ptStatistics_Max->isOn())			// Show maximum
			fprintf(hFile,"  gexOptions('statistics','field','max');\n");
		if(ptStatistics_Range->isOn())			// Show range
			fprintf(hFile,"  gexOptions('statistics','field','range');\n");
		if(ptStatistics_MaxRange->isOn())			// Show Max. range
			fprintf(hFile,"  gexOptions('statistics','field','max_range');\n");
		if(ptStatistics_Cp->isOn())			// Show Cp
			fprintf(hFile,"  gexOptions('statistics','field','cp');\n");
		if(ptStatistics_CpShift->isOn())		// Show Cp Shift
			fprintf(hFile,"  gexOptions('statistics','field','cp_shift');\n");
		if(ptStatistics_Cpk->isOn())			// Show Cpk
			fprintf(hFile,"  gexOptions('statistics','field','cpk');\n");
		if(ptStatistics_CpkLow->isOn())			// Show CpkLow
			fprintf(hFile,"  gexOptions('statistics','field','cpkL');\n");
		if(ptStatistics_CpkHigh->isOn())		// Show CpkHigh
			fprintf(hFile,"  gexOptions('statistics','field','cpkH');\n");
		if(ptStatistics_CpkShift->isOn())		// Show Cpk Shift
			fprintf(hFile,"  gexOptions('statistics','field','cpk_shift');\n");
		if(ptStatistics_Yield->isOn())			// Show yield
			fprintf(hFile,"  gexOptions('statistics','field','yield');\n");
		*/
		// Gage R&R info
		/*	CRO_cleanUp
		if(ptStatistics_GageEV->isOn())			// Show yield
			fprintf(hFile,"  gexOptions('statistics','field','ev');\n");
		if(ptStatistics_GageAV->isOn())			// Show yield
			fprintf(hFile,"  gexOptions('statistics','field','av');\n");
		if(ptStatistics_GageR_and_R->isOn())			// Show yield
			fprintf(hFile,"  gexOptions('statistics','field','r&r');\n");
		if(ptStatistics_GageGB->isOn())				// Show GB
			fprintf(hFile,"  gexOptions('statistics','field','gb');\n");
		if(ptStatistics_GagePV->isOn())				// Show yield
			fprintf(hFile,"  gexOptions('statistics','field','pv');\n");
		if(ptStatistics_GageTV->isOn())			// Show yield
			fprintf(hFile,"  gexOptions('statistics','field','tv');\n");
		if(ptStatistics_GageP_T->isOn())			// Show P/T ratio
			fprintf(hFile,"  gexOptions('statistics','field','p_t');\n");
		*/
		/*	CRO_cleanUp
		// Advanced statistics
		if(ptStatistics_Shape->isOn())			// Show Distribution shape
			fprintf(hFile,"  gexOptions('statistics','field','Shape');\n");
		if(ptStatistics_TestFlowID->isOn())
			fprintf(hFile,"  gexOptions('statistics','field','test_flow_id');\n");
		if(ptStatistics_Skew->isOn())			// Show Skew
			fprintf(hFile,"  gexOptions('statistics','field','Skew');\n");
		if(ptStatistics_Kurtosis->isOn())			// Show Kurtosis
			fprintf(hFile,"  gexOptions('statistics','field','Kurtosis');\n");
		if(ptStatistics_P0_5->isOn())			// Show Percentile 0.5%
			fprintf(hFile,"  gexOptions('statistics','field','P0.5%%');\n");
		if(ptStatistics_P2_5->isOn())			// Show Percentile 2.5%
			fprintf(hFile,"  gexOptions('statistics','field','P2.5%%');\n");
		if(ptStatistics_P10->isOn())			// Show Percentile 10%
			fprintf(hFile,"  gexOptions('statistics','field','P10%%');\n");
		if(ptStatistics_Quartile1->isOn())			// Show Quartile#1
			fprintf(hFile,"  gexOptions('statistics','field','Quartile1');\n");
		if(ptStatistics_Quartile2->isOn())			// Show Quartile#2
			fprintf(hFile,"  gexOptions('statistics','field','Quartile2');\n");
		if(ptStatistics_Quartile3->isOn())			// Show Quartile#3
			fprintf(hFile,"  gexOptions('statistics','field','Quartile3');\n");
		if(ptStatistics_P90->isOn())			// Show Percentile 90%
			fprintf(hFile,"  gexOptions('statistics','field','P90%%');\n");
		if(ptStatistics_P97_5->isOn())			// Show Percentile 97.5%
			fprintf(hFile,"  gexOptions('statistics','field','P97.5%%');\n");
		if(ptStatistics_P99_5->isOn())			// Show Percentile 99.5%
			fprintf(hFile,"  gexOptions('statistics','field','P99.5%%');\n");
		if(ptStatistics_Interquartiles->isOn())			// Show Interquartiles
			fprintf(hFile,"  gexOptions('statistics','field','InterQuartiles');\n");
		if(ptStatistics_SigmaInterQuartiles->isOn())			// Show SigmaInterQuartiles
			fprintf(hFile,"  gexOptions('statistics','field','SigmaInterQuartiles');\n");
		*/

		//	if(ptStatistics_Corr->isOn())			// Show correlation
		//		fprintf(hFile,"  gexOptions('statistics','field','correlation');\n");
		//	if(ptStatistics_perfLimits->isOn())	// Show perfect limits (100% yield)
		//		fprintf(hFile,"  gexOptions('statistics','field','perf_limits');\n");

		/* CRO_cleanUp
		// Statistics: sorting
		if(ptStatistics_SortTestNumber->isOn())	// Sort by test number
			fprintf(hFile,"  gexOptions('statistics','sorting','test_number');\n");
		if(ptStatistics_SortTestName->isOn())		// Sort by test name
			fprintf(hFile,"  gexOptions('statistics','sorting','test_name');\n");
		if(ptStatistics_SortTestFlowID->isOn())		// Sort by test flow ID
			fprintf(hFile,"  gexOptions('statistics','sorting','test_flow_id');\n");
		if(ptStatistics_SortMean->isOn())			// Sort by Mean
			fprintf(hFile,"  gexOptions('statistics','sorting','mean');\n");
		if(ptStatistics_SortMeanShift->isOn())			// Sort by Mean-Shift
			fprintf(hFile,"  gexOptions('statistics','sorting','mean_shift');\n");
		if(ptStatistics_SortSigma->isOn())			// Sort by Sigma
			fprintf(hFile,"  gexOptions('statistics','sorting','sigma');\n");
		if(ptStatistics_SortCp->isOn())			// Sort by Cp
			fprintf(hFile,"  gexOptions('statistics','sorting','cp');\n");
		if(ptStatistics_SortCpk->isOn())			// Sort by Cpk
			fprintf(hFile,"  gexOptions('statistics','sorting','cpk');\n");
		if(ptStatistics_SortR_and_R->isOn())			// Sort by gage R&R
			fprintf(hFile,"  gexOptions('statistics','sorting','r&r');\n");
			*/
	}

	// Statistics: alarm & warnings
	{
		/*
		if(ptStatistics_RedAlarmCpk->isOn())			// Cpk RED Alarm
		{
			strValue = ptStatistics_RedAlarmCpk->text(0);
			iIndex = strValue.find(':')+2;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// Disabled alarm
			*/

		/*
		double lfOptionStorageDevice;
		bool bGetOptionRslt;

		lfOptionStorageDevice = ReportOptions.GetOption("statistics","alarm_test_cpk").toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);

		if( !bGetOptionRslt || (lfOptionStorageDevice<0) || (!ptStatistics_RedAlarmCpk->isOn()) )
			lfOptionStorageDevice = -1;		// Disabled alarm

		strValue = QString::number(lfOptionStorageDevice);

		if(hFile != NULL)
			fprintf(hFile,"  gexOptions('statistics','alarm_test_cpk','%s');\n",strValue.toLatin1().constData());
			*/


		/*
		if(ptStatistics_YellowAlarmCpk->isOn())			// Cpk Warning Alarm
		{
			strValue = ptStatistics_YellowAlarmCpk->text(0);
			iIndex = strValue.find(':')+2;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// Disabled alarm
			*/

		/*
		lfOptionStorageDevice = ReportOptions.GetOption("statistics","warning_test_cpk").toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);

		if( !bGetOptionRslt || (lfOptionStorageDevice<0) || (!ptStatistics_YellowAlarmCpk->isOn()) )
			lfOptionStorageDevice = -1;		// Disabled alarm

		strValue = QString::number(lfOptionStorageDevice);

		if(hFile != NULL)
			fprintf(hFile,"  gexOptions('statistics','warning_test_cpk','%s');\n",strValue.toLatin1().constData());
			*/
	}

	// yield alarms
	{
		/*
		if(ptStatistics_RedAlarmTestYield->isOn())			// Test yield RED Alarm
		{
			strValue = ptStatistics_RedAlarmTestYield->text(0);
			iIndex = strValue.find(':')+2;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// Disabled alarm

		if(hFile != NULL)
			fprintf(hFile,"  gexOptions('statistics','alarm_test_yield','%s');\n",strValue.toLatin1().constData());


		if(ptStatistics_YellowAlarmTestYield->isOn())		// Test yield Warning Alarm
		{
			strValue = ptStatistics_YellowAlarmTestYield->text(0);
			iIndex = strValue.find(':')+2;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// Disabled alarm
		if(hFile != NULL)
			fprintf(hFile,"  gexOptions('statistics','warning_test_yield','%s');\n",strValue.toLatin1().constData());

			*/
	}

	// Statistics: alarm on drift
	{
		/* if(ptStatistics_CorrCheckMean->isOn())			// Sort by correlation
		{
			strValue = ptStatistics_CorrCheckMean->text(0);
			iIndex = strValue.find(':')+2;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// Disabled alarm
		if(hFile != NULL)
			fprintf(hFile,"  gexOptions('statistics','alarm_mean','%s');\n",strValue.toLatin1().constData());

		if(ptStatistics_CorrCheckSigma->isOn())			// Sort by correlation
		{
			strValue = ptStatistics_CorrCheckSigma->text(0);
			iIndex = strValue.find(':')+2;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// Disabled alarm
		if(hFile != NULL)
			fprintf(hFile,"  gexOptions('statistics','alarm_sigma','%s');\n",strValue.toLatin1().constData());

		if(ptStatistics_CorrCheckCpk->isOn())			// Sort by correlation
		{
			strValue = ptStatistics_CorrCheckCpk->text(0);
			iIndex = strValue.find(':')+2;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// Disabled alarm
		if(hFile != NULL)
			fprintf(hFile,"  gexOptions('statistics','alarm_cpk','%s');\n",strValue.toLatin1().constData()); */
	}
	
	// Drift formula for the Mean.: 1= Value drift, 2=Percentage of limits space
	/*
	if(hFile != NULL)
	{
		if(ptStatistics_CorrAbsoluteDrift->isOn())
			strValue = "value";
		else
		if(ptStatistics_CorrLimitsDrift->isOn())
			strValue = "limits";
		else
			strValue = "limits";
		fprintf(hFile,"  gexOptions('statistics','mean_drift_formula','%s');\n",strValue.toLatin1().constData());
	}
	*/


	// Generic Galaxy tests
	/* CRO_cleanUp
	if(hFile != NULL)
	{
		if(ptStatistics_IncludeGenericTests->isOn())
			strValue = "show";
		else
			strValue = "hide";
		fprintf(hFile,"  gexOptions('statistics','generic_galaxy_tests','%s');\n",strValue.toLatin1().constData());
	}
	*/

	if(hFile != NULL)
	{
		/* CRO_cleanUp by HTH case# 3801
		// Wafermap
		long iWafmapShow = 0;
		if(ptWafermap_Stacked->isOn())							// Build+Show bin STACKED wafermap
			iWafmapShow |= GEX_OPTION_WAFMAP_STACKED;
		if(ptWafermap_Individual->isOn())						// Build+Show All individual wafermaps
			iWafmapShow |= GEX_OPTION_WAFMAP_INDIVIDUAL;
		if(ptWafermap_DieMismatchBin->isOn())					// Build+Show bin mismatch wafermap
			iWafmapShow |= GEX_OPTION_WAFMAP_DIEMISMATCH_BIN;
		if(ptWafermap_DieMismatchBinToBin->isOn())					// Build+Show bin mismatch wafermap
			iWafmapShow |= GEX_OPTION_WAFMAP_DIEMISMATCH_BIN_TO_BIN;

		fprintf(hFile,"  gexOptions('wafer','chart_show','%ld');\n",iWafmapShow);
		*/

		/*
		long iParametricStackedWafermap = 0;
		if (ptWafermap_StackedParametricMean->isOn())
			iParametricStackedWafermap = GEX_OPTION_WAFMAP_PARAM_STACKED_MEAN;
		else
			iParametricStackedWafermap = GEX_OPTION_WAFMAP_PARAM_STACKED_MEDIAN;
		fprintf(hFile,"  gexOptions('wafer','parametric_stacked','%ld');\n", iParametricStackedWafermap);
		*/


		// Type of bin stacked wafermap
		/*
		long iBinStackedWafermap = 0;
		if (ptWafermap_StackedBinCount->isOn())
			iBinStackedWafermap = GEX_OPTION_WAFMAP_BIN_STACKED_BINCOUNT;
		else if (ptWafermap_StackedPassFailAll->isOn())
			iBinStackedWafermap = GEX_OPTION_WAFMAP_BIN_STACKED_PASSFAILALL;
#if 0	// Waiting for new options
		else if (ptWafermap_StackedPassFailAtLeast->isOn())
			iBinStackedWafermap = GEX_OPTION_WAFMAP_BIN_STACKED_PASSFAILATLEAST;
		else if (ptWafermap_StackedHighestBin->isOn())
			iBinStackedWafermap = GEX_OPTION_WAFMAP_BIN_STACKED_HIGHESTBIN;
		else if (ptWafermap_StackedLowestBin->isOn())
			iBinStackedWafermap = GEX_OPTION_WAFMAP_BIN_STACKED_HIGHESTFAILINGBIN;
#endif	
		//fprintf(hFile,"  gexOptions('wafer','bin_stacked','%ld');\n", iBinStackedWafermap);
		*/

#if 1 // BG: WaferMap orientation options
		/* CRO_cleanUp by HTH case# 3821
		if(ptWafermap_PositiveX_Left->isOn())		// Positive X probing: Left
			fprintf(hFile,"  gexOptions('wafer','chart_view','positive_x_left');\n");
		if(ptWafermap_PositiveX_Right->isOn())		// Positive X probing: Right
			fprintf(hFile,"  gexOptions('wafer','chart_view','positive_x_right');\n");
		if(ptWafermap_PositiveX_Detect->isOn())		// Positive X probing: Auto detect
			fprintf(hFile,"  gexOptions('wafer','chart_view','positive_x_auto');\n");

		if(ptWafermap_PositiveY_Up->isOn())		// Positive Y probing: Up
			fprintf(hFile,"  gexOptions('wafer','chart_view','positive_y_Up');\n");
		if(ptWafermap_PositiveY_Down->isOn())		// Positive Y probing: Down
			fprintf(hFile,"  gexOptions('wafer','chart_view','positive_y_down');\n");
		if(ptWafermap_PositiveY_Detect->isOn())		// Positive Y probing: Auto detect
			fprintf(hFile,"  gexOptions('wafer','chart_view','positive_y_auto');\n");	
		*/
#endif
		/* CRO_cleanUp by HTH case# 3822
		if (ptWafermap_ExportAlarm_FlatDiffers->isOn()) // Alarm the customer when flat location differs from file on wafermap export
				fprintf(hFile, "  gexOptions('wafer','alarms','flat_differs_on');\n");
		else
			fprintf(hFile, "  gexOptions('wafer','alarms','flat_differs_off');\n");

		if (ptWafermap_ExportAlarm_NoFlat->isOn()) // Alarm the customer when no flat is detected from file on wafermap export
			fprintf(hFile, "  gexOptions('wafer','alarms','no_flat_on');\n");
		else
			fprintf(hFile, "  gexOptions('wafer','alarms','no_flat_off');\n");
		*/

		/* CRO_cleanUp by HTH case# 3821
		if(ptWafermap_MirrorX->isOn())		// Swap the Dies on the X axis
			fprintf(hFile,"  gexOptions('wafer','chart_view','mirror_x');\n");
		else
			fprintf(hFile,"  gexOptions('wafer','chart_view','no_mirror_x');\n");
		if(ptWafermap_MirrorY->isOn())		// Swap the Dies on the Y axis
			fprintf(hFile,"  gexOptions('wafer','chart_view','mirror_y');\n");
		else
			fprintf(hFile,"  gexOptions('wafer','chart_view','no_mirror_y');\n");
		if(ptWafermap_AlwaysRound->isOn())		// Wafer shape always look round?
			fprintf(hFile,"  gexOptions('wafer','chart_view','shape_round');\n");
		else
			fprintf(hFile,"  gexOptions('wafer','chart_view','shape_default');\n");
		if(ptWafermap_FullWafer->isOn())		// Wafermap shows ALL parts (no matter the part filtering)
			fprintf(hFile,"  gexOptions('wafer','chart_view','all_parts');\n");
		else
			fprintf(hFile,"  gexOptions('wafer','chart_view','filtered_parts');\n");
			*/
		
		/*
		if(ptWafermap_RetestPolicy_Highest->isOn())
			fprintf(hFile,"  gexOptions('wafer','retest_policy','highest_bin');\n");
		else
			fprintf(hFile,"  gexOptions('wafer','retest_policy','last_bin');\n");
		*/

		/* CRO_cleanUp
		if(ptWafermap_Small->isOn())		// Small chart
			fprintf(hFile,"  gexOptions('wafer','chart_size','small');\n");
		if(ptWafermap_Medium->isOn())		// Medium chart
			fprintf(hFile,"  gexOptions('wafer','chart_size','medium');\n");
		if(ptWafermap_Large->isOn())		// Large chart
			fprintf(hFile,"  gexOptions('wafer','chart_size','large');\n");
		if(ptWafermap_Adaptive->isOn())	// Adaptive chart size
			fprintf(hFile,"  gexOptions('wafer','chart_size','auto');\n");
			*/

		/* CRO_cleanUp by HTH case# 3819
		if(ptWafermap_MarkerBin->isOn())	// Marker: Bin#
			fprintf(hFile,"  gexOptions('wafer','marker','bin');\n");
		if(ptWafermap_MarkerRetest->isOn())	// Marker: cross for Die retested
			fprintf(hFile,"  gexOptions('wafer','marker','retest');\n");
		*/

		// Extract Low-Yield alarm for failing pattern identification in stacked wafers
		/*
		strValue = ptWafermap_StackedLowYieldAlarm->text(0);
		iIndex = strValue.find(':')+2;
		strValue = strValue.mid(iIndex);
		fprintf(hFile,"  gexOptions('wafer','low_yield_pattern','%s');\n",strValue.toLatin1().constData());
		// */

		/* CRO_cleanUp by HTH case# 3820
		// Accept comapring wafers with different size?
		if(ptWafermap_CompareAnySize->isOn())
			fprintf(hFile,"  gexOptions('wafer','compare','any_size');\n");

		// Include die mismatch table in Wafermap compare report?
		if(!ptWafermap_IncludeDieMismatchTable->isOn())
			fprintf(hFile,"  gexOptions('wafer','compare','diemismatch_table_off');\n");

		// Include delta yield section in Wafermap compare report?
		if(!ptWafermap_IncludeDeltaYieldSection->isOn())
			fprintf(hFile,"  gexOptions('wafer','compare','deltayield_section_off');\n");
		*/

		// Statistic fields to be displayed with advanced charts (histograms, trend charts...)
		/*		CRO_cleanUp
		iIndex = (ptHistogram_Limits->isOn()) ? 1 : 0;	// Stats: test limits count.
		fprintf(hFile,"  gexOptions('adv_chart','field','limits','%d');\n",iIndex);
		if(ptHistogram_ExecCount->isOn())	// Stats: Exec count.
			fprintf(hFile,"  gexOptions('adv_chart','field','exec_count');\n");
		if(ptHistogram_Mean->isOn())		// Stats: Mean
			fprintf(hFile,"  gexOptions('adv_chart','field','mean');\n");
		if(ptHistogram_Quartile2->isOn())	// Stats: Median
			fprintf(hFile,"  gexOptions('adv_chart','field','Quartile2');\n");
		if(ptHistogram_Sigma->isOn())		// Stats: Sigma
			fprintf(hFile,"  gexOptions('adv_chart','field','sigma');\n");
		if(ptHistogram_Fails->isOn())		// Stats: Fails count
			fprintf(hFile,"  gexOptions('adv_chart','field','fail_count');\n");
		if(ptHistogram_Outlier->isOn())		// Stats: Outliers
			fprintf(hFile,"  gexOptions('adv_chart','field','outlier_count');\n");
		if(ptHistogram_Cp->isOn())			// Stats: Cp
			fprintf(hFile,"  gexOptions('adv_chart','field','cp');\n");
		if(ptHistogram_Cpk->isOn())			// Stats: Cpk
			fprintf(hFile,"  gexOptions('adv_chart','field','cpk');\n");
		if(ptHistogram_Min->isOn())			// Stats: Min
			fprintf(hFile,"  gexOptions('adv_chart','field','min');\n");
		if(ptHistogram_Max->isOn())			// Stats: Max
			fprintf(hFile,"  gexOptions('adv_chart','field','max');\n");
		*/


		// Histogram		CRO_cleanUp
			/*
		if(ptHistogram_TypeBars->isOn())			// chart type= BARS
			fprintf(hFile,"  gexOptions('adv_histogram','chart_type','bars');\n");
		if(ptHistogram_Type3D_Bars->isOn())			// chart type= 3D-BARS
			fprintf(hFile,"  gexOptions('adv_histogram','chart_type','3d_bars');\n");
		if(ptHistogram_TypeBarsOutline->isOn())
			fprintf(hFile,"  gexOptions('adv_histogram','chart_type','bars_outline');\n");	// Charttype = Bars outline
		if(ptHistogram_TypeSpline->isOn())			// chart type= Curve
			fprintf(hFile,"  gexOptions('adv_histogram','chart_type','curve');\n");
		if(ptHistogram_TypeBarsSpline->isOn())		// chart type= bars+Curve
			fprintf(hFile,"  gexOptions('adv_histogram','chart_type','bars_curve');\n");
		if(ptHistogram_TypeBarsGaussian->isOn())		// chart type= Gaussian+Bars
			fprintf(hFile,"  gexOptions('adv_histogram','chart_type','bars_gaussian');\n");
		if(ptHistogram_TypeBarsOutlineGaussian->isOn())		// chart type= Gaussian+Bars outline
			fprintf(hFile,"  gexOptions('adv_histogram','chart_type','bars_outline_gaussian');\n");
		if(ptHistogram_TypeGaussian->isOn())		// chart type= Gaussian
			fprintf(hFile,"  gexOptions('adv_histogram','chart_type','gaussian');\n");
			*/

		/*
		if(ptHistogram_Yaxis_Percentage->isOn())
			fprintf(hFile,"  gexOptions('adv_histogram','y_axis','percentage');\n");
		if(ptHistogram_Yaxis_Hits->isOn())
			fprintf(hFile,"  gexOptions('adv_histogram','y_axis','hits');\n");
		*/
		/*
		if(ptHistogram_Banner->isOn())				// Small chart
			fprintf(hFile,"  gexOptions('adv_histogram','chart_size','banner');\n");
		if(ptHistogram_Small->isOn())				// Small chart
			fprintf(hFile,"  gexOptions('adv_histogram','chart_size','small');\n");
		if(ptHistogram_Medium->isOn())				// Medium chart
			fprintf(hFile,"  gexOptions('adv_histogram','chart_size','medium');\n");
		if(ptHistogram_Large->isOn())				// Large chart
			fprintf(hFile,"  gexOptions('adv_histogram','chart_size','large');\n");
		if(ptHistogram_Adaptive->isOn())			// Adaptive chart size
			fprintf(hFile,"  gexOptions('adv_histogram','chart_size','auto');\n");
		*/
		/*	CRO_cleanUp
		if(ptHistogram_MarkerTestName->isOn())		// Marker: test name
			fprintf(hFile,"  gexOptions('adv_histogram','marker','test_name');\n");
		if(ptHistogram_MarkerMean->isOn())			// Marker: mean
			fprintf(hFile,"  gexOptions('adv_histogram','marker','mean');\n");
		if(ptHistogram_MarkerMedian->isOn())		// Marker: median
			fprintf(hFile,"  gexOptions('adv_histogram','marker','median');\n");
		if(ptHistogram_MarkerLimits->isOn())		// Marker:limits
			fprintf(hFile,"  gexOptions('adv_histogram','marker','limits');\n");
		if(ptHistogram_MarkerSpecLimits->isOn())	// Marker: Spec limits
			fprintf(hFile,"  gexOptions('adv_histogram','marker','speclimits');\n");
		if(ptHistogram_Marker2Sigma->isOn())		// Marker:2 sigma
			fprintf(hFile,"  gexOptions('adv_histogram','marker','2sigma');\n");
		if(ptHistogram_Marker3Sigma->isOn())		// Marker:3 sigma
			fprintf(hFile,"  gexOptions('adv_histogram','marker','3sigma');\n");
		if(ptHistogram_Marker6Sigma->isOn())		// Marker:6 sigma
			fprintf(hFile,"  gexOptions('adv_histogram','marker','6sigma');\n");
		*/
		/*
		strValue = ptHistogram_TotalBars->text(0);
		iIndex = strValue.find(':')+2;
		strValue = strValue.mid(iIndex);
		fprintf(hFile,"  gexOptions('adv_histogram','total_bars','%s');\n",strValue.toLatin1().constData());
		*/


		// Trend
		/*
		if(ptTrend_TypeLines->isOn())			// chart type= Lines
			fprintf(hFile,"  gexOptions('adv_trend','chart_type','lines');\n");
		if(ptTrend_TypeSpots->isOn())			// chart type= Spots
			fprintf(hFile,"  gexOptions('adv_trend','chart_type','spots');\n");
		if(ptTrend_TypeLinesSpots->isOn())		// chart type= Lines+Spots
			fprintf(hFile,"  gexOptions('adv_trend','chart_type','lines_spots');\n");
		*/
		/*
		if(ptTrend_Banner->isOn())				// Small chart
			fprintf(hFile,"  gexOptions('adv_trend','chart_size','banner');\n");
		if(ptTrend_Small->isOn())				// Small chart
			fprintf(hFile,"  gexOptions('adv_trend','chart_size','small');\n");
		if(ptTrend_Medium->isOn())				// Medium chart
			fprintf(hFile,"  gexOptions('adv_trend','chart_size','medium');\n");
		if(ptTrend_Large->isOn())				// Large chart
			fprintf(hFile,"  gexOptions('adv_trend','chart_size','large');\n");
		if(ptTrend_Adaptive->isOn())			// Adaptive chart size
			fprintf(hFile,"  gexOptions('adv_trend','chart_size','auto');\n");
		*/
		/*
		if (ptTrend_XAxisRunID->isOn())			// X axis uses Run ID
			fprintf(hFile,"  gexOptions('adv_trend','x_axis','run_id');\n");
		if (ptTrend_XAxisPartID->isOn())		// X axis uses Part ID
			fprintf(hFile,"  gexOptions('adv_trend','x_axis','part_id');\n");
		*/

		/*		CRO_cleanUp
		if(ptTrend_MarkerTestName->isOn())		// Marker: test name
			fprintf(hFile,"  gexOptions('adv_trend','marker','test_name');\n");
		if(ptTrend_MarkerMean->isOn())			// Marker: mean
			fprintf(hFile,"  gexOptions('adv_trend','marker','mean');\n");
		if(ptTrend_MarkerMedian->isOn())		// Marker: median
			fprintf(hFile,"  gexOptions('adv_trend','marker','median');\n");
		if(ptTrend_MarkerLimits->isOn())		// Marker:limits
			fprintf(hFile,"  gexOptions('adv_trend','marker','limits');\n");
		if(ptTrend_MarkerSpecLimits->isOn())	// Marker: Spec limits
			fprintf(hFile,"  gexOptions('adv_trend','marker','speclimits');\n");
		if(ptTrend_MarkerLot->isOn())			// Marker:Lot ID
			fprintf(hFile,"  gexOptions('adv_trend','marker','lot');\n");
		if(ptTrend_MarkerSubLot->isOn())		// Marker:Sub-Lot / Wafer ID
			fprintf(hFile,"  gexOptions('adv_trend','marker','sublot');\n");
		if(ptTrend_MarkerGroupName->isOn())		// Marker: dataset / group name
			fprintf(hFile,"  gexOptions('adv_trend','marker','group_name');\n");
		if(ptTrend_Marker2Sigma->isOn())		// Marker:2 sigma
			fprintf(hFile,"  gexOptions('adv_trend','marker','2sigma');\n");
		if(ptTrend_Marker3Sigma->isOn())		// Marker:3 sigma
			fprintf(hFile,"  gexOptions('adv_trend','marker','3sigma');\n");
		if(ptTrend_Marker6Sigma->isOn())		// Marker:6 sigma
			fprintf(hFile,"  gexOptions('adv_trend','marker','6sigma');\n");
		*/

		// Extract Total parts in a Rolling Yield
		/*
		strValue = ptTrend_RollingYield->text(0);
		iIndex = strValue.find(':')+2;
		strValue = strValue.mid(iIndex);
		fprintf(hFile,"  gexOptions('adv_trend','rolling_yield','%s');\n",strValue.toLatin1().constData());
		*/

		//// Correlation (XY-Scatter)
		/* case 3842
		if(ptScatter_Small->isOn())					// Small chart
			fprintf(hFile,"  gexOptions('adv_correlation','chart_size','small');\n");
		if(ptScatter_Medium->isOn())				// Medium chart
			fprintf(hFile,"  gexOptions('adv_correlation','chart_size','medium');\n");
		if(ptScatter_Large->isOn())					// Large chart
			fprintf(hFile,"  gexOptions('adv_correlation','chart_size','large');\n");
		if(ptScatter_Adaptive->isOn())				// Adaptive chart size
			fprintf(hFile,"  gexOptions('adv_correlation','chart_size','auto');\n");
		*/
		/*if(ptScatter_MarkerTestName->isOn())		// Marker: test name
			fprintf(hFile,"  gexOptions('adv_correlation','marker','test_name');\n");
		if(ptScatter_MarkerMean->isOn())			// Marker: mean
			fprintf(hFile,"  gexOptions('adv_correlation','marker','mean');\n");
		if(ptScatter_MarkerMedian->isOn())			// Marker: median
			fprintf(hFile,"  gexOptions('adv_correlation','marker','median');\n");
		if(ptScatter_MarkerLimits->isOn())			// Marker:limits
			fprintf(hFile,"  gexOptions('adv_correlation','marker','limits');\n");
		if(ptScatter_MarkerSpecLimits->isOn())		// Marker: Spec limits
			fprintf(hFile,"  gexOptions('adv_correlation','marker','speclimits');\n");
		if(ptScatter_Marker2Sigma->isOn())			// Marker:2 sigma
			fprintf(hFile,"  gexOptions('adv_correlation','marker','2sigma');\n");
		if(ptScatter_Marker3Sigma->isOn())			// Marker:3 sigma
			fprintf(hFile,"  gexOptions('adv_correlation','marker','3sigma');\n");
		if(ptScatter_Marker6Sigma->isOn())			// Marker:6 sigma
			fprintf(hFile,"  gexOptions('adv_correlation','marker','6sigma');\n");
		*/
		// Probability plot
		/*
		if(ptProbPlot_Small->isOn())				// Small chart
			fprintf(hFile,"  gexOptions('adv_probabilityplot','chart_size','small');\n");
		if(ptProbPlot_Medium->isOn())				// Medium chart
			fprintf(hFile,"  gexOptions('adv_probabilityplot','chart_size','medium');\n");
		if(ptProbPlot_Large->isOn())				// Large chart
			fprintf(hFile,"  gexOptions('adv_probabilityplot','chart_size','large');\n");
		if(ptProbPlot_Adaptive->isOn())				// Adaptive chart size
			fprintf(hFile,"  gexOptions('adv_probabilityplot','chart_size','auto');\n");
		*/

		/*		CRO_cleanUp
		if(ptProbPlot_MarkerTestName->isOn())		// Marker: test name
			fprintf(hFile,"  gexOptions('adv_probabilityplot','marker','test_name');\n");
		if(ptProbPlot_MarkerMean->isOn())			// Marker: mean
			fprintf(hFile,"  gexOptions('adv_probabilityplot','marker','mean');\n");
		if(ptProbPlot_MarkerMedian->isOn())			// Marker: median
			fprintf(hFile,"  gexOptions('adv_probabilityplot','marker','median');\n");
		if(ptProbPlot_MarkerLimits->isOn())			// Marker:limits
			fprintf(hFile,"  gexOptions('adv_probabilityplot','marker','limits');\n");
		if(ptProbPlot_MarkerSpecLimits->isOn())		// Marker: Spec limits
			fprintf(hFile,"  gexOptions('adv_probabilityplot','marker','speclimits');\n");
		if(ptProbPlot_Marker2Sigma->isOn())			// Marker:2 sigma
			fprintf(hFile,"  gexOptions('adv_probabilityplot','marker','2sigma');\n");
		if(ptProbPlot_Marker3Sigma->isOn())			// Marker:3 sigma
			fprintf(hFile,"  gexOptions('adv_probabilityplot','marker','3sigma');\n");
		if(ptProbPlot_Marker6Sigma->isOn())			// Marker:6 sigma
			fprintf(hFile,"  gexOptions('adv_probabilityplot','marker','6sigma');\n");
		*/


		/*
		if(ptProbPlot_Yaxis_Sigma->isOn())			// Y-axis type: Z sigma
			fprintf(hFile,"  gexOptions('adv_probabilityplot','y_axis','sigma');\n");
		if(ptProbPlot_Yaxis_Percent->isOn())		// Y-axis type: Percentile
			fprintf(hFile,"  gexOptions('adv_probabilityplot','y_axis','percentage');\n");
		*/

		// Box plot
		/*
		if(ptBoxPlotEx_Small->isOn())			// Small chart
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','chart_size','small');\n");
		if(ptBoxPlotEx_Medium->isOn())			// Medium chart
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','chart_size','medium');\n");
		if(ptBoxPlotEx_Large->isOn())			// Large chart
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','chart_size','large');\n");
		if(ptBoxPlotEx_Adaptive->isOn())			// Adaptive chart size
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','chart_size','auto');\n");
		*/
		/*		CRO_cleanUp
		if(ptBoxPlotEx_MarkerTestName->isOn())	// Marker: test name
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','marker','test_name');\n");
		if(ptBoxPlotEx_MarkerMean->isOn())		// Marker: mean
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','marker','mean');\n");
		if(ptBoxPlotEx_MarkerMedian->isOn())		// Marker: median
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','marker','median');\n");
		if(ptBoxPlotEx_MarkerLimits->isOn())		// Marker:limits
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','marker','limits');\n");
		if(ptBoxPlotEx_Marker2Sigma->isOn())		// Marker:2 sigma
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','marker','2sigma');\n");
		if(ptBoxPlotEx_Marker3Sigma->isOn())		// Marker:3 sigma
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','marker','3sigma');\n");
		if(ptBoxPlotEx_Marker6Sigma->isOn())		// Marker:6 sigma
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','marker','6sigma');\n");
		*/
		/*
		if(ptBoxplotEx_Intercative_V->isOn())		// Boxplot charting type: Vertical boxes
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','orientation','vertical');\n");
		if(ptBoxplotEx_Intercative_H->isOn())		// Boxplot charting type: horizontal boxes
			fprintf(hFile,"  gexOptions('adv_boxplot_ex','orientation','horizontal');\n");
		*/

		// Production yield/volume
		/*
		if(ptProdYield_YieldVolumeChart->isOn())
			fprintf(hFile,"  gexOptions('adv_production_yield','chart_type','yield_volume');\n");
		if(ptProdYield_YieldChart->isOn())
			fprintf(hFile,"  gexOptions('adv_production_yield','chart_type','yield');\n");
		if(ptProdYield_VolumeChart->isOn())
			fprintf(hFile,"  gexOptions('adv_production_yield','chart_type','volume');\n");
			*/

		/* CRO_cleanUp
		if(ptProdYield_MarkerTitle->isOn())
			fprintf(hFile,"  gexOptions('adv_production_yield','marker','title');\n");
		if(ptProdYield_MarkerYield->isOn())
			fprintf(hFile,"  gexOptions('adv_production_yield','marker','yield');\n");
		if(ptProdYield_MarkerVolume->isOn())
			fprintf(hFile,"  gexOptions('adv_production_yield','marker','volume');\n");
		*/

		// Boxplot
		/*	CRO_cleanUp
		if(ptBoxPlot_TestLimits->isOn())			// Show Test limits
			fprintf(hFile,"  gexOptions('adv_boxplot','field','limits');\n");
		if(ptBoxPlot_Mean->isOn())					// Show Mean
			fprintf(hFile,"  gexOptions('adv_boxplot','field','mean');\n");
		if(ptBoxPlot_Sigma->isOn())					// Show Sigma
			fprintf(hFile,"  gexOptions('adv_boxplot','field','sigma');\n");
		if(ptBoxPlot_Median->isOn())					// Show Median
			fprintf(hFile,"  gexOptions('adv_boxplot','field','median');\n");
		if(ptBoxPlot_Cp->isOn())					// Show Cp
			fprintf(hFile,"  gexOptions('adv_boxplot','field','cp');\n");
		if(ptBoxPlot_Cpk->isOn())					// Show Cpk
			fprintf(hFile,"  gexOptions('adv_boxplot','field','cpk');\n");
		if(ptBoxPlot_Repeatability->isOn())			// Show dataset repeatability
			fprintf(hFile,"  gexOptions('adv_boxplot','field','repeatability');\n");
		if(ptBoxPlot_RepeatabilityEV->isOn())			// Show Equipment variation EV
			fprintf(hFile,"  gexOptions('adv_boxplot','field','ev');\n");
		if(ptBoxPlot_ReproducibilityAV->isOn())		// Show ReproducibilityAV (appraiser variation)
			fprintf(hFile,"  gexOptions('adv_boxplot','field','av');\n");
		if(ptBoxPlot_R_and_R->isOn())				// Show R&R
			fprintf(hFile,"  gexOptions('adv_boxplot','field','r&r');\n");
		if(ptBoxPlot_GB->isOn())					// Show GB
			fprintf(hFile,"  gexOptions('adv_boxplot','field','gb');\n");
		if(ptBoxPlot_PV->isOn())					// Show Part Variation
			fprintf(hFile,"  gexOptions('adv_boxplot','field','pv');\n");
		if(ptBoxPlot_TV->isOn())					// Show Total Variation
			fprintf(hFile,"  gexOptions('adv_boxplot','field','tv');\n");
		if(ptBoxPlot_P_T->isOn())					// Show Total Variation
			fprintf(hFile,"  gexOptions('adv_boxplot','field','p_t');\n");
		*/


		/*
		// Boxplot charting type
		if(ptBoxplot_ChartLimits->isOn())	// Charting: Over test limits
			fprintf(hFile,"  gexOptions('adv_boxplot','chart_type','limits');\n");
		if(ptBoxplot_ChartRange->isOn())	// Charting: Over range
			fprintf(hFile,"  gexOptions('adv_boxplot','chart_type','range');\n");
		if(ptBoxplot_ChartAdaptive->isOn())	// Charting: Adaptive Limits or range
			fprintf(hFile,"  gexOptions('adv_boxplot','chart_type','adaptive');\n");
			*/

		/* CRO_cleanUp
		// Boxplot sort
		if(ptBoxplot_SortTestNumber->isOn())	// Sort by test number
			fprintf(hFile,"  gexOptions('adv_boxplot','sorting','test_number');\n");
		if(ptBoxplot_SortTestName->isOn())		// Sort by test name
			fprintf(hFile,"  gexOptions('adv_boxplot','sorting','test_name');\n");
		if(ptBoxplot_SortTestFlowID->isOn())		// Sort by test flow ID
			fprintf(hFile,"  gexOptions('adv_boxplot','sorting','test_flow_id');\n");
		if(ptBoxplot_SortMean->isOn())			// Sort by Mean
			fprintf(hFile,"  gexOptions('adv_boxplot','sorting','mean');\n");
		if(ptBoxplot_SortSigma->isOn())		// Sort by Sigma
			fprintf(hFile,"  gexOptions('adv_boxplot','sorting','sigma');\n");
		if(ptBoxplot_SortCp->isOn())			// Sort by Cp
			fprintf(hFile,"  gexOptions('adv_boxplot','sorting','cp');\n");
		if(ptBoxplot_SortCpk->isOn())			// Sort by Cpk
			fprintf(hFile,"  gexOptions('adv_boxplot','sorting','cpk');\n");
		if(ptBoxplot_SortR_and_R->isOn())			// Sort by R&R
			fprintf(hFile,"  gexOptions('adv_boxplot','sorting','r&r');\n");
			*/

		// R&R N*Sigma computation rule
		/*
		strValue = ptBoxplot_Nsigma->text(0);
		iIndex = strValue.find('=')+1;
		strValue = strValue.mid(iIndex);
		fprintf(hFile,"  gexOptions('adv_boxplot','r&r_sigma',%s);\n",strValue.toLatin1().constData());
		*/

		// R&R % shift: Over TV or over Limits ?
		if(ptBoxplot_ShiftOverTV->isOn())				// % shift over TV value
			fprintf(hFile,"  gexOptions('adv_boxplot','%%','over_tv');\n");
		if(ptBoxplot_ShiftOverLimits->isOn())			// % shift over Limits space value
			fprintf(hFile,"  gexOptions('adv_boxplot','%%','over_limits');\n");


		// Pareto
		/*
		if(ptPareto_CpCutOff->isOn())	// Cp cutoff defined.
		{
			strValue = ptPareto_CpCutOff->text(0);
			iIndex = strValue.find(':')+1;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// No Cp cutoff: report ALL Cp.
		fprintf(hFile,"  gexOptions('pareto','cutoff_cp','%s');\n",strValue.toLatin1().constData());	// Cp Cut-off limit
		*/

		/*
		if(ptPareto_CpkCutOff->isOn())	// Cpk cutoff defined.
		{
			strValue = ptPareto_CpkCutOff->text(0);
			iIndex = strValue.find(':')+1;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// No Cpk cutoff: report ALL Cp.
		fprintf(hFile,"  gexOptions('pareto','cutoff_cpk','%s');\n",strValue.toLatin1().constData());	// Cp Cut-off limit
		*/

		/*
		if(ptPareto_FailureCutOff->isOn())	// Test Failures cutoff defined.
		{
			strValue = ptPareto_FailureCutOff->text(0);
			iIndex = strValue.find(':')+1;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// No  Failure cutoff: report ALL test failures.
		fprintf(hFile,"  gexOptions('pareto','cutoff_failure','%s');\n",strValue.toLatin1().constData());	// Test failures Cut-off limit
		*/

		/* CRO_cleanup
		if(ptPareto_SignatureFailureCutOff->isOn())	// Signature Failure cutoff defined.
		{
			strValue = ptPareto_SignatureFailureCutOff->text(0);
			iIndex = strValue.find(':')+1;
			strValue = strValue.mid(iIndex);
		}
		else
			strValue = "-1";	// No Signature Failure cutoff: report ALL Signature failures.
		fprintf(hFile,"  gexOptions('pareto','cutoff_signature_failure','%s');\n",strValue.toLatin1().constData());	// Signature failures Cut-off limit
		*/

		//
		/* CRO_cleanup
		if(ptPareto_ExcludePassBin->isOn())				// Exclude PASS binnings in pareto section
			fprintf(hFile,"  gexOptions('pareto','excludebinnings','pass');\n");
		if(ptPareto_ExcludeFailBin->isOn())				// Exclude FAIL binnings in pareto section
			fprintf(hFile,"  gexOptions('pareto','excludebinnings','fail');\n");
		*/

		// Binning
		/*
		if(ptBinning_UseWafermapOnly->isOn())	// Compute binning from summary (SBR/HBR)
			fprintf(hFile,"  gexOptions('binning','computation','wafer_map');\n");
		if(ptBinning_UseSamplesOnly->isOn())	// Compute binning from samples (PRR)
			fprintf(hFile,"  gexOptions('binning','computation','samples');\n");
		if(ptBinning_UseSummary->isOn())		// Compute binning from wafermap.
			fprintf(hFile,"  gexOptions('binning','computation','summary');\n");
			*/
		/*
		if(ptBinning_InReport->isOn())			// Include section in report
			fprintf(hFile,"  gexOptions('binning','section','enabled');\n");
		else
			fprintf(hFile,"  gexOptions('binning','section','disabled');\n");
		*/

		// Datalog
		/*		CRO_cleanUp
		if(ptDatalog_Comment->isOn())	// Show Comment
			fprintf(hFile,"  gexOptions('adv_datalog','field','comment');\n");
		if(ptDatalog_TestNumber->isOn())	// Show test number
			fprintf(hFile,"  gexOptions('adv_datalog','field','test_number');\n");
		if(ptDatalog_TestName->isOn())	// Show test name
			fprintf(hFile,"  gexOptions('adv_datalog','field','test_name');\n");
		if(ptDatalog_Limits->isOn())	// Show limits
			fprintf(hFile,"  gexOptions('adv_datalog','field','limits');\n");
		if(ptDatalog_DieLoc->isOn())	// Show die XY loc
			fprintf(hFile,"  gexOptions('adv_datalog','field','die_loc');\n");
		*/
		/*
		if(ptDatalog_1Row->isOn())	// Single row display
			fprintf(hFile,"  gexOptions('adv_datalog','format','1row');\n");
		else
			fprintf(hFile,"  gexOptions('adv_datalog','format','2rows');\n");
		*/

		/* CRO_cleanUp by HTH case# 4236
		// What-if binnings
		fprintf(hFile,"  gexOptions('adv_what_if','pass_bin','%d');\n",ReportOptions.iWhatIf_PassBin);
		fprintf(hFile,"  gexOptions('adv_what_if','fail_bin','%d');\n",ReportOptions.iWhatIf_FailBin);
		strValue = (ptWhatif_Fail_BinningIsPass->isOn()) ? "1" : "0";
		fprintf(hFile,"  gexOptions('adv_what_if','fail_bin_is_pass','%s');\n",strValue.toLatin1().constData());
		*/

		/* CRO_cleanUp by HTH case# 4237
		// Pearson's correlation report
		fprintf(hFile,"  gexOptions('adv_pearson','cutoff','%g');\n",ReportOptions.lfPearsonCutoff);
		if(ptPearson_SortRatio->isOn())
			fprintf(hFile,"  gexOptions('adv_pearson','sorting','pearson');\n");
		if(ptPearson_SortTestName->isOn())
			fprintf(hFile,"  gexOptions('adv_pearson','sorting','test_name');\n");
		*/

		/* CRO_cleanUp by HTH case# 4238
		// Toolbox
		if(ptToolbox_CsvSortTestFlowID->isOn())
			fprintf(hFile,"  gexOptions('toolbox','csv_sorting','flow_id');\n");	// Sorting: By test flowID
		if(ptToolbox_CsvSortTestID->isOn())
			fprintf(hFile,"  gexOptions('toolbox','csv_sorting','test_id');\n");	// Sorting: By Test#

		if(ptToolbox_CsvSplitExport->isOn())
			fprintf(hFile,"  gexOptions('toolbox','csv_split_export','on');\n");	// Split export to 250-rows CSV files enabled
		else
			fprintf(hFile,"  gexOptions('toolbox','csv_split_export','off');\n");	// Split export to 250-rows CSV files Disabled

		if(ptToolbox_csvUnitsModeNormalized->isOn())
			fprintf(hFile,"  gexOptions('toolbox','csv_units_mode','normalized');\n");		// Normalize units 
		
		if(ptToolbox_csvUnitsModeScalingFactor->isOn())
			fprintf(hFile,"  gexOptions('toolbox','csv_units_mode','scaling_factor');\n");	// Units exported using scaling factor
		*/

		/////////////////////////////////////////////
		// Application preferences.
		/////////////////////////////////////////////

		/*
		if(hFile != NULL)
		{
			QMapIterator<QString, QString> it(ReportOptions.m_PrefMap);
			while (it.hasNext())
			{
				it.next();
				if (it.key()=="ssheet_editor")
					continue; // we will write the one specified in the OptionsDialog widget
				fprintf(hFile,"  gexOptions('preferences','%s','%s');\n", it.key().toLatin1().data(), it.value().toLatin1().data() );
			}
		}
		*/

		/*
		// Retrieve custom editor.
		strValue = ptPreferences_CustomTextEditor->text(0);
		// extract path from string 'Current editor: XXXX'
		strValue = strValue.mid(16);
		// Convert file path to be script compatible '\' becomes '\\'
		ConvertToScriptString(strValue);
		fprintf(hFile,"  gexOptions('preferences','text_editor','%s');\n",strValue.toLatin1().constData());
		*/

		/*
		// Retrieve custom Spread Sheet Editor.
		strValue = ptPreferences_CustomSpreadsheetEditor->text(0);
		// extract path from string 'Current editor: XXXX'
		strValue = strValue.mid(16);
		// Convert file path to be script compatible '\' becomes '\\'
		ConvertToScriptString(strValue);
		//fprintf(hFile,"  //custom SpreadsheetEditor. Fix Me : use the one defined in .csl or the custom ?\n");
		fprintf(hFile,"  gexOptions('preferences','ssheet_editor','%s');\n",strValue.toLatin1().constData() );
		*/

		// Closing node if Idle too loong...
		/*
		if(ptPreferences_AutoClose15minutes->isOn())
			strValue="15min";
		else
		if(ptPreferences_AutoClose30minutes->isOn())
			strValue="30min";
		else
		if(ptPreferences_AutoClose1hour->isOn())
			strValue="1hour";
		else
		if(ptPreferences_AutoClose2hours->isOn())
			strValue="2hours";
		else
		if(ptPreferences_AutoClose4hours->isOn())
			strValue="4hours";
		else
			strValue = "never";
		fprintf(hFile,"  gexOptions('preferences','auto_close','%s');\n",strValue.toLatin1().constData());
		*/

		/////////////////////////////////////////////
		// Interactive style.
		/////////////////////////////////////////////
		ReportOptions.WriteChartStyleToFile(hFile);

		/////////////////////////////////////////////
		// Custom Binning colors.
		/////////////////////////////////////////////
		ReportOptions.WriteBinColorsToFile(hFile);

		/////////////////////////////////////////////
		// Global Info Settings
		/////////////////////////////////////////////
		/*
		if(ptGlobalInfoSummarized->isOn())
			strValue="summarized";
		else if(ptGlobalInfoDetailed->isOn())
			strValue="detailed";
		fprintf(hFile,"  gexOptions('global_info','detail_level','%s');\n",strValue.toLatin1().constData());
		*/


		// Message to console saying 'success loading options'
		fprintf(hFile,"\n\n  sysLog('* Galaxy Examinator Options loaded! *');\n");

		// Close function
		fprintf(hFile,"}\n");
		fprintf(hFile,"\n");
	}
}


///////////////////////////////////////////////////////////
// Build contextMenu to expand or collapse option list
///////////////////////////////////////////////////////////
void GexOptions::onContextMenuRequested(Q3ListViewItem* /*item*/)
{
	QMenu menu(this);
	// If list has at least one child
	if (ListView->firstChild())
	{
		// If an item is selected and if it has child
		if (ListView->selectedItem() && ListView->selectedItem()->firstChild())
		{
			// If item is expanded
			if(ListView->selectedItem()->isOpen())
			{
				menu.addAction("Collapse selected", this, SLOT(onCollapseChildren()));
			}
			// If item is collapsed
			else
			{
				menu.addAction("Expand selected", this, SLOT(onExpandChildren()));
			}
		}
		menu.addAction("Collapse All", this, SLOT(onCollapseAll()));
		menu.addAction("Expand All", this, SLOT(onExpandAll()));
	}
	// show and exec menu
	menu.exec(QCursor::pos());
}

///////////////////////////////////////////////////////////
// Launch recursiv function to expand all children on each
// root item of the ListView
///////////////////////////////////////////////////////////
void GexOptions::onExpandAll()
{
	Q3ListViewItem *item = ListView->firstChild();
	while (item)
	{
		expand(item);
		item = item->nextSibling();
	}
}

///////////////////////////////////////////////////////////
// Launch recursiv function to collapse all children on each
// root item of the ListView
///////////////////////////////////////////////////////////
void GexOptions::onCollapseAll()
{
	Q3ListViewItem *item = ListView->firstChild();
	while (item)
	{
		collapse(item);
		item = item->nextSibling();
	}
}

///////////////////////////////////////////////////////////
// Launch recursiv function to expand all children of the
// selected item
///////////////////////////////////////////////////////////
void GexOptions::onExpandChildren()
{
	expand(ListView->selectedItem());
}

///////////////////////////////////////////////////////////
// Launch recursiv function to collapse all children of the
// selected item
///////////////////////////////////////////////////////////
void GexOptions::onCollapseChildren()
{
	collapse(ListView->selectedItem());
}

///////////////////////////////////////////////////////////
// Recursiv function to expand all children of the
// selected items, using in depth search first
///////////////////////////////////////////////////////////
void GexOptions::collapse(Q3ListViewItem* item)
{
	Q3ListViewItem *itemChild = item->firstChild();
	while(itemChild)
	{
		if (itemChild->firstChild())
			collapse(itemChild);
		itemChild = itemChild->nextSibling();
	}
	item->setOpen(false);
}

///////////////////////////////////////////////////////////
// Recursiv function to collapse all children of the
// selected items, using in depth search first
///////////////////////////////////////////////////////////
void GexOptions::expand(Q3ListViewItem* item)
{
	Q3ListViewItem *itemChild = item->firstChild();
	while(itemChild)
	{
		if (itemChild->firstChild())
			expand(itemChild);
		itemChild = itemChild->nextSibling();
	}
	item->setOpen(true);
}

