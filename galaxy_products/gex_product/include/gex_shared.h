// contains the defines shared by all galaxy products (gex.exe, database plugins, ...)

#ifndef GEX_SHARED
#define	GEX_SHARED	1

#define GEX_EVALCOPY_NOTICE             "Evaluation Copy Only - Not for commercial use."

#define GEX_DATABASE_ENTRY_DEFINITION	"/.gexdb_entry"	// Database Entry definition file
#define GEX_DATABASE_INDEX_DEFINITION	"/.gexdb_index"	// Database Index definition file
#define GEX_DATABASE_EXTERNAL_DB_DEF	"/.gexdb_extern" // Database Index definition file for External database
#define GEX_DATABASE_EXTERNAL_FTP_DEF	"/.gexdb_ftp"	// list of files copied over by FTP and inserted into the database.

///////////////////////////////////////////////////////////
// Galaxy Emails list
///////////////////////////////////////////////////////////
#define GEX_EMAIL_SUPPORT				"support@mentor.com"
#define GEX_EMAIL_SALES					"quantix_sales@mentor.com"
#define GEX_EMAIL_LICENSE				"gex.license@galaxysemi.com"
#define GEX_EMAIL_YIELD_MAN             "no-reply@mentor.com"
#define GEX_EMAIL_PAT_MAN				"no-reply@mentor.com"
#define GEX_EMAIL_NOREPLY				"no-reply@mentor.com"

///////////////////////////////////////////////////////////
// Messages
// Now temporarly in report options
//#define GEX_MESSAGE_UPGRADE_TO_PRO "Your Examinator release doesn't allow this function.\nyou need to upgrade to 'Examinator Pro'.\n\nPlease contact " GEX_EMAIL_SALES
//#define GEX_MESSAGE_UPGRADE_TO_PATMAN "WRITE ME"

///////////////////////////////////////////////////////////
// Database Query related IDs
///////////////////////////////////////////////////////////
// Combo box selection offset for: Query Time Period
#define	GEX_QUERY_TIMEPERIOD_TODAY			0	// Today
#define	GEX_QUERY_TIMEPERIOD_LAST2DAYS		1	// Last 2 days (Today-Yesterday)
#define	GEX_QUERY_TIMEPERIOD_LAST3DAYS		2	// Last 3 days (Today-and last 2days)
#define	GEX_QUERY_TIMEPERIOD_LAST7DAYS		3	// Last 7 days (1Week)
#define	GEX_QUERY_TIMEPERIOD_LAST14DAYS		4	// Last 14 days (2Weeks)
#define	GEX_QUERY_TIMEPERIOD_LAST31DAYS		5	// Last 31 days (Full month)
#define	GEX_QUERY_TIMEPERIOD_THISWEEK		6	// All days this week
#define	GEX_QUERY_TIMEPERIOD_THISMONTH		7	// All days this month
#define	GEX_QUERY_TIMEPERIOD_ALLDATES		8	// All dates (no restriction on time/date)
#define	GEX_QUERY_TIMEPERIOD_CALENDAR		9	// Custom: Calendar selection
#define	GEX_QUERY_TIMEPERIOD_LAST_N_X		10	// Custom: Last N * X

// Combo box selection offset for: Housekeeping Time Period
#define	GEX_QUERY_HOUSEKPERIOD_TODAY			11	// Today
#define	GEX_QUERY_HOUSEKPERIOD_1DAY				12	// Yesterday and older
#define	GEX_QUERY_HOUSEKPERIOD_2DAYS			13	// 2 days ago and older
#define	GEX_QUERY_HOUSEKPERIOD_3DAYS			14	// 3 days ago and older
#define	GEX_QUERY_HOUSEKPERIOD_4DAYS			15	// 4 days ago and older
#define	GEX_QUERY_HOUSEKPERIOD_1WEEK			16	// 1 week ago and older
#define	GEX_QUERY_HOUSEKPERIOD_2WEEKS			17	// 2 weeks ago and older
#define	GEX_QUERY_HOUSEKPERIOD_3WEEKS			18	// 3 weeks ago and older
#define	GEX_QUERY_HOUSEKPERIOD_1MONTH			19	// 1 month ago and older
#define	GEX_QUERY_HOUSEKPERIOD_CALENDAR			20	// Custom: Calendar selection. Note: must be SAME offset as GEX_QUERY_TIMEPERIOD_CALENDAR !

