/****************************************************************************************/
/* Copyright GalaxySemiconductor                                                        */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/

// File: gtl_core.h
// Interface of the Galaxy Tester Library

// Notes:
/*
    This header file contains the interface to the GTL (Galaxy Tester Library) core library.
    The libraries to use with are:
    - Windows: gtl.a/lib and gtl.dll in dynamic linkage, OR in static linkage gtl_core.a
        IGXL: gtl-igxl.dll (i.e. VisualBasic) interface
    - Linux: gtl_core.a or libgtl.so
        LTXC enVision : libGTLenVision.so.0
*/

/*
    Revision history:
    o 17 May 2005: Created.
    o 2012 : version 2.2
    o 2013 Q1 : version 2.3
    o 2013 Q2 : version 3.0 to 3.1
    o 2013 Q3 : version 3.1 to 3.3
    o 2013 Q3 : version 3.3 to 3.4
    o 2013 July : version 3.4 to 3.5
      - removed gtl_endlot() interface
      - protocol modification (ENDLOT message sent to GTM waits for a reply)
    o 2013 July : 3.5 to 3.6 (RC1)
      - updated IGXL interface to adopt gtl_init() full interface with site numbers
      - added some hidden keys for blocking / non-blocking connection and communications
      - added some hidden keys for network timeout in non-blocking mode
    o 2013 August : 3.6 to 3.7 (RC2)
      - changed protocol for Q_INIT to fix case 7484
    o 2013 August : 3.7 to 3.8 (RC3)
      - fixed SPAT limits and stats insertion inside SQLite file
      - added transfert for both good hard and soft bin list from GTM to GTL
      - added nb_part and nb_parts good in SQLite splitlot table
      - added start_t and finish_t in SQLite splitlot table
    o 2013 August : 3.8 to 3.9 (RC5 and 7.1 final):
      - now using good hard and soft bin list to identify Pass of Fail bin status for SQLite output update
    o 2013 Sept : 3.9 to 3.91:
      - fixed test def insertion when LL but no HL and viceversa

    o 2013 Nov : from 3.9 to 4.0:
      - added gtl_mptest(...) new function useful for MultiParametricResults tests.
      - added retest support: new command gtl_command("retest");
      - added resume support : if a previous SQLite exists on the tester, then GTL checks for same product/lot/sublot/tester and if same then resume is turned on.
      - added new command: "query" limited to 'select' queries. Exple: gtl_set(GTL_KEY_QUERY, "select MAX(run_id), * from ft_rollinglimits"); gtl_set(GTL_KEY_QUERY_OUTPUT_FILE, "output.csv"); gtl_command("query");
      API deprecation:
      - gtl_get_lib_state(...) vs gtl_get(GTL_KEY_LIB_STATE, ...);
      - gtl_get_lib_version(...) vs gtl_get(GTL_KEY_LIB_VERSION, ...);
      - gtl_set_prod_info(...) vs gtl_set(GTL_KEY_PRODUCT_ID, ...), gtl_set(GTL_KEY_LOT_ID, ...), ...
      - gtl_set_node_info(...) vs gtl_set(..., ...), ...
      - gtl_get_number_messages_in_stack() vs gtl_get(GTL_KEY_NUM_OF_MESSAGES_IN_STACK, ...)
      - ...

    o 2014 Jan : from 4.0 to 4.1 (7.2 preview)
      - added many keys for filling many splitlot table fields (case 7672)
    o 2014 Feb : from 4.1 to 4.2 (7.2)
      - fixed retest outliers detection
      - fixed retest/resume when temp sqlite invalid/empty
      - allow failing PAT test retrieval (case 7522)
      - allow pop last/first message through a gtl command
      - removed any deps with lib pthread or libstdc++ dll (gtl.dll is now fully 'standalone')
      - added 2 GTL keys: GTL_KEY_TESTER_EXEC_TYPE, GTL_KEY_TESTER_EXEC_VERSION
      - deprecated 1 key (GTL_KEY_SUBLOT_NUMBER), replaced with GTL_KEY_SUBLOT_ID
    o 2014  : from 4.2 to 4.3 (7.3)
      - March: fixed loging when using the QUERY COMMAND
      - May: fixed resume flow (GCORE-590)
    o 2014  : from 4.3 to 5.0 (7.3)
    o 2014 Nov : from 5.0 to 6.0 (7.4)
      - added new feature to allow the customer to customize the parameters to match for resume/restest (GCORE-1302)
      - added new key GTL_KEY_FIELDS_TO_MATCH
      - enhanced and validated the GTL profiler in order to estimate time spend in GTL
      - added new key GTL_KEY_RELOAD_LIMITS
    o 2015 Mar : from 6.0 to 6.1 (7.4)
      - aligned schemas between TDR and FT-PAT traceability SQLite (GCORE-196)
    o 2016 May : from 6.1 to 6.2 (7.6)
      - added new key GTL_KEY_LIMITS_GET_CONDITION (GCORE-7693). Possible values are "always", "if_changed"
        (default is "if_changed")
      - added new gtl_get_spat_limits() function to retrieve SPAT limits (GCORE-7693)
      - added new gtl_get_dpat_limits() function to retrieve DPAT limits (GCORE-7693)
      - added new gtl_set_binning() function (GCORE-7693)
      - fixed valid_splitot in SQLite

*/
/****************************************************************************************/

#ifndef _GTL_CORE_H_
#define _GTL_CORE_H_

// These versions will be send to the server for compatibility chek.
// 3.6=7.2 RC1   3.7=7.2 RC2    3.8=7.2 RC3 4.0=7.2 MPR preview     4.2=7.2 final   4.3=7.2 patch
// 5.0=7.3
// 6.0=7.4 G     6.1=7.4.D      6.2=7.6
#define GTL_VERSION_MAJOR 6
#define GTL_VERSION_MINOR 2

// The default port is none specified in configuration file. The GTM server is currently testing env var GTM_PORT.
#define GTL_DEFAULT_SERVER_PORT 4747

