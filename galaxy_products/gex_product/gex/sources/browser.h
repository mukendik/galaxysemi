#ifndef GEX_BROWSER_H
#define	GEX_BROWSER_H

// mainly DEFINES for the GEX browser web pages
// ALL BROWSER related constants

// Flag to specify HTML Sections GEX must NOT create (used for faster processing
// in situation when multiple report are generated under similar conditions):
#define GEX_HTMLSECTION_ALL			0xffff
#define GEX_HTMLSECTION_GLOBALINFO	1
#define GEX_HTMLSECTION_STATISTICS	2
#define GEX_HTMLSECTION_HISTOGRAM	4
#define GEX_HTMLSECTION_WAFERMAP	8
#define GEX_HTMLSECTION_BINNING		16
#define GEX_HTMLSECTION_PARETO		32
#define GEX_HTMLSECTION_ADVANCED	64

// Prefix string in any hyperlink that must be processed internally!
#define	GEX_BROWSER_ACTIONLINK		"_gex_"
#define	GEX_BROWSER_ACTIONBOOKMARK	"#_gex_"
#define	GEX_BROWSER_STDLINK			"_gexstd_"
#define	GEX_BROWSER_FLYINGLINK		"_flygex_"

// Flying neptus: no comment page!
#define	GEX_HTMLPAGE_FLYNEPTUS_NONE	"nocomment.htm"
// Home page depends of the GEX release!
#define	GEX_HTMLPAGE_HOME_ROOT				"home.htm"
#define	GEX_HTMLPAGE_HOME_ROOT_DATABSE		"db_home.htm"	// GexDB HELP home page
#define	GEX_HTMLPAGE_HOME_ROOT_MONITORING	"mo_home.htm"	// Monitoring HELP home page
#define	GEX_HTMLPAGE_HOME_ROOT_TOOLBOX		"tb_home.htm"	// ToolBox HELP home page
#define	GEX_HTMLPAGE_HOME					"_gex_home.htm"
#define	GEX_HTMLPAGE_HOME_PRO				"_gex_pro_home.htm"	// Gex Pro Edition
#define	GEX_HTMLPAGE_HOME_CREDENCE_SAPPHIRE	"_gex_home_credence_sapphire.htm"
#define	GEX_HTMLPAGE_HOME_CREDENCE_ASL		"_gex_home_credence_asl.htm"
#define	GEX_HTMLPAGE_HOME_LTX				"_gex_home_ltx.htm"
#define GEX_HTMLPAGE_HOME_LTXC              "_gex_home_ltxc.htm"
#define	GEX_HTMLPAGE_HOME_SZ				"_gex_home_sz.htm"
#define	GEX_HTMLPAGE_HOME_TERADYNE			"_gex_home_teradyne.htm"
#define	GEX_HTMLPAGE_HOME_DB				"_gex_db_home.htm"

#define	GEX_HTMLPAGE_HOME_DB_PRO			"_gex_db_pro_home.htm"
#define	GEX_HTMLPAGE_HOME_DB_TDR			"_gex_db_tdr_home.htm"
#define	GEX_HTMLPAGE_HOME_DB_PRO_PLUS		"_gex_db_pro_plus_home.htm"

#define	GEX_HTMLPAGE_HOME_MONITORING		"_gex_mo_home.htm"
#define	GEX_HTMLPAGE_HOME_YME               "_gex_yme_home.htm"
#define	GEX_HTMLPAGE_HOME_PATSERVER			"_gex_patserver_home.htm"
#define	GEX_HTMLPAGE_HOME_PME               "_gex_pme_home.htm"
#define	GEX_HTMLPAGE_HOME_TOOLBOX			"_gex_tb_home.htm"
#define	GEX_HTMLPAGE_HOME_YIELD123			"_gex_yield123_home.htm"
#define	GEX_HTMLPAGE_SPLASHHOME				"_gex_home_splash.htm"
#define	GEX_HTMLPAGE_HOME_GTM       		"_gex_gtm_home.htm"

///////////////////////////////////////////////////////////
// HTML special hyperlinks that trigger GEX wizards...
///////////////////////////////////////////////////////////

// Definitions for: ELSE Link: used when browser in in none of the sections listed in top bar !
#define	GEX_BROWSER_ELSE_TOPLINK			"top_else.htm"
#define	GEX_BROWSER_ELSE_HELP				"Examinator pages."

// Definitions for: HOME Link
#define	GEX_BROWSER_HOME_TOPLINK			"top_home.htm"
#define	GEX_BROWSER_HOME_HELP				"Reload home page."

// Definitions for: FILES Link
#define	GEX_BROWSER_FILES_TOPLINK			"top_files.htm"
#define	GEX_BROWSER_FILES_LINK				"files.htm"
#define	GEX_BROWSER_FILES_HELP				"View/change the data selection used for the analysis."

// Definitions for: SETTINGS Link
#define	GEX_BROWSER_SETTINGS_TOPLINK		"top_settings.htm"
#define	GEX_BROWSER_SETTINGS_HELP			"View/edit the settings used to create your report."