// Combo box selection offset for: Filter parameter
#define GEX_QUERY_FILTER_NONE				0
#define	GEX_QUERY_FILTER_BURNIN				1
#define	GEX_QUERY_FILTER_ORIGIN				2
#define	GEX_QUERY_FILTER_DIBNAME			3
#define	GEX_QUERY_FILTER_DIBTYPE			4
#define	GEX_QUERY_FILTER_FACILITY			5
#define	GEX_QUERY_FILTER_FAMILY				6
#define	GEX_QUERY_FILTER_FLOOR				7
#define	GEX_QUERY_FILTER_FREQUENCYSTEP		8
#define	GEX_QUERY_FILTER_LOADBOARDNAME		9
#define	GEX_QUERY_FILTER_LOADBOARDTYPE		10
#define	GEX_QUERY_FILTER_LOT				11
#define	GEX_QUERY_FILTER_OPERATOR			12
#define	GEX_QUERY_FILTER_PACKAGE			13
#define	GEX_QUERY_FILTER_PROBERNAME			14
#define	GEX_QUERY_FILTER_PROBERTYPE			15
#define	GEX_QUERY_FILTER_PROCESS			16
#define	GEX_QUERY_FILTER_PRODUCT			17
#define	GEX_QUERY_FILTER_PROGRAMNAME		18
#define	GEX_QUERY_FILTER_PROGRAMREVISION	19
#define	GEX_QUERY_FILTER_RETESTNBR			20
#define	GEX_QUERY_FILTER_SUBLOT				21
#define	GEX_QUERY_FILTER_TEMPERATURE		22
#define	GEX_QUERY_FILTER_TESTERNAME			23
#define	GEX_QUERY_FILTER_TESTERTYPE			24
#define	GEX_QUERY_FILTER_TESTCODE			25
#define GEX_QUERY_FILTER_SITENBR			26
#define GEX_QUERY_FILTER_WAFERID			27
#define GEX_QUERY_FILTER_USER1				28
#define GEX_QUERY_FILTER_USER2				29
#define GEX_QUERY_FILTER_USER3				30
#define GEX_QUERY_FILTER_USER4				31
#define GEX_QUERY_FILTER_USER5				32
#define GEX_QUERY_FILTER_TOTALPARTS			33

// Filter flags
#define	GEX_QUERY_FLAG_BURNIN					(1 << GEX_QUERY_FILTER_BURNIN)
#define	GEX_QUERY_FLAG_ORIGIN					(1 << GEX_QUERY_FILTER_ORIGIN)
#define	GEX_QUERY_FLAG_DIBNAME				(1 << GEX_QUERY_FILTER_DIBNAME)
#define	GEX_QUERY_FLAG_DIBTYPE				(1 << GEX_QUERY_FILTER_DIBTYPE)
#define	GEX_QUERY_FLAG_FACILITY				(1 << GEX_QUERY_FILTER_FACILITY)
#define	GEX_QUERY_FLAG_FAMILY						(1 << GEX_QUERY_FILTER_FAMILY)
#define	GEX_QUERY_FLAG_FLOOR						(1 << GEX_QUERY_FILTER_FLOOR)
#define	GEX_QUERY_FLAG_FREQUENCYSTEP		(1 << GEX_QUERY_FILTER_FREQUENCYSTEP)
#define	GEX_QUERY_FLAG_LOADBOARDNAME		(1 << GEX_QUERY_FILTER_LOADBOARDNAME)
#define	GEX_QUERY_FLAG_LOADBOARDTYPE		(1 << GEX_QUERY_FILTER_LOADBOARDTYPE)
#define	GEX_QUERY_FLAG_LOT						(1 << GEX_QUERY_FILTER_LOT)
#define	GEX_QUERY_FLAG_OPERATOR				(1 << GEX_QUERY_FILTER_OPERATOR)
#define	GEX_QUERY_FLAG_PACKAGE				(1 << GEX_QUERY_FILTER_PACKAGE)
#define	GEX_QUERY_FLAG_PROBERNAME			(1 << GEX_QUERY_FILTER_PROBERNAME)
#define	GEX_QUERY_FLAG_PROBERTYPE			(1 << GEX_QUERY_FILTER_PROBERTYPE)
#define	GEX_QUERY_FLAG_PRODUCT				(1 << GEX_QUERY_FILTER_PRODUCT)
#define	GEX_QUERY_FLAG_PROGRAMNAME			(1 << GEX_QUERY_FILTER_PROGRAMNAME)
#define	GEX_QUERY_FLAG_PROGRAMREVISION		(1 << GEX_QUERY_FILTER_PROGRAMREVISION)
#define	GEX_QUERY_FLAG_RETESTNBR			(1 << GEX_QUERY_FILTER_RETESTNBR)
#define	GEX_QUERY_FLAG_SUBLOT					(1 << GEX_QUERY_FILTER_SUBLOT)
#define	GEX_QUERY_FLAG_TEMPERATURE		(1 << GEX_QUERY_FILTER_TEMPERATURE)
#define	GEX_QUERY_FLAG_TESTERNAME			(1 << GEX_QUERY_FILTER_TESTERNAME)
#define	GEX_QUERY_FLAG_TESTERTYPE			(1 << GEX_QUERY_FILTER_TESTERTYPE)
#define	GEX_QUERY_FLAG_TESTCODE				(1 << GEX_QUERY_FILTER_TESTCODE)
#define	GEX_QUERY_FLAG_PROCESS				(1 << GEX_QUERY_FILTER_PROCESS)
#define GEX_QUERY_FLAG_SITENBR				(1 << GEX_QUERY_FILTER_SITENBR)
#define GEX_QUERY_FLAG_WAFERID				(1 << GEX_QUERY_FILTER_WAFERID)
#define GEX_QUERY_FLAG_USER1				(1 << GEX_QUERY_FILTER_USER1)
#define GEX_QUERY_FLAG_USER2				(1 << GEX_QUERY_FILTER_USER2)
#define GEX_QUERY_FLAG_USER3				(1 << GEX_QUERY_FILTER_USER3)
#define GEX_QUERY_FLAG_USER4				(1 << GEX_QUERY_FILTER_USER4)
#define GEX_QUERY_FLAG_USER5				((quint64) 1 << GEX_QUERY_FILTER_USER5)
#define GEX_QUERY_FLAG_TOTALPARTS			((quint64) 1 << GEX_QUERY_FILTER_TOTALPARTS)