/*======================================================================================*/
/* Defines                                                                              */
/*======================================================================================*/
/* Error Codes */
#define GTL_CORE_ERR_OKAY                       0	/* No error */
#define GTL_CORE_ERR_UNKNOWN_MESSAGE_TYPE       1	// GTL has received a message with an unknwon type (not a ack, nor DPAT,...)
#define GTL_CORE_ERR_CONF                       2	/* GTL can not read configuration file (bad path, syntax error,...) */
#define GTL_CORE_ERR_NOT_ALL_SITES_IN_DPAT      3	// Not all sites have reached the DPAT state and the deisred command cannot continue (retest, ...).
#define GTL_CORE_ERR_CRRF                       4	/* GTL can not read recipe file (bad path,...) */
#define GTL_CORE_ERR_GTM_INIT                   5	/* Error initializing connection to GTM server */
#define GTL_CORE_ERR_GTM_RECIPE                 6   /* GTM server couldn't read/parse recipe file or invalid recipe (check GTM logs for root cause) */
#define GTL_CORE_ERR_GTM_TESTLIST               7	/* Error retrieving/reading/analysing testlist from GTM server */
#define GTL_CORE_ERR_GTM_INITPAT                8	/* Error retrieving PAT configuration from GTM server */
#define GTL_CORE_ERR_GTM_COMM                   9	/* Error during communication with GTM server */
#define GTL_CORE_ERR_GTM_LICEXPIRED             10	/* License expired */
#define GTL_CORE_ERR_GTM_NOPATMODULE            11	/* PAT module not activated in license */
#define GTL_CORE_ERR_GTM_UNKNOWN_STATUS         12	/* Unknown status during GTM initialization */
#define GTL_CORE_ERR_LOADOPTION                 13	/* GTL disabled by Load option in conf file. */
#define GTL_CORE_ERR_BEGINJOB_NOT_EXEC          14	/* Illicit call to a function (as gtl_test, ...) prior to the beginjob one */
#define GTL_CORE_ERR_LIB_NOT_INITIALIZED        15	/* Library not initialized */
#define GTL_CORE_ERR_BAD_PROD_INFO              16	/* Prod info wrong */
#define GTL_CORE_ERR_FAILED_TO_SEND_PRODINFO    17	/* Failed to send Prod info to server (connection lost,...) */
#define GTL_CORE_ERR_FAILED_TO_SEND_RESULTS     18	/* Failed to send results to server (connection lost, corrupted packet, ...) */
#define GTL_CORE_ERR_INVALID_NUM_OF_SITES       19  /* Invalid num of sites has been declared at init time. Minimum is 1, max 255. */
#define GTL_CORE_ERR_LIB_ALREADY_INITIALIZED    20  /* Lib already inititalized. */
#define GTL_CORE_ERR_INVALID_SITE_NUMBER        21  /* Invalid site number, must be between 0 & 255. */
#define GTL_CORE_ERR_INVALID_PARAM_POINTER      22  /* Invalid pointer in function parameter (probably NULL) */
#define GTL_CORE_ERR_GTM_MAXCONN_REACHED        23  /* Max connection reached on the server side. The option is setable in GTM options. */
#define GTL_CORE_ERR_CANT_STAT_RECIPE           24 /* GTL cannot stat the recipe file (bad path,...) */
#define GTL_CORE_ERR_CANT_OPEN_RECIPE           25 /* GTL cannot open recipe, (bad path, no read access...) */
#define GTL_CORE_ERR_MALLOC_FAILED              26 /* Memory allocation error (out of mem, ...) */
#define GTL_CORE_ERR_MESSAGE_POPING_FAILED      27 /* Error in the message stack system. */
#define GTL_CORE_ERR_NO_MORE_MESSAGE            28 /* No more message to retrieve. this is not an error but just a warning that it is unusefull to try to pop messages. */
#define GTL_CORE_ERR_INVALID_TEST               29 /* Invalid test (number or name) or invalid site */
#define GTL_CORE_ERR_INVALID_COMMAND            30 /* Invalid command received from GTM (for the moment only 'RestartBaseline' is supported) */
#define GTL_CORE_ERR_VERSION_MISMATCH           31 /* Mismatch version between GTL and the supported version of GTL supported by GTM : both must be equal. */
#define GTL_CORE_ERR_UPDATE_SPAT                32 /* Error updating GTL with SPAT limits received from GTM */
#define GTL_CORE_ERR_NOT_ENABLED                33 /* GTL is not enabled */
#define GTL_CORE_ERR_INVALID_SITE_PTR           34 /* Invalid site pointer */
#define GTL_CORE_ERR_PROCESSMESSAGE             35 /* Error processing message received from GTM */
#define GTL_CORE_ERR_INVALID_OUTPUT_FOLDER      36 /* No or invalid output folder set (not writable for example). */
#define GTL_CORE_ERR_INVALID_PARAM_VALUE        37 /* Invalid value for one of the params */
#define GTL_CORE_ERR_FAILED_TO_INIT_OUTPUT      38 /* Failed to init the GTL output (mainly used today for traceability info). Root cause can be found in GTL log. */
#define GTL_CORE_ERR_GTL_BIN_ALREADY_CALLED     39 /* the gtl_binning() or gtl_set_binning() has already been called for that part/site/run */
#define GTL_CORE_ERR_INVALID_OUTPUT             40 /* The GTL output is not/no more valid (corrupted, has been closed,...) or the insertion into it has just failed. */
#define GTL_CORE_ERR_INVALID_PARTID             41 /* Invalid part ID (either null or empty) */
#define GTL_CORE_ERR_INVALID_LIBSTATE           42 // Invalid libstate (not in the list of known states : GTL_STATE_ENABLED, GTL_STATE_DISABLED,...)
#define GTL_CORE_ERR_GTM_NOLIC                  43 // No GTM license
#define GTL_CORE_ERR_DISABLED                   44 // GTL is disabled
#define GTL_CORE_ERR_INVALID_PIN                45 // Invalid pin index : it must be between 0 and the max as given in the recipe.
#define GTL_CORE_ERR_NO_TEMP_FOLDER             46 // GTL cannot find a usable temp folder. GTL first searchs for TMPDIR env var and then at misc locations... or use the one manually set with the GTL_KEY_TEMP_FOLDER key.
#define GTL_CORE_ERR_INVALID_OUTPUT_FILE        47 // GTL cannot open the given file for output/writing.
#define GTL_CORE_ERR_INVALID_LIMITS_TYPE        48 // unknown/invalid/illegal limits type (GTL_KEY_DESIRED_LIMITS, ...)
#define GTL_CORE_ERR_CANNOT_LOAD_DESIRED_LIMITS 49 // Failed to load the previous limits from the previous DB (perhaps product/lot/sublot/tester mismatch)
#define GTL_CORE_ERR_CANNOT_RELOAD_PREVIOUS_DB  50 // Failed to load a previous DB from the HDD to the memory (for resume or retest)
#define GTL_CORE_ERR_GTL_NOLIC                  51 // No GTL license
#define GTL_CORE_ERR_ILLEGAL_ON_THE_FLY_RETEST  52 // Illegal on the fly retest request: must be after endjob call, not inside a run.
#define GTL_CORE_ERR_UPDATE_DPAT                53 /* Error updating GTL with DPAT limits received from GTM */
#define GTL_CORE_ERR_ILLEGAL_QUERY              54 /* The query is not allowed: currently limited to queries starting with 'select' */
#define GTL_CORE_ERR_CANNOT_OPEN_IN_MEM_DB      55 /* sqlite3_open(":memory:") failed */
#define GTL_CORE_ERR_CANNOT_INSERT_RECIPE       56 /* Recipe insertion into GTL DB failed */
#define GTL_CORE_ERR_CANT_GET_MAX_RETEST_INDEX  57 /* Failed to retrieve the max retest index of previous splitlots */
#define GTL_CORE_ERR_CREATE_DB                  58 /* Failed to create traceability DB schema */
#define GTL_CORE_ERR_INSERT_NEW_SPLITLOT        59 /* Failed to insert new splitlot into traceability DB */
#define GTL_CORE_ERR_UPDATE_DB                  60 /* Failed to update traceability DB */
#define GTL_CORE_ERR_ARRAY_SIZE                 61 /* Array too small to hold required information */

