/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Trend' page.
/////////////////////////////////////////////////////////////////////////////

#include "report_build.h"
#include "report_options.h"
#include "report_classes_sorting.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "engine.h"
#include "gqtl_global.h"

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_AdvOptimizerDiags(BOOL /*bValidSection*/)
{
    CGexGroupOfFiles *pGroup = pGroupsList.isEmpty()?NULL:pGroupsList.first();
    CTest	*ptTestCell;	// Pointer to test cell to receive STDF info.
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    // Creates the 'Trend' page & header
    if(strOutputFormat=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Advanced report ----\nNOT Supported in .CSV format. You must switch to HTML report mode!\n\n");
    }
    else
        if (m_pReportOptions->isReportOutputHtmlBased())
        //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        mTotalAdvancedPages = 1;// Number of boxplot HTML pages that will be generated
        ptTestCell = pGroup->cMergedData.ptMergedTestList;
        while(ptTestCell != NULL)
        {
            if(ptTestCell->ldSamplesValidExecs > 0)
            {
                ptTestCell->iHtmlAdvancedPage = mTotalAdvancedPages;
            }
            // Point to next test cell
            ptTestCell = ptTestCell->GetNextTest();
        };

        // Will be used while creating all Histogram pages
        mTotalHtmlPages = mTotalAdvancedPages;

        // Keep count of total HTML pages to create (used to compute the ProgreeGauge size)
        m_pReportOptions->lAdvancedHtmlPages = mTotalAdvancedPages;
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Description : Compute the Kurtosis value for the specified list of samples.
//
// Argument(s) :
//      const CTestResult& testResult : The array that contains samples
//
//      dfResult: Receive the kurtosis value
//				 ( The kurtosis of a normal distribution is 0. )
//
// Return: true if successful;
//		   false if sigma is NULL (kurtosis value cannot be computed)
//
/////////////////////////////////////////////////////////////////////////////////////////////
bool ComputeKurtosis(const CTestResult& testResult, double *dfResult)
{
    double	dfValue=0.0,dfCumulX=0.0,dfCumulX2=0.0,dfMean=0.0,dfSigma=0.0;
    long	lTotalSamples=0;

    *dfResult = 0.0;

    for(int nIndex = 0; nIndex < testResult.count(); nIndex++)
    {
        if (testResult.isValidResultAt(nIndex))
        {
            if (testResult.isMultiResultAt(nIndex))
            {
                for (int nCount = 0; nCount < testResult.at(nIndex)->count(); ++nCount)
                {
                    if(testResult.at(nIndex)->isValidResultAt(nCount)){
                    dfValue		= testResult.at(nIndex)->multiResultAt(nCount);
                    dfCumulX	+= dfValue;
                    dfCumulX2	+= GS_POW(dfValue, 2.0 );

                    lTotalSamples++;
                    }
                }
            }
            else
            {
                dfValue		= testResult.resultAt(nIndex);
                dfCumulX	+= dfValue;
                dfCumulX2	+= GS_POW(dfValue, 2.0 );

                lTotalSamples++;
            }
        }
    }

    // Get the mean
    dfMean = dfCumulX / lTotalSamples;
    // Get the standard deviation (sigma)
    dfSigma = double(lTotalSamples)*GS_POW(dfMean,2.0) + dfCumulX2;
    dfSigma -= 2.0*dfMean*dfCumulX;
    dfSigma = fabs(dfSigma/double(lTotalSamples-1));
    if(dfSigma != 0.0)
        dfSigma = sqrt(dfSigma);
    else
        return false;


    // Apply the Kurtosis formula, using mean and sigma
    double	dfNewCumul	= 0.0;
    int		nTotalCount = 0;

    for(int nIndex = 0; nIndex < testResult.count(); nIndex++)
    {
        if(testResult.isValidResultAt(nIndex))
        {
            if (testResult.isMultiResultAt(nIndex))
            {
                for (int nCount = 0; nCount < testResult.at(nIndex)->count(); ++nCount)
                {
                    if(testResult.at(nIndex)->isValidResultAt(nCount)){
                    dfCumulX	= testResult.at(nIndex)->multiResultAt(nCount) - dfMean;
                    dfCumulX2	= GS_POW(dfCumulX, 4.0 );
                    dfNewCumul	+= dfCumulX2;
                    }
                }

                nTotalCount += testResult.at(nIndex)->count();
            }
            else
            {
                dfCumulX	= testResult.resultAt(nIndex) - dfMean;
                dfCumulX2	= GS_POW(dfCumulX, 4.0 );
                dfNewCumul	+= dfCumulX2;

                nTotalCount += 1;
            }
        }


    }

    *dfResult = (dfNewCumul / (nTotalCount * GS_POW(dfSigma,4.0) ) ) - 3.0;

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_AdvOptimizerDiags(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if(strOutputFormat=="HTML")

    {
        fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
        fprintf(hReportFile,"</body>\n");
        fprintf(hReportFile,"</html>\n");
        CloseReportFile();	// Close report file
    }
    return GS::StdLib::Stdf::NoError;
}


/////////////////////////////////////////////////////////////////////////////
// Writes Optimizer Diagnostic pages.
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteAdvOptimizerDiagsPage(void)
{
    CTest				*ptTestCell;	// Pointer to test cell to receive STDF info.
    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles	*pGroup = pGroupsList.isEmpty()?NULL:pGroupsList.first();

    // Open advanced.htm file
    if(OpenFile_Advanced() != GS::StdLib::Stdf::NoError)
        return;

    // Most of functions keep writing to the 'hReportFile' file.
    hReportFile = hAdvancedReport;

    WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black
    fprintf(hReportFile,"<p align=\"left\"><font color=\"#006699\" size=\"4\">More Reports: Quantix Optimizer Diagnostics<br>\n");
    fprintf(hReportFile,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\"></font></p>\n");

    fprintf(hReportFile,"<table border=\"0\" width=\"400\" height=\"21\" cellspacing=\"1\">\n");
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td width=\"50%%\" bgcolor=%s align=\"left\"><b>Test</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"50%%\" bgcolor=%s align=\"center\"><b>Kurtosis value</b></td>\n",szDataColor);
    fprintf(hReportFile,"</tr>\n");

    ptTestCell = pGroup->cMergedData.ptMergedTestList;
    while(ptTestCell != NULL)
    {
        if(ptTestCell->ldSamplesValidExecs > 0)
        {

            fprintf(hReportFile,"<tr>\n");

            if(ptTestCell->iHtmlHistoPage)
            {
                // If this test has a histogram page, create the hyperlink!
                                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" width=\"20%%\"><b><a name=\"T%s\"></a> <a href=\"histogram%d.htm#T%s\">%s</a></b></td>\n",
                    szFieldColor,ptTestCell->szTestLabel,ptTestCell->iHtmlHistoPage,ptTestCell->szTestLabel,ptTestCell->szTestLabel);
            }
            else
            {
                // Test doesn't have a Histogram/or histogram disabled : don't create a hyperlink
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\" width=\"20%%\"><b><a name=\"T%s\">%s</a></b></td>\n",
                    szFieldColor,ptTestCell->szTestLabel,ptTestCell->szTestLabel);
            }

            double dfResult;
            if(ComputeKurtosis(ptTestCell->m_testResult, &dfResult) == false)
                fprintf(hReportFile,"<td width=\"50%%\" bgcolor=%s align=\"center\">Sigma is NULL</td>\n",szDataColor);
            else
                fprintf(hReportFile,"<td width=\"50%%\" bgcolor=%s align=\"center\">%g</td>\n",szDataColor,dfResult);
            fprintf(hReportFile,"</tr>\n");
        }

        // Point to next test cell
        ptTestCell = ptTestCell->GetNextTest();
    };
}

