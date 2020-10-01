#include <gqtl_log.h>

#include "settings_dialog.h"
#include "settings_sql.h"
#include "picktest_dialog.h"
#include "gex_constants.h"
#include "browser_dialog.h"
#include "report_options.h"
#include "browser.h"
#include "product_info.h"
#include "pat_report_ft_gui.h"

// script_wizard.h
extern void ConvertToScriptString(QString &strFile);
// extern void ConvertFromScriptString(QString &strFile);		Not Used

extern GexMainwindow *	pGexMainWindow;
// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

///////////////////////////////////////////////////////////
// Writes 'SetReportType' sub-section in script file: Test Statistics
///////////////////////////////////////////////////////////
void GexSettings::WriteReportSettingsSection_Stats(FILE *hFile)
{
    int		iMenuSelection;

    fprintf(hFile,"  // Section: Test Statistics\n");

    iMenuSelection = comboStats->currentIndex();
    switch(iMenuSelection)
    {
        case GEX_STATISTICS_DISABLED:		// disabled
        default:
            fprintf(hFile,"  gexReportType('stats','disabled');\n");
            break;
        case GEX_STATISTICS_ALL:			// All tests
            fprintf(hFile,"  gexReportType('stats','all');\n");
            break;
        case GEX_STATISTICS_FAIL:			// Failing tests
            fprintf(hFile,"  gexReportType('stats','fails');\n");
            break;
        case GEX_STATISTICS_OUTLIERS:		// Tests with outliers
            fprintf(hFile,"  gexReportType('stats','outliers');\n");
            break;
        case GEX_STATISTICS_LIST:			// List of tests
            fprintf(hFile,"  gexReportType('stats','tests','%s');\n",editStats->text().toLatin1().constData());
            break;
        case GEX_STATISTICS_BADCP:			// Cp limit (report all tests with Cp lower or equal)
            fprintf(hFile,"  gexReportType('stats','cp','%s');\n",editStats->text().toLatin1().constData());
            break;
        case GEX_STATISTICS_BADCPK:			// Cpk limit (report all tests with Cpk lower or equal)
            fprintf(hFile,"  gexReportType('stats','cpk','%s');\n", editStats->text().toLatin1().constData());
            break;
        case GEX_STATISTICS_TOP_N_FAILTESTS:
            fprintf(hFile,"  gexReportType('stats','top_n_fails','%s');\n", editStats->text().toLatin1().constData());
            break;
    }
}

///////////////////////////////////////////////////////////
// Writes 'SetReportType' sub-section in script file: WaferMap
///////////////////////////////////////////////////////////
void GexSettings::WriteReportSettingsSection_WaferMap(FILE *hFile)
{
    int		iMenuSelection;
    QString strString;

    fprintf(hFile,"\n  // Section: Wafer map\n");

    switch(comboWafermapTests->currentIndex())
    {
        case GEX_WAFMAP_ALL:		// Wafermap for ALL tests
            strString = "all";
            break;
        case GEX_WAFMAP_LIST:	// Wafermap for list of tests
            strString = editWafer->text();
            break;
        case GEX_WAFMAP_TOP_N_FAILTESTS:
            strString = "top "+ editWafer->text()+" failtests";
            break;
    }

    iMenuSelection = comboWafer->currentIndex();
    switch(iMenuSelection)
    {
        case GEX_WAFMAP_DISABLED:		// disabled
        default:
            fprintf(hFile,"  gexReportType('wafer','disabled');\n");
            break;
        case GEX_WAFMAP_SOFTBIN:		// Software Binning
            fprintf(hFile,"  gexReportType('wafer','soft_bin');\n");
            break;
        case GEX_WAFMAP_STACK_SOFTBIN:
            fprintf(hFile,"  gexReportType('wafer','stack_soft_bin','%s');\n", editWafer->text().toLatin1().constData());
            break;
        case GEX_WAFMAP_HARDBIN:		// Hardware binning
            fprintf(hFile,"  gexReportType('wafer','hard_bin');\n");
            break;
        case GEX_WAFMAP_STACK_HARDBIN:
            fprintf(hFile,"  gexReportType('wafer','stack_hard_bin','%s');\n", editWafer->text().toLatin1().constData());
            break;
        case GEX_WAFMAP_TESTOVERLIMITS: // Test characterization (Zoning) wafermap
            fprintf(hFile,"  gexReportType('wafer','param_over_limits','%s');\n", strString.toLatin1().constData());
            break;
        case GEX_WAFMAP_STACK_TESTOVERLIMITS:
            fprintf(hFile,"  gexReportType('wafer','stack_param_over_limits','%s');\n", strString.toLatin1().constData());
            break;
        case GEX_WAFMAP_TESTOVERDATA:	// Test characterization (Zoning) wafermap
            fprintf(hFile,"  gexReportType('wafer','param_over_range','%s');\n", strString.toLatin1().constData());
            break;
        case GEX_WAFMAP_STACK_TESTOVERDATA:
            fprintf(hFile,"  gexReportType('wafer','stack_param_over_range','%s');\n", strString.toLatin1().constData());
            break;
        case GEX_WAFMAP_TEST_PASSFAIL: // Test characterization (Zoning) wafermap
            fprintf(hFile,"  gexReportType('wafer','test_passfail','%s');\n", strString.toLatin1().constData());
            break;
        case GEX_WAFMAP_STACK_TEST_PASSFAIL:
            fprintf(hFile,"  gexReportType('wafer','stack_test_passfail','%s');\n", strString.toLatin1().constData());
            break;
        case GEX_WAFMAP_ZONAL_SOFTBIN:		// Zoanl Soft binning
            fprintf(hFile,"  gexReportType('wafer','zonal_soft_bin','%s');\n", editWafer->text().toLatin1().constData());
            break;
        case GEX_WAFMAP_ZONAL_HARDBIN:		// Zonal Hardware binning
            fprintf(hFile,"  gexReportType('wafer','zonal_hard_bin','%s');\n", editWafer->text().toLatin1().constData());
            break;
    }
}