/* GTL disabled rootcause codes */
#define GTL_DISABLED_ROOTCAUSE_UNKNOWN              0	/* Unknown rootcause */
#define GTL_DISABLED_ROOTCAUSE_INIT                 1	/* Error initializing GTL Library */
#define GTL_DISABLED_ROOTCAUSE_CONF                 2	/* Error reading configuration file */
#define GTL_DISABLED_ROOTCAUSE_CGNI                 3	/* Couldn't get node information (Station nb, Tester type ...) */
#define GTL_DISABLED_ROOTCAUSE_CRRF                 4	/* Couldn't read recipe file */
#define GTL_DISABLED_ROOTCAUSE_GTM_INIT             5	/* Error initializing connection to GTM server */
#define GTL_DISABLED_ROOTCAUSE_GTM_RECIPE           6	/* GTM server couldn't read recipe file */
#define GTL_DISABLED_ROOTCAUSE_GTM_TESTLIST         7	/* Error retrieving testlist from GTM server */
#define GTL_DISABLED_ROOTCAUSE_GTM_INITPAT          8	/* Error retrieving PAT configuration from GTM server */
#define GTL_DISABLED_ROOTCAUSE_GTM_COMM             9	/* Error during communication with GTM server */
#define GTL_DISABLED_ROOTCAUSE_LOADOPTION           10	/* GTL disabled by option in configuration file */
#define GTL_DISABLED_ROOTCAUSE_USERCMD              11	/* GTL disabled by user command (-gtl_disabled) */
#define GTL_DISABLED_ROOTCAUSE_GTM_LICEXPIRED       12	/* GTM: license expired */
#define GTL_DISABLED_ROOTCAUSE_GTM_NOPATMODULE      13	/* GTM: no PAT MODULE enabled */
#define GTL_DISABLED_ROOTCAUSE_GTM_UNKNOWN_STATUS	14	/* GTM: unknown status received */
#define GTL_DISABLED_ROOTCAUSE_BAD_PRODINFO         15	/* Bad prod info */
#define GTL_DISABLED_ROOTCAUSE_INVALID_NUM_OF_SITES 16	/* Invaid num of stes */
#define GTL_DISABLED_ROOTCAUSE_PROCESSMESSAGE       17	/* Error processing message received from GTM */
#define GTL_DISABLED_ROOTCAUSE_GTM_NOLIC            18	/* GTM: no license */

/* GTL diabled rootcause strings */
#define GTL_DISABLED_ROOTCAUSE_SZ_UNKNOWN               "Unknown"
#define GTL_DISABLED_ROOTCAUSE_SZ_INIT                  "Error initializing GTL Library"
#define GTL_DISABLED_ROOTCAUSE_SZ_CONF                  "Error reading configuration file"
#define GTL_DISABLED_ROOTCAUSE_SZ_CGNI                  "Couldn't get node information (Station nb, Tester type ...)"
#define GTL_DISABLED_ROOTCAUSE_SZ_CRRF                  "Couldn't read recipe file"
#define GTL_DISABLED_ROOTCAUSE_SZ_GTM_INIT              "Error initializing connection to GTM server"
#define GTL_DISABLED_ROOTCAUSE_SZ_GTM_RECIPE            "GTM server couldn't read recipe file"
#define GTL_DISABLED_ROOTCAUSE_SZ_GTM_TESTLIST          "Error retrieving testlist from GTM server"
#define GTL_DISABLED_ROOTCAUSE_SZ_GTM_INITPAT           "Error retrieving PAT configuration from GTM server"
#define GTL_DISABLED_ROOTCAUSE_SZ_GTM_COMM              "Error during communication with GTM server"
#define GTL_DISABLED_ROOTCAUSE_SZ_LOADOPTION            "GTL disabled by option in configuration file"
#define GTL_DISABLED_ROOTCAUSE_SZ_USERCMD               "GTL disabled by user command (-gtl_disabled)"
#define GTL_DISABLED_ROOTCAUSE_SZ_GTM_LICEXPIRED        "License expired"
#define GTL_DISABLED_ROOTCAUSE_SZ_GTM_NOPATMODULE       "PAT module is not enabled"
#define GTL_DISABLED_ROOTCAUSE_SZ_GTM_UNKNOWN_STATUS	"Unknown status received from GTM during initialization"
#define GTL_DISABLED_ROOTCAUSE_SZ_PROCESSMESSAGE        "Error processing message received from GTM"
#define GTL_DISABLED_ROOTCAUSE_SZ_GTM_NOLIC             "No license"

/* GTL state definitions */
#define GTL_STATE_NOT_INITIALIZED   0       /* not initialized */
#define GTL_STATE_ENABLED   		1       /* enabled */
#define GTL_STATE_OFFLINE           2       /* enabled in offline mode (lost GTM connection) */
#define GTL_STATE_DISABLED			3       /* disabled */

/* GTL site state definitions */
/* Baseline: collecting measurements for dynamic limit computation. Perhaps SPAT is used if turned on in recipe. */
#define GTL_SITESTATE_BASELINE          0
// DPAT: running with limits computed after baseline completion.
// Warning: each individual test could actually use original, SPAT or DPAT limits as enabled in the recipe.
#define GTL_SITESTATE_DPAT              1

// Limit type per site
#define GTL_LIMIT_TYPE_SPAT 0
#define GTL_LIMIT_TYPE_DPAT 1
#define GTL_LIMIT_TYPE_NONE 2

#define GTL_LIMITS_LAST "last"
#define GTL_LIMITS_TIGHTEST "tightest"
#define GTL_LIMITS_WIDEST "widest"

/* Test limits flags */
#define GTL_TFLAG_HAS_LL				0x01	/* Original Low limit exists  */
#define GTL_TFLAG_HAS_HL				0x02	/* Original High limit exists */
#define GTL_TFLAG_HAS_LL1				0x04	/* Low limit (mode 1 for bi-modal distribution) exists  */
#define GTL_TFLAG_HAS_HL1				0x08	/* High limit (mode 1 for bi-modal distribution) exists */
#define GTL_TFLAG_HAS_LL2				0x10	/* Low limit (mode 2 for bi-modal distribution) exists  */
#define GTL_TFLAG_HAS_HL2				0x20	/* High limit (mode 2 for bi-modal distribution) exists */

