///////////////////////////////////////////////////////////
// Global const structures
///////////////////////////////////////////////////////////
#define GEX_CONSTANTS_CPP

#include "gex_constants.h"

///////////////////////////////////////////////////////////
// GENERAL PURPOSE
///////////////////////////////////////////////////////////
// Mapping table from Filters (GEX_QUERY_FILTER_xx) and position in .CSV index file
int FiltersMapping[] =
{
    0,	// Invalid entry
    18,	// Burn-in is Row#18
    23,	// Data Origin is Row#23
    33,	// DIB name is Row#33
    34,	// DIB type is Row#34
    14,	// FacilityID is Row#14
    13,	// FamilyID is Row#13
    15,	// FloorID is Row#15
    17,	// Frequency/Step is Row#17
    22,	// Loadboard name is Row#22
    21,	// LoadBoard type is Row#21
    2,	// LotID is Row#2
    6,	// Operator is Row#6
    11,	// PackageID is Row#11
    20,	// Prober Name is Row#20
    19,	// Prober Name is Row#19
    16,	// ProcessID is Row#16
    12,	// ProductID is Row#12
    4,	// Program Name is Row#4
    5,	// Program Revision is Row#5
    32,	// Retest count is row#32
    3,	// SubLot is Row#3
    10,	// Temperature is Row#10
    7,	// Tester name is Row#7
    8,	// Tester type is Row#8
    9,	// Test code is Row#9
    0,	// Site#...let filter to '*'
    24,	// WaferID is Rows#24
    25,	// User1 is row#25
    26,	// User2 is row#26
    27,	// User3 is row#27
    28,	// User4 is row#28
    29,	// User5 is row#29
};

// Advanced Report: Combo box settings lists:
const char* HistogramItems[] =
{
    "------ Histogram: Test Results ------",
    "Chart over test limits",				// Combo offset: GEX_ADV_HISTOGRAM_OVERLIMITS
    "Cumulate over test limits",			// Combo offset: GEX_ADV_HISTOGRAM_CUMULLIMITS
    "Chart over test results",				// Combo offset: GEX_ADV_HISTOGRAM_OVERDATA
    "Cumulate over test results",			// Combo offset: GEX_ADV_HISTOGRAM_CUMULDATA
    "Adaptive: data & limits",				// Combo offset: GEX_ADV_HISTOGRAM_DATALIMITS
    0
};

const char* FunctionalReportItems[] =
{
    "--- Build Functional Histogram using ---",
    "Cycle count of vector",				// Combo offset: GEX_ADV_FUNCTIONAL_CYCL_CNT
    "Relative vector address",			// Combo offset: GEX_ADV_FUNCTIONAL_REL_VAD
    0
};

const char* FTRCorrelationReportItems[] = {
    "------ Build FTR correlation table ------",
    "All tests",							// Combo offset: GEX_FTR_ALL
    "Test list...",				// Combo offset: GEX_FTR_LIST
    0
};

const char* TrendItems[] =
{
    "------ Trend: Test Results ------",
    "Chart over test limits",				// Combo offset: GEX_ADV_TREND_OVERLIMITS
    "Chart over test results",				// Combo offset: GEX_ADV_TREND_OVERDATA
    "Adaptive: data & limits",				// Combo offset: GEX_ADV_TREND_DATALIMITS
    "------ Trend: Test1-Test2  ------",
    "Chart difference between 2 tests",		// Combo offset: GEX_ADV_TREND_DIFFERENCE
    "------ Trend: Test Statistics  ------",
    "Chart Mean",					// Combo offset: GEX_ADV_TREND_AGGREGATE_MEAN
    "Chart Sigma",					// Combo offset: GEX_ADV_TREND_AGGREGATE_SIGMA
    "Chart Cp",						// Combo offset: GEX_ADV_TREND_AGGREGATE_CP
    "Chart Cpk",					// Combo offset: GEX_ADV_TREND_AGGREGATE_CPK
    "------ Trend: Binning/Yield ------",
    "Soft Bin (Sub-Lots)",						// Combo offset: GEX_ADV_TREND_SOFTBIN_SBLOTS
    "Soft Bin (all parts)",						// Combo offset: GEX_ADV_TREND_SOFTBIN_PARTS
    "Hard Bin (Sub-Lots)",						// Combo offset: GEX_ADV_TREND_HARDBIN_SBLOTS
    "Hard Bin (all parts)",						// Combo offset: GEX_ADV_TREND_HARDBIN_PARTS
    "Soft Bin + rolling yield",					// Combo offset: GEX_ADV_TREND_SOFTBIN_ROLLING
    "Hard Bin + rolling yield",					// Combo offset: GEX_ADV_TREND_HARDBIN_ROLLING
    0
};

