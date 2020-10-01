#ifndef TABLERENDERERKEYS_H
#define TABLERENDERERKEYS_H

#include <QMap>
#include <QString>

const QString OPEN_TABLE_ROW        = "<tr>\n";
const QString CLOSE_TABLE_ROW       = "</tr>\n";
const QString BREAK_PAGE            = "<p> <br clear=\"all\" style=\"page-break-before:always\" /></p>";
const QString CLOSE_TABLE           = "</table>";



const QString TEST_TNUMBER      =	"test_number"   ; // Test number
const QString TEST_TNAME        =	"test_name"     ;  // Test name
const QString TEST_GROUP        =	"group_name"    ;   // Group name
const QString TEST_TYPE         =	"test_type"     ;// Test type
const QString TEST_LTL          =	"l_limit"       ;// Low limit
const QString TEST_HTL          =	"h_limit"       ;// High limit
const QString TEST_LSL          =	"l_spec_limit"  ;// Low spec limit
const QString TEST_HSL          =	"h_spec_limit"  ;// High spec limit
const QString TEST_DRIFTL       =   "drift_limit"	;// (Test/Specs Limits)*100 = Limits drift
const QString TEST_DRIFTLOW     =   "l_drift_limit"	;// (Test/Specs Low Limits)*100 = LOW Limits drift
const QString TEST_DRIFTHIGH    =	"h_drift_limit" ;	// (Test/Specs High Limits)*100 = HIGH Limits drift
const QString TEST_SHAPE        =	"shape"         ;  // Distribution shape
const QString TEST_STATS_SRC    =	"stat_src"      ;   // Statistics source (Samples, summary,...)
const QString TEST_EXEC         =	"exec_count"	;// Exec count
const QString TEST_FAIL         =	"fail_count"	;// Fail count
const QString TEST_FAILPERCENT  =	"fail_pct"      ;   // Fail percentage
const QString TEST_FAILBIN		=   "fail_bin"      ;   // Fail Bin
const QString TEST_TESTFLOWID   =	"flow_id"       ;   // Test order in flow (Test flow ID)
const QString TEST_OUTLIER		=   "removed_count" ;    // Outliers
const QString TEST_MEAN         =   "mean"          ;// Mean
const QString TEST_MEANSHIFT    =	"mean_shift"    ;	// Mean shift
const QString TEST_T_TEST       =	"t_test"        ;   // Studen't T-Test
const QString TEST_SIGMA        =	"sigma"         ;   // Sigma
const QString TEST_SIGMASHIFT   =	"sigma_shift"   ;	// Sigma shift
const QString TEST_2_SIGMA		=   "2_sigma"   ;     // 2*Sigma
const QString TEST_3_SIGMA      =	"3_sigma"   ;    // 3*Sigma
const QString TEST_6_SIGMA      =	"6_sigma"   ;    // 6*Sigma
const QString TEST_MIN          =	"min"       ;    // Min.
const QString TEST_MAX          =	"max"       ;    // Max.
const QString TEST_RANGE        =	"range"     ;    // Range
const QString TEST_MAX_RANGE    =	"max_range" ;    // Range
const QString TEST_CP           =	"cp"         ;   // Cp
const QString TEST_CR           =   "cr"        ;    // Cr (GCORE-199)
const QString TEST_CPSHIFT		=   "cp_shift";      // Cp shift
const QString TEST_CRSHIFT      =   "cr_shift";
const QString TEST_CPK          =	"cpk"      ;    // Cpk
const QString TEST_CPKL         =	"cpkl"         ; // Cpk low
const QString TEST_CPKH         =	"cpkh"         ; // Cpk high
const QString TEST_CPKSHIFT     =   "cpk_shift";  // Cpk shift
const QString TEST_YIELD        =	"yield"    ;     // Yield
const QString TEST_GAGE_EV      =	"gage_ev"  ;     // Gage: Repeatability (EV)
const QString TEST_GAGE_AV      =	"gage_av"  ;     // Gage: Reproducibility (AV)
const QString TEST_GAGE_RR      =	"gage_rr"  ;     // Gage: R&R
const QString TEST_GAGE_GB      =	"gage_gb"  ;     // Gage: GuardBanding
const QString TEST_GAGE_PV      =	"gage_pv"  ;     // Gage: Part Variation (PV)
const QString TEST_GAGE_TV      =	"gage_tv"  ;     // Gage: Total Variation (TV)
const QString TEST_GAGE_P_T     =   "gage_p_t" ;    // Gage: Total Variation (TV)
const QString TEST_SKEW         =   "skew"     ;    // Skew
const QString TEST_KURTOSIS     =   "kurtosis" ;    // Kurtosis
const QString TEST_P0_5         =   "p0_5"     ;    // P0.5% quartile
const QString TEST_P2_5         =	"p2_5"     ;     // P2.5% quartile
const QString TEST_P10          =   "p10"      ;     // P10% quartile
const QString TEST_Q1           =	"q1"       ;     // Q1 quartile
const QString TEST_Q2           =	"q2"       ;     // Q2 quartile
const QString TEST_Q3           =	"q3"       ;     // Q3 quartile
const QString TEST_P90          =   "p90"      ;     // P90% quartile
const QString TEST_P97_5        =	"p97_5"    ;     // P97.5% quartile
const QString TEST_P99_5        =	"p99_5"    ;     // P99.5% quartile
const QString TEST_IQR          =	"iqr"      ;     // IQR
const QString TEST_SIGMAIQR     =   "sigma_iqr";     // IQR Sigma

