///////////////////////////////////////////////////////////
// Constants in Examinator-Monitoring
///////////////////////////////////////////////////////////
#ifndef GEXMO_CONSTANTS
#define	GEXMO_CONSTANTS	1

///////////////////////////////////////////////////////////
// Structures declared in gexmo_constants.cpp
///////////////////////////////////////////////////////////
#ifndef GEXMO_CONSTANTS_CPP
extern const char* gexMoLabelTaskFrequency[];
extern const char* gexMoLabelTaskReportEraseFrequency[];
extern const char* gexMoLabelTaskFrequencyDayOfWeek[];
extern const char* gexMoLabelSpecInfo[];
#endif

// Examinator Monitoring: creating tasks
#define	GEXMO_TASK_DATAPUMP             1
#define	GEXMO_TASK_YIELDMONITOR         2
#define	GEXMO_TASK_YIELDMONITOR_RDB     3
#define	GEXMO_TASK_REPORTING            5
#define	GEXMO_TASK_STATUS               6
#define GEXMO_TASK_CONVERTER            7
#define GEXMO_TASK_OUTLIER_REMOVAL      8
#define	GEXMO_TASK_AUTOADMIN            9
#define	GEXMO_TASK_TRIGGERPUMP          10
#define	GEXMO_TASK_PATPUMP              11
#define GEXMO_TASK_INCREMENTAL_UPDATE   12
#define GEXMO_TASK_SPM                  13
#define GEXMO_TASK_SYA                  14
#define GEXMO_TASK_OLD_DATAPUMP         99

// Filter string used to import data files...Includes COMPRESSED formats!
#define	GEX_VALID_TRIGGERFILES_EXT  "*.gtf"

// Database import mode: Copy or FTP
#define	GEXMO_FILE_COPY         0		// Copy files
#define	GEXMO_FILE_FTP          1		// FTP files

// Task Frequency

#define	GEXMO_TASKFREQ_1MIN             0		// Task every: 1 minute
#define	GEXMO_TASKFREQ_2MIN             1		// Task every: 2 minutes
#define	GEXMO_TASKFREQ_3MIN             2		// Task every: 3 minutes
#define	GEXMO_TASKFREQ_4MIN             3		// Task every: 4 minutes
#define	GEXMO_TASKFREQ_5MIN             4		// Task every: 5 minutes
#define	GEXMO_TASKFREQ_10MIN            5		// Task every: 10 minutes
#define	GEXMO_TASKFREQ_15MIN            6		// Task every: 15 minutes
#define	GEXMO_TASKFREQ_30MIN            7		// Task every: 30 minutes
#define	GEXMO_TASKFREQ_1HOUR            8		// Task every: 1 hour
#define	GEXMO_TASKFREQ_2HOUR            9		// Task every: 2 hours
#define	GEXMO_TASKFREQ_3HOUR            10		// Task every: 3 hours
#define	GEXMO_TASKFREQ_4HOUR            11		// Task every: 4 hours
#define	GEXMO_TASKFREQ_5HOUR            12		// Task every: 5 hours
#define	GEXMO_TASKFREQ_6HOUR            13		// Task every: 6 hours
#define	GEXMO_TASKFREQ_12HOUR           14		// Task every: 12 hours
#define	GEXMO_TASKFREQ_1DAY             15		// Task every: 1 day
#define	GEXMO_TASKFREQ_2DAY             16		// Task every: 2 days
#define	GEXMO_TASKFREQ_3DAY             17		// Task every: 3 days
#define	GEXMO_TASKFREQ_4DAY             18		// Task every: 4 days
#define	GEXMO_TASKFREQ_5DAY             19		// Task every: 5 days
#define	GEXMO_TASKFREQ_6DAY             20		// Task every: 6 days
#define	GEXMO_TASKFREQ_1WEEK            21		// Task every: 1 week
#define	GEXMO_TASKFREQ_2WEEK            22		// Task every: 2 weeks
#define	GEXMO_TASKFREQ_3WEEK            23		// Task every: 3 weeks
#define	GEXMO_TASKFREQ_1MONTH           24		// Task every: 1 month

// Auto Admin: Report Auto Erase Frequency
#define	GEXMO_RPTERASE_FREQ_1DAY        0		// Erase reports older than: 1 day
#define	GEXMO_RPTERASE_FREQ_2DAY        1		// Erase reports older than: 2 days
#define	GEXMO_RPTERASE_FREQ_3DAY        2		// Erase reports older than: 3 days
#define	GEXMO_RPTERASE_FREQ_4DAY        3		// Erase reports older than: 4 days
#define	GEXMO_RPTERASE_FREQ_5DAY        4		// Erase reports older than: 5 days
#define	GEXMO_RPTERASE_FREQ_6DAY        5		// Erase reports older than: 6 days
#define	GEXMO_RPTERASE_FREQ_1WEEK       6		// Erase reports older than: 1 week
#define	GEXMO_RPTERASE_FREQ_2WEEK       7		// Erase reports older than: 2 weeks
#define	GEXMO_RPTERASE_FREQ_3WEEK       8		// Erase reports older than: 3 weeks
#define	GEXMO_RPTERASE_FREQ_1MONTH      9		// Erase reports older than: 1 month
#define	GEXMO_RPTERASE_FREQ_2MONTH      10		// Erase reports older than: 2 month
#define	GEXMO_RPTERASE_FREQ_3MONTH      11		// Erase reports older than: 3 month
#define	GEXMO_RPTERASE_FREQ_4MONTH      12		// Erase reports older than: 4 month
#define	GEXMO_RPTERASE_FREQ_5MONTH      13		// Erase reports older than: 5 month
#define	GEXMO_RPTERASE_FREQ_6MONTH      14		// Erase reports older than: 6 month
#define	GEXMO_RPTERASE_FREQ_NEVER       15		// Never erase reports on the server