/* Value considered invalid (floating point max). Used to mark elements as not available or not executed. */
#define GTL_INVALID_VALUE_FLOAT			3.4e+38F
#define GTL_INVALID_VALUE_DOUBLE		1.7e+308
#define GTL_INVALID_VALUE_INT			0xFFFFFFFF
#define GTL_INVALID_VALUE_UINT			0xFFFFFFFF
/* Value considered infinite. */
#define GTL_INFINITE_VALUE_FLOAT		3.3e+38F
#define GTL_INFINITE_VALUE_DOUBLE		1.6e+308

// Size of messages returned by gtl_pop_last/first_message
#define GTL_MESSAGE_STRING_SIZE 1024

/* WARNING MESSAGES : 2000-2999 */
/* 2000-2099 : Main module */
#define GTL_MSGWARN_MAIN_BADBIN	2000   /* Bad Bin */
#define GTL_MSGWARN_MAIN_INVCMD	2001   /* Invalid GTL command */
/* 2100-2199 : Testlist module */
#define GTL_MSGWARN_TESTLIST_NOTEXEC 2100   /* Test was not executed during last run */
/* 2200-2299 : SQLite module */
#define GTL_MSGWARN_SQLITE_CANT_CHECK_CONTEXT   2200

/* Message Codes */
/* INFO MESSAGES : 0-999 */
/* 0-99 : General */
#define GTL_MSGINFO_VERSION						0		/* Display GTL version */
#define GTL_MSGINFO_STATE						1		/* Display GTL state */
#define GTL_MSGINFO_INIT						2		/* Initializing GTL */
#define GTL_MSGINFO_CONNECTING					3		/* Connecting to GTM */
#define GTL_MSGINFO_CONNECT_OK					4		/* Connecting to GTM sucessfull */
#define GTL_MSGINFO_CONNECT_NOK					5		/* Connecting to GTM unsucessfull */
#define GTL_MSGINFO_TEXT						6		/* Display information text */
#define GTL_MSGINFO_RECONNECT					7		/* Trying to reconnect to GTM */
#define GTL_MSGINFO_CHECKSESSION				8		/* Checking if GTM session still valid */
#define GTL_MSGINFO_SETSTATE_DISABLED			9		/* Set GTL state to DISABLED */
#define GTL_MSGINFO_SETSTATE_ENABLED			10		/* Set GTL state to ENABLED */
#define GTL_MSGINFO_ENDLOT						11		/* End lot detected */
#define GTL_MSGINFO_RESTART_BASELINE			12		/* Notification to re-start the baseline received from GTM */
#define GTL_MSGINFO_RESET						13		/* Reset GTL */
#define GTL_MSGINFO_SETSTATE_NOT_INITIALIZED	14		/* Set GTL state to NOT_INITIALIZED */
#define GTL_MSGINFO_SETSTATE_OFFLINE			15		/* Set GTL state to OFFLINE */
#define GTL_MSGINFO_END_SPLITLOT				16		/* End of splitlot */

/* ERROR MESSAGES : 1000-1999 */
/* 1000-1099 : Main module */
/* This code is currently unused and available for any new error message code. */
#define GTL_MSGERROR_MAIN_UNUSED_1				1000
#define GTL_MSGERROR_MAIN_CONF					1001	/* Couldn't read configuration file */
/* This code is currently unused and available for any new error message code. */
#define GTL_MSGERROR_MAIN_UNUSED_2				1002
#define GTL_MSGERROR_MAIN_CRRF					1003	/* Couldn't read recipe file */
#define GTL_MSGERROR_MAIN_GTM_INIT				1004	/* Error initializing connection to GTM server */
#define GTL_MSGERROR_MAIN_GTM_RECIPE			1005	/* GTM server couldn't read recipe file */
#define GTL_MSGERROR_MAIN_GTM_TESTLIST			1006	/* Error retrieving testlist from GTM server */
#define GTL_MSGERROR_MAIN_GTM_INITPAT			1007	/* Error retrieving PAT configuration from GTM server */
#define GTL_MSGERROR_MAIN_GTM_COMM				1008	/* Error during communication with GTM server */
#define GTL_MSGERROR_MAIN_GTM_LICEXPIRED		1009	/* License expired */
#define GTL_MSGERROR_MAIN_GTM_UNKNOWN_STATUS	1010	/* Unknown status received from GTM */
#define GTL_MSGERROR_MAIN_UPDATE_SPAT           1011	/* Error updating GTL with SPAT limits */
#define GTL_MSGERROR_MAIN_PROCESSMESSAGE        1012	/* Error processing message received from GTM */
#define GTL_MSGERROR_MAIN_GTM_NOLIC     		1013	/* No License */
#define GTL_MSGERROR_MAIN_UPDATE_DPAT           1014	/* Error updating GTL with DPAT limits */
/* 1100-1199 : Testlist module */
#define GTL_MSGERROR_TESTLIST_MAF				1100   /* Memory Allocation Failure */
#define GTL_MSGERROR_TESTLIST_NTL				1102   /* Couldn't find TestList */
#define GTL_MSGERROR_TESTLIST_EOTL				1103   /* End of TestList reached */
/* 1200-1299 : Socket module */
#define GTL_MSGERROR_SOCKET_CREATE				1200   /* Couldn't create socket */
#define GTL_MSGERROR_SOCKET_BIND				1201   /* Couldn't bind socket */
#define GTL_MSGERROR_SOCKET_READBACK			1202   /* Couldn't read back socket informations */
#define GTL_MSGERROR_SOCKET_HOSTNAME			1203   /* Couldn't retrieve hostname */
#define GTL_MSGERROR_SOCKET_OPTION				1204   /* Couldn't set socket options */
#define GTL_MSGERROR_SOCKET_CONNECT				1205   /* Couldn't connect to server socket */
#define GTL_MSGERROR_SOCKET_INIT				1206   /* Couldn't initialize socket system */
#define GTL_MSGERROR_SOCKET_CLEANUP				1207   /* Couldn't cleanup socket system */
#define GTL_MSGERROR_SOCKET_SEND				1208   /* Couldn't send request to server */
#define GTL_MSGERROR_SOCKET_REC					1209   /* Couldn't receive request from server */
#define GTL_MSGERROR_SOCKET_MAF					1210   /* Memory allocation failure */
#define GTL_MSGERROR_SOCKET_CBUF				1211   /* Error creating network buffer */
#define GTL_MSGERROR_SOCKET_RBUF				1212   /* Error reading network buffer */
#define GTL_MSGERROR_SOCKET_DOWN				1213   /* Socket is not up */
#define GTL_MSGERROR_SOCKET_CONFIG				1214   /* Error configuring socket */
#define GTL_MSGERROR_SOCKET_COMM				1215   /* Error during communication */
#define GTL_MSGERROR_SOCKET_TRANSFER_INIT		1216   /* Error during transfer of INIT message */
#define GTL_MSGERROR_SOCKET_TRANSFER_HBEAT		1217   /* Error during transfer of HEARTBEAT message */
#define GTL_MSGERROR_SOCKET_TRANSFER_RECONNECT	1218   /* Error during transfer of RECONNECT message */
/* 1300-1399 : Gnet module */
#define GTL_MSGERROR_GNET_ERROR					1300	/* Error in Gnet module */

