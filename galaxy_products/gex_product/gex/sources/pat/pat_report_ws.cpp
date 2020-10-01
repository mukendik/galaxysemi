#ifdef GCORE15334
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "engine.h"
//#include "pat_info.h"
#include "pat_definition.h"
#include "gex_pat_processing.h"
#include "pat_report_ws.h"
#include "gex_report.h"
#include "csl/csl_engine.h"
#include "browser.h"
#include "browser_dialog.h"
#include "script_wizard.h"
#include "outlierremoval/outlierremoval_taskdata.h"


//extern CPatInfo *       lPatInfo;
extern CGexReport *     gexReport;
extern GexMainwindow *  pGexMainWindow;
extern CReportOptions   ReportOptions;

extern QString          ConvertToScriptString(const QString &strFile);

namespace GS
{
namespace Gex
{

PATReportWS::PATReportWS(bool lShow)
    : mShow(lShow)
{
    lPatInfo = PATEngine::GetInstance().GetContext();
}

PATReportWS::~PATReportWS()
{

}

bool PATReportWS::Generate(const QString &outputFile, const PATProcessing& settings)
{
    mOutputFile = outputFile;
    mSettings   = settings;

    if (CreateScriptFile(Engine::GetInstance().GetAssistantScript()) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to create WS PAT report script file");
        return false;
    }

    if (ExecuteScriptFile(Engine::GetInstance().GetAssistantScript()) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to execute WS PAT report script file");
        return false;
    }

    // Report kept...check if option set to rename it to same name as trigger file?
    if(settings.iReportNaming == GEXMO_OUTLIER_REPORT_NAME_TRIGGERFILE)
    {
        QDir        lDir;
        QFileInfo	lFileInfo(settings.strTriggerFile);
        QString		lTriggerFileName = lFileInfo.baseName();	// Get trigger file name (without path, without extension)
        QString     lNewReportName;

        if (lTriggerFileName.isEmpty() == false)
        {
            // Get full path to report folder
            lFileInfo.setFile(gexReport->reportAbsFilePath());
            lNewReportName = lFileInfo.absolutePath() + "/" + lTriggerFileName + "." + lFileInfo.suffix();

            GS::Gex::Engine::RemoveFileFromDisk(lNewReportName);	// Delete file if exists

            // Rename report name to use trigger base name.
            lDir.rename(gexReport->reportAbsFilePath(), lNewReportName);

            // Update report name buffer
            if (gexReport->reportGenerationMode() == "legacy")
                gexReport->setLegacyReportName(lNewReportName);
            else
                gexReport->setReportName(lTriggerFileName);
        }
        else
        {
            GSLOG(SYSLOG_SEV_WARNING, "No trigger file name specified, default report naming is used.");
        }

    }

    return true;
}

const QString &PATReportWS::GetErrorMessage() const
{
    return mErrorMessage;
}

void PATReportWS::SetShow(bool show)
{
    mShow = show;
}

void PATReportWS::SetSites(const QList<int> &sites)
{
    mSites = sites;
}

bool PATReportWS::CreateScriptFile(const QString &scriptFile)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Create script file: '%1'").arg(scriptFile).toLatin1().data() );
    FILE* hFile = fopen(scriptFile.toLatin1().constData(),"w");

    if(hFile == NULL)
    {
        mErrorMessage = "  > Failed to create script file: " + scriptFile;
        return false;
    }

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        mErrorMessage = "  > Failed to write option section";
        return false;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile, "SetProcessData()\n");
    fprintf(hFile, "{\n");
    fprintf(hFile, "  var group_id;\n");
    fprintf(hFile, "  gexGroup('reset','all');\n");

    // Write dataset group
    QString lOutputDataFile = ConvertToScriptString(mOutputFile);

    // Check if single-site or multi-sites...
    if(mSites.count() > 1)
    {
        for (QList<int>::const_iterator iter = mSites.begin(); iter != mSites.end(); ++iter)
        {
            fprintf(hFile, "  group_id = gexGroup('insert','Site %d');\n", *iter);

            fprintf(hFile,
                    "  gexFile(group_id,'insert','%s','%d','last_instance',' ','','',' ');\n\n",
                    lOutputDataFile.toLatin1().constData(),
                    *iter);
        }
    }
    else
    {
        fprintf(hFile, "  group_id = gexGroup('insert','DataSet_1');\n");
        fprintf(hFile,
                "  gexFile(group_id,'insert','%s','all','last_instance',' ','','',' ');\n\n",
                lOutputDataFile.toLatin1().constData());
    }

    fprintf(hFile, "}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n\n");

    fprintf(hFile, "  gexOptions('output','format','%s');\n",
            mSettings.strOutputReportFormat.toLatin1().constData());

    // If no PAT limits clamping
    // Disable Examinator outlier GUI filter!
    fprintf(hFile,"  gexOptions('dataprocessing','data_cleaning_mode','none');\n");
    fprintf(hFile,"  gexOptions('dataprocessing','used_limits','standard_limits_only');\n");

    // Choose test merge option based on recipe test key option
    switch (lPatInfo->GetRecipeOptions().mOptionsTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            // Merge tests with same test number (even if test name is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            // Merge tests with same test name (even if test number is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge_name');\n");
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            // Merge tests with same test number and test name
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'no_merge');\n");
            break;