const char* ScatterItems[] =
{
    "------ Scatter: Test Results ------",
    "Chart over test limits",				// Combo offset: GEX_ADV_CORR_OVERLIMITS
    "Chart over test results",				// Combo offset: GEX_ADV_CORR_OVERDATA
    "Adaptive: data & limits",				// Combo offset: GEX_ADV_CORR_DATALIMITS
    0
};

// Advanced Report: Combo box settings lists:
const char* ProbabilityPlotItems[] =
{
    "------ Probability Plot: Test Results ------",
    "Chart over test limits",				// Combo offset: GEX_ADV_PROBPLOT_OVERLIMITS
    "Chart over test results",				// Combo offset: GEX_ADV_PROBPLOT_OVERDATA
    "Adaptive: data & limits",				// Combo offset: GEX_ADV_PROBPLOT_DATALIMITS
    0
};

// Advanced Report: Combo box settings lists:
const char* PearsonItems[] =
{
    "------ Chart style ------",
    "Chart over test limits",				// Combo offset: GEX_ADV_PROBPLOT_OVERLIMITS
    "Chart over test results",				// Combo offset: GEX_ADV_PROBPLOT_OVERDATA
    "Adaptive: data & limits",				// Combo offset: GEX_ADV_PROBPLOT_DATALIMITS
    0
};


// Advanced Report: Combo box settings lists:
const char* BoxPlotExItems[] =
{
    "------ BoxPlot: Test Results ------",
    "Chart over test limits",				// Combo offset: GEX_ADV_BOXPLOT_OVERLIMITS
    "Chart over test results",				// Combo offset: GEX_ADV_BOXPLOT_OVERDATA
    "Adaptive: data & limits",				// Combo offset: GEX_ADV_BOXPLOT_DATALIMITS
    0
};

// Advanced Report: Combo box settings lists:
const char* MultiChartItems[] =
{
    "------ Multi-chart: Test Results ------",
    "Chart over test limits",				// Combo offset: GEX_ADV_MULTICHART_OVERLIMITS
    "Chart over test results",				// Combo offset: GEX_ADV_MULTICHART_OVERDATA
    "Adaptive: data & limits",				// Combo offset: GEX_ADV_MULTICHART_DATALIMITS
    0
};

// Advanced Report: Combo box settings lists:
const char* Charac1ChartItems[] =
{
    "------ Chart type : ------",
    "Chart over test limits",				// Combo offset: GEX_ADV_MULTICHART_OVERLIMITS
    "Chart over test results",				// Combo offset: GEX_ADV_MULTICHART_OVERDATA
    "Adaptive: data & limits",				// Combo offset: GEX_ADV_MULTICHART_DATALIMITS
    0
};


const char* BoxPlotItems[] =
{
    "------ BoxPlot: Test Results ------",
    "All tests",							// Combo offset: GEX_ADV_ALL
    "Test list...",				// Combo offset: GEX_ADV_LIST
    0
};

const char* ProductionYieldItems[] =
{
    "------ Yield reports ------",
    "Yield per Sublot (or Wafer)",		// Combo offset: GEX_ADV_PRODYIELD_SBLOT
    "Yield per Lot",					// Combo offset: GEX_ADV_PRODYIELD_LOT
    "Yield per Group/Query",			// Combo offset: GEX_ADV_PRODYIELD_GROUP
    "Yield per day",					// Combo offset: GEX_ADV_PRODYIELD_DAY
    "Yield per week",					// Combo offset: GEX_ADV_PRODYIELD_WEEK
    "Yield per month",					// Combo offset: GEX_ADV_PRODYIELD_MONTH
    0
};

