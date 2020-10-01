///////////////////////////////////////////////////////////
// Constants used in the GEX project
///////////////////////////////////////////////////////////
#ifndef GEX_CONSTANTS
#define	GEX_CONSTANTS	1

//////////////////////////////////////////////////////////
// Structures declared in gex_constants.cpp
///////////////////////////////////////////////////////////
#ifndef GEX_CONSTANTS_CPP
///////////////////////////////////////////////////////////
// GENERAL PURPOSE
///////////////////////////////////////////////////////////
extern int FiltersMapping[];
extern const char* HistogramItems[];
extern const char* PearsonItems[];
extern const char* TrendItems[];
extern const char* ProbabilityPlotItems[];
extern const char* BoxPlotExItems[];
extern const char* ScatterItems[];
extern const char* BoxPlotItems[];
extern const char* MultiChartItems[];
extern const char* Charac1ChartItems[];
extern const char* DalalogItems[];
extern const char* ProductionYieldItems[];
extern const char* ProcessPartsItems[];
extern const char* gexFileProcessPartsItems[];
extern const char* gexLabelFileProcessPartsItems[];
extern const char* gexTimePeriodChoices[];
extern const char* gexLabelTimePeriodChoices[];
extern const char* gexLabelHousekeepingPeriodChoices[];
extern const char* gexFilterChoices[];
extern const char* gexLabelFilterChoices[];
extern const char* FunctionalReportItems[];
extern const char* FTRCorrelationReportItems[];

#ifndef gex_max
#define gex_max(a,b)	((a) < (b) ? (b) :(a))
#endif
#ifndef gex_min
#define gex_min(a,b)	((a) > (b) ? (b) :(a))
#endif
#ifndef gex_maxAbs
#define gex_maxAbs(a,b)	((fabs(a)) < (fabs(b)) ? (b) :(a))
#endif
#ifndef gex_minAbs
#define gex_minAbs(a,b)	((fabs(a)) > (fabs(b)) ? (b) :(a))
#endif

///////////////////////////////////////////////////////////
// PAT keyword arrays.
///////////////////////////////////////////////////////////
#endif

#define	GEX_C_DOUBLE_NAN	(double) 1.7e308	// NaN value for Galaxy software

#define C_STDF_COPYRIGHT	"Copyright (c) 2000-2016 Quantix"
#define C_HTML_FOOTER		"<br><br><br><hr>\n<font color=\"#006699\">Report created with: %s - www.mentor.com</font>\n"
#define GEX_EVALCOPY_SHORT_NOTICE	"Unregistered Quantix software"

// Computer screen size
#define	GEX_SCREENSIZE_SMALL		1
#define	GEX_SCREENSIZE_MEDIUM		2
#define	GEX_SCREENSIZE_LARGE		3

// GEX possible running modes
#define	GEX_RUNNINGMODE_STANDALONE	1		// Standalone: needs local license file
#define	GEX_RUNNINGMODE_CLIENT		2		// Client...needs to connact to GEX-LM server
#define	GEX_RUNNINGMODE_EVALUATION	3		// 4 days evaluation

// GEX Package Edition types: Standard, Advanced,...
#define	GEX_EDITION_STD				1		// Standard Examinator
#define	GEX_EDITION_ADV				2		// Advanced Examinator

// GEX Client status monitored on socket
#define	GEX_CLIENT_SOCKET_IDLE		1		// GEX client not currently in talks with server
#define	GEX_CLIENT_TRY_CONNECT		2		// GEX trying to connect to server
#define	GEX_CLIENT_GET_LICENSE		3		// GEX waiting a license acknowledge from server.
#define	GEX_CLIENT_REGISTERED		4		// GEX license granted. GEX can run!
#define	GEX_CLIENT_ALLUSED			5		// All GEX license used...need to prompt user if want to try again or abort...
#define	GEX_CLIENT_ALIVE_TIMEOUT	60		// GEX tells every minute Server that it is still alive.
#define	GEX_CLIENT_TRY_CONNECT_TIMEOUT	15	// Connection timeout after 15secs.

// GEX Assistant Module selected
#define	GEX_MODULE_INSTANT_REPORT	1		// Menu selected is for Instant-Report generation
#define	GEX_MODULE_DATA_MINING		2		// Menu selected is for data mining


///////////////////////////////////////////////////////////
// Flags to keep track of wizards selected
///////////////////////////////////////////////////////////
#define	GEX_ONEFILE_WIZARD          1
#define	GEX_CMPFILES_WIZARD         2
#define	GEX_ADDFILES_WIZARD         3
#define GEX_MIXFILES_WIZARD         4
#define GEX_ONEQUERY_WIZARD         5
#define GEX_CMPQUERY_WIZARD         6
#define GEX_SQL_QUERY_WIZARD        7
#define GEX_MIXQUERY_WIZARD         8
#define GEX_JASPER_WIZARD           9
#define GEX_CHAR_QUERY_WIZARD       10
#define GEX_FT_PAT_FILES_WIZARD     11
#define GEX_SHIFT_WIZARD            12

#define GEX_WIZARD_MIN              1
#define GEX_WIZARD_MAX              12

///////////////////////////////////////////////////////////
// Flags to keep track of wizard page selected
///////////////////////////////////////////////////////////
#define	GEX_OPTIONS						1
#define	GEX_BROWSER						2
#define GEX_SCRIPTING					3
#define	GEX_WIZARD_SETTINGS				100
#define	GEX_ONEFILE_WIZARD_P1			101
#define	GEX_CMPFILES_WIZARD_P1			201
#define	GEX_ADDFILES_WIZARD_P1			301
#define GEX_MIXFILES_WIZARD_P1			401
#define GEX_ONEQUERY_WIZARD_P1			501
#define GEX_CMPQUERY_WIZARD_P1			601
#define GEX_MIXQUERY_WIZARD_P1			603
#define GEX_ENTERPRISE_SQL_P1			602
#define GEX_REPORTS_CENTER				604
#define GEX_OPTIONS_CENTER				605
#define GEX_LOGS_CENTER					606
#define GEX_JAVASCRIPT_CENTER           607
#define GEX_SQLBROWSER                  608
#define GEX_GTM_TESTERS_WIDGET			609
#define GEX_FT_PAT_FILES_WIDGET         610
#define GEX_SHIFT_WIZARD_P1             611

#define GEX_DATABASE_ADMIN				701
#define GEX_DRILL_WHATIF_WIZARD_P1		801
#define	GEX_CHART_WIZARD_P1				802
#define	GEX_TABLE_WIZARD_P1				803
#define GEX_DRILL_3D_WIZARD_P1			901
#define GEXMO_TASKS						902
#define GEXMO_HISTORY					903
#define GEXTB_CONVERT_STDF_WIZARD_P1	1000
#define GEXTB_CONVERT_CSV_WIZARD_P1		1001
#define GEXTB_CONVERT_PAT_WIZARD_P1		1002
#define GEXTB_EDIT_CSV_WIZARD_P1		1003
#define GEXTB_EDIT_PAT_WIZARD_P1		1004
#define GEX_WS_PAT_PROCESS_WIZARD_P1	1005
#define GEX_FT_PAT_PROCESS_WIZARD_P1	1006