/*
    GTL keys to be used with gtl_set and gtl_get :
*/

// Auxiliary file. Will be written into xt_splitlot.aux_file
#define GTL_KEY_AUX_FILE "aux_file"

// The path and filename of the configutration file: exple: /mypath/gtl_tester.conf
// MUST be set for gtl_open() to work.
#define GTL_KEY_CONFIG_FILE "config_file"

// The current message in buffer. Will be overwritten at each pop_last/first_message command. Get only, no Set access.
#define GTL_KEY_CURRENT_MESSAGE "current_message"
#define GTL_KEY_CURRENT_MESSAGE_SEV "current_message_sev"
#define GTL_KEY_CURRENT_MESSAGE_ID "current_message_id"

// Desired limits to be set before gtl_command("retest"): "last", "tightest", "widest"
#define GTL_KEY_DESIRED_LIMITS "desired_limits"
// Desired site number : for example, to be set before gtl_get("site_state")
#define GTL_KEY_DESIRED_SITE_NB "desired_site_nb"

// the datafile name that should be written by test program (example: XXX.stdf,...). Optional.
// Set the short name of the file in which the test results data is generated. If set, this file name
// will be included in the SQLite file. It will later be used for a more thorough check when
// matching SQLite data with test reults data.
#define GTL_KEY_DATA_FILENAME "datafile_name"

//
#define GTL_KEY_FACILITY_ID "facility_id"
// Product family ID.
#define GTL_KEY_FAMILY_ID "family_id"
// Test flow ID.
#define GTL_KEY_FLOW_ID "flow_id"
// Test floor ID.
#define GTL_KEY_FLOOR_ID "floor_id"

// Name of the test program or job plan testing the devices.
#define GTL_KEY_JOB_NAME "job_name"
// job rev: will be written into xt_splitlot.job_rev
#define GTL_KEY_JOB_REVISION "job_revision"

// GTL version: for example "3.1"
#define GTL_KEY_LIB_VERSION "lib_version"
#define GTL_KEY_LIB_VERSION_MAJOR "lib_version_major"
#define GTL_KEY_LIB_VERSION_MINOR "lib_version_minor"

// the state of the GTL lib : GTL_STATE_NOT_INITIALIZED,...
#define GTL_KEY_LIB_STATE "lib_state"
// Lot ID: must be set before gtl_open/init
#define GTL_KEY_LOT_ID "lot_id"

// Maximum number of active sites: must be upper than 0. If the number of active sites vary over the whole test session,
// just give the maximum: example: on a quad site tester, 4, even if not all sites will be used at every runs.
// MUST be set for gtl_open() to work.
#define GTL_KEY_MAX_NUMBER_OF_ACTIVE_SITES "max_number_of_active_sites"

// Set the NTP server to be used by the command "ntp_query"
#define GTL_KEY_NTP_SERVER "ntp_server"

// The path and filename of the recipe file: exple: /mypath/my_recipe.csv
// MUST be set for gtl_open() to work.
#define GTL_KEY_RECIPE_FILE "recipe_file"
// Sites numbers: example: "0 1 2 3 4" or "1 2 3 4 5 6 7 8"...
// Numbers must be separated by a space
// MUST be set for gtl_open() to work.
#define GTL_KEY_SITES_NUMBERS "sites_numbers"

// max size of messages stack in byte: example: 100240 to have a stack of 100 messages max
#define GTL_KEY_MAX_MESSAGES_STACK_SIZE "max_messages_stack_size"
// Test mode code (such as production, maintenance, debug, ...)
// Usually a 1 character string: A = AEL (Automatic Edge Lock) mode, C = Checker mode, D = Development / Debug test mode
// E = Engineering mode (same as development), M = Maintenance test mode, P = Production test mode, Q = Quality Control, ...
#define GTL_KEY_MODE_CODE "mode_code"

// the current number of messages in the stack. Get access only, no gtl_set() possible.
#define GTL_KEY_NUM_OF_MESSAGES_IN_STACK "number_of_messages_in_stack"

// Operator name
#define GTL_KEY_OPERATOR_NAME "operator_name"

// This key must be set in order for the GTL to be initializable.
// This folder MUST be writable and the path not empty.
// In order to write in the current working dir, it is possible to simply set it to "."
// Sets the output folder where GTL will generate it's output files (log files, SQLite files...)
// Must be called before gtl open in order to make sure the very first log files are generated.
// gtl_set(GTL_KEY_OUTPUT_FOLDER, ...) will fail if GTL already initialized, to make sure files are not generated in several locations.
#define GTL_KEY_OUTPUT_FOLDER "output_folder"

// unused for the moment
#define GTL_KEY_OUTPUT_TYPE "output_type"
// unused for the moment
#define GTL_KEY_OUTPUT_OPTIONS "output_options"

// This key must be set in order for the GTL to be openable but can be overwritten later, before gtl close.
// This is the name of the GTL output file (SQLite format, containing traceablility, results,...) to be written in the output folder
// Set the short name of the GTL output file. If not set, a default name will be generated.
// This file is generated in the folder set by gtl_set("output_folder",...)
// This should not contain a full path, just the filename: exple: "my_product_lot.sqlite". This file WILL be generated in the GTL_KEY_OUTPUT_FOLDER.
#define GTL_KEY_OUTPUT_FILENAME "outputfile_name"

// Production info:
// product_id : must set before gtl open
#define GTL_KEY_PRODUCT_ID "product_id"
// GTL profiler. WIP. Available on demand only.
#define GTL_KEY_PROFILER "profiler"

// Query to be executed on the internal GTL database thanks to the gtl_command("query").
// Could be slow. Dangerous to use in production.
#define GTL_KEY_QUERY "query"
#define GTL_KEY_QUERY_OUTPUT_FILE "query_output_file"

// Retest key to be set or not if the upcoming session ???
//#define GTL_KEY_RETEST "retest"

// Define the condition under which limits should be retrieved using the gtl_get_xpat_limits(...) functions
// Possible values are:
// "always": always return XPAT limits if some are available
// "if_changed": only return XPAT limits if they changes since last call to gtl_get_xpat_limits(...)
#define GTL_KEY_LIMITS_GET_CONDITION "limits_get_condition"

