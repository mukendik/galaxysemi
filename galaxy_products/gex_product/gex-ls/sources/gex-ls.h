///////////////////////////////////////////////////////////
// Constants used in the GEX-LS project
///////////////////////////////////////////////////////////
#ifndef GEX_LS_CONSTANTS
#define	GEX_LS_CONSTANTS	1

#ifndef BYTE
  #define BYTE unsigned char
#endif

// Configuration file
#define	GEXLS_CONFIG_FILE				"/.gexls_config.conf"
#define	GEXLS_REPORT_FILE				"/.gexls_report.htm"
#define	GEXLS_REPORT_DIR				"/gexls_report"

// HTML		
#define C_STDF_COPYRIGHT				"Copyright (c) 2000-2008 Galaxy"
#define C_HTML_FOOTER					"<br><br><br><hr>\n<font color=\"#006699\">Report created with: %1 - www.mentor.com</font>"

// Color string for HTML report
#define HTML_COLOR_TEXT					"#000000"
#define HTML_COLOR_BACKGROUND			"#FFFFFF"

// GEX Client status monitored on socket
#define	GEX_CLIENT_SOCKET_IDLE			1		// client not currently in talks with server
#define	GEX_CLIENT_TRY_CONNECT			2		// trying to connect to server
#define	GEX_CLIENT_CONNECTED			3		// Connection established...now refresh display every 5 seconds.
#define	GEX_CLIENT_TRY_CONNECT_TIMEOUT	15		// Connection timeout after 15secs.
#define GEX_CLIENT_REFRESH				5		// Refreshes screen every 5 seconds.

// Examinator DataType allowed to process
#define	GEX_DATATYPE_ALLOWED_ANY		0		// Examinator: Allow any type of test data.
#define	GEX_DATATYPE_ALLOWED_CREDENCE	1		// Examinator: Allow Credence data only
#define	GEX_DATATYPE_ALLOWED_SZ			2		// Examinator: Allow SZ data only
#define	GEX_DATATYPE_ALLOWED_TERADYNE	3		// Examinator: Allow Teradyne data only
#define	GEX_DATATYPE_ALLOWED_DATABASE	4		// ExaminatorDB: Allow any files from database.

#endif
