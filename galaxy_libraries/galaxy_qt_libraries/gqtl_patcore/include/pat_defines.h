#ifndef PAT_DEFINES_H
#define PAT_DEFINES_H

#define GEX_PAT_CONFIG			"/.galaxy_patman.conf"
#define GEX_PAT_FT_CONFIG		"/.galaxy_ft_patman.conf"
#define GEX_PAT_CONFIG_TEMP 	"/.galaxy_patman_temp.conf"

// List of environment variables supported
// PAT-Man running at specific customer site
#define PATMAN_USER_ST				"STM_PATMAN"
#define	PATMAN_STDF_ORG_LIMITS		"STDF_ORG_LIMITS"

// GUI per-Test PAT editor
#define	GEX_PAT_TEST_SETTINGS_SPAT			0	// Edit SPAT settings per Test
#define	GEX_PAT_TEST_SETTINGS_DPAT			1	// Edit DPAT settings per Test
#define	GEX_PAT_TEST_SETTINGS_NNR			2	// Edit NNR settings per Test
#define	GEX_PAT_TEST_SETTINGS_SPC			3	// Edit SPC settings per Test

// Define number of sigma constants (for startup default)
#define	GEX_TPAT_3_SIGMA		3.0
#define	GEX_TPAT_4_SIGMA		4.0
#define	GEX_TPAT_5_SIGMA		5.0
#define	GEX_TPAT_6_SIGMA		6.0
#define	GEX_TPAT_7_SIGMA		7.0
#define	GEX_TPAT_8_SIGMA		8.0
#define	GEX_TPAT_9_SIGMA		9.0
#define	GEX_TPAT_10_SIGMA		10.0
#define	GEX_TPAT_11_SIGMA		11.0
#define	GEX_TPAT_12_SIGMA		12.0
#define	GEX_TPAT_13_SIGMA		13.0
#define	GEX_TPAT_14_SIGMA		14.0

// Tail direction (if any). Power of two flags
#define	GEX_PAT_TAIL_NONE		0x0
#define	GEX_PAT_TAIL_LEFT		0x1
#define	GEX_PAT_TAIL_RIGHT		0x2

#define GEX_TPAT_KEEPTYPE_NONEID			0		// "none", Outliers to keep for a given test: none (any outlier identified is kept)
#define GEX_TPAT_KEEPTYPE_LOWID				1		// "low", Outliers to keep for a given test: low values (eg: if test is a standby current leak)
#define GEX_TPAT_KEEPTYPE_HIGHID			2		// "high",Outliers to keep for a given test: high values (eg: if test is device working frequency!)

#define	GEX_TPAT_NNR_ENABLED				0		// NNR rule: Enabled
#define	GEX_TPAT_NNR_DISABLED				1		// NNR rule: Disabled

#define	GEX_TPAT_DOUBLE_INFINITE	1e308
#define	GEX_TPAT_FLOAT_INFINITE		1e38F

#define GEX_TPAT_OPEN_SECTION_OPTIONS	"<Outlier_Options"		// Open Config file Section Header: <Outlier Options>
#define GEX_TPAT_CLOSE_SECTION_OPTIONS	"</Outlier_Options>"	// Close Config file Section Header: <Outlier Options>
#define GEX_TPAT_OPEN_SECTION_RULES		"<Outlier_Rules>"		// Open Config file Section Header: <Outlier rules>
#define GEX_TPAT_CLOSE_SECTION_RULES	"</Outlier_Rules>"		// Close Config file Section Header: <Outlier rules>


// Define how the Test definitions are accessed (key)
#define	GEX_TBPAT_KEY_TESTNUMBER	0		// Access by test number index only (default)
#define	GEX_TBPAT_KEY_TESTNAME		1		// Access by test name only
#define	GEX_TBPAT_KEY_TESTMIX		2		// Access by combining test name + test number
#define	GEX_TBPAT_KEY_DETECT		3		// as detected in PAT settings file for each test