///////////////////////////////////////////////////////////
// #define used for Database access purpose
///////////////////////////////////////////////////////////
#define GEX_DATABASE_FOLDER				"/databases/"
#define GEX_DATABASE_YIELDMAN			"/yieldman/"
#define GEX_DATABASE_REPORTS			"/reports/"
#define GEX_DATABASE_REPORTS_MO			"/reports.mo/"	// Examinator monitoring creates reports in this folder!
#define GEX_DATABASE_WEB_EXCHANGE		"/user/"		// ExaminatorWeb: folder used to communicate between client & server, hold user specific HTML pages, etc...
#define GEX_DATABASE_WEB_IMPORT			"/import/"		// ExaminatorWeb: folder where files to import manually have to be located
#define GEX_DATABASE_FILTER_FOLDER		"/.filters/"	// Database sub-folder that holds tables of filters known values. MUST start with a '.'
#define GEX_DATABASE_TEMP_FOLDER		"/.temp/"		// Database sub-folder that holds temporary files
#define GEX_DATABASE_TEMP_FTP_FOLDER	"/.temp_ftp/"	// Database sub-folder that holds temporary FTP files

#define GEXWEB_RPTENTRY_DEFINITION		"/.gexweb_rptentry"	// ExaminatorWeb report definition file. In 'reports' folder
#define GEXWEB_STATUS_FILE				"gexweb_status"	// ExaminatorWeb status file (includes 1 line, format: <keyword> <parameters>
#define GEX_HTMLPAGE_WEB_REPORTS		"gexweb_reports.asp"	// ExaminatorWeb: HTML page created to list all existing reports + admin functions on them (delete, rename, etc.)
#define GEX_HTMLPAGE_WEB_DATABASES		"gexweb_databases.asp"	// ExaminatorWeb: HTML page created to list all existing user databases + admin functions on them (delete, import files, etc.)
#define GEX_HTMLPAGE_WEB_HOME			"gexweb_home.htm"	    // ExaminatorWeb home page
#define GEX_HTMLPAGE_WEB_FAILIMPORT		"gexweb_failimport.htm"	// ExaminatorDB: failed importing data files.
#define GEXMO_STATUS_FOLDER				".monitor"			// Folder under 'Databases' that includes all internal files used by Gex-MO to build report.
#define GEXMO_PATMAN_FOLDER				".patman"			// Folder under 'Databases' that includes all internal files used by Gex-MO to handle the PATMAN (Outlier removal) task.
#define GEXMO_FILES_IMPORTED			"processed.dat"		// Holds the list of files aready imported into a database (used when need to keep track of files imported)
#define GEXMO_AUTOREPORT_FOLDER			"examinator_monitoring"		// sub-Folder where to create the report on the intranet.
#define GEXMO_AUTOREPORT_HISTORY		"history"		// sub-Folder where to create the History report on the intranet.
#define GEXMO_AUTOREPORT_PRODUCTS		"products"		// sub-Folder where to create the per-product report on the intranet.
#define GEXMO_AUTOREPORT_TESTERS		"testers"		// sub-Folder where to create the per-tester report on the intranet.
#define GEXMO_AUTOREPORT_EMAILS			".emails"		// sub-Folder where to create the emails notifications

///////////////////////////////////////////////////////////
// Relative path of HTML pages from application path
///////////////////////////////////////////////////////////
#define GEX_HTML_FOLDER				"/html/pages/"
#define GEX_IMAGE_FOLDER			"/html/images/"
#define GEX_HELP_FOLDER				"/help/pages/"
#define GEXMO_AUTOREPORT_IMAGES_SRC	"/help/autoreport/"	// Where images for autoreport are stored under Examinator Binary application path (source). to be copied to intranet path...
#define GEXMO_AUTOREPORT_IMAGES_DEST ".images"			// Sub-folder in Intranet that holds .PNG images used by the reports.
#define GEX_HELPIMAGES_FOLDER		"/help/images/"
#define GEX_NEPTUS_FOLDER			"/help/neptus/"
#define GEX_SCRIPT_FOLDER			"/help/script/"
#define GEX_FLYNEPTUS_FOLDER		"/help/context/"
#define GEX_HTMLPAGE_NOREPORT		"_gexstd_report.htm"		// Examinator: No Report available yet...
#define GEX_HTMLPAGE_DB_NOREPORT	"_gexstd_db_report.htm"		// ExaminatorDB: No Report available yet...
#define GEX_HTMLPAGE_NOREPORT_LTXC  "_gexstd_report_ltxc.htm"	// Examinator: No Report available yet...
#define GEX_HTMLPAGE_DB_EMPTYQUERY	"_gexstd_db_empty.htm"		// ExaminatorDB: Query return is Empty!
#define GEX_HTMLPAGE_DB_NODATABASE	"_gexstd_db_data.htm"		// ExaminatorDB: No Database available yet...
#define GEX_HTMLPAGE_DB_NOSETTINGS	"_gexstd_db_settings.htm"	// ExaminatorDB: No Settings available yet...
#define GEX_HTMLPAGE_WEB_NOREPORT	"_gexstd_web_report.htm"	// ExaminatorDB: No Report available yet...
#define GEX_HTMLPAGE_WEB_EMPTYQUERY "_gexstd_web_empty.htm"	// ExaminatorDB: Query return is Empty!
#define GEX_HTMLPAGE_WEB_NODATABASE	"_gexstd_web_data.htm"		// ExaminatorDB: No Database available yet...
#define GEX_HTMLPAGE_WEB_NOSETTINGS	"_gexstd_web_settings.htm"	// ExaminatorDB: No Settings available yet...
#define GEX_HTMLPAGE_WEB_IMPORT		"_gexstd_web_import.htm"	// ExaminatorDB: Success importing data files.
#define	GEX_HTMLPAGE_CSVFILE		"_gexstd_csvfile.htm"
#define	GEX_HTMLPAGE_WORDFILE		"_gexstd_wordfile.htm"
#define	GEX_HTMLPAGE_ODTFILE		"_gexstd_odtfile.htm"
#define	GEX_HTMLPAGE_PPTFILE		"_gexstd_pptfile.htm"
#define	GEX_HTMLPAGE_PDFFILE		"_gexstd_pdffile.htm"
#define GEX_HTMLPAGE_INTERACTIVEONLY "_gexstd_interactive.htm"
#define GEX_HTMLPAGE_NOSETTINGS         "_gexstd_settings.htm"		// Examinator: No Settings available yet...
#define GEX_HTMLPAGE_NOSETTINGS_LTXC    "_gexstd_settings_ltxc.htm"	// Examinator: No Settings available yet...
#define	GEX_HTMLPAGE_HELP			"_gexstd_help.htm"
#define GEX_HTMLPAGE_HELP_PRO		"_gexpro_help.htm"
#define	GEX_HTMLPAGE_TOPBAR			"_gex_top_home.htm"
#define	GEX_HTMLPAGE_TOPBAR_TERADYNE		"_gex_top_home_teradyne.htm"
#define	GEXMO_HTMLPAGE_TOPBAR		"_gex_top_mo_home.htm"
#define	GEXTB_HTMLPAGE_TOPBAR		"_gex_top_tb_home.htm"		// Examinator-ToolBox home page.
#define	GEXGTM_HTMLPAGE_TOPBAR		"_gex_top_gtm_home.htm"
#define GEXDBTDR_HTMLPAGE_TOPBAR	"_gex_db_tdr_home.htm"
#define	GEX_HTMLPAGE_BOOTOMBAR		"_gex_bottom.htm"
#define	GEX_HTMLPAGE_HLP_SETTINGS	"_gexstd_manual_settings.htm"
#define	GEX_HTMLPAGE_HLP_OPTIONS	"_gexstd_manual_options.htm"
#define	GEX_HTMLPAGE_HLP_SCRIPTING	"_gexstd_manual_script.htm"
#define	GEX_HTMLPAGE_HLP_DRILLING	"_gexstd_manual_drill.htm"
#define	GEX_HTMLPAGE_HLP_ONEWIZ		"_gexstd_help_one_wizard.htm"
#define	GEX_HTMLPAGE_HLP_CMPWIZ		"_gexstd_help_cmp_wizard.htm"
#define	GEX_HTMLPAGE_HLP_ADDWIZ		"_gexstd_help_add_wizard.htm"
#define	GEX_HTMLPAGE_HLP_MIXWIZ		"_gexstd_help_mix_wizard.htm"
#define	GEX_HTMLPAGE_HLP_FOOTER		"_gexstd_help_footer.htm"
#define GEX_HTMLPAGE_HLP_STARTUP	"_gexstd_startup.htm"
#define GEX_HTMLPAGE_HLP_STARTUP_DB	"_gexstd_db_startup.htm"
#define GEX_HTMLPAGE_HLP_STARTUP_DBPAT		"_gexstd_dbpat_startup.htm"
#define GEX_HTMLPAGE_HLP_STARTUP_WEB		"_gexstd_web_startup.htm"
#define GEX_HTMLPAGE_HLP_STARTUP_MONITORING "_gexstd_mo_startup.htm"
#define GEX_HTMLPAGE_HLP_STARTUP_PATMAN		"_gexstd_patman_startup.htm"
#define GEX_HTMLPAGE_HLP_STARTUP_TOOLBOX	"_gexstd_tb_startup.htm"
#define GEX_HTMLPAGE_HLP_STARTUP_GTM        "_gexstd_gtm_startup.htm"

