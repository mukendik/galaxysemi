///////////////////////////////////////////////////////////
// PAT
///////////////////////////////////////////////////////////
#ifndef GEX_PAT_CONSTANTS_H
#define GEX_PAT_CONSTANTS_H

// List of rules available
const char* gexRuleSetItemsLongNamesGUI[] = 
{ 
	"N*Sigma (AEC-QOO1. JEDEC JESD62)",			// GEX_TPAT_RULETYPE_SIGMAID: Combo offset:0
	"N*RobustSigma",			// GEX_TPAT_RULETYPE_ROBUSTSIGMAID: Combo offset:1
	"Q1/Q3 IQR",				// Q1-N*IQR, Q3+N*IQR GEX_TPAT_RULETYPE_Q1Q3IQR ID: Combo offset:2
	"% of Limits",				// GEX_TPAT_RULETYPE_LIMITSID: Combo offset:3
	"New Limits",				// GEX_TPAT_RULETYPE_NEWLIMITSID: Combo offset:4
	"Range",					// GEX_TPAT_RULETYPE_RANGEID: Combo offset:5
	"Smart & Adaptive",			// GEX_TPAT_RULETYPE_SMARTID: Combo offset:6
	"Disabled",					// GEX_TPAT_RULETYPE_IGNORE: Combo offset:7
	0
};

const char* gexDefaultRuleSet[]=
{
    "n_sigma",
    "n_robust_sigma",
    "q1_q3_idq",
    "percent_of_limits",
    "new_limits",
    "range",
    "smart_adaptive",
    "disabled",
    0
};

// List of default factor associated with above 'gexRuleSetItemsLongNamesGUI' rules
const char* gexRuleFactorItems[] = 
{ 
	"6.0",	// +/- 6xSigma space
	"6.0",	// +/- 6xRobustSigma space
	"1.5",	// Q1-N*IQR, Q3+N*IQR
	"10",	// 10% of limit space
	"0.0",	// range: no factor defined.
	"6.0",	// Smart mode: no global factor defined, is forced on a per test basis.
	"0.0",	// Disabled mode: no factor defined.
	0
};

// List of default factor units with above 'gexRuleSetItemsLongNamesGUI' rules
const char* gexRuleFactorUnits[] = 
{ 
	"Sigma",		// +/- NxSigma space
	"RobustSigma",	// +/- NxSigma space
	"IQR",			// Q1-N*IQR, Q3+N*IQR
	"%",		// N% of limit space
	"",			// New limits
	"",			// Range: no factor defined.
	"",			// Smart mode: no factor defined.
	"",			// Disabled mode: no factor defined.
	0
};

// List of default factor Prefix associated with above 'gexRuleSetItemsLongNamesGUI' rules
const char* gexRuleFactorPrefixItems[] = 
{ 
	"+/-",	// +/- 6xSigma space
	"+/-",	// +/- 6xRobustSigma space
	"+/-",	// +/- of limit space
	"",		// Range: no factor defined.
	"",		// Smart mode: no factor defined.
	"",		// Disabled mode: no factor defined.
	0
};

// GUI names 'Rule type'...short version (used under Rule editor mostly)
// WARNING: Do NOT edit existing items in this list otherwise recipe compatibility will be affected.
const char* gexRuleSetItemsGUI[] = 
{ 
	"Mean +/-N*Sigma",			// GEX_TPAT_RULETYPE_SIGMAID: Combo offset:0
	"Median +/-N*RobustSigma",	// GEX_TPAT_RULETYPE_ROBUSTSIGMAID: Combo offset:1
	"Q1/Q3 IQR",				// Q1-N*IQR, Q3+N*IQR GEX_TPAT_RULETYPE_Q1Q3IQR ID: Combo offset:2
	"% of Limits",				// GEX_TPAT_RULETYPE_LIMITSID:
	"New Limits (LL=N. HL=T)",	// GEX_TPAT_RULETYPE_NEWLIMITSID: 
	"Range (Mean +/-N)",		// GEX_TPAT_RULETYPE_RANGEID: 
	"Smart & Adaptive",			// GEX_TPAT_RULETYPE_SMARTID: 
	"Disabled",					// GEX_TPAT_RULETYPE_IGNORE: 
	"-- Shapes --",			// separator.
	"Gaussian",				// GEX_TPAT_RULETYPE_GAUSSIANID
	"Gaussian (1 tail)",	// GEX_TPAT_RULETYPE_GAUSSIANTAILID
	"Gaussian (2 tails)",	// GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID
	"Log Normal",			// GEX_TPAT_RULETYPE_LOGNORMALID
	"Bi-Modal",				// GEX_TPAT_RULETYPE_BIMODALID
	"Multi-Modal",			// GEX_TPAT_RULETYPE_MULTIMODALID
	"Clamped (1 side)",		// GEX_TPAT_RULETYPE_CLAMPEDID
	"Clamped (2 sides)",	// GEX_TPAT_RULETYPE_DUALCLAMPEDID
	"Categories",			// GEX_TPAT_RULETYPE_CATEGORYID
	"Custom PAT Lib",		// GEX_TPAT_RULETYPE_CUSTOMLIBID
	0
};