// Advanced Report: Combo box settings lists:
const char* DalalogItems[] =
{
    "------ Datalog: Test Results ------",
    "All tests",							// Combo offset: GEX_ADV_DATALOG_ALL
    "Failing tests only",					// Combo offset: GEX_ADV_DATALOG_FAIL
    "Removed data points for tests...",				// Combo offset: GEX_ADV_DATALOG_OUTLIER
    "Test list...",							// Combo offset: GEX_ADV_DATALOG_LIST
    "Test list: values only",				// Combo offset: GEX_ADV_DATALOG_RAWDATA
    0
};


// File properties: parts to process
const char* ProcessPartsItems[] =
{
    "All Data / parts (any Bin)",					// Combo offset:GEX_PROCESSPART_ALL
    "All parts except...",							// Combo offset:GEX_PROCESSPART_EXPARTLIST
    "Good Data / parts only (BIN 1)",				// Combo offset:GEX_PROCESSPART_GOOD
    "Failing parts (all but BIN 1)",				// Combo offset:GEX_PROCESSPART_FAIL
    "List of Parts...",								// Combo offset:GEX_PROCESSPART_PARTLIST
    "List of Soft-Bins...",							// Combo offset:GEX_PROCESSPART_SBINLIST
    "All Soft-Bins except...",						// Combo offset:GEX_PROCESSPART_EXSBINLIST
    "List of Hard-Bins...",							// Combo offset:GEX_PROCESSPART_HBINLIST
    "All Hard-Bins except...",						// Combo offset:GEX_PROCESSPART_EXHBINLIST
    "Odd parts (1,3,5...)",							// Combo offset:GEX_PROCESSPART_ODD
    "Even parts (2,4,6...)",						// Combo offset:GEX_PROCESSPART_EVEN
    "First Test instances (ignore retests)",		// Combo offset:GEX_PROCESSPART_FIRSTINSTANCE
    "Last Test instances (only last in retests)",	// Combo offset:GEX_PROCESSPART_LASTINSTANCE
    "Parts inside...",								// Combo offset:GEX_PROCESSPART_PARTSINSIDE
    "Parts outside...",								// Combo offset:GEX_PROCESSPART_PARTSOUTSIDE
    "Bin data only (Ignore samples)",				// Combo offset:GEX_PROCESSPART_NO_SAMPLES
    0
};

// File properties: parts to process...but when calling script 'gexFile'
const char* gexFileProcessPartsItems[] =
{
    "all",					// Combo offset:GEX_PROCESSPART_ALL
    "allparts_except",		// Combo offset:GEX_PROCESSPART_EXPARTLIST
    "good",					// Combo offset:GEX_PROCESSPART_GOOD
    "fails",				// Combo offset:GEX_PROCESSPART_FAIL
    "parts",				// Combo offset:GEX_PROCESSPART_PARTLIST
    "bins",					// Combo offset:GEX_PROCESSPART_SBINLIST
    "allbins_except",		// Combo offset:GEX_PROCESSPART_EXSBINLIST
    "hbins",				// Combo offset:GEX_PROCESSPART_HBINLIST
    "allhbins_except",		// Combo offset:GEX_PROCESSPART_EXHBINLIST
    "odd_parts",			// Combo offset:GEX_PROCESSPART_ODD
    "even_parts",			// Combo offset:GEX_PROCESSPART_EVEN
    "first_instance",		// Combo offset:GEX_PROCESSPART_FIRSTINSTANCE
    "last_instance",		// Combo offset:GEX_PROCESSPART_LASTINSTANCE
    "parts_inside",			// Combo offset:GEX_PROCESSPART_PARTSINSIDE
    "parts_outside",		// Combo offset:GEX_PROCESSPART_PARTSOUTSIDE
    "no_samples",			// Combo offset:GEX_PROCESSPART_NO_SAMPLES
    0
};
const char* gexLabelFileProcessPartsItems[] =
{
    "All",					// Combo offset:GEX_PROCESSPART_ALL
    "All but",				// Combo offset:GEX_PROCESSPART_EXPARTLIST
    "Good",					// Combo offset:GEX_PROCESSPART_GOOD
    "Fails",				// Combo offset:GEX_PROCESSPART_FAIL
    "List",					// Combo offset:GEX_PROCESSPART_PARTLIST
    "Soft Bins",			// Combo offset:GEX_PROCESSPART_SBINLIST
    "All Soft Bins but",	// Combo offset:GEX_PROCESSPART_EXSBINLIST
    "Hard Bins",			// Combo offset:GEX_PROCESSPART_HBINLIST
    "All Hard Bins but",	// Combo offset:GEX_PROCESSPART_EXHBINLIST
    "Odd parts",			// Combo offset:GEX_PROCESSPART_ODD
    "Even parts",			// Combo offset:GEX_PROCESSPART_EVEN
    "First Test instances",	// Combo offset:GEX_PROCESSPART_FIRSTINSTANCE
    "Last Test instances",	// Combo offset:GEX_PROCESSPART_LASTINSTANCE
    "Parts inside",			// Combo offset:GEX_PROCESSPART_PARTSINSIDE
    "Parts outside",		// Combo offset:GEX_PROCESSPART_PARTSOUTSIDE
    0
};