// YIELD123
#define GEXY123_HTMLPAGE_NOREPORT		"_gexstd_yield123_report.htm"		// Report available yet...
#define GEXY123_HTMLPAGE_NOSETTINGS		"_gexstd_yield123_settings.htm"		// No Settings available yet...
#define	GEXY123_HTMLPAGE_WORDFILE		"_gexstd_yield123_wordfile.htm"		// Report created in WORD format
#define	GEXY123_HTMLPAGE_PPTFILE		"_gexstd_yield123_pptfile.htm"		// Report created in PowerPoint format
#define	GEXY123_HTMLPAGE_PDFFILE		"_gexstd_yield123_pdffile.htm"		// Report created in PDF format
#define GEXY123_HTMLPAGE_EMPTYQUERY		"_gexstd_yield123_empty.htm"		// Query return is Empty!
#define GEXY123_HTMLPAGE_NODATABASE		"_gexstd_yield123_data.htm"			// Database available yet...

#define GEX_EXPORT_FTR_CORRELATION_REPORT	"export_ftr_correlation.html"
#define GEX_HELP_USER_FTR_CORRELATION       "Export FTR correlation data to CSV"

#define	 COPYRIGHT_LICENSE_PDF_FILE		"CopyrightAndLegalNotices.pdf"		// Copyright license Pdf
#define COPYRIGHT_LICENSE_PDF_FILE		"CopyrightAndLegalNotices.pdf"		// Copyright license Pdf
#define EXAM_QUICK_START_FILE "qntx_exam_gs.pdf"
#define EXAM_INSTALL_FILE "qntx_exam_install.pdf"
#define USER_LICENSE_FILE "qntx_exam_lic_user.pdf"
#define RH_FILE "qntx_rh.pdf"
#define TER_INSTALL_FILE "qntx_ter_install.pdf"
#define TER_RH_FILE "qntx_ter_rh.pdf"
#define YM_QUICK_START_FILE "qntx_ym_gs.pdf"
#define YM_INSTALL_FILE "qntx_ym_install.pdf"

// **** ExaminatorMonitoring ****
// HOME Link
#define	GEXMO_BROWSER_HOME_TOPLINK			"top_mo_home.htm"
#define	GEXMO_BROWSER_HOME_HELP				"Reloads home page."

// Definitions for: TASKS Link
#define	GEXMO_BROWSER_TASKS_TOPLINK			"top_mo_tasks.htm"
#define	GEXMO_BROWSER_TASKS_LINK			"mo_tasks.htm"
#define	GEXMO_BROWSER_TASKS_HELP			"View/Edit the tasks scheduled."

// Definitions for: History Link
#define	GEXMO_BROWSER_HISTORY_TOPLINK		"top_mo_history.htm"
#define	GEXMO_BROWSER_HISTORY_LINK			"mo_history.htm"
#define	GEXMO_BROWSER_HISTORY_HELP			"Display Tasks history log"

// Definitions for: Options Link
#define	GEXMO_BROWSER_OPTIONS_TOPLINK		"top_mo_options.htm"

#define	GEXMO_BROWSER_HELP_TOPLINK			"top_mo_help.htm"

#define	GEXMO_BROWSER_ELSE_TOPLINK			"top_mo_else.htm"
#define	GEXMO_BROWSER_ELSE_HELP				"Quantix pages."

// **** Examinator ToolBox ****
// HOME Link
#define	GEXTB_BROWSER_HOME_TOPLINK			"top_tb_home.htm"
#define	GEXTB_BROWSER_HOME_HELP				"Examinator ToolBox home page."

#define	GEXTB_BROWSER_HELP_TOPLINK			"top_tb_help.htm"

#define	GEXTB_BROWSER_ELSE_TOPLINK			"top_tb_else.htm"
#define	GEXTB_BROWSER_ELSE_HELP				"Examinator ToolBox pages."

// Definitions for: Wizard to convert files to STDF
#define	GEXTB_BROWSER_MAKE_STDF				"tb_stdf.htm"
#define	GEXTB_BROWSER_MAKE_STDF_HELP		"Convert semiconductor data to STDF file!"

// Definitions for: Wizard to Dump STDF structures
#define	GEXTB_BROWSER_DUMP_STDF				"tb_stdfdump.htm"
#define	GEXTB_BROWSER_DUMP_STDF_HELP		"Dump STDF records into an ASCII file!"

// Definitions for: Wizard to Merge Test & Retest data files into one new file!
#define	GEXTB_BROWSER_MERGERETEST			"tb_retest.htm"
#define	GEXTB_BROWSER_MERGERETEST_HELP		"Smart merge of Test and Retest data...and know your real yield!"

// Definitions for: Wizard to Remove outliers & bin relevant parts that hold outliers at Wafer Sort
#define	GEXTB_BROWSER_PAT					"tb_pat.htm"
#define	GEXTB_BROWSER_PAT_HELP				"Identify parts with outliers & bin them!"

// Definitions for: Wizard to Remove outliers & bin relevant parts that hold outliers at Final Test
#define	GEXTB_BROWSER_FTPAT					"tb_ft_pat.htm"
#define	GEXTB_BROWSER_FTPAT_HELP			"Identify parts with outliers & bin them!"

// Definitions for: Wizard to Build the Static PAT test limits file
#define	GEXTB_BROWSER_WS_PAT_LIMITS			"tb_ws_patlimits.htm"
#define	GEXTB_BROWSER_WS_PAT_LIMITS_HELP	"Create the Outlier configuration file (including Static PAT limits, Bins,...)"

// Definitions for: Wizard to Build the Static PAT test limits file
#define	GEXTB_BROWSER_FT_PAT_LIMITS			"tb_ft_patlimits.htm"
#define	GEXTB_BROWSER_FT_PAT_LIMITS_HELP	"Create the Outlier configuration file (including Static PAT limits, Bins,...)"

// Definitions for: Wizard to Edit existing PAT File
#define	GEXTB_BROWSER_EDIT_PAT				"tb_edit_pat.htm"
#define	GEXTB_BROWSER_EDIT_PAT_HELP			"Edit the Outlier configuration file"

// Definitions for: Wizard to Release a PAT File to the production folder
#define	GEXTB_BROWSER_RELEASE_PAT			"tb_releasepat.htm"
#define	GEXTB_BROWSER_RELEASE_PAT_HELP		"Release Outlier configuration file to PAT-Man production folder"