//
const char* gexRuleSetItemsKeywordsForJson[] =
{
    "mean_n_sigma",         // GEX_TPAT_RULETYPE_SIGMAID
    "median_n_robust",      // GEX_TPAT_RULETYPE_ROBUSTSIGMAID
    "q1q3iqr",				// GEX_TPAT_RULETYPE_Q1Q3IQR
    "%limit",				// GEX_TPAT_RULETYPE_LIMITSID
    "newlimit",				// GEX_TPAT_RULETYPE_NEWLIMITSID
    "range",				// GEX_TPAT_RULETYPE_RANGEID
    "smart",				// GEX_TPAT_RULETYPE_SMARTID
    "disabled",				// GEX_TPAT_RULETYPE_IGNORE
    "separate",				// GEX_TPAT_RULETYPE_SEPARATORID
    "gaussian",				// GEX_TPAT_RULETYPE_GAUSSIANID
    "gaussian1tail",		// GEX_TPAT_RULETYPE_GAUSSIANTAILID
    "gaussian2tails",		// GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID
    "lognormal",			// GEX_TPAT_RULETYPE_LOGNORMALID
    "bimodal",				// GEX_TPAT_RULETYPE_BIMODALID
    "multimodal",			// GEX_TPAT_RULETYPE_MULTIMODALID
    "clamped1side",			// GEX_TPAT_RULETYPE_CLAMPEDID
    "clamped2sides",		// GEX_TPAT_RULETYPE_DUALCLAMPEDID
    "category",				// GEX_TPAT_RULETYPE_CATEGORYID
    "pat_lib",				// GEX_TPAT_RULETYPE_CUSTOMLIBID
    0
};

// 
const char* gexRuleSetItemsKeywords[] = 
{ 
	"sigma",				// GEX_TPAT_RULETYPE_SIGMAID: Combo offset:0
	"iqr",					// GEX_TPAT_RULETYPE_ROBUSTSIGMAID: Combo offset:1
	"q1q3iqr",				// Q1-N*IQR, Q3+N*IQR GEX_TPAT_RULETYPE_Q1Q3IQR ID: Combo offset:2
	"%limit",				// GEX_TPAT_RULETYPE_LIMITSID: 
	"newlimit",				// GEX_TPAT_RULETYPE_NEWLIMITSID: 
	"range",				// GEX_TPAT_RULETYPE_RANGEID: 
	"smart",				// GEX_TPAT_RULETYPE_SMARTID: 
	"disabled",				// GEX_TPAT_RULETYPE_IGNORE: 
	"disabled",				// separator:
	"gaussian",				// Rule: Gaussian distribution
	"gaussian1tail",		// Rule: Gaussian+Tail distribution
	"gaussian2tails",		// Rule: Gaussian+Two Tails distribution
	"lognormal",			// Rule: LogNormal distribution
	"bimodal",				// Rule: Bi-Modal distribution
	"multimodal",			// Rule: Multi-Modal distribution
	"clamped1side",			// Rule: Clamped distribution
	"clamped2sides",		// Rule: DoubleClamped distribution
	"category",				// Rule: Categories distribution
	"pat_lib",				// Rule: Custom PAT lib (if enabled).
	0
};

// GUI names 'SPAT Rule type'...short version (used under Rule editor mostly)
const char* gexSpatRuleSetItemsGUI[] = 
{ 
	"AEC (Median +/-6*RobustSigma)",	// GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA: Combo offset:0
	"Relax (+/- N*Sigma)",				// GEX_TPAT_SPAT_ALGO_SIGMA: Combo offset:1
	"New Limits (LL=N. HL=T)",			// GEX_TPAT_SPAT_ALGO_NEWLIMITS: Combo offset:2
	"Range (Mean +/- N)",				// GEX_TPAT_SPAT_ALGO_RANGE: Combo offset:2
	"Disabled",							// GEX_TPAT_SPAT_ALGO_IGNORE: 
	0
};