#define GEX_TPAT_IGNOREDATA_NONEID			0		// "none", Samples to ignore for a given test: none (keep all data points, do not ignore any)
#define GEX_TPAT_IGNOREDATA_NEGATIVEID		1		// "negative", ignore negative data samples (eg: when testing Iddq / Leakage)
#define GEX_TPAT_IGNOREDATA_POSITIVEID		2		// "positive",ignore positive data samples

#define GEX_TPAT_LIMITSSET_NEAR				0		// "Near outliers", Remove all outliers, from near outliers and higher
#define GEX_TPAT_LIMITSSET_MEDIUM			1		// "Medium outliers", Remove medium outliers and higher
#define GEX_TPAT_LIMITSSET_FAR				2		// "Far outliers", Remove far outliers only

#define GEX_TPAT_RULETYPE_SIGMAID			0			// Rule 'sigma' ID (offset in combo)
#define GEX_TPAT_RULETYPE_ROBUSTSIGMAID		1			// Rule 'N*RobustSigma' ID (offset in combo)
#define GEX_TPAT_RULETYPE_Q1Q3IQRID			2			// Rule 'Q1/Q3 IQR' ID (offset in combo)
#define GEX_TPAT_RULETYPE_LIMITSID			3			// Rule '%limit' ID (offset in combo)
#define GEX_TPAT_RULETYPE_NEWLIMITSID		4			// Rule 'newlimit' ID (offset in combo)
#define GEX_TPAT_RULETYPE_RANGEID			5			// Rule 'range' ID (offset in combo)
#define GEX_TPAT_RULETYPE_SMARTID			6			// Rule 'smart' ID (offset in combo)
#define GEX_TPAT_RULETYPE_IGNOREID			7			// Rule 'disabled' ID (offset in combo)
#define GEX_TPAT_RULETYPE_SEPARATORID		8			// Rule separator
#define GEX_TPAT_RULETYPE_GAUSSIANID		9			// Rule: Gaussian distribution
#define GEX_TPAT_RULETYPE_GAUSSIANTAILID	10			// Rule: Gaussian+Tail distribution
#define GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID	11		// Rule: Gaussian+Tail distribution
#define GEX_TPAT_RULETYPE_LOGNORMALID		12			// Rule: LogNormal distribution
#define GEX_TPAT_RULETYPE_BIMODALID			13			// Rule: Bi-Modal distribution, with each mode clearly apart from the other
#define GEX_TPAT_RULETYPE_MULTIMODALID		14			// Rule: Multi-Modal distribution
#define GEX_TPAT_RULETYPE_CLAMPEDID			15			// Rule: Clamped distribution
#define GEX_TPAT_RULETYPE_DUALCLAMPEDID		16			// Rule: DoubleClamped distribution
#define GEX_TPAT_RULETYPE_CATEGORYID		17			// Rule: Categories distribution
#define GEX_TPAT_RULETYPE_CUSTOMLIBID		18			// Rule: Custom PAT library/DLL

#define	GEX_TPAT_DRIFT_UNITS_TEST			0			// Mean drift units is 'test units'
#define	GEX_TPAT_DRIFT_UNITS_SIGMA			1			// Mean drift units is 'N*Sigma'
#define	GEX_TPAT_DRIFT_UNITS_P_LIMITS		2			// Mean drift units is 'Percentage of limits'
#define	GEX_TPAT_DRIFT_UNITS_NONE			3			// Mean drift units not defined.