// Definitions for: REPORT Link
#define	GEX_BROWSER_REPORT_TOPLINK			"top_report.htm"
#define	GEX_BROWSER_DBREPORT_LINK			"_gexstd_db_report.htm"
#define	GEX_BROWSER_REPORT_HELP				"Displays your report (home page)"

// Definitions for: OPTIONS Link
#define	GEX_BROWSER_OPTIONS_TOPLINK			"top_options.htm"
#define	GEX_BROWSER_OPTIONS_WIZARD			"options.htm"
#define	GEX_BROWSER_OPTIONS_HELP			"View/edit the environment options and preferences."

// Definitions for: HELP Link (in top bar,in html browser page)
#define	GEX_BROWSER_HELP_TOPLINK			"top_help.htm"
#define	GEX_BROWSER_HELP_LINK				"help.htm"
#define	GEX_BROWSER_HELPPRO_LINK			"gexpro_help.htm"
#define	GEX_BROWSER_HELP_HELP				"On-line Help"

// Definitions for: Wizard assistant.
#define	GEX_BROWSER_ASSISTANT_WIZARD		"wizard_assistant.htm"
#define	GEX_BROWSER_ASSISTANT_HELP			"Talk to the Quantix Assistant, he is the expert and has an answer to almost all questions!"

// Definitions for: Save settings into script file
#define	GEX_BROWSER_SAVE					"save.htm"
#define	GEX_BROWSER_SAVE_HELP				"Save your report settings into a script file"

// Definitions for: Wizard to analyze one file
#define	GEX_BROWSER_ONEFILE_WIZARD			"wizard_one_file.htm"
#define	GEX_BROWSER_ONEFILE_HELP			"Wizard to analyze one file"

// Definitions for: Wizard to compare multiple files
#define GEX_BROWSER_CMPFILES_WIZARD			"wizard_compare_files.htm"
#define GEX_BROWSER_CMPFILES_HELP			"Wizard to compare files"

#define GEX_BROWSER_FT_PAT_FILES_WIZARD     "wizard_ft_pat_file_analysis.htm"
#define GEX_BROWSER_FT_PAT_FILES_HELP       "Examinator-PAT wizard to FT PAT analysis"

// Definitions for: Wizard to merge files
#define GEX_BROWSER_ADDFILES_WIZARD			"wizard_merge_files.htm"
#define GEX_BROWSER_ADDFILES_HELP			"Wizard to merge files"

// Definitions for: Wizard to compare/merge files
#define GEX_BROWSER_MIXFILES_WIZARD			"wizard_mix_files.htm"
#define GEX_BROWSER_MIXFILES_HELP			"Wizard to merge and/or compare files"

// Definitions for: Wizard for Single Query
#define	GEX_BROWSER_ONEQUERY_WIZARD			"wizard_single_query.htm"
#define	GEX_BROWSER_ONEQUERY_HELP			"Examinator Wizard to Query your Data"

// Definitions for: Wizard for Compare Query
#define	GEX_BROWSER_CMPQUERY_WIZARD			"wizard_compare_query.htm"
#define	GEX_BROWSER_CMPQUERY_HELP			"Wizard to compare Datasets"

// Definitions for: Wizard for mix Query
#define	GEX_BROWSER_MIXQUERY_WIZARD			"wizard_merge_query.htm"
#define	GEX_BROWSER_MIXQUERY_HELP			"Wizard to merge Datasets"

// Definitions for: Wizard for SQL Production report/Template and SQL GUI
#define	GEX_BROWSER_SQL_QUERY_WIZARD		"wizard_sql_query.htm"
#define	GEX_BROWSER_SQL_QUERY_HELP			"Examinator Wizard to SQL Database"

// Definitions for: Wizard for Characterization WIzard
#define	GEX_BROWSER_CHAR_QUERY_WIZARD		"wizard_char_query.htm"
#define	GEX_BROWSER_CHAR_QUERY_HELP			"Examinator Wizard to Characterize your Data"

// Definitions for: Wizard for shift analysis
#define	GEX_BROWSER_SHIFT_WIZARD            "wizard_shift_analysis.htm"
#define	GEX_BROWSER_SHIFT_HELP              "Examinator Wizard to Analyze parametric shifts"

// Definitions for: Wizard for SQL Production report/Template and SQL GUI
#define	GEX_BROWSER_REPORTSCENTER			"reportscenter.htm"
#define	GEX_BROWSER_REPORTSCENTER_HELP		"Reports Center"

// Definitions for: Wizard for Database Admin.
#define	GEX_BROWSER_DBADMIN_WIZARD			"wizard_db_admin.htm"
#define	GEX_BROWSER_DBADMIN_HELP			"Wizard for Database Administration"

// Definitions for: Settings
#define	GEX_BROWSER_SETTINGS				"file_settings.htm"
#define	GEX_BROWSER_SETTINGSHLP				"Edit report settings (sections to create)"