#define	GEXTB_BROWSER_STDFCHECK_PAT			"tb_patstdfcheck.htm"
#define	GEXTB_BROWSER_STDFCHECK_PAT_HELP	"Check STDF PAT compliancy: Check records, test numbers, etc..."

#define	GEXTB_BROWSER_PROGRAM_PAT			"tb_patprogram.htm"
#define	GEXTB_BROWSER_PROGRAM_PAT_HELP		"Modify your test program to be PAT real-time ready!"

#define	GEXTB_BROWSER_RELOADDATA_PAT			"tb_patreload.htm"
#define	GEXTB_BROWSER_RELOADDATA_PAT_HELP		"PAT Reload dataset"

// Definitions for: Wizard to convert files to CSV
#define	GEXTB_BROWSER_MAKE_CSV				"tb_excel.htm"
#define	GEXTB_BROWSER_MAKE_CSV_HELP			"Convert semiconductor data to spreadsheet CSV file!"

// Definitions for: Wizard to Edit data file (spreadsheet)
#define	GEXTB_BROWSER_EDIT_CSV				"tb_edit.htm"
#define	GEXTB_BROWSER_EDIT_CSV_HELP			"Edit semiconductor data file!"

// Definitions for: Wizard to Edit DB keys files
#define	GEXTB_BROWSER_EDIT_DBKEYS			"tb_dbkeys.htm"
#define	GEXTB_BROWSER_EDIT_DBKEYS_HELP		"Create/Edit a database keys file"

// Definitions for: Wizard to Edit DB keys files
#define	GEXTB_BROWSER_CHECK_REGEXP			"tb_regexp.htm"
#define	GEXTB_BROWSER_CHECK_REGEXP_HELP		"Check Regular Expressions"

// HTML Report Toolbar created: GEX action Bookmarks
#define GEX_BROWSER_ACT_SELECT				"select"
#define GEX_BROWSER_ACT_DRILL				"drill"
#define GEX_BROWSER_ACT_ZOOM_IN				"zoom_in"
#define GEX_BROWSER_ACT_ZOOM_OUT			"zoom_out"

#define GEX_BROWSER_ACT_DRILL_CHART			"drill_chart"
#define GEX_BROWSER_ACT_DRILL_TABLE			"drill_table"
#define GEX_BROWSER_ACT_WHATIF				"what_if"

#define GEX_BROWSER_ACT_ENTERPRISE			"enterprise"
#define GEX_BROWSER_ACT_ADV_ENTERPRISE		"adv_enterprise_report"

#define GEX_BROWSER_ACT_OPEN_URL			"_gex_openurl"

// Definition for: link to Galaxy web site
#define GEX_BROWSER_GALAXYWEB_LINK			"www_galaxysemi_com.htm"
#define GEX_BROWSER_GALAXYWEB_HELP			"Visit Quantix web site!"

// Definitions for: Debug functions
#define	GEXDEBUG_BROWSER_FUNCTION1				"debug_function1.htm"
#define	GEXDEBUG_BROWSER_FUNCTION1_HELP			"Debug: function 1!"
#define	GEXDEBUG_BROWSER_FUNCTION2				"debug_function2.htm"
#define	GEXDEBUG_BROWSER_FUNCTION2_HELP			"Debug: function 2!"
#define	GEXDEBUG_BROWSER_FUNCTION3				"debug_function3.htm"
#define	GEXDEBUG_BROWSER_FUNCTION3_HELP			"Debug: function 3!"
#define	GEXDEBUG_BROWSER_FUNCTION4				"debug_function4.htm"
#define	GEXDEBUG_BROWSER_FUNCTION4_HELP			"Debug: function 4!"
#define	GEXDEBUG_BROWSER_FUNCTION5				"debug_function5.htm"
#define	GEXDEBUG_BROWSER_FUNCTION5_HELP			"Debug: function 5!"
#define	GEXDEBUG_BROWSER_VISHAY_DB_CLEANUP		"vishay_db_cleanup.htm"
#define	GEXDEBUG_BROWSER_VISHAY_DB_CLEANUP_HELP	"Vishay: GEXDB cleanup!"


//#########################################################
// ALL GEX Settings dialog box related IDs
//#########################################################

// Report format combo-box items
// Note1: The last items in this list MUST ALWAYS be the one removed by Yield123 (Interactive, CSV).
#define	GEX_SETTINGS_OUTPUT_HTML		0				// HTML muti_pages output
#define	GEX_SETTINGS_OUTPUT_WORD		1				// MS-Word output
#define	GEX_SETTINGS_OUTPUT_PPT			2				// Powerpoint slides output
#define	GEX_SETTINGS_OUTPUT_PDF			3				// PDF output
#define	GEX_SETTINGS_OUTPUT_CSV			4				// CSV output
#define	GEX_SETTINGS_OUTPUT_INTERACTIVE	5				// No report, Interactive mode only.
#define	GEX_SETTINGS_OUTPUT_ODT         6				// ODT
#define	GEX_SETTINGS_OUTPUT_TOTAL_ITEMS		(GEX_SETTINGS_OUTPUT_ODT+1)	// Tells how many OUTPUT formats are available

///////////////////////////////////////////////////////////
// User Settings
///////////////////////////////////////////////////////////
// Combo box selection offset for: Process (parts to process)
#define	GEX_PROCESSPART_ALL			0	// Process all parts
#define	GEX_PROCESSPART_EXPARTLIST	1	// All parts except...
#define	GEX_PROCESSPART_GOOD		2	// Good parts only
#define	GEX_PROCESSPART_FAIL		3	// Fail parts only
#define	GEX_PROCESSPART_PARTLIST	4	// Specific parts
#define	GEX_PROCESSPART_SBINLIST	5	// Specific Bins (Soft bins)
#define	GEX_PROCESSPART_EXSBINLIST	6	// All bins except  (Soft bins)
#define	GEX_PROCESSPART_HBINLIST	7	// Specific Bins (Hard bins)
#define	GEX_PROCESSPART_EXHBINLIST	8	// All bins except  (Hard bins)
#define	GEX_PROCESSPART_ODD			9	// Odd parts (1,3,5,...)
#define	GEX_PROCESSPART_EVEN		10	// Even parts (2,4,6,...)
#define GEX_PROCESSPART_FIRSTINSTANCE	11	// First Test instances (ignore retests)
#define GEX_PROCESSPART_LASTINSTANCE	12	// Last Test instances (only last in retests)
#define GEX_PROCESSPART_PARTSINSIDE		13	// Parts inside...
#define GEX_PROCESSPART_PARTSOUTSIDE	14	// Parts outside...
#define GEX_PROCESSPART_NO_SAMPLES		15	// Bin data only (ignore samples)

// Combo box selection offset for: Sites (sites to process)
#define	GEX_PROCESSSITE_ALL			0	// Process all sites
#define	GEX_PROCESSSITE_SITELIST	1	// Process specific sites

// Limits type settings
#define GEX_LIMITS_USEDLIMITS_SPECIFANY 0
#define GEX_LIMITS_USEDLIMITS_STDONLY 1
#define GEX_LIMITS_USEDLIMITS_SPECONLY 2
#define GEX_LIMITS_USEDLIMITS_MULTIIFANY 3
// Limits ref settings
#define GEX_LIMITS_REF_LIMITS_OLDEST 0
#define GEX_LIMITS_REF_LIMITS_LATEST 1


// Histograms/ Adv_histogram combos settings include:
#define	GEX_HISTOGRAM_DISABLED		0
#define	GEX_HISTOGRAM_OVERLIMITS	1
#define	GEX_HISTOGRAM_CUMULLIMITS	2
#define	GEX_HISTOGRAM_OVERDATA		3
#define	GEX_HISTOGRAM_CUMULDATA		4
#define	GEX_HISTOGRAM_DATALIMITS	5	// Adjust viewport to include both data and limits