// Define Column-ID for informations saved in the Config file
#define	GEX_TPAT_CONFIG_FIELD_SBIN					0	// Static PAT bin column ID
#define	GEX_TPAT_CONFIG_FIELD_DBIN					1	// Dynamic PAT bin column ID
#define	GEX_TPAT_CONFIG_FIELD_TNUM					2	// Test# column ID
#define	GEX_TPAT_CONFIG_FIELD_TNAME					3	// Test name column ID
#define	GEX_TPAT_CONFIG_FIELD_LSL					4	// Test Lower Spec Limit column ID
#define	GEX_TPAT_CONFIG_FIELD_USL					5	// Test Upper Spec Limit column ID
#define	GEX_TPAT_CONFIG_FIELD_SHAPE					6	// Historical distribution shape
#define	GEX_TPAT_CONFIG_FIELD_CPK					7	// Historical Cpk
#define	GEX_TPAT_CONFIG_FIELD_RMEAN					8	// Robust Mean column ID
#define	GEX_TPAT_CONFIG_FIELD_RSIGMA				9	// Robust Sigma column ID
#define	GEX_TPAT_CONFIG_FIELD_MEAN					10	// Mean column ID
#define	GEX_TPAT_CONFIG_FIELD_SIGMA					11	// Sigma (STD DEV) column ID
#define	GEX_TPAT_CONFIG_FIELD_RANGE					12	// Range column ID
#define	GEX_TPAT_CONFIG_FIELD_IGNOREDATA			13	// 'Samples to ignore' column ID
#define	GEX_TPAT_CONFIG_FIELD_KEEP					14	// 'Outliers to keep' column ID
#define	GEX_TPAT_CONFIG_FIELD_OUTLIER_LIMIT_SET		15	// 'Limits set'
#define	GEX_TPAT_CONFIG_FIELD_RULE					16	// DPAT-Rule type column ID
#define	GEX_TPAT_CONFIG_FIELD_NFACTOR				17	// Rule, N-Factor column ID (Head factor)
#define	GEX_TPAT_CONFIG_FIELD_TFACTOR				18	// Rule, T-Factor column ID (Tail factor)
#define GEX_TPAT_CONFIG_FIELD_SPC_PAT_DRIFT			19	// SPC: Define PAT-Median drift value
#define GEX_TPAT_CONFIG_FIELD_SPC_PAT_DRIFT_UNITS	20	// SPC: Define PAT-Median drift units (sigma, test values, % of limit space)
#define GEX_TPAT_CONFIG_FIELD_SPC_TEST_DRIFT		21	// SPC: Define Test Mean drift value
#define GEX_TPAT_CONFIG_FIELD_SPC_TEST_DRIFT_UNITS	22	// SPC: Define Test mean drift units (sigma, test values, % of limit space)
#define GEX_TPAT_CONFIG_FIELD_SPC_TEST_CPK			23	// SPC: Define Cpk level alarm
#define GEX_TPAT_CONFIG_FIELD_LLM_SCALE				24	// Low limit display scale factor (eg: 3 for mV, etc)
#define GEX_TPAT_CONFIG_FIELD_HLM_SCALE				25	// Low limit display scale factor (eg: 3 for mV, etc)
#define	GEX_TPAT_CONFIG_FIELD_SBINRULE				26	// Static PAT bin rule (AEC, or other...)
#define	GEX_TPAT_CONFIG_FIELD_SPAT_NFACTOR			27	// Rule, N-Factor column ID (Head factor)
#define	GEX_TPAT_CONFIG_FIELD_SPAT_TFACTOR			28	// Rule, T-Factor column ID (Tail factor)
#define GEX_TPAT_CONFIG_FIELD_DPAT_COMMENT			29	// Comment line.
#define GEX_TPAT_CONFIG_FIELD_SPAT_COMMENT			30	// Comment line.
#define	GEX_TPAT_CONFIG_FIELD_NNR_RULE				31	// NNR rule type.
#define	GEX_TPAT_CONFIG_FIELD_TTYPE					32	// Test Type column ID
#define	GEX_TPAT_CONFIG_FIELD_PINMAP_IDX			33	// Pinmap Idx column ID
#define GEX_TPAT_CONFIG_TOTAL_FIELDS				34//****Total count of fields!.


#define	GEX_TPAT_SMART_GAUSSIANID			0
#define	GEX_TPAT_SMART_GAUSSIANTAILID		1
#define	GEX_TPAT_SMART_GAUSSIANDUALTAILID	2
#define	GEX_TPAT_SMART_LOGNORMALID			3
#define	GEX_TPAT_SMART_MULTIMODALID			4
#define	GEX_TPAT_SMART_CLAMPEDID			5
#define	GEX_TPAT_SMART_DUALCLAMPEDID		6
#define	GEX_TPAT_SMART_CATEGORYID			7
#define	GEX_TPAT_SMART_UNKNOWNID			8