// Definitions for: Edit Bin Colors
#define	GEX_BROWSER_BINCOLORS				"bin_colors.htm"
#define	GEX_BROWSER_BINCOLORS_HELP			"Edit binning colors"

// Definitions for: Save PAT report to disk (CSV format)
#define	GEX_BROWSER_PATREPORT				"pat_report.htm"
#define	GEX_BROWSER_PATREPORT_HELP			"Export PAT report to disk..."

// Definitions for: Statistical Bin Limits
#define	GEX_BROWSER_SBL						"export_sbl.htm"
#define	GEX_BROWSER_SBL_HELP				"Export Statistical Bin Limits"

// Definitions for: Export Wafermap to file
#define	GEX_BROWSER_EXPORT_WAFMAP			"export_wafmap.htm"
#define	GEX_BROWSER_EXPORT_WAFMAP_HELP		"Export Wafermap to file..."

// Definitions for: Scripting Wizard
#define GEX_BROWSER_SCRIPTING_TOPLINK		"top_script.htm"
#define GEX_BROWSER_SCRIPTING_LINK			"script.htm"
#define GEX_BROWSER_SCRIPTING_HELP			"Scripting center: Build and run your own fully automated Script files!"

// Definitions for Help User Manual
#define GEX_HELP_USER_MANUAL				"help_user_manual.htm"
#define GEX_HELP_USER_MANUAL_HELP			"Open user manual"

// Definition for: Drilling wizard
#define GEX_BROWSER_DRILL_WHATIF			"drill_whatif.htm"
#define GEX_BROWSER_DRILL_3D				"drill_3d.htm"
#define GEX_BROWSER_DRILL_CHART				"drill_chart.htm"
#define GEX_BROWSER_DRILL_TABLE				"drill_table.htm"
#define GEX_BROWSER_DRILL_ALL				"drill_all.htm"
#define GEX_BROWSER_DRILL_HELP				"Interactive 2D/3D drill into data!"

// Definition for: Open Report wizard (non HTML)
#define GEX_BROWSER_OPENREPORT_WIZARD		"wizard_openreport.htm"

// **** GTM ****
// Definitions for: Home Link
#define	GEXGTM_BROWSER_HOME_TOPLINK			"top_gtm_home.htm"
#define	GEXGTM_BROWSER_HOME_HELP			"Reloads home page."
// Definitions for: Testers Link
#define	GEXGTM_BROWSER_TESTERS_TOPLINK		"top_gtm_testers.htm"
#define	GEXGTM_BROWSER_TESTERS_LINK         "gtm_testers.htm"
#define	GEXGTM_BROWSER_TESTERS_HELP         "View all connected testers under PAT."
// Definitions for: OPTIONS Link
#define	GEXGTM_BROWSER_OPTIONS_TOPLINK		"top_gtm_options.htm"
#define	GEXGTM_BROWSER_OPTIONS_WIZARD		"options.htm"
#define	GEXGTM_BROWSER_OPTIONS_HELP			"View/edit the environment options and preferences."
// Definitions for: Help Link
#define	GEXGTM_BROWSER_HELP_TOPLINK			"top_gtm_help.htm"
#define	GEXGTM_BROWSER_HELP_LINK			"gtm_help.htm"
#define	GEXGTM_BROWSER_HELP_HELP			"GTM help."
// Definitions for: No tab selected
#define	GEXGTM_BROWSER_ELSE_TOPLINK			"top_gtm_else.htm"
#define	GEXGTM_BROWSER_ELSE_HELP			"Quantix pages."

// Definitions copyright license
#define GEXCOPYRIGHT_LICENSE_LINK			"copyrightlicense.htm"
#define GEXCOPYRIGHT_LICENSE_HELP			"Open the copyright license pdf file"

// Definitions for Quick start pdf
#define EXAM_QUICKSTART_LINK			"exam_quick_start_guide.htm"
#define EXAM_QUICKSTART_HELP			"Open the quick start pdf file"

// Definitions for Install pdf
#define EXAM_INSTALL_LINK			"exam_installation_instructions.htm"
#define EXAM_INSTALL_HELP			"Open the install pdf file"

// Definitions for user license pdf
#define USER_LICENSE_LINK			"user_license_manual.htm"
#define USER_LICENSE_HELP			"Open the user license pdf file"

// Definitions for rh pdf
#define RH_LINK			"release_highlights.htm"
#define RH_HELP			"Open the release highlights pdf file"

// Definitions for Install pdf
#define TER_INSTALL_LINK			"ter_installation_instructions.htm"
#define TER_INSTALL_HELP			"Open the install pdf file"

// Definitions for rh pdf
#define TER_RH_LINK			"ter_release_highlights.htm"
#define TER_RH_HELP			"Open the release highlights pdf file"

// Definitions for Quick start pdf
#define YM_QUICKSTART_LINK			"ym_quick_start_guide.htm"
#define YM_QUICKSTART_HELP			"Open the quick start pdf file"

// Definitions for Install pdf
#define YM_INSTALL_LINK			"ym_installation_instructions.htm"
#define YM_INSTALL_HELP			"Open the install pdf file"

#endif