// Histogram 2nd combo: settings
#define	GEX_HISTOGRAM_ALL			0		// Histogram ALL tests
#define	GEX_HISTOGRAM_LIST			1		// Histogram List of tests
#define	GEX_HISTOGRAM_TOP_N_FAIL_TESTS	2

// Define AdvancedHistogram size (number of cells to draw AdvancedHistogram)
#define	TEST_ADVHISTOSIZE	40

// Test Statistics combo settings include:
#define	GEX_STATISTICS_DISABLED		0
#define	GEX_STATISTICS_ALL			1
#define	GEX_STATISTICS_FAIL			2
#define	GEX_STATISTICS_OUTLIERS		3
#define	GEX_STATISTICS_LIST			4
#define	GEX_STATISTICS_BADCP		5
#define	GEX_STATISTICS_BADCPK		6
#define	GEX_STATISTICS_TOP_N_FAILTESTS	7

// Wafermap combo settings include:
#define	GEX_WAFMAP_DISABLED				0
#define	GEX_WAFMAP_SOFTBIN				1
#define	GEX_WAFMAP_HARDBIN				2
#define	GEX_WAFMAP_TESTOVERLIMITS		3
#define	GEX_WAFMAP_TESTOVERDATA			4
#define	GEX_WAFMAP_TEST_PASSFAIL		5
// 6 = Text Separator
#define	GEX_WAFMAP_STACK_SOFTBIN		7
#define	GEX_WAFMAP_STACK_HARDBIN		8
#define	GEX_WAFMAP_STACK_TESTOVERLIMITS	9
#define	GEX_WAFMAP_STACK_TESTOVERDATA	10
#define	GEX_WAFMAP_STACK_TEST_PASSFAIL	11
// 12 = Text Separator
#define	GEX_WAFMAP_ZONAL_SOFTBIN		13
#define	GEX_WAFMAP_ZONAL_HARDBIN		14

// Wafermap 2nd combo: settings
#define	GEX_WAFMAP_ALL					0		// Wafermap ALL tests
#define	GEX_WAFMAP_LIST					1		// Wafermap List of tests
#define	GEX_WAFMAP_TOP_N_FAILTESTS		2		//

// Combo box selection offset for: Advanced Reports
#define GEX_ADV_DISABLED 0

#define GEX_ADV_SEPARATOR 1 // Text Separator
#define GEX_ADV_HISTOGRAM 2
#define GEX_ADV_TREND 3
#define GEX_ADV_CORRELATION 4
#define GEX_ADV_PROBABILITY_PLOT 5
#define GEX_ADV_BOXPLOT 6
#define GEX_ADV_MULTICHART 7

#define GEX_ADV_CANDLE_MEANRANGE 8
#define GEX_ADV_DATALOG 9
#define GEX_ADV_GUARDBANDING 10
#define GEX_ADV_PEARSON 11  // Test-to-Test Correlation (Pearson's  correlation check)
#define GEX_ADV_PAT_TRACEABILITY 12  // PAT results coded into STDF DTR records
#define GEX_ADV_PROD_YIELD 13  // Production reports (yield trends)

#define GEX_ADV_TEMPLATE 14  // Custom report - template based
#define GEX_ADV_GO_DIAGNOSTICS 15  // Optimizer Diagnostics

#define GEX_ADV_SHIFT 16  // Shift analysis

#define GEX_ADV_FUNCTIONAL 17
#define GEX_ADV_FTR_CORRELATION 18

#define GEX_ADV_OUTLIER_REMOVAL		100		// Oultier Removal (ToolBox report triggered) (keep at High ID)
#define GEX_ADV_REPORT_BUILDER      101

// 200 = Charac
#define GEX_ADV_CHARAC_BOXWHISKER_CHART	200
#define GEX_ADV_CHARAC_LINE_CHART       201

// If Advanced report = Histogram, combo settings include:
// ID0=	"------ Histogram: Test Results ------");
#define	GEX_ADV_HISTOGRAM_OVERLIMITS	1
#define	GEX_ADV_HISTOGRAM_CUMULLIMITS	2
#define	GEX_ADV_HISTOGRAM_OVERDATA		3
#define	GEX_ADV_HISTOGRAM_CUMULDATA		4
#define	GEX_ADV_HISTOGRAM_DATALIMITS	5	// Adjust viewport to include both data and limits

// If Advanced report = Datalog, combo settings include:
// ID0=	"------ Datalog: Test Results ------");
#define	GEX_ADV_DATALOG_ALL			1
#define	GEX_ADV_DATALOG_FAIL		2
#define GEX_ADV_DATALOG_OUTLIER		3
#define	GEX_ADV_DATALOG_LIST		4
#define	GEX_ADV_DATALOG_RAWDATA		5

// If Advanced report = Trend, combo settings include:
// ID0= "------ Trend: Test Results ------"
#define	GEX_ADV_TREND_OVERLIMITS		1
#define	GEX_ADV_TREND_OVERDATA			2
#define	GEX_ADV_TREND_DATALIMITS		3
// ID3= "------ Trend: Test1-Test2  ------"
#define GEX_ADV_TREND_DIFFERENCE		5
// ID5= "------ Trend: Statistics ------"
#define GEX_ADV_TREND_AGGREGATE_MEAN	7
#define GEX_ADV_TREND_AGGREGATE_SIGMA	8
#define GEX_ADV_TREND_AGGREGATE_CP		9
#define GEX_ADV_TREND_AGGREGATE_CPK		10
// ID10= "------ Trend: Binning/Yield ------"
#define GEX_ADV_TREND_SOFTBIN_SBLOTS	12
#define GEX_ADV_TREND_SOFTBIN_PARTS		13
#define GEX_ADV_TREND_HARDBIN_SBLOTS	14
#define GEX_ADV_TREND_HARDBIN_PARTS		15
#define GEX_ADV_TREND_SOFTBIN_ROLLING	16
#define GEX_ADV_TREND_HARDBIN_ROLLING	17

// If Advanced report = Correlation, combo settings include:
// ID0=	"------ Scatter: Test Results ------");
#define	GEX_ADV_CORR_OVERLIMITS		1
#define	GEX_ADV_CORR_OVERDATA		2
#define	GEX_ADV_CORR_DATALIMITS		3

// If Advanced report = Boxplot, combo settings include:
// ID0=	"------ Boxplot: Test Results ------");
#define	GEX_ADV_BOXPLOT_OVERLIMITS		1
#define	GEX_ADV_BOXPLOT_OVERDATA		2
#define	GEX_ADV_BOXPLOT_DATALIMITS		3

// If Advanced report = Probability plot, combo settings include:
// ID0=	"------ Probability plot: Test Results ------");
#define	GEX_ADV_PROBPLOT_OVERLIMITS		1
#define	GEX_ADV_PROBPLOT_OVERDATA		2
#define	GEX_ADV_PROBPLOT_DATALIMITS		3

//
#define	GEX_ADV_PEARSON_OVERLIMITS		1
#define	GEX_ADV_PEARSON_OVERDATA		2
#define	GEX_ADV_PEARSON_DATALIMITS		3


// If Advanced report = Multi-chart, combo settings include:
// ID0=	"------ Multi-chart: Test Results ------");
#define	GEX_ADV_MULTICHART_OVERLIMITS	1
#define	GEX_ADV_MULTICHART_OVERDATA		2
#define	GEX_ADV_MULTICHART_DATALIMITS	3