#define	GEX_TPAT_TAIL_MNGT_DISABLED         0
#define	GEX_TPAT_TAIL_MNGT_CONSERVATIVE     1
#define	GEX_TPAT_TAIL_MNGT_MEDIUM           2
#define	GEX_TPAT_TAIL_MNGT_LIBERAL          3

// Algorithm used to compute SPAT limits
#define	GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA			0
#define	GEX_TPAT_SPAT_ALGO_SIGMA				1
#define GEX_TPAT_SPAT_ALGO_NEWLIMITS			2
#define GEX_TPAT_SPAT_ALGO_RANGE				3
#define GEX_TPAT_SPAT_ALGO_IGNORE				4			// Rule 'disabled' ID (offset in combo)

// Algorithm used to compute DPAT limits
#define	GEX_TPAT_SMART_ALGO_MEAN			0
#define	GEX_TPAT_SMART_ALGO_MEDIAN			1
#define GEX_TPAT_SMART_ALGO_Q1Q3IQR			2
#define	GEX_TPAT_SMART_ALGO_CUSTOMLIB		3

// Bits used to define failure type (static bin, dynamic bin, good die failed because in bad cluster....)
#define	GEX_TPAT_BINTYPE_STATICFAIL		1
#define	GEX_TPAT_BINTYPE_DYNAMICFAIL	2
#define	GEX_TPAT_BINTYPE_NNR			4
#define	GEX_TPAT_BINTYPE_IDDQ_DELTA		8
#define	GEX_TPAT_BINTYPE_BADNEIGHBORS	16
#define	GEX_TPAT_BINTYPE_RETICLE		32
#define	GEX_TPAT_BINTYPE_BADCLUSTER		64
#define	GEX_TPAT_BINTYPE_ZPAT			128
#define	GEX_TPAT_BINTYPE_MVPAT			256

#define	GEX_TPAT_BUILDLIMITS_ALLBINS        0	// Build PAT limits using all samples from ALL bins
#define	GEX_TPAT_BUILDLIMITS_GOODSOFTBINS	1	// Build PAT limits using samples from GOOD soft Bins only
#define	GEX_TPAT_BUILDLIMITS_GOODHARDBINS	2	// Build PAT limits using samples from GOOD hard Bins only
#define	GEX_TPAT_BUILDLIMITS_LISTSOFTBINS	3	// Build PAT limits using all samples from a given list of soft bins
#define	GEX_TPAT_BUILDLIMITS_LISTHARDBINS	4	// Build PAT limits using all samples from a given list of hard bins
#define	GEX_TPAT_BUILDLIMITS_EXCEPTSOFTBINS	5	// Build PAT limits using all samples from all soft bins except a given list...
#define	GEX_TPAT_BUILDLIMITS_EXCEPTHARDBINS	6	// Build PAT limits using all samples from all hard bins except a given list...

#define	GEX_TPAT_FAILMODE_STATIC_L	-1	// Test failure mode: Low static
#define	GEX_TPAT_FAILMODE_STATIC_H	-2	// Test failure mode: Hogh static

// NNR Algorithm
#define	GEX_TPAT_NNR_ALGO_LOCAL_SIGMA		0	// Mean +/-N*Local_Sigma algorithm
#define	GEX_TPAT_NNR_ALGO_LOCAL_MEDIAN		1	// Median +/-N*Local_RobustSigma algorithm
#define	GEX_TPAT_NNR_ALGO_LOCAL_Q1Q3IQR		2	// Q1-N*IQR, Q3+N*IQR

// IDDQ Algorithm
#define	GEX_TPAT_IDDQ_DELTA_SIGMA		0	// Mean+N*Sigma
#define	GEX_TPAT_IDDQ_DELTA_ROBUSTS		1	// Median+N*RobustSigma
#define	GEX_TPAT_IDDQ_Q1Q3IQR			2	// Q1-N*IQR, Q3+N*IQR

