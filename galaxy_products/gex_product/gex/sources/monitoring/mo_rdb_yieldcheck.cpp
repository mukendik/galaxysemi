///////////////////////////////////////////////////////////
// Examinator Monitoring: Yield Monitoring Task dialog box
///////////////////////////////////////////////////////////
#include <qfiledialog.h>
 
#include <QDate>
#include <QDateTime>
#include <QDesktopWidget>

// Galaxy modules includes
#include <gqtl_sysutils.h>

#include "gexmo_constants.h"
#include "mo_yieldcheck.h"
#include "mo_rdb_yieldcheck.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "browser_dialog.h"
#include "pickproduct_id_dialog.h"
#include "mo_scheduler.h"
#include "mo_status.h"
#include "mo_email.h"
#include "mo_autoadmin.h"
#include "db_datakeys.h"
#include "db_transactions.h"
#include "tb_toolbox.h"
#include "mo_rdb_sylsbl_editor_dialog.h"
#include "report_options.h"
#include <gqtl_log.h>


// main.cpp
extern void				WriteDebugMessageFile(const QString & strMessage);
extern GexMainwindow	*pGexMainWindow;

// In activation_key.cpp
extern QDate	ExpirationDate;		// Date when release expires...
extern QDate	ReleaseDate;		// Date of release: will refuse computers with date earlier !

// report_build.cpp
extern CGexReport		*gexReport;			// Handle to report class
extern CReportOptions	ReportOptions;		// Holds options
extern CGexSkin			gGexSkin;			// holds the skin settings

// RDB Yield Tabs
#define RDB_YIELD_TAB_YL					0		// Yield Limit tab
#define RDB_YIELD_TAB_SYA					1		// SYL/SBL tab

// Basic Yield Tab
#define RDB_YIELD_EDITOR_YL_TITLE			0		// Yield task title (used when creating Yield exception email)
#define RDB_YIELD_EDITOR_YL_DATABASE		1		// Database name
#define RDB_YIELD_EDITOR_YL_PRODUCT			2		// Product name (eg: AX5578*)
#define RDB_YIELD_EDITOR_YL_RULESTATE		3		// Disabled/Enabled
#define RDB_YIELD_EDITOR_YL_TESTSTAGE		4		// Testing stage (eg: Wafer sort)
#define RDB_YIELD_EDITOR_YL_BINLIST			5		// Bins to monitor (eg: 1-7,8)
#define RDB_YIELD_EDITOR_YL_BINTYPE			6		// Bin type: Pass or Fail
#define RDB_YIELD_EDITOR_YL_MINPARTS		7		// Yield limit mode: Minimum # of parts to enable yield alarm
#define RDB_YIELD_EDITOR_YL_ALRMCOND		8		// Yield limit mode: Alarm condition: Low Yield or High Yield
#define RDB_YIELD_EDITOR_YL_YIELDLIMIT		9		// Yield limit mode: Hardcoded yield limit
#define RDB_YIELD_EDITOR_YL_EXCEPTIONTYPE	10		// Exception type: Standard or Critical
#define RDB_YIELD_EDITOR_YL_FROM			11		// Email 'From'
#define RDB_YIELD_EDITOR_YL_TO				12		// Email 'To'
#define RDB_YIELD_EDITOR_YL_EMAILFORMAT		13		// Email format : HTML or TEXT
#define RDB_YIELD_EDITOR_YL_EMAILCONTENT	14		// Email contents (eg: ASCII Report, PDF Report, etc...)
#define RDB_YIELD_EDITOR_YL_RPTLOCATION		15		// Report location (attachment or on server)
// Total columns in Yield-Limittable.
#define RDB_YIELD_EDITOR_YL_ROWS	(RDB_YIELD_EDITOR_YL_RPTLOCATION+1)	// Total Rows in table!