///////////////////////////////////////////////////////////
// Writes 'SetReportType' sub-section in script file: Histograms
///////////////////////////////////////////////////////////
void GexSettings::WriteReportSettingsSection_Histograms(FILE *hFile)
{
    int		iMenuSelection;
    QString strString;

    switch(comboHistogramTests->currentIndex())
    {
        case GEX_HISTOGRAM_ALL:		// Histogram ALL tests
            strString = "all";
            break;
        case GEX_HISTOGRAM_LIST:	// Histogram list of tests
            strString = editHistogram->text();
            break;
        case GEX_HISTOGRAM_TOP_N_FAIL_TESTS:
            strString = "top "+ editHistogram->text()+" failtests";
            break;
    }

    iMenuSelection = comboHistogram->currentIndex();
    switch(iMenuSelection)
    {
        case GEX_HISTOGRAM_DISABLED:		// disabled
        default:
            fprintf(hFile,"  gexReportType('histogram','disabled');\n");
            break;
        case GEX_HISTOGRAM_OVERLIMITS:		// Histogram of tests over TestLimits
            fprintf(hFile,"  gexReportType('histogram','test_over_limits','%s');\n", strString.toLatin1().constData());
            break;
        case GEX_HISTOGRAM_CUMULLIMITS:		// cumul-Histogram of tests over TestLimits
            fprintf(hFile,"  gexReportType('histogram','cumul_over_limits','%s');\n", strString.toLatin1().constData());
            break;
        case GEX_HISTOGRAM_OVERDATA:		// Histogram of tests over TestRange
            fprintf(hFile,"  gexReportType('histogram','test_over_range','%s');\n", strString.toLatin1().constData());
            break;
        case GEX_HISTOGRAM_CUMULDATA:		// cumul-Histogram of tests over TestRange
            fprintf(hFile,"  gexReportType('histogram','cumul_over_range','%s');\n", strString.toLatin1().constData());
            break;
        case GEX_HISTOGRAM_DATALIMITS:		// adaptive to show both: data & limits
            fprintf(hFile,"  gexReportType('histogram','adaptive','%s');\n", strString.toLatin1().constData());
            break;
    }
}