// GDBN Algorithm
#define	GEX_TPAT_GDBN_ALGO_SQUEEZE		0	// Squeeze algorithm
#define	GEX_TPAT_GDBN_ALGO_WEIGHT		1	// Weighting algorithm

// GDBN/Clustering Weighting Edge Die handling
#define	GEX_TPAT_GPAT_EDGE_IGNORE		0	// Ignore edge dies
#define	GEX_TPAT_GPAT_EDGE_GOOD			1	// consider edge dies GOOD
#define	GEX_TPAT_GPAT_EDGE_BAD			2	// consider edge dies BAD
#define	GEX_TPAT_GPAT_EDGE_SCALE		3	// Scale total computed weight

// GDBN/Clustering Edge Die type
#define GEX_TPAT_EDGE_DIE_ADJACENT      1   // Adjacent edge die
#define GEX_TPAT_EDGE_DIE_CORNER        2   // Corner edge die
#define GEX_TPAT_EDGE_DIE_BOTH          3   // Both edge die

// STDF Output generation; options are:
#define	GEX_PAT_STDF_OUTPUT_DEFAULT		-1	// STDF output mode is default (whatever bdefined by user GUI)
#define	GEX_PAT_STDF_OUTPUT_DISABLED	0	// STDF output disabled
#define	GEX_PAT_STDF_OUTPUT_WAFERMAP	1	// STDF output: light mode, only wafermap included
#define	GEX_PAT_STDF_OUTPUT_FULL		2	// STDF output: full mode (parametric + wafermap)

// Identifiers to keep track of area whe the bad die Neighbours
// 8 possible zones: Left, left-top,  letf-bottom, Right, right-top,  right-bottom, center-top,center-bottom
#define	GEX_TPAT_NEIGHBOR_LEFT			0
#define	GEX_TPAT_NEIGHBOR_LEFT_TOP		1
#define	GEX_TPAT_NEIGHBOR_LEFT_BOTTOM	2
#define	GEX_TPAT_NEIGHBOR_RIGHT			3
#define	GEX_TPAT_NEIGHBOR_RIGHT_TOP		4
#define	GEX_TPAT_NEIGHBOR_RIGHT_BOTTOM	5
#define	GEX_TPAT_NEIGHBOR_CENTER_TOP	6
#define	GEX_TPAT_NEIGHBOR_CENTER_BOTTOM	7

// List of possible Distribution shaped detected.
#define	PATMAN_LIB_SHAPE_UNSET              -2      // Unset shape
#define PATMAN_LIB_SHAPE_ERROR              -1      // Error
#define	PATMAN_LIB_SHAPE_UNKNOWN			0		// Unknown
#define	PATMAN_LIB_SHAPE_GAUSSIAN			1		// Gaussian
#define	PATMAN_LIB_SHAPE_GAUSSIAN_LEFT		2		// Gaussian: Left tailed
#define	PATMAN_LIB_SHAPE_GAUSSIAN_RIGHT		3		// Gaussian: Right tailed
#define	PATMAN_LIB_SHAPE_LOGNORMAL_LEFT		4		// LogNormal: Left tailed
#define	PATMAN_LIB_SHAPE_LOGNORMAL_RIGHT	5		// LogNormal: Righttailed
#define	PATMAN_LIB_SHAPE_BIMODAL			6		// Bi-Modal with each mode clearly appart from the other
#define	PATMAN_LIB_SHAPE_MULTIMODAL			7		// Multi-Modal (bi-modal partially merged, or more modes)
#define	PATMAN_LIB_SHAPE_CLAMPED_LEFT		8		// Clamped (left clamped)
#define	PATMAN_LIB_SHAPE_CLAMPED_RIGHT		9		// Clamped (right clamped)
#define	PATMAN_LIB_SHAPE_DOUBLECLAMPED		10		// Double Clamped
#define	PATMAN_LIB_SHAPE_CATEGORY			11		// Category

