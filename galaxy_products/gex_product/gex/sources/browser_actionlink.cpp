///////////////////////////////////////////////////////////
// User has clicked a hyperlink that includes the '_gex_'
// string in it...this is not a real hyperlink, but a
// tag for requesting GEX to perform some task!
///////////////////////////////////////////////////////////
#include <gqtl_log.h>

#include "assistant_flying.h"
#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "tb_toolbox.h"	// Examinator ToolBox
#include "gex_constants.h"
#include "gex_web_browser.h"
#include "product_info.h"
#include "browser_dialog.h"
#include "engine.h"
#include <QWebFrame>
#include <QWebElement>

extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// List of Hyperlink comments (to show in the status bar); and Top bar tab to
// display if the link is clicked or selected using forward/back or combo-box.
typedef struct
{
    const char *	strLinkName;
    const char *	strComment;
    const char *	szTopBarTab;
    bool			bShowHtmlBrowser;
} sHyperlinkComments;

sHyperlinkComments scomment[]=
{
    // HTML page name, page tooltip, associated TAB on top navigation bar
    // Top navigation bar
    {GEX_HTMLPAGE_HELP,			"",	GEX_BROWSER_HELP_TOPLINK,true},
    {GEX_HTMLPAGE_HELP_PRO,		"",	GEX_BROWSER_HELP_TOPLINK,true},
    {GEX_HTMLPAGE_HOME,			"",	GEX_BROWSER_HOME_TOPLINK,true},
    {GEX_HTMLPAGE_HOME_PRO,		"",	GEXTB_BROWSER_HOME_TOPLINK,true},
    {GEX_HTMLPAGE_HOME_DB,		"",	GEX_BROWSER_HOME_TOPLINK,true},
    {GEX_HTMLPAGE_HOME_DB_PRO,	"",	GEX_BROWSER_HOME_TOPLINK,true},
    {GEX_BROWSER_HOME_TOPLINK,GEX_BROWSER_HOME_HELP,GEX_BROWSER_HOME_TOPLINK,true},
    {GEX_BROWSER_FILES_TOPLINK,GEX_BROWSER_FILES_HELP,GEX_BROWSER_FILES_TOPLINK,false},
    {GEX_BROWSER_SETTINGS_TOPLINK,GEX_BROWSER_SETTINGS_HELP,GEX_BROWSER_SETTINGS_TOPLINK,false},
    {GEX_BROWSER_REPORT_TOPLINK,GEX_BROWSER_REPORT_HELP,GEX_BROWSER_REPORT_TOPLINK,true},
    {GEX_BROWSER_OPTIONS_TOPLINK,GEX_BROWSER_OPTIONS_HELP,GEX_BROWSER_OPTIONS_TOPLINK,false},
    {GEX_BROWSER_HELP_TOPLINK,GEX_BROWSER_HELP_HELP,GEX_BROWSER_HELP_TOPLINK,true},
    {GEX_BROWSER_HELP_LINK,GEX_BROWSER_HELP_HELP,GEX_BROWSER_HELP_TOPLINK,true},
    {GEX_BROWSER_SCRIPTING_TOPLINK,GEX_BROWSER_SCRIPTING_HELP,NULL,false},

    // Wizards
    {GEX_BROWSER_ASSISTANT_WIZARD,GEX_BROWSER_ASSISTANT_HELP,GEX_BROWSER_ELSE_TOPLINK,true},
    {GEX_BROWSER_ONEFILE_WIZARD,GEX_BROWSER_ONEFILE_HELP,GEX_BROWSER_FILES_TOPLINK,false},
    {GEXTB_BROWSER_STDFCHECK_PAT,GEXTB_BROWSER_STDFCHECK_PAT_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEX_BROWSER_CMPFILES_WIZARD,GEX_BROWSER_CMPFILES_HELP,GEX_BROWSER_FILES_TOPLINK,false},
    {GEX_BROWSER_FT_PAT_FILES_WIZARD,GEX_BROWSER_FT_PAT_FILES_HELP,GEX_BROWSER_FILES_TOPLINK,false},
    {GEX_BROWSER_ADDFILES_WIZARD,GEX_BROWSER_ADDFILES_HELP,GEX_BROWSER_FILES_TOPLINK,false},
    {GEX_BROWSER_MIXFILES_WIZARD,GEX_BROWSER_MIXFILES_HELP,GEX_BROWSER_FILES_TOPLINK,false},
    {GEX_BROWSER_ONEQUERY_WIZARD,GEX_BROWSER_ONEQUERY_HELP,GEX_BROWSER_FILES_TOPLINK,false},

    {GEX_BROWSER_CMPQUERY_WIZARD,GEX_BROWSER_CMPQUERY_HELP,GEX_BROWSER_FILES_TOPLINK,false},
    {GEX_BROWSER_MIXQUERY_WIZARD,GEX_BROWSER_MIXQUERY_HELP,GEX_BROWSER_FILES_TOPLINK,false},

    {GEX_BROWSER_SQL_QUERY_WIZARD,GEX_BROWSER_SQL_QUERY_HELP,GEX_BROWSER_FILES_TOPLINK,false},
    {GEX_BROWSER_CHAR_QUERY_WIZARD,GEX_BROWSER_CHAR_QUERY_HELP,GEX_BROWSER_FILES_TOPLINK,false},
    {GEX_BROWSER_REPORTSCENTER,GEX_BROWSER_REPORTSCENTER_HELP,GEX_BROWSER_FILES_TOPLINK,false},
    {GEX_BROWSER_SHIFT_WIZARD,GEX_BROWSER_SHIFT_HELP,GEX_BROWSER_FILES_TOPLINK,false},

    {GEX_BROWSER_DBADMIN_WIZARD,GEX_BROWSER_DBADMIN_HELP,GEX_BROWSER_ELSE_TOPLINK,false},
    {GEX_BROWSER_GALAXYWEB_LINK,GEX_BROWSER_GALAXYWEB_HELP,GEX_BROWSER_ELSE_TOPLINK,true},
    {GEX_BROWSER_SCRIPTING_LINK,GEX_BROWSER_SCRIPTING_HELP,GEX_BROWSER_ELSE_TOPLINK,false},
    {GEX_BROWSER_DRILL_WHATIF,GEX_BROWSER_DRILL_HELP,GEX_BROWSER_ELSE_TOPLINK,false},
    {GEX_BROWSER_DRILL_3D,GEX_BROWSER_DRILL_HELP,GEX_BROWSER_ELSE_TOPLINK,false},
    {GEX_BROWSER_DRILL_CHART,GEX_BROWSER_DRILL_HELP,GEX_BROWSER_ELSE_TOPLINK,false},
    {GEX_BROWSER_DRILL_TABLE,GEX_BROWSER_DRILL_HELP,GEX_BROWSER_ELSE_TOPLINK,false},
    {GEX_BROWSER_OPENREPORT_WIZARD,GEX_BROWSER_REPORT_HELP,NULL,false},
    {GEX_BROWSER_BINCOLORS,GEX_BROWSER_BINCOLORS_HELP,GEX_BROWSER_ELSE_TOPLINK,true},
    {GEX_BROWSER_PATREPORT,GEX_BROWSER_PATREPORT_HELP,GEX_BROWSER_ELSE_TOPLINK,true},
    {GEX_BROWSER_SBL,GEX_BROWSER_SBL_HELP,GEX_BROWSER_ELSE_TOPLINK,true},
    {GEX_BROWSER_EXPORT_WAFMAP,GEX_BROWSER_EXPORT_WAFMAP_HELP,GEX_BROWSER_ELSE_TOPLINK,true},


    // ExaminatorMonitoring
    {GEX_HTMLPAGE_HOME_MONITORING,"",GEXMO_BROWSER_HOME_TOPLINK,true},
    {GEXMO_BROWSER_HOME_TOPLINK,GEXMO_BROWSER_HOME_HELP,NULL,true},
    {GEXMO_BROWSER_TASKS_TOPLINK,GEXMO_BROWSER_TASKS_HELP,NULL,false},
    {GEXMO_BROWSER_TASKS_LINK,GEXMO_BROWSER_TASKS_HELP,GEXMO_BROWSER_TASKS_TOPLINK,false},
    {GEXMO_BROWSER_HISTORY_TOPLINK,GEXMO_BROWSER_HISTORY_HELP,NULL,false},
    {GEXMO_BROWSER_HISTORY_LINK,GEXMO_BROWSER_HISTORY_HELP,GEXMO_BROWSER_HISTORY_TOPLINK,false},
    {GEXMO_BROWSER_OPTIONS_TOPLINK,GEX_BROWSER_OPTIONS_HELP,NULL,false},
    {GEXMO_BROWSER_HELP_TOPLINK,GEX_BROWSER_HELP_HELP,NULL,true},
    {"top_mo_exit.htm","Exit",NULL,true},

    // GTM
    {GEX_HTMLPAGE_HOME_GTM,"",GEXGTM_BROWSER_HOME_TOPLINK,true},
    {GEXGTM_BROWSER_HOME_TOPLINK,GEXGTM_BROWSER_HOME_HELP,NULL,true},
    {GEXGTM_BROWSER_TESTERS_TOPLINK,GEXGTM_BROWSER_TESTERS_HELP,NULL,false},
    {GEXGTM_BROWSER_TESTERS_LINK,GEXGTM_BROWSER_TESTERS_HELP,GEXGTM_BROWSER_TESTERS_TOPLINK,false},
    {GEXGTM_BROWSER_OPTIONS_TOPLINK,GEX_BROWSER_OPTIONS_HELP,NULL,false},
    {GEXGTM_BROWSER_HELP_TOPLINK,GEX_BROWSER_HELP_HELP,NULL,true},

    // Examinator ToolBox
    {GEX_HTMLPAGE_HOME_TOOLBOX,"",GEXTB_BROWSER_HOME_TOPLINK,true},
    {GEX_HTMLPAGE_HOME_ROOT_TOOLBOX,GEXTB_BROWSER_HOME_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_HOME_TOPLINK,GEXTB_BROWSER_HOME_HELP,NULL,true},
    {GEXTB_BROWSER_MAKE_STDF,GEXTB_BROWSER_MAKE_STDF_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_DUMP_STDF,GEXTB_BROWSER_DUMP_STDF_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_MERGERETEST,GEXTB_BROWSER_MERGERETEST_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_FTPAT,GEXTB_BROWSER_FTPAT_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_PAT,GEXTB_BROWSER_PAT_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
#ifdef GCORE15334
    {GEXTB_BROWSER_FT_PAT_LIMITS,GEXTB_BROWSER_FT_PAT_LIMITS_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_WS_PAT_LIMITS,GEXTB_BROWSER_WS_PAT_LIMITS_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
#endif
    {GEXTB_BROWSER_EDIT_PAT,GEXTB_BROWSER_EDIT_PAT_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_PROGRAM_PAT,GEXTB_BROWSER_PROGRAM_PAT_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_RELOADDATA_PAT,GEXTB_BROWSER_RELOADDATA_PAT_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_RELEASE_PAT,GEXTB_BROWSER_RELEASE_PAT_HELP,GEXMO_BROWSER_HOME_TOPLINK,true},
    {GEXTB_BROWSER_MAKE_CSV,GEXTB_BROWSER_MAKE_CSV_HELP,GEXTB_BROWSER_ELSE_TOPLINK,false},
    {GEXTB_BROWSER_EDIT_CSV,GEXTB_BROWSER_EDIT_CSV_HELP,GEXTB_BROWSER_ELSE_TOPLINK,false},
    {GEXTB_BROWSER_EDIT_DBKEYS,GEXTB_BROWSER_EDIT_DBKEYS_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_CHECK_REGEXP,GEXTB_BROWSER_CHECK_REGEXP_HELP,GEXTB_BROWSER_ELSE_TOPLINK,true},
    {GEXTB_BROWSER_HELP_TOPLINK,GEX_BROWSER_HELP_HELP,NULL,true},


    // Options
    {GEX_BROWSER_OPTIONS_WIZARD,"",GEX_BROWSER_OPTIONS_TOPLINK,false},

    // Save settings into script (link from report HTML page)
    {GEX_BROWSER_SAVE,		GEX_BROWSER_SAVE_HELP,		NULL,true},

    // Settings
    {GEX_BROWSER_SETTINGS,GEX_BROWSER_SETTINGSHLP,GEX_BROWSER_SETTINGS_TOPLINK,false},

    // Debug links
    {GEXDEBUG_BROWSER_FUNCTION1,GEXDEBUG_BROWSER_FUNCTION1_HELP,GEX_BROWSER_ELSE_TOPLINK,true},
    {GEXDEBUG_BROWSER_FUNCTION2,GEXDEBUG_BROWSER_FUNCTION2_HELP,GEX_BROWSER_ELSE_TOPLINK,true},
    {GEXDEBUG_BROWSER_FUNCTION3,GEXDEBUG_BROWSER_FUNCTION3_HELP,GEX_BROWSER_ELSE_TOPLINK,true},
    {GEXDEBUG_BROWSER_FUNCTION4,GEXDEBUG_BROWSER_FUNCTION4_HELP,GEX_BROWSER_ELSE_TOPLINK,true},
    {GEXDEBUG_BROWSER_FUNCTION5,GEXDEBUG_BROWSER_FUNCTION5_HELP,GEX_BROWSER_ELSE_TOPLINK,true},
    {GEXDEBUG_BROWSER_VISHAY_DB_CLEANUP,GEXDEBUG_BROWSER_VISHAY_DB_CLEANUP_HELP,GEX_BROWSER_ELSE_TOPLINK,true},

    // Help User Manual
    {GEX_HELP_USER_MANUAL, GEX_HELP_USER_MANUAL_HELP, NULL, true},

    {GEX_EXPORT_FTR_CORRELATION_REPORT, GEX_HELP_USER_FTR_CORRELATION, NULL, true},
    {GEX_BROWSER_DRILL_ALL,GEX_BROWSER_DRILL_HELP,GEX_BROWSER_ELSE_TOPLINK,false},

    {GEXCOPYRIGHT_LICENSE_LINK,GEXCOPYRIGHT_LICENSE_HELP,NULL,false},
    {EXAM_QUICKSTART_LINK, EXAM_QUICKSTART_HELP, NULL, false},
    {EXAM_INSTALL_LINK, EXAM_INSTALL_HELP, NULL, false},
    {USER_LICENSE_LINK, USER_LICENSE_HELP, NULL, false},
    {RH_LINK, RH_HELP, NULL, false},
    {TER_INSTALL_LINK, TER_INSTALL_HELP, NULL, false},
    {TER_RH_LINK, TER_RH_HELP, NULL, false},
    {YM_QUICKSTART_LINK, YM_QUICKSTART_HELP, NULL, false},
    {YM_INSTALL_LINK, YM_INSTALL_HELP, NULL, false},
    {GEX_HTMLPAGE_HELP,			"",	GEX_BROWSER_HELP_TOPLINK,true},
    {GEXDBTDR_HTMLPAGE_TOPBAR,			"",	GEX_BROWSER_HOME_TOPLINK, true},
    {GEX_HTMLPAGE_DB_NOSETTINGS,			"",	GEX_BROWSER_SETTINGS_TOPLINK, true},
    {GEX_BROWSER_DBREPORT_LINK,			"",	GEX_BROWSER_REPORT_TOPLINK, true},
    {GEX_BROWSER_HELPPRO_LINK, GEX_BROWSER_HELP_HELP,GEX_BROWSER_HELP_TOPLINK,true},
    {NULL,NULL,NULL,false}
};

#if defined unix || __MACH__
  // HP-UX, Solaris compiler needs to see class name...
  #define GEX_ADDRESS_OF(function) (&GexMainwindow::function)
#else
  // Windows + Solaris
  #define GEX_ADDRESS_OF(function) (&function)
#endif

// MUST be in the EXACT SAME ORDER AS 'scomment' structure
ptActions GexMainwindow::pActionFunctions[]=
{
    // Top navigation bar
    GEX_ADDRESS_OF(GexMainwindow::Wizard_Assistant),	// just to maintain the correct offsets
    GEX_ADDRESS_OF(GexMainwindow::Wizard_Assistant),	// just to maintain the correct offsets
    GEX_ADDRESS_OF(GexMainwindow::DoNothing),	// just to maintain the correct offsets
    GEX_ADDRESS_OF(GexMainwindow::HomeLink),	// Examinator - Standard
    GEX_ADDRESS_OF(GexMainwindow::HomeLink),	// Examinator - Pro edition
    GEX_ADDRESS_OF(GexMainwindow::HomeLink),	// Examinator DB - Standard
    GEX_ADDRESS_OF(GexMainwindow::HomeLink),	// Examinator DB - Pro edition
    GEX_ADDRESS_OF(GexMainwindow::DataLink),
    GEX_ADDRESS_OF(GexMainwindow::ViewSettings),
    GEX_ADDRESS_OF(GexMainwindow::ReportLink),
    GEX_ADDRESS_OF(GexMainwindow::ViewOptions),
    GEX_ADDRESS_OF(GexMainwindow::HelpLink),
    GEX_ADDRESS_OF(GexMainwindow::HelpLink),
    GEX_ADDRESS_OF(GexMainwindow::DoNothing),

    // Wizards
    GEX_ADDRESS_OF(GexMainwindow::Wizard_Assistant),
    GEX_ADDRESS_OF(GexMainwindow::Wizard_OneFile_Page1),
    GEX_ADDRESS_OF(GexMainwindow::Wizard_OneFile_FileAudit),// Check STDF file contents to identify any discrepency (can be used to see what affects PAT (eg: test number duplication, etc))
    GEX_ADDRESS_OF(GexMainwindow::Wizard_CompareFile_Page1),
    GEX_ADDRESS_OF(GexMainwindow::Wizard_FT_PAT_FileAnalysis),
    GEX_ADDRESS_OF(GexMainwindow::Wizard_MergeFile_Page1),
    GEX_ADDRESS_OF(GexMainwindow::Wizard_MixFile_Page1),
    GEX_ADDRESS_OF(GexMainwindow::Wizard_OneQuery_Page1),	// Query single Dataset

    GEX_ADDRESS_OF(GexMainwindow::Wizard_CmpQuery_Page1),	// Query Compare Datasets
    GEX_ADDRESS_OF(GexMainwindow::Wizard_MergeQuery_Page1),	// Query Merge Datasets

    GEX_ADDRESS_OF(GexMainwindow::Wizard_SqlQuery),         // SQLProduction reports/Templates & SQL GUI
    GEX_ADDRESS_OF(GexMainwindow::Wizard_CharQuery),		// SQLProduction reports/Templates & SQL GUI
    GEX_ADDRESS_OF(GexMainwindow::ShowReportsCenter),		//
    GEX_ADDRESS_OF(GexMainwindow::Wizard_ShiftAnalysis),    // Shift analysis

    GEX_ADDRESS_OF(GexMainwindow::Wizard_AdminGui),     	// Database Admin
    GEX_ADDRESS_OF(GexMainwindow::DoNothing),				// NOK: should point to www.mentor.com!
    GEX_ADDRESS_OF(GexMainwindow::Wizard_Scripting),
    GEX_ADDRESS_OF(GexMainwindow::Wizard_DrillWhatif),		// GuardBanding
    GEX_ADDRESS_OF(GexMainwindow::Wizard_Drill3D),			// All OpenGL views.
    GEX_ADDRESS_OF(GexMainwindow::Wizard_DrillChart),		// Interactive 2D drill
    GEX_ADDRESS_OF(GexMainwindow::Wizard_DrillTable),		// Interactive Table drill
    GEX_ADDRESS_OF(GexMainwindow::Wizard_OpenReport),		// Reopen a non-HTML report (launches Word, or PDF reader, or CSV,...)
    GEX_ADDRESS_OF(GexMainwindow::Wizard_BinColors),		// Edit Binning colors
    GEX_ADDRESS_OF(GexMainwindow::Wizard_SavePatReport),	// Export PAT report to disk...
    GEX_ADDRESS_OF(GexMainwindow::Wizard_ExportSBL),		// Export SBL (Statistical Bin Limits)
    GEX_ADDRESS_OF(GexMainwindow::Wizard_ExportWafermap),	// Export wafermap to file (TSMC, G85 semi,...)

    // ExaminatorMonitoring
    GEX_ADDRESS_OF(GexMainwindow::DoNothing),				// MO_HOME
    GEX_ADDRESS_OF(GexMainwindow::HomeLink),				// 'Home' tab
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexMo_Tasks),		// 'Tasks' tab
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexMo_Tasks),		// 'Task' hyperlink on Browser pages.
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexMo_History),	// 'History' tab.
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexMo_History),	// 'History' hyperlink on Browser pages.
    GEX_ADDRESS_OF(GexMainwindow::ViewOptions),
    GEX_ADDRESS_OF(GexMainwindow::HelpLink),
    GEX_ADDRESS_OF(GexMainwindow::closeGex),						// 'Exit' button

    // GTM
    GEX_ADDRESS_OF(GexMainwindow::DoNothing),				// GTM_HOME
    GEX_ADDRESS_OF(GexMainwindow::HomeLink),				// 'Home' tab
    GEX_ADDRESS_OF(GexMainwindow::ShowGtmWidget),
    GEX_ADDRESS_OF(GexMainwindow::ShowGtmWidget),
    GEX_ADDRESS_OF(GexMainwindow::ViewOptions),
    GEX_ADDRESS_OF(GexMainwindow::HelpLink),

    // Examinator ToolBox
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb),                // TB_HOME
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb),                // Wizard to display Toolbox home page
    GEX_ADDRESS_OF(GexMainwindow::HomeLink),                    // 'Home' tab
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_STDF),           // Convert file to STDF
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_DumpSTDF),       // Dump STDF Records into a ASCII file
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_MergeRetest),    // Merge Test + Retest data into one new file!
    GEX_ADDRESS_OF(GexMainwindow::Wizard_FT_PAT),               // Clean file from outliers: FT PAT removal algorithm.
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_PAT),            // Clean file from outliers: WS PAT removal algorithm.
#ifdef GCORE15334
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_FT_PAT_Limits),  // Create the FT PAT limits file./
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_WS_PAT_Limits),  // Create the WS PAT limits file./
#endif
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_EditPAT_Limits), // Edit the Static PAT limits file.
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_ProgramPAT),     // Modify test program to be PAT enabled.
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_ReloadDatasetPAT),// Force reprocessing a data file (used when batch PAT processing done from GUI and user want to navigate into interactive mode into one of the files
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_ReleasePAT),     // Release PAT file to production folder
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_CSV),            // Convert file to CSV
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_Edit),           // Edit Data file in Spreadsheet table.
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_DbKeys),         // Edit DB Keys files
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_RegExp),         // Edit RegExp
    GEX_ADDRESS_OF(GexMainwindow::HelpLink),

    // Options
    //GEX_ADDRESS_OF(GexMainwindow::Wizard_Options),
    GEX_ADDRESS_OF(GexMainwindow::ShowOptionsCenter),

    // Save settings into script (link from report HTML page)
    GEX_ADDRESS_OF(GexMainwindow::SaveReportScript),

    // Settings
    GEX_ADDRESS_OF(GexMainwindow::Wizard_Settings),

    // Debug functions
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexDebug_Function1),			// GEX Debug: Function1
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexDebug_Function2),			// GEX Debug: Function2
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexDebug_Function3),			// GEX Debug: Function3
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexDebug_Function4),			// GEX Debug: Function4
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexDebug_Function5),			// GEX Debug: Function5
    GEX_ADDRESS_OF(GexMainwindow::Wizard_GexDebug_Vishay_Db_Cleanup),	// GEX Debug: Vishay GEXDB cleanup

    // Help user manual
    GEX_ADDRESS_OF(GexMainwindow::OnContextualHelp),					// Open the local user manual

    //Export FTR into csv
    GEX_ADDRESS_OF(GexMainwindow::exportFTRCorrelationReport),
    GEX_ADDRESS_OF(GexMainwindow::Wizard_DrillAll),

    //Help license
    GEX_ADDRESS_OF(GexMainwindow::OpenCopyrightLicense),
    GEX_ADDRESS_OF(GexMainwindow::OpenExamQuickStart),
    GEX_ADDRESS_OF(GexMainwindow::OpenExamInstall),
    GEX_ADDRESS_OF(GexMainwindow::OpenUserLicense),
    GEX_ADDRESS_OF(GexMainwindow::OpenRH),
    GEX_ADDRESS_OF(GexMainwindow::OpenTerInstall),
    GEX_ADDRESS_OF(GexMainwindow::OpenTerRH),
    GEX_ADDRESS_OF(GexMainwindow::OpenYMQuickStart),
    GEX_ADDRESS_OF(GexMainwindow::OpenYMInstall),
    GEX_ADDRESS_OF(GexMainwindow::HelpLink),
    GEX_ADDRESS_OF(GexMainwindow::HomeLink),
    GEX_ADDRESS_OF(GexMainwindow::ViewSettings),
    GEX_ADDRESS_OF(GexMainwindow::ReportLink),
    GEX_ADDRESS_OF(GexMainwindow::HelpLink),
    NULL,
};

