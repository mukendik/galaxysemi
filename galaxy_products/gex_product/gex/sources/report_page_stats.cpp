/////////////////////////////////////////////////////////////////////////////
// Creates HTML Test Statistics page.
/////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <QList>

#include "browser_dialog.h"
#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "cbinning.h"
#include "gex_constants.h"			// Constants shared in modules
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "report_classes_sorting.h"	// Classes to sort lists.
#include "interactive_charts.h"		// Layer classes, etc
#include "patman_lib.h"				// List of '#define' to return type of distribution (gaussian, lognormal, etc...)
#include "cstats.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "csl/csl_engine.h"
//#include "pat_info.h"
#include "pat_engine.h"
#include "gqtl_global.h"
#include "multi_limit_item.h"

// Galaxy QT libraries
#include "gqtl_sysutils.h"

#include "max_shift_calculator.h"

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

// cstats.cpp
extern double			ScalingPower(int iPower);

/////////////////////////////////////////////////////////////////////////////
// Build Test limits strings ( use ',' separator between value & units if CSV output)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::BuildAllTestLimitsStrings(void)
{
    CTest *								ptTestCell	= NULL;
    CGexGroupOfFiles *					pGroup		= NULL;
    CGexFileInGroup *					pFile		= NULL;
    QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    while(itGroupsList.hasNext())
    {
        pGroup	= itGroupsList.next();
        pFile   = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        // List of tests in this group.
        ptTestCell = pGroup->cMergedData.ptMergedTestList;

        pFile->updateOutputFormat(strOutputFormat);

        while(ptTestCell != NULL)
        {
            // If Low limit exists
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                pFile->FormatTestLimit(ptTestCell,
                                       ptTestCell->GetCurrentLimitItem()->szLowL,
                                       ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                                       ptTestCell->llm_scal);
            else
            {
                if (strOutputFormat=="CSV")
                    strcpy(ptTestCell->GetCurrentLimitItem()->szLowL,"n/a,n/a"); // No limits, no units (2 fields in CSV format)
                else
                if (m_pReportOptions->isReportOutputHtmlBased())
                    strcpy(ptTestCell->GetCurrentLimitItem()->szLowL,GEX_NA);
            }

            // If High limit exists
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                pFile->FormatTestLimit(ptTestCell,
                                       ptTestCell->GetCurrentLimitItem()->szHighL,
                                       ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                                       ptTestCell->hlm_scal);
            else
            {
                if (strOutputFormat=="CSV")
                    strcpy(ptTestCell->GetCurrentLimitItem()->szHighL,"n/a,n/a");
                else
                if (m_pReportOptions->isReportOutputHtmlBased())
                    strcpy(ptTestCell->GetCurrentLimitItem()->szHighL,GEX_NA);
            }

            // If Low Spec limit exists
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
                pFile->FormatTestLimit(ptTestCell,ptTestCell->szLowSpecL,ptTestCell->lfLowSpecLimit,ptTestCell->llm_scal);
            else
            {
                if (strOutputFormat=="CSV")
                    strcpy(ptTestCell->szLowSpecL, "n/a,n/a"); // No limits, no units (2 fields in CSV format)
                else
                if (m_pReportOptions->isReportOutputHtmlBased())
                    strcpy(ptTestCell->szLowSpecL,GEX_NA);
            }

            // If High Spec limit exists
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
                pFile->FormatTestLimit(ptTestCell,ptTestCell->szHighSpecL,ptTestCell->lfHighSpecLimit,ptTestCell->hlm_scal);
            else
            {
                if (strOutputFormat=="CSV")
                    strcpy(ptTestCell->szHighSpecL,"n/a,n/a");
                else
                if(m_pReportOptions->isReportOutputHtmlBased())
                    strcpy(ptTestCell->szHighSpecL,GEX_NA);
            }

            // Next cell
            ptTestCell = ptTestCell->GetNextTest();
        };
    };
}

/////////////////////////////////////////////////////////////////////////////
// Compute statistics based on all data collected
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::ComputeDataStatistics(bool computeShape, bool bFilter/*=false*/)
{
    // Get pointer to first group & first file (we always have them exist)
    CTest *ptTestCell=0;
    // Total parts (number can come from either Summary or samples count)
    long	lTotalOutliers;
    int		iCurrentGroup=1;
    long	lTotalTests=0;
    long	lTotalGood=0;
    float	lfValue;
    bool	bForceStatsComputation = false;

    // Ensure we use latest options set
    m_cStats.UpdateOptions(m_pReportOptions);

    // Status bar message.
    QString strProgressString = " Computing Statistics...";
    QString	strString;
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(strProgressString);
    QString orm=m_pReportOptions->GetOption("dataprocessing", "data_cleaning_mode").toString();
    double orf=m_pReportOptions->GetOption("dataprocessing", "data_cleaning_value").toDouble();

    QString strOptionStorageDevice = (m_pReportOptions->GetOption("speed","adv_stats")).toString();

    if(strOptionStorageDevice == "always")
    {
        m_pReportOptions->mComputeAdvancedTest = true;		// Always collect samples for 2D/3D Interactive Charting
    }
    else if(strOptionStorageDevice == "50mb")
    {
        if(lfTotalFileSize <= 52e6)
            m_pReportOptions->mComputeAdvancedTest = true;	// Compute Adv. stats. if 50MB of data or less.
    }
    else if(strOptionStorageDevice == "100mb")
    {
        if(lfTotalFileSize <= 1e8)
            m_pReportOptions->mComputeAdvancedTest = true;	// Compute Adv. stats. if 100MB of data or less.
    }
    else if(strOptionStorageDevice == "200mb")
    {
        if(lfTotalFileSize <= 2e8)
            m_pReportOptions->mComputeAdvancedTest = true;	// Compute Adv. stats. if 200MB of data or less.
    }
    else if(strOptionStorageDevice == "300mb")
    {
        if(lfTotalFileSize <= 3.1e8)
            m_pReportOptions->mComputeAdvancedTest = true;	// Compute Adv. stats. if 300MB of data or less.
    }
    else if(strOptionStorageDevice == "400mb")
    {
        if(lfTotalFileSize <= 3.1e8)
            m_pReportOptions->mComputeAdvancedTest = true;	// Compute Adv. stats. if 300MB of data or less.
    }
    else if(strOptionStorageDevice == "500mb")
    {
        if(lfTotalFileSize <= 5.2e8)
            m_pReportOptions->mComputeAdvancedTest = true;	// Compute Adv. stats. if 500MB of data or less.
    }
    else if(strOptionStorageDevice == "never")
    {
        m_pReportOptions->mComputeAdvancedTest = false;
    }
    else
    {
        GEX_ASSERT(false);
    }


    // Although Examinator-OEM doesn't allow advanced statistics, we need them for some chart scales!
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        m_pReportOptions->mComputeAdvancedTest=true;
    }

    // Flag if need to Data cleaning based on IQR rule (Q1,Q3 +/- N*IQR)
    bool	bIqrDataCleaning = (orm == "n_iqr");

    // Force to compute statistics if some flags are set.

    {
        QString strOptionStorageDevice = (m_pReportOptions->GetOption("statistics","computation")).toString();
        if((strOptionStorageDevice == "samples_only") || (strOptionStorageDevice == "samples_then_summary"))
            bForceStatsComputation = true;
    }

    QString scaling=(m_pReportOptions->GetOption("dataprocessing","scaling")).toString();
    //FIXME: not used ?
    //bool isScalingNormalized=(scaling=="normalized")?true:false;

    QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());
    CGexGroupOfFiles *					pGroup	= NULL;
    CGexFileInGroup *					pFile	= NULL;

    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    // Build stats for each parameter
    while(itGroupsList.hasNext())
    {
        // List of tests in this group.
        pGroup		= itGroupsList.next();
        if (!pGroup->pFilesList.isEmpty())
        {
            pFile		= pGroup->pFilesList.at(0);
            ptTestCell	= pGroup->cMergedData.ptMergedTestList;

            // If any outlier found (or filter active on parts)...
            // then statistics must be computed from samples, not Summary!
            if(pGroup->cMergedData.bPartsFiltered == true)
                lTotalOutliers = 1;	// No a real outlier...but flags that statistics must be computed from samples!
            else
                lTotalOutliers = 0;

            while(ptTestCell != NULL)
            {
                // Create test name string: include PinmapIndex# if exists
                BuildTestNumberString(ptTestCell);

                // If no test name defined, force one!
                if(ptTestCell->strTestName.isEmpty() == true)
                    ptTestCell->strTestName = "Test "+ QString::number(ptTestCell->lTestNumber);

                // Keep track of total tests computed (used to update GUI every 8 tests!)
                lTotalTests++;
                if((lTotalTests & 07) == 0)
                {
                    // Display next test# to process
                    strString = strProgressString + "Test#"+ QString::number(ptTestCell->lTestNumber);
                    GS::Gex::Engine::GetInstance().UpdateLabelStatus(strString);
                }

                // Update outliers count
                lTotalOutliers += ptTestCell->GetCurrentLimitItem()->ldOutliers;

                // Default execs count
                //FIXME: not used ?
                //ldTotalExecs = ptTestCell->ldExecs;

                // If Exec count = 0, maybe TSR was missing !...then check if PTR values
                // Also, if this report results from filtering then display the number
                // of samples processed rather than the TSR data.
                if(	(ptTestCell->ldExecs == 0) || (lTotalOutliers) || (bForceStatsComputation))
                {
                    // If we found samples / or filtered data...overwrite summary info with Samples info.
                    if(ptTestCell->ldSamplesValidExecs)
                    {
                        ptTestCell->bStatsFromSamples = true;

                        // If Filtering over some parts is enabled, then force computing Min/Max from data samples
                        if(ptTestCell->m_testResult.count() > 0 &&
                            (bForceStatsComputation || lTotalOutliers || bFilter || pGroup->cMergedData.bPartsFiltered)
                          )
                        {
                            m_cStats.ComputeLowLevelTestStatistics(ptTestCell,ScalingPower(ptTestCell->res_scal));
                        }
                        else
                        {
                            //FIXME: not used ?
                            //ldTotalExecs =
                            ptTestCell->ldExecs = ptTestCell->ldSamplesValidExecs;
                            ptTestCell->lfMin = ptTestCell->lfSamplesMin;
                            ptTestCell->lfMax = ptTestCell->lfSamplesMax;
                            ptTestCell->lfTotal = ptTestCell->lfSamplesTotal;
                            ptTestCell->lfTotalSquare = ptTestCell->lfSamplesTotalSquare;
                            ptTestCell->GetCurrentLimitItem()->ldFailCount = ptTestCell->GetCurrentLimitItem()->ldSampleFails;
                        }
                    }
                }

#if 1
                // Following block may be leading to possibly confusing results, where statistics based
                // on total and total square (mean, sigma...) are from the samples, but the nb of executions
                // and nb of fails are from the summary (if any)
                if(	(ptTestCell->lfTotal == -C_INFINITE) ||
                    (ptTestCell->lfTotalSquare == -C_INFINITE))
                {
                    if(ptTestCell->ldSamplesValidExecs)
                    {
                        // Uncommenting following line will switch the test all together to use samples only
                        // (even nb of executions and nb of fails)
                        // ptTestCell->bStatsFromSamples = true;

                        ptTestCell->lfTotal = ptTestCell->lfSamplesTotal;
                        ptTestCell->lfTotalSquare = ptTestCell->lfSamplesTotalSquare;

                        //FIXME: not used ?
                        //ldTotalExecs = ptTestCell->ldSamplesValidExecs;
                        if(ptTestCell->ldExecs <= 0)
                            ptTestCell->ldExecs = ptTestCell->ldSamplesValidExecs;
                        // If summary run count doesn't match samples count, adjust Total and Total square accordingly!
                        if(ptTestCell->ldExecs && (ptTestCell->ldExecs != ptTestCell->ldSamplesValidExecs))
                        {
                            ptTestCell->lfTotal = ptTestCell->lfSamplesTotal
                                    *(double)ptTestCell->ldExecs/(double)ptTestCell->ldSamplesValidExecs;
                            ptTestCell->lfTotalSquare = ptTestCell->lfSamplesTotalSquare
                                    *(double)ptTestCell->ldExecs/(double)ptTestCell->ldSamplesValidExecs;
                        }
                    }
                }
#endif

                // If summary didn't include the failcount, use the sampled data info!
                if(ptTestCell->GetCurrentLimitItem()->ldFailCount == -1)
                    ptTestCell->GetCurrentLimitItem()->ldFailCount = ptTestCell->GetCurrentLimitItem()->ldSampleFails;

                // If neither PTR values, this test was not executed !
                if(ptTestCell->ldExecs == 0 && ptTestCell->GetCurrentLimitItem()->ldOutliers == 0)
                    goto NextTestCell;

                // IF Muti-result parametric test, do not show master test record
                if(ptTestCell->lResultArraySize > 0)
                    goto NextTestCell;

                ///////////////////////////////////////////////////////////////
                // Compute BASIC statistics
                // Includes: Mean, Sigma, Range, Cp, Cpk, etc...
                ///////////////////////////////////////////////////////////////
                m_cStats.ComputeBasicTestStatistics(ptTestCell,ptTestCell->bStatsFromSamples);

                // Rebuild Histogram array as filters might have some impacts.
                m_cStats.RebuildHistogramArray(ptTestCell, m_pReportOptions->iHistogramType);

                ///////////////////////////////////////////////////////////////
                // Compute advanced statistics
                // Includes: mediane, etc...
                ///////////////////////////////////////////////////////////////
                if(m_pReportOptions->mComputeAdvancedTest)
                {
                    QString f=m_pReportOptions->GetOption("statistics", "cp_cpk_computation").toString();
                    m_cStats.ComputeAdvancedDataStatistics(ptTestCell,false,f=="percentile"?true:false,
                                                           bIqrDataCleaning);

                    // If outlier removal was set to Q1,Q3+/-N*IQRT, we then need to recompute all statistics!
                    if(bIqrDataCleaning)
                    {
                        // Recompute basic statistics since samples have been removed!
                        m_cStats.ComputeBasicTestStatistics(ptTestCell,true);
                        // Recompute advanced statistics since samples have been removed!
                        m_cStats.ComputeAdvancedDataStatistics(ptTestCell,true, f=="percentile"?true:false ,false);
                    }
                }

                // If Outlier is based on Sigma, compute it.
                if(ptTestCell->ldSamplesValidExecs && ptTestCell->m_testResult.count() > 0)
                {
                    lfValue = -1; // No N*Sigma outlier remomval
                    if ( (orm =="n_sigma") || (orm =="exclude_n_sigma") )
                     lfValue = (float)orf;

                    // Outlier/Inlier removal of N*Sigma is enabled...
                    if(lfValue>0)
                    {
                      //FIXME: not used ?
                      /*  double lfExponent = ScalingPower(ptTestCell->res_scal);
                        if (isScalingNormalized)
                            lfExponent = 1.0;*/

                        // Compute exponent to have value normalized.
                        double lfOutlierLimit1,lfOutlierLimit2;
                        int iOutliers=0;

                        do
                        {
                            // If range=0, no more filtering needed
                            if(ptTestCell->lfRange == 0.0)
                                break;

                            // Update Outlier limits
                            // Mean + N*Sigma/2;
                            ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier = ptTestCell->lfMean + (lfValue*ptTestCell->lfSigma);
                            // Mean - N*Sigma/2;
                            ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier = ptTestCell->lfMean - (lfValue*ptTestCell->lfSigma);

                            // Outlier limits scaled to result format
                            lfOutlierLimit2 = ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier;// /lfExponent;	// Mean + N*Sigma/2;
                            lfOutlierLimit1 = ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier;// /lfExponent;	// Mean - N*Sigma/2;

                            if (orm=="n_sigma")
                            {
                                iOutliers = RemoveDataPoints(NULL,pGroup,ptTestCell,lfOutlierLimit1,0.0,
                                                             GEX_REMOVE_LOWER_THAN);
                                iOutliers += RemoveDataPoints(NULL,pGroup,ptTestCell,0.0,lfOutlierLimit2,
                                                              GEX_REMOVE_HIGHER_THAN);
                            }
                            else if (orm=="exclude_n_sigma")
                            {
                                // Remove data within range, non-recursive as we remove data within range.
                                RemoveDataPoints(NULL,pGroup,ptTestCell,lfOutlierLimit1,lfOutlierLimit2,
                                                 GEX_REMOVE_DATA_RANGE);
                                iOutliers = 0;
                            }


                            // Recompute basic statistics since samples have been removed!
                            if(iOutliers)
                                m_cStats.ComputeBasicTestStatistics(ptTestCell,true);

                        }
                        while(iOutliers);
                    }
                }

                // Check for user 'Abort' signal
                if(GS::Gex::CSLEngine::GetInstance().IsAbortRequested())
                    goto exit_stats;

                // shape detection
                if(computeShape)
                {
                    if (lPatInfo)
                    {
                        patlib_GetDistributionType(ptTestCell,
                                                   lPatInfo->GetRecipeOptions().iCategoryValueCount,
                                                   lPatInfo->GetRecipeOptions().bAssumeIntegerCategory,
                                                   lPatInfo->GetRecipeOptions().mMinConfThreshold);
                    }
                    else
                        patlib_GetDistributionType(ptTestCell);
                }

                // Point to next test cell
                NextTestCell:
                ptTestCell = ptTestCell->GetNextTest();
            };	// Loop until all test cells read.

            // Check if for this group statistics must be from summary or samples
            if(lTotalOutliers)
                // We have outliers detetced...then we have to compute our own statistics and do not trust the Summary.
                pGroup->cMergedData.bPartsFiltered = true;
        }
        // Remember which current group is processed (1=reference, >=2: other)
        iCurrentGroup++;
    };	// Loop until all groups analyzed

    // Compute statistics related to comparing groups (mean, sigma shifts):
    // lust be done even if Stats report disabled because of the Interactive mode!
    if((m_pReportOptions->GetOption( "statistics", "shift_with" ) ).toString() == "shift_to_all" )
    {
        ComputeStatsShift();
        MaxShiftCalculator( this ).ComputeMaxShiftInReferenceGroupOfFiles();
    }
    else
        ComputeStatsShift();

    // Now that all basic stats are computed for all groups, compute Gage R&R if enabled
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if(pGroup != NULL && (m_pReportOptions->getAdvancedReport() == GEX_ADV_CANDLE_MEANRANGE))
    {
        // List of tests in this group.
        ptTestCell = pGroup->cMergedData.ptMergedTestList;

        while(ptTestCell != NULL)
        {
            // GCORE-11713
            if (pGroup->pFilesList.size() == 1)
            {
                QString lErrorMessage(
                    "Gage R&R report can only be done using the analysis mode"
                    " \"Compare Group of Files\". Please return to the home tab"
                    " and use \"Compare Group of Files\" mode to perform a Gage"
                    " R&R study");
                this->GetReportLogList().
                    addReportLog(lErrorMessage,
                                 GS::Gex::ReportLog::ReportError);
                break;
            }

            // If neither PTR values, this test was not executed !
            if(ptTestCell->ldExecs == 0 && ptTestCell->GetCurrentLimitItem()->ldOutliers == 0)
                goto NextGageTestCell;

            // IF Muti-result parametric test, do not show master test record
            if(ptTestCell->lResultArraySize > 0)
                goto NextGageTestCell;

            // Compute Gage R&R (only required when computing first group as it addresses all groups internally)
            ComputeTestGage(ptTestCell);

            // Compute XBar-R (only required when computing first group as it addresses all groups internally)
            ComputeTestXBarR(ptTestCell);

NextGageTestCell:
            ptTestCell = ptTestCell->GetNextTest();
        };	// Loop until all test cells read.
    }

    // Compute/Update Part count in case not accurate or not set
    int	iIndex;
    int	iBinCode;
    CBinning *ptBinList;

    // Back to front of the list
    itGroupsList.toFront();
    while(itGroupsList.hasNext())
    {
        // Get handle to file
        pGroup	= itGroupsList.next();
        QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

        while (itFilesList.hasNext())
        {
            pFile = itFilesList.next();

            // Clear variable
            lTotalGood = 0;

            // If Gross die count defined, use it to overload physical die count
            if(pFile->grossDieCount() > 0)
                pFile->getWaferMapData().iTotalPhysicalDies = pFile->grossDieCount();

            if(pFile->getPcrDatas().lPartCount == GS_MAX_UINT32 && pFile->getWaferMapData().bWaferMapExists)
            {
                // No PCR record: so we need to deduct the total good parts from the Bin list...
                pFile->getPcrDatas().lPartCount = pFile->getWaferMapData().iTotalPhysicalDies;
            }

            if(pFile->getPcrDatas().lGoodCount < 0 && pFile->getWaferMapData().bWaferMapExists
                    && pFile->getWaferMapData().getWafMap()!=NULL)
            {
                // No PCR record: so we need to deduct the total good parts from the Bin list...
                for(iIndex=0; iIndex < pFile->getWaferMapData().SizeX*pFile->getWaferMapData().SizeY; iIndex++)
                {
                    iBinCode = pFile->getWaferMapData().getWafMap()[iIndex].getBin();
                    if(iBinCode != GEX_WAFMAP_EMPTY_CELL)
                    {
                        // Check if this bin is a good bin.
                        ptBinList = pGroup->cMergedData.ptMergedSoftBinList;
                        while(ptBinList != NULL)
                        {
                            if(ptBinList->cPassFail == 'P' && ptBinList->iBinValue == iBinCode)
                            {
                                lTotalGood++;
                                break;
                            }
                            else
                                ptBinList = ptBinList->ptNextBin;
                        };
                    }
                }
                if(lTotalGood)
                    pFile->getPcrDatas().lGoodCount = lTotalGood;
            }
        }
    };

    if (scaling=="smart")
    {
        QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());

        while(itGroupsList.hasNext())
        {
            pGroup		= itGroupsList.next();
            if (!pGroup)
            {
                GSLOG(SYSLOG_SEV_WARNING, "error : pGroup NULL !");
                break;
            }
            ptTestCell	= pGroup->cMergedData.ptMergedTestList;

            while (ptTestCell)
            {
                ptTestCell->executeSmartScaling();

                ptTestCell = ptTestCell->GetNextTest();
            };
        };
    }