        default:
            // Use default option from options tab
            break;
    }

    // Forces to ALWAYS compute advanced statistics.
    fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");

    // Disables PartID support (uses less memory this way,
    // and not needed in reports created)
    fprintf(hFile,"  gexOptions('dataprocessing','part_id','hide');\n");

    // Force to compute statistics from samples, ignore summary.
    fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");
    fprintf(hFile,"  gexOptions('pareto','section','');\n");

    // Force computing binning from wafermap (so to avoid counting retest multiple times.
    fprintf(hFile,"  gexOptions('binning','computation','wafer_map');\n");

    // Disable STDF.FTR processing (for PAT speed improvement
    // since FTR are not used in PAT algorihtms)
    fprintf(hFile,"  gexOptions('dataprocessing','functional_tests','disabled');\n");
    // Force Part identification to use xy coordinates
    fprintf(hFile,"  gexOptions('dataprocessing', 'part_identification', 'xy');\n");

    // Disable mean, median and Nsigma markers
    fprintf(hFile,"  gexChartStyle('marker_mean','0 0 0 128');\n");
    fprintf(hFile,"  gexChartStyle('marker_median','0 0 0 128');\n");
    fprintf(hFile,"  gexChartStyle('marker_min','0 0 0 128');\n");
    fprintf(hFile,"  gexChartStyle('marker_max','0 0 0 128');\n");
    fprintf(hFile,"  gexChartStyle('marker_limits','0 255 0 0');\n");
    fprintf(hFile,"  gexChartStyle('marker_2sigma','0 0 0 128');\n");
    fprintf(hFile,"  gexChartStyle('marker_3sigma','0 0 0 128');\n");
    fprintf(hFile,"  gexChartStyle('marker_6sigma','0 0 0 128');\n");
    fprintf(hFile,"  gexChartStyle('marker_12sigma','0 0 0 128');\n");

    // Reset layer colors.
    fprintf(hFile,"  gexChartStyle('chart_layer','-1');\n");

    // Force wafermap binning colors.
    fprintf(hFile,"  gexBinStyle('clear','');\n");	// Empty current list.

    // Good Soft BINs (Pass dies)
    fprintf(hFile,"// Good SBIN bins\n");
    fprintf(hFile,
            "  gexBinStyle('bin_list','%s');\n",
            lPatInfo->GetRecipeOptions().pGoodSoftBinsList->GetRangeList().toLatin1().constData());
    fprintf(hFile,"  gexBinStyle('bin_color','0 128 0');\n");
    fprintf(hFile,"  gexBinStyle('add_bin','soft_bin');\n");

    // Good Hard BINs (Pass dies)
    fprintf(hFile,"// Good HBIN bins\n");
    fprintf(hFile,
            "  gexBinStyle('bin_list','%s');\n",
            lPatInfo->GetRecipeOptions().pGoodHardBinsList->GetRangeList().toLatin1().constData());
    fprintf(hFile,"  gexBinStyle('bin_color','0 128 0');\n");
    fprintf(hFile,"  gexBinStyle('add_bin','hard_bin');\n");

    // Used to build list of HBINs to show in Report distribution (Good HBIN + All PAT bins)
    QString lSampleBins = lPatInfo->GetRecipeOptions().pGoodHardBinsList->GetRangeList();

    if (WriteOutlierFailure(hFile, lSampleBins) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to write outliers failure bins");
        return false;
    }

    // Ensure last HBIN ends with a ',' separator
    lSampleBins += ",";

    // All other failures.: the overlap with previous PAT Fail bins definition is not a problem as Examinator stops on first mathcing bin (so always first declare Pat fail bins!)
    fprintf(hFile,"// Other bins\n");
    fprintf(hFile,"  gexBinStyle('bin_list','0,2-32767');\n");
    fprintf(hFile,"  gexBinStyle('bin_color','96 129 250');\n");
    fprintf(hFile,"  gexBinStyle('add_bin','soft_bin');\n");
    fprintf(hFile,"  gexBinStyle('add_bin','hard_bin');\n");

    // Enable custom wafermap colors
    fprintf(hFile,"  gexBinStyle('custom_colors','1');\n");

    fprintf(hFile,"  SetProcessData();\n");

    // Set Wafermap type in report: SoftBin or HardBin
    if(lPatInfo->GetRecipeOptions().iReport_WafermapType == 0)
        fprintf(hFile,"  gexReportType('wafer','soft_bin');\n");
    else
        fprintf(hFile,"  gexReportType('wafer','hard_bin');\n");

    // If gross die count available, specify it so reports will show Yield computed over gross die instead of physical dies in wafer
    if(mSettings.iGrossDiePerWafer > 0)
        fprintf(hFile,"  gexOptions('wafer','gross_die','%d');\n", mSettings.iGrossDiePerWafer);

    // Wafer orientation option has to be set depending on several sources
    WriteMapAxisDirection(hFile);

    QString lMarkerFile = Engine::GetInstance().GetAssistantScript() + ".gexParameter.csl";

    if (CreateMarkersFile(lMarkerFile, lSampleBins) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Failed to create markers file: %1").arg(lMarkerFile).toLatin1().constData());
        return false;
    }

    // full path to PAT markers file.
    lMarkerFile = ConvertToScriptString(lMarkerFile);
    fprintf(hFile,"  gexParameterFile('%s');\n", lMarkerFile.toLatin1().constData());

    // Write list of HBINs to use to compute statistics & show distributions
    lSampleBins = lSampleBins.replace(",,",","); // Ensure we do not have duplicates of separators.

    fprintf(hFile,"// Show distributions of this HBINS\n");
    fprintf(hFile,"  gexOptions('samples','hbin_list','%s');\n", lSampleBins.toLatin1().constData());

    // If some outliers identified, then create associated histograms & statistics
    QString lTestsWithOutliers = GetOutliersTestList();

    // GCORE-54
    // Build list of tests with no outliers
    QStringList lTWOList = lTestsWithOutliers.split(',', QString::SkipEmptyParts);
    QString lTestsWithNoOutliers;
    if(!gexReport->getGroupsList().isEmpty())
    {
        CGexGroupOfFiles *lGroup = gexReport->getGroupsList().first();
        CTest *     lTest = lGroup->cMergedData.ptMergedTestList;
        QString     lTestID;
        while(lTest)
        {
            lTestID = QString::number(lTest->lTestNumber);
            if (lTest->lPinmapIndex >= 0)
                lTestID += "." + QString::number(lTest->lPinmapIndex);
            if(!lTWOList.contains(lTestID))
            {
                if(!lTestsWithNoOutliers.isEmpty())
                    lTestsWithNoOutliers += ",";
                lTestsWithNoOutliers += lTestID;
            }
            lTest = lTest->GetNextTest();
        }
    }

    if(lPatInfo->GetRecipeOptions().bReport_Histo && lPatInfo->GetRecipeOptions().bReport_Histo_NoOutliers)
    {
        fprintf(hFile,"  gexReportType('adv_histogram','adaptive');\n");	// Adaptive: will show limits & data.

        // Disable test limits display over histogram (because corrupted in multi-site mode!).
        QStringList lHistoFields = ReportOptions.GetOption("adv_histogram","field").toString().split("|");
        fprintf(hFile,
                "  gexOptions('adv_histogram','field','%s');\n",
                lHistoFields.join("|").toLatin1().constData());

        // Histogram will be image we build (so we can see the markers)
        fprintf(hFile,"  gexReportType('histogram','advanced');\n");
        fprintf(hFile,
                "  gexReportType('histogram','test_over_range','all');\n");
        fprintf(hFile,
                "  gexReportType('adv_histogram','test_over_range','all');\n");

        fprintf(hFile,"  gexOptions('histogram','section_name','Histogram of Tests');\n");
    }
    else if(lPatInfo->GetRecipeOptions().bReport_Histo)
    {
        if(!lTestsWithOutliers.isEmpty())
        {
            fprintf(hFile,"  gexReportType('adv_histogram','adaptive');\n");	// Adaptive: will show limits & data.

            // Disable test limits display over histogram (because corrupted in multi-site mode!).
            QStringList lHistoFields = ReportOptions.GetOption("adv_histogram","field").toString().split("|");
            fprintf(hFile,
                    "  gexOptions('adv_histogram','field','%s');\n",
                    lHistoFields.join("|").toLatin1().constData());

            // Histogram will be image we build (so we can see the markers)
            fprintf(hFile,"  gexReportType('histogram','advanced');\n");
            fprintf(hFile,
                    "  gexReportType('histogram','test_over_range','%s');\n",
                    lTestsWithOutliers.toLatin1().constData());
            fprintf(hFile,
                    "  gexReportType('adv_histogram','test_over_range','%s');\n",
                    lTestsWithOutliers.toLatin1().constData());
        }
        else
        {
            fprintf(hFile,"  gexReportType('histogram','disabled');\n");
            fprintf(hFile,"  gexReportType('adv_histogram','disabled');\n");
        }

        fprintf(hFile,"  gexOptions('histogram','section_name','Histogram of Tests (failing PAT limits)');\n");
    }
    else if(lPatInfo->GetRecipeOptions().bReport_Histo_NoOutliers)
    {
        if(!lTestsWithNoOutliers.isEmpty())
        {
            fprintf(hFile,"  gexReportType('adv_histogram','adaptive');\n");	// Adaptive: will show limits & data.

            // Disable test limits display over histogram (because corrupted in multi-site mode!).
            QStringList lHistoFields = ReportOptions.GetOption("adv_histogram","field").toString().split("|");
            fprintf(hFile,
                    "  gexOptions('adv_histogram','field','%s');\n",
                    lHistoFields.join("|").toLatin1().constData());

            // Histogram will be image we build (so we can see the markers)
            fprintf(hFile,"  gexReportType('histogram','advanced');\n");
            fprintf(hFile,
                    "  gexReportType('histogram','test_over_range','%s');\n",
                    lTestsWithNoOutliers.toLatin1().constData());
            fprintf(hFile,
                    "  gexReportType('adv_histogram','test_over_range','%s');\n",
                    lTestsWithNoOutliers.toLatin1().constData());
        }
        else
        {
            fprintf(hFile,"  gexReportType('histogram','disabled');\n");
            fprintf(hFile,"  gexReportType('adv_histogram','disabled');\n");
        }

        fprintf(hFile,"  gexOptions('histogram','section_name','Histogram of Tests (not failing PAT limits)');\n");
    }
    else
    {
        fprintf(hFile,"  gexReportType('histogram','disabled');\n");
        fprintf(hFile,"  gexReportType('adv_histogram','disabled');\n");
    }

    if(lTestsWithOutliers.isEmpty() == false)
    {
        // Stats
        fprintf(hFile,"  gexReportType('stats','tests','%s');\n", lTestsWithOutliers.toLatin1().constData());

        if(lPatInfo->GetRecipeOptions().bReport_Stats)
            fprintf(hFile,
                    "  gexOptions('statistics','section_name','Statistics of Tests (failing PAT limits)');\n");
        else
            fprintf(hFile,"  gexReportType('stats','disabled','');\n");

        // Pareto
        if(lPatInfo->GetRecipeOptions().bReport_Pareto)
            fprintf(hFile,"  gexOptions('pareto','section','cp|cpk|failures');\n");
        else
            fprintf(hFile,"  gexOptions('pareto','section','');\n");	// Disabled binning
    }

    // Binning
    if(lPatInfo->GetRecipeOptions().bReport_Binning)
        fprintf(hFile,"  gexOptions('binning','section','enabled');\n");	// Binning enabled
    else
        fprintf(hFile,"  gexOptions('binning','section','disabled');\n");	// Disabled binning

    // Wafermap
    if(lPatInfo->GetRecipeOptions().bReport_Wafermap)
        fprintf(hFile,"  gexOptions('wafer','chart_show','all_individual');\n");	// Ensure wafermap is visible.
    else
        fprintf(hFile,"  gexOptions('wafer','chart_show','');\n");	// Disabled wafermap

    // Disable MyReports in case it is fromp a previous user session!
    fprintf(hFile,"  gexReportType('adv_my_report','disabled');\n");

    // 'More reports' section will hold the outlier removal summary table.
    fprintf(hFile,"  gexReportType('adv_data_cleaning','enabled');\n");

    // Get report built...and launch Viewer (unless disabled)
    if(mShow)
        fprintf(hFile,"  gexBuildReport('home','0');\n");	// Show report home page
    else
        fprintf(hFile,"  gexBuildReport('hide','0');\n");	// Do not show report page!

    // Now that custom report is created, make sure we reload standard Examinator settings
    fprintf(hFile,"\n");
    //    fprintf(hFile,"  SetPreferences();\n");
    fprintf(hFile,"  SetOptions();\n");

    // Last line of script executed must be the report format
    // otherwse it may conflict with the default options giving a different format than what the user wants.
    fprintf(hFile, "  gexOptions('output','format','%s');\n",
            mSettings.strOutputReportFormat.toLatin1().constData());

    fprintf(hFile,"}\n\n");
    fclose(hFile);

    // Let s copy into temp for Apps team to be able to retrieve it and reuse it
    QFile::copy(scriptFile, GS::Gex::Engine::GetInstance()
                .Get("TempFolder").toString()+QDir::separator()+"patreport.csl");

    return true;
}