// 
const char* gexSpatRuleSetItemsKeywords[] = 
{ 
	"aec",				// GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA: Combo offset:0
	"sigma",			// GEX_TPAT_SPAT_ALGO_SIGMA: Combo offset:1
	"newlimit",			// GEX_TPAT_SPAT_ALGO_NEWLIMITS: Combo offset:2
	"range",			// GEX_TPAT_SPAT_ALGO_RANGE: Combo offset:2
	"disabled",			// GEX_TPAT_SPAT_ALGO_IGNORE: disabled
	0
};

////////////////////////////////////////
// Smart Rules
////////////////////////////////////////
const char* gexSmartRuleSetItemsLongNamesGUI[] = 
{ 
	"Gaussian",				// GEX_TPAT_SMART_GAUSSIANID
	"Gaussian (1 tail)",	// GEX_TPAT_SMART_GAUSSIANTAILID
	"Gaussian (2 tails)",	// GEX_TPAT_SMART_GAUSSIANDUALTAILID
	"Log Normal",			// GEX_TPAT_SMART_LOGNORMALID
	"Multi-Modal",			// GEX_TPAT_SMART_MULTIMODALID
	"Clamped (1 side)",		// GEX_TPAT_SMART_CLAMPEDID
	"Clamped (2 sides)",	// GEX_TPAT_SMART_DUALCLAMPEDID
	"Categories",			// GEX_TPAT_SMART_CATEGORYID
	"Unknown",				// GEX_TPAT_SMART_UNKNOWNID
	0
};

const char* gexSmartAlgorithmLongNamesGUI[] = 
{ 
	"Mean +/- N*Sigma",							// GEX_TPAT_SMART_ALGO_MEAN
	"Median +/- N*Robust Sigma",				// GEX_TPAT_SMART_ALGO_MEDIAN
	"IQR: (LL: Q1-N*IQR. HL: Q3+N*IQR)",		// GEX_TPAT_SMART_ALGO_Q1Q3IQR
	"Custom PAT Lib",							// GEX_TPAT_SMART_ALGO_CUSTOMLIB
	0
};

// GUI 'Outliers to Keep' names...short version (used under Rule editor mostly)
const char* gexKeepOutliersSetItemsGUI[] = 
{ 
	"None",					// GEX_TPAT_KEEPTYPE_NONEID: Combo offset:0
	"Low values",			// GEX_TPAT_KEEPTYPE_LOWID: Combo offset:1
	"High values",			// GEX_TPAT_KEEPTYPE_HIGHID: Combo offset:2
	0
};

// 
const char* gexKeepOutliersSetItemsKeywords[] = 
{ 
	"none",					// GEX_TPAT_KEEPTYPE_NONEID: Combo offset:0
	"low",					// GEX_TPAT_KEEPTYPE_LOWID: Combo offset:1
	"high",					// GEX_TPAT_KEEPTYPE_HIGHID: Combo offset:2
	0
};

// GUI 'Samples to Ignore' names...short version (used under Rule editor mostly)
const char* gexIgnoreSamplesSetItemsGUI[] = 
{ 
	"None",					// GEX_TPAT_IGNOREDATA_NONEID: Combo offset:0
	"Negative values",		// GEX_TPAT_IGNOREDATA_NEGATIVEID: Combo offset:1
	"Positive values",		// GEX_TPAT_IGNOREDATA_POSITIVEID: Combo offset:2
	0
};

// 
const char* gexIgnoreSamplesSetItemsKeywords[] = 
{ 
	"none",					// GEX_TPAT_IGNOREDATA_NONEID: Combo offset:0
	"negative",				// GEX_TPAT_IGNOREDATA_NEGATIVEID: Combo offset:1
	"positive",				// GEX_TPAT_IGNOREDATA_POSITIVEID: Combo offset:2
	0
};

// GUI 'Outlier Limits set' names...long version (used under Rule editor mostly)
const char* gexOutlierLimitsSetItemsLongNamesGUI[] = 
{ 
	"Near outliers",		// GEX_TPAT_LIMITSSET_NEAR: Combo offset:0
	"Medium outliers",		// GEX_TPAT_LIMITSSET_MEDIUM: Combo offset:1
	"Far outliers",			// GEX_TPAT_LIMITSSET_FAR: Combo offset:2
	0
};

// GUI 'Outlier Limits set' names...short version (used under Rule editor mostly)
const char* gexOutlierLimitsSetItemsGUI[] = 
{ 
	"Near",			// GEX_TPAT_LIMITSSET_NEAR: Combo offset:0
	"Medium",		// GEX_TPAT_LIMITSSET_MEDIUM: Combo offset:1
	"Far",			// GEX_TPAT_LIMITSSET_FAR: Combo offset:2
	0
};