exit_stats:;
}

// Update the samples data in Gex memory + updates Test statistics HTML page.
void CGexReport::UpdateSamplesRawData(CGexChartOverlays* pChartsInfo,
                                      QTableWidget* /*pTableWidget*/,
                                      CGexFileInGroup* /*pFile*/,
                                      CTest* /*ptTestReferenceCell*/)
{
    // Read Table and replace the data in memory.
    // Create HTML page / info page
    CheckForNewHtmlPage(pChartsInfo,SECTION_ADV_TREND,0);
}

int	CGexReport::PrepareSection_Stats(BOOL /*bValidSection*/)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    QString	strStatisticsFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"), QString("fields")))
            .toString();
    QStringList qslStatisticsFieldsToInclude = strStatisticsFieldsOptions.split(QString("|"));

    QString strStatisticsCompareFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"), QString("compare_fields"))).toString();
    QStringList qslStatisticsCompareFieldsList = strStatisticsCompareFieldsOptions.split(QString("|"));

    QString strStatisticsGageFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"), QString("gage_fields"))).toString();
    QStringList qslStatisticsGageFieldsList = strStatisticsGageFieldsOptions.split(QString("|"));

    QString strStatisticsAdvFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"), QString("adv_fields"))).toString();
    QStringList qslStatisticsAdvFieldsList = strStatisticsAdvFieldsOptions.split(QString("|"));


    // Creates the Test Statistics page & header
    if ( (strOutputFormat=="CSV")
        && (m_pReportOptions->iStatsType != GEX_STATISTICS_DISABLED)
        )
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Statistics ----\n\n");
        fprintf(hReportFile,"Test,");

        if(qslStatisticsFieldsToInclude.contains(QString("test_name")))
            fprintf(hReportFile,"Name,");
        if(m_pReportOptions->iGroups > 1)
            fprintf(hReportFile,"Group name,");
        if(qslStatisticsFieldsToInclude.contains(QString("test_type")))
            fprintf(hReportFile,"Type,");
        if(qslStatisticsFieldsToInclude.contains(QString("limits")))
            fprintf(hReportFile,"Low Limit,units,High Limit,units,");
        if(qslStatisticsFieldsToInclude.contains(QString("spec_limits")))
            fprintf(hReportFile,"Low Spec.,units,High Spec.,units,");
        if(qslStatisticsFieldsToInclude.contains(QString("drift_limits")))
            fprintf(hReportFile,"Test Vs Spec %%,Low Limit Test/Spec %%,High Limit Test/Spec %%,");
        if(qslStatisticsAdvFieldsList.contains(QString("shape")))
            fprintf(hReportFile,"Shape,");
        if(qslStatisticsFieldsToInclude.contains(QString("stats_source")))
            fprintf(hReportFile,"Source,");
        if(qslStatisticsFieldsToInclude.contains(QString("exec_count")))
            fprintf(hReportFile,"Executions,");
        if(qslStatisticsFieldsToInclude.contains(QString("fail_count")))
            fprintf(hReportFile,"Fail count,");
        if(qslStatisticsFieldsToInclude.contains(QString("fail_percent")))
            fprintf(hReportFile,"Fail %%,");
        if(qslStatisticsFieldsToInclude.contains(QString("fail_bin")))
            fprintf(hReportFile,"Fail Bin#,");
        if(qslStatisticsFieldsToInclude.contains(QString("test_flow_id")))
            fprintf(hReportFile,"Flow ID,");
        if(qslStatisticsFieldsToInclude.contains(QString("removed_count")))
            fprintf(hReportFile,"Removed Count,");
        if(qslStatisticsFieldsToInclude.contains(QString("mean")))
            fprintf(hReportFile,"Mean,units,");
        if((qslStatisticsCompareFieldsList.contains(QString("mean_shift"))) && (m_pReportOptions->iGroups > 1))
            fprintf(hReportFile,"Mean shift,");
        if((qslStatisticsCompareFieldsList.contains(QString("t_test"))) && (m_pReportOptions->iGroups == 2))
            fprintf(hReportFile,"T-Test,");
        if(qslStatisticsFieldsToInclude.contains(QString("sigma")))
            fprintf(hReportFile,"Sigma,units,");
        if((qslStatisticsCompareFieldsList.contains(QString("sigma_shift"))) && (m_pReportOptions->iGroups > 1))
            fprintf(hReportFile,"Sigma shift,");
        if(qslStatisticsFieldsToInclude.contains(QString("2sigma")))
            fprintf(hReportFile,"2xSigma,units,");
        if(qslStatisticsFieldsToInclude.contains(QString("3sigma")))
            fprintf(hReportFile,"3xSigma,units,");
        if(qslStatisticsFieldsToInclude.contains(QString("6sigma")))
            fprintf(hReportFile,"6xSigma,units,");
        if(qslStatisticsFieldsToInclude.contains(QString("min")))
            fprintf(hReportFile,"Min,units,");
        if(qslStatisticsFieldsToInclude.contains(QString("max")))
            fprintf(hReportFile,"Max,units,");
        if(qslStatisticsFieldsToInclude.contains(QString("range")))
            fprintf(hReportFile,"Range,units,");
        if((qslStatisticsCompareFieldsList.contains(QString("max_range"))) && (m_pReportOptions->iGroups > 1))
            fprintf(hReportFile,"Max. Range,units,");
        if(qslStatisticsFieldsToInclude.contains(QString("cp")))
            fprintf(hReportFile,"Cp,");
        if(qslStatisticsFieldsToInclude.contains(QString("cr"))) // GCORE-199
            fprintf(hReportFile,"Cr,");
        if((qslStatisticsCompareFieldsList.contains(QString("cp_shift"))) && (m_pReportOptions->iGroups > 1))
            fprintf(hReportFile,"Cp shift,");
        if((qslStatisticsCompareFieldsList.contains(QString("cr_shift"))) && (m_pReportOptions->iGroups > 1))
            fprintf(hReportFile,"Cr shift,");
        if(qslStatisticsFieldsToInclude.contains(QString("cpk")))
            fprintf(hReportFile,"Cpk,");
        if(qslStatisticsAdvFieldsList.contains(QString("cpk_l")))
            fprintf(hReportFile,"Cpl,");
        if(qslStatisticsAdvFieldsList.contains(QString("cpk_h")))
            fprintf(hReportFile,"Cpu,");
        if((qslStatisticsCompareFieldsList.contains(QString("cpk_shift"))) && (m_pReportOptions->iGroups > 1))
            fprintf(hReportFile,"Cpk shift,");
        if(qslStatisticsFieldsToInclude.contains(QString("yield")))
            fprintf(hReportFile,"Yield,");
        if(qslStatisticsGageFieldsList.contains(QString("ev")))
            fprintf(hReportFile,"Gage EV,");
        if(qslStatisticsGageFieldsList.contains(QString("av")))
            fprintf(hReportFile,"Gage AV,");
        if(qslStatisticsGageFieldsList.contains(QString("r&r")))
            fprintf(hReportFile,"Gage R&R,");
        if(qslStatisticsGageFieldsList.contains(QString("gb")))
            fprintf(hReportFile,"Gage GB,");
        if(qslStatisticsGageFieldsList.contains(QString("pv")))
            fprintf(hReportFile,"Gage PV,");
        if(qslStatisticsGageFieldsList.contains(QString("tv")))
            fprintf(hReportFile,"Gage TV,");
        if(qslStatisticsGageFieldsList.contains(QString("p_t")))
            fprintf(hReportFile,"Gage P/T,");
        if(qslStatisticsAdvFieldsList.contains(QString("skew")))
            fprintf(hReportFile,"Skew,");
        if(qslStatisticsAdvFieldsList.contains(QString("kurtosis")))
            fprintf(hReportFile,"Kurtosis,");
        if(qslStatisticsAdvFieldsList.contains(QString("P0.5")))
            fprintf(hReportFile,"P0.5%%,units,");
        if(qslStatisticsAdvFieldsList.contains(QString("P2.5")))
            fprintf(hReportFile,"P2.5%%,units,");
        if(qslStatisticsAdvFieldsList.contains(QString("P10")))
            fprintf(hReportFile,"P10%%,units,");
        if(qslStatisticsAdvFieldsList.contains(QString("quartile1")))
            fprintf(hReportFile,"P25%% - Q1,units,");
        if(qslStatisticsAdvFieldsList.contains(QString("quartile2")))
            fprintf(hReportFile,"P50%% - Median,units,");
        if(qslStatisticsAdvFieldsList.contains(QString("quartile3")))
            fprintf(hReportFile,"P75%% - Q3,units,");
        if(qslStatisticsAdvFieldsList.contains(QString("P90")))
            fprintf(hReportFile,"P90%%,units,");
        if(qslStatisticsAdvFieldsList.contains(QString("P97.5")))
            fprintf(hReportFile,"P97.5%%,units,");
        if(qslStatisticsAdvFieldsList.contains(QString("P99.5")))
            fprintf(hReportFile,"P99.5%%,units,");
        if(qslStatisticsAdvFieldsList.contains(QString("inter_quartiles")))
            fprintf(hReportFile,"IQR,units,");
        if(qslStatisticsAdvFieldsList.contains(QString("sigma_inter_quartiles")))
            fprintf(hReportFile,"IQR SD,units,");
        fprintf(hReportFile,"\n");
    }

    // Will be used while creating all Statistic pages: only used if HTML pages created !
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if (m_pReportOptions->isReportOutputHtmlBased())
        mTotalHtmlPages = mTotalStatisticsPages;
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Writes a section with tests mismatches (test in one group, but not in the other)
// Test time difference, etc...
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteMismatchTestsPage(void)
{
    // If only one group, we can't have any mismatch in compare mode!
    // If more than two groups, too many combinations...so ignore this report section!
    if(getGroupsList().count() != 2)
        return;

    // Write list of mismatching tests in each group
    CGexGroupOfFiles *pGroup1 = getGroupsList().at(0);
    CTest *pTest1 = pGroup1->cMergedData.ptMergedTestList;	// Handle to test list in 1st group
    CGexFileInGroup *pFile1  = (pGroup1->pFilesList.isEmpty()) ? NULL : pGroup1->pFilesList.at(0);
    CGexGroupOfFiles *pGroup2 = getGroupsList().at(1);
    CTest *pTest2 = pGroup2->cMergedData.ptMergedTestList;	// Handle to test list in 2nd group
    CGexFileInGroup *pFile2  = (pGroup2->pFilesList.isEmpty()) ? NULL : pGroup2->pFilesList.at(0);

    // Scan list of tests in Group#1
    CTest *pTest,*ptTestCell;
    QList <CTest*>	cTestsOnlyInGroup1;

    for(pTest = pTest1; pTest != NULL; pTest = pTest->GetNextTest())
    {
        // Check if this test is also in the Group#2
        if(pFile2->FindTestCell(pTest->lTestNumber,pTest->lPinmapIndex,&ptTestCell,true,false,
                                pTest->strTestName.toLatin1().data()) != 1)
            cTestsOnlyInGroup1.append(pTest);	// This test is ONLY in Group#1
    }

    // Scan list of tests in Group#2
    QList <CTest*>	cTestsOnlyInGroup2;

    for(pTest = pTest2; pTest != NULL; pTest = pTest->GetNextTest())
    {
        // Check if this test is also in the Group#2
        if(pFile1->FindTestCell(pTest->lTestNumber,pTest->lPinmapIndex,&ptTestCell,true,false,
                                pTest->strTestName.toLatin1().data()) != 1)
            cTestsOnlyInGroup2.append(pTest);	// This test is ONLY in Group#1
    }

    // If perfect match, we sould have not detected any mismatch
    if(cTestsOnlyInGroup1.count() == 0 && cTestsOnlyInGroup2.count() == 0)
        return;

    // Mismatch detected...create section!
    QString strTitle = "Compare tests: Mismatch in test list!";
    WriteHtmlSectionTitle(hReportFile,strTitle,strTitle);

    // Open Table
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if (m_pReportOptions->isReportOutputHtmlBased())
        fprintf(hReportFile,"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");

    // Compute testing time
    long lTestDuration1=0;
    long lTestDuration2=0;
    if(pFile1->getMirDatas().lEndT <= 0 || pFile1->getMirDatas().lStartT <= 0)
        lTestDuration1 = -1;
    else
    {
        lTestDuration1 = pFile1->getMirDatas().lEndT-pFile1->getMirDatas().lStartT;
        if(lTestDuration1<=0)
        {
            // Sometimes start=end...so compute end-setup
            lTestDuration1 = pFile1->getMirDatas().lEndT-pFile1->getMirDatas().lSetupT;
        }
    }
    if(pFile2->getMirDatas().lEndT <= 0 || pFile2->getMirDatas().lStartT <= 0)
        lTestDuration2 = -1;
    else
    {
        lTestDuration2 = pFile2->getMirDatas().lEndT-pFile2->getMirDatas().lStartT;
        if(lTestDuration2<=0)
        {
            // Sometimes start=end...so compute end-setup
            lTestDuration2 = pFile2->getMirDatas().lEndT-pFile2->getMirDatas().lSetupT;
        }
    }

    QString strString;
    char	szString[80];
    float	fData;

    // Average testing time per unit for Group#1 (GOOD parts)
    if(pGroup1->cMergedData.lMergedTestTimeParts_Good != 0)
    {
        // Compute test time in seconds.
        double fValue = (pGroup1->cMergedData.lMergedAverageTestTime_Good/pGroup1->cMergedData.lMergedTestTimeParts_Good)/1000.0;
        sprintf(szString,"%.3lf sec. (excludes tester idle time)",fValue);
    }
    else
        sprintf(szString,GEX_NA);
    strString =  "Test time (GOOD parts): " + QString(szString);
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
        strString += "<br>";	// HTML or Flat HTML (word, pdf, ...)
    else
        strString += "\n";	// CSV format

    // Average testing time per unit for Group#1 (ALL parts)
    if(pGroup1->cMergedData.lMergedTestTimeParts_All != 0)
    {
        // Compute test time in seconds.
        double fValue = (pGroup1->cMergedData.lMergedAverageTestTime_All/pGroup1->cMergedData.lMergedTestTimeParts_All)/1000.0;
        sprintf(szString,"%.3lf sec. (excludes tester idle time)",fValue);
    }
    else
        sprintf(szString,GEX_NA);
    strString =  "Test time (ALL parts): " + QString(szString);
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
        strString += "<br>";	// HTML or Flat HTML (word, pdf, ...)
    else
        strString += "\n";	// CSV format

    // Average testing time / device
    if((pFile1->getPcrDatas().lPartCount) && (pFile1->getPcrDatas().lPartCount != GS_MAX_UINT32) && (lTestDuration1>0))
    {
        fData = (float) lTestDuration1/pFile1->getPcrDatas().lPartCount;
        sprintf(szString,"%.3f sec. / device (includes tester idle time between parts)",fData);
    }
    else
        sprintf(szString,GEX_NA);
    strString +=  "Average test time: " + QString(szString);
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
        strString += "<br>";	// HTML or Flat HTML (word, pdf, ...)
    else
        strString += "\n";	// CSV format
    WriteInfoLine(pGroup1->strGroupName.toLatin1().constData(), strString.toLatin1().constData());


    // Average testing time per unit for Group#2 (GOOD parts)
    if(pGroup2->cMergedData.lMergedTestTimeParts_Good != 0)
    {
        // Compute test time in seconds.
        double fValue = (pGroup2->cMergedData.lMergedAverageTestTime_Good/pGroup2->cMergedData.lMergedTestTimeParts_Good)/1000.0;
        sprintf(szString,"%.3lf sec. (excludes tester idle time)",fValue);
    }
    else
        sprintf(szString,GEX_NA);
    strString =  "Test time (GOOD parts): " + QString(szString);
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
        strString += "<br>";	// HTML or Flat HTML (word, pdf, ...)
    else
        strString += "\n";	// CSV format

    // Average testing time per unit for Group#2 (ALL parts)
    if(pGroup2->cMergedData.lMergedTestTimeParts_All != 0)
    {
        // Compute test time in seconds.
        double fValue = (pGroup2->cMergedData.lMergedAverageTestTime_All/pGroup2->cMergedData.lMergedTestTimeParts_All)/1000.0;
        sprintf(szString,"%.3lf sec. (excludes tester idle time)",fValue);
    }
    else
        sprintf(szString,GEX_NA);
    strString =  "Test time (ALL parts): " + QString(szString);
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
        strString += "<br>";	// HTML or Flat HTML (word, pdf, ...)
    else
        strString += "\n";	// CSV format

    // Average testing time / device
    if((pFile2->getPcrDatas().lPartCount) && (pFile2->getPcrDatas().lPartCount != GS_MAX_UINT32) && (lTestDuration2>0))
    {
        fData = (float) lTestDuration2/pFile2->getPcrDatas().lPartCount;
        sprintf(szString,"%.3f sec. / device (includes tester idle time between parts)",fData);
    }
    else
        sprintf(szString,GEX_NA);
    strString +=  "Average test time: " + QString(szString);
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
        strString += "<br>";	// HTML or Flat HTML (word, pdf, ...)
    else
        strString += "\n";	// CSV format
    WriteInfoLine(pGroup2->strGroupName.toLatin1().constData(), strString.toLatin1().constData());

    // List all tests ONLY in group#1
    QString strList,strTest;
    if(cTestsOnlyInGroup1.count())
    {
        // Title
        strTitle = "Tests ONLY in group: " + getGroupsList().at(0)->strGroupName;

        // Build list of tests
        strList = "";
        foreach(pTest, cTestsOnlyInGroup1)
        {
            // To be sure that all test aren't null
            if (pTest == NULL) break;
            strTest  = "T" + QString::number(pTest->lTestNumber);
            strTest += " - " + pTest->strTestName;
           //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            if(m_pReportOptions->isReportOutputHtmlBased())
               strTest += "<br>";	// HTML or Flat HTML (word, pdf, ...)
           else
               strTest += "\n";	// CSV format
            strList += strTest;
        }

        WriteInfoLine(hReportFile, strTitle.toLatin1().constData(), strList.toLatin1().constData());
    }

    // List all tests ONLY in group#2
    if(cTestsOnlyInGroup2.count())
    {
        // Title
        strTitle = "Tests ONLY in group: " + getGroupsList().at(1)->strGroupName;

        // Build list of tests
        strList = "";
        foreach(pTest, cTestsOnlyInGroup2)
        {
            // To be sure that all test aren't null
            if (pTest == NULL) break;
             strTest  = "T" + QString::number(pTest->lTestNumber);
             strTest += " - " + pTest->strTestName;
            //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
             if(m_pReportOptions->isReportOutputHtmlBased())
                strTest += "<br>";	// HTML or Flat HTML (word, pdf, ...)
            else
                strTest += "\n";	// CSV format
             strList += strTest;
        }

        WriteInfoLine(hReportFile, strTitle.toLatin1().constData(), strList.toLatin1().constData());
    }

    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
    {
        fprintf(hReportFile,"</table>\n<p></p>\n");
        fprintf(hReportFile,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">Note: Examinator considers the first group/dataset as the master test list!<br>To review tests only in the second group, simply reorder your groups (make it first in list) from the 'Data' page.\n",iHthmNormalFontSize);
    }
}