#define	GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR	0	// Tells to exclude all outliers (Near + Medium + far)
#define	GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM	1	// Tells to exclude Medium+far outliers
#define	GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR		2	// Tells to exclude FAR outliers only
#define	GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES	(1+GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR-GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR)	// Total number of classes

// FINAL TEST: Alarm Timout options
#define GEX_TPAT_ALRM_TIME_INFINITE			0			// Timeout: Infinite (production stopped)
#define GEX_TPAT_ALRM_TIME_NONE				1			// Timeout: none (production continues)
#define GEX_TPAT_ALRM_TIME_30S				2			// Timeout: 30 sec
#define GEX_TPAT_ALRM_TIME_1MIN				3			// Timeout: 1 minute
#define GEX_TPAT_ALRM_TIME_2MIN				4			// Timeout: 2 minutes
#define GEX_TPAT_ALRM_TIME_3MIN				5			// Timeout: 3 minutes
#define GEX_TPAT_ALRM_TIME_4MIN				6			// Timeout: 4 minutes
#define GEX_TPAT_ALRM_TIME_5MIN				7			// Timeout: 5 minutes
#define GEX_TPAT_ALRM_TIME_10MIN			8			// Timeout: 10 minutes
#define GEX_TPAT_ALRM_TIME_15MIN			9			// Timeout: 15 minutes
#define GEX_TPAT_ALRM_TIME_30MIN			10			// Timeout: 30 minutes
#define GEX_TPAT_ALRM_TIME_1H				11			// Timeout: 1 hour
#define GEX_TPAT_ALRM_TIME_2H				12			// Timeout: 2 hours
#define GEX_TPAT_ALRM_TIME_3H				13			// Timeout: 3 hours
#define GEX_TPAT_ALRM_TIME_4H				14			// Timeout: 4 hours
#define GEX_TPAT_ALRM_TIME_5H				15			// Timeout: 5 hours
#define GEX_TPAT_ALRM_TIME_6H				16			// Timeout: 6 hours

// FINAL TEST: Traceability mode
#define GEX_TPAT_TRACEABILITY_STDF			0			// Saves PAT Limits computed + messages into STDF file
#define GEX_TPAT_TRACEABILITY_HTML			1			// Saves PAT Limits computed + messages into HTML file
#define GEX_TPAT_TRACEABILITY_ASCII			2			// Saves PAT Limits computed + messages into ASCII file
#define GEX_TPAT_TRACEABILITY_NONE			3			// Doesn't save PAT results into file (only seen on GTM window GUI)

// FINAL TEST: Email Format
#define	GEX_PAT_EMAIL_HTML					0			// Email format: HTML
#define	GEX_PAT_EMAIL_ASCII					1			// Email format: ASCII

// WaferMap Bin type wanted
#define	GEX_WAFMAP_EXPORT_SOFTBIN			0			// Export Soft-Bin wafermap
#define	GEX_WAFMAP_EXPORT_HARDBIN			1			// Export Soft-Bin wafermap
#define	GEX_WAFMAP_EXPORT_CURRENTBIN		2			// Export current wafermap strcture (whatever it is: HardBin or SoftBin)

// WaferMap Source for PAT rule over any EWS flow (modulo 3 value, as WS1...EWS10 flows can be created)
#define	GEX_PAT_WAFMAP_SRC_SOFTBIN			0			// Rule source: STDF Soft-Bin wafermap
#define	GEX_PAT_WAFMAP_SRC_HARDBIN			1			// Rule source: STDF Hard-Bin wafermap
#define	GEX_PAT_WAFMAP_SRC_PROBER			2			// Rule source: Prober/Inspection wafermap

#define GEX_WAFMAP_EXPORT_BIN_QUALITY_PASS	"Pass"		// Indicates a quality that has a commercial value
#define GEX_WAFMAP_EXPORT_BIN_QUALITY_FAIL	"Fail"		// Indicates a quality that does not have a commercial value
#define GEX_WAFMAP_EXPORT_BIN_QUALITY_NULL	"Null"		// No bin

#define GEX_WAFMAP_EXPORT_BIN_MAPTYPE_2D	"2DArray"	// Specifies the map representation type