// Characterization viewport
#define	GEX_ADV_CHARAC_CHART_OVERLIMITS     1
#define	GEX_ADV_CHARAC_CHART_OVERDATA		2
#define	GEX_ADV_CHARAC_CHART_DATALIMITS     3

// If Advanced report = Production report (yield), combo settings include:
// ID0=	"------ Yield reports ------");
#define	GEX_ADV_PRODYIELD_SBLOT		1
#define	GEX_ADV_PRODYIELD_LOT		2
#define GEX_ADV_PRODYIELD_GROUP		3
#define GEX_ADV_PRODYIELD_DAY		4
#define	GEX_ADV_PRODYIELD_WEEK		5
#define	GEX_ADV_PRODYIELD_MONTH		6

// If Advanced report = Functional, combo settings include:
// ID0=	"------ Functional: Test Results ------");
#define	GEX_ADV_FUNCTIONAL_CYCL_CNT 	1
#define	GEX_ADV_FUNCTIONAL_REL_VAD      2

// If Advanced report = FTR correlation, combo settings include:
// ID0=	"------ Functional: Test Results ------");
#define	GEX_ADV_FTR_CORRELATION_ALL 	1
#define	GEX_ADV_FTR_CORRELATION_LIST    2

// Boxplot 2nd combo: settings
// ID0=	"------ BoxPlot: Test Results ------");
#define	GEX_ADV_ALL					1		// Chart ALL tests
#define	GEX_ADV_LIST				2		// Chart List of tests
#define	GEX_ADV_TOPNFAILTESTS		3		// Chart top N failed tests

// Advanced reports
#define	GEX_ADV_ALLTESTS			0		// ALL tests
#define	GEX_ADV_TESTSLIST			1		// List of tests
#define	GEX_ADV_TOP_N_FAILTESTS		2		// Top N failing tests

// If Advanced report = Production report
#define	GEX_ADV_PROD_YIELD_SOFTBIN	0		// Yield report over SOFT bin
#define	GEX_ADV_PROD_YIELD_HARDBIN	1		// Yield report over HARD bin

// If Advanced report = Production report
#define	GEX_ADV_PROD_YIELDAXIS_0_100		0	// Yield Axis is 0% to 100%
#define	GEX_ADV_PROD_YIELDAXIS_0_MAX		1	// Yield Axis is 0% to Max yield
#define	GEX_ADV_PROD_YIELDAXIS_MIN_100		2	// Yield Axis is Min yield to 100%
#define	GEX_ADV_PROD_YIELDAXIS_MIN_MAX		3	// Yield Axis is Min yield to Max yield

// If Advanced report = Production report
#define	GEX_ADV_PROD_VOLUMEAXIS_0_MAX		0	// Volume Axis is 0 to Max volume
#define	GEX_ADV_PROD_VOLUMEAXIS_0_CUSTOM	1	// Volume Axis is 0 to Custom volume
#define	GEX_ADV_PROD_VOLUMEAXIS_MIN_MAX		2	// Volume Axis is Min volume to Max volume
#define	GEX_ADV_PROD_VOLUMEAXIS_MIN_CUSTOM	3	// Volume Axis is Min volume to Custom volume

// Boxplot charting modes:
// case 3648 : options freed from this enum !!
// always used in GEX !!!!
#define	GEX_BOXPLOTTYPE_LIMITS		1		// Charting: Over test limits
#define	GEX_BOXPLOTTYPE_RANGE		2		// Charting: Over data range
#define	GEX_BOXPLOTTYPE_ADAPTIVE	3		// Adaptive: chart over limits if one dataset, over range otherwise.

// ProbabilityPlot charting Mode
#define GEX_PROBPLOTTYPE_LIMITS		1		// Charting: Over test limits
#define GEX_PROBPLOTTYPE_RANGE		2		// Charting: Over data range
#define GEX_PROBPLOTTYPE_ADAPTIVE	3		// Adaptive: chart over limits if one dataset, over range otherwise.

// DRILL types allowed
#define GEX_DRILL_GUARDBANDING		0	// Not for interactive OpenGL GUI
#define	GEX_DRILL_WAFERPROBE		1
#define	GEX_DRILL_PACKAGED			2
#define	GEX_DRILL_HISTOGRAM			3
#define	GEX_DRILL_TREND				4
#define	GEX_DRILL_CORRELATION		5

// Flags telling which parameters changed in what-if scenario
#define GEX_DRILL_LL				1
#define GEX_DRILL_HL				2
#define GEX_DRILL_CP				4
#define GEX_DRILL_CPK				8
#define GEX_DRILL_YIELD				16
#define GEX_DRILL_RESULT			32
#define GEX_DRILL_NULL_SIGMA        64

// YIELD 123 Settings
#define	GEXY123_MISSION_YIELD			0		// Mission: Yield Improvement (focus on production failures)
#define	GEXY123_MISSION_QUALITY			1		// Mission: Quality Assessement (focus production anomalies)
#define	GEXY123_MISSION_REPEATABILITY	2		// Mission: Repeatability (loops on a device)



// Flags
///////////////////////////////////////////////////////////
// Options Settings
///////////////////////////////////////////////////////////

// Database options
#define	GEX_OPTION_RDB_EXTRACT_PARAMETERS_ALL		1		// RDB extraction: by default, extract all parameters
#define	GEX_OPTION_RDB_EXTRACT_PARAMETERS_NONE		2		// RDB extraction: by default, extract no parameter

// Output Format: Bits flags
#define	GEX_OPTION_OUTPUT_HTML				1				// HTML muti_pages output
#define	GEX_OPTION_OUTPUT_CSV				2				// CSV output
#define	GEX_OPTION_OUTPUT_WORD				4				// MS-Word output
#define	GEX_OPTION_OUTPUT_PDF				8				// PDF output
#define	GEX_OPTION_OUTPUT_PPT				16				// Powerpoint slides output
#define GEX_OPTION_OUTPUT_INTERACTIVEONLY	32				// Interactive mode only.
#define GEX_OPTION_OUTPUT_ODT               64
#define GEX_OPTION_OUTPUT_FLAT_HTML		(GEX_OPTION_OUTPUT_WORD | GEX_OPTION_OUTPUT_PDF | GEX_OPTION_OUTPUT_PPT | GEX_OPTION_OUTPUT_ODT)
#define GEX_OPTION_OUTPUT_HTML_BASED	(GEX_OPTION_OUTPUT_HTML | GEX_OPTION_OUTPUT_FLAT_HTML | GEX_OPTION_OUTPUT_INTERACTIVEONLY)

// Output Paper size (page type)
#define	GEX_OPTION_PAPER_SIZE_LETTER	1
#define	GEX_OPTION_PAPER_SIZE_A4		2

// Global Info Settings
// #define GEX_OPTION_GLOBAL_INFO_SUMMARIZED		1	// Detail level of global info when Comparing/Merging : summarized
// #define GEX_OPTION_GLOBAL_INFO_DETAILED			2	// Detail level of global info when Comparing/Merging : detailed

// 'Testers' page
#define	GEXMO_OPTION_TESTERS_PRODUCT	1
#define	GEXMO_OPTION_TESTERS_OPERATOR	2
#define	GEXMO_OPTION_TESTERS_PROGRAM	4
#define	GEXMO_OPTION_TESTERS_PARTS		8
#define	GEXMO_OPTION_TESTERS_GOOD		16
#define	GEXMO_OPTION_TESTERS_FAIL		32
#define	GEXMO_OPTION_TESTERS_YIELD		64
#define	GEXMO_OPTION_TESTERS_ALARM		128
#define	GEXMO_OPTION_TESTERS_CHART		256