int	CGexReport::WriteTestListIndexPage(int iPageIndexType, BOOL bValidSection,
                                       qtTestListStatistics *pqtStatisticsList/*=NULL*/)
{
  GSLOG(SYSLOG_SEV_NOTICE,
        QString(" PageIndexType: %1 StatisticsList %2")
        .arg( iPageIndexType)
        .arg((pqtStatisticsList)?static_cast<int>(pqtStatisticsList->count()) : -1)
        .toLatin1().constData());

    CTest *			ptTestCell      = NULL;		// Pointer to test cell to receive STDF info.
    QString			strPageTitle	= "";		// Used to hold the page title.
    const char *	szHtmpPage		= NULL;		// Used to hold the html page root beggining of name string.
    const char *	szChartType		= NULL;
    QString         lMessage;
    char			szString[2048];
    int				iTestPageLink	= 0;
    QString			strDrillArgument;
    QString			strBookmark;
    QString strChartMode;
    // Define which Page title to use...
    switch(iPageIndexType)
    {
        case GEX_INDEXPAGE_STATS:
            strPageTitle = m_pReportOptions->GetOption("statistics", "section_name").toString(); // "Tests Statistics";
            szHtmpPage  = "stats";	// html page name starts with this string
            szChartType = "adv_histo";	// Interactive chart will show the associated Histogram
            strBookmark = strPageTitle;
            break;

        case GEX_INDEXPAGE_WAFERMAP:
            strPageTitle = m_pReportOptions->GetOption("wafer", "section_name").toString(); // "Wafermap of Tests";
            szHtmpPage  = "wafermap";	// html page name starts with this string

            if (m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERDATA || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERDATA)
                szChartType = "wafer_param_range";	// Interactive chart will show the associated Histogram over test data range
            else if (m_pReportOptions->iWafermapType == GEX_WAFMAP_TESTOVERLIMITS || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TESTOVERLIMITS)
                szChartType = "wafer_param_limits";	// Interactive chart will show the associated Histogram over test limits
            else if (m_pReportOptions->iWafermapType == GEX_WAFMAP_TEST_PASSFAIL || m_pReportOptions->iWafermapType == GEX_WAFMAP_STACK_TEST_PASSFAIL)
                szChartType = "wafer_test_passfail";	// Interactive chart will show the associated Histogram over test limits
            strBookmark = strPageTitle;
            break;

        case GEX_INDEXPAGE_HISTO:
            strPageTitle = m_pReportOptions->GetOption("histogram", "section_name").toString(); // "Histogram of Tests";
            szHtmpPage  = "histogram";	// html page name starts with this string
            szChartType = "adv_histo";	// Interactive chart will show the associated Histogram
            strBookmark = strPageTitle;
            break;
        case GEX_INDEXPAGE_ADVHISTO:
            strPageTitle = "More Reports: Histogram (Bar chart)";
            szHtmpPage  = "advanced";	// html page name starts with this string
            szChartType = "adv_histo";	// Interactive chart will show the associated Histogram
            strBookmark = "all_advanced";
            break;
        case GEX_INDEXPAGE_ADVTREND:
            strPageTitle = "More Reports: Trend (Control chart)";
            szHtmpPage  = "advanced";	// html page name starts with this string
            szChartType = "adv_trend";	// Interactive chart will show the associated Histogram
            strBookmark = "all_advanced";
            break;
        case GEX_INDEXPAGE_ADVPROBABILITY_PLOT:
            strPageTitle = "More Reports: Probability plot chart";
            szHtmpPage  = "advanced";				// html page name starts with this string
            szChartType = "adv_probabilityplot";	// Interactive chart will show the associated ProbabilityPlot
            strBookmark = "all_advanced";
            break;
        case GEX_INDEXPAGE_ADVBOXPLOT_EX:
            strPageTitle = "More Reports: Box plot chart";
            szHtmpPage  = "advanced";	// html page name starts with this string
            szChartType = "adv_boxplot";	// Interactive chart will show the associated BoxPlot
            strBookmark = "all_advanced";
            break;
        case GEX_INDEXPAGE_ADVMULTICHART:
            strPageTitle = "More Reports: Multi-chart";
            szHtmpPage  = "advanced";				// html page name starts with this string
            szChartType = "adv_multichart";	// Interactive chart will show the associated ProbabilityPlot
            strBookmark = "all_advanced";
            break;
        case GEX_INDEXPAGE_ADVBOXPLOT:
            strPageTitle = "More Reports: Gage R&R (Box plot/Candle chart)";
            szHtmpPage  = "advanced";	// html page name starts with this string
            szChartType = "adv_boxplot";	// Interactive chart will show the associated BoxPlot
            strBookmark = "all_advanced";
            break;
        case GEX_INDEXPAGE_ADVDIAGS:
            strPageTitle = "More Reports: Examinator Diagnostics";
            szHtmpPage  = "advanced";	// html page name starts with this string
            strBookmark = "all_advanced";
            break;
        case GEX_INDEXPAGE_ADVGO:
            strPageTitle = "More Reports: Quantix Optimizer Diagnostics";
            szHtmpPage  = "advanced";	// html page name starts with this string
            strBookmark = "all_advanced";
            break;
        case GEX_INDEXPAGE_ADVHISTOFUNCTIONAL:
            strPageTitle = "More Reports: Functional Reports";
            szHtmpPage  = "advanced";	// html page name starts with this string
            szChartType = "adv_histo_functional";	// Interactive chart will show the associated Histogram
            strBookmark = "all_advanced";
            break;
    }

    // Keep track of total HTML pages written
    lReportPageNumber++;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black
    // Under word/PDF files, no such table!
    if ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
        return GS::StdLib::Stdf::NoError;

    // Title + bookmark
    WriteHtmlSectionTitle(hReportFile, strPageTitle, strPageTitle);

    if(bValidSection == false)
    {
        fprintf(hReportFile,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">No statistics data available !<br>\n",iHthmNormalFontSize);
        return GS::StdLib::Stdf::NoError;
    }

    // Lists hyperlinks to all tests in list!
    int	iPageLinks=0;
    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles *pGroup=0;   // = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    CGexFileInGroup *pFile=0;     //= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    if(pGroup)
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    else
    {
        pFile=NULL;
        GSLOG(SYSLOG_SEV_ERROR, "Undefined group (groups list empty)!");
        return GS::StdLib::Stdf::NoError; // No error code in Stdf for this kind of error
    }

    fprintf(hReportFile,"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");
    if(m_pReportOptions->iFiles == 1)
    {
        // Parts to process
        WriteInfoLine("Parts processed",(pFile->BuildPartsToProcess()).toLatin1().data());
        // Testing on sites
        WriteInfoLine("Data from Sites",pFile->BuildSitesToProcess().toLatin1().data());
    }

    const char *	ptChartMode		= NULL;
    const char *	ptChartTitle    = NULL;
    const char *	ptLinkListTitle	= "List of tests";
    QString TestsInReport;

    switch(iPageIndexType)
    {
        case GEX_INDEXPAGE_STATS:
            // Tells what type of Test statistics charting is used
            switch(m_pReportOptions->iStatsType)
            {
            case GEX_STATISTICS_DISABLED: // 0=Disabled
                ptChartMode = "Statistics report is disabled!";
                break;
            case GEX_STATISTICS_ALL:
                ptChartMode = "All tests";
                break;
            case GEX_STATISTICS_FAIL:
                ptChartMode = "Failing tests";
                break;
            case GEX_STATISTICS_OUTLIERS:
                ptChartMode = "Tests with outliers";
                break;
            case GEX_STATISTICS_LIST:
                ptChartMode = "List of tests...";
                break;
            case GEX_STATISTICS_BADCP:
                ptChartMode = "Tests under Cpk limit...";
                break;
            case GEX_STATISTICS_BADCPK:
                ptChartMode = "Tests under Cp limit...";
                break;
            case GEX_STATISTICS_TOP_N_FAILTESTS:
                ptChartMode = QString("Top %1 failing tests").arg((int)m_pReportOptions->lfStatsLimit).toLatin1().data();
                break;
            }

            WriteInfoLine("Table includes",ptChartMode);

            switch(m_pReportOptions->iStatsType)
            {
                case GEX_STATISTICS_LIST:
                    // Tests selected to be in the Statistics list
                    lMessage = m_pReportOptions->pGexStatsRangeList->BuildTestListString("Tests ");
                    WriteInfoLine("Table lists", lMessage.toLatin1().constData());
                    break;
                case GEX_STATISTICS_BADCP:
                    sprintf(szString,"%g",m_pReportOptions->lfStatsLimit);
                    WriteInfoLine("Cpk limit:",szString);
                    break;
                case GEX_STATISTICS_BADCPK:
                    sprintf(szString,"%g",m_pReportOptions->lfStatsLimit);
                    WriteInfoLine("Cp limit:",szString);
                    break;
                case GEX_STATISTICS_TOP_N_FAILTESTS:
                    sprintf(szString,"%g",m_pReportOptions->lfStatsLimit);
                    WriteInfoLine("Top failing tests ", szString);
                    break;
            }
            if (m_pReportOptions->iStatsType==GEX_STATISTICS_TOP_N_FAILTESTS)
                WriteInfoLine("Sorting mode","From worst tests (with max failed count) to better tests");
            else
                WriteInfoLine("Sorting mode",GetReportSortingMode( (m_pReportOptions->GetOption("statistics","sorting")).toString() ));

            // If Histogram is disabled, do not list GEX_STATISTICS_DISABLED!
            if(m_pReportOptions->iStatsType == GEX_HISTOGRAM_DISABLED)
            {
                fprintf(hReportFile,"</table>\n<p></p>\n");
                return GS::StdLib::Stdf::NoError;
            }
            break;

        case GEX_INDEXPAGE_WAFERMAP:
            switch(m_pReportOptions->iWafermapTests)
            {
                case GEX_WAFMAP_ALL:// All tests
                    WriteInfoLine("Wafermap for", "All tests");
                    break;
                case GEX_WAFMAP_LIST:// test or test range.
                    lMessage = m_pReportOptions->pGexWafermapRangeList->BuildTestListString("Tests: ");
                    WriteInfoLine("Wafermap for", lMessage.toLatin1().constData());
                    break;
                case GEX_WAFMAP_TOP_N_FAILTESTS:
                    WriteInfoLine("Wafermap for", QString("Top %1 failing tests")
                                  .arg(m_pReportOptions->iWafermapNumberOfTests).toLatin1().data());
                    WriteInfoLine("Sorting mode", "From worst tests (with max failed count) to better tests");
                    break;
            }
            break;

        case GEX_INDEXPAGE_HISTO:
            // Tells what type of Histogram charting is used
            switch(m_pReportOptions->iHistogramType)
            {
            case GEX_HISTOGRAM_DISABLED: // 0=Disabled
                ptChartMode = "Histogram report is disabled!";
                break;
            case GEX_HISTOGRAM_OVERLIMITS: // 1=chart over limits,
                ptChartMode = "Histogram: Test results (chart over test limits)";
                break;
            case GEX_HISTOGRAM_CUMULLIMITS: // 2=cumul chart over limits,
                ptChartMode = "Histogram: Test results (cumulative over test limits)";
                break;
            case GEX_HISTOGRAM_OVERDATA:// 3=chart  over samples
                ptChartMode = "Histogram: Test results (chart over test results, or outlier window if active)";
                break;
            case GEX_HISTOGRAM_CUMULDATA:// 4=cumul chart  over samples
                ptChartMode = "Histogram: Test results (cumulative over test results)";
                break;
            }
            WriteInfoLine("Charting mode",ptChartMode);

            switch(m_pReportOptions->iHistogramTests)
            {
                case GEX_HISTOGRAM_ALL:// All tests
                    WriteInfoLine("Histograms for", "All tests");
                    break;
                case GEX_HISTOGRAM_LIST:// test or test range.
                    lMessage = m_pReportOptions->pGexHistoRangeList->BuildTestListString("Tests: ");
                    WriteInfoLine("Histograms for", lMessage.toLatin1().constData());
                    break;
                case GEX_HISTOGRAM_TOP_N_FAIL_TESTS:
                    WriteInfoLine("Histograms for",
                                  QString("Top %1 failing tests").arg(m_pReportOptions->iHistogramNumberOfTests).toLatin1().data());
                    WriteInfoLine("Sorting mode", "From worst tests (with max failed count) to better tests");
                    break;
            }

            // If Histogram is disabled, do not list hyperlinks!
            if(m_pReportOptions->iHistogramType == GEX_HISTOGRAM_DISABLED)
            {
                fprintf(hReportFile,"</table>\n<p></p>\n");
                return GS::StdLib::Stdf::NoError;
            }
            break;

        case GEX_INDEXPAGE_ADVHISTOFUNCTIONAL :
            // Most of functions keep writing to the 'hReportFile' file.
            hReportFile = hAdvancedReport;
            // Single file analysis
            if(m_pReportOptions->iFiles == 1)
            {
                WriteInfoLine(hReportFile, "Product",pFile->getMirDatas().szPartType);
                WriteInfoLine(hReportFile, "Program",pFile->getMirDatas().szJobName);
                WriteInfoLine(hReportFile, "Lot",pFile->getMirDatas().szLot);
                WriteInfoLine(hReportFile, "Date",TimeStringUTC(pFile->getMirDatas().lStartT));
            }
            strChartMode = QString("Histogram for : %1 ").arg(m_pReportOptions->getAdvancedReportSettings() == GEX_ADV_FUNCTIONAL_CYCL_CNT ? "Cycle count of vector " : "Relative vector address ");
            WriteInfoLine("Charting mode", strChartMode.toLatin1().constData());
            // Creates sub-string listing pin# if used.
           if (m_pReportOptions->strAdvancedTestList=="all")
                 lMessage = "All functionals tests (fails vectors)";
            else
                lMessage = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("Functionals tests (fails vectors): ");
            WriteInfoLine(hReportFile, "Histogram chart for tests", lMessage.toLatin1().constData());
            break;

        case GEX_INDEXPAGE_ADVHISTO:
            // Most of functions keep writing to the 'hReportFile' file.
            hReportFile = hAdvancedReport;
            // Single file analysis
            if(m_pReportOptions->iFiles == 1)
            {
                WriteInfoLine(hReportFile, "Product",pFile->getMirDatas().szPartType);
                WriteInfoLine(hReportFile, "Program",pFile->getMirDatas().szJobName);
                WriteInfoLine(hReportFile, "Lot",pFile->getMirDatas().szLot);
                WriteInfoLine(hReportFile, "Date",TimeStringUTC(pFile->getMirDatas().lStartT));
            }
            // Tells what type of Histogram charting is used
            switch(m_pReportOptions->getAdvancedReportSettings())
            {
            case GEX_ADV_HISTOGRAM_OVERLIMITS: // chart over limits,
            default:
                ptChartMode = "Histogram: Test results (chart over test limits)";
                break;
            case GEX_ADV_HISTOGRAM_CUMULLIMITS: // cumul chart over limits,
                ptChartMode = "Histogram: Test results (cumulative over test limits)";
                break;
            case GEX_ADV_HISTOGRAM_OVERDATA:// chart  over samples
                ptChartMode = "Histogram: Test results (chart over test results)";
                break;
            case GEX_ADV_HISTOGRAM_CUMULDATA:// cumul chart  over samples
                ptChartMode = "Histogram: Test results (cumulative over test results)";
                break;
            }
            WriteInfoLine("Charting mode", ptChartMode);
            // Creates sub-string listing pin# if used.
            if (m_pReportOptions->strAdvancedTestList.startsWith("top"))
                lMessage = m_pReportOptions->strAdvancedTestList;
            else if (m_pReportOptions->strAdvancedTestList=="all")
                lMessage = "All tests";
            else
                lMessage = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("Tests: ");
            WriteInfoLine(hReportFile, "Histogram chart for tests", lMessage.toLatin1().constData());

            WriteInfoLine(hReportFile, "Samples requested", GetPartFilterReportType(pFile->iProcessBins).toLatin1().constData());
            break;

        case GEX_INDEXPAGE_ADVTREND:
            // Text if Single file analysis
            if(m_pReportOptions->iFiles == 1)
            {
                WriteInfoLine(hReportFile,"Product", pFile->getMirDatas().szPartType);
                WriteInfoLine(hReportFile, "Program", pFile->getMirDatas().szJobName);
                WriteInfoLine(hReportFile, "Lot", pFile->getMirDatas().szLot);
                WriteInfoLine(hReportFile, "Date", TimeStringUTC(pFile->getMirDatas().lStartT));
            }
            // Tells what type of Trend charting is used
            ptChartTitle = "Trend chart for tests";
            switch(m_pReportOptions->getAdvancedReportSettings())
            {
            case GEX_ADV_TREND_OVERLIMITS: // chart over limits,
            default:
                ptChartMode = "Trend: Test results (Control chart over test limits)";
                break;
            case GEX_ADV_TREND_OVERDATA:	// chart  over samples
                ptChartMode = "Trend: Test results (Control chart over test results)";
                break;
            case GEX_ADV_TREND_DIFFERENCE:	// Difference chart between 2 tests
                ptChartMode = "Trend: Difference between 2 tests";
                break;

            case GEX_ADV_TREND_AGGREGATE_MEAN:	// Trend Mean value
                ptChartMode = "Trend: Tests Mean";
                break;
            case GEX_ADV_TREND_AGGREGATE_SIGMA:	// Trend Sigma value
                ptChartMode = "Trend: Tests Sigma";
                break;
            case GEX_ADV_TREND_AGGREGATE_CP:	// Trend Cp value
                ptChartMode = "Trend: Tests Cp";
                break;
            case GEX_ADV_TREND_AGGREGATE_CPK:	// Trend Cpk value
                ptChartMode = "Trend: Tests Cpk";
                break;

            case GEX_ADV_TREND_SOFTBIN_SBLOTS:		// Softbin trend - Over Sub-lots
                ptChartTitle	= "Trend chart for binning";
                ptChartMode		= "Trend: Software binning (over Sub-Lots)";
                ptLinkListTitle = "List of links";
                break;
            case GEX_ADV_TREND_SOFTBIN_PARTS:		// Softbin trend - Over ALL parts
                ptChartTitle	= "Trend chart for binning";
                ptChartMode		= "Trend: Software binning (over ALL parts)";
                ptLinkListTitle = "List of links";
                break;
            case GEX_ADV_TREND_HARDBIN_SBLOTS:		// HardBin trend - Over Sub-lots
                ptChartTitle	= "Trend chart for binning";
                ptChartMode		= "Trend: Hardware binning (over Sub-Lots)";
                ptLinkListTitle = "List of links";
                break;
            case GEX_ADV_TREND_HARDBIN_PARTS:		// HardBin trend - Over ALL parts
                ptChartTitle	= "Trend chart for binning";
                ptChartMode		= "Trend: Hardware binning (over ALL parts)";
                ptLinkListTitle = "List of links";
                break;
            case GEX_ADV_TREND_SOFTBIN_ROLLING:		// Softbin trend + Rolling Yield
                ptChartTitle	= "Trend chart for binning";
                ptChartMode		= "Trend: Soft bin + Rolling yield";
                ptLinkListTitle = "List of links";
                break;
            case GEX_ADV_TREND_HARDBIN_ROLLING:		// Hardbin trend + Rolling Yield
                ptChartTitle	= "Trend chart for binning";
                ptChartMode		= "Trend: Hard bin + Rolling yield";
                ptLinkListTitle = "List of links";
                break;
            }
            WriteInfoLine("Charting mode", ptChartMode );

            // Creates sub-string listing pin# if used.
            if (m_pReportOptions->strAdvancedTestList.startsWith("top"))
                TestsInReport = m_pReportOptions->strAdvancedTestList;
            else
            {	if (m_pReportOptions->strAdvancedTestList=="all")
                    TestsInReport = "All tests";
                else
                    TestsInReport = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("");
            }

            WriteInfoLine(hReportFile, ptChartTitle, TestsInReport.toLatin1().data() );

            WriteInfoLine(hReportFile, "Samples requested", GetPartFilterReportType(pFile->iProcessBins).toLatin1().constData());
            break;

        case GEX_INDEXPAGE_ADVPROBABILITY_PLOT:
            // Text if Single file analysis
            if(m_pReportOptions->iFiles == 1)
            {
                WriteInfoLine(hReportFile, "Product",pFile->getMirDatas().szPartType);
                WriteInfoLine(hReportFile, "Program",pFile->getMirDatas().szJobName);
                WriteInfoLine(hReportFile, "Lot",pFile->getMirDatas().szLot);
                WriteInfoLine(hReportFile, "Date",TimeStringUTC(pFile->getMirDatas().lStartT));
            }
            // Tells what type of Histogram charting is used
            switch(m_pReportOptions->getAdvancedReportSettings())
            {
                case GEX_ADV_PROBPLOT_OVERLIMITS: // chart over limits,
                default:
                    ptChartMode = "Probability Plot: Test results (chart over test limits)";
                    break;
                case GEX_ADV_PROBPLOT_OVERDATA:// chart  over samples
                    ptChartMode = "Probability Plot: Test results (chart over test results)";
                    break;
            }

            WriteInfoLine("Charting mode", ptChartMode);

            if (m_pReportOptions->strAdvancedTestList.startsWith("top"))
                TestsInReport = m_pReportOptions->strAdvancedTestList;
            else
            {
                if (m_pReportOptions->strAdvancedTestList=="all")
                    TestsInReport = "All tests";
                else
                    TestsInReport = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("Tests: ");
            }

            WriteInfoLine(hReportFile, "Probability Plot chart for tests", TestsInReport.toLatin1().data() );
            WriteInfoLine(hReportFile, "Samples requested", GetPartFilterReportType(pFile->iProcessBins).toLatin1().constData());
            break;

        case GEX_INDEXPAGE_ADVBOXPLOT_EX:
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "GEX_INDEXPAGE_ADVBOXPLOT_EX");
            // Text if Single file analysis
            if(m_pReportOptions->iFiles == 1)
            {
                WriteInfoLine(hReportFile, "Product", pFile->getMirDatas().szPartType);
                WriteInfoLine(hReportFile, "Program", pFile->getMirDatas().szJobName);
                WriteInfoLine(hReportFile, "Lot", pFile->getMirDatas().szLot);
                WriteInfoLine(hReportFile, "Date", TimeStringUTC(pFile->getMirDatas().lStartT));
            }
            // Tells what type of Histogram charting is used
            switch(m_pReportOptions->getAdvancedReportSettings())
            {
                case GEX_ADV_BOXPLOT_OVERLIMITS: // chart over limits,
                default:
                    ptChartMode = GEX_T("Box-Plot: Test results (chart over test limits)");
                    break;
                case GEX_ADV_BOXPLOT_OVERDATA:// chart  over samples
                    ptChartMode = GEX_T("Box-Plot: Test results (chart over test results)");
                    break;
            }

            WriteInfoLine("Charting mode", ptChartMode);

            // Creates sub-string listing pin# if used.
            if (m_pReportOptions->strAdvancedTestList.startsWith("top"))
                TestsInReport = m_pReportOptions->strAdvancedTestList;
            else
            {
                if (m_pReportOptions->strAdvancedTestList=="all")
                    TestsInReport = "All tests";
                else
                    TestsInReport = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("Tests: ");
            }
            WriteInfoLine(hReportFile, "Box-Plot chart for tests", TestsInReport.toLatin1().data());

            WriteInfoLine(hReportFile, "Samples requested", GetPartFilterReportType(pFile->iProcessBins).toLatin1().constData());
            break;

        case GEX_INDEXPAGE_ADVMULTICHART:
            // Text if Single file analysis
            if(m_pReportOptions->iFiles == 1)
            {
                WriteInfoLine(hReportFile, "Product",pFile->getMirDatas().szPartType);
                WriteInfoLine(hReportFile, "Program",pFile->getMirDatas().szJobName);
                WriteInfoLine(hReportFile, "Lot",pFile->getMirDatas().szLot);
                WriteInfoLine(hReportFile, "Date",TimeStringUTC(pFile->getMirDatas().lStartT));
            }
            // Tells what type of Histogram charting is used
            switch(m_pReportOptions->getAdvancedReportSettings())
            {
                case GEX_ADV_MULTICHART_OVERLIMITS: // chart over limits,
                default:
                    ptChartMode = "Multi-chart: Test results (chart over test limits)";
                    break;
                case GEX_ADV_MULTICHART_OVERDATA:// chart  over samples
                    ptChartMode = "Multi-chart: Test results (chart over test results)";
                    break;
            }

            WriteInfoLine("Charting mode", ptChartMode);


            // Creates sub-string listing pin# if used.
            lMessage = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("Tests: ");

            if (m_pReportOptions->strAdvancedTestList.startsWith("top"))
                TestsInReport = m_pReportOptions->strAdvancedTestList;
            else
            {
                if (m_pReportOptions->strAdvancedTestList=="all")
                    TestsInReport = "All tests";
                else
                    TestsInReport = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("Tests: ");
            }

            WriteInfoLine( hReportFile, "Multi-chart for tests", TestsInReport.toLatin1().data() );

            WriteInfoLine(hReportFile, "Samples requested", GetPartFilterReportType(pFile->iProcessBins).toLatin1().constData());
            break;

        case GEX_INDEXPAGE_ADVBOXPLOT:
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "GEX_INDEXPAGE_ADVBOXPLOT");
            // Text if Single file analysis
            if(m_pReportOptions->iFiles == 1)
            {
                WriteInfoLine(hReportFile, "Product", pFile->getMirDatas().szPartType);
                WriteInfoLine(hReportFile, "Program", pFile->getMirDatas().szJobName);
                WriteInfoLine(hReportFile, "Lot", pFile->getMirDatas().szLot);
                WriteInfoLine(hReportFile,GEX_T("Date"),TimeStringUTC(pFile->getMirDatas().lStartT));
            }
            // Creates sub-string listing pin# if used.
            switch(m_pReportOptions->getAdvancedReportSettings())
            {
            case GEX_ADV_ALL:// All tests
            default:
                lMessage = "All tests";
                break;
            case GEX_ADV_LIST:// List of tests
                lMessage = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("Tests: ");
                break;
            }
            WriteInfoLine(hReportFile, "Gage R&R, Boxplot/Candle chart for tests",
                          lMessage.isEmpty() ? lMessage.toLatin1().constData():"N/A");
            WriteInfoLine(hReportFile, "Samples requested", GetPartFilterReportType(pFile->iProcessBins).toLatin1().constData());
            WriteInfoLine(hReportFile, "Sorting mode ",
                          GetReportSortingMode( (m_pReportOptions->GetOption("adv_boxplot","sorting")).toString() ) );
            break;
    }

    // If multiple groups, display ALARM conditions
    if(m_pReportOptions->iGroups >1)
    {
        double lfOptionStorageDevice;
        bool bGetOptionRslt;

        // alarm mean
        bGetOptionRslt=false;
        lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_mean")).toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);
        if (!bGetOptionRslt)
            GSLOG(SYSLOG_SEV_WARNING, "Cannot get alarm_mean option");

        if(!bGetOptionRslt)
            lfOptionStorageDevice = 5;

        if(lfOptionStorageDevice>=0)
        {
            fprintf(hReportFile,"<tr><td width=\"160\" bgcolor=%s valign=\"top\" align=\"left\"><b>Alarm if Mean shift over</b></td>\n",
                    szFieldColor);
            fprintf(hReportFile,"<td width=\"590\" bgcolor=%s>%7.2g%%",szDataColor,lfOptionStorageDevice);
            fprintf(hReportFile,
              "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(Alarm level set in: <a href=\"_gex_options.htm\">Options</a> in 'Tests statistics/Correlation ALARM criteria')</td></tr>\n");
        }

        // alarm sigma
        bGetOptionRslt=false;
        lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_sigma")).toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);
        if (!bGetOptionRslt)
            GSLOG(SYSLOG_SEV_WARNING, "Cannot get alarm_sigma option");

        if(!bGetOptionRslt)
            lfOptionStorageDevice = 1;

        if(lfOptionStorageDevice>=0)
        {
            fprintf(hReportFile,"<tr><td width=\"160\" bgcolor=%s valign=\"top\" align=\"left\"><b>Alarm if Sigma shift over</b></td>\n",
                    szFieldColor);
            fprintf(hReportFile,"<td width=\"590\" bgcolor=%s>%7.2g%%",szDataColor,lfOptionStorageDevice);
            fprintf(hReportFile,
               "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(Alarm level set in: <a href=\"_gex_options.htm\">Options</a> in 'Tests statistics/Correlation ALARM criteria')</td></tr>\n");
        }


        // alarm sigma
        bGetOptionRslt=false;
        lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_cpk")).toDouble(&bGetOptionRslt);
        GEX_ASSERT(bGetOptionRslt);

        if(!bGetOptionRslt)
            lfOptionStorageDevice = 33;

        if(lfOptionStorageDevice>=0)
        {
            fprintf(hReportFile,"<tr><td width=\"160\" bgcolor=%s valign=\"top\" align=\"left\"><b>Alarm if Cpk shift over</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td width=\"590\" bgcolor=%s>%7.2g%%",szDataColor,lfOptionStorageDevice);
            fprintf(hReportFile,
               "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(Alarm level set in: <a href=\"_gex_options.htm\">Options</a> in 'Tests statistics/Correlation ALARM criteria')</td></tr>\n");
        }
    }

    // Report description
    switch(iPageIndexType)
    {
        case GEX_INDEXPAGE_STATS:
            break;
        case GEX_INDEXPAGE_HISTO:
            break;
        case GEX_INDEXPAGE_ADVHISTO:
            break;
        case GEX_INDEXPAGE_ADVTREND:
            break;
        case GEX_INDEXPAGE_ADVPROBABILITY_PLOT:
            break;
        case GEX_INDEXPAGE_ADVBOXPLOT_EX:
            break;
        case GEX_INDEXPAGE_ADVMULTICHART:
            break;
        case GEX_INDEXPAGE_ADVGO:
        case GEX_INDEXPAGE_ADVDIAGS:
            break;
        case GEX_INDEXPAGE_ADVBOXPLOT:
            WriteInfoLine(hReportFile, "Report Description:",
                "The Gage R&R / BoxPlot chart is ideal for Repeatability and Reproducibility studies..<br>"
                "If no low limit exists, Examinator computes LowLimit = Minimum/2 ; if no high limit exists, Examinator computes HighLimit = Maximum*2.<br>"
                "(Define the columns to include in the report: <a href=\"_gex_options.htm\">Options</a> in 'Gage R&R-Boxplot/Fields to include in chart')<br>");
            break;
    }
    // Close header table
    fprintf(hReportFile,"</table>\n<br>\n");

    // List index table
    fprintf(hReportFile,"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");
    fprintf(hReportFile,"<tr><td width=\"160\" bgcolor=%s valign=\"top\" align=\"left\"><b>%s</b></td>\n",szFieldColor,ptLinkListTitle);
    if(m_pReportOptions->iGroups >1)
    {
        fprintf(hReportFile,"<td width=\"160\" bgcolor=%s align=\"center\">Drift alarm<br>(Mean, Sigma, Cpk)</td>\n",szDataColor);
        fprintf(hReportFile,"<td width=\"430\" bgcolor=%s>Name</td></tr>\n",szDataColor);
    }
    else
        fprintf(hReportFile,"<td width=\"590\" bgcolor=%s>Name</td></tr>\n",szDataColor);

    // List of tests in Reference group.
    int iFuncTest = 0;
    if(m_pReportOptions->getAdvancedReport() == GEX_ADV_FUNCTIONAL && mAdvancedTestsListToReport.count())
        ptTestCell = mAdvancedTestsListToReport.first();
    else if(pqtStatisticsList)
        ptTestCell = (pqtStatisticsList->count()) ? pqtStatisticsList->first() : (NULL);
    else
        ptTestCell = pGroup->cMergedData.ptMergedTestList;

    int	iLastPageUsed=0;
    int lIndexTest      = 0;

    while(ptTestCell != NULL)
    {
        // Check if this test must be in the list...
        switch(iPageIndexType)
        {
            case GEX_INDEXPAGE_STATS:
                iTestPageLink = ptTestCell->iHtmlStatsPage;
                break;
            case GEX_INDEXPAGE_HISTO:
                iTestPageLink = ptTestCell->iHtmlHistoPage;
                break;
            case GEX_INDEXPAGE_WAFERMAP:
                iTestPageLink = ptTestCell->iHtmlWafermapPage;
                break;
            case GEX_INDEXPAGE_ADVHISTO:
            case GEX_INDEXPAGE_ADVTREND:
            case GEX_INDEXPAGE_ADVPROBABILITY_PLOT:
            case GEX_INDEXPAGE_ADVBOXPLOT_EX:
            case GEX_INDEXPAGE_ADVMULTICHART:
            case GEX_INDEXPAGE_ADVBOXPLOT:
            case GEX_INDEXPAGE_ADVGO:
            case GEX_INDEXPAGE_ADVDIAGS:
            case GEX_INDEXPAGE_ADVHISTOFUNCTIONAL:
                iTestPageLink = ptTestCell->iHtmlAdvancedPage;
                break;
        }
        // Check if valid HTML page index...
        if(iTestPageLink == 0)
            goto NextTestCell;	// no!...ignore this test!

        // Keep track of last page used.
        iLastPageUsed = iTestPageLink;

        // Compute the bookmark string
        switch(iPageIndexType)
        {
            case GEX_INDEXPAGE_STATS:
                strBookmark.sprintf("%s%d.htm#StatT%s",szHtmpPage,iTestPageLink,ptTestCell->szTestLabel);
                break;
            case GEX_INDEXPAGE_HISTO:
                strBookmark.sprintf("%s%d.htm#HistoT%s",szHtmpPage,iTestPageLink,ptTestCell->szTestLabel);
                break;
            case GEX_INDEXPAGE_WAFERMAP:
                strBookmark.sprintf("%s%d.htm",szHtmpPage,iTestPageLink);
                break;
            case GEX_INDEXPAGE_ADVHISTO:
            case GEX_INDEXPAGE_ADVTREND:
            case GEX_INDEXPAGE_ADVPROBABILITY_PLOT:
            case GEX_INDEXPAGE_ADVMULTICHART:
            case GEX_INDEXPAGE_ADVBOXPLOT_EX:
                if (!mAdvancedTestsListToReport.contains(ptTestCell))
                    goto NextTestCell;
                else
                    strBookmark.sprintf("%s%d.htm#AdvT%s",szHtmpPage,iTestPageLink,ptTestCell->szTestLabel);
                break;

            case GEX_INDEXPAGE_ADVHISTOFUNCTIONAL:
                break;

            case GEX_INDEXPAGE_ADVBOXPLOT:
            case GEX_INDEXPAGE_ADVDIAGS:
            case GEX_INDEXPAGE_ADVGO:
                strBookmark.sprintf("%s%d.htm#AdvT%s",szHtmpPage,iTestPageLink,ptTestCell->szTestLabel);
                break;
        }
        // Write test # +hyperlink to its page
        if(iPageIndexType != GEX_INDEXPAGE_ADVHISTOFUNCTIONAL)
        fprintf(hReportFile,"<tr><td width=\"160\" bgcolor=%s><b><a name=\"T%s\"></a> <a href=\"%s\">%s</a> </b></td>",
            szFieldColor,ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptTestCell->szTestLabel);
        else{
            if (!mAdvancedTestsListToReport.contains(ptTestCell))
                goto NextTestCell;

            foreach(const QString &strPattern, ptTestCell->mVectors.keys()){
                if(!ptTestCell->mVectors[strPattern].m_lstVectorInfo.count())
                    continue;

                QString strPatterBookMark = strBookmark;
                strPatterBookMark.sprintf("%s%d.htm#AdvT%s",szHtmpPage,iTestPageLink,ptTestCell->szTestLabel);
                QString strFunctionalTest = QString("%1 Pattern (%2)").arg(ptTestCell->szTestLabel).arg(strPattern);

                fprintf(hReportFile,"<tr><td width=\"160\" bgcolor=%s><b><a name=\"T%s\"></a> <a href=\"%s\">%s</a> </b></td>",
                        szFieldColor,ptTestCell->szTestLabel,strPatterBookMark.toLatin1().constData()
                        ,strFunctionalTest.toLatin1().constData());
            }
        }

        if(m_pReportOptions->iGroups >1)
        {
            // Multiple groups: display Alarm section for mean,sigma,cpk drift.
            if(ptTestCell->iAlarm)
            {
                // Red color background !
                fprintf(hReportFile,"<td width=\"160\" bgcolor=%s align=\"center\">",szAlarmColor);
                if(ptTestCell->iAlarm & GEX_ALARM_MEANSHIFT)
                    fprintf(hReportFile,"M ");
                else
                    fprintf(hReportFile,"- ");

                if(ptTestCell->iAlarm & GEX_ALARM_SIGMASHIFT)
                    fprintf(hReportFile,"S ");
                else
                    fprintf(hReportFile,"- ");

                if(ptTestCell->iAlarm & GEX_ALARM_CPKSHIFT)
                    fprintf(hReportFile,"C ");
                else
                    fprintf(hReportFile,"- ");
            }
            else
            {
                fprintf(hReportFile,"<td width=\"160\" bgcolor=%s align=\"center\">- - -",szDataColor);
            }
            // End of table cell
            fprintf(hReportFile,"</td>");
        }

        // Prepare Interactive Drill kink if needed.
        if(szChartType!=NULL )
        {
            // Hyperlink to Interactive Drill.
            switch(ptTestCell->lPinmapIndex)
            {
                case GEX_PTEST:	//  Parametric
                case GEX_MPTEST:
                default:

                    if (iPageIndexType == GEX_INDEXPAGE_WAFERMAP)
                    {
                        strDrillArgument= "drill_3d=";
                        strDrillArgument += szChartType;
                        strDrillArgument += "--g=0--f=0--Test=";
                        strDrillArgument += ptTestCell->szTestLabel;
                        strDrillArgument += "--Pinmap=";
                        strDrillArgument += QString::number(ptTestCell->lPinmapIndex);
                    }
                    else
                    {
                        strDrillArgument= "drill_chart=";
                        strDrillArgument += szChartType;
                        strDrillArgument += "--data=";
                        strDrillArgument += ptTestCell->szTestLabel;
                    }
                    break;

                case GEX_FTEST:	// Functional
                    strDrillArgument= "drill_table=func";
                    break;
            }
        }

        // Write test name
        if(ptTestCell->strTestName.isEmpty() == false)
        {
            if(iPageIndexType != GEX_INDEXPAGE_ADVHISTOFUNCTIONAL){
                // Build test name
                BuildTestNameString(pFile,ptTestCell,szString);
                fprintf(hReportFile,"<td bgcolor=%s>%s",szDataColor,szString);
                if(szChartType!=NULL)
                    fprintf(hReportFile," <a href=\"#_gex_drill--%s\"><img src=\"../images/zoom_in.png\" border=\"0\">\n",strDrillArgument.toLatin1().constData());
                fprintf(hReportFile,"</td>");
            }else {
                BuildTestNameString(pFile,ptTestCell,szString);
                QString strFuncTestName;
                foreach(const QString &strPattern, ptTestCell->mVectors.keys()){
                    if(!ptTestCell->mVectors[strPattern].m_lstVectorInfo.count())
                        continue;
                    strFuncTestName += QString("%1 from pattern %2<br>").arg(szString).arg(strPattern);
                }
                if(!strFuncTestName.isEmpty()){
                    fprintf(hReportFile,"<td bgcolor=%s>%s",szDataColor,strFuncTestName.toLatin1().constData());
                    if(szChartType!=NULL)
                        fprintf(hReportFile," <a href=\"#_gex_drill--%s\"><img src=\"../images/zoom_in.png\" border=\"0\">\n",strDrillArgument.toLatin1().constData());
                    fprintf(hReportFile,"</td>");
                }
            }
        }
        else
        {
            // No name available...show a question mark
            fprintf(hReportFile,"<td>?");
            if(szChartType!=NULL )
                fprintf(hReportFile," <a href=\"#_gex_drill--%s\"><img src=\"../images/zoom_in.png\" border=\"0\">\n",strDrillArgument.toLatin1().constData());
            fprintf(hReportFile,"</td>");
        }

        // Close table line
        fprintf(hReportFile,"</tr>\n");

        iPageLinks++;

        // Point to next test cell
        NextTestCell:
        if(m_pReportOptions->getAdvancedReport() == GEX_ADV_FUNCTIONAL)
        {
            if((++iFuncTest)<mAdvancedTestsListToReport.count())
                ptTestCell = mAdvancedTestsListToReport.at(iFuncTest);
            else
                ptTestCell = 0;
        }
        else if(pqtStatisticsList)
        {
            ++lIndexTest;

            if (lIndexTest < pqtStatisticsList->count())
                ptTestCell = (*pqtStatisticsList)[lIndexTest];
            else
                ptTestCell = NULL;
        }
        else
            ptTestCell = ptTestCell->GetNextTest();
    };	// Loop until all test cells read.

    // In some cases (see below), we had links to the list
    switch(iPageIndexType)
    {
            case GEX_INDEXPAGE_ADVTREND:
            switch(m_pReportOptions->getAdvancedReportSettings())
            {
                case GEX_ADV_TREND_DIFFERENCE:	// Difference chart between 2 tests
                    lMessage = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("Difference between Tests: ");
                    // Write test # +hyperlink to its page
                    fprintf(hReportFile,"<tr><td width=\"160\" bgcolor=%s><b><a name=\"Tdifference\"></a> <a href=\"%s%d.htm#Tdifference\">%s</a> </b></td>",
                        szFieldColor,szHtmpPage,iLastPageUsed,lMessage.toLatin1().constData());
                    // No name available...show a question mark
                    fprintf(hReportFile,"<td>%s</td>",lMessage.toLatin1().constData());
                    // Close table line
                    fprintf(hReportFile,"</tr>\n");
                    break;
                case GEX_ADV_TREND_SOFTBIN_SBLOTS:	// Soft Bin trend chart (Sub-Lots)
                case GEX_ADV_TREND_SOFTBIN_PARTS:	// Soft Bin trend chart (all parts)
                case GEX_ADV_TREND_HARDBIN_SBLOTS:	// Hard Bin trend chart (Sub-Lots)
                case GEX_ADV_TREND_HARDBIN_PARTS:	// Hard Bin trend chart (all parts)
                case GEX_ADV_TREND_SOFTBIN_ROLLING:	// Soft Bin trend chart with Rolling Yield
                case GEX_ADV_TREND_HARDBIN_ROLLING:	// Hard Bin trend chart with Rolling Yield
                    lMessage = m_pReportOptions->pGexAdvancedRangeList->BuildTestListString("Bin#: ");
                    // Write test # +hyperlink to its page
                    iLastPageUsed = iPageLinks = 1;
                    fprintf(hReportFile,"<tr><td width=\"160\" bgcolor=%s><b><a name=\"Tbinning\"></a> <a href=\"%s%d.htm#Tbinning\">%s</a> </b></td>",
                        szFieldColor,szHtmpPage,iLastPageUsed,lMessage.toLatin1().constData());
                    // No name available...show a question mark
                    fprintf(hReportFile,"<td>%s</td>",lMessage.toLatin1().constData());
                    // Close table line
                    fprintf(hReportFile,"</tr>\n");
                    break;
            }
    }

    fprintf(hReportFile,"</table>\n");

    // If no test listed, show warning in report.
    if(iPageLinks == 0)
    {
        WriteInfoLine(GEX_T("*WARNING*"),GEX_T("No test results found for your test selection"));

        // If database SUMMARY used instead of data samples, this could be why we didn't find data!

        if(m_pReportOptions->bSpeedUseSummaryDB)
            WriteInfoLine(GEX_T("Possible cause:"),GEX_T("Only Database SUMMARY records were used (see 'Options' tab, section 'Speed optimization')"));
    }

    // Write mis-match info (if any). Eg: when compating groups and some tests only in one group...
    if(iPageIndexType == GEX_INDEXPAGE_STATS)
        WriteMismatchTestsPage();

    // close HTML page unless we're creating all reports in one flat page (for WORD, PDF creation)
    if (strOutputFormat=="HTML")
    {
        fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
        fprintf(hReportFile,"</body>\n");
        fprintf(hReportFile,"</html>\n");
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
// write the Statistics index page...
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_Stats(BOOL bValidSection, qtTestListStatistics	*pqtStatisticsList)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    if (strOutputFormat=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if(m_pReportOptions->isReportOutputHtmlBased())
    {
        static char	szString[2048];
        // Close the last statistics page...if any was created!
        if(bValidSection && (hReportFile!=NULL)
            && (strOutputFormat=="HTML")
            )
        {
            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            fprintf(hReportFile,C_HTML_FOOTER,GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");
            CloseReportFile();	// Close report file
        }

        // Now, create the index table!...if multiple group, include ALARM column.
        // Generating HTML report file.
        if (strOutputFormat=="HTML")
        {
            // Open <stdf-filename>/report/stats.htm
            sprintf(szString,"%s/pages/stats.htm",m_pReportOptions->strReportDirectory.toLatin1().constData());
            hReportFile = fopen(szString,"wt");
            if(hReportFile == NULL)
                return GS::StdLib::Stdf::ReportFile;

            // Create Test index page
            WriteTestListIndexPage(GEX_INDEXPAGE_STATS,bValidSection,pqtStatisticsList);

            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            CloseReportFile();	// Close report file
        }

        // Write page break (ignored if not writing a flat HTML document)
        if(m_pReportOptions->iStatsType != GEX_STATISTICS_DISABLED)
            WritePageBreak();
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Writes header line of labels in statistics page.
/////////////////////////////////////////////////////////////////////////////
void CGexReport::WriteStatisticsLabelLine(bool bInteractiveURL/*=false*/)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    QString strStatisticsFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"), QString("fields"))).toString();
    QStringList qslStatisticsFieldsToInclude = strStatisticsFieldsOptions.split(QString("|"));

    QString strStatisticsCompareFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"), QString("compare_fields"))).toString();
    QStringList qslStatisticsCompareFieldsList = strStatisticsCompareFieldsOptions.split(QString("|"));

    QString strStatisticsGageFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"), QString("gage_fields"))).toString();
    QStringList qslStatisticsGageFieldsList = strStatisticsGageFieldsOptions.split(QString("|"));

    QString strStatisticsAdvFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"), QString("adv_fields"))).toString();
    QStringList qslStatisticsAdvFieldsList = strStatisticsAdvFieldsOptions.split(QString("|"));

    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Test</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("test_name")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Name</b></td>\n",szFieldColor);
    if(m_pReportOptions->iGroups > 1)
    {
        QString strGlobalInfoBookmark;
        // Hyperlink to Global info page.
        if (strOutputFormat=="HTML")
            strGlobalInfoBookmark = "global.htm";
        else
            strGlobalInfoBookmark = "#all_globalinfo";
        if(bInteractiveURL)
            fprintf(hReportFile,"<td bgcolor=%s align=\"left\" ><b>Group</b></td>\n",szFieldColor);
        else
            fprintf(hReportFile,"<td bgcolor=%s align=\"left\" ><b><a href=\"%s\">Group</a></b></td>\n",szFieldColor,strGlobalInfoBookmark.toLatin1().constData());
    }
    if(qslStatisticsFieldsToInclude.contains(QString("test_type")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Type</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("limits")))
    {
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Low L.</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>High L.</b></td>\n",szFieldColor);
    }
    if(qslStatisticsFieldsToInclude.contains(QString("spec_limits")))
    {
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Low Spec.</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>High Spec.</b></td>\n",szFieldColor);
    }
    if(qslStatisticsFieldsToInclude.contains(QString("drift_limits")))
    {
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Test Vs Spec %%</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Low Test/Spec %%</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>High Test/Spec %%</b></td>\n",szFieldColor);
    }

    if(qslStatisticsAdvFieldsList.contains(QString("shape")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Shape</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("stats_source")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Source</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("exec_count")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Execs</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("fail_count")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Fails</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("fail_percent")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Fails %%</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("fail_bin")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Fail Bin#</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("test_flow_id")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Flow ID</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("removed_count")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Outliers</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("mean")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Mean</b></td>\n",szFieldColor);
    if((qslStatisticsCompareFieldsList.contains(QString("mean_shift"))) && (m_pReportOptions->iGroups > 1))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Mean Shift</b></td>\n",szFieldColor);
    if((qslStatisticsCompareFieldsList.contains(QString("t_test"))) && (m_pReportOptions->iGroups == 2))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>T-Test</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("sigma")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Sigma</b></td>\n",szFieldColor);
    if((qslStatisticsCompareFieldsList.contains(QString("sigma_shift"))) && (m_pReportOptions->iGroups > 1))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Sigma Shift</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("2sigma")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>2xSigma</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("3sigma")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>3xSigma</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("6sigma")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>6xSigma</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("min")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Min</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("max")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Max</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("range")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Range</b></td>\n",szFieldColor);
    if((qslStatisticsCompareFieldsList.contains(QString("max_range"))) && (m_pReportOptions->iGroups > 1))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Max. Range</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("cp")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Cp</b></td>\n",szFieldColor);
    // GCORE-199
    if(qslStatisticsFieldsToInclude.contains(QString("cr")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Cr</b></td>\n",szFieldColor);

    if((qslStatisticsCompareFieldsList.contains(QString("cp_shift"))) && (m_pReportOptions->iGroups > 1))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Cp Shift</b></td>\n",szFieldColor);
    if((qslStatisticsCompareFieldsList.contains(QString("cr_shift"))) && (m_pReportOptions->iGroups > 1))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Cr Shift</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("cpk")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Cpk</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("cpk_l")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Cpl</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("cpk_h")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Cpu</b></td>\n",szFieldColor);
    if((qslStatisticsCompareFieldsList.contains(QString("cpk_shift"))) && (m_pReportOptions->iGroups > 1))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Cpk Shift</b></td>\n",szFieldColor);
    if(qslStatisticsFieldsToInclude.contains(QString("yield")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Yield</b></td>\n",szFieldColor);
    if(qslStatisticsGageFieldsList.contains(QString("ev")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Gage EV</b></td>\n",szFieldColor);
    if(qslStatisticsGageFieldsList.contains(QString("av")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Gage AV</b></td>\n",szFieldColor);
    if(qslStatisticsGageFieldsList.contains(QString("r&r")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Gage R&R</b></td>\n",szFieldColor);
    if(qslStatisticsGageFieldsList.contains(QString("gb")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Gage GB</b></td>\n",szFieldColor);
    if(qslStatisticsGageFieldsList.contains(QString("pv")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Gage PV</b></td>\n",szFieldColor);
    if(qslStatisticsGageFieldsList.contains(QString("tv")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Gage TV</b></td>\n",szFieldColor);
    if(qslStatisticsGageFieldsList.contains(QString("p_t")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Gage P/T</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("skew")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Skew</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("kurtosis")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>Kurtosis</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("P0.5")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>P0.5%%</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("P2.5")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>P2.5%%</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("P10")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>P10%%</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("quartile1")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>P25%% - Q1</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("quartile2")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>P50%% - Median</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("quartile3")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>P75%% - Q3</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("P90")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>P90%%</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("P97.5")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>P97.5%%</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("P99.5")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>P99.5%%</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("inter_quartiles")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>IQR</b></td>\n",szFieldColor);
    if(qslStatisticsAdvFieldsList.contains(QString("sigma_inter_quartiles")))
        fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b>IQR SD</b></td>\n",szFieldColor);
    fprintf(hReportFile,"</tr>\n");
}

void CGexReport::WriteStatsLinesEx(
    CGexGroupOfFiles * pGroup,
    CGexFileInGroup * pFile,
    uint uGroupID,
    CTest * ptTestReferenceCell,
    CTest * ptOtherTestCell,
    int& iLine,
    const QString &strStatisticsFieldsOptions,
    bool bMeanShiftAlarmOnly/*=false*/,
    bool bSigmaShiftAlarmOnly/*=false*/)
{
    double				lfLimitSpace=0;
    double				lfPercent=0;
    double				fData=0;
    QString				strString;
    QString				strBookmark;
    QString				strBackgroundColor;	// Used to define bacground color (Std, or Alarm!)
    QColor				cBackgroundColor;
    char				szString[2048]="";
    char				szTestName[1024]="";
    const char *		ptChar=0;
    bool				bReportTest=false;
    bool				bInteractiveURL=false;
    QString strBoxplotShiftOver = m_pReportOptions->GetOption("adv_boxplot", "delta").toString();
    bool bBoxplotShiftOverTV = (strBoxplotShiftOver == "over_tv");
    QString scaling = m_pReportOptions->GetOption("dataprocessing","scaling").toString();
    bool isScalingNormalized=(scaling=="normalized")?true:false;
    double lfOptionStorageDevice;
    bool bGetOptionRslt;

    if (!ptOtherTestCell)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Invalid pointer to test!").toLatin1().data());
        return;
    }

    if (ptOtherTestCell->GetCurrentLimitItem() == NULL)
    {
        return;
    }

    if((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0)
    {
        fData = ptOtherTestCell->GetCurrentLimitItem()->lfHighLimit; // High limit exists
        // If we have to keep values in normalized format, do not rescale!
        if (!isScalingNormalized)
        {
            // convert HighLimit to same scale as results:
            pFile->FormatTestResultNoUnits(&fData,ptOtherTestCell->hlm_scal);
            fData *=  ScalingPower(ptOtherTestCell->hlm_scal);	// normalized
            //fData /=  ScalingPower(ptOtherTestCell->res_scal);	// normalized
        }
        lfLimitSpace = fData;

        fData = ptOtherTestCell->GetCurrentLimitItem()->lfLowLimit;		// Low limit exists
        // If we have to keep values in normalized format, do not rescale!
        if (!isScalingNormalized)
        {
            // convert LowLimit to same scale as results:
            pFile->FormatTestResultNoUnits(&fData,ptOtherTestCell->llm_scal);
            fData *=  ScalingPower(ptOtherTestCell->llm_scal);	// normalized
            //fData /=  ScalingPower(ptOtherTestCell->res_scal);	// normalized
        }

        lfLimitSpace -= fData;
    }
    else
        lfLimitSpace = 0.0;

    TestShift lTestShift;
    GS::Core::MLShift lMLShift;

    if (ptOtherTestCell->mTestShifts.size() > 0)
    {
        lTestShift = ptOtherTestCell->mTestShifts.first();
        lMLShift = lTestShift.GetMlShift(ptOtherTestCell->GetCurrentLimitItem());
    }

    // Check if must only report tests with shift alarm...
    if((uGroupID == 0) && (getGroupsList().count() > 1))
    {
        bReportTest = true;
        if(bMeanShiftAlarmOnly)
        {
            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_mean")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            bGetOptionRslt = ((fabs(lMLShift.mMeanShiftPct) >= lfOptionStorageDevice) && (lfOptionStorageDevice >= 0));
            if(bGetOptionRslt)
                bReportTest = true;
            else
                bReportTest = false;	// No high mean shift error for this test!
        }
        if(bSigmaShiftAlarmOnly)
        {
            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_sigma")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            bGetOptionRslt = ((fabs(lTestShift.mSigmaShiftPercent) >= lfOptionStorageDevice) && (lfOptionStorageDevice >= 0));
            if(bGetOptionRslt)
                bReportTest = true;
            else
                bReportTest = false;	// No high mean shift error for this test!
        }
        if(bReportTest == false)
            return;	// This test doesn't fail the alarms selected...do not report it!
    }

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    QStringList qslStatisticsFieldsList = strStatisticsFieldsOptions.split(QString("|"));

    QString strStatisticsCompareFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"),
                                                                             QString("compare_fields"))).toString();
    QStringList qslStatisticsCompareFieldsList = strStatisticsCompareFieldsOptions.split(QString("|"));

    QString strStatisticsGageFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"),
                                                                          QString("gage_fields"))).toString();
    QStringList qslStatisticsGageFieldsList = strStatisticsGageFieldsOptions.split(QString("|"));

    QString strStatisticsAdvFieldsOptions = (m_pReportOptions->GetOption(QString("statistics"),
                                                                         QString("adv_fields"))).toString();
    QStringList qslStatisticsAdvFieldsList = strStatisticsAdvFieldsOptions.split(QString("|"));

    int lDistributionType;

    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (lPatInfo)
        lDistributionType = patlib_GetDistributionType(ptOtherTestCell,
                                                       lPatInfo->GetRecipeOptions().iCategoryValueCount,
                                                       lPatInfo->GetRecipeOptions().bAssumeIntegerCategory,
                                                       lPatInfo->GetRecipeOptions().mMinConfThreshold);
    else
        lDistributionType = patlib_GetDistributionType(ptOtherTestCell);

    // Write report line
    if (strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"%s,",ptOtherTestCell->szTestLabel);

        if(qslStatisticsFieldsList.contains(QString("test_name")))
        {
            BuildTestNameString(pFile,ptOtherTestCell,szTestName);
            fprintf(hReportFile,"%s,",szTestName);
        }
        if(m_pReportOptions->iGroups > 1)
            fprintf(hReportFile,"%s,",pGroup->strGroupName.toLatin1().constData());
        if(qslStatisticsFieldsList.contains(QString("test_type")))
        {
            BuildTestTypeString(pFile,ptOtherTestCell,szString,true);
            fprintf(hReportFile,"%s,",szString);
        }
        if(qslStatisticsFieldsList.contains(QString("limits")))
        {
            fprintf(hReportFile,"%s,",ptOtherTestCell->GetCurrentLimitItem()->szLowL);
            fprintf(hReportFile,"%s,",ptOtherTestCell->GetCurrentLimitItem()->szHighL);
        }
        if(qslStatisticsFieldsList.contains(QString("spec_limits")))
        {
            fprintf(hReportFile,"%s,",ptOtherTestCell->szLowSpecL);
            fprintf(hReportFile,"%s,",ptOtherTestCell->szHighSpecL);
        }
        if(qslStatisticsFieldsList.contains(QString("drift_limits")))
        {
            if(((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) &&
                ((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) &&
                ((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0) &&
                ((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0))
            {
                lfLimitSpace = (ptOtherTestCell->GetCurrentLimitItem()->lfHighLimit-ptOtherTestCell->GetCurrentLimitItem()->lfLowLimit);
                fData = ptOtherTestCell->lfHighSpecLimit-ptOtherTestCell->lfLowSpecLimit;
                if(fData)
                    fData = (lfLimitSpace / fData)*100.0;
                if(fData >1000)
                    fData = 999.99;	// Clamp % value in case very high!
                fprintf(hReportFile,"%.2lf %%",fData);
            }
            else
                fprintf(hReportFile,"n/a,");

            // LOW Test/Specs drift %
            if(((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) &&
                ((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0))
            {
                fData = 999.99;
                if(ptOtherTestCell->lfLowSpecLimit)
                    fData = (ptOtherTestCell->GetCurrentLimitItem()->lfLowLimit / ptOtherTestCell->lfLowSpecLimit)*100.0;
                if(fData >1000)
                    fData = 999.99;	// Clamp % value in case very high!
                fprintf(hReportFile,"%.2lf %%",fData);
            }
            else
                fprintf(hReportFile,"n/a,");

            // HIGH Test/Specs drift %
            if(((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) &&
                ((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0))
            {
                fData = 999.99;
                if(ptOtherTestCell->lfHighSpecLimit)
                    fData = (ptOtherTestCell->GetCurrentLimitItem()->lfHighLimit / ptOtherTestCell->lfHighSpecLimit)*100.0;
                if(fData >1000)
                    fData = 999.99;	// Clamp % value in case very high!
                fprintf(hReportFile,"%.2lf %%",fData);
            }
            else
                fprintf(hReportFile,"n/a,");
        }
        if(qslStatisticsAdvFieldsList.contains(QString("shape")))
            fprintf(hReportFile,"%s,", patlib_GetDistributionName(lDistributionType).toLatin1().constData());
        if(qslStatisticsFieldsList.contains(QString("stats_source")))
            fprintf(hReportFile,"%s,", (ptOtherTestCell->bStatsFromSamples) ? "Samples":"Summary");
        if(qslStatisticsFieldsList.contains(QString("exec_count")))
            fprintf(hReportFile,"%s,",CreateResultString(ptOtherTestCell->ldExecs));
        if(qslStatisticsFieldsList.contains(QString("fail_count")))
            fprintf(hReportFile,"%s,",CreateResultString(ptOtherTestCell->GetCurrentLimitItem()->ldFailCount));
        if(qslStatisticsFieldsList.contains(QString("fail_percent")))
        {
            if(ptOtherTestCell->ldSamplesValidExecs > 0)
                ptChar = CreateResultStringPercent(100.0*ptOtherTestCell->GetCurrentLimitItem()->ldFailCount/ptOtherTestCell->ldSamplesValidExecs);
            else
                ptChar = GEX_NA;
            fprintf(hReportFile,"%s,",ptChar);
        }
        if(qslStatisticsFieldsList.contains(QString("fail_bin")))
        {
            if(ptOtherTestCell->iFailBin >= 0)
                fprintf(hReportFile,"%d,",ptOtherTestCell->iFailBin);
            else
                fprintf(hReportFile,"n/a,");
        }
        if(qslStatisticsFieldsList.contains(QString("test_flow_id")))
            fprintf(hReportFile,"%d,",ptOtherTestCell->lTestFlowID);
        if(qslStatisticsFieldsList.contains(QString("removed_count")))
            fprintf(hReportFile,"%s,",CreateResultString(ptOtherTestCell->GetCurrentLimitItem()->ldOutliers));
        if(qslStatisticsFieldsList.contains(QString("mean")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfMean,ptOtherTestCell->res_scal));
        }

        if(m_pReportOptions->iGroups > 1)
        {
            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_mean")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            bGetOptionRslt =(fabs(lMLShift.mMeanShiftPct) >= lfOptionStorageDevice) && (lfOptionStorageDevice >= 0);
            if( uGroupID && bGetOptionRslt && ptOtherTestCell->bTestType != 'F')
            {
                // Mark this test with its Alarm bit set
                ptTestReferenceCell->iAlarm |= GEX_ALARM_MEANSHIFT;
                if(qslStatisticsCompareFieldsList.contains(QString("mean_shift")))
                    fprintf(hReportFile,"*F* ");
            }
            if(qslStatisticsCompareFieldsList.contains(QString("mean_shift")))
            {
                if (ptOtherTestCell->bTestType == 'F')
                    fprintf(hReportFile,"n/a,");
                else if(!uGroupID)
                    fprintf(hReportFile,"0.0 (0.00 %%),");
                else
                    fprintf(hReportFile,"%s (%.2f %%),", pFile->FormatTestResult(
                                ptOtherTestCell, lTestShift.mMeanShiftValue, ptOtherTestCell->res_scal, false),
                            lMLShift.mMeanShiftPct);
            }
        }
        if( (qslStatisticsCompareFieldsList.contains(QString("t_test"))) && (m_pReportOptions->iGroups == 2))
            fprintf(hReportFile,"%g,",ptOtherTestCell->getP_Value());
        if(qslStatisticsFieldsList.contains(QString("sigma")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSigma,ptOtherTestCell->res_scal));
        }

        if(m_pReportOptions->iGroups > 1)
        {
            // Check if Sigma increased over the limit...
            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_sigma")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            bGetOptionRslt = (fabs(lTestShift.mSigmaShiftPercent) >= lfOptionStorageDevice) && (lfOptionStorageDevice >= 0);
            if(uGroupID && bGetOptionRslt && (ptOtherTestCell->bTestType != 'F') )
            {
                // Mark this test with its Alarm bit set
                ptTestReferenceCell->iAlarm |= GEX_ALARM_SIGMASHIFT;
                if(qslStatisticsCompareFieldsList.contains(QString("sigma_shift")))
                    fprintf(hReportFile,"*F* ");
            }
            if(qslStatisticsCompareFieldsList.contains(QString("sigma_shift")))
            {
                if (ptOtherTestCell->bTestType == 'F')
                    fprintf(hReportFile,"n/a,");
                else if(!uGroupID)
                    fprintf(hReportFile,"0.0 (0.00 %%),");
                else
                    fprintf(hReportFile,"%s (%.2f %%),", pFile->FormatTestResult(
                                ptOtherTestCell, lTestShift.mSigmaShiftValue, ptOtherTestCell->res_scal, false),
                            lTestShift.mSigmaShiftPercent);
            }
        }
        if(qslStatisticsFieldsList.contains(QString("2sigma")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,2*ptOtherTestCell->lfSigma,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsFieldsList.contains(QString("3sigma")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,3*ptOtherTestCell->lfSigma,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsFieldsList.contains(QString("6sigma")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,6*ptOtherTestCell->lfSigma,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsFieldsList.contains(QString("min")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfMin,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsFieldsList.contains(QString("max")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfMax,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsFieldsList.contains(QString("range")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfRange,ptOtherTestCell->res_scal));
        }
        if((qslStatisticsCompareFieldsList.contains(QString("max_range"))) && (m_pReportOptions->iGroups > 1))
        {
            // Only display Max. Range on first group line.
            if(!uGroupID && ptOtherTestCell->bTestType != 'F')
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptTestReferenceCell,ptTestReferenceCell->lfMaxRange,ptTestReferenceCell->res_scal));
            else
                fprintf(hReportFile,",,");
        }
        if(qslStatisticsFieldsList.contains(QString("cp")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,");
            else
                fprintf(hReportFile,"%s,", CreateResultStringCpCrCpk(ptOtherTestCell->GetCurrentLimitItem()->lfCp));
        }
        if(qslStatisticsFieldsList.contains(QString("cr"))) // GCORE-199
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,"); // Functional test : just 'n/a'
            else
            {
                const char* lCpString=CreateResultStringCpCrCpk(ptOtherTestCell->GetCurrentLimitItem()->lfCp);
                if (strcmp(lCpString, GEX_NA)==0
                    || (strcmp(lCpString, "")==0)
                    || ptOtherTestCell->GetCurrentLimitItem()->lfCp == 0.0
                    || ptOtherTestCell->GetCurrentLimitItem()->lfCp == -0.0
                    || ptOtherTestCell->GetCurrentLimitItem()->lfCp == C_NO_CP_CPK
                    )
                    fprintf(hReportFile,"%s,", GEX_NA); // 'n/a .'
                else
                    fprintf(hReportFile,"%s,", CreateResultStringCpCrCpk(1/ptOtherTestCell->GetCurrentLimitItem()->lfCp));
            }
        }
        if((qslStatisticsCompareFieldsList.contains(QString("cp_shift"))) && (m_pReportOptions->iGroups > 1))
        {
            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_cp")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            if(bGetOptionRslt)
            {
                bGetOptionRslt = (lfOptionStorageDevice>=0) && ((fabs(lMLShift.mCpShift)) >= lfOptionStorageDevice);

                if(uGroupID && bGetOptionRslt && ptOtherTestCell->bTestType != 'F')
                {
                    // Mark this test with its Alarm bit set
                    ptTestReferenceCell->iAlarm |= GEX_ALARM_CPSHIFT;
                    if(qslStatisticsCompareFieldsList.contains(QString("cp_shift")))
                        fprintf(hReportFile,"*F* ");
                }
            }

            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile, "n/a,");
            else if(!uGroupID)
                fprintf(hReportFile,"0.0 %%,");
            else
                fprintf(hReportFile,"%s %%,",CreateResultStringCpCrCpkShift(lMLShift.mCpShift));
        }
        if((qslStatisticsCompareFieldsList.contains(QString("cr_shift"))) && (m_pReportOptions->iGroups > 1))
        {
            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_cr")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            if(bGetOptionRslt)
            {
                bGetOptionRslt = (lfOptionStorageDevice>=0) && ((fabs(lMLShift.mCrShift)) >= lfOptionStorageDevice);

                if(uGroupID && bGetOptionRslt && ptOtherTestCell->bTestType != 'F')
                {
                    // Mark this test with its Alarm bit set
                    ptTestReferenceCell->iAlarm |= GEX_ALARM_CRSHIFT;
                    if(qslStatisticsCompareFieldsList.contains(QString("cr_shift")))
                        fprintf(hReportFile,"*F* ");
                }
            }

            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile, "n/a,");
            else if(!uGroupID)
                fprintf(hReportFile,"0.0 %%,");
            else
                fprintf(hReportFile,"%s %%,",CreateResultStringCpCrCpkShift(lMLShift.mCrShift));
        }
        if(qslStatisticsFieldsList.contains(QString("cpk")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,");
            else
                fprintf(hReportFile,"%s,",CreateResultStringCpCrCpk(ptOtherTestCell->GetCurrentLimitItem()->lfCpk));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("cpk_l")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,");
            else
                fprintf(hReportFile,"%s,",CreateResultStringCpCrCpk(ptOtherTestCell->GetCurrentLimitItem()->lfCpkLow));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("cpk_h")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,");
            else
                fprintf(hReportFile,"%s,",CreateResultStringCpCrCpk(ptOtherTestCell->GetCurrentLimitItem()->lfCpkHigh));
        }
        if((qslStatisticsCompareFieldsList.contains(QString("cpk_shift"))) && (m_pReportOptions->iGroups > 1))
        {
            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_cpk")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            if(bGetOptionRslt)
            {
                bGetOptionRslt = (lfOptionStorageDevice>=0) && ((fabs(lMLShift.mCpkShift)) >= lfOptionStorageDevice);

                if(uGroupID && bGetOptionRslt && ptOtherTestCell->bTestType != 'F')
                {
                    // Mark this test with its Alarm bit set
                    ptTestReferenceCell->iAlarm |= GEX_ALARM_CPKSHIFT;
                    if(qslStatisticsCompareFieldsList.contains(QString("cpk_shift")))
                        fprintf(hReportFile,"*F* ");
                }
            }

            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,");
            else if(!uGroupID)
                fprintf(hReportFile,"0.00 %%,");
            else
                fprintf(hReportFile,"%s %%,",CreateResultStringCpCrCpkShift(lMLShift.mCpkShift));
        }
        if(qslStatisticsFieldsList.contains(QString("yield")))
        {
            if(ptOtherTestCell->ldExecs > 0)
                ptChar = CreateResultStringPercent(100.0 -(100.0*ptOtherTestCell->GetCurrentLimitItem()->ldFailCount/ptOtherTestCell->ldExecs));
            else
                ptChar = GEX_NA;
            fprintf(hReportFile,"%s,",ptChar);
        }
        if(qslStatisticsGageFieldsList.contains(QString("ev")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfEV,fData,lfPercent);
            }
            fprintf(hReportFile,"%s,",strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("av")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfAV,fData,lfPercent);
            }
            fprintf(hReportFile,"%s,",strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("r&r")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfRR,fData,lfPercent);
            }
            fprintf(hReportFile,"%s,",strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("pv")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfPV,fData,lfPercent);
            }
            fprintf(hReportFile,"%s,",strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("gb")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfGB,-1,lfPercent);
            }
            fprintf(hReportFile,"%s,",strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("tv")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfTV,fData,lfPercent);
            }
            fprintf(hReportFile,"%s,",strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("p_t")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
                strString = " ";
            else
                strString = QString::number(ptOtherTestCell->pGage->lfP_T,'f',2) + " %";
            fprintf(hReportFile,"%s,",strString.toLatin1().constData());
        }
        if(qslStatisticsAdvFieldsList.contains(QString("skew")))
        {
            if((ptOtherTestCell->lfSamplesSkew != -C_INFINITE) && (ptOtherTestCell->bTestType != 'F'))
                fprintf(hReportFile,"%g,",ptOtherTestCell->lfSamplesSkew);
            else
                fprintf(hReportFile,"n/a,");
        }
        if(qslStatisticsAdvFieldsList.contains(QString("kurtosis")))
        {
            if(ptOtherTestCell->lfSamplesKurt != -C_INFINITE && ptOtherTestCell->bTestType != 'F')
                fprintf(hReportFile,"%g,",ptOtherTestCell->lfSamplesKurt);
            else
                fprintf(hReportFile,"n/a,");
        }

        if(qslStatisticsAdvFieldsList.contains(QString("P0.5")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP0_5,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P2.5")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP2_5,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P10")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP10,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("quartile1")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesQuartile1,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("quartile2")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesQuartile2,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("quartile3")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesQuartile3,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P90")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP90,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P97.5")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP97_5,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P99.5")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP99_5,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("inter_quartiles")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesQuartile3-ptOtherTestCell->lfSamplesQuartile1,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("sigma_inter_quartiles")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"n/a,n/a,");
            else
                fprintf(hReportFile,"%s,",pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesSigmaInterQuartiles,ptOtherTestCell->res_scal));
        }

        fprintf(hReportFile,"\n");
    }
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    else if(m_pReportOptions->isReportOutputHtmlBased())
    {
        // Generating .HTML report file.
        bInteractiveURL = false;
        if(bMeanShiftAlarmOnly && bSigmaShiftAlarmOnly)
            bInteractiveURL = true;	// Do not include URLs if this report is created for a pre-PAT checker report.

        // Check if need to create new HTML Histo. page
        CheckForNewHtmlPage(NULL,SECTION_STATS,ptTestReferenceCell->iHtmlStatsPage);

        if(iLine == 0)
        {
            // Insert the line labels every 15 lines...eases reading reports! (or after each test if comparing test in files)
            WriteStatisticsLabelLine(bInteractiveURL);
            iLine++;	// Just to make sure we do not write the header in the middle of a same test serie from different groups!
        }

        fprintf(hReportFile,"<tr>\n");
        if (m_pReportOptions->getAdvancedReport() == GEX_ADV_CANDLE_MEANRANGE &&
                 ptTestReferenceCell->iHtmlAdvancedPage &&
                 ptTestReferenceCell->bTestType != 'F')
        {
            // Bookmark: are in same page if FLAT HTML page is generated
            if (strOutputFormat=="HTML")
                strBookmark.sprintf("advanced%d.htm#AdvT", ptTestReferenceCell->iHtmlAdvancedPage);
            else
                strBookmark = "#AdvT";	// Advanced section of Test bookmark header string.

            // If this test has a advanced page, create the hyperlink!
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b><a name=\"StatT%s\"></a> <a href=\"%s%s\">%s</a></b></td>\n",
                szFieldColor,ptOtherTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptOtherTestCell->szTestLabel,ptOtherTestCell->szTestLabel);
        }
        else if(ptTestReferenceCell->iHtmlHistoPage && ptTestReferenceCell->bTestType != 'F')
        {
            // Bookmark: are in same page if FLAT HTML page is generated
            if (strOutputFormat=="HTML")
                strBookmark.sprintf("histogram%d.htm#HistoT",ptTestReferenceCell->iHtmlHistoPage);
            else
                strBookmark = "#HistoT";	// Histogram of Test bookmark header string.
            if(bInteractiveURL)
                strBookmark = "#_gex_drill--drill_chart=adv_histo--data=";

            // If this test has a histogram page, create the hyperlink!
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b><a name=\"StatT%s\"></a> <a href=\"%s%s\">%s</a></b></td>\n",
                szFieldColor,ptOtherTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptOtherTestCell->szTestLabel,ptOtherTestCell->szTestLabel);
        }
        else
        {
            // Test doesn't have a Histogram/or histogram disabled/or functional test : don't create a hyperlink
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" ><b><a name=\"StatT%s\">%s</a></b></td>\n",
                szFieldColor,ptOtherTestCell->szTestLabel,ptOtherTestCell->szTestLabel);
        }
        if(qslStatisticsFieldsList.contains(QString("test_name")))
        {
            BuildTestNameString(pFile,ptOtherTestCell,szTestName);
            fprintf(hReportFile,"<td bgcolor=%s align=\"left\" >%s</td>\n",szDataColor,buildDisplayName(szTestName).toLatin1().constData());
        }
        if(m_pReportOptions->iGroups > 1)
            fprintf(hReportFile,"<td bgcolor=%s align=\"left\" >%s</td>\n",szDataColor,pGroup->strGroupName.toLatin1().constData());
        if(qslStatisticsFieldsList.contains(QString("test_type")))
        {
            char	szString[2048];
            BuildTestTypeString(pFile,ptOtherTestCell,szString,true);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,szString);
        }
        if(qslStatisticsFieldsList.contains(QString("limits")))
        {
            // If the limit is custom (from a 'what-if', then changed its background color.
            // // bit0=1 (no low limit forced), bit1=1 (no high limit forced)
            if(ptOtherTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL)
                ptChar = szDataColor;	// Standard color (not a custom limit)
            else
                ptChar = szDrillColorIf;// Custom color (custom limit from 'what-if')
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",
                    ptChar,ptOtherTestCell->GetCurrentLimitItem()->szLowL);
            if(ptOtherTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL)
                ptChar = szDataColor;	// Standard color (not a custom limit)
            else
                ptChar = szDrillColorIf;// Custom color (custom limit from 'what-if')
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",
                    ptChar,ptOtherTestCell->GetCurrentLimitItem()->szHighL);
        }
        if(qslStatisticsFieldsList.contains(QString("spec_limits")))
        {
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,ptOtherTestCell->szLowSpecL);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,ptOtherTestCell->szHighSpecL);
        }
        if(qslStatisticsFieldsList.contains(QString("drift_limits")))
        {
            strcpy(szString, GEX_NA);
            if(((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) &&
                ((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) &&
                ((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0) &&
                ((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0))
            {
                lfLimitSpace = (ptOtherTestCell->GetCurrentLimitItem()->lfHighLimit-ptOtherTestCell->GetCurrentLimitItem()->lfLowLimit);
                fData = ptOtherTestCell->lfHighSpecLimit-ptOtherTestCell->lfLowSpecLimit;
                if(fData)
                    fData = (lfLimitSpace / fData)*100.0;
                if(fData >1000)
                    fData = 999.99;	// Clamp % value in case very high!
                sprintf(szString,"%.2lf %%",fData);
            }
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,szString);

            // LOW Test/Spec drift %
            strcpy(szString, GEX_NA);
            if(((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) &&
                ((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0))
            {
                fData = 999.99;
                if(ptOtherTestCell->lfLowSpecLimit)
                    fData = (ptOtherTestCell->GetCurrentLimitItem()->lfLowLimit / ptOtherTestCell->lfLowSpecLimit)*100.0;
                if(fData >1000)
                    fData = 999.99;	// Clamp % value in case very high!
                sprintf(szString,"%.2lf %%",fData);
            }
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,szString);

            // HIGH Test/Spec drift %
            strcpy(szString, GEX_NA);
            if(((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) &&
                ((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0))
            {
                fData = 999.99;
                if(ptOtherTestCell->lfHighSpecLimit)
                    fData = (ptOtherTestCell->GetCurrentLimitItem()->lfHighLimit / ptOtherTestCell->lfHighSpecLimit)*100.0;
                if(fData >1000)
                    fData = 999.99;	// Clamp % value in case very high!
                sprintf(szString,"%.2lf %%",fData);
            }
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,szString);
        }
        if(qslStatisticsAdvFieldsList.contains(QString("shape")))
            fprintf(hReportFile,
                    "<td bgcolor=%s align=\"center\" >%s</td>\n",
                    szDataColor,
                    patlib_GetDistributionName(lDistributionType).toLatin1().constData());
        if(qslStatisticsFieldsList.contains(QString("stats_source")))
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, (ptOtherTestCell->bStatsFromSamples) ? "Samples":"Summary");
        if(qslStatisticsFieldsList.contains(QString("exec_count")))
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,CreateResultString(ptOtherTestCell->ldExecs));
        if(qslStatisticsFieldsList.contains(QString("fail_count")))
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,CreateResultString(ptOtherTestCell->GetCurrentLimitItem()->ldFailCount));
        if(qslStatisticsFieldsList.contains(QString("fail_percent")))
        {
            if(ptOtherTestCell->ldSamplesValidExecs > 0)
                ptChar = CreateResultStringPercent(100.0*ptOtherTestCell->GetCurrentLimitItem()->ldFailCount/ptOtherTestCell->ldSamplesValidExecs);
            else
                ptChar = GEX_NA;
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,ptChar);
        }
        if(qslStatisticsFieldsList.contains(QString("fail_bin")))
        {
            if(ptOtherTestCell->iFailBin >= 0)
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%d</td>\n",szDataColor,ptOtherTestCell->iFailBin);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
        }
        if(qslStatisticsFieldsList.contains(QString("test_flow_id")))
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%d</td>\n",szDataColor,ptOtherTestCell->lTestFlowID);
        if(qslStatisticsFieldsList.contains(QString("removed_count")))
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,CreateResultString(ptOtherTestCell->GetCurrentLimitItem()->ldOutliers));
        if(qslStatisticsFieldsList.contains(QString("mean")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n", szDataColor, GEX_NA);
            else
            {
                // Highlight in red if mean outside of limits!
                if( ptOtherTestCell->lfMean != GEX_C_DOUBLE_NAN &&
                    ((((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) && (ptOtherTestCell->lfMean < ptOtherTestCell->GetCurrentLimitItem()->lfLowLimit)) ||
                    (((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) && (ptOtherTestCell->lfMean > ptOtherTestCell->GetCurrentLimitItem()->lfHighLimit))) )
                    strBackgroundColor = szAlarmColor;
                else
                    strBackgroundColor = szDataColor;
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",strBackgroundColor.toLatin1().constData(),pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfMean,ptOtherTestCell->res_scal));
            }
        }

        TestShift lTestShift;
        GS::Core::MLShift lMLShift;
        if (ptOtherTestCell->mTestShifts.size() > 0)
        {
            lTestShift = ptOtherTestCell->mTestShifts.first();
            lMLShift = lTestShift.GetMlShift(ptOtherTestCell->GetCurrentLimitItem());
        }

        if(m_pReportOptions->iGroups > 1)
        {
            strBackgroundColor = szDataColor;	// Default: no alarm, all is fine

            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_mean")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            bGetOptionRslt = (lfOptionStorageDevice>=0) && (fabs(lMLShift.mMeanShiftPct) >= lfOptionStorageDevice);
            if(uGroupID && bGetOptionRslt && ptTestReferenceCell->bTestType != 'F')
            {
                // Mark this test with its Alarm bit set
                ptTestReferenceCell->iAlarm |= GEX_ALARM_MEANSHIFT;
                ptOtherTestCell->iAlarm |= GEX_ALARM_MEANSHIFT;
                strBackgroundColor = szAlarmColor;	// ALARM!: Mean shift over limit.
            }
            if(qslStatisticsCompareFieldsList.contains(QString("mean_shift")))
            {
                if (ptTestReferenceCell->bTestType == 'F')
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n", szDataColor, GEX_NA);
                else if(!uGroupID)
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >0.00 %%</td>\n",strBackgroundColor.toLatin1().constData());
                else
                {
                    if(lMLShift.mMeanShiftPct > C_INFINITE_PERCENTAGE)
                        lMLShift.mMeanShiftPct = C_INFINITE_PERCENTAGE;

                    if(lMLShift.mMeanShiftPct < -C_INFINITE_PERCENTAGE)
                        lMLShift.mMeanShiftPct = -C_INFINITE_PERCENTAGE;

                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s (%.2f %%)</td>\n",
                            strBackgroundColor.toLatin1().constData(),
                            pFile->FormatTestResult(ptOtherTestCell, lTestShift.mMeanShiftValue,ptOtherTestCell->res_scal),
                            lMLShift.mMeanShiftPct);
                }
            }
        }
        if( (qslStatisticsCompareFieldsList.contains(QString("t_test"))) && (m_pReportOptions->iGroups == 2) )
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%g</td>\n",szDataColor,ptOtherTestCell->getP_Value());
        if(qslStatisticsFieldsList.contains(QString("sigma")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",
                        szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSigma,ptOtherTestCell->res_scal));
        }
        if(m_pReportOptions->iGroups > 1)
        {
            strBackgroundColor = szDataColor;	// Default: no alarm, all is fine

            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_sigma")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            bGetOptionRslt = (lfOptionStorageDevice>=0) && (fabs(lTestShift.mSigmaShiftPercent) >= lfOptionStorageDevice);
            if(uGroupID && bGetOptionRslt && (ptOtherTestCell->bTestType != 'F') )
            {
                // Mark this test with its Alarm bit set
                ptTestReferenceCell->iAlarm |= GEX_ALARM_SIGMASHIFT;
                ptOtherTestCell->iAlarm |= GEX_ALARM_SIGMASHIFT;
                strBackgroundColor = szAlarmColor;	// ALARM!: Mean shift over limit.
            }
            if(qslStatisticsCompareFieldsList.contains(QString("sigma_shift")))
            {
                if (ptOtherTestCell->bTestType == 'F')
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n", szDataColor, GEX_NA);
                else if(!uGroupID)
                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >0.00 %%</td>\n",strBackgroundColor.toLatin1().constData());
                else
                {
                    if(lTestShift.mSigmaShiftPercent > C_INFINITE_PERCENTAGE)
                        lTestShift.mSigmaShiftPercent = C_INFINITE_PERCENTAGE;

                    if(lTestShift.mSigmaShiftPercent < -C_INFINITE_PERCENTAGE)
                        lTestShift.mSigmaShiftPercent = -C_INFINITE_PERCENTAGE;

                    fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s (%.2f %%)</td>\n",
                            strBackgroundColor.toLatin1().constData(),
                            pFile->FormatTestResult(ptOtherTestCell, lTestShift.mSigmaShiftValue,ptOtherTestCell->res_scal)
                            , lTestShift.mSigmaShiftPercent);
                }
            }
        }
        if(qslStatisticsFieldsList.contains(QString("2sigma")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,2*ptOtherTestCell->lfSigma,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsFieldsList.contains(QString("3sigma")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,3*ptOtherTestCell->lfSigma,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsFieldsList.contains(QString("6sigma")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,6*ptOtherTestCell->lfSigma,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsFieldsList.contains(QString("min")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfMin,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsFieldsList.contains(QString("max")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfMax,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsFieldsList.contains(QString("range")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfRange,ptOtherTestCell->res_scal));
        }
        if((qslStatisticsCompareFieldsList.contains(QString("max_range"))) && (m_pReportOptions->iGroups > 1))
        {
            // Only display max. range on first group line
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else if(!uGroupID)
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,
                  pFile->FormatTestResult(ptTestReferenceCell,ptTestReferenceCell->lfMaxRange,
                                          ptTestReferenceCell->res_scal));
            else
                fprintf(hReportFile,"<td bgcolor=%s>&nbsp;</td>\n",szDataColor);
        }
        if(qslStatisticsFieldsList.contains(QString("cp")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
            {
                // Highlight in red if Cp Alarm!
                strBackgroundColor = szDataColor;	// Default: no alarm, all is fine
                bool bRedAlarmCpValidity, bYellowAlarmCpValidity;
                double lfRedAlarmCpValue, lfYellowAlarmCpValue;

                GEX_ASSERT(m_pReportOptions);		// check pReportOptions != NULL

                lfRedAlarmCpValue = m_pReportOptions->GetOption("statistics", "alarm_test_cp").toDouble(&bRedAlarmCpValidity);
                lfYellowAlarmCpValue = m_pReportOptions->GetOption("statistics", "warning_test_cp").toDouble(&bYellowAlarmCpValidity);
                GEX_ASSERT( bRedAlarmCpValidity && bYellowAlarmCpValidity);			// check conversion

                bRedAlarmCpValidity = (lfRedAlarmCpValue >=0) && (ptOtherTestCell->GetCurrentLimitItem()->lfCp != C_NO_CP_CPK) && (ptOtherTestCell->GetCurrentLimitItem()->lfCp < lfRedAlarmCpValue);
                bYellowAlarmCpValidity = (lfYellowAlarmCpValue >=0) && (ptOtherTestCell->GetCurrentLimitItem()->lfCp != C_NO_CP_CPK) && (ptOtherTestCell->GetCurrentLimitItem()->lfCp < lfYellowAlarmCpValue);

                if(bRedAlarmCpValidity)
                    strBackgroundColor = szAlarmColor;	// ALARM!: Cpk under Alarm level.
                else if(bYellowAlarmCpValidity)
                    strBackgroundColor = szWarningColor;	// Warning!: Cpk under Warning level

                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",strBackgroundColor.toLatin1().constData(),
                        CreateResultStringCpCrCpk(ptOtherTestCell->GetCurrentLimitItem()->lfCp));
            }
        }
        // GCORE-199
        if(qslStatisticsFieldsList.contains(QString("cr")))
        {
            if (ptTestReferenceCell->bTestType == 'F' || ptOtherTestCell->GetCurrentLimitItem()->lfCp==0.0)
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,
                        CreateResultStringCpCrCpk(1.0/ptOtherTestCell->GetCurrentLimitItem()->lfCp));
        }

        if((qslStatisticsCompareFieldsList.contains(QString("cp_shift"))) && (m_pReportOptions->iGroups > 1))
        {
            strBackgroundColor = szDataColor;	// Default: no alarm, all is fine

            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_cp")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            if(bGetOptionRslt)
            {
                bGetOptionRslt = (lfOptionStorageDevice>=0) && ( (fabs(lMLShift.mCpShift)) >= lfOptionStorageDevice);

                if(uGroupID && bGetOptionRslt && ptOtherTestCell->bTestType != 'F')
                {
                    // Mark this test with its Alarm bit set
                    ptTestReferenceCell->iAlarm |= GEX_ALARM_CPSHIFT;
                    ptOtherTestCell->iAlarm |= GEX_ALARM_CPSHIFT;
                    strBackgroundColor = szAlarmColor;	// ALARM!: Mean shift over limit.
                }
            }

            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n", szDataColor, GEX_NA);
            else if(!uGroupID)
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >0 %%</td>\n",szDataColor);
            else
            {
                if(lMLShift.mCpShift > C_INFINITE_PERCENTAGE)
                    lMLShift.mCpShift = C_INFINITE_PERCENTAGE;
                if(lMLShift.mCpShift < -C_INFINITE_PERCENTAGE)
                    lMLShift.mCpShift = -C_INFINITE_PERCENTAGE;
                fprintf(hReportFile,
                        "<td bgcolor=%s align=\"center\" >%s %%</td>\n",
                        strBackgroundColor.toLatin1().constData(),
                        CreateResultStringCpCrCpkShift(lMLShift.mCpShift));
            }
        }
        if((qslStatisticsCompareFieldsList.contains(QString("cr_shift"))) && (m_pReportOptions->iGroups > 1))
        {
            strBackgroundColor = szDataColor;	// Default: no alarm, all is fine

            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_cr")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            if(bGetOptionRslt)
            {
                bGetOptionRslt = (lfOptionStorageDevice>=0) && ( (fabs(lMLShift.mCrShift)) >= lfOptionStorageDevice);

                if(uGroupID && bGetOptionRslt && ptOtherTestCell->bTestType != 'F')
                {
                    // Mark this test with its Alarm bit set
                    ptTestReferenceCell->iAlarm |= GEX_ALARM_CRSHIFT;
                    ptOtherTestCell->iAlarm |= GEX_ALARM_CRSHIFT;
                    strBackgroundColor = szAlarmColor;	// ALARM!: Mean shift over limit.
                }
            }

            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n", szDataColor, GEX_NA);
            else if(!uGroupID)
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >0 %%</td>\n",szDataColor);
            else
            {
                if(lMLShift.mCrShift > C_INFINITE_PERCENTAGE)
                    lMLShift.mCrShift = C_INFINITE_PERCENTAGE;
                if(lMLShift.mCrShift < -C_INFINITE_PERCENTAGE)
                    lMLShift.mCrShift = -C_INFINITE_PERCENTAGE;
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s %%</td>\n",
                        strBackgroundColor.toLatin1().constData(),
                        CreateResultStringCpCrCpkShift(lMLShift.mCrShift));
            }
        }
        if(qslStatisticsFieldsList.contains(QString("cpk")))
        {
            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n", szDataColor, GEX_NA);
            else
            {
                // Highlight in red if Cpk Alarm!
                strBackgroundColor = szDataColor;	// Default: no alarm, all is fine
                bool bRedAlarmCpkValidity, bYellowAlarmCpkValidity;
                double lfRedAlarmCpkValue, lfYellowAlarmCpkValue;

                GEX_ASSERT(m_pReportOptions);		// check pReportOptions != NULL

                lfRedAlarmCpkValue = m_pReportOptions->GetOption("statistics", "alarm_test_cpk").toDouble(&bRedAlarmCpkValidity);
                lfYellowAlarmCpkValue = m_pReportOptions->GetOption("statistics", "warning_test_cpk").toDouble(&bYellowAlarmCpkValidity);
                GEX_ASSERT( bRedAlarmCpkValidity && bYellowAlarmCpkValidity);			// check conversion

                bRedAlarmCpkValidity = (lfRedAlarmCpkValue >=0) && (ptOtherTestCell->GetCurrentLimitItem()->lfCpk != C_NO_CP_CPK) && (ptOtherTestCell->GetCurrentLimitItem()->lfCpk < lfRedAlarmCpkValue);
                bYellowAlarmCpkValidity = (lfYellowAlarmCpkValue >=0) && (ptOtherTestCell->GetCurrentLimitItem()->lfCpk != C_NO_CP_CPK) && (ptOtherTestCell->GetCurrentLimitItem()->lfCpk < lfYellowAlarmCpkValue);

                if(bRedAlarmCpkValidity)
                    strBackgroundColor = szAlarmColor;	// ALARM!: Cpk under Alarm level.
                else if(bYellowAlarmCpkValidity)
                    strBackgroundColor = szWarningColor;	// Warning!: Cpk under Warning level

                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",strBackgroundColor.toLatin1().constData(), CreateResultStringCpCrCpk(ptOtherTestCell->GetCurrentLimitItem()->lfCpk));
            }
        }
        if(qslStatisticsAdvFieldsList.contains(QString("cpk_l")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,CreateResultStringCpCrCpk(ptOtherTestCell->GetCurrentLimitItem()->lfCpkLow));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("cpk_h")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,CreateResultStringCpCrCpk(ptOtherTestCell->GetCurrentLimitItem()->lfCpkHigh));
        }
        if((qslStatisticsCompareFieldsList.contains(QString("cpk_shift"))) && (m_pReportOptions->iGroups > 1))
        {
            strBackgroundColor = szDataColor;	// Default: no alarm, all is fine

            lfOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_cpk")).toDouble(&bGetOptionRslt);
            GEX_ASSERT(bGetOptionRslt);

            if(bGetOptionRslt)
            {
                bGetOptionRslt = (lfOptionStorageDevice>=0) && ( (fabs(lMLShift.mCpkShift)) >= lfOptionStorageDevice);

                if(uGroupID && bGetOptionRslt && ptOtherTestCell->bTestType != 'F')
                {
                    // Mark this test with its Alarm bit set
                    ptTestReferenceCell->iAlarm |= GEX_ALARM_CPKSHIFT;
                    ptOtherTestCell->iAlarm |= GEX_ALARM_CPKSHIFT;
                    strBackgroundColor = szAlarmColor;	// ALARM!: Mean shift over limit.
                }
            }

            if (ptOtherTestCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n", szDataColor, GEX_NA);
            else if(!uGroupID)
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >0.00 %%</td>\n",strBackgroundColor.toLatin1().constData());
            else
            {
                if(lMLShift.mCpkShift > C_INFINITE_PERCENTAGE)
                    lMLShift.mCpkShift = C_INFINITE_PERCENTAGE;
                if(lMLShift.mCpkShift < -C_INFINITE_PERCENTAGE)
                    lMLShift.mCpkShift = -C_INFINITE_PERCENTAGE;
                fprintf(hReportFile,
                        "<td bgcolor=%s align=\"center\" >%s %%</td>\n",
                        strBackgroundColor.toLatin1().constData(),
                        CreateResultStringCpCrCpkShift(lMLShift.mCpkShift));
            }
        }
        if(qslStatisticsFieldsList.contains(QString("yield")))
        {
            double lfData=-1;
            if(ptOtherTestCell->ldExecs > 0)
            {
                lfData = 100.0 -(100.0*ptOtherTestCell->GetCurrentLimitItem()->ldFailCount/ptOtherTestCell->ldExecs);
                ptChar = CreateResultStringPercent(lfData);
            }
            else
                ptChar = GEX_NA;
            // Highlight in red if Yield Alarm!
            strBackgroundColor = szDataColor;	// Default: no alarm, all is fine

            double lfYieldAlarmOptionStorageDevice, lfYieldWarningOptionStorageDevice;
            bool bYieldAlarmGetOptionRslt=false, bYieldWarningGetOptionRslt=false;

            lfYieldAlarmOptionStorageDevice = (m_pReportOptions->GetOption("statistics","alarm_test_yield")).toDouble(&bYieldAlarmGetOptionRslt);
            lfYieldWarningOptionStorageDevice = (m_pReportOptions->GetOption("statistics","warning_test_yield")).toDouble(&bYieldWarningGetOptionRslt);
            GEX_ASSERT(bYieldAlarmGetOptionRslt && bYieldWarningGetOptionRslt);

            bYieldAlarmGetOptionRslt = (lfYieldAlarmOptionStorageDevice >= 0) && (lfData < lfYieldAlarmOptionStorageDevice);
            bYieldWarningGetOptionRslt = (lfYieldWarningOptionStorageDevice >= 0) && (lfData < lfYieldWarningOptionStorageDevice);
            // !!!! if (lfData < lfYieldWarningOptionStorageDevice), of course (lfData < lfYieldAlarmOptionStorageDevice) !!!!!

            if((lfData >= 0) && bYieldAlarmGetOptionRslt)
                strBackgroundColor = szAlarmColor;		// ALARM!: Cpk under Alarm level.
            else if((lfData >= 0) && bYieldWarningGetOptionRslt && (!bYieldAlarmGetOptionRslt) )
                strBackgroundColor = szWarningColor;	// Warning!: Cpk under Warning level

            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",strBackgroundColor.toLatin1().constData(),ptChar);
        }
        if(qslStatisticsGageFieldsList.contains(QString("ev")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfEV,fData,lfPercent);
            }
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("av")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfAV,fData,lfPercent);
            }
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("r&r")))
        {
            // Compute R&R
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "&nbsp;";
                strBackgroundColor = szDataColor;
            }
            else
            {
                // First group
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfRR,fData,lfPercent);

                // Check if R&R% alarm
                getR_R_AlarmColor(ptOtherTestCell, lfPercent, strBackgroundColor, cBackgroundColor);
            }

            // Print result
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",strBackgroundColor.toLatin1().constData(), strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("gb")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfGB,-1,lfPercent);
            }
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >&nbsp;%s</td>\n",szDataColor,strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("pv")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfPV,fData,lfPercent);
            }
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >&nbsp;%s</td>\n",szDataColor,strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("tv")))
        {
            if(uGroupID || ptOtherTestCell->pGage == NULL)
            {
                strString = "";
            }
            else
            {
                // Check if % computed over TV or Limit space
                if(bBoxplotShiftOverTV)
                    fData = (ptOtherTestCell->pGage->lfTV) ? ptOtherTestCell->pGage->lfTV : 1;
                else
                    fData = (lfLimitSpace) ? lfLimitSpace : 1;

                strString = ValueAndPercentageString(ptOtherTestCell->pGage->lfTV,fData,lfPercent);
            }
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >&nbsp;%s</td>\n",szDataColor,strString.toLatin1().constData());
        }
        if(qslStatisticsGageFieldsList.contains(QString("p_t")))
        {
            strString = (uGroupID || ptOtherTestCell->pGage == NULL) ? " " : ValueAndPercentageString(ptOtherTestCell->pGage->lfP_T,lfLimitSpace,lfPercent);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >&nbsp;%s</td>\n",szDataColor,strString.toLatin1().constData());
        }
        if(qslStatisticsAdvFieldsList.contains(QString("skew")))
        {
            if(ptOtherTestCell->lfSamplesSkew != -C_INFINITE && ptOtherTestCell->bTestType != 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%g</td>\n",szDataColor,ptOtherTestCell->lfSamplesSkew);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
        }
        if(qslStatisticsAdvFieldsList.contains(QString("kurtosis")))
        {
            if(ptOtherTestCell->lfSamplesKurt != -C_INFINITE && ptOtherTestCell->bTestType != 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%g</td>\n",szDataColor,ptOtherTestCell->lfSamplesKurt);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P0.5")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP0_5,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P2.5")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP2_5,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P10")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP10,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("quartile1")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesQuartile1,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("quartile2")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
            {
                // Highlight in red if mean outside of limits!
                if( (((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) && (ptOtherTestCell->lfSamplesQuartile2 < ptOtherTestCell->GetCurrentLimitItem()->lfLowLimit)) ||
                    (((ptOtherTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) && (ptOtherTestCell->lfSamplesQuartile2 > ptOtherTestCell->GetCurrentLimitItem()->lfHighLimit)) )
                    strBackgroundColor = szAlarmColor;
                else
                    strBackgroundColor = szDataColor;

                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",strBackgroundColor.toLatin1().constData(), pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesQuartile2,ptOtherTestCell->res_scal));
            }
        }
        if(qslStatisticsAdvFieldsList.contains(QString("quartile3")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesQuartile3,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P90")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP90,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P97.5")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP97_5,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("P99.5")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesP99_5,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("inter_quartiles")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesQuartile3-ptOtherTestCell->lfSamplesQuartile1,ptOtherTestCell->res_scal));
        }
        if(qslStatisticsAdvFieldsList.contains(QString("sigma_inter_quartiles")))
        {
            if (ptTestReferenceCell->bTestType == 'F')
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor, GEX_NA);
            else
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" >%s</td>\n",szDataColor,pFile->FormatTestResult(ptOtherTestCell,ptOtherTestCell->lfSamplesSigmaInterQuartiles,ptOtherTestCell->res_scal));
        }

        fprintf(hReportFile,"</tr>\n");
    }
}

void CGexReport::WriteStatsLines(CTest *ptTestReferenceCell,int iLine,
                                 const QString &strStatisticsFieldsOptions,
                                 bool bMeanShiftAlarmOnly/*=false*/,bool bSigmaShiftAlarmOnly/*=false*/,
                                 const QString &strOutputFormat)
{
    CGexGroupOfFiles *	pGroup=0;
    CGexFileInGroup  *	pFile=0;
    CTest *				ptOtherTestCell=0;	// Pointer to test cell of groups2 or higher
    int					lPinmapIndex=0;
    int					nGroupID = 0;

    // Create one stats line per test of each group...
    if (getGroupsList().count() == 1)
    {
        // Get handle to group
        pGroup = (nGroupID >= getGroupsList().size()) ? NULL : getGroupsList().at(nGroupID);

        // List of tests in group#2 or group#3, etc...
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        WriteStatsLinesEx(pGroup, pFile, nGroupID, ptTestReferenceCell, ptTestReferenceCell,
                          iLine, strStatisticsFieldsOptions,
                          bMeanShiftAlarmOnly,bSigmaShiftAlarmOnly);
    }
    else
    {
        for(nGroupID=0; nGroupID < getGroupsList().count(); nGroupID++)
        {
            if( ( strOutputFormat == "PPT" ) &&  ( nGroupID > 0 ) && ( nGroupID % 0x0f == 0 ) )
            {
                CreateNewHtmlStatisticPage();
                WriteStatisticsLabelLine();
            }

            // Get handle to group
            pGroup = getGroupsList().at(nGroupID);
            if (!pGroup)
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Unfindable group %1").arg(nGroupID).toLatin1().data() );
                continue; // or break ?
            }

            // List of tests in group#2 or group#3, etc...
            pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            if (!pFile)
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Unfindable file for group '%1'").arg(pGroup->strGroupName)
                      .toLatin1().data() );
                continue; // or break ?
            }

            // Check if pinmap index...
            lPinmapIndex = ptTestReferenceCell->lPinmapIndex;
            switch(ptTestReferenceCell->lPinmapIndex)
            {
                case GEX_PTEST:	//  Parametric
                case GEX_MPTEST:
                    lPinmapIndex = GEX_PTEST;
                    break;
                default:		// Pinmap of multi-parametric
                case GEX_FTEST:	// Functional: keep pinmap index as-is.
                    break;
            }

            // Get pointer to test cell.
            if(pFile->FindTestCell(
                  ptTestReferenceCell->lTestNumber,lPinmapIndex,&ptOtherTestCell,false,false,
                        ptTestReferenceCell->strTestName.toLatin1().data()) != 1)
                goto next_stats_test;	// This test doesn't exist, move to next!

            WriteStatsLinesEx(pGroup, pFile, nGroupID, ptTestReferenceCell, ptOtherTestCell, iLine,
                              strStatisticsFieldsOptions,
                              bMeanShiftAlarmOnly,bSigmaShiftAlarmOnly);

next_stats_test:;
        }
    }
}