bool PATReportWS::CreateMarkersFile(const QString &markersFile, QString& sampleBins)
{
    FILE * hMarkerFile = fopen(markersFile.toLatin1().constData(), "w");

    if(hMarkerFile == NULL)
    {
        mErrorMessage = "  > Failed to create PAT markers file: " + markersFile;
        return false;
    }

    //  Index to list all limits sets ('near outliers', 'medium outliers', 'far outliers'
    int                 lSeverityLimit;
    QString             lTestName;
    QString             lTestNumber;
    QString             lBin;
    QString             lLabel;
    CPatDefinition *    lPatDef    = NULL;
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;

    for(itPATDefinifion = lPatInfo->GetUnivariateRules().begin();
        itPATDefinifion != lPatInfo->GetUnivariateRules().end(); ++itPATDefinifion)
    {
        lPatDef = *itPATDefinifion;

        // Check if we've reached the end of the list...
        if(lPatDef == NULL)
            break;

        // Normalize test name for script file
        lTestName = lPatDef->m_strTestName;
        lTestName.replace("'","\\'");

        // Check if SPAT bin assigned for this test
        if(lPatDef->m_lFailStaticBin >= 0)
        {
            lBin = "," + QString::number(lPatDef->m_lFailStaticBin) + ",";

            if(sampleBins.contains(lBin) == false)
                sampleBins += lBin;
        }

        // Check if PPAT bin assigned for this test
        if(lPatDef->m_lFailDynamicBin >= 0)
        {
            lBin = "," + QString::number(lPatDef->m_lFailDynamicBin) + ",";

            if(sampleBins.contains(lBin) == false)
                sampleBins += QString::number(lPatDef->m_lFailDynamicBin) + ",";
        }

        // Only generate Histogram lines with failures.
        if(lPatDef->m_TotalFailures_AllParts)
        {
            lTestNumber = QString::number(lPatDef->m_lTestNumber);

            if(lPatDef->mPinIndex >= 0)
            {
                lTestNumber += ".";
                lTestNumber += lPatDef->mPinIndex;
            }

            // Show original test limits (in red) if this test failed PAT
            if(lPatDef->m_lFailDynamicBin >= 0 || lPatDef->m_lFailStaticBin >= 0)
            {
                if(lPatDef->m_lfLowLimit != -GEX_TPAT_DOUBLE_INFINITE)
                {
                    fprintf(hMarkerFile,
                            "  gexParameter('%s','%s','Marker','%g 2 255 0 0','Org. LL.','');\n",
                            lTestNumber.toLatin1().constData(),
                            lTestName.toLatin1().constData(),
                            lPatDef->m_lfLowLimit);
                }
                if(lPatDef->m_lfHighLimit != GEX_TPAT_DOUBLE_INFINITE)
                {
                    fprintf(hMarkerFile,
                            "  gexParameter('%s','%s','Marker','%g 2 255 0 0','Org. HL.','');\n",
                            lTestNumber.toLatin1().constData(),
                            lTestName.toLatin1().constData(),
                            lPatDef->m_lfHighLimit);
                }
            }

            // If Dynamic PAT limits defined, show markers
            if(lPatDef->m_lFailDynamicBin >= 0)
            {
                // Insert markers to show PAT limits (for each site ....).
                for (int lIdx = 0; lIdx < mSites.count(); ++lIdx)
                {
                    GS::PAT::DynamicLimits lDynLimits = lPatDef->GetDynamicLimits(mSites.at(lIdx));

                    // If 3 sets of limits are defined and are identical, this could be a Cpk rule set...
                    if((lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] > -GEX_TPAT_DOUBLE_INFINITE) &&
                       (lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] < GEX_TPAT_DOUBLE_INFINITE) &&
                        (lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] == lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]) &&
                        (lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] == lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]) &&
                        (lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] == lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]) &&
                        (lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] == lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]) &&
                        (lPatInfo->GetRecipeOptions().mSmart_IgnoreHighCpkEnabled && lPatInfo->GetRecipeOptions().lfSmart_IgnoreHighCpk > 0))
                    {
                        // All PAT limits identical and valid: Tells it is a Cpk=XX limit that was used to compute PAT limits.
                        fprintf(hMarkerFile,
                                "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','CpkL=%g','');\n",
                                lTestNumber.toLatin1().constData(),
                                lTestName.toLatin1().constData(),
                                lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],
                                lIdx,
                                lPatInfo->GetRecipeOptions().lfSmart_IgnoreHighCpk);
                        fprintf(hMarkerFile,
                                "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','CpkH=%g','');\n",
                                lTestNumber.toLatin1().constData(),
                                lTestName.toLatin1().constData(),
                                lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],
                                lIdx,
                                lPatInfo->GetRecipeOptions().lfSmart_IgnoreHighCpk);
                    }
                    else
                    {
                        bool	lMarkersSetLow  = false;
                        bool	lMarkersSetHigh = false;

                        // Check if some limits are equal; if so it means limits where relaxed (Smart & adaptive mode)
                        if((lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] > -GEX_TPAT_DOUBLE_INFINITE) &&
                            (lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] == lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]))
                        {
                            if(lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] == lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR])
                            {
                                lLabel = "N,M,F relaxed";
                                fprintf(hMarkerFile,
                                        "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','-%s','');\n",
                                        lTestNumber.toLatin1().constData(),
                                        lTestName.toLatin1().constData(),
                                        lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],
                                        lIdx,
                                        lLabel.toLatin1().constData());

                                lMarkersSetLow = true;
                            }
                            else
                            {
                                lLabel = "N,M relaxed";
                                fprintf(hMarkerFile,
                                        "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','-%s','');\n",
                                        lTestNumber.toLatin1().constData(),
                                        lTestName.toLatin1().constData(),
                                        lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],
                                        lIdx,
                                        lLabel.toLatin1().constData());

                                lLabel = "F";
                                fprintf(hMarkerFile,
                                        "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','-%s','');\n",
                                        lTestNumber.toLatin1().constData(),
                                        lTestName.toLatin1().constData(),
                                        lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR],
                                        lIdx,
                                        lLabel.toLatin1().constData());

                                lMarkersSetLow = true;
                            }
                        }

                        if((lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] < GEX_TPAT_DOUBLE_INFINITE) &&
                            (lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] == lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM]))
                        {
                            if(lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] == lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR])
                            {
                                lLabel = "N,M,F relaxed";
                                fprintf(hMarkerFile,
                                        "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','+%s','');\n",
                                        lTestNumber.toLatin1().constData(),
                                        lTestName.toLatin1().constData(),
                                        lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],
                                        lIdx,
                                        lLabel.toLatin1().constData());

                                lMarkersSetHigh = true;
                            }
                            else
                            {
                                lLabel = "N,M relaxed";
                                fprintf(hMarkerFile,
                                        "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','+%s','');\n",
                                        lTestNumber.toLatin1().constData(),
                                        lTestName.toLatin1().constData(),
                                        lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR],
                                        lIdx,
                                        lLabel.toLatin1().constData());

                                lLabel = "F";
                                fprintf(hMarkerFile,
                                        "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','+%s','');\n",
                                        lTestNumber.toLatin1().constData(),
                                        lTestName.toLatin1().constData(),
                                        lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR],
                                        lIdx,
                                        lLabel.toLatin1().constData());

                                lMarkersSetHigh = true;
                            }
                        }

                        // One side of makers (or both sides) not defined yet.
                        if((lMarkersSetLow == false) || (lMarkersSetHigh == false))
                        {
                            for(lSeverityLimit = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR;
                                lSeverityLimit >= GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
                                --lSeverityLimit)
                            {
                                switch(lSeverityLimit)
                                {
                                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR:
                                        lLabel = "N";
                                        break;
                                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM:
                                        lLabel = "M";
                                        break;
                                    case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR:
                                        lLabel = "F";
                                        break;
                                }

                                // If this distribution type was disabled, do not display its markers!
                                if(lDynLimits.mLowDynamicLimit1[lSeverityLimit] > -GEX_TPAT_DOUBLE_INFINITE &&
                                   lDynLimits.mHighDynamicLimit1[lSeverityLimit] < GEX_TPAT_DOUBLE_INFINITE)
                                {
                                    if(lMarkersSetLow == false)
                                      fprintf(hMarkerFile,
                                              "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','-%s','');\n",
                                              lTestNumber.toLatin1().constData(),
                                              lTestName.toLatin1().constData(),
                                              lDynLimits.mLowDynamicLimit1[lSeverityLimit],
                                              lIdx,
                                              lLabel.toLatin1().constData());

                                    if(lMarkersSetHigh == false)
                                      fprintf(hMarkerFile,
                                              "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','+%s','');\n",
                                              lTestNumber.toLatin1().constData(),
                                              lTestName.toLatin1().constData(),
                                              lDynLimits.mHighDynamicLimit1[lSeverityLimit],
                                              lIdx,
                                              lLabel.toLatin1().constData());

                                    // If two sets of limits for this test (clear bi-modal), then display them
                                    if(lDynLimits.mLowDynamicLimit2[lSeverityLimit] != lDynLimits.mLowDynamicLimit1[lSeverityLimit] &&
                                       lDynLimits.mHighDynamicLimit2[lSeverityLimit] != lDynLimits.mHighDynamicLimit1[lSeverityLimit])
                                    {
                                        fprintf(hMarkerFile,
                                                "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','-%s','');\n",
                                                lTestNumber.toLatin1().constData(),
                                                lTestName.toLatin1().constData(),
                                                lDynLimits.mLowDynamicLimit2[lSeverityLimit],
                                                lIdx,
                                                lLabel.toLatin1().constData());

                                        fprintf(hMarkerFile,
                                                "  gexParameter('%s','%s','Marker','%g 2 255 0 255 %d','+%s','');\n",
                                                lTestNumber.toLatin1().constData(),
                                                lTestName.toLatin1().constData(),
                                                lDynLimits.mHighDynamicLimit2[lSeverityLimit],
                                                lIdx,
                                                lLabel.toLatin1().constData());
                                    }
                                }
                            }
                        }
                    }
                }// Loop for each testing site
            }// DPAT limits

            if(lPatDef->m_lFailStaticBin >= 0)
            {
                // Display SPAT limits (if any)
                if(lPatDef->m_lfLowStaticLimit > -GEX_TPAT_DOUBLE_INFINITE &&
                   lPatDef->m_lfHighStaticLimit < GEX_TPAT_DOUBLE_INFINITE)
                {
                    fprintf(hMarkerFile,
                            "  gexParameter('%s','%s','Marker','%g 2 255 0 255','-SPAT','');\n",
                            lTestNumber.toLatin1().constData(),
                            lTestName.toLatin1().constData(),
                            lPatDef->m_lfLowStaticLimit);
                    fprintf(hMarkerFile,
                            "  gexParameter('%s','%s','Marker','%g 2 255 0 255','+SPAT','');\n",
                            lTestNumber.toLatin1().constData(),
                            lTestName.toLatin1().constData(),
                            lPatDef->m_lfHighStaticLimit);
                }
            }
        }
    }

    fclose(hMarkerFile);

    return true;
}