// Lot Retest Code. Indicates whether the lot of parts has been previously tested under the
// same test conditions. Recommended values are:
// Y = Entire lot has been retested
// N = Lot has not been previously tested
// 0 - 9 = Number of times lot has been previously tested.
// Default : empty space: ' '
#define GTL_KEY_RETEST_CODE "retest_code"
// In case of disconnection, reconnection to a server mode : on, off, ...
#define GTL_KEY_RECONNECTION_MODE "reconnection_mode"
// Retested hard bins: to be set before gtl_command("retest"). Optional. Will be written in xt_splitlot table.
// Standard usual values: all_pass, all_fail, all, all_fails_no_dpat, or a list of bin numbers,
#define GTL_KEY_RETEST_HBINS "retest_hbins"
// Rom code ID: will be written into xt_splitlot.rom_cod
#define GTL_KEY_ROM_CODE "rom_code"

// node info
#define GTL_KEY_STATION_NUMBER "station_number"
// Test specification name
#define GTL_KEY_SPEC_NAME "spec_name"
// Test specification version number.
#define GTL_KEY_SPEC_VERSION "spec_version"
// Test setup ID. Will be written into xt_splitlot table
#define GTL_KEY_SETUP_ID "setup_id"
// Site state (off, baseline, dpat, ...)
#define GTL_KEY_SITE_STATE "site_state"
// Only used to trace the sockets by creating some files. For debug purpose only.
#define GTL_KEY_SOCKET_TRACE "socket_trace"
// Splitlot ID: a 'splitlot' is one of the splits of a lot: could be retest or not.
// Set internally by GTL, cannot be overwritten i.e. "gtl_set()", read access only (gtl_get()).
// Usually start from 1 for initial test, incremented at each splitlots (retest or not)
#define GTL_KEY_SPLITLOT_ID "splitlot_id"
// sometimes also called sublot id. Will be written in the xt_splitlot table.
#define GTL_KEY_SUBLOT_ID "sublot_id"
// GTL_KEY_SUBLOT_NUMBER DEPRECATED in V4.2
#define GTL_KEY_SUBLOT_NUMBER "sublot_number"
// TRACEABILITY_MODE : on or off
//#define GTL_KEY_TRACEABILITY_MODE "traceability_mode" // Removed: 'TRACEABILITY' can not be turned off anymore.

// This key allows to manually set the temp folder to be used by GTL.
// By default, it is set to the value of any of these environment variables:
// (ordered): TMPDIR , TMP, TEMP.
// If none are found/set by the OS/user/testprogram, GTL will refuse to start unless you manually set it.
#define GTL_KEY_TEMP_FOLDER "temp_folder"
// Test Conditions code. A user-defined field specifying the phase of device testing or test
// conditions: for example, "WAFER" for wafertest, "CHAR" for characterization or "HOT" for hot test....
#define GTL_KEY_TEST_CODE "test_code"
// Tester name (sometimes called node name)
#define GTL_KEY_TESTER_NAME "tester_name"
// Testing stage : WS, FT, ... default is FT
#define GTL_KEY_TESTING_STAGE "testing_stage"
// Tester type
#define GTL_KEY_TESTER_TYPE "tester_type"
// Tester executive software type
#define GTL_KEY_TESTER_EXEC_TYPE "tester_executive_type"
// Tester executive software version
#define GTL_KEY_TESTER_EXEC_VERSION "tester_executive_version"
// Test temp
#define GTL_KEY_TEST_TEMPERATURE "test_temperature"
// User text: will be written into xt_splitlot
#define GTL_KEY_USER_TXT "user_text"
// User name
#define GTL_KEY_USER_NAME "user_name"
// Fields to match
#define GTL_KEY_FIELDS_TO_MATCH "fields_to_match"
// Provides a way to disable enable auto limit reload: on, off
#define GTL_KEY_RELOAD_LIMITS "reload_limits"

// GTL commands to be used through gtl_command(...) function:

// Open: opens/initializes a GTL session: a test of a splitlot, either a full or part of a lot.
// was gtl_init(...) in GTL 3.x API
//    It returns GTL_CORE_ERR_OKAY if the initialization was successful, GTL_CORE_ERR_XXX else.
//    It has to be called before the first part is tested, while the test program load.
//    Mandatory keys to be set before trying to open are:
// - ConfigFile: must give ip and port of server to connect to
// - Recipe: can be null/empty : if so the recipe will be searched on the server
// - MaxNumberOfActiveSites: must be upper than 0
// - SitesNumbers : table of int, example: 0, 1, 2, 3, 4, 5, ...
// - max messages stack size : max size in memory (in octet) reserved for messages received from the server. Currently, one message use 1032 octet.
//   returns GTL_CORE_ERR_OKAY on success, else another result
#define GTL_COMMAND_OPEN "open"

// Close the current GTL sesssion ("gtl_close()" in previous versions)
// Close the current sesssion including the network connection with server.
// End the splitlot at the same time.
// The user needs to call the "open" command in order to start a new spitlot.
// Returns usual status code.
#define GTL_COMMAND_CLOSE "close"

// trigger the start of a retest : to be called :
// - before gtl open for a multi session usage (through different GTLM/GTL sessions)
// - after a gtl endjob for on the fly usage (within the same GTM/GTL session)
#define GTL_COMMAND_RETEST "retest"
// Execute a query on the internal SQLite DB. Limited to query starting with "select".
#define GTL_COMMAND_QUERY "query"

// begin a job i.e. a run : a test on 1 or several sites. Similar to gtl_beginjob()
// This function informs the GTL that a run has started.
// It has to be called at the beginning of each run, before the first test is executed.
// returns GTL_CORE_ERR_OKAY on success, else another code.
#define GTL_COMMAND_BEGINJOB "beginjob"

// End a job/run (was gtl_endjob() in GTL version 4)
// This function informs the GTL that a run is about to finish.
// It has to be called at the end of each run.
// It has to be called after all tests have been executed and after the binning has been set.
// Returns usual status code.
// This function will send all previous unsent test results to the server.
#define GTL_COMMAND_ENDJOB "endjob"

// Clear the message stacks  (was gtl_clear_messages_stack() in previous versions)
// Warning : will delete permanently all messages, whatever their severity.
#define GTL_COMMAND_CLEAR_MESSAGES_STACK "clear_messages_stack"

// pop the first/last message in the stack if any.
// To be used with gtl_get GTL_KEY_CURRENT_MESSAGE, GTL_KEY_CURRENT_MESSAGE_SEV. See example in unit test.
#define GTL_COMAND_POP_FIRST_MESSAGE "pop_first_message"
#define GTL_COMAND_POP_LAST_MESSAGE "pop_last_message"
//! \brief Query the specified NTP (NetworkTimeProtocol) server to retrieve the time as returned by the NTP server
//! \brief Usefull to detect whether the tester clock/time is in sync with the real world time
//! \brief This feature in in WIP and not officially supported.
#define GTL_COMAND_NTP_QUERY "ntp_query"