TestShift CGexReport::ComputeStatsShift(CTest* referenceTest, CTest* testToCompare, TestShift& testShift)
{
    if (!testToCompare || !referenceTest)
        return testShift;

    QString strOptionStorageDevice;
    strOptionStorageDevice = (m_pReportOptions->GetOption("statistics","mean_drift_formula")).toString();
    testShift.SetValidity((referenceTest->ldSamplesValidExecs > 0) && (testToCompare->ldSamplesValidExecs > 0));

//DEBUG NO COMMIT 1
    /*
    if(testToCompare->GetUserTestName() == "BG_pst_3.8")
        GSLOG(SYSLOG_SEV_DEBUG, "WOTOXOR");
*/

    if ( strOptionStorageDevice == QString("limits") )
    {
        testShift.mMeanShiftValue = testToCompare->lfMean - referenceTest->lfMean;
        // Percentage of limits space drift.
        for (int lMLIdx = 0; lMLIdx < referenceTest->MultiLimitItemCount(); ++lMLIdx)
        {
            GS::Core::MultiLimitItem* lRefML = referenceTest->GetMultiLimitItem(lMLIdx);
            GS::Core::MultiLimitItem* lComparedML = testToCompare->GetMultiLimitItem(lMLIdx);
            GS::Core::MLShift lMLimitShift = testShift.GetMlShift(lComparedML);
            if ((lRefML->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0)
            {
                // If both limits exist... (BYTE	bLimitFlag;
                // bit0=1 (no low limit), bit1=1 (no high limit))
                lMLimitShift.mMeanShiftPct = testShift.mMeanShiftValue;
                double lRefLimitsRange = (lRefML->lfHighLimit - lRefML->lfLowLimit) / 100.0;
                if(lRefLimitsRange)
                    lMLimitShift.mMeanShiftPct /= lRefLimitsRange;
            }
            else
            {
                // Not both limits exist....can't compute drift...
                lMLimitShift.mMeanShiftPct = 0;
            }
            testShift.SetMlShift(lComparedML, lMLimitShift);
        }
    }
    else // ( strOptionStorageDevice == QString("value") or anything else )
    {
        // Absolute value shift.
        double lMeanShitPct;
        if(referenceTest->lfMean != 0)
        {
            testShift.mMeanShiftValue = testToCompare->lfMean - referenceTest->lfMean;
            if (testShift.mMeanShiftValue != 0.0)
                lMeanShitPct = 100.0 * (testShift.mMeanShiftValue / fabs(referenceTest->lfMean));
            else
                lMeanShitPct = 0.0;
        }
        else
        {
            if(testToCompare->lfMean == 0)
                lMeanShitPct = 0;	// Both Mean = 0!
            else
                lMeanShitPct = 100.0 * ((testToCompare->lfMean / 1e-12) - 1);
        }
        for (int lMLIdx=0; lMLIdx < referenceTest->MultiLimitItemCount(); ++lMLIdx)
        {
            GS::Core::MultiLimitItem* lComparedML = testToCompare->GetMultiLimitItem(lMLIdx);
            GS::Core::MLShift lMLimitShift = testShift.GetMlShift(lComparedML);
            lMLimitShift.mMeanShiftPct = lMeanShitPct;
            testShift.SetMlShift(lComparedML, lMLimitShift);
        }
    }

    // If original sigma != 0
    if(referenceTest->lfSigma != 0)
    {
        testShift.mSigmaShiftValue = testToCompare->lfSigma - referenceTest->lfSigma;
        testShift.mSigmaShiftPercent = 100.0 * ((testToCompare->lfSigma / referenceTest->lfSigma) - 1);
    }
    else
    {
        if(testToCompare->lfSigma == 0)
        {
            testShift.mSigmaShiftPercent = 0;	// Both sigma = 0!
            testShift.mSigmaShiftValue = 0;
        }
        else
            testShift.mSigmaShiftPercent = 100.0 * ((testToCompare->lfSigma / 1e-12) - 1);
    }
    // If drift too high...truncate it!
    if(testShift.mSigmaShiftPercent > C_INFINITE_PERCENTAGE)
        testShift.mSigmaShiftPercent = C_INFINITE_PERCENTAGE;
    if(testShift.mSigmaShiftPercent < -C_INFINITE_PERCENTAGE)
        testShift.mSigmaShiftPercent = -C_INFINITE_PERCENTAGE;

    for (int lMLIdx = 0; lMLIdx < testToCompare->MultiLimitItemCount(); ++lMLIdx)
    {
        GS::Core::MultiLimitItem* lMLToCompare = testToCompare->GetMultiLimitItem(lMLIdx);
        GS::Core::MultiLimitItem* lRefML = referenceTest->GetMultiLimitItem(lMLIdx);
        if (!lMLToCompare ||
                !lRefML ||
                lMLToCompare->IsValid() == false ||
                lRefML->IsValid() == false)
            continue;

        GS::Core::MLShift& lMLimitShift = testShift.mMLShifts[lMLIdx].second;
        // If the 2 Cp exist...compute the Cp shift
        if((lMLToCompare->lfCp > 0) && (lRefML->lfCp > 0))
            lMLimitShift.SetCpShiftValue(100.0*((lMLToCompare->lfCp/lRefML->lfCp)-1));

        // If the 2 Cp exist...compute the Cr shift considering Cr = 1/Cp
        if((lMLToCompare->lfCp > 0) && (lRefML->lfCp > 0))
            lMLimitShift.SetCrShiftValue(100.0*((lRefML->lfCp/lMLToCompare->lfCp)-1));

        // If the 2 Cpk exist...compute the shift
        if((lMLToCompare->lfCpk != C_NO_CP_CPK) && (lRefML->lfCpk != C_NO_CP_CPK) &&
                (lMLToCompare->lfCpk != 0) && (lRefML->lfCpk != 0))
            lMLimitShift.SetCpkShiftValue(100.0*((lMLToCompare->lfCpk/lRefML->lfCpk)-1));

        testShift.mMLShifts[lMLIdx] =
                    QPair<GS::Core::MultiLimitItem*, GS::Core::MLShift>(lMLToCompare,lMLimitShift);
    }

    return testShift;
}

int	CGexReport::ComputeStatsShift(void)
{
    CGexGroupOfFiles *lRefGroup;
    CGexFileInGroup  *lFileToCompare;
    CTest			*lComparedTest;	// Pointer to test cell of groups2 or higher
    CTest           *lRefTest;
    int				lPinmapIndex;
    // Variables for computing student's T-Test
    double	lfValue;

    // Return if not at least 2 groups to compare !
    if(getGroupsList().count() <=1)
        return 0;

    int lMaxRefGroupToCheck = 1;
    if ((m_pReportOptions->GetOption("statistics","shift_with")).toString()=="shift_to_all")
    {
        lMaxRefGroupToCheck = getGroupsList().size();
    }

    // loop on all group
    // Compare with all other groups if needed
    for (int lRefGroupIdx = 0; lRefGroupIdx < lMaxRefGroupToCheck; ++lRefGroupIdx)
    {
        // For each group, loop on each test
        // First: write info about reference group#1
        lRefGroup = getGroupsList().at(lRefGroupIdx);
        lRefTest = lRefGroup->cMergedData.ptMergedTestList;
        // For each test, calculate the shift with all other tests
        while(lRefTest != NULL)
        {
            lRefTest->lfMaxRange = 0.0;
            // For each test, scan all the groups!
            for (int lComparedGroupIdx = 0; lComparedGroupIdx < getGroupsList().size(); ++lComparedGroupIdx)
            {
                CGexGroupOfFiles *lComparedGroup = getGroupsList().at(lComparedGroupIdx);
                if (!lComparedGroup)
                    continue;
                lFileToCompare	= (lComparedGroup->pFilesList.isEmpty()) ? NULL : lComparedGroup->pFilesList.first();
                // Check if pinmap index...
                if(lRefTest->lPinmapIndex >= 0)
                    lPinmapIndex = lRefTest->lPinmapIndex;
                else
                    lPinmapIndex = GEX_PTEST;

                lComparedTest = NULL;
                if(lFileToCompare->FindTestCell(lRefTest->lTestNumber, lPinmapIndex, &lComparedTest, false, false,
                                                lRefTest->strTestName.toLatin1().data())==1)
                {
                    TestShift lTestShift;
                    if (!lComparedTest->mTestShifts.contains(lRefGroup->strGroupName)) //if not exists init
                        lTestShift.SetGroups(lRefGroup->strGroupName, lComparedGroup->strGroupName);
                    else
                        lTestShift = lComparedTest->mTestShifts.value(lRefGroup->strGroupName);
                    ComputeStatsShift(lRefTest, lComparedTest, lTestShift);
                    lComparedTest->mTestShifts.insert(lRefGroup->strGroupName, lTestShift); // replace if exists
                }

                // Update Max. range value
                // For the first group, only take the range, otherwise, calculate the max range
                if (lComparedGroupIdx == 0 && lComparedGroup->GetGroupId() == lRefGroup->GetGroupId())
                {
                    lRefTest->lfMaxRange = lRefTest->lfRange;
                }
                else if (lComparedTest)
                {
                    lRefTest->lfMaxRange= gex_maxAbs(lRefTest->lfMaxRange, lComparedTest->lfRange);
                }

                // If only two groups compared, then compte Student's T-Test
                if((getGroupsList().count() == 2) && (lComparedTest != NULL) &&
                        lRefTest->ldSamplesValidExecs > 2 &&
                        lComparedTest->ldSamplesValidExecs > 2)
                {
                    // T-Test formula can be found at: http://en.wikipedia.org/wiki/Student's_t-test
                    lfValue = ((double)lRefTest->ldSamplesValidExecs-1)*GS_POW(lRefTest->lfSigma,2.0);
                    lfValue += ((double)lComparedTest->ldSamplesValidExecs-1)*GS_POW(lComparedTest->lfSigma,2.0);
                    lfValue *= (((double)1.0/(double)lRefTest->ldSamplesValidExecs) + ((double)1.0/(double)lComparedTest->ldSamplesValidExecs));
                    lfValue /= (double)(lRefTest->ldSamplesValidExecs+lComparedTest->ldSamplesValidExecs-2);
                    if(lfValue)
                        lfValue = (lRefTest->lfMean-lComparedTest->lfMean)/sqrt(lfValue);
                    else
                        lfValue = C_INFINITE;	// Division by 0....

                    lRefTest->setT_Test(lfValue);
                    lComparedTest->setT_Test(lfValue);
                    if(lfValue < C_INFINITE)
                    {
                        double dFd = (lRefTest->ldSamplesValidExecs+lComparedTest->ldSamplesValidExecs - 1);
                        lRefTest->setP_Value(qAbs(lfValue)/dFd);
                        lComparedTest->setP_Value(qAbs(lfValue)/dFd);
                    }
                    else
                    {
                        lRefTest->setP_Value(C_INFINITE);
                        lComparedTest->setP_Value(C_INFINITE);
                    }
                }
            }
            // Move to next test cell
            lRefTest = lRefTest->GetNextTest();
        };
    }
    // Drifts computed
    return 1;
}

// Creates ALL pages for the Test Statistics report
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CreatePages_Stats(qtTestListStatistics *pqtStatisticsList_Stats,int &iCellIndex/*=0*/,
                bool bMeanShiftAlarmOnly/*=false*/,bool bSigmaShiftAlarmOnly/*=false*/,int iMaxParameters/*=-1*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Creating Stats page...");
    // Scan the list of tests, and decide which test
    // will belong to which histogram page
    CTest	*ptTestCell = NULL;	// Pointer to test cell to receive STDF info.
    int		iStatus;

    // Status bar message.
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Building Statistics...");

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    QString strStatisticsFieldsOptions = (ReportOptions.GetOption(QString("statistics"), QString("fields"))).toString();

    // Do not create this section if:
    // o section disabled and output format != HTML
    // o HTML-based format and secion is part of the sections to skip
    if(	( (m_pReportOptions->iStatsType == GEX_STATISTICS_DISABLED)
        && (strOutputFormat!="HTML")
            ) ||
        (	m_pReportOptions->isReportOutputHtmlBased()
                //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            && (m_pReportOptions->iHtmlSectionsToSkip & GEX_HTMLSECTION_STATISTICS)))
    {
        // Update process bar...
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
        return GS::StdLib::Stdf::NoError;
    }

    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.

    iStatus = PrepareSection_Stats(true);
    if(iStatus != GS::StdLib::Stdf::NoError)
        return iStatus;

    // In case no data match the filter, ensure first page is erased (hyperlink always exists on home page!)
    char	szString[2048];
    sprintf(szString,"%s/pages/stats1.htm", m_pReportOptions->strReportDirectory.toLatin1().constData());
    unlink(szString);

    // Reset HTML page counter
    iCurrentHtmlPage=0;

    int		iStatsLine=0;	// Keeps track of number of line written
    int		iLinesInPage=0;	// When creating flat HTML, must insert page break at smart position (before too many lines are written)
    int		iParametersInPage=0;

    // Read list based on sorting filter (defined by user in 'Options')
    foreach(ptTestCell, *pqtStatisticsList_Stats)
    {
        // Write page break (ignored if not writing a flat HTML document)
        if(iLinesInPage && (iLinesInPage >= iLinesPerFlatPage)
                && ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
            )
        {
            // Insert page break (only for PowerPoint as to avoid writing outside of visible image window; PDF & Word: let the application manage page breaks in table.
            CreateNewHtmlStatisticPage();

            iStatsLine = 0;	// Ensure we do not have more than about 15 lines before new line header.
            iLinesInPage = 0;
        }

        // Function writes report for each test (list data for all groups)
        WriteStatsLines(ptTestCell,iStatsLine, strStatisticsFieldsOptions, bMeanShiftAlarmOnly,bSigmaShiftAlarmOnly, strOutputFormat);

        // If single group: table header is written every 15 lines ; if comparing files (group#>1): header is after EACH test block.
        if(m_pReportOptions->iGroups == 1)
        {
            iStatsLine++;	// 15 lines blocks maximum before writting again the table field names...each group has one line per test!

            // Keep track of the line count (never reset, incremented at each line written)
            iLinesInPage++;
        }
        else
        {
            if (strOutputFormat=="PPT")
                CreateNewHtmlStatisticPage();
        }
        if(
            (iStatsLine > 0xf)
                && (strOutputFormat=="HTML")
            )
            iStatsLine = 0;	// Ensure we do not have more than about 15 lines before new line header.Only applies to standad HTML pages, not flat HTML file.

        // Parameters per page
        ++iParametersInPage;

        ++iCellIndex;

        if((iMaxParameters > 0) && (iParametersInPage > iMaxParameters))
            break;
    };	// Loop until all test cells read.

    // If full test list processed....
    if(iCellIndex == -1 || ptTestCell == NULL || iCellIndex == pqtStatisticsList_Stats->count())
    {
        iCellIndex = -1;	// Ensures caller knows we're done with the list.
        // If at least one Statistics page was generated, close it.
        if(iCurrentHtmlPage)
        {
            // Close table of statistics
            fprintf(hReportFile,"</table>\n");

            // Write navigation bar.
            WriteNavigationButtons(GEX_T("stats"));
        }
        iStatus = CloseSection_Stats(true, pqtStatisticsList_Stats);
    }
    else
        iStatus = GS::StdLib::Stdf::NoError;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "End of creating Stats page...");
    return iStatus;
}