bool PATReportWS::ExecuteScriptFile(const QString &scriptFile)
{
    // Execute script.
    CSLStatus lStatus = CSLEngine::GetInstance().RunScript(scriptFile);

    // Sets flag to say if we succeeded reading the STDF file (and so can now create it's summary version)
    if(lStatus.IsFailed() == false && gexReport->isCompleted() == true)
    {
        // If report created, next HTML report may not need any HTML page to rebuild...
        if (pGexMainWindow)
        {
//            GSLOG(2, "Core codes uses GexMainWindow : clean me !");
            pGexMainWindow->m_bDatasetChanged	= false;
            pGexMainWindow->iHtmlSectionsToSkip	= GEX_HTMLSECTION_ALL;
        }
        else
        {
            //Reset the one ReportOptions
//            GSLOG(3, "PATReport::ExecuteScriptFile: reset iHtmlSectionsToSkip ? clean me !")
        }

        CGexGroupOfFiles *  lGroup  = gexReport->getGroupsList().isEmpty() ?
                                          NULL : gexReport->getGroupsList().first();

        if(lGroup == NULL)
        {
            GSLOG(SYSLOG_SEV_ERROR, "*PAT Error* No datasets created");
            return false;
        }

        CGexFileInGroup *   lFile   = (lGroup->pFilesList.isEmpty()) ?
                                          NULL : lGroup->pFilesList.first();

        if(lFile == NULL)
        {
            GSLOG(SYSLOG_SEV_ERROR, "*PAT Error* No datasets created");
            return false;
        }

        QString lString = "  SetupTime," +
                          QString((lFile->getMirDatas().lSetupT > 0) ?
                                      TimeStringUTC(lFile->getMirDatas().lSetupT): "n/a.\n");
        GSLOG(SYSLOG_SEV_DEBUG, lString.toLatin1().constData());

        lString = "  StartTime," +
                  QString((lFile->getMirDatas().lStartT > 0) ?
                              TimeStringUTC(lFile->getMirDatas().lStartT): "n/a.\n");
        GSLOG(SYSLOG_SEV_DEBUG, lString.toLatin1().constData());

        lString = "  EndTime," +
                  QString((lFile->getMirDatas().lEndT > 0) ?
                              TimeStringUTC(lFile->getMirDatas().lEndT): "n/a.\n");
        GSLOG(SYSLOG_SEV_DEBUG, lString.toLatin1().constData());

        lString = "  Product," + QString(lFile->getMirDatas().szPartType);
        GSLOG(SYSLOG_SEV_DEBUG, lString.toLatin1().constData());

        lString = "  DesignRev," + QString(lFile->getMirDatas().szDesignRev);
        GSLOG(SYSLOG_SEV_DEBUG, lString.toLatin1().constData());

        lString = "  Program," + QString(lFile->getMirDatas().szJobName);
        GSLOG(SYSLOG_SEV_DEBUG, lString.toLatin1().constData());

        lString = "  ProgramRev," + QString(lFile->getMirDatas().szJobRev);
        GSLOG(SYSLOG_SEV_DEBUG, lString.toLatin1().constData());

        lString = "  Lot," + QString(lFile->getMirDatas().szLot);
        GSLOG(SYSLOG_SEV_DEBUG, lString.toLatin1().constData());

        lString = "  SubLot," + QString(lFile->getMirDatas().szSubLot);
        GSLOG(SYSLOG_SEV_DEBUG, lString.toLatin1().constData());

        lString = "  WaferID," + QString(lFile->getWaferMapData().szWaferID);
        GSLOG(SYSLOG_SEV_DEBUG, lString.toLatin1().constData());

        return true;
    }
    else
    {
        mErrorMessage = "executing script failed : " + scriptFile;
        return false;
    }
}