// Outlier/Inliner removal
#define	GEX_OPTION_OUTLIER_NONE		0
#define	GEX_OPTION_OUTLIER_100LIM	1
#define	GEX_OPTION_OUTLIER_150LIM	2
#define	GEX_OPTION_OUTLIER_200LIM	3
#define	GEX_OPTION_OUTLIER_250LIM	4
#define	GEX_OPTION_OUTLIER_300LIM	5
#define	GEX_OPTION_OUTLIER_SIGMA	6
#define	GEX_OPTION_OUTLIER_IQR		7
#define	GEX_OPTION_INLINER_SIGMA	8

// Multi result processing
#define GEX_MULTIRESULT_USE_FIRST	0
#define GEX_MULTIRESULT_USE_LAST	1
#define GEX_MULTIRESULT_USE_MIN		2
#define GEX_MULTIRESULT_USE_MAX		3
#define GEX_MULTIRESULT_USE_MEAN	4
#define GEX_MULTIRESULT_USE_MEDIAN	5

// Trend x axis...
#define GEX_TREND_XAXIS_RUNID		0
#define GEX_TREND_XAXIS_PARTID		1

// Chart size
#define	GEX_CHARTSIZE_AUTO			0
#define	GEX_CHARTSIZE_SMALL			1
#define	GEX_CHARTSIZE_MEDIUM		2
#define	GEX_CHARTSIZE_LARGE			3
#define	GEX_CHARTSIZE_BANNER		4

// Chart size XY dimensions (in pixels)
#define	GEX_CHARTSIZE_SMALL_X		200
#define	GEX_CHARTSIZE_SMALL_Y		100
#define	GEX_CHARTSIZE_MEDIUM_X		400
#define	GEX_CHARTSIZE_MEDIUM_Y		200
#define	GEX_CHARTSIZE_LARGE_X		800
#define	GEX_CHARTSIZE_LARGE_Y		400
#define	GEX_CHARTSIZE_BANNER_X		900	// Banner type chart: Very large but tiny Y axis, allows maximizing number of charts / page
#define	GEX_CHARTSIZE_BANNER_Y		160

// iTrendChartType
#define	GEX_CHARTTYPE_LINES			4
#define	GEX_CHARTTYPE_SPOTS			5
#define	GEX_CHARTTYPE_LINESSPOTS	6

// Probing direction
#define	GEX_OPTION_WAFMAP_POSITIVE_AUTO		1		// Auto detect Probing direction
#define	GEX_OPTION_WAFMAP_POSITIVE_LEFT		2		// Positive X Probing direction: Left
#define	GEX_OPTION_WAFMAP_POSITIVE_RIGHT	3		// Positive X Probing direction: Right
#define	GEX_OPTION_WAFMAP_POSITIVE_UP		4		// Positive Y Probing direction: Up
#define	GEX_OPTION_WAFMAP_POSITIVE_DOWN		5		// Positive Y Probing direction: Down

// Notch orientation
#define	GEX_OPTION_WAFMAP_NOTCH_IGNORE		1		// Ignore notch orientation
#define	GEX_OPTION_WAFMAP_NOTCH_DOWN		2		// Notch should be Down
#define	GEX_OPTION_WAFMAP_NOTCH_UP			3		// Notch should be Up
#define	GEX_OPTION_WAFMAP_NOTCH_LEFT		4		// Notch should be Left
#define	GEX_OPTION_WAFMAP_NOTCH_RIGHT		5		// Notch should be Right
#define	GEX_OPTION_WAFMAP_NOTCH_AUTO		6		// Auto detect Notch orientation (from STDF file)

// Export csv Units mode
#define GEX_OPTIONS_TOOLBOX_UNITS_NORMALIZED		0	// Units normalized
#define GEX_OPTIONS_TOOLBOX_UNITS_SCALING_FACTOR	1	// Units using scaling factor

// Pearson's report sorting by...
#define	GEX_PEARSON_SORTING_TESTNAME		1
#define	GEX_PEARSON_SORTING_RATIO			2

//#########################################################
// ALL GEX internal source code related constants
//#########################################################

// Test# offset for files with parameters specified without a test#!
#define	GEX_TESTNBR_OFFSET_EXT				786000	// Extended tests created by Examinator. eg: Test time, Binning, etc...
#define GEX_TESTNBR_OFFSET_EXT_SBIN			(GEX_TESTNBR_OFFSET_EXT)
#define GEX_TESTNBR_OFFSET_EXT_HBIN			(GEX_TESTNBR_OFFSET_EXT+1)
#define GEX_TESTNBR_OFFSET_EXT_DIEX			(GEX_TESTNBR_OFFSET_EXT+2)
#define GEX_TESTNBR_OFFSET_EXT_DIEY			(GEX_TESTNBR_OFFSET_EXT+3)
#define GEX_TESTNBR_OFFSET_EXT_TTIME		(GEX_TESTNBR_OFFSET_EXT+4)
#define GEX_TESTNBR_OFFSET_EXT_TEMPERATURE	(GEX_TESTNBR_OFFSET_EXT+5)
#define GEX_TESTNBR_OFFSET_EXT_TESTING_SITE (GEX_TESTNBR_OFFSET_EXT+6)
#define GEX_TESTNBR_OFFSET_EXT_PARTID		(GEX_TESTNBR_OFFSET_EXT+7)
#define GEX_TESTNBR_OFFSET_EXT_TEST_TIME_T	(GEX_TESTNBR_OFFSET_EXT+8)
// !!!! WHEN ADDING A CUSTOM TEST, MAKE SURE TO UPDATE BELOW MIN,MAX VALUES !!!!
#define GEX_TESTNBR_OFFSET_EXT_MIN			(GEX_TESTNBR_OFFSET_EXT)				// Min custom test#
#define GEX_TESTNBR_OFFSET_EXT_MAX			(GEX_TESTNBR_OFFSET_EXT_TEST_TIME_T)	// Max custom test#
#define isGexTest(lTestNb) (((lTestNb>=GEX_TESTNBR_OFFSET_EXT_MIN) && (lTestNb<=GEX_TESTNBR_OFFSET_EXT_MAX)) ? true:false)

// Scripts files created to automate proceses
#define	GEX_SCRIPT_CONFIG			1
#define	GEX_SCRIPT_ASSISTANT		2
#define	GEX_LOCAL_UNIXFOLDER		"/.examinator"
#define	GEX_DEFAULT_DIR				"/GalaxySemi"
#define	GEX_SCRIPT_CONFIG_NAME		"/.galaxy_examinator.csl"
#define	GEXDB_SCRIPT_CONFIG_NAME	"/.galaxy_examinatordb.csl"
#define	GEXWEB_SCRIPT_CONFIG_NAME	"/.galaxy_examinatorweb.csl"
#define	GEX_SCRIPT_ASSISTANT_NAME	"/.gex_assistant.csl"
#define	GEXDB_SCRIPT_ASSISTANT_NAME	"/.gexdb_assistant.csl"
#define	GEXWEB_SCRIPT_ASSISTANT_NAME "/.gexweb_assistant.csl"
#define	GEX_SCRIPTING_CENTER_NAME   "/.galaxy_scriptingcenter_profile.csl"
#define GEX_CLIENT_NODE				"/.gex_localconfig.conf"
#define GEXDB_CLIENT_NODE			"/.gexdb_localconfig.conf"
#define GEXWEB_CLIENT_NODE			"/.gexweb_localconfig.conf"
#define GEXMO_TASKS_FILE			"/.gexmo_tasks.conf"
#define GEXMO_TASKS_XML_FILE		"/.gexmo_tasks.xml"
#define GEXMO_STATUS_FILE			"/.gexmo_status.conf"
#define GEXMO_LOG_FILE				"/.gexmo_history.log"
// PAT-38
#define GEXMO_LOG_FILE_ROOT			"gexmo_history"
#define GEXMO_REPORT_FILE_ROOT		"gexmo_report"
#define	GTM_SCRIPT_CONFIG_NAME		"/.galaxy_gtm.csl"
#define	GTM_SCRIPT_ASSISTANT_NAME	"/.gtm_assistant.csl"
#define	YM_SCRIPT_CONFIG_NAME		"/.galaxy_yieldman.csl"
#define	PAT_SCRIPT_CONFIG_NAME		"/.galaxy_patman.csl"
#define	YM_SCRIPT_ASSISTANT_NAME	"/.yieldman_assistant.csl"
#define	PAT_SCRIPT_ASSISTANT_NAME	"/.patman_assistant.csl"