///////////////////////////////////////////////////////////
// Parse hyperlink to display the correct message in
// the status bar...either the hpyerlink path, or
// the comment if it is a GEX action link
///////////////////////////////////////////////////////////
void GexMainwindow::OnGexLinkHovered(const QString& strLink, const QString& /*strTitle*/, const QString& /*strTextContent*/)
{
    OnGexLinkHovered(strLink);
}

void GexMainwindow::OnHeaderPageLoaded()
{
  // GCORE-7369 - find another way to load page for teradyne
    if (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus() ||
        GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eTerOEM)
    {
      QString teradyne_header_html =
        navigationSkinDir() + "pages/" + GEX_HTMLPAGE_TOPBAR_TERADYNE;

      pageHeader->setSource( QUrl::fromLocalFile( teradyne_header_html ) );
    }
}

void GexMainwindow::OnHeaderLinkClicked(const QUrl & urlClicked)
{
    if (m_pWebViewReport)
        m_pWebViewReport->linkClicked(urlClicked);

    if (pageHeader)
        pageHeader->setSource(urlClicked);
}

///////////////////////////////////////////////////////////
// Parse hyperlink to display the correct message in
// the status bar...either the hpyerlink path, or
// the comment if it is a GEX action link
///////////////////////////////////////////////////////////
void GexMainwindow::OnGexLinkHovered(const QString &strHyperlink)
{
    // Extract LAST section of the link that may include a GEX command
    QString gexAction = strHyperlink.section(GEX_BROWSER_ACTIONLINK,-1);

    if(gexAction.isEmpty() == false)
    {
        // Hyperlink is a special string for triggering GEX action
        int iIndex=0;
        // Loop until we find the link in our list
        while(scomment[iIndex].strLinkName != NULL)
        {
            if(gexAction == scomment[iIndex].strLinkName)
            {
                // Link found: display its comment in the Status bar
                statusBar()->showMessage(scomment[iIndex].strComment);
                return;
            }
            iIndex++;
        };
    }

    // Link not found..just show it 'as-is'
    statusBar()->showMessage(strHyperlink);
}