bool PATReportWS::WriteOutlierFailure(FILE *handle, QString &sampleBins)
{
    if (handle == NULL)
    {
        mErrorMessage = "  > No script file opened to write outlier failures";
        return false;
    }

    // Outlier failures (PAT bins, default is 0, but user can customize to have multiple Bins....)
    tdPATBins::ConstIterator itBin;
    QString lBins;

    // Find all Static PAT colors
    for ( itBin = lPatInfo->GetPATSoftBins().begin(); itBin != lPatInfo->GetPATSoftBins().end(); ++itBin )
    {
        if((*itBin).bFailType & GEX_TPAT_BINTYPE_STATICFAIL)
            lBins += QString::number((*itBin).iBin) + ",";
    }
    if(lBins.isEmpty() == false)
    {
        fprintf(handle,"// SPAT bins\n");
        fprintf(handle,"  gexBinStyle('bin_list','%s');\n",lBins.toLatin1().constData());
        fprintf(handle,"  gexBinStyle('bin_color','%d %d %d');\n",lPatInfo->GetRecipeOptions().cStaticFailColor.red(),lPatInfo->GetRecipeOptions().cStaticFailColor.green(),lPatInfo->GetRecipeOptions().cStaticFailColor.blue());
        fprintf(handle,"  gexBinStyle('add_bin','soft_bin');\n");
        fprintf(handle,"  gexBinStyle('add_bin','hard_bin');\n");

        // Keep track of PAT bins enabled
        sampleBins += "," + lBins;
    }

    // Find all Dynamic PAT colors
    lBins.clear();
    for ( itBin = lPatInfo->GetPATSoftBins().begin(); itBin != lPatInfo->GetPATSoftBins().end(); ++itBin )
    {
        if((*itBin).bFailType & GEX_TPAT_BINTYPE_DYNAMICFAIL)
            lBins += QString::number((*itBin).iBin) + ",";
    }
    if(lBins.isEmpty() == false)
    {
        fprintf(handle,"// DPAT bins\n");
        fprintf(handle,"  gexBinStyle('bin_list','%s');\n",lBins.toLatin1().constData());
        fprintf(handle,"  gexBinStyle('bin_color','%d %d %d');\n",lPatInfo->GetRecipeOptions().cDynamicFailColor.red(),lPatInfo->GetRecipeOptions().cDynamicFailColor.green(),lPatInfo->GetRecipeOptions().cDynamicFailColor.blue());
        fprintf(handle,"  gexBinStyle('add_bin','soft_bin');\n");
        fprintf(handle,"  gexBinStyle('add_bin','hard_bin');\n");

        // Keep track of PAT bins enabled
        sampleBins += "," + lBins;
    }

    // NNR color
    if(lPatInfo->GetRecipeOptions().IsNNREnabled())
    {
        fprintf(handle,"// NNR bins\n");

        QList<CNNR_Rule>::iterator itNNRBegin = lPatInfo->GetRecipeOptions().GetNNRRules().begin();
        QList<CNNR_Rule>::iterator itNNREnd   = lPatInfo->GetRecipeOptions().GetNNRRules().end();
        for (; itNNRBegin != itNNREnd; ++itNNRBegin)
        {
            fprintf(handle,"  gexBinStyle('bin_list','%s');\n",
                    QString::number((*itNNRBegin).GetSoftBin()).toLatin1().constData());
            fprintf(handle,"  gexBinStyle('bin_color','%d %d %d');\n",
                    (*itNNRBegin).GetFailBinColor().red(),
                    (*itNNRBegin).GetFailBinColor().green(),
                    (*itNNRBegin).GetFailBinColor().blue());
            fprintf(handle,"  gexBinStyle('add_bin','soft_bin');\n");
            fprintf(handle,"  gexBinStyle('add_bin','hard_bin');\n");

            // Keep track of PAT bins enabled
            sampleBins += "," + QString::number((*itNNRBegin).GetSoftBin());
        }
    }

    // IDDQ-Delta color
    if(lPatInfo->GetRecipeOptions().mIsIDDQ_Delta_enabled)
    {
        fprintf(handle,"// IDDQ-Delta bins\n");

        QList<CIDDQ_Delta_Rule>::iterator itIDDQBegin = lPatInfo->GetRecipeOptions().mIDDQ_Delta_Rules.begin();
        QList<CIDDQ_Delta_Rule>::iterator itIDDQEnd   = lPatInfo->GetRecipeOptions().mIDDQ_Delta_Rules.end();
        for (; itIDDQBegin != itIDDQEnd; ++itIDDQBegin)
        {
            fprintf(handle,"  gexBinStyle('bin_list','%s');\n",
                    QString::number((*itIDDQBegin).GetSoftBin()).toLatin1().constData());
            fprintf(handle,"  gexBinStyle('bin_color','%d %d %d');\n",
                    (*itIDDQBegin).GetFailBinColor().red(),
                    (*itIDDQBegin).GetFailBinColor().green(),
                    (*itIDDQBegin).GetFailBinColor().blue());
            fprintf(handle,"  gexBinStyle('add_bin','soft_bin');\n");
            fprintf(handle,"  gexBinStyle('add_bin','hard_bin');\n");

            // Keep track of PAT bins enabled
            sampleBins += "," + QString::number((*itIDDQBegin).GetSoftBin());
        }
    }

    // GDBN color
    if((lPatInfo->GetRecipeOptions().mIsGDBNEnabled == true) ||
       (lPatInfo->GetRecipeOptions().bCustomPatLib && lPatInfo->GetRecipeOptions().mGDBNCustomLib))
    {
        fprintf(handle,"// GDBN bins\n");

        QList<CGDBN_Rule>::iterator itGDBNBegin = lPatInfo->GetRecipeOptions().mGDBNRules.begin();
        QList<CGDBN_Rule>::iterator itGDBNEnd   = lPatInfo->GetRecipeOptions().mGDBNRules.end();
        for (; itGDBNBegin != itGDBNEnd; ++itGDBNBegin)
        {
            fprintf(handle,"  gexBinStyle('bin_list','%s');\n",
                    QString::number((*itGDBNBegin).mSoftBin).toLatin1().constData());
            fprintf(handle,"  gexBinStyle('bin_color','%d %d %d');\n",
                    (*itGDBNBegin).mFailBinColor.red(),
                    (*itGDBNBegin).mFailBinColor.green(),
                    (*itGDBNBegin).mFailBinColor.blue());
            fprintf(handle,"  gexBinStyle('add_bin','soft_bin');\n");
            fprintf(handle,"  gexBinStyle('add_bin','hard_bin');\n");

            // Keep track of PAT bins enabled
            sampleBins += "," + QString::number((*itGDBNBegin).mSoftBin);
        }

    }

    // Reticle color
    if(lPatInfo->GetRecipeOptions().GetReticleEnabled())
    {
        fprintf(handle,"// Reticle bins\n");

        QList<PATOptionReticle>::iterator itReticleBegin = lPatInfo->GetRecipeOptions().GetReticleRules().begin();
        QList<PATOptionReticle>::iterator itReticleEnd   = lPatInfo->GetRecipeOptions().GetReticleRules().end();
        for (; itReticleBegin != itReticleEnd; ++itReticleBegin)
        {
            fprintf(handle,"  gexBinStyle('bin_list','%s');\n",
                    QString::number((*itReticleBegin).GetReticleSBin()).toLatin1().constData());
            fprintf(handle,"  gexBinStyle('bin_color','%d %d %d');\n",
                    (*itReticleBegin).GetReticleColor().red(),
                    (*itReticleBegin).GetReticleColor().green(),
                    (*itReticleBegin).GetReticleColor().blue());
            fprintf(handle,"  gexBinStyle('add_bin','soft_bin');\n");
            fprintf(handle,"  gexBinStyle('add_bin','hard_bin');\n");

            // Keep track of PAT bins enabled
            sampleBins += "," + QString::number((*itReticleBegin).GetReticleSBin());
        }
    }

    // Clustering Potato color
    if(lPatInfo->GetRecipeOptions().mClusteringPotato)
    {
        fprintf(handle,"// Clustering bins\n");

        QList<CClusterPotatoRule>::iterator itClusteringBegin = lPatInfo->GetRecipeOptions().mClusterPotatoRules.begin();
        QList<CClusterPotatoRule>::iterator itClusteringEnd   = lPatInfo->GetRecipeOptions().mClusterPotatoRules.end();
        for (; itClusteringBegin != itClusteringEnd; ++itClusteringBegin)
        {
            fprintf(handle,"  gexBinStyle('bin_list','%s');\n",
                    QString::number((*itClusteringBegin).mSoftBin).toLatin1().constData());
            fprintf(handle,"  gexBinStyle('bin_color','%d %d %d');\n",
                    (*itClusteringBegin).mFailBinColor.red(),
                    (*itClusteringBegin).mFailBinColor.green(),
                    (*itClusteringBegin).mFailBinColor.blue());
            fprintf(handle,"  gexBinStyle('add_bin','soft_bin');\n");
            fprintf(handle,"  gexBinStyle('add_bin','hard_bin');\n");

            // Keep track of PAT bins enabled
            sampleBins += "," + QString::number((*itClusteringBegin).mSoftBin);
        }
    }

    // Z-PAT Composite Map color
    if(lPatInfo->GetRecipeOptions().GetExclusionZoneEnabled() && lPatInfo->GetRecipeOptions().lfCompositeExclusionZoneYieldThreshold > 0)
    {
        fprintf(handle,"// Z-PAT bins\n");
        fprintf(handle,"  gexBinStyle('bin_list','%s');\n",QString::number(lPatInfo->GetRecipeOptions().iCompositeZone_SBin).toLatin1().constData());
        fprintf(handle,"  gexBinStyle('bin_color','%d %d %d');\n",lPatInfo->GetRecipeOptions().cZpatColor.red(),lPatInfo->GetRecipeOptions().cZpatColor.green(),lPatInfo->GetRecipeOptions().cZpatColor.blue());
        fprintf(handle,"  gexBinStyle('add_bin','soft_bin');\n");
        fprintf(handle,"  gexBinStyle('add_bin','hard_bin');\n");

        // Keep track of PAT bins enabled
        sampleBins += "," + QString::number(lPatInfo->GetRecipeOptions().iCompositeZone_SBin);
    }

    // MV PAT color
    if(lPatInfo->GetRecipeOptions().GetMVPATEnabled())
    {
        fprintf(handle,"// MV PAT bins\n");

        lBins.clear();

        QList<GS::Gex::PATMultiVariateRule>::iterator itMVPatBegin = lPatInfo->GetMultiVariateRules().begin();
        QList<GS::Gex::PATMultiVariateRule>::iterator itMVPatEnd  = lPatInfo->GetMultiVariateRules().end();
        for (; itMVPatBegin != itMVPatEnd; ++itMVPatBegin)
        {
                lBins += QString::number((*itMVPatBegin).GetBin()) + ",";
        }

        if (lBins.isEmpty() == false)
        {
            fprintf(handle,"  gexBinStyle('bin_list','%s');\n", lBins.toLatin1().constData());
            fprintf(handle,"  gexBinStyle('bin_color','%d %d %d');\n",
                    lPatInfo->GetRecipeOptions().GetMVPATColor().red(),
                    lPatInfo->GetRecipeOptions().GetMVPATColor().green(),
                    lPatInfo->GetRecipeOptions().GetMVPATColor().blue());
            fprintf(handle,"  gexBinStyle('add_bin','soft_bin');\n");
            fprintf(handle,"  gexBinStyle('add_bin','hard_bin');\n");

            // Keep track of PAT bins enabled
            sampleBins += "," + lBins;
        }
    }

    return true;
}