// Spec Info to monitor
#define	GEXMO_SPECINFO_VALUE            0		// Monitor 'Parameter Value'
#define	GEXMO_SPECINFO_CP               1		// Monitor 'Cp'
#define	GEXMO_SPECINFO_CPK              2		// Monitor 'Cpk'
#define	GEXMO_SPECINFO_MEAN             3		// Monitor 'Mean'
#define	GEXMO_SPECINFO_RANGE            4		// Monitor 'Range'
#define	GEXMO_SPECINFO_SIGMA            5		// Monitor 'Sigma'
#define	GEXMO_SPECINFO_PASSRATE         6		// Monitor 'Yield/Passing %rate'
#define	GEXMO_SPECINFO_SPC_7_SIDE       7		// Monitor 'SPC/AIAG: Run of 7 points on one side of the mean'
#define	GEXMO_SPECINFO_SPC_7_TREND      8		// Monitor 'SPC/AIAG: Trend of 7 points up or down'

// Offset of parameters in .CSV Log file <databases_path>/.monitor/<yyymmdd>.dat
// File that lists all data files processed in the day.
#define	GEXMO_LOT_PROCESSED_STARTTIME   0
#define	GEXMO_LOT_PROCESSED_PRODUCT     1
#define	GEXMO_LOT_PROCESSED_LOT         2
#define	GEXMO_LOT_PROCESSED_SUBLOT      3
#define	GEXMO_LOT_PROCESSED_NODENAME    4
#define	GEXMO_LOT_PROCESSED_OPERATOR    5
#define	GEXMO_LOT_PROCESSED_JOBNAME     6
#define	GEXMO_LOT_PROCESSED_JOBREV      7
#define	GEXMO_LOT_PROCESSED_GOODPARTS   8
#define	GEXMO_LOT_PROCESSED_TOTALPARTS  9
#define	GEXMO_LOT_PROCESSED_WAFER       10

// Type of HTML Monitoring page to create
#define	GEXMO_REPORTPAGE_NOHISTORY      -1
#define	GEXMO_REPORTPAGE_MASTERHOME     0		// Index.htm home page for all web site
#define	GEXMO_REPORTPAGE_HOME           1		// Home.htm home page per child-site
#define	GEXMO_REPORTPAGE_PRODUCTS       2		// Products page per child-site
#define	GEXMO_REPORTPAGE_TESTERS        3		// Testers page per child-site
#define	GEXMO_HISTORYPAGE_PRODUCTS      4		// History products page per child-site
#define	GEXMO_HISTORYPAGE_TESTERS       5		// History testers page per child-site

// Web site status: Organization type
#define	GEXMO_STATUS_WEB_MERGED         0		// Create 1 common web site for ALL databases
#define	GEXMO_STATUS_WEB_INDIVIDUAL     1		// Create 1 web site per databases + one home page with links to them.

// Insertion status
#define GEXMO_INSERTION_STATUS_OK       "Inserted"
#define GEXMO_INSERTION_STATUS_NOK      "Rejected"
#define GEXMO_INSERTION_STATUS_DELAY    "Delayed"

// PostImport task: what to do with original data files...
#define	GEXMO_POSTIMPORT_RENAME         0		// Rename files to '.read' after import
#define	GEXMO_POSTIMPORT_DELETE         1		// Delete files after import
#define	GEXMO_POSTIMPORT_LEAVE          2		// Leave files on server afetr import: Monitoring updates a list of files processed...
#define	GEXMO_POSTIMPORT_MOVE           3		// Move file to alternate folder
#define	GEXMO_POSTIMPORT_FTP            4		// FTP file (move file a temporarly folder + create trigger file for FTP side application)

// PostImport Failure: what to do with BAD files
#define	GEXMO_BAD_POSTIMPORT_RENAME     0	// Rename bad files to '.bad'
#define	GEXMO_BAD_POSTIMPORT_MOVE       1	// Move bad files to a custom folder

// PostImport Delay: what to do with DELAYED files
#define	GEXMO_DELAY_POSTIMPORT_LEAVE    0	// Leave delayed files unchanged
#define	GEXMO_DELAY_POSTIMPORT_RENAME   1	// Rename delayed files to '.delay'
#define	GEXMO_DELAY_POSTIMPORT_MOVE     2	// Move delayed files to a custom folder

// File Conversion dialog: Defines about what to do with source files converted
#define GEXMO_TASK_CONVERTER_SUCCESS_RENAME     0
#define GEXMO_TASK_CONVERTER_SUCCESS_DELETE     1
#define GEXMO_TASK_CONVERTER_SUCCESS_MOVE       2

#define GEXMO_TASK_CONVERTER_FAIL_RENAME        0
#define GEXMO_TASK_CONVERTER_FAIL_MOVE          1

#define	GEXMO_TASK_CONVERTER_OUTPUT_STDF        0	// Output format = STDF
#define	GEXMO_TASK_CONVERTER_OUTPUT_CSV         1	// Output format = CSV
#define	GEXMO_TASK_CONVERTER_OUTPUT_ATDF        2	// Output format = ATDF

// PostProcessing Crash: what to do with files when Gex crashs
#define	GEXMO_QUARANTINE_POSTCRASH_RENAME       0	// Rename bad files to '.quarantine'
#define	GEXMO_QUARANTINE_POSTCRASH_MOVE         1	// Move bad files to a custom folder


#endif	// Only if GEXMO_CONSTANTS doesn't exist yet !