// Query: Time period to consider...used in script 'gexSetQuery'
const char* gexTimePeriodChoices[] =
{
    "today",					// Combo offset:GEX_QUERY_TIMEPERIOD_TODAY
    "last_2days",				// Combo offset:GEX_QUERY_TIMEPERIOD_LAST2DAYS
    "last_3days",				// Combo offset:GEX_QUERY_TIMEPERIOD_LAST3DAYS
    "last_7days",				// Combo offset:GEX_QUERY_TIMEPERIOD_LAST7DAYS
    "last_14days",				// Combo offset:GEX_QUERY_TIMEPERIOD_LAST14DAYS
    "last_31days",				// Combo offset:GEX_QUERY_TIMEPERIOD_LAST31DAYS
    "this_week",				// Combo offset:GEX_QUERY_TIMEPERIOD_THISWEEK
    "this_month",				// Combo offset:GEX_QUERY_TIMEPERIOD_THISMONTH
    "all_dates",				// Combo offset:GEX_QUERY_TIMEPERIOD_ALLDATES
    "calendar",					// Combo offset:GEX_QUERY_TIMEPERIOD_CALENDAR
    "last_n_x",					// Combo offset:GEX_QUERY_TIMEPERIOD_LAST_N_X
    0
};

const char* gexLabelTimePeriodChoices[] =
{
    "Today",											// Combo offset:GEX_QUERY_TIMEPERIOD_TODAY
    "Last 2 days (today and yesterday)",				// Combo offset:GEX_QUERY_TIMEPERIOD_LAST2DAYS
    "Last 3 days (today and last 2 days)",				// Combo offset:GEX_QUERY_TIMEPERIOD_LAST3DAYS
    "Last 7 days (1 week)",								// Combo offset:GEX_QUERY_TIMEPERIOD_LAST7DAYS
    "Last 14 days (2 weeks)",							// Combo offset:GEX_QUERY_TIMEPERIOD_LAST14DAYS
    "Last 31 days (1 full month)",						// Combo offset:GEX_QUERY_TIMEPERIOD_LAST31DAYS
    "This week",										// Combo offset:GEX_QUERY_TIMEPERIOD_THISWEEK
    "This month",										// Combo offset:GEX_QUERY_TIMEPERIOD_THISMONTH
    "All dates (no restriction on time/date)",			// Combo offset:GEX_QUERY_TIMEPERIOD_ALLDATES
    "Calendar (Pick dates from calendar)",				// Combo offset:GEX_QUERY_TIMEPERIOD_CALENDAR
    "Last...",											// Combo offset:GEX_QUERY_TIMEPERIOD_LAST_N_X
    0
};