// HTML file created by Flying Neptus to show 3D selections info
#define GEX3D_NEPTUS_INFO				"/.gex_drill_info.htm"

// List of section FLAGS (MUST BE power of 2) a GEX assistant script may include:
#define	GEX_SCRIPT_SECTION_FAVORITES	1
#define	GEX_SCRIPT_SECTION_OPTIONS		2
#define	GEX_SCRIPT_SECTION_SETTINGS		4
#define	GEX_SCRIPT_SECTION_GROUPS		8
#define	GEX_SCRIPT_SECTION_REPORT		16
#define GEX_SCRIPT_SECTION_DRILL		32
#define GEX_SCRIPT_SECTION_PLUGIN		64


// When ChartSize= AUTO, Define size classes accoring to number of plots to do
#define	GEX_CHARTSIZEAUTO_MEDIUM		40	// Max# of plots in order to create MEDIUM images
#define	GEX_CHARTSIZEAUTO_LARGE			10	// Max# of plots in order to create LARGE images

// When creating the test index HTML page, page can link to one of:
#define	GEX_INDEXPAGE_STATS					1
#define	GEX_INDEXPAGE_WAFERMAP				2
#define	GEX_INDEXPAGE_HISTO					3
#define	GEX_INDEXPAGE_ADVHISTO				4
#define	GEX_INDEXPAGE_ADVTREND				5
#define	GEX_INDEXPAGE_ADVBOXPLOT			6
#define	GEX_INDEXPAGE_ADVDIAGS				7
#define	GEX_INDEXPAGE_ADVGO					8
#define	GEX_INDEXPAGE_ADVBOXPLOT_EX			9
#define	GEX_INDEXPAGE_ADVPROBABILITY_PLOT	10
#define	GEX_INDEXPAGE_ADVMULTICHART			11
#define	GEX_INDEXPAGE_ADVHISTOFUNCTIONAL    12

// Folder where HTML report is created...
#define	DEFAULT_REPORT_NAME	"examinator_report"

// Maximum length for a file path+name
#define GEX_MAX_PATH 2048

///////////////////////////////////////////////////////////
// STDF Versions supported
///////////////////////////////////////////////////////////
#define	GEX_STDFV4		4
// Suffixe to files name converted to STDF from: ATDF, GDF, CSV, etc...
#define GEX_TEMPORARY_STDF	"_temp_gex"
#define GEX_TEMPORARY_HTML	"_temp_gex_htm"
#define GEX_TEMPORARY_CSV	"_galaxy.csv"
#define GEX_TEMPORARY_EMAIL	"_temp_gex_email"

///////////////////////////////////////////////////////////
// Definitions when ANALYZING a STDF file
///////////////////////////////////////////////////////////

// Message for 'Not Applicable or Not available'. MUST be TWO word (so keep space between "n/a" and ".").
#define GEX_NA "n/a ."

#define	C_NO_TEMPERATURE	(double) -500		// If no testting temperature known.
#define	C_INFINITE_PERCENTAGE	(double) 999	// Any value higher than 999% will show 999%
#define GEX_T(string) (char *)(string)
#define M_CHECK_INFINITE(val) (val <= -C_INFINITE? -C_INFINITE: (val >= C_INFINITE ? C_INFINITE: val))
#define TimeString(time) GEX_T((time<=0 ? GEX_NA: CGexSystemUtils::GetLocalTime(time).toLatin1().data())))
#define TimeStringUTC(time) GEX_T((time<=0 ? GEX_NA: CGexSystemUtils::GetUtcTime(time).toLatin1().data()))
#define TimeStringUTC_F(time, format) GEX_T((time<=0 ? GEX_NA: CGexSystemUtils::GetUtcTime(time, format).toLatin1().data()))

// Alarm flags Bits on test results drift (when comparing data)
#define	GEX_ALARM_MEANSHIFT		1
#define	GEX_ALARM_SIGMASHIFT	2
#define GEX_ALARM_CPSHIFT       3
#define GEX_ALARM_CRSHIFT       5
#define	GEX_ALARM_CPKSHIFT		4

// Drift formula type
// #define	GEX_DRIFT_ALARM_VALUE	1	// Drift is computed over value drift.
// #define	GEX_DRIFT_ALARM_LIMITS	2	// Drift is computed over percentage of limit space drift.

// Units scaling type
#define	GEX_UNITS_RESCALE_NONE			1	// Leave units to the default as found in test data files
#define	GEX_UNITS_RESCALE_SMART			2	// Rescale units to the most appropriate units (eg: mVolts or uVolots, etc)
#define	GEX_UNITS_RESCALE_NORMALIZED	3	// Normalize all units (results & test limits)
#define GEX_UNITS_RESCALE_TO_LIMITS		4	// Rescale units to limits units

// Origin of the wafermap (top-left corner) in the image pixmap
#define GEX_WAFMAP_ORGX					30
#define GEX_WAFMAP_ORGY					30
#define GEX_WAFMAP_TITLE				60

// Maximum binning classes in wafer map
#define MAX_WAFMAP_BIN			16
// Maximum Binning colors in wafermap (top X bins)
#define	GEX_WAFMAP_BINCOLORS	11

// Flags: Type of dataset sample removal
#define	GEX_REMOVE_HIGHER_THAN	0
#define	GEX_REMOVE_LOWER_THAN	1
#define	GEX_REMOVE_DATA_RANGE	2


// Maximum info per HTML page
#define	MAX_STATS_PERPAGE		80	// 80 tests per statistics page
#define	MAX_HISTOGRAM_PERPAGE	7	// 7 histograms per page
#define	MAX_DATALOG_PERPAGE		500	// Maximum datalog lines in a HTML datalog page
#define	MAX_PERCENTAGE_LINES_PER_WORD_PAGE				23	// Maximum lines for Word, or PDF file (pareto or binning, all including a histogram bar) in a page to feet in A4 and Letter format (used when creating Word, PPT, PDF files)
#define	MAX_PERCENTAGE_LINES_PER_PPT_PAGE				16	// Maximum lines PPT file (pareto or binning, all including a histogram bar) in a page to feet in A4 and Letter format (used when creating Word, PPT, PDF files)
#define	MAX_STATSLINES_PER_FLAT_PAGE					32	// Maximum lines in a stats page to feet in A4 and Letter format (used when creating Word, PPT, PDF files)

// Define default MIR string sizes
#define	MIR_STRING_SIZE		255

///////////////////////////////////////////////////////////
// HTML report related constants
///////////////////////////////////////////////////////////
#define	szFieldColor	"\"#CCECFF\""
#define	szAlarmColor	"\"#fc6e42\""
#define	szWarningColor	"\"#ffff80\""
#define	szDataColor		"\"#F8F8F8\""
// Drill What-If colors in HTML report
#define	szDrillColorIf		"\"#ffff00\""
#define	szDrillColorThen	"\"#81ffa2\""
#endif	// Only if GEX_CONSTANTS doesn't exist yet !