///////////////////////////////////////////////////////////
// Writes 'SetReportType' sub-section in script file: Advanced Reports
///////////////////////////////////////////////////////////
bool GexSettings::WriteReportSettingsSection_Advanced(FILE* hFile,
                                                      bool /*bSetDrillParams*/)
{
    int		iMenuSelection;
    QString strString;
    QString strTest1;
    QString strTest2;
    bool	bWhatIf=false;
    QString strFuncTestType = "";

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" '%1' Tests : '%2'")
           .arg(comboAdvancedReport->currentText())
           .arg(comboAdvanced->currentText()).toLatin1().constData()
           );
    fprintf(hFile,"\n  // Section: Advanced Report\n");
    fprintf(hFile,"  gexReportType('adv_my_report','disabled');\n");	// Disable template...unless template file selected later on

    if(comboAdvanced->currentText()==ADV_ALL_TESTS)
        strString = "all";	// GEX_ADV_ALLTESTS:		// ALL tests
    else if(comboAdvanced->currentText()==ADV_TEST_LIST)
        strString = editAdvanced->text();  // Test#/list/range to process //GEX_ADV_TESTSLIST:	// list of tests
    else if(comboAdvanced->currentText()==ADV_TOP_N_FAILTEST)
        strString = "top "+ editAdvanced->text()+" failtests";	// GEX_ADV_TOP_N_FAILTESTS: // Top N fail test

    //strString = editAdvanced->text();
    strString = strString.trimmed();	// remove leading spaces.

    iMenuSelection = comboAdvancedReport->currentData().toInt();
    switch(iMenuSelection)
    {
        case GEX_ADV_DISABLED: // Advanced reports is Disabled
            default:	// Text separators
            fprintf(hFile,"  gexReportType('adv_histogram','disabled');\n");
            fprintf(hFile,"  gexReportType('adv_trend','disabled');\n");
            fprintf(hFile,"  gexReportType('adv_correlation','disabled');\n");
            fprintf(hFile,"  gexReportType('adv_boxplot','disabled');\n");
            fprintf(hFile,"  gexReportType('adv_multichart','disabled');\n");
            fprintf(hFile,"  gexReportType('adv_datalog','disabled');\n");
            fprintf(hFile,"  gexReportType('adv_optimizer','disabled');\n");
            break;

        case GEX_ADV_FUNCTIONAL:

            strFuncTestType =
                    (comboAdvancedReportSettings->currentIndex() == GEX_ADV_FUNCTIONAL_CYCL_CNT) ? "cycl_cnt"
                    : ((comboAdvancedReportSettings->currentIndex() == GEX_ADV_FUNCTIONAL_REL_VAD) ? "rel_vadr" :"uknown");
            fprintf(hFile,"  gexReportType('adv_functional_histogram','%s','%s');\n",strFuncTestType.toLatin1().constData(),strString.toLatin1().constData());
            break;
        case GEX_ADV_FTR_CORRELATION:
            switch(comboAdvancedReportSettings->currentIndex()){
                default:
                    fprintf(hFile,"  gexReportType('adv_ftr_correlation','all');\n");
                    break;
                case GEX_ADV_FTR_CORRELATION_LIST:
                    fprintf(hFile,"  gexReportType('adv_ftr_correlation','tests','%s');\n",editAdvanced->text().trimmed().toLatin1().constData());
                    break;
            }
            break;

        // Adv Histogram
        case GEX_ADV_HISTOGRAM:
            switch(comboAdvancedReportSettings->currentIndex())
            {
                case GEX_ADV_HISTOGRAM_OVERLIMITS:		// chart over test limits
                default:
                    fprintf(hFile,"  gexReportType('adv_histogram','test_over_limits','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_HISTOGRAM_CUMULLIMITS:		// cumul over test limits
                    fprintf(hFile,"  gexReportType('adv_histogram','cumul_over_limits','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_HISTOGRAM_OVERDATA: // chart over test results
                    fprintf(hFile,"  gexReportType('adv_histogram','test_over_range','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_HISTOGRAM_CUMULDATA:	// cumul over test results
                    fprintf(hFile,"  gexReportType('adv_histogram','cumul_over_range','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_HISTOGRAM_DATALIMITS:	// Adaptive: data + test limits visible on chart.
                    fprintf(hFile,"  gexReportType('adv_histogram','adaptive','%s');\n",strString.toLatin1().constData());
                    break;
            }
            break;

        // Trend chart
        case GEX_ADV_TREND:
            switch(comboAdvancedReportSettings->currentIndex())
            {
                case GEX_ADV_TREND_OVERLIMITS:
                default:
                    fprintf(hFile,"  gexReportType('adv_trend','test_over_limits','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_OVERDATA:
                    fprintf(hFile,"  gexReportType('adv_trend','test_over_range','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_DATALIMITS:	// Adaptive: data + test limits visible on chart.
                    fprintf(hFile,"  gexReportType('adv_trend','adaptive','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_DIFFERENCE:
                    // Get TestX & TestY values.
                    strTest1 = editTestX->text();
                    strTest2 = editTestY->text();
                    fprintf(hFile,"  gexReportType('adv_trend','difference','%s,%s');\n",strTest1.toLatin1().constData(),strTest2.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_AGGREGATE_MEAN:
                    fprintf(hFile,"  gexReportType('adv_trend','test_mean','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_AGGREGATE_SIGMA:
                    fprintf(hFile,"  gexReportType('adv_trend','test_sigma','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_AGGREGATE_CP:
                    fprintf(hFile,"  gexReportType('adv_trend','test_cp','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_AGGREGATE_CPK:
                    fprintf(hFile,"  gexReportType('adv_trend','test_cpk','%s');\n",strString.toLatin1().constData());
                    break;

                case GEX_ADV_TREND_SOFTBIN_SBLOTS:
                    fprintf(hFile,"  gexReportType('adv_trend','soft_bin_sublots','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_SOFTBIN_PARTS:
                    fprintf(hFile,"  gexReportType('adv_trend','soft_bin_parts','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_HARDBIN_SBLOTS:
                    fprintf(hFile,"  gexReportType('adv_trend','hard_bin_sublots','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_HARDBIN_PARTS:
                    fprintf(hFile,"  gexReportType('adv_trend','hard_bin_parts','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_SOFTBIN_ROLLING:
                    fprintf(hFile,"  gexReportType('adv_trend','soft_bin_rolling','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_TREND_HARDBIN_ROLLING:
                    fprintf(hFile,"  gexReportType('adv_trend','hard_bin_rolling','%s');\n",strString.toLatin1().constData());
                    break;
            }
            break;

        // Correlation Scatter chart
        case GEX_ADV_CORRELATION:
            // Get Test in X, and Test in Y values.
            strTest1 = editTestX->text();
            strTest2 = editTestY->text();
            switch(comboAdvancedReportSettings->currentIndex())
            {
                case GEX_ADV_CORR_OVERLIMITS:
                default:
                    fprintf(hFile,"  gexReportType('adv_correlation','test_over_limits','%s %s');\n", strTest1.toLatin1().constData(), strTest2.toLatin1().constData());
                    break;
                case GEX_ADV_CORR_OVERDATA:
                    fprintf(hFile,"  gexReportType('adv_correlation','test_over_range','%s %s');\n", strTest1.toLatin1().constData(), strTest2.toLatin1().constData());
                    break;
                case GEX_ADV_CORR_DATALIMITS:
                    fprintf(hFile,"  gexReportType('adv_correlation','adaptive','%s %s');\n", strTest1.toLatin1().constData(), strTest2.toLatin1().constData());
                    break;
            }
            break;

        // Adv boxplot
        case GEX_ADV_PROBABILITY_PLOT:
            switch(comboAdvancedReportSettings->currentIndex())
            {
                case GEX_ADV_PROBPLOT_OVERLIMITS:		// chart over test limits
                default:
                    fprintf(hFile,"  gexReportType('adv_probabilityplot','test_over_limits','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_PROBPLOT_OVERDATA: // chart over test results
                    fprintf(hFile,"  gexReportType('adv_probabilityplot','test_over_range','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_PROBPLOT_DATALIMITS:	// Adaptive: data + test limits visible on chart.
                    fprintf(hFile,"  gexReportType('adv_probabilityplot','adaptive','%s');\n",strString.toLatin1().constData());
                    break;
            }
            break;

        // Adv boxplot
        case GEX_ADV_BOXPLOT:
            switch(comboAdvancedReportSettings->currentIndex())
            {
                case GEX_ADV_BOXPLOT_OVERLIMITS:		// chart over test limits
                default:
                    fprintf(hFile,"  gexReportType('adv_boxplot_ex','test_over_limits','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_BOXPLOT_OVERDATA: // chart over test results
                    fprintf(hFile,"  gexReportType('adv_boxplot_ex','test_over_range','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_BOXPLOT_DATALIMITS:	// Adaptive: data + test limits visible on chart.
                    fprintf(hFile,"  gexReportType('adv_boxplot_ex','adaptive','%s');\n",strString.toLatin1().constData());
                    break;
            }
            break;

        // Adv boxplot
        case GEX_ADV_MULTICHART:
            switch(comboAdvancedReportSettings->currentIndex())
            {
                case GEX_ADV_MULTICHART_OVERLIMITS:		// chart over test limits
                default:
                    fprintf(hFile,"  gexReportType('adv_multichart','test_over_limits','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_MULTICHART_OVERDATA: // chart over test results
                    fprintf(hFile,"  gexReportType('adv_multichart','test_over_range','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_MULTICHART_DATALIMITS:	// Adaptive: data + test limits visible on chart.
                    fprintf(hFile,"  gexReportType('adv_multichart','adaptive','%s');\n",strString.toLatin1().constData());
                    break;
            }
            break;

        // BoxPlot
        case GEX_ADV_CANDLE_MEANRANGE:
            switch(comboAdvancedReportSettings->currentIndex())
            {

                case GEX_ADV_ALL:
                default:
                    fprintf(hFile,"  gexReportType('adv_boxplot','tests','all');\n");
                    break;
                case GEX_ADV_LIST:
                    fprintf(hFile,"  gexReportType('adv_boxplot','tests','%s');\n",strString.toLatin1().constData());
                    break;
            }
            break;

        // Datalog
        case GEX_ADV_DATALOG:
            switch(comboAdvancedReportSettings->currentIndex())
            {
                case GEX_ADV_DATALOG_ALL:
                default:
                    fprintf(hFile,"  gexReportType('adv_datalog','all');\n");
                    break;
                case GEX_ADV_DATALOG_OUTLIER:
                    fprintf(hFile,"  gexReportType('adv_datalog','outliers','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_DATALOG_FAIL:
                    fprintf(hFile,"  gexReportType('adv_datalog','fails');\n");
                    break;
                case GEX_ADV_DATALOG_LIST:
                    fprintf(hFile,"  gexReportType('adv_datalog','tests','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_DATALOG_RAWDATA:
                    fprintf(hFile,"  gexReportType('adv_datalog','tests_only','%s');\n",strString.toLatin1().constData());
                    break;
            }
            break;

        // Diagnostics
        case GEX_ADV_PEARSON:
            /*
            //GSLOG(SYSLOG_SEV_WARNING, "Check me : imposing here some options in case of 2 datasets AND Pearson ?");
            // Is there anyway to know here the user ask for a Compare files/DataSet ?
            if ()
            {
                fprintf(hFile,"  // impose some options for the Pearson to be correct :\n");
                fprintf(hFile,"  gexOptions('dataprocessing','clean_samples','true');\n");
                fprintf(hFile,"  gexOptions('dataprocessing','sublot_sorting','partid');\n");
            }
            */

            switch(comboAdvancedReportSettings->currentIndex())
            {
                case GEX_ADV_PEARSON_OVERLIMITS:
                    fprintf(hFile,"  gexReportType('adv_pearson','test_over_limits','%s');\n", strString.toLatin1().constData());
                break;
                case GEX_ADV_PEARSON_OVERDATA:
                    fprintf(hFile,"  gexReportType('adv_pearson','test_over_range','%s');\n", strString.toLatin1().constData());
                break;
                case GEX_ADV_PEARSON_DATALIMITS:
                    fprintf(hFile,"  gexReportType('adv_pearson','adaptive','%s');\n", strString.toLatin1().constData());
                break;
                default:
                    GSLOG(SYSLOG_SEV_WARNING, QString("Unknown setting %1").arg( comboAdvancedReportSettings->currentText()).toLatin1().constData());
                    fprintf(hFile,"  gexReportType('adv_pearson','');\n");
                    break;
            }
        break;

        // Displays PAT-Man report created in STDF.DTR records
        case GEX_ADV_PAT_TRACEABILITY:
            if (pGexMainWindow->GetWizardType() == GEX_FT_PAT_FILES_WIZARD)
            {
                if (pGexMainWindow->mPATReportFTGui)
                {
                    strString = pGexMainWindow->mPATReportFTGui->GetTraceabilityFile();
                    ConvertToScriptString(strString);
                    fprintf(hFile,
                            "  gexReportType('adv_pat','traceability_file', '%s');\n",
                            strString.toLatin1().constData());
                }
                else
                    GSLOG(SYSLOG_SEV_WARNING, "FT PAT report gui not allocated");
            }
            else
                fprintf(hFile,"  gexReportType('adv_pat','');\n");
        break;

        // Production reports (yield trend)
        case GEX_ADV_PROD_YIELD:
            // Define binning to use: soft_bin or hard_bin
            if(comboAdvancedReportSettingsBinningType->currentIndex())
                strString = "hard_bin";
            else
                strString = "soft_bin";
            fprintf(hFile,"  gexReportType('adv_global_options','binning','%s');\n",strString.toLatin1().constData());

            strString = lineEditAdvTitle->text();	// Chart title
            strString = strString.trimmed();	// remove leading spaces.
            if(!strString.isEmpty())
                fprintf(hFile,"  gexReportType('adv_production_yield','title','%s');\n",strString.toLatin1().constData());

            strString = editAdvancedProduction->text();	// Bin#/list to use for Yield computation
            strString = strString.trimmed();	// remove leading spaces.
            if(strString.isEmpty())
                strString = "1";	// If no binning specified, assume it's Bin1
            switch(comboAdvancedReportSettings->currentIndex())
            {
                case GEX_ADV_PRODYIELD_SBLOT:
                    fprintf(hFile,"  gexReportType('adv_production_yield','sublot','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_PRODYIELD_LOT:
                    fprintf(hFile,"  gexReportType('adv_production_yield','lot','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_PRODYIELD_GROUP:
                    fprintf(hFile,"  gexReportType('adv_production_yield','group','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_PRODYIELD_DAY:
                    fprintf(hFile,"  gexReportType('adv_production_yield','daily','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_PRODYIELD_WEEK:
                    fprintf(hFile,"  gexReportType('adv_production_yield','weekly','%s');\n",strString.toLatin1().constData());
                    break;
                case GEX_ADV_PRODYIELD_MONTH:
                    fprintf(hFile,"  gexReportType('adv_production_yield','monthly','%s');\n",strString.toLatin1().constData());
                    break;
            }
            switch(comboBoxYieldAxis->currentIndex())
            {
                case GEX_ADV_PROD_YIELDAXIS_0_100:
                    fprintf(hFile,"  gexReportType('adv_production_yield','yield_axis','0-100');\n");
                    break;
                case GEX_ADV_PROD_YIELDAXIS_0_MAX:
                    fprintf(hFile,"  gexReportType('adv_production_yield','yield_axis','0-max');\n");
                    break;
                case GEX_ADV_PROD_YIELDAXIS_MIN_100:
                    fprintf(hFile,"  gexReportType('adv_production_yield','yield_axis','min-100');\n");
                    break;
                case GEX_ADV_PROD_YIELDAXIS_MIN_MAX:
                    fprintf(hFile,"  gexReportType('adv_production_yield','yield_axis','min-max');\n");
                    break;
            }
            switch(comboBoxVolumeAxis->currentIndex())
            {
                case GEX_ADV_PROD_VOLUMEAXIS_0_MAX:
                    fprintf(hFile,"  gexReportType('adv_production_yield','volume_axis','0-max','0');\n");
                    break;
                case GEX_ADV_PROD_VOLUMEAXIS_0_CUSTOM:
                    fprintf(hFile,"  gexReportType('adv_production_yield','volume_axis','0-custom','%d');\n", spinBoxVolumeAxis->value());
                    break;
                case GEX_ADV_PROD_VOLUMEAXIS_MIN_MAX:
                    fprintf(hFile,"  gexReportType('adv_production_yield','volume_axis','min-max','0');\n");
                    break;
                case GEX_ADV_PROD_VOLUMEAXIS_MIN_CUSTOM:
                    fprintf(hFile,"  gexReportType('adv_production_yield','volume_axis','min-custom','%d');\n", spinBoxVolumeAxis->value());
                    break;
            }
            if(checkBoxYieldMarker->isChecked())
                fprintf(hFile,"  gexReportType('adv_production_yield','yield_marker','%d');\n", spinBoxYieldMarker->value());
            else
                fprintf(hFile,"  gexReportType('adv_production_yield','yield_marker','0');\n");
            break;

        case GEX_ADV_GUARDBANDING:
            bWhatIf = true;
            fprintf(hFile,"  gexReportType('drill_what_if');\n");
            // Ensure stats computed from samples!
            fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");
            // Ensure binning computed from wafermap!
            fprintf(hFile,"  gexOptions('binning','computation','wafer_map');\n");
            // Ensure we are in 'Continue on Fail' mode!
            fprintf(hFile,"  gexOptions('dataprocessing','fail_count', 'all');\n");
            break;

        case GEX_ADV_TEMPLATE:	// 'MyReport' Template selected: for creating custom report.
            // Convert file path to be script compatible '\' become '\\'
            strString = m_strTemplateFile;
            ConvertToScriptString(strString);
            fprintf(hFile,"  gexReportType('adv_my_report','template','%s');\n",strString.toLatin1().constData());
            break;

        case GEX_ADV_SHIFT:
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "GEX_ADV_SHIFT");
              fprintf(hFile,"  gexReportType('adv_shift');\n");
              fprintf(hFile,"  // imposed options. Do not modify. Mandatory for the Shift analysis to work\n");
              fprintf(hFile,"  gexOptions('dataprocessing','part_id','show');\n");
              fprintf(hFile,"  gexOptions('dataprocessing','scaling','normalized');\n");
              fprintf(hFile,"  gexOptions('dataprocessing','clean_samples','true');\n");
              fprintf(hFile,"  gexOptions('dataprocessing','sublot_sorting','partid');\n");
              fprintf(hFile,"  gexOptions('adv_trend','x_axis','part_id');\n");
            break;
//        case GEX_ADV_CHARAC_BOXWHISKER_CHART:
//            switch(comboAdvancedReportSettings->currentIndex())
//            {
//                case GEX_ADV_CHARAC_CHART_OVERLIMITS:		// chart over test limits
//                default:
//                    fprintf(hFile,"  gexReportType('adv_charac1','test_over_limits','%s');\n", strString.toLatin1().constData());
//                    break;
//                case GEX_ADV_CHARAC_CHART_OVERDATA: // chart over test results
//                    fprintf(hFile,"  gexReportType('adv_charac1','test_over_range','%s');\n", strString.toLatin1().constData());
//                    break;
//                case GEX_ADV_CHARAC_CHART_DATALIMITS:	// Adaptive: data + test limits visible on chart.
//                    fprintf(hFile,"  gexReportType('adv_charac1','adaptive','%s');\n", strString.toLatin1().constData());
//                    break;
//            }
//            break;
    }
    return bWhatIf;
}

void GexSettings::WriteReportSettingsSection_Limits(FILE *file)
{
    // Overwrite default 'used limits' option with the one from the setting page
    QString lUsedLimit = comboBoxUsedLimits->currentData().toString();
    fprintf(file,"  gexOptions('dataprocessing','used_limits','%s');\n", lUsedLimit.toLatin1().constData());
    fprintf(file,"\n");
}

///////////////////////////////////////////////////////////
// Writes 'SetReportType' section in script file
///////////////////////////////////////////////////////////
bool GexSettings::WriteScriptReportSettingsSection(FILE *hFile,bool bSetDrillParams,QString &strStartupPage)
{
    bool bWhatIf = false;

    // Writes 'Favorite Scripts' section
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"// Setup the GEX 'Settings' section\n");
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"SetReportType()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n");

    // Write report settings
    bWhatIf = WriteReportSettingsSection(hFile, bSetDrillParams, strStartupPage);

    // Message to console saying 'success loading options'
    fprintf(hFile,"\n  sysLog('* Quantix Examinator Settings loaded! *');\n");


    // Close function
    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");

    // Notifies caller if a 'Drill' action is included...if so, will not create the standard Drill script section to avoid conflict.
    return bWhatIf;
}

///////////////////////////////////////////////////////////
// Writes 'SetReportType' section in script file
///////////////////////////////////////////////////////////
bool GexSettings::WriteReportSettingsSection(FILE *hFile,bool bSetDrillParams,QString &strStartupPage)
{
    int		iMenuSelection;
    bool	bWhatIf = false;

    // If user is overwriting the default output defined in the settings, take into account the change.
    if(comboBoxOutputFormat->isVisible())
    {
        fprintf(hFile,"  // Update output format\n");
        switch(comboBoxOutputFormat->currentIndex())
        {
            case GEX_SETTINGS_OUTPUT_HTML: // HTML output
                fprintf(hFile,"  gexOptions('output','format','html');\n");
                break;
            case GEX_SETTINGS_OUTPUT_WORD: // Word output
                fprintf(hFile,"  gexOptions('output','format','word');\n");
                break;
            case GEX_SETTINGS_OUTPUT_PPT: // PPT output
                fprintf(hFile,"  gexOptions('output','format','ppt');\n");
                break;
            case GEX_SETTINGS_OUTPUT_PDF: // PDF output
                fprintf(hFile,"  gexOptions('output','format','pdf');\n");
                break;
            case GEX_SETTINGS_OUTPUT_CSV: // CSV  output
                fprintf(hFile,"  gexOptions('output','format','csv');\n");
                break;
            case GEX_SETTINGS_OUTPUT_INTERACTIVE:
                fprintf(hFile,"  gexOptions('output','format','interactive');\n");
                break;
            case GEX_SETTINGS_OUTPUT_ODT:
                fprintf(hFile,"  gexOptions('output','format','ODT');\n");
                break;
            default:
                GSLOG(SYSLOG_SEV_WARNING, QString("No known output format for combobox index %1")
                      .arg( comboBoxOutputFormat->currentIndex()).toLatin1().constData());
                break;
        }
        fprintf(hFile,"\n");
    }

    // Check type of report to create
    switch(widgetStackReport->currentIndex())
    {
        case WIDGET_REPORT_STD:	// Standard Instant report
            // If Quantix Optimizer Diagnostic...enable ALL standard reports.
            iMenuSelection = comboAdvancedReport->currentData().toInt();
            if(iMenuSelection == GEX_ADV_GO_DIAGNOSTICS)
            {
                // Section: Quantix Optimizer Diagnostics
                fprintf(hFile,"  // Section: Quantix Optimizer Diagnostics\n");
                fprintf(hFile,"  gexReportType('stats','all');\n");
                fprintf(hFile,"  gexReportType('wafer','disabled');\n");
                WriteReportSettingsSection_Histograms(hFile);
                fprintf(hFile,"  gexReportType('adv_trend','test_over_limits','all');\n");
                fprintf(hFile,"  gexReportType('adv_optimizer','all');\n");

                // Force startup page to be the 'Advanced.htm' page
                strStartupPage = "advanced";
            }
            else
            {
                bWhatIf = WriteSettings_InteractiveReport(hFile,bSetDrillParams);
                if(bWhatIf)
                    strStartupPage = GEX_BROWSER_ACT_WHATIF;	// home page to show is "what_if" page!
            }
            break;

        case WIDGET_REPORT_SQL:	// SQL Enterprise Report
            WriteSettings_SqlReport(hFile);
            break;

        case WIDGET_REPORT_CHAR_WIZARD:   // Characterization Wizard
            WriteSettings_CharacterizationReport(hFile);

            // Force startup page to be the 'Advanced.htm' page
            strStartupPage = "advanced";
            break;

        default:
            GSLOG(SYSLOG_SEV_WARNING, "No default case");
            break;
    }

    // Notifies caller if a 'Drill' action is included...if so, will not create the standard Drill script section to avoid conflict.
    return bWhatIf;
}

///////////////////////////////////////////////////////////
// Interactive reports: Write associated CSL script file
///////////////////////////////////////////////////////////
bool GexSettings::WriteSettings_InteractiveReport(FILE *hFile,bool bSetDrillParams)
{
    // Limits
    WriteReportSettingsSection_Limits(hFile);

    // Test statistics
    WriteReportSettingsSection_Stats(hFile);

    // wafermap
    WriteReportSettingsSection_WaferMap(hFile);

    // histogram
    fprintf(hFile,"\n  // Section: Histogram\n");
    WriteReportSettingsSection_Histograms(hFile);

    // Advanced reports
    return WriteReportSettingsSection_Advanced(hFile,bSetDrillParams);
}

///////////////////////////////////////////////////////////
// If Interactive Drill enabled, write section.
///////////////////////////////////////////////////////////
void GexSettings::WriteBuildDrillSection(FILE* hFile,
                                         bool /*bWriteDrillSettings*/)
{
    // Writes 'Drill' section
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"// GEX 'DataMining - Yield Analysis' section\n");
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"SetDataMining()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n");
    fprintf(hFile,"  gexReportType('drill_disabled');\n");
#if 0

    // Close function
    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");
    return;

    // Check if Examinator-Instant Report
    if(pGexMainWindow->iGexAssistantSelected == GEX_MODULE_INSTANT_REPORT)
    {
        // Check if DRILL
        iDrillType = GEX_DRILL_GUARDBANDING;
    }
    else
    if(pGexMainWindow->iGexAssistantSelected != GEX_MODULE_DATA_MINING)
    {
        // Do not accept data mining functions...
        iDrillType = -1;
    }

    switch(iDrillType)
    {
        default:	// Text separators: Interactive Drill is Disabled
            fprintf(hFile,"  gexReportType('drill_disabled');\n");
            break;

        case GEX_DRILL_GUARDBANDING: // Guard banding 'What-if' investigation
            break;

        case GEX_DRILL_WAFERPROBE:		// Wafer probe parts
            switch(iDrillSubType)
            {
                case GEX_WAFMAP_SOFTBIN:		// Sotware binning
                case GEX_WAFMAP_STACK_SOFTBIN:
                    fprintf(hFile,"  gexReportType('drill_wafer','soft_bin');\n");
                    break;
                case GEX_WAFMAP_HARDBIN:		// Hardware binning
                case GEX_WAFMAP_STACK_HARDBIN:
                    fprintf(hFile,"  gexReportType('drill_wafer','hard_bin');\n");
                    break;
                case GEX_WAFMAP_TESTOVERLIMITS: // Test characterization (Zoning) wafermap
                case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                    fprintf(hFile,"  gexReportType('drill_wafer','param_over_limits','');\n");
                    break;
                case GEX_WAFMAP_TESTOVERDATA:	// Test characterization (Zoning) wafermap
                case GEX_WAFMAP_STACK_TESTOVERDATA:
                    fprintf(hFile,"  gexReportType('drill_wafer','param_over_range','');\n");
                    break;
                case GEX_WAFMAP_TEST_PASSFAIL:	// Test characterization (Zoning) wafermap
                case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                    fprintf(hFile,"  gexReportType('drill_wafer','test_passfail','');\n");
                    break;
            }
            break;

        case GEX_DRILL_PACKAGED:		// Packaged parts
            switch(iDrillSubType)
            {
                case GEX_WAFMAP_SOFTBIN:		// Sotware binning
                case GEX_WAFMAP_STACK_SOFTBIN:
                    fprintf(hFile,"  gexReportType('drill_packaged','soft_bin');\n");
                    break;
                case GEX_WAFMAP_HARDBIN:		// Hardware binning
                case GEX_WAFMAP_STACK_HARDBIN:
                    fprintf(hFile,"  gexReportType('drill_packaged','hard_bin');\n");
                    break;
                case GEX_WAFMAP_TESTOVERLIMITS: // Test characterization (Zoning) wafermap
                case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                    fprintf(hFile,"  gexReportType('drill_packaged','param_over_limits','1000');\n");
                    break;
                case GEX_WAFMAP_TESTOVERDATA:	// Test characterization (Zoning) wafermap
                case GEX_WAFMAP_STACK_TESTOVERDATA:
                    fprintf(hFile,"  gexReportType('drill_packaged','param_over_range','1000');\n");
                    break;
                case GEX_WAFMAP_TEST_PASSFAIL:	// Test characterization (Zoning) wafermap
                case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                    fprintf(hFile,"  gexReportType('drill_packaged','test_passfail','1000');\n");
                    break;
            }
            break;

        case GEX_DRILL_HISTOGRAM:	// Histogram
            fprintf(hFile,"  gexReportType('drill_histo');\n");
            break;
        case GEX_DRILL_TREND:		// Trend
            fprintf(hFile,"  gexReportType('drill_trend');\n");
            break;
        case GEX_DRILL_CORRELATION: // Correlation
            fprintf(hFile,"  gexReportType('drill_correlation');\n");
            break;
    }
#endif
    // Message to console saying 'success loading options'
    fprintf(hFile,"\n  sysLog('* Quantix Examinator Drill info loaded! *');\n");

    // Close function
    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");
}