// 
const char* gexOutlierLimitsSetItemsKeywords[] = 
{ 
	"near",					// GEX_TPAT_LIMITSSET_NEAR: Combo offset:0
	"medium",				// GEX_TPAT_LIMITSSET_MEDIUM: Combo offset:1
	"far",					// GEX_TPAT_LIMITSSET_FAR: Combo offset:2
	0
};

// GUI 'Median drift units' names...(used under Rule editor)
const char* gexMedianDriftUnitsSetItemsGUI[] = 
{ 
	"Test units",	// GEX_TPAT_DRIFT_UNITS_TEST: Combo offset:0
	"Sigma",		// GEX_TPAT_DRIFT_UNITS_SIGMA: Combo offset:1
	"% of limits",	// GEX_TPAT_DRIFT_UNITS_P_LIMITS: Combo offset:2
	"None",			// GEX_TPAT_DRIFT_UNITS_NONE: Combo offset:3
	0
};

// 
const char* gexMedianDriftUnitsSetItemsKeywords[] = 
{ 
	"test",			// GEX_TPAT_DRIFT_UNITS_TEST: Combo offset:0
	"sigma",		// GEX_TPAT_DRIFT_UNITS_SIGMA: Combo offset:1
	"percent",		// GEX_TPAT_DRIFT_UNITS_P_LIMITS: Combo offset:2
	"none",			// GEX_TPAT_DRIFT_UNITS_NONE: Combo offset:3
	0
};

// GUI 'NNR rule' names...short version (used under Rule editor mostly)
const char* gexNnrRuleSetItemsGUI[] = 
{ 
	"Enabled",		// GEX_TPAT_NNR_ENABLED: Combo offset:0
	"Disabled",		// GEX_TPAT_NNR_DISABLED: Combo offset:1
	0
};

// 
const char* gexNnrRuleSetItemsKeywords[] = 
{ 
	"enabled",		// GEX_TPAT_NNR_ENABLED: Combo offset:0
	"disabled",		// GEX_TPAT_NNR_DISABLED: Combo offset:1
	0
};

// 7103 : Final test: List of alarm severity
const char* gexFT_Alarm_Severities[] =
{
    "Critical",		// Critical : sev < 0
    "Warning",      // Warning: sev = 0
    "Notice",		// Just a notice : sev > 0
    "Ignore",        // Ignore : message wont be send to the GTL.
    0
};

// Final test: List of alarm timeout options available
const char* gexFT_Alarm_Timeout[] = 
{ 
    "Stop production",					// Stop production on alarm (user can enter a key or CTRL+C at tester to exit)
    "Continue production",				// Ignore alarm
	"Suspend (30 seconds timeout)",		// Suspend tester for X seconds
	"Suspend (1 minute timeout)",		// Suspend tester for X seconds
	"Suspend (2 minutes timeout)",		// Suspend tester for X seconds
	"Suspend (3 minutes timeout)",		// Suspend tester for X seconds
	"Suspend (4 minutes timeout)",		// Suspend tester for X seconds
	"Suspend (5 minutes timeout)",		// Suspend tester for X seconds
	"Suspend (10 minutes timeout)",		// Suspend tester for X seconds
	"Suspend (15 minutes timeout)",		// Suspend tester for X seconds
	"Suspend (30 minutes timeout)",		// Suspend tester for X seconds
	"Suspend (1 hour timeout)",			// Suspend tester for X seconds
	"Suspend (2 hours timeout)",		// Suspend tester for X seconds
	"Suspend (3 hours timeout)",		// Suspend tester for X seconds
	"Suspend (4 hours timeout)",		// Suspend tester for X seconds
	"Suspend (5 hours timeout)",		// Suspend tester for X seconds
	"Suspend (6 hours timeout)",		// Suspend tester for X seconds
	0
};

// Tail management options (only used when rule is SMART & ADAPTIVE)
const char* gexTailMngtRuleSetItemsGUI[] =
{
    "Disabled",     // The tail management is disabled
    "Conservative", // The tail management is conservative (hole of 1.5*IQR)
    "Medium",       // The tail management is medium (hole of 2*IQR)
    "Liberal",      // [Default value] The tail management is Liberal (hole of 3*IQR)
    0
};

const char* gexTailMngtRuleSetItemsKeywords[] =
{
    "disabled",     // The tail management is disabled
    "conservative", // The tail management is conservative (hole of 1.5*IQR)
    "medium",       // The tail management is medium (hole of 2*IQR)
    "liberal",      // [Default value] The tail management is Liberal (hole of 3*IQR)
    0
};

#endif