#define GEX_WAFMAP_EXPORT_ORIGIN_CENTER		"Center"	// Origin location is the center wafermap
#define GEX_WAFMAP_EXPORT_ORIGIN_LOWERLEFT	"LowerLeft"	// Origin location is the lowerleft corner
#define GEX_WAFMAP_EXPORT_ORIGIN_UPPERLEFT	"UpperLeft"	// Origin location is the upperleft corner
#define GEX_WAFMAP_EXPORT_ORIGIN_LOWERRIGHT	"LowerRight"// Origin location is the lowerright corner
#define GEX_WAFMAP_EXPORT_ORIGIN_UPPERRIGHT	"UpperRight"// Origin location is the upperright corner

#define GEX_WAFMAP_EXPORT_AXIS_UPRIGHT		"UpRight"	// Axis direction is Up for Y and Right for X
#define GEX_WAFMAP_EXPORT_AXIS_UPLEFT		"UpLeft"	// Axis direction is Up for Y and Left for X
#define GEX_WAFMAP_EXPORT_AXIS_DOWNRIGHT	"DownRight"	// Axis direction is Down for Y and Right for X
#define GEX_WAFMAP_EXPORT_AXIS_DOWNLEFT		"DownLeft"	// Axis direction is Down for Y and Left for X

// PAT Output Datalog file: Same ortder as in (Process PAT file' GUI
#define	GEX_TPAT_DLG_OUTPUT_NONE		0				// No Output datalog file created after PAT
#define	GEX_TPAT_DLG_OUTPUT_STDF		1				// Output datalog file created after PAT: STDF format
#define	GEX_TPAT_DLG_OUTPUT_ATDF		2				// Output datalog file created after PAT: ATDF Format

// FT PAT algorithms
#define FT_PAT_SINGLE_SITE_ALGO         0               // Basic baseline algorithm (per site)
#define FT_PAT_MERGED_SITE_ALGO         1               // Baseline algorithm using merged site

// FT PAT tuning type
#define FT_PAT_TUNING_EVERY_N_PARTS     0               // Every N parts
#define FT_PAT_TUNING_EVERY_N_OUTLIERS  1               // Every N outliers detected

// FT PAT tuning mode
#define FT_PAT_TUNING_OFF               0               // Tuning deactivated
#define FT_PAT_TUNING_ON                1               // Tuning activated

// FT PAT baseline threshold
#define FT_PAT_TUNING_MIN_SIZE              100             // Minimum size of the tuning
#define FT_PAT_TUNING_MAX_SIZE              500             // Maximum size of the tuning
#define FT_PAT_TUNING_DEFAULT_SIZE          500             // Default size of the tuning
#define FT_PAT_BASELINE_MIN_SIZE            22              // Minimum size of the baseline
#define FT_PAT_BASELINE_MAX_SIZE            500             // Maximum size of the baseline
#define FT_PAT_BASELINE_DEFAULT_SIZE        59              // Default size of the baseline
#define FT_PAT_SUBSEQUENT_BL_MIN_SIZE       50              // Minimum size of the subsequent baseline
#define FT_PAT_SUBSEQUENT_BL_MAX_SIZE       100             // Maximum size of the subsequent baseline
#define FT_PAT_SUBSEQUENT_BL_DEFAULT_SIZE   100             // Default size of the subsequent baseline

// FT PAT minimum samples per site
#define FT_PAT_MINIMUM_SAMPLES_PER_SITE 10              // Minimum samples per sites allowed

// For PAT trigger keys
namespace GS
{
namespace PAT
{
namespace Trigger
{
const char * const sPATYieldLimitSource     = "PATYieldLimitSource";
const char * const sPATOutlierRemovalTask   = "OutlierRemovalTask";
const char * const sMergeBinRuleJSHook      = "MergeBinRuleJSHook";
const char * const sProcessID               = "ProcessID";
const char * const sOnOppositeAxisDir       = "OnOppositeAxisDir";
}
}
}



#endif // PAT_DEFINES_H