///////////////////////////////////////////////////////////
// Find the matching link name into the table, return its
// index...used to call the right function, or display the
// right tooltip
///////////////////////////////////////////////////////////
int GexMainwindow::LookupLinkName(const QString& strLink)
{
    // Extract the command line (string after '?????_gex_')
    QString gexAction;
    QString gexPageName;

    // If this is a regular link...then let's call the default method
    // Check if email link...
    gexAction = strLink.section(GEX_BROWSER_ACTIONLINK,-1);

    // If action includes a parameter (#xxx'), remove the parameter for now!
    gexAction = gexAction.section('#',0,0);

    if(gexAction.isEmpty())
        return -1;

    //GCORE-9822 some page urls miss an index simply because they don't have '_gex' in their path
    QString strLinkFileNameOnly = QFileInfo(strLink).fileName();

    int iIndex=0;
    while(scomment[iIndex].strLinkName != NULL)
    {
        // When link entry found, return its index.
        gexPageName = scomment[iIndex].strLinkName;
        if((gexAction == gexPageName.section(GEX_BROWSER_ACTIONLINK,-1)) ||
                (gexAction == gexPageName)				 ||
                strLinkFileNameOnly == gexPageName			 ||
                strLinkFileNameOnly == gexPageName.section(GEX_BROWSER_ACTIONLINK,-1))
            return iIndex;
        // Current entry wasn't matching our link...check next one.
        iIndex++;
    };

    // Entry not found.
    return -1;
}


