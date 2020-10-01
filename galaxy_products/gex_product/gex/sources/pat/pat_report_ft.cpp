#include "pat_report_ft.h"
#include "pat_recipe_io.h"
#include "engine.h"
#include "csl/csl_engine.h"
#include "gqtl_log.h"
#include "gqtl_sysutils.h"
#include "gex_report.h"
#include "pat_info.h"
#include "pat_definition.h"
#include "stdf_content_utils.h"

extern CGexReport *     gexReport;
extern CReportOptions   ReportOptions;

extern QString          ConvertToScriptString(const QString &strFile);

namespace GS
{
namespace Gex
{

PATReportFT::PATReportFT()
//    : mOptionsPAT(NULL)
{
}

PATReportFT::~PATReportFT()
{
}

bool PATReportFT::Generate(const QStringList &outputFiles, const QString &traceabilityFile, const QString &lRecipe)
{
    if (lRecipe.isEmpty())
    {
        mErrorMessage = "Recipe file name is empty";
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }
;
    QSharedPointer<PATRecipeIO> lPatRecipeIO(PATRecipeIO::CreateRecipeIo(lRecipe));

    if (lPatRecipeIO.isNull())
    {
        mErrorMessage = "Unable to instanciate Recipe reader";
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    lPatRecipeIO->SetRecipeName(lRecipe);

    if (lPatRecipeIO->Read(mRecipe) == false)
    {
        mErrorMessage = "Failed to read recipe file " + lRecipe;

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return false;
    }

    // Split file over each site
    QString lSTDFCompliancy = ReportOptions.GetOption("dataprocessing", "stdf_compliancy").toString();
    bool    lValidSiteOnly  = false;

    if (lSTDFCompliancy == "stringent")
        lValidSiteOnly = true;

    std::vector<int> lSite;
    if (outputFiles.count() > 0 &&
        GS::Gex::StdfContentUtils::GetSites((std::string)(outputFiles.first().toLatin1().constData()),
                                            lSite, lValidSiteOnly) == false)
    {
        mErrorMessage = "Error : Cannot extract site from input file";
        return false;
    }
    mSites = QList<int>::fromVector( QVector<int>::fromStdVector(lSite));

    mOutputFiles        = outputFiles;
    mTraceabilityFile   = traceabilityFile;

    if (CreateScriptFile(Engine::GetInstance().GetAssistantScript()) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to create FT PAT report script file");
        return false;
    }

    if (ExecuteScriptFile(Engine::GetInstance().GetAssistantScript()) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to execute FT PAT report script file");
        return false;
    }

    return true;
}

const QString &PATReportFT::GetErrorMessage() const
{
    return mErrorMessage;
}

void PATReportFT::SetReportFormat(const QString &lReportFormat)
{
    mReportFormat = lReportFormat;
}

bool PATReportFT::CreateScriptFile(const QString &scriptFile)
{
    FILE * hFile = fopen(scriptFile.toLatin1().constData(),"w");

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
    // Check if single-site or multi-sites...
    if(mSites.count() > 1)
    {
        for (QList<int>::const_iterator iter = mSites.begin(); iter != mSites.end(); ++iter)
        {
            fprintf(hFile, "  group_id = gexGroup('insert','Site %d');\n", *iter);

            for (int lIdx = 0; lIdx < mOutputFiles.count(); ++lIdx)
            {
                fprintf(hFile,
                        "  gexFile(group_id,'insert','%s','%d','last_instance',' ','','',' ');\n\n",
                        ConvertToScriptString(mOutputFiles.at(lIdx)).toLatin1().constData(),
                        *iter);
            }
        }
    }
    else
    {
        fprintf(hFile, "  group_id = gexGroup('insert','DataSet_1');\n");

        for (int lIdx = 0; lIdx < mOutputFiles.count(); ++lIdx)
        {
            fprintf(hFile,
                    "  gexFile(group_id,'insert','%s','all','last_instance',' ','','',' ');\n\n",
                    ConvertToScriptString(mOutputFiles.at(lIdx)).toLatin1().constData());
        }
    }

    fprintf(hFile, "}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n\n");

    // Report Output format
    fprintf(hFile,
            "  gexOptions('output','format','%s');\n",
            mReportFormat.toLatin1().constData());

    // If no PAT limits clamping
    // Disable Examinator outlier GUI filter!
    fprintf(hFile,"  gexOptions('dataprocessing','data_cleaning_mode','none');\n");

    // Forces to ALWAYS compute advanced statistics.
    fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");

    // Disables PartID support (uses less memory this way,
    // and not needed in reports created)
    fprintf(hFile,"  gexOptions('dataprocessing','part_id','show');\n");

    // Force to compute statistics from samples, ignore summary.
    fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");
    fprintf(hFile,"  gexOptions('pareto','section','');\n");

    // Force computing binning from wafermap (so to avoid counting retest multiple times.
    fprintf(hFile,"  gexOptions('binning','computation','samples');\n");

    // Disable STDF.FTR processing (for PAT speed improvement
    // since FTR are not used in PAT algorihtms)
    fprintf(hFile,"  gexOptions('dataprocessing','functional_tests','disabled');\n");
    // Force Part identification to use part_id coordinates
    fprintf(hFile,"  gexOptions('dataprocessing', 'part_identification', 'part_id');\n");
    // Force data file order to be done based on the date
    fprintf(hFile,"  gexOptions('dataprocessing', 'sorting', 'date');\n");
    // Force to split MPR
    fprintf(hFile,"  gexOptions('dataprocessing','multi_parametric_merge_mode','no_merge');\n");

    // Choose test merge option based on recipe test key option
    switch (mRecipe.GetOptions().mTestKey)
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

    // Force Trend chart to be on run_id instead of part_id
    fprintf(hFile,"  gexOptions('adv_trend','x_axis','run_id');\n");

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
            mRecipe.GetOptions().pGoodSoftBinsList->GetRangeList().toLatin1().constData());
    fprintf(hFile,"  gexBinStyle('bin_color','0 128 0');\n");
    fprintf(hFile,"  gexBinStyle('add_bin','soft_bin');\n");

    // Good Hard BINs (Pass dies)
    fprintf(hFile,"// Good HBIN bins\n");
    fprintf(hFile,
            "  gexBinStyle('bin_list','%s');\n",
            mRecipe.GetOptions().pGoodHardBinsList->GetRangeList().toLatin1().constData());
    fprintf(hFile,"  gexBinStyle('bin_color','0 128 0');\n");
    fprintf(hFile,"  gexBinStyle('add_bin','hard_bin');\n");

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
    if(mRecipe.GetOptions().iReport_WafermapType == 0)
        fprintf(hFile,"  gexReportType('wafer','soft_bin');\n");
    else
        fprintf(hFile,"  gexReportType('wafer','hard_bin');\n");

    // Binning
    if(mRecipe.GetOptions().bReport_Binning)
        fprintf(hFile,"  gexOptions('binning','section','enabled');\n");	// Binning enabled
    else
        fprintf(hFile,"  gexOptions('binning','section','disabled');\n");	// Disabled binning

    // Wafermap
    if(mRecipe.GetOptions().bReport_Wafermap)
        fprintf(hFile,"  gexOptions('wafer','chart_show','all_individual');\n");	// Ensure wafermap is visible.
    else
        fprintf(hFile,"  gexOptions('wafer','chart_show','');\n");	// Disabled wafermap

    // Disable MyReports in case it is fromp a previous user session!
    fprintf(hFile,"  gexReportType('adv_my_report','disabled');\n");

    // 'More reports' section will hold the outlier removal summary table.
    if (mTraceabilityFile.isEmpty() == false)
    {
        QString lTraceabilityDataFile = ConvertToScriptString(mTraceabilityFile);
        fprintf(hFile,
                "  gexReportType('adv_pat','traceability_file', '%s');\n",
                lTraceabilityDataFile.toLatin1().constData());
    }

    // Get report built...and launch Viewer (unless disabled)
//    if(mShow)
        fprintf(hFile,"  gexBuildReport('home','0');\n");	// Show report home page
//    else
//        fprintf(hFile,"  gexBuildReport('hide','0');\n");	// Do not show report page!

    fprintf(hFile,"}\n\n");
    fclose(hFile);

    return true;
}

bool PATReportFT::ExecuteScriptFile(const QString &scriptFile)
{
    // Execute script.
    CSLStatus lStatus = CSLEngine::GetInstance().RunScript(scriptFile);

    // Sets flag to say if we succeeded reading the STDF file (and so can now create it's summary version)
    if(lStatus.IsFailed() == false && gexReport->isCompleted() == true)
    {
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

        return true;
    }
    else
    {
        mErrorMessage = "executing script failed : " + scriptFile;
        return false;
    }
}

}   // namespace Gex
}   // namespace GS