// SYA tab info
#define RDB_YIELD_EDITOR_SYA_TITLE				0		// Yield task title (used when creating Yield exception email)
#define RDB_YIELD_EDITOR_SYA_DATABASE			1		// Database name
#define RDB_YIELD_EDITOR_SYA_PRODUCT			2		// Product name (eg: AX5578*)
#define RDB_YIELD_EDITOR_SYA_RULESTATE			3		// Disabled/Enabled
#define RDB_YIELD_EDITOR_SYA_TESTSTAGE			4		// Testing stage (eg: Wafer sort)
#define RDB_YIELD_EDITOR_SYA_PERIOD				5		// SYL/SBL: Validity period (re-computing frequency)
#define RDB_YIELD_EDITOR_SYA_EXPIRES			6		// SYL/SBL: Rule expiration date (when expired, requires user to force a new computation)
#define RDB_YIELD_EDITOR_SYA_BINLIST			7		// Bins to monitor (eg: 1-7,8)
#define RDB_YIELD_EDITOR_SYA_RULE				8		// SYL/SBL: Rule type (N*Sigma, N*IQR...)
#define RDB_YIELD_EDITOR_SYA_N1					9		// SYL/SBL: N value
#define RDB_YIELD_EDITOR_SYA_N2					10		// SYL/SBL: N value
#define RDB_YIELD_EDITOR_SYA_LOTS				11		// SYL/SBL: Minimum lots required to compute SYL/SBL
#define RDB_YIELD_EDITOR_SYA_EXCEPTIONTYPE		12		// Exception type: Standard or Critical
#define RDB_YIELD_EDITOR_SYA_FROM				13		// Email 'From'
#define RDB_YIELD_EDITOR_SYA_TO					14		// Email 'To'
#define RDB_YIELD_EDITOR_SYA_EMAILFORMAT		15		// Email format : HTML or TEXT
#define RDB_YIELD_EDITOR_SYA_EMAILCONTENT		16		// Email contents (eg: ASCII Report, PDF Report, etc...)
#define RDB_YIELD_EDITOR_SYA_RPTLOCATION		17		// Report location (attachment or on server)
#define RDB_YIELD_EDITOR_SYA_SYL1_LL_STATE		18		// SYL1 (LL) (On or Off)
#define RDB_YIELD_EDITOR_SYA_SYL1_HL_STATE		19		// SYL1 (HL) (On or Off)
#define RDB_YIELD_EDITOR_SYA_SYL2_LL_STATE		20		// SYL2 (LL) (On or Off)
#define RDB_YIELD_EDITOR_SYA_SYL2_HL_STATE		21		// SYL2 (HL) (On or Off)
#define RDB_YIELD_EDITOR_SYA_SBL1_LL_DISABLED	22		// Disabled SBL1 (LL) (list of bins)
#define RDB_YIELD_EDITOR_SYA_SBL1_HL_DISABLED	23		// Disabled SBL1 (HL) (list of bins)
#define RDB_YIELD_EDITOR_SYA_SBL2_LL_DISABLED	24		// Disabled SBL2 (LL) (list of bins)
#define RDB_YIELD_EDITOR_SYA_SBL2_HL_DISABLED	25		// Disabled SBL2 (HL) (list of bins)
#define RDB_YIELD_EDITOR_SYA_IGNORENULLSIGMA	26		// Ignore SYL/SBL if sigma is 0 (On or Off)
#define RDB_YIELD_EDITOR_SYA_IGNOREOUTLIERS		27		// Ignore outliers (On or Off)
#define RDB_YIELD_EDITOR_SYA_USEGROSSDIE		28		// Use Gross Die (On or Off)
#define RDB_YIELD_EDITOR_SYA_MINDATAPOINTS		29		// Min datapoints to compute SYL/SBL
// Total columns in table.
#define RDB_YIELD_EDITOR_SYA_ROWS	( RDB_YIELD_EDITOR_SYA_MINDATAPOINTS+1)	// Total Rows in table!