bool PATReportWS::WriteMapAxisDirection(FILE *handle)
{
    if (handle == NULL)
    {
        mErrorMessage = "  > No script file opened to write map axis direction";
        return false;
    }

    // 1) If PAT gtf, use settings defined in GTF, auto if none defined (do not use local options)
    // 2) If manual PAT-processing, or file analysis, use local options
    // TODO: handle ePositiveRight/ePositiveForceRight and ePositiveLeft/ePositiveForceLeft differently, but
    //       for now, no corresponding CSL option available
    if(!mSettings.strTriggerFile.isEmpty())
    {
        switch(mSettings.mStdfXDirection)
        {
            case PATProcessing::ePositiveLeft:
            case PATProcessing::ePositiveForceLeft:
                // Use 'left' mode for positive X direction
                fprintf(handle,"  gexOptions('wafer','positive_x','left');\n");
                break;

            case PATProcessing::ePositiveRight:
            case PATProcessing::ePositiveForceRight:
                // Use 'right' mode for positive X direction
                fprintf(handle,"  gexOptions('wafer','positive_x','right');\n");
                break;

            case PATProcessing::eDefault:
            default:
                // Use 'auto' mode for positive X direction
                fprintf(handle,"  gexOptions('wafer','positive_x','auto');\n");
                break;

        }

        switch(mSettings.mStdfYDirection)
        {
            case PATProcessing::ePositiveUp:
            case PATProcessing::ePositiveForceUp:
                // Use 'up' mode for positive Y direction
                fprintf(handle,"  gexOptions('wafer','positive_y','up');\n");
                break;

            case PATProcessing::ePositiveDown:
            case PATProcessing::ePositiveForceDown:
                // Use 'down' mode for positive Y direction
                fprintf(handle,"  gexOptions('wafer','positive_y','down');\n");
                break;

            case PATProcessing::eDefault:
            default:
                // Use 'auto' mode for positive Y direction
                fprintf(handle,"  gexOptions('wafer','positive_y','auto');\n");
                break;
        }
    }

    return true;
}

QString PATReportWS::GetOutliersTestList() const
{
    QList<CPatFailingTest>::iterator    itPart;
    CPatFailingTest                     lFailTest;
    QStringList                         lTestList;
    tdIterPatOutlierParts               itOutlierParts(lPatInfo->m_lstOutlierParts);
    CPatOutlierPart *                   lOutlierPart = NULL;
    QString                             lTest;

    while(itOutlierParts.hasNext())
    {
        lOutlierPart = itOutlierParts.next();

        for (itPart = lOutlierPart->cOutlierList.begin(); itPart != lOutlierPart->cOutlierList.end(); ++itPart )
        {
            // Get handle to faulty test.
            lFailTest = *itPart;

            // Keep track of failing tests
            lTest = QString::number(lFailTest.mTestNumber);

            if (lFailTest.mPinIndex >= 0)
                lTest += "." + QString::number(lFailTest.mPinIndex);

            if (lTestList.indexOf(lTest) == -1)
                lTestList.append(lTest);
        }
    };

    return lTestList.join(",");
}

}   // namespace Gex
}   // namespace GS
#endif
