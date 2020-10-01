// contains the Examinator version info shared by all galaxy products (gex.exe, database plugins, ...)

#ifndef GEX_VERSION
#define	GEX_VERSION	1

// DO NOT CHANGE TABS IN BELOW defines
#define	GEX_APP_VERSION_DATE	20161102
#define	GEX_APP_VERSION_MAJOR	7
#define	GEX_APP_VERSION_MINOR	7
#define	GEX_APP_VERSION_PATCH	0
#define	GEX_APP_VERSION_BUILD	000
#define	GEX_APP_REVISION		"#abbrev_commit_hash#"
#define	GEX_APP_VERSION			"V7.7.0"

// Galaxy Reports versions supported
#define GEX_MIN_GRT_VERSION_V1				0.3f			// V6.2
#define GEX_MAX_GRT_VERSION_V1				0.4f			// V6.2
#define GEX_MIN_GRT_VERSION_V2				0.5f			// V6.3 and higher
// 0.5 : from Gex 6.3
// 0.6 : from gex 6.4
// 0.7 : from gex 6.5
#define GEX_MAX_GRT_VERSION_V2				0.7f			// V6.5 and higher
#define GEX_MIN_GRT_VERSION					GEX_MIN_GRT_VERSION_V1
#define GEX_MAX_GRT_VERSION					GEX_MAX_GRT_VERSION_V2

#define GEX_MIN_GRXML_VERSION			0.6f
// 0.1 the one in Gex 6.2
// 0.5 : the one in Gex 6.3
// 0.6 : from gex 6.4
// 0.7 : from gex 6.5
#define GEX_MAX_GRXML_VERSION			0.66f

// supported Galaxy Trigger File versions
#define GEX_MIN_GTF_VERSION 0.3f
#define GEX_MAX_GTF_VERSION 0.4f

// supported csl files version
// warning : some versions could be read supported but not write supported
// example : Gex 6.3 should read csl version 0 and 1 and write csl version 1
// example : Gex 6.4 should read csl version 1 & 2 and write csl version 2
// 2.2 : mainly for 6.5
// 2.3 : modified adv_pearson sorting : now 3 choices : test_name|pearson_asc|pearson_dsc
// 2.4 : added new gexOptions(...)
// 2.5 (7.2): added new MPR option : test_name and test_number
// 2.6 (7.3): added 'Cr' option flag for test stats (GCORE-199)
// 2.7 (7.4): added parameter to gexFile fonction (type: controls or samples)
#define GEX_MIN_CSL_VERSION 2.1f
#define GEX_MAX_CSL_VERSION 2.7f

// Galaxy Apps names
// in 6.5, GEX_APP_EXAMINATOR mutates from 'Examinator Characterization' to 'Examinator'
#define	GEX_APP_EXAMINATOR				"Examinator - " GEX_APP_VERSION
#define	GEX_APP_EXAMINATOR_OEM_ASL		"Visual Examinator - " GEX_APP_VERSION
#define	GEX_APP_EXAMINATOR_OEM_SAPPHIRE	"Examinator Sapphire D series - " GEX_APP_VERSION
#define	GEX_APP_EXAMINATOR_OEM_SZ		"Examinator SPACE - " GEX_APP_VERSION
#define GEX_APP_EXAMINATOR_OEM_LTXC     "Examinator for LTX-Credence - " GEX_APP_VERSION
#define GEX_APP_EXAMINATOR_OEM_TER      "Teradyne-Examinator - " GEX_APP_VERSION
#define GEX_APP_EXAMINATOR_TERPROPLUS   "Teradyne-Examinator-Pro+ - " GEX_APP_VERSION
// in 6.5, GEX_APP_EXAMINATOR_DB became GEX_APP_EXAMINATOR_PRO
#define GEX_APP_EXAMINATOR_PRO			"Examinator-Pro - " GEX_APP_VERSION
#define GEX_APP_EXAMINATOR_WEB			"Examinator Web - " GEX_APP_VERSION
#define GEX_APP_EXAMINATOR_MO			"Quantix Yield-Man - " GEX_APP_VERSION
#define GEX_APP_YME                     "Quantix Yield-Man Enterprise - " GEX_APP_VERSION
#define GEX_APP_EXAMINATOR_TB			"Examinator ToolBox - " GEX_APP_VERSION
#define GEX_APP_EXAMINATOR_YIELD123		"Quantix Yield123 - " GEX_APP_VERSION
#define GEX_APP_PATMAN					"Quantix PAT-Man - " GEX_APP_VERSION
#define GEX_APP_PME                     "Quantix PAT-Man Enterprise - " GEX_APP_VERSION
#define GEX_APP_EXAMINATOR_DB_PAT		"Examinator PAT - " GEX_APP_VERSION
#define GEX_APP_GTM                     "Quantix Tester Monitor - " GEX_APP_VERSION

#endif