// add this define in order to use the dynamic version of GTL.
// Then, dont forget to link with libgtl.a/lib instead of gtl_core.a
#ifdef DYNGTL
    #define gtl_set d_gtl_set
    #define gtl_get d_gtl_get
    #define gtl_beginjob d_gtl_beginjob
    #define gtl_binning d_gtl_binning
    #define gtl_set_binning d_gtl_set_binning
    #define gtl_command d_gtl_command
    //#define gtl_get_number_messages_in_stack d_gtl_get_number_messages_in_stack
    //#define gtl_get_site_state d_gtl_get_site_state
    //#define gtl_init d_gtl_init
    #define gtl_log d_gtl_log
    //#define gtl_pop_first_message d_gtl_pop_first_message
    //#define gtl_pop_last_message d_gtl_pop_last_message
    //#define gtl_set_node_info d_gtl_set_node_info
    //#define gtl_set_prod_info d_gtl_set_prod_info
    #define gtl_test d_gtl_test
    #define gtl_mptest d_gtl_mptest
    #define gtl_get_spat_limits d_gtl_get_spat_limits
    #define gtl_get_dpat_limits d_gtl_get_dpat_limits
#endif


/* PUBLIC Functions                                                                     */
/*
    gtl_set(key, value):
        This function allows to set some variables of the GTL. Examples:
        - logfile_name : the name of the log file if desired
        - node_name : must be set before init

    gtl_get(key, value):
        This function allows to retrieve the value of a variable of the GTL.
        The keys are the same as the set function even if some variables are read-only i.e. not allowed to be set.
        Warning: The value string must be allocated by the user and large enough to receive the value of the variable:
        example: char MyValue[2048]=""; gtl_get(GTL_KEY_XXX, MyValue);


    gtl_command (work in progress):
        Some arguments can be passed to the function to interact with the GTL allowing dynamic toggling of many features,
        as well as forcing actions (eg: reset baseline learning phase, enable/disable datalog, etc):
        - all commands as defined in all GTL_COMMAND_XXXXXX defines
        -gtl_debugon : sets the debug mode; advanced trace and information details are reported
        -gtl_debugoff : disables debug mode
        -gtl_quieton : Set 'Quiet' mode, so no messages reported to console
        -gtl_quietoff : Disable 'Quiet' mode, display all messages to console
        -gtl_info : report full status info (PAT running mode, status, etc)
        -gtl_help : report all available options and their meaning to the console
        -gtl_status : report status mode (eg: running baseline, PAT disabled, etc . . .)
        -gtl_test <test number> : display PAT settings for given test#
        -gtl_testlist <test list> : display PAT settings for given tests

    gtl_test(site_no, test_number, test_name, test_result):
        This function set the measurement value to/inside the GTL buffer for the specified test on specified site.
        It should be called for each test under PAT.
        If called several times in the same run for the same test number/name, the previous result is overwritten.
        It can be called for non PAT test but then the GTL will just ignore the call,
            not using unusefull memory nor sending unusefull packets over the network but these tests.
        This function in any cases will have to search for test index in the current tests list
            to be able to write testresults into the internal results buffer. This search has a complexity (big O) of n.
        Does not send results to server immediately (will be done later) and does not do any network tasks.
        Could fail if:
        - beginjob not call prior (GTL_CORE_ERR_BEGINJOB_NOT_EXEC)
        - lib not initialized (GTL_CORE_ERR_LIB_NOT_INITIALIZED)
        - test does not exist in the test list (GTL_CORE_ERR_INVALID_TEST)
        If fail, GTL does not change state, it will just continue as usual.
        Returns usual status code.

    gtl_mptest(site_no, test_number, test_name, test_results, pin_indexes, nb_of_pins)
        This function set the measurement values to/inside the GTL buffer for the specified test on specified site
        and given pin indexes (as given in the recipe, usually starting from 0).
        If called several times in the same run for the same test number/name/pins, the previous results are overwritten.
        It can be called for non PAT tests (not appearing in the recipe) but then the GTL will just ignore the call,
            not using unusefull memory nor sending unusefull packets over the network for these 'unknown' tests.
        This function in any cases will have to search for a test index in the current tests list
            to be able to write testresults into the internal results buffer. This search has a complexity (big O) of n.
        Does not send results to server immediately (will be done later) and does not do any network tasks.
        Could fail if:
        - beginjob not call prior (GTL_CORE_ERR_BEGINJOB_NOT_EXEC)
        - lib not initialized (GTL_CORE_ERR_LIB_NOT_INITIALIZED)
        - test or pin indexes does not exist in the test list (GTL_CORE_ERR_INVALID_TEST)
        If fail, GTL does not change state, it will just continue as usual.
        Returns usual status code.

    gtl_binning():
        To be called at the end of each test of part with binning information.
        This function allows to retrieve the binning of the part under test on the specified site from the GTL.
        The binning affected by the test program has to be given in order to trace original bin.
        The bin to assigned to the DUT will be in the argument: int *nNewBinning:
        this is the new binning to be affected to this part.
        This bin number is:
        - either the same unchanged binning if the part current binning is already a FAIL binning or if no test failed the PAT limits on that site,
        - the overloaded PAT binning (defined in the recipe) if the current binning is PASS, but at least one test failed the PAT limits.
        It has to be called :
        - after all tests have been executed and
        - before the binning is sent to the handler and/or tester OS.
        Calling this function twice for the same site/part will return GTL_CORE_ERR_GTL_BIN_ALREADY_CALLED.
        Returns usual status code.

    gtl_set_binning():
        This function should be called only in case the PAT limits have been pushed to the test program/tester os and used
        by them. This function will inform the GTL about the binning decided for current site (eventually a PAT binning)
        so that the GTL can add the required information into the traceability file in case of outlier / PAT binning.
        If the GTL is managing the comparison to PAT limits, then the gtl_binning() function should be called instead.
        To be called at the end of each test of part with binning information.
        The binning affected by the test program has to be given in order to trace original bin.
        It has to be called :
        - after all tests have been executed and
        Calling this function twice for the same site/part will return GTL_CORE_ERR_GTL_BIN_ALREADY_CALLED.
        Returns usual status code.

    gtl_pop_first/last_message(...):
        Poping messages : the first in the stack is the oldest, the last the more recent one.
		The severity could be: 	<0 = critical, 0 = warning, >0 = notice
        The user is responsible to allocate enough space for the 'message' string to receive a message string of 1024 chars max.
        Returns OK or GTL_CORE_ERR_NO_MORE_MESSAGE.
		
*/