///////////////////////////////////////////////////////////
// Update Top window content according to the link shown
// in the browser window.
///////////////////////////////////////////////////////////
void GexMainwindow::UpdateTopNavigationBar(int iIndex)
{
    // Check if lookup table index, and if so, check if we have a specific
    // HTML page to refresh in the top navigation bar
    if((iIndex <0) || (scomment[iIndex].szTopBarTab == NULL))
        return;

    // Build correct page name : _Gex_ + page name
    QString strTopBarPage = navigationSkinDir() + "pages/" + GEX_BROWSER_ACTIONLINK;
    QString strTopBar = scomment[iIndex].szTopBarTab;

    // Check if the Navigation Bar is the standard or custom (Gex-ToolBox, or Gex-Monitoring,...)
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        if(strTopBar == GEX_BROWSER_ELSE_TOPLINK)
            strTopBar = GEXMO_BROWSER_ELSE_TOPLINK;
    }
    else
    {
        // Ignore the Toolbox navigation link if in fact it is Gex-Pro running
        if(strTopBar == GEXTB_BROWSER_ELSE_TOPLINK)
            strTopBar = GEX_BROWSER_ELSE_TOPLINK;
        if(strTopBar == GEXTB_BROWSER_HOME_TOPLINK)
            strTopBar = GEX_BROWSER_HOME_TOPLINK;
    }

    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM() && (strTopBar == GEX_BROWSER_ELSE_TOPLINK))
        strTopBar = GEXGTM_BROWSER_ELSE_TOPLINK;

    // Use correct Navigation HTML bar
    strTopBarPage += strTopBar;

    QUrl urlTopPage = QUrl::fromLocalFile(strTopBarPage);

    // If page selected is already the default, ignore action!
    if(pageHeader->source() == urlTopPage)
        return;

    // GCORE-7369 - modify the structor of the page to include some OEM infos
    ReloadHeaderPage( urlTopPage );

    // Keep focus on the browser page, so that accelerators always concern its window only.
    m_pWebViewReport->widget()->setFocus();
}

