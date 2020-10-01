#ifndef GEXLM_H
#define GEXLM_H

// Examinator DataType allowed to process
#define	GEX_DATATYPE_ALLOWED_ANY			0		// Examinator: Allow any type of test data.
#define	GEX_DATATYPE_ALLOWED_CREDENCE		1		// Examinator: Allow All Credence testers 
#define	GEX_DATATYPE_ALLOWED_SZ				2		// Examinator: Allow SZ data only
#define	GEX_DATATYPE_ALLOWED_TERADYNE		3		// Examinator: Allow Teradyne data only
#define	GEX_DATATYPE_ALLOWED_DATABASE		4		// Examinator-Pro : Allow any files from database.
#define	GEX_DATATYPE_ALLOWED_DATABASEWEB	5		// ExaminatorWeb: Allow any files from database.
#define	GEX_DATATYPE_GEX_MONITORING			6		// Examinator-Monitoring
#define	GEX_DATATYPE_GEX_TOOLBOX			7		// Examinator-ToolBox: STDF converter, Dump, etc...
#define	GEX_DATATYPE_ALLOWED_LTX			8		// Examinator-OEM for LTX
#define	GEX_DATATYPE_ALLOWED_CREDENCE_ASL	9		// Examinator-OEM for Credence-ASL
#define	GEX_DATATYPE_GEX_SCORE				10		// G-Score (Galaxy-Score): Generates score for a given file.

#ifdef _WIN32
#define	strOSplatform	"[PC-Windows]"
#endif
#ifdef __unix__
	// UNIX Platform
	#ifdef __sun__
		#define	strOSplatform	"[Sun-Solaris]";
	#endif
	#ifdef hpux
		#define	strOSplatform	"[HP-UX]";
	#endif
	#ifdef __linux__
		#define	strOSplatform	"[Linux]";
	#endif
#endif
// Socket timer info.
#define	CHECK_ALIVE		60000	// Checks every 60,000ms for nodes alive.
#define	MAX_ALIVE_WAIT	1800	// If a node gives no sign after 180sec, consider it dead.

// Used for encrypting data.
#define GEX_CRYPTO_KEY		"(gex@galaxysemi.com)"
#define GEX_KEY_BYTES		12

#endif // ifdef GEXLM_H