#ifdef GTLCORE_MODULE
    int gtl_set(const char* lKey, char* lValue);
    int gtl_get(const char* lKey, char* lValue);
    // gtl_command("beginjob");
    int gtl_beginjob();
    int gtl_binning(unsigned int site_nb, int original_hbin, int original_sbin, int* new_hbin, int* new_sbin,
                     const char* part_id);
    int gtl_set_binning(unsigned int site_nb, int hbin, int sbin, const char* part_id);

    // execute a single GTL command.
    int gtl_command(const char *command);

    // DEPRECATED in 4.0: use gtl_get("lib_version"...) instead
    // int gtl_get_lib_version(int *major, int *minor);
    // int  gtl_get_limit_type(const int site_nb); // Not yet implemented
    // DEPRECATED in 4.0: use gtl_get(...) instead
    //int gtl_get_number_messages_in_stack();

    // gtl_set("desired_site", n); and then gtl_get("site_state");
    //int gtl_get_site_state(const int site_nb); // to do: add parameter int* site_state);

    // replace gtl_init(...). Call the needed gtl_set() before calling gtl_open().
    //int gtl_open();
    // DEPRECATED in 4.0: use instead gtl_set(...) and then gtl_command("open").
    /* int gtl_init(const char *FullConfigFileName, const char *szFullRecipeFileName, int MaxNumberOfActiveSites,
                 int* SitesNumbers, const int MaxMessageStackSize);
    */
    /* To be used through GTL_LOG macro */
    int gtl_log(int severity, char* function, char* file, int line_no, char* message, ...);
    //int gtl_pop_first_message(int* severity, char* message, int* messageID);
    //int gtl_pop_last_message(int* severity, char* message, int* messageID);
    // DEPRECATED in 4.0: use several gtl_set(...) instead
    //int gtl_set_node_info(unsigned int StationNumber, char* HostID, char* NodeName, char* UserName, char* TesterType, char* TesterExec, char* TestJobName, char* TestJobFile, char* TestJobPath, char* TestSourceFilesPath);
    // DEPRECATED in 4.0: use several gtl_set(...) instead
    //int gtl_set_prod_info(char* OperatorName, char* JobRevision, char* LotID, char* SublotID, char* ProductID);
    int gtl_test(unsigned int SiteNb, long TestNb, char *TestName, double Result);
    int gtl_mptest(unsigned int SiteNb, long TestNb, char *TestName, double* TestResults, int* PinIndexes, long ArraysSize);
    int gtl_get_spat_limits(unsigned int SiteNb, long* TestNumbers, unsigned int* Flags, double* LowLimits, double* HighLimits, unsigned int* HardBins, unsigned int* SoftBins, unsigned int ArrayTotalSize, unsigned int* ArrayFilledSize);
    int gtl_get_dpat_limits(unsigned int SiteNb, long* TestNumbers, unsigned int* Flags, double* LowLimits, double* HighLimits, unsigned int* HardBins, unsigned int* SoftBins, unsigned int ArrayTotalSize, unsigned int* ArrayFilledSize);
#else
    #if defined(__cplusplus)
        extern "C"
        {
            int gtl_set(const char* lKey, char* value);
            int gtl_get(const char* lKey, char* lValue);
            int gtl_beginjob();
            int gtl_binning(unsigned int site_nb, int original_hbin, int original_sbin, int* new_hbin, int* new_sbin,
                        const char* part_id);
            int gtl_set_binning(unsigned int site_nb, int hbin, int sbin, const char* part_id);
            int gtl_command(char *command);
            //int gtl_get_lib_version(int *major, int *minor);
            int gtl_get_limit_type(const int site_nb);
            //int gtl_get_number_messages_in_stack();
            //int gtl_get_site_state(const int site_nb);
            //int gtl_open();
            //int gtl_init(char *FullConfigFileName, char *szFullRecipeFileName, int MaxNumberOfActiveSites,
            //             int* SitesNumbers, const int MaxMessageStackSize);
            /* To be used through GTL_LOG macro */
            int gtl_log(int severity, char* function, char* file, int line_no, char* message, ...);
            //int gtl_pop_first_message(int* severity,char* message, int* messageID);
            //int gtl_pop_last_message(int* severity, char* message, int* messageID);
            //int gtl_set_node_info(unsigned int StationNumber, char* HostID, char* NodeName, char* UserName,
              //    char* TesterType, char* TesterExec, char* TestJobName, char* TestJobFile, char* TestJobPath,
              //    char* TestSourceFilesPath);
            //int gtl_set_prod_info(char* OperatorName, char* JobRevision, char* LotID, char* SublotID, char* ProductID);
            int gtl_test(unsigned int uiSiteNb, long lTestNb, char *szTestName, double lfResult);
            int gtl_mptest(unsigned int SiteNb, long TestNb, char *TestName, double* TestResults, int*, long ArraySize);
            int gtl_get_spat_limits(unsigned int SiteNb, long* TestNumbers, unsigned int* Flags, double* LowLimits, double* HighLimits, unsigned int* HardBins, unsigned int* SoftBins, unsigned int ArrayTotalSize, unsigned int* ArrayFilledSize);
            int gtl_get_dpat_limits(unsigned int SiteNb, long* TestNumbers, unsigned int* Flags, double* LowLimits, double* HighLimits, unsigned int* HardBins, unsigned int* SoftBins, unsigned int ArrayTotalSize, unsigned int* ArrayFilledSize);
        }
    #else
        extern int gtl_set();
        extern int gtl_get();
        extern int gtl_beginjob();
        extern int gtl_binning();
        extern int gtl_set_binning();
        extern int gtl_command();
        //extern int gtl_get_lib_version();
        extern int gtl_get_limit_type();
        //extern int gtl_get_number_messages_in_stack();
        //extern int gtl_get_site_state();
        //extern int gtl_open();
        //extern int gtl_init();
        /* To be used through GTL_LOG macro */
        extern int gtl_log();
        //extern int gtl_pop_first_message();
        //extern int gtl_pop_last_message();
        //extern int gtl_set_node_info();
        //extern int gtl_set_prod_info();
        extern int gtl_test();
        extern int gtl_mptest();
        extern int gtl_get_spat_limits();
        extern int gtl_get_dpat_limits();
    #endif /* #if defined(__cplusplus) */
#endif /* #ifdef GTLCORE_MODULE */

/*
  Convenient optional log function : logs both to file and syslog network if possible
  Example: GTL_LOG(6, "Hello world !", 0);
  To fullfill variable argument syntax, always add a last arg of 0 even if no argument used.
  Levels : 7=debug 6=information 5=notice 4=warning 3=error 2=critical 1=alert 0=emergency
  Check me: do we have __PRETTY_FUNCTION__ on all testers ? or __func__ ? and __VERSION__ ?
*/
#if defined(_MSC_VER) || defined(__BORLANDC__)
	#define GTL_LOG
#else
	#define GTL_LOG(sev, message, ...) gtl_log((sev), ((char*)__func__), ((char*)__FILE__), (__LINE__), (message), __VA_ARGS__);
#endif

#endif /* #ifndef _GTL_CORE_H_ */