const char* gexLabelHousekeepingPeriodChoices[] =
{
    "Today (Data of the day)",		// Combo offset:GEX_QUERY_HOUSEKPERIOD_TODAY
    "Data at least 1 day old",		// Combo offset:GEX_QUERY_HOUSEKPERIOD_1DAY
    "Data at least 2 days old",		// Combo offset:GEX_QUERY_HOUSEKPERIOD_2DAYS
    "Data at least 3 days old",		// Combo offset:GEX_QUERY_HOUSEKPERIOD_3DAYS
    "Data at least 4 days old",		// Combo offset:GEX_QUERY_HOUSEKPERIOD_4DAYS
    "Data at least 1 week old",		// Combo offset:GEX_QUERY_HOUSEKPERIOD_1WEEK
    "Data at least 2 weeks old",	// Combo offset:GEX_QUERY_HOUSEKPERIOD_2WEEKS
    "Data at least 3 weeks old",	// Combo offset:GEX_QUERY_HOUSEKPERIOD_3WEEKS
    "Data at least 1 month old",	// Combo offset:GEX_QUERY_HOUSEKPERIOD_1MONTH
    "Calendar",						// Combo offset:GEX_QUERY_HOUSEKPERIOD_CALENDAR
    0
};

// Query filter: possible filters...used in script 'gexSetQuery'
const char* gexFilterChoices[] =
{
    "--Select Filter--",			// MIR: Combo offset:GEX_QUERY_FILTER_NONE
    "dbf_burnin_time",				// MIR: Combo offset:GEX_QUERY_FILTER_BURNIN
    "dbf_data_origin",				// MIR: Combo offset:GEX_QUERY_FILTER_ORIGIN
    "dbf_dib_id",					// SDR: Combo offset:GEX_QUERY_FILTER_DIBNAME
    "dbf_dib_type",					// SDR: Combo offset:GEX_QUERY_FILTER_DIBTYPE
    "dbf_facility_id",				// MIR: Combo offset:GEX_QUERY_FILTER_FACILITY
    "dbf_family_id",				// MIR: Combo offset:GEX_QUERY_FILTER_FAMILY
    "dbf_floor_id",					// MIR: Combo offset:GEX_QUERY_FILTER_FLOOR
    "dbf_freq_id",					// MIR: Combo offset:GEX_QUERY_FILTER_FREQUENCYSTEP
    "dbf_loadboard_id",				// SDR: Combo offset:GEX_QUERY_FILTER_LOADBOARDNAME
    "dbf_loadboard_type",			// SDR: Combo offset:GEX_QUERY_FILTER_LOADBOARDTYPE
    "dbf_lot_id",					// MIR: Combo offset:GEX_QUERY_FILTER_LOT
    "dbf_operator_name",			// MIR: Combo offset:GEX_QUERY_FILTER_OPERATOR
    "dbf_package_type",				// MIR: Combo offset:GEX_QUERY_FILTER_PACKAGE
    "dbf_prober_id",				// SDR: Combo offset:GEX_QUERY_FILTER_PROBERNAME
    "dbf_prober_type",				// SDR: Combo offset:GEX_QUERY_FILTER_PROBERTYPE
    "dbf_process_id",				// MIR: Combo offset:GEX_QUERY_FILTER_PROCESS
    "dbf_product_id",				// MIR: Combo offset:GEX_QUERY_FILTER_PRODUCT
    "dbf_program_name",				// MIR: Combo offset:GEX_QUERY_FILTER_PROGRAMNAME
    "dbf_program_rev",				// MIR: Combo offset:GEX_QUERY_FILTER_PROGRAMREVISION
    "dbf_retest_nbr",				// MIR: Combo offset:GEX_QUERY_FILTER_RETESTNBR
    "dbf_sublot_id",				// MIR: Combo offset:GEX_QUERY_FILTER_SUBLOT
    "dbf_temperature",				// MIR: Combo offset:GEX_QUERY_FILTER_TEMPERATURE
    "dbf_tester_name",				// MIR: Combo offset:GEX_QUERY_FILTER_TESTERNAME
    "dbf_tester_type",				// MIR: Combo offset:GEX_QUERY_FILTER_TESTERTYPE
    "dbf_testing_code",				// MIR: Combo offset:GEX_QUERY_FILTER_TESTCODE
    "dbf_site_nbr",					// MIR: Combo offset:GEX_QUERY_FILTER_SITENBR
    "dbf_wafer_id",					// MIR: Combo offset:GEX_QUERY_FILTER_WAFERID
    "dbf_user_1",					// MIR: Combo offset:GEX_QUERY_FILTER_USER1
    "dbf_user_2",					// MIR: Combo offset:GEX_QUERY_FILTER_USER2
    "dbf_user_3",					// MIR: Combo offset:GEX_QUERY_FILTER_USER3
    "dbf_user_4",					// MIR: Combo offset:GEX_QUERY_FILTER_USER4
    "dbf_user_5",					// MIR: Combo offset:GEX_QUERY_FILTER_USER5
    "dbf_total_parts",              // combo offset:GEX_QUERY_FILTER_TOTALPARTS
    0
};