// Database storage mode
#define	DB_STORAGEMODE_ZIP		0	// Zip storage
#define	DB_STORAGEMODE_COPY		1	// Full copy storage
#define	DB_STORAGEMODE_LINK		2	// Link copy storage
#define	DB_STORAGEMODE_SUMMARY	3	// Summary storage only
#define	DB_STORAGEMODE_CORP		4	// Link to corporate database
#define	DB_STORAGEMODE_NONE		5	// NO storage!...Very useful if only want to monitor data and not store any file!

// Combine filters: Yes ot NO combo box
#define	GEX_QUERY_COMBINEFILTERS_YES		0
#define	GEX_QUERY_COMBINEFILTERS_NO			1

// GEX database types to list in combo box
#define DB_TYPE_FILEBASED       0x001
#define DB_TYPE_SQL             0x002
#define DB_TYPE_BLACKHOLE       0x008

#define DB_STATUS_ADR_LINK      0x010
#define DB_STATUS_UPLOADED      0x020
#define DB_STATUS_CONNECTED     0x040
#define DB_STATUS_UPTODATE      0x080

#define DB_SUPPORT_INSERTION    0x100
#define DB_SUPPORT_UPDATE       0x200
#define DB_SUPPORT_ER           0x400
#define DB_SUPPORT_RC           0x800


// ex: List of DB for Examinator-PRO for query extraction
// contains data (not blackhole) + connected + uptodate
#define DB_SELECT_FOR_EXTRACTION        DB_TYPE_FILEBASED|DB_TYPE_SQL|DB_STATUS_CONNECTED|DB_STATUS_UPTODATE
// ex: List of DB that can be used for Datapump
// all db (filebased + sql + blackhole) not necessary connected
#define DB_SELECT_FOR_INSERTION         DB_TYPE_FILEBASED|DB_TYPE_SQL|DB_SUPPORT_INSERTION

// Database type to filter the list in combo box
#define DB_TDR_ALL                  0x00
#define DB_TDR_YM_PROD              0x01
#define DB_TDR_MANUAL_PROD          0x02
#define DB_TDR_MANUAL_CHARAC        0x04
#define DB_ADR                      0x08
#define DB_ADR_LOCAL                0x10

// Name of TDR databases
#define GEXDB_CHAR_TDR_NAME         "Characterization TDR"
#define GEXDB_MAN_PROD_TDR_NAME     "Manual Production TDR"
#define GEXDB_YM_PROD_TDR_NAME      "Yield-Man Production TDR"
#define GEXDB_ADR_NAME              "Yield-Man Production ADR"
#define GEXDB_ADR_LOCAL_NAME        "Manual Production ADR"
#define GEXDB_ADMIN_DB_NAME         "Yield-Man Administration database"

// Keys of TDR databases
#define GEXDB_CHAR_TDR_KEY          "charac_tdr"
#define GEXDB_MAN_PROD_TDR_KEY      "man_prod_tdr"
#define GEXDB_YM_PROD_TDR_KEY       "ym_prod_tdr"
#define GEXDB_ADR_KEY               "adr"
#define GEXDB_ADR_LOCAL_KEY         "local_adr"
#define GEXDB_ADMIN_DB_KEY          "ym_admin_db"

#endif