const QString FIELD_COLOR       = "\"#CCECFF\"";
const QString ALARM_COLOR       = "\"#FF0000\"";
const QString WARNING_COLOR     = "\"#ffff80\"";
const QString DATA_COLOR        = "\"#F8F8F8\"";

static QMap<QString, QString> initColumnsLabel () {
    QMap<QString, QString> lMap;
    lMap.insert(TEST_TNUMBER,      "Test");
    lMap.insert(TEST_TNAME,        "Name");
    lMap.insert(TEST_GROUP,        "Group/Dataset");
    lMap.insert(TEST_TYPE,         "Type");
    lMap.insert(TEST_LTL,          "Low Limit");
    lMap.insert(TEST_HTL,          "High Limit");
    lMap.insert(TEST_LSL,          "Low Spec.");
    lMap.insert(TEST_HSL,          "High Spec.");
    lMap.insert(TEST_DRIFTL,       "Test Vs Spec %");
    lMap.insert(TEST_DRIFTLOW,     "LL Test/Spec %");
    lMap.insert(TEST_DRIFTHIGH,    "HL Test/Spec %");
    lMap.insert(TEST_SHAPE,        "Shape");
    lMap.insert(TEST_STATS_SRC,    "Source");
    lMap.insert(TEST_EXEC,         "Exec.");
    lMap.insert(TEST_FAIL,         "Fail");
    lMap.insert(TEST_FAILPERCENT,  "Fail %");
    lMap.insert(TEST_FAILBIN,      "Fail Bin");
    lMap.insert(TEST_TESTFLOWID,   "Flow ID");
    lMap.insert(TEST_OUTLIER,      "Removed Count");
    lMap.insert(TEST_MEAN,         "Mean");
    lMap.insert(TEST_MEANSHIFT,    "Mean shift %");
    lMap.insert(TEST_T_TEST,       "T-Test");
    lMap.insert(TEST_SIGMA,        "Sigma");
    lMap.insert(TEST_SIGMASHIFT,   "Sigma shift %");
    lMap.insert(TEST_2_SIGMA,      "2xSigma");
    lMap.insert(TEST_3_SIGMA,      "3xSigma");
    lMap.insert(TEST_6_SIGMA,      "6xSigma");
    lMap.insert(TEST_MIN,          "Min.");
    lMap.insert(TEST_MAX,          "Max.");
    lMap.insert(TEST_RANGE,        "Range");
    lMap.insert(TEST_MAX_RANGE,    "Max. Range");
    lMap.insert(TEST_CP,           "Cp");
    lMap.insert(TEST_CR,           "Cr");
    lMap.insert(TEST_CPSHIFT,      "Cp shift %");
    lMap.insert(TEST_CRSHIFT,      "Cr shift %");
    lMap.insert(TEST_CPK,          "Cpk");
    lMap.insert(TEST_CPKL,         "Cpl");
    lMap.insert(TEST_CPKH,         "Cpu");
    lMap.insert(TEST_CPKSHIFT,     "Cpk shift %");
    lMap.insert(TEST_YIELD,        "Yield");
    lMap.insert(TEST_GAGE_EV,      "Gage: EV");
    lMap.insert(TEST_GAGE_AV,      "Gage: AV");
    lMap.insert(TEST_GAGE_RR,      "Gage: R&R");
    lMap.insert(TEST_GAGE_GB,      "Gage: GB");
    lMap.insert(TEST_GAGE_PV,      "Gage: PV");
    lMap.insert(TEST_GAGE_TV,      "Gage: TV");
    lMap.insert(TEST_GAGE_P_T,     "Gage: P/T");
    lMap.insert(TEST_SKEW,         "Skew");
    lMap.insert(TEST_KURTOSIS,     "Kurtosis");
    lMap.insert(TEST_P0_5,         "P0.5%");
    lMap.insert(TEST_P2_5,         "P2.5%");
    lMap.insert(TEST_P10,          "P10%");
    lMap.insert(TEST_Q1,           "P25% - Q1");
    lMap.insert(TEST_Q2,           "P50% - Median");
    lMap.insert(TEST_Q3,           "P75% - Q3");
    lMap.insert(TEST_P90,          "P90%");
    lMap.insert(TEST_P97_5,        "P97.5%");
    lMap.insert(TEST_P99_5,        "P99.5%");
    lMap.insert(TEST_IQR,          "IQR");
    lMap.insert(TEST_SIGMAIQR,     "IQR SD");

    return lMap;
}

static QMap<QString, QString> columnsLabel = initColumnsLabel();

class CTest;
//-- Pair of groupID and CTest pointer
typedef QList< QPair<int,CTest*> >         CTestContainer;
typedef CTestContainer::Iterator           CTestContainerIter;
typedef CTestContainer::ConstIterator      CTestContainerConstIter;

#endif // TABLERENDERERKEYS_H