void GexMainwindow::ReloadHeaderPage( const QUrl &url )
{
  if (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus() ||
      GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GS::LPPlugin::LicenseProvider::eTerOEM)
  {
    QStringList new_header_content;
    QFile header_file( url.toLocalFile() );
    if( header_file.open( QIODevice::ReadOnly ) )
    {
      QTextStream stream( &header_file );
      QRegExp img_pattern
        ( "(^\\s*<img\\s*id=\"logoTopRight\".*src=\").*(\"\\s*>\\s*$)" );

      while( ! stream.atEnd() )
      {
        QString line = stream.readLine();
        if ( img_pattern.exactMatch( line ) )
        {
          line.replace( img_pattern, "\\1../images/logo_teradyne_w.png\\2" );
        }
        new_header_content.append( line );
      }

      header_file.close();
    }
    // Load the new page
    pageHeader->setHtml( new_header_content.join( '\n' ) );
  }
  else
    pageHeader->setSource( url );
}

void GexMainwindow::onBrowserActionLink(const GexWebBrowserAction& lBA)   //const QString & strLink, int nIndex)
{
    GSLOG(7, QString("on BrowserActionLink %1 %2 from url %3")
             .arg(lBA.link()).arg(lBA.index()).arg(lBA.m_DesiredUrl.toString()).toLatin1().data() );

    QString strLink=lBA.link();
    int nIndex=lBA.index();
    if (nIndex != -1)
    {
        GSLOG(7, QString("Running action index %1").arg(nIndex).toLatin1().constData() );
        // Link found. Update top navigation bar if needed
        UpdateTopNavigationBar(nIndex);
        // Launch the function associated with this link.
        (this->*pActionFunctions[nIndex])();

        if (pActionFunctions[nIndex] == GEX_ADDRESS_OF(GexMainwindow::Wizard_GexTb_ReloadDatasetPAT))
        {
            QString lDesiredPage= lBA.m_DesiredUrl.toString().section("#_gex_", 0,0);
            GSLOG(6, QString("Reload Dataset PAT action requested, jumping to the desired page '%1'...")
                   .arg(lDesiredPage).toLatin1().data() );
            // The link can be very dirty as : ..../page.htm#anchor#_gex_tb_patreload.htm#2
            LoadUrl( lBA.m_DesiredUrl.toLocalFile().section("#_gex_", 0,0) );
        }
    }
    else
    {
        if (strLink.startsWith("http://"))
        {
            // Load the new page
            m_pWebViewReport->loadUrl(QUrl(strLink));
            // Keep trace of this link
            AddNewUrl(QUrl(strLink));
            // Keep focus on the browser page, so that accelerators always concern its window only.
            m_pWebViewReport->widget()->setFocus();
        }
        else
            GSLOG(6, QString("Link unsupported : %1").arg(strLink).toLatin1().data() );

    }
    GSLOG(7, "on Browser Action Link ok");
}