const char* gexLabelFilterChoices[] =
{
    "--Select Filter--",		// MIR: Combo offset:GEX_QUERY_FILTER_NONE
    "Burn-in time",				// MIR: Combo offset:GEX_QUERY_FILTER_BURNIN
    "Data Origin",				// MIR: Combo offset:GEX_QUERY_FILTER_ORIGIN
    "DIB ID",					// SDR: Combo offset:GEX_QUERY_FILTER_DIBNAME
    "DIB type",					// SDR: Combo offset:GEX_QUERY_FILTER_DIBTYPE
    "Facility ID",				// MIR: Combo offset:GEX_QUERY_FILTER_FACILITY
    "Family ID",				// MIR: Combo offset:GEX_QUERY_FILTER_FAMILY
    "Floor ID",					// MIR: Combo offset:GEX_QUERY_FILTER_FLOOR
    "Freq/Step ID",				// MIR: Combo offset:GEX_QUERY_FILTER_FREQUENCYSTEP
    "Load board ID",			// SDR: Combo offset:GEX_QUERY_FILTER_LOADBOARDNAME
    "Load board type",			// SDR: Combo offset:GEX_QUERY_FILTER_LOADBOARDTYPE
    "Lot ID",					// MIR: Combo offset:GEX_QUERY_FILTER_LOT
    "Operator name",			// MIR: Combo offset:GEX_QUERY_FILTER_OPERATOR
    "Package type",				// MIR: Combo offset:GEX_QUERY_FILTER_PACKAGE
    "Prober ID",				// SDR: Combo offset:GEX_QUERY_FILTER_PROBERNAME
    "Prober type",				// SDR: Combo offset:GEX_QUERY_FILTER_PROBERTYPE
    "Process ID",				// MIR: Combo offset:GEX_QUERY_FILTER_PROCESS
    "Product ID",				// MIR: Combo offset:GEX_QUERY_FILTER_PRODUCT
    "Program name",				// MIR: Combo offset:GEX_QUERY_FILTER_PROGRAMNAME
    "Program rev.",				// MIR: Combo offset:GEX_QUERY_FILTER_PROGRAMREVISION
    "Retest instance",			// MIR: Combo offset:GEX_QUERY_FILTER_RETESTNBR
    "Sublot ID",				// MIR: Combo offset:GEX_QUERY_FILTER_SUBLOT
    "Temperature",				// MIR: Combo offset:GEX_QUERY_FILTER_TEMPERATURE
    "Tester name",				// MIR: Combo offset:GEX_QUERY_FILTER_TESTERNAME
    "Tester type",				// MIR: Combo offset:GEX_QUERY_FILTER_TESTERTYPE
    "Testing code",				// MIR: Combo offset:GEX_QUERY_FILTER_TESTCODE
    "Site# (hardware)",			// MIR: Combo offset:GEX_QUERY_FILTER_SITENBR
    "Wafer ID",					// WIR: Combo offset:GEX_QUERY_FILTER_WAFERID
    "User 1",					// WIR: Combo offset:GEX_QUERY_FILTER_USER1
    "User 2",					// WIR: Combo offset:GEX_QUERY_FILTER_USER2
    "User 3",					// WIR: Combo offset:GEX_QUERY_FILTER_USER3
    "User 4",					// WIR: Combo offset:GEX_QUERY_FILTER_USER4
    "User 5",					// WIR: Combo offset:GEX_QUERY_FILTER_USER5
    "Total Parts",              // combo offset:GEX_QUERY_FILTER_TOTALPARTS
    0
};

